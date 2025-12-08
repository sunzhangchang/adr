#pragma once
#include "button.h"

#define MAX_BUTTONS_PER_SCENE 16
#define MAX_SCENE_TITLE_LEN 64
#define MAX_SCENES 32

// 场景结构
typedef struct {
    wchar_t title[MAX_SCENE_TITLE_LEN];
    Button buttons[MAX_BUTTONS_PER_SCENE];
    int btn_count;
} Scene;

// 初始化场景（分配/设置每个场景的按钮及 action）
void init_scenes(void);

// 返回场景数量
int get_scene_count(void);

// 获取场景指针
Scene* get_scene(int idx);

// 在指定场景追加按钮（返回 0 成功，-1 失败）
int scene_add_button(int scene_idx, const Button* btn);

// 修改场景标题（复制字符串）
void scene_set_title(int scene_idx, const wchar_t* title);

// 新增：动态创建场景（返回新场景索引，失败返回 -1）
int scene_add_scene(const wchar_t* title);

// 新增：按标题查找场景索引，找不到返回 -1
int scene_find_by_title(const wchar_t* title);
