// @range 0x0880AC74-0x0880BB98
// @data 0x0883A51C-0x0883A534
// @asm 0x0880AC94-0x0880B010
// @asm 0x0880BA30-0x0880BB98
/* The player character: setup, drawing, input lock, the effect attached to it, the marks it leaves
   on the cells under it, and (in asm) its per-frame update, the stick input and the movement. */
#include "libfield.h"

void fld_player_update(void);                  /* 0x0880AC94 */
void fld_player_loaded_a(int *file, FieldChr *c);
void fld_player_loaded_b(int *file, FieldChr *c);
void fld_fx_free(int fx);                      /* 0x088271DC */
int fld_fx_attach(FieldChr *c, int kind, int b);      /* 0x08827144 */
void fld_cell_mark(int cx, int cz, int v);     /* 0x0881CE28 */
PadState *pad_get(void);
int region_get(void);
void fld_dir_vec(Vec3 *out, int i);
float fld_stick_axis(int v);
void fld_chr_collide(Vec3 *out, Vec3 *vel, Vec3 *pos, float r, int flag);
void fld_npc_near(FieldNpc **out, int *n, FieldNpc *p, float r);
int fld_dir_nearest(Vec3 *v);
Vec3 *vec3_cross_nw(Vec3 *out, Vec3 *a, Vec3 *b);
Vec3 *vec3_zero(Vec3 *out);

void fld_player_init(void)
{
    memset(&field_player, 0, sizeof(FieldPlayer));
}

void fld_player_reset(void)
{
    fld_chr_reset(&field_player.c);
}

void fld_player_draw(void)
{
    FieldPlayer *p = &field_player;
    FieldChr *c = &field_player.c;

    if (field_player.file_a && *p->file_a && p->file_b && *p->file_b)
        fld_chr_draw(c);
}

void fld_player_lock(void)
{
    field_player.lock = 1;
}

void fld_player_stop(void)
{
    FieldChr *c = &field_player.c;

    c->frame = 0;
    c->anim = 0;
}

void fld_player_setup(float x, float z, int dir)
{
    fld_chr_setup(&field_player.c, 1, 1, 0, 0, 0, dir, 1, 0, (int)fld_player_loaded_a,
                  (int)fld_player_loaded_b, 0);
    field_player.c.pos.x = x;
    field_player.c.pos.z = z;
    fld_map_height(x, z, &field_player.c.pos);
    field_player.active = 1;
    fld_player_update();
}

int fld_player_loaded(void)
{
    FieldChr *c = &field_player.c;

    if (c->pad08[0] && c->pad08[1])
        return 1;
    return 0;
}

FieldChr *fld_player_get(void)
{
    FieldPlayer *p = &field_player;
    FieldChr *r;

    if (field_player.active != 0)
        r = &p->c;
    else
        r = 0;
    return r;
}

void fld_player_get_pos(Vec3 *out)
{
    FieldPlayer *p = &field_player;

    if (field_player.active)
        fld_chr_get_pos(out, &p->c);
}

void fld_player_fx(int kind, int b)
{
    FieldChr *c = &field_player.c;

    if (kind == 0 && b == 0) {
        fld_fx_free(c->fx);
        c->fx = 0;
    } else {
        c->fx = fld_fx_attach(c, kind, b);
    }
}

void fld_player_set_lock(int lock)
{
    field_player.lock = lock;
}

int fld_player_idle(void)
{
    return !field_player.busy;
}

/* marks the cells under the four corners of the player with v */
void fld_player_mark_cells(int v)
{
    FieldChr *c = &field_player.c;
    Vec3 q[4];
    int cx;
    int cz;
    int i;
    Vec3 *p;

    if (fld_map_get()) {
        fld_chr_corners(0.1f, &c->pos, &q[0], &q[1], &q[2], &q[3]);
        for (i = 0, p = q; i < 4; i++, p++) {
            fld_pos_to_cell(p->x, p->z, &cx, &cz);
            if (fld_map_attr(cx, cz) != 3)
                fld_cell_mark(cx, cz, v);
        }
    }
}

void fld_player_loaded_a(int *file, FieldChr *c)
{
    field_player.file_a = file;
    if (file)
        c->pad08[0] = *file;
}

void fld_player_loaded_b(int *file, FieldChr *c)
{
    field_player.file_b = file;
    if (file)
        c->pad08[1] = *file;
}

/* the direction pad (diagonals first) or else the stick: sets the direction, the walking flag and the
   unit step of the player */
