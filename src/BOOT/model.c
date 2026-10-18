// @range 0x0882C428-0x0882D208
// @asm 0x0882D160-0x0882D1B4
// @data 0x088ED7D0-0x088ED820
/* Models: a TGMS geometry with its bone matrices (local and skinned), the TGMT animations it
   plays (with cross-fade from the previous one), a material animation (TGMA), private copies of
   the replaceable materials and textures, and the two draw passes. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct Tms Tms;
typedef struct Tgmt Tgmt;

typedef struct TmsCore {               /* the parts of Tms used here (see tms.c) */
    unsigned int magic;
    void *draws;
    char *verts;
    unsigned int *mats;                /* +0x0C TmsMat (0xA4) */
    int *texs;                         /* +0x10 TmsTex (0x28) */
    void *prims;
    void *unk18;
    Mat4 *inv;                         /* +0x1C inverse bind matrices */
    Mat4 *mtx;
    char *names;
    int nmats;                         /* +0x28 */
    int nmats2;                        /* +0x2C materials with copies */
    int pad30;
    int ntexs;                         /* +0x34 */
    int ntexs2;                        /* +0x38 textures with copies */
    int nbones;                        /* +0x3C */
} TmsCore;

typedef struct MatRows {
    float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
} __attribute__((aligned(16))) MatRows;

typedef struct MatWords {
    unsigned int w[41];
} MatWords;

typedef struct TexWords {
    int w[10];
} TexWords;

typedef struct TgmtCore {
    unsigned int magic;
    int pad[11];
    unsigned int nframes;              /* +0x30 */
} TgmtCore;

typedef struct TgmaCore {
    int pad[7];
    unsigned int len;                  /* +0x1C */
} TgmaCore;

typedef struct TmsHooks {
    void *tex;
    void *draw;
    void *mats;                        /* +0x08 */
    void *texs;                        /* +0x0C */
    unsigned int mask;                 /* +0x10 */
} TmsHooks;

typedef struct Model {                 /* 0x68 */
    TmsCore *tms;
    void **anims;                      /* +0x04 animation data list, -1 terminated */
    int nanims;                        /* +0x08 */
    Mat4 *bones;                       /* +0x0C */
    Mat4 *skin;                        /* +0x10 bones times inverse bind */
    TgmtCore *cur;                     /* +0x14 */
    TgmtCore *prev;                    /* +0x18 */
    int index;                         /* +0x1C */
    float frame;                       /* +0x20 */
    int prev_index;                    /* +0x24 */
    float prev_frame;                  /* +0x28 */
    void *mats;                        /* +0x2C */
    void *texs;                        /* +0x30 */
    TgmaCore *tgma;                    /* +0x34 */
    float tgma_time;                   /* +0x38 */
    unsigned int flags;                /* +0x3C 1 blending, 2 loop */
    int buf_size;                      /* +0x40 */
    void *buf[2];                      /* +0x44 inflated animations */
    int len[2];                        /* +0x4C */
    TmsHooks hooks;                    /* +0x54 */
} Model;

typedef struct ModelDraw {             /* 0x089033C4 default */
    int layer[2];                      /* display list layers of the two passes */
    char outline : 1;                  /* +0x08 */
    char pad8[27];
    int cull;                          /* +0x24 */
    char stencil : 1;                  /* +0x28 */
} ModelDraw;

extern float model_blend_rate[10];     /* 0x08874FC0 1/n for animations shorter than 10 frames */
extern ModelDraw model_draw_def;       /* 0x089033C4 */

