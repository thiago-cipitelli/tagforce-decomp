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

void func_088587EC(void);
void func_08865060(void);
void func_088650E4(void);
void func_08858800(void);
void func_08813840(DdCharSlot *s, int all);
void *func_088138B8(void *list, DdCharSlot *s);
void func_088276BC(void);
void func_088138F8(DdCharSlot *s);
void func_08813974(DdCharSlot *s);
void func_088282E4(int chr, int variant, void *setup);
void func_08813554(DdCharSlot *s, void *setup);
void func_08813684(DdCharSlot *s);
void dd_lp_close(void);
void func_08813AB8(DdCharSlot *s, int arg);
void func_0886529C(DdCharSlot *s);
void func_088652BC(DdCharSlot *s, int a, int b);
int func_08813F1C(DdCharSlot *s);
void func_08813D48(DdCharSlot *s, void *out, int bone);
void func_08813D8C(DdCharSlot *s, void *out, int bone);
void func_0886533C(DdCharSlot *s);
void func_08813FB4(DdCharSlot *s, int arg);
int func_08813FBC(DdCharSlot *s);
void *func_0880507C(unsigned int chr, int variant);
void *dd_ryusen_draw(void *list);
void dd_ryusen_set_layer(int layer);
void dd_ryusen_set_chara(unsigned int chr, int variant);
void dd_ryusen_set_alpha(int alpha, int now);

typedef void (*DdCharSetup)(void *setup);
void func_08814B4C(void *);
void func_0885DF50(void *);
void func_0885B188(void *);
void func_088148B0(void *);
void dd_chara_setup_06b(void *);
void dd_chara_setup_06a(void *);
void func_0885C900(void *);
void func_0885D6CC(void *);
void func_0885C0F8(void *);
void func_0885AC28(void *);
void func_0885AED8(void *);
void func_0885BD80(void *);
void func_0885E4B0(void *);
void func_0885ECB4(void *);
void func_088676D8(void *);
void func_0885E744(void *);
void func_0885EF5C(void *);
void func_0885B9FC(void *);
void func_0885B6E8(void *);
void func_08860F88(void *);
void func_0885CBB0(void *);
void func_0885B438(void *);
void func_0885CE58(void *);
void func_0885D97C(void *);
void func_0885D41C(void *);
void func_0885C650(void *);
void func_0885C3A0(void *);
void func_0885F4BC(void *);
void func_0885D108(void *);
void func_0885DCA0(void *);
void func_0885EA04(void *);
void func_0885A1C8(void *);
void func_0885E200(void *);
void func_0885F20C(void *);
void func_08859F18(void *);
void func_08860CD8(void *);
void func_08867980(void *);
void func_08867C30(void *);
void func_0885F980(void *);
void func_0885A478(void *);

extern DdCharMgr dd_chara_mgr;

DdCharSetup dd_chara_setup_fn(int chr, int variant);
int dd_chara_redraw(int arg);

char *dd_kao_ptrs_name = "_kao-ptrs.txt";

void dd_chara_init(void)
{
    memset(&dd_chara_mgr, 0, sizeof(dd_chara_mgr));
    dd_chara_mgr.cur_idx = -1;
    func_088587EC();
    func_08865060();
}

void dd_chara_term(void)
{
    DdCharMgr *m = &dd_chara_mgr;
    int i;
    DdCharMgr *p;

    func_088650E4();
    func_08858800();
    for (i = 0, p = m; i < 4; i++, p = (DdCharMgr *)((char *)p + sizeof(DdCharSlot)))
        func_08813840(p->slot, 1);
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
    list = func_088138B8(list, dd_chara_mgr.cur);
    mtx_get_proj(&fovy, &aspect, &near, &far);
    far = 10000.0f;
    mtx_set_mode(0);
    mtx_identity();
    mtx_perspective(fovy, aspect, near, far);
    list = mtx_flush_alias(list);
    func_088276BC();
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
    func_088138F8(dd_chara_mgr.cur);
}

int dd_chara_update2(void)
{
    func_08813974(dd_chara_mgr.cur);
    return 0;
}

