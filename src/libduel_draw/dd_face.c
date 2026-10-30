// @range 0x0881299C-0x0881351C
// @data 0x08875104-0x0887541C
// @asm 0x0881299C-0x0881351C
/* Portraits of the duelists on the LP panel: the frame, the cut-in face (kao.gim/karada.gim of
   the character's cut-in pack) or the icon of the character, the blinking turn marker, the
   pad button icon; the sprite table of duel_thing01.gim used by the panel. */
#include "dd_d1.h"

void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_tex_func(void *list, int func, int comp);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
void *dl_scissor(void *list, int x, int y, int w, int h);
unsigned int *gfx_get_state(void);
unsigned char *syspack_get(void);
int dd_input_player(void);
int dd_view_tag_get(void);
int dd_cobjs_map_get(int side);
int dd_cobjs_map_other(int side);
void *dd_cutin_get(int id, int sub);
int func_0884DC44(int chr);
int func_0884DC64(int chr);
void *func_0884DB7C(int chr);
void *func_08867EE0(void *list, unsigned short chr, int x, int y, int scale, void *kao, void *karada);
float func_0886B08C(int side);
float func_0886B1B8(int side);

typedef struct DdInt8 {
    int v[8];
} DdInt8;

DdInt8 dd_face_pos = { { 0x17, 0x17, 0x3C, 0xD, 0x32, 0x17, 0xD, 0xD } };
char dd_face_kao[] = "kao.gim";
char dd_face_karada[] = "karada.gim";

void *dd_face(float k, void *list, int x, int y, int side, int chr, int turn);
void *dd_face_frame(void *list, int x, int y, int w, int h, int side);
void *dd_face_blink(void *list, int side);
void *dd_button_icon(void *list, int x, int y);

#ifdef NON_MATCHING /* funções de desenho; forma não reproduzida */
/* portrait(s) of side `side` at x, y (two in a tag duel, sliding when they swap) */
void *dd_face_panel(void *list, int x, int y, int side)
{
    DdInt8 pos = dd_face_pos;
    int turn;
    int chr[2];
    float k[2];
    int off[4];
    unsigned int front[2];
    float sw;
    int i;
    int f;

    if (side == 0)
        turn = dd_input_player() == 0;
    else
        turn = dd_input_player();
    if (dd_view_tag_get() == 0)
        return dd_face(0.0f, list, x + pos.v[side * 4 + 0], y + pos.v[side * 4 + 1], side,
                       dd_cobjs_map_get(side), turn);
    sw = func_0886B1B8(side);
    front[1] = sw < 0.5f;
    front[0] = sw >= 0.5f;
    chr[0] = dd_cobjs_map_get(side);
    chr[1] = dd_cobjs_map_other(side);
    k[0] = func_0886B08C(side) / 2.0f;
    k[1] = 1.0f - k[0];
    off[2] = (int)(k[0] * (float)pos.v[side * 4 + 2] + k[1] * (float)pos.v[side * 4 + 0] + 0.999f);
    off[3] = (int)(k[1] * (float)pos.v[side * 4 + 2] + k[0] * (float)pos.v[side * 4 + 0] + 0.999f);
    off[0] = (int)(k[0] * (float)pos.v[side * 4 + 3] + k[1] * (float)pos.v[side * 4 + 1] + 0.999f);
    off[1] = (int)(k[1] * (float)pos.v[side * 4 + 3] + k[0] * (float)pos.v[side * 4 + 1] + 0.999f);
    for (i = 0; i < 2; i++) {
        if (front[i] == 0 || dd_view_tag_get()) {
            f = front[i];
            list = dd_face(k[f], list, x + off[f + 2], y + off[f], side, chr[f],
                           i == 0 && turn && sw == 0.0f);
        }
    }
    return list;
}

/* one portrait of size 27..47 (k: 0 big, 1 small) */
void *dd_face(float k, void *list, int x, int y, int side, int chr, int turn)
{
    int id = func_0884DC44(chr);
    int sub = func_0884DC64(chr);
    int size = (int)((1.0f - k) * 47.0f + k * 27.0f + 0.999f);
    int big = (int)((float)size * 1.1f);
    int x0 = x - size / 2;
    int y0 = y - size / 2;
    void *gim = dd_gim_get(0x4001F);
    void *pack;
    void *kao;
    void *karada;

    if (gim)
        list = gim_set_texture(list, gim);
    list = dd_face_frame(list, x0, y0, size, size, side);
    if (charmap_lookup(id, 0) < 0) {
        gim = func_0884DB7C(chr);
        if (gim) {
            list = gim_set_texture(list, gim);
            list = draw_set_offsets(list, 0x28, 0x28, size - 7, size - 7);
            list = draw_sprite(list, x0 + 3, y0 + 3, 0, 0, 0, 0, 0);
        }
    } else {
        pack = dd_cutin_get(id, sub);
        kao = ehp_find(pack, dd_face_kao, 0);
        karada = ehp_find(pack, dd_face_karada, 0);
        list = dl_scissor(list, x0 + 3, y0 + 3, size - 7, size - 7);
        list = dl_enable(list, 2);
        list = func_08867EE0(list, id, x - big / 2, y - big / 2 - 2, big * 100 / 51, kao, karada);
        list = dl_disable(list, 2);
    }
    if (turn)
        list = dd_face_blink(list, side);
    return list;
}

