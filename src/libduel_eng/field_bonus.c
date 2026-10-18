// @range 0x088F1E5C-0x088F2D70
// @asm 0x088F275C-0x088F2D70
/* ATK/DEF bonus that the card in zone z of player p gives to monster qz of player q: field spells
   by type, attribute boosters, a few named cards. */
#include "duel.h"
#include "card_db.h"

extern short duel_field_tbl[6][24];    /* 0x08911DC8 field spells 0x10F0-0x10F5 by type */

int field_get_card(unsigned int p, int z);

/* Returns the ATK bonus in the low half and the DEF bonus in the high half. */
unsigned int duel_field_bonus(DuelStats *st, unsigned int p, int z, unsigned int q, int qz)
{
    int atk = 0;
    int def = 0;
    int id = duel.players[p & 1].field[z].card.id;

    if ((int)(st->imm & ((q != p) + 1)) > 0) {
        return 0;
    }
    switch (id) {
    case 0x1323:
        if (duel.players[q & 1].field[qz].pos != 0) {
            def += 500;
        }
        break;
    case 0x10F0:
    case 0x10F1:
    case 0x10F2:
    case 0x10F3:
    case 0x10F4:
    case 0x10F5:
        if (st->attr != 0 && st->attr < 21) {
            atk += duel_field_tbl[id - 0x10F0][st->attr];
            def += duel_field_tbl[id - 0x10F0][st->attr];
        }
        break;
    case 0x183F:
        if (st->attr == 16) {
            atk += 200;
            def = atk;
        }
        break;
    case 0x1AA7:
        if (st->attr == 9) {
            atk += 300;
            def = atk;
        }
        break;
    case 0x1344:
        if (st->mask & 0x20) {
            atk += 500;
            def -= 400;
        }
        break;
    case 0x1345:
        if (st->mask & 8) {
            atk += 500;
            def -= 400;
        }
        break;
    case 0x1346:
        if (st->mask & 0x10) {
            atk += 500;
            def -= 400;
        }
        break;
    case 0x1347:
        if (st->mask & 0x40) {
            atk += 500;
            def -= 400;
        }
        break;
    case 0x1348:
        if (st->mask & 2) {
            atk += 500;
            def -= 400;
        }
        break;
    case 0x1349:
        if (st->mask & 4) {
            atk += 500;
            def -= 400;
        }
        break;
    case 0x150B:
        if (st->mask & 8) {
            atk += 200;
            def = atk;
        }
        break;
    case 0x159D:
        if (card_is_in_group_158d(field_get_card(q, qz))) {
            atk += 500;
            def = atk;
        }
        break;
    case 0x1A77:
        if (field_get_card(q, qz) == 0x19FD || card_is_in_group_1a6f_b(duel.players[q & 1].field[qz].card.id)) {
            atk += 500;
        }
        break;
    }
    return atk & 0xFFFF | (def & 0xFFFF) << 16;
}

int duel_count_type(unsigned int p, unsigned int type);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_count_same_name_other(unsigned int p, int z);

/* Same for monster effects and continuous cards. */
unsigned int duel_card_bonus(DuelStats *st, unsigned int p, int z, unsigned int q, int qz)
{
    int id = field_get_card(p, z);
    int atk = 0;
    int def = 0;
    unsigned int opp;
    int i;

    switch (id) {
    case 0x1192:
        if (st->mask & 2) {
            atk += 500;
        }
        if (st->mask & 4) {
            atk -= 400;
        }
        break;
    case 0x121A:
        if (st->mask & 4) {
            atk += 500;
        }
        if (st->mask & 2) {
            atk -= 400;
        }
        break;
    case 0x11B2:
        if (st->mask & 8) {
            atk += 500;
        }
        if (st->mask & 0x10) {
            atk -= 400;
        }
        break;
    case 0x11FC:
        if (st->mask & 0x10) {
            atk += 500;
        }
        if (st->mask & 8) {
            atk -= 400;
        }
        break;
    case 0x11B5:
        if (st->mask & 0x20) {
            atk += 500;
        }
        if (st->mask & 0x40) {
            atk -= 400;
        }
        break;
    case 0x1207:
        if (st->mask & 0x40) {
            atk += 500;
        }
        if (st->mask & 0x20) {
            atk -= 400;
        }
        break;
    case 0x13A0:
        if (st->attr == 13 && q == p && duel.players[p & 1].field[z].pos != 0) {
            atk += 500;
            def = atk;
        }
        break;
    case 0x1399:
        if (st->attr == 15 && q == p) {
            atk += 400;
        }
        break;
    case 0x1483:
        if (p != q && q == duel.turn_player && duel.phase == 3) {
            atk -= 300;
        }
        break;
    case 0x16AB:
        if (p == q && (st->mask & 8)) {
            atk += 200;
        }
        break;
    case 0x1822:
        if (p != q && duel_slot_has_effect(p, z, id)) {
            atk -= 300;
        }
        break;
    case 0x182A:
        if (st->mask & 0x40) {
            atk += 300;
        }
        break;
    case 0x185E:
        if (p != q && duel_slot_has_effect(p, z, id)) {
            atk -= 500;
        }
        break;
    case 0x18AF:
        if (p != q && duel_slot_has_effect(p, z, id)) {
            atk -= 700;
            def = atk;
        }
        break;
    case 0x191C:
        if (st->attr == 7) {
            int n[2] = { 0 };

            for (i = 0; i <= 4; i++) {
                if (field_get_card(q, i) == 0x191C && duel.players[q & 1].field[i].faceup != 0) {
                    n[duel.players[q & 1].field[i].pos]++;
                }
            }
            if (n[1] == 0) {
                atk += 500;
            }
            if (n[0] == 0) {
                def += 500;
            }
        }
        break;
    }
    opp = (q != p) + 1;
    if ((int)(st->imm & opp) <= 0) {
        switch (id) {
        case 0x13F6:
            if (st->mask & 8) {
                atk -= 500;
            }
            break;
        case 0x14CF:
            if (st->attr == 15 && q == p) {
                atk += duel_count_type(q, 15) * 200;
                atk += duel_count_type(q, 0x12) * 200;
            }
            break;
        case 0x1429:
            if (q == p) {
                def += 300;
            }
            break;
        case 0x15A2:
            if (q == p && q == duel.turn_player && duel.phase == 3) {
                atk += 200;
            }
            break;
        case 0x19B2:
            if (card_is_in_group_19bb(field_get_card(q, qz))) {
                atk += 300;
            }
            break;
        }
    }
    if ((int)(st->imm2 & opp) <= 0) {
        switch (id) {
        case 0x138E:
            if (duel_count_same_name_other(q, qz)) {
                atk += 500;
                def += 500;
            }
            break;
        }
    }
    return atk & 0xFFFF | (def & 0xFFFF) << 16;
}

