// @range 0x08824040-0x088243E4
// @data 0x088ED278-0x088ED284
/* EHF command handlers, part 3: bone matrix and scale from typed arguments, and a random jitter
   translation renewed every `period` seconds. */
#include "psp.h"
#include "ehfcmd.h"

int rand_next(void);

/* 0x10010: multiplies by bone `idx` of `model`. */
int ehf_cmd_bone_args(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    int idx;
    Vec4 model;
    Mat4 m;
    void *out[2] = { &model, &idx };
    char fmt[3] = "nn";

    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, fmt);
    mtx_push();
    tgm_get_bone_matrix(*(void **)&model, &m, idx);
    mtx_mult(&m);
    return 2;
}

/* 0x10011: scale. */
int ehf_cmd_scale(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 s;
    Mat4 m;
    void *out[1] = { &s };
    char fmt[2] = "I";

    if (phase != 5) {
        return 0;
    }
    ehf_cmd_args(0, cmd, 0, out, fmt);
    s.w = 1.0f;
    mtx_push();
    mat4_identity(&m);
    m.m[0] = s.x;
    m.m[5] = s.y;
    m.m[10] = s.z;
    mtx_mult(&m);
    return 2;
}

/* 0x10012: translation by a random offset within +-range, renewed every `period` seconds (state:
   the offset and, in w, the time it was drawn). */
int ehf_cmd_jitter(Vec4 *st, EhfNode *node, EhfCmd *cmd, int phase)
{
    Vec4 range;
    float period;
    float dt;
    Vec4 pos;

    switch (phase) {
    case 1:
        return 0x10;
    case 2:
        st->w = -9999.0f;
        return 0x10;
    default:
        return 0;
    case 5: {
        void *out[2] = { &range, &period };
        char fmt[3] = "If";

        ehf_cmd_args(0, cmd, 0, out, fmt);
        dt = node->time - st->w;
        if (st->w < 0.0f || dt < 0.0f || dt >= period * 60.0f) {
            st->x = range.x * (float)rand_next() * 2.0f / 2147483648.0f - range.x;
            rand_next();
            rand_next();
            st->y = range.y * (float)rand_next() * 2.0f / 2147483648.0f - range.y;
            rand_next();
            rand_next();
            rand_next();
            rand_next();
            st->z = range.z * (float)rand_next() * 2.0f / 2147483648.0f - range.z;
            rand_next();
            rand_next();
            rand_next();
            st->w = node->time;
        }
        pos = *st;
        pos.w = 1.0f;
        mtx_push();
        mtx_translate(&pos);
        return 2;
    }
    }
}
