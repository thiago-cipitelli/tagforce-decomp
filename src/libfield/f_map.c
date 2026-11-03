// @range 0x08807844-0x08808214
// @asm 0x08807844-0x08807BDC
// @asm 0x08807F50-0x08807FB0
/* Cell map of the room: walkable cells (with the doors of the dormitory and the stage closed by
   story flags), height of a point on the ground triangles, cell <-> position, marked cells, the
   door cells leading to another room and the cells of the special hall. */
#include "libfield.h"

void fld_map_height(Vec3 *p, float x, float z);
int fld_bg_id(void);

Vec3 *fld_map_cells(void);
int fld_bg_id(void);
void vec3_cross_nw(Vec3 *out, Vec3 *a, Vec3 *b);

/* height of the ground at (x, z) into p->y: the plane of the triangle of the height grid under the
   point (the cell is split on its diagonal); the stairs of the hostel use the next column */
void fld_map_height(Vec3 *p, float x, float z)
{
    FieldMap *m = fld_map_get();
    Vec3 *v;
    int cx;
    int cz;
    int i;
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 e1;
    Vec3 e2;
    Vec3 n;

    if (m == 0)
        return;
    v = fld_map_cells();
    cx = x / 0.5f;
    cz = z / 0.5f;
    switch (fld_bg_id()) {
    case 12:
    case 37:
        if ((cx == 15 && cz == 24) || (cx == 15 && cz == 20) || (cx == 15 && cz == 16) || (cx == 15 && cz == 12))
            cx++;
        break;
    }
    i = cx + cz * m->vw;
    if (i < 0)
        return;
    if (i >= m->n_vert)
        return;
    if (p->x - v[i].x < p->z - v[i].z) {
        a = v[i];
        b = v[cx + (cz + 1) * m->vw];
        c = v[cx + 1 + (cz + 1) * m->vw];
    } else {
        a = v[i];
        b = v[cx + 1 + (cz + 1) * m->vw];
        c = v[cx + 1 + cz * m->vw];
    }
    vec3_sub(&e1, &b, &a);
    vec3_sub(&e2, &c, &a);
    vec3_cross_nw(&n, &e1, &e2);
    p->y = (vec3_dot(&n, &a) - p->x * n.x - p->z * n.z) / n.y;
}

void fld_pos_set(Vec3 *p, int cx, int cz)
{
    p->x = 0.25f + 0.5f * cx;
    p->z = 0.25f + 0.5f * cz;
    fld_map_height(p, p->x, p->z);
}

void fld_pos_to_cell(float x, float z, int *cx, int *cz)
{
    *cx = x / 0.5f;
    *cz = z / 0.5f;
}

#ifdef NON_MATCHING /* 8 palavras: o original mantém o deslocamento i*4 numa variável de indução (t0) ao lado de i; com o índice com sizeof o mwcc refaz o sll a cada volta, com i*4 com sinal reduz a ponteiros (20 palavras) */
int fld_cell_marked(int cx, int cz)
{
    FieldBg *b = &field_bg_state;
    int i;

    for (i = 0; i < field_bg_state.n_cells; i++) {
        FieldCell *c = (FieldCell *)((char *)b->cells + i * sizeof(FieldCell));

        if (cx == c->x && cz == c->z)
            return 1;
    }
    return 0;
}
#endif

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
        if ((cx == 9 && cz == 0) || (cx == 10 && cz == 0))
            r = 0x27;
        break;
    }
    return r;
}

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
