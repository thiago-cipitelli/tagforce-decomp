// @range 0x08809274-0x0880A248
// @rodata 0x0880E528-0x0880E544
// @asm 0x08809624-0x0880979C
// @asm 0x08809C64-0x08809FE8
// @data 0x0880E8BC-0x0880E8D4
/* Recipe grid helpers (cards per row, entry under the cursor, the reveal animation), the menus
   (delete / rename / save / load confirmations, the saved-deck options) and the on-screen
   keyboard used to rename a saved deck. */
#include "librecipe_viewer.h"
#include "menu.h"
#include "card_db.h"

typedef struct RvOskData {     /* SceUtilityOskData */
    int unk00;
    int unk04;
    int language;
    int unk12;
    int inputtype;
    int lines;
    int unk24;
    unsigned short *desc;
    unsigned short *intext;
    int outtextlength;
    unsigned short *outtext;
    int result;
    int outtextlimit;
} RvOskData;

typedef struct RvOsk {         /* rvosk 0x0880E904 (.bss), 0x1B0 bytes */
    unsigned char state;       /* +0x00 0 idle, 1 fade in, 2 open, 3 fade out, 4 done */
    unsigned char pad1[3];
    struct RvOskParams {       /* +0x04 SceUtilityOskParams */
        int size;
        int language;
        int button_swap;
        int graphics_thread;
        int access_thread;
        int font_thread;
        int sound_thread;
        int result;
        int reserved[4];
        int datacount;
        RvOskData *data;
        int state;
        int unk60;
    } params;
    RvOskData data;            /* +0x44 */
    unsigned short intext[24]; /* +0x78 */
    unsigned short outtext[128]; /* +0xA8 */
    unsigned short *target;    /* +0x1A8 */
    void (*done)(unsigned short *text, int r); /* +0x1AC */
} RvOsk;

typedef struct RvOskParams RvOskParams;

typedef struct RvMenuItem {    /* a row of the saved-deck options menu, 6 bytes */
    unsigned short gim;        /* resource id of the button sprite */
    unsigned char u, v;
    unsigned short text;       /* strTbl entry */
} RvMenuItem;

typedef struct RvLabel {       /* see rv_draw.c */
    void *gim;
    unsigned char u, v, w, h;
    unsigned short *text;
    unsigned int color;
} RvLabel;

extern RvOsk rvosk;
extern MenuWidget rvm;         /* 0x0880EAB4 */

unsigned short *table_get_entry(void *t, unsigned int i);
int wcslen(const unsigned short *s);
void sysvar_set_a90(int v);
unsigned short region_pick_value(int flag);
int sceUtilityOskInitStart(void *params);
int sceUtilityOskShutdownStart(void);
int sceUtilityOskUpdate(int n);
int sceUtilityOskGetStatus(void);
void sceGuSync(int mode, int what);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *rv_draw_label(void *list, RvLabel *b, int x, int y);
void rv_select(void);

int rv_osk_open(unsigned short *text, unsigned short *desc, int maxlen, void (*done)(unsigned short *, int));
void rv_menu_open(unsigned short kind);
void *rv_menu_draw_item(void *dl, int index, int x, int y);

/* clears the saved deck under the list cursor */
void rv_delete_deck(void)
{
    RvPos *p = &rv.pos[rv.tab];
    int i = p->top + p->cursor;
    SaveRec *s = sysstate_rec_get(i);

    s->used = 0;
    rv.decks[i].need = 0;
    rv.decks[i].have = 0;
}

/* opens the keyboard on the name of the saved deck under the list cursor */
void rv_rename_deck(void)
{
    RvPos *p = &rv.pos[rv.tab];
    SaveRec *s = sysstate_rec_get(p->top + p->cursor);

    if (s->used) {
        if (rv_osk_open(s->name, table_get_entry(rvres_get(0x8033), 128), 16, 0) == 0)
            rv.busy = 1;
    }
}

/* cards in grid row `row` (0..3 main deck, 4..5 side deck, 6 extra deck), at least 7 */
int rv_grid_row_cols(int row)
{
    RvGrid *g = &rv.grid;
    int n = 7;

    switch (row) {
    case 0 ... 2:
        n = g->main_cols;
        break;
    case 3:
        n = g->c.n_main - g->main_cols * 3;
        break;
    case 4:
        n = g->side_cols;
        break;
    case 5:
        n = g->c.n_side - g->side_cols;
        break;
    case 6:
        n = g->c.n_extra;
        break;
    }
    if (n < 7)
        n = 7;
    return n;
}

/* entry under the grid cursor, 0 on an empty cell */
RvEntry *rv_grid_entry(void)
{
    RvGrid *g = &rv.grid;
    int row = g->row;
    RvEntry *e = 0;
    int i;

    if (row < 4) {
        i = g->col + row * g->main_cols;
        if (i < g->c.n_main)
            e = &g->c.main[i];
    } else if ((row -= 4) < 2) {
        i = g->col + row * g->side_cols;
        if (i < g->c.n_side)
            e = &g->c.side[i];
    } else if (g->col < g->c.n_extra) {
        e = &g->c.extra[g->col];
    }
    return e;
}

