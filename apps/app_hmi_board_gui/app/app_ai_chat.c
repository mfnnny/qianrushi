#include <stdio.h>
#include <string.h>

#include <rtthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "app_ai_chat.h"
#include "app_file_ini.h"
#include "app_wifi.h"

#define DBG_TAG "ai.chat"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/*
 * Run tools/ai_relay_server.py on your PC, then set this to the PC's LAN IP.
 * Keep the real API key on the PC side, not inside MCU flash.
 */
#ifndef AI_RELAY_HOST_DEFAULT
#define AI_RELAY_HOST_DEFAULT       "192.168.18.128"
#endif

#ifndef AI_RELAY_PORT_DEFAULT
#define AI_RELAY_PORT_DEFAULT       8080
#endif

#ifndef AI_RELAY_PATH
#define AI_RELAY_PATH               "/chat"
#endif

#define AI_CHAT_PROMPT_MAX_LEN      256
#define AI_CHAT_HTTP_MAX_LEN        1400
#define AI_CHAT_STACK_SIZE          8192

static volatile int g_ai_busy = 0;
static volatile int g_ai_reply_ready = 0;
static char g_ai_prompt[AI_CHAT_PROMPT_MAX_LEN];
static char g_ai_reply[AI_CHAT_REPLY_MAX_LEN];
static char g_ai_http_body[AI_CHAT_PROMPT_MAX_LEN + 32];
static char g_ai_http_escaped[AI_CHAT_PROMPT_MAX_LEN * 2];
static char g_ai_http_req[AI_CHAT_HTTP_MAX_LEN];
static char g_ai_http_rx[AI_CHAT_HTTP_MAX_LEN];
static char g_ai_relay_host[64];
static char g_ai_runtime_relay_host[64];
static int g_ai_runtime_relay_port = 0;
static struct rt_mutex g_ai_lock;
static rt_sem_t g_ai_sem = RT_NULL;
static rt_thread_t g_ai_thread = RT_NULL;
static int g_ai_lock_inited = 0;

static void ai_chat_lock_init(void)
{
    if (!g_ai_lock_inited)
    {
        rt_mutex_init(&g_ai_lock, "ai_lock", RT_IPC_FLAG_PRIO);
        g_ai_lock_inited = 1;
    }
}

static void ai_chat_set_reply(const char *text)
{
    ai_chat_lock_init();
    rt_mutex_take(&g_ai_lock, RT_WAITING_FOREVER);
    rt_strncpy(g_ai_reply, text, sizeof(g_ai_reply) - 1);
    g_ai_reply[sizeof(g_ai_reply) - 1] = '\0';
    g_ai_reply_ready = 1;
    rt_mutex_release(&g_ai_lock);
}

static int ai_chat_json_escape(char *out, int out_len, const char *in)
{
    int pos = 0;

    while (*in && pos < out_len - 1)
    {
        char ch = *in++;

        if (ch == '"' || ch == '\\')
        {
            if (pos + 2 >= out_len)
            {
                break;
            }
            out[pos++] = '\\';
            out[pos++] = ch;
        }
        else if (ch == '\r' || ch == '\n')
        {
            if (pos + 2 >= out_len)
            {
                break;
            }
            out[pos++] = '\\';
            out[pos++] = 'n';
        }
        else
        {
            out[pos++] = ch;
        }
    }

    out[pos] = '\0';
    return pos;
}

static int ai_chat_send_all(int sock, const char *buf, int len)
{
    int sent = 0;

    while (sent < len)
    {
        int ret = send(sock, buf + sent, len - sent, 0);
        if (ret <= 0)
        {
            return -1;
        }
        sent += ret;
    }

    return 0;
}

static int ai_chat_http_post(const char *prompt, char *reply, int reply_len)
{
    struct addrinfo hints;
    struct addrinfo *res = RT_NULL;
    struct addrinfo *rp = RT_NULL;
    char port_str[8];
    int sock = -1;
    int total = 0;
    int ret = -1;
    int relay_port = AI_RELAY_PORT_DEFAULT;
    const char *relay_host = AI_RELAY_HOST_DEFAULT;

    ai_chat_json_escape(g_ai_http_escaped, sizeof(g_ai_http_escaped), prompt);
    rt_snprintf(g_ai_http_body, sizeof(g_ai_http_body), "{\"prompt\":\"%s\"}", g_ai_http_escaped);

    ai_chat_lock_init();
    rt_mutex_take(&g_ai_lock, RT_WAITING_FOREVER);
    if (g_ai_runtime_relay_host[0] != '\0')
    {
        rt_strncpy(g_ai_relay_host, g_ai_runtime_relay_host, sizeof(g_ai_relay_host) - 1);
        g_ai_relay_host[sizeof(g_ai_relay_host) - 1] = '\0';
        if (g_ai_runtime_relay_port > 0)
        {
            relay_port = g_ai_runtime_relay_port;
        }
    }
    else
    {
        memset(g_ai_relay_host, 0, sizeof(g_ai_relay_host));
    }
    rt_mutex_release(&g_ai_lock);

    if (g_ai_relay_host[0] != '\0')
    {
        relay_host = g_ai_relay_host;
    }
    else if (ini_load_ai_relay_info(g_ai_relay_host, sizeof(g_ai_relay_host), &relay_port) == 0 && g_ai_relay_host[0] != '\0')
    {
        relay_host = g_ai_relay_host;
    }

    LOG_I("AI relay: %s:%d", relay_host, relay_port);
    rt_snprintf(port_str, sizeof(port_str), "%d", relay_port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(relay_host, port_str, &hints, &res) != 0)
    {
        rt_snprintf(reply, reply_len, "AI relay DNS failed: %s", relay_host);
        return -1;
    }

    for (rp = res; rp != RT_NULL; rp = rp->ai_next)
    {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0)
        {
            continue;
        }

        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }

        closesocket(sock);
        sock = -1;
    }

    freeaddrinfo(res);

    if (sock < 0)
    {
        rt_snprintf(reply, reply_len, "Connect AI relay failed: %s:%d", relay_host, relay_port);
        return -1;
    }

    rt_snprintf(g_ai_http_req, sizeof(g_ai_http_req),
                "POST %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                AI_RELAY_PATH, relay_host, (int)strlen(g_ai_http_body), g_ai_http_body);

    if (ai_chat_send_all(sock, g_ai_http_req, strlen(g_ai_http_req)) != 0)
    {
        rt_snprintf(reply, reply_len, "Send request failed");
        goto exit;
    }

    memset(g_ai_http_rx, 0, sizeof(g_ai_http_rx));
    while (total < (int)sizeof(g_ai_http_rx) - 1)
    {
        int n = recv(sock, g_ai_http_rx + total, sizeof(g_ai_http_rx) - 1 - total, 0);
        if (n <= 0)
        {
            break;
        }
        total += n;
    }
    g_ai_http_rx[total] = '\0';

    if (total <= 0)
    {
        rt_snprintf(reply, reply_len, "No reply from AI relay");
        goto exit;
    }

    {
        char *body_start = strstr(g_ai_http_rx, "\r\n\r\n");
        if (body_start)
        {
            body_start += 4;
        }
        else
        {
            body_start = g_ai_http_rx;
        }

        rt_strncpy(reply, body_start, reply_len - 1);
        reply[reply_len - 1] = '\0';
        ret = 0;
    }

exit:
    closesocket(sock);
    return ret;
}

