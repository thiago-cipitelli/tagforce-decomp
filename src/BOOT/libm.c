// @range 0x088087F8-0x0880A5C0
// @gcc 0x088087F8-0x08808AE0 nl_ef_asin
// @gcc 0x08808AE0-0x08808D2C nl_ef_atan2
// @gcc 0x08808D2C-0x0880907C nl_ef_rem_pio2
// @gcc 0x0880907C-0x088091A0 nl_ef_sqrt
// @gcc 0x088091A0-0x088092C0 libm_kf_cos
// @gcc 0x088092C0-0x08809A80 libm_kf_rem_pio2
// @gcc 0x08809A80-0x08809B54 libm_kf_sin
// @gcc 0x08809B54-0x08809B88 nl_s_isinf
// @gcc 0x08809B88-0x08809BAC nl_s_isnan
// @gcc 0x08809BAC-0x08809BB4 nl_s_matherr
// @gcc 0x08809BB4-0x08809E50 nl_sf_atan
// @gcc 0x08809E50-0x08809E70 nl_sf_copysign
// @gcc 0x08809E70-0x08809F38 nl_sf_cos
// @gcc 0x08809F38-0x08809F4C libm_sf_fabs
// @gcc 0x08809F4C-0x0880A028 nl_sf_floor
// @gcc 0x0880A028-0x0880A03C nl_sf_isnan
// @gcc 0x0880A03C-0x0880A154 nl_sf_scalbn
// @gcc 0x0880A154-0x0880A220 libm_sf_sin
// @gcc 0x0880A220-0x0880A340 nl_wf_asin
// @gcc 0x0880A340-0x0880A484 libm_wf_atan2
// @gcc 0x0880A484-0x0880A5B4 libm_wf_sqrt
// @gcc 0x0880A5B4-0x0880A5C0 nl_abs
/* newlib libm (fdlibm float code, GCC) of the Sony SDK libc: the __ieee754_ and __kernel_ cores,
   sinf/cosf/atanf/asinf/atan2f/sqrtf with their SVID error wrappers and a few bit helpers. The
   functions that load their polynomial constants through %hi/%lo stay in the split asm; their C
   under NON_MATCHING is the fdlibm source, checked against the asm (see below). The names carry
   a libm_ prefix where the plain one belongs to the VFPU versions (see vfpu_math.c).
   The `@gcc` ranges are the functions that already come out of the newlib source with the SDK's GCC (libm_kf_cos.c,
   libm_kf_rem_pio2.c, libm_kf_sin.c, libm_sf_fabs.c, libm_sf_sin.c, libm_wf_atan2.c, libm_wf_sqrt.c and the nl_*.c of
   newlib 1.12.0; docs/build.md, "Trilha GCC"); libm_asinf needs rule G of tools/psp_gcc_peep.py (rodada 38). */

#ifndef __MWERKS__
#include <math.h>
#endif

float __ieee754_asinf(float x);
float __ieee754_atan2f(float y, float x);
int __ieee754_rem_pio2f(float x, float *y);
float __ieee754_sqrtf(float x);
float __kernel_cosf(float x, float y);
int __kernel_rem_pio2f(float *x, float *y, int e0, int nx, int prec, const int *ipio2);
float __kernel_sinf(float x, float y, int iy);
int matherr(void *exc);
float libm_atanf(float x);
float libm_copysignf(float x, float y);
float libm_fabsf(float x);
float libm_floorf(float x);
int libm_isnanf(float x);
float libm_scalbnf(float x, int n);

typedef union {
    float f;
    unsigned int i;
} libm_bits;

#define GET_FLOAT_WORD(w, d) do { libm_bits gf_; gf_.f = (d); (w) = gf_.i; } while (0)
#define SET_FLOAT_WORD(d, w) do { libm_bits sf_; sf_.i = (w); (d) = sf_.f; } while (0)

/* The C under NON_MATCHING below is the newlib fdlibm float code the asm was compiled from
   (ef_asin, ef_atan2, ef_rem_pio2, ef_sqrt, kf_cos, kf_rem_pio2, kf_sin, sf_atan, sf_floor,
   sf_scalbn, wf_asin, wf_atan2, wf_sqrt): every threshold, constant and table checked against the
   original asm and .rodata (GCC folded pi+tiny and the like into single constants). */
