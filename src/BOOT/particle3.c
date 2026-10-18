// @range 0x08827B8C-0x08828590
// @asm 0x08827B8C-0x08827D98
// @asm 0x08828334-0x08828424
// @data 0x088ED434-0x088ED474
/* EHF particles, part 3: positions in a cone around a direction, emitter positions moved into the
   current model space, velocity generators (constant, radial from a center) and the per-frame
   motion commands (integration, acceleration, drag). */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct EhfParticle {
    Vec4 pos;                          /* w: age (0 free) */
    Vec4 vel;                          /* +0x10 w: lifetime */
    Vec4 unk20;
} EhfParticle;

typedef struct EhfPool {
    EhfParticle *p;
    int n;
} EhfPool;

typedef void (*EhfPosFn)(Vec4 *out, struct EhfEmitter *e, void *arg);

typedef struct EhfEmitter {
    EhfPosFn pos;
    void *pos_arg;                     /* +0x04 */
    void *vel;
    void *vel_arg;                     /* +0x0C */
    int bursts;                        /* +0x10 */
    float accum;                       /* +0x14 */
    int next;                          /* +0x18 */
} EhfEmitter;

typedef struct EhfCone {               /* random direction within `cosang` of the axis rotated by q */
    Vec4 q;                            /* rotation from +y to the cone axis */
    float center[3];                   /* +0x10 */
    float cosang;                      /* +0x1C */
    float r0;                          /* +0x20 */
    float r1;                          /* +0x24 */
} EhfCone;

typedef struct EhfLocal {              /* previous position callback, result moved to model space */
    EhfPosFn pos;
    void *arg;
} EhfLocal;

typedef struct EhfRadial {             /* velocity away from `center` with a random speed */
    Vec4 center;
    float v0;                          /* +0x10 */
    float v1;                          /* +0x14 */
} EhfRadial;

typedef struct GfxState {
    unsigned int frame;
    int fps;
} GfxState;

extern EhfPool *ehf_pool;
extern EhfEmitter *ehf_emitter;
extern Vec4 ehf_ptcl_x;               /* 0x088ED3D0 (1, 0, 0, 0) */
extern int rand3_slot;
extern unsigned int rand3_seeds[][3];

GfxState *gfx_get_state(void);
int rand_next(void);
void vec3_add(float *out, float *a, float *b);
float *vec3_scale(float *out, float *a, float s);
float vec3_dot2(Vec4 *a, Vec4 *b);
Vec4 *vec4_add(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec4_sub(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec4_scale(Vec4 *out, Vec4 *a, float s);
Vec4 *vec4_normalize(Vec4 *out, Vec4 *a);
float sqrtf(float x);
float sqrtf_fpu(float x);
float sqrtf_vfpu(float x);
float sinf(float x);
float cosf(float x);

int ehf_ptcl_cone(float *out, EhfEmitter *e, EhfCone *c);

#define RAND15(s) ((int)((s) >> 16 & 0x7FFF))

#ifdef NON_MATCHING /* the out pointer is copied to $s2 earlier, so the seed pointer lands in $a0 instead of $t0 */
/* Random point in the cone, between radii r0 and r1. */
int ehf_ptcl_cone(float *out, EhfEmitter *e, EhfCone *c)
{
    Mat4 m;
    float ang;
    Vec4 v;
    float s;

    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    ang = RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f * 6.283185307179586;
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    v.y = c->cosang + (1.0f - c->cosang) * (RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f);
    s = sqrtf_fpu(1.0f - v.y * v.y);
    v.x = s * sinf(ang);
    v.z = s * cosf(ang);
    v.w = 0.0f;
    quat_to_mat4(&m, &c->q);
    mat4_transform_vec4(&v, &m, &v);
    rand3_seeds[rand3_slot][0] *= 0x41C64E6D;
    rand3_seeds[rand3_slot][0] += 0x3039;
    vec4_scale(&v, &v, c->r0 + (c->r1 - c->r0) * (RAND15(rand3_seeds[rand3_slot][0]) / 32767.0f));
    vec3_add(out, c->center, &v.x);
    out[3] = 0.0f;
    return 0;
}
#endif

Vec4 ehf_ptcl_up = { 0.0f, 1.0f, 0.0f, 0.0f };

/* 0x22: positions in a cone (center, direction, half angle, r0, r1). */
int ehf_cmd_ptcl_cone(EhfCone *c, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0x30;
    case 2:
        e->pos = (EhfPosFn)ehf_ptcl_cone;
        e->pos_arg = c;
        return 0x30;
    case 5: {
        Vec4 dir;
        Vec4 axis;
        float ang;
        void *out[5] = { c->center, &dir, &ang, &c->r0, &c->r1 };
        char fmt[6] = "IIfff";
        float d;
        float w;

        ehf_cmd_args(0, cmd, 0, out, fmt);
        dir.w = 0.0f;
        vec4_normalize(&dir, &dir);
        c->cosang = cosf(ang);
        vec3_cross(&axis, &ehf_ptcl_up, &dir);
        if (vec3_dot2(&axis, &axis) < 1e-05f) {
            Vec4 x = ehf_ptcl_x;

            vec4_add(&x, &x, &dir);
            vec3_cross(&axis, &ehf_ptcl_up, &x);
        }
        axis.w = 0.0f;
        vec4_normalize(&axis, &axis);
        d = vec3_dot2(&ehf_ptcl_up, &dir);
        w = sqrtf(0.5f * (1.0f + d));
        vec4_scale(&c->q, &axis, sqrtf(0.5f * (1.0f - d)));
        c->q.w = w;
        c->cosang = cosf(ang);
        return 0;
    }
    }
    return 0;
}

/* Position of the previous callback (origin without one) moved to the current model space. */
void ehf_ptcl_local(Vec4 *out, EhfEmitter *e, EhfLocal *l)
{
    Mat4 m;

    if (l->pos != 0) {
        l->pos(out, e, l->arg);
    } else {
        Vec4 z = { 0 };

        *out = z;
    }
    out->w = 1.0f;
    mtx_get34(&m);
    mat4_transform_vec4(out, &m, out);
}

/* 0x16: positions of the generator set before are relative to the current model matrix. */
int ehf_cmd_ptcl_local(EhfLocal *l, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 8;
    case 2:
        l->pos = e->pos;
        l->arg = e->pos_arg;
        e->pos = (EhfPosFn)ehf_ptcl_local;
        e->pos_arg = l;
        return 8;
    }
    return 0;
}

