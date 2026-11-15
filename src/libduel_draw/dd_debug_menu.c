// @range 0x088589FC-0x08859CC0
// @rodata 0x08870E90-0x08870EC8
// @data 0x08877880-0x08877978
/* Debug test menu of the duel scene (mode 12 of dd_modes; not reachable in the retail game). Seven
   rows, 14 px apart from y = 100: MRK (card number, edited digit by digit), Name (the same card,
   stepped in name order), FaceChange (Up/Down), TurnChange (Attack/Defence), FildMagic (cycles
   the field spell shown on the board), GameSet (Win/Lose/Draw) and Undo. The card goes to the
   zone the menu was opened on (debug_target) through zone_cmd. */
#include "dd_duelui.h"
#include "allegrex.h"

typedef struct DebugMenu {             /* 0x08C5B2D0, 0x28 */
    int card;                          /* +0x00 4007..6957 */
    unsigned short *name;              /* +0x04 */
    int face;                          /* +0x08 bit 14 of the zone_cmd argument */
    int pos;                           /* +0x0C bit 15 */
    int saved_card;                    /* +0x10 values when the row was entered */
    unsigned short *saved_name;        /* +0x14 */
    int saved_face;                    /* +0x18 */
    int saved_pos;                     /* +0x1C */
    int item;                          /* +0x20 row 0..6 */
    int sel;                           /* +0x24 -1 = moving between rows; else the cursor inside the row */
} DebugMenu;

typedef struct DebugTarget {           /* 0x08C5B2F8: where the menu was opened */
    PopupLoc loc;
    int card;                          /* +0x04 = debug_last_card */
} DebugTarget;

typedef struct ZoneArg {               /* second argument of zone_cmd, as in dd_zone_cmd.c */
    unsigned short lo : 14;
    unsigned short big : 1;
    unsigned short huge : 1;
} ZoneArg;

extern DebugMenu debug_menu;
extern DebugTarget debug_target;
extern int debug_last_card;            /* 0x08C5B2FC */

void dv_0883603C(void);

void debug_menu_init(void)
{
    memset(&debug_menu, 0, sizeof(debug_menu));
    debug_menu.sel = -1;
    debug_menu.card = ALLEGREX_MIN(ALLEGREX_MAX(debug_last_card, 4007), 6957);
    debug_menu.name = card_get_name(debug_menu.card);
}

void debug_menu_term(void)
{
    dv_0883603C();
}

void zone_cmd(PopupLoc loc, ZoneArg arg, int value);
void dd_mode_pop(void);
void dd_info_select(int id);
int dd_field_cur_id(void);
int dd_field_set(int id);
unsigned int dv_slot_faceup(int side, int z);
unsigned int dv_slot_pos(int side, int z);
void dv_088C936C(int side, int a, int b, int id, int opp);
int dv_088C9878(int side, int z, int x, int y, int id, int opp);
void dv_088C452C(int side, int n);
unsigned short *wtext_from_int(int v, unsigned short *out);
unsigned short *wcscat(unsigned short *d, const unsigned short *s);

/* UTF-16 captions (0x08877880-0x08877968) */
static unsigned short dbg_s_cursor[] = { '>', 0 };
static unsigned short dbg_s_mrk[] = { 'M', 'R', 'K', 0 };
static unsigned short dbg_s_name[] = { 'N', 'a', 'm', 'e', 0 };
static unsigned short dbg_s_face[] = { 'F', 'a', 'c', 'e', 'C', 'h', 'a', 'n', 'g', 'e', 0 };
static unsigned short dbg_s_up[] = { 'U', 'p', 0 };
static unsigned short dbg_s_down[] = { 'D', 'o', 'w', 'n', 0 };
static unsigned short dbg_s_turn[] = { 'T', 'u', 'r', 'n', 'C', 'h', 'a', 'n', 'g', 'e', 0 };
static unsigned short dbg_s_attack[] = { 'A', 't', 't', 'a', 'c', 'k', 0 };
static unsigned short dbg_s_defence[] = { 'D', 'e', 'f', 'e', 'n', 'c', 'e', 0 };
static unsigned short dbg_s_undo[] = { 'U', 'n', 'd', 'o', 0 };
static unsigned short dbg_s_field[] = { 'F', 'i', 'l', 'd', 'M', 'a', 'g', 'i', 'c', 0 };
static unsigned short dbg_s_gameset[] = { 'G', 'a', 'm', 'e', 'S', 'e', 't', 0 };
static unsigned short dbg_s_result[3][10] = {
    { 'W', 'i', 'n', 0 }, { 'L', 'o', 's', 'e', 0 }, { 'D', 'r', 'a', 'w', 0 }
};

/* the selection bar under the text: a (w >> 6) x 12 rectangle blended over what was drawn */
#define DBG_HILITE(color, x, y, w) \
    do { \
        list = font_text_clear(); \
        list = dl_ambient_color(list, color); \
        list = dl_enable(list, 4); \
        list = dl_blend_func(list, 0, 10, 10, 0xFF2267F2, 0xFFFFFFFF); \
        list = draw_set_offsets(list, 0, 0, (w) >> 6, 12); \
        list = draw_sprite(list, x, y, 0, 0, 0, 0, 0); \
        font_text_set(list); \
    } while (0)

