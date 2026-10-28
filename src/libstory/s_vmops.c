// @range 0x08808E68-0x0880B3E4
// @asm 0x088097F4-0x088098DC
// @asm 0x0880A13C-0x0880A248
// @asm 0x0880AEF4-0x0880B048
// @data 0x0881C0E0-0x0881C16C
/* Script instructions 0xC3A0..0xC3F5 (inventory, money, flags, field commands, duel setup, sound,
   characters on the map, partner) and the helpers of the interpreter: block skipping, comparisons
   and the text lookup of the running script. */
#include "libstory.h"

#define CMD (story_vm.script[story_vm.pc])

int vm_is_open(unsigned int op, int a);
int vm_is_close(unsigned int op, int a);
int st_chr_place(int chr);
int st_sv_alt(void);
int st_ev2_get_0(void);
int st_chr_is_away(int i);
void st_fld_setup(int kind, int arg);
void st_se_play(int id);
void st_snd_load(int id);
void deck_slot_add_card(int i, unsigned int id);
void deck_name_set(int i, int c);
void deck_slot_set_level(int i, int n);
int st_free_state0(void);
int st_pl_room_get(void);
int st_pl_x150_get(void);
void st_pl_pos_set(float x, float z, int dir);
void scal_flag2_set(unsigned int id, int on);
int st_chr_model(int chr);
int st_morning_text_b(void);
int st_morning_text_a(void);
void st_costume_add(int *list, int n);
void st_bgm_play(int id);
void st_bgm_stop(void);
void st_bgm_nop(void);
void op_item_add(void)
{
    StoryVm *const vm = &story_vm;

    switch (CMD.b) {
    case 1:
        st_pl_item_inc(vm->script[vm->pc].a);
        break;
    case 2:
        st_pl_item_dec(vm->script[vm->pc].a);
        break;
    }
}

void op_card_del(void)
{
    StoryVm *const vm = &story_vm;

    switch (CMD.b) {
    case 1:
        break;
    case 2:
        st_pl_card_dec(vm->script[vm->pc].a);
        break;
    }
}

void op_card_del_x20(void)
{
    int id = st_pl_x20_get();

    if (id > 0 && id < 0x33)
        st_pl_card_dec(id);
}

void op_card_del_last(void)
{
    int id = st_pl_card_last_get();

    if (id > 0 && id < 0x33)
        st_pl_card_dec(id);
}

void op_affinity_add(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_affinity_add(chr, CMD.b);
}

void op_affinity_sub(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_affinity_add(chr, -CMD.b);
}

void op_money_get(void)
{
    int n = CMD.a;
    char res[4];

    if (fld_cmd_start(0x1A, &n)) {
        while (!fld_cmd_poll(0x1A, res))
            sys_frame_wait();
    }
}

void op_money_lose(void)
{
    int n = -CMD.a;
    char res[4];

    if (fld_cmd_start(0x1A, &n)) {
        while (!fld_cmd_poll(0x1A, res))
            sys_frame_wait();
    }
}

void op_points(void)
{
    StoryVm *const vm = &story_vm;
    int n;
    int v = CMD.a;
    char res[4];

    n = v;
    if (fld_cmd_start(0x1B, &n)) {
        while (!fld_cmd_poll(0x1B, res))
            sys_frame_wait();
        switch (vm->script[vm->pc].op) {
        case 0xC3A8:
            st_pl_points_add(v);
            break;
        case 0xC3A9:
            st_pl_points_add(-v);
            break;
        }
    }
}

void op_cal_flag(void)
{
    StoryVm *const vm = &story_vm;
    int on = 0;

    switch (CMD.b) {
    case 1:
        on = 1;
        break;
    case 2:
        on = 0;
        break;
    }
    scal_flag_set(vm->script[vm->pc].a, on);
}

void op_cal_flag2(void)
{
    op_cal_flag();
}

void op_self_flag(void)
{
    StoryVm *const vm = &story_vm;
    int *slot = story_vm_slot;
    int on = 0;

    switch (CMD.b) {
    case 1:
        on = 1;
        break;
    case 2:
        on = 0;
        break;
    }
    st_chr_flag_set(*slot, vm->script[vm->pc].a, on);
}

void op_chr_flag(void)
{
    StoryVm *const vm = &story_vm;
    int on = 0;
    int chr;

    switch (CMD.c) {
    case 1:
        on = 1;
        break;
    case 2:
        on = 0;
        break;
    }
    chr = st_id_to_chr(vm->script[vm->pc].a);
    st_chr_flag_set(chr, vm->script[vm->pc].b, on);
}

