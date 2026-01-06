#include "button.h"
#include "scene.h"
#include "story.h"
#include <stdio.h>

// 记录从配置文件加载的各按钮的倒计时
static int g_action_time[32];
static void init_action_time_map(void) {
    for (int i = 0; i < 32; i++)
        g_action_time[i] = -1;
}

// 按按钮名映射到枚举
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
    if (strcmp(s, "ACT_SHIP_FAC") == 0)
        return ACT_SHIP_FAC;
    if (strcmp(s, "ACT_START_SHIP_FAC") == 0)
        return ACT_START_SHIP_FAC;
    if (strcmp(s, "ACT_STOP_SHIP_FAC") == 0)
        return ACT_STOP_SHIP_FAC;
    if (strcmp(s, "ACT_SHIP") == 0)
        return ACT_SHIP;

    if (strcmp(s, "ACT_SAIL") == 0)
        return ACT_SAIL;
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
        // 去掉尾部空白
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

// 新增按钮。如果配置中为该 action 指定了时间，则覆盖传入的 total_time
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
        .fac_id = -1,
    };
    // 若配置中存在指定时间，覆盖
    if (action >= 0 && action < 32 && g_action_time[(int)action] >= 0) {
        btn.total_time = g_action_time[(int)action];
    }
    wprintf(L"%s button created with total_time=%d\n", label, btn.total_time);
    wcsncpy_s(btn.label, 64, label, _TRUNCATE);
    return btn;
}

// 启动按钮倒计时
static inline void start_countdown(Button* b, DWORD now) {
    if (b->total_time > 0) {
        b->countdown = b->total_time;
        b->end_tick = now + b->total_time * 1000;
    }
}

// 处理单个按钮倒计时结束后的行为，本来有用的，现在没用了
static void handle_button_completion(Button* buttons, int idx, int count) {
    Button* b = &buttons[idx];

    // 根据 action 作后续处理
    switch (b->action) {
    default:
        break;
    }
}

// 更新按钮倒计时
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

// 解锁玻璃厂和冶炼厂按钮
static int fk_pre_factory_flg = 0;
static void handle_pre_factory() {
    Item* food = inv_get_item(ITEM_FOOD);
    Item* water = inv_get_item(ITEM_WATER);

    if (!fk_pre_factory_flg && food && food->count >= 20 && water &&
        water->count >= 20) {
        add_story(get_story_text(STORY_PREPARE_FACTORY));

        Button glass = new_button(L"生产玻璃", ACT_GLASS, 15);
        scene_add_button(0, &glass);
        activate_item(ITEM_GLASS);

        Button iron = new_button(L"冶铁", ACT_IRON, 20);
        scene_add_button(0, &iron);
        activate_item(ITEM_IRON);

        fk_pre_factory_flg = 1;
    }
}

// 添加工厂按钮及其启动/暂停按钮
static void add_fac_button(const wchar_t* label, ButtonAction action1,
                           ButtonAction action2, ButtonAction action3,
                           FactoryType fac_id) {
    Button btn = new_button(label, action1, 0);
    btn.fac_id = fac_id;
    scene_add_button(0, &btn);

    wchar_t full_label[64];
    swprintf_s(full_label, 64, L"启用一个%s", label);

    Button start_fac = new_button(full_label, action2, 0);
    start_fac.is_st = 1;
    start_fac.fac_id = fac_id;
    scene_add_button(1, &start_fac);

    swprintf_s(full_label, 64, L"暂停一个%s", label);

    Button stop_fac = new_button(full_label, action3, 0);
    stop_fac.is_st = 2;
    stop_fac.fac_id = fac_id;
    scene_add_button(1, &stop_fac);
}

// 解锁伐木场按钮
static int fk_wood_fac_flg = 0;
static void handle_pre_wood_fac(DWORD now) {
    Item* glass = inv_get_item(ITEM_GLASS);
    Item* iron = inv_get_item(ITEM_IRON);

    if (!fk_wood_fac_flg && glass && glass->count >= 20 && iron &&
        iron->count >= 10) {
        add_story(get_story_text(STORY_PRE_WOOD_FAC));

        scene_add_scene(L"工厂");

        // 伐木场因为不消耗资源所以不用加启动/暂停按钮
        Button btn = new_button(L"伐木场", ACT_WOOD_FAC, 0);
        btn.fac_id = FAC_WOOD;
        scene_add_button(0, &btn);
        activate_fac(FAC_WOOD, now);

        fk_wood_fac_flg = 1;
    }
}

// 各种标记
static int fk_pre_fish_count = 0;

static int fk_pre_water_fac_flg = 0;
static int fk_pre_iron_glass_fac_flg = 0;
static int fk_pre_food_fac_flg = 0;
static int fk_pre_ship_fac_flg = 0;

static int fk_ship_btn = 0;
static int fk_sail_btn = 0;

