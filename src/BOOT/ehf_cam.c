// @range 0x0883D708-0x0883DD58
// @data 0x088EE0C8-0x088EE0D8
/* EHF scene commands that set the camera (0x18 perspective, 0x19 ortho, 0x1A look-at; phase 5
   on entering the node pushes the matrix, phase 6 on leaving pops it), plus the TGMA material
   animation: in-place relocation of the file and per-frame evaluation of UV offset, UV scale and
   alpha of each material layer. */
#include "psp.h"
#include "libc.h"
#include "allegrex.h"

typedef struct TgmaTrack {             /* 8 bytes */
    int unk0;
    void *keys;                        /* +0x04 offset in 16-byte keys until relocated */
} TgmaTrack;

typedef struct TgmaNode {              /* 0x24 */
    unsigned short layer[4][4];        /* per layer: track of UV offset, UV scale, alpha (0xFFFF none) */
    int material;                      /* +0x20 material index in the model */
} TgmaNode;

typedef struct Tgma {
    int magic;                         /* "TGMA" */
    TgmaNode *nodes;                   /* +0x04 */
    TgmaTrack *tracks;                 /* +0x08 */
    char *keys;                        /* +0x0C */
    unsigned int n_nodes;              /* +0x10 */
    unsigned int n_tracks;             /* +0x14 */
    int unk18[2];
    unsigned int flags;                /* +0x20 bit 0: relocated */
} Tgma;

int ehf_cmd_args(void *st, void *cmd, int first, void *out, const char *fmt);
void mtx_store(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_perspective(float fovy, float aspect, float near, float far);
void mtx_ortho(float l, float r, float b, float t, float n, float f);
void mtx_look_at(float *eye, float *target, float *up);
float keyframe_eval(float t, TgmaTrack *track);

int ehf_cmd_perspective(void *a, void *b, void *cmd, int phase)
{
    switch (phase) {
    case 5: {
        float fovy, near, far;
        float *out[3] = { &fovy, &near, &far };
        char fmt[] = "fff";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        mtx_store(0);
        mtx_push();
        mtx_identity();
        mtx_perspective(fovy, 480.0f / 272.0f, near, far);
        mtx_store(2);
        return 0x80000000;
    }
    case 6: {
        mtx_store(0);
        mtx_pop();
        mtx_store(2);
        return 0;
    }
    }
    return 0;
}

int ehf_cmd_ortho(void *a, void *b, void *cmd, int phase)
{
    switch (phase) {
    case 5: {
        float l, r, bo, t, n, f;
        float *out[6] = { &l, &r, &bo, &t, &n, &f };
        char fmt[] = "ffffff";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        mtx_store(0);
        mtx_push();
        mtx_identity();
        mtx_ortho(l, r, bo, t, n, f);
        mtx_store(1);
        mtx_push();
        mtx_identity();
        mtx_store(2);
        return 0x80000000;
    }
    case 6: {
        mtx_store(1);
        mtx_pop();
        mtx_store(0);
        mtx_pop();
        mtx_store(2);
        return 0;
    }
    }
    return 0;
}

int ehf_cmd_look_at(void *a, void *b, void *cmd, int phase)
{
    switch (phase) {
    case 5: {
        float eye[3], target[3], up[3];
        float *out[3] = { eye, target, up };
        char fmt[] = "III";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        mtx_store(1);
        mtx_push();
        mtx_identity();
        mtx_look_at(eye, target, up);
        mtx_store(2);
        return 0x80000000;
    }
    case 6: {
        mtx_store(1);
        mtx_pop();
        mtx_store(2);
        return 0;
    }
    }
    return 0;
}

/* Turns the file's offsets into pointers once (flag bit 0); 0 if not a TGMA. */
Tgma *tgma_relocate(Tgma *t)
{
    unsigned int i;

    if (t == 0) {
        return 0;
    }
    if (t->magic != 0x414D4754) {
        return 0;
    }
    if (t->flags & 1) {
        return t;
    }
    t->flags |= 1;
    t->nodes = (TgmaNode *)((char *)t + (int)t->nodes);
    t->tracks = (TgmaTrack *)((char *)t + (int)t->tracks);
    t->keys = (char *)t + (int)t->keys;
    for (i = 0; i < t->n_tracks; i++) {
        t->tracks[i].keys = t->keys + (int)t->tracks[i].keys * 0x10;
    }
    return t;
}

/* Evaluates the animation at time `t` into the model's material layers (UV offset and scale in `uv`, alpha in `mats`). */
void tgma_apply(Tgma *anim, char *model, char *mats, char *uv, float t)
{
    unsigned int n = anim->n_nodes;
    unsigned int i;

    for (i = 0; i < n; i++) {
        int j;
        unsigned short *e = anim->nodes[i].layer[0];
        char *layer = *(char **)(model + 0xC) + anim->nodes[i].material * 0xA4;
        char *m = mats + i * 0xA4;

        for (j = 0; j < 4; j++, e += 4, layer += 0x20, m += 0x20) {
            unsigned int *ls = (unsigned int *)(layer + 0x18);   /* flags, texture */
            char *tex;
            int idx;

            if (!(ls[0] & 1)) {
                break;
            }
            tex = (char *)ls[1];
            if (tex != 0 && (idx = *(int *)(tex + 0x24)) != -1) {
                float *o = (float *)(uv + idx * 0x28);

                if (e[0] != 0xFFFF) {
                    TgmaTrack *k = &anim->tracks[e[0]];
                    o[5] = keyframe_eval(t, k);
                    o[6] = keyframe_eval(t, k + 1);
                } else {
                    o[6] = 0.0f;
                    o[5] = 0.0f;
                }
                if (e[1] != 0xFFFF) {
                    TgmaTrack *k = &anim->tracks[e[1]];
                    o[7] = keyframe_eval(t, k);
                    o[8] = keyframe_eval(t, k + 1);
                } else {
                    o[8] = 1.0f;
                    o[7] = 1.0f;
                }
                if (e[2] != 0xFFFF) {
                    int a = (unsigned int)(keyframe_eval(t, &anim->tracks[e[2]]) * 255.0f);
                    a = ALLEGREX_MAX(a, 0);
                    a = ALLEGREX_MIN(a, 255);
                    *(unsigned int *)(m + 0x24) &= 0xFFFFFF;
                    *(unsigned int *)(m + 0x24) |= a << 24;
                }
            }
        }
    }
}
