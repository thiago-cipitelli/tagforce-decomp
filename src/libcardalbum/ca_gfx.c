// @range 0x08804D08-0x08805CA4
// @data 0x0880DA40-0x0880DBA0
/* 3D part of the screen: GE state, light and camera of the binder, the cover model opening/closing,
   the page models (one copy per page, with the card pictures as texture hooks) animated through the
   page kinds while they turn, and the 3D cursor over the card cells. */
#include "libcardalbum.h"

typedef struct Vec3 {
    float x, y, z;
} Vec3;

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

typedef struct Light {         /* BOOT light, 0x3C */
    int type;
    unsigned int ambient;
    unsigned int diffuse;
    unsigned int specular;
    Vec3 pos;                  /* +0x10 */
    Vec3 dir;
    float spot_exp;
    float spot_cutoff;
    float att[3];
} Light;

typedef struct CaKind {        /* page animation: model, frame range, kind once it ends */
    unsigned short model;
    float start;
    float end;
    unsigned char next;
} CaKind;

typedef struct CaCell {        /* 3D cursor position of a card cell */
    float x, z;
} CaCell;

typedef struct Mat4 {
    float m[16];
} Mat4;

extern Light ca_light;

unsigned int *dl_offset(unsigned int *list, int x, int y);
unsigned int *dl_viewport(unsigned int *list, int cx, int cy, int w, int h);
unsigned int *dl_depth_range(unsigned int *list, int near, int far);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask);
unsigned int *dl_depth_func(unsigned int *list, int func);
unsigned int *dl_front_face(unsigned int *list, int cw);
unsigned int *dl_shade_model(unsigned int *list, int mode);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *view_init(unsigned int *list);
unsigned int *lighting_enable(unsigned int *list);
unsigned int *light_enable(unsigned int *list, int n);
unsigned int *light_set(unsigned int *list, int n, Light *l);
unsigned int *light_set_ambient(unsigned int *list, unsigned int color);
unsigned int *dl_material_colors(unsigned int *list, unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular);
unsigned int *dl_color_material(unsigned int *list, int comps);
unsigned int *dl_specular_power(float p, unsigned int *list);
unsigned int *dl_fog(float near, float far, unsigned int *list, unsigned int color);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_tex_offset(unsigned int *list, float u, float v);
unsigned int *dl_tex_scale(float u, float v, unsigned int *list);
unsigned int *dl_bone_matrix(unsigned int *list, int index, float *m);
Mat4 *mat4_identity(Mat4 *out);
void mtx_set_mode(int mode);
void mtx_identity(void);
void mtx_translate(Vec3 *v);
void mtx_perspective(float fovy, float aspect, float near, float far);
void camera_set(Vec4 *pos, Vec4 *target, Vec4 *up);
void camera_load_view(void);
GfxState *gfx_get_state(void);

void *ca_model_alloc(unsigned int size);
void ca_model_free(void *p);
int ca_page_tex_hook(unsigned int **list, int a1, int tex, int a3, ModelDraw *d);
void ca_page_draw(CaPage *p);
void ca_cursor3d_draw(int pos);
void ca_pages_shift_back(void);
void ca_pages_shift_fwd(void);
CaPage *ca_page_new(int slot, int kind, int page);

extern Vec4 ca_cam_pos, ca_cam_target, ca_cam_up;
extern CaKind ca_kinds[8];
extern CaCell ca_cells[18];

