// @range 0x08822F84-0x08823BD0
// @asm 0x088234C4-0x088236D0
// @data 0x088ED200-0x088ED230
/* EHF command handlers, part 1: frame windows and loops, raw GE words, and the transform
   commands (translate, rotate, scale/rotate/translate from typed arguments, matrices, quaternions,
   visibility spheres). A handler gets the instance state, the node (frame counter and time), the
   command (argument count at +3, arguments from +0x10) and the phase (1/2 setup, 5 draw, 6 after
   the children, 0x1xx messages); it returns flags for the walker (1 skip the children, 2 pop the
   matrix after them). */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

extern unsigned int *postfx_param;     /* 0x088FA000 list saved by command 0xA */

/* 0xA: draws the children into layer arg0, then returns to the previous list. */
int ehf_cmd_layer(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 5:
        postfx_param = dl_select_layer(cmd->a.i[0], 0);
        return 0x80000000;
    case 6:
        dl_set_list(postfx_param);
        postfx_param = 0;
        return 0;
    }
    return 0;
}

/* 4: children only within frames [arg0, arg1), relative to arg0. */
int ehf_cmd_window(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    int *a;

    if (phase != 5) {
        return 0;
    }
    a = cmd->a.i;
    if (node->frame < a[0] || node->frame >= a[1]) {
        return 1;
    }
    node->frame -= a[0];
    node->time = FRAME16(node);
    return 0;
}

/* 5: past arg1, loops back over [arg0, arg1). */
int ehf_cmd_loop(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    int *a;
    int end;
    int f;

    if (phase != 5) {
        return 0;
    }
    a = cmd->a.i;
    end = a[1];
    f = node->frame;
    if (f >= end) {
        node->frame = a[0] + (f - end) % (end - a[0]);
        node->time = FRAME16(node);
    }
    return 0;
}

/* 6: holds at frame arg0. */
int ehf_cmd_hold(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    int v;

    if (phase != 5) {
        return 0;
    }
    v = cmd->a.i[0];
    if (node->frame >= v) {
        node->frame = v;
        node->unk4 = 0;
        node->time = FRAME16(node);
    }
    return 0;
}

/* 7: frames counted from the first frame the command was reached. */
int ehf_cmd_start(int *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 1:
        return 4;
    case 2:
        *ctx = -1;
        return 4;
    case 5:
        if (*ctx < 0) {
            *ctx = node->frame;
        }
        node->frame -= *ctx;
        node->time = FRAME16(node);
        return 0;
    }
    return 0;
}

/* 0xC: copies the arguments (GE words) into the saved list. */
int ehf_cmd_ge(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    unsigned int *p;

    switch (phase) {
    case 5:
        p = postfx_get_param();
        memcpy(p, cmd->a.i, cmd->argc << 2);
        postfx_set_param(p + cmd->argc);
        return 0;
    }
    return 0;
}

int ehf_cmd_nop(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    return 0;
}

/* 8: translate (3 args) or rotate + translate (6 args). */
int ehf_cmd_trs(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 t;
    Vec4 r;
    float *a;

    switch (phase) {
    case 5:
        a = cmd->a.f;
        mtx_push();
        switch (cmd->argc) {
        case 6:
            t.x = a[0];
            t.y = a[1];
            t.z = a[2];
            t.w = 1.0f;
            r.x = a[3];
            r.y = a[4];
            r.z = a[5];
            r.w = 0.0f;
            mtx_rotate_xyz(&r);
            mtx_translate(&t);
            break;
        case 3:
            t.x = a[0];
            t.y = a[1];
            t.z = a[2];
            t.w = 1.0f;
            mtx_translate(&t);
            break;
        }
        return 2;
    }
    return 0;
}

/* 9: translation, rotation and scale from 1 to 3 typed vectors. */
int ehf_cmd_srt(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 t;
    Vec4 r;
    Vec4 s;
    Mat4 m;
    void *out[3] = { &t, &r, &s };
    char fmt[4] = "III";
    int n;

    if (phase != 5) {
        return 0;
    }
    n = ehf_cmd_args(0, cmd, 0, out, fmt);
    t.w = 1.0f;
    s.w = 1.0f;
    r.w = 0.0f;
    switch (n) {
    case 1:
        mat4_identity(&m);
        mat4_translate(&m, &m, &t);
        break;
    case 2:
        mat4_identity(&m);
        if (&r != 0) {
            mat4_rotate_xyz(&m, &m, &r);
        }
        mat4_translate(&m, &m, &t);
        break;
    case 3:
        mat4_identity(&m);
        m.m[0] = s.x;
        m.m[5] = s.y;
        m.m[10] = s.z;
        if (&r != 0) {
            mat4_rotate_xyz(&m, &m, &r);
        }
        mat4_translate(&m, &m, &t);
        break;
    default:
        return 0;
    }
    mtx_push();
    mtx_mult(&m);
    return 2;
}

/* 0x10001: identity. */
int ehf_cmd_identity(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    if (phase != 5) {
        return 0;
    }
    mtx_push();
    mtx_identity();
    return 2;
}

/* 0x10002: multiplies by the matrix in the arguments. */
int ehf_cmd_matrix(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    int *a = cmd->a.i;
    Mat4 m;

    if (phase != 5) {
        return 0;
    }
    memcpy(&m, a, 0x40);
    mtx_push();
    mtx_mult(&m);
    return 2;
}

