// @range 0x08823674-0x08825088
// @data 0x08876208-0x08876228
/* Choice windows opened in the message window (dd_msg_win): the list of the duel's choices
   (dv_list2, tabs with "n/m" labels and the text of the entry under the cursor), the attribute and
   type pickers (dd_pick grids), heads/tails and the number choice. Their answers go to the duel
   through dv_w624_set. */
#include "dd_field_ui.h"

typedef struct DdChoice {              /* entry of the duel's choice list */
    unsigned short text : 13;          /* dialog text id */
    unsigned short b13 : 1;
    unsigned short on : 1;             /* can be chosen */
    unsigned short b15 : 1;
    unsigned short pad;
} DdChoice;

typedef struct DdChoices {             /* dv_list2() */
    int pad[3];
    int n;                             /* +0x0C */
    DdChoice e[1];                     /* +0x10 */
} DdChoices;

typedef unsigned int *(*DdPickItemFn)(unsigned int *list, int x, int y, int index);
typedef unsigned int *(*DdPickMarkFn)(unsigned int *list, int x, int y, int w, int h);

typedef struct DdPickCfg {             /* dd_pick.c, 0x2C */
    int first;
    int last;
    int cols;
    unsigned short *title;
    unsigned short *(*name)(int index);
    DdPickItemFn draw_item;
    DdPickMarkFn draw_mark;
    int cell_w;
    int cell_h;
    int min_pick;
    int max_pick;
} DdPickCfg;

extern int dd_prompt_none;             /* 0x08C58810 no entry of the list can be chosen */
extern MenuWidget *dd_prompt_list;     /* 0x08C58814 */
extern int dd_attr_mask;               /* 0x08C58818 attributes offered (bit per attribute) */
extern MenuWidget *dd_attr_pick;       /* 0x08C5881C */
extern MenuWidget *dd_type_pick;       /* 0x08C58820 */
extern unsigned int dd_type_mask;      /* 0x08C58824 types offered */
extern MenuWidget *dd_coin_w;          /* 0x08C58828 */

int dv_w624_set();
DdChoices *dv_list2(void);
unsigned short *dd_text(int id);
void *dd_gim_get(int id);
int dd_input_on(void);
unsigned int *dd_pad(void);
void dd_mode_push(int m);
void dd_mode_set_flag(int f);
unsigned int *dd_wline_draw(unsigned int *list, int style, int n, float *pts, int closed);
unsigned int *dd_coin_tex(float scale, unsigned int *list, int x, int y, int w, int h, int back);
void dd_pick_init(DdPickCfg *cfg);
void dd_pick_setup(MenuWidget *w);
void dd_pick_update(MenuWidget *w);
unsigned int dd_pick_mask();
int dd_pick_width(MenuWidget *w);
int dd_pick_height(MenuWidget *w);
unsigned int *dd_pick_draw();
void dd_pick_open_confirm(void);
unsigned short *dlg_table_sel(int i);
unsigned short *card_db_table_sel(int i);
unsigned char *syspack_get(void);
void *ehp_find(void *pack, const char *name, int *size);
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);
int wsprintf(unsigned short *out, const unsigned short *fmt, ...);
void func_08825824(unsigned short *title, unsigned short *text, int kind, void (*setup)(MenuWidget *w, int open));
int func_08825728(void);
void func_088259D8(void);
void func_088259FC(void (*fn)(void));
void func_08825A08(int (*fn)(void));
int func_08825A14(void);
void func_08825A70(int v);
void func_08823724(MenuWidget *w, int open);
void func_08823808(void);
void func_0882382C(unsigned int *dl, unsigned short i, int x, int y);
void *func_08823D84(void *list);
int func_08823D8C(void);
void func_08823F44(MenuWidget *w, int open);
void func_08823FF4(void);
int func_0882401C(void);
unsigned int *func_0882404C(unsigned int *dl, int x, int y);
unsigned int *func_08824374(unsigned int *dl, int x, int y, int idx);
unsigned int *func_08824484(unsigned int *dl, int x, int y);
void func_0882464C(MenuWidget *w, int open);
void func_088246FC(void);
int func_08824764(void);
unsigned int *func_08824794(unsigned int *dl, int x, int y);
unsigned int *func_08824ABC(unsigned int *dl, int x, int y, int idx);
unsigned int *func_08824C08(unsigned int *dl, int x, int y);
void func_08824D18(MenuWidget *w, int open);
void func_08824D58(void);
void func_08824D7C(unsigned int *dl, int x, int y);
void func_08824FF0(MenuWidget *w, int open);
void func_0882504C(void);
void func_088250E8(void *dl, int x, int y, int w, int h);