// 工厂启动/暂停处理宏，简化代码
#define FAC_ST(NAME)                                                           \
    case ACT_START_##NAME##_FAC: {                                             \
        Factory* fac = inv_get_fac(FAC_##NAME);                                \
        if (fac->count > fac->active_count) {                                  \
            fac_add_active_count(fac, +1);                                     \
        }                                                                      \
        break;                                                                 \
    }                                                                          \
                                                                               \
    case ACT_STOP_##NAME##_FAC: {                                              \
        Factory* fac = inv_get_fac(FAC_##NAME);                                \
        if (fac->active_count > 0) {                                           \
            fac_add_active_count(fac, -1);                                     \
        }                                                                      \
        break;                                                                 \
    }

// Beta分布的近似计算函数（简化处理）
double calc_success_rate(double a1, double b1, double rate, int num) {
    double a = a1 + rate * num;
    double b = b1;
    // 根据Beta分布的性质，计算成功概率（这里简化处理为直接使用Beta分布的均值作为概率）
    return a / (a + b);
}

// 判断是否返航成功
int is_success(double success_rate) {
    return (double)rand() < success_rate * RAND_MAX;
}

// 水声资料收集完成度
static double finish_rate = 0;

// 处理按钮点击事件
void handle_button_click(Button* buttons, int count, int idx, DWORD now) {
    if (idx < 0 || idx >= count)
        return;

    Button* b = &buttons[idx];

    switch (b->action) {
    case ACT_WOOD: {
        ++fk_pre_fish_count;

        Item* wood = inv_get_item(ITEM_WOOD);
        wood->count += 10;

        // 点击两次伐木后解锁捕鱼和取水按钮
        if (fk_pre_fish_count == 2) {
            add_story(get_story_text(STORY_TALK));

            Button fishing = new_button(L"捕鱼", ACT_FISH, 10);
            scene_add_button(0, &fishing);
            activate_item(ITEM_FOOD);

            Button water = new_button(L"获取淡水", ACT_WATER, 10);
            scene_add_button(0, &water);
            activate_item(ITEM_WATER);
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
        if (build_fac(FAC_WOOD)) {
            // 伐木场建造失败
            add_story(get_story_text(STORY_WOOD_FAC_FAILED));
        } else {
            // 伐木场建造成功，解锁水厂
            if (!fk_pre_water_fac_flg) {
                add_story(get_story_text(STORY_PRE_WATER_FAC));

                add_fac_button(L"水厂", ACT_WATER_FAC, ACT_START_WATER_FAC,
                               ACT_STOP_WATER_FAC, FAC_WATER);

                activate_fac(FAC_WATER, now);

                fk_pre_water_fac_flg = 1;
            }
        }

        break;
    }

    case ACT_WATER_FAC: {
        if (build_fac(FAC_WATER)) {
            // 水厂建造失败
            add_story(get_story_text(STORY_WATER_FAC_FAILED));
        } else {
            // 水厂建造成功，解锁玻璃厂和冶炼厂
            if (!fk_pre_iron_glass_fac_flg) {
                add_story(get_story_text(STORY_PRE_IRON_GLASS_FAC));

                add_fac_button(L"冶炼厂", ACT_IRON_FAC, ACT_START_IRON_FAC,
                               ACT_STOP_IRON_FAC, FAC_IRON);

                activate_fac(FAC_IRON, now);

                add_fac_button(L"玻璃厂", ACT_GLASS_FAC, ACT_START_GLASS_FAC,
                               ACT_STOP_GLASS_FAC, FAC_GLASS);

                activate_fac(FAC_GLASS, now);

                fk_pre_iron_glass_fac_flg = 1;
            }
        }

        break;
    }

        FAC_ST(WATER)

    case ACT_IRON_FAC: {
        if (build_fac(FAC_IRON)) {
            // 冶炼厂建造失败
            add_story(get_story_text(STORY_IRON_FAC_FAILED));
        }

        break;
    }

        FAC_ST(IRON)

    case ACT_GLASS_FAC: {
        if (build_fac(FAC_GLASS)) {
            // 玻璃厂建造失败
            add_story(get_story_text(STORY_GLASS_FAC_FAILED));
        } else {
            // 玻璃厂建造成功，解锁食品加工厂
            if (!fk_pre_food_fac_flg) {
                add_story(get_story_text(STORY_PRE_FOOD_FAC));

                add_fac_button(L"食品加工厂", ACT_FOOD_FAC, ACT_START_FOOD_FAC,
                               ACT_STOP_FOOD_FAC, FAC_FOOD);

                activate_fac(FAC_FOOD, now);

                fk_pre_food_fac_flg = 1;
            }
        }
        break;
    }

        FAC_ST(GLASS)

    case ACT_FOOD_FAC: {
        if (build_fac(FAC_FOOD)) {
            // 食品加工厂建造失败
            add_story(get_story_text(STORY_FOOD_FAC_FAILED));
        } else {
            if (!fk_pre_ship_fac_flg) {
                // 食品加工厂建造成功，解锁造船厂
                add_story(get_story_text(STORY_PRE_SHIP_FAC));

                add_fac_button(L"造船厂", ACT_SHIP_FAC, ACT_START_SHIP_FAC,
                               ACT_STOP_SHIP_FAC, FAC_SHIP);

                activate_fac(FAC_SHIP, now);

                fk_pre_ship_fac_flg = 1;
            }
        }

        break;
    }

        FAC_ST(FOOD)

    case ACT_SHIP_FAC: {
        if (build_fac(FAC_SHIP)) {
            // 造船厂建造失败
            add_story(get_story_text(STORY_SHIP_FAC_FAILED));
        } else {
            // 造船厂建成后可解锁造船按钮
            if (!fk_ship_btn) {
                add_story(get_story_text(STORY_SHIP_FAC_SUCCEEDED));

                Button ship = new_button(L"造船", ACT_SHIP, 30);
                scene_add_button(0, &ship);
                activate_item(ITEM_SHIP);

                fk_ship_btn = 1;
            }
        }

        break;
    }

        FAC_ST(SHIP)

    case ACT_SHIP: {
        int fac_cnt = inv_get_fac(FAC_SHIP)->active_count;

        // 没有已启用的造船厂
        if (fac_cnt == 0) {
            add_story(get_story_text(STORY_NO_ACTIVE_SHIP_FAC));
            break;
        }

        Item* ship = inv_get_item(ITEM_SHIP);
        Item* wood = inv_get_item(ITEM_WOOD);
        Item* glass = inv_get_item(ITEM_GLASS);
        Item* iron = inv_get_item(ITEM_IRON);

        int wood_needed = fac_cnt * 20;
        int glass_needed = fac_cnt * 20;
        int iron_needed = fac_cnt * 40;

        // 材料不足
        if (wood->count < wood_needed || glass->count < glass_needed ||
            iron->count < iron_needed) {
            add_story(get_story_text(STORY_SHIP_FAILED));

            wchar_t txt[256];
            swprintf_s(txt, 256, L"造船需要材料：木头 %d，玻璃 %d，钢铁 %d。",
                       wood_needed, glass_needed, iron_needed);
            add_story(txt);
            break;
        }

        ship->count += fac_cnt;
        wood->count -= wood_needed;
        glass->count -= glass_needed;
        iron->count -= iron_needed;

        if (!fk_sail_btn) {
            // 解锁出航按钮
            Button sail = new_button(L"出航", ACT_SAIL, 0);
            scene_add_button(0, &sail);
            fk_sail_btn = 1;

            add_story(get_story_text(STORY_SAIL_NOTICE));
        }

        wchar_t txt[256];
        swprintf_s(txt, 256, L"当前返航成功率: %.2f%%。",
                   calc_success_rate(1.0, 2.0, 0.7, ship->count) * 100.0);
        add_story(txt);

        start_countdown(b, now);
        break;
    }

    case ACT_SAIL: {
        Item* ship = inv_get_item(ITEM_SHIP);
        Item* food = inv_get_item(ITEM_FOOD);
        Item* water = inv_get_item(ITEM_WATER);

        int ship_consumed = ship->count;
        int food_needed = 40 * ship_consumed;
        int water_needed = 40 * ship_consumed;

        if (ship->count < 1 || food->count < food_needed ||
            water->count < water_needed) {
            wchar_t txt[256];
            swprintf_s(txt, 256, L"出航需要：食物 %d，淡水 %d。", food_needed,
                       water_needed);
            add_story(txt);

            add_story(get_story_text(STORY_CANNOT_SAIL));
        } else {
            add_story(get_story_text(STORY_SAIL));

            food->count -= food_needed;
            water->count -= water_needed;

            // 此处简化为用概率计算  TODO: 航海冒险
            if (is_success(calc_success_rate(1.0, 2.0, 0.7, ship_consumed))) {
                add_story(get_story_text(STORY_SAIL_SUCCEEDED));

                finish_rate +=
                    calc_success_rate(0.1, 10.0, 0.7, ship_consumed) * 100.0;

                finish_rate = (finish_rate > 100.0 ? 100.0 : finish_rate);

                wchar_t txt[256];
                swprintf_s(txt, 256, L"水声资料完成度为 %.2f%%。", finish_rate);

                add_story(txt);

                if (finish_rate >= 100.0) {
                    add_story(get_story_text(STORY_GAME_WON));
                }
            } else {
                ship->count -= ship_consumed;
                add_story(get_story_text(STORY_SAIL_FAILED));
            }
        }

        start_countdown(b, now);
        break;
    }

    default:
        add_story(L"此按钮尚未实现行为。");
        break;
    }
}
