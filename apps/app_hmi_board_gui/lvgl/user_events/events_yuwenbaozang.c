#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include "../user_ui_c_files/ui.h"

#define TEST_IN_GUOXUETANGSHI 	0
#define TEST_IN_SANZIJING 		1

#define GUOXUETANGSHI_SRC_PATH	"/assets/yuwen/guoxue/"
#define SANZIJING_SRC_PATH		"/assets/yuwen/sanzijing/"

static int g_yuwenbaozang_mode = TEST_IN_GUOXUETANGSHI;
static int g_guoxuetangshi_index = 1;
static int g_guoxuetangshi_total_cnt = 0;
static int g_sanzijing_index = 1;
static int g_sanzijing_total_cnt = 0;

static const char *g_sd_asset_prefix = "";
static lv_obj_t *g_yuwen_img_area = NULL;

static void fs_set_plain_image(lv_obj_t **area, lv_obj_t *screen, lv_obj_t *img)
{
    if (*area != NULL) {
        lv_obj_add_flag(*area, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_set_parent(img, screen);
    lv_obj_set_width(img, 300);
    lv_obj_set_height(img, 200);
    lv_obj_set_x(img, 0);
    lv_obj_set_y(img, 24);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
}

static void fs_set_scroll_image(lv_obj_t **area, lv_obj_t *screen, lv_obj_t *img)
{
    if (*area == NULL)
    {
        *area = lv_obj_create(screen);
        lv_obj_set_width(*area, 300);
        lv_obj_set_height(*area, 200);
        lv_obj_set_x(*area, 0);
        lv_obj_set_y(*area, 24);
        lv_obj_set_align(*area, LV_ALIGN_CENTER);
        lv_obj_set_scroll_dir(*area, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(*area, LV_SCROLLBAR_MODE_AUTO);
        lv_obj_set_style_bg_opa(*area, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(*area, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(*area, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_parent(img, *area);
        lv_obj_set_align(img, LV_ALIGN_TOP_MID);
        lv_obj_set_x(img, 0);
        lv_obj_set_y(img, 0);
        lv_obj_set_width(img, LV_SIZE_CONTENT);
        lv_obj_set_height(img, LV_SIZE_CONTENT);
    }

    lv_obj_clear_flag(*area, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_parent(img, *area);
    lv_obj_set_align(img, LV_ALIGN_TOP_MID);
    lv_obj_set_x(img, 0);
    lv_obj_set_y(img, 0);
    lv_obj_set_width(img, LV_SIZE_CONTENT);
    lv_obj_set_height(img, LV_SIZE_CONTENT);
    lv_obj_move_foreground(*area);
}

static DIR *fs_open_asset_dir(const char *directory)
{
    DIR *dir;
    static char fallback_path[80];

    dir = opendir(directory);
    if (dir != NULL)
    {
        g_sd_asset_prefix = "";
        return dir;
    }

    if (strncmp(directory, "/assets/", 8) == 0)
    {
        snprintf(fallback_path, sizeof(fallback_path), "/sdcard%s", directory);
        dir = opendir(fallback_path);
        if (dir != NULL)
        {
            g_sd_asset_prefix = "/sdcard";
            return dir;
        }
    }

    return NULL;
}

void fs_make_lvgl_asset_path(char *path, size_t size, const char *directory, int index)
{
    snprintf(path, size, "S:%s%s%d.bin", g_sd_asset_prefix, directory, index);
}

int fs_cnt_bin_files(const char* directory) 
{
    int count = 0;
    DIR* dir;
    struct dirent* entry;

    dir = fs_open_asset_dir(directory);
    if (dir == NULL) {
        rt_kprintf("open dir fail: %s\n", directory);
        return -1;
    }

    // 閬嶅巻鐩綍涓殑鏂囦欢
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".bin") != NULL) {
            count++;
        }
    }

    // 鍏抽棴鐩綍
    closedir(dir);

    return count;
}

static void update_yuwenbaozang_screen(void)
{
    char path[64] = {0};
    int total_cnt = 0;

    if (g_yuwenbaozang_mode == TEST_IN_GUOXUETANGSHI) {
        total_cnt = g_guoxuetangshi_total_cnt;
        fs_make_lvgl_asset_path(path, sizeof(path), GUOXUETANGSHI_SRC_PATH, g_guoxuetangshi_index);
    } else {
        total_cnt = g_sanzijing_total_cnt;
        fs_make_lvgl_asset_path(path, sizeof(path), SANZIJING_SRC_PATH, g_sanzijing_index);
    }

    if (total_cnt <= 0) {
        lv_img_set_src(ui_Imageyuwenbaozang, NULL);
        lv_label_set_text(ui_LabelTitle4, "Chinese: SD assets missing");
        rt_kprintf("missing assets for yuwenbaozang\n");
        return;
    }

    rt_kprintf("src -> %s\n", path);
    lv_label_set_text(ui_LabelTitle4, g_yuwenbaozang_mode == TEST_IN_SANZIJING ? "三字经" : "国学唐诗");
    if (g_yuwenbaozang_mode == TEST_IN_SANZIJING) {
        fs_set_scroll_image(&g_yuwen_img_area, ui_yuwenbaozang, ui_Imageyuwenbaozang);
    } else {
        fs_set_plain_image(&g_yuwen_img_area, ui_yuwenbaozang, ui_Imageyuwenbaozang);
    }
    lv_img_set_src(ui_Imageyuwenbaozang, path);
    if (g_yuwenbaozang_mode == TEST_IN_SANZIJING && g_yuwen_img_area != NULL) {
        lv_obj_scroll_to_y(g_yuwen_img_area, 0, LV_ANIM_OFF);
    }
}
void Btnguoxuetangshi_event_handler(lv_event_t * e)
{
	// Your code here
	lv_obj_add_state( ui_Btnguoxuetangshi, LV_STATE_CHECKED );
	lv_obj_clear_state( ui_Btnsanzijing, LV_STATE_CHECKED );
	g_yuwenbaozang_mode = TEST_IN_GUOXUETANGSHI;
	update_yuwenbaozang_screen();
}

void Btnsanzijing_event_handler(lv_event_t * e)
{
	// Your code here
	lv_obj_clear_state( ui_Btnguoxuetangshi, LV_STATE_CHECKED );
	lv_obj_add_state( ui_Btnsanzijing, LV_STATE_CHECKED );
	g_yuwenbaozang_mode = TEST_IN_SANZIJING;
	update_yuwenbaozang_screen();
}

void Screenyuwenbaozangleft_event_handler(lv_event_t * e)
{
	// Your code here
	if (g_yuwenbaozang_mode == TEST_IN_GUOXUETANGSHI) {
		g_guoxuetangshi_index++;
		if (g_guoxuetangshi_index == (g_guoxuetangshi_total_cnt + 1)) {
			g_guoxuetangshi_index = 1;	
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_guoxuetangshi_index, g_guoxuetangshi_total_cnt);
	} else {
		g_sanzijing_index++;
		if (g_sanzijing_index == (g_sanzijing_total_cnt + 1)) {
			g_sanzijing_index = 1;	
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_sanzijing_index, g_sanzijing_total_cnt);
	}
	update_yuwenbaozang_screen();
}

void Screenyuwenbaozangright_event_handler(lv_event_t * e)
{
	// Your code here
	if (g_yuwenbaozang_mode == TEST_IN_GUOXUETANGSHI) {
		g_guoxuetangshi_index--;
		if (g_guoxuetangshi_index == 0) {
			g_guoxuetangshi_index = g_guoxuetangshi_total_cnt;
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_guoxuetangshi_index, g_guoxuetangshi_total_cnt);
	} else {
		g_sanzijing_index--;
		if (g_sanzijing_index == 0) {
			g_sanzijing_index = g_sanzijing_total_cnt;
		}
		rt_kprintf("%s >>> %d/%d\n", __func__, g_sanzijing_index, g_sanzijing_total_cnt);
	}
	update_yuwenbaozang_screen();
}

void screen_yuwenbaozang_pre_init(lv_event_t * e)
{

	rt_kprintf("%s >>>\n", __func__);

	lv_obj_add_flag(ui_TabViezhinengai1, LV_OBJ_FLAG_HIDDEN);

	g_guoxuetangshi_total_cnt = fs_cnt_bin_files(GUOXUETANGSHI_SRC_PATH);
	g_sanzijing_total_cnt = fs_cnt_bin_files(SANZIJING_SRC_PATH);

	if (g_yuwenbaozang_mode == TEST_IN_GUOXUETANGSHI) {
		lv_obj_add_state( ui_Btnguoxuetangshi, LV_STATE_CHECKED );
		lv_obj_clear_state( ui_Btnsanzijing, LV_STATE_CHECKED );
	} else {
		lv_obj_clear_state( ui_Btnguoxuetangshi, LV_STATE_CHECKED );
		lv_obj_add_state( ui_Btnsanzijing, LV_STATE_CHECKED );
	}

	update_yuwenbaozang_screen();
}