/* the card text follows the grid cursor */
void rv_grid_select(void)
{
    RvEntry *e = rv_grid_entry();

    if (e && e->id != rv.scroll.card) {
        rv.scroll.card = e->id;
        rv.scroll.pos = 0;
    }
}

/* reveals the cards of the shown recipe a fifteenth at a time */
void rv_counter_step(void)
{
    RvCards *c = &rv.grid.c;
    int n = c->n_extra + (c->n_main + c->n_side);

    if (rv.shown < n) {
        int d = n / 15;
        if (n < 15)
            d = 1;
        rv.shown += (unsigned char)d;
        if (n < rv.shown)
            rv.shown = n;
    }
}

#ifdef NON_MATCHING /* o original lê as posições pela base de rv e testa as abas com beql; registradores diferem */
/* hands the recipe under the list cursor to the caller */
void rv_apply_select(void)
{
    unsigned int t = rv.tab;
    unsigned short i = rv.pos[t].top + rv.pos[t].cursor;
    unsigned short *name = 0;
    int ok = 0;
    RecipeData rd;

    switch (t) {
    case 0:
        if (i < 50) {
            SaveRec *s = sysstate_rec_get(i);
            if (s->used) {
                memcpy(&rd, &s->recipe, sizeof(rd));
                name = s->name;
                ok = 1;
            }
        }
        break;
    case 1:
        if (i < rv.n_recipes) {
            recipe_load(rv.recipes[i].index, &rd);
            name = recipe_get_name(rv.recipes[i].index);
            ok = 1;
        }
        break;
    case 2:
        if (i < rv.n_ext) {
            memcpy(&rd, &rv.ext[i].recipe, sizeof(rd));
            ok = 1;
            name = rv.ext[i].name;
        }
        break;
    }
    if (ok && rv.on_select)
        rv.on_select(&rd, name);
}
#endif

unsigned short rv_osk_lang[6] = { 0, 1, 4, 2, 5, 3 };   /* SceUtility language by cfg_get_language() */

/* starts the on-screen keyboard on `text` ('$' are doubled in the game's strings) */
int rv_osk_open(unsigned short *text, unsigned short *desc, int maxlen, void (*done)(unsigned short *, int))
{
    unsigned short *in = rvosk.intext;
    int lang = cfg_get_language();
    int dollar = 0;
    unsigned int i;
    unsigned short *s;
    unsigned short *out;
    RvOskParams *p;
    RvOskData *d;
    int r;

    if (lang < 0 || lang >= 6)
        lang = 1;
    memset(&rvosk, 0, sizeof(rvosk));
    sysvar_set_a90(1);
    rvosk.target = text;
    rvosk.done = done;
    for (i = 0, s = text, out = in; i < wcslen(text) && (int)i < 23; s++, i++) {
        unsigned short c = *s;

        if (c == '$') {
            if (dollar) {
                dollar = 0;
                continue;
            }
            dollar = 1;
        }
        *out++ = c;
        if (c == 0)
            break;
    }
    p = &rvosk.params;
    p->size = 0x40;
    p->language = rv_osk_lang[lang];
    d = &rvosk.data;
    if (region_pick_value(0) == 0x2000)
        p->button_swap = 0;
    else
        p->button_swap = 1;
    p->graphics_thread = 0x11;
    p->access_thread = 0x13;
    p->font_thread = 0x12;
    p->sound_thread = 0x10;
    p->reserved[0] = 0;
    p->reserved[1] = 0;
    p->data = d;
    p->datacount = 1;
    if (lang == 0)
        d->unk00 = 1;
    else
        d->unk00 = 0;
    d->lines = 1;
    d->unk24 = 1;
    d->outtext = rvosk.outtext;
    d->desc = desc;
    d->intext = in;
    d->outtextlength = maxlen;
    d->unk04 = 0;
    d->language = 0;
    d->unk12 = 0;
    d->inputtype = 0;
    d->result = 0;
    d->outtextlimit = 0;
    r = sceUtilityOskInitStart(p);
    if (r == 0) {
        fade_start_in(2, 0xA0000000, 8);
        rvosk.state = 1;
    }
    return r;
}

