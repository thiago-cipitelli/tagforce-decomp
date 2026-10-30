// @range 0x0881BA90-0x0881C240
// @asm 0x0881BAB0-0x0881BEA8
// @asm 0x0881C03C-0x0881C1FC
// @data 0x0883C9C8-0x0883C9E0
/* The partner walking behind the player on the map (field_partner): it follows the position the player
   had on the previous frame, slides along the NPCs it bumps into and marks the cells it leaves and
   enters. */
#include "libfield.h"

FieldChr *fld_player_get(void);
void fld_chr_update(FieldChr *c);
int fld_dir_nearest(Vec3 *v);
/* the caller passes a fifth argument (0) that fld_chr_collide does not read */
void fld_chr_collide(Vec3 *out, Vec3 *vel, Vec3 *pos, float r, int flag);
void fld_npc_near(FieldNpc **out, int *n, FieldNpc *p, float r);
int fld_map_walkable(int x, int z, int flag);
int fld_cell_free(int x, int z);
Vec3 *vec3_cross_nw(Vec3 *out, Vec3 *a, Vec3 *b);
Vec3 *vec3_zero(Vec3 *out);

void fpt_face(void);
void fpt_loaded_a(int *file, FieldChr *c);
void fpt_loaded_b(int *file, FieldChr *c);
void fpt_place(void);

/* side vectors of fpt_update (kept in asm) */
Vec3 fpt_up = { 0.0f, 1.0f, 0.0f };
Vec3 fpt_down = { 0.0f, -1.0f, 0.0f };

void fpt_clear(void)
{
    memset(&field_partner, 0, sizeof(field_partner));
}

void fpt_reset(void)
{
    fld_chr_reset(&field_partner.c);
}

#ifdef NON_MATCHING /* lógica conferida (fpt_up/fpt_down são os modelos locais up/down). 26 palavras: mov.s $f12 depois dos ponteiros em vec3_scale (speed variável) e ordem das cópias de c->vel e das divisões por 0.5 */
void fpt_update(void)
{
    FieldPartner *p = &field_partner;
    FieldChr *c = &field_partner.c;
    FieldChr *pl = fld_player_get();
    float speed = 0.1f;
    Vec3 e;
    Vec3 d;
    FieldNpc *near[32];
    Vec3 up;
    Vec3 down;
    float m[9];
    Vec3 side;
    Vec3 to;
    Vec3 t;
    int n;
    int i;
    float k;

    if (field_partner.active == 0)
        return;
    if ((c->run = pl->run) != 0)
        speed *= 2.0f;
    vec3_sub(&d, &p->last, &c->pos);
    e = d;
    d.y = 0.0f;
    if (fld_vec3_len(&e) < 1.0f) {
        vec3_normalize(&d, &d);
        vec3_scale(speed, &d, &d);
        fld_chr_collide(&d, &d, &c->pos, speed, 0);
        vec3_normalize(&d, &d);
        vec3_scale(speed, &d, &d);
        {
            Vec3 tmp = { 0.0f, 1.0f, 0.0f };
            up = *(Vec3 *)&tmp;
        }
        {
            Vec3 tmp = { 0.0f, -1.0f, 0.0f };
            down = *(Vec3 *)&tmp;
        }
        fld_npc_near(near, &n, (FieldNpc *)c, 0.5f);
        for (i = 0; i < n; i++) {
            vec3_sub(&to, &near[i]->m.c.pos, &c->pos);
            if (vec3_dot(&to, &d) < 0.0f)
                continue;
            mat3_identity(m);
            mat3_rot_y(m, m, 1.5707964f);
            mat3_transform_vec3(&side, m, &d);
            k = vec3_dot(&to, &side);
            if (k > 0.0f)
                vec3_cross_nw(&t, &down, &to);
            else if (k < 0.0f)
                vec3_cross_nw(&t, &up, &to);
            else
                vec3_zero(&t);
            vec3_dot(&t, &d);
            vec3_scale(speed, &t, &t);
            d = t;
        }
    }
    if (fld_vec3_len(&e) >= 0.5f) {
        vec3_normalize(&d, &d);
        vec3_scale(speed, &d, &d);
        c->vel = d;
        fld_cell_mark(c->pos.x / 0.5f, c->pos.z / 0.5f, 0);
        vec3_add(&c->pos, &c->pos, &c->vel);
        fld_cell_mark(c->pos.x / 0.5f, c->pos.z / 0.5f, 2);
        if (fld_vec3_len(&d) > 0.0f)
            c->anim = 1;
        else
            c->anim = 0;
        c->dir = fld_dir_nearest(&c->vel);
    } else {
        c->anim = 0;
    }
    p->last = pl->pos;
    fpt_face();
    fld_chr_update(c);
}
#endif

void fpt_draw(void)
{
    FieldPartner *p = &field_partner;
    FieldChr *c = &field_partner.c;

    if (p->file_a != 0 && *p->file_a != 0 && p->file_b != 0 && *p->file_b != 0)
        fld_chr_draw(c);
}

void fpt_setup(int kind, int id)
{
    fld_chr_setup(&field_partner.c, kind, id, 0, 0, 0, 0, 1, 0, (int)fpt_loaded_a, (int)fpt_loaded_b, 0);
    fpt_place();
    fld_map_height(field_partner.c.pos.x, field_partner.c.pos.z, &field_partner.c.pos);
    field_partner.active = 1;
}

int fpt_ready(void)
{
    FieldChr *c = &field_partner.c;

    if (c->pad08[0] && c->pad08[1])
        return 1;
    return 0;
}

void fpt_free(void)
{
    field_partner.active = 0;
    fld_unload(0, field_partner.c.id);
    fld_unload(1, field_partner.c.id);
}

void fpt_loaded_a(int *file, FieldChr *c)
{
    field_partner.file_a = file;
    if (file)
        c->pad08[0] = *file;
}

void fpt_loaded_b(int *file, FieldChr *c)
{
    field_partner.file_b = file;
    if (file)
        c->pad08[1] = *file;
}

/* the first walkable cell around the player, starting behind it */
#ifdef NON_MATCHING /* the two divisions of the cell index are scheduled around the li of ok in another order */
void fpt_place(void)
{
    FieldChr *c = &field_partner.c;
    FieldChr *pl = fld_player_get();
    int dir = (pl->dir + 4) % 8;
    int i;
    Vec3 p;
    Vec3 v;
    Vec3 t;
    int x;
    int z;
    int ok;

    for (i = 0; i < 8; i++) {
        p = pl->pos;
        fld_dir_vec(&t, dir);
        v = t;
        vec3_scale(0.5f, &v, &v);
        vec3_add(&p, &p, &v);
        x = p.x / 0.5f;
        z = p.z / 0.5f;
        ok = 1;
        ok &= fld_map_walkable(x, z, 0);
        ok &= fld_cell_free(x, z);
        ok &= fld_map_attr(x, z) != 3;
        if (ok) {
            c->dir = dir;
            c->pos = p;
            break;
        }
        if (++dir == 8)
            dir = 0;
    }
    if (i == 8) {
        c->pos = pl->pos;
        c->dir = pl->dir;
    }
}
#endif

void fpt_face(void)
{
    FieldChr *c = &field_partner.c;
    FieldChr *pl = fld_player_get();
    Vec3 d;

    vec3_sub(&d, &pl->pos, &c->pos);
    c->dir = fld_dir_nearest(&d);
}
