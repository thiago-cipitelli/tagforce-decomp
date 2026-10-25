// @range 0x0881A20C-0x0881C0E8
// @asm 0x0881A8D4-0x0881AA50
// @asm 0x0881BBFC-0x0881BE08
// @rodata 0x08821074-0x08821090
// @data 0x08822578-0x088228C0
/* The lesson: the editor's lists filled with the lesson's cards, and the lesson script run one
   command per frame (a byte opcode and its arguments): talk text, message windows, full-screen
   boards, highlighted areas, the buttons the player may use or that the lesson presses for him,
   goals to reach (a move opened, a card put...), loops, menu locks, the search word and the end. */
#include "libdeck_tutorial.h"

typedef struct DtButton {
    unsigned short res;
    unsigned char u, v;
    unsigned short text;
} DtButton;

typedef struct DtArgs8 {         /* command arguments: two halves and a word */
    unsigned short h0;
    unsigned short h1;
    unsigned int w;
} DtArgs8;

typedef struct DtHintBtn {       /* a button of the hints window */
    DtButton b;
    unsigned short pad;
} DtHintBtn;

extern DtHintBtn dt_hint_btns[5];

typedef struct DtLessonCard {    /* a card of the lessons, 6 bytes */
    unsigned short lessons;      /* bit per lesson that gets it */
    unsigned short id;
    unsigned char count;         /* +0x04 */
    unsigned char list;          /* +0x05 dts.list it goes to */
} DtLessonCard;

int cfg_get_language(void);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short region_pick_value(int flag);
void se_play(int id);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int card_attribute_icon_is3(int id);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *gim_set_texture(void *list, void *gim);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);

void dt_list_alloc(DtList *l, unsigned short tab);
void dt_list_copy(DtList *dst, DtList *src, int alloc);
void dt_list_total(DtList *l);
void dt_list_add(DtList *l, DtEnt *src, int pos);
void dt_attr_count_update(void);
void dt_list_sort(DtList *l, int reset, int keep);
unsigned short dt_list_filter_reset(DtList *l);
unsigned short dt_sort_key(unsigned short id, void *tab);
void dt_set_mode(unsigned short mode);
void dt_cardlist_setup(void);
void dt_layout_scroll(int keep, int count, unsigned short view, unsigned short total,
                      unsigned short *top, unsigned short *cx, unsigned short *cy);
void *dt_draw_ibutton(void *list, DtButton *b, int x, int y, unsigned int color);
int dt_text_update(void);
void dt_text_init(void *heap, int x, int w, int size, int wait, int style);
void dt_text_set_icon(void *gim);
MenuItemDef *dt_text_def(void);
void dt_text_set(unsigned short *text, unsigned short *name);
void dt_text_place(int x, int y, int w);
void dt_word_set(unsigned short *text);

int dt_press_update(void);
int dt_goal_update(void);
int dt_press_step(void);
void dt_win_open(int x, int y, unsigned short msg);
void *dt_draw_board1(void *list, int x, int y, int w, int h);
void *dt_draw_board2(void *list, int x, int y, int w, int h);
void *dt_draw_board3(void *list, int x, int y, int w, int h);
void *dt_draw_hint2(void *list, int x, int y);
void *dt_draw_hint3(void *list, int x, int y);

