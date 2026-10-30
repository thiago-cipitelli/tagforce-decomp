// @range 0x0881F0A8-0x08820138
// @asm 0x0881F0A8-0x0881F264
// @asm 0x0881F264-0x0881F454
// @asm 0x0881F46C-0x0881F60C
// @asm 0x0881F60C-0x0881F704
// @asm 0x0881F704-0x0881F78C
// @asm 0x0881F78C-0x0881F7EC
// @asm 0x0881FAF8-0x0881FB28
// @asm 0x0881FB28-0x0881FBD4
// @asm 0x0881FC48-0x0881FD30
// @asm 0x0881FD30-0x0881FDB4
// @asm 0x0881FDB4-0x0881FEC0
// @asm 0x0881FEC0-0x0881FFB8
// @data 0x088760B0-0x088760D0
/* Effects played on field cards: summon/set beams, the target marks of graves and decks (shared by
   all their cards), damage/recovery and ATK/DEF change counters, destruction and the short lock of
   the cursor after a move. */
#include "dd_field_ui.h"

typedef struct DdTask DdTask;
struct DdTask {
    DdTask *next;
    DdTask *prev;
    void (*draw)(DdTask *t, void *arg);
    void (*update)(float dt, DdTask *t, void *arg);
    void (*leave)(DdTask *t, void *arg);
    float time;
    void *arg;
};

typedef struct DdTasks {
    DdTask head;
    int remove_cur;
    DdCard *glow[512];                 /* +0x020 */
    int pile_mark[16][2];              /* +0x820 (handle, users) per zone */
} DdTasks;

typedef struct DdCounter {             /* 0x10 */
    int value;
    int uid;
    DdTask *popup;                     /* +0x08 */
    int flag;                          /* +0x0C */
} DdCounter;

typedef int (*DdBeamFn)(float s, float zone_kind, int id, float *pos, void *dir, void (*done)(DdTask *), DdTask *t);
typedef int (*DdLpFn)(int player, int zone);
typedef void (*DdNumFn)(float *pos, int from, int to, void (*done)(DdTask *), DdTask *t);

extern DdTasks dd_tasks;               /* 0x08C57A10 */
extern const int dd_glow_kind;         /* 0x088701B4 */

DdVec4 dd_beam_up __attribute__((aligned(16))) = { 0.0f, 0.0f, 1.0f, 0.0f };
DdVec4 dd_beam_up2 __attribute__((aligned(16))) = { 0.0f, 0.0f, 1.0f, 0.0f };

void *dd_alloc(int n);
int dd_free(void *p);
DdTask *dd_task_add(DdTask *t);
int dd_task_remove_cur(void);
void dd_task_remove(DdTask *t);
void dd_effect_flags(void);
void dd_effect_leave(DdTask *t, int *arg);
void dd_effect_detach(DdTask *t);
void dd_bigcard_leave(DdTask *t, void *arg);
DdTask *dd_popup_now(int player, int card, int flag, int text);
void dd_cursor_lock(int on);
int func_08866BC0(int player, int zone, int idx);
float *quat_mul(float *out, float *a, float *b);
void func_08820A8C(void *out, void *q, int back);
float func_0880D0B4(int player, int zone);
int dd_fx_spiral_start_ex(float s, float zone_kind, int id, float *pos, void *dir, void (*done)(DdTask *), DdTask *t);
int dd_fx_spin_start(float k, int a, float *pos, void *b, int c, int d, int e);
void func_0880CDD8(DdCard *c, void (*cb)(void));
void func_0880CE10(DdCard *c, void (*cb)(void));
void func_08820D40(void);
void dd_task_pause(int h);
int func_0880CB44(void);
int func_0880CC64(void);
int func_0880CD38(void);
void func_0880C9BC(int a);
void func_0880C48C(int a, int b);
void func_08820938(DdTask *t, void *arg);
void func_08820988(float *out, float *m);
void dd_fx_shatter_start_t(int id, float *pos, float *dir, int a, int b, float s, float zone_kind);
int dd_fx_twist_start_t(float s, float zone_kind, int id, float *pos, void *dir, void (*done)(DdTask *), DdTask *t);
void dd_fx_orb_start(void *a, void *b, void (*cb)(DdCard *c));
int func_08866B00(int player, int zone);
int func_08866AE8(int player, int zone);
int func_08866AD0(int player, int zone);
int func_08866B30(int player, int zone);
int func_08866B48(int player, int zone);
int func_08866B18(int player, int zone);
void dd_fx_tally_start(int up, float *pos, int from, int to, void (*done)(DdTask *), DdTask *t);
void dd_fx_counter_start2(float *pos, int from, int to, void (*done)(DdTask *), DdTask *t);
void dd_fx_blink_start(float *pos, int from, int to, void (*done)(DdTask *), DdTask *t);
void dd_fx_throb_start(float *pos, int from, int to, void (*done)(DdTask *), DdTask *t);
void dd_fx_tally_start_fixed(float *pos, int from, int to, void (*done)(DdTask *), DdTask *t);
void dd_fx_counter_start(float *pos, int from, int to, void (*done)(DdCounter *), DdCounter *c);
int func_088667BC(int uid);
void dd_ring_start(int value, void (*done)(DdTask *), DdTask *t);
int func_08866998(int uid);
int func_088667C4(int a, int b);
void func_08811D60(int a, int b);

