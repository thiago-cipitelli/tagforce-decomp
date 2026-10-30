// @range 0x0881EBA0-0x0881F0A8
// @asm 0x0881EBA0-0x0881ECA0
// @asm 0x0881ECA0-0x0881ED0C
// @asm 0x0881ED0C-0x0881EE28
// @asm 0x0881EE28-0x0881EF2C
// @asm 0x0881EF2C-0x0881EFB4
// @asm 0x0881EFB4-0x0881F03C
// @rodata 0x088701B4-0x088701B8
/* Card picture/text popups queued as tasks (wait until the commentary and the camera are free)
   and the glow effect attached to the field cards (up to 512 at a time). */
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

typedef struct DdPopup {               /* 0x18 */
    int player;
    int card;                          /* +0x04 */
    int handle;                        /* +0x08 */
    int flag;                          /* +0x0C request flag while shown */
    int done;                          /* +0x10 */
    int text;                          /* +0x14 0: card picture, else a text popup */
} DdPopup;

typedef struct DdTasks {
    DdTask head;
    int remove_cur;
    DdCard *glow[512];                 /* +0x20 cards with a glow effect */
} DdTasks;

extern DdTasks dd_tasks;               /* 0x08C57A10 */

const int dd_glow_kind = 1;

void *dd_alloc(int n);
int dd_free(void *p);
DdTask *dd_task_add(DdTask *t);
int dd_task_remove_cur(void);
void dd_task_remove(DdTask *t);
void dd_bigcard_draw(DdTask *t, int *arg);
void dd_bigcard_update(float dt, DdTask *t, int *arg);
void dd_bigcard_leave(DdTask *t, void *arg);
int dd_fx_cardaura_start(int card, int *xy, int a, int b);
int dd_fx_noise_start(int *xy, int a, int b);
int func_08819A70(void);
int func_0880CC64(void);
void func_08820938(DdTask *t, void *arg);
void func_08809184(int uid);
void func_08820A8C(void *out, void *pos, int back);
int dd_fx_spin_start(float k, int a, float *pos, void *b, int c, int d, int e);
void func_0880CDD8(DdCard *c, void (*cb)(void));
void func_0880CE10(DdCard *c, void (*cb)(void));
void func_08820D40(void);
void dd_task_pause(int h);
void dd_task_kill(int h);
void dd_popup(int player, int zone, int card, int flag, int text);

/* shows a card picture (or a text) now */
#ifdef NON_MATCHING /* forma reconstruída; alocação e ordem dos blocos diferentes */
void dd_popup_now(int player, int card, int flag, int text)
{
    DdPopup *p = dd_alloc(sizeof(DdPopup));
    int xy[3];
    DdTask t;

    memset(p, 0, sizeof(DdPopup));
    p->player = player;
    p->card = card;
    p->flag = flag;
    p->text = text;
    xy[0] = 0x13A;
    xy[2] = 0;
    xy[1] = 0x7A;
    if (text == 0)
        p->handle = dd_fx_cardaura_start(p->card, xy, 0, 0);
    else
        p->handle = dd_fx_noise_start(xy, 0, 0);
    memset(&t, 0, sizeof(t));
    t.draw = (void (*)(DdTask *, void *))dd_bigcard_draw;
    t.update = (void (*)(float, DdTask *, void *))dd_bigcard_update;
    t.leave = dd_bigcard_leave;
    t.arg = p;
    dd_task_add(&t);
}
#endif

/* waiting popup: starts when the commentary is off and the camera idle */
#ifdef NON_MATCHING /* forma reconstruída; alocação e ordem dos blocos diferentes */
void dd_popup_wait(float dt, DdTask *t, DdPopup *p)
{
    if (func_08819A70() == 0 && func_0880CC64() != 0) {
        dd_popup(p->player, 0x10, p->card, 0, p->text);
        dd_task_remove_cur();
        return;
    }
    req_flag_set(p->flag);
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação e ordem dos blocos diferentes */
void dd_popup(int player, int zone, int card, int flag, int text)
{
    DdPopup *p;
    DdTask t;

    if (func_08819A70() == 0 && func_0880CC64() != 0) {
        dd_popup_now(player, card, flag, text);
        req_flag_set(flag);
        if (zone == 0x10)
            func_08809184(card);
        else
            dd_cursor_set(player, zone, 0, 1);
        return;
    }
    p = dd_alloc(sizeof(DdPopup));
    memset(p, 0, sizeof(DdPopup));
    p->player = player;
    p->card = card;
    p->flag = flag;
    p->text = text;
    memset(&t, 0, sizeof(t));
    t.update = (void (*)(float, DdTask *, void *))dd_popup_wait;
    t.leave = func_08820938;
    t.arg = p;
    dd_task_add(&t);
}
#endif

/* starts the glow of a card */
#ifdef NON_MATCHING /* forma reconstruída; alocação e ordem dos blocos diferentes */
void dd_glow_on(DdCard *c)
{
    float pos[4];
    char dir[16];
    int i;

    if (c->glow != -1)
        return;
    func_08820A8C(dir, (char *)c + 0x40, 0);
    *(DdVec4 *)pos = *(DdVec4 *)((char *)c + 0x10);
    if (c->loc.zone != 0xB)
        pos[1] += 0.2f;
    c->glow = dd_fx_spin_start((float)dd_glow_kind, 1, pos, dir, -1, 0, 0);
    for (i = 0; i < 512; i++) {
        if (dd_tasks.glow[i] == 0) {
            dd_tasks.glow[i] = c;
            break;
        }
    }
    func_0880CDD8(c, func_08820D40);
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação e ordem dos blocos diferentes */
void dd_glow_off(DdCard *c)
{
    int i;

    if (c->glow == -1)
        return;
    dd_task_pause(c->glow);
    c->glow = -1;
    for (i = 0; i < 512; i++) {
        if (c == dd_tasks.glow[i]) {
            dd_tasks.glow[i] = 0;
            break;
        }
    }
    func_0880CE10(c, func_08820D40);
}
#endif

#ifdef NON_MATCHING /* forma reconstruída; alocação e ordem dos blocos diferentes */
void dd_glow_all_off(void)
{
    DdTasks *s = &dd_tasks;
    int i;
    DdCard *c;

    for (i = 0; i < 512; i++) {
        c = s->glow[i];
        if (c != 0 && c->glow != -1) {
            dd_task_pause(c->glow);
            c->glow = -1;
            s->glow[i] = 0;
            func_0880CE10(c, func_08820D40);
        }
    }
}
#endif

void dd_effect_leave(DdTask *t, int *arg)
{
    if (*arg != 0) {
        dd_task_kill(*arg);
        *arg = 0;
    }
    dd_free(arg);
}

void dd_effect_flags(void)
{
    req_flag_set(0x2A);
    req_flag_set(0x2D);
}

void dd_effect_detach(DdTask *t)
{
    *(int *)t->arg = 0;
    dd_task_remove(t);
}
