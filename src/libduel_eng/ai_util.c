// @range 0x08809B50-0x0880A8AC
// @asm 0x08809DA8-0x08809F2C
// @asm 0x0880A54C-0x0880A8AC
/* Helpers of the computer player: picking cards for a dialog, comparing the strongest monsters of
   both players. */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_w_08913CF4;   /* player the computer is choosing for [?] */
extern int duel_w_08919624;

int duel_088BFF78(unsigned int p, int id, int a);
int duel_088087C0(unsigned int p, int id, int a);
void duel_0884C7C0(int i);
void duel_0884CB8C(int i, int v);
int duel_count_field_all(unsigned int p);
int duel_rand(int n);
int duel_hand_revealed(unsigned int p);
void duel_0880546C(unsigned int p, int z, DuelStats *st);
void duel_08809FC4(unsigned int p, int *atk, int *def, int f);
void duel_08805308(unsigned int p, int z, DuelStats *st);
void duel_08805378(unsigned int p, int z, DuelStats *st);
int duel_08854D4C(DuelAct *a, unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_slot_ef098();
int field_get_card(unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_slot_affected(unsigned int p, int z, int id);

int duel_08809B50(unsigned int p, int id, int a)
{
    int i;

    if (duel_088BFF78(p, id, a) == 0) {
        return -1;
    }
    duel_w_08913CF4 = p;
    i = duel_088087C0(p, id, a);
    if (i < 0) {
        return -1;
    }
    duel_0884C7C0(i);
    duel_0884CB8C(i, 1);
    duel_w_08919624 = 1;
    return i;
}

int duel_08809BE8(unsigned int p, int id, int a, int n)
{
    int k = duel_088BFF78(p, id, a);
    int i;

    n = MIN(n, k);
    duel_w_08913CF4 = p;
    for (i = 0; i < n; i++) {
        int j = duel_088087C0(p, id, a);

        if (j < 0) {
            break;
        }
        duel_0884C7C0(j);
        duel_0884CB8C(j, i + 1);
    }
    duel_w_08919624 = i;
    return i;
}

int duel_08809CA8(unsigned int p, int id, int a, int n)
{
    int k = duel_088BFF78(p, id, a);
    int i;

    n = MIN(n, k);
    duel_w_08913CF4 = p;
    switch (id) {
    case 0x18E0:
        k = duel_count_field_all(1 - p);
        n = MIN(n, k);
        break;
    }
    if (n == 0) {
        n = 1;
    }
    for (i = 0; i < n; i++) {
        int j = duel_088087C0(p, id, a);

        if (j < 0) {
            break;
        }
        duel_0884C7C0(j);
        duel_0884CB8C(j, i + 1);
    }
    duel_w_08919624 = i;
    return i;
}

#ifdef NON_MATCHING /* the original keeps the card id in the register of the id parameter */
int duel_08809DA8(unsigned int p, int id)
{
    switch (id) {
    case 0x169D:
        id = duel.players[p & 1].zone_d[duel_rand(duel.players[p & 1].n_d)].id;
        break;
    case 0x1568:
        id = 0x161C;
        break;
    default:
        p = 1 - p;
        id = 0x161C;
        if (duel_hand_revealed(p) == 0 || duel.players[p & 1].n_hand == 0) {
            if (duel.players[p & 1].n_grave != 0) {
                int i = duel_rand(duel.players[p & 1].n_grave);

                if (card_attribute_icon_is3(duel.players[p & 1].grave[i].id) == 0) {
                    id = duel.players[p & 1].grave[i].id;
                }
            }
        } else {
            id = duel.players[p & 1].hand[duel_rand(duel.players[p & 1].n_hand)].id;
        }
        break;
    }
    duel_w_08919624 = id;
    return id;
}
#endif

int duel_08809F2C(unsigned int p)
{
    int a0;
    int a1;
    int b0;
    int b1;

    duel_08809FC4(1 - p, &a0, &a1, 0);
    duel_08809FC4(p, &b0, &b1, 0);
    if (MAX(a0, a1) < b0) {
        return 2;
    }
    if (MAX(b0, b1) < a0) {
        return -2;
    }
    return 0;
}

void duel_08809FC4(unsigned int p, int *atk, int *def, int f)
{
    DuelStats st;
    int z;

    *def = 0;
    *atk = 0;
    for (z = 0; z < 5; z++) {
        if (duel.players[p & 1].field[z].card.id != 0 && (duel.players[p & 1].field[z].faceup != 0 || f == 0)) {
            duel_0880546C(p, z, &st);
            *atk = MAX(*atk, st.atk);
            *def = MAX(*def, st.def);
        }
    }
}

int duel_0880A0B4(unsigned int p)
{
    DuelStats st;
    int z;
    int m = 0;

    for (z = 0; z < 5; z++) {
        if (duel.players[p & 1].field[z].card.id != 0) {
            duel_0880546C(p, z, &st);
            if (duel.players[p & 1].field[z].pos != 0) {
                m = MAX(m, st.def);
            } else {
                m = MAX(m, st.atk);
            }
        }
    }
    return m;
}

int duel_0880A17C(unsigned int p, unsigned int q, int skip, int fa, int fd)
{
    DuelStats st;
    int z;
    int m = -1;

    for (z = 0; z < 5; z++) {
        if (z != skip && duel.players[q & 1].field[z].card.id != 0) {
            int v = 0;

            if (p == q) {
                duel_08805308(q, z, &st);
            } else {
                duel_08805378(q, z, &st);
            }
            if (fa != 0) {
                v = st.atk < 0 ? 0 : st.atk;
            }
            if (fd != 0) {
                v = MAX(v, st.def);
            }
            if (m < v) {
                m = v;
            }
        }
    }
    return m;
}

int duel_0880A2B8(unsigned int p, unsigned int q, int skip, int fa, int fd)
{
    int best = -1;
    DuelStats st;
    int z;
    int m = -1;

    for (z = 0; z < 5; z++) {
        if (z != skip && duel.players[q & 1].field[z].card.id != 0) {
            int v = 0;

            if (p == q) {
                duel_08805308(q, z, &st);
            } else {
                duel_08805378(q, z, &st);
            }
            if (fa != 0) {
                v = st.atk < 0 ? 0 : st.atk;
            }
            if (fd != 0) {
                v = MAX(v, st.def);
            }
            if (m < v) {
                m = v;
                best = z;
            }
        }
    }
    return best;
}

int duel_0880A400(unsigned int p, unsigned int q, int skip, int fa, int fd)
{
    int best = -1;
    DuelStats st;
    int z;
    int m = 99999;

    for (z = 0; z < 5; z++) {
        if (z != skip && duel.players[q & 1].field[z].card.id != 0) {
            int v = 0;

            if (p == q) {
                duel_08805308(q, z, &st);
            } else {
                duel_08805378(q, z, &st);
            }
            if (fa != 0) {
                v = st.atk < 0 ? 0 : st.atk;
            }
            if (fd != 0) {
                v = MAX(v, st.def);
            }
            if (v < m) {
                m = v;
                best = z;
            }
        }
    }
    return best;
}

#ifdef NON_MATCHING /* mwcc merges the identical bodies of the 0x186C and 0x186B/0x1193 cases */
int duel_0880A54C(DuelAct *a, unsigned int p, int skip, int any)
{
    int best = -1;
    int m = -1;
    int z;

    for (z = 0; z < 5; z++) {
        if (z != skip && duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0 &&
            duel_08854D4C(a, p, z) != 0) {
            int v = duel_field_atk(p, z);

            if (p == duel_field_controller(p, z, 1) && duel_attack_check(p, z, 1) != 0) {
                switch (a->id) {
                case 0x165D:
                    v -= 1000;
                    if (v < 1000) {
                        v = -1;
                    }
                    break;
                case 0x1318:
                case 0x1661:
                    v -= 500;
                    if (v < 0x5DC) {
                        v = -1;
                    }
                    break;
                case 0x1841:
                    v = 0x10000 - v;
                    break;
                case 0x1322:
                    if (duel_slot_ef098(p, z) == 0) {
                        v = -1;
                    }
                    break;
                case 0x143A:
                    if (field_get_card(p, z) != 0x1439) {
                        v = -1;
                    }
                    v += 1000;
                    break;
                }
                if (duel_act_immune(a, p, z) != 0) {
                    v = -1;
                }
                switch (field_get_card(p, z)) {
                case 0xFF8:
                case 0x112D:
                    if (a->id == 0x1238) {
                        return z;
                    }
                    break;
                case 0x1193:
                case 0x186B:
                    v += 500;
                    break;
                case 0x186C:
                    v += 500;
                    break;
                case 0x13C3:
                    switch (a->id) {
                    case 0x150C:
                        break;
                    case 0x1514:
                        v += 500;
                        break;
                    default:
                        v = -1;
                        break;
                    }
                    break;
                case 0x1596:
                    v = -1;
                    break;
                }
                if (card_get_type(a->id) != 0x17 && duel_slot_affected(p, z, 0x1660) != 0) {
                    v = -1;
                }
                if (m < v) {
                    m = v;
                    best = z;
                }
            }
        }
    }
    if (any != 0 && best == -1) {
        for (z = 0; z < 5; z++) {
            if (z != skip && duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0 &&
                duel_08854D4C(a, p, z) != 0) {
                return z;
            }
        }
    }
    return best;
}
#endif
