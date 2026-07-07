#ifndef UI_KID_STYLE_H
#define UI_KID_STYLE_H

#include "ui.h"

#define KID_FONT_ZH (&ui_font_chnfont)

static inline void kid_screen_style(lv_obj_t *screen)
{
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFDF4), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static inline void kid_card_style(lv_obj_t *obj, uint32_t color, uint32_t border)
{
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, border ? 2 : 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(obj, lv_color_hex(border ? border : color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0xDDE6F0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(obj, 90, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static inline void kid_btn_style(lv_obj_t *btn, uint32_t color)
{
    kid_card_style(btn, color, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(btn, KID_FONT_ZH, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn, lv_color_hex(0x20304A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFF2A0), LV_PART_MAIN | LV_STATE_CHECKED);
}

static inline lv_obj_t *kid_label(lv_obj_t *parent, const char *text, lv_coord_t x, lv_coord_t y,
                                  lv_coord_t w, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_width(label, w);
    lv_obj_set_height(label, LV_SIZE_CONTENT);
    lv_obj_set_x(label, x);
    lv_obj_set_y(label, y);
    lv_label_set_text(label, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, KID_FONT_ZH, LV_PART_MAIN | LV_STATE_DEFAULT);
    return label;
}

static inline void kid_page_title(lv_obj_t *parent, lv_obj_t **title, const char *text)
{
    *title = kid_label(parent, text, 0, 10, 480, 0x20304A);
    lv_obj_set_align(*title, LV_ALIGN_TOP_MID);
    lv_obj_set_style_text_align(*title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static inline void kid_menu_btn(lv_obj_t *btn, lv_obj_t **label, const char *text,
                                lv_coord_t x, lv_coord_t y, uint32_t color)
{
    lv_obj_set_width(btn, 100);
    lv_obj_set_height(btn, 54);
    lv_obj_set_x(btn, x);
    lv_obj_set_y(btn, y);
    lv_obj_set_align(btn, LV_ALIGN_TOP_LEFT);
    kid_btn_style(btn, color);
    *label = kid_label(btn, text, 8, 13, 84, 0x20304A);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static inline void kid_list_btn(lv_obj_t *btn, lv_obj_t **label, const char *text,
                                lv_coord_t x, lv_coord_t y, lv_coord_t w)
{
    lv_obj_set_width(btn, w);
    lv_obj_set_height(btn, 42);
    lv_obj_set_x(btn, x);
    lv_obj_set_y(btn, y);
    lv_obj_set_align(btn, LV_ALIGN_TOP_LEFT);
    kid_btn_style(btn, 0xFFFFFF);
    *label = kid_label(btn, text, 10, 10, w - 20, 0x20304A);
}

static inline void kid_small_back_btn(lv_obj_t *btn, lv_obj_t **label)
{
    lv_obj_set_width(btn, 96);
    lv_obj_set_height(btn, 32);
    lv_obj_set_x(btn, 88);
    lv_obj_set_y(btn, 8);
    lv_obj_set_align(btn, LV_ALIGN_TOP_LEFT);
    kid_btn_style(btn, 0xFFFFFF);
    lv_obj_set_style_shadow_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    *label = kid_label(btn, "返回主页", 4, 8, 88, 0x20304A);
    lv_obj_set_style_text_align(*label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static inline void kid_content_image_style(lv_obj_t *img)
{
    lv_obj_set_width(img, 300);
    lv_obj_set_height(img, 200);
    lv_obj_set_x(img, 0);
    lv_obj_set_y(img, 24);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
}

#endif
