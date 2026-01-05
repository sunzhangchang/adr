#include "story.h"

// 剧情文本  TODO: 可考虑改为从外部文件加载以便扩展
static const wchar_t story_text[][256] = {
    L"你被派遣去发展航海事业。",
    L"你听到工人们的讨论声，原来是食物和淡水的库存不多了。",
    L"是时候获取材料来准备建造工厂了。",
    L"你决定从伐木场开始。",
    L"你没有足够的材料来建造伐木场。",
    L"下一步是建造水厂。",
    L"你没有足够的材料来建造水厂。",
    L"你准备建造冶铁厂和玻璃厂。",
    L"你没有足够的材料来建造冶铁厂。",
    L"你没有足够的材料来建造玻璃厂。",
    L"食物是航海必备的物资，你决定建造食品加工厂。",
    L"你没有足够的材料来建造食品加工厂。",
    L"做好了充分的准备，是时候该建造造船厂，准备造船出航。",
    L"你没有足够的材料来建造造船厂。",
    L"造船厂建成了！你现在可以造船出航了。",
    L"目前没有已启用的造船厂，无法造船。",
    L"你没有足够的材料来造船。你可以收集足够的资源后再建造，或者尝试减少已启用"
    L"的造船厂数量。",

    L"扬帆起航！",
    L"随着你的船只数量增加，出航携带的食物和淡水越多，返航成功率越大，返航成功时带回水声资料越多。",
    L"你没有足够的物资来出航。确保你至少有一艘船和足够的食物与淡水。",
    L"在航行途中遇到风暴，所有船只均已损毁。",
    L"你的舰队顺利返航。",
    L"你已经收集齐全所有水声资料，游戏胜利！"};

// 根据 index 获取剧情文本
const wchar_t* get_story_text(int idx) {
    // 判断是否越界
    return (idx < 0 || idx >= sizeof(story_text) / sizeof(story_text[0]))
               ? L""
               : story_text[idx];
}

// 循环队列存放剧情
static int hd = 0, tl = 0;
static wchar_t lines[MAX_LINE][256];

// 记录剧情显示的最大宽度（用于换行）
static int g_story_width = 50; // 默认值

void set_story_width(int width) { g_story_width = width; }

// 判断是否为宽字符（判断 CJK 范围）
int is_wide_char(wchar_t c) {
    return (c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0xA4CF) ||
           (c >= 0xAC00 && c <= 0xD7A3) || (c >= 0xF900 && c <= 0xFAFF) ||
           (c >= 0xFE10 && c <= 0xFE19) || (c >= 0xFE30 && c <= 0xFE6F) ||
           (c >= 0xFF00 && c <= 0xFF60) || (c >= 0xFFE0 && c <= 0xFFE6) ||
           (c >= 0x3000 && c <= 0x303F);
}

// 将文本按最大宽度换行，返回行数
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

// 在剧情队列中添加文本
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

// 获取某一行剧情文本
const wchar_t* get_story_line(int idx) {
    return (idx < 0 || idx >= get_story_line_count())
               ? NULL
               : lines[(hd + idx) % MAX_LINE];
}

// 获取队列中的剧情行数
int get_story_line_count(void) {
    return (tl >= hd) ? tl - hd : MAX_LINE - hd + tl;
}
