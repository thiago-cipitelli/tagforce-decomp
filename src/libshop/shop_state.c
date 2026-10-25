// @range 0x08805AB4-0x08807CAC
// @asm 0x08805F40-0x08806634
// @asm 0x08806CC4-0x088074FC
// @asm 0x08807810-0x08807914
// @asm 0x088079A8-0x08807CAC
// @rodata 0x0881C0E8-0x0881C124
// @data 0x0881C900-0x0881C970
/* Main state machine of the shop: the shelf with the 48 pack kinds (two rows of packs per kind, the
   cursor and the shelf rotation), buying a pack (it flies from the shelf to the counter), the list of
   the cards that came out (scrolling, marks, jumps between groups of the sort key) and the card info
   window; the button code on the shelf. */
#include "libshop.h"


unsigned short region_pick_value(int flag);
void se_play(int id);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int sysstate_get_u32_54(int i);
void sysstate_set_byte_3e38(int i);
int inv_get_flag7(unsigned short id);
void inv_set_flag7(unsigned short id, int on);
int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim);
void cardinfo_set_lists(void *list_gim, void *text_pack, void *index);
int cardinfo_has_alt_pict(void);
int cardinfo_update(void);
Mat4 *mat4_identity(Mat4 *out);
Mat4 *mat4_mul_rot_x(Mat4 *out, Mat4 *m, float a);
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a);
void mat4_copy(Mat4 *out, Mat4 *m);
void mat4_transform_vec4(Vec4 *out, Mat4 *m, Vec4 *v);
void vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
void vec4_copy(Vec4 *out, Vec4 *v);
Vec4 *mat3_to_quat(Vec4 *out, Mat4 *m);
Vec4 *quat_slerp(Vec4 *out, Vec4 *a, Vec4 *b, float t);
Mat4 *quat_to_mat4(Mat4 *out, Vec4 *q);

int shop_main_input(void);
void shop_shelf_input(void);
void shop_pick_input(void);
void shop_list_input(void);
void shop_buy_step(void);
void shop_info_step(void);
void shop_list_page(void);
void shop_list_mark(void);
void shop_list_jump(int back);
int shop_list_find_group(int back);
void shop_clerk_update(void);
void shop_tabs_update(void);
unsigned short *shop_pack_msg(int pack, int alt);
void shop_clerk_set(int what);
void shop_clerk_voice(int what);
void shop_list_open(void);
void shop_list_back(void);
void shop_buy_done_update(void);
int shop_result_input(void);
void shop_result_update(void);
int shop_menu_input(void);
void shop_menu_step(void);
void shop_card_detail(ShopItem *it);
void shop_help_open(void);
void shop_dialog_open(int what, int arg);
void shop_dialog_update(void);
void shop_info_input(int *sel);
void shop_tabs_hide(void);
void shop_points_add(int delta, int x, int y);
void shop_packs_refresh(void);
int shop_pack_can_buy(int pack);
unsigned short shop_pack_price(int pack);
int shop_bought_total(void);
void shop_stock_save(void);
void shop_open_start(void);
void shop_after_open(void);
int shop_item_key(ShopItem *it, int key);

/* the pack message has to follow the cursor */
inline void shop_pack_msg_refresh(void)
{
    if (shop.msg_dirty) {
        if ((unsigned short)shop_pack_can_buy(shop.pack) != 0) {
            shop_msg_set(shop_pack_msg(shop.pack, shop_clerk.alt), 0);
            shop_clerk_set(3);
        }
        shop.msg_dirty = 0;
    }
}

