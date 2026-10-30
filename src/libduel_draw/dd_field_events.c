// @range 0x08820138-0x08820F88
// @asm 0x08820CC0-0x08820D40
// @data 0x088760D0-0x08876100
/* Field effects driven by duel events: the LP change number, the deck shuffle (coins flying to the
   deck), coin toss and die roll, fusion, the orbs, the ring and ring glow, the wait-for-condition
   task, rotation angles from a matrix/quaternion (card marks and glows follow the cards) and the
   opening of the action window. */
#include "dd_field_ui.h"

typedef struct DdTask DdTask;
struct DdTask {                        /* 0x1C */
    DdTask *next;
    DdTask *prev;
    void (*draw)(DdTask *t, void *arg);
    void (*update)(float dt, DdTask *t, void *arg);
    void (*leave)(DdTask *t, void *arg);
    float time;                        /* +0x14 */
    void *arg;                         /* +0x18 */
};

typedef struct DdTasksTail {           /* the end of DdTasks (0x08C57A10, dd_task.c) */
    char pad[0x8A0];
    int coin_busy;                     /* +0x8A0 the shuffle coins are flying */
} DdTasksTail;

typedef struct DdScrPos {
    int x, y, z;
} DdScrPos;

typedef struct DdWait {                /* argument of the wait-for-condition task */
    int flag;                          /* request flag raised while waiting */
    int (*cond)(void);
    int want;                          /* wait until (cond() != 0) == (want != 0) */
    int pad;
} DdWait;

extern DdTasksTail dd_tasks;
extern const int dd_glow_kind;
extern int D_08C582B0;
extern int D_08C582B4;
extern int D_08C582B8;

void *dd_alloc(int n);
int dd_free(void *p);
DdTask *dd_task_add(DdTask *t);
int dd_task_remove_cur(void);
void dd_task_remove(DdTask *t);
int dd_task_pause(int h);
DdTask *dd_flag_task(int flag);
void dd_flag_task_done(DdTask *t);
void dd_task_free_pair(int *p);
int dd_cobjs_busy(void);
int dd_cobjs_idle(void);
void dd_cobjs_en_set(int i, int on);
int dd_zone_count(int player, int zone);
float dd_speed_get(void);
unsigned short *dd_text(int id);
void dd_info_disable(int off);
void dd_mode_push(int m);
void dd_mode_set_card(int card);
void dd_mode_set_flag(int f);
int region_get(void);
int dd_fx_lpjump_start_move(DdScrPos *pos, int value, unsigned int r, unsigned int g, unsigned int b, unsigned int a, DdScrPos *to, void (*term)(int *), void *arg);
int dd_fx_coinfly_start(int side, int height, void (*term)(int *), void *arg);
int dd_fx_coin_start(int count, int *result, void (*term)(DdTask *), void *arg);
int dd_fx_die_start(int count, int *result, int kind, void (*term)(DdTask *), void *arg);
int dd_fx_fusion_start(int n, int *cards, int result, void (*term)(DdTask *), void *arg);
int dd_orbs_start(int count, int flag, void (*term)(DdTask *), void *arg);
int dd_ring_start(int count, int a, int b);
int dd_ring_done(int task);
int dd_fx_ringglow_start(DdScrPos *pos, void *term, void *arg);
int dd_fx_ringglow_done(int id);
int dd_fx_spin_move(int id, float *pos, float *rot, float s);
void dd_slot_xform(float *pos, float *rot, short at);
float asinf_vfpu(float x);
float atan2f_vfpu(float y, float x);
float fmaxf_vfpu(float a, float b);
float fminf_vfpu(float a, float b);
float *mat4_identity(float *out);
float *mat4_translate(float *out, float *m, float *v);
float *quat_mul(float *out, float *a, float *b);
void func_088202A0(int *p);
void func_088202E4(float dt, DdTask *t, void *arg);
void func_08820344(int player);
void func_08820524(float dt, DdTask *t, DdWait *w);
void func_08820594(DdTask *t, void *arg);
void func_08820770(float dt, DdTask *t, int h);
void func_08820808(float dt, DdTask *t, int h);
void func_08820840(float dt, DdTask *t, int h);
int func_0882092C(void);
void func_08820938(DdTask *t, void *arg);
void func_08820980(DdCard *c);
void func_08820A8C(float *out, float *q, int back);
void func_08820E34(int on);
void func_08820F28(void);
void func_08820F58(void);
int func_088226C0(unsigned short *text, int a1, int a2);
void func_088257E4(short x, short y);
int *func_08825818(void);
int func_08825A14(void);
void func_08825B88(int on);

