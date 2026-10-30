// @range 0x08812C60-0x08813808
// @asm 0x08812D64-0x08812F78
// @asm 0x08813400-0x08813748
// @data 0x08820410-0x08820480
/* Animation of the opened pack: its model placed and the card pictures requested, the cards moving
   through their steps (in the pack, rising, turning to the camera, shown, leaving) along curves, the
   card picture rendered into VRAM as the texture of the card model, and the curve points. */
#include "libshop.h"

void model_update(Model *m, float *dt);
Vec4 *vec4_copy(Vec4 *out, Vec4 *v);
Vec4 *vec4_set(Vec4 *out, float x, float y, float z, float w);
Vec4 *vec4_sub(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec4_scale(Vec4 *out, Vec4 *a, float s);
Vec4 *vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
Mat4 *mat4_copy(Mat4 *out, Mat4 *m);
Mat4 *mat4_identity(Mat4 *out);
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a);
Mat4 *mat4_transpose(Mat4 *out, Mat4 *m);
void mtx_get34(Mat4 *m);
void *cip_read_card(void *h, int id, int variant);
int inv_get_hi(unsigned short id);
int card_get_prop6_hi(unsigned short id);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_tex_offset(float u, float v, unsigned int *list);
unsigned int *dl_tex_scale(float u, float v, unsigned int *list);
unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
void *cardlist_draw_quad(void *list, int x, int y);
unsigned int *card_draw(void *list, int x, int y, unsigned int id);
void *sceGeEdramGetAddr(void);

Vec4 *shop_open_fly(Vec4 *out, Vec4 *a, Vec4 *b, Vec4 *c, float t);
void shop_open_card_move(ShopOpenCard *c, int i);

Vec4 shop_pack_start = { 0.0f, -20.0f, -70.0f, 1.0f };

/* frames of each card step */
unsigned short shop_card_steps[8] = { 0, 12, 16, 24, 14, 0, 0, 0 };

/* places the pack of the kind being opened and starts reading the pictures of its cards */
void shop_open_setup(void)
{
    Model *m = shop_model_get(0x8004);
    int i;

    shop_model_set_box(0x8004, shop_open->pack.kind);
    m->frame = 0.0f;
    {
        Vec4 v = shop_pack_start;
        vec4_copy(&shop_open->pack.pos, &v);
    }
    shop_open->pack.rot = 0.0f;
    shop_open->pack.speed = 0.0f;
    for (i = 0; i < shop_open->n_cards; i++) {
        unsigned short id = shop_open->cards[i].id;
        cip_read_card(shop.cip, id, inv_get_hi(id));
    }
}

#ifdef NON_MATCHING /* o original recalcula o ponteiro da carta e testa a última; alocação diferente */
/* advances the cards; the next one starts when the previous is shown; 1 once the last one left */
int shop_open_cards_done(short speed)
{
    int i;
    unsigned int n = 0;
    ShopOpenCard *c;
    unsigned short st;
    unsigned short len;

    for (i = 0; i < shop_open->n_cards; i++, n = shop_open->n_cards) {
        c = &shop_open->cards[i];
        st = c->state;
        len = shop_card_steps[st];
        if (st == 3) {
            if (card_get_prop6_hi(c->id))
                len = 0;
        } else if (st == 5 || st == 0) {
            continue;
        }
        c->timer += speed;
        if (len != 0 && c->timer >= len) {
            c->state++;
            c->timer = 0;
        }
        if (st == 2 && c->state == 3) {
            switch (card_get_prop6_hi(c->id)) {
            case 4:
                se_play(0x626A);
                break;
            case 3:
                se_play(0x626B);
                break;
            case 1:
                se_play(0x626C);
                break;
            }
        }
        if (i < shop_open->n_cards - 1 && c->state > 3 && shop_open->cards[i + 1].state == 0) {
            shop_open->cards[i + 1].state = 1;
            se_play(0x6267);
            shop_open->cards[i + 1].timer = 0;
        }
        shop_open_card_move(c, i);
    }
    return shop_open->cards[n - 1].state == 5;
}
#endif

/* position and rotation of a card for its step */
void shop_open_card_move(ShopOpenCard *c, int i)
{
    unsigned short len = shop_card_steps[c->state];
    Vec4 *ctrl = 0;
    float t;
    Vec4 a, b, d;

    if (len == 0)
        t = 0.0f;
    else if (c->timer > len)
        t = 1.0f;
    else
        t = (float)c->timer / (float)len;
    switch (c->state) {
    case 0:
        mat4_copy(&c->rot, &shop_open->card_rot);
        vec4_copy(&a, &shop_open->card_from);
        vec4_copy(&b, &a);
        t = 0.0f;
        break;
    case 1:
        mat4_copy(&c->rot, &shop_open->card_rot);
        vec4_copy(&a, &shop_open->card_from);
        vec4_copy(&b, &shop_open->card_up);
        break;
    case 2:
        mat4_mul_rot_y(&c->rot, &shop_open->card_face, t * 6.2831855f);
        vec4_copy(&a, &shop_open->card_up);
        vec4_copy(&b, &shop_open->card_show);
        ctrl = &shop_open->card_ctrl2;
        break;
    case 3:
        mat4_copy(&c->rot, &shop_open->card_face);
        vec4_copy(&a, &shop_open->card_show);
        vec4_copy(&b, &a);
        t = 0.0f;
        break;
    case 4:
        mat4_mul_rot_y(&c->rot, &shop_open->card_face, t * 6.2831855f);
        vec4_copy(&a, &shop_open->card_show);
        vec4_copy(&b, &shop_open->card_out);
        ctrl = &shop_open->card_ctrl3;
        break;
    default:
        return;
    }
    if (ctrl == 0) {
        vec4_sub(&d, &b, &a);
        vec4_scale(&d, &d, t);
        vec3_add_w(&c->pos, &a, &d);
    } else {
        shop_open_fly(&c->pos, &a, &b, ctrl, t);
    }
}

