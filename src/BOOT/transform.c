// @range 0x088390C4-0x08839A8C
// @asm 0x088390C4-0x08839364
// @asm 0x08839558-0x08839868
// @data 0x088EE010-0x088EE050
/* Cubic splines through key positions: the second-derivative terms of a natural spline (a
   tridiagonal system solved forward into a work buffer and back into `d`), open or cyclic over a
   ring of `ring` keys, and the evaluation of the curve at a fractional key index. Vectors keep
   w = 1. */
#include "psp.h"
#include "allegrex.h"

typedef struct Vec4 {
    float x, y, z, w;
} Vec4;

extern Vec4 spline_work[64];          /* 0x08905110: gamma[32], then delta[32] */

/* Diagonal and numerator of the tridiagonal system (one pair per solver; both solvers are in asm,
   so the initializers live here instead of as local templates). */
Vec4 spline_four = { 4.0f, 4.0f, 4.0f, 0.0f };
Vec4 spline_one = { 1.0f, 1.0f, 1.0f, 0.0f };
Vec4 spline_ring_four = { 4.0f, 4.0f, 4.0f, 0.0f };
Vec4 spline_ring_one = { 1.0f, 1.0f, 1.0f, 0.0f };

void vec3_zero(Vec4 *v);
void vec4_zero(Vec4 *v);
Vec4 *vec3_add_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_sub_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_mul_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_div_w(Vec4 *out, Vec4 *a, Vec4 *b);
Vec4 *vec3_scale_w(Vec4 *out, Vec4 *a, float s);
Vec4 *vec3_lerp_w(float t, Vec4 *out, Vec4 *a, Vec4 *b);

#ifdef NON_MATCHING /* register allocation of the two induction variables over spline_work differs */
/* Second-derivative terms `d` of the spline through the `n` keys `p`. Returns n (0 when n < 0). */
int spline_init(Vec4 *d, Vec4 *p, int n)
{
    Vec4 v;
    Vec4 t;
    Vec4 b;
    Vec4 *w = spline_work;
    int i;

    if (n < 3) {
        switch (n) {
        case 1:
            vec3_zero(d);
            d[0].w = 1.0f;
            return 1;
        case 2:
            vec3_zero(d);
            vec3_zero(d + 1);
            d[0].w = 1.0f;
            d[1].w = 1.0f;
            return 2;
        default:
            return 0;
        }
    }
    vec4_zero(&w[0]);
    vec4_zero(&w[32]);
    for (i = 1; i < n - 1; i++) {
        Vec4 four = spline_four;

        vec3_sub_w(&t, &four, &w[i - 1]);
        {
            Vec4 one = spline_one;

            vec3_div_w(&w[i], &one, &t);
        }
        vec3_scale_w(&v, &p[i], 2.0f);
        vec3_sub_w(&v, &p[i + 1], &v);
        vec3_add_w(&v, &v, &p[i - 1]);
        vec3_scale_w(&v, &v, 3.0f);
        vec3_sub_w(&v, &v, &(w + 32)[i - 1]);
        vec3_mul_w(&w[i + 32], &v, &w[i]);
    }
    vec4_zero(&d[n - 1]);
    d[n - 1].w = 1.0f;
    for (i = n - 2; i >= 0; i--) {
        vec3_mul_w(&b, &w[i], &d[i + 1]);
        vec3_sub_w(&d[i], &w[i + 32], &b);
        d[i].w = 1.0f;
    }
    return n;
}
#endif

/* Point of the spline at key index `t` (clamped to the last segment). */


