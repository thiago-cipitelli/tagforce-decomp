// @range 0x0882FC3C-0x08833420
// @asm 0x0882FCE8-0x0882FD48
// @asm 0x0882FD74-0x0882FFE4
// @asm 0x08830E4C-0x088310BC
// @asm 0x08831594-0x0883297C
// @asm 0x08832E68-0x08833420
// @data 0x088ED960-0x088ED9A8
/* EHF animation player (a Flash-like format: coordinates in twips, a timeline of place/remove
   commands, nested sprites, shapes and text). An instance holds a pool of 0x28-byte clips linked
   in circular lists sorted by depth; ehf_draw advances the timeline once per tick and walks the
   display list, evaluating each clip's keyframe track (matrix, color transform, morph ratio) and
   drawing shapes with stencil masking. */
#include "psp.h"
#include "libc.h"

#define EHF_MAGIC 0x1A464845           /* "EHF\x1a" */
/* A file or an instance: files resolve to their instance. */
#define EHF_INST(p) (*(int *)(p) == EHF_MAGIC ? ehf_resolve((EhfFile *)(p)) : (EhfInst *)(p))

typedef struct EhfInst EhfInst;

typedef struct EhfHdr {                /* file + 0x10, offsets from the start of the file */
    int movie;                         /* +0x00 EhfMovie */
    int unk04;
    int inst;                          /* +0x08 instance embedded in the file */
    int inst_size;                     /* +0x0C instance size (header + clip pool) */
    int cmds;                          /* +0x10 root timeline */
    int cmds_size;                     /* +0x14 */
    int chars;                         /* +0x18 character blob, ids in 16-byte units */
    int unk1c;
    int tex;                           /* +0x20 texture table, 16 bytes per entry */
    int tex_size;                      /* +0x24 */
    EhfInst *attached;                 /* +0x28 instance used instead of the embedded one */
} EhfHdr;

typedef struct EhfFile {
    int magic;
    int pad04[3];
    EhfHdr h;                          /* +0x10 */
} EhfFile;

typedef struct EhfMovie {
    int frames;                        /* +0x00 */
    int unk04;
    unsigned int rate;                 /* +0x08 ticks per frame */
    int unk0c;
    int xmin, xmax, ymin, ymax;        /* +0x10 bounds in twips */
} EhfMovie;

typedef struct EhfTex {                /* texture table entry */
    int unk0;
    void *gim;                         /* +0x04 */
    int unk8;
    unsigned int flags;                /* +0x0C 1 additive blend, 0x10 linear filter */
} EhfTex;

typedef struct EhfCmd {                /* timeline command, 12 bytes */
    int chr;                           /* character id, -1 remove or end of frame */
    unsigned short track;              /* +0x04 index in the character's track table */
    unsigned short depth;              /* +0x06 0xFFFF with chr -1: nothing to remove */
    unsigned short clip;               /* +0x08 mask depth, 0 none */
    unsigned short wait;               /* +0x0A frames to wait after this command */
} EhfCmd;

typedef struct EhfTrack {              /* keyframes of a placed character */
    float m[6];                        /* base 2x3 matrix a, b, c, d, tx, ty */
    float pos_scale;                   /* +0x18 */
    float scale_scale;                 /* +0x1C */
    float rot_scale;                   /* +0x20 */
    unsigned short flags;              /* +0x24 1 pos, 2 scale, 4 rotation, 8 color mul, 0x30 color add, 0x40 ratio */
    unsigned short nkeys;              /* +0x26 */
    unsigned short key_size;           /* +0x28 */
    unsigned short pad2a;
    unsigned char keys[1];             /* +0x2C u16 frame then the values of each flag */
} EhfTrack;

typedef struct EhfVec4 {
    float x, y, z, w;
} EhfVec4;

typedef struct EhfColor {              /* Flash color transform: mul (0x100 = 1) then add */
    int mul[4];
    int add[4];
} EhfColor;

typedef struct EhfClip {               /* a placed character, 0x28 */
    struct EhfClip *next;              /* circular list sorted by depth */
    struct EhfClip *prev;
    int *chr;                          /* +0x08 character (type first), 0 when the slot is free */
    EhfTrack *track;                   /* +0x0C */
    struct EhfClip *child;             /* +0x10 display list of a sprite */
    EhfCmd *cmd;                       /* +0x14 next timeline command */
    EhfCmd *cmd_end;                   /* +0x18 */
    unsigned short start;              /* +0x1C parent frame at placement */
    unsigned short depth;              /* +0x1E */
    unsigned short clip;               /* +0x20 */
    unsigned short frame;              /* +0x22 */
    unsigned short wait;               /* +0x24 */
    unsigned short frames;             /* +0x26 */
} EhfClip;

struct EhfInst {
    EhfFile *file;                     /* +0x00 */
    EhfHdr *hdr;                       /* +0x04 */
    EhfMovie *movie;                   /* +0x08 */
    int wait;                          /* +0x0C ticks left in the current frame */
    unsigned short w, h;               /* +0x10 */
    short x, y;                        /* +0x14 */
    unsigned char ready;               /* +0x18 */
    unsigned char pad19;
    unsigned short unk1a;
    int tick;                          /* +0x1C last tick drawn */
    unsigned short target;             /* +0x20 frame to stop at */
    unsigned short limit;              /* +0x22 */
    short saved;                       /* +0x24 target saved by ehf_seek */
    short pad26;
    EhfClip root;                      /* +0x28 */
    EhfClip pool[1];                   /* +0x50 up to hdr->inst_size */
};

typedef struct EhfOpts {               /* draw options */
    int time;                          /* +0x00 the animation advances once per distinct value */
    void *(*tex_cb)(void *list, void *arg, EhfTex *tex, int off); /* +0x04 */
    void *tex_arg;                     /* +0x08 */
    unsigned int flags;                /* +0x0C 4 color, 8 depth mask, 0x40, 0x80 eval callback,
                                          0x100 additive, 0x400 linear filter, 0x800, 0x10000 pos,
                                          0x20000 scale, 0x40000 rotation, 0x80000 current matrix */
    int angle;                         /* +0x10 */
    float x, y;                        /* +0x14 */
    int pad1c;
    float sx, sy;                      /* +0x20 x0.05 */
    int (*eval_cb)(int depth, void *arg, void *ev); /* +0x28 */
    void *eval_arg;                    /* +0x2C */
    unsigned long long mask;           /* +0x30 visible depths */
    int pad38[2];
    EhfColor color;                    /* +0x40 */
} EhfOpts;

typedef struct EhfEval {               /* handed to EhfOpts.eval_cb, which may call fn itself */
    void (*fn)(float *m, EhfColor *cx, int *ratio, EhfTrack *t, int start, int frame);
    float *m;
    float *parent;
    EhfColor *cx;
    int *ratio;
    EhfTrack *track;
    unsigned int start;
    unsigned int frame;
} EhfEval;

