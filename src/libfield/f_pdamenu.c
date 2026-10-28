// @range 0x08824028-0x08825F7C
// @asm 0x08824028-0x0882405C
// @asm 0x088249EC-0x08825218
// @data 0x0883D530-0x0883D630
/* Pages of the PDA (field_pdamenu, the menu opened on the map): the options page, the save prompt, the
   event log with its scrolling voiced reader, the calendar, the files of the PDA pack and the 3D cover
   drawn behind the pages. */
#include "libfield.h"

int cfg_is_mode1(void);
unsigned int cfg_get_byte0_bits4_5(void);
int cfg_get_byte3(int i);
unsigned int cfg_get_bits_1_3_4(void);
unsigned int cfg_get_flag(int which);
void cfg_set_byte0_bit3(int on);
void cfg_set_byte0_bits4_5(int v);
void cfg_set_bits_1_3_4(int v);
void cfg_set_flag(int which, int on);
void snd_set_volume(int channel, int vol);
unsigned short region_pick_value(int flag);
void menu_reset(MenuWidget *w);
void menu_open(MenuWidget *w);
unsigned int bgm_get_status(int id);
void bgm_stop(int id);
void font_set_line_extra(int v);
void font_set_flag8000(int on);
int font_wtext_lines(int w, unsigned short *text);
unsigned int font_sample_glyph_size(void);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short *wcscat(unsigned short *d, const unsigned short *s);
void *model_load(void *data, void **anims);
void model_anim_set(void *m, void *data);
int model_anim_frame_count(void *m);
void mtx_translate(Vec4 *v);
void mtx_scale3(Vec3 *s);
void camera_set(Vec4 *eye, Vec4 *target, Vec4 *up);
void camera_load_view(void);
void *lighting_disable(void *list);
void *dl_offset(void *list, int x, int y);
void *dl_viewport(void *list, int cx, int cy, int w, int h);
void *dl_depth_range(void *list, int near, int far);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_alpha_test(void *list, int func, int ref, int mask);
void *dl_depth_func(void *list, int func);
void *dl_front_face(void *list, int cw);
void *dl_shade_model(void *list, int mode);
void *dl_color(void *list, unsigned int color);
void *dl_fog(float near, float far, void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
/* voice player of the event reader (libfield, outside this file) */
void *fface_new(void *heap, int id);
int fface_ready(void *voice);
void fface_set(void *voice, int a, int b, int part);
void fface_free(void *voice);

void fpda_options_menu(void);
void fpda_options_apply(void);
void fpda_reader_open(void);
int fpda_cal_cell(unsigned char month, unsigned char day);
void *fpda_cover_model(void *pack);
void *fpda_option_msgs(void *pack);

/* message of each option (option_msg_?.bin) */
unsigned short field_pda_opt_msg[16] = { 0x13, 0x14, 0x15, 0x17, 0x16, 0x18, 0x19, 0x1A, 0x1B };
/* highest value of each option */
unsigned char field_pda_opt_max[16] = { 1, 2, 15, 15, 15, 3, 1, 1, 1 };
/* calendar: weekday of the 1st and length of the 4 months */
short field_pda_wday[8] = { 0, -3, -5, -1 };
short field_pda_mdays[4] = { 31, 30, 31, 31 };

#ifdef NON_MATCHING /* os desvios saem com o 0 no delay slot (move v0,zero; b; nop) em outra ordem */
unsigned char fpda_opt_max(int i)
{
    unsigned char v;

    if (i < 0)
        return 0;
    if (i >= 9)
        v = 0;
    else
        v = field_pda_opt_max[i];
    return v;
}
#endif

/* options page: up/down choose, left/right change, the sound volumes apply at once */
void fpda_options_update(void)
{
    PadState *pad = pad_get();
    int d = 0;
    short v;
    unsigned char max;

    if (pad->b.rep & 0x10) {
        field_pdamenu->cursor = (field_pdamenu->cursor + 8) % 9;
        if (cfg_get_language() != 0 && field_pdamenu->cursor == 2)
            field_pdamenu->cursor--;
        menu_reset(&field_pdamenu->menu);
        fpda_options_menu();
        se_play(0x6212);
    }
    if (pad->b.rep & 0x40) {
        field_pdamenu->cursor = (field_pdamenu->cursor + 1) % 9;
        if (cfg_get_language() != 0 && field_pdamenu->cursor == 2)
            field_pdamenu->cursor++;
        menu_reset(&field_pdamenu->menu);
        fpda_options_menu();
        se_play(0x6212);
    }
    if (pad->b.rep & 0x80)
        d = -1;
    if (pad->b.rep & 0x20)
        d = 1;
    if (d != 0) {
        max = field_pda_opt_max[field_pdamenu->cursor];
        v = (short)field_pdamenu->opt[field_pdamenu->cursor];
        v += (short)d;
        if (v < 0)
            v = 0;
        if (v > max)
            v = max;
        switch (field_pdamenu->cursor) {
        case 2:
            snd_set_volume(2, v);
            se_play(0x6218);
            break;
        case 3:
            snd_set_volume(0, v);
            se_play(0x6212);
            break;
        case 4:
            snd_set_volume(1, v);
            se_play(0x6212);
            break;
        default:
            se_play(0x6212);
            break;
        }
        field_pdamenu->opt[field_pdamenu->cursor] = v;
    }
    if (pad->b.trig & region_pick_value(1)) {
        memcpy(field_pdamenu->opt, field_pdamenu->saved, 9);
        fpda_options_apply();
        field_pdamenu->state = 2;
        menu_reset(&field_pdamenu->menu);
        se_play(0x6214);
    } else if (pad->b.trig & region_pick_value(0)) {
        fpda_options_apply();
        field_pdamenu->state = 2;
        menu_reset(&field_pdamenu->menu);
        se_play(0x6213);
    }
}

/* reads the options from the system config */
void fpda_options_load(void)
{
    if (field_pdamenu != 0) {
        if (cfg_is_mode1())
            field_pdamenu->opt[0] = 0;
        else
            field_pdamenu->opt[0] = 1;
        field_pdamenu->opt[1] = cfg_get_byte0_bits4_5();
        if (cfg_get_language() == 0)
            field_pdamenu->opt[2] = cfg_get_byte3(2);
        field_pdamenu->opt[3] = cfg_get_byte3(0);
        field_pdamenu->opt[4] = cfg_get_byte3(1);
        field_pdamenu->opt[5] = cfg_get_bits_1_3_4();
        if (cfg_get_flag(0))
            field_pdamenu->opt[6] = 0;
        else
            field_pdamenu->opt[6] = 1;
        if (cfg_get_flag(2))
            field_pdamenu->opt[7] = 0;
        else
            field_pdamenu->opt[7] = 1;
        if (cfg_get_flag(3))
            field_pdamenu->opt[8] = 0;
        else
            field_pdamenu->opt[8] = 1;
        memcpy(field_pdamenu->saved, field_pdamenu->opt, 9);
    }
}

/* writes the options back to the system config */
void fpda_options_apply(void)
{
    if (field_pdamenu != 0) {
        if (field_pdamenu->opt[0])
            cfg_set_byte0_bit3(0);
        else
            cfg_set_byte0_bit3(1);
        if (field_pdamenu->opt[1] <= field_pda_opt_max[1])
            cfg_set_byte0_bits4_5(field_pdamenu->opt[1]);
        if (cfg_get_language() == 0 && field_pdamenu->opt[2] <= field_pda_opt_max[2])
            snd_set_volume(2, field_pdamenu->opt[2]);
        if (field_pdamenu->opt[3] <= field_pda_opt_max[3])
            snd_set_volume(0, field_pdamenu->opt[3]);
        if (field_pdamenu->opt[4] <= field_pda_opt_max[4])
            snd_set_volume(1, field_pdamenu->opt[4]);
        if (field_pdamenu->opt[5] <= field_pda_opt_max[5])
            cfg_set_bits_1_3_4(field_pdamenu->opt[5]);
        if (field_pdamenu->opt[6])
            cfg_set_flag(0, 0);
        else
            cfg_set_flag(0, 1);
        if (field_pdamenu->opt[7])
            cfg_set_flag(2, 0);
        else
            cfg_set_flag(2, 1);
        if (field_pdamenu->opt[8])
            cfg_set_flag(3, 0);
        else
            cfg_set_flag(3, 1);
    }
}

/* two-choice page: returns 3 or 4 a moment after the choice 0 or 1 is confirmed */
int fpda_choice_update(void)
{
    PadState *pad = pad_get();

    if (field_pdamenu->confirm) {
        if (field_pdamenu->timer++ >= 12) {
            field_pdamenu->confirm = 0;
            field_pdamenu->timer = 0;
            switch (field_pdamenu->cursor) {
            case 0:
                return 3;
            case 1:
                return 4;
            }
        }
        return 0;
    }
    if (pad->b.trig & region_pick_value(1)) {
        field_pdamenu->state = 2;
        se_play(0x6214);
        return 0;
    }
    if (pad->b.trig & region_pick_value(0)) {
        field_pdamenu->confirm = 1;
        field_pdamenu->timer = 0;
        se_play(0x6213);
        return 0;
    }
    if (pad->b.rep & 0x10) {
        field_pdamenu->cursor = (field_pdamenu->cursor + 1) % 2;
        se_play(0x6212);
    }
    if (pad->b.rep & 0x40) {
        field_pdamenu->cursor = (field_pdamenu->cursor + 1) % 2;
        se_play(0x6212);
    }
    return 0;
}

/* event log: a list of 5 visible events, accept opens the reader */
void fpda_events_update(void)
{
    PadState *pad = pad_get();
    int n;

    if (pad->b.rep & 0x10) {
        if (field_pdamenu->sel > 0) {
            field_pdamenu->sel--;
            se_play(0x6212);
        } else if (field_pdamenu->top > 0) {
            field_pdamenu->top--;
            se_play(0x6212);
        }
    }
    if (pad->b.rep & 0x40) {
        n = field_pdamenu->count > 5 ? 5 : field_pdamenu->count;
        if (field_pdamenu->sel < n - 1) {
            field_pdamenu->sel++;
            se_play(0x6212);
        } else if (field_pdamenu->top < field_pdamenu->count - 5) {
            field_pdamenu->top++;
            se_play(0x6212);
        }
    }
    pad = pad_get();
    if (pad->b.trig & region_pick_value(1)) {
        field_pdamenu->state = 2;
        se_play(0x6214);
        return;
    }
    pad = pad_get();
    if (pad->b.trig & region_pick_value(0)) {
        if (field_pdamenu->top + field_pdamenu->sel < field_pdamenu->count) {
            fpda_reader_open();
            field_pdamenu->state = 7;
            se_play(0x6262);
        } else {
            se_play(0x6215);
        }
    }
}

#ifdef NON_MATCHING /* registros a0/a1 trocados na rolagem e s0/s1 trocados entre pad e step */
/* event reader: the text scrolls with the voice (sub 0 wait, 1 playing, 2 free scroll) */
void fpda_reader_update(void)
{
    int quit = 0;
    int lim;
    unsigned int pos;

    switch (field_pdamenu->sub) {
    case 0:
        if (fface_ready(field_pdamenu->voice) == 0)
            return;
        fface_set(field_pdamenu->voice, 1, 1, field_pdamenu->lines[0].part);
        field_pdamenu->sub++;
        break;
    case 1:
        field_pdamenu->acc += field_pdamenu->speed;
        field_pdamenu->frame++;
        if (field_pdamenu->frame >= field_pdamenu->frames) {
            field_pdamenu->pos = field_pdamenu->max;
            field_pdamenu->y = 0x1500;
            field_pdamenu->frame = field_pdamenu->frames;
            fface_set(field_pdamenu->voice, 0, 0, 0);
            field_pdamenu->sub++;
            bgm_stop(0x6262);
        } else if (field_pdamenu->acc + field_pdamenu->speed > 0x2780) {
            lim = 0x1500 - field_pdamenu->lh;
            field_pdamenu->acc = 0;
            field_pdamenu->y += field_pdamenu->lh;
            if (lim < field_pdamenu->y) {
                if ((pos = field_pdamenu->pos + (field_pdamenu->y - lim)) < field_pdamenu->max) {
                    field_pdamenu->pos = pos;
                    field_pdamenu->y = lim;
                }
            }
            if (field_pdamenu->line_end[field_pdamenu->line] <= field_pdamenu->pos + field_pdamenu->y) {
                field_pdamenu->line++;
                fface_set(field_pdamenu->voice, 1, 1, field_pdamenu->lines[field_pdamenu->line].part);
            }
        }
        if (field_pdamenu->y >= 0x1500 - field_pdamenu->lh * 2) {
            if (field_pdamenu->max < field_pdamenu->pos + field_pdamenu->lh / (0x2780 / field_pdamenu->speed)) {
                unsigned short d = field_pdamenu->max - field_pdamenu->pos;

                field_pdamenu->pos = field_pdamenu->max;
                field_pdamenu->y -= d;
            } else {
                field_pdamenu->pos = field_pdamenu->pos + field_pdamenu->lh / (0x2780 / field_pdamenu->speed);
                field_pdamenu->y -= (unsigned short)(field_pdamenu->lh / (0x2780 / field_pdamenu->speed));
            }
        }
        if (pad_get()->b.trig & region_pick_value(0)) {
            field_pdamenu->pos = field_pdamenu->max;
            field_pdamenu->y = 0x1500;
            field_pdamenu->frame = field_pdamenu->frames;
            fface_set(field_pdamenu->voice, 0, 0, 0);
            field_pdamenu->sub = 2;
            bgm_stop(0x6262);
        }
        break;
    case 2: {
        PadState *pad = pad_get();
        unsigned int step = field_pdamenu->lh / 6;

        if (pad->b.trig & region_pick_value(0)) {
            quit = 1;
        } else if (pad->b.dir & 0x10) {
            if (field_pdamenu->pos <= step)
                field_pdamenu->pos = 0;
            else
                field_pdamenu->pos -= step;
        } else if (pad->b.dir & 0x40) {
            field_pdamenu->pos += step;
            if (field_pdamenu->pos > field_pdamenu->max)
                field_pdamenu->pos = field_pdamenu->max;
        }
        break;
    }
    }
    if (pad_get()->b.trig & region_pick_value(1))
        quit = 1;
    if (quit) {
        if (bgm_get_status(0x6262) & 0x21)
            bgm_stop(0x6262);
        if (field_pdamenu->line_end != 0) {
            heap_set_level(field_pdamenu->heap, field_pdamenu->line_end);
            field_pdamenu->line_end = 0;
        }
        if (field_pdamenu->lines != 0) {
            heap_set_level(field_pdamenu->heap, field_pdamenu->lines);
            field_pdamenu->lines = 0;
        }
        fface_free(field_pdamenu->voice);
        field_pdamenu->voice = 0;
        field_pdamenu->state = 6;
        field_pdamenu->sub = 0;
        se_play(0x6214);
    }
}
#endif

#ifdef NON_MATCHING /* o mwcc junta o -0x40 e o -1 de (total - 0x40 + lh - 1); com 1U a ordem das divisoes muda */
/* loads the lines of the chosen event and lays them out */
void fpda_reader_open(void)
{
    FieldPdaEvLine tmp[100];
    FieldPdaEvLine *d;
    FieldPdaEvLine *src;
    FieldPdaEvent *ev;
    int total;
    int n = 0;
    int i;
    unsigned int g;

    ev = &field_pdamenu->events[field_pdamenu->top + field_pdamenu->sel];
    field_pdamenu->voice = fface_new(field_pdamenu->heap, ev->voice & 0x3FF);
    src = field_pda_event_get(ev->id);
    ev->read = 1;
    if (src->text != 0) {
        d = tmp;
        do {
            memcpy(d, src, sizeof(FieldPdaEvLine));
            src++;
            d++;
            n++;
        } while (src->text != 0);
    }
    field_pdamenu->lines = heap_alloc(field_pdamenu->heap, n * 8);
    memcpy(field_pdamenu->lines, tmp, n * 8);
    field_pdamenu->nlines = n;
    field_pdamenu->line_end = heap_alloc(field_pdamenu->heap, n * 4);
    memset(field_pdamenu->line_end, 0, n * 4);
    field_pdamenu->line = 0;
    font_set_size(14, 14);
    font_set_line_extra(0x80);
    if (cfg_get_language() == 0)
        font_set_flag8000(1);
    total = 0;
    for (i = 0; i < n; i++) {
        total += font_wtext_lines(0x2780, field_pdamenu->lines[i].text);
        total += 0x40;
        field_pdamenu->line_end[i] = total;
    }
    g = font_sample_glyph_size();
    field_pdamenu->speed = ((g >> 16) & 0xFFFF) >> 2;
    field_pdamenu->lh = (g & 0xFFFF) + 0x80;
    if (cfg_get_language() == 0)
        font_set_flag8000(0);
    font_set_line_extra(0);
    field_pdamenu->pos = 0;
    if (total - 0x40 < 0x1500)
        field_pdamenu->max = 0;
    else
        field_pdamenu->max = total - 0x1540;
    field_pdamenu->frames = (0x2780 / field_pdamenu->speed) * ((total - 0x40 + (field_pdamenu->lh - 1)) / field_pdamenu->lh);
    field_pdamenu->frame = 0;
    field_pdamenu->acc = 0;
    field_pdamenu->y = 0;
    field_pdamenu->b4 = 0;
    field_pdamenu->sub = 0;
}
#endif

/* calendar: L/R change the month, the d-pad moves the day across the months */
void fpda_calendar_update(void)
{
    PadState *pad = pad_get();
    int day = field_pdamenu->day;
    int moved = 0;
    int month, first, last, over;

    if (pad->b.trig & 0x6000) {
        field_pdamenu->state = 2;
        se_play(0x6214);
        return;
    }
    if (pad->b.trig & 0x100) {
        field_pdamenu->month = (field_pdamenu->month + 3) % 4;
        moved = 1;
    }
    if (pad->b.trig & 0x200) {
        field_pdamenu->month = (field_pdamenu->month + 1) % 4;
        moved = 1;
    }
    if (moved) {
        first = 1 - field_pda_wday[field_pdamenu->month];
        last = first + field_pda_mdays[field_pdamenu->month] - 1;
        if (field_pdamenu->day < first)
            field_pdamenu->day = first;
        else if (last < field_pdamenu->day)
            field_pdamenu->day = last;
        se_play(0x621B);
    } else {
        if (pad->b.rep & 0x10) {
            day -= 7;
            se_play(0x6212);
        }
        if (pad->b.rep & 0x40) {
            day += 7;
            se_play(0x6212);
        }
        if (pad->b.rep & 0x80) {
            day -= 1;
            se_play(0x6212);
        }
        if (pad->b.rep & 0x20) {
            day += 1;
            se_play(0x6212);
        }
        if (day != field_pdamenu->day) {
            month = field_pdamenu->month;
            first = 1 - field_pda_wday[month];
            last = first + field_pda_mdays[month] - 1;
            if (day < first) {
                if (month > 0) {
                    field_pdamenu->month = month - 1;
                    over = first - day;
                    first = 1 - field_pda_wday[field_pdamenu->month];
                    last = first + field_pda_mdays[field_pdamenu->month] - 1;
                    field_pdamenu->day = last - over + 1;
                }
            } else if (last < day) {
                if (month < 3) {
                    field_pdamenu->month = month + 1;
                    over = day - (last + 1);
                    first = 1 - field_pda_wday[field_pdamenu->month];
                    field_pdamenu->day = first + over;
                }
            } else {
                field_pdamenu->day = day;
            }
        }
    }
}

/* day of the calendar cell (row, col) of a month, -1 outside the month */
int fpda_cal_day(unsigned char month, unsigned char row, unsigned char col)
{
    int d = col + (field_pda_wday[month] + row * 7);

    if (d > 0) {
        if (field_pda_mdays[month] < d)
            d = -1;
    } else {
        d = -1;
    }
    return d;
}

/* cell of a day cell index, 0xFF outside the month */
int fpda_cal_cell(unsigned char month, unsigned char day)
{
    int c = day - field_pda_wday[month];
    unsigned int r;

    if (c > 0) {
        if (field_pda_mdays[month] - field_pda_wday[month] < c)
            r = 0xFF;
        else
            r = c & 0xFF;
    } else {
        r = 0xFF;
    }
    return r;
}

unsigned char fpda_cal_mark(unsigned char month, unsigned char day)
{
    unsigned char c = fpda_cal_cell(month, day);

    if (c < 42)
        return *(&field_pdamenu->marks[0][c] + month * 42);
    return 0;
}

/* help window of the options page */
void fpda_options_menu(void)
{
    MenuWidget *w = &field_pdamenu->menu;
    MenuItemDef *d = &w->def;
    unsigned short pad[6] = { ' ', ' ', ' ', ' ', ' ', 0 };

    wcscpy(field_pdamenu->title, pad);
    wcscat(field_pdamenu->title, table_get_entry(fpda_option_msgs(0), 0x1C));
    memset(w, 0, sizeof(MenuWidget));
    d->visible = 1;
    d->style = 0;
    d->x = 0x10F;
    d->y = 0xC5;
    d->max_w = 200;
    d->min_w = 200;
    d->min_h = 0x40;
    d->auto_size = 1;
    d->color = 0xFFFFFFFF;
    d->font_flag4 = 1;
    d->pad_l = 6;
    d->pad_t = 6;
    d->pad_r = 6;
    d->pad_b = 6;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->text = table_get_entry(fpda_option_msgs(0), field_pda_opt_msg[field_pdamenu->cursor]);
    d->show_title = 1;
    d->auto_title = 1;
    d->title = field_pdamenu->title;
    d->style2 = 2;
    d->title_font_size = 12;
    d->title_color = 0xFFFFFFFF;
    w->heap = field_pdamenu->heap;
    w->no_cancel = 1;
    w->no_input = 1;
    menu_open(w);
}

/* YES/NO window of the save prompt, centered */
void fpda_save_menu(void)
{
    MenuWidget *w = &field_pdamenu->menu;
    MenuItemDef *d = &w->def;

    memset(w, 0, sizeof(MenuWidget));
    d->visible = 1;
    d->style = 0;
    d->max_w = 0x140;
    d->min_w = 0xF0;
    d->min_h = 0x5A;
    d->auto_size = 1;
    d->color = 0xFFFFFFFF;
    d->font_flag4 = 1;
    d->pad_l = 10;
    d->pad_t = 8;
    d->pad_r = 10;
    d->pad_b = 0;
    d->font_size = 12;
    d->text_color = 0xFF000000;
    d->text = table_get_entry(fld_gim_get(5), 100);
    d->open_anim = 2;
    d->close_anim = 2;
    w->heap = field_pdamenu->heap;
    w->no_cancel = 1;
    w->buttons = 3;
    w->initial = 3;
    w->cancel_resets = 1;
    w->button = 3;
    menu_open(w);
    d->x = (480 - d->w) / 2;
    d->y = (272 - d->h) / 2;
    field_pdamenu->saving = 1;
}

void *fpda_pack_gim(void *pack)
{
    if (pack == 0)
        return 0;
    return ehp_find(pack, "pda_pack01.gim", 0);
}

/* model of the PDA cover, loaded once */
void *fpda_cover_model(void *pack)
{
    void *tms;
    int *tmt;

    if (field_pda_cover != 0)
        return field_pda_cover;
    if (pack != 0) {
        tms = ehp_find(pack, "gx_pda_cover01.tms", 0);
        tmt = ehp_find(pack, "gx_pda_cover01.tmt", 0);
        if (tmt == 0) {
            field_pda_cover = model_load(tms, 0);
        } else if (*tmt != 0x544D4754) {
            field_pda_cover = model_load(tms, (void **)tmt);
        } else {
            field_pda_cover = model_load(tms, 0);
            model_anim_set(field_pda_cover, tmt);
        }
        return field_pda_cover;
    }
    return 0;
}

void *fpda_event_names(void *pack)
{
    if (field_pda_names != 0)
        return field_pda_names;
    if (pack != 0) {
        field_pda_names = ehp_find(pack, "event_name.bin", 0);
        return field_pda_names;
    }
    return 0;
}

/* option_msg_<lang>.bin */
void *fpda_option_msgs(void *pack)
{
    if (field_pda_msgs != 0)
        return field_pda_msgs;
    if (pack != 0) {
        unsigned char lang[6] = "jegfis";
        char name[256];

        sprintf(name, "option_msg_%c.bin", lang[cfg_get_language()]);
        field_pda_msgs = ehp_find(pack, name, 0);
        return field_pda_msgs;
    }
    return 0;
}

void fpda_cover_free(void)
{
    if (field_pda_cover != 0) {
        model_free(field_pda_cover);
        field_pda_cover = 0;
    }
}

/* scale and camera of the cover */
Vec3 field_pda_cover_scale = { 66.0f, 66.0f, 66.0f };
Vec4 field_pda_cover_eye = { 0.0f, 0.0f, 20.0f, 1.0f };
Vec4 field_pda_cover_target = { 0.0f, 0.0f, 0.0f, 1.0f };
Vec4 field_pda_cover_up = { 0.0f, 1.0f, 0.0f, 0.0f };

/* render state, orthographic projection and camera of the cover */
void fpda_cover_begin(void)
{
    Vec3 scale;
    Vec4 eye;
    Vec4 target;
    Vec4 up;
    void *list;

    mtx_push();
    list = dl_select_layer(4, 0);
    list = lighting_disable(list);
    list = dl_offset(list, 0x710, 0x778);
    list = dl_viewport(list, 0x800, 0x800, 480, 272);
    list = dl_depth_range(list, 0xFFFF, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 4);
    list = dl_scissor(list, 0, 0, 480, 272);
    list = dl_enable(list, 2);
    list = dl_alpha_test(list, 6, 0, 0xFF);
    list = dl_disable(list, 0);
    list = dl_depth_func(list, 7);
    list = dl_enable(list, 1);
    list = dl_front_face(list, 1);
    list = dl_enable(list, 5);
    list = dl_disable(list, 9);
    list = dl_shade_model(list, 1);
    list = dl_enable(list, 8);
    list = dl_color(list, 0xFF0000FF);
    list = dl_fog(50.0f, 100.0f, list, 0x404040);
    list = dl_disable(list, 7);
    list = dl_set_tex_filter(list, 1, 1);
    mtx_set_mode(0);
    mtx_identity();
    mtx_ortho(-240.0f, 240.0f, -136.0f, 136.0f, -10000.0f, 10000.0f);
    scale = field_pda_cover_scale;
    mtx_scale3(&scale);
    mtx_set_mode(1);
    mtx_identity();
    mtx_set_mode(2);
    mtx_identity();
    mtx_set_mode(1);
    up = field_pda_cover_up;
    target = field_pda_cover_target;
    eye = field_pda_cover_eye;
    camera_set(&eye, &target, &up);
    camera_load_view();
    dl_set_list(dl_set_tex_filter(list, 1, 1));
}

/* draws the cover, opened once the PDA is past its opening (cover_x slides it) */
void fpda_cover_draw(void)
{
    Vec3 pos;
    ModelDrawOpts opts;
    float dt[4];
    void *m;

    mtx_set_mode(2);
    memset(dt, 0, sizeof(dt));
    if (field_pdamenu->state < 2)
        dt[0] = 0.0f;
    else
        dt[0] = 1.0f;
    memset(&opts, 0, sizeof(opts));
    opts.a = 4;
    opts.b = 5;
    mtx_identity();
    pos.x = field_pdamenu->cover_x * 0.01532f + -3.36f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    m = fpda_cover_model(0);
    mtx_translate((Vec4 *)&pos);
    model_update(m, dt);
    model_draw(m, &opts);
    mtx_pop();
}

/* sets the cover animation to its last frame (closed) */
void fpda_cover_close(void)
{
    void *m = fpda_cover_model(0);

    if (m != 0)
        *(float *)((char *)m + 0x20) = (float)model_anim_frame_count(m);
}
