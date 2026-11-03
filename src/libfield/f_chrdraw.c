// @range 0x08805DF4-0x08806FC4
// @rodata 0x08834E14-0x08834E3C
// @asm 0x08805DF4-0x08805FD0
// @asm 0x08805FD0-0x08806200
// @asm 0x08806200-0x08806390
// @asm 0x08806390-0x08806A14
// @asm 0x08806AF0-0x08806CE0
// @asm 0x08806CE0-0x08806DF0
// @asm 0x08806E6C-0x08806ED4
/* Drawing of the SD characters: the 3D model, the sprite sheets (sd_*_sla.gim) as billboards under
   an orthographic projection, the flat sprite; the vertex quad of a sprite cell and the kind of
   drawing each chara id uses. The large draw callbacks are still in asm. */
#include "libfield.h"

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

#ifdef NON_MATCHING /* conversões e divisões em outra ordem (escalonamento de ponto flutuante) */
/* quad of the sprite cell (u, v, w, h) of a tw x th sheet centered on x and standing on y */
void fld_sprite_quad(float x, float y, FieldVtx *q, int tw, int th, int u, int v, int w, int h,
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
