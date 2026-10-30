// @range 0x0884E0F4-0x0884F3D0
// @asm 0x0884E1E8-0x0884E41C
// @asm 0x0884E41C-0x0884E46C
// @data 0x08876D30-0x08876DA0
/* Card cursor marker and attack arrow (dd_mark): a model standing on the zone under the cursor,
   turned towards the zone it points at and gliding between zones, and an arrow of 60 particles
   along a Hermite curve with a head billboard, lit by a wave that runs from the attacker to the
   target (perticle_for_arrow.gim, arrow_head.gim). */
#include "libduel_draw.h"

void *memset(void *, int, unsigned int);
int memcmp2(const void *a, const void *b, int n);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
int dl_get_layer(void);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_tex_func(void *list, int func, int comp);
void *dl_shade_model(void *list, int mode);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
void *dl_alloc_inline(void *list, int bytes);
void *gim_set_texture(void *list, void *gim);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *ehp_find(void *pack, const char *name, int *size);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_load(FxMat4 *m);
void mtx_get34(FxMat4 *m);
void mtx_translate(FxVec4 *v);
void mtx_mult(FxMat4 *m);
FxMat4 *mat4_identity(FxMat4 *out);
FxMat4 *mat4_translate(FxMat4 *out, FxMat4 *m, FxVec4 *v);
void mat4_mul(FxMat4 *r, FxMat4 *a, FxMat4 *b);
FxMat4 *mat4_inverse_ortho(FxMat4 *out, FxMat4 *m);
FxMat4 *quat_to_mat4(FxMat4 *out, FxVec4 *q);
FxVec4 *quat_from_axis_angle(FxVec4 *out, FxVec4 *axis, float angle);
FxVec4 *vec4_lerp(FxVec4 *out, FxVec4 *a, FxVec4 *b, float t);
FxVec3 *vec3_scale(FxVec3 *out, FxVec3 *v, float s);
FxVec3 *vec3_add(FxVec3 *out, FxVec3 *a, FxVec3 *b);
float atan2f(float y, float x);
void model_update(void *m, float *dt);
void model_draw(void *m, void *c);
void req_flag_set(int i);
void se_play(int id);

void *dd_alloc(int size);
void dd_free(void *p);
DdObj *dd_task_add(DdObj *desc);
void dd_task_remove_cur(void);
void dd_task_remove(DdObj *o);
void dd_cursor_unlisten(void (*fn)(short, int));
void dd_slot_xform(void *pos, void *rot, short at);
float dd_speed_get(void);
void dd_cursor_get(short *loc);
void *dd_gim_get(int id);
void *dd_model_get(int id);
void *dd_pack_get(int id);
unsigned int dv_08835AEC(int player, int zone, int idx);
int dd_scene_flags(int a, int b);
int func_088062E4(void);

extern DdMarkMgr dd_mark;

void dd_mark_create();
void dd_mark_reset(void);
void dd_mark_draw(DdObj *o, DdMarker *m);
void dd_mark_free(DdObj *o, DdMarker *m);
void dd_mark_idle(void);
void dd_mark_glide(DdObj *o, DdMarker *m);
void dd_mark_point(short loc, int unused);
void dd_mark_aim(DdLoc loc, int unused, int hl);
int dd_arrow_ok(DdLoc loc, int hl);
void dd_arrow_new(FxVec3 *from, FxVec3 *to, unsigned int color);
void dd_arrow_kill(void);
void dd_arrow_free(DdObj *o, DdArrow *a);
void dd_arrow_draw(DdObj *o, DdArrow *a);
void dd_arrow_update(DdObj *o, DdArrow *a);
void *dd_billboard(void *list, FxVec3 *pos, FxVtx *v);
void dd_hermite(float t, FxVec3 *out, FxVec3 *p0, FxVec3 *m0, FxVec3 *p1, FxVec3 *m1);

static FxVec4 dd_quat_id = { 0.0f, 0.0f, 0.0f, 1.0f };
static FxVec4 dd_axis_y = { 0.0f, 1.0f, 0.0f, 0.0f };

void dd_mark_init(void)
{
    memset(&dd_mark, 0, sizeof(dd_mark));
}

void dd_mark_term(void)
{
    DdMarkMgr *k = &dd_mark;

    if (k->marker != 0)
        dd_task_remove(k->marker);
    memset(k, 0, sizeof(*k));
}

void dd_mark_show(void)
{
    if (dd_mark.marker == 0)
        dd_mark_create();
    else
        dd_mark_reset();
}

void dd_mark_hide(void)
{
    DdMarkMgr *k = &dd_mark;

    if (k->marker != 0) {
        dd_cursor_unlisten(dd_mark_point);
        dd_task_remove(k->marker);
        k->marker = 0;
    }
    if (k->arrow != 0)
        dd_arrow_kill();
}

