// @range 0x088260A4-0x088264DC
/* Task table of the duel effects: 254 slots, each with a state (1/2 running, 3 killed, 4 freed
   next frame), an update callback, a terminate callback and a zeroed work buffer from the duel
   heap. Handles are slot << 24 | serial. */
#include "dd_d3.h"

extern DdTaskMgr dd_task_mgr;

void *func_0881CA54(int size);         /* duel heap alloc */
void func_0881CA64(void *p);           /* duel heap free */
void dd_card_quad_init(void);
void func_08838CDC(void);
void dd_rstate_reset_all(void);

void dd_task_free(DdTask *t);

void dd_task_init(void)
{
    memset(&dd_task_mgr, 0, sizeof(dd_task_mgr));
    dd_card_quad_init();
    func_08838CDC();
}

void dd_task_term_all(void)
{
    DdTaskMgr *m = &dd_task_mgr;
    int i;

    for (i = 0; i < DD_TASK_MAX; i++) {
        DdTask *t = &m->task[i];

        if (t->state != 0)
            dd_task_free(t);
    }
}

void dd_task_update(void)
{
    DdTaskMgr *m = &dd_task_mgr;
    int i;

    dd_rstate_reset_all();
    for (i = 0; i < DD_TASK_MAX; i++) {
        DdTask *t = &m->task[i];

        switch (t->state) {
        case 0:
            break;
        case 1:
        case 2:
            t->state = t->update(t->state, t->mem);
            break;
        case 3:
            t->state = 4;
            break;
        case 4:
            dd_task_free(t);
            break;
        }
    }
}

static DdTask *dd_task_find(int id)
{
    DdTaskMgr *m = &dd_task_mgr;
    unsigned int i;
    DdTask *t;

    if (id == -1)
        return 0;
    i = (id >> 24) & 0xFF;
    if (i >= DD_TASK_MAX)
        return 0;
    t = &m->task[i];
    if (t->handle != id)
        return 0;
    return t;
}

int dd_task_create(DdTaskUpdateFn update, DdTaskTermFn term, void *arg, int size)
{
    DdTaskMgr *m = &dd_task_mgr;
    DdTask *t = 0;
    int i;
    int slot;
    void *mem;

    for (i = 0; i < DD_TASK_MAX; i++) {
        if (m->task[i].state == 0) {
            t = &m->task[i];
            slot = i;
            break;
        }
    }
    if (t == 0)
        return -1;
    t->mem = mem = func_0881CA54(size);
    if (mem == 0)
        return -1;
    memset(mem, 0, size);
    t->handle = ((slot & 0xFF) << 24) | (m->serial & 0xFFFFFF);
    t->state = 1;
    t->update = update;
    t->term = term;
    t->arg = arg;
    if (m->serial < 0xFFFFFF)
        m->serial++;
    else
        m->serial = 0;
    return t->handle;
}

int dd_task_kill(int id)
{
    DdTask *t = dd_task_find(id);

    if (t == 0)
        return 0;
    if (t->state == 0)
        return 0;
    t->state = 3;
    return 1;
}

void dd_task_free(DdTask *t)
{
    if (t->term != 0)
        t->term(t->arg);
    if (t->mem != 0) {
        func_0881CA64(t->mem);
        t->mem = 0;
    }
    t->state = 0;
    t->flags = 0;
    t->handle = -1;
    t->update = 0;
    t->term = 0;
}

int dd_task_alive(int id)
{
    DdTask *t = dd_task_find(id);

    if (t == 0)
        return 0;
    return t->state != 0;
}

void *dd_task_mem(int id)
{
    DdTask *t = dd_task_find(id);

    if (t == 0)
        return 0;
    return t->mem;
}

int dd_task_flag_set(int id, unsigned int flags)
{
    DdTask *t = dd_task_find(id);

    if (t == 0)
        return 0;
    t->flags |= flags;
    return 1;
}

int dd_task_flag_test(int id, unsigned int flags)
{
    DdTask *t = dd_task_find(id);

    if (t == 0)
        return 0;
    return (t->flags & flags) != 0;
}

int dd_task_pause(int id)
{
    return dd_task_flag_set(id, 1);
}
