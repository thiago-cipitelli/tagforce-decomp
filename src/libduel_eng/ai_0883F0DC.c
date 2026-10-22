// @range 0x0883F0DC-0x088400B0
// @asm 0x0883F3A0-0x0883FA64
/* Duel events run by the computer player: step tables and card checks. */
#include "duel.h"

extern int (*duel_ev_funcs[])(void);   /* 0x089124AC */
extern unsigned int duel_w_0891960C;
extern int duel_w_08919610;
extern int duel_ev_unk;

void duel_088C8E88(int q, int sz);
void duel_088C7D74(unsigned int bit, unsigned int on);
void duel_088C7DD4(unsigned int p, unsigned int bit, unsigned int on);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_count_monsters(unsigned int p);
int duel_count_field_all(unsigned int p);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_find_effect_card(unsigned int p, int id, int skip);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088C8AD0(unsigned int p, int z, int id, int v);
unsigned int duel_slot_effect12_arg(unsigned int p, int z, int id);
int duel_slot_has_effect12_arg(unsigned int p, int z, int id, int arg);
void duel_088C44E4(unsigned int p, int id);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088C7BA4(void);
void duel_088CAB04();
void duel_088C7CF0(void);
void duel_08835944(int a);
int duel_088C5510(unsigned int p, int n, int f);
void duel_088AFD5C(int kind, unsigned int a);
void duel_0884ACD8(unsigned int p, int msg);
extern int duel_player_type[2];
extern unsigned int duel_turn_player;
extern int duel_w_08919600;
extern int duel_w_08919638;
extern int duel_w_08919640;
extern int duel_w_0891963C;
extern int duel_w_089147D8;
extern unsigned int duel_w_08919608;

int duel_0883F0DC(void)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 5; z < 10; z++) {
            if (duel.players[p & 1].field[z].card.id == 0x13F9) {
                duel_088C8E88(p, z);
            }
        }
    }
    return 1;
}

int duel_0883F174(void)
{
    if (duel_ev_funcs[duel_w_0891960C] != 0) {
        if (duel_ev_funcs[duel_w_0891960C]() != 0) {
            duel_w_08919610 = 0;
            duel_w_0891960C++;
        }
        return 0;
    }
    return 1;
}

int duel_0883F1DC(unsigned int p, int flag)
{
    switch (duel_ev_unk) {
    case 0:
        duel_w_0891960C = 6;
        duel_w_08919610 = 1;
        duel_088C7D74(0, 1);
        duel_088C7DD4(p, 0x13, 1);
        if (flag != 0) {
            duel_cmd((duel_special.player[0] != 0 ? 0x8000 : 0) | 0x1B, duel_special.zone[0], 1, 0);
        }
        duel_special.padc[2] = 1;
        duel_ev_unk++;
        break;
    case 1:
        duel_0883F174();
        return duel_w_0891960C > 8;
    }
    return 0;
}

int duel_0883F2CC(unsigned int p)
{
    if (duel_slot_has_effect(p, 0xB, 0x14FF) == 0) {
        return 0;
    }
    if (duel.players[p & 1].n_hand != 0) {
        return 0;
    }
    if (duel_count_monsters(p) != 0) {
        return 0;
    }
    if (duel_count_field_all(p) != 0) {
        return 0;
    }
    p = 1 - p;
    if (duel_hand_count_name(p, 0x14FF) == 0 && duel_find_effect_card(p, 0x14FF, -1) == 0) {
        return 0;
    }
    return 1;
}

