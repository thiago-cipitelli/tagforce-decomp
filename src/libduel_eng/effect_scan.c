// @range 0x088E0890-0x088E1FE0
// @asm 0x088E0C30-0x088E0E8C
// @asm 0x088E1390-0x088E1574
// @asm 0x088E1D00-0x088E1FE0
/* Effect list upkeep and scans by source: drop expiring entries at the end of a turn, reset a zone,
   move the entries of a source to another zone, count and find the entries placed on a zone by
   a given card (kinds 6-11 point back to their source zone). */
#include "duel.h"

typedef struct DuelListCard {          /* view of duel.list[] entries */
    unsigned short id : 13;
    unsigned short placed : 2;
    unsigned short b15 : 1;
    unsigned short effects;
} DuelListCard;

typedef struct DuelEffectBits {        /* view of DuelEffect with the kind byte as nibbles */
    unsigned short src;
    unsigned char kind : 4;
    unsigned char kind_hi : 4;
    unsigned char pad3;
    unsigned short arg;
    unsigned short next;
} DuelEffectBits;

extern DuelListCard duel_cards[512];

#define EFF(i) (&duel.effects[i])
#define EFFB(i) ((DuelEffectBits *)&duel.effects[i])
#define SRC_SLOT(s) (&duel.players[(unsigned char)(s) & 1].field[(unsigned char)((s) >> 8)])

void duel_effect_free(unsigned int i);
void duel_effects_free_src(unsigned int e, unsigned short src, int all);
int field_get_card(unsigned int p, int z);
unsigned int duel_slot_immunity(unsigned int p, int z);
unsigned int duel_slot_ref(unsigned int p, int z);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
int card_get_type(int id);
int card_is_monster(int id);
int card_is_attribute8(int id);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
int duel_slot_e1390(unsigned int p, int z, int id, unsigned int q);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
unsigned short duel_slot_e1854(unsigned int p, int z);

/* Free the kind 3 entries of the card with uid. */
void duel_088E0890(int uid)
{
    unsigned int e = duel_cards[uid].effects;

    while (e != 0) {
        unsigned int next = EFF(e)->next;

        switch (EFF(e)->kind & 0xF) {
        case 3:
            duel_effect_free(e);
            break;
        }
        e = next;
    }
}

/* Count down the timed entries (kinds 1 and 2) and drop the ones that expire with the turn. */
void duel_effects_tick(void)
{
    int i;

    for (i = 1; i < 0x200; i++) {
        DuelEffectBits *ef = EFFB(i);

        switch (ef->kind) {
        case 1:
        case 2:
            if (ef->kind_hi != 0) {
                ef->kind_hi -= 1;
            } else {
                duel_effect_free(i);
            }
            break;
        case 7:
            switch (duel.players[(unsigned char)ef->src & 1].field[(unsigned char)(ef->src >> 8)].card.id) {
            case 0x175D:
                if (ef->arg < duel.turn) {
                    duel_effect_free(i);
                }
                break;
            }
            break;
        case 8:
            switch (duel.players[(unsigned char)ef->src & 1].field[(unsigned char)(ef->src >> 8)].card.id) {
            case 0x1505:
            case 0x1644:
            case 0x1843:
            case 0x1958:
            case 0x1A4E:
            case 0x1A79:
                duel_effect_free(i);
                break;
            }
            break;
        }
    }
}

/* Drop the kind 4 entries and the entries placed by cards 0x1852 and 0x1561. */
void duel_effects_end_turn(void)
{
    unsigned int next;
    unsigned int p;
    int z;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z <= 11; z++) {
            if (z == 11 || duel.players[p & 1].field[z].card.id != 0) {
                unsigned int e = duel.players[p & 1].field[z].effects;

                while (e != 0) {
                    DuelEffect *ef = EFF(e);
                    next = ef->next;

                    if ((ef->kind & 0xF) == 4) {
                        duel_effect_free(e);
                    } else if ((ef->kind & 0xF) >= 6) {
                        switch (duel.players[(unsigned char)ef->src & 1].field[(unsigned char)(ef->src >> 8)].card.id) {
                        case 0x1561:
                        case 0x1852:
                            duel_effect_free(e);
                            break;
                        }
                    }
                    e = next;
                }
            }
        }
    }
}

