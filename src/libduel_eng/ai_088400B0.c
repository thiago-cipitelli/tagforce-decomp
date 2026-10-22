// @range 0x088400B0-0x08840B5C
/* Computer player: cards it answers with from the field (zone scans posting duel_0884F30C requests). */
#include "duel.h"

extern unsigned int duel_turn_player;
extern unsigned int duel_w_08919608;

int field_get_card(unsigned int p, int z);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_zone_faceup(unsigned int p, int z, int i);
int duel_count_active(int id);
int card_is_attribute7(int id);
int card_attribute_icon_is3(int id);
int card_is_in_group_1a6f_b(int id);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088D1BEC(unsigned int p, int z, int a, int b);
void duel_088D189C(unsigned int p, int z, int a, int b);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_0883FA64(unsigned int p, int id);

int duel_088400B0(void)
{
    int z;
    unsigned int i;
    unsigned int q;
    int id;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = duel_turn_player;
        q ^= i / 5;
        z = i % 5;
        id = field_get_card(q, z);
        switch (id) {
        case 0x17D2:
        case 0x17D3:
        case 0x17D7:
        case 0x17D8:
        case 0x17DA:
            if (duel.players[q & 1].field[z].faceup != 0) {
                duel_0884F30C((q << 31 & 0x80000000) | 0xA200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                              CARD_UID(duel.players[q & 1].field[z].card), 0);
                duel_w_08919608++;
                return 0;
            }
        }
    }
    return 1;
}

int duel_08840234(unsigned int p)
{
    int i;
    unsigned int q;
    int v;

    for (i = 0; i < 2; i++) {
        q = p ^ i;
        v = duel_slot_effect_arg(q, 0xB, 0x15B8);
        if (v >= 0) {
            duel_088C89BC(q, 0xB, 0x15B8, 0);
            duel_0884F30C((q << 31 & 0x80000000) | 0xA5015B8, 0, v);
            return 0;
        }
    }
    return 1;
}

int duel_088402E0(unsigned int p)
{
    int i;
    unsigned int q;
    int v;

    for (i = 0; i < 2; i++) {
        q = p ^ i;
        v = duel_slot_effect_arg(q, 0xB, 0x16B9);
        if (v >= 0) {
            duel_088C89BC(q, 0xB, 0x16B9, 0);
            duel_0884F30C((q << 31 & 0x80000000) | 0xA4F16B9, (unsigned int)v >> 3, v);
            return 0;
        }
    }
    return 1;
}

int duel_0884038C(unsigned int p)
{
    int i;
    int n;
    unsigned int q;
    DuelCard *c;

    for (i = 0; i < 2; i++) {
        q = p ^ i;
        for (n = duel.players[q & 1].n_f - 1; n >= 0; n--) {
            c = &duel.players[q & 1].zone_f[n];
            if (c->id == 0x16BE && !c->b21 && !c->b20 && duel_zone_faceup(q, 0xF, n) != 0
                && duel_0884F30C((q << 31 & 0x80000000) | 0xA4F0000 | c->id, CARD_UID(*c), 0) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

int duel_088404E4(unsigned int p)
{
    int i;
    unsigned int q;
    int v;
    unsigned int o;

    for (i = 0; i < 2; i++) {
        q = p ^ i;
        v = duel_slot_effect_arg(q, 0xB, 0x18BC);
        if (v > 0) {
            o = v & 1;
            duel_0884F30C((o << 31 & 0x80000000) | 0xA000000 | ((o != q) << 24 & 0x1000000) | 0x4F18BC, v, 0);
            duel_088C89BC(q, 0xB, 0x18BC, 0);
            return 0;
        }
    }
    return 1;
}

int duel_088405A8(unsigned int p)
{
    return duel_0883FA64(p, 0x11CF);
}

int duel_088405B0(unsigned int p)
{
    return duel_0883FA64(p, 0x12AC);
}

int duel_088405B8(unsigned int p)
{
    return duel_0883FA64(p, 0x1644);
}

int duel_088405C0(unsigned int p)
{
    return duel_0883FA64(p, 0x1911);
}

int duel_088405C8(unsigned int p)
{
    int z;

    for (; duel_w_08919608 <= 4; duel_w_08919608++) {
        z = duel_w_08919608;
        if (!duel_slot_e8170(p, z, 0xFBD)) {
            continue;
        }
        duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
        duel_088D189C(p, z, 0, 0);
        duel_w_08919608++;
        return 0;
    }
    return 1;
}

int duel_088406C4(unsigned int p)
{
    int i;
    unsigned int q;
    int z;
    DuelPlayer *pl;
    int id;

    if (duel_count_active(0x150E) != 0) {
        return 1;
    }
    for (i = 0; i < 2; i++) {
        q = p ^ i;
        pl = &duel.players[q & 1];
        for (z = 0; z <= 4; z++) {
            id = field_get_card(q, z);
            if (card_is_attribute7(id) != 0
                && (pl->field[z].flags >> 16 & 1 || pl->field[z].flags >> 17 & 1 || pl->field[z].flags >> 19 & 1)
                && SLOT_ACTIVE(&pl->field[z])) {
                duel_0884F30C((q << 31 & 0x80000000) | 0xA200000 | (z << 16 & 0x1F0000) | (id & 0xFFFF),
                              CARD_UID(pl->field[z].card), 0);
                return 0;
            }
        }
    }
    return 1;
}

int duel_08840854(unsigned int p)
{
    int i;
    unsigned int q;
    int z;
    DuelPlayer *pl;
    int id;

    for (i = 0; i < 2; i++) {
        q = p ^ i;
        pl = &duel.players[q & 1];
        for (z = 0; z <= 4; z++) {
            id = field_get_card(q, z);
            if (pl->field[z].flags >> 3 & 1 && SLOT_ACTIVE(&pl->field[z])) {
                switch (id) {
                case 0x1386:
                case 0x1387:
                case 0x1388:
                    duel_088C4448(q, z, CARD_UID(pl->field[z].card));
                    duel_088D1BEC(q, z, 0, 0);
                    return 0;
                }
            }
        }
    }
    return 1;
}

int duel_088409B0(unsigned int p, int b)
{
    unsigned int i;
    int z;
    unsigned int q;
    int id;
    int c;

    for (; (i = duel_w_08919608) < 10; duel_w_08919608++) {
        q = p ^ i / 5;
        z = i % 5;
        id = duel.players[q & 1].field[z].card.id;
        c = field_get_card(q, z);
        if (card_attribute_icon_is3(id) != 0 && card_is_in_group_1a6f_b(c) != 0
            && SLOT_ACTIVE(&duel.players[q & 1].field[z])
            && duel_0884F30C((q << 31 & 0x80000000) | 0xA200000 | (z << 16 & 0x1F0000) | (c & 0xFFFF),
                             CARD_UID(duel.players[q & 1].field[z].card), b) != 0) {
            duel_w_08919608++;
            return 0;
        }
    }
    return 1;
}
