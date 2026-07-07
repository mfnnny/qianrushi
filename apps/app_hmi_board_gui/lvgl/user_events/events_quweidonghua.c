#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../user_ui_c_files/ui.h"

#define GAME_SNAKE      0
#define GAME_2048       1

#define SNAKE_COLS      15
#define SNAKE_ROWS      13
#define SNAKE_MAX_LEN   (SNAKE_COLS * SNAKE_ROWS)

typedef struct
{
    int x;
    int y;
} point_t;

static int g_game_mode = GAME_SNAKE;
static lv_obj_t *g_game_panel = RT_NULL;
static lv_obj_t *g_game_title = RT_NULL;
static lv_obj_t *g_game_status = RT_NULL;
static lv_obj_t *g_snake_cells[SNAKE_ROWS][SNAKE_COLS];
static lv_obj_t *g_2048_cells[4][4];
static lv_obj_t *g_2048_labels[4][4];
static lv_obj_t *g_btn_up = RT_NULL;
static lv_obj_t *g_btn_down = RT_NULL;
static lv_obj_t *g_btn_left = RT_NULL;
static lv_obj_t *g_btn_right = RT_NULL;
static lv_obj_t *g_btn_restart = RT_NULL;
static lv_timer_t *g_snake_timer = RT_NULL;

static point_t g_snake[SNAKE_MAX_LEN];
static int g_snake_len = 0;
static point_t g_food;
static int g_snake_dir = 1;
static int g_snake_next_dir = 1;
static int g_snake_score = 0;
static int g_snake_over = 0;

static int g_2048_board[4][4];
static int g_2048_score = 0;
static int g_2048_over = 0;

static void snake_timer_cb(lv_timer_t *timer);

