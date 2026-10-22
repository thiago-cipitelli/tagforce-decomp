// @range 0x08834734-0x08835AEC
// @asm 0x08834A84-0x08834E44
/* Steps of the computer player's actions (duel_w_08919644 is the step, duel_w_08919638 cleared
   when the action is done). */
#include "duel.h"

extern unsigned int duel_w_0891964C;   /* player acting */
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_w_08919644;            /* step */
extern int duel_w_08919658;
extern int duel_w_08919628;
extern int duel_w_0891962C;
extern int duel_w_08919638;
extern int duel_w_08919624;
extern unsigned int duel_ai_player;
extern int duel_w_08919630;            /* kind of choice the computer makes */
extern int duel_w_08919634;
extern int duel_w_0891963C;
extern int duel_w_08919640;
extern int (*duel_w_08919660)();       /* filter of the choice */
extern int duel_w_089195E8;
extern int duel_player_type[2];
extern unsigned int duel_turn;
extern unsigned int duel_view_player;
extern unsigned int duel_turn_player;
extern int duel_phase;
extern int duel_err;

int duel_088C7548(unsigned int p, unsigned int i, int x);
void duel_0884F398(unsigned int code, int b);
void duel_0884F654(unsigned int code, int b, int c);
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp);
DuelCard *duel_0884C7CC(void);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0884AD18(int msg);
void duel_088367A8(unsigned int p, int b, int id, int uid);
int duel_card_protected(int id, int uid);
int duel_can_pay(unsigned int p, int id);
int card_get_type(int id);
int card_is_monster(int id);
int duel_can_place_st(unsigned int p, int id);
int duel_0886AFF8(unsigned int p, int i, int id);
int duel_df664(unsigned int p);
int duel_card_e59b0(int id);
int card_attribute_icon_in_2_3(int id);
unsigned int duel_08828740(unsigned int p, int uid);
unsigned int duel_08827D98(unsigned int p, int uid);
int card_is_1a9f(int id);
int card_is_in_group_1a60(int id);
int card_get_prop5_hi(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_field_protected(unsigned int p, int z);
int duel_monster_can_act(unsigned int p, int z);
int duel_facedown_can_act(unsigned int p, int z);
int duel_attack_forbidden(unsigned int p, int z);
int duel_can_attack_any(unsigned int p, int z, int arg);
int duel_0884E340(int id);
int card_is_in_group_1517(int id);
int duel_0886981C(DuelAct *a, DuelAct *b);
int duel_0886AF00(unsigned int p, int z, int id);
int duel_0886AE08(unsigned int p, int z, int id);
unsigned int duel_zone_count(unsigned int p, int z);
int duel_08869728(unsigned int p, int i, int a);
int duel_088695B8(unsigned int p, unsigned int q, int z, int a);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_08818C34(unsigned int p, int a, int (*fn)());
unsigned short duel_08835834(unsigned int p, int z);
unsigned short duel_088358F8(unsigned int p, int z);
void duel_cmd(unsigned short cmd, int p, unsigned short id, int arg);

void duel_08834734(int a, int b, int c)
{
    switch (duel_w_08919644) {
    case 0:
        duel_w_08919658 = duel_088C7548(duel_w_0891964C, duel_w_08919654, a);
        duel_w_08919644++;
        break;
    case 1:
        duel_w_08919644++;
        break;
    default:
        if (a != 0) {
            if (b != 0) {
                duel_0884F654((unsigned short)duel_w_08919628 | ((duel_w_0891964C << 31 & 0x80000000) | (duel_w_08919658 << 16 & 0x1F0000)),
                              (unsigned short)duel_w_0891962C, c);
            } else {
                duel_0884F398((unsigned short)duel_w_08919628 | ((duel_w_0891964C << 31 & 0x80000000) | (duel_w_08919658 << 16 & 0x1F0000)),
                              (unsigned short)duel_w_0891962C);
            }
        }
        duel_w_08919638 = 0;
        break;
    }
}

void duel_08834850(unsigned short k)
{
    switch (k) {
    case 9:
    case 8:
        duel_088C9878(duel_w_0891964C, duel_w_08919650 + duel_w_08919654, 0, 0, 0, 0);
        break;
    }
    duel_w_08919638 = 0;
}

void duel_088348BC(int a, unsigned int p, int b)
{
    switch (duel_w_08919644) {
    case 0:
        duel_0884ACD8(p, 0x114);
        duel_w_08919644++;
        break;
    case 1:
        duel_0884C588(p, 6, 0xFFFE, b);
        duel_w_08919644++;
        break;
    case 2:
        duel_088367A8(p, b, duel_0884C7CC()->id, duel_0884C7CC()->owner + (duel_0884C7CC()->uid_hi << 1));
        duel_w_08919638 = 0;
        break;
    }
}

void duel_088349BC(void)
{
    switch (duel_w_08919644) {
    case 0:
        if (duel_turn + 1 < 10) {
            duel_0884ACD8(duel_ai_player, 0x52);
            duel_w_08919638 = 0;
        } else {
            duel_0884AD18(6);
            duel_w_08919644++;
        }
        break;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_cmd((duel_w_0891964C ? 0x8000 : 0) | 0x23, 10, 0, 0);
        }
        duel_w_08919638 = 0;
        break;
    }
}

