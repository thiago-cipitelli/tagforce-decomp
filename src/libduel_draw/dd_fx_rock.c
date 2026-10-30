// @range 0x088513A4-0x088522F0
// @asm 0x088513A4-0x088517DC
// @asm 0x08851834-0x088522F0
/* Character effect "whirlpool": a tilted swirl of water (nsea.gim/sen_07.gim scrolled), rocks
   (082rock.gim) falling into it and two rings of spray (PT88_R_a.gim) on both sides. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_color(void *list, unsigned int color);
void *dl_ambient_light(void *list, unsigned int color);
void *dl_tex_offset(float u, float v, void *list);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_load(FxMat4 *m);
void mtx_translate(FxVec3 *v);
void mtx_mult(FxMat4 *m);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
FxVec4 *vec4_set(FxVec4 *out, float x, float y, float z, float w);
FxVec4 *vec3_normalize2(FxVec4 *out, FxVec4 *v);
FxVec4 *vec3_cross(FxVec4 *out, FxVec4 *a, FxVec4 *b);
FxVec3 *mat3_transform_vec3(FxVec3 *out, float *m, FxVec3 *v);
float fabsf_vfpu(float x);
float sinf(float x);
float cosf(float x);

float dd_ease_lin(int t, int len);
float dd_ease_cos(int t, int len, int n);
float dd_ease_cos_inv(int t, int len, int n);
void dd_mtx_push_field(void);
void dd_mtx_pop_all(void);
void dd_mtx_billboard_field(FxMat4 *out);
void dd_translate3(float x, float y, float z);
void dd_scale3(float x, float y, float z);
void dd_mat3_rot_x(float a, float *out, float *m);
void dd_field_dir_get(FxVec4 *up);
int dd_task_create(void *update, void *a, void *b, int size);
void *dd_task_mem(int task);
void *dd_depth_enable(void *list, int on, int mask);
void *dd_blend_enable(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dd_tex_pack(void *list, int tex);
void *dd_tex_enable(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);

typedef struct RandSeed {              /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;
    unsigned int b, c;
} RandSeed;
extern int rand3_slot;                 /* BOOT variable imports, 0 in the relocated ELF */
extern RandSeed rand3_seeds[];

#define RAND15() (rand3_seeds[rand3_slot].lcg *= 0x41C64E6D, rand3_seeds[rand3_slot].lcg += 0x3039, \
                  (int)(rand3_seeds[rand3_slot].lcg >> 16 & 0x7FFF))

typedef struct DdPart {                /* 0x18 */
    unsigned int flags;
    int t;
    int cell;
    FxVec3 dir;
} DdPart;

typedef struct DdFxRock {              /* 0x1680 */
    int arg;                           /* +0x0000 */
    int frame;                         /* +0x0004 */
    int pad08[2];
    FxMat4 mtx;                        /* +0x0010 swirl plane */
    float scroll;                      /* +0x0050 */
    FxVtx swirl[17][2];                /* +0x0054 */
    float tilt[9];                     /* +0x0384 */
    int spray_cur[2];                  /* +0x03A8 */
    FxVtx cell[8][6];                  /* +0x03B0 */
    DdPart spray[2][64];               /* +0x0830 */
    int rock_cur;                      /* +0x1430 */
    FxVtxT rock_cell[5][2];            /* +0x1434 */
    DdPart rock[16];                   /* +0x14FC */
    int pad167c;
} DdFxRock;

int dd_fx_rock_update(int phase, DdFxRock *w);

