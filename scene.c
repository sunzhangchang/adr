#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static Scene scenes[MAX_SCENES];
static int scene_count = 0;

// 初始化场景
void init_scenes(void) {
    scene_count = 0;
    Scene* s0 = &scenes[scene_count++];
    wcsncpy_s(s0->title, MAX_SCENE_TITLE_LEN, L"海滨小屋", _TRUNCATE);
    s0->btn_count = 0;

    Button b0 = new_button(L"伐木", ACT_WOOD, 5);
    scene_add_button(0, &b0);
}

// getter
int get_scene_count(void) { return scene_count; }

Scene* get_scene(int idx) {
    if (idx < 0 || idx >= scene_count)
        return NULL;
    return &scenes[idx];
}

// 为指定场景添加按钮
int scene_add_button(int scene_idx, const Button* btn) {
    if (scene_idx < 0 || scene_idx >= scene_count)
        return -1;
    Scene* s = &scenes[scene_idx];
    if (s->btn_count >= MAX_BUTTONS_PER_SCENE)
        return -1;

    for (int i = 0; i < s->btn_count; i++) {
        if (wcscmp(s->buttons[i].label, btn->label) == 0) {
            // 已存在同名按钮，忽略添加
            wprintf(L"Button '%s' already exists in scene '%s', skipping add\n",
                    btn->label, s->title);
            return -1;
        }
    }

    s->buttons[s->btn_count++] = *btn;
    wprintf(L"Button '%s' added to scene '%s'\n", btn->label, s->title);

    return 0;
}

void scene_set_title(int scene_idx, const wchar_t* title) {
    if (scene_idx < 0 || scene_idx >= scene_count)
        return;
    wcsncpy_s(scenes[scene_idx].title, MAX_SCENE_TITLE_LEN, title, _TRUNCATE);
}

// 按标题查找场景索引
int scene_find_by_title(const wchar_t* title) {
    if (!title)
        return -1;
    for (int i = 0; i < scene_count; i++) {
        if (wcscmp(scenes[i].title, title) == 0)
            return i;
    }
    return -1;
}

// 动态创建场景
int scene_add_scene(const wchar_t* title) {
    if (!title)
        return -1;
    int exist = scene_find_by_title(title);
    if (exist != -1)
        return exist;
    if (scene_count >= MAX_SCENES)
        return -1;
    Scene* s = &scenes[scene_count];
    wcsncpy_s(s->title, MAX_SCENE_TITLE_LEN, title, _TRUNCATE);
    s->btn_count = 0;
    scene_count++;
    return scene_count - 1;
}
