// @range 0x0880720C-0x08807CD4
// @data 0x08874A90-0x08874AA8
/* The 3D field behind the cards: two background slots (fmagicNNNN.ehp model, animation and
   camera tracks) that cross-fade when the field spell changes, the camera, fog and lights set from
   the background's tracks, and the queries other parts make (view matrix, projection). */
#include "dd_d1.h"

void mtx_set_mode(int mode);
void mtx_identity(void);
void mtx_perspective(float fovy, float aspect, float near, float far);
void mtx_push(void);
void mtx_pop(void);
void mtx_get34(DdMat *m);
void mtx_get_view_dir(DdVec4 *out);
void camera_set(void *pos, void *target, void *up);
void camera_load_view(void);
unsigned int *dl_viewport(unsigned int *list, int cx, int cy, int w, int h);
unsigned int *lighting_disable(unsigned int *list);
unsigned int *lighting_enable(unsigned int *list);
void *dl_set_tex_filter(void *list, int min, int mag);
unsigned int *dl_shade_model(unsigned int *list, int mode);
unsigned int *dl_fog(float near, float far, unsigned int *list, unsigned int color);
unsigned int *light_set_ambient(unsigned int *list, unsigned int color);
unsigned int *light_enable(unsigned int *list, int n);
unsigned int *light_disable(unsigned int *list, int n);
unsigned int *light_set(unsigned int *list, int n, DdLight *l);
unsigned int *gfx_get_state(void);
float model_anim_frame(Model *m);
void model_update(Model *m, float *dt);
int model_anim_play(Model *m, int n, unsigned int flags);
void model_draw(void *model, void *params);
void req_flag_set(int i);
float keyframe_eval(void *track, float t);

/* other ranges of libduel_draw */
void dd_cam_from_tms(DdCam *cam, void *tms, float t);
void dd_cam_default(DdCam *cam);
void dd_field_on_clear(void);
void dd_field_on_load(void);
void dd_field_fx_start(DdFieldSlot *s, int id);
void dd_field_fx_stop(DdFieldSlot *s, int id);
void dd_field_slot_tick(DdFieldSlot *s, int id, int frame, int anim_frame, int loop);
void func_0884F3D0(int kind, int id, void (*cb)(void), int arg);
void func_08859E64(int a);

void dd_field_draw_slot(int i);
int dd_field_set(int id);

void dd_field_init(void)
{
    DdField *const f = &dd_field;

    memset(f, 0, sizeof(DdField));
    dd_field.next = -1;
    dd_field.slot[0].id = 0;
    f->slot[0].fx = -1;
    f->slot[1].fx = -1;
    dd_field.slot[0].state = 1;
}

void dd_field_free(void)
{
    DdField *f = &dd_field;
    int i;
    DdFieldSlot *s;

    for (i = 0, s = f->slot; i < 2; i++, s++) {
        if (s->model) {
            dd_field_fx_stop(s, s->id);
            model_free(s->model);
        }
    }
    memset(f, 0, sizeof(DdField));
}

/* draws both background slots (the current one first) on layer 1 */
void dd_field_draw(void)
{
    void *list = dl_select_layer(1, 0);

    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(0.38397199f, 480.0f / 272.0f, 0.1f, 10.0f);
    mtx_set_mode(1);
    camera_set(&(DdVec3){ 0.0f, 0.3f, 0.5f }, &(DdVec3){ 0 }, &(DdVec3){ 0.0f, 1.0f, 0.0f });
    camera_load_view();
    list = dl_viewport(list, 0x800, 0x800, 480, 272);
    list = lighting_disable(list);
    list = dl_set_tex_filter(list, 1, 1);
    dl_set_list(list);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    dd_field_draw_slot(dd_field.cur);
    dd_field_draw_slot(!dd_field.cur);
    mtx_pop();
    dl_set_list(dl_disable(dl_select_layer(1, 0), 7));
}

/* animates the slots and runs their state machine (load, show, wait for the switch, free) */
void dd_field_update(float dt)
{
    DdField *const f = &dd_field;
    float upd[4];
    int i;
    DdFieldSlot *s;
    int loop;
    DdFieldSlot *other;
    Model *m;
    int frame;

    memset(upd, 0, sizeof(upd));
    upd[0] = dt * (float)(int)gfx_get_state()[1];
    for (i = 0, s = f->slot; i < 2; i++, s++) {
        loop = 1;
        other = &f->slot[1 - i];
        m = s->model;
        if (m) {
            if (!(dt > 0.0f))
                loop = 0;
            frame = s->time * (float)(int)gfx_get_state()[1];
            dd_field_slot_tick(s, s->id, frame, model_anim_frame(m), loop);
            model_update(s->model, upd);
        }
        s->time += dt;
        switch (s->state) {
        case 0:
            if (f->next >= 0) {
                int id = f->next;

                f->next = -1;
                dd_field_set(id);
            }
            break;
        case 1:
            if (dd_bg_get(s->id) && other->state == 0) {
                dd_bg_filelist(s->id, (int *)&s->tms, (int *)&s->tmt, (int *)&s->tma, (int *)&s->cam);
                s->model = model_load(s->tms, 0);
                model_anim_set(s->model, s->tmt);
                model_set_tgma(s->model, s->tma);
                s->time = 0.0f;
                s->fresh = 1;
                s->state = 2;
                f->cur = i;
                f->cleared = 0;
                func_0884F3D0(0, s->id, dd_field_on_clear, 0);
                model_anim_play(s->model, 0, 2);
                upd[0] = 0.0f;
                model_update(s->model, upd);
            }
            req_flag_set(0);
            break;
        case 2:
            if (f->cleared)
                s->state = 3;
            req_flag_set(0);
            break;
        case 3:
            break;
        case 4:
            if (f->loaded) {
                dd_field_fx_stop(s, s->id);
                if (s->model) {
                    model_free(s->model);
                    s->model = 0;
                }
                dd_bg_release(s->id);
                s->tms = 0;
                s->tmt = 0;
                s->tma = 0;
                s->cam = 0;
                s->id = -1;
                s->state = 0;
            }
            req_flag_set(0);
            break;
        }
    }
}

