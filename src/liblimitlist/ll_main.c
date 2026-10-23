// @range 0x088060C4-0x08806490
// @rodata 0x0880D09C-0x0880D0BC
// @data 0x0880D5E8-0x0880D5EC
/* Entry points of the limit list sub-screen (init / per-frame update): working heap, the
   load -> fade in -> list -> menu / card info -> fade out -> release state machine. */
#include "liblimitlist.h"

int module_start(void)
{
    return 0;
}

/* 0x088060CC: export; heap 0 = allocate an own 5 MB block */
void limitlist_init(int mode, void *heap)
{
    memset(&ll, 0, sizeof(ll));
    ll.mode = mode;
    if (heap == 0) {
        ll.fpl = sceKernelCreateFpl("", 2, 0, 0x500000, 1, 0);
        sceKernelAllocateFpl(ll.fpl, &ll.fpl_mem, 0);
        ll.heap = heap_create(ll.fpl_mem, 0x500000);
    } else {
        ll.heap = heap;
    }
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    llres_init();
    if (!(bgm_get_status(6) & 1)) {
        snd_cmd_fefb();
        bgm_fade(330);
    }
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
}

/* 0x08806214: export; 0 while running, 1 when done, 2 when left with cancel */
int limitlist_update(void)
{
    switch (ll.state) {
    case 0:
        if (llres_poll()) {
            ll_build();
            ll.state = 1;
            fade_start(1, 0xFF000000, 16);
            gfx_overlay_enable(0);
            bgm_play(6);
        }
        break;
    case 1:
        if (fade_is_done())
            ll.state = 2;
        break;
    case 2:
        ll_input();
        break;
    case 4:
        ll_info_update();
        break;
    case 3:
    case 5:
        ll_menu_update();
        break;
    case 6:
        if (fade_is_done())
            ll.state = 7;
        break;
    case 7:
        if (limitlist_release())
            return ll.cancelled ? 2 : 1;
        break;
    }
    return 0;
}

/* frees everything over a few frames; 1 when done */
int limitlist_release(void)
{
    switch (ll.release) {
    case 0:
        ll_free_lists();
        llres_free();
        cardlist_clear();
        if (ll.cip)
            cip_close(ll.cip);
        ll.release++;
        break;
    case 1:
        if (cip_is_idle(ll.cip))
            ll.release++;
        break;
    case 2:
        if (ll.fpl > 0) {
            sceKernelFreeFpl(ll.fpl, ll.fpl_mem);
            sceKernelDeleteFpl(ll.fpl);
        }
        ll.release = 0;
        return 1;
    }
    return 0;
}
