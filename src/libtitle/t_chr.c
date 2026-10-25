// @range 0x0882647C-0x08826B8C
// @asm 0x088266E8-0x08826AB4
// @data 0x08830230-0x088306C8
/* The last helpers of the title: numbers drawn with the font or with digit sprites, a character's
   portrait (face over body, scaled) and the character table of the face sprites (the same code as
   libduel_mgr's dmgr_partner.c). */
#include "libtitle.h"

int charmap_lookup(unsigned short c, int a);
void *gim_set_texture_sub(void *list, void *g, int a, int b, int c, int d);

/* n right-aligned at x (digits from the right, step apart); leading zeros when zeros (as liblimitlist) */
void *tnum_draw(void *list, short x, short y, int n, int step, unsigned short digits, int zeros)
{
    short txt[2] = { '0', 0 };
    int px = (unsigned short)x;
    int i;

    font_text_set(list);
    for (i = digits - 1; i >= 0; i--) {
        px = (unsigned short)(px - step);
        txt[0] = n % 10 + '0';
        font_draw_text(px << 6, y << 6, (480 - px) << 6, (unsigned short *)txt);
        n /= 10;
        if (!zeros && n == 0)
            break;
    }
    return font_text_clear();
}

/* n with the small digit sprites of title gim 8, right-aligned over `digits` places */
void *tnum_draw_spr(void *list, short x, short y, unsigned int n, short digits)
{
    int i;
    int d;

    list = tmenu_gfx_setup(list, 0);
    list = gim_set_texture(list, ehp_find(title_pack(), title_gim_name(8), 0));
    list = draw_set_offsets(list, 7, 9, 7, 9);
    x += (short)((digits - 1) * 6);
    for (i = 0; i < digits; i++) {
        d = n % 10;
        list = draw_sprite(list, x, y, 0, (short)(d % 5 * 7 + 64), (short)(d / 5 * 9 + 46), 0, 0);
        x -= 6;
        n /= 10;
    }
    return list;
}

#ifdef NON_MATCHING /* cadeia de testes do personagem e alocação dos temporários de escala diferentes */
void *tpt_draw_chr(void *list, unsigned short kind, int x, int y, int scale, void *face,
                               void *body)
{
    unsigned char m[4];
    int whole = 0;
    int u;
    int v;
    int c;
    int sw;

    if (face == 0) {
        return list;
    }
    if (body == 0) {
        return list;
    }
    c = charmap_lookup(kind, 0);
    u = 0;
    v = 0;
    switch (c) {
    case 1000:
        u = 0x54;
        break;
    case 0x3EF:
        v = 0x33;
        u = 0x54;
        break;
    case 0x3F0:
        v = 0x66;
        u = 0x54;
        break;
    case 0x3F1:
        v = 0x99;
        u = 0x54;
        break;
    case 0x3F2:
        v = 0xCC;
        u = 0x54;
        break;
    case 0x3F3:
        u = 0xA8;
        break;
    case 0x3F4:
        v = 0x33;
        u = 0xA8;
        break;
    default:
        if (charmap_lookup2(c, m) == 0) {
            return list;
        }
        goto draw;
    }
    whole = 1;
draw:
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 1, 1);
    if (whole) {
        list = gim_set_texture(list, body);
        list = draw_set_offsets(list, 0x33, 0x54, scale * 0x33 / 100, scale * 0x54 / 100);
        return draw_sprite(list, x, y - scale * 6 / 100, 0, v, u, 0, 0);
    }
    list = gim_set_texture(list, body);
    sw = scale * 0x33 / 100;
    list = draw_set_offsets(list, 0x33, 0x40, sw, (scale << 6) / 100);
    list = draw_sprite(list, x, y + (scale << 4) / 100, 0, m[1] * 0x33, 0, 0, 0);
    list = gim_set_texture_sub(list, face, 0, 0, 0, m[2] & 0xF);
    list = draw_set_offsets(list, 0x33, 0x32, sw, scale * 0x32 / 100);
    return draw_sprite(list, x, y, 0, m[0] % 5 * 0x33, m[0] / 5 * 0x32, 0, 0);
}
#endif

