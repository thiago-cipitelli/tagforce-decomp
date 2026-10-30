// @range 0x08807064-0x0880720C
/* The duel view as a whole: init and free of its parts (field, cursor, cards, LP panel...), the
   3D pass of each frame and the per-frame update, gated by the scene flags. */
#include "libduel_draw.h"

/* other ranges of libduel_draw */
void dd_field_init(void);
void dd_field_free(void);
void dd_field_draw(void);
void dd_field_update(float dt);
void dd_info_init(void);
void dd_info_term(void);
void dd_info_draw(void);
void dd_info_update(float dt);
void func_08809B1C(void);
void func_08809CE4(void);
void func_08809E4C(void);
void func_08809EF0(float dt);
void func_088116FC(void);
void func_08811710(void);
void func_08811718(void);
void func_08811C60(float dt);
void func_08814DFC(void);
void func_08816FD0(void);
void func_08816FE4(void);

/* reads the view parameters (debug build only; does nothing here) */
int dd_param_load(int a, const char *path)
{
    return 0;
}

void dd_view_init(void)
{
    dd_field_init();
    dd_info_init();
    func_08814DFC();
    func_08809B1C();
    func_08816FD0();
    func_088116FC();
}

void dd_view_free(void)
{
    dd_field_free();
    dd_info_term();
    func_08809CE4();
    func_08816FE4();
    func_08811710();
}

/* clears layer 0 and draws the field, the cards and the effects */
void dd_view_draw3d(void)
{
    dl_set_list(dl_clear(dl_clear_stencil(dl_clear_depth(dl_clear_color(dl_select_layer(0, 0), 0), 0), 0), 0x17));
    if (dd_scene_flags(1, -1))
        dd_field_draw();
    if (dd_scene_flags(2, -1))
        func_08809E4C();
    if (dd_scene_flags(4, -1))
        func_08811718();
    if (dd_scene_flags(4, -1))
        dd_info_draw();
}

void dd_view_update(float dt)
{
    if (dd_scene_flags(1, -1))
        dd_field_update(dt);
    func_08809EF0(dt);
    func_08811C60(dt);
    dd_info_update(dt);
}
