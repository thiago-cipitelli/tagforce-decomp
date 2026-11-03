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
