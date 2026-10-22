// @range 0x08807460-0x088078D0
// @data 0x0880B650-0x0880B65C
/* Keypad and nixie digits: key layout, cursor moves, EHF animation ranges of key.ehf (30 frames per
   key: press, release, highlight) and nixi.ehf (one range per glyph), excluded card ids. */
#include "libpassword.h"

static unsigned short forbidden_ids[6];

/* cards that have a password but cannot be obtained here (sorted, binary search) */
int card_is_forbidden(unsigned short id)
{
    int lo = 0, hi = 5, mid;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (id == forbidden_ids[mid])
            return 1;
        if (forbidden_ids[mid] < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

static unsigned short forbidden_ids[6] = { 0x143D, 0x1445, 0x1446, 0x19A3, 0x19A4, 0x19A5 };

/* mode 0 highlight, 1 press, 2 flash (intro), 3 still frame; clear/back have a second set of frames
   once the 8 digits are typed */
void key_set_anim(unsigned short key, unsigned short mode)
{
    void *ehf;
    unsigned short base;
    unsigned short first;
    unsigned short last;

    ehf = res_get(RES_KEYPAD);
    if (ehf != 0) {
        base = key * 30;
        if (pw.ndigits >= 8 && (unsigned int)(key - KEY_CLEAR) < 2)
            base += 90;
        switch (mode) {
        case 0:
            first = base + 1;
            last = base + 15;
            break;
        case 1:
            first = base + 16;
            last = base + 29;
            break;
        case 2:
            last = base + 1;
            first = last;
            break;
        case 3:
            last = base;
            first = last;
            break;
        }
        ehf_set_anim(ehf, first, last + 1);
    }
}

int key_anim_done(void)
{
    void *ehf;
    int frame;
    int target, limit;

    ehf = res_get(RES_KEYPAD);
    if (ehf != 0) {
        frame = ehf_get_frame_count(ehf);
        ehf_get_size(ehf, &target, &limit);
        if (frame < limit - 1)
            return 0;
    }
    return 1;
}

/* glyph 0-9 a digit, 10 blank, 12 blinking cursor, 13 lit (lookup) */
void digit_set_anim(unsigned short slot, unsigned short glyph)
{
    void *ehf;
    unsigned short first, last;

    if (slot < 8) {
        ehf = res_get(RES_DIGIT0 + slot);
        if (ehf == 0)
            return;
        switch (glyph) {
        case 0 ... 9:
        case 10:
            last = glyph;
            first = last;
            break;
        case 12:
            first = 12;
            last = 15;
            break;
        case 13:
            first = 16;
            last = 50;
            break;
        default:
            return;
        }
        ehf_set_anim(ehf, first, last + 1);
    }
}

/* row 0 is the enter key, row 1 clear/0/back, rows 2-4 the digits 1-9 */
unsigned char key_at(unsigned short col, unsigned short row)
{
    switch (row) {
    case 0:
        return KEY_ENTER;
    case 1:
        switch (col) {
        case 0:
            return KEY_CLEAR;
        case 1:
            return 0;
        case 2:
            return KEY_BACK;
        default:
            return 0xFF;
        }
    default:
        return col + (row - 2) * 3 + 1;
    }
}

void key_select(unsigned short key)
{
    KeyCursor *c = &pw.cur;

    switch (key) {
    case KEY_ENTER:
        c->row = 0;
        break;
    case KEY_BACK:
        c->col = 2;
        c->row = 1;
        break;
    case 0:
        c->col = 1;
        c->row = 1;
        break;
    case KEY_CLEAR:
        c->col = 0;
        c->row = 1;
        break;
    default:
        c->col = (key - 1) % 3;
        c->row = (key - 1) / 3 + 2;
        break;
    }
    key_set_anim(key, c->anim);
}
