// @range 0x08816890-0x088171F4
// @asm 0x08816FC4-0x08817034
// @data 0x0881D15C-0x0881D194
/* Story requests and character lookups: the getters of the character records that pick the field
   or the alternate (school vacation) version of each value, the start of a character event script
   by kind, the story mode, the requests to the field (lesson, exit, night) and the field fade. */
#include "libstory.h"

typedef struct StReq {                 /* 0x08ACBAC4, 0xC */
    int night;                         /* +0x0 */
    int fade;                          /* +0x4 field faded out */
    int mode;                          /* +0x8 story mode 1..4 */
} StReq;

typedef struct StReqSave {             /* *st_req_save, 0x10 */
    int lesson;                        /* +0x0 */
    int exit;                          /* +0x4 */
    int x8;                            /* +0x8 */
    unsigned short xc;                 /* +0xC */
} StReqSave;

extern StReqSave *st_req_save;         /* 0x08ACBAC0 */
extern StReq st_req;

int st_sv_alt(void);
int charmap_lookup(int ch, int *kind_out);
void sysflag_apply(unsigned int which, int v);
int st_id_to_item(int id);
int st_chr_best(void);
void st_chr_x1c_set(int i, int v);
int vm_script_id(void);
void vm_start(int id, StoryCmd *script);
StoryCmd *st_iso_find_es(int id);
StoryCmd *st_scr_find_es(int i, unsigned int key);
int st_duel_param0(void);
StoryCmd *st_free_today(void);
StoryCmd *st_free_entry(int id);
int st_free_state2(void);
StoryCmd *st_ev_lesson_script(void);
StoryCmd *st_ev_script_174ac(int n);
int st_mod_loaded(int id);
int st_chr_attr(int chr);
int st_chr_look(int chr);
int st_chr_pick(int chr);
int st_chr_x04(int chr);
int st_chr_x08(int chr);
int st_chr_x0c(int chr);
int st_chr_x10(int chr);
int st_chr_x14(int chr);
int st_chr_x16(int chr);
int st_chr_x17(int chr);
int st_chr_x24(int chr);
int st_chr_x28(int chr);
int st_chr_x30(int chr);
int st_chr_x32(int chr);
int st_chr_x34(int chr);
int st_chr_x36(int chr);
int st_chr_x38(int chr);
int st_chr_x52(int chr);
int st_chr_x54(int chr);
int st_chr_x58(int chr);
int st_chr_x5c(int chr);
int st_chr_x60(int chr);
int st_chr_x64(int chr);
int st_chr_x65(int chr);
int st_chr_x66(int chr);
int st_chr_x67(int chr);
int st_chr_x68(int chr);
int st_chr_x6c(int chr);
int st_chr_x70(int chr);
int st_chr_x72(int chr);
int st_chr_x74(int chr);
int st_chr_x76(int chr);
int st_chr_x78(int chr);
int st_chr_x7a_rand(int chr);

void st_init_816890(void)
{
    memset(&st_req, 0, sizeof(StReq));
}

void st_term_8168a4(void)
{
    memset(&st_req, 0, sizeof(StReq));
}

void st_req_set_save(StReqSave *p)
{
    st_req_save = p;
}

void st_req_clear_save(void)
{
    memset(st_req_save, 0, sizeof(StReqSave));
}

int st_chr_place(int chr)
{
    if (st_sv_alt())
        return st_chr_x52(chr);
    return st_chr_look(chr);
}

/* sound group of a character: its model, or the BOOT table when it has none */
int st_chr_model(int chr)
{
    int item = st_id_to_item(chr);
    int r;

    if (item > 0x28)
        return charmap_lookup(item, 0);
    if (st_sv_alt()) {
        r = st_chr_x68(chr);
        if (r < 0)
            r = st_chr_x24(chr);
    } else
        r = st_chr_x24(chr);
    if (r < 1)
        r = charmap_lookup(item, 0);
    return r;
}

int st_chr_costume(int chr)
{
    int r;

    if (st_sv_alt()) {
        r = st_chr_x6c(chr);
        if (r < 0)
            r = st_chr_x28(chr);
    } else
        r = st_chr_x28(chr);
    if (r < 1)
        return -1;
    return r;
}

int st_chr_x(int chr)
{
    if (st_sv_alt())
        return st_chr_x5c(chr);
    return st_chr_x0c(chr);
}

int st_chr_y(int chr)
{
    if (st_sv_alt())
        return st_chr_x60(chr);
    return st_chr_x10(chr);
}

int st_chr_x_b(int chr)
{
    if (st_sv_alt())
        return st_chr_x54(chr);
    return st_chr_x04(chr);
}

int st_chr_y_b(int chr)
{
    if (st_sv_alt())
        return st_chr_x58(chr);
    return st_chr_x08(chr);
}

int st_chr_dir(int chr)
{
    if (st_sv_alt())
        return st_chr_x65(chr);
    return st_chr_attr(chr);
}

int st_chr_dir_b(int chr)
{
    if (st_sv_alt())
        return st_chr_x64(chr);
    return st_chr_x14(chr);
}

int st_chr_flag_a(int chr)
{
    if (st_sv_alt())
        return st_chr_x66(chr);
    return st_chr_x16(chr);
}