void dd_beam(DdCard *c, DdBeamFn fn);
void dd_beam_set(DdCard *c, int player, int zone, DdBeamFn fn);
void dd_beam_summon(DdCard *c);
void dd_beam_set2(DdCard *c, int player, int zone);
void dd_mark_on(DdCard *c);
void dd_mark_off(DdCard *c);
void dd_mark_all_off(void);
void dd_destroy_wait(float dt, DdTask *t, void *arg);
void dd_destroy(int a);
void dd_move_wait(float dt, DdTask *t, void *arg);
void dd_move(int a, int b);
void dd_flip(DdCard *c);
void dd_flip_now(DdCard *c);
void dd_shake_done(DdCard *c);
void dd_shake(DdCard *c);
void dd_stat_counter(DdCard *c, int delta, int def);
void dd_counter_flag(float dt, DdTask *t, DdCounter *c);
void dd_counter_done(DdCounter *c);
void dd_damage_counter(float *pos, int value, int uid);
void dd_counter_wait(float dt, DdTask *t, void *arg);
void dd_counter(DdCard *card, int value, int uid);
void dd_num_counter(DdCard *c, int delta, int kind);
void dd_flag_task_update(float dt, DdTask *t, int *flag);
void dd_flag_task_done(DdTask *t);
DdTask *dd_flag_task(int flag);
void dd_lock_update(float dt, DdTask *t, void *arg);
void dd_lock_cursor(void);
void dd_task_free_pair(int *p);

/* summon beam of a card (fn: the effect starter) */
#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_beam(DdCard *c, DdBeamFn fn)
{
    int *h = dd_alloc(4);
    DdTask t;
    DdTask *task;
    float rot[4];
    float up[4];
    char dir[16];
    int known;

    memset(h, 0, 4);
    memset(&t, 0, sizeof(t));
    t.update = (void (*)(float, DdTask *, void *))dd_effect_flags;
    t.leave = (void (*)(DdTask *, void *))dd_effect_leave;
    t.draw = 0;
    t.arg = h;
    task = dd_task_add(&t);
    known = func_08866BC0(c->loc.player, c->loc.zone, c->loc.zone < 0xB ? 0 : c->loc.idx);
    if (known == 0) {
        *(DdVec4 *)rot = *(DdVec4 *)((char *)c + 0x40);
    } else {
        *(DdVec4 *)up = dd_beam_up;
        quat_mul(rot, (float *)((char *)c + 0x40), up);
    }
    func_08820A8C(dir, rot, ((float *)c)[6] < 0.0f);
    *h = fn(1.0f, func_0880D0B4(c->loc.player, c->loc.zone), known == 0 ? -1 : c->id,
            (float *)((char *)c + 0x10), dir, dd_effect_detach, task);
    req_flag_set(0x2A);
}
#endif

