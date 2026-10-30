// @range 0x0885538C-0x08855B10
// @asm 0x0885554C-0x08855634
// @asm 0x08855A20-0x08855B08
// @rodata 0x08870C34-0x08870D3C
// @data 0x08876E30-0x08877730
/* Character 6 of the duel (two variants): model callbacks (motion ids, camera positions by
   camera id) and the monster movies played for the summon/attack of some cards
   (movie/<card>_summon.pmf, _attack.pmf; "_a" variants outside Japan). */
#include "libduel_draw.h"

void *memset(void *, int, unsigned int);
void qsort(void *base, unsigned int n, unsigned int es, int (*cmp)(const void *, const void *));
void *bsearch(const void *key, const void *base, unsigned int n, unsigned int size, int (*cmp)(const void *, const void *));
int region_get(void);
int ehfile_read_async(int dir, const char *name, void *buf, int offset, int size, int (*cb)(void *, int), void *arg);
int ehfile_make_path_alias(char *out, int dir, const char *name);
void movie_set_memory(unsigned int base, unsigned int size);
int sys_frame_end(void);
int movie_play(const char *path, unsigned int flags, int arg);
void *sceGeEdramGetAddr(void);

int dd_int_cmp(const void *a, const void *b);
void func_0882837C(void);
void func_088283D4(void);
void func_08828470(void *s, int a, int b);
void func_08828528(void *s, int a, int b);
void *dd_alloc_sub(int size);
void dd_free_sub(void *p);

typedef struct FxVec4u {               /* Vec4 without the 16-byte alignment */
    float x, y, z, w;
} FxVec4u;

typedef struct DdCharSetup6 {          /* callbacks of the character setup (func_088282E4) */
    int pad00[5];
    int (*load)(void);                 /* +0x14 */
    int (*unload)(void);               /* +0x18 */
    void (*anim)(void *s, int a, int b); /* +0x1C */
    int pad20;
    int (*motion)(void *s, int motion); /* +0x24 */
    int (*camera)(void *s, FxVec4u *eye, FxVec4u *target, int id); /* +0x28 */
    void (*term)(void *s, int a, int b); /* +0x2C */
} DdCharSetup6;

typedef struct DdCamKey {
    int id;
} DdCamKey;

typedef struct DdCamEntry {            /* 0x20, sorted by id */
    int id;
    float v[7];                        /* eye x,y,z then target x,y,z,w */
} DdCamEntry;

typedef struct DdMovie {               /* 0x10, sorted by (card, kind) */
    int card;                          /* +100000: version outside Japan */
    int kind;                          /* 0 summon, 1 attack */
    char *name;
    int pad;
} DdMovie;

extern int dd_c6b_sorted;
extern char dd_movie_buf[0x40];
extern int dd_movie_sorted;
extern int dd_c6a_sorted;

int dd_c6b_load(void);
int dd_c6b_unload(void);
void dd_c6b_anim(void *s, int a, int b);
int dd_c6b_motion(void *s, int motion);
int dd_c6b_camera(void *s, FxVec4u *eye, FxVec4u *target, int id);
void dd_c6b_term(void *s, int a, int b);
int dd_c6a_load(void);
int dd_c6a_unload(void);
void dd_c6a_anim(void *s, int a, int b);
int dd_c6a_motion(void *s, int motion);
int dd_c6a_camera(void *s, FxVec4u *eye, FxVec4u *target, int id);
void dd_c6a_term(void *s, int a, int b);
char *dd_movie_find(int card, int kind);
int dd_movie_cmp(const void *a, const void *b);
int dd_movie_loaded(void *arg, int r);

DdCamEntry dd_c6b_cams[5] = {
    { 40, { -44.2617378f, 2.21720529f, -11.2490053f, -0.00128113083f, 0.0609360449f, -0.00187225256f, 0.998139083f } },
    { 37, { -8.05303478f, -2.24291468f, -0.757364452f, -0.0545635372f, 0.704998374f, 0.0545636602f, 0.704998493f } },
    { 38, { -25.3654652f, 3.62398314f, -12.0894871f, -0.000238778637f, -2.45441497e-05f, 0.00227811304f, 0.999997377f } },
    { 39, { 0.0f, 0.0f, 44.0f, 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } },
};