TChr *tchr_find(unsigned short id)
{
    TChr *c;
    int lo = 0;
    int hi = 0x28;
    int mid;
    int k;

    do {
        mid = (lo + hi) / 2;
        c = &tchr_tab[mid];
        k = c->id;
        if (k == id) {
            return c;
        }
        if (k < id) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    } while (lo <= hi);
    return 0;
}

int tchr_face(unsigned short id, unsigned short var)
{
    TChr *c = tchr_find(id);

    if (c == 0) {
        return -1;
    }
    if (var > 0 && var >= c->nvar) {
        return -1;
    }
    return c->face + var;
}

TChr tchr_tab[41] = {
    { 0, 0, 0, 0, 0, { "", "", "" }, -1, 0 },
    { 1, 0, 0, 0, 0, { "", "", "" }, 0, 0 },
    { 2, 1, 1, 0, 0, { "jdi", "", "" }, 1, 0 },
    { 3, 1, 1, 0, 0, { "sho", "", "" }, 2, 0 },
    { 4, 1, 1, 0, 0, { "hyt", "", "" }, 3, 0 },
    { 5, 3, 1, 0, 0, { "ask", "", "" }, 4, 0 },
    { 6, 1, 2, 1, 0, { "mab", "maa", "" }, 5, 0 },
    { 7, 2, 1, 0, 0, { "mis", "", "" }, 7, 0 },
    { 8, 3, 1, 0, 0, { "ryo", "", "" }, 8, 0 },
    { 9, 4, 1, 0, 0, { "cro", "", "" }, 9, 0 },
    { 10, 4, 1, 0, 0, { "dai", "", "" }, 10, 0 },
    { 11, 3, 2, 0, 0, { "fba", "fbb", "" }, 11, 0 },
    { 12, 5, 1, 0, 0, { "cmu", "", "" }, 13, 0 },
    { 13, 5, 1, 0, 0, { "tny", "", "" }, 14, 0 },
    { 14, 5, 2, 0, 0, { "zrg", "zmg", "" }, 15, 0 },
    { 15, 5, 1, 0, 0, { "ad3", "", "" }, 17, 0 },
    { 16, 5, 2, 0, 0, { "tiy", "tit", "" }, 18, 0 },
    { 17, 5, 1, 0, 0, { "amu", "", "" }, 20, 0 },
    { 18, 6, 3, 1, 0, { "kgy", "kgo", "kgw" }, 21, 0 },
    { 19, 3, 1, 0, 0, { "tai", "", "" }, 24, 0 },
    { 20, 0, 1, 0, 0, { "mtg", "", "" }, -1, 0 },
    { 21, 3, 1, 0, 0, { "jun", "", "" }, 26, 0 },
    { 22, 3, 1, 0, 0, { "moe", "", "" }, 27, 0 },
    { 23, 0, 1, 0, 0, { "sal", "", "" }, -1, 0 },
    { 24, 1, 2, 1, 0, { "rei", "rec", "" }, 29, 0 },
    { 25, 6, 1, 0, 0, { "ani", "", "" }, 31, 0 },
    { 26, 6, 1, 0, 0, { "oto", "", "" }, 32, 0 },
    { 27, 6, 1, 0, 0, { "bmg", "", "" }, 33, 0 },
    { 28, 6, 1, 0, 0, { "sas", "", "" }, 34, 0 },
    { 29, 0, 1, 0, 0, { "kmz", "", "" }, -1, 0 },
    { 30, 6, 1, 0, 0, { "sei", "", "" }, 36, 0 },
    { 31, 6, 2, 0, 0, { "tob", "tom", "" }, 37, 0 },
    { 32, 6, 1, 0, 0, { "cho", "", "" }, 39, 0 },
    { 33, 6, 1, 0, 0, { "shg", "", "" }, 40, 0 },
    { 34, 0, 0, 0, 0, { "", "", "" }, -1, 0 },
    { 35, 0, 1, 0, 0, { "kni", "", "" }, -1, 0 },
    { 36, 4, 1, 0, 0, { "ayu", "", "" }, 43, 0 },
    { 37, 6, 1, 0, 0, { "boy", "", "" }, 44, 0 },
    { 38, 6, 1, 0, 0, { "sra", "", "" }, 45, 0 },
    { 39, 0, 1, 0, 0, { "sam", "", "" }, -1, 0 },
    { 40, 5, 1, 0, 0, { "daa", "", "" }, 47, 0 },
};
