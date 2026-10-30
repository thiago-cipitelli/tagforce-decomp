// @range 0x0882A94C-0x0882BF00
// @asm 0x0882A94C-0x0882B724
// @asm 0x0882B974-0x0882BDDC
// @data 0x088769F0-0x08876A68
/* Coin and die windows: the coin or die model rendered into a 128x128 VRAM texture with two
   rotating lights and drawn as a sprite; the window where the player picks die faces (one or
   more, with their dice shown above) and the OK button window below it. */
#include "dd_d3.h"

typedef struct DdCamera {              /* BOOT camera, 0xC0 */
    FxVec4 pos, target, up;
    unsigned char rest[0x90];
} DdCamera;

typedef struct DdLight {               /* BOOT light, 0x3C */
    int type;
    unsigned int ambient, diffuse, specular;
    float pos[3];
    float dir[3];
    float spot[2];
    float att[3];
} DdLight;

typedef struct DdModelDraw {           /* BOOT model_draw options, 0x34 */
    int layer[2];
    int pad08[8];
    unsigned char front : 1;           /* +0x28 */
    unsigned char pad28 : 7;
    unsigned char pad29[7];
    void *user;
} DdModelDraw;

typedef struct DdDice {                /* 0x08C5A9D8, 0x14C */
    MenuWidget *menu;                  /* +0x000 face list */
    MenuWidget ok;                     /* +0x004 OK button */
    int state;                         /* +0x124 0 picking, 1 on OK, 2 done */
    int picked[6];                     /* +0x128 faces picked (1..6) */
    int n_picked;                      /* +0x140 */
    int max;                           /* +0x144 */
    unsigned int allowed;              /* +0x148 bit per face (bit 1 = face 1) */
} DdDice;

extern DdDice dd_dice;

unsigned int *gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm);
unsigned int *gfx_set_default_target(unsigned int *list);
unsigned int *dl_offset(unsigned int *list, int x, int y);
unsigned int *dl_viewport(unsigned int *list, int cx, int cy, int w, int h);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle);
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr);
int dl_get_layer(void);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_ortho(float left, float right, float bottom, float top, float near, float far);
void mtx_scale3(float *v);
void mtx_rotate_x(float a);
void mtx_rotate_y(float a);
DdCamera *camera_get(void);
void camera_set(FxVec4 *pos, FxVec4 *target, FxVec4 *up);
void camera_load_view(void);
unsigned int *gfx_get_state(void);
float cosf_vfpu(float x);
float sinf_vfpu(float x);
float *vec3_normalize(float *out, float *v);
unsigned int *lighting_enable(unsigned int *list);
unsigned int *lighting_disable(unsigned int *list);
unsigned int *light_set_ambient(unsigned int *list, unsigned int color);
unsigned int *light_set(unsigned int *list, int n, DdLight *l);
unsigned int *light_enable(unsigned int *list, int n);
void model_update(void *m, float *dt);
void model_draw(void *m, DdModelDraw *c);
void *func_08804268(int id);           /* model of the duel resources */
void *sceGeEdramGetAddr(void);
void *gim_set_texture(void *list, void *gim);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
unsigned short *dlg_get_text(int i);
int se_play(int id);
void func_08825824(int a, int b, int c, void *setup);
void dd_mode_push(int a);
void dd_mode_set_flag(int a);
void func_088259FC(void *fn);
void func_08825A08(void *fn);
int func_08825728(void);
void func_08825B9C(MenuWidget *w);
void *dd_text(int a);
int dv_w624_set(unsigned int mask);

unsigned int dd_coin_tex_tmpl[16] = {
    0x00000000, 0x00000000, 0xBF800000, 0x00000000, 0x00000000, 0x3F800000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xBF800000, 0x00000000, 0x00000000, 0xBF800000, 0x00000000, 0x00000000,
};

