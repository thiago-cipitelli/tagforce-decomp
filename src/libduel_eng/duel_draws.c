// @range 0x088C4864-0x088C77D4
// @asm 0x088C4864-0x088C5424
// @asm 0x088C5510-0x088C5E70
// @asm 0x088C5E78-0x088C625C
// @asm 0x088C626C-0x088C6508
// @asm 0x088C6BC8-0x088C7478
/* LP payment, drawing cards, sending cards from the deck and the D zone, with their commands to the
   presentation. The long step functions are kept in asm. */
#include "duel.h"
#include "card_db.h"

typedef struct DuelLpEv {
    unsigned short n;
    unsigned char player : 1;
    unsigned char b2_1 : 7;
    unsigned char pad3;
} DuelLpEv;

extern int duel_w_08919624;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_find_effect_card(unsigned int p, int id, int z);
void duel_088C448C(int p, int a);
int duel_088C4730(unsigned int p, int n, int f, int id);
void duel_088AFD5C();

void duel_088C01F8(unsigned int p, int id, int a);
int duel_088C5510(unsigned int p, int n, int f);
void duel_088C5E78(unsigned int p, unsigned int n, int x, int y);
int duel_088DFCC8(unsigned int uid);
void duel_088C626C(unsigned int p, unsigned int uid, int a);
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
DuelCard *duel_0884CCBC(int k);
void duel_088C6BC8(unsigned int p, int z, int i, DuelCard *c, int a);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);

typedef struct DuelMoveEv {            /* event argument of a card moving (see duel_hand.c) */
    unsigned short uid : 9;
    unsigned short player : 1;
    unsigned short b10 : 1;
    unsigned short opp : 1;
    unsigned short zone : 4;
    unsigned short pad;
} DuelMoveEv;

int card_attribute_icon_is3(int id);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
int duel_0884F30C(unsigned int a, unsigned short uid, DuelMoveEv ev);
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
unsigned int duel_dfc10(void);
int card_is_monster(int id);
int duel_locks_ok(int p, int monster);
int duel_d_count_name(unsigned int p, unsigned short id);
int duel_c_count_name(unsigned int p, unsigned short id);
void duel_088C67F0(unsigned int p, int id, int all, int x);
int duel_pick_st_zone(unsigned int p);
void duel_088C1618(unsigned int p);
void duel_088D1438(unsigned int p, int z, int a, int b);

/* 0x088C4864, 0x088C5510, 0x088C5E78, 0x088C626C, 0x088C6BC8: kept in asm. */

void duel_088C5424(unsigned int p, int n)
{
    DuelLpEv ev;

    if (n != 0) {
        if (duel_find_effect_card(1 - p, 0x1256, -1) != 0) {
            duel_088C448C(1 - p, 0x1256);
            duel_088C4730(p, n, 1, 0x1256);
        } else {
            if (n > 0xFFFF) {
                n = 0xFFFF;
            }
            duel_cmd((p ? 0x8000 : 0) | 0x24, n, 4, 0);
            ev.player = p;
            ev.n = n;
            duel_088AFD5C(0x11, ev);
            duel_088C01F8(p, 0x144D, 1);
        }
    }
}

int duel_088C5E70(unsigned int p, int n)
{
    return duel_088C5510(p, n, 1);
}

#ifdef NON_MATCHING
/* 26 palavras: $s2/$s3 trocados entre id e z, e o original lê o uid de ev por c e o id do switch pela base
   (o contrário daqui); as formas que trocam isso fazem o mwcc reaproveitar o uid entre ev e as chamadas */
