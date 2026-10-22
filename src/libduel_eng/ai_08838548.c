// @range 0x08838548-0x08838A84
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
