// @range 0x08825F7C-0x08826BB0
// @rodata 0x08839368-0x08839458
/* "You got ..." messages of the map (field_get): an item with its icon, or a card that spins into
   view in 3D (its face rendered into VRAM by a model hook) and can be opened in the card viewer. */
#include "libfield.h"

void *fld_cip_get(void);
void fmsg_lock(int on);
void fmsg_close(void);
int fmsg_moving(void);
int fmsg_done(void);
void fmsg_set_text(unsigned short *text, int title);
void fmsg_reset(int x, int w, int size, int input, int style);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
unsigned short *card_get_name(int id);
void *mat4_zero(Mat4 *out);
Mat4 *mat4_mul_rot_y(float a, Mat4 *out, Mat4 *m);
FieldCamera *camera_get(void);
int inv_get_hi(unsigned short id);
unsigned int cip_read_card(void *handle, int id, int variant);
int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim);
void cardinfo_set_lists(void *list_gim, unsigned int *text_pack, unsigned short *index);
int cardinfo_update(void);
void cardinfo_draw(void);
int fade_is_done(void);
void cardlist_init(void *cfg);
void cardlist_clear(void);
void *cardlist_draw_quad(void *list, int x, int y);
void *card_draw(void *list, int x, int y, unsigned int id);
void *model_load(void *data, void **anims);
void *model_hook(void *m, int which, void *v);
void mtx_perspective(float fovy, float aspect, float near, float far);
void mtx_load(Mat4 *m);
void *dl_tex_flush(void *list);
void *gfx_set_draw_target(void *list, int addr, int a, int w, int h, int fmt);
void *gfx_set_default_target(void *list);
void *dl_tex_mode(void *list, int mode, int a, int b, int c);
void *dl_tex_image(void *list, int mip, int w, int h, int stride, const void *data);
void *dl_tex_func(void *list, int func, int comp);
void *dl_tex_wrap(void *list, int u, int v);
void *dl_tex_offset(float u, float v, void *list);
void *dl_tex_scale(void *list, float u, float v);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_ambient_color(void *list, unsigned int color);
void *sceGeEdramGetAddr(void);
void *gim_set_texture(void *list, void *gim);
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);
int fade_apply(void **list);

void fget_item_update(void);
void fget_card_update(void);
void fget_item_draw(void);
void fget_card_draw(void);
int fget_card_hook(void **list, void *arg, int pass);

/* icon of each item in the item texture */
const FieldOff field_get_icons[22] = {
    { 0, 0 },     { 0, 120 },   { 24, 120 },  { 48, 120 },  { 72, 120 },  { 96, 120 },
    { 120, 120 }, { 144, 120 }, { 168, 120 }, { 192, 120 }, { 216, 120 }, { 0, 144 },
    { 48, 120 },  { 0, 120 },   { 24, 120 },  { 48, 120 },  { 0, 120 },   { 24, 120 },
    { 48, 120 },  { 0, 120 },   { 24, 120 },  { 10, 10 },
};
const FieldOff field_get_icon_pos = { 12, 12 };
const FieldSpr field_get_icon_frame = { 232, 0, 24, 24 };

void fget_clear(void)
{
    memset(&field_get, 0, sizeof(field_get));
}

void fget_free(void)
{
    model_free(field_get.card.model);
    cardlist_clear();
    memset(&field_get, 0, sizeof(field_get));
}

void fget_update(void)
{
    switch (field_get.kind) {
    case 1:
        fget_item_update();
        break;
    case 2:
        fget_card_update();
        break;
    }
}

void fget_draw(void)
{
    switch (field_get.kind) {
    case 1:
        fget_item_draw();
        break;
    case 2:
        fget_card_draw();
        break;
    }
}

/* loads the card model and the textures of the card renderer */
void fget_load(void)
{
    void **l;

    field_get.card.model = model_load(fld_gim_get(9), 0);
    l = field_get.card.list;
    memset(l, 0, sizeof(field_get.card.list));
    l[0] = fld_gim_get(10);
    l[1] = fld_gim_get(11);
    l[3] = fld_gim_get(15);
    l[4] = fld_gim_get(16);
    l[5] = fld_gim_get(17);
    l[7] = fld_cip_get();
    cardlist_init(l);
}

