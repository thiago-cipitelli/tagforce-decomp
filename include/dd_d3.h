/* Private header of the 0x08825DDC-0x08837098 files of libduel_draw: BOOT calls, the module
   functions they share and the helpers of the effect tasks (dd_fx_*.c). */
#ifndef DD_D3_H
#define DD_D3_H

#include "ehsys.h"
#include "menu.h"
#include "libc.h"
#include "psp.h"
#include "libduel_draw.h"

/* BOOT display-list calls (signatures of src/BOOT) */
unsigned int *dl_select_layer(int layer, int *size);
int dl_set_list(unsigned int *list);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *dl_light_mode(unsigned int *list, int mode);
unsigned int *dl_ambient_light(unsigned int *list, unsigned int color);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_material_colors(unsigned int *list, unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular);
unsigned int *dl_color_material(unsigned int *list, int comps);
unsigned int *dl_shade_model(unsigned int *list, int mode);
unsigned int *dl_front_face(unsigned int *list, int cw);
unsigned int *dl_tex_map_mode(unsigned int *list, int mode, int a, int b);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(unsigned int *list, float u, float v);
unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_depth_func(unsigned int *list, int func);
unsigned int *dl_depth_mask(unsigned int *list, int mask);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);

void *dd_fb_addr(int back);            /* VRAM address of the front (0) or back (1) frame buffer */
void *dd_vram_addr(int offset);
int dd_task_create(DdTaskUpdateFn update, DdTaskTermFn term, void *arg, int size);
int dd_task_kill(int id);
int dd_task_alive(int id);
void *dd_task_mem(int id);
int dd_task_flag_set(int id, unsigned int flags);
int dd_task_flag_test(int id, unsigned int flags);
int dd_task_pause(int id);

#ifdef DD_FX
typedef struct DdTexVert {             /* float texture + float position (0x183) */
    float u, v;
    float x, y, z;
} DdTexVert;

typedef struct DdRandSeed {
    unsigned int lcg;
    unsigned int b, c;
} DdRandSeed;

extern int rand3_slot;
extern DdRandSeed rand3_seeds[];

#define DD_RAND(r) ((r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, (int)((r)->lcg >> 16 & 0x7FFF))

float *vec3_copy(float *out, float *v);
float *vec3_set(float *out, float x, float y, float z);
float *vec3_add(float *out, float *a, float *b);
float *vec3_sub(float *out, float *a, float *b);
float *vec3_mul(float *out, float *a, float *b);
float *vec3_scale(float *out, float *v, float s);
float *vec3_normalize(float *out, float *v);
float *mat3_identity(float *out);
float *mat3_from_euler(float *out, float *m, float *angles);
float *mat3_transform_vec3(float *out, float *m, float *v);
float sinf(float x);
float cosf(float x);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(float *v);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
void mtx_rotate_z(float a);
void mtx_rotate_zyx(float *v);
void mtx_scale3(float *v);
void mtx_mult(float *m);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *verts);
unsigned int *dd_tex_pack(unsigned int *list, int i);
unsigned int *dd_tex_card(unsigned int *list, int id, int mode);
unsigned int *dd_tex_enable(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v);
unsigned int *dd_tex_enable_ofs(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v, float u, float v);
unsigned int *dd_tex_enable_uv(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v, float u, float v, float su, float sv);
unsigned int *dd_depth_enable(unsigned int *list, int on, int mask);
unsigned int *dd_blend_enable(unsigned int *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void dd_card_quad_draw(int id, float *pos, float *rot, float *scale, int alpha, float t);
void dd_card_quad_draw1(int id, float *pos, float *rot, int alpha, float t);
void dd_mtx_billboard_view(float *out);
void dd_scale3(float x, float y, float z);
void dd_translate3(float x, float y, float z);
float dd_ease_lin(int t, int n);
float dd_ease_lin_inv(int t, int n);
float dd_ease_lin_out(int t, int n, int end);
float dd_ease_sin(int t, int n, int k);
float dd_ease_sin_inv(int t, int n, int k);
float dd_ease_cos(int t, int n, int k);
float dd_ease_cos_out(int t, int n, int end, int k);
float dd_ease_cos_inv(int t, int n, int k);
int dd_step_clamp(int v, int d, int lim);
int dd_anim_step(void);
int dd_cue_se(int id, int now, int at, unsigned int *flags, unsigned int bit);
#endif

#endif