#ifdef NON_MATCHING /* o compilador dobra r = 0 em r | duel_08827D98(); leiaute do teste card_get_prop5_hi == 5 não reproduzido */
unsigned int duel_08834A84(int id, unsigned int p, int i)
{
    unsigned int r;
    int uid = CARD_UID(duel.players[p & 1].hand[i]);

    r = 0;
    if (p == (duel_view_player ^ 1)) {
        return 0;
    }
    if (duel_card_protected(id, -1)) {
        if (p == duel_view_player) {
            duel_err = 0x10;
        }
        return 0;
    }
    if (!duel_can_pay(p, id)) {
        if (p == duel_view_player) {
            duel_err = 0x17;
        }
        return 0;
    }
    switch (duel_phase) {
    case 2:
    case 4:
        switch (card_get_type(id)) {
        case 0x16:
        case 0x17:
            if (duel_can_place_st(p, id)) {
                if (duel_0886AFF8(p, i, id)) {
                    r |= 8;
                }
                if (duel_df664(p) && !duel_card_e59b0(id)) {
                    r |= 0x80;
                }
            } else {
                duel_err = 0xC;
            }
            break;
        default:
            if (card_attribute_icon_in_2_3(id)) {
                duel_err = 0xD;
                r |= duel_08828740(p, uid);
            } else if (!(duel.players[p & 1].flags >> 17 & 1)) {
                r = r | duel_08827D98(p, uid) | duel_08828740(p, uid);
            } else {
                duel_err = 2;
                r |= duel_08828740(p, uid);
            }
            if (card_is_1a9f(id) && duel_can_place_st(p, id) && duel_df664(p)) {
                r |= 0x80;
            }
            if (card_is_in_group_1a60(id) && duel_0886AFF8(p, i, id)) {
                r |= 8;
            }
            break;
        }
        break;
    case 3:
        switch (card_get_type(id)) {
        case 0x16:
            if (card_get_prop5_hi(id) == 5) {
                if (duel_can_place_st(p, id)) {
                    if (duel_0886AFF8(p, i, id)) {
                        r |= 8;
                    }
                } else {
                    duel_err = 0xC;
                }
            } else {
                duel_err = 5;
            }
            break;
        case 0x17:
            duel_err = 0xF;
            if (duel_0886AFF8(p, i, id)) {
                r |= 8;
            }
            break;
        default:
            duel_err = 6;
            break;
        }
        break;
    }
    if (!card_is_monster(id) && duel_slot_has_effect(0, 0xB, 0x1407)) {
        r &= ~0x88;
    }
    return r;
}
#endif

