// @range 0x08819C04-0x0881CA2C
// @asm 0x08819C30-0x08819F2C
// @asm 0x08819F2C-0x0881A020
// @asm 0x0881A020-0x0881A0AC
// @asm 0x0881A0AC-0x0881A458
// @asm 0x0881A458-0x0881A494
// @asm 0x0881A4BC-0x0881A4F8
// @asm 0x0881A504-0x0881ABD8
// @asm 0x0881ABD8-0x0881AFC0
// @asm 0x0881B004-0x0881B134
// @asm 0x0881B134-0x0881B1AC
// @asm 0x0881B21C-0x0881B340
// @asm 0x0881B340-0x0881B3C0
// @asm 0x0881B48C-0x0881B598
// @asm 0x0881B5A4-0x0881B6E0
// @asm 0x0881B70C-0x0881B9B4
// @asm 0x0881BA40-0x0881BACC
// @asm 0x0881BACC-0x0881BCB8
// @asm 0x0881BCB8-0x0881BE90
// @asm 0x0881BE90-0x0881BF1C
// @asm 0x0881BF1C-0x0881BFA8
// @asm 0x0881BFA8-0x0881BFF0
// @asm 0x0881BFF0-0x0881C144
// @asm 0x0881C150-0x0881C250
// @asm 0x0881C3A8-0x0881C510
// @asm 0x0881C510-0x0881C678
// @asm 0x0881C678-0x0881C764
// @asm 0x0881C764-0x0881C8F0
// @asm 0x0881C91C-0x0881C9A0
// @data 0x08875E60-0x08875F84
/* The card list window of prompts (choose N cards out of a list: deck search, graveyard, ...):
   list of card rows (picture, name, attribute, ATK/DEF, level), picks with min/max, a sort window
   (key and direction), a filter window by card kind and an options window (card info, sort,
   filter, ...). */
#include "dd_field_ui.h"

typedef struct DdSelDesc {             /* what the caller passes to dd_sel_open */
    int n;
    int unk4;
    int *ids;                          /* +0x08 card ids */
    int reverse;                       /* +0x0C sort by index from the end */
    unsigned short *title;             /* +0x10 0: the default title */
} DdSelDesc;

typedef struct DdSel {                 /* 0x2CA8 at 0x08C547B4 */
    MenuWidget list;                   /* +0x0000 */
    MenuWidget sub;                    /* +0x0120 sort/filter/options window */
    unsigned short title[0x100];       /* +0x0240 */
    int done;                          /* +0x0440 the cursor is on the OK button */
    int n_all;                         /* +0x0444 */
    int ids[512];                      /* +0x0448 */
    int n;                             /* +0x0C48 entries after the filter */
    int filt[512];                     /* +0x0C4C */
    int order[512];                    /* +0x144C sorted entries (indexes into ids) */
    int n_saved;                       /* +0x1C4C */
    int saved[512];                    /* +0x1C50 order before the sort window */
    int max;                           /* +0x2450 */
    int min;                           /* +0x2454 */
    int n_picked;                      /* +0x2458 */
    int picked[512];                   /* +0x245C */
    int last;                          /* +0x2C5C entry whose card is shown */
    int mode;                          /* +0x2C60 0 list, 1 sort, 2 options, 3 filter */
    int key;                           /* +0x2C64 sort key */
    int dir;                           /* +0x2C68 */
    int key_saved;                     /* +0x2C6C */
    int dir_saved;                     /* +0x2C70 */
    int filter;                        /* +0x2C74 */
    int filter_saved;                  /* +0x2C78 */
    int opts[8];                       /* +0x2C7C */
    int n_opts;                        /* +0x2C9C */
    int opts_w;                        /* +0x2CA0 */
    int reverse;                       /* +0x2CA4 */
} DdSel;

typedef struct DdSelOpt {              /* an entry of the options window */
    int id;
    int u, v;                          /* icon */
    int text;
} DdSelOpt;

typedef struct DdUv16 {
    short u, v;
} DdUv16;

extern DdSel dd_sel;                   /* 0x08C547B4 */

DdSelOpt dd_sel_opts[5] = {
    { 0, 0, 0x42, 4 }, { 1, 0x4E, 0x69, 0x35 }, { 2, 0x5B, 0x69, 0x36 }, { 3, 0x4E, 0x5C, 0x34 }, { 4, 0x34, 0x4F, 0x33 },
};
unsigned short dd_sel_qmark[2] = { '?', 0 };
/* (attribute-icon mask, icon) of the row icons */
int dd_sel_icons[11][2] = {
    { 2, 0 }, { 4, 2 }, { 8, 3 }, { 0x10, 1 }, { 0x20, 4 }, { 0x40, 6 }, { 0x80, 5 }, { 0x100, 7 }, { 0x200, 7 },
    { 0x400, 8 }, { 0x800, 9 },
};
DdUv16 dd_sel_key_uv[10] = {
    { 0x1F, 0xF }, { 0, 0x20 }, { 0x44, 0x20 }, { 0x33, 0x20 }, { 0x11, 0x20 },
    { 0x22, 0x20 }, { 0x66, 0x20 }, { 0x55, 0x20 }, { 0x44, 0x31 }, { 0x11, 0x31 },
};
DdUv16 dd_sel_dir_uv[2] = { { 0x22, 0x31 }, { 0x33, 0x31 } };
unsigned char dd_sel_trap_order[8] = { 9, 9, 2, 1, 4, 0, 3, 0 };
unsigned char dd_sel_spell_order[8] = { 0x13, 0x0A, 0x13, 0x13, 0x0B, 0x13, 0x13, 0 };
int dd_sel_filter_icon[7] = { 0, 0, 1, 2, 3, 7, 8 };
DdUv16 dd_sel_filter_uv[7] = {
    { 0x66, 0x5E }, { 0, 0x5E }, { 0x11, 0x5E }, { 0x22, 0x5E }, { 0x33, 0x5E }, { 0x44, 0x5E }, { 0x55, 0x5E },
};