#ifdef NON_MATCHING /* os parâmetros ficam em $s0-$s3 e os ponteiros em $s4/$s5 no original, e a ordem das gravações dos campos de bits intercala os dois DdPos; nenhuma forma reproduziu */
/* Glides the marker from zone (player, zone) to (to_player, to_zone), turned to the target. */
void dd_mark_move(int player, int zone, int to_player, int to_zone)
{
    DdMarkMgr *k = &dd_mark;
    DdObj *o;
    DdMarker *m;
    float rot[4];
    DdPos p;
    DdLoc l;

    if (k->marker != 0)
        dd_mark_reset();
    else
        dd_mark_show();
    o = k->marker;
    m = o->data;
    if (to_zone >= 5)
        to_zone = 11;
    l.player = to_player;
    l.zone = to_zone;
    l.idx = 0;
    dd_mark_aim(l, 0, 1);
    m->moving = 1;
    m->at.player = player;
    m->at.zone = zone;
    p.player = to_player;
    m->to.player = to_player;
    p.zone = to_zone;
    p.idx = 0;
    m->to.zone = to_zone;
    p.f14 = 0;
    p.f15 = 0;
    dd_slot_xform(&m->dest, rot, *(short *)&p);
    m->dest.y = -2.0f;
    o->update = (DdObjFn)dd_mark_glide;
    o->time = 0.0f;
    m->dur = 0.5f / dd_speed_get();
    m->from = m->pos;
    req_flag_set(11);
}
#endif

#ifdef NON_MATCHING /* o original mantém o `beql zone → li 1` sem dobrar em sltiu; as formas testadas dobram */
int dd_mark_is_at(int player, int zone)
{
    DdMarkMgr *k = &dd_mark;
    DdMarker *m;

    if (k->marker == 0)
        return 0;
    m = k->marker->data;
    if (m->at.player == player) {
        if (m->at.zone == zone)
            return 1;
        return 0;
    }
    return 0;
}
#endif

void dd_mark_create(int player, int zone)
{
    DdMarker *m;
    DdObj desc;

    m = dd_alloc(sizeof(DdMarker));
    m->at.player = player;
    m->at.zone = zone;
    memset(&desc, 0, sizeof(desc));
    desc.data = m;
    dd_mark.marker = dd_task_add(&desc);
    dd_mark_reset();
}

void dd_mark_reset(void)
{
    DdObj *o = dd_mark.marker;
    DdMarker *m = o->data;
    DdPos p;
    short cur[4];

    p.player = m->at.player;
    p.zone = m->at.zone;
    p.idx = 0;
    p.f14 = 0;
    p.f15 = 0;
    dd_slot_xform(m, &m->rot, *(short *)&p);
    m->pos.y += 30.0f;
    m->moving = 0;
    o->draw = (DdObjFn)dd_mark_draw;
    o->update = (DdObjFn)dd_mark_idle;
    o->free = (DdObjFn)dd_mark_free;
    dd_cursor_get(cur);
    dd_mark_point(cur[0], 0);
}

void dd_mark_draw(DdObj *o, DdMarker *m)
{
    void *list;
    void *tex;
    void *model;
    int layer;
    int c[13];
    FxMat4 mtx;

    list = dl_select_layer(4, 0);
    tex = dd_gim_get(0x40005);
    if (tex != 0)
        list = gim_set_texture(list, tex);
    quat_to_mat4(&mtx, &m->rot);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(&m->pos);
    mtx_mult(&mtx);
    model = dd_model_get(0x1001C);
    if (model != 0) {
        layer = dl_get_layer();
        dl_set_list(dl_disable(list, 1));
        memset(c, 0, sizeof(c));
        c[0] = layer;
        c[1] = layer;
        model_update(model, 0);
        model_draw(model, c);
        list = dl_enable(dl_select_layer(layer, 0), 1);
    }
    mtx_pop();
    dl_set_list(list);
}

void dd_mark_free(DdObj *o, DdMarker *m)
{
    if (m != 0)
        dd_free(m);
}

void dd_mark_idle(void)
{
    short cur[2];

    dd_cursor_get(cur);
    dd_mark_point(cur[0], 0);
}

void dd_mark_glide(DdObj *o, DdMarker *m)
{
    if (o->time >= m->dur) {
        m->pos = m->dest;
    } else {
        vec4_lerp(&m->pos, &m->from, &m->dest, o->time / m->dur);
        req_flag_set(11);
    }
}

void dd_mark_point(short loc, int unused)
{
    dd_mark_aim(*(DdLoc *)&loc, unused, 0);
}

