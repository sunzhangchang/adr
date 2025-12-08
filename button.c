#include "button.h"
#include "scene.h"
#include "story.h"
#include <stdio.h>

// 全局动作时间映射：-1 表示未配置
static int g_action_time[32]; // 足够容纳动作枚举
static void init_action_time_map(void) {
    for (int i = 0; i < 32; i++)
        g_action_time[i] = -1;
}

// 按动作名映射到枚举
static ButtonAction action_from_string_local(const char* s) {
    if (!s)
        return ACT_NONE;
    if (strcmp(s, "ACT_WOOD") == 0)
        return ACT_WOOD;
    if (strcmp(s, "ACT_FISH") == 0)
        return ACT_FISH;
    if (strcmp(s, "ACT_WATER") == 0)
        return ACT_WATER;
    if (strcmp(s, "ACT_GLASS") == 0)
        return ACT_GLASS;
    if (strcmp(s, "ACT_IRON") == 0)
        return ACT_IRON;
    if (strcmp(s, "ACT_WOOD_FAC") == 0)
        return ACT_WOOD_FAC;
    if (strcmp(s, "ACT_WATER_FAC") == 0)
        return ACT_WATER_FAC;
    if (strcmp(s, "ACT_START_WATER_FAC") == 0)
        return ACT_START_WATER_FAC;
    if (strcmp(s, "ACT_STOP_WATER_FAC") == 0)
        return ACT_STOP_WATER_FAC;
    return ACT_NONE;
}

