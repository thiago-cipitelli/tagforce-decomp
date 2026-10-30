// @range 0x08807CD4-0x08808D10
// @data 0x08874AB0-0x08874BA0
// @asm 0x08807CD4-0x08808210
/* Camera, fog and lights of the field: read from the tracks of the background's camera file
   (bone 0 the camera, bones 1..4 the lights) or the default set; the field spell effects that
   start with a background. */
#include "dd_d1.h"

float keyframe_eval(void *track, float t);
void anim_eval_track(DdXyzw *out, void *tms, void *tr, float t);
void quat_to_mat4(DdMtx *out, DdXyzw *q);
DdXyzw *mat4_transform_xyz(DdXyzw *out, DdMtx *m, DdXyzw *v);
DdXyzw *vec3_add_w(DdXyzw *out, DdXyzw *a, DdXyzw *b);
float atanf(float x);
float cosf_vfpu(float x);
unsigned int dd_track_rgb(void *tracks, int k, float t);

/* other ranges of libduel_draw */
int func_0884FD60(int id, int a, int b);
int func_0885036C(int id, int a, int b);
int func_088517DC(int id, int a, int b);
int func_088522F0(int id, int a, int b);
int func_08853674(int id, int a, int b);
int func_088546C4(int id, int a, int b);
int func_08854C44(int id, int a, int b);
void func_0882636C(int fx);

typedef struct DdTrack {               /* BOOT AnimTrack */
    int n;
    void *keys;
} DdTrack;

typedef struct DdTms {                 /* camera file of a background (TGMT layout) */
    unsigned int magic;
    unsigned short (*chan)[4];         /* +0x04 per bone: translation, rotation track */
    DdTrack *tracks;                   /* +0x08 */
    void *keys;
    DdTrack *rtracks;                  /* +0x10 */
} DdTms;

DdXyzw dd_axis_y = { 0.0f, 1.0f, 0.0f, 0.0f };
DdXyzw dd_axis_nz = { 0.0f, 0.0f, -1.0f, 0.0f };
DdXyzw dd_axis_z = { 0.0f, 0.0f, 1.0f, 0.0f };

#define TRACK(n) (&tms->tracks[n])

#ifdef NON_MATCHING /* forma igual; tms e o índice da luz trocam de $s e o original usa i*8 como indução para os canais */
/* camera of the field at time t from the background tracks */
void dd_fcam_from_tms(DdCam *cam, DdTms *tms, float t)
{
    float focal = keyframe_eval(TRACK(0), t);
    float film = keyframe_eval(TRACK(1), t);
    float n = keyframe_eval(TRACK(2), t);
    float far = keyframe_eval(TRACK(3), t);
    float near = n < 1.0f ? 1.0f : n;
    DdLight l;
    DdXyzw pos;
    DdXyzw q;
    DdMtx mat;
    DdXyzw dir;
    DdXyzw up;
    DdXyzw at;
    unsigned short *bone;
    DdTrack *tr;
    int fog;
    float fog_near;
    float fog_far;
    int i;
    int k;

    cam->fov = 2.0f * atanf(0.5f * film / (0.03937f * focal));
    cam->near = near;
    cam->far = far;
    bone = tms->chan[0];
    tr = TRACK(bone[0]);
    pos.x = keyframe_eval(tr, t);
    pos.y = keyframe_eval(tr + 1, t);
    pos.z = keyframe_eval(tr + 2, t);
    anim_eval_track(&q, tms, &tms->rtracks[bone[1]], t);
    quat_to_mat4(&mat, &q);
    mat4_transform_xyz(&up, &mat, &dd_axis_y);
    mat4_transform_xyz(&dir, &mat, &dd_axis_nz);
    vec3_add_w(&at, &pos, &dir);
    cam->eye = pos;
    cam->up = up;
    cam->at = at;
    fog = keyframe_eval(TRACK(4), t);
    fog_near = keyframe_eval(TRACK(8), t);
    fog_far = keyframe_eval(TRACK(9), t);
    cam->fog_color = dd_track_rgb(tms->tracks, 5, t);
    cam->fog = fog;
    cam->fog_near = fog_near;
    cam->fog_far = fog_far;
    cam->ambient = dd_track_rgb(tms->tracks, 10, t) | 0xFF000000;
    for (i = 0, k = 20; i < 4; i++, k += 16) {
        if ((cam->light_on[i] = keyframe_eval(TRACK(k + 1), t)) == 0)
            continue;
        l.type = keyframe_eval(TRACK(k), t);
        l.ambient = dd_track_rgb(tms->tracks, k + 2, t);
        l.diffuse = dd_track_rgb(tms->tracks, k + 5, t);
        l.specular = dd_track_rgb(tms->tracks, k + 8, t);
        l.atten[0] = keyframe_eval(TRACK(k + 11), t);
        l.atten[1] = keyframe_eval(TRACK(k + 12), t);
        l.atten[2] = keyframe_eval(TRACK(k + 13), t);
        bone = tms->chan[i + 1];
        tr = TRACK(bone[0]);
        pos.x = keyframe_eval(tr, t);
        pos.y = keyframe_eval(tr + 1, t);
        pos.z = keyframe_eval(tr + 2, t);
        anim_eval_track(&q, tms, &tms->rtracks[bone[1]], t);
        quat_to_mat4(&mat, &q);
        mat4_transform_xyz(&dir, &mat, &dd_axis_z);
        switch (l.type) {
        case 0:
            l.pos = *(DdXyz *)&dir;
            break;
        case 1:
            l.pos = *(DdXyz *)&pos;
            break;
        case 2:
            l.dir = *(DdXyz *)&dir;
            l.pos = *(DdXyz *)&pos;
            l.spot_exp = keyframe_eval(TRACK(k + 15), t);
            l.spot_cutoff = cosf_vfpu(keyframe_eval(TRACK(k + 14), t) / 2.0f);
            break;
        }
        cam->light[i] = l;
    }
}

