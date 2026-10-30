// @range 0x08814DFC-0x08815678
// @asm 0x08814E38-0x088153E4
// @data 0x08875770-0x088757D4
/* The field cursor: a packed location (player, zone, index) over a 7x7 grid of the field zones
   (hand row, monster/spell rows, deck, grave, ...), moved with the d-pad; zone 11 (the hand) walks
   its cards by index. */
#include "dd_field_ui.h"

extern DdCursor dd_cursor;             /* 0x08C54640 */

/* grid[row][col] of field locations */
DdLoc dd_cursor_grid[7][7] = {
    { { 1, 11, 6 }, { 1, 11, 5 }, { 1, 11, 4 }, { 1, 11, 3 }, { 1, 11, 2 }, { 1, 11, 1 }, { 1, 11, 0 } },
    { { 1, 13, 0 }, { 1, 9, 0 }, { 1, 8, 0 }, { 1, 7, 0 }, { 1, 6, 0 }, { 1, 5, 0 }, { 1, 12, 0 } },
    { { 1, 14, 0 }, { 1, 4, 0 }, { 1, 3, 0 }, { 1, 2, 0 }, { 1, 1, 0 }, { 1, 0, 0 }, { 1, 10, 0 } },
    { { 1, 15, 0 }, { 0, 16, 0 }, { 0, 16, 0 }, { 0, 16, 0 }, { 0, 16, 0 }, { 0, 16, 0 }, { 0, 15, 0 } },
    { { 0, 10, 0 }, { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 3, 0 }, { 0, 4, 0 }, { 0, 14, 0 } },
    { { 0, 12, 0 }, { 0, 5, 0 }, { 0, 6, 0 }, { 0, 7, 0 }, { 0, 8, 0 }, { 0, 9, 0 }, { 0, 13, 0 } },
    { { 0, 11, 0 }, { 0, 11, 1 }, { 0, 11, 2 }, { 0, 11, 3 }, { 0, 11, 4 }, { 0, 11, 5 }, { 0, 11, 6 } },
};

int func_0880B770(int player, int zone);
int dd_cursor_nearest_slot(int player, DdCursor *c);
int dd_cursor_nearest_hand(int player, DdCursor *c);
int func_08866884(int player);
int func_0886689C(int player);
void dd_cursor_set(int player, int zone, int idx, int save);

void dd_cursor_init(void)
{
    memset(&dd_cursor, 0, sizeof(DdCursor));
    dd_cursor_set(0, 7, 0, 0);
}