void op_self_level_up(void)
{
    st_chr_level_up(*story_vm_slot);
}

void op_chr_level_up(void)
{
    st_chr_level_up(st_id_to_chr(CMD.a));
}

void op_room(void)
{
    st_pl_room_set(CMD.a);
}

void op_card_get(void)
{
    StoryVm *const vm = &story_vm;
    int id = CMD.a;
    int dir;
    char res[4];

    if (fld_cmd_start(0x19, &id)) {
        while (!fld_cmd_poll(0x19, res))
            sys_frame_wait();
        switch (vm->script[vm->pc].b) {
        case 1:
            dir = 1;
            break;
        case 2:
            dir = 0;
            break;
        }
        inv_adjust_count(vm->script[vm->pc].a, dir);
    }
}

void op_card_seen(void)
{
    inv_set_bit3(CMD.a);
}

void op_deck_name(void)
{
    int rec = st_id_to_rec(CMD.a);
    int c = CMD.b;

    deck_slot_add_card(rec, c);
    deck_name_set(c, rec);
}

void op_mail(void)
{
    st_mail_receive(CMD.a);
}

void op_sysflag2(void)
{
    sysflag_set2(CMD.a, CMD.b);
}

void op_field_mode(void)
{
    if (CMD.a == 0)
        st_fld_setup(2, 0);
    else
        st_fld_setup(3, CMD.a);
}

void op_field_4(void)
{
    st_fld_setup(4, CMD.a);
}

void op_field_5(void)
{
    st_fld_setup(5, 0);
}

void op_field_6(void)
{
    st_fld_setup(6, CMD.a);
}

void op_end_6f(void)
{
    story_vm.status = 0x6F;
}

void op_field_8(void)
{
    st_fld_setup(8, 0);
}

void op_field_9(void)
{
    st_fld_setup(9, CMD.a);
}

#ifdef NON_MATCHING /* o id lido do script sai com bnezl e o store no delay slot no original */
void op_bgm(void)
{
    StoryVm *const vm = &story_vm;
    int id;
    char res[4];

    switch (CMD.b) {
    case 1:
        if (vm->script[vm->pc].a != 0)
            id = vm->script[vm->pc].a;
        else
            id = -2;
        break;
    case 2:
        id = -1;
        break;
    }
    if (fld_cmd_start(0x15, &id)) {
        while (!fld_cmd_poll(0x15, res))
            sys_frame_wait();
    }
}
#endif

void op_fade(int mode)
{
    StoryVm *const vm = &story_vm;

    switch (CMD.op) {
    case 0xC3BE:
        mode = 1;
        break;
    case 0xC3BF:
        mode = 0;
        break;
    }
    st_fade(mode, 1, vm->script[vm->pc].a);
}

void op_chr_show(void)
{
    struct {
        int rec;
        int x, y;
        int visible;
        int flag, dir;
        int text_a, text_b;
    } arg;
    int chr = st_id_to_chr(CMD.a);
    int rec = st_id_to_rec(CMD.a);
    char res[4];

    switch (CMD.op) {
    case 0xC3C0:
        if (!st_chr_is_away(chr))
            return;
        if (st_chr_place(chr) != st_pl_room_get()) {
            st_chr_away_set(chr, 0);
            return;
        }
        arg.rec = rec;
        arg.x = st_chr_x(chr);
        arg.y = st_chr_y(chr);
        arg.visible = 1;
        arg.flag = st_chr_flag_a(chr);
        arg.dir = st_chr_dir(chr);
        arg.text_a = st_chr_text_a(chr);
        arg.text_b = st_chr_text_b(chr);
        st_chr_away_set(chr, 0);
        break;
    case 0xC3C1:
        if (st_chr_is_away(chr))
            return;
        if (st_chr_place(chr) != st_pl_room_get()) {
            st_chr_away_set(chr, 1);
            return;
        }
        arg.rec = rec;
        arg.x = -1;
        arg.y = -1;
        arg.visible = 0;
        arg.flag = -1;
        arg.dir = -1;
        arg.text_a = 0;
        arg.text_b = 0;
        st_chr_away_set(chr, 1);
        break;
    }
    if (fld_cmd_start(9, &arg)) {
        while (!fld_cmd_poll(9, res))
            sys_frame_wait();
    }
}