void shop_main_update(void)
{
    int side;
    int i;
    float f;

    if (shop.state < 7) {
        shop_clerk_update();
        shop_msgwin_update();
        shop_tabs_update();
    }
    switch (shop.state) {
    case 0:
        if (shop_msgwin_moving() == 0 && shop_clerk.state == 2) {
            shop.state = 1;
            shop_msg_set(shop_pack_msg(shop.pack, shop_clerk.alt), 0);
            shop_clerk_set(3);
            if (shop.mode == 0)
                shop_clerk_voice(0);
        }
        break;
    case 1:
        if (shop.slide[0] < 0.0f)
            shop.slide[0] += 0.4f;
        if (shop.slide[1] < 0.0f)
            shop.slide[1] += 0.4f;
        if (shop.mode == 1)
            shop_wait_deck_pack();
        break;
    case 2:
        if (++shop.timer > 18) {
            shop.timer = 0;
            shop.state = 3;
        }
        break;
    case 4:
        for (side = 0; side < 2; side++) {
            for (i = 0; i < 10; i++) {
                f = shop.lift[side][i];
                if (SHELF_SIDE(shop.cur) != side || SHELF_IDX(shop.cur) != i) {
                    f -= 0.5f;
                    if (f < 0.0f)
                        shop.lift[side][i] = 0.0f;
                    else
                        shop.lift[side][i] = f;
                }
            }
        }
        shop_buy_step();
        break;
    case 3:
        for (side = 0; side < 2; side++) {
            for (i = 0; i < 10; i++) {
                f = shop.lift[side][i];
                if (SHELF_SIDE(shop.cur) == side && SHELF_IDX(shop.cur) == i) {
                    f += 0.5f;
                    if (f > 3.0f)
                        shop.lift[side][i] = 3.0f;
                    else
                        shop.lift[side][i] = f;
                } else {
                    f -= 0.5f;
                    if (f < 0.0f)
                        shop.lift[side][i] = 0.0f;
                    else
                        shop.lift[side][i] = f;
                }
            }
        }
        if (!SHELF_SIDE(shop.cur)) {
            if (shop.rot < 26.0f) {
                shop.rot += 8.666667f;
                if (shop.rot > 26.0f)
                    shop.rot = 26.0f;
            }
        } else {
            if (shop.rot > -26.0f) {
                shop.rot -= 8.666667f;
                if (shop.rot < -26.0f)
                    shop.rot = -26.0f;
            }
        }
        break;
    case 5:
        if (shop.timer < 3) {
            shop.timer = 0;
            shop.state = 1;
        } else {
            shop.timer -= 2;
        }
        break;
    case 6:
    case 8:
    case 10:
    case 11:
    case 12:
        break;
    case 7:
        if (shop.mode != 1) {
            if (fade_is_done()) {
                shop_open_start();
                shop.state = 8;
                gfx_overlay_enable(0);
            }
        } else {
            shop_wait_deck_pack();
            if (fade_is_done())
                gfx_overlay_enable(1);
        }
        break;
    case 9:
    case 13:
        if (shop.info)
            shop_info_step();
        break;
    case 14:
        if (fade_is_done()) {
            shop_after_open();
            fade_start(1, 0xFF000000, 16);
        }
        break;
    }
}

/* up up down down left right left right square circle */
unsigned int shop_code_seq[11] = { 0x10, 0x10, 0x40, 0x40, 0x80, 0x20, 0x80, 0x20, 0x4000, 0x2000, 0 };

