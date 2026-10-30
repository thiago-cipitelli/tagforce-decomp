// @range 0x088143A4-0x08814DFC
// @asm 0x088143D4-0x088145B0
// @asm 0x08814A64-0x08814B4C
// @asm 0x08814D0C-0x08814DF4
// @data 0x088755B8-0x08875770
// @rodata 0x0886FF0C-0x08870014
/* Model hooks of the duel-disc and field models: the card texture rendered into VRAM for the disc
   slots ("4006.iff"/"0000.iff"), the material of "duel_disc_02.iff", the per-bone animation callback
   and two hook tables (bone remap + camera/light pose lookup). */
#include "libduel_draw.h"

typedef struct DdAnimBone {
    unsigned short mtx;                /* matrix index, 0xFFFF: none */
    unsigned short unk2[3];
} DdAnimBone;

typedef struct DdAnimHdr {
    int unk0[6];
    DdAnimBone *bones;                 /* +0x18 */
} DdAnimHdr;

typedef struct DdHookAnim {            /* animation set of a model object */
    DdAnimHdr *hdr;                    /* +0x00 */
    int unk4[2];
    float *mats;                       /* +0x0C 0x40 bytes per matrix */
} DdHookAnim;

typedef struct DdHookObj {             /* model object the hooks receive */
    int id;                            /* +0x00 */
    int unk4[10];
    void (*bone_cb)(struct DdHookObj *o, void *arg, float *mtx, int bone, float t); /* +0x2C */
    int unk30;
    int snd;                           /* +0x34 */
    DdHookAnim *anim;                  /* +0x38 */
    int unk3c;
    int kind;                          /* +0x40 */
    int unk44[4];
    int active;                        /* +0x54 */
    int unk58;
    float speed;                       /* +0x5C */
} DdHookObj;

typedef struct DdTexMat {              /* material of a hooked texture */
    const char *name;                  /* +0x00 */
    int unk4;
    short unk8;
    signed char func, comp;            /* +0x0A */
    unsigned int env;                  /* +0x0C */
    signed char wrap_u, wrap_v;        /* +0x10 */
    float u, v, su, sv;                /* +0x14 */
} DdTexMat;

typedef struct DdVec4 {
    float v[4];
} DdVec4;

typedef struct DdMtx {                 /* 4x4 matrix as 16 float members (copied member by member) */
    float xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww;
} __attribute__((aligned(16))) DdMtx;

typedef struct DdPose {                /* camera/light pose table entry (bsearch by id) */
    int id;
    float pos[3];
    float rot[4];
} DdPose;

extern DdHookAnim *dd_hook_default_anim;   /* 0x08C54630 */
extern int dd_pose_a_sorted;               /* 0x08C54634 */
extern int dd_pose_b_sorted;               /* 0x08C54638 */

int func_08865330(void);
int func_0882837C(void);
int func_088283D4(void);
int func_08828470(void *o, int a);
int func_08828528(void *o, int a);
int func_0881DF4C(const void *a, const void *b);
void func_088153E4(int a, int b, int c, int d);
unsigned int *dd_disc_material(unsigned int *list, DdTexMat *m, void *gim);

/* used by dd_disc_tex_hook (kept in asm) */
char dd_str_4006[] = "4006.iff";
char dd_str_0000[] = "0000.iff";
char dd_str_disc02[] = "duel_disc_02.iff";

DdHookAnim *dd_hook_anim(DdHookObj *o)
{
    if (o->id >= 0 && o->anim != 0)
        return o->anim;
    return dd_hook_default_anim;
}

/* texture hook: the disc's card slots get the card picture rendered at VRAM+0x160000 */
#ifdef NON_MATCHING /* o teste de x158 sai por bnez em vez de bnezl direto ao epílogo */
int dd_disc_tex_hook(unsigned int **plist, int *mdl, int mesh, int part, int *arg)
{
    DdTexMat *m = *(DdTexMat **)(mdl[3] + mesh * 0xA4 + part * 0x20 + 0x1C);
    int *st = (int *)arg[0x30 / 4];
    unsigned int *list;

    if (m == 0)
        return 0;
    if (strcmp(dd_str_4006, m->name) == 0) {
        st = (int *)st[0x150 / 4];
    } else if (strcmp(dd_str_0000, m->name) == 0) {
        st = (int *)-1;
    } else if (strcmp(dd_str_disc02, m->name) != 0 || st[0x158 / 4] != 0 || st[8 / 4] == 0) {
        return 0;
    } else {
        *plist = dd_disc_material(*plist, m, (void *)st[8 / 4]);
        return 1;
    }
    list = gfx_set_draw_target(*plist, 0x160000, -1, 0x80, 0x100, 3);
    list = dl_disable(list, 1);
    list = card_draw(list, 0, 0, (int)st);
    list = dl_enable(list, 1);
    list = gfx_set_default_target(list);
    list = dl_tex_flush(list);
    list = dl_tex_func(list, 0, 0);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x80, 0x100, 0x80, (void *)((int)sceGeEdramGetAddr() + 0x160000));
    list = dl_tex_offset(0.0f, 0.6015625f, list);
    list = dl_tex_scale(0.84375f, -0.6015625f, list);
    *plist = dl_tex_wrap(list, 0, 0);
    return 1;
}
#endif

