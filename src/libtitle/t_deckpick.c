// @range 0x08824DA8-0x0882647C
// @asm 0x08825720-0x08825BBC
// @asm 0x08825D7C-0x088262C8
// @data 0x0883022C-0x08830230
/* Deck choice of the duel setups (state 12 and the ad hoc lobby): the lists of the own deck and the
   saved decks, of the recipes and of the deck files of the extra data, the cursor with L/R to change
   list, the rows with the deck name, owner, number and duel record, and the faster cursor repeat while
   up/down is held (tpad_accel). */
#include "libtitle.h"

void tpt_set(short i, int kind, int type2, int src, int deck);
void tpt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname);
unsigned short *chara_get_name(int set, int id);
unsigned short *recipe_get_name(int n);
void *title_text_frame(void *list, int x, int y, int w, unsigned short *text, int align);
void *tnum_draw(void *list, short x, short y, int n, int step, unsigned short digits, int zeros);
void *tnum_draw_spr(void *list, short x, short y, unsigned int n, short digits, int color);
void tscroll_calc2(short *pos, short *len, int count, int n, int top, int sel, int x, short y, short h);
void *tscroll_draw(void *list, unsigned int color, int x, int y, int h, int pos, int len);
void *tframe_cursor(void *list, unsigned int color, int x, int y, int w, int h);
void font_set_flag4(int on);
unsigned int pad_get_repeat(void);
void pad_set_repeat(int delay, int rate);
void pad_set_repeat_from_fps(void);
int tdl_seat_of(int sub, int i);
DeckName *deck_name_get(int i);

void tdl_decks_build(int type);
int tdl_deck_check(RecipeData *r);
int tdl_deck_get(int type, int list, unsigned int pos, int *out);
int tdl_deck_set(int seat, int type, int list, int pos);
void tpad_accel(int on);
int tdl_deck_input(int type, int *dk, int *deck);
void *tdl_deck_item(void *list, int i, int x, int y);
void *tdl_deck_record(void *list, int x, int y, unsigned short *v);

void tdl_decks_clear(void)
{
    TExtList *e = &title.duel.tail.ext;

    memset(e->saved, 0, sizeof(e->saved));
    memset(e->recipe, 0, sizeof(e->recipe));
    e->nrecipe = 0;
    memset(e->pos, 0, sizeof(e->pos));
    memset(e->row, 0, sizeof(e->row));
    e->type = 0;
    e->u04 = 0;
    e->dk = 0;
    e->deck = 0;
}

void tdl_deck_enter(void)
{
    TPtEnt s;
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    TExtList *e = &q->ext;

    tdl_decks_clear();
    title.step2 = 0;
    title.frame = 0;
    title.u86c = 0;
    title.u86d = 1;
    title.arg = 0;
    title.u874 = 1;
    title.u878 = 0;
    title.u879 = 6;
    tdl_seat_of(q->u001, q->u002);
    tpt_get_ent(&s, q->u003);
    e->type = s.kind;
    switch (e->type) {
    case 1:
    case 0x29:
        if (e->stage == 2) {
            q->u004 = 2;
            t->u86d = 3;
        } else {
            t->u86d = 2;
            q->u004 = 0;
        }
        break;
    default:
        t->u86d = 1;
        q->u004 = 5;
        break;
    }
    title_ehf_load(4, 0, q->u004, q->u004);
    tdl_decks_build(e->type);
}

void tdl_decks_build(int type)
{
    Title *t = &title;
    TExtList *e = &title.duel.tail.ext;
    RecipeData rec;
    RecipeData own;
    RecipeData file;
    DeckName *dn;
    DeckSlot *ds;
    SaveRec *sr;
    int i;
    int n;
    int skip;
    int cnt;
    unsigned short id;

    e->type = type;
    if (type == 1 || type == 0x29) {
        skip = 0;
        n = 0;
        for (i = 1; i < 0x15F; i++) {
            dn = deck_name_get(i);
            if (recipe_get_id(i) != 0) {
                if (dn->set) {
                    recipe_load(i, &rec);
                    e->recipe[n].ok = tdl_deck_check(&rec);
                    e->recipe[n].num = i - skip;
                    e->recipe[n].id = i;
                    e->recipe[n].chara = dn->val;
                    n++;
                    e->nrecipe = n;
                }
            } else {
                skip++;
            }
        }
        for (i = 0; i < 51; i++) {
            if (i == 0) {
                deck_copy_out(&own);
                e->saved[i].ok = tdl_deck_check(&own);
                e->saved[i].id = i;
            } else {
                sr = sysstate_rec_get(i - 1);
                if (sr->used) {
                    e->saved[i].ok = tdl_deck_check(&sr->recipe);
                    e->saved[i].id = i;
                }
            }
        }
        for (i = 0; i < e->loaded; i++) {
            file = e->rec[i].recipe;
            e->file[i].id = i;
            e->file[i].ok = tdl_deck_check(&file);
        }
        t->u874 = 51;
        t->u879 = 6;
    } else {
        ds = deck_slot_get(type);
        cnt = ds->count;
        if (cnt > 6)
            cnt = 6;
        n = 0;
        for (i = 0; i < cnt; i++) {
            id = ds->cards[i];
            if (recipe_get_id(id) != 0) {
                e->recipe[n].id = id;
                e->recipe[n].ok = 1;
                e->recipe[n].num = i + 1;
                n++;
                e->nrecipe = n;
            }
        }
        t->u879 = n;
        t->u874 = e->nrecipe;
    }
}

