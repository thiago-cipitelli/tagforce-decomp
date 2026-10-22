// @range 0x08853CE8-0x08854D4C
/* Computer player: how the action a would treat the monster in zone z of p, by card id
   (1: good for the acting player, -1: bad, 0: not a target). */
#include "duel.h"

unsigned int duel_field_type_ex(unsigned int p, int z, int skip);
int field_get_card(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_field_e5ae0(unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_slot_count_src(unsigned int p, int z, int id);
int card_same_name(int id, int id2);
int card_is_in_group_18be(int id);
int card_is_in_group_1a6x(int id);
int card_get_attribute_icon(int id);
int duel_08853C80(DuelAct *a, unsigned int p, int z);

int duel_act_check_08853CE8(DuelAct *a, unsigned int p, int z)
{
    DuelSlot *s = &duel.players[p & 1].field[z];
    unsigned char k = a->zone;
    int id = s->card.id;
    unsigned int t;
    int card;

    t = (unsigned short)duel_field_type_ex(p, z, k > 10 ? 0 : duel.players[a->player & 1].field[k].unk4);
    if (z >= 5) {
        return 0;
    }
    if (s->card.id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    card = field_get_card(p, z);
    switch (a->id) {
    case 0x1231:
    case 0x1514:
        return a->player == p ? 1 : -1;
    case 0x1318:
        return a->player == p;
    case 0x1322:
    case 0x1466:
    case 0x169A:
    case 0x1877:
        return a->player != p && duel_slot_ef098(p, z, a->id) != 0 ? 1 : -1;
    case 0x18D1:
        return a->player != p;
    case 0xFEE:
        if (card != 0x10BC) {
            return 0;
        }
        if (a->player != p) {
            return -1;
        }
        return duel_slot_count_src(p, z, 0xFEE) != 0 ? -1 : 1;
    case 0x10E3:
        if (card_same_name(card, 0xFE4) != 0 || card == 0xFE5) {
            return 1;
        }
        return 0;
    case 0x1232:
        return card == 0x1114;
    case 0x12D3:
        return card == 0x1296;
    case 0x1467:
        switch (card) {
        case 0xFC9:
        case 0x142D:
        case 0x129E:
            return 1;
        }
        return 0;
    case 0x1496:
        return card == 0x1414;
    case 0x150A:
        return a->b3_4 == 1 ? duel_field_atk(p, z) < 1300 : 1;
    case 0x159E:
        return duel_field_atk(p, z) < 1001;
    case 0x16CC:
        return card_is_in_group_18be(card);
    case 0x1759:
        return duel_field_level(p, z) == 1;
    case 0x17A0:
        if (a->player != p) {
            return 0;
        }
        if (duel_field_e5ae0(p, z) == 0) {
            return 0;
        }
        return duel_field_level(p, z) < 4 ? 1 : 0;
    case 0x17FA:
        if (duel_field_level(p, z) > 6) {
            return 0;
        }
        return card_get_attribute_icon(id) == 3 ? 1 : 0;
    case 0x17FB:
        if (duel_field_level(p, z) > 6) {
            return 0;
        }
        return card_get_attribute_icon(id) == 2 ? 1 : 0;
    case 0x18D0:
        switch (card) {
        case 0x185A:
        case 0x18B8:
        case 0x18B9:
            return a->player != p ? -1 : 1;
        }
        return 0;
    case 0x1909:
        return card == 0x18A9;
    case 0x1980:
        return t == 0xF && duel_field_atk(p, z) < 1501;
    case 0x1989:
        return duel_field_same_name(p, z, 0x18F9);
    case 0x19B0:
        return card == 0x194E;
    case 0x19D8:
        switch (card) {
        case 0x1757:
        case 0x191D:
            return 1;
        }
        return 0;
    case 0x1A69:
        if (card_is_in_group_1a6x(card) == 0) {
            return 0;
        }
        return a->player != p ? -1 : 1;
    case 0x1A9E:
        return card_is_in_group_1a6x(card);
    case 0x15B0:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x1580) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x15B3:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x1580 && card != 0x15B0) {
            return -(card == 0x15D8);
        }
        if (a->player != p) {
            return -1;
        }
        return 1;
    case 0x15CE:
    case 0x15CF:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x15CD) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x15D1:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x15D0) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x15D3:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x15D2) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x15D5:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x15D4) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x15D6:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x15D7) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x15D7:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x15D6) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x1620:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        return card != 0x160A ? -(card == 0x15D8) : 1;
    case 0x1917:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x190B) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x1693:
        if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        return a->player != p ? -1 : 1;
    case 0x1825:
        if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (t != 7) {
            return 0;
        }
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        return a->player != p ? -1 : 1;
    case 0x1916:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x1915) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x194B:
        if (duel_08853C80(a, p, z) == 0) {
            return 0;
        }
        if (card != 0x1947) {
            return -(card == 0x15D8);
        }
        return a->player != p ? -1 : 1;
    case 0x10D7:
    case 0x10D8:
        return t == 0xA;
    case 0x10EB:
    case 0x12C6:
        return t == 7;
    case 0x10DE:
    case 0x14ED:
        return t == 0x11;
    case 0x10EC:
        return t == 9;
    case 0x10EE:
        return t == 0xE;
    case 0x10E2:
        return t == 0x13;
    case 0x10D3:
    case 0x1392:
    case 0x13F6:
    case 0x149F:
    case 0x150C:
    case 0x193A:
        return t == 0xF;
    case 0x10E9:
        return t == 0x12;
    case 0x10E1:
        return t == 1;
    case 0x10ED:
        return t == 0x10;
    case 0x10DC:
        return t == 0xD;
    case 0x10E5:
        return t == 0xC;
    case 0x10E8:
        return t == 2;
    case 0x10DA:
        return t == 0xB;
    case 0x10D5:
        return t == 3;
    case 0x130D:
    case 0x130E:
        return t != 7;
    case 0x10EA:
        return duel_field_attr_is(p, z, 5);
    case 0x10DB:
        return duel_field_attr_is(p, z, 3);
    case 0x1234:
    case 0x12EE:
        return duel_field_attr_is(p, z, 4);
    case 0x1242:
    case 0x10D9:
        return duel_field_attr_is(p, z, 1);
    case 0x12EF:
        return duel_field_attr_is(p, z, 6);
    case 0x10D4:
        return duel_field_attr_is(p, z, 2);
    case 0x1366:
    case 0x15E6:
    case 0x1880:
    case 0x1881:
    case 0x19D7:
    case 0x1286:
    case 0x1370:
    case 0x19BD:
    case 0x1A18:
        return -1;
    }
    return 1;
}
