// @range 0x088DFFC0-0x088E0890
// @asm 0x088DFFF8-0x088E0890
/* Effect list changes: allocate and free entries (unlinking them from every list), add to a zone or
   to a card of the list, remove by source, kind or card. Index 11 of a player's zones is the
   player's own list (DuelPlayer.effects). */
#include "duel.h"

typedef struct DuelListCard {
    unsigned short id : 13;
    unsigned short placed : 2;
    unsigned short b15 : 1;
    unsigned short effects;
} DuelListCard;

typedef struct DuelEffectBits {
    unsigned short src;
    unsigned char kind : 4;
    unsigned char kind_hi : 4;
    unsigned char pad3;
    unsigned short arg;
    unsigned short next;
} DuelEffectBits;

extern DuelListCard duel_cards[512];

void duel_bzero(void *p, int n);

#define EFF(i) ((DuelEffectBits *)&duel.effects[i])

int duel_effect_alloc(void)
{
    int i = 1;

    while (EFF(i)->kind != 0) {
        i++;
    }
    return i;
}

#ifdef NON_MATCHING /* first draft; loop rotation and registers still differ */
/* Free entry i and unlink it from every list. */
void duel_effect_free(unsigned int i)
{
    unsigned short next = EFF(i)->next;
    unsigned int p;
    int k;

    duel_bzero(&duel.effects[i], 8);
    for (p = 0; (int)p < 2; p++) {
        for (k = 0; k < 12; k++) {
            if (i == duel.players[p & 1].field[k].effects) {
                duel.players[p & 1].field[k].effects = next;
            }
        }
        if (duel_special.slot[p].effects == i) {
            duel_special.slot[p].effects = next;
        }
    }
    for (k = 1; k < 0x200; k++) {
        if (duel_cards[k].effects == i) {
            duel_cards[k].effects = next;
        }
    }
    for (k = 1; k < 0x200; k++) {
        if (EFF(k)->next == i) {
            EFF(k)->next = next;
        }
    }
}

/* Remove the effects (kind above 5) placed by the card in zone z of player p from every zone. */
void duel_effects_clear_src(unsigned short p, short z)
{
    unsigned int q;
    int k;

    for (q = 0; (int)q < 2; q++) {
        for (k = 0; k < 12; k++) {
            unsigned int e = duel.players[q & 1].field[k].effects;

            while (e != 0) {
                unsigned int i = e;
                DuelEffectBits *ef = EFF(i);

                e = ef->next;
                if (ef->kind > 5 && ef->src == (unsigned short)((p & 0xFF) | z << 8)) {
                    duel_effect_free(i);
                }
            }
        }
    }
}

/* Append an effect to the list of zone z of player p. */
void duel_slot_effect_add(unsigned int p, int z, unsigned char kind, unsigned short src, unsigned short arg)
{
    int i = duel_effect_alloc();
    unsigned short *link = &duel.players[p & 1].field[z].effects;

    while (*link != 0) {
        link = &EFF(*link)->next;
    }
    *link = i;
    EFF(i)->kind = kind;
    EFF(i)->kind_hi = kind >> 4;
    EFF(i)->src = src;
    EFF(i)->arg = arg;
    EFF(i)->next = 0;
}

/* Append an effect to the list of the card with uid. */
void duel_list_effect_add(int uid, unsigned char kind, unsigned short src, unsigned short arg)
{
    int i = duel_effect_alloc();
    unsigned int e = duel_cards[uid].effects;

    if (e == 0) {
        duel_cards[uid].effects = i;
    } else {
        unsigned int last;

        do {
            last = e;
            e = EFF(e)->next;
        } while (e != 0);
        EFF(last)->next = i;
    }
    EFF(i)->kind = kind;
    EFF(i)->kind_hi = kind >> 4;
    EFF(i)->src = src;
    EFF(i)->arg = arg;
    EFF(i)->next = 0;
}

void duel_slot_effects_free(DuelSlot *s)
{
    unsigned int e = s->effects;

    while (e != 0) {
        unsigned int i = e;

        e = EFF(i)->next;
        duel_effect_free(i);
    }
}

/* Free the entries above kind 5 with source src (only the first unless all). */
void duel_effects_free_src(unsigned int e, short src, int all)
{
    while (e != 0) {
        unsigned int i = e;
        DuelEffectBits *ef = EFF(i);

        e = ef->next;
        if (ef->kind >= 6 && ef->src == src) {
            duel_effect_free(i);
            if (all == 0) {
                return;
            }
        }
    }
}

void duel_effects_free_card(unsigned int e, short id, int all)
{
    while (e != 0) {
        unsigned int i = e;
        DuelEffectBits *ef = EFF(i);

        e = ef->next;
        if (ef->kind <= 5 && ef->src == id) {
            duel_effect_free(i);
            if (all == 0) {
                return;
            }
        }
    }
}

void duel_effects_free_kind(unsigned int e, short id, unsigned int kind, int all)
{
    while (e != 0) {
        unsigned int i = e;
        DuelEffectBits *ef = EFF(i);

        e = ef->next;
        if (ef->kind == kind && ef->src == id) {
            duel_effect_free(i);
            if (all == 0) {
                return;
            }
        }
    }
}

/* Free every effect of the zone but the 0x164F card effects. */
void duel_slot_effects_free_keep(DuelSlot *s)
{
    unsigned int e = s->effects;

    while (e != 0) {
        unsigned int i = e;
        DuelEffectBits *ef = EFF(i);

        e = ef->next;
        if (ef->kind > 5 || ef->src != 0x164F) {
            duel_effect_free(i);
        }
    }
}

/* Card c leaves: drop the effects that pointed to it (card 0x1368 in a spell/trap zone and the
   player effects of the cards below). */
void duel_effects_drop_card(DuelCard *c)
{
    int uid = CARD_UID(*c);
    unsigned int p;
    int z;
    unsigned int e;

    for (p = 0; (int)p < 2; p++) {
        for (z = 5; z < 11; z++) {
            DuelSlot *s = &duel.players[p & 1].field[z];

            if (s->card.id == 0x1368 && SLOT_ACTIVE(s)) {
                e = s->effects;
                while (e != 0) {
                    unsigned int i = e;
                    DuelEffectBits *ef = EFF(i);

                    e = ef->next;
                    if (ef->kind == 3 && ef->src == 0x1368 && ef->arg == uid) {
                        duel_effect_free(i);
                    }
                }
            }
        }
    }
    e = duel.players[c->owner].effects;
    while (e != 0) {
        unsigned int i = e;
        DuelEffectBits *ef = EFF(i);

        e = ef->next;
        if (ef->kind < 6 && ef->arg == CARD_UID(*c)) {
            switch (ef->src) {
            case 0x13A6:
            case 0x1466:
            case 0x1522:
            case 0x156B:
            case 0x1571:
            case 0x17DE:
            case 0x185C:
            case 0x18BC:
            case 0x197F:
            case 0x19F7:
            case 0x19F8:
            case 0x1A61:
            case 0x1A62:
            case 0x1A64:
            case 0x1A9F:
                duel_effect_free(i);
                break;
            }
        }
    }
}
#endif
