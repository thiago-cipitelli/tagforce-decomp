// @range 0x08810814-0x08811BE0
// @asm 0x08810814-0x08810F44
// @asm 0x088114E8-0x0881181C
// @asm 0x0881181C-0x08811B1C
/* Scene commands of the field scripts: the story parameters of the scene, placing the player,
   the camera target, saving the positions of the player and the NPCs, door sounds, waits, and the
   scene GIM loaded with them. */
#include "libfield.h"

void fpt_setup(int a, int b);
void fpt_free(void);
void fld_hud_hide(void);
void fld_hud_move(int a, int b, int c);
void fget_card_open(int a, int b, int c);
void fmoney_set_gim(void *gim);
void fcal_open(void *heap, int a, int b);
void fget_load(void);
void fld_scene_gim_loaded(int *file, void *arg);

extern unsigned char field_story_ext[0x10];    /* 0x08FD5748 */

void fld_player_draw(void);
void fpt_draw(void);
void fld_hud_draw(void);
void fget_draw(void);
void fld_sctab_draw(void);
void *fdlg_draw_buttons(void *list, unsigned int mask);
void *fld_dlg_draw(void *list);
void *fbag_draw(void *list);
void *fld_dl_clear(void *list, int z, unsigned int color);
void fcal_draw(void);
void *fclk_draw(void *list);
void fegg_draw(void);
void *fcho_draw(void *list);

void fld_voice_play(int n);
int fld_npc_facing_id(FieldNpc *p);
int fld_npc_get_c0(int id);
void fld_npc_turn_to(int id, FieldNpc *p);
void fld_npc_turn_back(void);
void fld_player_lock(void);
int fld_player_loaded(void);
int fld_player_idle(void);
void fld_scene_input(int *cmd, int *arg);
void fld_scene_frame(void);
void fld_sctab_update(int *cmd, int *arg, int *picked);
void fld_sctab_open(void);
void fld_hud_show(void);
int fld_hud_hidden(void);
int fmsg_done(void);
MenuItemDef *fmsg_def(void);
void fmsg_set_input(int on);
void fmsg_set_text(unsigned short *text, int title);
void fmsg_close(void);
int fpt_ready(void);
void fbag_init(void);
int fbag_update(int *picked);
int fcal_update(void);
void fget_item_open(int item);
void fget_item_close(int *flag);
void fget_card_close(int *flag);
void fclk_open(void);
int fclk_update(void);
void fclk_get(int *hour, int *minute);
void fegg_init(int rare, int kind);
int fegg_update(void);
void fcho_open(void);
int fcho_update(void);
void fld_dlg_open(unsigned short *text, int buttons, int button);
int fld_dlg_update(void);

#ifdef NON_MATCHING /* C novo, conferido com o asm (mesmo tamanho). 161 palavras, quase todas deslocamento de duas diferenças: no case 5 o original não põe o move a0 no atraso do jal de fld_voice_play, e no case 5 do calendário recarrega o 23 que aqui é reaproveitado */
/* per frame: the NPC in front of the player, then the scene mode (x88): 0 lock and turn back,
   1 input, 2/3 the question box, 5/6 talking (sctab), 7/8 the bag, 9/10 a message, 11/12 the
   calendar, 13..16 the get windows, 17/18 the clock, 19/20 the choice list, 23 leave for the
   scene command (bg); while no scene runs, the egg (21/22); then fld_scene_frame */
