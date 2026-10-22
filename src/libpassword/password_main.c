// @range 0x08804000-0x08804844
// @rodata 0x0880B420-0x0880B444
// @data 0x0880B484-0x0880B650
/* Password screen entry: init, per-frame scene dispatch, unload, and the resources of
   labo/password.ehp (read asynchronously, looked up by id, language letter in the file names). */
#include "libpassword.h"

typedef struct PwRes {                 /* 0x10 */
    unsigned short id;
    void *data;                        /* +0x04 cached pointer into the pack (or a heap copy) */
    const char *name;                  /* +0x08 printf format, %c = language letter */
    const char *tmf;                   /* +0x0C */
} PwRes;

typedef struct CardListCfg {           /* see BOOT cardlist.c */
    void *gim;
    void *digits_gim;
    void *frame_ehf;
    void *attr_gim;
    void *kind_gim;
    void *cil_gim;
    void *icon_gim;
    void *cip;
    int frame_view;
} CardListCfg;

extern unsigned char pw_heap_mem[];    /* 0x0880B670, 3 MB */
extern unsigned char pw_cip_work[];    /* 0x08B0C7B4 */

int res_load_step(void);
void *res_pack_request(void);
int res_load_cb(void *pack, int size);
int res_cmp(const void *a, const void *b);
int res_find(unsigned short id, PwRes *t, int n);
int password_unload(void);
void res_reset(void);
void st_intro(void);
void st_input(void);
void st_check(void);
int st_found(void);
int st_not_found(void);
int st_quit(void);

PwRes res_table[17] = {
    { RES_BG, 0, "bg.gim", 0 },
    { RES_MONITOR, 0, "monitor_shadow.gim", 0 },
    { RES_ICON01, 0, "icon01.gim", 0 },
    { RES_ICON03, 0, "icon03.gim", 0 },
    { RES_FRAME, 0, "frame.gim", 0 },
    { RES_SPELLTRAP, 0, "spelltrap_%c.gim", 0 },
    { RES_KEYPAD, 0, "key.ehf", "key.tmf" },
    { 0x1011, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1012, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1013, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1014, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1015, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1016, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1017, 0, "nixi.ehf", "nixi.tmf" },
    { 0x1018, 0, "nixi.ehf", "nixi.tmf" },
    { RES_SCORE, 0, "score-atkdef10.ehf", 0 },
    { RES_STRINGS, 0, "strTbl_%c.bin", 0 },
};

int module_entry(void)
{
    return 0;
}

void password_init(void)
{
    memset(&pw, 0, sizeof(pw));
    pw.heap = heap_create(pw_heap_mem, 0x300000);
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    res_reset();
    snd_cmd_fefa();
    bgm_fade(0x150);
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    font_set_line_extra(0x80);
}

int password_update(void)
{
    switch (pw.scene) {
    case 0:
        if (res_load_step()) {
            pw.scene = 1;
            fade_start(1, 0xFF000000, 16);
            bgm_play(8);
            se_play(0x627E);
            gfx_overlay_enable(0);
        }
        break;
    case 1:
        st_intro();
        break;
    case 2:
        st_input();
        break;
    case 3:
        st_check();
        break;
    case 4:
        if (st_found()) {
            pw.step = 0;
            pw.timer = 0;
            pw.scene = 7;
            fade_start_in(1, 0xFF000000, 16);
            bgm_cmd(600);
        }
        break;
    case 5:
        if (st_not_found()) {
            pw.step = 0;
            pw.timer = 0;
            pw.scene = 7;
            fade_start_in(1, 0xFF000000, 16);
            bgm_cmd(600);
        }
        break;
    case 6:
        if (st_quit()) {
            pw.step = 0;
            pw.timer = 0;
            pw.scene = 7;
            fade_start_in(1, 0xFF000000, 16);
            bgm_cmd(600);
        }
        break;
    case 7:
        if (fade_is_done())
            pw.scene = 8;
        break;
    case 8:
        if (password_unload())
            return 1;
        break;
    }
    return 0;
}

/* scene 8: wait for the sound effects, release the card list and the card image reader */
int password_unload(void)
{
    switch (pw.step) {
    case 0:
        if (snd_count_se() == 0) {
            bgm_cmd3(0x150);
            pw.step++;
        }
        break;
    case 1:
        font_set_line_extra(0);
        cardlist_clear();
        if (pw.cip != 0) {
            cip_close(pw.cip);
            pw.step++;
        }
        break;
    case 2:
        if (cip_is_idle(pw.cip))
            return 1;
        break;
    }
    return 0;
}

void res_reset(void)
{
    memset(&pw_load, 0, sizeof(pw_load));
    pw_load.ready = 1;
    qsort(res_table, 17, sizeof(PwRes), res_cmp);
}

/* scene 0: open the card image reader, read the pack, hand the card list its sprites */
int res_load_step(void)
{
    CardListCfg cfg;

    if (pw_load.ready) {
        switch (pw_load.step) {
        case 0:
            pw.cip = cip_open_h(pw_cip_work, 0xB000, 0);
            res_pack_request();
            break;
        case 1:
            if (cip_is_loaded(pw.cip))
                pw_load.step++;
            break;
        case 2:
            keypad_reset();
            memset(&cfg, 0, sizeof(cfg));
            cfg.gim = res_get(RES_ICON01);
            cfg.digits_gim = res_get(RES_ICON03);
            cfg.frame_ehf = res_get(RES_SCORE);
            cfg.attr_gim = res_get(RES_FRAME);
            cfg.kind_gim = res_get(RES_SPELLTRAP);
            cfg.cip = pw.cip;
            cardlist_init(&cfg);
            pw_load.step++;
            break;
        default:
            pw_load.step = 0;
            return 1;
        }
    }
    return 0;
}

/* the pack once loaded; otherwise starts the read (if none is in flight) and returns 0 */
void *res_pack_request(void)
{
    int size;

    if (pw_pack != 0)
        return pw_pack;
    if (pw_load.ready) {
        pw_load.ready = 0;
        size = ehfile_get_size(3, "labo/password.ehp");
        pw_pack = heap_alloc(pw.heap, size);
        ehfile_read_async_alias(3, "labo/password.ehp", pw_pack, size, res_load_cb, pw_pack);
    }
    return 0;
}

/* resource by id: looked up in the pack on first use; the digit tubes get a private copy each */
void *res_get(unsigned short id)
{
    int i;
    PwRes *r;
    void *pack;
    char name[256];
    void *src;
    int size;

    i = res_find(id, res_table, 17);
    if (i < 0)
        return 0;
    r = &res_table[i];
    if (r->data != 0)
        return r->data;
    pack = res_pack_request();
    {
        char lang[6] = "jegfis";

        if (pack == 0)
            return 0;
        sprintf(name, r->name, lang[cfg_get_language()]);
    }
    r->data = ehp_find(pack, name, 0);
    if (id >= 0x1011 && id < 0x1019) {
        src = r->data;
        size = ehp_get_size_cur(pack, src);
        r->data = heap_alloc(pw.heap, size);
        memcpy(r->data, src, size);
    }
    return r->data;
}

int res_load_cb(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack != 0)
            ehp_mount(0, pack);
        pw_load.step++;
    }
    pw_load.ready = 1;
    return ok;
}

int res_cmp(const void *a, const void *b)
{
    return ((PwRes *)a)->id - ((PwRes *)b)->id;
}

int res_find(unsigned short id, PwRes *t, int n)
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
