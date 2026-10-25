// @range 0x08814F58-0x08817DD4
// @asm 0x0881561C-0x08815A64
// @asm 0x08815B9C-0x08817DD4
// @rodata 0x0881C29C-0x0881C2B8
/* Shine of the rare cards when they turn to the camera in the opened pack: a light bar sweeping
   over the card (rare), and the larger effects of the super/ultra rares; GE state and matrix
   helpers of the effect. */
#include "libshop.h"

typedef struct ShopGlowVtx {    /* GE vertex: 8888 color, float xyz (0x99C with 8-bit indices) */
    unsigned int color;
    float x, y, z;
} ShopGlowVtx;

typedef struct ShopGlowBar {    /* shop_glow_bar 0x08F98C50 */
    ShopGlowVtx v[2][3];        /* +0x00 two columns: top, middle (alpha), bottom */
    unsigned char idx[2][2];    /* +0x60 strip indices 0, 1, 3, 4 */
    Vec4 corner[2];             /* +0x70 card corners, the scissor box */
} ShopGlowBar;

typedef struct ShopGlowRare {   /* shop_glow_rare_st 0x08F98C40 */
    int t;                      /* sweep time */
    int done;                   /* the sweep ended */
    int wait;                   /* time after the sweep */
} ShopGlowRare;

extern ShopGlowBar shop_glow_bar_st;
extern ShopGlowRare shop_glow_rare_st;

unsigned int *dl_ambient_light(unsigned int *list, unsigned int color);
unsigned int *dl_depth_mask(unsigned int *list, int mask);
unsigned int *dl_light_mode(unsigned int *list, int mode);
unsigned int *dl_material_colors(unsigned int *list, unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular);
unsigned int *dl_color_material(unsigned int *list, int comps);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(unsigned int *list, float u, float v); /* args in a0, f12, f13 either way */
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
Mat4 *mat4_transpose(Mat4 *out, Mat4 *m);
Mat4 *mat4_copy(Mat4 *out, Mat4 *m);
void mat4_mul(Mat4 *r, Mat4 *a, Mat4 *b);
void mtx_get34(Mat4 *m);
Vec4 *vec4_set(Vec4 *out, float x, float y, float z, float w);
Mat4 *mat4_identity(Mat4 *out);
void mat4_transform_vec4(Vec4 *out, Mat4 *m, Vec4 *v);
unsigned int *dl_tex_map_mode(unsigned int *list, int mode, int a, int b);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *vertices);
void mtx_scale3(float *v);
void mtx_translate(float *v);
void mtx_push(void);
void mtx_pop(void);
void mtx_load(Mat4 *m);
void mtx_rotate_z(float a);
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a);
float cosf(float x);
float roundf_vfpu(float x);
float sqrtf_vfpu(float x);
float atan2f_vfpu(float y, float x);

void shop_glow_view_rot(Mat4 *out);
void shop_glow_rot_only(Mat4 *out, Mat4 *m);
void shop_glow_mvp(Mat4 *out);
float shop_glow_ratio(int t, int max);
float shop_glow_ratio_inv(int t, int max);
void shop_glow_draw_bar(int first, Mat4 *m, int t, int max, int r, int g, int b, int a);
int shop_glow_rare(int t, Mat4 *m, unsigned short speed, void *vram);
int shop_glow_super(int t, Mat4 *m, unsigned short speed, void *vram);
int shop_glow_ultra(int t, Mat4 *m, unsigned short speed, void *vram);

void shop_glow_ge_state(int layer)
{
    unsigned int *list = dl_select_layer(layer, 0);

    list = dl_enable(list, 0);
    list = dl_disable(list, 2);
    list = dl_disable(list, 3);
    list = dl_disable(list, 5);
    list = dl_disable(list, 6);
    list = dl_disable(list, 7);
    list = dl_enable(list, 8);
    list = dl_enable(list, 10);
    list = dl_disable(list, 11);
    list = dl_disable(list, 12);
    list = dl_disable(list, 13);
    list = dl_disable(list, 14);
    list = dl_disable(list, 15);
    list = dl_enable(list, 16);
    list = dl_disable(list, 17);
    list = dl_disable(list, 18);
    list = dl_disable(list, 19);
    list = dl_disable(list, 20);
    list = dl_disable(list, 21);
    list = dl_color(list, 0);
    list = dl_light_mode(list, 0);
    list = dl_ambient_light(list, 0);
    list = dl_material_colors(list, 0, 0, 0, 0);
    list = dl_color_material(list, 7);
    list = dl_shade_model(list, 1);
    list = dl_front_face(list, 1);
    list = dl_tex_map_mode(list, 0, 0, 0);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(list, 1.0f, 1.0f);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_depth_func(list, 7);
    dl_set_list(list);
}

