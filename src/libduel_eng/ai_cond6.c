// @range 0x08814EC8-0x08815E90
// @asm 0x08815548-0x088157AC
// @asm 0x088157AC-0x08815950
// @asm 0x088159C8-0x08815AF8
// @asm 0x08815BD0-0x08815E20
/* The computer player's choice of a card to activate: the tables of cards with their conditions. */
#include "duel.h"

typedef struct DuelAiCard {            /* 8 */
    unsigned int id;
    int (*cond)(DuelAct *a, DuelAct *b);
} DuelAiCard;

extern DuelAiCard duel_ai_cards[0x101];        /* 0x08905B30 */
extern DuelAiCard duel_ai_chain_cards[0x56];   /* 0x08906338 cards answering a chain */
extern unsigned int duel_w_08913CF4;
extern unsigned int duel_ai_player;            /* 0x08919648 */

int card_get_type(int id);
int card_is_monster(int id);
int duel_0886C2E0(unsigned int p, unsigned int id);
int duel_count_active(int id);
int field_get_card(unsigned int p, int z);
int duel_0886981C(DuelAct *a, DuelAct *b);
int duel_08869ACC(DuelAct *a, DuelAct *b);
int duel_ai_0882054C(DuelAct *a);
void duel_08835F34(unsigned int p, int z, int i, int d);
int duel_hand_find_name(unsigned int p, int id);
int duel_0880AFC4(unsigned int p);
int duel_08821094(unsigned int p);
int duel_088695B8(unsigned int p, unsigned int q, int z, int d);
int duel_0880A8AC(unsigned int p, unsigned int q);
int duel_rand(int n);
void duel_0880546C(unsigned int p, int z, DuelStats *st);
unsigned int duel_0884E4A0(DuelAct *a);
int card_get_atk(int id);
int card_get_def(int id);
int duel_08815548(unsigned int p, unsigned int mask, int atk, int def);

int duel_08814EC8(unsigned int p, DuelAiCard *e, DuelAct *b)
{
    int i;
    int z;
    unsigned int id = e->id;
    int r;

    if (card_get_type(id) == 0x17) {
        for (i = 0; i < duel_chain.unk480; i++) {
            if (duel_chain.acts[i].player == p && duel_chain.acts[i].id == id) {
                return 0;
            }
        }
    }
    if (card_is_monster(id) == 0 && duel_0886C2E0(p, id) != 0) {
        return 0;
    }
    if (card_get_type(id) == 0x17 && duel_count_active(0x184F) != 0) {
        return 0;
    }
    if (card_get_type(id) == 0x16 && duel_count_active(0x171D) != 0) {
        return 0;
    }
    for (z = 0; z <= 10; z++) {
        if (id == field_get_card(p, z)) {
            DuelAct t = {0};

            t.player = p;
            t.zone = z;
            t.uid = CARD_UID(duel.players[p & 1].field[z].card);
            t.id = id;
            if (duel.players[p & 1].field[z].faceup != 0) {
                r = duel_0886981C(&t, b);
            } else {
                r = duel_08869ACC(&t, b);
            }
            if (r != 0 && duel_ai_0882054C(&t) == 0 && e->cond(&t, b) != 0) {
                duel_ai_player = p;
                duel_08835F34(p, z, 0, 3);
                return 1;
            }
        }
    }
    i = duel_hand_find_name(p, e->id);
    if (i >= 0 && duel_0880AFC4(p) != 0) {
        DuelAct t = {0};

        t.player = p;
        t.zone = 0xB;
        t.uid = CARD_UID(duel.players[p & 1].hand[i]);
        t.id = id;
        if (duel_08869ACC(&t, b) != 0 && duel_ai_0882054C(&t) == 0 && e->cond(&t, b) != 0) {
            duel_ai_player = p;
            duel_08835F34(p, 0xB, i, 3);
            return 1;
        }
    }
    return 0;
}

int duel_08815288(unsigned int p)
{
    unsigned int i;
    int r;

    duel_w_08913CF4 = p;
    for (i = 0; i < 0x101; i++) {
        r = !duel_08814EC8(p, &duel_ai_cards[i], 0);
        if (!r) {
            return 1;
        }
    }
    return duel_08821094(p) != 0;
}

int duel_0881530C(unsigned int p, DuelAct *b)
{
    unsigned int i;
    int r;

    duel_w_08913CF4 = p;
    for (i = 0; i < 0x56; i++) {
        r = !duel_08814EC8(p, &duel_ai_chain_cards[i], b);
        if (!r) {
            return 1;
        }
    }
    if (!(p != (b->player ^ b->opp) || card_is_monster(b->id) != 0 || b->id == 0x13F9)) {
        return 0;
    }
    for (i = 0; i < 0x101; i++) {
        r = !duel_08814EC8(p, &duel_ai_cards[i], b);
        if (!r) {
            return 1;
        }
    }
    return 0;
}

