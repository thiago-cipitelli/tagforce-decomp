// @range 0x0884D65C-0x0884DF2C
// @rodata 0x08870B90-0x08870C34
// @data 0x08876D00-0x08876D14
/* The duel characters (dd_chara_mgr): four slots with the model, camera and voice of the
   duelists, the one currently shown, the per-character setup callback and the face table
   (_kao-ptrs.txt) of a character's pack. */
#include "libduel_draw.h"

void *memset(void *, int, unsigned int);
void *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(void *list);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_get_proj(float *fovy, float *aspect, float *near, float *far);
void mtx_perspective(float fovy, float aspect, float near, float far);
void *mtx_flush_alias(void *list);
void snd_channel_update(int slot);
void *ehp_find(void *pack, const char *name, int *size);
int sysvar_get_a98(void);
int charmap_lookup(int ch, int *kind_out);
int charmap_lookup2(int code, DdCharInfo *out);

void caption_init(void);
void voice_tables_init(void);
void voice_tables_term(void);
void caption_term(void);
void dd_chr_free(DdCharSlot *s, int all);
void *dd_chr_draw(void *list, DdCharSlot *s);
void dd_mtx_save(void);
void dd_chr_tick(DdCharSlot *s);
void dd_chr_update(DdCharSlot *s);
void dd_mon_cb_init(int chr, int variant, void *setup);
void dd_chr_init(DdCharSlot *s, void *setup);
void dd_chr_load(DdCharSlot *s);
void dd_lp_close(void);
void dd_chr_anim_set(DdCharSlot *s, int arg);
void voice_reset(DdCharSlot *s);
void voice_start(DdCharSlot *s, int a, int b);
int dd_chr_anim_done(DdCharSlot *s);
void dd_chr_bone_mtx(DdCharSlot *s, void *out, int bone);
void dd_chr_attach_mtx(DdCharSlot *s, void *out, int bone);
void voice_stop(DdCharSlot *s);
void dd_chr_flag_set(DdCharSlot *s, int arg);
int dd_chr_flag_get(DdCharSlot *s);
void *dd_cutin_get(unsigned int chr, int variant);
void *dd_ryusen_draw(void *list);
void dd_ryusen_set_layer(int layer);
void dd_ryusen_set_chara(unsigned int chr, int variant);
void dd_ryusen_set_alpha(int alpha, int now);

typedef void (*DdCharSetup)(void *setup);
void dd_hookb_install(void *);
void chr3_setup(void *);
void chr4_setup(void *);
void dd_hooka_install(void *);
void dd_chara_setup_06b(void *);
void dd_chara_setup_06a(void *);
void chr7_setup(void *);
void chr8_setup(void *);
void chr9_setup(void *);
void chr10_setup(void *);
void chr11_setup(void *);
void chr12_setup(void *);
void chr13_setup(void *);
void chr14_setup(void *);
void chr15_setup(void *);
void chr16_setup(void *);
void chr17_setup(void *);
void chr18_setup(void *);
void chr18_alt_setup(void *);
void chr19_setup(void *);
void chr20_setup(void *);
void chr21_setup(void *);
void chr22_setup(void *);
void chr23_setup(void *);
void chr24_setup(void *);
void chr25_setup(void *);
void chr26_setup(void *);
void chr27_setup(void *);
void chr28_setup(void *);
void chr30_setup(void *);
void chr31_setup(void *);
void chr32_setup(void *);
void chr33_setup(void *);
void chr36_setup(void *);
void chr37_setup(void *);
void chr38_setup(void *);
void chr40_setup(void *);
void chr1000_setup(void *);
void chr_generic_setup(void *);
void chr_kind3_setup(void *);

extern DdCharMgr dd_chara_mgr;

DdCharSetup dd_chara_setup_fn(int chr, int variant);
int dd_chara_redraw(int arg);

char *dd_kao_ptrs_name = "_kao-ptrs.txt";

void dd_chara_init(void)
{
    memset(&dd_chara_mgr, 0, sizeof(dd_chara_mgr));
    dd_chara_mgr.cur_idx = -1;
    caption_init();
    voice_tables_init();
}