void *tms_alloc(unsigned int size);
void tms_free(void *p);
TmsCore *tms_relocate(void *p);
int tms_visible(TmsCore *t);
unsigned int *tms_draw(unsigned int *list, TmsCore *t, Mat4 *bones, unsigned int mode, TmsHooks *h, ModelDraw *c);
TgmtCore *tgm_relocate(void *p);
int is_gzip(const void *p);
int gzip_decompress(void *dst, int dst_len, const void *src, int *out_size);
void mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b);
void anim_eval(TgmtCore *a, TmsCore *t, Mat4 *bones, float *dt, float frame);
void anim_blend(TgmtCore *a, TmsCore *t, Mat4 *bones, TgmtCore *prev, float *dt, float frame, float prev_frame, float w);
void tgma_apply(TgmaCore *anim, TmsCore *model, void *mats, void *uv, float t);
void mtx_mark_cur_dirty(void);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_stencil_op(unsigned int *list, int fail, int zfail, int zpass);
unsigned int *dl_stencil_func(unsigned int *list, int func, int ref, int mask);
unsigned int *model_outline_begin(unsigned int *list, Model *m, ModelDraw *c);
unsigned int *model_outline_end(unsigned int *list, Model *m);
int model_anim_frame_count(Model *m);
int model_set_buf_size(Model *m, int size);
void model_free(Model *m);
void model_mtx_dirty(void);
unsigned int *model_stencil_begin(unsigned int *list);
unsigned int *model_stencil_end(unsigned int *list);

/* Current animation from raw TGMT data, or inflated into the buffer not holding the previous one. */
void model_set_anim_data(Model *m, void *data)
{
    int k;
    int size;

    if (data == 0) {
        m->cur = 0;
        return;
    }
    if (*(unsigned int *)data == 0x544D4754) {
        m->cur = data;
        return;
    }
    if (is_gzip(data) == 0) {
        return;
    }
    if (m->buf_size == 0) {
        model_set_buf_size(m, 0x18000);
    }
    k = m->buf[0] == m->prev;
    m->cur = m->buf[k];
    m->len[k] = gzip_decompress(m->cur, m->buf_size, data, &size);
    m->cur = tgm_relocate(m->cur);
}

void *(*model_alloc)(unsigned int size) = tms_alloc;
void (*model_freefn)(void *p) = tms_free;

void model_set_allocator(void *(*alloc)(unsigned int size), void (*freefn)(void *p))
{
    if (alloc == 0) {
        model_alloc = tms_alloc;
        model_freefn = tms_free;
    } else {
        model_alloc = alloc;
        model_freefn = freefn;
    }
}

/* Model of a TGMS file with the animation list `anims` (-1 terminated) or none. */
Model *model_load(void *data, void **anims)
{
    TmsCore *t = tms_relocate(data);
    Model *m;
    int i;

    if (t == 0) {
        return 0;
    }
    m = model_alloc(0x68);
    if (m == 0) {
        return 0;
    }
    memset(m, 0, 0x68);
    m->tms = t;
    if (t->nbones > 0) {
        MatRows id = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        m->bones = model_alloc(t->nbones << 6);
        if (m->bones == 0) {
            goto fail;
        }
        m->skin = model_alloc(t->nbones << 6);
        if (m->skin == 0) {
            goto fail;
        }
        for (i = 0; i < t->nbones; i++) {
            *(MatRows *)&m->bones[i] = id;
            *(MatRows *)&m->skin[i] = id;
        }
    }
    if (t->nmats2 > 0) {
        m->mats = model_alloc(t->nmats2 * 0xA4);
        if (m->mats == 0) {
            goto fail;
        }
        for (i = 0; i < t->nmats; i++) {
            unsigned int *s = &t->mats[i * 41];

            if (s[0] & 4) {
                MatWords *d = (MatWords *)((char *)m->mats + s[38] * 0xA4);

                *d = *(MatWords *)s;
            }
        }
    }
    if (t->ntexs2 > 0) {
        m->texs = model_alloc(t->ntexs2 * 0x28);
        if (m->texs == 0) {
            goto fail;
        }
        for (i = 0; i < t->ntexs; i++) {
            int *s = &t->texs[i * 10];

            if (s[9] != -1) {
                TexWords *d = (TexWords *)((char *)m->texs + s[9] * 0x28);

                *d = *(TexWords *)s;
            }
        }
    }
    if (anims == 0) {
        m->anims = 0;
        m->nanims = 0;
        m->cur = 0;
        m->index = -1;
        m->frame = 0.0f;
        m->prev_index = -1;
        m->prev_frame = 0.0f;
    } else {
        void **p;

        m->anims = anims;
        m->nanims = 0;
        i = 0;
        p = anims;
        while (1) {
            if (*(int *)p == -1) {
                break;
            }
            p++;
            i++;
        }
        m->nanims = i;
        model_set_anim_data(m, anims[0]);
        m->index = 0;
        m->frame = 0.0f;
        m->prev_index = 0;
        m->prev_frame = 0.0f;
    }
    m->hooks.mats = m->mats;
    m->hooks.texs = m->texs;
    return m;
fail:
    if (m != 0) {
        model_free(m);
    }
    return 0;
}

