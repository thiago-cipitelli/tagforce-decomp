// @range 0x0881DF94-0x0881EBA0
// @asm 0x0881DFD0-0x0881E00C
// @asm 0x0881E0D0-0x0881E150
// @asm 0x0881E150-0x0881E214
// @asm 0x0881E214-0x0881E2A8
// @asm 0x0881E344-0x0881E3B8
// @asm 0x0881E3B8-0x0881E468
// @asm 0x0881E478-0x0881E5BC
// @asm 0x0881E610-0x0881E7EC
// @asm 0x0881E7EC-0x0881E9F0
// @asm 0x0881EAAC-0x0881EBA0
/* The list of running presentation tasks of the scene (draw/update/leave callbacks with a timer,
   removable from inside their own callbacks) and two of its users: the summon/set name banner of
   a card and the card picture shown big. */
#include "dd_field_ui.h"

typedef struct DdTask DdTask;
struct DdTask {                        /* 0x1C */
    DdTask *next;
    DdTask *prev;
    void (*draw)(DdTask *t, void *arg);         /* +0x08 */
    void (*update)(float dt, DdTask *t, void *arg); /* +0x0C */
    void (*leave)(DdTask *t, void *arg);        /* +0x10 */
    float time;                        /* +0x14 */
    void *arg;                         /* +0x18 */
};

typedef struct DdTasks {               /* 0x8A4 at 0x08C57A10 */
    DdTask head;
    int remove_cur;                    /* +0x1C the running task asked to be removed */
    char unk20[0x800];
    int slots[16][2];                  /* +0x820 */
    int unk8a0;
} DdTasks;

typedef struct DdBanner {              /* 0x18: argument of the name banner tasks */
    int id;                            /* card id */
    int handle;                        /* +0x04 */
    int state;                         /* +0x08 0 starting, 1 shown, 2 leaving */
    float t0;                          /* +0x0C */
    int done;                          /* +0x10 */
    int player;                        /* +0x14 */
} DdBanner;

extern MenuWidget dd_menu3;            /* 0x08C574E4 */
extern DdTasks dd_tasks;               /* 0x08C57A10 */

int dd_input_on(void);
void *dd_alloc(int n);
int dd_free(void *p);
int dd_mode_can_x(void);
void func_088252B0(void);
void func_088252B8(void);
void func_088252C0(void);
void dd_mark_init(void);
void dd_mark_term(void);
void func_08822B0C(void);
void func_08822B90(void);
void func_08822C2C(void);
int region_get(void);
void dd_lp_load(int *a);
void dd_lp_unload(void);
void dd_task_init(void);
void dd_task_term_all(void);
void dd_task_update(void);
void efs_update(void);
void mtx_clear_dirty(void);
int dd_fx_flipup_risen(int h);
int func_08806C84(int id);
int func_08825728(void);
void dd_task_pause(int h);
void func_08820EA8(int a, int b);
void func_08820E58(int a);
int dd_task_alive(int h);
void func_08820938(DdTask *t, void *arg);
void func_08820A8C(void *out, void *pos, int back);
int func_0880D0B4(int player, int zone);
int dd_fx_rays_start_t(int id, void *pos, void *cb, void *arg, float a, float b);
void func_08820980(void);
int dd_fx_flipup_start(int id, int *xy, int a, int b);
int dv_list_id(int uid);
int dv_slot_pos(int player, int zone);
void func_08820CC0(void *a, void *b, DdLoc l);
int dd_fx_summon_start_t(int a, int b, int c, void *d, void *cb, void *arg);
int dd_fx_summon_start_dark_t(int a, int b, int c, void *d, void *cb, void *arg);
void func_088091C8(int uid);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
void dd_task_remove(DdTask *t);
DdTask *dd_task_add(DdTask *t);

void dd_menu3_update(void)
{
    MenuWidget *w = &dd_menu3;

    if (dd_input_on() != 0)
        *(int *)((char *)w + 0x524) = menu_update(w);
}

#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
int dd_menu3_busy(void)
{
    int r;

    if (dd_menu3.state == 6) {
        r = 0;
    } else {
        r = 1;
        if (dd_menu3.state == 1 || dd_menu3.state == 0)
            r = 0;
    }
    return r;
}
#endif