unsigned int duel_08834E44(int id, unsigned int p, int z)
{
    DuelSlot *s = &duel.players[p & 1].field[z];
    unsigned int r = 0;

    if (z > 4) {
        return 0;
    }
    if (p == (duel_view_player ^ 1)) {
        if (card_is_monster(id) && s->faceup && card_is_in_group_1517(id)) {
            DuelAct t = {0};

            t.player = p;
            t.opp = 1;
            t.zone = z;
            t.kind = 0;
            t.b3_4 = 3;
            t.uid = CARD_UID(duel.players[p & 1].field[z].card);
            duel_err = 4;
            t.id = id;
            if (duel_0886981C(&t, 0)) {
                r |= 8;
            }
        }
        return r;
    }
    switch (duel_phase) {
    case 2:
    case 4:
        if (duel_field_protected(p, z)) {
            if (p == duel_view_player) {
                duel_err = 0x10;
            }
        } else if (duel_monster_can_act(p, z)) {
            if (s->faceup) {
                if (s->pos != 0) {
                    r |= 0x100;
                } else {
                    r |= 0x200;
                }
            } else {
                if (s->pos == 0) {
                    r |= 0x200;
                }
                if (duel_facedown_can_act(p, z)) {
                    r |= 0x20;
                }
            }
        }
        break;
    case 3:
        if (!duel_attack_forbidden(p, z)) {
            duel_err = 7;
            if (duel_can_attack_any(p, z, 1)) {
                r |= 1;
            }
        } else {
            duel_err = 8;
        }
        break;
    }
    if (*(unsigned short *)((char *)&duel.players[0].field[0].faceup + (p & 1) * 0xFAC + z * 20) && card_is_monster(id) && duel_0884E340(id)) {
        DuelAct t = {0};

        t.player = p;
        t.zone = z;
        t.kind = 0;
        t.b3_4 = 3;
        t.uid = CARD_UID(duel.players[p & 1].field[z].card);
        duel_err = 4;
        t.id = id;
        if (duel_0886981C(&t, 0)) {
            r |= 8;
        }
    }
    return r;
}

unsigned int duel_08835274(int id, unsigned int p, int z)
{
    DuelSlot *s = &duel.players[p & 1].field[z];
    unsigned int r = 0;
    int ok;

    if (duel_field_protected(p, z)) {
        if (p == duel_view_player) {
            duel_err = 0x10;
        }
        return 0;
    }
    if (p == (duel_view_player ^ 1)) {
        if (!card_is_monster(id) && s->faceup && card_is_in_group_1517(id)) {
            DuelAct t = {0};

            t.player = p;
            t.opp = 1;
            t.zone = z;
            t.kind = 0;
            t.b3_4 = 3;
            t.uid = CARD_UID(duel.players[p & 1].field[z].card);
            duel_err = 4;
            t.id = id;
            if (duel_0886981C(&t, 0)) {
                r |= 8;
            }
        }
        return r;
    }
    if (s->faceup == 0) {
        switch (card_get_type(id)) {
        case 0x16:
            duel_err = 4;
            ok = 0;
            if (card_get_prop5_hi(id) == 5) {
                ok = 1;
            }
            if (duel_phase == 2 || duel_phase == 4) {
                if (duel_turn_player == duel_view_player) {
                    ok = 1;
                }
            }
            if (duel_phase == 1 && id == 0x131E) {
                ok = 1;
            }
            if (ok) {
                if (card_get_prop5_hi(id) == 3) {
                    duel_err = 10;
                }
                if (duel_0886AF00(p, z, id)) {
                    r |= 8;
                }
            } else if (duel_phase == 3) {
                duel_err = 5;
            }
            break;
        case 0x17:
            duel_err = 4;
            if (duel_0886AF00(p, z, id)) {
                r |= 8;
            }
            break;
        }
    } else if (duel_0884E340(id)) {
        duel_err = 4;
        if (duel_0886AE08(p, z, id)) {
            r |= 8;
        }
    }
    if (!card_is_monster(id) && duel_slot_has_effect(0, 0xB, 0x1407)) {
        r &= ~8;
    }
    return r;
}

