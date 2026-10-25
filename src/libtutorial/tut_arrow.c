// @range 0x08807A10-0x08807E24
// @rodata 0x0880F850-0x0880F884
// @data 0x0880FA98-0x0880FAA8
/* Bobbing arrows that point at the field (markers 5..12, two arrow commands) and the per-frame
   draw of the overlays: sprite list, highlight, arrows. */
#include "libtutorial.h"

typedef struct TutMarker TutMarker;
struct TutMarker {               /* tut_markers 0x08812F50, 0x1C bytes */
    int on;
    int x, y;                    /* +0x04 */
    int dir;                     /* +0x0C 2 = points up */
    int timer;                   /* +0x10 */
    int *spr;                    /* +0x14 sprite id */
    void (*draw)(TutMarker *m);  /* +0x18 */
};

extern int tut_arrow_ids_b[3];   /* 0x08812F04 */
extern int tut_arrow_ids_a[5];   /* 0x08812F10 */
extern int tut_marker_unk[11];   /* 0x08812F24 */
extern TutMarker tut_markers[13];

/* tut_sprite.c */
int tut_spr_alloc(void *gim);
void tut_spr_set_tex(int i, int u, int v, int tw, int th);
void tut_spr_set_pos(int i, int x, int y, int z, int w, int h);
void tut_spr_draw_one(int i, int flip);
void tut_sprites_reset(void);
void tut_sprites_term(void);
void *tut_spr_begin(void);
void tut_spr_set_list(void *list);
void *tut_spr_get_list(void);
void tut_spr_end(void *list);
void *tut_dl_reset2(void *list);
void *tut_hl_draw(void *list);

/* 0..amp over period frames, in 1/30 steps */
unsigned int tut_bob(void *unused, unsigned int period, int amp)
{
    return amp * (tut.frame % period) / 30;
}

void tut_arrow_draw_a(TutMarker *m)
{
    int *id = m->spr;
    int bob = tut_bob(&m->timer, 10, 20);

    if (m->dir == 2)
        tut_spr_set_pos(*id, m->x - 0x10, m->y - bob, 0, 0x20, 0x26);
    else
        tut_spr_set_pos(*id, m->x - 0x10, m->y - 0x26 + bob, 0, 0x20, 0x26);
    tut_spr_draw_one(*id, m->dir);
}

void tut_arrow_draw_b(TutMarker *m)
{
    int *id = m->spr;
    int bob = tut_bob(&m->timer, 10, 20);

    if (m->dir == 2)
        tut_spr_set_pos(*id, m->x - 0x10, m->y - bob, 0, 0x20, 0x26);
    else
        tut_spr_set_pos(*id, m->x - 0x10, m->y - 0x26 + bob, 0, 0x20, 0x26);
    tut_spr_draw_one(*id, m->dir);
}

/* markers 5..9 and 10..12 are the arrows of the two arrow commands */
void tut_sprites_init(void)
{
    int i;
    void *gim;
    TutMarker *m;
    int *id;

    tut_sprites_reset();
    memset(tut_marker_unk, 0, 0x2C);
    memset(tut_markers, 0, 0x1C);
    gim = ehp_find(tut_pack, "tuto_duel01.gim", 0);
    for (i = 0, m = tut_markers; i < 13; i++, m++) {
        switch (i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            id = &tut_arrow_ids_a[i - 5];
            *id = tut_spr_alloc(gim);
            tut_spr_set_tex(*id, 0x47, 0, 0x20, 0x26);
            m->spr = id;
            m->draw = tut_arrow_draw_a;
            break;
        case 10:
        case 11:
        case 12:
            id = &tut_arrow_ids_b[i - 10];
            *id = tut_spr_alloc(gim);
            tut_spr_set_tex(*id, 0x47, 0, 0x20, 0x26);
            m->spr = id;
            m->draw = tut_arrow_draw_b;
            break;
        }
    }
}

void tut_sprites_term_thunk(void)
{
    tut_sprites_term();
}

void tut_sprites_draw(void)
{
    int i;
    TutMarker *m;

    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    tut_spr_set_list(tut_dl_reset2(tut_hl_draw(tut_spr_begin())));
    for (i = 0, m = tut_markers; i < 13; i++, m++) {
        if (m->on != 0 && m->draw != 0)
            m->draw(m);
    }
    tut_spr_end(tut_spr_get_list());
}

void tut_marker_show(int i, int on)
{
    tut_markers[i].on = on;
}

void tut_marker_place(int i, int x, int y, int dir)
{
    tut_markers[i].x = x;
    tut_markers[i].y = y;
    tut_markers[i].dir = dir;
}