DtLessonCard dt_lesson_cards[0x75] = {
    { 0x1F, 0x1164, 1, 0 }, { 0x1F, 0x1164, 1, 0 }, { 0x1F, 0x1156, 1, 0 }, { 0x1F, 0x1156, 1, 0 },
    { 0x1F, 0xFED, 1, 0 }, { 0x1F, 0xFED, 1, 0 }, { 0x1F, 0xFED, 1, 0 }, { 0x1F, 0xFF2, 1, 0 },
    { 0x1F, 0xFF2, 1, 0 }, { 0x1F, 0x115F, 1, 0 }, { 0x1F, 0x1180, 1, 0 }, { 0x1F, 0x1180, 1, 0 },
    { 0x1F, 0xFEC, 1, 0 }, { 0x1F, 0xFEC, 1, 0 }, { 0x1F, 0xFEC, 1, 0 }, { 0x1F, 0x110F, 1, 0 },
    { 0x1F, 0x110F, 1, 0 }, { 0x1F, 0x110F, 1, 0 }, { 0x1F, 0x1220, 1, 0 }, { 0x1F, 0x1220, 1, 0 },
    { 0x1F, 0x13C4, 1, 0 }, { 0x1F, 0x13C4, 1, 0 }, { 0x1F, 0x13C4, 1, 0 }, { 0x1F, 0x1168, 1, 0 },
    { 0x1F, 0x1168, 1, 0 }, { 0x1F, 0x1168, 1, 0 }, { 0x1F, 0x1173, 1, 0 }, { 0x1F, 0x1173, 1, 0 },
    { 0x1F, 0x133E, 1, 0 }, { 0x1F, 0x11B2, 1, 0 }, { 0x1F, 0x17E8, 1, 0 }, { 0x1F, 0x16AB, 1, 0 },
    { 0x1F, 0x1319, 1, 0 }, { 0x1F, 0x17F9, 1, 0 }, { 0x1F, 0x1714, 1, 0 }, { 0x1F, 0x131A, 1, 0 },
    { 0x1F, 0x10DB, 1, 0 }, { 0x1F, 0x12C4, 1, 0 }, { 0x1F, 0x1928, 1, 0 }, { 0x1F, 0x12E7, 1, 0 },
    { 0x1D, 0xFA7, 3, 3 }, { 0x1D, 0xFBC, 3, 3 }, { 0x1D, 0xFC9, 3, 3 }, { 0x1D, 0x18A6, 3, 3 },
    { 0x1D, 0x18A7, 3, 3 }, { 0x1D, 0x18A8, 3, 3 }, { 0x1D, 0x18A9, 3, 3 }, { 0x12, 0xFB5, 3, 3 },
    { 0x12, 0xFC4, 3, 3 }, { 0x12, 0xFCB, 3, 3 }, { 0x12, 0xFD1, 3, 3 }, { 0x12, 0x18C8, 3, 3 },
    { 0x12, 0x18C9, 3, 3 }, { 0x4, 0x10DB, 1, 2 }, { 0x4, 0x10DB, 1, 2 }, { 0x4, 0x1345, 1, 2 },
    { 0x4, 0x1345, 1, 2 }, { 0x18, 0xFAA, 1, 3 }, { 0x18, 0xFE4, 1, 3 }, { 0x18, 0xFE2, 1, 3 },
    { 0x18, 0x109D, 1, 3 }, { 0x18, 0x12A8, 1, 3 }, { 0x18, 0x13CA, 1, 3 }, { 0x18, 0x181F, 1, 3 },
    { 0x10, 0xFA9, 1, 3 }, { 0x10, 0xFAC, 1, 3 }, { 0x10, 0xFBF, 1, 3 }, { 0x10, 0xFC1, 1, 3 },
    { 0x10, 0xFD5, 1, 3 }, { 0x10, 0xFD7, 1, 3 }, { 0x10, 0xFE1, 1, 3 }, { 0x10, 0xFE4, 1, 3 },
    { 0x10, 0xFE6, 1, 3 }, { 0x10, 0xFF0, 1, 3 }, { 0x10, 0xFF3, 1, 3 }, { 0x10, 0x1008, 1, 3 },
    { 0x10, 0x101B, 1, 3 }, { 0x10, 0x102E, 1, 3 }, { 0x10, 0x103B, 1, 3 }, { 0x10, 0x103D, 1, 3 },
    { 0x10, 0x104B, 1, 3 }, { 0x10, 0x1087, 1, 3 }, { 0x10, 0x109F, 1, 3 }, { 0x10, 0x10A1, 1, 3 },
    { 0x10, 0x10C5, 1, 3 }, { 0x10, 0x1108, 1, 3 }, { 0x10, 0x113A, 1, 3 }, { 0x10, 0x1167, 1, 3 },
    { 0x10, 0x116E, 1, 3 }, { 0x10, 0x116F, 1, 3 }, { 0x10, 0x1171, 1, 3 }, { 0x10, 0x1172, 1, 3 },
    { 0x10, 0x1186, 1, 3 }, { 0x10, 0x119C, 1, 3 }, { 0x10, 0x11A5, 1, 3 }, { 0x10, 0x11AB, 1, 3 },
    { 0x10, 0x11C4, 1, 3 }, { 0x10, 0x11C5, 1, 3 }, { 0x10, 0x11C8, 1, 3 }, { 0x10, 0x11CC, 1, 3 },
    { 0x10, 0x11D5, 1, 3 }, { 0x10, 0x11DC, 1, 3 }, { 0x10, 0x11DD, 1, 3 }, { 0x10, 0x11E0, 1, 3 },
    { 0x10, 0x11E8, 1, 3 }, { 0x10, 0x11F7, 1, 3 }, { 0x10, 0x1215, 1, 3 }, { 0x10, 0x1218, 1, 3 },
    { 0x10, 0x121C, 1, 3 }, { 0x10, 0x1229, 1, 3 }, { 0x10, 0x122C, 1, 3 }, { 0x10, 0x127B, 1, 3 },
    { 0x10, 0x1288, 1, 3 }, { 0x10, 0x13AC, 1, 3 }, { 0x10, 0x13C5, 1, 3 }, { 0x10, 0x1464, 1, 3 },
    { 0x10, 0x17CF, 1, 3 },
};

