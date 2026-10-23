// @range 0x08815E90-0x08818C34
// @asm 0x08815E90-0x0881616C
// @asm 0x0881616C-0x088163E4
// @asm 0x0881657C-0x08816A74
// @asm 0x08816A74-0x08816D74
// @asm 0x08816D74-0x08816EA4
// @asm 0x088170EC-0x088171C8
// @asm 0x088172F0-0x0881739C
// @asm 0x08817598-0x08818C34
/* Computer player: picking one of its cards (zone mask bit player * 16 + zone) to send away. */
#include "duel.h"
#include "card_db.h"

int duel_rand(int n);
int duel_08815548(unsigned int p, unsigned int mask, int atk, int def);
int duel_can_attack(unsigned int p, int z, int arg);
void duel_0880546C(unsigned int p, int z, DuelStats *st);
int duel_0880AA04(int id, int f);
int duel_08815E20(unsigned int p, unsigned int mask);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
int field_get_card(unsigned int p, int z);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_hand_find_name(unsigned int p, int id);
int duel_0881541C(unsigned int p, unsigned short id);
int duel_08806730(unsigned int p, int n, unsigned short id);
int duel_08807B38(int id, int a, int b);
int duel_grave_count_id(unsigned int p, unsigned int id);
void duel_088A8D40(DuelAct *a, unsigned int p, int z);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_088071E8(int id);
int duel_field_atk(unsigned int p, int z);
int duel_field_base_atk(unsigned int p, int z);
unsigned int duel_d_find_name(unsigned int p, int id);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_088BFFD8(unsigned int p, unsigned int key, int c);
int duel_field_level(unsigned int p, int z);
int duel_08845070(int id);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_list_card_level(unsigned int p, int i);
unsigned int duel_0884E4A0(DuelAct *a);
int duel_088159C8(unsigned int p, unsigned int mask, int (*fn)(unsigned int p, int z));
int duel_08815950(DuelAct *a, unsigned int p, int atk, int def, int z);
int duel_088157AC(unsigned int p, unsigned int mask, unsigned int pos);
int duel_0880A17C(unsigned int p, unsigned int q, int skip, int fa, int fd);
int duel_0880A54C(DuelAct *a, unsigned int p, int skip, int any);
int duel_field_def(unsigned int p, int z);
int duel_find_st(unsigned int p, int id);

extern int duel_atk_zone;              /* 0x08913F2C */
extern unsigned int duel_atk_tp;       /* 0x08913F14 */
extern int duel_atk_tz;                /* 0x08913F30 */
extern unsigned int duel_w_08913CF4;
extern int duel_w_08913EDC[5];
extern DuelAct *duel_w_089147DC;       /* one past the card being resolved */

extern unsigned int duel_turn_player;
extern int duel_phase;

#ifdef NON_MATCHING /* mesma forma; os registradores de i, t e do ponteiro da lista (s3-s5) saem trocados nos laços de busca */
/* A spell/trap of p in the mask: a face-down one of kind 2-4 first, then face-up ones, then sets. */
int duel_08815E90(unsigned int p, unsigned int mask)
{
    int z[11];
    int n = 0;
    int i;
    int j;
    int t;
    unsigned int b;

    for (i = 5, b = p * 16 + 5; i <= 10; i++) {
        if (mask & (1 << b)) {
            z[n++] = i;
        }
        b++;
    }
    for (i = n - 1; i > 0; i--) {
        j = duel_rand(i + 1);
        t = z[i];
        z[i] = z[j];
        z[j] = t;
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel.players[p & 1].field[t].flags >> 1 & 1) {
            switch (card_get_prop5_hi(duel.players[p & 1].field[t].card.id)) {
            case 2:
            case 3:
            case 4:
                return t;
            }
        }
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel.players[p & 1].field[t].faceup != 0 && !(duel.players[p & 1].field[t].flags >> 1 & 1)) {
            return t;
        }
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel.players[p & 1].field[t].faceup == 0 && duel.players[p & 1].field[t].flags >> 20 & 1) {
            return t;
        }
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel.players[p & 1].field[t].faceup == 0) {
            return t;
        }
    }
    if (n > 0) {
        return z[duel_rand(n)];
    }
    return -1;
}
#endif

