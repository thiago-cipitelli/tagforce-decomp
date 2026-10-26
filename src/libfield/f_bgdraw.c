// @range 0x08808214-0x08809C40
// @asm 0x08808214-0x08808770
// @asm 0x08808874-0x08808B8C
// @asm 0x08808B8C-0x08808E38
// @asm 0x08808E38-0x08809034
// @asm 0x088090E8-0x088091DC
// @asm 0x088092E0-0x088094B4
// @asm 0x088094B4-0x08809630
// @asm 0x08809700-0x088099E0
// @asm 0x08809A10-0x08809B40
/* Room loading and drawing: the models of each area and their stencil shadows, the placed objects,
   the camera of the room, the mesh mask by area and the hook that hides the door meshes ("_DD."). */
#include "libfield.h"

int fld_story_flag(int id, int arg);           /* 0x0880F228 */
int fld_bg_hook(void **list, void *m, int a, int b, int phase);

extern const char str_DD[];                    /* "_DD." */
extern const char str_Dd[];                    /* "_Dd." */
extern const char str_dD[];                    /* "_dD." */
extern const char str_dd[];                    /* "_dd." */

/* shows meshes 0..4, 5..9 or 10..14 only for the area */
void fld_bg_mask_area(void *m, int area)
{
    unsigned int mask = 0;
    int i;
    int b;

    for (i = 0; i < 32; i++) {
        b = 1;
        switch (area) {
        default:
            if (i >= 0 && i < 5)
                b = 0;
            break;
        case 1:
            if (i >= 5 && i < 10)
                b = 0;
            break;
        case 2:
            if (i >= 10 && i < 15)
                b = 0;
            break;
        }
        mask |= b << i;
    }
    model_set_mask(m, mask);
}

/* area of a cell: the hostel outside has two (x < 15 and the rest) */
int fld_area_at(int cx, int cz)
{
    int r;

    switch (field_bg_state.id) {
    case 3:
        if (cx > 14)
            r = 0;
        else
            r = 1;
        break;
    default:
        r = -1;
        break;
    }
    return r;
}

void fld_obj_init(FieldObj *o)
{
    memset(o, 0, sizeof(FieldObj));
    mat4_identity(&o->mtx);
    o->mode = 2;
    o->x54 = 0;
}

void fld_bg_draw_models(void)
{
    FieldBg *b = &field_bg_state;
    ModelDrawOpts o;
    int i;

    memset(&o, 0, sizeof(o));
    o.a = 1;
    o.b = 2;
    if (field_bg_state.area == -1) {
        for (i = 0; i < b->n_models; i++)
            model_draw(b->model[i], &o);
    } else {
        model_draw(b->model[b->area], &o);
    }
}

#ifdef NON_MATCHING /* quadro com 12 bytes a mais entre a matriz e as opções, e n_objs lido absoluto no original */
void fld_bg_draw_objs(void)
{
    FieldBg *b = &field_bg_state;
    ModelDrawOpts o;
    Mat4 m;
    Mat4 t;
    int i;
    FieldObj *p;

    memset(&o, 0, sizeof(o));
    o.a = 1;
    o.b = 2;
    for (i = 0; i < field_bg_state.n_objs; i++) {
        p = &b->obj[i];
        if (b->area == -1 || p->info[2] == b->area) {
            mat4_identity(&t);
            t.m[14] = -0.01f;
            mat4_mul(&m, &p->mtx, &t);
            mtx_set_mode(2);
            mtx_push();
            mtx_load(&m);
            model_draw(p->model, &o);
            mtx_pop();
        }
    }
}
#endif

void fld_bg_draw_shadow(void)
{
    FieldBg *b = &field_bg_state;
    void *dl = dl_select_layer(1, 0);
    ModelDrawOpts o;
    int i;

    dl = dl_depth_mask(dl, 1);
    dl = dl_stencil_op(dl, 1, 0, 0xFF);
    dl = dl_stencil_func(dl, 0, 0, 2);
    dl_set_list(dl);
    if (field_bg_state.model2[0]) {
        memset(&o, 0, sizeof(o));
        o.a = 1;
        o.b = 2;
        if (b->area == -1) {
            for (i = 0; i < b->n_models; i++)
                model_draw(b->model2[i], &o);
        } else {
            model_draw(b->model2[b->area], &o);
        }
    }
}

int fld_bg_stencil_hook(void **list, void *m, int a, int b, int phase)
{
    switch (phase) {
    case 0:
        *list = dl_pixel_mask(*list, 0xFFFFFF);
        *list = dl_depth_mask(*list, 1);
        *list = dl_enable(*list, 1);
        *list = dl_enable(*list, 3);
        *list = dl_stencil_op(*list, 1, 1, 0xFF);
        *list = dl_stencil_func(*list, 0, 0, 2);
        break;
    case 1:
        *list = dl_pixel_mask(*list, 0);
        *list = dl_depth_mask(*list, 0);
        break;
    }
    return 0;
}

void fld_bg_set_hook(void)
{
    model_hook(field_bg_state.model[0], 1, fld_bg_hook);
}

void fld_bg_model_update(int id)
{
}

void fld_bg_model_draw(int id)
{
}

void fld_bg_model_free(int id)
{
}

#ifdef NON_MATCHING /* forma igual; registradores s0-s3 trocados */
/* hides (phase 0) and shows again (phase 1) the door meshes */
int fld_bg_hook(void **list, void *m, int a, int b, int phase)
{
    char **name = *(char ***)(*(int *)((char *)m + 0xC) + a * 0xA4 + b * 0x20 + 0x1C);
    int hit = 0;

    if (name != 0 && (int)strlen(*name) >= 7) {
        if (strstr(*name, str_DD) || strstr(*name, str_Dd) || strstr(*name, str_dD) || strstr(*name, str_dd))
            hit = 1;
    }
    switch (phase) {
    case 0:
        if (hit)
            *list = dl_disable(*list, 1);
        break;
    case 1:
        if (hit)
            *list = dl_enable(*list, 1);
        break;
    }
    return 0;
}
#endif

/* sound of the door being opened (open = 1) or closed */
void fld_bg_door_se(int open, int side)
{
    FieldBg *b = &field_bg_state;
    int se = -1;

    if (open != 1) {
        switch (b->id) {
        case 2:
        case 3:
            se = 0x625F;
            break;
        case 0xC:
        case 0x25:
            switch (side) {
            case 0:
                se = 0x625D;
                break;
            case 1:
                se = 0x625B;
                break;
            }
            break;
        }
    } else {
        switch (b->id) {
        case 2:
        case 3:
            se = 0x6260;
            break;
        case 0xC:
        case 0x25:
            switch (side) {
            case 0:
                se = 0x625E;
                break;
            case 1:
                se = 0x625C;
                break;
            }
            break;
        }
    }
    if (se != -1)
        fld_story_flag(se, 0);
}