#ifdef NON_MATCHING /* função grande de render para textura; o modelo dos vetores locais fica em dd_coin_tex_tmpl */
/* the scene around the coin/die model: ortho camera, two lights turning with the frame counter */
static unsigned int *dd_dice_scene_begin(unsigned int *list, float scale, DdCamera *saved)
{
    float s[3] = { scale, scale, scale };
    FxVec4 eye = { 0.0f, 0.0f, -1.0f, 0.0f };
    FxVec4 at = { 0 };
    FxVec4 up = { 0.0f, 1.0f, 0.0f, 0.0f };
    DdLight l0, l1;
    float t;

    list = gfx_set_draw_target(list, 0x160000, 0xFFFFFFFF, 128, 128, 3);
    list = dl_offset(list, 0x7C0, 0x7C0);
    list = dl_viewport(list, 0x800, 0x800, 128, 128);
    list = dl_enable(list, 1);
    list = dl_clear_color(list, 0x6000);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_clear(list, 0x17);
    mtx_set_mode(0);
    mtx_push();
    mtx_identity();
    mtx_ortho(-64.0f, 64.0f, -64.0f, 64.0f, -100.0f, 100.0f);
    mtx_scale3(s);
    *saved = *camera_get();
    camera_set(&eye, &at, &up);
    mtx_set_mode(1);
    camera_load_view();
    t = (float)*gfx_get_state() / 20.0f;
    memset(&l0, 0, sizeof(l0));
    l0.ambient = 0xFF000000;
    l0.specular = 0xFF000000;
    l0.diffuse = 0xFF808080;
    {
        float d[3] = { cosf_vfpu(t), sinf_vfpu(t), -1.0f };
        l0.dir[0] = d[0];
        l0.dir[1] = d[1];
        l0.dir[2] = d[2];
    }
    vec3_normalize(l0.dir, l0.dir);
    t *= 1.25f;
    memset(&l1, 0, sizeof(l1));
    l1.ambient = 0xFF000000;
    l1.specular = 0xFF000000;
    l1.diffuse = 0xFF808080;
    {
        float d[3] = { cosf_vfpu(t), sinf_vfpu(t), -1.0f };
        l1.dir[0] = d[0];
        l1.dir[1] = d[1];
        l1.dir[2] = d[2];
    }
    vec3_normalize(l1.dir, l1.dir);
    list = lighting_enable(list);
    list = light_set_ambient(list, 0xFF101010);
    list = light_set(list, 0, &l0);
    list = light_set(list, 1, &l1);
    list = light_enable(list, 0);
    list = light_enable(list, 1);
    mtx_set_mode(2);
    mtx_push();
    mtx_identity();
    return list;
}

static unsigned int *dd_dice_scene_end(unsigned int *list, int model_id, DdCamera *saved, int x, int y, int w, int h)
{
    DdModelDraw md;
    void *model;

    memset(&md, 0, sizeof(md));
    md.layer[0] = dl_get_layer();
    md.layer[1] = md.layer[0];
    md.front = 1;
    dl_set_list(list);
    model = func_08804268(model_id);
    if (model != 0) {
        model_update(model, 0);
        model_draw(model, &md);
    }
    list = dl_select_layer(md.layer[0], 0);
    camera_set(&saved->pos, &saved->target, &saved->up);
    list = lighting_disable(list);
    mtx_set_mode(2);
    mtx_pop();
    mtx_set_mode(0);
    mtx_pop();
    list = gfx_set_default_target(list);
    list = dl_offset(list, 0x710, 0x778);
    list = dl_viewport(list, 0x800, 0x800, 480, 272);
    list = dl_enable(list, 9);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 128, 128, 128, (unsigned char *)sceGeEdramGetAddr() + 0x160000);
    list = dl_tex_func(list, 3, 1);
    list = dl_tex_wrap(list, 0, 0);
    list = dl_tex_offset(list, 0.0f, 0.0f);
    list = dl_tex_scale(list, 1.0f, 1.0f);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = draw_set_offsets(list, w, h, w, h);
    list = draw_sprite(list, x, y, 0, (128 - w) / 2, (128 - h) / 2, 0, 0);
    return dl_enable(list, 1);
}

