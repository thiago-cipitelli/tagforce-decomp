// @range 0x08815678-0x08816FD0
// @asm 0x088157A0-0x08815BC8
// @asm 0x08815BC8-0x08815D3C
// @asm 0x08815D3C-0x08815E74
// @asm 0x08816064-0x08816180
// @asm 0x08816180-0x08816240
// @asm 0x0881665C-0x08816964
// @asm 0x08816964-0x08816CD8
// @asm 0x08816DCC-0x08816FD0
// @data 0x088757D4-0x08875AC4
// @rodata 0x0887005C-0x08870074
/* The 3D field cursor: its pose follows the card under the cursor location, the cursor model
   (field_cursor00/01/02.iff, lit when the location can act), the arrow sprite and the icons of
   the actions available on the location; also the jump to the first selectable card and the
   cycling through selectable locations. */
#include "libduel_draw.h"

extern DdCursor dd_cursor;             /* 0x08C54640 */

typedef struct DdUv {                  /* sprite cell of the cursor arrow */
    int u, v;                          /* normal */
    int u2, v2;                        /* raised (hand row) */
} DdUv;

/* arrow sprite cell per (player * 16 + zone) */
DdUv dd_cursor_uv[32] = {
    { 64, 96, 0, 96 },
    { 64, 96, 0, 96 },
    { 64, 96, 0, 96 },
    { 64, 96, 0, 96 },
    { 64, 96, 0, 96 },
    { 64, 128, 0, 128 },
    { 64, 128, 0, 128 },
    { 64, 128, 0, 128 },
    { 64, 128, 0, 128 },
    { 64, 128, 0, 128 },
    { 64, 96, 0, 96 },
    { 0, 0, 0, 0 },
    { 64, 128, 0, 128 },
    { 0, 128, 0, 128 },
    { 64, 96, 0, 96 },
    { 64, 96, 0, 96 },
    { 64, 64, 0, 64 },
    { 64, 64, 0, 64 },
    { 64, 64, 0, 64 },
    { 64, 64, 0, 64 },
    { 64, 64, 0, 64 },
    { 64, 32, 0, 32 },
    { 64, 32, 0, 32 },
    { 64, 32, 0, 32 },
    { 64, 32, 0, 32 },
    { 64, 32, 0, 32 },
    { 64, 64, 0, 64 },
    { 64, 0, 64, 0 },
    { 64, 32, 0, 32 },
    { 64, 32, 0, 32 },
    { 64, 64, 0, 64 },
    { 64, 64, 0, 64 },
};

/* used by the functions kept in asm */
DdVec4 dd_cursor_rot0 __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 1.0f };
int dd_cursor_zone_order[2][16] = {
    { 11, 12, 5, 6, 7, 8, 9, 13, 10, 0, 1, 2, 3, 4, 14, 15 },
    { 15, 10, 0, 1, 2, 3, 4, 14, 12, 5, 6, 7, 8, 9, 13, 11 },
};

int func_0880B770(int player, int zone);
void *func_0880B7B8(int player, int zone, int idx);
void func_0880A8E0(float *pos, float *rot, DdLoc2 loc);
int func_0880702C(void);
int func_08806E58(DdLoc loc);
void func_088091C8(int id);
int func_08866BF0(int player, int zone, int idx);
int func_08866998(int h);
int func_088669B4(int a);
int func_08866A0C(void);
int func_0886683C(int player);
unsigned int func_088669CC(int player, int zone, int idx);
int func_08866BD8(int player, int zone);
void *func_08804268(int id);
void *func_088044B0(int id);
int func_08806BC4(void);
float func_0880CC04(int n);
unsigned int *func_088212C4(unsigned int *list, int x, int y, int icon);
int func_08866884(int player);
int func_0886689C(int player);
void dd_cursor_update(void);
unsigned int *dd_cursor_draw_normal(unsigned int *list);
unsigned int *dd_cursor_draw_locked(unsigned int *list);
void dd_cursor_hand_toggle(void);
void dd_cursor_next_target(void);
int dd_cursor_nearest_slot(int player, DdCursor *c);
int dd_cursor_nearest_hand(int player, DdCursor *c);
int dd_cursor_alpha_hook(unsigned int **plist, void *mdl, int mesh, int part, int after);
int dd_cursor_tex_locked(unsigned int **plist, int *mdl, int mesh, int part);
int dd_cursor_tex_normal(unsigned int **plist, int *mdl, int mesh, int part);
unsigned int *dd_cursor_draw_model(unsigned int *list, DdLoc loc, int locked);
unsigned int *dd_cursor_draw_arrow(unsigned int *list, DdLoc loc, int raised, int blink_bit);
unsigned int *dd_cursor_draw_icons(unsigned int *list, DdLoc loc);