void fld_scene_update(void)
{
    FieldScene *s = &field_scene;
    int *mode;
    int *arg;
    FieldChr *pl;
    MenuItemDef *d;
    int r;

    fld_ctx_get();
    mode = &field_scene.x88;
    pl = fld_player_get();
    field_scene.pad8c[0] = fld_npc_facing_id((FieldNpc *)pl);
    if (field_scene.saving != 0) {
        arg = &s->bg;
        switch (*mode) {
        case 0:
            fld_player_lock();
            fld_npc_turn_back();
            *mode = 1;
            break;
        case 1:
            fld_scene_input(mode, arg);
            break;
        case 2:
            fld_dlg_open(table_get_entry(fld_gim_get(5), 0x66), 3, 2);
            *mode = 3;
            break;
        case 3:
            switch (fld_dlg_update()) {
            case -2:
            case 3:
                *mode = 0;
                break;
            case 2:
                *mode = 23;
                *arg = 1;
                break;
            }
            break;
        case 4:
            if (fld_player_idle() != 0)
                *mode = 0;
            break;
        case 5:
            fld_sctab_open();
            r = fld_npc_get_c0(s->pad8c[0]);
            if (r != 0 && r != -1)
                fld_voice_play(r);
            *mode = 6;
            break;
        case 6:
            fld_sctab_update(mode, arg, &s->save[1]);
            if (*mode == 0)
                fld_voice_play(-1);
            break;
        case 7:
            fbag_init();
            *mode = 8;
            break;
        case 8:
            switch (fbag_update(&s->save[1])) {
            case 1:
                *mode = 0;
                break;
            case 2:
                *arg = 3;
                *mode = 23;
                break;
            case 3:
                *mode = 23;
                *arg = 4;
                break;
            }
            break;
        case 9:
            d = fmsg_def();
            fmsg_set_input(1);
            d->pad_l = 4;
            d->pad_t = 2;
            switch (*arg) {
            case 2:
                fmsg_set_text(table_get_entry(fld_gim_get(5), 12), 0);
                break;
            case 3:
                fmsg_set_text(table_get_entry(fld_gim_get(5), 13), 0);
                break;
            case 4:
                fmsg_set_text(table_get_entry(fld_gim_get(5), 13), 0);
                break;
            case 5:
                fmsg_set_text(table_get_entry(fld_gim_get(5), 14), 0);
                break;
            }
            *mode = 10;
            break;
        case 10:
            if (fmsg_done() != 0) {
                fmsg_close();
                *mode = 0;
            }
            break;
        case 11:
            fcal_open(fld_heap_get(), *s->file, 0);
            fld_hud_hide();
            *mode = 12;
            break;
        case 12:
            s->xa8 = 0;
            if (fld_hud_hidden() != 0) {
                switch (fcal_update()) {
                case 1:
                    fld_hud_show();
                    *mode = 0;
                    break;
                case 3:
                    *mode = 23;
                    *arg = 8;
                    s->xa8 = 1;
                    break;
                case 4:
                    *mode = 23;
                    *arg = 9;
                    s->xa8 = 1;
                    break;
                case 5:
                    *mode = 23;
                    *arg = 20;
                    break;
                }
            }
            break;
        case 13:
            fget_item_open(s->pad8c[1]);
            ((void (*)(int, int, int, int, int))s->story.w[10])(0, s->pad8c[1], 1, s->pad8c[2], s->pad8c[3]);
            *mode = 14;
            break;
        case 14:
            fget_item_close(mode);
            break;
        case 15:
            fget_card_open(s->pad8c[4], 1, 1);
            ((void (*)(int, int, int, int, int))s->story.w[10])(1, s->pad8c[4], 1, s->pad8c[5], s->pad8c[6]);
            *mode = 16;
            break;
        case 16:
            fget_card_close(mode);
            break;
        case 17:
            fclk_open();
            *mode = 18;
            break;
        case 18:
            r = fclk_update();
            if (r == 0) {
                *mode = 0;
            } else if (r != -1) {
                *mode = 23;
                switch (r) {
                case 1:
                    *arg = 10;
                    break;
                case 2:
                    *arg = 11;
                    break;
                case 3:
                    *arg = 12;
                    break;
                }
                fclk_get(&s->save[2], &s->save[3]);
            }
            break;
        case 19:
            fcho_open();
            *mode = 20;
            break;
        case 20:
            r = fcho_update();
            if (r == 0) {
                *mode = 0;
            } else if (r != -1) {
                *mode = 23;
                switch (r) {
                case 1:
                default:
                    *arg = 15;
                    break;
                case 2:
                    *arg = 16;
                    break;
                case 3:
                    *arg = 17;
                    break;
                case 4:
                    *arg = 18;
                    break;
                case 5:
                    *arg = 19;
                    break;
                }
            }
            break;
        case 23:
            switch (*arg) {
            case 2:
            case 5:
            case 3:
            case 4:
                fld_npc_turn_to(s->save[0], (FieldNpc *)pl);
                s->saving = 0;
                break;
            default:
                s->saving = 0;
                break;
            }
            break;
        }
        if (*mode == 23 && s->saving == 1 && (s->save[0] = s->pad8c[0]) == -1) {
            switch (*arg) {
            case 2:
            case 3:
            case 4:
            case 5:
                *mode = 9;
                break;
            }
        }
    } else if (s->x18 != 0) {
        switch (*mode) {
        case 21:
            fegg_init(s->x38, s->x3c);
            *mode = 22;
            break;
        case 22:
            r = fegg_update();
            if (r != -1) {
                ((void (*)(int, int))s->story.w[11])(r, 1);
                s->x40 = r;
                s->x18 = 0;
            }
            break;
        }
    }
    fld_scene_frame();
    if (fld_player_loaded() != 0)
        s->x04 = 0;
    if (fpt_ready() != 0)
        s->x08 = 0;
}
#endif

