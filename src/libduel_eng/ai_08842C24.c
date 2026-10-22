// @range 0x08842C24-0x08843858
// @asm 0x08842D98-0x08843344
// @data 0x08907E18-0x08907E38
/* Computer player: steps of the standby, main and battle phases (duel_w_08919600 / duel_w_08919618). */
#include "duel.h"

extern unsigned int duel_turn_player;
extern unsigned int duel_turn;
extern int duel_phase;
extern int duel_player_type[2];
extern int duel_w_08919600;
extern int duel_w_089195D8;
extern int duel_w_089195E0;
extern int duel_w_089195E8;
extern unsigned int duel_w_0891960C;
extern int duel_w_08919610;
extern int duel_w_08919614;
extern int duel_w_08919618;
extern int duel_w_08919624;
extern int duel_w_08919638;
extern int duel_w_0891963C;
extern int duel_w_08919640;
extern unsigned int duel_w_08913CF4;

int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088C7DD4(unsigned int p, unsigned int bit, unsigned int on);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_bzero(void *p, int n);
int field_get_card(unsigned int p, int z);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_0886B0E4(unsigned int a, unsigned short uid, int c);
void duel_088AFD5C(int kind, unsigned int a);
int duel_0883F174();
int duel_08836868(unsigned int p);
int duel_0883698C(unsigned int p);
int duel_088369D8(unsigned int p);
int duel_08805248(void);
int duel_0884AF20(int a);
void duel_0884ACD8(unsigned int p, int msg);
void duel_08835944(int a);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
int duel_088BFFD8(unsigned int p, int id, int i);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
int duel_can_attack(unsigned int p, int z, int arg);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);

int duel_08842C24(void)
{
    unsigned int p = duel_turn_player;

    switch (duel_w_08919600) {
    case 0:
        duel_cmd((p != 0 ? 0x8000 : 0) | 2, 0, 0, 0);
        if (duel_player_type[p] == 1) {
            duel_bzero(&duel_w_08913CF4, 0x21C);
        }
        duel_w_08919600++;
        return 0;
    case 1:
        if (duel_slot_has_effect(p, 0xB, 0x1356) != 0) {
            duel_088C7DD4(p, 0x18, 1);
        }
        if (duel_w_089195E8 != 1 && duel_turn == duel_w_089195D8 + 1) {
            duel_cmd((p != 0 ? 0x8000 : 0) | 0xB, 1, 0, 0);
        }
        if (duel_turn == duel_w_089195D8 + 4) {
            duel_cmd((p != 0 ? 0x8000 : 0) | 0xB, 2, 0, 0);
        }
        break;
    }
    return 1;
}

#ifdef NON_MATCHING /* registradores: o original guarda (p & 1) * 0xFAC em s1 e testa duel_w_08919640 num switch */
int duel_08842D98(void)
{
    unsigned int p = duel_turn_player;
    unsigned int b24 = duel.players[p & 1].flags >> 24 & 1;
    int has = duel_slot_has_effect(p, 0xB, 0x13B1);
    int z;
    int id;
    int m;
    int n;

    switch (duel_w_08919600) {
    case 0:
        duel_w_08919614 = 0;
        if (b24) {
            return 1;
        }
        if (has) {
            duel_w_08919600 = 10;
            return 0;
        }
        duel_cmd((p != 0 ? 0x8000 : 0) | 0xE, 0, 0, 0);
        duel_w_08919600++;
        duel_w_08919638 = 0;
        return 0;
    case 1:
        duel_w_08919600++;
    case 2:
        if (duel_player_type[p] == 1) {
            if (duel_08805248() != 0) {
                duel_w_08919600 = 0x1E;
            }
            return 0;
        }
        duel_08835944(2);
        duel_w_08919600++;
        return 0;
    case 3:
        if (duel_w_08919638 != 0) {
            duel_w_0891963C = 1;
            duel_w_08919600 = 2;
            return 0;
        }
        if (duel_w_08919640 == 0x12) {
            if (duel_088369D8(p) != 0) {
                duel_w_08919600 = 0x1E;
                return 0;
            }
        } else if (duel_w_08919640 == 0x10 && duel_08836868(p) != 0) {
            duel_w_08919600 = 0x15;
            duel_w_0891960C = 0;
            duel_w_08919614 = 0;
            return 0;
        }
        m = duel_088369D8(p) != 0 ? 0x20 : 0;
        n = duel_08836868(p) != 0 ? 8 : 0;
        duel_0884AF20(1 << duel_phase | n | m);
        duel_w_08919600 = 0x14;
        return 0;
    case 0xA:
        if (duel_player_type[p] == 1) {
            if (duel_08836868(p) != 0) {
                duel_w_08919624 = 3;
            } else {
                duel_w_08919624 = 5;
            }
            duel_w_08919600 = 0x14;
        } else {
            duel_0884ACD8(p, 0x135);
            duel_w_08919600++;
        }
        return 0;
    case 0xB:
        m = duel_0883698C(p) == 0 && duel_08836868(p) != 0 ? 8 : 0;
        n = duel_088369D8(p) != 0 ? 0x20 : 0;
        duel_0884AF20(n | m);
        duel_w_08919600 = 0x14;
        return 0;
    case 0x14:
        if (duel_w_08919624 == 5) {
            duel_w_08919600 = 0x1E;
            return 0;
        }
        if (duel_w_08919624 == 3 && duel_08836868(p) != 0) {
            duel_w_08919614 = 0;
            duel_w_0891960C = 0;
            duel_w_08919600++;
            return 0;
        }
        duel_w_08919600 = 2;
        return 0;
    case 0x15:
        if (duel_phase == 2 || duel_phase == 4) {
            duel_088AFD5C(3, 0);
        }
        duel_w_08919600++;
        return 0;
    case 0x16:
        if (duel_0883F174(p) == 0) {
            return 0;
        }
        if (duel_w_08919614 == 0) {
            duel_cmd((p != 0 ? 0x8000 : 0) | 0x10, 0, 0, 0);
            duel_w_08919600 = 2;
            return 0;
        }
        return 1;
    case 0x1E:
        if (duel_phase == 2 || duel_phase == 4) {
            for (z = 0; z < 5; z++) {
                id = field_get_card(p, z);
                if (id == 0x1954 && duel.players[p & 1].field[z].faceup != 0
                    && duel_0884F30C((p << 31 & 0x80000000) | 0x600000 | (z << 16 & 0x1F0000) | 0x1954,
                                     CARD_UID(duel.players[p & 1].field[z].card), 0) != 0) {
                    return 0;
                }
            }
            duel_088AFD5C(4, 0);
            return 1;
        }
        return 1;
    }
    return 0;
}
#endif