void dd_cursor_draw(void)
{
    DdCursor *c = &dd_cursor;
    float m[16];
    unsigned int *list;
    int h;

    dd_cursor_update();
    if (dd_cursor.cur.player == 0)
        c->lit = 0;
    else
        c->lit = 1;
    if (func_08806E58(c->cur) != 0) {
        h = func_08866BF0(c->cur.player, c->cur.zone, c->cur.idx);
        if (h > 0) {
            if (func_08866998(h) != 0)
                c->lit = 1;
            else
                c->lit = 0;
        }
    }
    list = dl_enable(dl_select_layer(4, 0), 1);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(c->pos);
    quat_to_mat4(m, c->rot);
    mtx_mult(m);
    if (c->locked != 0)
        list = dd_cursor_draw_locked(list);
    else
        list = dd_cursor_draw_normal(list);
    mtx_pop();
    dl_set_list(list);
}

/* follows the card under the cursor and notifies the listeners when the location changed */
#ifdef NON_MATCHING /* muitos bitfields; alocação de registradores */
void dd_cursor_update(void)
{
    DdCursor *c = &dd_cursor;
    float pos[4] = { 0 };
    float rot[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    int player, zone, idx, n;
    int arg;
    int *card;
    int i;
    DdLoc2 l;
    DdLoc t = { 0 };

    if (dd_cursor.cur.zone == 0xB) {
        n = func_0880B770(c->cur.player, 0xB);
        if (n == 0) {
            dd_cursor_set(c->cur.player, dd_cursor_nearest_slot(c->cur.player, c), 0, 0);
        } else {
            if (c->cur.idx >= n)
                c->cur.idx = n - 1;
            card = func_0880B7B8(c->cur.player, c->cur.zone, c->cur.idx);
            if (card != 0) {
                *(DdVec4 *)pos = *(DdVec4 *)(card + 4);
                *(DdVec4 *)rot = *(DdVec4 *)(card + 16);
            }
        }
    }
    if (c->cur.zone != 0xB) {
        l.player = c->cur.player;
        l.zone = c->cur.zone;
        l.idx = c->cur.idx;
        l.b14 = 0;
        l.b15 = 0;
        func_0880A8E0(pos, rot, l);
        pos[1] = 0.0f;
    }
    *(DdVec4 *)c->pos = *(DdVec4 *)pos;
    *(DdVec4 *)c->rot = *(DdVec4 *)rot;
    if (c->dirty == 0)
        return;
    player = c->cur.player;
    zone = c->cur.zone;
    arg = 0;
    idx = c->cur.idx;
    switch (zone) {
    case 0xE:
        idx = func_08866884(player) - 1;
        break;
    case 0xF:
        idx = func_0886689C(player) - 1;
        break;
    case 0xD:
        idx = 0;
        break;
    case 0xC:
        idx = 0;
        break;
    }
    if (idx < 0)
        idx = 0;
    card = func_0880B7B8(player, zone, idx);
    if (card != 0) {
        if (func_0880702C() == 0) {
            t.player = player;
            t.zone = zone;
            t.idx = idx;
            if (func_08806E58(t) == 0 && !((char *)card)[0x8D] >> 5 & 1) {
                func_088091C8(0);
                goto notify;
            }
        }
        func_088091C8(card[0x88 / 4]);
        arg = card[0x88 / 4];
    }
notify:
    for (i = 0; i < 16; i++) {
        if (c->listeners[i] != 0)
            c->listeners[i](c->cur, arg);
    }
    c->dirty = 0;
}
#endif

/* the first selectable location of the battle phase */
#ifdef NON_MATCHING /* alocação de registradores */
void dd_cursor_first_target(void)
{
    int order[2][16] = {
        { 11, 12, 5, 6, 7, 8, 9, 13, 10, 0, 1, 2, 3, 4, 14, 15 },
        { 15, 10, 0, 1, 2, 3, 4, 14, 12, 5, 6, 7, 8, 9, 13, 11 },
    };
    int player, k, zone, n, i;
    unsigned int f;

    if (func_088669B4(0) == 0)
        return;
    switch (func_08866A0C()) {
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        for (player = 0; player < 2; player++) {
            for (k = 0; k < 16; k++) {
                zone = order[player][k];
                if (zone == 0xB)
                    n = func_0886683C(player);
                else
                    n = 1;
                for (i = 0; i < n; i++) {
                    f = func_088669CC(player, zone, i);
                    if (f != 0 && (f & 0x808) != 0) {
                        dd_cursor_set(player, zone, i, 0);
                        return;
                    }
                }
            }
        }
        break;
    }
}
#endif

/* square button: cycles the cursor */
#ifdef NON_MATCHING /* teste do laço de zonas no topo e booleano materializado */
void dd_cursor_cycle(void)
{
    int found;
    int player, zone, n, i;

    switch (func_08866A0C()) {
    case 2:
        dd_cursor_hand_toggle();
        break;
    case 1:
        return;
    case 0:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    default:
        found = 0;
        for (player = 0; player < 2; player++) {
            for (zone = 0; zone < 16; zone++) {
                if (zone == 0xB)
                    n = func_08866BD8(player, zone);
                else
                    n = 1;
                for (i = 0; i < n; i++) {
                    if ((func_088669CC(player, zone, i) & ~0x402) != 0) {
                        found = 1;
                        goto out;
                    }
                }
            }
        }
    out:
        if (found == 0)
            dd_cursor_hand_toggle();
        else
            dd_cursor_next_target();
        break;
    }
}
#endif

/* draws the cursor model of a location */
unsigned int *dd_cursor_draw_model(unsigned int *list, DdLoc loc, int locked)
{
    int id;
    void *mdl;
    int draw[13];

    switch (loc.zone) {
    default:
        id = 0x1000E;
        if (loc.player > 0)
            id++;
        break;
    case 0xB:
        if (loc.player == 0)
            id = 0x10011;
        else
            id = 0x10013;
        break;
    case 0xA:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
        id = 0x10012;
        if (loc.player > 0)
            id++;
        break;
    }
    if (locked != 0)
        id += 8;
    mdl = func_08804268(id);
    memset(draw, 0, sizeof(draw));
    draw[0] = dl_get_layer();
    draw[1] = draw[0];
    model_hook(mdl, 1, dd_cursor_alpha_hook);
    if (locked != 0)
        model_hook(mdl, 0, dd_cursor_tex_locked);
    else
        model_hook(mdl, 0, dd_cursor_tex_normal);
    dl_set_list(list);
    model_draw(mdl, draw);
    return dl_select_layer(draw[0], 0);
}

/* registers a listener of the cursor location */
void dd_cursor_listen(DdCursorCb cb)
{
    DdCursor *c = &dd_cursor;
    int i;

    for (i = 0; i < 16; i++) {
        if (c->listeners[i] == 0)
            break;
    }
    if (i != 16)
        c->listeners[i] = cb;
}

void dd_cursor_unlisten(DdCursorCb cb)
{
    DdCursor *c = &dd_cursor;
    int i;

    for (i = 0; i < 16; i++) {
        if (c->listeners[i] == cb) {
            c->listeners[i] = 0;
            return;
        }
    }
}

/* the monster slot (5..9) closest to pos */
#ifdef NON_MATCHING /* alocação de registradores */
int dd_cursor_nearest_slot(int player, DdCursor *c)
{
    float best = 1e16f;
    float pos[4];
    float rot[4];
    int i, k;
    float d;
    DdLoc2 l;

    l.player = player;
    l.idx = 0;
    l.b14 = 0;
    l.b15 = 0;
    k = 0;
    for (i = 0; i < 5; i++) {
        l.zone = i + 5;
        func_0880A8E0(pos, rot, l);
        d = vec4_distance_xyz(pos, c->pos);
        if (d < best) {
            best = d;
            k = i;
        }
    }
    return k + 5;
}
#endif

/* the hand card closest to pos */
#ifdef NON_MATCHING /* alocação de registradores (s0..s4) */
int dd_cursor_nearest_hand(int player, DdCursor *c)
{
    float best;
    int n;
    int i;
    int k;
    int *card;
    float d;

    best = 1e16f;
    k = -1;
    n = func_0880B770(player, 0xB);
    for (i = 0; i < n; i++) {
        card = func_0880B7B8(player, 0xB, i);
        if (card != 0) {
            d = vec4_distance_xyz((float *)(card + 4), c->pos);
            if (d < best) {
                best = d;
                k = i;
            }
        }
    }
    return k;
}
#endif

int dd_cursor_alpha_hook(unsigned int **plist, void *mdl, int mesh, int part, int after)
{
    unsigned int *list = *plist;

    if (after == 0)
        *plist = dl_alpha_test(dl_enable(list, 0), 6, 0, 0xFF);
    else
        *plist = dl_disable(list, 0);
    return 0;
}

/* locked cursor: field_cursor02 animated in 3 frames */
int dd_cursor_tex_locked(unsigned int **plist, int *mdl, int mesh, int part)
{
    DdTexMat *m = *(DdTexMat **)(mdl[3] + mesh * 0xA4 + part * 0x20 + 0x1C);
    void *gim;
    unsigned int *list;

    if (m == 0)
        return 0;
    if (strcmp("field_cursor02.iff", m->name) != 0)
        return 0;
    gim = func_088044B0((gfx_get_state()[0] >> 2) % 3 + 0x40021);
    if (gim == 0)
        return 0;
    list = dl_enable(*plist, 9);
    list = gim_set_texture_frame(list, gim, 0);
    list = dl_tex_func(list, m->func, m->comp);
    list = dl_tex_env_color(list, m->env);
    list = dl_tex_wrap(list, m->wrap_u, m->wrap_v);
    list = dl_tex_offset(m->u, 1.0f - m->v, list);
    *plist = dl_tex_scale(m->su, -m->sv, list);
    return 1;
}

/* normal cursor: field_cursor00/01 lit when the location can act */
int dd_cursor_tex_normal(unsigned int **plist, int *mdl, int mesh, int part)
{
    DdCursor *c = &dd_cursor;
    DdTexMat *m = *(DdTexMat **)(mdl[3] + mesh * 0xA4 + part * 0x20 + 0x1C);
    int id;
    void *gim;
    unsigned int *list;

    if (m == 0)
        return 0;
    if (c->lit == 0)
        return 0;
    if (strcmp("field_cursor00.iff", m->name) == 0)
        id = 0x40027;
    else if (strcmp("field_cursor01.iff", m->name) == 0)
        id = 0x40028;
    else
        return 0;
    gim = func_088044B0(id);
    if (gim == 0)
        return 0;
    list = dl_enable(*plist, 9);
    list = gim_set_texture_frame(list, gim, 0);
    list = dl_tex_func(list, m->func, m->comp);
    list = dl_tex_env_color(list, m->env);
    list = dl_tex_wrap(list, m->wrap_u, m->wrap_v);
    list = dl_tex_offset(m->u, 1.0f - m->v, list);
    *plist = dl_tex_scale(m->su, -m->sv, list);
    return 1;
}

unsigned int *dd_cursor_draw_normal(unsigned int *list)
{
    DdCursor *c = &dd_cursor;
    int raised;
    int layer;

    list = dd_cursor_draw_model(list, dd_cursor.cur, 0);
    if (func_08806BC4() == 0) {
        if (c->cur.player == 0 && c->cur.zone == 0xD)
            raised = 1;
        else
            raised = 0;
        list = dd_cursor_draw_arrow(list, c->cur, raised, 3);
    }
    if (func_08806BC4() == 0) {
        layer = dl_get_layer();
        dl_set_list(list);
        list = dd_cursor_draw_icons(dl_select_layer(5, 0), c->cur);
        dl_set_list(list);
        list = dl_select_layer(layer, 0);
    }
    return list;
}

unsigned int *dd_cursor_draw_locked(unsigned int *list)
{
    list = dd_cursor_draw_model(list, dd_cursor.cur, 0);
    return dd_cursor_draw_arrow(list, dd_cursor.cur, 1, 1);
}

/* screen position of the current model matrix origin */
#define DD_SCREEN_X(m) ((480.0f * (m)[12]) / (m)[15] + 2122.0f - 1808.0f)
#define DD_SCREEN_Y(m) ((-272.0f * (m)[13]) / (m)[15] + 2044.0f - 1912.0f)

/* the arrow sprite over the location (blinks with bit blink_bit of the frame counter) */
DdVec3 dd_cursor_icons_up = { 0.0f, 0.0f, -60.0f };
DdVec3 dd_cursor_icons_down = { 0.0f, 0.0f, 60.0f };

#ifdef NON_MATCHING /* cadeia de testes vira teste de faixa; quadro 16 bytes menor */
unsigned int *dd_cursor_draw_arrow(unsigned int *list, DdLoc loc, int raised, int blink_bit)
{
    float model[16];
    float view[16];
    float proj[16];
    float res[16];
    float sx, sy;
    int zone;
    int k;
    DdUv *uv;
    int dx, dy, du;
    void *gim;

    mtx_set_mode(2);
    mtx_push();
    zone = loc.zone;
    if (zone == 0xF || zone == 0xC || zone == 0xE || zone == 0xD) {
        float ofs[3] = { 0 };

        ofs[1] = func_0880CC04(func_0880B770(loc.player, zone));
        mtx_translate(ofs);
    }
    mtx_set_mode(2);
    mtx_get34(model);
    mtx_set_mode(1);
    mtx_get34(view);
    mtx_set_mode(0);
    mtx_get34(proj);
    mat4_mul(res, view, model);
    mat4_mul(res, proj, res);
    mtx_set_mode(2);
    sx = DD_SCREEN_X(res);
    sy = DD_SCREEN_Y(res);
    mtx_set_mode(2);
    mtx_pop();
    k = zone + loc.player * 16;
    dx = 0;
    dy = -0xB;
    du = 0;
    uv = &dd_cursor_uv[k];
    if (k == 0xB) {
        dx = -6;
        dy = -0x2E;
    }
    if (raised != 0) {
        dx = -0x10;
        dy = -0x20;
        uv = (DdUv *)&uv->u2;
    }
    if ((1 << blink_bit) & gfx_get_state()[0])
        du = 0x20;
    list = dl_depth_mask(list, 0);
    list = dl_alpha_test(dl_enable(list, 0), 6, 0, 0xFF);
    gim = func_088044B0(0x4001D);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, 0x20, 0x20, 0x20, 0x20);
    list = draw_sprite(list, (int)(sx + (float)dx), (int)(sy + (float)dy), 0x7FFF, uv->u + du, uv->v, 0, 0);
    return dl_enable(list, 0);
}
#endif

