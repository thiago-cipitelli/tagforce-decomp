// @range 0x0882BF84-0x0882C214
// @data 0x08876A68-0x08876A74
/* A card lying on the field drawn as a textured quad (62x88 in the XZ plane) with the card image
   rendered into VRAM, a texture scale that grows with t and a given alpha. */
#include "dd_d3.h"

typedef struct DdCardVert {            /* float texture + float position */
    float u, v;
    float x, y, z;
} DdCardVert;

extern DdCardVert dd_card_quad[4];

void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(float *v);
void mtx_rotate_zyx(float *v);
void mtx_scale3(float *v);
unsigned int *draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *verts);
unsigned int *dd_tex_card(unsigned int *list, int id, int mode);
unsigned int *dd_depth_enable(unsigned int *list, int on, int mask);
unsigned int *dd_blend_enable(unsigned int *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
unsigned int *dd_tex_enable_uv(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v, float u, float v, float su, float sv);

void dd_card_quad_draw(int id, float *pos, float *rot, float *scale, int alpha, float t);

void dd_card_quad_init(void)
{
    float hu = 0.4921875f;
    float hw = 31.0f;
    float hh = 44.0f;

    dd_card_quad[0].u = -hu;
    dd_card_quad[0].v = -hu;
    dd_card_quad[0].x = -hw;
    dd_card_quad[0].y = 0.0f;
    dd_card_quad[0].z = -hh;
    dd_card_quad[1].u = -hu;
    dd_card_quad[1].v = hu;
    dd_card_quad[1].x = -hw;
    dd_card_quad[1].y = 0.0f;
    dd_card_quad[1].z = hh;
    dd_card_quad[2].u = hu;
    dd_card_quad[2].v = -hu;
    dd_card_quad[2].x = hw;
    dd_card_quad[2].y = 0.0f;
    dd_card_quad[2].z = -hh;
    dd_card_quad[3].u = hu;
    dd_card_quad[3].v = hu;
    dd_card_quad[3].x = hw;
    dd_card_quad[3].y = 0.0f;
    dd_card_quad[3].z = hh;
}

void dd_card_quad_draw1(int id, float *pos, float *rot, int alpha, float t)
{
    float scale[3] = { 1.0f, 1.0f, 1.0f };

    dd_card_quad_draw(id, pos, rot, scale, alpha, t);
}

void dd_card_quad_draw(int id, float *pos, float *rot, float *scale, int alpha, float t)
{
    unsigned int *list = dl_select_layer(4, 0);
    float s;

    list = dd_depth_enable(list, 1, 1);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    mtx_translate(pos);
    mtx_rotate_zyx(rot);
    mtx_scale3(scale);
    list = dl_ambient_light(list, (alpha << 24) | 0xFFFFFF);
    list = dd_tex_card(list, id, 1);
    s = 1.0f + t / 64.0f;
    list = dd_tex_enable_uv(list, 1, 1, 1, 0, 0, 1, 1, 0.5f, 0.5f, s, s);
    list = dd_blend_enable(list, 1, 0, 2, 3, 0, 0);
    list = dl_color(list, 0xFFFFFFFF);
    list = draw_array(list, 4, 0x183, 4, 0, dd_card_quad);
    mtx_pop();
    dl_set_list(list);
}