int duel_08843344(unsigned int p)
{
    int z;
    DuelPlayer *pl = &duel.players[p & 1];
    int id;
    unsigned int m = (p << 31 & 0x80000000) | 0x600000;

    for (z = 0; z <= 4; z++) {
        id = field_get_card(p, z);
        switch (id) {
        case 0x1954:
            if (pl->field[z].faceup != 0
                && duel_0886B0E4(m | ((unsigned int)z << 16 & 0x1F0000) | (id & 0xFFFF),
                                 CARD_UID(pl->field[z].card), 0) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

int duel_08843440(unsigned int p)
{
    int z;
    DuelPlayer *pl = &duel.players[p & 1];

    for (z = 0; z <= 4; z++) {
        if (pl->field[z].card.id != 0 && duel_slot_effect_arg(p, z, 0x151E) == 1) {
            return z;
        }
    }
    return -1;
}

int duel_088434D8(unsigned int p)
{
    int z;
    DuelPlayer *pl = &duel.players[p & 1];

    for (z = 0; z <= 4; z++) {
        if (pl->field[z].card.id != 0 && duel_slot_effect_arg(p, z, 0x151E) == 0) {
            return z;
        }
    }
    return -1;
}

int duel_0884356C(void)
{
    unsigned int p = duel_turn_player;
    int a;
    int b;
    unsigned int q;

    switch (duel_w_08919618) {
    case 0:
        duel_w_089195E0 = duel_phase;
        duel_w_08919618++;
        return 0;
    case 1:
        if (duel_count_free_monster_zones(p) != 0 && duel_phase_ok2(p) != 0 && duel_088BFFD8(p, 0x151E, 0) != 0) {
            duel_0884ACD8(p, 0x11D);
            duel_w_08919618++;
        } else {
            duel_w_08919618 = 8;
        }
        return 0;
    case 2:
        duel_0884C588(p, 6, 0x151E, 0);
        duel_w_08919618++;
        return 0;
    case 3:
        duel_0882619C(p, duel_0884C7CC(), 1, 0, 0x151E0000);
        duel_w_08919618++;
        return 0;
    case 4:
        a = duel_08843440(p);
        q = 1 - p;
        b = duel_088434D8(q);
        if (a >= 0) {
            if (b >= 0 && duel_can_attack(p, a, 1) != 0 && duel_can_attack_target(p, a, q, b, 1) != 0) {
                duel_w_08919618++;
                return 0;
            }
        }
        return 1;
    case 5:
        if (duel_w_089195E0 == 3) {
            duel_w_0891960C = 1;
            duel_w_08919610 = 0;
        } else {
            duel_w_0891960C = 0;
            duel_w_08919610 = 0;
        }
        duel_w_08919618++;
        return 0;
    case 6:
        if (duel_0883F174(p) != 0) {
            duel_w_08919618++;
        }
        return 0;
    case 7:
        if (duel_w_089195E0 != 3) {
            duel_cmd((duel_w_089195E0 + 0xC) | (p != 0 ? 0x8000 : 0), 0, 0, 0);
        }
        duel_w_08919618++;
        return 0;
    }
    return 1;
}

int duel_08843844(void)
{
    return duel_w_08919618 == 6;
}