unsigned int *shop_glow_tex(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v)
{
    if (on == 1) {
        list = dl_enable(list, 9);
        list = dl_set_tex_filter(list, min, mag);
        list = dl_tex_func(list, func, comp);
        list = dl_tex_wrap(list, wrap_u, wrap_v);
        return dl_tex_offset(list, 0.0f, 0.0f);
    }
    return dl_disable(list, 9);
}

unsigned int *shop_glow_depth(unsigned int *list, int on, int mask)
{
    if (on == 1) {
        list = dl_enable(list, 1);
        return dl_depth_mask(list, mask);
    }
    return dl_disable(list, 1);
}

unsigned int *shop_glow_blend(unsigned int *list, int on, int op, int src, int dst, unsigned int sfix, unsigned int dfix)
{
    if (on == 1) {
        list = dl_enable(list, 4);
        return dl_blend_func(list, op, src, dst, sfix, dfix);
    }
    return dl_disable(list, 4);
}

void *shop_glow_set_res(unsigned int *list, int id)
{
    return gim_set_texture(list, shop_res_get(id));
}

/* rotation part of the view matrix, inverted */
void shop_glow_view_rot(Mat4 *out)
{
    Mat4 m, view;

    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(2);
    mat4_transpose(&m, &view);
    shop_glow_rot_only(&m, &m);
    mat4_copy(out, &m);
}

void shop_glow_scale(float x, float y, float z)
{
    Vec3 v = { 0 };

    v.x = x;
    v.y = y;
    v.z = z;
    mtx_scale3(&v.x);
}

void shop_glow_translate(float x, float y, float z)
{
    Vec3 v = { 0 };

    v.x = x;
    v.y = y;
    v.z = z;
    mtx_translate(&v.x);
}

/* projection * view * model */
void shop_glow_mvp(Mat4 *out)
{
    Mat4 model, view, proj, r;

    mtx_set_mode(2);
    mtx_get34(&model);
    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(0);
    mtx_get34(&proj);
    mat4_mul(&r, &view, &model);
    mat4_mul(&r, &proj, &r);
    mtx_set_mode(2);
    mat4_copy(out, &r);
}

/* drops the translation */
void shop_glow_rot_only(Mat4 *out, Mat4 *m)
{
    Mat4 t;

    mat4_copy(&t, m);
    vec4_set((Vec4 *)&t.m[12], 0.0f, 0.0f, 0.0f, 1.0f);
    t.m[3] = 0.0f;
    t.m[7] = 0.0f;
    t.m[11] = 0.0f;
    mat4_copy(out, &t);
}

float shop_glow_ratio(int t, int max)
{
    if (t < 0)
        t = 0;
    if (t > max)
        t = max;
    return (float)t / (float)max;
}

float shop_glow_ratio_inv(int t, int max)
{
    return 1.0f - shop_glow_ratio(t, max);
}

float shop_glow_ease(int t, int max, int n)
{
    float f;
    int i;

    if (t < 0)
        t = 0;
    if (t > max)
        t = max;
    f = 1.0f - (float)t / (float)max;
    for (i = 0; i < n; i++)
        f = cosf((1.0f - f) * 1.5707964f);
    return f;
}

/* 0 common, 1-2 rare, 3 super rare, 4-6 ultra/ultimate/secret; 1 once the effect is over */
int shop_card_glow(int t, Mat4 *m, unsigned short rarity, unsigned short speed, void *vram)
{
    int r;

    switch (rarity) {
    case 0:
        r = 1;
        break;
    case 1:
    case 2:
        return shop_glow_rare(t, m, speed, vram);
    case 3:
        r = shop_glow_super(t, m, speed, vram);
        break;
    case 4:
    case 5:
    case 6:
        r = shop_glow_ultra(t, m, speed, vram);
        break;
    default:
        r = 1;
        break;
    }
    return r;
}

