// @range 0x088070A8-0x088087C0
// @asm 0x08807C7C-0x08808364
/* Computer player: card lists and the search for tribute targets. */
#include "duel.h"
#include "card_db.h"

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

extern DuelPick duel_w_0891524C;        /* same as duel_pick.c */

int duel_tributable(unsigned int p, unsigned int q, int z);
int duel_can_tribute(unsigned int p, unsigned int q, int z);
int duel_0881616C(unsigned int p, unsigned int mask, int a);
int duel_0881657C(unsigned int p, unsigned int mask, int a);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_list_card_level(unsigned int p, int i);
int duel_08827DA4(unsigned int p, int uid, int a, int b);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_d_count_name(unsigned int p, unsigned short id);
int duel_count_monsters(unsigned int p);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_0880A54C(DuelAct *a, unsigned int p, int skip, int any);
void duel_0880546C(unsigned int p, int z, DuelStats *st);
int duel_field_atk(unsigned int p, int z);
int duel_count_active(int id);
int duel_0886C448(DuelAct *a);
int duel_count_field_all(unsigned int p);
int duel_088AA224(DuelAct *a);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_hand_count_monsters(unsigned int p);
int duel_ai_088204A8(DuelAct *a, int b);
unsigned int duel_0884E4A0(DuelAct *a);
int duel_088159C8(unsigned int p, unsigned int mask, int (*fn)());
int duel_08817474();
int duel_0884CBC4(int i);
int duel_0880AF74(unsigned int p, int id);
int duel_0880AA04(int id, int a);
int duel_rand(int n);
int duel_count_faceup_name(unsigned int p, int id);

extern unsigned int duel_w_08913CF4;
extern unsigned int duel_atk_tp;       /* 0x08913F14 */
extern int duel_atk_zone;              /* 0x08913F2C */
extern int duel_atk_tz;                /* 0x08913F30 */

int duel_088070A8(int id)
{
    switch (id) {
    case 0xFD6:
    case 0x11E4:
    case 0x13E9:
    case 0x10D6:
    case 0x10DD:
    case 0x10DF:
    case 0x10E0:
    case 0x10E7:
    case 0x1174:
    case 0x133B:
    case 0x13E3:
    case 0x1185:
    case 0x1309:
    case 0x130B:
    case 0x1333:
    case 0x1335:
    case 0x1339:
    case 0x133A:
    case 0x133C:
    case 0x133E:
    case 0x133F:
    case 0x1342:
    case 0x13C7:
    case 0x19FE:
    case 0x1435:
        return 1;
    }
    return 0;
}

int duel_088071E8(int id)
{
    switch (id) {
    case 0x1333:
    case 0x1335:
    case 0x1339:
    case 0x133A:
    case 0x133C:
    case 0x133E:
    case 0x133F:
    case 0x1342:
    case 0x139D:
    case 0x139E:
    case 0x13C7:
    case 0x14AB:
    case 0x14DD:
    case 0x14E7:
    case 0x14F6:
    case 0x152F:
    case 0x156C:
    case 0x156D:
    case 0x1591:
    case 0x17C3:
    case 0x17D6:
    case 0x17E5:
    case 0x17E6:
    case 0x1867:
    case 0x18B7:
    case 0x18F7:
    case 0x1939:
    case 0x196A:
    case 0x19C5:
        return 1;
    }
    return 0;
}

int duel_08807358(unsigned int p, unsigned int q, int z, int any)
{
    if (any) {
        return duel_can_tribute(p, q, z);
    } else {
        return duel_tributable(p, q, z);
    }
}

/* Zones (bit q * 16 + z) of the monsters p could tribute. */
unsigned int duel_08807388(unsigned int p, int any)
{
    unsigned int mask = 0;
    int q;
    int z;

    for (q = 0; q < 2; q++) {
        for (z = 0; z <= 4; z++) {
            if (duel_08807358(p, q, z, any)) {
                mask |= 1 << (z + q * 16);
            }
        }
    }
    return mask;
}

