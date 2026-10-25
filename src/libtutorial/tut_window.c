// @range 0x088056E0-0x08806BC0
// @rodata 0x0880F760-0x0880F850
// @data 0x0880FA1C-0x0880FA88
/* Message windows of a lesson: nine slots (0..5 talk windows with a bust-up face, typed by the
   text box; 6 question with a list of answers, 7 free note, 8 full-screen board with a picture)
   updated and drawn by a per-slot callback; faces are loaded on demand and drawn in a corner. */
#include "libtutorial.h"

typedef struct TutSlot TutSlot;
typedef void *(*TutSlotFn)(void *list, int i, TutSlot *s);

struct TutSlot {                 /* tut_slots 0x08810388, 0x420 bytes */
    int index;                   /* +0x000 */
    int state;                   /* +0x004 0 idle, 1 requested, 2 face loading, 3 open, 4 opening,
                                    5 shown, 6 waiting, 7 closing, 8 closed */
    unsigned short x;            /* +0x008 note position */
    unsigned short y;            /* +0x00A */
    unsigned char chara;         /* +0x00C face: character */
    unsigned char expr;          /* +0x00D face: expression */
    unsigned short text[0x200];  /* +0x00E expanded message */
    unsigned char pic;           /* +0x40E board picture */
    unsigned char pad40f;
    int unk410;
    int unk414;
    int busy;                    /* +0x418 tut_window_busy */
    TutSlotFn update;            /* +0x41C */
};

extern TutSlot *tut_cur_slot;    /* 0x08810220 */
extern TutSlot tut_slots[9];
extern void *tut_tex[4];         /* 0x08812EDC tuto_duel01, window02, tuto_bigwin01 (+ sprites) */

void font_set_line_extra(int v);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
unsigned short *chara_get_name(int i, int j);
void *gim_set_texture(void *list, void *g);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_set_tex_filter(void *list, int min, int mag);
int wstr_copy(unsigned short *d, int n, const unsigned short *s);
void tut_text_expand(unsigned short *src, unsigned short *dst, int n);
void duel_w624_set_thunk(int v);
void *tut_dl_reset(void *list);
void *tut_draw_tex(void *list, int tex, int u, int v, int w, int h, int x, int y, int sw, int sh,
                   unsigned int color);
void *tut_draw_tex_ex(void *list, int tex, int u, int v, int w, int h, int x, int y, int sw, int sh,
                      unsigned int color, int a, int b);
void tut_text_init(void *heap, int x, int w, int size, int a, int b);
int tut_text_update(void);
void tut_text_draw(void **list);
int tut_text_done(void);
void tut_text_set_icon(void *gim);
void tut_text_set_wait(int on);
void tut_text_set(unsigned short *text, unsigned short *name);
void tut_text_close(void);
int tut_text_typing(void);
int tut_text_anim_busy(void);
void tut_text_place(int x, int y, int w);
int tut_face_ready(TutFace *f);
void *tut_face_render(void *list, TutFace *f, int x, int y);
void tut_face_set(TutFace *f, unsigned int show, unsigned int talk, unsigned int expr);

void *tut_choice_draw_item(void *list, int index, int x, int y);
int tut_windows_start(void);
void tut_talk_start(int i);
int tut_face_find(int chara, int *out);
void *tut_face_draw(void *list, int i);
void *tut_board_pic1(void *list, int x, int y, int w, int h);
void *tut_board_pic2(void *list, int x, int y, int w, int h);
void *tut_board_pic3(void *list, int x, int y, int w, int h);
void *tut_board_pic4(void *list, int x, int y, int w, int h);
void *tut_board_pic5(void *list, int x, int y, int w, int h);
void *tut_board_pic6(void *list, int x, int y, int w, int h);

MenuDrawRectFn tut_board_pics[8] = {
    0, tut_board_pic1, tut_board_pic2, tut_board_pic3, tut_board_pic4, tut_board_pic5, tut_board_pic6, 0,
};

