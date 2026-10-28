// @range 0x08827C44-0x08827FE4
// @asm 0x08827D10-0x08827F20
// @data 0x0883D67C-0x0883D690
/* Clock of the map (field_hour_menu): chooses the time to wait until, in quarters of an hour, with two
   windows and a confirmation dialog. The rest of the unit follows at 0x08827FE4. */
#include "libfield.h"

FieldStory *fld_story_get(void);
void *syspack_get(void);
int fmsg_done(void);
int menu_update(MenuWidget *w);
void fld_dlg_open(unsigned short *text, int a, int b);
int fld_dlg_update(void);
void *fld_dlg_draw(void *list);
/* the rest of the clock unit (0x08827FE4-) */
void fclk_title_setup(void);
void fclk_list_setup(void);
void fclk_hint_init(void);
void fclk_list_update(void);
void fclk_time_open(void);
void fclk_time_update(void);
void fclk_finish(void);
int fclk_closed(void);
void fclk_free(void);
void *fclk_time_draw(void *list);
void fclk_time_to_index(int *out, int hour, int minute);

void fclk_open(void)
{
    FieldStory *st = fld_story_get();

    memset(&field_hour_menu, 0, sizeof(field_hour_menu));
    field_hour_menu.open = 1;
    fclk_time_to_index(&field_hour_menu.min_sel, *(int *)st->w[3], *(int *)st->w[4]);
    field_hour_menu.sel = ++field_hour_menu.min_sel;
    field_hour_menu.gim = ehp_find(syspack_get(), "buttom_icon1.gim", 0);
    menu_init(&field_hour_menu.menu, fld_heap_get());
    fclk_title_setup();
    fclk_list_setup();
    fld_story_flag(0x6216, 0);
}

#ifdef NON_MATCHING /* a forma bate, mas a tabela de saltos (0x088399B4) fica no meio do .rodata das funções seguintes da unidade (0x08827FE4-); fica em asm até a unidade inteira estar em C */
/* returns -1 while open, then 2 or 3 (the choice confirmed) */
int fclk_update(void)
{
    FieldHourMenu *c = &field_hour_menu;

    switch (field_hour_menu.state) {
    case 0:
        fclk_hint_init();
        c->state = 1;
        break;
    case 1:
        fclk_list_update();
        return -1;
    case 2:
        fclk_time_open();
        c->state = 3;
        break;
    case 3:
        fclk_time_update();
        break;
    case 4:
        if (fmsg_done()) {
            c->state = 0;
            c->open = 1;
        }
        break;
    case 5:
        fld_dlg_open(table_get_entry(fld_gim_get(5), 0x67), 3, 2);
        c->state = 6;
        break;
    case 6:
        switch (fld_dlg_update()) {
        case -2:
        case 3:
            c->open = 0;
            c->state = 0;
            break;
        case 2:
            c->ok = 2;
            c->state = 9;
            break;
        }
        break;
    case 7:
        fld_dlg_open(table_get_entry(fld_gim_get(5), 0x68), 3, 2);
        c->state = 8;
        break;
    case 8:
        switch (fld_dlg_update()) {
        case -2:
        case 3:
            c->open = 0;
            c->state = 0;
            break;
        case 2:
            c->ok = 3;
            c->state = 9;
            break;
        }
        break;
    case 9:
        fclk_finish();
        c->state = 10;
        break;
    case 10:
        if (fclk_closed() != 0) {
            c->state = 11;
        } else {
            menu_update(&c->w[0]);
            menu_update(&c->w[1]);
        }
        break;
    case 11:
        fclk_free();
        return c->ok;
    }
    return -1;
}
#endif

void fclk_draw(void *list)
{
    FieldHourMenu *c = &field_hour_menu;
    int i;

    menu_layout(&list, &field_hour_menu.menu);
    for (i = 0; i < 2; i++)
        menu_draw(&list, &c->w[i]);
    list = fld_dlg_draw(list);
    switch (c->state) {
    case 3:
        if (c->w[1].state < 5)
            list = fclk_time_draw(list);
        break;
    }
}

/* the time chosen */
void fclk_get(int *hour, int *minute)
{
    *hour = field_hour_menu.hour;
    *minute = field_hour_menu.minute;
}
