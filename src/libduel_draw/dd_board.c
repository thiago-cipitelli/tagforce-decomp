// @range 0x0880E45C-0x0880FCFC
// @rodata 0x0886FE64-0x0886FE80
// @data 0x08874EA0-0x08874F70
// @asm 0x0880E788-0x0880FAC4
/* Drawing of the field and of the card objects: the layer setup, the field sheet model, the
   deck/extra/graveyard piles (a pile model with the top card on it and the count), every card
   object with its picture (models with texture hooks), the counters of the monster zones. */
#include "dd_d1.h"

void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_get34(DdMtx *m);
void mtx_identity(void);
void mtx_translate(DdXyzw *v);
void mtx_scale3(float *v);
void mtx_mult(DdMtx *m);
void mtx_perspective(float fovy, float aspect, float near, float far);
DdXyzw *vec4_zero(DdXyzw *out);
void mat4_transform_vec4(DdXyzw *out, DdMtx *m, DdXyzw *v);
void mat4_mul(DdMtx *out, DdMtx *a, DdMtx *b);
void quat_to_mat4(DdMtx *out, DdXyzw *q);
void camera_set(void *pos, void *target, void *up);
void camera_load_view(void);
unsigned int *lighting_disable(unsigned int *list);
int model_anim_play(Model *m, int n, unsigned int flags);
void model_update(Model *m, float *dt);
void model_draw(void *m, void *params);
void *model_hook(Model *m, int which, void *fn);
unsigned int *gfx_get_state(void);
unsigned int *mtx_flush_alias(unsigned int *list);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
unsigned int *dl_viewport(unsigned int *list, int cx, int cy, int w, int h);
unsigned int *dl_ambient_light(unsigned int *list, unsigned int color);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_tex_func(void *list, int func, int comp);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(unsigned int *list, float u, float v);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
int wcslen(const unsigned short *s);
unsigned short *wtext_from_int(int v, unsigned short *out);
void *dd_card_texture(void *list, int id);
float dd_count_scale(int k, int v);
int dd_hl_count(int p, int zone);
void zone_view_save(int a);
int dv_must_answer(int player);
int dv_08835924(void);
unsigned int dv_08835AEC(int player, int zone, int index);
int dv_zone_pos(int player, int zone, int index);
int dv_zone_faceup(int player, int zone, int index);
int dv_zone_card_uid(int player, int zone, int index);
int sel_active(void);
float sel_progress(int player);
int dd_cobj_counter_shown(DdCobj *o, DdPos at);
void *dd_counter_draw(void *list, DdPos at, DdXyzw *pos);
void dd_counter_sel(DdXyzw *pos, int n);
void *dd_lpp_phase_draw(void *list);
void dd_cursor_get(DdLoc *cur);
void *dd_cursor_draw_model(void *list, DdLoc at, int on);

extern int dd_board_hilite;            /* 0x08C3D930 the zone drawn is highlighted */
extern int dd_board_card;              /* 0x08C3D934 card on top of the pile drawn, <0 none */

/* flag 0x40 of the scene: player 0 chooses a card on the field */
void dd_board_choose_flag(void)
{
    if (dv_must_answer(0)) {
        switch (dv_08835924()) {
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            dd_scene_flags(0x40, 1);
            return;
        }
    }
    dd_scene_flags(0x40, 0);
}

/* layer 3: stencil cleared, perspective and camera of the card objects */
void dd_board_begin(void)
{
    void *list = dl_clear(dl_clear_stencil(dl_select_layer(3, 0), 0), 0x14);

    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(1.23972404f, 480.0f / 272.0f, 100.0f, 10000.0f);
    mtx_set_mode(1);
    camera_set(&(DdXyz){ 0.0f, 680.0f, 700.0f }, &(DdXyz){ 0 }, &(DdXyz){ 0.0f, 1.0f, 0.0f });
    camera_load_view();
    zone_view_save(1);
    list = dl_viewport(list, 0x84A, 0x7FC, 0x3C0, 0x220);
    list = lighting_disable(list);
    list = dl_set_tex_filter(list, 1, 1);
    dl_set_list(list);
}

void dd_board_end(void)
{
    dl_set_list(dl_viewport(dl_select_layer(6, 0), 0x800, 0x800, 480, 272));
}

/* the field sheet */
void dd_board_sheet(void)
{
    int params[13];

    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    memset(params, 0, sizeof(params));
    params[0] = 3;
    params[1] = 3;
    model_draw(dd_model_get(0x10002), params);
    mtx_pop();
}

/* layer 3: 2D overlays, then the big viewport for the cards */
void dd_board_overlay(void)
{
    void *list = dl_viewport(dl_select_layer(3, 0), 0x800, 0x800, 480, 272);

    list = dd_lpp_phase_draw(list);
    list = dl_viewport(list, 0x84A, 0x7FC, 0x3C0, 0x220);
    list = dl_enable(list, 1);
    list = dl_ambient_light(list, 0xFFFFFFFF);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 10);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x14);
    dl_set_list(list);
}

