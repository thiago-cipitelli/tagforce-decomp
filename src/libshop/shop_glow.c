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

typedef struct ShopGlowUv {     /* GE vertex 0x183: float uv, float xyz */
    float u, v;
    float x, y, z;
} ShopGlowUv;

typedef struct ShopGlowFanVtx { /* GE vertex 0x19F: float uv, 8888 color, float xyz */
    float u, v;
    unsigned int color;
    float x, y, z;
} ShopGlowFanVtx;

typedef struct ShopGlowOrb {    /* light orb rising around the card */
    unsigned int flags;         /* 1 alive */
    int t;
    unsigned int alpha;
    float ang[2];               /* +0x0C angle of each of the two halves */
    float rad[2];               /* +0x14 radius wobble */
    Vec4 pos[2];                /* +0x20 */
} ShopGlowOrb;

typedef struct ShopGlowSpark {  /* spark left behind by an orb */
    unsigned int flags;         /* 1 alive */
    int t;
    unsigned int alpha;
    int kind;                   /* +0x0C sprite of the sheet */
    Vec4 pos;                   /* +0x10 */
    Vec4 vel;                   /* +0x20 */
} ShopGlowSpark;

typedef struct ShopGlowSparks {
    int next;                   /* ring cursor */
    ShopGlowSpark s[6];         /* +0x10 */
} ShopGlowSparks;

typedef struct ShopGlowSuper {  /* shop_glow_super_st 0x08F91170 */
    int t;
    int done;
    int wait;
    ShopGlowUv halo[4];         /* +0x0C card-sized quad, drawn 8 times scaled up */
    int next;                   /* +0x5C next orb */
    ShopGlowFanVtx fan[6];      /* +0x60 orb: triangle fan */
    ShopGlowOrb orb[24];        /* +0xF0 */
    ShopGlowUv spark_uv_a[8][2];        /* +0x6F0 sprite sheet 4 x 2 */
    ShopGlowSparks spark_a[24][2];      /* +0x830 */
    ShopGlowUv spark_uv_b[4][2];        /* +0x4130 sprite sheet 2 x 2 */
    ShopGlowSparks spark_b[24][2];      /* +0x41D0 */
} ShopGlowSuper;

typedef struct ShopGlowBurst {  /* burst of 16 sparks out of the card */
    unsigned int flags;         /* 1 alive */
    int t;
    int kind[16];               /* +0x08 sprite of the sheet */
    Vec4 dir[16];               /* +0x50 */
} ShopGlowBurst;

typedef struct ShopGlowRay {    /* fan of 16 light rays */
    unsigned int flags;         /* 1 alive */
    int t;
    int kind[16];               /* +0x08 */
    float ang[16];              /* +0x48 */
} ShopGlowRay;

typedef struct ShopGlowMaskVtx {        /* GE vertex 0x800100: s16 xyz, screen coordinates */
    short x, y, z;
} ShopGlowMaskVtx;

typedef struct ShopGlowUltra {  /* shop_glow_ultra_st 0x08F8EF30 */
    int t;
    int done;
    int wait;
    ShopGlowUv halo[4];         /* +0x0C */
    ShopGlowFanVtx fan[6];      /* +0x5C flash at the card center */
    int next_burst;             /* +0xEC */
    ShopGlowUv spark_uv[8][2];  /* +0xF0 sprite sheet 4 x 2 */
    ShopGlowBurst burst[16];    /* +0x230 */
    ShopGlowMaskVtx mask[2];    /* +0x1730 card rectangle on screen */
    int next_ray;               /* +0x173C */
    ShopGlowUv ray_uv[8][4];    /* +0x1740 */
    ShopGlowRay ray[16];        /* +0x19C0 */
} ShopGlowUltra;

extern ShopGlowBar shop_glow_bar_st;
extern ShopGlowRare shop_glow_rare_st;
extern ShopGlowSuper shop_glow_super_st;
extern ShopGlowUltra shop_glow_ultra_st;

