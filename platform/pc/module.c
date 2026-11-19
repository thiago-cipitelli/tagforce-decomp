/* ModuleMgrForUser on the host (port step 4b): every gmodule is already linked in, so loading one only records its
   name and gives it back the data of a fresh load (below); starting it calls its entry (module_start / module_entry,
   renamed <module>_<name> by tools/host_link.py and listed in host_mod_syms). Sony modules (USRDIR/module/*.prx:
   libfont, sceATRAC...) load and start as no-ops.

   On the PSP every load of a gmodule maps its .data and .bss anew, and the game relies on it: libfield keeps the
   PDA cover model in a global "loaded once" (field_pda_cover) that the next load must see as 0, or it points into
   a field heap that was rebuilt in between (the crash in fpda_cover_draw / fcal_free). So the initial .data of
   every gmodule is copied at start-up and put back, with .bss cleared, on each load (host_mod_data, modules.c). */
#include <string.h>

int dprintf(int fd, const char *fmt, ...);
void *__libc_malloc(unsigned int);   /* the game defines malloc */

typedef struct { const char *module, *name; void *addr; } HostModSym;
extern const HostModSym host_mod_syms[];
typedef struct { const char *module; char *data, *data_end, *bss, *bss_end; } HostModData;
extern const HostModData host_mod_data[];
static char *mod_init[64];   /* initial .data of host_mod_data[i] */

__attribute__((constructor)) static void mod_snapshot(void)
{
    int i;

    for (i = 0; host_mod_data[i].module && i < 64; i++) {
        const HostModData *d = &host_mod_data[i];
        if (d->data && d->data_end > d->data && (mod_init[i] = __libc_malloc(d->data_end - d->data)))
            memcpy(mod_init[i], d->data, d->data_end - d->data);
    }
}

static void mod_reset(const char *name)
{
    int i;

    for (i = 0; host_mod_data[i].module && i < 64; i++) {
        const HostModData *d = &host_mod_data[i];
        if (strcmp(d->module, name))
            continue;
        if (mod_init[i])
            memcpy(d->data, mod_init[i], d->data_end - d->data);
        if (d->bss && d->bss_end > d->bss)
            memset(d->bss, 0, d->bss_end - d->bss);
        return;
    }
}

#define NMOD 64
static char mods[NMOD][32];

int sceKernelLoadModule(const char *path, int flags, void *opt)
{
    const char *b = strrchr(path, '/');
    int i, n;

    b = b ? b + 1 : path;
    n = strlen(b);
    if (n > 4 && !strcmp(b + n - 4, ".prx"))
        n -= 4;
    for (i = 1; i < NMOD; i++) {
        if (!mods[i][0]) {
            memcpy(mods[i], b, n < 31 ? n : 31);
            mods[i][n < 31 ? n : 31] = 0;
            mod_reset(mods[i]);
            dprintf(2, "module: load %s = %d\n", path, i);
            return i;
        }
    }
    return 0x80020001;
}

int sceKernelStartModule(int uid, unsigned int args, void *argp, int *status, void *opt)
{
    const HostModSym *s;
    int r = 0;

    if (uid <= 0 || uid >= NMOD || !mods[uid][0])
        return 0x8002012E;
    for (s = host_mod_syms; s->module; s++) {
        if (!strcmp(s->module, mods[uid]) && (!strcmp(s->name, "module_start") || !strcmp(s->name, "module_entry"))) {
            dprintf(2, "module: start %s (%s)\n", mods[uid], s->name);
            r = ((int (*)(unsigned int, void *))s->addr)(args, argp);
            break;
        }
    }
    /* *status is not written: dmgr_load_draw passes locals of a frame that is gone when the ehfile thread starts the
       module (harmless dead stack on the PSP, a live frame of the scene thread here); nothing reads it */
    (void)status;
    (void)r;
    return uid;
}

int sceKernelStopModule(int uid, unsigned int args, void *argp, int *status, void *opt)
{
    if (status)
        *status = 0;
    return 0;
}

int sceKernelUnloadModule(int uid)
{
    if (uid > 0 && uid < NMOD) {
        dprintf(2, "module: unload %s\n", mods[uid]);
        mods[uid][0] = 0;
    }
    return 0;
}
