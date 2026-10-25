// @range 0x0880D6C0-0x0880DBD0
// @rodata 0x088225B0-0x088225B8
// @data 0x08822C24-0x08822ED8
/* Resources: the deck/all_%c.ehp pack (read asynchronously, language letter in the name), the card
   image reader opened on its own heap block, and the resources looked up in the pack by id through
   a table sorted at startup (pack names in the language or, for the OCG pack lists, the region). */
#include "libdeck.h"

typedef struct DkRes {                 /* dk_res: resources of the pack, 12 bytes */
    unsigned short id;
    unsigned short pack;
    void *data;                        /* +0x04 */
    const char *name;                  /* +0x08 printf format, %c = language letter */
} DkRes;

typedef struct DkLoad {                /* dk_load 0x095D2710 */
    unsigned char ready : 1;           /* no read in flight */
    unsigned char step : 7;
} DkLoad;

int cfg_get_language(void);
int region_get(void);
int sprintf(char *buf, const char *fmt, ...);
void qsort(void *base, unsigned int n, unsigned int es, int (*cmp)(const void *, const void *));

extern void *dk_pack;                  /* 0x095D26F4 deck/all_%c.ehp */
extern void *dk_cip_mem;               /* 0x095D2700 */
extern DkLoad dk_load;                 /* 0x095D2710 */

int dk_pack_loaded(void *pack, int size);
void *dk_pack_get(void);

const unsigned char dk_lang[] = "jegfis";

DkRes dk_res[25] = {
    { 0x8001, 0, 0, "cis_all.gim" },
    { 0x8002, 0, 0, "ciss.gim" },
    { 0x8006, 0, 0, "panel.gim" },
    { 0x8007, 0, 0, "icon01.gim" },
    { 0x8008, 0, 0, "icon02.gim" },
    { 0x8009, 0, 0, "icon03.gim" },
    { 0x800A, 0, 0, "icon04.gim" },
    { 0x800B, 0, 0, "icon05.gim" },
    { 0x800C, 0, 0, "icon06.gim" },
    { 0x8010, 0, 0, "spelltrap.gim" },
    { 0x8011, 0, 0, "frame.gim" },
    { 0x8012, 0, 0, "detail_bg.gim" },
    { 0x8013, 0, 0, "s_cut.gim" },
    { 0x8014, 0, 0, "help_menu.gim" },
    { 0x8015, 0, 0, "ocglist.gim" },
    { 0x8021, 0, 0, "bg.ehf" },
    { 0x8022, 0, 0, "d_window0.ehf" },
    { 0x8023, 0, 0, "score-atkdef10.ehf" },
    { 0x8031, 0, 0, "fileTbl.dat" },
    { 0x8032, 0, 0, "sameCardTbl.dat" },
    { 0x8033, 0, 0, "strTbl.bin" },
    { 0x8034, 0, 0, "help_strtbl_contents.bin" },
    { 0x8035, 0, 0, "help_strtbl_label.bin" },
    { 0x8036, 0, 0, "ocg_packname_%c.bin" },
    { 0x8037, 0, 0, "ocg_packlist_%c.bin" },
};

int dk_u16_cmp(const void *a, const void *b)
{
    return *(unsigned short *)a - *(unsigned short *)b;
}

static int dk_res_find(unsigned short id, DkRes *t, int n)
{
    int lo = 0, hi = n - 1, mid;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (id == t[mid].id)
            return mid;
        if (t[mid].id < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

void dk_res_init(void)
{
    memset(&dk_load, 0, sizeof(dk_load));
    dk_load.ready = 1;
    qsort(dk_res, 25, sizeof(DkRes), dk_u16_cmp);
}

/* the pack, then the card image reader; 1 when both are ready */
int dk_res_load(void)
{
    if (dk_load.ready) {
        switch (dk_load.step) {
        case 0:
            if (dk_pack_get() != 0)
                dk_load.step++;
            if (dk_cip_mem == 0)
                dk_cip_mem = heap_alloc(dk_heap, 0xA2E000);
            dk_cip = cip_open_h(dk_cip_mem, 4, 1);
            break;
        case 1:
            if (cip_is_loaded(dk_cip))
                dk_load.step++;
            break;
        default:
            dk_load.step = 0;
            return 1;
        }
    }
    return 0;
}

void dk_res_free(void);

void dk_res_free_alias(void)
{
    dk_res_free();
}

void dk_res_free(void)
{
    if (dk_pack != 0) {
        heap_set_level(dk_heap, dk_pack);
        dk_pack = 0;
    }
    if (dk_cip_mem != 0) {
        heap_set_level(dk_heap, dk_cip_mem);
        dk_cip_mem = 0;
    }
}

/* the pack once loaded; otherwise starts the read (if none is in flight) and returns 0 */
void *dk_pack_get(void)
{
    char name[256];
    int size;

    if (dk_pack != 0)
        return dk_pack;
    if (dk_load.ready) {
        sprintf(name, "deck/all_%c.ehp", dk_lang[cfg_get_language()]);
        dk_load.ready = 0;
        size = ehfile_get_size(3, name);
        dk_pack = heap_alloc(dk_heap, size);
        ehfile_read_async_alias(3, name, dk_pack, size, dk_pack_loaded, dk_pack);
    }
    return 0;
}

/* resource by id, looked up in the pack on first use */
void *dk_res_get(unsigned short id)
{
    int i;
    DkRes *r;
    void *pack;
    char name[256];

    i = dk_res_find(id, dk_res, 25);
    if (i < 0)
        return 0;
    r = &dk_res[i];
    if (r->data != 0)
        return r->data;
    pack = dk_pack_get();
    if (pack == 0)
        return 0;
    if (id - 0x8036 < 2u) {
        char lang[7] = "jagfisb";

        sprintf(name, r->name, lang[region_get()]);
    } else {
        sprintf(name, r->name, dk_lang[cfg_get_language()]);
    }
    r->data = ehp_find(pack, name, 0);
    return r->data;
}

int dk_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack != 0)
            ehp_mount(0, pack);
        dk_load.step++;
    }
    dk_load.ready = 1;
    return ok;
}