#ifdef NON_MATCHING /* o primeiro break do switch vai direto ao epílogo com o move v0,zero no atraso; o original salta para o bloco comum do return 0 */
/* input of the current state; 1 once the shop can be left */
int shop_main_input(void)
{
    PadState *pad = pad_get();

    if (shop.state < 7) {
        if (pad->b.trig) {
            unsigned int code[11];
            memcpy(code, shop_code_seq, sizeof(code));
            if (pad->b.trig & code[shop.code_pos]) {
                shop.code_pos++;
                if (code[shop.code_pos] == 0) {
                    shop.code_done = 1;
                    shop_packs_refresh();
                    se_play(0x621A);
                    return 0;
                }
            } else {
                shop.code_pos = 0;
            }
        }
    } else if (shop.state >= 9 && shop.state < 15) {
        shop_info_input(&shop.sel_id);
    }
    if (shop.hud) {
        shop_dialog_update();
        return 0;
    }
    if (shop.info)
        return 0;
    switch (shop.state) {
    default:
        return 0;
    case 1:
        shop_shelf_input();
        break;
    case 3:
        shop_pick_input();
        break;
    case 9:
    case 10:
    case 11:
        shop_list_input();
        break;
    case 13:
        return shop_menu_input();
    case 12:
        shop_menu_step();
        break;
    case 8:
        if (shop_result_input())
            shop_result_update();
        break;
    case 6:
        if (fade_is_done() && snd_count_se() == 0) {
            gfx_overlay_enable(1);
            return 1;
        }
        break;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* o original testa o resultado de shop_pack_can_buy com 1 e depois 2 (beql) e põe os corpos na mesma ordem, com um bloco de junção; nem switch nem if/else reproduzem */
/* on the shelf: choose the pack kind, buy, leave */
void shop_shelf_input(void)
{
    PadState *pad = pad_get();
    unsigned short pack = shop.pack;
    unsigned short r;
    int n;

    if (shop.back || (pad->b.trig & region_pick_value(0))) {
        r = shop_pack_can_buy(shop.pack);
        switch (r) {
        case 1:
            if (shop_clerk.alt == 0)
                shop_msg_set(table_get_entry(shop_str_get(0xC001), 0x1D), 0);
            else
                shop_msg_set(table_get_entry(shop_str_get(0xC001), 0x1E), 0);
            se_play(0x6215);
            break;
        case 2:
            if (shop.back) {
                shop.back = 0;
            } else {
                shop_clerk_voice(2);
                se_play(0x6213);
            }
            shop.timer = 0;
            shop.state = 2;
            memset(shop.lift, 0, sizeof(shop.lift));
            if (shop.stock[shop.pack] == 0)
                shop.stock[shop.pack] = 0xAA;
            r = shop.stock[shop.pack];
            n = r & 0xF;
            if (n > 0) {
                shop.cur = n - 1;
                shop.rot = 26.0f;
            } else {
                shop.cur = (r >> 4 & 0xF) - 1 | 0x80;
                shop.rot = -26.0f;
            }
            break;
        }
    } else if (pad->b.trig & region_pick_value(1)) {
        if (shop_bought_total() > 0) {
            shop_stock_save();
            shop.state = 7;
            shop_msgwin_close();
            fade_start_in(1, 0xFF000000, 16);
            se_play(0x6214);
        } else {
            shop_clerk_voice(5);
            shop_dialog_open(1, 0);
            se_play(0x621A);
        }
    } else {
        if (pad->b.rep & 0x10)
            shop.pack = (shop.pack + 36) % 48;
        if (pad->b.rep & 0x40)
            shop.pack = (shop.pack + 12) % 48;
        if (pad->b.rep & 0x80)
            shop.pack = (shop.pack + 47) % 48;
        if (pad->b.rep & 0x20)
            shop.pack = (shop.pack + 1) % 48;
        if (pack != shop.pack) {
            shop_model_set_box(0x8002, shop.pack);
            shop_model_set_box(0x8001, shop.pack);
            shop_msg_set(0, 0);
            shop.msg_dirty = 1;
            shop.slide[1] = -5.0f;
            se_play(0x6212);
        }
        if ((pad->b.down & 0xF0) == 0)
            shop_pack_msg_refresh();
    }
}
#endif

/* in front of the shelf of one kind: pick the pack to buy */
void shop_pick_input(void)
{
    PadState *pad = pad_get();
    unsigned short side = SHELF_SIDE(shop.cur);
    int points;

    if (shop.back || (pad->b.trig & region_pick_value(1))) {
        shop.timer = 18;
        shop.state = 5;
    } else if (pad->b.rep & region_pick_value(0)) {
        points = sysstate_get_u32_54(0);
        if (points < shop_pack_price(shop.pack)) {
            shop_dialog_open(2, 0);
            shop_clerk_voice(4);
        } else {
            if (shop.mode == 0) {
                shop.mode = 1;
                shop_wait_deck_pack();
            }
            shop.state = 4;
            shop.step = 0;
        }
    }
    if (pad->b.rep & 0x10) {
        if (SHELF_IDX(shop.cur) > 0) {
            shop.cur--;
            se_play(0x6266);
        }
    }
    if (pad->b.rep & 0x40) {
        if (SHELF_IDX(shop.cur) < (shop.stock[shop.pack] >> (side << 2) & 0xF) - 1) {
            shop.cur++;
            se_play(0x6266);
        }
    }
    if (pad->b.rep & 0x80) {
        side = SHELF_SIDE(shop.cur);
        if ((shop.stock[shop.pack] & 0xF) > 0) {
            shop.cur = SHELF_IDX(shop.cur);
            if (SHELF_IDX(shop.cur) >= (shop.stock[shop.pack] & 0xF) - 1)
                shop.cur = (shop.stock[shop.pack] & 0xF) - 1;
            if (side != SHELF_SIDE(shop.cur))
                se_play(0x6266);
        }
    }
    if (pad->b.rep & 0x20) {
        side = SHELF_SIDE(shop.cur);
        if ((shop.stock[shop.pack] >> 4 & 0xF) > 0) {
            shop.cur = SHELF_IDX(shop.cur);
            if (SHELF_IDX(shop.cur) >= (shop.stock[shop.pack] >> 4 & 0xF) - 1)
                shop.cur = (shop.stock[shop.pack] >> 4 & 0xF) - 1;
            shop.cur |= 0x80;
            if (side != SHELF_SIDE(shop.cur))
                se_play(0x6266);
        }
    }
    shop_pack_msg_refresh();
}

/* the list of the cards that came out */
void shop_list_input(void)
{
    PadState *pad = pad_get();
    int cursor = shop.cursor;
    int top = shop.top;
    int n = shop.n_items;
    ShopItem *it = top + cursor < n ? shop_cur_item() : 0;
    unsigned int b;

    if (pad->b.rep & 0x10) {
        if (cursor > 0) {
            shop.cursor--;
            se_play(0x6212);
        } else if (top > 0) {
            shop.top--;
            se_play(0x6212);
        } else if (pad->b.trig & 0x10) {
            if (n < 5) {
                shop.top = 0;
                shop.cursor = 4;
            } else {
                shop.top = shop.n_items - 5;
                shop.cursor = shop.n_items - shop.top - 1;
            }
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x40) {
        if (cursor < 4) {
            shop.cursor++;
            se_play(0x6212);
        } else if (top < n - 5) {
            shop.top++;
            se_play(0x6212);
        } else if (pad->b.trig & 0x40) {
            shop.cursor = 0;
            shop.top = 0;
            se_play(0x6212);
        }
    } else if (pad->b.down & 0x100) {
        shop.state = 10;
        shop_list_page();
    } else if (pad->b.down & 0x200) {
        shop.state = 11;
        shop_list_mark();
    } else {
        shop.state = 9;
        b = pad->b.trig;
        if (b & region_pick_value(1)) {
            shop_dialog_open(3, 0);
            se_play(0x621A);
        } else if (b & region_pick_value(0)) {
            if (it) {
                shop.info = 1;
                shop.step = 0;
                se_play(0x6213);
            }
        } else if (b & 0x1000) {
            if (it)
                shop_card_detail(it);
            else
                shop_help_open();
            se_play(0x6216);
        }
    }
    shop_sel_update();
}

/* flight of the bought pack: offset from the shelf, control point, counter */
Vec4 shop_fly_off = { 0.0f, -3.0f, -8.0f, 1.0f };
Vec4 shop_fly_ctrl = { 0.0f, -3.44000006f, 5.0f, 1.0f };
Vec4 shop_fly_to = { 0.0f, -3.44000006f, 5.0f, 1.0f };
Vec4 shop_fly_end = { 0.0f, -3.44000006f, 5.0f, 1.0f };

#ifdef NON_MATCHING /* cálculo do voo do pack com templates Vec4 copiados pela pilha e índices do bitfield; forma e registradores diferentes */
/* buying the pack under the cursor: points, the pack flies to the counter */
void shop_buy_step(void)
{
    unsigned short side = SHELF_SIDE(shop.cur);
    Mat4 m;
    Vec4 v;
    float t;

    switch (shop.step) {
    case 0:
        if ((shop.stock[shop.pack] >> (side << 2) & 0xF) > 0) {
            shop_points_add(-shop_pack_price(shop.pack), 240, 136);
            shop.spent += shop_pack_price(shop.pack);
            se_play(0x6265);
            se_play(0x6267);
            shop_clerk_set(4);
            shop_clerk_voice(3);
            sysstate_set_byte_3e38(shop.pack);
            shop.step++;
        } else {
            shop.state = 3;
        }
        break;
    case 1:
        shop.lift[SHELF_SIDE(shop.cur)][SHELF_IDX(shop.cur)] += 2.5f;
        if (shop.lift[SHELF_SIDE(shop.cur)][SHELF_IDX(shop.cur)] > 12.0f) {
            shop.lift[SHELF_SIDE(shop.cur)][SHELF_IDX(shop.cur)] = 12.0f;
            shop.step++;
        }
        break;
    case 2: {
        Vec4 off;
        Vec4 c;
        unsigned char *s;
        int n;

        mat4_identity(&m);
        mat4_mul_rot_y(&m, &m, 3.1415927f * shop.rot / 180.0f);
        mat4_mul_rot_x(&m, &m, 0.7853982f);
        mat4_copy(&shop.rot0, &m);
        v.x = -3.0f + 6.0f * SHELF_SIDE(shop.cur);
        v.y = shop.lift[SHELF_SIDE(shop.cur)][SHELF_IDX(shop.cur)];
        v.w = 1.0f;
        v.z = -1.4f + 0.3f * SHELF_IDX(shop.cur);
        mat4_transform_vec4(&v, &m, &v);
        off = shop_fly_off;
        vec3_add_w(&shop.from, &off, &v);
        mat4_copy(&shop.rotm, &shop.rot0);
        vec4_copy(&shop.pos, &shop.from);
        shop.timer = 0;
        shop.taken = 1;
        c = shop_fly_ctrl;
        shop.ctrl.x = c.x;
        shop.ctrl.y = 2.0f + shop.from.y;
        shop.ctrl.z = c.z;
        shop.ctrl.w = 1.0f;
        shop.stock[shop.pack] -= (unsigned char)(1 << (side << 2));
        shop.bought[shop.pack]++;
        shop.lift[side][SHELF_IDX(shop.cur)] = 0.0f;
        s = &shop.stock[shop.pack];
        n = *s >> (side << 2) & 0xF;
        if (n > 0) {
            if (SHELF_IDX(shop.cur) >= n - 1) {
                shop.cur &= 0x80;
                shop.cur |= (unsigned char)((*s >> (side << 2) & 0xF) - 1);
            }
        } else {
            side = (side + 1) & 1;
            if ((*s >> (side << 2) & 0xF) > 0) {
                shop.cur = side << 7;
                shop.cur |= (unsigned char)((*s >> (side << 2) & 0xF) - 1);
            } else {
                shop.back = 1;
            }
        }
        shop.step++;
        break;
    }
    case 3:
        if (++shop.timer < 10) {
            Mat4 id;
            Vec4 q0, q1, q;
            Vec4 to;
            float u, a, b, c2;

            to = shop_fly_to;
            t = shop.timer / 10.0f;
            u = 1.0f - t;
            b = 2.0f * t * u;
            a = u * u;
            c2 = t * t;
            shop.pos.z = c2 * to.z + (a * shop.from.z + b * shop.ctrl.z);
            shop.pos.x = c2 * to.x + (a * shop.from.x + b * shop.ctrl.x);
            shop.pos.y = c2 * to.y + (a * shop.from.y + b * shop.ctrl.y);
            mat4_identity(&id);
            mat3_to_quat(&q0, &shop.rot0);
            mat3_to_quat(&q1, &id);
            quat_slerp(&q, &q0, &q1, t);
            quat_to_mat4(&shop.rotm, &q);
        } else {
            Vec4 to;
            to = shop_fly_end;
            vec4_copy(&shop.pos, &to);
            mat4_identity(&shop.rotm);
            shop.timer = 0;
            shop.step++;
        }
        break;
    case 4:
        if (shop.timer++ > 12) {
            shop.step++;
            shop.timer = 0;
        }
        break;
    case 5:
        shop.pos.x -= 2.5f;
        if (shop.pos.x < -12.0f)
            shop.step++;
        break;
    case 6:
        shop.taken = 0;
        shop.state = 3;
        shop.step = 0;
        break;
    }
}
#endif

/* the card info window over the list */
void shop_info_step(void)
{
    ShopItem *it;

    switch (shop.step) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        shop.step++;
    case 1:
        if (fade_is_done() == 0)
            return;
        it = shop_cur_item();
        cardinfo_open(it->id, shop_res_get(0x2028), shop_res_get(0x2029));
        cardinfo_set_lists(shop_res_get(0x2032), shop_str_get(0xC006), shop_str_get(0xC007));
        cardinfo_has_alt_pict();
        shop.info_open = 1;
        shop.step++;
        return;
    case 2:
        if (cardinfo_update() == 0)
            return;
        shop.info_open = 0;
        fade_start(1, 0xFF000000, 16);
        shop.step++;
    case 3:
        if (fade_is_done()) {
            shop.step = 0;
            shop.info = 0;
        }
        return;
    }
}

/* L/R: page through the list, up/down: jump between groups of the sort key */
void shop_list_page(void)
{
    PadState *pad = pad_get();
    unsigned int b = pad->b.rep;

    if (b & 0x8000) {
        if (shop.top > 5)
            shop.top -= 5;
        else
            shop.top = 0;
        se_play(0x6212);
    } else if (b & 0x2000) {
        if (shop.n_items < 6) {
            shop.top = 0;
        } else {
            int max;
            shop.top += 5;
            max = shop.n_items - 5;
            if (shop.top > max)
                shop.top = max;
        }
        se_play(0x6212);
    } else if (b & 0x1000) {
        shop_list_jump(1);
        se_play(0x6212);
    } else if (b & 0x4000) {
        shop_list_jump(0);
        se_play(0x6212);
    }
}

#ifdef NON_MATCHING /* o original carrega n_items antes de top/cursor e testa o índice mascarado e o inteiro em registradores diferentes */
/* marks/unmarks the card under the cursor */
void shop_list_mark(void)
{
    PadState *pad = pad_get();
    unsigned int b = pad->b.trig;
    unsigned int i;
    unsigned short id;

    if (b & region_pick_value(1)) {
        i = (unsigned short)(shop.top + shop.cursor);
        if (i < shop.n_items) {
            id = i < shop.n_items ? shop.items[shop.order[i]].id : 0;
            if (inv_get_flag7(id))
                inv_set_flag7(id, 0);
            else
                inv_set_flag7(id, 1);
            se_play(0x6213);
        }
    } else if (b & 0x8000) {
        shop_help_open();
        se_play(0x6216);
    }
}
#endif

void shop_list_jump(int back)
{
    int n = shop.n_items;
    int i;

    if (n != 0 && (i = shop_list_find_group(back)) >= 0) {
        if (i < 5) {
            shop.top = 0;
            shop.cursor = i;
        } else if (i > n - 5) {
            shop.top = n - 5;
            shop.cursor = i - shop.top;
        } else {
            shop.top = i;
            shop.cursor = 0;
        }
    }
}

#ifdef NON_MATCHING /* a chave de ordenação fica crua em s0 e é mascarada depois da primeira chamada; registradores e leiaute dos dois laços diferentes */
/* first line of the next (or the previous) group of the sort key */
int shop_list_find_group(int back)
{
    unsigned short key = shop.sort;
    int i = shop.top + shop.cursor;
    int n = shop.n_items;
    ShopItem *it;
    int v;
    int w;

    if (i >= n)
        return n - 1;
    it = shop_item_at(i);
    if (back) {
        if (i <= 0)
            return -1;
        i--;
        v = shop_item_key(shop_item_at(i), key);
        if (key - 8 < 2u && v > 0)
            v /= 100;
        for (i--; i >= 0; i--) {
            w = shop_item_key(shop_item_at(i), key);
            if (key - 8 < 2u && w > 0)
                w /= 100;
            if (w != v) {
                i++;
                break;
            }
        }
        if (i < 0)
            i = 0;
        return i;
    } else {
        v = shop_item_key(it, key);
        if (key - 8 < 2u && v > 0)
            v /= 100;
        for (i++; i < n; i++) {
            w = shop_item_key(shop_item_at(i), key);
            if (key - 8 < 2u && w > 0)
                w /= 100;
            if (w != v)
                break;
        }
        if (i < n)
            return i;
        return -1;
    }
}
#endif
