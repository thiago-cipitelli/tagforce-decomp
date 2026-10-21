// @range 0x088B7D94-0x088B9CC8
// @asm 0x088B94F4-0x088B967C
/* Card effect handlers: candidate lists for duel_088BFC64. */
#include "duel.h"

int duel_list_card_level(unsigned int p, int i);
int duel_card_dfac8(unsigned int p, int id, int field);
void duel_088BFC64(unsigned int p, int zone, unsigned int i);
int card_same_name(int id, int id2);
int duel_count_active(int id);
int duel_find_effect_card(unsigned int p, int id, int skip);
int card_is_monster(int id);
int card_get_level(int id);
int card_get_type(int id);
int card_is_in_group_1a58(int id);
int card_not_in_group_1a6f(int id);
int card_get_prop5_hi(int id);
int card_is_in_group_16a3(int id);
int card_is_in_group_1911(int id);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int duel_card_e5b88(int id, int a);
int card_is_attribute0(int id);
int duel_zone_f_card_usable(unsigned int p, unsigned int q, int i);
int duel_zone_faceup(unsigned int p, int z, int i);
int card_get_attribute_icon(int id);
int card_get_atk_q(int id);
int card_get_kind(int id);
int duel_088DFCC8(unsigned int uid);
int card_is_in_group_170b(int id);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
int card_attribute_icon_in_2_3(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);

void duel_088B7D94(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) == 7 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B7E4C(unsigned int p, int id)
{
    int k;
    unsigned int i;

    switch (id) {
    case 0x1610:
        k = 0x1377;
        break;
    case 0x1611:
        k = 0xFC9;
        break;
    case 0x167D:
        k = 0x167C;
        break;
    case 0x1713:
        k = 0x16F8;
        break;
    case 0x195C:
        k = 0x1951;
        break;
    case 0x19B1:
        k = 0x19A9;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(duel.players[p & 1].zone_d[i].id, k)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
    if (duel_count_active(0x159D) == 0 || duel_find_effect_card(p, 0x158B, -1)) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            if (card_same_name(duel.players[p & 1].grave[i].id, k)) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    }
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (card_same_name(duel.players[p & 1].hand[i].id, k)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
}

void duel_088B8048(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_level(id) < 3 && card_get_type(id) == 0x12 && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B813C(int a, int b, unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0x16) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B81E8(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        id = duel.players[p & 1].zone_c[i].id;
        if (card_get_level(id) < 7 && card_is_in_group_1a58(id) && card_not_in_group_1a6f(id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088B82C0(unsigned int p, int id)
{
    int k;
    unsigned int i;

    switch (id) {
    case 0x1629:
        k = 0x1377;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(duel.players[p & 1].zone_d[i].id, k)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
    if (duel_count_active(0x159D) == 0 || duel_find_effect_card(p, 0x158B, -1)) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            if (card_same_name(duel.players[p & 1].grave[i].id, k)) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    }
}

void duel_088B841C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d && (int)i < 3; i++) {
        duel_088BFC64(p, 0xD, i);
    }
}

void duel_088B84A0(unsigned int p)
{
    if (duel.players[1 - p & 1].n_d != 0) {
        duel_088BFC64(1 - p, 0xD, 0);
    }
}

void duel_088B84F0(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_prop5_hi(duel.players[p & 1].grave[i].id) == 3) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B859C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_prop5_hi(duel.players[p & 1].zone_d[i].id) == 3) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B8648(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_type(id) == 0x12 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B86F4(unsigned int p, int b, int c)
{
    unsigned int i;

    if (c == 1) {
        for (i = 0; i < duel.players[p & 1].n_d; i++) {
            if (card_is_in_group_16a3(duel.players[p & 1].zone_d[i].id)) {
                duel_088BFC64(p, 0xD, i);
            }
        }
    } else {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            if (card_is_in_group_16a3(duel.players[p & 1].grave[i].id)) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    }
}

void duel_088B881C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_prop5_hi(duel.players[p & 1].zone_d[i].id) == 3) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B88C8(unsigned int p, int b, unsigned int n)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_is_in_group_1911(id) && card_get_level(id) < n) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B89AC(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_1911(id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B8A88(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && duel_card_e5b88(id, 1)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B8B50(unsigned int p, int b, int i)
{
    if (card_is_attribute0(duel.players[p & 1].hand[i].id)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B8BC4(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[1 - p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[1 - p & 1].grave[i].id)) {
            duel_088BFC64(1 - p, 0xE, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B8CEC(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0x16) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B8D98(unsigned int p, int b, int c)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && duel_card_e5b88(id, c ? 2 : 1)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B8E74(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (!card_is_monster(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B8F1C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_f; i++) {
        duel_088BFC64(p, 0xF, i);
    }
}

void duel_088B8F98(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_f; i++) {
        if (card_is_monster(duel.players[p & 1].zone_f[i].id) && duel_zone_f_card_usable(p, p, i) && duel_zone_faceup(p, 0xF, i)) {
            duel_088BFC64(p, 0xF, i);
        }
    }
}

void duel_088B9070(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_get_attribute_icon(id) == 2 || card_get_prop5_hi(id) == 6) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B913C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_get_atk_q(id) < 0x5DD && duel_card_e5b88(id, 3)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B9218(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_type(id) == 9 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B92C4(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_type(duel.players[p & 1].zone_d[i].id) == 10) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B9370(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && duel_card_e5b88(id, card_get_kind(b)) && duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i]))) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B944C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_in_group_170b(duel.players[p & 1].zone_d[i].id)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

#ifdef NON_MATCHING /* registradores de c e i trocados ($s3/$s4) */
void duel_088B94F4(unsigned int p, int b, int c)
{
    DuelAct act;
    char *q;
    int n;
    unsigned int i;
    int id;

    q = (char *)&act;
    n = 0x18;
    if (q != 0) {
        do {
            *q++ = 0;
        } while (--n);
    }
    act.player = p;
    act.zone = 0xB;
    act.b3_4 = 0;
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_get_prop5_hi(id) == 3) {
            act.uid = CARD_UID(duel.players[p & 1].grave[i]);
            act.id = id;
            if (duel_act_check_08853CE8(&act, p, c) == 1) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    }
}
#endif

void duel_088B967C(unsigned int p, int b, int i)
{
    if (card_same_name(duel.players[p & 1].hand[i].id, 0x12EC)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B96F4(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && !card_attribute_icon_in_2_3(id)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B97B8(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && duel_card_e5b88(id, 1) && duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i]))) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B9880(unsigned int p, int b, unsigned int n)
{
    unsigned int i;
    int id;
    int id2;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        switch (card_get_type(id)) {
        case 0xB:
        case 0x10:
        case 10:
            if (duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) <= n && duel_card_dfac8(p, id, 0) &&
                !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].hand[i]))) {
                duel_088BFC64(p, 0xB, i);
            }
            break;
        }
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id2 = duel.players[p & 1].zone_d[i].id;
        switch (card_get_type(id2)) {
        case 0xB:
        case 0x10:
        case 10:
            if (card_get_level(id2) <= n && duel_card_dfac8(p, id2, 0) &&
                !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
                duel_088BFC64(p, 0xD, i);
            }
            break;
        }
    }
}

void duel_088B9AE0(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && duel_card_e5b88(id, 1)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B9BA8(unsigned int p, int b)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_get_attribute_icon(id) == 0 && card_get_type(id) == card_get_type(b) &&
            card_get_level(id) < 3 && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}
