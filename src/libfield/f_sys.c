// @range 0x0880FBB4-0x08810814
// @asm 0x0880FBB4-0x0880FD04
// @asm 0x0880FF00-0x08810008
// @rodata 0x08838854-0x08838878
/* The field system: init / per-frame update / draw / staged termination of all the subsystems, and
   the field commands run by the scripts (start and poll through the command table): context,
   camera, scene open/close, fade, ... */
#include "libfield.h"

void fld_ctx_init(void);
void fld_res_init(void);
int fld_res_term(void);
void fld_res_update(void);
int fld_day_part(void);
int fld_bg_init(void);
void fld_bg_term(void);
void fld_bg_free(void);
void fld_bg_update(void);
void fld_bg_draw(void);
void fld_items_init(void);
void fld_items_term(void);
void fld_items_free(void);
void fld_items_update(void);
void fld_items_draw(void);
void fld_chr_init(void);
void fld_chr_term(void);
void fld_chr_free_all(void);
void fld_npc_init(void);
void fld_npc_term(void);
void fld_npc_free(void);
void fld_npc_update(void);
void fld_npc_draw(void);
void fld_bu_init(void);
void fld_bu_term(void);
void fld_bu_free_all(void);
void fld_bu_update(void);
void fld_bu_draw(void);
void fld_gim_free(int name);
void fld_load_cancel_chr(void);
void fld_load_cancel_bg(void);
void fld_load_cancel_all(void);
void fld_req_cancel_all(void);
void fld_player_init(void);
void fld_player_reset(void);
void fld_sys_init_all(void);
void fld_sys_free_all(void);
void fld_sys_term_all(void);
void fld_sys_poll_all(void);
void fld_scene_init(void);
void fld_scene_term(void);
void fld_scene_free(void);
int fld_cmd_poll(int kind, void *arg);

void ftalk_clear(void);
void ftalk_free(void);
void ftalk_nop(void);
void ftalk_update(void);
void ftalk_draw(void);
void fmoney_init(int a, int b, int c, int d);
void fmoney_free(void);
void fmoney_update(void);
void *fmoney_draw(void *list);
void fmoney_add(int a, int b, int c);
int fmoney_shown(void);
void fmoney_open(void);
void fmoney_blink(int a, int b);
void fcam_init(void);
void fcam_nop(void);
void fcam_nop2(void);
void fcam_update(void);
void fcam_begin(void);
void fbm_clear(void);
void fbm_free(void);
void fbm_nop(void);
void fbm_update(void);
void fbm_draw(void);
void fbm_setup(FieldBmCmd *c);
int fbm_ready(void);
void fld_scene_update(void);
void fld_scene_draw(void);
void fld_scene_tab_init(void);
void fld_scene_tab_free(void);
void fld_hud_init(void);
void fld_hud_free(void);
void fld_hud_open(int a);
void fld_hud_blink(int a, int b, int c);
void fpt_clear(void);
void fpt_reset(void);
void func_0881C770(void);
void func_0881C784(void);
void func_0881C7A8(void);
void func_0881C904(void);
void func_08825F7C(void);
void func_08825F90(void);
void func_088274B4(void);
void func_088274C8(void);
void func_088274EC(void);
void func_088274F4(void);
void func_088275FC(void);
void func_08827788(int a);
int func_08827810(void);
void func_08829F5C(void);
void func_08829F70(void);
void func_08829F94(void);
void func_08829F9C(void);
void func_0882A068(void);
void func_0882A404(void);
void func_0882A918(void);
void func_0882AB1C(void);
void func_0882AB24(void);

extern int field_term0, field_term1, field_term2, field_term3, field_term4; /* 0x08FD1A30.. */
extern unsigned int field_fade_colors[];       /* 0x0883884C */
extern FieldScene field_scene;

#ifdef NON_MATCHING /* o laço de 25 comandos sai desenrolado x8 com resto no original e x5 aqui */
void fld_sys_init(void)
{
    int i;
    int a;
    int b;

    memset(&field_sys, 0, sizeof(FieldSys));
    dl_set_list(view_init(dl_select_layer(0, 0)));
    model_set_allocator(fld_alloc, fld_free);
    for (i = 0; i < 25; i++)
        field_cmds[i].busy = 0;
    fld_ctx_init();
    fld_res_init();
    efs_set_heap(fld_heap_get());
    fld_sys_init_all();
    a = 0x39;
    b = 0x39;
    switch (fld_day_part()) {
    case 2:
    case 3:
        a -= 0x10;
        b -= 0x10;
        break;
    }
    fmoney_init(500, a, 0x16C, b);
    fmsg_init(fld_heap_get(), 0x27, 400, 0xE, 1, 3);
}
#endif

