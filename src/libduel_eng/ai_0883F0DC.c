// @range 0x0883F0DC-0x088400B0
// @rodata 0x08907C90-0x08907CB4
/* Duel events run by the computer player: step tables and card checks. */
#include "duel.h"

extern int (*duel_ev_funcs[])();   /* 0x089124AC; each step takes the player */
extern unsigned int duel_w_0891960C;
extern int duel_w_08919610;
extern int duel_ev_unk;

void duel_088C8E88(int q, int sz);
void duel_088C7D74(unsigned int, unsigned int);
void duel_088C7DD4(unsigned int, unsigned int, unsigned int);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_count_monsters(unsigned int p);
int duel_count_field_all(unsigned int p);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_find_effect_card(unsigned int p, int id, int skip);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
unsigned int duel_slot_effect_arg(unsigned int, int, int);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088C8AD0(unsigned int, int, int, int);
unsigned int duel_slot_effect12_arg(unsigned int, int, int);
int duel_slot_has_effect12_arg(unsigned int p, int z, int id, int arg);
void duel_088C44E4(unsigned int p, int id);
void duel_088C77D4(unsigned int p, unsigned short id, unsigned short a, unsigned short b);
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

/* p is not read here: it stays in a0 for the step function (duel_08837500(p) is one of them) */
int duel_0883F174(p)
unsigned int p;
{
    if (duel_ev_funcs[duel_w_0891960C] != 0) {
        if (duel_ev_funcs[duel_w_0891960C](p) != 0) {
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
#ifdef __MWERKS__
        duel_0883F174();   /* p is still in a0 */
#else
        duel_0883F174(p);
#endif
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

/* Shape notes (mwcc): v is int; prot as one `||`; the two field loops of case 3 share the counter m, and the
   player pointer is written out at each use (PLO) instead of living in a local: with a `pl` variable, or with a
   counter per loop, the loop counters/pointers and the invariant event word land in other registers. */
#define PFO(T, f) (*(T *)((char *)&duel.players[0].f + ofs))
#define PLO ((DuelPlayer *)((char *)duel.players + ofs))
int duel_0883F3A0(void)
{
    unsigned int q;
    unsigned int p;
    int ofs;
    int v;
    int j;
    unsigned int b24;
    int m;
    unsigned int prot;
    int z;
    unsigned int f;

    p = duel_turn_player;
    ofs = (p & 1) * 0xFAC;
    f = PFO(unsigned int, flags);
    b24 = f >> 24 & 1;
    prot = (f & 1) != 0 || b24 != 0;
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
        if (PFO(unsigned int, n_hand) == 0 && duel_find_effect_card(p, 0x1911, -1) != 0) {
            duel_088C77D4(p, 0x1911, 1, 0);
        }
        for (j = 0; j < 2; j++) {
            q = p ^ !j;
            if (duel_slot_has_effect(q, 0xB, 0x1504) != 0) {
                duel_088C44E4(1 - q, 0x1504);
                duel_088C7BA4();
                for (z = duel.players[q & 1].n_hand - 1; z >= 0; z--) {
                    duel_088CAB04(q, z, 1);
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
        if (PFO(int, n_d) != 0) {
            for (m = 0; m <= 4; m++) {
                if (duel_slot_e8170(p, m, 0x14C4) != 0) {
                    duel_0884F30C((p << 31 & 0x80000000) | 0x200000 | ((unsigned int)m << 16 & 0x1F0000) | 0x14C4,
                                  CARD_UID(PLO->field[m].card), 0);
                }
            }
            for (m = 0; m <= 4; m++) {
                if (duel_slot_e8170(p, m, 0x1A97) != 0) {
                    duel_0884F30C((p << 31 & 0x80000000) | 0x200000 | ((unsigned int)m << 16 & 0x1F0000) | 0x1A97,
                                  CARD_UID(PLO->field[m].card), 0);
                }
            }
            for (m = PFO(unsigned int, n_grave) - 1; m >= 0; m--) {
                if (PLO->grave[m].id == 0x1984) {
                    duel_0884F30C((p << 31 & 0x80000000) | 0x4E1984, CARD_UID(PLO->grave[m]), 0);
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
        if (duel_w_08919638 != 0) {
            if (duel_w_08919640 == 0xC) {
                duel_w_08919600++;
                return 0;
            }
            duel_w_0891963C = 1;
            duel_w_08919600--;
            return 0;
        }
        duel_w_08919600--;
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
        if (duel_w_089147D8 != 0) {
            duel_w_08919600--;
            return 0;
        }
        duel_w_08919600++;
        return 0;
    }
    return 1;
}

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
