// @range 0x08817500-0x088177C4
/* The sub-screen modules the story loads (charalist, cardalbum, shop, ...): twelve slots of module
   ids, the load (one module at a time, started with sceKernelStartModule once ehfile has loaded it),
   the unload and the per-frame hook the loaded module installs. */
#include "libstory.h"

typedef struct StMod {                 /* 0x08ACBAE4, 0x38 */
    int cur;                           /* +0x00 slot being loaded */
    int id[12];                        /* +0x04 module id per slot (0: not loaded) */
    void (*update)(void);              /* +0x34 called by st_draw_hook */
} StMod;

extern StMod st_mod;

int ehfile_request_module(int dir, const char *name, int (*cb)(void *, int), void *arg);

int st_mod_loaded(int id);
int st_mod_cb(void *arg, int id);

void st_init_817500(void)
{
    memset(&st_mod, 0, sizeof(StMod));
}

void st_term_817514(void)
{
    memset(&st_mod, 0, sizeof(StMod));
}

/* loads module `name` in slot `slot` (when no other module is loaded) and starts it */
void st_mod_load(int slot, char *name, void (*init)(void), void (*update)(void))
{
    int arg = 1;
    StMod *s = &st_mod;
    int i;

    if (slot < 0 || slot >= 12)
        return;
    if (s->id[slot] > 0)
        return;
    i = 0;
    do {
        if (st_mod_loaded(i))
            return;
        i++;
    } while (i < 12);
    s->cur = slot;
    ehfile_request_module(2, name, st_mod_cb, 0);
    while (s->id[s->cur] == 0)
        sys_frame_wait();
    if (sceKernelStartModule(s->id[s->cur], 4, &arg, 0, 0) < 0)
        return;
    if (init)
        init();
    if (update)
        s->update = update;
}

void st_draw_hook(void)
{
    StMod *s = &st_mod;

    if (s->update)
        s->update();
}

void st_mod_clear_update(void)
{
    st_mod.update = 0;
}

void st_mod_unload(int slot, void (*term)(void))
{
    int status;
    StMod *s = &st_mod;
    int *p;

    if (slot < 0 || slot >= 12)
        return;
    p = (int *)(slot * 4 + (char *)s->id); /* &s->id[slot], in the operand order of the original */
    if (*p <= 0)
        return;
    gfx_overlay_enable(1);
    if (term)
        term();
    s->update = 0;
    sys_frame_wait();
    sceKernelStopModule(*p, 0, 0, &status, 0);
    sceKernelUnloadModule(*p);
    *p = 0;
}

int st_mod_loaded(int slot)
{
    StMod *s = &st_mod;

    if (slot < 0 || slot >= 12)
        return 0;
    return s->id[slot] > 0;
}

/* ehfile callback: the module of the current slot is loaded */
int st_mod_cb(void *arg, int id)
{
    StMod *s = &st_mod;

    if (id <= 0)
        return 0;
    s->id[s->cur] = id;
    return 1;
}
