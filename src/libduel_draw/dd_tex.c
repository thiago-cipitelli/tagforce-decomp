// @range 0x08826A64-0x08826E30
// @data 0x08876278-0x088764F0
/* Textures of the duel effects: a GIM of an effect pack by index, the frame buffer used as a
   texture (whole, a rectangle, or a copy of the screen in VRAM) and a card rendered into VRAM. */
#include "dd_d3.h"

void *func_08804A88(int id);           /* effect pack of a card id */
void *func_088045C0(int id);
unsigned int *func_088046F0(unsigned int *list, int id);
unsigned char *syspack_get(void);
void *ehp_find(void *pack, const char *name, int *size);
void *gim_set_texture(void *list, void *gim);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
unsigned int *dl_tex_flush(unsigned int *list);
unsigned int *dl_tex_sync(unsigned int *list);
unsigned int *dl_copy_image(unsigned int *list, int psm, int sx, int sy, int w, int h, int srcw, void *src, int dx, int dy, int dstw, void *dst);
unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
unsigned int *card_draw(void *list, int x, int y, unsigned int id);
unsigned int *gfx_set_default_target(unsigned int *list);

char *dd_tex_names[34] = {
    "window01.gim", "st4k_emb02P4O.gim", "eff_nekoptcl01P8O.gim", "lens_aP8A.gim", "PT88_R.gim",
    "PT88_R_a.gim", "sen_0a.gim", "sen_1a.gim", "sen_2a.gim", "nsea.gim", "m530snd.gim",
    "rai_BL.gim", "sen_07.gim", "blu_light.gim", "blu_light_g.gim", "orang_light.gim", "PT34.gim",
    "PT55.gim", "light_064.gim", "kemuri.gim", "PT42.gim", "thblue2.gim", "spel_mark01.gim",
    "thred.gim", "duel_thing01.gim", "082rock.gim", "fusion_mark_tmp.gim", "k02_256b.gim",
    "mahojin.gim", "k02_256y.gim", "duel_ouija.gim", "m169_onp01.gim", "PT35.gim", "handshake01.gim",
};

unsigned int *dd_tex_pack(unsigned int *list, int i)
{
    void *pack;

    switch (i) {
    case 0:
        pack = syspack_get();
        break;
    case 25:
        pack = func_08804A88(4934);
        break;
    case 27:
        pack = func_08804A88(4937);
        break;
    case 29:
        pack = func_08804A88(5791);
        break;
    case 31:
        pack = func_08804A88(4899);
        break;
    case 32:
        pack = func_08804A88(4933);
        break;
    default:
        pack = func_088045C0(0x80001);
        break;
    }
    return gim_set_texture(list, ehp_find(pack, dd_tex_names[i], 0));
}

unsigned int *dd_tex_fb(unsigned int *list)
{
    void *fb = dd_fb_addr(0);

    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 512, 512, 512, fb);
    return dl_tex_flush(list);
}

unsigned int *dd_tex_fb_rect(unsigned int *list, int x, int y, int w, int h)
{
    unsigned char *fb = dd_fb_addr(0);

    fb += (x + y * 512) * 4;
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, w, h, 512, fb);
    return dl_tex_flush(list);
}

unsigned int *dd_tex_fb_copy(unsigned int *list)
{
    void *src = dd_fb_addr(1);
    void *dst = dd_vram_addr(0x174000);

    list = dl_tex_sync(list);
    list = dl_tex_flush(list);
    list = dl_copy_image(list, 3, 0, 0, 480, 272, 512, src, 0, 0, 512, dst);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 512, 512, 512, dst);
    list = dl_tex_sync(list);
    return dl_tex_flush(list);
}

unsigned int *dd_tex_card(unsigned int *list, int id, int mode)
{
    void *tex;

    if (mode == 1)
        return func_088046F0(list, id);
    tex = dd_vram_addr(0x160000);
    list = dl_tex_flush(list);
    list = gfx_set_draw_target(list, 0x160000, 0xFFFFFFFF, 128, 256, 3);
    list = card_draw(list, 0, 0, id);
    list = gfx_set_default_target(list);
    list = dl_tex_flush(list);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    return dl_tex_image(list, 0, 128, 256, 128, tex);
}
