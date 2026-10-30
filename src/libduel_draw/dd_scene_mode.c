// @range 0x0881CA2C-0x0881DF94
// @asm 0x0881CA74-0x0881CA80
// @asm 0x0881CAC8-0x0881CB84
// @asm 0x0881CB90-0x0881CC60
// @asm 0x0881CC60-0x0881CCF8
// @asm 0x0881CCF8-0x0881CD80
// @asm 0x0881CD80-0x0881CDD4
// @asm 0x0881CDD4-0x0881CEDC
// @asm 0x0881CEDC-0x0881CFF8
// @asm 0x0881CFF8-0x0881D044
// @asm 0x0881D1B0-0x0881D658
// @asm 0x0881D87C-0x0881D9E0
// @asm 0x0881D9E0-0x0881DB14
// @asm 0x0881DBC8-0x0881DC40
// @asm 0x0881DC40-0x0881DE48
// @data 0x08875F84-0x088760B0
/* The duel scene's input modes: a stack of modes (1 field, 2 card list, 3 yes/no, 4 numbers,
   5 card info, 6.. other sub-screens), each with enter/draw/update/leave; the scene heaps, the
   message pack, the replay pad and the camera taken from an animation. */
#include "dd_field_ui.h"

typedef struct DdModeDef {
    int id;
    void (*enter)(void);
    unsigned int *(*draw)(unsigned int *list);
    void (*update)(void);
    void (*leave)(void);
} DdModeDef;

typedef struct DdModeStack {           /* 0x44 at 0x08C57468 */
    int mode;                          /* +0x00 */
    int flag;                          /* +0x04 request flag while the mode runs, -1 none */
    struct {
        int mode, flag;
    } stack[4];                        /* +0x08 */
    int depth;                         /* +0x28 */
    int entering;                      /* +0x2C first frame: wait for the input to settle */
    int cancel_armed;                  /* +0x30 the cancel button was released */
    void (*menu_cb)(void);             /* +0x34 */
    void (*sel_cb)(void);              /* +0x38 */
    int card;                          /* +0x3C card shown by mode 5 */
    int phase;                         /* +0x40 mode 5: 0 fade out, 1 shown, 2 fade in */
} DdModeStack;

extern void *dd_heap_main;             /* 0x08C57460 */
extern void *dd_heap_sub;              /* 0x08C5745C */
extern int dd_hand_hint;               /* 0x08C57464 */
extern DdModeStack dd_mode;            /* 0x08C57468 */
extern void *dd_msg_pack;              /* 0x08C574AC */
extern unsigned int dd_pad_none[10];   /* 0x08C574B0 */
extern int (*dd_act_filter)(void);     /* 0x08C574D8 */
extern int dd_no_input;                /* 0x08C574DC */
extern int dd_msg_loaded;              /* 0x08C574E0 */
extern MenuWidget dd_menu3;            /* 0x08C574E4 */

void dd_m1_enter(void);
unsigned int *dd_m1_draw(unsigned int *list);
void dd_m1_update(void);
void dd_m1_leave(void);
void dd_m2_enter(void);
unsigned int *dd_m2_draw(unsigned int *list);
void dd_m2_update(void);
void dd_m2_leave(void);
void dd_m3_enter(void);
unsigned int *dd_m3_draw(unsigned int *list);
void dd_m3_update(void);
void dd_m3_leave(void);
void dd_m4_enter(void);
unsigned int *dd_m4_draw(unsigned int *list);
void dd_m4_update(void);
void dd_m4_leave(void);
void dd_m5_enter(void);
unsigned int *dd_m5_draw(unsigned int *list);
void dd_m5_update(void);
void dd_m5_leave(void);
unsigned int *dd_m10_draw(unsigned int *list);
void zone_nop1(void);
unsigned int *zone_identity(unsigned int *list);
void zone_confirm_hurry(void);
void zone_nop2(void);
void browse_start(void);
unsigned int *browse_identity(unsigned int *list);
void browse_update(void);
void browse_nop(void);
void hl_start(void);
unsigned int *hl_draw(unsigned int *list);
void hl_update(void);
void hl_term(void);
void opt_start(void);
unsigned int *opt_draw_layer(unsigned int *list);
void opt_tick(void);
void opt_term(void);
void sel_begin(void);
unsigned int *sel_draw(unsigned int *list);
void sel_update(void);
void sel_end(void);
void debug_menu_init(void);
unsigned int *debug_menu_draw(unsigned int *list);
void debug_menu_update(void);
void debug_menu_term(void);

