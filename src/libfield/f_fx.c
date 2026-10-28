// @range 0x08826BB0-0x088274B4
// @rodata 0x08839458-0x088398E4
// @data 0x0883D630-0x0883D650
/* Emotion bubbles over the characters of the map (field_fx): a pool of 32 billboards, each following
   its character, with an animated icon (exclamation, sweat, notes...) or a sprite given by the caller. */
#include "libfield.h"

float rsqrtf(float x);
void mtx_load(Mat4 *m);
void vec4_scale(FieldVec4 *out, Vec4 *v, float s);
void mat4_transform_vec4(FieldVec4 *out, Mat4 *m, FieldVec4 *v);
float cosf_vfpu(float x);
FieldCamera *camera_get(void);
int fld_cell_special(int x, int z);

void *fld_fx_submit(void *list, Mat4 *m, FieldVtx *v);
void fld_fx_quad(FieldVtx *v, int tw, int th, int u, int vv, int w, int h, unsigned int color,
                 float x, float y);
FieldFx *fld_fx_alloc(void);

/* bubble frames in fx.gim (u, v) */
const unsigned short field_fx_bubble[6][2] = {
    { 168, 0 }, { 212, 0 }, { 168, 36 }, { 212, 36 }, { 168, 72 }, { 212, 72 },
};

const FieldFxKind field_fx_kinds[15] = {
    { 0, 0, 0, 0, 0 },     { 0, 0, 0, 11, -1 },   { 3, 72, 0, 11, -2 },  { 2, 0, 24, 14, -3 },
    { 2, 72, 24, 13, -4 }, { 3, 0, 48, 11, -2 },  { 3, 72, 48, 11, -2 }, { 1, 0, 72, 11, -1 },
    { 1, 72, 72, 11, -3 }, { 2, 0, 96, 14, -4 },  { 3, 72, 96, 11, -2 }, { 3, 120, 96, 11, -3 },
    { 3, 120, 72, 11, -3 }, { 3, 120, 48, 11, -2 }, { 3, 120, 24, 11, -2 },
};

