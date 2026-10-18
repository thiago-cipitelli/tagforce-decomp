// @range 0x088253FC-0x0882644C
// @asm 0x08826014-0x0882644C
// @data 0x088ED380-0x088ED3D0
/* EHF primitives: a small axis gizmo (command 0xB), and primitives whose vertices are filled by
   later commands (0x10201..0x10206: begin with room in the list or on outside vertices, then
   texture coordinates, colors, positions, or a ribbon built from a polyline facing the camera). */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct EhfPrim {               /* 0x088FA008 primitive being filled */
    unsigned int *end;                 /* last word of the PRIM command */
    void *verts;                       /* +0x04 */
    int count;                         /* +0x08 */
    int prim;                          /* +0x0C */
    int vtype;                         /* +0x10 */
    unsigned char ext : 1;             /* +0x14 vertices outside the list */
} EhfPrim;

typedef struct EhfVtx1 {               /* 0x19F: texture, color, position */
    float u, v;
    unsigned int color;
    float x, y, z;
} EhfVtx1;

typedef struct EhfVtx2 {               /* 0x183: texture, position */
    float u, v;
    float x, y, z;
} EhfVtx2;

typedef struct EhfVtx3 {               /* 0x19C: color, position; 0x180: position */
    unsigned int color;
    float x, y, z;
} EhfVtx3;

extern EhfPrim ehf_prim;
extern unsigned char ehf_axis_vtx[][0x48];   /* 0x088ED2F0 (defined in ehf_cmd4.c as words) */

unsigned int *postfx_get_param(void);
void postfx_set_param(unsigned int *p);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *dl_push(unsigned int *list, unsigned int *words, int n);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *vertices);
void mtx_scale3(Vec4 *v);
Vec4 *vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_sub_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_scale_w(Vec4 *out, Vec4 *a, float s);
float vec3_dot2(Vec4 *a, Vec4 *b);

static int ehf_vertex_set(int vtype, int field, int i, void *src);

/* 0xB: axis gizmo arg0 scaled by arg1, color arg2. */
int ehf_cmd_axis(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    switch (phase) {
    case 5: {
        int idx = cmd->a.i[0];
        float s = cmd->a.f[1];
        unsigned int color = cmd->a.i[2];
        unsigned int *list = postfx_get_param();

        mtx_push();
        mtx_identity();
        {
            Vec4 v = { s, s, s };

            mtx_scale3(&v);
        }
        list = dl_color(list, color);
        list = draw_array(list, 3, 0x1E0, 3, 0, ehf_axis_vtx[idx]);
        mtx_pop();
        postfx_set_param(list);
        return 0;
    }
    }
    return 0;
}

void ehf_prim_reset(void)
{
    memset(&ehf_prim, 0, sizeof(ehf_prim));
    ehf_prim.end = 0;
    ehf_prim.verts = 0;
}

/* Vertex size and layout of the supported vertex types (without the transform flag): 4 tex+color+pos
   float, 3 color+pos float, 2 tex+pos, 1 tex+color+pos 16 bits; 0 when unsupported. */
static int ehf_vtype_info(unsigned int vtype, int *size, int *kind)
{
    vtype &= ~0x800000;
    *kind = 0;
    *size = 0;
    switch (vtype) {
    case 0x19F:
        *kind = 1;
        *size = 0x18;
        break;
    case 0x183:
        *kind = 2;
        *size = 0x14;
        break;
    case 0x19C:
        *kind = 3;
        *size = 0x10;
        break;
    case 0x180:
        *kind = 4;
        *size = 0xC;
        break;
    }
    return *kind;
}

/* Writes texture coordinates (field 3), a color (0x1C) or a position (0x180) of vertex `i`; returns
   the bytes written. */
