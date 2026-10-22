// @range 0x088060A4-0x08806A2C
// @asm 0x088060A4-0x088066B0
// @data 0x0880A5D4-0x0880A7D0
/* Builds the three tabs of the duel record screen from the save (visible items, values, "new"
   flags, text heights), the pad handling (tab switch, scroll, exit) and the item tables (text index
   in strTbl and whether the item is listed at all). */
#include "libc.h"
#include "font.h"
#include "ehsys.h"
#include "game.h"
#include "libduelrecord.h"

typedef struct DrecItem {
    unsigned short visible : 1;
    unsigned short text : 15;
} DrecItem;

#define DREC_SAVE_VALUES 0x00 /* int[0x28]: tab 0 values */
#define DREC_SAVE_NEW0 0xA0   /* tab 0: bit 0 = new */
#define DREC_SAVE_STATE1 0xCC /* tab 1: 0 locked, 1 new, 2 seen */
#define DREC_SAVE_STATE2 0x120 /* tab 2: same */

DrecItem drec_items0[40] = {
    { 1, 0x009 }, { 1, 0x00A }, { 1, 0x00B }, { 1, 0x00C }, { 1, 0x00D }, { 1, 0x00E },
    { 1, 0x00F }, { 1, 0x010 }, { 1, 0x011 }, { 1, 0x012 }, { 1, 0x013 }, { 1, 0x014 },
    { 1, 0x015 }, { 1, 0x016 }, { 1, 0x0EE }, { 1, 0x017 }, { 1, 0x018 }, { 1, 0x019 },
    { 1, 0x01A }, { 1, 0x01B }, { 1, 0x01C }, { 1, 0x01D }, { 1, 0x01E }, { 1, 0x01F },
    { 1, 0x020 }, { 1, 0x021 }, { 1, 0x022 }, { 1, 0x023 }, { 1, 0x024 }, { 0, 0x025 },
    { 1, 0x026 }, { 1, 0x027 }, { 1, 0x028 }, { 1, 0x029 }, { 1, 0x02A }, { 1, 0x02B },
    { 1, 0x105 }, { 1, 0x02C }, { 1, 0x02D }, { 1, 0x02E },
};

DrecItem drec_items1[83] = {
    { 1, 0x02F }, { 1, 0x030 }, { 1, 0x031 }, { 1, 0x032 }, { 1, 0x033 }, { 1, 0x034 },
    { 1, 0x0F0 }, { 1, 0x035 }, { 1, 0x036 }, { 1, 0x037 }, { 1, 0x0F1 }, { 1, 0x038 },
    { 1, 0x039 }, { 1, 0x03A }, { 1, 0x03B }, { 1, 0x03C }, { 1, 0x03D }, { 1, 0x03E },
    { 1, 0x03F }, { 1, 0x040 }, { 1, 0x041 }, { 1, 0x042 }, { 1, 0x043 }, { 1, 0x044 },
    { 1, 0x045 }, { 1, 0x046 }, { 1, 0x047 }, { 1, 0x048 }, { 1, 0x049 }, { 1, 0x04A },
    { 1, 0x04B }, { 1, 0x04C }, { 1, 0x04D }, { 1, 0x04E }, { 1, 0x04F }, { 1, 0x050 },
    { 1, 0x051 }, { 1, 0x052 }, { 1, 0x053 }, { 1, 0x054 }, { 1, 0x055 }, { 1, 0x056 },
    { 1, 0x057 }, { 1, 0x058 }, { 1, 0x059 }, { 1, 0x05A }, { 1, 0x05B }, { 1, 0x05C },
    { 1, 0x05D }, { 1, 0x05E }, { 1, 0x05F }, { 1, 0x106 }, { 1, 0x060 }, { 1, 0x061 },
    { 1, 0x062 }, { 1, 0x063 }, { 1, 0x064 }, { 1, 0x065 }, { 1, 0x066 }, { 1, 0x067 },
    { 1, 0x068 }, { 1, 0x069 }, { 1, 0x06A }, { 1, 0x06B }, { 1, 0x06C }, { 1, 0x06D },
    { 1, 0x06E }, { 1, 0x06F }, { 1, 0x070 }, { 1, 0x107 }, { 1, 0x108 }, { 1, 0x071 },
    { 1, 0x072 }, { 1, 0x073 }, { 1, 0x074 }, { 1, 0x075 }, { 1, 0x076 }, { 1, 0x077 },
    { 1, 0x078 }, { 1, 0x079 }, { 1, 0x109 }, { 1, 0x10A }, { 1, 0x0EF },
};