void fld_player_input(void)
{
    FieldChr *c = &field_player.c;
    float m[9];
    Vec3 u;
    Vec3 w;
    unsigned int b = pad_get()->b.down;

    if ((b & 0x20) && (b & 0x40)) {
        c->anim = 1;
        c->dir = 7;
        {
            Vec3 t;

            fld_dir_vec(&t, 7);
            c->vel = t;
        }
    } else if ((b & 0x20) && (b & 0x10)) {
        c->anim = 1;
        c->dir = 5;
        {
            Vec3 t;

            fld_dir_vec(&t, 5);
            c->vel = t;
        }
    } else if ((b & 0x80) && (b & 0x40)) {
        c->anim = 1;
        c->dir = 1;
        {
            Vec3 t;

            fld_dir_vec(&t, 1);
            c->vel = t;
        }
    } else if ((b & 0x80) && (b & 0x10)) {
        c->anim = 1;
        c->dir = 3;
        {
            Vec3 t;

            fld_dir_vec(&t, 3);
            c->vel = t;
        }
    } else if (b & 0x10) {
        c->anim = 1;
        c->dir = 4;
        {
            Vec3 t;

            fld_dir_vec(&t, 4);
            c->vel = t;
        }
    } else if (b & 0x40) {
        c->anim = 1;
        c->dir = 0;
        {
            Vec3 t;

            fld_dir_vec(&t, 0);
            c->vel = t;
        }
    } else if (b & 0x80) {
        c->anim = 1;
        c->dir = 2;
        {
            Vec3 t;

            fld_dir_vec(&t, 2);
            c->vel = t;
        }
    } else if (b & 0x20) {
        c->anim = 1;
        c->dir = 6;
        {
            Vec3 t;

            fld_dir_vec(&t, 6);
            c->vel = t;
        }
    } else {
        c->vel.x = fld_stick_axis(pad_get()->b.analog_x);
        c->vel.y = 0.0f;
        c->vel.z = fld_stick_axis(pad_get()->b.analog_y);
        if (fld_vec3_len(&c->vel) >= 0.5f) {
            float best = 2.0f;
            int i;
            float d;

            mat3_identity(m);
            mat3_rot_y(m, m, 0.7853982f);
            mat3_transform_vec3(&c->vel, m, &c->vel);
            vec3_normalize(&c->vel, &c->vel);
            for (i = 0; i < 8; i++) {
                Vec3 t;

                fld_dir_vec(&t, i);
                u = *(Vec3 *)&t;
                vec3_sub(&w, &u, &c->vel);
                d = fld_vec3_len(&w);
                if (d < best) {
                    best = d;
                    c->dir = i;
                }
            }
            c->anim = 1;
        } else {
            c->anim = 0;
            memset(&c->vel, 0, sizeof(Vec3));
        }
    }
}

/* the step of the player for this frame: its last velocity clipped against the blocked cells, then
   slid along the NPC in front of it (a second NPC in front stops it); run with the run button */
void fld_player_move(void)
{
    FieldChr *c = &field_player.c;
    int flag;
    unsigned int mask;
    Vec3 v;
    Vec3 up;
    Vec3 down;
    float m[9];
    Vec3 side;
    Vec3 to;
    Vec3 t;
    FieldNpc *near[32];
    int n;
    Vec3 q;
    int i;
    float k;

    fld_map_get();
    flag = 0;
    if (region_get() != 0)
        mask = 0x2000;
    else
        mask = 0x4000;
    if (pad_get()->b.down & mask)
        c->run = 1;
    else
        c->run = 0;
    if (pad_get()->b.down & mask)
        flag = 1;
    vec3_normalize(&v, &c->vel);
    vec3_scale(0.1f, &v, &v);
    fld_chr_collide(&v, &v, &c->pos, 0.1f, flag);
    vec3_normalize(&v, &v);
    vec3_scale(0.1f, &v, &v);
    if (c->anim == 1 && fld_vec3_len(&v) > 0.0f)
        c->dir = fld_dir_nearest(&v);
    {
        Vec3 tmp = { 0.0f, 1.0f, 0.0f };
        up = *(Vec3 *)&tmp;
    }
    {
        Vec3 tmp = { 0.0f, -1.0f, 0.0f };
        down = *(Vec3 *)&tmp;
    }
    vec3_normalize(&v, &v);
    vec3_scale(0.1f, &v, &v);
    fld_npc_near(near, &n, (FieldNpc *)c, 0.45f);
    for (i = 0; i < n; i++) {
        vec3_sub(&to, &near[i]->m.c.pos, &c->pos);
        if (vec3_dot(&to, &v) < 0.0f)
            continue;
        if (i > 0) {
            memset(&v, 0, sizeof(v));
            break;
        }
        mat3_identity(m);
        mat3_rot_y(m, m, 1.5707964f);
        mat3_transform_vec3(&side, m, &v);
        k = vec3_dot(&to, &side);
        if (k > 0.0f)
            vec3_cross_nw(&t, &down, &to);
        else if (k < 0.0f)
            vec3_cross_nw(&t, &up, &to);
        else
            vec3_zero(&t);
        vec3_dot(&t, &v);
        vec3_scale(0.1f, &t, &t);
        vec3_add(&q, &c->pos, &t);
        fld_chr_collide(&v, &t, &q, 0.1f, flag);
    }
    if (fld_vec3_len(&v) <= 0.01f)
        memset(&v, 0, sizeof(v));
    c->vel = v;
}