/* bone hook: forwards the bone matrix (or identity) to the object's callback */
void dd_hook_bone(int bone, void *arg, int *ctx)
{
    DdMtx ident = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    DdHookObj *o = (DdHookObj *)ctx[3];
    int still = 0;
    DdHookAnim *a;
    int h;
    unsigned int idx;
    float *mats;
    float *mtx;

    if (o->bone_cb != 0 && o->active != 0) {
        if (charmap_lookup(o->id, 0) < 0) {
            switch (o->kind) {
            case 1:
            case 0x10:
            case 0x11:
            case 0x12:
                return;
            }
        }
        h = snd_channel_get_u32(o->snd);
        if (h == 0)
            return;
        if (bgm_query(h, 1) < 10)
            still = 1;
        a = dd_hook_anim(o);
        mats = a->mats;
        idx = a->hdr->bones[bone].mtx;
        if (idx != 0xFFFF)
            mtx = mats + idx * 16;
        else
            mtx = &ident.xx;
        o->bone_cb(o, arg, mtx, bone, still ? 0.0f : o->speed * (float)(int)gfx_get_state()[1]);
    }
}

int dd_hook_true(void)
{
    return 1;
}

int dd_hook_8147d0(void)
{
    return func_08865330() != 0;
}

/* material of "duel_disc_02.iff": the deck texture, or texturing off */
unsigned int *dd_disc_material(unsigned int *list, DdTexMat *m, void *gim)
{
    if (m != 0 && gim != 0) {
        list = dl_enable(list, 9);
        list = gim_set_texture_frame(list, gim, 0);
        list = dl_tex_func(list, m->func, m->comp);
        list = dl_tex_env_color(list, m->env);
        list = dl_tex_wrap(list, m->wrap_u, m->wrap_v);
        list = dl_tex_offset(m->u, 1.0f - m->v, list);
        return dl_tex_scale(m->su, -m->sv, list);
    }
    return dl_disable(list, 9);
}

DdPose dd_pose_a[5] = {
    { 40, { -44.26263f, 2.2257454f, -11.231956f }, { -0.0065024323f, 0.06097078f, -0.0015242406f, 0.9981172f } },
    { 37, { -7.783676f, -2.086039f, -0.7466821f }, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, { -25.366402f, 3.6329231f, -12.072714f }, { 8.37e-09f, -1.08e-09f, 0.0022780737f, 0.9999974f } },
    { 39, { 0.0f, 0.0f, 44.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } },
};

int dd_hooka_init(void *o);
int dd_hooka_term(void *o);
int dd_hooka_bone_cb(void *o, int a);
int dd_hooka_bone_map(void *o, int bone);
int dd_hooka_pose(void *o, float *pos, float *rot, int id);

/* hook table of the first field model */
void dd_hooka_install(void **h)
{
    h[0x14 / 4] = dd_hooka_init;
    h[0x18 / 4] = dd_hooka_term;
    h[0x1C / 4] = dd_hooka_bone_cb;
    h[0x24 / 4] = dd_hooka_bone_map;
    h[0x28 / 4] = dd_hooka_pose;
}

int dd_hooka_init(void *o)
{
    func_0882837C();
    return 0;
}

int dd_hooka_term(void *o)
{
    func_088283D4();
    return 0;
}


int dd_hooka_bone_cb(void *o, int a)
{
    return func_08828470(o, a);
}

/* model bone -> skeleton bone */
int dd_hooka_bone_map(void *o, int bone)
{
    switch (bone) {
    case 14: return 0x10;
    case 7: return 0x28;
    case 4: return 0x2E;
    case 2: return 0x39;
    case 1: return 0x35;
    case 20: return 0x30;
    case 12: return 0x2D;
    case 19: return 0x3;
    case 26: return 0x7;
    case 11: return 0x2C;
    case 6: return 0x27;
    case 16: return 0x2;
    case 31: return 0x19;
    case 8: return 0x29;
    case 5: return 0x26;
    case 15: return 0x5;
    case 32: return 0x1A;
    case 30: return 0x3D;
    case 13: return 0x0;
    case 29: return 0x38;
    case 25: return 0x15;
    case 3: return 0x3A;
    case 22: return 0x13;
    case 23: return 0x32;
    case 18: return 0x6;
    case 10: return 0x2B;
    case 21: return 0x22;
    case 24: return 0x23;
    case 9: return 0x2A;
    case 17: return 0x11;
    case 0: return 0x36;
    default: return 0;
    }
}