/* frame of a portrait: 9 slices of duel_bup_frame01.gim (column of `side`) */
void *dd_face_frame(void *list, int x, int y, int w, int h, int side)
{
    int u = side * 0x30;
    int xr = x + w - 4;
    int yb = y + h - 4;

    list = draw_set_offsets(list, 0x28, 0x28, w - 7, h - 7);
    list = draw_sprite(list, x + 3, y + 3, 0, u + 3, 3, 0, 0);
    list = draw_set_offsets(list, 3, 3, 3, 3);
    list = draw_sprite(list, x, y, 0, u, 0, 0, 0);
    list = draw_set_offsets(list, 4, 3, 4, 3);
    list = draw_sprite(list, xr, y, 0, u + 0x2B, 0, 0, 0);
    list = draw_set_offsets(list, 4, 4, 4, 4);
    list = draw_sprite(list, xr, yb, 0, u + 0x2B, 0x2B, 0, 0);
    list = draw_set_offsets(list, 4, 4, 4, 4);
    list = draw_sprite(list, x, yb, 0, u, 0x2B, 0, 0);
    list = draw_set_offsets(list, 1, 3, w - 7, 3);
    list = draw_sprite(list, x + 3, y, 0, u + 3, 0, 0, 0);
    list = draw_set_offsets(list, 1, 4, w - 7, 4);
    list = draw_sprite(list, x + 3, yb, 0, u + 3, 0x2B, 0, 0);
    list = draw_set_offsets(list, 3, 1, 3, h - 7);
    list = draw_sprite(list, x, y + 3, 0, u, 3, 0, 0);
    list = draw_set_offsets(list, 4, 1, 4, h - 7);
    return draw_sprite(list, xr, y + 3, 0, u + 0x2B, 3, 0, 0);
}

