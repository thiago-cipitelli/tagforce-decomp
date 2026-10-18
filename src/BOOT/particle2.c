// @range 0x08827030-0x08827B8C
// @data 0x088ED3F0-0x088ED434
/* EHF particles, part 2: position generators on a ring (particles spread evenly by emission
   index), along four tetrahedral directions turned by a random rotation, and on a spherical shell. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct Vec3 {
    float v[3];
} Vec3;

typedef struct EhfPool {
    void *p;
    int n;
} EhfPool;

typedef struct EhfEmitter {
    void *pos;
    void *pos_arg;                     /* +0x04 */
    void *vel;
    void *vel_arg;                     /* +0x0C */
    int bursts;                        /* +0x10 */
    float accum;                       /* +0x14 */
    int next;                          /* +0x18 */
} EhfEmitter;

typedef struct EhfRing {               /* point on a ring in plane `axis`, angle by emission index */
    float center[3];
    int axis;                          /* +0x0C 0 yz, 1 xz, 2 xy */
    float r0;                          /* +0x10 */
    float r1;                          /* +0x14 */
    float height;                      /* +0x18 */
} EhfRing;

typedef struct EhfTetra {              /* random direction pulled toward one of four axes */
    float center[3];
    Vec3 dir[4];                       /* +0x0C */
    float r0;                          /* +0x3C */
    float r1;                          /* +0x40 */
    float pull;                        /* +0x44 */
    int count;                         /* +0x48 */
} EhfTetra;

typedef struct EhfShell {              /* random point between radii r0 and r1 */
    float center[3];
    float r0;                          /* +0x0C */
    float r1;                          /* +0x10 */
} EhfShell;

extern EhfPool *ehf_pool;
extern EhfEmitter *ehf_emitter;
extern int rand3_slot;
extern unsigned int rand3_seeds[][3];

void vec3_add(float *out, float *a, float *b);
float *vec3_scale(float s, float *out, float *a);
float *vec3_normalize(float *out, float *a);
float *mat3_identity(float *m);
float *mat3_from_euler(float *out, float *m, float *v);
float *mat3_transform_vec3(float *out, float *m, float *v);
float sqrtf_fpu(float x);
float sinf(float x);
float cosf(float x);

#define RAND15(s) ((int)((s) >> 16 & 0x7FFF))

/* Point on the ring: the angle steps by 2pi/n with the emitter's index in the pool. */
int ehf_ptcl_ring(float *out, EhfEmitter *e, EhfRing *d)
{
    float ang;
    float r;
    float v[3];
    float h;
    float c;

    ang = (float)e->next / ehf_pool->n * 6.283185307179586;
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    r = d->r0 + (d->r1 - d->r0) * (RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f);
    v[0] = r * sinf(ang);
    v[2] = c = r * cosf(ang);
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    h = d->height * (RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f - 0.5f);
    v[1] = h;
    switch (d->axis) {
    case 0:
        c = v[0];
        v[0] = h;
        v[1] = c;
        break;
    case 2:
        v[2] = h;
        v[1] = c;
        break;
    }
    vec3_add(out, d->center, v);
    return 0;
}

/* 0x1F: positions on a ring (center, axis, r0, r1, height). */
int ehf_cmd_ptcl_ring(EhfRing *d, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0x1C;
    case 2:
        e->pos = (void *)ehf_ptcl_ring;
        e->pos_arg = d;
        return 0x1C;
    case 5: {
        void *out[5] = { d, &d->axis, &d->r0, &d->r1, &d->height };
        char fmt[6] = "Infff";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        return 0;
    }
    }
    return 0;
}

