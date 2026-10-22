// @range 0x08804000-0x08804690
// @data 0x0880A560-0x0880A5CC
/* Library entry points (init/update), state machine of the duel record screen and loading of
   database/duelrecord.ehp with the lookup of its files (tab.gim, parts.gim, strTbl_<lang>.bin). */
#include "psp.h"
#include "libc.h"
#include "font.h"
#include "libduelrecord.h"

void drec_build(void);
void drec_input(void);
int drec_term(void);
void drec_files_init(void);
int drec_load_poll(void);
void drec_pack_free(void);
void *drec_pack_load(void);
int drec_pack_loaded(void *pack, int size);
int drec_file_cmp(const void *a, const void *b);
int drec_bsearch(unsigned short key, void *base, int n, int size);

int module_entry(void)
{
    return 0;
}

void duelrecord_init(void *heap)
{
    memset(&drec, 0, 0x34);
    if (heap == 0) {
        drec.fpl = sceKernelCreateFpl("", 2, 0, 0x200000, 1, 0);
        sceKernelAllocateFpl(drec.fpl, &drec.fpl_mem, 0);
        drec.heap = heap_create(drec.fpl_mem, 0x200000);
    } else {
        drec.heap = heap;
    }
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    drec_files_init();
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
}

DrecFile drec_files[3] = {
    { 0x1002, 0, "tab.gim" },
    { 0x1003, 0, "parts.gim" },
    { 0x1020, 0, "strTbl_%c.bin" },
};

int duelrecord_update(void)
{
    switch (drec.state) {
    case 0:
        if (drec_load_poll()) {
            drec_build();
            drec.state = 1;
            fade_start(1, 0xFF000000, 16);
            gfx_overlay_enable(0);
        }
        break;
    case 1:
        if (fade_is_done())
            drec.state = 2;
        break;
    case 2:
        drec_input();
        break;
    case 3:
        if (fade_is_done())
            drec.state = 4;
        break;
    case 4:
        if (drec_term())
            return 1;
        break;
    }
    return 0;
}

int drec_term(void)
{
    int i;

    drec_pack_free();
    for (i = 0; i < 3; i++) {
        if (drec.entries[i] != 0)
            heap_set_level(drec.heap, drec.entries[i]);
    }
    if (drec.fpl > 0) {
        sceKernelFreeFpl(drec.fpl, drec.fpl_mem);
        sceKernelDeleteFpl(drec.fpl);
    }
    return 1;
}

void drec_files_init(void)
{
    memset(&drec_pack_flags, 0, 1);
    drec_pack_flags.ready = 1;
    qsort(drec_files, 3, sizeof(DrecFile), drec_file_cmp);
}

int drec_load_poll(void)
{
    if (drec_pack_flags.ready) {
        switch (drec_pack_flags.loads) {
        case 0:
            if (drec_pack_load() != 0)
                drec_pack_flags.loads++;
            break;
        default:
            drec_pack_flags.loads = 0;
            return 1;
        }
    }
    return 0;
}

static void drec_pack_release(void);

void drec_pack_free(void)
{
    drec_pack_release();
}

static void drec_pack_release(void)
{
    if (drec_pack != 0) {
        heap_set_level(drec.heap, drec_pack);
        drec_pack = 0;
    }
}

void *drec_pack_load(void)
{
    int size;

    if (drec_pack != 0)
        return drec_pack;
    if (drec_pack_flags.ready) {
        drec_pack_flags.ready = 0;
        size = ehfile_get_size(3, "database/duelrecord.ehp");
        drec_pack = heap_alloc(drec.heap, size);
        ehfile_read_async_alias(3, "database/duelrecord.ehp", drec_pack, size, drec_pack_loaded, drec_pack);
    }
    return 0;
}

void *drec_get_file(unsigned short id)
{
    int i;
    DrecFile *f;
    void *pack;
    char name[256];

    i = drec_bsearch(id, drec_files, 3, sizeof(DrecFile));
    if (i < 0)
        return 0;
    f = &drec_files[i];
    if (f->data != 0)
        return f->data;
    pack = drec_pack_load();
    {
        char lang[6] = { 'j', 'e', 'g', 'f', 'i', 's' };

        if (pack == 0)
            return 0;
        sprintf(name, f->name, lang[cfg_get_language()]);
        return f->data = ehp_find(pack, name, 0);
    }
}

int drec_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack != 0)
            ehp_mount(0, pack);
        drec_pack_flags.loads++;
    }
    drec_pack_flags.ready = 1;
    return ok;
}

int drec_file_cmp(const void *a, const void *b)
{
    return *(unsigned short *)a - *(unsigned short *)b;
}

int drec_bsearch(unsigned short key, void *base, int n, int size)
{
    int half = size / 2;
    int lo = 0;
    int hi = n - 1;
    int mid;
    unsigned short v;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        v = ((unsigned short *)base)[mid * half];
        if (key == v)
            return mid;
        if (v < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}