/* fills the editor's lists with the cards of lesson `lesson` (main, side, extra, trunk) */
void dt_lists_init(unsigned short lesson)
{
    int i;
    DtList *l;
    DtLessonCard *c;
    unsigned int k;
    DtEnt e;
    DtList *p;
    int j;

    for (i = 0, l = dts.list; i <= 3; i++, l++) {
        dt_list_alloc(l, i);
        l->tab = i;
        dt_list_total(l);
    }
    l = &dts.list[4];
    dt_list_copy(l, &dts.list[2], 1);
    dt_list_total(l);
    l->tab = 4;
    dt_list_sort(l, 1, 0);
    dt_list_filter_reset(l);
    dts.n_owned = 0;
    for (k = 0, c = dt_lesson_cards; k < 0x75; k++, c++) {
        if (c->lessons & 1 << lesson) {
            memset(&e, 0, 6);
            e.id = c->id;
            e.key = dt_sort_key(e.id, dt_eres_get(0x8031));
            e.count = c->count;
            dt_list_add(&dts.list[c->list], &e, -1);
            dts.n_owned++;
        }
    }
    for (j = 0, p = dts.list; j <= 3; j++, p++) {
        p->count = p->n;
        dt_list_total(p);
        dt_list_filter_reset(p);
        dt_list_sort(p, 1, 0);
    }
    l = &dts.list[4];
    dt_list_total(l);
    dt_list_filter_reset(l);
    dt_list_sort(l, 1, 0);
    wcscpy(dts.name, table_get_entry(dt_res_get(0x1040), 0xDE));
    dt_attr_count_update();
}

/* the editor of lesson `lesson`: both panes on the trunk, fonts, menus, the talk box, the script */
void dt_editor_start(unsigned short lesson)
{
    dt_lists_init(lesson);
    dtv.pane[0].list = &dts.list[3];
    dtv.pane[1].list = &dts.list[3];
    dt_set_mode(0);
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    font_set_line_extra(0);
    menu_init(&dt_menu, dts.heap);
    dt_text_init(dts.heap, 0, 0x1E0, 0xE, 1, 0);
    dt_text_set_icon(dt_res_get(0x1010));
    dt_text_def()->gim = dt_res_get(0x1011);
    dt.text_box = 0;
    dt.board = 0;
    dt_cardlist_setup();
    dt.script = dt_res_get(0x1041);
}

/* commands of the lesson script, by opcode */
void (*dt_cmds[24])(void);

