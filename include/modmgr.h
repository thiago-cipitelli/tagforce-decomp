/* modmgr: registry of loadable modules (Sony libs via sceUtility or PRX via ehfile) with refcounts.
   State lives in .bss (symbols/BOOT.txt: modmgr 0x08BE9F78). */
#ifndef MODMGR_H
#define MODMGR_H

typedef struct ModTableEntry {   /* static description, 0x10 */
    int id;
    int arg;                     /* ehfile dir index; -1/-2 are replaced by modmgr_init's arguments */
    const char *name;
    unsigned char type;          /* 0 PRX (ehfile), 1 net, 2 av, 3 usb (sceUtility) */
    unsigned char handle;        /* sceUtility module id */
    unsigned char pad[2];
} ModTableEntry;

typedef struct ModEntry {        /* registry entry, 0x4C */
    int id;
    int arg;
    int module_id;               /* +0x08 SceUID */
    int state;                   /* +0x0C 0 free, 1 loading, 2 loaded */
    int refcount;                /* +0x10 */
    char name[0x20];             /* +0x14 */
    int type;                    /* +0x34 */
    int handle;                  /* +0x38 */
    int argsize;                 /* +0x3C sceKernelStartModule args */
    void *argp;
    int *status;
    void *opt;
} ModEntry;

typedef struct ModMgr {
    ModEntry entries[16];
    int count;                   /* +0x4C0 */
} ModMgr;

extern ModMgr modmgr;
extern int modmgr_status_by_id;   /* status slot used by modmgr_load_by_id */
extern int modmgr_status_default; /* status slot when the caller passes none */
extern ModTableEntry static_module_table[];

void modmgr_init(int arg_m1, int arg_m2);
int modmgr_register_table(const ModTableEntry *t);
ModEntry *modmgr_find(int id);
int modmgr_load_by_id(int id);
int modmgr_load_ex(int id, int argsize, void *argp, int *status, void *opt);
int modmgr_update(int id);
int modmgr_none_loading(void);
int modmgr_load_sync(int dir, const char *name);
int modmgr_load_and_start(int dir, const char *name);

#endif