#ifdef NON_MATCHING /* corpo de laço com índice circular e duas fontes; registradores diferentes */
/* Ring of spray on side `side` (0 front, 1 back): 64 particles thrown around the swirl. */
void dd_fx_rock_spray(int phase, int side, DdFxRock *w, FxMat4 *cam)
{
    DdPart *p;
    int *cur;
    int c;
    float base;
    float ang;
    float s;
    float y;
    FxVec3 v;
    void *list;
    int n;
    int i;

    if (side == 1) {
        p = w->spray[0];
        cur = &w->spray_cur[0];
    } else {
        p = w->spray[1];
        cur = &w->spray_cur[1];
    }
    c = *cur;
    base = side == 1 ? 0.0f : 3.1415927f;
    n = phase == 1 ? 64 : 1;
    for (i = 0; i < n; i++) {
        DdPart *q = &p[c];

        q->flags |= 1;
        q->t = n - 1 - i;
        ang = base + RAND15() / 32767.0f * 3.1415927f;
        q->cell = RAND15() % 8;
        vec3_set(&q->dir, cosf(ang) * 2.5f, 0.5f, sinf(ang) * 2.5f);
        if (c < 63)
            c++;
        else
            c = 0;
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 0, 1);
    list = dd_blend_enable(list, 1, 0, 6, 7, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 5);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.0f, 3.0f, 0.0f);
    for (i = 0; i < 64; i++, p++) {
        if (p->flags & 1) {
            s = dd_ease_lin(p->t, 64) * 0.9f + 0.1f;
            y = dd_ease_cos_inv(p->t, 64, 2);
            list = dl_ambient_light(list, (unsigned int)(dd_ease_lin(p->t, 8) * 255.0f *
                                                         dd_ease_cos(p->t - 56, 8, 1)) << 24 | 0xFFFFFF);
            mtx_push();
            vec3_set(&v, p->dir.x * s, p->dir.y * y, p->dir.z * s);
            mat3_transform_vec3(&v, w->tilt, &v);
            mtx_translate(&v);
            mtx_mult(cam);
            list = draw_array(list, 4, 0x19F, 6, 0, w->cell[p->cell]);
            mtx_pop();
            if (p->t < 63)
                p->t++;
            else
                p->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    *cur = c;
}
#endif

int dd_fx_rock_start(int arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_rock_update, a, b, sizeof(DdFxRock));

    if (task != -1)
        ((DdFxRock *)dd_task_mem(task))->arg = arg;
    return task;
}

