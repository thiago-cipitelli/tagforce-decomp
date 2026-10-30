// @range 0x08810758-0x08811718
// @asm 0x08810758-0x0881168C
/* ATK/DEF of the monsters drawn over the field (animated when they change, blue when they rise,
   red when they fall), the selection count marker over a card, and small helpers. */
#include "dd_d1.h"

void mtx_set_mode(int mode);
void mtx_get34(DdMtx *m);
void mat4_transform_vec4(DdXyzw *out, DdMtx *m, DdXyzw *v);
void mat4_mul(DdMtx *out, DdMtx *a, DdMtx *b);
void mat4_identity(DdMtx *m);
void mat4_translate(DdMtx *out, DdMtx *m, DdXyzw *v);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);

void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_tex_func(void *list, int func, int comp);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
void *dl_ambient_color(void *list, unsigned int color);
int wcslen(const unsigned short *s);
unsigned short *wtext_from_int(int v, unsigned short *out);
void font_set_size(int w, int h);
unsigned int font_set_color(unsigned int c);
int font_measure_text(unsigned short *text);
int font_draw_text(int x, int y, int w, unsigned short *text);
void font_text_set(void *list);
void *font_text_clear(void);
void func_08866A1C(int player, int zone, int *out);

#ifdef NON_MATCHING /* conversões float->unsigned por canal de cor; forma não reproduzida */
extern DdXyzw dd_counter_off;          /* 0x08874F80 */
extern unsigned int dd_counter_col[4]; /* 0x08874F90 atk, def colours (defense position, attack position) */

static unsigned int dd_dim(unsigned int c, float r, float g, float b)
{
    return (c & 0xFF000000) | (unsigned int)((float)(c >> 16 & 0xFF) * b) << 16 |
           (unsigned int)((float)(c >> 8 & 0xFF) * g) << 8 | (unsigned int)((float)(c & 0xFF) * r);
}

/* ATK and DEF of the monster at `at`, projected above its card at pos */
void *dd_counter_draw(void *list, DdPos at, DdXyzw *pos)
{
    DdMtx view;
    DdMtx proj;
    DdMtx vp;
    DdMtx m;
    DdXyzw off;
    DdXyzw p;
    DdXyzw s;
    DdZoneAnim *za = &dd_cobjs.za[at.player][at.zone];
    int w = 7;
    int h = 9;
    int changed = 0;
    int val[2];
    unsigned int col[2];
    unsigned short txt[16];
    unsigned short *c;
    int k;
    int x;
    int y;

    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(0);
    mtx_get34(&proj);
    mat4_mul(&vp, &proj, &view);
    mtx_set_mode(2);
    off = dd_counter_off;
    if (pos->z < 0.0f)
        off.z = dd_counter_off.z * -1.0f;
    mat4_identity(&m);
    mat4_translate(&m, &m, pos);
    mat4_transform_vec4(&p, &m, &off);
    mat4_transform_vec4(&s, &vp, &p);
    s.x = s.x * 480.0f / s.w + 2122.0f - 1808.0f;
    s.y = s.y * -272.0f / s.w + 2044.0f - 1912.0f;
    val[0] = za->a;
    val[1] = za->b;
    if (at.f15) {
        col[0] = dd_counter_col[0];
        col[1] = dd_counter_col[1];
    } else {
        col[0] = dd_counter_col[2];
        col[1] = dd_counter_col[3];
    }
    if (za->a < za->a1) {
        col[0] = dd_dim(col[0], 0.5f, 0.5f, 1.0f);
        changed = 1;
    }
    if (za->b < za->b1) {
        col[1] = dd_dim(col[1], 0.5f, 0.5f, 1.0f);
        changed = 1;
    }
    if (za->a1 < za->a) {
        col[0] = dd_dim(col[0], 1.0f, 0.5f, 0.5f);
        changed = 1;
    }
    if (za->b1 < za->b) {
        col[1] = dd_dim(col[1], 1.0f, 0.5f, 0.5f);
        changed = 1;
    }
    if (changed) {
        w = 8;
        h = 10;
    }
    if (pos->z >= 0.0f)
        y = (int)(s.y + 9.0f - (float)h);
    else
        y = (int)(s.y - 9.0f - (float)h);
    list = dl_enable(list, 9);
    list = dl_set_tex_filter(list, 0, 0);
    for (k = 0; k < 2; k++, y += h) {
        c = wtext_from_int(val[k], txt);
        x = (int)(s.x - (float)(w * wcslen(c) / 2));
        list = dl_ambient_color(list, col[k]);
        for (; *c; c++, x += w) {
            list = draw_set_offsets(list, 7, 9, w, h);
            list = draw_sprite(list, x, y, 0, (*c - '0') * 7, 0, 0, 0);
        }
    }
    return dl_set_tex_filter(list, 1, 1);
}
#endif

