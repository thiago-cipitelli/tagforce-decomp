// @range 0x088B4560-0x088B7D94
// @asm 0x088B4D54-0x088B4E64
// @asm 0x088B52F0-0x088B5418
// @asm 0x088B6A04-0x088B6BF8
/* Card effect handlers: candidate lists for duel_088BFC64 (player, zone, index). */
#include "duel.h"

void duel_088BFC64(unsigned int p, int zone, unsigned int i);
int card_is_monster(int id);
int card_get_attribute_icon(int id);
int card_get_type(int id);
int card_get_prop5_hi(int id);
int card_is_1a9f(int id);
int duel_card_e59b0(int id);
int duel_pick_st_zone(unsigned int p);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int card_get_atk_q(int id);
int card_attribute_to_group(int id);
int card_get_level(int id);
int duel_card_dfac8(unsigned int p, int id, int field);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_grave_find_uid(unsigned int p, int uid);
int card_attribute_icon_in_2_3(int id);
int card_same_name(int id, int id2);
int card_is_in_group_1a58(int id);
int card_not_in_group_1a6f(int id);
int card_get_kind(int id);
int duel_card_e5b88(int id, int a);
int duel_zone_faceup(unsigned int p, int z, int i);
int duel_field_level(unsigned int p, int z);
int duel_list_card_level(unsigned int p, int i);
int duel_088B5F74(int k, int n);
int duel_088DFCC8(unsigned int uid);
int card_get_def_q(int id);
int card_is_in_group_1566(int id);
int card_is_in_group_158d(int id);
int card_is_attribute8(int id);

extern unsigned int duel_w_08914920;

typedef struct DuelCardB {             /* DuelCard with bits 15-16 as one field */
    unsigned int lo : 15;
    unsigned int b15_16 : 2;
    unsigned int hi : 15;
} DuelCardB;

