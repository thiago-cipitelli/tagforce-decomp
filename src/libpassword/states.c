// @range 0x0880597C-0x08807460
// @rodata 0x0880B460-0x0880B484
/* Password screen scenes: intro message, keypad input (8 digits), digit reveal and lookup
   (card_id_from_password), card found (name typed out, card added to the collection), not found,
   quit. Messages come from strTbl_<lang>.bin and are typed out one glyph every 2 frames. */
#include "libpassword.h"


void keypad_reset(void)
{
    pw.ndigits = 0;
    pw.cur.anim = 3;
    key_select(0);
    digits_refresh();
}

/* typed digits, then the blinking cursor tube, then blank tubes */
void digits_refresh(void)
{
    int i;
    unsigned char n;

    for (i = 0; i < 8; i++) {
        n = pw.ndigits;
        if (i < n)
            digit_set_anim(i, pw.digits[i]);
        else if (i == n)
            digit_set_anim(n, GLYPH_CURSOR);
        else
            digit_set_anim(i, GLYPH_BLANK);
    }
}

/* scene 1: intro message; keys flash at random while it types out */
void st_intro(void)
{
    RandSeed *r;

    switch (pw.step) {
    case 0:
        wcscpy(pw.text, table_get_entry(res_get(RES_STRINGS), 1));
        pw.text_len = wtext_count_tags(pw.text);
        pw.text_pos = 0;
        pw.typing = 1;
        pw.timer = 0;
        pw.step = 1;
        rand3_seeds[rand3_slot].lcg = 3;
        break;
    case 1:
        if (pad_get()->b.trig & 0x6000) {
            pw.text_pos = pw.text_len;
            digits_refresh();
            pw.cur.anim = 0;
            key_set_anim(key_at(pw.cur.col, pw.cur.row), 0);
            pw.scene = 2;
            bgm_stop(0x627E);
            pw.timer = 0;
            pw.text_skip = 1;
            pw.typing = 0;
        } else {
            if (!pw.text_skip) {
                r = &rand3_seeds[rand3_slot];
                r->lcg *= 0x41C64E6D;
                r->lcg += 0x3039;
                key_set_anim((int)(r->lcg >> 16 & 0x7FFF) % 12, 2);
            }
            if (++pw.timer >= 2) {
                if (++pw.text_pos >= pw.text_len) {
                    digits_refresh();
                    pw.cur.anim = 0;
                    key_set_anim(key_at(pw.cur.col, pw.cur.row), 0);
                    pw.scene = 2;
                    bgm_stop(0x627E);
                    pw.text_skip = 1;
                    pw.typing = 0;
                }
                pw.timer = 0;
            }
        }
        break;
    }
}


/* scene 2: keypad. Circle/cross (region dependent) press the key under the cursor or quit,
   square deletes, the d-pad moves (only the enter row once 8 digits are typed) */
void st_input(void)
{
    PadState *pad;
    KeyCursor *c;
    unsigned char key;
    unsigned int trig;
    int ok;
    int i;
    KeyCursor prev;

    pad = pad_get();
    c = &pw.cur;
    key = key_at(c->col, c->row);
    memcpy(&prev, c, 2);
    if (c->anim == 1) {
        if (key_anim_done()) {
            c->anim = 0;
            key_set_anim(key, 0);
        }
    }
    trig = pad->b.trig;
    if (trig & region_pick_value(0)) {
        ok = 1;
        switch (key) {
        case KEY_ENTER:
            for (i = 0; i < 8; i++)
                digit_set_anim(i, GLYPH_CURSOR);
            pw.step = 0;
            pw.timer = 0;
            pw.scene = 3;
            se_play(0x6282);
            break;
        case KEY_BACK:
            if (pw.ndigits == 0) {
                ok = 0;
                break;
            }
            digit_set_anim(pw.ndigits, GLYPH_BLANK);
            pw.ndigits--;
            digit_set_anim(pw.ndigits, GLYPH_CURSOR);
            se_play(0x6281);
            break;
        case KEY_CLEAR:
            if (pw.ndigits == 0) {
                ok = 0;
                break;
            }
            pw.ndigits = 0;
            digit_set_anim(0, GLYPH_CURSOR);
            for (i = 1; i < 8; i++)
                digit_set_anim(i, GLYPH_BLANK);
            se_play(0x6280);
            break;
        default:
            digit_set_anim(pw.ndigits, key);
            pw.digits[pw.ndigits++] = key;
            digit_set_anim(pw.ndigits, GLYPH_CURSOR);
            se_play(0x627F);
            if (pw.ndigits >= 8) {
                c->row = 0;
                c->col = c->col & 0xFE;
            }
            break;
        }
        if (ok) {
            c->anim = 1;
            key_set_anim(key, 1);
            return;
        }
    } else if (trig & region_pick_value(1)) {
        pw.step = 0;
        pw.timer = 0;
        pw.scene = 6;
        key_set_anim(key, 3);
    } else if (trig & 0x8000) {
        if (pw.ndigits > 0) {
            c->anim = 1;
            key_set_anim(KEY_BACK, 1);
            if (pw.ndigits >= 8 && key == KEY_ENTER)
                key_select(0);
            digit_set_anim(pw.ndigits, GLYPH_BLANK);
            pw.ndigits--;
            digit_set_anim(pw.ndigits, GLYPH_CURSOR);
            return;
        }
    } else {
        trig = pad->b.rep;
        if (trig & 0x10) {
            if (pw.ndigits >= 8)
                c->row = (c->row + 1) & 1;
            else if (c->row == 4)
                c->row = 1;
            else
                c->row++;
        } else if (trig & 0x40) {
            if (pw.ndigits >= 8)
                c->row = (c->row + 1) & 1;
            else if (c->row == 1)
                c->row = 4;
            else
                c->row--;
        } else if (trig & 0x80) {
            if (pw.ndigits >= 8) {
                if (c->row == 1)
                    c->col = (c->col + 2) & 3;
            } else {
                c->col = (c->col + 2) % 3;
            }
        } else if (trig & 0x20) {
            if (pw.ndigits >= 8) {
                if (c->row == 1)
                    c->col = (c->col + 2) & 3;
            } else {
                c->col = (c->col + 1) % 3;
            }
        }
    }
    if (prev.col != c->col || prev.row != c->row) {
        key = key_at(c->col, c->row);
        c->anim = 0;
        key_set_anim(key, c->anim);
    }
}

