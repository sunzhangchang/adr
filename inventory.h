#pragma once
#include <minwindef.h>
#include <wchar.h>

#define MAX_INVENTORY_ITEMS 64

typedef enum {
    ITEM_WOOD = 0,  // 木头
    ITEM_FOOD,      // 食物
    ITEM_WATER,     // 淡水
    ITEM_GLASS,     // 玻璃
    ITEM_IRON,      // 钢铁
    ITEM_WOOD_FAC,  // 伐木场
    ITEM_WATER_FAC, // 水厂
} ItemType;

typedef struct {
    wchar_t label[32];
    int count;
    int activated;
    int active_count; // 工厂已启用的数量
    int end_tick;
} Item;

int inv_add_new_item(int idx, int count);
int inv_add_new_fac(int idx, int count, DWORD now);

int get_inv_count(void);
Item* inv_get_item(int idx);

void handle_factories(DWORD now);
