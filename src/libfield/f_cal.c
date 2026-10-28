// @range 0x0881EDC8-0x0881FFEC
// @asm 0x0881FCEC-0x0881FFEC
// @rodata 0x088392E0-0x08839344
/* The calendar of the field (field_cal, on the heap): opening, the slide in and out of the page,
   the per-state input and the frame, title bar and month tabs of the page. The pages themselves
   are drawn by the code that follows (0x0881FFEC on). */
#include "libfield.h"

void *gim_set_texture(void *list, void *gim);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_tex_func(void *list, int func, int comp);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void font_set_flag1(int on);
void font_set_flag2(int on);
void font_set_flag8(int on);
void font_set_flag8000(int on);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);
PadState *pad_get(void);
FieldStory *fld_story_get(void);

/* the story hooks (f_scene.c copies them from the story module) */
extern struct {
    int (*mark)(int month, int n);
    FieldCalEvent *(*events)(int *n);
} field_story_ext;

/* tabs of the page: color row, label, title (data of the next file) */
extern unsigned char fcal_tab_v[6];        /* 0x0883D420 */
extern unsigned short fcal_tab_text[6];    /* 0x0883D430 */
extern unsigned short fcal_tab_titles[6];  /* 0x0883D440 */

/* the pages (0x0881FFEC on) */
void *func_0882593C(int pack);
void func_0882596C(int pack);
void func_08825A48(int pack);
void func_08825A94(int pack);
unsigned char func_08820A04(void);
int func_08820B04(void);
int func_088254D0(unsigned char month, unsigned char week, unsigned char day);
void func_08825F38(void);
void func_08823DAC(void);
void func_08825218(void);
void func_08824830(void);
void func_088249EC(void);
int func_08824668(void);
void func_0882405C(void);
void func_08825B38(void);
void func_08825B6C(void);
void func_08825E48(void);
void *func_0881FFEC(void *list, int x, int y);
void *func_088213DC(void *list, int x, int y, int step);
void *func_08820BD0(void *list, int x, int y);
void *func_08821E44(void *list, int x, int y);
void *func_08821D24(void *list, int x, int y);
void *func_088216B0(void *list, int x, int y);
void *func_0882244C(void *list, int x, int y, int w, int h, unsigned int uv);
void *func_088220D8(void *list, unsigned int xy, unsigned int uv, int alpha, unsigned short *text, int lit,
                    int cur);

void fcal_free(void);
int fcal_event_cmp(const void *a, const void *b);
void *fcal_draw_frame(void *list, int x, int y);
void *fcal_draw_page(void *list, int x, int y);
void *fcal_draw_lines(void *list, int x, int y);
void *fcal_draw_title(void *list, int x, int y, unsigned short *text);
void *func_0881FBCC(void *list, int x, int y, unsigned short *text);
void *fcal_draw_tabs(void *list, int x, int y, int step);

/* mode 1: open on the event list, else slide the page in */
void fcal_open(void *heap, int pack, int mode)
{
    int month;
    int i;
    int n;
    FieldCalEvent *src;

    if (field_cal == 0)
        field_cal = heap_alloc(heap, sizeof(FieldCal));
    memset(field_cal, 0, sizeof(FieldCal));
    field_cal->heap = heap;
    field_cal->gim = func_0882593C(pack);
    func_0882596C(pack);
    func_08825A48(pack);
    func_08825A94(pack);
    field_cal->month = *(int *)fld_story_get()->w[0] - 5;
    field_cal->day = *(int *)fld_story_get()->w[1];
    field_cal->x20 = func_08820A04();
    field_cal->xa4 = func_08820B04();
    if (field_story_ext.mark != 0) {
        for (month = 0; month < 4; month++) {
            for (i = 0; i < 42; i++) {
                int k = func_088254D0(month, i / 7, i % 7);

                if (k > 0)
                    field_cal->marks[month][i] = field_story_ext.mark(month + 5, k);
            }
        }
    }
    if (field_story_ext.events != 0) {
        src = field_story_ext.events(&n);
        if (n > 0) {
            field_cal->events = heap_alloc(heap, n * sizeof(FieldCalEvent));
            if (field_cal->events == 0) {
                n = 0;
            } else {
                memcpy(field_cal->events, src, n * sizeof(FieldCalEvent));
                qsort(field_cal->events, n, sizeof(FieldCalEvent), fcal_event_cmp);
            }
        }
        field_cal->n_events = n;
    }
    switch (mode) {
    case 1:
        field_cal->state = 9;
        field_cal->t = 10;
        field_cal->x = field_cal->x_home;
        field_cal->timer = 0;
        func_08825F38();
        break;
    default:
        field_cal->x = -0x120;
        field_cal->state = 1;
        se_play(0x6263);
        break;
    }
}

