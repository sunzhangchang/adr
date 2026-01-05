#include "ui.h"
#include "inventory.h"
#include "story.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <windows.h>

// 全局 SDL 对象
static SDL_Window* win = NULL;
static SDL_Renderer* renderer = NULL;
static TTF_Font* font = NULL;
static int win_w = 1000;
static int win_h = 600;
static int font_h = 18;

// 将 wchar 转换为 UTF-8 编码
static void wchar_to_utf8(const wchar_t* w, char* out, int outlen) {
    if (!w || !out)
        return;
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
    if (n <= 0 || n > outlen) {
        out[0] = 0;
        return;
    }
    WideCharToMultiByte(CP_UTF8, 0, w, -1, out, outlen, NULL, NULL);
}

// 初始化 SDL UI
int init_sdl_ui(int width, int height) {
    // 初始化 SDL，失败返回 -1
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return -1;

    // 初始化字体，失败返回 -1
    if (TTF_Init() != 0) {
        SDL_Quit();
        return -1;
    }

    // 设置窗口尺寸
    win_w = width > 0 ? width : 1000;
    win_h = height > 0 ? height : 600;

    // 创建窗口
    win =
        SDL_CreateWindow("ADR", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         win_w, win_h, SDL_WINDOW_SHOWN);

    // 窗口创建失败，清理并返回 -1
    if (!win) {
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // 渲染器创建失败，清理并返回 -1
    if (!renderer) {
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // 尝试加载常见中文字体，若不存在可替换路径，优先使用 微软雅黑 ttc
    const char* font_path = "C:\\Windows\\Fonts\\msyh.ttc";
    font = TTF_OpenFont(font_path, 16);

    // ttc 加载失败，则回退到 msyh.ttf
    if (!font) {
        font_path = "C:\\Windows\\Fonts\\msyh.ttf";
        font = TTF_OpenFont(font_path, 16);
    }

    // 失败，回退到 Arial 字体
    if (!font) {
        font_path = "C:\\Windows\\Fonts\\arial.ttf";
        font = TTF_OpenFont(font_path, 16);
    }

    // 这些字体都加载失败，清理并返回 -1
    if (!font) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // 成功，记录字体行高
    font_h = TTF_FontLineSkip(font);
    return 0;
}

// 关闭 SDL UI
void shutdown_sdl_ui(void) {
    if (font) {
        TTF_CloseFont(font);
        font = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (win) {
        SDL_DestroyWindow(win);
        win = NULL;
    }
    TTF_Quit();
    SDL_Quit();
}

// 绘制文本到 renderer（UTF-8）
static void draw_text_utf8(int x, int y, const char* utf8, SDL_Color color) {
    if (!renderer || !font || !utf8)
        return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, utf8, color);
    if (!surf)
        return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (!tex) {
        SDL_FreeSurface(surf);
        return;
    }
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_FreeSurface(surf);
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

// 清空某一区域
void clear_area(SHORT x, SHORT y, DWORD width, int height) {
    if (!renderer)
        return;
    SDL_Rect r = {x, y, (int)width, height};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &r);
}

// 绘制剧情区
void draw_story(int left_width, int rows) {
    if (!renderer)
        return;
    // 清左侧剧情区
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect area = {0, 0, left_width, win_h};
    SDL_RenderFillRect(renderer, &area);

    // 标题
    char tmp[512];
    wchar_to_utf8(L"剧情", tmp, sizeof(tmp));
    draw_text_utf8(8, 4, tmp, (SDL_Color){255, 255, 255, 255});

    // 内容起始 y 坐标
    int y0 = 4 + font_h + 4;

    // 计算可见行数（向下可用高度 / 行高）
    int available_height = win_h - y0 - 4; // 底部留一点空隙
    int visible_lines =
        (available_height > 0) ? (available_height / font_h) : 0;
    if (visible_lines <= 0)
        return;

    // 显示最新的 visible_lines 行（新剧情显示在最上方）
    int cnt = get_story_line_count();
    int visible_cnt = cnt < visible_lines ? cnt : visible_lines;

    int y = y0;
    for (int i = 0; i < visible_cnt; ++i) {
        // 从队尾读取剧情文本
        wchar_to_utf8(get_story_line(--cnt), tmp, sizeof(tmp));
        draw_text_utf8(8, y, tmp, (SDL_Color){220, 220, 220, 255});
        y += font_h;
    }
}

// 绘制场景列表（位于按钮区顶部）
void draw_scene_list(int left_width, int cols, int rows, int selected,
                     int startY, int btn_width) {
    if (!renderer)
        return;
    int x0 = left_width;
    int w = btn_width;
    // 背景（仅按钮区背景）
    SDL_Rect area = {x0, 0, w, win_h};
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(renderer, &area);

    char tmp[512];

    int scene_cnt = get_scene_count();

    // 水平排列：每项等宽分配（考虑左右内边距 8）
    if (scene_cnt <= 0)
        return;
    int spacing = 6;
    // 考虑间距后平均分配可用宽度，避免越界
    int available_w = w - 16;
    int tot_spacing = spacing * (scene_cnt - 1);
    int item_w = (scene_cnt > 0) ? ((available_w - tot_spacing) / scene_cnt)
                                 : available_w;
    if (item_w < 60)
        item_w = 60; // 最小宽度保障
    int item_h = font_h + 8;
    int item_x = x0 + 8;

    for (int i = 0; i < scene_cnt; i++) {
        // 若超出右侧区域，停止绘制
        if (item_x + item_w > x0 + w - 8)
            break;
        SDL_Rect ir = {item_x, startY, item_w, item_h};
        if (i == selected) {
            SDL_SetRenderDrawColor(renderer, 80, 80, 120, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        }
        SDL_RenderFillRect(renderer, &ir);

        // 绘制场景名称
        wchar_to_utf8(get_scene(i)->title, tmp, sizeof(tmp));
        draw_text_utf8(item_x + 6, startY + 2, tmp,
                       (SDL_Color){255, 255, 255, 255});

        item_x += item_w + spacing;
    }
}

// 绘制按钮
void draw_buttons(int left_width, int cols, int rows, const Button buttons[],
                  int btn_count, int selected, int bnt_start_y, int btn_width,
                  const wchar_t* title) {
    if (!renderer)
        return;

    int x0 = left_width;
    int w = btn_width;

    // 按钮起始 y 坐标
    int y = bnt_start_y;
    int item_h = font_h + 8;
    for (int i = 0; i < btn_count; i++) {
        const Button* btn = &buttons[i];
        wchar_t display_label[64];

        // 格式化显示文本到 display_labels
        // 如果有倒计时，显示倒计时
        if (btn->countdown > 0) {
            swprintf_s(display_label, 64, L"%s (%d)", btn->label,
                       btn->countdown);
        } else {
            // 没有倒计时，检查是否为工厂启用/暂停按钮
            if (btn->is_st == 1) {
                swprintf_s(display_label, 64, L"%s (已启用 x%d)", btn->label,
                           inv_get_fac(btn->fac_id)->active_count);
            } else if (btn->is_st == 2) {
                const Factory* fac = inv_get_fac(btn->fac_id);
                swprintf_s(display_label, 64, L"%s (已暂停 x%d)", btn->label,
                           fac->count - fac->active_count);
            } else {
                // 不是工厂启用/暂停按钮，若有关联的工厂，则说明是建造工厂的按钮
                if (btn->fac_id != -1) {
                    // cnt 记录当前已写入长度
                    int cnt =
                        swprintf_s(display_label, 64, L"%s: ", btn->label);

                    // 显示建造该工厂所需的材料
                    const Ingredient* ing = &inv_get_fac(btn->fac_id)->ing;
                    for (int i = 0; i < ing->count; ++i) {
                        cnt += swprintf_s(
                            display_label + cnt, 64 - cnt, L"%s -%d ",
                            inv_get_item(ing->id[i])->label, ing->num[i]);
                    }
                } else {
                    // 没有关联的工厂，直接显示按钮标签
                    wcsncpy_s(display_label, 64, btn->label, _TRUNCATE);
                }
            }
        }

        // 背景
        SDL_Rect br = {x0 + 8, y, w - 16, item_h};
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderFillRect(renderer, &br);
        // 文本
        char tmp[512];
        wchar_to_utf8(display_label, tmp, sizeof(tmp));
        draw_text_utf8(x0 + 12, y + 2, tmp, (SDL_Color){255, 255, 255, 255});
        y += item_h + 6;
    }

    // 绘制按钮-仓库分割线（竖线）
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    int sepX = left_width + btn_width;
    SDL_RenderDrawLine(renderer, sepX, 0, sepX, win_h);
}

// 绘制仓库区域
void draw_inventory(int left_width, int cols, int rows, int inv_start_x,
                    int inv_start_y, int inv_width) {
    if (!renderer)
        return;
    int x0 = inv_start_x;
    int w = inv_width;
    SDL_Rect area = {x0, 0, w, win_h};
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &area);

    // 仓库标题
    char tmp[256];
    wchar_to_utf8(L"仓库 | 每 10 秒产量", tmp, sizeof(tmp));
    draw_text_utf8(x0 + 8, 4, tmp, (SDL_Color){200, 200, 200, 255});
    int y = inv_start_y + font_h + 4;

    // 显示所有已激活的物品
    int items_count = get_items_count();
    for (int i = 0; i < items_count; i++) {
        Item* item = inv_get_item(i);
        if (!item || !item->activated)
            continue;
        wchar_t buf[128];
        // 物品后面显示每 10 秒产量
        swprintf_s(buf, 128, L"%s x%d | %+d", item->label, item->count,
                   item->output);
        wchar_to_utf8(buf, tmp, sizeof(tmp));
        draw_text_utf8(x0 + 8, y, tmp, (SDL_Color){200, 200, 200, 255});
        y += font_h;
    }
    
    y += font_h;

    // 显示所有已激活的工厂
    int facs_count = get_facs_count();
    for (int i = 0; i < facs_count; i++) {
        Factory* fac = inv_get_fac(i);
        if (!fac || !fac->activated)
            continue;
        wchar_t buf[128];
        swprintf_s(buf, 128, L"%s x%d", fac->label, fac->count);
        wchar_to_utf8(buf, tmp, sizeof(tmp));
        draw_text_utf8(x0 + 8, y, tmp, (SDL_Color){200, 200, 200, 255});
        y += font_h;
    }
}

// 初始化界面并显示
void init_ui(int left_width, int cols, int rows, int bnt_start_y, int btn_width,
             int inv_start_x, int inv_width) {
    if (!renderer)
        return;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    draw_story(left_width, rows);
    draw_inventory(left_width, cols, rows, inv_start_x, 0, inv_width);
    SDL_RenderPresent(renderer);
}

// 更新 UI
void update_ui_display(int left_width, int cols, int rows, int bnt_start_y,
                       int btn_width, int inv_start_x, int inv_width,
                       int cur_scene) {
    if (!renderer)
        return;
    // 清屏
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 绘制剧情区
    draw_story(left_width, rows);

    // 场景列表（顶部）
    // 计算 sceneStartY 使其与 main 中 bnt_start_y 的计算保持一致：
    // main 使 bnt_start_y = sceneStartY + (fontH+8) + 6
    int item_h = font_h + 8;
    int scene_start_y = bnt_start_y - item_h - 6;
    if (scene_start_y < 0)
        scene_start_y = 8;
    draw_scene_list(left_width, cols, rows, cur_scene, scene_start_y,
                    btn_width);

    const Scene* cur_sc = get_scene(cur_scene);

    draw_buttons(left_width, cols, rows, cur_sc->buttons, cur_sc->btn_count, 0,
                 bnt_start_y, btn_width, NULL);

    draw_inventory(left_width, cols, rows, inv_start_x, 0, inv_width);

    SDL_RenderPresent(renderer);
}

// 返回字体行高（像素），对齐项目布局
int ui_get_font_height() {
    if (font == NULL) {
        // 未初始化字体时返回一个默认值
        return 18;
    }
    return font_h;
}
