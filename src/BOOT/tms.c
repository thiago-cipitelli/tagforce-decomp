// @range 0x0882B65C-0x0882C428
// @asm 0x0882B914-0x0882BA88
// @asm 0x0882C300-0x0882C414
// @data 0x088ED7B0-0x088ED7D0
/* TGMS model geometry: relocation of the header, materials with four texture layers (texture
   from the TMS set or a hook, lighting, alpha test, depth, blend, masks, environment-map callbacks),
   meshes drawn as prims with up to eight bone matrices cached between prims, culling by the
   bounding box against the frustum. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct TmsTex {                /* 0x28 */
    char *name;
    void *gim;                         /* +0x04 */
    short pad8;
    signed char func;                  /* +0x0A */
    signed char rgba;                  /* +0x0B */
    unsigned int env;                  /* +0x0C */
    signed char wrap_u;                /* +0x10 */
    signed char wrap_v;                /* +0x11 */
    short pad12;
    float off_u;                       /* +0x14 */
    float off_v;                       /* +0x18 */
    float scale_u;                     /* +0x1C */
    float scale_v;                     /* +0x20 */
    int alt;                           /* +0x24 index in the hook's set, -1 none */
} TmsTex;

typedef unsigned int *(*TmsLayerFn)(unsigned int *list, int after);

typedef struct TmsLayer {              /* 0x20 */
    unsigned int flags;                /* 1 on, 2 lit, 4 alpha test, 8 blend, 0x10 depth write, 0x20 pixel mask */
    TmsTex *tex;                       /* +0x04 */
    TmsLayerFn fn;                     /* +0x08 */
    unsigned int color;                /* +0x0C */
    signed char atest_func;            /* +0x10 */
    unsigned char atest_ref;           /* +0x11 */
    unsigned char atest_mask;          /* +0x12 */
    signed char depth_func;            /* +0x13 */
    signed char blend_op;              /* +0x14 */
    signed char blend_src;             /* +0x15 */
    signed char blend_dst;             /* +0x16 */
    signed char pad17;
    unsigned int fixa;                 /* +0x18 */
    unsigned int fixb;                 /* +0x1C */
} TmsLayer;

typedef struct TmsMat {                /* 0xA4 */
    unsigned int flags;                /* 1 opaque pass, 2 no cull, 4 replaceable, 8 maskable */
    unsigned int ambient;              /* +0x04 */
    unsigned int diffuse;              /* +0x08 */
    unsigned int specular;             /* +0x0C */
    unsigned int emissive;             /* +0x10 */
    float spec_power;                  /* +0x14 */
    TmsLayer layer[4];                 /* +0x18 */
    int alt;                           /* +0x98 */
    unsigned short nprims;             /* +0x9C */
    unsigned short prim;               /* +0x9E */
    unsigned char mask_bit;            /* +0xA0 */
    unsigned char padA1[3];
} TmsMat;

typedef struct TmsPrim {               /* 0x18 */
    unsigned short bone[8];
    int vtype;                         /* +0x10 */
    unsigned short ndraws;             /* +0x14 */
    unsigned short draw;               /* +0x16 */
} TmsPrim;

typedef struct TmsDraw {               /* 0x0C */
    int prim;
    int count;                         /* +0x04 */
    int offset;                        /* +0x08 */
} TmsDraw;

typedef struct Tms {
    unsigned int magic;                /* "TGMS" */
    TmsDraw *draws;                    /* +0x04 */
    char *verts;                       /* +0x08 */
    TmsMat *mats;                      /* +0x0C */
    TmsTex *texs;                      /* +0x10 */
    TmsPrim *prims;                    /* +0x14 */
    void *unk18;                       /* +0x18 */
    void *unk1c;                       /* +0x1C */
    Mat4 *mtx;                         /* +0x20 */
    char *names;                       /* +0x24 */
    int nmats;                         /* +0x28 */
    int pad2c[2];
    int ntexs;                         /* +0x34 */
    int pad38;
    int nbones;                        /* +0x3C */
    int pad40;
    float bmin[3];                     /* +0x44 */
    float bmax[3];                     /* +0x50 */
    unsigned int flags;                /* +0x5C 1 relocated */
} Tms;

typedef struct TmsHooks {
    int (*tex)(unsigned int **list, Tms *t, int mat, int layer, void *arg);
    int (*draw)(unsigned int **list, Tms *t, int mat, int layer, int after, void *arg);
    TmsMat *mats;                      /* +0x08 replacements for materials with flag 4 */
    TmsTex *texs;                      /* +0x0C replacements for textures with alt >= 0 */
    unsigned int mask;                 /* +0x10 bits of the hidden materials */
} TmsHooks;