/* 0 while open, 1 once closed, 5 when the menu returned 2 */
int fcal_update(void)
{
    int r;
    int k;

    if (field_cal == 0)
        return 1;
    if (field_cal->dlg) {
        r = menu_update(&field_cal->menu);
        if (r == -1)
            return 0;
        if (r == 2)
            return 5;
        field_cal->dlg = 0;
    }
    switch (field_cal->state) {
    case 0:
        break;
    case 1:
        k = 10 - field_cal->t; /* ease: (steps left)^2 */
        k *= k;
        field_cal->x = (field_cal->x_home - 0x120) * k / 100;
        field_cal->t++;
        if (field_cal->t >= 10) {
            field_cal->state = 2;
            field_cal->x = field_cal->x_home;
            se_play(0x6264);
        }
        break;
    case 2:
        if (field_cal->timer++ >= 12) {
            field_cal->state = 3;
            field_cal->timer = 0;
        }
        break;
    case 3:
        func_08823DAC();
        break;
    case 6:
        func_08824830();
        break;
    case 7:
        func_088249EC();
        break;
    case 4:
        if (pad_get()->b.trig & 0x6000) {
            se_play(0x6214);
            field_cal->state = 2;
        }
        break;
    case 5:
        func_08825218();
        break;
    case 8:
        if (field_cal->timer++ >= 4) {
            field_cal->state = 9;
            field_cal->timer = 0;
        }
        break;
    case 9:
        return func_08824668();
    case 10:
        func_0882405C();
        break;
    case 11:
        k = 10 - field_cal->t; /* ease: (steps left)^2 */
        k *= k;
        field_cal->x = (field_cal->x_home - 0x120) * k / 100;
        field_cal->t--;
        if (field_cal->t != 0)
            break;
        field_cal->state = 12;
        field_cal->x = -0x120;
    case 12:
        fcal_free();
        return 1;
    }
    return 0;
}

void fcal_draw(void)
{
    void *list;
    void *gim;

    if (field_cal == 0)
        return;
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    list = dl_select_layer(3, 0);
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    list = fcal_draw_page(list, field_cal->x, field_cal->y);
    list = fcal_draw_frame(list, field_cal->x, field_cal->y);
    if (field_cal->dlg) {
        menu_draw(&list, &field_cal->menu);
    } else if (field_cal->state == 10) {
        gim = field_cal->gim;
        menu_draw(&list, &field_cal->menu);
        if (gim != 0) {
            list = gim_set_texture(list, gim);
            list = dl_ambient_color(list, 0xFFFFFFFF);
            list = dl_set_tex_filter(list, 0, 0);
            list = draw_set_offsets(list, 0x10, 0x10, 0x10, 0x10);
            list = draw_sprite(list, field_cal->menu.def.x + 6, field_cal->menu.def.y - 0xE, 0, 0xC7, 0x2F, 0, 0);
        }
    }
    dl_set_list(list);
    func_08825B6C();
    func_08825E48();
}

void fcal_free(void)
{
    void *heap = field_cal->heap;

    func_08825B38();
    if (field_cal->events != 0) {
        heap_set_level(heap, field_cal->events);
        field_cal->events = 0;
    }
    heap_set_level(heap, field_cal);
    field_cal = 0;
}

/* qsort: latest month, then latest day first */
int fcal_event_cmp(const void *a, const void *b)
{
    const FieldCalEvent *ea = a;
    const FieldCalEvent *eb = b;

    if (ea->month == eb->month)
        return eb->day - ea->day;
    return eb->month - ea->month;
}

