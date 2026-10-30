// @range 0x088117B8-0x08812C60
// @asm 0x0881189C-0x088121E0
// @asm 0x08812450-0x08812550
// @asm 0x0881259C-0x08812754
// @asm 0x088129D4-0x08812C60
/* Opening the bought packs one after the other: the pack model flies in and turns, it bursts, the
   cards fly out and turn over one by one (names drawn under them, gold for the rare ones), and the
   cards are added to the bought list (copies of the same card are counted) before the list opens. */
#include "libshop.h"

typedef struct ModelDrawOpts {  /* BOOT model_draw options, 0x34 */
    int layer[2];
    int pad[11];
} ModelDrawOpts;

typedef struct CardListCfg {    /* BOOT cardlist_init argument, 0x24 */
    void *gim_icons;
    void *gim_digits;
    void *ehf;
    void *file_tbl;
    void *spelltrap;
    int pad[2];
    void *cip;                  /* +0x1C */
    int pad20;
} CardListCfg;

extern Vec4 shop_open_v0, shop_open_v1, shop_open_v2, shop_open_v3;
extern Vec4 shop_open_v4, shop_open_v5, shop_open_v6;

void cardlist_init(CardListCfg *cfg);
void *malloc(unsigned int n);
void free(void *p);
int partner_count_get(void);
int partner_id_get(int i);
int inv_get_count(unsigned short id);
int inv_get_state(unsigned short id);
void mtx_get34(Mat4 *m);
void mtx_load(Mat4 *m);
void model_update(Model *m, float *dt);
void model_draw(Model *m, ModelDrawOpts *o);
void model_hook(Model *m, int which, void *fn);
Mat4 *mat4_translate(Mat4 *out, Mat4 *m, Vec4 *v);
Mat4 *mat4_mul_rot_y(Mat4 *out, Mat4 *m, float a);
void mat4_mul(Mat4 *r, Mat4 *a, Mat4 *b);
void vec4_copy(Vec4 *out, Vec4 *v);
void vec4_sub(Vec4 *out, Vec4 *a, Vec4 *b);
void vec4_scale(Vec4 *out, Vec4 *v, float s);
void vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
int cip_query(void *c, unsigned short id, int alt);
int inv_get_hi(unsigned short id);
unsigned short *card_get_name(unsigned short id);
int card_get_prop6_hi(unsigned short id);
unsigned short shop_pack_n_cards(unsigned short pack);
unsigned short shop_ui_sort_key(unsigned short id, unsigned short *tab);
void shop_pack_open(int pack, unsigned short *out);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
void *sceGeEdramGetAddr(void);
void shop_open_layout(void);
void shop_open_fly(Vec4 *out, Vec4 *a, Vec4 *b, Vec4 *c, float t);
int shop_open_cards_done(short speed);
int shop_card_tex_hook();
int shop_card_glow(int a, Mat4 *m, int rarity, int speed, void *vram);

void shop_open_next(void);
int shop_open_next_kind(int kind);
void shop_open_add(void);
int shop_items_grow(int n);
void shop_item_refresh(ShopItem *it);
void shop_open_draw_bg(void);
void shop_open_setup(void);
void shop_open_draw_pack(ShopOpenPack *p);
void shop_open_draw_card(ShopOpenCard *c);

void shop_open_start(void)
{
    CardListCfg cfg;

    shop_open = heap_alloc(shop.heap, sizeof(ShopOpen));
    memset(shop_open, 0, sizeof(ShopOpen));
    memset(&cfg, 0, sizeof(cfg));
    cfg.gim_icons = shop_res_get(0x2023);
    cfg.gim_digits = shop_res_get(0x2025);
    cfg.ehf = shop_ehf_get(0x4010);
    cfg.file_tbl = shop_res_get(0x2031);
    cfg.spelltrap = shop_res_get(0x2030);
    cfg.cip = shop.cip;
    cardlist_init(&cfg);
    memset(&shop.sel_id, 0, 0x1C);
    shop.sel_id = -1;
    shop_open_layout();
    shop_open->pack.kind = shop_open_next_kind(-1);
    fade_start(1, 0xFF000000, 16);
}

