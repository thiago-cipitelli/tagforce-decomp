// @range 0x08828590-0x0882923C
// @asm 0x08829184-0x0882923C
// @data 0x088ED474-0x088ED4D0
/* EHF particles, part 4: drawing the current pool as camera-facing quads (two triangles of a
   strip per particle, uv rectangle and size from the sprite state), particle colors (constant
   or interpolated over the age by color keys) and the sprite texture, size and uv commands. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct EhfParticle {
    Vec4 pos;                          /* w: age (0 free) */
    Vec4 vel;                          /* +0x10 w: lifetime */
    unsigned int color;                /* +0x20 ABGR */
    float pad[3];
} EhfParticle;

typedef struct EhfPool {
    EhfParticle *p;
    int n;
} EhfPool;

typedef struct Vec2 {
    float u, v;
} Vec2;

typedef struct EhfSprite {             /* instance memory of command 0x14 */
    Vec2 uv0;                          /* uv rectangle */
    Vec2 uv1;                          /* +0x08 */
    float size;                        /* +0x10 */
} EhfSprite;

typedef struct EhfSpriteVtx {          /* vertex type 0x19F */
    float u, v;
    unsigned int color;                /* +0x08 */
    float x, y, z;                     /* +0x0C */
} EhfSpriteVtx;

typedef struct EhfColorKey {
    unsigned int color;
    float t;                           /* +0x04 age */
} EhfColorKey;

typedef struct EhfNodeCtx {
    int pad[2];
    void *ehp;                         /* +0x08 archive of the EHF file */
} EhfNodeCtx;

typedef struct EhfNodeX {              /* EhfNode with its owner */
    int frame;
    int unk4;
    float time;
    EhfNodeCtx *ctx;                   /* +0x0C */
} EhfNodeX;

extern EhfPool *ehf_pool;
extern EhfSprite *ehf_sprite;          /* 0x088FA028 */
extern void *ehf_default_gim;          /* 0x088745B0: an embedded GIM file whose first word is read as the
                                          texture pointer */

unsigned int *postfx_get_param(void);
void postfx_set_param(unsigned int *list);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_material_colors(unsigned int *list, int a, int b, int c, int d);
unsigned int *dl_color_material(unsigned int *list, int mode);
unsigned int *dl_ambient_light(unsigned int *list, int color);
unsigned int *dl_color(unsigned int *list, int color);
unsigned int *dl_tex_env_color(unsigned int *list, int color);
unsigned int *dl_tex_func(unsigned int *list, int func, int rgba);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
void *dl_alloc_inline(unsigned int **list, int size);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *vertices);
unsigned int *mtx_flush_alias(unsigned int *list);
unsigned int *gim_set_texture(unsigned int *list, void *gim);
unsigned int *gim_set_texture_frame(unsigned int *list, void *gim, int frame);
void *ehp_find(void *ehp, void *name, int flags);
void mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b);
Vec4 *vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_scale_w(Vec4 *out, Vec4 *a, float s);

Vec4 ehf_sprite_corner[4] = {
    { -0.5f, 0.5f, 0.0f, 1.0f },
    { 0.5f, 0.5f, 0.0f, 1.0f },
    { -0.5f, -0.5f, 0.0f, 1.0f },
    { 0.5f, -0.5f, 0.0f, 1.0f },
};

void ehf_sprite_vertex(EhfSpriteVtx *v, Vec2 *uv, Vec4 *pos, unsigned int color);

/* 0x14: sprite state for the commands below; at the end of the node, the live particles of the
   current pool are drawn as quads in view space. */
