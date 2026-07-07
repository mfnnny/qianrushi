
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "rtthread.h"
#include "minIni.h"

#define sizearray(a)        (sizeof(a) / sizeof((a)[0]))

#define DATA_INI_FILE       "data.ini"
#define DATA_SECTION_NAME   "data"
#define DATA_SSID_NAME      "ssid"
#define DATA_PASSWD_NAME    "password"
#define DATA_MAX_NUM        "max_num"
#define DATA_TEST_MODE      "test_mode"
#define AI_RELAY_TXT_FILE   "/ai_relay.txt"
#define AI_RELAY_INI_FILE_PRIMARY DATA_INI_FILE
#define AI_RELAY_INI_FILE   "/ai_relay.ini"
#define AI_RELAY_INI_FILE_RELATIVE "ai_relay.ini"
#define AI_RELAY_INI_FILE_FALLBACK "/sdcard/ai_relay.ini"
#define AI_RELAY_SECTION    "relay"
#define AI_RELAY_HOST       "host"
#define AI_RELAY_PORT       "port"

void ini_test(void)
{
    char str[100] = {0};
    long n;
    
    {
        int fd = 0;
        int ret;
       /* 
        fd = open(DATA_INI_FILE, O_RDONLY);
        rt_kprintf("fd: %d\n", fd);
        
        ret = lseek(fd, 5, SEEK_SET);
        rt_kprintf("ret: %d\n", ret);
        
        ret = lseek(fd, 0, SEEK_CUR);
        rt_kprintf("ret: %d\n", ret);
        
        ret = lseek(fd, -5, SEEK_END);
        rt_kprintf("ret: %d\n", ret);
        
        ret = lseek(fd, 0, SEEK_CUR);
        rt_kprintf("ret: %d\n", ret);
        
        close(fd);*/
    }

    n = ini_puts("data", "string", "correct", DATA_INI_FILE);
    rt_kprintf("n: %d, str: %s\n", n, str);
    n = ini_puts("data", "string1", "correct1", DATA_INI_FILE);
    rt_kprintf("n: %d, str: %s\n", n, str);
    n = ini_puts("data", "string2", "correct2", DATA_INI_FILE);
    rt_kprintf("n: %d, str: %s\n", n, str);

    n = ini_gets("data", "string", "", str, sizearray(str), DATA_INI_FILE);
    rt_kprintf("n: %d, str: %s\n", n, str);
    n = ini_gets("data", "string1", "", str, sizearray(str), DATA_INI_FILE);
    rt_kprintf("n: %d, str: %s\n", n, str);
    n = ini_gets("data", "string2", "", str, sizearray(str), DATA_INI_FILE);
    rt_kprintf("n: %d, str: %s\n", n, str);
}

#include <stdio.h>

void fopen_test(void)
{
    FILE *fp=NULL;
    if((fp=fopen("flog","r"))==NULL) {
        perror("fopen error\n");
    } else {
        char buf[]="hello";
        fwrite(buf,5,1,fp);
        fclose(fp);
    }
}

int ini_load_wifi_ap_info(char *ssid, char *password)
{
    char str[100] = {0};
    int n;
    int ret = -1;

    n = ini_gets(DATA_SECTION_NAME, DATA_SSID_NAME, "", str, sizearray(str), DATA_INI_FILE);
    if (n && ssid) {
        strcpy(ssid, str);
        ret = 0;
    } else {
        ret = -1;
    }

    n = ini_gets(DATA_SECTION_NAME, DATA_PASSWD_NAME, "", str, sizearray(str), DATA_INI_FILE);
    if (n && password) {
        strcpy(password, str);
        ret = 0;
    } else {
        ret = -1;
    }

    return ret;
}

