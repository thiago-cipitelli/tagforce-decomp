// @range 0x08829F5C-0x0882A3A8
/* Full-screen picture of the scene commands (field_pict): a 480x272 image loaded from the voice
   pool, faded in and out over 15 frames; the id -2 fades a black screen instead. */
#include "libfield.h"

void *gim_set_texture(void *list, void *gim);
void *dl_ambient_color(void *list, unsigned int color);
void *fld_dl_clear(void *list, int z, unsigned int color);
unsigned int fld_color_lerp(float t, unsigned int a, unsigned int b);
void fld_req_cancel_all(void);
void fld_load_voice(int name, void (*cb)(), void *arg);

void fpic_nop(void);
void fpic_loaded(int *file);
void fpic_fade(int in);

void fpic_clear(void)
{
    memset(&field_pict, 0, sizeof(field_pict));
}

void fpic_term(void)
{
    fpic_nop();
    fpic_clear();
}

void fpic_nop(void)
{
}

void fpic_update(void)
{
    FieldPict *p = &field_pict;

    switch (field_pict.state) {
    case 0:
        break;
    case 1:
        p->count++;
        fpic_fade(1);
        if (p->count >= 15) {
            p->count = 0;
            p->busy = 0;
            p->state = 2;
        }
        break;
    case 2:
        break;
    case 3:
        p->count++;
        fpic_fade(0);
        if (p->count >= 15) {
            if (p->fade_only == 0)
                fld_req_cancel_all();
            p->state = 0;
            p->busy = 0;
        }
        break;
    }
}

void fpic_draw(void)
{
    FieldPict *p = &field_pict;
    void *list;
    int i;
    int x;

    if (field_pict.state != 0) {
        list = dl_select_layer(3, 0);
        if (p->fade_only != 0) {
            list = fld_dl_clear(list, 0, p->color & 0xFF000000);
        } else {
            list = dl_ambient_color(list, p->color);
            list = gim_set_texture(list, p->gim);
            list = draw_set_offsets(list, 0x40, 0x100, 0x40, 0x100);
            for (i = 0, x = 0; i < 7; i++, x += 0x40)
                list = draw_sprite(list, x, 0, 0, x, 0, 0, 0);
            list = draw_set_offsets(list, 0x20, 0x100, 0x20, 0x100);
            list = draw_sprite(list, 0x1C0, 0, 0, 0x1C0, 0, 0, 0);
            list = draw_set_offsets(list, 0x10, 0x100, 0x100, 0x10);
            list = draw_sprite(list, 0, 0x100, 0, 0x1E0, 0, 0, 3);
            list = draw_set_offsets(list, 0x10, 0x100, 0x100, 0x10);
            list = draw_sprite(list, 0x100, 0x100, 0, 0x1F0, 0, 0, 3);
            list = dl_disable(list, 2);
        }
        dl_set_list(list);
    }
}

/* scene command: -1 fades out, -2 fades a black screen in, else loads and shows picture id */
void fpic_cmd(int *arg)
{
    FieldPict *p = &field_pict;
    int id = *arg;

    switch (id) {
    default:
        if (p->state == 0) {
            p->count = 0;
            p->fade_only = 0;
            p->busy = 1;
            fld_load_voice(id, fpic_loaded, 0);
        }
        break;
    case -2:
        if (p->state == 0) {
            p->count = 0;
            p->busy = 1;
            p->fade_only = 1;
            p->state = 1;
        }
        break;
    case -1:
        if (p->state != 0) {
            p->busy = 1;
            p->state = 3;
        }
        break;
    }
}

int fpic_idle(void)
{
    return !field_pict.busy;
}

void fpic_loaded(int *file)
{
    FieldPict *p = &field_pict;

    if ((field_pict.gim = (void *)*file) != 0)
        p->state = 1;
}

void fpic_fade(int in)
{
    FieldPict *p = &field_pict;
    float t;

    if (in != 0)
        t = p->count / 15.0f;
    else
        t = 1.0f - p->count / 15.0f;
    p->color = fld_color_lerp(t, 0xFFFFFF, 0xFFFFFFFF);
}