typedef struct TmsCull {
    int pad[9];
    int on;                            /* +0x24 */
} TmsCull;

typedef struct Light {
    int type;
    unsigned int ambient;
    unsigned int diffuse;
    unsigned int specular;
    float pos[3];
    float dir[3];
    float spot[2];
    float att[3];
} Light;

typedef struct LightState {
    unsigned char lighting : 1;
    unsigned char on : 4;
    Light l[4];
    unsigned int ambient;
} LightState;

typedef struct Camera {
    Vec4 pos;
    Vec4 target;
    Vec4 up;
    Vec4 dir;
    Mat4 view;
    Vec4 right;                        /* +0x80 */
    Vec4 up2;                          /* +0x90 */
} Camera;

extern void *(*tms_lookup)(char *name);
extern unsigned short tms_bone_cache[8]; /* 0x089033B4 */
extern void *tms_hook_a;               /* 0x089033A8 */
extern void *tms_hook_b;               /* 0x089033AC */
extern LightState light_state;

unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_tex_func(unsigned int *list, int func, int rgba);
unsigned int *dl_tex_env_color(unsigned int *list, unsigned int color);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(unsigned int *list, float u, float v);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *dl_material_colors(unsigned int *list, unsigned int a, unsigned int d, unsigned int s, unsigned int e);
unsigned int *dl_specular_power(float p, unsigned int *list);
unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_depth_func(unsigned int *list, int func);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
unsigned int *dl_depth_mask(unsigned int *list, int mask);
unsigned int *dl_pixel_mask(unsigned int *list, unsigned int mask);
unsigned int *dl_bone_matrix(unsigned int *list, int index, float *m);
unsigned int *dl_tex_map_mode(unsigned int *list, int mode, int a, int b);
unsigned int *dl_light(unsigned int *list, int n, int type, int comp, float *pos);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *vertices);
unsigned int *gim_set_texture_frame(unsigned int *list, void *gim, int frame);
void mtx_push(void);
Camera *camera_get(void);
float *vec3_neg(float *out, float *v);
unsigned int *tms_env_reflect(unsigned int *list, int after);
unsigned int *tms_env_light(unsigned int *list, int after);
int tms_visible(Tms *t);
unsigned int *tms_draw_prims(unsigned int *list, Tms *t, float *bones, TmsMat *m);
float sqrtf(float x);
void *heap_malloc_cur(int align, unsigned int size);
void free(void *p);

Tms *tms_relocate(Tms *t)
{
    int i;
    int k;

    if (t == 0) {
        return 0;
    }
    if (t->magic != 0x534D4754) {
        return 0;
    }
    if (t->flags & 1) {
        return t;
    }
    {
        t->flags |= 1;
        t->draws = (TmsDraw *)((char *)t + (int)t->draws);
        t->verts = (char *)t + (int)t->verts;
        t->mats = (TmsMat *)((char *)t + (int)t->mats);
        t->texs = (TmsTex *)((char *)t + (int)t->texs);
        t->prims = (TmsPrim *)((char *)t + (int)t->prims);
        if (t->nbones > 0) {
            t->unk18 = (char *)t + (int)t->unk18;
            t->unk1c = (char *)t + (int)t->unk1c;
        } else {
            t->unk18 = 0;
            t->unk1c = 0;
        }
        if (t->mtx != 0) {
            t->mtx = (Mat4 *)((char *)t + (int)t->mtx);
        }
        t->names = (char *)t + (int)t->names;
        for (i = 0; i < t->nmats; i++) {
            TmsMat *m = &t->mats[i];

            for (k = 0; k < 4; k++) {
                TmsLayer *l = &m->layer[k];

                if ((int)l->tex != -1) {
                    l->tex = &t->texs[(int)l->tex];
                } else {
                    l->tex = 0;
                }
                switch ((int)l->fn) {
                default:
                    l->fn = 0;
                    break;
                case 1:
                    l->fn = tms_env_reflect;
                    break;
                case 2:
                    l->fn = tms_env_light;
                    break;
                }
            }
        }
        for (i = 0; i < t->ntexs; i++) {
            TmsTex *x = &t->texs[i];

            x->name = (char *)((int)t->names + (int)x->name);
            if (tms_lookup != 0) {
                x->gim = tms_lookup(x->name);
            }
        }
    }
    return t;
}

