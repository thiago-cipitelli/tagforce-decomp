// @range 0x08805DF4-0x08806FC4
// @rodata 0x08834E14-0x08834E3C
// @asm 0x08805DF4-0x08805FD0
// @asm 0x08805FD0-0x08806200
// @asm 0x08806200-0x08806390
// @asm 0x08806390-0x08806A14
// @asm 0x08806CE0-0x08806DF0
// @asm 0x08806E6C-0x08806ED4
/* Drawing of the SD characters: the 3D model, the sprite sheets (sd_*_sla.gim) as billboards under
   an orthographic projection, the flat sprite; the vertex quad of a sprite cell and the kind of
   drawing each chara id uses. The large draw callbacks are still in asm. */
#include "libfield.h"

void fld_chr_model_quads(int which, FieldVtx *q, int dir, int anim, int frame);
void fld_chr_sprite_quad(int which, FieldVtx *q, FieldChr *c);
void *gim_set_texture_sub(void *list, void *gim, int a, int b, int c, int frame);
void *fld_fx_draw(void *list, void *fx);
void fld_ortho_begin(void);
void fld_ortho_end(void);
FieldCamera *camera_get(void);
void mat4_transform_vec4(Vec4 *out, Mat4 *m, Vec4 *v);
void fld_sprite_quad(float x, FieldVtx *q, int tw, int th, int u, int v, int w, int h, float y,
                     unsigned int color);

#ifdef NON_MATCHING /* C novo, conferido com o asm. 4 palavras: nas duas chamadas de fld_chr_model_quads o original lê c->anim antes de c->dir (protótipo K&R e unsigned não mudam) */
/* character drawn as a model of two quads (sheet by facing: even dir tex[1], odd tex[0]) at its
   matrix, the second one moved by the light offset of field_chr_glb; then its effect */
void *fld_chr_draw_model(void *list, FieldChr *c)
{
    FieldChrGlb *g = &field_chr_glb;
    int tex = 0;
    FieldVtx *q;
    Mat4 m __attribute__((aligned(16)));
    Mat4 t __attribute__((aligned(16)));

    switch (c->dir) {
    case 1:
    case 3:
    case 5:
    case 7:
        tex = c->tex[0];
        break;
    case 0:
    case 2:
    case 4:
    case 6:
        tex = c->tex[1];
        break;
    }
    if (tex == 0)
        return list;
    list = gim_set_texture_frame(list, (void *)tex, 0);
    list = dl_enable(list, 1);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    q = dl_alloc_inline(&list, sizeof(FieldVtx) * 4);
    fld_chr_model_quads(1, q, c->dir, c->anim, c->frame);
    mtx_load((Mat4 *)c->shadow);
    list = draw_array(list, 4, 0x19F, 4, 0, q);
    q = dl_alloc_inline(&list, sizeof(FieldVtx) * 4);
    fld_chr_model_quads(0, q, c->dir, c->anim, c->frame);
    m = *(Mat4 *)c->shadow;
    mat4_identity(&t);
    mat4_translate(&t, &t, (Vec4 *)&g->light);
    mat4_mul(&m, &t, (Mat4 *)c->shadow);
    mtx_load(&m);
    list = draw_array(list, 4, 0x19F, 4, 0, q);
    mtx_pop();
    if (c->fx != 0)
        list = fld_fx_draw(list, (void *)c->fx);
    return list;
}
#endif

#ifdef NON_MATCHING /* bate (0 palavras), mas a tabela de saltos (0x08834DD4) fica entre a de fld_chr_draw_model e a de fld_chr_sprite_quad, ainda em asm; promover junto com elas */
/* the two quads of a character model (which 1 upper, 0 lower half) with the sheet cell of its facing
   pair: the idle cell, or the walk cell of frame % 6 */
