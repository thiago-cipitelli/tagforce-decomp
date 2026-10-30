// @range 0x0884AE24-0x0884BA1C
// @asm 0x0884AE24-0x0884B260
// @asm 0x0884B260-0x0884BA1C
/* Parts of the card effect "aura" (dd_fx_aura task): a pulsing yellow and blue halo around the
   character's bone 38 while it is the attacking monster, and a spell circle of 24 blades
   (lens_aP8A.gim) on bone 40/41 started by the animation events 8-10 of the character. */
#include "libduel_draw.h"

void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void *dl_ambient_light(void *list, unsigned int color);
void *draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_load(FxMat4 *m);
void mtx_translate(FxVec3 *v);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
FxMat4 *mat4_translate(FxMat4 *out, FxMat4 *m, FxVec4 *v);
FxVec4 *mat4_transform_vec4(FxVec4 *out, FxMat4 *m, FxVec4 *v);
FxVec3 *vec3_set(FxVec3 *out, float x, float y, float z);
float sinf(float x);
float cosf(float x);

float dd_ease_lin(int t, int len);
float dd_ease_cos(int t, int len, int n);
void dd_mtx_push_cam(void);
void dd_mtx_pop_all(void);
void dd_mtx_strip(FxMat4 *out, FxMat4 *m);
int dd_cue_se(int se, int t, int at, unsigned int *flags, unsigned int bit);
int func_08813FC4(int who, int ev);
void *dd_depth_enable(void *list, int on, int mask);
void *dd_blend_enable(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dd_tex_pack(void *list, int tex);
void *dd_tex_enable(void *list, int on, int min, int mag, int func, int comp, int wrapu, int wrapv);
void dd_chara_bone_mtx(void *out, int bone);
int dd_chara_get_08813FBC(void);

#ifdef NON_MATCHING /* o original mantém a = 255 num $s em vez de dobrar a constante e materializa a condição do id em v0; restante próximo */
int dd_fx_aura_halo(int phase, DdFxHalo *w, DdFxArg *arg)
{
    int run = arg->run;
    void *list;
    float s;
    float r;
    float ang;
    int a;
    int i;

    if (phase == 1) {
        int on = arg->id < 247 || arg->id >= 290;

        if (on)
            w->alpha = 1.0f;
        else
            w->alpha = 0.0f;
        switch (((int *)arg)[2]) {
        case 12:
        case 13:
        case 15:
        case 16:
        case 28:
            w->visible = 0;
            break;
        default:
            w->visible = 1;
            break;
        }
    }
    if (w->visible != 0) {
        dd_mtx_push_cam();
        dd_chara_bone_mtx(&w->mtx, 38);
        if (phase == 1) {
            switch (((int *)arg)[2]) {
            case 9:
                s = 0.8f;
                break;
            default:
                s = 1.0f;
                break;
            }
            a = 255;
            r = 5.0f * s;
            w->yellow[0].color = a << 24 | 0xFFFF00;
            w->yellow[0].x = 0.0f;
            w->yellow[0].y = 0.0f;
            w->yellow[0].z = 0.0f;
            for (i = 0; i < 17; i++) {
                ang = 6.2831855f * (float)i / 16.0f;
                w->yellow[i + 1].color = 0xFFFF00;
                w->yellow[i + 1].x = r * sinf(ang);
                w->yellow[i + 1].y = -0.25f;
                w->yellow[i + 1].z = r * cosf(ang);
            }
            w->blue[0].color = a << 24 | 0xFF;
            w->blue[0].x = 0.0f;
            w->blue[0].y = 0.0f;
            w->blue[0].z = 0.0f;
            for (i = 0; i < 17; i++) {
                ang = 6.2831855f * (float)i / 16.0f;
                w->blue[i + 1].color = 0xFF;
                w->blue[i + 1].x = r * sinf(ang);
                w->blue[i + 1].y = -0.25f;
                w->blue[i + 1].z = r * cosf(ang);
            }
        }
        list = dl_select_layer(7, 0);
        list = dd_tex_enable(list, 0, 1, 1, 0, 1, 1, 1);
        list = dd_depth_enable(list, 1, 1);
        mtx_set_mode(2);
        mtx_push();
        mtx_load(&w->mtx);
        list = dl_ambient_light(list, (unsigned int)(w->alpha * (sinf((float)w->frame * 0.08726647f) * 0.2f + 0.8f) * 255.0f) << 24 | 0xFFFFFF);
        list = dd_blend_enable(list, 1, 2, 2, 10, 0, 0xFFFFFFFF);
        list = draw_array(list, 5, 0x19C, 18, 0, w->yellow);
        list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
        list = draw_array(list, 5, 0x19C, 18, 0, w->blue);
        mtx_pop();
        dl_set_list(list);
        dd_mtx_pop_all();
        if (dd_chara_get_08813FBC() == 1)
            w->alpha = 1.0f;
        else
            w->alpha = 0.0f;
    }
    w->frame++;
    if (run == 1)
        w->t++;
    return 2;
}
#endif

#ifdef NON_MATCHING /* função de 2 KB: escalonamento das cópias de vértices e das contas das lâminas diferente; C equivalente */
int dd_fx_aura_circle(int phase, DdFxCircle *w, DdFxArg *arg)
{
    int run = arg->run;
    void *list;
    float in;
    float k;
    float ang;
    float d;
    unsigned int c;
    FxVec4 up = { 0.0f, 1.0f, 0.0f, 1.0f };
    FxMat4 rot;
    FxVec3 p;
    int go;
    int q;
    int i;

    if (phase == 1) {
        if (arg->id == 51)
            w->bone = 41;
        else if (arg->id >= 91 && arg->id < 102)
            w->bone = 40;
        else if (arg->id >= 118 && arg->id < 129)
            w->bone = 40;
        else
            w->bone = -1;
        switch (arg->id) {
        case 100:
        case 101:
        case 127:
        case 128:
            w->bone = -1;
            break;
        }
    }
    if (w->bone == -1)
        return 2;
    if (w->started == 0) {
        go = 0;
        if (w->bone == 41) {
            if (func_08813FC4(((int *)arg)[5], 8) == 1)
                go = 1;
        } else if (func_08813FC4(((int *)arg)[5], 9) == 1) {
            go = 1;
        } else if (func_08813FC4(((int *)arg)[5], 10) == 1) {
            go = 1;
        }
        phase = 1;
        if (go != 1)
            return 2;
        w->started = 1;
    }
    dd_cue_se(0x6227, w->t, 0, &w->se, 1);
    dd_mtx_push_cam();
    dd_chara_bone_mtx(&w->mtx, w->bone);
    dd_mtx_strip(&rot, &w->mtx);
    mat4_transform_vec4(&up, &rot, &up);
    mat4_translate(&w->mtx, &w->mtx, &up);
    in = dd_ease_lin(w->t, 4) * 0.5f + 0.5f;
    k = dd_ease_cos(w->t, 16, 1);
    if (phase == 1) {
        w->ring[0].u = 0.5f; w->ring[0].v = 0.5f; w->ring[0].color = 0xFF00AAFF;
        w->ring[0].x = 0.0f; w->ring[0].y = 0.09555846f; w->ring[0].z = 0.0f;
        w->ring[1].u = 0.0f; w->ring[1].v = 0.0f; w->ring[1].color = 0xAAFF;
        w->ring[1].x = -15.0f; w->ring[1].y = 0.09555846f; w->ring[1].z = 15.0f;
        w->ring[2].u = 1.0f; w->ring[2].v = 0.0f; w->ring[2].color = 0xAAFF;
        w->ring[2].x = 15.0f; w->ring[2].y = 0.09555846f; w->ring[2].z = 15.0f;
        w->ring[3].u = 1.0f; w->ring[3].v = 1.0f; w->ring[3].color = 0xAAFF;
        w->ring[3].x = 15.0f; w->ring[3].y = 0.09555846f; w->ring[3].z = -15.0f;
        w->ring[4].u = 0.0f; w->ring[4].v = 1.0f; w->ring[4].color = 0xAAFF;
        w->ring[4].x = -15.0f; w->ring[4].y = 0.09555846f; w->ring[4].z = -15.0f;
        w->ring[5] = w->ring[1];
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 6, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 3);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&w->mtx);
    c = (unsigned int)(in * 255.0f * k) << 24 | 0xFFFFFF;
    list = dl_ambient_light(list, c);
    mtx_push();
    mtx_rotate_y((float)w->t * 0.017453292f);
    list = draw_array(list, 5, 0x19F, 6, 0, w->ring);
    mtx_pop();
    mtx_pop();
    dl_set_list(list);
    if (phase == 1) {
        w->blade[0].u = 0.5f; w->blade[0].v = 0.5f; w->blade[0].color = 0xC800AAFF;
        w->blade[0].x = 0.0f; w->blade[0].y = 0.0f; w->blade[0].z = 0.0f;
        w->blade[1].u = 0.0f; w->blade[1].v = 0.0f; w->blade[1].color = 0xAAFF;
        w->blade[1].x = -3.0f; w->blade[1].y = 0.0f; w->blade[1].z = -14.4f;
        w->blade[2].u = 1.0f; w->blade[2].v = 0.0f; w->blade[2].color = 0xAAFF;
        w->blade[2].x = 6.0f; w->blade[2].y = 0.0f; w->blade[2].z = -14.4f;
        w->blade[3].u = 1.0f; w->blade[3].v = 1.0f; w->blade[3].color = 0xAAFF;
        w->blade[3].x = 6.0f; w->blade[3].y = 0.0f; w->blade[3].z = 3.6f;
        w->blade[4].u = 0.0f; w->blade[4].v = 1.0f; w->blade[4].color = 0xAAFF;
        w->blade[4].x = -3.0f; w->blade[4].y = 0.0f; w->blade[4].z = 3.6f;
        w->blade[5].u = 0.0f; w->blade[5].v = 0.0f; w->blade[5].color = 0xAAFF;
        w->blade[5].x = -3.0f; w->blade[5].y = 0.0f; w->blade[5].z = -14.4f;
    }
    list = dl_select_layer(7, 0);
    list = dd_tex_enable(list, 1, 1, 1, 0, 1, 1, 1);
    list = dd_depth_enable(list, 1, 1);
    list = dd_blend_enable(list, 1, 0, 2, 10, 0, 0xFFFFFFFF);
    list = dd_tex_pack(list, 3);
    mtx_set_mode(2);
    mtx_push();
    mtx_load(&w->mtx);
    list = dl_ambient_light(list, c);
    for (i = 0; i < 24; i++) {
        q = i % 4;
        ang = (float)(i / 4) * 6.2831855f / 6.0f + -0.08726647f + (float)q * 0.62831855f / 4.0f +
              ((float)(3 - q) * 0.0017453294f + 0.008726646f) * (float)w->t;
        d = ((float)q * 0.7f / 3.0f + 0.3f) * -4.777923f;
        vec3_set(&p, d * sinf(ang), 0.09555846f, d * cosf(ang));
        mtx_push();
        mtx_translate(&p);
        mtx_rotate_y(ang);
        mtx_rotate_x(0.34906587f);
        list = draw_array(list, 5, 0x19F, 6, 0, w->blade);
        mtx_pop();
    }
    mtx_pop();
    dl_set_list(list);
    dd_mtx_pop_all();
    w->frame++;
    if (run == 1 && w->t < 16)
        w->t++;
    return 2;
}
#endif