#ifdef NON_MATCHING /* mesma forma; os registradores de i, t e do ponteiro da lista (s3-s5) saem trocados nos laços de busca */
/* A monster of p in the mask to give up: one marked (flag 7) first, else the best by the attack plan. */
int duel_0881616C(unsigned int p, unsigned int mask, int a)
{
    int n = 0;
    int i;
    int t;
    unsigned int pb = p << 4;
    unsigned int m;

    int z[11];
    int j;
    unsigned int b;
    DuelStats st;

    for (i = 0, b = pb; i <= 10; i++) {
        if (mask & (1 << b)) {
            z[n++] = i;
        }
        b++;
    }
    for (i = n - 1; i > 0; i--) {
        j = duel_rand(i + 1);
        t = z[i];
        z[i] = z[j];
        z[j] = t;
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel.players[p & 1].field[t].flags >> 7 & 1 && duel_0880AA04(duel.players[p & 1].field[t].card.id, a) != 0) {
            return t;
        }
    }
    if (duel_phase == 3 && duel_turn_player == p) {
        m = 0;
        for (i = 0; i <= 10; i++) {
            if ((mask & 1 << pb) && duel_can_attack(p, i, 1)) {
                m |= 1 << pb;
            }
            pb++;
        }
        t = duel_08815548(p, m, 1, 0);
        if (t >= 0) {
            duel_0880546C(p, t, &st);
            if (st.atk >= duel.players[(1 - p) & 1].lp) {
                return t;
            }
        }
    }
    t = duel_08815548(p, mask, 1, 1);
    if (t >= 0) {
        return t;
    }
    return duel_08815E90(p, mask);
}
#endif

int duel_088163E4(unsigned int p, unsigned int mask)
{
    int z[11];
    unsigned int m = 0;
    int n = 0;
    int i;
    int j;
    int t;
    unsigned int pb = p << 4;
    unsigned int b;

    for (i = 0, b = pb; i <= 10; i++) {
        if (mask & (1 << b)) {
            z[n++] = i;
        }
        b++;
    }
    for (i = n - 1; i > 0; i--) {
        j = duel_rand(i + 1);
        t = z[i];
        z[i] = z[j];
        z[j] = t;
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel_0880AA04(duel.players[p & 1].field[t].card.id, 0) != 0) {
            m |= 1 << (t + pb);
        }
    }
    t = duel_08815548(p, mask ^ m, -1, -1);
    if (t >= 0) {
        return t;
    }
    return duel_08815E20(p, mask);
}

#ifdef NON_MATCHING /* função grande com cinco varreduras da lista embaralhada; mantida em asm */
/* A card of p in the mask (zones) the computer gives up: one it does not control, a summon material
   whose sibling is in the hand, a card it wants in the graveyard, ... then the weakest. */
int duel_0881657C(unsigned int p, unsigned int mask, int strong)
{
    int z[11];
    int n = 0;
    int i;
    int j;
    int t;
    unsigned int k;
    int id;
    unsigned int b;
    unsigned int m = 0;
    DuelStats st;

    for (i = 0, b = p << 4; i <= 10; i++) {
        if (mask & (1 << b)) {
            z[n++] = i;
        }
        b++;
    }
    for (i = n - 1; i > 0; i--) {
        j = duel_rand(i + 1);
        t = z[i];
        z[i] = z[j];
        z[j] = t;
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (p != duel_field_controller(p, t, 1)) {
            return t;
        }
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel.players[p & 1].field[t].flags >> 5 & 1) {
            continue;
        }
        switch (field_get_card(p, t)) {
        case 0x1521:
        case 0x1798:
        case 0x17E8:
        case 0x17E9:
        case 0x1873:
        case 0x1874:
            for (k = 0; k < duel.players[p & 1].n_hand; k++) {
                id = duel.players[p & 1].hand[k].id;
                if ((int)card_get_level(id) >= 7 && card_attribute_icon_in_2_3(id) == 0 &&
                    card_get_kind(id) == card_get_kind(field_get_card(p, t))) {
                    return t;
                }
            }
            break;
        }
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        switch (duel.players[p & 1].field[t].card.id) {
        case 0x1A8E:
        case 0x163F:
        case 0x14F3:
        case 0x11E4:
        case 0xFD6:
            return t;
        }
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel_slot_affected(p, t, 0x149D) != 0 || duel_slot_affected(p, t, 0x1286) != 0 ||
            duel_slot_affected(p, t, 0x13F3) != 0 || duel_slot_affected(p, t, 0x14B2) != 0) {
            return t;
        }
    }
    for (i = 0; i < n; i++) {
        t = z[i];
        if (duel_0880AA04(duel.players[p & 1].field[t].card.id, 0) != 0) {
            m |= 1 << (t + p * 16);
        }
    }
    t = duel_08815548(p, mask ^ m, -1, -1);
    if (strong == 0) {
        if (t < 0) {
            t = duel_08815E20(p, mask);
        }
    } else if (t < 0) {
        t = -1;
    } else {
        duel_0880546C(p, t, &st);
        if (MAX(st.atk, st.def) > 1400) {
            t = -1;
        }
    }
    return t;
}
#endif

