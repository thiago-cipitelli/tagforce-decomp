// @range 0x08818724-0x08818FFC
// @rodata 0x08838B80-0x08838B90
// @data 0x0883C88C-0x0883C8B0
/* Camera, light and render state of the field (field_cam): the GE state of a frame, the camera
   following the player, the light of the time of day, the shake command and a full-screen quad
   drawn over the scene. */
#include "libfield.h"

void *dl_offset(void *list, int x, int y);
void *dl_viewport(void *list, int cx, int cy, int w, int h);
void *dl_depth_range(void *list, int near, int far);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *dl_depth_func(void *list, int func);
void *dl_front_face(void *list, int cw);
void *dl_shade_model(void *list, int mode);
void *dl_color(void *list, unsigned int color);
void *dl_fog(float near, float far, void *list, unsigned int color);
void *dl_clear_color(void *list, unsigned int color);
void *dl_clear_stencil(void *list, int s);
void *dl_clear_depth(void *list, int z);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_scale3(Vec3 *s);
void camera_set(Vec4 *eye, Vec4 *target, Vec4 *up);
void camera_load_view(void);
Mat4 *camera_get(void);
void vec4_add(Vec4 *out, Vec4 *a, Vec4 *b);
void *light_set(void *list, int i, void *light);
void *light_set_ambient(void *list, unsigned int color);
void *lighting_enable(void *list);
void *light_disable(void *list, int i);
int fld_bg_id(void);
FieldStory *fld_story_get(void);
void fld_player_get_pos(Vec3 *out);

void fcam_shake_tick(void);
void *fcam_setup(void *list);
void *fcam_view(void *list);
void *fcam_light(void *list);
void *fld_dl_quad(void *list, void *verts);

const Vec4 fcam_eye = { 4.0f, 4.0f, 4.0f, 1.0f };

void fcam_init(void)
{
    memset(&field_cam, 0, sizeof(field_cam));
}

void fcam_nop(void)
{
}

void fcam_nop2(void)
{
}

void fcam_update(void)
{
    pad_get();
    fcam_shake_tick();
}

void fcam_begin(void)
{
    void *list = dl_select_layer(0, 0);

    list = fcam_setup(list);
    list = fcam_view(list);
    list = fcam_light(list);
    dl_set_list(list);
    mtx_set_mode(2);
    mtx_identity();
}

void *fcam_apply(void *list)
{
    list = fcam_setup(list);
    list = fcam_view(list);
    return fcam_light(list);
}

/* a quad over the whole screen in color (z: depth) */
void *fld_dl_clear(void *list, short z, unsigned int color)
{
    struct {
        unsigned int color;
        short x, y, z, pad;
    } *v = dl_alloc_inline(&list, 0x18);

    memset(v, 0, 0x18);
    v[0].x = 0;
    v[0].y = 0;
    v[0].z = z;
    v[0].color = color;
    v[1].x = 480;
    v[1].y = 272;
    v[1].z = z;
    v[1].color = color;
    list = fld_dl_quad(list, v);
    return list;
}

Vec4 *fld_light_dir(void)
{
    return &field_cam.light_dir;
}

void fld_light_set(int light)
{
    FieldCam *const c = &field_cam;

    field_cam.mode = light;
    switch (fld_bg_id()) {
    case 3:
    case 0x15:
    case 4:
    case 2:
    case 5:
        if (*(int *)fld_story_get()->w[3] >= 0x16)
            c->mode = 2;
        break;
    }
}

void fcam_cmd_shake(int *a)
{
    FieldShake *const s = &field_cam.shake;

    field_cam.shaking = 1;
    memset(s, 0, sizeof(*s));
    s->forever = a[0] == 0;
    s->power = a[1];
    s->frames = a[0];
}

int fcam_shake_done(void)
{
    return (field_cam.shaking != 0) ^ 1;
}

void fcam_cmd_stop(void)
{
    field_cam.shaking = 0;
    field_cam.x04 = 0;
}

int fcam_stop_done(void)
{
    return (field_cam.x04 != 0) ^ 1;
}

void *fcam_setup(void *list)
{
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
    mtx_set_mode(0);
    mtx_identity();
    mtx_ortho(-240.0f, 240.0f, -136.0f, 136.0f, -10000.0f, 10000.0f);
    {
        Vec3 s = { 64.0f, 64.0f, 64.0f };

        mtx_scale3(&s);
    }
    mtx_set_mode(1);
    mtx_identity();
    mtx_set_mode(2);
    mtx_identity();
    list = dl_clear_color(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    return dl_set_tex_filter(list, 1, 1);
}

void *fcam_view(void *list)
{
    FieldCam *const c = &field_cam;
    FieldShake *const sh = &field_cam.shake;
    Vec3 p;
    Vec4 eye;
    Vec4 target;
    Vec4 up = { 0.0f, 1.0f, 0.0f, 0.0f };
    Mat4 *m;

    mtx_set_mode(1);
    fld_player_get_pos(&p);
    target.x = p.x;
    eye.x = p.x + fcam_eye.x;
    target.y = p.y;
    eye.y = p.y + fcam_eye.y;
    eye.z = p.z + fcam_eye.z;
    eye.w = 1.0f;
    target.w = 1.0f;
    target.z = p.z;
    if (field_cam.shaking) {
        vec4_add(&eye, &eye, &sh->off);
        vec4_add(&target, &target, &sh->off);
    }
    camera_set(&eye, &target, &up);
    camera_load_view();
    m = camera_get();
    c->light_dir = ((Vec4 *)m)[3];
    mtx_set_mode(2);
    mtx_identity();
    return list;
}

void *fcam_light(void *list)
{
    int *const l = field_cam.light;
    unsigned char r, g, b;

    switch (field_cam.mode) {
    case 0:
        r = 0xFF;
        g = 0xFF;
        b = 0xFF;
        break;
    case 1:
        r = 0xFF;
        g = 200;
        b = 0x5A;
        break;
    case 2:
        r = 0x78;
        g = 0x8C;
        b = 0xFF;
        break;
    }
    memset(l, 0, sizeof(field_cam.light));
    list = light_set(list, 0, l);
    list = light_set_ambient(list, (r | g << 8) | b << 16 | 0xFF000000);
    list = lighting_enable(list);
    return light_disable(list, 0);
}

void fcam_shake_tick(void)
{
    FieldShake *const s = &field_cam.shake;
    FieldCam *const c = &field_cam;
    float k;

    if (s->forever != 0 || s->count < s->frames) {
        switch (s->power) {
        case 0:
            k = 0.005f;
            break;
        case 1:
            k = 0.01f;
            break;
        case 2:
            k = 0.02f;
            break;
        case 3:
            k = 0.04f;
            break;
        }
        s->off.x = k * ((int)(rand() & 0xFFFF) % 10);
        s->off.y = k * ((int)(rand() & 0xFFFF) % 10);
        s->off.z = k * ((int)(rand() & 0xFFFF) % 10);
        s->count++;
    } else {
        c->shaking = 0;
    }
}

void *fld_dl_quad(void *list, void *verts)
{
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_disable(list, 9);
    list = dl_disable(list, 1);
    list = dl_draw_array(list, 6, 0x80011C, 2, 0, verts);
    list = dl_enable(list, 9);
    return dl_enable(list, 1);
}
