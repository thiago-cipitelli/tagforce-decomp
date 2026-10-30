// @range 0x0884A86C-0x0884AE24
// @asm 0x0884A8C4-0x0884AE24
/* Card effect "eye flash": the screen flashes white and two magenta lens flares (lens_aP8A.gim)
   light up on the character's eyes (bones 31 and 32) for 60 frames. */
#include "libduel_draw.h"

void *memcpy(void *, const void *, unsigned int);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_ambient_light(void *list, unsigned int color);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(FxVec3 *v);
void mtx_mult(FxMat4 *m);

float func_088279D8(int t, int len);
float func_08827A00(int t, int len, int end);
float func_08827A2C(int t, int len, int n);
float func_08827B94(int t, int len, int end, int n);
void func_088277BC(void);
void func_08827668(void);
void func_08827258(FxMat4 *out);
void func_08827ED4(int bone, FxVec3 *out);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
void *func_0882698C(void *list, int on, int mask);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *func_08826A64(void *list, int tex);
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);

typedef struct FxVtx2D {               /* GU_TRANSFORM_2D | GU_COLOR_8888 | GU_VERTEX_16BIT */
    unsigned int color;
    short x, y, z;
    short pad;
} FxVtx2D;

typedef struct DdFxEyes {              /* 0xFC */
    int t;                             /* +0x00 */
    int frame;                         /* +0x04 */
    DdFxArg *arg;                      /* +0x08 */
    FxVtx2D screen[2][4];              /* +0x0C one per frame parity */
    FxVtx flare[6];                    /* +0x6C */
} DdFxEyes;

typedef struct DdBones2 {
    int bone[2];
} DdBones2;

int dd_fx_eyes_update(int phase, DdFxEyes *w);

int dd_fx_eyes_start(DdFxArg *arg, void *a, void *b)
{
    int task = func_0882623C(dd_fx_eyes_update, a, b, sizeof(DdFxEyes));

    if (task != -1)
        ((DdFxEyes *)func_08826440(task))->arg = arg;
    return task;
}

#ifdef NON_MATCHING /* a cópia do modelo {31, 32} sai intercalada com dois lui no original; o resto bate */
int dd_fx_eyes_update(int phase, DdFxEyes *w)
{
    int run = w->arg->run;
    int buf = w->frame % 2;
    void *list;
    float a;
    unsigned int ia;
    unsigned int ib;
    unsigned int c;
    FxMat4 cam;
    FxVec3 eye[2];
    DdBones2 bones = { 31, 32 };
    int i;
    int side;
    int j;
    float S;

    func_088277BC();
    for (i = 0, side = -1; i < 2; i++, side += 2) {
        func_08827ED4(bones.bone[i], &eye[i]);
        eye[i].x += (float)side * -1.0f;
        eye[i].y += -0.2f;
    }
    func_08827258(&cam);
    if (phase == 1) {
        w->screen[0][0].color = 0; w->screen[0][0].x = 0; w->screen[0][0].y = 0; w->screen[0][0].z = 0;
        w->screen[0][1].color = 0; w->screen[0][1].x = 0; w->screen[0][1].y = 272; w->screen[0][1].z = 0;
        w->screen[0][2].color = 0; w->screen[0][2].x = 480; w->screen[0][2].y = 0; w->screen[0][2].z = 0;
        w->screen[0][3].color = 0; w->screen[0][3].x = 480; w->screen[0][3].y = 272; w->screen[0][3].z = 0;
        memcpy(w->screen[1], w->screen[0], sizeof(w->screen[0]));
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 0, 1, 1, 0, 0, 1, 1);
    list = func_0882698C(list, 0, 1);
    list = func_088269D8(list, 1, 2, 2, 10, 0, 0xFFFFFFFF);
    ia = func_08827A00(w->t, 40, 60) * 255.0f;
    ib = func_088279D8(w->t, 40) * 255.0f;
    c = ia << 24 | 0xFFFFFF;
    w->screen[buf][0].color = c;
    w->screen[buf][1].color = c;
    w->screen[buf][2].color = c;
    w->screen[buf][3].color = ib << 24 | 0xFFFFFF;
    list = draw_array(list, 4, 0x80011C, 4, 0, w->screen[buf]);
    dl_set_list(list);
    S = 10.0f;
    if (phase == 1) {
        w->flare[0].u = 0.5f; w->flare[0].v = 0.5f; w->flare[0].color = 0xC8FF00FF;
        w->flare[0].x = 0.0f; w->flare[0].y = 0.0f; w->flare[0].z = 0.0f;
        w->flare[1].u = 0.0f; w->flare[1].v = 0.0f; w->flare[1].color = 0xFF00FF;
        w->flare[1].x = -S; w->flare[1].y = S; w->flare[1].z = 0.0f;
        w->flare[2].u = 1.0f; w->flare[2].v = 0.0f; w->flare[2].color = 0xFF00FF;
        w->flare[2].x = S; w->flare[2].y = S; w->flare[2].z = 0.0f;
        w->flare[3].u = 1.0f; w->flare[3].v = 1.0f; w->flare[3].color = 0xFF00FF;
        w->flare[3].x = S; w->flare[3].y = -S; w->flare[3].z = 0.0f;
        w->flare[4].u = 0.0f; w->flare[4].v = 1.0f; w->flare[4].color = 0xFF00FF;
        w->flare[4].x = -S; w->flare[4].y = -S; w->flare[4].z = 0.0f;
        w->flare[5] = w->flare[1];
    }
    list = dl_select_layer(7, 0);
    list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
    list = func_0882698C(list, 0, 1);
    list = func_088269D8(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = func_08826A64(list, 3);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    a = func_08827A2C(w->t, 20, 1) * 255.0f;
    list = dl_ambient_light(list, (unsigned int)(a * func_08827B94(w->t, 40, 60, 1)) << 24 | 0xFFFFFF);
    for (j = 0; j < 2; j++) {
        mtx_push();
        mtx_translate(&eye[j]);
        mtx_mult(&cam);
        list = draw_array(list, 5, 0x19F, 6, 0, w->flare);
        mtx_pop();
    }
    mtx_pop();
    dl_set_list(list);
    func_08827668();
    w->frame++;
    if (run == 1)
        w->t++;
    if (w->t < 60)
        return 2;
    return 4;
}
#endif
