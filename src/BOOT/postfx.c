// @range 0x0881FFB0-0x08820ED4
// @asm 0x0882009C-0x088209FC
// @asm 0x08820B1C-0x08820EBC
/* Full-screen effects drawn from the frame buffer through two 256x128 work textures in VRAM
   (0x1F0000 and 0x1E0000): a shrink, a darkened copy and additive blends of offset copies
   (blur/glow), and a threshold pass through a CLUT that keeps only the bright half. The vertex
   lists (sprite pairs) are in the data. */
#include "psp.h"

extern unsigned short postfx_vtx_088EB9E0[];
extern unsigned short postfx_vtx_088EBA00[];
extern unsigned short postfx_vtx_088EBB00[];
extern unsigned short postfx_vtx_088EBC00[];
extern unsigned short postfx_vtx_088EBC40[];
extern unsigned short postfx_vtx_088EBC60[];
extern unsigned short postfx_vtx_088EBD60[];
extern unsigned short postfx_vtx_088EC160[];
extern unsigned short postfx_vtx_088EC1E0[];
extern unsigned short postfx_vtx_088EC200[];
extern unsigned short postfx_vtx_088EC300[];
extern unsigned short postfx_vtx_088EC500[];
extern unsigned short postfx_vtx_088EC580[];
extern unsigned short postfx_vtx_088EC600[];
extern unsigned short postfx_vtx_088EC620[];
extern unsigned short postfx_vtx_088EC720[];
extern unsigned short postfx_vtx_088EC920[];
extern unsigned short postfx_vtx_088EC9A0[];
extern unsigned short postfx_vtx_088ECA20[];
extern unsigned short postfx_vtx_088ECA40[];
extern unsigned short postfx_vtx_088ECB40[];
extern unsigned short postfx_vtx_088ECC40[];
extern unsigned short postfx_vtx_088ECCC0[];
extern unsigned short postfx_vtx_088ECCE0[];
extern unsigned short postfx_vtx_088ECDE0[];
extern unsigned short postfx_vtx_088ECEE0[];
extern unsigned short postfx_vtx_088ECF60[];
extern unsigned short postfx_vtx_088ECF80[];
extern unsigned short postfx_vtx_088ED080[];
extern unsigned short postfx_vtx_088ED180[];
extern unsigned int postfx_clut_a[256];  /* 0x088F9800 */
extern unsigned int postfx_clut_b[256];  /* 0x088F9C00 */
extern int postfx_param;                 /* 0x088FA000 */

void *sceGeEdramGetAddr(void);
unsigned int *gfx_get_state(void);
unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_disable(unsigned int *list, int state);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *dl_tex_sync(unsigned int *list);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *dl_tex_wrap(unsigned int *list, int u, int v);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_draw_array(unsigned int *list, int prim, unsigned int vtype, int count, void *indices, void *vertices);
unsigned int *dl_clut_mode(unsigned int *list, int psm, int shift, int mask, int start);
unsigned int *dl_clut_load(unsigned int *list, int n, void *clut);
unsigned int *dl_stencil_op(unsigned int *list, int fail, int zfail, int zpass);
unsigned int *dl_stencil_func(unsigned int *list, int func, int ref, int mask);

void postfx_blur(unsigned int *list, int src, int dst, int w, void *v0, int n0, void *v1, int n1, void *v2, int n2, void *v3, int n3);

void postfx_clear_depth(unsigned int *list)
{
    dl_clear(dl_clear_depth(list, 0), 0x12);
}

void postfx_blur_a(unsigned int *list)
{
    postfx_blur(list, 0x1F8000, 0x1F0000, 0x80, postfx_vtx_088EB9E0, 2, postfx_vtx_088EBA00, 16, postfx_vtx_088EBB00, 16, postfx_vtx_088EBC00, 4);
}

void postfx_blur_b(unsigned int *list)
{
    postfx_blur(list, 0x1F0000, 0x1E0000, 0x100, postfx_vtx_088EBC40, 2, postfx_vtx_088EBC60, 16, postfx_vtx_088EBD60, 64, postfx_vtx_088EC160, 8);
}