/* draws the characters and the HUD, then the menu of the scene mode (x88), and the calendar over it */
void fld_scene_draw(void)
{
    FieldScene *const s = &field_scene;
    void *list;

    fld_player_draw();
    fpt_draw();
    fld_hud_draw();
    fget_draw();
    fld_sctab_draw();
    switch (field_scene.x88) {
    case 1:
        if (pad_get()->b.down & 0x200) {
            list = dl_select_layer(4, 0);
            list = fdlg_draw_buttons(list, s->padac[2]);
            dl_set_list(list);
        }
        break;
    case 3:
        list = dl_select_layer(3, 0);
        list = fld_dlg_draw(list);
        dl_set_list(list);
        break;
    case 8:
        list = dl_select_layer(3, 0);
        list = fbag_draw(list);
        dl_set_list(list);
        break;
    case 12:
        if (s->xa8 == 0) {
            list = dl_select_layer(3, 0);
            list = fld_dl_clear(list, 0xFFFF, 0x80000000);
            dl_set_list(list);
            fcal_draw();
            break;
        }
    case 18:
        list = dl_select_layer(3, 0);
        list = fclk_draw(list);
        dl_set_list(list);
        break;
    case 22:
        fegg_draw();
        break;
    case 20:
        list = dl_select_layer(3, 0);
        list = fcho_draw(list);
        dl_set_list(list);
        break;
    }
    if (s->xa8 != 0) {
        list = dl_select_layer(3, 0);
        list = fld_dl_clear(list, 0xFFFF, 0x80000000);
        dl_set_list(list);
        fcal_draw();
    }
}

FieldStory *fld_story_get(void)
{
    return &field_scene.story;
}

void fld_cmd_scene(FieldStory *s)
{
    fld_ctx_get();
    field_scene.loading = 1;
    field_scene.story = *s;
    memcpy(field_story_ext, &s->w[12], 0x10);
    fld_load_gim(0, fld_scene_gim_loaded, 0);
}

int fld_cmd_scene_done(void)
{
    return !field_scene.loading;
}

void fld_cmd_player(float *a)
{
    field_scene.x04 = 1;
    fld_player_setup(a[0], a[1], ((int *)a)[2]);
}

int fld_cmd_player_done(void)
{
    return !field_scene.x04;
}

void fld_cmd_target(int *a)
{
    FieldScene *s = &field_scene;

    if (a[0] == -1) {
        fpt_free();
        s->x08 = 0;
    } else {
        s->x08 = 1;
        fpt_setup(a[0], a[1]);
    }
}

int fld_cmd_target_done(void)
{
    return !field_scene.x08;
}

void fld_cmd_save_begin(void)
{
    FieldScene *s = &field_scene;

    field_scene.saving = 1;
    switch (field_scene.story.w[9]) {
    case 0:
        s->x88 = 0;
        break;
    case 1:
        fld_hud_hide();
        s->story.w[9] = 0;
        s->x88 = 12;
        break;
    }
}

int fld_cmd_save(FieldSave *o)
{
    FieldScene *s = &field_scene;
    FieldChr *p;
    FieldNpc *n[32];
    int cnt;
    int i;

    if (field_scene.saving == 0) {
        p = fld_player_get();
        o->bg = s->bg;
        o->x = p->pos.x;
        o->z = p->pos.z;
        o->dir = p->dir;
        o->save[0] = s->save[0];
        o->save[1] = s->save[1];
        o->save[2] = s->save[2];
        o->save[3] = s->save[3];
        o->save[4] = s->save[4];
        fld_npc_all(n, &cnt);
        for (i = 0; i < cnt; i++) {
            o->npc[i].id = n[i]->m.c.kind;
            o->npc[i].cx = n[i]->m.c.pos.x / 0.5f;
            o->npc[i].cz = n[i]->m.c.pos.z / 0.5f;
            o->npc[i].dir = n[i]->m.c.dir;
        }
        o->n_npc = cnt;
        return 1;
    }
    return 0;
}

