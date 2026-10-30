// @range 0x08866074-0x088666D8
// @asm 0x0886646C-0x088666D8
/* "Declare a card name" list of the duel: every card that can be declared (all of them, the
   non-token ones, or only monsters, by effect kind 75..77), sorted by name, in a 10-row menu;
   left/right jump to the previous/next initial, R + up/down move nine rows. */
#include "dd_duelui.h"

typedef struct DeclareList {           /* 0x26CC, heap */
    int ids[2480];
    int count;                         /* 0x26C0 */
    int result;                        /* 0x26C4 menu_update */
    MenuWidget *menu;                  /* 0x26C8 */
} DeclareList;

extern DeclareList *declare_list;      /* 0x08C5B974 */

void *dd_text(int id);
void *dd_alloc(int size);
void dd_free(void *p);
void dd_mode_push(int mode);
void dd_mode_set_flag(int mode);
int dd_input_on(void);
PadState *dd_pad(void);
void func_08825824(void *panel, int a, int b, void (*setup)(MenuWidget *w, int init));
void func_088259FC(void (*cb)(void));
void func_08825A08(int (*cb)(void));
int func_08825728(void);
void dv_w624_set(int v);
void se_play(int id);

void declare_build(int kind);
void declare_menu_setup(MenuWidget *w, int init);
void declare_close(void);
int declare_update(void);
void *declare_draw_item(void *dl, int index, int x, int y);
int declare_cmp(const void *a, const void *b);

void declare_free(void)
{
    if (declare_list != 0) {
        dd_free(declare_list);
        declare_list = 0;
    }
}

void declare_open(int kind)
{
    void *panel = dd_text(28);

    declare_list = dd_alloc(sizeof(DeclareList));
    declare_build(kind);
    func_08825824(panel, 0, 32, declare_menu_setup);
    dd_mode_push(4);
    dd_mode_set_flag(49);
    func_088259FC(declare_close);
    func_08825A08(declare_update);
    se_play(0x621A);
}

void declare_menu_setup(MenuWidget *w, int init)
{
    DeclareList *d = declare_list;

    if (init) {
        d->menu = w;
        w->count = d->count;
        w->rows = 10;
        w->item_w = 300;
        w->item_h = 18;
        w->draw_item = declare_draw_item;
    }
}

void declare_close(void)
{
    DeclareList *d = declare_list;

    dv_w624_set(d->ids[d->result]);
    dd_free(d);
    declare_list = 0;
}

unsigned short *declare_name(int i)
{
    return card_get_name(declare_list->ids[i]);
}

void *declare_draw_item(void *dl, int index, int x, int y)
{
    unsigned short *name;
    int h;

    font_set_size(12, 12);
    font_set_color(0xFF000000);
    name = declare_name((unsigned short)index);
    font_measure_text(name);
    h = font_line_height() >> 6;
    font_text_set(dl);
    font_draw_text(x << 6, (y + (9 - h / 2)) << 6, 300 << 6, name);
    return font_text_clear();
}

void declare_build(int kind)
{
    DeclareList *d = declare_list;
    int id;

    d->count = 0;
    switch (kind) {
    case 75:
    default:
        id = 4007;
        do {
            if (card_get_flag31(id) != 0 && card_is_attribute9(id) == 0) {
                d->ids[d->count] = id;
                d->count++;
            }
            id++;
        } while (id < 6958);
        break;
    case 76:
        id = 4007;
        do {
            if (card_get_flag31(id) != 0 && card_is_attribute9(id) == 0 && card_get_attribute_icon(id) != 3) {
                d->ids[d->count] = id;
                d->count++;
            }
            id++;
        } while (id < 6958);
        break;
    case 77:
        id = 4007;
        do {
            if (card_get_flag31(id) != 0 && card_is_attribute9(id) == 0 && card_get_attribute_icon(id) != 3 &&
                card_is_monster(id) != 0) {
                d->ids[d->count] = id;
                d->count++;
            }
            id++;
        } while (id < 6958);
        break;
    }
    qsort(d->ids, d->count, 4, declare_cmp);
}

int declare_cmp(const void *a, const void *b)
{
    int ib = *(int *)b;
    int sa = card_get_sort(*(int *)a);

    return sa - card_get_sort(ib);
}

#ifdef NON_MATCHING /* o original mantém pos * 4 num registrador salvo a mais (s5 em vez de s4) */
int declare_update(void)
{
    DeclareList *d = declare_list;
    PadState *pad;
    int pos, cur, step, moved;
    int *p;

    if (dd_input_on()) {
        d->result = menu_update(d->menu);
        if (d->menu->state == 3) {
            pad = dd_pad();
            pos = d->menu->scroll + d->menu->cursor;
            p = &d->ids[pos];
            cur = card_get_sort_position(*p);
            moved = 0;
            if (pad->b.rep & 0x80) {
                if (pos <= 0)
                    step = 1;
                else if (card_get_sort_position(*p) == card_get_sort_position(p[-1]))
                    step = 1;
                else
                    step = 2;
                cur -= step;
                while (pos > 0 && card_get_sort_position(*p) != cur) {
                    p--;
                    pos--;
                }
                if (card_get_sort_position(d->ids[pos]) == cur)
                    pos++;
                moved = 1;
            } else if (pad->b.rep & 0x20) {
                while (pos < d->count - 1) {
                    if (card_get_sort_position(*p) != cur)
                        break;
                    p++;
                    pos++;
                }
                moved = 1;
            } else if ((pad->b.down & 0x200) && (pad->b.rep & 0x10)) {
                pos -= 9;
                if (pos < 0)
                    pos = 0;
                moved = 1;
            } else if ((pad->b.down & 0x200) && (pad->b.rep & 0x40)) {
                pos += 9;
                if (pos >= d->count)
                    pos = d->count - 1;
                moved = 1;
            }
            if (moved) {
                if (pos <= d->count - 10) {
                    d->menu->cursor = 0;
                    d->menu->scroll = pos;
                } else {
                    d->menu->cursor = pos - d->count + 10;
                    d->menu->scroll = d->count - 10;
                }
            }
        }
    }
    return func_08825728() == 0;
}
#endif