/* set beam: the face shown is the one of the destination */
#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_beam_set(DdCard *c, int player, int zone, DdBeamFn fn)
{
    int *h = dd_alloc(4);
    DdTask t;
    DdTask *task;
    float rot[4];
    float up[4];
    char dir[16];

    memset(h, 0, 4);
    memset(&t, 0, sizeof(t));
    t.update = (void (*)(float, DdTask *, void *))dd_effect_flags;
    t.leave = (void (*)(DdTask *, void *))dd_effect_leave;
    t.draw = 0;
    t.arg = h;
    task = dd_task_add(&t);
    if (func_08866BC0(c->loc.player, c->loc.zone, c->loc.zone < 0xB ? 0 : c->loc.idx) == 0) {
        *(DdVec4 *)rot = *(DdVec4 *)((char *)c + 0x40);
    } else {
        *(DdVec4 *)up = dd_beam_up2;
        quat_mul(rot, (float *)((char *)c + 0x40), up);
    }
    func_08820A8C(dir, rot, ((float *)c)[6] < 0.0f);
    *h = fn(1.0f, func_0880D0B4(c->loc.player, c->loc.zone),
            func_08866BC0(player, zone, zone < 0xB ? 0 : c->loc.idx) == 0 ? -1 : c->id,
            (float *)((char *)c + 0x10), dir, dd_effect_detach, task);
    req_flag_set(0x2D);
}
#endif

void dd_beam_summon(DdCard *c)
{
    dd_beam(c, dd_fx_spiral_start_ex);
}

void dd_beam_set2(DdCard *c, int player, int zone)
{
    dd_beam_set(c, player, zone, dd_fx_twist_start_t);
}