/* window of slots 6 (question), 7 (note) and 8 (board) */
void tut_box_setup(int kind, MenuWidget *w, TutSlot *s)
{
    MenuItemDef *d;

    memset(w, 0, 0x120);
    w->def.visible = 1;
    w->def.style = 0;
    d = &w->def;
    w->def.style2 = 2;
    if (kind == 6) {
        d->style = 0;
        d->style2 = 2;
        d->x = 0x4F;
        d->y = 0x20;
        d->auto_size = 1;
        d->w = 0x140;
        d->h = 0x68;
        d->color = 0xFFFFFFFF;
        d->text = 0;
        d->font_size = 14;
        d->font_flag4 = 1;
        d->pad_l = 10;
        d->pad_r = 10;
        d->pad_t = 0x1C;
        d->pad_b = 8;
        d->text_color = 0xFF000000;
    } else if (kind == 7) {
        d->gim = tut_tex[1];
        d->style = 0;
        d->x = s->x;
        d->y = s->y;
        d->auto_size = 1;
        d->max_w = 0x1DC - s->x;
        d->max_h = 0x10C - s->y;
        d->color = 0xFFFFFFFF;
        d->text = s->text;
        d->font_size = 12;
        d->font_flag4 = 1;
        d->pad_l = 6;
        d->pad_r = 6;
        d->pad_t = 2;
        d->pad_b = 2;
        d->text_color = 0xFF000000;
        if (cfg_get_language() == 0)
            font_set_line_extra(0xC0);
    } else {
        d->style = 0;
        d->x = 0x28;
        d->y = 0x23;
        d->w = 400;
        d->h = 0xC0;
        d->color = 0;
        d->text = s->text;
        d->font_size = 14;
        d->font_flag4 = 1;
        d->pad_l = 12;
        d->pad_r = 4;
        d->pad_t = 12;
        d->pad_b = 4;
        d->text_color = 0xFF000000;
        if (cfg_get_language() == 0)
            d->font_flag8000 = 1;
    }
    d->title = 0;
    d->title_font_size = 14;
    d->title_color = 0xFF000000;
    d->open_anim = 2;
    d->close_anim = 2;
    d->show_title = 0;
    d->auto_title = 1;
    w->heap = tut_heap_get();
    w->no_input = 0;
    w->no_cancel = 1;
    w->cancel_blocked = 1;
    w->keep_open = 0;
    w->f0_4 = 0;
    w->cancel_resets = 0;
    if (kind == 6) {
        w->style = 2;
        w->count = 4;
        w->rows = 4;
        w->arrow_off = 0x88;
        w->item_w = 0x140 - (d->pad_l + d->pad_r);
        w->item_h = 0x10;
        w->draw_item = tut_choice_draw_item;
    } else if (kind == 7) {
        w->no_input = 1;
        w->draw_item = 0;
    } else {
        w->style = 0;
        w->no_input = 1;
        w->draw_item = 0;
        if (s->pic > 0)
            w->draw_all = tut_board_pics[s->pic];
    }
    menu_open(w);
    font_set_line_extra(0);
}

static TutSlot *tut_slot(int i)
{
    return &tut_slots[i];
}

/* the character after the next line break, 0 at the end */
static unsigned short *wstr_next_line(unsigned short *p)
{
    unsigned short *s = p;

    while (*s != '\n') {
        if (*s == 0)
            return 0;
        s++;
    }
    return s + 1;
}

/* question window: line 0 is the question, the next ones the answers */
void *tut_choice_draw_item(void *list, int index, int x, int y)
{
    unsigned short buf[0x100];
    unsigned short *p = tut_slots[6].text;
    int tx = x + 2;
    int ty = y + 1;
    unsigned short *q;
    int i;
    int n;

    if (index == 0) {
        q = wstr_next_line(p);
        if (q == 0)
            return list;
        wstr_copy(buf, q - p, p);
        font_text_set(list);
        font_set_size(14, 14);
        font_set_color(0xFF000000);
        font_set_flag4(1);
        font_draw_wtext(tx << 6, (y - 0x18) << 6, 0x4A00, buf);
        list = font_text_clear();
    }
    n = 0;
    for (i = 0; i <= (unsigned short)index; i++) {
        p = wstr_next_line(p);
        if (p == 0)
            return list;
        q = wstr_next_line(p);
        if (q != 0)
            n = q - p;
        else
            n = 0xFF;
    }
    wstr_copy(buf, n, p);
    font_text_set(list);
    font_set_size(14, 14);
    font_set_color(0xFF000000);
    font_set_flag4(1);
    font_draw_wtext(tx << 6, ty << 6, 0x4A00, buf);
    return font_text_clear();
}