unsigned int *dd_coin_tex(float scale, unsigned int *list, int x, int y, int w, int h, int back)
{
    DdCamera saved;
    float t;

    list = dd_dice_scene_begin(list, scale, &saved);
    if (back)
        mtx_rotate_y(3.1415927f);
    t = (float)*gfx_get_state() / 30.0f;
    mtx_rotate_y(cosf_vfpu(t) * 0.15707964f);
    t = (float)*gfx_get_state() / 30.0f;
    mtx_rotate_x(sinf_vfpu(t) * 0.15707964f);
    return dd_dice_scene_end(list, 0x10021, &saved, x, y, w, h);
}
#endif

unsigned int dd_die_tex_tmpl[14] = {
    0x00000000, 0x00000000, 0xBF800000, 0x00000000, 0x00000000, 0x3F800000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xBF800000, 0x00000000, 0x00000000, 0xBF800000,
};

#ifdef NON_MATCHING /* função grande de render para textura; o modelo dos vetores locais fica em dd_die_tex_tmpl */
unsigned int *dd_die_tex(float scale, unsigned int *list, int x, int y, int w, int h, int face)
{
    DdCamera saved;
    float t;

    list = dd_dice_scene_begin(list, scale, &saved);
    t = (float)*gfx_get_state() / 30.0f;
    mtx_rotate_y(cosf_vfpu(t) * 0.15707964f);
    t = (float)*gfx_get_state() / 30.0f;
    mtx_rotate_x(sinf_vfpu(t) * 0.15707964f);
    switch (face) {             /* turn the face up */
    case 0:
    case 5:
        break;
    case 1:
        mtx_rotate_x(4.712389f);
        break;
    case 2:
        mtx_rotate_y(3.1415927f);
        break;
    case 3:
        mtx_rotate_y(1.5707964f);
        break;
    case 4:
        mtx_rotate_y(4.712389f);
        break;
    case 6:
        mtx_rotate_x(1.5707964f);
        mtx_rotate_y(1.5707964f);
        break;
    }
    return dd_dice_scene_end(list, 0x10026, &saved, x, y, w, h);
}
#endif

void dd_dice_setup(MenuWidget *w, int init);
void dd_dice_mask(void);
int dd_dice_update(void);
void dd_dice_ok_open(void);
void dd_dice_ok_reset(void);
int dd_dice_ok_update(void);
unsigned int *dd_dice_ok_draw(unsigned int *list, int x, int y);
unsigned int *dd_dice_draw(unsigned int *list, int x, int y);
unsigned int *dd_die_tex(float scale, unsigned int *list, int x, int y, int w, int h, int face);

void dd_dice_open(int a, int max, unsigned int allowed)
{
    memset(&dd_dice, 0, sizeof(dd_dice));
    dd_dice.n_picked = 0;
    dd_dice.max = max;
    dd_dice.allowed = allowed;
    func_08825824(0, a, 0x30, dd_dice_setup);
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(dd_dice_mask);
    func_08825A08(dd_dice_update);
    se_play(0x621A);
}

void dd_dice_setup(MenuWidget *w, int init)
{
    DdDice *d = &dd_dice;

    if (init == 0)
        return;
    d->menu = w;
    w->keep_open = 1;
    w->cancel_blocked = 0;
    w->skip_disabled = 1;
    w->disabled = ~(d->allowed >> 1);
    w->count = 6;
    w->rows = 1;
    w->item_w = 100;
    w->item_h = 140;
    w->draw_all = (MenuDrawRectFn)dd_dice_draw;
}

void dd_dice_mask(void)
{
    DdDice *d = &dd_dice;
    unsigned int m = 0;
    int i;

    for (i = 0; i < d->n_picked; i++)
        m |= 1 << d->picked[i];
    dv_w624_set(m);
}