/* choice among the duel's list (dv_list2); nothing to choose answers 0 at once */
void func_08823674(unsigned short *text)
{
    dd_prompt_none = 0;
    func_08825824(dd_text(28), text, 0x30, func_08823724);
    if (dd_prompt_list == 0 || dd_prompt_none != 0) {
        func_088259D8();
        dv_w624_set(0);
    } else {
        dd_mode_push(4);
        dd_mode_set_flag(0x31);
        func_088259FC(func_08823808);
        func_08825A08(func_08823D8C);
        se_play(0x621A);
    }
}

/* window setup: one tab per entry, the entries that can't be chosen disabled */
void func_08823724(MenuWidget *w, int open)
{
    DdChoices *c;
    unsigned int off;
    int n_on;
    int n;
    int i;

    if (open) {
        c = dv_list2();
        off = 0;
        n_on = 0;
        n = dv_list2()->n;
        for (i = 0; i < n; i++) {
            if (!c->e[i].on)
                off |= 1 << i;
            else
                n_on++;
        }
        w->def.pad_b = 80;
        w->count = n;
        w->rows = n;
        w->disabled = off;
        w->item_w = 70;
        w->item_h = 18;
        w->draw_item = (MenuDrawItemFn)func_0882382C;
        w->draw_cursor = (MenuDrawCursorFn)func_08823D84;
        dd_prompt_list = w;
        if (n_on == 0)
            dd_prompt_none = 1;
    }
}

void func_08823808(void)
{
    dv_w624_set(func_08825A14());
}

/* tab i of the list (lit under the cursor, "i/n" label); tab 0 also draws the box with the text of
   the entry under the cursor */
