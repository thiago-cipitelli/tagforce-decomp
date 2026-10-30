// @range 0x0881794C-0x08819A70
// @asm 0x08817C2C-0x08817D88
// @asm 0x08817EF4-0x08817F08
// @asm 0x08817F08-0x088182C8
// @asm 0x088185C0-0x088186AC
// @asm 0x08818AC4-0x08818B2C
// @asm 0x08818B2C-0x08818C34
// @asm 0x08818FB0-0x0881917C
// @asm 0x08819184-0x08819430
// @asm 0x08819430-0x08819684
// @asm 0x0881972C-0x088197C8
// @asm 0x08819920-0x08819968
// @asm 0x088199C8-0x08819A70
/* The duel-event handlers 1..73 of the dispatcher table (dd_event.c): cut-ins (duel start/end,
   phase and turn banners, attack, flip), card moves and stat changes on the field, message windows
   and prompts. The player numbers of the engine are turned into screen sides by dv_player_to_side. */
#include "dd_field_ui.h"

extern int dd_ev_attack[4];            /* 0x08C5479C attacker side, zone, target side, zone */
extern int dd_ev_flip_cutin;           /* 0x08C547AC the next flip plays the cut-in */
extern int dd_ev_player;               /* 0x08C547B0 */

/* other files and ranges (unprototyped: some are called with fewer arguments) */
void *dd_ehf_get();
int dd_game_flag119();
int dd_08806B5C();
int dd_view_x14_set();
int dd_is_tag_duel();
int dd_view_locs_set();
int dd_loc_visible();
int dd_view_x10_set();
int dd_field_set();
int dd_info_select();
int dd_info_select_uid();
int dd_info_show_uid();
int dd_info_hide();
int dd_cobjs_map_get();
int dd_cobj_goto();
int dd_cobj_flip();
int dd_cobj_to_top();
void *dd_cobj_at();
void *dd_cobj_find();
int dd_zone_sync();
int dd_cobj_end_a2();
int dd_cobj_end_a3();
int dd_cobj_end_a4();
int dd_cobj_set_card();
int dd_cobj_spawn();
int dd_cobj_remove();
int dd_cobj_hide();
int dd_cobj_extra_up();
int dd_cobjs_counters();
int dd_zone_anim_update();
int func_0880D028();
int dd_hl_push();
int dd_hl_pop();
int dd_lpp_set();
int dd_lpp_turn_set();
int dd_lpp_phase_set();
int dd_ev_36();
int dd_sel_open_engine();
void *dd_alloc();
int dd_free();
int dd_mode_push();
int dd_mode_cancel();
int dd_mode_set_card();
unsigned int *dd_pad();
int dd_act();
int dd_popup();
int dd_glow_on();
int dd_glow_off();
int dd_glow_all_off();
int dd_beam_summon();
int dd_beam_set2();
int dd_mark_on();
int dd_mark_off();
int dd_mark_all_off();
int dd_destroy();
int dd_move();
int dd_flip();
int dd_flip_now();
int dd_stat_counter();
int dd_counter();
int dd_num_counter();
int dd_lock_cursor();
int func_08820138();
int func_08820344();
int func_08820430();
int func_088204BC();
int func_0882059C();
int func_08820678();
int func_088206D4();
int func_088207A8();
int func_088208B0();
int func_08820F88();
int func_08821D20();
int func_0882214C();
int func_088226C0();
int func_08822838();
int func_0882308C();
int func_08823674();
int func_08823E34();
int func_08824544();
int func_08824CC8();
int func_08824F3C();
int func_088259D8();
int dd_wait_dots_open();
int dd_dice_open();
int dd_chara_set_anim_range();
int dd_mark_show();
int dd_mark_hide();
int dd_mark_move();
int dd_mode_pop();
int dd_chara_busy_clear();
int dd_chara_reset();
int declare_open();
int dv_player_to_side();
int dv_mode_bit();
int dv_phase();
int dv_list_id();
int dv_hand_uid();
int dv_w624_set();
int dv_08835AEC();
int dv_zone_card_id();
int dv_0884C824();
int dv_0884C830();
void *dv_chain();
int dv_unk2c();