int ini_save_wifi_ap_info(char *ssid, char *password)
{
    int n;
    int ret = -1;

    n = ini_puts(DATA_SECTION_NAME, DATA_SSID_NAME, ssid, DATA_INI_FILE);
    if (n == 1) {
        ret = 0;
    } else {
        ret = -1;
    }

    n = ini_puts(DATA_SECTION_NAME, DATA_PASSWD_NAME, password, DATA_INI_FILE);
    if (n == 1) {
        ret = 0;
    } else {
        ret = -1;
    }

    return ret;
}

int ini_load_shuxueyunsuan_info(int *max_num, int *test_mode)
{
    int ret = -1;
    long value;

    if (max_num) {
        value = ini_getl(DATA_SECTION_NAME, DATA_MAX_NUM, -1, DATA_INI_FILE);
        if (value != -1) {
            *max_num = value;
            ret = 0;
        } else {
            ret = -1;
        }
    }

    if (test_mode) {
        value = ini_getl(DATA_SECTION_NAME, DATA_TEST_MODE, -1, DATA_INI_FILE);
        if (value != -1) {
            *test_mode = value;
            ret = 0;
        } else {
            ret = -1;
        }
    }

    return ret;
}

int ini_save_shuxueyunsuan_info(int *max_num, int *test_mode)
{
    int n;
    int ret = -1;
    long value;

    if (max_num) {
        value = *max_num;
        value = ini_putl(DATA_SECTION_NAME, DATA_MAX_NUM, value, DATA_INI_FILE);
        if (n == 1) {
            ret = 0;
        } else {
            ret = -1;
        }
    }

    if (test_mode) {
        value = *test_mode;
        n = ini_putl(DATA_SECTION_NAME, DATA_TEST_MODE, value, DATA_INI_FILE);
        if (n == 1) {
            ret = 0;
        } else {
            ret = -1;
        }
    }

    return ret;
}

int ini_load_ai_relay_info(char *host, int host_len, int *port)
{
    char str[100] = {0};
    char txt[128] = {0};
    const char *filenames[] = {
        AI_RELAY_INI_FILE_PRIMARY,
        AI_RELAY_INI_FILE,
        AI_RELAY_INI_FILE_RELATIVE,
        AI_RELAY_INI_FILE_FALLBACK,
    };
    const char *filename = AI_RELAY_INI_FILE;
    long value;
    int ret = -1;
    int fd;
    int len;

    if (host && host_len > 0)
    {
        fd = open(AI_RELAY_TXT_FILE, O_RDONLY);
        if (fd >= 0)
        {
            len = read(fd, txt, sizeof(txt) - 1);
            close(fd);
            if (len > 0)
            {
                char *line_end;
                txt[len] = '\0';
                line_end = strchr(txt, '\n');
                if (line_end != RT_NULL)
                {
                    *line_end = '\0';
                    if (port)
                    {
                        char *port_str = line_end + 1;
                        *port = atoi(port_str);
                        if (*port <= 0 || *port > 65535)
                        {
                            *port = 8080;
                        }
                    }
                }
                rt_strncpy(host, txt, host_len - 1);
                host[host_len - 1] = '\0';
                if (host[0] != '\0')
                {
                    rt_kprintf("load ai relay from %s: %s:%d\n", AI_RELAY_TXT_FILE, host, port ? *port : 8080);
                    return 0;
                }
            }
        }
        else
        {
            rt_kprintf("open %s for load failed\n", AI_RELAY_TXT_FILE);
        }
    }

    if (host && host_len > 0)
    {
        int n = 0;
        for (int i = 0; i < (int)sizearray(filenames); i++)
        {
            filename = filenames[i];
            n = ini_gets(AI_RELAY_SECTION, AI_RELAY_HOST, "", str, sizearray(str), filename);
            if (n > 0)
            {
                break;
            }
        }
        if (n > 0)
        {
            rt_strncpy(host, str, host_len - 1);
            host[host_len - 1] = '\0';
            ret = 0;
            rt_kprintf("load ai relay from %s: %s\n", filename, host);
        }
    }

    if (port)
    {
        value = ini_getl(AI_RELAY_SECTION, AI_RELAY_PORT, -1, filename);
        if (value > 0 && value <= 65535)
        {
            *port = (int)value;
            ret = 0;
        }
    }

    return ret;
}