static int ehf_vertex_set(int vtype, int field, int i, void *src)
{
    EhfPrim *p = &ehf_prim;
    int kind;
    int size;
    float *s = src;

    if (ehf_vtype_info(vtype, &size, &kind) == 0) {
        return 0;
    }
    switch (kind) {
    case 1: {
        EhfVtx1 *v = p->verts;

        switch (field) {
        case 0x180:
            v[i].x = s[0];
            v[i].y = s[1];
            v[i].z = s[2];
            return 0xC;
        case 3:
            v[i].u = s[0];
            v[i].v = s[1];
            return 8;
        case 0x1C:
            v[i].color = *(unsigned int *)src;
            return 4;
        }
        break;
    }
    case 2: {
        EhfVtx2 *v = p->verts;

        switch (field) {
        case 0x180:
            v[i].x = s[0];
            v[i].y = s[1];
            v[i].z = s[2];
            return 0xC;
        case 3:
            v[i].u = s[0];
            v[i].v = s[1];
            return 8;
        }
        break;
    }
    case 3: {
        EhfVtx3 *v = p->verts;

        switch (field) {
        case 0x180:
            v[i].x = s[0];
            v[i].y = s[1];
            v[i].z = s[2];
            return 0xC;
        case 0x1C:
            v[i].color = *(unsigned int *)src;
            return 4;
        }
        break;
    }
    case 4: {
        EhfVtx3 *v = p->verts;

        switch (field) {
        case 0x180:
            v[i].x = s[0];
            v[i].y = s[1];
            v[i].z = s[2];
            return 0xC;
        }
        break;
    }
    }
    return 0;
}

/* Starts a primitive of `count` vertices: on `verts`, or on zeroed room in the list jumped over
   (needs a supported vertex type). */
int ehf_prim_begin(int prim, int vtype, int count, void *verts, int have)
{
    EhfPrim *p = &ehf_prim;
    unsigned int *list = postfx_get_param();
    int size;
    int kind;
    unsigned int cmd[4];
    void *v;

    if (have == 0 && ehf_vtype_info(vtype, &size, &kind) == 0) {
        return 1;
    }
    size /= 4;
    if (verts == 0) {
        unsigned int e = (unsigned int)(list + count * size) + 8;

        cmd[0] = e >> 8 & 0xFF0000 | 0x10000000;
        cmd[1] = e & 0xFFFFFF | 0x08000000;
        list = dl_push(list, cmd, 2);
        memset(list, 0, size * count);
        v = list;
        list += size * count;
    } else {
        v = verts;
    }
    list = draw_array(list, prim, vtype, count, 0, v);
    p->end = list - 1;
    p->verts = v;
    p->count = count;
    p->prim = prim;
    p->vtype = vtype;
    if (verts == 0) {
        p->ext = 0;
    } else {
        p->ext = 1;
    }
    postfx_set_param(list);
    return 0x80000000;
}

/* 0x10201: primitive arg0 of type arg1 with arg2 vertices in the list. */
int ehf_cmd_prim(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPrim *p = &ehf_prim;
    EhfArg *a = (EhfArg *)cmd->a.i;

    switch (phase) {
    case 6:
        p->end = 0;
        p->verts = 0;
        return 0;
    case 5:
        return ehf_prim_begin(a[0].i, a[1].i, a[2].i, 0, 0);
    }
    return 0;
}

/* 0x10202: same with typed arguments (prim, type, count, vertices). */
int ehf_cmd_prim_args(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPrim *p = &ehf_prim;
    int prim;
    int vtype;
    int count;
    int verts;
    void *out[4] = { &prim, &vtype, &count, &verts };
    char fmt[5] = "nnnn";

    switch (phase) {
    case 6:
        p->end = 0;
        p->verts = 0;
        return 0;
    case 5:
        ehf_cmd_args(0, cmd, 0, out, fmt);
        return ehf_prim_begin(prim, vtype, count, (void *)verts, 0);
    }
    return 0;
}