EhfColor ehf_color_identity = { { 0x100, 0x100, 0x100, 0x100 }, { 0, 0, 0, 0 } };

/* Element-wise copy (the original does not use a struct copy here). */
#define EHF_COLOR_SET(d, s) \
    ((d).mul[0] = (s).mul[0], (d).mul[1] = (s).mul[1], (d).mul[2] = (s).mul[2], (d).mul[3] = (s).mul[3], \
     (d).add[0] = (s).add[0], (d).add[1] = (s).add[1], (d).add[2] = (s).add[2], (d).add[3] = (s).add[3])
EhfVec4 ehf_row3 = { 0.0f, 0.0f, 0.0f, 1.0f };
float ehf_light_scale[3] = { 32768.0f, 32768.0f, 1.0f };
float ehf_light_dir[3] = { -5.0f, 10.0f, 5.0f };

extern int ehf_debug_range;            /* 0x0890382C draw only depths in [lo, hi] */
extern int ehf_debug_lo;               /* 0x08903838 */
extern int ehf_debug_hi;               /* 0x0890383C */
extern int ehf_hold;                   /* 0x08903848 tick value that does not advance */
extern float ehf_light_pos[3];         /* 0x08903850 */
extern int gfx_saved_mtx_mode;         /* 0x0890385C */

unsigned int *gfx_get_state(void);
void mat4_identity(float *m);
void mat4_mul(float *out, float *a, float *b);
float cosf_vfpu(float x);
float sinf_vfpu(float x);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_scale3(float *v);
void mtx_get34(float *m);
void *mtx_commit(void *list);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_specular_power(void *list, float p);
void *dl_tex_wrap(void *list, int u, int v);
void *dl_material_colors(void *list, unsigned int e, unsigned int a, unsigned int d, unsigned int s);
void *dl_color_material(void *list, int comps);
void *dl_tex_offset(void *list, float u, float v);
void *dl_tex_scale(void *list, float u, float v);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_shade_model(void *list, int mode);
void *dl_stencil_op(void *list, int fail, int zfail, int zpass);
void *dl_stencil_func(void *list, int func, int ref, int mask);
void *dl_pixel_mask(void *list, unsigned int mask);
void *dl_clear_depth(void *list, int z);
void *dl_clear(void *list, int flags);
void *dl_ambient_light(void *list, unsigned int color);
void *dl_set_matrix(void *list, int type, float *m);
void *dl_morph_weight(float w, void *list, int i);
void *dl_light_mode(void *list, int mode);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *gim_set_texture(void *list, void *gim);
void *dl_light(void *list, int light, int type, int comps, float *pos);
void *dl_light_color(void *list, int light, int comp, unsigned int color);
void *dl_material(void *list, int mode, unsigned int color);

EhfInst *ehf_resolve(EhfFile *f);
int ehf_inst_reset(EhfInst *e, EhfFile *f, int keep);
int ehf_clip_step(EhfInst *e, EhfClip *c);
void ehf_track_eval(float *m, EhfColor *cx, int *ratio, EhfTrack *t, int start, int frame);
void *ehf_draw_list(void *list, EhfInst *e, EhfClip *clip, float *mtx, EhfColor *color, EhfOpts *o);
void *ehf_draw_shape(void *list, EhfInst *e, EhfClip *c, unsigned int frame, float *mtx, EhfColor *color, EhfOpts *o);
void *ehf_draw_text(void *list, EhfInst *e, EhfClip *c, unsigned int frame, float *mtx, EhfColor *color);
EhfClip *ehf_clip_alloc(EhfInst *e);
EhfClip *ehf_list_insert(EhfClip *head, EhfClip *c);
EhfClip *ehf_list_remove(EhfClip *head, unsigned int depth);
void ehf_clip_free(EhfClip *c);
void mat4_scale2(float *m, float sx, float sy);
void mat4_translate2(float *m, float x, float y);
void mat4_rotate_z(float *m, int angle);
float mat4_det(float *m);
void mat4_inverse(float *out, float *m);
int ehf_get_data_ptr(EhfFile *f);

/* Calls cb(arg, entry) for each texture entry and stores the next byte of `flags` (0 when none)
   in the entry's flags. */
int ehf_tex_foreach(EhfFile *f, unsigned char *flags, void (*cb)(void *arg, EhfTex *t), void *arg)
{
    EhfHdr *h = &f->h;
    EhfTex *t;
    EhfTex *end;

    if (h->tex == 0) {
        return 1;
    }
    t = (EhfTex *)((char *)f + h->tex);
    end = (EhfTex *)((char *)t + h->tex_size);
    for (; t < end; t++) {
        cb(arg, t);
        t->flags = flags == 0 ? 0 : *flags++;
    }
    return 1;
}

#ifdef NON_MATCHING /* the null checks branch differently (bnez vs bnezl around the header pointer) */
int ehf_tex_set(EhfFile *f, void *gim, int i)
{
    EhfTex *t;
    EhfTex *base;

    if (f == 0) {
        return 0;
    }
    if (gim == 0) {
        return 0;
    }
    if (f->h.tex == 0) {
        return 0;
    }
    base = (EhfTex *)((char *)f + f->h.tex);
    t = base + i;
    if ((EhfTex *)((char *)base + f->h.tex_size) < t) {
        return 0;
    }
    t->gim = gim;
    return 1;
}
#endif

int ehf_get_u32_1c(EhfFile *f)
{
    return f->h.inst_size;
}

void ehf_attach(EhfFile *f, EhfInst *e)
{
    f->h.attached = e;
}

EhfInst *ehf_resolve(EhfFile *f)
{
    EhfHdr *h = &f->h;
    EhfInst *e = h->attached;

    if (e == 0) {
        e = (EhfInst *)((char *)f + h->inst);
    }
    return e;
}

#ifdef NON_MATCHING /* the instance pointer is kept in $a1 until after the merge in the original */
/* Sets up `e` (or the instance embedded in `f`) for a w x h display; negative sizes take the
   movie bounds. */
EhfInst *ehf_inst_init(float w, float h, EhfFile *f, EhfInst *e)
{
    EhfMovie *mv = (EhfMovie *)((char *)f + f->h.movie);
    EhfHdr *hd = &f->h;

    if (e == 0) {
        e = (EhfInst *)((char *)f + hd->inst);
    }
    memset(e, 0, 0x50);
    if (w < 0.0f || h < 0.0f) {
        e->w = (mv->xmax - mv->xmin) / 20;
        e->h = (mv->ymax - mv->ymin) / 20;
    } else {
        e->w = w + 0.5f;
        e->h = h + 0.5f;
    }
    e->target = 0;
    e->limit = mv->frames;
    gfx_saved_mtx_mode = -1;
    ehf_inst_reset(e, f, 0);
    return e;
}
#endif

