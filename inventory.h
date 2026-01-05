#pragma once
#include <minwindef.h>
#include <wchar.h>

#define MAX_INVENTORY_ITEMS 64

// 物品枚举，以 ITEM_ 开头
typedef enum {
    ITEM_WOOD = 0, // 木头
    ITEM_FOOD,     // 食物
    ITEM_WATER,    // 淡水
    ITEM_GLASS,    // 玻璃
    ITEM_IRON,     // 钢铁
    ITEM_SHIP,     // 船
} ItemType;

// 工厂枚举，以 FAC_ 开头
typedef enum {
    FAC_WOOD = 0, // 伐木场
    FAC_WATER,    // 水厂
    FAC_IRON,     // 冶炼厂
    FAC_GLASS,    // 玻璃厂
    FAC_FOOD,     // 食品加工厂
    FAC_SHIP,     // 造船厂
} FactoryType;

/**
 * 物品结构体
 * @public label 物品名称
 * @public count 物品数量
 * @public output 物品每 10 秒产量
 * @public activated 物品是否已激活
 */
typedef struct {
    wchar_t label[32];
    int count;
    int output;
    int activated;
} Item;

/**
 * 建造工厂所需材料
 * @public id 材料物品 ID 数组
 * @public num 材料物品数量数组
 * @public count 材料种类数量
 */
typedef struct {
    int id[8];
    int num[8];
    int count;
} Ingredient;

/**
 * 工厂生产所需材料及产出
 * @public inputs 所需材料
 * @public output_id 产出物品 ID
 * @public output_num 产出物品数量
 */
typedef struct {
    Ingredient inputs;
    int output_id;
    int output_num;
} Yield;

/**
 * 工厂结构体
 * @public label 工厂名称
 * @public count 工厂数量
 * @public activated 工厂是否已激活
 * @public active_count 工厂已启用的数量
 * @public end_tick 工厂当前生产结束时间戳
 * @public ing 建造工厂所需材料
 * @public yield 工厂生产所需材料及产出
 */
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
