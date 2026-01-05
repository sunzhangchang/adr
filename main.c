#include "button.h"
#include "inventory.h"
#include "scene.h"
#include "story.h"
#include "ui.h"
#include <SDL2/SDL.h>
#include <locale.h>
#include <windows.h>

// 主循环
void main_loop(void) {
    setlocale(LC_ALL, "");

    // 窗口尺寸 宽 高
    const int win_w = 1000;
    const int win_h = 600;

    // 初始化 SDL UI
    if (init_sdl_ui(win_w, win_h) != 0) {
        // 无法初始化 SDL UI，退出
        return;
    }

    // 读取按钮倒计时配置（可选文件，若不存在则使用代码默认）
    load_button_times("button_times.cfg");

    int left_width = 400;         // 剧情区宽度
    const int center_width = 400; // 按钮区宽度
    const int inv_width = 200;    // 仓库区宽度

    // 计算剧情显示宽度（字符宽度估算）
    set_story_width(left_width / 9);

    // 显示开场剧情
    add_story(get_story_text(STORY_INTRO));

    // 仓库初始化
    inv_init();

    // 仓库初始为 木头 x15
    activate_item(ITEM_WOOD);
    inv_get_item(ITEM_WOOD)->count = 15;

    // 场景初始化
    init_scenes();

    // 当前场景 ID
    int cur_scene_id = 0;
    // 当前场景
    Scene* sc = get_scene(cur_scene_id);

    // SDL UI 初始化界面
    int scene_start_y = 8; // 场景行起始 Y（像素）
    // btn_start_y 需要基于字体高度与场景行高度计算，后续在 init 之前计算
    int inv_start_x = left_width + center_width + 10;
    // 字体已在 init_sdl_ui 初始化，获取字体高度并计算 btn_start_y
    int scene_item_h = ui_get_font_height() + 8;
    int btn_start_y = scene_start_y + scene_item_h + 6;

    // 初始化 UI
    init_ui(left_width, win_w, win_h, btn_start_y, center_width, inv_start_x,
            inv_width);

    // 是否运行中
    BOOL running = TRUE;
    // 延时时间
    Uint32 waitTimeoutMs = 16; // 约 60 FPS

    while (running) {
        Uint32 now = SDL_GetTicks();

        // 更新所有场景中各按钮的倒计时
        int scene_cnt = get_scene_count();
        for (int i = 0; i < scene_cnt; i++) {
            const Scene* scene = get_scene(i);
            if (!scene)
                continue;

            int bnt_cnt = scene->btn_count;
            if (bnt_cnt > 0) {
                update_button_countdown((Button*)scene->buttons, bnt_cnt, now);
            }
        }

        // 更新工厂生产物品
        handle_factories(now);

        // 刷新当前场景与其按钮数组
        sc = get_scene(cur_scene_id);
        Button* scene_btns = sc->buttons;
        int scene_bnt_cnt = sc->btn_count;

        scene_cnt = get_scene_count();
        // 更新 UI 显示
        update_ui_display(left_width, win_w, win_h, btn_start_y, center_width,
                          inv_start_x, inv_width, cur_scene_id);

        // 处理鼠标点击事件
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = FALSE;
            } else if (ev.type == SDL_MOUSEBUTTONDOWN &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                int mx = ev.button.x;
                int my = ev.button.y;

                // 判断是否为点击切换场景：按 ui.c 中水平布局一致性计算项宽
                int sceneCountLocal = scene_cnt;
                int availableW = center_width - 16;
                int itemW = sceneCountLocal > 0 ? (availableW / sceneCountLocal)
                                                : availableW;
                if (itemW < 60)
                    itemW = 60;
                int spacing = 6;
                int itemX0 = left_width + 8;
                int itemY = scene_start_y;
                int clickedScene = -1;
                for (int si = 0; si < sceneCountLocal; si++) {
                    int x1 = itemX0 + si * (itemW + spacing);
                    int x2 = x1 + itemW;
                    if (mx >= x1 && mx <= x2 && my >= itemY &&
                        my < itemY + scene_item_h) {
                        clickedScene = si;
                        break;
                    }
                }

                if (clickedScene >= 0) {
                    cur_scene_id = clickedScene;
                    continue;
                }

                // 未点击切换场景，则检测当前场景的动作按钮点击
                const int btn_h = ui_get_font_height() + 8;
                const int btn_spacing = btn_h + 6;
                for (int btn_i = 0; btn_i < scene_bnt_cnt; btn_i++) {
                    int btnY = btn_start_y + btn_i * btn_spacing;
                    // 按钮绘制区域为 x: leftWidth+8 .. leftWidth+buttonsWidth-8
                    if (mx >= left_width + 8 &&
                        mx <= left_width + center_width - 8 && my >= btnY &&
                        my < btnY + btn_h) {
                        if (scene_btns[btn_i].countdown > 0 ||
                            scene_btns[btn_i].active == 0)
                            break;

                        // 处理按钮点击
                        handle_button_click(scene_btns, scene_bnt_cnt, btn_i,
                                            (DWORD)now);
                        break;
                    }
                }
            }
        }

        // 小延时以避免 CPU 占满
        SDL_Delay(waitTimeoutMs);
    }

    // 退出前销毁 SDL UI
    shutdown_sdl_ui();
}

// 这里的 main 函数实际上是 SDL_main 的别名，为程序入口
int main(int argc, char* argv[]) {
    main_loop();
    return 0;
}