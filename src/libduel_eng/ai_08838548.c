// @range 0x08838548-0x08839360
// @asm 0x08838A84-0x08839360
/* Battle: effects at the attack declaration (cards that end the battle phase or deal damage). */
#include "duel.h"

extern unsigned int duel_w_0891964C;
extern int duel_w_0891960C;
extern int duel_w_08919610;
extern unsigned int duel_w_08919624;
extern int duel_w_08919638;
extern int duel_w_08919640;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned short duel_h_0891968E;
extern int duel_player_type[2];
extern unsigned int duel_atk_tp;       /* 0x08913F14 */
extern int duel_atk_w1C;               /* 0x08913F1C */
extern int duel_atk_w20;               /* 0x08913F20 */
extern int duel_atk_zone;              /* 0x08913F2C */
extern int duel_atk_snap[17];          /* 0x08913FFC */
extern int duel_atk_direct;            /* 0x08913F18 */
extern int duel_atk_w28;               /* 0x08913F28 */
extern int duel_atk_tz;                /* 0x08913F30 */
extern unsigned int duel_view_player;
extern int duel_w_089147D8;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_can_attack(unsigned int p, int z, int arg);
void duel_08836ED0(int *s);
int field_get_card(unsigned int p, int z);
void duel_088C4564(unsigned int p, int n);
void duel_0884AF88(int msg);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0881956C(unsigned int p, int id, int (*fn)());
void duel_08835978(int (*fn)());
void duel_088D1AC8(unsigned int p, int z);
int duel_08837020(void);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_088C45B4(unsigned int p, int n);
unsigned int duel_slot_immunity(unsigned int p, int z);
int duel_find_effect_card(unsigned int p, int id, int z);
void duel_088C448C(int p, int a);
void duel_088C625C(unsigned int p, int a, int b);
int duel_effect_applies(int id, unsigned int p, unsigned int ref);
int duel_count_active(int id);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int card_is_in_group_1a6f(int id);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
void duel_088C44E4(unsigned int p, int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088C7D74(int a, int b);
void duel_0884DB38(int a, int b);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
void duel_088AFD5C(int kind, unsigned int a);

int duel_08838548(unsigned int q, int z, int i)
{
    int k = z + i;

    if (k < 5 && duel_tributable(duel_special.player[0], q, k)) {
        if (q != duel_special.player[0] || duel_atk_zone != k) {
            return 0x800;
        }
    }
    return 0;
}

#define FIELD(T, f) (*(T *)((char *)&duel.players[0].field[0].f + (p & 1) * 0xFAC + duel_atk_zone * 20))

int duel_088385C8(unsigned int p)
{
    switch (duel_w_08919610) {
    case 0:
        if (duel_atk_w1C != 0) {
            return 1;
        }
        if (!duel_can_attack(p, duel_atk_zone, 1)) {
            duel_cmd((p ? 0x8000 : 0) | 0x1B, duel_atk_zone, 1, 0);
            duel_w_0891960C = 1;
            return 0;
        }
        duel_08836ED0(duel_atk_snap);
        duel_atk_w1C = 1;
        if (FIELD(unsigned short, faceup) & ~(FIELD(unsigned int, flags) >> 5 & 1) & ~(FIELD(unsigned int, flags) >> 1 & 1)) {
            switch (field_get_card(p, duel_atk_zone)) {
            case 0x127D:
            case 0x127E:
            case 0x127F:
            case 0x12A5:
                duel_088C4564(p, 500);
                return 1;
            case 0x11CD:
            case 0x18A4:
                duel_088C4564(p, 1000);
                return 1;
            case 0x128F:
            case 0x12A0:
                duel_w_08919610 = 100;
                return 0;
            case 0x1115:
                if (duel_player_type[p] == 1) {
                    duel_w_08919624 = 0;
                } else {
                    duel_0884AF88(0x38);
                }
                duel_w_08919610 = 200;
                return 0;
            }
        }
        return 1;
    case 100:
        duel_0884ACD8(p, 0xE0);
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, 1, duel_08838548);
        } else {
            duel_08835978(duel_08838548);
        }
        duel_w_08919610++;
        return 0;
    case 101:
        if (duel_w_08919638 != 0 && duel_w_08919640 == 0xB) {
            duel_088D1AC8(duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08919610++;
            return 0;
        }
        duel_w_08919610--;
        return 0;
    case 102:
        return !duel_08837020();
    case 200:
        duel_08832AF4(p, duel_atk_zone, 1, duel_w_08919624);
        duel_w_08919610++;
        return 0;
    case 201:
        if (duel_h_0891968E == 0) {
            duel_088C45B4(p, duel.players[p & 1].lp >> 1);
        }
        return 1;
    }
    return 1;
}

int duel_08838980(unsigned int p)
{
    unsigned int q = duel_atk_tp;
    unsigned int im = duel_slot_immunity(p, duel_atk_zone);
    int k;
    int n;
    int i;

    if (duel_atk_w20 != 0) {
        return 1;
    }
    duel_atk_w20 = 1;
    if ((int)(im & 2) <= 0) {
        k = duel_find_effect_card(q, 0x131D, -1);
        if (k > 0) {
            duel_088C448C(q, 0x131D);
            duel_088C625C(p, k, 1);
        }
    }
    n = duel_effect_applies(0x1320, p, im);
    if (n > 0) {
        duel_088C448C(-1, 0x1320);
        for (i = 0; i < n; i++) {
            duel_088C4564(p, 500);
        }
    }
    return 1;
}

#define FIELDQ(T, f, q, z) (*(T *)((char *)&duel.players[0].field[0].f + ((q) & 1) * 0xFAC + (z) * 20))