#ifdef NON_MATCHING /* alocação/ordem não reproduzidas */
/* advances the ATK/DEF animations */
void dd_counter_tick(float dt)
{
    DdCobjs *b = &dd_cobjs;
    int p;
    int z;
    DdZoneAnim *za;
    float k;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            za = &b->za[p][z];
            if (za->t < za->dur) {
                za->t += dt;
                if (za->t >= za->dur) {
                    za->a = za->a1;
                    za->b = za->b1;
                } else {
                    k = za->t / za->dur;
                    za->a = (float)za->a0 * (1.0f - k) + (float)za->a1 * k;
                    za->b = (float)za->b0 * (1.0f - k) + (float)za->b1 * k;
                }
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* alocação/ordem não reproduzidas */
/* marker with the selection count n over the card at pos */
void dd_counter_sel(DdXyzw *pos, int stack, int n)
{
    DdMtx view;
    DdMtx proj;
    DdMtx vp;
    DdXyzw p;
    DdXyzw s;
    unsigned short txt[16];
    void *list;
    void *gim;
    int w;

    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(0);
    mtx_get34(&proj);
    mtx_set_mode(2);
    mat4_mul(&vp, &proj, &view);
    p = *pos;
    p.w = 1.0f;
    p.y += dd_stack_y(stack) + 0.1f;
    p.z -= 44.0f;
    mat4_transform_vec4(&s, &vp, &p);
    s.x = s.x * 480.0f / s.w + 2122.0f - 1808.0f;
    s.y = s.y * -272.0f / s.w + 2044.0f - 1912.0f;
    list = dl_select_layer(4, 0);
    list = dl_disable(list, 1);
    gim = dd_gim_get(0x4001E);
    if (gim)
        list = gim_set_texture(list, gim);
    list = dl_tex_func(list, 0, 1);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = draw_set_offsets(list, 0x34, 0x36, 0x1C, 0xE);
    list = draw_sprite(list, (int)s.x - 0xE, (int)s.y, 0, 0x2E, 0x48, 0, 0);
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    wtext_from_int(n, txt);
    w = font_measure_text(txt);
    font_text_set(list);
    font_draw_text((int)s.x * 64 - w / 2, (int)s.y << 6, 0x8000, txt);
    list = font_text_clear();
    list = dl_enable(list, 1);
    dl_set_list(list);
}
#endif

#ifdef NON_MATCHING /* alocação/ordem não reproduzidas */
/* takes the ATK/DEF of the monster at (p, z) from the engine without animation */
void dd_counter_reset(int p, int z)
{
    DdCobjs *b = &dd_cobjs;
    DdZoneAnim *za;
    int st[11];

    if (z >= 0 && z < 5) {
        za = &b->za[p][z];
        memset(za, 0, sizeof(DdZoneAnim));
        func_08866A1C(p, z, st);
        za->a = st[5];
        za->b = st[6];
        za->a0 = za->a1 = za->a;
        za->b0 = za->b1 = za->b;
    }
}
#endif

/* sprite (u, v) of a 2D table entry at x, y */
void *dd_sprite_at(void *list, int x, int y, unsigned short *uv)
{
    return draw_sprite(draw_set_offsets(list, uv[2], uv[3], uv[2], uv[3]), x, y, 0, uv[0], uv[1], 0, 0);
}

void dd_lpp_clear(void)
{
    memset(&dd_lpp, 0, sizeof(DdLpp));
}

void dd_lpp_free(void)
{
}
