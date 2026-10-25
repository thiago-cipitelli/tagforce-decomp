// @range 0x0880C998-0x0880CE48
// @asm 0x0880CA5C-0x0880CB58
/* Options screen of the main menu (state 17): enter/update/draw, the edit state (tcfg) and the copy
   of the system options in and out of it. */
#include "libtitle.h"

int tcfg_update(void);
void *tcfg_draw(void *list);
void tcfg_create(void *heap, void *pack, int mode);
void tcfg_read(TCfgVals *v);
void tcfg_free(void);

void tcfg_enter(short state, short sub)
{
    title_goto_fade(0, 0xFFF, 0xFFE);
    title.state = state;
    title.sub = sub;
    title.step = 0;
    title.frame = 0;
    title.color0 = 0xFFFFFFFF;
    title.color1 = 0xFFFFFFFF;
    tcfg_create(title.heap, title_pack(), 0);
    title_ehf_load(5, 2, 0, 480);
    title_ehf_load(6, 2, 0, 480);
    title_ehf_load(13, 2, 0, 16);
}

#ifdef NON_MATCHING /* o caso 2 desvia para um bloco `return` próprio (salto para salto) que o mwcc encurta */
void tcfg_screen_update(void)
{
    Title *const t = &title;

    switch (title.sub) {
    case 0:
        t->sub = 1;
        break;
    case 1:
        break;
    case 2:
        if (title_ehf_frame(13) == 16) {
            title_ehf_anim(13, 2, 17, 96);
            t->sub++;
            break;
        }
        return;
    case 3:
        switch (tcfg_update()) {
        case 0:
        case 1:
            break;
        case 2:
            title_goto_fade(1, 0xFFF, 5);
            t->sub = 4;
            break;
        }
        break;
    case 4:
        break;
    case 5:
        tcfg_free();
        title_set_state(9, 8);
        break;
    }
}
#endif

void *tcfg_screen_draw(void *list)
{
    Title *const t = &title;

    list = title_ehf_draw(list, 6);
    list = title_ehf_draw(list, 5);
    if (title.sub >= 2) {
        list = title_ehf_draw(list, 13);
        list = title_screen_label(list);
    }
    list = tcfg_draw(list);
    t->frame++;
    return list;
}

void tcfg_create(void *heap, void *pack, int mode)
{
    TCfg *c = tcfg = heap_alloc(heap, sizeof(TCfg));

    memset(c, 0, sizeof(TCfg));
    c->heap = heap;
    c->pack = pack;
    c->mode = mode;
    tcfg_read(&c->cur);
    tcfg_read(&c->orig);
    if (mode == 0) {
        c->step = 0;
        c->color = 0xFFFFFFFF;
        c->color2 = 0xFF000000;
    } else {
        c->step = 3;
        c->color = 0xFF000000;
        c->color2 = 0xFF000000;
    }
}

void tcfg_read(TCfgVals *v)
{
    v->mode1 = cfg_is_mode1();
    v->speed = cfg_get_byte0_bits4_5();
    v->vol[0] = cfg_get_byte3(2);
    v->vol[1] = cfg_get_byte3(0);
    v->vol[2] = cfg_get_byte3(1);
    v->b134 = cfg_get_bits_1_3_4();
    v->flag2 = cfg_get_flag(2);
    v->flag0 = cfg_get_flag(0);
    v->flag3 = cfg_get_flag(3);
}

void tcfg_write(TCfgVals *v)
{
    cfg_set_byte0_bit3(v->mode1);
    cfg_set_byte0_bits4_5(v->speed);
    snd_set_volume(2, v->vol[0]);
    snd_set_volume(0, v->vol[1]);
    snd_set_volume(1, v->vol[2]);
    cfg_set_bits_1_3_4(v->b134);
    cfg_set_flag(2, v->flag2);
    cfg_set_flag(0, v->flag0);
    cfg_set_flag(3, v->flag3);
}

void tcfg_free(void)
{
    heap_set_level(tcfg->heap, tcfg);
}