DdModeDef dd_modes[13] = {
    { 0, 0, 0, 0, 0 },
    { 1, dd_m1_enter, dd_m1_draw, dd_m1_update, dd_m1_leave },
    { 2, dd_m2_enter, dd_m2_draw, dd_m2_update, dd_m2_leave },
    { 3, dd_m3_enter, dd_m3_draw, dd_m3_update, dd_m3_leave },
    { 4, dd_m4_enter, dd_m4_draw, dd_m4_update, dd_m4_leave },
    { 5, dd_m5_enter, dd_m5_draw, dd_m5_update, dd_m5_leave },
    { 6, zone_nop1, zone_identity, zone_confirm_hurry, zone_nop2 },
    { 7, browse_start, browse_identity, browse_update, browse_nop },
    { 8, hl_start, hl_draw, hl_update, hl_term },
    { 9, opt_start, opt_draw_layer, opt_tick, opt_term },
    { 10, sel_begin, sel_draw, sel_update, sel_end },
    { 10, 0, dd_m10_draw, 0, 0 },
    { 12, debug_menu_init, debug_menu_draw, debug_menu_update, debug_menu_term },
};
DdVec4 dd_cam_up __attribute__((aligned(16))) = { 0.0f, 1.0f, 0.0f, 0.0f };
DdVec4 dd_cam_fwd __attribute__((aligned(16))) = { 0.0f, 0.0f, -1.0f, 0.0f };

void *heap_alloc(void *h, int n);
int heap_set_level(void *h, void *p);
int req_flag_get(int i);
int dd_scene_flags(int a, int b);
int dd_08806BC4(void);
int dd_is_tag_duel(void);
int hl_open(void);
void dd_info_scroll(int v);
int dd_cobjs_idle(void);
int dv_turn_side(void);
int dv_player_is2(void);
int dv_is_type0(int a);
void guide_draw(void);
int func_08825764(void);
void func_088257B8(void);
int browse_active(void);
void browse_close(void);
int hl_running(void);
void func_08864F50(void);
int opt_is_open(void);
void opt_close(void);
int dd_sel_active(void);
void dd_sel_close(void);
int dd_sel_busy(void);
void dd_sel_update(void);
void *dd_sel_draw(void *list);
void dd_sel_reset(void);
void dd_cursor_move(int key);
void dd_cursor_cycle(void);
unsigned int dv_08835AEC(int player, int zone, int idx);
int dv_zone_card_id(int player, int zone, int idx);
int dd_loc_visible(DdLoc l);
void popup_open(void *arg);
void func_08820F88(void *arg);
void duel_08836028(void);
void browse_mode_set7(void);
void func_08825580(void);
unsigned int *func_088255E8(unsigned int *list);
unsigned int *func_08825674(void);
int func_088257D0(void);
int func_088257C4(void);
void func_08825778(void);
void func_08825588(void);
void *dd_gim_get(int id);
void *dd_bin_get(int id);
void libduel_draw_08855688(int card, int a, int b);
void *table_get_entry(void *pack, int i);
int fade_is_done(void);
void fade_start(int type, unsigned int color, int frames);
void fade_start_in(int type, unsigned int color, int frames);
int fade_apply(unsigned int **list);
int cardinfo_update(void);
int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim);
void cardinfo_set_layer(unsigned short layer);
void cardinfo_set_lists(void *list_gim, void *text_pack, void *index);
void cardinfo_draw(void);
int cardinfo_is_active(void);
void cardinfo_close(void);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
float keyframe_eval(void *tr, float t);
float atanf(float x);
void anim_eval_track(float *out, void *a, void *tr, float t);
float *mat4_transform_xyz(float *out, float *m, DdVec4 *v);
float *vec3_add_w(float *out, float *a, float *b);
void mtx_perspective(float fovy, float aspect, float near, float far);
void camera_set(float *pos, float *target, float *up);
void camera_load_view(void);
void dd_view_x10_set(int a);
void dv_08835F34(int player, int zone, int idx, int act);
void dd_field_camera0(void);
int dd_mode_get(void);
void dd_mode_pop(void);
int dd_mode_busy(void);
unsigned int *dd_pad(void);
int dd_input_on(void);
void dd_act(int player, int zone, int idx, int act);
void dd_m5_close(void);
void dd_menu3_update(void);
int dd_menu3_busy(void);
unsigned int *dd_menu3_draw(unsigned int *list);
void dd_menu3_reset(void);

