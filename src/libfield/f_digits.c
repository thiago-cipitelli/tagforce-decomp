// @range 0x08814D94-0x08815FD8
// @asm 0x08815984-0x08815AA8
// @rodata 0x08838AC4-0x08838AE0
/* Sprite digits and strings of digits of the HUD, the icons of the day parts and the hands of the
   clock (rotated quads, still in asm). */
#include "libfield.h"
#include "allegrex.h"

void *fld_clock_hand(float angle, void *list, int x, int y, int z, int u, int v, int w, int h);

extern const FieldHudCell field_hudbox_cells[2];   /* 0x08838A1C the plate and the per-unit bar */
extern const int field_hudbox_pos[9][2];           /* 0x08838A34 plate, bar, text, then (number, label) x 3 */
extern const int field_hudbox_icons[3][2];         /* 0x08838AAC */
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);

/* the counter n (clamped to 0..999) as three digits from the right, starting dx right of (px, py) */
#define FLD_HUD_COUNT(n, px, py, dx)                                                          \
    if ((n) >= 0) {                                                                            \
        int c = ALLEGREX_MIN(ALLEGREX_MAX((n), 0), 999);                                       \
        int x0 = (px);                                                                         \
        int y0 = (py);                                                                         \
                                                                                               \
        for (i = 0; i < 3; i++) {                                                              \
            buf[1] = 0;                                                                        \
            buf[0] = c % 10 + '0';                                                             \
            c /= 10;                                                                           \
            font_text_set(list);                                                               \
            x = s->pos.x + (float)x0 + (float)((2 - i) * 12) dx;                                \
            font_draw_text(x << 6, (int)(s->pos.y + (float)y0) << 6, (480 - x) << 6, buf);     \
            list = font_text_clear();                                                          \
        }                                                                                      \
    }

#define FLD_HUD_LABEL(n, k, e)                                                                 \
    if ((n) >= 0) {                                                                            \
        font_text_set(list);                                                                   \
        text = table_get_entry(fld_gim_get(5), e);                                             \
        font_draw_text((int)(s->pos.x + (float)field_hudbox_pos[k][0]) << 6,                    \
                       (int)(s->pos.y + (float)field_hudbox_pos[k][1]) << 6,                    \
                       (480 - (int)(s->pos.x + (float)field_hudbox_pos[k][0])) << 6, text);     \
        list = font_text_clear();                                                              \
    }

/* the info box of the HUD (field_hud.boxw): its text, the three counters p[1..3] (with labels in the
   Japanese layout, icons otherwise), the plate and the bar of p[4] units */
void *fld_hud_draw_all(void *list)
{
    FieldHud *h = &field_hud;
    int *p = field_hud.p;
    FieldHudName *s = &field_hud.boxw;
    int i;
    int x;
    unsigned short *text;
    const FieldHudCell *c;
    const int (*q)[2];
    int n;
    unsigned short buf[256];

    fld_story_get();
    if (s->state == 0)
        return list;
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_color(0xFF000000);
    if (p[0] != 0) {
        font_text_set(list);
        font_draw_wtext((int)(s->pos.x + (float)field_hudbox_pos[2][0]) << 6,
                        (int)(s->pos.y + (float)field_hudbox_pos[2][1]) << 6, 0x2580, (unsigned short *)p[0]);
        list = font_text_clear();
    }
    if (region_get() == 0) {
        FLD_HUD_COUNT(p[1], field_hudbox_pos[3][0], field_hudbox_pos[3][1], )
        FLD_HUD_COUNT(p[2], field_hudbox_pos[5][0], field_hudbox_pos[5][1], )
        FLD_HUD_COUNT(p[3], field_hudbox_pos[7][0], field_hudbox_pos[7][1], )
        FLD_HUD_LABEL(p[1], 4, 15)
        FLD_HUD_LABEL(p[2], 6, 16)
        FLD_HUD_LABEL(p[3], 8, 17)
    } else {
        FLD_HUD_COUNT(p[1], field_hudbox_pos[3][0], field_hudbox_pos[3][1], + 15.0f)
        FLD_HUD_COUNT(p[2], field_hudbox_pos[5][0], field_hudbox_pos[5][1], + 15.0f)
        FLD_HUD_COUNT(p[3], field_hudbox_pos[7][0], field_hudbox_pos[7][1], + 15.0f)
        list = gim_set_texture(list, h->gim19);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        if (p[1] >= 0)
            list = fld_draw_cell1(list, s->pos.x + (float)field_hudbox_pos[3][0], s->pos.y + (float)field_hudbox_pos[3][1] + 1.0f,
                                  field_hudbox_icons[0][0], field_hudbox_icons[0][1], 13, 13);
        if (p[2] >= 0)
            list = fld_draw_cell1(list, s->pos.x + (float)field_hudbox_pos[5][0], s->pos.y + (float)field_hudbox_pos[5][1] + 1.0f,
                                  field_hudbox_icons[1][0], field_hudbox_icons[1][1], 13, 13);
        if (p[3] >= 0)
            list = fld_draw_cell1(list, s->pos.x + (float)field_hudbox_pos[7][0], s->pos.y + (float)field_hudbox_pos[7][1] + 1.0f,
                                  field_hudbox_icons[2][0], field_hudbox_icons[2][1], 13, 13);
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    c = field_hudbox_cells;
    q = field_hudbox_pos;
    if (p[0] != 0) {
        list = gim_set_texture(list, *(void **)((char *)h + c[0].gim * 4 + 4)); /* (&h->gim0)[gim] */
        list = fld_draw_cell1(list, s->pos.x + (float)q[0][0], s->pos.y + (float)q[0][1],
                              c[0].u, c[0].v, c[0].w,
                              c[0].h);
    }
    n = p[4];
    if (n > 0) {
        list = gim_set_texture(list, (&h->gim0)[field_hudbox_cells[1].gim]);
        list = fld_draw_cell1(list, s->pos.x + (float)field_hudbox_pos[1][0], s->pos.y + (float)field_hudbox_pos[1][1],
                              field_hudbox_cells[1].u, field_hudbox_cells[1].v, n * field_hudbox_cells[1].w,
                              field_hudbox_cells[1].h);
    }
    return list;
}

void fld_hud_draw_free(void)
{
}

void *fld_digit(float scale, void *list, int x, int y, int z, int u, int v, int w, int h, int frame)
{
    list = draw_set_offsets(list, w, h, (int)(w * scale), (int)(h * scale));
    return draw_sprite(list, x, y, z, u + w * frame, v, 0, 0);
}

#ifdef NON_MATCHING /* só a ordem de um move de argumento (a3) e do sw do dígito na pilha difere */
void *fld_digits(void *list, char *s, int x, int y, int z, int u, int v, int w, int h, int gap,
                 float scale)
{
    int n = strlen(s) - 1;
    int i = n;
    float step;

    if (i >= 0) {
        step = gap + w * scale;
        do {
            list = fld_digit(scale, list, (int)(x - step * (n - i)), y, z, u, v, w, h, s[i] - '0');
        } while (--i >= 0);
    }
    return list;
}
#endif

void *fld_day_icon(void *list, int n, int x, int y, int z)
{
    int u;
    int v;

    switch (n) {
    case 0:
    case 1:
    case 2:
    case 3:
        u = 0;
        v = 0;
        break;
    case 4:
    case 5:
    case 6:
        u = 0;
        v = 16;
        n -= 4;
        break;
    }
    return fld_digit(1.0f, list, x, y, z, u, v, 64, 16, n);
}

void *fld_clock_hands(void *list, int x, int y, int z, int h, int m)
{
    float a = 360.0f * (m / 60.0f);

    list = fld_clock_hand((3.1415927f * a) / 180.0f, list, x, y, z, 0xF8, 0x20, 3, 0x27);
    return fld_clock_hand((3.1415927f * (360.0f * (h / 12.0f) + a / 12.0f)) / 180.0f, list, x, y, z, 0xFC,
                          0x20, 3, 0x27);
}

void vec3_set(Vec3 *out, float x, float y, float z);
void mat3_rot_z(float *out, float *m, float angle);
void *dl_tex_wrap(void *list, int u, int v);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);

