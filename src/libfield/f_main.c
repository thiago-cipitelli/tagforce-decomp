// @range 0x08804000-0x088044D8
// @rodata 0x08832A48-0x08832AB8
/* Scene 5 (field): module entry, the scene thread (a room picked from a menu, then the startup
   sequence of field commands run once per frame until SELECT), its exit and the init. */
#include "libfield.h"

void field_main(void);
void field_exit(void);
int field_init(void);
int field_seq_update(void);
int field_bg_of_room(int room);
int field_draw_cb(void);
void field_exit_cb(void);
void fld_sys_init(void);
void *fld_heap_get(void);
void fld_sys_update(void);
int fld_sys_draw(void);
int fld_sys_term(void);
int fld_cmd_start(int kind, void *arg);
int fld_cmd_poll(int kind, void *arg);

int module_entry(int arg)
{
    if (arg == 0)
        ehscript_register((int (*)(void))field_main, (int (*)(void))field_exit, field_scene_data);
    return 0;
}

void field_main(void)
{
    int done = 0;

    gfx_overlay_enable(0);
    field_room = menu_wait_select(field_room_menu, -1, 0);
    if (field_room == -1)
        field_room = 0;
    field_bg = field_bg_of_room(field_room);
    if (field_init()) {
        sys_state_set_callbacks(2, field_draw_cb, (int (*)(void))field_exit_cb);
        do {
            done |= field_seq_update();
            sys_frame_wait();
            if (!(pad_get()->b.dir_release & 1))
                continue;
            sys_state_set_default_callbacks();
            sys_frame_wait();
            scene_pop_to(5, 0);
            break;
        } while (!done);
    }
}

void field_exit(void)
{
    menu_free_items(&field_seq.menu);
    for (;;) {
        if (fld_sys_term())
            break;
        sys_frame_wait();
    }
}

int field_init(void)
{
    FieldSeq *s = &field_seq;
    int i;

    memset(s, 0, sizeof(FieldSeq));
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    money_add(99999);
    fld_sys_init();
    menu_init(&field_seq.menu, fld_heap_get());
    for (i = 0; i < 12; i++)
        font_mbs_to_wcs(s->text[i], 0x400, field_seq_src[i]);
    return 1;
}

int field_seq_update(void)
{
    FieldSeq *s = &field_seq;
    FieldSeqStep *e;
    int kind;

    fld_sys_update();
    if (s->state == 0) {
        s->state = 1;
        s->active = 1;
    }
    if (s->active) {
        e = &field_seq_steps[s->index];
        if (!s->started) {
            fld_cmd_start(e->kind, e->arg);
            s->started = 1;
        } else {
            switch (kind = e->kind) {
            case 0x11: {
                char buf[0x228];
                if (fld_cmd_poll(kind, buf))
                    s->done = 1;
                break;
            }
            case 0x1E: {
                char buf[8];
                if (fld_cmd_poll(kind, buf))
                    s->done = 1;
                break;
            }
            case 0x14: {
                int r;
                if (fld_cmd_poll(kind, &r))
                    s->done = 1;
                break;
            }
            case 0xC:
                s->done = 1;
                break;
            default:
                if (fld_cmd_poll(kind, 0))
                    s->done = 1;
                break;
            }
            if (s->done) {
                s->started = 0;
                s->index++;
                s->done = 0;
                if (s->index >= 15)
                    s->active = 0;
            }
        }
    }
    return 0;
}

int field_draw_cb(void)
{
    return fld_sys_draw();
}

void field_exit_cb(void)
{
}

void field_counter_b_add(int unused, int i, int v)
{
    field_counter_b[i] += v;
}

void field_counter_a_add(int i, int v)
{
    field_counter_a[i] += v;
}

/* bg id of a room of the menu */
int field_bg_of_room(int room)
{
    int r;

    switch (room) {
    case 0: r = 0xC; break;
    case 1: r = 0x25; break;
    case 2: r = 0xD; break;
    case 3: r = 0xE; break;
    case 4: r = 0xF; break;
    case 5: r = 0x10; break;
    case 6: r = 0x11; break;
    case 7: r = 0x12; break;
    case 8: r = 0x13; break;
    case 9: r = 0x14; break;
    case 10: r = 0x15; break;
    case 11: r = 0x16; break;
    case 12: r = 0x17; break;
    case 13: r = 0x18; break;
    case 14: r = 0x1; break;
    case 15: r = 0x2; break;
    case 16: r = 0x3; break;
    case 17: r = 0x4; break;
    case 18: r = 0x5; break;
    case 19: r = 0x1A; break;
    case 20: r = 0x19; break;
    case 21: r = 0x1B; break;
    case 22: r = 0x1D; break;
    case 23: r = 0x1E; break;
    case 24: r = 0x1F; break;
    case 25: r = 0x20; break;
    case 26: r = 0x24; break;
    case 27: r = 0x27; break;
    }
    return r;
}