#ifdef NON_MATCHING /* desvios bltzl das atribuições de candidatos e alocação de registradores (big/i) não reproduzidos */
/* Hand index of the card p would discard for a cost (fn tells which cards qualify); -1 if none. */
int duel_08816A74(unsigned int p, int (*fn)(), int a, int keep)
{
    int big = -1;
    int x179a = -1;
    int x1181 = -1;
    int lim3 = -1;
    int other = -1;
    unsigned int i;
    int id;
    int lv;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (!fn(p, 0xB, i)) {
            continue;
        }
        id = duel.players[p & 1].hand[i].id;
        if ((int)card_get_level(id) > 4 && card_attribute_icon_in_2_3(id) == 0) {
            if (big >= 0) {
                lv = card_get_level(id);
                if ((int)card_get_level(duel.players[p & 1].hand[big].id) < lv) {
                    big = i;
                }
            } else {
                big = i;
            }
        }
        switch (id) {
        case 0x1181:
            x1181 = i;
            break;
        case 0x179A:
            x179a = i;
            break;
        case 0x197F:
        case 0x19FE:
        case 0x1AAA:
        case 0x1B2A:
            return i;
        default:
            if (duel_08807B38(id, a, 1) != 0) {
                return i;
            }
            break;
        }
        if (duel_08806730(p, 1, id) == 0 && card_get_limit(id) == 3) {
            if (lim3 >= 0) {
                if (duel_rand(2) != 0) {
                    lim3 = i;
                }
            } else {
                lim3 = i;
            }
        } else {
            if (other >= 0) {
                if (duel_rand(2) != 0) {
                    other = i;
                }
            } else {
                other = i;
            }
        }
    }
    if (duel_hand_find_name(p, 0x12EA) >= 0 || duel_hand_find_name(p, 0x1366) >= 0 || duel_0881541C(p, 0x137D) != 0) {
        if (big >= 0) {
            return big;
        }
    }
    if (duel_grave_count_id(p, 0x179A) != 0 && duel_phase != 5 && x179a >= 0) {
        return x179a;
    }
    if (x1181 >= 0) {
        return x1181;
    }
    if (lim3 >= 0) {
        return lim3;
    }
    if (keep != 0) {
        other = -1;
    }
    return other;
}
#endif

#ifdef NON_MATCHING /* os contadores dos dois laços aninhados (s2/s4) saem trocados */
/* Use the effect of a on a random zone of p or else of the opponent that accepts it. */
int duel_08816D74(DuelAct *a, unsigned int p)
{
    int z[11];
    int i;
    int j;
    int t;
    int k;

    for (i = 0; i <= 10; i++) {
        z[i] = i;
    }
    for (i = 10; i > 0; i--) {
        j = duel_rand(i + 1);
        t = z[i];
        z[i] = z[j];
        z[j] = t;
    }
    for (k = 0; k < 2; k++) {
        for (j = 0; j <= 10; j++) {
            if (!duel_0884E3E4(a, p, z[j])) {
                continue;
            }
            duel_088A8D40(a, p, z[j]);
            return 1;
        }
        p ^= 1;
    }
    return 0;
}
#endif

