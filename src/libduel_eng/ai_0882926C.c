// @range 0x0882926C-0x0882A23C
/* Target filters of the summon checks (callbacks (q, z, b) that return 0x800 when the monster in
   zone z + b of player q can be tributed for the card in the attack/summon record). */
#include "duel.h"

typedef struct DuelAtkRec {            /* 0x08914A14, 0x1C: the attack being resolved (sent as is to the peer) */
    unsigned int player : 1;           /* attacking card */
    unsigned int zone : 4;
    unsigned int b5 : 1;               /* the attack goes on (not cancelled) [?] */
    unsigned int kind : 2;             /* bits 6-7 */
    unsigned int id : 15;
    unsigned int uid : 9;
    unsigned short h18;                /* +0x04 duel_h_08914A18 */
    unsigned short atk_id;             /* +0x06 duel_h_08914A1A */
    unsigned char n;                   /* +0x08 number of targets */
    unsigned char count;               /* +0x09 distinct targets */
    unsigned short uids[3];            /* +0x0A */
    unsigned short locs[3];            /* +0x10 player | zone << 8 */
    unsigned char f0 : 1;              /* +0x16 duel_w_08914A2A */
    unsigned char f1 : 1;
    unsigned char owner : 1;           /* bit 0 of the uid of the attacking card */
    unsigned char f3 : 1;
    unsigned char f4 : 2;
    unsigned char f6 : 1;
    unsigned char f7 : 1;
    unsigned char mode : 3;            /* +0x17 1 attack, 2 direct attack, 3 summon, 4 special summon [?] */
    unsigned char g3 : 5;
} DuelAtkRec;

extern DuelAtkRec duel_atk_rec;      /* duel_summon (duel.h) seen as the attack/summon record */
extern unsigned short duel_h_08914A1A;

