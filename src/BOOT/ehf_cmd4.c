// @range 0x088243E4-0x088253FC
// @asm 0x088243E4-0x088245CC
// @asm 0x08825094-0x088252EC
// @data 0x088ED284-0x088ED380
/* EHF command handlers, part 4: an axis billboard turned towards a fixed up vector, points carried
   by a bone, trails (a ring of positions, one or two channels, optionally smoothed by a cyclic
   spline), interpolation between keyframes and callbacks. The constants are named globals because
   the first function stays in asm (see docs/build.md). */
#include "psp.h"
#include "libc.h"
#include "allegrex.h"
#include "ehfcmd.h"

typedef struct EhfTrailCh {
    int max;                           /* capacity */
    int n;                             /* points stored */
    int start;                         /* oldest point when the ring is full */
    int head;                          /* next slot */
    Vec4 *pts;                         /* +0x10 */
    Vec4 *d;                           /* +0x14 spline terms, or 0 */
    unsigned char kind;                /* +0x18 1 or 2 channels */
    unsigned char spline : 1;          /* +0x19 */
    unsigned char ring : 1;
    unsigned char pad1a[6];
} EhfTrailCh;

typedef struct EhfTrail {              /* state of command 0x10301, followed by the buffers */
    EhfTrailCh a;
    EhfTrailCh b;                      /* +0x20 second channel (kind 2) */
} EhfTrail;

typedef struct EhfKey {
    int frame;
    float v[4];
} EhfKey;

unsigned char ehf_fmt_ii2[4] = "II";
Vec4 ehf_up = { 0.0f, 1.0f, 0.0f, 1.0f };
Mat4 ehf_billboard_basis2 = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
unsigned char ehf_fmt_nnn[4] = "nnn";
char ehf_fmt_ffff[8] = "FFff";

extern EhfTrail *ehf_trail;            /* 0x088FA004 trail of the command being drawn */

int spline_init_ring(Vec4 *d, Vec4 *p, int start, int ring, int n);
Vec4 *vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_sub_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_scale_w(Vec4 *out, Vec4 *a, float s);
void vec4_lerp(float t, Vec4 *out, Vec4 *a, Vec4 *b);

#ifdef NON_MATCHING /* same matrix initializer as ehf_billboard_axis */
/* 0x10013: billboard at `pos` whose y axis is `dir`, its z axis facing the up vector. */
int ehf_cmd_billboard_up(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 x;
    Vec4 y;
    Vec4 z;
    Vec4 pos;
    Vec4 dir;
    Mat4 cur;
    void *out[2] = { &pos, &dir };
    char fmt[3] = "II";

    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, fmt);
    mtx_get34(&cur);
    mat4_transform_vec4(&pos, &cur, &pos);
    vec3_normalize2(&dir, &dir);
    z = ehf_up;
    vec3_cross(&x, &dir, &z);
    vec3_normalize2(&x, &x);
    vec3_cross(&y, &x, &dir);
    vec3_normalize2(&y, &y);
    {
        Mat4 m = { x.x, x.y, x.z, 0.0f, y.x, y.y, y.z, 0.0f, dir.x, dir.y, dir.z, 0.0f, pos.x, pos.y, pos.z, 1.0f };

        cur = m;
    }
    mtx_push();
    mtx_load(&cur);
    return 2;
}
#endif

/* 0x10300: moves the points arg0 and arg1 by bone arg3 of the model arg2. */
int ehf_cmd_bone_points(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfArg *a = (EhfArg *)cmd->a.i;
    Mat4 m;

    switch (phase) {
    case 5:
        model_get_bone(a[2].p, &m, a[3].i);
        mat4_transform_vec4(a[0].p, &m, a[0].p);
        mat4_transform_vec4(a[1].p, &m, a[1].p);
        return 0;
    }
    return 0;
}

/* 0x10301: trail of arg0 points with arg1 channels (arg2 ring, arg3 spline); its state and buffers
   are the instance memory. While drawing the children it is the current trail. */
