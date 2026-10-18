// @range 0x08859044-0x08859824
// @data 0x088EFFF8-0x088F00E4
/* Frame wait, a debug text menu, synchronous file/module loads and the module manager. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "ehfile.h"
#include "modmgr.h"
#include "allegrex.h"

int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, EhfileCallback cb, void *arg);
int ehfile_request_module(int dir, const char *name, EhfileCallback cb, void *arg);
static int menu_count_lines(const char *text, int sel);
static int ehfile_read_done_cb(void *arg, int result);
int modmgr_load_done_cb(void *arg, int result);
int modmgr_start(void *arg, int module_id);
int modmgr_cmp(const void *a, const void *b);
static void modmgr_addref(ModEntry *m);
static void modmgr_release(ModEntry *m);

ModTableEntry static_module_table[] = {
    { 1, -2, "libfont.prx", 0, 0 },
    { 2, -1, "videocodec.prx", 0xFF, 0 },
    { 3, -1, "audiocodec.prx", 2, 0 },
    { 4, -1, "mpegbase.prx", 0xFF, 0 },
    { 5, -2, "mpeg.prx", 2, 3 },
    { 6, -2, "libatrac3plus.prx", 2, 2 },
    { 7, -1, "sc_sascore.prx", 2, 1 },
    { 0, 0, 0, 0, 0 },
};

void sys_frame_wait(void)
{
    ehscript_wake_main();
}

/* Debug text menu: `text` has one entry per line; up/down move, circle selects, cross cancels.
   Returns the index, -1 on cancel, -2 when `cancel_cb` says so. */
int menu_wait_select(const char *text, int idx, int (*cancel_cb)(void))
{
    PadState *pad;
    int n;
    int no_cb;

    idx = ALLEGREX_MAX(idx, 0);
    no_cb = !cancel_cb; /* hoisted by the programmer, not the compiler */
    for (;;) {
        n = menu_count_lines(text, idx);
        idx = ALLEGREX_MIN(idx, n - 1);
        pad = pad_get();
        if (!no_cb) {
            if (cancel_cb()) {
                sys_frame_wait();
                return -2;
            }
        }
        if (pad->b.dir_rep & 0x10) {
            idx = (idx + n - 1) % n;
        } else if (pad->b.dir_rep & 0x40) {
            idx = (idx + 1) % n;
        } else if (pad->b.trig & 0x2000) {
            sys_frame_wait();
            return idx;
        } else if (pad->b.trig & 0x4000) {
            sys_frame_wait();
            return -1;
        }
        sys_frame_wait();
    }
}

int ehfile_read_sync(int dir, const char *name, void *buf, int size)
{
    int done = 0;

    ehfile_read_async_alias(dir, name, buf, size, ehfile_read_done_cb, &done);
    while (done == 0)
        sys_frame_wait();
    return done;
}

int modmgr_load_sync(int dir, const char *name)
{
    int id = 0;

    ehfile_request_module(dir, name, modmgr_load_done_cb, &id);
    while (id == 0)
        sys_frame_wait();
    return id;
}

int modmgr_load_and_start(int dir, const char *name)
{
    int status;
    int id;

    id = modmgr_load_sync(dir, name);
    if (id <= 0)
        return id;
    sceKernelStartModule(id, 0, 0, &status, 0);
    return id;
}

/* Copies each line into a scratch buffer (the print was compiled out) and counts them. */
static int menu_count_lines(const char *text, int sel)
{
    char line[128];
    char *p;
    int n = 0;

    for (;;) {
        p = line;
        while (*text != '\n' && *text != 0) {
            *p = *text;
            p++;
            text++;
        }
        if (p == line)
            break;
        *p = 0;
        n++;
        if (*text == 0)
            break;
        text++;
    }
    return n;
}

static int ehfile_read_done_cb(void *arg, int result)
{
    if (result < 0)
        return -1;
    *(int *)arg = result;
    return 1;
}

int modmgr_load_done_cb(void *arg, int result)
{
    if (result <= 0)
        return -1;
    *(int *)arg = result;
    return 1;
}

