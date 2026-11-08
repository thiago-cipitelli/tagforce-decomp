// @range 0x08806FC4-0x08807844
/* The room (bg_NN_NN): its models, door models and marker cells, the area the player stands in
   (with a 30 frame crossfade when it changes) and the cell map used by the collision. */
#include "libfield.h"

void fld_bg_free(void);
void fld_bg_model_free(int id);                /* 0x08809A08 */
void fld_bg_model_update(int id);              /* 0x088099F8 */
FieldChr *fld_player_get(void);                /* 0x0880B170 */
void fld_pos_to_cell(float x, float z, int *cx, int *cz); /* 0x08807F24 */
int fld_area_at(int cx, int cz);               /* 0x08808804 */
void fld_bg_doors_update(void);                  /* 0x08808874 */
void fld_load_bg(int id, void (*cb)(), int arg); /* 0x0880C56C */
void fld_bg_loaded();                          /* 0x08808214 */
void *fld_story_get(void);                     /* 0x08811130 */
int fld_bg_index(int id);                      /* 0x0880EF2C */
int fld_time_index(int t);                     /* 0x0880CF48 */
void fld_light_set(int light);                 /* 0x08818884 */
void fld_load_cancel_bg(void);                 /* 0x0880C6B8 */

int fld_bg_init(void)
{
    memset(&field_bg_state, 0, sizeof(FieldBg));
    return 1;
}

void fld_bg_term(void)
{
    fld_bg_free();
    fld_bg_init();
}

void fld_bg_free(void)
{
    FieldBg *b = &field_bg_state;
    int i;

    fld_bg_model_free(field_bg_state.id);
    if (field_bg_state.cells) {
        fld_free(b->cells);
        b->cells = 0;
    }
    for (i = 0; i < b->n_models; i++) {
        model_free(b->model[i]);
        b->model[i] = 0;
        if (b->model2[i]) {
            model_free(b->model2[i]);
            b->model2[i] = 0;
        }
    }
    for (i = 0; i < b->n_doors; i++) {
        model_free(b->door[i]);
        b->door[i] = 0;
    }
    b->map = 0;
}

void fld_bg_update(void)
{
    FieldBg *b = &field_bg_state;
    int i;
    FieldChr *p;
    int cx;
    int cz;
    int a;

    for (i = 0; i < b->n_models; i++) {
        if (b->model[i])
            model_update(b->model[i], 0);
    }
    p = fld_player_get();
    if (p) {
        fld_pos_to_cell(p->pos.x, p->pos.z, &cx, &cz);
        a = fld_area_at(cx, cz);
        if (b->area != a) {
            b->prev_area = b->area;
            b->fade_t = 0;
            b->fade = 1;
            b->area = a;
        }
    }
    fld_bg_doors_update();
    if (b->fade == 1) {
        if (b->fade_t < 30)
            b->fade_t++;
        else
            b->fade = 0;
    }
    fld_bg_model_update(b->id);
}

void *dl_clear_depth(void *list, int z);
void *dl_clear(void *list, int flags);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_set_tex_filter(void *list, int min, int mag);
void mtx_translate(Vec4 *v);
void *model_hook(void *m, int which, void *v);
void *fcam_apply(void *list);
void *fmrk_begin(void *list);
void *fmrk_volume(void *list, Vec4 *pos);
void *fmrk_fill(void *list);
void fld_bg_model_draw(int id);
void fld_bg_draw_objs(void);
void fld_bg_draw_models(void);
void fld_bg_draw_shadow(void);
void *fld_bg_draw_sky(void *list);
void fld_bg_draw_to_tex(void);
void fld_bg_draw_from_tex(void);
int fld_bg_stencil_hook(void **list, void *m, int a, int b, int phase);
void fld_pos_set(Vec3 *p, int cx, int cz);
int fld_bg_id(void);

typedef struct FieldVec3w { Vec3 v; int w; } __attribute__((aligned(16))) FieldVec3w;  /* the w of the mark is copied as a word (lw/sw); aligned like Vec4 */
extern Vec3 field_mark_pos;                    /* 0x0883A4A0 the mark of room 3 */
extern Vec4 field_mark_w[2];                   /* 0x0883A4B0 w of the mark volumes */

/* draws the room: the render state of layers 1 and 2, the objects, area models, shadow and sky
   under the light offset, and during an area change (fade 1) the previous area through the stencil
   texture; then the marks of the marked cells (room 3: one fixed spot) */