unsigned int *tms_tex_apply(unsigned int *list, TmsTex *x)
{
    if (x != 0 && x->gim != 0) {
        list = dl_enable(list, 9);
        list = gim_set_texture_frame(list, x->gim, 0);
        list = dl_tex_func(list, x->func, x->rgba);
        list = dl_tex_env_color(list, x->env);
        list = dl_tex_wrap(list, x->wrap_u, x->wrap_v);
        list = dl_tex_offset(list, x->off_u, 1.0f - x->off_v);
        list = dl_tex_scale(list, x->scale_u, -x->scale_v);
    } else {
        list = dl_disable(list, 9);
    }
    return list;
}

#ifdef NON_MATCHING /* the vertex pointer argument is computed before the vertex type */
unsigned int *tms_draw_prims(unsigned int *list, Tms *t, float *bones, TmsMat *m)
{
    int i;

    for (i = 0; i < m->nprims; i++) {
        TmsPrim *p = &(&t->prims[m->prim])[i];
        int k;

        if (bones != 0) {
            for (k = 0; k < 8; k++) {
                unsigned short b = p->bone[k];

                if (b == 0xFFFF) {
                    break;
                }
                if (b != tms_bone_cache[k]) {
                    list = dl_bone_matrix(list, k, bones + b * 16);
                    tms_bone_cache[k] = b;
                }
            }
        }
        for (k = 0; k < p->ndraws; k++) {
            TmsDraw *d = &(&t->draws[p->draw])[k];

            list = draw_array(list, d->prim, p->vtype, d->count, 0, t->verts + d->offset);
        }
    }
    return list;
}
#endif

/* Draws the materials of pass `mode` (1 opaque, 2 translucent, else all). */
unsigned int *tms_draw(unsigned int *list, Tms *t, float *bones, unsigned int mode, TmsHooks *h, TmsCull *c)
{
    int i;

    if (c != 0 && c->on != 0 && tms_visible(t) == 0) {
        return list;
    }
    list = dl_set_tex_filter(list, 1, 1);
    if (t->nbones > 0) {
        for (i = 0; i < 8; i++) {
            tms_bone_cache[i] = 0xFFFF;
        }
    }
    if (t->nbones == 0 && t->mtx != 0) {
        mtx_push();
        mtx_mult(t->mtx);
    }
    for (i = 0; i < t->nmats; i++) {
        TmsMat *m = &t->mats[i];
        int k;

        switch (m->flags & 1) {
        case 0:
            if ((mode & 3) == 2) {
                continue;
            }
            break;
        case 1:
            if ((mode & 3) == 1) {
                continue;
            }
            break;
        }
        if (h != 0 && (m->flags & 8) && (h->mask & (1 << m->mask_bit))) {
            continue;
        }
        if ((m->flags & 4) && h != 0 && h->mats != 0) {
            m = &h->mats[m->alt];
        }
        if (m->flags & 2) {
            list = dl_disable(list, 5);
        } else {
            list = dl_enable(list, 5);
        }
        for (k = 0; k < 4; k++) {
            TmsLayer *l = &m->layer[k];
            TmsTex *x = l->tex;
            int r;

            if (!(l->flags & 1)) {
                break;
            }
            r = 0;
            if (h != 0 && h->tex != 0) {
                r = h->tex(&list, t, i, k, c);
            }
            if (r == 0) {
                if (x != 0 && x->alt != -1 && h != 0 && h->texs != 0) {
                    x = &h->texs[x->alt];
                }
                if (x != 0 && x->gim != 0) {
                    list = tms_tex_apply(list, x);
                } else {
                    list = dl_disable(list, 9);
                }
            }
            list = dl_color(list, l->color);
            if (light_state.lighting && (l->flags & 2)) {
                list = dl_enable(list, 10);
                list = dl_material_colors(list, m->ambient, m->diffuse, m->specular, m->emissive);
                list = dl_specular_power(m->spec_power, list);
            } else {
                list = dl_disable(list, 10);
            }
            if (l->flags & 4) {
                list = dl_enable(list, 0);
                list = dl_alpha_test(list, l->atest_func, l->atest_ref, l->atest_mask);
            } else {
                list = dl_disable(list, 0);
            }
            list = dl_depth_func(list, l->depth_func);
            if (l->flags & 8) {
                list = dl_enable(list, 4);
                list = dl_blend_func(list, l->blend_op, l->blend_src, l->blend_dst, l->fixa, l->fixb);
            } else {
                list = dl_disable(list, 4);
            }
            if (l->flags & 0x10) {
                list = dl_depth_mask(list, 1);
            } else {
                list = dl_depth_mask(list, 0);
            }
            if (l->flags & 0x20) {
                list = dl_pixel_mask(list, 0xFF000000);
            } else {
                list = dl_pixel_mask(list, 0);
            }
            if (l->fn != 0) {
                list = l->fn(list, 0);
            }
            if (h == 0 || h->draw == 0 || h->draw(&list, t, i, k, 0, c) >= 0) {
                list = tms_draw_prims(list, t, bones, m);
            }
            if (h != 0 && h->draw != 0) {
                h->draw(&list, t, i, k, 1, c);
            }
            if (l->fn != 0) {
                list = l->fn(list, 1);
            }
        }
    }
    if (t->nbones == 0 && t->mtx != 0) {
        mtx_pop();
    }
    list = dl_depth_func(list, 7);
    list = dl_depth_mask(list, 0);
    list = dl_pixel_mask(list, 0);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(list, 1.0f, 1.0f);
    return list;
}

