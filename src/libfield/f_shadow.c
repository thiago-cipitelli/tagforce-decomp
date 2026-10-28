// @range 0x0881C240-0x0881CE68
// @asm 0x0881CC4C-0x0881CD3C
// @data 0x0883C9E0-0x0883D3C0
/* Draw list of the field (field_draw): the characters and items queued during the frame, drawn
   sorted by depth after their stencil shadows (a 12-sided cylinder per character, lit by
   field_shadow), and the attribute of each cell of the room (who stands on it). */
#include "libfield.h"

void vec3_normalize2(Vec4 *out, Vec4 *v);
void vec3_cross(Vec4 *out, Vec4 *a, Vec4 *b);
void mtx_translate(Vec4 *v);
void mtx_scale3(Vec4 *s);
void mtx_mult(Vec4 *m);
void mtx_get34(Mat4 *m);
void *mtx_commit(void *list);
void mat4_transform_vec4(Vec4 *out, Mat4 *m, Vec4 *v);
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a);
Mat4 *camera_get(void);
void *dl_clear_depth(void *list, int z);
void *dl_clear(void *list, int flags);
void *dl_front_face(void *list, int cw);
void *dl_color(void *list, unsigned int color);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *dl_tex_func(void *list, int func, int comp);
void *dl_tex_offset(void *list, float u, float v);
void *dl_tex_scale(void *list, float u, float v);
void *dl_tex_mode(void *list, int mode, int a, int b, int c);
void *dl_tex_wrap(void *list, int u, int v);
void *dl_set_tex_filter(void *list, int min, int mag);
void *lighting_disable(void *list);
void *fcam_apply(void *list);
int fld_bg_area_is(int cx, int cz);
PadState *pad_get(void);

/* the light of the shadows: a frame around the direction dir */
void fsh_light_set(Vec4 *dir)
{
    Vec4 up = { 1.0f, 0.0f, 0.0f, 0.0f };
    Vec4 d;
    Vec4 side;

    vec3_normalize2(&d, dir);
    vec3_cross(&side, &up, &d);
    vec3_cross(&up, &d, &side);
    vec3_normalize2(&field_shadow.light[0], &up);
    field_shadow.light[1] = *(Vec4 *)&d; /* the cast keeps the original registers */
    vec3_normalize2(&field_shadow.light[2], &side);
    {
        Vec4 origin = { 0.0f, 0.0f, 0.0f, 1.0f };
        Vec4 *o = &origin; /* copied through the stack as in the original */

        field_shadow.light[3] = *o;
    }
}

void fsh_set_alpha(int alpha)
{
    field_shadow.alpha = alpha;
}

/* stencil pass: only the stencil is written */
void *fsh_begin(void *list)
{
    list = dl_clear_depth(list, 0);
    list = dl_clear(list, 2);
    list = dl_disable(list, 9);
    list = dl_depth_mask(list, 1);
    list = dl_enable(list, 3);
    list = dl_stencil_op(list, 1, 0, 0xFF);
    list = dl_enable(list, 5);
    return dl_pixel_mask(list, 0xFFFFFF);
}

FieldShVtx fsh_cyl_caps[48];
FieldShVtx fsh_cyl_side[72];

/* the shadow volume of a character at pos: the cylinder drawn twice (front faces, back faces) */
void *fsh_volume(float scale, void *list, Vec4 *pos)
{
    Vec4 lift = { 0.0f, 0.5f, 0.0f, 0.0f };
    Vec4 shrink = { 0.25f, 0.25f, 0.25f, 0.0f };
    Vec4 size = { 1.0f, 2.0f, 1.0f, 0.0f };
    Vec4 up = { 0.0f, 0.8f, 0.0f, 0.0f };
    Vec4 width = { scale, 1.0f, scale, 0.0f };

    mtx_push();
    mtx_identity();
    mtx_translate(pos);
    mtx_translate(&up);
    mtx_mult(field_shadow.light);
    mtx_scale3(&size);
    mtx_translate(&lift);
    mtx_scale3(&shrink);
    mtx_scale3(&width);
    list = dl_stencil_func(list, 0, 0, 4);
    list = dl_front_face(list, 1);
    list = draw_array(list, 4, 0x143, 0x30, 0, fsh_cyl_caps);
    list = draw_array(list, 3, 0x143, 0x48, 0, fsh_cyl_side);
    list = dl_stencil_func(list, 0, 0, 5);
    list = dl_front_face(list, 0);
    list = draw_array(list, 4, 0x143, 0x30, 0, fsh_cyl_caps);
    list = draw_array(list, 3, 0x143, 0x48, 0, fsh_cyl_side);
    mtx_pop();
    return list;
}

