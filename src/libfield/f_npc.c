// @range 0x08809C40-0x0880AC74
/* NPCs of the room (32 walking on paths) and the extra characters (80): reset, drawing, the ones
   near the player, the NPC in front of the player, the commands that place them, turn them to the
   player while talking and wait for their files. */
#include "libfield.h"

void fld_npc_free(void);
FieldNpc *fld_npc_find(int id);
int fld_npc_facing(float *best, Vec3 *pos, FieldNpc *p, FieldNpc *n);
void fld_npc_check_loaded(FieldNpc *n);

void fld_npc_init(void)
{
    memset(&field_npcs, 0, sizeof(FieldNpcs));
}

void fld_npc_term(void)
{
    fld_npc_free();
    fld_npc_init();
}

void fld_npc_free(void)
{
    FieldNpcs *b = &field_npcs;
    int i;
    FieldNpc *n;
    FieldMob *m;

    for (i = 0; i < 32; i++) {
        n = &b->npc[i];
        if (b->npc[i].path) {
            fld_free(b->npc[i].path);
            n->path = 0;
        }
        if (n->m.active)
            fld_chr_reset(&n->m.c);
    }
    for (i = 0; i < 80; i++) {
        m = &b->mob[i];
        if (b->mob[i].active)
            fld_chr_reset(&m->c);
    }
}

int fld_cell_free(int x, int z);
void fld_cell_mark(int x, int z, int v);
void fld_pos_set(Vec3 *p, int cx, int cz);
void fld_pos_to_cell(float x, float z, int *cx, int *cz);
void fld_chr_update(FieldChr *c);
int fld_map_walkable(int x, int z, int flag);
int fld_cell_marked(int cx, int cz);
void vec3_lerp(Vec3 *out, Vec3 *a, Vec3 *b, float t);

/* the NPCs wander: when idle (anim 0) and the field runs (ctx->pad04), after waiting xa8 frames they
   pick a random neighbour cell within xa4 of their home (x9c, xa0) that is free, walkable and not
   marked (10 tries), then walk to it (anim 1, xbc 0..1 by 0.1); their cells are marked busy (the
   NPC 0x3A, big, a 5 x 7 block); then the extra characters */
void fld_npc_update(void)
{
    FieldNpcs *b = &field_npcs;
    FieldCtx *ctx = fld_ctx_get();
    int i;
    FieldNpc *npc;
    int n;
    int dx;
    int dz;
    Vec3 from;
    Vec3 to;
    float t;

    for (i = 0; i < 32; i++) {
        npc = &b->npc[i];
        if (npc->m.c.visible == 0)
            continue;
        if (npc->m.active == 0)
            continue;
        if (npc->xa4 > 0) {
            switch (npc->m.c.anim) {
            case 0:
                if (ctx->pad04 != 0) {
                    if (npc->xa8 <= 0) {
                        n = 0;
                        for (;;) {
                            npc->xb4 = npc->xac;
                            npc->xb8 = npc->xb0;
                            switch ((int)(rand() & 0xFFFF) % 4) {
                            case 0:
                                npc->xb8 = npc->xb0 + 1;
                                npc->m.c.dir = 1;
                                break;
                            case 1:
                                npc->xb4 = npc->xac - 1;
                                npc->m.c.dir = 3;
                                break;
                            case 2:
                                npc->xb8 = npc->xb0 - 1;
                                npc->m.c.dir = 5;
                                break;
                            case 3:
                                npc->xb4 = npc->xac + 1;
                                npc->m.c.dir = 7;
                                break;
                            }
                            if (n >= 10)
                                break;
                            n++;
                            if (fabsf_vfpu(npc->m.x9c - npc->xb4) > npc->xa4)
                                continue;
                            if (fabsf_vfpu(npc->xa0 - npc->xb8) > npc->xa4)
                                continue;
                            if (fld_cell_free(npc->xb4, npc->xb8) == 0)
                                continue;
                            if (fld_map_walkable(npc->xb4, npc->xb8, 0) == 0)
                                continue;
                            if (fld_cell_marked(npc->xb4, npc->xb8) != 0)
                                continue;
                            break;
                        }
                        if (n < 10) {
                            npc->m.c.anim = 1;
                            npc->xbc = 0;
                        } else {
                            npc->m.c.anim = 0;
                            npc->xa8 = (int)(rand() & 0xFFFF) % 120 + 90;
                        }
                    } else {
                        npc->xa8--;
                    }
                }
                break;
            case 1:
                if (npc->turned == 0) {
                    fld_pos_set(&from, npc->xac, npc->xb0);
                    fld_pos_set(&to, npc->xb4, npc->xb8);
                    vec3_lerp(&npc->m.c.pos, &from, &to, *(float *)&npc->xbc);
                    if (*(float *)&npc->xbc < 1.0f) {
                        float k = 0.1f;

                        t = *(float *)&npc->xbc + k;
                        *(float *)&npc->xbc = t;
                        if (t > 1.0f)
                            *(float *)&npc->xbc = 1.0f;
                        fld_cell_mark(npc->xb4, npc->xb8, 3);
                    } else {
                        npc->m.c.anim = 0;
                        npc->xa8 = (int)(rand() & 0xFFFF) % 120 + 90;
                        fld_cell_mark(npc->xac, npc->xb0, 0);
                        fld_pos_to_cell(npc->m.c.pos.x, npc->m.c.pos.z, &npc->xac, &npc->xb0);
                    }
                }
                break;
            }
        }
        fld_cell_mark(npc->xac, npc->xb0, 3);
        if (npc->m.c.id == 0x3A) {
            for (dx = -3; dx <= 1; dx++) {
                for (dz = -4; dz <= 2; dz++)
                    fld_cell_mark(npc->xac + dx, npc->xb0 + dz, 5);
            }
        }
        fld_chr_update(&npc->m.c);
    }
    for (i = 0; i < 80; i++) {
        FieldMob *e = &b->mob[i];

        if (e->c.visible != 0 && e->active != 0)
            fld_chr_update(&e->c);
    }
}