void fld_sys_update(void)
{
    FieldSys *s = &field_sys;

    if (field_sys.active) {
        fmoney_update();
        fld_res_update();
        fmsg_update();
        fcam_update();
        fbm_update();
        fld_bg_update();
        if (s->mode == 0)
            func_0882AB24();
        fld_items_update();
        fld_npc_update();
        fld_scene_update();
        fld_bu_update();
        ftalk_update();
        func_088274F4();
        func_08829F9C();
        fld_sys_poll_all();
        fld_ctx_get()->frame++;
    }
}

int fld_sys_draw(void)
{
    FieldSys *s = &field_sys;
    void *dl;

    if (field_sys.active) {
        if (s->fading) {
            dl = dl_select_layer(6, 0);
            dl = dl_disable(dl, 1);
            if (fade_apply(&dl)) {
                if (s->fade_out == 0)
                    s->drawing = 0;
                s->fading = 0;
            }
            dl_set_list(dl);
        }
        if (s->drawing) {
            dl = dl_select_layer(4, 0);
            fmsg_draw(&dl);
            dl_set_list(dl);
            fcam_begin();
            fbm_draw();
            fld_bg_draw();
            fld_items_draw();
            dl = dl_select_layer(3, 0);
            dl = fmoney_draw(dl);
            dl_set_list(dl);
            fld_scene_draw();
            func_0882A068();
            fld_bu_draw();
            fld_npc_draw();
            func_0881C904();
            ftalk_draw();
            func_088275FC();
        }
    }
}

#ifdef NON_MATCHING /* o original carrega o primeiro flag antes de salvar ra (escalonamento do prólogo) */
int fld_sys_term(void)
{
    if (field_term0 == 0) {
        fld_sys_free_all();
        field_term0 = 1;
    }
    if (field_term1 == 0) {
        efs_term();
        field_term1 = 1;
    }
    if (field_term2 == 0) {
        fmoney_free();
        field_term2 = 1;
    }
    if (field_term3 == 0) {
        fmsg_free();
        field_term3 = 1;
    }
    if (field_term4 == 0) {
        if (fld_res_term() == 0)
            return 0;
        field_term4 = 1;
    }
    model_set_allocator(0, 0);
    field_term0 = 0;
    field_term1 = 0;
    field_term2 = 0;
    field_term3 = 0;
    field_term4 = 0;
    return 1;
}
#endif

int fld_cmd_start(int kind, void *arg)
{
    FieldCmd *c = &field_cmds[kind];

    field_sys.active = 1;
    if (c->busy) {
        if (fld_cmd_poll(kind, 0) == 0)
            return 0;
    }
    c->start(arg);
    c->busy = 1;
    return 1;
}

int fld_cmd_poll(int kind, void *arg)
{
    FieldCmd *c = &field_cmds[kind];

    if (c->poll(arg)) {
        c->busy = 0;
        return 1;
    }
    return 0;
}

void fld_cmd_mode(int *a)
{
    FieldCtx *c = fld_ctx_get();

    field_sys.x00 = 1;
    c->x14 = *a;
    field_sys.mode = 0;
    field_sys.x00 = 0;
}

int fld_cmd_mode_done(void)
{
    return !field_sys.x00;
}

void fld_cmd_story(FieldBmCmd *c)
{
    field_sys.x04 = 1;
    fbm_setup(c);
    field_sys.mode = 1;
}

int fld_cmd_story_done(void)
{
    return !field_sys.x04;
}

void fld_cmd_open(int *a)
{
    FieldSys *s = &field_sys;
    int m;
    FieldCtx *c;

    if (a[1] >= 12)
        a[1] = 1;
    s->x08 = 1;
    if (a[0])
        func_08827788(a[1]);
    switch (s->mode) {
    case 0:
        m = 0;
        break;
    case 1:
        m = 1;
        break;
    }
    fld_hud_open(m);
    fmoney_open();
    c = fld_ctx_get();
    c->se_on = 1;
    c->pad10 = 1;
    if (s->mode == 0)
        func_0882A918();
}

int fld_cmd_open_done(void)
{
    return !field_sys.x08;
}