void op_wait(int unused, int n)
{
    struct {
        int frames;
        int mode;
    } arg;
    char res[4];

    arg.frames = CMD.a;
    switch (CMD.b) {
    case 1:
        n = 0;
        break;
    case 2:
        n = 1;
        break;
    case 3:
        n = 2;
        break;
    case 4:
        n = 3;
        break;
    }
    arg.mode = n;
    if (fld_cmd_start(0x16, &arg) && arg.frames != 0) {
        while (!fld_cmd_poll(0x16, res))
            sys_frame_wait();
    }
}

void op_field_17(void)
{
    char arg[4];
    char res[4];

    if (fld_cmd_start(0x17, arg)) {
        while (!fld_cmd_poll(0x17, res))
            sys_frame_wait();
    }
}

void op_field_1d(void)
{
    int id = st_remap_8126ac(CMD.a);
    char res[4];

    if (fld_cmd_start(0x1D, &id)) {
        while (!fld_cmd_poll(0x1D, res))
            sys_frame_wait();
    }
}

void op_duel_vs(void)
{
    st_lesson_set_a(*story_vm_slot);
    st_duel_set_vs(CMD.a, CMD.b);
    st_duel_set_params_b(-1, -1, -1);
}

void op_duel_tag(void)
{
    st_lesson_set_a(*story_vm_slot);
    st_duel_set_tag(CMD.a, CMD.b);
    st_duel_set_params_b(-1, -1, -1);
}

void op_duel_opp2(void)
{
    st_duel_set_opp2(CMD.a, CMD.b);
}

void op_duel_partner(void)
{
    st_duel_set_partner(CMD.a, CMD.b);
}

void op_duel_watch(void)
{
    st_lesson_set_a(*story_vm_slot);
    st_duel_set_watch(CMD.a, CMD.b);
    st_duel_set_params_b(-1, -1, -1);
}

void op_duel_opp(void)
{
    st_lesson_set_a(*story_vm_slot);
    st_duel_set_opp(CMD.a, CMD.b);
    st_duel_set_params_b(-1, -1, -1);
}

void op_duel_args(void)
{
    st_duel_set_args(CMD.a, CMD.b);
}

void op_duel_rule(void)
{
    st_duel_set_rule(CMD.a, CMD.b, CMD.c);
}

void op_duel_x11e(void)
{
    st_duel_set_x11e(CMD.a, CMD.b);
}

void op_duel_params_a(void)
{
    st_duel_set_params_a(CMD.a, CMD.b, CMD.c);
}

void op_duel_params_b(void)
{
    st_duel_set_params_b(CMD.a, CMD.b, CMD.c);
}

void op_lesson(void)
{
    story_vm.status = 0x68;
    st_lesson_set_a(*story_vm_slot);
    st_lesson_set_b(CMD.b);
    st_free_state2_set(CMD.a);
    sysstate_max_byte_97d0(st_remap_812628(CMD.b), 2);
}

void op_lesson_duel(void)
{
    story_vm.status = 0x69;
    st_lesson_set_a(*story_vm_slot);
    st_duel_set_params_a(CMD.a, CMD.b, 0);
    st_lesson_set_b(CMD.c);
}

#ifdef NON_MATCHING /* as constantes das comparações viram o modo; nenhuma ordem de case/if reproduz os blocos mortos */
void op_deck_select(void)
{
    StoryVm *const vm = &story_vm;
    int deck;
    int mode;
    int m;

    if (CMD.a == 0)
        deck = st_id_to_item(st_pl_x14_get());
    else
        deck = st_id_to_rec(vm->script[vm->pc].a);
    m = story_mode_get();
    if (m == 3)
        mode = 3;
    else if (m == 2)
        mode = 2;
    else if (m == 1)
        mode = 1;
    else
        mode = 0;
    deck_slot_set_level(deck, mode);
    m = story_mode_get();
    if (m == 3)
        vm->var[1] = 3;
    else if (m == 2)
        vm->var[1] = 2;
    else
        vm->var[1] = 1;
}
#endif

void op_chapter(void)
{
    st_sv_set_chapter(CMD.a - 1);
    st_msg_unload(2);
    st_msg_load_iso(st_sv_chapter());
    story_vm.next = st_iso_find_es(1);
}

void op_free_6a(void)
{
    story_vm.status = 0x6A;
    st_free_state2_set(CMD.a);
}

void op_free_state1(void)
{
    st_free_state1_set(1);
}

void op_ev2_clear(void)
{
    st_ev2_set_0(0);
}

void op_chr_name(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_named_set(chr);
    st_chr_name_real(chr);
}