static void ai_chat_thread_entry(void *parameter)
{
    char prompt[AI_CHAT_PROMPT_MAX_LEN];
    char reply[AI_CHAT_REPLY_MAX_LEN];

    RT_UNUSED(parameter);

    while (1)
    {
        rt_sem_take(g_ai_sem, RT_WAITING_FOREVER);

        ai_chat_lock_init();
        rt_mutex_take(&g_ai_lock, RT_WAITING_FOREVER);
        rt_strncpy(prompt, g_ai_prompt, sizeof(prompt) - 1);
        prompt[sizeof(prompt) - 1] = '\0';
        rt_mutex_release(&g_ai_lock);

        if (wifi_get_status() != 0)
        {
            ai_chat_set_reply("WiFi not ready. Set WiFi first.");
            g_ai_busy = 0;
            continue;
        }

        memset(reply, 0, sizeof(reply));
        ai_chat_http_post(prompt, reply, sizeof(reply));
        ai_chat_set_reply(reply);
        g_ai_busy = 0;
    }
}

int ai_chat_request_async(const char *prompt)
{
    if (prompt == RT_NULL || prompt[0] == '\0')
    {
        return -RT_EINVAL;
    }

    ai_chat_lock_init();
    if (g_ai_sem == RT_NULL)
    {
        g_ai_sem = rt_sem_create("ai_sem", 0, RT_IPC_FLAG_PRIO);
        if (g_ai_sem == RT_NULL)
        {
            return -RT_ERROR;
        }
    }

    if (g_ai_thread == RT_NULL)
    {
        g_ai_thread = rt_thread_create("ai_chat", ai_chat_thread_entry, RT_NULL, AI_CHAT_STACK_SIZE, 20, 20);
        if (g_ai_thread == RT_NULL)
        {
            return -RT_ERROR;
        }
        rt_thread_startup(g_ai_thread);
    }

    if (g_ai_busy)
    {
        return -RT_EBUSY;
    }

    rt_mutex_take(&g_ai_lock, RT_WAITING_FOREVER);
    rt_strncpy(g_ai_prompt, prompt, sizeof(g_ai_prompt) - 1);
    g_ai_prompt[sizeof(g_ai_prompt) - 1] = '\0';
    g_ai_reply_ready = 0;
    rt_mutex_release(&g_ai_lock);

    g_ai_busy = 1;
    rt_sem_release(g_ai_sem);
    return RT_EOK;
}

int ai_chat_is_busy(void)
{
    return g_ai_busy;
}

int ai_chat_take_reply(char *buf, int buf_len)
{
    int ready;

    if (buf == RT_NULL || buf_len <= 0)
    {
        return 0;
    }

    ai_chat_lock_init();
    rt_mutex_take(&g_ai_lock, RT_WAITING_FOREVER);
    ready = g_ai_reply_ready;
    if (ready)
    {
        rt_strncpy(buf, g_ai_reply, buf_len - 1);
        buf[buf_len - 1] = '\0';
        g_ai_reply_ready = 0;
    }
    rt_mutex_release(&g_ai_lock);

    return ready;
}

int ai_chat_set_relay_host(const char *host, int port)
{
    if (host == RT_NULL || host[0] == '\0' || port <= 0 || port > 65535)
    {
        return -RT_EINVAL;
    }

    ai_chat_lock_init();
    rt_mutex_take(&g_ai_lock, RT_WAITING_FOREVER);
    rt_strncpy(g_ai_runtime_relay_host, host, sizeof(g_ai_runtime_relay_host) - 1);
    g_ai_runtime_relay_host[sizeof(g_ai_runtime_relay_host) - 1] = '\0';
    g_ai_runtime_relay_port = port;
    rt_mutex_release(&g_ai_lock);

    LOG_I("AI relay runtime host: %s:%d", g_ai_runtime_relay_host, g_ai_runtime_relay_port);
    return RT_EOK;
}