int duel_08807474(unsigned int p, int a, unsigned int mask);

/* Best monster for p to tribute: player | zone << 8, or -1. */
int duel_08807434(unsigned int p, int any)
{
    return duel_08807474(p, any, duel_08807388(p, any));
}

int duel_08807474(unsigned int p, int a, unsigned int mask)
{
    unsigned int q = 1 - p;
    int r = duel_0881616C(q, mask, 0);

    if (r >= 0) {
        return (unsigned short)(q & 0xFF | (r & 0xFF) << 8);
    }
    r = duel_0881657C(p, mask, 1);
    if (r >= 0) {
        return (unsigned short)(p & 0xFF | (r & 0xFF) << 8);
    }
    return -1;
}

int duel_08807814(unsigned int p, int a, unsigned int mask);

/* Tribute number i for card id, skipping the n tributes already picked in buf; -1 if none fits. */
int duel_0880750C(unsigned int p, int id, int a, int n, unsigned short *buf)
{
    unsigned int mask;
    int any;
    int i;
    int r;
    DuelStats st;

    any = !a;
    mask = duel_08807388(p, any);
    for (i = 0; i < n; i++) {
        mask &= ~(1 << ((buf[i] >> 8 & 0xFF) + (buf[i] & 0xFF) * 16));
    }
    r = duel_08807474(p, any, mask);
    if (r != -1) {
        return r;
    }
    if ((r = duel_08807814(p, any, mask)) != -1) {
        duel_field_stats(r & 0xFF, (unsigned short)r >> 8 & 0xFF, &st);
        switch (id) {
        case 0x1388:
        case 0x1688:
        case 0x1654:
            if (st.batk > 0) {
                return r;
            }
        }
        if (st.level < 6 && card_get_atk(id) > st.atk) {
            return r;
        }
    }
    return -1;
}

int duel_088077D4(unsigned int p, int any)
{
    return duel_08807814(p, any, duel_08807388(p, any));
}

int duel_08807814(unsigned int p, int a, unsigned int mask)
{
    unsigned int q = 1 - p;
    int r = duel_0881616C(q, mask, 0);

    if (r >= 0) {
        return (unsigned short)(q & 0xFF | (r & 0xFF) << 8);
    }
    r = duel_0881657C(p, mask, 0);
    if (r >= 0) {
        return (unsigned short)(p & 0xFF | (r & 0xFF) << 8);
    }
    return -1;
}

/* Tributes needed to summon the card with uid: 0, 1, 2 or 3 (special). */
int duel_088078AC(unsigned int p, int uid, int a)
{
    int id = duel.list[uid].id;
    int lvl = duel_list_card_level(p, uid);

    switch (id) {
    case 0x1386:
    case 0x1387:
    case 0x1388:
    case 0x16EC:
    case 0x1725:
    case 0x1729:
    case 0x1985:
    case 0x1986:
    case 0x1B2E:
    case 0x1B2F:
        return 3;
    }
    if (lvl < 5) {
        return 0;
    }
    if (a != 0) {
        a = 0x40;
    } else {
        a = 0x10;
    }
    if (a & duel_08827DA4(p, uid, 2, 0)) {
        return id == 0x1864;
    }
    if (lvl < 7) {
        return 1;
    }
    return 2;
}

/* Are there enough tributes to summon the card with uid? */
int duel_088079F8(unsigned int p, int uid, int a)
{
    int id = duel.list[uid].id;
    int n = duel_088078AC(p, uid, 0);
    int i;
    int r;
    unsigned short buf[4];

    for (i = 0; i < n; i++) {
        r = duel_0880750C(p, id, a, i, buf);
        if (r == -1) {
            return 0;
        }
        buf[i] = r;
    }
    return 1;
}

