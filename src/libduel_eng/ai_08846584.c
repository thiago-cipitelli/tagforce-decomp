// @range 0x08846584-0x088480D0
// @asm 0x08846584-0x088480D0
/* Battle: ATK/DEF of the attacker and of the attack target with every modifier applied (duel_special.act). */
#include "duel.h"

typedef struct DuelAiView {            /* 0x08913CF4, see ai_0880B448.c */
    unsigned int player;
    int unk4[5];
    DuelStats st[2][5];                /* +0x18 monsters as the computer sees them */
} DuelAiView;

extern DuelAiView duel_w_08913CF4;
extern unsigned int duel_view_player;

void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_slot_count_src(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_active(int id);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_count_monsters(unsigned int p);
int duel_hand_count_type(unsigned int p, int type);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_effect_sum(unsigned int p, int z, unsigned int id);
int card_is_in_group_1a90(int id);
int card_is_in_group_1a6f(int id);
int card_is_in_group_1a6x(int id);
int duel_slot_counter_1a90(unsigned int p, int z);
int duel_count_1a90(void);
unsigned int duel_slot_immunity(unsigned int p, int z);
int duel_effect_applies(int id, unsigned int p, unsigned int ref);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_08846498(unsigned int p, int z, int id, int f);
void duel_0884DAF0(int a, int b);

/* x * 2^-n, rounding up at each halving (n < 0 doubles) */
#define DUEL_HALVE(x, n, i)                             \
    do {                                                \
        for ((i) = 0; (i) < -(n); (i)++) (x) <<= 1;     \
        for ((i) = 0; (i) < (n); (i)++) (x) = ((x) + 1) >> 1; \
    } while (0)

/* act[k].pad14: 0 base ATK, 1 base DEF, 2 ATK, 3 DEF (>= 0), 4 attribute, 5 level, 6/7 destroyed flags (duel_088482F8) */

#ifdef NON_MATCHING /* 7 KB com os laços de divisão desenrolados; mesma forma geral, difere a escolha de registradores e o cálculo de jogador/zona do primeiro laço */
/* mode: 0 simulation, 1 real battle, >= 2 also shows the cards that apply; view: take the stats from duel_w_08913CF4 */
void duel_08846584(int mode, int view)
{
    unsigned int ap = duel_special.player[0];   /* attacker */
    unsigned int tp = duel_special.player[1];   /* attacked player */
    unsigned int op;
    unsigned int pk;
    int zk;
    int k;
    int i;
    int add;                           /* ATK modifier */
    int add2;                          /* DEF modifier */
    int n;                             /* halvings of the attacker's modifier */
    int h;
    int n2;
    int h2;
    int nul;
    int sign;
    int c;
    int r;
    int x;
    int once;
    unsigned int e;
    unsigned int ep;
    unsigned int ez;
    DuelEffect *ef;
    DuelSpecAct *aa = &duel_special.act[ap];
    DuelSpecAct *ta = &duel_special.act[1 - ap];
    DuelStats st[2];                   /* [ap] attacker, [1 - ap] target */

    /* both cards of the battle */
    for (k = 0; k < 2; k++) {
        if (k == ap) {
            pk = duel_special.player[0];
            zk = duel_special.zone[0];
        } else {
            pk = duel_special.player[1];
            zk = duel_special.zone[1];
        }
        if (view != 0) {
            st[k] = duel_w_08913CF4.st[pk][zk];
        } else {
            duel_field_stats(pk, zk, &st[k]);
        }
        duel_special.act[k].unk0 = pk;
        duel_special.act[k].unk4 = zk;
        duel_special.act[k].unk8 = duel.players[pk & 1].field[zk].pos;
        duel_special.act[k].uid = CARD_UID(duel.players[pk & 1].field[zk].card);
        duel_special.act[k].id = st[k].id;
        duel_special.act[k].pad14[0] = 0;
        duel_special.act[k].pad14[1] = 0;
        duel_special.act[k].pad14[2] = 0;
        duel_special.act[k].pad14[3] = 0;
        duel_special.act[k].pad14[4] = st[k].attr;
        duel_special.act[k].pad14[5] = st[k].level;
        duel_special.act[k].pad14[6] = 0;
        duel_special.act[k].pad14[7] = 0;
        duel_special.act[k].unk34 = 0;
        duel_special.recs[k].w0 = 0;
    }
    if (duel_special.unk8 != 0) {      /* direct attack: no target */
        ta->id = 0;
        ta->pad14[2] = 0;
        ta->pad14[3] = 0;
    }

    /* attacker */
    add = 0;
    add2 = 0;
    n = duel_slot_count_src(ap, duel_special.zone[0], 0x1381);
    op = 1 - ap;
    h = duel_find_effect_card(op, 0x1905, -1);
    h += duel_find_effect_card(ap, 0x1905, duel_special.zone[0]);
    n += h;
    nul = 0;
    if ((st[ap].attr == 4 || (st[ap].mask & 0x10)) && duel_count_active(0x1951) != 0) {
        nul = 1;
    }
    if (duel_slot_e8170(ap, duel_special.zone[0], 0x1955) != 0 && duel_count_monsters(op) == 2) {
        n--;
    }
    if (SLOT_ACTIVE(&duel.players[ap & 1].field[duel_special.zone[0]])) {
        switch (aa->id) {
        case 0x13A7:
            if (view != 0 && duel.players[ap & 1].lp > 2000) {
                add += 3000;
            }
            break;
        case 0x14D7:
            if (view != 0) {
                add += duel_hand_count_type(ap, 1) * 1000;
            }
            break;
        case 0x13AA:
            if (duel_special.unk8 == 0 && view != 0 && ta->pad14[4] == 15) {
                add += 2000;
                add2 += 2000;
            }
            break;
        case 0x14CC:
            if (duel_special.unk8 == 0 && view != 0 && ta->pad14[4] != 0 &&
                ta->pad14[4] == duel.players[ap & 1].field[duel_special.zone[0]].link) {
                add += 1000;
            }
            break;
        case 0x1643:
            if (duel_special.unk8 == 0) {
                add += st[op].batk;
            }
            break;
        case 0x1956:
            if (aa->unk8 != 0) {
                st[ap].atk /= 2;
                n++;
            }
            break;
        case 0x18F2:
            if (view != 0 && duel_special.unk8 == 0) {
                add += 500;
            }
            break;
        }
    }
    if (duel_slot_has_effect(ap, 0xB, 0x1846) != 0) {
        add += duel_slot_affected_ex(ap, duel_special.zone[0], 0x1846) * 1500;
    }
    if (duel_special.unk8 == 0 && card_is_in_group_1a90(ta->id) != 0) {
        c = duel_slot_counter_1a90(ap, duel_special.zone[0]);
        x = c * duel_count_1a90() * 300;
        add -= x;
        add2 -= x;
    }
    for (e = duel.players[ap & 1].field[duel_special.zone[0]].effects; e != 0;) {
        ef = &duel.effects[e];
        e = ef->next;
        switch (ef->src) {
        case 0x1119:
            if ((ef->kind & 0xF) == 4) {
                st[ap].atk = 0;
                add = 0;
            }
            break;
        case 0x13A7:
            if (ef->arg == 1) {
                add += 3000;
            }
            break;
        case 0x13AA:
            add += 2000;
            add2 += 2000;
            break;
        case 0x14CC:
            add += 1000;
            break;
        case 0x152D:
            st[ap].atk = 2400;
            nul = 1;
            break;
        }
    }
    if (duel_slot_has_effect(0, 0xB, 0x1257) != 0) {   /* modifiers are reversed */
        add = -add;
        add2 = -add2;
    }
    DUEL_HALVE(add, n, i);
    for (i = 0; i < h; i++) {
        add2 = (add2 + 1) >> 1;
    }
    if (nul) {
        add = 0;
    }
    st[ap].atk += add;
    st[ap].def += add2;

    if (duel_special.unk8 == 0) {
        /* target */
        add = 0;
        add2 = 0;
        sign = duel_slot_has_effect(0, 0xB, 0x1257) != 0 ? -1 : 1;
        n2 = duel_slot_count_src(tp, duel_special.zone[1], 0x1381);
        h2 = duel_find_effect_card(1 - tp, 0x1905, -1);
        h2 += duel_find_effect_card(tp, 0x1905, duel_special.zone[1]);
        n2 += h2;
        nul = 0;
        if ((st[ap].attr == 4 || (st[ap].mask & 0x10) != 0) && duel_count_active(0x1951) != 0) {
            nul = 1;
        }
        if (duel_slot_e8170(tp, duel_special.zone[1], 0x1955) != 0 && duel_count_monsters(1 - tp) == 2) {
            n2--;
        }
        if (SLOT_ACTIVE(&duel.players[tp & 1].field[duel_special.zone[1]])) {
            switch (ta->id) {
            case 0x13A7:
                if ((view != 0 && duel.players[tp & 1].lp > 2000) ||
                    duel_slot_effect_with(tp, duel_special.zone[1], 0x13A7, 1) != 0) {
                    add += 3000;
                }
                break;
            case 0x13AA:
                if ((view != 0 && aa->pad14[4] == 15) || duel_slot_has_effect(tp, duel_special.zone[1], 0x13AA) != 0) {
                    add += 2000;
                    add2 += 2000;
                }
                break;
            case 0x14CC:
                if ((view != 0 && aa->pad14[4] != 0 &&
                     aa->pad14[4] == duel.players[tp & 1].field[duel_special.zone[1]].link) ||
                    duel_slot_has_effect(tp, duel_special.zone[1], 0x14CC) != 0) {
                    add += 1000;
                }
                break;
            case 0x1643:
                if (duel_special.unk8 == 0) {
                    add += st[ap].batk;
                }
                break;
            case 0x18F2:
                if (view != 0) {
                    add -= 500;
                }
                break;
            }
        }
        add += duel_slot_effect_sum(tp, duel_special.zone[1], 0x1853);
        if (card_is_in_group_1a90(aa->id) != 0) {
            c = duel_slot_counter_1a90(tp, duel_special.zone[1]);
            x = c * duel_count_1a90() * 300;
            add -= x;
            add2 -= x;
        }
        DUEL_HALVE(add, n2, i);
        for (i = 0; i < h2; i++) {
            add2 = (add2 + 1) >> 1;
        }
        if (nul) {
            add = 0;
        }
        st[op].atk += sign * add;
        st[op].def += sign * add2;

        /* 0x1238: the attacker gains half of the target's ATK per card */
        r = duel_slot_affected_ex(ap, duel_special.zone[0], 0x1238);
        once = 0;
        x = r * ((st[op].atk + 1) >> 1);
        DUEL_HALVE(x, n, i);
        st[ap].atk += x;
        if (mode >= 2) {
            for (e = duel.players[ap & 1].field[duel_special.zone[0]].effects; e != 0;) {
                ef = &duel.effects[e];
                e = ef->next;
                if ((ef->kind & 0xF) == 10) {
                    ep = EFF_PLAYER(ef);
                    ez = EFF_ZONE(ef);
                    if (duel_slot_e8170(ep, ez, 0x1238) != 0) {
                        duel_088C4448(ep, ez, CARD_UID(duel.players[ep & 1].field[ez].card));
                    }
                }
            }
        }
        /* 0x18FF / 0x1A66: +1000 when weaker than the target (once) */
        if (card_is_in_group_1a6f(aa->id) != 0 &&
            duel_effect_applies(0x18FF, ap, duel_slot_immunity(ap, duel_special.zone[0])) != 0) {
            once = 1;
        }
        if (card_is_in_group_1a6x(aa->id) != 0 &&
            duel_effect_applies(0x1A66, ap, duel_slot_immunity(ap, duel_special.zone[0])) != 0) {
            once = 1;
        }
        if (once && st[ap].atk < st[op].atk) {
            x = 1000;
            DUEL_HALVE(x, n, i);
            once = 0;
            st[ap].atk += sign * x;
        }
        /* 0x159E on the target: +2500 against a strong attacker (destroyed above 1000 ATK) */
        if ((c = duel_slot_affected(tp, duel_special.zone[1], 0x159E)) != 0) {
            if (st[op].atk > 1000) {
                if (mode != 0) {
                    duel_08846498(tp, duel_special.zone[1], 0x159E, 1);
                }
            } else if ((aa->unk8 != 0 ? st[ap].def : st[ap].atk) >= 2500) {
                if (c >= 2 && mode != 0) {     /* only one copy applies */
                    duel_08846498(tp, duel_special.zone[1], 0x159E, 0);
                }
                if (duel_slot_affected_ex(tp, duel_special.zone[1], 0x159E) != 0) {
                    x = 2500;
                    DUEL_HALVE(x, n2, i);
                    st[op].atk += sign * x;
                    x = ((x + 1) >> 1) * r;
                    DUEL_HALVE(x, n, i);
                    st[ap].atk += sign * x;
                }
            }
        }
        if (once && st[ap].atk < st[op].atk) {
            x = 1000;
            DUEL_HALVE(x, n, i);
            st[ap].atk += sign * x;
        }
        /* 0x159E on the attacker */
        if ((c = duel_slot_affected(ap, duel_special.zone[0], 0x159E)) != 0) {
            if (st[ap].atk > 1000) {
                if (mode != 0) {
                    duel_08846498(ap, duel_special.zone[0], 0x159E, 1);
                }
            } else if ((ta->unk8 != 0 ? st[op].def : st[op].atk) >= 2500) {
                if (c >= 2 && mode != 0) {     /* only one copy applies */
                    duel_08846498(ap, duel_special.zone[0], 0x159E, 0);
                }
                if (duel_slot_affected_ex(ap, duel_special.zone[0], 0x159E) != 0) {
                    x = 2500;
                    DUEL_HALVE(x, n, i);
                    st[ap].atk += sign * x;
                }
            }
        }
    }
    /* 0x150A: shown when the ATK reaches 1300 */
    if (st[ap].atk >= 1300 && mode != 0) {
        duel_08846498(ap, duel_special.zone[0], 0x150A, 1);
    }
    if (duel_special.unk8 == 0 && st[op].atk >= 1300 && mode != 0) {
        duel_08846498(tp, duel_special.zone[1], 0x150A, 1);
    }
    for (k = 0; k < 2; k++) {
        duel_special.act[k].pad14[0] = st[k].batk < 0 ? 0 : st[k].batk;
        duel_special.act[k].pad14[1] = st[k].bdef < 0 ? 0 : st[k].bdef;
        duel_special.act[k].pad14[2] = st[k].atk < 0 ? 0 : st[k].atk;
        duel_special.act[k].pad14[3] = st[k].def < 0 ? 0 : st[k].def;
        if (mode != 0 && view == 0 && duel_special.act[k].unk0 == duel_view_player) {
            duel_0884DAF0(0, st[k].atk);
        }
    }
}
#endif