#ifdef NON_MATCHING /* the resolved instance stays in $v0 before the copy to $s3 in the original */
int ehf_set_frame(EhfInst *e, unsigned int frame)
{
    EhfInst *i;
    EhfClip *root;
    unsigned short target;
    unsigned short limit;

    if (e == 0) {
        return 0;
    }
    i = EHF_INST(e);
    root = &i->root;
    if ((int)frame < i->root.frame) {
        ehf_inst_reset(i, i->file, 0);
    }
    target = i->target;
    limit = i->limit;
    i->target = 0;
    i->limit = i->movie->frames;
    while (root->frame != frame) {
        ehf_clip_step(i, root);
    }
    i->target = target;
    i->limit = limit;
    return 1;
}
#endif

int ehf_set_anim(EhfInst *e, int target, int limit)
{
    unsigned short keep;

    if (e == 0) {
        return 0;
    }
    e = EHF_INST(e);
    e->target = target;
    if (limit < 0) {
        e->limit = e->movie->frames;
    } else {
        e->limit = limit;
    }
    keep = e->unk1a;
    if (target == 0) {
        ehf_inst_reset(e, e->file, 0);
        e->unk1a = keep;
    } else {
        e->target--;
        ehf_inst_reset(e, e->file, 0);
        e->target = target;
        e->unk1a = keep;
    }
    return 1;
}

int ehf_get_frame_count(EhfInst *e)
{
    if (e == 0) {
        return -1;
    }
    e = EHF_INST(e);
    return e->root.frame;
}

int ehf_set_frame_alias(EhfInst *e, unsigned int frame)
{
    return ehf_set_frame(e, frame);
}

int ehf_get_data_ptr(EhfFile *f)
{
    if (f == 0) {
        return -1;
    }
    f = *(int *)f == EHF_MAGIC ? f : *(EhfFile **)f;
    return *(int *)((char *)f + f->h.movie);
}

void ehf_hold_set(int tick)
{
    ehf_hold = tick;
}

void ehf_seek(EhfInst *e, int frame)
{
    EhfInst *i;

    if (e == 0) {
        return;
    }
    i = EHF_INST(e);
    if (ehf_get_data_ptr((EhfFile *)e) < frame) {
        frame = 1;
    }
    ehf_hold = frame;
    if (i->saved == 0) {
        i->saved = i->target;
        i->target = frame;
    } else {
        i->target = frame;
    }
    ehf_inst_reset(i, i->file, 0);
}

void ehf_hold_clear(void)
{
    ehf_hold = 0;
}

void ehf_hold_release(EhfInst *e)
{
    EhfInst *i;

    if (e == 0) {
        return;
    }
    i = EHF_INST(e);
    ehf_hold = 0;
    i->target = i->saved;
    ehf_set_frame(e, i->target);
    ehf_inst_reset(i, i->file, 0);
}

int ehf_get_cur_frame(EhfInst *e)
{
    if (e == 0) {
        return -1;
    }
    e = EHF_INST(e);
    return e->tick;
}

void ehf_get_size(EhfInst *e, int *target, int *limit)
{
    if (e == 0) {
        *limit = -1;
        *target = -1;
        return;
    }
    e = EHF_INST(e);
    *target = e->target;
    *limit = e->limit;
}

int ehf_set_pos(EhfInst *e, short x, short y)
{
    if (e == 0) {
        return 0;
    }
    e = EHF_INST(e);
    e->x = x;
    e->y = y;
    return 1;
}

void *ehf_draw_begin(void *list, EhfInst *e, EhfOpts *o);
void *ehf_draw_end(void *list, EhfInst *e, EhfOpts *o);
void ehf_nop(EhfInst *e, EhfClip *c);
int ehf_camera(EhfInst *e, float *out);

/* Advances `e` by one frame when o->time changed and enough ticks passed, then draws it. */
void *ehf_draw(void *list, EhfInst *e, EhfOpts *o)
{
    float m[16];
    EhfColor color;
    float cur[16];
    unsigned int *st;
    int t;

    if (e == 0) {
        return list;
    }
    e = EHF_INST(e);
    list = ehf_draw_begin(list, e, o);
    if (ehf_debug_range == 0 && ehf_hold != o->time && e->tick != o->time) {
        e->tick = o->time;
        if (e->wait > 0) {
            e->wait--;
        } else {
            e->wait = e->movie->rate - 1;
            st = gfx_get_state();
            if ((int)st[1] < 60) {
                e->wait = st[1] / (60 / e->movie->rate) - 1;
            }
            ehf_clip_step(e, &e->root);
        }
    }
    ehf_nop(e, &e->root);
    mat4_identity(m);
    if (o->flags & 0x80000) {
        mtx_get34(cur);
        mat4_mul(m, m, cur);
    } else {
        mat4_translate2(m, e->x, e->y);
    }
    if (o->flags & 0x10000) {
        mat4_translate2(m, o->x, o->y);
    }
    if (o->flags & 0x20000) {
        mat4_scale2(m, 0.05f * o->sx, 0.05f * o->sy);
    } else {
        mat4_scale2(m, 0.05f, 0.05f);
    }
    if (o->flags & 0x40000) {
        mat4_rotate_z(m, o->angle);
    }
    mat4_translate2(m, -(float)e->movie->xmin, -(float)e->movie->ymin);
    ehf_camera(e, m);
    if (o->flags & 4) {
        color = o->color;
    } else {
        EHF_COLOR_SET(color, ehf_color_identity);
    }
    list = ehf_draw_list(list, e, &e->root, m, &color, o);
    return ehf_draw_end(list, e, o);
}

float *mat4_inv_rigid(float *out, float *m)
{
    float tx = -m[12];
    float ty = -m[13];
    float tz = -m[14];

    {
        EhfVec4 r = { 0 };

        r.x = m[0];
        r.y = m[4];
        r.z = m[8];
        ((EhfVec4 *)out)[0] = r;
    }
    {
        EhfVec4 r = { 0 };

        r.x = m[1];
        r.y = m[5];
        r.z = m[9];
        ((EhfVec4 *)out)[1] = r;
    }
    {
        EhfVec4 r = { 0 };

        r.x = m[2];
        r.y = m[6];
        r.z = m[10];
        ((EhfVec4 *)out)[2] = r;
    }
    {
        EhfVec4 r = ehf_row3;

        r.x = m[0] * tx + m[1] * ty + m[2] * tz;
        r.y = m[4] * tx + m[5] * ty + m[6] * tz;
        r.z = m[8] * tx + m[9] * ty + m[10] * tz;
        ((EhfVec4 *)out)[3] = r;
    }
    return out;
}

float *vec3_rotate(float *out, float *m, float *v)
{
    float x = v[0];
    float y = v[1];
    float z = v[2];

    out[0] = m[0] * x + m[4] * y + m[8] * z;
    out[1] = m[1] * x + m[5] * y + m[9] * z;
    out[2] = m[2] * x + m[6] * y + m[10] * z;
    return out;
}