/* a hand of the clock: the w x h cell (u, v) as a quad turned by angle around its centre, at
   (x, y) with depth z */
void *fld_clock_hand(float angle, void *list, int x, int y, int z, int u, int v, int w, int h)
{
    float m[9];
    Vec3 p0;
    Vec3 p1;
    Vec3 p2;
    Vec3 p3;
    float cw;
    float ch;
    FieldTexVtx *q;

    vec3_set(&p0, 0.0f, 0.0f, 0.0f);
    vec3_set(&p1, w, 0.0f, 0.0f);
    vec3_set(&p2, 0.0f, h, 0.0f);
    vec3_set(&p3, w, h, 0.0f);
    mat3_identity(m);
    mat3_rot_z(m, m, angle);
    cw = 0.5f * w;
    ch = 0.5f * h;
    p0.x -= cw;
    p0.y -= ch;
    p1.x -= cw;
    p1.y -= ch;
    p2.x -= cw;
    p2.y -= ch;
    p3.x -= cw;
    p3.y -= ch;
    mat3_transform_vec3(&p0, m, &p0);
    mat3_transform_vec3(&p1, m, &p1);
    mat3_transform_vec3(&p2, m, &p2);
    mat3_transform_vec3(&p3, m, &p3);
    p0.x += cw;
    p0.y += ch;
    p1.x += cw;
    p1.y += ch;
    p2.x += cw;
    p2.y += ch;
    p3.x += cw;
    p3.y += ch;
    q = dl_alloc_inline(&list, sizeof(FieldTexVtx) * 4);
    q[0].u = u + w;
    q[0].v = v + h;
    q[0].color = 0xFFFFFFFF;
    q[0].x = p3.x + (float)x;
    q[0].y = p3.y + (float)y;
    q[0].z = z;
    q[1].u = u + w;
    q[1].v = v;
    q[1].color = 0xFFFFFFFF;
    q[1].x = p1.x + (float)x;
    q[1].y = p1.y + (float)y;
    q[1].z = z;
    q[2].u = u;
    q[2].v = v + h;
    q[2].color = 0xFFFFFFFF;
    q[2].x = p2.x + (float)x;
    q[2].y = p2.y + (float)y;
    q[2].z = z;
    q[3].u = u;
    q[3].v = v;
    q[3].color = 0xFFFFFFFF;
    q[3].x = p0.x + (float)x;
    q[3].y = p0.y + (float)y;
    q[3].z = z;
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    list = dl_set_tex_filter(list, 1, 1);
    list = dl_tex_wrap(list, 1, 1);
    list = dl_draw_array(list, 4, 0x80011E, 4, 0, q);
    mtx_pop();
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_tex_wrap(list, 0, 0);
    return list;
}
