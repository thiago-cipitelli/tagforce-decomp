// @range 0x08807844-0x08808214
// @asm 0x08807844-0x08807BDC
// @asm 0x08807BDC-0x08807EE0
// @asm 0x08807F50-0x08807FB0
// @asm 0x08807FB0-0x08808160
/* Cell map of the room: walkable cells (with the doors of the dormitory and the stage closed by
   story flags), height of a point on the ground triangles, cell <-> position, marked cells, the
   door cells leading to another room and the cells of the special hall. */
#include "libfield.h"

void fld_map_height(float x, float z, Vec3 *p);
int fld_bg_id(void);

void fld_pos_set(Vec3 *p, int cx, int cz)
{
    p->x = 0.25f + 0.5f * cx;
    p->z = 0.25f + 0.5f * cz;
    fld_map_height(p->x, p->z, p);
}

void fld_pos_to_cell(float x, float z, int *cx, int *cz)
{
    *cx = x / 0.5f;
    *cz = z / 0.5f;
}

#ifdef NON_MATCHING /* alocação: o original guarda n_cells em a3 e recalcula cells + i*4 a cada volta */
int fld_cell_marked(int cx, int cz)
{
    FieldBg *b = &field_bg_state;
    int i;

    for (i = 0; i < field_bg_state.n_cells; i++) {
        if (cx == b->cells[i].x && cz == b->cells[i].z)
            return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* o default do original salta direto ao epílogo e os testes de 0x24 saem em outra forma */
/* room a door cell leads to, -1 if none */
int fld_cell_door(int cx, int cz)
{
    int r = -1;

    switch (fld_bg_id()) {
    case 0xC:
    case 0x25:
        if (cx == 0xF && cz == 0x18)
            r = 0xD;
        else if (cx == 0xF && cz == 0x14)
            r = 0xE;
        else if (cx == 0xF && cz == 0x10)
            r = 0xF;
        else if (cx == 0xF && cz == 0xC)
            r = 0x10;
        if (cx == 0xD && cz == 0x18)
            r = 0x11;
        else if (cx == 0xD && cz == 0x14)
            r = 0x12;
        else if (cx == 0xD && cz == 0x10)
            r = 0x13;
        else if (cx == 0xD && cz == 0xC)
            r = 0x14;
        else if (cx == 0xD && cz == 6)
            r = 0x15;
        break;
    case 0x24:
        if (cx == 9 && cz == 0)
            r = 0x27;
        else if (cx == 10 && cz == 0)
            r = 0x27;
        break;
    default:
        r = -1;
        break;
    }
    return r;
}
#endif

int fld_cell_special(int cx, int cz)
{
    int *p = 0;
    int n = 0;
    int i;

    switch (fld_bg_id()) {
    case 0x15:
        p = field_hall_cells[0];
        n = 14;
        break;
    }
    if (p != 0) {
        for (i = 0; i < n; i++, p += 2) {
            if (cx == p[0] && cz == p[1])
                return 1;
        }
    }
    return 0;
}

int fld_bg_id(void)
{
    return field_bg_state.id;
}
