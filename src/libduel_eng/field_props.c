// @range 0x088F6380-0x088F71D8
// @asm 0x088F71A4-0x088F71D8
/* Effective level of monster z of player p: printed level, copies, level-changing effects in its
   list, 0x15E3 and 0x150B. */
#include "duel.h"
#include "card_db.h"

int duel_slot_e1854(unsigned int p, int z);
int field_get_card(unsigned int p, int z);
unsigned int duel_slot_immunity(unsigned int p, int z);
unsigned int duel_slot_ref(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
void duel_slot_foreach_effect(unsigned int p, int z, int (*fn)(DuelEffect *e, int *ctx), int *ctx);
int duel_level_cb(DuelEffect *e, int *ctx);
unsigned int duel_field_type_ex(unsigned int p, int z, int skip);
unsigned int duel_field_type(unsigned int p, int z);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);

int duel_field_level(unsigned int p, int z)
{
    unsigned int e;
    int id = duel.players[p & 1].field[z].card.id;
    unsigned int imm = duel_slot_immunity(p, z);
    unsigned int ref = duel_slot_ref(p, z);
    int lvl;
    int ctx[2];
    unsigned int q;

    if (id == 0) {
        return 0;
    }
    lvl = card_get_level(id);
    if (z >= 5) {
        return lvl;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return lvl;
    }
    if (id == 0x19ED) {
        unsigned short r = duel_slot_e1854(p, z);

        if (r != 0xFFFF) {
            return duel_field_level(r & 0xFF, (unsigned char)((r & 0xFFFF) >> 8));
        }
    }
    ctx[0] = CARD_UID(duel.players[p & 1].field[z].card);
    ctx[1] = lvl;
    duel_slot_foreach_effect(0, 11, duel_level_cb, ctx);
    lvl = ctx[1];
    for (e = duel.players[p & 1].field[z].effects; e != 0;) {
        DuelEffect *ef = &duel.effects[e];
        unsigned short src = ef->src;
        unsigned short player = src & 0xFF;
        unsigned short zone = src >> 8 & 0xFF;

        e = ef->next;
        switch (ef->kind & 0xF) {
        case 1:
            switch (src) {
            case 0x15C7:
                lvl -= ef->arg * 2;
                break;
            }
            break;
        case 5:
            switch (src) {
            case 0x1602:
            case 0x1608:
                lvl = (unsigned char)((ef->arg & 0xF0) >> 4);
                e = duel.effects[duel.effects[(unsigned short)e].next].next;
                break;
            case 0x1472:
                switch (id) {
                case 0x1472:
                case 0x172F:
                case 0x1809:
                    lvl = 4;
                    break;
                case 0x1636:
                    lvl = 10;
                    break;
                }
                break;
            }
            break;
        case 10: {
            int cid = duel.players[player & 1].field[zone].card.id;

            if (cid != 0 && !(duel.players[player & 1].field[zone].flags >> 5 & 1)) {
                if (card_get_type((unsigned short)cid) == 0x17) {
                    if ((int)(ref & ((p != player) + 1)) > 0) {
                        break;
                    }
                } else if ((int)(imm & ((p != player) + 1)) > 0) {
                    break;
                }
                switch ((unsigned short)cid) {
                case 0x15E3:
                    lvl -= 2;
                    break;
                }
            }
            break;
        }
        }
    }
    for (q = 0; (int)q < 2; q++) {
        if (duel.players[q & 1].field[10].card.id != 0 && SLOT_ACTIVE(&duel.players[q & 1].field[10]) &&
            (int)(imm & ((p != q) + 1)) <= 0) {
            switch (duel.players[q & 1].field[10].card.id) {
            case 0x150B:
                if (duel_field_attr_is(p, z, 3)) {
                    lvl--;
                }
                break;
            }
        }
    }
    return lvl;
}

unsigned int duel_field_type(unsigned int p, int z)
{
    return duel_field_type_ex(p, z, -1);
}

/* Effective type of monster z of player p: printed type, then the latest of the type-changing
   cards (ordered by their activation stamp, `skip` excluded) and effects in its list. */
