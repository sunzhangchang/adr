#pragma once
#include "inventory.h"
#include <wchar.h>
#include <windows.h>

// 按钮动作枚举
typedef enum {
    ACT_NONE = 0,
    ACT_WOOD,            // 伐木
    ACT_FISH,            // 捕鱼
    ACT_WATER,           // 取水
    ACT_GLASS,           // 生产玻璃
    ACT_IRON,            // 冶铁
    ACT_WOOD_FAC,        // 伐木场
    ACT_WATER_FAC,       // 水厂
    ACT_START_WATER_FAC, // 启用水厂
    ACT_STOP_WATER_FAC,  // 停止水厂
} ButtonAction;

// Button 模型
typedef struct {
    wchar_t label[64];
    int active;          // 是否显示
    int countdown;       // 当前倒计时剩余秒数
    int total_time;      // 倒计时总秒数
    DWORD end_tick;      // 结束时间戳
    ButtonAction action; // 新字段：按钮动作
    int on_complete; // 倒计时结束时触发的内部标记（0 无，1）
    int is_st; // 是否为工厂的启动或停止按钮（0 不是，1 启动，2 停止）
    FactoryType fac_id; // 关联的物品索引（如有）
} Button;

Button new_button(const wchar_t* label, ButtonAction action, int total_time);

// 更新按钮倒计时显示（使用 GameState）
void update_button_countdown(Button* buttons, int count, DWORD now);

// 处理按钮点击（使用 GameState）
void handle_button_click(Button* buttons, int count, int btnIndex, DWORD now);

// 从配置文件加载按钮倒计时，path 可为 "button_times.cfg"，返回 0 成功，非0失败
int load_button_times(const char* path);