typedef struct RandSeed {       /* BOOT rand3_seeds entry, 12 bytes (variable imports, 0 in the relocated ELF) */
    unsigned int lcg;           /* x * 0x41C64E6D + 0x3039 */
    unsigned int b, c;
} RandSeed;
extern int rand3_slot;
extern RandSeed rand3_seeds[];
#define GLOW_RAND(r) ((r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, (int)((r)->lcg >> 16 & 0x7FFF))

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
float sinf(float x);
void mtx_identity(void);
void mtx_mult(Mat4 *m);
Vec4 *vec4_copy(Vec4 *out, Vec4 *v);
Vec4 *vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_scale_w(Vec4 *out, Vec4 *a, float s);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
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

#ifdef NON_MATCHING /* mesma sequência de chamadas e constantes; difere nos spills dos contadores dos laços (4 slots de ponteiro + 2 de contador no original) e no $f2/$v1 mantido sobre shop_glow_ratio, que no original é static */
/* super rare: halo pulsing out of the card, the light bar, orbs rising around the card that leave
   two kinds of sparks */
int shop_glow_super(int t, Mat4 *m, unsigned short speed, void *vram)
{
    ShopGlowSuper *g = &shop_glow_super_st;
    int first = t == 0;
    Mat4 view, rot;
    Vec4 a, b, c;
    float fade_in, fade_out, k, y, ph;
    unsigned int *list;
    RandSeed *r;
    int i, j, n;

    if (first == 1)
        memset(g, 0, sizeof(ShopGlowSuper));
    if (g->done == 1) {
        g->wait += speed;
        return g->wait >= 24;
    }
    shop_glow_ge_state(3);
    shop_glow_view_rot(&view);
    fade_in = shop_glow_ratio(g->t, 8);
    fade_out = shop_glow_ratio_inv(g->t - 34, 16);
    mat4_identity(&rot);
    mat4_mul_rot_y(&rot, &rot, 3.14159274f);
    mat4_mul(&rot, m, &rot);

    /* halo */
    if (first == 1) {
        float w = 2.689f, h = 3.817f;

        g->halo[0].u = 0.0f;
        g->halo[0].v = 0.0f;
        g->halo[0].x = -w;
        g->halo[0].y = h;
        g->halo[0].z = 0.0f;
        g->halo[1].u = 0.0f;
        g->halo[1].v = 0.6015625f;
        g->halo[1].x = -w;
        g->halo[1].y = -h;
        g->halo[1].z = 0.0f;
        g->halo[2].u = 0.84375f;
        g->halo[2].v = 0.0f;
        g->halo[2].x = w;
        g->halo[2].y = h;
        g->halo[2].z = 0.0f;
        g->halo[3].u = 0.84375f;
        g->halo[3].v = 0.6015625f;
        g->halo[3].x = w;
        g->halo[3].y = -h;
        g->halo[3].z = 0.0f;
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 0, 1, 1);
    list = shop_glow_depth(list, 0, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    list = dl_tex_image(list, 0, 128, 256, 128, vram);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&rot);
    list = dl_ambient_light(list, (unsigned int)(255.0f * fade_in * fade_out) << 24 | 0xFFFFFF);
    for (i = 0; i < 8; i++) {
        k = 1.0f + 0.3f * i / 8.0f;
        mtx_push();
        shop_glow_scale(k, k, 0.0f);
        list = dl_color(list, (8 - i) * 100 / 8 << 24 | 0xFFBE82);
        list = draw_array(list, 4, 0x183, 4, 0, g->halo);
        mtx_pop();
    }
    mtx_pop();
    dl_set_list(list);
    shop_glow_draw_bar(first, &rot, g->t, 8, 0x82, 0xBE, 0xFF, 200);

    /* orbs */
    if (first == 1) {
        float one = 1.0f;

        g->fan[0].u = 0.5f;
        g->fan[0].v = 0.5f;
        g->fan[0].color = 0xFFFFBE82;
        g->fan[0].x = 0.0f;
        g->fan[0].y = 0.0f;
        g->fan[0].z = 0.0f;
        g->fan[1].u = 0.0f;
        g->fan[1].v = 0.0f;
        g->fan[1].color = 0xFFBE82;
        g->fan[1].x = -one;
        g->fan[1].y = one;
        g->fan[1].z = 0.0f;
        g->fan[2].u = one;
        g->fan[2].v = 0.0f;
        g->fan[2].color = 0xFFBE82;
        g->fan[2].x = one;
        g->fan[2].y = one;
        g->fan[2].z = 0.0f;
        g->fan[3].u = one;
        g->fan[3].v = one;
        g->fan[3].color = 0xFFBE82;
        g->fan[3].x = one;
        g->fan[3].y = -one;
        g->fan[3].z = 0.0f;
        g->fan[4].u = 0.0f;
        g->fan[4].v = one;
        g->fan[4].color = 0xFFBE82;
        g->fan[4].x = -one;
        g->fan[4].y = -one;
        g->fan[4].z = 0.0f;
        g->fan[5] = g->fan[1];
    }
    for (i = 0; i < speed; i++) {
        ShopGlowOrb *o = &g->orb[g->next];

        o->flags |= 1;
        o->t = 0;
        j = 0;
        do {
            r = &rand3_seeds[rand3_slot];
            o->ang[j] = 6.2831855f * ((float)GLOW_RAND(r) / 32767.0f);
            o->rad[j] = 1.5f * ((float)GLOW_RAND(r) / 32767.0f);
        } while (++j < 2);
        if (g->next < 23)
            g->next++;
        else
            g->next = 0;
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 1, 1, 1);
    list = shop_glow_depth(list, 1, 1);
    list = shop_glow_blend(list, 1, 0, 6, 10, 0, -1);
    list = shop_glow_set_res(list, 0x2041);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        ShopGlowOrb *o = &g->orb[i];
        float f;
        unsigned int al;

        if (!(o->flags & 1) == 1)
            continue;
        f = o->t / 24.0f;
        al = fade_out * (fade_in * (255.0f * shop_glow_ratio(o->t, 4) * shop_glow_ratio_inv(o->t - 20, 4)));
        list = dl_ambient_light(list, al << 24 | 0xFFFFFF);
        ph = 3.14159274f * f;
        o->alpha = al;
        y = 11.0f * f - 5.5f;
        for (j = 0; j < 2; j++) {
            float an = o->ang[j] + 0.13962634f * o->t;
            float rr = 3.0f + o->rad[j] * sinf(ph);

            vec4_set(&o->pos[j], rr * sinf(an), y, 0.5f * (rr * cosf(an)), 1.0f);
            mat4_transform_vec4(&a, &rot, &o->pos[j]);
            mtx_push();
            shop_glow_translate(a.x, a.y, a.z);
            mtx_mult(&view);
            list = draw_array(list, 5, 0x19F, 6, 0, g->fan);
            mtx_pop();
        }
        o->t += speed;
        if (o->t >= 24)
            o->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* small sparks */
    if (first == 1) {
        for (i = 0; i < 8; i++) {
            float du = 0.25f, dv = 0.5f, k = 0.3f, u = i % 4 * du, v = i / 4 * dv;

            g->spark_uv_a[i][0].u = u;
            g->spark_uv_a[i][0].v = v;
            g->spark_uv_a[i][0].x = -k;
            g->spark_uv_a[i][0].y = k;
            g->spark_uv_a[i][0].z = 0.0f;
            g->spark_uv_a[i][1].u = u + du;
            g->spark_uv_a[i][1].v = v + dv;
            g->spark_uv_a[i][1].x = k;
            g->spark_uv_a[i][1].y = -k;
            g->spark_uv_a[i][1].z = 0.0f;
        }
    }
    for (i = 0; i < 24; i++) {
        ShopGlowOrb *o = &g->orb[i];

        if (!(o->flags & 1) == 1)
            continue;
        for (j = 0; j < 2; j++) {
            ShopGlowSparks *q = &g->spark_a[i][j];
            ShopGlowSpark *s = &q->s[q->next];
            float an;

            s->flags |= 1;
            s->t = 0;
            s->alpha = o->alpha;
            vec4_copy(&s->pos, &o->pos[j]);
            r = &rand3_seeds[rand3_slot];
            an = 6.2831855f * ((float)GLOW_RAND(r) / 32767.0f);
            s->kind = GLOW_RAND(r) % 8;
            vec4_set(&s->vel, 0.2f * sinf(an), -1.5f, 0.2f * cosf(an), 1.0f);
            if (q->next < 5)
                q->next++;
            else
                q->next = 0;
        }
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 0, 1, 1);
    list = shop_glow_depth(list, 1, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    list = shop_glow_set_res(list, 0x2040);
    list = dl_color(list, 0xFFFFBE82);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        for (j = 0; j < 2; j++) {
            for (n = 0; n < 6; n++) {
                ShopGlowSpark *s = &g->spark_a[i][j].s[n];
                float f;

                if (!(s->flags & 1) == 1)
                    continue;
                f = s->t / 6.0f;
                list = dl_ambient_light(list, (unsigned int)(fade_out * (fade_in * ((float)s->alpha * shop_glow_ease(s->t, 6, 1)))) << 24 | 0xFFFFFF);
                vec4_set(&b, s->vel.x, s->vel.y * f, s->vel.z, 1.0f);
                vec3_add_w(&b, &b, &s->pos);
                mat4_transform_vec4(&b, &rot, &b);
                mtx_push();
                shop_glow_translate(b.x, b.y, b.z);
                mtx_mult(&view);
                list = draw_array(list, 6, 0x183, 2, 0, g->spark_uv_a[s->kind]);
                mtx_pop();
                s->t += speed;
                if (s->t >= 6)
                    s->flags &= ~1;
            }
        }
    }
    mtx_pop();
    dl_set_list(list);

    /* large sparks */
    if (first == 1) {
        for (i = 0; i < 4; i++) {
            float du = 0.5f, dv = 0.5f, k = 0.6f, u = i % 2 * du, v = i / 2 * dv;

            g->spark_uv_b[i][0].u = u;
            g->spark_uv_b[i][0].v = v;
            g->spark_uv_b[i][0].x = -k;
            g->spark_uv_b[i][0].y = k;
            g->spark_uv_b[i][0].z = 0.0f;
            g->spark_uv_b[i][1].u = u + du;
            g->spark_uv_b[i][1].v = v + dv;
            g->spark_uv_b[i][1].x = k;
            g->spark_uv_b[i][1].y = -k;
            g->spark_uv_b[i][1].z = 0.0f;
        }
    }
    for (i = 0; i < 24; i++) {
        ShopGlowOrb *o = &g->orb[i];

        if (!(o->flags & 1) == 1)
            continue;
        for (j = 0; j < 2; j++) {
            ShopGlowSparks *q = &g->spark_b[i][j];
            ShopGlowSpark *s = &q->s[q->next];
            float an;

            s->flags |= 1;
            s->t = 0;
            s->alpha = o->alpha;
            vec4_copy(&s->pos, &o->pos[j]);
            r = &rand3_seeds[rand3_slot];
            an = 6.2831855f * ((float)GLOW_RAND(r) / 32767.0f);
            s->kind = GLOW_RAND(r) % 4;
            vec4_set(&s->vel, 0.3f * sinf(an), -1.5f, 0.3f * cosf(an), 1.0f);
            if (q->next < 5)
                q->next++;
            else
                q->next = 0;
        }
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 0, 1, 1);
    list = shop_glow_depth(list, 1, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    list = shop_glow_set_res(list, 0x2042);
    list = dl_color(list, 0xC8FFBE82);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 24; i++) {
        for (j = 0; j < 2; j++) {
            for (n = 0; n < 6; n++) {
                ShopGlowSpark *s = &g->spark_b[i][j].s[n];
                float f;

                if (!(s->flags & 1) == 1)
                    continue;
                f = s->t / 6.0f;
                list = dl_ambient_light(list, (unsigned int)(fade_out * (fade_in * ((float)s->alpha * shop_glow_ease(s->t, 6, 1)))) << 24 | 0xFFFFFF);
                vec4_set(&c, s->vel.x, s->vel.y * f, s->vel.z, 1.0f);
                vec3_add_w(&c, &c, &s->pos);
                mat4_transform_vec4(&c, &rot, &c);
                mtx_push();
                shop_glow_translate(c.x, c.y, c.z);
                mtx_mult(&view);
                list = draw_array(list, 6, 0x183, 2, 0, g->spark_uv_b[s->kind]);
                mtx_pop();
                s->t += speed;
                if (s->t >= 6)
                    s->flags &= ~1;
            }
        }
    }
    mtx_pop();
    dl_set_list(list);
    g->t += speed;
    if (g->t >= 50)
        g->done = 1;
    return 0;
}
#endif