void op_costumes(void)
{
    StoryVm *const vm = &story_vm;
    int n = 0;
    int list[9];
    int *p;
    int c;

    memset(list, 0, sizeof(list));
    p = list;
    for (;;) {
        if (vm->script[vm->pc].op != 0xC3D8 || !(n < 10))
            break;
        c = st_chr_costume(st_id_to_chr(vm->script[vm->pc].a));
        if (c > 0) {
            *p++ = c;
            vm->pc++;
            n++;
        }
    }
    vm->pc--;
    if (n != 0)
        st_costume_add(list, n);
}

void op_cal_flag2_1(void)
{
    scal_flag2_set(1, 1);
}

void op_sysflag(void)
{
    switch (CMD.a) {
    case 0xF:
        sysflag_set(6);
        break;
    case 0x17:
        sysflag_set(5);
        break;
    }
}

void op_talk_66(void)
{
    story_vm.status = 0x66;
    story_vm.var[0] = st_id_to_chr(CMD.a);
}

void op_shop(void)
{
    StoryVm *const vm = &story_vm;
    struct {
        int first;
        int rec;
    } arg;
    int res;

    if (st_pl_x150_get() != 0)
        arg.first = 0;
    else
        arg.first = 1;
    arg.rec = st_id_to_rec(vm->script[vm->pc].a);
    if (fld_cmd_start(0x1C, &arg)) {
        while (!fld_cmd_poll(0x1C, &res))
            sys_frame_wait();
        switch (res) {
        case 0x32:
            st_pl_x150_set(1);
            sysflag_set(1);
            break;
        }
    }
}

void op_end_67(void)
{
    story_vm.status = 0x67;
}

void op_se(void)
{
    st_se_play(st_remap_812498(CMD.a));
}

void op_snd_load(void)
{
    st_snd_load(st_remap_8124b0(CMD.a));
}

void op_bgm_play(void)
{
    st_bgm_play(st_remap_8124b0(CMD.a));
}

void op_bgm_stop(void)
{
    st_bgm_stop();
}

void op_bgm_c3e3(void)
{
    st_bgm_nop();
}

void op_crowd(void)
{
    st_sv_set_08(1);
    st_sv_set_0c(CMD.a);
    st_sv_set_crowd(CMD.b);
    st_chr_clear_ext_all();
    st_sv_set_14(-1);
}

void op_pl_pos(void)
{
    int dir = chr_attr_remap(CMD.a);

    st_pl_pos_set(0.25f + 0.5f * (int)CMD.b, 0.25f + 0.5f * (int)CMD.c, dir);
    scal_flag2_set(3, 1);
}

void op_chr_x68(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x68_set(chr, CMD.b);
}

void op_chr_x6c(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x6c_set(chr, CMD.b);
}

void op_nop2(void)
{
}

void op_chr_x70(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x70_set(chr, CMD.b);
}

void op_chr_x72(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x72_set(chr, CMD.b);
}

void op_chr_x74(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x74_set(chr, CMD.b, CMD.c);
}

void op_chr_x78(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x78_set(chr, CMD.b);
}

void op_sv_14(void)
{
    st_sv_set_14(st_remap_8124b0(CMD.a));
}

void op_chr_pos(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x5c_set(chr, CMD.b, CMD.c, st_sv_get_0c());
    st_chr_set_pos_b(chr, CMD.b, CMD.c);
}

void op_chr_dir(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x65_set(chr, CMD.b, CMD.c);
    st_chr_set_dir_b(chr, chr_attr_remap(CMD.b));
}

void op_chr_x67(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x67_set(chr, CMD.b);
}

void op_chr_x7a(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x7a_set(chr, CMD.b, CMD.c);
}

void op_chr_x7a_2(void)
{
    int chr = st_id_to_chr(CMD.a);

    st_chr_x7a_set(chr, CMD.b, CMD.c);
}

void op_partner_join(void)
{
    StoryVm *const vm = &story_vm;
    struct {
        int rec;
        int model;
    } arg;
    char res[4];
    int rec = st_id_to_rec(CMD.a);
    int chr = st_id_to_chr(CMD.a);

    arg.rec = rec;
    arg.model = st_chr_model(chr);
    if (st_pl_x14_get() == -1 && fld_cmd_start(0xB, &arg)) {
        while (!fld_cmd_poll(0xB, res))
            sys_frame_wait();
        st_pl_x14_set(st_id_to_chr(vm->script[vm->pc].a));
    }
}

void op_partner_leave(void)
{
    struct {
        int rec;
        int model;
    } arg;
    char res[4];

    arg.rec = -1;
    arg.model = 0;
    if (st_pl_x14_get() != -1 && fld_cmd_start(0xB, &arg)) {
        while (!fld_cmd_poll(0xB, res))
            sys_frame_wait();
        st_pl_x14_set(-1);
    }
}