/* runs the keyboard on display list layer `layer`; 1 when it is closed */
int rv_osk_update(int layer)
{
    switch (rvosk.state) {
    case 1:
    case 2:
    case 3: {
        void *dl = dl_select_layer(layer, 0);
        if (fade_apply(&dl)) {
            if (rvosk.state == 1)
                rvosk.state = 2;
            if (rvosk.state == 3)
                rvosk.state = 4;
        }
        dl_set_list(dl);
        break;
    }
    }
    sceGuSync(0, 0);
    switch (rvosk.state) {
    case 0:
        sysvar_set_a90(0);
        return 1;
    case 1:
    case 2:
        switch (sceUtilityOskGetStatus()) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            if (sceUtilityOskUpdate(2) == 0)
                break;
        case 3: {
            int r = sceUtilityOskShutdownStart();
            if (rvosk.params.result == 0) {
                unsigned int i;
                int k = 0;
                unsigned short c;

                for (i = 0; i < wcslen(rvosk.outtext); i++) {
                    c = rvosk.outtext[i];
                    if (c == 0)
                        break;
                    if (c == '$')
                        rvosk.target[k++] = c;
                    rvosk.target[k++] = c;
                }
                rvosk.target[k] = 0;
            }
            if (rvosk.done)
                rvosk.done(rvosk.outtext, r);
            fade_start(2, 0xA0000000, 8);
            rvosk.state = 3;
            break;
        }
        case 4:
            break;
        }
        break;
    case 3:
        break;
    case 4:
        sysvar_set_a90(0);
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* forma igual; temporários do bloco comum de inicialização (a0/v1) diferem */
/* confirmation or options menu: 1 delete, 2 overwrite, 3 saved, 4 rename, 5 options, 6 load */
void rv_menu_open(unsigned short kind)
{
    MenuWidget *w = &rvm;
    MenuItemDef *d = &rvm.def;

    memset(w, 0, sizeof(*w));
    d->color = 0xFFFFFFFF;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    rvm.heap = rv.heap;
    rvm.no_cancel = 1;
    d->style = 0;
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->open_anim = 2;
    d->close_anim = 2;
    switch (kind) {
    case 0:
        return;
    case 1:
        d->text = table_get_entry(rvres_get(0x8033), 0x9B);
        w->initial = 4;
        w->cancel_resets = 1;
        w->buttons = 2;
        break;
    case 2:
        d->text = table_get_entry(rvres_get(0x8033), 0x7C);
        w->initial = 4;
        w->cancel_resets = 1;
        w->buttons = 2;
        break;
    case 3:
        d->text = table_get_entry(rvres_get(0x8033), 0x9D);
        w->initial = 1;
        w->cancel_resets = 1;
        w->buttons = 1;
        break;
    case 4:
        d->text = table_get_entry(rvres_get(0x8033), 0x7D);
        w->initial = 4;
        w->cancel_resets = 1;
        w->buttons = 2;
        break;
    case 5:
        w->item_w = 212;
        d->auto_size = 1;
        w->item_h = 19;
        w->count = 2;
        w->style = 2;
        w->draw_item = rv_menu_draw_item;
        break;
    case 6:
        d->text = table_get_entry(rvres_get(0x8033), 0x7E);
        w->buttons = 2;
        w->initial = 4;
        w->cancel_resets = 1;
        break;
    default:
        return;
    }
    if (w->buttons == 0) {
        d->pad_l = 2;
        d->pad_t = 2;
        d->pad_r = 2;
        d->pad_b = 2;
    } else {
        d->max_w = 240;
        d->pad_l = 10;
        d->pad_r = 10;
        d->pad_b = 0;
        d->pad_t = 8;
    }
    menu_open(w);
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2;
    rv.osk = kind;
}
#endif

/* menu states; 1 when the screen should close */
int rv_name_update(void)
{
    int r = 0;
    int res = menu_update(&rvm);
    unsigned int kind;

    if (res != -1) {
    kind = rv.osk;
    menu_reset(&rvm);
    rv.osk = 0;
    switch (kind) {
    case 1:
        if (res == 1) {
            rv_apply_select();
            r = 1;
        }
        break;
    case 2:
        if (res == 1)
            rv.state = 5;
        break;
    case 3:
        r = 1;
        break;
    case 4:
        if (res == 1)
            rv_delete_deck();
        break;
    case 5:
        switch (res) {
        case 0:
            rv_rename_deck();
            break;
        case 1:
            rv_menu_open(4);
            se_play(0x621A);
            break;
        }
        break;
    case 6:
        if (res == 1)
            r = 1;
        break;
    }
    }
    return r;
}

void *rv_osk_draw(void *dl)
{
    menu_draw(&dl, &rvm);
    return dl;
}

RvMenuItem rv_menu_items[2] = {
    { 0x800B, 0x66, 0x00, 0x9F },
    { 0x800B, 0x11, 0x11, 0x7B },
};

void *rv_menu_draw_item(void *dl, int index, int x, int y)
{
    RvMenuItem *m = &rv_menu_items[index];
    RvLabel b;

    memset(&b, 0, sizeof(b));
    b.gim = rvres_get(m->gim);
    b.u = m->u;
    b.v = m->v;
    b.w = 17;
    b.h = 17;
    b.text = table_get_entry(rvres_get(0x8033), m->text);
    return rv_draw_label(dl, &b, x, y + 1);
}
