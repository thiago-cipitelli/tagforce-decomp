// @range 0x0882D9F0-0x0882ECAC
// @asm 0x0882DE34-0x0882E408
// @asm 0x0882E89C-0x0882ECAC
// @data 0x088ED860-0x088ED900
/* TGMT skeletal animation: bones walked depth first from the root of the TGMS hierarchy, each
   one the parent matrix times the keyed translation, the slerped rotation and the keyed scale;
   cross-fade between two animations; Bezier float keys and quaternion keys (float or 16-bit). */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct MatF {                  /* Mat4 copied float by float */
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float x, y, z, w;
} __attribute__((aligned(16))) MatF;

typedef struct AnimKey {               /* 0x10 Bezier key */
    unsigned int t;
    float v;                           /* +0x04 */
    float out;                         /* +0x08 control after */
    float in;                          /* +0x0C control before the next key */
} AnimKey;

typedef struct AnimTrack {             /* 8 */
    int n;
    AnimKey *keys;                     /* +0x04 */
} AnimTrack;

typedef struct RotTrack {              /* 8 */
    int n;
    int first;                         /* +0x04 first key in the time and value arrays */
} RotTrack;

typedef struct Tgmt {
    unsigned int magic;                /* "TGMT" */
    unsigned short (*chan)[4];         /* +0x04 per bone: translation, rotation, scale tracks */
    AnimTrack *tracks;                 /* +0x08 */
    AnimKey *keys;                     /* +0x0C */
    RotTrack *rtracks;                 /* +0x10 */
    unsigned int *rtimes;              /* +0x14 */
    void *rvals;                       /* +0x18 Vec4, or 4 shorts with flag 4 */
    int pad1c;
    unsigned int ntracks;              /* +0x20 */
    int pad24[3];
    unsigned int nframes;              /* +0x30 */
    unsigned int flags;                /* +0x34 1 relocated, 2 bones not chained, 4 packed rotations */
} Tgmt;

typedef struct TmsSkel {
    int pad[6];
    unsigned short (*hier)[4];         /* +0x18 parent, first child, next sibling */
    int pad1c[9];
    unsigned short root;               /* +0x40 */
} TmsSkel;

typedef struct AnimUpd {               /* the dt argument of model_update */
    float dt;
    void (*bone)(int bone, Mat4 *m, struct AnimUpd *u);
} AnimUpd;

void mat4_mul(Mat4 *out, Mat4 *a, Mat4 *b);
Vec4 *vec3_scale_w(Vec4 *out, Vec4 *a, float s);
Vec4 *vec3_lerp_w(Vec4 *out, Vec4 *a, Vec4 *b, float t);
Vec4 *quat_identity(Vec4 *q);
Vec4 *quat_slerp(Vec4 *out, Vec4 *a, Vec4 *b, float t);
Vec4 *quat_normalize(Vec4 *out, Vec4 *q);
float keyframe_eval(AnimTrack *tr, float t);
float anim_cubic(float p0, float p1, float p2, float p3, float s);
void anim_eval_track(Vec4 *out, Tgmt *a, RotTrack *tr, float t);
void anim_eval_track_packed(float t, Vec4 *out, Tgmt *a, RotTrack *tr);

/* Bone matrices of animation a at frame `frame`. */
void anim_eval(Tgmt *a, TmsSkel *t, Mat4 *bones, float frame, AnimUpd *u)
{
    unsigned short b = t->root;
    MatF id = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

    while (b != 0xFFFF) {
        unsigned short *h = t->hier[b];
        unsigned short *ch = a->chan[b];
        Mat4 *parent;
        Mat4 *out = &bones[b];

        if (h[0] != 0xFFFF) {
            parent = &bones[h[0]];
        } else {
            parent = (Mat4 *)&id;
        }
        if ((a->flags & 2) == 2) {
            parent = (Mat4 *)&id;
        }
        if (ch[0] != 0xFFFF) {
            AnimTrack *tr = &a->tracks[ch[0]];
            MatF tm = id;
            float x = keyframe_eval(&tr[0], frame);
            float y = keyframe_eval(&tr[1], frame);

            tm.z = keyframe_eval(&tr[2], frame);
            tm.w = 1.0f;
            tm.x = x;
            tm.y = y;
            mat4_mul(out, parent, (Mat4 *)&tm);
        } else {
            *out = *parent;
        }
        if (ch[1] != 0xFFFF) {
            Vec4 q;
            Mat4 r;

            anim_eval_track(&q, a, &a->rtracks[ch[1]], frame);
            quat_to_mat4(&r, &q);
            mat4_mul(out, out, &r);
        }
        if (ch[2] != 0xFFFF) {
            AnimTrack *tr = &a->tracks[ch[2]];
            float sx = keyframe_eval(&tr[0], frame);
            float sy = keyframe_eval(&tr[1], frame);
            float sz = keyframe_eval(&tr[2], frame);

            vec3_scale_w((Vec4 *)&out->m[0], (Vec4 *)&out->m[0], sx);
            vec3_scale_w((Vec4 *)&out->m[4], (Vec4 *)&out->m[4], sy);
            vec3_scale_w((Vec4 *)&out->m[8], (Vec4 *)&out->m[8], sz);
        }
        if (u != 0 && u->bone != 0) {
            u->bone(b, out, u);
        }
        if (h[1] != 0xFFFF) {
            b = h[1];
        } else if (h[2] != 0xFFFF) {
            b = h[2];
        } else {
            b = h[0];
            if (b != 0xFFFF) {
                while (b != 0xFFFF && t->hier[b][2] == 0xFFFF) {
                    b = t->hier[b][0];
                }
                if (b != 0xFFFF) {
                    b = t->hier[b][2];
                }
            } else {
                b = 0xFFFF;
            }
        }
    }
}