/* texture hook of the card model: renders the card (back or picture) into VRAM and uses it */
int shop_card_tex_hook(unsigned int **plist, int a1, int which)
{
    unsigned int *list;

    if (which != 0) {
        if (which != 1)
            return 0;
        list = *plist;
    } else {
        list = *plist;
    }
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 3, 1);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_tex_flush(list);
    list = gfx_set_draw_target(list, 0x160000, -1, 0x80, 0x100, 3);
    if (which == 0)
        list = cardlist_draw_quad(list, 0, 0);
    else
        list = card_draw(list, 0, 0, *shop_open->cur);
    list = gfx_set_default_target(list);
    list = dl_tex_flush(list);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x80, 0x100, 0x80, (char *)sceGeEdramGetAddr() + 0x160000);
    list = dl_tex_func(list, 3, 0);
    list = dl_tex_wrap(list, 1, 1);
    list = dl_tex_offset(0.0f, 0.6015625f, list);
    list = dl_tex_scale(0.84375f, -0.6015625f, list);
    *plist = dl_enable(list, 1);
    return 1;
}

Vec4 shop_card_from = { -9.0f, -1.0f, -5.0f, 1.0f };
Vec4 shop_card_up = { -9.0f, -1.0f, -5.0f, 1.0f };
Vec4 shop_card_mid = { -9.0f, -1.0f, -5.0f, 1.0f };
Vec4 shop_card_mid_off = { 2.0f, 7.0f, 7.0f, 1.0f };
Vec4 shop_card_v500 = { 0.0f, 0.0f, 0.0f, 1.0f };

#ifdef NON_MATCHING /* os temporários Vec4 ficam 16 bytes mais acima no quadro do original e os ponteiros em s0 são recalculados */
/* curve points of the cards, in the space of the card camera */
void shop_open_layout(void)
{
    Vec4 v0, v1, v2, v3;
    Vec4 *p;
    Mat4 *m;

    shop_gfx_cards();
    m = &shop_open->card_rot;
    mat4_identity(m);
    mat4_mul_rot_y(m, m, 3.1415927f);
    p = &shop_open->card_from;
    v0 = shop_card_from;
    vec4_copy(p, &v0);
    p->y += 6.0f;
    p = &shop_open->card_up;
    v1 = shop_card_up;
    vec4_copy(p, &v1);
    p->y += 18.0f;
    p = &shop_open->card_mid;
    v2 = shop_card_mid;
    vec4_copy(p, &v2);
    v3 = shop_card_mid_off;
    vec3_add_w(p, p, &v3);
    m = &shop_open->card_face;
    mtx_set_mode(1);
    mtx_get34(m);
    *(Vec4 *)&m->m[12] = shop_card_v500;
    mat4_mul_rot_y(m, m, 3.1415927f);
    mat4_transpose(m, m);
    vec4_set(&shop_open->card_show, -0.2f, 11.3f, 4.6f, 1.0f);
    vec4_set(&shop_open->card_out, 24.0f, -2.0f, 2.0f, 1.0f);
    vec4_set(&shop_open->card_ctrl1, shop_open->card_mid.x + 2.0f, shop_open->card_up.y + 6.0f,
             shop_open->card_up.z + 2.0f, 1.0f);
    vec4_set(&shop_open->card_ctrl2, shop_open->card_show.x - 1.0f, shop_open->card_show.y + 10.0f,
             shop_open->card_up.z + 4.0f, 1.0f);
    vec4_set(&shop_open->card_ctrl3, shop_open->card_out.x - 10.0f, shop_open->card_show.y + 5.0f,
             shop_open->card_out.z + 1.0f, 1.0f);
}
#endif

/* quadratic bezier a -> b with control point c */
Vec4 *shop_open_fly(Vec4 *out, Vec4 *a, Vec4 *b, Vec4 *c, float t)
{
    float u = 1.0f - t;
    Vec4 s, p, q, r;

    vec4_scale(&p, a, u * u);
    vec4_scale(&q, c, 2.0f * t * u);
    vec4_scale(&r, b, t * t);
    vec3_add_w(&s, &p, &q);
    vec3_add_w(&s, &s, &r);
    vec4_copy(out, &s);
    return out;
}