#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
float __ieee754_asinf(float x)
{
    float t = 0.0f, w, p, q, c, r, s;
    int hx, ix;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7FFFFFFF;
    if (ix == 0x3F800000) {
        /* asin(+-1) = +-pi/2 */
        return x * 1.57079637050628662109375f + x * -4.37113900018624283e-8f;
    } else if (ix > 0x3F800000) {
        return (x - x) / (x - x); /* |x| > 1: NaN */
    } else if (ix < 0x3F000000) { /* |x| < 0.5 */
        if (ix < 0x32000000) {    /* |x| < 2**-27 */
            if (1.0e30f + x > 1.0f) {
                return x;
            }
        } else {
            t = x * x;
        }
        /* fdlibm's dangling else: for a tiny x the polynomial below runs with t = 0 */
        p = t * (1.6666667163e-01f + t * (-3.2556581497e-01f + t * (2.0121252537e-01f +
            t * (-4.0055535734e-02f + t * (7.9153501429e-04f + t * 3.4793309169e-05f)))));
        q = 1.0f + t * (-2.4033949375e+00f + t * (2.0209457874e+00f + t * (-6.8828397989e-01f +
            t * 7.7038154006e-02f)));
        w = p / q;
        return x + x * w;
    }
    /* 1 > |x| >= 0.5 */
    w = 1.0f - libm_fabsf(x);
    t = w * 0.5f;
    p = t * (1.6666667163e-01f + t * (-3.2556581497e-01f + t * (2.0121252537e-01f +
        t * (-4.0055535734e-02f + t * (7.9153501429e-04f + t * 3.4793309169e-05f)))));
    q = 1.0f + t * (-2.4033949375e+00f + t * (2.0209457874e+00f + t * (-6.8828397989e-01f +
        t * 7.7038154006e-02f)));
    s = __ieee754_sqrtf(t);
    if (ix >= 0x3F79999A) { /* |x| > 0.975 */
        w = p / q;
        t = 1.57079637050628662109375f - (2.0f * (s + s * w) - -4.37113900018624283e-8f);
    } else {
        unsigned int iw;

        w = s;
        GET_FLOAT_WORD(iw, w);
        SET_FLOAT_WORD(w, iw & 0xFFFFF000);
        c = (t - w * w) / (s + w);
        r = p / q;
        p = 2.0f * s * r - (-4.37113900018624283e-8f - 2.0f * c);
        q = 0.785398185253143310546875f - 2.0f * w;
        t = 0.785398185253143310546875f - (p - q);
    }
    if (hx > 0) {
        return t;
    }
    return -t;
}

float __ieee754_atan2f(float y, float x)
{
    /* pi_o_4, pi_o_2, pi, pi_lo; "+ tiny" (1e-30) dropped: it does not change a float */
    float z;
    int k, m, hx, hy, ix, iy;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7FFFFFFF;
    GET_FLOAT_WORD(hy, y);
    iy = hy & 0x7FFFFFFF;
    if (ix > 0x7F800000 || iy > 0x7F800000) {
        return x + y; /* NaN */
    }
    if (hx == 0x3F800000) {
        return libm_atanf(y); /* x = 1.0 */
    }
    m = ((hy >> 31) & 1) | ((hx >> 30) & 2); /* 2*sign(x) + sign(y) */

    if (iy == 0) {
        switch (m) {
        case 0:
        case 1:
            return y;
        case 2:
            return 3.1415925026e+00f;
        case 3:
            return -3.1415925026e+00f;
        }
    }
    if (ix == 0) {
        return hy < 0 ? -1.5707963705e+00f : 1.5707963705e+00f;
    }
    if (ix == 0x7F800000) {
        if (iy == 0x7F800000) {
            switch (m) {
            case 0:
                return 7.8539818525e-01f;
            case 1:
                return -7.8539818525e-01f;
            case 2:
                return 3.0f * 7.8539818525e-01f;
            case 3:
                return -3.0f * 7.8539818525e-01f;
            }
        } else {
            switch (m) {
            case 0:
                return 0.0f;
            case 1:
                return -0.0f;
            case 2:
                return 3.1415925026e+00f;
            case 3:
                return -3.1415925026e+00f;
            }
        }
    }
    if (iy == 0x7F800000) {
        return hy < 0 ? -1.5707963705e+00f : 1.5707963705e+00f;
    }

    k = (iy - ix) >> 23;
    if (k > 60) {
        z = 1.5707963705e+00f + 0.5f * 1.5099578832e-07f; /* |y/x| > 2**60 */
    } else if (hx < 0 && k < -60) {
        z = 0.0f; /* |y|/x < -2**60 */
    } else {
        z = libm_atanf(libm_fabsf(y / x));
    }
    switch (m) {
    case 0:
        return z;
    case 1: {
        unsigned int zh;

        GET_FLOAT_WORD(zh, z);
        SET_FLOAT_WORD(z, zh ^ 0x80000000);
        return z;
    }
    case 2:
        return 3.1415925026e+00f - (z - 1.5099578832e-07f);
    default:
        return (z - 1.5099578832e-07f) - 3.1415925026e+00f;
    }
}

