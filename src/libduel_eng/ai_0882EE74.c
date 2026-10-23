// @range 0x0882EE74-0x0882FF4C
// @asm 0x0882F320-0x0882F6BC
/* Summoning a monster (duel_summon), end: summons that send cards of the field (0x19A3, 0x19A4,
   0x19C8) and duel_summon_run, which picks the step machine of the card being summoned. */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_w_08919624;            /* answer of the last question */
extern int duel_w_0891964C;            /* picked player */
extern int duel_w_08919650;            /* picked zone */
extern int duel_w_08919654;
extern unsigned int duel_turn_player;
extern unsigned int duel_w_089147E4;   /* duel_chain.uids_head */
extern unsigned short duel_w_089147E8[]; /* duel_chain.uids */
extern int duel_w_089149F0;            /* count of duel_chain.ids */
extern unsigned int duel_w_089149F8;   /* materials picked */

int duel_08835AC0(void);
int duel_host_call(int a, int b, unsigned int c, int d);
int duel_0884AD18(int msg);
void duel_wfmt_d();
void duel_wfmt_s();
void duel_0881956C(unsigned int p, int id, int (*fn)());
void duel_08835978(int (*fn)());
void duel_088359B0(int (*fn)());
int duel_08827DA4(unsigned int p, int uid, int a, int b);
void duel_0884AD40(int msg, int uid);
int duel_0884ACD8(unsigned int p, int msg);
int duel_summon_tributes(void);
int card_get_type(int id);
int card_get_kind(int id);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
int duel_088D1608(int p, int z, int flags);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884CCBC(int i);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_08827544(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
unsigned short *card_db_table_sel(int i);
unsigned short *dlg_table_sel(int i);
int duel_0882A550(unsigned int p, int z, int i);
int duel_0882A838(unsigned int p, int z, int i);
int duel_0882A940(unsigned int p, int z, int i);
int duel_0882E0A8(unsigned int p, int z, int i);
int field_get_card(unsigned int p, int z);
int duel_field_protected(unsigned int p, int z);
int duel_count_free_monster_zones(unsigned int p);
int duel_zone_usable(unsigned int p, unsigned int z);
int card_is_in_group_1a6f_b(int id);
unsigned int duel_card_locate(unsigned int uid);
void duel_088D19C0(unsigned int p, int z, int a, int b);
void duel_088C78E0(unsigned int p);
int duel_count_active(int id);
int duel_count_tributes();
int duel_0886AAB4(DuelAct *a, int b);
int duel_slot_efb84(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
void duel_088D1BEC(unsigned int p, int z, int a, int b);
int duel_0882EA4C(unsigned int p, int z, int i);

unsigned int duel_field_flag9(unsigned int p, int z);
int card_get_prop5_hi(int id);
int duel_count_free_zones(unsigned int p);
int duel_0882F174(unsigned int p, int z, int i);
void duel_088D1D58(DuelAct *a, unsigned int mask);
int card_is_in_group_1a6f(int id);
int card_is_in_group_19bb(int id);
int duel_pick_monster_zone(unsigned int p);
void duel_088263B4(unsigned int p, int uid, int z, int kind, int n);
void duel_08825DE0(unsigned int p, int uid, int z, int n, int face);
void duel_0884F91C(unsigned int a, unsigned short b);
int duel_0882B8D8(void);
int duel_0882BBB4(void);
int duel_0882BF2C(void);
int duel_0882C0C0(void);
int duel_0882C270(void);
int duel_0882C400(void);
int duel_0882C62C(void);
int duel_0882C7B8(void);
int duel_0882C8C0(void);
int duel_0882CA98(void);
int duel_0882CAC0(void);
int duel_0882CD40(void);
int duel_0882D068(void);
int duel_0882D0D0(void);
int duel_0882D130(void);
int duel_0882D180(void);
int duel_0882D2A0(void);
int duel_0882D398(void);
int duel_0882D850(void);
int duel_0882D9E4(void);
int duel_0882DB98(void);
int duel_0882DE18(void);
int duel_0882E1B8(void);
int duel_0882E84C(void);
int duel_0882EC88(void);
int duel_0882F320(void);

/* Zone filter (0x19A3): own face-up card of type 0x17 that may be sent. */
int duel_0882EE74(unsigned int p, int z, int i)
{
    int zz = z + i;
    int id;

    if (p != duel_summon.player) {
        return 0;
    }
    if (zz > 0xA) {
        return 0;
    }
    id = duel.players[p & 1].field[zz].card.id;
    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[zz].faceup == 0) {
        return 0;
    }
    if (card_get_type(id) != 0x17) {
        return 0;
    }
    if ((zz > 4 || duel_field_flag9(p, zz) != 0) && duel_slot_efb84(p, zz) != 0) {
        return 0x800;
    }
    return 0;
}

/* Zone filter (0x19A4): own face-up spell/trap of type 0x16 (prop 4) that may be sent. */
int duel_0882EF84(unsigned int p, int z, int i)
{
    int id;
    int zz = z + i;

    if (p != duel_summon.player) {
        return 0;
    }
    if (zz < 5) {
        return 0;
    }
    if (zz > 0xA) {
        return 0;
    }
    id = duel.players[p & 1].field[zz].card.id;
    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[zz].faceup == 0) {
        return 0;
    }
    if (card_get_prop5_hi(id) != 4) {
        return 0;
    }
    if (card_get_type(id) != 0x16) {
        return 0;
    }
    if (duel_slot_efb84(p, zz) != 0) {
        return 0x800;
    }
    return 0;
}