/* Point templates of ehf_cmd_prim_ribbon (in asm, so they are named; defined after the template of
   ehf_cmd_prim_args so that the data keeps its order). */
Vec4 ehf_ribbon_p0 = { 0.0f, 0.0f, 0.0f, 1.0f };
Vec4 ehf_ribbon_p1 = { 0.0f, 0.0f, 0.0f, 1.0f };
Vec4 ehf_ribbon_p2 = { 0.0f, 0.0f, 0.0f, 1.0f };
Vec4 ehf_ribbon_p3 = { 0.0f, 0.0f, 0.0f, 1.0f };

/* 0x25: primitive on the vertices in the command (arg3...). */
int ehf_cmd_prim_inline(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPrim *p = &ehf_prim;
    EhfArg *a = (EhfArg *)cmd->a.i;

    switch (phase) {
    case 6:
        p->end = 0;
        p->verts = 0;
        return 0;
    case 5:
        return ehf_prim_begin(a[0].i, a[1].i, a[2].i, &a[3], 1);
    }
    return 0;
}

/* 0x10203: texture coordinates of every vertex, from a format string arg0 ('V' two values, 'v' a
   pointer) repeated over the arguments. */
int ehf_cmd_prim_uv(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPrim *p = &ehf_prim;
    char *fmt;
    char *f;
    int *a;
    int *args;
    int i;

    switch (phase) {
    case 5:
        f = cmd->a.p[0];
        a = &cmd->a.i[1];
        args = a;
        fmt = f;
        for (i = 0; i < p->count; i++) {
            if (*f == 0) {
                f = fmt;
                a = args;
            }
            switch (*f++) {
            case 'v':
                ehf_vertex_set(p->vtype, 3, i, (void *)*a);
                a++;
                break;
            case 'V':
                ehf_vertex_set(p->vtype, 3, i, a);
                a += 2;
                break;
            }
        }
        return 0;
    }
    return 0;
}

#define EHF_RGBA_F(c) (((unsigned int)(c)[3] & 0xFF) << 24 | (((unsigned int)(c)[2] & 0xFF) << 16 | ((unsigned int)(c)[0] & 0xFF | ((unsigned int)(c)[1] & 0xFF) << 8)))
#define EHF_RGBA_B(b) ((b)[12] << 24 | ((b)[8] << 16 | ((b)[0] | (b)[4] << 8)))

/* 0x10204: vertex colors, from 'I' four integers, 'i' a pointer to them, 'V' four floats or 'v' a
   pointer to them (0..255). */
int ehf_cmd_prim_color(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPrim *p = &ehf_prim;
    char *fmt;
    char *f;
    int *a;
    int *args;
    int i;
    unsigned int color;

    switch (phase) {
    case 5:
        f = cmd->a.p[0];
        a = &cmd->a.i[1];
        args = a;
        fmt = f;
        for (i = 0; i < p->count; i++) {
            if (*f == 0) {
                f = fmt;
                a = args;
            }
            switch (*f++) {
            case 'v': {
                float *v = (float *)*a;

                color = EHF_RGBA_F(v);
                ehf_vertex_set(p->vtype, 0x1C, i, &color);
                a++;
                break;
            }
            case 'V':
                color = EHF_RGBA_F((float *)a);
                ehf_vertex_set(p->vtype, 0x1C, i, &color);
                a += 4;
                break;
            case 'i': {
                unsigned char *b = (unsigned char *)*a;

                color = EHF_RGBA_B(b);
                ehf_vertex_set(p->vtype, 0x1C, i, &color);
                a++;
                break;
            }
            case 'I':
                color = EHF_RGBA_B((unsigned char *)a);
                ehf_vertex_set(p->vtype, 0x1C, i, &color);
                a += 4;
                break;
            }
        }
        return 0;
    }
    return 0;
}

