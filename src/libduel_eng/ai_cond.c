// @range 0x0881E0A0-0x0881F058
// @asm 0x0881E27C-0x0881E348
/* Conditions the computer player checks before activating a card (one per card). */
#include "duel.h"
#include "card_db.h"

int duel_card_check_9148(unsigned int p, int id);
int duel_field_def(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_field_base_atk(unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_count_monsters(unsigned int p);
int duel_count_field_all(unsigned int p);
int duel_count_name(unsigned int p, int id);
int duel_count_grave_type(unsigned int p, unsigned int type);
int duel_grave_count_id(unsigned int p, int id);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_hand_count_level(unsigned int p, int lvl);
int duel_hand_count_monsters(unsigned int p);
int duel_list_counter(int uid, int id);
int duel_088AA224(DuelAct *a);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_08836868(unsigned int p);
int duel_08809F2C();
int duel_can_attack(unsigned int p, int z, int arg);
DuelCard *duel_0884C7CC(void);
unsigned int duel_0884E4A0(DuelAct *a);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_0884E5E0(DuelAct *a);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_0881A038(DuelAct *a, int b);
int duel_08819FEC(DuelAct *a, int b);
int duel_0881D6EC(DuelAct *a, int b);
int duel_088159C8(unsigned int p, unsigned int mask, int (*fn)());
int duel_088171C8();
int duel_0880AEF4(unsigned int p, int id);
int duel_0880AF74(unsigned int p, int id);
int duel_0880B07C(unsigned int p);
int duel_0880A17C(unsigned int p, unsigned int q, int a, int b, int c);
int duel_0880ABE0(DuelAct *a, unsigned int p, int b);
unsigned int duel_08807434(unsigned int p, int a);
int duel_08809B50(unsigned int p, unsigned short id, int a);
int duel_08817220(unsigned int p, int z);
int duel_08817408(unsigned int p, int z);

int duel_ai_0881E0A0(DuelAct *a)
{
    int z;

    if (duel_card_check_9148(a->player, 0xFBC) == 0) {
        return 0;
    }
    for (z = 0; z < 5; z++) {
        unsigned int q = 1 - a->player;

        if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup != 0 &&
            duel_field_def(q, z) < 0x9C5) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881E170(DuelAct *a, int b)
{
    if (duel_count_monsters(1 - a->player) != 0) {
        return duel_0881A038(a, b);
    }
    return 0;
}

int duel_ai_0881E1C4(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[(1 - p) & 1].lp <= 0x960) {
        return 1;
    }
    if (duel_hand_count_name(p, 0x1894) != 0) {
        return 1;
    }
    return duel_count_field_all(1 - a->player) > 0;
}

int duel_ai_0881E254(DuelAct *a)
{
    return duel_hand_count_level(a->player, 5) > 0;
}

#ifdef NON_MATCHING /* the original tests atk < lp then atk <= 1000 with a dead `b` between; mwcc merges them */
int duel_ai_0881E27C(DuelAct *a)
{
    unsigned int m = duel_0884E4A0(a);
    int z = duel_088159C8(1 - a->player, m, duel_088171C8);
    int atk;
    int r;

    if (z >= 0) {
        atk = duel_field_base_atk(1 - a->player, z) / 2;
        r = 1;
        if (atk < duel.players[(1 - a->player) & 1].lp && atk <= 0x3E8) {
            r = 0;
        }
        return r;
    }
    return 0;
}
#endif

int duel_ai_0881E348(DuelAct *a, int b)
{
    return duel_08819FEC(a, b) != 0 && duel_0881D6EC(a, b) != 0;
}

int duel_ai_0881E394(DuelAct *a, int b)
{
    if (duel_list_counter(a->uid, a->id) < 3) {
        return duel_0881A038(a, b);
    }
    return duel_count_monsters(1 - a->player) >= 2;
}

int duel_ai_0881E40C(DuelAct *a)
{
    return duel_088BFFD8(a->player, a->id, duel_088AA224(a)) > 1;
}

int duel_ai_0881E44C(DuelAct *a)
{
    return !duel_slot_has_effect(a->player, 11, a->id);
}

int duel_ai_0881E47C(DuelAct *a, int b)
{
    if (a->b3_4 == 0) {
        return duel_08819FEC(a, b);
    }
    return 1;
}

int duel_ai_0881E4AC(DuelAct *a)
{
    int z;

    if (duel_0880AEF4(a->player, 0x12BE) != 0) {
        return 1;
    }
    if (duel_0880AEF4(a->player, 0xFBB) != 0) {
        return 1;
    }
    for (z = 0; z <= 10; z++) {
        unsigned int id = duel.players[a->player & 1].field[z].card.id;

        if (id != 0 && duel.players[a->player & 1].field[z].faceup != 0 && card_is_in_group_1983((unsigned short)id)) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881E58C(DuelAct *a)
{
    int r;

    if (duel_hand_count_name(a->player, 0x1181) == 0) {
        if (duel_hand_count_name(a->player, 0x179A) != 0) {
            r = 1;
        } else {
            r = 0;
        }
    } else {
        r = 1;
    }
    return r;
}

int duel_ai_0881E5E0(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (duel.players[a->player & 1].field[z].card.id != 0 && duel_08817220(a->player, z) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881E67C(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (duel.players[a->player & 1].field[z].card.id != 0 && duel_08817408(a->player, z) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881E718(DuelAct *a)
{
    return duel_slot_has_effect(1 - a->player, 11, 0x12A1);
}

int duel_ai_0881E734(DuelAct *a)
{
    return duel_count_name(a->player, 0x12D7) > 0;
}

int duel_ai_0881E75C(DuelAct *a)
{
    return duel_0880AF74(a->player, 0x15A3);
}

int duel_ai_0881E76C(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (duel_0884E3E4(a, a->player, z) != 0) {
            unsigned int p = a->player;

            if (card_attribute_icon_is3(duel.players[p & 1].field[z].card.id) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_ai_0881E818(DuelAct *a)
{
    if (duel_0880B07C(a->player) == 0) {
        return 0;
    }
    if (duel_count_grave_type(a->player, 0x15) != 0) {
        return 1;
    }
    if (duel_0880AF74(a->player, 0x18F6) == 0 && duel_grave_count_id(a->player, 0x18F6) != 0) {
        return 1;
    }
    if (duel_hand_count_monsters(a->player) == 0) {
        return 1;
    }
    return 0;
}

int duel_ai_0881E8BC(DuelAct *a)
{
    unsigned int p;

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel_count_monsters(1 - a->player) == 0) {
        return 0;
    }
    p = a->player;
    return duel_0880A17C(p, p, -1, 1, 0) > 999;
}

int duel_ai_0881E938(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        unsigned int p = a->player;

        if (duel.players[p & 1].field[z].card.id != 0 && p != duel.players[p & 1].field[z].card.owner &&
            duel_0884E3E4(a, p, z) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_0881E9EC(DuelAct *a)
{
    int z;
    int sum = 0;
    unsigned int p;
    unsigned int t;

    for (z = 0; z <= 10; z++) {
        if (duel.players[a->player & 1].field[z].card.id != 0) {
            sum += duel_field_atk(a->player, z) / 2;
        }
    }
    p = a->player;
    if (duel.players[(1 - p) & 1].lp <= sum) {
        return 1;
    }
    t = duel_08807434(p, 0);
    if ((int)t >= 0 && duel_field_atk(t & 0xFF, (unsigned char)((t & 0xFFFF) >> 8)) >= 1000) {
        return 1;
    }
    return 0;
}

int duel_ai_0881EB10(DuelAct *a)
{
    unsigned int p = a->player;
    int lp = duel.players[(1 - p) & 1].lp;

    if (lp <= duel_count_monsters(p) * 500) {
        return 1;
    }
    if (duel_count_monsters(p) > 1 && (int)duel_08807434(a->player, 0) >= 0) { return 1; } return 0;
}

int duel_ai_0881EBD8(DuelAct *a)
{
    unsigned int p = a->player;
    int lp = duel.players[(1 - p) & 1].lp;

    if (lp <= duel_0884E5E0(a) * 700) {
        return 1;
    }
    if (duel_count_monsters(p) > 1 && (int)duel_08807434(a->player, 0) >= 0) { return 1; } return 0;
}

int duel_ai_0881EC94(DuelAct *a)
{
    DuelPlayer *q = &duel.players[(1 - a->player) & 1];

    return q->lp <= duel_0884E5E0(a) * 500;
}

int duel_ai_0881ED00(DuelAct *a)
{
    return !(duel_08809F2C(a->player) > 0);
}

int duel_ai_0881ED28(DuelAct *a)
{
    return !duel_0884E948(a, a->player);
}

int duel_ai_0881ED50(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[(1 - p) & 1].lp <= 0x4B0) {
        return 1;
    }
    return duel_count_monsters(p) > 2;
}

int duel_ai_0881EDB8(DuelAct *a)
{
    unsigned int id;

    if (duel_count_monsters(a->player) < 2) {
        return 0;
    }
    if (duel_08809B50(a->player, a->id, 0) >= 0) {
        id = duel_0884C7CC()->id;
        return card_attribute_to_group(id) != 0 && id != a->id;
    }
    return 0;
}

int duel_ai_0881EE58(DuelAct *a)
{
    DuelStats st;
    int z;

    if (duel_08836868(a->player) != 0 && duel_can_attack(a->player, a->zone, 1) != 0) {
        for (z = 0; z <= 4; z++) {
            unsigned int q = 1 - a->player;

            if (duel.players[q & 1].field[z].card.id != 0) {
                int v;

                duel_field_stats(q, z, &st);
                if (duel.players[(1 - a->player) & 1].field[z].pos != 0) {
                    v = st.def;
                } else {
                    v = st.atk;
                }
                if (v >= 1000 && v <= 1999) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_ai_0881EF84(DuelAct *a)
{
    return duel_08836868(a->player) != 0 && !duel_count_monsters(1 - a->player);
}

int duel_ai_0881EFD8(DuelAct *a)
{
    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel_count_monsters(1 - a->player) >= 2) {
        return 0;
    }
    return !duel_0880ABE0(a, 1 - a->player, 0);
}
