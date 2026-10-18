// @range 0x0882D208-0x0882D9F0
// @asm 0x0882D208-0x0882D7EC
// @data 0x088ED820-0x088ED858
/* Toon shading for models: two 64x32 ramps drawn into VRAM at 0x1F0000 from the step tables of
   the draw context (light ramp in 5650, the second one in 4444), then layers lit by light 0
   sample the ramp; and the TGMT relocation. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct RampVtx {               /* vertex type 0x800110 / 0x800118 (through mode) */
    short c;
    short x;
    short y;
    short z;
} RampVtx;

typedef struct ToonDraw {              /* ModelDraw (see model.c) */
    int layer[2];
    char flags8;
    char pad9[3];
    float *pos0;                       /* +0x0C step positions 0..1 */
    unsigned char *val0;               /* +0x10 step intensities (high nibble) */
    int n0;                            /* +0x14 */
    float *pos1;                       /* +0x18 */
    unsigned char *val1;               /* +0x1C */
    int n1;                            /* +0x20 */
} ToonDraw;

typedef struct ToonModel {
    char pad[0x54];
    void *tex_hook;                    /* +0x54 */
} ToonModel;

typedef struct TmsLayerFnPart {
    int pad[2];
    void *fn;                          /* +0x08 */
} TmsLayerFnPart;

typedef struct TgmtHdr {
    unsigned int magic;
    void *p1;
    int *tracks;                       /* +0x08 */
    char *keys;                        /* +0x0C */
    void *p4;
    void *p5;
    void *p6;
    int pad1c;
    unsigned int ntracks;              /* +0x20 */
    int pad24[4];
    unsigned int flags;                /* +0x34 */
} TgmtHdr;

unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
void *dl_get_layer(void);
void dl_set_cur_list_base(void *p);
void *dl_get_cur_list_base(void);
void sceGuStart(int cid, void *list, int size);
int sceGuFinish(void);
void *sceGuGetMemory(int size);
void *gfx_set_draw_target(void *list, int addr, int a, int w, int h, int fmt);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *gfx_set_default_target(void *list);
void *dl_tex_sync(void *list);
void *dl_tex_flush(void *list);
void *dl_tex_mode(void *list, int pixfmt, int maxmips, int a2, int swizzle);
void *dl_tex_image(void *list, int mip, int w, int h, int stride, const void *data);
unsigned int *dl_tex_func(unsigned int *list, int func, int rgba);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(unsigned int *list, float u, float v);
unsigned int *tms_env_light(unsigned int *list, int after);
unsigned int *tms_env_light_add(unsigned int *list, int after);
int toon_tex_hook(unsigned int **list, int *t, int mat, int layer, void *arg);

/* Memory inside the current list for n bytes (a sceGu sublist skipped over by a jump). */
#define TOON_ALLOC(list, p, n, size_) \
    do { \
        void *layer_ = dl_get_layer(); \
        void *base_; \
        dl_set_list(list); \
        list = dl_select_layer((int)layer_, &size_); \
        dl_set_cur_list_base(list); \
        sceGuStart(2, list, size_); \
        p = sceGuGetMemory(n); \
        layer_ = dl_get_layer(); \
        base_ = dl_get_cur_list_base(); \
        dl_set_list((unsigned int *)((char *)base_ + sceGuFinish() - 8)); \
        list = dl_select_layer((int)layer_, 0); \
    } while (0)

RampVtx toon_ramp0[4] = { { -1, 0, 0, 0 }, { -1, 0, 32, 0 }, { -1, 64, 0, 0 }, { -1, 64, 32, 0 } };
RampVtx toon_ramp1[3] = { { 0, 0, 32, 0 }, { 0, 64, 0, 0 }, { 0, 64, 32, 0 } };