void func_0882382C(unsigned int *dl, unsigned short i, int x, int y)
{
    MenuWidget *w = dd_prompt_list;
    float pts[8][3];
    unsigned short label[64];
    unsigned short *text;
    DdChoices *c;
    int k;
    int frame = 0;
    int tw, th;
    int lh, oy, tx, ty;
    void *gim;

    if (i == 0) {
        tw = w->item_w * w->count - 9;
        th = w->def.y + w->def.h - w->def.pad_t - 29 - (y + 3);
        pts[0][0] = x + 4;
        pts[0][1] = y + 22;
        pts[1][0] = pts[0][0] + tw;
        pts[1][1] = pts[0][1];
        pts[2][0] = pts[1][0];
        pts[2][1] = pts[0][1] + th;
        pts[3][0] = pts[0][0];
        pts[3][1] = pts[2][1];
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = dd_wline_draw(dl, 0, 4, &pts[0][0], 1);
        font_text_set(dl);
        font_set_size(12, 12);
        c = dv_list2();
        k = dd_prompt_list->cursor;
        text = dlg_get_text(c->e[k].text);
        if (c->e[k].on)
            font_set_color(0xFF000000);
        else
            font_set_color(0xFF666666);
        font_draw_wtext_ex((int)pts[0][0] << 6, (int)pts[0][1] << 6, tw << 6, th << 6, text, -1);
        dl = font_text_clear();
    }
    gim = dd_gim_get(0x4001A);
    if (gim)
        dl = gim_set_texture(dl, gim);
    if (dd_prompt_list == 0 || dd_prompt_list->cursor != i) {
        dl = dl_ambient_color(dl, 0xFF808080);
        frame = 2;
        dl = draw_set_offsets(dl, 8, 18, 8, 18);
        if (i == 0)
            dl = draw_sprite(dl, x, y + 3, 0, 8, 0, 0, 0);
        else
            dl = draw_sprite(dl, x, y + 3, 0, 24, 0, 0, 0);
        if (i == w->count - 1)
            dl = draw_sprite(dl, x + w->item_w - 8, y + 3, 0, 56, 0, 0, 0);
        else
            dl = draw_sprite(dl, x + w->item_w - 8, y + 3, 0, 40, 0, 0, 0);
        dl = draw_set_offsets(dl, 1, 18, w->item_w - 16, 18);
        dl = draw_sprite(dl, x + 8, y + 3, 0, 12, 0, 0, 0);
    } else {
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
        dl = draw_set_offsets(dl, 8, 18, 8, 18);
        if (i == 0)
            dl = draw_sprite(dl, x, y + 3, 0, 0, 0, 0, 0);
        else
            dl = draw_sprite(dl, x, y + 3, 0, 16, 0, 0, 0);
        if (i == w->count - 1)
            dl = draw_sprite(dl, x + w->item_w - 8, y + 3, 0, 48, 0, 0, 0);
        else
            dl = draw_sprite(dl, x + w->item_w - 8, y + 3, 0, 32, 0, 0, 0);
        dl = draw_set_offsets(dl, 1, 18, w->item_w - 16, 18);
        dl = draw_sprite(dl, x + 8, y + 3, 0, 4, 0, 0, 0);
    }
    wsprintf(label, dd_text(27), i + 1);
    font_set_size(12, 12);
    tw = font_measure_text(label) >> 6;
    lh = (font_line_height() >> 6) / 2;
    oy = frame + (9 - lh);
    ty = y + 3 + oy;
    tx = x + (w->item_w - tw) / 2;
    dl = dl_scissor(dl, tx, ty, tw + 1, 15 - oy);
    dl = dl_enable(dl, 2);
    font_set_color(0xFF00354A);
    font_text_set(dl);
    font_draw_text(tx << 6, ty << 6, (480 - tx) << 6, label);
    dl_disable(font_text_clear(), 2);
}

/* no cursor drawing */
void *func_08823D84(void *list)
{
    return list;
}

/* window update: L/R also move the cursor */
int func_08823D8C(void)
{
    MenuWidget *w = dd_prompt_list;
    unsigned int *pad;

    if (dd_input_on())
        func_08825A70(menu_update(w));
    if (w->state == 3) {
        pad = dd_pad();
        if (pad[1] & 0x100) {
            menu_cursor_prev(w);
            se_play(0x6212);
        } else if (pad[1] & 0x200) {
            menu_cursor_next(w);
            se_play(0x6212);
        }
    }
    return !func_08825728();
}

/* attribute picker: n attributes among those of mask */
void func_08823E34(unsigned short *text, int n, int mask)
{
    unsigned short *title = dd_text(28);
    DdPickCfg cfg;

    dd_attr_mask = mask;
    memset(&cfg, 0, sizeof(DdPickCfg));
    cfg.first = 1;
    cfg.last = 7;
    cfg.cols = 6;
    cfg.title = dd_text(31);
    cfg.name = dlg_table_sel;
    cfg.draw_item = func_08824374;
    cfg.draw_mark = (DdPickMarkFn)func_08824484;
    cfg.cell_w = 25;
    cfg.cell_h = 25;
    cfg.min_pick = n;
    cfg.max_pick = n;
    dd_pick_init(&cfg);
    func_08825824(title, text, 0x30, func_08823F44);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_08825A08(func_0882401C);
    func_088259FC(func_08823FF4);
    se_play(0x621A);
}