/* 0x08858A64. FaceChange and TurnChange only measure their value (Up/Down, Attack/Defence): the
   original never draws it. */
unsigned int *debug_menu_draw(unsigned int *list)
{
    unsigned short buf[256];
    DebugMenu *m = &debug_menu;
    unsigned int color;
    int div;
    int i;
    int x;
    unsigned short *s;
    unsigned short *name;
    int id;

    list = dl_enable(list, 4);
    list = dl_disable(list, 1);
    list = dl_disable(list, 0);
    list = dl_blend_func(list, 0, 10, 10, 0, 0xFF404040);
    list = draw_set_offsets(list, 0, 0, 200, 100);
    list = draw_sprite(list, 200, 100, 0, 0, 0, 0, 0);
    font_set_size(12, 12);
    font_text_set(list);
    if (*gfx_get_state() >> 2 & 1)
        color = 0xFFFFFFFF;
    else
        color = 0xFF404040;
    if (m->sel < 0) {
        int y = m->item * 14 + 100;

        font_set_color(color);
        font_draw_text(200 << 6, y << 6, 280 << 6, dbg_s_cursor);
        font_set_color(0xFFFFFFFF);
    }
    font_draw_text(216 << 6, 100 << 6, 264 << 6, dbg_s_mrk);
    if (m->item == 0 && m->sel >= 0)
        DBG_HILITE(color, m->sel * 12 + 276, 100, 12 << 6);
    buf[1] = 0;
    for (div = 1000, i = 0, x = 276; i < 4; i++, x += 12, div /= 10) {
        buf[0] = m->card / div % 10 + '0';
        font_draw_text((unsigned int)x << 6, 100 << 6, (480 - x) << 6, buf);
    }
    font_draw_text(216 << 6, 114 << 6, 264 << 6, dbg_s_name);
    if (m->item == 1 && m->sel >= 0) {
        int w = font_measure_text(m->name);

        DBG_HILITE(color, 276, 114, w);
    }
    font_draw_text(276 << 6, 114 << 6, 204 << 6, m->name);
    font_draw_text(216 << 6, 128 << 6, 264 << 6, dbg_s_face);
    s = m->face ? dbg_s_up : dbg_s_down;
    if (m->item == 2 && m->sel >= 0) {
        int w = font_measure_text(s);

        DBG_HILITE(color, 276, 128, w);
    }
    font_draw_text(216 << 6, 142 << 6, 264 << 6, dbg_s_turn);
    s = m->pos ? dbg_s_defence : dbg_s_attack;
    if (m->item == 3 && m->sel >= 0) {
        int w = font_measure_text(s);

        DBG_HILITE(color, 276, 142, w);
    }
    id = dd_field_cur_id();
    font_draw_text(216 << 6, 156 << 6, 264 << 6, dbg_s_field);
    name = card_get_name(id);
    wtext_from_int(id, buf);
    wcscat(buf, name);
    if (m->item == 4 && m->sel >= 0) {
        int w = font_measure_text(buf);

        DBG_HILITE(color, 276, 156, w);
    }
    font_draw_text(276 << 6, 156 << 6, 204 << 6, buf);
    font_draw_text(216 << 6, 170 << 6, 264 << 6, dbg_s_gameset);
    if (m->sel >= 0) {
        s = dbg_s_result[m->sel];
        {
        int w = font_measure_text(s);

        DBG_HILITE(color, 292, 170, w);
    }
        font_draw_text(292 << 6, 170 << 6, 188 << 6, s);
    }
    font_draw_text(216 << 6, 184 << 6, 264 << 6, dbg_s_undo);
    return font_text_clear();
}

/* puts the chosen card on the target zone, with the face and position bits of the menu */
static inline void dbg_send(DebugMenu *m)
{
    DdPos arg = { debug_target.loc.side, debug_target.loc.zone, debug_target.loc.index, m->face, m->pos };

    zone_cmd(debug_target.loc, *(ZoneArg *)&arg, m->card);
}

