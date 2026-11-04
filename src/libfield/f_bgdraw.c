// @range 0x08808214-0x08809C40
// @asm 0x08808214-0x08808770
// @asm 0x088092E0-0x088094B4
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
void fld_bg_mask_area(void *m, int area);
void fld_bg_door_se(int open, int side);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);

extern const char str_DD[];                    /* "_DD." */
extern const char str_Dd[];                    /* "_Dd." */
extern const char str_dD[];                    /* "_dD." */
extern const char str_dd[];                    /* "_dd." */

typedef struct FieldBgAnim {            /* tma/tmt file of the room, by name */
    char name[32];
    void *data;
} FieldBgAnim;

typedef struct FieldDoorInfo {          /* placed object (door) of a room, 0x2C: FieldObj.info */
    char *name;                         /* +0x00 its .tms */
    int side;                           /* +0x04 1 hinged, 0 sliding */
    int area;                           /* +0x08 */
    float x, y, z;                      /* +0x0C */
    float rx, ry, rz;                   /* +0x18 degrees */
    float turn;                         /* +0x24 turns when open / slide axis */
    int kind;                           /* +0x28 */
} FieldDoorInfo;

extern char *field_bg_models[][2];             /* 0x0883A340 the two area models of each room */
extern FieldDoorInfo field_bg_doors[][10];     /* 0x08834E3C */
void *ehp_get_name_by_index(void *pack, int i, char *name);
char *fld_after_dot(char *s);
void *model_load(void *data, void **anims);
void *ehp_find(void *pack, const char *name, int flags);
void model_set_tgma(void *m, void *a);
void model_anim_set(void *m, void *data);
int model_anim_play(void *m, int n, unsigned int flags);
void fld_obj_init(FieldObj *o);
void fld_bg_set_hook(int id);
int fld_bg_index(int id);
int fld_time_index(int t);
void fld_bg_mask_area(void *m, int area);

#ifdef NON_MATCHING /* C novo, conferido com o asm (mesma sequência de chamadas). ~139 palavras: tamanho 1396 contra 1372 e outros slots de spill para i, pa, pt e os ponteiros das portas (ordem das declarações não muda) */
/* the room pack is loaded: its area models (and their _mask stencil models), the doors with the
   placed objects that use them, the tma/tmt animations and the height map (made relative to its
   first vertex, which becomes the light offset); then the meshes of the time of day */
void fld_bg_loaded(int *file)
{
    FieldBg *b = &field_bg_state;
    FieldStory *st = fld_story_get();
    int ntma = 0;
    int ntmt = 0;
    void *pack;
    int nmodels;
    int ndoors;
    FieldBgAnim *pa;
    FieldBgAnim *pt;
    int nobj;
    int i;
    int j;
    int k;
    void *data;
    char *ext;
    char *p;
    FieldDoorInfo *e;
    Vec3 *v;
    FieldBgAnim tmas[2];
    FieldBgAnim tmts[2];
    char name[64];
    char mask[32];
    char tma[32];
    char tmt[32];
    Vec3 l;

    field_bg_state.file = file;
    if (file == 0)
        return;
    pack = (void *)*file;
    nmodels = 0;
    ndoors = 0;
    pa = tmas;
    pt = tmts;
    nobj = 0;
    for (i = 0;; i++) {
        data = ehp_get_name_by_index(pack, i, name);
        if (data == 0)
            break;
        ext = fld_after_dot(name);
        if (strcmp(ext, "tms") == 0) {
            if (strstr(name, "door") != 0) {
                b->door[ndoors] = model_load(data, 0);
                for (j = 0; j < 10; j++) {
                    e = &field_bg_doors[fld_bg_index(b->id)][j];
                    if (e->name != 0 && strcmp(e->name, name) == 0) {
                        fld_obj_init(&b->obj[nobj]);
                        b->obj[nobj].info = (int *)e;
                        b->obj[nobj].model = b->door[ndoors];
                        nobj++;
                    }
                }
                ndoors++;
            } else {
                for (k = 0; k < 2; k++) {
                    p = field_bg_models[fld_bg_index(b->id)][k];
                    if (p != 0 && strcmp(name, p) == 0) {
                        b->model[k] = model_load(data, 0);
                        nmodels++;
                        strcpy(mask, name);
                        p = strstr(mask, ".tms");
                        if (p != 0) {
                            strcpy(p, "_mask.tms");
                            data = ehp_find(pack, mask, 0);
                            if (data != 0)
                                b->model2[k] = model_load(data, 0);
                        }
                        break;
                    }
                }
            }
        } else if (strcmp(ext, "tma") == 0) {
            strcpy(pa->name, name);
            ntma++;
            pa->data = data;
            pa++;
        } else if (strcmp(ext, "tmt") == 0) {
            strcpy(pt->name, name);
            ntmt++;
            pt->data = data;
            pt++;
        } else if (strcmp(ext, "hmp") == 0) {
            b->map = data;
        }
    }
    b->n_objs = nobj;
    b->n_doors = ndoors;
    b->n_models = nmodels;
    for (k = 0; k < b->n_models; k++) {
        p = field_bg_models[fld_bg_index(b->id)][k];
        strcpy(tma, p);
        strcpy(fld_after_dot(tma), "tma");
        strcpy(tmt, p);
        strcpy(fld_after_dot(tmt), "tmt");
        for (j = 0; j < ntma; j++) {
            if (strcmp(tma, tmas[j].name) == 0) {
                model_set_tgma(b->model[k], tmas[j].data);
                break;
            }
        }
        for (j = 0; j < ntmt; j++) {
            if (strcmp(tmt, tmts[j].name) == 0) {
                model_anim_set(b->model[k], tmts[j].data);
                model_anim_play(b->model[k], 0, 2);
                break;
            }
        }
    }
    v = (Vec3 *)((char *)b->map + ((int *)b->map)[3]);
    l = v[0];
    b->light = l;
    for (i = 0; i < b->map->n_vert; i++)
        vec3_sub(&v[i], &v[i], &l);
    if (st != 0) {
        k = fld_time_index(*(int *)st->w[3]);
        for (i = 0; i < b->n_models; i++)
            fld_bg_mask_area(b->model[i], k);
    }
    fld_bg_set_hook(b->id);
    b->loading = 0;
}
#endif

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

