// @range 0x0884DF2C-0x0884E0F4
// @data 0x08876D14-0x08876D28
/* Speed lines behind the duel character (dd_ryusen): ryusen01.ehf of the character's pack drawn
   in 2D before the model, with an alpha that fades over 10 frames. */
#include "libduel_draw.h"

void *memset(void *, int, unsigned int);
void *ehp_find(void *pack, const char *name, int *size);
unsigned int *gfx_get_state(void);
void *gfx_begin_2d(void *list);
void *ehf_draw(void *list, void *ehf, DdEhfOpts *o);
void *gfx_end_2d(void *list);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, unsigned int flags);
void *func_0880507C(unsigned int chr, int variant);

extern DdRyusen dd_ryusen;

char *dd_ryusen_name = "ryusen01.ehf";

void *dd_ryusen_draw(void *list)
{
    DdRyusen *r = &dd_ryusen;
    void *ehf = 0;
    void *pack;
    DdEhfOpts o;

    if (r->chr != 0 && (pack = func_0880507C(r->chr, r->variant)) != 0)
        ehf = ehp_find(pack, dd_ryusen_name, 0);
    if (ehf == 0)
        return list;
    memset(&o, 0, sizeof(o));
    o.time = *gfx_get_state();
    o.flags |= 4;
    o.color[3] = r->alpha;
    o.color[0] = 0x100;
    o.color[1] = 0x100;
    o.color[2] = 0x100;
    o.color[7] = 0;
    o.color[4] = 0;
    o.color[5] = 0;
    o.color[6] = 0;
    list = gfx_begin_2d(list);
    list = ehf_draw(list, ehf, &o);
    list = gfx_end_2d(list);
    if (++r->t >= 10)
        r->alpha = r->alpha_to;
    else
        r->alpha = (r->alpha_from * (10 - r->t) + r->alpha_to * r->t) / 10;
    list = dl_clear_stencil(list, 0);
    return dl_clear(list, 0x14);
}

void dd_ryusen_set_layer(int layer)
{
    dd_ryusen.layer = layer;
}

void dd_ryusen_set_chara(unsigned int chr, int variant)
{
    dd_ryusen.chr = chr;
    dd_ryusen.variant = variant;
}

/* Fades to `alpha` (now = 1 sets it at once). */
void dd_ryusen_set_alpha(int alpha, int now)
{
    DdRyusen *r = &dd_ryusen;

    if (now != 0) {
        r->alpha_from = alpha;
        r->alpha_to = alpha;
        r->alpha = alpha;
    } else {
        r->alpha_to = alpha;
        r->t = 0;
        r->alpha_from = r->alpha;
    }
}