DrecItem drec_items2[130] = { /* 12 unused zero entries at the end */
    { 1, 0x07A }, { 1, 0x07B }, { 1, 0x07C }, { 1, 0x07D }, { 1, 0x07E }, { 1, 0x07F },
    { 1, 0x080 }, { 1, 0x081 }, { 1, 0x082 }, { 1, 0x083 }, { 1, 0x084 }, { 1, 0x085 },
    { 1, 0x086 }, { 1, 0x087 }, { 1, 0x088 }, { 1, 0x089 }, { 1, 0x08A }, { 1, 0x08B },
    { 1, 0x08C }, { 1, 0x08D }, { 1, 0x08E }, { 1, 0x08F }, { 1, 0x090 }, { 1, 0x091 },
    { 1, 0x092 }, { 1, 0x093 }, { 1, 0x094 }, { 1, 0x095 }, { 1, 0x096 }, { 1, 0x097 },
    { 1, 0x098 }, { 1, 0x099 }, { 1, 0x09A }, { 1, 0x09B }, { 1, 0x09C }, { 1, 0x09D },
    { 1, 0x09E }, { 1, 0x09F }, { 1, 0x0A0 }, { 1, 0x0A1 }, { 1, 0x0A2 }, { 1, 0x0A3 },
    { 1, 0x0A4 }, { 1, 0x0A5 }, { 1, 0x0A6 }, { 1, 0x0A7 }, { 1, 0x0A8 }, { 1, 0x0A9 },
    { 1, 0x10B }, { 1, 0x0AA }, { 1, 0x0AB }, { 1, 0x0AC }, { 1, 0x0AD }, { 1, 0x0AE },
    { 1, 0x0AF }, { 1, 0x0B0 }, { 1, 0x10C }, { 1, 0x0B1 }, { 1, 0x0B2 }, { 1, 0x0B3 },
    { 1, 0x0B4 }, { 1, 0x0B5 }, { 1, 0x0B6 }, { 1, 0x0B7 }, { 1, 0x0B8 }, { 1, 0x0B9 },
    { 1, 0x0BA }, { 1, 0x0BB }, { 1, 0x0BC }, { 1, 0x0BD }, { 1, 0x0BE }, { 1, 0x0BF },
    { 1, 0x0C0 }, { 1, 0x0C1 }, { 1, 0x0C2 }, { 1, 0x0C3 }, { 1, 0x0C4 }, { 1, 0x0C5 },
    { 1, 0x0C6 }, { 1, 0x0C7 }, { 1, 0x0C8 }, { 1, 0x0C9 }, { 1, 0x0CA }, { 1, 0x0CB },
    { 1, 0x0CC }, { 1, 0x0CD }, { 1, 0x0CE }, { 1, 0x0CF }, { 1, 0x0D0 }, { 1, 0x0D1 },
    { 1, 0x0D2 }, { 1, 0x0D3 }, { 1, 0x0D4 }, { 1, 0x0D5 }, { 1, 0x0D6 }, { 1, 0x0D7 },
    { 1, 0x0D8 }, { 1, 0x0D9 }, { 1, 0x0DA }, { 1, 0x0DB }, { 1, 0x0DC }, { 1, 0x0DD },
    { 1, 0x0DE }, { 1, 0x0DF }, { 1, 0x0E0 }, { 1, 0x0E1 }, { 1, 0x0E2 }, { 1, 0x0E3 },
    { 1, 0x0E4 }, { 1, 0x0E5 }, { 1, 0x0E6 }, { 1, 0x0E7 }, { 1, 0x0E8 }, { 1, 0x0E9 },
    { 1, 0x0EA }, { 1, 0x0EB }, { 1, 0x0EC }, { 0, 0x0ED },
};