int duel_08807AB4(unsigned int p)
{
    if (duel_find_effect_card(1 - p, 0x1256, -1) != 0) {
        return 0;
    }
    if (duel_find_effect_card(p, 0x144D, -1) != 0) {
        return 2;
    }
    return !(duel_hand_count_name(p, 0x144D) + duel_d_count_name(p, 0x144D) > 0);
}

int duel_08807B38(int id, int a, int b)
{
    switch (id) {
    case 0x1175:
    case 0x11BC:
    case 0x1653:
    case 0x1655:
    case 0x1793:
    case 0x1794:
    case 0x1799:
        return !a && b != 0;
    case 0x14A5:
    case 0x16F9:
    case 0x1946:
    case 0x12A2:
    case 0x13E3:
    case 0x17C5:
    case 0x197F:
    case 0x19FE:
    case 0x1AAA:
    case 0x1B2A:
        return 1;
    case 0x1965:
    case 0x1966:
    case 0x1968:
    case 0x1969:
        return !a;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original guarda o byte de a->player/a->zone em a1/a2 desde o início e o reaproveita nos case; p e a trocam s0/s1 */
/* Should the computer activate the card of a now? */
int duel_08807C7C(DuelAct *a)
{
    unsigned int pl = a->player;
    unsigned int p = pl ^ a->opp;
    int atk;
    int n;
    int z;
    unsigned int i;
    int v;

    duel_w_08913CF4 = p;
    switch (a->id) {
    case 0x10D6:
        return duel_count_monsters(p) >= 2;
    case 0x12A3:
        return !duel.players[pl & 1].field[a->zone].pos;
    case 0x1200:
        return duel_0884E948(a, 1 - pl) > 0;
    case 0x13E3:
        return duel_0880A54C(a, 1 - pl, -1, 0) >= 0;
    case 0x1657:
        if (duel_special.player[0] == duel_atk_tp) {
            return 0;
        }
        if (a->uid != CARD_UID(duel.players[pl & 1].field[a->zone].card)) {
            return 1;
        }
        if (pl == duel_special.player[0] && duel.players[duel_atk_tp & 1].field[duel_atk_tz].card.id != 0) {
            return 1;
        }
        return 0;
    case 0x154C: {
        DuelStats st;

        duel_0880546C(duel_atk_tp, duel_atk_tz, &st);
        return st.def < duel_field_atk(duel_special.player[0], duel_atk_zone) - 400;
    }
    case 0x14AB:
        return duel.players[pl & 1].lp >= 4500;
    case 0x19CB:
        if (duel_count_active(0x1398) != 0) {
            return 0;
        }
    case 0x1181:
        return !duel_0886C448(a);
    case 0x17E2:
        return duel_0884E948(a, 1 - pl) > 0;
    case 0x18E0:
        return duel_count_field_all(1 - p) > 0;
    case 0x1593:
        return !duel.players[pl & 1].field[10].card.id;
    case 0x1293:
    case 0x139D:
    case 0x1807:
    case 0x180B:
    case 0x180C:
    case 0x1333:
    case 0x1335:
    case 0x133C:
    case 0x133E:
    case 0x133F:
    case 0x1342:
    case 0x136A:
    case 0x152F:
    case 0x1612:
    case 0x17E5:
    case 0x17E6:
    case 0x183D:
    case 0x1867:
    case 0x18F4:
    case 0x1939:
    case 0x19C5:
    case 0x15B6:
    case 0x1919:
    case 0x194F:
    case 0x17C5:
        return duel_088BFFD8(p, a->id, duel_088AA224(a)) > 0;
    case 0x16CB:
    case 0x17E3:
    case 0x1826:
    case 0x1861:
    case 0x19D3:
    case 0x19D4: {
        DuelStats st;

        /* only when no monster of the opponent stands in the way */
        atk = duel_field_atk(a->player, a->zone);
        p = 1 - p;
        n = 0;
        for (z = 0; z <= 4; z++) {
            if (duel.players[p & 1].field[z].card.id != 0) {
                duel_0880546C(p, z, &st);
                v = duel.players[p & 1].field[z].pos ? st.def : st.atk;
                if (v < atk) {
                    return 1;
                }
                n++;
            }
        }
        return !n;
    }
    case 0x1967:
        for (i = 0; i < duel.players[p & 1].n_hand; i++) {
            if (!duel_08807B38(duel.players[p & 1].hand[i].id, 0, 0)) {
                continue;
            }
            return 1;
        }
        return 0;
    case 0x14C4:
    case 0x1A97:
        if (duel_088BFFD8(p, a->id, duel_088AA224(a)) == 0) {
            return 0;
        }
        return duel_hand_count_monsters(p) < 2;
    case 0x1984:
        if (duel.players[p & 1].n_d == 1) {
            return 1;
        }
        return duel_ai_088204A8(a, 0);
    case 0x175D:
        return duel_088159C8(a->player, duel_0884E4A0(a), duel_08817474) >= 0;
    case 0x1A6D:
    case 0x1A6E:
    case 0x1A6F:
        return 0;
    }
    return 1;
}
#endif

/* Has pick entry i been used already? */
int duel_08808364(int i)
{
    return duel_0884CBC4(i) > 0;
}

/* First unused pick entry with card id, or -1. */
int duel_08808384(int id)
{
    unsigned int i;

    for (i = 0; i < duel_w_0891524C.n; i++) {
        if (!duel_08808364(i) && duel_w_0891524C.c[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* First unused pick entry p does not hold yet and whose card is not limited, or -1. */
int duel_08808410(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel_w_0891524C.n; i++) {
        if (!duel_08808364(i)) {
            id = duel_w_0891524C.c[i].id;
            if (duel_0880AF74(p, id) == 0) {
                if (duel_0880AA04(id, 0) != 0) {
                    return i;
                }
                if (card_get_limit(id) < 3) {
                    return i;
                }
            }
        }
    }
    return -1;
}

/* A random unused pick entry with a card other than id (any unused one if there is none), or -1. */
int duel_088084E0(int id)
{
    unsigned int i;
    int n = 0;
    int k;

    for (i = 0; i < duel_w_0891524C.n; i++) {
        if (!duel_08808364(i) && duel_w_0891524C.c[i].id != id) {
            n++;
        }
    }
    if (n == 0) {
        for (i = 0; i < duel_w_0891524C.n; i++) {
            if (!duel_08808364(i)) {
                return i;
            }
        }
    } else {
        k = duel_rand(n);
        for (i = 0; i < duel_w_0891524C.n; i++) {
            if (!duel_08808364(i) && duel_w_0891524C.c[i].id != id) {
                if (k-- == 0) {
                    return i;
                }
            }
        }
    }
    return -1;
}

/* First unused pick entry with card id owned by owner, or -1. */
int duel_08808628(unsigned int owner, int id)
{
    unsigned int i;

    for (i = 0; i < duel_w_0891524C.n; i++) {
        if (!duel_08808364(i) && duel_w_0891524C.c[i].id == id && duel_w_0891524C.c[i].owner == owner) {
            return i;
        }
    }
    return -1;
}

/* Is pick entry i a good choice for the effect of card id? */
int duel_088086D4(int id, int i)
{
    switch (id) {
    case 0x137D:
    case 0x1366:
    case 0x15E6:
        switch (duel_w_0891524C.c[i].id) {
        case 0x1596:
            return 1;
        case 0x13C3:
            return id != 0x137D;
        case 0x1914:
            return !duel_count_faceup_name(0, 0x1784) && !duel_count_faceup_name(1, 0x1784);
        }
        break;
    case 0x1335:
        switch (duel_w_0891524C.c[i].id) {
        case 0x1835:
            return 1;
        }
        break;
    }
    return 0;
}
