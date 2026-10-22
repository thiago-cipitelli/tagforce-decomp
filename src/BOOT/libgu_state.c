// @range 0x08805EC4-0x08807070
// @asm 0x08805EC4-0x08807070
/* libgu of the Sony SDK (GCC; names from the export NIDs): primitives, lighting, texture,
   blending and the other state commands. Every function reaches the current context through the
   libgu globals (%hi/%lo), so all of them stay in the split asm; the C below (NON_MATCHING) writes
   the same GE words, through the context builders of libgu_ctx.c where the original does. */

#ifdef NON_MATCHING /* GCC: the current context is loaded through %hi/%lo */
#include "libgu.h"

void gu_ctx_sprite(GuList *l, int x, int y, int z, int u, int v, int flip, int rot);
void gu_ctx_clear(GuList *l, int flags);
void gu_ctx_depth_range(GuList *l, int near, int far);
void gu_ctx_depth_offset(GuList *l, int offset);
void gu_ctx_patch_prim(GuList *l, int prim);
void gu_ctx_material(GuList *l, int mode, unsigned int color);
void gu_ctx_model_color(GuList *l, unsigned int emissive, unsigned int ambient, unsigned int diffuse,
                        unsigned int specular);
void gu_ctx_front_face(GuList *l, int order);
void gu_ctx_shade_model(GuList *l, int mode);
void gu_ctx_light(GuList *l, int light, int type, int components, const float *pos);
void gu_ctx_light_color(GuList *l, int light, int component, unsigned int color);
void gu_ctx_tex_mode(GuList *l, int tpsm, int maxmips, int a2, int swizzle);
void gu_ctx_scissor(GuList *l, int x0, int y0, int x1, int y1);

/* relative BASE + address command pair (VADDR 0x01 / IADDR 0x02) */
static void gu_addr(unsigned int cmd, const void *p)
{
    unsigned int a = (unsigned int)p;

    GU_PUSH((((a & 0xFFFFFFF) >> 24) << 16) | 0x10000000);
    GU_PUSH((a & 0xFFFFFF) | cmd);
}

static void gu_vertex_setup(int vtype, const void *indices, const void *vertices)
{
    if (vtype != 0) {
        GU_PUSH((vtype & 0xFFFFFF) | 0x12000000);
    }
    if (indices != 0) {
        gu_addr(0x02000000, indices);
    }
    if (vertices != 0) {
        gu_addr(0x01000000, vertices);
    }
}

/* BBOX test of count vertices; the BJUMP target is patched by sceGuEndObject */
void sceGuBeginObject(int vtype, int count, const void *indices, const void *vertices)
{
    GuList *l = gu.list;
    unsigned int *p;

    gu_vertex_setup(vtype, indices, vertices);
    p = l->cur;
    p[0] = count | 0x07000000;
    l->cur = p + 3;
    l->obj_stack[l->obj_depth++] = p + 1;
    p[1] = 0x10000000;
    p[2] = 0x09000000;
}

void sceGuDrawArray(int prim, int vtype, int count, const void *indices, const void *vertices)
{
    gu_vertex_setup(vtype, indices, vertices);
    GU_PUSH((prim & 7) << 16 | count | 0x04000000);
    gu_update_stall();
}

/* n PRIM commands of the same vertices */
void sceGuDrawArrayN(int prim, int vtype, int count, int n, const void *indices, const void *vertices)
{
    gu_vertex_setup(vtype, indices, vertices);
    while (n-- > 0) {
        GU_PUSH((prim & 7) << 16 | count | 0x04000000);
    }
    gu_update_stall();
}

void sceGuDrawBezier(int vtype, int ucount, int vcount, const void *indices, const void *vertices)
{
    gu_vertex_setup(vtype, indices, vertices);
    GU_PUSH(vcount << 8 | ucount | 0x05000000);
    gu_update_stall();
}

void sceGuDrawSpline(int vtype, int ucount, int vcount, int uedge, int vedge, const void *indices,
                     const void *vertices)
{
    gu_vertex_setup(vtype, indices, vertices);
    GU_PUSH(vedge << 18 | uedge << 16 | vcount << 8 | ucount | 0x06000000);
    gu_update_stall();
}

/* size of the sprites of gu_draw_sprite: texture width/height and screen width/height */
void gu_sprite_size(int uw, int vh, int xw, int yh)
{
    GuList *l = gu.list;

    l->pad38[3] = yh;
    l->pad38[0] = uw;
    l->pad38[1] = vh;
    l->pad38[2] = xw;
}

