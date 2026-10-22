// @range 0x08861E7C-0x08862E24
// @asm 0x08862208-0x088622B8
// @asm 0x0886268C-0x08862E24
/* The computer player: handlers of the cards it activates. */
#include "duel.h"

void duel_088C4448(unsigned int p, int z, unsigned int uid);
unsigned int duel_card_locate(unsigned int uid);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_0884E5E0(DuelAct *a);
int duel_088619FC(DuelAct *a, int b);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088328B8(unsigned int p, unsigned short a, int b);
void duel_0884AD40(int msg, int uid);
void duel_088C4564(unsigned int p, int n);
unsigned short duel_slot_e1854(unsigned int p, int z);
void duel_088D1B20(unsigned int p, unsigned int mask);
int field_get_card(unsigned int p, int z);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_08856860(DuelAct *a, unsigned int p, int z);
int duel_0884E690(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
unsigned short *card_get_name(int id);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
void duel_0884ACD8(unsigned int p, int msg);
int duel_08835AC0(void);
int duel_field_base_atk(unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_088619C4(DuelAct *a, int b);
void duel_088D1AC8(unsigned int p, int z);
int duel_count_tributable_except(unsigned int p, int src);
int duel_count_free_monster_zones(unsigned int p);
int duel_08874528(DuelAct *a, int b);
int duel_count_active(int id);
int duel_act_target_is_monster(DuelAct *a);
int duel_hand_can_discard(unsigned int p, int n, int uid, int monster, int type16);
int duel_pick_st_zone(unsigned int p);
int duel_phase_ok2(unsigned int p);
int duel_grave_count_id(unsigned int p, int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_08861480(int id, int n);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_08832974(unsigned int p, unsigned short id, int a);
int duel_0886191C(DuelAct *a, int b);
void duel_08832788(unsigned int p, int id, int a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);

extern int duel_w_08914904;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_phase;

int duel_08861E7C(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned char z;

    if (a->b3_4 != 2) {
        t = duel_card_locate(a->uid);
        p = a->player;
        if (p == (t & 0xFF) && (z = (unsigned char)a->zone, z == ((unsigned short)t >> 8 & 0xFF))) {
            if ((t >> 16 & 0xFFFF) != 0xFFFF && duel.players[p & 1].field[z].faceup != 0) {
                duel_088C7E68(p, z, 4, 1);
                duel_088C89BC(a->player, a->zone, 0x1174, 1);
            }
            duel_088C4448(a->player, a->zone, a->uid);
            return 1;
        }
        if (a->id == 0x1A90 && duel_0884E5E0(a) != 0) {
            duel_088C4448(a->player, a->zone, a->uid);
            return 1;
        }
        return -1;
    }
    duel_088C4448(a->player, a->zone, a->uid);
    return 1;
}

int duel_08862000(DuelAct *a, int b)
{
    int r = duel_088619FC(a, b);

    if (r < 0) {
        duel_088C77D4(a->player, a->id, 1, 0);
    }
    return r;
}

int duel_08862054(DuelAct *a)
{
    if (a->kind != 0x23) {
        duel_088328B8(a->player, 0xFFFF, 1);
    }
    return 1;
}

int duel_08862098(DuelAct *a)
{
    if (a->kind == 2) {
        return 1;
    }
    switch (duel_w_08914904) {
    case 0:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD40(0xE4, a->uid);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_w_08919624 == 0) {
            return -1;
        }
        duel_088C4564(a->player, 500);
        return 1;
    }
    return 1;
}

int duel_08862168(DuelAct *a)
{
    unsigned int p = a->player;
    int z = a->zone;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return -1;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return 1;
    }
    return -1;
}

#ifdef NON_MATCHING /* o original calcula o bit da segunda zona antes do primeiro e mantém o 0 inicial da máscara num registrador; o mwcc agenda de outro jeito (176 bytes contra 168) */
int duel_08862208(DuelAct *a)
{
    unsigned int t = duel_slot_e1854(a->player, a->zone);
    unsigned int p = a->player;
    unsigned int z = (unsigned short)t >> 8 & 0xFF;
    unsigned int q = t & 0xFF;
    unsigned int m = 0;

    m |= 1 << (a->zone + p * 16);
    m |= 1 << (z + q * 16);
    duel_088D1B20(p ^ a->opp, m);
    a->w[0] = field_get_card(q, z);
    return 1;
}
#endif

int duel_088622B8(DuelAct *a, int b)
{
    unsigned short buf[512];
    int (*fn)(DuelAct *a, unsigned int p, int z);
    int id;
    unsigned int p;
    int z;

    if (a->kind == 0x23) {
        return 1;
    }
    switch (duel_w_08914904) {
    case 0:
        id = 0;
        fn = duel_0884E3E4;
        switch (a->id) {
        case 0xFE0:
            id = 0xFE0;
            break;
        case 0x167D:
            id = 0xFC9;
            break;
        case 0x1713:
            id = 0xFC9;
            break;
        case 0x187E:
            id = 0x13C3;
            break;
        case 0x19B5:
            id = 0x18F6;
            break;
        case 0x192A:
            fn = duel_08856860;
            break;
        }
        if (duel_0884E690(a, fn) == 0) {
            return 1;
        }
        if (id != 0) {
            duel_wfmt_s(buf, 0x13, card_get_name(id));
            duel_0884ACD8(a->player, (int)buf);
        } else {
            duel_0884ACD8(a->player, 0x12);
        }
        duel_088359E8(a->player, a->id, fn);
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            switch (a->id) {
            case 0xFFF:
                a->w[0] = duel_field_atk(p, z) + 1 >> 1;
                break;
            case 0x14E4:
                a->w[0] = duel_field_atk(p, z);
                a->n7++;
                break;
            case 0x140B:
            case 0x15A3:
                a->w[0] = duel_field_level(p, z);
                break;
            case 0x1768:
                a->w[0] = duel_field_level(p, z) + 3;
                a->n7++;
                break;
            case 0x1708:
                a->w[1] = field_get_card(p, z);
                break;
            case 0x1927:
                a->w[1] = CARD_UID(duel.players[p & 1].field[z].card);
                break;
            case 0x1929:
                a->w[0] = duel_field_base_atk(p, z);
                break;
            case 0x1855:
            case 0x19A5:
                a->w[0] = duel_field_base_atk(p, z);
                duel_088619C4(a, b);
                break;
            case 0x1A2B:
            case 0x1A2C:
            case 0x1A2D:
            case 0x1A2E:
            case 0x1A61:
            case 0x1AA0:
                duel_088619C4(a, b);
                break;
            }
            duel_088D1AC8(p, z);
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}

int duel_08862648(DuelAct *a)
{
    if (a->kind == 2) {
        duel_088D1AC8(a->player, a->zone);
    }
    return 1;
}

#ifdef NON_MATCHING /* só difere na ordem do cálculo do endereço do slot (z * 20 antes de jogador * 0xFAC) depois de duel_slot_e1854, como em duel_08862208 */
int duel_0886268C(DuelAct *a, int b)
{
    int msg = 0x103;
    unsigned int m = 0;
    unsigned int p;
    unsigned short t;

    switch (duel_w_08914904) {
    case 0:
        switch (a->id) {
        case 0x1298:
            a->w[2] = 2;
            m = 1;
            if (duel_0884E5E0(a) != 0) {
                m |= 2;
            }
            break;
        case 0x14E0:
            m = 1;
            if (duel_0884E5E0(a) != 0) {
                m |= 2;
            }
            break;
        case 0x1581:
            a->w[2] = 2;
            m = 1;
            if (duel_0884E5E0(a) != 0 && duel_count_tributable_except(a->player, -1) != 0) {
                m |= 2;
            }
            break;
        case 0x1599:
            p = a->player;
            if (duel.players[p & 1].n_d != 0) {
                m |= 1;
            }
            if (duel.players[(1 - p) & 1].n_d != 0) {
                m |= 2;
            }
            msg = 0x117;
            break;
        case 0x15CF:
            if (duel_08874528(a, b) != 0) {
                m |= 1;
            }
            if (duel_phase == 2 && duel_count_active(0x13F2) == 0) {
                m |= 2;
            }
            break;
        case 0x15D3:
        case 0x1916:
            if (duel_08874528(a, b) != 0) {
                m |= 1;
            }
            if (duel_slot_has_effect(a->player, a->zone, 0x15D3) == 0) {
                m |= 2;
            }
            break;
        case 0x15E7:
        case 0x1885:
            m = 3;
            break;
        case 0x15F1:
            if (duel_act_target_is_monster((DuelAct *)b) != 0) {
                m |= 1;
            }
            if (duel_hand_can_discard(a->player, 1, a->uid, 0, 1) != 0) {
                m |= 2;
            }
            break;
        case 0x15F7:
            a->w[2] = 1;
            if (duel_pick_st_zone(a->player) >= 0 && duel_0884E5E0(a) != 0) {
                m |= 1;
            }
            a->w[2] = 2;
            if (duel_phase_ok2(a->player) != 0 && duel_count_free_monster_zones(a->player) != 0 && duel_0884E5E0(a) != 0) {
                m |= 2;
            }
            break;
        case 0x1679:
            if (duel_grave_count_id(a->player, 0x167A) != 0) {
                m |= 1;
            }
            if (duel_grave_count_id(a->player, 0x167B) != 0) {
                m |= 2;
            }
            break;
        case 0x1776:
            m = 1;
            if (duel_0884E5E0(a) != 0 && (unsigned int)duel.players[a->player & 1].n_d >= 2) {
                m |= 2;
            }
            break;
        case 0x1841:
            p = a->player;
            if (duel.players[p & 1].n_d != 0) {
                if (duel_phase == 0 || duel_find_effect_card(1 - p, 0x178B, -1) == 0) {
                    m |= 1;
                }
            }
            if (duel.players[(1 - a->player) & 1].n_hand != 0) {
                m |= 2;
            }
            break;
        }
        if (m == 0) {
            return -1;
        }
        p = a->player;
        if (duel_player_type[p] == 1) {
            switch (a->id) {
            case 0x1298:
                if (duel_0884E948(a, 1 - p) != 0) {
                    duel_w_08919624 = 1;
                } else {
                    duel_w_08919624 = 0;
                }
                break;
            case 0x14E0:
            case 0x1599:
            case 0x1679:
            case 0x1776:
                if (m & 2) {
                    duel_w_08919624 = 1;
                } else {
                    duel_w_08919624 = 0;
                }
                break;
            default:
                if (m & 1) {
                    duel_w_08919624 = 0;
                } else {
                    duel_w_08919624 = 1;
                }
                break;
            }
        } else {
            duel_0884ADAC(msg, duel_08861480(a->id, 0), duel_08861480(a->id, 1), 0, 0, m);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        a->w[2] = duel_w_08919624 + 1;
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        duel_w_08914904++;
        return 0;
    case 2:
        switch (a->id) {
        case 0x1581:
            if (a->w[2] == 2) {
                duel_088328B8(a->player, 0xFFFF, 1);
            }
            break;
        case 0x1599:
            duel_0886191C(a, b);
            duel_088619C4(a, b);
            break;
        case 0x15CF:
            if (a->w[2] == 1) {
                duel_088619C4(a, b);
            }
            if (a->w[2] == 2) {
                t = duel_slot_e1854(a->player, a->zone);
                duel_088D1AC8(a->player, a->zone);
                p = (unsigned short)t >> 8 & 0xFF;
                m = t & 0xFF;
                a->w[3] = CARD_UID(duel.players[m & 1].field[p].card);
            }
            break;
        case 0x15D3:
        case 0x1916:
            if (a->w[2] == 1) {
                duel_088619C4(a, b);
            }
            if (a->w[2] == 2) {
                duel_088C87A8(a->player, a->zone, 0x15D3, 2, 0);
            }
            break;
        case 0x15F1:
            if (a->w[2] == 2) {
                duel_08832788(a->player, 1, 0, 1);
            }
            break;
        case 0x1679:
            duel_0886191C(a, b);
            break;
        }
        return 1;
    }
    return 0;
}
#endif
