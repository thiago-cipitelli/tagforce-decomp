// @range 0x088279A4-0x08827F68
// @asm 0x08827BA4-0x08827D64
/* Small helpers of the effect scripts: easing curves of a frame counter (linear, sine, cosine,
   their complements and their "last n frames" variants), a random direction, a clamped step,
   a random percentage, model animation helpers and positions read from the duel scene. */
#include "dd_d3.h"

typedef struct DdRandSeed {            /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;                  /* x * 0x41C64E6D + 0x3039 */
    unsigned int b, c;
} DdRandSeed;

extern int rand3_slot;
extern DdRandSeed rand3_seeds[];

float sinf(float x);
float cosf(float x);
float *vec3_set(float *out, float x, float y, float z);
float *vec3_normalize(float *out, float *v);
float *vec3_scale(float *out, float *v, float s);
float *vec3_copy(float *out, float *v);
int model_anim_play(void *m, int n, unsigned int flags);
void model_update(void *m, float *dt);
float model_anim_frame(void *m);
int model_anim_frame_count(void *m);
unsigned int *func_0881DE48(unsigned int *list, int arg, float f);
void func_0884DAE4(float *m, int id);
void func_0884DAF8(float *m, int id);

#define DD_RAND(r) ((r)->lcg *= 0x41C64E6D, (r)->lcg += 0x3039, (int)((r)->lcg >> 16 & 0x7FFF))

float dd_ease_lin(int t, int n)
{
    if (t < 0)
        t = 0;
    if (n < t)
        t = n;
    return (float)t / (float)n;
}

float dd_ease_lin_inv(int t, int n)
{
    return 1.0f - dd_ease_lin(t, n);
}

float dd_ease_lin_out(int t, int n, int end)
{
    return 1.0f - dd_ease_lin(t - (end - n), n);
}

float dd_ease_sin(int t, int n, int k)
{
    float v;
    int i;

    if (t < 0)
        t = 0;
    if (n < t)
        t = n;
    v = (float)t / (float)n;
    for (i = 0; i < k; i++)
        v = sinf(1.5707964f * v);
    return v;
}

float dd_ease_sin_inv(int t, int n, int k)
{
    return 1.0f - dd_ease_sin(t, n, k);
}

float dd_ease_cos(int t, int n, int k)
{
    float v;
    int i;

    if (t < 0)
        t = 0;
    if (n < t)
        t = n;
    v = 1.0f - (float)t / (float)n;
    for (i = 0; i < k; i++)
        v = cosf(1.5707964f * (1.0f - v));
    return v;
}

float dd_ease_cos_inv(int t, int n, int k)
{
    return 1.0f - dd_ease_cos(t, n, k);
}

float dd_ease_cos_out(int t, int n, int end, int k)
{
    return dd_ease_cos(t - (end - n), n, k);
}

#ifdef NON_MATCHING /* agendamento: o original divide o 2º sorteio logo e reaproveita a constante 32767 num registrador */
void dd_rand_dir(float *out, float len)
{
    DdRandSeed *r = &rand3_seeds[rand3_slot];
    float v[3];

    vec3_set(v, DD_RAND(r) / 32767.0f * (DD_RAND(r) % 2 * 2 - 1),
             DD_RAND(r) / 32767.0f * (DD_RAND(r) % 2 * 2 - 1),
             DD_RAND(r) / 32767.0f * (DD_RAND(r) % 2 * 2 - 1));
    vec3_normalize(v, v);
    vec3_scale(v, v, len);
    vec3_copy(out, v);
}
#endif

int dd_step_clamp(int v, int d, int lim)
{
    int r = v + d;

    if (d >= 0) {
        if (lim < r)
            r = lim;
    } else {
        if (r < lim)
            r = lim;
    }
    return r;
}

int dd_rand_percent(int p)
{
    DdRandSeed *r = &rand3_seeds[rand3_slot];

    return DD_RAND(r) % 100 < p;
}

void dd_layer_draw_f(int layer, int arg, float f)
{
    unsigned int *list = dl_select_layer(layer, 0);

    list = func_0881DE48(list, arg, f);
    dl_set_list(list);
}

int dd_model_anim_done(void *m)
{
    float n = model_anim_frame_count(m);

    return model_anim_frame(m) >= n;
}

void dd_model_rewind(void *m)
{
    float dt[4];

    dt[0] = 0.0f;
    dt[1] = 0.0f;
    dt[2] = 0.0f;
    dt[3] = 0.0f;
    model_anim_play(m, 0, 0);
    model_update(m, dt);
}

void dd_slot_pos(int id, float *out)
{
    float m[16];

    func_0884DAE4(m, id);
    vec3_set(out, m[12], m[13], m[14]);
}

void dd_slot_pos2(int id, float *out)
{
    float m[16];

    func_0884DAF8(m, id);
    vec3_set(out, m[12], m[13], m[14]);
}

void dd_pair_get(int *src, int *a, int *b)
{
    *a = src[0];
    *b = src[1];
}