/* 1: the duel start animation */
int dd_ev_01(void)
{
    DdCutinDesc d;

    memset(&d, 0, sizeof(d));
    d.ehf = dd_ehf_get(0x20004);
    d.end = -1;
    d.flag = 1;
    d.always = 1;
    d.start = 0;
    d.done = 0;
    d.done_arg = 0;
    dd_cutin_play(&d);
    return 1;
}

/* 2: duel end banner (1 win, 2 lose, 3 draw) with the reason caption */
int dd_ev_02(int kind)
{
    DdCutinDesc d;
    int id;
    int k;

    switch (kind) {
    case 1:
    default:
        id = 0x20005;
        break;
    case 2:
        id = 0x20006;
        break;
    case 3:
        id = 0x20007;
        break;
    }
    memset(&d, 0, sizeof(d));
    d.ehf = dd_ehf_get(id);
    d.end = -1;
    d.loop_from = 0x32;
    d.loop_to = 0x46;
    d.layer = 7;
    d.start = 0;
    d.done = 0;
    d.done_arg = 0;
    d.flag = 2;
    d.always = 1;
    if (kind == 1 || kind == 2) {
        d.text_frame = 0x1E;
        d.text_x = 0x13A;
        d.text_y = 0xB6;
        if (kind == 2) {
            k = dv_unk2c(1);
            if (k == 0xB)
                k = 9;
            else if (k == 8 && dd_game_flag119() != 0)
                k = 1;
            d.text = k + 0x4F;
        } else {
            k = dv_unk2c(0);
            if (k == 0xB)
                k = 9;
            else if (k == 8 && dd_game_flag119() != 0)
                k = 1;
            d.text = k + 0x43;
        }
    }
    dd_cutin_play(&d);
    return 1;
}

/* 3: start of a phase: the deck flashes when a draw is possible, the cursor returns */
int dd_ev_03(void)
{
    int found;
    int player;
    struct {
        void (*get)(DdLoc *out);
    } arg;

    dd_view_x10_set(1);
    if (dv_phase() == 0) {
        found = 0;
        if (dv_08835AEC(0, 0xD, 0) & 0x1000) {
            player = 0;
            found = 1;
        } else if (dv_08835AEC(1, 0xD, 0) & 0x1000) {
            player = 1;
            found = player;
        }
        if (found != 0) {
            if (cfg_get_flag(3) != 0) {
                dd_act(player, 0xD, 0, 0xC);
            } else {
                dd_cursor_set(player, 0xD, 0, 0);
                memset(&arg, 0, sizeof(arg));
                arg.get = dd_cursor_get;
                func_08820F88(&arg);
            }
        }
    }
    dd_cursor_restore();
    dd_cursor_first_target();
    dd_ev_36();
    return 1;
}

/* 4: phase banner */
#ifdef NON_MATCHING /* cópia morta no delay slot do jr da tabela */
int dd_ev_04(int player, int phase)
{
    DdCutinDesc d;

    dd_lpp_turn_set(dv_player_to_side(player), 0);
    dd_lpp_phase_set(phase, 0);
    memset(&d, 0, sizeof(d));
    d.ehf = dd_ehf_get(0x20009);
    d.flag = 4;
    d.done = 0;
    d.done_arg = 0;
    d.se = 0x6231;
    switch (phase) {
    default:
        d.start = 0;
        d.end = -1;
        break;
    case 0:
        d.start = 0;
        d.end = 0x18;
        break;
    case 1:
        d.start = 0x19;
        d.end = 0x31;
        break;
    case 2:
        d.start = 0x32;
        d.end = 0x4A;
        break;
    case 3:
        d.start = 0x4B;
        d.end = 0x63;
        break;
    case 4:
        d.start = 0x64;
        d.end = 0x7C;
        break;
    case 5:
        d.start = 0x7D;
        d.end = 0x95;
        break;
    }
    dd_cutin_play(&d);
    if (phase == 3)
        dd_cobjs_counters(0);
    dd_info_hide(0);
    dd_mark_hide();
    func_088259D8();
    return 1;
}
#endif