#ifdef NON_MATCHING /* 344 palavras: máquina de passos longa com templates Vec4 copiados pela pilha; alocação diferente. Chamadas conferidas com o asm (passo 0 chama shop_open_setup) */
/* the opening, step by step; 1 once START (skip) was pressed or all packs are open */
int shop_result_input(void)
{
    PadState *pad = pad_get();
    unsigned short speed = 1;
    int i;
    int k;
    int all;
    ShopOpen *o;

    if (pad->b.trig & region_pick_value(0))
        return 1;
    if (pad->b.down & region_pick_value(1))
        speed = 3;
    shop_open->speed = speed;
    switch (shop_open->step) {
    case 0:
        shop_open_next();
        shop_open_setup();
        shop_open->step++;
        shop_open->timer = 0;
        se_play(0x6268);
        return 0;
    case 1:
        shop_open->timer += speed;
        if (shop_open->timer >= 24) {
            Vec4 e = shop_open_v3;
            vec4_copy(&shop_open->pack.pos, &e);
            o = shop_open;
            o->pack.rot = 0.0f;
            for (i = 0; i < shop_open->n_cards; i++) {
                vec4_copy(&o->cards[i].pos, &shop_open->card_from);
                mat4_copy(&o->cards[i].rot, &shop_open->card_rot);
                o->cards[i].state = 0;
                o->cards[i].timer = 0;
            }
            shop_open->timer = 0;
            shop_open->step = 2;
            se_play(0x6269);
        } else {
            Vec4 a = shop_open_v0, b = shop_open_v1, c = shop_open_v2;
            shop_open_fly(&shop_open->pack.pos, &c, &b, &a, shop_open->timer / 24.0f);
            shop_open->pack.rot = shop_open->timer * 12.566371f / 24.0f;
        }
        break;
    case 2:
        shop_open->pack.speed = speed;
        break;
    case 3:
        all = 1;
        for (i = 0; i < shop_open->n_cards; i++) {
            unsigned short id = shop_open->cards[i].id;
            if (cip_query(shop.cip, id, inv_get_hi(id)))
                all = 0;
        }
        if (all) {
            shop_open->cards[0].state = 1;
            se_play(0x6267);
            shop_open->step = 4;
        }
        break;
    case 4:
        if (shop_open_cards_done(speed)) {
            shop_open->step = 5;
            shop_open->timer = 0;
        }
        break;
    case 5:
        shop_open->timer += speed;
        if (shop_open->timer >= 12) {
            if (shop_open->opened >= shop.bought[shop_open->pack.kind]) {
                k = shop_open_next_kind(shop_open->pack.kind);
                if (k < 0)
                    return 1;
                shop_open->pack.kind = k;
                shop_open->opened = 0;
                shop_model_set_box(0x8004, shop_open->pack.kind);
            }
            shop_open->step = 0;
            shop_open->timer = 0;
        } else {
            Vec4 d;
            Vec4 a = shop_open_v4, b = shop_open_v5, c;
            vec4_sub(&d, &b, &a);
            vec4_scale(&d, &d, shop_open->timer / 12.0f);
            c = shop_open_v6;
            vec3_add_w(&shop_open->pack.pos, &c, &d);
        }
        break;
    }
    return 0;
}

/* the 3D pack and cards, the names of the cards shown and the "skip" hint */
void shop_draw_open(void)
{
    unsigned int *list;
    int i;
    int w;
    int x;
    unsigned int color;
    unsigned short *name;
    unsigned short *hint;

    shop_gfx_open();
    mtx_set_mode(2);
    shop_open_draw_bg();
    shop_gfx_cards();
    mtx_set_mode(2);
    shop_open_draw_pack(&shop_open->pack);
    for (i = 0; i < shop_open->n_cards; i++) {
        if (shop_open->cards[i].state != 0 && shop_open->cards[i].state != 5)
            shop_open_draw_card(&shop_open->cards[i]);
    }
    list = dl_select_layer(4, 0);
    list = shop_dl_2d(list);
    font_text_set(list);
    font_set_flag4(0);
    for (i = 0; i < shop_open->n_cards; i++) {
        ShopOpenCard *c = &shop_open->cards[i];
        name = card_get_name(c->id);
        font_set_size(18, 18);
        switch (c->state) {
        case 3:
            w = font_measure_text(name);
            x = (0x7800 - w) / 2;
            break;
        case 4:
            w = font_measure_text(name);
            x = (0x7800 - w) / 2;
            x -= (x + w) * c->timer / 14;
            break;
        case 2:
            w = font_measure_text(name);
            x = 0x7800 - (0x7800 - (0x7800 - w) / 2) * c->timer / 16;
            break;
        default:
            continue;
        }
        color = 0xFFFFFFFF;
        font_set_color(0xFF000000);
        switch (card_get_prop6_hi(c->id)) {
        case 4:
        case 3:
            font_set_color(0xFF008080);
            color = 0xFF00FFFF;
            break;
        }
        font_draw_wtext(x & ~0x3F, 0x3840, w << 6, name);
        font_draw_wtext(((x >> 6) + 2) << 6, 0x3840, w << 6, name);
        font_draw_wtext(((x >> 6) + 1) << 6, 0x3800, w << 6, name);
        font_draw_wtext(((x >> 6) + 1) << 6, 0x3880, w << 6, name);
        font_set_color(color);
        font_draw_wtext(((x >> 6) + 1) << 6, 0x3840, w << 6, name);
    }
    font_set_flag4(1);
    font_set_color(0xFFFFFFFF);
    font_set_size(14, 14);
    hint = table_get_entry(shop_str_get(0xC001), 0x1B);
    w = font_measure_text(hint);
    font_draw_text(0x7700 - (w + 0x40), 0x4300 - font_line_height(),
                   w + 0x40, table_get_entry(shop_str_get(0xC001), 0x1B));
    dl_set_list(font_text_clear());
}
#endif

