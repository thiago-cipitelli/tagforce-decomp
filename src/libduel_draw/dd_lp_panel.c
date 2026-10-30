// @range 0x08855B10-0x08857100
// @asm 0x08855B10-0x08855C50
// @asm 0x08855C64-0x08855E90
// @asm 0x08855E90-0x08856278
// @asm 0x088562C4-0x0885650C
// @asm 0x088565F8-0x088566EC
// @asm 0x088566EC-0x08856DC0
// @asm 0x08856DC0-0x08857100
/* The LP panel of the duel (dd_lp): its textures (lp_pnl01_j/02_j and digits in Japan,
   pnl_a00.ehf elsewhere), the object that counts the LP from one value to another, the panel
   showing the two LP (with "*" padding or "?" for hidden values), the 2D camera it is drawn
   with, and the shine of its border cycling through 9 colours. */
#include "libduel_draw.h"

void *memset(void *, int, unsigned int);
int region_get(void);
void *ehp_find(void *pack, const char *name, int *size);
int ehf_tex_or_flags(void *ehf, void *flags);
float fabsf_vfpu(float x);
void se_play(int id);
unsigned int *gfx_get_state(void);
void *dl_offset(void *list, int x, int y);
void *dl_viewport(void *list, int cx, int cy, int w, int h);
void *dl_depth_range(void *list, int near, int far);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int comp);
void *dl_set_color(void *list, unsigned int color);
void *dl_tex_offset(float u, float v, void *list);
void *dl_clear_depth(void *list, unsigned int z);
void *dl_clear(void *list, unsigned int flags);
void *dl_shade_model(void *list, int mode);
void *dl_alloc_inline(void *list, int bytes);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_stencil_op(void *list, int fail, int zfail, int zpass);
void *dl_stencil_func(void *list, int func, int ref, int mask);
void *dl_pixel_mask(void *list, unsigned int mask);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *gim_set_texture(void *list, void *gim);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_get_proj(float *fovy, float *aspect, float *near, float *far);
void mtx_perspective(float fovy, float aspect, float near, float far);
void camera_set(FxVec4 *pos, FxVec4 *target, FxVec4 *up);
void camera_load_view(void);
void *mat3_identity(float *out);
void *mat3_rot_z(float *out, float *m, float angle);
void *mat3_mul(float *out, float *a, float *b);
FxVec3 *mat3_transform_vec3(FxVec3 *out, float *m, FxVec3 *v);

void *dd_pack_get(int id);
void *dd_alloc(int size);
DdObj *dd_task_add(DdObj *desc);
void dd_task_remove(DdObj *o);
void lpc_digits_set(void *digits, int value);
void lpc_draw(DdObj *o, void *data);
void lpc_update(DdObj *o, void *data);
void lpc_free(DdObj *o, void *data);
void lps_draw(DdObj *o, void *data);
void lps_update(DdObj *o, void *data);
void lps_free(DdObj *o, void *data);

typedef struct DdLpMgr {               /* 0x08C5B294, 0x30 */
    DdObj *obj;                        /* +0x00 */
    float fovy, aspect, near, far;     /* +0x04 projection saved by dd_lp_begin_2d */
    void *pnl01;                       /* +0x14 Japan */
    void *pnl02;
    void *nmb01;
    void *nmb02;
    void *pnl_a01;                     /* +0x24 elsewhere */
    void *nmb01_a;
    void *ehf;                         /* +0x2C */
} DdLpMgr;

typedef struct DdLpCount {             /* LP counting object, 0x90 */
    int x, y;                          /* +0x00 */
    int from;                          /* +0x08 */
    int to;                            /* +0x0C */
    int cur;                           /* +0x10 */
    int speed;                         /* +0x14 */
    int sx, sy;                        /* +0x18 top-left of the panel */
    int active;                        /* +0x20 */
    int digits[20];                    /* +0x24 */
    int pad74[2];
    float scale;                       /* +0x7C */
    float alpha;                       /* +0x80 */
    float d1;                          /* +0x84 */
    float d2;                          /* +0x88 */
    int wide;                          /* +0x8C 5 digits */
} DdLpCount;

typedef struct DdLpShow {              /* LP display object, 0x34 */
    int x, y;                          /* +0x00 */
    int lp[2];                         /* +0x08 */
    int sx, sy;                        /* +0x10 */
    int active;                        /* +0x18 */
    int t;                             /* +0x1C */
    char text[2][5];                   /* +0x20 digits, least significant first */
    short pad2a;
    int wide[2];                       /* +0x2C */
} DdLpShow;

