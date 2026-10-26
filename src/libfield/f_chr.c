// @range 0x08805220-0x08805DF4
// @asm 0x08805664-0x08805708
// @asm 0x088058F8-0x08805930
// @asm 0x08805930-0x08805CBC
/* SD characters of the map: setup (model or sprite by chara kind), per-frame animation counters,
   shadow/draw registration, the eight walking directions and the collision of a moving character
   against the cells of the map. */
#include "libfield.h"

void fld_chr_free_all(void);
void fld_sprites_init(void);                   /* 0x08826BB0 */
void fld_sprites_free(void);                   /* 0x08826BC4 */
void fld_sprites_add(int fx);                    /* 0x08826BD8 */
void fld_pos_set(Vec3 *pos, int x, int z);     /* 0x08807EE0 */
int fld_chr_kind(int id);                      /* 0x08806DF0 */
int fld_chr_tex(int id);                       /* 0x08806E6C */
void fld_load_chr(int id, int n, int arg, FieldChr *c); /* 0x0880C53C */
Vec3 *fld_light_dir(void);                     /* 0x08818878 */
void fld_chr_draw_model(void);                 /* 0x08805DF4 */
void fld_chr_draw_sprite(void);                /* 0x08806200 */
void fld_chr_draw_flat(void);                  /* 0x08806AF0 */
float fld_vec3_len(Vec3 *v);                    /* 0x0880D140 */
FieldMap *fld_map_get(void);                   /* 0x088077E0 */
int fld_map_walkable(int x, int z, int flag);  /* 0x08807844 */
int fld_map_attr(int x, int z);                /* 0x0881CDBC */
void fld_chr_corners(float r, Vec3 *p, Vec3 *a, Vec3 *b, Vec3 *c, Vec3 *d);
void fld_dir_vec(Vec3 *out, int i);
void fld_load_cancel_chr(void);                /* 0x0880C5C4 */

void fld_chr_init(void)
{
    memset(&field_chr_glb, 0, sizeof(FieldChrGlb));
    fld_sprites_init();
}

void fld_chr_term(void)
{
    fld_chr_free_all();
    fld_chr_init();
}

int fld_chr_setup(FieldChr *c, int kind, int id, int x, int z, int type, int dir, int visible,
                  int arg8c, int a, int b, int fx)
{
    c->kind = kind;
    c->id = id;
    memset(c->pad08, 0, 8);
    c->type = type;
    c->dir = dir;
    c->visible = visible;
    c->arg8c = arg8c;
    fld_pos_set(&c->pos, x, z);
    c->anim = 0;
    c->frame = 0;
    c->fx = fx;
    switch (fld_chr_kind(id)) {
    case 0:
        switch (type) {
        case 0:
            fld_load_chr(id, 1, b, c);
        case 1:
            fld_load_chr(id, 0, a, c);
            break;
        }
        break;
    case 1:
        fld_load_gim(1, (void *)a, c);
        fld_load_gim(fld_chr_tex(id), (void *)b, c);
        break;
    case 2:
        fld_load_chr(id, 0, a, c);
        break;
    }
    return 1;
}

void fld_chr_free_all(void)
{
    fld_sprites_free();
}

void fld_chr_reset(FieldChr *c)
{
    c->pad08[0] = 0;
    c->pad08[1] = 0;
}

void fld_chr_update(FieldChr *c)
{
    FieldChrGlb *g = &field_chr_glb;
    unsigned int n;
    Vec3 l;

    if (c->run)
        n = 2;
    else
        n = 3;
    switch (c->anim) {
    case 0:
        if (c->id == 0x3A) {
            if (fld_ctx_get()->frame % 5 == 0)
                c->frame++;
        } else {
            c->frame = 0;
        }
        break;
    case 1:
        if (fld_ctx_get()->frame % n == 0) {
            c->frame++;
            c->frame %= 6;
        }
        break;
    }
    fld_shadow_set(&c->pos, c->shadow);
    vec3_scale(0.2f, &l, fld_light_dir());
    g->light_w = 0;
    g->light = l;
    if (c->fx)
        fld_sprites_add(c->fx);
}

void fld_chr_draw(FieldChr *c)
{
    int alpha = c->kind != 1;

    if (c->visible) {
        if (c->type == 1) {
            switch (c->dir) {
            case 0:
            case 2:
            case 4:
            case 6:
                return;
            }
        }
        switch (fld_chr_kind(c->id)) {
        case 0:
            fld_draw_add(1.0f, c->shadow, 1, alpha, fld_chr_draw_model, c);
            break;
        case 1:
            fld_draw_add(1.0f, c->shadow, 1, alpha, fld_chr_draw_sprite, c);
            break;
        case 2:
            fld_draw_add(0.0f, c->shadow, 0, alpha, fld_chr_draw_flat, c);
            break;
        }
    }
}

void fld_chr_get_pos(Vec3 *out, FieldChr *c)
{
    *out = c->pos;
}