/* 0x10205: vertex positions, from 'V' three floats or 'v' a pointer to them. */
int ehf_cmd_prim_pos(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPrim *p = &ehf_prim;
    char *fmt;
    char *f;
    int *a;
    int *args;
    int i;

    switch (phase) {
    case 5:
        f = cmd->a.p[0];
        a = &cmd->a.i[1];
        args = a;
        fmt = f;
        for (i = 0; i < p->count; i++) {
            if (*f == 0) {
                f = fmt;
                a = args;
            }
            switch (*f++) {
            case 'v':
                ehf_vertex_set(p->vtype, 0x180, i, (void *)*a);
                a++;
                break;
            case 'V':
                ehf_vertex_set(p->vtype, 0x180, i, a);
                a += 3;
                break;
            }
        }
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* the original keeps each point in a temporary before copying it; mwcc 3.0.1 propagates the copy */
/* 0x10206: a ribbon of half width arg ('F' value or 'f' pointer) along the points that follow ('V'
   three floats or 'v' a pointer), facing the camera: two vertices per point, swapped whenever the
   side normal turns over. */
int ehf_cmd_prim_ribbon(void *ctx, EhfNode *node, EhfCmd *cmd, int phase)
{
    EhfPrim *p = &ehf_prim;
    Vec4 cur;
    Vec4 prev;
    Vec4 pdir;
    Vec4 dir;
    Vec4 view;
    Vec4 n;
    Vec4 side[2];
    Vec4 pn;
    char *fmt;
    char *f;
    int *args;
    int *a;
    float w;
    int flip;
    int i;
    int k;

    switch (phase) {
    case 5:
        fmt = cmd->a.p[0];
        args = &cmd->a.i[1];
        a = args;
        f = fmt + 1;
        switch (fmt[0]) {
        case 'f':
            w = *(float *)*a;
            a = args + 1;
            break;
        case 'F':
            w = *(float *)a;
            a = args + 1;
            break;
        }
        switch (*f++) {
        case 'v': {
            float *v = (float *)*a;
            Vec4 t = { v[0], v[1], v[2], 1.0f };

            cur = t;
            a++;
            break;
        }
        case 'V': {
            float *v = (float *)a;
            Vec4 t = { v[0], v[1], v[2], 1.0f };

            cur = t;
            a += 3;
            break;
        }
        }
        mtx_get_view_dir(&view);
        flip = 0;
        for (i = 1; i < p->count / 2; i++) {
            prev = cur;
            if (*f == 0) {
                f = fmt + 2;
                a = args + 2;
            }
            if (i < p->count / 2) {
                switch (*f++) {
                case 'v': {
                    float *v = (float *)*a;
                    Vec4 t = { v[0], v[1], v[2], 1.0f };

                    cur = t;
                    a++;
                    break;
                }
                case 'V': {
                    float *v = (float *)a;
                    Vec4 t = { v[0], v[1], v[2], 1.0f };

                    cur = t;
                    a += 3;
                    break;
                }
                }
                if (i == 1) {
                    vec3_sub_w(&dir, &cur, &prev);
                    pdir = dir;
                } else {
                    pdir = dir;
                    vec3_sub_w(&dir, &cur, &prev);
                }
            } else {
                pdir = dir;
            }
            vec3_cross(&n, &dir, &view);
            vec3_normalize2(&n, &n);
            vec3_cross(&pn, &pdir, &view);
            vec3_normalize2(&pn, &pn);
            if (vec3_dot2(&pn, &n) < 0.0f) {
                flip ^= 1;
            }
            vec3_scale_w(&n, &n, w);
            vec3_add_w(&side[0], &prev, &n);
            vec3_sub_w(&side[1], &prev, &n);
            k = (i - 1) * 2;
            ehf_vertex_set(p->vtype, 0x180, k, &side[flip & 1]);
            ehf_vertex_set(p->vtype, 0x180, k + 1, &side[(flip + 1) & 1]);
        }
        return 0;
    }
    return 0;
}
#endif