/* one textured sprite at (x, y, z) from (u, v); flip bit 0 mirrors u, bit 1 v; rot 1-3 */
void gu_draw_sprite(int x, int y, int z, int u, int v, int flip, int rot)
{
    gu_ctx_sprite(gu.list, x, y, z, u, v, flip, rot);
    gu_update_stall();
}

/* TRXSBP/TRXSBW/TRXPOS, TRXDBP/TRXDBW/TRXDPOS, TRXSIZE, TRXKICK (32-bit when psm == 3); nothing
   when a coordinate pair passes 1023 */
void sceGuCopyImage(int psm, int sx, int sy, int width, int height, int srcw, void *src, int dx,
                    int dy, int destw, void *dest)
{
    unsigned int s = (unsigned int)src;
    unsigned int d = (unsigned int)dest;

    if (((sx + width) | (dx + width)) & 0xFFFFFC00) {
        return;
    }
    GU_PUSH((s & 0xFFFFFF) | 0xB2000000);
    GU_PUSH((((s & 0xFFFFFFF) >> 24) << 16) | srcw | 0xB3000000);
    GU_PUSH(sy << 10 | sx | 0xEB000000);
    GU_PUSH((d & 0xFFFFFF) | 0xB4000000);
    GU_PUSH((((d & 0xFFFFFFF) >> 24) << 16) | destw | 0xB5000000);
    GU_PUSH(dy << 10 | dx | 0xEC000000);
    GU_PUSH((height - 1) << 10 | (width - 1) | 0xEE000000);
    GU_PUSH((psm == 3) | 0xEA000000);
}

/* sceGuClearColor/Stencil/Depth only record the value for sceGuClear */
void gu_set_color_mask(unsigned int color)
{
    gu.list->pad38[4] = color & 0xFFFFFF;
}

void gu_set_param_4c(unsigned int stencil)
{
    gu.list->pad38[5] = stencil;
}

void gu_set_param_50(unsigned int depth)
{
    gu.list->pad38[6] = depth;
}

void gu_set_param_494(int flags)
{
    gu_ctx_clear(gu.list, flags);
}

/* sceGuViewport: OFFSETX/Y scale and center (float24) */
void gu_Viewport(int cx, int cy, int width, int height)
{
    GU_PUSH(GU_F24((float)(width / 2)) | 0x42000000);
    GU_PUSH(GU_F24((float)(-height / 2)) | 0x43000000);
    GU_PUSH(GU_F24((float)cx) | 0x45000000);
    GU_PUSH(GU_F24((float)cy) | 0x46000000);
}

void gu_DepthRange(int near, int far)
{
    gu_ctx_depth_range(gu.list, near, far);
}

void sceGuDepthOffset(int offset)
{
    gu_ctx_depth_offset(gu.list, offset);
}

/* sceGuOffset: OFFSETX/OFFSETY in 1/16 pixel */
void gu_Offset(int x, int y)
{
    GU_PUSH(x << 4 | 0x4C000000);
    GU_PUSH(y << 4 | 0x4D000000);
}

void sceGuPatchPrim(int prim)
{
    gu_ctx_patch_prim(gu.list, prim);
}

/* AMBIENTCOLOR + AMBIENTALPHA */
void sceGuAmbient(unsigned int color)
{
    GU_PUSH((color & 0xFFFFFF) | 0x5C000000);
    GU_PUSH(color >> 24 | 0x5D000000);
}

void sceGuMaterial(int mode, unsigned int color)
{
    gu_ctx_material(gu.list, mode, color);
}

void sceGuColor(unsigned int color)
{
    gu_ctx_material(gu.list, 7, color);
}

/* MATERIALAMBIENT + MATERIALALPHA */
void sceGuAmbientColor(unsigned int color)
{
    GU_PUSH((color & 0xFFFFFF) | 0x55000000);
    GU_PUSH(color >> 24 | 0x58000000);
}

void sceGuModelColor(unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular)
{
    gu_ctx_model_color(gu.list, emissive, ambient, diffuse, specular);
}

void gu_FrontFace(int order)
{
    gu_ctx_front_face(gu.list, order);
}

void sceGuShadeModel(int mode)
{
    gu_ctx_shade_model(gu.list, mode);
}

