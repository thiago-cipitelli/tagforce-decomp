// @range 0x0882AD90-0x0882B65C
// @data 0x088ED770-0x088ED7AC
/* Camera (position, target, up, view matrix) and the four GE lights kept with their enable bits,
   re-emitted into a display list on demand. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct Camera {                /* 0x089031F0 */
    Vec4 pos;
    Vec4 target;                       /* +0x10 */
    Vec4 up;                           /* +0x20 */
    Vec4 dir;                          /* +0x30 from the target to the eye */
    Mat4 view;                         /* +0x40 */
    float rot[12];                     /* +0x80 3x3 part of the view */
} Camera;

typedef struct Vec3 {
    float x, y, z;
} Vec3;

typedef struct Light {                 /* 0x3C */
    int type;                          /* 0 directional, 1 point, 2 spot */
    unsigned int ambient;              /* +0x04 */
    unsigned int diffuse;              /* +0x08 */
    unsigned int specular;             /* +0x0C */
    Vec3 pos;                          /* +0x10 */
    Vec3 dir;                          /* +0x1C */
    float spot_exp;                    /* +0x28 */
    float spot_cutoff;                 /* +0x2C */
    float att[3];                      /* +0x30 */
} Light;

typedef struct LightState {            /* 0x089032B0 */
    unsigned char lighting : 1;
    unsigned char on0 : 1;
    unsigned char on1 : 1;
    unsigned char on2 : 1;
    unsigned char on3 : 1;
    Light l[4];                        /* +0x04 */
    unsigned int ambient;              /* +0xF4 */
} LightState;

extern Camera *camera_ptr;             /* 0x089031E0 */
extern Camera camera_data;             /* 0x089031F0 */
extern LightState light_state;         /* 0x089032B0 */
extern void *(*tms_lookup)(char *name); /* 0x089033B0 resolves TMS texture names (ehp_tms_find) */

unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_light(unsigned int *list, int n, int type, int comp, float *pos);
unsigned int *dl_light_color(unsigned int *list, int n, int comp, unsigned int color);
unsigned int *dl_light_spot(float exponent, float cutoff, unsigned int *list, int n, float *dir);
unsigned int *dl_light_att(float a, float b, float c, unsigned int *list, int n);
unsigned int *dl_light_mode(unsigned int *list, int mode);
unsigned int *dl_ambient_light(unsigned int *list, unsigned int color);
Vec4 *vec3_sub_w(Vec4 *out, Vec4 *a, Vec4 *b);
void mat4_look_at(Mat4 *out, Vec4 *pos, Vec4 *dir, Vec4 *up);
void mat4_inverse_ortho(float *out, Mat4 *m);
void mtx_set_view_dir(Vec4 *v);

void camera_update(Camera *cam)
{
    cam->up.w = 0.0f;
    vec3_normalize2(&cam->up, &cam->up);
    cam->dir.w = 0.0f;
    vec3_sub_w(&cam->dir, &cam->pos, &cam->target);
    vec3_normalize2(&cam->dir, &cam->dir);
    mat4_look_at(&cam->view, &cam->pos, &cam->dir, &cam->up);
    mat4_inverse_ortho(cam->rot, &cam->view);
    mtx_set_view_dir(&cam->dir);
}

void camera_set(Vec4 *pos, Vec4 *target, Vec4 *up);
void camera_reset(void);

Vec4 camera_def_pos = { 0.0f, 0.0f, 1.0f, 1.0f };
Vec4 camera_def_target = { 0.0f, 0.0f, 0.0f, 1.0f };
Vec4 camera_def_up = { 0.0f, 1.0f, 0.0f, 0.0f };

void camera_reset(void)
{
    Vec4 pos;
    Vec4 target;
    Vec4 up;

    camera_ptr = &camera_data;
    up = camera_def_up;
    target = camera_def_target;
    pos = camera_def_pos;
    camera_set(&pos, &target, &up);
}

void camera_clear(void)
{
    camera_ptr = 0;
}

Camera *camera_get(void)
{
    return camera_ptr;
}

void camera_set(Vec4 *pos, Vec4 *target, Vec4 *up)
{
    if (pos != 0) {
        camera_ptr->pos = *pos;
    }
    if (target != 0) {
        camera_ptr->target = *target;
    }
    if (up != 0) {
        camera_ptr->up = *up;
    }
    camera_update(camera_ptr);
}

void camera_load_view(void)
{
    mtx_load(&camera_ptr->view);
}

unsigned int *light_reset(unsigned int *list);
void tms_set_lookup(void *(*fn)(char *name));