#ifdef NON_MATCHING /* alocação de registradores e um bloco de junção a mais (forma igual) */
int dd_dice_update(void)
{
    MenuWidget *w = dd_dice.menu;
    DdDice *d = &dd_dice;
    int r;

    switch (dd_dice.state) {
    case 0:
        r = menu_update(w);
        if (r >= 0) {
            if (d->n_picked < d->max) {
                d->picked[d->n_picked] = r + 1;
                d->n_picked++;
                w->disabled |= 1 << r;
                w->cancelled = w->accepted = 0;
                if (d->n_picked >= d->max) {
                    dd_dice_ok_open();
                    d->state = 1;
                }
            }
        } else if (r == -2) {
            if (d->n_picked > 0) {
                d->n_picked--;
                w->disabled &= ~(1 << (d->picked[d->n_picked] - 1));
            }
            w->cancelled = w->accepted = 0;
        }
        break;
    case 1:
        r = dd_dice_ok_update();
        if (r == 2) {
            dd_dice_ok_reset();
            menu_finish(w);
            d->state = 2;
        } else if (r == 3 || r == -2) {
            dd_dice_ok_reset();
            d->n_picked--;
            w->disabled &= ~(1 << (d->picked[d->n_picked] - 1));
            d->state = 0;
        }
        break;
    case 2:
        menu_update(w);
        break;
    }
    return !func_08825728();
}
#endif

#ifdef NON_MATCHING /* alocação de registradores e um bloco de junção a mais (forma igual) */
unsigned int *dd_dice_draw(unsigned int *list, int x, int y)
{
    MenuWidget *w = dd_dice.menu;
    DdDice *d = &dd_dice;
    int inner = w->def.w - w->def.pad_l - w->def.pad_r - 9;
    int cx = x + (inner - dd_dice.n_picked * 40) / 2;
    int face;
    int i;

    for (i = 0; i < dd_dice.n_picked; i++, cx += 40)
        list = dd_die_tex(0.7f, list, cx, y, 40, 40, d->picked[i]);
    if (d->state == 0) {
        face = (unsigned short)(w->scroll + w->cursor);
        cx = x + (inner - 100) / 2;
        list = dl_tex_func(list, 0, 0);
        list = menu_draw_cursor(list, w, cx + 4, y + 44, 92, 92);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 1);
        list = dd_die_tex(1.5f, list, cx, y + 40, 100, 100, face + 1);
        list = dl_disable(list, 1);
        list = gim_set_texture(list, w->icon_gim);
        list = menu_draw_arrow(list, cx, y + 88, 2);
        list = menu_draw_arrow(list, cx + 100, y + 88, 3);
        list = dl_enable(list, 1);
    }
    if (d->state == 1)
        list = dd_dice_ok_draw(list, x + inner / 2, y + 60);
    return list;
}
#endif

void dd_dice_ok_open(void)
{
    MenuWidget *w = &dd_dice.ok;

    func_08825B9C(w);
    w->def.title = dlg_get_text(590);
    w->def.text = dd_text(32);
    w->def.pad_l = 20;
    w->def.pad_r = 20;
    w->def.pad_t = 10;
    w->def.pad_b = 10;
    w->def.w = 0;
    w->cancel_blocked = 0;
    w->cancel_resets = 1;
    w->initial = 3;
    w->buttons = 3;
    menu_open(w);
}

void dd_dice_ok_reset(void)
{
    menu_reset(&dd_dice.ok);
}

int dd_dice_ok_update(void)
{
    return menu_update(&dd_dice.ok);
}

unsigned int *dd_dice_ok_draw(unsigned int *list, int x, int y)
{
    MenuWidget *w = &dd_dice.ok;

    w->def.x = x - w->def.w / 2;
    w->def.y = y;
    menu_draw((void **)&list, w);
    return list;
}
