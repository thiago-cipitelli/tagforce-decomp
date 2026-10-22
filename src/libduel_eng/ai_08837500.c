// @range 0x08837500-0x08838548
// @data 0x08907C0C-0x08907C3C
/* Battle phase step machine of a player (duel_w_08919610): choose the attacker and the target. */
#include "duel.h"

extern int duel_w_0891960C;            /* battle step */
extern int duel_w_08919610;
extern int duel_w_08919614;
extern int duel_w_08919624;
extern int duel_w_08919638;
extern int duel_w_0891963C;
extern int duel_w_08919640;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_player_type[2];
extern int duel_w_08913EF8;
extern unsigned int duel_atk_tp;       /* 0x08913F14 */
extern int duel_atk_direct;            /* 0x08913F18 */
extern int duel_atk_w1C;               /* 0x08913F1C */
extern int duel_atk_w20;               /* 0x08913F20 */
extern int duel_atk_w24;               /* 0x08913F24 */
extern int duel_atk_w28;               /* 0x08913F28 */
extern int duel_atk_zone;              /* 0x08913F2C */
extern int duel_atk_tz;                /* 0x08913F30 */
extern int duel_w_08913EF0;
extern int duel_w_08913EFC;
extern unsigned short duel_w_0891968C;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_08843844(void);
int duel_08843440(unsigned int p);
int duel_088434D8(unsigned int p);
int duel_08843344(unsigned int p);
int duel_08820FD0(unsigned int p, int z);
int duel_0880EA88(unsigned int p, int a);
int duel_can_attack(unsigned int p, int z, int arg);
int duel_can_attack_any(unsigned int p, int z, int arg);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
int duel_find_effect_card(unsigned int p, int id, int z);
int field_get_card(unsigned int p, int z);
void duel_088CC178(unsigned int p, int z, unsigned int v);
int duel_088369D8(unsigned int p);
int duel_host_call(int a, int b, int c, int d);
void duel_08835944(int k);
int duel_slot_has_effect(unsigned int p, int z, int id);
void duel_0884AD18(int msg);
void duel_0884AF20(int f);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_08836C98(unsigned int p, int z);
int duel_0880EB64(unsigned int p, int z, int a);
int duel_can_direct_attack(unsigned int p, int z);
int duel_direct_attack_kind(unsigned int p, int z, int arg);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_count_e17a0(unsigned int p, int id);
void duel_088C448C(int p, int a);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_count_attack_targets(unsigned int p, int z);
void duel_08832950(unsigned int p, unsigned short id);
int duel_088E9150(int id);
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088328E8(unsigned int p, unsigned short id, int a);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_08836E74(unsigned int p, int z);
void duel_08835978(int (*fn)());
void duel_088359B0(int (*fn)());
int duel_08837B3C(unsigned int q, int z, int i);