/* 0x08859170 */
void debug_menu_update(void)
{
    DebugMenu *m = &debug_menu;
    DebugTarget *t = &debug_target;
    PadState *pad;
    int changed;
    int sort;
    int i;
    int d;

    pad = pad_get();
    changed = 0;
    if (debug_menu.sel < 0) {
        if (pad->b.trig & 0x10) {
            if (--m->item < 0)
                m->item = 6;
        } else if (pad->b.trig & 0x40) {
            if (++m->item >= 7)
                m->item = 0;
        } else if (pad->b.release & region_pick_value(1)) {
            dbg_send(m);
            dd_mode_pop();
        } else if (pad->b.trig & region_pick_value(0)) {
            m->saved_card = m->card;
            m->saved_name = m->name;
            m->saved_face = m->face;
            m->saved_pos = m->pos;
            switch (m->item) {
            case 0:
            case 1:
            case 4:
            case 5:
                m->sel = 0;
                break;
            case 2:
                switch (t->loc.zone) {
                case 0 ... 4:
                    if (dv_slot_pos(t->loc.side, t->loc.zone))
                        dv_088C936C(t->loc.side, t->loc.zone, 1, 0, 0);
                    else
                        dv_088C9878(t->loc.side, t->loc.zone, 1, 1, 0, 0);
                    break;
                case 5 ... 9:
                case 10:
                    dv_088C936C(t->loc.side, t->loc.zone, 1, 0, 0);
                    break;
                }
                break;
            case 3:
                switch (t->loc.zone) {
                case 0 ... 4:
                    if (dv_slot_faceup(t->loc.side, t->loc.zone))
                        dv_088C9878(t->loc.side, t->loc.zone, 0, 1, 0, 0);
                    else
                        dv_088C9878(t->loc.side, t->loc.zone, 1, 1, 0, 0);
                    break;
                }
                break;
            case 6:
                m->card = t->card;
                m->name = card_get_name(m->card);
                dd_info_select(m->card);
                break;
            }
        }
    } else {
        switch (m->item) {
        default:
            m->sel = -1;
            break;
        case 0: {
            int step[4] = { 1000, 100, 10, 1 };

            if (pad->b.rep & 0x80) {
                if (--m->sel < 0)
                    m->sel = 3;
            } else if (pad->b.rep & 0x20) {
                if (++m->sel >= 4)
                    m->sel = 0;
            } else if (pad->b.rep & 0x10) {
                m->card -= step[m->sel];
                if (m->card < 4007)
                    m->card = 4007;
                while (card_get_flag31(m->card) == 0 && m->card > 4007)
                    m->card--;
                changed = 1;
            } else if (pad->b.rep & 0x40) {
                m->card += step[m->sel];
                if (m->card >= 6958)
                    m->card = 6957;
                while (card_get_flag31(m->card) == 0 && m->card < 6957)
                    m->card++;
                changed = 1;
            } else if (pad->b.trig & region_pick_value(0)) {
                m->sel = -1;
                dbg_send(m);
            } else if (pad->b.trig & region_pick_value(1)) {
                m->card = m->saved_card;
                m->name = m->saved_name;
                m->sel = -1;
            }
            if (changed) {
                m->name = card_get_name(m->card);
                dd_info_select(m->card);
            }
            break;
        }
        case 1:
            sort = card_get_sort(m->card);
            i = card_get_sort_position(m->card);
            if (pad->b.rep & 0x80) {
                i--;
                if (i < 0)
                    i = 0;
                sort = card_get_sort(card_top_get_index(i));
                changed = 1;
            } else if (pad->b.rep & 0x20) {
                i++;
                if (i >= card_top_count())
                    i = card_top_count() - 1;
                sort = card_get_sort(card_top_get_index(i));
                changed = 1;
            } else if (pad->b.rep & 0x10) {
                sort--;
                if (sort < 0)
                    sort = 0;
                changed = 1;
            } else if (pad->b.rep & 0x40) {
                sort++;
                if (sort >= 2480)
                    sort = 2479;
                changed = 1;
            } else if (pad->b.trig & region_pick_value(0)) {
                m->sel = -1;
                dbg_send(m);
            } else if (pad->b.trig & region_pick_value(1)) {
                m->card = m->saved_card;
                m->name = m->saved_name;
                m->sel = -1;
            }
            if (changed) {
                for (i = 4007; i < 6958; i++) {
                    if (sort == card_get_sort(i)) {
                        m->card = i;
                        break;
                    }
                }
                m->name = card_get_name(m->card);
                dd_info_select(m->card);
            }
            break;
        case 4:
            i = dd_field_cur_id();
            if (pad->b.trig & 0x90) {
                d = -1;
            } else if (pad->b.trig & 0x60) {
                d = 1;
            } else {
                if (pad->b.release & region_pick_value(1))
                    m->sel = -1;
                break;
            }
            do {
                i += d;
                if (i < 0)
                    i = 6957;
                if (i == 1)
                    i = 4007;
                if (i == 4006)
                    i = 0;
                if (i >= 6958)
                    i = 0;
            } while (i != 0 && card_get_prop5_hi(i) != 2);
            dd_field_set(i);
            break;
        case 5:
            if (pad->b.trig & 0x90) {
                if (--m->sel < 0)
                    m->sel = 2;
            } else if (pad->b.trig & 0x60) {
                if (++m->sel > 2)
                    m->sel = 0;
            } else if (pad->b.release & region_pick_value(1)) {
                m->sel = -1;
            } else if (pad->b.trig & region_pick_value(0)) {
                switch (m->sel) {
                case 0:
                    dv_088C452C(1, 0);
                    break;
                case 1:
                    dv_088C452C(0, 0);
                    break;
                case 2:
                    dv_088C452C(0, 0);
                    dv_088C452C(1, 0);
                    break;
                }
                m->sel = -1;
            }
            break;
        case 2:
        case 3:
        case 6:
            m->sel = -1;
            break;
        }
    }
}
