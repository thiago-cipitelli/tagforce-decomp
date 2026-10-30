// @range 0x08823BD0-0x08824040
// @data 0x088ED230-0x088ED278
/* EHF command handlers, part 2: billboards (facing the camera, or turning about an axis towards
   it) and bone matrices. The constants copied by the handlers are named globals here. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

Mat4 ehf_billboard_basis = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
unsigned char ehf_fmt_ii[4] = "II";
unsigned char ehf_fmt_i[4] = "I";

/* Loads a matrix at `pos` (in the current space) whose z axis is `dir` and whose y axis faces the
   camera. The constant column is read word by word from the template (an initializer list makes
   mwcc copy the whole template first). */
int ehf_billboard_axis(Vec4 *pos, Vec4 *dir)
{
    Vec4 p;
    Vec4 z;
    Vec4 y;
    Vec4 x;
    Vec4 view;
    Mat4 cur;

    p = *pos;
    z = *dir;
    mtx_get34(&cur);
    mat4_transform_vec4(&p, &cur, &p);
    mtx_get_view_dir(&view);
    vec3_normalize2(&z, &z);
    vec3_cross(&x, &z, &view);
    vec3_normalize2(&x, &x);
    vec3_cross(&y, &x, &z);
    vec3_normalize2(&y, &y);
    {
        Mat4 m;

        m.m[0] = x.x;
        m.m[1] = x.y;
        m.m[2] = x.z;
        m.m[3] = ehf_billboard_basis.m[3];
        m.m[4] = z.x;
        m.m[5] = z.y;
        m.m[6] = z.z;
        m.m[7] = ehf_billboard_basis.m[7];
        m.m[8] = y.x;
        m.m[9] = y.y;
        m.m[10] = y.z;
        m.m[11] = ehf_billboard_basis.m[11];
        m.m[12] = p.x;
        m.m[13] = p.y;
        m.m[14] = p.z;
        m.m[15] = ehf_billboard_basis.m[15];
        cur = m;
    }
    mtx_push();
    mtx_load(&cur);
    return 2;
}

/* 0x1000C: axis billboard from float arguments (position, direction). */
int ehf_cmd_billboard_axis(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    float *a = cmd->a.f;
    Vec4 pos;
    Vec4 dir;

    if (phase != 5) {
        return 0;
    }
    memcpy(&pos, &a[0], 0xC);
    pos.w = 1.0f;
    memcpy(&dir, &a[3], 0xC);
    dir.w = 0.0f;
    return ehf_billboard_axis(&pos, &dir);
}

/* 0x1000E: same with typed arguments. */
int ehf_cmd_billboard_axis_args(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 pos;
    Vec4 dir;
    void *out[2] = { &pos, &dir };
    unsigned char fmt[3];
    char *f;

    f = (char *)fmt;
    fmt[0] = ehf_fmt_ii[0];
    fmt[1] = ehf_fmt_ii[1];
    fmt[2] = ehf_fmt_ii[2];
    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, f);
    pos.w = 1.0f;
    dir.w = 0.0f;
    return ehf_billboard_axis(&pos, &dir);
}

/* Loads a matrix at `pos` with the rotation of the view undone (screen-facing billboard). */
int ehf_billboard(Vec4 *pos)
{
    Vec4 p;
    Mat4 cur;
    Mat4 m;

    p = *pos;
    mtx_get34(&cur);
    mat4_transform_vec4(&p, &cur, &p);
    mtx_set_mode(1);
    mtx_get34(&m);
    mtx_set_mode(2);
    m.m[15] = 1.0f;
    m.m[3] = 0.0f;
    m.m[7] = 0.0f;
    m.m[11] = 0.0f;
    m.m[12] = 0.0f;
    m.m[13] = 0.0f;
    m.m[14] = 0.0f;
    mat4_transpose(&m, &m);
    m.m[12] = p.x;
    m.m[13] = p.y;
    m.m[14] = p.z;
    mtx_push();
    mtx_load(&m);
    return 2;
}

/* 0x1000B: billboard at the float argument position. */
int ehf_cmd_billboard(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    float *a = cmd->a.f;
    Vec4 pos;

    if (phase != 5) {
        return 0;
    }
    memcpy(&pos, &a[0], 0xC);
    pos.w = 1.0f;
    return ehf_billboard(&pos);
}

/* 0x1000D: same with a typed argument. */
int ehf_cmd_billboard_args(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 pos;
    void *out[1] = { &pos };
    unsigned char fmt[2];
    char *f;

    f = (char *)fmt;
    fmt[0] = ehf_fmt_i[0];
    fmt[1] = ehf_fmt_i[1];
    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, f);
    pos.w = 1.0f;
    return ehf_billboard(&pos);
}

/* 0x1000F: multiplies by bone arg1 of the model *arg0. */
int ehf_cmd_bone(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    int *a = cmd->a.i;
    Mat4 m;

    if (phase != 5) {
        return 0;
    }
    mtx_push();
    model_get_bone(*(void **)a[0], &m, a[1]);
    mtx_mult(&m);
    return 2;
}