/* Zone filter (0x19C8): any own card that may be sent. */
int duel_0882F0A4(unsigned int p, int z, int i)
{
    int zz = z + i;

    if (p != duel_summon.player) {
        return 0;
    }
    if (zz > 0xA) {
        return 0;
    }
    if (duel.players[p & 1].field[zz].card.id == 0) {
        return 0;
    }
    if ((zz > 4 || duel_field_flag9(p, zz) != 0) && duel_slot_efb84(p, zz) != 0) {
        return 0x800;
    }
    return 0;
}

/* Zone filter of duel_0882F320: a card not picked yet, by the filter of the card summoned. */
int duel_0882F174(unsigned int p, int z, int i)
{
    int zz = z + i;
    int k;

    if (zz > 0xA) {
        return 0;
    }
    if (duel.players[p & 1].field[zz].card.id == 0) {
        return 0;
    }
    if (duel_summon.n == 0) {
        if (duel_count_free_zones(p) == 0) {
            if (p != duel_summon.player) {
                return 0;
            }
            if (zz > 4) {
                return 0;
            }
            if (duel_zone_usable(p, zz) == 0) {
                return 0;
            }
        }
    } else {
        for (k = 0; k < (duel_summon.n & 0xFF); k++) {
            if (duel_summon.locs[k] == (unsigned short)((unsigned char)p | (unsigned char)zz << 8)) {
                return 0;
            }
        }
    }
    switch (duel_summon.id) {
    case 0x19A3:
        return duel_0882EE74(p, z, i);
    case 0x19A4:
        return duel_0882EF84(p, z, i);
    case 0x19C8:
        return duel_0882F0A4(p, z, i);
    }
    return 0;
}

#ifdef NON_MATCHING /* o original relê duel_summon.n depois de gravar a zona e calcula o máximo em v1 */
/* Summon that sends two (0x19C8) or three own cards of the field picked with duel_0882F174. */
int duel_0882F320(void)
{
    int k;
    int max;

    switch (duel_summon.step) {
    case 0:
        duel_summon.n = 0;
        duel_0884ACD8(duel_summon.player, 0x3A);
    case 2:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882F174);
        } else {
            duel_08835978(duel_0882F174);
        }
        duel_summon.step = 1;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_summon.locs[duel_summon.n++] = (unsigned char)duel_w_0891964C | (unsigned char)(duel_w_08919650 + duel_w_08919654) << 8;
            max = duel_summon.id != 0x19C8 ? 3 : 2;
            if (duel_summon.n >= max) {
                duel_summon.step = 0xA;
            } else {
                duel_summon.step++;
            }
            return 0;
        }
        return -1;
    case 0xA: {
        unsigned int mask = 0;
        DuelAct a = { 0 };

        a.player = duel_summon.player;
        for (k = 0; k < duel_summon.n; k++) {
            mask |= 1 << ((unsigned char)(duel_summon.locs[k] >> 8) + (duel_summon.locs[k] & 0xFF) * 16);
        }
        duel_088D1D58(&a, mask);
        duel_summon.n = 0;
        duel_summon.step++;
        return 0;
    }
    case 0xB:
        return 1;
    }
    return 0;
}

#endif

/* Runs one step of the summon of duel_summon: the step machine of the card (or the tributes of a
   normal summon), then puts the monster on the field when it is done. Returns 1 when finished. */