int tdl_deck_check(RecipeData *r)
{
    RecipeData d = *r;

    return deck_validate(&d, 0);
}

int tdl_deck_default(int id)
{
    TExtList *e = &title.duel.tail.ext;
    int deck = 1;
    int i;

    if (id == 1 || id == 0x29)
        return 0;
    tdl_decks_build(id);
    for (i = 0; i < e->nrecipe; i++) {
        if (e->recipe[i].ok)
            deck = e->recipe[i].id;
    }
    return deck;
}

void tdl_deck_update(void)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    TExtList *e = &q->ext;
    unsigned char seat = q->u003;
    int r;

    switch (r = tdl_deck_input(e->type, &e->dk, &e->deck)) {
    case -1:
        break;
    case -2:
        t->step = 10;
        t->next_sub = q->u001;
        t->next_step = 16;
        t->next_step2 = 0;
        if (t->next_sub == 4)
            t->next_step2 = 1;
        break;
    default:
        tframe_cursor_mode(1);
        tdl_deck_set(seat, e->type, e->dk, r);
        t->step = 10;
        t->next_sub = q->u001;
        t->next_step = 16;
        t->next_step2 = 0;
        if (t->next_sub == 4)
            t->next_step2 = 1;
        break;
    }
}

int tdl_deck_get(int type, int list, unsigned int pos, int *out)
{
    TExtList *e = &title.duel.tail.ext;
    unsigned int id = 0;
    int ok = 0;

    if (type == 1 || type == 0x29) {
        if (list == 0) {
            if (e->saved[pos].ok) {
                id = pos;
                ok = 1;
            }
        } else if (list == 2) {
            if (e->file[pos].ok) {
                id = pos;
                ok = 1;
            }
        } else if (e->recipe[pos].ok) {
            id = e->recipe[pos].id;
            ok = 1;
        }
    } else {
        id = e->recipe[pos].id;
        ok = 1;
    }
    if (ok && out != 0) {
        *out = id;
        return 1;
    }
    return 0;
}

int tdl_deck_set(int seat, int type, int list, int pos)
{
    TFdTail *q = &title.duel.tail;
    TExtList *e = &q->ext;
    int idx = 0;

    if (tdl_deck_get(type, list, pos, &idx) != 0) {
        if (list == 2) {
            memcpy(q->decks[seat].name, e->rec[idx].name, 0x40);
            q->decks[seat].recipe = e->rec[idx].recipe;
            tpt_set_all(seat, -1, -1, list, idx, 0, q->decks[seat].name);
        } else {
            tpt_set(seat, -1, -1, list, idx);
        }
        return idx;
    }
    return -1;
}

void tpad_accel(int on)
{
    TExtList *e = &title.duel.tail.ext;
    unsigned int down = pad_get()->b.down;
    int rate;

    if (e->accel_keys & down) {
        e->accel_keys = down & 0x50;
        rate = 4;
        if (e->accel_t < 120)
            e->accel_t++;
        rate -= e->accel_t / 30;
        if (rate > 0)
            pad_set_repeat((unsigned short)(pad_get_repeat() >> 16), rate);
    } else {
        if (e->accel_keys != 0)
            on = 0;
        e->accel_keys = down & 0x50;
    }
    if (on == 0) {
        e->accel_keys = 0;
        e->accel_t = 0;
        pad_set_repeat_from_fps();
    }
}