/* 5: turn change banner */
int dd_ev_05(int player)
{
    DdCutinDesc d;

    dd_lpp_turn_set(dv_player_to_side(player), 0);
    memset(&d, 0, sizeof(d));
    d.ehf = dd_ehf_get(0x20008);
    d.flag = 5;
    d.se = 0x6230;
    d.done = 0;
    d.done_arg = 0;
    switch (dv_player_to_side(player)) {
    default:
        d.start = 0;
        d.end = -1;
        break;
    case 0:
        d.start = 0x32;
        d.end = 0x62;
        break;
    case 1:
        d.start = 0;
        d.end = 0x30;
        break;
    }
    dd_cutin_play(&d);
    return 1;
}

int dd_ev_06(void)
{
    dd_field_set();
    return 1;
}

/* 7: puts the cursor on a location */
int dd_ev_07(int player, int zone, int idx)
{
    dd_cursor_set(dv_player_to_side(player), zone, idx, 0);
    return 1;
}

int dd_ev_08(void)
{
    return 1;
}

/* 9: the scene's own player changes */
int dd_ev_09(int player)
{
    dd_ev_player = player;
    dd_ev_reset(dv_player_to_side(player));
    dd_mark_hide();
    dd_info_hide(0);
    return 1;
}

int dd_ev_10(void)
{
    return 1;
}

/* end of the attack cut-in: the attack animation itself */
#ifdef NON_MATCHING /* ordem dos lw dos argumentos */
int dd_ev_attack_done(int *a)
{
    return dd_mark_move(a[0], a[1], a[2], a[3]);
}
#endif

/* 11: attack declaration: attacker and target values go to the panels; a direct attack (target
   zone >= 5) plays the attack cut-in first */
#ifdef NON_MATCHING /* locais DdLoc e alocação */
int dd_ev_11(int zone, int tplayer, int tzone)
{
    int me = dd_ev_player;
    int tid = -1;
    DdLoc t = { 0 };
    DdLoc a = { 0 };
    DdLoc l = { 0 };
    DdCutinDesc d;
    DdCard *card;
    int atk, def;
    int aid;

    t.player = dv_player_to_side(tplayer);
    t.zone = tzone;
    a.player = dv_player_to_side(me);
    a.zone = zone;
    dd_view_locs_set(a, t);
    card = dd_cobj_at(dv_player_to_side(me), zone, 0);
    atk = card_get_atk(card->id);
    def = card_get_def(card->id);
    aid = card->uid;
    dd_ev_slot_set(dv_player_to_side(me), card->id, atk, def);
    if (tzone != -1) {
        if (tzone >= 5) {
            dd_ev_slot_set(dv_player_to_side(tplayer), -1, 0, 0);
        } else {
            card = dd_cobj_at(dv_player_to_side(tplayer), tzone, 0);
            atk = card_get_atk(card->id);
            def = card_get_def(card->id);
            tid = card->uid;
            dd_ev_slot_set(dv_player_to_side(tplayer), card->id, atk, def);
        }
    }
    if (tzone < 0) {
        dd_mark_show(dv_player_to_side(me), zone);
    } else if (tzone < 5) {
        dd_mark_move(dv_player_to_side(me), zone, dv_player_to_side(tplayer), tzone);
    } else {
        dd_ev_attack[0] = dv_player_to_side(me);
        dd_ev_attack[1] = zone;
        dd_ev_attack[2] = dv_player_to_side(tplayer);
        dd_ev_attack[3] = tzone;
        memset(&d, 0, sizeof(d));
        d.ehf = dd_ehf_get(0x2000D);
        d.end = -1;
        d.done = (void (*)(int))dd_ev_attack_done;
        d.done_arg = (int)dd_ev_attack;
        d.flag = 0xB;
        d.always = 1;
        d.start = 0;
        if (dd_ev_attack[0] != 0)
            d.y = 100.0f;
        else
            d.y = -100.0f;
        dd_cutin_play(&d);
        req_flag_set(0xB);
    }
    dd_info_show_uid(1 - dv_player_to_side(me), aid, 0);
    l.player = dv_player_to_side(tplayer);
    l.zone = tzone;
    if (tid >= 0 && dd_loc_visible(l) != 0)
        dd_info_select_uid(tid);
    return 1;
}
#endif

/* 12: battle damage step */
int dd_ev_12(void)
{
    dd_zone_anim_update(1);
    func_0882059C(0xC, func_0880D028, 0);
    dd_ev_8817670(0x46);
    dd_mark_hide();
    return 1;
}