void fld_chr_model_quads(int which, FieldVtx *q, int dir, int anim, int frame)
{
    int pair;
    int *e;
    int u;
    int v;
    float u1;
    float u0;
    float v0;
    float v1;

    switch (dir) {
    case 0:
    case 1:
        pair = 0;
        break;
    case 2:
    case 3:
        pair = 1;
        break;
    case 4:
    case 5:
        pair = 2;
        break;
    case 6:
    case 7:
        pair = 3;
        break;
    }
    switch (anim) {
    case 0:
        e = field_chr_cells[pair][0];
        break;
    case 1:
        e = field_chr_cells[pair][frame % 6] + 2;
        break;
    }
    if (which != 0) {
        u = e[0];
        v = e[1];
        u1 = (u + 51) / 512.0f;
        q[0].u = u1;
        v0 = v / 256.0f;
        q[0].v = v0;
        q[0].x = 0.402421862f;
        q[0].y = 1.42945313f;
        u0 = u / 512.0f;
        q[1].u = u0;
        q[1].v = v0;
        q[1].x = -0.3984375f;
        q[1].y = 1.42945313f;
        q[2].u = u1;
        v1 = (v + 61) / 256.0f;
        q[2].v = v1;
        q[2].x = 0.402421862f;
        q[2].y = 0.261875033f;
        q[3].u = u0;
        q[3].v = v1;
        q[3].x = -0.3984375f;
        q[3].y = 0.261875033f;
    } else {
        u = e[0];
        u1 = (u + 51) / 512.0f;
        v = e[1];
        q[0].u = u1;
        v0 = (v + 61) / 256.0f;
        q[0].v = v0;
        q[0].x = 0.402421862f;
        q[0].y = 0.261875033f;
        u0 = u / 512.0f;
        q[1].u = u0;
        q[1].v = v0;
        q[1].x = -0.3984375f;
        q[1].y = 0.261875033f;
        q[2].u = u1;
        v1 = (v + 84) / 256.0f;
        q[2].v = v1;
        q[2].x = 0.402421862f;
        q[2].y = -0.17835936f;
        q[3].u = u0;
        q[3].v = v1;
        q[3].x = -0.3984375f;
        q[3].y = -0.17835936f;
    }
    q[3].color = 0xFFFFFFFF;
    q[2].color = 0xFFFFFFFF;
    q[1].color = 0xFFFFFFFF;
    q[0].color = 0xFFFFFFFF;
    q[3].z = 0.0f;
    q[2].z = 0.0f;
    q[1].z = 0.0f;
    q[0].z = 0.0f;
}
#endif

#ifdef NON_MATCHING /* C novo, conferido com o asm. 2 palavras: o teste do case 2 sai beq/nop para o corpo do default no original, beql com cópia aqui (ordens dos case e default fundido testados) */
/* character drawn from its two sprite sheets: tex[1] first, then tex[0] over it, each a billboard
   quad under the orthographic projection; frames from the charmap entry; then its effect */
void *fld_chr_draw_sprite(void *list, FieldChr *c)
{
    CharmapPos p;
    FieldVtx *q;
    int frame;
    int pframe;

    if (charmap_lookup2(c->id, &p) != 0) {
        switch (p.b) {
        case 2:
            frame = 0;
            break;
        case 3:
            frame = 0;
            break;
        default:
            frame = 0;
            break;
        case 1:
            frame = 1;
            break;
        }
        pframe = p.c;
    } else {
        frame = 0;
        pframe = 0;
    }
    list = dl_enable(list, 1);
    q = dl_alloc_inline(&list, sizeof(FieldVtx) * 4);
    fld_chr_sprite_quad(1, q, c);
    fld_ortho_begin();
    list = gim_set_texture_sub(list, (void *)c->tex[1], 0, 0, 0, frame);
    list = draw_array(list, 4, 0x19F, 4, 0, q);
    fld_ortho_end();
    q = dl_alloc_inline(&list, sizeof(FieldVtx) * 4);
    fld_chr_sprite_quad(0, q, c);
    fld_ortho_begin();
    list = gim_set_texture_sub(list, (void *)c->tex[0], 0, 0, 0, pframe);
    list = draw_array(list, 4, 0x19F, 4, 0, q);
    fld_ortho_end();
    if (c->fx != 0)
        list = fld_fx_draw(list, (void *)c->fx);
    return list;
}
#endif