#ifdef NON_MATCHING /* the original keeps the vertex table addresses in $s registers */
void postfx_glow_a(unsigned int *list)
{
    char *edram;
    int fb;
    char *frame;
    unsigned int i;

    if (postfx_clut_a[1] == 0) {
        postfx_clut_a[0] = 0;
        for (i = 1; i < 0x40; i++) {
            postfx_clut_a[i] = 0xFFFFFFFF;
        }
        for (i = 0x40; i < 0x100; i++) {
            postfx_clut_a[i] = 0;
        }
    }
    edram = sceGeEdramGetAddr();
    if (!(*gfx_get_state() & 1)) {
        fb = 0x88000;
    } else {
        fb = 0;
    }
    frame = edram + fb;
    list = gfx_set_draw_target(list, 0x1F0000, 0xFFFFFFFE, 0x100, 0x80, 0);
    list = dl_disable(list, 1);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_func(list, 4, 1);
    list = dl_tex_image(list, 0, 0x200, 0x200, 0x200, frame);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_tex_wrap(list, 1, 1);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 10, 0xFFFFFFFF, 0);
    list = dl_draw_array(list, 6, 0x800182, 0x10, 0, postfx_vtx_088EC200);
    list = gfx_set_default_target(list);
    list = gfx_set_draw_target(list, 0x1E0000, 0xFFFFFFFE, 0x100, 0x80, 0);
    list = dl_disable(list, 3);
    list = dl_disable(list, 1);
    list = dl_disable(list, 9);
    list = dl_color(list, 0);
    list = dl_draw_array(list, 6, 0x800182, 2, 0, postfx_vtx_088EC1E0);
    list = dl_enable(list, 9);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_tex_mode(list, 0, 0, 0, 0);
    list = dl_tex_func(list, 0, 1);
    list = dl_tex_image(list, 0, 0x100, 0x80, 0x100, edram + 0x1F0000);
    list = dl_enable(list, 4);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_blend_func(list, 0, 10, 10, 0xFBFBFBFB, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, 8, 0, postfx_vtx_088EC500);
    list = dl_blend_func(list, 2, 10, 10, 0x3F3F3F3F, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, 32, 0, postfx_vtx_088EC300);
    list = gfx_set_default_target(list);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_tex_mode(list, 6, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x100, 0x80, 0x100, edram + 0x1E0000);
    list = dl_tex_func(list, 1, 0);
    list = dl_clut_mode(list, 3, 5, 0x3F, 0);
    list = dl_clut_load(list, 0x20, postfx_clut_a);
    list = dl_blend_func(list, 2, 10, 10, 0xFFFFFFFF, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, 8, 0, postfx_vtx_088EC580);
    list = dl_draw_array(list, 6, 0x800182, 8, 0, postfx_vtx_088EC580);
    list = dl_disable(list, 9);
    dl_disable(list, 4);
}
#endif

#ifdef NON_MATCHING /* same as postfx_glow_a */
void postfx_glow_b(unsigned int *list)
{
    char *edram;
    int fb;
    char *frame;
    unsigned int i;

    if (postfx_clut_b[1] == 0) {
        postfx_clut_b[0] = 0;
        for (i = 1; i < 0x40; i++) {
            postfx_clut_b[i] = 0xFFFFFFFF;
        }
        for (i = 0x40; i < 0x100; i++) {
            postfx_clut_b[i] = 0;
        }
    }
    edram = sceGeEdramGetAddr();
    if (!(*gfx_get_state() & 1)) {
        fb = 0x88000;
    } else {
        fb = 0;
    }
    frame = edram + fb;
    list = gfx_set_draw_target(list, 0x1F0000, 0xFFFFFFFE, 0x100, 0x80, 0);
    list = dl_disable(list, 1);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_func(list, 4, 1);
    list = dl_tex_image(list, 0, 0x200, 0x200, 0x200, frame);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_tex_wrap(list, 1, 1);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 10, 0xFFFFFFFF, 0);
    list = dl_draw_array(list, 6, 0x800182, 0x10, 0, postfx_vtx_088EC620);
    list = gfx_set_default_target(list);
    list = gfx_set_draw_target(list, 0x1E0000, 0xFFFFFFFE, 0x100, 0x80, 0);
    list = dl_disable(list, 3);
    list = dl_disable(list, 1);
    list = dl_disable(list, 9);
    list = dl_color(list, 0);
    list = dl_draw_array(list, 6, 0x800182, 2, 0, postfx_vtx_088EC600);
    list = dl_enable(list, 9);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_tex_mode(list, 0, 0, 0, 0);
    list = dl_tex_func(list, 0, 1);
    list = dl_tex_image(list, 0, 0x100, 0x80, 0x100, edram + 0x1F0000);
    list = dl_enable(list, 4);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_blend_func(list, 0, 10, 10, 0x3F3F3F3F, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, 32, 0, postfx_vtx_088EC720);
    list = dl_blend_func(list, 2, 10, 10, 0xFBFBFBFB, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, 8, 0, postfx_vtx_088EC920);
    list = gfx_set_default_target(list);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_tex_mode(list, 6, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x100, 0x80, 0x100, edram + 0x1E0000);
    list = dl_tex_func(list, 1, 0);
    list = dl_clut_mode(list, 3, 5, 0x3F, 0);
    list = dl_clut_load(list, 0x20, postfx_clut_b);
    list = dl_blend_func(list, 2, 10, 10, 0xFFFFFFFF, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, 8, 0, postfx_vtx_088EC9A0);
    list = dl_draw_array(list, 6, 0x800182, 8, 0, postfx_vtx_088EC9A0);
    list = dl_disable(list, 9);
    dl_disable(list, 4);
}
#endif