/* icon cell and its duration in frames; -1 no icon, -2 loops */
const FieldFxAnim field_fx_anim[15] = {
    { { { 0, 0 } } },
    { { { -1, 15 }, { 0, 20 }, { 1, 20 }, { 2, 20 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 3 }, { 1, 3 }, { 2, 3 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
    { { { 0, 10 }, { 1, 10 }, { -2, 0 } } },
};

void fld_sprites_init(void)
{
    memset(field_fx, 0, sizeof(field_fx));
}

void fld_sprites_free(void)
{
    memset(field_fx, 0, sizeof(field_fx));
}

/* places the bubble over its character and advances the icon animation */
void fld_sprites_add(FieldFx *fx)
{
    Vec3 *pos;
    Mat4 m2;
    Mat4 m;

    if (fx != 0 && fx->active != 0) {
        pos = &fx->chr->pos;
        mat4_identity(&m);
        mat4_identity(&m2);
        m.m[12] = pos->x;
        m.m[13] = pos->y + 1.25f;
        m.m[14] = pos->z;
        fx->mtx = m;
        if (fx->kind != 0) {
            const FieldFxAnim *a = &field_fx_anim[fx->kind];

            if (fx->time >= a->f[fx->step].time) {
                fx->step++;
                if (field_fx_anim[fx->kind].f[fx->step].cell == -2)
                    fx->step = 0;
                fx->time = 0;
            } else {
                fx->time++;
            }
        }
    }
}

/* draws the bubble and its icon facing the camera */
void *fld_fx_draw(void *list, FieldFx *fx)
{
    const FieldFxFrame *step;
    unsigned int color;
    const FieldFxKind *kind;
    int shown;
    FieldVtx *v;
    const unsigned short *uv;

    FieldCamera *cam;
    FieldVec4 off;
    Mat4 t;
    Mat4 m;
    Mat4 rot;
    void *gim = fld_gim_get(4);

    if (fld_cell_special((int)(fx->chr->pos.x / 0.5f), (int)(fx->chr->pos.z / 0.5f)) != 0)
        return list;
    if (fx->def != 0)
        color = fx->def->color;
    else
        color = 0xFFFFFFFF;
    list = gim_set_texture_frame(list, gim, 0);
    if (fx->def != 0) {
        uv = field_fx_bubble[fx->def->bubble];
        step = &field_fx_anim[fx->kind].f[fx->step];
    } else {
        kind = &field_fx_kinds[fx->kind];
        uv = field_fx_bubble[kind->bubble];
        step = &field_fx_anim[fx->kind].f[fx->step];
    }
    v = dl_alloc_inline(&list, 0x60);
    fld_fx_quad(v, 256, 128, uv[0], uv[1], 44, 36, color, 0.0f, 0.5625f);
    list = fld_fx_submit(list, &fx->mtx, v);
    cam = camera_get();
    shown = 0;
    mat4_identity(&t);
    vec4_scale(&off, &cam->dir, 0.1f);
    off.w = 1.0f;
    mat4_translate(&t, &t, (Vec4 *)&off);
    mat4_mul(&m, &fx->mtx, &t);
    v = dl_alloc_inline(&list, 0x60);
    {
        float origin[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        off = *(FieldVec4 *)origin;
    }
    if (fx->def != 0) {
        if (fx->def->gim != 0) {
            fld_fx_quad(v, fx->def->tw, fx->def->th, fx->def->u, fx->def->v, fx->def->w, fx->def->h,
                        color, 0.0f, 0.5625f);
            list = gim_set_texture_frame(list, fx->def->gim, 0);
            shown = 1;
            off.x = fx->def->x / 64.0f;
            off.y = fx->def->y / 64.0f;
        }
    } else if (step->cell != -1) {
        fld_fx_quad(v, 256, 128, kind->u + step->cell * 24, kind->v, 24, 24, color, 0.0f, 0.5625f);
        shown = 1;
        off.x = kind->x / 64.0f;
        off.y = kind->y / 64.0f;
    }
    if (shown) {
        rot = *(Mat4 *)cam->rot;
        rot.m[12] = 0.0f;
        rot.m[13] = 0.0f;
        rot.m[14] = 0.0f;
        mat4_transform_vec4(&off, &rot, &off);
        off.x -= off.z * cosf_vfpu(0.2617994f);
        off.y -= off.z * cosf_vfpu(0.2617994f);
        off.z = 0.0f;
        mat4_identity(&t);
        mat4_translate(&t, &t, (Vec4 *)&off);
        mat4_mul(&m, &m, &t);
        list = fld_fx_submit(list, &m, v);
    }
    return list;
}

FieldFx *fld_fx_attach(FieldChr *c, int kind, FieldFxDef *def)
{
    FieldFx *fx;

    if (kind < 0 || kind >= 15)
        return 0;
    if (def == 0 && kind == 0)
        return 0;
    if ((FieldFx *)c->fx != 0)
        fx = (FieldFx *)c->fx;
    else
        fx = fld_fx_alloc();
    fx->active = 1;
    fx->chr = c;
    fx->kind = kind;
    fx->def = def;
    return fx;
}

void fld_fx_free(FieldFx *fx)
{
    memset(fx, 0, sizeof(FieldFx));
}

int fld_fx_kind(FieldFx *fx)
{
    return fx->kind;
}

/* draws a quad of 4 vertices with the matrix m moved a little in front of the character */
void *fld_fx_submit(void *list, Mat4 *m, FieldVtx *v)
{
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    {
        Vec4 off = { 0.21f, 0.0f, 0.21f, 0.0f };
        Mat4 r;
        Mat4 t;

        mat4_identity(&t);
        mat4_translate(&t, &t, (Vec4 *)&off);
        mat4_mul(&r, &t, m);
        mtx_load(&r);
    }
    list = draw_array(list, 4, 0x19F, 4, 0, v);
    mtx_pop();
    return list;
}

/* the 4 vertices (strip) of a w x h cell at (u, vv) of a tw x th texture, with its bottom left at (x, y),
   leaning back 45 degrees */
void fld_fx_quad(FieldVtx *v, int tw, int th, int u, int vv, int w, int h, unsigned int color,
                 float x, float y)
{
    float sw = (float)w / 64.0f;
    float dy = sw * rsqrtf(2.0f);
    float y1 = y - (float)h / 64.0f * 1.2f;
    float x1 = x + sw * sqrtf_vfpu(2.0f);

    v[0].u = (float)(u + w) / (float)tw;
    v[0].v = (float)vv / (float)th;
    v[0].x = x1;
    v[0].y = y + dy;
    v[1].u = (float)u / (float)tw;
    v[1].v = (float)vv / (float)th;
    v[1].x = x;
    v[1].y = y;
    v[2].u = (float)(u + w) / (float)tw;
    v[2].v = (float)(vv + h) / (float)th;
    v[2].x = x1;
    v[2].y = y1 + dy;
    v[3].u = (float)u / (float)tw;
    v[3].v = (float)(vv + h) / (float)th;
    v[3].x = x;
    v[3].y = y1;
    v[0].color = v[1].color = v[2].color = v[3].color = color;
    v[0].z = v[1].z = v[2].z = v[3].z = 0.0f;
}

FieldFx *fld_fx_alloc(void)
{
    FieldFx *fx = field_fx;
    int i;

    for (i = 0; i < 32; i++, fx++) {
        if (fx->active == 0)
            return fx;
    }
    return 0;
}
