/* ModuleMgrForUser on the host (port step 4b): every gmodule is already linked in, so loading one only records its
   name; starting it calls its entry (module_start / module_entry, renamed <module>_<name> by tools/host_link.py and
   listed in host_mod_syms). Sony modules (USRDIR/module/*.prx: libfont, sceATRAC...) load and start as no-ops. */
#include <string.h>

int dprintf(int fd, const char *fmt, ...);

typedef struct { const char *module, *name; void *addr; } HostModSym;
extern const HostModSym host_mod_syms[];

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