#ifdef NON_MATCHING /* the vertex templates and the list sizes sit in other stack slots */
unsigned int *model_outline_begin(unsigned int *list, ToonModel *m, ToonDraw *c)
{
    RampVtx *v;
    int n;
    int i;
    unsigned int size0;
    unsigned int size1;
    unsigned int size2;
    unsigned int size3;

    list = dl_disable(list, 9);
    list = dl_disable(list, 3);
    if (c->n0 < 2) {
        n = 4;
        TOON_ALLOC(list, v, 0x20, size0);
        v[0] = toon_ramp0[0];
        v[1] = toon_ramp0[1];
        v[2] = toon_ramp0[2];
        v[3] = toon_ramp0[3];
    } else {
        RampVtx *p;

        n = c->n0 * 2;
        TOON_ALLOC(list, v, c->n0 << 4, size1);
        for (i = 0, p = v; i < c->n0; i++, p += 2) {
            short col = (unsigned char)((c->val0[i] & 0xF0) >> 4) << 12;
            short x;

            p[1].c = col;
            p[0].c = col;
            x = c->pos0[i] * 64.0f;
            p[0].x = x;
            p[0].y = 0;
            p[0].z = 0;
            p[1].x = x;
            p[1].y = 32;
            p[1].z = 0;
        }
    }
    list = gfx_set_draw_target(list, 0x1F0000, -2, 0x40, 0x20, 0);
    list = dl_draw_array(list, 4, 0x800110, n, 0, v);
    list = gfx_set_default_target(list);
    if (c->n1 < 2) {
        n = 4;
        TOON_ALLOC(list, v, 0x20, size2);
        v[0] = (RampVtx){ 0 };
        v[1] = toon_ramp1[0];
        v[2] = toon_ramp1[1];
        v[3] = toon_ramp1[2];
    } else {
        RampVtx *p;

        n = c->n1 * 2;
        TOON_ALLOC(list, v, c->n1 << 4, size3);
        for (i = 0, p = v; i < c->n1; i++, p += 2) {
            unsigned char k = (unsigned char)((c->val1[i] & 0xF0) >> 4);
            short col = k * 0x101 + k * 0x10;
            short x;

            p[1].c = col;
            p[0].c = col;
            x = c->pos1[i] * 64.0f;
            p[0].x = x;
            p[0].y = 0;
            p[0].z = 0;
            p[1].x = x;
            p[1].y = 32;
            p[1].z = 0;
        }
    }
    list = gfx_set_draw_target(list, 0x1F0000, -2, 0x40, 0x20, 2);
    list = dl_draw_array(list, 4, 0x800118, n, 0, v);
    list = gfx_set_default_target(list);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    m->tex_hook = toon_tex_hook;
    return list;
}
#endif

unsigned int *model_outline_end(unsigned int *list, ToonModel *m)
{
    m->tex_hook = 0;
    return list;
}

/* Texture hook: layers using the light-0 projection sample the ramp instead of their texture. */
int toon_tex_hook(unsigned int **list, int *t, int mat, int layer, void *arg)
{
    TmsLayerFnPart *l = (TmsLayerFnPart *)(t[3] + mat * 0xA4 + layer * 0x20 + 0x18);
    unsigned int *dl = *list;

    if (l->fn == tms_env_light) {
        l->fn = tms_env_light_add;
    } else if (l->fn != tms_env_light_add) {
        return 0;
    }
    dl = dl_enable(dl, 9);
    dl = dl_tex_mode(dl, 2, 0, 0, 0);
    dl = dl_tex_image(dl, 0, 0x40, 0x20, 0x40, (char *)sceGeEdramGetAddr() + 0x1F0000);
    dl = dl_tex_func(dl, 3, 1);
    dl = dl_tex_wrap(dl, 1, 1);
    dl = dl_tex_offset(dl, 0.0f, 0.0f);
    dl = dl_tex_scale(dl, 1.0f, 1.0f);
    *list = dl;
    return 1;
}

TgmtHdr *tgm_relocate(TgmtHdr *a)
{
    unsigned int i;

    if (a == 0) {
        return 0;
    }
    if (a->magic != 0x544D4754) {
        return 0;
    }
    if (a->flags & 1) {
        return a;
    }
    a->flags |= 1;
    a->p1 = (char *)a + (int)a->p1;
    a->tracks = (int *)((char *)a + (int)a->tracks);
    a->keys = (char *)a + (int)a->keys;
    a->p4 = (char *)a + (int)a->p4;
    a->p5 = (char *)a + (int)a->p5;
    a->p6 = (char *)a + (int)a->p6;
    for (i = 0; i < a->ntracks; i++) {
        a->tracks[i * 2 + 1] = (int)(a->keys + a->tracks[i * 2 + 1] * 0x10);
    }
    return a;
}