int dd_board_piles_z[4] = { 12, 13, 14, 15 };
int dd_board_zones[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
DdXyzw dd_board_zaxis = { 0.0f, 0.0f, 1.0f, 0.0f };

void dd_board_pile(float scale, DdXyzw *pos, DdXyzw *rot, int n, int card);
void dd_board_pile_count(int n);
int dd_board_pile_tex(void **list, int unused, int which);
int dd_board_pile_color(void **list);
void dd_board_card_face(float scale, unsigned short card);

#ifdef NON_MATCHING /* funções grandes de desenho; forma não reproduzida */
/* the piles (side deck, deck, graveyard, banished) of both players */
void dd_board_piles(void)
{
    DdCobjs *b = &dd_cobjs;
    int p;
    unsigned int k;
    int zone;
    int n;
    int card;
    float scale;
    DdCobj *head;
    DdCobj *o;
    DdCobj *first;
    DdLoc at;
    DdXyzw pos;
    DdXyzw rot;
    float side;

    mtx_set_mode(2);
    mtx_push();
    for (p = 0; p < 2; p++) {
        for (k = 0; k < 4; k++) {
            zone = dd_board_piles_z[k];
            scale = 1.0f;
            card = 0;
            if (zone == 13 && b->en[p] == 0)
                continue;
            n = 0;
            head = dd_zone_list(p, zone);
            first = head->next;
            for (o = first; o != head; o = o->next) {
                if (o->t >= o->dur && !o->b4) {
                    card = o->card;
                    n++;
                }
            }
            if (zone == 15 || zone == 14) {
                if (!dv_zone_faceup(p, zone, n - 1))
                    card = 0;
            } else if ((zone == 13 || zone == 12) && first != head) {
                card = first->card;
                if (first->t < first->dur)
                    card = 0;
                else if (!first->b5) {
                    if (!dv_zone_faceup(p, zone, 0))
                        card = 0;
                    else if (first->uid != dv_zone_card_uid(p, zone, 0))
                        card = 0;
                }
            }
            dd_board_hilite = dd_scene_flags(0x40, -1) && (dv_08835AEC(p, zone, 0) & ~2) != 0;
            memset(&at, 0, sizeof(at));
            at.player = p;
            at.zone = zone;
            dd_slot_xform(&pos, &rot, *(DdPos *)&at);
            pos.y = 0.0f;
            if (zone - 12 < 2 && sel_active()) {
                side = pos.x >= 0.0f ? 100.0f : -100.0f;
                pos.x += side * sel_progress(p);
                scale = 1.0f - sel_progress(p);
            }
            dd_board_pile(scale, &pos, &rot, n, card);
            if (dd_scene_flags(0x80, -1) && n > 0)
                dd_board_pile_count(n);
            if (b->sel[p][zone] > 0)
                dd_counter_sel(&pos, n);
        }
    }
    mtx_pop();
}

int dd_board_card_draw(DdCobj *o, int p, int zone, int i);

/* every card object, the one under the cursor last; highlights of the zones to choose */
void dd_board_cards(void)
{
    int zones[16];
    DdLoc at = { 0 };
    unsigned int drawn[2] = { 0 };
    int p;
    unsigned int k;
    DdCobj *head;
    DdCobj *o;
    DdCobj *cur_o;
    int i;
    int zone;
    DdLoc cur;
    void *list;
    DdXyzw pos;
    DdXyzw rot;
    DdMtx m;

    memcpy(zones, dd_board_zones, sizeof(zones));
    list = dl_select_layer(4, 0);
    list = dl_enable(list, 5);
    list = dl_disable(list, 4);
    list = dl_disable(list, 0x15);
    list = dl_disable(list, 10);
    list = dl_tex_func(list, 0, 0);
    dl_set_list(list);
    mtx_set_mode(2);
    mtx_push();
    for (p = 0; p < 2; p++) {
        at.player = p;
        for (k = 0; k < 16; k++) {
            at.zone = zones[k];
            head = dd_zone_list(at.player, at.zone);
            cur_o = 0;
            for (o = head->next, i = 0; o != head; o = o->next, i++) {
                zone = at.zone;
                if ((zone == 15 || zone == 14 || zone == 13 || zone == 12) && o->t >= o->dur)
                    continue;
                if (o->b4)
                    continue;
                if (zone == 11) {
                    dd_cursor_get(&cur);
                    if (cur.player == p && cur.zone == 11 && cur.idx == i) {
                        cur_o = o;
                        continue;
                    }
                }
                list = dl_enable(dl_select_layer(4, 0), 1);
                list = dl_tex_func(list, at.zone == 11 ? 1 : 7, 0);
                dl_set_list(list);
                if (dd_board_card_draw(o, p, at.zone, i))
                    drawn[p] |= 1 << at.zone;
                dl_set_list(dl_tex_func(dl_select_layer(4, 0), 7, 0));
            }
            if (cur_o) {
                dd_cursor_get(&cur);
                list = dl_tex_func(dl_enable(dl_select_layer(4, 0), 1), 1, 0);
                dl_set_list(list);
                if (dd_board_card_draw(cur_o, p, 11, cur.idx))
                    drawn[p] |= 0x800;
                dl_set_list(dl_tex_func(dl_select_layer(4, 0), 7, 0));
            }
        }
    }
    for (p = 0; p < 2; p++) {
        for (zone = 0; zone < 16; zone++) {
            if (!(drawn[p] & 1 << zone) && dd_scene_flags(0x40, -1) &&
                (dv_08835AEC(p, zone, 0) & ~0x402) != 0) {
                drawn[p] |= 1 << zone;
                at.player = p;
                at.zone = zone;
                at.idx = 0;
                dd_slot_xform(&pos, &rot, *(DdPos *)&at);
                mtx_identity();
                mtx_translate(&pos);
                quat_to_mat4(&m, &rot);
                mtx_mult(&m);
                mtx_mult(&m);
                list = dl_select_layer(4, 0);
                list = dd_cursor_draw_model(list, at, 1);
                list = dl_enable(list, 5);
                dl_set_list(list);
            }
        }
    }
    list = dl_tex_func(dl_select_layer(4, 0), 1, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    dl_set_list(list);
    mtx_pop();
}

/* one card object at place i of (p, zone); 1 when it is highlighted */
int dd_board_card_draw(DdCobj *o, int p, int zone, int i)
{
    int hilite = 0;
    void *list = dl_select_layer(4, 0);
    float scale;
    DdXyzw off;
    DdMtx m;
    DdXyzw axis;
    DdLoc at = { 0 };
    float k;

    if (dd_scene_flags(0x40, -1) && (dv_08835AEC(p, zone, i) & ~2) != 0)
        hilite = 1;
    if (hilite && (*gfx_get_state() & 0x10)) {
        list = dl_tex_func(list, 4, 0);
        list = dl_disable(list, 10);
        list = dl_ambient_color(list, 0xFF404040);
    } else {
        list = dl_tex_func(list, 1, 0);
        list = dl_ambient_color(list, 0xFFFFFFFF);
    }
    dl_set_list(list);
    scale = dd_count_scale(p, zone);
    mtx_identity();
    memset(&off, 0, sizeof(off));
    if (zone == 11 && sel_active()) {
        k = 1.0f - sel_progress(p);
        off.y = o->pos.z >= 0.0f ? -150.0f : 250.0f;
        off.y *= sel_progress(p);
    }
    mtx_translate(&o->pos);
    mtx_translate(&off);
    quat_to_mat4(&m, &o->rot);
    mtx_mult(&m);
    dd_board_card_face(scale, 0);
    axis = dd_board_zaxis;
    quat_to_mat4(&m, &axis);
    mtx_mult(&m);
    dd_board_card_face(scale, o->card);
    if (hilite) {
        at.player = p;
        at.zone = zone;
        at.idx = i;
        list = dd_cursor_draw_model(dl_select_layer(4, 0), at, 1);
        list = dl_enable(list, 5);
        dl_set_list(list);
    }
    if (dd_cobjs.sel[p][zone] > 0)
        dd_counter_sel(&o->pos, 0);
    return hilite;
}

/* overlays of the cards on the field (attack/defense position markers...) */
void dd_board_marks(void)
{
    DdPos at = { 0 };
    void *list;
    void *gim;
    int p;
    int z;
    DdCobj *head;
    DdCobj *o;

    list = dl_select_layer(6, 0);
    list = dl_disable(list, 1);
    list = dl_tex_func(list, 0, 1);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    gim = dd_gim_get(0x40006);
    if (gim)
        list = gim_set_texture(list, gim);
    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            at.player = p;
            at.zone = z;
            at.idx = 0;
            at.f14 = dv_zone_faceup(p, z, 0);
            at.f15 = dv_zone_pos(p, z, 0);
            head = dd_zone_list(at.player, at.zone);
            o = head->next;
            if (o != head && dd_cobj_counter_shown(o, at))
                list = dd_counter_draw(list, at, &o->pos1);
        }
    }
    list = dl_enable(list, 1);
    dl_set_list(list);
}