int fld_bg_id(void);
void fld_bg_slide_update(FieldObj *o);
void fld_bg_door_update(FieldObj *o);

/* doors of the room: each opens when the player comes within 0.75 of it (unless the room or the
   story keeps it shut) and closes when it moves away; then the hinged (info[1] 1) or sliding (0)
   animation */
void fld_bg_doors_update(void)
{
    FieldBg *b = &field_bg_state;
    FieldChr *pl = fld_player_get();
    FieldStory *st = fld_story_get();
    FieldCtx *ctx = fld_ctx_get();
    int i;
    FieldObj *o;
    int near;
    int ok;
    Vec3 p;
    Vec3 d;

    if (pl == 0)
        return;
    if (ctx->pad10 == 0)
        return;
    for (i = 0; i < b->n_objs; i++) {
        o = &b->obj[i];
        p.x = ((float *)o->info)[3] + -1.0f * b->light.x;
        p.y = ((float *)o->info)[4] + -1.0f * b->light.y;
        p.z = ((float *)o->info)[5] + -1.0f * b->light.z;
        ok = 1;
        vec3_sub(&d, &pl->pos, &p);
        near = fld_vec3_len(&d) <= 0.75f;
        switch (fld_bg_id()) {
        case 2:
            if (ctx->x14 != 0)
                ok = 0;
            break;
        case 12:
        case 37:
            if (*(int *)st->w[3] >= 22) {
                switch (o->info[10]) {
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 18:
                case 19:
                case 38:
                    ok = 0;
                    break;
                }
            }
            break;
        }
        switch (o->mode) {
        case 0:
        default:
            if (!near) {
                o->mode = 3;
                if (o->info[1] == 0)
                    fld_bg_door_se(1, o->info[1]);
            }
            break;
        case 1:
            if (!near) {
                o->mode = 3;
                if (o->info[1] == 0)
                    fld_bg_door_se(1, o->info[1]);
            }
            break;
        case 2:
            if (near && ok) {
                o->mode = 1;
                fld_bg_door_se(0, o->info[1]);
            }
            break;
        case 3:
            if (near && ok) {
                o->mode = 1;
                fld_bg_door_se(0, o->info[1]);
            }
            break;
        }
        switch (o->info[1]) {
        case 0:
            fld_bg_slide_update(o);
            break;
        case 1:
            fld_bg_door_update(o);
            break;
        }
    }
}

void vec3_lerp(Vec3 *out, Vec3 *a, Vec3 *b, float t);

/* sliding door: opens/closes like fld_bg_door_update but slides by 1.5 along the axis info[9] turned
   with its rotation, from its position relative to field_bg_state.light */