void dd_heap_set(void *main, void *sub)
{
    dd_heap_main = main;
    dd_heap_sub = sub;
}

void dd_heap_clear(void)
{
    dd_heap_main = 0;
    dd_heap_sub = 0;
}

void *dd_alloc(int n)
{
    return heap_alloc(dd_heap_main, n);
}

int dd_free(void *p)
{
    return heap_set_level(dd_heap_main, p);
}

/* export: the scene heap (in asm: an unnamed export cannot sit between C functions of a file
   split by @asm, tools/fix_ld.py lists the C functions by their names) */
#ifdef NON_MATCHING
void *func_0881CA74(void)
{
    return dd_heap_main;
}
#endif

void *dd_alloc_sub(int n)
{
    return heap_alloc(dd_heap_sub, n);
}

int dd_free_sub(void *p)
{
    return heap_set_level(dd_heap_sub, p);
}

void dd_mode_init(void)
{
    memset(&dd_mode, 0, sizeof(DdModeStack));
}

void dd_mode_term(void)
{
    memset(&dd_mode, 0, sizeof(DdModeStack));
}

/* replaces the current mode */
#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
void dd_mode_set(int m)
{
    DdModeStack *s = &dd_mode;

    if (dd_modes[dd_mode.mode].leave != 0)
        dd_modes[s->mode].leave();
    s->flag = -1;
    s->entering = 1;
    s->mode = m;
    if (dd_modes[m].enter != 0)
        dd_modes[m].enter();
}
#endif

int dd_mode_get(void)
{
    return dd_mode.mode;
}