int duel_summon_run(void)
{
    int r;
    int k;
    unsigned int q;
    int z;
    int any;
    unsigned int id = duel_summon.id;

    switch (id) {
    case 0xFDE:
    case 0xFDF:
    case 0xFE9:
        r = duel_0882C0C0();
        break;
    case 0x111C:
        r = duel_0882C8C0();
        break;
    case 0x112E:
    case 0x128C:
    case 0x1758:
    case 0x176C:
    case 0x17C2:
    case 0x1894:
    case 0x18B9:
    case 0x18E8:
    case 0x1982:
    case 0x198C:
    case 0x19F7:
    case 0x19F8:
    case 0x19F0:
        r = duel_0882C270();
        break;
    case 0x1299:
    case 0x164C:
    case 0x18F6:
    case 0x18F9:
        r = duel_0882D068();
        break;
    case 0x19AA:
        r = duel_0882D0D0();
        break;
    case 0x138A:
        r = duel_0882CAC0();
        break;
    case 0x13E9:
        r = duel_0882CD40();
        break;
    case 0x1466:
    case 0x1483:
    case 0x1484:
    case 0x1485:
    case 0x1486:
    case 0x1487:
    case 0x16C5:
    case 0x16C6:
    case 0x16C7:
    case 0x16C8:
    case 0x16C9:
    case 0x16CB:
    case 0x16E4:
    case 0x18B4:
    case 0x19CA:
        r = duel_0882D398();
        break;
    case 0x147B:
    case 0x1838:
    case 0x1A20:
        r = duel_0882D850();
        break;
    case 0x1864:
    case 0x1A21:
    case 0x1A96:
    case 0x1B2D:
        r = duel_0882D9E4();
        break;
    case 0x1578:
        r = duel_0882DB98();
        break;
    case 0x1A92:
        r = duel_0882DE18();
        break;
    case 0x15B1:
    case 0x15B4:
    case 0x15F9:
    case 0x15FA:
    case 0x1953:
    case 0x1954:
    case 0x1A6D:
    case 0x1A6E:
    case 0x1A6F:
        r = duel_0882E1B8();
        break;
    case 0x13B5:
    case 0x154B:
        r = duel_0882E84C();
        break;
    case 0x17C6:
    case 0x19A5:
    case 0x1A65:
        r = duel_0882C400();
        break;
    case 0x17C7:
    case 0x17C8:
        r = duel_0882D130();
        break;
    case 0x19A6:
        r = duel_0882C62C();
        break;
    case 0x1895:
        r = duel_0882C7B8();
        break;
    case 0x1806:
        r = duel_0882D180();
        break;
    case 0x194C:
        r = duel_0882D2A0();
        break;
    case 0x196B:
    case 0x196C:
    case 0x196D:
    case 0x196E:
        r = duel_0882EC88();
        break;
    case 0x19A3:
    case 0x19A4:
    case 0x19C8:
        r = duel_0882F320();
        break;
    case 0x1386:
    case 0x1387:
    case 0x1388:
    case 0x1905:
    case 0x16EC:
    case 0x1725:
    case 0x1729:
    case 0x1985:
    case 0x1986:
    case 0x1B2E:
    case 0x1B2F:
        r = duel_0882CA98();
        break;
    default:
        if (card_is_in_group_1a6f(id) != 0) {
            r = duel_0882B8D8();
        } else if (card_is_in_group_19bb(duel_summon.id) != 0) {
            r = duel_0882BBB4();
        } else if (card_get_type(duel_summon.id) == 9) {
            r = duel_0882BF2C();
        } else {
            r = duel_summon_tributes();
        }
        break;
    }
    if (r < 0) {
        duel_summon.f3 = 1;
        duel_host_call(0x29, 0, 0, 0);
        return 1;
    }
    if (r == 1) {
        any = 1;
        if (duel_summon.res_id != 0x1578 && duel_summon.res_id != 0x1A92) {
            any = 0;
        }
        q = duel_summon.player ^ any;
        z = duel_pick_monster_zone(q);
        for (k = 0; k < duel_summon.n; k++) {
            if ((duel_summon.locs[k] & 0xFF) == q && duel_zone_usable(q, (unsigned char)((duel_summon.locs[k] & 0xFFFF) >> 8))) {
                z = (unsigned char)(duel_summon.locs[k] >> 8);
                break;
            }
        }
        if (z < 0) {
            duel_host_call(0x29, 0, 0, 0);
        } else if (duel_summon.f1) {
            duel_088263B4(duel_summon.player, duel_summon.uid, z, duel_summon.kind, duel_summon.n);
        } else {
            duel_08825DE0(duel_summon.player, duel_summon.uid, z, duel_summon.n, !duel_summon.kind);
        }
        if (duel_summon.id == 0x1488) {
            if (duel_summon.f1) {
                z = duel_pick_monster_zone(duel_summon.player);
                z = z << 16 & 0x1F0000;
                duel_0884F91C((unsigned short)duel_summon.id | ((duel_summon.player << 31 & 0x80000000 | 0x200000) | z), duel_summon.uid);
            }
        }
        return 1;
    }
    return 0;
}
