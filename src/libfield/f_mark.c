// @range 0x0882B238-0x0882B7C8
// @data 0x0883DBC0-0x0883DCA8
/* Stencil marker of the rooms (fmrk): a prism drawn into the stencil buffer over the target spot
   and a pulsing full-screen color where the stencil is set. */
#include "libfield.h"

void *dl_clear_depth(void *list, int z);
void *dl_clear(void *list, unsigned int flags);
void *dl_front_face(void *list, int cw);
void *dl_color(void *list, unsigned int color);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
int fld_bg_id(void);
void mtx_translate(Vec4 *v);
void mtx_rotate_y(float a);
void mtx_mult(Mat4 *m);
void mtx_scale3(Vec4 *s);

extern FieldVtx8 fmrk_side[15];
extern FieldVtx8 fmrk_cap[6];

void *fmrk_begin(void *list)
{
    list = dl_clear_depth(list, 0);
    list = dl_clear(list, 2);
    list = dl_disable(list, 9);
    list = dl_depth_mask(list, 1);
    list = dl_enable(list, 3);
    list = dl_stencil_op(list, 1, 0, 0xFF);
    list = dl_enable(list, 5);
    return dl_pixel_mask(list, 0xFFFFFF);
}

typedef struct FmrkVec {
    float v[4];
} FmrkVec;

FmrkVec fmrk_top = { { 0.0f, 0.5f, 0.0f, 0.0f } };

void *fmrk_volume(void *list, Vec4 *pos)
{
    FmrkVec top;
    float size[4] = { 0.25f, 0.25f, 0.25f, 0.0f };
    float scale[4] = { 1.1f, 2.0f, 1.1f, 0.0f };
    float off[4] = { 0.0f, 0.65f, 0.0f, 0.0f };
    float m[16];

    top = fmrk_top;
    switch (fld_bg_id()) {
    case 0x0D:
    case 0x0E:
    case 0x0F:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x26:
        if ((int)(pos->x / 0.5f) == 12 && (int)(pos->z / 0.5f) == 5)
            scale[2] -= 0.1f;
        break;
    case 3:
        scale[0] = 1.75f;
        scale[2] = 4.8f;
        break;
    }
    mat4_identity((Mat4 *)m);
    m[5] = -1.0f;
    m[10] = -1.0f;
    mtx_push();
    mtx_identity();
    mtx_translate(pos);
    mtx_translate((Vec4 *)off);
    if (fld_bg_id() == 3)
        mtx_rotate_y(-0.39269909f);
    mtx_mult((Mat4 *)m);
    mtx_scale3((Vec4 *)scale);
    mtx_translate((Vec4 *)&top);
    mtx_scale3((Vec4 *)size);
    list = dl_stencil_func(list, 0, 0, 4);
    list = dl_front_face(list, 1);
    list = draw_array(list, 4, 0xA1, 15, 0, fmrk_side);
    list = draw_array(list, 3, 0xA1, 6, 0, fmrk_cap);
    list = dl_stencil_func(list, 0, 0, 5);
    list = dl_front_face(list, 0);
    list = draw_array(list, 4, 0xA1, 15, 0, fmrk_side);
    list = draw_array(list, 3, 0xA1, 6, 0, fmrk_cap);
    mtx_pop();
    return list;
}

/* prism of the marker: 15 strip vertices for the sides, 6 for the caps */
FieldVtx8 fmrk_side[15] = {
    { 127, 255, 52, -51, 104, 127, -126, 127 },
    { 127, 255, 52, -51, 104, 127, -126, 127 },
    { 127, 255, 85, -84, -41, 127, -126, -126 },
    { 0, 255, -84, -84, 42, -126, -126, 127 },
    { 127, 255, 85, -84, -41, 127, -126, -126 },
    { 0, 255, -51, -51, -103, -126, -126, -126 },
    { 127, 254, 52, 52, -103, 127, 127, -126 },
    { 0, 254, -84, 85, -41, -126, 127, -126 },
    { 127, 127, 85, 85, 42, 127, 127, 127 },
    { 0, 127, -51, 52, 104, -126, 127, 127 },
    { 127, 0, 52, -51, 104, 127, -126, 127 },
    { 0, 127, -51, 52, 104, -126, 127, 127 },
    { 0, 0, -84, -84, 42, -126, -126, 127 },
    { 0, 127, -84, 85, -41, -126, 127, -126 },
    { 0, 0, -51, -51, -103, -126, -126, -126 },
};
FieldVtx8 fmrk_cap[6] = {
    { 127, 0, 52, -51, 104, 127, -126, 127 },
    { 254, 0, 85, -84, -41, 127, -126, -126 },
    { 127, 127, 85, 85, 42, 127, 127, 127 },
    { 254, 0, 85, -84, -41, 127, -126, -126 },
    { 254, 127, 52, 52, -103, 127, 127, -126 },
    { 127, 127, 85, 85, 42, 127, 127, 127 },
};

void *fmrk_fill(void *list)
{
    float t = fabsf_vfpu((int)(gfx_get_state()[0] % 40 - 20) / 20.0f);

    list = dl_enable(list, 9);
    list = dl_front_face(list, 1);
    list = dl_depth_mask(list, 0);
    list = dl_disable(list, 3);
    list = dl_stencil_func(list, 0, 0, 0);
    list = dl_pixel_mask(list, 0);
    list = dl_enable(list, 3);
    list = dl_disable(list, 9);
    list = dl_disable(list, 1);
    list = dl_stencil_op(list, 4, 0, 0xFF);
    list = dl_stencil_func(list, 0, 0, 0);
    list = dl_color(list, (unsigned int)(32.0f + 32.0f * t) << 24 | 0x8080FF);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = draw_set_offsets(list, 1, 1, 0x1E0, 0x110);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    list = dl_disable(list, 3);
    list = dl_enable(list, 9);
    list = dl_enable(list, 1);
    return dl_color(list, 0xFFFFFFFF);
}