unsigned short *dd_text(int id);
unsigned int *dd_pad(void);
void *dd_alloc(int size);
void dd_free(void *p);
void *dd_heap_get(void);
void *dd_gim_get(int id);
void dd_info_select(int uid);
void dd_mode_push(int id);
void dd_mode_set_flag(int id);
void dd_mode_set_sel_cb(void (*cb)(void));
void dd_mode_set_card(int id);
int dd_is_tag_duel(void);
int hl_allowed(void);
void hl_open(void);
void opt_open(void);
void func_08825B9C(MenuWidget *w);
unsigned int *dd_wline_draw(unsigned int *list, int a, int n, float *v, int b);
unsigned int *dd_wline_sprite(unsigned int *list, int a, float *p, float *q);
unsigned int *func_088212C4(unsigned int *list, int x, int y, int icon);
unsigned int *dd_card_texture(unsigned int *list, int uid);
int dv_0884C824(void);
unsigned short *dv_0884C7EC(int i);
int dv_0884C8E4(int id);
int duel_0884C830(int id);
void declare_open(int a);
void dv_0884C7C0(int id);
void dv_w624_set(int n);
void dv_0884CB8C(int id, int k);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
void *memcpy(void *d, const void *s, unsigned int n);
int menu_cursor_y(Menu *m, MenuItemDef *def);
int font_draw_text(int x, int y, int w, unsigned short *text);
int font_get_cursor_x(void);
unsigned int *card_draw_glow(unsigned int *list, int x, int y, int up, int t, int on);
unsigned int *cardlist_draw_bar(unsigned int *list, int x, int y, int w, int pos, int size);
unsigned short *wtext_from_int(int v, unsigned short *out);
unsigned short *card_get_name(int id);
int card_get_attribute_icon(int id);
int card_get_atk_q(int id);
int card_get_def_q(int id);
int card_get_level(int id);
unsigned int *card_draw_frame_special(unsigned int *list, int x, int y, int attr);
int card_get_prop5_hi(int id);
int card_get_attribute(int id);
unsigned int *cardlist_draw_stars(unsigned int *list, int x, int y, int n, unsigned int color);
unsigned int *cardlist_draw_icon(unsigned int *list, int x, int y, int attr, int sub);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
int card_get_limit(int id);
int card_get_sort(int id);
int card_get_kind(int id);
int card_get_type(int id);

int dd_sel_filter(int *out, int n, int filter);
void dd_sel_sort(int *list, int n, int key, int dir);
void dd_sel_set_minmax(int min, int max);
void *dd_sel_draw_item(void *list, int i, int x, int y);
void *dd_sel_draw_after(void *list, int x, int y, int w, int h);
void dd_sel_done_cb(void);
unsigned int *dd_sel_draw_key(unsigned int *list, int x, int y, int key);
unsigned int *dd_sel_draw_dir(unsigned int *list, int x, int y, int dir);
unsigned int *dd_sel_draw_filter_icon(unsigned int *list, int x, int y, int f);
int dd_sel_can_accept(void);
int dd_sel_key_of(int i, int id, int key);
void dd_sel_build_opts(void);
void *dd_sel_sort_draw_item(void *list, int i, int x, int y);
void *dd_sel_opts_draw_item(void *list, int i, int x, int y);
void *dd_sel_filter_draw_item(void *list, int i, int x, int y);
int dd_sel_cmp_up(const void *a, const void *b);
int dd_sel_cmp_down(const void *a, const void *b);

void dd_sel_reset(void)
{
    menu_reset(&dd_sel.list);
    menu_reset(&dd_sel.sub);
}

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_open(DdSelDesc *d)
{
    DdSel *s = &dd_sel;
    MenuItemDef *w = &dd_sel.list.def;
    unsigned short *title = dd_text(0x2E);

    memset(s, 0, sizeof(DdSel));
    s->n_all = d->n;
    if (s->n_all > 0x200)
        s->n_all = 0x200;
    memcpy(s->ids, d->ids, s->n_all << 2);
    s->last = -1;
    s->max = 0;
    s->min = 0;
    s->n_picked = 0;
    s->reverse = d->reverse;
    s->dir = 1;
    s->filter = 0;
    s->key = 0;
    s->n = dd_sel_filter(s->filt, s->n_all, 0);
    memcpy(s->order, s->filt, s->n << 2);
    dd_sel_sort(s->order, s->n, s->key, s->dir);
    if (d->title != 0)
        wcscpy(s->title, d->title);
    else
        wcscpy(s->title, title);
    memset(s, 0, 0x120);
    w->visible = 1;
    w->auto_size = 1;
    w->font_flag4 = 1;
    w->show_title = 1;
    w->auto_title = 1;
    w->y = 0x32;
    w->w = 0xFE;
    w->color = -1;
    w->title_color = -1;
    w->pad_l = 2;
    w->pad_t = 2;
    w->pad_r = 2;
    w->pad_b = 2;
    w->style2 = 2;
    w->open_anim = 2;
    w->close_anim = 2;
    w->title = s->title;
    w->title_font_size = 0xC;
    w->style = 0;
    s->list.heap = dd_heap_get();
    s->list.no_input = 0;
    s->list.no_cancel = 1;
    s->list.cancel_blocked = 0;
    s->list.keep_open = 1;
    s->list.f0_4 = 0;
    s->list.center = 1;
    s->list.style = 3;
    s->list.rows = 5;
    s->list.item_w = 0x31;
    s->list.item_h = 0x98;
    s->list.draw_item = (MenuDrawItemFn)dd_sel_draw_item;
    s->list.draw_before = (MenuDrawRectFn)dd_sel_draw_after;
    s->list.count = s->n;
    s->list.cancel_resets = 0;
    s->list.accept_blocked = 1;
    s->list.item_y = 0x14;
    menu_open(&s->list);
    w->w = 0xFE;
    w->h += 0x2A;
    w->x = 0x13A - w->w / 2;
    s->list.cursor_h = 0x93 - s->list.item_y;
    se_play(0x621A);
}
#endif