#ifdef NON_MATCHING /* alocação de registradores: o original reaproveita $s0 para q nas duas definições e usa ponteiro no segundo laço; aqui sobra um $s */
int duel_08838A84(unsigned int p)
{
    unsigned int q = duel_atk_tp;
    int z;
    int f;
    int id;

    switch (duel_w_08919610) {
    case 0:
        duel_08836ED0(duel_atk_snap);
        duel_w_08919610++;
    case 1:
        if (duel_count_active(0x1469)) {
            duel_088C77D4(p, 0x1469, 1, CARD_UID(duel.players[p & 1].field[duel_atk_zone].card));
        }
        duel_w_08919610++;
    case 2:
        q = 1 - p;
        for (z = 5; z < 10; z++) {
            if (duel.players[q & 1].field[z].card.id == 0x1381 && SLOT_ACTIVE(&duel.players[q & 1].field[z]) &&
                !duel_st_affects(p, duel_atk_zone, q, z)) {
                duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                duel_088C87A8(p, duel_atk_zone, (q & 0xFF) | (z & 0xFF) << 8, 7, 0);
            }
        }
        z = 0;
        if (duel_slot_has_effect(q, 0xB, 0x195F) && duel_count_monsters_fn(q, card_is_in_group_1a6f) &&
            !duel_slot_ref_immune(p, duel_atk_zone, 1)) {
            duel_088C44E4(q, 0x195F);
            duel_088C89BC(q, 0xB, 0x195F, 0);
            z = 1;
        }
        if (duel_atk_w28 == 0) {
            for (id = 0; id <= 4; id++) {
                if (duel_slot_e8170(q, id, 0x19A8) && duel.players[q & 1].field[id].pos == 0 &&
                    !duel_slot_has_effect(q, id, 0xFB6)) {
                    duel_088C4448(q, id, CARD_UID(duel.players[q & 1].field[id].card));
                    duel_088C87A8(q, id, 0xFB6, 2, 0);
                    z = 1;
                }
            }
        }
        if (z) {
            duel_cmd((p ? 0x8000 : 0) | 0x20, duel_atk_zone, 0, 0);
            return 0;
        }
        duel_w_08919610++;
        return 0;
    case 3:
        if (duel_08837020() != 0) {
            return 0;
        }
        if (duel_atk_w28 == 0) {
            duel_088C7D74(1, 1);
            if (FIELDQ(unsigned short, faceup, p, duel_atk_zone) & ~(FIELDQ(unsigned int, flags, p, duel_atk_zone) >> 5 & 1) &
                ~(FIELDQ(unsigned int, flags, p, duel_atk_zone) >> 1 & 1)) {
                switch (field_get_card(p, duel_atk_zone)) {
                case 0x18AB:
                case 0x18AC:
                case 0x18AD:
                    duel_088C4448(p, duel_atk_zone, CARD_UID(duel.players[p & 1].field[duel_atk_zone].card));
                    duel_088C77D4(1 - p, 0x18AB, 4, CARD_UID(duel.players[p & 1].field[duel_atk_zone].card));
                    break;
                }
            }
            if (p == duel_view_player) {
                duel_0884DB38(0xC, 1);
            }
        }
        f = 0;
        switch (field_get_card(p, duel_atk_zone)) {
        case 0x158D:
            if (duel_atk_w28 == 0) {
                f = 1;
            }
            break;
        case 0x154C:
            f = 1;
            break;
        case 0x1954:
            f = 1;
            break;
        case 0x1AA0:
            if (duel_atk_w28 == 0) {
                duel_088C87A8(p, duel_atk_zone, 0x1AA0, 2, 0);
            }
            break;
        }
        if (f) {
            duel_0884F30C(((p << 31 & 0x80000000) | 0x24200000 | (duel_atk_zone << 16 & 0x1F0000)) | (field_get_card(p, duel_atk_zone) & 0xFFFF),
                          CARD_UID(duel.players[p & 1].field[duel_atk_zone].card), duel_atk_w28 == 0);
        }
        if (duel_atk_direct == 0 && FIELDQ(unsigned short, faceup, q, duel_atk_tz)) {
            f = 0;
            switch (field_get_card(q, duel_atk_tz)) {
            case 0x1A78:
                f = 1;
                break;
            }
            if (f) {
                duel_0884F30C(((q << 31 & 0x80000000) | 0x24200000 | (duel_atk_tz << 16 & 0x1F0000)) | (field_get_card(q, duel_atk_tz) & 0xFFFF),
                              CARD_UID(duel.players[q & 1].field[duel_atk_tz].card), duel_atk_w28 == 0);
            }
        }
        for (z = 5; z < 10; z++) {
            id = duel.players[q & 1].field[z].card.id;
            switch (id) {
            case 0x13F9:
            case 0x153F:
            case 0x1931:
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    duel_0884F30C(((q << 31 & 0x80000000) | 0x24200000 | (z << 16 & 0x1F0000)) | (id & 0xFFFF),
                                  CARD_UID(duel.players[q & 1].field[z].card), duel_atk_w28 == 0);
                }
                break;
            }
        }
        duel_088AFD5C(0x12, duel_atk_w28 == 0);
        duel_w_089147D8 = 0;
        duel_w_08919610++;
        return 0;
    case 4:
        if (duel_08837020() != 0) {
            return 0;
        }
        if (duel_w_089147D8 == 0) {
            return 1;
        }
        duel_088AFD5C(0x13, 0);
        duel_w_089147D8 = 0;
        return 0;
    }
    return 1;
}
#endif