FieldShVtx fsh_cyl_caps[48] = {
    { 0.0f, 0.4774648f, 0, 32767, 0, 28377, 32767, -16382 },
    { 0.083333336f, 0.4774648f, 0, 32767, 0, 16383, 32767, -28376 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.16666667f, 0.4774648f, 0, 32767, 0, 0, 32767, -32766 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.25f, 0.4774648f, 0, 32767, 0, -16382, 32767, -28376 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.33333334f, 0.4774648f, 0, 32767, 0, -28376, 32767, -16383 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.4166667f, 0.4774648f, 0, 32767, 0, -32766, 32767, 0 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.5f, 0.4774648f, 0, 32767, 0, -28376, 32767, 16383 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.5833333f, 0.4774648f, 0, 32767, 0, -16383, 32767, 28377 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.6666666f, 0.4774648f, 0, 32767, 0, 0, 32767, 32767 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.74999994f, 0.4774648f, 0, 32767, 0, 16383, 32767, 28377 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.83333325f, 0.4774648f, 0, 32767, 0, 28377, 32767, 16383 },
    { 0.5f, 0.63661975f, 0, 32767, 0, 0, 32767, 0 },
    { 0.91666657f, 0.4774648f, 0, 32767, 0, 32767, 32767, 0 },
    { 0.9999999f, 0.4774648f, 0, 32767, 0, 28377, 32767, -16382 },
    { 0.9999999f, 0.4774648f, 0, 32767, 0, 28377, 32767, -16382 },
    { 0.0f, 0.15915494f, 0, -32766, 0, 28377, -32766, -16382 },
    { 0.0f, 0.15915494f, 0, -32766, 0, 28377, -32766, -16382 },
    { 0.083333336f, 0.15915494f, 0, -32766, 0, 16383, -32766, -28376 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.16666667f, 0.15915494f, 0, -32766, 0, 0, -32766, -32766 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.25f, 0.15915494f, 0, -32766, 0, -16382, -32766, -28376 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.33333334f, 0.15915494f, 0, -32766, 0, -28376, -32766, -16383 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.4166667f, 0.15915494f, 0, -32766, 0, -32766, -32766, 0 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.5f, 0.15915494f, 0, -32766, 0, -28376, -32766, 16383 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.5833333f, 0.15915494f, 0, -32766, 0, -16383, -32766, 28377 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.6666666f, 0.15915494f, 0, -32766, 0, 0, -32766, 32767 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.74999994f, 0.15915494f, 0, -32766, 0, 16383, -32766, 28377 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.83333325f, 0.15915494f, 0, -32766, 0, 28377, -32766, 16383 },
    { 0.5f, 0.0f, 0, -32766, 0, 0, -32766, 0 },
    { 0.91666657f, 0.15915494f, 0, -32766, 0, 32767, -32766, 0 },
    { 0.9999999f, 0.15915494f, 0, -32766, 0, 28377, -32766, -16382 },
};