/* GE state for EHF drawing; pushes the projection and view matrices and computes the light
   position used by shapes. */
void *ehf_draw_begin(void *list, EhfInst *e, EhfOpts *o)
{
    float scale[3];
    float pos[3];
    float m[18];
    float inv[16];

    list = dl_disable(list, 0);
    list = dl_disable(list, 0x11);
    list = dl_disable(list, 7);
    list = dl_disable(list, 5);
    list = dl_disable(list, 0xF);
    list = dl_disable(list, 0x10);
    list = dl_disable(list, 0x12);
    list = dl_disable(list, 0x13);
    list = dl_disable(list, 0x14);
    list = dl_tex_func(list, 0, 1);
    list = dl_disable(list, 0x15);
    list = dl_disable(list, 8);
    list = dl_enable(list, 10);
    list = dl_disable(list, 0xB);
    list = dl_disable(list, 0xC);
    list = dl_disable(list, 0xD);
    list = dl_disable(list, 0xE);
    list = dl_specular_power(list, 0.0f);
    list = dl_tex_wrap(list, 1, 1);
    list = dl_material_colors(list, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000);
    list = dl_color_material(list, 7);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(list, 16.0f, 16.0f);
    if (o->flags & 0x400) {
        list = dl_set_tex_filter(list, 1, 1);
    } else {
        list = dl_set_tex_filter(list, 0, 0);
    }
    list = dl_enable(list, 4);
    if (o->flags & 0x100) {
        list = dl_blend_func(list, 0, 2, 10, 0xFFFFFFFF, 0xFFFFFFFF);
    } else {
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
    }
    list = dl_shade_model(list, 1);
    if (!(o->flags & 0x800)) {
        list = dl_disable(list, 3);
    }
    if (o->flags & 0x40) {
        list = dl_enable(list, 1);
    } else {
        list = dl_disable(list, 1);
    }
    mtx_set_mode(1);
    mtx_push();
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    scale[0] = ehf_light_scale[0];
    scale[1] = ehf_light_scale[1];
    scale[2] = ehf_light_scale[2];
    mtx_scale3(scale);
    mtx_get34(m);
    mat4_inv_rigid(inv, m);
    pos[0] = ehf_light_dir[0];
    pos[1] = ehf_light_dir[1];
    pos[2] = ehf_light_dir[2];
    vec3_rotate(ehf_light_pos, inv, pos);
    return mtx_commit(list);
}

void *ehf_draw_end(void *list, EhfInst *e, EhfOpts *o)
{
    mtx_set_mode(1);
    mtx_pop();
    list = mtx_commit(list);
    mtx_set_mode(2);
    mtx_pop();
    list = mtx_commit(list);
    return dl_tex_scale(list, 1.0f, 1.0f);
}

/* Runs the timeline of `c` for one frame: places and removes characters, then steps the nested
   sprites. */
int ehf_clip_step(EhfInst *e, EhfClip *c)
{
    EhfCmd *cmd;
    EhfClip *n;
    int *chr;
    char *tbl = 0;
    EhfClip *k;

    c->frame++;
    if (c->frame >= c->frames || (c == &e->root && c->frame >= e->limit)) {
        if (c == &e->root) {
            ehf_inst_reset(e, e->file, 1);
        } else {
            c->cmd = (EhfCmd *)((char *)c->chr + c->chr[2]);
            c->frame = 0;
            c->wait = 0;
            if (c->child != 0) {
                ehf_clip_free(c);
            }
        }
    }
    if (c->wait > 0) {
        c->wait--;
    } else {
        cmd = c->cmd;
        for (;;) {
            if (cmd >= c->cmd_end) {
                break;
            }
            if (cmd->chr == -1) {
                if (cmd->depth != 0xFFFF) {
                    c->child = ehf_list_remove(c->child, cmd->depth);
                }
            } else {
                n = ehf_clip_alloc(e);
                n->chr = (int *)((char *)e->file + e->hdr->chars + cmd->chr * 0x10);
                n->depth = cmd->depth;
                n->start = c->frame;
                n->clip = cmd->clip;
                chr = n->chr;
                switch (chr[0]) {
                case 0:
                    tbl = (char *)chr + chr[8];
                    break;
                case 1:
                    tbl = (char *)chr + chr[3];
                    break;
                case 3:
                    tbl = (char *)chr + chr[4];
                    n->cmd = (EhfCmd *)((char *)chr + chr[2]);
                    n->cmd_end = (EhfCmd *)((char *)n->cmd + chr[3]);
                    n->frame = 0xFFFF;
                    n->frames = chr[1];
                    n->wait = 0;
                    break;
                }
                n->track = (EhfTrack *)(tbl + ((int *)tbl)[cmd->track]);
                c->child = ehf_list_insert(c->child, n);
            }
            if (cmd->wait > 0) {
                c->wait = cmd->wait - 1;
                cmd++;
                break;
            }
            cmd++;
        }
        c->cmd = cmd;
    }
    k = c->child;
    if (k != 0) {
        do {
            if (k->chr[0] == 3) {
                ehf_clip_step(e, k);
            }
            k = k->next;
        } while (k != c->child);
    }
    return 1;
}

void ehf_nop(EhfInst *e, EhfClip *c)
{
}

#ifdef NON_MATCHING /* float scheduling */
/* A sprite marked as camera (chr[6]) holding a shape: replaces `out` with the view that maps the
   camera rectangle onto the instance. */
int ehf_camera(EhfInst *e, float *out)
{
    float cam[16];
    float m2[16];
    float m1[16];
    float inv[16];
    EhfColor cx;
    EhfClip *c;
    EhfClip *head;
    int *shape;
    float s;
    EhfMovie *mv;

    head = e->root.child;
    if (head == 0) {
        return 1;
    }
    c = head;
    do {
        if (c->chr[0] == 3 && c->chr[6] != 0 && c->child != 0 && c->child->child != 0) {
            shape = c->child->child->chr;
            if (shape[0] == 0) {
                ehf_track_eval(m1, &cx, 0, c->track, c->start, e->root.frame);
                ehf_track_eval(m2, &cx, 0, c->child->track, c->child->start, c->frame);
                mat4_mul(m1, m1, m2);
                mat4_inverse(inv, m1);
                mat4_identity(out);
                mat4_translate2(out, e->x, e->y);
                mat4_translate2(out, -(float)e->w * 0.5f, -(float)e->h * 0.5f);
                mv = e->movie;
                s = (float)shape[3] * 2048.0f;
                mat4_scale2(out, (float)e->w / s, (float)(mv->xmax - mv->xmin) / (float)(mv->ymax - mv->ymin) * (float)e->h / s);
                mat4_mul(out, out, inv);
                return 1;
            }
        }
        c = c->next;
    } while (c != head);
    (void)cam;
    return 1;
}
#endif