#ifdef NON_MATCHING /* registradores: o original guarda (p & 1) * 0xFAC em s1 para todos os casos */
int duel_0883F3A0(void)
{
    unsigned int p = duel_turn_player;
    unsigned int f = duel.players[p & 1].flags;
    unsigned int b24 = f >> 24 & 1;
    int prot = (f & 1) || b24;
    int i;
    unsigned int q;
    int n;
    unsigned int v;
    int z;

    switch (duel_w_08919600) {
    case 0:
        if (prot | duel_slot_has_effect(p, 0xB, 0x14FF) | duel_slot_has_effect(p, 0xB, 0x1548)) {
            if (duel_player_type[p] == 1 && duel_0883F2CC(p) != 0) {
                duel_cmd((p != 0 ? 0x8000 : 0) | 0x23, 5, 0, 0);
            } else if (b24) {
                duel_0884ACD8(p, 0x132);
            } else {
                duel_0884ACD8(p, 0x133);
            }
            duel_088C7DD4(p, 0, 0);
            return 1;
        }
        duel_cmd((p != 0 ? 0x8000 : 0) | 0xC, 0, 0, 0);
        duel_cmd((p != 0 ? 0x8000 : 0) | 0x28, 0xD, 0, 0);
        duel_w_08919600++;
        return 0;
    case 1:
        if (duel.players[p & 1].n_hand == 0 && duel_find_effect_card(p, 0x1911, -1) != 0) {
            duel_088C77D4(p, 0x1911, 1, 0);
        }
        for (i = 0; i < 2; i++) {
            q = p ^ !i;
            if (duel_slot_has_effect(q, 0xB, 0x1504) != 0) {
                duel_088C44E4(1 - q, 0x1504);
                duel_088C7BA4();
                for (n = duel.players[q & 1].n_hand - 1; n >= 0; n--) {
                    duel_088CAB04(q, n, 1);
                }
                duel_088C7CF0();
            }
        }
        duel_w_08919600++;
        return 0;
    case 2:
        v = duel_slot_effect12_arg(p, 0xB, 0x14FD);
        if ((int)v >= 0) {
            duel_0884F30C((p << 31 & 0x80000000) | 0x25014FD, v, 0);
            duel_088C8AD0(p, 0xB, 0x14FD, v);
            return 0;
        }
        if (duel_slot_has_effect12_arg(p, 0xB, 0x1885, 0) != 0) {
            duel_088C44E4(1 - p, 0x1885);
            duel_w_08919600 = 7;
            return 0;
        }
        duel_w_08919600++;
    case 3:
        if (duel.players[p & 1].n_d != 0) {
            for (z = 0; z < 5; z++) {
                if (duel_slot_e8170(p, z, 0x14C4) != 0) {
                    duel_0884F30C((p << 31 & 0x80000000) | 0x200000 | (z << 16 & 0x1F0000) | 0x14C4,
                                  CARD_UID(duel.players[p & 1].field[z].card), 0);
                }
            }
            for (z = 0; z < 5; z++) {
                if (duel_slot_e8170(p, z, 0x1A97) != 0) {
                    duel_0884F30C((p << 31 & 0x80000000) | 0x200000 | (z << 16 & 0x1F0000) | 0x1A97,
                                  CARD_UID(duel.players[p & 1].field[z].card), 0);
                }
            }
            for (n = duel.players[p & 1].n_grave - 1; n >= 0; n--) {
                if (duel.players[p & 1].grave[n].id == 0x1984) {
                    duel_0884F30C((p << 31 & 0x80000000) | 0x4E1984, CARD_UID(duel.players[p & 1].grave[n]), 0);
                }
            }
        }
        duel_w_08919600++;
        return 0;
    case 4:
        if (duel_slot_has_effect(p, 0xB, 0x14C4) != 0) {
            duel_w_08919600 += 3;
        } else if (duel_player_type[p] == 1) {
            duel_w_08919600 += 2;
        } else {
            duel_08835944(1);
            duel_w_08919600++;
        }
        return 0;
    case 5:
        if (duel_w_08919638 == 0) {
            duel_w_08919600--;
        } else if (duel_w_08919640 == 0xC) {
            duel_w_08919600++;
        } else {
            duel_w_0891963C = 1;
            duel_w_08919600--;
        }
        return 0;
    case 6:
        duel_088C5510(p, 1, 0);
        duel_w_08919600++;
        return 0;
    case 7:
        duel_088AFD5C(1, 0);
        duel_w_089147D8 = 0;
        duel_w_08919600++;
        return 0;
    case 8:
        if (duel_w_089147D8 == 0) {
            duel_w_08919600++;
        } else {
            duel_w_08919600--;
        }
        return 0;
    }
    return 1;
}
#endif

int duel_0883FA64(unsigned int p, int id)
{
    int z;

    for (; duel_w_08919608 < 5; duel_w_08919608++) {
        z = duel_w_08919608;
        if (duel_slot_e8170(p, z, id) != 0
            && duel_0884F30C((p << 31 & 0x80000000) | 0xA200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                             CARD_UID(duel.players[p & 1].field[z].card), 0) != 0) {
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_0883FBA4(unsigned int p, int id)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        if (duel_slot_e8170(q, z, id) != 0
            && duel_0884F30C((q << 31 & 0x80000000) | 0xA200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                             CARD_UID(duel.players[q & 1].field[z].card), 0) != 0) {
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_0883FD00(unsigned int p, int id)
{
    int z;

    for (; duel_w_08919608 < 5; duel_w_08919608++) {
        z = duel_w_08919608 + 5;
        if (duel_slot_e8170(p, z, id) != 0
            && duel_0884F30C((p << 31 & 0x80000000) | 0xA200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                             CARD_UID(duel.players[p & 1].field[z].card), 0) != 0) {
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_0883FE44(int id)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = duel_turn_player;
        q ^= i / 5;
        z = i % 5 + 5;
        if (duel_slot_e8170(q, z, id) != 0
            && duel_0884F30C((q << 31 & 0x80000000) | 0xA200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                             CARD_UID(duel.players[q & 1].field[z].card), 0) != 0) {
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_0883FFA0(unsigned int p, unsigned int id)
{
    int i;
    unsigned int q;
    int v;

    for (i = 0; i < 2; i++) {
        q = p ^ i;
        v = duel_slot_effect_arg(q, 0xB, id);
        if (v > 0) {
            duel_0884F30C((q << 31 & 0x80000000) | 0xA4E0000 | (id & 0xFFFF), v, 0);
            duel_088C89BC(q, 0xB, id, 0);
            return 0;
        }
    }
    return 1;
}

int duel_08840050(unsigned int p)
{
    return duel_0883FA64(p, 0x1756);
}

int duel_08840058(unsigned int p)
{
    return duel_0883FA64(p, 0x16C1);
}

int duel_08840060(unsigned int p)
{
    return duel_0883FA64(p, 0x1AAD);
}

int duel_08840068(unsigned int p)
{
    return duel_0883FBA4(p, 0x12A0);
}

int duel_08840070(unsigned int p)
{
    return duel_0883FBA4(p, 0x1835);
}

int duel_08840078(unsigned int p)
{
    return duel_0883FBA4(p, 0x19C0);
}

int duel_08840080(unsigned int p)
{
    return duel_0883FBA4(p, 0x1643);
}

int duel_08840088(unsigned int p)
{
    return duel_0883FBA4(p, 0x17BC);
}

int duel_08840090(unsigned int p)
{
    return duel_0883FBA4(p, 0x1A1F);
}

int duel_08840098(unsigned int p)
{
    return duel_0883FBA4(p, 0x1A20);
}

int duel_088400A0(unsigned int p)
{
    return duel_0883FBA4(p, 0x1A21);
}

int duel_088400A8(unsigned int p)
{
    return duel_0883FBA4(p, 0x1A40);
}