/* opens the packs left (skip), builds the display order of the bought list, goes to the list */
void shop_result_update(void)
{
    unsigned short *kind = &shop_open->pack.kind;
    unsigned short n;
    int i;

    if (*kind < 48) {
        n = shop.bought[*kind];
        if (shop_open->opened < n) {
            while (shop_open->opened < n)
                shop_open_next();
            shop_open->opened = 0;
            (*kind)++;
        }
    }
    while (*kind < 48) {
        n = shop.bought[*kind];
        while (shop_open->opened < n)
            shop_open_next();
        shop_open->opened = 0;
        (*kind)++;
    }
    if (shop_open) {
        heap_set_level(shop.heap, shop_open);
        shop_open = 0;
    }
    shop.order = heap_alloc(shop.heap, shop.n_items * 2);
    shop.n_owned = 0;
    for (i = 0; i < shop.n_items; i++) {
        shop.order[i] = i;
        shop.n_owned += shop_item_at(i)->count;
    }
    shop_sel_update();
    shop.sort = 1;
    shop.order_dir = 0;
    for (i = 0; i < shop.n_items; i++)
        card_get_prop6_hi(shop.items[i].id);
    shop.state = 9;
}

#ifdef NON_MATCHING /* mesma forma; o contador, o deslocamento da carta e o ponteiro dos ids ficam em s4/s3/s2 no original */
/* opens one pack of the current kind */
void shop_open_next(void)
{
    unsigned short ids[10];
    unsigned short *p;
    int i;

    shop_pack_open(shop_open->pack.kind, ids);
    shop_open->n_cards = shop_pack_n_cards(shop_open->pack.kind);
    for (i = 0, p = ids; i < shop_open->n_cards; i++, p++) {
        ShopOpenCard *c = &shop_open->cards[i];
        unsigned short *r = &c->id;
        r[0] = *p;
        r[1] = shop_ui_sort_key(*p, shop_str_get(0xC003));
        r[2] = shop_open->pack.kind;
        r[3] = 1;
        c->state = 0;
    }
    shop_open->opened++;
    shop_open_add();
}
#endif

/* next kind with packs bought after `kind`, -1 if none */
int shop_open_next_kind(int kind)
{
    int k;

    for (k = kind + 1; k < 48; k++) {
        if (shop.bought[k] > 0)
            return k;
    }
    return -1;
}

#ifdef NON_MATCHING /* busca para trás na lista e inserção com laços aninhados; alocação diferente */
/* adds the cards of the opened pack to the bought list */
void shop_open_add(void)
{
    unsigned short n = 0;
    unsigned short kind = shop_open->pack.kind;
    unsigned short idx[10];
    int i;
    int k;
    int found;

    for (i = 0; i < shop_open->n_cards; i++) {
        found = 0;
        for (k = shop.n_items - 1; k >= 0 && kind == shop.items[k].pack; k--) {
            if (shop_open->cards[i].id == shop.items[k].id) {
                found = 1;
                if (shop.items[k].count < 100)
                    shop.items[k].count++;
                break;
            }
        }
        if (!found)
            idx[n++] = i;
    }
    if (n != 0 && shop_items_grow(n)) {
        for (i = 0; i < n; i++) {
            ShopItem *it = &shop.items[shop.n_items++];
            it->id = shop_open->cards[idx[i]].id;
            it->key = shop_open->cards[idx[i]].key;
            it->pack = kind;
            it->count = 1;
            shop_item_refresh(it);
        }
    }
}
#endif