/* the pile model with n cards and the top card */
void dd_board_pile(float scale, DdXyzw *pos, DdXyzw *rot, int n, int card)
{
    DdMtx m;
    float sc[3] = { 0 };
    float upd[4];
    Model *mdl;
    int params[13];

    if (n <= 0)
        return;
    sc[0] = scale;
    sc[1] = scale;
    sc[2] = scale;
    mtx_identity();
    mtx_translate(pos);
    quat_to_mat4(&m, rot);
    mtx_mult(&m);
    mtx_scale3(sc);
    mdl = dd_model_get(0x10003);
    memset(upd, 0, sizeof(upd));
    upd[0] = (float)n;
    model_anim_play(mdl, 0, 0);
    model_update(mdl, upd);
    model_hook(mdl, 0, dd_board_pile_tex);
    model_hook(mdl, 1, dd_board_pile_color);
    dd_board_card = card;
    memset(params, 0, sizeof(params));
    params[0] = 4;
    params[1] = 4;
    model_draw(mdl, params);
}

/* the count of a pile, projected on screen above it */
void dd_board_pile_count(int n)
{
    DdMtx view;
    DdMtx proj;
    DdMtx vp;
    DdMtx world;
    DdMtx all;
    DdXyzw p;
    DdXyzw s;
    unsigned short txt[16];
    unsigned short *c;
    int x;
    float y;
    void *list;
    void *gim;

    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(0);
    mtx_get34(&proj);
    mat4_mul(&vp, &proj, &view);
    mtx_set_mode(2);
    mtx_get34(&world);
    mat4_mul(&all, &vp, &world);
    vec4_zero(&p);
    p.y = dd_stack_y(n) + 0.1f;
    p.w = 1.0f;
    mat4_transform_vec4(&s, &all, &p);
    s.x = s.x * 480.0f / s.w + 2122.0f - 1808.0f;
    s.y = s.y * -272.0f / s.w + 2044.0f - 1912.0f;
    c = wtext_from_int(n, txt);
    x = (int)(s.x - (float)(wcslen(c) * 7 / 2));
    y = s.y - 4.0f;
    list = dl_select_layer(4, 0);
    gim = dd_gim_get(0x40006);
    if (gim)
        list = gim_set_texture(list, gim);
    list = dl_disable(list, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_tex_func(list, 0, 1);
    for (; *c; c++, x += 7) {
        list = draw_set_offsets(list, 7, 9, 7, 9);
        list = draw_sprite(list, x, (int)y, 0, (*c - '0') * 7, 0, 0, 0);
    }
    list = dl_set_tex_filter(list, 1, 1);
    list = dl_enable(list, 1);
    dl_set_list(list);
}
#endif

/* texture hook of the pile model: the top card picture */
int dd_board_pile_tex(void **list, int unused, int which)
{
    if (which != 1)
        return 0;
    if (dd_board_card < 0)
        return 0;
    *list = dl_tex_scale(dl_tex_offset(dl_tex_func(dl_enable(dd_card_texture(*list, dd_board_card), 9), 0, 0), 0.0f, 1.0f), 1.0f, -1.0f);
    return 1;
}

/* colour hook of the pile model: dimmed while it can be chosen (blinking) */
int dd_board_pile_color(void **list)
{
    void *l = *list;

    if (dd_board_hilite != 0 && (*gfx_get_state() >> 4 & 1)) {
        l = dl_tex_func(l, 4, 0);
        l = dl_disable(l, 10);
        *list = dl_ambient_color(l, 0xFF404040);
    } else {
        l = dl_tex_func(l, 1, 0);
        *list = dl_ambient_color(l, 0xFFFFFFFF);
    }
    return 0;
}

typedef struct DdCardVtx {
    float u, v;
    float x, y, z;
} DdCardVtx;

DdCardVtx dd_board_card_quad[4] = {
    { -0.4921875f, -0.4921875f, -31.0f, 0.0f, -44.0f },
    { -0.4921875f, 0.4921875f, -31.0f, 0.0f, 44.0f },
    { 0.4921875f, -0.4921875f, 31.0f, 0.0f, -44.0f },
    { 0.4921875f, 0.4921875f, 31.0f, 0.0f, 44.0f },
};

/* one face of a card (picture `card`, 0 the back) scaled for its zone */
void dd_board_card_face(float scale, unsigned short card)
{
    void *list = dl_select_layer(4, 0);
    float k;

    list = dl_tex_offset(list, 0.5f, 0.5f);
    k = scale / 64.0f + 1.0f;
    list = dl_tex_scale(list, k, k);
    list = dl_tex_wrap(list, 1, 1);
    list = dd_card_texture(list, card);
    list = mtx_flush_alias(list);
    list = dl_draw_array(list, 4, 0x183, 4, 0, dd_board_card_quad);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(list, 1.0f, 1.0f);
    list = dl_tex_wrap(list, 0, 0);
    dl_set_list(list);
}