MatF anim_blend_id = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* the frame argument is saved one instruction later in the prologue */
/* Same blending animation p at frame `pframe` into a at `frame` by w. */
void anim_blend(Tgmt *a, TmsSkel *t, Mat4 *bones, Tgmt *p, float frame, float pframe, float w, AnimUpd *u)
{
    unsigned short b = t->root;
    MatF id = anim_blend_id;

    while (b != 0xFFFF) {
        unsigned short *h = t->hier[b];
        unsigned short *ch = a->chan[b];
        unsigned short *pch = p->chan[b];
        Mat4 *parent;
        Mat4 *out = &bones[b];
        Vec4 va;
        Vec4 vb;
        Vec4 v;
        MatF tm;
        Vec4 qa;
        Vec4 qb;
        Vec4 q;
        Mat4 r;

        if (h[0] != 0xFFFF) {
            parent = &bones[h[0]];
        } else {
            parent = (Mat4 *)&id;
        }
        if ((a->flags & 2) == 2) {
            parent = (Mat4 *)&id;
        }
        if (ch[0] != 0xFFFF) {
            AnimTrack *tr = &a->tracks[ch[0]];
            AnimTrack *ptr = &p->tracks[pch[0]];

            tm = id;
            va.x = keyframe_eval(&tr[0], frame);
            va.y = keyframe_eval(&tr[1], frame);
            va.z = keyframe_eval(&tr[2], frame);
            va.w = 1.0f;
            vb.x = keyframe_eval(&ptr[0], pframe);
            vb.y = keyframe_eval(&ptr[1], pframe);
            vb.z = keyframe_eval(&ptr[2], pframe);
            vb.w = 1.0f;
            vec3_lerp_w(&v, &vb, &va, w);
            tm.x = v.x;
            tm.y = v.y;
            tm.z = v.z;
            tm.w = v.w;
            mat4_mul(out, parent, (Mat4 *)&tm);
        } else {
            *out = *parent;
        }
        if (ch[1] == 0xFFFF) {
            quat_identity(&qa);
        } else {
            anim_eval_track(&qa, a, &a->rtracks[ch[1]], frame);
        }
        if (pch[1] == 0xFFFF) {
            quat_identity(&qb);
        } else {
            anim_eval_track(&qb, p, &p->rtracks[pch[1]], pframe);
        }
        quat_slerp(&q, &qb, &qa, w);
        quat_to_mat4(&r, &q);
        mat4_mul(out, out, &r);
        if (ch[2] != 0xFFFF) {
            AnimTrack *tr = &a->tracks[ch[2]];
            AnimTrack *ptr = &p->tracks[pch[2]];
            Vec4 sa;
            Vec4 sb;
            Vec4 s;

            sa.x = keyframe_eval(&tr[0], frame);
            sa.y = keyframe_eval(&tr[1], frame);
            sa.z = keyframe_eval(&tr[2], frame);
            sb.x = keyframe_eval(&ptr[0], pframe);
            sb.y = keyframe_eval(&ptr[1], pframe);
            sb.z = keyframe_eval(&ptr[2], pframe);
            vec3_lerp_w(&s, &sb, &sa, w);
            vec3_scale_w((Vec4 *)&out->m[0], (Vec4 *)&out->m[0], s.x);
            vec3_scale_w((Vec4 *)&out->m[4], (Vec4 *)&out->m[4], s.y);
            vec3_scale_w((Vec4 *)&out->m[8], (Vec4 *)&out->m[8], s.z);
        }
        if (u != 0 && u->bone != 0) {
            u->bone(b, out, u);
        }
        if (h[1] != 0xFFFF) {
            b = h[1];
        } else if (h[2] != 0xFFFF) {
            b = h[2];
        } else {
            b = h[0];
            if (b != 0xFFFF) {
                while (b != 0xFFFF && t->hier[b][2] == 0xFFFF) {
                    b = t->hier[b][0];
                }
                if (b != 0xFFFF) {
                    b = t->hier[b][2];
                }
            } else {
                b = 0xFFFF;
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* two mul.s with the operands swapped */
float anim_cubic(float p0, float p1, float p2, float p3, float s)
{
    float s2 = s * s;
    float r = 1.0f - s;
    float r2 = r * r;

    return s2 * s * p3 + (3.0f * r * s2 * p2 + (r2 * r * p0 + 3.0f * r2 * s * p1));
}
#endif

/* Cubic Bezier between keys a and b. */
float anim_bezier(float t, AnimKey *a, AnimKey *b)
{
    return anim_cubic(a->v, a->out, a->in, b->v, (t - (float)a->t) / (float)(b->t - a->t));
}

float keyframe_eval(AnimTrack *tr, float t)
{
    AnimKey *k = tr->keys;
    int n = tr->n;
    int lo;
    int hi;

    if (n == 0) {
        return 0.0f;
    }
    if (n == 1) {
        return k[0].v;
    }
    if (t <= (float)k[0].t) {
        return k[0].v;
    }
    if (t >= (float)k[n - 1].t) {
        return k[n - 1].v;
    }
    lo = 0;
    hi = n - 1;
    while (hi - lo > 1) {
        int mid = lo + (hi - lo) / 2;

        if (t <= (float)k[mid].t) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    return anim_bezier(t, &k[lo], &k[hi]);
}

void anim_eval_track(Vec4 *out, Tgmt *a, RotTrack *tr, float t)
{
    unsigned int *times = &a->rtimes[tr->first];
    Vec4 *v = &((Vec4 *)a->rvals)[tr->first];
    int n;
    int lo;
    int hi;

    if ((a->flags & 4) == 4) {
        anim_eval_track_packed(t, out, a, tr);
        return;
    }
    n = tr->n;
    if (n == 0) {
        *out = (Vec4){ 0.0f, 0.0f, 0.0f, 1.0f };
        return;
    }
    if (n == 1) {
        *out = v[0];
        return;
    }
    if (t <= (float)times[0]) {
        *out = v[0];
        return;
    }
    if (t >= (float)times[n - 1]) {
        *out = v[n - 1];
        return;
    }
    lo = 0;
    hi = n - 1;
    while (hi - lo > 1) {
        int mid = lo + (hi - lo) / 2;

        if (t <= (float)times[mid]) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    quat_slerp(out, &v[lo], &v[hi], (t - (float)times[lo]) / (float)(times[hi] - times[lo]));
    quat_normalize(out, out);
}

Vec4 anim_quat_id = { 0.0f, 0.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* the two key pointers are computed in another order around the first load */
void anim_eval_track_packed(float t, Vec4 *out, Tgmt *a, RotTrack *tr)
{
    unsigned int *times = &a->rtimes[tr->first];
    short *v = &((short *)a->rvals)[tr->first * 4];
    int n = tr->n;
    int lo;
    int hi;

    if (n == 0) {
        *out = anim_quat_id;
    } else if (n == 1) {
        out->x = v[0] / 32760.0f;
        out->y = v[1] / 32760.0f;
        out->z = v[2] / 32760.0f;
        out->w = v[3] / 32760.0f;
    } else if (t <= (float)times[0]) {
        out->x = v[0] / 32760.0f;
        out->y = v[1] / 32760.0f;
        out->z = v[2] / 32760.0f;
        out->w = v[3] / 32760.0f;
    } else if (t >= (float)times[n - 1]) {
        v += (n - 1) * 4;
        out->x = v[0] / 32760.0f;
        out->y = v[1] / 32760.0f;
        out->z = v[2] / 32760.0f;
        out->w = v[3] / 32760.0f;
    } else {
        short *p0;
        short *p1;
        Vec4 q0;
        Vec4 q1;
        float f;

        lo = 0;
        hi = n - 1;
        while (hi - lo > 1) {
            int mid = lo + (hi - lo) / 2;

            if (t <= (float)times[mid]) {
                hi = mid;
            } else {
                lo = mid;
            }
        }
        f = (t - (float)times[lo]) / (float)(times[hi] - times[lo]);
        p0 = v + lo * 4;
        p1 = v + hi * 4;
        q0.x = p0[0] / 32760.0f;
        q0.y = p0[1] / 32760.0f;
        q0.z = p0[2] / 32760.0f;
        q0.w = p0[3] / 32760.0f;
        q1.x = p1[0] / 32760.0f;
        q1.y = p1[1] / 32760.0f;
        q1.z = p1[2] / 32760.0f;
        q1.w = p1[3] / 32760.0f;
        quat_slerp(out, &q0, &q1, f);
        quat_normalize(out, out);
    }
}
#endif