/* Loads character `chr` into slot `i`. */
void dd_chara_load(int i, int chr, int variant)
{
    DdCharSlot *s = &dd_chara_mgr.slot[i];
    DdCharSetup fn;
    char setup[52];

    fn = dd_chara_setup_fn(chr, variant);
    func_088282E4(chr, variant, setup);
    if (fn != 0)
        fn(setup);
    func_08813554(s, setup);
    func_08813684(s);
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
    func_08813AB8(m->cur, arg);
    func_0886529C(m->cur);
    return 1;
}

void dd_chara_anim(int a, int b)
{
    func_088652BC(dd_chara_mgr.cur, a, b);
}

int dd_chara_ready(void)
{
    DdCharMgr *m = &dd_chara_mgr;

    if (m->busy != 0)
        return 1;
    return func_08813F1C(m->cur);
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
    func_08813D48(dd_chara_mgr.cur, out, bone);
}

void dd_chara_bone_mtx(void *out, int bone)
{
    func_08813D8C(dd_chara_mgr.cur, out, bone);
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
    func_0886533C(dd_chara_mgr.cur);
}

int dd_chara_busy(void)
{
    return dd_chara_mgr.busy;
}

void dd_chara_set_08813FB4(int arg)
{
    func_08813FB4(dd_chara_mgr.cur, arg);
}

int dd_chara_get_08813FBC(void)
{
    return func_08813FBC(dd_chara_mgr.cur);
}

/* Face entry of slot `i`: from _kao-ptrs.txt in the character's pack, by variant for the two
   player characters when sysvar a98 is set. */
int dd_chara_kao(int i)
{
    unsigned int chr = dd_chara_mgr.slot[i].chr;
    int variant = dd_chara_mgr.slot[i].variant;
    void *pack;
    int *t;

    pack = func_0880507C(chr, variant);
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
    case 2: fn = func_08814B4C; break;
    case 3: fn = func_0885DF50; break;
    case 4: fn = func_0885B188; break;
    case 5: fn = func_088148B0; break;
    case 6:
        if (variant == 0)
            fn = dd_chara_setup_06a;
        else
            fn = dd_chara_setup_06b;
        break;
    case 7: fn = func_0885C900; break;
    case 8: fn = func_0885D6CC; break;
    case 9: fn = func_0885C0F8; break;
    case 10: fn = func_0885AC28; break;
    case 11: fn = func_0885AED8; break;
    case 12: fn = func_0885BD80; break;
    case 13: fn = func_0885E4B0; break;
    case 14: fn = func_0885ECB4; break;
    case 15: fn = func_088676D8; break;
    case 16: fn = func_0885E744; break;
    case 17: fn = func_0885EF5C; break;
    case 18:
        if (variant == 0)
            fn = func_0885B9FC;
        else
            fn = func_0885B6E8;
        break;
    case 19: fn = func_08860F88; break;
    case 20: fn = func_0885CBB0; break;
    case 21: fn = func_0885B438; break;
    case 22: fn = func_0885CE58; break;
    case 23: fn = func_0885D97C; break;
    case 24: fn = func_0885D41C; break;
    case 25: fn = func_0885C650; break;
    case 26: fn = func_0885C3A0; break;
    case 27: fn = func_0885F4BC; break;
    case 28: fn = func_0885D108; break;
    case 29: fn = 0; break;
    case 30: fn = func_0885DCA0; break;
    case 31: fn = func_0885EA04; break;
    case 32: fn = func_0885A1C8; break;
    case 33: fn = func_0885E200; break;
    case 34: fn = 0; break;
    case 35: fn = 0; break;
    case 36: fn = func_0885F20C; break;
    case 37: fn = func_08859F18; break;
    case 38: fn = func_08860CD8; break;
    case 39: fn = 0; break;
    case 40: fn = func_08867980; break;
    }
    if (fn == 0) {
        DdCharInfo info = { 0 };

        code = charmap_lookup(chr, 0);
        charmap_lookup2(code, &info);
        if (code == 1000)
            fn = func_08867C30;
        else if (info.kind < 3)
            fn = func_0885F980;
        else if (info.kind == 3)
            fn = func_0885A478;
        else
            fn = func_0885F980;
    }
    return fn;
}
