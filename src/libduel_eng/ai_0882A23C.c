// @range 0x0882A23C-0x0882AFA4
// @asm 0x0882A23C-0x0882A550
// @asm 0x0882A680-0x0882A838
// @asm 0x0882AAA0-0x0882ABB4
// @data 0x08907940-0x0890794C
/* Summoning a monster (duel_summon, 0x08914A14): the zone filters used while the player or the
   computer picks the tributes, and the start of a summon. */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_w_08919638;
extern unsigned int duel_w_0891964C;
extern int duel_w_0891962C;

int duel_count_free_monster_zones(unsigned int p);
int duel_field_locate(int uid);
int duel_zone_usable(unsigned int p, unsigned int z);
int field_get_card(unsigned int p, int z);
int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_can_tribute(unsigned int p, unsigned int q, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
unsigned int duel_field_type(unsigned int p, int z);
int card_get_type(int id);
int card_get_kind(int id);
int duel_field_level(unsigned int p, int z);
int duel_count_active(int id);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_list_card_level(unsigned int p, int i);
int card_is_in_group_1a6x(int id);

/* monsters asked one after the other by 0x1288 (duel_0882A23C) */
int duel_ritual_ids[3] = { 0x1288, 0x129B, 0x12B8 };

#ifdef NON_MATCHING /* desvios do fim com atrasos vazios (b fim; nop) que nenhuma forma do fonte reproduziu */
/* Zone filter (0x800 = can be picked): the monsters of the ritual of 0x1288, one per step. */
int duel_0882A23C(unsigned int p, int z, int i)
{
    int id = duel_ritual_ids[duel_summon.n];
    int blocked = 0;
    int zz;
    int k;
    int j;

    if (duel_count_free_monster_zones(duel_summon.player) == 0) {
        blocked = 1;
        for (k = 0; k < duel_summon.n; k++) {
            unsigned int loc = duel_field_locate(duel_summon.uids[k]);

            if ((int)loc >= 0 && (loc & 0xFF) == duel_summon.player
                && duel_zone_usable(loc & 0xFF, (unsigned char)((loc & 0xFFFF) >> 8))) {
                blocked = 0;
            }
        }
        for (k = duel_summon.n + 1; k < 3; k++) {
            for (j = 0; j < 5; j++) {
                if (duel_ritual_ids[k] == field_get_card(duel_summon.player, j)
                    && duel_tributable(duel_summon.player, duel_summon.player, j)
                    && duel_zone_usable(duel_summon.player, j)) {
                    blocked = 0;
                    break;
                }
            }
        }
    }
    zz = z + i;
    if (zz < 5) {
        if (duel_tributable(duel_summon.player, p, zz) == 0) {
            return 0;
        }
        if (p != duel_summon.player && duel.players[p & 1].field[zz].faceup == 0) {
            return 0;
        }
        if (id != field_get_card(p, zz)) {
            return 0;
        }
        if (!blocked) {
            return 0x800;
        }
        if (p != duel_summon.player) {
            return 0;
        }
        if (duel_zone_usable(p, zz) == 0) {
            return 0;
        }
        return 0x800;
    }
    if (blocked) {
        return 0;
    }
    if (p == duel_summon.player && z == 0xB && id == duel.players[p & 1].hand[i].id) {
        return 0x800;
    }
    return 0;
}
#endif

/* Zone filter: own monsters of the attribute (or type) the card being summoned asks for. */
int duel_0882A550(unsigned int p, int z, int i)
{
    unsigned int id;
    int zz;
    int r;

    if (p != duel_summon.player) {
        return 0;
    }
    zz = z + i;
    if (zz > 4) {
        return 0;
    }
    id = duel_summon.id;
    switch (id) {
    case 0x1466:
    case 0x18B4:
    case 0x19CA:
        r = (unsigned short)duel_field_type(p, zz) == card_get_type(id);
        break;
    case 0x16C9:
    case 0x16CB:
    case 0x16E4:
        r = duel_field_attr_is(p, zz, duel_summon.step2 ? 2 : 1);
        break;
    default:
        r = duel_field_attr_is(p, zz, card_get_kind(id));
        break;
    }
    if (r != 0) {
        return 0x800;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original mantém cópias mortas depois dos beql do teste 0x13E7/0x13E5 */
/* Zone filter: monsters that may be tributed for the summon (0x13E5/0x13E7 when none was picked
   yet). */
int duel_0882A680(unsigned int p, int z, int i)
{
    int k;
    int r;

    int zz = z + i;

    if (zz > 4) {
        return 0;
    }
    if (duel_summon.f1) {
        if (duel_tributable(duel_summon.player, p, zz) == 0) {
            return 0;
        }
    } else {
        if (duel_can_tribute(duel_summon.player, p, zz) == 0) {
            return 0;
        }
    }
    if (duel_summon.n == 0) {
        int id = field_get_card(p, zz);

        switch (id) {
        case 0x13E5:
            r = 0x800;
            break;
        case 0x13E7:
            r = 0x800;
            break;
        default:
            r = 0;
            break;
        }
    } else {
        for (k = 0; k < (duel_summon.n & 0xFF); k++) {
            if (duel_summon.locs[k] == (unsigned short)((unsigned char)p | (unsigned char)zz << 8)) {
                return 0;
            }
        }
        if (duel_count_free_monster_zones(duel_summon.player) == 0) {
            if ((duel_summon.locs[0] & 0xFF) != duel_summon.player
                || !duel_zone_usable(duel_summon.locs[0] & 0xFF, (unsigned char)((duel_summon.locs[0] & 0xFFFF) >> 8))) {
                if (p == duel_summon.player) {
                    if (duel_zone_usable(p, zz) == 0) {
                        return 0;
                    }
                } else {
                    return 0;
                }
            }
        }
        r = 0x800;
    }
    return r;
}

#endif

/* Zone filter: opponent monsters that may be tributed. */
int duel_0882A838(unsigned int p, int z, int i)
{
    int k;

    int zz = z + i;

    if (zz > 4) {
        return 0;
    }
    if (p != 1 - duel_summon.player) {
        return 0;
    }
    if (duel_tributable(p, p, zz) == 0) {
        return 0;
    }
    if (duel_summon.n == 0) {
        if (duel_count_free_monster_zones(1 - duel_summon.player) == 0) {
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
    return 0x800;
}

/* Zone filter: face-up opponent monsters of level 3 or less that may be tributed. */
int duel_0882A940(unsigned int p, int z, int i)
{
    int zz = z + i;

    if (zz > 4) {
        return 0;
    }
    if (p != 1 - duel_summon.player) {
        return 0;
    }
    if (duel.players[p & 1].field[zz].faceup == 0) {
        return 0;
    }
    if (duel_tributable(p, p, zz) == 0) {
        return 0;
    }
    if (duel_field_level(p, zz) > 3) {
        return 0;
    }
    return 0x800;
}

/* Hand filter: the card being summoned. */
int duel_0882AA10(unsigned int p, int z, int i)
{
    if (p == duel_summon.player && z == 0xB && duel_summon.uid != CARD_UID(duel.players[p & 1].hand[i])) {
        return 0x800;
    }
    return 0;
}

#ifdef NON_MATCHING /* só a alocação de registradores difere (need em v1, base em a0) */
/* Zone filter: own spell/trap zone holding 0x19B2 with enough counters for the level. */
int duel_0882AAA0(unsigned int p, int z, int i)
{
    int zz = z + i;
    int need;

    if (duel_summon.player != p) {
        return 0;
    }
    if (zz < 5) {
        return 0;
    }
    if (zz >= 10) {
        return 0;
    }
    if (duel_count_active(0x13F2) != 0) {
        return 0;
    }
    if (duel_slot_e8170(p, zz, 0x19B2) != 0) {
        need = (duel_list_card_level(duel_summon.player, duel_summon.uid) >= 7) + 1;

        if (duel.players[duel_summon.player & 1].field[zz].link >= need) {
            return 0x800;
        }
    }
    return 0;
}

#endif

/* Zone filter: monsters of the 0x1A6x group that may be tributed (0x1A65). */
int duel_0882ABB4(unsigned int p, int z, int i)
{
    int k;

    int zz = z + i;

    if (zz > 4) {
        return 0;
    }
    if (duel_tributable(duel_summon.player, p, zz) == 0) {
        return 0;
    }
    if (duel_summon.n == 0) {
        if (p != duel_summon.player && duel.players[p & 1].field[zz].faceup == 0) {
            return 0;
        }
        if (card_is_in_group_1a6x(field_get_card(p, zz)) == 0) {
            return 0;
        }
        return 0x800;
    } else {
        for (k = 0; k < (duel_summon.n & 0xFF); k++) {
            if (duel_summon.locs[k] == (unsigned short)((unsigned char)p | (unsigned char)zz << 8)) {
                return 0;
            }
        }
    }
    return 0x800;
}

/* Starts a summon of the card uid of player p. mode: 0 face-up (also with 0x135D active), 1 set,
   2 by an effect; n: tributes asked by the caller. */
void duel_summon_init(unsigned int p, int uid, int mode, int n)
{
    duel_summon.player = p;
    duel_summon.id = duel.list[uid].id;
    duel_summon.uid = uid;
    duel_summon.res_id = 0;
    duel_summon.f2 = n > 0;
    duel_summon.f4 = n > 1 ? n - 1 : 0;
    duel_summon.b5 = 1;
    duel_summon.kind = 2;
    duel_summon.f1 = 0;
    duel_summon.f3 = 0;
    switch (mode) {
    case 0:
        duel_summon.kind = 1;
        duel_summon.b5 = duel_count_active(0x135D) > 0;
        break;
    case 1:
        duel_summon.kind = 0;
        break;
    case 2:
        duel_summon.f1 = 1;
        break;
    }
    duel_summon.f0 = 1;
    duel_summon.n = 0;
    duel_summon.step = 0;
    duel_summon.step2 = 0;
    duel_summon.step3 = 0;
    duel_summon.unk1b = 0;
}

int duel_0882AF5C(int mode, int by_effect, int n)
{
    duel_summon_init(duel_w_0891964C, duel_w_0891962C, by_effect != 0 ? 2 : mode, n);
    duel_w_08919638 = 0;
    return 1;
}