/* target mark of a card; graves and decks share one mark per zone */
#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_mark_on(DdCard *c)
{
    int *pile = 0;
    int zone;
    int pile_zone;
    float pos[4];
    char dir[16];
    int h;
    int i;

    if (c->mark != -1)
        return;
    zone = c->loc.zone;
    pile_zone = (unsigned int)(zone - 0xE) < 2;
    if (pile_zone) {
        pile = dd_tasks.pile_mark[zone];
        if (pile[0] != -1) {
            pile[1]++;
            return;
        }
    }
    func_08820A8C(dir, (char *)c + 0x40, ((float *)c)[6] < 0.0f);
    *(DdVec4 *)pos = *(DdVec4 *)((char *)c + 0x10);
    if (c->loc.zone != 0xB)
        pos[1] += 0.2f;
    h = dd_fx_spin_start((float)dd_glow_kind, 0, pos, dir, -1, 0, 0);
    if (pile_zone) {
        pile[0] = h;
        pile[1] = 1;
        return;
    }
    c->mark = h;
    for (i = 0; i < 512; i++) {
        if (dd_tasks.glow[i] == 0) {
            dd_tasks.glow[i] = c;
            break;
        }
    }
    func_0880CDD8(c, func_08820D40);
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_mark_off(DdCard *c)
{
    int zone = c->loc.zone;
    int n;
    int i;

    if ((unsigned int)(zone - 0xE) < 2) {
        n = dd_tasks.pile_mark[zone][1] - 1;
        dd_tasks.pile_mark[zone][1] = n;
        if (n == 0) {
            dd_task_pause(dd_tasks.pile_mark[zone][0]);
            dd_tasks.pile_mark[zone][0] = -1;
        }
    } else if (c->mark != -1) {
        dd_task_pause(c->mark);
        c->mark = -1;
        for (i = 0; i < 512; i++) {
            if (c == dd_tasks.glow[i]) {
                dd_tasks.glow[i] = 0;
                break;
            }
        }
        func_0880CE10(c, func_08820D40);
    }
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_mark_all_off(void)
{
    DdTasks *s = &dd_tasks;
    int i;
    DdCard *c;

    for (i = 0; i < 512; i++) {
        c = s->glow[i];
        if (c != 0 && c->mark != -1) {
            dd_task_pause(c->mark);
            c->mark = -1;
            s->glow[i] = 0;
            func_0880CE10(c, func_08820D40);
        }
    }
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_destroy_wait(float dt, DdTask *t, void *arg)
{
    int a = (int)t->arg;

    if (func_0880CB44() == 0 && func_0880CC64() != 0) {
        func_0880C9BC(a);
        dd_task_remove_cur();
    } else {
        req_flag_set(0x10);
    }
}
#endif

/* destruction of the cards of a zone, once the camera is free */
void dd_destroy(int a)
{
    DdTask t;

    if (func_0880CB44() == 0 && func_0880CC64() != 0) {
        func_0880C9BC(a);
    } else {
        memset(&t, 0, sizeof(t));
        t.update = dd_destroy_wait;
        t.arg = (void *)a;
        dd_task_add(&t);
        req_flag_set(0x10);
    }
}

void dd_move_wait(float dt, DdTask *t, void *arg)
{
    req_flag_set(0x27);
    if (func_0880CB44() == 0 && func_0880CC64() != 0 && func_0880CD38() != 0) {
        func_0880C48C(((int *)t->arg)[0], ((int *)t->arg)[1]);
        dd_task_remove_cur();
    }
}

void dd_move(int a, int b)
{
    int *p;
    DdTask t;

    req_flag_set(0x27);
    if (func_0880CB44() == 0 && func_0880CC64() != 0 && func_0880CD38() != 0) {
        func_0880C48C(a, b);
        return;
    }
    p = dd_alloc(8);
    memset(p, 0, 8);
    p[0] = a;
    p[1] = b;
    memset(&t, 0, sizeof(t));
    t.update = dd_move_wait;
    t.leave = func_08820938;
    t.arg = p;
    dd_task_add(&t);
}

/* flip of a card: postponed while it moves */
void dd_flip(DdCard *c)
{
    float dir[4];
    float m[16];

    if (c->moving || c->moving2) {
        c->flip_pending = 1;
        req_flag_set(0x1D);
        return;
    }
    quat_to_mat4(m, (float *)((char *)c + 0x40));
    func_08820988(dir, m);
    dd_fx_shatter_start_t(c->id, (float *)((char *)c + 0x10), dir, 0, 0, 1.0f, func_0880D0B4(c->loc.player, c->loc.zone));
}

void dd_flip_now(DdCard *c)
{
    float dir[4];
    float m[16];

    quat_to_mat4(m, (float *)((char *)c + 0x40));
    func_08820988(dir, m);
    dd_fx_shatter_start_t(c->id, (float *)((char *)c + 0x10), dir, 0, 0, 1.0f, func_0880D0B4(c->loc.player, c->loc.zone));
}

void dd_shake_done(DdCard *c)
{
    c->b4 = 0;
}

#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_shake(DdCard *c)
{
    c->b4 = 1;
    dd_fx_orb_start((char *)c + 0x20, (char *)c + 0x30, dd_shake_done);
}
#endif

/* ATK/DEF change counter over a card (atk 0: DEF) */
#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_stat_counter(DdCard *c, int delta, int def)
{
    int to;

    if (def == 0)
        to = func_08866B00(c->loc.player, c->loc.zone);
    else
        to = func_08866AE8(c->loc.player, c->loc.zone);
    delta = to - delta;
    if (delta < 1)
        delta = 0;
    if (to < 1)
        to = 0;
    dd_fx_tally_start(def == 0, (float *)((char *)c + 0x10), delta, to, dd_flag_task_done, dd_flag_task(0x25));
}
#endif

void dd_counter_flag(float dt, DdTask *t, DdCounter *c)
{
    req_flag_set(c->flag);
}

void dd_counter_done(DdCounter *c)
{
    if (func_088667BC(c->uid) == 0x169C && c->value < 0x14)
        dd_ring_start(c->value, dd_flag_task_done, dd_flag_task(0x23));
    dd_task_remove(c->popup);
}

/* damage counter */
#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_damage_counter(float *pos, int value, int uid)
{
    int from = value - 1;
    int to = value;
    DdCounter *c;
    DdTask t;

    if (from < 1)
        from = 0;
    if (to < 1)
        to = 0;
    c = dd_alloc(sizeof(DdCounter));
    memset(c, 0, sizeof(DdCounter));
    c->value = value;
    c->uid = uid;
    c->flag = 0x23;
    memset(&t, 0, sizeof(t));
    t.update = (void (*)(float, DdTask *, void *))dd_counter_flag;
    t.leave = func_08820938;
    t.draw = 0;
    t.arg = c;
    c->popup = dd_task_add(&t);
    req_flag_set(c->flag);
    dd_fx_counter_start(pos, from, to, dd_counter_done, c);
}
#endif

/* waits for the card popup to close before the counter */
#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_counter_wait(float dt, DdTask *t, void *arg)
{
    DdCounter *c = t->arg;
    DdTask *p;
    float pos[4] = { 0 };

    for (p = &dd_tasks.head; p != c->popup; ) {
        p = p->prev;
        if (p == &dd_tasks.head)
            break;
    }
    if (p == &dd_tasks.head) {
        dd_damage_counter(pos, c->value, c->uid);
        dd_task_remove_cur();
    } else {
        req_flag_set(c->flag);
    }
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_counter(DdCard *card, int value, int uid)
{
    DdCounter *c;
    DdTask t;
    DdTask *popup;
    float pos[4];

    if (card != 0) {
        *(DdVec4 *)pos = *(DdVec4 *)((char *)card + 0x10);
        dd_damage_counter(pos, value, uid);
        return;
    }
    popup = dd_popup_now(func_08866998(uid), func_088667BC(uid), 0x23, 0);
    c = dd_alloc(sizeof(DdCounter));
    memset(c, 0, sizeof(DdCounter));
    c->value = value;
    c->uid = uid;
    c->popup = popup;
    c->flag = 0x23;
    memset(&t, 0, sizeof(t));
    t.leave = func_08820938;
    t.update = dd_counter_wait;
    t.arg = c;
    dd_task_add(&t);
    req_flag_set(0x23);
}
#endif

/* stat counters of the other kinds (0 ATK, 1 level, 2 ..., 3 counters) */
#ifdef NON_MATCHING /* forma reconstruída; alocação, quadro e ordem dos blocos diferentes */
void dd_num_counter(DdCard *c, int delta, int kind)
{
    int flag = 0x25;
    DdLpFn get;
    DdNumFn show;
    int to;

    if (kind == 3) {
        get = func_08866AD0;
        show = dd_fx_counter_start2;
        flag = 0x26;
    } else if (kind == 2) {
        get = func_08866B30;
        show = dd_fx_blink_start;
    } else if (kind == 1) {
        get = func_08866B48;
        show = dd_fx_throb_start;
    } else if (kind == 0) {
        get = func_08866B18;
        show = dd_fx_tally_start_fixed;
    } else {
        return;
    }
    to = get(c->loc.player, c->loc.zone);
    delta = to - delta;
    if (delta < 1)
        delta = 0;
    if (to < 1)
        to = 0;
    show((float *)((char *)c + 0x10), delta, to, dd_flag_task_done, dd_flag_task(flag));
}
#endif

void dd_flag_task_update(float dt, DdTask *t, int *flag)
{
    req_flag_set(*flag);
}

void dd_flag_task_done(DdTask *t)
{
    dd_task_remove(t);
}

/* a task that only keeps a request flag up */
DdTask *dd_flag_task(int flag)
{
    int *f = dd_alloc(4);
    DdTask t;
    DdTask *r;

    memset(f, 0, 4);
    *f = flag;
    memset(&t, 0, sizeof(t));
    t.update = (void (*)(float, DdTask *, void *))dd_flag_task_update;
    t.leave = dd_bigcard_leave;
    t.draw = 0;
    t.arg = f;
    r = dd_task_add(&t);
    req_flag_set(*f);
    return r;
}

void dd_lock_update(float dt, DdTask *t, void *arg)
{
    if (t->time >= 0.5f) {
        dd_cursor_lock(0);
        dd_task_remove_cur();
    } else {
        req_flag_set(0x17);
    }
}

/* locks the cursor for half a second */
void dd_lock_cursor(void)
{
    DdTask t;

    memset(&t, 0, sizeof(t));
    t.update = dd_lock_update;
    t.draw = 0;
    t.leave = 0;
    t.arg = 0;
    dd_cursor_lock(1);
    dd_task_add(&t);
    req_flag_set(0x17);
}

void dd_task_free_pair(int *p)
{
    dd_task_remove((DdTask *)p[0]);
    func_08811D60(p[1], p[2]);
    dd_free(p);
}
