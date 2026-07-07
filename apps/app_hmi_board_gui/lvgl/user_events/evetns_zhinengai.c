#include <stdio.h>

#include "../user_ui_c_files/ui.h"
#include "app_ai_chat.h"
#include "app_file_ini.h"

static lv_obj_t *g_ai_panel = NULL;
static lv_obj_t *g_ai_input = NULL;
static lv_obj_t *g_ai_output = NULL;
static lv_obj_t *g_ip_panel = NULL;
static lv_obj_t *g_ip_input = NULL;
static lv_obj_t *g_ip_status = NULL;
static lv_obj_t *g_ai_keyboard = NULL;
static lv_timer_t *g_ai_timer = NULL;
static char g_ai_ui_reply[AI_CHAT_REPLY_MAX_LEN];

static void ai_show_chat_panel(void)
{
    if (g_ai_panel != NULL)
    {
        lv_obj_clear_flag(g_ai_panel, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_ip_panel != NULL)
    {
        lv_obj_add_flag(g_ip_panel, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_ai_keyboard != NULL)
    {
        lv_obj_add_flag(g_ai_keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    lv_label_set_text(ui_LabelTitle3, "智能AI");
}

static void ai_show_ip_panel(void)
{
    char host[64] = {0};
    int port = 8080;

    if (g_ai_panel != NULL)
    {
        lv_obj_add_flag(g_ai_panel, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_ip_panel != NULL)
    {
        lv_obj_clear_flag(g_ip_panel, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_ai_keyboard != NULL)
    {
        lv_obj_add_flag(g_ai_keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_ip_input != NULL && ini_load_ai_relay_info(host, sizeof(host), &port) == 0 && host[0] != '\0')
    {
        lv_textarea_set_text(g_ip_input, host);
    }

    lv_label_set_text(ui_LabelTitle3, "输入IP");
}

static void ai_chat_poll_timer_cb(lv_timer_t *timer)
{
    RT_UNUSED(timer);

    if (g_ai_output != NULL && ai_chat_take_reply(g_ai_ui_reply, sizeof(g_ai_ui_reply)))
    {
        lv_label_set_text(g_ai_output, g_ai_ui_reply);
    }
}

static void ai_keyboard_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        lv_obj_add_flag(g_ai_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ai_textarea_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_FOCUSED)
    {
        lv_keyboard_set_textarea(g_ai_keyboard, lv_event_get_target(e));
        lv_obj_clear_flag(g_ai_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ai_send_event_cb(lv_event_t *e)
{
    const char *prompt;
    int ret;

    RT_UNUSED(e);

    prompt = lv_textarea_get_text(g_ai_input);
    if (prompt == NULL || prompt[0] == '\0')
    {
        lv_label_set_text(g_ai_output, "Type a question first.");
        return;
    }

    ret = ai_chat_request_async(prompt);
    if (ret == -RT_EBUSY)
    {
        lv_label_set_text(g_ai_output, "AI is still thinking...");
        return;
    }
    else if (ret != RT_EOK)
    {
        lv_label_set_text(g_ai_output, "AI request start failed.");
        return;
    }

    lv_obj_add_flag(g_ai_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(g_ai_output, "Thinking...");
}

static void ai_save_ip_event_cb(lv_event_t *e)
{
    const char *host;
    int ret;

    RT_UNUSED(e);

    host = lv_textarea_get_text(g_ip_input);
    if (host == NULL || host[0] == '\0')
    {
        lv_label_set_text(g_ip_status, "IP不能为空");
        return;
    }

    ai_chat_set_relay_host(host, 8080);
    ret = ini_save_ai_relay_info(host, 8080);
    if (ret == 0)
    {
        lv_label_set_text(g_ip_status, "已保存，端口为8080");
        rt_kprintf("AI relay host saved: %s:8080\n", host);
    }
    else if (ret == -2)
    {
        lv_label_set_text(g_ip_status, "保存失败：无法创建配置文件");
    }
    else if (ret == -3)
    {
        lv_label_set_text(g_ip_status, "保存失败：写入配置文件失败");
    }
    else if (ret == -4)
    {
        lv_label_set_text(g_ip_status, "保存失败：关闭配置文件失败");
    }
    else
    {
        lv_label_set_text(g_ip_status, "IP已生效，但未保存到TF卡");
    }

    lv_obj_add_flag(g_ai_keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void ai_chat_ui_create(void)
{
    lv_obj_t *send_btn;
    lv_obj_t *send_label;
    lv_obj_t *ip_title;
    lv_obj_t *ip_save_btn;
    lv_obj_t *ip_save_label;
    char saved_host[64] = {0};
    int saved_port = 8080;

    if (g_ai_panel != NULL)
    {
        return;
    }

    lv_obj_add_flag(ui_Imagezhinengai, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_TabViezhinengai, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text(ui_Label13, "输入IP");
    lv_label_set_text(ui_Label14, "AI Chat");

    g_ai_panel = lv_obj_create(ui_zhinengai);
    lv_obj_set_width(g_ai_panel, 444);
    lv_obj_set_height(g_ai_panel, 226);
    lv_obj_set_x(g_ai_panel, 18);
    lv_obj_set_y(g_ai_panel, 38);
    lv_obj_set_align(g_ai_panel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(g_ai_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_ai_panel, lv_color_hex(0xF7F7F7), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_ai_panel, 230, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(g_ai_panel, 6, LV_PART_MAIN | LV_STATE_DEFAULT);

    g_ai_input = lv_textarea_create(g_ai_panel);
    lv_obj_set_width(g_ai_input, 330);
    lv_obj_set_height(g_ai_input, 44);
    lv_obj_set_x(g_ai_input, -42);
    lv_obj_set_y(g_ai_input, -78);
    lv_obj_set_align(g_ai_input, LV_ALIGN_CENTER);
    lv_textarea_set_one_line(g_ai_input, true);
    lv_textarea_set_max_length(g_ai_input, 120);
    lv_textarea_set_placeholder_text(g_ai_input, "Ask AI...");
    lv_textarea_set_text(g_ai_input, "Tell me a short story.");
    lv_obj_add_event_cb(g_ai_input, ai_textarea_event_cb, LV_EVENT_ALL, NULL);

    send_btn = lv_btn_create(g_ai_panel);
    lv_obj_set_width(send_btn, 62);
    lv_obj_set_height(send_btn, 44);
    lv_obj_set_x(send_btn, 160);
    lv_obj_set_y(send_btn, -78);
    lv_obj_set_align(send_btn, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(send_btn, ai_send_event_cb, LV_EVENT_CLICKED, NULL);

    send_label = lv_label_create(send_btn);
    lv_label_set_text(send_label, "Send");
    lv_obj_set_style_text_color(send_label, lv_color_hex(0x20304A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(send_label);

    g_ai_output = lv_label_create(g_ai_panel);
    lv_obj_set_width(g_ai_output, 410);
    lv_obj_set_height(g_ai_output, 126);
    lv_obj_set_x(g_ai_output, 0);
    lv_obj_set_y(g_ai_output, 18);
    lv_obj_set_align(g_ai_output, LV_ALIGN_CENTER);
    lv_label_set_long_mode(g_ai_output, LV_LABEL_LONG_WRAP);
    lv_label_set_text(g_ai_output, "Set WiFi, run PC relay, then tap Send.");
    lv_obj_set_style_text_color(g_ai_output, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);

    g_ip_panel = lv_obj_create(ui_zhinengai);
    lv_obj_set_width(g_ip_panel, 444);
    lv_obj_set_height(g_ip_panel, 226);
    lv_obj_set_x(g_ip_panel, 18);
    lv_obj_set_y(g_ip_panel, 38);
    lv_obj_set_align(g_ip_panel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(g_ip_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_ip_panel, lv_color_hex(0xF7F7F7), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_ip_panel, 230, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(g_ip_panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    ip_title = lv_label_create(g_ip_panel);
    lv_label_set_text(ip_title, "电脑中转IP");
    lv_obj_set_width(ip_title, 410);
    lv_obj_set_x(ip_title, 10);
    lv_obj_set_y(ip_title, 8);
    lv_obj_set_align(ip_title, LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_text_font(ip_title, &ui_font_chnfont, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ip_title, lv_color_hex(0x20304A), LV_PART_MAIN | LV_STATE_DEFAULT);

    g_ip_input = lv_textarea_create(g_ip_panel);
    lv_obj_set_width(g_ip_input, 280);
    lv_obj_set_height(g_ip_input, 44);
    lv_obj_set_x(g_ip_input, 10);
    lv_obj_set_y(g_ip_input, 38);
    lv_obj_set_align(g_ip_input, LV_ALIGN_TOP_LEFT);
    lv_textarea_set_one_line(g_ip_input, true);
    lv_textarea_set_max_length(g_ip_input, 63);
    lv_textarea_set_placeholder_text(g_ip_input, "192.168.18.128");
    lv_textarea_set_text(g_ip_input, "192.168.18.128");
    if (ini_load_ai_relay_info(saved_host, sizeof(saved_host), &saved_port) == 0 && saved_host[0] != '\0')
    {
        lv_textarea_set_text(g_ip_input, saved_host);
        ai_chat_set_relay_host(saved_host, saved_port);
    }
    lv_obj_add_event_cb(g_ip_input, ai_textarea_event_cb, LV_EVENT_ALL, NULL);

    ip_save_btn = lv_btn_create(g_ip_panel);
    lv_obj_set_width(ip_save_btn, 90);
    lv_obj_set_height(ip_save_btn, 44);
    lv_obj_set_x(ip_save_btn, 314);
    lv_obj_set_y(ip_save_btn, 38);
    lv_obj_set_align(ip_save_btn, LV_ALIGN_TOP_LEFT);
    lv_obj_add_event_cb(ip_save_btn, ai_save_ip_event_cb, LV_EVENT_CLICKED, NULL);

    ip_save_label = lv_label_create(ip_save_btn);
    lv_label_set_text(ip_save_label, "保存");
    lv_obj_set_style_text_font(ip_save_label, &ui_font_chnfont, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(ip_save_label);

    g_ip_status = lv_label_create(g_ip_panel);
    lv_obj_set_width(g_ip_status, 410);
    lv_obj_set_height(g_ip_status, 84);
    lv_obj_set_x(g_ip_status, 10);
    lv_obj_set_y(g_ip_status, 100);
    lv_obj_set_align(g_ip_status, LV_ALIGN_TOP_LEFT);
    lv_label_set_long_mode(g_ip_status, LV_LABEL_LONG_WRAP);
    lv_label_set_text(g_ip_status, "请输入电脑IP，然后点击保存。端口固定为8080。");
    lv_obj_set_style_text_font(g_ip_status, &ui_font_chnfont, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_ip_status, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(g_ip_panel, LV_OBJ_FLAG_HIDDEN);

    g_ai_keyboard = lv_keyboard_create(ui_zhinengai);
    lv_obj_set_width(g_ai_keyboard, 444);
    lv_obj_set_height(g_ai_keyboard, 120);
    lv_obj_set_x(g_ai_keyboard, 18);
    lv_obj_set_y(g_ai_keyboard, 76);
    lv_obj_set_align(g_ai_keyboard, LV_ALIGN_CENTER);
    lv_keyboard_set_textarea(g_ai_keyboard, g_ai_input);
    lv_obj_add_event_cb(g_ai_keyboard, ai_keyboard_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(g_ai_keyboard, LV_OBJ_FLAG_HIDDEN);

    g_ai_timer = lv_timer_create(ai_chat_poll_timer_cb, 300, NULL);
    ai_show_chat_panel();
}

void Btntonghuagushi_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);

    lv_obj_add_state(ui_Btntonghuagushi, LV_STATE_CHECKED);
    lv_obj_clear_state(ui_Btnzhishibaike, LV_STATE_CHECKED);
    ai_show_ip_panel();
}

void Btnzhishibaike_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);

    lv_obj_clear_state(ui_Btntonghuagushi, LV_STATE_CHECKED);
    lv_obj_add_state(ui_Btnzhishibaike, LV_STATE_CHECKED);
    ai_show_chat_panel();
    lv_label_set_text(g_ai_output, "AI chat ready.");
}

void Screenzhinengaileft_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
}

void Screenzhinengairight_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
}

void screen_zhinengai_pre_init(lv_event_t *e)
{
    RT_UNUSED(e);

    rt_kprintf("%s >>>\n", __func__);

    ai_chat_ui_create();
    lv_obj_clear_state(ui_Btntonghuagushi, LV_STATE_CHECKED);
    lv_obj_add_state(ui_Btnzhishibaike, LV_STATE_CHECKED);
    ai_show_chat_panel();
}
