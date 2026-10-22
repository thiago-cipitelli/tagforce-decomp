// @range 0x08805F10-0x088065B0
/* The two lesson lists (tab 0: lessons 8..23, tab 1: basics 0..7) built from the cleared-lesson
   flags, with each title's height in rows, and the per-frame input: tabs, scrolling, choose, cancel. */
#include "libtutoriallist.h"

/* lesson at row i of a tab, TL_NONE past the end */
#define TL_ENTRY(tab, i) ((i) >= tl.count[tab] ? TL_NONE : tl.list[tab][i])

/* lists the cleared lessons (sysstate byte 2), measures their titles and scrolls to the preselected one */
void tlist_build(void)
{
    int sel_tab = 0;
    int sel_pos = 0;
    int line_h;
    int i;
    int pos;
    int top;
    int next_h;
    int rows;
    int j;
    int h;

    font_set_size(14, 14);
    line_h = font_line_height();
    for (i = 0; i < TL_LESSONS; i++) {
        if (sysstate_get_byte_97d0(i) == 2) {
            if (i >= 8) {
                tl.list[0][tl.count[0]++] = i;
                if (i == tl.selected) {
                    sel_tab = 0;
                    sel_pos = tl.count[0] - 1;
                }
            } else {
                tl.list[1][tl.count[1]++] = i;
                if (i == tl.selected) {
                    sel_tab = 1;
                    sel_pos = tl.count[1] - 1;
                }
            }
            tl.lines[i] = (font_wtext_lines(0x4400, table_get_entry(tlres_get(0x1020), i + 4)) + 64) / line_h;
        }
    }
    tl.lines[TL_NONE] = 1;
    tl.tab = sel_tab;
    while (sel_pos != tl.top[tl.tab] + tl.cursor[tl.tab]) {
        if (tl.top[tl.tab] + tl.cursor[tl.tab] >= tl.count[tl.tab] - 1)
            break;
        pos = tl.top[tl.tab] + tl.cursor[tl.tab];
        next_h = tl.lines[TL_ENTRY(tl.tab, pos + 1)];
        top = tl.top[tl.tab];
        rows = 0;
        for (j = 0; j < TL_ROWS; j++) {
            h = tl.lines[TL_ENTRY(tl.tab, top + j)];
            if (j > tl.cursor[tl.tab])
                break;
            rows += h;
        }
        if (rows + next_h > 8)
            tl.top[tl.tab]++;
        else
            tl.cursor[tl.tab]++;
    }
}

void tlist_input(void)
{
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    int tab;
    int pos;
    int top;
    int next_h;
    int rows;
    int i;
    int h;

    if (trig & region_pick_value(1)) {         /* cancel (circle/cross by region) */
        tl.cancelled = 1;
        fade_start_in(1, 0xFF000000, 16);
        bgm_cmd(600);
        tl.state = 3;
        se_play(0x6214);
        return;
    }
    if (trig & region_pick_value(0)) {         /* confirm */
        tab = tl.tab;
        pos = tl.top[tab] + tl.cursor[tab];
        if (TL_ENTRY(tab, pos) < TL_NONE) {
            tl.selected = TL_ENTRY(tab, pos);
            fade_start_in(1, 0xFF000000, 16);
            if (tl.selected >= 8)
                bgm_cmd(600);
            tl.state = 3;
            se_play(0x6213);
        }
    } else if (trig & 0x83A0) {                /* left/right/L/R/square: other tab */
        tl.tab = (tl.tab + 1) & 1;
        se_play(0x621B);
    } else if (pad->b.rep & 0x10) {            /* up */
        tab = tl.tab;
        if (tl.cursor[tab] > 0) {
            tl.cursor[tab]--;
            se_play(0x6212);
        } else if (tl.top[tab] > 0) {
            tl.top[tab]--;
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x40) {            /* down */
        pos = tl.top[tl.tab] + tl.cursor[tl.tab];
        top = tl.top[tl.tab];
        if (pos < tl.count[tl.tab] - 1) {
            next_h = tl.lines[TL_ENTRY(tl.tab, pos + 1)];
            rows = 0;
            for (i = 0; i < TL_ROWS; i++) {
                h = tl.lines[TL_ENTRY(tl.tab, top + i)];
                if (i > tl.cursor[tl.tab])
                    break;
                rows += h;
            }
            if (rows + next_h > 8)
                tl.top[tl.tab]++;
            else
                tl.cursor[tl.tab]++;
            se_play(0x6212);
        }
    }
}