/* Random direction, every particle pulled toward the next of the four axes, at a random radius. */
int ehf_ptcl_tetra(float *out, EhfEmitter *e, EhfTetra *t)
{
    float v[3];
    float d[3];
    int i;

    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    rand3_seeds[rand3_slot][1] *= 0x15A4E35;
    rand3_seeds[rand3_slot][1] += 1;
    rand3_seeds[rand3_slot][2] *= 0x343FD;
    rand3_seeds[rand3_slot][2] += 1;
    v[0] = RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f;
    v[1] = RAND15(rand3_seeds[rand3_slot][1]) / 32767.0f;
    v[2] = RAND15(rand3_seeds[rand3_slot][2]) / 32767.0f;
    v[0] = 2.0f * v[0] - 1.0f;
    v[1] = 2.0f * v[1] - 1.0f;
    v[2] = 2.0f * v[2] - 1.0f;
    i = t->count % 8;
    switch (i) {
    case 0:
    case 1:
    case 2:
    case 3:
        vec3_scale(t->pull, d, t->dir[i].v);
        vec3_add(v, v, d);
        break;
    }
    vec3_normalize(v, v);
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    vec3_scale(t->r0 + (t->r1 - t->r0) * (RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f), v, v);
    vec3_add(out, t->center, v);
    out[3] = 0.0f;
    t->count++;
    return 0;
}

/* 0x20: tetrahedral spread (center, r0, r1, pull); the axes get a random rotation. */
int ehf_cmd_ptcl_tetra(void *arg, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;
    EhfTetra *t = arg;

    switch (phase) {
    case 1:
        return 0x4C;
    case 2: {
        float a[3];
        float m[9];
        int i;

        if (e != 0) {
            e->pos = (void *)ehf_ptcl_tetra;
            e->pos_arg = arg;
        }
        rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
        rand3_seeds[rand3_slot][0] += 0x3039;
        rand3_seeds[rand3_slot][1] *= 0x15A4E35;
        rand3_seeds[rand3_slot][1] += 1;
        rand3_seeds[rand3_slot][2] *= 0x343FD;
        rand3_seeds[rand3_slot][2] += 1;
        a[0] = RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f;
        a[1] = RAND15(rand3_seeds[rand3_slot][1]) / 32767.0f;
        a[2] = RAND15(rand3_seeds[rand3_slot][2]) / 32767.0f;
        a[0] = (a[0] - 0.5f) * 6.283185307179586;
        a[1] = (a[1] - 0.5f) * 6.283185307179586;
        a[2] = 0.0f;
        mat3_from_euler(m, mat3_identity(m), a);
        t->dir[0] = (Vec3){ 1.0f, 1.0f, -1.0f };
        t->dir[1] = (Vec3){ 1.0f, -1.0f, 1.0f };
        t->dir[2] = (Vec3){ -1.0f, 1.0f, 1.0f };
        t->dir[3] = (Vec3){ -1.0f, -1.0f, -1.0f };
        for (i = 0; i < 4; i++) {
            mat3_transform_vec3(t->dir[i].v, m, t->dir[i].v);
        }
        t->count = 0;
        return 0x4C;
    }
    case 5: {
        void *out[4] = { t, &t->r0, &t->r1, &t->pull };
        char fmt[5] = "Ifff";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        return 0;
    }
    }
    return 0;
}

/* Random point in the spherical shell (uniform direction). */
int ehf_ptcl_shell(float *out, EhfEmitter *e, EhfShell *s)
{
    float r;
    float ang;
    float v[3];
    float c;

    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    r = s->r0 + (s->r1 - s->r0) * (RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f);
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    ang = (RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f - 0.5f) * 6.283185307179586;
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    v[1] = RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f * 2.0f - 1.0f;
    c = sqrtf_fpu(1.0f - v[1] * v[1]);
    v[0] = c * sinf(ang);
    v[2] = c * cosf(ang);
    vec3_scale(r, v, v);
    vec3_add(out, s->center, v);
    out[3] = 0.0f;
    return 0;
}

/* 0x21: positions in a spherical shell (center, r0, r1). */
int ehf_cmd_ptcl_shell(EhfShell *s, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0x14;
    case 2:
        e->pos = (void *)ehf_ptcl_shell;
        e->pos_arg = s;
        return 0x14;
    case 5: {
        void *out[3] = { s, &s->r0, &s->r1 };
        char fmt[4] = "Iff";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        return 0;
    }
    }
    return 0;
}
