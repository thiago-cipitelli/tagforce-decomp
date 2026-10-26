// @range 0x0880F228-0x0880FBB4
// @data 0x0883C630-0x0883C640
/* Sound effects of the field and the objects lying on cells (two lists of 8): placing them, their
   shadows and cell marks, picking one up, and drawing them as sprites of the shared GIM. */
#include "libfield.h"

void fld_items_free(void);
int fld_item_free_a(void);
int fld_item_free_b(void);
void fld_items_loaded(int *file, int *flag);
void *fld_item_draw_a(void *list, FieldItem *it);
void *fld_item_draw_b(void *list, FieldItem *it);
void fld_item_quad(FieldVtx *q, int id);
void *fld_item_draw(void *list, FieldItem *it, FieldVtx *q);

extern int field_item_uv[][2];                 /* 0x0883860C */
extern int field_item_tab[][2];                /* 0x088386B4 */

Vec4 field_item_ofs = { 0.21f, 0.0f, 0.21f, 0.0f };

int fld_story_flag(int id, int mode)
{
    int ok;
    int r = 0;

    if (field_ctx.se_on == 0)
        return 0;
    switch (mode) {
    case 1:
        if (snd_count_se())
            ok = 0;
        else
            ok = 1;
        break;
    case 2:
        snd_cmd_fef9();
    default:
        ok = 1;
        break;
    }
    if (ok)
        r = se_play(id);
    return r;
}

void fld_bgm_volume(int a, int b)
{
    bgm_set_volume(a, b, 1500);
}

void fld_items_init(void)
{
    memset(&field_items, 0, sizeof(FieldItems));
}

void fld_items_term(void)
{
    fld_items_free();
    fld_items_init();
}

void fld_items_free(void)
{
    memset(&field_items, 0, sizeof(FieldItems));
}

void fld_items_update(void)
{
    FieldItems *b = &field_items;
    int i;
    FieldItem *it;

    if (fld_map_get()) {
        for (i = 0; i < 8; i++) {
            it = &b->a[i];
            if (b->a[i].active) {
                fld_shadow_set(&it->pos, &it->mtx);
                fld_cell_mark(it->pos.x / 0.5f, it->pos.z / 0.5f, 4);
            }
        }
        for (i = 0; i < 8; i++) {
            it = &b->b[i];
            if (b->b[i].active) {
                fld_shadow_set(&it->pos, &it->mtx);
                fld_cell_mark(it->pos.x / 0.5f, it->pos.z / 0.5f, 4);
            }
        }
    }
}

void fld_items_draw(void)
{
    FieldItems *b = &field_items;
    int i;
    FieldItem *it;

    if (field_items.file && *b->file) {
        for (i = 0; i < 8; i++) {
            it = &b->a[i];
            if (b->a[i].active)
                fld_draw_add(1.0f, &it->mtx, 0, 1, fld_item_draw_a, it);
        }
        for (i = 0; i < 8; i++) {
            it = &b->b[i];
            if (b->b[i].active)
                fld_draw_add(1.0f, &it->mtx, 0, 1, fld_item_draw_b, it);
        }
    }
}

/* id of the item on the cell (taken away when take is set), -1 if none */
int fld_item_take_a(int cx, int cz, int take)
{
    FieldItems *b = &field_items;
    int i;
    FieldItem *it;

    for (i = 0; i < 8; i++) {
        it = &b->a[i];
        if (b->a[i].active && (int)(it->pos.x / 0.5f) == cx && (int)(it->pos.z / 0.5f) == cz) {
            if (take) {
                it->active = 0;
                fld_cell_mark(it->pos.x / 0.5f, it->pos.z / 0.5f, 0);
            }
            return it->id;
        }
    }
    return -1;
}

int fld_item_take_b(int cx, int cz, int take)
{
    FieldItems *b = &field_items;
    int i;
    FieldItem *it;

    for (i = 0; i < 8; i++) {
        it = &b->b[i];
        if (b->b[i].active && (int)(it->pos.x / 0.5f) == cx && (int)(it->pos.z / 0.5f) == cz) {
            if (take) {
                it->active = 0;
                fld_cell_mark(it->pos.x / 0.5f, it->pos.z / 0.5f, 0);
            }
            return it->id;
        }
    }
    return -1;
}

int *fld_item_info(int i)
{
    return field_item_tab[i];
}