void fld_npc_draw(void)
{
    FieldNpcs *b = &field_npcs;
    int i;
    FieldNpc *n;
    FieldMob *m;

    for (i = 0; i < 32; i++) {
        n = &b->npc[i];
        if (b->npc[i].m.c.visible && n->m.active && n->m.file && *n->m.file)
            fld_chr_draw(&n->m.c);
    }
    for (i = 0; i < 80; i++) {
        m = &b->mob[i];
        if (b->mob[i].c.visible && m->active && m->file && *m->file)
            fld_chr_draw(&m->c);
    }
}

/* NPCs closer than r to p */
void fld_npc_near(FieldNpc **out, int *n, FieldNpc *p, float r)
{
    FieldNpcs *b = &field_npcs;
    int i;
    FieldNpc *c;
    Vec3 d;

    *n = 0;
    for (i = 0; i < 32; i++) {
        c = &b->npc[i];
        if (b->npc[i].m.active && c->m.c.visible) {
            vec3_sub(&d, &c->m.c.pos, &p->m.c.pos);
            if (!(fld_vec3_len(&d) > r)) {
                out[*n] = c;
                *n = *n + 1;
            }
        }
    }
}

void fld_npc_all(FieldNpc **out, int *n)
{
    FieldNpcs *b = &field_npcs;
    int i;
    FieldNpc *c;

    *n = 0;
    for (i = 0; i < 32; i++) {
        c = &b->npc[i];
        if (b->npc[i].m.active) {
            out[*n] = c;
            *n = *n + 1;
        }
    }
}

int fld_npc_place_done(void);

typedef struct FieldNpcPlace {          /* one character of the place command, 0x40 */
    int kind;
    int id;
    int xc0;                           /* +0x08 */
    int x;                             /* +0x0C */
    int z;                             /* +0x10 */
    int x2;                            /* +0x14 */
    int z2;                            /* +0x18 */
    int visible;                       /* +0x1C */
    int mob;                           /* +0x20 extra character, else an NPC */
    int xa4;                           /* +0x24 */
    int dir;                           /* +0x28 */
    int xd0;                           /* +0x2C */
    int xd4;                           /* +0x30 */
    int fx;                            /* +0x34 */
    void *path;                        /* +0x38 0x18 bytes per point */
    int n_path;                        /* +0x3C */
} FieldNpcPlace;

typedef struct FieldNpcPlaceCmd {
    FieldNpcPlace *e;
    int n;
    int talk_id;
    int talk_dir;
} FieldNpcPlaceCmd;

void fld_npc_loaded_a(int *file, FieldNpc *n);
void fld_npc_loaded_b(int *file, FieldNpc *n);
int fld_npc_free_slot(void);
int fld_mob_free_slot(void);
int fld_fx_attach(FieldChr *c, int kind, int b);