void dd_chara_term(void)
{
    DdCharMgr *m = &dd_chara_mgr;
    int i;
    DdCharMgr *p;

    voice_tables_term();
    caption_term();
    for (i = 0, p = m; i < 4; i++, p = (DdCharMgr *)((char *)p + sizeof(DdCharSlot)))
        dd_chr_free(p->slot, 1);
    memset(m, 0, sizeof(*m));
    m->cur_idx = -1;
}

/* Draws the current character on layer 7 behind the ryusen lines, with the far plane at 10000. */
void dd_chara_draw(void)
{
    void *list;
    float fovy;
    float aspect;
    float near;
    float far;

    list = dl_select_layer(7, 0);
    list = dd_ryusen_draw(list);
    mtx_set_mode(0);
    mtx_push();
    mtx_set_mode(1);
    mtx_push();
    mtx_set_mode(2);
    mtx_push();
    list = dd_chr_draw(list, dd_chara_mgr.cur);
    mtx_get_proj(&fovy, &aspect, &near, &far);
    far = 10000.0f;
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(fovy, aspect, near, far);
    list = mtx_flush_alias(list);
    dd_mtx_save();
    dl_set_list(list);
    mtx_set_mode(0);
    mtx_pop();
    mtx_set_mode(1);
    mtx_pop();
    mtx_set_mode(2);
    mtx_pop();
    dl_set_list(mtx_flush_alias(dl_select_layer(8, 0)));
}

void dd_chara_update(void)
{
    dd_chr_tick(dd_chara_mgr.cur);
}

int dd_chara_update2(void)
{
    dd_chr_update(dd_chara_mgr.cur);
    return 0;
}

/* Loads character `chr` into slot `i`. */
void dd_chara_load(int i, int chr, int variant)
{
    DdCharSlot *s = &dd_chara_mgr.slot[i];
    DdCharSetup fn;
    char setup[52];

    fn = dd_chara_setup_fn(chr, variant);
    dd_mon_cb_init(chr, variant, setup);
    if (fn != 0)
        fn(setup);
    dd_chr_init(s, setup);
    dd_chr_load(s);
    s->index = i;
}

void dd_chara_select(int i)
{
    DdCharMgr *m = &dd_chara_mgr;

    if (m->cur_idx != i) {
        dd_chara_redraw(-1);
        m->cur = &m->slot[i];
        m->cur_idx = i;
        dd_chara_redraw(-1);
    }
}

void dd_chara_set_voice(int v)
{
    dd_chara_mgr.cur->voice = v;
}

void dd_chara_set_anim_range(int i, int from, int len)
{
    int to = from + len;
    /* the original adds i * 0x180 to the manager's base, not to &slot[0] */
    DdCharMgr *m = (DdCharMgr *)((char *)&dd_chara_mgr + i * sizeof(DdCharSlot));

    m->slot[0].anim_from = from;
    m->slot[0].anim_to = to;
}

int dd_chara_redraw(int arg)
{
    DdCharMgr *m = &dd_chara_mgr;

    if (m->cur == 0)
        return 1;
    dd_lp_close();
    dd_ryusen_set_layer(0x2000B);
    dd_ryusen_set_chara(m->cur->chr, m->cur->variant);
    dd_ryusen_set_alpha(0xFF, 1);
    dd_chr_anim_set(m->cur, arg);
    voice_reset(m->cur);
    return 1;
}

void dd_chara_anim(int a, int b)
{
    voice_start(dd_chara_mgr.cur, a, b);
}

int dd_chara_ready(void)
{
    DdCharMgr *m = &dd_chara_mgr;

    if (m->busy != 0)
        return 1;
    return dd_chr_anim_done(m->cur);
}

void dd_chara_busy_clear(void)
{
    dd_chara_mgr.busy = 0;
}

void dd_chara_reset(void)
{
    DdCharMgr *m = &dd_chara_mgr;

    if (m->cur != 0) {
        dd_chara_redraw(-1);
        dd_lp_close();
        snd_channel_update(m->cur->index);
    }
}

void dd_chara_bone_pos(void *out, int bone)
{
    dd_chr_bone_mtx(dd_chara_mgr.cur, out, bone);
}

void dd_chara_bone_mtx(void *out, int bone)
{
    dd_chr_attach_mtx(dd_chara_mgr.cur, out, bone);
}