int ehf_cmd_trail(EhfTrail *t, EhfNode *node, EhfCmd *cmd, int phase)
{
    int *a = cmd->a.i;
    int factor;
    int size;

    switch (phase) {
    default:
        return 0;
    case 1:
        if (a[3] != 0) {
            factor = 1;
        } else {
            factor = 2;
        }
        return (a[1] << 5) + factor * (a[1] * (a[0] << 4));
    case 2:
        factor = a[3] != 0 ? 1 : 2;
        size = (a[1] << 5) + factor * (a[1] * (a[0] << 4));
        t->a.max = a[0];
        t->a.n = 0;
        t->a.start = 0;
        t->a.head = 0;
        t->a.pts = (Vec4 *)((char *)t + (a[1] << 5));
        if (a[3] != 0) {
            t->a.d = t->a.pts + a[0];
        } else {
            t->a.d = 0;
        }
        t->a.kind = a[1];
        t->a.spline = a[3];
        t->a.ring = a[2];
        if (a[1] == 2) {
            t->b = t->a;
            if (a[3] != 0) {
                t->b.pts = t->a.d + a[0];
                t->b.d = t->b.pts + a[0];
            } else {
                t->b.pts = t->a.pts + a[0];
                t->b.d = 0;
            }
        }
        return size;
    case 5:
        ehf_trail = t;
        return 0x80000000;
    case 6:
        ehf_trail = 0;
        return 0;
    }
}

/* 0x10302: appends the points arg0 (and arg1) to the current trail, every frame the node moved. */
int ehf_cmd_trail_add(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfTrail *t = ehf_trail;
    void **a;
    unsigned char ring;

    switch (phase) {
    case 5:
        if (node->unk4 == 0 || ((ring = t->a.ring) == 0 && t->a.n >= t->a.max)) {
            return 0;
        }
        a = cmd->a.p;
        if (ring && t->a.n >= t->a.max) {
            t->a.start = (t->a.start + 1) % t->a.max;
        }
        t->a.pts[t->a.head] = *(Vec4 *)a[0];
        if (t->a.spline) {
            spline_init_ring(t->a.d, t->a.pts, t->a.start, t->a.max, t->a.n + 1);
        }
        if (t->a.kind == 2) {
            t->b.pts[t->a.head] = *(Vec4 *)a[1];
            if (t->a.spline) {
                spline_init_ring(t->b.d, t->b.pts, t->a.start, t->a.max, t->a.n + 1);
            }
        }
        t->a.head = (t->a.head + 1) % t->a.max;
        t->a.n = ALLEGREX_MIN(t->a.n + 1, t->a.max);
        return 0;
    }
    return 0;
}

/* Point `idx` of the current trail (from the oldest; negative counts back from the newest). */
int ehf_trail_get(int idx, Vec4 *p0, Vec4 *p1)
{
    EhfTrail *t = ehf_trail;

    if (t->a.ring && t->a.n >= t->a.max) {
        if (idx >= 0) {
            idx = (idx + t->a.start) % t->a.max;
        } else {
            idx += t->a.head;
            if (idx < 0) {
                idx += t->a.max;
                if (idx < t->a.start) {
                    idx = t->a.start;
                }
            }
        }
    } else if (idx < 0) {
        idx += t->a.n;
    }
    if (p0 != 0) {
        *p0 = t->a.pts[idx];
    }
    if (p1 != 0 && t->a.kind == 2) {
        *p1 = t->b.pts[idx];
    }
    return 0;
}

/* 0x10303: reads point arg0 into *arg1 (and *arg2). */
int ehf_cmd_trail_get(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfTrail *t = ehf_trail;
    int *a = cmd->a.i;

    if (phase != 5) {
        return 0;
    }
    if (t->a.kind == 1) {
        return ehf_trail_get(a[0], (Vec4 *)a[1], 0);
    }
    return ehf_trail_get(a[0], (Vec4 *)a[1], (Vec4 *)a[2]);
}