int ini_save_ai_relay_info(const char *host, int port)
{
    int fd;
    int len;
    int written;
    int n;
    char buffer[128];
    const char *filenames[] = {
        AI_RELAY_INI_FILE,
        AI_RELAY_INI_FILE_RELATIVE,
        AI_RELAY_INI_FILE_FALLBACK,
    };
    const char *saved_filename = RT_NULL;

    if (host == RT_NULL || host[0] == '\0' || port <= 0 || port > 65535)
    {
        return -1;
    }

    rt_snprintf(buffer, sizeof(buffer), "%s\n%d\n", host, port);
    len = strlen(buffer);
    fd = open(AI_RELAY_TXT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0)
    {
        written = write(fd, buffer, len);
        close(fd);
        if (written == len)
        {
            rt_kprintf("saved %s: %s:%d\n", AI_RELAY_TXT_FILE, host, port);
            return 0;
        }
        rt_kprintf("write %s failed: %d/%d\n", AI_RELAY_TXT_FILE, written, len);
    }
    else
    {
        rt_kprintf("open %s for save failed\n", AI_RELAY_TXT_FILE);
    }

    n = ini_puts(AI_RELAY_SECTION, AI_RELAY_HOST, host, AI_RELAY_INI_FILE_PRIMARY);
    if (n == 1)
    {
        n = ini_putl(AI_RELAY_SECTION, AI_RELAY_PORT, port, AI_RELAY_INI_FILE_PRIMARY);
        if (n == 1)
        {
            rt_kprintf("saved %s: [%s] %s=%s %s=%d\n",
                       AI_RELAY_INI_FILE_PRIMARY, AI_RELAY_SECTION, AI_RELAY_HOST, host, AI_RELAY_PORT, port);
            return 0;
        }
    }

    rt_kprintf("save %s failed, try fallback files\n", AI_RELAY_INI_FILE_PRIMARY);

    rt_snprintf(buffer, sizeof(buffer), "[%s]\n%s=%s\n%s=%d\n", AI_RELAY_SECTION, AI_RELAY_HOST, host, AI_RELAY_PORT, port);
    len = strlen(buffer);

    for (int i = 0; i < (int)sizearray(filenames); i++)
    {
        fd = open(filenames[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            rt_kprintf("open %s failed\n", filenames[i]);
            continue;
        }

        written = write(fd, buffer, len);
        if (written != len)
        {
            rt_kprintf("write %s failed: %d/%d\n", filenames[i], written, len);
            close(fd);
            continue;
        }

        if (close(fd) != 0)
        {
            rt_kprintf("close %s failed\n", filenames[i]);
            continue;
        }

        saved_filename = filenames[i];
        break;
    }

    if (saved_filename == RT_NULL)
    {
        return -2;
    }

    rt_kprintf("saved %s: %s", saved_filename, buffer);
    return 0;
}

static void ai_relay_cmd(int argc, char **argv)
{
    char host[64] = {0};
    int port = 8080;
    int ret;

    if (argc == 2 && strcmp(argv[1], "load") == 0)
    {
        ret = ini_load_ai_relay_info(host, sizeof(host), &port);
        rt_kprintf("ai relay load ret=%d host=%s port=%d\n", ret, host, port);
    }
    else if (argc >= 3 && strcmp(argv[1], "save") == 0)
    {
        if (argc >= 4)
        {
            port = atoi(argv[3]);
        }
        ret = ini_save_ai_relay_info(argv[2], port);
        rt_kprintf("ai relay save ret=%d host=%s port=%d\n", ret, argv[2], port);
    }
    else
    {
        rt_kprintf("usage:\n");
        rt_kprintf("  ai_relay_cmd load\n");
        rt_kprintf("  ai_relay_cmd save <host> [port]\n");
    }
}
MSH_CMD_EXPORT(ai_relay_cmd, ai relay config command);