#ifdef NON_MATCHING /* forma igual; nas abas 2 e 1 o original troca os registradores do ponteiro da tabela e do contador number (s1/s5, s2/s6), e o addiu do ponteiro sai depois dos zeros (18 palavras) */
void drec_build(void)
{
    int i;
    int total;
    int unlocked;
    int number;
    DrecItem *it;
    DrecEntry *e;

    int line_h;
    int n;
    int k;
    DrecEntry *list0;
    DrecEntry *list1;
    DrecEntry *list2;
    unsigned short *text;

    drec.save = (unsigned char *)sysstate2_get_ptr_51c();
    font_set_size(0xE, 0xE);
    line_h = font_line_height();
    unlocked = 0;
    total = 0;
    number = 0;
    list0 = heap_alloc(drec.heap, 0x28 * sizeof(DrecEntry));
    memset(list0, 0, 0x28 * sizeof(DrecEntry));
    e = list0;
    n = 0;
    for (it = drec_items0, i = 0; i < 0x28; it++, i++) {
        if (it->visible) {
            e->index = i;
            e->number = number;
            total++;
            e->is_new = drec.save[DREC_SAVE_NEW0 + i];
            number++;
            if (e->is_new)
                drec.save[DREC_SAVE_NEW0 + i] = 0;
            e->has_value = 1;
            e->value = ((int *)drec.save)[i];
            e->lines = (font_wtext_lines(0x5280, table_get_entry(drec_get_file(0x1020), drec_text_index(0, i))) + 0x40) / line_h;
            n++;
            unlocked++;
            e++;
        }
    }
    drec.entries[0] = list0;
    drec.count[0] = n;

    list2 = heap_alloc(drec.heap, 0x76 * sizeof(DrecEntry));
    memset(list2, 0, 0x76 * sizeof(DrecEntry));
    e = list2;
    n = 0;
    number = 0;
    for (it = drec_items2, i = 0; i < 0x76; it++, i++) {
        if (it->visible) {
            e->number = number;
            total++;
            number++;
            if (drec.save[DREC_SAVE_STATE2 + i] != 0) {
                e->index = i;
                e->is_new = drec.save[DREC_SAVE_STATE2 + i] == 1;
                if (e->is_new)
                    drec.save[DREC_SAVE_STATE2 + i] = 2;
                e->has_value = 0;
                e->value = 0;
                e->lines = (font_wtext_lines(0x6600, table_get_entry(drec_get_file(0x1020), drec_text_index(2, i))) + 0x40) / line_h;
                unlocked++;
                n++;
                e++;
            }
        }
    }
    drec.entries[2] = list2;
    drec.count[2] = n;

    list1 = heap_alloc(drec.heap, 0x53 * sizeof(DrecEntry));
    memset(list1, 0, 0x53 * sizeof(DrecEntry));
    e = list1;
    n = 0;
    number = 0;
    for (it = drec_items1, i = 0; i < 0x53; it++, i++) {
        if (it->visible) {
            e->number = number;
            total++;
            number++;
            if (drec.save[DREC_SAVE_STATE1 + i] != 0) {
                e->index = i;
                e->is_new = drec.save[DREC_SAVE_STATE1 + i] == 1;
                if (e->is_new)
                    drec.save[DREC_SAVE_STATE1 + i] = 2;
                e->has_value = 0;
                e->value = 0;
                switch (i) {
                case 0: /* decks at level 3 */
                    k = 2;
                    do {
                        if (deck_slot_get_level(k) == 3)
                            e->value++;
                        k++;
                    } while (k < 0x98);
                    e->has_value = 1;
                    break;
                case 1:
                    e->value = day_next();
                    e->has_value = 1;
                    break;
                case 2: /* unlocked items of tab 2 */
                    e->value = drec.count[2];
                    e->has_value = 1;
                    break;
                }
                text = table_get_entry(drec_get_file(0x1020), drec_text_index(1, i));
                if (e->has_value)
                    k = font_wtext_lines(0x5280, text);
                else
                    k = font_wtext_lines(0x6600, text);
                n++;
                unlocked++;
                e->lines = (k + 0x40) / line_h;
                e++;
            }
        }
    }
    drec.entries[1] = list1;
    drec.count[1] = n;

    drec.n_tabs = 3;
    if (!sysflag_test(7))
        drec.n_tabs--;
    drec.percent = unlocked * 100 / total;
}

#endif

void drec_input(void)
{
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    int rows;

    if (trig & 0x6000) {
        fade_start_in(1, 0xFF000000, 16);
        drec.state = 3;
        se_play(0x6214);
    } else if (trig & 0x180) {
        drec.tab = (drec.tab + drec.n_tabs - 1) % drec.n_tabs;
        se_play(0x621B);
    } else if (trig & 0x8220) {
        drec.tab = (drec.tab + 1) % drec.n_tabs;
        se_play(0x621B);
    } else if (pad->b.rep & 0x10) {
        int t = drec.tab;

        if (drec.scroll[t] > 0) {
            drec.scroll[t]--;
            se_play(0x6212);
        } else if ((trig & 0x10) && drec_tab_rows(t) > 0) {
            drec.scroll[drec.tab] = drec_tab_rows(drec.tab);
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x40) {
        rows = drec_tab_rows(drec.tab);
        if (drec.scroll[drec.tab] < rows) {
            drec.scroll[drec.tab]++;
            se_play(0x6212);
        } else if ((pad->b.trig & 0x40) && rows > 0) {
            drec.scroll[drec.tab] = 0;
            se_play(0x6212);
        }
    }
}

unsigned short drec_text_index(unsigned short tab, unsigned short i)
{
    switch (tab) {
    case 0:
        if (i < 0x28)
            return drec_items0[i].text;
        break;
    case 1:
        if (i < 0x53)
            return drec_items1[i].text;
        break;
    case 2:
        if (i < 0x75)
            return drec_items2[i].text;
        break;
    }
    return 0;
}

/* largest scroll position: first entry of the last page (9 lines) */
int drec_tab_rows(tab)
unsigned short tab;
{
    int n = drec.count[tab];
    int i;
    int lines;

    if (n <= 0)
        return 0;
    i = 0;
    lines = 0;
    do {
        lines += drec_entry(tab, n - i - 1)->lines;
        if (n - 1 <= i)
            return 0;
        i++;
    } while (lines < 9);
    return n - i;
}