void sceGuLight(int light, int type, int components, const float *pos)
{
    gu_ctx_light(gu.list, light, type, components, pos);
}

void sceGuLightMode(int mode)
{
    GU_PUSH((mode & 1) | 0x5E000000);
}

/* LCA/LLA/LQA of light n */
void sceGuLightAtt(float atten0, float atten1, float atten2, int light)
{
    light *= 3;
    GU_PUSH((unsigned int)(light + 0x7B) << 24 | GU_F24(atten0));
    GU_PUSH((unsigned int)(light + 0x7C) << 24 | GU_F24(atten1));
    GU_PUSH((unsigned int)(light + 0x7D) << 24 | GU_F24(atten2));
}

/* SPOTEXP, SPOTCUT and the three LDX/LDY/LDZ direction words of light n */
void sceGuLightSpot(float exponent, float cutoff, int light, const float *direction)
{
    int k = light * 3;

    GU_PUSH((unsigned int)(light + 0x87) << 24 | GU_F24(exponent));
    GU_PUSH((unsigned int)(light + 0x8B) << 24 | GU_F24(cutoff));
    GU_PUSH((unsigned int)(k + 0x6F) << 24 | GU_F24(direction[0]));
    GU_PUSH((unsigned int)(k + 0x70) << 24 | GU_F24(direction[1]));
    GU_PUSH((unsigned int)(k + 0x71) << 24 | GU_F24(direction[2]));
}

void sceGuLightColor(int light, int component, unsigned int color)
{
    gu_ctx_light_color(gu.list, light, component, color);
}

void sceGuTexMode(int tpsm, int maxmips, int a2, int swizzle)
{
    gu_ctx_tex_mode(gu.list, tpsm, maxmips, a2, swizzle);
}

void sceGuTexFilter(int min, int mag)
{
    GU_PUSH(mag << 8 | min | 0xC6000000);
}

/* TFUNC with the color-doubling bit kept in the context (+0x60) */
void sceGuTexFunc(int tfx, int tcc)
{
    GuList *l = gu.list;

    l->pad38[12] = tcc;
    l->pad38[11] = tfx;
    GU_PUSH(l->pad38[10] << 16 | tcc << 8 | tfx | 0xC9000000);
}

void sceGuTexEnvColor(unsigned int color)
{
    GU_PUSH((color & 0xFFFFFF) | 0xCA000000);
}

void sceGuTexWrap(int u, int v)
{
    GU_PUSH(v << 8 | u | 0xC7000000);
}

/* TEXLEVEL: bias * 16 clamped to a signed byte */
void sceGuTexLevelMode(float bias, int mode)
{
    extern const float gu_texlevel_scale; /* 0x088EB114 16.0 */
    int b = (int)(bias * gu_texlevel_scale);

    if (b < -128) {
        b = -128;
    }
    if (b > 127) {
        b = 127;
    }
    GU_PUSH((b & 0xFF) << 16 | mode | 0xC8000000);
}

/* TEXMAPMODE: mode and the projection mode kept in the context */
void sceGuTexMapMode(int mode, int a1, int a2)
{
    GuList *l = gu.list;

    l->pad38[14] = mode & 3;
    GU_PUSH(l->pad38[13] << 8 | (mode & 3) | 0xC0000000);
    GU_PUSH(a2 << 8 | a1 | 0xC1000000);
}

void sceGuTexProjMapMode(int mode)
{
    GuList *l = gu.list;

    l->pad38[13] = mode & 3;
    GU_PUSH((mode & 3) << 8 | l->pad38[14] | 0xC0000000);
}

void sceGuTexSync(void)
{
    GU_PUSH(0xCC000000);
}

static int gu_log2(int v)
{
    int n = 0;

    while ((1 << (n + 1)) <= v) {
        n++;
    }
    return n;
}

/* TBP/TBW/TSIZE of mip level n, then TFLUSH */
void sceGuTexImage(int mipmap, int width, int height, int tbw, const void *tbp)
{
    unsigned int p = (unsigned int)tbp;

    GU_PUSH((unsigned int)(mipmap + 0xA0) << 24 | (p & 0xFFFFFF));
    GU_PUSH((unsigned int)(mipmap + 0xA8) << 24 | (((p & 0xFFFFFFF) >> 24) << 16) | tbw);
    GU_PUSH((unsigned int)(mipmap + 0xB8) << 24 | gu_log2(height) << 8 | gu_log2(width));
    GU_PUSH(0xCB000000);
}