#ifdef NON_MATCHING /* o original calcula o endereço de field_dirs[i] antes de salvar ra/s0; só escalonamento */
/* walking direction i turned by 45 degrees */
void fld_dir_vec(Vec3 *out, int i)
{
    Vec3 v = field_dirs[i];
    float m[9];


    mat3_identity(m);
    mat3_rot_y(m, m, 0.7853982f);
    mat3_transform_vec3(&v, m, &v);
    vec3_normalize(&v, &v);
    *out = v;
}
#endif

/* direction 0..7 closest to v (only the odd ones when diag is 1) */
int fld_dir_from_vec(Vec3 *v, int diag)
{
    float best = -1.0f;
    int r = -1;
    Vec3 n;
    float m[9];
    Vec3 d;
    Vec3 t;
    int i;
    Vec3 *p;
    float l;

    mat3_identity(m);
    mat3_rot_y(m, m, -0.7853982f);
    mat3_transform_vec3(&n, m, v);
    vec3_normalize(&n, &n);
    for (i = 0, p = field_dirs; i < 8; i++, p++) {
        if (diag == 1) {
            switch (i) {
            case 0:
            case 2:
            case 4:
            case 6:
                continue;
            }
        }
        vec3_normalize(&d, p);
        vec3_sub(&t, &n, &d);
        l = fld_vec3_len(&t);
        if (best > l || best == -1.0f) {
            r = i;
            best = l;
        }
    }
    return r;
}

int fld_dir_nearest(Vec3 *v)
{
    float best = -1.0f;
    int r = 0;
    Vec3 n;
    Vec3 d;
    Vec3 t;
    int i;
    float l;

    vec3_normalize(&n, v);
    for (i = 0; i < 8; i++) {
        fld_dir_vec(&d, i);
        vec3_normalize(&d, &d);
        vec3_sub(&t, &n, &d);
        l = fld_vec3_len(&t);
        if (best < 0.0f || l < best) {
            r = i;
            best = l;
        }
    }
    return r;
}

#ifdef NON_MATCHING /* o original tem um bloco morto "li v0,1; b" e o teste x < 450 em bnezl */
int fld_in_range(int x)
{
    if (x >= 100) {
        if (x >= 450)
            return 0;
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* o original copia cada modelo Vec3 para um temporário na pilha antes de d; o mwcc propaga a cópia */
/* clips the motion vel of a character of radius r at pos against blocked cells */
void fld_chr_collide(float r, Vec3 *out, Vec3 *vel, Vec3 *pos)
{
    FieldMap *m = fld_map_get();
    Vec3 v = *vel;
    Vec3 p = *pos;
    Vec3 d;
    Vec3 q;
    Vec3 c[4];
    int i;
    int j;
    int x;
    int z;
    Vec3 *e;

    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0: {
            Vec3 t = { 0.0f, 0.0f, 1.0f };
            d = t;
            break;
        }
        case 1: {
            Vec3 t = { 0.0f, 0.0f, -1.0f };
            d = t;
            break;
        }
        case 2: {
            Vec3 t = { 1.0f, 0.0f, 0.0f };
            d = t;
            break;
        }
        case 3: {
            Vec3 t = { -1.0f, 0.0f, 0.0f };
            d = t;
            break;
        }
        }
        vec3_normalize(&d, &d);
        vec3_scale(r, &d, &d);
        fld_chr_corners(r, &p, &c[0], &c[1], &c[2], &c[3]);
        for (j = 0, e = c; j < 4; j++, e++) {
            vec3_add(&q, e, &d);
            x = q.x / 0.5f;
            z = q.z / 0.5f;
            if (fld_map_walkable(x, z, 1) == 0 || fld_map_attr(x, z) == 5 || q.x < 0.0f || q.z < 0.0f
                || x >= m->w || z >= m->h) {
                switch (i) {
                case 0:
                    if (v.z >= 0.0f)
                        v.z = 0.0f;
                    break;
                case 1:
                    if (v.z <= 0.0f)
                        v.z = 0.0f;
                    break;
                case 2:
                    if (v.x >= 0.0f)
                        v.x = 0.0f;
                    break;
                case 3:
                    if (v.x <= 0.0f)
                        v.x = 0.0f;
                    break;
                }
            }
        }
    }
    *out = v;
}
#endif

/* the four corners of the square of side 1 - 2r (minus a margin) around p */
void fld_chr_corners(float r, Vec3 *p, Vec3 *a, Vec3 *b, Vec3 *c, Vec3 *d)
{
    Vec3 t;
    float s = (0.5f - r) - 0.001f;

    t.y = 0.0f;
    t.x = -0.5f * s;
    t.z = -0.5f * s;
    vec3_add(a, &t, p);
    t.y = 0.0f;
    t.x = 0.5f * s;
    t.z = -0.5f * s;
    vec3_add(b, &t, p);
    t.x = -0.5f * s;
    t.y = 0.0f;
    t.z = 0.5f * s;
    vec3_add(c, &t, p);
    t.x = 0.5f * s;
    t.z = 0.5f * s;
    t.y = 0.0f;
    vec3_add(d, &t, p);
}

void fld_chr_unload(void)
{
    field_chr_glb.unloading = 1;
    fld_load_cancel_chr();
    field_chr_glb.unloading = 0;
}

int fld_chr_unload_done(void)
{
    return !field_chr_glb.unloading;
}