/* "You got <item>." */
void fget_item_open(int item)
{
    MenuItemDef *d = fmsg_def();
    unsigned short *name = table_get_entry(fld_gim_get(6), item);

    memset(&field_get, 0, 0x10C);
    field_get.id = item;
    wsprintf(field_get.text, table_get_entry(fld_gim_get(5), 0x1B), name);
    fmsg_reset(0x27, 400, 14, 1, 3);
    d->pad_l = 0x28;
    d->pad_t = 5;
    fmsg_set_text(field_get.text, 0);
    field_get.kind = 1;
}

void fget_item_close(int *flag)
{
    FieldGet *g = &field_get;

    if (field_get.done != 0 && fmsg_moving() == 0) {
        g->kind = 0;
        *flag = 0;
    }
}

/* "You got <card>." (reset: also places the message window) */
void fget_card_open(int card, int reset, int close)
{
    FieldGet *g = &field_get;
    FieldGetCard *c = &field_get.card;
    MenuItemDef *d = fmsg_def();
    unsigned short *name = card_get_name(card);

    memset(g, 0, 0x10C);
    field_get.id = card;
    c->spin = 0;
    field_get.card.close = close;
    mat4_zero(&c->mtx);
    wsprintf(field_get.text, table_get_entry(fld_gim_get(5), 0x1C), name);
    if (reset) {
        fmsg_reset(0x27, 400, 14, 1, 3);
        d->pad_l = 4;
        d->pad_t = 2;
    }
    fmsg_set_text(g->text, 0);
    g->kind = 2;
    fld_story_flag(0x621A, 0);
}

void fget_card_close(int *flag)
{
    FieldGet *g = &field_get;

    if (field_get.done != 0 && fmsg_moving() == 0) {
        g->kind = 0;
        *flag = 0;
    }
}

/* nonzero once the message is closed */
int fget_done(void)
{
    FieldGet *g = &field_get;

    switch (field_get.kind) {
    case 2:
        if (g->done != 0) {
            g->kind = 0;
            return 1;
        }
        return 0;
    }
    return 1;
}

void fget_item_update(void)
{
    FieldGet *g = &field_get;

    if (fmsg_done()) {
        fmsg_close();
        g->done = 1;
    }
}

void fget_item_draw(void)
{
    const FieldOff *icon = &field_get_icons[field_get.id];
    MenuItemDef *d = fmsg_def();
    void *frame = fld_gim_get(0);
    void *items = fld_gim_get(3);
    void *list;
    int x, y;

    if (frame == 0 || items == 0)
        return;
    if (fmsg_def()->visible != 0) {
        list = dl_select_layer(4, 0);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = gim_set_texture(list, frame);
        y = field_get_icon_pos.y;
        x = field_get_icon_pos.x;
        list = fld_draw_cell1(list, d->x + x, d->y + y, field_get_icon_frame.u, field_get_icon_frame.v,
                              field_get_icon_frame.w, field_get_icon_frame.h);
        list = gim_set_texture(list, items);
        list = fld_draw_cell1(list, d->x + x, d->y + y, icon->x, icon->y, 24, 24);
        dl_set_list(list);
    }
}

