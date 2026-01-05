#pragma once
#include <minwindef.h>
#include <wchar.h>

#define MAX_INVENTORY_ITEMS 64

typedef enum {
    ITEM_WOOD = 0, // 木头
    ITEM_FOOD,     // 食物
    ITEM_WATER,    // 淡水
    ITEM_GLASS,    // 玻璃
    ITEM_IRON,     // 钢铁
    ITEM_SHIP,     // 船
} ItemType;

typedef enum {
    FAC_WOOD = 0, // 伐木场
    FAC_WATER,    // 水厂
    FAC_IRON,     // 冶炼厂
    FAC_GLASS,    // 玻璃厂
    FAC_FOOD,     // 食品加工厂
    FAC_SHIP,     // 造船厂
} FactoryType;

typedef struct {
    wchar_t label[32];
    int count;
    int output;
    int activated;
} Item;

typedef struct {
    int id[8];
    int num[8];
    int count;
} Ingredient;

typedef struct {
    Ingredient inputs;
    int output_id;
    int output_num;
} Yield;

typedef struct {
    wchar_t label[32];
    int count;
    int activated;

    int active_count; // 不要直接修改这个值！！！工厂已启用的数量
    int end_tick;

    Ingredient ing;
    Yield yield;
} Factory;

void inv_init();

void activate_item(int id);
void activate_fac(int id, DWORD now);

int get_items_count();
int get_facs_count();

Item* inv_get_item(int id);
Factory* inv_get_fac(int id);

int build_fac(int id);

void handle_factories(DWORD now);

// 用这个函数修改 active_count！！！
void fac_add_active_count(Factory* fac, int delta);