void fld_items_cmd_a(FieldList *l)
{
    FieldItems *b = &field_items;
    int i;
    FieldItemCmd *e;
    FieldItem *it;
    int k;

    field_items.loading_a = 1;
    for (i = 0; i < l->n; i++) {
        e = &((FieldItemCmd *)l->items)[i];
        k = fld_item_free_a();
        if (k != -1)
            it = &b->a[k];
        it->active = 1;
        it->id = e->id;
        fld_pos_set(&it->pos, e->x, e->z);
    }
    fld_load_gim(0, (void (*)(int *, void *))fld_items_loaded, b);
}

int fld_items_done_a(void)
{
    return !field_items.loading_a;
}

void fld_items_cmd_b(FieldList *l)
{
    FieldItems *b = &field_items;
    int i;
    FieldItemCmd *e;
    FieldItem *it;
    int k;

    field_items.loading_b = 1;
    for (i = 0; i < l->n; i++) {
        e = &((FieldItemCmd *)l->items)[i];
        k = fld_item_free_b();
        if (k != -1)
            it = &b->b[k];
        it->active = 1;
        it->id = e->id;
        fld_pos_set(&it->pos, e->x, e->z);
    }
    fld_load_gim(0, (void (*)(int *, void *))fld_items_loaded, &b->loading_b);
}

int fld_items_done_b(void)
{
    return !field_items.loading_b;
}

void *fld_item_draw_a(void *list, FieldItem *it)
{
    FieldVtx *q;

    list = gim_set_texture_frame(list, field_items.gim, 0);
    q = dl_alloc_inline(&list, 0x60);
    fld_item_quad(q, it->id);
    return list = fld_item_draw(list, it, q);
}

void *fld_item_draw_b(void *list, FieldItem *it)
{
    FieldVtx *q;

    list = gim_set_texture_frame(list, field_items.gim, 0);
    q = dl_alloc_inline(&list, 0x60);
    fld_item_quad(q, 1);
    return list = fld_item_draw(list, it, q);
}

void *fld_item_draw(void *list, FieldItem *it, FieldVtx *q)
{
    Vec4 t;
    Mat4 m;
    Mat4 a;

    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    t.x = field_item_ofs.x;
    t.y = field_item_ofs.y;
    t.z = field_item_ofs.z;
    t.w = field_item_ofs.w;
    mat4_identity(&a);
    mat4_translate(&a, &a, &t);
    mat4_mul(&m, &a, &it->mtx);
    mtx_load(&m);
    list = draw_array(list, 4, 0x19F, 4, 0, q);
    mtx_pop();
    return list;
}

void fld_item_quad(FieldVtx *q, int id)
{
    int *e = field_item_uv[id];
    int u = e[0];
    int v = e[1];
    float u1 = (u + 24) / 256.0f;
    float v0 = v / 256.0f;
    float u0;
    float v1;

    q[0].u = u1;
    q[0].v = v0;
    q[0].x = 0.192187488f;
    q[0].y = 0.46125f;
    u0 = u / 256.0f;
    q[1].u = u0;
    q[1].v = v0;
    q[1].x = -0.1875f;
    q[1].y = 0.46125f;
    q[2].u = u1;
    v1 = (v + 24) / 256.0f;
    q[2].v = v1;
    q[2].x = 0.192187488f;
    q[2].y = 0.0f;
    q[3].u = u0;
    q[3].v = v1;
    q[3].x = -0.1875f;
    q[3].y = 0.0f;
    q[3].color = 0xFFFFFFFF;
    q[2].color = 0xFFFFFFFF;
    q[1].color = 0xFFFFFFFF;
    q[0].color = 0xFFFFFFFF;
    q[3].z = 0.0f;
    q[2].z = 0.0f;
    q[1].z = 0.0f;
    q[0].z = 0.0f;
}

int fld_item_free_a(void)
{
    FieldItems *b = &field_items;
    int i;

    for (i = 0; i < 8; i++) {
        if (b->a[i].active == 0)
            return i;
    }
    return -1;
}

int fld_item_free_b(void)
{
    FieldItems *b = &field_items;
    int i;

    for (i = 0; i < 8; i++) {
        if (b->b[i].active == 0)
            return i;
    }
    return -1;
}

void fld_items_loaded(int *file, int *flag)
{
    FieldItems *b = &field_items;

    field_items.file = file;
    if (file) {
        b->gim = fld_gim_get(3);
        *flag = 0;
    }
}
