// @range 0x08804000-0x08804408
// @rodata 0x0880D670-0x0880D694
// @data 0x0880D6AC-0x0880D6B0
/* Entry points of the card album sub-screen (init / per-frame update): working heap, the
   load -> fade in -> cover opening -> browse/turn/index/detail -> exit state machine, and the release. */
#include "libcardalbum.h"

int module_entry(void)
{
    return 0;
}

/* 0x08804008: export; heap 0 = allocate an own 8 MB block */
void cardalbum_init(int mode, void *heap)
{
    memset(&ca, 0, 0x13B0);
    ca.mode = mode;
    if (heap == 0) {
        ca.fpl = sceKernelCreateFpl("", 2, 0, 0x800000, 1, 0);
        sceKernelAllocateFpl(ca.fpl, &ca.fpl_mem, 0);
        ca.heap = heap_create(ca.fpl_mem, 0x800000);
    } else {
        ca.heap = heap;
    }
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    ca_gfx_init();
    ca_res_init();
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

/* 0x08804130: export; 1 once the screen has been left and released */
int cardalbum_update(void)
{
    switch (ca.state) {
    case 0:
        if (ca_load_poll()) {
            ca_build();
            ca.state = 1;
            ca.timer = 0;
            fade_start(1, 0xFF000000, 16);
            gfx_overlay_enable(0);
        }
        return 0;
    case 1:
        if (fade_is_done() && ca.timer++ > 32) {
            ca_book_open();
            ca_skipmenu_anim(6);
            ca.state = 2;
            ca.timer = 0;
        }
        break;
    case 2:
        if (ca.book == 2) {
            ca_skipmenu_anim(7);
            ca.cur.pos = 3;
            ca.state = 3;
        }
        break;
    case 3:
        ca_input();
        break;
    case 4:
        ca_turn_update();
        break;
    case 6:
        ca_detail_update();
        break;
    case 5:
        ca_index_update();
        break;
    case 8:
        if (ca_term())
            return 1;
        break;
    }
    ca_prefetch_update();
    return 0;
}

/* closes the card readers, waits for them, frees everything */
int ca_term(void)
{
    int busy;

    switch (ca.step) {
    case 0:
        busy = 0;
        cardlist_clear();
        if (ca.cip_h) {
            cip_close(ca.cip_h);
            busy = 1;
        }
        if (ca.cip_m) {
            cip_close(ca.cip_m);
            busy = 1;
        }
        if (busy)
            ca.step++;
        else
            ca.step = 2;
        break;
    case 1:
        if (cip_is_idle(ca.cip_h) && cip_is_idle(ca.cip_m))
            ca.step++;
        break;
    case 2:
        ca_pages_free();
        ca_res_free();
        if (ca.fpl > 0) {
            sceKernelFreeFpl(ca.fpl, ca.fpl_mem);
            sceKernelDeleteFpl(ca.fpl);
        }
        return 1;
    }
    return 0;
}
