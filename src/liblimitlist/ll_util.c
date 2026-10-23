// @range 0x08804000-0x08804128
// @rodata 0x0880D06C-0x0880D094
// @data 0x0880D0BC-0x0880D230
/* Lookups shared by the screen: the card's line in fileTbl.dat, a (card, value) pair in a counted
   table, the card kind -> icon index, and the game's default forbidden/limited list. */
#include "liblimitlist.h"

/* unreferenced word at the start of the module's read-only data */
const int ll_rodata_0880D06C = 0;

/* the official list the "reset" menu option restores (inv_set_limit for each) */
LlLimit ll_default_limits[93] = {
    { 0x16E4, 0 }, { 0x16CB, 0 }, { 0x1181, 0 }, { 0x11E4, 0 },
    { 0x1331, 0 }, { 0x14A5, 0 }, { 0x161C, 0 }, { 0x14FB, 0 },
    { 0x16EC, 0 }, { 0x1619, 0 }, { 0x14FF, 0 }, { 0x1539, 0 },
    { 0x1325, 0 }, { 0x132F, 0 }, { 0x132B, 0 }, { 0x12EC, 0 },
    { 0x12FC, 0 }, { 0x10F7, 0 }, { 0x12EA, 0 }, { 0x165C, 0 },
    { 0x1246, 0 }, { 0x10F6, 0 }, { 0x1360, 0 }, { 0x14F0, 0 },
    { 0x1739, 0 }, { 0x1378, 0 }, { 0x138D, 0 }, { 0x151E, 0 },
    { 0x0FB8, 1 }, { 0x0FBA, 1 }, { 0x0FB7, 1 }, { 0x0FB9, 1 },
    { 0x1657, 1 }, { 0x13A7, 1 }, { 0x0FD6, 1 }, { 0x16F8, 1 },
    { 0x1296, 1 }, { 0x178B, 1 }, { 0x19FE, 1 }, { 0x13A6, 1 },
    { 0x179A, 1 }, { 0x14CB, 1 }, { 0x185C, 1 }, { 0x0FBB, 1 },
    { 0x1770, 1 }, { 0x1617, 1 }, { 0x11F5, 1 }, { 0x100C, 1 },
    { 0x19CB, 1 }, { 0x1284, 1 }, { 0x1694, 1 }, { 0x1324, 1 },
    { 0x131B, 1 }, { 0x1445, 1 }, { 0x143D, 1 }, { 0x1322, 1 },
    { 0x132D, 1 }, { 0x12D2, 1 }, { 0x145C, 1 }, { 0x1538, 1 },
    { 0x12D5, 1 }, { 0x12CC, 1 }, { 0x196F, 1 }, { 0x143C, 1 },
    { 0x1366, 1 }, { 0x1102, 1 }, { 0x145D, 1 }, { 0x1638, 1 },
    { 0x15A3, 1 }, { 0x1315, 1 }, { 0x1461, 1 }, { 0x1409, 1 },
    { 0x17A6, 1 }, { 0x13FA, 1 }, { 0x123B, 1 }, { 0x1317, 1 },
    { 0x135C, 1 }, { 0x151C, 1 }, { 0x1404, 1 }, { 0x1548, 1 },
    { 0x137D, 1 }, { 0x16F9, 2 }, { 0x1152, 2 }, { 0x129A, 2 },
    { 0x1612, 2 }, { 0x142A, 2 }, { 0x14D0, 2 }, { 0x131F, 2 },
    { 0x14E6, 2 }, { 0x1363, 2 }, { 0x140E, 2 }, { 0x1888, 2 },
    { 0x188C, 2 },
};

/* index of `id` in a sorted, counted u16 table (fileTbl.dat), 0 when absent */
unsigned short ll_file_index(unsigned short id, unsigned short *tab)
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
int ll_pair_find(unsigned short a, unsigned short b, int *tab)
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
int ll_kind_icon(unsigned short kind)
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
