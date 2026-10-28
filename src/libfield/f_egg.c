// @range 0x088290BC-0x08829F5C
// @rodata 0x088399E4-0x08839A4C
// @data 0x0883D690-0x0883D6D8
/* The golden egg lottery (field_egg): the items roll over the head of the player until a button
   stops them; the golden egg, when it comes out, shows the goldegg_pan model and the Effect_04
   effect before the message. */
#include "libfield.h"

typedef struct FeggFrame {
    int unused[2];
    int x, y;
    short u, v, w, h;
} FeggFrame;

void *gim_set_texture(void *list, void *gim);
void *dl_ambient_color(void *list, unsigned int color);
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);
int *fld_item_info(int i);
void fld_player_fx(int kind, void *icon);
unsigned int fld_color_lerp(float t, unsigned int a, unsigned int b);
unsigned short region_pick_value(int flag);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short *wcscat(unsigned short *d, const unsigned short *s);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
FieldCamera *camera_get(void);
void mat4_mul_rot_x(float a, Mat4 *out, Mat4 *m);
void *efs_start(const char *name, const char *path, int flag);
void efs_send(void *e, int a, int b, void *arg);
void efs_inst_free(void *e);
void efs_update(void);
void *fld_dl_clear(void *list, int z, unsigned int color);
void *light_set(void *list, int i, void *light);
void *light_enable(void *list, int i);
void *light_set_ambient(void *list, unsigned int color);
void mtx_scale3(Vec3 *s);
void *fcam_apply(void *list);
void *model_load(void *data, void **anims);
void model_anim_set(void *m, void *data);

void fegg_start(void);
void fegg_fade_in(void);
void fegg_roll(void);
void fegg_result(void);
void fegg_wait(void);
void fegg_anim(void);
void fegg_draw_model(void);
void fegg_free(void);
void fegg_draw_icon(void);
void fegg_make_list(void);
void fegg_loaded(int *file);

/* frame of the item icon in the message window */
const FeggFrame fegg_frame = { { 10, 10 }, 12, 12, 0xE8, 0, 0x18, 0x18 };
/* five orders of the ten items of the lottery */
const signed char fegg_lists[5][10] = {
    { 1, 8, 15, 19, 21, 26, 34, 38, 40, 45 },
    { 2, 9, 13, 16, 22, 39, 46, 30, 41, 33 },
    { 4, 7, 23, 20, 12, 27, 32, 42, 44, 47 },
    { 3, 5, 11, 17, 25, 28, 43, 48, 35, 37 },
    { 6, 29, 14, 18, 24, 10, 31, 36, 43, 49 },
};

void fegg_init(int rare, int kind)
{
    memset(&field_egg, 0, sizeof(field_egg));
    field_egg.rare = rare;
    field_egg.kind = kind;
}

int fegg_update(void)
{
    FieldEgg *e = &field_egg;
    int ret = -1;

    switch (field_egg.state) {
    case 0:
        fegg_start();
        e->state++;
        break;
    case 1:
        fegg_fade_in();
        ret = -1;
        break;
    case 2:
        fegg_roll();
        break;
    case 3:
        fegg_result();
        break;
    case 4:
        fegg_wait();
        if (e->hit != 0)
            fegg_anim();
        break;
    case 5:
        if (fmsg_moving() == 0)
            e->state = 6;
        break;
    case 6:
        fegg_free();
        ret = e->item;
        break;
    }
    return ret;
}

void fegg_draw(void)
{
    FieldEgg *e = &field_egg;

    switch (field_egg.state) {
    case 3:
    case 4:
    case 5:
        fegg_draw_icon();
        if (e->hit != 0)
            fegg_draw_model();
        break;
    }
}

void fegg_start(void)
{
    int *info;
    int se;

    fmsg_set_input(0);
    fmsg_set_text(table_get_entry(fld_gim_get(5), 0x62), 0);
    fegg_make_list();
    field_egg.cur = (int)(rand() & 0xFFFF) % field_egg.n_items;
    field_egg.item = field_egg.items[field_egg.cur];
    info = fld_item_info(field_egg.item);
    memset(&field_egg.icon, 0, sizeof(field_egg.icon));
    field_egg.icon.gim = 0;
    field_egg.icon.sx = 0x100;
    field_egg.icon.sy = 0x100;
    field_egg.icon.u = info[0];
    field_egg.icon.v = info[1];
    field_egg.icon.w = 0x18;
    field_egg.icon.h = 0x18;
    field_egg.icon.color = 0xFFFFFF;
    field_egg.icon.mode = 2;
    field_egg.icon.dx = 0xD;
    field_egg.icon.dy = -3;
    fld_player_fx(0, &field_egg.icon);
    fld_load_gim(5, (void (*)(int *, void *))fegg_loaded, 0);
    switch (field_egg.kind) {
    case 0x1E:
    default:
        se = 0x61FB;
        break;
    case 0x1F:
        se = 0x620E;
        break;
    }
    fld_story_flag(se, 0);
}

