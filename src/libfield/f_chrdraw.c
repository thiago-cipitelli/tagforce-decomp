// @range 0x08805DF4-0x08806FC4
// @asm 0x08805DF4-0x08805FD0
// @asm 0x08805FD0-0x08806200
// @asm 0x08806200-0x08806390
// @asm 0x08806390-0x08806A14
// @asm 0x08806AF0-0x08806CE0
// @asm 0x08806CE0-0x08806DF0
// @asm 0x08806DF0-0x08806E6C
// @asm 0x08806E6C-0x08806ED4
// @asm 0x08806F24-0x08806FC4
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

#ifdef NON_MATCHING /* o original mantém blocos mortos "li; b; nop" depois da cadeia de testes */
/* 0 model, 1 sprite sheet, 2 flat sprite */
int fld_chr_kind(int id)
{
    int r = 0;

    if (id < 1000 || id >= 1013) {
        if (id < 100 || id >= 450) {
            if (id == 0x3A) {
                r = 2;
            } else if (id > 0 && id < 0x34) {
                r = 0;
            }
        } else {
            r = 1;
        }
    }
    return r;
}
#endif

#ifdef NON_MATCHING /* o original mantém blocos mortos "li; b; nop" depois dos testes do switch */
int fld_chr_tex(int id)
{
    CharmapPos p;
    int r;

    if (charmap_lookup2(id, &p) != 0) {
        switch (p.b) {
        case 1:
            r = 2;
            break;
        case 2:
            r = 3;
            break;
        case 3:
            r = 4;
            break;
        default:
            r = 2;
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

#ifdef NON_MATCHING /* casos do switch sem preencher o delay slot no original (blocos mortos) */
int fld_chr_group(int id)
{
    CharmapPos p;
    int r;

    if (charmap_lookup2(id, &p) != 0) {
        switch (p.a) {
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
        default:
            r = 0;
            break;
        }
    } else {
        r = 0;
    }
    return r;
}
#endif