void *tms_set_hook_a(void *p)
{
    void *old = tms_hook_a;

    tms_hook_a = p;
    return old;
}

void *tms_set_hook_b(void *p)
{
    void *old = tms_hook_b;

    tms_hook_b = p;
    return old;
}

/* Layer callback 1: environment map from the camera axes (lights 2 and 3 as texture sources). */
unsigned int *tms_env_reflect(unsigned int *list, int after)
{
    switch (after) {
    case 0: {
        Camera *cam = camera_get();
        float v[3];

        vec3_neg(v, &cam->up2.x);
        list = dl_light(list, 2, 0, 2, &cam->right.x);
        list = dl_light(list, 3, 0, 2, v);
        list = dl_tex_map_mode(list, 2, 2, 3);
        break;
    }
    case 1:
        list = dl_light(list, 2, light_state.l[2].type, 6, light_state.l[2].pos);
        list = dl_light(list, 3, light_state.l[3].type, 6, light_state.l[3].pos);
        list = dl_tex_map_mode(list, 0, 0, 0);
        break;
    }
    return list;
}

/* Layer callback 2: texture projected from light 0. */
unsigned int *tms_env_light(unsigned int *list, int after)
{
    switch (after) {
    case 0:
        list = dl_color(list, light_state.l[0].diffuse);
        list = dl_light(list, 0, light_state.l[0].type, 2, light_state.l[0].pos);
        list = dl_tex_map_mode(list, 2, 0, 0);
        break;
    case 1:
        list = dl_light(list, 0, light_state.l[0].type, 6, light_state.l[0].pos);
        list = dl_tex_map_mode(list, 0, 0, 0);
        break;
    }
    return list;
}

/* Same with additive blending. */
unsigned int *tms_env_light_add(unsigned int *list, int after)
{
    switch (after) {
    case 0:
        list = dl_color(list, light_state.l[0].diffuse);
        list = dl_light(list, 0, light_state.l[0].type, 2, light_state.l[0].pos);
        list = dl_tex_map_mode(list, 2, 0, 0);
        list = dl_enable(list, 4);
        list = dl_blend_func(list, 0, 10, 2, 0xFFFFFFFF, 0);
        break;
    case 1:
        list = dl_light(list, 0, light_state.l[0].type, 6, light_state.l[0].pos);
        list = dl_tex_map_mode(list, 0, 0, 0);
        break;
    }
    return list;
}

DrawSphere tms_sphere_def = { { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 0.0f, 0.0f, 0.0f };

#ifdef NON_MATCHING /* one mul.s operand order of the radius */
/* Bounding sphere of the box against the frustum. */
int tms_visible(Tms *t)
{
    DrawSphere s = tms_sphere_def;
    float dx;
    float dy;
    float dz;

    mtx_get_proj(&s.fovy, 0, &s.near, &s.far);
    s.pos.x = (t->bmin[0] + t->bmax[0]) * 0.5f;
    dx = t->bmax[0] - t->bmin[0];
    s.pos.y = (t->bmin[1] + t->bmax[1]) * 0.5f;
    dy = t->bmax[1] - t->bmin[1];
    s.pos.z = (t->bmin[2] + t->bmax[2]) * 0.5f;
    dz = t->bmax[2] - t->bmin[2];
    s.radius = sqrtf(0.25f * (dx * dx + dy * dy + dz * dz));
    return draw_sphere_visible(&s);
}
#endif

void *tms_alloc(unsigned int size)
{
    return heap_malloc_cur(0x10, size);
}

void tms_free(void *p)
{
    free(p);
}