/* scene 3: the digits light up one by one, then the password is looked up; a card is accepted
   if its id is in range, flag31 is set and it is not one of the excluded ids */
void st_check(void)
{
    KeyCursor *c = &pw.cur;
    int pass;
    int id;
    int i;

    if (c->anim == 1 && key_anim_done()) {
        c->anim = 0;
        key_set_anim(0, 3);
    }
    switch (pw.step) {
    case 0:
        if (++pw.timer >= 30) {
            wcscpy(pw.text, table_get_entry(res_get(RES_STRINGS), 12));
            pw.text_len = wtext_count_tags(pw.text);
            pw.text_pos = 0;
            pw.timer = 0;
            pw.step = 1;
            pw.typing = 1;
            se_play(0x6283);
        }
        break;
    case 1:
        if (pad_get()->b.trig & 0x6000) {
            pw.text_pos = pw.text_len;
            pw.step++;
            pw.timer = 0;
            pw.typing = 0;
        } else if (++pw.timer >= 2) {
            if (++pw.text_pos >= pw.text_len) {
                pw.step++;
                pw.typing = 0;
            }
            pw.timer = 0;
        }
        break;
    case 2 ... 9:   /* light the digits one by one */
        if (++pw.timer >= 20) {
            digit_set_anim(pw.step - 2, pw.digits[pw.step - 2]);
            pw.step++;
            pw.timer = 0;
        }
        break;
    case 10:
        bgm_stop(0x6283);
        pass = 0;
        for (i = 0; i < 8; i++) {
            pass *= 10;
            pass += pw.digits[i];
        }
        id = card_id_from_password(pass);
        if (id >= 0xFA7 && id <= 0x1B2D && card_get_flag31(id) && !card_is_forbidden(id)) {
            pw.card_id = id;
            cip_read_card(pw.cip, id, inv_get_hi(id));
            pw.scene = 4;
        } else {
            pw.scene = 5;
        }
        pw.password = pass;
        pw.step = 0;
        pw.timer = 0;
        break;
    }
}

/* scene 4: "password <n>" message, the card name typed out, the card image, then the second message
   and the help lines; the card goes into the collection (inv_set_bit0) */