/* the binding and the top and bottom edges of the page */
void *fcal_draw_frame(void *list, int x, int y)
{
    int side;

    if (field_cal->gim == 0)
        return list;
    list = gim_set_texture(list, field_cal->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    side = y + 8;
    list = draw_set_offsets(list, 0x29, 0x100, 0x29, 0x100);
    list = draw_sprite(list, x, side, 0, 0, 0, 0, 0);
    list = draw_set_offsets(list, 0x1E, 0x100, 0x1E, 0x100);
    list = draw_sprite(list, x + 0x102, side, 0, 0x29, 0, 0, 0);
    list = draw_set_offsets(list, 0x120, 8, 0x120, 8);
    list = draw_sprite(list, x, y, 0, 0x47, 0, 0, 0);
    list = draw_set_offsets(list, 0x120, 8, 0x120, 8);
    return draw_sprite(list, x, y + 0x108, 0, 0x47, 8, 0, 0);
}

/* the page of the current state and its title */
void *fcal_draw_page(void *list, int x, int y)
{
    unsigned short *title = 0;
    int px = x + 0x26;
    int py = y + 4;

    list = fcal_draw_lines(list, px, py);
    switch (field_cal->state) {
    case 0:
    case 1:
        break;
    case 2:
        list = fcal_draw_tabs(list, px, py, field_cal->timer);
        title = table_get_entry(fld_gim_get(5), 0x31);
        break;
    case 3:
    case 11:
        list = fcal_draw_tabs(list, px, py, 12);
        title = table_get_entry(fld_gim_get(5), 0x31);
        break;
    case 4:
        list = func_0881FFEC(list, px, py);
        title = table_get_entry(fld_gim_get(5), 0x34);
        break;
    case 8:
        list = func_088213DC(list, px, py, field_cal->timer);
        title = table_get_entry(fld_gim_get(5), 0x35);
        break;
    case 9:
        list = func_088213DC(list, px, py, 4);
        title = table_get_entry(fld_gim_get(5), 0x35);
        break;
    case 10:
        list = func_08820BD0(list, px, py);
        title = table_get_entry(fld_gim_get(5), 0x36);
        break;
    case 7:
        title = table_get_entry(fld_gim_get(5), 0x32);
        list = func_08821E44(list, px, py);
        break;
    case 6:
        list = func_08821D24(list, px, py);
        title = table_get_entry(fld_gim_get(5), 0x32);
        break;
    case 5:
        list = func_088216B0(list, px, py);
        title = table_get_entry(fld_gim_get(5), 0x33);
        break;
    }
    return fcal_draw_title(list, px, py, title);
}

/* the ruled lines of the page */
void *fcal_draw_lines(void *list, int x, int y)
{
    int i;

    if (field_cal->gim == 0)
        return list;
    list = gim_set_texture(list, field_cal->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    i = 0;
    list = draw_set_offsets(list, 0x10, 0x10, 0xDD, 0x10);
    do {
        list = draw_sprite(list, x, y + 0x11 + i, 0, 0xF0, 0x2F, 0, 0);
        i += 0x10;
    } while (i < 0xF7);
    return list;
}

/* the title bar of the page */
void *fcal_draw_title(void *list, int x, int y, unsigned short *text)
{
    if (field_cal->gim == 0)
        return list;
    list = gim_set_texture(list, field_cal->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0xDD, 0x11);
    list = draw_sprite(list, x, y, 0, 0xB4, 0x20, 0, 0);
    if (text == 0)
        return list;
    font_text_set(list);
    font_set_size(0xD, 0xD);
    font_set_color(0xFFEDEED4);
    font_draw_text((x + 6) * 64, (y + 2) * 64, 0x3440, text);
    return font_text_clear();
}

/* the note box at the bottom of the page (kept under its address: the pages call it) */
void *func_0881FBCC(void *list, int x, int y, unsigned short *text)
{
    if (field_cal->gim == 0)
        return list;
    list = gim_set_texture(list, field_cal->gim);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = func_0882244C(list, x + 3, y + 0xD1, 0xD8, 0x36, 0x580047);
    if (text == 0)
        return list;
    font_text_set(list);
    if (cfg_get_language() == 0)
        font_set_flag8000(1);
    font_set_size(0xE, 0xE);
    font_set_color(0xFF492B02);
    font_draw_wtext_ex((x + 0xB) * 64, (y + 0xD5) * 64, 0x3200, 0xC80, text, -1);
    if (cfg_get_language() == 0)
        font_set_flag8000(0);
    return font_text_clear();
}

#define FCAL_XY(x, y) ((unsigned short)(x) | (unsigned short)(y) << 16)

/* the month tabs; step < 12 while they unfold one after the other */
#ifdef NON_MATCHING /* same shape; mwcc keeps step in $fp but spills unfolding/last differently */
void *fcal_draw_tabs(void *list, int x, int y, int step)
{
    int unfolding = 0;
    int last = 5;
    int cur;
    int ty;
    int i;
    unsigned short *text = 0;
    int alpha;
    int lit;
    int d;

    if (step < 12) {
        unfolding = 1;
        last = (step - 1) / 2;
    } else {
        text = table_get_entry(fld_gim_get(5), fcal_tab_titles[field_cal->cur]);
    }
    list = func_0881FBCC(list, x, y, text);
    for (i = last; i >= 0; i--) {
        ty = y + 0x11 + i * 32;
        alpha = 0xFF;
        lit = 0;
        cur = 0;
        if (!unfolding && field_cal->cur == i) {
            if (!field_cal->flag_b0 || field_cal->flag_b1_1 != 1)
                lit = 1;
            cur = 1;
        }
        if (lit) {
            list = func_088220D8(list, FCAL_XY(x, ty), FCAL_XY(0x16E, fcal_tab_v[i] * 24), 0xFF,
                                 table_get_entry(fld_gim_get(5), fcal_tab_text[i]), 1, 1);
        } else {
            text = table_get_entry(fld_gim_get(5), fcal_tab_text[i]);
            if (unfolding && i == last && i * 2 + 2 >= step) {
                d = i * 2 + 2 - step;
                if (d != 0) {
                    ty -= d * 32 / 2;
                    text = 0;
                }
                alpha = 0xFF - ((d + 1) * 0x60 & 0xFF) & 0xFF;
            }
            list = func_088220D8(list, FCAL_XY(x, ty), FCAL_XY(0x16E, fcal_tab_v[i] * 24), alpha, text, 0, cur);
        }
    }
    return list;
}
#endif