/* Constant velocity. */
void ehf_ptcl_vel_const(Vec4 *v, EhfParticle *p, EhfEmitter *e, float *c)
{
    v->x = c[0];
    v->y = c[1];
    v->z = c[2];
}

/* 0x15: constant velocity (vector). */
int ehf_cmd_ptcl_vel_const(float *c, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0xC;
    case 2:
        e->vel = (void *)ehf_ptcl_vel_const;
        e->vel_arg = c;
        return 0xC;
    case 5: {
        void *out[1] = { c };
        char fmt[2] = "I";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        return 0;
    }
    }
    return 0;
}

/* Velocity away from the center (+x at the center), speed between v0 and v1. */
void ehf_ptcl_vel_radial(Vec4 *v, EhfParticle *p, EhfEmitter *e, EhfRadial *r)
{
    Vec4 d;
    float len;

    vec4_sub(&d, &p->pos, &r->center);
    len = sqrtf_vfpu(vec3_dot2(&d, &d));
    if (len > 1e-06f) {
        vec4_scale(&d, &d, 1.0f / len);
    } else {
        d = (Vec4){ 1.0f, 0.0f, 0.0f, 0.0f };
    }
    vec4_scale(v, &d, r->v0 + (r->v1 - r->v0) * rand_next() / 2147483648.0f);
    v->w = 0.0f;
}

/* 0x29: radial velocity (center, v0, v1). */
int ehf_cmd_ptcl_vel_radial(EhfRadial *r, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfEmitter *e = ehf_emitter;

    switch (phase) {
    case 1:
        return 0x20;
    case 2:
        e->vel = (void *)ehf_ptcl_vel_radial;
        e->vel_arg = r;
        return 0x20;
    case 5: {
        void *out[3] = { r, &r->v0, &r->v1 };
        char fmt[3] = "Iff";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        return 0;
    }
    }
    return 0;
}

#ifdef NON_MATCHING /* register allocation of the frame time computation differs (as ehf_cmd_pool) */
/* 0x10: moves the live particles of the current pool by their velocity. */
int ehf_cmd_ptcl_move(void *inst, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPool *pool = ehf_pool;

    switch (phase) {
    case 5: {
        float dt = node->unk4 * (1.0f / gfx_get_state()->fps);
        EhfParticle *p;
        EhfParticle *end;

        for (p = pool->p, end = p + pool->n; p < end; p++) {
            if (p->pos.w != 0.0f) {
                p->pos.x += p->vel.x * dt;
                p->pos.y += p->vel.y * dt;
                p->pos.z += p->vel.z * dt;
            }
        }
        return 0;
    }
    }
    return 0;
}
#endif

/* 0xD0017: adds the vector argument to the velocity of the live particles. */
int ehf_cmd_ptcl_accel(void *inst, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPool *pool = ehf_pool;

    switch (phase) {
    case 5: {
        float *acc = cmd->a.f;
        EhfParticle *p;
        EhfParticle *end;

        gfx_get_state();
        for (p = pool->p, end = p + pool->n; p < end; p++) {
            if (p->pos.w != 0.0f) {
                vec3_add(&p->vel.x, &p->vel.x, acc);
            }
        }
        return 0;
    }
    }
    return 0;
}

/* 0x28: drag: velocities scaled by 1 - arg0 * frames. */
int ehf_cmd_ptcl_drag(void *inst, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPool *pool = ehf_pool;

    switch (phase) {
    case 5: {
        float n = node->unk4;
        float k = 1.0f - cmd->a.f[0] * n;
        EhfParticle *p;
        EhfParticle *end;

        for (p = pool->p, end = p + pool->n; p < end; p++) {
            if (p->pos.w != 0.0f) {
                vec3_scale(&p->vel.x, &p->vel.x, k);
            }
        }
        return 0;
    }
    }
    return 0;
}
