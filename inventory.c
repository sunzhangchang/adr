#include "inventory.h"

static Item items[MAX_INVENTORY_ITEMS];
static int inv_count = 0;

static const wchar_t* item_labels[] = {
    L"木头",   // ITEM_WOOD
    L"食物",   // ITEM_FOOD
    L"淡水",   // ITEM_WATER
    L"玻璃",   // ITEM_GLASS
    L"钢铁",   // ITEM_IRON
    L"伐木场", // ITEM_WOOD_FAC
    L"水厂",   // ITEM_WATER_FAC
};

int get_inv_count(void) { return inv_count; }

int inv_add_new_item(int idx, int count) {
    if (idx < MAX_INVENTORY_ITEMS) {
        if (idx >= inv_count) {
            inv_count = idx + 1;
        }
        wcsncpy_s(items[idx].label, 32, item_labels[idx], _TRUNCATE);
        items[idx].count = count;
        items[idx].activated = 1;

        return 1;
    }
    return -1;
}

int inv_add_new_fac(int idx, int count, DWORD now) {
    if (inv_add_new_item(idx, count) == -1) {
        return -1;
    }
    items[idx].active_count = count;
    items[idx].end_tick = now + 10 * 1000;
    return 1;
}

Item* inv_get_item(int idx) { return &items[idx]; }

void handle_factories(DWORD now) {
    for (int i = 0; i < inv_count; i++) {
        Item* fac = &items[i];
        if (!fac->activated)
            continue;

        int remaining =
            (int)((fac->end_tick > now) ? ((fac->end_tick - now + 999) / 1000)
                                        : 0);

        if (remaining > 0) {
            continue;
        }

        fac->end_tick = now + 10 * 1000;

        switch (i) {
        case ITEM_WOOD_FAC: {
            Item* wood = inv_get_item(ITEM_WOOD);
            wood->count += 5 * fac->active_count;
            break;
        }

        case ITEM_WATER_FAC: {
            Item* water = inv_get_item(ITEM_WATER);
            Item* wood = inv_get_item(ITEM_WOOD);
            int cnt = wood->count / 10;
            cnt = ((cnt > fac->active_count) ? fac->active_count : cnt);
            water->count += 5 * cnt;
            wood->count -= cnt * 10;
            break;
        }

        default: {
            break;
        }
        }
    }
}