/* the icons of the actions possible on the location (up to 13, centered over it) */
#ifdef NON_MATCHING /* quadro e alocação */
unsigned int *dd_cursor_draw_icons(unsigned int *list, DdLoc loc)
{
    float pos[3];
    float ofs[3];
    float a[3] = { 0.0f, 0.0f, -60.0f };
    float b[3] = { 0.0f, 0.0f, 60.0f };
    float model[16];
    float view[16];
    float proj[16];
    float res[16];
    int icons[13];
    int deck = 0;
    float sx, sy;
    unsigned int f;
    int n, i, x, y;

    mtx_set_mode(2);
    mtx_push();
    mtx_get34(model);
    if (model[14] >= 0.0f) {
        if (loc.zone == 0xD) {
            float z[3] = { 0 };

            deck = 1;
            *(DdVec3 *)pos = *(DdVec3 *)z;
        } else {
            *(DdVec3 *)ofs = *(DdVec3 *)a;
            *(DdVec3 *)pos = *(DdVec3 *)ofs;
        }
    } else {
        *(DdVec3 *)ofs = *(DdVec3 *)b;
        *(DdVec3 *)pos = *(DdVec3 *)ofs;
    }
    mtx_translate(pos);
    mtx_set_mode(2);
    mtx_get34(model);
    mtx_set_mode(1);
    mtx_get34(view);
    mtx_set_mode(0);
    mtx_get34(proj);
    mat4_mul(res, view, model);
    mat4_mul(res, proj, res);
    sx = DD_SCREEN_X(res);
    sy = DD_SCREEN_Y(res);
    mtx_set_mode(2);
    mtx_pop();
    if (deck) {
        sx += -6.0f;
        sy += -33.0f;
    }
    n = 0;
    f = func_088669CC(loc.player, loc.zone, loc.idx);
    for (i = 0; i < 13; i++) {
        if (f & (1 << i))
            icons[n++] = i;
    }
    x = (int)(sx - (float)(n * 14 / 2));
    y = (int)(sy - 13.0f);
    list = dl_disable(list, 1);
    list = dl_enable(list, 4);
    list = dl_enable(list, 9);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    for (i = 0; i < n; i++) {
        list = func_088212C4(list, x, y, icons[i]);
        x += 14;
    }
    return dl_enable(list, 1);
}
#endif