/* room for n more lines in the bought list */
int shop_items_grow(int n)
{
    void *tmp;
    int from_heap = 1;

    tmp = malloc(shop.n_items * sizeof(ShopItem));
    if (tmp == 0) {
        from_heap = 0;
        tmp = heap_alloc(shop.heap, shop.n_items * sizeof(ShopItem));
        if (tmp == 0)
            return 0;
    }
    memcpy(tmp, shop.items, shop.n_items * sizeof(ShopItem));
    if (shop.items)
        heap_set_level(shop.heap, shop.items);
    shop.items = heap_alloc(shop.heap, (shop.n_items + n) * sizeof(ShopItem));
    memcpy(shop.items, tmp, shop.n_items * sizeof(ShopItem));
    if (from_heap) {
        free(tmp);
        return 1;
    }
    heap_set_level(shop.heap, tmp);
    return 1;
}

/* partner decks holding the card and copies owned */
void shop_item_refresh(ShopItem *it)
{
    unsigned short id = it->id;
    int i;

    it->partners = 0;
    for (i = 0; i < partner_count_get(); i++) {
        if (id == (unsigned short)partner_id_get(i))
            it->partners++;
    }
    it->owned = inv_get_count(id);
}

/* the duel field behind the opening */
void shop_open_draw_bg(void)
{
    ModelDrawOpts o;
    Model *m;

    memset(&o, 0, sizeof(o));
    o.layer[0] = 2;
    o.layer[1] = 2;
    mtx_identity();
    m = shop_model_get(0x8005);
    model_update(m, 0);
    model_draw(m, &o);
}

#ifdef NON_MATCHING /* matriz e vetores na pilha em outra ordem */
/* the pack turning in front of the camera; it bursts once its animation reaches frame 40 */
void shop_open_draw_pack(ShopOpenPack *p)
{
    Vec4 v;
    Mat4 m;
    float dt[4];
    ModelDrawOpts o;
    Model *mdl;

    memset(&o, 0, sizeof(o));
    o.layer[0] = 2;
    o.layer[1] = 2;
    mtx_identity();
    mdl = shop_model_get(0x8004);
    memset(dt, 0, sizeof(dt));
    dt[0] = p->speed;
    mtx_get34(&m);
    v.y = -6.0f;
    v.w = 1.0f;
    v.x = 0.0f;
    v.z = 0.0f;
    mat4_translate(&m, &m, &v);
    mat4_mul_rot_y(&m, &m, p->rot);
    v.y *= -1.0f;
    mat4_translate(&m, &m, &v);
    vec4_copy(&v, &p->pos);
    mat4_translate(&m, &m, &v);
    mtx_load(&m);
    model_update(mdl, dt);
    model_draw(mdl, &o);
    if (dt[0] > 0.0f && mdl->frame >= 40.0f) {
        p->speed = 0.0f;
        shop_open->step = 3;
    }
}

/* one card: model with the card picture as texture, and its glow once shown */
void shop_open_draw_card(ShopOpenCard *c)
{
    Vec4 v;
    Mat4 m;
    ModelDrawOpts o;
    Model *mdl;

    memset(&o, 0, sizeof(o));
    o.layer[0] = 2;
    o.layer[1] = 2;
    mtx_identity();
    mdl = shop_model_get(0x8003);
    shop_open->cur = &c->id;
    model_hook(mdl, 0, shop_card_tex_hook);
    mtx_get34(&m);
    mat4_mul(&m, &m, &c->rot);
    vec4_copy(&v, &c->pos);
    mat4_translate(&m, &m, &v);
    mtx_load(&m);
    model_update(mdl, 0);
    model_draw(mdl, &o);
    if (c->state == 3 && card_get_prop6_hi(c->id) != 0) {
        if (shop_card_glow(c->timer, &m, card_get_prop6_hi(c->id), shop_open->speed,
                           (char *)sceGeEdramGetAddr() + 0x160000)) {
            c->state++;
            c->timer = 0;
        }
    }
}
#endif
