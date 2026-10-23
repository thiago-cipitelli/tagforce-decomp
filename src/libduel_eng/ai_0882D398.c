// @range 0x0882D398-0x0882EE74
// @asm 0x0882E0A8-0x0882E1B8
// @asm 0x0882E1B8-0x0882E84C
// @asm 0x0882EA4C-0x0882EC88
// @data 0x08907974-0x089079C4
/* Summoning a monster (duel_summon), continued: summons that ask for monsters of an attribute
   (0x14A0 makes them come from the hand), summons that may skip the tributes, and summons that send
   named materials. */
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

/* Name of the attribute (or type) asked by the card being summoned. */
#define SUMMON_ATTR_NAME(name)                                                   \
    switch (duel_summon.id) {                                                    \
    case 0x1466:                                                                 \
    case 0x18B4:                                                                 \
    case 0x19CA:                                                                 \
        name = card_db_table_sel(card_get_type(duel_summon.id));                 \
        break;                                                                   \
    case 0x16C9:                                                                 \
    case 0x16CB:                                                                 \
    case 0x16E4:                                                                 \
        name = dlg_table_sel(duel_summon.step3 ? 2 : 1);                         \
        break;                                                                   \
    default:                                                                     \
        name = dlg_table_sel(card_get_kind(duel_summon.id));                     \
        break;                                                                   \
    }

/* Summon that sends monsters of an attribute: step2 counts them, then the player picks them one by
   one with duel_0882A550 (or from the hand with 0x14A0). */
int duel_0882D398(void)
{
    unsigned short msg[256];
    unsigned short tmp[256];
    unsigned short *name;
    int n;

    switch (duel_summon.step) {
    case 0:
        duel_088C7BA4();
    case 4:
        n = duel_08827544(duel_summon.id);
        SUMMON_ATTR_NAME(name)
        duel_summon.step2 = n;
        if (duel_slot_has_effect(duel_summon.player, 0xB, 0x14A0) != 0) {
            if (n >= 2) {
                duel_wfmt_s(tmp, 0xD6, name);
                duel_wfmt_d(msg, tmp, n);
            } else {
                duel_wfmt_s(msg, 0xD8, name);
            }
            duel_summon.step = 5;
        } else {
            if (n >= 2) {
                duel_wfmt_s(tmp, 0xD7, name);
                duel_wfmt_d(msg, tmp, n);
            } else {
                duel_wfmt_s(msg, 0xD9, name);
            }
            duel_summon.step = 1;
        }
        if (duel_player_type[duel_summon.player] == 0) {
            duel_0884ACD8(duel_summon.player, (int)msg);
        }
        return 0;
    case 1:
        switch (duel_summon.id) {
        case 0x18B4:
            duel_0884C588(duel_summon.player, 0x47, duel_summon.id, duel_summon.step3);
            break;
        default:
            duel_0884C588(duel_summon.player, MIN(duel_summon.step2, 0x20) + 6, duel_summon.id, duel_summon.step3);
            break;
        }
        duel_summon.step++;
        duel_summon.step2 = 0;
        return 0;
    case 2:
        if (++duel_summon.step2 <= (unsigned int)duel_w_08919624) {
            duel_088CB400(duel_summon.player, duel_0884CCBC(duel_summon.step2), 0);
            return 0;
        }
        duel_summon.step++;
        return 0;
    case 3:
        switch (duel_summon.id) {
        case 0x16C9:
        case 0x16CB:
        case 0x16E4:
            if (duel_summon.step3++ == 0) {
                duel_summon.step = 4;
                return 0;
            }
            break;
        case 0x18B4:
            duel_summon.locs[0] = duel_w_08919624;
            break;
        }
        duel_088C7CF0();
        duel_summon.n = 0;
        return 1;
    case 5:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882A550);
        } else {
            duel_088359B0(duel_0882A550);
        }
        duel_summon.step++;
        return 0;
    case 6:
        if (duel_08835AC0() != 0) {
            duel_088D1608(duel_w_0891964C, duel_w_08919650 + duel_w_08919654, 0);
            if (--duel_summon.step2 == 0) {
                duel_summon.step = 3;
                return 0;
            }
        }
        duel_summon.step--;
        return 0;
    }
    return 0;
}

/* Summon that may skip the tributes (asks first when an effect allows it). */
int duel_0882D850(void)
{
    int mask;

    switch (duel_summon.step) {
    case 0:
        mask = duel_summon.kind ? 0x40 : 0x10;
        if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 2, duel_summon.f4)) > 0) {
            if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 0, duel_summon.f4)) > 0) {
                if (duel_player_type[duel_summon.player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    duel_0884AD40(0x72, duel_summon.uid);
                }
            } else {
                duel_w_08919624 = 1;
            }
        } else {
            duel_w_08919624 = 0;
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_summon.n = 0;
            duel_summon.res_id = duel_summon.id;
            return 1;
        }
        duel_summon.step = 0;
        break;
    }
    return duel_summon_tributes();
}