int ehf_cmd_sprite(EhfSprite *s, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec2 uv[4] = { 0 };
    unsigned int *list;
    EhfPool *pool = ehf_pool;

    switch (phase) {
    case 1:
        return 0x14;
    case 2:
        memset(s, 0, 0x14);
        s->size = 50.0f;
        s->uv0 = (Vec2){ 0 };
        s->uv1 = (Vec2){ 1.0f, 1.0f };
        return 0x14;
    case 5:
        list = postfx_get_param();
        list = dl_disable(list, 0);
        list = dl_disable(list, 3);
        list = dl_disable(list, 0x11);
        list = dl_disable(list, 4);
        list = dl_disable(list, 10);
        list = dl_disable(list, 0xB);
        list = dl_disable(list, 0xC);
        list = dl_disable(list, 0xD);
        list = dl_disable(list, 0xE);
        list = dl_disable(list, 9);
        list = dl_material_colors(list, -1, -1, -1, -1);
        list = dl_color_material(list, 7);
        list = dl_ambient_light(list, -1);
        list = dl_color(list, -1);
        list = dl_tex_env_color(list, -1);
        postfx_set_param(list);
        ehf_sprite = s;
        return 0x80000000;
    case 6: {
        Mat4 view;
        Mat4 world;
        Mat4 mv;
        Vec4 c;
        Vec4 q;
        EhfSpriteVtx *base;
        EhfSpriteVtx *v;
        int count;
        int i;

        ehf_sprite = 0;
        list = postfx_get_param();
        mtx_set_mode(1);
        mtx_get34(&world);
        mtx_identity();
        mtx_set_mode(2);
        mtx_get34(&view);
        mtx_identity();
        list = mtx_flush_alias(list);
        mat4_mul(&mv, &world, &view);
        base = v = dl_alloc_inline(&list, pool->n * 6 * sizeof(EhfSpriteVtx));
        uv[0] = s->uv0;
        uv[1].u = s->uv1.u;
        uv[1].v = s->uv0.v;
        uv[2].u = s->uv0.u;
        uv[2].v = s->uv1.v;
        uv[3] = s->uv1;
        count = 0;
        for (i = 0; i < pool->n; i++) {
            EhfParticle *p = &pool->p[i];

            if (p->pos.w != 0.0f) {
                unsigned int color = p->color;

                c = p->pos;
                c.w = 1.0f;
                mat4_transform_vec4(&c, &mv, &c);
                vec3_scale_w(&q, &ehf_sprite_corner[0], s->size);
                vec3_add_w(&q, &q, &c);
                ehf_sprite_vertex(&v[0], &uv[0], &q, color);
                ehf_sprite_vertex(&v[1], &uv[0], &q, color);
                vec3_scale_w(&q, &ehf_sprite_corner[1], s->size);
                vec3_add_w(&q, &q, &c);
                ehf_sprite_vertex(&v[2], &uv[1], &q, color);
                vec3_scale_w(&q, &ehf_sprite_corner[2], s->size);
                vec3_add_w(&q, &q, &c);
                ehf_sprite_vertex(&v[3], &uv[2], &q, color);
                vec3_scale_w(&q, &ehf_sprite_corner[3], s->size);
                vec3_add_w(&q, &q, &c);
                ehf_sprite_vertex(&v[4], &uv[3], &q, color);
                ehf_sprite_vertex(&v[5], &uv[3], &q, color);
                v += 6;
                count += 6;
            }
        }
        list = draw_array(list, 4, 0x19F, count, 0, base);
        mtx_set_mode(1);
        mtx_load(&world);
        mtx_set_mode(2);
        mtx_load(&view);
        list = mtx_flush_alias(list);
        postfx_set_param(list);
        return 0;
    }
    }
    return 0;
}

/* 0x11: color (ABGR) of every particle of the pool. */
int ehf_cmd_ptcl_color(void *inst, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPool *pool = ehf_pool;

    switch (phase) {
    case 5: {
        unsigned int color;
        void *out[1] = { &color };
        char fmt[2] = "n";
        EhfParticle *p;
        EhfParticle *end;

        ehf_cmd_args(0, cmd, 0, out, fmt);
        for (p = pool->p, end = p + pool->n; p < end; p++) {
            p->color = color;
        }
        return 0;
    }
    }
    return 0;
}