void model_free(Model *m)
{
    if (m != 0) {
        if (m->buf[0] != 0) {
            model_freefn(m->buf[0]);
        }
        if (m->buf[1] != 0) {
            model_freefn(m->buf[1]);
        }
        if (m->bones != 0) {
            model_freefn(m->bones);
        }
        if (m->skin != 0) {
            model_freefn(m->skin);
        }
        if (m->mats != 0) {
            model_freefn(m->mats);
        }
        if (m->texs != 0) {
            model_freefn(m->texs);
        }
        model_freefn(m);
    }
}

/* Starts animation n of the list; flag 1 cross-fades from the current one, 2 loops. */
int model_anim_play(Model *m, int n, unsigned int flags)
{
    void *data;

    if (m->anims == 0) {
        return -1;
    }
    data = m->anims[n];
    if (n < 0 || n >= m->nanims || data == 0) {
        return -1;
    }
    if (flags & 1) {
        m->prev = m->cur;
        m->prev_index = m->index;
        m->prev_frame = m->frame;
    }
    m->flags = flags;
    model_set_anim_data(m, data);
    m->index = n;
    m->frame = 0.0f;
    return 0;
}

int model_anim_index(Model *m)
{
    return m->index;
}

/* Single animation from data (none when null). */
void model_anim_set(Model *m, void *data)
{
    if (data == 0) {
        m->anims = 0;
        m->cur = 0;
        m->index = -1;
    } else {
        m->nanims = 1;
        m->prev_index = 0;
        m->prev_frame = 0.0f;
        m->anims = (void **)&m->cur;
        model_set_anim_data(m, data);
        m->index = 0;
        m->frame = 0.0f;
    }
}

void model_set_tgma(Model *m, TgmaCore *a)
{
    m->tgma = a;
    m->tgma_time = 0.0f;
}

/* Advances the animations by *dt frames (1 when null) and rebuilds the skin matrices. */
void model_update(Model *m, float *dt)
{
    TmsCore *t = m->tms;
    TgmtCore *a = m->cur;
    TgmaCore *ma = m->tgma;
    float d;

    if (t == 0) {
        return;
    }
    if (dt != 0) {
        d = *dt;
    } else {
        d = 1.0f;
    }
    if (a != 0) {
        int n = model_anim_frame_count(m);
        int i;

        m->frame += d;
        if (m->flags & 2) {
            float f = n;

            while (m->frame >= f) {
                m->frame -= f;
                m->flags &= ~1;
            }
        } else if (m->frame > (float)n) {
            m->frame = n;
        }
        if ((m->flags & 1) == 0) {
            anim_eval(a, t, m->bones, dt, m->frame);
        } else {
            float k;

            if (a->nframes >= 10) {
                k = 0.1f;
            } else {
                k = model_blend_rate[a->nframes];
            }
            if (m->frame * k >= 1.0f) {
                m->flags &= ~1;
                anim_eval(a, t, m->bones, dt, m->frame);
            } else {
                anim_blend(a, t, m->bones, m->prev, dt, m->frame, m->prev_frame, m->frame * k);
            }
        }
        for (i = 0; i < m->tms->nbones; i++) {
            mat4_mul(&m->skin[i], &m->bones[i], &t->inv[i]);
        }
    }
    if (ma != 0) {
        m->tgma_time += d;
        while (m->tgma_time > (float)ma->len) {
            m->tgma_time -= (float)ma->len;
        }
        tgma_apply(ma, t, m->mats, m->texs, m->tgma_time);
    }
}

