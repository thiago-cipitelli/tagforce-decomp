// @range 0x08805FF8-0x08806D4C
// @data 0x0880CC1C-0x0880CD20
/* 3D part of the machine: GE state, the two lights, the cameras (one per scene), the lid of the
   machine opening and closing, the cards flying into the slot and the card coming out (its face is
   rendered to a texture by the model's texture hook). */
#include "libconv_machine.h"

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

typedef struct Mat4 {
    float m[16];
} __attribute__((aligned(16))) Mat4;

typedef struct CmCam {         /* 0x30 */
    Vec3 tr;                   /* projection offset */
    float fov;                 /* degrees */
    float near;
    float far;
    int pad[2];
    Vec4 pos;                  /* +0x20 */
} CmCam;

extern Light cm_light0, cm_light1;

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
unsigned int *lighting_disable(unsigned int *list);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
unsigned int *card_draw(unsigned int *list, int x, int y, unsigned int id);
char *sceGeEdramGetAddr(void);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
void mtx_rotate_z(float a);
void model_set_allocator(void *(*alloc)(unsigned int size), void (*freefn)(void *p));
void model_update(Model *m, float *dt);
void model_draw(Model *m, ModelDraw *c);
void *model_hook(Model *m, int which, void *v);
int model_anim_frame_count(Model *m);
int heap_set_level(void *h, void *p);

void *cm_model_alloc(unsigned int size);
void cm_model_free(void *p);
int cm_card_tex_hook(unsigned int **list, int a1, int tex);
void cm_camera_set(int i);
void cm_meter_set(int v);