extern int field_chr_sprite_cells[5][7][2];     /* 0x08834C14 sheet cell (u, v) by facing: idle, 6 walk frames */
extern signed char field_chr_sprite_dy[][5][7]; /* 0x08834D30 vertical offset by size, facing and frame */
int fld_chr_group(int id);
int fld_chr_size(int id);
void mtx_get34(Mat4 *m);
void mat4_scale(Mat4 *out, Mat4 *m, float s);
void mat4_transform_vec4(Vec4 *out, Mat4 *m, Vec4 *v);

#ifdef NON_MATCHING /* C novo, conferido com o asm (mesma sequência de chamadas; parte inteira igual). 121 palavras, todas no cálculo dos vértices: o original carrega todas as constantes float em registradores antes das contas; a tabela de saltos fica entre as de fld_chr_model_quads e fld_chr_group */
/* the quad of a sprite character (which 1 the body sheet, 0 the head sheet) at the screen position
   of its matrix: the cell by facing (five columns, mirrored for the left facings) and frame */
void fld_chr_sprite_quad(int which, FieldVtx *q, FieldChr *c)
{
    int id = c->id;
    int dir = c->dir;
    int frame = c->frame;
    int anim = c->anim;
    int col;
    int u;
    int v;
    int flip;
    int dy;
    int g;
    signed char *d;
    float t;
    Vec4 b;
    Vec4 a;
    Mat4 proj __attribute__((aligned(16)));
    Mat4 view __attribute__((aligned(16)));
    Mat4 model __attribute__((aligned(16)));
    Mat4 m __attribute__((aligned(16)));
    float u1;
    float u0;
    float v0;
    float v1;

    camera_get();
    switch (dir) {
    case 1:
        col = 0;
        flip = 0;
        break;
    case 0:
        col = 2;
        flip = 0;
        break;
    case 3:
        col = 1;
        flip = 0;
        break;
    case 2:
        col = 3;
        flip = 0;
        break;
    case 5:
        col = 1;
        flip = col;
        break;
    case 4:
        col = 4;
        flip = 0;
        break;
    case 7:
        col = 0;
        flip = 1;
        break;
    case 6:
        col = 3;
        flip = 1;
        break;
    }
    switch (which) {
    case 0:
        g = fld_chr_group(id);
        u = col * 51 + (g >= 5) * 255;
        v = g % 5 * 50;
        d = field_chr_sprite_dy[fld_chr_size(id)][col];
        switch (anim) {
        case 0:
            dy = d[0];
            break;
        case 1:
            dy = d[frame % 6 + 1];
            break;
        }
        break;
    case 1:
    case 2:
        switch (anim) {
        case 0:
            u = field_chr_sprite_cells[col][0][0];
            v = field_chr_sprite_cells[col][0][1];
            break;
        case 1:
            u = field_chr_sprite_cells[col][frame % 6 + 1][0];
            v = field_chr_sprite_cells[col][frame % 6 + 1][1];
            break;
        }
        break;
    }
    mtx_set_mode(0);
    mtx_get34(&proj);
    mtx_set_mode(1);
    mtx_get34(&view);
    mtx_set_mode(2);
    mtx_get34(&model);
    mat4_scale(&proj, &proj, 64.0f);
    mat4_identity(&m);
    mat4_mul(&m, &m, &view);
    mat4_mul(&m, &m, (Mat4 *)c->shadow);
    mat4_mul(&m, &m, &proj);
    a.w = 1.0f;
    a.x = 0.0f;
    a.y = 0.0f;
    a.z = 0.0f;
    mat4_transform_vec4(&a, &m, &a);
    b.y = 1.25f;
    b.w = 1.0f;
    b.x = 0.0f;
    b.z = 0.0f;
    mat4_transform_vec4(&b, &m, &b);
    switch (which) {
    case 0:
        t = dy;
        u1 = (u + 51) / 512.0f;
        q[0].u = u1;
        v0 = v / 256.0f;
        q[0].v = v0;
        u0 = u / 512.0f;
        q[0].x = a.x + 26.0f;
        v1 = (v + 50) / 256.0f;
        q[0].y = a.y + 80.0f - 12.0f + t;
        q[0].z = b.z + 5.0f + 1.0f;
        q[1].u = u0;
        q[1].v = v0;
        q[1].x = a.x - 25.0f;
        q[1].y = a.y + 80.0f - 12.0f + t;
        q[1].z = b.z + 5.0f + 1.0f;
        q[2].u = u1;
        q[2].v = v1;
        q[2].x = a.x + 26.0f;
        q[2].y = a.y + 80.0f - 50.0f - 12.0f + t;
        q[2].z = b.z + (a.z - b.z) * 0.625f + 5.0f + 1.0f;
        q[3].u = u0;
        q[3].v = v1;
        q[3].x = a.x - 25.0f;
        q[3].y = a.y + 80.0f - 50.0f - 12.0f + t;
        q[3].z = b.z + (a.z - b.z) * 0.625f + 5.0f + 1.0f;
        break;
    case 1:
        u1 = (u + 51) / 512.0f;
        q[0].u = u1;
        v0 = v / 256.0f;
        q[0].v = v0;
        u0 = u / 512.0f;
        q[0].x = a.x + 26.0f;
        v1 = (v + 64) / 256.0f;
        q[0].y = a.y + 64.0f - 12.0f;
        q[0].z = b.z + (a.z - b.z) * 0.2f + 5.0f;
        q[1].u = u0;
        q[1].v = v0;
        q[1].x = a.x - 25.0f;
        q[1].y = a.y + 64.0f - 12.0f;
        q[1].z = b.z + (a.z - b.z) * 0.2f + 5.0f;
        q[2].u = u1;
        q[2].v = v1;
        q[2].x = a.x + 26.0f;
        q[2].y = a.y - 12.0f;
        q[2].z = a.z + 5.0f;
        q[3].u = u0;
        q[3].v = v1;
        q[3].x = a.x - 25.0f;
        q[3].y = a.y - 12.0f;
        q[3].z = a.z + 5.0f;
        break;
    }
    q[3].color = 0xFFFFFFFF;
    q[2].color = 0xFFFFFFFF;
    q[1].color = 0xFFFFFFFF;
    q[0].color = 0xFFFFFFFF;
    if (flip) {
        t = q[0].u;
        q[0].u = q[1].u;
        q[1].u = t;
        t = q[2].u;
        q[2].u = q[3].u;
        q[3].u = t;
    }
}
#endif

