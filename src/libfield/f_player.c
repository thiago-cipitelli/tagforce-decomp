// @range 0x0880AC74-0x0880BB98
// @asm 0x0880AC94-0x0880B010
// @asm 0x0880B330-0x0880B694
// @asm 0x0880B694-0x0880BA30
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