/* 0x1D: color keys (color, age)...: particle colors interpolated over their age. */
int ehf_cmd_ptcl_color_keys(void *inst, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPool *pool = ehf_pool;

    switch (phase) {
    case 5: {
        int n;
        EhfColorKey *k;
        EhfParticle *p;
        EhfParticle *end;

        if ((cmd->argc & 1) || cmd->argc < 2) {
            return 0;
        }
        k = (EhfColorKey *)cmd->a.i;
        n = (unsigned int)cmd->argc >> 1;
        for (p = pool->p, end = p + pool->n; p < end; p++) {
            float age = p->pos.w;
            int i;
            EhfColorKey *a;
            EhfColorKey *b;
            float t;
            float u;
            float ca;
            float cb;

            if (age == 0.0f) {
                continue;
            }
            if (age <= k[0].t) {
                p->color = k[0].color;
                continue;
            }
            if (age >= k[n - 1].t) {
                p->color = k[n - 1].color;
                continue;
            }
            for (i = 1; i < n; i++) {
                if (age < k[i].t) {
                    break;
                }
            }
            b = &k[i];
            a = b - 1;
            t = (b->t - age) / (b->t - a->t);
            u = 1.0f - t;
            p->color = 0;
            ca = a->color >> 24 & 0xFF;
            cb = b->color >> 24 & 0xFF;
            p->color |= ((unsigned int)(ca * t + cb * u) & 0xFF) << 24;
            ca = a->color >> 16 & 0xFF;
            cb = b->color >> 16 & 0xFF;
            p->color |= ((unsigned int)(ca * t + cb * u) & 0xFF) << 16;
            ca = a->color >> 8 & 0xFF;
            cb = b->color >> 8 & 0xFF;
            p->color |= ((unsigned int)(ca * t + cb * u) & 0xFF) << 8;
            ca = a->color >> 0 & 0xFF;
            cb = b->color >> 0 & 0xFF;
            p->color |= ((unsigned int)(ca * t + cb * u) & 0xFF) << 0;
        }
        return 0;
    }
    }
    return 0;
}

/* 0x12: sprite texture by name in the EHF's archive (none: texturing off). */
int ehf_cmd_sprite_tex(void *inst, EhfNodeX *node, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 5: {
        void *tex = ehp_find(node->ctx->ehp, cmd->a.p[0], 0);
        unsigned int *list = postfx_get_param();

        if (tex == 0) {
            list = dl_disable(list, 9);
        } else {
            list = dl_enable(list, 9);
            list = gim_set_texture(list, tex);
        }
        postfx_set_param(list);
        return 0;
    }
    }
    return 0;
}

/* 0x13: sprite size. */
int ehf_cmd_sprite_size(void *inst, EhfNode *node, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 5: {
        float size;
        void *out[1] = { &size };
        char fmt[2] = "f";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        ehf_sprite->size = size;
        return 0;
    }
    }
    return 0;
}

/* 0x1B: sprite uv rectangle (u0, v0, u1, v1). */
int ehf_cmd_sprite_uv(void *inst, EhfNode *node, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 5:
        memcpy(ehf_sprite, cmd->a.f, 0x10);
        return 0;
    }
    return 0;
}

void ehf_sprite_vertex(EhfSpriteVtx *v, Vec2 *uv, Vec4 *pos, unsigned int color)
{
    v->u = uv->u;
    v->v = uv->v;
    v->color = color;
    v->x = pos->x;
    v->y = pos->y;
    v->z = pos->z;
}

#ifdef NON_MATCHING /* the name argument goes through $v1 before the ehp_find call in the original */
/* 0x101: texture (name resolved to the GIM at load, none: the default), frame, function and wrap. */
int ehf_cmd_tex(void *inst, EhfNodeX *node, EhfCmd *cmd, int phase)
{
    void **a = cmd->a.p;

    switch (phase) {
    case 0:
        a[0] = ehp_find(node->ctx->ehp, a[0], 0);
        return 0;
    case 5: {
        void *tex = a[0];
        unsigned int *list;

        if (tex == 0) {
            tex = ehf_default_gim;
        }
        list = postfx_get_param();
        list = gim_set_texture_frame(list, tex, (int)a[1]);
        list = dl_tex_func(list, (int)a[2], (int)a[3]);
        list = dl_tex_wrap(list, (int)a[4], (int)a[5]);
        postfx_set_param(list);
        return 0;
    }
    }
    return 0;
}
#endif
