// @range 0x0884C718-0x0884D050
// @asm 0x0884C8E4-0x0884CB8C
/* List of cards a dialog lets the player pick from (each with the zone and index it came from), the
   duel random generator, pile sizes and a few checks of the presentation. */
#include "duel.h"

typedef struct DuelPick {              /* 0x0891524C */
    int unk0;
    int kind;                          /* +0x004 what the list is for */
    int cur;                           /* +0x008 picked entry */
    unsigned int n;                    /* +0x00C */
    DuelCard c[512];                   /* +0x010 */
    union {
        unsigned short w;
        struct {
            unsigned short zone : 8;
            unsigned short n : 8;
        } b;
    } where[512];                      /* +0x810 zone and number */
} DuelPick;

extern DuelPick duel_w_0891524C;
extern unsigned int duel_view_player;  /* 0x089139F4 */
extern unsigned int duel_089195C8;     /* random seed */
extern unsigned int duel_turn;
extern int duel_w_089195D8;
extern int duel_w_08917DC4;
extern int duel_w_089195E4;

int duel_req_flag(int f);
int duel_zone_faceup(unsigned int p, int z, int i);
int duel_msg_busy(void);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, int c);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_find_st(unsigned int p, int id);
int card_is_monster(int id);
unsigned int duel_grave_find_uid(unsigned int p, int uid);
unsigned int duel_f_find_uid(unsigned int p, int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int duel_zone_f_card_usable(unsigned int p, unsigned int q, int i);
unsigned int duel_card_locate(int uid);

unsigned int duel_0884C718(void)
{
    int k = duel_w_0891524C.kind;
    unsigned int r = 1;

    if (k < 6) {
        return 0;
    }
    if (k == 0x26 || (unsigned int)(k - 0x47) < 2) {
        return duel_w_0891524C.n;
    }
    if (k > 6 && k < 0x26) {
        r = k - 6;
    } else if (k > 0x27 && k < 0x47) {
        r = k - 0x27;
    }
    return MIN(r, duel_w_0891524C.n);
}

int duel_0884C7B8(void)
{
    return duel_req_flag(0x32);
}

void duel_0884C7C0(int i)
{
    duel_w_0891524C.cur = i;
}

DuelCard *duel_0884C7CC(void)
{
    return &duel_w_0891524C.c[duel_w_0891524C.cur];
}

DuelCard *duel_0884C7EC(int i)
{
    return &duel_w_0891524C.c[i];
}

unsigned int duel_0884C804(void)
{
    return duel_w_0891524C.where[duel_w_0891524C.cur].b.zone;
}

unsigned int duel_0884C824(void)
{
    return duel_w_0891524C.n;
}

unsigned int duel_0884C830(int i)
{
    unsigned int q = duel_w_0891524C.c[i].owner;

    if (duel_w_0891524C.kind == 4 && q == (duel_view_player ^ 1) && duel_zone_faceup(q, 0xF, i) == 0) {
        return 0;
    }
    if (duel_w_0891524C.kind == 0x4A) {
        return 0;
    }
    return duel_w_0891524C.c[i].id;
}

unsigned int duel_0884C8CC(int i)
{
    return duel_w_0891524C.where[i].b.zone;
}

#ifdef NON_MATCHING /* argument order of the card uid and register allocation not reproduced */
unsigned int duel_0884C8E4(int i)
{
    DuelCard *c = &duel_w_0891524C.c[i];
    unsigned int z = duel_w_0891524C.where[i].b.zone;
    unsigned int f;

    switch (z) {
    default:
        f = 1;
        break;
    case 0xB:
        f = 2;
        break;
    case 0xD:
        f = 4;
        break;
    case 0xE:
        f = 8;
        break;
    case 0xC:
        f = 0x10;
        break;
    case 0xF:
        f = 0x20;
        break;
    case 0x10:
        f = 0;
        break;
    }
    if (card_is_monster(c->id) != 0 && z - 0xE < 2) {
        if (z == 0xE && duel_grave_card_usable(duel_view_player, c->owner,
                                               duel_grave_find_uid(c->owner, CARD_UID(*c))) == 0) {
            f |= 0x80;
        }
        if (z == 0xF && duel_zone_f_card_usable(duel_view_player, c->owner,
                                                duel_f_find_uid(c->owner, CARD_UID(*c))) == 0) {
            f |= 0x80;
        }
        switch (*(unsigned int *)c >> 15 & 3) {
        case 2:
            f |= 0x100;
            break;
        case 1:
            f |= 0x200;
            break;
        }
    }
    if (c->b21) {
        f |= 0x40;
    }
    if (z == 0xF) {
        unsigned int t = duel_card_locate(CARD_UID(*c));

        if ((t & 0xFFFF) >> 8 == 0xF) {
            unsigned char m = duel.players[t & 1].marks[t >> 16];

            if (m == 0x80) {
                f = (f & 0x3F) | 0x800;
            } else if (m == 0x40) {
                f |= 0x400;
            }
        }
    }
    if (duel_w_0891524C.kind == 0x4A) {
        f &= 0x3F;
    }
    if (c->owner == (duel_view_player ^ 1)) {
        f |= 0x1000;
    }
    return f;
}
#endif

void duel_0884CB8C(int i, int v)
{
    if (i < 0x200) {
        duel_w_0891524C.where[i].w = duel_w_0891524C.where[i].b.zone | (unsigned char)v << 8;
    }
}

int duel_0884CBC4(int i)
{
    return duel_w_0891524C.where[i].w >> 8;
}

void duel_0884CBE0(int k)
{
    unsigned int i;

    for (i = 0; i < duel_w_0891524C.n; i++) {
        int v = duel_0884CBC4(i);

        if (v == k) {
            duel_0884CB8C(i, 0);
        } else if (v > k) {
            duel_0884CB8C(i, v - 1);
        }
    }
}

void duel_0884CC68(int k)
{
    unsigned int i;

    for (i = 0; i < duel_w_0891524C.n; i++) {
        if (k == duel_w_0891524C.where[i].w >> 8) {
            duel_w_0891524C.cur = i;
            return;
        }
    }
    duel_w_0891524C.cur = -1;
}

DuelCard *duel_0884CCBC(int k)
{
    unsigned int i;

    for (i = 0; i < duel_w_0891524C.n; i++) {
        if (k == duel_w_0891524C.where[i].w >> 8) {
            return &duel_w_0891524C.c[i];
        }
    }
    return &duel_w_0891524C.c[i];
}

unsigned int duel_0884CD24(void)
{
    duel_089195C8 = duel_089195C8 * 0x343FD + 0x269EC3;
    return duel_089195C8 >> 16 & 0x7FFF;
}

int duel_rand(int n)
{
    return (unsigned int)(n * duel_0884CD24()) >> 15;
}

int duel_0884CD88(unsigned int p, int z)
{
    switch (z) {
    case 0xC:
        return duel.players[p & 1].n_18;
    case 0xD:
        return duel.players[p & 1].n_d;
    case 0xE:
        return duel.players[p & 1].n_grave;
    case 0xF:
        return duel.players[p & 1].n_f;
    }
    return 0;
}

void duel_0884CE44(void)
{
    if (duel_w_089195D8 == 0xFFFF && duel_msg_busy() == 0) {
        duel_w_089195D8 = duel_turn;
        duel_cmd((duel_view_player ? 0x8000 : 0) | 0xB, 0, 0, 0);
    }
}

void duel_0884CEBC(unsigned int p)
{
    if (duel_w_08917DC4 == 0) {
        int v = 1;

        duel_w_08917DC4 = v;
        if (p == duel_view_player) {
            v = 2;
        }
        duel_w_089195E4 = v;
        duel_cmd((p ? 0x8000 : 0) | 0x23, 0xB, 0, 0);
    }
}

int duel_0884CF30(unsigned int p)
{
    if (duel_hand_count_name(p, 0xFB7) == 0) {
        return 0;
    }
    if (duel_hand_count_name(p, 0xFB8) == 0) {
        return 0;
    }
    if (duel_hand_count_name(p, 0xFB9) == 0) {
        return 0;
    }
    if (duel_hand_count_name(p, 0xFBA) == 0) {
        return 0;
    }
    return duel_hand_count_name(p, 0xFBB) != 0;
}

int duel_0884CFBC(unsigned int p)
{
    if (duel_find_effect_card(p, 0x1468, -1) == 0) {
        return 0;
    }
    if (duel_find_st(p, 0x1497) < 0) {
        return 0;
    }
    if (duel_find_st(p, 0x1498) < 0) {
        return 0;
    }
    if (duel_find_st(p, 0x1499) < 0) {
        return 0;
    }
    return duel_find_st(p, 0x149A) >= 0;
}