#ifdef NON_MATCHING /* C novo, conferido com o asm. 9 palavras: o original carrega a constante 0.25 antes da conversão de padac[0] */
/* walks the player toward the centre of its target cell (x at +0xAC, z at +0xB0) by steps of 0.1;
   busy drops to 0 once it is within 0.1 */
void fld_player_seek(void)
{
    FieldPlayer *p = &field_player;
    FieldChr *c = &field_player.c;
    Vec3 to;
    Vec3 v;
    Vec3 d;

    {
        char *q = (char *)&to;
        int k = 12;

        if (q)
            do {
                *q++ = 0;
            } while (--k);
    }
    to.x = 0.25f + 0.5f * field_player.padac[0];
    to.z = 0.25f + 0.5f * field_player.padac[1];
    vec3_sub(&d, &to, &c->pos);
    d.y = 0.0f;
    if (fld_vec3_len(&d) <= 0.1f) {
        p->busy = 0;
        v = d;
    } else {
        v = d;
        vec3_normalize(&v, &v);
        vec3_scale(0.1f, &v, &v);
    }
    c->vel = v;
    c->anim = 1;
    if (fld_vec3_len(&c->vel) > 0.0f)
        c->dir = fld_dir_nearest(&c->vel);
}
#endif

void fld_player_input(void);
void fld_player_move(void);
void fld_player_seek(void);
int fld_bg_id(void);
int fld_bg_event(int bg, int cx, int cz);
int fld_story_flag(int id, int arg);
int fld_fx_kind(FieldFx *fx);

#ifdef NON_MATCHING /* C novo, conferido com o asm. 6 palavras: ordem das divisões por 0.5 nos argumentos de fld_bg_event (o original carrega pos.x em $f0 e divide antes de ler pos.z) */
/* per frame: input and step (twice when running), the block in front of the rooms 0x0C/0x25, the walk
   toward a target cell, the cell marks, the effect of the event under the player (padac[2] on) and
   the footstep sound flags on frames 1 and 4 of the walk (padac[3] last frame) */
void fld_player_update(void)
{
    FieldPlayer *p = &field_player;
    FieldChr *c;
    int k;
    int ev;
    int cx;
    int cz;

    fld_ctx_get();
    c = &field_player.c;
    if (field_player.lock != 0) {
        fld_player_input();
        fld_player_move();
        switch (fld_bg_id()) {
        case 12:
        case 37:
            cz = p->c.pos.z / 0.5f;
            cx = p->c.pos.x / 0.5f;
            if (cx == 14 && cz >= 11 && 26 > cz && p->c.pos.x >= 7.25f && c->vel.x > 0.0f)
                c->vel.x = 0.0f;
            break;
        }
        if (c->run != 0) {
            fld_player_mark_cells(0);
            vec3_add(&c->pos, &c->pos, &c->vel);
            fld_player_mark_cells(1);
            fld_player_input();
            fld_player_move();
            switch (fld_bg_id()) {
            case 12:
            case 37:
                cz = p->c.pos.z / 0.5f;
                cx = p->c.pos.x / 0.5f;
                if (cx == 14 && cz >= 11 && 26 > cz && p->c.pos.x >= 7.25f && c->vel.x > 0.0f)
                    c->vel.x = 0.0f;
                break;
            }
        }
    } else {
        c->anim = 0;
        memset(&c->vel, 0, sizeof(Vec3));
    }
    if (p->busy != 0)
        fld_player_seek();
    fld_player_mark_cells(0);
    vec3_add(&c->pos, &c->pos, &c->vel);
    fld_player_mark_cells(1);
    fld_chr_update(c);
    ev = fld_bg_event(fld_bg_id(), c->pos.x / 0.5f, c->pos.z / 0.5f);
    if (ev == 0) {
        if (p->padac[2] != 0)
            fld_player_fx(0, 0);
        p->padac[2] = 0;
    } else {
        switch (ev) {
        case 1:
        default:
            k = 11;
            break;
        case 2:
            k = 12;
            break;
        case 3:
            k = 13;
            break;
        case 4:
            k = 14;
            break;
        }
        if (p->padac[2] == 0 || fld_fx_kind((FieldFx *)c->fx) != k)
            fld_player_fx(k, 0);
        p->padac[2] = 1;
    }
    switch (c->anim) {
    case 0:
        p->padac[3] = -1;
        break;
    case 1:
        if (p->padac[3] != c->frame || p->padac[3] == -1) {
            switch (c->frame) {
            case 1:
                fld_story_flag(0x6259, 0);
                break;
            case 4:
                fld_story_flag(0x625A, 0);
                break;
            }
            p->padac[3] = c->frame;
        }
        break;
    }
}
#endif
