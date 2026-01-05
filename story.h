#pragma once

#include <wchar.h>

// 剧情文本枚举，以 STORY_ 开头
enum StoryText {
    STORY_INTRO = 0,
    STORY_TALK,
    STORY_PREPARE_FACTORY,
    STORY_PRE_WOOD_FAC,
    STORY_WOOD_FAC_FAILED,
    STORY_PRE_WATER_FAC,
    STORY_WATER_FAC_FAILED,
    STORY_PRE_IRON_GLASS_FAC,
    STORY_IRON_FAC_FAILED,
    STORY_GLASS_FAC_FAILED,
    STORY_PRE_FOOD_FAC,
    STORY_FOOD_FAC_FAILED,
    STORY_PRE_SHIP_FAC,
    STORY_SHIP_FAC_FAILED,
    STORY_SHIP_FAC_SUCCEEDED,
    STORY_NO_ACTIVE_SHIP_FAC,
    STORY_SHIP_FAILED,

    STORY_SAIL,
    STORY_SAIL_NOTICE,
    STORY_CANNOT_SAIL,
    STORY_SAIL_FAILED,
    STORY_SAIL_SUCCEEDED,
    STORY_GAME_WON,
};

const wchar_t* get_story_text(int idx);

#define MAX_LINE 100

void set_story_width(int width);

void add_story(const wchar_t* text);

const wchar_t* get_story_line(int idx);
int get_story_line_count(void);
