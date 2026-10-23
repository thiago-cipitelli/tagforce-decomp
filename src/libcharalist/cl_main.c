// @range 0x08804810-0x08804F48
// @rodata 0x0880DEA0-0x0880DEBC
// @data 0x0880E018-0x0880E01C
/* Entry points of the duelist list (init / per-frame update / results, and the random pick used by
   the story): working heap, the load -> fade in -> list -> detail -> fade out -> release machine. */
#include "libcharalist.h"

void rand_seed_from_rtc(void);
int rand_next_alias(void);
void snd_cmd_fefb(void);
void bgm_fade(int x);
void bgm_play(int id);
unsigned short cfg_get_byte3(int i);

int module_start(void)
{
    rand_seed_from_rtc();
    return 0;
}

/* 0x08804830: export; mode 0/1 title/story, 3/4 labo (two flavors); heap 0 = own 8 MB block */
void charalist_init(unsigned short mode, void *heap)
{
    memset(&cl, 0, 0x180);
    switch (mode) {
    case 3:
        cl.mode = 3;
        cl.sub = 2;
        break;
    case 4:
        cl.mode = 3;
        cl.sub = 3;
        break;
    default:
        cl.mode = mode;
        break;
    }
    cl.chosen = -1;
    cl.costume = -1;
    if (heap == 0) {
        cl.fpl = sceKernelCreateFpl("", 2, 0, 0x800000, 1, 0);
        sceKernelAllocateFpl(cl.fpl, &cl.fpl_mem, 0);
        cl.heap = heap_create(cl.fpl_mem, 0x800000);
    } else {
        cl.heap = heap;
    }
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    cl_res_init();
    switch (cl.mode) {          /* title and story: the list has its own music */
    case 0:
    case 1:
        snd_cmd_fefb();
        bgm_fade(330);
        cl.bgm = -1;
        break;
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

/* 0x08804A4C: export; 1 when left, 2 when left with cancel (labo/random modes) */
int charalist_update(void)
{
    switch (cl.state) {
    case 0:
        if (cl_load_poll()) {
            cl_list_build();
            cl.state = 1;
            fade_start(1, 0xFF000000, 16);
            gfx_overlay_enable(0);
            switch (cl.mode) {
            case 0:
            case 1:
                bgm_play(6);
                break;
            }
            cl.volume = cfg_get_byte3(0);
        }
        break;
    case 1:
        if (fade_is_done())
            cl.state = 2;
        break;
    case 2:
        cl_input();
        break;
    case 3:
        cl_detail_update();
        break;
    case 4:
        if (cl.timer++ > 16) {
            fade_start_in(1, 0xFF000000, 16);
            cl.timer = 0;
            cl.state = 5;
        }
        break;
    case 5:
        if (fade_is_done())
            cl.state = 6;
        break;
    case 6:
        if (cl_term())
            return cl.cancelled ? 2 : 1;
        break;
    }
    return 0;
}

/* frees the lists and the own block once the reads are over */
int cl_term(void)
{
    int t;
    int i;
    ClEntry *e;

    if (cl.mode != 2 && cl_res_release() == 0)
        return 0;
    for (t = 0; t < 3; t++) {
        for (i = 0; i < cl.count[t]; i++) {
            e = cl_entry(t, i);
            if (!e->simple)
                heap_set_level(cl.heap, e->u.info);
        }
        if (cl.list[t])
            heap_set_level(cl.heap, cl.list[t]);
    }
    if (cl.fpl > 0) {
        sceKernelFreeFpl(cl.fpl, cl.fpl_mem);
        sceKernelDeleteFpl(cl.fpl);
    }
    return 1;
}

/* 0x08804D20: export; picks a random duelist (and costume) among the met ones, without a screen */
void charalist_random(void *heap)
{
    int n;
    int t;
    int r;
    ClEntry *e;
    unsigned char v;

    memset(&cl, 0, 0x180);
    cl.heap = heap;
    cl.mode = 2;
    cl.chosen = -1;
    cl.costume = -1;
    cl_list_build();
    n = 0;
    for (t = 0; t < 3; t++) {
        n += cl.count[t];
        if (t == 1 && n > 0)
            break;
    }
    if (n <= 0) {
        cl.chosen = 0;
        cl.costume = 0;
    } else {
        r = rand_next_alias() % n;
        for (t = 0; t < 3; t++) {
            if (r < cl.count[t])
                break;
            r -= cl.count[t];
        }
        e = cl_entry(t, r);
        cl.chosen = e->id;
        cl.costume = 0;
        if (!e->simple) {
            if (e->u.info->n > 1) {
                cl.costume = rand_next_alias() % e->u.info->n;
            } else {
                v = e->u.info->sel;
                if (v)
                    cl.costume = v;
            }
        }
        cl_term();
    }
}

/* 0x08804EF8: export; duelist chosen, -1 on cancel */
int charalist_result_id(void)
{
    if (cl.cancelled)
        return -1;
    return cl.chosen;
}

/* 0x08804F20: export */
int charalist_result_costume(void)
{
    if (cl.cancelled)
        return -1;
    return cl.costume;
}