float *dd_chara_view(void)
{
    return dd_chara_mgr.cur->view;
}

float *dd_chara_proj(void)
{
    return dd_chara_mgr.cur->proj;
}

float *dd_chara_pos(void)
{
    return dd_chara_mgr.cur->pos;
}

void dd_chara_busy_set(void)
{
    dd_chara_mgr.busy = 1;
    voice_stop(dd_chara_mgr.cur);
}

int dd_chara_busy(void)
{
    return dd_chara_mgr.busy;
}

void dd_chara_set_08813FB4(int arg)
{
    dd_chr_flag_set(dd_chara_mgr.cur, arg);
}

int dd_chara_get_08813FBC(void)
{
    return dd_chr_flag_get(dd_chara_mgr.cur);
}

/* Face entry of slot `i`: from _kao-ptrs.txt in the character's pack, by variant for the two
   player characters when sysvar a98 is set. */
int dd_chara_kao(int i)
{
    unsigned int chr = dd_chara_mgr.slot[i].chr;
    int variant = dd_chara_mgr.slot[i].variant;
    void *pack;
    int *t;

    pack = dd_cutin_get(chr, variant);
    if (pack == 0)
        return 0;
    t = ehp_find(pack, dd_kao_ptrs_name, 0);
    if (t == 0)
        return 0;
    if (sysvar_get_a98() != 0 && chr < 2)
        return t[variant + 1];
    return t[0];
}

unsigned int dd_chara_chr(int i)
{
    return dd_chara_mgr.slot[i].chr;
}

int dd_chara_variant(int i)
{
    return dd_chara_mgr.slot[i].variant;
}

/* Setup callback of a character (camera, lights, animations of its model). */
DdCharSetup dd_chara_setup_fn(int chr, int variant)
{
    DdCharSetup fn = 0;
    int code;

    switch (chr) {
    case 2: fn = dd_hookb_install; break;
    case 3: fn = chr3_setup; break;
    case 4: fn = chr4_setup; break;
    case 5: fn = dd_hooka_install; break;
    case 6:
        if (variant == 0)
            fn = dd_chara_setup_06a;
        else
            fn = dd_chara_setup_06b;
        break;
    case 7: fn = chr7_setup; break;
    case 8: fn = chr8_setup; break;
    case 9: fn = chr9_setup; break;
    case 10: fn = chr10_setup; break;
    case 11: fn = chr11_setup; break;
    case 12: fn = chr12_setup; break;
    case 13: fn = chr13_setup; break;
    case 14: fn = chr14_setup; break;
    case 15: fn = chr15_setup; break;
    case 16: fn = chr16_setup; break;
    case 17: fn = chr17_setup; break;
    case 18:
        if (variant == 0)
            fn = chr18_setup;
        else
            fn = chr18_alt_setup;
        break;
    case 19: fn = chr19_setup; break;
    case 20: fn = chr20_setup; break;
    case 21: fn = chr21_setup; break;
    case 22: fn = chr22_setup; break;
    case 23: fn = chr23_setup; break;
    case 24: fn = chr24_setup; break;
    case 25: fn = chr25_setup; break;
    case 26: fn = chr26_setup; break;
    case 27: fn = chr27_setup; break;
    case 28: fn = chr28_setup; break;
    case 29: fn = 0; break;
    case 30: fn = chr30_setup; break;
    case 31: fn = chr31_setup; break;
    case 32: fn = chr32_setup; break;
    case 33: fn = chr33_setup; break;
    case 34: fn = 0; break;
    case 35: fn = 0; break;
    case 36: fn = chr36_setup; break;
    case 37: fn = chr37_setup; break;
    case 38: fn = chr38_setup; break;
    case 39: fn = 0; break;
    case 40: fn = chr40_setup; break;
    }
    if (fn == 0) {
        DdCharInfo info = { 0 };

        code = charmap_lookup(chr, 0);
        charmap_lookup2(code, &info);
        if (code == 1000)
            fn = chr1000_setup;
        else if (info.kind < 3)
            fn = chr_generic_setup;
        else if (info.kind == 3)
            fn = chr_kind3_setup;
        else
            fn = chr_generic_setup;
    }
    return fn;
}