#ifdef NON_MATCHING /* register allocation of the loop counters and of the source fields differs */
/* The card in zone z of player p leaves it: drop its own timed entries and every entry it placed. */
void duel_slot_effects_reset(unsigned int p, unsigned int z)
{
    DuelSlot *s = &duel.players[p & 1].field[z];
    unsigned int src;
    int k;
    unsigned int next;
    unsigned int q;

    if (s->card.id != 0) {
        unsigned int e = duel.players[p & 1].field[z].effects;

        while (e != 0) {
            DuelEffect *ef = EFF(e);

            next = ef->next;
            switch (ef->kind & 0xF) {
            case 7:
            case 9:
            case 3:
            case 2:
                duel_effect_free(e);
                break;
            }
            e = next;
        }
        src = (p & 0xFF) | (z & 0xFF) << 8;
        for (q = 0; (int)q < 2; q++) {
            for (k = 0; k <= 11; k++) {
                duel_effects_free_src(duel.players[q & 1].field[k].effects, src, 1);
            }
        }
        s->link = 0;
        duel.players[p & 1].field[z].flags &= ~0x10;
    }
}

/* Set kind and arg of entry i; kind 0 empties it, a card kind on a zone entry keeps its card id. */
void duel_effect_set(unsigned int p, int z, int i, int kind, unsigned short arg)
{
    DuelEffectBits *ef = EFFB(i);

    if (kind == 0) {
        kind = 1;
        ef->kind_hi = 0;
        ef->src = 0;
    } else if (ef->kind >= 6 && kind < 6) {
        ef->src = duel.players[(unsigned char)ef->src & 1].field[(unsigned char)(ef->src >> 8)].card.id;
    }
    ef->kind = kind;
    ef->arg = arg;
}
#endif

/* Entries placed from zone `from` now come from zone `to`. */
void duel_effects_move_src(unsigned short from, unsigned short to)
{
    unsigned int e;
    unsigned int fp = from & 0xFF;
    unsigned int p;
    int z;
    unsigned int fz = from >> 8 & 0xFF;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            if (duel.players[p & 1].field[z].card.id != 0) {
                e = duel.players[p & 1].field[z].effects;

                while (e != 0) {
                    DuelEffect *ef = EFF(e);

                    e = ef->next;
                    if ((ef->kind & 0xF) >= 6) {
                        unsigned int sz = ef->src >> 8 & 0xFF;
                        unsigned int sp = ef->src & 0xFF;

                        if (sp == fp && sz == fz) {
                            ef->src = to;
                        }
                    }
                }
            }
        }
    }
}

/* Entries of list e (kinds 6-11) placed by card id, from enabled zones; active: source not negated;
   imm/ref: per side (1 own, 2 opponent), skip sources the zone is immune to. */
int duel_effects_count_src(unsigned int p, unsigned int e, int id, int active, int imm, int ref)
{
    unsigned int next;
    int n = 0;

    for (; e != 0; e = next) {
        DuelEffect *ef = EFF(e);
        int kind;
        unsigned int sp;
        int sz;
        int cid;
        int side;

        next = ef->next;
        kind = ef->kind & 0xF;
        if (kind < 6 || kind > 11) {
            continue;
        }
        sp = ef->src & 0xFF;
        sz = ef->src >> 8 & 0xFF;
        cid = field_get_card(sp, sz);
        if (cid != id) {
            continue;
        }
        if (duel.zone_off & 1 << (sz + sp * 16)) {
            continue;
        }
        if (active != 0 && (duel.players[sp & 1].field[sz].flags >> 5 & 1)) {
            continue;
        }
        side = (p != sp) + 1;
        if ((int)(imm & side) > 0) {
            if (card_get_type(cid) == 0x16) {
                continue;
            }
            if (sz >= 5 && sz <= 10 && card_is_monster(cid)) {
                continue;
            }
        }
        if ((int)(ref & side) > 0 && card_get_type(cid) == 0x17) {
            continue;
        }
        n++;
    }
    return n;
}

int duel_slot_count_src(unsigned int p, int z, int id)
{
    return duel_effects_count_src(p, duel.players[p & 1].field[z].effects, id, 0, 0, 0);
}

int duel_special_count_src(unsigned int p, int z, int id)
{
    return duel_effects_count_src(p, duel_special.slot[!(p == duel_special.player[0] && z == duel_special.zone[0])].effects, id, 0, 0, 0);
}

int duel_slot_affected(unsigned int p, int z, int id)
{
    return duel_effects_count_src(p, duel.players[p & 1].field[z].effects, id, 1, 0, 0);
}