/* LP change number jumping from the player's LP panel (red: damage, green: recovery) */
void func_08820138(int player, int delta)
{
    DdScrPos from[2] = { { 314, 210, 0 }, { 314, 80, 0 } };
    DdScrPos to[2] = { { 190, 6, 0 }, { 460, 6, 0 } };
    unsigned int r, g, b, a;
    DdTask *t;
    int *arg;

    if (delta < 0) {
        r = 255;
        g = 60;
        b = 60;
        a = 255;
    } else {
        r = 0;
        g = 239;
        b = 187;
        a = 255;
    }
    t = dd_flag_task(14);
    arg = dd_alloc(12);
    arg[0] = (int)t;
    arg[1] = player;
    arg[2] = delta;
    dd_fx_lpjump_start_move(&from[player], delta, r, g, b, a, &to[player], dd_task_free_pair, arg);
}

/* end of the shuffle coins */
void func_088202A0(int *p)
{
    dd_cobjs_en_set(p[1], 1);
    dd_task_remove((DdTask *)p[0]);
    dd_free(p);
    func_08820E34(0);
}

/* waits until the field cards are idle to start the shuffle */
void func_088202E4(float dt, DdTask *t, void *arg)
{
    int player = (int)t->arg;

    if (dd_cobjs_busy() != 0 || dd_cobjs_idle() == 0) {
        req_flag_set(0x13);
    } else {
        func_08820344(player);
        dd_task_remove_cur();
    }
}

/* deck shuffle: coins fly over the deck (or a task waits for the cards to stop) */
void func_08820344(int player)
{
    DdTask t;
    int n;
    int *arg;
    DdTask *flag;

    if (dd_cobjs_busy() == 0 && dd_cobjs_idle() != 0) {
        arg = dd_alloc(8);
        n = dd_zone_count(player, 13);
        flag = dd_flag_task(0x13);
        dd_cobjs_en_set(player, 0);
        arg[0] = (int)flag;
        arg[1] = player;
        dd_fx_coinfly_start(player == 0, n, func_088202A0, arg);
        func_08820E34(1);
    } else {
        memset(&t, 0, sizeof(DdTask));
        t.update = func_088202E4;
        t.arg = (void *)player;
        dd_task_add(&t);
        req_flag_set(0x13);
    }
}

/* coin toss: bit i of bits set = coin i shows tails */
void func_08820430(int a0, int count, int a2, unsigned int bits)
{
    int res[3] = { 0 };

    res[0] = !(bits & 1);
    res[1] = !(bits & 2);
    res[2] = !(bits & 4);
    dd_fx_coin_start(count, res, dd_flag_task_done, dd_flag_task(0x37));
}

/* die roll */
void func_088204BC(int player, int value)
{
    int res[2] = { 0 };

    res[0] = value;
    dd_fx_die_start(1, res, !player, dd_flag_task_done, dd_flag_task(0x38));
}

/* wait task: ends when the condition reaches the wanted value */
void func_08820524(float dt, DdTask *t, DdWait *w)
{
    req_flag_set(w->flag);
    if ((w->want != 0 && w->cond() != 0) || (w->want == 0 && w->cond() == 0))
        dd_task_remove_cur();
}

void func_08820594(DdTask *t, void *arg)
{
    func_08820938(t, arg);
}