void fld_cmd_door(void)
{
    field_scene.x10 = 1;
    fld_story_flag(0x6261, 0);
}

int fld_cmd_door_done(void)
{
    return !field_scene.x10;
}

void fld_cmd_wait(int *a)
{
    fld_hud_move(*a, 250, 120);
    field_scene.x14 = 1;
}

int fld_cmd_wait_done(void)
{
    return !field_scene.x14;
}

void fld_cmd_sc18(int *a)
{
    field_scene.x18 = 1;
    field_scene.x38 = a[0];
    field_scene.x3c = a[1];
    field_scene.x88 = 0x15;
}

int fld_cmd_sc18_done(int *out)
{
    *out = field_scene.x40;
    return !field_scene.x18;
}

void fld_cmd_x1c(int *a)
{
    field_scene.x1c = 1;
    fget_card_open(*a, 0, 0);
}

int fld_cmd_x1c_done(void)
{
    return !field_scene.x1c;
}

int fdlg_button(PadState *pad, unsigned int mask);
int fld_bg_event(int bg, int x, int z);
void fld_player_stop(void);
int fld_story_flag(int id, int arg);
int fld_cell_marked(int cx, int cz);
int fld_cell_door(int cx, int cz);
int fld_item_take_a(int cx, int cz, int take);
int fld_item_take_b(int cx, int cz, int take);
unsigned short region_pick_value(int flag);
int fld_bg_id(void);

#ifdef NON_MATCHING /* C novo, conferido com o asm. 2 palavras: a constante 0.5 das divisões sai em v0 no original, v1 aqui */
/* input of the field: the menu buttons (with the button bar held), talking/examining the cell,
   opening the menu or the map, then the scene events of the cell the player stands on (marked cell,
   door, items); *cmd/*arg get the scene command to run */
void fld_scene_input(int *cmd, int *arg)
{
    FieldScene *s = &field_scene;
    PadState *pad = pad_get();
    FieldChr *pl = fld_player_get();
    int cx = pl->pos.x / 0.5f;
    int cz = pl->pos.z / 0.5f;
    unsigned int trig;
    int k;

    if (!(pad->b.down & 0x200)) {
        if (pad->b.trig & 0x1000) {
            if (s->pad8c[0] != -1) {
                *cmd = 23;
                *arg = 2;
            } else {
                switch (fld_bg_event(fld_bg_id(), cx, cz)) {
                case 0:
                    break;
                case 1:
                    *cmd = 17;
                    break;
                case 2:
                    *cmd = 19;
                    break;
                case 3:
                    *cmd = 23;
                    *arg = 13;
                    break;
                case 4:
                    *cmd = 23;
                    *arg = 14;
                    break;
                }
            }
        }
        trig = pad->b.trig;
        if (trig & region_pick_value(0)) {
            *cmd = 5;
            fld_player_stop();
            fld_story_flag(0x6216, 0);
        } else if (trig & 0x8000) {
            *cmd = 11;
            fld_player_stop();
        }
    } else {
        switch (fdlg_button(pad, s->padac[2])) {
        case -1:
            break;
        case 0:
            if (s->pad8c[0] != -1) {
                *cmd = 23;
                *arg = 2;
            }
            break;
        case 1:
            if (s->pad8c[0] != -1) {
                *cmd = 23;
                *arg = 5;
            }
            break;
        case 2:
            *cmd = 7;
            break;
        case 3:
            *cmd = 2;
            break;
        }
    }
    if (fld_cell_marked(cx, cz)) {
        *cmd = 23;
        *arg = 0;
    } else {
        s->padac[0] = cx;
        s->padac[1] = cz;
    }
    k = fld_cell_door(cx, cz);
    if (k != -1) {
        switch (fld_bg_id()) {
        case 12:
        case 37:
            s->save[4] = k;
            *cmd = 23;
            *arg = 21;
            break;
        case 36:
            *cmd = 23;
            *arg = 22;
            break;
        }
    }
    k = fld_item_take_a(cx, cz, 1);
    if (k > 0) {
        s->pad8c[1] = k;
        s->pad8c[2] = cx;
        s->pad8c[3] = cz;
        *cmd = 13;
        fld_player_stop();
    }
    k = fld_item_take_b(cx, cz, 1);
    if (k > 0) {
        s->pad8c[4] = k;
        s->pad8c[5] = cx;
        s->pad8c[6] = cz;
        *cmd = 15;
        fld_player_stop();
    }
}
#endif

