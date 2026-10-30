// @range 0x088546C4-0x08854C44
// @asm 0x0885471C-0x08854C44
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

float func_088279A4(int t, int len);
float func_08827AD4(int t, int len, int n);
void func_08827840(void);
void func_08827668(void);
void func_08827288(FxMat4 *out);
void func_08827300(float x, float y, float z);
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
    int task = func_0882623C(dd_fx_sparkle_update, a, b, sizeof(DdFxSparkle));

    if (task != -1)
        ((DdFxSparkle *)func_08826440(task))->arg = arg;
    return task;
}

#ifdef NON_MATCHING /* laço de criação com saída para o desenho no meio; C equivalente */
int dd_fx_sparkle_update(int phase, DdFxSparkle *w)
{
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

    func_08827840();
    func_08827288(&cam);
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            w->cell[i][0].u = (float)(i % 4) * 0.25f;
            w->cell[i][0].v = (float)(i / 4) * 0.5f;
            w->cell[i][0].x = -0.15f;
            w->cell[i][0].y = 0.15f;
            w->cell[i][0].z = 0.0f;
            w->cell[i][1].u = (float)(i % 4) * 0.25f + 0.25f;
            w->cell[i][1].v = (float)(i / 4) * 0.5f + 0.5f;
            w->cell[i][1].x = 0.15f;
            w->cell[i][1].y = -0.15f;
            w->cell[i][1].z = 0.0f;
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
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_0882698C(list, 1, 1);
    list = func_088269D8(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = func_08826A64(list, 5);
    list = dl_color(list, 0xFF1830FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    func_08827300(0.02f, 1.13f, 0.0f);
    for (i = 0; i < 32; i++) {
        p = &w->pair[i];
        if (p->flags & 1) {
            list = dl_ambient_light(list, (unsigned int)(func_088279A4(p->t, 4) * 255.0f *
                                                         func_08827AD4(p->t - 16, 16, 1)) << 24 | 0xFFFFFF);
            for (j = 0; j < 2; j++) {
                s = &p->s[j];
                mtx_push();
                s->pos.y = s->speed * (float)p->t;
                mtx_translate(&s->pos);
                mtx_mult(&cam);
                list = draw_array(list, 6, 0x183, 2, 0, w->cell[s->cell]);
                mtx_pop();
            }
            if (p->t < 31)
                p->t++;
            else
                p->flags &= ~1;
        }
    }
    mtx_pop();
    dl_set_list(list);
    func_08827668();
    w->frame++;
    return 2;
}
#endif