/* talk windows 0..5: face, typing animation, wait for the button */
void *tut_talk_update(void *list, int i, TutSlot *s)
{
    TutFaceRef *f = &tutbox.faces[tutbox.cur_face];

    pad_get();
    region_pick_value(0);
    switch (s->state) {
    case 0:
    case 1:
        break;
    case 2:
        if (tut_face_ready(f->face))
            s->state = 3;
        break;
    case 3:
        s->state = 4;
        tut_face_set(f->face, 1, 1, s->expr);
    case 4:
        if (tut_text_anim_busy() == 0)
            s->state = 5;
    case 5:
        switch (i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            if (tut_text_typing())
                tut_face_set(f->face, 1, 1, s->expr);
            else
                tut_face_set(f->face, 1, 0, s->expr);
            if (tut_text_done()) {
                s->state = 7;
                tut_text_set_wait(0);
            }
        }
        break;
    case 6:
        s->busy = 0;
        break;
    case 7:
        s->busy = 0;
        break;
    case 8:
        if (tut_text_anim_busy() == 0) {
            s->state = 0;
            s->busy = 0;
        }
        break;
    }
    return list;
}

/* windows 6..8 */
void *tut_box_update(void *list, int kind, TutSlot *s)
{
    MenuWidget *w = &tutbox.box;
    int r;

    pad_get();
    region_pick_value(0);
    switch (s->state) {
    case 3:
    case 4:
        tut_box_setup(kind, w, s);
        if (kind == 6 || kind == 8) {
            s->state = 5;
        } else {
            s->state = 6;
            s->busy = 0;
        }
    case 5:
        if (kind == 6) {
            r = menu_update(w);
            switch (r) {
            case -3:
                break;
            case -2:
                s->state = 0;
                s->busy = 0;
                break;
            case -1:
                break;
            default:
                duel_w624_set_thunk(r + 1);
                s->state = 0;
                s->busy = 0;
                break;
            }
        } else if (kind == 8) {
            unsigned int trig = pad_get()->b.trig;

            menu_update(w);
            if (w->state == 3 && (trig & region_pick_value(0)))
                s->state = 7;
        }
        break;
    case 6:
        break;
    case 7:
        if (kind == 7 || kind == 8) {
            menu_finish(w);
            menu_update(w);
            s->state = 8;
        }
        break;
    case 8:
        menu_update(w);
        if (w->state == 6) {
            menu_reset(w);
            if (kind != 8) {
                s->busy = 0;
                s->state = 0;
            } else {
                s->busy = 0;
            }
        }
        break;
    }
    return list;
}

void tut_slot_reset(TutSlot *s)
{
    memset(s->text, 0, 0x400);
    s->state = 0;
    s->unk410 = 0;
    s->unk414 = 0;
    s->busy = 0;
}

void tut_windows_init(void)
{
    int i;
    TutSlot *s;
    void *pack;

    for (i = 0, s = tut_slots; i < 9; i++, s++) {
        tut_slot_reset(s);
        s->index = i;
        s->update = tut_talk_update;
    }
    tut_text_init(tut_heap_get(), 0x27, 400, 14, 1, 3);
    pack = tut_pack;
    if (pack != 0) {
        tut_tex[0] = ehp_find(pack, "tuto_duel01.gim", 0);
        tut_text_set_icon(tut_tex[0]);
        tut_tex[1] = ehp_find(pack, "window02.gim", 0);
        tut_tex[2] = ehp_find(pack, "tuto_bigwin01.gim", 0);
    }
    tut_text_update();
    tut_slots[6].update = tut_box_update;
    tut_slots[7].update = tut_box_update;
    tut_slots[8].update = tut_box_update;
    tut_cur_slot = 0;
}

void tut_windows_term(void)
{
}