/* per frame: the input given to the editor, then the current wait or the next command */
void dt_editor_update(void)
{
    PadState *pad;
    unsigned char op;

    if (pad_get()->b.down & 0x100)
        dt.text_on = 0;
    else
        dt.text_on = 1;
    if (dt.show & 4) {
        pad = pad_get();
        dt.pad.down = pad->b.down & dt.btn_mask2;
        dt.pad.trig = pad->b.trig & dt.btn_mask2;
        dt.pad.release = pad->b.release & dt.btn_mask2;
        dt.pad.rep = pad->b.rep & dt.btn_mask2;
        if (pad->b.trig & ~dt.btn_mask2)
            se_play(0x6215);
    }
    if ((dt.show & 8) && dt.loop_n < 0 && (pad_get()->b.trig & 0x6000))
        dt.loop_n = 1;
    switch (dt.step) {
    case 0:
        op = *dt.script++;
        dt.text_box = 0;
        dt.board = 0;
        if (dt_cmds[op] != 0)
            dt_cmds[op]();
        break;
    case 1:
        if (dt.text_on && dt_text_update())
            dt.step = 0;
        break;
    case 4:
        if (dt.text_on) {
            if (dt.board_win.state == 3 && (pad_get()->b.trig & region_pick_value(0))) {
                se_play(0x6213);
                menu_finish(&dt.board_win);
                dt.board_win.cancelled = 1;
                break;
            }
            if (menu_update(&dt.board_win) != -1) {
                menu_reset(&dt.board_win);
                dt.step = 0;
            }
        }
        break;
    case 9:
        if (dt_press_update())
            dt.step = 0;
        break;
    case 10:
        if (dt_goal_update())
            dt.step = 0;
        break;
    case 16:
        if (dt.done++ > 8) {
            dt.done = 0;
            dt.step = 0;
            dt.show |= 0x10;
        }
        break;
    }
    if (dt.show & 0x10)
        dt.pad.down |= dt.btn_one;
}

#ifdef NON_MATCHING /* mesma lógica; o original sai do caso padrão por um bloco próprio (b + lw ra) e o mwcc junta as saídas */
/* step 9: the lesson presses the buttons itself (press 0) or the card's (press 1) */
int dt_press_update(void)
{
    int r = 1;
    unsigned int m;

    switch (dt.press) {
    case 0:
        if (dt_press_step()) {
            dt.press_step = 0;
            r = 1;
        } else {
            r = 0;
        }
        break;
    case 1:
        switch (dt.press_step) {
        case 0:
            if (card_attribute_icon_is3(dt_cur_entry(dtv.side)->id)) {
                m = region_pick_value(0);
                dt.press_step = 2;
            } else {
                m = 0x40;
                dt.press_step++;
            }
            dt.btn_mask = m;
            dt.btn_mode = 0;
            break;
        case 1:
            if (dt_press_step())
                dt.press_step = 0;
            break;
        case 2:
            if (dt_press_step()) {
                dt.btn_mask = region_pick_value(0);
                dt.press_step++;
                dt.btn_mode = 0;
            }
            break;
        case 3:
            if (dt_press_step()) {
                dt.press_step = 0;
                return 1;
            }
            break;
        }
        r = 0;
        break;
    default:
        break;
    }
    return r;
}
#endif

/* step 10: 1 once the goal is reached */
int dt_goal_update(void)
{
    switch (dt.goal) {
    case 0:
        dt.done++;
        break;
    case 1:
        if (dt.pad.trig != 0)
            dt.done++;
        break;
    case 2:
        if (dt.pad.rep != 0)
            dt.done++;
        break;
    case 3:
    case 4:
        break;
    case 5:
        return dts.state == 0;
    case 6:
        return (pad_get()->b.trig & 0x6000) != 0;
    default:
        return 1;
    }
    return dt.goal_n <= dt.done;
}

