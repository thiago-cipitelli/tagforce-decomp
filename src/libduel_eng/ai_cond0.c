// @range 0x0881D0D8-0x0881E0A0
// @asm 0x0881D0D8-0x0881D17C
// @asm 0x0881D17C-0x0881D318
// @asm 0x0881DEA0-0x0881DF94
/* Conditions the computer player checks before activating a card (continued in ai_cond.c). */
#include "duel.h"
#include "card_db.h"

int duel_field_atk(unsigned int p, int z);
int duel_count_field(unsigned int p);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_count_src(unsigned int p, int z, int id);
int duel_088AA224(DuelAct *a);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_08836868(unsigned int p);
int duel_08809F2C();
int duel_08809FC4();
int duel_08806730(unsigned int p, int a, int id);
int duel_08807B38(int id, int a, int b);
int duel_0880AA04(int id, int a);
int duel_0880AD78(unsigned int p);
int duel_0880A54C(DuelAct *a, unsigned int p, int z, int b);
int duel_08819FEC(DuelAct *a, int b);
int duel_count_free_monster_zones(unsigned int p);
int duel_hand_count_monsters(unsigned int p);
int duel_08809B50(unsigned int p, int id, int a);
DuelCard *duel_0884C7CC(void);
int duel_0884E5E0(DuelAct *a);
int duel_0880A8AC(unsigned int p, unsigned int q);
int duel_field_level(unsigned int p, int z);
int duel_count_monsters(unsigned int p);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int duel_count_name(unsigned int p, int id);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_slot_ef098();
int field_get_card(unsigned int p, int z);
int duel_088154D8(unsigned int p);
int duel_088086D4(int id, int a);
int duel_088070A8(int id);
unsigned int duel_088077D4(unsigned int p, int a);

extern struct DuelDlgHead {
    int unk0;
} duel_dlg_cards;

