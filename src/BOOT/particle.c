// @range 0x0882644C-0x08827030
// @asm 0x0882644C-0x0882684C
// @data 0x088ED3E0-0x088ED3F0
/* EHF particles, part 1: the particle pool (aging and expiry), the emitter (spawns one particle
   every `period` seconds, position and velocity from callbacks set by the commands that follow),
   and position generators: random point in a box, in a box moving between two centers, on a disc. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct EhfParticle {
    Vec4 pos;                          /* w: age (0 free) */
    Vec4 vel;                          /* +0x10 w: lifetime */
    Vec4 unk20;
} EhfParticle;

typedef struct EhfPool {               /* instance memory of command 0xE, followed by the particles */
    EhfParticle *p;
    int n;
    int pad[2];
} EhfPool;

typedef struct EhfEmitter {
    void (*pos)(EhfParticle *p, struct EhfEmitter *e, void *arg);
    void *pos_arg;                     /* +0x04 */
    void (*vel)(Vec4 *v, EhfParticle *p, struct EhfEmitter *e, void *arg);
    void *vel_arg;                     /* +0x0C */
    int bursts;                        /* +0x10 rounds of the pool left, < 0 forever */
    float accum;                       /* +0x14 */
    int next;                          /* +0x18 */
} EhfEmitter;

typedef struct EhfBox {                /* random point in center +- size/2 */
    float center[3];
    float size[3];                     /* +0x0C */
} EhfBox;

typedef struct Vec3 {
    float v[3];
} Vec3;

typedef struct EhfBoxLerp {            /* same, the center moving from c1 to c0 over `dt` */
    Vec3 c0;
    Vec3 size;                         /* +0x0C */
    Vec3 c1;                           /* +0x18 */
    float dt;                          /* +0x24 */
} EhfBoxLerp;

typedef struct EhfDisc {               /* random point on a disc in plane `axis` */
    float center[3];
    int axis;                          /* +0x0C 0 yz, 1 xz, 2 xy */
    float r0;                          /* +0x10 */
    float r1;                          /* +0x14 */
    float height;                      /* +0x18 */
} EhfDisc;

typedef struct GfxState {
    unsigned int frame;
    int fps;
} GfxState;

extern EhfPool *ehf_pool;              /* 0x088FA020 */
extern EhfEmitter *ehf_emitter;        /* 0x088FA024 */
extern int rand3_slot;
extern unsigned int rand3_seeds[][3];

GfxState *gfx_get_state(void);
void vec3_add(float *out, float *a, float *b);
void vec3_mul(float *out, float *a, float *b);
void vec3_lerp(float t, float *out, float *a, float *b);
float sinf(float x);
float cosf(float x);