/* raises flag until cond() becomes want (nothing if it already is) */
void func_0882059C(int flag, int (*cond)(void), int want)
{
    DdTask t;
    DdWait *w;

    if ((want != 0 && cond() != 0) || (want == 0 && cond() == 0))
        return;
    req_flag_set(flag);
    w = dd_alloc(sizeof(DdWait));
    memset(w, 0, sizeof(DdWait));
    w->flag = flag;
    w->cond = cond;
    w->want = want;
    memset(&t, 0, sizeof(DdTask));
    t.update = (void (*)(float, DdTask *, void *))func_08820524;
    t.leave = func_08820594;
    t.arg = w;
    dd_task_add(&t);
}

/* fusion effect */
void func_08820678(int result, int *cards, int n)
{
    dd_fx_fusion_start(n, cards, result, dd_flag_task_done, dd_flag_task(0x35));
}

/* the orbs (mode 0), or the tutorial card shown on the fourth one */
void func_088206D4(int n, int mode)
{
    DdTask *t;

    if (mode != 0) {
        if (sysvar_get_a98() == 0 && cfg_get_flag(0) != 0 && n == 4) {
            dd_mode_set_card(0x1468);
            dd_mode_push(11);
        }
    } else {
        t = dd_flag_task(0x3B);
        dd_orbs_start(n + 1, region_get() == 0, dd_flag_task_done, t);
    }
}

void func_08820770(float dt, DdTask *t, int h)
{
    if (dd_ring_done(h) != 0)
        dd_task_remove_cur();
    req_flag_set(0x3C);
}

/* the ring, with a task raising flag 0x3C until it ends */
void func_088207A8(void)
{
    DdTask t;
    int h;

    h = dd_ring_start(20, 0, 0);
    memset(&t, 0, sizeof(DdTask));
    t.update = (void (*)(float, DdTask *, void *))func_08820770;
    t.arg = (void *)h;
    t.draw = 0;
    t.leave = 0;
    dd_task_add(&t);
}

void func_08820808(float dt, DdTask *t, int h)
{
    if (dd_fx_ringglow_done(h) != 0)
        dd_task_remove_cur();
    req_flag_set(0x39);
}

/* the ring glow lasts one second (game speed), then waits for its end */
void func_08820840(float dt, DdTask *t, int h)
{
    if (t->time >= 1.0f / dd_speed_get()) {
        dd_task_pause(h);
        t->update = (void (*)(float, DdTask *, void *))func_08820808;
    }
    req_flag_set(0x39);
}

/* ring glow at the centre of the screen */
void func_088208B0(void)
{
    DdTask t;
    DdScrPos pos;
    int h;

    pos.x = 314;
    pos.y = 132;
    pos.z = 0;
    h = dd_fx_ringglow_start(&pos, 0, 0);
    memset(&t, 0, sizeof(DdTask));
    t.update = (void (*)(float, DdTask *, void *))func_08820840;
    t.arg = (void *)h;
    t.draw = 0;
    t.leave = 0;
    dd_task_add(&t);
    req_flag_set(0x39);
}

int func_0882092C(void)
{
    return D_08C582B0;
}

/* default task leave: frees the task argument */
void func_08820938(DdTask *t, void *arg)
{
    if (arg != 0) {
        dd_free(arg);
        if (arg == t->arg)
            t->arg = 0;
    }
}

void func_08820980(DdCard *c)
{
    c->fx = 0;
}

/* euler angles (x, y, z, 0) of a rotation matrix; near +-90 degrees of pitch the yaw goes to z */
void func_08820988(float *out, float *m)
{
    float s;

    s = asinf_vfpu(-fminf_vfpu(fmaxf_vfpu(m[2], -1.0f), 1.0f));
    out[1] = s;
    if ((s >= 1.5393804f && s <= 1.6022123f) || (s >= -1.6022123f && s <= -1.5393804f)) {
        out[0] = 0.0f;
        out[2] = -atan2f_vfpu(m[4], m[5]);
    } else {
        out[0] = atan2f_vfpu(m[6], m[10]);
        out[2] = atan2f_vfpu(m[1], m[0]);
    }
    out[3] = 0.0f;
}