#ifdef NON_MATCHING /* lui do flag agendado depois do sw s0 */
int dd_hooka_pose(void *o, float *pos, float *rot, int id)
{
    DdPose *e;
    int key;

    if (dd_pose_a_sorted == 0) {
        qsort(dd_pose_a, 5, sizeof(DdPose), func_0881DF4C);
        dd_pose_a_sorted = 1;
    }
    key = id;
    e = bsearch(&key, dd_pose_a, 5, sizeof(DdPose), func_0881DF4C);
    if (e == 0)
        return 0;
    *(DdVec4 *)pos = *(DdVec4 *)e->pos;
    pos[3] = 1.0f;
    *(DdVec4 *)rot = *(DdVec4 *)e->rot;
    return 1;
}
#endif
DdPose dd_pose_b[5] = {
    { 40, { -44.292492f, 2.2225974f, -11.174187f }, { -0.0011785935f, 0.062230006f, -0.0018646852f, 0.9980594f } },
    { 37, { -7.9361987f, -2.2903676f, -0.7556554f }, { 0.054560795f, -0.70503765f, -0.054566864f, -0.7049592f } },
    { 38, { -25.343857f, 3.62176f, -12.086877f }, { -0.00013577816f, 0.0012709051f, -0.0017187565f, 0.99999774f } },
    { 39, { 0.0f, 0.0f, 44.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, { -0.16259804f, 1.6453018f, -110.260826f }, { -0.00013507096f, 0.0012709805f, -0.0011623177f, 0.9999985f } },
};

int dd_hookb_init(void *o);
int dd_hookb_term(void *o);
int dd_hookb_bone_cb(void *o, int a);
int dd_hookb_bone_map(void *o, int bone);
int dd_hookb_pose(void *o, float *pos, float *rot, int id);
int dd_hookb_2c(void *o, int a);

/* hook table of the second field model */
void dd_hookb_install(void **h)
{
    h[0x14 / 4] = dd_hookb_init;
    h[0x18 / 4] = dd_hookb_term;
    h[0x1C / 4] = dd_hookb_bone_cb;
    h[0x24 / 4] = dd_hookb_bone_map;
    h[0x28 / 4] = dd_hookb_pose;
    h[0x2C / 4] = dd_hookb_2c;
}

int dd_hookb_init(void *o)
{
    func_0882837C();
    return 0;
}

int dd_hookb_term(void *o)
{
    func_088283D4();
    return 0;
}


int dd_hookb_bone_cb(void *o, int a)
{
    return func_08828470(o, a);
}

/* model bone -> skeleton bone */
int dd_hookb_bone_map(void *o, int bone)
{
    switch (bone) {
    case 14: return 0x10;
    case 7: return 0x3F;
    case 4: return 0x25;
    case 2: return 0x4A;
    case 1: return 0x46;
    case 20: return 0x39;
    case 12: return 0x44;
    case 19: return 0x3;
    case 26: return 0x7;
    case 11: return 0x43;
    case 6: return 0x3E;
    case 16: return 0x2;
    case 31: return 0x19;
    case 8: return 0x40;
    case 5: return 0x3D;
    case 15: return 0x5;
    case 32: return 0x1A;
    case 30: return 0x4E;
    case 13: return 0x0;
    case 29: return 0x49;
    case 25: return 0x15;
    case 3: return 0x4B;
    case 22: return 0x13;
    case 23: return 0x3B;
    case 18: return 0x6;
    case 10: return 0x42;
    case 21: return 0x2C;
    case 24: return 0x2D;
    case 9: return 0x41;
    case 17: return 0x11;
    case 0: return 0x47;
    default: return 0;
    }
}

#ifdef NON_MATCHING /* lui do flag agendado depois do sw s0 */
int dd_hookb_pose(void *o, float *pos, float *rot, int id)
{
    DdPose *e;
    int key;

    if (dd_pose_b_sorted == 0) {
        qsort(dd_pose_b, 5, sizeof(DdPose), func_0881DF4C);
        dd_pose_b_sorted = 1;
    }
    key = id;
    e = bsearch(&key, dd_pose_b, 5, sizeof(DdPose), func_0881DF4C);
    if (e == 0)
        return 0;
    *(DdVec4 *)pos = *(DdVec4 *)e->pos;
    pos[3] = 1.0f;
    *(DdVec4 *)rot = *(DdVec4 *)e->rot;
    return 1;
}
#endif

int dd_hookb_2c(void *o, int a)
{
    return func_08828528(o, a);
}