void fld_ortho_begin(void)
{
    mtx_set_mode(0);
    mtx_push();
    mtx_identity();
    mtx_ortho(-240.0f, 240.0f, -136.0f, 136.0f, -10000.0f, 10000.0f);
    mtx_set_mode(1);
    mtx_push();
    mtx_identity();
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
}

void fld_ortho_end(void)
{
    mtx_set_mode(2);
    mtx_pop();
    mtx_set_mode(1);
    mtx_pop();
    mtx_set_mode(0);
    mtx_pop();
}

/* flat sprite (frame % 6 of tex[0]) at the character matrix, raised by 1.078 and moved by
   field_chr_flat_ofs turned with the camera rotation */
void *fld_chr_draw_flat(void *list, FieldChr *c)
{
    int f = c->frame % 6;
    FieldVtx *q = dl_alloc_inline(&list, sizeof(FieldVtx) * 4);
    Mat4 m __attribute__((aligned(16)));
    Mat4 t __attribute__((aligned(16)));
    Vec4 p;
    Mat4 rot __attribute__((aligned(16)));

    fld_sprite_quad(0.0f, q, 512, 512, field_chr_flat_cells[f][0], field_chr_flat_cells[f][1], 256, 170,
                    0.0f, 0xFFFFFFFF);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    {
        char *d = (char *)&p;
        int n = 16;

        if (d)
            do {
                *d++ = 0;
            } while (--n);
    }
    rot = *(Mat4 *)camera_get()->rot;
    p.y = 1.078125f;
    mat4_identity(&t);
    mat4_translate(&t, &t, &p);
    mat4_mul(&m, &t, (Mat4 *)c->shadow);
    {
        Vec4 tmp = field_chr_flat_ofs;

        rot.m[12] = 0.0f;
        rot.m[13] = 0.0f;
        rot.m[14] = 0.0f;
        p = *(Vec4 *)&tmp;
    }
    mat4_transform_vec4(&p, &rot, &p);
    mat4_translate(&m, &m, &p);
    mtx_load(&m);
    list = gim_set_texture(list, (void *)c->tex[0]);
    list = draw_array(list, 4, 0x19F, 4, 0, q);
    mtx_pop();
    return list;
}

