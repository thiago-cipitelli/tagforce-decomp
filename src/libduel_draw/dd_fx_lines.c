// @range 0x08847D0C-0x08848B40
// @asm 0x08847DD8-0x08848B40
/* Card effect "speed lines" (effects 102-106): a tunnel of 33x33 vertices turning towards the
   camera (sen_07.gim), bursts of 8 sparks flying outwards (blu_light_g.gim) and lens flares
   (lens_aP8A.gim), with the whoosh sound 0x6237 when it starts and ends. */
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
void mtx_rotate_z(float a);
void mtx_translate(FxVec3 *v);
float sinf(float x);
float cosf(float x);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
FxVec3 *vec3_scale(FxVec3 *out, FxVec3 *v, float s);

float func_088279A4(int t, int len);
float func_088279D8(int t, int len);
float func_08827A2C(int t, int len, int n);
float func_08827AD4(int t, int len, int n);
void func_088272BC(float x, float y, float z);
void func_08826E30(int layer, int a, int b);
void func_08826FEC(int layer);
int func_08828028(int se, int t, int at, int len, unsigned int *flags, unsigned int bit);
int func_0882810C(int se, int t, int at, int len, unsigned int *flags, unsigned int bit);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
void *func_0882698C(void *list, int on, int mask);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *func_08826A64(void *list, int tex);
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);

typedef struct RandSeed {              /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;
    unsigned int b, c;
} RandSeed;
extern int rand3_slot;                 /* BOOT variable imports, 0 in the relocated ELF */
extern RandSeed rand3_seeds[];

#define RAND15() (rand3_seeds[rand3_slot].lcg *= 0x41C64E6D, rand3_seeds[rand3_slot].lcg += 0x3039, \
                  (int)(rand3_seeds[rand3_slot].lcg >> 16 & 0x7FFF))

typedef struct DdLinesSpark {          /* burst of 8 sparks, 0x88 */
    unsigned int flags;                /* bit 0 alive */
    int t;
    struct {
        int cell;                      /* +0x08 cell of the texture */
        FxVec3 dir;                    /* +0x0C */
    } p[8];
} DdLinesSpark;

typedef struct DdLinesFlare {          /* 0x10 */
    unsigned int flags;
    int t;
    float scale;
    float angle;
} DdLinesFlare;

typedef struct DdFxLines {             /* 0x7150 */
    DdFxHead h;
    unsigned int se;                   /* +0x0018 sounds played */
    FxVtx ring[33][33];                /* +0x001C */
    unsigned char idx[66];             /* +0x6634 */
    int spark_cur;                     /* +0x6678 */
    FxVtxT cell[8][2];                 /* +0x667C sprite of each texture cell */
    DdLinesSpark spark[16];            /* +0x67BC */
    int flare_cur;                     /* +0x703C */
    FxVtx flare_v[6];                  /* +0x7040 */
    DdLinesFlare flare[8];             /* +0x70D0 */
} DdFxLines;

int dd_fx_lines_update(int phase, DdFxLines *w);

extern int dd_fx_lines_arg;

void dd_fx_lines_times(int id, int *delay, int *len)
{
    switch (id) {
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
        *delay = 10;
        break;
    default:
        *delay = 0;
        *len = 0;
        return;
    }
    *len = 64;
}

void dd_fx_lines_set(int v)
{
    dd_fx_lines_arg = v;
}

int dd_fx_lines_start(DdFxArg *arg, void *a, void *b)
{
    int task = func_0882623C(dd_fx_lines_update, a, b, sizeof(DdFxLines));

    if (task != -1)
        ((DdFxLines *)func_08826440(task))->h.arg = arg;
    return task;
}