/* opens the list with the engine's candidates; pick between min and max */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_open_engine(int max, int min, unsigned short *title, int reverse)
{
    DdSelDesc d;
    int ids[512];
    int n, i;

    n = dv_0884C824();
    if (max > n)
        max = n;
    if (min > n)
        min = n;
    for (i = 0; i < n; i++)
        ids[i] = *dv_0884C7EC(i) & 0x1FFF;
    memset(&d, 0, sizeof(d));
    d.ids = ids;
    d.unk4 = 0;
    d.n = n;
    d.reverse = reverse;
    d.title = title;
    dd_sel_open(&d);
    dd_mode_push(2);
    dd_mode_set_sel_cb(dd_sel_done_cb);
    dd_mode_set_flag(0x31);
    dd_sel_set_minmax(min, max);
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void *dd_sel_draw(void *list)
{
    menu_draw(&list, &dd_sel.list);
    switch (dd_sel.mode) {
    case 1:
        menu_draw(&list, &dd_sel.sub);
        break;
    case 2:
        menu_draw(&list, &dd_sel.sub);
        break;
    case 3:
        menu_draw(&list, &dd_sel.sub);
        break;
    }
    return list;
}
#endif

int dd_sel_sort_update(void);
void dd_sel_open_sort(void);
int dd_sel_opts_update(void);
void dd_sel_open_opts(void);
void dd_sel_open_filter(void);
int dd_sel_filter_update(void);
void dd_sel_show(int i);
void dd_sel_toggle(int i);
void dd_sel_unpick(int i);

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_update(void)
{
    DdSel *s = &dd_sel;
    unsigned int *pad = dd_pad();
    unsigned int ok = region_pick_value(0) & 0xFFFF;
    unsigned int cancel = region_pick_value(1) & 0xFFFF;
    int i;

    if (s->min > 0 || s->n_picked > 0)
        s->list.cancel_blocked = 1;
    else
        s->list.cancel_blocked = 0;
    if (s->list.state != 3) {
        menu_update(&s->list);
        return;
    }
    switch (s->mode) {
    case 0:
        if (pad[1] & 0x1000) {
            dd_sel_open_opts();
            s->mode = 2;
        } else if (s->done == 0) {
            if (s->list.count != 0)
                menu_update(&s->list);
            i = s->list.scroll + s->list.cursor;
            if (s->last != i) {
                s->last = i;
                dd_sel_show(i);
            }
            if (pad[1] & ok) {
                dd_sel_toggle(i);
            } else if (pad[1] & cancel) {
                dd_sel_unpick(i);
            } else if ((pad[3] & 0x40) || s->list.count == 0) {
                s->done = 1;
                s->list.no_cancel = 0;
                se_play(0x6212);
            }
        } else if ((pad[3] & 0x10) && s->list.count != 0) {
            s->list.no_cancel = 1;
            s->done = 0;
            se_play(0x6212);
        } else if (s->min == 0 && s->max == 0) {
            if (pad[1] & (ok | cancel))
                dd_sel_unpick(-1);
        } else if (dd_sel_can_accept() == 0) {
            if (pad[1] & (ok | cancel))
                se_play(0x6215);
        } else if (pad[1] & ok) {
            se_play(0x6213);
            menu_finish(&s->list);
        } else if (pad[1] & cancel) {
            dd_sel_unpick(-1);
            s->list.no_cancel = 1;
            s->done = 0;
        }
        break;
    case 1:
        if (dd_sel_sort_update() != 0)
            s->mode = 0;
        break;
    case 2:
        switch (dd_sel_opts_update()) {
        case 3:
            dd_sel_open_filter();
            s->mode = 3;
            break;
        case 2:
            dd_sel_open_sort();
            s->mode = 1;
            break;
        case 1:
            s->mode = 0;
            break;
        }
        break;
    case 3:
        if (dd_sel_filter_update() != 0) {
            s->mode = 0;
            if (s->list.count == 0) {
                s->done = 1;
                s->list.no_cancel = 0;
                se_play(0x6212);
            }
        }
        break;
    }
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_busy(void)
{
    if (dd_sel.list.state == 6)
        return 0;
    if (dd_sel.list.state == 1 || dd_sel.list.state == 0)
        return 0;
    return 1;
}
#endif

int dd_sel_active(void)
{
    return dd_sel.list.state == 3;
}

void dd_sel_set_minmax(int min, int max)
{
    dd_sel.min = min;
    dd_sel.max = max;
}

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_get_picked(int *out)
{
    if (out != 0)
        memcpy(out, dd_sel.picked, dd_sel.n_picked << 2);
    return dd_sel.n_picked;
}
#endif

void dd_sel_close(void)
{
    menu_finish(&dd_sel.list);
}

unsigned int *dd_sel_draw_number(unsigned int *list, int x, int y, int v);
int dd_sel_icons_of(int *out, int attr);
unsigned int *dd_sel_draw_icon(unsigned int *list, int x, int y, int icon);

/* a row of the list: picture, pick mark, name, frame, ATK/DEF and level */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void *dd_sel_draw_item(void *dl, int i, int x, int y)
{
    DdSel *s = &dd_sel;
    unsigned int *list = dl;
    int id = s->order[(unsigned short)i];
    int f = dv_0884C8E4(id);
    int uid;
    float r[12] = { 0 };
    int icons[2];
    unsigned short num[8];
    void *gim;
    int k, n, atk, def, lv;
    unsigned short *name;

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 1);
    uid = duel_0884C830(id);
    if (uid <= 0)
        uid = -1;
    r[0] = (float)(x + 4);
    r[9] = r[0];
    r[1] = (float)(y + 0x14);
    r[3] = (float)(x + 0x2C);
    r[6] = r[3];
    r[4] = (float)(y + 0x15);
    r[7] = (float)(y + 0x93);
    r[10] = r[7];
    list = dd_wline_draw(list, 0, 4, r, 1);
    gim = dd_gim_get(0x40005);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = draw_set_offsets(dl_ambient_color(list, 0xFFFFFFFF), 0x25, 0xF, 0x25, 0xF);
    list = draw_sprite(list, x + 5, y + 0x19, 0, (f & 0x1000) ? 0x58 : 0x30, 0x10, 0, 0);
    n = dd_sel_icons_of(icons, f);
    if (n == 1) {
        list = dd_sel_draw_icon(list, x + 0x10, y + 0x19, icons[0]);
    } else if (n == 2) {
        list = dd_sel_draw_icon(list, x + 8, y + 0x19, icons[0]);
        list = dd_sel_draw_icon(list, x + 0x18, y + 0x19, icons[1]);
    }
    list = dd_card_texture(list, uid);
    list = draw_set_offsets(list, 0x40, 0x40, 0x20, 0x2E);
    list = draw_sprite(list, x + 8, y + 0x2D, 0, 0, 0, 0, 0);
    k = -1;
    for (n = 0; n < s->n_picked; n++) {
        if (s->picked[n] == s->order[(unsigned short)i]) {
            k = n;
            break;
        }
    }
    if (k >= 0) {
        gim = dd_gim_get(0x40005);
        if (gim != 0)
            list = gim_set_texture(list, gim);
        list = dl_tex_func(list, 0, 1);
        list = dl_enable(list, 4);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = draw_set_offsets(list, 0x17, 0xE, 0x17, 0xE);
        list = draw_sprite(list, x + 0xD, y + 0x27, 0, 0x35, 0x69, 0, 0);
        if (s->max == 1) {
            list = func_088212C4(list, x + 0x12, y + 0x27, 0xB);
        } else {
            wtext_from_int(k + 1, num);
            font_text_set(list);
            font_set_size(0xC, 0xC);
            font_set_color(0xFF000000);
            font_draw_text((x + (0x31 - (font_measure_text(num) >> 6)) / 2) << 6, (y + 0x29) << 6, 0xA00, num);
            list = font_text_clear();
        }
    }
    if (uid > 0) {
        name = card_get_name(uid);
        font_text_set(list);
        font_set_size(0xC, 0xC);
        font_set_color(0xFF000000);
        font_draw_text((x + 4) << 6, (y + 0x5F) << 6, 0xA00, name);
        list = font_text_clear();
        switch (card_get_attribute_icon(uid)) {
        case 0:
        case 1:
        case 2:
        case 3:
            gim = dd_gim_get(0x40006);
            if (gim != 0)
                list = gim_set_texture(list, gim);
            list = draw_set_offsets(dl_ambient_color(list, 0xFFFFFFFF), 8, 8, 8, 8);
            list = draw_sprite(list, x + 6, y + 0x6E, 0, 0, 0x53, 0, 0);
            list = draw_sprite(list, x + 6, y + 0x7B, 0, 9, 0x53, 0, 0);
            atk = card_get_atk_q(uid);
            def = card_get_def_q(uid);
            lv = card_get_level(uid);
            if (atk == 0xFFFF)
                atk = -1;
            if (def == 0xFFFF)
                def = -1;
            list = dd_sel_draw_number(list, x + 0xF, y + 0x6C, atk);
            list = dd_sel_draw_number(list, x + 0xF, y + 0x79, def);
            list = cardlist_draw_stars(dl_ambient_color(list, 0xFFFFFFFF), x + 6, y + 0x86, lv, 0xFF000000);
            list = dl_ambient_color(list, 0xFFFFFFFF);
            break;
        case 7:
        case 8:
            list = dl_ambient_color(list, 0xFFFFFFFF);
            if (card_get_prop5_hi(uid) != 0) {
                list = card_draw_frame_special(list, x + 6, y + 0x6C, card_get_attribute(uid));
                list = cardlist_draw_icon(list, x + 0x19, y + 0x6C, card_get_attribute(uid), card_get_prop5_hi(uid));
            } else {
                list = card_draw_frame_special(list, x + 0x10, y + 0x6C, card_get_attribute(uid));
            }
            break;
        }
    }
    if (s->done == 0 && s->mode == 2 && (unsigned short)i == s->list.scroll + s->list.cursor) {
        s->sub.def.x = x + (0x31 - s->sub.def.w) / 2;
        s->sub.def.y = y + 0x32 - s->sub.def.h;
    }
    return list;
}
#endif

/* sort and filter bar, scroll bar and the OK button */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void *dd_sel_draw_after(void *dl, int x0, int y0, int w0, int h0)
{
    DdSel *s = &dd_sel;
    unsigned int *list;
    int x = s->list.def.x + s->list.def.pad_l;
    int y = menu_cursor_y(&s->list.menu, &s->list.def);
    int w = s->list.def.w;
    int cx = w - 9;
    unsigned short *t = dd_text(0x2D);
    void *gim;
    int px, bw, pos, size, tw, n;

    font_text_set(dl);
    font_set_size(0xC, 0xC);
    font_set_color(0xFF000000);
    font_draw_text((x + 8) << 6, (y + 2) << 6, 0x3C40, t);
    list = font_text_clear();
    px = font_get_cursor_x() >> 6;
    gim = dd_gim_get(0x40005);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dd_sel_draw_key(list, px + 0xC, y - 1, s->key);
    list = dd_sel_draw_dir(list, px + 0x1F, y - 1, s->dir);
    if (s->filter != 0) {
        t = dd_text(0x42);
        font_text_set(list);
        font_draw_text((px + 0x35) << 6, (y + 2) << 6, (0x1E0 - (px + 0x35)) << 6, t);
        list = font_text_clear();
        px = font_get_cursor_x();
        gim = dd_gim_get(0x40006);
        if (gim != 0)
            list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dd_sel_draw_filter_icon(list, (px >> 6) + 0xC, y - 1, s->filter);
    }
    n = s->list.count;
    bw = w - 0x27;
    if (n == 0) {
        pos = 0;
        size = bw;
    } else {
        pos = s->list.scroll * bw / n;
        size = s->list.rows * bw / n;
    }
    gim = dd_gim_get(0x40004);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = card_draw_glow(list, x + 9, y + 0x9E, 1, gfx_get_state()[1], pos > 0);
    list = cardlist_draw_bar(list, x + 0x13, y + 0x9E, bw, pos, size);
    list = card_draw_glow(list, x + 0x13 + bw, y + 0x9E, 0, gfx_get_state()[1], s->list.scroll < s->list.count - s->list.rows);
    cx = x + 4 + cx / 2;
    font_set_size(0xC, 0xC);
    if (s->min == 0 && s->max == 0)
        t = dd_text(0x31);
    else
        t = dd_text(0x16);
    tw = (font_measure_text(t) >> 6) + 2;
    list = menu_draw_button(list, &s->list, cx - 0x1E, y + 0xA8, 0, s->done != 0);
    font_set_size(0xC, 0xC);
    if (dd_sel_can_accept() != 0)
        font_set_color(0xFF000000);
    else
        font_set_color(0xFF808080);
    font_text_set(list);
    font_draw_text((cx - tw / 2) << 6, (y + 0xAF) << 6, tw << 6, t);
    return font_text_clear();
}
#endif

void dd_sel_show(int i)
{
    int uid = duel_0884C830(dd_sel.order[i]);

    if (uid < 1)
        uid = -1;
    dd_info_select(uid);
}

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_toggle(int i)
{
    DdSel *s = &dd_sel;
    int k;

    for (k = 0; k < s->n_picked; k++) {
        if (s->order[i] == s->picked[k]) {
            if (k < s->n_picked - 1)
                memcpy(&s->picked[k], &s->picked[k + 1], (s->n_picked - k - 1) * 4);
            s->n_picked--;
            se_play(0x6214);
            return;
        }
    }
    if (s->n_picked < s->max) {
        s->picked[s->n_picked] = s->order[i];
        s->n_picked++;
        se_play(0x6213);
        if (s->n_picked >= s->max) {
            s->done = 1;
            s->list.no_cancel = 0;
        }
    } else {
        se_play(0x6215);
    }
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_unpick(int i)
{
    if (dd_sel.n_picked != 0) {
        dd_sel.n_picked--;
        se_play(0x6214);
    } else if (dd_sel.list.cancel_blocked) {
        se_play(0x6215);
    } else {
        menu_finish(&dd_sel.list);
        se_play(0x6214);
    }
}
#endif

/* end of the prompt: the picks go back to the engine */
void dd_sel_done_cb(void)
{
    int picks[512];
    int n = dd_sel_get_picked(picks);
    int i;

    dv_0884C7C0(picks[0]);
    dv_w624_set(n);
    for (i = 0; i < n; i++)
        dv_0884CB8C(picks[i], i + 1);
}

/* ATK/DEF digits right-aligned at x + 0x15 ("?" when unknown) */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
unsigned int *dd_sel_draw_number(unsigned int *list, int x, int y, int v)
{
    char d[4];
    unsigned short q[2];

    font_text_set(list);
    font_set_size(10, 0xC);
    if (v < 0) {
        q[0] = dd_sel_qmark[0];
        q[1] = dd_sel_qmark[1];
        font_draw_text(x << 6, y << 6, (0x1E0 - x) << 6, q);
    } else {
        x += 0x15;
        if (v < 0)
            v = 0;
        if (v > 9999)
            v = 9999;
        do {
            memset(d, 0, sizeof(d));
            d[0] = v % 10 + '0';
            font_draw_text(x << 6, y << 6, (0x1E0 - x) << 6, (unsigned short *)d);
            v /= 10;
            x -= 7;
        } while (v > 0);
    }
    return font_text_clear();
}
#endif

/* up to two icons of a candidate's flags */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_icons_of(int *out, int attr)
{
    int tab[11][2];
    int n = 0;
    int i;

    memcpy(tab, dd_sel_icons, sizeof(tab));
    for (i = 0; i < 11; i++) {
        if (attr & tab[i][0]) {
            n++;
            *out++ = tab[i][1];
            if (n >= 2)
                return n;
        }
    }
    return n;
}
#endif

unsigned int *dd_sel_draw_icon(unsigned int *list, int x, int y, int icon)
{
    void *gim = dd_gim_get(0x40005);

    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 0x10, 0x10, 0x10, 0x10);
    return draw_sprite(list, x, y, 0, (icon % 8) << 4, (icon / 8) << 4, 0, 0);
}


#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_open_sort(void)
{
    DdSel *s = &dd_sel;

    func_08825B9C(&s->sub);
    s->sub.def.title = dd_text(0x21);
    s->sub.cancel_blocked = 0;
    s->sub.style = 3;
    s->sub.count = 10;
    s->sub.rows = 10;
    s->sub.draw_item = (MenuDrawItemFn)dd_sel_sort_draw_item;
    s->sub.item_h = 0x11;
    s->sub.item_w = 0x11;
    menu_open(&s->sub);
    s->sub.def.x = 0xF0;
    s->sub.def.y = 0x32;
    memcpy(s->saved, s->order, s->n << 2);
    s->n_saved = s->n;
    s->dir_saved = s->dir;
    s->key_saved = s->key;
    se_play(0x6216);
}
#endif

void dd_sel_sub_reset(void)
{
    menu_reset(&dd_sel.sub);
}

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_sort_update(void)
{
    DdSel *s = &dd_sel;
    int r = menu_update(&s->sub);
    unsigned int *pad;
    int k, flip, moved;

    if (r != -1) {
        if (r == -2) {
            memcpy(s->order, s->saved, 0x800);
            s->key = s->key_saved;
            s->dir = s->dir_saved;
        }
        dd_sel_sub_reset();
        return 1;
    }
    pad = dd_pad();
    if (menu_item_is_open(&s->sub.menu, &s->sub.def) == 0)
        return 0;
    flip = (pad[1] & 0x50) != 0;
    if (flip) {
        s->dir = !s->dir;
        se_play(0x6212);
    }
    k = s->sub.scroll + s->sub.cursor;
    moved = s->key != k;
    if (moved)
        s->key = k;
    if (moved || flip) {
        memcpy(s->order, s->saved, s->n << 2);
        dd_sel_sort(s->order, s->n, s->key, s->dir);
    }
    return 0;
}
#endif

void *dd_sel_sub_draw(void *list)
{
    menu_draw(&list, &dd_sel.sub);
    return list;
}

/* an entry of the sort window; the current key also shows both directions */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void *dd_sel_sort_draw_item(void *dl, int i, int x, int y)
{
    unsigned int *list = dl_ambient_color(dl, 0xFFFFFFFF);
    void *gim = dd_gim_get(0x40005);
    float r[12] = { 0 };
    int k;

    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = dd_sel_draw_key(list, x, y, (unsigned short)i);
    if ((unsigned short)i != dd_sel.sub.scroll + dd_sel.sub.cursor)
        return list;
    r[0] = (float)(x + 8);
    r[1] = (float)(y + 0x1A);
    r[3] = (float)(x + 0x19);
    r[4] = r[1];
    r[6] = r[3];
    r[7] = (float)(y + 0x2B);
    r[9] = r[0];
    r[10] = r[7];
    list = dl_ambient_color(list, 0xFF808080);
    list = dd_wline_sprite(list, 0, &r[0], &r[6]);
    list = dd_wline_draw(list, 0, 4, r, 1);
    gim = dd_gim_get(0x40005);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = dd_sel_draw_dir(list, (int)r[0], (int)r[1], dd_sel.dir == 0);
    for (k = 0; k < 12; k += 3) {
        r[k] -= 4.0f;
        r[k + 1] -= 4.0f;
    }
    list = dl_ambient_color(list, 0x80FFFFFF);
    list = dd_wline_sprite(list, 0, &r[0], &r[6]);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dd_wline_draw(list, 0, 4, r, 1);
    gim = dd_gim_get(0x40005);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    return dd_sel_draw_dir(list, (int)r[0], (int)r[1], dd_sel.dir);
}
#endif

unsigned int *dd_sel_draw_key(unsigned int *list, int x, int y, int key)
{
    list = draw_set_offsets(list, 0x11, 0x11, 0x11, 0x11);
    return draw_sprite(list, x, y, 0, dd_sel_key_uv[key].u, dd_sel_key_uv[key].v, 0, 0);
}

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
unsigned int *dd_sel_draw_dir(unsigned int *list, int x, int y, int dir)
{
    list = draw_set_offsets(list, 0x11, 0x11, 0x11, 0x11);
    return draw_sprite(list, x, y, 0, dd_sel_dir_uv[dir != 0].u, dd_sel_dir_uv[dir != 0].v, 0, 0);
}
#endif

typedef struct DdSelKey {
    unsigned short id;
    unsigned short idx;
    int key;
} DdSelKey;

/* sorts the entries by key (ties by index) */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_sort(int *list, int n, int key, int dir)
{
    DdSelKey *k;
    DdSelKey *p;
    int i;

    if (n == 0)
        return;
    k = dd_alloc(n << 3);
    for (i = 0, p = k; i < n; i++, p++) {
        p->id = list[i];
        p->idx = i;
        p->key = dd_sel_key_of(list[i], dd_sel.ids[list[i]], key);
    }
    if (dir == 0)
        qsort(k, n, 8, dd_sel_cmp_down);
    else
        qsort(k, n, 8, dd_sel_cmp_up);
    for (i = 0; i < n; i++)
        list[i] = k[i].id;
    if (k != 0)
        dd_free(k);
}
#endif

/* sort value of a card for a key (0 list order, 1 sort, 2 kind, 3 level, 4 ATK, 5 DEF, 6 type,
   7 attribute, 8 spell/trap kind, 9 limit) */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_key_of(int i, int id, int key)
{
    int a;

    switch (key) {
    case 0:
        if (dd_sel.reverse != 0)
            i = 0x200 - i;
        return i;
    case 1:
        return card_get_sort(id);
    case 2:
        return card_get_attribute_icon(id);
    case 9:
        return card_get_limit(id);
    }
    a = card_get_attribute_icon(id);
    if (a == 8) {
        if (key == 8)
            return dd_sel_spell_order[card_get_prop5_hi(id)];
    } else if (a == 7) {
        if (key == 8)
            return dd_sel_trap_order[card_get_prop5_hi(id)];
    } else {
        switch (key) {
        case 7:
            return card_get_kind(id);
        case 6:
            return card_get_type(id);
        case 5:
            a = card_get_def(id);
            if (a != 0xFFFF)
                return 0xFFFF - a;
            return 0x10000;
        case 4:
            a = card_get_atk(id);
            if (a != 0xFFFF)
                return 0xFFFF - a;
            return 0x10000;
        case 3:
            return 0xFFFF - card_get_level(id);
        }
    }
    return 0xFFFF7FFF;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_cmp_down(const void *pa, const void *pb)
{
    const DdSelKey *a = pa;
    const DdSelKey *b = pb;

    if (a->key == b->key) {
        if (a->idx < b->idx)
            return -1;
        if (b->idx < a->idx)
            return 1;
        return 0;
    }
    if (a->key == 0xFFFF7FFF)
        return 1;
    if (b->key == 0xFFFF7FFF)
        return -1;
    if (a->key < b->key)
        return -1;
    if (b->key < a->key)
        return 1;
    return 0;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_cmp_up(const void *pa, const void *pb)
{
    const DdSelKey *a = pa;
    const DdSelKey *b = pb;

    if (a->key == b->key) {
        if (b->idx < a->idx)
            return -1;
        if (a->idx < b->idx)
            return 1;
        return 0;
    }
    if (a->key == 0xFFFF7FFF)
        return 1;
    if (b->key == 0xFFFF7FFF)
        return -1;
    if (a->key < b->key)
        return 1;
    if (b->key < a->key)
        return -1;
    return 0;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_can_accept(void)
{
    if (dd_sel.min == 0 && dd_sel.max == 0)
        return 1;
    if (dd_sel.n_picked < dd_sel.min)
        return 0;
    return 1;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_open_opts(void)
{
    DdSel *s = &dd_sel;

    dd_sel_build_opts();
    func_08825B9C(&s->sub);
    s->sub.def.pad_t = 2;
    s->sub.def.pad_b = 2;
    s->sub.def.pad_l = 2;
    s->sub.def.pad_r = 2;
    s->sub.def.title = 0;
    s->sub.cancel_blocked = 0;
    s->sub.style = 2;
    s->sub.def.show_title = 0;
    s->sub.draw_item = (MenuDrawItemFn)dd_sel_opts_draw_item;
    s->sub.count = s->n_opts;
    s->sub.rows = s->n_opts;
    s->sub.item_h = 0x11;
    s->sub.item_w = s->opts_w + 0x12;
    menu_open(&s->sub);
    s->sub.def.style2 = 1;
    s->sub.def.z = s->sub.def.w / 2;
    s->sub.def.x = s->list.def.x + s->list.def.w / 2 - s->sub.def.w / 2;
    s->sub.def.y = s->list.def.y + s->list.def.h - 0x1E - s->sub.def.title_w;
    se_play(0x6216);
}
#endif

void dd_sel_sub_reset2(void)
{
    menu_reset(&dd_sel.sub);
}

/* 0 open, 1 closed, 2 sort, 3 filter */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_opts_update(void)
{
    DdSel *s = &dd_sel;
    int r = menu_update(&s->sub);
    int ret;

    if (r == -1)
        return 0;
    ret = 1;
    if (r != -2) {
        switch (s->opts[r]) {
        case 4:
            hl_open();
            break;
        case 3:
            opt_open();
            break;
        case 2:
            ret = 3;
            break;
        case 1:
            ret = 2;
            break;
        case 0:
            dd_mode_set_card(s->ids[s->order[s->list.scroll + s->list.cursor]]);
            dd_mode_push(5);
            dd_mode_set_flag(0x31);
            break;
        }
    }
    dd_sel_sub_reset2();
    return ret;
}
#endif

void *dd_sel_sub_draw2(void *list)
{
    menu_draw(&list, &dd_sel.sub);
    return list;
}

void *dd_sel_opts_draw_item(void *dl, int i, int x, int y)
{
    DdSelOpt *o = &dd_sel_opts[dd_sel.opts[(unsigned short)i]];
    unsigned int *list = dl_ambient_color(dl, 0xFFFFFFFF);
    unsigned short *t;

    list = dl_set_tex_filter(list, 0, 1);
    list = gim_set_texture(list, dd_gim_get(0x40005));
    list = draw_set_offsets(list, 0xD, 0xD, 0xD, 0xD);
    list = draw_sprite(list, x, y + 1, 0, o->u, o->v, 0, 0);
    font_set_size(0xC, 0xC);
    t = dd_text(o->text);
    font_text_set(list);
    font_set_color(0xFF000000);
    font_draw_text((x + 0xF) << 6, (y + 1) << 6, (0x1E0 - (x + 0xF)) << 6, t);
    return font_text_clear();
}

/* entries of the options window: card info (when the card is known), sort, filter, ... */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_build_opts(void)
{
    DdSel *s = &dd_sel;
    int n = 0;
    int id, i, w;

    if (s->done == 0) {
        id = s->order[s->list.scroll + s->list.cursor];
        dv_0884C8E4(id);
        if (duel_0884C830(id) >= 1) {
            s->opts[0] = 0;
            n = 1;
        }
    }
    s->opts[n] = 1;
    s->opts[n + 1] = 2;
    n += 2;
    if (dd_is_tag_duel() == 0)
        s->opts[n++] = 3;
    if (hl_allowed() != 0)
        s->opts[n++] = 4;
    s->opts_w = 0;
    s->n_opts = n;
    font_set_size(0xC, 0xC);
    for (i = 0; i < s->n_opts; i++) {
        w = font_measure_text(dd_text(dd_sel_opts[s->opts[i]].text)) >> 6;
        if (w <= s->opts_w)
            w = s->opts_w;
        s->opts_w = w;
    }
}
#endif

/* entries passing filter (0 all, else the attribute icon of dd_sel_filter_icon) */
#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_filter(int *out, int n, int filter)
{
    int want, k, i;

    if (filter != 0) {
        want = dd_sel_filter_icon[filter];
        k = 0;
        for (i = 0; i < n; i++) {
            if (card_get_attribute_icon(dd_sel.ids[i]) == want) {
                *out++ = i;
                k++;
            }
        }
        return k;
    }
    for (i = 0; i < n; i++)
        out[i] = i;
    return n;
}
#endif


#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void dd_sel_open_filter(void)
{
    DdSel *s = &dd_sel;

    func_08825B9C(&s->sub);
    s->sub.def.title = dd_text(0x42);
    s->sub.cancel_blocked = 0;
    s->sub.style = 3;
    s->sub.count = 7;
    s->sub.rows = 7;
    s->sub.draw_item = (MenuDrawItemFn)dd_sel_filter_draw_item;
    s->sub.item_h = 0x11;
    s->sub.item_w = 0x11;
    menu_open(&s->sub);
    s->sub.def.x = 0xF0;
    s->sub.def.y = 0x32;
    memcpy(s->saved, s->order, s->n << 2);
    s->filter_saved = s->filter;
    se_play(0x6216);
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
int dd_sel_filter_update(void)
{
    DdSel *s = &dd_sel;
    int r = menu_update(&s->sub);
    int f;

    if (r == -1) {
        f = s->sub.scroll + s->sub.cursor;
        if (s->filter == f)
            return 0;
        s->filter = f;
        s->n = dd_sel_filter(s->filt, s->n_all, f);
        memcpy(s->order, s->filt, s->n << 2);
        dd_sel_sort(s->order, s->n, s->key, s->dir);
        s->list.count = s->n;
        if ((unsigned short)s->n <= s->list.scroll + s->list.cursor) {
            s->list.cursor = 0;
            s->list.scroll = 0;
        }
        return 0;
    }
    if (r == -2) {
        s->n = dd_sel_filter(s->filt, s->n_all, s->filter_saved);
        memcpy(s->order, s->filt, s->n << 2);
        dd_sel_sort(s->order, s->n, s->key, s->dir);
        s->list.count = s->n;
        s->filter = s->filter_saved;
        if ((unsigned short)s->n <= s->list.scroll + s->list.cursor) {
            s->list.cursor = 0;
            s->list.scroll = 0;
        }
    }
    dd_sel_sub_reset();
    return 1;
}
#endif

void *dd_sel_sub_draw3(void *list)
{
    menu_draw(&list, &dd_sel.sub);
    return list;
}

#ifdef NON_MATCHING /* forma reconstruída; alocação/ordem de blocos diferente */
void *dd_sel_filter_draw_item(void *dl, int i, int x, int y)
{
    unsigned int *list = dl_ambient_color(dl, 0xFFFFFFFF);
    void *gim = dd_gim_get(0x40006);

    if (gim != 0)
        list = gim_set_texture(list, gim);
    return dd_sel_draw_filter_icon(list, x, y, (unsigned short)i);
}
#endif

unsigned int *dd_sel_draw_filter_icon(unsigned int *list, int x, int y, int f)
{
    list = draw_set_offsets(list, 0x11, 0x11, 0x11, 0x11);
    return draw_sprite(list, x, y, 0, dd_sel_filter_uv[f].u, dd_sel_filter_uv[f].v, 0, 0);
}