/* Summon that may skip the tributes, then goes on at step 0xB of duel_summon_tributes. */
int duel_0882D9E4(void)
{
    int mask;

    switch (duel_summon.step) {
    case 0:
        mask = duel_summon.kind ? 0x40 : 0x10;
        if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 2, duel_summon.f4)) > 0) {
            if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 0, duel_summon.f4)) > 0) {
                if (duel_player_type[duel_summon.player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    duel_0884AD40(0x13F, duel_summon.uid);
                }
            } else {
                duel_0884ACD8(duel_summon.player, 0x12);
                duel_w_08919624 = 1;
            }
        } else {
            duel_w_08919624 = 0;
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_summon.res_id = duel_summon.id;
            duel_summon.step = 0xB;
        } else {
            duel_summon.res_id = 0;
            duel_summon.step = 0;
        }
        break;
    }
    return duel_summon_tributes();
}

/* Summon of 0x1578: two opponent monsters (duel_0882A838) as tributes. */
int duel_0882DB98(void)
{
    unsigned short msg[256];
    unsigned int p;
    int z;

    switch (duel_summon.step) {
    case 0:
        duel_summon.step++;
    case 1:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_wfmt_d(msg, 2, 2);
            duel_0884AD18((int)msg);
        }
        duel_summon.n = 0;
        duel_summon.step++;
        return 0;
    case 2:
        if (duel_w_08919624 == 0) {
            return -1;
        }
    case 4:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882A838);
        } else {
            duel_08835978(duel_0882A838);
        }
        duel_summon.step++;
        return 0;
    case 3:
    case 5:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.locs[duel_summon.n++] = (unsigned char)p | (unsigned char)z << 8;
            duel_summon.step++;
            return 0;
        }
        duel_host_call(0x29, 0, 0, 0);
        duel_summon.step = 1;
        return 0;
    case 6:
        duel_summon.res_id = duel_summon.id;
        return 1;
    }
    return 0;
}

/* Summon of 0x1A92 by an effect: one face-up opponent monster of level 3 or less
   (duel_0882A940). */
int duel_0882DE18(void)
{
    unsigned int p;
    int z;

    if (duel_summon.f1) {
        switch (duel_summon.step) {
        case 0:
            duel_summon.step++;
        case 1:
            if (duel_player_type[duel_summon.player] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884AD18(1);
            }
            duel_summon.n = 0;
            duel_summon.step++;
            return 0;
        case 2:
            if (duel_w_08919624 == 0) {
                return -1;
            }
            if (duel_player_type[duel_summon.player] == 1) {
                duel_0881956C(duel_summon.player, duel_summon.id, duel_0882A940);
            } else {
                duel_08835978(duel_0882A940);
            }
            duel_summon.step++;
            return 0;
        case 3:
            if (duel_08835AC0() != 0) {
                p = duel_w_0891964C;
                z = duel_w_08919650 + duel_w_08919654;
                duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
                duel_summon.locs[duel_summon.n++] = (unsigned char)p | (unsigned char)z << 8;
                duel_summon.step++;
                duel_summon.res_id = duel_summon.id;
                return 1;
            }
            duel_host_call(0x29, 0, 0, 0);
            duel_summon.step = 1;
            return 0;
        }
        return 0;
    }
    return duel_summon_tributes();
}

#ifdef NON_MATCHING /* só a alocação de registradores difere (zona em s0, carta em s1) */
/* Zone filter: own cards that are one of the materials still asked (duel_chain.ids). */
int duel_0882E0A8(unsigned int p, int z, int i)
{
    int zz = z + i;
    int id;
    int k;

    if (zz > 0xA) {
        return 0;
    }
    if (p != duel_summon.player) {
        return 0;
    }
    id = field_get_card(p, zz);
    if (id == 0) {
        return 0;
    }
    if (duel_field_protected(p, zz) != 0) {
        return 0;
    }
    if (duel_summon.n == 0 && duel_count_free_monster_zones(duel_summon.player) == 0 && duel_zone_usable(p, zz) == 0) {
        return 0;
    }
    for (k = 0; k < duel_w_089149F0; k++) {
        if (duel_chain.ids[k] == id) {
            return 0x800;
        }
    }
    return 0;
}

#endif

