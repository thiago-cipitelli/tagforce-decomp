// @range 0x0884FD60-0x0885036C
// @asm 0x0884FDB8-0x0885036C
/* Character effect "music notes" (m169_onp01.gim): one note at a time pops near bone 4 of the
   duel character's model, bobbing and tilted at random, faded in and out. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_color(void *list, unsigned int color);
void *dl_ambient_light(void *list, unsigned int color);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(FxVec3 *v);
void mtx_rotate_z(float a);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
FxVec3 *vec3_add(FxVec3 *out, FxVec3 *a, FxVec3 *b);
float sinf(float x);
void model_get_bone(void *model, FxMat4 *out, int bone);

float dd_ease_lin(int t, int len);
float dd_ease_lin_inv(int t, int len);
void dd_mtx_push_field(void);
void dd_mtx_pop_all(void);
void *dd_field_model(void);
void dd_rand_dir(float len, FxVec3 *out);
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

typedef struct DdNote {                /* +0x288 */
    unsigned int flags;                /* bit 0 shown */
    int t;
    int life;
    int fade_in;
    int fade_out;
    int cell;                          /* +0x14 note of the texture */
    float angle;
    float phase;
    FxVec3 offset;                     /* +0x20 */
} DdNote;

typedef struct DdFxNote {              /* 0x2B4 */
    int arg;                           /* +0x000 */
    int frame;                         /* +0x004 */
    FxVtxT cell[8][4];                 /* +0x008 */
    DdNote note;                       /* +0x288 */
} DdFxNote;

int dd_fx_note_update(int phase, DdFxNote *w);

int dd_fx_note_start(int arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_note_update, a, b, sizeof(DdFxNote));

    if (task != -1)
        ((DdFxNote *)dd_task_mem(task))->arg = arg;
    return task;
}

#ifdef NON_MATCHING /* registradores e ordem das constantes da inicialização dos vértices diferentes; C equivalente */
int dd_fx_note_update(int phase, DdFxNote *w)
{
    DdNote *n;
    void *list;
    float u;
    float f;
    int sign;
    FxMat4 m;
    FxVec3 p;
    FxVec3 pos;
    int i;

    dd_mtx_push_field();
    model_get_bone(dd_field_model(), &m, 4);
    vec3_set(&pos, m.m[12], m.m[13], m.m[14]);
    if (phase == 1) {
        for (i = 0; i < 8; i++) {
            u = (float)i * 0.125f;
            w->cell[i][0].u = u; w->cell[i][0].v = 0.1875f;
            w->cell[i][0].x = -0.15f; w->cell[i][0].y = 0.15f; w->cell[i][0].z = 0.0f;
            w->cell[i][1].u = u; w->cell[i][1].v = 0.3125f;
            w->cell[i][1].x = -0.15f; w->cell[i][1].y = -0.15f; w->cell[i][1].z = 0.0f;
            w->cell[i][2].u = u + 0.125f; w->cell[i][2].v = 0.1875f;
            w->cell[i][2].x = 0.15f; w->cell[i][2].y = 0.15f; w->cell[i][2].z = 0.0f;
            w->cell[i][3].u = u + 0.125f; w->cell[i][3].v = 0.3125f;
            w->cell[i][3].x = 0.15f; w->cell[i][3].y = -0.15f; w->cell[i][3].z = 0.0f;
        }
    }
    n = &w->note;
    if (!(n->flags & 1)) {
        n->flags |= 1;
        f = RAND15() / 32767.0f;
        n->t = 0;
        n->life = f * 15.0f + 30.0f;
        n->fade_in = f * 4.0f + 8.0f;
        n->fade_out = f * 8.0f + 16.0f;
        n->cell = RAND15() % 8;
        sign = RAND15();
        n->angle = RAND15() / 32767.0f * 0.69813174f * (float)((sign & 1) * 2 - 1);
        n->phase = RAND15() / 32767.0f * 6.2831855f;
        dd_rand_dir(1.0f, &n->offset);
    }
    list = dl_select_layer(1, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 31);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 7);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    if (n->flags & 1) {
        f = dd_ease_lin_inv(n->t - (n->life - n->fade_out), n->fade_out);
        list = dl_ambient_light(list, (unsigned int)(dd_ease_lin(n->t, n->fade_in) * 255.0f * f) << 24 | 0xFFFFFF);
        vec3_add(&p, &n->offset, &pos);
        p.y += sinf(n->phase + (float)n->t * 0.17453294f) * 0.15f;
        mtx_push();
        mtx_translate(&p);
        mtx_rotate_z(n->angle);
        list = draw_array(list, 4, 0x183, 4, 0, w->cell[n->cell]);
        mtx_pop();
        if (n->t < n->life - 1)
            n->t++;
        else
            n->flags &= ~1;
    }
    mtx_pop();
    list = dl_disable(list, 7);
    dl_set_list(list);
    dd_mtx_pop_all();
    w->frame++;
    return 2;
}
#endif
