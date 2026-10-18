// @range 0x088F5D54-0x088F6380
// @data 0x08911F20-0x08911F40
/* Why a card changes monster z of player p (equip, control, field and card bonuses), the table of
   these markers for every slot, and accessors for ATK/DEF. */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_view_player;

int field_get_card(unsigned int p, int z);
int duel_field_protected(unsigned int p, int z);
int duel_field_act(unsigned int player, int zone, int unused);
int duel_chain_targets(unsigned int p, int z, unsigned int player, int zone);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
void duel_field_base_stats(unsigned int p, int z, DuelStats *st);
unsigned int duel_field_bonus(DuelStats *st, unsigned int p, int z, unsigned int q, int qz);
unsigned int duel_card_bonus(DuelStats *st, unsigned int p, int z, unsigned int q, int qz);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_effect_reason(DuelStats *st, unsigned int p, int z, unsigned int q, int qz);

/* Marker for card qz of player q acting on monster z of player p (0 none). */
int duel_effect_reason(DuelStats *st, unsigned int p, int z, unsigned int q, int qz)
{
    unsigned int e;

    for (e = duel.players[p & 1].field[z].effects; e != 0;) {
        DuelEffect *ef = &duel.effects[e];
        unsigned short src = ef->src;
        unsigned short player = src & 0xFF;
        unsigned short zone = src >> 8 & 0xFF;

        e = ef->next;
        switch (ef->kind & 0xF) {
        case 10:
        case 11:
        case 12:
        case 13:
            if (q == player && qz == zone) {
                return 1;
            }
            break;
        case 6:
        case 7:
            if (q == player && qz == zone) {
                switch (field_get_card(player, zone)) {
                case 0x1280:
                case 0x128A:
                case 0x1743:
                    return 4;
                case 0xFF9:
                    return 5;
                default:
                    if (card_get_prop5_hi(duel.players[player & 1].field[zone].card.id) == 4) {
                        return 2;
                    }
                    return 6;
                }
            }
            break;
        }
    }
    if (z < 5 && duel.players[p & 1].field[z].faceup != 0 && SLOT_ACTIVE(&duel.players[q & 1].field[qz])) {
        if (qz == 10) {
            if (duel_field_bonus(st, q, qz, p, z)) {
                return 3;
            }
        } else {
            if (duel_card_bonus(st, q, qz, p, z)) {
                return 5;
            }
            if (duel_slot_e8170(q, qz, 0x1905)) {
                if (p != q || z != qz) {
                    return 5;
                }
            }
        }
    }
    return 0;
}

/* Markers of every slot for the card in zone z of player p (7: protected or the player's own set
   card, 0x100: targeted by it in the chain). */
void duel_slot_markers(unsigned int p, int z, unsigned short (*out)[11])
{
    DuelStats st;
    DuelStats st2;
    unsigned int q;
    int i;

    for (q = 0; (int)q < 2; q++) {
        for (i = 0; i <= 10; i++) {
            out[q][i] = 0;
        }
    }
    if (z <= 10 && duel.players[p & 1].field[z].card.id != 0) {
        if (z < 5 && duel_field_protected(p, z)) {
            out[p][z] = 7;
        } else if (duel.players[p & 1].field[z].faceup != 0) {
            if (duel.players[p & 1].field[z].flags >> 5 & 1) {
                out[p][z] = 7;
            }
        } else if (p == duel_view_player && z >= 5 && duel_field_act(p, z, 0)) {
            out[p][z] = 7;
        }
        duel_field_stats(p, z, &st);
        for (q = 0; (int)q < 2; q++) {
            for (i = 0; i <= 10; i++) {
                if (duel.players[q & 1].field[i].card.id != 0) {
                    duel_field_stats(q, i, &st2);
                    if (out[q][i] == 0) {
                        out[q][i] = duel_effect_reason(&st2, q, i, p, z);
                    }
                    if (out[q][i] == 0) {
                        out[q][i] = duel_effect_reason(&st, p, z, q, i);
                    }
                    if (duel_chain_targets(q, i, p, z)) {
                        out[q][i] |= 0x100;
                    }
                }
            }
        }
    }
}

int duel_field_atk(unsigned int p, int z)
{
    DuelStats st;

    duel_field_stats(p, z, &st);
    return st.atk;
}

int duel_field_base_atk(unsigned int p, int z)
{
    DuelStats st;

    duel_field_base_stats(p, z, &st);
    return st.batk;
}

int duel_field_def(unsigned int p, int z)
{
    DuelStats st;

    duel_field_stats(p, z, &st);
    return st.def;
}

int duel_field_base_def(unsigned int p, int z)
{
    DuelStats st;

    duel_field_base_stats(p, z, &st);
    return st.bdef;
}