#ifdef NON_MATCHING /* o original relê a contagem (duel_w_089149F0) a cada volta do laço que zera duel_chain.ids; nenhuma declaração reproduziu */
/* Summon that sends the materials named by the card (two or three fixed ids) from the field. */
int duel_0882E1B8(void)
{
    unsigned int p;
    int z;
    int id;
    int uid;
    int k;
    int j;
    unsigned int loc;
    unsigned int loc2;
    unsigned short t;

    switch (duel_summon.step) {
    case 0:
        duel_summon.step2 = 0;
        duel_w_089149F0 = 2;
        switch (duel_summon.id) {
        case 0x15B1:
            duel_chain.ids[0] = 0x1580;
            duel_chain.ids[1] = 0x15B0;
            break;
        case 0x15B4:
            duel_chain.ids[0] = 0x1580;
            duel_chain.ids[1] = 0x15B0;
            duel_chain.ids[2] = 0x15B3;
            duel_w_089149F0 = 3;
            break;
        case 0x15F9:
            duel_chain.ids[0] = 0x1580;
            duel_chain.ids[1] = 0x15B3;
            break;
        case 0x15FA:
            duel_chain.ids[0] = 0x15B0;
            duel_chain.ids[1] = 0x15B3;
            break;
        case 0x1953:
            duel_chain.ids[0] = 0x1947;
            duel_chain.ids[1] = 0x194B;
            break;
        case 0x1954:
            duel_chain.ids[0] = 0x1953;
            duel_chain.ids[1] = 0x15B4;
            break;
        case 0x1A6D:
            duel_chain.ids[0] = 0x19FD;
            duel_chain.ids[1] = 0x1A74;
            break;
        case 0x1A6E:
            duel_chain.ids[0] = 0x19FD;
            duel_chain.ids[1] = 0x1A70;
            break;
        case 0x1A6F:
            duel_chain.ids[0] = 0x19FD;
            duel_chain.ids[1] = 0x1A75;
            break;
        }
        duel_w_089149F8 = 0;
        if (card_is_in_group_1a6f_b(duel_summon.id) != 0) {
            duel_0884ACD8(duel_summon.player, 0x4C);
        } else {
            duel_0884ACD8(duel_summon.player, 0x46);
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882E0A8);
        } else {
            duel_08835978(duel_0882E0A8);
        }
        duel_summon.step++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            id = field_get_card(p, z);
            uid = CARD_UID(duel.players[p & 1].field[z].card);
            for (k = 0; k < duel_w_089149F0; k++) {
                if (duel_chain.ids[k] == id) {
                    duel_chain.ids[k] = 0;
                }
            }
            duel_w_089147E8[(duel_w_089147E4 + duel_w_089149F8) & 0x7F] = uid;
            if (++duel_w_089149F8 < duel_w_089149F0) {
                duel_summon.step--;
            } else {
                duel_summon.step++;
            }
            return 0;
        }
        duel_summon.step = 0;
        return 0;
    case 3:
        if (duel_w_089149F8 == duel_w_089149F0) {
            for (k = 0; k < duel_w_089149F0 - 1; k++) {
                loc = duel_card_locate(duel_chain.uids[(duel_w_089147E4 + k) & 0x7F]);
                for (j = 1; j < duel_w_089149F0; j++) {
                    loc2 = duel_card_locate(duel_chain.uids[(duel_w_089147E4 + j) & 0x7F]);
                    if ((unsigned char)((loc & 0xFFFF) >> 8) > (unsigned char)((loc2 & 0xFFFF) >> 8)) {
                        loc = loc2;
                        t = duel_chain.uids[(duel_w_089147E4 + k) & 0x7F];
                        duel_chain.uids[(duel_w_089147E4 + k) & 0x7F] = duel_chain.uids[(duel_w_089147E4 + j) & 0x7F];
                        duel_chain.uids[(duel_w_089147E4 + j) & 0x7F] = t;
                    }
                }
            }
        }
        duel_w_089149F8--;
        loc = duel_card_locate(duel_w_089147E8[(duel_w_089147E4 + duel_w_089149F8) & 0x7F]);
        z = (unsigned char)((loc & 0xFFFF) >> 8) + (unsigned short)(loc >> 16);
        if (z < 0xB) {
            if (card_is_in_group_1a6f_b(duel_summon.id) != 0) {
                duel_summon.step2 |= (unsigned char)(1 << duel.players[duel_summon.player & 1].field[z].card.owner);
                duel_088D19C0(duel_summon.player, z, 0, 0);
            } else {
                duel_088D1608(duel_summon.player, z, 0);
            }
        }
        if (duel_w_089149F8 == 0) {
            for (k = 0; k < 2; k++) {
                if (duel_summon.step2 >> (duel_turn_player ^ k) & 1) {
                    duel_088C78E0(duel_turn_player ^ k);
                }
            }
            duel_summon.step++;
        }
        return 0;
    }
    return 1;
}
#endif