/* open the first requested talk window once the text box is idle */
int tut_windows_start(void)
{
    TutBox *b = &tutbox;
    int i;
    TutSlot *s;
    int j;
    TutSlot *t;

    if (tut_text_anim_busy() == 0) {
        for (i = 0, s = tut_slots; i < 6; i++, s++) {
            if (s->state == 1) {
                for (j = 0, t = tut_slots; j <= 5; j++, t++) {
                    if (t->state == 7)
                        t->state = 0;
                }
                switch (i) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    tut_face_find(s->chara, &b->cur_face);
                    tut_talk_start(i);
                    s->state = 2;
                    break;
                case 6:
                case 7:
                    break;
                case 8:
                    tut_talk_start(i);
                    s->state = 3;
                    break;
                }
                return 1;
            }
        }
    }
    return 0;
}

/* place the text box by window and give it the text and the speaker's name */
void tut_talk_start(int i)
{
    TutSlot *s = &tut_slots[i];
    unsigned short *name = 0;

    switch (i) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5: {
        int c = s->chara;

        if (c == 11)
            c = 6;
        name = chara_get_name(0, c);
    }
    }
    switch (i) {
    case 0:
        tut_text_place(0x4F, 0x14, 400);
        break;
    case 1:
        tut_text_place(0x4F, 0x14, 0x105);
        break;
    case 2:
        tut_text_place(0xD8, 0x14, 0x105);
        break;
    case 3:
        tut_text_place(0x4F, 0xD4, 400);
        break;
    case 4:
        tut_text_place(0x4F, 0xD4, 0x105);
        break;
    case 5:
        tut_text_place(0xD8, 0xD4, 0x105);
        break;
    case 6:
    case 7:
        break;
    case 8:
        tut_text_place(0x4F, 0, 400);
        break;
    }
    tut_text_set(s->text, name);
}

int tut_face_find(int chara, int *out)
{
    TutBox *b = &tutbox;
    int i;

    for (i = 0; i < 8; i++) {
        if (b->faces[i].chara == chara) {
            *out = i;
            return 1;
        }
    }
    return 0;
}

/* face of the talk window i in its corner, behind a 64x64 scissor */
void *tut_face_draw(void *list, int i)
{
    TutFaceRef *f = &tutbox.faces[tutbox.cur_face];
    TutFace *face = f->face;
    int x;
    int y;
    int fx;
    int fy;

    if (tut_pack != 0) {
        list = gim_set_texture(list, tut_tex[0]);
        list = tut_dl_reset(list);
        switch (i) {
        case 0:
        case 1:
            x = 4;
            y = 8;
            break;
        case 3:
        case 4:
            x = 4;
            y = 200;
            break;
        case 2:
            x = 0x90;
            y = 8;
            break;
        case 5:
            x = 0x90;
            y = 200;
            break;
        }
        list = draw_set_offsets(list, 0x47, 0x47, 0x47, 0x47);
        list = draw_sprite(list, x, y, 0, 0, 0, 0, 0);
    }
    fx = x + 3;
    fy = y + 3;
    list = dl_scissor(list, fx, fy, 0x40, 0x40);
    list = dl_enable(list, 2);
    list = tut_face_render(list, face, fx - 14, fy);
    return dl_disable(list, 2);
}

void tut_windows_draw(void)
{
    MenuWidget *w = &tutbox.box;
    void *list = 0;
    TutSlot *u;
    int k;
    int i;
    TutSlot *s;
    TutSlot *t;
    int j;

    list = dl_select_layer(8, 0);
    list = tut_dl_reset(list);
    dl_set_list(list);
    tut_windows_start();
    for (i = 0, s = tut_slots; i < 9; i++, s++) {
        if (s->update != 0)
            list = s->update(list, i, s);
    }
    tut_text_update();
    list = dl_select_layer(8, 0);
    for (j = 6, t = &tut_slots[6]; j <= 8; j++, t++) {
        if (t->state >= 3 && t->state < 9) {
            switch (j) {
            case 7:
                if (cfg_get_language() == 0) {
                    font_set_line_extra(0xC0);
                    menu_draw(&list, w);
                    font_set_line_extra(0);
                } else {
                    menu_draw(&list, w);
                }
                break;
            case 8:
                list = tut_draw_tex(list, 2, 0, 0, 0x200, 0x100, 0, 0, 0x200, 0x100, 0xFFFFFFFF);
                list = tut_draw_tex_ex(list, 2, 0, 0x100, 0x100, 0x10, 0x1E0, 0, 0x10, 0x100, 0xFFFFFFFF, 0, 3);
                list = tut_draw_tex_ex(list, 2, 0x100, 0x100, 0xE0, 0x10, 0x1F0, 0x20, 0x10, 0xE0, 0xFFFFFFFF, 0, 3);
            case 6:
                menu_draw(&list, w);
                break;
            }
        }
    }
    tut_text_draw(&list);
    for (k = 0, u = tut_slots; k <= 5; k++, u++) {
        if (u->state >= 3 && u->state < 8)
            list = tut_face_draw(list, k);
    }
    dl_set_list(list);
}

