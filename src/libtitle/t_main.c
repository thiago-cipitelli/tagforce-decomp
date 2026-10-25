// @range 0x08804000-0x08804348
// @rodata 0x0882A940-0x0882A97C
/* Scene 13 (title): module entry, the scene thread (picks the first state from how the scene was
   entered, then runs title_update once per frame until it returns 0x28), its exit and the init. */
#include "libtitle.h"

void title_main(void);
void title_exit(void);
void title_draw_cb(void);
void title_exit_cb(void);

int module_entry(void)
{
    ehscript_register((int (*)(void))title_main, (int (*)(void))title_exit, title_scene_data);
    return 0;
}

void title_main(void)
{
    Title *const t = &title;
    TGameHdr *gs;

    title_init();
    scene_get_transition_info(&title.trans);
    switch (title.trans.type) {
    case 0:
        t->state = 0;
        break;
    case 1:
        t->state = 0;
        break;
    case 2:
        switch (t->trans.from) {
        case 2:
            if (t->trans.arg != 0)
                title_goto(3, 0);
            else
                title_goto(5, 0);
            break;
        case 4:
            if (t->trans.arg == 2)
                title_goto(0x1A, 0);
            else
                title_goto(9, 5);
            break;
        case 6:
            gs = (TGameHdr *)game_state_get();
            switch (gs->kind) {
            case 2:
                title_goto(0x18, gs->tutorial + 7);
                break;
            case 5:
                title_goto(5, 0);
                break;
            default:
                title_goto(9, 0);
                break;
            }
            break;
        case 14:
            t->arg = 5;
            title_goto(9, 6);
            break;
        case 0:
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        default:
            title_goto(5, 0);
            break;
        }
        break;
    case 3:
        if (t->trans.from == 6 && t->trans.arg != 0)
            title_goto(5, 0);
        break;
    }
    sys_state_set_callbacks(13, (int (*)(void))title_draw_cb, (int (*)(void))title_exit_cb);
    for (;;) {
        if (title_update() == 0x28)
            break;
        sys_frame_wait();
    }
}

void title_exit(void)
{
}

void title_init(void)
{
    rand_seed_from_rtc();
    sysvar_set_a90(0);
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    memset(&title, 0, sizeof(title));
    title.heap = heap_create(title_heap_mem, sizeof(title_heap_mem));
    gfx_overlay_enable(0);
    tmenu_init(&title.tm[0]);
    font_set_flag2(1);
    func_0880C0F0(title.heap, 0x27, 0x140, 0xE, 0, 0);
    twin_init(&title.win[0]);
    twin_init(&title.win[1]);
    twin_init(&title.win[2]);
    tmenu_init(&title.tm[0]);
    tmenu_init(&title.tm[1]);
    func_088108A0(&title.u508, title.heap);
}

void title_draw_cb(void)
{
    title_draw();
}

void title_exit_cb(void)
{
    menu_free_items(&title.menu);
    func_0880C2F0();
    func_088225A0(0);
    title_module_stop();
}