#ifdef NON_MATCHING /* função de 3,4 KB: escalonamento dos laços de vértices e das partículas diferente (811 palavras); C equivalente */
int dd_fx_lines_update(int phase, DdFxLines *w)
{
    DdFxArg *arg = w->h.arg;
    int run = arg->run;
    void *list;
    float in;
    float out;
    float fr;
    float rad;
    float fc;
    float ang;
    float sn;
    float cs;
    float dist;
    float a;
    float s;
    unsigned int alpha;
    DdLinesSpark *sp;
    DdLinesFlare *f;
    FxVtx *v;
    FxVec3 p;
    int r1;
    int r2;
    int r3;
    int i;
    int j;

    func_08826E30(7, 0xF0, 0x88);
    if (phase == 1)
        dd_fx_lines_times(arg->id, &w->h.delay, &w->h.len);
    in = func_088279A4(w->h.t, 16);
    out = func_088279D8(w->h.t - (w->h.len - 16), 16);
    func_08828028(0x6237, w->h.wait, w->h.delay, 16, &w->se, 1);
    func_0882810C(0x6237, w->h.t, w->h.len - 16, 16, &w->se, 2);
    if (phase == 1) {
        for (i = 0; i < 33; i++) {
            fr = (float)i / 32.0f;
            rad = fr * -500.0f + 800.0f;
            alpha = (1.0f - sinf(fr * 1.5707964f)) * 200.0f;
            for (j = 0; j < 33; j++) {
                fc = (float)j / 32.0f;
                ang = fc * 6.2831855f;
                sn = sinf(ang);
                cs = cosf(ang);
                v = &w->ring[i][j];
                v->u = fc * 6.0f;
                v->v = fr * 0.2f;
                v->color = alpha << 24 | 0xFFB982;
                v->x = rad * sn;
                v->y = rad * cs;
                v->z = 0.0f;
            }
        }
        for (i = 0; i < 33; i++) {
            w->idx[i * 2] = i;
            w->idx[i * 2 + 1] = i + 33;
        }
    }
    if (w->h.wait == w->h.delay) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = func_0882698C(list, 0, 0);
        list = func_088269D8(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
        list = func_08826A64(list, 12);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
        for (i = 0; i < 2; i++) {
            list = dl_tex_offset(0.0f, (float)w->h.frame * -0.03f + (float)i / 2.0f, list);
            mtx_push();
            mtx_rotate_z((float)i * 0.5235988f + 0.2617994f);
            for (j = 0; j < 32; j++)
                list = draw_array(list, 4, 0x99F, 66, w->idx, w->ring[j]);
            mtx_pop();
        }
        mtx_pop();
        dl_set_list(list);
    }
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            w->cell[i][0].u = (float)(i % 4) * 0.25f;
            w->cell[i][0].v = (float)(i / 4) * 0.5f;
            w->cell[i][0].x = -20.0f;
            w->cell[i][0].y = 20.0f;
            w->cell[i][0].z = 0.0f;
            w->cell[i][1].u = (float)(i % 4) * 0.25f + 0.25f;
            w->cell[i][1].v = (float)(i / 4) * 0.5f + 0.5f;
            w->cell[i][1].x = 20.0f;
            w->cell[i][1].y = -20.0f;
            w->cell[i][1].z = 0.0f;
        }
    }
    if (w->h.wait == w->h.delay) {
        sp = &w->spark[w->spark_cur];
        sp->flags |= 1;
        sp->t = 0;
        for (j = 0; j < 8; j++) {
            r1 = RAND15();
            r2 = RAND15();
            r3 = RAND15();
            dist = r1 / 32767.0f * 300.0f + 100.0f;
            ang = r2 / 32767.0f * 6.2831855f;
            sp->p[j].cell = r3 % 8;
            vec3_set(&sp->p[j].dir, dist * sinf(ang), dist * cosf(ang), 0.0f);
        }
        if (w->spark_cur < 15)
            w->spark_cur++;
        else
            w->spark_cur = 0;
    }
    if (w->h.wait == w->h.delay) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        list = func_08826A64(list, 14);
        list = dl_color(list, 0xFFFF8764);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        for (i = 0; i < 16; i++) {
            sp = &w->spark[i];
            if (sp->flags & 1) {
                a = out * in * sinf((float)sp->t * 3.1415927f / 16.0f) * 255.0f;
                s = func_088279D8(sp->t, 16);
                list = dl_ambient_light(list, (unsigned int)a << 24 | 0xFFFFFF);
                for (j = 0; j < 8; j++) {
                    vec3_scale(&p, &sp->p[j].dir, s + 0.5f);
                    mtx_push();
                    mtx_translate(&p);
                    list = draw_array(list, 6, 0x183, 2, 0, w->cell[sp->p[j].cell]);
                    mtx_pop();
                }
                if (sp->t < 15)
                    sp->t++;
                else
                    sp->flags &= ~1;
            }
        }
        mtx_pop();
        dl_set_list(list);
    }
    if (phase == 1) {
        w->flare_v[0].u = 0.5f; w->flare_v[0].v = 0.5f; w->flare_v[0].color = 0xFFFFEBC8;
        w->flare_v[0].x = 0.0f; w->flare_v[0].y = 0.0f; w->flare_v[0].z = 0.0f;
        w->flare_v[1].u = 0.0f; w->flare_v[1].v = 0.0f; w->flare_v[1].color = 0xFFEBC8;
        w->flare_v[1].x = -200.0f; w->flare_v[1].y = 200.0f; w->flare_v[1].z = 0.0f;
        w->flare_v[2].u = 1.0f; w->flare_v[2].v = 0.0f; w->flare_v[2].color = 0xFFEBC8;
        w->flare_v[2].x = 200.0f; w->flare_v[2].y = 200.0f; w->flare_v[2].z = 0.0f;
        w->flare_v[3].u = 1.0f; w->flare_v[3].v = 1.0f; w->flare_v[3].color = 0xFFEBC8;
        w->flare_v[3].x = 200.0f; w->flare_v[3].y = -200.0f; w->flare_v[3].z = 0.0f;
        w->flare_v[4].u = 0.0f; w->flare_v[4].v = 1.0f; w->flare_v[4].color = 0xFFEBC8;
        w->flare_v[4].x = -200.0f; w->flare_v[4].y = -200.0f; w->flare_v[4].z = 0.0f;
        w->flare_v[5] = w->flare_v[1];
    }
    if (w->h.wait == w->h.delay) {
        f = &w->flare[w->flare_cur];
        f->flags |= 1;
        f->t = 0;
        f->scale = RAND15() / 32767.0f * 3.0f + 1.0f;
        f->angle = RAND15() / 32767.0f * 6.2831855f;
        if (w->flare_cur < 7)
            w->flare_cur++;
        else
            w->flare_cur = 0;
    }
    if (w->h.wait == w->h.delay) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
        list = func_0882698C(list, 0, 1);
        list = func_088269D8(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
        list = func_08826A64(list, 3);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        for (i = 0; i < 8; i++) {
            f = &w->flare[i];
            if (f->flags & 1) {
                s = f->scale * (func_08827A2C(f->t, 8, 1) * 0.8f + 0.2f);
                a = out * in * func_08827AD4(f->t, 8, 1) * 255.0f;
                list = dl_ambient_light(list, (unsigned int)a << 24 | 0xFFFFFF);
                mtx_push();
                mtx_rotate_z(f->angle);
                func_088272BC(s, s, 0.0f);
                list = draw_array(list, 5, 0x19F, 6, 0, w->flare_v);
                mtx_pop();
                if (f->t < 7)
                    f->t++;
                else
                    f->flags &= ~1;
            }
        }
        mtx_pop();
        dl_set_list(list);
    }
    func_08826FEC(7);
    w->h.frame++;
    if (run == 1) {
        if (w->h.wait < w->h.delay)
            w->h.wait++;
        else
            w->h.t++;
    }
    if (w->h.t < w->h.len)
        return 2;
    return 4;
}
#endif