/* window setup of the attribute picker: sized to the grid */
void func_08823F44(MenuWidget *w, int open)
{
    if (open) {
        dd_attr_pick = w;
        dd_pick_setup(w);
        w->draw_all = (MenuDrawRectFn)func_0882404C;
        w->disabled = ~(dd_attr_mask >> 1);
        w->def.auto_size = 0;
        w->def.w = w->def.pad_l + w->def.pad_r + dd_pick_width(w) + 92;
        w->def.h = w->def.font_size + (w->def.pad_t + w->def.pad_b) + dd_pick_height(w) + 42;
    } else {
        dd_pick_open_confirm();
    }
}

void func_08823FF4(void)
{
    dv_w624_set(dd_pick_mask(dd_attr_pick));
}

int func_0882401C(void)
{
    dd_pick_update(dd_attr_pick);
    return !func_08825728();
}

/* content of the attribute picker: frame, title icon and text, then the grid */
unsigned int *func_0882404C(unsigned int *dl, int x, int y)
{
    static char window_gim[] = "window01.gim";
    int pw = dd_pick_width(dd_attr_pick);
    int ph = dd_pick_height(dd_attr_pick);
    float pts[6][3] = { 0 };
    void *gim;
    unsigned short *text;
    int w, h;

    pts[0][0] = x + 28;
    pts[0][1] = y + 7;
    pts[1][0] = x + 45 + pw;
    pts[1][1] = y + 7;
    pts[2][0] = x + 45 + pw;
    pts[2][1] = y + 26 + ph;
    pts[3][0] = x + 45;
    pts[3][1] = y + 26 + ph;
    pts[4][0] = x + 45;
    pts[4][1] = y + 24;
    pts[5][0] = x + 28;
    pts[5][1] = y + 24;
    gim = ehp_find(syspack_get(), window_gim, 0);
    if (gim) {
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, gim);
    }
    w = 2.0f + (pts[1][0] - pts[0][0]);
    h = 1.0f + (pts[5][1] - pts[0][1]);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = draw_set_offsets(dl, 1, 1, w, h);
    dl = draw_sprite(dl, pts[0][0] - 1.0f, pts[0][1] - 1.0f, 0, 54, 101, 0, 0);
    dl = dd_wline_draw(dl, 0, 6, &pts[0][0], 1);
    gim = dd_gim_get(0x40005);
    if (gim)
        dl = gim_set_texture(dl, gim);
    dl = draw_set_offsets(dl, 17, 17, 17, 17);
    dl = draw_sprite(dl, pts[0][0], pts[0][1], 0, 85, 32, 0, 0);
    text = dd_text(29);
    if (text) {
        font_set_size(12, 12);
        font_set_color(0xFF202020);
        font_text_set(dl);
        font_draw_text((int)(3.0f + (17.0f + pts[0][0])) << 6, (int)(2.0f + pts[0][1]) << 6,
                       (480 - (int)(3.0f + (17.0f + pts[0][0]))) << 6, text);
        dl = font_text_clear();
    }
    return dd_pick_draw(dl, dd_attr_pick, x + 45, y + 24, -1, -1);
}

/* attribute icon idx, dimmed when not offered */
unsigned int *func_08824374(unsigned int *dl, int x, int y, int idx)
{
    int u = (idx - 1) * 17;
    void *gim = dd_gim_get(0x40004);

    if (gim) {
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, gim);
    }
    if ((1 << idx) & dd_attr_mask)
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
    else
        dl = dl_ambient_color(dl, 0x80808080);
    dl = draw_set_offsets(dl, 17, 17, 17, 17);
    return draw_sprite(dl, x, y, 0, u, 68, 0, 0);
}

/* mark of a picked entry */
unsigned int *func_08824484(unsigned int *dl, int x, int y)
{
    void *gim = dd_gim_get(0x40005);

    if (gim) {
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, gim);
    }
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = draw_set_offsets(dl, 25, 24, 25, 24);
    return draw_sprite(dl, x, y, 0, 28, 93, 0, 0);
}

