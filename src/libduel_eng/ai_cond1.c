// @range 0x0881C118-0x0881D0D8
// @asm 0x0881C1AC-0x0881C2D0
// @asm 0x0881C3B0-0x0881C4DC
// @asm 0x0881C538-0x0881C6D4
// @asm 0x0881C6D4-0x0881C850
// @asm 0x0881C8F4-0x0881CA34
// @asm 0x0881CFF4-0x0881D0D8
/* Conditions the computer player checks before activating a card (continued in ai_cond0.c). */
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
int duel_grave_count_id(unsigned int p, int id);
int duel_08869ACC(DuelAct *a, int b);
int duel_can_special_summon(unsigned int p);
int duel_can_attack(unsigned int p, int z, int arg);
void duel_field_stats(unsigned int p, int z, DuelStats *st);

typedef struct DuelAiSummon {          /* 8: card id and its condition, ends with 0x1379 */
    unsigned int id;
    int (*cond)(DuelAct *a, int b);
} DuelAiSummon;

extern DuelAiSummon duel_ai_summons[]; /* 0x089067D0 */
extern int duel_w_08913D08;

extern struct DuelDlgHead {
    int unk0;
} duel_dlg_cards;

int duel_ai_0881C118(DuelAct *a)
{
    return duel_count_monsters(1 - a->player) < duel_count_monsters(a->player);
}

int duel_ai_0881C160(DuelAct *a)
{
    if (duel_08809F2C(a->player) > 0) {
        return 0;
    }
    return duel_count_monsters(a->player) > 1;
}

#ifdef NON_MATCHING /* the original schedules a nop before the ATK test of the target */
int duel_ai_0881C1AC(DuelAct *a, int b)
{
    int z;

    if (duel_088154D8(a->player) != 0) {
        return 1;
    }
    for (z = 0; z < 5; z++) {
        if (duel_0884E3E4(a, 1 - a->player, z) != 0 && duel_act_immune(a, 1 - a->player, z) == 0 &&
            duel_slot_ef098(1 - a->player, z, a->id) != 0) {
            int id = field_get_card(1 - a->player, z);

            if (id != 0x13C3 && id != 0x1596 && duel_field_atk(1 - a->player, z) >= 1800) {
                return 1;
            }
        }
    }
    return 0;
}
#endif

int duel_ai_0881C2D0(DuelAct *a, int b)
{
    if (duel_count_monsters_fn(a->player, card_is_in_group_1911) != 0) {
        return duel_ai_0881C1AC(a, b);
    }
    return 0;
}

