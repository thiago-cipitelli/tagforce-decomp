// @range 0x08806A18-0x088075F4
// @asm 0x08806B20-0x08806CE4
// @data 0x0881BD98-0x0881C02C
/* Story script interpreter: the scripts are arrays of 10-byte instructions (StoryCmd); vm_update
   starts the queued script or reports what stopped the last one, vm_run executes instructions until
   one of them has to wait for the next frame. */
#include "libstory.h"

void st_chr_clear_ext_all(void);
void st_sv_set_14(int);
void st_sv_set_alt(int);
void st_sv_set_0c(int);
void st_sv_set_crowd(int);
void st_sv_set_chapter(int);
void st_sv_set_08(int);
int st_sv_alt(void);
int st_sv_get_08(void);
void st_flag666c_set(int);
void st_free_state1_set(int);
int st_free_state0(void);
int st_free_state1(void);
void st_ev2_set_0(int);
void st_msg_unload(int);
void stalk_bu_close_all(void);
void vm_reset(void);
void vm_stop_all(void);
void vm_start(int id, StoryCmd *script);
int vm_run(void);

void st_init_806a18(void)
{
    memset(&story_vm, 0, sizeof(StoryVm));
    vm_reset();
}

void st_term_806a48(void)
{
    memset(&story_vm, 0, sizeof(StoryVm));
}

void vm_set_slot(int *slot)
{
    story_vm_slot = slot;
}

void vm_clear_slot(void)
{
    memset(story_vm_slot, 0, 4);
}

void vm_reset(void)
{
    story_vm.script = 0;
    story_vm.pc = 1;
    story_vm.status = 0x6B;
}

void vm_stop_all(void)
{
    st_chr_clear_ext_all();
    st_sv_set_14(-1);
    st_sv_set_alt(0);
    st_sv_set_0c(0);
    st_sv_set_crowd(0);
    st_sv_set_chapter(0);
    story_vm.next = 0;
}

void vm_start(int id, StoryCmd *script)
{
    *story_vm_slot = id;
    story_vm.script = script;
    story_vm.pc = 1;
}

int vm_script_id(void)
{
    return *story_vm_slot;
}

#ifdef NON_MATCHING /* o retorno 0x6B passa por um bloco de salto ao epílogo a mais no mwcc */
int vm_update(void)
{
    StoryVm *vm = &story_vm;
    int *slot = story_vm_slot;
    int status;

    if (story_vm.script != 0)
        return 0x65;
    status = vm->status;
    st_msg_unload(1);
    if (vm->next != 0) {
        st_flag666c_set(0);
        st_free_state1_set(0);
        st_ev2_set_0(0);
        vm_start(*slot, (StoryCmd *)vm->next);
        st_sv_set_alt(1);
        st_sv_set_0c(0);
        st_sv_set_crowd(0);
        st_sv_set_08(0);
        vm->next = 0;
        return 0x6C;
    }
    stalk_bu_close_all();
    switch (status) {
    case 0x66: case 0x67: case 0x68: case 0x69: case 0x6A: case 0x6F:
        break;
    case 0x6B: case 0x6C: case 0x6D: case 0x6E:
    default:
        if (st_sv_alt()) {
            switch (st_sv_get_08()) {
            case 1:
                st_sv_set_08(0);
                return 0x6D;
            case 2:
                vm_reset();
                vm_stop_all();
                return 0x6E;
            }
            return 0x6B;
        } else if (st_free_state0()) {
            switch (st_free_state1()) {
            case 0:
                break;
            case 1: case 2:
                st_flag666c_set(0);
                st_free_state1_set(2);
                return 0x6E;
            }
        }
        break;
    }
    vm_reset();
    vm_stop_all();
    *slot = -1;
    return status;
}
#endif

void vm_step(void)
{
    StoryVm *vm = &story_vm;

    if (story_vm.script != 0 && !vm_run())
        vm->script = 0;
}

int vm_var(int i)
{
    return story_vm.var[i];
}