int st_chr_flag_b(int chr)
{
    if (st_sv_alt())
        return st_chr_x67(chr);
    return st_chr_x17(chr);
}

/* starts the event script of a character; kind picks which event (0 random, 1..5 by slot, 6..8 the
   lesson duel, 9 the best partner, 10..13 fixed scripts) */
void st_event_start(int kind, int chr, int ev)
{
    StoryCmd *script;

    switch (kind) {
    case 0:
    default:
        if (st_sv_alt()) {
            ev = st_chr_x7a_rand(chr);
            script = st_iso_find_es(ev);
        } else {
            ev = st_chr_pick(chr);
            script = st_scr_find_es(chr, ev);
        }
        break;
    case 1:
        if (st_sv_alt()) {
            ev = st_chr_x70(chr);
            if (ev < 0)
                script = 0;
            else
                script = st_iso_find_es(ev);
        } else {
            ev = st_chr_x30(chr);
            script = st_scr_find_es(chr, ev);
        }
        break;
    case 2:
        if (st_sv_alt()) {
            ev = st_chr_x72(chr);
            if (ev < 0)
                script = 0;
            else
                script = st_iso_find_es(ev);
        } else {
            ev = st_chr_x32(chr);
            script = st_scr_find_es(chr, ev);
        }
        break;
    case 3:
        if (st_sv_alt()) {
            ev = st_chr_x78(chr);
            if (ev < 0)
                script = 0;
            else
                script = st_iso_find_es(ev);
        } else {
            ev = st_chr_x38(chr);
            script = st_scr_find_es(chr, ev);
        }
        break;
    case 4:
        if (st_sv_alt()) {
            ev = st_chr_x74(chr);
            if (ev < 0) {
                ev = st_chr_x34(chr);
                script = st_scr_find_es(chr, ev);
            } else
                script = st_iso_find_es(ev);
        } else {
            ev = st_chr_x34(chr);
            script = st_scr_find_es(chr, ev);
        }
        break;
    case 5:
        if (st_sv_alt()) {
            ev = st_chr_x76(chr);
            if (ev < 0) {
                ev = st_chr_x36(chr);
                script = st_scr_find_es(chr, ev);
            } else
                script = st_iso_find_es(ev);
        } else {
            ev = st_chr_x36(chr);
            script = st_scr_find_es(chr, ev);
        }
        break;
    case 6:
    case 7:
    case 8:
        chr = st_duel_param0();
        if (st_sv_alt())
            script = st_iso_find_es(ev);
        else
            script = st_scr_find_es(chr, ev);
        break;
    case 9:
        chr = st_chr_best();
        script = st_ev_lesson_script();
        break;
    case 10:
        chr = 0;
        script = st_ev_script_174ac(1);
        break;
    case 11:
        chr = vm_script_id();
        script = st_iso_find_es(2);
        break;
    case 12:
        chr = 0;
        script = st_free_today();
        break;
    case 13:
        chr = 0;
        if (st_free_state2() <= 0)
            script = st_free_today();
        else
            script = st_free_entry(st_free_state2());
        break;
    }
    if (chr > 0 || ev > 0)
        st_chr_x1c_set(chr, ev);
    vm_start(chr, script);
}

/* story mode 1..4; any other value goes back to mode 1 */
#ifdef NON_MATCHING /* o original não reaproveita o 1 do último teste do switch como argumento de sysflag_apply (li a0,1) */
int story_mode_set(int m)
{
    StReq *s = &st_req;
    int r;

    switch (m) {
    default:
        sysflag_apply(1, 0);
        m = 1;
        r = 0;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        r = 1;
        break;
    }
    s->mode = m;
    return r;
}
#endif

int story_mode_get(void)
{
    return st_req.mode;
}

int story_mode_is1(void)
{
    return st_req.mode == 1;
}

void st_req_set_lesson(int v)
{
    StReqSave *p = st_req_save;

    if (v >= 0 || v < 0x18)
        p->lesson = v;
}

int st_req_lesson(void)
{
    return st_req_save->lesson;
}

void st_req_set_night(int v)
{
    st_req.night = v;
}

int st_req_is_night(void)
{
    return st_req.night != 0;
}

void st_req_set_exit(int v)
{
    st_req_save->exit = v;
}

int st_req_exit(void)
{
    return st_req_save->exit;
}

void st_req_set_c(int v)
{
    st_req_save->xc = v;
}

int st_req_c(void)
{
    return st_req_save->xc;
}

void st_req_set_8(int v)
{
    st_req_save->x8 = v;
}

int st_req_get_8(void)
{
    return st_req_save->x8;
}

/* fades the field out (out != 0) or in over `frames` frames (15 by default), waiting if asked */
void st_fade(int out, int wait, int frames)
{
    StReq *s = &st_req;
    int arg[3];
    int r;

    arg[0] = out;
    if (frames == 0)
        arg[1] = 15;
    else
        arg[1] = frames;
    arg[2] = 0;
    if (s->fade != 0 && out != 0)
        return;
    if (s->fade == 0 && out == 0)
        return;
    if (!st_mod_loaded(0))
        return;
    s->fade = out;
    fld_cmd_start(0x18, arg);
    if (wait) {
        while (!fld_cmd_poll(0x18, &r))
            sys_frame_wait();
    }
}

void st_req_set_x10(int v)
{
    st_req.fade = v;
}