void fget_card_update(void)
{
    FieldGet *g = &field_get;
    FieldGetCard *c = &field_get.card;
    FieldCamera *cam;
    void **lists;
    void *gim;
    float a;
    Mat4 view;
    Mat4 t;
    Mat4 m;
    Mat4 r;

    switch (field_get.step) {
    case 0:
        gim = fld_cip_get();
        if (cip_read_card(gim, g->id, inv_get_hi(g->id)) != 0)
            g->step = 1;
        break;
    case 1:
        cam = camera_get();
        view = *(Mat4 *)cam->rot;
        view.m[12] = cam->target.x;
        view.m[13] = cam->target.y;
        view.m[14] = cam->target.z;
        mat4_identity(&t);
        t.m[13] = 0.31f;
        t.m[14] = -0.5f;
        mat4_identity(&m);
        mat4_mul(&m, &m, &view);
        mat4_mul(&m, &m, &t);
        a = (float)c->spin / 15.0f * 3.1415927f;
        mat4_identity(&r);
        mat4_mul_rot_y(a, &r, &r);
        mat4_mul(&m, &m, &r);
        if (c->spin < 15)
            c->spin++;
        else
            g->step = 2;
        c->mtx = m;
        break;
    case 2:
        if (fmsg_done()) {
            if (g->card.close != 0)
                fmsg_close();
            g->done = 1;
        }
        if (pad_get()->b.trig & 0x1000) {
            fade_start_in(1, 0xFF000000, 16);
            g->step = 3;
            fmsg_lock(1);
        }
        break;
    case 3:
        if (fade_is_done()) {
            gim = fld_gim_get(12);
            cardinfo_open(g->id, gim, fld_gim_get(14));
            lists = fld_gim_get(18);
            cardinfo_set_lists(fld_gim_get(13), lists[1], lists[0]);
            g->step = 4;
        }
        break;
    case 4:
        if (cardinfo_update()) {
            fade_start(1, 0xFF000000, 16);
            g->step = 5;
        }
        break;
    case 5:
        if (fade_is_done()) {
            g->step = 2;
            fmsg_lock(0);
        }
        break;
    }
}

void fget_card_draw(void)
{
    FieldGetCard *c = &field_get.card;
    void *model;
    void *list;
    ModelDrawOpts opts;

    switch (field_get.step) {
    case 3:
    case 5:
        list = dl_select_layer(5, 0);
        list = dl_disable(list, 1);
        fade_apply(&list);
        dl_set_list(list);
    case 0:
    case 1:
    case 2:
        model = c->model;
        memset(&opts, 0, sizeof(opts));
        mtx_set_mode(0);
        mtx_push();
        mtx_identity();
        mtx_perspective(0.383972466f, 1.7647059f, 5.0f, 400.0f);
        model_hook(model, 0, fget_card_hook);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        mtx_load(&c->mtx);
        opts.a = 4;
        opts.b = 4;
        model_update(model, 0);
        model_draw(model, &opts);
        mtx_set_mode(2);
        mtx_pop();
        mtx_set_mode(0);
        mtx_pop();
        break;
    case 4:
        cardinfo_draw();
        break;
    }
}

/* draw hook of the card model: renders the back (pass 0) or the face (pass 1) of the card into VRAM
   and maps it on the model */
int fget_card_hook(void **list, void *arg, int pass)
{
    FieldGet *g = &field_get;
    void *l = *list;

    if (pass != 0 && pass != 1)
        return 0;
    l = dl_enable(l, 9);
    l = dl_enable(l, 4);
    l = dl_tex_func(l, 3, 1);
    l = dl_blend_func(l, 0, 2, 3, 0, 0);
    l = dl_disable(l, 1);
    l = dl_ambient_color(l, 0xFFFFFFFF);
    l = dl_tex_flush(l);
    l = gfx_set_draw_target(l, 0x160000, -1, 0x80, 0x100, 3);
    if (pass == 0)
        l = cardlist_draw_quad(l, 0, 0);
    else
        l = card_draw(l, 0, 0, g->id);
    l = gfx_set_default_target(l);
    l = dl_tex_flush(l);
    l = dl_enable(l, 9);
    l = dl_tex_mode(l, 3, 0, 0, 0);
    l = dl_tex_image(l, 0, 0x80, 0x100, 0x80, (char *)sceGeEdramGetAddr() + 0x160000);
    l = dl_tex_func(l, 3, 0);
    l = dl_tex_wrap(l, 1, 1);
    l = dl_tex_offset(0.0f, 0.6015625f, l);
    l = dl_tex_scale(l, 0.84375f, -0.6015625f);
    *list = dl_enable(l, 1);
    return 1;
}
