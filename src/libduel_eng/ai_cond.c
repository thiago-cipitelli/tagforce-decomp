// @range 0x0881E0A0-0x08820618
// @asm 0x0881E27C-0x0881E348
// @asm 0x0881F58C-0x0881F644
// @asm 0x088201F0-0x08820260
// @asm 0x08820350-0x088203F4
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
int duel_0880ABE0();
unsigned int duel_08807434(unsigned int p, int a);
int duel_08809B50(unsigned int p, unsigned short id, int a);
int duel_08817220(unsigned int p, int z);
int duel_08817408(unsigned int p, int z);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_monsters_state(unsigned int p, int pos, int state);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
int duel_slot_e68a0(unsigned int p, int z);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_d_count_name(unsigned int p, unsigned short id);
unsigned int duel_d_find_name(unsigned int p, int id);
int duel_0881A074(DuelAct *a, int b);
int duel_08819F9C(DuelAct *a, int b);
int duel_0881749C(unsigned int p, int z);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_0880EA88(unsigned int p, int a);
int duel_0881657C(unsigned int p, unsigned int mask, int a);
int duel_count_facedown_st(unsigned int p);
int duel_slot_value_1909(unsigned int p, int z);
int duel_list_card_level(unsigned int p, int i);
int duel_count_field(unsigned int p);
void duel_act_0887E140(DuelAct *a, int d[2]);
int duel_088054A8(unsigned int p, int a, int b);
int duel_0886C448(DuelAct *a);

extern int duel_w_08914920;
extern int duel_phase;

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

int duel_ai_0881F058(DuelAct *a)
{
    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    return duel_attack_check(a->player, a->zone, 1) != 0;
}

int duel_ai_0881F0AC(DuelAct *a, int b)
{
    int p;
    int z;

    if (duel_0881A074(a, b) == 0) {
        return 0;
    }
    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel_0884E3E4(a, p, z) != 0 && duel_0881749C(p, z) >= 500) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_ai_0881F168(DuelAct *a)
{
    if (duel_count_field_all(1 - a->player) < 2) {
        return 0;
    }
    return duel_08836868(a->player) != 0;
}

int duel_ai_0881F1BC(DuelAct *a, int b)
{
    if (duel_find_effect_card(1 - a->player, 0x1614, -1) != 0) {
        return 0;
    }
    if (duel_find_effect_card(1 - a->player, 0x17C2, -1) != 0) {
        return 0;
    }
    return duel_0881A074(a, b);
}

int duel_ai_0881F244(DuelAct *a)
{
    if (duel_count_monsters_state(1 - a->player, 0, 1) == 0) {
        return 0;
    }
    return duel_0884E948(a, 1 - a->player) != 0;
}

int duel_ai_0881F2A4(DuelAct *a)
{
    unsigned int p;
    int lp;

    if (duel_0884E948(a, 1 - a->player) == 0) {
        return 0;
    }
    if (duel_hand_count_name(a->player, 0x18E8) != 0) {
        return 0;
    }
    p = a->player;
    lp = duel.players[(1 - p) & 1].lp;
    if (lp <= card_get_atk(a->id) && duel_08836868(p) != 0) {
        return 1;
    }
    if (duel_attack_check(a->player, a->zone, 1) == 0) {
        return 1;
    }
    return !(duel_08809F2C(a->player) > 0);
}

void duel_ai_0881F3A4(DuelAct *a, int b)
{
    a->w[2] = duel_slot_e68a0(a->player, a->zone);
    duel_08819F9C(a, b);
}

int duel_ai_0881F3F4(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = (1 - p) & 1;

    if (duel.players[q].n_hand + duel_0884E948(a, 1 - p) == 0) {
        return 0;
    }
    if (duel.players[q].lp < 0xB55 && duel_08836868(p) != 0) {
        return 1;
    }
    if (duel_attack_check(a->player, a->zone, 1) == 0) {
        return 1;
    }
    return !(duel_08809F2C(a->player) > 0);
}

int duel_ai_0881F4D4(DuelAct *a)
{
    int r;

    if (duel.players[a->player & 1].field[a->zone].flags >> 21 & 1) {
        return 0;
    }
    a->b3_4 = 1;
    r = duel_0884E948(a, 1 - a->player);
    a->b3_4 = 3;
    return r != 0;
}

#ifdef NON_MATCHING /* the original keeps p != zone owner as bnel/li 1/b/li 0; mwcc folds it into xor/sltu */
int duel_ai_0881F58C(DuelAct *a)
{
    int z = a->zone;
    int r;

    if (z >= 5) {
        unsigned int p = a->player;

        if (p != (duel_slot_e1854(p, z) & 0xFF)) {
            r = 1;
        } else {
            r = 0;
        }
        return r;
    }
    if (duel_w_08914920 != 0) {
        return 0;
    }
    for (z = 0; z <= 10; z++) {
        r = !duel_st_affects(a->player, a->zone, a->player, z);
        if (!r) {
            return 0;
        }
    }
    return 1;
}
#endif

