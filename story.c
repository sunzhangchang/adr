#include "story.h"

static const wchar_t story_text[][256] = {
    L"你被派遣去发展航海事业。",
    L"你听到工人们的讨论声，原来是食物和淡水的库存不多了。",
    L"是时候获取材料来准备建造工厂了。",
    L"你决定从伐木场开始。",
    L"你没有足够的材料来建造伐木场。",
    L"下一步是建造水厂。",
    L"你没有足够的材料来建造水厂。",
    L"冶铁厂。",
    L"玻璃厂。",
    L"你没有足够的材料来建造冶铁厂。",
    L"你没有足够的材料来建造玻璃厂。",
    L"食品加工厂。",
    L"你没有足够的材料来建造食品加工厂。"};

const wchar_t* get_story_text(int idx) {
    return (idx < 0 || idx >= sizeof(story_text) / sizeof(story_text[0]))
               ? L""
               : story_text[idx];
}

// 循环队列存放剧情
static int hd = 0, tl = 0;
static wchar_t lines[MAX_LINE][256];

// 全局变量：记录剧情显示的最大宽度（用于换行）
static int g_story_width = 50; // 默认值

void set_story_width(int width) { g_story_width = width; }

int is_wide_char(wchar_t c) {
    return (c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0xA4CF) ||
           (c >= 0xAC00 && c <= 0xD7A3) || (c >= 0xF900 && c <= 0xFAFF) ||
           (c >= 0xFE10 && c <= 0xFE19) || (c >= 0xFE30 && c <= 0xFE6F) ||
           (c >= 0xFF00 && c <= 0xFF60) || (c >= 0xFFE0 && c <= 0xFFE6) ||
           (c >= 0x3000 && c <= 0x303F);
}

int wrap_text(const wchar_t* text, int maxWidth, wchar_t lines[][256],
              int maxLines) {
    int lineCount = 0;
    const wchar_t* p = text;
    while (*p && lineCount < maxLines) {
        wchar_t buf[256] = {0};
        const wchar_t* start = p;
        const wchar_t* lastSpace = NULL;
        int disp = 0;
        int copied = 0;
        while (*p) {
            if (*p == L'\n') {
                p++;
                break;
            }
            int w = is_wide_char(*p) ? 2 : 1;
            if (disp + w > maxWidth)
                break;
            if (*p == L' ' || *p == L'\t')
                lastSpace = p;
            if (copied < 255)
                buf[copied++] = *p;
            disp += w;
            p++;
        }
        if (*p && disp >= maxWidth && lastSpace != NULL && lastSpace > start) {
            int copyLen = (int)(lastSpace - start);
            if (copyLen > 255)
                copyLen = 255;
            wcsncpy_s(buf, 256, start, copyLen);
            buf[copyLen] = 0;
            p = lastSpace + 1;
        }
        while (wcslen(buf) > 0 && buf[wcslen(buf) - 1] == L' ')
            buf[wcslen(buf) - 1] = 0;
        wcscpy_s(lines[lineCount++], 256, buf);
        if (p == start)
            break;
    }
    wcscpy_s(lines[lineCount++], 256, L"");
    return lineCount;
}

void add_story(const wchar_t* text) {
    // 使用 wrap_text 对文本进行换行处理
    wchar_t wrapped[MAX_LINE][256];
    int wrapped_cnt = wrap_text(text, g_story_width, wrapped, MAX_LINE);
    if (wrapped_cnt <= 0)
        return;

    for (int i = wrapped_cnt - 1; i >= 0; --i) {
        wcscpy_s(lines[tl], 256, wrapped[i]);
        tl = (tl + 1) % MAX_LINE;
        if (tl == hd) {
            hd = (hd + 1) % MAX_LINE;
        }
    }
}

const wchar_t* get_story_line(int idx) {
    return (idx < 0 || idx >= get_story_line_count())
               ? NULL
               : lines[(hd + idx) % MAX_LINE];
}

int get_story_line_count(void) {
    return (tl >= hd) ? tl - hd : MAX_LINE - hd + tl;
}