/* Turns the marker towards `loc` and draws the arrow to it (red, blue when hl is set). */
void dd_mark_aim(DdLoc loc, int unused, int hl)
{
    DdMarkMgr *k = &dd_mark;
    DdMarker *m;
    DdPos p;
    FxVec4 pos;
    FxVec4 rot;
    FxVec4 tmp;
    FxVec4 axis;
    FxVec4 q;
    float dz;
    float dx;

    if (dd_mark.marker == 0)
        return;
    m = dd_mark.marker->data;
    if (m->moving != 0)
        return;
    p.player = loc.player;
    p.zone = loc.zone;
    p.idx = 0;
    p.f14 = 0;
    p.f15 = 0;
    dd_slot_xform(&pos, &tmp, *(short *)&p);
    dx = pos.x - m->pos.x;
    dz = pos.z - m->pos.z;
    if (dx == 0.0f && dz == 0.0f) {
        q = dd_quat_id;
        rot = *(FxVec4 *)&q;
    } else {
        axis = dd_axis_y;
        quat_from_axis_angle(&rot, &axis, atan2f(-dx, -dz));
    }
    m->rot = rot;
    if (k->arrow != 0 && (memcmp2(&k->arrow_from, &m->at, 2) != 0 || memcmp2(&k->arrow_to, &loc, 2) != 0))
        dd_arrow_kill();
    if (dd_arrow_ok(loc, hl)) {
        dd_arrow_new((FxVec3 *)m, (FxVec3 *)&pos, hl != 0 ? 0xFF0606FF : 0xFFFF6060);
        k->arrow_from = *(short *)&m->at;
        k->arrow_to = *(short *)&loc;
    }
}

typedef struct { unsigned int player:1; unsigned int zone:5; unsigned int idx:10; } DdLocI;
int dd_arrow_ok(DdLoc loc, int hl)
{
    int zone;
    unsigned int r;

    if (dd_mark.arrow != 0)
        return 0;
    if (hl != 0 && (zone = loc.zone) == 11) {
    } else {
        zone = loc.zone;
        if (zone < 0 || zone > 4)
            return 0;
    }
    r = dv_08835AEC(loc.player, zone, loc.idx);
    if (hl != 0 || (r & 0x800))
        return 1;
    return 0;
}

void dd_arrow_new(FxVec3 *from, FxVec3 *to, unsigned int color)
{
    DdArrow *a;
    void *pack;
    DdObj desc;

    a = dd_alloc(sizeof(DdArrow));
    memset(a, 0, sizeof(DdArrow));
    pack = dd_pack_get(0x80001);
    a->tex_ptcl = ehp_find(pack, "perticle_for_arrow.gim", 0);
    a->tex_head = ehp_find(pack, "arrow_head.gim", 0);
    a->alive = 1;
    a->from = *from;
    a->to = *to;
    a->color = color;
    memset(&desc, 0, sizeof(desc));
    desc.draw = (DdObjFn)dd_arrow_draw;
    desc.update = (DdObjFn)dd_arrow_update;
    desc.free = (DdObjFn)dd_arrow_free;
    desc.data = a;
    dd_mark.arrow = dd_task_add(&desc);
}

void dd_arrow_kill(void)
{
    ((DdArrow *)dd_mark.arrow->data)->alive = 0;
    dd_mark.arrow = 0;
}

void dd_arrow_free(DdObj *o, DdArrow *a)
{
    dd_free(a);
}

#define RGB_A(rgb, a) ((rgb) << 8 >> 8 | (a) << 24)