void op_sv_08_2(void)
{
    st_sv_set_08(2);
}

/* skips forward to the instruction op (with the given arguments, 0 = any) closing the current block;
   nested blocks are counted */
#ifdef NON_MATCHING /* registradores s3/s4 de depth e do argumento a trocados */
int vm_skip(unsigned int op, unsigned int a, unsigned int b, unsigned int c)
{
    unsigned int cop, ca, cb, cc;
    int depth = 0;
    StoryVm *vm = &story_vm;
    StoryCmd *p;

    if (vm_is_close(op, a))
        depth = 1;
    do {
        vm->pc++;
        p = &vm->script[vm->pc];
        cop = p->op;
        cb = p->b;
        cc = p->c;
        ca = p->a;
        if (cop == 0)
            return 0;
        if (vm_is_open(cop, ca))
            depth++;
        else if (vm_is_close(cop, ca))
            depth--;
    } while (cop != op || depth > 0 || (a != 0 && ca != a && ca != 0) || (b != 0 && cb != b && cb != 0)
             || (c != 0 && cc != c && cc != 0));
    return 1;
}
#endif

int vm_cmp(unsigned int v, unsigned int ref, int mode)
{
    int r = 0;

    switch (mode) {
    case 1:
        r = v == ref;
        break;
    case 2:
        r = v != ref;
        break;
    case 3:
        r = !(v < ref);
        break;
    case 4:
        r = v < ref;
        break;
    }
    return r;
}

int vm_test(int v, int mode)
{
    int r = 0;

    switch (mode) {
    case 1:
        r = v != 0;
        break;
    case 2:
        r = !v;
        break;
    }
    return r;
}

int vm_test2(int v, int mode)
{
    int r = 0;

    switch (mode) {
    case 1:
        r = v != 0;
        break;
    case 2:
        r = !v;
        break;
    }
    return r;
}

/* instruction that opens a block (if/choice/random) */
int vm_is_open(unsigned int op, int a)
{
    int r;

    switch (op) {
    case 0xC35E: case 0xC35F: case 0xC360: case 0xC361: case 0xC362: case 0xC363: case 0xC364:
    case 0xC365: case 0xC366: case 0xC367: case 0xC368: case 0xC369: case 0xC36A: case 0xC36B:
    case 0xC36C: case 0xC36D: case 0xC36E: case 0xC36F: case 0xC370: case 0xC371: case 0xC372:
    case 0xC373: case 0xC374: case 0xC375: case 0xC376: case 0xC377: case 0xC378: case 0xC379:
    case 0xC37A: case 0xC37B: case 0xC37C: case 0xC37D: case 0xC37E:
        r = 1;
        break;
    case 0xC35C: case 0xC35D:
        r = a == 1;
        break;
    default:
        r = 0;
        break;
    }
    return r;
}

/* instruction that closes a block */
int vm_is_close(unsigned int op, int a)
{
    int r;

    switch (op) {
    case 0xC37F: case 0xC380: case 0xC381: case 0xC382: case 0xC383: case 0xC384: case 0xC385:
    case 0xC386: case 0xC387: case 0xC388: case 0xC389: case 0xC38A: case 0xC38B: case 0xC38C:
    case 0xC38D: case 0xC38E: case 0xC38F: case 0xC390: case 0xC391: case 0xC392: case 0xC393:
    case 0xC394: case 0xC395: case 0xC396: case 0xC397: case 0xC398: case 0xC399: case 0xC39A:
    case 0xC39B: case 0xC39C: case 0xC39D: case 0xC39E: case 0xC39F:
        r = 1;
        break;
    case 0xC35C: case 0xC35D:
        r = a == 11;
        break;
    default:
        r = 0;
        break;
    }
    return r;
}

/* text id of the running script (message packs of the free mode, the event or the character) */
int vm_text(int id)
{
    int *slot = story_vm_slot;
    int ei, el;

    if (st_free_state0()) {
        ei = st_free_xc();
        el = st_free_x8();
    } else if (st_ev2_get_0()) {
        ei = st_morning_text_b();
        el = st_morning_text_a();
    } else if (st_sv_alt()) {
        ei = st_iso_get_ei();
        el = st_iso_get_el();
    } else {
        ei = st_scr_get_ei(*slot);
        el = st_scr_get_el(*slot);
    }
    return st_text_at(id, ei, el);
}
