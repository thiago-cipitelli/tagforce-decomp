// @range 0x088C004C-0x088C0CD0
// @asm 0x088C07FC-0x088C09C8
/* Effect helpers after the candidate lists (effects_088BC9E0.c): whether a card is in the chain, the
   active field spell, and loops over the monster or spell/trap zones holding a card. */
#include "duel.h"

int card_is_monster(int id);
int duel_count_monster_active_both(int id);
void duel_088CC718(int id);
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088CC1A0(unsigned int p, int z, unsigned int v);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int field_get_card(unsigned int p, int z);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
void duel_088CC178(unsigned int p, int z, unsigned int v);
int duel_slot_value_1909(unsigned int p, int z);
void duel_088CC1D0(unsigned int p, int z, int a);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
unsigned int duel_field_type(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_088E9150(int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);
extern unsigned int duel_turn_player;
extern unsigned int duel_w_089147E0;
extern unsigned int duel_w_08917DB8;
extern int duel_w_08917DC0;

int duel_088C004C(unsigned int p, unsigned int z, unsigned int uid)
{
    int i;
    DuelAct *a;

    for (i = 0; i < duel_chain.unk480; i++) {
        a = &duel_chain.acts[i];
        if (a->player == p && a->zone == z && a->uid == uid) {
            return 1;
        }
    }
    for (i = 0; i < duel_w_089147E0; i++) {
        a = &((DuelAct *)duel_chain.pad0)[i];
        if (a->player == p && a->zone == z && a->uid == uid) {
            return 1;
        }
    }
    return 0;
}

int duel_088C0120(void)
{
    int id = 0;
    int i;

    for (i = 0; i < 2; i++) {
        if (duel.players[i & 1].field[10].card.id != 0 && duel.players[i & 1].field[10].faceup != 0 &&
            !(duel.players[i & 1].field[10].flags >> 1 & 1) && !(duel_w_08917DB8 & 1 << (i * 16 + 10))) {
            id = duel.players[i & 1].field[10].card.id & 0xFFFF;
        }
    }
    if (id == 0 && duel_count_monster_active_both(0x13A2)) {
        id = 0x10F4;
    }
    if (duel_w_08917DC0 != id) {
        duel_088CC718(id);
        return 1;
    }
    return 0;
}

void duel_088C01F8(unsigned int p, int id, unsigned int v)
{
    int z;
    int hi;

    if (card_is_monster(id)) {
        z = 0;
        hi = 4;
    } else {
        z = 5;
        hi = 9;
    }
    for (; z <= hi; z++) {
        if (duel_slot_e8170(p, z, id)) {
            duel_088CC1A0(p, z, v);
        }
    }
}

void duel_088C02A4(int id, unsigned int v)
{
    int p;
    int z;
    int lo;
    int hi;

    if (card_is_monster(id)) {
        lo = 0;
        hi = 4;
    } else {
        lo = 5;
        hi = 9;
    }
    for (p = 0; p < 2; p++) {
        for (z = lo; z <= hi; z++) {
            if (duel_slot_e8170(p, z, id)) {
                duel_088CC1A0(p, z, v);
            }
        }
    }
}

int duel_088C0364(int id)
{
    int k;
    unsigned int q;
    int z;
    int lo;
    int hi;

    if (card_is_monster(id)) {
        lo = 0;
        hi = 4;
    } else {
        lo = 5;
        hi = 9;
    }
    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = lo; z <= hi; z++) {
            if (id == field_get_card(q, z) && duel.players[q & 1].field[z].faceup != 0 && duel.players[q & 1].field[z].link != 0) {
                duel_0884F30C((q << 31 & 0x80000000) | 0x200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                              CARD_UID(duel.players[q & 1].field[z].card), duel.players[q & 1].field[z].link);
                duel_088CC178(q, z, 0);
                return 1;
            }
        }
    }
    return 0;
}