/* 8-bit chunks of 2/pi */
static const int libm_two_over_pi[198] = {
    0xA2, 0xF9, 0x83, 0x6E, 0x4E, 0x44, 0x15, 0x29, 0xFC, 0x27, 0x57, 0xD1, 0xF5, 0x34, 0xDD, 0xC0,
    0xDB, 0x62, 0x95, 0x99, 0x3C, 0x43, 0x90, 0x41, 0xFE, 0x51, 0x63, 0xAB, 0xDE, 0xBB, 0xC5, 0x61,
    0xB7, 0x24, 0x6E, 0x3A, 0x42, 0x4D, 0xD2, 0xE0, 0x06, 0x49, 0x2E, 0xEA, 0x09, 0xD1, 0x92, 0x1C,
    0xFE, 0x1D, 0xEB, 0x1C, 0xB1, 0x29, 0xA7, 0x3E, 0xE8, 0x82, 0x35, 0xF5, 0x2E, 0xBB, 0x44, 0x84,
    0xE9, 0x9C, 0x70, 0x26, 0xB4, 0x5F, 0x7E, 0x41, 0x39, 0x91, 0xD6, 0x39, 0x83, 0x53, 0x39, 0xF4,
    0x9C, 0x84, 0x5F, 0x8B, 0xBD, 0xF9, 0x28, 0x3B, 0x1F, 0xF8, 0x97, 0xFF, 0xDE, 0x05, 0x98, 0x0F,
    0xEF, 0x2F, 0x11, 0x8B, 0x5A, 0x0A, 0x6D, 0x1F, 0x6D, 0x36, 0x7E, 0xCF, 0x27, 0xCB, 0x09, 0xB7,
    0x4F, 0x46, 0x3F, 0x66, 0x9E, 0x5F, 0xEA, 0x2D, 0x75, 0x27, 0xBA, 0xC7, 0xEB, 0xE5, 0xF1, 0x7B,
    0x3D, 0x07, 0x39, 0xF7, 0x8A, 0x52, 0x92, 0xEA, 0x6B, 0xFB, 0x5F, 0xB1, 0x1F, 0x8D, 0x5D, 0x08,
    0x56, 0x03, 0x30, 0x46, 0xFC, 0x7B, 0x6B, 0xAB, 0xF0, 0xCF, 0xBC, 0x20, 0x9A, 0xF4, 0x36, 0x1D,
    0xA9, 0xE3, 0x91, 0x61, 0x5E, 0xE6, 0x1B, 0x08, 0x65, 0x99, 0x85, 0x5F, 0x14, 0xA0, 0x68, 0x40,
    0x8D, 0xFF, 0xD8, 0x80, 0x4D, 0x73, 0x27, 0x31, 0x06, 0x06, 0x15, 0x56, 0xCA, 0x73, 0xA8, 0xC9,
    0x60, 0xE2, 0x7B, 0xC0, 0x8C, 0x6B,
};