int duel_ai_0881C324(DuelAct *a)
{
    if (duel_count_name(a->player, 0x16A0) != 0) {
        return 1;
    }
    if (duel_count_monsters(a->player) != 0) {
        if (duel_hand_count_name(a->player, 0x18CA) != 0) {
            return 1;
        }
        if (duel_08809F2C(a->player) > 0) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* control flow of the ATK and owner tests not reproduced */
int duel_ai_0881C3B0(DuelAct *a)
{
    int y;
    int x;
    DuelCard *c;
    unsigned int id;

    if (duel_08809B50(a->player, a->id, 0) < 0) {
        return 0;
    }
    c = duel_0884C7CC();
    id = c->id;
    duel_08809FC4(1 - a->player, &x, &y, 0);
    if (!(card_get_atk(id) > 0x6A4 && MAX(x, y) < card_get_atk(id))) {
        if (card_get_attribute_icon(id) == 0 || card_get_special_group(id) != 0) {
            return 0;
        }
    }
    if (c->owner == a->player) {
        return 1;
    }
    if (duel_088070A8(id) == 0) {
        return 1;
    }
    return 0;
}
#endif

int duel_ai_0881C4DC(DuelAct *a)
{
    if (duel_08809B50(a->player, a->id, 0) >= 0 && card_get_def(duel_0884C7CC()->id) > 0x708) {
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* control flow of the ATK/DEF choice not reproduced */
int duel_ai_0881C538(DuelAct *a)
{
    int y;
    int x;
    int v;
    unsigned int id;

    if (duel_08809B50(a->player, a->id, 0) >= 0 && duel_088086D4(a->id, duel_dlg_cards.unk0) == 0) {
        id = duel_0884C7CC()->id;
        if (a->id == 0x1881 || a->id == 0x1536) {
            v = MAX(card_get_def(id), card_get_atk(id));
        } else if (a->id == 0x1880 || a->id == 0x14F7) {
            v = card_get_def(id);
        } else {
            v = card_get_atk(id);
        }
        duel_08809FC4(1 - a->player, &x, &y, 0);
        if (v > 0x6A4 && MAX(x, y) < v) {
            return 1;
        }
        if (v > 1000 && card_get_attribute_icon(id) != 0 && card_get_special_group(id) == 0) {
            return 1;
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* control flow of the ATK/DEF maximum not reproduced */
int duel_ai_0881C6D4(DuelAct *a)
{
    int v = 0;
    int y;
    int x;
    unsigned int id;

    if (duel_08809B50(1 - a->player, a->id, 0) >= 0) {
        id = duel_0884C7CC()->id;
        v = MAX(card_get_def(id), card_get_atk(id));
    }
    if (duel_08809B50(a->player, a->id, 0) < 0) {
        return 0;
    }
    id = duel_0884C7CC()->id;
    duel_08809FC4(1 - a->player, &x, &y, 0);
    v = MAX(MAX(x, y), v);
    if (card_get_atk(id) < 0x6A5 || card_get_def(id) < v) {
        return card_get_attribute_icon(id) == 1;
    }
    return 1;
}
#endif

int duel_ai_0881C850(DuelAct *a)
{
    unsigned int t = duel_088077D4(a->player, 0);

    if (t == -1) {
        return 0;
    }
    return duel_field_atk(t & 0xFF, t & 0xFF) < 0x960;
}

int duel_ai_0881C894(DuelAct *a)
{
    if (duel.players[a->player & 1].lp > 4000) {
        return 0;
    }
    return duel_088BFFD8(a->player, a->id, 0) > 0;
}

#ifdef NON_MATCHING /* copy of the action and table walk not reproduced */
int duel_ai_0881C8F4(DuelAct *a)
{
    DuelAct t = *a;
    DuelAiSummon *e;

    if (duel.players[a->player & 1].lp > 2000) {
        for (e = duel_ai_summons; e->id != 0x1379; e++) {
            t.id = e->id;
            if (card_get_type(t.id) == 0x16 && duel_grave_count_id(1 - a->player, t.id) != 0 &&
                e->cond(&t, 0) != 0 && duel_08869ACC(&t, 0) != 0) {
                duel_w_08913D08 = t.id;
                return 1;
            }
        }
    }
    return 0;
}
#endif

int duel_ai_0881CA34(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].flags >> 17 & 1) {
        return 0;
    }
    return duel_can_special_summon(p) != 0;
}

int duel_ai_0881CA90(DuelAct *a)
{
    DuelStats st;
    int m;
    int z;

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel_count_monsters(1 - a->player) > 0) {
        m = 0;
        for (z = 0; z < 5; z++) {
            if (duel.players[a->player & 1].field[z].card.id != 0 && duel.players[a->player & 1].field[z].pos == 0) {
                m = MAX(m, duel_field_atk(a->player, z));
            }
        }
        for (z = 0; z < 5; z++) {
            unsigned int q = 1 - a->player;

            if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].pos != 0) {
                duel_field_stats(q, z, &st);
                if (st.def >= m && st.atk < m) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_ai_0881CC24(DuelAct *a)
{
    int z;
    int n = 0;

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    for (z = 0; z <= 4; z++) {
        n += duel_can_attack(a->player, z, 1);
    }
    return !(n < 2);
}

int duel_ai_0881CCA0(DuelAct *a)
{
    DuelStats st;
    int m;
    int z;

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    if (duel_count_monsters(1 - a->player) > 0) {
        m = 0;
        for (z = 0; z < 5; z++) {
            if (duel.players[a->player & 1].field[z].card.id != 0 && duel.players[a->player & 1].field[z].pos == 0) {
                m = MAX(m, duel_field_atk(a->player, z));
            }
        }
        for (z = 0; z < 5; z++) {
            unsigned int q = 1 - a->player;

            if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].pos == 0) {
                duel_field_stats(q, z, &st);
                if (st.atk >= m && st.def < m) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_ai_0881CE34(DuelAct *a)
{
    DuelStats st;
    int m = 0;
    int z;

    if (duel_08836868(a->player) == 0) {
        return 0;
    }
    for (z = 0; z < 5; z++) {
        if (duel.players[a->player & 1].field[z].card.id != 0 && duel.players[a->player & 1].field[z].pos == 0) {
            m = MAX(m, duel_field_atk(a->player, z));
        }
    }
    for (z = 0; z < 5; z++) {
        if (duel_0884E3E4(a, 1 - a->player, z) != 0) {
            duel_field_stats(1 - a->player, z, &st);
            if (duel.players[(1 - a->player) & 1].field[z].pos == 0) {
                if (st.atk >= m && st.def < m) {
                    return 1;
                }
            } else {
                if (st.def >= m && st.atk < m) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* the original folds the faceup offset into the base address of duel */
int duel_ai_0881CFF4(DuelAct *a)
{
    int r;

    if (a->zone >= 10) {
        return 0;
    }
    if (duel_08809F2C(a->player) > 0) {
        return 0;
    }
    duel.players[a->player & 1].field[a->zone].faceup = 1;
    r = duel_08809F2C(a->player) > 0;
    duel.players[a->player & 1].field[a->zone].faceup = 0;
    return r;
}
#endif