/* GE state of the screen, light and projection; the light direction template is ca_light_dir0 (0x0880DA40) */
void ca_gfx_init(void)
{
    unsigned int *list;
    Mat4 m __attribute__((aligned(16)));  /* 16-byte aligned for the VFPU */
    int i;

    list = dl_select_layer(0, 0);
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
    list = dl_disable(list, 3);
    list = dl_disable(list, 6);
    list = dl_disable(list, 0x12);
    list = dl_disable(list, 0x11);
    list = dl_disable(list, 0x15);
    list = dl_color(list, 0xFF0000FF);
    list = view_init(list);
    model_set_allocator(ca_model_alloc, ca_model_free);
    memset(&ca_light, 0, sizeof(ca_light));
    ca_light.type = 0;
    ca_light.ambient = 0xFF000000;
    ca_light.diffuse = 0xFFFFFFFF;
    ca_light.specular = 0;
    ca_light.pos = (Vec3){ -0.3301596f, 0.8549623f, -0.4000427f };
    list = lighting_enable(list);
    list = light_enable(list, 0);
    list = light_set_ambient(list, 0xFF333333);
    list = dl_material_colors(list, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    list = dl_color_material(list, 7);
    list = dl_specular_power(10.0f, list);
    list = dl_fog(50.0f, 100.0f, list, 0x404040);
    list = dl_disable(list, 7);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(0.19970527f, 1.7647059f, 10.0f, 40.0f);
    mtx_set_mode(1);
    mtx_identity();
    mtx_set_mode(2);
    mtx_identity();
    list = dl_clear_color(list, 0xFF770000);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_set_tex_filter(list, 1, 1);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(1.0f, -1.0f, list);
    list = dl_enable(list, 9);
    mat4_identity(&m);
    for (i = 0; i < 8; i++)
        list = dl_bone_matrix(list, i, m.m);
    dl_set_list(list);
}

/* per frame: lit 3D layer, camera pulled sideways while the cover opens */
void ca_gfx_begin3d(void)
{
    unsigned int *list;
    Vec3 v;

    list = dl_select_layer(1, 0);
    list = light_set(list, 0, &ca_light);
    list = lighting_enable(list);
    list = light_enable(list, 0);
    list = light_set_ambient(list, 0xFF333333);
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
    list = dl_color(list, 0xFF0000FF);
    list = dl_fog(50.0f, 100.0f, list, 0x404040);
    list = dl_disable(list, 7);
    list = dl_set_tex_filter(list, 1, 1);
    dl_set_list(list);
    mtx_set_mode(0);
    mtx_identity();
    v = (Vec3){ 0.14f, 0.0f, 0.0f };
    v.x += ca.book_x;
    mtx_translate(&v);
    mtx_perspective(0.19970527f, 1.7647059f, 10.0f, 40.0f);
    mtx_set_mode(1);
    mtx_identity();
    {
        Vec4 pos, target, up;
        up = ca_cam_up;
        target = ca_cam_target;
        pos = ca_cam_pos;
        camera_set(&pos, &target, &up);
    }
    camera_load_view();
    mtx_set_mode(2);
    mtx_identity();
}

/* camera of the binder */
Vec4 ca_cam_pos = { 0.0f, 26.472485f, 12.659547f, 1.0f };
Vec4 ca_cam_target = { 0.0f, -63.986427f, -29.96891f, 1.0f };
Vec4 ca_cam_up = { 0.0f, 0.42628458f, -0.9045891f, 0.0f };

/* the frame range each page kind plays and the kind it turns into */
CaKind ca_kinds[8] = {
    { 0x1021, 0.0f, 55.0f, 0 },    /* 0 left stack */
    { 0x1022, 25.0f, 25.0f, 1 },   /* 1 open, left side */
    { 0x1022, 0.0f, 0.0f, 2 },     /* 2 open, right side */
    { 0x1021, 0.0f, 14.0f, 2 },    /* 3 */
    { 0x1022, 0.0f, 25.0f, 1 },    /* 4 turning back */
    { 0x1022, 30.0f, 55.0f, 2 },   /* 5 turning forward */
    { 0x1021, 14.0f, 30.0f, 7 },   /* 6 */
    { 0x1021, 55.0f, 55.0f, 0 },   /* 7 hidden */
};

/* 3D cursor over the 18 cells: 6 columns (3 per side) by 3 rows */
CaCell ca_cells[18] = {
    { -2.8f, -1.34f }, { -1.855f, -1.34f }, { -0.91f, -1.34f },
    { 0.91f, -1.34f }, { 1.855f, -1.34f }, { 2.8f, -1.34f },
    { -2.8f, 0.0f }, { -1.855f, 0.0f }, { -0.91f, 0.0f },
    { 0.91f, 0.0f }, { 1.855f, 0.0f }, { 2.8f, 0.0f },
    { -2.8f, 1.34f }, { -1.855f, 1.34f }, { -0.91f, 1.34f },
    { 0.91f, 1.34f }, { 1.855f, 1.34f }, { 2.8f, 1.34f },
};

/* the cover starts opening, the first three pages appear behind it */
void ca_book_open(void)
{
    Model *m = ca_model_get(CA_COVER);

    ca.book = 1;
    m->frame = 0.0f;
    ca_pages_free();
    ca_page_new(4, 3, 0);
    ca_page_new(5, 3, 1);
    ca_page_new(6, 3, 2);
}

/* gives the page the model of `kind`, with the cells without a card (or whose picture is not ready
   on the visible side) masked out */
void ca_page_set_kind(CaPage *p, int kind, float speed)
{
    CaKind *k = &ca_kinds[kind];
    Model *src = ca_model_get(k->model);
    Model *m;
    unsigned int mask;
    int i;
    int j;
    unsigned short id;

    if (p->page >= CA_PAGES(ca.n_cards) && k->model == CA_PAGE1)
        src = ca_model_get(CA_PAGE2);
    m = &p->model;
    memcpy(m, src, sizeof(Model));
    mask = 0;
    for (i = 0; i < CA_PER_PAGE; i++) {
        if (p->cards[i] == 0) {
            mask |= 1 << (i < 9 ? i + 11 : i + 12);
        } else if (kind == 1 && i < 9) {
            j = 2 - i % 3 + i / 3 * 3;   /* the cell facing it on the other side */
            id = p->cards[j + 9];
            if (id != 0 && cip_query(ca.cip_m, id, inv_get_hi(id)) == 0)
                mask |= 1 << (i + 11);
        } else if (kind == 2 && i >= 9) {
            j = 2 - i % 3 + i / 3 * 3;
            id = p->cards[j - 9];
            if (id != 0 && cip_query(ca.cip_m, id, inv_get_hi(id)) == 0)
                mask |= 1 << (i + 12);
        }
    }
    model_set_mask(m, mask);
    model_hook(m, 0, ca_page_tex_hook);
    p->model.frame = k->start;
    if (k->start == k->end)
        p->speed = 0.0f;
    else
        p->speed = speed;
    p->kind = kind;
}

/* the cover */
void ca_book_draw(void)
{
    ModelDraw d;
    float dt[4];
    Model *m;

    mtx_set_mode(2);
    memset(&d, 0, sizeof(d));
    d.layer[0] = 1;
    d.layer[1] = 1;
    mtx_identity();
    m = ca_model_get(CA_COVER);
    memset(dt, 0, sizeof(dt));
    if (ca.book != 0)
        dt[0] = 1.0f;
    model_update(m, dt);
    switch (ca.book) {
    case 0:
        m->frame = 0.0f;
        break;
    case 1:
        ca.book_x += 0.011071429f;
        if (m->frame >= 14.0f) {
            ca.book = 2;
            m->frame = 14.0f;
        }
        break;
    case 2:
        m->frame = 14.0f;
        break;
    case 3:
        ca.book_x -= 0.0096875f;
        if (m->frame >= 30.0f) {
            ca.book = 0;
            m->frame = 0.0f;
        }
        break;
    }
    model_draw(m, &d);
}

void ca_pages_draw(void)
{
    CaPage *p;

    for (p = ca.pages; p; p = p->next) {
        if (p->kind != 0 && p->kind != 7)
            ca_page_draw(p);
    }
    if (ca.state == 3)
        ca_cursor3d_draw(ca.cur.pos);
}

/* a page; when its animation passes the end of the kind's range it turns into the next kind */
void ca_page_draw(CaPage *p)
{
    Vec3 v;
    ModelDraw d;
    float dt[4];
    Model *m;
    CaKind *k;

    mtx_set_mode(2);
    memset(&d, 0, sizeof(d));
    d.layer[0] = 1;
    d.layer[1] = 1;
    d.user = p;
    mtx_identity();
    m = &p->model;
    memset(dt, 0, sizeof(dt));
    v.x = p->pos[0];
    v.y = p->pos[1];
    v.z = p->pos[2];
    mtx_translate(&v);
    dt[0] = p->speed;
    model_update(m, dt);
    model_draw(m, &d);
    if (p->speed > 0.0f) {
        k = &ca_kinds[p->kind];
        if (m->frame > k->end) {
            switch (p->kind) {
            case 4:
                ca_pages_shift_back();
                break;
            case 5:
                ca_pages_shift_fwd();
                break;
            }
            ca_page_set_kind(p, k->next, p->speed);
        }
    }
}

void ca_cursor3d_draw(int pos)
{
    Vec3 v;
    ModelDraw d;
    float dt[4];
    Model *m;

    mtx_set_mode(2);
    memset(&d, 0, sizeof(d));
    d.layer[0] = 1;
    d.layer[1] = 1;
    mtx_identity();
    m = ca_model_get(CA_CURSOR);
    m->flags |= 2;
    memset(dt, 0, sizeof(dt));
    v.x = ca_cells[pos].x;
    v.y = 0.0f;
    v.z = ca_cells[pos].z;
    mtx_translate(&v);
    dt[0] = 1.0f;
    model_update(m, dt);
    model_draw(m, &d);
}

void *ca_model_alloc(unsigned int size)
{
    return heap_alloc(ca.heap, size);
}

void ca_model_free(void *p)
{
    heap_set_level(ca.heap, p);
}

/* texture hook of the page models: textures 18..35 are the card cells; a card whose picture is still
   loading shows one of the two dummy cards, alternating every 4 frames */
int ca_page_tex_hook(unsigned int **list, int a1, int tex, int a3, ModelDraw *d)
{
    CaPage *p = d->user;
    unsigned int *l;
    unsigned short id;
    int loaded = 0;
    void *gim;

    if (tex < 18 || tex >= 36)
        return 0;
    id = p->cards[tex - 18];
    if (id == 0)
        return 0;
    l = *list;
    if (cip_query(ca.cip_m, id, inv_get_hi(id)) == 0)
        l = cip_get_image(l, ca.cip_m, id, inv_get_hi(id), &loaded);
    if (loaded == 0) {
        switch ((unsigned short)((gfx_get_state()->frame >> 2) % 3)) {
        case 0:
            gim = ca_res_get(0x100C);
            break;
        case 1:
            gim = ca_res_get(0x100D);
            break;
        default:
            return 0;
        }
        l = gim_set_texture(l, gim);
        l = dl_tex_func(l, 0, 1);
    } else {
        l = dl_tex_func(l, 0, 0);
    }
    l = dl_tex_offset(l, 0.0f, 1.0f);
    l = dl_tex_scale(1.0f, -1.0f, l);
    *list = l;
    return 1;
}