int duel_08837500(unsigned int p)
{
    int i;
    int a;
    int b;
    int c;
    int d;
    int r;

    duel_special.player[0] = p;
    switch (duel_w_08919610) {
    case 0:
        duel_atk_direct = 0;
        duel_atk_w24 = 0;
        duel_cmd((p ? 0x8000 : 0) | 0x15, 0, 0, 0);
        duel_w_08919610++;
        return 0;
    case 1:
        if (duel_08843844() != 0) {
            a = duel_08843440(p);
            i = 1 - p;
            b = duel_088434D8(i);
            if (a >= 0 && b >= 0 && duel_can_attack(p, a, 1) && duel_can_attack_target(p, a, i, b, 1)) {
                duel_atk_zone = duel_08843440(p);
                duel_atk_w20 = 0;
                duel_atk_w1C = 0;
                return 1;
            }
            duel_w_0891960C = 0xC;
            duel_w_08919610 = 0;
            duel_w_08919614 = 0;
            return 0;
        }
        if (duel_find_effect_card(p, 0x1A79, -1)) {
            for (c = 0; c <= 4; c++) {
                if (field_get_card(p, c) == 0x1A79 && duel.players[p & 1].field[c].link) {
                    r = 0;
                    if (duel_can_attack(p, c, 1)) {
                        for (d = 0; d <= 4; d++) {
                            if (!duel_can_attack_target(p, c, 1 - p, d, 1)) {
                                continue;
                            }
                            r = 1;
                            break;
                        }
                    }
                    if (!r) {
                        duel_088CC178(p, c, 0);
                        return 0;
                    }
                }
            }
        }
        if (duel_player_type[p] == 1) {
            i = duel_0880EA88(p, !duel_088369D8(p));
            if (duel_08820FD0(p, i)) {
                duel_w_0891963C = 1;
                return 0;
            }
            if (i != 0) {
                duel_atk_zone = duel_w_08913EF8;
                duel_atk_w20 = 0;
                duel_atk_w1C = 0;
                duel_atk_w28 = 0;
                return 1;
            }
            duel_w_0891960C = 0xC;
            duel_w_08919610 = 0;
            duel_w_08919614 = 0;
            return 0;
        }
        for (i = 0; i <= 4; i++) {
            if (!duel_can_attack_any(p, i, 1)) {
                continue;
            }
            duel_host_call(7, p, i, 0);
            break;
        }
        duel_08835944(3);
        duel_w_08919610++;
        return 0;
    case 2:
        if (duel_w_08919638 != 0) {
            if (duel_w_08919640 == 0) {
                duel_atk_zone = duel_w_08919650 + duel_w_08919654;
                duel_atk_w20 = 0;
                duel_atk_w1C = 0;
                duel_atk_w28 = 0;
                duel_host_call(0xB, duel_atk_zone, 1 - p, -1);
                return 1;
            }
            duel_w_0891963C = 1;
            duel_w_08919610--;
        } else {
        if (duel_088369D8(p)) {
            if (duel_slot_has_effect(p, 0xB, 0x11ED)) {
                duel_0884AD18(0xDD);
                duel_w_08919610++;
                return 0;
            }
            switch (duel_w_08919640 - 0xD) {
            case 4:
                duel_w_0891960C = 0xC;
                duel_w_08919610 = 0;
                duel_w_08919614 = 0;
                return 0;
            case 5:
                if (duel_08843344(p)) {
                    duel_w_0891960C = 0xC;
                    duel_w_08919610 = 0;
                    duel_w_08919614 = 1;
                    return 0;
                }
                break;
            }
        }
        duel_w_08919610++;
        duel_w_08919624 = 0;
        return 0;
        }
        return 0;
    case 3:
        if (duel_w_08919624 != 0) {
            duel_w_0891960C = 0xC;
            duel_w_08919610 = 0;
            duel_w_08919614 = 0;
            return 0;
        }
        r = 8;
        if (duel_088369D8(p)) {
            r |= 0x10;
            if (duel_08843344(p)) {
                r |= 0x20;
            }
        }
        duel_0884AF20(r);
        duel_w_08919610++;
        return 0;
    case 4:
        switch (duel_w_08919624) {
        case 4:
            duel_w_0891960C = 0xC;
            duel_w_08919610 = 0;
            duel_w_08919614 = 0;
            duel_088C89BC(p, 0xB, 0x11ED, 1);
            return 0;
        case 5:
            duel_w_0891960C = 0xC;
            duel_w_08919610 = 0;
            duel_w_08919614 = 1;
            return 0;
        default:
            duel_w_08919610 = 1;
            return 0;
        }
    }
    return 0;
}

int duel_08837B3C(unsigned int q, int z, int i)
{
    if (q != duel_special.player[0] && z + i < 5 &&
        duel_can_attack_target(duel_special.player[0], duel_atk_zone, q, z + i, 1)) {
        return 0x800;
    }
    return 0;
}