void op_msg(void);
void op_msg_c353(void);
void op_msg_pair(void);
void op_c356(void);
void op_c357(void);
void op_c358(void);
void op_c359(void);
void op_c35a(void);
void op_nop(void);
void op_choice(void);
void op_random(void);
void op_if_c35e(void);
void op_if_c35f(void);
void op_if_c360(void);
void op_if_c361(void);
void op_if_c362(void);
void op_if_c363(void);
void op_if_c364(void);
void op_if_c365(void);
void op_if_c366(void);
void op_if_c367(void);
void op_if_c368(void);
void op_if_c369(void);
void op_if_c36a(void);
void op_if_c36b(void);
void op_if_c36c(void);
void op_if_c36d(void);
void op_if_c36e(void);
void op_if_c36f(void);
void op_if_c370(void);
void op_if_c371(void);
void op_if_c372(void);
void op_if_c373(void);
void op_if_c374(void);
void op_if_c375(void);
void op_if_c376(void);
void op_if_c377(void);
void op_if_c378(void);
void op_if_c379(void);
void op_if_c37a(void);
void op_if_c37b(void);
void op_if_c37c(void);
void op_if_c37d(void);
void op_if_c37e(void);
void op_item_add(void);
void op_card_del(void);
void op_card_del_x20(void);
void op_card_del_last(void);
void op_affinity_add(void);
void op_affinity_sub(void);
void op_money_get(void);
void op_money_lose(void);
void op_points(void);
void op_cal_flag(void);
void op_cal_flag2(void);
void op_self_flag(void);
void op_chr_flag(void);
void op_self_level_up(void);
void op_chr_level_up(void);
void op_room(void);
void op_card_get(void);
void op_card_seen(void);
void op_deck_name(void);
void op_mail(void);
void op_sysflag2(void);
void op_field_mode(void);
void op_field_4(void);
void op_field_5(void);
void op_field_6(void);
void op_end_6f(void);
void op_field_8(void);
void op_field_9(void);
void op_bgm(void);
void op_fade(void);
void op_chr_show(void);
void op_wait(void);
void op_field_17(void);
void op_field_1d(void);
void op_duel_vs(void);
void op_duel_tag(void);
void op_duel_opp2(void);
void op_duel_partner(void);
void op_duel_watch(void);
void op_duel_opp(void);
void op_duel_args(void);
void op_duel_rule(void);
void op_duel_x11e(void);
void op_duel_params_a(void);
void op_duel_params_b(void);
void op_lesson(void);
void op_lesson_duel(void);
void op_deck_select(void);
void op_chapter(void);
void op_free_6a(void);
void op_free_state1(void);
void op_ev2_clear(void);
void op_chr_name(void);
void op_costumes(void);
void op_cal_flag2_1(void);
void op_sysflag(void);
void op_talk_66(void);
void op_shop(void);
void op_end_67(void);
void op_se(void);
void op_snd_load(void);
void op_bgm_play(void);
void op_bgm_stop(void);
void op_bgm_c3e3(void);
void op_crowd(void);
void op_pl_pos(void);
void op_chr_x68(void);
void op_chr_x6c(void);
void op_nop2(void);
void op_chr_x70(void);
void op_chr_x72(void);
void op_chr_x74(void);
void op_chr_x78(void);
void op_sv_14(void);
void op_chr_pos(void);
void op_chr_dir(void);
void op_chr_x67(void);
void op_chr_x7a(void);
void op_chr_x7a_2(void);
void op_partner_join(void);
void op_partner_leave(void);
void op_sv_08_2(void);

