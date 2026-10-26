// @range 0x08806FC4-0x08807844
// @asm 0x088071E8-0x08807660
/* The room (bg_NN_NN): its models, door models and marker cells, the area the player stands in
   (with a 30 frame crossfade when it changes) and the cell map used by the collision. */
#include "libfield.h"

void fld_bg_free(void);
void fld_bg_model_free(int id);                /* 0x08809A08 */
void fld_bg_model_update(int id);              /* 0x088099F8 */
FieldChr *fld_player_get(void);                /* 0x0880B170 */
void fld_pos_to_cell(float x, float z, int *cx, int *cz); /* 0x08807F24 */
int fld_area_at(int cx, int cz);               /* 0x08808804 */
void fld_bg_cam_update(void);                  /* 0x08808874 */
void fld_load_bg(int id, void (*cb)(), int arg); /* 0x0880C56C */
void fld_bg_loaded();                          /* 0x08808214 */
void *fld_story_get(void);                     /* 0x08811130 */
int fld_bg_index(int id);                      /* 0x0880EF2C */
int fld_time_index(int t);                     /* 0x0880CF48 */
void fld_light_set(int light);                 /* 0x08818884 */
void fld_load_cancel_bg(void);                 /* 0x0880C6B8 */

int fld_bg_init(void)
{
    memset(&field_bg_state, 0, sizeof(FieldBg));
    return 1;
}

void fld_bg_term(void)
{
    fld_bg_free();
    fld_bg_init();
}

void fld_bg_free(void)
{
    FieldBg *b = &field_bg_state;
    int i;

    fld_bg_model_free(field_bg_state.id);
    if (field_bg_state.cells) {
        fld_free(b->cells);
        b->cells = 0;
    }
    for (i = 0; i < b->n_models; i++) {
        model_free(b->model[i]);
        b->model[i] = 0;
        if (b->model2[i]) {
            model_free(b->model2[i]);
            b->model2[i] = 0;
        }
    }
    for (i = 0; i < b->n_doors; i++) {
        model_free(b->door[i]);
        b->door[i] = 0;
    }
    b->map = 0;
}

void fld_bg_update(void)
{
    FieldBg *b = &field_bg_state;
    int i;
    FieldChr *p;
    int cx;
    int cz;
    int a;

    for (i = 0; i < b->n_models; i++) {
        if (b->model[i])
            model_update(b->model[i], 0);
    }
    p = fld_player_get();
    if (p) {
        fld_pos_to_cell(p->pos.x, p->pos.z, &cx, &cz);
        a = fld_area_at(cx, cz);
        if (b->area != a) {
            b->prev_area = b->area;
            b->fade_t = 0;
            b->fade = 1;
            b->area = a;
        }
    }
    fld_bg_cam_update();
    if (b->fade == 1) {
        if (b->fade_t < 30)
            b->fade_t++;
        else
            b->fade = 0;
    }
    fld_bg_model_update(b->id);
}

int fld_bg_area_is(int cx, int cz)
{
    FieldBg *b = &field_bg_state;
    int a = fld_area_at(cx, cz);
    int r;

    if (b->area != -1) {
        if (a == b->area)
            r = 1;
        else
            r = 0;
    } else {
        r = 1;
    }
    return r;
}

void fld_bg_cmd_load(FieldBgCmd *c)
{
    FieldBg *b = &field_bg_state;
    unsigned int size;
    int *st;
    int bg;
    int t;

    field_bg_state.loading = 1;
    field_bg_state.id = c->id;
    if (c->n_cells) {
        size = c->n_cells << 2;
        b->cells = fld_alloc(size);
        memcpy(b->cells, c->cells, size);
    }
    b->n_cells = c->n_cells;
    fld_load_bg(c->id, fld_bg_loaded, 0);
    st = fld_story_get();
    bg = fld_bg_index(b->id);
    t = fld_time_index(*(int *)st[3]);
    fld_light_set(field_bg_lights[bg][t].light);
}

int fld_bg_load_done(void)
{
    return !field_bg_state.loading;
}

void fld_bg_cmd_unload(void)
{
    field_bg_state.unloading = 1;
    fld_load_cancel_bg();
    field_bg_state.unloading = 0;
}

int fld_bg_unload_done(void)
{
    return !field_bg_state.unloading;
}

FieldMap *fld_map_get(void)
{
    return field_bg_state.map;
}

unsigned char *fld_map_cells(void)
{
    FieldMap *m = fld_map_get();

    if (m != 0)
        return (unsigned char *)m + ((int *)m)[3];
    return 0;
}

unsigned char *fld_map_attrs(void)
{
    FieldMap *m = fld_map_get();

    if (m != 0)
        return (unsigned char *)m + ((int *)m)[4];
    return 0;
}