int dd_ev_13(int player, int a)
{
    dd_lpp_set(dv_player_to_side(player), a, 0);
    return 1;
}

int dd_ev_14(int player, int a, int b)
{
    if (b == 0)
        func_08820138(dv_player_to_side(player), a);
    return 1;
}

int dd_ev_15(void)
{
    return 1;
}

int dd_ev_16(int player)
{
    dd_destroy(dv_player_to_side(player));
    return 1;
}

/* 17: a card is drawn into the hand */
int dd_ev_17(int player, int a)
{
    DdCard *card;
    int idx;

    idx = dv_hand_uid(dv_player_to_side(player), a);
    card = dd_cobj_find(dv_player_to_side(player), 0xB, idx);
    dd_cobj_hide(card);
    dd_info_select_uid(card->uid);
    return 1;
}

int dd_ev_18(int player, int a)
{
    dd_cobj_extra_up(dv_player_to_side(player), a);
    return 1;
}

int dd_ev_19(int player)
{
    func_08820344(dv_player_to_side(player));
    return 1;
}

int dd_ev_20(int player, int a)
{
    dd_zone_sync(dv_player_to_side(player), a);
    return 1;
}

int dd_ev_21(int player, int a)
{
    dd_cobj_flip(dv_player_to_side(player), a);
    return 1;
}

int dd_ev_22(int player, int a, int b)
{
    dd_cobj_to_top(dv_player_to_side(player), a, b);
    return 1;
}

/* 23: focuses a location and shows its card */
int dd_ev_23(int player, int zone, int idx, int show)
{
    DdCard *card;

    dd_cursor_set(dv_player_to_side(player), zone, idx, 1);
    dd_lock_cursor();
    if (show != 0) {
        card = dd_cobj_at(dv_player_to_side(player), zone, idx);
        if (card != 0)
            dd_info_select_uid(card->uid);
    }
    return 1;
}

/* 24: a card moves between two locations */
#ifdef NON_MATCHING /* bitfields dos DdLoc2 por valor */
int dd_ev_24(int uid, int arg, DdLoc2 from, DdLoc2 to)
{
    int id;

    from.player = dv_player_to_side(from.player);
    to.player = dv_player_to_side(to.player);
    id = dv_list_id(uid);
    if (card_is_attribute9(id) == 0 || (unsigned int)(to.zone - 0xE) > 2)
        dd_cobj_goto(to, from, uid, arg);
    else
        dd_cobj_remove(from, id, uid & 0xFFFF);
    return 1;
}
#endif

/* 25: two cards swap */
int dd_ev_25(int a, DdLoc2 p, DdLoc2 q)
{
    int u1, u2;

    p.player = dv_player_to_side(p.player);
    q.player = dv_player_to_side(q.player);
    u1 = ((DdCard *)dd_cobj_at(p.player, p.zone, 0))->uid;
    u2 = ((DdCard *)dd_cobj_at(q.player, q.zone, 0))->uid;
    dd_cobj_goto(q, p, u1, 0);
    dd_cobj_goto(p, q, u2, 0);
    return 1;
}

int dd_ev_26(DdLoc2 p)
{
    p.player = dv_player_to_side(p.player);
    dd_cobj_goto(p, p, -1, 0);
    return 1;
}

int dd_ev_27(DdLoc2 p, int a, unsigned short b)
{
    DdCard *card;

    p.player = dv_player_to_side(p.player);
    card = dd_cobj_at(p.player, p.zone, p.idx);
    if (card != 0)
        dd_cobj_set_card(card, a, b);
    else
        dd_cobj_spawn(p, a, b);
    return 1;
}

int dd_ev_28(int player, int zone)
{
    DdCard *card = dd_cobj_at(dv_player_to_side(player), zone, 0);

    if (card == 0)
        return 1;
    dd_cobj_end_a2(card);
    return 1;
}

int dd_ev_29(int player, int zone, int a, int idx)
{
    DdCard *card = dd_cobj_find(dv_player_to_side(player), zone, idx);

    if (card == 0)
        return 1;
    dd_flip(card);
    return 1;
}

int dd_ev_30(int player, int zone)
{
    DdCard *card = dd_cobj_at(dv_player_to_side(player), zone, 0);

    if (card == 0)
        return 1;
    dd_flip_now(card);
    return 1;
}