#ifdef NON_MATCHING /* no original shop_glow_ratio é static (len fica em $f2 durante a chamada); aqui ela é global porque os efeitos em asm a chamam; também troca s1/s3 nos laços */
/* the light bar sweeping over the card */
void shop_glow_draw_bar(int first, Mat4 *m, int t, int max, int r, int g, int b, int a)
{
    ShopGlowBar *bar = &shop_glow_bar_st;
    float w = 5.378f;           /* card size */
    float h = 7.634f;
    float one = 1.0f;
    unsigned int *list;
    Vec4 scr[2];
    Mat4 mvp;
    float ang, len;
    int i;

    if (t > max)
        t = max;
    if (first == 1) {
        unsigned int c, ca;
        float x;

        memset(bar, 0, sizeof(ShopGlowBar));
        ca = r | (g << 8 | (a << 24 | b << 16));
        c = r | (b << 16 | g << 8);
        for (i = 0; i < 2; i++) {
            x = -h + h * 2.0f * i;
            bar->v[i][0].color = c;
            bar->v[i][0].x = x;
            bar->v[i][0].y = one;
            bar->v[i][0].z = 0.0f;
            bar->v[i][1].color = ca;
            bar->v[i][1].x = x;
            bar->v[i][1].y = 0.5f * -one;
            bar->v[i][1].z = 0.0f;
            bar->v[i][2].color = c;
            bar->v[i][2].x = x;
            bar->v[i][2].y = -one;
            bar->v[i][2].z = 0.0f;
            bar->idx[i][0] = i * 3;
            bar->idx[i][1] = i * 3 + 1;
        }
        vec4_set(&bar->corner[0], -(w / 2.0f), h / 2.0f, 0.0f, one);
        vec4_set(&bar->corner[1], w / 2.0f, -(h / 2.0f), 0.0f, one);
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 0, 1, 1, 0, 1, 1, 1);
    list = shop_glow_depth(list, 0, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(m);
    shop_glow_mvp(&mvp);
    for (i = 0; i < 2; i++) {
        mat4_transform_vec4(&scr[i], &mvp, &bar->corner[i]);
        scr[i].x = roundf_vfpu(2048.0f + 240.0f * scr[i].x / scr[i].w - 1808.0f);
        scr[i].y = roundf_vfpu(2048.0f + -136.0f * scr[i].y / scr[i].w - 1912.0f);
    }
    list = dl_scissor(list, scr[0].x, scr[0].y, scr[1].x - scr[0].x, scr[1].y - scr[0].y);
    list = dl_enable(list, 2);
    ang = 1.5707964f - atan2f_vfpu(h, w);
    len = (sqrtf_vfpu(h * h + w * w) / 2.0f + 1.0f) * (-1.0f + 2.0f * shop_glow_ratio(t, max));
    mtx_rotate_z(ang);
    shop_glow_translate(0.0f, len, 0.0f);
    list = dl_ambient_light(list, -1);
    for (i = 0; i < 2; i++)
        list = draw_array(list, 4, 0x99C, 4, bar->idx, &bar->v[0][i]);
    list = dl_scissor(list, 0, 0, 480, 272);
    list = dl_disable(list, 2);
    mtx_pop();
    dl_set_list(list);
}
#endif

/* rare: one sweep, then a short wait */
int shop_glow_rare(int t, Mat4 *m, unsigned short speed, void *vram)
{
    ShopGlowRare *g = &shop_glow_rare_st;
    int first = t == 0;
    Mat4 view, rot;

    if (first == 1)
        memset(g, 0, sizeof(ShopGlowRare));
    if (g->done == 1) {
        g->wait += speed;
        return g->wait >= 24;
    }
    shop_glow_ge_state(3);
    shop_glow_view_rot(&view);
    shop_glow_ratio(g->t, 8);
    shop_glow_ratio_inv(g->t - 7, 1);
    mat4_identity(&rot);
    mat4_mul_rot_y(&rot, &rot, 3.14159274f);
    mat4_mul(&rot, m, &rot);
    shop_glow_draw_bar(first, &rot, g->t, 8, 0xFF, 0xE1, 0xCD, 200);
    g->t += speed;
    if (g->t >= 8)
        g->done = 1;
    return 0;
}
