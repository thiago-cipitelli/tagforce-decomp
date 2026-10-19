// @range 0x088E2964-0x088E5800
// @data 0x08911D40-0x08911D88
// @asm 0x088E3920-0x088E403C
// @asm 0x088E50EC-0x088E5800
/* Which effect entries still apply: an entry placed by a card counts while the card stays on the
   field and is not negated; bit count; card lists by zone kind. */
#include "duel.h"

int field_get_card(unsigned int p, int z);
int card_get_prop5_hi(int id);
int duel_field_atk(unsigned int p, int z);
unsigned short duel_act_target(DuelAct *a, int i);
int duel_act_0884E7E8(DuelAct *a);
int duel_act_0884E898(DuelAct *a);
int duel_count_field_all(unsigned int p);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_act_0887E140(DuelAct *a, int *n);
int card_get_type(int id);
int card_is_monster(int id);
extern unsigned int duel_turn_player;

/* Entry ef is still in force (b, c: extra conditions for some cards). */
int duel_effect_live(DuelEffect *ef, int b, int c)
{
    unsigned int sp = ef->src & 0xFF;
    unsigned int sz = ef->src >> 8 & 0xFF;

    switch (ef->kind & 0xF) {
    case 10:
    case 11:
    case 12:
    case 13:
        if (duel.players[sp & 1].field[sz].card.id != 0) {
            return 1;
        }
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        switch (field_get_card(sp, sz)) {
        case 0x1103:
        case 0x137D:
        case 0x17B7:
        case 0x180F:
        case 0x1A36:
        case 0x1A7E:
        case 0x1A9B:
        case 0x1A9C:
            if (!(duel.players[sp & 1].field[sz].flags >> 5 & 1)) {
                return b;
            }
            break;
        case 0x1602:
            if (!(duel.players[sp & 1].field[sz].flags >> 5 & 1)) {
                return b && c;
            }
            break;
        case 0x1243:
        case 0x13F0:
        case 0x14B2:
        case 0x14FC:
        case 0x16A2:
        case 0x184B:
        case 0x18D3:
        case 0x19ED:
            if (!(duel.players[sp & 1].field[sz].flags >> 5 & 1)) {
                return 1;
            }
            break;
        }
        break;
    }
    return 0;
}

/* Number of set bits. */
int duel_bit_count(unsigned int x)
{
    x = ((x & 0xAAAAAAAA) >> 1) + (x & 0x55555555);
    x = ((x & 0xCCCCCCCC) >> 2) + (x & 0x33333333);
    x = ((x & 0xF0F0F0F0) >> 4) + (x & 0x0F0F0F0F);
    x = ((x & 0xFF00FF00) >> 8) + (x & 0x00FF00FF);
    return ((x & 0xFFFF0000) >> 16) + (x & 0xFFFF);
}