int duel_special_affected(unsigned int p, int z, int id)
{
    return duel_effects_count_src(p, duel_special.slot[!(p == duel_special.player[0] && z == duel_special.zone[0])].effects, id, 1, 0, 0);
}

int duel_slot_affected_ex(unsigned int p, int z, int id)
{
    unsigned int imm = duel_slot_immunity(p, z);
    unsigned int ref = duel_slot_ref(p, z);

    return duel_effects_count_src(p, duel.players[p & 1].field[z].effects, id, 1, imm, ref);
}

int duel_slot_e12e0(unsigned int p, int z, int id, unsigned int q)
{
    if (card_get_type(id) == 0x17) {
        if (duel_slot_ref_immune(p, z, p != q)) {
            return 0;
        }
    } else if (duel_slot_immune(p, z, p != q)) {
        return 0;
    }
    return duel_slot_e1390(p, z, id, q);
}

#ifdef NON_MATCHING /* register allocation of the parameters and of the source fields differs */
/* Entries on zone z of p placed by card id of player q, source not negated. */
int duel_slot_e1390(unsigned int p, int z, int id, unsigned int q)
{
    int n = 0;
    unsigned int e = duel.players[p & 1].field[z].effects;
    unsigned int sp;
    int sz;
    int cid;
    int kind;

    while (e != 0) {
        DuelEffect *ef = EFF(e);
        unsigned short src = ef->src;

        e = ef->next;
        kind = ef->kind & 0xF;
        if (kind >= 6 && kind <= 11) {
            sp = src & 0xFF;
            sz = src >> 8 & 0xFF;
            cid = field_get_card(sp, sz);
            if (q == sp && !(duel.players[sp & 1].field[sz].flags >> 5 & 1) && cid == id) {
                n++;
            }
        }
    }
    return n;
}

/* Entries of kind 10 and above on zone z of p whose source zone holds a card. */
int duel_slot_count_14c0(unsigned int p, int z)
{
    int n = 0;
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) >= 10) {
            unsigned int sz = ef->src >> 8 & 0xFF;
            unsigned int sp = ef->src & 0xFF;

            if (duel.players[sp & 1].field[sz].card.id != 0) {
                n++;
            }
        }
    }
    return n;
}
#endif

/* Zone sz of player q has an entry of kind 10 and above from zone z of p. */
int duel_slot_has_src10(unsigned int p, unsigned int z, unsigned int q, int sz)
{
    unsigned int e = duel.players[q & 1].field[sz].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) >= 10) {
            unsigned int ez = ef->src >> 8 & 0xFF;
            unsigned int ep = ef->src & 0xFF;

            if (ep == p && ez == z) {
                return 1;
            }
        }
    }
    return 0;
}

/* Same with kinds 6 and 7 counting when their high nibble is odd. */
int duel_slot_has_src(unsigned int p, unsigned int z, unsigned int q, int sz)
{
    unsigned int e = duel.players[q & 1].field[sz].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);
        int kind;
        int hit;

        e = ef->next;
        kind = ef->kind & 0xF;
        hit = kind >= 10;
        switch (kind) {
        case 6:
        case 7:
            if ((int)(((DuelEffectBits *)ef)->kind_hi & 1) > 0) {
                hit = 1;
            }
            break;
        }
        if (hit) {
            unsigned int ez = ef->src >> 8 & 0xFF;
            unsigned int ep = ef->src & 0xFF;

            if (ep == p && ez == z) {
                return 1;
            }
        }
    }
    return 0;
}

/* Kind of the entry placed on zone z of p by zone sz of q, 0 if none. */
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz)
{
    unsigned int e;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    e = duel.players[p & 1].field[z].effects;
    while (e != 0) {
        DuelEffect *ef = EFF(e);
        int kind;

        e = ef->next;
        kind = ef->kind & 0xF;
        if (kind >= 6) {
            unsigned int ez = ef->src >> 8 & 0xFF;
            unsigned int ep = ef->src & 0xFF;

            if (ep == q && ez == sz) {
                return kind;
            }
        }
    }
    return 0;
}

/* Face-up monsters of p affected by card id. */
int duel_count_e17a0(unsigned int p, int id)
{
    int z;
    int n = 0;
    DuelPlayer *pl = &duel.players[p & 1];

    for (z = 0; z < 5; z++) {
        if (pl->field[z].card.id != 0 && pl->field[z].faceup != 0 && duel_slot_affected(p, z, id)) {
            n++;
        }
    }
    return n;
}

