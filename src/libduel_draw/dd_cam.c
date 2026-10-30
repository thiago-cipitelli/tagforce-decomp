// @range 0x08826E30-0x088279A4
// @data 0x088764F0-0x08876508
/* Camera and matrix helpers of the duel effects: perspective camera on a layer, viewport and
   offset, VRAM addresses of the frame buffers, billboard and model-view-projection matrices from
   the GE stacks, the duel camera or the field camera, and save/restore of the four stacks. */
#include "dd_d3.h"

void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_load(float *m);
void mtx_get34(float *m);
void mtx_get_proj(float *fovy, float *aspect, float *near, float *far);
void mtx_perspective(float fovy, float aspect, float near, float far);
void mtx_look_at(float *eye, float *target, float *up);
void mtx_scale3(float *v);
void mtx_translate(float *v);
float *mat4_transpose(float *out, float *m);
float *mat4_copy(float *out, float *m);
float *mat4_mul(float *out, float *a, float *b);
float *mat4_identity(float *out);
float *mat4_mul_rot_x(float *out, float *m, float a);
float *mat3_identity(float *out);
float *mat3_rot_x(float *out, float *m, float a);
float *mat3_mul(float *out, float *a, float *b);
float *vec4_set(float *out, float x, float y, float z, float w);
unsigned int *dl_viewport(unsigned int *list, int cx, int cy, int w, int h);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *dl_offset(unsigned int *list, int x, int y);
void *sceGeEdramGetAddr(void);
unsigned int *gfx_get_state(void);
float *dd_chara_view(void);            /* duel camera projection */
float *dd_chara_proj(void);            /* duel camera view */
void func_08807808(float *m);          /* field camera view */
void func_08807844(float *fovy, float *aspect, float *near, float *far);

extern float dd_proj_saved[4];         /* projection replaced by dd_proj_set */
extern float dd_mtx_saved[4][16];      /* projection, view, texture, model (dd_mtx_save) */
extern float dd_proj_params[4];        /* perspective of dd_mtx_save */

void dd_cam_begin(int layer, int ox, int oy, float fovy, float aspect, float near, float far, float *eye, float *target, float *up);
void dd_mtx_strip(float *out, float *m);
void dd_mtx_mvp(float *out, float *view, float *proj);

void dd_cam_default(int layer, int ox, int oy)
{
    float eye[3] = { 0.0f, 0.0f, 500.0f };
    float target[3] = { 0 };
    float up[3] = { 0.0f, 1.0f, 0.0f };

    dd_cam_begin(layer, ox, oy, 1.2397240400f, 1.7647058964f, 100.0f, 10000.0f, eye, target, up);
}

void dd_cam_begin(int layer, int ox, int oy, float fovy, float aspect, float near, float far, float *eye, float *target, float *up)
{
    unsigned int *list = dl_select_layer(layer, 0);

    mtx_set_mode(0);
    mtx_push();
    mtx_identity();
    mtx_perspective(fovy, aspect, near, far);
    mtx_set_mode(1);
    mtx_push();
    mtx_identity();
    mtx_look_at(eye, target, up);
    list = dl_viewport(list, ox + 0x710, oy + 0x778, 480, 272);
    dl_set_list(list);
}

void dd_cam_end(int layer)
{
    unsigned int *list = dl_select_layer(layer, 0);

    mtx_set_mode(0);
    mtx_pop();
    mtx_set_mode(1);
    mtx_pop();
    if (layer == 4)
        list = dl_viewport(list, 0x84A, 0x7FC, 960, 544);
    else
        list = dl_viewport(list, 0x800, 0x800, 480, 272);
    dl_set_list(list);
}

void dd_clear_stencil(int layer, unsigned int s)
{
    unsigned int *list = dl_select_layer(layer, 0);

    list = dl_clear_stencil(list, s);
    list = dl_clear(list, 0x14);
    dl_set_list(list);
}

void dd_set_offset(int layer, int x, int y)
{
    unsigned int *list = dl_select_layer(layer, 0);

    list = dl_offset(list, x, y);
    dl_set_list(list);
}

void *dd_fb_addr(int back)
{
    unsigned char *vram = sceGeEdramGetAddr();

    if (back == 1)
        return vram + ((*gfx_get_state() & 1) ? 0 : 0x88000);
    return vram + ((*gfx_get_state() & 1) ? 0x88000 : 0);
}

void *dd_vram_addr(int offset)
{
    return (unsigned char *)sceGeEdramGetAddr() + offset;
}

void dd_mtx_billboard(float *out, float *m)
{
    float t[16];

    mat4_transpose(t, m);
    dd_mtx_strip(t, t);
    mat4_copy(out, t);
}

void dd_mtx_billboard_view(float *out)
{
    float m[16];

    mtx_set_mode(1);
    mtx_get34(m);
    dd_mtx_billboard(out, m);
    mtx_set_mode(2);
}

void dd_mtx_billboard_cam(float *out)
{
    dd_mtx_billboard(out, dd_chara_proj());
}