/* high bits of n*pi/2, n = 1..32 */
static const int libm_npio2_hw[32] = {
    0x3FC90F00, 0x40490F00, 0x4096CB00, 0x40C90F00, 0x40FB5300, 0x4116CB00, 0x412FED00, 0x41490F00,
    0x41623100, 0x417B5300, 0x418A3A00, 0x4196CB00, 0x41A35C00, 0x41AFED00, 0x41BC7E00, 0x41C90F00,
    0x41D5A000, 0x41E23100, 0x41EEC200, 0x41FB5300, 0x4203F200, 0x420A3A00, 0x42108300, 0x4216CB00,
    0x421D1400, 0x42235C00, 0x4229A500, 0x422FED00, 0x42363600, 0x423C7E00, 0x4242C700, 0x42490F00,
};

/* x - n*pi/2 as y[0] + y[1]; returns n. pio2_1 = 0x3FC90F80, pio2_1t = 0x37354443,
   pio2_2 = 0x37354400, pio2_2t = 0x2E85A308, pio2_3 = 0x2E85A300, pio2_3t = 0x248D3132 */
int __ieee754_rem_pio2f(float x, float *y)
{
    float z, w, t, r, fn;
    float tx[3];
    int i, j, n, ix, hx;
    int e0, nx;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7FFFFFFF;
    if (ix <= 0x3F490FD8) { /* |x| ~<= pi/4 */
        y[0] = x;
        y[1] = 0.0f;
        return 0;
    }
    if (ix < 0x4016CBE4) { /* |x| < 3pi/4: n = +-1 */
        if (hx > 0) {
            z = x - 1.5707855225e+00f;
            if ((ix & 0xFFFFFFF0) != 0x3FC90FD0) {
                y[0] = z - 1.0804334124e-05f;
                y[1] = (z - y[0]) - 1.0804334124e-05f;
            } else { /* near pi/2: 24+24+24 bit pi */
                z -= 1.0804273188e-05f;
                y[0] = z - 6.0770999344e-11f;
                y[1] = (z - y[0]) - 6.0770999344e-11f;
            }
            return 1;
        } else {
            z = x + 1.5707855225e+00f;
            if ((ix & 0xFFFFFFF0) != 0x3FC90FD0) {
                y[0] = z + 1.0804334124e-05f;
                y[1] = (z - y[0]) + 1.0804334124e-05f;
            } else {
                z += 1.0804273188e-05f;
                y[0] = z + 6.0770999344e-11f;
                y[1] = (z - y[0]) + 6.0770999344e-11f;
            }
            return -1;
        }
    }
    if (ix <= 0x43490F80) { /* |x| ~<= 2^7*(pi/2): medium size */
        t = libm_fabsf(x);
        n = (int)(t * 6.3661980629e-01f + 0.5f);
        fn = (float)n;
        r = t - fn * 1.5707855225e+00f;
        w = fn * 1.0804334124e-05f; /* 1st round good to 40 bits */
        if (n < 32 && (int)(ix & 0xFFFFFF00) != libm_npio2_hw[n - 1]) {
            y[0] = r - w;
        } else {
            unsigned int high;

            j = ix >> 23;
            y[0] = r - w;
            GET_FLOAT_WORD(high, y[0]);
            i = j - ((high >> 23) & 0xFF);
            if (i > 8) { /* 2nd iteration, good to 57 bits */
                t = r;
                w = fn * 1.0804273188e-05f;
                r = t - w;
                w = fn * 6.0770999344e-11f - ((t - r) - w);
                y[0] = r - w;
                GET_FLOAT_WORD(high, y[0]);
                i = j - ((high >> 23) & 0xFF);
                if (i > 25) { /* 3rd iteration, 74 bits */
                    t = r;
                    w = fn * 6.0770943833e-11f;
                    r = t - w;
                    w = fn * 6.1232342629e-17f - ((t - r) - w);
                    y[0] = r - w;
                }
            }
        }
        y[1] = (r - y[0]) - w;
        if (hx < 0) {
            y[0] = -y[0];
            y[1] = -y[1];
            return -n;
        }
        return n;
    }
    if (ix > 0x7F7FFFFF) { /* inf or NaN */
        y[0] = y[1] = x - x;
        return 0;
    }
    /* large argument: z = scalbn(|x|, ilogb(x) - 7) */
    e0 = (ix >> 23) - 134;
    SET_FLOAT_WORD(z, ix - (e0 << 23));
    for (i = 0; i < 2; i++) {
        tx[i] = (float)(int)z;
        z = (z - tx[i]) * 256.0f;
    }
    tx[2] = z;
    nx = 3;
    while (tx[nx - 1] == 0.0f) {
        nx--;
    }
    n = __kernel_rem_pio2f(tx, y, e0, nx, 2, libm_two_over_pi);
    if (hx < 0) {
        y[0] = -y[0];
        y[1] = -y[1];
        return -n;
    }
    return n;
}