#ifdef NON_MATCHING /* conversões e divisões em outra ordem (escalonamento de ponto flutuante) */
/* quad of the sprite cell (u, v, w, h) of a tw x th sheet centered on x and standing on y */
void fld_sprite_quad(float x, FieldVtx *q, int tw, int th, int u, int v, int w, int h, float y,
                     unsigned int color)
{
    float sw = w / 64.0f;
    float sh = h / 64.0f;
    float u1 = (float)(u + w) / tw;
    float v0 = (float)v / th;
    float u0 = (float)u / tw;
    float v1 = (float)(v + h) / th;
    float y0 = y + 0.5f * sh + 0.23f * sh;
    float y1 = y + -0.5f * sh;
    float x0 = x + 0.5f * sw;
    float x1 = x + -0.5f * sw;

    q[0].u = u1;
    q[0].v = v0;
    q[0].x = x0;
    q[0].y = y0;
    q[1].u = u0;
    q[1].v = v0;
    q[1].x = x1;
    q[1].y = y0;
    q[2].u = u1;
    q[2].v = v1;
    q[2].x = x0;
    q[2].y = y1;
    q[3].u = u0;
    q[3].v = v1;
    q[3].x = x1;
    q[3].y = y1;
    q[3].color = color;
    q[2].color = color;
    q[1].color = color;
    q[0].color = color;
    q[3].z = 0.0f;
    q[2].z = 0.0f;
    q[1].z = 0.0f;
    q[0].z = 0.0f;
}
#endif

/* 0 model, 1 sprite sheet, 2 flat sprite */
int fld_chr_kind(int id)
{
    int r = 0;

    switch (id) {
    case 1 ... 0x33:
        r = 0;
        break;
    case 0x3A:
        r = 2;
        break;
    case 100 ... 449:
        r = 1;
        break;
    case 1000 ... 1012:
        break;
    }
    return r;
}

#ifdef NON_MATCHING /* 6 palavras: o original testa 3, 2, 1 (o r = 3 do case 2 reaproveita o li v0,3 do teste do case 3); aqui o teste de 1 vem antes do de 2. Melhor das 24 ordens de case e das variantes com default: case 1: e return */
int fld_chr_tex(int id)
{
    CharmapPos p;
    int r;

    if (charmap_lookup2(id, &p) != 0) {
        switch (p.b) {
        case 2:
            r = 3;
            break;
        default:
        case 1:
            r = 2;
            break;
        case 3:
            r = 4;
            break;
        }
    } else {
        r = 2;
    }
    return r;
}
#endif

int fld_chr_size(int id)
{
    CharmapPos p;
    int r;

    if (charmap_lookup2(id, &p) != 0) {
        switch (p.b) {
        case 1:
            r = 0;
            break;
        case 2:
            r = 1;
            break;
        case 3:
            r = 2;
            break;
        default:
            r = 0;
            break;
        }
    } else {
        r = 0;
    }
    return r;
}

int fld_chr_group(int id)
{
    CharmapPos p;
    int r;

    if (charmap_lookup2(id, &p) != 0) {
        switch (p.a) {
        default:
        case 0:
            r = 0;
            break;
        case 1:
            r = 1;
            break;
        case 2:
            r = 2;
            break;
        case 3:
            r = 3;
            break;
        case 4:
            r = 4;
            break;
        case 5:
            r = 5;
            break;
        case 6:
            r = 6;
            break;
        case 7:
            r = 7;
            break;
        case 8:
            r = 8;
            break;
        case 9:
            r = 9;
            break;
        }
    } else {
        r = 0;
    }
    return r;
}