/* 0x10304: same with typed arguments. */
int ehf_cmd_trail_get_args(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfTrail *t = ehf_trail;
    Vec4 *p1;
    Vec4 *p0;
    int idx;
    void *out[3] = { &idx, &p0, &p1 };
    unsigned char fmt[4];
    char *f;

    f = (char *)fmt;
    fmt[0] = ehf_fmt_nnn[0];
    fmt[1] = ehf_fmt_nnn[1];
    fmt[2] = ehf_fmt_nnn[2];
    fmt[3] = ehf_fmt_nnn[3];
    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, f);
    if (t->a.kind == 1) {
        return ehf_trail_get(idx, p0, 0);
    }
    return ehf_trail_get(idx, p0, p1);
}

/* Appends p0/p1 to the current two-channel trail, then pushes every pair apart along its
   direction: p0 by w0, p1 by w1. */
int ehf_trail_push2(Vec4 *p0, Vec4 *p1, float w0, float w1)
{
    EhfTrail *t = ehf_trail;
    Vec4 d0;
    Vec4 d1;
    int i;
    int k;

    if (t->a.ring || t->a.n < t->a.max) {
        if (t->a.ring && t->a.n >= t->a.max) {
            t->a.start = (t->a.start + 1) % t->a.max;
        }
        t->a.pts[t->a.head] = *p0;
        t->b.pts[t->a.head] = *p1;
        t->a.head = (t->a.head + 1) % t->a.max;
        t->a.n = ALLEGREX_MIN(t->a.n + 1, t->a.max);
    }
    for (i = 0; i < t->a.n; i++) {
        k = (t->a.start + i) % t->a.max;
        vec3_sub_w(&d0, &t->a.pts[k], &t->b.pts[k]);
        vec3_normalize2(&d0, &d0);
        vec3_scale_w(&d1, &d0, w1);
        vec3_scale_w(&d0, &d0, w0);
        vec3_add_w(&t->a.pts[k], &t->a.pts[k], &d0);
        vec3_sub_w(&t->b.pts[k], &t->b.pts[k], &d1);
    }
    if (t->a.spline) {
        spline_init_ring(t->a.d, t->a.pts, t->a.start, t->a.max, t->a.n);
        spline_init_ring(t->b.d, t->b.pts, t->a.start, t->a.max, t->a.n);
    }
    return 0;
}

/* 0x10305: ehf_trail_push2(arg2, arg3, arg0, arg1) every frame the node moved. */
int ehf_cmd_trail_push2(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfArg *a = (EhfArg *)cmd->a.i;

    if (phase != 5) {
        return 0;
    }
    if (node->unk4 == 0) {
        return 0;
    }
    return ehf_trail_push2(a[0].p, a[1].p, a[2].f, a[3].f);
}

/* 0x10306: same with typed arguments. */
int ehf_cmd_trail_push2_args(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 w;
    Vec4 p1;
    Vec4 p0;
    void *out[4] = { &p0, &p1, &w.x, &w.y };
    char fmt[5];
    char *s;
    char *d;
    int n;

    s = ehf_fmt_ffff;
    d = fmt;
    n = 5;
    do {
        n--;
        *d++ = *s++;
    } while (n > 0);
    if (phase != 5) {
        return 0;
    }
    if (node->unk4 == 0) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, fmt);
    return ehf_trail_push2(&p0, &p1, w.x, w.y);
}

/* 0x10310: stores the vector arg1..arg4 into *arg0. */
int ehf_cmd_set_vec(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfArg *a = (EhfArg *)cmd->a.i;

    switch (phase) {
    case 1:
    case 2:
    case 5: {
        Vec4 v = { a[1].f, a[2].f, a[3].f, a[4].f };

        *(Vec4 *)a[0].p = v;
        return 0;
    }
    }
    return 0;
}

/* 0x10311: converts the argument 1 into *arg0. */
int ehf_cmd_set_arg(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 1:
    case 2:
    case 5: {
        void *v = cmd->a.p[0];
        void *out[1] = { v };
        char fmt[2] = "F";

        ehf_cmd_args(0, cmd, 1, out, fmt);
        return 0;
    }
    }
    return 0;
}