/* euler angles (x, y, z) of a quaternion (x, y, z, w); back is unused */
void func_08820A8C(float *out, float *q, int back)
{
    float x2 = q[0] + q[0];
    float y2 = q[1] + q[1];
    float z2 = q[2] + q[2];
    float y = q[1];
    float s, a;
    float xy, zz, xx, wz, yz, yy, wx;

    s = -(q[0] * z2 - q[3] * y2);
    if (s >= 1.0f)
        s = 1.0f;
    else if (s <= -1.0f)
        s = -1.0f;
    if (s == 0.0f || s == -0.0f) {
        if (y == 1.0f || y == -1.0f)
            a = 3.1415927f;
        else
            a = 0.0f;
    } else {
        a = asinf_vfpu(s);
    }
    out[1] = a;
    zz = q[2] * z2;
    xx = q[0] * x2;
    xy = q[0] * y2;
    wz = q[3] * z2;
    if (a < 1.5707964f) {
        if (a > -1.5707964f) {
            yz = q[1] * z2;
            yy = q[1] * y2;
            wx = q[3] * x2;
            if (xy + wz == 0.0f)
                wz = 1.1754944e-38f;
            out[0] = atan2f_vfpu(yz + wx, 1.0f - (xx + yy));
            out[2] = atan2f_vfpu(xy + wz, 1.0f - (yy + zz));
        } else {
            out[0] = -atan2f_vfpu(xy - wz, 1.0f - (xx + zz));
            out[2] = 0.0f;
        }
    } else {
        out[0] = atan2f_vfpu(xy - wz, 1.0f - (xx + zz));
        out[2] = 0.0f;
    }
}

#ifdef NON_MATCHING /* quadro de pilha 16 bytes maior no original (80) que o gerado (64) */
/* position and rotation of a field slot, the rotation turned 180 degrees around z */
void func_08820CC0(float *pos, float *rot, int at)
{
    FxVec4 tmp_pos;
    FxVec4 tmp_rot;

    if (pos == 0)
        pos = &tmp_pos.x;
    if (rot == 0)
        rot = &tmp_rot.x;
    dd_slot_xform(pos, rot, at);
    {
        FxVec4 flip = { 0.0f, 0.0f, 1.0f, 0.0f };

        quat_mul(rot, rot, &flip.x);
    }
}
#endif

/* moves the glow and the target mark of a card object with it */
int func_08820D40(DdCobj *o)
{
    float m[16];
    float r[16];
    float e[4];

    if (o->x94 == -1 && o->x90 == -1)
        return 0;
    mat4_identity(m);
    quat_to_mat4(r, &o->rot.x);
    mat4_mul(m, r, m);
    mat4_translate(m, m, &o->pos.x);
    func_08820A8C(e, &o->rot.x, o->pos.z < 0.0f);
    if (o->x94 != -1)
        dd_fx_spin_move(o->x94, &o->pos.x, e, (float)dd_glow_kind);
    if (o->x90 != -1)
        dd_fx_spin_move(o->x90, &o->pos.x, e, (float)dd_glow_kind);
    return 0;
}

/* the shuffle coins are flying */
void func_08820E34(int on)
{
    DdTasksTail *t = &dd_tasks;

    if (on)
        t->coin_busy = 1;
    else
        t->coin_busy = 0;
}

/* "choose an action" message window, the card info panel hidden */
void func_08820E58(void)
{
    func_088226C0(dd_text(0x37), 1, 0);
    func_08825B88(0);
    func_088257E4(0, 100);
    func_08825818();
    dd_info_disable(1);
}

/* closes it; with a card, its detail mode when the answer was 1 or 2 */
void func_08820EA8(int card, int flag)
{
    dd_info_disable(0);
    if (card > 0) {
        switch (func_08825A14()) {
        case 1:
        case 2:
            dd_mode_set_card(card);
            dd_mode_push(5);
            dd_mode_set_flag(flag);
            break;
        }
    }
}

void func_08820F28(void)
{
    memset(&D_08C582B8, 0, 0x9C);
    D_08C582B4 = 0;
}

void func_08820F58(void)
{
    memset(&D_08C582B8, 0, 0x9C);
    D_08C582B4 = 0;
}