int st_found(void)
{
    unsigned short msg[0x400];
    unsigned short name[0x400];
    int i;
    unsigned short ch;

    switch (pw.step) {
    case 0:
        if (++pw.timer >= 30) {
            wsprintf(msg, table_get_entry(res_get(RES_STRINGS), 3), pw.password);
            wcscpy(pw.text, msg);
            pw.text_len = wtext_count_tags(pw.text);
            pw.text_pos = 0;
            pw.timer = 0;
            pw.step = 1;
            pw.typing = 1;
        }
        break;
    case 1:
        if (++pw.timer >= 2) {
            if (++pw.text_pos >= pw.text_len) {
                pw.step = 2;
                wtext_strip_tags(name, card_get_name(pw.card_id));
                wsprintf(pw.name_full, table_get_entry(res_get(RES_STRINGS), 2), name);
                memset(pw.name_shown, 0, sizeof(pw.name_shown));
                pw.name_pos = 0;
                se_play(0x6284);
            }
            pw.timer = 0;
        }
        break;
    case 2:
        if (++pw.timer >= 2) {
            i = pw.name_pos;
            ch = pw.name_full[i];
            if (ch != 0) {
                pw.name_shown[i] = ch;
                pw.name_pos++;
                if (ch == '$') {   /* a 4-glyph tag: copied whole */
                    memcpy(&pw.name_shown[i + 1], &pw.name_full[i + 1], 6);
                    pw.name_pos += 3;
                }
            } else {
                pw.step = 3;
                pw.typing = 0;
            }
            pw.timer = 0;
        }
        break;
    case 3:
        if (++pw.timer >= 60) {
            pw.step = 4;
            pw.timer = 0;
            se_play(0x6286);
        }
        break;
    case 4:
        pw.timer++;
        if (pw.timer > 120 && ((pad_get()->b.trig & 0x6000) || pw.timer > 600)) {
            inv_set_bit0(pw.card_id);
            pw.step = 5;
            pw.timer = 0;
        }
        break;
    case 5:
        if (++pw.timer >= 120) {
            wcscpy(pw.text2, table_get_entry(res_get(RES_STRINGS), 4));
            pw.text_len = wtext_count_tags(pw.text2);
            pw.text2_pos = 0;
            pw.step = 6;
            pw.timer = 0;
            pw.typing = 1;
        }
        break;
    case 6:
        if (pad_get()->b.trig & 0x6000) {
            pw.text2_pos = pw.text_len;
            pw.step = 7;
            pw.typing = 0;
            pw.timer = 0;
        } else if (++pw.timer >= 2) {
            if (++pw.text2_pos >= pw.text_len) {
                pw.step = 7;
                pw.typing = 0;
            }
            pw.timer = 0;
        }
        break;
    case 7:
        if (pad_get()->b.trig & region_pick_value(1))
            return 1;
        if (pad_get()->b.trig & region_pick_value(0)) {
            pw.timer = 0;
            pw.step = 8;
        }
        break;
    case 8:
        if (++pw.timer >= 30) {
            pw.text_pos = 0;
            pw.timer = 0;
            pw.step = 0;
            pw.scene = 1;
            keypad_reset();
            memset(pw.text, 0, sizeof(pw.text));
        }
        break;
    }
    return 0;
}

/* scene 5: no card for this password */
int st_not_found(void)
{
    unsigned short msg[0x400];

    switch (pw.step) {
    case 0:
        if (++pw.timer >= 30) {
            wsprintf(msg, table_get_entry(res_get(RES_STRINGS), 10), pw.password);
            wcscpy(pw.text, msg);
            pw.text_len = wtext_count_tags(pw.text);
            pw.text_pos = 0;
            pw.timer = 0;
            pw.step = 1;
            pw.typing = 1;
        }
        break;
    case 1:
        if (++pw.timer >= 2) {
            if (++pw.text_pos >= pw.text_len) {
                pw.step = 2;
                wcscpy(pw.text2, table_get_entry(res_get(RES_STRINGS), 11));
                pw.text_len = wtext_count_tags(pw.text2);
                pw.text2_pos = 0;
                se_play(0x6285);
            }
            pw.timer = 0;
        }
        break;
    case 2:
        if (pad_get()->b.trig & 0x6000) {
            pw.text2_pos = pw.text_len;
            pw.step = 3;
            pw.timer = 0;
            pw.typing = 0;
        }
        if (++pw.timer >= 2) {
            if (++pw.text2_pos >= pw.text_len) {
                pw.step = 3;
                pw.typing = 0;
            }
            pw.timer = 0;
        }
        break;
    case 3:
        if (pad_get()->b.trig & region_pick_value(1))
            return 1;
        if (pad_get()->b.trig & region_pick_value(0)) {
            pw.timer = 0;
            pw.step = 4;
        }
        break;
    case 4:
        if (++pw.timer >= 30) {
            pw.text_pos = 0;
            pw.timer = 0;
            pw.step = 0;
            pw.scene = 1;
            keypad_reset();
            memset(pw.text, 0, sizeof(pw.text));
        }
        break;
    }
    return 0;
}

/* scene 6: quit confirmation */
int st_quit(void)
{
    switch (pw.step) {
    case 0:
        if (++pw.timer >= 30) {
            wcscpy(pw.text, table_get_entry(res_get(RES_STRINGS), 9));
            pw.text_len = wtext_count_tags(pw.text);
            pw.text_pos = 0;
            pw.timer = 0;
            pw.step = 1;
            pw.typing = 1;
        }
        break;
    case 1:
        if (pad_get()->b.trig & 0x6000) {
            pw.text_pos = pw.text_len;
            pw.step = 2;
            pw.timer = 0;
            pw.typing = 0;
        } else if (++pw.timer >= 2) {
            if (++pw.text_pos >= pw.text_len) {
                pw.step = 2;
                pw.typing = 0;
            }
            pw.timer = 0;
        }
        break;
    case 2:
        if (pad_get()->b.trig & region_pick_value(0))
            return 1;
        if (pad_get()->b.trig & region_pick_value(1)) {
            pw.timer = 0;
            pw.step = 3;
        }
        break;
    case 3:
        if (++pw.timer >= 30) {
            pw.text_pos = 0;
            pw.timer = 0;
            pw.step = 0;
            pw.scene = 1;
            digits_refresh();
            memset(pw.text, 0, sizeof(pw.text));
        }
        break;
    }
    return 0;
}