#ifdef NON_MATCHING /* função de 2,7 KB; C equivalente */
int dd_fx_rock_update(int phase, DdFxRock *w)
{
    FxMat4 cam;
    void *list;
    float f;
    float u;
    float sn;
    float cs;
    float k;
    float ang;
    unsigned int c;
    FxVec3 v;
    int i;
    int j;
    int n;

    dd_mtx_push_field();
    dd_mtx_billboard_field(&cam);
    if (phase == 1) {
        float rows[3] = { 0.0f, 0.6f, 1.0f };
        FxVec4 dir = { 0.0f, 1.0f, 0.0f, 1.0f };
        FxVec4 up;
        FxVec4 x;
        FxVec4 y;
        FxVec4 z;
        FxVec3 m[3];

        for (i = 0; i < 8; i++) {
            for (j = 0; j < 3; j++) {
                f = (float)(i / 4) * 0.5f * rows[j];
                k = 0.8f - rows[j] * 1.6f;
                c = j > 1 ? 0xFF1F569C : 0xFF00141A;
                w->cell[i][j * 2].u = (float)(i % 4) * 0.25f;
                w->cell[i][j * 2].v = f;
                w->cell[i][j * 2].color = c;
                w->cell[i][j * 2].x = -0.8f;
                w->cell[i][j * 2].y = k;
                w->cell[i][j * 2].z = 0.0f;
                w->cell[i][j * 2 + 1].u = (float)(i % 4) * 0.25f + 0.25f;
                w->cell[i][j * 2 + 1].v = f;
                w->cell[i][j * 2 + 1].color = c;
                w->cell[i][j * 2 + 1].x = 0.8f;
                w->cell[i][j * 2 + 1].y = k;
                w->cell[i][j * 2 + 1].z = 0.0f;
            }
        }
        dd_field_dir_get(&up);
        vec3_normalize2(&y, &dir);
        vec3_cross(&x, &y, &up);
        vec3_normalize2(&x, &x);
        vec3_cross(&z, &x, &y);
        vec3_normalize2(&z, &z);
        vec3_set(&m[0], x.x, x.y, x.z);
        vec3_set(&m[1], y.x, y.y, y.z);
        vec3_set(&m[2], z.x, z.y, z.z);
        dd_mat3_rot_x(-0.2617994f, w->tilt, (float *)m);
        vec4_set((FxVec4 *)&w->mtx.m[0], x.x, x.y, x.z, 0.0f);
        vec4_set((FxVec4 *)&w->mtx.m[4], y.x, y.y, y.z, 0.0f);
        vec4_set((FxVec4 *)&w->mtx.m[8], z.x, z.y, z.z, 0.0f);
        vec4_set((FxVec4 *)&w->mtx.m[12], 0.0f, 1.5f, 0.0f, 1.0f);
    }
    dd_fx_rock_spray(phase, 0, w, &cam);
    if (phase == 1) {
        for (i = 0; i < 17; i++) {
            f = (float)i / 16.0f * 3.1415927f;
            sn = sinf(f);
            cs = cosf(f);
            u = (float)i / 16.0f * 1.0f;
            w->swirl[i][0].u = u; w->swirl[i][0].v = 0.0f;
            w->swirl[i][0].color = (unsigned int)(fabsf_vfpu(sn) * 255.0f) << 24 | 0x778D3;
            w->swirl[i][0].x = cs * 0.3f; w->swirl[i][0].y = 0.0f; w->swirl[i][0].z = sn * 0.3f;
            w->swirl[i][1].u = u; w->swirl[i][1].v = 1.0f; w->swirl[i][1].color = 0x778D3;
            w->swirl[i][1].x = cs * 0.4f; w->swirl[i][1].y = 1.2f; w->swirl[i][1].z = sn * 0.4f;
        }
    }
    list = dl_select_layer(1, 0);
    list = dd_depth_enable(list, 1, 1);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&w->mtx);
    list = dl_ambient_light(list, 0xFFFFFFFF);
    list = dd_tex_enable(list, 1, 1, 1, 4, 0, 0, 0);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 9);
    list = dl_tex_offset(0.0f, w->scroll, list);
    mtx_push();
    dd_scale3(0.85f, 1.0f, 0.85f);
    list = draw_array(list, 4, 0x19F, 34, 0, w->swirl);
    mtx_pop();
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 12);
    list = dl_tex_offset(0.0f, w->scroll, list);
    list = draw_array(list, 4, 0x19F, 34, 0, w->swirl);
    w->scroll -= 0.02f;
    if (w->scroll < -1.0f)
        w->scroll += 1.0f;
    mtx_pop();
    dl_set_list(list);
    if (phase == 1) {
        float size[5] = { 0.05f, 0.05f, 0.05f, 0.025f, 0.025f };
        float u0[5] = { 0.0f, 0.25f, 0.5f, 0.75f, 0.875f };
        float u1[5] = { 0.25f, 0.5f, 0.75f, 0.875f, 1.0f };
        float v1[5] = { 1.0f, 1.0f, 1.0f, 0.5f, 0.5f };

        for (i = 0; i < 5; i++) {
            w->rock_cell[i][0].u = u0[i]; w->rock_cell[i][0].v = 0.0f;
            w->rock_cell[i][0].x = -size[i]; w->rock_cell[i][0].y = size[i]; w->rock_cell[i][0].z = 0.0f;
            w->rock_cell[i][1].u = u1[i]; w->rock_cell[i][1].v = v1[i];
            w->rock_cell[i][1].x = size[i]; w->rock_cell[i][1].y = -size[i]; w->rock_cell[i][1].z = 0.0f;
        }
    }
    if (w->frame % 4 == 0) {
        n = phase == 1 ? 16 : 1;
        for (i = 0; i < n; i++) {
            DdPart *r = &w->rock[w->rock_cur];

            r->flags |= 1;
            r->t = (n - 1 - i) * 4;
            ang = RAND15() / 32767.0f * 6.2831855f;
            r->cell = RAND15() % 5;
            vec3_set(&r->dir, cosf(ang) * 2.0f, -3.0f, sinf(ang) * 2.0f);
            if (w->rock_cur < 15)
                w->rock_cur++;
            else
                w->rock_cur = 0;
        }
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 25);
    list = dl_color(list, 0xFF9B9B9B);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.0f, 3.5f, 0.0f);
    for (i = 0; i < 16; i++) {
        DdPart *r = &w->rock[i];

        if (r->flags & 1) {
            k = dd_ease_cos_inv(r->t, 64, 1);
            list = dl_ambient_light(list, (unsigned int)(dd_ease_lin(r->t, 8) * 255.0f *
                                                         dd_ease_cos(r->t - 56, 8, 1)) << 24 | 0xFFFFFF);
            mtx_push();
            vec3_set(&v, r->dir.x, r->dir.y * k, r->dir.z);
            mtx_translate(&v);
            mtx_mult(&cam);
            list = draw_array(list, 6, 0x183, 2, 0, w->rock_cell[r->cell]);
            mtx_pop();
            if (r->t < 63)
                r->t++;
            else
                r->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    dd_fx_rock_spray(phase, 1, w, &cam);
    dd_mtx_pop_all();
    w->frame++;
    return 2;
}
#endif