/* jumps between the hand and the field */
void dd_cursor_hand_toggle(void)
{
    DdCursor *c = &dd_cursor;

    se_play(0x6212);
    if (dd_cursor.cur.player == 0 && c->cur.zone == 0xB) {
        c->hand_idx = c->cur.idx;
        if (c->hand_back.player != 0 || c->hand_back.zone != 0xB)
            dd_cursor_set(c->hand_back.player, c->hand_back.zone, c->hand_back.idx, 0);
        else
            dd_cursor_set(0, 2, 0, 0);
    } else if (func_0886683C(0) != 0) {
        c->hand_back = c->cur;
        dd_cursor_set(0, 0xB, c->hand_idx, 0);
    }
}

/* the next selectable location after the current one (wrapping to the first) */
#ifdef NON_MATCHING /* teste do laço no topo */
void dd_cursor_next_target(void)
{
    DdCursor *c = &dd_cursor;
    int have = 0;
    int passed = 0;
    int player, zone, n, i;
    DdLoc l;

    se_play(0x6212);
    for (player = 0; player < 2; player++) {
        for (zone = 0; zone < 16; zone++) {
            if (zone == 0xB)
                n = func_08866BD8(player, zone);
            else
                n = 1;
            for (i = 0; i < n; i++) {
                if ((func_088669CC(player, zone, i) & ~0x402) == 0)
                    continue;
                if (have == 0) {
                    l.player = player;
                    l.zone = zone;
                    l.idx = i;
                    have = 1;
                }
                if (passed != 0) {
                    l.player = player;
                    l.zone = zone;
                    l.idx = i;
                    goto done;
                }
                if (player == c->cur.player && zone == c->cur.zone && i == c->cur.idx)
                    passed = 1;
            }
        }
    }
done:
    dd_cursor_set(l.player, l.zone, l.idx, 0);
}
#endif