#ifdef NON_MATCHING /* the se_play(0x6215) of an empty list branches to the shared return -1 with a nop delay slot */
int tdl_deck_input(int type, int *dk, int *deck)
{
    Title *t = &title;
    TFdTail *q = &title.duel.tail;
    TExtList *e = &q->ext;
    int last = 0;
    int rmax = 0;
    int rowmax = 0;
    int moved = 0;
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    unsigned int rep = pad->b.rep;
    int list;
    int got;

    tpad_accel(1);
    if (title.u86d > 0)
        last = t->u86d - 1;
    if (last != 0) {
        if ((trig & 0x20) || (trig & 0x200) || (trig & 0x8000)) {
            se_play(0x621B);
            e->pos[t->u86c] = t->arg;
            moved = 1;
            e->row[t->u86c] = t->u878;
            if (t->u86c < last)
                t->u86c++;
            else
                t->u86c = 0;
            title_ehf_anim(4, 0, q->u004 + t->u86c, 0);
        } else if ((trig & 0x80) || (trig & 0x100)) {
            se_play(0x621B);
            e->pos[t->u86c] = t->arg;
            moved = 1;
            e->row[t->u86c] = t->u878;
            if (t->u86c != 0)
                t->u86c--;
            else
                t->u86c = last;
            title_ehf_anim(4, 0, q->u004 + t->u86c, 0);
        }
    }
    if (moved && (e->type == 1 || e->type == 0x29)) {
        switch (t->u86c) {
        case 0:
            t->u874 = 51;
            break;
        case 1:
            t->u874 = e->nrecipe;
            break;
        case 2:
            t->u874 = e->loaded;
            break;
        }
        t->arg = e->pos[t->u86c];
        t->u878 = e->row[t->u86c];
        t->u879 = t->u874;
        if (t->u879 > 6)
            t->u879 = 6;
    }
    if (t->u874 > 0 && (rmax = t->u874 - 1) < t->arg)
        t->arg = rmax;
    if (t->u879 > 0 && (rowmax = t->u879 - 1) < t->u878)
        t->arg = rowmax;
    if ((trig & 0x40) || (rep & 0x40)) {
        if (t->arg < rmax) {
            t->arg++;
            se_play(0x6212);
            if (t->u878 < rowmax)
                t->u878++;
            else
                t->u878 = rowmax;
        } else if (trig & 0x40) {
            t->arg = 0;
            t->u878 = 0;
            se_play(0x6212);
        }
    } else if ((trig & 0x10) || (rep & 0x10)) {
        if (t->arg != 0) {
            t->arg--;
            se_play(0x6212);
            if (t->u878 != 0)
                t->u878--;
            else
                t->u878 = 0;
        } else if (trig & 0x10) {
            t->arg = rmax;
            t->u878 = rowmax;
            se_play(0x6212);
        }
    }
    if (trig & region_pick_value(0)) {
        tpad_accel(0);
        if (t->u874 != 0) {
            switch (type) {
            case 1:
            case 0x29:
                list = t->u86c;
                break;
            default:
                list = 1;
                break;
            }
            if (dk != 0)
                *dk = list;
            if (deck != 0 && tdl_deck_get(type, list, t->arg, &got) != 0) {
                se_play(0x6213);
                *deck = got;
                return t->arg;
            }
            se_play(0x6215);
            return -1;
        }
        se_play(0x6215);
    } else if (trig & region_pick_value(1)) {
        tpad_accel(0);
        se_play(0x6214);
        return -2;
    }
    return -1;
}
#endif

void *tdl_deck_draw(void *list)
{
    Title *t = &title;
    int ia = 0x6E;
    int ib = 0x71;
    int i;
    int top;
    int n;
    int y;
    short pos;
    short len;

    if (title.duel.tail.ext.type == 1 || title.duel.tail.ext.type == 0x29) {
        ia = t->u86c + 0x6D;
        ib = t->u86c + 0x70;
    }
    list = title_ehf_draw(list, 4);
    n = t->u879;
    if (t->u874 < n)
        n = t->u874;
    top = t->arg - t->u878;
    if (top <= 0)
        top = 0;
    if (n != 0) {
        for (i = 0; i < n; i++, top++) {
            y = i * 0x1D + 0x38;
            list = title_sprite(list, 1, ia, 0xAA, y);
            list = title_sprite(list, 1, ib, 0x12A, y);
            list = tdl_deck_item(list, top, 0xAA, y);
        }
        tscroll_calc2(&pos, &len, t->u874, 6, t->arg, t->u878, 0x130, 0x41, 0x9D);
        list = tscroll_draw(list, 0xFFFFFFFF, 0x130, 0x41, 0x9D, pos, len);
        list = tframe_cursor(list, 0xFFFFFFFF, 0xA8, t->u878 * 29 + 0x36, 0x88, 0x21);
    }
    return list;
}