void fegg_fade_in(void)
{
    FieldEgg *e = &field_egg;

    field_egg.icon.color = fld_color_lerp(field_egg.fade / 30.0f, 0xFFFFFF, 0xFFFFFFFF);
    if (field_egg.fade >= 30) {
        if (e->loaded != 0) {
            e->icon.gim = fld_gim_get(3);
            e->state = 2;
        }
    } else {
        e->fade++;
    }
}

void fegg_roll(void)
{
    FieldEgg *e = &field_egg;
    int *info;
    PadState *pad;

    if (field_egg.frame % 3 == 0) {
        e->item = e->items[e->cur];
        info = fld_item_info(e->item);
        e->icon.u = info[0];
        e->icon.v = info[1];
        if (++e->cur >= e->n_items)
            e->cur = 0;
        fld_story_flag(0x626D, 0);
    }
    e->frame++;
    pad = pad_get();
    if (pad->b.trig & region_pick_value(0)) {
        fld_story_flag(0x6213, 0);
        if (e->item == 0x32) {
            e->hit = 1;
            fld_story_flag(0x626E, 0);
        }
        e->state = 3;
    }
}

void fegg_result(void)
{
    FieldEgg *e = &field_egg;
    MenuItemDef *m = fmsg_def();
    int input = !field_egg.hit;
    unsigned short *fmt;

    if (region_get() == 0) {
        wcscpy(e->text, table_get_entry(fld_gim_get(6), e->item + 0x32));
        wcscat(e->text, table_get_entry(fld_gim_get(5), 99));
    } else {
        fmt = table_get_entry(fld_gim_get(5), 99);
        wsprintf(e->text, fmt, table_get_entry(fld_gim_get(6), e->item + 0x32));
    }
    m->pad_l = 0x28;
    m->pad_t = 5;
    fmsg_set_input(input);
    fmsg_set_text(e->text, 0);
    e->state++;
}

void fegg_wait(void)
{
    FieldEgg *e = &field_egg;
    int done = 0;

    if (field_egg.hit != 0) {
        if (e->step == 4)
            done = 1;
    } else if (fmsg_done() != 0) {
        done = 1;
    }
    if (done) {
        fmsg_close();
        e->state = 5;
        fld_player_fx(0, 0);
    }
}

void fegg_anim(void)
{
    FieldEgg *e = &field_egg;
    int reset = 0;
    int se;
    FieldCamera *cam;
    Mat4 m;
    Mat4 r;
    PadState *pad;

    switch (field_egg.step) {
    case 0:
        cam = camera_get();
        m = *(Mat4 *)cam->rot;
        ((Vec4 *)&m)[3].x = cam->target.x;
        ((Vec4 *)&m)[3].y = cam->target.y;
        ((Vec4 *)&m)[3].z = cam->target.z;
        mat4_identity(&r);
        mat4_mul_rot_x(0.52359879f, &r, &r);
        ((Vec4 *)&m)[3].y = cam->target.y;
        mat4_mul(&e->mtx_efs, &m, &r);
        ((Vec4 *)&m)[3].y = -0.1f + cam->target.y;
        mat4_mul(&e->mtx_model, &m, &r);
        model_update(e->model, 0);
        reset = 1;
        e->step++;
        break;
    case 1:
        model_update(e->model, 0);
        if (e->count >= 0x35) {
            e->step++;
            switch (e->kind) {
            case 0x1E:
            default:
                se = 0x61FC;
                break;
            case 0x1F:
                se = 0x620F;
                break;
            }
            fld_story_flag(se, 0);
        }
        break;
    case 2:
        e->efs = efs_start("Effect_04", "", 0);
        if (e->efs != 0)
            efs_send(e->efs, 0x101, 0x10003, &e->mtx_efs);
        e->step++;
        break;
    case 3:
        model_update(e->model, 0);
        if (e->count >= 0x5A) {
            fmsg_set_input(1);
            pad = pad_get();
            if (pad->b.trig & region_pick_value(0)) {
                if (e->efs != 0)
                    efs_inst_free(e->efs);
                e->step++;
            }
        }
        break;
    case 4:
        break;
    }
    if (reset)
        e->count = 0;
    else
        e->count++;
}

