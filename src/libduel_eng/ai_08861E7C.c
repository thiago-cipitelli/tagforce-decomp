// @range 0x08861E7C-0x0886268C
// @asm 0x08862208-0x088622B8
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

extern int duel_w_08914904;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

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