int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_can_tribute(unsigned int p, unsigned int q, int z);
int card_id_remap_164a(int id);
int duel_st_affects();
unsigned int duel_field_type(unsigned int p, int z);
int card_get_type(int id);
int card_get_kind(int id);
int duel_field_atk(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
int field_get_card(unsigned int p, int z);
int card_is_in_group_1b2d(int id);
int duel_count_free_monster_zones();
int duel_zone_usable(unsigned int p, unsigned int z);

/* Tribute for the summon of the card a->id (fusion-like monsters with conditions). */
int duel_0882926C(unsigned int q, int z, int b)
{
    int k = z + b;
    unsigned int id;
    int i;
    int found;
    unsigned char n;

    if (k > 4) {
        return 0;
    }
    if (duel_atk_rec.f1) {
        if (duel_tributable(duel_atk_rec.player, q, k) == 0) {
            return 0;
        }
    } else {
        switch (id = duel_atk_rec.id) {
        case 0x164A:
        case 0x164B:
        case 0x164C:
        case 0x164D:
        case 0x164E:
        case 0x164F:
            if (!duel_atk_rec.b5) {
                break;
            }
            found = 0;
            for (i = 5; i < 10; i++) {
                unsigned char o;

                o = duel_atk_rec.player;
                if (duel.players[o & 1].field[i].card.id == card_id_remap_164a(duel_atk_rec.id) &&
                    duel.players[o & 1].field[i].faceup != 0 && duel_st_affects(q, k, o, i) == 0) {
                    found = 1;
                }
            }
            if (!found) {
                return 0;
            }
            break;
        case 0x16EC:
        case 0x1725:
        case 0x1729:
        case 0x1985:
        case 0x1986:
        case 0x1B2E:
        case 0x1B2F:
            if (q != duel_atk_rec.player) {
                return 0;
            }
            if ((unsigned short)duel_field_type(q, k) != card_get_type(id)) {
                return 0;
            }
            break;
        case 0x194C:
            if (q != duel_atk_rec.player && duel.players[q & 1].field[k].faceup == 0) {
                return 0;
            }
            if (duel_field_atk(q, k) < 2000) {
                return 0;
            }
            break;
        case 0x1A40:
            if (q != duel_atk_rec.player && duel.players[q & 1].field[k].faceup == 0) {
                return 0;
            }
            if (duel_field_attr_is(q, k, card_get_kind(id)) == 0) {
                return 0;
            }
            break;
        case 0x1A96:
            if (duel_h_08914A1A == id) {
                if (q != duel_atk_rec.player) {
                    return 0;
                }
                if (duel_atk_rec.player == duel.players[q & 1].field[k].card.owner) {
                    return 0;
                }
            }
            break;
        case 0x1B2D:
            if (duel_h_08914A1A == id) {
                if (q != duel_atk_rec.player && duel.players[q & 1].field[k].faceup == 0) {
                    return 0;
                }
                if (card_is_in_group_1b2d(field_get_card(q, k)) == 0) {
                    return 0;
                }
            }
            break;
        }
        if (duel_can_tribute(duel_atk_rec.player, q, k) == 0) {
            return 0;
        }
    }
    if ((n = duel_atk_rec.n) == 0) {
        if (duel_count_free_monster_zones(duel_atk_rec.player) == 0) {
            if (q != duel_atk_rec.player || duel_zone_usable(q, k) == 0) {
                return 0;
            }
        }
    } else {
        int j;

        for (j = 0; j < n; j++) {
            if (duel_atk_rec.locs[j] == (unsigned short)((unsigned char)q | (unsigned char)k << 8)) {
                return 0;
            }
        }
    }
    return 0x800;
}

int duel_slot_value_1909(unsigned int p, int z);

/* Tribute 0x10BC for the cards 0xFDE/0xFDF/0xFE9: needs a 0xFEE equipped with a counter of at
   least 2/4/6 [?]. */
int duel_088297A0(unsigned int q, int z, int b)
{
    int k = z + b;
    int need = 0;
    DuelEffect *ef;

    if (k > 4) {
        return 0;
    }
    if (duel_tributable(duel_atk_rec.player, q, k) == 0) {
        return 0;
    }
    if (field_get_card(q, k) != 0x10BC) {
        return 0;
    }
    if (duel_count_free_monster_zones(duel_atk_rec.player) == 0) {
        if (duel_atk_rec.player != q) {
            return 0;
        }
        if (duel_zone_usable(q, k) == 0) {
            return 0;
        }
    }
    switch (duel_atk_rec.id) {
    case 0xFDE:
        need = 2;
        break;
    case 0xFDF:
        need = 4;
        break;
    case 0xFE9:
        need = 6;
        break;
    }
    k = duel.players[q & 1].field[k].effects;
    while (k != 0) {
        ef = &duel.effects[k];
        k = ef->next;
        if ((ef->kind & 0xF) == 10) {
            unsigned int ez = EFF_ZONE(ef);
            unsigned int ep = EFF_PLAYER(ef);

            if (duel.players[ep & 1].field[ez].card.id == 0xFEE && !(duel.players[ep & 1].field[ez].flags >> 5 & 1) &&
                duel_slot_value_1909(ep, ez) >= need) {
                return 0x800;
            }
        }
    }
    return 0;
}

int card_same_name(int id, int id2);
int card_is_in_group_18b2(int id);
int duel_slot_count_src(unsigned int p, int z, int id);
int duel_is_cur_card(int id);
int duel_field_level(unsigned int p, int z);

/* Monsters of a given name or kind for the summons that need a specific tribute. */
int duel_08829988(unsigned int q, int z, int b)
{
    int id;
    int k = z + b;

    if (k > 4) {
        return 0;
    }
    if (duel_tributable(duel_atk_rec.player, q, k) == 0) {
        return 0;
    }
    if (duel_atk_rec.player != q && duel.players[q & 1].field[k].faceup == 0) {
        return 0;
    }
    id = field_get_card(q, k);
    switch (duel_atk_rec.id) {
    case 0x112E:
        if (card_same_name(id, 0x112D) != 0 && duel_slot_count_src(q, k, 0x1238) != 0) {
            return 0x800;
        }
        break;
    case 0x128C:
        if (card_same_name(id, 0xFF8) != 0 && duel_slot_count_src(q, k, 0x1238) != 0) {
            return 0x800;
        }
        break;
    case 0x1758:
        if (duel_is_cur_card(0x175E) == 0) {
            return 0;
        }
        if (duel.players[q & 1].field[k].faceup != 0 && id == 0x173B) {
            return 0x800;
        }
        break;
    case 0x176C:
        if (duel_is_cur_card(0x169F) == 0) {
            return 0;
        }
        if (duel.players[q & 1].field[k].faceup != 0 && id == 0x173B) {
            return 0x800;
        }
        break;
    case 0x17C2:
        if (card_same_name(id, 0x1122) != 0) {
            return 0x800;
        }
        break;
    case 0x17C6:
        if (duel_field_level(q, k) >= 6 && (unsigned short)duel_field_type(q, k) == 0x12) {
            return 0x800;
        }
        break;
    case 0x1894:
        if (card_same_name(id, 0xFF8) != 0) {
            return 0x800;
        }
        break;
    case 0x18B9:
        if (card_same_name(id, 0x18B8) != 0) {
            return 0x800;
        }
        break;
    case 0x18E8:
        if (card_same_name(id, 0x1707) != 0) {
            return 0x800;
        }
        break;
    case 0x1982:
        if (card_same_name(id, 0xFC9) != 0) {
            return 0x800;
        }
        break;
    case 0x198C:
        if (card_same_name(id, 0x17DB) != 0) {
            return 0x800;
        }
        break;
    case 0x19A5:
        if ((unsigned short)duel_field_type(q, k) == 3) {
            return 0x800;
        }
        break;
    case 0x19F7:
        if (card_same_name(id, 0x19F6) != 0) {
            return 0x800;
        }
        break;
    case 0x19F8:
        if (card_same_name(id, 0x19F7) != 0) {
            return 0x800;
        }
        break;
    case 0x19F0:
        if (card_is_in_group_18b2(id) != 0) {
            return 0x800;
        }
        break;
    }
    return 0;
}

int duel_slot_efb84(unsigned int p, int z);
int duel_field_same_name(unsigned int p, int z, int id);

/* Like duel_08829988, for a monster that is not already a target of the record. */
int duel_08829D74(unsigned int q, int z, int b)
{
    int k = z + b;
    int i;
    unsigned char n;

    if (k > 4) {
        return 0;
    }
    if (duel.players[q & 1].field[k].card.id == 0) {
        return 0;
    }
    if ((n = duel_atk_rec.n) == 0) {
        if (duel_count_free_monster_zones(duel_atk_rec.player) == 0) {
            if (q != duel_atk_rec.player) {
                return 0;
            }
            if (duel_zone_usable(q, k) == 0) {
                return 0;
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            if (duel_atk_rec.locs[i] == (unsigned short)((unsigned char)q | (unsigned char)k << 8)) {
                return 0;
            }
        }
    }
    return duel_08829988(q, z, b);
}

/* Own monsters for the cards 0x1895 and 0x19A6. */
int duel_08829EB8(unsigned int q, int z, int b)
{
    int k = z + b;

    if (q != duel_atk_rec.player) {
        return 0;
    }
    if (k > 4) {
        return 0;
    }
    if (duel.players[q & 1].field[k].card.id == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(duel_atk_rec.player) == 0) {
        if (q != duel_atk_rec.player || duel_zone_usable(q, k) == 0) {
            return 0;
        }
    }
    switch (duel_atk_rec.id) {
    case 0x1895:
        if (duel_field_same_name(q, k, 0x1522) != 0) {
            return 0x800;
        }
        break;
    case 0x19A6:
        if (duel_slot_efb84(q, k) == 0) {
            return 0;
        }
        if (duel_field_same_name(q, k, 0x18F9) != 0) {
            return 0x800;
        }
        break;
    }
    return 0;
}

/* Tribute for the cards 0x111A-0x111B (a chain of monsters: the n-th target must be 0x1119 + n) [?]. */
int duel_0882A018(unsigned int q, int z, int b)
{
    int k = z + b;
    int id;
    unsigned int p;

    if (k > 4) {
        return 0;
    }
    if (duel_tributable(duel_atk_rec.player, q, k) == 0) {
        return 0;
    }
    id = duel_atk_rec.n + 0x1119;
    if (id != field_get_card(q, k)) {
        return 0;
    }
    p = duel_atk_rec.player;
    if (q != p) {
        if (duel.players[q & 1].field[k].faceup == 0) {
            return 0;
        }
    }
    if (duel_count_free_monster_zones(p) == 0) {
        if (q == duel_atk_rec.player && duel_zone_usable(q, k) != 0) {
            return 0x800;
        }
        for (q = 0; (int)q < duel_atk_rec.n; q++) {
            if ((duel_atk_rec.locs[q] & 0xFF) == duel_atk_rec.player &&
                duel_zone_usable(duel_atk_rec.locs[q] & 0xFF, (unsigned char)((duel_atk_rec.locs[q] & 0xFFFF) >> 8)) != 0) {
                return 0x800;
            }
        }
        for (k = duel_atk_rec.n + 1; k < 3; k++) {
            int j;

            for (j = 0; j <= 4; j++) {
                id = k + 0x1119;
                if (id == field_get_card(duel_atk_rec.player, j) &&
                    duel_tributable(duel_atk_rec.player, duel_atk_rec.player, j) != 0 &&
                    duel_zone_usable(duel_atk_rec.player, j) != 0) {
                    return 0x800;
                }
            }
        }
        return 0;
    }
    return 0x800;
}