/* d-pad: 0x10 up, 0x40 down, 0x80 left, 0x20 right */
#ifdef NON_MATCHING /* mesma forma, alocação de registradores diferente (s4/s5, dr/dc) */
void dd_cursor_move(int key)
{
    DdCursor *c = &dd_cursor;
    DdLoc cur = c->cur;
    int zone;
    int step;
    int row;
    int col;
    int dc;
    int dr;
    int r;
    int idx;
    DdLoc l;

    if (dd_cursor.locked)
        return;
    zone = cur.zone;
    c->dirty = 1;
    switch (zone) {
    case 0xB:
        step = 0;
        if (cur.player == 0) {
            if (key == 0x10) {
                dd_cursor_set(c->cur.player, dd_cursor_nearest_slot(c->cur.player, c), 0, 0);
                se_play(0x6212);
                return;
            }
            if (key == 0x80)
                step = -1;
            if (key == 0x20)
                step = 1;
        } else {
            if (key == 0x40) {
                dd_cursor_set(c->cur.player, dd_cursor_nearest_slot(c->cur.player, c), 0, 0);
                se_play(0x6212);
                return;
            }
            if (key == 0x80)
                step = 1;
            if (key == 0x20)
                step = -1;
        }
        se_play(0x6212);
        if (cur.idx == 0 && step < 0)
            c->cur.idx = func_0880B770(c->cur.player, 0xB);
        c->cur.idx = c->cur.idx + (unsigned short)step;
        idx = c->cur.idx;
        if (idx >= func_0880B770(c->cur.player, 0xB))
            c->cur.idx = 0;
        dd_cursor_set(c->cur.player, 0xB, c->cur.idx, 0);
        return;
    case 0xF:
        if (cur.player == 0) {
            if (key == 0x80) {
                c->col--;
                c->row++;
                goto done;
            }
        } else if (key == 0x20) {
            c->col++;
            c->row--;
            goto done;
        }
        break;
    }
    switch (key) {
    case 0x80:
        dc = -1;
        dr = 0;
        break;
    case 0x20:
        dc = 1;
        dr = 0;
        break;
    case 0x10:
        dc = 0;
        dr = -1;
        break;
    case 0x40:
        dc = 0;
        dr = 1;
        break;
    }
    row = c->row;
    col = c->col;
    r = row + dr;
    if (r < 0 || r >= 7) {
        l.player = cur.player;
        l.zone = 0xB;
        c->col = 2;
    } else {
        c->row = r;
        c->col = col + dc;
        if (c->col < 0)
            c->col = 6;
        else if (c->col > 6)
            c->col = 0;
        l = dd_cursor_grid[c->row][c->col];
    }
    if (l.zone == 0x10) {
        c->row += dr;
    } else if (l.zone == 0xB) {
        idx = l.player;
        if (func_0880B770(idx, 0xB) > 0) {
            if (zone == 0xD) {
                if (cur.player == 0) {
                    if (key == 0x20)
                        idx = 0;
                    else
                        idx = func_0880B770(idx, 0xB) - 1;
                } else if (key == 0x80) {
                    idx = 0;
                } else {
                    idx = func_0880B770(idx, 0xB) - 1;
                }
            } else {
                idx = dd_cursor_nearest_hand(c->cur.player, c);
            }
            c->cur.zone = dd_cursor_grid[c->row][c->col].zone;
            c->cur.idx = idx;
            se_play(0x6212);
            return;
        }
        if (zone == 0xD) {
            if (key == 0x80) {
                if (cur.player == 0) {
                    c->col = 5;
                    c->row = 5;
                } else {
                    c->col = 6;
                    c->row = 1;
                }
            } else if (key == 0x20) {
                if (cur.player == 0) {
                    c->col = 0;
                    c->row = 5;
                } else {
                    c->col = 1;
                    c->row = 1;
                }
            } else {
                c->col = col;
                c->row = row;
            }
        } else {
            c->col = col;
            c->row = row;
        }
    }
done:
    se_play(0x6212);
    c->cur = dd_cursor_grid[c->row][c->col];
}
#endif

/* puts the cursor on (player, zone, idx); save != 0 keeps the previous location for restore */
void dd_cursor_set(int player, int zone, int idx, int save)
{
    DdCursor *c = &dd_cursor;
    int j, i;

    if (save != 0 && c->saved == 0)
        c->saved_loc = c->cur;
    if (idx == 0xFFFF)
        idx = 0;
    c->saved = save;
    c->cur.player = player;
    c->cur.zone = zone;
    c->cur.idx = idx;
    c->dirty = 1;
    if (zone == 0xB) {
        c->col = 0;
        c->row = player == 0 ? 6 : 0;
        return;
    }
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 7; j++) {
            if (player == dd_cursor_grid[i][j].player && zone == dd_cursor_grid[i][j].zone) {
                c->col = j;
                c->row = i;
                return;
            }
        }
    }
}


void dd_cursor_restore(void)
{
    DdCursor *c = &dd_cursor;

    if (c->saved != 0)
        dd_cursor_set(c->saved_loc.player, c->saved_loc.zone, c->saved_loc.idx, 0);
}

/* the cursor location as the engine sees it (deck/grave/... point at their top card) */
void dd_cursor_get(DdLoc *out)
{
    DdCursor *c = &dd_cursor;
    DdLoc l = c->cur;

    switch (l.zone) {
    case 0xE:
        l.idx = func_08866884(l.player) - 1;
        break;
    case 0xF:
        l.idx = func_0886689C(l.player) - 1;
        break;
    case 0xD:
        l.idx = 0;
        break;
    case 0xC:
        l.idx = 0;
        break;
    }
    *out = l;
}

void dd_cursor_lock(int on)
{
    dd_cursor.locked = on;
}

void dd_cursor_nop(void)
{
}