unsigned int *view_init(unsigned int *list)
{
    unsigned int *r;

    camera_reset();
    r = light_reset(list);
    tms_set_lookup(0);
    return r;
}

void view_term(void)
{
    camera_clear();
    tms_set_lookup(0);
}

/* Colors, position (and spot direction) and attenuation of light n. */
unsigned int *light_emit(unsigned int *list, int n, Light *l)
{
    list = dl_light_color(list, n, 1, l->ambient);
    list = dl_light_color(list, n, 2, l->diffuse);
    list = dl_light_color(list, n, 4, l->specular);
    switch (l->type) {
    case 0:
        list = dl_light(list, n, 0, 6, &l->pos.x);
        break;
    case 1:
        list = dl_light(list, n, 1, 6, &l->pos.x);
        break;
    case 2:
        list = dl_light(list, n, 2, 6, &l->pos.x);
        list = dl_light_spot(l->spot_exp, l->spot_cutoff, list, n, &l->dir.x);
        break;
    }
    list = dl_light_att(l->att[0], l->att[1], l->att[2], list, n);
    return list;
}

unsigned int *light_set(unsigned int *list, int n, Light *l)
{
    light_state.l[n] = *l;
    return light_emit(list, n, l);
}

unsigned int *lighting_enable(unsigned int *list)
{
    light_state.lighting = 1;
    return dl_enable(list, 10);
}

unsigned int *lighting_disable(unsigned int *list)
{
    light_state.lighting = 0;
    return dl_disable(list, 10);
}

unsigned int *light_enable(unsigned int *list, int n)
{
    switch (n) {
    case 0:
        light_state.on0 = 1;
        list = dl_enable(list, 0xB);
        break;
    case 1:
        light_state.on1 = 1;
        list = dl_enable(list, 0xC);
        break;
    case 2:
        light_state.on2 = 1;
        list = dl_enable(list, 0xD);
        break;
    case 3:
        light_state.on3 = 1;
        list = dl_enable(list, 0xE);
        break;
    }
    return list;
}

unsigned int *light_disable(unsigned int *list, int n)
{
    switch (n) {
    case 0:
        light_state.on0 = 0;
        list = dl_disable(list, 0xB);
        break;
    case 1:
        light_state.on1 = 0;
        list = dl_disable(list, 0xC);
        break;
    case 2:
        light_state.on2 = 0;
        list = dl_disable(list, 0xD);
        break;
    case 3:
        light_state.on3 = 0;
        list = dl_disable(list, 0xE);
        break;
    }
    return list;
}

unsigned int *light_set_ambient(unsigned int *list, unsigned int color)
{
    light_state.ambient = color;
    return dl_ambient_light(list, color);
}

/* Emits the whole light state. */
unsigned int *light_apply(unsigned int *list)
{
    int i;

    if (light_state.lighting) {
        list = dl_enable(list, 10);
    } else {
        list = dl_disable(list, 10);
    }
    if (light_state.on0) {
        list = dl_enable(list, 0xB);
    } else {
        list = dl_disable(list, 0xB);
    }
    if (light_state.on1) {
        list = dl_enable(list, 0xC);
    } else {
        list = dl_disable(list, 0xC);
    }
    if (light_state.on2) {
        list = dl_enable(list, 0xD);
    } else {
        list = dl_disable(list, 0xD);
    }
    if (light_state.on3) {
        list = dl_enable(list, 0xE);
    } else {
        list = dl_disable(list, 0xE);
    }
    for (i = 0; i < 4; i++) {
        list = light_emit(list, i, &light_state.l[i]);
    }
    list = dl_ambient_light(list, light_state.ambient);
    return dl_light_mode(list, 1);
}

/* Default lights: white directional lights along +z, all off; applied when a list is given. */
unsigned int *light_reset(unsigned int *list)
{
    Light l;
    int i;

    memset(&light_state, 0, 0xF8);
    l.type = 0;
    l.ambient = 0xFF000000;
    l.diffuse = 0xFF000000;
    l.specular = 0xFF000000;
    l.pos = (Vec3){ 0.0f, 0.0f, 1.0f };
    l.att[0] = 1.0f;
    l.att[1] = 0.0f;
    l.att[2] = 0.0f;
    for (i = 0; i < 4; i++) {
        light_state.l[i] = l;
    }
    light_state.ambient = 0xFF000000;
    if (list == 0) {
        return list;
    }
    list = light_apply(list);
    return list;
}

void tms_set_lookup(void *(*fn)(char *name))
{
    tms_lookup = fn;
}