int duel_ai_0881F644(DuelAct *a, int b)
{
    if (duel_08819F9C(a, b) != 0) {
        return duel_count_field_cards(1 - a->player, 1, 0, 1) > 0;
    }
    return 0;
}

int duel_ai_0881F690(DuelAct *a)
{
    unsigned int p = a->player;
    DuelPlayer *pl = &duel.players[p & 1];

    if (pl->field[10].card.id != 0x175E) {
        return 0;
    }
    return !(pl->lp < duel.players[(1 - p) & 1].lp * 2);
}

int duel_ai_0881F700(DuelAct *a)
{
    return duel_d_count_name(a->player, a->id);
}

int duel_ai_0881F710(DuelAct *a)
{
    int n;

    if ((int)duel_d_find_name(a->player, 0x12E5) < 0) {
        return 0;
    }
    if (duel_0880AF74(a->player, 0x12E5) != 0) {
        return 0;
    }
    n = duel_count_monsters(a->player);
    return !(n + duel_hand_count_monsters(a->player) < 3);
}

int duel_ai_0881F790(DuelAct *a)
{
    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel_attack_check(a->player, a->zone, 1) == 0) {
        return 0;
    }
    if (duel_count_monsters(1 - a->player) == 0) {
        return 0;
    }
    return duel.players[(1 - a->player) & 1].lp < 2000;
}

int duel_ai_0881F840(DuelAct *a)
{
    if (duel_0884E948(a, 1 - a->player) == 0) {
        return 0;
    }
    if (duel_phase == 4) {
        return 1;
    }
    if (duel_0880ABE0(a, 1 - a->player, 0) != 0) {
        return 1;
    }
    if (duel_08836868(a->player) == 0) {
        return 1;
    }
    if (duel_08809F2C(a->player) < 0) {
        return 1;
    }
    return !duel_0880EA88(a->player, 0);
}

int duel_ai_0881F908(DuelAct *a)
{
    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel_08809F2C(a->player) < 1) {
        return 0;
    }
    return duel_count_monsters(1 - a->player) > 2;
}

int duel_ai_0881F974(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[(1 - p) & 1].lp > 0x7D0) {
        return 0;
    }
    if (duel_08836868(p) == 0) {
        return 0;
    }
    return !(duel_count_monsters_fn(a->player, card_is_in_group_1911) < 2);
}

int duel_ai_0881FA08(DuelAct *a)
{
    if (duel_count_monsters(1 - a->player) == 0) {
        return 0;
    }
    return duel_08836868(a->player) != 0;
}

int duel_ai_0881FA58(DuelAct *a)
{
    if (duel_08809F2C(a->player) < 0) {
        return 1;
    }
    return duel_0880ABE0(a, 1 - a->player, 0) != 0;
}

int duel_ai_0881FAB0(DuelAct *a)
{
    if (duel_count_monsters_pos(1 - a->player, 1, 0) != 0) {
        return duel_0880AF74(a->player, 0x198E) > 0;
    }
    return 0;
}

int duel_ai_0881FB04(DuelAct *a)
{
    return duel_0880ABE0(a, 1 - a->player, 1) != 0;
}

int duel_ai_0881FB30(DuelAct *a)
{
    if (duel_hand_count_name(a->player, 0x198C) != 0) {
        return 0;
    }
    if (duel_0880ABE0(a, 1 - a->player, 1) != 0) {
        return 1;
    }
    return duel_0884E948(a, 1 - a->player) > 1;
}

int duel_ai_0881FBB0(DuelAct *a)
{
    if (duel_0880ABE0(a, 1 - a->player, 1) != 0) {
        return 1;
    }
    return duel_0884E948(a, 1 - a->player) > 1;
}

int duel_ai_0881FC10(DuelAct *a)
{
    unsigned int m;

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].pos != 0) {
        return 0;
    }
    m = duel_0884E4A0(a);
    return duel_0881657C(a->player, m, 1) >= 0;
}

int duel_ai_0881FCBC(DuelAct *a)
{
    return duel_count_monsters(1 - a->player) > 0;
}

int duel_ai_0881FCE8(DuelAct *a)
{
    return !duel_count_monsters(1 - a->player);
}

int duel_ai_0881FD18(DuelAct *a)
{
    return duel_count_monsters(1 - a->player) > 1;
}

int duel_ai_0881FD48(DuelAct *a, int b)
{
    unsigned int m = duel_0884E4A0(a);

    if (duel_0881657C(a->player, m, 1) < 0) {
        return 0;
    }
    return duel_0881A038(a, b);
}

int duel_ai_0881FDA4(DuelAct *a)
{
    return duel_count_field_cards(a->player, 0, 1, 0) > 0;
}