/* Summon of 0x154B/0x13B5: asks whether to tribute three monsters (duel_0886AAB4) and goes on at
   step 0x1F of duel_summon_tributes. */
int duel_0882E84C(void)
{
    switch (duel_summon.step) {
    case 0:
    case 0x1E:
        duel_w_08919624 = 0;
        if (duel_summon.f4 == 0 && (duel_summon.kind == 0 || duel_count_active(0x135D))
            && duel_count_tributes(duel_summon.player, duel_summon.id, 1) >= 3) {
            DuelAct a = { 0 };

            a.id = duel_summon.id;
            a.player = duel_summon.player;
            a.zone = 0;
            a.uid = duel_summon.uid;
            a.b3_4 = 1;
            if (duel_0886AAB4(&a, 0)) {
                if (duel_player_type[duel_summon.player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    duel_0884AD18(0x123);
                }
            }
        }
        duel_summon.n = 0;
        duel_summon.step = 1;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_summon.step = 0x1F;
        } else {
            duel_summon.step = 0;
        }
        break;
    }
    return duel_summon_tributes();
}

#ifdef NON_MATCHING /* registradores (zona, carta e id pedido) e desvios para o fim diferem */
/* Zone filter for 0x196B-0x196E: first the named monster (0x18BF-0x18C2) with another monster of
   the attribute beside it, then a monster of the attribute. */
int duel_0882EA4C(unsigned int p, int z, int i)
{
    int zz = z + i;
    int id;
    int want;
    int k;

    if (zz > 0xA) {
        return 0;
    }
    if (p != duel_summon.player) {
        return 0;
    }
    id = field_get_card(p, zz);
    if (id == 0) {
        return 0;
    }
    if (duel_slot_efb84(p, zz) == 0) {
        return 0;
    }
    if (duel_summon.n == 0) {
        switch (duel_summon.id) {
        case 0x196B:
            want = 0x18BF;
            break;
        case 0x196C:
            want = 0x18C0;
            break;
        case 0x196D:
            want = 0x18C1;
            break;
        case 0x196E:
            want = 0x18C2;
            break;
        }
        if (id != want) {
            return 0;
        }
        for (k = 0; k < 5; k++) {
            if (k == zz) {
                continue;
            }
            if (duel_field_attr_is(p, k, card_get_kind(duel_summon.id)) != 0) {
                if (duel_count_free_monster_zones(p) != 0) {
                    return 0x800;
                }
                if (duel_zone_usable(p, zz) != 0 || duel_zone_usable(p, k) != 0) {
                    return 0x800;
                }
            }
        }
    } else if (duel_summon.locs[0] != (unsigned short)((unsigned char)p | (unsigned char)zz << 8)
               && duel.players[p & 1].field[zz].faceup != 0
               && duel_field_attr_is(p, zz, card_get_kind(duel_summon.id)) != 0) {
        if (duel_count_free_monster_zones(p) != 0) {
            return 0x800;
        }
        if (duel_zone_usable(p, zz) != 0) {
            return 0x800;
        }
    }
    return 0;
}

#endif

/* Summon of 0x196B-0x196E by an effect: the two monsters of duel_0882EA4C are destroyed. */
int duel_0882EC88(void)
{
    if (!duel_summon.f1) {
        return duel_summon_tributes();
    }
    switch (duel_summon.step) {
    case 0:
        duel_summon.n = 0;
        duel_0884ACD8(duel_summon.player, 0x3A);
    case 2:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882EA4C);
        } else {
            duel_08835978(duel_0882EA4C);
        }
        duel_summon.step++;
        return 0;
    case 1:
    case 3:
        if (duel_08835AC0() != 0) {
            duel_summon.locs[duel_summon.n++] = (unsigned char)duel_w_0891964C | (unsigned char)(duel_w_08919650 + duel_w_08919654) << 8;
            duel_summon.step++;
            return 0;
        }
        return -1;
    case 4:
        duel_088C7BA4();
        duel_088D1BEC(duel_summon.locs[0] & 0xFF, (unsigned char)(duel_summon.locs[0] >> 8), 0, 0);
        duel_088D1BEC(duel_summon.locs[1] & 0xFF, (unsigned char)(duel_summon.locs[1] >> 8), 0, 0);
        duel_088C7CF0();
        duel_summon.n = 0;
        duel_summon.step++;
        return 0;
    case 5:
        return 1;
    }
    return 0;
}
