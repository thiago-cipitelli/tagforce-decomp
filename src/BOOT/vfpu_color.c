// @range 0x0881D214-0x0881D8A8
// @gcc 0x0881D214-0x0881D2F4
// @gcc 0x0881D3D8-0x0881D3F4
// @gcc 0x0881D48C-0x0881D5C0
// @gcc 0x0881D6C0-0x0881D6F8
// @gcc 0x0881D740-0x0881D7AC
// @gcc 0x0881D7DC-0x0881D804
// @gcc 0x0881D854-0x0881D8A8
/* VFPU colors (four floats r, g, b, a in [0, 1]; the _rgb variants leave alpha as in the first
   operand) and the VFPU random generator. Hand-written VFPU code kept as whole `asm` functions; the
   functions the SDK compiled with GCC come from C (vfpu_color_gcc.c). The port C of the VFPU functions is left
   out of that GCC object (`#elif !defined(__mips__)`), so its literal pool is exactly the original's (`@rodata`).
   Functions with an output color return it. Outside the PSP build each one has a C equivalent; the
   random generator state (VFPU control registers 136-143) is the C array vfpu_rcx. */

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

int vfpu_rng_save(unsigned int *st);
int vfpu_rng_load(unsigned int *st);

#if !defined(__MWERKS__) && !defined(__mips__)
#include <math.h>

/* VFPU random generator: the hardware algorithm is unknown, a xorshift128 stands in for it. */
unsigned int vfpu_rcx[8] = { 0x3F800001, 0x3F800002, 0x3F800004, 0x3F800008, 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000 };

/* vrndf1: uniform in [1, 2) */
float vfpu_rndf1(void)
{
    unsigned int t = vfpu_rcx[0] ^ (vfpu_rcx[0] << 11);
    union {
        unsigned int u;
        float f;
    } r;

    vfpu_rcx[0] = vfpu_rcx[1];
    vfpu_rcx[1] = vfpu_rcx[2];
    vfpu_rcx[2] = vfpu_rcx[3];
    vfpu_rcx[3] = vfpu_rcx[3] ^ (vfpu_rcx[3] >> 19) ^ t ^ (t >> 8);
    r.u = 0x3F800000 | (vfpu_rcx[3] >> 9);
    return r.f;
}