void fegg_draw_model(void)
{
    FieldEgg *e = &field_egg;
    void *list;

    switch (field_egg.step) {
    case 0:
        return;
    case 1:
    case 2:
    case 3:
        if (e->model != 0) {
            FieldGeLight light;
            ModelDrawOpts opts;
            Vec3 scale = { 0.85f, 0.85f, 0.85f };

            memset(&opts, 0, sizeof(opts));
            opts.a = 3;
            opts.b = 4;
            list = dl_select_layer(3, 0);
            list = fld_dl_clear(list, 0xFFFF, 0x80000000);
            list = dl_enable(list, 1);
            memset(&light, 0, sizeof(light));
            light.diffuse = 0xFF000000;
            light.specular = 0xFFFFFFFF;
            {
                Vec3 dir = { -0.5f, 3.0f, 1.0f };

                light.type = 0;
                light.kind = 0;
                light.dir = *(Vec3 *)&dir; /* through memory, as in the original */
            }
            list = light_set(list, 0, &light);
            list = light_enable(list, 0);
            list = light_set_ambient(list, 0xFF404040);
            dl_set_list(list);
            mtx_set_mode(2);
            mtx_push();
            mtx_load(&e->mtx_model);
            mtx_scale3(&scale);
            model_draw(e->model, &opts);
            mtx_pop();
            dl_set_list(fcam_apply(dl_select_layer(3, 0)));
        }
        mtx_set_mode(2);
        efs_update();
        break;
    case 4:
        break;
    }
}

void fegg_free(void)
{
    FieldEgg *e = &field_egg;

    fld_heap_get();
    if (field_egg.items != 0) {
        fld_free(e->items);
        e->items = 0;
    }
    if (e->model != 0) {
        model_free(e->model);
        e->model = 0;
    }
}

void fegg_draw_icon(void)
{
    FieldEgg *e;
    MenuItemDef *m;
    int *info;
    void *gim;
    int dy;
    int x;
    int y;
    void *list;

    e = &field_egg;
    m = fmsg_def();
    info = fld_item_info(field_egg.item);
    gim = fld_gim_get(0);
    switch (cfg_get_language()) { case 0: dy = 0; break; default: dy = -7; break; }
    list = dl_select_layer(4, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, gim);
    y = fegg_frame.y;
    x = fegg_frame.x;
    list = fld_draw_cell1(list, m->x + x, dy + (m->y + y), fegg_frame.u, fegg_frame.v, fegg_frame.w,
                          fegg_frame.h);
    list = gim_set_texture(list, e->icon.gim);
    list = fld_draw_cell1(list, m->x + x, dy + (m->y + y), info[0], info[1], 0x18, 0x18);
    dl_set_list(list);
}

void fegg_make_list(void)
{
    FieldEgg *e = &field_egg;
    int rare;
    int at = 0;
    int k;
    int i;
    int j;
    const signed char *p;

    if (field_egg.rare != 0 && (int)(rand() & 0xFFFF) % 10 == 0)
        rare = 1;
    else
        rare = 0;
    e->n_items = 10;
    if (rare) {
        e->n_items++;
        at = (int)(rand() & 0xFFFF) % 10;
    }
    e->items = fld_alloc(e->n_items << 2);
    memset(e->items, 0, e->n_items << 2);
    k = (int)(rand() & 0xFFFF) % 5;
    if (rare) {
        for (i = 0; i < 10; i++) {
            j = i;
            if (i >= at)
                j = i + 1;
            e->items[j] = fegg_lists[k][i];
        }
        e->items[at] = 0x32;
    } else {
        i = 0;
        p = fegg_lists[k];
        for (; i < 10; i++) {
            e->items[i] = *p;
            p++;
        }
    }
}

void fegg_loaded(int *file)
{
    FieldEgg *e = &field_egg;
    void *tms;
    void *tmt;

    if ((field_egg.pack = (void *)*file) != 0) {
        tms = ehp_find(field_egg.pack, "goldegg_pan.tms", 0);
        tmt = ehp_find(e->pack, "goldegg_pan.tmt", 0);
        e->model = model_load(tms, 0);
        model_anim_set(e->model, tmt);
        e->loaded = 1;
    }
}