void dd_tasks_init(void)
{
    DdTasks *s = &dd_tasks;
    int i;
    int *p;
    int key;

    memset(s, 0, sizeof(DdTasks));
    dd_tasks.head.next = &s->head;
    dd_tasks.head.prev = &s->head;
    for (i = 0; i < 16; i++)
        s->slots[i][0] = -1;
    func_088252B0();
    dd_mark_init();
    func_08822B0C();
    memset(&key, 0, sizeof(key));
    key = region_get();
    dd_lp_load(&key);
    dd_task_init();
}

#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_tasks_term(void)
{
    DdTask *t;

    func_08822B90();
    dd_mark_term();
    func_088252B8();
    t = dd_tasks.head.prev;
    while (t != 0 && t != &dd_tasks.head) {
        DdTask *cur = t;

        t = t->prev;
        dd_task_remove(cur);
    }
    dd_lp_unload();
    dd_task_term_all();
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_tasks_draw(void)
{
    DdTask *t;
    DdTask *next;

    func_088252C0();
    func_08822C2C();
    mtx_set_mode(2);
    efs_update();
    t = dd_tasks.head.prev;
    if (t != &dd_tasks.head) {
        for (;;) {
            if (t->draw != 0)
                t->draw(t, t->arg);
            if (dd_tasks.remove_cur != 0) {
                dd_tasks.remove_cur = 0;
                next = t->prev;
                dd_task_remove(t);
            } else {
                next = t->prev;
            }
            if (next == &dd_tasks.head)
                break;
            t = next;
        }
    }
    mtx_clear_dirty();
    mtx_set_mode(2);
    mtx_identity();
    dd_task_update();
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_tasks_update(float dt)
{
    DdTask *t;
    DdTask *next;

    t = dd_tasks.head.prev;
    if (t == &dd_tasks.head)
        return;
    for (;;) {
        t->time = t->time + dt;
        if (t->update != 0)
            t->update(dt, t, t->arg);
        if (dd_tasks.remove_cur != 0) {
            dd_tasks.remove_cur = 0;
            next = t->prev;
            dd_task_remove(t);
        } else {
            next = t->prev;
        }
        if (next == &dd_tasks.head)
            break;
        t = next;
    }
}
#endif

/* adds a copy of t at the head of the list */
DdTask *dd_task_add(DdTask *t)
{
    DdTask *n = dd_alloc(sizeof(DdTask));

    *n = *t;
    n->next = dd_tasks.head.next;
    n->prev = &dd_tasks.head;
    n->next->prev = n;
    n->prev->next = n;
    return n;
}

/* the running task removes itself after its callback */
int dd_task_remove_cur(void)
{
    dd_tasks.remove_cur = 1;
    return 0;
}

#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_task_remove(DdTask *t)
{
    if (t != 0 && t->prev != 0) {
        t->next->prev = t->prev;
        t->prev->next = t->next;
        t->prev = 0;
        t->next = 0;
        if (t->leave != 0)
            t->leave(t, t->arg);
        dd_free(t);
    }
}
#endif

/* the effect of a card on the field */
#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_card_fx_start(DdCard *c)
{
    float *pos = (float *)((char *)c + 0x10);
    int id;
    char out[8];

    if (c->loc2.b14)
        id = c->id;
    else
        id = -1;
    func_08820A8C(out, (char *)c + 0x40, pos[2] < 0.0f);
    c->fx = dd_fx_rays_start_t(id, pos, func_08820980, c, ((float *)out)[1],
                          (float)func_0880D0B4(c->loc.player, c->loc.zone));
}
#endif

void dd_banner_draw(DdTask *t, void *arg)
{
}

void dd_banner_leave(DdTask *t, void *arg)
{
    func_08820938(t, arg);
}

#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_banner_update(float dt, DdTask *t, void *arg)
{
    DdBanner *b = arg;

    switch (b->state) {
    case 2:
        if (t->time - b->t0 < 0.5f)
            break;
        if (func_08806C84(b->player) == 0) {
            b->done = 1;
            b->id = -1;
        }
        if (b->done != 0) {
            if (func_08825728() == 0) {
                dd_task_pause(b->handle);
                b->handle = -1;
                dd_task_remove_cur();
                func_08820EA8(b->id, 0);
                return;
            }
        } else if (func_08825728() == 0 && dd_mode_can_x() != 0) {
            b->done = 1;
            func_08820E58(b->id);
        }
        break;
    case 1:
        if (dd_fx_flipup_risen(b->handle) != 0) {
            b->t0 = t->time;
            b->state = 2;
        }
        break;
    case 0:
        req_flag_set(0x33);
        break;
    }
    req_flag_set(0);
}
#endif

void dd_banner_open(DdBanner *b)
{
    int xy[3];

    xy[0] = 0x13A;
    xy[2] = 0;
    xy[1] = 0x7A;
    b->handle = dd_fx_flipup_start(b->id, xy, 0, 0);
    b->state = 1;
}

/* name banner of a summoned card */
#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_banner_summon(DdCard *c)
{
    int uid = c->uid;
    int player = c->loc2.player;
    int zone = c->loc2.zone;
    DdBanner *b = dd_alloc(sizeof(DdBanner));
    DdTask t;
    DdLoc2 l;
    float pos[4];
    char a[8];
    char out[16];

    memset(b, 0, sizeof(DdBanner));
    b->id = dv_list_id(uid);
    b->player = player;
    memset(&t, 0, sizeof(t));
    t.draw = dd_banner_draw;
    t.update = (void (*)(float, DdTask *, void *))dd_banner_update;
    t.leave = dd_banner_leave;
    t.arg = b;
    dd_task_add(&t);
    l.player = player;
    l.zone = zone;
    l.idx = 0;
    l.b14 = 1;
    b->state = 0;
    l.b15 = dv_slot_pos(player, zone);
    func_08820CC0(a, pos, *(DdLoc *)&l);
    func_08820A8C(out, pos, ((float *)a)[2] < 0.0f);
    b->handle = dd_fx_summon_start_t(((int *)out)[1], func_0880D0B4(c->loc.player, c->loc.zone), b->id, a,
                              dd_banner_open, b);
    func_088091C8(c->uid);
    req_flag_set(0x33);
    req_flag_set(0);
}
#endif

/* name banner of a set card (face down: no name) */
#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_banner_set(DdCard *c)
{
    int up = c->loc2.b14;
    int uid = c->uid;
    int player = c->loc2.player;
    int zone = c->loc2.zone;
    DdBanner *b = dd_alloc(sizeof(DdBanner));
    DdTask t;
    DdLoc2 l;
    float pos[4];
    char a[8];
    char out[16];
    int id;

    memset(b, 0, sizeof(DdBanner));
    b->id = dv_list_id(uid);
    b->id = dv_list_id(uid);
    b->player = player;
    memset(&t, 0, sizeof(t));
    t.draw = dd_banner_draw;
    t.update = (void (*)(float, DdTask *, void *))dd_banner_update;
    t.leave = dd_banner_leave;
    t.arg = b;
    dd_task_add(&t);
    l.player = player;
    l.zone = zone;
    l.idx = 0;
    l.b14 = 1;
    b->state = 0;
    l.b15 = dv_slot_pos(player, zone);
    if (up)
        id = b->id;
    else
        id = -1;
    func_08820CC0(a, pos, *(DdLoc *)&l);
    func_08820A8C(out, pos, ((float *)a)[2] < 0.0f);
    b->handle = dd_fx_summon_start_dark_t(((int *)out)[1], func_0880D0B4(c->loc.player, c->loc.zone), id, a,
                              dd_banner_open, b);
    func_088091C8(c->uid);
    req_flag_set(0);
}
#endif

/* the card picture over the field (task draw) */
void dd_bigcard_draw(DdTask *t, int *arg)
{
    unsigned int *list = dl_select_layer(6, 0);

    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = card_draw(list, 0x104, 0x2D, arg[1]);
    dl_set_list(list);
}

void dd_bigcard_leave(DdTask *t, void *arg)
{
    func_08820938(t, arg);
}

#ifdef NON_MATCHING /* forma reconstruída; ordem dos blocos e alocação diferentes */
void dd_bigcard_update(float dt, DdTask *t, int *arg)
{
    if (t->time <= 1.25f)
        return;
    if (arg[2] != 0 && dd_task_alive(arg[2]) != 0) {
        req_flag_set(arg[3]);
        return;
    }
    if (arg[4] == 0 && func_08806C84(arg[0]) == 0) {
        arg[4] = 1;
        arg[1] = -1;
    }
    if (arg[4] != 0) {
        if (func_08825728() == 0) {
            dd_task_remove_cur();
            func_08820EA8(arg[1], arg[3]);
            return;
        }
    } else if (func_08825728() == 0 && dd_mode_can_x() != 0) {
        arg[4] = 1;
        func_08820E58(arg[1]);
    }
    req_flag_set(arg[3]);
}
#endif
