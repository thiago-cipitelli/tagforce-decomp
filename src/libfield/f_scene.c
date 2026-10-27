// @range 0x08810814-0x08811BE0
// @asm 0x08810814-0x08810F44
// @asm 0x08810F44-0x08811130
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
void func_0882623C(int a, int b, int c);
void fmoney_set_gim(void *gim);
void func_08826074(void);
void func_0881EDC8(void *heap, int a, int b);
void fld_scene_gim_loaded(int *file, void *arg);

extern unsigned char field_story_ext[0x10];    /* 0x08FD5748 */

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
    func_0882623C(*a, 0, 0);
}

int fld_cmd_x1c_done(void)
{
    return !field_scene.x1c;
}

void fld_scene_gim_loaded(int *file, void *arg)
{
    FieldScene *s = &field_scene;
    void *g;

    field_scene.file = file;
    if (file) {
        g = fld_gim_get(0);
        fmoney_set_gim(fld_gim_get(2));
        fmsg_set_cursor(g);
        func_08826074();
        if (s->story.w[9] == 1) {
            func_0881EDC8(fld_heap_get(), *(int *)s->file, 1);
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