static void set_obj_hidden(lv_obj_t *obj, int hidden)
{
    if (obj == RT_NULL)
    {
        return;
    }
    if (hidden)
    {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static void snake_timer_start(void)
{
    if (g_snake_timer == RT_NULL)
    {
        g_snake_timer = lv_timer_create(snake_timer_cb, 360, RT_NULL);
    }
    else
    {
        lv_timer_resume(g_snake_timer);
    }
}

static void snake_timer_stop(void)
{
    if (g_snake_timer != RT_NULL)
    {
        lv_timer_pause(g_snake_timer);
    }
}

static lv_obj_t *make_game_btn(lv_obj_t *parent, const char *text, int x, int y, int w, int h)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_t *label;

    lv_obj_set_width(btn, w);
    lv_obj_set_height(btn, h);
    lv_obj_set_x(btn, x);
    lv_obj_set_y(btn, y);
    lv_obj_set_align(btn, LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xEAF4FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &ui_font_chnfont, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x20304A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    return btn;
}

static void snake_place_food(void)
{
    int tries = 0;

    while (tries < 200)
    {
        int hit = 0;
        int x = rand() % SNAKE_COLS;
        int y = rand() % SNAKE_ROWS;

        for (int i = 0; i < g_snake_len; i++)
        {
            if (g_snake[i].x == x && g_snake[i].y == y)
            {
                hit = 1;
                break;
            }
        }

        if (!hit)
        {
            g_food.x = x;
            g_food.y = y;
            return;
        }
        tries++;
    }

    g_food.x = 0;
    g_food.y = 0;
}

static void snake_draw(void)
{
    char buf[48];

    for (int y = 0; y < SNAKE_ROWS; y++)
    {
        for (int x = 0; x < SNAKE_COLS; x++)
        {
            uint32_t color = 0xF4F8F4;

            if (g_food.x == x && g_food.y == y)
            {
                color = 0xFF6B7A;
            }

            for (int i = 0; i < g_snake_len; i++)
            {
                if (g_snake[i].x == x && g_snake[i].y == y)
                {
                    color = (i == 0) ? 0x1EA65B : 0x60D98A;
                    break;
                }
            }

            lv_obj_set_style_bg_color(g_snake_cells[y][x], lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    rt_snprintf(buf, sizeof(buf), g_snake_over ? "得分:%d  游戏结束" : "得分:%d", g_snake_score);
    lv_label_set_text(g_game_status, buf);
}

static void snake_new_game(void)
{
    g_snake_len = 4;
    g_snake[0].x = 7;
    g_snake[0].y = 6;
    g_snake[1].x = 6;
    g_snake[1].y = 6;
    g_snake[2].x = 5;
    g_snake[2].y = 6;
    g_snake[3].x = 4;
    g_snake[3].y = 6;
    g_snake_dir = 1;
    g_snake_next_dir = 1;
    g_snake_score = 0;
    g_snake_over = 0;
    snake_place_food();
    snake_draw();
}

static void snake_step(void)
{
    point_t head;
    int grow = 0;

    if (g_game_mode != GAME_SNAKE || g_snake_over)
    {
        return;
    }

    if ((g_snake_dir == 0 && g_snake_next_dir != 2) ||
        (g_snake_dir == 1 && g_snake_next_dir != 3) ||
        (g_snake_dir == 2 && g_snake_next_dir != 0) ||
        (g_snake_dir == 3 && g_snake_next_dir != 1))
    {
        g_snake_dir = g_snake_next_dir;
    }

    head = g_snake[0];
    if (g_snake_dir == 0) head.y--;
    if (g_snake_dir == 1) head.x++;
    if (g_snake_dir == 2) head.y++;
    if (g_snake_dir == 3) head.x--;

    if (head.x < 0 || head.x >= SNAKE_COLS || head.y < 0 || head.y >= SNAKE_ROWS)
    {
        g_snake_over = 1;
        snake_draw();
        return;
    }

    for (int i = 0; i < g_snake_len; i++)
    {
        if (g_snake[i].x == head.x && g_snake[i].y == head.y)
        {
            g_snake_over = 1;
            snake_draw();
            return;
        }
    }

    if (head.x == g_food.x && head.y == g_food.y)
    {
        grow = 1;
        g_snake_score += 10;
    }

    if (grow && g_snake_len < SNAKE_MAX_LEN)
    {
        g_snake_len++;
    }

    for (int i = g_snake_len - 1; i > 0; i--)
    {
        g_snake[i] = g_snake[i - 1];
    }
    g_snake[0] = head;

    if (grow)
    {
        snake_place_food();
    }

    snake_draw();
}

static void snake_timer_cb(lv_timer_t *timer)
{
    RT_UNUSED(timer);
    if (lv_scr_act() != ui_quweidonghua)
    {
        snake_timer_stop();
        return;
    }
    snake_step();
}

static int board_has_move(void)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (g_2048_board[y][x] == 0)
            {
                return 1;
            }
            if (x < 3 && g_2048_board[y][x] == g_2048_board[y][x + 1])
            {
                return 1;
            }
            if (y < 3 && g_2048_board[y][x] == g_2048_board[y + 1][x])
            {
                return 1;
            }
        }
    }
    return 0;
}

static void board_add_tile(void)
{
    int empty[16][2];
    int cnt = 0;

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (g_2048_board[y][x] == 0)
            {
                empty[cnt][0] = x;
                empty[cnt][1] = y;
                cnt++;
            }
        }
    }

    if (cnt > 0)
    {
        int pick = rand() % cnt;
        g_2048_board[empty[pick][1]][empty[pick][0]] = (rand() % 10 == 0) ? 4 : 2;
    }
}

static uint32_t tile_color(int value)
{
    switch (value)
    {
    case 0: return 0xF1F4F8;
    case 2: return 0xFFF2C1;
    case 4: return 0xFFE09C;
    case 8: return 0xFFC185;
    case 16: return 0xFF9D7E;
    case 32: return 0xFF7F8A;
    case 64: return 0xF66578;
    case 128: return 0xBDE986;
    case 256: return 0x8EE78D;
    case 512: return 0x65D6A6;
    case 1024: return 0x66C5F2;
    default: return 0x9BB8FF;
    }
}

static void board_draw(void)
{
    char buf[64];

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            int value = g_2048_board[y][x];
            lv_obj_set_style_bg_color(g_2048_cells[y][x], lv_color_hex(tile_color(value)), LV_PART_MAIN | LV_STATE_DEFAULT);
            if (value == 0)
            {
                lv_label_set_text(g_2048_labels[y][x], "");
            }
            else
            {
                lv_label_set_text_fmt(g_2048_labels[y][x], "%d", value);
            }
        }
    }

    rt_snprintf(buf, sizeof(buf), g_2048_over ? "得分:%d  游戏结束" : "得分:%d", g_2048_score);
    lv_label_set_text(g_game_status, buf);
}

