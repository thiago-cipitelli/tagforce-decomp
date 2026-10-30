// @range 0x088264DC-0x08826A64
/* GE render state of the duel effect layers: the default state of a layer and the helpers that
   switch texturing, depth writes and blending on or off in a display list. */
#include "libduel_draw.h"

unsigned int *dd_rstate_reset(int layer)
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
    return (unsigned int *)dl_set_list(list);
}

void dd_rstate_reset_all(void)
{
    dd_rstate_reset(1);
    dd_rstate_reset(2);
    dd_rstate_reset(4);
    dd_rstate_reset(6);
    dd_rstate_reset(7);
}

unsigned int *dd_tex_enable(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v)
{
    if (on == 1) {
        list = dl_enable(list, 9);
        list = dl_set_tex_filter(list, min, mag);
        list = dl_tex_func(list, func, comp);
        list = dl_tex_wrap(list, wrap_u, wrap_v);
        list = dl_tex_offset(list, 0.0f, 0.0f);
        list = dl_tex_scale(list, 1.0f, 1.0f);
    } else {
        list = dl_disable(list, 9);
    }
    return list;
}

unsigned int *dd_tex_enable_ofs(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v, float u, float v)
{
    if (on == 1) {
        list = dl_enable(list, 9);
        list = dl_set_tex_filter(list, min, mag);
        list = dl_tex_func(list, func, comp);
        list = dl_tex_wrap(list, wrap_u, wrap_v);
        list = dl_tex_offset(list, u, v);
        list = dl_tex_scale(list, 1.0f, 1.0f);
    } else {
        list = dl_disable(list, 9);
    }
    return list;
}

unsigned int *dd_tex_enable_uv(unsigned int *list, int on, int min, int mag, int func, int comp, int wrap_u, int wrap_v, float u, float v, float su, float sv)
{
    if (on == 1) {
        list = dl_enable(list, 9);
        list = dl_set_tex_filter(list, min, mag);
        list = dl_tex_func(list, func, comp);
        list = dl_tex_wrap(list, wrap_u, wrap_v);
        list = dl_tex_offset(list, u, v);
        list = dl_tex_scale(list, su, sv);
    } else {
        list = dl_disable(list, 9);
    }
    return list;
}

unsigned int *dd_depth_enable(unsigned int *list, int on, int mask)
{
    if (on == 1) {
        list = dl_enable(list, 1);
        list = dl_depth_mask(list, mask);
    } else {
        list = dl_disable(list, 1);
    }
    return list;
}

unsigned int *dd_blend_enable(unsigned int *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb)
{
    if (on == 1) {
        list = dl_enable(list, 4);
        list = dl_blend_func(list, op, src, dst, fixa, fixb);
    } else {
        list = dl_disable(list, 4);
    }
    return list;
}
