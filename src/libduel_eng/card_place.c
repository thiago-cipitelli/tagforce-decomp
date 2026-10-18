// @range 0x088E5800-0x088E5F84
// @asm 0x088E5920-0x088E59B0
// @asm 0x088E5BE8-0x088E5F84
/* Small card rules (targets of an activation, kinds), card references (copy, swap, compare), the
   next uid of a player, and placing a card in a monster or spell/trap zone. */
#include "duel.h"
#include "card_db.h"

typedef struct DuelListCard {          /* view of duel.list[] entries */
    unsigned short id : 13;
    unsigned short placed : 2;         /* times put on the field, saturating at 3 */
    unsigned short b15 : 1;
    unsigned short hi;
} DuelListCard;

extern int duel_order_ctr;             /* 0x089195CC activation order of the zones */
extern unsigned int duel_turn_player;

int duel_act_target2(DuelAct *a, int i);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_card_protected(int id, int uid);
void duel_bzero(void *p, int n);
void duel_088DF554(unsigned int p, int z, int bit, int v);
int duel_count_active(int id);
void duel_slot_effect_add(unsigned int p, int z, int a, int id, int b);
unsigned int duel_field_flag9(unsigned int p, int z);

/* Card uid is one of the targets of activation a (only for the cards that keep targets). */
int duel_act_has_target(DuelAct *a, int uid)
{
    int i;

    switch (a->id) {
    case 0x132E:
    case 0x1754:
    case 0x1A30:
    case 0x14B4:
        return 1;
    case 0x147E:
    case 0x1489:
    case 0x16AB:
    case 0x1727:
        for (i = 0; i < a->n_targets; i++) {
            if (uid == duel_act_target2(a, i)) {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

/* The single target of a is a monster zone. */
int duel_act_target_is_monster(DuelAct *a)
{
    return a->n_targets == 1 && (unsigned char)((duel_act_target(a, 0) & 0xFFFF) >> 8) < 5;
}

#ifdef NON_MATCHING /* the original keeps the 1 of the n_targets test as the return value */
int duel_act_target_flag9(DuelAct *a)
{
    unsigned int t = duel_act_target(a, 0);
    unsigned int p = t & 0xFF;
    int z = (unsigned char)((t & 0xFFFF) >> 8);

    if (a->n_targets == 1) {
        if (z < 5 || z > 10) {
            if (z < 5) {
                if (duel_field_flag9(p, z)) {
                    return 1;
                }
                return 0;
            }
            return 0;
        }
        return 1;
    }
    return 0;
}
#endif

int duel_card_e59b0(int id)
{
    if (duel_slot_has_effect(0, 0xB, 0x1407)) {
        return 1;
    }
    if (duel_card_protected(id, -1)) {
        return 1;
    }
    return card_is_in_group_17ae(id);
}

unsigned int duel_field_flag9(unsigned int p, int z)
{
    return duel.players[p & 1].field[z].flags >> 9 & 1;
}

unsigned int duel_slot_e5a48(unsigned int p, int z)
{
    unsigned int id = duel.players[p & 1].field[z].card.id;

    switch (id) {
    case 0x1636:
    case 0x172F:
    case 0x1809:
        return duel.players[p & 1].field[z].flags >> 9 & 1;
    }
    return card_attribute_to_group(id);
}

int duel_field_e5ae0(unsigned int p, int z)
{
    switch (card_get_attribute_icon(duel.players[p & 1].field[z].card.id)) {
    case 0:
    case 7:
        return 1;
    case 8:
        return !(duel_slot_e5a48(p, z) != 0);
    }
    return 0;
}

int duel_card_e5b88(int id, int kind)
{
    return kind == card_get_kind(id);
}

void duel_card_copy(DuelCard *dst, DuelCard *src)
{
    *dst = *src;
}

void duel_card_swap(DuelCard *a, DuelCard *b)
{
    DuelCard t = *a;

    *a = *b;
    *b = t;
}

int duel_card_same(DuelCard *a, DuelCard *b)
{
    return *(int *)a == *(int *)b;
}

#ifdef NON_MATCHING /* the original folds the field offset into the base address of each access */
/* Next uid counter of player p, wrapping past 0xFF. */
void duel_uid_next(unsigned int p)
{
    int *n = &duel.players[p & 1].unk4;

    (*n)++;
    if ((unsigned int)*n > 0xFF) {
        *n = duel.players[p & 1].unk8 + 2;
    }
}
#endif

#ifdef NON_MATCHING /* the original stores the activation counter before the zone fields */
/* Put card c in monster zone z of player p with position pos and face. */
void duel_place_monster(unsigned int p, int z, DuelCard *c, unsigned short pos, int up)
{
    unsigned int q = p & 1;
    DuelSlot *s = &duel.players[q].field[z];

    duel_bzero(s, 0x14);
    duel_card_copy(&s->card, c);
    duel_order_ctr++;
    s->unk4 = duel_order_ctr;
    s->pos = pos;
    s->faceup = up;
    ((DuelListCard *)&duel.list[CARD_UID(*c)])->placed++;
    duel.players[q].field[z].flags |= 0x200000;
    duel_088DF554(p, z, 7, up);
    duel_088DF554(p, z, 0, p != c->owner);
}
#endif

#ifdef NON_MATCHING /* same as duel_place_monster */
/* Put card c in spell/trap zone z of player p, face up or set. */
void duel_place_st(unsigned int p, int z, DuelCard *c, int up)
{
    unsigned short id = c->id;
    unsigned int q = p & 1;
    DuelSlot *s = &duel.players[q].field[z];

    duel_bzero(s, 0x14);
    duel_card_copy(&s->card, c);
    duel_order_ctr++;
    s->unk4 = duel_order_ctr;
    s->pos = 0;
    s->faceup = up;
    ((DuelListCard *)&duel.list[CARD_UID(*c)])->placed++;
    s->link = 0;
    duel_088DF554(p, z, 0, p != c->owner);
    duel_088DF554(p, z, 0x14, up == 0);
    duel_088DF554(p, z, 7, up);
    if (up == 0 && card_get_type(id) == 0x16 && duel_count_active(0x1390)) {
        duel_slot_effect_add(p, z, (p == duel_turn_player) << 4 | 1, 0x1390, 0);
    }
}
#endif