/* Opaque pass on layer[0], translucent pass on layer[1]. */
void model_draw(Model *m, ModelDraw *c)
{
    unsigned int *list;

    if (m->tms == 0) {
        return;
    }
    if (c == 0) {
        c = &model_draw_def;
    }
    if (c->cull != 0 && tms_visible(m->tms) == 0) {
        return;
    }
    list = (unsigned int *)dl_select_layer(c->layer[0], 0);
    if (c->outline) {
        list = model_outline_begin(list, m, c);
    }
    if (c->stencil) {
        list = model_stencil_begin(list);
    }
    list = tms_draw(list, m->tms, m->skin, 1, &m->hooks, c);
    if (c->stencil) {
        list = model_stencil_end(list);
    }
    if (c->outline) {
        list = model_outline_end(list, m);
    }
    dl_set_list(list);
    model_mtx_dirty();
    list = (unsigned int *)dl_select_layer(c->layer[1], 0);
    if (c->outline) {
        list = model_outline_begin(list, m, c);
    }
    if (c->stencil) {
        list = model_stencil_begin(list);
    }
    list = tms_draw(list, m->tms, m->skin, 2, &m->hooks, c);
    if (c->stencil) {
        list = model_stencil_end(list);
    }
    if (c->outline) {
        list = model_outline_end(list, m);
    }
    dl_set_list(list);
}

void model_get_bone(void *model, Mat4 *out, int i)
{
    memcpy(out, &((Model *)model)->bones[i], 0x40);
}

float model_anim_frame(Model *m)
{
    if (m == 0) {
        return -1.0f;
    }
    return m->frame;
}

int model_anim_frame_count(Model *m)
{
    if (m == 0) {
        return -1;
    }
    if (m->cur == 0) {
        return -2;
    }
    return m->cur->nframes - ((m->flags & 2) != 0);
}

void model_set_mask(Model *m, unsigned int mask)
{
    m->hooks.mask = mask;
}

unsigned int model_get_mask(Model *m)
{
    return m->hooks.mask;
}

/* Reallocates the two animation buffers. */
int model_set_buf_size(Model *m, int size)
{
    int i;

    if (m->buf_size == size) {
        return 1;
    }
    for (i = 0; i < 2; i++) {
        void *p = model_alloc(size);

        memcpy(p, m->buf[i], m->len[i]);
        if (m->cur == m->buf[i]) {
            m->cur = p;
        }
        if (m->prev == m->buf[i]) {
            m->prev = p;
        }
        if (m->buf[i] != 0) {
            model_freefn(m->buf[i]);
        }
        m->buf[i] = p;
    }
    m->buf_size = size;
    return 1;
}

#ifdef NON_MATCHING /* the original leaves the delay slots after the stores unfilled */
/* Texture (0) or draw (1) hook; v != -1 sets it. Returns the previous one. */
void *model_hook(Model *m, int which, void *v)
{
    void *old;

    switch (which) {
    case 0:
        old = m->hooks.tex;
        if (v != (void *)-1) {
            m->hooks.tex = v;
        }
        break;
    case 1:
        old = m->hooks.draw;
        if (v != (void *)-1) {
            m->hooks.draw = v;
        }
        break;
    default:
        old = 0;
        break;
    }
    return old;
}
#endif

void model_mtx_dirty(void)
{
    mtx_mark_cur_dirty();
}

unsigned int *model_stencil_begin(unsigned int *list)
{
    list = dl_stencil_op(list, 1, 0xFF, 0xFF);
    list = dl_stencil_func(list, 0, 0, 2);
    return dl_enable(list, 3);
}

unsigned int *model_stencil_end(unsigned int *list)
{
    return dl_disable(list, 3);
}
