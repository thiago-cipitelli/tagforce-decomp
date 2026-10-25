// @range 0x08806BC0-0x08807A10
// @data 0x0880FA88-0x0880FA98
/* Overlays drawn over the duel: the "quit the tutorial?" box, a small sprite list (tuto_duel01.gim)
   and the screen highlight (stencil hole that grows from / shrinks to the full screen). */
#include "libtutorial.h"

typedef struct TutSprite {       /* 0x30 bytes */
    int used;
    void *gim;
    int x, y, z;                 /* +0x08 */
    int w, h;                    /* +0x14 */
    int u, v;                    /* +0x1C */
    int tw, th;                  /* +0x24 */
    int pad2c;
} TutSprite;

typedef struct TutSprites {      /* tut_spr 0x088128A8 */
    void *list;                  /* display list being built */
    TutSprite spr[32];
} TutSprites;

typedef struct TutHl {           /* highlight rectangle, 12 bytes */
    unsigned char state;         /* 0 off, 1 growing, 2 shown, 3 shrinking */
    unsigned char pad1;
    short x0, y0, x1, y1;        /* +0x02 */
    short t;                     /* +0x0A animation step 0..10 */
} TutHl;

typedef struct TutMarker TutMarker;
struct TutMarker {               /* tut_markers 0x08812F50, 0x1C bytes */
    int on;
    int x, y;                    /* +0x04 */
    int dir;                     /* +0x0C 2 = points up */
    int timer;                   /* +0x10 */
    int *spr;                    /* +0x14 sprite id */
    void (*draw)(TutMarker *m);  /* +0x18 */
};

extern TutSprites tut_spr;
extern TutHl tut_hl_cur[2];      /* 0x08812EAC drawn this frame */
extern TutHl tut_hl_tgt[2];      /* 0x08812EC4 requested */
extern void *tut_tex[4];
extern int tut_arrow_ids_b[3];   /* 0x08812F04 */
extern int tut_arrow_ids_a[5];   /* 0x08812F10 */
extern int tut_marker_unk[11];   /* 0x08812F24 */
extern TutMarker tut_markers[13];

unsigned short *sysmsg_get(int i);
void *gim_set_texture(void *list, void *g);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_color(void *list, unsigned int color);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
void *dl_clear_color(void *list, unsigned int color);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear_depth(void *list, unsigned int z);
void *dl_clear(void *list, unsigned int flags);
void *dl_stencil_func(void *list, int func, int ref, int mask);
void *dl_stencil_op(void *list, int fail, int zfail, int zpass);

void *tut_dl_reset(void *list);
void *tut_draw_tex_ex(void *list, int tex, int u, int v, int w, int h, int x, int y, int sw, int sh,
                      unsigned int color, int a, int b);
void *tut_quad_ex(void *list, int x, int y, int w, int h, int u, int v, int tw, int th, int fu, int fx);

void *tut_draw_tex(void *list, int tex, int u, int v, int w, int h, int x, int y, int sw, int sh,
                   unsigned int color)
{
    return tut_draw_tex_ex(list, tex, u, v, w, h, x, y, sw, sh, color, 0, 0);
}

/* rectangle of texture tex (tuto_duel01, window02, tuto_bigwin01) */
void *tut_draw_tex_ex(void *list, int tex, int u, int v, int w, int h, int x, int y, int sw, int sh,
                      unsigned int color, int a, int b)
{
    list = tut_dl_reset(list);
    list = dl_color(list, color);
    list = gim_set_texture(list, tut_tex[tex]);
    return tut_quad_ex(list, u, v, w, h, x, y, sw, sh, a, b);
}

/* START during the lesson: "quit the tutorial?" (yes/no, no by default) */
void tut_pause_open(void)
{
    MenuWidget *w = &tut.quit;
    MenuItemDef *d = &w->def;

    memset(w, 0, 0x120);
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->max_w = 0xF0;
    d->color = 0xFFFFFFFF;
    d->pad_l = 10;
    d->pad_r = 10;
    d->pad_t = 8;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->style = 0;
    d->pad_b = 0;
    d->text = sysmsg_get(0x18);
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = tut_heap_get();
    w->no_cancel = 1;
    w->initial = 3;
    w->buttons = 3;
    w->cancel_resets = 1;
    w->button = 3;
    menu_open(w);
    d->x = (0x1E0 - d->w) / 2;
    d->y = (0x110 - d->h) / 2;
    tut.quit_open = 1;
    tut.saved_lock = tut_get_lock();
    tut_set_lock(1);
}

void *tut_spr_draw(TutSprite *s, void *list, int flip)
{
    list = gim_set_texture(list, s->gim);
    list = draw_set_offsets(list, s->tw, s->th, s->w, s->h);
    if (flip > 3)
        flip = 0;
    return draw_sprite(list, s->x, s->y, s->z, s->u, s->v, flip, 0);
}

TutSprite *tut_spr_get(int i)
{
    return &tut_spr.spr[i];
}

int tut_spr_alloc(void *gim)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (tut_spr.spr[i].used == 0) {
            TutSprite *s = &tut_spr.spr[i];

            s->used = 1;
            s->gim = gim;
            return i;
        }
    }
    return -1;
}

void tut_spr_set_tex(int i, int u, int v, int tw, int th)
{
    TutSprite *s = tut_spr_get(i);

    s->u = u;
    s->v = v;
    s->tw = tw;
    s->th = th;
}

void tut_spr_set_pos(int i, int x, int y, int z, int w, int h)
{
    TutSprite *s = tut_spr_get(i);

    s->x = x;
    s->y = y;
    s->z = z;
    s->w = w;
    s->h = h;
}

