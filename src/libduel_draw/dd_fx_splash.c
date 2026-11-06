// @range 0x0885036C-0x088513A4
// @data 0x08876DA0-0x08876DB8
/* Character effect "splash" (a 420-frame loop): during two windows of the loop, water hits two
   fixed points and spawns spreading ripples (sen_07.gim), rising spray rings (nsea.gim) and
   droplets thrown upwards (PT35.gim). */
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
void mtx_translate(FxVec3 *v);
void mtx_mult(FxMat4 *m);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
FxVec3 *vec3_add(FxVec3 *out, FxVec3 *a, FxVec3 *b);
FxVec3 *vec3_copy(FxVec3 *out, FxVec3 *v);
float sinf(float x);
float cosf(float x);

float dd_ease_sin(int t, int len, int n);
float dd_ease_cos(int t, int len, int n);
void dd_mtx_push_field(void);
void dd_mtx_pop_all(void);
void dd_mtx_billboard_field(FxMat4 *out);
void dd_translate3(float x, float y, float z);
void dd_scale3(float x, float y, float z);
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

typedef struct DdRipple {              /* 0x20 */
    unsigned int flags;
    int t;
    int life;
    float u, v;                        /* texture offset */
    FxVec3 pos;
} DdRipple;

typedef struct DdDropPart {            /* 0x1C */
    int cell;
    FxVec3 pos;
    FxVec3 dir;
} DdDropPart;

typedef struct DdDrop {                /* 0x7C */
    unsigned int flags;
    int t;
    int life;
    DdDropPart p[4];
} DdDrop;

typedef struct DdFxSplash {            /* 0x24AC */
    int arg;                           /* +0x0000 */
    int frame;                         /* +0x0004 0..419 */
    int ripple_cur;                    /* +0x0008 */
    FxVtx ripple_v[17][3];             /* +0x000C */
    unsigned char ripple_idx[34];      /* +0x04D4 */
    DdRipple ripple[48];               /* +0x04F8 */
    int spray_cur;                     /* +0x0AF8 */
    FxVtx spray_v[17][3];              /* +0x0AFC */
    unsigned char spray_idx[34];       /* +0x0FC4 */
    DdRipple spray[32];                /* +0x0FE8 */
    int drop_cur;                      /* +0x13E8 */
    FxVtxT cell[8][2];                 /* +0x13EC */
    DdDrop drop[32];                   /* +0x152C */
} DdFxSplash;

int dd_fx_splash_update(int phase, DdFxSplash *w);

int dd_fx_splash_start(int arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_splash_update, a, b, sizeof(DdFxSplash));

    if (task != -1)
        ((DdFxSplash *)dd_task_mem(task))->arg = arg;
    return task;
}