void fld_bg_slide_update(FieldObj *o)
{
    Vec3 v;
    Vec3 a;
    Vec3 dir;
    Vec4 d;
    Mat4 m __attribute__((aligned(16)));
    Vec4 p;
    Vec4 r;
    float z;
    float y;
    float x;
    float t;

    {
        char *q = (char *)&v;
        int n = 12;

        if (q)
            do {
                *q++ = 0;
            } while (--n);
    }
    {
        char *q = (char *)&a;
        int n = 12;

        if (q)
            do {
                *q++ = 0;
            } while (--n);
    }
    {
        char *q = (char *)&d;
        int n = 16;

        if (q)
            do {
                *q++ = 0;
            } while (--n);
    }
    d.x = ((float *)o->info)[9];
    r.x = 3.1415927f * ((float *)o->info)[6] / 180.0f;
    r.y = 3.1415927f * ((float *)o->info)[7] / 180.0f;
    r.z = 3.1415927f * ((float *)o->info)[8] / 180.0f;
    r.w = 0.0f;
    mat4_identity(&m);
    mat4_rotate_xyz(&m, &m, &r);
    mat4_transform_vec4(&d, &m, &d);
    dir.x = d.x;
    dir.y = d.y;
    dir.z = d.z;
    vec3_scale(&dir, &dir, 1.5f);
    x = ((float *)o->info)[3] + -1.0f * field_bg_state.light.x;
    y = ((float *)o->info)[4] + -1.0f * field_bg_state.light.y;
    z = ((float *)o->info)[5] + -1.0f * field_bg_state.light.z;
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
        } else {
            t = o->open - (0.1f + 0.1f * o->open);
            o->open = t;
            if (t < 0.0f)
                o->open = 0.0f;
        }
        break;
    }
    vec3_lerp(&v, &a, &dir, o->open);
    p.x = x + v.x;
    p.y = y + v.y;
    p.z = z + v.z;
    p.w = 0.0f;
    mat4_identity(&o->mtx);
    mat4_mul(&o->mtx, &o->mtx, &m);
    mat4_translate(&o->mtx, &o->mtx, &p);
}

/* door of the room: opens/closes by 0.1 + 10% a frame, turned around y by info[9] turns when open,
   then placed at its position relative to field_bg_state.light */
void fld_bg_door_update(FieldObj *o)
{
    FieldBg *b = &field_bg_state;
    float turn = 6.2831855f * ((float *)o->info)[9];
    float t;
    float ang;
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
    ang = o->open * turn;
    mat4_identity(&o->mtx);
    r.x = 3.1415927f * ((float *)o->info)[6] / 180.0f;
    r.y = 3.1415927f * ((float *)o->info)[7] / 180.0f + ang;
    r.z = 3.1415927f * ((float *)o->info)[8] / 180.0f;
    r.w = 0.0f;
    mat4_rotate_xyz(&o->mtx, &o->mtx, &r);
    p.x = ((float *)o->info)[3] + -1.0f * b->light.x;
    p.y = ((float *)o->info)[4] + -1.0f * b->light.y;
    p.z = ((float *)o->info)[5] + -1.0f * b->light.z;
    p.w = 0.0f;
    mat4_translate(&o->mtx, &o->mtx, &p);
}

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

void *dl_tex_mode(void *list, int psm, int maxmips, int a2, int swizzle);
void *sceGeEdramGetAddr(void);
void *dl_tex_image(void *list, int mip, int w, int h, int tbw, void *addr);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_tex_wrap(void *list, int u, int v);
void *dl_tex_offset(void *list, float u, float v);
void *dl_tex_scale(void *list, float u, float v);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);

/* lays the 256x256 texture drawn by fld_bg_draw_to_tex over the screen in four strips, through the
   stencil, faded by field_bg_state.fade_t */
void fld_bg_draw_from_tex(void)
{
    void *list = dl_select_layer(1, 0);
    unsigned int color;
    FieldTexVtx *v;
    int i;
    int a;
    int b;

    list = dl_enable(list, 3);
    list = dl_stencil_op(list, 2, 1, 0xFF);
    list = dl_stencil_func(list, 0, 0, 0);
    list = dl_tex_flush(list);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x100, 0x100, 0x100, (char *)sceGeEdramGetAddr() + 0x164000);
    list = dl_tex_func(list, 0, 0);
    list = dl_tex_wrap(list, 0, 0);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(list, 1.0f, 1.0f);
    list = dl_set_tex_filter(list, 1, 1);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_enable(list, 4);
    list = dl_disable(list, 1);
    color = (unsigned int)(255.0f - 8.5f * (float)field_bg_state.fade_t) << 24 | 0xFFFFFF;
    v = dl_alloc_inline(&list, sizeof(FieldTexVtx) * 8);
    for (i = 0, a = 0, b = 1; i < 4; a += 2, b += 2, i++) {
        FieldTexVtx *p = (FieldTexVtx *)((char *)v + a * sizeof(FieldTexVtx));
        FieldTexVtx *q;

        p->u = i * 64;
        p->v = 0;
        p->color = color;
        p->x = i * 120;
        p->y = 0;
        p->z = 0;
        q = (FieldTexVtx *)((char *)v + b * sizeof(FieldTexVtx));
        q->u = (i + 1) * 64;
        q->v = 256;
        q->color = color;
        q->x = (i + 1) * 120;
        q->y = 272;
        q->z = 0;
    }
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    list = dl_draw_array(list, 6, 0x80011E, 8, 0, v);
    mtx_pop();
    dl_set_list(list);
}

void fld_bg_set_hook(int id)
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
