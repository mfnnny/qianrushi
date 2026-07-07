#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../user_ui_c_files/ui.h"

extern int fs_cnt_bin_files(const char* directory);
extern void fs_make_lvgl_asset_path(char *path, size_t size, const char *directory, int index);

#define TEST_IN_RENSHIZIMU 		0
#define TEST_IN_QUWEIDANCI 		1

#define RENSHIZIMU_SRC_PATH		"/assets/yingyu/zimu/"
#define QUWEIDANCI_SRC_PATH		"/assets/yingyu/danci/"

static int g_yingyuqimeng_mode = TEST_IN_RENSHIZIMU;
static int g_renshizimu_index = 1;
static int g_renshizimu_total_cnt = 0;
static int g_quweidanci_index = 1;
static int g_quweidanci_total_cnt = 0;

static void update_yingyuqimeng_screen(void)
{
    char path[64] = {0};
    int total_cnt = 0;

    if (g_yingyuqimeng_mode == TEST_IN_RENSHIZIMU) {
        total_cnt = g_renshizimu_total_cnt;
        if (total_cnt > 0) {
            srand((unsigned)time(NULL));
            g_renshizimu_index = rand() % total_cnt + 1;
        }
        fs_make_lvgl_asset_path(path, sizeof(path), RENSHIZIMU_SRC_PATH, g_renshizimu_index);
    } else {
        total_cnt = g_quweidanci_total_cnt;
        fs_make_lvgl_asset_path(path, sizeof(path), QUWEIDANCI_SRC_PATH, g_quweidanci_index);
    }

    if (total_cnt <= 0) {
        lv_img_set_src(ui_Imageyingyuqimeng, NULL);
        lv_label_set_text(ui_LabelTitle6, "English: SD assets missing");
        rt_kprintf("missing assets for yingyuqimeng\n");
        return;
    }

    rt_kprintf("src -> %s\n", path);
    lv_label_set_text(ui_LabelTitle6, g_yingyuqimeng_mode == TEST_IN_RENSHIZIMU ? "认识字母" : "趣味单词");
    lv_img_set_src(ui_Imageyingyuqimeng, path);
}
void Btnrenshizimu_event_handler(lv_event_t * e)
{
	// Your code here
	lv_obj_add_state( ui_Btnrenshizimu, LV_STATE_CHECKED );
	lv_obj_clear_state( ui_Btnquweidanci, LV_STATE_CHECKED );
	g_yingyuqimeng_mode = TEST_IN_RENSHIZIMU;
	update_yingyuqimeng_screen();
}

void Btnquweidanci_event_handler(lv_event_t * e)
{
	// Your code here
	lv_obj_clear_state( ui_Btnrenshizimu, LV_STATE_CHECKED );
	lv_obj_add_state( ui_Btnquweidanci, LV_STATE_CHECKED );
	g_yingyuqimeng_mode = TEST_IN_QUWEIDANCI;
	update_yingyuqimeng_screen();
}

void Screenyingyuqimengleft_event_handler(lv_event_t * e)
{
	// Your code here
	if (g_yingyuqimeng_mode == TEST_IN_RENSHIZIMU) {
		g_renshizimu_index++;
		if (g_renshizimu_index == (g_renshizimu_total_cnt + 1)) {
			g_renshizimu_index = 1;	
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_renshizimu_index, g_renshizimu_total_cnt);
	} else {
		g_quweidanci_index++;
		if (g_quweidanci_index == (g_quweidanci_total_cnt + 1)) {
			g_quweidanci_index = 1;	
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_quweidanci_index, g_quweidanci_total_cnt);
	}
	update_yingyuqimeng_screen();
}

void Screenyingyuqimengright_event_handler(lv_event_t * e)
{
	// Your code here
	if (g_yingyuqimeng_mode == TEST_IN_RENSHIZIMU) {
		g_renshizimu_index--;
		if (g_renshizimu_index == 0) {
			g_renshizimu_index = g_renshizimu_total_cnt;
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_renshizimu_index, g_renshizimu_total_cnt);
	} else {
		g_quweidanci_index--;
		if (g_quweidanci_index == 0) {
			g_quweidanci_index = g_quweidanci_total_cnt;
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_quweidanci_index, g_quweidanci_total_cnt);
	}
	update_yingyuqimeng_screen();
}

void screen_yingyuqimeng_pre_init(lv_event_t * e)
{

	rt_kprintf("%s >>>\n", __func__);

	lv_obj_add_flag(ui_TabViezhinengai3, LV_OBJ_FLAG_HIDDEN);

	g_renshizimu_total_cnt = fs_cnt_bin_files(RENSHIZIMU_SRC_PATH);
	g_quweidanci_total_cnt = fs_cnt_bin_files(QUWEIDANCI_SRC_PATH);

	if (g_yingyuqimeng_mode == TEST_IN_RENSHIZIMU) {
		lv_obj_add_state( ui_Btnrenshizimu, LV_STATE_CHECKED );
		lv_obj_clear_state( ui_Btnquweidanci, LV_STATE_CHECKED );
	} else {
		lv_obj_clear_state( ui_Btnrenshizimu, LV_STATE_CHECKED );
		lv_obj_add_state( ui_Btnquweidanci, LV_STATE_CHECKED );
	}

	update_yingyuqimeng_screen();
}