typedef struct FxVtx2DT {              /* GU_TEXTURE_16BIT | GU_COLOR_8888 | GU_VERTEX_16BIT, 2D */
    short u, v;
    unsigned int color;
    short x, y, z;
    short pad;
} FxVtx2DT;

extern DdLpMgr dd_lp;

unsigned int dd_lp_shine(int i, unsigned int frame);

#ifdef NON_MATCHING /* o original guarda o tmf num $s antes das outras buscas; ordem dos registradores diferente */
void dd_lp_load(void)
{
    DdLpMgr *m = &dd_lp;
    void *pack;
    void *tmf;

    memset(m, 0, sizeof(*m));
    if (region_get() == 0) {
        pack = dd_pack_get(0x80001);
        m->pnl01 = ehp_find(pack, "lp_pnl01_j.gim", 0);
        m->pnl02 = ehp_find(pack, "lp_pnl02_j.gim", 0);
        m->nmb01 = ehp_find(pack, "lp_pnl_nmb01_j.gim", 0);
        m->nmb02 = ehp_find(pack, "lp_pnl_nmb02_j.gim", 0);
    } else {
        pack = dd_pack_get(0x80001);
        tmf = ehp_find(pack, "pnl_a00.tmf", 0);
        m->pnl_a01 = ehp_find(pack, "lp_pnl_a01.gim", 0);
        m->nmb01_a = ehp_find(pack, "lp_pnl_nmb01_a.gim", 0);
        m->ehf = ehp_find(pack, "pnl_a00.ehf", 0);
        ehf_tex_or_flags(m->ehf, tmf);
    }
}
#endif

void dd_lp_unload(void)
{
    memset(&dd_lp, 0, sizeof(dd_lp));
}

#ifdef NON_MATCHING /* contagem de dígitos e conversões float com outra alocação */
/* Counts the LP shown at (x, y) from `from` to `to`. */
void dd_lp_count(int x, int y, int from, int to)
{
    DdLpCount *c;
    DdObj desc;
    int na;
    int nb;

    if (from < 0)
        from = 0;
    if (from > 99999)
        from = 99999;
    if (to < 0)
        to = 0;
    if (to > 99999)
        to = 99999;
    c = dd_alloc(sizeof(DdLpCount));
    c->x = x;
    c->y = y;
    c->from = from;
    c->to = to;
    c->cur = from;
    c->active = 1;
    c->pad74[0] = 0;
    c->pad74[1] = 0;
    if (region_get() == 0) {
        c->sx = (float)x - 64.0f;
        c->sy = (float)y - 48.0f;
    } else {
        na = 0;
        nb = 0;
        c->sx = (float)x - 92.0f;
        c->sy = (float)y - 61.0f;
        c->speed = 8000;
        c->scale = 0.75f;
        c->alpha = 1.0f;
        c->d1 = 0.01f;
        c->d2 = 0.02f;
        for (; to > 0; to /= 10)
            na++;
        for (; from > 0; from /= 10)
            nb++;
        if (na < 5 && nb < 5)
            c->wide = 0;
        else
            c->wide = 1;
    }
    lpc_digits_set(c->digits, c->from);
    memset(&desc, 0, sizeof(desc));
    desc.draw = lpc_draw;
    desc.update = lpc_update;
    desc.free = lpc_free;
    desc.data = c;
    dd_lp.obj = dd_task_add(&desc);
}
#endif

