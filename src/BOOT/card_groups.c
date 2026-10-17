// @range 0x0884EA38-0x08850A70
// @rodata 0x088AEB84-0x088AEBB4
// @asm 0x0884F2F8-0x0884F504
/* Hard-coded card groups: membership tests on card ids (4000..6957), special classes and alternate
   ids used by the duel engine and menus. Every list is a switch; the original author appended a few
   ids at the end of some lists, so the case order is not always ascending (it determines the code). */
#include "psp.h"
#include "game.h"
#include "card_db.h"

int card_is_in_group_198c(int id)
{
    switch (id) {
    case 0x17D1: case 0x17D2: case 0x17D3: case 0x17D4: case 0x17D5: case 0x17D6: case 0x17D7:
    case 0x17D8: case 0x17D9: case 0x17DA: case 0x17DB: case 0x1812: case 0x1814: case 0x1816:
    case 0x1817: case 0x181A: case 0x1822: case 0x1823: case 0x185E: case 0x18AF: case 0x1906:
    case 0x198C:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a6f(int id)
{
    switch (id) {
    case 0x18A6: case 0x18A7: case 0x18A8: case 0x18A9: case 0x18C8: case 0x18C9: case 0x18F9:
    case 0x1943: case 0x194D: case 0x194E: case 0x1956: case 0x1957: case 0x1958: case 0x1981:
    case 0x1987: case 0x197F: case 0x19A6: case 0x19EF: case 0x19FD: case 0x19F3: case 0x19F4:
    case 0x19FF: case 0x1A00: case 0x1A01: case 0x1A6D: case 0x1A6E: case 0x1A6F:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a6f_b(int id)
{
    switch (id) {
    case 0x19FD: case 0x1A6D: case 0x1A6E: case 0x1A6F:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a6x(int id)
{
    switch (id) {
    case 0x1A02: case 0x1A03: case 0x1A04: case 0x1A05: case 0x1A60: case 0x1A61: case 0x1A62:
    case 0x1A63: case 0x1A64: case 0x1A65:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1946(int id)
{
    switch (id) {
    case 0x166A: case 0x1670: case 0x1671: case 0x1672: case 0x1681: case 0x16B3: case 0x16B4:
    case 0x16CF: case 0x17EE: case 0x1945: case 0x1946:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a2f(int id)
{
    switch (id) {
    case 0x18C3: case 0x191C: case 0x1A2F:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1973(int id)
{
    switch (id) {
    case 0x1961: case 0x1965: case 0x1966: case 0x1967: case 0x1968: case 0x1969: case 0x196A:
    case 0x1970: case 0x1973:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_19bf(int id)
{
    switch (id) {
    case 0x1837: case 0x1913: case 0x1962: case 0x19BF:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_19bb(int id)
{
    switch (id) {
    case 0x18AB: case 0x18AC: case 0x18AD: case 0x19AA: case 0x19AD: case 0x19AE: case 0x19B2:
    case 0x19BB:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_18b2(int id)
{
    switch (id) {
    case 0x152E: case 0x17C7: case 0x17C8: case 0x17C9: case 0x18B1: case 0x18B2:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_18a4(int id)
{
    switch (id) {
    case 0x189E: case 0x189F: case 0x18A0: case 0x18A4:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_183f(int id)
{
    switch (id) {
    case 0xFE4: case 0xFE5: case 0x1128: case 0x1246: case 0x1477: case 0x181F: case 0x182A:
    case 0x182B: case 0x182C: case 0x183F:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a5c(int id)
{
    switch (id) {
    case 0x157B: case 0x18EF: case 0x18F0: case 0x18F1: case 0x18F2: case 0x18F3: case 0x18F4:
    case 0x18F5: case 0x18FA: case 0x18FB: case 0x19F4: case 0x1A50: case 0x1A52: case 0x1A53:
    case 0x1A56: case 0x1A58: case 0x1A59: case 0x1A5C:
        return 1;
    default:
        return 0;
    }
}

int card_is_1a5c_or_1a53(int id)
{
    switch (id) {
    case 0x1A53: case 0x1A5C:
        return 1;
    default:
        return 0;
    }
}

int card_is_1a6c(int id)
{
    switch (id) {
    case 0x1A6C:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a75(int id)
{
    switch (id) {
    case 0x1A70: case 0x1A74: case 0x1A75:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a90(int id)
{
    switch (id) {
    case 0x1A90: case 0x1A91: case 0x1A92: case 0x1A93: case 0x1A94: case 0x1A95:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1aad(int id)
{
    switch (id) {
    case 0x1AAA: case 0x1AAB: case 0x1AAC: case 0x1AAD:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1b2d(int id)
{
    switch (id) {
    case 0x1291: case 0x1294: case 0x1B2A: case 0x1B2B: case 0x1B2C: case 0x1B2D:
        return 1;
    default:
        return 0;
    }
}

/* Alternate ids of a card (fusion material groups): writes up to 2 ids to out, returns the count.
   Everything matches except the 0x1823 test: the original branches to the shared "return 0" block
   with a plain beq while every source form tried merges it into a beql. */
#ifdef NON_MATCHING
int card_id_remap_1a6c(int id, int *out)
{
    int tmp[2];

    if (out == 0)
        out = tmp;
    switch (id) {
    case 0x17D1: out[0] = 0x1822; return 1;
    case 0x17D2: out[0] = 0x17D3; return 1;
    case 0x17D3: out[0] = 0x17D4; return 1;
    case 0x17D4: out[0] = 0x17D3; return 1;
    case 0x17D5: out[0] = 0x17D6; return 1;
    case 0x17D6: out[0] = 0x17D5; return 1;
    case 0x17D7: out[0] = 0x17D8; return 1;
    case 0x17D8: out[0] = 0x1823; return 1;
    case 0x17D9: out[0] = 0x17DA; return 1;
    case 0x17DA: out[0] = 0x17DB; return 1;
    case 0x17DB: out[0] = 0x17DA; return 1;
    case 0x1812: out[0] = 0x1814; return 1;
    case 0x1814: out[0] = 0x1816; return 1;
    case 0x1816: out[0] = 0x1814; return 1;
    case 0x1817: out[0] = 0x181A; return 1;
    case 0x181A: out[0] = 0x1817; return 1;
    case 0x1822: out[0] = 0x17D1; out[1] = 0x185E; return 2;
    case 0x1823: break;
    case 0x185E: out[0] = 0x1822; out[1] = 0x18AF; return 2;
    case 0x18AF: out[0] = 0x185E; return 1;
    case 0x1906: return 0;
    case 0x198C: out[0] = 0x17DB; return 1;
    case 0x1A6C: out[0] = 0x1A70; return 1;
    default: break;
    }
    return 0;
}
#endif

/* 0 none, 1 or 2: hard-coded card lists (the last id of each list was appended out of order). */
int card_get_special_group(int id)
{
    switch (id) {
    case 0xFCE: case 0xFF9: case 0xFFA: case 0x100C: case 0x1051: case 0x1086: case 0x10B0:
    case 0x1152: case 0x1174: case 0x117B: case 0x1190: case 0x119B: case 0x11C2: case 0x11C3:
    case 0x11D8: case 0x11ED: case 0x11F0: case 0x11F5: case 0x1200: case 0x1209: case 0x1227:
    case 0x1280: case 0x1282: case 0x12A1: case 0x12A9: case 0x130F: case 0x1331: case 0x133B:
    case 0x1369: case 0x136A: case 0x13AB: case 0x13AD: case 0x13B0: case 0x13B4: case 0x13BD:
    case 0x1412: case 0x1413: case 0x144E: case 0x144F: case 0x1481: case 0x1489: case 0x148B:
    case 0x14C9: case 0x14CD: case 0x14CE: case 0x14EF: case 0x14FB: case 0x1525: case 0x1530:
    case 0x1585: case 0x1587: case 0x1590: case 0x1593: case 0x1595: case 0x15DD: case 0x1613:
    case 0x1618: case 0x161D: case 0x161F: case 0x1623: case 0x1647: case 0x1689: case 0x16B8:
    case 0x16BA: case 0x16C2: case 0x178E: case 0x1795: case 0x179A: case 0x17D5: case 0x17E0:
    case 0x17EA: case 0x17EC: case 0x1839: case 0x1877: case 0x189F: case 0x18B5: case 0x18BE:
    case 0x18BF: case 0x18C0: case 0x18C1: case 0x18C2: case 0x1A1B: case 0x1A29: case 0x1A2A:
    case 0x1A90:
        return 1;
    case 0x14FD: case 0x1503: case 0x1526: case 0x1527: case 0x1534: case 0x1535: case 0x1694:
    case 0x16AB: case 0x17EE:
        return 2;
    default:
        return 0;
    }
}

/* 0..5: hard-coded card classes used by the icon/group tests below. */
int card_get_special_class(int id)
{
    switch (id) {
    case 0xFDE: case 0xFDF: case 0xFE9: case 0x111C: case 0x112E: case 0x128C: case 0x138A:
    case 0x127D: case 0x127E: case 0x127F: case 0x12A5: case 0x154A: case 0x146E: case 0x13E9:
    case 0x14FC: case 0x1466: case 0x1483: case 0x1484: case 0x1485: case 0x1486: case 0x1487:
    case 0x1578: case 0x16C5: case 0x16C6: case 0x16C7: case 0x16C8: case 0x16C9: case 0x16CB:
    case 0x16E4: case 0x1534:
        return 1;
    case 0x1758: case 0x176C: case 0x17C2: case 0x17C6: case 0x1894: case 0x1895: case 0x18B4:
    case 0x18B9: case 0x18E8: case 0x1982: case 0x198C: case 0x19A3: case 0x19A4: case 0x19A5:
    case 0x19A6: case 0x19C8: case 0x19CA: case 0x19F0: case 0x1A65:
        return 2;
    case 0xFE5: case 0x1117:
        return 3;
    case 0x1643: case 0x1644: case 0x1645: case 0x167C: case 0x181A: case 0x1788: case 0x17D4:
    case 0x17DB: case 0x1816: case 0x186B: case 0x1906: case 0x1951: case 0x19A8: case 0x19A9:
    case 0x19CD: case 0x19CE: case 0x18A4:
        return 4;
    case 0x17C9:
        return 5;
    default:
        return 0;
    }
}

int card_attribute_icon_in_2_3(int id)
{
    switch (card_get_attribute_icon(id)) {
    case 0:
        return 0;
    case 3: case 2:
        return 1;
    default:
        return card_get_special_class(id) > 0;
    }
}

int card_is_group_b(int id)
{
    switch (id) {
    case 0x17C4:
        return 0;
    default:
        return !card_attribute_icon_in_2_3(id);
    }
}

int card_is_group_c(int id)
{
    switch (id) {
    case 0x15B1: case 0x15B4: case 0x15F9: case 0x15FA: case 0x1953: case 0x1954: case 0x1A6D:
    case 0x1A6E: case 0x1A6F: case 0x1299: case 0x1488: case 0x164C: case 0x1806: case 0x18F6:
    case 0x18F9: case 0x196B: case 0x196C: case 0x196D: case 0x196E: case 0x19AA: case 0x19F7:
    case 0x19F8: /* first test's delay slot holds the prologue store, so the next case gets the beql */
        return 1;
    default:
        return card_get_special_class(id) < 3;
    }
}

int card_is_in_group_1a58(int id)
{
    if (card_is_attribute7(id) != 0)
        return 0;
    switch (id) {
    case 0x16EC: case 0x1725: case 0x1729: case 0x1771: case 0x17BC: case 0x18AB: case 0x18AC:
    case 0x18E0: case 0x1905: case 0x1938: case 0x193B: case 0x1985: case 0x1986: case 0x19A8:
    case 0x19A9: case 0x19CD: case 0x19CE: case 0x1A40: case 0x1A62: case 0x1B2E: case 0x1B2F:
    case 0x157D: case 0x157E: case 0x15FC: case 0x18C8: case 0x18C9: case 0x1943: case 0x1956:
    case 0x1958: case 0x1957: case 0x1981: case 0x1987: case 0x19EF: case 0x19F3: case 0x19F4:
    case 0x19FF: case 0x1A00: case 0x1A01: case 0x1A4E: case 0x1A58:
        return 0;
    }
    switch (card_get_special_class(id)) {
    case 2: case 4:
        return 0;
    default:
        return 1;
    }
}

int card_not_in_group_1a6f(int id)
{
    switch (id) {
    case 0x15B1: case 0x15B4: case 0x15F9: case 0x15FA: case 0x1953: case 0x1954: case 0x1A6D:
    case 0x1A6E: case 0x1A6F:
        return 0;
    default:
        return 1;
    }
}

/* 1..3 from the card type (22/23 = fusion/ritual-like) and prop5_hi, else a hard-coded list. */
int card_get_type_class(int id)
{
    int type = card_get_type(id);
    int hi = card_get_prop5_hi(id);

    switch (type) {
    case 23:
        if (hi == 1)
            return 3;
        return 2;
    case 22:
        if (hi == 5)
            return 2;
        return 1;
    default:
        switch (id) {
    case 0xFE0: case 0x1119: case 0x111A: case 0x111B: case 0x148C: case 0x13A7: case 0x14B6:
    case 0x14C7: case 0x158A: case 0x1594: case 0x15FC: case 0x168F: case 0x16B9: case 0x1771:
    case 0x1774: case 0x17C2: case 0x17C6: case 0x17D4: case 0x183A: case 0x1906: case 0x1936:
    case 0x1A24: case 0x1A25: case 0x1A60: case 0x1AAC: case 0x1B2C:
            return 2;
        default:
            return 1;
        }
    }
}

int card_is_in_group_1909(int id)
{
    switch (id) {
    case 0xFF9: case 0x1009: case 0x1102: case 0x1400: case 0x1810: case 0x1AA3: case 0x149D:
    case 0x14AC: case 0x159C: case 0x17A1: case 0x187C: case 0x1A7E: case 0x1AA9: case 0x123B:
    case 0x12C8: case 0x169C: case 0x188C: case 0x130D: case 0x131A: case 0xFEE: case 0x1529:
    case 0x15A0: case 0x15EE: case 0x180F: case 0x1909:
        return 1;
    default:
        return 0;
    }
}

int card_get_group_value_1983(int id)
{
    switch (id) {
    case 0x128E: return 1;
    case 0x1610: return 3;
    case 0x1611: return 3;
    case 0x1615: return 255;
    case 0x1617: return 1;
    case 0x161A: return 3;
    case 0x1624: return 255;
    case 0x1695: return 255;
    case 0x16DE: return 255;
    case 0x186A: return 255;
    case 0x1817: return 5;
    case 0x1983: return 255;
    default: return 0;
    }
}

int card_is_in_group_1983(int id)
{
    switch (id) {
    case 0x1610: case 0x1611: case 0x1615: case 0x161A: case 0x1695: case 0x16DE: case 0x186A:
    case 0x1983:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a0c(int id)
{
    switch (id) {
    case 0x16FE: case 0x1837: case 0x18F8: case 0x1913: case 0x1962: case 0x19B2: case 0x19BF: case 0x1A8D:
        return 1;
    case 0x1A0C:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_132b(int id)
{
    switch (id) {
    case 0x10F8: case 0x10F9: case 0x10FA: case 0x10FB: case 0x10FC: case 0x12E8: case 0x10FD:
    case 0x10FE: case 0x10FF: case 0x1100: case 0x12E9: case 0x1307: case 0x1804: case 0x12CC:
    case 0x12EC: case 0x12FA: case 0x1324: case 0x1325: case 0x12CE: case 0x12F0: case 0x132B:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1a60(int id)
{
    switch (id) {
    case 0xFE0: case 0xFEE: case 0x114F: case 0x158A: case 0x1594: case 0x168F: case 0x1774:
    case 0x179C: case 0x1A24: case 0x1A25: case 0x1A60:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1517(int id)
{
    switch (id) {
    case 0x1232: case 0x149C: case 0x1A67: case 0x1517:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1872(int id)
{
    switch (id) {
    case 0x1013: case 0x10A8: case 0x10B3: case 0x1228: case 0x179C: case 0x1870: case 0x1871:
    case 0x1872:
        return 1;
    default:
        return 0;
    }
}

int card_not_in_group_1a53(int id)
{
    switch (id) {
    case 0x14B6: case 0x14C7: case 0x14DA: case 0x157E: case 0x18FB: case 0x18FC: case 0x18FD:
    case 0x1936: case 0x1955: case 0x19D6: case 0x1A53:
        return 0;
    default:
        return 1;
    }
}

int card_is_in_group_17d2(int id)
{
    switch (id) {
    case 0x1652: case 0x170A: case 0x17D2:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_147e(int id)
{
    switch (id) {
    case 0x12D3: case 0x147E:
        return 1;
    default:
        return 0;
    }
}

int card_is_1a9f(int id)
{
    switch (id) {
    case 0x1A9F:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_17ae(int id)
{
    switch (id) {
    case 0x1497: case 0x1498: case 0x1499: case 0x149A: case 0x17AD: case 0x17AE:
        return 1;
    default:
        return 0;
    }
}

/* Alternate-art id of the 0x164A..0x164F series (0 when not one of them). */
int card_id_remap_164a(int id)
{
    switch (id) {
    case 0x164A: return 0x165C;
    case 0x164B: return 0x165D;
    case 0x164C: return 0x165E;
    case 0x164D: return 0x165F;
    case 0x164E: return 0x1660;
    case 0x164F: return 0x1661;
    default: return 0;
    }
}
