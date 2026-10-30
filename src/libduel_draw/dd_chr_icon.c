// @range 0x08867EE0-0x088682AC
// @asm 0x08867EE0-0x088682AC
/* Portrait of a duelist at scale percent: the fixed portraits 1000..1012 (one sprite of the
   portrait sheet), or for charmap characters a body sprite plus the face (palette from the
   charmap entry). */
#include "dd_duelui.h"

void *gim_set_texture_sub(void *list, void *gim, int a, int b, int c, int frame);

#ifdef NON_MATCHING /* usa $fp; alocação de registradores não reproduzida */
void *chr_icon_draw(void *dl, int chr, int x, int y, int scale, void *face_gim, void *gim)
{
    DdCharPos pos;
    int fixed = 0;
    int u = 0, v = 0;
    int id, w;

    if (face_gim == 0 || gim == 0)
        return dl;
    id = charmap_lookup((unsigned short)chr, 0);
    switch (id) {
    case 1012: u = 51; v = 168; fixed = 1; break;
    case 1011: v = 168; fixed = 1; break;
    case 1010: u = 204; v = 84; fixed = 1; break;
    case 1009: u = 153; v = 84; fixed = 1; break;
    case 1008: u = 102; v = 84; fixed = 1; break;
    case 1007: u = 51; v = 84; fixed = 1; break;
    case 1000: v = 84; fixed = 1; break;
    default:
        if (charmap_lookup2(id, &pos) == 0)
            return dl;
        break;
    }
    dl = dl_ambient_color(dl, 0xFFFFFFFF);
    dl = dl_set_tex_filter(dl, 1, 1);
    if (fixed) {
        dl = gim_set_texture(dl, gim);
        dl = draw_set_offsets(dl, 51, 84, scale * 51 / 100, scale * 84 / 100);
        return draw_sprite(dl, x, y - scale * 6 / 100, 0, u, v, 0, 0);
    }
    dl = gim_set_texture(dl, gim);
    w = scale * 51 / 100;
    dl = draw_set_offsets(dl, 51, 64, w, (scale << 6) / 100);
    dl = draw_sprite(dl, x, y + (scale << 4) / 100, 0, pos.sub * 51, 0, 0, 0);
    dl = gim_set_texture_sub(dl, face_gim, 0, 0, 0, pos.c & 0xF);
    dl = draw_set_offsets(dl, 51, 50, w, scale * 50 / 100);
    return draw_sprite(dl, x, y, 0, pos.kind % 5 * 51, pos.kind / 5 * 50, 0, 0);
}
#else
void *chr_icon_draw(void *dl, int chr, int x, int y, int scale, void *face_gim, void *gim);
#endif
