// @range 0x08808934-0x08809538
// @data 0x0880D63C-0x0880D640
/* Menus of the screen: help, "illegal card in your deck" warning, and the list menu (reset to
   the default list, clear, or load a list found in the extra data through the BOOT save dialog). */
#include "liblimitlist.h"
#include "menu.h"

typedef struct LlMenu {        /* llm 0x0880D6CC (.bss), 0x530 bytes */
    unsigned short kind;       /* +0x00 0 list menu, 1 saved lists, 3 illegal card warning */
    unsigned short step;       /* +0x02 extra data dialog step */
    unsigned char dialog : 1;  /* +0x04 extra data dialog running */
    unsigned char pad4 : 7;
    unsigned char pad5[3];
    unsigned short text[0x200]; /* +0x08 warning text */
    int n_saves;               /* +0x408 */
    LlSave *saves;             /* +0x40C */
    MenuWidget w;              /* +0x410 */
} LlMenu;

extern LlMenu llm;

unsigned short *table_get_entry(void *t, unsigned int i);
unsigned short *card_get_name(int id);
int card_get_limit(int id);
int wsprintf(unsigned short *out, const unsigned short *fmt, ...);
int extdata_scan(int kind, int *count);
void extdata_open(void *heap);
int extdata_load(void *out, int *count);
void *extdata_draw(void *dl);
void newgame_init_inventory(void);
void inv_set_limit(unsigned short id, unsigned short v);
void sysstate_set_player_name(const unsigned short *name);
unsigned short *sysmsg_get(int i);

void ll_menu_open_saves(void);
void *ll_menu_draw_help(void *dl, int index, int x, int y);
void *ll_menu_draw_save(void *dl, int index, int x, int y);
int ll_apply_list(unsigned short choice);
void ll_saves_free(void);

void ll_menu_open_help(void)
{
    MenuWidget *w;
    MenuItemDef *d;

    memset(&llm, 0, sizeof(llm));
    w = &llm.w;
    d = &w->def;
    d->visible = 1;
    d->font_flag4 = 1;
    d->auto_size = 1;
    d->show_title = 1;
    d->auto_title = 1;
    d->color = 0xFFFFFFFF;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = ll.heap;
    w->no_cancel = 1;
    d->style = 0;
    d->title = table_get_entry(llres_get(0x1020), 3);
    d->style2 = 2;
    w->count = 2;
    d->pad_l = 2;
    d->pad_t = 2;
    d->pad_r = 2;
    d->pad_b = 2;
    d->title_font_size = 12;
    d->title_color = 0xFFFFFFFF;
    w->style = 2;
    w->item_w = 300;
    w->item_h = 16;
    w->draw_item = ll_menu_draw_help;
    llm.kind = 0;
    menu_open(w);
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2 + 20;
    ll.state = 3;
}

void ll_menu_open_illegal(unsigned short id)
{
    MenuWidget *w;
    MenuItemDef *d;
    unsigned short *fmt;

    memset(&llm, 0, sizeof(llm));
    w = &llm.w;
    d = &w->def;
    fmt = table_get_entry(llres_get(0x1020), 6);
    switch (card_get_limit(id)) {
    case 1:
        fmt = table_get_entry(llres_get(0x1020), 7);
        break;
    case 2:
        fmt = table_get_entry(llres_get(0x1020), 8);
        break;
    }
    wsprintf(llm.text, fmt, card_get_name(id));
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->color = 0xFFFFFFFF;
    d->max_w = 240;
    d->pad_l = 6;
    d->pad_t = 6;
    d->pad_r = 6;
    d->pad_b = 6;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = ll.heap;
    w->no_cancel = 1;
    d->text = llm.text;
    w->initial = 3;
    w->buttons = 3;
    w->cancel_resets = 1;
    d->style = 0;
    menu_open(w);
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2 + 20;
    llm.kind = 3;
    ll.state = 5;
}

/* list menu: "default list", "clear", then one row per saved list */
void ll_menu_open_saves(void)
{
    MenuWidget *w = &llm.w;
    MenuItemDef *d = &w->def;
    unsigned short n = llm.n_saves + 2;

    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->font_flag4 = 1;
    d->auto_size = 1;
    d->show_title = 1;
    d->auto_title = 1;
    d->color = 0xFFFFFFFF;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = ll.heap;
    w->no_cancel = 1;
    d->style = 0;
    d->title = table_get_entry(llres_get(0x1020), 4);
    d->style2 = 2;
    d->pad_l = 2;
    d->pad_t = 2;
    d->pad_r = 2;
    d->pad_b = 2;
    d->title_font_size = 12;
    d->title_color = 0xFFFFFFFF;
    w->style = 2;
    w->rows = 8;
    w->item_w = 300;
    w->item_h = 16;
    w->draw_item = ll_menu_draw_save;
    llm.kind = 1;
    w->count = n;
    menu_open(w);
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2 + 20;
}