#ifdef NON_MATCHING /* dois laços de dígitos com region_get por iteração; alocação diferente */
/* Shows the two LP at (x, y); a negative value shows -value "?" digits. */
void dd_lp_show(int x, int y, int lp0, int lp1)
{
    int n0 = 0;
    int n1 = 0;
    DdLpShow *s;
    DdObj desc;
    int show;
    int v;
    int i;

    s = dd_alloc(sizeof(DdLpShow));
    s->x = x;
    s->y = y;
    s->active = 1;
    s->t = 0;
    if (lp0 < 0) {
        v = fabsf_vfpu((float)lp0);
        for (i = 0; i < 5; i++) {
            if (v > 0) {
                s->text[0][i] = '?';
                v--;
                n0++;
            } else {
                s->text[0][i] = '*';
            }
        }
        s->lp[0] = lp0;
    } else {
        if (lp0 < 0)
            lp0 = 0;
        if (lp0 > 99999)
            lp0 = 99999;
        for (i = 0, v = lp0; i < 5; i++) {
            show = 0;
            if (region_get() == 0) {
                if (v > 0)
                    show = 1;
                else if (i == 0)
                    show = 1;
            } else if (v > 0) {
                show = 1;
            } else if (i < 4) {
                show = 1;
            }
            if (show) {
                n0++;
                s->text[0][i] = v % 10 + '0';
                v /= 10;
            } else {
                s->text[0][i] = '*';
            }
        }
        s->lp[0] = lp0;
    }
    if (lp1 < 0) {
        v = fabsf_vfpu((float)lp1);
        for (i = 0; i < 5; i++) {
            if (v > 0) {
                s->text[1][i] = '?';
                v--;
                n1++;
            } else {
                s->text[1][i] = '*';
            }
        }
        s->lp[1] = lp1;
    } else {
        if (lp1 < 0)
            lp1 = 0;
        if (lp1 > 99999)
            lp1 = 99999;
        for (i = 0, v = lp1; i < 5; i++) {
            show = 0;
            if (region_get() == 0) {
                if (v > 0)
                    show = 1;
                else if (i == 0)
                    show = 1;
            } else if (v > 0) {
                show = 1;
            } else if (i < 4) {
                show = 1;
            }
            if (show) {
                n1++;
                s->text[1][i] = v % 10 + '0';
                v /= 10;
            } else {
                s->text[1][i] = '*';
            }
        }
        s->lp[1] = lp1;
    }
    if (region_get() == 0) {
        s->sx = (float)x - 64.0f;
        s->sy = (float)y - 48.0f;
    } else {
        s->sx = (float)x - 92.0f;
        s->sy = (float)y - 61.0f;
        if (n0 < 5)
            s->wide[0] = 0;
        else
            s->wide[0] = 1;
        if (n1 < 5)
            s->wide[1] = 0;
        else
            s->wide[1] = 1;
    }
    memset(&desc, 0, sizeof(desc));
    desc.draw = lps_draw;
    desc.update = lps_update;
    desc.free = lps_free;
    desc.data = s;
    dd_lp.obj = dd_task_add(&desc);
    se_play(0x622C);
}
#endif

int dd_lp_busy(void)
{
    return dd_lp.obj != 0;
}

void dd_lp_close(void)
{
    DdLpMgr *m = &dd_lp;

    if (m->obj != 0) {
        dd_task_remove(m->obj);
        m->obj = 0;
    }
}

#ifdef NON_MATCHING /* cópia dos modelos de vetor da câmera intercalada de outro modo */
/* Sets up a 2D-like perspective (camera at z = 10) with the origin at (x, y). */
void dd_lp_begin_2d(void *list, int x, int y)
{
    FxVec4 up = { 0.0f, 1.0f, 0.0f, 0.0f };
    FxVec4 target = { 0.0f, 0.0f, 0.0f, 1.0f };
    FxVec4 eye = { 0.0f, 0.0f, 10.0f, 1.0f };

    mtx_set_mode(0);
    mtx_push();
    mtx_set_mode(1);
    mtx_push();
    mtx_get_proj(&dd_lp.fovy, &dd_lp.aspect, &dd_lp.near, &dd_lp.far);
    list = dl_offset(list, 0x710 - x, 0x778 - y);
    list = dl_viewport(list, 0x800, 0x800, 480, 272);
    list = dl_depth_range(list, 0xFFFF, 0);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(0.383972436f, 1.76470589f, 5.0f, 400.0f);
    mtx_set_mode(1);
    mtx_identity();
    camera_set(&eye, &target, &up);
    camera_load_view();
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_disable(list, 0);
    list = dl_disable(list, 1);
    list = dl_set_color(list, 0xFFFFFFFF);
    list = dl_disable(list, 10);
    list = dl_enable(list, 5);
    list = dl_tex_offset(0.0f, 0.0f, list);
    dl_enable(list, 9);
}
#endif

void *dd_lp_end_2d(void *list)
{
    list = dl_offset(list, 0x710, 0x778);
    mtx_set_mode(0);
    mtx_pop();
    mtx_perspective(dd_lp.fovy, dd_lp.aspect, dd_lp.near, dd_lp.far);
    mtx_set_mode(1);
    mtx_pop();
    return list;
}

/* Fills a 2D sprite (2 vertices) of size (w, h) at (x, y, z) with texture origin (u, v). */
void dd_lp_quad(FxVtx2DT *v, int x, int y, int z, int u, int tv, int w, int h,
                unsigned int color)
{
    v[0].u = u + w;
    v[0].v = tv;
    v[0].x = x + w;
    v[0].y = y;
    v[0].z = z;
    v[0].color = color;
    v[1].u = u;
    v[1].v = tv;
    v[1].x = x;
    v[1].y = y;
    v[1].z = z;
    v[1].color = color;
    v[2].u = u + w;
    v[2].v = tv + h;
    v[2].x = x + w;
    v[2].y = y + h;
    v[2].z = z;
    v[2].color = color;
    v[3].u = u;
    v[3].v = tv + h;
    v[3].x = x;
    v[3].y = y + h;
    v[3].z = z;
    v[3].color = color;
}