int dd_ev_31(void)
{
    return 1;
}

int dd_ev_32(DdLoc2 p, int a)
{
    dd_popup(dv_player_to_side(p.player), p.zone, a, 0x20, 0);
    return 1;
}

int dd_ev_33(int player, int zone, int idx)
{
    int v = dv_zone_card_id(dv_player_to_side(player), zone, idx);

    dd_popup(dv_player_to_side(player), zone, v, 0x21, 1);
    return 1;
}

int dd_ev_34(void)
{
    return 1;
}

int dd_ev_35(int player, int zone, int a, int b)
{
    DdCard *card;

    if (zone == 0x10)
        card = 0;
    else
        card = dd_cobj_at(dv_player_to_side(player), zone, 0);
    dd_counter(card, a, b);
    return 1;
}

#ifdef NON_MATCHING /* salto de retorno antecipado */
int dd_ev_36(void)
{
    if (dv_phase() != 3)
        return 1;
    dd_zone_anim_update(0);
    func_0882059C(0, func_0880D028, 0);
    if (dv_mode_bit(1) == 0)
        dd_mark_hide();
    return 1;
}
#endif

/* 37: a card's stats change (some cards have their own animation) */
#ifdef NON_MATCHING /* cadeia de testes */
int dd_ev_37(int player, int zone, int a, int card_id)
{
    DdCard *card = dd_cobj_at(dv_player_to_side(player), zone, 0);

    if (card == 0)
        return 1;
    if (card_id == 0x1A90)
        dd_num_counter(card, a, 0);
    else if (card_id == 0x1513)
        dd_num_counter(card, a, 2);
    else if (card_id == 0x1439)
        dd_num_counter(card, a, 1);
    else if (card_id == 1)
        dd_stat_counter(card, a, 0);
    else if (card_id == 0)
        dd_stat_counter(card, a, 1);
    else
        dd_stat_counter(card, a, 0);
    return 1;
}
#endif

int dd_ev_38(int player, int zone, int a)
{
    DdCard *card = dd_cobj_at(dv_player_to_side(player), zone, 0);

    if (card == 0)
        return 1;
    dd_num_counter(card, -(a != 3), 3);
    return 1;
}

int dd_ev_39(int player, int a)
{
    dd_move(dv_player_to_side(player), a);
    return 1;
}

int dd_ev_40(int player, int zone, int idx)
{
    DdCard *card = dd_cobj_find(dv_player_to_side(player), zone, idx);

    if (card == 0)
        return 1;
    dd_glow_on(card);
    return 1;
}

int dd_ev_41(void)
{
    dd_glow_all_off();
    return 1;
}

int dd_ev_42(int player, int zone, int idx)
{
    DdCard *card = dd_cobj_find(dv_player_to_side(player), zone, idx);

    if (card == 0)
        return 1;
    dd_glow_off(card);
    dd_beam_summon(card);
    card->b4 = 1;
    return 1;
}

int dd_ev_43(int player, int zone, int idx)
{
    DdCard *card = dd_cobj_find(dv_player_to_side(player), zone, idx);

    if (card == 0)
        return 1;
    dd_mark_on(card);
    return 1;
}

int dd_ev_44(void)
{
    dd_mark_all_off();
    return 1;
}

int dd_ev_45(int player, int zone, int idx)
{
    DdCard *card = dd_cobj_find(dv_player_to_side(player), zone, idx);

    if (card == 0)
        return 1;
    dd_mark_off(card);
    dd_beam_set2(card, dv_player_to_side(player), zone);
    card->b4 = 1;
    return 1;
}

int dd_ev_46(int player, int zone, int idx)
{
    dd_ev_flip_cutin = 1;
    dd_hl_push(dv_player_to_side(player), zone, idx);
    return 1;
}

/* end of the flip cut-in */
void dd_ev_flip_done(DdFlip *f)
{
    DdLoc l = f->loc;

    if (f->value > 0)
        dd_popup(l.player, l.zone, f->value, 0x2F, 0);
    dd_hl_pop(l.player, l.zone, l.idx);
    dd_free(f);
}