/* monster type picker: one type among those of mask */
void func_08824544(unsigned short *text, unsigned int mask)
{
    unsigned short *title = dd_text(28);
    DdPickCfg cfg;

    memset(&cfg, 0, sizeof(DdPickCfg));
    cfg.first = 1;
    cfg.last = 21;
    cfg.cols = 5;
    cfg.title = dd_text(31);
    cfg.name = card_db_table_sel;
    cfg.draw_item = func_08824ABC;
    cfg.draw_mark = (DdPickMarkFn)func_08824C08;
    cfg.cell_w = 25;
    cfg.cell_h = 25;
    cfg.min_pick = 1;
    cfg.max_pick = 1;
    dd_pick_init(&cfg);
    dd_type_mask = mask;
    func_08825824(title, text, 0x30, func_0882464C);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_08825A08(func_08824764);
    func_088259FC(func_088246FC);
    se_play(0x621A);
}

/* window setup of the type picker */
void func_0882464C(MenuWidget *w, int open)
{
    if (open) {
        dd_type_pick = w;
        dd_pick_setup(w);
        w->draw_all = (MenuDrawRectFn)func_08824794;
        w->disabled = ~(dd_type_mask >> 1);
        w->def.auto_size = 0;
        w->def.w = w->def.pad_l + w->def.pad_r + dd_pick_width(w) + 92;
        w->def.h = w->def.font_size + (w->def.pad_t + w->def.pad_b) + dd_pick_height(w) + 42;
    } else {
        dd_pick_open_confirm();
    }
}

/* answer: the picked type (lowest bit 1-20 of the mask) */
void func_088246FC(void)
{
    int r = 0;
    unsigned int m = dd_pick_mask(dd_type_pick);
    int i;

    for (i = 1; i <= 20; i++) {
        if (m & (1 << i)) {
            r = i;
            break;
        }
    }
    dv_w624_set(r);
}

int func_08824764(void)
{
    dd_pick_update(dd_type_pick);
    return !func_08825728();
}

/* content of the type picker */
unsigned int *func_08824794(unsigned int *dl, int x, int y)
{
    static char window_gim[] = "window01.gim";
    int pw = dd_pick_width(dd_type_pick);
    int ph = dd_pick_height(dd_type_pick);
    float pts[6][3] = { 0 };
    void *gim;
    unsigned short *text;
    int w, h;

    pts[0][0] = x + 28;
    pts[0][1] = y + 7;
    pts[1][0] = x + 45 + pw;
    pts[1][1] = y + 7;
    pts[2][0] = x + 45 + pw;
    pts[2][1] = y + 26 + ph;
    pts[3][0] = x + 45;
    pts[3][1] = y + 26 + ph;
    pts[4][0] = x + 45;
    pts[4][1] = y + 24;
    pts[5][0] = x + 28;
    pts[5][1] = y + 24;
    gim = ehp_find(syspack_get(), window_gim, 0);
    if (gim) {
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, gim);
    }
    w = 2.0f + (pts[1][0] - pts[0][0]);
    h = 1.0f + (pts[5][1] - pts[0][1]);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = draw_set_offsets(dl, 1, 1, w, h);
    dl = draw_sprite(dl, pts[0][0] - 1.0f, pts[0][1] - 1.0f, 0, 54, 101, 0, 0);
    dl = dd_wline_draw(dl, 0, 6, &pts[0][0], 1);
    gim = dd_gim_get(0x40005);
    if (gim)
        dl = gim_set_texture(dl, gim);
    dl = draw_set_offsets(dl, 17, 17, 17, 17);
    dl = draw_sprite(dl, pts[0][0], pts[0][1], 0, 102, 32, 0, 0);
    text = dd_text(30);
    if (text) {
        font_set_size(12, 12);
        font_set_color(0xFF202020);
        font_text_set(dl);
        font_draw_text((int)(3.0f + (17.0f + pts[0][0])) << 6, (int)(2.0f + pts[0][1]) << 6,
                       (480 - (int)(3.0f + (17.0f + pts[0][0]))) << 6, text);
        dl = font_text_clear();
    }
    return dd_pick_draw(dl, dd_type_pick, x + 45, y + 24, -1, -1);
}

