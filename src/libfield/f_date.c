// @range 0x088274B4-0x08827C44
// @asm 0x088275FC-0x08827788
// @rodata 0x088398E4-0x08839924
// @data 0x0883D650-0x0883D67C
/* The date banner of a new day (field_date): the date of the story ("July 3 (Mon)", "3rd July (Mon)"
   in the western versions) over a black screen, with an optional second line, fading out after a
   second. */
#include "libfield.h"

FieldStory *fld_story_get(void);
unsigned int fld_color_lerp(unsigned int a, unsigned int b, float t);
void *fld_dl_clear(void *list, short z, unsigned int color);
int font_measure_text(unsigned short *text);
void font_set_flag8000(int on);
unsigned short *wtext_from_int(int v, unsigned short *out);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short *wcscat(unsigned short *d, const unsigned short *s);

void fdate_nop(void);
void *fdate_text(void *list, unsigned short *text, int size, int x, int y, unsigned int color,
                 unsigned int shadow);
void fdate_text_jp(void);
void fdate_text_us(void);
extern unsigned short field_date_space[2];

/* offsets of the 8 copies drawn in the shadow color around the text */
const FieldOff field_date_outline[8] = {
    { 0, -2 }, { 1, -1 }, { 2, 0 }, { 1, 1 }, { 0, 2 }, { -1, 1 }, { -2, 0 }, { -1, -1 },
};

void fdate_clear(void)
{
    memset(&field_date, 0, sizeof(field_date));
}

void fdate_term(void)
{
    fdate_nop();
    fdate_clear();
}

void fdate_nop(void)
{
}

void fdate_update(void)
{
    FieldDate *d = &field_date;
    float t;

    switch (field_date.state) {
    case 1:
        if (d->time < 30) {
            d->time++;
        } else {
            d->state = 2;
            d->time = 0;
        }
        break;
    case 2:
        if (d->time < 30) {
            t = (float)d->time / 30.0f;
            d->time++;
            d->bg = fld_color_lerp(0xFF000000, 0, t);
            d->shadow = fld_color_lerp(0xFF000000, 0, t);
            d->color = fld_color_lerp(0xFFFFFFFF, 0xFFFFFF, t);
        } else {
            d->time = 0;
            d->state = 0;
        }
        break;
    }
}

#ifdef NON_MATCHING /* os registros $f0/$f1 do centro do texto saem trocados (240 - largura * 0,5) */
void fdate_draw(void)
{
    FieldDate *d = &field_date;
    void *list;
    int sub;
    unsigned short *text;

    if (field_date.state != 0) {
        list = dl_select_layer(4, 0);
        list = fld_dl_clear(list, 0, d->bg);
        dl_set_list(list);
        list = dl_select_layer(4, 0);
        font_set_flag4(0);
        font_set_size(22, 22);
        font_set_flag8000(0);
        list = fdate_text(list, d->text, 22,
                          (int)(240.0f - (float)(font_measure_text(d->text) >> 6) * 0.5f), 110,
                          d->color, d->shadow);
        font_set_size(15, 15);
        font_set_flag8000(cfg_get_language() == 0);
        sub = d->sub;
        if (sub != -1) {
            text = table_get_entry(fld_gim_get(7), sub);
            list = fdate_text(list, text, 15,
                              (int)(240.0f - (float)(font_measure_text(text) >> 6) * 0.5f), 150,
                              d->color, d->shadow);
        }
        dl_set_list(list);
    }
}
#endif

/* shows the date of the story, with the text sub of the message table under it (-1 none) */
void fdate_start(int sub)
{
    field_date.state = 1;
    field_date.bg = 0xFF000000;
    field_date.shadow = 0xFF000000;
    field_date.color = 0xFFFFFFFF;
    field_date.time = 0;
    field_date.sub = sub;
    switch (region_get()) {
    case 0:
        fdate_text_jp();
        break;
    case 1:
        fdate_text_us();
        break;
    }
}

int fdate_done(void)
{
    return field_date.state == 0;
}

/* text at (x, y) with an outline in the shadow color */
void *fdate_text(void *list, unsigned short *text, int size, int x, int y, unsigned int color,
                 unsigned int shadow)
{
    int i;
    const FieldOff *o;

    font_text_set(list);
    font_set_size(size, size);
    font_set_color(shadow);
    for (i = 0, o = field_date_outline; i < 8; i++, o++)
        font_draw_text((x + o->x) * 64, (y + o->y) * 64, (480 - (x + o->x)) * 64, text);
    font_set_color(color);
    font_draw_text(x << 6, y << 6, (480 - x) * 64, text);
    return font_text_clear();
}

/* "<month>月<day>日 (<weekday>)" */
void fdate_text_jp(void)
{
    FieldStory *st = fld_story_get();
    unsigned short month[4];
    unsigned short day[4];
    unsigned short open[2] = { '(', 0 };
    unsigned short close[2] = { ')', 0 };

    wtext_from_int(*(int *)st->w[0], month);
    wtext_from_int(*(int *)st->w[1], day);
    wcscpy(field_date.text, month);
    wcscat(field_date.text, table_get_entry(fld_gim_get(5), 0x51));
    wcscat(field_date.text, day);
    wcscat(field_date.text, table_get_entry(fld_gim_get(5), 0x50));
    wcscat(field_date.text, field_date_space);
    wcscat(field_date.text, open);
    wcscat(field_date.text, table_get_entry(fld_gim_get(5), *(int *)st->w[2] + 0x50));
    wcscat(field_date.text, close);
}

unsigned short field_date_space[2] = { ' ', 0 };

/* "<day><st/nd/rd/th> <month> (<weekday>)" */
void fdate_text_us(void)
{
    FieldDate *d = &field_date;
    FieldStory *st = fld_story_get();
    unsigned short day[4];
    unsigned short open[2] = { '(', 0 };
    unsigned short close[2] = { ')', 0 };
    unsigned short sfx[4][3] = { { 's', 't', 0 }, { 'n', 'd', 0 }, { 'r', 'd', 0 }, { 't', 'h', 0 } };
    int month = *(int *)st->w[0] + 0x47; /* message of the month name */

    wtext_from_int(*(int *)st->w[1], day);
    wcscpy(d->text, day);
    switch (*(int *)st->w[1]) {
    case 11:
    case 12:
    case 13:
        wcscat(d->text, sfx[3]);
        break;
    default:
        switch (*(int *)st->w[1] % 10) {
        case 1:
            wcscat(d->text, sfx[0]);
            break;
        case 2:
            wcscat(d->text, sfx[1]);
            break;
        case 3:
            wcscat(d->text, sfx[2]);
            break;
        default:
            wcscat(d->text, sfx[3]);
            break;
        }
        break;
    }
    wcscat(d->text, field_date_space);
    wcscat(d->text, table_get_entry(fld_gim_get(5), month));
    wcscat(d->text, field_date_space);
    wcscat(d->text, open);
    wcscat(d->text, table_get_entry(fld_gim_get(5), *(int *)st->w[2] + 0x50));
    wcscat(d->text, close);
}