static void board_new_game(void)
{
    memset(g_2048_board, 0, sizeof(g_2048_board));
    g_2048_score = 0;
    g_2048_over = 0;
    board_add_tile();
    board_add_tile();
    board_draw();
}

static int compress_line(int *line)
{
    int tmp[4] = {0};
    int pos = 0;
    int moved = 0;

    for (int i = 0; i < 4; i++)
    {
        if (line[i] != 0)
        {
            tmp[pos++] = line[i];
        }
    }

    for (int i = 0; i < 3; i++)
    {
        if (tmp[i] != 0 && tmp[i] == tmp[i + 1])
        {
            tmp[i] *= 2;
            g_2048_score += tmp[i];
            tmp[i + 1] = 0;
            i++;
        }
    }

    pos = 0;
    for (int i = 0; i < 4; i++)
    {
        int v = tmp[i];
        if (v != 0)
        {
            tmp[pos++] = v;
        }
    }
    while (pos < 4)
    {
        tmp[pos++] = 0;
    }

    for (int i = 0; i < 4; i++)
    {
        if (line[i] != tmp[i])
        {
            moved = 1;
        }
        line[i] = tmp[i];
    }

    return moved;
}

static void game_2048_move(int dir)
{
    int moved = 0;
    int line[4];

    if (g_2048_over)
    {
        return;
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (dir == 0) line[j] = g_2048_board[j][i];
            if (dir == 1) line[j] = g_2048_board[i][3 - j];
            if (dir == 2) line[j] = g_2048_board[3 - j][i];
            if (dir == 3) line[j] = g_2048_board[i][j];
        }

        moved |= compress_line(line);

        for (int j = 0; j < 4; j++)
        {
            if (dir == 0) g_2048_board[j][i] = line[j];
            if (dir == 1) g_2048_board[i][3 - j] = line[j];
            if (dir == 2) g_2048_board[3 - j][i] = line[j];
            if (dir == 3) g_2048_board[i][j] = line[j];
        }
    }

    if (moved)
    {
        board_add_tile();
    }

    if (!board_has_move())
    {
        g_2048_over = 1;
    }

    board_draw();
}

static void show_snake_objects(int show)
{
    for (int y = 0; y < SNAKE_ROWS; y++)
    {
        for (int x = 0; x < SNAKE_COLS; x++)
        {
            set_obj_hidden(g_snake_cells[y][x], !show);
        }
    }
}

static void show_2048_objects(int show)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            set_obj_hidden(g_2048_cells[y][x], !show);
        }
    }
}

static void switch_game(int mode)
{
    g_game_mode = mode;
    show_snake_objects(mode == GAME_SNAKE);
    show_2048_objects(mode == GAME_2048);
    lv_label_set_text(ui_LabelTitle2, mode == GAME_SNAKE ? "贪吃蛇" : "2048");
    lv_label_set_text(g_game_title, mode == GAME_SNAKE ? "贪吃蛇" : "2048");

    if (mode == GAME_SNAKE)
    {
        snake_timer_start();
        snake_new_game();
    }
    else
    {
        snake_timer_stop();
        board_new_game();
    }
}

static void dir_btn_event_cb(lv_event_t *e)
{
    int dir;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    dir = (int)(intptr_t)lv_event_get_user_data(e);
    if (g_game_mode == GAME_SNAKE)
    {
        g_snake_next_dir = dir;
    }
    else
    {
        game_2048_move(dir);
    }
}

static void restart_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    if (g_game_mode == GAME_SNAKE)
    {
        snake_new_game();
    }
    else
    {
        board_new_game();
    }
}

