// @range 0x088546C4-0x08854C44
/* Character effect "sparkles": pairs of small sparks (PT88_R_a.gim) rising from around the
   character's feet, 32 alive at once. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_color(void *list, unsigned int color);
void *dl_ambient_light(void *list, unsigned int color);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(FxVec3 *v);
void mtx_mult(FxMat4 *m);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
float sinf(float x);
float cosf(float x);

float dd_ease_lin(int t, int len);
float dd_ease_cos(int t, int len, int n);
void dd_mtx_push_field(void);
void dd_mtx_pop_all(void);
void dd_mtx_billboard_field(FxMat4 *out);
void dd_translate3(float x, float y, float z);
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
#define RANDF() (RAND15() / 32767.0f)

typedef struct DdSpark {               /* 0x14 */
    int cell;
    float speed;                       /* rise per frame */
    FxVec3 pos;
} DdSpark;

typedef struct DdSparkPair {           /* 0x30 */
    unsigned int flags;
    int t;
    DdSpark s[2];
} DdSparkPair;

typedef struct DdFxSparkle {           /* 0x74C */
    int arg;                           /* +0x000 */
    int frame;                         /* +0x004 */
    int cur;                           /* +0x008 */
    FxVtxT cell[8][2];                 /* +0x00C */
    DdSparkPair pair[32];              /* +0x14C */
} DdFxSparkle;

int dd_fx_sparkle_update(int phase, DdFxSparkle *w);

int dd_fx_sparkle_start(int arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_sparkle_update, a, b, sizeof(DdFxSparkle));

    if (task != -1)
        ((DdFxSparkle *)dd_task_mem(task))->arg = arg;
    return task;
}

/* counters and the pair pointer of the draw loop are own variables (ci, k, pp): they decide the $s registers */
int dd_fx_sparkle_update(int phase, DdFxSparkle *w)
{
    int ci;
    int k;
    FxMat4 cam;
    void *list;
    DdSparkPair *p;
    DdSpark *s;
    float ang;
    float r;
    float f;
    int n;
    int i;
    int j;
    float h;
    float q;
    float hv;

    dd_mtx_push_field();
    dd_mtx_billboard_field(&cam);
    if (phase == 1) {
        h = 0.15f;
        q = 0.25f;
        hv = 0.5f;
        for (ci = 0; ci < 8; ci++) {
            float u = (float)(ci % 4) * q;
            float v = (float)(ci / 4) * hv;

            w->cell[ci][0].u = u;
            w->cell[ci][0].v = v;
            w->cell[ci][0].x = -h;
            w->cell[ci][0].y = h;
            w->cell[ci][0].z = 0.0f;
            w->cell[ci][1].u = u + q;
            w->cell[ci][1].v = v + hv;
            w->cell[ci][1].x = h;
            w->cell[ci][1].y = -h;
            w->cell[ci][1].z = 0.0f;
        }
    }
    n = phase == 1 ? 32 : 1;
    for (i = 0; i < n; i++) {
        p = &w->pair[w->cur];
        p->flags |= 1;
        p->t = n - 1 - i;
        for (j = 0; j < 2; j++) {
            s = &p->s[j];
            ang = RANDF() * 6.2831855f;
            f = RANDF();
            r = f * 0.2f + 0.05f;
            s->cell = RAND15() % 8;
            s->speed = cosf(f * 1.5707964f) * 0.018f + 0.008f;
            vec3_set(&s->pos, r * sinf(ang), 0.0f, r * cosf(ang));
        }
        if (w->cur < 31)
            w->cur++;
        else
            w->cur = 0;
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 5);
    list = dl_color(list, 0xFF1830FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_translate3(0.02f, 1.13f, 0.0f);
    for (i = 0; i < 32; i++) {
        DdSparkPair *pp = &w->pair[i];
        if (!(pp->flags & 1) != 1) {
            list = dl_ambient_light(list, (unsigned int)(dd_ease_lin(pp->t, 4) * 255.0f *
                                                         dd_ease_cos(pp->t - 16, 16, 1)) << 24 | 0xFFFFFF);
            for (k = 0; k < 2; k++) {
                s = &pp->s[k];
                mtx_push();
                s->pos.y = s->speed * (float)pp->t;
                mtx_translate(&s->pos);
                mtx_mult(&cam);
                list = draw_array(list, 6, 0x183, 2, 0, w->cell[s->cell]);
                mtx_pop();
            }
            if (pp->t < 31)
                pp->t++;
            else
                pp->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    dd_mtx_pop_all();
    w->frame++;
    return 2;
}