/* GE state of the screen, lights and projection; light positions copied from local templates (0x0880CC1C) */
void cm_gfx_init(void)
{
    unsigned int *list;
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
    model_set_allocator(cm_model_alloc, cm_model_free);
    memset(&cm_light0, 0, sizeof(cm_light0));
    cm_light0.type = 0;
    cm_light0.ambient = 0xFF000000;
    cm_light0.diffuse = 0xFFFFFFFF;
    cm_light0.specular = 0;
    {
        Vec3 p0 = { 0.0f, 2.0f, 3.0f };
        cm_light0.pos = *(Vec3 *)&p0;
    }
    memset(&cm_light1, 0, sizeof(cm_light1));
    cm_light1.type = 0;
    cm_light1.ambient = 0xFF000000;
    cm_light1.diffuse = 0xFF404040;
    cm_light1.specular = 0;
    {
        Vec3 p1 = { 0.0f, 0.0f, -5.0f };
        cm_light1.pos = *(Vec3 *)&p1;
    }
    list = lighting_enable(list);
    list = light_enable(list, 0);
    list = light_enable(list, 1);
    list = light_set_ambient(list, 0xFF000000);
    list = dl_material_colors(list, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    list = dl_color_material(list, 7);
    list = dl_specular_power(10.0f, list);
    list = dl_fog(50.0f, 100.0f, list, 0x404040);
    list = dl_disable(list, 7);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(0.17453294f, 1.7647059f, 30.0f, 300.0f);
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
    {
        Mat4 m;

        mat4_identity(&m);
        for (i = 0; i < 8; i++)
            list = dl_bone_matrix(list, i, m.m);
    }
    dl_set_list(list);
}

/* per frame: lit 3D layer with the main camera */
void cm_gfx_begin3d(void)
{
    unsigned int *list;

    list = dl_select_layer(2, 0);
    list = light_set(list, 0, &cm_light0);
    list = light_set(list, 1, &cm_light1);
    list = lighting_enable(list);
    list = light_enable(list, 0);
    list = light_enable(list, 1);
    list = light_set_ambient(list, 0xFF000000);
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
    cm_camera_set(0);
    mtx_set_mode(2);
    mtx_identity();
}

/* the lid: plays backwards while opening (cm.lid 1), forwards while closing (2) */
void cm_draw_lid(void)
{
    unsigned int *list;
    ModelDraw d;
    float dt[4];
    Model *m;

    list = dl_select_layer(2, 0);
    list = lighting_enable(list);
    dl_set_list(list);
    cm_camera_set(1);
    mtx_set_mode(2);
    memset(&d, 0, sizeof(d));
    d.layer[0] = 2;
    d.layer[1] = 2;
    mtx_identity();
    m = cm_model_get(0x1020);
    memset(dt, 0, sizeof(dt));
    switch (cm.lid) {
    case 1:
        dt[0] = -1.0f;
        break;
    case 2:
        dt[0] = 1.0f;
        break;
    }
    model_update(m, dt);
    switch (cm.lid) {
    case 1:
        if (m->frame < 0.0f) {
            m->frame = 0.0f;
            cm.lid = 0;
            cm.lid_done = 1;
        }
        break;
    case 2:
        if (m->frame >= model_anim_frame_count(m)) {
            m->frame = model_anim_frame_count(m);
            cm.lid = 0;
            cm.lid_done = 0;
        }
        break;
    }
    model_draw(m, &d);
}

/* a card flying into the slot, face down */
void cm_draw_card3d(CmCard *c)
{
    unsigned int *list;
    Vec3 v;
    ModelDraw d;
    Model *m;

    switch (c->state) {
    case 1:
        if (c->t < 0)
            return;
        break;
    default:
        return;
    }
    list = dl_select_layer(2, 0);
    list = lighting_disable(list);
    dl_set_list(list);
    cm_camera_set(2);
    mtx_set_mode(2);
    memset(&d, 0, sizeof(d));
    d.layer[0] = 2;
    d.layer[1] = 3;
    mtx_identity();
    v.x = c->x;
    v.y = c->y;
    v.z = c->z;
    mtx_translate(&v);
    if (c->state == 1)
        mtx_rotate_x(3.3091445f);
    m = cm_model_get(0x1021);
    model_hook(m, 0, 0);
    model_update(m, 0);
    model_draw(m, &d);
}

/* the card coming out, with its face rendered by cm_card_tex_hook */
void cm_draw_result3d(void)
{
    unsigned int *list;
    Vec3 v;
    ModelDraw d;
    Model *m;
    CmOut *o = &cm.out;

    if (o->shown == 0)
        return;
    list = dl_select_layer(2, 0);
    list = lighting_disable(list);
    dl_set_list(list);
    cm_camera_set(3);
    mtx_set_mode(2);
    memset(&d, 0, sizeof(d));
    d.layer[0] = 3;
    d.layer[1] = 3;
    mtx_identity();
    v.x = o->x;
    v.y = o->y;
    v.z = o->z;
    mtx_translate(&v);
    mtx_rotate_y(3.1415927f);
    mtx_rotate_x(3.1415927f * o->rx / 180.0f);
    mtx_rotate_z(3.1415927f * (180.0f + o->rz) / 180.0f);
    m = cm_model_get(0x1021);
    model_hook(m, 0, cm_card_tex_hook);
    model_update(m, 0);
    model_draw(m, &d);
}

void *cm_model_alloc(unsigned int size)
{
    return heap_alloc(cm.heap, size);
}

void cm_model_free(void *p)
{
    heap_set_level(cm.heap, p);
}

/* texture hook of the card model: face (0) is cm.result rendered to VRAM 0x160000, back (1) the
   same texture */
int cm_card_tex_hook(unsigned int **list, int a1, int tex)
{
    unsigned int *l;

    switch (tex) {
    case 1:
    case 0:
        l = dl_enable(*list, 9);
        l = dl_enable(l, 4);
        l = dl_tex_func(l, 3, 1);
        l = dl_blend_func(l, 0, 2, 3, 0, 0);
        l = dl_disable(l, 1);
        l = dl_ambient_color(l, 0xFFFFFFFF);
        l = dl_tex_flush(l);
        l = gfx_set_draw_target(l, 0x160000, 0xFFFFFFFF, 0x80, 0x100, 3);
        if (tex == 0)
            l = card_draw(l, 0, 0, cm.result);
        l = gfx_set_default_target(l);
        l = dl_tex_flush(l);
        l = dl_enable(l, 9);
        l = dl_tex_mode(l, 3, 0, 0, 0);
        l = dl_tex_image(l, 0, 0x80, 0x100, 0x80, sceGeEdramGetAddr() + 0x160000);
        l = dl_tex_func(l, 3, 0);
        l = dl_tex_wrap(l, 1, 1);
        l = dl_tex_offset(l, 0.0f, 0.6015625f);
        l = dl_tex_scale(0.84375f, -0.6015625f, l);
        *list = dl_enable(l, 1);
        return 1;
    }
    return 0;
}

/* cameras: 0 machine, 1 lid, 2 cards flying in, 3 card coming out */
CmCam cm_cams[4] = {
    { { 0.0f, 0.0f, 0.0f }, 10.0f, 30.0f, 300.0f, { 0, 0 }, { 0.0f, 100.0f, 0.0f, 1.0f } },
    { { 0.705f, 0.0f, 0.0f }, 10.0f, 30.0f, 300.0f, { 0, 0 }, { 0.0f, 276.0f, 16.0f, 1.0f } },
    { { 0.705f, 0.0f, 0.0f }, 20.0f, 15.0f, 150.0f, { 0, 0 }, { 0.0f, 276.0f, 16.0f, 1.0f } },
    { { -0.705f, 0.0f, 0.0f }, 10.0f, 30.0f, 300.0f, { 0, 0 }, { 0.0f, 276.0f, 0.0f, 1.0f } },
};
Vec4 cm_cam_target = { 0.0f, 0.0f, 0.0f, 1.0f };
Vec4 cm_cam_up = { 0.0f, 0.0f, -1.0f, 0.0f };

void cm_camera_set(int i)
{
    CmCam *c = &cm_cams[i];

    mtx_set_mode(0);
    mtx_identity();
    mtx_translate(&c->tr);
    mtx_perspective(3.1415927f * c->fov / 180.0f, 1.7647059f, c->near, c->far);
    mtx_set_mode(1);
    mtx_identity();
    {
        Vec4 target, up;
        up = cm_cam_up;
        target = cm_cam_target;
        camera_set(&c->pos, &target, &up);
    }
    camera_load_view();
}

/* the lid shuts at once */
void cm_lid_close(void)
{
    Model *m;

    cm_meter_set(0x65);
    m = cm_model_get(0x1020);
    if (m)
        m->frame = model_anim_frame_count(m);
}