/* places the characters of the command; with no free slot the original reuses the slot of the
   previous character (an uninitialized pointer for the first one, kept here) */
void fld_npc_cmd_place(FieldNpcPlaceCmd *cmd)
{
    FieldNpcs *b;
    FieldNpcPlace *e;
    int i;
    FieldMob *m;
    FieldNpc *n;
    int k;
    int fx;
    int size;

    field_npcs.loading = 1;
    b = &field_npcs;
    for (i = 0; i < cmd->n; i++) {
        e = &cmd->e[i];
        if (e->mob != 0) {
            k = fld_mob_free_slot();
            if (k != -1)
                m = &b->mob[k];
            m->active = 0;
            b->pending++;
            m->loaded = 1;
            fld_chr_setup(&m->c, e->kind, e->id, e->x, e->z, 1, e->dir, e->visible, 1, fld_npc_loaded_a,
                          fld_npc_loaded_b, 0);
        } else {
            k = fld_npc_free_slot();
            if (k != -1)
                n = &b->npc[k];
            n->m.active = 0;
            n->xa4 = e->xa4;
            n->xa8 = (unsigned short)rand() % 120 + 90;
            n->m.x9c = e->x;
            n->xa0 = e->z;
            n->xac = e->x2;
            n->xb0 = e->z2;
            n->xb4 = e->x2;
            n->xb8 = e->z2;
            n->xc0 = e->xc0;
            n->xd0 = e->xd0;
            n->xd4 = e->xd4;
            fx = fld_fx_attach((FieldChr *)n, e->fx, 0);
            if (e->path != 0) {
                size = e->n_path * 0x18;
                n->n_path = e->n_path;
                n->path = fld_alloc(size);
                memcpy(n->path, e->path, size);
            }
            b->pending++;
            n->m.loaded = 1;
            fld_chr_setup(&n->m.c, e->kind, e->id, e->x2, e->z2, 1, e->dir, e->visible, 0, fld_npc_loaded_a,
                          fld_npc_loaded_b, fx);
        }
    }
    b->talk_id = cmd->talk_id;
    b->talk_dir = cmd->talk_dir;
    if (cmd->n <= 0)
        b->loading = 0;
}

int fld_npc_place_done(void)
{
    return !field_npcs.loading;
}

void fld_npc_cmd_move(int *a)
{
    FieldNpcs *b = &field_npcs;
    FieldNpc *n;

    field_npcs.setting = 1;
    n = fld_npc_find(a[0]);
    fld_pos_set(&n->m.c.pos, a[1], a[2]);
    if (a[5] != -1)
        n->m.c.dir = a[5];
    if (a[3] != -1)
        n->m.c.visible = a[3];
    if (a[4] != -1)
        n->xa4 = a[4];
    if (a[1] != -1)
        n->m.x9c = a[1];
    if (a[2] != -1)
        n->xa0 = a[2];
    if (a[1] != -1)
        n->xac = a[1];
    if (a[2] != -1)
        n->xb0 = a[2];
    if (a[1] != -1)
        n->xb4 = a[1];
    if (a[2] != -1)
        n->xb8 = a[2];
    if (a[6] != 0)
        n->xd0 = a[6];
    if (a[7] != 0)
        n->xd4 = a[7];
    b->setting = 0;
}

int fld_npc_move_done(void)
{
    return !field_npcs.setting;
}

/* id of the NPC the player at p faces, -1 if none */
int fld_npc_facing_id(FieldNpc *p)
{
    float best = 0.0f;
    FieldNpcs *b = &field_npcs;
    int r = -1;
    int i;
    FieldNpc *n;
    int j;
    Vec3 v;

    if (p == 0)
        return r;
    for (i = 0; i < 32; i++) {
        n = &b->npc[i];
        if (b->npc[i].m.c.visible && n->m.active) {
            if (fld_npc_facing(&best, &n->m.c.pos, p, n))
                r = n->m.c.kind;
            if (n->path) {
                for (j = 0; j < n->n_path; j++) {
                    fld_pos_set(&v, n->path[j * 6], n->path[j * 6 + 1]);
                    if (fld_npc_facing(&best, &v, p, n))
                        r = n->m.c.kind;
                }
            }
        }
    }
    return r;
}