DdMovie dd_movies[49] = {
    { 6344, 0, "movie/fwm_summon.pmf" },
    { 6344, 1, "movie/fwm_attack.pmf" },
    { 6486, 0, "movie/rampart_summon.pmf" },
    { 6486, 1, "movie/rampart_attack.pmf" },
    { 106486, 0, "movie/rampart_summon_a.pmf" },
    { 106486, 1, "movie/rampart_attack_a.pmf" },
    { 6467, 0, "movie/shining_fwm_summon.pmf" },
    { 6467, 1, "movie/shining_fwm_attack.pmf" },
    { 6639, 0, "movie/eric_summon.pmf" },
    { 6639, 1, "movie/eric_attack.pmf" },
    { 6394, 0, "movie/jairoid_summon.pmf" },
    { 6394, 1, "movie/jairoid_attack.pmf" },
    { 6127, 0, "movie/oz_summon.pmf" },
    { 6127, 1, "movie/oz_attack.pmf" },
    { 6485, 0, "movie/cyb_summon.pmf" },
    { 6485, 1, "movie/cyb_attack.pmf" },
    { 106485, 0, "movie/cyb_summon_a.pmf" },
    { 106485, 1, "movie/cyb_attack_a.pmf" },
    { 6126, 0, "movie/ojama_summon.pmf" },
    { 6126, 1, "movie/ojama_attack.pmf" },
    { 6540, 0, "movie/armed_summon.pmf" },
    { 6540, 1, "movie/armed_attack.pmf" },
    { 6481, 0, "movie/water_summon.pmf" },
    { 6481, 1, "movie/water_attack.pmf" },
    { 6396, 0, "movie/ctd_summon.pmf" },
    { 6396, 1, "movie/ctd_attack.pmf" },
    { 6397, 0, "movie/ced_summon.pmf" },
    { 6397, 1, "movie/ced_attack.pmf" },
    { 6315, 0, "movie/gear_summon.pmf" },
    { 6315, 1, "movie/gear_attack.pmf" },
    { 6292, 0, "movie/redeye_summon.pmf" },
    { 6292, 1, "movie/redeye_attack.pmf" },
    { 6293, 0, "movie/vg_summon.pmf" },
    { 6293, 1, "movie/vg_attack.pmf" },
    { 6024, 0, "movie/sof_summon.pmf" },
    { 6024, 1, "movie/sof_attack.pmf" },
    { 5185, 0, "movie/dm_summon.pmf" },
    { 5185, 1, "movie/dm_attack.pmf" },
    { 6648, 0, "movie/tris_summon.pmf" },
    { 6648, 1, "movie/tris_attack.pmf" },
    { 6565, 0, "movie/rabiel_summon.pmf" },
    { 6565, 1, "movie/rabiel_attack.pmf" },
    { 6564, 0, "movie/hamon_summon.pmf" },
    { 6564, 1, "movie/hamon_attack.pmf" },
    { 6563, 0, "movie/uria_summon.pmf" },
    { 6563, 1, "movie/uria_attack.pmf" },
    { 4027, 0, "movie/exodia_finish.pmf" },
    { 5224, 0, "movie/ouijaboard_death.pmf" },
    { 105224, 0, "movie/ouijaboard_final.pmf" },
};

DdCamEntry dd_c6a_cams[5] = {
    { 40, { -44.2617378f, 2.21720529f, -11.2490053f, -0.00128113385f, 0.0609360449f, -0.00187225267f, 0.998139083f } },
    { 37, { -8.05303478f, -2.24291468f, -0.757364333f, -0.0545635186f, 0.704998434f, 0.0545636788f, 0.704998434f } },
    { 38, { -25.306572f, 3.61661363f, -12.1139879f, -0.000238748864f, -2.48320393e-05f, 0.00107234565f, 0.999999404f } },
    { 39, { 0.0f, 0.0f, 44.0f, 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } },
};

/* Setup of character 6 (b): the callbacks of its model. */
void dd_chara_setup_06b(DdCharSetup6 *s)
{
    s->load = dd_c6b_load;
    s->unload = dd_c6b_unload;
    s->anim = dd_c6b_anim;
    s->motion = dd_c6b_motion;
    s->camera = dd_c6b_camera;
    s->term = dd_c6b_term;
}

int dd_c6b_load(void)
{
    func_0882837C();
    return 0;
}

int dd_c6b_unload(void)
{
    func_088283D4();
    return 0;
}

void dd_c6b_anim(void *s, int a, int b)
{
    func_08828470(s, a, b);
}

int dd_c6b_motion(void *s, int motion)
{
    switch (motion) {
    case 14: return 29;
    case 7: return 60;
    case 4: return 44;
    case 2: return 71;
    case 1: return 67;
    case 20: return 54;
    case 12: return 65;
    case 19: return 3;
    case 26: return 19;
    case 11: return 64;
    case 6: return 59;
    case 16: return 2;
    case 31: return 38;
    case 8: return 61;
    case 5: return 58;
    case 15: return 11;
    case 32: return 39;
    case 30: return 75;
    case 13: return 0;
    case 29: return 70;
    case 25: return 34;
    case 3: return 72;
    case 22: return 32;
    case 23: return 56;
    case 18: return 12;
    case 10: return 63;
    case 21: return 48;
    case 24: return 49;
    case 9: return 62;
    case 17: return 30;
    case 0: return 68;
    }
    return 0;
}

#ifdef NON_MATCHING /* só o lui do flag de ordenação sai antes do sw s0 do prólogo no original; nenhuma forma testada reproduziu */
int dd_c6b_camera(void *s, FxVec4u *eye, FxVec4u *target, int id)
{
    DdCamKey key;
    DdCamEntry *e;

    if (dd_c6b_sorted == 0) {
        qsort(dd_c6b_cams, 5, sizeof(DdCamEntry), dd_int_cmp);
        dd_c6b_sorted = 1;
    }
    key.id = id;
    e = bsearch(&key, dd_c6b_cams, 5, sizeof(DdCamEntry), dd_int_cmp);
    if (e == 0)
        return 0;
    *eye = *(FxVec4u *)&e->v[0];
    eye->w = 1.0f;
    *target = *(FxVec4u *)&e->v[3];
    return 1;
}
#endif