void duel_088C050C(unsigned int p, int id)
{
    int z;
    int hi;

    if (card_is_monster(id)) {
        z = 0;
        hi = 4;
    } else {
        z = 5;
        hi = 9;
    }
    for (; z <= hi; z++) {
        if (duel_slot_e8170(p, z, id)) {
            duel_088C7E68(p, z, 4, 1);
        }
    }
}

void duel_088C05B0(int id)
{
    int p;
    int z;
    int lo;
    int hi;

    if (card_is_monster(id)) {
        lo = 0;
        hi = 4;
    } else {
        lo = 5;
        hi = 9;
    }
    for (p = 0; p < 2; p++) {
        for (z = lo; z <= hi; z++) {
            if (duel_slot_e8170(p, z, id)) {
                duel_088C7E68(p, z, 4, 1);
            }
        }
    }
}

int duel_088C0668(int id)
{
    int k;
    unsigned int q;
    int z;
    int lo;
    int hi;

    if (card_is_monster(id)) {
        lo = 0;
        hi = 4;
    } else {
        lo = 5;
        hi = 9;
    }
    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (z = lo; z <= hi; z++) {
            if (id == field_get_card(q, z) && duel.players[q & 1].field[z].faceup != 0 && duel.players[q & 1].field[z].flags >> 4 & 1) {
                duel_0884F30C((q << 31 & 0x80000000) | 0x200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                              CARD_UID(duel.players[q & 1].field[z].card), 0);
                duel_088C7E68(q, z, 4, 0);
                return 1;
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* o teste do bit 5 sai como beqz para o corpo seguido de b fim, que nem o if, nem o switch de um case, nem ifs aninhados reproduzem */
void duel_088C07FC(unsigned int p, int z, int c)
{
    int q;
    int k;

    if (duel.players[p & 1].field[z].flags >> 4 & 1 && duel_slot_value_1909(p, z) < 4) {
        duel_088CC1D0(p, z, 1);
        if (c == 0 && !(duel.players[p & 1].field[z].flags >> 5 & 1)) {
            duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
            for (q = 0; q < 2; q++) {
                for (k = 0; k < 5; k++) {
                    if (duel.players[q & 1].field[k].card.id != 0 && duel.players[q & 1].field[k].faceup != 0 &&
                        (unsigned short)duel_field_type(q, k) == 2 && duel_st_affects(q, k, p, z)) {
                        duel_088C87A8(q, k, (unsigned char)p | (unsigned char)z << 8, 7, 0);
                    }
                }
            }
        }
    }
}
#endif

void duel_088C09C8(unsigned int p, int c)
{
    int q;
    int z;
    int on;

    for (q = 0; q < 2; q++) {
        for (z = 0; z < 5; z++) {
            if (field_get_card(q, z) == 0x1009 && duel.players[q & 1].field[z].faceup != 0) {
                if (duel_088E9150(0xFF9)) {
                    on = !(duel.players[q & 1].field[z].flags >> 5 & 1);
                    if (on && !(duel.players[q & 1].field[z].flags >> 4 & 1)) {
                        duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                        duel_088C7E68(q, z, 4, 1);
                    }
                    if (q == p && duel_slot_value_1909(q, z) < 4) {
                        duel_088CC1D0(q, z, 1);
                        if (on && c == 0) {
                            duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                            duel_088C87A8(q, z, 0x1009, 3, 0);
                        }
                    }
                } else {
                    duel_088C7E68(q, z, 4, 0);
                    duel_088C89BC(q, z, 0x1009, 1);
                    duel_088CC1D0(q, z, 0);
                }
            }
        }
    }
}

void duel_088C0BB0(unsigned int p, int z)
{
    int k;
    unsigned int q;
    int i;

    for (k = 0; k < 2; k++) {
        q = duel_turn_player ^ k;
        for (i = 0; i < 5; i++) {
            if ((q != p || i != z) && duel_slot_e8170(q, i, 0x104C)) {
                duel_0884F30C((q << 31 & 0x80000000) | 0x200000 | ((unsigned int)i << 16 & 0x1F0000) | 0x104C, CARD_UID(duel.players[q & 1].field[i].card), 0);
            }
        }
    }
}