int duel_08835620(unsigned int p, int z, unsigned int i)
{
    int r = 0;

    switch (z) {
    case 0xB:
        if (i >= duel.players[p & 1].n_hand) {
            return 0;
        }
        if (duel_ai_player == p && duel_08869728(p, i, 0)) {
            r |= 8;
        }
        break;
    case 0xD:
    case 0xC:
    case 0xE:
    case 0xF:
        r |= duel_08835834(p, z);
        r |= duel_088358F8(p, z);
        break;
    default:
        if (duel_088695B8(duel_ai_player, p, z + i, 0)) {
            r |= 8;
        }
        break;
    }
    return r;
}

int duel_08835740(int a, unsigned int p, int z, unsigned int i)
{
    int r = 0;

    switch (z) {
    case 0xB:
        if (i >= duel.players[p & 1].n_hand) {
            return 0;
        }
        if (duel_ai_player == p && duel_08869728(p, i, a)) {
            r |= 8;
        }
        break;
    case 0xD:
    case 0xC:
    case 0xE:
    case 0xF:
        r |= duel_08835834(p, z);
        break;
    default:
        if (duel_088695B8(duel_ai_player, p, z + i, a)) {
            r |= 8;
        }
        break;
    }
    return r;
}

unsigned short duel_08835834(unsigned int p, int z)
{
    if (duel_zone_count(p, z) != 0) {
        switch (z) {
        case 0xD:
            if (duel_w_089195E8 == 3) {
                return 2;
            }
            break;
        case 0xC:
            if (duel_w_089195E8 == 3) {
                return 2;
            }
            if (p == duel_view_player) {
                return 2;
            }
            break;
        case 0xE:
        case 0xF:
            return 2;
        }
    }
    return 0;
}

unsigned short duel_088358F8(unsigned int p, int z)
{
    if (duel_088BFF78(p, 0xFFFE, z) != 0) {
        return 8;
    }
    return 0;
}

int duel_08835924(void)
{
    if (duel_w_08919634 != 0) {
        return duel_w_08919630;
    }
    return 0;
}

void duel_08835944(int k)
{
    duel_w_08919630 = k;
    duel_w_08919660 = 0;
    duel_w_0891963C = 0;
    duel_w_08919638 = 0;
    duel_ai_player = duel_view_player;
}

void duel_08835978(int (*fn)())
{
    duel_w_08919630 = 7;
    duel_w_08919660 = fn;
    duel_w_0891963C = 0;
    duel_w_08919638 = 0;
    duel_ai_player = duel_view_player;
}

void duel_088359B0(int (*fn)())
{
    duel_w_08919630 = 9;
    duel_w_08919660 = fn;
    duel_w_0891963C = 0;
    duel_w_08919638 = 0;
    duel_ai_player = duel_view_player;
}

void duel_088359E8(unsigned int p, int a, int (*fn)())
{
    if (duel_player_type[p] == 1) {
        duel_08818C34(p, a, fn);
    } else {
        duel_w_08919630 = 8;
        duel_w_08919660 = fn;
        duel_w_0891963C = 0;
        duel_w_08919638 = 0;
        duel_ai_player = p;
    }
}

void duel_08835A54(unsigned int p, int a, int (*fn)())
{
    if (duel_player_type[p] == 1) {
        duel_08818C34(p, a, fn);
    } else {
        duel_w_08919630 = 10;
        duel_w_08919660 = fn;
        duel_w_0891963C = 0;
        duel_w_08919638 = 0;
        duel_ai_player = p;
    }
}

int duel_08835AC0(void)
{
    return duel_w_08919638 != 0 && duel_w_08919640 == 0xB;
}