/* Card id can be used from zone kind zone (0xD-0x16) regardless of its type. */
int duel_card_zone_ok(int id, int zone)
{
    if (card_get_prop5_hi(id) == 1) {
        return 1;
    }
    switch (id) {
    case 0x148C:
    case 0x15FC:
    case 0x168F:
    case 0x17C2:
    case 0x17C6:
    case 0x17D4:
    case 0x183A:
    case 0x1936:
    case 0x1A24:
    case 0x1A25:
        return 1;
    }
    switch (zone) {
    case 0x14:
        switch (id) {
        case 0x1231:
        case 0x1238:
        case 0x1243:
        case 0x12CF:
        case 0x12D0:
        case 0x12F1:
        case 0x12F2:
        case 0x1329:
        case 0x132A:
        case 0x1330:
        case 0x1381:
        case 0x138E:
        case 0x1409:
        case 0x13EE:
        case 0x1492:
        case 0x14BE:
        case 0x14EB:
        case 0x1514:
        case 0x153D:
        case 0x15A6:
        case 0x167B:
        case 0x16E3:
        case 0x1763:
        case 0x184B:
        case 0x1853:
        case 0x1882:
        case 0x192D:
        case 0x19B4:
        case 0x1A60:
        case 0x1A69:
            return 1;
        }
        break;
    case 0x16:
        switch (id) {
        case 0x12BA:
        case 0x134B:
        case 0x14A7:
        case 0x18D4:
        case 0x195E:
        case 0x1990:
        case 0x19E0:
        case 0x1A17:
            return 1;
        }
        break;
    case 0xD:
        switch (id) {
        case 0x1119:
        case 0x111A:
        case 0x111B:
        case 0x13A7:
        case 0x1493:
        case 0x1853:
        case 0x1883:
            return 1;
        }
        break;
    case 0xE:
        switch (id) {
        case 0xFE0:
        case 0x151B:
            return 1;
        }
        break;
    case 0xF:
        switch (id) {
        case 0x1352:
        case 0x135A:
        case 0x138F:
        case 0x17CA:
        case 0x19B6:
            return 1;
        }
        break;
    case 0x10:
        switch (id) {
        case 0x1352:
        case 0x135A:
            return 1;
        }
        break;
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x15:
        break;
    }
    return 0;
}

int duel_card_e3070(int id)
{
    switch (id) {
    case 0x1300:
    case 0x12FD:
    case 0x1517:
    case 0x1932:
    case 0x1A38:
        return 1;
    }
    return 0;
}

/* Activation a has something to act on (by card: a target in a monster zone, an opposing
   monster in range, the right kind or step). */