void postfx_blur_c(unsigned int *list)
{
    postfx_blur(list, 0x1F0000, 0x1E0000, 0x100, postfx_vtx_088ECA20, 2, postfx_vtx_088ECA40, 16, postfx_vtx_088ECB40, 16, postfx_vtx_088ECC40, 8);
}

void postfx_blur_d(unsigned int *list)
{
    postfx_blur(list, 0x1F0000, 0x1E0000, 0x100, postfx_vtx_088ECCC0, 2, postfx_vtx_088ECCE0, 16, postfx_vtx_088ECDE0, 16, postfx_vtx_088ECEE0, 8);
}

void postfx_blur_e(unsigned int *list)
{
    postfx_blur(list, 0x1F0000, 0x1E0000, 0x100, postfx_vtx_088ECF60, 2, postfx_vtx_088ECF80, 16, postfx_vtx_088ED080, 16, postfx_vtx_088ED180, 8);
}

#ifdef NON_MATCHING /* the original spills the frame texture address instead of a parameter */
/* Shrinks the frame buffer into `src` (v1), darkens `dst` (v0), adds `src` into `dst` (v2) and
   draws `dst` back through the stencil (v3). */
void postfx_blur(unsigned int *list, int src, int dst, int w, void *v0, int n0, void *v1, int n1, void *v2, int n2, void *v3, int n3)
{
    char *edram;
    int fb;
    char *frame;

    edram = sceGeEdramGetAddr();
    if (!(*gfx_get_state() & 1)) {
        fb = 0x88000;
    } else {
        fb = 0;
    }
    frame = edram + fb;
    list = gfx_set_draw_target(list, src, 0xFFFFFFFE, w, 0x80, 0);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_disable(list, 1);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_func(list, 4, 1);
    list = dl_set_tex_filter(list, 1, 1);
    list = dl_tex_image(list, 0, 0x200, 0x200, 0x200, frame);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_tex_wrap(list, 1, 1);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 6, 10, 0xFFFFFFFF, 0);
    list = dl_draw_array(list, 6, 0x800182, n1, 0, v1);
    list = gfx_set_default_target(list);
    list = gfx_set_draw_target(list, dst, 0xFFFFFFFE, w, 0x80, 0);
    list = dl_disable(list, 3);
    list = dl_disable(list, 1);
    list = dl_disable(list, 9);
    list = dl_color(list, 0);
    list = dl_draw_array(list, 6, 0x800182, n0, 0, v0);
    list = dl_enable(list, 9);
    list = dl_color(list, 0xFFFFFFFF);
    list = dl_tex_mode(list, 0, 0, 0, 0);
    list = dl_tex_func(list, 0, 1);
    list = dl_tex_image(list, 0, w, 0x80, w, edram + src);
    list = dl_enable(list, 4);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_blend_func(list, 0, 10, 10, 0xFFFFFFFF, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, n2, 0, v2);
    list = gfx_set_default_target(list);
    list = dl_stencil_op(list, 3, 0xFF, 0xFF);
    list = dl_stencil_func(list, 0, 0, 0);
    list = dl_enable(list, 3);
    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_tex_image(list, 0, w, 0x80, w, edram + dst);
    list = dl_tex_func(list, 1, 0);
    list = dl_blend_func(list, 2, 10, 10, 0xFFFFFFFF, 0xFFFFFFFF);
    list = dl_draw_array(list, 6, 0x800182, n3, 0, v3);
    list = dl_disable(list, 9);
    dl_disable(list, 4);
}
#endif

int postfx_get_param(void)
{
    return postfx_param;
}

void postfx_set_param(int v)
{
    postfx_param = v;
}