void duel_088B4560(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_attribute_icon(id) == 2) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B4628(unsigned int p, int k)
{
    unsigned int i;
    int id = k;

    switch (id) {
    case 0x15B6:
        id = 0x15B7;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (id == duel.players[p & 1].zone_d[i].id) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B46EC(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_type(duel.players[p & 1].zone_d[i].id) == 0x16 && card_get_prop5_hi(duel.players[p & 1].zone_d[i].id) == 6) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B47B0(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if ((!card_is_monster(id) || card_is_1a9f(id)) && !duel_card_e59b0(id) && (duel_pick_st_zone(p) >= 0 || card_get_prop5_hi(id) == 2)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B4878(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[p & 1].grave[i].id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B4938(unsigned int p, int b, int uid)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[p & 1].grave[i].id) && duel_grave_card_usable(p, p, i) && uid != CARD_UID(duel.players[p & 1].grave[i])) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B4A2C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_get_atk_q(id) < 0x5DD && !card_attribute_to_group(id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B4B04(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_level(id) < 4 && duel_card_dfac8(p, id, 0) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B4C1C(unsigned int p)
{
    int i = 0;

    do {
        duel_088BFC64(1 - p, 0xB, duel.unk3822[i]);
    } while (++i < 3);
}

void duel_088B4C7C(unsigned int p)
{
    unsigned int i;
    DuelCard *c;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        c = &duel.players[p & 1].grave[i];
        if (card_is_monster(duel.players[p & 1].grave[i].id) && c->b17 && !c->b20) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

#ifdef NON_MATCHING /* o original endereça a cabeça da lista como símbolo próprio (duel+0x1F6E) e recarrega a base de duel; o mwcc reaproveita a base (corpo igual) */
/* Cards of the graveyard named by the 0x12E5 effects of the card uid. */
void duel_088B4D54(unsigned int p, int b, int uid)
{
    unsigned int k = ((unsigned short *)&duel.list[uid])[1];
    DuelPlayer *pl;
    DuelEffect *e;
    int i;

    if (k == 0) {
        return;
    }
    pl = &duel.players[p & 1];
    do {
        e = &duel.effects[k];
        k = e->next;
        if ((e->kind & 0xF) < 6 && e->src == 0x12E5) {
            i = duel_grave_find_uid(p, e->arg);
            if (i >= 0 && duel_grave_card_usable(p, p, i) && ((DuelCardB *)&pl->grave[i])->b15_16) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    } while (k != 0);
}
#endif

void duel_088B4E64(unsigned int p, int b, int lvl)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_get_type(id) == 10 && lvl + 1 == card_get_level(id) && duel_card_dfac8(p, id, 1) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B4F94(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[1 - p & 1].n_grave; i++) {
        id = duel.players[1 - p & 1].grave[i].id;
        if (card_is_monster(id) && (!card_attribute_icon_in_2_3(id) || duel.players[1 - p & 1].grave[i].b14)) {
            duel_088BFC64(1 - p, 0xE, i);
        }
    }
}

void duel_088B5070(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        duel_088BFC64(p, 0xD, i);
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        duel_088BFC64(p, 0xE, i);
    }
}

void duel_088B5150(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0xB && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B5214(unsigned int p)
{
    unsigned int i;
    DuelCard *c;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        c = &duel.players[p & 1].grave[i];
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0x16 && c->b18 && !c->b20) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

#ifdef NON_MATCHING /* o original mantém os corpos dos case como blocos mortos (li; b; nop) depois da cadeia de beql; o mwcc atual os remove */
/* Card i of the hand if it has the name that card id asks for. */
void duel_088B52F0(unsigned int p, int id, int i)
{
    int c = duel.players[p & 1].hand[i].id;

    switch (id) {
    case 0x1476:
        id = 0x1121;
        break;
    case 0x15D0:
        id = 0x15D1;
        break;
    case 0x15D4:
        id = 0x15D5;
        break;
    case 0x165B:
        id = 0x1645;
        break;
    case 0x16FD:
        break;
    case 0x17DD:
        id = 0xFF8;
        break;
    case 0x1821:
        id = 0x1820;
        break;
    case 0x189A:
        id = 0xFA7;
        break;
    }
    if (card_same_name(c, id)) {
        duel_088BFC64(p, 0xB, i);
    }
}
#endif

void duel_088B5418(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B54C0(unsigned int p, int id)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == card_get_type(id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B5588(unsigned int p, int b, int id)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (id == duel.players[p & 1].hand[i].id) {
            duel_088BFC64(p, 0xB, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (id == duel.players[p & 1].zone_d[i].id) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B569C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_type(duel.players[p & 1].zone_d[i].id) == 0x16) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B5748(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        if (card_is_monster(id) && duel_card_dfac8(p, id, 1)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && duel_card_dfac8(p, id, 1) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        id = duel.players[p & 1].zone_c[i].id;
        if (card_is_in_group_1a58(id) && card_not_in_group_1a6f(id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088B5944(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_monster(duel.players[p & 1].zone_d[i].id) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B5A24(unsigned int p, int x)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && duel_card_e5b88(id, card_get_kind(x))) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B5B00(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (((DuelCardB *)&duel.players[p & 1].grave[i])->b15_16 == 2) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B5BAC(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B5C54(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_f; i++) {
        if (card_is_monster(duel.players[p & 1].zone_f[i].id) && duel_zone_faceup(p, 0xF, i)) {
            duel_088BFC64(p, 0xF, i);
        }
    }
}

/* Monsters of the graveyard that are the source of a pending 0x16 entry of the chain. */
void duel_088B5D14(unsigned int p)
{
    unsigned int i;
    struct DuelActX *x;
    int k;

    for (i = 0; i < duel_w_08914920; i++) {
        if (duel_chain.kinds[i] == 0x16) {
            x = &duel_chain.srcs[i];
            if (x->s_b16 && x->s_b15 && x->s_zone2 == 0xE) {
                k = duel_grave_find_uid(p, x->s_uid);
                if (k >= 0 && card_is_monster(duel.players[p & 1].grave[k].id) && duel_grave_card_usable(p, p, k)) {
                    duel_088BFC64(p, 0xE, k);
                }
            }
        }
    }
}

/* Card i of the hand if it is not already in the current uid list of the chain. */
void duel_088B5E58(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;
    unsigned int h;
    int n;
    int j;
    int uid;

    if (card_is_monster(id) && duel_card_dfac8(p, id, 0)) {
        h = duel_chain.uids_head;
        n = duel_chain.uids[h & 0x7F];
        j = 0;
        if (n > 0) {
            uid = CARD_UID(duel.players[p & 1].hand[i]);
            do {
                j++;
                if (duel_chain.uids[(h + j) & 0x7F] == uid) {
                    return;
                }
            } while (j < n);
        }
        duel_088BFC64(p, 0xB, i);
    }
}

/* Whether some of the levels stored in the chain uid list up to index k sum exactly to n. */
int duel_088B5F74(int k, int n)
{
    int v;

    for (; k >= 0; k--) {
        v = duel_chain.uids[(duel_chain.uids_head + k) & 0x7F];
        if (v < n) {
            if (k != 0 && duel_088B5F74(k - 1, n - v)) {
                return 1;
            }
        } else if (v == n) {
            return 1;
        }
    }
    return !n;
}

/* Monsters of type 3 in the graveyard whose level is a sum of levels of the player's monsters. */
void duel_088B601C(unsigned int p)
{
    int n = 0;
    int lv[13];
    int i;
    int id;
    int l;

    for (i = 1; i <= 12; i++) {
        lv[i] = 0;
    }
    for (i = 0; i <= 4; i++) {
        if (duel.players[p & 1].field[i].card.id != 0) {
            duel_chain.uids[(duel_chain.uids_head + n) & 0x7F] = duel_field_level(p, i);
            n++;
        }
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        l = card_get_level(id);
        if (card_get_type(id) == 3 && duel_grave_card_usable(p, p, i) && (lv[l] != 0 || duel_088B5F74(n - 1, l))) {
            lv[l] = 1;
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B61D8(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 5 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B62AC(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_level(id) < 5 && card_get_type(id) == 0xF) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B6388(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0xF) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B6434(unsigned int p, int x, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_type(id) == card_get_type(x) && duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i]))) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B6510(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        duel_088BFC64(p, 0xE, i);
    }
}

void duel_088B658C(unsigned int p, int b, int lvl)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_get_type(id) == 0x11 && duel_grave_card_usable(p, p, i) && lvl == MIN(card_get_level(id), 6)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B6690(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_get_attribute_icon(id) == 0 && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B676C(unsigned int p)
{
    if (duel.players[p & 1].n_d != 0) {
        duel_088BFC64(p, 0xD, 0);
    }
}

void duel_088B67B4(unsigned int p, int id)
{
    unsigned int i;
    int k;

    switch (id) {
    case 0x1507:
        k = 0x1508;
        break;
    case 0x1508:
        k = 0x1507;
        break;
    }
    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        if (k == duel.players[p & 1].zone_c[i].id) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088B6880(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[1 - p & 1].n_grave; i++) {
        duel_088BFC64(1 - p, 0xE, i);
    }
}

void duel_088B6900(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && duel_card_dfac8(p, id, 1) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

#ifdef NON_MATCHING /* os laços dos case 1 e 2 trocam s2/s3 (contador e ponteiro) em relação ao original com qualquer combinação de variáveis por case */
void duel_088B6A04(unsigned int p, int b, int mode)
{
    switch (mode) {
    case 0: {
        unsigned int i;

        for (i = 0; i < duel.players[p & 1].n_d; i++) {
            if (card_is_monster(duel.players[p & 1].zone_d[i].id) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
                duel_088BFC64(p, 0xD, i);
            }
        }
        break;
    }
    case 1: {
        unsigned int j;

        for (j = 0; j < duel.players[p & 1].n_d; j++) {
            if (card_get_type(duel.players[p & 1].zone_d[j].id) == 0x16) {
                duel_088BFC64(p, 0xD, j);
            }
        }
        break;
    }
    case 2: {
        unsigned int k;

        for (k = 0; k < duel.players[p & 1].n_d; k++) {
            if (card_get_type(duel.players[p & 1].zone_d[k].id) == 0x17) {
                duel_088BFC64(p, 0xD, k);
            }
        }
        break;
    }
    }
}
#endif

void duel_088B6BF8(unsigned int p, int x)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_def_q(id) < 0x7D1 && card_get_type(id) == card_get_type(x) && duel_card_dfac8(p, id, 1)
            && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B6D40(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_type(duel.players[p & 1].zone_d[i].id) == 0x16) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B6DEC(unsigned int p, int b, int mode)
{
    unsigned int i;

    if (mode == 0) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            if (card_get_type(duel.players[p & 1].grave[i].id) == 2 && duel_grave_card_usable(p, p, i)) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    } else {
        for (i = 0; i < duel.players[1 - p & 1].n_grave; i++) {
            if (card_is_monster(duel.players[1 - p & 1].grave[i].id)) {
                duel_088BFC64(1 - p, 0xE, i);
            }
        }
    }
}

void duel_088B6F34(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_get_type(id) == 2 && !card_attribute_icon_in_2_3(id) && card_is_in_group_1a58(id)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B6FD8(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_in_group_1566(duel.players[p & 1].zone_d[i].id)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B7080(unsigned int p, int id)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == card_get_type(id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B7160(unsigned int p, int id)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0xD && id != duel.players[p & 1].grave[i].id && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B7240(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (duel_card_e5b88(id, 2) && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 5 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B7318(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) == 4 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B73D0(unsigned int p, int b, unsigned int x)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_same_name(id, x) && duel_card_dfac8(p, id, 1) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B74F4(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_atk_q(id) < 0x5DD && card_is_in_group_158d(id) && duel_card_dfac8(p, id, 1)
            && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B761C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_type(duel.players[p & 1].zone_d[i].id) == 0x17) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B76C8(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (!duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B7794(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_prop5_hi(duel.players[p & 1].zone_d[i].id) == 2) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B7840(unsigned int p, int b, int lvl)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        id = duel.players[p & 1].zone_c[i].id;
        if (lvl == card_get_level(id) && card_is_in_group_1a58(id) && card_not_in_group_1a6f(id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088B7920(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_158d(id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088B79FC(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_level(id) == 4 && duel_card_dfac8(p, id, 1) && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B7B14(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_def_q(id) < 0x4B1 && card_get_type(id) == 2) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088B7BF0(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_attribute8(id) && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 5 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088B7CC4(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[1 - p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[1 - p & 1].grave[i].id) && duel_grave_card_usable(p, 1 - p, i)) {
            duel_088BFC64(1 - p, 0xE, i);
        }
    }
}