FieldShVtx fsh_cyl_side[72] = {
    { 0.083333336f, 0.15915494f, 8481, 0, -31649, 16383, -32766, -28376 },
    { 0.16666667f, 0.15915494f, 8481, 0, -31649, 0, -32766, -32766 },
    { 0.083333336f, 0.4774648f, 8481, 0, -31649, 16383, 32767, -28376 },
    { 0.16666667f, 0.15915494f, 8481, 0, -31649, 0, -32766, -32766 },
    { 0.16666667f, 0.4774648f, 8481, 0, -31649, 0, 32767, -32766 },
    { 0.083333336f, 0.4774648f, 8481, 0, -31649, 16383, 32767, -28376 },
    { 0.16666667f, 0.15915494f, -8480, 0, -31649, 0, -32766, -32766 },
    { 0.25f, 0.15915494f, -8480, 0, -31649, -16382, -32766, -28376 },
    { 0.16666667f, 0.4774648f, -8480, 0, -31649, 0, 32767, -32766 },
    { 0.25f, 0.15915494f, -8480, 0, -31649, -16382, -32766, -28376 },
    { 0.25f, 0.4774648f, -8480, 0, -31649, -16382, 32767, -28376 },
    { 0.16666667f, 0.4774648f, -8480, 0, -31649, 0, 32767, -32766 },
    { 0.25f, 0.15915494f, -23169, 0, -23169, -16382, -32766, -28376 },
    { 0.33333334f, 0.15915494f, -23169, 0, -23169, -28376, -32766, -16383 },
    { 0.25f, 0.4774648f, -23169, 0, -23169, -16382, 32767, -28376 },
    { 0.33333334f, 0.15915494f, -23169, 0, -23169, -28376, -32766, -16383 },
    { 0.33333334f, 0.4774648f, -23169, 0, -23169, -28376, 32767, -16383 },
    { 0.25f, 0.4774648f, -23169, 0, -23169, -16382, 32767, -28376 },
    { 0.33333334f, 0.15915494f, -31649, 0, -8480, -28376, -32766, -16383 },
    { 0.4166667f, 0.15915494f, -31649, 0, -8480, -32766, -32766, 0 },
    { 0.33333334f, 0.4774648f, -31649, 0, -8480, -28376, 32767, -16383 },
    { 0.4166667f, 0.15915494f, -31649, 0, -8480, -32766, -32766, 0 },
    { 0.4166667f, 0.4774648f, -31649, 0, -8480, -32766, 32767, 0 },
    { 0.33333334f, 0.4774648f, -31649, 0, -8480, -28376, 32767, -16383 },
    { 0.4166667f, 0.15915494f, -31649, 0, 8481, -32766, -32766, 0 },
    { 0.5f, 0.15915494f, -31649, 0, 8481, -28376, -32766, 16383 },
    { 0.4166667f, 0.4774648f, -31649, 0, 8481, -32766, 32767, 0 },
    { 0.5f, 0.15915494f, -31649, 0, 8481, -28376, -32766, 16383 },
    { 0.5f, 0.4774648f, -31649, 0, 8481, -28376, 32767, 16383 },
    { 0.4166667f, 0.4774648f, -31649, 0, 8481, -32766, 32767, 0 },
    { 0.5f, 0.15915494f, -23169, 0, 23170, -28376, -32766, 16383 },
    { 0.5833333f, 0.15915494f, -23169, 0, 23170, -16383, -32766, 28377 },
    { 0.5f, 0.4774648f, -23169, 0, 23170, -28376, 32767, 16383 },
    { 0.5833333f, 0.15915494f, -23169, 0, 23170, -16383, -32766, 28377 },
    { 0.5833333f, 0.4774648f, -23169, 0, 23170, -16383, 32767, 28377 },
    { 0.5f, 0.4774648f, -23169, 0, 23170, -28376, 32767, 16383 },
    { 0.5833333f, 0.15915494f, -8480, 0, 31650, -16383, -32766, 28377 },
    { 0.6666666f, 0.15915494f, -8480, 0, 31650, 0, -32766, 32767 },
    { 0.5833333f, 0.4774648f, -8480, 0, 31650, -16383, 32767, 28377 },
    { 0.6666666f, 0.15915494f, -8480, 0, 31650, 0, -32766, 32767 },
    { 0.6666666f, 0.4774648f, -8480, 0, 31650, 0, 32767, 32767 },
    { 0.5833333f, 0.4774648f, -8480, 0, 31650, -16383, 32767, 28377 },
    { 0.6666666f, 0.15915494f, 8481, 0, 31650, 0, -32766, 32767 },
    { 0.74999994f, 0.15915494f, 8481, 0, 31650, 16383, -32766, 28377 },
    { 0.6666666f, 0.4774648f, 8481, 0, 31650, 0, 32767, 32767 },
    { 0.74999994f, 0.15915494f, 8481, 0, 31650, 16383, -32766, 28377 },
    { 0.74999994f, 0.4774648f, 8481, 0, 31650, 16383, 32767, 28377 },
    { 0.6666666f, 0.4774648f, 8481, 0, 31650, 0, 32767, 32767 },
    { 0.74999994f, 0.15915494f, 23170, 0, 23170, 16383, -32766, 28377 },
    { 0.83333325f, 0.15915494f, 23170, 0, 23170, 28377, -32766, 16383 },
    { 0.74999994f, 0.4774648f, 23170, 0, 23170, 16383, 32767, 28377 },
    { 0.83333325f, 0.15915494f, 23170, 0, 23170, 28377, -32766, 16383 },
    { 0.83333325f, 0.4774648f, 23170, 0, 23170, 28377, 32767, 16383 },
    { 0.74999994f, 0.4774648f, 23170, 0, 23170, 16383, 32767, 28377 },
    { 0.83333325f, 0.15915494f, 31650, 0, 8481, 28377, -32766, 16383 },
    { 0.91666657f, 0.15915494f, 31650, 0, 8481, 32767, -32766, 0 },
    { 0.83333325f, 0.4774648f, 31650, 0, 8481, 28377, 32767, 16383 },
    { 0.91666657f, 0.15915494f, 31650, 0, 8481, 32767, -32766, 0 },
    { 0.91666657f, 0.4774648f, 31650, 0, 8481, 32767, 32767, 0 },
    { 0.83333325f, 0.4774648f, 31650, 0, 8481, 28377, 32767, 16383 },
    { 0.91666657f, 0.15915494f, 31650, 0, -8480, 32767, -32766, 0 },
    { 0.9999999f, 0.15915494f, 31650, 0, -8480, 28377, -32766, -16382 },
    { 0.91666657f, 0.4774648f, 31650, 0, -8480, 32767, 32767, 0 },
    { 0.9999999f, 0.15915494f, 31650, 0, -8480, 28377, -32766, -16382 },
    { 0.9999999f, 0.4774648f, 31650, 0, -8480, 28377, 32767, -16382 },
    { 0.91666657f, 0.4774648f, 31650, 0, -8480, 32767, 32767, 0 },
    { 0.0f, 0.15915494f, 23170, 0, -23169, 28377, -32766, -16382 },
    { 0.083333336f, 0.15915494f, 23170, 0, -23169, 16383, -32766, -28376 },
    { 0.0f, 0.4774648f, 23170, 0, -23169, 28377, 32767, -16382 },
    { 0.083333336f, 0.15915494f, 23170, 0, -23169, 16383, -32766, -28376 },
    { 0.083333336f, 0.4774648f, 23170, 0, -23169, 16383, 32767, -28376 },
    { 0.0f, 0.4774648f, 23170, 0, -23169, 28377, 32767, -16382 },
};