int duel_slot_e1854(unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_count_zone_f_type(unsigned int p, unsigned int type);
int duel_grave_count_id(unsigned int p, unsigned int id);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));

#ifdef NON_MATCHING /* same code; the effect loop of the original keeps its index and pointer in $s1/$s0 and
   the 0x19ED case branches the other way */
/* Base ATK/DEF of monster z of player p: printed values, cards whose values depend on the duel,
   and the effects in its list that set or swap them. */
void duel_field_base_stats(unsigned int p, int z, DuelStats *st)
{
    int same = 0;
    unsigned int active = SLOT_ACTIVE(&duel.players[p & 1].field[z]);
    unsigned short e;

    st->id = duel.players[p & 1].field[z].card.id;
    st->batk = card_get_atk(st->id);
    st->bdef = card_get_def(st->id);
    if (duel.players[p & 1].field[z].faceup == 0) {
        return;
    }
    switch (st->id = field_get_card(p, z)) {
    case 0x120E:
        if (duel.players[p & 1].field[z].link != 0) {
            if (duel.players[p & 1].field[z].link < duel.turn) {
                st->batk <<= 1;
                st->bdef <<= 1;
            } else {
                st->batk = (st->batk + 1) >> 1;
                st->bdef = (st->bdef + 1) >> 1;
            }
        }
        break;
    case 0x1472:
        if (duel_field_flag9(p, z)) {
            st->batk = 1600;
            st->bdef = 1800;
        }
        break;
    case 0x14FC:
        same = 1;
        break;
    case 0x1636:
        if (duel_field_flag9(p, z)) {
            st->bdef = 3000;
        }
        break;
    case 0x172F:
        if (duel_field_flag9(p, z)) {
            st->batk = 1800;
            st->bdef = 500;
        }
        break;
    case 0x1809:
        if (duel_field_flag9(p, z)) {
            st->bdef = 2000;
        }
        break;
    case 0x1838:
        if (duel.players[p & 1].field[z].link != 0) {
            st->batk /= 2;
            st->bdef /= 2;
        }
        break;
    case 0x1864:
        if (duel.players[p & 1].field[z].link != 0) {
            st->batk = 2000;
        }
        break;
    case 0x18B0:
        if (active) {
            st->bdef = duel_count_zone_f_type(p, 6) * 700;
        }
        break;
    case 0x18C5:
        if (active) {
            st->batk = duel_grave_count_id(p, 0xFBE) * 1000 + duel_grave_count_id(p, 0x18C5) * 1000;
        }
        break;
    case 0x18B4:
    case 0x18FB:
        st->batk = duel.players[p & 1].field[z].link;
        st->bdef = duel.players[p & 1].field[z].link;
        break;
    case 0x191B:
        if (active) {
            st->batk = duel_count_zone_f_type(p, 9) * 1000;
        }
        break;
    case 0x19D2:
        if (duel.players[p & 1].field[z].link != 0) {
            int t = st->batk;

            st->batk = st->bdef;
            st->bdef = t;
        }
        break;
    case 0x1A1E:
        if (active && duel_count_monsters_fn(p, card_is_in_group_183f) >= 2) {
            st->batk <<= 1;
            st->bdef <<= 1;
        }
        break;
    case 0x1A4E:
        st->batk = duel.players[p & 1].field[z].link * 800;
        st->bdef = duel.players[p & 1].field[z].link * 800;
        break;
    case 0x19ED: {
        unsigned int r = duel_slot_e1854(p, z);

        if ((r & 0xFFFF) == 0xFFFF) {
            same = 1;
        } else {
            DuelStats t;

            duel_field_stats(r & 0xFF, (r & 0xFFFF) >> 8, &t);
            st->batk = t.atk;
            st->bdef = t.def;
            same = 1;
        }
        break;
    }
    }
    for (e = duel.players[p & 1].field[z].effects; e != 0;) {
        DuelEffect *ef = &duel.effects[e];
        int src = ef->src;
        int k;

        e = ef->next;
        k = ef->kind & 0xF;
        if (k == 5 || k == 4 || k == 2) {
            if (src == 0x1621) {
                st->batk = 200;
            } else if (src == 0x1608 || src == 0x1602) {
                st->batk = duel.effects[e].arg;
                st->bdef = duel.effects[duel.effects[e].next].arg;
                e = duel.effects[duel.effects[e].next].next;
            } else if (src == 0x14B0) {
                st->batk = ef->arg;
            } else if ((src == 0x19F1 || src == 0x12CB) && !same) {
                int t = st->batk;

                st->batk = st->bdef;
                st->bdef = t;
            }
        }
    }
}
#endif