/* face = character * 100 + expression (character 0 = 2) */
void tut_window_open(int slot, unsigned short face, unsigned short x, unsigned short y, unsigned char pic)
{
    TutSlot *s = tut_slot(slot);

    tut_slot_reset(s);
    switch (slot) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        tut_text_set_wait(1);
        s->chara = face / 100;
        s->expr = face % 100;
        if (s->chara == 0)
            s->chara = 2;
        s->busy = 1;
        s->state = 1;
        break;
    case 8:
        s->pic = pic;
    case 6:
    case 7:
    default:
        s->x = x;
        s->y = y;
        s->chara = 0;
        s->expr = 0;
        s->busy = 1;
        s->state = 3;
        break;
    }
    tut_cur_slot = s;
}

/* slot < 2 closes the player's talk windows, < 5 the opponent's */
void tut_window_close(int slot)
{
    int closed = 0;
    int i;
    TutSlot *s;

    if (slot < 2) {
        for (i = 0; i <= 2; i++) {
            s = tut_slot(i);
            if (s->state != 0) {
                s->state = 0;
                closed = 1;
            }
        }
        tut_text_close();
    } else if (slot < 5) {
        for (i = 3; i <= 5; i++) {
            s = tut_slot(i);
            if (s->state != 0) {
                s->state = 0;
                closed = 1;
            }
        }
    } else {
        s = tut_slot(slot);
        if (s->state != 0) {
            s->busy = 0;
            if (slot != 8)
                s->state = 7;
            else
                s->state = 0;
        }
    }
    if (closed) {
        tut_text_close();
        tut_text_set_wait(1);
    }
    tut_cur_slot = 0;
}

void tut_window_set_text(int slot, unsigned short *text)
{
    TutSlot *s = tut_slot(slot);

    if (text != 0)
        tut_text_expand(text, s->text, 0x200);
    else
        memset(s->text, 0, 0x400);
    tut_cur_slot = s;
}

int tut_window_busy(int slot)
{
    return tut_slot(slot)->busy;
}

void *tut_board_pic(void *list, int x, int y, int w, int h, int n);

void *tut_board_pic1(void *list, int x, int y, int w, int h)
{
    return tut_board_pic(list, x, y, w, h, 1);
}

void *tut_board_pic2(void *list, int x, int y, int w, int h)
{
    return tut_board_pic(list, x, y, w, h, 2);
}

void *tut_board_pic3(void *list, int x, int y, int w, int h)
{
    return tut_board_pic(list, x, y, w, h, 3);
}

void *tut_board_pic4(void *list, int x, int y, int w, int h)
{
    return tut_board_pic(list, x, y, w, h, 4);
}

void *tut_board_pic5(void *list, int x, int y, int w, int h)
{
    return tut_board_pic(list, x, y, w, h, 5);
}

void *tut_board_pic6(void *list, int x, int y, int w, int h)
{
    return tut_board_pic(list, x, y, w, h, 6);
}

/* tuto_duelimg<n>.gim centered in the board (256 x 92, picture 1 is 114 high) */
void *tut_board_pic(void *list, int x, int y, int w, int h, int n)
{
    char name[0x100];
    void *pack = tut_pack;
    int ph;

    if (pack == 0)
        return list;
    sprintf(name, "tuto_duelimg%02d.gim", n);
    pack = ehp_find(pack, name, 0);
    if (pack == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, pack);
    ph = 0x5C;
    if (n == 1)
        ph += 0x16;
    list = draw_set_offsets(list, 0x100, ph, 0x100, ph);
    return draw_sprite(list, x + (w - 0x100) / 2, y + (h - ph) / 2, 0, 0, 0, 0, 0);
}