/* 1: talk text (message, x, y, w) */
void dt_cmd_text(void)
{
    unsigned short a[4];

    memcpy(a, dt.script, 8);
    dt_text_place(a[1], a[2], a[3]);
    dt_text_set(table_get_entry(dt_res_get(0x1040), a[0]), 0);
    dt_text_update();
    dt.text_box = 1;
    dt.script += 8;
    dt.step = 1;
}

/* 2: message window (message, x, y) */
void dt_cmd_win(void)
{
    unsigned short a[3];

    memcpy(a, dt.script, 6);
    dt_win_open(a[1], a[2], a[0]);
    dt.script += 6;
    dt.show |= 1;
}

/* 3: closes the message window */
void dt_cmd_win_close(void)
{
    menu_reset(&dt.win);
    dt.show &= ~1;
}

/* 4: full-screen board (message, picture 1..3) */
void dt_cmd_board(void)
{
    unsigned short a[2];
    MenuWidget *m;
    MenuItemDef *d;

    memcpy(a, dt.script, 4);
    m = &dt.board_win;
    d = &m->def;
    memset(m, 0, 0x120);
    d->visible = 1;
    d->font_flag4 = 1;
    d->x = 0x28;
    d->y = 0x23;
    d->w = 400;
    d->h = 0xC0;
    d->style = 0;
    if (cfg_get_language() == 0)
        d->font_flag8000 = 1;
    d->pad_l = 0xC;
    d->pad_r = 0xC;
    d->pad_t = 4;
    d->pad_b = 4;
    d->font_size = 0xE;
    d->text_color = 0xFF000000;
    d->text = table_get_entry(dt_res_get(0x1040), a[0]);
    m->heap = dts.heap;
    d->open_anim = 2;
    m->no_cancel = 1;
    d->close_anim = 2;
    switch (a[1]) {
    case 1:
        m->draw_all = (MenuDrawRectFn)dt_draw_board1;
        break;
    case 2:
        m->draw_all = (MenuDrawRectFn)dt_draw_board2;
        break;
    case 3:
        m->draw_all = (MenuDrawRectFn)dt_draw_board3;
        break;
    }
    menu_open(m);
    dt.script += 4;
    dt.step = 4;
    dt.board = 1;
}

/* 5: highlights an area (0..3), showing the list it is about */
void dt_cmd_highlight(void)
{
    unsigned short a[1];

    memcpy(a, dt.script, 2);
    dt.hl = a[0];
    switch (dt.hl) {
    case 0:
        dt_select_tab(0, 0, 2);
        break;
    case 1:
        if (dtv.mode > 1)
            dt_select_tab(1, 3, 2);
        else
            dt_select_tab(0, 3, 2);
        break;
    case 2:
        dt_select_tab(0, 1, 2);
        break;
    case 3:
        dt_select_tab(0, 2, 2);
        break;
    }
    dt.script += 2;
    dt.show |= 2;
}

/* 6 */
void dt_cmd_highlight_off(void)
{
    dt.show &= ~2;
}

/* 7: the buttons the player may use (Circle and Cross swapped on the western layout), then the
   next command at once */
void dt_cmd_allow(void)
{
    unsigned int a;
    unsigned int mask;
    unsigned char op;

    memcpy(&a, dt.script, 4);
    mask = a;
    if (!(dt.show & 0x20) && region_pick_value(0) == 0x4000) {
        mask &= ~0x6000;
        if (a & 0x2000)
            mask |= 0x4000;
        if (a & 0x4000)
            mask |= 0x2000;
    }
    dt.btn_mask2 = mask;
    dt.script += 4;
    dt.show |= 4;
    op = *dt.script++;
    dt.text_box = 0;
    dt.board = 0;
    if (dt_cmds[op] != 0)
        dt_cmds[op]();
}

/* 8 */
void dt_cmd_allow_off(void)
{
    memset(&dt.pad, 0, 0x28);
    dt.btn_mask2 = 0;
    dt.show &= ~4;
}