void sceGuTexOffset(float u, float v)
{
    GU_PUSH(GU_F24(u) | 0x4A000000);
    GU_PUSH(GU_F24(v) | 0x4B000000);
}

void sceGuTexFlush(void)
{
    GU_PUSH(0xCB000000);
}

void sceGuTexSlope(float slope)
{
    GU_PUSH(GU_F24(slope) | 0xD0000000);
}

void sceGuClutMode(int cpsm, int shift, int mask, int a3)
{
    GU_PUSH(a3 << 16 | mask << 8 | shift << 2 | cpsm | 0xC5000000);
}

void sceGuClutLoad(int num_blocks, const void *cbp)
{
    unsigned int p = (unsigned int)cbp;

    GU_PUSH((p & 0xFFFFFF) | 0xB0000000);
    GU_PUSH((((p & 0xFFFFFFF) >> 24) << 16) | 0xB1000000);
    GU_PUSH(num_blocks | 0xC4000000);
}

/* FOGCOLOR, FOG1 = far, FOG2 = 1 / (far - near) (0 when near == far) */
void sceGuFog(float near, float far, unsigned int color)
{
    extern const float gu_one;         /* 0x088EB118 1.0 */
    float d = far - near;
    float k = 0.0f;

    if (d > 0.0f || d < 0.0f) {
        k = gu_one / d;
    }
    GU_PUSH((color & 0xFFFFFF) | 0xCF000000);
    GU_PUSH(GU_F24(far) | 0xCD000000);
    GU_PUSH(GU_F24(k) | 0xCE000000);
}

void sceGuColorFunc(int func, unsigned int color, unsigned int mask)
{
    GU_PUSH(func | 0xD8000000);
    GU_PUSH((color & 0xFFFFFF) | 0xD9000000);
    GU_PUSH((mask & 0xFFFFFF) | 0xDA000000);
}

void sceGuAlphaFunc(int func, int value, int mask)
{
    GU_PUSH((mask & 0xFF) << 16 | (value & 0xFF) << 8 | func | 0xDB000000);
}

/* sceGuDepthFunc */
void gu_DepthFunc(int function)
{
    GU_PUSH(function | 0xDE000000);
}

void sceGuDepthMask(int mask)
{
    GU_PUSH(mask | 0xE7000000);
}

void sceGuPixelMask(unsigned int mask)
{
    GU_PUSH((mask & 0xFFFFFF) | 0xE8000000);
    GU_PUSH(mask >> 24 | 0xE9000000);
}

/* sceGuScissor */
void gu_Scissor(int x0, int y0, int x1, int y1)
{
    gu_ctx_scissor(gu.list, x0, y0, x1, y1);
}

void sceGuStencilFunc(int func, int ref, int mask)
{
    GU_PUSH(mask << 16 | ref << 8 | func | 0xDC000000);
}

void sceGuLogicalOp(int op)
{
    GU_PUSH((op & 0xF) | 0xE6000000);
}

void sceGuStencilOp(int fail, int zfail, int zpass)
{
    GU_PUSH(zpass << 16 | zfail << 8 | fail | 0xDD000000);
}

void sceGuBlendFunc(int op, int src, int dest, unsigned int srcfix, unsigned int destfix)
{
    GU_PUSH(op << 8 | dest << 4 | src | 0xDF000000);
    GU_PUSH((srcfix & 0xFFFFFF) | 0xE0000000);
    GU_PUSH((destfix & 0xFFFFFF) | 0xE1000000);
}

/* a 0xFF000000 word, BOFS = index * 12, then the 4x3 part of the matrix (float24) */
void sceGuBoneMatrix(int index, const float *m)
{
    int i;

    GU_PUSH(0xFF000000);
    GU_PUSH(index * 12 | 0x2A000000);
    for (i = 0; i < 4; i++) {
        GU_PUSH(GU_F24(m[i * 4 + 0]) | 0x2B000000);
        GU_PUSH(GU_F24(m[i * 4 + 1]) | 0x2B000000);
        GU_PUSH(GU_F24(m[i * 4 + 2]) | 0x2B000000);
    }
}

void sceGuMorphWeight(int index, float weight)
{
    GU_PUSH(0xFF000000);
    GU_PUSH((unsigned int)(index + 0x2C) << 24 | GU_F24(weight));
}
#endif
