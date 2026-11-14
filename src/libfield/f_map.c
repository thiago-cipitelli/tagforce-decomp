// @range 0x08807844-0x08808214
/* Cell map of the room: walkable cells (with the doors of the dormitory and the stage closed by
   story flags), height of a point on the ground triangles, cell <-> position, marked cells, the
   door cells leading to another room and the cells of the special hall. */
#include "libfield.h"

void fld_map_height(Vec3 *p, float x, float z);
int fld_bg_id(void);

unsigned char *fld_map_attrs(void);
FieldStory *fld_story_get(void);

/* 1 when the cell can be walked on: the stairs of the hostel are shut (always for flag 0, from the
   22nd for flag 1, which leaves (13, 12) open) and the doors of room 2 while ctx->x14; then the
   attribute of the cell */
/* i >= n_vert vai por goto ao "r = 0" do m == 0 (beqz; nop para o bloco do fim no original) */
int fld_map_walkable(int cx, int cz, int flag)
{
    FieldMap *m = fld_map_get();
    FieldStory *st = fld_story_get();
    FieldCtx *ctx = fld_ctx_get();
    unsigned char *a;
    int i;
    int r;

    if (flag != 0) {
        if (*(int *)st->w[3] >= 22) {
            switch (fld_bg_id()) {
            case 12:
            case 37:
                if ((cx == 15 && cz == 24) || (cx == 15 && cz == 20) || (cx == 15 && cz == 16) ||
                    (cx == 15 && cz == 12) || (cx == 13 && cz == 24) || (cx == 13 && cz == 20) ||
                    (cx == 13 && cz == 16))
                    return 0;
                break;
            }
        }
    } else {
        switch (fld_bg_id()) {
        case 12:
        case 37:
            if ((cx == 15 && cz == 24) || (cx == 15 && cz == 20) || (cx == 15 && cz == 16) ||
                (cx == 15 && cz == 12) || (cx == 13 && cz == 24) || (cx == 13 && cz == 20) ||
                (cx == 13 && cz == 16) || (cx == 13 && cz == 12))
                return 0;
            break;
        }
    }
    switch (fld_bg_id()) {
    case 2:
        if (ctx->x14 != 0) {
            if ((cx == 1 && cz == 3) || (cx == 1 && cz == 4) || (cx == 10 && cz == 1) || (cx == 11 && cz == 1) ||
                (cx == 21 && cz == 1) || (cx == 22 && cz == 1))
                return 0;
        }
        break;
    }
    if (m != 0) {
        a = fld_map_attrs();
        if (cx < 0) {
            r = 0;
        } else {
            if (cx >= m->w) {
                r = 0;
            } else {
                if (cz < 0) {
                    r = 0;
                } else {
                    if (cz >= m->h) {
                        r = 0;
                    } else {
                        i = cx + cz * m->w;
                        if (i >= 0) {
                            if (i >= m->n_vert)
                                goto zero;
                            r = a[i];
                        } else {
                            r = 0;
                        }
                    }
                }
            }
        }
    } else {
    zero:
        r = 0;
    }
    return r;
}

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

int fld_cell_marked(int cx, int cz)
{
    FieldBg *b = &field_bg_state;
    int i;

    for (i = 0; i < field_bg_state.n_cells; i++) {
        FieldCell *c = (FieldCell *)((char *)b->cells + (unsigned int)(i * 4));

        if (cx == c->x && cz == c->z)
            return 1;
    }
    return 0;
}

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