#ifdef NON_MATCHING /* the original keeps the constant 1 in a3 for both calls and loads the MAX operands in the other order */
int duel_ai_0881D0D8(DuelAct *a)
{
    int a0;
    int a1;
    int b0;
    int b1;

    duel_08809FC4(1 - a->player, &a0, &a1);
    duel_08809FC4(a->player, &b0, &b1, 1);
    if (MAX(a1, a0) < b0) {
        return 0;
    }
    if (MAX(a1, a0) < b1) {
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* the original swaps the registers of the card id and the hand offset */
int duel_ai_0881D17C(DuelAct *a)
{
    unsigned int i;
    int r;
    int n = 0;

    if (duel.players[(1 - a->player) & 1].n_hand < 3) {
        return 0;
    }
    if (duel_find_effect_card(a->player, 0x1306, -1) != 0) {
        return 1;
    }
    i = 0;
    if (duel.players[a->player & 1].n_hand != 0) {
        do {
            unsigned int id = duel.players[a->player & 1].hand[i].id;

            r = !duel_08806730(a->player, 1, id);
            if (!r) {
                return 0;
            }
            if (duel_08807B38(id, 0, 0) != 0) {
                n++;
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    return duel.players[(1 - a->player) & 1].n_hand < 5 ? n > 0 : 1;
}
#endif

int duel_ai_0881D318(DuelAct *a)
{
    return duel_0880AD78(1 - a->player) >= 0;
}

int duel_ai_0881D348(DuelAct *a)
{
    unsigned int p = a->player;
    int z;

    if (duel.players[p & 1].lp < duel.players[(1 - p) & 1].lp) {
    z = duel_0880A54C(a, p, -1, 0);
    if (duel_slot_count_src(a->player, z, 0x1237) != 0) {
        z = duel_0880A54C(a, a->player, z, 0);
    }
    return z >= 0 && duel_field_atk(a->player, z) > 0x5DB;
    }
    return 0;
}

int duel_ai_0881D438(DuelAct *a)
{
    return duel_find_effect_card(a->player, 0xFF8, -1) > 0;
}

int duel_ai_0881D464(DuelAct *a)
{
    unsigned int i;
    int r;

    i = 0;
    if (duel.players[a->player & 1].n_hand != 0) {
        do {
            unsigned int id = duel.players[a->player & 1].hand[i].id;

            r = !duel_08806730(a->player, 1, id);
            if (!r) {
                return 0;
            }
            if (duel_0880AA04(id, 0) != 0) {
                return 0;
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    i = 0;
    if (duel.players[a->player & 1].n_grave != 0) {
        do {
            r = !duel_0880AA04(duel.players[a->player & 1].grave[i].id, 0);
            if (!r) {
                return 1;
            }
            i++;
        } while (i < duel.players[a->player & 1].n_grave);
    }
    return 0;
}

int duel_ai_0881D5E8(DuelAct *a)
{
    int z;

    if (duel_count_field(1 - a->player) == 0) {
        return 0;
    }
    if (duel_find_effect_card(a->player, 0x13C3, -1) != 0) {
        return 1;
    }
    z = duel_0880A54C(a, a->player, -1, 0);
    return z >= 0 && duel_field_atk(a->player, z) > 0x707;
}

int duel_ai_0881D68C(DuelAct *a, int b)
{
    int z = duel_0880A54C(a, a->player, -1, 0);

    return z >= 0 && duel_field_atk(a->player, z) > 0x513;
}

int duel_ai_0881D6EC(DuelAct *a, int b)
{
    return duel_0880A54C(a, a->player, -1, 0) >= 0;
}

int duel_ai_0881D71C(DuelAct *a, int b)
{
    return duel_08819FEC(a, b) != 0 && duel_ai_0881D68C(a, b) != 0;
}

int duel_ai_0881D768(DuelAct *a, int b)
{
    int z;

    if (a->b3_4 == 0) {
        z = duel_0880A54C(a, a->player, -1, 0);
        return z >= 0 && duel_08819FEC(a, b) != 0;
    }
    return duel_088BFFD8(a->player ^ a->opp, a->id, duel_088AA224(a)) > 0;
}

int duel_ai_0881D810(DuelAct *a, int b)
{
    int z = duel_0880A54C(a, a->player, -1, 0);

    return z >= 0 && duel_field_atk(a->player, z) < 0x514 && duel_08819FEC(a, b) != 0;
}

int duel_ai_0881D890(DuelAct *a)
{
    int z;

    if (duel_08809F2C(a->player) >= 0) {
        return 0;
    }
    z = duel_0880A54C(a, 1 - a->player, -1, 0);
    return z >= 0 && duel_field_atk(1 - a->player, z) > 0x513;
}

int duel_ai_0881D91C(DuelAct *a)
{
    if (duel_08836868(a->player) != 0) {
        return duel_count_monsters_pos(a->player, 1, 1) > 0;
    }
    return 0;
}

int duel_ai_0881D968(DuelAct *a, int b)
{
    if (a->b3_4 == 0) {
        return duel_08819FEC(a, b);
    }
    return (unsigned int)duel.players[a->player & 1].n_d < 0x14;
}

int duel_ai_0881D9C8(DuelAct *a)
{
    unsigned int i;
    int r;

    if (duel_08809F2C(a->player) >= 0) {
        return 0;
    }
    if (duel.players[a->player & 1].n_hand < 4) {
        return 0;
    }
    i = 0;
    if (duel.players[a->player & 1].n_hand != 0) {
        do {
            unsigned int id = duel.players[a->player & 1].hand[i].id;

            r = !duel_08806730(a->player, 1, id);
            if (!r) {
                return 0;
            }
            if (duel_0880AA04(id, 0) != 0) {
                return 0;
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    return 1;
}

int duel_ai_0881DAF0(DuelAct *a)
{
    unsigned int i;
    int n = 0;

    if (duel.players[a->player & 1].n_hand < 4) {
        return 0;
    }
    i = 0;
    if (duel.players[a->player & 1].n_hand != 0) {
        do {
            if (a->uid != CARD_UID(duel.players[a->player & 1].hand[i])) {
                unsigned int id = duel.players[a->player & 1].hand[i].id;

                if (duel_08806730(a->player, 1, id) == 0 && card_get_limit(id) >= 3 && duel_0880AA04(id, 0) == 0) {
                    n++;
                }
            }
            i++;
        } while (i < duel.players[a->player & 1].n_hand);
    }
    return n > 1;
}

int duel_ai_0881DC58(DuelAct *a)
{
    int k = duel_088BFFD8(a->player, a->id, 0);
    int f = duel_count_free_monster_zones(a->player);

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel_find_effect_card(a->player, 0x17A3, -1) != 0 && duel_find_effect_card(1 - a->player, 0x1398, -1) != 0) {
        return 0;
    }
    return MIN(k, f) > 1;
}

int duel_ai_0881DD24(DuelAct *a, int b)
{
    if (duel_find_effect_card(a->player, 0x140E, -1) != 0) {
        return 0;
    }
    if (duel_08809F2C(a->player) > 0) {
        return 0;
    }
    return duel_08819FEC(a, b);
}

int duel_ai_0881DD94(DuelAct *a)
{
    int k;

    if (duel_08809F2C(a->player) >= 0) {
        return 0;
    }
    k = duel_088BFFD8(a->player, a->id, 0);
    return duel_0884E5E0(a) <= k;
}

int duel_ai_0881DDF4(DuelAct *a)
{
    if (duel.players[a->player & 1].n_hand > 2) {
        return 0;
    }
    return !duel_hand_count_monsters(a->player);
}

int duel_ai_0881DE54(DuelAct *a)
{
    if (duel_08809B50(a->player, (unsigned short)a->id, 0) >= 0) {
        return (int)card_get_level(duel_0884C7CC()->id) > 4;
    }
    return 0;
}

#ifdef NON_MATCHING /* the original keeps the final atk + x < lp as bnez/li 0/b with v0 = 1 left from 1 - p; mwcc folds it */
int duel_ai_0881DEA0(DuelAct *a)
{
    if (duel.players[a->player & 1].lp > 4000) {
        return 0;
    }
    if (duel_08809B50(a->player, a->id, 0) >= 0) {
        unsigned int id = duel_0884C7CC()->id;
        unsigned int p = a->player;
        int atk = card_get_atk(id);
        int x = duel_0880A8AC(p, p);
        if (duel.players[(1 - a->player) & 1].lp <= atk + x) { return 1; } return 0;
    }
    return 0;
}
#endif

int duel_ai_0881DF94(DuelAct *a)
{
    int lv;
    int z;

    if (duel_08809B50(a->player, a->id, 0) >= 0) {
        lv = card_get_level(duel_0884C7CC()->id);
        for (z = 0; z < 5; z++) {
            if (duel.players[a->player & 1].field[z].card.id != 0 && duel_field_level(a->player, z) >= lv) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

int duel_ai_0881E070(DuelAct *a)
{
    return duel.players[a->player & 1].n_hand < 7;
}