/* Axis gizmo of ehf_cmd_axis (ehf_prim.c): two sets of three vertices (vertex type 0x1E0). */
unsigned int ehf_axis_vtx[0x90 / 4] = {
    0x00000000, 0x00000000, 0x3F800000, 0x00000000, 0x3F5DB3D7, 0x00000000,
    0x00000000, 0x00000000, 0x3F800000, 0xBF000000, 0xBF5DB3D7, 0x00000000,
    0x00000000, 0x00000000, 0x3F800000, 0x3F000000, 0xBF5DB3D7, 0x00000000,
    0x00000000, 0x00000000, 0x3F800000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x3F800000, 0xBF000000, 0xBFDDB3D7, 0x00000000,
    0x00000000, 0x00000000, 0x3F800000, 0x3F000000, 0xBFDDB3D7, 0x00000000,
};

#ifdef NON_MATCHING /* lógica conferida; forma com a zeragem byte a byte dos Vec4 temporários (EHF_KEY_VEC) e as posições de pilha do original; 102 palavras: registradores e o load antecipado de k->frame no laço */
/* 0x10312: *arg0 interpolated between the keys (frame, vector) arg1, arg6, ... at the node frame. */
/* a Vec4 built from a key: zeroed byte by byte, filled, then copied (as the original) */
#define EHF_KEY_VEC(dst, k)                                                                   \
    {                                                                                         \
        Vec4 kv_;                                                                             \
        unsigned char *kz_ = (unsigned char *)&kv_;                                           \
        int kn_ = sizeof(Vec4);                                                               \
                                                                                              \
        if (kz_)                                                                              \
            do {                                                                              \
                *kz_++ = 0;                                                                   \
            } while (--kn_);                                                                  \
        kv_.x = (k)->v[0];                                                                    \
        kv_.y = (k)->v[1];                                                                    \
        kv_.z = (k)->v[2];                                                                    \
        kv_.w = (k)->v[3];                                                                    \
        dst = kv_;                                                                            \
    }

int ehf_cmd_keys(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 a0;
    Vec4 a1;
    Vec4 *out = cmd->a.p[0];
    EhfKey *k0 = cmd->a.p[1];
    int f;
    int n;
    int i;
    void **p;

    if (phase != 5) {
        return 0;
    }
    f = node->frame;
    n = (cmd->argc - 1) / 5;
    if (k0->frame < f) {
        EHF_KEY_VEC(*out, k0);
        return 0;
    }
    for (i = 1, p = &cmd->a.p[5]; i < n; i++, p += 5) {
        EhfKey *k1 = p[1];
        EhfKey *k = p[-4];

        if (f < k1->frame) {

            EHF_KEY_VEC(a0, k);
            EHF_KEY_VEC(a1, k1);
            vec4_lerp((float)(f - k->frame) / (float)(k1->frame - k->frame), out, &a0, &a1);
            return 0;
        }
    }
    {
        EhfKey *k = cmd->a.p[n * 5 - 4];

        EHF_KEY_VEC(*out, k);
    }
    return 0;
}
#endif

/* 0x10313: calls the function set by message 0x110 with the node's argument. */
int ehf_cmd_callback(void (**st)(int), int *msg, EhfCmd *cmd, int phase)
{
    switch (phase) {
    default:
        return 0;
    case 1:
        return 4;
    case 2:
        *st = 0;
        return 4;
    case 4:
        if (*st != 0) {
            (*st)(msg[3]);
        }
        return 0;
    case 0x110:
        *st = (void (*)(int))msg[0];
        return 0;
    }
}

typedef struct EhfCallback2 {
    void (*fn)(int, void *);
    void *arg;
} EhfCallback2;

/* 0x10314: same with a user pointer. */
int ehf_cmd_callback2(EhfCallback2 *st, int *msg, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 1:
        return 8;
    case 2:
        st->fn = 0;
        st->arg = 0;
        return 8;
    case 4:
        if (st->fn != 0) {
            st->fn(msg[3], st->arg);
        }
        return 0;
    case 0x110: {
        EhfCallback2 *src = (EhfCallback2 *)msg[0];

        st->fn = src->fn;
        st->arg = src->arg;
        return 0;
    }
    }
    return 0;
}