/* blinking glow over the portrait of the player on turn */
void *dd_face_blink(void *list, int side)
{
    unsigned int f = *gfx_get_state();
    int x = side == 0 ? 0x8A : 0x1A9;
    int a = 20 - f % 40;
    void *gim = dd_gim_get(0x4001F);

    if (gim)
        list = gim_set_texture(list, gim);
    list = dl_enable(list, 4);
    list = dl_disable(list, 1);
    list = dl_blend_func(list, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dl_ambient_color(list, (a * a * 200 / 400) << 24 | 0xFFFFFF);
    list = dl_tex_func(list, 0, 1);
    list = draw_set_offsets(list, 0x20, 0x20, 0x40, 0x40);
    list = draw_sprite(list, x, 0x12, 0, 0x60, side << 5, 0, 0);
    list = dl_enable(list, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}
#endif

#ifdef NON_MATCHING /* forma não reproduzida */
/* pad icon by the portrait of the player who acts */
void *dd_turn_icon(void *list)
{
    int x;

    if (dd_input_player() == 0)
        x = 0x94;
    else
        x = 0x1CD;
    return dd_button_icon(list, x, 0x4A);
}
#endif

typedef struct DdIconUv {
    int u;
    int v;
} DdIconUv;

DdIconUv dd_button_uv[4] = { { 0x0, 0x54 }, { 0x12, 0x54 }, { 0x0, 0x6A }, { 0x12, 0x6A } };
char dd_button_gim[] = "buttom_icon1.gim";

#ifdef NON_MATCHING /* forma não reproduzida */
/* animated pad button (4 frames) from the system pack */
void *dd_button_icon(void *list, int x, int y)
{
    void *gim = 0;
    unsigned char *pack = syspack_get();
    unsigned int f;

    if (pack)
        gim = ehp_find(pack, dd_button_gim, 0);
    if (gim == 0)
        return list;
    list = gim_set_texture(list, gim);
    f = *gfx_get_state();
    list = dl_enable(list, 9);
    list = dl_disable(list, 1);
    list = dl_disable(list, 4);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = draw_set_offsets(list, 0x12, 0x16, 0x12, 0x16);
    f = f / 30 & 3;
    list = draw_sprite(list, x, y, 0, dd_button_uv[f].u, dd_button_uv[f].v, 0, 0);
    list = dl_enable(list, 4);
    return dl_enable(list, 1);
}
#endif

DdUv dd_uv[86] = {
    { 0x0, 0x0, 0x7, 0x9 }, { 0x7, 0x0, 0x7, 0x9 }, { 0xE, 0x0, 0x7, 0x9 }, { 0x15, 0x0, 0x7, 0x9 },
    { 0x1C, 0x0, 0x7, 0x9 }, { 0x23, 0x0, 0x7, 0x9 }, { 0x2A, 0x0, 0x7, 0x9 }, { 0x31, 0x0, 0x7, 0x9 },
    { 0x38, 0x0, 0x7, 0x9 }, { 0x3F, 0x0, 0x7, 0x9 }, { 0x46, 0x0, 0x7, 0x9 }, { 0x4D, 0x0, 0x7, 0x9 },
    { 0x54, 0x0, 0x7, 0x9 }, { 0x0, 0x9, 0x7, 0x9 }, { 0x7, 0x9, 0x7, 0x9 }, { 0xE, 0x9, 0x7, 0x9 },
    { 0x15, 0x9, 0x7, 0x9 }, { 0x1C, 0x9, 0x7, 0x9 }, { 0x23, 0x9, 0x7, 0x9 }, { 0x2A, 0x9, 0x7, 0x9 },
    { 0x31, 0x9, 0x7, 0x9 }, { 0x38, 0x9, 0x7, 0x9 }, { 0x3F, 0x9, 0x7, 0x9 }, { 0x46, 0x9, 0x7, 0x9 },
    { 0x4D, 0x9, 0x7, 0x9 }, { 0x54, 0x9, 0x7, 0x9 }, { 0x5B, 0x9, 0x7, 0x9 }, { 0x0, 0x12, 0x9, 0x8 },
    { 0x9, 0x12, 0x9, 0x8 }, { 0x12, 0x12, 0x9, 0x8 }, { 0x1B, 0x12, 0x9, 0x8 }, { 0x24, 0x12, 0x9, 0x8 },
    { 0x2D, 0x12, 0x9, 0x8 }, { 0x36, 0x12, 0x1B, 0x8 }, { 0x0, 0x1A, 0x9, 0xC }, { 0x9, 0x1A, 0x9, 0xC },
    { 0x12, 0x1A, 0x9, 0xC }, { 0x1B, 0x1A, 0x9, 0xC }, { 0x24, 0x1A, 0x9, 0xC }, { 0x2D, 0x1A, 0x9, 0xC },
    { 0x36, 0x1A, 0x9, 0xC }, { 0x3F, 0x1A, 0x9, 0xC }, { 0x48, 0x1A, 0x9, 0xC }, { 0x51, 0x1A, 0x9, 0xC },
    { 0x5A, 0x1A, 0x12, 0xC }, { 0x0, 0x26, 0x7, 0x8 }, { 0x7, 0x26, 0x7, 0x8 }, { 0xE, 0x26, 0x7, 0x8 },
    { 0x15, 0x26, 0x7, 0x8 }, { 0x1C, 0x26, 0x7, 0x8 }, { 0x23, 0x26, 0x7, 0x8 }, { 0x2A, 0x26, 0x7, 0x8 },
    { 0x31, 0x26, 0x7, 0x8 }, { 0x38, 0x26, 0x7, 0x8 }, { 0x3F, 0x26, 0x7, 0x8 }, { 0x46, 0x26, 0x1C, 0x8 },
    { 0x0, 0x2E, 0x13, 0xB }, { 0x13, 0x2E, 0x13, 0xB }, { 0x26, 0x2E, 0x13, 0xB }, { 0x39, 0x2E, 0x13, 0xB },
    { 0x4C, 0x2E, 0x13, 0xB }, { 0x5F, 0x2E, 0x13, 0xB }, { 0x0, 0x39, 0x13, 0xB }, { 0x13, 0x39, 0x13, 0xB },
    { 0x26, 0x39, 0x13, 0xB }, { 0x39, 0x39, 0x13, 0xB }, { 0x4C, 0x39, 0x13, 0xB }, { 0x5F, 0x39, 0x13, 0xB },
    { 0x0, 0x43, 0x10, 0x10 }, { 0x10, 0x43, 0x10, 0x10 }, { 0x20, 0x43, 0x10, 0x10 }, { 0x30, 0x43, 0x10, 0x10 },
    { 0x40, 0x43, 0x10, 0x10 }, { 0x50, 0x43, 0x10, 0x10 }, { 0x60, 0x43, 0x10, 0x10 }, { 0x70, 0x43, 0x10, 0x10 },
    { 0x0, 0x53, 0x9, 0x9 }, { 0x9, 0x53, 0x9, 0x9 }, { 0x12, 0x53, 0x9, 0x9 }, { 0x1B, 0x53, 0x9, 0x9 },
    { 0x24, 0x53, 0x9, 0x9 }, { 0x2D, 0x53, 0xB, 0xB }, { 0x38, 0x53, 0xB, 0xB }, { 0x43, 0x53, 0xB, 0xB },
    { 0x5A, 0x1A, 0x9, 0xC }, { 0x63, 0x1A, 0x9, 0xC },
};
