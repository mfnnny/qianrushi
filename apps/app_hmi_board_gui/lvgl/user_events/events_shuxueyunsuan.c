
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../user_ui_c_files/ui.h"
#include "rtthread.h"

#define MATH_MODE_NUMBER 0
#define MATH_MODE_SHAPE  1
#define MATH_MAX_NUM     100
#define TUXING_SRC_PATH  "/assets/shuxue/tuxing/"

extern int fs_cnt_bin_files(const char *directory);
extern void fs_make_lvgl_asset_path(char *path, size_t size, const char *directory, int index);

static int g_math_mode = MATH_MODE_NUMBER;
static int g_random_num1;
static int g_random_num2;
static int g_answer;
static char g_operator = '+';
static int g_has_question = 0;
static int g_tuxing_index = 1;
static int g_tuxing_total_cnt = 0;

static void set_number_widgets_hidden(int hidden)
{
    if (hidden) {
        lv_obj_add_flag(ui_TabViezhinengai2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Label27, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Label28, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_TextareaMathAnswer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Btnchongxinchuti, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Btnchakandaan, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_KeyboardMathAnswer, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui_TabViezhinengai2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Label27, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Label28, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_TextareaMathAnswer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Btnchongxinchuti, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Btnchakandaan, LV_OBJ_FLAG_HIDDEN);
    }
}

static void show_number_mode(void)
{
    g_math_mode = MATH_MODE_NUMBER;
    lv_label_set_text(ui_LabelTitle5, "数字运算");
    lv_label_set_text(ui_Label27, "固定100以内加减法");
    lv_label_set_text(ui_Label28, "点击重新出题开始");
    lv_textarea_set_text(ui_TextareaMathAnswer, "");
    lv_obj_add_flag(ui_Imageshuxueyunsuan, LV_OBJ_FLAG_HIDDEN);
    set_number_widgets_hidden(0);
    lv_obj_add_state(ui_Btnjiafayunsuan, LV_STATE_CHECKED);
    lv_obj_clear_state(ui_Btnjianfayunsuan, LV_STATE_CHECKED);
}

static void show_shape_mode(void)
{
    char path[64] = {0};

    g_math_mode = MATH_MODE_SHAPE;
    lv_label_set_text(ui_LabelTitle5, "认识图形");
    set_number_widgets_hidden(1);
    lv_obj_add_state(ui_Btnjianfayunsuan, LV_STATE_CHECKED);
    lv_obj_clear_state(ui_Btnjiafayunsuan, LV_STATE_CHECKED);

    g_tuxing_total_cnt = fs_cnt_bin_files(TUXING_SRC_PATH);
    if (g_tuxing_total_cnt <= 0) {
        lv_obj_clear_flag(ui_Label28, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_Label28, "请上传图形图片到 /assets/shuxue/tuxing/");
        lv_obj_add_flag(ui_Imageshuxueyunsuan, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if (g_tuxing_index > g_tuxing_total_cnt) {
        g_tuxing_index = 1;
    }

    fs_make_lvgl_asset_path(path, sizeof(path), TUXING_SRC_PATH, g_tuxing_index);
    lv_img_set_src(ui_Imageshuxueyunsuan, path);
    lv_obj_clear_flag(ui_Imageshuxueyunsuan, LV_OBJ_FLAG_HIDDEN);
}

void Btnjiafayunsuan_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
    show_number_mode();
}

void Btnjianfayunsuan_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
    show_shape_mode();
}

void MathShapeNext_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);

    if (g_math_mode != MATH_MODE_SHAPE || g_tuxing_total_cnt <= 1) {
        return;
    }

    g_tuxing_index++;
    if (g_tuxing_index > g_tuxing_total_cnt) {
        g_tuxing_index = 1;
    }

    show_shape_mode();
}

void Btnchongxinchuti_event_handler(lv_event_t *e)
{
    char txt[64] = {0};

    RT_UNUSED(e);

    if (g_math_mode != MATH_MODE_NUMBER) {
        show_number_mode();
    }

    srand((unsigned)time(NULL) + rt_tick_get());
    if ((rand() % 2) == 0) {
        g_operator = '+';
        g_random_num1 = rand() % (MATH_MAX_NUM + 1);
        g_random_num2 = rand() % (MATH_MAX_NUM - g_random_num1 + 1);
        g_answer = g_random_num1 + g_random_num2;
    } else {
        g_operator = '-';
        g_random_num1 = rand() % (MATH_MAX_NUM + 1);
        g_random_num2 = rand() % (g_random_num1 + 1);
        g_answer = g_random_num1 - g_random_num2;
    }

    g_has_question = 1;
    lv_textarea_set_text(ui_TextareaMathAnswer, "");
    snprintf(txt, sizeof(txt), "请计算: %d %c %d = ?", g_random_num1, g_operator, g_random_num2);
    lv_label_set_text(ui_Label28, txt);
    lv_obj_clear_flag(ui_TextareaMathAnswer, LV_OBJ_FLAG_HIDDEN);
}

void Btnchakandaan_event_handler(lv_event_t *e)
{
    const char *answer_text;
    int user_answer;

    RT_UNUSED(e);

    if (!g_has_question) {
        lv_label_set_text(ui_Label28, "请先点击重新出题");
        return;
    }

    answer_text = lv_textarea_get_text(ui_TextareaMathAnswer);
    if (answer_text == NULL || answer_text[0] == '\0') {
        lv_label_set_text(ui_Label28, "请先输入答案");
        return;
    }

    user_answer = atoi(answer_text);
    if (user_answer == g_answer) {
        lv_label_set_text(ui_Label28, "结果正确");
    } else {
        lv_label_set_text(ui_Label28, "结果错误");
    }

    lv_obj_add_flag(ui_KeyboardMathAnswer, LV_OBJ_FLAG_HIDDEN);
}

void Dropdownmaxnum_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
}

void Btnfanhuizhuye4_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
}

void screen_shuxueyunsuan_pre_init(lv_event_t *e)
{
    RT_UNUSED(e);

    rt_kprintf(" %s >>>\n", __func__);

    if (g_math_mode == MATH_MODE_SHAPE) {
        show_shape_mode();
    } else {
        show_number_mode();
    }
}