/* the first n cards of zone D of p to the graveyard (x: for the opponent; y: banished, zone 0xF) */
void duel_088C5E78(unsigned int p, unsigned int n, int x, int y)
{
    int m = duel.players[p & 1].n_d;
    int i;
    int z;

    if (n < (unsigned int)m) {
        m = n;
    }
    duel_cmd((p ? 0x8000 : 0) | 0x56, m, y | duel_dfc10(), x);
    duel_088C7BA4();
    for (i = 0; i < m; i++) {
        DuelCard *c = &duel.players[p & 1].zone_d[i];
        unsigned int id = c->id;
        DuelMoveEv ev;

        z = y ? 0xF : 0xE;
        if (!y) {
            if (duel_088DFCC8(CARD_UID(duel.players[p & 1].zone_d[i])) == 0) {
                z = 0xF;
            }
        }
        ev.opp = p ^ x;
        ev.player = p;
        ev.zone = z;
        ev.uid = CARD_UID(duel.players[p & 1].zone_d[i]);
        ev.b10 = 0;
        if (z == 0xE) {
            switch (id) {
            case 0x14A5:
            case 0x16F9:
                duel_088C4448((int)c->owner, 0xE, CARD_UID(*c));
                duel_088C77D4((int)c->owner, c->id, 1, CARD_UID(*c));
                break;
            case 0x106D:
            case 0x1653:
            case 0x1655:
                if (x == 0) {
                    break;
                }
            case 0x1185:
            case 0x12A2:
            case 0x1309:
            case 0x13E3:
            case 0x1687:
            case 0x16F5:
            case 0x17C5:
            case 0x19FE:
                duel_0884F30C((p << 31 & 0x80000000 | 0x384E0000) | c->id, CARD_UID(*c), ev);
                break;
            }
            if (card_get_type(id) == 9) {
                for (z = 0; z <= 4; z++) {
                    if (duel_slot_e8170((int)c->owner, z, 0x1A8D)) {
                        duel_088C7E68((int)c->owner, z, 4, 1);
                    }
                }
            }
        }
        duel_088AFD5C(0x1C, ev);
    }
    duel_088C7CF0();
}
#endif

void duel_088C625C(unsigned int p, int n, int x)
{
    duel_088C5E78(p, n, x, 0);
}

void duel_088C6264(unsigned int p, int n, int x)
{
    duel_088C5E78(p, n, x, 1);
}

#ifdef NON_MATCHING
/* 13 palavras: o original faz os stores de ev.opp antes de mascarar k (`k & 0x1F`); a ordem dos 5 stores de ev
   (perm_stmts) não resolve */
/* card uid of zone D of p to the graveyard of its owner (x: for the opponent) */
void duel_088C626C(unsigned int p, unsigned int uid, int x)
{
    unsigned int id = duel.list[uid].id;
    unsigned int o = uid & 1;
    int k = card_attribute_icon_is3(id) ? 0xC : 0xD;
    int i;
    DuelMoveEv ev;

    ev.opp = p ^ x;
    ev.player = p;
    ev.zone = 0xE;
    ev.uid = uid;
    ev.b10 = 0;
    duel_cmd(0x3F, (uid & 0x3FF) << 6 | ((k & 0x1F) << 1 | (p & 1)), (o & 1) | 0x5C, x != 0);
    switch (id) {
    case 0x14A5:
    case 0x16F9:
        duel_088C4448(o, 0xE, uid);
        duel_088C77D4(o, id, 1, uid);
        break;
    case 0x106D:
    case 0x1653:
    case 0x1655:
        if (x == 0) {
            break;
        }
    case 0x1185:
    case 0x12A2:
    case 0x1309:
    case 0x13E3:
    case 0x1687:
    case 0x16F5:
    case 0x17C5:
    case 0x19FE:
        duel_0884F30C((o << 31 & 0x80000000 | 0x384E0000) | (id & 0xFFFF), uid, ev);
        break;
    }
    if (card_get_type(id) == 9) {
        for (i = 0; i <= 4; i++) {
            if (duel_slot_e8170(o, i, 0x1A8D)) {
                duel_088C7E68(o, i, 4, 1);
            }
        }
    }
    duel_088AFD5C(0x1C, ev);
}
#endif

/* sends the cards named like id of p from zone D (his own) and zone C to the graveyard (only the first one
   unless all); just shows them when the locks forbid it */
void duel_088C6508(unsigned int p, int id, int all, int x)
{
    unsigned int i;
    int n = 0;

    if (duel_locks_ok(p, card_is_monster(id)) == 0) {
        duel_088C67F0(p, id, all, x);
        return;
    }
    if (duel_d_count_name(p, id) + duel_c_count_name(p, id) == 0) {
        return;
    }
    duel_088C7BA4();
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(duel.players[p & 1].zone_d[i].id, id)) {
            DuelCard *c = &duel.players[p & 1].zone_d[i];

            if ((c->owner ^ p) == 0) {
                if (!all && n) {
                    break;
                }
                n++;
                duel_088C626C(p, CARD_UID(*c), x);
            }
        }
    }
    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        if (card_same_name(duel.players[p & 1].zone_c[i].id, id)) {
            DuelCard *c = &duel.players[p & 1].zone_c[i];

            if (!all && n) {
                break;
            }
            n++;
            duel_088C626C(p, CARD_UID(*c), x);
        }
    }
    duel_088C7CF0();
}