/* How much the computer wants to keep the card at (p, z): 3 a face-up 0x118A, 2 attribute 9, 1 listed. */
int duel_08816EA4(unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (duel.players[p & 1].field[z].faceup != 0 && id == 0x118A) {
        return 3;
    }
    if (card_is_attribute9(id) != 0) {
        return 2;
    }
    return duel_088071E8(id) != 0;
}

int duel_08816F48(unsigned int p, int z)
{
    int r = 0;

    if (card_get_special_group(duel.players[p & 1].field[z].card.id) != 0 && !duel.players[p & 1].field[z].faceup != 0) {
        r = 1;
    }
    return r;
}

int duel_08816FE4(unsigned int p, int z)
{
    int r = 0;

    if (card_get_special_group(duel.players[p & 1].field[z].card.id) != 0 && duel.players[p & 1].field[z].faceup != 0) {
        r = 1;
    }
    return r;
}

int duel_08817078(unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    return (int)card_get_level(id) > 4 || card_attribute_icon_in_2_3(id) != 0;
}

#ifdef NON_MATCHING /* a ordem da soma (id != 0x139C) + v + 1 e o recálculo do endereço do campo não reproduzidos */
/* Score of the monster at (p, z) as target of the attacking monster. */
int duel_088170EC(unsigned int p, int z)
{
    int atk = duel_field_atk(duel_special.player[0], duel_atk_zone);
    int v;
    DuelStats st;

    duel_0880546C(p, z, &st);
    if (duel.players[p & 1].field[z].pos != 0) {
        v = st.def + 1 - atk;
        if (v > 0) {
            v = 0;
        }
        return v + 1 + (duel.players[p & 1].field[z].card.id != 0x139C);
    }
    return (st.atk - atk) * 2;
}
#endif

int duel_088171C8(unsigned int p, int z)
{
    return duel_field_base_atk(p, z);
}

int duel_088171D0(unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].flags >> 7 & 1) {
        return 0;
    }
    return (z < 5) + 1;
}

/* Card the monster at (p, z) can turn into, when p has it in the hand or deck; else 0. */
int duel_08817220(unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;
    int out[2];
    int *to;

    if (card_is_in_group_198c(id) != 0) {
        to = out - 1 + card_id_remap_1a6c(id, out);
        if (id < *to && (duel_hand_find_name(p, *to) >= 0 || (int)duel_d_find_name(p, *to) >= 0)) {
            return *to;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* a ordem dos testes do alvo (zona extraída com ext antes do jogador) não reproduzida */
/* Is (p, z) a target of the card being resolved, played by the opponent of p? */
int duel_088172F0(unsigned int p, int z)
{
    DuelAct *a = duel_w_089147DC - 1;
    int i;
    unsigned int t;

    if (a != 0) {
        if (a->player != p) {
            for (i = 0; i < a->n_targets; i++) {
                t = duel_act_target(a, i);
                if ((t & 0xFF) == p && ((unsigned short)t >> 8 & 0xFF) == z) {
                    return 1;
                }
            }
        } else {
            return 0;
        }
    }
    return 0;
}
#endif

int duel_0881739C(unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0x17DE) {
        return 20000;
    }
    return 10000 - card_get_atk(duel.players[p & 1].field[z].card.id);
}

int duel_08817408(unsigned int p, int z)
{
    return duel_088BFFD8(p, 0x157A, field_get_card(p, z));
}

int duel_0881743C(unsigned int p, int z)
{
    return duel_08845070(duel.players[p & 1].field[z].card.id);
}

int duel_08817474(unsigned int p, int z)
{
    DuelStats st;

    duel_field_stats(p, z, &st);
    return st.def - st.atk;
}

int duel_0881749C(unsigned int p, int z)
{
    if (p == duel_turn_player) {
        return 2000 - duel_field_atk(p, z);
    }
    return duel_field_atk(p, z) - 2000;
}

int duel_088174E0(unsigned int p, int z)
{
    int lvl = duel_field_level(p, z);

    if (duel_088BFFD8(p, 0x1A98, lvl) != 0) {
        if (duel_088172F0(p, z) != 0) {
            return 100;
        }
        return lvl;
    }
    return 0;
}

int duel_08817550(unsigned int p, int i)
{
    return duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i]));
}