float __ieee754_sqrtf(float x)
{
    float z;
    unsigned int r, hx;
    int ix, s, q, m, t, i;

    GET_FLOAT_WORD(ix, x);
    hx = ix & 0x7FFFFFFF;
    if (hx > 0x7F7FFFFF) {
        return x * x + x; /* sqrt(NaN) = NaN, sqrt(+inf) = +inf, sqrt(-inf) = NaN */
    }
    if (hx == 0) {
        return x; /* sqrt(+-0) = +-0 */
    }
    if (ix < 0) {
        return (x - x) / (x - x); /* sqrt(-ve) = NaN */
    }
    m = ix >> 23;
    if (hx < 0x00800000) { /* subnormal */
        for (i = 0; (ix & 0x00800000) == 0; i++) {
            ix <<= 1;
        }
        m -= i - 1;
    }
    m -= 127;
    ix = (ix & 0x007FFFFF) | 0x00800000;
    if (m & 1) {
        ix += ix;
    }
    m >>= 1;

    /* sqrt bit by bit */
    ix += ix;
    q = s = 0;
    r = 0x01000000;
    while (r != 0) {
        t = s + r;
        if (t <= ix) {
            s = t + r;
            ix -= t;
            q += r;
        }
        ix += ix;
        r >>= 1;
    }
    /* rounding: one-tiny and one+tiny fold to 1.0, so round to even */
    if (ix != 0) {
        q += q & 1;
    }
    ix = (q >> 1) + 0x3F000000;
    ix += m << 23;
    SET_FLOAT_WORD(z, ix);
    return z;
}

/* cos(x + y) for |x| <= pi/4 */
static const int libm_init_jk[3] = { 4, 7, 9 };

/* pi/2 in 8-bit pieces */
static const float libm_PIo2[11] = {
    1.5703125f, 4.57763672e-04f, 2.59876251e-05f, 7.54371285e-08f, 6.00266503e-11f,
    7.38964445e-13f, 5.38458167e-15f, 5.6378513e-18f, 8.30092288e-20f, 3.27563523e-22f,
    6.33310156e-25f,
};

/* sin(x + y) for |x| <= pi/4 (iy = 0: y is 0) */
float __kernel_sinf(float x, float y, int iy)
{
    float z, r, v;
    int ix;

    GET_FLOAT_WORD(ix, x);
    ix &= 0x7FFFFFFF;
    if (ix < 0x32000000) { /* |x| < 2**-27 */
        if ((int)x == 0) {
            return x;
        }
    }
    z = x * x;
    v = z * x;
    r = 8.3333337680e-03f + z * (-1.9841270114e-04f + z * (2.7557314297e-06f +
        z * (-2.5050759689e-08f + z * 1.5896910177e-10f)));
    if (iy == 0) {
        return x + v * (-1.6666667163e-01f + z * r);
    }
    return x - ((z * (0.5f * y - v * r) - y) - v * -1.6666667163e-01f);
}
#endif

/* 1 when the double (lo, hi words) is an infinity */
int __isinfd(unsigned int lo, unsigned int hi);
#ifndef __MWERKS__ /* PSP: nl_s_isinf.c */
int __isinfd(unsigned int lo, unsigned int hi)
{
    unsigned int v = (hi & 0x7FFFFFFF) | ((lo | -lo) >> 31);

    return v == 0x7FF00000;
}
#endif