#ifdef NON_MATCHING /* the owner (chara) and the recipe id swap places between s8 and the stack */
void *tdl_deck_item(void *list, int i, int x, int y)
{
    Title *t = &title;
    TExtList *e = &title.duel.tail.ext;
    int id = 0;
    int saved = 0;
    int has_rec = 0;
    unsigned short name[32] = { 0 };
    int ok = 1;
    int own = 0;
    int rec = 0;
    int file = 0;
    unsigned char chara = 0;
    unsigned int color = 0xFF000000;
    unsigned int amb;
    unsigned short *text;
    SaveRec *sr;

    if (e->type == 1 || e->type == 0x29) {
        switch (t->u86c) {
        case 0:
            if (i == 0) {
                deck_name_copy_out(name);
                own = 1;
            } else {
                rec = i - 1;
                saved = 1;
                has_rec = 1;
            }
            ok = e->saved[i].ok;
            break;
        case 1:
            id = e->recipe[i].id;
            chara = e->recipe[i].chara;
            ok = e->recipe[i].ok;
            break;
        default:
            file = 1;
            id = e->file[i].id;
            ok = e->file[i].ok;
            break;
        }
    } else {
        id = e->recipe[i].id;
    }
    if (ok != 1) {
        amb = 0;
        list = tmenu_gfx_setup(list, 2);
        if (ok == 2)
            color = 0xFF0070FF;
        else if (ok == 0)
            amb = 0x40000000;
        list = dl_ambient_color(list, amb);
        list = draw_set_offsets(list, 0, 0, 0x84, 0x1D);
        list = draw_sprite(list, x, y, 0, 0, 0, 0, 0);
    }
    if (own) {
        font_set_size(12, 12);
        font_set_color(color);
        font_set_flag4(1);
        list = title_sprite(list, 1, 0x73, 0xAA, 0x38);
        list = title_text_frame(list, x + 16, y + 3, 0x6F, name, 2);
    } else if (saved) {
        sr = sysstate_rec_get(rec);
        if (sr->used) {
            font_set_size(12, 12);
            font_set_color(color);
            font_set_flag4(1);
            if (i != 0)
                list = title_text_frame(list, x + 16, y + 3, 0x6F, sr->name, 2);
            font_set_size(10, 10);
            font_set_color(color);
            font_set_flag4(1);
            if (has_rec)
                list = tdl_deck_record(list, x + 16, y + 16, (unsigned short *)sr->unk44);
        } else {
            unsigned short nodata[12] = { '-', ' ', 'N', 'O', ' ', 'D', 'A', 'T', 'A', ' ', '-', 0 };

            font_set_size(12, 12);
            font_set_color(color);
            font_set_flag4(1);
            list = title_text_frame(list, x + 16, y + 3, 0x6F, nodata, 2);
        }
    } else if (file) {
        font_set_size(12, 12);
        font_set_color(color);
        font_set_flag4(1);
        list = title_text_frame(list, x + 16, y + 3, 0x6F, e->rec[i].name, 2);
    } else {
        text = recipe_get_name(id);
        font_set_size(12, 12);
        font_set_color(color);
        font_set_flag4(1);
        list = title_text_frame(list, x + 16, y + 3, 0x6F, text, 2);
        if (chara != 0) {
            font_text_set(list);
            font_set_size(10, 10);
            font_set_color(color);
            font_set_flag4(1);
            text = chara_get_name(0, chara);
            if (font_measure_text(text) >> 6 > 0x5C)
                text = chara_get_name(1, chara);
            font_draw_wtext((x + 0x15) * 64, (y + 0x10) * 64, 0x1700, text);
            list = font_text_clear();
        }
        list = tnum_draw_spr(list, x, y + 0x11, e->recipe[i].num, 3, -1);
    }
    if (ok == 2 && t->arg == i)
        list = title_msg_font(list, 0, 0xF6, 0x1E0, 0, 0x4D, 3, 15, 15, 0xFFFFFFFF);
    return list;
}
#endif

void *tdl_deck_record(void *list, int x, int y, unsigned short *v)
{
    unsigned short sep[2] = { '/', 0 };

    font_set_size(9, 10);
    list = tnum_draw(list, x + 12, y, v[0] % 100, 6, 2, 1);
    font_text_set(list);
    font_draw_text((x + 12) * 64, y * 64, 0x280, sep);
    list = font_text_clear();
    list = tnum_draw(list, x + 0x1C, y, v[1], 6, 2, 1);
    font_text_set(list);
    font_draw_text((x + 0x1C) * 64, y * 64, 0x280, sep);
    list = font_text_clear();
    list = tnum_draw(list, x + 0x2C, y, v[2], 6, 2, 1);
    list = tnum_draw(list, x + 0x3C, y, v[3], 6, 2, 1);
    sep[0] = ':';
    font_text_set(list);
    font_draw_text((x + 0x3D) * 64, (y - 1) * 64, 0x280, sep);
    list = font_text_clear();
    return tnum_draw(list, x + 0x4E, y, v[4], 6, 2, 1);
}
