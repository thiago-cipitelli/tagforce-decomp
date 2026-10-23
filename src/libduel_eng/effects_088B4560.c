// @range 0x088B4560-0x088B5944
// @asm 0x088B4D54-0x088B4E64
// @asm 0x088B52F0-0x088B5418
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
