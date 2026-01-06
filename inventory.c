#include "inventory.h"

static Item items[MAX_INVENTORY_ITEMS];
static int inv_count = 0;
static Factory facs[MAX_INVENTORY_ITEMS];
static int fac_count = 0;

// 这里采用注册表的方式初始化物品和工厂  TODO: 可以改成从配置文件加载
static const wchar_t* item_labels[] = {
    L"木头", // ITEM_WOOD
    L"食物", // ITEM_FOOD
    L"淡水", // ITEM_WATER
    L"玻璃", // ITEM_GLASS
    L"钢铁", // ITEM_IRON
    L"船",   // ITEM_SHIP
};

static const wchar_t* fac_labels[] = {
    L"伐木场",     // FAC_WOOD
    L"水厂",       // FAC_WATER
    L"冶炼厂",     // FAC_IRON
    L"玻璃厂",     // FAC_GLASS
    L"食品加工厂", // FAC_FOOD
    L"造船厂",     // FAC_SHIP
};

// 建造工厂所需材料
static const Ingredient ings[] = {
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_GLASS, ITEM_IRON}, {20, 20, 40, 10}, 4},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {50, 50, 30}, 3},
};

// 工厂产出
static const Yield yield[] = {
    {{{}, {}, 0}, ITEM_WOOD, 10},
    {{{ITEM_WOOD}, {10}, 1}, ITEM_WATER, 10},
    {{{ITEM_WATER, ITEM_WOOD}, {10, 20}, 2}, ITEM_IRON, 5},
    {{{ITEM_WATER, ITEM_WOOD}, {10, 10}, 2}, ITEM_GLASS, 5},
    {{{ITEM_WATER, ITEM_GLASS}, {20, 10}, 1}, ITEM_FOOD, 5},
    {{{}, {}, 0}, -1, 0},
};

int get_items_count() { return inv_count; }
int get_facs_count() { return fac_count; }

// 物品和工厂初始为未激活状态，激活后显示在界面上
// 注册物品
static void reg_item(int id, const wchar_t* label) {
    Item* it = &items[id];
    *it = (Item){
        .label = L"",
        .count = 0,
        .output = 0,
        .activated = 0,
    };
    wcsncpy_s(it->label, 32, label, _TRUNCATE);
    wprintf(L"Registered item: %s\n", it->label);
}

// 注册工厂
static void reg_fac(int id, const wchar_t* label, const Ingredient* ing,
                    const Yield* yd) {
    Factory* fac = &facs[id];
    *fac = (Factory){
        .label = L"",
        .count = 0,
        .activated = 0,
        .active_count = 0,
        .end_tick = 0,
        .ing = *ing,
        .yield = *yd,
    };
    wcsncpy_s(fac->label, 32, label, _TRUNCATE);
    wprintf(L"Registered factory: %s\n", fac->label);
}

// 初始化仓库系统
void inv_init() {
    inv_count = sizeof(item_labels) / sizeof(item_labels[0]);
    for (int i = 0; i < inv_count; ++i) {
        reg_item(i, item_labels[i]);
    }

    fac_count = sizeof(fac_labels) / sizeof(fac_labels[0]);
    for (int i = 0; i < fac_count; ++i) {
        reg_fac(i, fac_labels[i], &ings[i], &yield[i]);
    }
}

// 激活物品
void activate_item(int id) {
    if (id < MAX_INVENTORY_ITEMS) {
        items[id].activated = 1;
        wprintf(L"Activated item: %s\n", items[id].label);
    }
}

// 激活工厂
void activate_fac(int id, DWORD now) {
    if (id < MAX_INVENTORY_ITEMS) {
        facs[id].activated = 1;
        facs[id].end_tick = now + 10 * 1000;
        wprintf(L"Activated factory: %s\n", facs[id].label);
    }
}

// getter
Item* inv_get_item(int id) { return &items[id]; }
Factory* inv_get_fac(int id) { return &facs[id]; }

// 建造工厂，返回 0 表示成功，1 表示失败（材料不足）
int build_fac(int id) {
    Factory* fac = &facs[id];
    Ingredient* ing = &fac->ing;
    for (int i = 0; i < ing->count; ++i) {
        Item* it = &items[ing->id[i]];
        if (!it || it->count < ing->num[i]) {
            return 1;
        }
    }

    for (int i = 0; i < ing->count; ++i) {
        Item* it = &items[ing->id[i]];
        it->count -= ing->num[i];
    }
    fac->count += 1;
    fac_add_active_count(fac, +1);
    return 0;
}

// 处理所有工厂的生产过程
void handle_factories(DWORD now) {
    for (int i = 0; i < fac_count; i++) {
        Factory* fac = &facs[i];
        if (!fac->activated)
            continue;

        // 检查工厂是否到达生产时间
        int remaining =
            (int)((fac->end_tick > now) ? ((fac->end_tick - now + 999) / 1000)
                                        : 0);

        if (remaining > 0) {
            continue;
        }

        const Ingredient* inputs = &(fac->yield.inputs);

        // num 为最大可生产数量
        int num = 1e9;
        for (int j = 0; j < inputs->count; ++j) {
            Item* it = inv_get_item(inputs->id[j]);
            num = min(num, min(fac->active_count, it->count / inputs->num[j]));
        }

        num = min(num, fac->active_count);
        for (int j = 0; j < inputs->count; ++j) {
            Item* it = inv_get_item(inputs->id[j]);
            it->count -= inputs->num[j] * num;
        }

        // 更新生产物品的数量
        Item* out = inv_get_item(fac->yield.output_id);
        out->count += fac->yield.output_num * num;

        // 重置工厂的生产计时器
        fac->end_tick = now + 10 * 1000;
    }
}

// 用这个函数修改 active_count！！！
void fac_add_active_count(Factory* fac, int delta) {
    fac->active_count += delta;

    // 更新输入输出物品的产出速率
    const Yield* yd = &fac->yield;
    const Ingredient* inputs = &yd->inputs;
    for (int i = 0; i < inputs->count; ++i) {
        Item* it = inv_get_item(inputs->id[i]);
        it->output -= delta * inputs->num[i];
    }

    Item* out = inv_get_item(yd->output_id);
    out->output += delta * yd->output_num;
}