#ifdef NON_MATCHING /* 5,8 KB: escolha do alvo por carta; o original compartilha os blocos de cada estratégia; mantida em asm */
#define TARGET(q, z) do { duel_088A8D40(a, (q), (z)); return 1; } while (0)
#define IN_MASK(q, z) (mask & 1 << ((z) + (q) * 16))

/* Monster target for the destroying cards: the opponent's (or own for some), by duel_0880A54C. */
static inline int duel_ai_target_monster(DuelAct *a, unsigned int mask)
{
    unsigned int q = a->player;
    int z;

    switch (a->id) {
    case 0x1980:
        if (duel_atk_tp == q && IN_MASK(duel_atk_tp, duel_atk_tz)) {
            TARGET(duel_atk_tp, duel_atk_tz);
        }
        break;
    case 0x130D:
    case 0x130E:
    case 0x13E3:
    case 0x13E9:
    case 0x13EA:
    case 0x13F3:
    case 0x149D:
    case 0x1842:
    case 0x18D1:
        q = 1 - q;
        break;
    case 0x1237:
        if (duel.players[(1 - q) & 1].lp < duel.players[q & 1].lp) {
            q = 1 - q;
        }
        break;
    }
    z = duel_0880A54C(a, q, -1, 1);
    if (z >= 0) {
        TARGET(q, z);
    }
    z = duel_0880A54C(a, 1 - q, -1, 0);
    if (z >= 0) {
        TARGET(1 - q, z);
    }
    return -1;
}