/* darken the pixels whose stencil was left odd by the volumes */
void *fsh_end(void *list)
{
    list = dl_enable(list, 9);
    list = dl_front_face(list, 1);
    list = dl_depth_mask(list, 0);
    list = dl_disable(list, 3);
    list = dl_stencil_func(list, 0, 0, 0);
    list = dl_pixel_mask(list, 0);
    list = dl_enable(list, 3);
    list = dl_disable(list, 9);
    list = dl_disable(list, 1);
    list = dl_stencil_op(list, 4, 0, 0xFF);
    list = dl_stencil_func(list, 0, 0, 0);
    list = dl_color(list, 0x80000000);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = draw_set_offsets(list, 1, 1, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    list = dl_disable(list, 3);
    list = dl_enable(list, 9);
    list = dl_enable(list, 1);
    return dl_color(list, 0xFFFFFFFF);
}

void fld_draw_clear(void)
{
    memset(&field_draw, 0, sizeof(field_draw));
}

void fld_cells_free(void);

void fld_draw_term(void)
{
    fld_cells_free();
    fld_draw_clear();
}

void fld_cells_free(void)
{
    FieldDraw *d = &field_draw;

    if (field_draw.cells != 0) {
        fld_free(d->cells);
        d->cells = 0;
    }
}

/* the shadow matrix of something standing at pos (pos.y set to the floor); allocates the cell
   attributes of the room on first use */
void fld_shadow_set(Vec3 *pos, void *shadow)
{
    FieldDraw *d = &field_draw;
    FieldMap *map = fld_map_get();
    Vec3 cam;
    Mat4 m __attribute__((aligned(16)));
    Mat4 rot __attribute__((aligned(16)));

    if (field_draw.cells == 0 && map != 0) {
        int size = map->n * 4;

        d->cells = fld_alloc(size);
        memset(d->cells, 0, size);
        d->w = map->w;
        d->h = map->h;
    }
    fld_map_height(pos->x, pos->z, pos);
    vec3_scale(0.3f, &cam, (Vec3 *)&camera_get()->m[12]);
    mat4_identity(&rot);
    mat4_mul_rot_y(&rot, &rot, 0.7853982f);
    mat4_identity(&m);
    m.m[12] = pos->x;
    m.m[13] = pos->y;
    m.m[14] = pos->z;
    mat4_mul(shadow, &m, &rot);
}

int fld_draw_cmp(const void *a, const void *b);

/* draw the frame: shadow volumes, then the queued entries sorted by depth (depth only, then
   color), the ones flagged arg8c last */
void fld_draw_flush(void)
{
    FieldDraw *d = &field_draw;
    void *list = dl_select_layer(1, 0);
    int i;

    mtx_identity();
    list = fcam_apply(list);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_enable(list, 0);
    if (pad_get()->b.down & 0x200)
        list = dl_disable(list, 9);
    else
        list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(list, 1.0f, 1.0f);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_wrap(list, 0, 0);
    list = dl_set_tex_filter(list, 0, 0);
    list = lighting_disable(list);
    {
        Vec4 dir = { 0.0f, -1.0f, 0.0f, 0.0f };

        fsh_light_set(&dir);
    }
    fsh_set_alpha(100);
    list = fsh_begin(list);
    for (i = 0; i < d->n; i++) {
        if (d->ent[i].volume)
            list = fsh_volume(d->ent[i].scale, list, &((Vec4 *)d->ent[i].mtx.m)[3]);
    }
    list = fsh_end(list);
    list = dl_depth_mask(list, 1);
    qsort(d, d->n, sizeof(FieldDrawEnt), fld_draw_cmp);
    for (i = 0; i < d->n; i++) {
        if (d->ent[i].arg->arg8c == 0)
            list = d->ent[i].fn(list, d->ent[i].arg);
    }
    list = dl_depth_mask(list, 0);
    list = dl_pixel_mask(list, 0xFFFFFFFF);
    for (i = 0; i < d->n; i++) {
        if (d->ent[i].arg->arg8c == 0)
            list = d->ent[i].fn(list, d->ent[i].arg);
    }
    list = dl_pixel_mask(list, 0);
    for (i = 0; i < d->n; i++) {
        if (d->ent[i].arg->arg8c != 0)
            list = d->ent[i].fn(list, d->ent[i].arg);
    }
    memset(d, 0, 0x28B0);
    mtx_set_mode(0);
    list = mtx_commit(list);
    mtx_set_mode(1);
    list = mtx_commit(list);
    mtx_set_mode(2);
    list = mtx_commit(list);
    dl_set_list(list);
}

#ifdef NON_MATCHING /* the original divides x before loading z (every form tried loads both first); its prologue copies scale after volume, as if scale were the third parameter */
int fld_draw_add(float scale, void *shadow, int volume, int area, void *fn, void *arg)
{
    FieldDraw *d = &field_draw;
    FieldDrawEnt *e;

    if (area != 0 && fld_bg_area_is(((Mat4 *)shadow)->m[12] / 0.5f, ((Mat4 *)shadow)->m[14] / 0.5f) == 0)
        return 0;
    if (d->n < 0x82) {
        e = &d->ent[d->n];
        memcpy(e, shadow, 0x40);
        e->volume = volume;
        e->scale = scale;
        e->fn = fn;
        e->arg = arg;
        d->n++;
        return 1;
    }
    return 0;
}
#endif

/* qsort: the farthest from the camera first */
int fld_draw_cmp(const void *a, const void *b)
{
    Mat4 view;
    Vec4 pa;
    Vec4 pb;

    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(2);
    mat4_transform_vec4(&pa, &view, &((Vec4 *)((FieldDrawEnt *)a)->mtx.m)[3]);
    mat4_transform_vec4(&pb, &view, &((Vec4 *)((FieldDrawEnt *)b)->mtx.m)[3]);
    if (pa.z > pb.z)
        return 1;
    return -1;
}

int fld_map_attr(int x, int z)
{
    FieldDraw *d = &field_draw;
    int i = x + z * field_draw.w;

    if (field_draw.cells != 0)
        return d->cells[i];
    return 0;
}

int fld_cell_free(int x, int z)
{
    return fld_map_attr(x, z) == 0;
}

void fld_cell_mark(int x, int z, int v)
{
    FieldDraw *d = &field_draw;
    int i = x + z * field_draw.w;

    if (field_draw.cells != 0)
        d->cells[i] = v;
}
