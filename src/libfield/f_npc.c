// @range 0x08809C40-0x0880AC74
// @asm 0x08809D20-0x0880A104
// @asm 0x0880A2FC-0x0880A58C
// @asm 0x0880A860-0x0880A898
// @asm 0x0880AA4C-0x0880AB48
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

int fld_npc_place_done(void)
{
    return !field_npcs.loading;
}

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

#ifdef NON_MATCHING /* C novo, conferido com o asm. ~118 palavras: o mwcc tira do laço os endereços de fld_npc_loaded_a/b (s8 e pilha); o original os recalcula a cada chamada e guarda size e fx em s6/s8 */
/* places the characters of the command; with no free slot the original reuses the slot of the
   previous character (an uninitialized pointer for the first one, kept here) */
void fld_npc_cmd_place(FieldNpcPlaceCmd *cmd)
{
    FieldNpcs *b;
    FieldNpc *n;
    FieldMob *m;
    FieldNpcPlace *e;
    int i;
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
            fld_chr_setup(&m->c, e->kind, e->id, e->x, e->z, 1, e->dir, e->visible, 1, (int)fld_npc_loaded_a,
                          (int)fld_npc_loaded_b, 0);
        } else {
            k = fld_npc_free_slot();
            if (k != -1)
                n = &b->npc[k];
            n->m.active = 0;
            n->xa4 = e->xa4;
            n->xa8 = (rand() & 0xFFFF) % 120 + 90;
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
            fld_chr_setup(&n->m.c, e->kind, e->id, e->x2, e->z2, 1, e->dir, e->visible, 0, (int)fld_npc_loaded_a,
                          (int)fld_npc_loaded_b, fx);
        }
    }
    b->talk_id = cmd->talk_id;
    b->talk_dir = cmd->talk_dir;
    if (cmd->n <= 0)
        b->loading = 0;
}
#endif

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

#ifdef NON_MATCHING /* o teste r == -1 sai bne/nop no original e bnel aqui */
int fld_npc_get_c0(int id)
{
    FieldNpc *n = fld_npc_find(id);
    int r;

    if (n) {
        r = n->xc0;
        if (r == -1)
            r = 0;
    } else {
        r = -1;
    }
    return r;
}
#endif

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

#ifdef NON_MATCHING /* o caso dot <= *best tem bloco próprio depois do "b" no original */
/* 1 if pos is in front of p (nearer along its direction than *best) */
int fld_npc_facing(float *best, Vec3 *pos, FieldNpc *p, FieldNpc *n)
{
    float r = 0.75f;
    Vec3 d;
    Vec3 f;
    Vec3 t;
    float l;
    float dot;
    int res;

    if (n->m.c.id == 0x3A)
        r = 1.5f;
    vec3_sub(&d, pos, &p->m.c.pos);
    l = fld_vec3_len(&d);
    if (fabsf_vfpu(d.y) >= 1.0f) {
        res = 0;
    } else if (l <= r) {
        fld_dir_vec(&t, p->m.c.dir);
        f = t;
        dot = vec3_dot(&f, &d);
        if (dot > *best) {
            *best = dot;
            res = 1;
        } else {
            res = 0;
        }
    } else {
        res = 0;
    }
    return res;
}
#endif

void fld_npc_loaded_a(int *file, FieldNpc *n)
{
    n->m.file = file;
    if (file) {
        n->m.c.pad08[0] = *file;
        fld_npc_check_loaded(n);
    }
}

void fld_npc_loaded_b(int *file, FieldNpc *n)
{
    if (file) {
        n->m.c.pad08[1] = *file;
        fld_npc_check_loaded(n);
    }
}

void fld_npc_check_loaded(FieldNpc *n)
{
    FieldNpcs *b = &field_npcs;
    FieldNpc *t;

    if (fld_in_range(n->m.c.id)) {
        if (n->m.c.pad08[0] && n->m.c.pad08[1]) {
            n->m.active = 1;
            n->m.loaded = 0;
            b->pending--;
        }
    } else if (n->m.c.pad08[0]) {
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
