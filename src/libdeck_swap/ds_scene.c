// @range 0x088157E0-0x08815ED4
/* Entry points of the deck swap screen called by libduel_mgr between the duels of a match (init
   with the game state and the partners, per-frame update until the screen is done, unload) and the
   popups (incomplete deck, leaving, search result, time out). */
#include "libdeck_swap.h"
#include "menu.h"

extern MenuWidget ds_popup;            /* 0x0881B170 */
extern unsigned short ds_popup_text[0x400]; /* 0x0881A970 */
extern short ds_popup_timer;           /* 0x0881A96C frames the time-out popup was shown */

int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set);
int cfg_get_language(void);
int sysvar_get_a98(void);
void *libduel_draw_0881CA74(void);
void *libduel_draw_088046E4(void);
int menu_update(MenuWidget *w);
void menu_open(MenuWidget *w);
void menu_reset(MenuWidget *w);
void menu_draw(void **dl, MenuWidget *w);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
void ds_res_init(void);
int ds_load_step(void);
void ds_res_free(void);
void ds_lists_free(void);
void ds_lists_commit(int timeout);
void ds_update_anim(void);
void ds_update_input(void);
int ds_unload(void);

int module_start(void)
{
    return 0;
}

/* export: gs = the game state (gs[0] the cards the deck must have), the partner decks */
void libdeck_swap_088157E8(int *gs, int n_partners, void *partners)
{
    memset(&ds, 0, sizeof(ds));
    memset(&dsv, 0, sizeof(dsv));
    ds.heap = libduel_draw_0881CA74();
    ds.cip = libduel_draw_088046E4();
    ds.gs = gs;
    ds.n_partners = n_partners;
    ds.partners = partners;
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    ds_res_init();
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    ds.state = 0;
}

/* export: once per frame; 1 when the screen is done */
int libdeck_swap_088158E0(void)
{
    switch (ds.state) {
    case 0:
        if (ds_load_step() != 0) {
            gfx_overlay_enable(0);
            fade_start(1, 0xFF000000, 16);
            ds.state = 1;
            if (sysvar_get_a98())
                ds.time_limit = 5400;
            switch (((unsigned char *)game_state_get())[0x11C]) {   /* duel kind */
            case 4:
            case 5:
                bgm_play(13);
                break;
            default:
                bgm_play(12);
                break;
            }
        }
        return 0;
    case 9:
        if (fade_is_done())
            ds.state = 10;
        break;
    case 10:
        gfx_overlay_enable(1);
        return ds_unload() != 0;
    }
    ds_update_anim();
    ds_update_input();
    return 0;
}

int ds_unload(void)
{
    ds_res_free();
    ds_lists_free();
    return 1;
}

/* 2 deck incomplete, 3 leave (keep the deck), 4 search result (arg = count), 5 time out */
void ds_popup_open(unsigned short id, unsigned short arg)
{
    MenuWidget *m = &ds_popup;
    MenuItemDef *d = &ds_popup.def;
    unsigned char buttons = 1;

    if (dsv.popup != 0)
        menu_reset(&ds_popup);
    memset(m, 0, sizeof(MenuWidget));
    memset(ds_popup_text, 0, sizeof(ds_popup_text));
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->max_w = 0xF0;
    d->color = -1;
    d->pad_l = 10;
    d->pad_r = 10;
    d->pad_t = 8;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->style = 0;
    d->pad_b = 0;
    d->open_anim = 2;
    d->close_anim = 2;
    switch (id) {
    case 2:
        wcscpy(ds_popup_text, table_get_entry(res_get(0x8033), 0x8D));
        m->initial = 4;
        buttons = 2;
        m->cancel_resets = 1;
        break;
    case 3:
        wcscpy(ds_popup_text, table_get_entry(res_get(0x8033), 0x8E));
        m->cancel_blocked = 1;
        break;
    case 4:
        if (arg <= 0)
            wcscpy(ds_popup_text, table_get_entry(res_get(0x8033), 0x89));
        else
            wsprintf(ds_popup_text, table_get_entry(res_get(0x8033), 0x81), arg);
        m->initial = 1;
        m->cancel_resets = 1;
        break;
    case 5:
        wcscpy(ds_popup_text, table_get_entry(res_get(0x8033), 0x9E));
        ds_popup_timer = 0;
        buttons = 0;
        d->pad_b = 8;
        break;
    }
    d->text = ds_popup_text;
    m->heap = ds.heap;
    m->no_cancel = 1;
    m->buttons = buttons;
    menu_open(m);
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2;
    dsv.popup = id;
}

/* the popup's input; the time-out popup closes the screen by itself after 46 frames */
void ds_popup_update(void)
{
    int r = menu_update(&ds_popup);

    if (dsv.popup == 5 && ds.state < 9 && ds_popup_timer++ > 45) {
        ds_lists_commit(0);
        fade_start_in(1, 0xFF000000, 16);
        ds.state = 9;
        menu_reset(&ds_popup);
        dsv.popup = 0;
    }
    if (r != -1) {
        menu_reset(&ds_popup);
        switch (dsv.popup) {
        case 2:
            if (r == 1) {
                ds_lists_commit(1);
                fade_start_in(1, 0xFF000000, 16);
                ds.state = 9;
            }
            break;
        }
        dsv.popup = 0;
    }
}

void *ds_popup_draw(void *list)
{
    menu_draw(&list, &ds_popup);
    return list;
}