void fld_npc_get_d0(int id, int *a, int *b)
{
    FieldNpc *n = fld_npc_find(id);

    if (n) {
        *a = n->xd0;
        *b = n->xd4;
    } else {
        *a = 0;
        *b = 0;
    }
}

int fld_npc_get_c0(int id)
{
    FieldNpc *n = fld_npc_find(id);
    int r;

    if (n) {
        r = n->xc0;
        if (r == -1)
            return 0;
    } else {
        r = -1;
    }
    return r;
}

void fld_npc_turn_to(int id, FieldNpc *p)
{
    FieldNpc *n = fld_npc_find(id);
    Vec3 d;
    int dir;

    vec3_sub(&d, &p->m.c.pos, &n->m.c.pos);
    dir = fld_dir_from_vec(&d, 1);
    field_npcs.talking = 1;
    field_npcs.talk_id = id;
    n->turned = 1;
    n->saved_dir = n->m.c.dir;
    n->saved_anim = n->m.c.anim;
    n->m.c.dir = dir;
    n->m.c.anim = 0;
}

void fld_npc_turn_back(void)
{
    FieldNpcs *b = &field_npcs;
    FieldNpc *n;

    if (field_npcs.talking) {
        n = fld_npc_find(b->talk_id);
        b->talking = 0;
        b->talk_id = 0;
        n->turned = 0;
        n->m.c.dir = n->saved_dir;
        n->m.c.anim = n->saved_anim;
    }
}

int fld_npc_free_slot(void)
{
    FieldNpcs *b = &field_npcs;
    int i;

    FieldNpc *n;

    for (i = 0; i < 32; i++) {
        n = &b->npc[i];
        if (!b->npc[i].m.active && !n->m.loaded)
            return i;
    }
    return -1;
}

int fld_mob_free_slot(void)
{
    FieldNpcs *b = &field_npcs;
    int i;

    FieldMob *m;

    for (i = 0; i < 80; i++) {
        m = &b->mob[i];
        if (!b->mob[i].active && !m->loaded)
            return i;
    }
    return -1;
}

FieldNpc *fld_npc_find(int id)
{
    FieldNpcs *b = &field_npcs;
    int i;
    FieldNpc *n;

    for (i = 0; i < 32; i++) {
        n = &b->npc[i];
        if (b->npc[i].m.active && n->m.c.kind == id)
            return n;
    }
    return 0;
}

/* 1 if pos is in front of p (nearer along its direction than *best) */
int fld_npc_facing(float *best, Vec3 *pos, FieldNpc *p, FieldNpc *n)
{
    float r = 0.75f;
    Vec3 d;
    Vec3 f;
    Vec3 t;
    float l;
    float dot;

    if (n->m.c.id == 0x3A)
        r = 1.5f;
    vec3_sub(&d, pos, &p->m.c.pos);
    l = fld_vec3_len(&d);
    if (fabsf_vfpu(d.y) >= 1.0f)
        return 0;
    if (l <= r) {
        fld_dir_vec(&t, p->m.c.dir);
        f = *(Vec3 *)&t;
        dot = vec3_dot(&f, &d);
        if (dot > *best) {
            *best = dot;
            return 1;
        }
    }
    return 0;
}

void fld_npc_loaded_a(int *file, FieldNpc *n)
{
    n->m.file = file;
    if (file) {
        n->m.c.tex[0] = *file;
        fld_npc_check_loaded(n);
    }
}

void fld_npc_loaded_b(int *file, FieldNpc *n)
{
    if (file) {
        n->m.c.tex[1] = *file;
        fld_npc_check_loaded(n);
    }
}

void fld_npc_check_loaded(FieldNpc *n)
{
    FieldNpcs *b = &field_npcs;
    FieldNpc *t;

    if (fld_in_range(n->m.c.id)) {
        if (n->m.c.tex[0] && n->m.c.tex[1]) {
            n->m.active = 1;
            n->m.loaded = 0;
            b->pending--;
        }
    } else if (n->m.c.tex[0]) {
        n->m.active = 1;
        n->m.loaded = 0;
        b->pending--;
    }
    if (b->pending <= 0) {
        if (b->talk_id != -1) {
            t = fld_npc_find(b->talk_id);
            b->talking = 1;
            t->turned = 1;
            t->saved_dir = b->talk_dir;
            t->saved_anim = t->m.c.anim;
        }
        b->loading = 0;
    }
}