#ifdef NON_MATCHING /* mesma sequência de chamadas e constantes, 4132 B contra 4140; difere na alocação dos $s, no layout da pilha (mvp abaixo de scr e a no original) e no $f/$v mantido sobre shop_glow_ratio, que no original é static */
/* ultra/ultimate/secret rare: halo, light bar, a flash at the card center, bursts of sparks out of
   the card (masked to its rectangle) and fans of rays */
int shop_glow_ultra(int t, Mat4 *m, unsigned short speed, void *vram)
{
    ShopGlowUltra *g = &shop_glow_ultra_st;
    int first = t == 0;
    Mat4 view, rot;
    Vec4 a;
    float fade_in, fade_out, k;
    unsigned int *list;
    unsigned int al;
    RandSeed *r;
    int i, j;

    if (first == 1)
        memset(g, 0, sizeof(ShopGlowUltra));
    if (g->done == 1) {
        g->wait += speed;
        return g->wait >= 24;
    }
    shop_glow_ge_state(3);
    shop_glow_view_rot(&view);
    fade_in = shop_glow_ratio(g->t, 16);
    fade_out = shop_glow_ratio_inv(g->t - 18, 32);
    mat4_identity(&rot);
    mat4_mul_rot_y(&rot, &rot, 3.14159274f);
    mat4_mul(&rot, m, &rot);

    /* halo */
    if (first == 1) {
        float w = 2.689f, h = 3.817f;

        g->halo[0].u = 0.0f;
        g->halo[0].v = 0.0f;
        g->halo[0].x = -w;
        g->halo[0].y = h;
        g->halo[0].z = 0.0f;
        g->halo[1].u = 0.0f;
        g->halo[1].v = 0.6015625f;
        g->halo[1].x = -w;
        g->halo[1].y = -h;
        g->halo[1].z = 0.0f;
        g->halo[2].u = 0.84375f;
        g->halo[2].v = 0.0f;
        g->halo[2].x = w;
        g->halo[2].y = h;
        g->halo[2].z = 0.0f;
        g->halo[3].u = 0.84375f;
        g->halo[3].v = 0.6015625f;
        g->halo[3].x = w;
        g->halo[3].y = -h;
        g->halo[3].z = 0.0f;
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 0, 1, 1);
    list = shop_glow_depth(list, 0, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    list = dl_tex_image(list, 0, 128, 256, 128, vram);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&rot);
    al = (unsigned int)(255.0f * fade_in * fade_out) << 24 | 0xFFFFFF;
    list = dl_ambient_light(list, al);
    for (i = 0; i < 8; i++) {
        k = 1.0f + 0.3f * i / 8.0f;
        mtx_push();
        shop_glow_scale(k, k, 0.0f);
        list = dl_color(list, (8 - i) * 100 / 8 << 24 | 0x6ED2FF);
        list = draw_array(list, 4, 0x183, 4, 0, g->halo);
        mtx_pop();
    }
    mtx_pop();
    dl_set_list(list);
    shop_glow_draw_bar(first, &rot, g->t, 8, 0xFF, 0xD2, 0x6E, 0xFF);

    /* flash */
    if (first == 1) {
        float six = 6.0f;

        g->fan[0].u = 0.5f;
        g->fan[0].v = 0.5f;
        g->fan[0].color = 0xFF6ED2FF;
        g->fan[0].x = 0.0f;
        g->fan[0].y = 0.0f;
        g->fan[0].z = 0.0f;
        g->fan[1].u = 0.0f;
        g->fan[1].v = 0.0f;
        g->fan[1].color = 0x6ED2FF;
        g->fan[1].x = -six;
        g->fan[1].y = six;
        g->fan[1].z = 0.0f;
        g->fan[2].u = 1.0f;
        g->fan[2].v = 0.0f;
        g->fan[2].color = 0x6ED2FF;
        g->fan[2].x = six;
        g->fan[2].y = six;
        g->fan[2].z = 0.0f;
        g->fan[3].u = 1.0f;
        g->fan[3].v = 1.0f;
        g->fan[3].color = 0x6ED2FF;
        g->fan[3].x = six;
        g->fan[3].y = -six;
        g->fan[3].z = 0.0f;
        g->fan[4].u = 0.0f;
        g->fan[4].v = 1.0f;
        g->fan[4].color = 0x6ED2FF;
        g->fan[4].x = -six;
        g->fan[4].y = -six;
        g->fan[4].z = 0.0f;
        g->fan[5] = g->fan[1];
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 1, 1, 1);
    list = shop_glow_depth(list, 0, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    list = shop_glow_set_res(list, 0x2041);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    shop_glow_translate(rot.m[12], rot.m[13], rot.m[14]);
    mtx_mult(&view);
    list = dl_ambient_light(list, al);
    list = draw_array(list, 5, 0x19F, 6, 0, g->fan);
    mtx_pop();
    dl_set_list(list);

    /* bursts */
    if (first == 1) {
        for (i = 0; i < 8; i++) {
            float du = 0.25f, dv = 0.5f, k = 0.3f, u = i % 4 * du, v = i / 4 * dv;

            g->spark_uv[i][0].u = u;
            g->spark_uv[i][0].v = v;
            g->spark_uv[i][0].x = -k;
            g->spark_uv[i][0].y = k;
            g->spark_uv[i][0].z = 0.0f;
            g->spark_uv[i][1].u = u + du;
            g->spark_uv[i][1].v = v + dv;
            g->spark_uv[i][1].x = k;
            g->spark_uv[i][1].y = -k;
            g->spark_uv[i][1].z = 0.0f;
        }
    }
    for (i = 0; i < speed; i++) {
        ShopGlowBurst *b = &g->burst[g->next_burst];

        b->flags |= 1;
        b->t = 0;
        for (j = 0; j < 16; j++) {
            float an, len;

            r = &rand3_seeds[rand3_slot];
            an = 6.2831855f * ((float)GLOW_RAND(r) / 32767.0f);
            len = 1.0f + 5.0f * ((float)GLOW_RAND(r) / 32767.0f);
            b->kind[j] = GLOW_RAND(r) % 8;
            vec4_set(&b->dir[j], len * sinf(an), len * cosf(an), 0.0f, 1.0f);
        }
        if (g->next_burst < 15)
            g->next_burst++;
        else
            g->next_burst = 0;
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 0, 1, 1);
    list = shop_glow_depth(list, 0, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    list = shop_glow_set_res(list, 0x2040);
    list = dl_color(list, 0xFF6ED2FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    for (i = 0; i < 16; i++) {
        ShopGlowBurst *b = &g->burst[i];
        unsigned int ba;
        float sc;

        if (!(b->flags & 1) == 1)
            continue;
        ba = fade_out * (fade_in * (255.0f * sinf(3.14159274f * b->t / 16.0f)));
        sc = 0.7f + 0.3f * shop_glow_ratio(b->t, 16);
        list = dl_ambient_light(list, ba << 24 | 0xFFFFFF);
        for (j = 0; j < 16; j++) {
            vec3_scale_w(&a, &b->dir[j], sc);
            mat4_transform_vec4(&a, &rot, &a);
            mtx_push();
            shop_glow_translate(a.x, a.y, a.z);
            mtx_mult(&view);
            list = draw_array(list, 6, 0x183, 2, 0, g->spark_uv[b->kind[j]]);
            mtx_pop();
        }
        b->t += speed;
        if (b->t >= 16)
            b->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);

    /* the sparks only show over the card: depth mask of its rectangle */
    if (first == 1) {
        float w = 2.689f, h = 3.817f;
        Vec4 scr[2];
        Mat4 mvp;

        vec4_set(&scr[0], -w, h, 0.0f, 1.0f);
        vec4_set(&scr[1], w, -h, 0.0f, 1.0f);
        shop_glow_mvp(&mvp);
        for (i = 0; i < 2; i++) {
            mat4_transform_vec4(&scr[i], &mvp, &scr[i]);
            scr[i].x = 2048.0f + 240.0f * scr[i].x / scr[i].w - 1808.0f;
            scr[i].y = 2048.0f + -136.0f * scr[i].y / scr[i].w - 1912.0f;
            g->mask[i].x = scr[i].x;
            g->mask[i].y = scr[i].y;
            g->mask[i].z = -1;
        }
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 0, 1, 1, 0, 0, 1, 1);
    list = shop_glow_depth(list, 1, 0);
    list = shop_glow_blend(list, 1, 0, 2, 3, 0, 0);
    list = dl_color(list, 0);
    list = dl_alpha_test(list, 7, 0, 0xFF);
    list = draw_array(list, 6, 0x800100, 2, 0, g->mask);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    dl_set_list(list);

    /* rays */
    if (first == 1) {
        for (i = 0; i < 8; i++) {
            float du = 0.25f, dv = 0.5f, k = 0.3f, z = 10.0f, u = i % 4 * du, v = i / 4 * dv;

            g->ray_uv[i][0].u = u;
            g->ray_uv[i][0].v = v;
            g->ray_uv[i][0].x = -k;
            g->ray_uv[i][0].y = 2.0f;
            g->ray_uv[i][0].z = -z;
            g->ray_uv[i][1].u = u;
            g->ray_uv[i][1].v = v + dv;
            g->ray_uv[i][1].x = -k;
            g->ray_uv[i][1].y = 2.0f;
            g->ray_uv[i][1].z = z;
            g->ray_uv[i][2].u = u + du;
            g->ray_uv[i][2].v = v;
            g->ray_uv[i][2].x = k;
            g->ray_uv[i][2].y = 2.0f;
            g->ray_uv[i][2].z = -z;
            g->ray_uv[i][3].u = u + du;
            g->ray_uv[i][3].v = v + dv;
            g->ray_uv[i][3].x = k;
            g->ray_uv[i][3].y = 2.0f;
            g->ray_uv[i][3].z = z;
        }
    }
    for (i = 0; i < speed; i++) {
        ShopGlowRay *y = &g->ray[g->next_ray];
        RandSeed *r2;

        y->flags |= 1;
        y->t = 0;
        j = 0;
        do {
            r = &rand3_seeds[rand3_slot];
            y->kind[j] = GLOW_RAND(r) % 8;
            r2 = &rand3_seeds[rand3_slot];
            y->ang[j] = 6.2831855f * ((float)GLOW_RAND(r2) / 32767.0f);
        } while (++j < 16);
        if (g->next_ray < 15)
            g->next_ray++;
        else
            g->next_ray = 0;
    }
    list = dl_select_layer(3, 0);
    list = shop_glow_tex(list, 1, 1, 1, 0, 0, 1, 1);
    list = shop_glow_depth(list, 1, 1);
    list = shop_glow_blend(list, 1, 0, 2, 10, 0, -1);
    list = shop_glow_set_res(list, 0x2040);
    list = dl_color(list, 0xFF6ED2FF);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&rot);
    for (i = 0; i < 16; i++) {
        ShopGlowRay *y = &g->ray[i];
        unsigned int ya;

        if (!(y->flags & 1) == 1)
            continue;
        ya = fade_out * (fade_in * (255.0f * shop_glow_ratio(y->t, 4)));
        k = 20.0f * y->t / 16.0f;
        list = dl_ambient_light(list, ya << 24 | 0xFFFFFF);
        mtx_push();
        shop_glow_translate(0.0f, 0.0f, k);
        for (j = 0; j < 16; j++) {
            mtx_push();
            mtx_rotate_z(y->ang[j]);
            list = draw_array(list, 4, 0x183, 4, 0, g->ray_uv[y->kind[j]]);
            mtx_pop();
        }
        mtx_pop();
        y->t += speed;
        if (y->t >= 16)
            y->flags &= ~1;
    }
    mtx_pop();
    dl_set_list(list);
    g->t += speed;
    if (g->t >= 50)
        g->done = 1;
    return 0;
}
#endif