int duel_act_e30c0(DuelAct *a)
{
    int i;
    int z;

    switch (a->id) {
    case 0x101E:
    case 0x10F6:
    case 0x10F7:
    case 0x1147:
    case 0x119A:
    case 0x119B:
    case 0x1233:
    case 0x1236:
    case 0x123A:
    case 0x123C:
    case 0x123D:
    case 0x123E:
    case 0x124F:
    case 0x1250:
    case 0x1254:
    case 0x12B1:
    case 0x12D7:
    case 0x12E3:
    case 0x12E4:
    case 0x12E7:
    case 0x12F7:
    case 0x12F8:
    case 0x1304:
    case 0x1305:
    case 0x1317:
    case 0x1331:
    case 0x134B:
    case 0x1363:
    case 0x1386:
    case 0x1387:
    case 0x138D:
    case 0x1398:
    case 0x13FA:
    case 0x1413:
    case 0x142E:
    case 0x1430:
    case 0x145E:
    case 0x1461:
    case 0x1475:
    case 0x147F:
    case 0x14C5:
    case 0x14CB:
    case 0x14E4:
    case 0x14F5:
    case 0x1518:
    case 0x1542:
    case 0x154B:
    case 0x156C:
    case 0x1591:
    case 0x15F3:
    case 0x15FA:
    case 0x1613:
    case 0x161C:
    case 0x164B:
    case 0x1665:
    case 0x166D:
    case 0x166F:
    case 0x1695:
    case 0x16A7:
    case 0x16C0:
    case 0x16D1:
    case 0x16D7:
    case 0x1711:
    case 0x1718:
    case 0x174D:
    case 0x1758:
    case 0x175B:
    case 0x1773:
    case 0x179E:
    case 0x179F:
    case 0x17A8:
    case 0x17DB:
    case 0x17EA:
    case 0x17F2:
    case 0x17F9:
    case 0x1868:
    case 0x186A:
    case 0x186B:
    case 0x188A:
    case 0x18BE:
    case 0x18C9:
    case 0x18D6:
    case 0x18DC:
    case 0x1906:
    case 0x192E:
    case 0x1945:
    case 0x195E:
    case 0x1970:
    case 0x1977:
    case 0x198B:
    case 0x198C:
    case 0x19A9:
    case 0x19DD:
    case 0x19F4:
    case 0x1A05:
    case 0x1A09:
    case 0x1A0E:
    case 0x1436:
    case 0x143D:
    case 0x1440:
        return 1;
    case 0x123B:
    case 0x188C:
        for (i = 0, z = 0; i <= 4; i++, z++) {
            unsigned int q = 1 - a->player;
            int hit;

            if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup != 0) {
                hit = 0;
                switch (a->id) {
                case 0x123B:
                    hit = !(duel_field_atk(q, i) < 0x5DC);
                    break;
                case 0x188C:
                    hit = duel_field_atk(q, i) < 0x5DD;
                    break;
                }
                if (hit) {
                    return 1;
                }
            }
        }
        break;
    case 0x1321:
    case 0x15EF:
    case 0x162D:
    case 0x16CF:
    case 0x1707:
    case 0x17BC:
    case 0x188D:
    case 0x1923:
    case 0x19D5:
    case 0x1AA8:
        return duel_act_0884E7E8(a) > 0;
    case 0x1388:
    case 0x14C3:
    case 0x15A8:
    case 0x15B4:
    case 0x167C:
    case 0x1708:
    case 0x185F:
    case 0x190A:
    case 0x1969:
    case 0x1A1E:
    case 0x1A35:
        for (i = 0; i < a->n_targets; i++) {
            if ((int)(duel_act_target(a, i) >> 8 & 0xFF) < 5) {
                return 1;
            }
        }
        break;
    case 0x14E0:
        return a->w[2] == 2;
    case 0x1526:
    case 0x17DA:
        return a->kind != 5;
    case 0x152A:
    case 0x179A:
    case 0x18BB:
    case 0x1A28:
        return a->b3_4 == 1;
    case 0x158A:
    case 0x162B:
    case 0x1771:
    case 0x1774:
        return a[-1].zone < 5;
    case 0x1615:
        return a->b3_4 == 3;
    case 0x183E:
        return duel_act_e30c0(a - 1);
    case 0x15FD:
    case 0x1935:
    case 0x1A74:
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* the original keeps a dead copy of the first instruction of each case body reached by beql */
/* What activation a waits for before it resolves (0: nothing, 1, 2, 4, 6: kinds of choice). */
int duel_act_e3920(DuelAct *a)
{
    int i;
    int z;

    switch (a->id) {
    case 0x12FE:
    case 0x12FF:
    case 0x140D:
    case 0x1470:
    case 0x148F:
    case 0x153E:
    case 0x1541:
    case 0x15F1:
    case 0x1634:
    case 0x16A6:
    case 0x16DD:
    case 0x1721:
    case 0x176B:
    case 0x18DD:
    case 0x18DE:
    case 0x1960:
    case 0x148C:
    case 0x14C7:
    case 0x158A:
    case 0x15FC:
    case 0x1774:
    case 0x17C6:
    case 0x17D4:
    case 0x183A:
    case 0x1A24:
    case 0x1A25:
        return 1;
    case 0x12E6:
    case 0x12EB:
    case 0x132D:
    case 0x134C:
    case 0x134D:
    case 0x1364:
    case 0x137C:
    case 0x14E1:
    case 0x0FFA:
    case 0x1086:
    case 0x117B:
    case 0x11F0:
    case 0x128E:
    case 0x152B:
    case 0x15B1:
    case 0x15D6:
    case 0x15D7:
    case 0x15F9:
    case 0x1617:
    case 0x161E:
    case 0x16B2:
    case 0x16B8:
    case 0x1753:
    case 0x17E2:
    case 0x185F:
    case 0x1962:
    case 0x19A3:
    case 0x19C8:
    case 0x19F3:
    case 0x1A9A:
        return 1;
    case 0x14C3:
    case 0x1514:
    case 0x15A8:
    case 0x1970:
    case 0x15B4:
    case 0x167C:
    case 0x1708:
    case 0x174C:
    case 0x190A:
    case 0x1969:
    case 0x1A1E:
        if ((int)(duel_act_target(a, 0) >> 8 & 0xFF) < 5) {
            break;
        }
        return 2;
    case 0x0FCE:
    case 0x11ED:
    case 0x1246:
    case 0x131B:
    case 0x131C:
    case 0x13C3:
    case 0x1406:
    case 0x14AA:
    case 0x14DF:
    case 0x1575:
    case 0x1633:
    case 0x170F:
    case 0x17A9:
    case 0x1848:
    case 0x1851:
    case 0x19B0:
    case 0x13B5:
    case 0x16C7:
    case 0x17B8:
    case 0x185C:
    case 0x18E0:
    case 0x18F8:
    case 0x1A0A:
    case 0x1A11:
        return 4;
    case 0x1321:
    case 0x15EF:
    case 0x162D:
    case 0x16CF:
    case 0x17A8:
    case 0x188D:
    case 0x1923:
    case 0x1707:
    case 0x17BC:
    case 0x19D5:
    case 0x1AA8:
        if (duel_act_0884E898(a) != 0) {
            return 6;
        }
        break;
    case 0x1A35:
        for (i = 0; i < a->n_targets; i++) {
            if (!((int)(duel_act_target(a, i) >> 8 & 0xFF) < 5)) {
                return 6;
            }
        }
        break;
    case 0x1298:
        if (a->w[2] == 1) {
            return 1;
        }
        break;
    case 0x12FD:
    case 0x1517:
        switch (a->kind) {
        case 0x1F:
        case 0x20:
        case 0x21:
            return 0;
        }
        return 1;
    case 0x14DE:
        return a->w[0] == 1;
    case 0x162B:
        z = a[-1].zone;
        if (z < 5) {
            return 0;
        }
        if (z > 10) {
            return 0;
        }
        return 1;
    case 0x1A28:
        if (a->b3_4 != 1) {
            return 0;
        }
        if (duel_count_field_all(1 - a->player) != 0) {
            return 6;
        }
        break;
    case 0x1A9F:
        if (a->b3_4 == 1) {
            return 1;
        }
        break;
    }
    return 0;
}
#endif

/* Number of choices activation a asks for. */
int duel_act_e403c(DuelAct *a)
{
    switch (a->id) {
    case 0x12CC:
    case 0x12FA:
    case 0x1769:
    case 0x1970:
    case 0x19B7:
        return 1;
    case 0x1209:
    case 0x1324:
    case 0x1325:
    case 0x1509:
    case 0x150D:
    case 0x151C:
    case 0x1571:
    case 0x176C:
    case 0x182E:
        return 2;
    case 0x11F5:
    case 0x12D5:
    case 0x13B4:
    case 0x14E8:
    case 0x15A4:
    case 0x17A8:
    case 0x1803:
        return 3;
    }
    return 0;
}

/* Which players activation a lets respond (bit 0: the opponent, bit 1: the player; 0x1256 on a side
   lets that side respond to everything). */
int duel_act_e4168(DuelAct *a)
{
    int flags = 0;
    int n[2];
    unsigned char p;
    int x;
    int r;

    if (duel_find_effect_card(a->player, 0x1256, -1)) {
        flags |= 2;
    }
    if (duel_find_effect_card(1 - a->player, 0x1256, -1)) {
        flags |= 1;
    }
    switch (a->id) {
    case 0x0FFF:
    case 0x10B0:
    case 0x10FD:
    case 0x10FE:
    case 0x10FF:
    case 0x1100:
    case 0x1190:
    case 0x11A6:
    case 0x11BC:
    case 0x129A:
    case 0x12E9:
    case 0x1301:
    case 0x1307:
    case 0x1339:
    case 0x1355:
    case 0x135A:
    case 0x135C:
    case 0x13A8:
    case 0x1404:
    case 0x144B:
    case 0x144E:
    case 0x144F:
    case 0x1460:
    case 0x1491:
    case 0x14AA:
    case 0x14F5:
    case 0x1520:
    case 0x1525:
    case 0x1565:
    case 0x1586:
    case 0x158C:
    case 0x15F4:
    case 0x161D:
    case 0x1627:
    case 0x1637:
    case 0x1641:
    case 0x16C5:
    case 0x16D7:
    case 0x16DC:
    case 0x16E4:
    case 0x1702:
    case 0x173F:
    case 0x174A:
    case 0x174B:
    case 0x1756:
    case 0x1767:
    case 0x1770:
    case 0x1794:
    case 0x179B:
    case 0x17C7:
    case 0x17C8:
    case 0x17E1:
    case 0x17F6:
    case 0x1804:
    case 0x1855:
    case 0x188B:
    case 0x18B6:
    case 0x18B7:
    case 0x18C8:
    case 0x18D6:
    case 0x18DA:
    case 0x18DC:
    case 0x1906:
    case 0x191D:
    case 0x1926:
    case 0x1929:
    case 0x192C:
    case 0x1943:
    case 0x194A:
    case 0x198B:
    case 0x19A4:
    case 0x19BB:
    case 0x19CE:
    case 0x19CF:
    case 0x19DE:
    case 0x19F0:
    case 0x193B:
    case 0x1A0D:
    case 0x1A0E:
    case 0x1AA0:
    case 0x1435:
    case 0x1B2D:
        return 2;
    case 0x10DD:
    case 0x14B2:
    case 0x1574:
    case 0x15EE:
    case 0x16CC:
    case 0x172F:
    case 0x1762:
    case 0x187B:
    case 0x18D0:
    case 0x18D1:
    case 0x19B0:
    case 0x1A51:
    case 0x1A69:
        if (a->b3_4 != 0) {
            return 2;
        }
        break;
    case 0x1984:
        if (a->b3_4 == 0) {
            return 2;
        }
        break;
    case 0x16FA:
        if (a->b3_4 == 1) {
            return 2;
        }
        break;
    case 0x11CF:
    case 0x1282:
        if (a->kind == 2) {
            return 2;
        }
        break;
    case 0x1421:
        if (a->w[0] != 0) {
            return 2;
        }
        break;
    case 0x19D3:
        if (a->kind == 0x16) {
            return 2;
        }
        break;
    case 0x14E0:
        if (a->w[2] == 1) {
            return 2;
        }
        break;
    case 0x190A:
        if (a->b3_4 == 2) {
            break;
        }
        if (card_get_type(a->idx) == 0x16) {
            return 2;
        }
        break;
    case 0x118A:
    case 0x1578:
        return 1;
    case 0x12A1:
        if (a->b3_4 != 1) {
            return 1;
        }
        break;
    case 0x169A:
        if (a->b3_4 == 1) {
            return 1;
        }
        break;
    case 0x1101:
    case 0x138D:
    case 0x1718:
    case 0x1950:
    case 0x195E:
    case 0x1A40:
        return 3;
    case 0x15A0:
        if (a->b3_4 != 0) {
            return 3;
        }
        break;
    case 0x12DC:
        return a->player != duel_turn_player ? 1 : 2;
    case 0x14E1:
        p = a->player;
        return (duel_act_target(a, 0) & 0xFF) == p ? 1 : 2;
    case 0x16DE:
        x = duel_slot_effect_arg(a->player, a->zone, a->id);
        if (a->player == x) {
            return 1;
        }
        if (a->player == 1 - x) {
            return 2;
        }
        break;
    case 0x10F8:
    case 0x13F0:
    case 0x13F3:
    case 0x141F:
    case 0x15E7:
    case 0x163F:
    case 0x1666:
    case 0x16FE:
    case 0x174E:
    case 0x1761:
    case 0x1802:
    case 0x1869:
    case 0x18D7:
    case 0x1914:
    case 0x1AAD:
        r = 0;
        duel_act_0887E140(a, n);
        p = a->player;
        if (n[p] < 0) {
            r |= 1;
        }
        if (n[1 - p] < 0) {
            r |= 2;
        }
        if (n[p] > 0) {
            r |= flags & 1;
        }
        if (n[1 - p] > 0) {
            r |= flags & 2;
        }
        return r;
    case 0x104C:
    case 0x10F9:
    case 0x10FA:
    case 0x10FB:
    case 0x10FC:
    case 0x12A2:
    case 0x12E8:
    case 0x133A:
    case 0x1352:
    case 0x137A:
    case 0x1380:
    case 0x138F:
    case 0x1450:
    case 0x1451:
    case 0x1454:
    case 0x1455:
    case 0x1459:
    case 0x14E6:
    case 0x14F3:
    case 0x151B:
    case 0x1635:
    case 0x1665:
    case 0x16A0:
    case 0x170B:
    case 0x170F:
    case 0x1757:
    case 0x175A:
    case 0x176A:
    case 0x1792:
    case 0x1793:
    case 0x1987:
    case 0x19CD:
    case 0x19E0:
    case 0x1A14:
    case 0x1A23:
    case 0x1AA2:
        return flags & 1;
    case 0x1174:
        if (a->b3_4 == 1) {
            return flags & 1;
        }
        break;
    case 0x1370:
        if (a->kind == 2) {
            return flags & 1;
        }
        break;
    case 0x137B:
    case 0x1405:
    case 0x142C:
    case 0x1467:
    case 0x14C0:
    case 0x14ED:
    case 0x16DA:
    case 0x17A4:
    case 0x17A5:
        if (a->b3_4 != 0) {
            return flags & 1;
        }
        break;
    case 0x1506:
        if (a->kind != 5) {
            return flags & 1;
        }
        break;
    case 0x11C9:
    case 0x131F:
    case 0x166F:
    case 0x16F5:
        return flags & 2;
    case 0x1322:
        if (a->b3_4 != 0) {
            return flags & 2;
        }
        break;
    case 0x1877:
        if (a->kind == 2) {
            return flags & 2;
        }
        break;
    case 0x143E:
        return a->b3_4 == 0 ? flags & 3 : 2;
    }
    return 0;
}

/* Number of cards card id takes (0: none). */
int duel_card_e4de8(int id)
{
    switch (id) {
    case 0x12CC:
    case 0x12EC:
    case 0x131F:
    case 0x1420:
    case 0x145A:
    case 0x14E2:
    case 0x1548:
    case 0x1567:
    case 0x161A:
    case 0x162A:
    case 0x16C1:
    case 0x16D2:
    case 0x16D9:
    case 0x1748:
    case 0x1774:
    case 0x1776:
    case 0x1790:
    case 0x1888:
    case 0x196F:
    case 0x198D:
    case 0x19B7:
    case 0x1A5B:
        return 1;
    case 0x15AA:
    case 0x17F3:
    case 0x1801:
    case 0x1944:
        return 2;
    case 0x12D5:
    case 0x14E8:
        return 3;
    case 0x1403:
    case 0x16CD:
    case 0x17A5:
    case 0x1841:
    case 0x1847:
    case 0x184E:
    case 0x18F9:
    case 0x191A:
        break;
    }
    return 0;
}

int duel_card_e4fd0(int id)
{
    switch (id) {
    case 0x1296:
    case 0x1302:
    case 0x14C7:
    case 0x14D5:
    case 0x14DA:
    case 0x14DE:
    case 0x1517:
    case 0x1529:
    case 0x1534:
    case 0x159D:
    case 0x172E:
    case 0x1760:
    case 0x17B9:
    case 0x17C2:
    case 0x17C6:
    case 0x183A:
    case 0x1936:
    case 0x1955:
    case 0x19C6:
    case 0x1A07:
    case 0x1A1D:
    case 0x1A24:
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* the target is masked to 0xFF before 0xFFFF in the original */
/* Which sides the cards activation a sends to the graveyard belong to (bit 0: its player, bit 1: the
   opponent), or a fixed answer by card. */
int duel_act_e50ec(DuelAct *a)
{
    int r = 0;
    int i;

    for (i = 0; i < a->n_targets; i++) {
        unsigned short t = duel_act_target(a, i);
        unsigned int tp = t & 0xFF;

        if ((t >> 8 & 0xFF) == 0xE) {
            r |= (a->player != tp) + 1;
        }
    }
    switch (a->id) {
    case 0x100C:
    case 0x106D:
    case 0x1138:
    case 0x1152:
    case 0x1294:
    case 0x1359:
    case 0x1366:
    case 0x137D:
    case 0x138A:
    case 0x13F5:
    case 0x1446:
    case 0x148B:
    case 0x14A7:
    case 0x14C1:
    case 0x14D2:
    case 0x14EA:
    case 0x14EF:
    case 0x14F6:
    case 0x14F7:
    case 0x156A:
    case 0x156D:
    case 0x1576:
    case 0x158B:
    case 0x1616:
    case 0x1630:
    case 0x1631:
    case 0x1638:
    case 0x164A:
    case 0x164F:
    case 0x1664:
    case 0x168F:
    case 0x16A4:
    case 0x16A8:
    case 0x16BC:
    case 0x16D5:
    case 0x16F8:
    case 0x1714:
    case 0x1745:
    case 0x1769:
    case 0x1788:
    case 0x178A:
    case 0x178C:
    case 0x1795:
    case 0x17A2:
    case 0x17B7:
    case 0x17F1:
    case 0x17F7:
    case 0x1820:
    case 0x183D:
    case 0x1845:
    case 0x1864:
    case 0x187A:
    case 0x1880:
    case 0x1881:
    case 0x1895:
    case 0x18A4:
    case 0x18CC:
    case 0x191F:
    case 0x1927:
    case 0x1938:
    case 0x1944:
    case 0x1951:
    case 0x196F:
    case 0x1971:
    case 0x1973:
    case 0x1974:
    case 0x1979:
    case 0x19D7:
    case 0x1A03:
    case 0x1A12:
    case 0x1A42:
    case 0x1A58:
    case 0x1A59:
    case 0x1A64:
    case 0x1A6B:
    case 0x1A9B:
        return 1;
    case 0x1379:
    case 0x143D:
    case 0x1488:
    case 0x14E7:
    case 0x15E6:
    case 0x15FD:
    case 0x19F4:
        return 2;
    case 0x1365:
    case 0x1536:
    case 0x14F0:
    case 0x14FB:
    case 0x1990:
        return 3;
    case 0x12EA:
        break;
    case 0x13FE:
    case 0x1445:
    case 0x1610:
    case 0x1611:
    case 0x1629:
    case 0x167D:
    case 0x1713:
    case 0x195C:
    case 0x198E:
    case 0x19B1:
    case 0x1A05:
        return 0;
    case 0x12A6:
    case 0x1415:
    case 0x158F:
    case 0x1592:
    case 0x1791:
    case 0x1A95:
        return 0;
    case 0x12BA:
        r |= (a->b15_1 != a->player) + 1;
        break;
    case 0x133B:
    case 0x1449:
    case 0x144C:
    case 0x1452:
    case 0x1457:
    case 0x1549:
    case 0x179A:
    case 0x1AA9:
        r |= a->b3_4 == 2;
        break;
    case 0x1534:
    case 0x187F:
    case 0x197C:
        r |= a->b3_4 == 1;
        break;
    case 0x1686:
        r |= a->w[2] == 2;
        break;
    case 0x190A:
        if (a->b3_4 == 2) {
            break;
        }
        if (card_is_monster(a->idx)) {
            return 1;
        }
        break;
    }
    return r;
}
#endif
