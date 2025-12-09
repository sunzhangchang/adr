#pragma once
#include "scene.h"
#include <wchar.h>
#include <windows.h>

// SDL UI 初始化/销毁（main 调用）
int init_sdl_ui(int width, int height); // 返回 0 成功
void shutdown_sdl_ui(void);

void main_loop(void);

void draw_inventory(int left_width, int cols, int rows, int inv_start_x,
                    int inv_start_y, int inv_width);
// 绘制剧情区
void redraw_story(int leftWileft_widthdth, int rows);
// 清空矩形区域（像素单位）
void clear_area(SHORT x, SHORT y, DWORD width, int height);
// 清空按钮区
// 初始化 UI（绘制初始界面）
void init_ui(int left_width, int cols, int rows, int bnt_start_y, int btn_width,
             int inv_start_x, int inv_width);
// 更新 UI 显示（使用 GameState*，减少参数）
void update_ui_display(int left_width, int cols, int rows, int bnt_start_y,
                       int btn_width, int inv_start_x, int inv_width,
                       int cur_scene);
// 新增：绘制场景列表（位于按钮区顶部）
void draw_scene_list(int left_width, int cols, int rows, int selected,
                     int startY, int btn_width);
// 新增：获取当前字体行高（像素），由 ui 提供
int ui_get_font_height(void);