/* 47: a face-down card is flipped (cut-in when requested by event 46) */
#ifdef NON_MATCHING /* bitfields e alocação */
int dd_ev_47(int player, int zone, int idx)
{
    char *eng = dv_chain();
    unsigned short *h = (unsigned short *)(eng + 0x300 + (*(int *)(eng + 0x480) - 1) * 0x18);
    int v = *h;
    DdFlip *f;
    DdCutinDesc d;

    if (dd_ev_flip_cutin != 0) {
        dd_ev_flip_cutin = 0;
        f = dd_alloc(8);
        memset(f, 0, 8);
        f->loc.player = dv_player_to_side(player);
        f->loc.zone = zone;
        f->loc.idx = idx;
        f->value = v;
        memset(&d, 0, sizeof(d));
        d.ehf = dd_ehf_get(0x2000C);
        d.end = -1;
        d.done = (void (*)(int))dd_ev_flip_done;
        d.flag = 0x2F;
        d.always = 1;
        d.se = 0x6243;
        d.start = 0;
        d.done_arg = (int)f;
        dd_cutin_play(&d);
    } else {
        if (v != 0 && !(((unsigned char *)h)[4] & 2) && !(((unsigned char *)h)[4] & 4))
            dd_popup(dv_player_to_side(player), zone, v, 0x2F, 0);
        dd_hl_pop(dv_player_to_side(player), zone, idx);
    }
    return 1;
}
#endif

int dd_ev_48(void)
{
    return 1;
}

/* 49: message windows (text id or string, kind 0..13) */
#ifdef NON_MATCHING /* switch com tabela e testes aninhados */
int dd_ev_49(int kind, unsigned int text, unsigned int arg)
{
    int id;
    void *msg;
    int flag;
    unsigned int hi;

    if ((text & 0xFFFF0000) == 0) {
        msg = dlg_get_text(text);
        id = text;
    } else {
        msg = (void *)text;
        id = -1;
    }
    dd_view_x14_set(kind);
    switch (kind) {
    case 0:
        func_088226C0(msg, 0, 0);
        break;
    case 1:
        if (id == 6)
            flag = 1;
        else
            flag = 0;
        if ((int)arg > 0)
            dd_info_select_uid(arg);
        func_088226C0(msg, 1, flag);
        break;
    case 2:
        dv_w624_set(0);
        switch (arg) {
        case 3:
        case 2:
            break;
        case 1:
        case 0:
            if ((*dd_pad() & (region_pick_value(0) & 0xFFFF)) == 0)
                goto done;
            if (dd_is_tag_duel() != 0)
                goto done;
            break;
        default:
            goto done;
        }
        if ((*dd_pad() & (region_pick_value(0) & 0xFFFF)) == 0 &&
            (*dd_pad() & (region_pick_value(1) & 0xFFFF)) != 0)
            break;
        func_0882214C(msg, arg);
        break;
    case 4:
        func_08821D20(msg, arg);
        break;
    case 5:
        func_08824544(msg, arg);
        break;
    case 6:
        func_08823E34(msg, arg & 0xFFFF, arg >> 16);
        break;
    case 7:
        func_08822838(msg, arg & 0xFFFF, arg >> 16);
        dd_info_select_uid(arg & 0xFFFF);
        break;
    case 8:
        func_08824CC8(msg);
        break;
    case 9:
        dd_dice_open(msg, arg & 0xFFFF, arg >> 16);
        break;
    case 10:
        hi = arg >> 16;
        func_08824F3C(msg, (hi & 0xFF) & 0xF, hi >> 8 & 0xFF, ((hi & 0xFF) & 0xF0) >> 4, arg & 0xFFFF);
        break;
    case 12:
        func_0882308C(msg);
        break;
    case 3:
        func_08823674(msg);
        break;
    case 13:
        dd_wait_dots_open(msg);
        break;
    }
done:
    dd_ev_36();
    return 1;
}
#endif

