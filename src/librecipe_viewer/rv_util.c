// @range 0x08804778-0x08804918
// @rodata 0x0880E4E8-0x0880E50C
/* Lookups shared by the screen: the card's line in fileTbl.dat, a (card, value) pair in a counted
   table, the card kind -> icon index (same code as liblimitlist's ll_util.c), and the number of
   partners holding a card. */
#include "librecipe_viewer.h"
#include "game.h"

/* index of `id` in a sorted, counted u16 table (fileTbl.dat), 0 when absent */
unsigned short rv_file_index(unsigned short id, unsigned short *tab)
{
    unsigned short r = 0;
    unsigned short *ids;
    int lo;
    int hi;
    int mid;

    if (tab == 0)
        return r;
    ids = tab + 1;
    hi = tab[0] - 1;
    lo = 0;
    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (id == ids[mid]) {
            r = mid;
            break;
        }
        if (ids[mid] < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return r;
}

/* index of the pair (a, b) in a table { int n; unsigned int pairs[n]; }, -1 when absent */
int rv_pair_find(unsigned short a, unsigned short b, int *tab)
{
    unsigned int key = a | b << 16;
    int n = tab[0];
    unsigned int *p = (unsigned int *)tab;
    int i;

    for (i = 0; i < n; i++) {
        if (key == p[1])
            return i;
        p++;
    }
    return -1;
}

/* icon index of a card kind, -1 for kinds without one */
int rv_kind_icon(unsigned short kind)
{
    switch (kind) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 7:
        return 4;
    case 8:
        return 5;
    }
    return -1;
}

/* partners that hold card `id` */
int rv_partner_count(int id)
{
    int n = partner_count_get();
    int i;
    int c = 0;

    for (i = 0; i < n; i++) {
        if (id == partner_id_get(i))
            c++;
    }
    return c;
}
