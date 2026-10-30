// @range 0x0884774C-0x08847D0C
/* Card effect "nsea" (effects 24-26, 100, 101, 113, 114): a glowing frame on the character's
   bone 39 (nsea.gim with a random texture offset, inside a fading outline of quads). */
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
void mtx_load(FxMat4 *m);

void func_088277BC(void);
void func_08827668(void);
void func_08827944(float a, FxMat4 *out, FxMat4 *m);
float func_088279A4(int t, int len);
float func_08827A00(int t, int len, int end);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
void *func_0882698C(void *list, int on, int mask);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *func_08826A64(void *list, int tex);
void *func_088266D8(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);
void dd_chara_bone_mtx(void *out, int bone);

typedef struct RandSeed {              /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;
    unsigned int b, c;
} RandSeed;
extern int rand3_slot;                 /* BOOT variable imports, 0 in the relocated ELF */
extern RandSeed rand3_seeds[];

#define RAND15() (rand3_seeds[rand3_slot].lcg *= 0x41C64E6D, rand3_seeds[rand3_slot].lcg += 0x3039, \
                  (int)(rand3_seeds[rand3_slot].lcg >> 16 & 0x7FFF))

typedef struct DdFxNsea {              /* 0x160 */
    DdFxHead h;
    int pad18[2];
    FxMat4 mtx;                        /* +0x20 */
    int pad60;
    FxVtxT quad[4];                    /* +0x64 */
    FxVtxC frame[10];                  /* +0xB4 */
} DdFxNsea;

int dd_fx_nsea_update(int phase, DdFxNsea *w);

void dd_fx_nsea_times(int id, int *delay, int *len)
{
    switch (id) {
    case 24:
        *delay = 25;
        break;
    case 25:
    case 26:
        *delay = 30;
        break;
    case 100:
        *delay = 70;
        break;
    case 101:
        *delay = 60;
        break;
    case 113:
    case 114:
        *delay = 0;
        break;
    default:
        *delay = 0;
        *len = 0;
        return;
    }
    *len = 28;
}

int dd_fx_nsea_start(DdFxArg *arg, void *a, void *b)
{
    int task = func_0882623C(dd_fx_nsea_update, a, b, sizeof(DdFxNsea));

    if (task != -1)
        ((DdFxNsea *)func_08826440(task))->h.arg = arg;
    return task;
}

int dd_fx_nsea_update(int phase, DdFxNsea *w)
{
    DdFxArg *arg = w->h.arg;
    int run = arg->run;
    void *list;
    float in;
    float out;
    float W;
    float tu;
    float tv;
    float H;
    int i;

    W = 31.0f;
    H = 44.0f;
    func_088277BC();
    if (phase == 1)
        dd_fx_nsea_times(arg->id, &w->h.delay, &w->h.len);
    dd_chara_bone_mtx(&w->mtx, 39);
    func_08827944(1.5707964f, &w->mtx, &w->mtx);
    switch (arg->id) {
    case 24:
    case 25:
    case 26:
        in = 1.0f;
        break;
    default:
        in = func_088279A4(w->h.t, 8);
        break;
    }
    out = func_08827A00(w->h.t, 16, w->h.len);
    if (phase == 1) {
        w->quad[0].u = 0.0f; w->quad[0].v = 0.0f; w->quad[0].x = -W; w->quad[0].y = H; w->quad[0].z = 0.0f;
        w->quad[1].u = 0.0f; w->quad[1].v = 0.2f; w->quad[1].x = -W; w->quad[1].y = -H; w->quad[1].z = 0.0f;
        w->quad[2].u = 0.2f; w->quad[2].v = 0.0f; w->quad[2].x = W; w->quad[2].y = H; w->quad[2].z = 0.0f;
        w->quad[3].u = 0.2f; w->quad[3].v = 0.2f; w->quad[3].x = W; w->quad[3].y = -H; w->quad[3].z = 0.0f;
        w->frame[0].color = 0x8080C4FF; w->frame[0].x = -W; w->frame[0].y = H; w->frame[0].z = 0.0f;
        w->frame[2].color = 0x8080C4FF; w->frame[2].x = W; w->frame[2].y = H; w->frame[2].z = 0.0f;
        w->frame[4].color = 0x8080C4FF; w->frame[4].x = W; w->frame[4].y = -H; w->frame[4].z = 0.0f;
        w->frame[6].color = 0x8080C4FF; w->frame[6].x = -W; w->frame[6].y = -H; w->frame[6].z = 0.0f;
        w->frame[8] = w->frame[0];
        w->frame[1].color = 0x0080C4FF; w->frame[1].x = -(W * 0.7f); w->frame[1].y = H * 0.7f; w->frame[1].z = 0.0f;
        w->frame[3].color = 0x0080C4FF; w->frame[3].x = W * 0.7f; w->frame[3].y = H * 0.7f; w->frame[3].z = 0.0f;
        w->frame[5].color = 0x0080C4FF; w->frame[5].x = W * 0.7f; w->frame[5].y = -(H * 0.7f); w->frame[5].z = 0.0f;
        w->frame[7].color = 0x0080C4FF; w->frame[7].x = -(W * 0.7f); w->frame[7].y = -(H * 0.7f); w->frame[7].z = 0.0f;
        w->frame[9] = w->frame[1];
    }
    if (w->h.wait == w->h.delay) {
        list = dl_select_layer(7, 0);
        list = func_0882698C(list, 0, 0);
        list = func_088269D8(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
        list = func_08826A64(list, 9);
        list = dl_color(list, 0x8080C4FF);
        mtx_set_mode(2);
        mtx_push();
        mtx_load(&w->mtx);
        list = dl_ambient_light(list, (unsigned int)(255.0f * in * out) << 24 | 0xFFFFFF);
        list = func_088266D8(list, 0, 1, 1, 0, 0, 0, 0);
        list = draw_array(list, 4, 0x19C, 10, 0, w->frame);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        tv = RAND15() / 32767.0f;
        tu = RAND15() / 32767.0f;
        list = dl_tex_offset(tu, tv, list);
        for (i = 0; i < 3; i++)
            list = draw_array(list, 4, 0x183, 4, 0, w->quad);
        mtx_pop();
        dl_set_list(list);
    }
    func_08827668();
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
