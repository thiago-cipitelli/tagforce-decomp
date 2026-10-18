// @range 0x088E1FE0-0x088E2964
/* Effect lists: a zone, a special card and every card of the list (duel.list) keep a chain of
   DuelEffect entries (card id, kind, arg); find, count and walk them. Kinds above 5 are not
   effects of a card. */
#include "duel.h"

typedef struct DuelListCard {          /* view of duel.list[] entries */
    unsigned short id : 13;
    unsigned short placed : 2;
    unsigned short b15 : 1;
    unsigned short effects;            /* +0x02 head of the card's effect list */
} DuelListCard;

typedef struct DuelEffectBits {        /* view of DuelEffect with the kind byte as nibbles */
    unsigned short src;
    unsigned char kind : 4;
    unsigned char kind_hi : 4;
    unsigned char pad3;
    unsigned short arg;
    unsigned short next;
} DuelEffectBits;

extern DuelListCard duel_cards[512];   /* 0x08917DC8 = duel.list */

#define EFF(i) (&duel.effects[i])

DuelEffect *duel_effect_find(unsigned int e, unsigned int id)
{
    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == id) {
            return ef;
        }
    }
    return 0;
}

DuelEffect *duel_effect_find_kind(unsigned int e, unsigned int id, unsigned int kind)
{
    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == id && (ef->kind & 0xF) == kind) {
            return ef;
        }
    }
    return 0;
}

/* Kind 1 or 2 entry of card id with no high nibble, with arg when arg >= 0. */
DuelEffect *duel_effect_find12(unsigned int e, unsigned int id, int arg)
{
    DuelEffect *ef;

    while (e != 0) {
        ef = EFF(e);
        e = ef->next;
        switch (ef->kind & 0xF) {
        case 1:
        case 2:
            if (ef->src == id && ((DuelEffectBits *)ef)->kind_hi == 0) {
                if (arg < 0 || arg == ef->arg) {
                    return ef;
                }
            }
            break;
        }
    }
    return 0;
}

int duel_effect_count(unsigned int e, unsigned int id)
{
    int n = 0;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == id) {
            n++;
        }
    }
    return n;
}

int duel_effect_count_kind(unsigned int e, unsigned int id, unsigned int kind)
{
    int n = 0;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == id && (ef->kind & 0xF) == kind) {
            n++;
        }
    }
    return n;
}

int duel_effect_count_arg(unsigned int e, unsigned int id, unsigned int arg)
{
    int n = 0;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == id && ef->arg == arg) {
            n++;
        }
    }
    return n;
}

int duel_slot_counter(unsigned int p, int z, int id)
{
    return duel_effect_count(duel.players[p & 1].field[z].effects, id);
}

int duel_slot_counter_kind(unsigned int p, int z, int id, int kind)
{
    return duel_effect_count_kind(duel.players[p & 1].field[z].effects, id, kind);
}

int duel_slot_counter_arg(unsigned int p, int z, int id, int arg)
{
    return duel_effect_count_arg(duel.players[p & 1].field[z].effects, id, arg);
}

int duel_slot_has_effect(unsigned int p, int z, int id)
{
    return duel_effect_find(duel.players[p & 1].field[z].effects, id) != 0;
}

int duel_special_has_effect(int p, int z, int id)
{
    unsigned int f = p == duel_special.player[0];

    if (f) {
        f = z == duel_special.zone[0];
    }
    return duel_effect_find(duel_special.slot[f ^ 1].effects, id) != 0;
}

unsigned int duel_slot_effect_index(unsigned int p, int z, unsigned int id, unsigned int arg)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);
        unsigned int next = ef->next;

        if ((ef->kind & 0xF) < 6 && ef->src == id && ef->arg == arg) {
            return e;
        }
        e = next;
    }
    return 0;
}

int duel_slot_effect_with(unsigned int p, int z, unsigned int id, unsigned int arg)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == id && ef->arg == arg) {
            return 1;
        }
    }
    return 0;
}

int duel_slot_effect_kind(unsigned int p, int z, int id, int kind)
{
    return duel_effect_find_kind(duel.players[p & 1].field[z].effects, id, kind) != 0;
}

int duel_slot_has_effect2(unsigned int p, int z, int id)
{
    return duel_effect_find12(duel.players[p & 1].field[z].effects, id, -1) != 0;
}

int duel_slot_has_effect12_arg(unsigned int p, int z, int id, int arg)
{
    return duel_effect_find12(duel.players[p & 1].field[z].effects, id, arg) != 0;
}

unsigned int duel_slot_effect_arg(unsigned int p, int z, int id)
{
    DuelEffect *ef = duel_effect_find(duel.players[p & 1].field[z].effects, id);

    if (ef != 0) {
        return ef->arg;
    }
    return -1;
}

unsigned int duel_slot_effect_kind_arg(unsigned int p, int z, int id, int kind)
{
    DuelEffect *ef = duel_effect_find_kind(duel.players[p & 1].field[z].effects, id, kind);

    if (ef != 0) {
        return ef->arg;
    }
    return -1;
}

unsigned int duel_slot_effect12_arg(unsigned int p, int z, int id)
{
    DuelEffect *ef = duel_effect_find12(duel.players[p & 1].field[z].effects, id, -1);

    if (ef != 0) {
        return ef->arg;
    }
    return -1;
}

unsigned int duel_card_effect_arg(DuelSlot *s, int id)
{
    DuelEffect *ef = duel_effect_find(s->effects, id);

    if (ef != 0) {
        return ef->arg;
    }
    return -1;
}

int duel_list_has_effect(int uid, int id)
{
    return duel_effect_find(duel_cards[uid].effects, id) != 0;
}

unsigned int duel_list_effect_arg(int uid, int id)
{
    DuelEffect *ef = duel_effect_find(duel_cards[uid].effects, id);

    if (ef != 0) {
        return ef->arg;
    }
    return -1;
}

int duel_list_counter(int uid, int id)
{
    return duel_effect_count(duel_cards[uid].effects, id);
}

/* Sum of the args of the effects of card id in zone z of player p. */
int duel_slot_effect_sum(unsigned int p, int z, unsigned int id)
{
    int n = 0;
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == id) {
            n += ef->arg;
        }
    }
    return n;
}

/* Call fn on each effect of zone z of player p until it returns nonzero; that entry or 0. */
DuelEffect *duel_slot_foreach_effect(unsigned int p, int z, int (*fn)(DuelEffect *e, void *ctx), void *ctx)
{
    unsigned int e = duel.players[p & 1].field[z].effects;
    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if (!!fn(ef, ctx)) {
            return ef;
        }
    }
    return 0;
}

DuelEffect *duel_card_foreach_effect(DuelSlot *s, int (*fn)(DuelEffect *e, void *ctx), void *ctx)
{
    unsigned int e = s->effects;
    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if (!!fn(ef, ctx)) {
            return ef;
        }
    }
    return 0;
}