/* enters a mode over the current one */
#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
void dd_mode_push(int m)
{
    DdModeStack *s = &dd_mode;

    req_flag_get(0);
    s->stack[s->depth].mode = s->mode;
    s->stack[s->depth].flag = s->flag;
    s->depth++;
    s->flag = -1;
    s->entering = 1;
    s->mode = m;
    if (dd_modes[m].enter != 0)
        dd_modes[m].enter();
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
void dd_mode_pop(void)
{
    DdModeStack *s = &dd_mode;

    if (dd_modes[dd_mode.mode].leave != 0)
        dd_modes[s->mode].leave();
    s->depth--;
    s->mode = s->stack[s->depth].mode;
    s->flag = s->stack[s->depth].flag;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
unsigned int *dd_mode_draw(unsigned int *list)
{
    DdModeStack *s = &dd_mode;

    if (dd_mode.entering == 0 && dd_modes[s->mode].draw != 0)
        list = dd_modes[s->mode].draw(list);
    if (s->flag >= 0)
        req_flag_set(s->flag);
    return list;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
unsigned int *dd_mode_draw_one(unsigned int *list, int m)
{
    if (dd_modes[m].draw != 0)
        list = dd_modes[m].draw(list);
    return list;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
void dd_mode_update(void)
{
    DdModeStack *s = &dd_mode;
    unsigned int *pad = (unsigned int *)pad_get();
    int y = pad[5];

    if (y >= 0x9F) {
        dd_info_scroll((y - 0x9E) * 4);
        y = pad[5];
    }
    if (y < 0x62)
        dd_info_scroll((y - 0x62) * 4);
    if (dd_is_tag_duel() == 0 && (pad[1] & 8) && hl_open() != 0)
        se_play(0x6217);
    if (s->entering == 0 && dd_modes[s->mode].update != 0)
        dd_modes[s->mode].update();
    if (s->entering != 0 && dd_mode_busy() != 0)
        s->entering = 0;
}
#endif

/* closes the open window of the current mode (export used on interruptions) */
#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
int dd_mode_cancel(void)
{
    switch (dd_mode_get()) {
    case 2:
        if (dd_sel_active() != 0)
            dd_sel_close();
        break;
    case 3:
    case 4:
        if (func_08825764() != 0)
            func_088257B8();
        break;
    case 5:
        if (cardinfo_is_active() != 0)
            cardinfo_close();
        break;
    case 7:
        if (browse_active() != 0)
            browse_close();
        break;
    case 8:
        if (hl_running() != 0)
            func_08864F50();
        break;
    case 9:
        if (opt_is_open() != 0)
            opt_close();
        break;
    case 12:
        dd_mode_pop();
        break;
    }
    return dd_mode_get() == 1;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
int dd_mode_can_x(void)
{
    int m = dd_mode_get();
    int r;

    if (m == 5)
        r = 0;
    else if (m == 9 || m == 8)
        r = 0;
    else
        r = 1;
    return r;
}
#endif

void dd_mode_set_flag(int f)
{
    dd_mode.flag = f;
    if (f >= 0)
        req_flag_set(f);
}

int dd_mode_get_flag(void)
{
    return dd_mode.flag;
}

int dd_mode_busy(void)
{
    return dd_cobjs_idle() != 0;
}

/* mode 1: the field */
void dd_m1_enter(void)
{
    dd_scene_flags(7, 1);
    dd_mode.cancel_armed = 0;
}

unsigned int *dd_m1_draw(unsigned int *list)
{
    unsigned int *pad = dd_pad();
    int layer = dl_get_layer();

    dl_set_list(list);
    if (dd_08806BC4() == 0 && (pad[0] & 0x100) && dv_turn_side() == dv_player_is2() &&
        dv_is_type0(dv_player_is2()) != 0 && dd_is_tag_duel() == 0) {
        guide_draw();
        if (dd_hand_hint == 0) {
            dd_hand_hint = 1;
            se_play(0x621A);
        }
    } else {
        dd_hand_hint = 0;
    }
    return dl_select_layer(layer, 0);
}

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
void dd_m1_update(void)
{
    DdModeStack *s = &dd_mode;
    unsigned int *pad = dd_pad();
    unsigned int k;
    int player, zone;
    DdLoc l;
    struct {
        int card;
        void (*get)(DdLoc *out);
    } arg;
    struct {
        void (*get)(DdLoc *out);
    } arg2;

    if (pad[1] & 1)
        dd_scene_flags(0x80, dd_scene_flags(0x80, -1) == 0);
    if ((pad[0] & (region_pick_value(1) & 0xFFFF)) == 0 && (pad[2] & (region_pick_value(1) & 0xFFFF)) == 0)
        s->cancel_armed = 1;
    if (dd_08806BC4() == 2)
        return;
    if ((pad[0] & 0x100) == 0) {
        k = pad[3];
        if (k & 0x80) {
            dd_cursor_move(0x80);
        } else if (k & 0x10) {
            dd_cursor_move(0x10);
        } else if (k & 0x40) {
            dd_cursor_move(0x40);
        } else if (k & 0x20) {
            dd_cursor_move(0x20);
        }
    } else if (dd_is_tag_duel() == 0) {
        k = pad[1];
        if (k & 0x10) {
            dd_cursor_set(1, 7, 0, 0);
            se_play(0x6212);
        } else if (k & 0x40) {
            dd_cursor_set(0, 7, 0, 0);
            se_play(0x6212);
        } else if (k & 0x80) {
            dd_cursor_set(0, 0xA, 0, 0);
            se_play(0x6212);
        } else if (k & 0x20) {
            dd_cursor_set(1, 0xA, 0, 0);
            se_play(0x6212);
        }
    }
    if (pad[3] & 0x200)
        dd_cursor_cycle();
    if (dd_08806BC4() > 0)
        return;
    if (dd_is_tag_duel() == 0 && (pad[0] & 0x100)) {
        k = pad[1];
        player = -1;
        if (k & 0x2000) {
            player = 0;
            zone = 0xF;
        } else if (k & 0x4000) {
            player = 0;
            zone = 0xE;
        } else if (k & 0x1000) {
            player = 1;
            zone = 0xE;
        } else if (k & 0x8000) {
            player = 1;
            zone = 0xF;
        }
        if (player >= 0 && (dv_08835AEC(player, zone, 0) & 2)) {
            dd_act(player, zone, 0, 1);
            s->cancel_armed = 0;
        }
        return;
    }
    if (pad[1] & ((region_pick_value(1) & 0xFFFF) | 0x200)) {
        if (dv_08835AEC(0, 0xD, 0) & 0x1000) {
            se_play(0x6213);
            dd_act(0, 0xD, 0, 0xC);
            s->cancel_armed = 0;
            return;
        }
        if (dv_08835AEC(1, 0xD, 0) & 0x1000) {
            se_play(0x6213);
            dd_act(1, 0xD, 0, 0xC);
            s->cancel_armed = 0;
            return;
        }
    }
    k = pad[1];
    if (k & (region_pick_value(0) & 0xFFFF)) {
        memset(&arg2, 0, sizeof(arg2));
        arg2.get = dd_cursor_get;
        func_08820F88(&arg2);
        s->cancel_armed = 0;
    } else if (k & 0x1000) {
        dd_cursor_get(&l);
        arg.card = dv_zone_card_id(l.player, l.zone, l.idx);
        memset(&arg, 0, sizeof(arg));
        if (dd_loc_visible(l) == 0)
            arg.card = 0;
        arg.get = dd_cursor_get;
        popup_open(&arg);
        s->cancel_armed = 0;
    } else if ((pad[2] & (region_pick_value(1) & 0xFFFF)) && s->cancel_armed != 0) {
        duel_08836028();
        s->cancel_armed = 0;
    }
}
#endif

void dd_m1_leave(void)
{
    dd_scene_flags(7, 0);
}

void dd_mode_set_sel_cb(void (*cb)(void))
{
    dd_mode.sel_cb = cb;
}

/* mode 2: the card list */
void dd_m2_enter(void)
{
}

unsigned int *dd_m2_draw(unsigned int *list)
{
    return dd_sel_draw(list);
}

void dd_m2_update(void)
{
    DdModeStack *s = &dd_mode;
    unsigned int *pad = dd_pad();

    dd_sel_update();
    if (dd_sel_active() != 0 && (pad[1] & 0x8000)) {
        browse_mode_set7();
        return;
    }
    if (dd_sel_busy() != 0)
        return;
    if (pad[0] & (region_pick_value(1) & 0xFFFF))
        return;
    dd_mode_pop();
    if (s->sel_cb != 0)
        s->sel_cb();
}

void dd_m2_leave(void)
{
    dd_sel_reset();
}

/* mode 3: menu window */
void dd_m3_enter(void)
{
}

unsigned int *dd_m3_draw(unsigned int *list)
{
    return dd_menu3_draw(list);
}

void dd_m3_update(void)
{
    DdModeStack *s = &dd_mode;

    dd_menu3_update();
    if (dd_menu3_busy() != 0)
        return;
    dd_mode_pop();
    if (s->menu_cb != 0)
        s->menu_cb();
}

void dd_m3_leave(void)
{
    dd_menu3_reset();
}

/* mode 4: numeric input */
void dd_m4_enter(void)
{
    func_08825580();
}

unsigned int *dd_m4_draw(unsigned int *list)
{
    return func_088255E8(list);
}

void dd_m4_update(void)
{
    unsigned int *pad = dd_pad();
    int done = (int)func_08825674();

    if (func_08825764() != 0 && func_088257D0() != 0 && func_088257C4() != 1 && (pad[1] & 0x8000)) {
        browse_mode_set7();
        return;
    }
    if (done != 0) {
        dd_mode_pop();
        func_08825778();
    }
}

void dd_m4_leave(void)
{
    func_08825588();
}

void dd_mode_set_card(int card)
{
    dd_mode.card = card;
}

/* mode 5: card information screen */
void dd_m5_enter(void)
{
    dd_mode.phase = 0;
    fade_start_in(1, 0xFF000000, 0x10);
}

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
unsigned int *dd_m5_draw(unsigned int *list)
{
    DdModeStack *s = &dd_mode;
    int layer = dl_get_layer();

    dl_set_list(list);
    switch (dd_mode.phase) {
    case 1:
        list = dl_select_layer(4, 0);
        list = dl_clear_color(list, 0xFF000000);
        list = dl_clear_depth(list, 0);
        list = dl_clear_stencil(list, 0);
        list = dl_clear(list, 0x17);
        dl_set_list(list);
        cardinfo_draw();
        break;
    case 2:
    case 0:
        if (s->depth > 0) {
            list = dl_select_layer(layer, 0);
            list = dd_mode_draw_one(list, s->stack[s->depth - 1].mode);
            dl_set_list(list);
        }
        list = dl_select_layer(9, 0);
        list = dl_disable(list, 1);
        fade_apply(&list);
        list = dl_enable(list, 1);
        dl_set_list(list);
        break;
    }
    list = dl_select_layer(layer, 0);
    return list;
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
void dd_m5_update(void)
{
    int done = 0;
    void **pack;

    switch (dd_mode.phase) {
    case 1:
        if (dd_input_on() != 0)
            done = cardinfo_update();
        if (done != 0)
            dd_m5_close();
        break;
    case 2:
        if (fade_is_done() != 0)
            dd_mode_pop();
        break;
    case 0:
        if (fade_is_done() != 0) {
            dd_scene_flags(7, 0);
            dd_mode.phase = 1;
            cardinfo_open(dd_mode.card, dd_gim_get(0x4001B), dd_gim_get(0x4001C));
            cardinfo_set_layer(8);
            pack = dd_bin_get(0x80024);
            cardinfo_set_lists(dd_gim_get(0x40024), pack[1], pack[0]);
        }
        break;
    }
}
#endif

void dd_m5_leave(void)
{
    dd_scene_flags(7, 1);
}

void dd_m5_close(void)
{
    dd_scene_flags(7, 1);
    dd_mode.phase = 2;
    fade_start(1, 0xFF000000, 0x10);
}

/* mode 11: the card zoom */
unsigned int *dd_m10_draw(unsigned int *list)
{
    int card = dd_mode.card;
    int layer = dl_get_layer();

    dl_set_list(list);
    libduel_draw_08855688(card, 0, 3);
    dd_mode_pop();
    return dl_select_layer(layer, 0);
}

/* a text of the scene's message pack (by language) */
#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
unsigned short *dd_text(int id)
{
    if (dd_msg_pack == 0 || dd_msg_loaded == 0) {
        dd_msg_pack = dd_bin_get(cfg_get_language() + 0x8000D);
        dd_msg_loaded = 1;
    }
    return table_get_entry(dd_msg_pack, id);
}
#endif

/* camera of an animation at time t (position, target, fov, near/far keyed) */
#ifdef NON_MATCHING /* forma reconstruída; endereçamento da tabela de modos e alocação diferentes */
int dd_camera_from_anim(void *a, float t)
{
    int *anim = a;
    float fov_w, fov_h, near, far, fovy;
    unsigned short *idx;
    int *keys;
    float pos[4];
    float rot[4];
    float m[16];
    float target[4];
    float up[4];
    float fwd[4];

    if (anim == 0)
        return 0;
    fov_w = keyframe_eval((void *)anim[2], t);
    fov_h = keyframe_eval((void *)(anim[2] + 8), t);
    near = keyframe_eval((void *)(anim[2] + 0x10), t);
    far = keyframe_eval((void *)(anim[2] + 0x18), t);
    if (near < 1.0f)
        near = 1.0f;
    fovy = atanf((0.5f * fov_h) / (0.03937f * fov_w)) * 2.0f;
    idx = (unsigned short *)anim[1];
    keys = (int *)(anim[2] + idx[0] * 8);
    pos[0] = keyframe_eval(keys, t);
    pos[1] = keyframe_eval(keys + 2, t);
    pos[2] = keyframe_eval(keys + 4, t);
    anim_eval_track(rot, anim, (void *)(anim[4] + idx[1] * 8), t);
    quat_to_mat4(m, rot);
    mat4_transform_xyz(up, m, &dd_cam_up);
    mat4_transform_xyz(fwd, m, &dd_cam_fwd);
    vec3_add_w(target, pos, fwd);
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(fovy, 1.7647059f, near, far);
    mtx_set_mode(1);
    mtx_identity();
    camera_set(pos, target, up);
    camera_load_view();
    return 1;
}
#endif

void dd_mode_de48(void)
{
    dd_field_camera0();
}

/* the pad, or an empty one while the input is off (replays) */
unsigned int *dd_pad(void)
{
    if (dd_no_input != 0) {
        memset(dd_pad_none, 0, sizeof(dd_pad_none));
        return dd_pad_none;
    }
    return (unsigned int *)pad_get();
}

void dd_input_set(int on)
{
    dd_no_input = !on;
}

int dd_input_on(void)
{
    return !dd_no_input;
}

/* sends an action on a location to the engine (unless the filter refuses it) */
void dd_act(int player, int zone, int idx, int act)
{
    if (dd_act_filter == 0 || dd_act_filter() != 0) {
        dd_view_x10_set(0);
        dv_08835F34(player, zone, idx, act);
    }
}

void dd_act_set_filter(int (*f)(void))
{
    dd_act_filter = f;
}

int dd_int_cmp(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

void dd_menu3_reset(void)
{
    menu_reset(&dd_menu3);
}

unsigned int *dd_menu3_draw(unsigned int *list)
{
    menu_draw((void **)&list, &dd_menu3);
    return list;
}
