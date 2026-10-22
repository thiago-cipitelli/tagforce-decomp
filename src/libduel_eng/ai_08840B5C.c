// @range 0x08840B5C-0x088415C4
/* Computer player: responses that negate or destroy a card on the field. */
#include "duel.h"

extern unsigned int duel_w_08919608;
extern int duel_ai_ids_08907CB4[2];

int duel_088409B0(unsigned int p, int b);
int duel_0883FA64(unsigned int p, int id);
int duel_0883FD00(unsigned int p, int id);
int duel_0883FFA0(unsigned int p, unsigned int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_088D189C(unsigned int p, int z, unsigned int q, int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_088D1438(unsigned int p, int z, int a, int b);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
int duel_088D1608(unsigned int p, int z, int flags);
int duel_count_active(int id);
int duel_field_level(unsigned int p, int z);
int duel_find_effect_card(unsigned int p, int id, int skip);
void duel_088C448C(int p, int a);
unsigned int duel_088D13D8(unsigned int p, unsigned short id, unsigned int mask);
unsigned int duel_0884E4A0(DuelAct *a);
void duel_088C44E4(unsigned int p, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088CAB04();
void duel_088D1BEC(unsigned int p, int z, int a, int b);

int duel_08840B5C(unsigned int p)
{
    return duel_088409B0(p, 0x0);
}

int duel_08840B64(unsigned int p)
{
    return duel_088409B0(p, 0x1);
}

int duel_08840B6C(unsigned int p, int id)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        if (duel.players[q & 1].field[z].card.id != 0 && duel_slot_has_effect(q, z, id) != 0) {
            if (duel_088D189C(q, z, 0, id) == 0) {
                duel_088C89BC(q, z, id, 1);
            }
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08840CA0(unsigned int p)
{
    return duel_08840B6C(p, 0x1619);
}

int duel_08840CA8(unsigned int p)
{
    return duel_08840B6C(p, 0x1A7B);
}

int duel_08840CB0(unsigned int p, int id)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        if (duel.players[q & 1].field[z].card.id != 0 && duel_slot_has_effect(q, z, id) != 0) {
            if (duel_088D1438(q, z, 0, 0) == 0) {
                duel_088C89BC(q, z, id, 1);
            }
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08840DE4(unsigned int p)
{
    return duel_08840CB0(p, 0x1409);
}

int duel_08840DEC(unsigned int p)
{
    return duel_08840CB0(p, 0x1337);
}

int duel_08840DF4(unsigned int p)
{
    return duel_08840CB0(p, 0x16CE);
}

int duel_08840DFC(unsigned int p)
{
    if (duel_count_active(0x148E) != 0) {
        return 1;
    }
    if (duel_count_active(0x14DA) != 0) {
        return 1;
    }
    return duel_08840CB0(p, 0x1481);
}

int duel_08840E54(unsigned int p)
{
    return duel_08840CB0(p, 0x16A4);
}

int duel_08840E5C(unsigned int p)
{
    return duel_08840CB0(p, 0x1876);
}

int duel_08840E64(unsigned int p)
{
    return duel_08840CB0(p, 0x1A31);
}

int duel_08840E6C(unsigned int p)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        if (duel.players[q & 1].field[z].card.id == 0x1608) {
            duel_088D1438(q, z, 0, 0);
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_08840F40(unsigned int p)
{
    int z;
    unsigned int i;
    unsigned int q;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        if (duel.players[q & 1].field[z].card.id != 0 && duel_slot_has_effect(q, z, 0x17BE) != 0
            && duel_slot_ref_immune(q, z, 0) == 0) {
            if (duel_088D1608(q, z, 0) == 0) {
                duel_088C89BC(q, z, 0x17BE, 1);
            }
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_0884107C(unsigned int p)
{
    int q;
    int z;
    unsigned int mask;
    int b;
    unsigned int t;

    if (duel_w_08919608 != 0) {
        return 1;
    }
    if (duel_count_active(0x13F8) != 0) {
        mask = 0;
        for (q = 0, b = 0; q < 2; q++, b += 16) {
            for (z = 0; z < 5; z++) {
                if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup != 0
                    && duel_field_level(q, z) < 4
                    && (duel.players[q & 1].field[z].flags >> 16 & 1 || duel.players[q & 1].field[z].flags >> 17 & 1)) {
                    mask |= 1 << (z + b);
                }
            }
        }
        if (mask != 0) {
            t = 1 - p;
            t = duel_find_effect_card(t, 0x13F8, -1) != 0 ? t : p;
            duel_088C448C(-1, 0x13F8);
            duel_088D13D8(t, 0x13F8, mask);
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}

int duel_088411F8(unsigned int p)
{
    int id;
    unsigned int v;

    for (; duel_w_08919608 < 2; duel_w_08919608++) {
        id = duel_ai_ids_08907CB4[duel_w_08919608];
        if (duel_slot_has_effect(p, 0xB, id) != 0) {
            DuelAct t = {0};

            t.player = p;
            t.b3_4 = 0;
            t.id = id;
            v = duel_0884E4A0(&t);
            if (v != 0) {
                duel_088C44E4(p, t.id);
                duel_088D13D8(p, t.id, v);
                duel_w_08919608++;
                return 0;
            }
        }
    }
    return 1;
}

int duel_08841320(unsigned int p)
{
    return duel_0883FD00(1 - p, 0x1468);
}

int duel_08841330(unsigned int p)
{
    return duel_0883FD00(1 - p, 0x1540);
}

int duel_08841340(unsigned int p)
{
    return duel_0883FD00(1 - p, 0x17AF);
}

int duel_08841350(unsigned int p)
{
    return duel_0883FA64(1 - p, 0x1487);
}

int duel_08841360(unsigned int p)
{
    if (duel_slot_has_effect(p, 0xB, 0x13F5) != 0) {
        duel_0884F30C((p << 31 & 0x80000000) | 0xA5013F5, 0, 0);
        duel_088C89BC(p, 0xB, 0x13F5, 0);
        return 0;
    }
    return 1;
}

int duel_088413D0(unsigned int p)
{
    int i;
    int n;
    unsigned int q;
    int z;

    for (i = 0; i < 2; i++) {
        q = p ^ i;
        if (!duel_slot_has_effect(q, 0xB, 0x1379)) {
            continue;
        }
        for (n = duel.players[q & 1].n_hand - 1; n >= 0; n--) {
            if (duel_slot_effect_with(q, 0xB, 0x1379, CARD_UID(duel.players[q & 1].hand[n])) != 0) {
                duel_088C44E4(q, 0x1379);
                duel_088CAB04(q, n, 0);
            }
        }
        for (z = 5; z <= 10; z++) {
            if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup == 0
                && duel_slot_effect_with(q, 0xB, 0x1379, CARD_UID(duel.players[q & 1].field[z].card)) != 0) {
                duel_088C44E4(q, 0x1379);
                duel_088D1BEC(q, z, 0, 0x1379);
            }
        }
        duel_088C89BC(q, 0xB, 0x1379, 1);
        return 0;
    }
    return 1;
}

int duel_08841594(unsigned int p)
{
    return duel_0883FFA0(p, 0x1466);
}

int duel_0884159C(unsigned int p)
{
    return duel_0883FFA0(p, 0x16F9);
}

int duel_088415A4(unsigned int p)
{
    return duel_0883FFA0(p, 0x1836);
}

int duel_088415AC(unsigned int p)
{
    return duel_0883FFA0(p, 0x19F7);
}

int duel_088415B4(unsigned int p)
{
    return duel_0883FFA0(p, 0x19F8);
}

int duel_088415BC(unsigned int p)
{
    return duel_0883FFA0(p, 0x1A9F);
}