void fld_player_set_lock(int lock);
void fld_player_update(void);
void fpt_update(void);
int fld_sctab_state(void);
void fld_npc_get_d0(int id, int *a, int *b);
int charmap_lookup(int ch, int *kind_out);
unsigned short sysstate2_rec_get_u16(int which, int i);
int deck_slot_get_level(int id);
void fld_hud_box_open(unsigned short *title, int a, int b, int c, int d, int e);
void fld_hud_box_close(void);
int fld_hud_flash(void);
int fld_hud_move_done(void);
void fld_hud_update(void);
void fget_update(void);
int fget_done(void);

#ifdef NON_MATCHING /* C novo, conferido com o asm. 14 palavras: o original monta a máscara de botões em tempo de execução (move 0; ori 1; ori 2; ori 4) e o mwcc dobra para li 7/li 15 (tipos menores e bitfields não reproduzem) */
/* per frame: the player and the partner, the info box of the character in front (its record and
   deck, nothing for the shop keepers and the like), the HUD and the get window; then the buttons
   allowed in the button bar (8 only while ctx->x14 is clear) */
void fld_scene_frame(void)
{
    FieldScene *s = &field_scene;
    FieldCtx *ctx = fld_ctx_get();
    int kind;
    int title;
    int b;
    int r;
    int w0;
    int w1;
    int w2;
    int deck;
    unsigned int mask;

    if (field_scene.x88 != 1)
        fld_player_set_lock(0);
    fld_player_update();
    fpt_update();
    if (s->saving != 0 && s->pad8c[0] != -1 && fld_sctab_state() != 2 && s->x88 != 8 && s->x88 != 14 &&
        s->x88 != 16 && s->x88 != 18) {
        kind = 0;
        title = 0;
        b = 0;
        fld_npc_get_d0(s->pad8c[0], &title, &b);
        r = charmap_lookup(s->pad8c[0], &kind);
        switch (s->pad8c[0]) {
        case 20:
        case 23:
        case 29:
        case 34:
        case 39:
        case 143:
        case 144:
        case 145:
        case 146:
        case 147:
        case 148:
        case 149:
        case 150:
        case 151:
            b = 0;
            w0 = -1;
            w1 = w0;
            w2 = w0;
            deck = w0;
            break;
        default:
            if (r < 0 || kind != 0) {
                w0 = sysstate2_rec_get_u16(0, s->pad8c[0]);
                w1 = sysstate2_rec_get_u16(1, s->pad8c[0]);
                w2 = sysstate2_rec_get_u16(2, s->pad8c[0]);
                deck = deck_slot_get_level(s->pad8c[0]);
            } else {
                b = 0;
                w0 = -1;
                w1 = w0;
                w2 = w0;
                deck = w0;
            }
            break;
        }
        fld_hud_box_open((unsigned short *)title, b, w0, w1, w2, deck);
    } else {
        fld_hud_box_close();
    }
    if (s->x10 != 0 && fld_hud_flash() == 0)
        s->x10 = 0;
    if (s->x14 != 0 && fld_hud_move_done() != 0)
        s->x14 = 0;
    fld_hud_update();
    fget_update();
    if (fget_done())
        s->x1c = 0;
    if (s->x88 == 1)
        ctx->pad04 = 1;
    else
        ctx->pad04 = 0;
    mask = 0;
    mask |= 1;
    mask |= 2;
    mask |= 4;
    s->padac[2] = mask;
    if (ctx->x14 == 0) {
        mask |= 8;
        s->padac[2] = mask;
    }
}
#endif

void fld_scene_gim_loaded(int *file, void *arg)
{
    FieldScene *s = &field_scene;
    void *g;

    field_scene.file = file;
    if (file) {
        g = fld_gim_get(0);
        fmoney_set_gim(fld_gim_get(2));
        fmsg_set_cursor(g);
        fget_load();
        if (s->story.w[9] == 1) {
            fcal_open(fld_heap_get(), *(int *)s->file, 1);
            s->xa8 = 1;
        } else {
            s->xa8 = 0;
        }
        s->loading = 0;
    }
}

void fld_scene_tab_init(void)
{
    memset(&field_scene_tab, 0, sizeof(FieldScTab));
}

void fld_scene_tab_free(void)
{
    memset(&field_scene_tab, 0, sizeof(FieldScTab));
}