void modmgr_init(int arg_m1, int arg_m2)
{
    ModTableEntry *e = static_module_table;

    memset(&modmgr, 0, sizeof modmgr);
    if (static_module_table[0].name) {
        do {
            switch (e->arg) { /* mwcc tests cases in reverse source order: -2 is tested first */
            case -1:
                e->arg = arg_m1;
                break;
            case -2:
                e->arg = arg_m2;
                break;
            }
            e++;
        } while (e->name);
    }
    modmgr_register_table(static_module_table);
}

int modmgr_load_by_id(int id)
{
    return modmgr_load_ex(id, 0, 0, &modmgr_status_by_id, 0);
}

int modmgr_load_ex(int id, int argsize, void *argp, int *status, void *opt)
{
    ModEntry *m;

    m = modmgr_find(id);
    if (m == 0)
        return 1;
    if (status == 0)
        status = &modmgr_status_default;
    modmgr_addref(m);
    if (m->state != 0)
        return 0;
    switch (m->type) {
    case -1:
        return 0;
    case 0:
        ehfile_request_module(m->arg, m->name, modmgr_start, m);
        m->state = 1;
        m->argsize = argsize;
        m->argp = argp;
        m->status = status;
        m->opt = opt;
        return 0;
    case 1:
        sceUtilityLoadNetModule(m->handle);
        break;
    case 2:
        sceUtilityLoadAvModule(m->handle);
        break;
    case 3:
        sceUtilityLoadUsbModule(m->handle);
        break;
    }
    m->state = 2;
    return 0;
}

int modmgr_none_loading(void)
{
    ModMgr *mm = &modmgr;
    int i;

    for (i = 0; i < mm->count; i++)
        if (mm->entries[i].state == 1)
            return 0;
    return 1;
}

int modmgr_update(int id)
{
    ModEntry *m;
    int status;

    m = modmgr_find(id);
    if (m == 0)
        return 1;
    switch (m->state) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
        modmgr_release(m);
        if (m->refcount <= 0) {
            switch (m->type) {
            case -1:
                break;
            case 0:
                sceKernelStopModule(m->module_id, 0, 0, &status, 0);
                sceKernelUnloadModule(m->module_id);
                m->module_id = 0;
                break;
            case 1:
                sceUtilityUnloadNetModule(m->handle);
                break;
            case 2:
                sceUtilityUnloadAvModule(m->handle);
                break;
            case 3:
                sceUtilityUnloadUsbModule(m->handle);
                break;
            }
            m->state = 0;
        }
        break;
    }
    return 0;
}

int modmgr_register_table(const ModTableEntry *t)
{
    ModMgr *mm = &modmgr;
    ModEntry *m;
    int dup = 0;
    int i;

    while (t->name) {
        for (i = 0; i < mm->count; i++) {
            if (t->id == mm->entries[i].id) {
                dup = 1;
                break;
            }
        }
        if (i == mm->count) {
            m = &mm->entries[mm->count];
            memset(m, 0, sizeof *m);
            m->id = t->id;
            m->arg = t->arg;
            m->type = t->type;
            m->handle = t->handle;
            strncpy(m->name, t->name, 0x20);
            m->name[0x1F] = 0;
            mm->count++;
        }
        t++;
    }
    qsort(mm->entries, mm->count, sizeof(ModEntry), modmgr_cmp);
    return dup;
}

ModEntry *modmgr_find(int id)
{
    int key = id;
    return bsearch(&key, modmgr.entries, modmgr.count, sizeof(ModEntry), modmgr_cmp);
}

/* ehfile callback: the PRX is loaded, start it with the saved arguments. */
int modmgr_start(void *arg, int module_id)
{
    ModEntry *m = arg;

    if (module_id <= 0)
        return 0;
    m->module_id = module_id;
    sceKernelStartModule(module_id, m->argsize, m->argp, m->status, m->opt);
    m->state = 2;
    return 1;
}

int modmgr_cmp(const void *a, const void *b)
{
    return *(const int *)b - *(const int *)a;
}

static void modmgr_addref(ModEntry *m)
{
    m->refcount++;
}

static void modmgr_release(ModEntry *m)
{
    m->refcount--;
}
