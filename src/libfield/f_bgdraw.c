// @range 0x08808214-0x08809C40
// @asm 0x08808214-0x08808770
// @asm 0x08808874-0x08808B8C
// @asm 0x08808B8C-0x08808E38
// @asm 0x08808E38-0x08809034
// @asm 0x088092E0-0x088094B4
// @asm 0x08809700-0x088099E0
/* Room loading and drawing: the models of each area and their stencil shadows, the placed objects,
   the camera of the room, the mesh mask by area and the hook that hides the door meshes ("_DD."). */
#include "libfield.h"

int fld_story_flag(int id, int arg);           /* 0x0880F228 */
int fld_bg_hook(void **list, void *m, int a, int b, int phase);
void *dl_tex_flush(void *list);
void *gfx_set_draw_target(void *list, int base, int fbp, int w, int h, int psm);
void *gfx_set_default_target(void *list);
void *dl_offset(void *list, int x, int y);
void *dl_viewport(void *list, int cx, int cy, int w, int h);
void *dl_clear_color(void *list, unsigned int rgba);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear_depth(void *list, int z);
void *dl_clear(void *list, int flags);
FieldStory *fld_story_get(void);
int fld_bg_index(int id);
int fld_time_index(int t);
void fld_bg_door_se(int open, int side);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);

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
    o->open = 0;
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

void fld_bg_draw_objs(void)
{
    FieldBg *b = &field_bg_state;
    ModelDrawOpts o;
    Mat4 m __attribute__((aligned(16)));
    Mat4 t __attribute__((aligned(16)));
    int i;
    FieldObj *p;

    memset(&o, 0, sizeof(o));
    o.a = 1;
    o.b = 2;
    for (i = 0; i < b->n_objs; i++) {
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

#ifdef NON_MATCHING /* C novo, conferido com o asm. 1 palavra: o original faz mul.s f20,open,turn (operandos trocados; turn * open, *=, temporário e ordem das declarações não mudam) */
/* door of the room: opens/closes by 0.1 + 10% a frame, turned around y by info[9] turns when open,
   then placed at its position relative to field_bg_state.light */
void fld_bg_door_update(FieldObj *o)
{
    FieldBg *b = &field_bg_state;
    float turn = 6.2831855f * ((float *)o->info)[9];
    float t;
    Vec4 p;
    Vec4 r;

    switch (o->mode) {
    case 0:
        break;
    case 2:
        break;
    case 1:
        if (o->open >= 1.0f) {
            o->mode = 0;
        } else {
            t = o->open + (0.1f + 0.1f * o->open);
            o->open = t;
            if (t > 1.0f)
                o->open = 1.0f;
        }
        break;
    case 3:
        if (o->open <= 0.0f) {
            o->mode = 2;
            fld_bg_door_se(1, o->info[1]);
        } else {
            t = o->open - (0.1f + 0.1f * o->open);
            o->open = t;
            if (t < 0.0f)
                o->open = 0.0f;
        }
        break;
    }
    turn = turn * o->open;
    mat4_identity(&o->mtx);
    r.x = 3.1415927f * ((float *)o->info)[6] / 180.0f;
    r.y = 3.1415927f * ((float *)o->info)[7] / 180.0f + turn;
    r.z = 3.1415927f * ((float *)o->info)[8] / 180.0f;
    r.w = 0.0f;
    mat4_rotate_xyz(&o->mtx, &o->mtx, &r);
    p.x = ((float *)o->info)[3] + -1.0f * b->light.x;
    p.y = ((float *)o->info)[4] + -1.0f * b->light.y;
    p.z = ((float *)o->info)[5] + -1.0f * b->light.z;
    p.w = 0.0f;
    mat4_translate(&o->mtx, &o->mtx, &p);
}
#endif

#ifdef NON_MATCHING /* C novo, conferido com o asm. 26 palavras: o original carrega o 136 e o 272 junto do uso e põe as cores em v0/a0 (aqui as constantes sobem para o início) */
/* sky gradient behind the room: top, middle and bottom colour of the area at this time of day */
void *fld_bg_draw_sky(void *list)
{
    FieldSkyVtx *v = dl_alloc_inline(&list, sizeof(FieldSkyVtx) * 6);
    FieldStory *st = fld_story_get();
    int bg = fld_bg_index(field_bg_state.id);
    int t = fld_time_index(*(int *)st->w[3]);
    unsigned int *c = field_bg_sky[field_bg_lights[bg][t].sky];
    unsigned int top = c[0];
    unsigned int mid = c[1];
    unsigned int bot;

    v[1].color = top;
    v[1].x = 0;
    v[1].y = 0;
    v[1].z = 0;
    v[0].color = top;
    v[0].x = 480;
    v[0].y = 0;
    v[0].z = 0;
    v[3].color = mid;
    v[3].x = 0;
    v[3].y = 136;
    v[3].z = 0;
    v[2].color = mid;
    v[2].x = 480;
    v[2].y = 136;
    v[2].z = 0;
    bot = c[2];
    v[5].color = bot;
    v[5].x = 0;
    v[5].y = 272;
    v[5].z = 0;
    v[4].color = bot;
    v[4].x = 480;
    v[4].y = 272;
    v[4].z = 0;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_disable(list, 9);
    list = dl_enable(list, 3);
    list = dl_disable(list, 4);
    list = dl_disable(list, 1);
    list = dl_stencil_op(list, 2, 0, 0xFF);
    list = dl_stencil_func(list, 0, 0, 0);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    list = dl_draw_array(list, 4, 0x80011C, 6, 0, v);
    mtx_pop();
    return list;
}
#endif

/* draws the model of the previous area into the 256x256 texture at 0x164000, then restores the screen */
void fld_bg_draw_to_tex(void)
{
    ModelDrawOpts o;
    void *dl;

    memset(&o, 0, sizeof(o));
    o.a = 1;
    o.b = 1;
    dl = dl_select_layer(1, 0);
    dl = dl_tex_flush(dl);
    dl = gfx_set_draw_target(dl, 0x164000, -2, 0x100, 0x100, 3);
    dl = dl_offset(dl, 0x780, 0x780);
    dl = dl_viewport(dl, 0x800, 0x800, 0x100, 0x100);
    dl = dl_scissor(dl, 0, 0, 0x100, 0x100);
    dl = dl_enable(dl, 2);
    dl = dl_enable(dl, 1);
    dl = dl_clear_color(dl, 0);
    dl = dl_clear_stencil(dl, 0);
    dl = dl_clear_depth(dl, 0);
    dl = dl_clear(dl, 0x17);
    dl_set_list(dl);
    model_draw(field_bg_state.model[field_bg_state.prev_area], &o);
    dl = dl_select_layer(1, 0);
    dl = gfx_set_default_target(dl);
    dl = dl_offset(dl, 0x710, 0x778);
    dl = dl_viewport(dl, 0x800, 0x800, 480, 272);
    dl = dl_scissor(dl, 0, 0, 480, 272);
    dl = dl_enable(dl, 2);
    dl_set_list(dl);
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

/* hides (phase 0) and shows again (phase 1) the door meshes */
int fld_bg_hook(void **list, void *m, int a, int b, int phase)
{
    int hit = 0;
    char **name = *(char ***)(*(int *)((char *)m + 0xC) + a * 0xA4 + b * 0x20 + 0x1C);

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
