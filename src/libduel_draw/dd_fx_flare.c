// @range 0x08848B40-0x08849060
/* Card effect "flare" (effects 247-281): a yellow and a blue lens flare (lens_aP8A.gim) on the
   character's bone 38, with its start and hit sounds; kinds 15 and 16 only play the sounds. */
#include "libduel_draw.h"

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

float func_088279A4(int t, int len);
float func_08827A2C(int t, int len, int n);
void func_08827738(void);
void func_08827668(void);
void func_08827258(FxMat4 *out);
void func_08827F14(int bone, FxVec3 *out);
int func_08827F68(int se, int t, int at, unsigned int *flags, unsigned int bit);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
void *func_0882698C(void *list, int on, int mask);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *func_08826A64(void *list, int tex);
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);
void dd_chara_set_08813FB4(int arg);
void dd_fx_hit_times(int id, int *delay, int *hit, int *len);

typedef struct DdFxFlare {             /* 0x158 */
    int wait;                          /* +0x00 */
    int delay;                         /* +0x04 */
    int hit;                           /* +0x08 frame of the hit */
    int t;                             /* +0x0C */
    int len;                           /* +0x10 */
    int frame;                         /* +0x14 */
    int hit_done;                      /* +0x18 */
    int visible;                       /* +0x1C */
    DdFxArg *arg;                      /* +0x20 */
    FxVec3 pos;                        /* +0x24 */
    int se;                            /* +0x30 */
    unsigned int se_done;              /* +0x34 */
    FxVtx yellow[6];                   /* +0x38 */
    FxVtx blue[6];                     /* +0xC8 */
} DdFxFlare;

int dd_fx_flare_update(int phase, DdFxFlare *w);

int dd_fx_flare_start(DdFxArg *arg, void *a, void *b)
{
    int task = func_0882623C(dd_fx_flare_update, a, b, sizeof(DdFxFlare));

    if (task != -1)
        ((DdFxFlare *)func_08826440(task))->arg = arg;
    return task;
}

int dd_fx_flare_update(int phase, DdFxFlare *w)
{
    DdFxArg *arg = w->arg;
    int run = arg->run;
    void *list;
    float in;
    float out;
    float S;
    FxMat4 cam;

    S = 40.0f;
    func_08827738();
    if (phase == 1) {
        dd_fx_hit_times(arg->id, &w->delay, &w->hit, &w->len);
        w->visible = 1;
        w->se = 0x6239;
        switch (((int *)arg)[2]) {
        case 13:
            w->se = 0x6238;
            break;
        case 15:
        case 16:
            w->visible = 0;
            break;
        }
    }
    in = func_08827A2C(w->t, 6, 1);
    out = 1.0f - func_088279A4(w->t - (w->len - 16), 16);
    func_08827258(&cam);
    func_08827F14(38, &w->pos);
    func_08827F68(w->se, w->wait, w->delay, &w->se_done, 1);
    if (w->wait == w->hit && w->hit_done == 0) {
        w->hit_done = 1;
        dd_chara_set_08813FB4(1);
    }
    func_08827F68(0x623B, w->t, 2, &w->se_done, 2);
    if (w->visible != 0) {
        if (phase == 1) {
            w->yellow[0].u = 0.5f; w->yellow[0].v = 0.5f; w->yellow[0].color = 0x80FFFF00;
            w->yellow[0].x = 0.0f; w->yellow[0].y = 0.0f; w->yellow[0].z = 0.0f;
            w->yellow[1].u = 0.0f; w->yellow[1].v = 0.0f; w->yellow[1].color = 0xFFFF00;
            w->yellow[1].x = -S; w->yellow[1].y = S; w->yellow[1].z = 0.0f;
            w->yellow[2].u = 1.0f; w->yellow[2].v = 0.0f; w->yellow[2].color = 0xFFFF00;
            w->yellow[2].x = S; w->yellow[2].y = S; w->yellow[2].z = 0.0f;
            w->yellow[3].u = 1.0f; w->yellow[3].v = 1.0f; w->yellow[3].color = 0xFFFF00;
            w->yellow[3].x = S; w->yellow[3].y = -S; w->yellow[3].z = 0.0f;
            w->yellow[4].u = 0.0f; w->yellow[4].v = 1.0f; w->yellow[4].color = 0xFFFF00;
            w->yellow[4].x = -S; w->yellow[4].y = -S; w->yellow[4].z = 0.0f;
            w->yellow[5] = w->yellow[1];
            w->blue[0].u = 0.5f; w->blue[0].v = 0.5f; w->blue[0].color = 0xFF0000FF;
            w->blue[0].x = 0.0f; w->blue[0].y = 0.0f; w->blue[0].z = 0.0f;
            w->blue[1].u = 0.0f; w->blue[1].v = 0.0f; w->blue[1].color = 0xFF;
            w->blue[1].x = -S; w->blue[1].y = S; w->blue[1].z = 0.0f;
            w->blue[2].u = 1.0f; w->blue[2].v = 0.0f; w->blue[2].color = 0xFF;
            w->blue[2].x = S; w->blue[2].y = S; w->blue[2].z = 0.0f;
            w->blue[3].u = 1.0f; w->blue[3].v = 1.0f; w->blue[3].color = 0xFF;
            w->blue[3].x = S; w->blue[3].y = -S; w->blue[3].z = 0.0f;
            w->blue[4].u = 0.0f; w->blue[4].v = 1.0f; w->blue[4].color = 0xFF;
            w->blue[4].x = -S; w->blue[4].y = -S; w->blue[4].z = 0.0f;
            w->blue[5] = w->blue[1];
        }
        if (w->wait == w->hit) {
            list = dl_select_layer(7, 0);
            list = func_088266D8(list, 1, 1, 1, 0, 1, 1, 1);
            list = func_0882698C(list, 0, 1);
            list = func_08826A64(list, 3);
            mtx_set_mode(2);
            mtx_push();
            mtx_identity();
            mtx_translate(&w->pos);
            mtx_mult(&cam);
            list = dl_ambient_light(list, (unsigned int)(in * 255.0f * out) << 24 | 0xFFFFFF);
            list = func_088269D8(list, 1, 2, 2, 10, 0, 0xFFFFFFFF);
            list = draw_array(list, 5, 0x19F, 6, 0, w->yellow);
            list = func_088269D8(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
            list = draw_array(list, 5, 0x19F, 6, 0, w->blue);
            mtx_pop();
            dl_set_list(list);
        }
    }
    func_08827668();
    w->frame++;
    if (run == 1) {
        if (w->wait < w->hit)
            w->wait++;
        else
            w->t++;
    }
    if (w->t < w->len)
        return 2;
    return 4;
}