void tut_spr_draw_one(int i, int flip)
{
    tut_spr.list = tut_spr_draw(tut_spr_get(i), tut_spr.list, flip);
}

void tut_sprites_reset(void)
{
    void *pack = tut_pack;

    memset(&tut_spr, 0, 0x604);
    memset(tut_hl_tgt, 0, 0xC);
    tut_tex[3] = ehp_find(pack, "tuto_duel01.gim", 0);
}

void tut_sprites_term(void)
{
}

void *tut_spr_begin(void)
{
    void *list = dl_select_layer(8, 0);

    list = dl_disable(list, 0);
    list = dl_disable(list, 3);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_disable(list, 2);
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    return tut_spr.list = dl_ambient_color(list, 0xFFFFFFFF);
}

void tut_spr_set_list(void *list)
{
    tut_spr.list = list;
}

void *tut_spr_get_list(void)
{
    return tut_spr.list;
}

void tut_spr_end(void *list)
{
    dl_set_list(list);
    tut_spr.list = 0;
}

/* textured, alpha-blended 2D state */
void *tut_dl_reset(void *list)
{
    list = dl_clear_color(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_disable(list, 0);
    list = dl_disable(list, 3);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_disable(list, 2);
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    return dl_ambient_color(list, 0xFFFFFFFF);
}

void *tut_dl_reset2(void *list)
{
    list = dl_clear_color(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_disable(list, 0);
    list = dl_disable(list, 3);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_disable(list, 2);
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    return dl_ambient_color(list, 0xFFFFFFFF);
}

/* untextured dark shade */
void *tut_dl_shade(void *list)
{
    list = dl_clear_color(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_disable(list, 0);
    list = dl_disable(list, 3);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_disable(list, 2);
    list = dl_disable(list, 9);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_color(list, 0xD0000000);
    return dl_set_tex_filter(list, 0, 0);
}

void *tut_quad(void *list, int x, int y, int w, int h, int u, int v, int tw, int th)
{
    return tut_quad_ex(list, x, y, w, h, u, v, tw, th, 0, 0);
}

void *tut_quad_ex(void *list, int x, int y, int w, int h, int u, int v, int tw, int th, int fu, int fx)
{
    list = draw_set_offsets(list, tw, th, w, h);
    return draw_sprite(list, x, y, 0, u, v, fu, fx);
}

/* the highlighted rectangles are cut out of a dark full-screen shade through the stencil */
void *tut_hl_render(void *list)
{
    TutHl *a = &tut_hl_cur[0];
    TutHl *b = &tut_hl_cur[1];

    list = dl_clear_depth(list, 0);
    list = dl_clear(list, 2);
    list = dl_disable(list, 9);
    list = dl_disable(list, 0);
    list = dl_enable(list, 4);
    list = dl_enable(list, 3);
    list = dl_stencil_func(list, 0, 4, 4);
    list = dl_stencil_op(list, 1, 0, 0xFF);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_color(list, 0);
    if (tut_hl_cur[0].state != 0)
        list = tut_quad(list, a->x0, a->y0, a->x1, a->y1, 0, 0, 0, 0);
    if (b->state != 0)
        list = tut_quad(list, b->x0, b->y0, b->x1, b->y1, 0, 0, 0, 0);
    list = dl_stencil_op(list, 3, 1, 0xFF);
    if (a->state != 0 || b->state != 0) {
        list = dl_color(list, 0xC0000000);
        list = draw_set_offsets(list, 0, 0, 0x1E0, 0x110);
        list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    }
    list = dl_clear_depth(list, 0);
    list = dl_clear(list, 2);
    return dl_disable(list, 3);
}

void tut_hl_clear(int i)
{
    if (i >= 0 && i < 3)
        memset(&tut_hl_tgt[i], 0, 0xC);
}

/* grow from / shrink to the full screen in 10 frames */
void tut_hl_update(void)
{
    int i;
    TutHl *t;
    TutHl *c;

    for (i = 0, t = tut_hl_tgt, c = tut_hl_cur; i < 2; i++, t++, c++) {
        c->state = t->state;
        if (t->state == 1) {
            c->x0 = t->x0 * t->t / 10;
            c->y0 = t->y0 * t->t / 10;
            c->x1 = 0x1E0 - (0x1E0 - t->x1) * t->t / 10;
            c->y1 = 0x110 - (0x110 - t->y1) * t->t / 10;
            if (t->t >= 10) {
                t->state = 2;
                t->t = 0;
                *c = *t;
            } else {
                t->t++;
            }
        } else if (t->state == 3) {
            c->x0 = t->x0 - t->x0 * t->t / 10;
            c->y0 = t->y0 - t->y0 * t->t / 10;
            c->x1 = t->x1 + (0x1E0 - t->x1) * t->t / 10;
            c->y1 = t->y1 + (0x110 - t->y1) * t->t / 10;
            if (t->t >= 10)
                tut_hl_clear(i);
            else
                t->t++;
        }
    }
}

void *tut_hl_draw(void *list)
{
    tut_hl_update();
    list = tut_dl_shade(list);
    list = gim_set_texture(list, tut_tex[3]);
    return tut_hl_render(list);
}

void tut_highlight_set(int i, int x0, int y0, int x1, int y1)
{
    if (i >= 0 && i < 3) {
        TutHl *h = &tut_hl_tgt[i];

        h->state = 1;
        h->x0 = x0;
        h->y0 = y0;
        h->x1 = x1;
        h->y1 = y1;
    }
}

void tut_highlight_off(int i)
{
    if (i >= 0 && i < 3 && tut_hl_tgt[i].state != 0)
        tut_hl_tgt[i].state = 3;
}