void fld_bg_draw(void)
{
    FieldBg *b = &field_bg_state;
    Vec3 l = field_bg_state.light;
    void *list;
    void *hook;
    int i;
    ModelDrawOpts o;
    Vec3 p;
    Vec3 q;
    Vec4 v0;
    Vec4 v1;
    FieldVec3w t0;
    FieldVec3w t1;

    if (field_bg_state.file != 0 && *b->file != 0) {
        list = dl_select_layer(1, 0);
        list = dl_clear_depth(list, 0);
        list = dl_clear(list, 2);
        list = dl_enable(list, 3);
        list = dl_enable(list, 1);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_enable(list, 4);
        list = dl_alpha_test(list, 6, 0, 0xFF);
        list = dl_enable(list, 0);
        list = dl_set_tex_filter(list, 1, 1);
        list = dl_stencil_op(list, 1, 2, 0xFF);
        list = dl_stencil_func(list, 0, 0, 2);
        dl_set_list(list);
        list = dl_select_layer(2, 0);
        list = dl_enable(list, 1);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_enable(list, 4);
        list = dl_alpha_test(list, 6, 0, 0xFF);
        list = dl_enable(list, 0);
        list = dl_stencil_op(list, 1, 0, 0xFF);
        dl_set_list(list);
        memset(&o, 0, sizeof(o));
        o.a = 1;
        o.b = 2;
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        vec3_scale(&l, &l, -1.0f);
        mtx_translate((Vec4 *)&l);
        switch (b->fade) {
        case 0:
            fld_bg_draw_objs();
            fld_bg_draw_models();
            fld_bg_draw_shadow();
            list = dl_select_layer(1, 0);
            list = fld_bg_draw_sky(list);
            dl_set_list(list);
            break;
        case 1:
            fld_bg_draw_objs();
            fld_bg_draw_models();
            fld_bg_draw_shadow();
            list = dl_select_layer(1, 0);
            list = fld_bg_draw_sky(list);
            dl_set_list(list);
            fld_bg_draw_to_tex();
            o.a = 1;
            o.b = 1;
            hook = model_hook(b->model[b->prev_area], 1, fld_bg_stencil_hook);
            model_draw(b->model[b->prev_area], &o);
            model_hook(b->model[b->prev_area], 1, hook);
            fld_bg_draw_from_tex();
            break;
        }
        mtx_set_mode(2);
        mtx_pop();
        list = dl_select_layer(1, 0);
        list = fcam_apply(list);
        list = fmrk_begin(list);
        if (fld_bg_id() == 3) {
            p = field_mark_pos;
            fld_map_height(&p, p.x, p.z);
            t0.v = p;
            t0.w = *(int *)&field_mark_w[0].w;
            v0 = *(Vec4 *)&t0;
            list = fmrk_volume(list, &v0);
        } else {
            for (i = 0; i < b->n_cells; i++) {
                fld_pos_set(&q, b->cells[i].x, b->cells[i].z);
                t1.v = q;
                t1.w = *(int *)&field_mark_w[1].w;
                v1 = *(Vec4 *)&t1;
                list = fmrk_volume(list, &v1);
            }
        }
        list = fmrk_fill(list);
        dl_set_list(list);
        fld_bg_model_draw(b->id);
    }
}

int fld_bg_area_is(int cx, int cz)
{
    FieldBg *b = &field_bg_state;
    int a = fld_area_at(cx, cz);
    int r;

    if (b->area != -1) {
        if (a == b->area)
            r = 1;
        else
            r = 0;
    } else {
        r = 1;
    }
    return r;
}

void fld_bg_cmd_load(FieldBgCmd *c)
{
    FieldBg *b = &field_bg_state;
    unsigned int size;
    int *st;
    int bg;
    int t;

    field_bg_state.loading = 1;
    field_bg_state.id = c->id;
    if (c->n_cells) {
        size = c->n_cells << 2;
        b->cells = fld_alloc(size);
        memcpy(b->cells, c->cells, size);
    }
    b->n_cells = c->n_cells;
    fld_load_bg(c->id, fld_bg_loaded, 0);
    st = fld_story_get();
    bg = fld_bg_index(b->id);
    t = fld_time_index(*(int *)st[3]);
    fld_light_set(field_bg_lights[bg][t].light);
}

int fld_bg_load_done(void)
{
    return !field_bg_state.loading;
}

void fld_bg_cmd_unload(void)
{
    field_bg_state.unloading = 1;
    fld_load_cancel_bg();
    field_bg_state.unloading = 0;
}

int fld_bg_unload_done(void)
{
    return !field_bg_state.unloading;
}

FieldMap *fld_map_get(void)
{
    return field_bg_state.map;
}

unsigned char *fld_map_cells(void)
{
    FieldMap *m = fld_map_get();

    if (m != 0)
        return (unsigned char *)m + ((int *)m)[3];
    return 0;
}

unsigned char *fld_map_attrs(void)
{
    FieldMap *m = fld_map_get();

    if (m != 0)
        return (unsigned char *)m + ((int *)m)[4];
    return 0;
}