int duel_0881541C(unsigned int p, unsigned short id)
{
    int z;

    for (z = 5; z < 10; z++) {
        if (duel.players[p & 1].field[z].card.id != 0 && id == field_get_card(p, z) && duel_088695B8(p, p, z, 0) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_088154D8(unsigned int p)
{
    int n = duel_0880A8AC(p, p);
    int m = duel_0880A8AC(p, 1 - p);

    return duel.players[p & 1].lp <= m - n;
}

#ifdef NON_MATCHING /* registradores */
int duel_08815548(unsigned int p, unsigned int mask, int atk, int def)
{
    int z[11];
    DuelStats st;
    int n = 0;
    int i;
    int k;
    int t;
    int v;
    int c;
    int zi;
    int m = -1;
    int best = -1;
    unsigned int b;

    for (i = 0, b = p << 4; i <= 4; i++, b++) {
        if (mask & (1 << b)) {
            z[n++] = i;
        }
    }
    for (i = n - 1; i > 0; i--) {
        k = duel_rand(i + 1);
        t = z[i];
        z[i] = z[k];
        z[k] = t;
    }
    for (i = 0; i < n; i++) {
        zi = z[i];
        v = 0;
        duel_0880546C(p, zi, &st);
        if (atk != 0 && def != 0) {
            v = MAX(st.atk, st.def);
        } else if (atk != 0) {
            v = st.atk;
        } else if (def != 0) {
            v = st.def;
        }
        if (m < 0) {
            best = zi;
            m = v;
        } else {
            c = 0;
            if (atk > 0 && m < v) {
                c = 1;
            }
            if (atk < 0 && v < m) {
                c = 1;
            }
            if (def > 0 && m < v) {
                c = 1;
            }
            if (def < 0 && v < m) {
                c = 1;
            }
            if (c) {
                best = zi;
                if (atk > 0 && m < v) {
                    m = v;
                }
                if (atk < 0 && v < m) {
                    m = v;
                }
                if (def > 0 && m < v) {
                    m = v;
                }
                if (def < 0 && v < m) {
                    m = v;
                }
            }
        }
    }
    return best;
}
#endif

#ifdef NON_MATCHING /* registradores */
int duel_088157AC(unsigned int p, unsigned int mask, unsigned int pos)
{
    DuelStats st;
    int z[11];
    int n = 0;
    int i;
    int k;
    int t;
    int v;
    int m = -1;
    int best = -1;

    for (i = 0; i < 5; i++) {
        if (mask & (1 << (p * 16 + i))) {
            z[n++] = i;
        }
    }
    for (i = n - 1; i > 0; i--) {
        k = duel_rand(i + 1);
        t = z[i];
        z[i] = z[k];
        z[k] = t;
    }
    for (i = 0; i < n; i++) {
        if ((int)pos < 0 || pos == duel.players[p & 1].field[z[i]].pos) {
            duel_0880546C(p, z[i], &st);
            v = duel.players[p & 1].field[z[i]].pos == 0 ? st.atk : st.def;
            if (m < 0 || m < v) {
                m = v;
                best = z[i];
            }
        }
    }
    return best;
}
#endif

int duel_08815950(DuelAct *a, unsigned int p, int atk, int def, int z)
{
    unsigned int m = duel_0884E4A0(a);

    duel_w_08913CF4 = p;
    return duel_08815548(p, m & ~(1 << (z + p * 16)), atk, def);
}

#ifdef NON_MATCHING /* registradores */
int duel_088159C8(unsigned int p, unsigned int mask, int (*fn)(unsigned int p, int z))
{
    int z[11];
    int n = 0;
    int i;
    int k;
    int t;
    int v;
    int c;
    int best;
    int m;
    unsigned int b;

    for (i = 0, b = p << 4; i <= 10; i++, b++) {
        if (mask & (1 << b)) {
            z[n++] = i;
        }
    }
    for (i = n - 1; i > 0; i--) {
        k = duel_rand(i + 1);
        t = z[i];
        z[i] = z[k];
        z[k] = t;
    }
    best = -1;
    m = 0;
    for (i = 0; i < n; i++) {
        c = z[i];
        v = fn(p, c);
        if (m < v) {
            m = v;
            best = c;
        }
    }
    return best;
}
#endif

int duel_08815AF8(unsigned int p, int (*f)(unsigned int p, int zone, int i), int (*g)(unsigned int p, int i))
{
    unsigned int i;
    int v;
    int best = -1;
    int m = 0;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (f(p, 0xB, i) != 0) {
            v = g(p, i);
            if (m < v) {
                m = v;
                best = i;
            }
        }
    }
    return best;
}

#ifdef NON_MATCHING /* registradores */
int duel_08815BD0(unsigned int p, int (*f)(unsigned int p, int zone, int i), int atk, int def)
{
    unsigned int i;
    unsigned int id;
    int v;
    int m = -1;
    int best = -1;
    int c;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (f(p, 0xB, i) != 0) {
            id = duel.players[p & 1].hand[i].id;
            v = 0;
            if (atk != 0 && def != 0) {
                v = MAX(card_get_atk(id), card_get_def(id));
            } else if (atk != 0) {
                v = card_get_atk(id);
            } else if (def != 0) {
                v = card_get_def(id);
            }
            if (m < 0) {
                m = v;
                best = i;
            } else {
                c = 0;
                if (atk > 0 && m < v) {
                    c = 1;
                }
                if (atk < 0 && v < m) {
                    c = 1;
                }
                if (def > 0 && m < v) {
                    c = 1;
                }
                if (def < 0 && v < m) {
                    c = 1;
                }
                if (c) {
                    if (atk > 0 && m < v) {
                        m = v;
                    }
                    if (atk < 0 && v < m) {
                        m = v;
                    }
                    if (def > 0 && m < v) {
                        m = v;
                    }
                    if (def < 0 && v < m) {
                        m = v;
                    }
                    best = i;
                }
            }
        }
    }
    return best;
}
#endif

int duel_08815E20(unsigned int p, unsigned int mask)
{
    int z[11];
    int i;
    int n = 0;
    unsigned int b = p << 4;

    for (i = 0; i <= 10; i++) {
        if (mask & (1 << b)) {
            z[n++] = i;
        }
        b++;
    }
    if (n > 0) {
        return z[duel_rand(n)];
    }
    return -1;
}