/* menu states (3 and 5) */
void ll_menu_update(void)
{
    MenuWidget *w = &llm.w;
    int r;

    if (llm.dialog) {
        switch (llm.step) {
        case 0:
            if (extdata_scan(2, &llm.n_saves))
                llm.step++;
            break;
        case 1:
            if (llm.n_saves == 0) {
                llm.step = 3;
            } else {
                int size = llm.n_saves * sizeof(LlSave);
                llm.saves = heap_alloc(ll.heap, size);
                memset(llm.saves, 0, size);
                llm.step++;
            }
            break;
        case 2:
            if (extdata_load(llm.saves, &llm.n_saves))
                llm.step++;
            break;
        case 3:
            llm.step = 0;
            llm.dialog = 0;
            ll_menu_open_saves();
            break;
        }
        return;
    }
    r = menu_update(w);
    if (r == -1)
        return;
    menu_reset(w);
    switch (llm.kind) {
    case 0:
        switch (r) {
        case 0:
            extdata_open(ll.heap);
            llm.dialog = 1;
            llm.step = 0;
            break;
        case 1:
            newgame_init_inventory();
            ll_build();
            ll.state = 2;
            ll_saves_free();
            break;
        default:
            ll.state = 2;
            ll_saves_free();
            break;
        }
        break;
    case 1:
        switch (r) {
        case -2:
            ll.state = 2;
            ll_saves_free();
            break;
        default:
            ll_apply_list(r);
            ll.state = 2;
            ll_saves_free();
            break;
        }
        break;
    case 3:
        if (r == 2) {
            fade_start_in(1, 0xFF000000, 16);
            ll.cancelled = 1;
            ll.state = 6;
        } else {
            ll.state = 2;
        }
        ll_saves_free();
        break;
    }
}

void *ll_menu_draw(void *list)
{
    if (llm.dialog)
        list = extdata_draw(list);
    else
        menu_draw(&list, &llm.w);
    return list;
}

unsigned short ll_help_items[2] = { 4, 5 };   /* strTbl entries of the help menu rows */

void *ll_menu_draw_help(void *dl, int index, int x, int y)
{
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    font_text_set(dl);
    font_draw_text(x << 6, (y + 2) << 6, (480 - x) << 6, table_get_entry(llres_get(0x1020), ll_help_items[index]));
    return font_text_clear();
}

void *ll_menu_draw_save(void *dl, int index, int x, int y)
{
    font_set_size(12, 12);
    font_set_color(0xFF000000);
    font_text_set(dl);
    switch (index) {
    case 0:
        font_draw_wtext(x << 6, (y + 2) << 6, 19200, sysmsg_get(23));
        break;
    case 1:
        font_draw_wtext(x << 6, (y + 2) << 6, 19200, table_get_entry(llres_get(0x1020), 9));
        break;
    default:
        font_draw_wtext(x << 6, (y + 2) << 6, 19200, llm.saves[index - 2].name);
        break;
    }
    return font_text_clear();
}

/* applies choice 0 (new game list), 1 (the default list) or a saved list, then rebuilds */
int ll_apply_list(unsigned short choice)
{
    if (choice == 0) {
        newgame_init_inventory();
    } else {
        int id;

        for (id = 4007; id < 6958; id++)
            inv_set_limit(id, 3);
        if (choice == 1) {
            unsigned short *t;

            for (id = 0, t = (unsigned short *)ll_default_limits; (unsigned int)id < 186; id += 2)
                inv_set_limit(t[id], t[id + 1]);
            sysstate_set_player_name(table_get_entry(llres_get(0x1020), 9));
        } else {
            for (id = 0; id < llm.saves[choice - 2].count; id++)
                inv_set_limit(llm.saves[choice - 2].limits[id].id, llm.saves[choice - 2].limits[id].limit);
            sysstate_set_player_name(llm.saves[choice - 2].name);
        }
    }
    ll_build();
    return 1;
}

void ll_saves_free(void)
{
    int i;

    if (llm.saves) {
        for (i = 0; i < llm.n_saves; i++) {
            if (llm.saves[i].limits)
                heap_set_level(ll.heap, llm.saves[i].limits);
        }
        heap_set_level(ll.heap, llm.saves);
        llm.saves = 0;
    }
}