/* Zone (player | zone << 8) holding an entry placed by zone z of p, 0xFFFF if none. */
unsigned short duel_slot_e1854(unsigned int p, int z)
{
    unsigned int q;
    unsigned int k;

    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0xFFFF;
    }
    for (q = 0; (int)q < 2; q++) {
        for (k = 0; (int)k <= 10; k++) {
            if (!(!duel_st_affects(q, k, p, z) != 0)) {
                return (q & 0xFF) | (k & 0xFF) << 8;
            }
        }
    }
    return 0xFFFF;
}

/* Same for the two special cards. */
unsigned short duel_special_e1934(unsigned int p, unsigned int z)
{
    unsigned int e;
    int i;

    for (i = 0; i < 2; i++) {
        e = duel_special.slot[i].effects;
        while (e != 0) {
            DuelEffect *ef = EFF(e);
            int kind;

            e = ef->next;
            kind = ef->kind & 0xF;
            if (kind >= 6) {
                unsigned int ez = ef->src >> 8 & 0xFF;
                unsigned int ep = ef->src & 0xFF;

                if (ep == p && ez == z) {
                    if (i == 0) {
                        return (duel_special.player[0] & 0xFF) | (duel_special.zone[0] & 0xFF) << 8;
                    }
                    return (duel_special.player[1] & 0xFF) | (duel_special.zone[1] & 0xFF) << 8;
                }
            }
        }
    }
    return 0xFFFF;
}

/* Source of the first entry on zone z of p placed by card id (an active one if active), 0xFFFF if none. */
unsigned short duel_slot_src_of(unsigned int p, int z, int id, int active)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) >= 6) {
            unsigned int src = ef->src;
            unsigned int sp = src & 0xFF;
            unsigned int sz = src >> 8 & 0xFF;

            if (id == field_get_card(sp, sz) && (active == 0 || SLOT_ACTIVE(&duel.players[sp & 1].field[sz]))) {
                return src;
            }
        }
    }
    return 0xFFFF;
}

/* Same, active and with position pos. */
unsigned short duel_slot_src_of_pos(unsigned int p, int z, int id, unsigned int pos)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) >= 6) {
            unsigned int src = ef->src;
            unsigned int sp = src & 0xFF;
            unsigned int sz = src >> 8 & 0xFF;

            if (id == field_get_card(sp, sz) && SLOT_ACTIVE(&duel.players[sp & 1].field[sz]) &&
                pos == (duel.players[sp & 1].field[sz].flags >> 4 & 1)) {
                return src;
            }
        }
    }
    return 0xFFFF;
}

/* Source of the kind 13 entry of zone z of p, 0xFFFF if none. */
unsigned short duel_slot_src13(unsigned int p, int z)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);

        e = ef->next;
        if ((ef->kind & 0xF) == 13) {
            return ef->src;
        }
    }
    return 0xFFFF;
}

#ifdef NON_MATCHING /* the zone of the source is computed before its player; the two kind tests are laid out apart */
/* Source of the first kind 10 or 11 entry whose card is of attribute 8 (an active one if active). */
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = EFF(e);
        unsigned int src;

        e = ef->next;
        switch (ef->kind & 0xF) {
        case 10:
        case 11:
            src = ef->src;
            if (card_is_attribute8(duel.players[(unsigned char)src & 1].field[(unsigned char)(src >> 8)].card.id) &&
                (active == 0 || SLOT_ACTIVE(&duel.players[(unsigned char)src & 1].field[(unsigned char)(src >> 8)]))) {
                return src;
            }
            break;
        }
    }
    return 0xFFFF;
}

/* The card in zone z of p can act on (x, y): not while it is bound by a kind 12 or 13 entry. */
int duel_slot_can_act(unsigned int p, unsigned int z, int x, int y)
{
    unsigned int src = duel_slot_e1854(p, z);
    DuelAct a = { 0 };

    a.id = field_get_card(p, z);
    a.player = p;
    a.zone = z;
    a.b3_4 = 0;
    a.uid = CARD_UID(duel.players[p & 1].field[z].card);
    if ((src & 0xFFFF) != 0xFFFF) {
        switch (duel_st_affects(src & 0xFF, (src & 0xFFFF) >> 8 & 0xFF, p, z)) {
        case 13:
            return 0;
        case 12:
            return 0;
        }
    }
    return duel_act_check_08853CE8(&a, x, y) == 1;
}
#endif
