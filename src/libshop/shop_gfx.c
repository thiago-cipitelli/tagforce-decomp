// @range 0x088052F8-0x08805AB4
// @data 0x0881C890-0x0881C900
/* The lit layer of the pack opening and the card layer (light, projection, cameras), the GE state they
   share, and helpers over the bought list (card under the cursor, adding the cards to the collection,
   copies of a card) and the shelf boxes. */
#include "libshop.h"

extern Light shop_light;
extern Vec4 shop_cam2_pos, shop_cam2_target, shop_cam2_up;
extern Vec4 shop_cam3_pos, shop_cam3_target, shop_cam3_up;



/* lit 3D layer: the pack opening */
void shop_gfx_open(void)
{
    unsigned int *list;

    list = dl_select_layer(2, 0);
    memset(&shop_light, 0, sizeof(shop_light));
    shop_light.type = 0;
    shop_light.ambient = 0xFF000000;
    shop_light.diffuse = 0xFFFFFFFF;
    shop_light.specular = 0;
    shop_light.pos = (Vec3){ 0.0f, -0.227982432f, 0.973665237f };
    shop_light.att[0] = 1.0f;
    list = lighting_enable(list);
    list = light_enable(list, 0);
    list = light_set_ambient(list, 0xFF4C4C4C);
    list = shop_ge_state(list);
    list = dl_fog(0.0f, 30.0f, list, 0x330019);
    list = dl_enable(list, 7);
    list = dl_set_tex_filter(list, 1, 1);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(1.55479038f, 1.7647059f, 1.0f, 20.0f);
    mtx_set_mode(1);
    mtx_identity();
    mtx_set_mode(2);
    mtx_identity();
    mtx_set_mode(1);
    {
        Vec4 pos, target, up;
        up = shop_cam2_up;
        target = shop_cam2_target;
        pos = shop_cam2_pos;
        camera_set(&pos, &target, &up);
    }
    camera_load_view();
    list = dl_set_tex_filter(list, 1, 1);
    dl_set_list(list);
}

Vec4 shop_cam2_pos = { 1.53244734f, 3.12566876f, 6.26977777f, 1.0f };
Vec4 shop_cam2_target = { -21.3451767f, -66.1514969f, -62.1209259f, 1.0f };
Vec4 shop_cam2_up = { -0.283192217f, 0.719516993f, -0.634111583f, 0.0f };

/* unlit 3D layer: the cards coming out */
void shop_gfx_cards(void)
{
    unsigned int *list;

    list = dl_select_layer(2, 0);
    list = lighting_disable(list);
    list = shop_ge_state(list);
    list = dl_fog(50.0f, 100.0f, list, 0x404040);
    list = dl_disable(list, 7);
    list = dl_set_tex_filter(list, 1, 1);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(0.696166515f, 1.7647059f, 1.0f, 100.0f);
    mtx_set_mode(1);
    mtx_identity();
    mtx_set_mode(2);
    mtx_identity();
    mtx_set_mode(1);
    {
        Vec4 pos, target, up;
        up = shop_cam3_up;
        target = shop_cam3_target;
        pos = shop_cam3_pos;
        camera_set(&pos, &target, &up);
    }
    camera_load_view();
    list = dl_set_tex_filter(list, 1, 1);
    dl_set_list(list);
}

Vec4 shop_cam3_pos = { -2.26097107f, 17.3653641f, 17.779047f, 1.0f };
Vec4 shop_cam3_target = { 11.4001017f, -27.6475983f, -70.4661179f, 1.0f };
Vec4 shop_cam3_up = { 0.0688634515f, 0.892963231f, -0.444830865f, 0.0f };

/* GE state shared by the 3D layers */
unsigned int *shop_ge_state(unsigned int *list)
{
    list = dl_offset(list, 0x710, 0x778);
    list = dl_viewport(list, 0x800, 0x800, 480, 272);
    list = dl_depth_range(list, 0xFFFF, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 4);
    list = dl_scissor(list, 0, 0, 480, 272);
    list = dl_enable(list, 2);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_disable(list, 0);
    list = dl_depth_func(list, 7);
    list = dl_enable(list, 1);
    list = dl_front_face(list, 1);
    list = dl_enable(list, 5);
    list = dl_disable(list, 9);
    list = dl_shade_model(list, 1);
    list = dl_enable(list, 8);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x14);
    return dl_color(list, 0xFF0000FF);
}

/* the card under the cursor; restarts the info timer when it changes */
void shop_sel_update(void)
{
    ShopItem *it = shop_cur_item();
    int id;

    if (it == 0) {
        shop.sel_id = -1;
    } else if ((id = it->id) == 0) {
        shop.sel_id = -1;
    } else if (shop.sel_id != id) {
        shop.sel_id = id;
        shop.sel_timer = 0;
    }
}

/* mode 1: waits for the deck pack, then mode 2 */
void shop_wait_deck_pack(void)
{
    switch (shop.mode) {
    case 1:
        if (shop_load_deck_pack())
            shop.mode = 2;
        break;
    }
}

/* adds the bought cards to the collection */
void shop_items_commit(void)
{
    int i;
    int j;

    for (i = 0; i < shop.n_items; i++) {
        unsigned short n = shop.items[i].count;
        unsigned short id = shop.items[i].id;
        for (j = 0; j < n; j++)
            inv_adjust_count(id, 1);
    }
}

/* a pack on the shelf with its picture and name */
unsigned int *shop_draw_box(void *list, ShopBox *b, int x, int y)
{
    UiIconText p;

    memset(&p, 0, sizeof(p));
    p.w = 0x11;
    p.h = 0x11;
    p.u = b->x;
    p.v = b->y;
    p.gim = shop_res_get(b->gim);
    p.text = table_get_entry(shop_str_get(0xC001), b->name);
    return shop_ui_draw_icon_text(list, &p, x, y);
}

/* copies of card `id` in the bought list */
unsigned short shop_items_count(unsigned short id)
{
    unsigned short n = 0;
    int i;

    for (i = 0; i < shop.n_items; i++) {
        if (id == shop.items[i].id)
            n += shop.items[i].count;
    }
    return n;
}