#ifdef NON_MATCHING /* ordem das contas de ponto flutuante diferente */
/* Fills a textured quad (4 vertices, u v z x y z) centred on (x, y) from a texel rectangle. */
void dd_lp_rect(float x, float y, float z, float w, float h, float *v, int tx, int ty, int tw, int th,
                int texw, int texh, float vz)
{
    float fw = (float)texw;
    float fh = (float)texh;
    float u0 = (float)tx / fw - 0.5f / fw;
    float u1 = (float)(tx + tw) / fw - 0.5f / fw;
    float v0;
    float v1;
    float x1;
    float y1;

    v[0] = u1;
    v0 = (float)ty / fh - 0.5f / fh;
    v[1] = v0;
    v1 = (float)(ty + th) / fh - 0.5f / fh;
    x1 = x + w * 0.5f;
    v[3] = x1;
    y1 = y + h * 0.5f;
    v[4] = y1;
    v[5] = z;
    v[2] = vz;
    v[6] = u0;
    x = x + w * -0.5f;
    v[7] = v0;
    v[9] = x;
    v[10] = y1;
    v[11] = z;
    v[8] = vz;
    v[12] = u1;
    v[13] = v1;
    y = y + h * -0.5f;
    v[15] = x1;
    v[16] = y;
    v[17] = z;
    v[14] = vz;
    v[18] = u0;
    v[19] = v1;
    v[21] = x;
    v[22] = y;
    v[23] = z;
    v[20] = vz;
}
#endif

/* Draws the Japanese LP panel frame at (x, y): three shining bands, the border masked through
   the stencil with a rotating light, and the front plate. */