/* type icon idx (7 per row), dimmed when not offered */
unsigned int *func_08824ABC(unsigned int *dl, int x, int y, int idx)
{
    int u = (idx - 1) % 7 * 17;
    int v = (idx - 1) / 7 * 17;
    void *gim = dd_gim_get(0x40004);

    if (gim) {
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, gim);
    }
    if ((1 << idx) & dd_type_mask)
        dl = dl_ambient_color(dl, 0xFFFFFFFF);
    else
        dl = dl_ambient_color(dl, 0x80808080);
    dl = draw_set_offsets(dl, 17, 17, 17, 17);
    return draw_sprite(dl, x, y, 0, u, v, 0, 0);
}

unsigned int *func_08824C08(unsigned int *dl, int x, int y)
{
    void *gim = dd_gim_get(0x40005);

    if (gim) {
        dl = dl_enable(dl, 9);
        dl = gim_set_texture(dl, gim);
    }
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = draw_set_offsets(dl, 25, 24, 25, 24);
    return draw_sprite(dl, x, y, 0, 28, 93, 0, 0);
}

/* heads or tails */
void func_08824CC8(unsigned short *text)
{
    func_08825824(0, text, 0x30, func_08824D18);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(func_08824D58);
    se_play(0x621A);
}

void func_08824D18(MenuWidget *w, int open)
{
    if (open == 0)
        return;
    dd_coin_w = w;
    w->count = 2;
    w->rows = 2;
    w->item_w = 0x64;
    w->item_h = 0x78;
    w->draw_all = (MenuDrawRectFn)func_08824D7C;
}

void func_08824D58(void)
{
    dv_w624_set(func_08825A14());
}

/* the coin (face by the cursor) between two arrows, its caption below */
void func_08824D7C(unsigned int *dl, int x, int y)
{
    MenuWidget *w = dd_coin_w;
    int msg, back, tw;
    unsigned short *text;

    x += (w->def.w - w->def.pad_l - w->def.pad_r - 109) / 2;
    if (!(unsigned short)(w->scroll + w->cursor)) {
        back = 0;
        msg = 26;
    } else {
        back = 1;
        msg = 25;
    }
    dl = menu_draw_cursor(dl, w, x + 4, y + 4, 92, 92);
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 0, 1);
    dl = dd_coin_tex(1.5f, dl, x, y, 100, 100, back);
    dl = dl_disable(dl, 1);
    dl = gim_set_texture(dl, w->icon_gim);
    dl = menu_draw_arrow(dl, x, y + 48, 2);
    dl = menu_draw_arrow(dl, x + 100, y + 48, 3);
    font_text_set(dl);
    font_set_size(12, 12);
    text = dd_text(msg);
    tw = font_measure_text(text) >> 6;
    font_set_color(0xFF000000);
    x += 50 - tw / 2;
    font_draw_text(x << 6, (y + 100) << 6, (480 - x) << 6, text);
    dl_enable(font_text_clear(), 1);
}

/* number choice: min..max by step, shown with dialog format fmt */
void func_08824F3C(unsigned short *text, int min, int max, int step, int fmt)
{
    unsigned short *title = dd_text(28);

    dd_num_prompt.max = max;
    dd_num_prompt.min = min;
    dd_num_prompt.step = step;
    dd_num_prompt.fmt = fmt;
    func_08825824(title, text, 0x20, func_08824FF0);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(func_0882504C);
    se_play(0x621A);
}

void func_08824FF0(MenuWidget *w, int open)
{
    DdNumPrompt *p = &dd_num_prompt;

    if (open) {
        p->w = w;
        w->count = (p->max - p->min) / p->step + 1;
        w->rows = 1;
        w->item_w = 200;
        w->item_h = 40;
        w->draw_all = (MenuDrawRectFn)func_088250E8;
    }
}

void func_0882504C(void)
{
    int i = func_08825A14();

    dv_w624_set(dd_num_prompt.min + i * dd_num_prompt.step);
}