/* Combines the clip's color transform `cx` with the parent's `p` into `cx`. */
#define EHF_DIV256(x) ((x) / 256)

void *ehf_draw_list(void *list, EhfInst *e, EhfClip *clip, float *mtx, EhfColor *p, EhfOpts *o)
{
    EhfEval ev;
    float m2[16];
    float m[16];
    EhfColor cx;
    EhfClip *c;
    int masking = -1;
    int mr, mg, mb, ma;
    int ar, ag, ab, aa;

    c = clip->child;
    if (c == 0) {
        return list;
    }
    do {
        if (c->chr == 0) {
            goto next;
        }
        if (ehf_debug_range != 0 && (c->depth < ehf_debug_lo || c->depth > ehf_debug_hi)) {
            goto next;
        }
        if ((o->flags & 8) && !(o->mask & (1 << c->depth))) {
            goto next;
        }
        if (c->clip > 0) {
            list = dl_clear_depth(list, 0);
            list = dl_clear(list, 0x12);
            list = dl_stencil_op(list, 1, 0xFF, 0xFF);
            list = dl_stencil_func(list, 2, 2, 2);
            list = dl_enable(list, 3);
            list = dl_pixel_mask(list, 0xFFFFFF);
            masking = c->clip;
        } else if (c->depth < masking) {
            list = dl_stencil_op(list, 1, 0xFF, 0xFF);
            list = dl_stencil_op(list, 2, 0xFF, 0xFF);
            list = dl_stencil_func(list, 0, 2, 2);
            list = dl_enable(list, 3);
        } else if (masking > 0) {
            list = dl_disable(list, 3);
            masking = -1;
        }
        switch (c->chr[0]) {
        case 0:
            list = ehf_draw_shape(list, e, c, clip->frame, mtx, p, o);
            break;
        case 1:
            list = ehf_draw_text(list, e, c, clip->frame, mtx, p);
            break;
        case 3:
            if (c->chr[6] != 0) {
                break;
            }
            if (o->flags & 0x80) {
                ev.m = m;
                ev.cx = &cx;
                ev.parent = mtx;
                ev.ratio = 0;
                ev.track = c->track;
                ev.start = c->start;
                ev.frame = clip->frame;
                ev.fn = ehf_track_eval;
                if (o->eval_cb(c->depth, o->eval_arg, &ev) == 0) {
                    ehf_track_eval(m, &cx, 0, c->track, c->start, clip->frame);
                }
            } else {
                ehf_track_eval(m, &cx, 0, c->track, c->start, clip->frame);
            }
            mat4_mul(m2, mtx, m);
            mb = EHF_DIV256(cx.mul[2] * p->mul[2]);
            mg = EHF_DIV256(cx.mul[1] * p->mul[1]);
            ma = EHF_DIV256(cx.mul[3] * p->mul[3]);
            ab = cx.add[2] + EHF_DIV256(cx.mul[2] * p->add[2]);
            ag = cx.add[1] + EHF_DIV256(cx.mul[1] * p->add[1]);
            ar = cx.add[0] + EHF_DIV256(cx.mul[0] * p->add[0]);
            aa = cx.add[3] + EHF_DIV256(cx.mul[3] * p->add[3]);
            mr = EHF_DIV256(cx.mul[0] * p->mul[0]);
            cx.mul[0] = mr;
            cx.mul[1] = mg;
            cx.mul[2] = mb;
            cx.mul[3] = ma;
            cx.add[0] = ar;
            cx.add[1] = ag;
            cx.add[2] = ab;
            cx.add[3] = aa;
            list = ehf_draw_list(list, e, c, m2, &cx, o);
            break;
        default:
            return list;
        }
        if (c->clip > 0) {
            list = dl_disable(list, 3);
            list = dl_pixel_mask(list, 0);
        }
    next:
        c = c->next;
    } while (c != clip->child);
    if (masking >= 0) {
        list = dl_disable(list, 3);
    }
    return list;
}

#ifdef NON_MATCHING /* keyframe search layout and float scheduling differ */
/* Base matrix and color of a track at `frame`, interpolated between the surrounding keys. */
void ehf_track_eval(float *m, EhfColor *cx, int *ratio, EhfTrack *t, int start, int frame)
{
    unsigned short f;
    unsigned int size;
    unsigned char *lo;
    unsigned char *hi;
    unsigned char *k;
    unsigned char *pa;
    unsigned char *pb;
    int n;
    float w1;
    float w0;

    m[0] = t->m[0];
    m[1] = t->m[1];
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = t->m[2];
    m[5] = t->m[3];
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 1.0f;
    m[11] = 0.0f;
    m[12] = t->m[4];
    m[13] = t->m[5];
    m[14] = 0.0f;
    m[15] = 1.0f;
    *cx = ehf_color_identity;
    if (t->nkeys > 0) {
        f = frame - start;
        size = t->key_size;
        lo = t->keys;
        hi = lo + size * (t->nkeys - 1);
        k = lo;
        if (*(unsigned short *)lo < f && (k = hi, f < *(unsigned short *)hi)) {
            n = hi - lo;
            k = lo + size * ((n / (int)size) / 2);
            while ((int)size < n) {
                if (*(unsigned short *)k == f) {
                    goto exact;
                }
                if (*(unsigned short *)k < f) {
                    lo = k;
                } else {
                    hi = k;
                }
                n = hi - lo;
                k = lo + size * ((n / (int)size) / 2);
            }
        } else {
            goto exact;
        }
        pa = k + 2;
        pb = k + size + 2;
        w1 = ((float)f - (float)*(unsigned short *)k) /
             ((float)*(unsigned short *)(k + size) - (float)*(unsigned short *)k);
        w0 = 1.0f - w1;
        if (t->flags & 1) {
            m[12] += w0 * (((short *)pa)[0] * t->pos_scale) + w1 * (((short *)pb)[0] * t->pos_scale);
            m[13] += w0 * (((short *)pa)[1] * t->pos_scale) + w1 * (((short *)pb)[1] * t->pos_scale);
            pa += 4;
            pb += 4;
        }
        if (t->flags & 2) {
            m[0] += w0 * (((short *)pa)[0] * t->scale_scale) + w1 * (((short *)pb)[0] * t->scale_scale);
            m[5] += w0 * (((short *)pa)[1] * t->scale_scale) + w1 * (((short *)pb)[1] * t->scale_scale);
            pa += 4;
            pb += 4;
        }
        if (t->flags & 4) {
            m[1] += w0 * (((short *)pa)[0] * t->rot_scale) + w1 * (((short *)pb)[0] * t->rot_scale);
            m[4] += w0 * (((short *)pa)[1] * t->rot_scale) + w1 * (((short *)pb)[1] * t->rot_scale);
            pa += 4;
            pb += 4;
        }
        if (t->flags & 8) {
            cx->mul[0] = (signed char)pa[0] * 2.015748f * w0 + (signed char)pb[0] * 2.015748f * w1;
            cx->mul[1] = (signed char)pa[1] * 2.015748f * w0 + (signed char)pb[1] * 2.015748f * w1;
            cx->mul[2] = (signed char)pa[2] * 2.015748f * w0 + (signed char)pb[2] * 2.015748f * w1;
            cx->mul[3] = (signed char)pa[3] * 2.015748f * w0 + (signed char)pb[3] * 2.015748f * w1;
            pa += 4;
            pb += 4;
        }
        if (t->flags & 0x30) {
            cx->add[0] = pa[0] * w0 + pb[0] * w1;
            cx->add[1] = pa[1] * w0 + pb[1] * w1;
            cx->add[2] = pa[2] * w0 + pb[2] * w1;
            cx->add[3] = pa[3] * w0 + pb[3] * w1;
            pa += 4;
            pb += 4;
        }
        if (t->flags & 0x40) {
            if (ratio != 0) {
                *ratio = *(unsigned short *)pa * w0 + *(unsigned short *)pb * w1;
            }
        } else if (ratio != 0) {
            *ratio = -1;
        }
        return;
    exact:
        pa = k + 2;
        if (t->flags & 1) {
            m[12] += ((short *)pa)[0] * t->pos_scale;
            m[13] += ((short *)pa)[1] * t->pos_scale;
            pa += 4;
        }
        if (t->flags & 2) {
            m[0] += ((short *)pa)[0] * t->scale_scale;
            m[5] += ((short *)pa)[1] * t->scale_scale;
            pa += 4;
        }
        if (t->flags & 4) {
            m[1] += ((short *)pa)[0] * t->rot_scale;
            m[4] += ((short *)pa)[1] * t->rot_scale;
            pa += 4;
        }
        if (t->flags & 8) {
            cx->mul[0] = (signed char)pa[0] * 0x100 / 0x7F;
            cx->mul[1] = (signed char)pa[1] * 0x100 / 0x7F;
            cx->mul[2] = (signed char)pa[2] * 0x100 / 0x7F;
            cx->mul[3] = (signed char)pa[3] * 0x100 / 0x7F;
            pa += 4;
        }
        if (t->flags & 0x30) {
            cx->add[0] = pa[0];
            cx->add[1] = pa[1];
            cx->add[2] = pa[2];
            cx->add[3] = pa[3];
            pa += 4;
        }
        if (t->flags & 0x40) {
            if (ratio != 0) {
                *ratio = *(unsigned short *)pa;
            }
        } else if (ratio != 0) {
            *ratio = -1;
        }
    }
}
#endif