#ifdef NON_MATCHING /* função de 1,7 KB com três listas de vértices; escalonamento diferente */
void dd_lp_frame(void *list, int x, int y)
{
    unsigned int frame = *gfx_get_state();
    unsigned int c[3] = { 0 };
    FxVtx2DT *v;
    FxVtx2DT *l;
    float rot[9];
    float m[9];
    float t[9];
    FxVec3 p;
    float cx;
    short bx;
    short by;
    int i;

    c[0] = dd_lp_shine(0, frame);
    c[1] = dd_lp_shine(1, frame);
    c[2] = dd_lp_shine(2, frame);
    list = dl_clear_depth(list, 0);
    list = dl_clear(list, 2);
    list = dl_shade_model(list, 1);
    v = dl_alloc_inline(&list, sizeof(FxVtx2DT) * 6);
    v[0].u = 128; v[0].v = 0; v[0].x = x + 128; v[0].y = y; v[0].z = 0; v[0].color = c[0];
    v[1].u = 0; v[1].v = 0; v[1].x = x; v[1].y = y; v[1].z = 0; v[1].color = c[0];
    v[2].u = 128; v[2].v = 48; v[2].x = x + 128; v[2].y = (float)y + 48.0f; v[2].z = 0; v[2].color = c[1];
    v[3].u = 0; v[3].v = 48; v[3].x = x; v[3].y = (float)y + 48.0f; v[3].z = 0; v[3].color = c[1];
    v[4].u = 128; v[4].v = 96; v[4].x = x + 128; v[4].y = y + 96; v[4].z = 0; v[4].color = c[2];
    v[5].u = 0; v[5].v = 96; v[5].x = x; v[5].y = y + 96; v[5].z = 0; v[5].color = c[2];
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, dd_lp.pnl01);
    list = draw_array(list, 4, 0x80011E, 6, 0, v);
    list = dl_stencil_op(list, 1, 1, 0xFF);
    list = dl_stencil_func(list, 0, 0, 2);
    list = dl_enable(list, 3);
    list = dl_pixel_mask(list, 0xFFFFFF);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_enable(list, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    v = dl_alloc_inline(&list, sizeof(FxVtx2DT) * 4);
    dd_lp_quad(v, x, y, 0, 0, 96, 128, 96, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_array(list, 4, 0x80011E, 4, 0, v);
    list = dl_pixel_mask(list, 0);
    list = dl_disable(list, 0);
    list = dl_stencil_op(list, 2, 1, 0xFF);
    list = dl_stencil_func(list, 0, 0, 0);
    list = dl_enable(list, 3);
    list = dl_blend_func(list, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dl_enable(list, 4);
    l = dl_alloc_inline(&list, sizeof(FxVtx2DT) * 4);
    bx = (float)x + -16.0f;
    by = (float)y + -32.0f;
    l[0].u = 64; l[0].v = 192; l[0].x = bx + 160; l[0].y = by; l[0].z = 0; l[0].color = 0x50FFFFFF;
    l[1].u = 0; l[1].v = 192; l[1].x = bx; l[1].y = by; l[1].z = 0; l[1].color = 0x50FFFFFF;
    l[2].u = 64; l[2].v = 256; l[2].x = bx + 160; l[2].y = by + 160; l[2].z = 0; l[2].color = 0x50FFFFFF;
    l[3].u = 0; l[3].v = 256; l[3].x = bx; l[3].y = by + 160; l[3].z = 0; l[3].color = 0x50FFFFFF;
    cx = ((float)(int)((float)x + -16.0f) + 80.0f) * -1.0f;
    for (i = 0; i < 4; i++) {
        p.z = 1.0f;
        p.x = l[i].x;
        p.y = l[i].y;
        mat3_identity(rot);
        mat3_rot_z(rot, rot, (float)frame / 20.0f);
        mat3_identity(t);
        t[8] = 1.0f;
        t[6] = cx;
        t[7] = ((float)(int)((float)y + -32.0f) + 80.0f) * -1.0f;
        mat3_mul(m, rot, t);
        t[6] *= -1.0f;
        t[7] *= -1.0f;
        mat3_mul(m, t, m);
        mat3_transform_vec3(&p, m, &p);
        l[i].x = p.x;
        l[i].y = p.y;
    }
    list = dl_set_tex_filter(list, 1, 1);
    list = draw_array(list, 4, 0x80011E, 4, 0, l);
    list = dl_disable(list, 3);
    v = dl_alloc_inline(&list, sizeof(FxVtx2DT) * 4);
    dd_lp_quad(v, x, y, 0, 0, 0, 128, 96, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, dd_lp.pnl02);
    draw_array(list, 4, 0x80011E, 4, 0, v);
}
#endif

#ifdef NON_MATCHING /* tabela de 81 bytes copiada byte a byte e conversões; formas diferentes */
/* Colour of band `i` of the panel shine at `frame`: 9 keys of 5 frames, interpolated. */
unsigned int dd_lp_shine(int i, unsigned int frame)
{
    unsigned char tab[81] = {
        0xAF, 0xFF, 0xFF, 0x3F, 0xE5, 0xCB, 0x31, 0x5F, 0x7B, 0xB1, 0xFF, 0xFF, 0x42, 0xE1, 0x79, 0x33,
        0x7A, 0x73, 0xA7, 0xFF, 0xAF, 0x7D, 0xDF, 0x3E, 0x2E, 0x82, 0x3E, 0xF5, 0xFF, 0xA5, 0xD9, 0xDC,
        0x43, 0x61, 0x89, 0x3E, 0xFF, 0xFF, 0xAA, 0xD5, 0x7D, 0x43, 0x7D, 0x7E, 0x34, 0xFF, 0xAD, 0xB1,
        0xD6, 0x48, 0x7C, 0x7F, 0x39, 0x3B, 0xFF, 0xAB, 0xEC, 0xCE, 0x46, 0xCC, 0x85, 0x37, 0x64, 0xFF,
        0xB0, 0xFF, 0x6D, 0x42, 0xD4, 0x71, 0x35, 0x81, 0xA6, 0xB4, 0xFF, 0x38, 0x7D, 0xCF, 0x2E, 0x33,
        0x7B
    };
    unsigned int f = frame % 45;
    unsigned int k = f / 5;
    unsigned int n = k + 1;
    unsigned char *a;
    unsigned char *b;
    unsigned int r;
    unsigned int g;
    unsigned int bl;
    float t;

    if (n > 8)
        n = 0;
    a = &tab[k * 9 + i * 3];
    b = &tab[n * 9 + i * 3];
    t = (float)(f % 5) / 5.0f;
    bl = (float)a[2] + t * ((float)b[2] - (float)a[2]);
    g = (float)a[1] + t * ((float)b[1] - (float)a[1]);
    r = (float)a[0] + t * ((float)b[0] - (float)a[0]);
    return (r & 0xFF) | (bl & 0xFF) << 16 | 0xFF000000 | (g & 0xFF) << 8;
}
#endif