#ifdef NON_MATCHING /* register allocation of the frame time computation differs */
/* 0xE: pool of arg0 particles; drawn children see it as the current pool. */
int ehf_cmd_pool(EhfPool *pool, EhfNode *node, EhfCmd *cmd, int phase)
{
    unsigned int flag = 0;
    int n;

    switch (phase) {
    case 2:
        n = cmd->a.i[0];
        memset(pool, 0, 8);
        pool->n = n;
        pool->p = (EhfParticle *)(pool + 1);
        memset(pool->p, 0, n * sizeof(EhfParticle));
        ehf_pool = pool;
        flag = 0x80000000;
    case 1:
        return flag | (cmd->a.i[0] * sizeof(EhfParticle) + 0x10);
    case 3:
        ehf_pool = 0;
        return 0;
    case 5:
        ehf_pool = pool;
        return 0x80000000;
    case 6: {
        float inv = 1.0f / gfx_get_state()->fps;
        int dn = node->unk4;
        EhfParticle *p;
        EhfParticle *end;
        float dt;

        ehf_pool = 0;
        dt = dn * inv;
        for (p = pool->p, end = p + pool->n; p < end; p++) {
            if (p->pos.w > 0.0f) {
                p->pos.w += dt;
                if (p->pos.w > p->vel.w) {
                    p->pos.w = 0.0f;
                }
            }
        }
        return 0;
    }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* same as ehf_cmd_pool */
/* 0x1C: emitter: every arg1 seconds a free particle of the current pool gets lifetime arg0, and
   arg2 rounds of the pool are emitted. */
int ehf_cmd_emitter(EhfEmitter *e, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfArg *a = (EhfArg *)cmd->a.i;

    switch (phase) {
    case 1:
        return 0x1C;
    case 2:
        e->pos = 0;
        e->pos_arg = 0;
        e->vel = 0;
        e->vel_arg = 0;
        e->bursts = a[2].i;
        e->accum = 0.0f;
        e->next = 0;
        ehf_emitter = e;
        return 0x8000001C;
    case 3:
        ehf_emitter = 0;
        return 0;
    case 5:
        ehf_emitter = e;
        return 0x80000000;
    case 6: {
        EhfPool *pool = ehf_pool;
        EhfParticle *base = pool->p;
        float dt = (float)node->unk4 * (1.0f / gfx_get_state()->fps);
        EhfParticle *p;

        if (e->bursts != 0) {
        e->accum += dt;
        if (e->accum > a[1].f) {
            do {
                p = &base[e->next];
                if (p->pos.w != 0.0f) {
                    break;
                }
                if (e->pos != 0) {
                    e->pos(p, e, e->pos_arg);
                    p->pos.w = dt;
                } else {
                    Vec4 z = { 0 };

                    p->pos = z;
                    p->pos.w = dt;
                }
                if (e->vel != 0) {
                    e->vel(&p->vel, p, e, e->vel_arg);
                } else {
                    Vec4 z = { 0 };

                    p->vel = z;
                }
                p->vel.w = a[0].f;
                if (++e->next >= pool->n) {
                    if (e->bursts > 0) {
                        e->bursts--;
                    }
                    e->next = 0;
                }
                e->accum -= a[1].f;
            } while (e->accum > a[1].f);
        }
        }
        return 0;
    }
    }
    return 0;
}
#endif

#define RAND15(s) ((int)((s) >> 16 & 0x7FFF))

/* Random point in the box. */
void ehf_ptcl_box(float *out, EhfEmitter *e, EhfBox *b)
{
    float v[3];

    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    rand3_seeds[rand3_slot][1] *= 0x15A4E35;
    rand3_seeds[rand3_slot][1] += 1;
    rand3_seeds[rand3_slot][2] *= 0x343FD;
    rand3_seeds[rand3_slot][2] += 1;
    v[0] = RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f;
    v[1] = RAND15(rand3_seeds[rand3_slot][1]) / 32767.0f;
    v[2] = RAND15(rand3_seeds[rand3_slot][2]) / 32767.0f;
    v[0] -= 0.5f;
    v[1] -= 0.5f;
    v[2] -= 0.5f;
    vec3_mul(v, v, b->size);
    vec3_add(out, v, b->center);
}

/* 0xF: positions in the box (center, size). */
int ehf_cmd_ptcl_box(EhfBox *b, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0x18;
    case 2:
        e->pos = (void (*)(EhfParticle *, EhfEmitter *, void *))ehf_ptcl_box;
        e->pos_arg = b;
        return 0x18;
    case 5: {
        void *out[2] = { b, b->size };
        char fmt[3] = "II";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        return 0;
    }
    }
    return 0;
}

/* Random point in the box moved along its path (the particle's age over `dt`). */
int ehf_ptcl_box_lerp(float *out, EhfParticle *p, EhfBoxLerp *b)
{
    float v[3];
    float c[3];

    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    rand3_seeds[rand3_slot][1] *= 0x15A4E35;
    rand3_seeds[rand3_slot][1] += 1;
    rand3_seeds[rand3_slot][2] *= 0x343FD;
    rand3_seeds[rand3_slot][2] += 1;
    v[0] = RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f;
    v[1] = RAND15(rand3_seeds[rand3_slot][1]) / 32767.0f;
    v[2] = RAND15(rand3_seeds[rand3_slot][2]) / 32767.0f;
    v[0] -= 0.5f;
    v[1] -= 0.5f;
    v[2] -= 0.5f;
    vec3_mul(v, v, b->size.v);
    vec3_lerp(p->vel.y / b->dt, c, b->c1.v, b->c0.v);
    vec3_add(out, v, c);
    return 0;
}

/* 0x17: same moving with the node: the previous center is kept while the node time advances. */
int ehf_cmd_ptcl_box_lerp(EhfBoxLerp *b, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0x28;
    case 2:
        e->pos = (void (*)(EhfParticle *, EhfEmitter *, void *))ehf_ptcl_box_lerp;
        e->pos_arg = b;
        return 0x28;
    case 5: {
        void *out[2] = { b, &b->size };
        char fmt[3] = "II";
        float dt = (float)node->unk4 / gfx_get_state()->fps;

        if (node->frame == 0) {
            ehf_cmd_args(0, cmd, 0, out, fmt);
            b->c1 = b->c0;
        }
        if (dt > 1e-06f) {
            b->c1 = b->c0;
            b->dt = dt;
            ehf_cmd_args(0, cmd, 0, out, fmt);
        }
        return 0;
    }
    }
    return 0;
}

/* Random point on the disc: random angle, radius between r0 and r1, height +- height/2. */
int ehf_ptcl_disc(float *out, EhfEmitter *e, EhfDisc *d)
{
    float ang;
    float r;
    float v[3];
    float h;
    float c;

    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    ang = RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f * 6.283185307179586;
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

/* 0x1E: positions on a disc (center, axis, r0, r1, height). */
int ehf_cmd_ptcl_disc(EhfDisc *d, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0x1C;
    case 2:
        e->pos = (void (*)(EhfParticle *, EhfEmitter *, void *))ehf_ptcl_disc;
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