int dd_fx_splash_update(int phase, DdFxSplash *w)
{
    int on = 0;
    void *list;
    FxMat4 cam;
    FxVec3 pos;
    FxVec3 p;
    DdRipple *r;
    DdDrop *d;
    float f;
    float u;
    float sn;
    float cs;
    float ang;
    float rad;
    float s;
    float k;
    float y;
    float one;
    float a;
    float hr;
    float cw;
    float ch;
    float cu;
    float cv;
    DdDropPart *pj;
    int i;
    int j;
    int i3;
    int j2;
    int i2;
    void *dlist;
    int j3;
    DdDrop *dd;

    dd_mtx_push_field();
    one = 1.0f;
    dd_mtx_billboard_field(&cam);
    if (w->frame >= 308 && w->frame < 319) {
        FxVec3 a = { 0.392f, 0.803f, -2.536f };

        on = 1;
        pos = *(FxVec3 *)&a;
    }
    if (w->frame >= 346 && w->frame < 357) {
        FxVec3 b = { 1.359f, 0.803f, -1.61f };

        on = 1;
        pos = *(FxVec3 *)&b;
    }
    if (phase == 1) {
        for (i = 0; i < 17; i++) {
            f = (float)i / 16.0f * 6.2831855f;
            u = (float)i / 16.0f * 3.0f;
            sn = sinf(f);
            cs = cosf(f);
            w->ripple_v[i][0].u = u; w->ripple_v[i][0].v = 0.0f; w->ripple_v[i][0].color = 0xFFFFFF;
            w->ripple_v[i][0].x = sn * 0.5f; w->ripple_v[i][0].y = 0.0f; w->ripple_v[i][0].z = cs * 0.5f;
            w->ripple_v[i][1].u = u; w->ripple_v[i][1].v = 0.24000001f; w->ripple_v[i][1].color = 0x30FFFFFF;
            w->ripple_v[i][1].x = sn * 1.7f; w->ripple_v[i][1].y = 0.0f; w->ripple_v[i][1].z = cs * 1.7f;
            w->ripple_v[i][2].u = u; w->ripple_v[i][2].v = 0.3f; w->ripple_v[i][2].color = 0xFFFFFF;
            w->ripple_v[i][2].x = sn * 2.0f; w->ripple_v[i][2].y = 0.0f; w->ripple_v[i][2].z = cs * 2.0f;
            w->ripple_idx[i * 2] = i * 3;
            w->ripple_idx[i * 2 + 1] = i * 3 + 1;
        }
    }
    if (on == 1) {
        r = &w->ripple[w->ripple_cur];
        r->flags |= 1;
        r->t = 0;
        r->life = (RAND15() % 16) + 32;
        r->u = RAND15() / 32767.0f;
        r->v = RAND15() / 32767.0f;
        vec3_copy(&r->pos, &pos);
        if (w->ripple_cur < 47)
            w->ripple_cur++;
        else
            w->ripple_cur = 0;
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 12);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.0f, 0.01f, 0.0f);
    for (j = 0; j < 48; j++) {
        r = &w->ripple[j];
        if (!(r->flags & 1) != 1) {
            list = dl_ambient_light(list, (unsigned int)(dd_ease_cos(r->t, r->life, 1) * 255.0f) << 24 | 0xFFFFFF);
            list = dl_tex_offset(r->u, r->v, list);
            mtx_push();
            mtx_translate(&r->pos);
            s = dd_ease_sin(r->t, r->life, 1);
            dd_scale3(s, 0.0f, s);
            list = draw_array(list, 4, 0x99F, 34, w->ripple_idx, w->ripple_v[0]);
            list = draw_array(list, 4, 0x99F, 34, w->ripple_idx, &w->ripple_v[0][1]);
            mtx_pop();
            if (r->t < r->life - 1)
                r->t++;
            else
                r->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    if (phase == 1) {
        for (i2 = 0; i2 < 17; i2++) {
            f = (float)i2 / 16.0f * 6.2831855f;
            u = one * ((float)i2 / 16.0f);
            sn = sinf(f);
            cs = cosf(f);
            w->spray_v[i2][0].u = u; w->spray_v[i2][0].v = 0.0f; w->spray_v[i2][0].color = 0xFFFFFF;
            w->spray_v[i2][0].x = sn * 0.3f; w->spray_v[i2][0].y = 0.0f; w->spray_v[i2][0].z = cs * 0.3f;
            w->spray_v[i2][1].u = u; w->spray_v[i2][1].v = 0.5f; w->spray_v[i2][1].color = 0x80FFFFFF;
            w->spray_v[i2][1].x = sn * 0.55f; w->spray_v[i2][1].y = 0.4f; w->spray_v[i2][1].z = cs * 0.55f;
            w->spray_v[i2][2].u = u; w->spray_v[i2][2].v = 1.0f; w->spray_v[i2][2].color = 0xFFFFFF;
            w->spray_v[i2][2].x = sn * 0.8f; w->spray_v[i2][2].y = 0.8f; w->spray_v[i2][2].z = cs * 0.8f;
            w->spray_idx[i2 * 2] = i2 * 3;
            w->spray_idx[i2 * 2 + 1] = i2 * 3 + 1;
        }
    }
    if (on == 1) {
        r = &w->spray[w->spray_cur];
        r->flags |= 1;
        r->t = 0;
        r->life = (RAND15() % 16) + 16;
        r->u = RAND15() / 32767.0f;
        r->v = RAND15() / 32767.0f;
        vec3_copy(&r->pos, &pos);
        if (w->spray_cur < 31)
            w->spray_cur++;
        else
            w->spray_cur = 0;
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 0, 0, 0);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 9);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.0f, -0.3f, 0.0f);
    for (j2 = 0; j2 < 32; j2++) {
        r = &w->spray[j2];
        if (!(r->flags & 1) != 1) {
            list = dl_ambient_light(list, (unsigned int)(dd_ease_cos(r->t, r->life, 1) * 255.0f) << 24 | 0xFFFFFF);
            list = dl_tex_offset(r->u, r->v, list);
            mtx_push();
            mtx_translate(&r->pos);
            s = dd_ease_sin(r->t, r->life, 1) * 0.5f + 1.0f;
            dd_scale3(s, dd_ease_cos(r->t, r->life, 1), s);
            list = draw_array(list, 4, 0x99F, 34, w->spray_idx, w->spray_v[0]);
            list = draw_array(list, 4, 0x99F, 34, w->spray_idx, &w->spray_v[0][1]);
            mtx_pop();
            if (r->t < r->life - 1)
                r->t++;
            else
                r->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    if (phase == 1) {
        hr = 0.4f;
        cw = 0.25f;
        ch = 0.5f;
        for (i3 = 0; i3 < 8; i3++) {
            cu = (float)(i3 % 4) * cw;
            cv = (float)(i3 / 4) * ch;
            w->cell[i3][0].u = cu;
            w->cell[i3][0].v = cv;
            w->cell[i3][0].x = -hr;
            w->cell[i3][0].y = hr;
            w->cell[i3][0].z = 0.0f;
            w->cell[i3][1].u = cu + cw;
            w->cell[i3][1].v = cv + ch;
            w->cell[i3][1].x = hr;
            w->cell[i3][1].y = -hr;
            w->cell[i3][1].z = 0.0f;
        }
    }
    if (on == 1) {
        d = &w->drop[w->drop_cur];
        d->flags |= 1;
        d->t = 0;
        d->life = (RAND15() % 16) + 16;
        for (j = 0; j < 4; j++) {
            pj = &d->p[j];
            ang = RAND15() / 32767.0f * 6.2831855f;
            rad = RAND15() / 32767.0f * 0.15f + 0.15f;
            pj->cell = RAND15() % 8;
            vec3_copy(&pj->pos, &pos);
            y = RAND15() / 32767.0f;
            vec3_set(&pj->dir, rad * sinf(ang), y * 1.5f + 0.5f, rad * cosf(ang));
        }
        if (w->drop_cur < 31)
            w->drop_cur++;
        else
            w->drop_cur = 0;
    }
    dlist = dl_select_layer(1, 0);
    dlist = dd_tex_enable(dlist, 1, 1, 1, 0, 0, 1, 1);
    dlist = dd_depth_enable(dlist, 1, 1);
    dlist = dd_blend_enable(dlist, 1, 0, 6, 10, 0, 0xFFFFFFFF);
    dlist = dd_tex_pack(dlist, 32);
    dlist = dl_color(dlist, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.0f, -0.3f, 0.0f);
    for (i = 0; i < 32; i++) {
        dd = &w->drop[i];
        if (!(dd->flags & 1) != 1) {
            s = dd_ease_sin(dd->t, dd->life, 1);
            a = dd_ease_cos(dd->t, dd->life, 1);
            k = sinf((float)dd->t * 2.4434612f / (float)dd->life);
            f = s + 1.0f;
            dlist = dl_ambient_light(dlist, (unsigned int)(a * 255.0f) << 24 | 0xFFFFFF);
            for (j3 = 0; j3 < 4; j3++) {
                pj = &dd->p[j3];
                vec3_set(&p, pj->dir.x * f, pj->dir.y * k, pj->dir.z * f);
                vec3_add(&p, &p, &pj->pos);
                mtx_push();
                mtx_translate(&p);
                mtx_mult(&cam);
                dd_scale3(s, s, 0.0f);
                dlist = draw_array(dlist, 6, 0x183, 2, 0, w->cell[pj->cell]);
                mtx_pop();
            }
            if (dd->t < dd->life - 1)
                dd->t++;
            else
                dd->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(dlist);
    dd_mtx_pop_all();
    if (w->frame < 419)
        w->frame++;
    else
        w->frame = 0;
    return 2;
}
