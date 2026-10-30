// @range 0x088649D4-0x08864B80
/* Input of the scrolling list widget: cancel/accept buttons (swapped by region) set the result,
   up/down move the cursor with wrap-around. */
#include "libduel_draw.h"

void listw_input_reset(DdList *l)
{
    l->result = 0;
}

void listw_input(DdList *l)
{
    PadState *pad;

    if (l->result != 2 && l->result != 1 && l->active) {
        pad = pad_get();
        if (pad->b.trig & region_pick_value(0)) {
            if (!l->no_cancel) {
                l->result = 1;
                se_play(0x6213);
            }
        } else if (pad->b.trig & region_pick_value(1)) {
            if (!l->no_accept) {
                l->result = 2;
                se_play(0x6214);
            }
        } else if (!l->no_move) {
            if (pad->b.rep & 0x40) {
                if (l->cursor < l->count - 1)
                    l->cursor++;
                else
                    l->cursor = 0;
                se_play(0x6212);
            }
            if (pad->b.rep & 0x10) {
                if (l->cursor > 0)
                    l->cursor--;
                else
                    l->cursor = l->count - 1;
                se_play(0x6212);
            }
        }
    }
}

void listw_term(DdList *l)
{
}

void listw_set_active(DdList *l, int on)
{
    l->active = on;
}

int listw_cursor(DdList *l)
{
    return l->cursor;
}

void listw_set_cursor(DdList *l, int c)
{
    l->cursor = c;
}
