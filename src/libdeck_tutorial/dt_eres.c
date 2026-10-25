// @range 0x088175FC-0x08817B78
// @rodata 0x0882106C-0x08821074
// @data 0x0882226C-0x08822520
/* The editor's per-frame cursor update (from libdeck dk_scene.c) and its resources (the code of
   libdeck dk_res.c, next to the lesson's own pack of dt_res.c). Resources: the deck/all_%c.ehp pack (read asynchronously, language letter in the name), the card
   image reader opened on its own heap block, and the resources looked up in the pack by id through
   a table sorted at startup (pack names in the language or, for the OCG pack lists, the region). */
#include "libdeck_tutorial.h"

typedef struct DtERes {                 /* dt_eres: resources of the pack, 12 bytes */
    unsigned short id;
    unsigned short pack;
    void *data;                        /* +0x04 */
    const char *name;                  /* +0x08 printf format, %c = language letter */
} DtERes;

typedef struct DtELoad {                /* dt_eload 0x095D2710 */
    unsigned char ready : 1;           /* no read in flight */
    unsigned char step : 7;
} DtELoad;

int cfg_get_language(void);
int region_get(void);
int sprintf(char *buf, const char *fmt, ...);
void qsort(void *base, unsigned int n, unsigned int es, int (*cmp)(const void *, const void *));

extern void *dt_epack;                 /* 0x08823DB4 deck/all_%c.ehp */
extern void *dt_ecip_mem;              /* 0x08823DC0 */
extern DtELoad dt_eload;               /* 0x08824720 */
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehp_mount(void (*cb)(void *), void *pack);
void *ehp_find(void *pack, const char *name, int *size);

int dt_epack_loaded(void *pack, int size);
void *dt_epack_get(void);

/* the cursor card: marked as seen after 16 frames, info text refreshed when it changes */
void dt_cursor_update(void)
{
    DtEnt *e;

    if (++dtv.cursor.timer >= 0x10) {
        if (dtv.cursor.ent != 0)
            dtv.cursor.ent->marked = 1;
        if (dtv.cursor.timer > 0x8000)
            dtv.cursor.timer = 0x100;
    }
    if (!dtv.lock && (e = dt_cur_entry(dtv.side)) != 0 && e->id != dtv.cursor.id)
        dt_info_update();
}

const unsigned char dt_elang[] = "jegfis";

DtERes dt_eres[25] = {
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

int dt_u16_cmp(const void *a, const void *b)
{
    return *(unsigned short *)a - *(unsigned short *)b;
}

static int dt_eres_find(unsigned short id, DtERes *t, int n)
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

void dt_files_init(void)
{
    memset(&dt_eload, 0, sizeof(dt_eload));
    dt_eload.ready = 1;
    qsort(dt_eres, 25, sizeof(DtERes), dt_u16_cmp);
}

/* the pack, then the card image reader; 1 when both are ready */
int dt_files_poll(void)
{
    if (dt_eload.ready) {
        switch (dt_eload.step) {
        case 0:
            if (dt_epack_get() != 0)
                dt_eload.step++;
            if (dt_ecip_mem == 0)
                dt_ecip_mem = heap_alloc(dts.heap, 0xCE000);   /* a smaller reader than libdeck (0xA2E000) */
            dts.cip = cip_open_h(dt_ecip_mem, 4, 0);
            break;
        case 1:
            if (cip_is_loaded(dts.cip))
                dt_eload.step++;
            break;
        default:
            dt_eload.step = 0;
            return 1;
        }
    }
    return 0;
}

/* the pack once loaded; otherwise starts the read (if none is in flight) and returns 0 */
void *dt_epack_get(void)
{
    char name[256];
    int size;

    if (dt_epack != 0)
        return dt_epack;
    if (dt_eload.ready) {
        sprintf(name, "deck/all_%c.ehp", dt_elang[cfg_get_language()]);
        dt_eload.ready = 0;
        size = ehfile_get_size(3, name);
        dt_epack = heap_alloc(dts.heap, size);
        ehfile_read_async_alias(3, name, dt_epack, size, dt_epack_loaded, dt_epack);
    }
    return 0;
}

/* resource by id, looked up in the pack on first use */
void *dt_eres_get(unsigned short id)
{
    int i;
    DtERes *r;
    void *pack;
    char name[256];

    i = dt_eres_find(id, dt_eres, 25);
    if (i < 0)
        return 0;
    r = &dt_eres[i];
    if (r->data != 0)
        return r->data;
    pack = dt_epack_get();
    if (pack == 0)
        return 0;
    if (id - 0x8036 < 2u) {
        char lang[7] = "jagfisb";

        sprintf(name, r->name, lang[region_get()]);
    } else {
        sprintf(name, r->name, dt_elang[cfg_get_language()]);
    }
    r->data = ehp_find(pack, name, 0);
    return r->data;
}

int dt_epack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack != 0)
            ehp_mount(0, pack);
        dt_eload.step++;
    }
    dt_eload.ready = 1;
    return ok;
}