void fld_cmd_close(void)
{
    FieldSys *s = &field_sys;
    int a;
    int b;
    FieldCtx *c;

    field_sys.x0c = 1;
    fmoney_free();
    a = 0x39;
    b = 0x39;
    switch (fld_day_part()) {
    case 2:
    case 3:
        a -= 0x10;
        b -= 0x10;
        break;
    }
    fmoney_init(500, a, 0x16C, b);
    fmsg_free();
    fmsg_init(fld_heap_get(), 0x27, 400, 0xE, 1, 3);
    fld_sys_term_all();
    s->active = 0;
    c = fld_ctx_get();
    c->se_on = 0;
    c->pad10 = 0;
    if (s->mode == 0)
        func_0882AB1C();
    s->x0c = 0;
}

int fld_cmd_close_done(void)
{
    return !field_sys.x0c;
}

void fld_cmd_fade(FieldFadeCmd *f)
{
    FieldSys *s = &field_sys;

    field_sys.fading = 1;
    if (f->out)
        fade_start(1, field_fade_colors[f->color], f->frames);
    else
        fade_start_in(1, field_fade_colors[f->color], f->frames);
    s->fade_out = f->out;
    if (f->out)
        s->drawing = 1;
}

int fld_cmd_fade_done(void)
{
    return !field_sys.fading;
}

void fld_cmd_x14(int *a)
{
    fmoney_add(*a, 200, 100);
    field_sys.x14 = 1;
}

int fld_cmd_x14_done(void)
{
    return !field_sys.x14;
}

void fld_cmd_x18(int *a)
{
    switch (*a) {
    case -1:
    default:
        fld_hud_blink(-1, 0, 0);
        fmoney_blink(-1, 0);
        break;
    case 0:
        fld_hud_blink(10, 5, 0);
        break;
    case 1:
        fld_hud_blink(10, 5, 1);
        break;
    case 2:
        fld_hud_blink(10, 5, 2);
        break;
    case 3:
        fld_hud_blink(10, 5, 3);
        break;
    case 4:
        fld_hud_blink(10, 5, 4);
        break;
    case 6:
        fld_hud_blink(10, 5, 5);
        break;
    case 7:
        fld_hud_blink(10, 5, 6);
        break;
    case 5:
        fmoney_blink(10, 5);
        break;
    }
}

int fld_cmd_x18_done(void)
{
    return !field_sys.x18;
}

void fld_sys_init_all(void)
{
    fcam_init();
    func_0881C770();
    fbm_clear();
    fld_bg_init();
    fld_items_init();
    fld_chr_init();
    fld_npc_init();
    fld_scene_init();
    fld_bu_init();
    ftalk_clear();
    func_088274B4();
    func_08829F5C();
}

void fld_sys_free_all(void)
{
    fbm_nop();
    fld_bg_free();
    fld_npc_free();
    fld_scene_free();
    fld_chr_free_all();
    fld_items_free();
    ftalk_nop();
    fld_bu_free_all();
    func_0881C7A8();
    fcam_nop2();
    func_088274EC();
    func_08829F94();
}

void fld_sys_term_all(void)
{
    int i;

    fcam_nop();
    func_0881C784();
    fbm_free();
    fld_bg_term();
    fld_items_term();
    fld_chr_term();
    fld_npc_term();
    fld_scene_term();
    fld_bu_term();
    ftalk_free();
    func_088274C8();
    func_08829F70();
    for (i = 1; i < 9; i++)
        fld_gim_free(i);
    fld_load_cancel_chr();
    fld_load_cancel_bg();
    fld_load_cancel_all();
    fld_req_cancel_all();
}

void fld_sys_poll_all(void)
{
    FieldSys *s = &field_sys;

    if (func_08827810())
        s->x08 = 0;
    if (fbm_ready())
        s->x04 = 0;
    if (fmoney_shown())
        s->x14 = 0;
}

void fld_scene_init(void)
{
    memset(&field_scene, 0, sizeof(FieldScene));
    fld_hud_init();
    func_08825F7C();
    fld_player_init();
    fpt_clear();
    fld_scene_tab_init();
}

void fld_scene_term(void)
{
    fld_scene_free();
    fld_scene_init();
}

void fld_scene_free(void)
{
    fld_hud_free();
    func_08825F90();
    fld_player_reset();
    fpt_reset();
    fld_scene_tab_free();
    func_0882A404();
}