/* 9: the lesson presses buttons (mode, buttons) */
void dt_cmd_press(void)
{
    DtArgs8 a;
    unsigned int mask;

    memcpy(&a, dt.script, 8);
    dt.press = a.h0;
    mask = a.w;
    if (!(dt.show & 0x20) && region_pick_value(0) == 0x4000) {
        mask &= ~0x6000;
        if (a.w & 0x2000)
            mask |= 0x4000;
        if (a.w & 0x4000)
            mask |= 0x2000;
    }
    dt.btn_mask = mask;
    dt.press_step = 0;
    dt.btn_mode = 0;
    dt.done = 0;
    dt.script += 8;
    dt.step = 9;
}

/* 10: waits for a goal (kind, count) */
void dt_cmd_goal(void)
{
    unsigned short a[2];

    memcpy(a, dt.script, 4);
    dt.goal = a[0];
    dt.goal_n = a[1];
    dt.done = 0;
    dt.script += 4;
    dt.step = 10;
}

/* 11: freezes the card info on a card */
void dt_cmd_lock_info(void)
{
    unsigned short a[1];

    memcpy(a, dt.script, 2);
    dtv.cursor.scroll.id = a[0];
    dtv.cursor.scroll.pos = 0;
    dt.script += 2;
    dtv.lock = 1;
}

/* 12 */
void dt_cmd_unlock_info(void)
{
    dtv.lock = 0;
    dt_info_update();
}

/* 13: loop start (count, 0 = until a shoulder button) */
void dt_cmd_loop(void)
{
    unsigned short a[1];

    memcpy(a, dt.script, 2);
    if (a[0] <= 0)
        dt.loop_n = -1;
    else
        dt.loop_n = a[0];
    dt.script += 2;
    dt.show |= 8;
    dt.loop = dt.script;
}

/* 14: loop end */
void dt_cmd_loop_end(void)
{
    if (dt.show & 8) {
        if (dt.loop_n > 0)
            dt.loop_n--;
        if (dt.loop_n == 0)
            dt.show &= ~8;
        else
            dt.script = dt.loop;
    }
}

/* 15: locks the items of the next menu (message, allowed items) */
void dt_cmd_menu_lock(void)
{
    DtArgs8 a;

    memcpy(&a, dt.script, 8);
    dt.lock = a.w;
    dt.lock_msg = a.h1;
    dt.script += 8;
    dt.step = 15;
}

/* 16: holds buttons for the player after a short wait */
void dt_cmd_hold(void)
{
    unsigned int a;

    memcpy(&a, dt.script, 4);
    dt.btn_one = a;
    dt.done = 0;
    dt.script += 4;
    dt.step = 16;
}

/* 17 */
void dt_cmd_hold_off(void)
{
    memset(&dt.pad, 0, 0x28);
    dt.btn_one = 0;
    dt.show &= ~0x10;
}

/* 18: Circle and Cross kept as they are (or swapped again) */
void dt_cmd_keep_buttons(void)
{
    if (dt.show & 0x20)
        dt.show &= ~0x20;
    else
        dt.show |= 0x20;
}

/* 19: the search word */
void dt_cmd_word(void)
{
    unsigned short a[1];

    memcpy(a, dt.script, 2);
    dt_word_set(table_get_entry(dt_res_get(0x1040), a[0]));
    dt.script += 2;
}

/* 20: the button hints window (kind, x, y) */
void dt_cmd_hint(void)
{
    unsigned short a[3];
    MenuWidget *m;
    MenuItemDef *d;

    memcpy(a, dt.script, 6);
    m = &dt.menu;
    d = &m->def;
    memset(m, 0, 0x120);
    d->x = a[1];
    d->y = a[2];
    d->visible = 1;
    d->font_flag4 = 1;
    d->color = 0xFFFFFFFF;
    d->text_color = 0xFFFFFFFF;
    d->pad_l = 2;
    d->pad_t = 2;
    d->pad_r = 2;
    d->pad_b = 2;
    d->font_size = 0xC;
    d->style = 0;
    d->gim = dt_res_get(0x1011);
    m->heap = dts.heap;
    m->no_cancel = 1;
    d->style = 0;
    switch (a[0]) {
    case 0:
        d->w = 0x93;
        d->h = 0x2F;
        m->draw_all = (MenuDrawRectFn)dt_draw_hint2;
        break;
    case 1:
        d->w = 0x93;
        d->h = 0x41;
        m->draw_all = (MenuDrawRectFn)dt_draw_hint3;
        break;
    }
    menu_open(m);
    dt.script += 6;
    dt.show |= 0x40;
}