void duel_088C6728(unsigned int p, unsigned int uid, int a)
{
    unsigned int id = duel.list[uid].id;

    if (duel_088DFCC8(uid) == 0) {
        int k = card_attribute_icon_is3(id) ? 0xC : 0xD;
        int f = a != 0;

        duel_cmd((p ? 0x8000 : 0) | 0x33, k, f, uid);
        duel_088AFD5C(0x24, 0);
    } else {
        duel_088C626C(p, uid, a);
    }
}

/* shows the cards named like id in zone D, then zone C, of p (only the first one unless all) */
void duel_088C67F0(unsigned int p, int id, int all, int x)
{
    int n = 0;
    unsigned int i;
    int f;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(duel.players[p & 1].zone_d[i].id, id)) {
            DuelCard *c = &duel.players[p & 1].zone_d[i];

            if (!all && n) {
                break;
            }
            n++;
            f = x != 0;
            duel_cmd((p ? 0x8000 : 0) | 0x33, 0xD, f, CARD_UID(*c));
        }
    }
    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        if (card_same_name(duel.players[p & 1].zone_c[i].id, id)) {
            DuelCard *c = &duel.players[p & 1].zone_c[i];

            if (!all && n) {
                break;
            }
            n++;
            f = x != 0;
            duel_cmd((p ? 0x8000 : 0) | 0x33, 0xC, f, CARD_UID(*c));
        }
    }
    if (n) {
        duel_088AFD5C(0x24, 0);
    }
}

void duel_088C69EC(unsigned int p, int a, int b)
{
    duel_cmd((p ? 0x8000 : 0) | 0x33, 0xD, b, a);
}

int duel_088C6A10(unsigned int p, int id, int x)
{
    unsigned int i;
    int r;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        r = !card_same_name(duel.players[p & 1].zone_d[i].id, (unsigned short)id);
        if (!r) {
            duel_088CB728(p, 0xD, CARD_UID(duel.players[p & 1].zone_d[i]), p, x, 1, 0);
            return 1;
        }
    }
    return 0;
}

void duel_088C6B24(unsigned int p)
{
    int i;

    for (i = duel_w_08919624 - 1; i >= 0; i--) {
        unsigned int o = duel_0884CCBC(i + 1)->owner;
        DuelCard *c = duel_0884CCBC(i + 1);
        unsigned int uid = o + (c->uid_hi << 1);

        duel_cmd((p ? 0x8000 : 0) | 0x57, 0xD, uid, 0);
    }
}

void duel_088C7478(unsigned int p, int i, DuelCard *c, int a)
{
    duel_088C6BC8(p, 0xB, i, c, a);
}

void duel_088C7490(unsigned int p, int i, int a)
{
    duel_088C6BC8(p, 0xD, i, duel_field_card_deb2c(p, 0xD, i), a);
}

void duel_088C74EC(unsigned int p, int i, int a)
{
    duel_088C6BC8(p, 0xC, i, duel_field_card_deb2c(p, 0xC, i), a);
}

/* card i of the hand of p to a spell/trap zone (face down when set); returns the zone */
int duel_088C7548(unsigned int p, int i, int set)
{
    int z = duel_pick_st_zone(p);
    unsigned int id = duel.players[p & 1].hand[i].id;
    struct DuelActX ev;

    if (card_get_type(id) == 0x16 && card_get_prop5_hi(id) == 2) {
        if (duel.players[p & 1].field[10].card.id != 0) {
            duel_088D1438(p, 10, 0, 0);
        }
        z = 10;
    }
    duel_088C1618(p);
    duel_cmd((p ? 0x8000 : 0) | 0x50, id, (set & 1) << 14 | ((p & 1) | (z & 0x1F) << 1),
             CARD_UID(duel.players[p & 1].hand[i]));
    if (set) {
        if (card_get_type(id) == 0x17) {
            duel_088C77D4(p, 0x198A, 1, 1);
        }
    } else if (!set) {
        ev.s_uid = CARD_UID(duel.players[p & 1].hand[i]);
        ev.s_player = p;
        ev.s_zone = i;
        ev.s_b14 = 0;
        ev.s_b15 = 0;
        ev.s_b16 = 0;
        ev.s_b17 = 0;
        duel_088AFD5C(10, ev);
    }
    return z;
}