void dd_mtx_billboard_field(float *out)
{
    float m[16];

    func_08807808(m);
    dd_mtx_billboard(out, m);
}

void dd_scale3(float x, float y, float z)
{
    float v[3] = { x, y, z };

    mtx_scale3(v);
}

void dd_translate3(float x, float y, float z)
{
    float v[3] = { x, y, z };

    mtx_translate(v);
}

void dd_mtx_mvp(float *out, float *view, float *proj)
{
    float model[16];
    float r[16];

    mtx_set_mode(2);
    mtx_get34(model);
    mat4_mul(r, view, model);
    mat4_mul(r, proj, r);
    mat4_copy(out, r);
}

void dd_mtx_mvp_cur(float *out)
{
    float view[16];
    float proj[16];

    mtx_set_mode(1);
    mtx_get34(view);
    mtx_set_mode(0);
    mtx_get34(proj);
    dd_mtx_mvp(out, view, proj);
}

void dd_mtx_mvp_cam(float *out)
{
    float view[16];
    float proj[16];

    mat4_copy(view, dd_chara_proj());
    mat4_copy(proj, dd_chara_view());
    dd_mtx_mvp(out, view, proj);
}

void dd_proj_set(float fovy, float aspect, float near, float far)
{
    float *const save = dd_proj_saved;
    float *const cur = dd_proj_params;
    float f = cur[0], a = cur[1], n = cur[2], r = cur[3];

    save[0] = f;
    save[1] = a;
    save[2] = n;
    save[3] = r;
    if (fovy != 0.0f)
        f = fovy;
    if (aspect != 0.0f)
        a = aspect;
    if (near != 0.0f)
        n = near;
    if (far != 0.0f)
        r = far;
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(f, a, n, r);
    mtx_set_mode(2);
}

void dd_proj_restore(void)
{
    float *const p = dd_proj_saved;

    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(p[0], p[1], p[2], p[3]);
    mtx_set_mode(2);
}

void dd_mtx_strip(float *out, float *m)
{
    float t[16];

    mat4_copy(t, m);
    vec4_set(&t[12], 0.0f, 0.0f, 0.0f, 1.0f);
    t[3] = 0.0f;
    t[7] = 0.0f;
    t[11] = 0.0f;
    mat4_copy(out, t);
}

void dd_mtx_push_identity(void)
{
    mtx_set_mode(0);
    mtx_push();
    mtx_identity();
    mtx_set_mode(1);
    mtx_push();
    mtx_identity();
    mtx_set_mode(3);
    mtx_push();
    mtx_identity();
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
}

void dd_mtx_pop_all(void)
{
    mtx_set_mode(0);
    mtx_pop();
    mtx_set_mode(1);
    mtx_pop();
    mtx_set_mode(3);
    mtx_pop();
    mtx_set_mode(2);
    mtx_pop();
}

void dd_mtx_save(void)
{
    mtx_set_mode(0);
    mtx_get34(dd_mtx_saved[0]);
    mtx_set_mode(1);
    mtx_get34(dd_mtx_saved[1]);
    mtx_set_mode(3);
    mtx_get34(dd_mtx_saved[2]);
    mtx_set_mode(2);
    mtx_get34(dd_mtx_saved[3]);
    {
        float *const p = dd_proj_params;
        mtx_get_proj(&p[0], &p[1], &p[2], &p[3]);
    }
}

void dd_mtx_push_saved(void)
{
    mtx_set_mode(0);
    mtx_push();
    mtx_load(dd_mtx_saved[0]);
    mtx_set_mode(1);
    mtx_push();
    mtx_load(dd_mtx_saved[1]);
    mtx_set_mode(3);
    mtx_push();
    mtx_load(dd_mtx_saved[2]);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(dd_mtx_saved[3]);
}

void dd_mtx_push_cam(void)
{
    mtx_set_mode(0);
    mtx_push();
    mtx_load(dd_chara_view());
    mtx_set_mode(1);
    mtx_push();
    mtx_load(dd_chara_proj());
    mtx_set_mode(3);
    mtx_push();
    mtx_identity();
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
}

void dd_mtx_push_field(void)
{
    float fovy, aspect, near, far;
    float view[16];

    mtx_set_mode(0);
    mtx_push();
    mtx_identity();
    func_08807844(&fovy, &aspect, &near, &far);
    mtx_perspective(fovy, aspect, near, far);
    mtx_set_mode(1);
    mtx_push();
    func_08807808(view);
    mtx_load(view);
    mtx_set_mode(3);
    mtx_push();
    mtx_identity();
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
}

void dd_mat3_rot_x(float *out, float *m, float a)
{
    float r[9];

    mat3_identity(r);
    mat3_rot_x(r, r, a);
    mat3_mul(out, m, r);
}

void dd_mat4_rot_x(float *out, float *m, float a)
{
    float r[16];

    mat4_identity(r);
    mat4_mul_rot_x(r, r, a);
    mat4_mul(out, m, r);
}