/* 1 when the double (lo, hi words) is a NaN */
int __isnand(unsigned int lo, unsigned int hi);
#ifndef __MWERKS__ /* PSP: nl_s_isnan.c */
int __isnand(unsigned int lo, unsigned int hi)
{
    unsigned int v = (hi & 0x7FFFFFFF) | ((lo | -lo) >> 31);

    return v > 0x7FF00000;
}
#endif

/* SVID matherr: no handler */
#ifndef __MWERKS__ /* PSP: nl_s_matherr.c */
int matherr(void *exc)
{
    return 0;
}
#endif

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
/* atan(0.5), atan(1), atan(1.5), atan(inf), high and low parts */
static const float libm_atanhi[4] = { 4.6364760399e-01f, 7.8539812565e-01f, 9.8279368877e-01f, 1.5707962513e+00f };
static const float libm_atanlo[4] = { 5.0121582440e-09f, 3.7748947079e-08f, 3.4473217170e-08f, 7.5497894159e-08f };

float libm_atanf(float x)
{
    float w, s1, s2, z;
    int ix, hx, id;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7FFFFFFF;
    if (ix >= 0x50800000) { /* |x| >= 2^34 */
        if (ix > 0x7F800000) {
            return x + x; /* NaN */
        }
        if (hx > 0) {
            return libm_atanhi[3] + libm_atanlo[3];
        }
        return -libm_atanhi[3] - libm_atanlo[3];
    }
    if (ix < 0x3EE00000) {     /* |x| < 0.4375 */
        if (ix < 0x31000000) { /* |x| < 2^-29 */
            if (1.0e30f + x > 1.0f) {
                return x;
            }
        }
        id = -1;
    } else {
        x = libm_fabsf(x);
        if (ix < 0x3F980000) {     /* |x| < 1.1875 */
            if (ix < 0x3F300000) { /* 7/16 <= |x| < 11/16 */
                id = 0;
                x = (2.0f * x - 1.0f) / (2.0f + x);
            } else { /* 11/16 <= |x| < 19/16 */
                id = 1;
                x = (x - 1.0f) / (x + 1.0f);
            }
        } else if (ix < 0x401C0000) { /* |x| < 2.4375 */
            id = 2;
            x = (x - 1.5f) / (1.0f + 1.5f * x);
        } else { /* 2.4375 <= |x| < 2^34 */
            id = 3;
            x = -1.0f / x;
        }
    }
    z = x * x;
    w = z * z;
    /* aT[0..10]: odd and even terms */
    s1 = z * (3.3333334327e-01f + w * (1.4285714924e-01f + w * (9.0908870101e-02f +
         w * (6.6610731184e-02f + w * (4.9768779427e-02f + w * 1.6285819933e-02f)))));
    s2 = w * (-2.0000000298e-01f + w * (-1.1111110449e-01f + w * (-7.6918758452e-02f +
         w * (-5.8335702866e-02f + w * -3.6531571299e-02f))));
    if (id < 0) {
        return x - x * (s1 + s2);
    }
    z = libm_atanhi[id] - ((x * (s1 + s2) - libm_atanlo[id]) - x);
    return hx < 0 ? -z : z;
}
#endif

#ifndef __MWERKS__ /* PSP: nl_sf_copysign.c */
float libm_copysignf(float x, float y)
{
    libm_bits a;
    libm_bits b;

    a.f = x;
    b.f = y;
    a.i = (a.i & 0x7FFFFFFF) | (b.i & 0x80000000);
    return a.f;
}
#endif