void dd_arrow_draw(DdObj *o, DdArrow *a)
{
    FxVec3 up = { 0.0f, 1.0f, 0.0f };
    FxVec3 down = { 0.0f, -1.0f, 0.0f };
    void *list;
    FxVec3 p;
    FxMat4 m;
    FxMat4 t;
    FxVec4 at;
    unsigned int c;
    unsigned int c0;
    unsigned int c1;
    float f;
    int i;
    FxVtx *v;

    list = dl_select_layer(4, 0);
    vec3_scale(&up, &up, 500.0f);
    vec3_scale(&down, &down, 500.0f);
    list = dl_enable(list, 9);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_tex_func(list, 0, 1);
    list = dl_shade_model(list, 1);
    list = dl_blend_func(list, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dl_enable(list, 4);
    list = gim_set_texture(list, a->tex_ptcl);
    for (i = 0; i < 64; i++) {
        f = (float)i / 63.0f;
        c = RGB_A(a->color, a->alpha[i]);
        if (i < 60) {
            v = dl_alloc_inline(&list, sizeof(FxVtx) * 4);
            v[0].u = 1.0f; v[0].v = 0.0f; v[0].x = 10.0f; v[0].y = 10.0f; v[0].z = 0.0f; v[0].color = c;
            v[1].u = 0.0f; v[1].v = 0.0f; v[1].x = -10.0f; v[1].y = 10.0f; v[1].z = 0.0f; v[1].color = c;
            v[2].u = 1.0f; v[2].v = 1.0f; v[2].x = 10.0f; v[2].y = -10.0f; v[2].z = 0.0f; v[2].color = c;
            v[3].u = 0.0f; v[3].v = 1.0f; v[3].x = -10.0f; v[3].y = -10.0f; v[3].z = 0.0f; v[3].color = c;
            dd_hermite(f, &p, &a->from, &up, &a->to, &down);
            list = dd_billboard(list, &p, v);
        }
    }
    v = dl_alloc_inline(&list, sizeof(FxVtx) * 4);
    c0 = RGB_A(a->color, a->alpha[60]);
    c1 = RGB_A(a->color, a->alpha[63]);
    v[0].u = 1.0f; v[0].v = 0.0f; v[0].x = 17.5f; v[0].y = 17.5f; v[0].z = 0.0f; v[0].color = c0;
    v[1].u = 0.0f; v[1].v = 0.0f; v[1].x = -17.5f; v[1].y = 17.5f; v[1].z = 0.0f; v[1].color = c0;
    v[2].u = 1.0f; v[2].v = 1.0f; v[2].x = 17.5f; v[2].y = -17.5f; v[2].z = 0.0f; v[2].color = c1;
    v[3].u = 0.0f; v[3].v = 1.0f; v[3].x = -17.5f; v[3].y = -17.5f; v[3].z = 0.0f; v[3].color = c1;
    mat4_identity(&m);
    at.x = a->to.x;
    at.y = 17.5f + a->to.y;
    at.z = a->to.z;
    at.w = 1.0f;
    mat4_identity(&t);
    mat4_translate(&t, &t, &at);
    mat4_mul(&m, &m, &t);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&m);
    list = gim_set_texture(list, a->tex_head);
    list = draw_array(list, 4, 0x19F, 4, 0, v);
    mtx_pop();
    list = dl_enable(list, 1);
    dl_set_list(list);
}

/* The wave runs 3 particles a frame from the attacker; lit particles fade by 10 a frame and the
   object ends once all are dark after the arrow was released. */
void dd_arrow_update(DdObj *o, DdArrow *a)
{
    int all_dark = 1;
    int i;
    int v;
    int lit;

    if (a->head == 0 && a->done == 0 && dd_scene_flags(8, -1) == 0 && func_088062E4() == 0)
        se_play(0x6242);
    if (a->head >= 63) {
        a->head = 0;
    } else {
        a->head += 3;
        if (a->head >= 64)
            a->head = 63;
    }
    if (a->alive == 0 && a->head == 63)
        a->done = 1;
    for (i = 0; i < 64; i++) {
        v = a->alpha[i];
        lit = 0;
        if (a->done == 0 && i <= a->head && a->head - 3 < i) {
            v = 0xFF;
            lit = 1;
        }
        if (!lit) {
            v -= 10;
            if (v < 0)
                v = 0;
        }
        a->alpha[i] = v;
        all_dark &= v > 0 ? 0 : 1;
    }
    if (all_dark)
        dd_task_remove_cur();
}

/* Draws the 4 vertices `v` as a quad facing the camera at `pos`. */
void *dd_billboard(void *list, FxVec3 *pos, FxVtx *v)
{
    FxMat4 m;
    FxMat4 view;
    FxMat4 t;
    FxVec4 p = { 0.0f, 0.0f, 0.0f, 1.0f };

    p.x = pos->x;
    p.y = pos->y;
    p.z = pos->z;
    mat4_identity(&t);
    mat4_translate(&t, &t, &p);
    mtx_set_mode(1);
    mtx_get34(&view);
    mat4_inverse_ortho(&view, &view);
    view.m[15] = 1.0f;
    view.m[12] = 0.0f;
    view.m[13] = 0.0f;
    view.m[14] = 0.0f;
    mat4_mul(&m, &t, &view);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&m);
    list = draw_array(list, 4, 0x19F, 4, 0, v);
    mtx_pop();
    return list;
}

/* Cubic Hermite curve between p0 and p1 with tangents m0 and m1. */
void dd_hermite(float t, FxVec3 *out, FxVec3 *p0, FxVec3 *m0, FxVec3 *p1, FxVec3 *m1)
{
    float u = 1.0f - t;
    float t2 = 2.0f * t;
    FxVec3 a;
    FxVec3 b;
    FxVec3 c;
    FxVec3 d;
    FxVec3 r;

    vec3_scale(&a, p0, u * ((1.0f + t2) * u));
    vec3_scale(&b, m0, u * (t * u));
    vec3_scale(&c, m1, u * (-1.0f * t * t));
    vec3_scale(&d, p1, (3.0f - t2) * (t * t));
    vec3_add(&r, &a, &b);
    vec3_add(&r, &r, &c);
    vec3_add(&r, &r, &d);
    *out = r;
}
