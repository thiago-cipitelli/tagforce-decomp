// @range 0x08809B50-0x0880A17C
// @asm 0x08809DA8-0x08809F2C
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