/* newlib cosf */
#ifndef __MWERKS__ /* PSP: nl_sf_cos.c */
float libm_cosf(float x)
{
    return cosf(x);
}
#endif

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
#ifndef __MWERKS__ /* PSP: nl_sf_floor.c */
float libm_floorf(float x)
{
    int i0, j0;
    unsigned int i, ix;

    GET_FLOAT_WORD(i0, x);
    ix = i0 & 0x7FFFFFFF;
    j0 = (ix >> 23) - 0x7F;
    if (j0 < 23) {
        if (j0 < 0) { /* |x| < 1: +0 or -1 (huge+x > 0 raises inexact) */
            if (1.0e30f + x > 0.0f) {
                if (i0 >= 0) {
                    i0 = 0;
                } else if (ix != 0) {
                    i0 = 0xBF800000;
                }
            }
        } else {
            i = 0x007FFFFF >> j0;
            if ((i0 & i) == 0) {
                return x; /* integral */
            }
            if (1.0e30f + x > 0.0f) {
                if (i0 < 0) {
                    i0 += 0x00800000 >> j0;
                }
                i0 &= ~i;
            }
        }
    } else {
        if (ix > 0x7F7FFFFF) {
            return x + x; /* inf or NaN */
        }
        return x; /* integral */
    }
    SET_FLOAT_WORD(x, i0);
    return x;
}
#endif
#endif

#ifndef __MWERKS__ /* PSP: nl_sf_isnan.c */
int libm_isnanf(float x)
{
    libm_bits b;

    b.f = x;
    return (b.i & 0x7FFFFFFF) > 0x7F800000;
}
#endif

#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
float libm_scalbnf(float x, int n)
{
    int k, ix;
    unsigned int hx;

    GET_FLOAT_WORD(ix, x);
    hx = ix & 0x7FFFFFFF;
    k = hx >> 23;
    if (hx == 0) {
        return x;
    }
    if (hx > 0x7F7FFFFF) {
        return x + x; /* NaN or Inf */
    }
    if (hx < 0x00800000) { /* subnormal */
        x *= 33554432.0f;  /* 2^25 */
        GET_FLOAT_WORD(ix, x);
        k = ((ix & 0x7F800000) >> 23) - 25;
        if (n < -50000) {
            return 1.0e-30f * x; /* underflow */
        }
    }
    k = k + n;
    if (k > 0xFE) {
        return 1.0e30f * libm_copysignf(1.0e30f, x); /* overflow */
    }
    if (k > 0) {
        SET_FLOAT_WORD(x, (ix & 0x807FFFFF) | (k << 23));
        return x;
    }
    if (k < -22) {
        if (n > 50000) { /* integer overflow in n+k */
            return 1.0e30f * libm_copysignf(1.0e30f, x);
        }
        return 1.0e-30f * libm_copysignf(1.0e-30f, x);
    }
    k += 25; /* subnormal result */
    SET_FLOAT_WORD(x, (ix & 0x807FFFFF) | (k << 23));
    return x * 2.9802322388e-08f; /* 2^-25 */
}
#endif

/* newlib sinf */
#ifdef NON_MATCHING /* GCC: constants loaded through %hi/%lo */
/* newlib wf_*.c wrappers: __fdlib_version is _LIB_VERSION (0x08873880, = 1 _XOPEN_; -1 _IEEE_
   returns the core result untouched, 2 _POSIX_ sets errno without asking matherr). A domain error
   fills a struct exception (type DOMAIN, retval 0, or NaN for sqrtf outside _SVID_), lets matherr
   handle it, else errno = EDOM (33); a nonzero exc.err overrides errno. */
int *__errno(void);
extern int __fdlib_version;

struct libm_exception {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
    int err;
};

static float libm_domain_error(struct libm_exception *exc)
{
    if (__fdlib_version == 2 || !matherr(exc)) {
        *__errno() = 33;
    }
    if (exc->err != 0) {
        *__errno() = exc->err;
    }
    return (float)exc->retval;
}

float libm_asinf(float x)
{
    float z = __ieee754_asinf(x);
    struct libm_exception exc;

    if (__fdlib_version == -1 || libm_isnanf(x)) {
        return z;
    }
    if (libm_fabsf(x) > 1.0f) {
        exc.type = 1;
        exc.name = "asinf";
        exc.err = 0;
        exc.arg1 = exc.arg2 = (double)x;
        exc.retval = 0.0;
        return libm_domain_error(&exc);
    }
    return z;
}

#endif

#ifndef __MWERKS__ /* PSP: nl_abs.c */
int int_abs(int x)
{
    return x < 0 ? -x : x;
}
#endif