static void create_game_ui(void)
{
    if (g_game_panel != RT_NULL)
    {
        return;
    }

    set_obj_hidden(ui_TabViewquweidonghua, 1);
    set_obj_hidden(ui_Imagequweidonghua, 1);

    g_game_panel = lv_obj_create(ui_quweidonghua);
    lv_obj_set_width(g_game_panel, 462);
    lv_obj_set_height(g_game_panel, 218);
    lv_obj_set_x(g_game_panel, 8);
    lv_obj_set_y(g_game_panel, 42);
    lv_obj_set_align(g_game_panel, LV_ALIGN_TOP_LEFT);
    lv_obj_clear_flag(g_game_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_game_panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_game_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_game_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    g_game_title = lv_label_create(g_game_panel);
    lv_obj_set_width(g_game_title, 130);
    lv_obj_set_x(g_game_title, 310);
    lv_obj_set_y(g_game_title, 0);
    lv_obj_set_style_text_font(g_game_title, &ui_font_chnfont, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_game_title, lv_color_hex(0x20304A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(g_game_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    g_game_status = lv_label_create(g_game_panel);
    lv_obj_set_width(g_game_status, 140);
    lv_obj_set_x(g_game_status, 305);
    lv_obj_set_y(g_game_status, 24);
    lv_obj_set_style_text_font(g_game_status, &ui_font_chnfont, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_game_status, lv_color_hex(0x56677F), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(g_game_status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    for (int y = 0; y < SNAKE_ROWS; y++)
    {
        for (int x = 0; x < SNAKE_COLS; x++)
        {
            lv_obj_t *cell = lv_obj_create(g_game_panel);
            lv_obj_set_width(cell, 14);
            lv_obj_set_height(cell, 14);
            lv_obj_set_x(cell, 22 + x * 15);
            lv_obj_set_y(cell, 10 + y * 15);
            lv_obj_set_align(cell, LV_ALIGN_TOP_LEFT);
            lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_radius(cell, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(cell, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            g_snake_cells[y][x] = cell;
        }
    }

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            lv_obj_t *cell = lv_obj_create(g_game_panel);
            lv_obj_t *label;
            lv_obj_set_width(cell, 50);
            lv_obj_set_height(cell, 50);
            lv_obj_set_x(cell, 32 + x * 56);
            lv_obj_set_y(cell, 6 + y * 52);
            lv_obj_set_align(cell, LV_ALIGN_TOP_LEFT);
            lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_radius(cell, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(cell, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            label = lv_label_create(cell);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(label, lv_color_hex(0x20304A), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_center(label);
            g_2048_cells[y][x] = cell;
            g_2048_labels[y][x] = label;
        }
    }

    g_btn_up = make_game_btn(g_game_panel, "上", 354, 54, 48, 34);
    g_btn_left = make_game_btn(g_game_panel, "左", 304, 92, 48, 34);
    g_btn_right = make_game_btn(g_game_panel, "右", 404, 92, 48, 34);
    g_btn_down = make_game_btn(g_game_panel, "下", 354, 130, 48, 34);
    g_btn_restart = make_game_btn(g_game_panel, "新游戏", 326, 176, 98, 34);

    lv_obj_add_event_cb(g_btn_up, dir_btn_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)0);
    lv_obj_add_event_cb(g_btn_right, dir_btn_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)1);
    lv_obj_add_event_cb(g_btn_down, dir_btn_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)2);
    lv_obj_add_event_cb(g_btn_left, dir_btn_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)3);
    lv_obj_add_event_cb(g_btn_restart, restart_btn_event_cb, LV_EVENT_CLICKED, RT_NULL);

    snake_timer_stop();
}

void Btnchangchangyingyu_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
    lv_obj_add_state(ui_Btnchangchangyingyu, LV_STATE_CHECKED);
    lv_obj_clear_state(ui_Btnxiaozhupeiqi, LV_STATE_CHECKED);
    switch_game(GAME_SNAKE);
}

void Btnxiaozhupeiqi_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
    lv_obj_clear_state(ui_Btnchangchangyingyu, LV_STATE_CHECKED);
    lv_obj_add_state(ui_Btnxiaozhupeiqi, LV_STATE_CHECKED);
    switch_game(GAME_2048);
}

void Screenquweidonghualeft_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
    if (g_game_mode == GAME_SNAKE)
    {
        g_snake_next_dir = 1;
    }
    else
    {
        game_2048_move(1);
    }
}

void Screenquweidonghuaright_event_handler(lv_event_t *e)
{
    RT_UNUSED(e);
    if (g_game_mode == GAME_SNAKE)
    {
        g_snake_next_dir = 3;
    }
    else
    {
        game_2048_move(3);
    }
}

void screen_quweidonghua_pre_init(lv_event_t *e)
{
    RT_UNUSED(e);

    rt_kprintf("%s >>>\n", __func__);
    create_game_ui();

    if (g_game_mode == GAME_SNAKE)
    {
        lv_obj_add_state(ui_Btnchangchangyingyu, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_Btnxiaozhupeiqi, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_Btnchangchangyingyu, LV_STATE_CHECKED);
        lv_obj_add_state(ui_Btnxiaozhupeiqi, LV_STATE_CHECKED);
    }
    switch_game(g_game_mode);
}