// 从配置文件加载动作倒计时，格式：key=value，例如：ACT_LIGHT=5
int load_button_times(const char* path) {
    if (!path)
        return -1;
    FILE* f = fopen(path, "r");
    init_action_time_map();
    if (!f)
        return -1;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // trim
        char* p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        if (*p == 0 || *p == '#')
            continue;
        char* eq = strchr(p, '=');
        if (!eq)
            continue;
        *eq = 0;
        char* key = p;
        char* val = eq + 1;
        // trim trailing
        char* end = key + strlen(key) - 1;
        while (end >= key && (*end == ' ' || *end == '\t'))
            *end-- = 0;
        end = val + strlen(val) - 1;
        while (end >= val &&
               (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
            *end-- = 0;
        int t = atoi(val);
        if (t < 0)
            continue;
        ButtonAction a = action_from_string_local(key);
        if (a != ACT_NONE) {
            g_action_time[(int)a] = t;
        }
    }
    fclose(f);
    return 0;
}

// 修改 new_button：如果配置中为该 action 指定了时间则覆盖传入 total_time
Button new_button(const wchar_t* label, ButtonAction action, int total_time) {
    Button btn = (Button){
        .label = L"",
        .active = 1,
        .countdown = 0,
        .total_time = total_time,
        .end_tick = 0,
        .action = action,
        .on_complete = 0,
        .is_st = 0,
        .item_idx = -1,
    };
    // 若配置中存在指定时间，覆盖
    if (action >= 0 && action < 32 && g_action_time[(int)action] >= 0) {
        btn.total_time = g_action_time[(int)action];
    }
    wcsncpy_s(btn.label, 32, label, _TRUNCATE);
    return btn;
}

static inline void start_countdown(Button* b, DWORD now) {
    if (b->total_time > 0) {
        b->countdown = b->total_time;
        b->end_tick = now + b->total_time * 1000;
    }
}

// 处理单个按钮倒计时结束后的行为（从 update_button_countdown 调用）
static void handle_button_completion(Button* buttons, int idx, int count) {
    // buttons[idx] 刚刚从倒计时运行态变为结束态
    Button* b = &buttons[idx];

    // 根据 action 作后续处理
    switch (b->action) {
    default:
        break;
    }
}

// 修改：update_button_countdown 接收 GameState*
void update_button_countdown(Button* buttons, int count, DWORD now) {
    for (int i = 0; i < count; i++) {
        if (buttons[i].active && buttons[i].countdown > 0) {
            int prev = buttons[i].countdown;
            int remaining =
                (int)((buttons[i].end_tick > now)
                          ? ((buttons[i].end_tick - now + 999) / 1000)
                          : 0);
            if (remaining != prev) {
                buttons[i].countdown = remaining > 0 ? remaining : 0;
                if (prev > 0 && remaining <= 0) {
                    // 倒计时结束，调用完成处理
                    handle_button_completion(buttons, i, count);
                }
            }
        }
    }
}

static int fk_pre_factory_flg = 0;
static void handle_pre_factory() {
    Item* food = inv_get_item(ITEM_FOOD);
    Item* water = inv_get_item(ITEM_WATER);

    if (!fk_pre_factory_flg && food && food->count >= 20 && water &&
        water->count >= 20) {
        add_story(get_story_text(STORY_PREPARE_FACTORY));

        Button glass = new_button(L"生产玻璃", ACT_GLASS, 15);
        scene_add_button(0, &glass);
        inv_add_new_item(ITEM_GLASS, 0);

        Button iron = new_button(L"冶铁", ACT_IRON, 20);
        scene_add_button(0, &iron);
        inv_add_new_item(ITEM_IRON, 0);

        fk_pre_factory_flg = 1;
    }
}

static void add_fac_button(const wchar_t* label, ButtonAction action1,
                           ButtonAction action2, ButtonAction action3,
                           ItemType item_idx) {
    Button fac = new_button(label, action1, 0);
    scene_add_button(0, &fac);

    wchar_t full_label[64];
    swprintf_s(full_label, 64, L"启用一个%s", label);

    Button start_fac = new_button(full_label, action2, 0);
    start_fac.is_st = 1;
    start_fac.item_idx = item_idx;
    scene_add_button(1, &start_fac);

    swprintf_s(full_label, 64, L"暂停一个%s", label);

    Button stop_fac = new_button(full_label, action3, 0);
    stop_fac.is_st = 2;
    stop_fac.item_idx = item_idx;
    scene_add_button(1, &stop_fac);
}

static int fk_wood_fac_flg = 0;
static void handle_pre_wood_fac(DWORD now) {
    Item* glass = inv_get_item(ITEM_GLASS);
    Item* iron = inv_get_item(ITEM_IRON);

    if (!fk_wood_fac_flg && glass && glass->count >= 20 && iron &&
        iron->count >= 10) {
        add_story(get_story_text(STORY_PRE_WOOD_FAC));

        scene_add_scene(L"工厂");

        Button fac = new_button(L"伐木场", ACT_WOOD_FAC, 0);
        scene_add_button(0, &fac);
        inv_add_new_fac(ITEM_WOOD_FAC, 0, now);

        fk_wood_fac_flg = 1;
    }
}

static int fk_pre_fish_count = 0;

static int fk_pre_water_fac_flg = 0;

void handle_button_click(Button* buttons, int count, int idx, DWORD now) {
    if (idx < 0 || idx >= count)
        return;
    Button* b = &buttons[idx];

    switch (b->action) {
    case ACT_WOOD: {
        ++fk_pre_fish_count;

        Item* wood = inv_get_item(ITEM_WOOD);
        wood->count += 10;

        if (fk_pre_fish_count == 2) {
            add_story(get_story_text(STORY_TALK));

            Button fishing = new_button(L"捕鱼", ACT_FISH, 10);
            scene_add_button(0, &fishing);
            inv_add_new_item(ITEM_FOOD, 0);

            Button water = new_button(L"获取淡水", ACT_WATER, 10);
            scene_add_button(0, &water);
            inv_add_new_item(ITEM_WATER, 0);
        }

        handle_pre_factory();

        start_countdown(b, now);
        break;
    }

    case ACT_FISH: {
        Item* food = inv_get_item(ITEM_FOOD);
        food->count += 10;

        handle_pre_factory();

        start_countdown(b, now);
        break;
    }

    case ACT_WATER: {
        Item* water = inv_get_item(ITEM_WATER);
        water->count += 10;

        handle_pre_factory();

        start_countdown(b, now);
        break;
    }

    case ACT_GLASS: {
        Item* glass = inv_get_item(ITEM_GLASS);
        glass->count += 5;

        handle_pre_wood_fac(now);

        start_countdown(b, now);
        break;
    }

    case ACT_IRON: {
        Item* iron = inv_get_item(ITEM_IRON);
        iron->count += 5;

        handle_pre_wood_fac(now);

        start_countdown(b, now);
        break;
    }

    case ACT_WOOD_FAC: {
        Item* food = inv_get_item(ITEM_FOOD);
        Item* wood = inv_get_item(ITEM_WOOD);
        Item* iron = inv_get_item(ITEM_IRON);
        Item* fac = inv_get_item(ITEM_WOOD_FAC);

        if (food && food->count >= 20 && wood && wood->count >= 20 && iron &&
            iron->count >= 10) {
            food->count -= 20;
            wood->count -= 20;
            iron->count -= 10;
            fac->count += 1;
            fac->active_count += 1;

            if (!fk_pre_water_fac_flg) {
                add_story(get_story_text(STORY_PRE_WATER_FAC));

                add_fac_button(L"水厂", ACT_WATER_FAC, ACT_START_WATER_FAC,
                               ACT_STOP_WATER_FAC, ITEM_WATER_FAC);

                inv_add_new_fac(ITEM_WATER_FAC, 0, now);

                fk_pre_water_fac_flg = 1;
            }
        } else {
            add_story(get_story_text(STORY_WOOD_FAC_FAILED));
        }

        break;
    }

    case ACT_WATER_FAC: {
        Item* food = inv_get_item(ITEM_FOOD);
        Item* wood = inv_get_item(ITEM_WOOD);
        Item* iron = inv_get_item(ITEM_IRON);
        Item* fac = inv_get_item(ITEM_WATER_FAC);

        if (food && food->count >= 20 && wood && wood->count >= 20 && iron &&
            iron->count >= 10) {
            food->count -= 20;
            wood->count -= 20;
            iron->count -= 10;
            fac->count += 1;
            fac->active_count += 1;
        } else {
            add_story(get_story_text(STORY_WATER_FAC_FAILED));
        }

        break;
    }

    case ACT_START_WATER_FAC: {
        Item* fac = inv_get_item(ITEM_WATER_FAC);
        if (fac->count > fac->active_count) {
            fac->active_count += 1;
        }
        break;
    }

    case ACT_STOP_WATER_FAC: {
        Item* fac = inv_get_item(ITEM_WATER_FAC);
        if (fac->active_count > 0) {
            fac->active_count -= 1;
        }
        break;
    }

    default:
        add_story(L"此按钮尚未实现行为。");
        break;
    }
}