/* 50: prompts (choice of zone, yes/no, ...) */
#ifdef NON_MATCHING /* cadeia de faixas */
int dd_ev_50(int player, int kind)
{
    int ok, zone;
    void *msg;

    if (kind == 0x4A) {
        dd_sel_open_engine(1, 1, 0, 0);
    } else if (kind == 0x4D || kind == 0x4C || kind == 0x4B) {
        declare_open(kind);
    } else if (kind >= 0x28 && kind <= 0x47) {
        dd_sel_open_engine(kind - 0x27, 1, 0, 0);
    } else if (kind == 0x27) {
    } else if (kind == 0x48) {
        dd_sel_open_engine(dv_0884C824(), 0, 0, 0);
    } else if (kind >= 7 && kind <= 0x26) {
        dd_sel_open_engine(kind - 6, kind - 6, 0, 0);
    } else if (kind == 6) {
        dd_sel_open_engine(1, 1, 0, 0);
    } else if (kind == 5 || kind == 4 || kind == 3 || kind == 2 || kind == 1) {
        zone = 0;
        ok = 0;
        if (kind == 4) {
            ok = 1;
            zone = 0xF;
        } else if (kind == 3) {
            ok = 1;
            zone = 0xE;
        } else if (kind == 2) {
            zone = 0xD;
        } else if (kind == 1) {
            zone = 0xC;
        }
        msg = dlg_get_text((kind - 1) * 2 + dv_player_to_side(player) + 0x246);
        if (kind != 5)
            dd_cursor_set(dv_player_to_side(player), zone, 0, 1);
        dd_sel_open_engine(0, 0, msg, ok);
    }
    return 1;
}
#endif

int dd_ev_51(int player, int zone)
{
    DdCard *card = dd_cobj_at(dv_player_to_side(player), zone, 0);

    if (card == 0)
        return 1;
    dd_cobj_end_a3(card);
    return 1;
}

int dd_ev_52(DdLoc2 p)
{
    DdCard *card = dd_cobj_at(dv_player_to_side(p.player), p.zone, 0);

    if (card == 0)
        return 1;
    dd_cobj_end_a4(card);
    return 1;
}

/* 53: a list of cards to show (n entries from the engine) */
#ifdef NON_MATCHING /* alocação do laço */
int dd_ev_53(int player, int uid, int n)
{
    int *list;
    int i;
    int *p;

    list = dd_alloc(n << 2);
    p = list;
    for (i = 0; i < n; i++)
        *p++ = dv_0884C830(i);
    func_08820678(dv_list_id(uid), list, n);
    dd_free(list);
    return 1;
}
#endif

int dd_ev_54(void)
{
    dd_info_select();
    return 1;
}

int dd_ev_55(int player, int a, int b, int c, int d)
{
    func_08820430(dv_player_to_side(player), b, c, d);
    return 1;
}

int dd_ev_56(int player, int a, int b)
{
    func_088204BC(dv_player_to_side(player), b);
    return 1;
}

int dd_ev_57(int a, int on)
{
    if (on != 0)
        func_088208B0();
    return 1;
}

int dd_ev_58(void)
{
    if (sysvar_get_a98() == 0 && cfg_get_flag(0) != 0) {
        dd_mode_set_card(0xFBB);
        dd_mode_push(0xB);
    }
    return 1;
}

int dd_ev_59(int a, int b, int c, int d)
{
    func_088206D4(c, d);
    return 1;
}

int dd_ev_60(void)
{
    func_088207A8();
    return 1;
}

#ifdef NON_MATCHING /* ordem dos testes do switch */
int dd_ev_61(int kind)
{
    switch (kind) {
    case 0:
        func_0882059C(0x3D, dd_mode_cancel);
        break;
    case 1:
        break;
    case 2:
        break;
    }
    return 1;
}
#endif

int dd_ev_73(int player, int a)
{
    dd_chara_set_anim_range(dd_cobjs_map_get(dv_player_to_side(player)), dd_08806B5C(dv_player_to_side(player)), a);
    return 1;
}

/* export: commentary on/off (the exports that follow are in dd_voice.c) */
#ifdef NON_MATCHING /* teste do segundo caso sai com bnez + b em vez de beqz */
void dd_voice_enable(int on)
{
    if (on != 0 && dd_scene_flags(8, -1) != 0)
        return;
    if (on == 0 && dd_scene_flags(8, -1) == 0)
        return;
    if (on != 0) {
        dd_chara_busy_clear();
        dd_scene_flags(8, 1);
        dd_mode_push(6);
    } else {
        dd_mode_pop();
        dd_scene_flags(8, 0);
        dd_chara_reset();
    }
}
#endif
