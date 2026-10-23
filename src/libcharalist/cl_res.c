// @range 0x08804000-0x08804810
// @data 0x0880DEBC-0x0880E018
/* Resources of the screen: database/charalist.ehp (textures, strings, per-duelist voice tables) read
   asynchronously and mounted, and the gzipped bust-up packs of field/bustup/, one unpacked at a time. */
#include "libcharalist.h"

typedef struct ClFile {
    unsigned short id;
    void *data;                /* found entry, cached */
    const char *name;          /* "%c" = language letter */
} ClFile;

int cl_res_cmp(const void *a, const void *b);
int cl_pack_loaded(void *pack, int size);
int cl_bustup_loaded(void *buf, int size);
int cl_bsearch(unsigned short key, void *base, int n, int size);
int read_u32_before(void *p, int size);
void gzip_decompress_alias(void *dst, int size, void *src);

ClFile cl_files[9] = {
    { 0x1001, 0, "bg.gim" },
    { 0x1002, 0, "tab.gim" },
    { 0x1003, 0, "parts.gim" },
    { 0x1004, 0, "main_face01.gim" },
    { 0x1005, 0, "main_face02.gim" },
    { 0x1006, 0, "common_head.gim" },
    { 0x1007, 0, "common_body.gim" },
    { 0x1008, 0, "voice_test.gim" },
    { 0x1020, 0, "strTbl_%c.bin" },
};

void cl_res_init(void)
{
    memset(&cl_res, 0, 1);
    cl_res.ready = 1;
    memset(&cl_bust, 0, sizeof(cl_bust));
    cl_bust.ready = 1;
    qsort(cl_files, 9, sizeof(ClFile), cl_res_cmp);
}

/* 1 once the pack is mounted */
int cl_load_poll(void)
{
    if (cl_res.ready) {
        switch (cl_res.step) {
        case 0:
            if (cl_pack_load())
                cl_res.step++;
            break;
        default:
            cl_res.step = 0;
            return 1;
        }
    }
    return 0;
}

/* frees everything once no read is in flight; 1 when done */
int cl_res_release(void)
{
    if (ehfile_busy())
        return 0;
    charalist_res_free();
    return 1;
}

/* 0x08804178: export */
void charalist_res_free(void)
{
    int i;
    int j;

    if (cl_pack) {
        heap_set_level(cl.heap, cl_pack);
        cl_pack = 0;
    }
    for (i = 0; i < CL_CHARAS; i++) {
        for (j = 0; j < 3; j++) {
            if (cl_bust.data[i][j]) {
                heap_set_level(cl.heap, cl_bust.data[i][j]);
                cl_bust.data[i][j] = 0;
            }
        }
    }
    if (cl_bust.mem) {
        heap_set_level(cl.heap, cl_bust.mem);
        cl_bust.mem = 0;
    }
}

/* starts the read the first time; the pack once it is in memory */
void *cl_pack_load(void)
{
    int size;

    if (cl_pack)
        return cl_pack;
    if (cl_res.ready) {
        cl_res.ready = 0;
        size = ehfile_get_size(3, "database/charalist.ehp");
        cl_pack = heap_alloc(cl.heap, size);
        ehfile_read_async_alias(3, "database/charalist.ehp", cl_pack, size, cl_pack_loaded, cl_pack);
    }
    return 0;
}

void *cl_res_get(unsigned short id)
{
    ClFile *f;
    void *pack;
    int i = cl_bsearch(id, cl_files, 9, sizeof(ClFile));

    if (i < 0)
        return 0;
    f = &cl_files[i];
    if (f->data)
        return f->data;
    pack = cl_pack_load();
    {
        char name[256];
        char codes[6] = "jegfis";   /* language letters by cfg_get_language() */
        if (pack == 0)
            return 0;
        sprintf(name, f->name, codes[cfg_get_language()]);
        return f->data = ehp_find(pack, name, 0);
    }
}

/* body (which 0) or mouth (1) animation of the duelist's current costume; starts reading and
   unpacking its bust-up pack as needed, 0 meanwhile */
void *cl_bustup_get(ClEntry *e, unsigned short which)
{
    ClBust *b = &cl_bust;
    unsigned short id;
    unsigned short var;
    void *data;
    int size;
    void *pack;
    char name[256];

    if (!cl_bust.ready)
        return 0;
    if (e->simple)
        return 0;
    id = e->id;
    var = e->u.info->cur;
    if (id == b->id && var == b->var)
        return b->ehf[which];
    data = b->data[id][var];
    if (data) {
        size = read_u32_before(data, b->size[id][var]);
        if (b->mem)
            heap_set_level(cl.heap, b->mem);
        b->mem = heap_alloc(cl.heap, size);
        gzip_decompress_alias(b->mem, size, data);
        pack = b->mem;
        ehp_mount(0, pack);
        b->ehf[0] = ehp_find(pack, "body.ehf", 0);
        b->ehf[1] = ehp_find(pack, "mouth.ehf", 0);
        ehf_set_anim(b->ehf[0], 0, 1);
        ehf_set_anim(b->ehf[1], 0, 1);
        b->id = id;
        b->var = var;
        return b->ehf[which];
    }
    sprintf(name, "field/bustup/bu_%s.ehp.gz", e->u.info->bustup[var]);
    size = ehfile_get_size(3, name);
    b->size[id][var] = size;
    b->ready = 0;
    b->data[id][var] = heap_alloc(cl.heap, size);
    ehfile_read_async_alias(3, name, b->data[id][var], size, cl_bustup_loaded, 0);
    return 0;
}

/* table of the voice ids of duelist `id`, costume `var` */
void *cl_voicetbl_get(unsigned short id, unsigned short var)
{
    char name[256];
    void *pack = cl_pack_load();

    if (pack == 0)
        return 0;
    sprintf(name, "chr%03d%02d_voiceidchgtbl.bin", id, var);
    return ehp_find(pack, name, 0);
}

int cl_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        cl_res.step++;
    }
    cl_res.ready = 1;
    return ok;
}

int cl_bustup_loaded(void *buf, int size)
{
    cl_bust.ready = 1;
    return size > 0;
}

int cl_res_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

/* binary search over records of `size` bytes keyed by their first u16 */
int cl_bsearch(unsigned short key, void *base, int n, int size)
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