int vm_run(void)
{
    StoryVm *vm = &story_vm;
    unsigned int op;

    if (story_vm.script == 0)
        return 0;
    if (vm->script[vm->pc].op == 0)
        return 0;
    for (;;) {
        switch (op = vm->script[vm->pc].op) {
        case 0xC351: case 0xC352:
            op_msg();
            break;
        case 0xC353:
            op_msg_c353();
            break;
        case 0xC354: case 0xC355:
            op_msg_pair();
            break;
        case 0xC356:
            op_c356();
            break;
        case 0xC357:
            op_c357();
            break;
        case 0xC358:
            op_c358();
            break;
        case 0xC359:
            op_c359();
            break;
        case 0xC35A:
            op_c35a();
            break;
        case 0xC35B:
            op_nop();
            break;
        case 0xC35C:
            op_choice();
            break;
        case 0xC35D:
            op_random();
            break;
        case 0xC35E:
            op_if_c35e();
            break;
        case 0xC35F:
            op_if_c35f();
            break;
        case 0xC360:
            op_if_c360();
            break;
        case 0xC361:
            op_if_c361();
            break;
        case 0xC362:
            op_if_c362();
            break;
        case 0xC363:
            op_if_c363();
            break;
        case 0xC364:
            op_if_c364();
            break;
        case 0xC365:
            op_if_c365();
            break;
        case 0xC366:
            op_if_c366();
            break;
        case 0xC367:
            op_if_c367();
            break;
        case 0xC368:
            op_if_c368();
            break;
        case 0xC369:
            op_if_c369();
            break;
        case 0xC36A:
            op_if_c36a();
            break;
        case 0xC36B:
            op_if_c36b();
            break;
        case 0xC36C:
            op_if_c36c();
            break;
        case 0xC36D:
            op_if_c36d();
            break;
        case 0xC36E:
            op_if_c36e();
            break;
        case 0xC36F:
            op_if_c36f();
            break;
        case 0xC370:
            op_if_c370();
            break;
        case 0xC371:
            op_if_c371();
            break;
        case 0xC372:
            op_if_c372();
            break;
        case 0xC373:
            op_if_c373();
            break;
        case 0xC374:
            op_if_c374();
            break;
        case 0xC375:
            op_if_c375();
            break;
        case 0xC376:
            op_if_c376();
            break;
        case 0xC377:
            op_if_c377();
            break;
        case 0xC378:
            op_if_c378();
            break;
        case 0xC379:
            op_if_c379();
            break;
        case 0xC37A:
            op_if_c37a();
            break;
        case 0xC37B:
            op_if_c37b();
            break;
        case 0xC37C:
            op_if_c37c();
            break;
        case 0xC37D:
            op_if_c37d();
            break;
        case 0xC37E:
            op_if_c37e();
            break;
        case 0xC37F: case 0xC380: case 0xC381: case 0xC382: case 0xC383: case 0xC384: case 0xC385: case 0xC386: case 0xC387: case 0xC388: case 0xC389: case 0xC38A: case 0xC38B: case 0xC38C: case 0xC38D: case 0xC38E: case 0xC38F: case 0xC390: case 0xC391: case 0xC392: case 0xC393: case 0xC394: case 0xC395: case 0xC396: case 0xC397: case 0xC398: case 0xC399: case 0xC39A: case 0xC39B: case 0xC39C: case 0xC39D: case 0xC39E: case 0xC39F:
            break;
        case 0xC3A0:
            op_item_add();
            break;
        case 0xC3A1:
            op_card_del();
            break;
        case 0xC3A2:
            op_card_del_x20();
            break;
        case 0xC3A3:
            op_card_del_last();
            break;
        case 0xC3A4:
            op_affinity_add();
            break;
        case 0xC3A5:
            op_affinity_sub();
            break;
        case 0xC3A6:
            op_money_get();
            break;
        case 0xC3A7:
            op_money_lose();
            break;
        case 0xC3A8: case 0xC3A9:
            op_points();
            break;
        case 0xC3AA:
            op_cal_flag();
            break;
        case 0xC3AB:
            op_cal_flag2();
            break;
        case 0xC3AC:
            op_self_flag();
            break;
        case 0xC3AD:
            op_chr_flag();
            break;
        case 0xC3AE:
            op_self_level_up();
            break;
        case 0xC3AF:
            op_chr_level_up();
            break;
        case 0xC3B0:
            op_room();
            break;
        case 0xC3B1:
            op_card_get();
            break;
        case 0xC3B2:
            op_card_seen();
            break;
        case 0xC3B3:
            op_deck_name();
            break;
        case 0xC3B4:
            op_mail();
            break;
        case 0xC3B5:
            op_sysflag2();
            break;
        case 0xC3B6:
            op_field_mode();
            break;
        case 0xC3B7:
            op_field_4();
            break;
        case 0xC3B8:
            op_field_5();
            break;
        case 0xC3B9:
            op_field_6();
            break;
        case 0xC3BA:
            op_end_6f();
            break;
        case 0xC3BB:
            op_field_8();
            break;
        case 0xC3BC:
            op_field_9();
            break;
        case 0xC3BD:
            op_bgm();
            break;
        case 0xC3BE: case 0xC3BF:
            op_fade();
            break;
        case 0xC3C0: case 0xC3C1:
            op_chr_show();
            break;
        case 0xC3C2:
            op_wait();
            break;
        case 0xC3C3:
            op_field_17();
            break;
        case 0xC3C4:
            op_field_1d();
            break;
        case 0xC3C5:
            op_duel_vs();
            break;
        case 0xC3C6:
            op_duel_tag();
            break;
        case 0xC3C7:
            op_duel_opp2();
            break;
        case 0xC3C8:
            op_duel_partner();
            break;
        case 0xC3C9:
            op_duel_watch();
            break;
        case 0xC3CA:
            op_duel_opp();
            break;
        case 0xC3CB:
            op_duel_args();
            break;
        case 0xC3CC:
            op_duel_rule();
            break;
        case 0xC3CD:
            op_duel_x11e();
            break;
        case 0xC3CE:
            op_duel_params_a();
            break;
        case 0xC3CF:
            op_duel_params_b();
            break;
        case 0xC3D0:
            op_lesson();
            break;
        case 0xC3D1:
            op_lesson_duel();
            break;
        case 0xC3D2:
            op_deck_select();
            break;
        case 0xC3D3:
            op_chapter();
            return 0;
        case 0xC3D4:
            op_free_6a();
            break;
        case 0xC3D5:
            op_free_state1();
            break;
        case 0xC3D6:
            op_ev2_clear();
            break;
        case 0xC3D7:
            op_chr_name();
            break;
        case 0xC3D8:
            op_costumes();
            break;
        case 0xC3D9:
            return 0;
        case 0xC3DA:
            op_cal_flag2_1();
            break;
        case 0xC3DB:
            op_sysflag();
            break;
        case 0xC3DC:
            op_talk_66();
            break;
        case 0xC3DD:
            op_shop();
            break;
        case 0xC3DE:
            op_end_67();
            break;
        case 0xC3DF:
            op_se();
            break;
        case 0xC3E0:
            op_snd_load();
            break;
        case 0xC3E1:
            op_bgm_play();
            break;
        case 0xC3E2:
            op_bgm_stop();
            break;
        case 0xC3E3:
            op_bgm_c3e3();
            break;
        case 0xC3E4:
            op_crowd();
            break;
        case 0xC3E5:
            op_pl_pos();
            break;
        case 0xC3E6:
            op_chr_x68();
            break;
        case 0xC3E7:
            op_chr_x6c();
            break;
        case 0xC3E8:
            op_nop2();
            break;
        case 0xC3E9:
            op_chr_x70();
            break;
        case 0xC3EA:
            op_chr_x72();
            break;
        case 0xC3EB:
            op_chr_x74();
            break;
        case 0xC3EC:
            op_chr_x78();
            break;
        case 0xC3ED:
            op_sv_14();
            break;
        case 0xC3EE:
            op_chr_pos();
            break;
        case 0xC3EF:
            op_chr_dir();
            break;
        case 0xC3F0:
            op_chr_x67();
            break;
        case 0xC3F1:
            op_chr_x7a();
            break;
        case 0xC3F2:
            op_chr_x7a_2();
            break;
        case 0xC3F3:
            op_partner_join();
            break;
        case 0xC3F4:
            op_partner_leave();
            break;
        case 0xC3F5:
            op_sv_08_2();
            break;
        default:
            return op != 0;
        }
        vm->pc++;
    }
}
