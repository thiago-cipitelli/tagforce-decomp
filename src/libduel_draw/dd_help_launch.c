// @range 0x08864B80-0x08865060
// @rodata 0x0887288C-0x088728A4
/* Help screen launched from the duel: checks it is allowed, fades out the duel, runs the help
   scene (duel or deck help by the current phase) on its own draw layer and fades back. */
#include "dd_duelui.h"

extern int hl_prev_mode;               /* 0x08C5B968 draw mode before the help */
extern int hl_state;                   /* 0x08C5B96C 0 fade out, 1 help, 2 fade back */
extern int hl_category;                /* 0x08C5B970 */

int dd_mode_get(void);
void dd_mode_push(int mode);
void dd_mode_set_flag(int mode);
void *dd_mode_draw_one(void *dl, int mode);
void dd_mode_pop(void);
void *dd_heap_get(void);
void *dd_gim_get(int id);
void *dd_bin_get(int id);
int dd_view_x14_get(void);
int dd_is_tag_duel(void);
int dd_cutin_looping(void);
void dd_scene_flags(int id, int on);
int dv_phase(void);
int dv_08835924(void);
void sys_frame_wait(void);

int sysvar_get_a98(void);
int req_flag_get(int i);
int cfg_get_language(void);
int fade_is_done(void);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_apply(void **list);
int dl_get_layer(void);
int dl_set_list(void *list);
void *dl_select_layer(int layer, unsigned int *size);
void hscene_init(int category, void *heap, void *frame_gim, void *texts, void *pack);
int hscene_update(void);
void *hscene_draw(void *dl);
int hscene_term(void);

int hl_allowed(void);
void hl_close(void);

int hl_open(void)
{
    if (hl_allowed() == 0)
        return 0;
    hl_prev_mode = dd_mode_get();
    dd_mode_push(8);
    dd_mode_set_flag(0);
    return 1;
}

void hl_start(void)
{
    int cat = 0;

    switch (dv_phase()) {
    case 0:
        cat = 4;
        break;
    case 1:
        cat = 5;
        break;
    case 2:
    case 4:
        cat = 6;
        break;
    case 3:
        cat = 7;
        break;
    case 5:
        cat = 8;
        break;
    }
    if (dv_08835924() == 5 || dv_08835924() == 4 || (hl_prev_mode == 4 && dd_view_x14_get() == 2))
        cat = 9;
    hl_category = cat;
    hl_state = 0;
    fade_start_in(1, 0xFF000000, 16);
}

void hl_term(void)
{
    while (!hscene_term())
        sys_frame_wait();
}

void *hl_draw(void *dl)
{
    int layer;

    switch (hl_state) {
    case 0:
    case 2:
        dl = dd_mode_draw_one(dl, hl_prev_mode);
        layer = dl_get_layer();
        dl_set_list(dl);
        dl = dl_select_layer(9, 0);
        dl = dl_disable(dl, 1);
        fade_apply(&dl);
        dl = dl_enable(dl, 1);
        dl_set_list(dl);
        dl = dl_select_layer(layer, 0);
        break;
    case 1:
        dl = dl_disable(dl, 1);
        dl = hscene_draw(dl);
        dl = dl_enable(dl, 1);
        font_set_flag8000(0);
        break;
    }
    return dl;
}

void hl_update(void)
{
    void *heap;
    void *gim;
    void *texts;
    int done;

    dd_mode_set_flag(0);
    switch (hl_state) {
    case 0:
        if (fade_is_done()) {
            hl_state = 1;
            dd_scene_flags(0x87, 0);
            heap = dd_heap_get();
            gim = dd_gim_get(0x40020);
            texts = dd_bin_get(cfg_get_language() + 0x80017);
            hscene_init(hl_category, heap, gim, texts, dd_bin_get(cfg_get_language() + 0x8001E));
        }
        break;
    case 2:
        if (fade_is_done())
            dd_mode_pop();
        break;
    case 1:
        done = hscene_update();
        font_set_flag8000(0);
        if (done)
            hl_close();
        break;
    }
}

int hl_running(void)
{
    return hl_state == 1;
}

void hl_close_alias(void)
{
    hl_close();
}

/* the help is available outside link duels, tag-duel partner turns and some modes */
int hl_allowed(void)
{
    int m;

    if (game_state_get()->pad10e[0x118 - 0x10E] == 5)
        return 0;
    m = dd_mode_get();
    switch (m) {
    case 6:
    case 8:
    case 5:
        return 0;
    }
    if (sysvar_get_a98() != 0)
        return 0;
    if (dd_is_tag_duel() != 0)
        return 0;
    if (dd_cutin_looping() != 0)
        return 0;
    return !req_flag_get(2);
}

void hl_close(void)
{
    hl_state = 2;
    dd_scene_flags(0x87, 1);
    fade_start(1, 0xFF000000, 16);
}
