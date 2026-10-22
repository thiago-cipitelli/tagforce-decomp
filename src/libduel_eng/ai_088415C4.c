// @range 0x088415C4-0x088420F4
/* Computer player: responses with counters, effects kept in the player's effect list and hand checks. */
#include "duel.h"

typedef struct DuelAiPick {             /* 8 */
    int id;
    int msg;                            /* show the message 0xFA for a human player */
} DuelAiPick;

extern unsigned int duel_w_08919608;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_turn;
extern DuelAiPick duel_ai_picks_08907CBC[5];

int duel_0883FA64(unsigned int p, int id);
int duel_0883FE44(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_has_effect2(unsigned int p, int z, int id);
int duel_slot_effect_arg(unsigned int p, int z, int id);
unsigned int duel_slot_effect_kind_arg(unsigned int p, int z, int id, int kind);
int duel_slot_counter_kind(unsigned int p, int z, int id, int kind);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int skip);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
void duel_088C8A38(unsigned int p, int z, int id, int k, int c);
void duel_088C89BC(unsigned int p, int z, int id, int c);
unsigned int duel_card_locate(unsigned int uid);
int field_get_card(unsigned int p, int z);
int duel_0886B0E4(unsigned int a, int b, int c);
int duel_list_find_id(unsigned int p, unsigned int id);
void duel_0884AD40(int msg, int uid);
int duel_count_monsters(unsigned int p);
int duel_count_active(int id);
void duel_088C44E4(unsigned int p, int id);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_088C4730(unsigned int p, int n, int f, int id);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088C452C(unsigned int p, int n);
void duel_088326E8(unsigned int p, unsigned int id);

int duel_088415C4(unsigned int p)
{
    unsigned int q;
    int v;
    int n;

    for (; duel_w_08919608 < 2; duel_w_08919608++) {
        q = p ^ duel_w_08919608;
        v = duel_slot_effect_kind_arg(q, 0xB, 0x14E2, 1);
        if (v > 0 && duel_find_effect_card(1 - q, 0x178B, -1) == 0) {
            n = duel_slot_counter_kind(q, 0xB, 0x14E2, 1);
            n *= duel_slot_counter_kind(q, 0xB, 0x14E2, 2);
            if (n != 0) {
                duel_0884F30C((q << 31 & 0x80000000) | 0xA5014E2, v, n);
                duel_088C8A38(q, 0xB, 0x14E2, 2, 1);
                return 0;
            }
        }
    }
    return 1;
}

int duel_088416EC(unsigned int p)
{
    unsigned int q;
    int v;

    for (; duel_w_08919608 < 2; duel_w_08919608++) {
        q = p ^ duel_w_08919608;
        v = duel_slot_effect_arg(q, 0xB, 0x13A6);
        if (v > 0) {
            if ((unsigned char)((duel_card_locate(v) & 0xFFFF) >> 8) == 0xE) {
                duel_0884F30C((q << 31 & 0x80000000) | 0xA4E13A6, v, 0);
            }
            duel_088C89BC(q, 0xB, 0x13A6, 0);
            return 0;
        }
    }
    return 1;
}

int duel_088417C8(void)
{
    return duel_0883FE44(0x17B2);
}

int duel_088417D0(unsigned int p)
{
    return duel_0883FA64(p, 0x12A3);
}

int duel_088417D8(void)
{
    return duel_0883FE44(0x12DC);
}

int duel_088417E0(void)
{
    return duel_0883FE44(0x17B6);
}

int duel_088417E8(void)
{
    return duel_0883FE44(0x1972);
}

int duel_088417F0(void)
{
    return duel_0883FE44(0x1802);
}

int duel_088417F8(unsigned int p)
{
    unsigned int i;
    unsigned int q;
    int z;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5 + 5;
        if (!duel_slot_e8170(q, z, 0x17F8)) {
            continue;
        }
        duel_0884F30C((q << 31 & 0x80000000) | 0xA000000 | ((q ^ p) << 24 & 0x1000000) | 0x200000
                          | (z << 16 & 0x1F0000) | (field_get_card(q, z) & 0xFFFF),
                      CARD_UID(duel.players[q & 1].field[z].card), 0);
        duel_w_08919608++;
        return 0;
    }
    return 1;
}