#endif

#define VEC3(x, y, z) (*(DdXyzw *)&(DdXyz){ x, y, z })

unsigned int dd_rgb_from_vec(DdXyz *v);

/* default light of the field */
#define DD_LIGHT_DEFAULT(l, on_next)                                                   \
    (l)->type = 0;                                                                      \
    (l)->ambient = dd_rgb_from_vec(&(DdXyz){ 0 });                                    \
    (l)->diffuse = dd_rgb_from_vec(&(DdXyz){ 1.0f, 1.0f, 1.0f });                     \
    (l)->specular = dd_rgb_from_vec(&(DdXyz){ 0 });                                   \
    (l)->atten[0] = 0.5f;                                                               \
    (l)->atten[1] = 0.0f;                                                               \
    (l)->atten[2] = 0.0f;                                                               \
    switch ((l)->type) {                                                                \
    case 0:                                                                             \
        (l)->pos = (DdXyz){ 0.0f, 0.0f, 1.0f };                                        \
        break;                                                                          \
    case 1:                                                                             \
        (l)->pos = (DdXyz){ 0 };                                                       \
        break;                                                                          \
    case 2:                                                                             \
        (l)->dir = (DdXyz){ 0.0f, 0.0f, 1.0f };                                        \
        (l)->pos = (DdXyz){ 0 };                                                       \
        (l)->spot_exp = 0.0f;                                                           \
        (l)->spot_cutoff = 0.3f;                                                        \
        break;                                                                          \
    }

/* default camera: fixed view, white fog off, one directional light */
void dd_fcam_default(DdCam *cam)
{
    DdLight *l;

    cam->eye = VEC3(0.0f, 0.3f, 0.5f);
    cam->up = VEC3(0.0f, 1.0f, 0.0f);
    cam->at = VEC3(0, 0, 0);
    cam->fov = 0.38397199f;
    cam->near = 0.1f;
    cam->far = 10.0f;
    cam->fog = 0;
    cam->fog_near = 100.0f;
    cam->fog_far = 200.0f;
    cam->fog_color = dd_rgb_from_vec(&(DdXyz){ 1.0f, 1.0f, 1.0f });
    cam->ambient = dd_rgb_from_vec(&(DdXyz){ 0.3f, 0.3f, 0.3f });
    cam->ambient |= 0xFF000000;
    l = &cam->light[0];
    cam->light_on[0] = 1;
    DD_LIGHT_DEFAULT(l, 1)
    cam->light_on[1] = 0;
    l = &cam->light[1];
    DD_LIGHT_DEFAULT(l, 2)
    cam->light_on[2] = 0;
    l = &cam->light[2];
    DD_LIGHT_DEFAULT(l, 3)
    cam->light_on[3] = 0;
    l = &cam->light[3];
    DD_LIGHT_DEFAULT(l, 4)
}

/* colour 0x00BBGGRR from a vector of three 0..1 floats */
unsigned int dd_rgb_from_vec(DdXyz *v)
{
    unsigned int c = (unsigned int)(255.0f * v->x);

    c |= (unsigned int)(255.0f * v->y) << 8;
    c |= (unsigned int)(255.0f * v->z) << 16;
    return c;
}

/* transition callbacks of the background switch */
void dd_field_on_clear(void)
{
    dd_field.cleared = 1;
}

void dd_field_on_load(void)
{
    dd_field.loaded = 1;
    dl_set_list(dl_clear(dl_clear_stencil(dl_clear_depth(dl_clear_color(dl_select_layer(2, 0), 0xFF000000), 0), 0), 0x17));
}

/* starts the effect of the field spell `id` shown with the background */
void dd_field_fx_start(DdFieldSlot *s, int id)
{
    switch (id) {
    case 0x1323:
        s->fx = func_0884FD60(id, 0, 0);
        break;
    case 0x1345:
        s->fx = func_0885036C(id, 0, 0);
        break;
    case 0x1346:
        s->fx = func_088517DC(id, 0, 0);
        break;
    case 0x1349:
        s->fx = func_088522F0(id, 0, 0);
        break;
    case 0x169F:
        s->fx = func_08853674(id, 0, 0);
        break;
    case 0x1A67:
        s->fx = func_088546C4(id, 0, 0);
        break;
    case 0x1AA7:
        s->fx = func_08854C44(id, 0, 0);
        break;
    }
}

void dd_field_fx_stop(DdFieldSlot *s, int id)
{
    if (s->fx != -1) {
        func_0882636C(s->fx);
        s->fx = -1;
    }
}

void dd_field_slot_tick(DdFieldSlot *s, int id, int frame, int anim_frame, int loop)
{
}