/* 21 */
void dt_cmd_hint_close(void)
{
    menu_reset(&dt.menu);
    dt.show &= ~0x40;
}

/* 22: scrolls the trunk (0: shows it, 1: to the entry given) */
void dt_cmd_scroll(void)
{
    DtArgs8 a;
    DtPane *p;
    unsigned short view;
    unsigned short grid;

    memcpy(&a, dt.script, 8);
    switch (a.h0) {
    case 0:
        if (dtv.mode > 1)
            dt_select_tab(1, 3, 2);
        else
            dt_select_tab(0, 3, 2);
        break;
    case 1:
        p = &dtv.pane[dtv.side];
        view = p->list_mode == 0 ? p->lay.grid_rows : p->lay.rows;
        grid = p->list_mode == 0 ? p->lay.cols : 1;
        dt_layout_scroll(0, a.w, (grid & 0xFF) | (view & 0xFF) << 8,
                         p->list->count, &p->top, &p->cx, &p->cy);
        break;
    }
    dt.script += 8;
}

/* 23: end of the lesson */
void dt_cmd_end(void)
{
    dts.unload_step = 0;
    dt.state = 3;
    fade_start_in(1, 0xFF000000, 16);
}

/* the lesson's lock on a menu about to open (step 15) */
void dt_menu_lock(MenuWidget *m)
{
    if ((dt.show & 1) && dt.lock_msg != 0)
        dt_win_open(dt.win.def.x, dt.win.def.y, dt.lock_msg);
    m->disabled = ~dt.lock;
    dt.step = 0;
}

/* the pictures of the boards */
void *dt_draw_board1(void *list, int x, int y, int w, int h)
{
    void *gim = dt_res_get(0x1014);

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 0x100, 0x100, 0x100, 0x100);
    return draw_sprite(list, x + (w - 0x100) / 2, y + 2, 0, 0, 0, 0, 0);
}

void *dt_draw_board2(void *list, int x, int y, int w, int h)
{
    void *gim = dt_res_get(0x1015);

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 0x100, 0x100, 0x100, 0x100);
    return draw_sprite(list, x + (w - 0x100) / 2, y + 2, 0, 0, 0, 0, 0);
}

void *dt_draw_board3(void *list, int x, int y, int w, int h)
{
    void *gim = dt_res_get(0x1016);

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 0x100, 0x100, 0x100, 0x100);
    return draw_sprite(list, x + (w - 0x100) / 2, y + 2, 0, 0, 0, 0, 0);
}

/* one press of dt.btn_mask: wait, down, held, up, wait; 1 when done */
int dt_press_step(void)
{
    switch (dt.btn_mode) {
    case 0:
        if (dt.done++ > 8) {
            dt.pad.trig = dt.btn_mask;
            dt.pad.down = dt.btn_mask;
            dt.pad.rep = dt.btn_mask;
            dt.done = 0;
            dt.btn_mode++;
        }
        break;
    case 1:
        dt.pad.trig = 0;
        dt.pad.down = 0;
        dt.pad.rep = 0;
        dt.btn_mode++;
        dt.done = 0;
        break;
    case 2:
        if (dt.done++ > 8) {
            dt.btn_mode++;
            dt.done = 0;
        }
        break;
    case 3:
        if (dt.done++ > 8) {
            dt.btn_mode++;
            dt.done = 0;
        }
        break;
    case 4:
        dt.btn_mode = 0;
        dt.done = 0;
        return 1;
    }
    return 0;
}