/* switches to background `id` once the shown one can be released */
int dd_field_set(int id)
{
    DdField *f = &dd_field;
    int cur = dd_field.cur;
    DdFieldSlot *next = &f->slot[1 - cur];
    DdFieldSlot *s = &f->slot[cur];

    if (next->state != 0 || s->state != 3) {
        f->next = id;
        return 1;
    }
    next->id = id;
    next->state = 1;
    s->state = 4;
    f->loaded = 0;
    func_0884F3D0(1, id, dd_field_on_load, 0);
    req_flag_set(0);
    return 1;
}

int dd_field_cur_id(void)
{
    return dd_field.slot[dd_field.cur].id;
}

void dd_field_view_get(DdMat *m)
{
    DdField *f = &dd_field;

    if (m)
        *m = f->view;
}

void dd_field_proj_get(float *fov, float *aspect, float *near, float *far)
{
    DdField *f = &dd_field;

    if (fov)
        *fov = f->fov;
    if (aspect)
        *aspect = f->aspect;
    if (near)
        *near = f->near;
    if (far)
        *far = f->far;
}

void dd_field_dir_get(DdVec4 *out)
{
    DdField *f = &dd_field;

    if (out)
        *out = f->dir;
}

/* model of the shown background, 0 while it has no data */
Model *dd_field_model(void)
{
    DdFieldSlot *s = &dd_field.slot[dd_field.cur];
    Model *m = s->model;

    if (m == 0 || *(int *)m == 0)
        return 0;
    return m;
}

void *dd_field_camera(void *list, void *tms, int mode, float t);

void *dd_field_camera0(void *list, void *tms, float t)
{
    return dd_field_camera(list, tms, 0, t);
}

/* draws background slot i */
void dd_field_draw_slot(int i)
{
    DdField *f = &dd_field;
    DdFieldSlot *s = &f->slot[i];
    void *list;
    int params[13];

    switch (s->state) {
    case 0:
        return;
    case 1:
        return;
    case 2:
        if (s->fresh) {
            s->fresh = 0;
            dd_field_fx_start(s, s->id);
        }
        break;
    case 3:
        break;
    case 4:
        if (f->loaded)
            return;
        break;
    }
    list = dl_select_layer(1, 0);
    list = dd_field_camera(list, s->cam, f->mode, model_anim_frame(s->model));
    list = dl_set_tex_filter(list, 1, 1);
    list = dl_shade_model(list, 1);
    dl_set_list(list);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    memset(params, 0, sizeof(params));
    params[0] = 1;
    params[1] = 1;
    model_draw(s->model, params);
    mtx_pop();
}

/* projection, camera, fog and lights of the field from the background's tracks (or defaults) */
void *dd_field_camera(void *list, void *tms, int mode, float t)
{
    DdField *f = &dd_field;
    DdCam *cam = &dd_field.cam;
    int i;
    DdCam *c;

    if (tms == 0 || mode != 0)
        dd_cam_default(cam);
    else
        dd_cam_from_tms(cam, tms, t);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(cam->fov, 480.0f / 272.0f, cam->near, cam->far);
    mtx_set_mode(1);
    mtx_identity();
    camera_set(&cam->eye, &cam->at, &cam->up);
    camera_load_view();
    f->fov = cam->fov;
    f->aspect = 480.0f / 272.0f;
    f->near = cam->near;
    f->far = cam->far;
    mtx_get34(&f->view);
    mtx_get_view_dir(&f->dir);
    func_08859E64(0);
    if (cam->fog == 0) {
        list = dl_disable(list, 7);
    } else {
        list = dl_enable(list, 7);
        list = dl_fog(cam->fog_near, cam->fog_far, list, cam->fog_color);
    }
    list = lighting_enable(list);
    list = light_set_ambient(list, cam->ambient);
    for (i = 0, c = cam; i < 4; i++, c = (DdCam *)((int *)c + 1)) {
        if (cam->light_on[i] == 0) {
            list = light_disable(list, i);
        } else {
            list = light_enable(list, i);
            list = light_set(list, i, &cam->light[i]);
        }
    }
    return dl_viewport(list, 0x800, 0x800, 480, 272);
}

/* colour 0x00BBGGRR from the three tracks k, k+1, k+2 at time t */
unsigned int dd_track_rgb(void *tracks, int k, float t)
{
    float r = keyframe_eval((char *)tracks + k * 8, t);
    float g = keyframe_eval((char *)tracks + (k + 1) * 8, t);
    float b = keyframe_eval((char *)tracks + (k + 2) * 8, t);

    unsigned int c = (int)(255.0f * r) | (int)(255.0f * g) << 8;

    return c | (int)(255.0f * b) << 16;
}
