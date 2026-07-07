#ifndef __APP_AI_CHAT_H__
#define __APP_AI_CHAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AI_CHAT_REPLY_MAX_LEN       768

int ai_chat_request_async(const char *prompt);
int ai_chat_is_busy(void);
int ai_chat_take_reply(char *buf, int buf_len);
int ai_chat_set_relay_host(const char *host, int port);

#ifdef __cplusplus
}
#endif

#endif