#ifdef NON_MATCHING /* one instruction of the prologue lands on the other side of the first branch */
/* 0x10003: matrix set from outside (messages 0x101..0x105), for the node with id arg0. */
int ehf_cmd_matrix_slot(EhfMtxState *st, int *msg, EhfCmd *cmd, int phase)
{
    int argc = cmd->argc;
    int *a = cmd->a.i;
    Mat4 *ref;

    switch (phase) {
    case 5:
        if (st->kind == 0) {
            return 0;
        }
        mtx_push();
        switch (st->kind) {
        case 1:
            mtx_mult(&st->m);
            break;
        case 2:
            mtx_mult(st->ref);
            break;
        case 3:
            mtx_load(&st->m);
            break;
        }
        return 2;
    case 0x101:
        if (msg[0] != 0) {
            memcpy(&st->m, (void *)msg[0], 0x40);
            st->kind = 1;
        } else {
            st->kind = 0;
        }
        return 0;
    case 0x102:
        ref = (Mat4 *)msg[0];
        st->ref = ref;
        if (ref != 0) {
            st->kind = 2;
        } else {
            st->kind = 0;
        }
        return 0;
    case 0x104:
        ref = (Mat4 *)((int *)msg[0])[1];
        if (st->id == *(int *)msg[0]) {
            st->ref = ref;
            if (ref != 0) {
                st->kind = 2;
            } else {
                st->kind = 0;
            }
        }
        return 0;
    case 0x103:
        ref = (Mat4 *)((int *)msg[0])[1];
        if (st->id == *(int *)msg[0]) {
            if (ref != 0) {
                memcpy(&st->m, ref, 0x40);
                st->kind = 1;
            } else {
                st->kind = 0;
            }
        }
        return 0;
    case 0x105:
        if (msg[0] != 0) {
            memcpy(&st->m, (void *)msg[0], 0x40);
            st->kind = 3;
        } else {
            st->kind = 0;
        }
        return 0;
    case 2:
        st->kind = 0;
        st->id = 0;
        if (argc > 0) {
            int id = a[0];

            if (id >= 0x4000 && id < 0xC001) {
                st->id = id;
            }
        }
        return 0x50;
    case 1:
        return 0x50;
    }
    return 0;
}
#endif

/* 0x10004: translate + rotate, then skips the children when the sphere (center, radius) is out of
   view. */
int ehf_cmd_cull(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    float *a = cmd->a.f;
    Vec4 t = { a[0], a[1], a[2], 1.0f };
    Vec4 r = { a[3], a[4], a[5] };
    DrawSphere s;

    if (phase != 5) {
        return 0;
    }
    mtx_push();
    mtx_rotate_xyz(&r);
    mtx_translate(&t);
    s.pos.x = a[6];
    s.pos.y = a[7];
    s.pos.z = a[8];
    s.pos.w = 1.0f;
    s.radius = a[9];
    mtx_get_proj(&s.fovy, 0, &s.near, &s.far);
    if (draw_sphere_visible(&s) == 0) {
        mtx_pop();
        return 1;
    }
    return 2;
}

/* 0x10005: same with typed arguments. */
int ehf_cmd_cull_args(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 t;
    float radius;
    Vec4 r;
    Vec4 c;
    DrawSphere s;
    void *out[4] = { &t, &r, &c, &radius };
    char fmt[5] = "IIIf";

    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, fmt);
    t.w = 1.0f;
    c.w = 1.0f;
    r.w = 0.0f;
    mtx_push();
    mtx_rotate_xyz(&r);
    mtx_translate(&t);
    s.pos = c;
    s.radius = radius;
    mtx_get_proj(&s.fovy, 0, &s.near, &s.far);
    if (draw_sphere_visible(&s) == 0) {
        mtx_pop();
        return 1;
    }
    return 2;
}

/* 0x10006: translation oscillating as amp * sin(phase + speed * time). */
int ehf_cmd_wave(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    float *a = cmd->a.f;
    Vec4 t;

    if (phase != 5) {
        return 0;
    }
    t.x = a[0] * msl_sinf(a[6] + a[3] * node->time);
    t.y = a[1] * msl_sinf(a[7] + a[4] * node->time);
    t.z = a[2] * msl_sinf(a[8] + a[5] * node->time);
    t.w = 1.0f;
    mtx_push();
    mtx_translate(&t);
    return 2;
}

/* 0x10007: rotation start + speed * time. */
int ehf_cmd_spin(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    float *a = cmd->a.f;
    Vec4 r;

    if (phase != 5) {
        return 0;
    }
    r.x = a[3] + a[0] * node->time;
    r.y = a[4] + a[1] * node->time;
    r.z = a[5] + a[2] * node->time;
    r.w = 0.0f;
    mtx_push();
    mtx_rotate_xyz(&r);
    return 2;
}

/* 0x10008: rotation from a typed vector. */
int ehf_cmd_rotate(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 r;
    void *out[1] = { &r };
    char fmt[2] = "I";

    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, fmt);
    r.w = 0.0f;
    mtx_push();
    mtx_rotate_xyz(&r);
    return 2;
}

/* 0x10009: rotation start + (speed + accel * time) * time. */
int ehf_cmd_spin_accel(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    float *a = cmd->a.f;
    Vec4 r;
    float time;
    float vx, vy, vz;

    if (phase != 5) {
        return 0;
    }
    time = node->time;
    vx = a[0] + a[6] * time;
    vy = a[1] + a[7] * time;
    vz = a[2] + a[8] * time;
    r.x = a[3] + vx * time;
    r.y = a[4] + vy * time;
    r.z = a[5] + vz * time;
    mtx_push();
    mtx_rotate_xyz(&r);
    return 2;
}

/* 0x1000A: rotation by a quaternion. */
int ehf_cmd_quat(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    float *a = cmd->a.f;
    Mat4 m;
    Vec4 q;

    if (phase != 5) {
        return 0;
    }
    q.x = a[0];
    q.y = a[1];
    q.z = a[2];
    q.w = a[3];
    mtx_push();
    quat_to_mat4(&m, &q);
    mtx_mult(&m);
    return 2;
}