#define CLAMP255(v) do { if ((v) < 0) (v) = 0; if ((v) > 0xFF) (v) = 0xFF; } while (0)

typedef struct EhfMesh {               /* 0x18 per mesh of a shape */
    unsigned int color;                /* +0x00 RGB, alpha in the low byte of the next word */
    unsigned int alpha;                /* +0x04 */
    unsigned int vtype;                /* +0x08 */
    unsigned int tex;                  /* +0x0C texture offset in the table, 0 untextured */
    int verts;                         /* +0x10 */
    unsigned int prim;                 /* +0x14 prim << 16 | count */
} EhfMesh;

#ifdef NON_MATCHING /* the original keeps values in caller-saved registers across ehf_track_eval */
/* A shape: color transform as ambient light, matrix, optional morph between two vertex sets,
   then each mesh with its texture, material color and lights. */
void *ehf_draw_shape(void *list, EhfInst *e, EhfClip *c, unsigned int frame, float *mtx, EhfColor *p, EhfOpts *o)
{
    EhfColor cx;
    float m[16];
    float lpos[3];
    float zero[3];
    EhfEval ev;
    int ratio;
    int *shape = c->chr;
    int r, g, b, a;
    EhfMesh *mh;
    EhfMesh *end;
    unsigned int cur;
    EhfTex *tex;
    unsigned int col;
    float w;

    ratio = -1;
    ehf_track_eval(m, &cx, &ratio, c->track, c->start, frame);
    if (o->flags & 0x80) {
        ev.m = m;
        ev.cx = &cx;
        ev.track = c->track;
        ev.start = c->start;
        ev.fn = ehf_track_eval;
        ev.parent = mtx;
        ev.ratio = &ratio;
        ev.frame = frame;
        if (o->eval_cb(c->depth, o->eval_arg, &ev) == 0) {
            ehf_track_eval(m, &cx, &ratio, c->track, c->start, frame);
        }
    }
    b = EHF_DIV256(cx.mul[2] * p->mul[2]);
    g = EHF_DIV256(cx.mul[1] * p->mul[1]);
    a = EHF_DIV256(cx.mul[3] * p->mul[3]);
    cx.add[2] += EHF_DIV256(cx.mul[2] * p->add[2]);
    cx.add[1] += EHF_DIV256(cx.mul[1] * p->add[1]);
    cx.add[0] += EHF_DIV256(cx.mul[0] * p->add[0]);
    cx.add[3] += EHF_DIV256(cx.mul[3] * p->add[3]);
    r = EHF_DIV256(cx.mul[0] * p->mul[0]);
    if (a < 0) {
        a = 0;
    }
    if (b < 0) {
        b = 0;
    }
    if (g < 0) {
        g = 0;
    }
    if (g > 0xFF) {
        g = 0xFF;
    }
    if (a > 0xFF) {
        a = 0xFF;
    }
    if (b > 0xFF) {
        b = 0xFF;
    }
    if (r < 0) {
        r = 0;
    }
    if (r > 0xFF) {
        r = 0xFF;
    }
    cx.mul[0] = r;
    cx.mul[1] = g;
    cx.mul[2] = b;
    cx.mul[3] = a;
    list = dl_ambient_light(list, r + g * 0x100 + a * 0x1000000 + b * 0x10000);
    mat4_translate2(m, ((float *)shape)[1], ((float *)shape)[2]);
    mat4_scale2(m, ((float *)shape)[3], ((float *)shape)[3]);
    mat4_mul(m, mtx, m);
    list = dl_set_matrix(list, 1, m);
    if (ratio >= 0) {
        w = (float)ratio;
        list = dl_morph_weight(1.0f - w * (1.0f / 65535.0f), list, 0);
        list = dl_morph_weight(w * (1.0f / 65535.0f), list, 1);
    }
    cur = 0xFFFFFFFF;
    if (o->flags & 0x100) {
        list = dl_blend_func(list, 0, 2, 10, 0xFFFFFFFF, 0xFFFFFFFF);
    } else {
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
    }
    mh = (EhfMesh *)((char *)shape + shape[4]);
    end = (EhfMesh *)((char *)mh + shape[5]);
    for (; mh < end; mh++) {
        if (mh->tex == 0) {
            cur = 0xFFFFFFFF;
            list = dl_disable(list, 9);
        } else if (cur != (mh->tex & 0xFFFF)) {
            list = dl_enable(list, 9);
            cur = (unsigned short)mh->tex;
            tex = (EhfTex *)((char *)e->file + e->hdr->tex + cur);
            list = dl_tex_func(list, 0, 1);
            if (o->tex_cb != 0) {
                list = o->tex_cb(list, o->tex_arg, tex, cur);
            } else {
                list = gim_set_texture(list, tex->gim);
            }
            if (!(o->flags & 0x100) && !(tex->flags & 1)) {
                list = dl_blend_func(list, 0, 2, 3, 0, 0);
            } else {
                list = dl_blend_func(list, 0, 2, 10, 0xFFFFFFFF, 0xFFFFFFFF);
            }
            if (!(tex->flags & 0x10) && !(o->flags & 0x400)) {
                list = dl_set_tex_filter(list, 0, 0);
            } else {
                list = dl_set_tex_filter(list, 1, 1);
            }
        }
        CLAMP255(cx.add[2]);
        CLAMP255(cx.add[0]);
        CLAMP255(cx.add[1]);
        CLAMP255(cx.add[3]);
        col = cx.add[0] + cx.add[1] * 0x100 + cx.add[3] * 0x1000000 + cx.add[2] * 0x10000;
        list = dl_material_colors(list, col, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
        list = dl_material(list, 1, (unsigned char)mh->alpha * 0x1000000 + (mh->color & 0xFFFFFF));
        zero[0] = 0.0f;
        zero[1] = 0.0f;
        zero[2] = 0.0f;
        list = dl_light_color(list, 0, 2, col);
        list = dl_light_color(list, 0, 4, col);
        list = dl_light(list, 0, 0, 6, zero);
        lpos[0] = ehf_light_pos[0];
        lpos[1] = ehf_light_pos[1] * -1.0f;
        lpos[2] = ehf_light_pos[2];
        list = dl_light_color(list, 1, 2, col);
        list = dl_light_color(list, 1, 4, col);
        list = dl_light(list, 1, 0, 6, lpos);
        list = dl_enable(list, 0xB);
        list = dl_light_mode(list, 1);
        list = dl_draw_array(list, (mh->prim & 0x7FFFF) >> 16, mh->vtype & 0xFFFFFF, mh->prim & 0xFFFF, 0,
                             (char *)shape + shape[6] + mh->verts);
    }
    return list;
}
#endif

typedef struct EhfGlyph {              /* 0x14 per glyph of a text */
    float x, y;
    float scale;
    unsigned int color;                /* +0x0C */
    int chr;                           /* +0x10 offset of the glyph shape in the character blob */
} EhfGlyph;

#ifdef NON_MATCHING /* the original keeps the text character in $v0 across ehf_track_eval */
/* A text: one glyph shape per character, placed by its own offset and scale. */
void *ehf_draw_text(void *list, EhfInst *e, EhfClip *c, unsigned int frame, float *mtx, EhfColor *p)
{
    int *txt = c->chr;
    EhfGlyph *gl = (EhfGlyph *)((char *)txt + txt[1]);
    EhfGlyph *end = (EhfGlyph *)((char *)gl + txt[2]);
    float base[16];
    float g2[16];
    float m[16];
    EhfColor cx;
    int mr, mg, mb, ma;
    int ar, ag, ab, aa;
    float *r;
    int *shape;
    EhfMesh *mh;
    EhfMesh *mend;

    ehf_track_eval(m, &cx, 0, c->track, c->start, frame);
    r = (float *)txt;
    mb = EHF_DIV256(cx.mul[2] * p->mul[2]);
    mg = EHF_DIV256(cx.mul[1] * p->mul[1]);
    ma = EHF_DIV256(cx.mul[3] * p->mul[3]);
    ab = cx.add[2] + EHF_DIV256(cx.mul[2] * p->add[2]);
    ag = cx.add[1] + EHF_DIV256(cx.mul[1] * p->add[1]);
    ar = cx.add[0] + EHF_DIV256(cx.mul[0] * p->add[0]);
    aa = cx.add[3] + EHF_DIV256(cx.mul[3] * p->add[3]);
    mr = EHF_DIV256(cx.mul[0] * p->mul[0]);
    cx.mul[0] = mr;
    cx.mul[1] = mg;
    cx.mul[2] = mb;
    cx.mul[3] = ma;
    cx.add[0] = ar;
    cx.add[1] = ag;
    cx.add[2] = ab;
    cx.add[3] = aa;
    base[0] = r[5];
    base[1] = r[6];
    base[2] = 0.0f;
    base[3] = 0.0f;
    base[4] = r[7];
    base[5] = r[8];
    base[6] = 0.0f;
    base[7] = 0.0f;
    base[8] = 0.0f;
    base[9] = 0.0f;
    base[10] = 1.0f;
    base[11] = 0.0f;
    base[12] = r[9];
    base[13] = r[10];
    base[14] = 0.0f;
    base[15] = 1.0f;
    mat4_mul(m, m, base);
    mat4_mul(m, mtx, m);
    for (; gl < end; gl++) {
        mat4_translate2(g2, gl->x, gl->y);
        mat4_scale2(g2, gl->scale, gl->scale);
        shape = (int *)((char *)e->file + e->hdr->chars + gl->chr);
        mat4_translate2(g2, ((float *)shape)[1], ((float *)shape)[2]);
        list = dl_set_matrix(list, 1, g2);
        list = dl_material(list, 1, gl->color);
        mh = (EhfMesh *)((char *)shape + 0x20);
        mend = (EhfMesh *)((char *)mh + shape[3]);
        for (; mh < mend; mh++) {
            list = dl_draw_array(list, (mh->prim & 0x7FFFF) >> 16, mh->vtype & 0xFFFFFF, mh->prim & 0xFFFF, 0,
                                 (char *)shape + shape[4] + mh->verts);
        }
    }
    return list;
}
#endif

/* Rewinds `e` to frame 0 of `f` and replays up to the target frame (`keep`: called from the
   timeline at the end of the movie). */
int ehf_inst_reset(EhfInst *e, EhfFile *f, int keep)
{
    EhfClip *root;
    char *cmds;

    e->file = f;
    e->hdr = &f->h;
    e->movie = (EhfMovie *)((char *)f + f->h.movie);
    e->wait = 0;
    e->root.frame = 0;
    root = &e->root;
    cmds = (char *)f + e->hdr->cmds;
    e->root.cmd = (EhfCmd *)cmds;
    e->root.cmd_end = (EhfCmd *)(cmds + e->hdr->cmds_size);
    e->root.wait = 0;
    e->root.child = 0;
    e->root.frames = e->movie->frames;
    memset(root + 1, 0, e->hdr->inst_size - 0x50);
    if (keep) {
        while (root->frame != e->target) {
            root->frame--;
            ehf_clip_step(e, root);
            root->frame++;
        }
    } else {
        root->frame--;
        ehf_clip_step(e, root);
        while (root->frame != e->target) {
            ehf_clip_step(e, root);
        }
    }
    e->ready = 1;
    return 1;
}

EhfClip *ehf_clip_alloc(EhfInst *e)
{
    EhfClip *last = (EhfClip *)((char *)e + e->hdr->inst_size) - 1;
    EhfClip *c;

    for (c = e->pool; c <= last; c++) {
        if (c->chr == 0) {
            c->child = 0;
            return c;
        }
    }
    return 0;
}

/* Inserts `c` in the depth-sorted list `head` (replacing a clip at the same depth). */
EhfClip *ehf_list_insert(EhfClip *head, EhfClip *c)
{
    EhfClip *k;

    if (head == 0) {
        c->prev = c;
        c->next = c;
        return c;
    }
    k = head;
    for (;;) {
        if (k->depth == c->depth) {
            if (k->child != 0) {
                ehf_clip_free(k);
            }
            if (k->next == k) {
                k->chr = 0;
                c->prev = c;
                c->next = c;
                return c;
            }
            k->chr = 0;
            c->prev = k->prev;
            c->next = k->next;
            c->prev->next = c;
            c->next->prev = c;
            if (k == head) {
                return c;
            }
            return head;
        }
        if (c->depth < k->depth) {
            c->prev = k->prev;
            c->next = k;
            c->prev->next = c;
            c->next->prev = c;
            if (k == head) {
                return c;
            }
            return head;
        }
        k = k->next;
        if (k == head) {
            c->next = head;
            (c->prev = head->prev)->next = c;
            c->next->prev = c;
            return head;
        }
    }
}

EhfClip *ehf_list_remove(EhfClip *head, unsigned int depth)
{
    EhfClip *k = head;

    do {
        if (k->depth == depth) {
            goto found;
        }
        k = k->next;
    } while (k != head);
    return head;
found:
    k->chr = 0;
    if (k->child != 0) {
        ehf_clip_free(k);
    }
    if (k->prev == k) {
        return 0;
    }
    k->prev->next = k->next;
    k->next->prev = k->prev;
    if (k == head) {
        head = k->next;
    }
    return head;
}

void ehf_clip_free(EhfClip *c)
{
    EhfClip *k = c->child;

    do {
        if (k->child != 0) {
            ehf_clip_free(k);
        }
        k->chr = 0;
        k = k->next;
    } while (k != c->child);
    c->child = 0;
}

void mat4_scale2(float *m, float sx, float sy)
{
    float s[16];

    mat4_identity(s);
    s[0] = sx;
    s[5] = sy;
    mat4_mul(m, m, s);
}

void mat4_translate2(float *m, float x, float y)
{
    float t[16];

    mat4_identity(t);
    t[12] = x;
    t[13] = y;
    mat4_mul(m, m, t);
}

void mat4_rotate_z(float *m, int angle)
{
    float r[16];
    float a;
    float c;

    mat4_identity(r);
    a = angle * 3.1415927f / 32768.0f;
    c = cosf_vfpu(a);
    r[4] = sinf_vfpu(a);
    r[1] = -r[4];
    r[0] = c;
    r[5] = c;
    mat4_mul(m, m, r);
}

#ifdef NON_MATCHING /* float scheduling */
float mat4_det(float *m)
{
    float m4 = m[4], m5 = m[5], m6 = m[6], m7 = m[7];
    float m8 = m[8], m9 = m[9], m10 = m[10], m11 = m[11];
    float m12 = m[12], m13 = m[13], m14 = m[14], m15 = m[15];

    return m[0] * (m14 * m7 * m9 + m15 * m5 * m10 + m13 * m6 * m11 - m13 * m7 * m10 - m15 * m6 * m9 - m14 * m5 * m11)
         - m[1] * (m14 * m7 * m8 + m15 * m4 * m10 + m12 * m6 * m11 - m12 * m7 * m10 - m15 * m6 * m8 - m14 * m4 * m11)
         + m[2] * (m13 * m7 * m8 + m15 * m4 * m9 + m12 * m5 * m11 - m12 * m7 * m9 - m15 * m5 * m8 - m13 * m4 * m11)
         - m[3] * (m13 * m6 * m8 + m14 * m4 * m9 + m12 * m5 * m10 - m12 * m6 * m9 - m14 * m5 * m8 - m13 * m4 * m10);
}
#endif

#ifdef NON_MATCHING /* float scheduling */
void mat4_inverse(float *out, float *m)
{
    float m0 = m[0], m1 = m[1], m2 = m[2], m3 = m[3];
    float m4 = m[4], m5 = m[5], m6 = m[6], m7 = m[7];
    float m8 = m[8], m9 = m[9], m10 = m[10], m11 = m[11];
    float m12 = m[12], m13 = m[13], m14 = m[14], m15 = m[15];
    float a = m6 * m11 - m7 * m10;
    float b = m6 * m15 - m7 * m14;
    float c = m10 * m15 - m11 * m14;
    float d = m2 * m11 - m3 * m10;
    float f = m2 * m15 - m3 * m14;
    float g = m2 * m7 - m3 * m6;
    float h = m4 * m9 - m5 * m8;
    float i = m4 * m13 - m5 * m12;
    float j = m8 * m13 - m9 * m12;
    float k = m0 * m9 - m1 * m8;
    float l = m0 * m13 - m1 * m12;
    float n = m0 * m5 - m1 * m4;
    float det = 1.0f / mat4_det(m);
    float ndet = -det;

    out[0] = det * (m13 * a + (m5 * c - m9 * b));
    out[4] = ndet * (m12 * a + (m4 * c - m8 * b));
    out[8] = det * (m15 * h + (m7 * j - m11 * i));
    out[12] = ndet * (m14 * h + (m6 * j - m10 * i));
    out[1] = ndet * (m13 * d + (m1 * c - m9 * f));
    out[5] = det * (m12 * d + (m0 * c - m8 * f));
    out[9] = ndet * (m15 * k + (m3 * j - m11 * l));
    out[13] = det * (m14 * k + (m2 * j - m10 * l));
    out[2] = det * (m13 * g + (m1 * b - m5 * f));
    out[6] = ndet * (m12 * g + (m0 * b - m4 * f));
    out[10] = det * (m15 * n + (m3 * i - m7 * l));
    out[14] = ndet * (m14 * n + (m2 * i - m6 * l));
    out[3] = ndet * (m9 * g + (m1 * a - m5 * d));
    out[7] = det * (m8 * g + (m0 * a - m4 * d));
    out[11] = ndet * (m11 * n + (m3 * h - m7 * k));
    out[15] = det * (m10 * n + (m2 * h - m6 * k));
}
#endif
