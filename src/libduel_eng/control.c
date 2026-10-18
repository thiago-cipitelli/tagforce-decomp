// @range 0x088EE93C-0x088EF170
// @asm 0x088EEA30-0x088EEF84
/* Monster z of player p: marking it as having attacked, the player that controls it (the effects in
   its list that change control), and two placement checks. */
#include "duel.h"
#include "card_db.h"

int duel_count_active(int id);
int duel_count_attr(unsigned int p, int attr);
int duel_pick_st_zone(unsigned int p);
int field_get_card(unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
unsigned int duel_slot_immunity(unsigned int p, int z);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
int duel_effect_applies(int id, unsigned int p, unsigned int ref);
void duel_slot_e01fc(unsigned int p, int z, int a, int id, int b);
int duel_slot_eef84(unsigned int p, int z);

/* Monster z of player p has declared an attack (bit 21) and used one of its two attacks. */
void duel_mark_attacked(unsigned int p, int z)
{
    duel.players[p & 1].field[z].flags |= 0x200000;
    duel.players[p & 1].field[z].flags |= 1 << ((duel.players[p & 1].field[z].flags >> 22 & 1) ? 23 : 22);
    switch (field_get_card(p, z)) {
    case 0x186C:
    case 0x18A6:
    case 0x19C1:
    case 0x1A4E:
        duel_slot_e01fc(p, z, 1, field_get_card(p, z), 0);
        break;
    }
}

#ifdef NON_MATCHING /* same code; the owner, the current controller and the effect fields get other $s registers */
/* Player controlling monster z of player p: its owner, flipped by bit 0 of the flags, then by the
   control-changing effects in its list (unless arg, which only counts the permanent ones). */
unsigned int duel_field_controller(unsigned int p, int z, int arg)
{
    unsigned int base = duel.players[p & 1].field[z].card.owner;
    unsigned int cur = base;
    unsigned int imm = duel_slot_immunity(p, z);
    unsigned int e;

    if (duel_effect_applies(0x1632, p, imm)) {
        return cur;
    }
    cur ^= duel.players[p & 1].field[z].flags & 1;
    for (e = duel.players[p & 1].field[z].effects; e != 0;) {
        DuelEffect *ef = &duel.effects[e];
        unsigned short src = ef->src;

        e = ef->next;
        switch (ef->kind & 0xF) {
        case 1:
            if (arg != 0) {
                break;
            }
        case 5:
            if (card_get_special_group(src) == 1 && (duel_count_active(0x148E) || duel_count_active(0x14DA))) {
                break;
            }
            switch (src) {
            case 0x11CF:
            case 0x1227:
            case 0x12C3:
            case 0x12D1:
            case 0x12E2:
            case 0x12FC:
            case 0x1322:
            case 0x13AB:
            case 0x13B0:
            case 0x13EA:
            case 0x142A:
            case 0x1430:
            case 0x143A:
            case 0x14B8:
            case 0x14CD:
            case 0x14CE:
            case 0x1466:
            case 0x1581:
            case 0x169A:
            case 0x17E0:
            case 0x1877:
            case 0x18C6:
            case 0x18B5:
            case 0x18BF:
            case 0x18C0:
            case 0x18C1:
            case 0x18C2:
            case 0x195D:
            case 0x19C0:
                cur = ef->arg & 1;
                break;
            case 0x1857:
                cur = base;
                break;
            case 0x1925:
                cur = 1 - (ef->arg & 1);
                break;
            }
            break;
        case 10:
            if (arg != 0) {
                break;
            } else {
                unsigned int q = EFF_PLAYER(ef);
                int zz = EFF_ZONE(ef);
                DuelSlot *s = &duel.players[q & 1].field[zz];

                if (s->card.id == 0 || s->faceup == 0 || s->flags >> 5 & 1) {
                    break;
                }
                if ((int)(imm & ((p != q) + 1)) > 0 && card_get_type(s->card.id) != 0x17) {
                    break;
                }
                switch (s->card.id) {
                case 0x1322:
                case 0x1466:
                case 0x169A:
                case 0x1877:
                    base = q;
                    cur = q;
                    break;
                case 0x13EA:
                    base = s->link;
                    cur = base;
                    break;
                }
            }
            break;
        case 6:
        case 7:
            if (arg != 0) {
                break;
            } else {
                unsigned int q = EFF_PLAYER(ef);
                int zz = EFF_ZONE(ef);

                if (duel.players[q & 1].field[zz].card.id == 0 || duel.players[q & 1].field[zz].faceup == 0 ||
                    duel.players[q & 1].field[zz].flags >> 5 & 1) {
                    break;
                }
                switch (field_get_card(q, zz)) {
                case 0x18BF:
                case 0x18C0:
                case 0x18C1:
                case 0x18C2:
                    if (duel_field_attr_is(p, z, card_get_kind(field_get_card(q, zz)))) {
                        base = ef->arg & 1;
                        cur = base;
                    }
                    break;
                case 0x1A9C:
                    if (duel_slot_ref_immune(p, z, p != q) == 0) {
                        base = q;
                        cur = q;
                    }
                    break;
                }
            }
            break;
        }
    }
    return cur;
}
#endif

int duel_slot_eef84(unsigned int p, int z)
{
    int id;

    if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
        id = field_get_card(p, z);
        if (card_is_in_group_17d2(id)) {
            return 1;
        }
        switch (id) {
        case 0x17E4:
        case 0x1826:
        case 0x1860:
            if (duel_count_attr(p, 3) || duel_count_attr(1 - p, 3)) {
                return 1;
            }
            break;
        }
    }
    return 0;
}

int duel_slot_ef098(unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 1;
    }
    if (duel_slot_eef84(p, z)) {
        return 0;
    }
    if (duel_field_flag9(p, z) != 0 && duel_pick_st_zone(1 - p) < 0) {
        return 0;
    }
    return 1;
}