int duel_0884196C(unsigned int p)
{
    unsigned int i;
    int id;

    if (duel_w_08919608 < 5) {
        for (; (i = duel_w_08919608) < 5; duel_w_08919608++) {
            id = duel_ai_picks_08907CBC[i].id;
            if (!duel_0886B0E4((p << 31 & 0x80000000) | 0xA500000 | (id & 0xFFFF), 0, 0)) {
                continue;
            }
            if (duel_player_type[p] == 0 && duel_ai_picks_08907CBC[i].msg != 0) {
                duel_0884AD40(0xFA, duel_list_find_id(p, id));
            } else {
                duel_w_08919624 = 1;
            }
            duel_w_08919608 += 5;
            return 0;
        }
    } else {
        i = duel_w_08919608 - 5;
        if (duel_w_08919624 != 0) {
            duel_0884F30C((p << 31 & 0x80000000) | 0xA500000 | (unsigned short)duel_ai_picks_08907CBC[i].id, 0, 0);
        } else {
            duel_088C89BC(p, 0xB, duel_ai_picks_08907CBC[i].id, 0);
            duel_w_08919608++;
        }
        duel_w_08919608 -= 5;
        return duel_w_08919608 >= 5;
    }
    return 1;
}

int duel_08841B54(unsigned int p)
{
    if (duel_slot_has_effect(p, 0xB, 0x151E) != 0 && duel_count_monsters(p) != 0 && duel_count_monsters(1 - p) == 0) {
        duel_088C44E4(p, 0x151E);
        duel_088C77D4(p, 0x151E, 5, 0);
        return 0;
    }
    return 1;
}

int duel_08841BD8(unsigned int p)
{
    if (duel_slot_has_effect(p, 0xB, 0x151E) != 0) {
        duel_088C44E4(p, 0x151E);
        duel_088C77D4(p, 0x151E, 5, 0);
        return 0;
    }
    return 1;
}

int duel_08841C30(unsigned int p)
{
    int v;

    if (duel_slot_has_effect(p, 0xB, 0x18FE) != 0) {
        v = duel_slot_effect_arg(p, 0xB, 0x18FE);
        duel_088C44E4(p, 0x18FE);
        duel_088C4730(p, v, 0, 0x18FE);
        duel_088C89BC(p, 0xB, 0x18FE, 0);
        return 0;
    }
    return 1;
}

int duel_08841CB8(unsigned int p)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        if (duel.players[q & 1].field[z].card.id != 0 && duel_slot_has_effect(q, z, 0x1A74) != 0) {
            duel_088C89BC(q, z, 0x1A74, 1);
            duel_088C89BC(q, z, 0x1419, 1);
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08841DD8(unsigned int p, int id, int n)
{
    unsigned int e;
    DuelEffectBits *ef;

    if (duel_slot_has_effect(p, 0xB, id) != 0) {
        e = duel.players[p & 1].effects;
        while (e != 0) {
            ef = (DuelEffectBits *)&duel.effects[e];
            e = ef->next;
            if (ef->kind == 1 && ef->src == id) {
                duel_cmd((p != 0 ? 0x8000 : 0) | 0x3B, id, 1, n - ((unsigned int)ef->kind_hi >> 1));
            }
        }
    }
    return 1;
}

int duel_08841ED0(unsigned int p)
{
    return duel_08841DD8(p, 0x123B, 3);
}

int duel_08841EDC(unsigned int p)
{
    return duel_08841DD8(p, 0x188C, 3);
}

int duel_08841EE8(unsigned int p)
{
    return duel_08841DD8(p, 0x18D5, 2);
}

int duel_08841EF4(unsigned int p)
{
    int i;
    int v;
    int d;

    for (i = 0; i < 2; i++, p ^= 1) {
        v = duel_slot_effect_arg(p, 0xB, 0x169C);
        if (v >= 0) {
            d = duel_turn + 1 - v;
            duel_cmd((p != 0 ? 0x8000 : 0) | 0x3B, 0x169C, 1, d);
            if (d >= 20) {
                duel_088C77D4(p, 0x169C, 1, 1);
            }
        }
    }
    return 1;
}

int duel_08841FAC(unsigned int p)
{
    if (duel_slot_has_effect2(p, 0xB, 0x143D) != 0) {
        duel_088C44E4(p, 0x143D);
        duel_088C452C(p, (duel.players[p & 1].lp + 5) / 10);
    }
    return 1;
}

int duel_08842038(unsigned int p)
{
    unsigned int n;

    if (duel_count_active(0x1401) == 0) {
        n = 6;
        if (duel_slot_has_effect(p, 0xB, 0x159F) != 0) {
            n = 7;
        }
        if (duel_find_effect_card(1 - p, 0x1800, -1) != 0) {
            n = 5;
        }
        if (n < duel.players[p & 1].n_hand) {
            duel_088326E8(p, duel.players[p & 1].n_hand - n);
            return 0;
        }
    }
    return 1;
}