static float sat(float x)
{
    return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x;
}
#elif !defined(__MWERKS__)
/* the SDK's GCC: vrndf1 into S000, read back through the stack */
static inline float vfpu_rndf1(void)
{
    float r;

    __asm__ ("vrndf1.s S000\n\tsv.s S000, %0" : "=m"(r));
    return r;
}
#elif defined(NON_MATCHING)
static asm float vfpu_rndf1(void)
{
    addiu sp, sp, -16
    vrndf1.s S000
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#endif

#ifdef NON_MATCHING /* PSP: vfpu_color_gcc.c */
/* RGB565 (r in the low bits); alpha ignored, negative channels give 0 */
unsigned short color_to_565(Vec4 *c)
{
    int r;
    int g;
    int b;

    if (c->x < 0.0f) {
        r = 0;
    } else {
        r = (int)(c->x * 31.0f);
        r = r < 31 ? r : 31;
    }
    if (c->y < 0.0f) {
        g = 0;
    } else {
        g = (int)(c->y * 63.0f);
        g = g < 63 ? g : 63;
    }
    if (c->z < 0.0f) {
        b = 0;
    } else {
        b = (int)(c->z * 31.0f);
        b = b < 31 ? b : 31;
    }
    return (unsigned short)(b << 11 | g << 5 | r);
}
#endif

#ifndef __MWERKS__ /* PSP: vfpu_color_gcc.c */
Vec4 *color_set(Vec4 *out, float r, float g, float b, float a)
{
    out->x = r;
    out->y = g;
    out->z = b;
    out->w = a;
    return out;
}
#else
Vec4 *color_set(Vec4 *out, float r, float g, float b, float a);
#endif

#ifndef __MWERKS__ /* PSP: vfpu_color_gcc.c */
Vec4 *color_set_rgb(Vec4 *out, float r, float g, float b)
{
    out->x = r;
    out->y = g;
    out->z = b;
    return out;
}
#else
Vec4 *color_set_rgb(Vec4 *out, float r, float g, float b);
#endif

#ifdef __MWERKS__
asm Vec4 *color_copy(Vec4 *out, Vec4 *c)
{
    lwc1 $f4, 12(a1)
    lwc1 $f1, 0(a1)
    lwc1 $f2, 4(a1)
    lwc1 $f3, 8(a1)
    move v0, a0
    swc1 $f1, 0(a0)
    swc1 $f2, 4(a0)
    swc1 $f3, 8(a0)
    jr ra
    swc1 $f4, 12(a0)
}
#elif !defined(__mips__)
Vec4 *color_copy(Vec4 *out, Vec4 *c)
{
    *out = *c;
    return out;
}
#else
Vec4 *color_copy(Vec4 *out, Vec4 *c);
#endif

#ifdef __MWERKS__
asm Vec4 *color_zero(Vec4 *out)
{
    move v0, a0
    sw zero, 0(a0)
    sw zero, 4(a0)
    sw zero, 8(a0)
    jr ra
    sw zero, 12(a0)
}
#elif !defined(__mips__)
Vec4 *color_zero(Vec4 *out)
{
    out->x = out->y = out->z = out->w = 0.0f;
    return out;
}
#else
Vec4 *color_zero(Vec4 *out);
#endif

/* RGBA8888 (r in the low byte), channels clamped to [0, 1] */
#ifdef __MWERKS__
asm unsigned int color_pack(Vec4 *c)
{
    lv.q C000, 0(a0)
    vsat0.q C000, C000
    viim.s S010, 255
    vscl.q C000, C000, S010
    vf2iz.q C000, C000, 23
    vi2uc.q S000, C000
    mfv v0, S000
    jr ra
    nop
}
#elif !defined(__mips__)
unsigned int color_pack(Vec4 *c)
{
    unsigned int r = (unsigned int)(sat(c->x) * 255.0f);
    unsigned int g = (unsigned int)(sat(c->y) * 255.0f);
    unsigned int b = (unsigned int)(sat(c->z) * 255.0f);
    unsigned int a = (unsigned int)(sat(c->w) * 255.0f);

    return a << 24 | b << 16 | g << 8 | r;
}
#else
unsigned int color_pack(Vec4 *c);
#endif

/* RGBA4444 (r in the low nibble), channels clamped to [0, 1] */
#ifdef __MWERKS__
asm unsigned short color_to_4444(Vec4 *c)
{
    lv.q C000, 0(a0)
    vsat0.q C000, C000
    viim.s S010, 15
    vscl.q C000, C000, S010
    vf2iz.q C000, C000, 0
    mfv v1, S000
    mfv t0, S001
    mfv t1, S002
    mfv t2, S003
    ins v1, t0, 0x4, 0x4
    ins v1, t1, 0x8, 0x4
    ins v1, t2, 0xc, 0x4
    jr ra
    andi v0, v1, 0xffff
}
#elif !defined(__mips__)
unsigned short color_to_4444(Vec4 *c)
{
    unsigned int r = (unsigned int)(sat(c->x) * 15.0f);
    unsigned int g = (unsigned int)(sat(c->y) * 15.0f);
    unsigned int b = (unsigned int)(sat(c->z) * 15.0f);
    unsigned int a = (unsigned int)(sat(c->w) * 15.0f);

    return (unsigned short)(a << 12 | b << 8 | g << 4 | r);
}
#else
unsigned short color_to_4444(Vec4 *c);
#endif

/* RGBA5551: rgb clamped to [0, 1]; the alpha bit is set when c->w > alpha_ref */
#ifdef __MWERKS__
asm unsigned short color_to_5551(Vec4 *c, float alpha_ref)
{
    lwc1 $f1, 12(a0)
    lv.q C000, 0(a0)
    vsat0.q C000, C000
    viim.s S010, 31
    vscl.t C000, C000, S010
    vf2iz.t C000, C000, 0
    mfv v1, S000
    mfv t0, S001
    mfv t1, S002
    c.le.s $f1, $f12
    ins v1, t0, 0x5, 0x5
    ins v1, t1, 0xa, 0x5
    ori t0, zero, 0x8000
    bc1tl L_0881D3CC
    move t0, zero
L_0881D3CC:
    or v1, v1, t0
    jr ra
    andi v0, v1, 0xffff
}
#elif !defined(__mips__)
unsigned short color_to_5551(Vec4 *c, float alpha_ref)
{
    unsigned int r = (unsigned int)(sat(c->x) * 31.0f);
    unsigned int g = (unsigned int)(sat(c->y) * 31.0f);
    unsigned int b = (unsigned int)(sat(c->z) * 31.0f);

    return (unsigned short)((c->w <= alpha_ref ? 0 : 0x8000) | b << 10 | g << 5 | r);
}
#else
unsigned short color_to_5551(Vec4 *c, float alpha_ref);
#endif

/* from RGBA8888 (r in the low byte) */
#ifdef __MWERKS__
Vec4 *color_unpack(Vec4 *out, unsigned int rgba);
#elif !defined(__mips__)
Vec4 *color_unpack(Vec4 *out, unsigned int rgba)
{
    out->x = (float)(rgba & 0xFF) / 255.0f;
    out->y = (float)(rgba >> 8 & 0xFF) / 255.0f;
    out->z = (float)(rgba >> 16 & 0xFF) / 255.0f;
    out->w = (float)(rgba >> 24) / 255.0f;
    return out;
}
#else
Vec4 *color_unpack(Vec4 *out, unsigned int rgba)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "mtv %2, S010\n"
        ".word 0xD0380100 /* vuc2ifs.s C000, S010 */\n"
        "vi2f.q C000, C000, 31\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (rgba)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
asm Vec4 *color_from_4444(Vec4 *out, unsigned short c)
{
    andi v1, a1, 0xffff
    move v0, a0
    ext t0, v1, 0x0, 0x4
    ext t1, v1, 0x4, 0x4
    ext t2, v1, 0x8, 0x4
    ext t3, v1, 0xc, 0x4
    lui at, 0x3d88
    ori at, at, 0x8889
    mtv at, S010
    mtv t0, S000
    mtv t1, S001
    mtv t2, S002
    mtv t3, S003
    vi2f.q C000, C000, 0
    vscl.q C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *color_from_4444(Vec4 *out, unsigned short c)
{
    out->x = (float)(c & 0xF) * (1.0f / 15.0f);
    out->y = (float)(c >> 4 & 0xF) * (1.0f / 15.0f);
    out->z = (float)(c >> 8 & 0xF) * (1.0f / 15.0f);
    out->w = (float)(c >> 12 & 0xF) * (1.0f / 15.0f);
    return out;
}
#else
Vec4 *color_from_4444(Vec4 *out, unsigned short c);
#endif

/* from RGBA5551: alpha is `alpha` when the alpha bit is set, else 0 */
#ifdef __MWERKS__
asm Vec4 *color_from_5551(Vec4 *out, unsigned short c, float alpha)
{
    andi v1, a1, 0xffff
    move v0, a0
    ext t0, v1, 0x0, 0x5
    ext t1, v1, 0x5, 0x5
    ext t2, v1, 0xa, 0x5
    lui at, 0x3d04
    ori at, at, 0x2108
    mtv at, S010
    mtv t0, S000
    mtv t1, S001
    mtv t2, S002
    vi2f.q C000, C000, 0
    vscl.q C000, C000, S010
    mfc1 t1, $f12
    andi t0, v1, 0x8000
    movz t1, zero, t0
    sv.q C000, 0(a0)
    sw t1, 12(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *color_from_5551(Vec4 *out, unsigned short c, float alpha)
{
    out->x = (float)(c & 0x1F) * (1.0f / 31.0f);
    out->y = (float)(c >> 5 & 0x1F) * (1.0f / 31.0f);
    out->z = (float)(c >> 10 & 0x1F) * (1.0f / 31.0f);
    out->w = (c & 0x8000) ? alpha : 0.0f;
    return out;
}
#else
Vec4 *color_from_5551(Vec4 *out, unsigned short c, float alpha);
#endif

#ifdef NON_MATCHING /* PSP: vfpu_color_gcc.c */
/* from RGB565 with the given alpha. As in the original, green is read from bits 1-6 and the scales
   are 1/15 for red and blue and 1/31 for green. */
Vec4 *color_from_565(Vec4 *out, unsigned short c, float alpha)
{
    out->x = (float)(c & 0x1F) * 0.0666666701f;
    out->y = (float)(c >> 1 & 0x3F) * 0.0322580636f;
    out->z = (float)(c >> 11) * 0.0666666701f;
    out->w = alpha;
    return out;
}
#endif

#ifdef __MWERKS__
Vec4 *color_add(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_add(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w + b->w;
    return out;
}
#else
Vec4 *color_add(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vadd.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
Vec4 *color_add_rgb(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_add_rgb(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w;
    return out;
}
#else
Vec4 *color_add_rgb(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "vadd.t C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
Vec4 *color_sub(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_sub(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w - b->w;
    return out;
}
#else
Vec4 *color_sub(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vsub.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
Vec4 *color_sub_rgb(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_sub_rgb(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    out->w = a->w;
    return out;
}
#else
Vec4 *color_sub_rgb(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "vsub.t C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
Vec4 *color_mul(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_mul(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    out->w = a->w * b->w;
    return out;
}
#else
Vec4 *color_mul(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vmul.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
Vec4 *color_mul_rgb(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_mul_rgb(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = a->x * b->x;
    out->y = a->y * b->y;
    out->z = a->z * b->z;
    out->w = a->w;
    return out;
}
#else
Vec4 *color_mul_rgb(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "lv.q C010, 0(%3)\n"
        "vmul.t C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* out = 1 - c */
#ifdef __MWERKS__
Vec4 *color_neg(Vec4 *out, Vec4 *c);
#elif !defined(__mips__)
Vec4 *color_neg(Vec4 *out, Vec4 *c)
{
    out->x = 1.0f - c->x;
    out->y = 1.0f - c->y;
    out->z = 1.0f - c->z;
    out->w = 1.0f - c->w;
    return out;
}
#else
Vec4 *color_neg(Vec4 *out, Vec4 *c)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vocp.q C000, C000\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (c)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
Vec4 *color_neg_rgb(Vec4 *out, Vec4 *c);
#elif !defined(__mips__)
Vec4 *color_neg_rgb(Vec4 *out, Vec4 *c)
{
    out->x = 1.0f - c->x;
    out->y = 1.0f - c->y;
    out->z = 1.0f - c->z;
    out->w = c->w;
    return out;
}
#else
Vec4 *color_neg_rgb(Vec4 *out, Vec4 *c)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vocp.t C000, C000\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (c)
        : "memory");
    return out;
}
#endif

/* out = a + (b - a) * t */
#ifdef __MWERKS__
asm Vec4 *vec4_lerp2(Vec4 *out, Vec4 *a, Vec4 *b, float t)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S030
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vsub.q C000, C020, C010
    vscl.q C000, C000, S030
    vadd.q C010, C010, C000
    sv.q C010, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *vec4_lerp2(Vec4 *out, Vec4 *a, Vec4 *b, float t)
{
    out->x = a->x + (b->x - a->x) * t;
    out->y = a->y + (b->y - a->y) * t;
    out->z = a->z + (b->z - a->z) * t;
    out->w = a->w + (b->w - a->w) * t;
    return out;
}
#else
Vec4 *vec4_lerp2(Vec4 *out, Vec4 *a, Vec4 *b, float t);
#endif

#ifdef __MWERKS__
asm Vec4 *color_lerp_rgb(Vec4 *out, Vec4 *a, Vec4 *b, float t)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S030
    lv.q C010, 0(a1)
    lv.q C020, 0(a2)
    vsub.t C000, C020, C010
    vscl.t C000, C000, S030
    vadd.t C010, C010, C000
    sv.q C010, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *color_lerp_rgb(Vec4 *out, Vec4 *a, Vec4 *b, float t)
{
    out->x = a->x + (b->x - a->x) * t;
    out->y = a->y + (b->y - a->y) * t;
    out->z = a->z + (b->z - a->z) * t;
    out->w = a->w;
    return out;
}
#else
Vec4 *color_lerp_rgb(Vec4 *out, Vec4 *a, Vec4 *b, float t);
#endif

#ifdef __MWERKS__
asm Vec4 *color_scale(Vec4 *out, Vec4 *c, float s)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S020
    lv.q C010, 0(a1)
    vscl.q C000, C010, S020
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *color_scale(Vec4 *out, Vec4 *c, float s)
{
    out->x = c->x * s;
    out->y = c->y * s;
    out->z = c->z * s;
    out->w = c->w * s;
    return out;
}
#else
Vec4 *color_scale(Vec4 *out, Vec4 *c, float s);
#endif

#ifdef __MWERKS__
asm Vec4 *color_scale_rgb(Vec4 *out, Vec4 *c, float s)
{
    move v0, a0
    mfc1 t0, $f12
    mtv t0, S010
    lv.q C000, 0(a1)
    vscl.t C000, C000, S010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *color_scale_rgb(Vec4 *out, Vec4 *c, float s)
{
    out->x = c->x * s;
    out->y = c->y * s;
    out->z = c->z * s;
    out->w = c->w;
    return out;
}
#else
Vec4 *color_scale_rgb(Vec4 *out, Vec4 *c, float s);
#endif

/* each channel clamped to [lo, hi] */
#ifdef __MWERKS__
asm Vec4 *color_clamp(Vec4 *out, Vec4 *c, float lo, float hi)
{
    move v0, a0
    mfc1 t0, $f12
    mfc1 t1, $f13
    mtv t0, S010
    mtv t1, S011
    lv.q C000, 0(a1)
    vpfxt X, X, X, X
    vmax.q C000, C000, C010
    vpfxt Y, Y, Y, Y
    vmin.q C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *color_clamp(Vec4 *out, Vec4 *c, float lo, float hi)
{
    out->x = fminf(fmaxf(c->x, lo), hi);
    out->y = fminf(fmaxf(c->y, lo), hi);
    out->z = fminf(fmaxf(c->z, lo), hi);
    out->w = fminf(fmaxf(c->w, lo), hi);
    return out;
}
#else
Vec4 *color_clamp(Vec4 *out, Vec4 *c, float lo, float hi);
#endif

#ifdef __MWERKS__
asm Vec4 *color_clamp_rgb(Vec4 *out, Vec4 *c, float lo, float hi)
{
    move v0, a0
    mfc1 t0, $f12
    mfc1 t1, $f13
    mtv t0, S010
    mtv t1, S011
    lv.q C000, 0(a1)
    vpfxt X, X, X, W
    vmax.t C000, C000, C010
    vpfxt Y, Y, Y, W
    vmin.t C000, C000, C010
    sv.q C000, 0(a0)
    jr ra
    nop
}
#elif !defined(__mips__)
Vec4 *color_clamp_rgb(Vec4 *out, Vec4 *c, float lo, float hi)
{
    out->x = fminf(fmaxf(c->x, lo), hi);
    out->y = fminf(fmaxf(c->y, lo), hi);
    out->z = fminf(fmaxf(c->z, lo), hi);
    out->w = c->w;
    return out;
}
#else
Vec4 *color_clamp_rgb(Vec4 *out, Vec4 *c, float lo, float hi);
#endif

#ifdef __MWERKS__
Vec4 *color_max(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_max(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = fmaxf(a->x, b->x);
    out->y = fmaxf(a->y, b->y);
    out->z = fmaxf(a->z, b->z);
    out->w = fmaxf(a->w, b->w);
    return out;
}
#else
Vec4 *color_max(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vmax.q C000, C010, C020\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

/* Note: the original computes vmin.q C000, C000, C010, i.e. min(<stale C000>, a), never reading b
   (it loads a and b into C010 and C020); the C version gives the intended min(a, b). */
#ifdef __MWERKS__
Vec4 *color_min(Vec4 *out, Vec4 *a, Vec4 *b);
#elif !defined(__mips__)
Vec4 *color_min(Vec4 *out, Vec4 *a, Vec4 *b)
{
    out->x = fminf(a->x, b->x);
    out->y = fminf(a->y, b->y);
    out->z = fminf(a->z, b->z);
    out->w = fminf(a->w, b->w);
    return out;
}
#else
Vec4 *color_min(Vec4 *out, Vec4 *a, Vec4 *b)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C010, 0(%2)\n"
        "lv.q C020, 0(%3)\n"
        "vmin.q C000, C000, C010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (a), "r" (b)
        : "memory");
    return out;
}
#endif

#ifdef __MWERKS__
asm int color_equal(Vec4 *a, Vec4 *b)
{
    lv.q C000, 0(a0)
    lv.q C010, 0(a1)
    move v0, zero
    vcmp.q EQ, C000, C010
    bvtl 5, L_0881D710
    addiu v0, zero, 1
L_0881D710:
    jr ra
    nop
}
#elif !defined(__mips__)
int color_equal(Vec4 *a, Vec4 *b)
{
    return a->x == b->x && a->y == b->y && a->z == b->z && a->w == b->w;
}
#else
int color_equal(Vec4 *a, Vec4 *b);
#endif

/* 1 when every channel is +0.0 or -0.0 (bit test) */
#ifdef __MWERKS__
asm int color_is_zero(Vec4 *c)
{
    lw v0, 4(a0)
    lw a1, 0(a0)
    lw t0, 8(a0)
    lw a3, 12(a0)
    or a2, a1, v0
    or a0, a2, t0
    or v1, a0, a3
    ins v1, zero, 0x1f, 0x1
    jr ra
    sltiu v0, v1, 1
}
#elif !defined(__mips__)
int color_is_zero(Vec4 *c)
{
    unsigned int *w = (unsigned int *)c;

    return ((w[0] | w[1] | w[2] | w[3]) & 0x7FFFFFFF) == 0;
}
#else
int color_is_zero(Vec4 *c);
#endif

/* out = c / max(channels), clamped to [0, 1]. The scale is 0 when the second value of the vsrt4/vsrt3
   sort, min(max(r, a), max(g, b)), is <= 0. */
#ifdef __MWERKS__
Vec4 *color_normalize(Vec4 *out, Vec4 *c);
#elif !defined(__mips__)
Vec4 *color_normalize(Vec4 *out, Vec4 *c)
{
    float m = fmaxf(fmaxf(c->x, c->w), fmaxf(c->y, c->z));
    float k = fminf(fmaxf(c->x, c->w), fmaxf(c->y, c->z)) <= 0.0f ? 0.0f : 1.0f / m;

    out->x = sat(c->x * k);
    out->y = sat(c->y * k);
    out->z = sat(c->z * k);
    out->w = sat(c->w * k);
    return out;
}
#else
Vec4 *color_normalize(Vec4 *out, Vec4 *c)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vsrt4.q C010, C000\n"
        "vzero.s S020\n"
        "vsrt3.q C010, C010\n"
        "vcmp.s LE, S011, S020\n"
        "vrcp.s S010, S010\n"
        "vcmovt.s S010, S020, 0\n"
        ".word 0xDE000055  /* vpfxd [0:1],[0:1],[0:1],[0:1] */\n"
        "vscl.q C000, C000, S010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (c)
        : "memory");
    return out;
}
#endif

/* rgb / max(r, g, b, 0) clamped to [0, 1] (0 when the maximum is <= 0); alpha kept */
#ifdef __MWERKS__
Vec4 *color_normalize_rgb(Vec4 *out, Vec4 *c);
#elif !defined(__mips__)
Vec4 *color_normalize_rgb(Vec4 *out, Vec4 *c)
{
    float m = fmaxf(fmaxf(c->x, c->y), fmaxf(c->z, 0.0f));
    float k = m <= 0.0f ? 0.0f : 1.0f / m;

    out->x = sat(c->x * k);
    out->y = sat(c->y * k);
    out->z = sat(c->z * k);
    out->w = c->w;
    return out;
}
#else
Vec4 *color_normalize_rgb(Vec4 *out, Vec4 *c)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "lv.q C000, 0(%2)\n"
        "vzero.s S003\n"
        "vsrt4.q C010, C000\n"
        "vzero.s S020\n"
        "vsrt3.q C010, C010\n"
        "vcmp.s LE, S010, S020\n"
        "vrcp.s S010, S010\n"
        "vcmovt.s S010, S020, 0\n"
        ".word 0xDE000815  /* vpfxd [0:1],[0:1],[0:1],m */\n"
        "vscl.t C000, C000, S010\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out), "r" (c)
        : "memory");
    return out;
}
#endif

/* Next vrndf1 value in [1, 2) without advancing the generator (state saved and restored around it). */
#ifdef __MWERKS__
asm float vrand_peekf(void)
{
    addiu sp, sp, -64
    sw ra, 48(sp)
    jal vfpu_rng_save
    move a0, sp
    vrndf1.s S000
    sv.s S000, 32(sp)
    jal vfpu_rng_load
    move a0, sp
    lwc1 $f0, 32(sp)
    lw ra, 48(sp)
    jr ra
    addiu sp, sp, 64
}
#elif !defined(__mips__)
float vrand_peekf(void)
{
    unsigned int st[8];
    float r;

    vfpu_rng_save(st);
    r = vfpu_rndf1();
    vfpu_rng_load(st);
    return r;
}
#else
float vrand_peekf(void);
#endif

#ifdef NON_MATCHING /* PSP: vfpu_color_gcc.c */
/* uniform in [0, max) */
float vrand_float(float max)
{
    return (vfpu_rndf1() - 1.0f) * max;
}
#endif

/* uniform in [0, pi) */
#ifdef __MWERKS__
asm float vrand_pi(void)
{
    addiu sp, sp, -16
    vrndf1.s S000
    vcst.s S001, VFPU_PI
    vpfxt 1, Y, Z, W
    vsub.s S000, S000, S000
    vmul.s S000, S000, S001
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#elif !defined(__mips__)
float vrand_pi(void)
{
    return (vfpu_rndf1() - 1.0f) * 3.14159265f;
}
#else
float vrand_pi(void);
#endif

/* uniform in [0, 2/pi) (VFPU_2_PI, as in the original) */
#ifdef __MWERKS__
asm float vrand_2_pi(void)
{
    addiu sp, sp, -16
    vrndf1.s S000
    vcst.s S001, VFPU_2_PI
    vpfxt 1, Y, Z, W
    vsub.s S000, S000, S000
    vmul.s S000, S000, S001
    sv.s S000, 0(sp)
    lwc1 $f0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#elif !defined(__mips__)
float vrand_2_pi(void)
{
    return (vfpu_rndf1() - 1.0f) * 0.636619772f;
}
#else
float vrand_2_pi(void);
#endif

/* four channels uniform in [0, 1) */
#ifdef __MWERKS__
Vec4 *vrand_color(Vec4 *out);
#elif !defined(__mips__)
Vec4 *vrand_color(Vec4 *out)
{
    out->x = vfpu_rndf1() - 1.0f;
    out->y = vfpu_rndf1() - 1.0f;
    out->z = vfpu_rndf1() - 1.0f;
    out->w = vfpu_rndf1() - 1.0f;
    return out;
}
#else
Vec4 *vrand_color(Vec4 *out)
{
    __asm__ (
        ".set push\n"
        ".set noreorder\n"
        "vrndf1.q C000\n"
        "vpfxt 1, 1, 1, 1\n"
        "vsub.q C000, C000, C000\n"
        "sv.q C000, 0(%1)\n"
        ".set pop\n"
        : "=m" (*out)
        : "r" (out)
        : "memory");
    return out;
}
#endif

#ifdef NON_MATCHING /* PSP: vfpu_color_gcc.c */
/* uniform integer in [0, n) */
int vrand_int(int n)
{
    return (int)((vfpu_rndf1() - 1.0f) * (float)n);
}
#endif
