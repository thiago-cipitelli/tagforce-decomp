// @range 0x0883AB6C-0x0883B62C
// @asm 0x0883AC7C-0x0883B62C
/* The tunnel of effects 27-31, 38-41 and 54: a disc of 33 textured rings that scrolls toward the
   center, with light streaks bursting at random angles (task of 0x67CC bytes). */
#include "dd_fx.h"

int dd_tunnel_update(int phase, DdTunnelFx *d);

/* grow time and length of the tunnel by effect id */
void dd_tunnel_kind(int id, int *grow, int *end)
{
    switch (id) {
    case 27: case 28: case 29: case 30: case 31: case 38: case 39:
        *grow = 10;
        break;
    case 40: case 54:
        *grow = 18;
        break;
    case 41:
        *grow = 16;
        break;
    default:
        *grow = 0;
        *end = 0;
        return;
    }
    *end = 50;
}

int dd_fx27_start(DdFxSlot *s, int a, int b)
{
    int task = func_0882623C(dd_tunnel_update, a, b, sizeof(DdTunnelFx));
    DdTunnelFx *d;

    if (task != -1) {
        d = func_08826440(task);
        d->slot = s;
    }
    return task;
}

#ifdef NON_MATCHING /* função de 2,4 KB: alocação de registradores e laços desenrolados diferem */
int dd_tunnel_update(int phase, DdTunnelFx *d)
{
    DdFxSlot *s = d->slot;
    int run = s->param;
    DdRandSeed *r;
    DdStreak *st;
    DdTCVtx *v;
    void *list;
    float in;
    float out;
    float fr;
    float fj;
    float radius;
    float e;
    float ang;
    float u;
    unsigned int alpha;
    int i;
    int j;

    func_08826E30(7, 240, 136);
    if (phase == 1)
        dd_tunnel_kind(s->id, &d->grow_end, &d->end);
    in = func_08827A2C(d->t, 8, 1);
    out = func_088279A4(d->t - (d->end - 8), 8);
    func_08827F68(0x6236, d->grow, d->grow_end, &d->se_done, 1);

    /* the disc */
    if (phase == 1) {
        for (i = 0; i < 33; i++) {
            fr = i / 32.0f;
            radius = fr * -600.0f + 800.0f;
            alpha = (unsigned int)((1.0f - fr) * 255.0f);
            for (j = 0; j < 33; j++) {
                v = &d->grid[i][j];
                fj = j / 32.0f;
                ang = fj * 6.2831855f;
                v->u = fj * 3.0f;
                v->v = fr * 0.5f;
                v->color = alpha << 24 | 0xFF00C4;
                v->x = radius * sinf(ang);
                v->y = radius * cosf(ang);
                v->z = 0.0f;
            }
        }
        for (i = 0; i < 33; i++) {
            d->idx[i * 2] = i;
            d->idx[i * 2 + 1] = i + 33;
        }
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = func_0882698C(list, 0, 0);
        list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
        list = func_08826A64(list, 9);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        list = dl_ambient_light(list, (unsigned int)(in * 255.0f * (1.0f - out)) << 24 | 0xFFFFFF);
        list = dl_tex_offset(0.0f, d->frame * 0.03f, list);
        for (i = 0; i < 32; i++)
            list = draw_array(list, 4, 0x99F, 66, d->idx, d->grid[i]);
        mtx_pop();
        dl_set_list(list);
    }

    /* the streaks */
    if (phase == 1) {
        v = d->streak_quad;
        v[0].u = 0.0f;  v[0].v = 0.0f;  v[0].color = 0xFFFFFF;   v[0].x = -50.0f;  v[0].y = -200.0f; v[0].z = 0.0f;
        v[1].u = 0.0f;  v[1].v = 0.2f;  v[1].color = 0xFFFFFF;   v[1].x = -100.0f; v[1].y = -400.0f; v[1].z = 0.0f;
        v[2].u = 0.1f;  v[2].v = 0.0f;  v[2].color = 0xFFFFFF;   v[2].x = 0.0f;    v[2].y = -200.0f; v[2].z = 0.0f;
        v[3].u = 0.1f;  v[3].v = 0.2f;  v[3].color = 0xFFFFFFFF; v[3].x = 0.0f;    v[3].y = -400.0f; v[3].z = 0.0f;
        v[4].u = 0.2f;  v[4].v = 0.0f;  v[4].color = 0xFFFFFF;   v[4].x = 50.0f;   v[4].y = -200.0f; v[4].z = 0.0f;
        v[5].u = 0.2f;  v[5].v = 0.2f;  v[5].color = 0xFFFFFF;   v[5].x = 100.0f;  v[5].y = -400.0f; v[5].z = 0.0f;
    }
    if (d->grow == d->grow_end) {
        st = &d->streak[d->streak_next];
        st->t = 0;
        st->flags |= 1;
        st->ang[0] = DD_RAND(r) / 32767.0f * 6.2831855f;
        st->ang[1] = DD_RAND(r) / 32767.0f * 6.2831855f;
        st->ang[2] = DD_RAND(r) / 32767.0f * 6.2831855f;
        st->ang[3] = DD_RAND(r) / 32767.0f * 6.2831855f;
        if (d->streak_next < 7)
            d->streak_next++;
        else
            d->streak_next = 0;
    }
    if (d->grow == d->grow_end) {
        list = dl_select_layer(7, 0);
        list = func_088266D8(list, 1, 1, 1, 0, 0, 0, 0);
        list = func_0882698C(list, 0, 0);
        list = func_088269D8(list, 1, 0, 6, 10, 0, -1);
        list = func_08826A64(list, 23);
        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        for (i = 0; i < 8; i++) {
            st = &d->streak[i];
            if (!(st->flags & 1))
                continue;
            e = func_08827A2C(st->t, 8, 1);
            list = dl_ambient_light(list, (unsigned int)((1.0f - out) * func_08827AD4(st->t, 8, 1) * 255.0f) << 24
                                              | 0xFFFFFF);
            for (j = 0; j < 4; j++) {
                u = DD_RAND(r) / 32767.0f;
                list = dl_tex_offset(DD_RAND(r) / 32767.0f, u, list);
                mtx_push();
                mtx_rotate_z(st->ang[j]);
                func_088272BC(e * 0.5f + 0.5f, 1.0f, 0.0f);
                list = draw_array(list, 4, 0x19F, 6, 0, d->streak_quad);
                mtx_pop();
            }
            if (st->t < 7)
                st->t++;
            else
                st->flags &= ~1;
        }
        mtx_pop();
        dl_set_list(list);
    }
    func_08826FEC(7);
    d->frame++;
    if (run == 1) {
        if (d->grow < d->grow_end)
            d->grow++;
        else
            d->t++;
    }
    return d->t < d->end ? 2 : 4;
}
#endif
