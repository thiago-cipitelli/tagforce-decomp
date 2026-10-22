// @range 0x08804000-0x088042F4
// @data 0x0880A0F0-0x0880A0F4
/* Entry points of the tutorial list sub-screen (init / per-frame update / result): working heap,
   preselected lesson, the load -> fade in -> input -> fade out -> done state machine. */
#include "libtutoriallist.h"

int module_entry(void)
{
    return 0;
}

/* 0x08804008: export; selected < 0 = none, heap 0 = allocate an own 2 MB block */
void tlist_init(int selected, void *heap)
{
    memset(&tl, 0, 0xB8);
    if (heap == 0) {
        tl.fpl = sceKernelCreateFpl("", 2, 0, 0x200000, 1, 0);
        sceKernelAllocateFpl(tl.fpl, &tl.fpl_mem, 0);
        tl.heap = heap_create(tl.fpl_mem, 0x200000);
    } else {
        tl.heap = heap;
    }
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    tlres_init();
    if (selected >= 0)
        tl.selected = selected;
    else
        tl.selected = 8;
    if (tl.selected >= 8) {
        snd_cmd_fefb();
        bgm_fade(330);
    }
    font_set_flag4(1);
    font_set_flag2(1);
    font_set_line_extra(0);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
}

/* 0x08804154: export; 0 while running, 1 when a lesson was chosen, 2 on cancel */
int tlist_update(void)
{
    switch (tl.state) {
    case 0:
        if (tlres_poll()) {
            tlist_build();
            tl.state = 1;
            fade_start(1, 0xFF000000, 16);
            gfx_overlay_enable(0);
            bgm_play(6);
        }
        break;
    case 1:
        if (fade_is_done())
            tl.state = 2;
        break;
    case 2:
        tlist_input();
        break;
    case 3:
        if (fade_is_done())
            tl.state = 4;
        break;
    case 4:
        if (tlist_release()) {
            if (tl.cancelled)
                return 2;
            return 1;
        }
        break;
    }
    return 0;
}

int tlist_release(void)
{
    tlres_free();
    if (tl.fpl > 0) {
        sceKernelFreeFpl(tl.fpl, tl.fpl_mem);
        sceKernelDeleteFpl(tl.fpl);
    }
    return 1;
}

/* 0x088042c8: export; lesson chosen, -1 on cancel */
int tlist_result(void)
{
    if (tl.cancelled)
        return -1;
    return tl.selected;
}