unsigned int duel_field_type_ex(unsigned int p, int z, int skip)
{
    int i;
    unsigned int q;
    int best = 0;
    unsigned int e;
    unsigned int type;
    int id = duel.players[p & 1].field[z].card.id;
    unsigned int imm = duel_slot_immunity(p, z);
    unsigned int ref = duel_slot_ref(p, z);

    if (id == 0) {
        return 0;
    }
    type = card_get_type(id);
    if (z >= 5) {
        return type & 0xFFFF;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return type & 0xFFFF;
    }
    if (id == 0x19ED) {
        unsigned short r = duel_slot_e1854(p, z);

        if (r != 0xFFFF) {
            return duel_field_type(r & 0xFF, (unsigned char)((r & 0xFFFF) >> 8));
        }
    }
    for (q = 0; (int)q < 2; q++) {
        for (i = 0; i < 5; i++) {
            if (duel_slot_e8170(q, i, 0x12A1) && duel_slot_effect_with(q, i, 0x12A1, p) &&
                best < duel.players[q & 1].field[i].unk4 && skip != duel.players[q & 1].field[i].unk4) {
                best = duel.players[q & 1].field[i].unk4;
                type = 10;
            }
            if ((int)(ref & ((p != q) + 1)) <= 0 && duel_slot_e8170(q, i + 5, 0x1357) &&
                best < duel.players[q & 1].field[i + 5].unk4 && skip != duel.players[q & 1].field[i + 5].unk4 &&
                duel.players[q & 1].field[i + 5].link != 0) {
                best = duel.players[q & 1].field[i + 5].unk4;
                type = duel.players[q & 1].field[i + 5].link;
            }
        }
    }
    for (e = duel.players[p & 1].field[z].effects; e != 0;) {
        DuelEffect *ef = &duel.effects[e];
        unsigned short src = ef->src;
        unsigned int zone;

        q = src & 0xFF;
        zone = src >> 8 & 0xFF;
        e = ef->next;
        switch (ef->kind & 0xF) {
        case 1:
        case 2:
        case 3:
        case 4:
            switch (src) {
            case 0x15AE:
                if (best < ef->arg) {
                    best = ef->arg;
                    type = 1;
                }
                break;
            }
            break;
        case 5:
            switch (src) {
            case 0x1602:
            case 0x1608:
                if (best == 0) {
                    type = (unsigned char)(ef->arg >> 8);
                    e = duel.effects[duel.effects[(unsigned short)e].next].next;
                }
                break;
            case 0x1472:
                if (best == 0) {
                    switch (id) {
                    case 0x1472:
                        type = 20;
                        break;
                    case 0x1636:
                        type = 14;
                        break;
                    case 0x172F:
                        type = 2;
                        break;
                    case 0x1809:
                        type = 7;
                        break;
                    }
                }
                break;
            }
            break;
        case 10:
            if (duel.players[q & 1].field[zone].card.id != 0 &&
                SLOT_ACTIVE(&duel.players[q & 1].field[zone]) &&
                best < duel.players[q & 1].field[zone].unk4 && skip != duel.players[q & 1].field[zone].unk4) {
                int cid = duel.players[q & 1].field[zone].card.id;

                if (card_get_type((unsigned short)cid) == 0x17) {
                    if ((int)(ref & ((p != q) + 1)) > 0) {
                        break;
                    }
                } else if ((int)(imm & ((p != q) + 1)) > 0) {
                    break;
                }
                switch ((unsigned short)cid) {
                case 0x149F:
                    best = duel.players[q & 1].field[zone].unk4;
                    type = 1;
                    break;
                }
            }
            break;
        }
    }
    return type & 0xFFFF;
}