/* the button hints */
#ifdef NON_MATCHING /* cópias de struct de 6 bytes: a primeira membro a membro com inserção de bitfield, a segunda com lwc1/swc1; nenhuma grafia testada reproduziu */
void *dt_draw_hint2(void *list, int x, int y)
{
    DtButton a = dt_hint_btns[0].b;
    DtButton b = dt_hint_btns[1].b;

    list = dt_draw_ibutton(list, &a, x, y + 1, 0);
    return dt_draw_ibutton(list, &b, x, y + 0x13, 0);
}

void *dt_draw_hint3(void *list, int x, int y)
{
    DtButton a = dt_hint_btns[2].b;
    DtButton b = dt_hint_btns[3].b;
    DtButton c = dt_hint_btns[4].b;

    list = dt_draw_ibutton(list, &a, x, y + 1, 0);
    list = dt_draw_ibutton(list, &b, x, y + 0x13, 0);
    return dt_draw_ibutton(list, &c, x, y + 0x25, 0);
}
#endif

/* the message window at (x, y) */
void dt_win_open(int x, int y, unsigned short msg)
{
    MenuWidget *m = &dt.win;
    MenuItemDef *d = &m->def;

    if (dt.show & 1)
        menu_reset(m);
    memset(m, 0, 0x120);
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->max_w = 0x1DC - x;
    d->max_h = 0x10C - y;
    d->color = 0xFFFFFFFF;
    d->style = 0;
    d->x = x;
    d->y = y;
    if (cfg_get_language() == 0) {
        font_set_line_extra(0xC0);
        d->font_flag8000 = 1;
    }
    d->pad_l = 6;
    d->pad_r = 6;
    d->pad_t = 2;
    d->pad_b = 2;
    d->font_size = 0xC;
    d->text_color = 0xFFFFFFFF;
    d->text = table_get_entry(dt_res_get(0x1040), msg);
    d->gim = dt_res_get(0x1011);
    d->style = 2;
    m->heap = dts.heap;
    m->no_cancel = 1;
    menu_open(m);
    font_set_line_extra(0);
}

/* the quit confirmation (START) */
void dt_pause_open(void)
{
    MenuWidget *m = &dt.quit;
    MenuItemDef *d = &m->def;

    memset(m, 0, 0x120);
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->max_w = 0xF0;
    d->color = 0xFFFFFFFF;
    d->pad_l = 10;
    d->pad_r = 10;
    d->pad_t = 8;
    d->font_size = 0xC;
    d->text_color = 0xFF000000;
    d->style = 0;
    d->pad_b = 0;
    d->text = table_get_entry(dt_res_get(0x1040), 1);
    d->open_anim = 2;
    d->close_anim = 2;
    m->heap = dts.heap;
    m->no_cancel = 1;
    m->initial = 3;
    m->buttons = 3;
    m->cancel_resets = 1;
    m->button = 3;
    menu_open(m);
    d->x = (0x1E0 - d->w) / 2;
    d->y = (0x110 - d->h) / 2;
    dt.quit_open = 1;
}

void (*dt_cmds[24])(void) = {
    0, dt_cmd_text, dt_cmd_win, dt_cmd_win_close, dt_cmd_board, dt_cmd_highlight,
    dt_cmd_highlight_off, dt_cmd_allow, dt_cmd_allow_off, dt_cmd_press, dt_cmd_goal,
    dt_cmd_lock_info, dt_cmd_unlock_info, dt_cmd_loop, dt_cmd_loop_end, dt_cmd_menu_lock,
    dt_cmd_hold, dt_cmd_hold_off, dt_cmd_keep_buttons, dt_cmd_word, dt_cmd_hint,
    dt_cmd_hint_close, dt_cmd_scroll, dt_cmd_end,
};

DtHintBtn dt_hint_btns[5] = {
    { { 0x8008, 0x00, 0x20, 6 } },
    { { 0x8008, 0x33, 0x31, 0x14 } },
    { { 0x800A, 0x33, 0x00, 0x35 } },
    { { 0x800A, 0x44, 0x00, 0x36 } },
    { { 0x800A, 0x55, 0x00, 0x37 } },
};