int duel_ai_0881FDD4(DuelAct *a)
{
    unsigned char z;
    unsigned int p;

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    z = a->zone;
    p = a->player;
    if (duel.players[p & 1].field[z].pos != 0) {
        return 0;
    }
    if (duel_attack_check(p, z, 1) != 0) {
        return duel_count_name(a->player, 0x195A) > 0;
    }
    return 0;
}

int duel_ai_0881FE88(DuelAct *a)
{
    return duel.players[a->player & 1].field[10].card.id == 0x175E;
}

int duel_ai_0881FEC0(DuelAct *a)
{
    unsigned int i;

    if (duel.players[a->player & 1].flags >> 17 & 1) {
        return 0;
    }
    i = 0;
    if (duel.players[a->player & 1].n_hand != 0) {
        do {
            unsigned int id = duel.players[a->player & 1].hand[i].id;

            if (card_is_monster(id) != 0 && card_attribute_icon_in_2_3(id) == 0 &&
                duel_list_card_level(a->player, CARD_UID(duel.players[a->player & 1].hand[i])) >= 5) {
                return 1;
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    return 0;
}

int duel_ai_08820020(DuelAct *a)
{
    return !duel_count_name(a->player, 0x19FD);
}

int duel_ai_0882004C(DuelAct *a, int b)
{
    if (duel_088054A8(a->player, -1, 0) < 2000) {
        return 0;
    }
    return duel_0881A074(a, b);
}

int duel_ai_088200A8(DuelAct *a)
{
    if (duel_hand_count_name(a->player, 0x19FD) != 0) {
        return duel_count_monsters_fn(a->player, card_is_in_group_1a75) > 0;
    }
    return 0;
}

int duel_ai_088200F8(DuelAct *a, int b)
{
    if (duel_088BFFD8(a->player, a->id, 0) < 2) {
        return 0;
    }
    return duel_0881A074(a, b);
}

int duel_ai_08820154(DuelAct *a, int b)
{
    if (duel_0881A074(a, b) != 0) {
        return duel_0881A038(a, b);
    }
    return 0;
}

int duel_ai_08820198(DuelAct *a)
{
    return duel_count_facedown_st(a->player) > 1;
}

int duel_ai_088201C0(DuelAct *a)
{
    return duel_slot_value_1909(a->player, a->zone) > 1;
}

#ifdef NON_MATCHING /* the original folds the field link offset into the base address of duel; mwcc adds it after the index */
int duel_ai_088201F0(DuelAct *a)
{
    unsigned int p = a->player;
    DuelSlot *s = &duel.players[p & 1].field[a->zone];

    return s->link <= (unsigned int)duel_0884E948(a, 1 - p);
}
#endif

int duel_ai_08820260(DuelAct *a)
{
    int n = 0;
    int z;

    if (!duel_count_monsters(a->player) < 3) {
        return 0;
    }
    if (duel_0880ABE0(a, 1 - a->player, 0) != 0) {
        return 1;
    }
    for (z = 0; z < 5; z++) {
        if (duel.players[(1 - a->player) & 1].field[z].card.id != 0 && duel.players[(1 - a->player) & 1].field[z].pos != 0) {
            n++;
        }
    }
    return !(n < 3);
}

#ifdef NON_MATCHING /* the original addresses both n_hand fields from their own lui; mwcc shares the base of duel */
int duel_ai_08820350(DuelAct *a)
{
    int n0 = duel_count_field(0);
    int n1 = duel_count_field(1);

    return duel.players[(1 - a->player) & 1].lp <= (duel.players[1].n_hand + duel.players[0].n_hand + n0 + n1) * 300;
}
#endif

int duel_ai_088203F4(DuelAct *a, int b)
{
    int z;

    if (a->b3_4 == 0) {
        return duel_08819FEC(a, b);
    }
    for (z = 0; z < 5; z++) {
        if (duel.players[a->player & 1].field[z].card.id != 0 && duel.players[a->player & 1].field[z].faceup == 0 &&
            (duel.players[a->player & 1].field[z].flags >> 7 & 1)) {
            return 1;
        }
    }
    return 0;
}

int duel_ai_088204A8(DuelAct *a)
{
    int v[2];
    unsigned int p;
    unsigned int q;

    duel_act_0887E140(a, v);
    p = a->player;
    q = 1 - p;
    if (duel.players[q & 1].lp + v[q] <= 0) {
        return 1;
    }
    if (duel_find_effect_card(p, 0x1964, -1) != 0) {
        return 1;
    }
    return duel_find_effect_card(a->player, 0x1982, -1) != 0;
}

int duel_ai_0882054C(DuelAct *a)
{
    if (card_get_type(a->id) == 0x16) {
        if (duel_find_effect_card(1 - a->player, 0x17D4, -1) != 0) {
            return 1;
        }
    } else if (card_get_type(a->id) == 0x17) {
        if (duel_find_effect_card(1 - a->player, 0x17C6, -1) != 0) {
            return 1;
        }
    } else {
        if (duel_find_effect_card(a->player, 0x1771, -1) != 0) {
            return 1;
        }
    }
    return duel_0886C448(a);
}
