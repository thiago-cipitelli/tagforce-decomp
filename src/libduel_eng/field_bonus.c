// @range 0x088F1E5C-0x088F21B8
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