/* The computer picks the targets of its card a (b: the card it answers, if any); id selects the strategy. */
int duel_08817598(DuelAct *a, DuelAct *b, int id)
{
    unsigned int mask = duel_0884E4A0(a);
    unsigned int p;
    unsigned int q;
    unsigned int t;
    unsigned int m;
    int z;
    int i;
    int v;
    unsigned int f;
    DuelStats st;

    if (mask == 0) {
        return 1;
    }
    p = a->player;
    duel_w_08913CF4 = p ^ a->opp;
    switch (id) {
    case 0x16A2:
        z = duel_088159C8(p, mask, duel_0881743C);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x158E:
    case 0x18F0:
        z = duel_088159C8(1 - a->player, mask, duel_088171D0);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        break;
    case 0x14D3:
    case 0x1538:
        if (b != 0 && b->n_targets == 1 && ((unsigned short)duel_act_target(b, 0) >> 8) < 11) {
            t = duel_act_target(b, 0);
            if (IN_MASK(t & 0xFF, (unsigned short)t >> 8)) {
                TARGET(t & 0xFF, (unsigned short)t >> 8);
            }
        }
        if (a->kind - 0x12 < 2 && duel_special.player[0] == 1 - a->player &&
            IN_MASK(duel_special.player[0], duel_atk_zone)) {
            TARGET(duel_special.player[0], duel_atk_zone);
        }
        /* fall through */
    case 0x1694:
    case 0x172A:
    case 0x178E:
        z = duel_088159C8(a->player, mask, duel_08816FE4);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        z = duel_08815548(1 - a->player, mask, 1, 0);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        break;
    case 0x184B:
        if (b != 0 && b->n_targets == 1 && ((unsigned short)duel_act_target(b, 0) >> 8) < 11) {
            t = duel_act_target(b, 0);
            if (IN_MASK(t & 0xFF, (unsigned short)t >> 8)) {
                TARGET(t & 0xFF, (unsigned short)t >> 8);
            }
        }
        return duel_08816D74(a, a->player);
    case 0x16CE:
        for (z = 0; z < 5; z++) {
            if (IN_MASK(a->player, z) && (unsigned int)duel_w_08913EDC[z] > 1 && duel_field_def(a->player, z) != 0) {
                TARGET(a->player, z);
            }
        }
        break;
    case 0x1978:
        for (z = 0; z < 5; z++) {
            q = a->player;
            if (IN_MASK(q, z) && duel_088BFFD8(q, a->id, duel.players[q & 1].field[z].card.id) != 0) {
                TARGET(a->player, z);
            }
        }
        break;
    case 0x188A:
        z = duel_088159C8(p, mask, duel_08816FE4);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x157A:
        z = duel_088159C8(p, mask, duel_08817408);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x15B8:
        if (b != 0 && b->player != p) {
            for (i = 0; i < b->n_targets; i++) {
                t = duel_act_target(b, i);
                if ((t & 0xFF) == a->player && ((unsigned short)t >> 8) < 5 && IN_MASK(t & 0xFF, (unsigned short)t >> 8)) {
                    TARGET(t & 0xFF, (unsigned short)t >> 8);
                }
            }
        }
        break;
    case 0x146B:
        z = duel_08815950(a, 1 - p, -1, 1, -1);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        break;
    case 0x138D:
    case 0x1475:
        q = 1 - a->player;
        z = duel_08815950(a, q, 1, 0, -1);
        if (z < 0) {
            q = 1 - q;
            z = duel_08815950(a, q, 1, 0, -1);
        }
        if (z >= 0) {
            TARGET(q, z);
        }
        break;
    case 0x1326:
    case 0x148D:
        return duel_08816D74(a, a->player);
    case 0x13FE:
        if (duel_turn_player == p) {
            return duel_08816D74(a, p);
        }
        break;
    case 0x10E6:
    case 0x130A:
    case 0x1581:
    case 0x1913:
        /* a monster of the opponent this one beats in battle */
        v = 0;
        for (z = 0; z < 5; z++) {
            if (duel.players[a->player & 1].field[z].card.id != 0 && duel.players[a->player & 1].field[z].pos == 0 &&
                v <= duel_field_atk(a->player, z)) {
                v = duel_field_atk(a->player, z);
            }
        }
        if (v == 0) {
            v = duel_0880A17C(a->player, a->player, -1, 1, 0);
        }
        a->n7 = 0;
        for (z = 0; z < 5; z++) {
            q = 1 - a->player;
            if (IN_MASK(q, z)) {
                duel_0880546C(q, z, &st);
                q = 1 - a->player;
                if (duel.players[q & 1].field[z].pos == 0) {
                    if (v <= st.atk && st.def < v) {
                        TARGET(q, z);
                    }
                } else if (v <= st.def && st.atk < v) {
                    TARGET(q, z);
                }
            }
        }
        break;
    case 0x12F1:
    case 0x12F2:
    case 0x1329:
    case 0x132A:
    case 0x1492:
    case 0x192D:
    case 0x16E3:
        /* the monster of p in the current battle */
        q = a->player;
        z = duel_special.player[0] != q ? duel_atk_tz : duel_atk_zone;
        if (IN_MASK(q, z)) {
            TARGET(q, z);
        }
        return duel_08816D74(a, q);
    case 0x190E:
        z = duel_088159C8(p, mask, duel_0881749C);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        z = duel_088159C8(1 - a->player, mask, duel_0881749C);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        break;
    case 0x12BB:
    case 0x1A0D:
        z = duel_088159C8(1 - a->player, mask, duel_088171C8);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        break;
    case 0x1889:
        m = 0;
        for (z = 0; z <= 10; z++) {
            if (IN_MASK(a->player, z)) {
                f = duel.players[a->player & 1].field[z].flags;
                if ((f >> 23 & 1) != (f >> 22 & 1) &&
                    card_attribute_icon_in_2_3(duel.players[a->player & 1].field[z].card.id) == 0) {
                    m |= 1 << (z + a->player * 16);
                }
            }
        }
        z = duel_08815548(a->player, m, 1, 0);
        if (z < 0) {
            z = duel_08815548(a->player, mask, 1, 0);
        }
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x1669:
        z = duel_088157AC(p, mask, -1);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x1853:
    case 0x1945:
        z = duel_08815950(a, a->player, 1, 0, -1);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x14B9:
    case 0x1612:
    case 0x1624:
    case 0x17FC:
    case 0x1893:
    case 0x1A7C:
        q = a->player;
        z = duel_08815950(a, q, 1, 1, -1);
        if (z < 0) {
            q = 1 - q;
            z = duel_08815950(a, q, -1, -1, -1);
        }
        if (z >= 0) {
            TARGET(q, z);
        }
        break;
    case 0x13EB:
    case 0x1485:
    case 0x1487:
    case 0x1280:
    case 0x128A:
    case 0x1391:
    case 0x15F5:
        q = 1 - a->player;
        z = duel_08815950(a, q, 1, 1, -1);
        if (z < 0) {
            q = 1 - q;
            z = duel_08815950(a, q, -1, -1, -1);
        }
        if (z >= 0) {
            TARGET(q, z);
        }
        break;
    case 0x1690:
        if (duel_turn_player != p && IN_MASK(p, a->zone)) {
            TARGET(p, a->zone);
        }
        z = duel_08815950(a, p, 1, 1, -1);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x175D:
        z = duel_088159C8(p, mask, duel_08817474);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x1537:
        z = duel_088159C8(p, mask, duel_08816F48);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x171A:
        z = duel_088159C8(1 - p, mask, duel_08817078);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        break;
    case 0x1A6D: case 0x15E9: case 0x19DB: case 0x16BA: case 0x16AB: case 0x18C6: case 0x18BA:
    case 0x186E: case 0x1844: case 0x1727: case 0x16CB: case 0x1685: case 0x1656: case 0x1284:
    case 0x1281: case 0x1466: case 0x13B0: case 0x1322: case 0x12FC: case 0x1227: case 0x11C3:
        /* destroy a card of the opponent, else one of p's own it wants gone */
        z = duel_0881616C(1 - a->player, mask, 0);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        z = duel_088163E4(a->player, mask);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x132D:
        if (((duel_turn_player != p && duel_phase == 5) || (duel_turn_player == p && duel_phase < 2)) &&
            (z = duel_find_st(p, 0x1539)) >= 0) {
            TARGET(a->player, z);
        }
        /* fall through */
    case 0x1A9A: case 0x1A1E: case 0x1A09: case 0x19F4: case 0x19F3: case 0x1970: case 0x1962:
    case 0x18F8: case 0x166F: case 0x174D: case 0x18BB: case 0x17DA: case 0x14E1: case 0x15A8:
    case 0x1718: case 0x128E: case 0x164B: case 0x1617: case 0x15FA: case 0x15F9: case 0x15B4:
    case 0x15B1: case 0x179A: case 0x161E: case 5000: case 0x174C: case 0x1298: case 0x128D:
    case 0x101E: case 0x1005: case 0x156C: case 0x152B: case 0x152A: case 0x14CB: case 0x14C3:
    case 0x142E: case 0x134B: case 0x12F8: case 0x1253: case 0x119B: case 0x119A: case 0x147E:
    case 0x137C: case 0x134D: case 0x134C: case 0x12EB: case 0x12E6: case 0x117B: case 0x1086:
    case 0xFFA:
        /* the original leaves the third argument of duel_0881616C unset here; 0 assumed */
        z = duel_0881616C(1 - a->player, mask, 0);
        if (z >= 0) {
            TARGET(1 - a->player, z);
        }
        z = duel_088163E4(a->player, mask);
        if (z >= 0) {
            TARGET(a->player, z);
        }
        break;
    case 0x10D3:
    case 0x13E3:
    case 0x13E9:
    case 0x1842:
        if (duel_ai_target_monster(a, mask) > 0) {
            return 1;
        }
        break;
    case 0x1362:
    case 0x1420:
        if (IN_MASK(duel_atk_tp, duel_atk_tz)) {
            TARGET(duel_atk_tp, duel_atk_tz);
        }
        break;
    case 0x1103:
    case 0x1243:
    case 0x1330:
    case 0x14B2:
    case 0x14BE:
    case 0x14EB:
        /* the opponent's monster in the current battle */
        if (duel_special.player[0] == 1 - a->player) {
            if (IN_MASK(duel_special.player[0], duel_atk_zone)) {
                TARGET(duel_special.player[0], duel_atk_zone);
            }
        } else if (IN_MASK(duel_atk_tp, duel_atk_tz)) {
            TARGET(duel_atk_tp, duel_atk_tz);
        }
        break;
    }
    return duel_08816D74(a, 1 - (a->player ^ a->opp));
}
#undef TARGET
#undef IN_MASK
#endif
