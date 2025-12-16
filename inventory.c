#include "inventory.h"

static Item items[MAX_INVENTORY_ITEMS];
static int inv_count = 0;
static Factory facs[MAX_INVENTORY_ITEMS];
static int fac_count = 0;

static const wchar_t* item_labels[] = {
    L"木头", // ITEM_WOOD
    L"食物", // ITEM_FOOD
    L"淡水", // ITEM_WATER
    L"玻璃", // ITEM_GLASS
    L"钢铁", // ITEM_IRON
};

static const wchar_t* fac_labels[] = {
    L"伐木场",     // FAC_WOOD
    L"水厂",       // FAC_WATER
    L"冶炼厂",     // FAC_IRON
    L"玻璃厂",     // FAC_GLASS
    L"食品加工厂", // FAC_FOOD
};

static const Ingredient ings[] = {
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_IRON}, {20, 20, 10}, 3},
    {{ITEM_FOOD, ITEM_WOOD, ITEM_GLASS, ITEM_IRON}, {20, 20, 20, 10}, 4},
};

static const Yield yield[] = {
    {{{}, {}, 0}, ITEM_WOOD, 5},
    {{{ITEM_WOOD}, {10}, 1}, ITEM_WATER, 5},
    {{{ITEM_WATER, ITEM_WOOD}, {10, 20}, 2}, ITEM_IRON, 5},
    {{{ITEM_WATER, ITEM_WOOD}, {10, 10}, 2}, ITEM_GLASS, 5},
    {{{ITEM_WATER, ITEM_GLASS}, {20, 10}, 1}, ITEM_FOOD, 5},
};

int get_items_count() { return inv_count; }
int get_facs_count() { return fac_count; }

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

void activate_item(int id) {
    if (id < MAX_INVENTORY_ITEMS) {
        items[id].activated = 1;
        wprintf(L"Activated item: %s\n", items[id].label);
    }
}

void activate_fac(int id, DWORD now) {
    if (id < MAX_INVENTORY_ITEMS) {
        facs[id].activated = 1;
        facs[id].end_tick = now + 10 * 1000;
        wprintf(L"Activated factory: %s\n", facs[id].label);
    }
}

Item* inv_get_item(int id) { return &items[id]; }
Factory* inv_get_fac(int id) { return &facs[id]; }

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

void handle_factories(DWORD now) {
    for (int i = 0; i < fac_count; i++) {
        Factory* fac = &facs[i];
        if (!fac->activated)
            continue;

        int remaining =
            (int)((fac->end_tick > now) ? ((fac->end_tick - now + 999) / 1000)
                                        : 0);

        if (remaining > 0) {
            continue;
        }

        const Ingredient* inputs = &(fac->yield.inputs);

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

        Item* out = inv_get_item(fac->yield.output_id);
        out->count += fac->yield.output_num * num;

        fac->end_tick = now + 10 * 1000;
    }
}

void fac_add_active_count(Factory* fac, int delta) {
    fac->active_count += delta;

    const Yield* yd = &fac->yield;
    const Ingredient* inputs = &yd->inputs;
    for (int i = 0; i < inputs->count; ++i) {
        Item* it = inv_get_item(inputs->id[i]);
        it->output -= delta * inputs->num[i];
    }

    Item* out = inv_get_item(yd->output_id);
    out->output += delta * yd->output_num;
}
