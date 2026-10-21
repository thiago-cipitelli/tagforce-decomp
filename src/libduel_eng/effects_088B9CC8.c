// @range 0x088B9CC8-0x088BB8C0
// @asm 0x088BAB94-0x088BAE0C
// @asm 0x088BAF7C-0x088BB0A0
/* Card effect handlers: candidate lists for duel_088BFC64 (after effects_088B7D94.c). */
#include "duel.h"

int duel_list_card_level(unsigned int p, int i);
int duel_card_dfac8(unsigned int p, int id, int field);
void duel_088BFC64(unsigned int p, int zone, unsigned int i);
int card_is_monster(int id);
int card_get_level(int id);
int card_get_type(int id);
int card_get_prop5_hi(int id);
int card_get_attribute_icon(int id);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int card_get_special_group(unsigned short id);
int card_get_atk_q(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int card_get_kind(int id);
int card_is_in_group_198c(int id);
void duel_088B5588(unsigned int p, int b, int c);
void duel_088B2F34(unsigned int p, int b, int c);
int card_get_atk_q(int id);
int duel_card_e5b88(int id, int a);
int card_same_name(int id, int id2);
int duel_tributable(unsigned int p, unsigned int q, int z);
int card_is_in_group_1a58(int id);
int card_not_in_group_1a6f(int id);
int duel_088A6300(int id);
int duel_088A6360(int id, int i);
int duel_088A62AC(int id, unsigned int p, unsigned int z);
int card_attribute_icon_is2(int id);
int card_attribute_icon_is3(int id);
int card_is_in_group_18a4(int id);
int duel_0884E948(DuelAct *a, unsigned int p);
int card_is_in_group_1a2f(int id);
int duel_088DFCC8(unsigned int uid);
extern DuelAct *duel_w_089147DC;

void duel_088B9CC8(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0x16 && card_get_prop5_hi(duel.players[p & 1].grave[i].id) == 4) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B9D8C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_get_attribute_icon(id) == 0 && card_get_level(id) < 4 && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B9E7C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_attribute_icon(id) == 0 && card_get_level(id) < 4) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B9F54(unsigned int p, int b, int uid)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_special_group(duel.players[p & 1].grave[i].id) == 1 && uid != CARD_UID(duel.players[p & 1].grave[i])) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BA034(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_special_group(duel.players[p & 1].grave[i].id) == 1) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BA0E0(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_level(id) < 3 && card_get_attribute_icon(id) == 0 && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BA1D0(unsigned int p, int b, int c)
{
    if (c != 0) {
        duel_088B5588(p, b, c);
    } else {
        duel_088B2F34(p, b, c);
    }
}

void duel_088BA200(unsigned int p, int b)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_atk_q(id) < 0x5DD && card_get_type(id) == card_get_type(b) && duel_card_dfac8(p, id, 1) &&
            !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BA348(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_attribute_icon(duel.players[p & 1].grave[i].id) == 0) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BA3F0(unsigned int p, int b, unsigned int c)
{
    unsigned int i;
    int id;
    int id2;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        if (((unsigned short)c >> 8 & 0xFF) == card_get_type(id) && (c & 0xFF) == card_get_kind(id)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id2 = duel.players[p & 1].zone_d[i].id;
        if (((unsigned short)c >> 8 & 0xFF) == card_get_type(id2) && (c & 0xFF) == card_get_kind(id2)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BA564(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_in_group_198c(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BA60C(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_attribute_icon(id) == 0 && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) == 4 &&
        duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BA6F0(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        if (card_is_monster(id) && duel_card_e5b88(id, 3) && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
}

void duel_088BA7D0(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_atk_q(id) < 0x7D1 && card_get_type(id) == 0x12 && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BA8C4(unsigned int p, int id)
{
    unsigned int i;
    int k;

    switch (id) {
    case 0x183D:
        k = 0x1782;
        break;
    case 0x1A42:
        k = 0xFE4;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_same_name(duel.players[p & 1].grave[i].id, k) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BA9B0(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BAA58(unsigned int p, int b, int i)
{
    if (duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) >= 7) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BAAE8(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0xB) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

#ifdef NON_MATCHING /* contador e ponteiro do laço que preenche mats trocam $s1/$s2 */
void duel_088BAB94(unsigned int p, int b)
{
    unsigned int i;
    int id;
    int n;
    int n_ok;
    int j;

    int q;
    int z;
    unsigned int *l;
    int k;
    int n_locs;
    unsigned int locs[10];
    int mats[3];

    n_locs = 0;
    l = locs;
    for (q = 0; q < 2; q++) {
        for (z = 0; z <= 4; z++) {
            if (duel_tributable(p, q, z) && (q == p || duel.players[q & 1].field[z].faceup)) {
                *l++ = (unsigned short)((unsigned char)q | (unsigned char)z << 8);
                n_locs++;
            }
        }
    }
    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        id = duel.players[p & 1].zone_c[i].id;
        if (card_is_in_group_1a58(id) && card_not_in_group_1a6f(id) && duel_card_e5b88(id, card_get_kind(b))) {
            n = duel_088A6300(id);
            n_ok = 0;
            for (j = 0; j < n; j++) {
                mats[j] = duel_088A6360(id, j);
            }
            for (k = 0; k < n_locs; k++) {
                for (j = 0; j < n; j++) {
                    if (mats[j] && duel_088A62AC(mats[j], locs[k] & 0xFF, (unsigned short)locs[k] >> 8 & 0xFF)) {
                        mats[j] = 0;
                        n_ok++;
                        break;
                    }
                }
            }
            if (n_ok + 1 >= n) {
                duel_088BFC64(p, 0xC, i);
            }
        }
    }
}
#endif

void duel_088BAE0C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_type(id) == 0xB && card_get_level(id) < 4 && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BAF00(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        duel_088BFC64(p, 0xE, i);
    }
}

#ifdef NON_MATCHING /* c e i trocam $s1/$s4 */
void duel_088BAF7C(unsigned int p, int b, int c)
{
    int n;
    int j;
    unsigned int i;

    n = duel_088A6300(c);
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        int id = duel.players[p & 1].grave[i].id;

        for (j = 0; j < n; j++) {
            if (card_same_name(duel_088A6360(c, j), id) && duel_grave_card_usable(p, p, i)) {
                duel_088BFC64(p, 0xE, i);
                break;
            }
        }
    }
}
#endif

void duel_088BB0A0(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_attribute_icon_is2(duel.players[p & 1].grave[i].id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BB160(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_attribute_icon_is3(duel.players[p & 1].grave[i].id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BB220(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_type(id) == 0xB &&
        duel_w_089147DC->w[2] == duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BB310(unsigned int p, int b, int i)
{
    int lv = duel_w_089147DC->w[2];
    int id;
    unsigned int j;
    unsigned int k;
    int id2;

    if (lv == 0) {
        id = duel.players[p & 1].hand[i].id;
        if (card_is_monster(id) && card_get_type(id) == 2 && card_get_level(id) > 1 &&
            duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i]))) {
            id = card_get_level(id);
            for (j = 0; j < duel.players[p & 1].n_grave; j++) {
                lv = duel.players[p & 1].grave[j].id;
                if (card_get_type(lv) == 2 && card_get_level(lv) < id && duel_grave_card_usable(p, p, j)) {
                    duel_088BFC64(p, 0xB, i);
                    return;
                }
            }
        }
    } else {
        for (k = 0; k < duel.players[p & 1].n_grave; k++) {
            id2 = duel.players[p & 1].grave[k].id;
            if (card_is_monster(id2) && card_get_type(id2) == 2 && card_get_level(id2) < lv && duel_grave_card_usable(p, p, k)) {
                duel_088BFC64(p, 0xE, k);
            }
        }
    }
}

void duel_088BB56C(unsigned int p, unsigned int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_in_group_18a4(id) && id != b && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BB618(unsigned int p, int b, int uid)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        switch (duel.players[p & 1].grave[i].id) {
        case 0xFBE:
        case 0x18C5:
            if (uid != CARD_UID(duel.players[p & 1].grave[i])) {
                duel_088BFC64(p, 0xE, i);
            }
            break;
        }
    }
}

void duel_088BB708(unsigned int p, int b, int i)
{
    int uid = CARD_UID(duel.players[p & 1].hand[i]);

    if (card_is_monster(duel.players[p & 1].hand[i].id) && duel_088DFCC8(uid)) {
        duel_w_089147DC->w[2] = duel_list_card_level(p, uid);
        if (duel_0884E948(duel_w_089147DC, p)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
}

void duel_088BB7E4(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_1a2f(id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}