int duel_08837B9C(unsigned int p)
{
    unsigned int q = 1 - p;
    int k;

    duel_atk_tp = q;
    switch (duel_w_08919610) {
    case 0:
        if (duel_atk_w28 != 0) {
            if (!duel_08836C98(p, duel_atk_zone)) {
                duel_w_08919624 = 1;
            } else if (duel_player_type[p] == 1) {
                duel_w_08919624 = duel_0880EB64(p, duel_atk_zone, 0);
            } else {
                duel_0884AD18(0xDF);
            }
            duel_w_08919610++;
            return 0;
        }
        duel_w_08919610++;
    case 1:
        if (duel_atk_w28 != 0 && duel_w_08919624 == 0) {
            duel_08836E74(p, duel_atk_zone);
            return 0;
        }
        if (duel_can_direct_attack(p, duel_atk_zone)) {
            duel_atk_direct = 1;
            duel_w_08919610 = 5;
            return 0;
        }
        if (duel_08843844()) {
            k = duel_088434D8(q);
            duel_088C77D4(p, 0x151E, 4, 0);
            if (k >= 0) {
                duel_atk_tz = k;
                duel_w_08919610 = 5;
                return 0;
            }
            duel_w_0891960C = 1;
            duel_w_08919610 = 0;
            return 0;
        }
        if (!duel_slot_immune(p, duel_atk_zone, 1) && duel_count_e17a0(q, 0x1318) > 0) {
            duel_088C448C(q, 0x1318);
            if (duel_count_e17a0(q, 0x1318) == 1) {
                for (k = 0; k < 5; k++) {
                    if (duel_slot_affected(q, k, 0x1318)) {
                        break;
                    }
                }
                duel_atk_tz = k;
                duel_w_08919610 = 5;
                return 0;
            }
            duel_w_08919610 = 3;
            return 0;
        }
        switch (duel_direct_attack_kind(p, duel_atk_zone, 1)) {
        case 2:
            duel_atk_direct = 1;
            duel_w_08919610 = 5;
            return 0;
        case 1:
            if (duel_count_attack_targets(p, duel_atk_zone)) {
                if (duel_find_effect_card(q, 0x139C, -1)) {
                    duel_088C448C(q, 0x139C);
                    duel_08832950(q, 0x139C);
                    duel_w_08919610 = 10;
                    return 0;
                }
                if (duel_088E9150(0x1443)) {
                    k = duel_slot_e8170(1, 10, 0x1443);
                    duel_088C4448(k, 10, CARD_UID(duel.players[k & 1].field[10].card));
                    duel_08832950(q, 0x139C);
                    duel_w_08919610 = 10;
                    return 0;
                }
                if (duel_slot_has_effect(q, 0xB, 0x177A)) {
                    duel_08832950(q, 0x139C);
                    duel_w_08919610 = 10;
                    return 0;
                }
            } else {
                duel_atk_direct = 1;
                duel_w_08919610 = 5;
                return 0;
            }
            if (duel_player_type[p] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884AD18(0xE1);
            }
            duel_w_08919610 = 2;
            return 0;
        default:
            duel_w_08919610 = 3;
            return 0;
        }
    case 2:
        if (duel_w_08919624 != 0) {
            duel_atk_direct = 1;
            duel_w_08919610 = 5;
            return 0;
        }
        duel_w_08919610++;
    case 3:
        if (duel_count_attack_targets(p, duel_atk_zone) >= 2) {
            if (duel_find_effect_card(q, 0x139C, -1)) {
                duel_088C448C(q, 0x139C);
                duel_088328E8(q, 0x139C, duel_atk_zone);
                duel_w_08919610 = 0xB;
                return 0;
            }
            if (duel_088E9150(0x1443)) {
                k = duel_slot_e8170(1, 10, 0x1443);
                duel_088C4448(k, 10, CARD_UID(duel.players[k & 1].field[10].card));
                duel_088328E8(q, 0x139C, duel_atk_zone);
                duel_w_08919610 = 0xB;
                return 0;
            }
            if (duel_slot_has_effect(q, 0xB, 0x177A)) {
                duel_088328E8(q, 0x139C, duel_atk_zone);
                duel_w_08919610 = 0xB;
                return 0;
            }
        }
        if (duel_player_type[p] == 1) {
            if (duel_0880EB64(p, duel_atk_zone, duel_atk_w28 != 0 ? !duel_08836C98(p, duel_atk_zone) : !duel_088369D8(p))) {
                duel_atk_direct = duel_w_08913EF0;
                duel_atk_tz = duel_w_08913EFC;
                duel_w_08919610 = 5;
                return 0;
            }
            duel_08836E74(p, duel_atk_zone);
            return 0;
        }
        duel_host_call(10, 0, 0, 0);
        for (k = 0; k <= 4; k++) {
            if (!duel_can_attack_target(p, duel_atk_zone, q, k, 1)) {
                continue;
            }
            duel_host_call(7, q, k, 0);
            break;
        }
        if (duel_atk_w28 != 0 && !duel_08836C98(p, duel_atk_zone) && duel_direct_attack_kind(p, duel_atk_zone, 1) != 1) {
            duel_088359B0(duel_08837B3C);
        } else {
            duel_08835978(duel_08837B3C);
        }
        duel_w_08919610++;
        return 0;
    case 4:
        if (duel_w_08919638 != 0) {
            if (duel_w_08919640 == 0xB) {
                duel_atk_tz = duel_w_08919650 + duel_w_08919654;
                duel_w_08919610++;
                return 0;
            }
            duel_w_08919610--;
        } else {
            if (duel_atk_w28 != 0) {
                duel_w_08919610 = 0;
                return 0;
            }
            duel_08836E74(p, duel_atk_zone);
        }
        return 0;
    case 5:
        duel_088C89BC(q, 0xB, 0x177A, 1);
        if (duel_atk_direct != 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x1A, duel_atk_zone, 1, 0);
            duel_atk_tz = 5;
            switch (field_get_card(p, duel_atk_zone)) {
            case 0x1A41:
            case 0x1A52:
                if (!duel_can_direct_attack(p, duel_atk_zone) && duel_direct_attack_kind(p, duel_atk_zone, 1) &&
                    !duel_direct_attack_kind(p, duel_atk_zone, 0)) {
                    duel_088C77D4(p, 0x1A41, 4, 0);
                }
                break;
            }
            return 1;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x19, duel_atk_zone, duel_atk_tz, duel_atk_tp);
        return 1;
    case 10:
        if (duel_w_0891968C != 0) {
            duel_atk_direct = 1;
            duel_w_08919610 = 5;
            return 0;
        }
        duel_088328E8(q, 0x139C, duel_atk_zone);
        duel_w_08919610++;
        return 0;
    case 11:
        duel_atk_tz = duel_w_0891968C >> 8 & 0xFF;
        duel_w_08919610 = 5;
        return 0;
    }
    return 1;
}