#ifdef NON_MATCHING /* register allocation differs */
/* spline_init over `n` keys of a ring of `ring`, starting at key `start`. */
int spline_init_ring(Vec4 *d, Vec4 *p, int start, int ring, int n)
{
    Vec4 v;
    Vec4 t;
    Vec4 b;
    Vec4 *w = spline_work;
    int i;
    int k;
    int prev;
    int cur;

    start %= ring;
    if (n < 3) {
        switch (n) {
        case 1:
            vec3_zero(&d[start]);
            d[start].w = 1.0f;
            return 1;
        case 2:
            vec3_zero(&d[start]);
            k = (start + 1) % ring;
            vec3_zero(&d[k]);
            d[start].w = 1.0f;
            d[k].w = 1.0f;
            return 2;
        default:
            return 0;
        }
    }
    vec4_zero(&w[start]);
    vec4_zero(&w[start + 32]);
    for (i = 1; i < n - 1; i++) {
        Vec4 four = spline_ring_four;
        Vec4 one = spline_ring_one;

        k = start + i;
        prev = (k - 1) % ring;
        vec3_sub_w(&t, &four, &w[prev]);
        cur = k % ring;
        vec3_div_w(&w[cur], &one, &t);
        vec3_scale_w(&v, &p[cur], 2.0f);
        vec3_sub_w(&v, &p[(k + 1) % ring], &v);
        vec3_add_w(&v, &v, &p[prev]);
        vec3_scale_w(&v, &v, 3.0f);
        vec3_sub_w(&v, &v, &w[prev + 32]);
        vec3_mul_w(&w[cur + 32], &v, &w[cur]);
    }
    k = (n + start + ring - 1) % ring;
    vec4_zero(&d[k]);
    d[k].w = 1.0f;
    for (i = n - 2; i >= 0; i--) {
        k = (start + i) % ring;
        vec3_mul_w(&b, &w[k], &d[(start + i + 1) % ring]);
        vec3_sub_w(&d[k], &w[k + 32], &b);
        d[k].w = 1.0f;
    }
    return n;
}
#endif

/* Point of the spline at key index `t` (clamped to the last segment). */
void spline_eval(float t, Vec4 *out, Vec4 *p, Vec4 *d, int n)
{
    Vec4 c;
    Vec4 b;
    Vec4 a;
    int k;
    float f2;
    float f;

    if (n < 3) {
        switch (n) {
        case 1:
            *out = *p;
            break;
        case 2:
            vec3_lerp_w(t, out, p, p + 1);
            out->w = 1.0f;
            break;
        default:
            vec4_zero(out);
            out->w = 1.0f;
            break;
        }
    } else {
        k = ALLEGREX_MIN((int)t, n - 1);
        f = t - k;
        f2 = f * f;
        vec3_scale_w(&c, &d[k], 2.0f);
        vec3_add_w(&c, &c, &d[k + 1]);
        vec3_scale_w(&c, &c, 1.0f / 3.0f);
        vec3_sub_w(&c, &p[k + 1], &c);
        vec3_sub_w(&b, &c, &p[k]);
        vec3_sub_w(&c, &d[k + 1], &d[k]);
        vec3_scale_w(&a, &c, 1.0f / 3.0f);
        vec3_scale_w(&b, &b, f);
        vec3_scale_w(&c, &d[k], f2);
        vec3_scale_w(&a, &a, f2 * f);
        vec3_add_w(out, &p[k], &b);
        vec3_add_w(out, out, &c);
        vec3_add_w(out, out, &a);
        out->w = 1.0f;
    }
}

/* spline_eval over a ring of `ring` keys starting at `start`. */
void spline_eval_ring(float t, Vec4 *out, Vec4 *p, Vec4 *d, int start, int ring, int n)
{
    Vec4 c;
    Vec4 b;
    Vec4 a;
    int k;
    int k1;
    float f2;
    float f;

    start %= ring;
    if (n < 3) {
        switch (n) {
        case 1:
            *out = *p;
            break;
        case 2:
            k = (start + 1) % ring;
            vec3_lerp_w(t, out, &p[start], &p[k]);
            out->w = 1.0f;
            break;
        default:
            vec4_zero(out);
            out->w = 1.0f;
            break;
        }
    } else {
        k = ALLEGREX_MIN((int)t, n - 1);
        k1 = (start + k + 1) % ring;
        f = t - k;
        f2 = f * f;
        ring = (start + k) % ring;
        vec3_scale_w(&c, &d[ring], 2.0f);
        vec3_add_w(&c, &c, &d[k1]);
        vec3_scale_w(&c, &c, 1.0f / 3.0f);
        vec3_sub_w(&c, &p[k1], &c);
        vec3_sub_w(&b, &c, &p[ring]);
        vec3_sub_w(&c, &d[k1], &d[ring]);
        vec3_scale_w(&a, &c, 1.0f / 3.0f);
        vec3_scale_w(&b, &b, f);
        vec3_scale_w(&c, &d[ring], f2);
        vec3_scale_w(&a, &a, f2 * f);
        vec3_add_w(out, &p[ring], &b);
        vec3_add_w(out, out, &c);
        vec3_add_w(out, out, &a);
        out->w = 1.0f;
    }
}