void dd_c6b_term(void *s, int a, int b)
{
    func_08828528(s, a, b);
}

/* Export (libduel_mgr): starts reading the movie of (card, kind) into dd_movie_buf. */
void libduel_draw_0885563C(int card, int kind)
{
    char *name = dd_movie_find(card, kind);

    if (name != 0)
        ehfile_read_async(3, name, dd_movie_buf, 0, 0, dd_movie_loaded, 0);
}

/* Export (libduel_mgr): plays the movie of (card, kind) full screen; -1 when the card has none. */
int libduel_draw_08855688(int card, int kind, unsigned int flags)
{
    char *name;
    char path[256];
    void *mem;
    int r;

    name = dd_movie_find(card, kind);
    if (name == 0)
        return -1;
    ehfile_make_path_alias(path, 3, name);
    mem = dd_alloc_sub(0x17CC00);
    movie_set_memory((unsigned int)mem, 0x17CC00);
    sys_frame_end();
    r = movie_play(path, flags, 40);
    memset(sceGeEdramGetAddr(), 0, 0x110000);
    movie_set_memory(0, 0);
    dd_free_sub(mem);
    return r != 0;
}

char *dd_movie_find(int card, int kind)
{
    DdMovie *t = dd_movies;
    DdMovie key;
    DdMovie *e;

    if (region_get() != 0)
        card += 100000;
    if (dd_movie_sorted == 0) {
        qsort(t, 49, sizeof(DdMovie), dd_movie_cmp);
        dd_movie_sorted = 1;
    }
    key.card = card;
    key.kind = kind;
    e = bsearch(&key, t, 49, sizeof(DdMovie), dd_movie_cmp);
    if (e == 0) {
        key.card = card - 100000;
        e = bsearch(&key, t, 49, sizeof(DdMovie), dd_movie_cmp);
        if (e == 0)
            return 0;
    }
    return e->name;
}

int dd_movie_cmp(const void *a, const void *b)
{
    int d = ((DdMovie *)a)->card - ((DdMovie *)b)->card;

    switch (d) {
    case 0:
        return ((DdMovie *)a)->kind - ((DdMovie *)b)->kind;
    }
    return d;
}

int dd_movie_loaded(void *arg, int r)
{
    return 1;
}

/* Setup of character 6 (a): the callbacks of its model. */
void dd_chara_setup_06a(DdCharSetup6 *s)
{
    s->load = dd_c6a_load;
    s->unload = dd_c6a_unload;
    s->anim = dd_c6a_anim;
    s->motion = dd_c6a_motion;
    s->camera = dd_c6a_camera;
    s->term = dd_c6a_term;
}

int dd_c6a_load(void)
{
    func_0882837C();
    return 0;
}

int dd_c6a_unload(void)
{
    func_088283D4();
    return 0;
}

void dd_c6a_anim(void *s, int a, int b)
{
    func_08828470(s, a, b);
}

int dd_c6a_motion(void *s, int motion)
{
    switch (motion) {
    case 14: return 5;
    case 7: return 41;
    case 4: return 36;
    case 2: return 59;
    case 1: return 57;
    case 20: return 55;
    case 12: return 46;
    case 19: return 7;
    case 26: return 25;
    case 11: return 45;
    case 6: return 40;
    case 16: return 2;
    case 31: return 28;
    case 8: return 42;
    case 5: return 39;
    case 15: return 3;
    case 32: return 29;
    case 30: return 62;
    case 13: return 0;
    case 29: return 58;
    case 25: return 67;
    case 3: return 60;
    case 22: return 53;
    case 23: return 69;
    case 18: return 8;
    case 10: return 44;
    case 21: return 54;
    case 24: return 68;
    case 9: return 43;
    case 17: return 10;
    case 0: return 70;
    }
    return 0;
}

#ifdef NON_MATCHING /* só o lui do flag de ordenação sai antes do sw s0 do prólogo no original; nenhuma forma testada reproduziu */
int dd_c6a_camera(void *s, FxVec4u *eye, FxVec4u *target, int id)
{
    DdCamKey key;
    DdCamEntry *e;

    if (dd_c6a_sorted == 0) {
        qsort(dd_c6a_cams, 5, sizeof(DdCamEntry), dd_int_cmp);
        dd_c6a_sorted = 1;
    }
    key.id = id;
    e = bsearch(&key, dd_c6a_cams, 5, sizeof(DdCamEntry), dd_int_cmp);
    if (e == 0)
        return 0;
    *eye = *(FxVec4u *)&e->v[0];
    eye->w = 1.0f;
    *target = *(FxVec4u *)&e->v[3];
    return 1;
}
#endif

void dd_c6a_term(void *s, int a, int b)
{
    func_08828528(s, a, b);
}