/* Effective kind (card_get_kind) of monster z of player p, with the stamp of the card that set it. */
int duel_field_kind_ex(unsigned int p, int z, unsigned int *stamp)
{
    int i;
    unsigned int q;
    unsigned int e;
    int best = 0;
    int kind;
    int id = duel.players[p & 1].field[z].card.id;
    unsigned int imm = duel_slot_immunity(p, z);
    unsigned int ref = duel_slot_ref(p, z);

    if (stamp != 0) {
        *stamp = 0;
    }
    if (id == 0) {
        return 0;
    }
    kind = card_get_kind(id);
    if (z >= 5) {
        return kind;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return kind;
    }
    if (id == 0x19ED) {
        unsigned short r = duel_slot_e1854(p, z);

        if (r != 0xFFFF) {
            return duel_field_kind_ex(r & 0xFF, (unsigned char)((r & 0xFFFF) >> 8), 0);
        }
    }
    for (q = 0; (int)q < 2; q++) {
        for (i = 5; i < 10; i++) {
            if ((int)(ref & ((p != q) + 1)) <= 0 && duel.players[q & 1].field[i].card.id == 0x171F &&
                SLOT_ACTIVE(&duel.players[q & 1].field[i]) && best < duel.players[q & 1].field[i].unk4 &&
                duel.players[q & 1].field[i].link != 0) {
                best = duel.players[q & 1].field[i].unk4;
                kind = duel.players[q & 1].field[i].link;
            }
        }
    }
    for (e = duel.players[p & 1].field[z].effects; e != 0;) {
        DuelEffect *ef = &duel.effects[e];
        unsigned short src = ef->src;
        unsigned int zone;

        q = src & 0xFF;
        zone = src >> 8 & 0xFF;
        e = ef->next;
        switch (ef->kind & 0xF) {
        case 3:
            switch (src) {
            case 0x183B:
                if (ef->arg >= best) {
                    kind = duel.players[p & 1].field[z].link;
                }
                break;
            }
            break;
        case 5:
            switch (src) {
            case 0x1602:
            case 0x1608:
                if (best == 0) {
                    kind = ef->arg & 0xF;
                    e = duel.effects[duel.effects[(unsigned short)e].next].next;
                }
                break;
            case 0x1472:
                if (best == 0) {
                    switch (id) {
                    case 0x1472:
                    case 0x1809:
                        kind = 5;
                        break;
                    case 0x1636:
                        kind = 3;
                        break;
                    case 0x172F:
                        kind = 2;
                        break;
                    }
                }
                break;
            }
            break;
        case 10:
            if (duel.players[q & 1].field[zone].card.id != 0 && SLOT_ACTIVE(&duel.players[q & 1].field[zone]) &&
                best < duel.players[q & 1].field[zone].unk4) {
                int cid = duel.players[q & 1].field[zone].card.id;

                if (card_get_type((unsigned short)cid) == 0x17) {
                    if ((int)(ref & ((p != q) + 1)) > 0) {
                        break;
                    }
                } else if ((int)(imm & ((p != q) + 1)) > 0) {
                    break;
                }
                switch ((unsigned short)cid) {
                case 0x145B:
                    if (duel.players[q & 1].field[zone].link != 0) {
                        best = duel.players[q & 1].field[zone].unk4;
                        kind = duel.players[q & 1].field[zone].link;
                    }
                    break;
                }
            }
            break;
        }
    }
    if (stamp != 0) {
        *stamp = best;
    }
    return kind;
}

int duel_field_kind(unsigned int p, int z)
{
    return duel_field_kind_ex(p, z, 0);
}

/* Kind of monster z of player p as a bit mask (0x18C7 and 0x19EF count as kinds 3-6 too). */
unsigned int duel_field_attrs(unsigned int p, int z)
{
    int stamp;
    int kind = duel_field_kind_ex(p, z, (unsigned int *)&stamp);

    if (stamp == 0) {
        switch (field_get_card(p, z)) {
        case 0x18C7:
        case 0x19EF:
            if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
                return 1 << kind | 0x78;
            }
            break;
        }
    }
    return 1 << kind;
}

#ifdef NON_MATCHING /* the original ANDs the call result first (and v0,v0,v1); mwcc swaps the operands */
int duel_field_attr_is(unsigned int p, int z, int kind)
{
    return (int)(duel_field_attrs(p, z) & 1 << kind) > 0;
}
#endif
