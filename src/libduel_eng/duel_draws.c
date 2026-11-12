// @range 0x088C4864-0x088C77D4
// @asm 0x088C4864-0x088C5424
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
int duel_088C4730(unsigned int, int, int, int);
void duel_088AFD5C();

void duel_088C01F8(unsigned int p, int id, int a);
int duel_088C5510(unsigned int p, int n, int f);
void duel_088C5E78(unsigned int p, unsigned int n, int x, int y);
int duel_088DFCC8(unsigned int uid);
void duel_088C626C(unsigned int p, unsigned int uid, int a);
void duel_088CB728(int, int, int, unsigned int, int, int, int);
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

int card_attribute_icon_is3(int);
void duel_088C4448(unsigned int, int, int);
void duel_088C77D4(unsigned int p, unsigned short id, unsigned short a, unsigned short b);
#ifdef __MWERKS__
int duel_0884F30C(unsigned int a, unsigned short uid, DuelMoveEv ev);
#else
int duel_0884F30C(unsigned int code, int uid, int arg); /* the definition's; DuelMoveEv arguments go as their word */
#endif
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088C7E68(unsigned int, int, int, int);
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

int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_has_effect12_arg(unsigned int p, int z, int id, int arg);
int duel_count_monster_active(unsigned int p, int id);
void duel_088C44E4(unsigned int p, int id);
void duel_088C4508(unsigned int p, int id);
void duel_088C77EC(unsigned int p, int uid);
int duel_slot_effect_with(unsigned int, int, int, int);
void duel_088C8AD0(unsigned int, int, int, int);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
int duel_0886956C(int uid);

int duel_slot_has_src10(unsigned int, unsigned int, unsigned int, int);
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active);
int field_get_card(unsigned int, int);
unsigned short duel_slot_src13(unsigned int p, int z);
extern int duel_atk_direct;            /* 0x08913F18 */
extern int duel_atk_zone;              /* 0x08913F2C attacker zone */

typedef struct DuelDmgEv {             /* event argument of battle damage */
    unsigned short n;
    unsigned short p : 1;              /* attacker */
    unsigned short z : 4;
    unsigned short tp : 1;             /* damaged player */
    unsigned short tz : 4;
    unsigned short direct : 1;
    unsigned short pad : 5;
} DuelDmgEv;

#ifdef NON_MATCHING /* 7 palavras (eram 65): 1 - tp direto nos argumentos dos laços de 0x187D/0x130C/0x1720/0x14ED/0x187B (sem q: o mwcc cria o temporário depois do ponteiro de indução), card.id relido no 0x15D6 (or na ordem do original), pp antes de zz e contadores por busca; falta só o laço de 0x17B5 (q e contador externo trocados, s1/s3); busca aleatória e exaustiva das declarações não fecha */
/* battle damage of record r (r->a: attacker, r->b: target) with the cards that react to it */
void duel_088C4864(DuelSpecRec *r)
{
    int z3;
#ifdef __MWERKS__ /* declaração só para o matching; o port usa o protótipo do arquivo */
    extern int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int ev);
#endif
    extern int duel_slot_effect_with();
    unsigned int tp = r->b[0];
    int self = 0;
    int z4;
    int z2;
    unsigned int q;
    DuelSpecAct *act;
    int j;
    unsigned int n;
    unsigned int pp;
    unsigned short src;
    int z1;
    int zz;
    int k;
    unsigned int id;
    DuelDmgEv ev;

    if (r->a[0] == duel_special.player[0] && r->a[1] == duel_atk_zone) {
        self = 1;
    }
    act = &duel_special.act[self ? duel_special.player[0] : 1 - duel_special.player[0]];
    id = act->id;
    n = r->w0;
    if (n == 0) {
        return;
    }
    ev.n = n > 0xFFFF ? 0xFFFF : n;
    ev.p = r->a[0];
    ev.z = r->a[1];
    ev.tp = r->b[0];
    ev.tz = r->b[1];
    ev.direct = duel_atk_direct;
    duel_cmd((tp ? 0x8000 : 0) | 0x25, ev.n, 1, id);
    if (ev.p != ev.tp) {
        switch (id) {
        case 0x1814:
            if (duel_atk_direct) {
                duel_088C87A8(r->a[0], r->a[1], 0x1814, 0x22, 0);
            }
            break;
        case 0x1596:
        case 0x1598:
        case 0x15D0:
        case 0x182D:
        case 0x19CF:
        case 0x1A1F:
            if (!duel_atk_direct) {
                break;
            }
        case 0x1018:
        case 0x1082:
        case 0x11B1:
        case 0x12A7:
        case 0x139F:
        case 0x13B2:
        case 0x1480:
        case 0x14FF:
        case 0x1501:
        case 0x1502:
        case 0x1504:
        case 0x1506:
        case 0x1522:
        case 0x1531:
        case 0x1532:
        case 0x1563:
        case 0x1564:
        case 0x15B9:
        case 0x15DE:
        case 0x161E:
        case 0x1656:
        case 0x1685:
        case 0x1686:
        case 0x168B:
        case 0x16BD:
        case 0x1746:
        case 0x178A:
        case 0x178C:
        case 0x183C:
        case 0x1891:
        case 0x1967:
        case 0x19CA:
            duel_0884F30C((unsigned short)id | ((r->a[0] << 31 & 0x80000000) | 0x1E200000 | (r->a[1] << 16 & 0x1F0000)),
                          CARD_UID(duel.players[r->a[0] & 1].field[r->a[1]].card), *(unsigned int *)&ev);
            break;
        case 0xFCB:
            if (!ev.direct && duel_special.act[1 - r->a[0]].unk8) {
                for (z1 = 5; z1 < 10; z1++) {
                    if (duel_slot_e8170(1 - tp, z1, 0x187D)) {
                        duel_0884F30C(((1 - tp) << 31 & 0x80000000 | 0x1E200000) | ((unsigned int)z1 << 16 & 0x1F0000)
                                      | duel.players[(1 - tp) & 1].field[z1].card.id, CARD_UID(duel.players[(1 - tp) & 1].field[z1].card), *(unsigned int *)&ev);
                    }
                }
            }
            break;
        }
        for (z1 = 5; z1 < 10; z1++) {
            if (duel_slot_e8170(1 - tp, z1, 0x130C)) {
                duel_0884F30C(((1 - tp) << 31 & 0x80000000 | 0x1E200000) | ((unsigned int)z1 << 16 & 0x1F0000)
                              | duel.players[(1 - tp) & 1].field[z1].card.id, CARD_UID(duel.players[(1 - tp) & 1].field[z1].card), *(unsigned int *)&ev);
            }
        }
        for (z2 = 5; z2 < 10; z2++) {
            if (duel_slot_e8170(1 - tp, z2, 0x1720)) {
                duel_0884F30C(((unsigned int)(1 - tp) << 31 & 0x80000000 | 0x1E200000) | ((unsigned int)z2 << 16 & 0x1F0000)
                              | duel.players[(1 - tp) & 1].field[z2].card.id, CARD_UID(duel.players[(1 - tp) & 1].field[z2].card), *(unsigned int *)&ev);
            }
        }
    }
    if (duel_slot_effect_with(r->a[0], r->a[1], 0x1511, ev.tp)) {
        duel_0884F30C((ev.p << 31 & 0x80000000) | 0x1E501511, 0, *(unsigned int *)&ev);
    }
    for (z1 = 5; z1 < 10; z1++) {
        if (duel_slot_e8170(1 - tp, z1, 0x14ED) && duel_slot_has_src10(1 - tp, z1, r->a[0], r->a[1])) {
            duel_0884F30C(((unsigned int)(1 - tp) << 31 & 0x80000000 | 0x1E200000) | ((unsigned int)z1 << 16 & 0x1F0000)
                          | duel.players[(1 - tp) & 1].field[z1].card.id, CARD_UID(duel.players[(1 - tp) & 1].field[z1].card), *(unsigned int *)&ev);
        }
    }
    if (act->pad14[4] == 11) {
        for (k = 5; k < 10; k++) {
            if (duel_slot_e8170(1 - tp, k, 0x187B)) {
                duel_0884F30C(((1 - tp) << 31 & 0x80000000 | 0x1E200000) | ((unsigned int)k << 16 & 0x1F0000)
                              | duel.players[(1 - tp) & 1].field[k].card.id, CARD_UID(duel.players[(1 - tp) & 1].field[k].card), *(unsigned int *)&ev);
            }
        }
    }
    src = duel_slot_src_attr8(r->a[0], r->a[1], 1);
    if (src != 0xFFFF) {
        pp = src & 0xFF;
        zz = src >> 8 & 0xFF;
        switch (id = duel.players[pp & 1].field[zz].card.id) {
        case 0x15D6:
        case 0x15D7:
            if (pp != ev.tp) {
                duel_0884F30C((pp << 31 & 0x80000000 | 0x1E200000 | (zz << 16 & 0x1F0000)) | duel.players[pp & 1].field[zz].card.id,
                              CARD_UID(duel.players[pp & 1].field[zz].card), *(unsigned int *)&ev);
            }
            break;
        }
    }
    if (!(duel.players[r->b[0] & 1].field[r->b[1]].flags >> 5 & 1)) {
        switch (field_get_card(r->b[0], r->b[1])) {
        case 0x1281:
            if (duel_slot_src13(r->b[0], r->b[1]) != 0xFFFF) {
                duel_088C4448(r->b[0], r->b[1], CARD_UID(duel.players[r->b[0] & 1].field[r->b[1]].card));
                duel_088C4730(1 - r->b[0], ev.n, 1, 0x1281);
            }
        }
    }

    if (duel_atk_direct) {
        for (k = 0; k < 2; k++) {
            q = duel.turn_player ^ k;
            for (z3 = 5; z3 < 10; z3++) {
                if (duel_slot_e8170(q, z3, 0x17B5)) {
                    duel_0884F30C((q << 31 & 0x80000000 | 0x1E200000) | ((unsigned int)z3 << 16 & 0x1F0000) | 0x17B5,
                                  CARD_UID(duel.players[q & 1].field[z3].card), *(unsigned int *)&ev);
                }
            }
        }
    }
    for (j = 0; j < 2; j++) {
        q = duel.turn_player ^ j;
        for (z4 = 5; z4 < 10; z4++) {
            if (duel_slot_e8170(q, z4, 0x171E)) {
                duel_0884F30C((q << 31 & 0x80000000 | 0x1E200000) | ((unsigned int)z4 << 16 & 0x1F0000) | 0x171E,
                              CARD_UID(duel.players[q & 1].field[z4].card), *(unsigned int *)&ev);
            }
        }
    }
    duel_088AFD5C(0xF, *(unsigned int *)&ev);
}
#endif

/* 0x088C5E78, 0x088C626C, 0x088C6BC8: kept in asm. */

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

/* p draws n cards (f: drawn by an effect; 0x1209: the effect of that card); 0 when the deck runs short
   or the draw is stopped */
int duel_088C5510(unsigned int p, int n, int f)
{
    int e1 = duel_slot_has_effect(p, 0xB, 0x123B);
    int e2 = duel_slot_has_effect(p, 0xB, 0x188C);
    int e3 = duel_slot_has_effect(p, 0xB, 0x18D5);
    int is1209 = f == 0x1209;
    int any = is1209 | e1 | e2 | e3;
    int lack = (unsigned int)duel.players[p & 1].n_d < (unsigned int)n;
    unsigned int o;
    int i, z;
    DuelMoveEv ev, ev2;

    if (n <= 0) {
        return 0;
    }
    if (duel.phase != 0) {
        if (duel_count_monster_active(1 - p, 0x178B)) {
            duel_088C448C(1 - p, 0x178B);
            return 0;
        }
    } else if (p == duel.turn_player && duel_slot_has_effect12_arg(p, 0xB, 0x1885, 0)) {
        duel_088C44E4(1 - p, 0x1885);
        return 0;
    }
    duel_088C7BA4();
    for (i = 0; i < n; i++) {
        DuelCard *c = &duel.players[p & 1].zone_d[i];

        duel_cmd((p ? 0x8000 : 0) | 0x55, f != 0, 1, 0);
        if (lack) {
            continue;
        }
        ev.player = p;
        ev.b10 = f != 0;
        ev.uid = CARD_UID(*c);
        duel_088AFD5C(0x19, ev);
        if (any) {
            int id = c->id;

            duel_088C4508(p, id);
            duel_088C77EC(p, CARD_UID(*c));
            if (e1 && card_is_monster(id) && card_get_atk(id) >= 1500) {
                duel_088C77D4(p, 0x123B, 2, CARD_UID(*c));
            }
            if (e2 && card_is_monster(id) && card_get_atk_q(id) <= 1500
                && !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(*c))) {
                duel_088C77D4(p, 0x188C, 2, CARD_UID(*c));
            }
            if (is1209 && card_get_type(id) == 22) {
                duel_088C77D4(p, 0x1209, 2, CARD_UID(*c));
            }
        }
        switch (c->id) {
        case 0x12A1:
            if (duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(*c))
#ifdef __MWERKS__
                && !duel_0884F30C((p << 31 & 0x80000000 | 0x32500000) | c->id, CARD_UID(*c), ev)) {
#else
                && !duel_0884F30C((p << 31 & 0x80000000 | 0x32500000) | c->id, CARD_UID(*c), *(int *)&ev)) {
#endif
                duel_088C8AD0(p, 0xB, 0x12A1, CARD_UID(*c));
            }
            break;
        case 0x17CC:
            if (f) {
#ifdef __MWERKS__
                duel_0884F30C((p << 31 & 0x80000000 | 0x32500000) | (c->id & 0xFFFF), CARD_UID(*c), ev);
#else
                duel_0884F30C((p << 31 & 0x80000000 | 0x32500000) | (c->id & 0xFFFF), CARD_UID(*c), *(int *)&ev);
#endif
            }
            break;
        }
    }
    duel_088C7CF0();
    if (lack) {
        return 0;
    }
    duel_088C01F8(p, 0x1405, 1);
    o = 1 - p;
    for (i = 5; i < 10; i++) {
        if (duel_slot_e8170(o, i, 0x1353)) {
            ev2.player = p;
            ev2.b10 = 1;
            ev2.uid = 0;
            duel_0884F30C((o << 31 & 0x80000000 | 0x32200000) | ((unsigned int)i << 16 & 0x1F0000)
#ifdef __MWERKS__
                          | duel.players[o & 1].field[i].card.id, CARD_UID(duel.players[o & 1].field[i].card), ev2);
#else
                          | duel.players[o & 1].field[i].card.id, CARD_UID(duel.players[o & 1].field[i].card), *(int *)&ev2);
#endif
        }
    }
    {
    /* the rest passes no event */
#ifdef __MWERKS__ /* declaração só para o matching; o port usa o protótipo do arquivo */
    extern int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int ev);
#endif

    if (f) {
        for (i = 0; i < 2; i++) {
            for (z = 5; z < 10; z++) {
                if (duel.players[i & 1].field[z].card.id == 0x1802 && duel.players[i & 1].field[z].faceup
                    && !(duel.players[i & 1].field[z].flags >> 1 & 1)) {
                    duel_088C87A8(i, z, 0x1802, 1, n << (p << 3));
                }
            }
        }
    }
    if (!f && duel_slot_has_effect(p, 0xB, 0x1911)) {
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(p, z, 0x1911)) {
                duel_0884F30C((p << 31 & 0x80000000 | 0x32200000) | ((unsigned int)z << 16 & 0x1F0000) | 0x1911,
                              CARD_UID(duel.players[p & 1].field[z].card), 0);
            }
        }
    }
    if (duel.phase == 0 && duel.turn_player == p) {
        for (z = 5; z < 10; z++) {
            if (duel_slot_e8170(p, z, 0x16CD) && !(duel.players[p & 1].field[z].flags >> 4 & 1)
                && duel_0884F30C((p << 31 & 0x80000000 | 0x200000) | ((unsigned int)z << 16 & 0x1F0000) | 0x16CD,
                                 CARD_UID(duel.players[p & 1].field[z].card), 0)) {
                duel_088C7E68(p, z, 4, 1);
            }
        }
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            int uid = CARD_UID(duel.players[p & 1].grave[i]);

            if (duel.players[p & 1].grave[i].id == 0x1A61 && !duel.players[p & 1].grave[i].b21 && !duel_0886956C(uid)) {
                duel_0884F30C((p << 31 & 0x80000000) | 0x4E1A61, uid, 0);
            }
        }
    }
    for (z = 0; z <= 4; z++) {
        if (duel_slot_e8170(o, z, 0x1817)) {
            duel_088C87A8(o, z, 0x1817, 3, 0);
        }
    }
    }
    return 1;
}

int duel_088C5E70(unsigned int p, int n)
{
    return duel_088C5510(p, n, 1);
}

/* the first n cards of zone D of p to the graveyard (x: for the opponent; y: banished, zone 0xF) */
void duel_088C5E78(unsigned int p, unsigned int n, int x, int y)
{
    int m = duel.players[p & 1].n_d;
    int i;

    if (n < (unsigned int)m) {
        m = n;
    }
    duel_cmd((p ? 0x8000 : 0) | 0x56, m, y | duel_dfc10(), x);
    duel_088C7BA4();
    for (i = 0; i < m; i++) {
        DuelCard *c = &duel.players[p & 1].zone_d[i];
        unsigned int id = c->id;
        DuelMoveEv ev;
        int z;

        z = y ? 0xF : 0xE;
        if (!y) {
            if (duel_088DFCC8(CARD_UID(duel.players[p & 1].zone_d[i])) == 0) {
                z = 0xF;
            }
        }
        ev.opp = p ^ x;
        ev.player = p;
        ev.zone = z;
        ev.uid = CARD_UID(*(DuelCard *)c);
        ev.b10 = 0;
        if (z == 0xE) {
            switch (id) {
            case 0x14A5:
            case 0x16F9:
                duel_088C4448((int)c->owner, 0xE, CARD_UID(*c));
                duel_088C77D4((int)c->owner, duel.players[p & 1].zone_d[i].id, 1, CARD_UID(*c));
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
#ifdef __MWERKS__
                duel_0884F30C((p << 31 & 0x80000000 | 0x384E0000) | duel.players[p & 1].zone_d[i].id, CARD_UID(*c), ev);
#else
                duel_0884F30C((p << 31 & 0x80000000 | 0x384E0000) | duel.players[p & 1].zone_d[i].id, CARD_UID(*c), *(int *)&ev);
#endif
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
   (perm_stmts) não resolve; nem tipo de k, if/else, k pré-mascarado, argumento num local antes/depois dos stores,
   associação do | (2026-11-08) */
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
#ifdef __MWERKS__
        duel_0884F30C((o << 31 & 0x80000000 | 0x384E0000) | (id & 0xFFFF), uid, ev);
#else
        duel_0884F30C((o << 31 & 0x80000000 | 0x384E0000) | (id & 0xFFFF), uid, *(int *)&ev);
#endif
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

int duel_088C6A10(unsigned int p, unsigned short id, int x)
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

#ifdef NON_MATCHING
extern unsigned int duel_turn_player;
extern unsigned int duel_phase;
int card_is_attribute9(int);
int card_is_in_group_1911(int id);
int card_is_monster(int id);
int card_attribute_icon_is3(int);
int duel_card_protected(int id, int uid);
int duel_d_find_name(unsigned int, int);

/* 54 palavras (2026-11-09), só registradores: o parâmetro i (não usado) é o "grave" do original (75 -> 54, c volta
   a s2); falta a rotação p/ban/grave (original s3/s4/s5, aqui s4/s5/s3): 720 ordens das declarações, tipos de
   ban/i/p, cópia de p num local e grave = 0 em outras posições não mudam nada. Resolveram a forma: !ban materializado num bloco,
   (unsigned short) no 3º argumento de duel_088C77D4, (unsigned int)q << 31, (x & 0xF) << 8 | ev.zone */
/* card c (index i) of zone z of p banished or sent to the graveyard (x: by the opponent's effect), with the
   triggers of the cards that watch it */
void duel_088C6BC8(unsigned int p, int z, int i, DuelCard *c, int x)
{
    unsigned int id;
    unsigned int o;
    int f;
    int ban;
    int k;
    int q;
    DuelMoveEv ev;

    id = c->id;
    o = (int)c->owner;
    ban = !duel_088DFCC8(CARD_UID(*c));
    i = 0;
    {
        int u = !ban;
        if (u) {
            int t = !card_is_attribute9(id);
            if (t) {
                i = 1;
            }
        }
    }
    f = 0;
    {
        char *d = (char *)&ev;
        int n = 4;

        if (d) {
            do {
                *d++ = 0;
            } while (--n);
        }
    }
    ev.uid = CARD_UID(*c);
    ev.player = p;
    ev.b10 = 1;
    ev.opp = p ^ x;
    ev.zone = ban ? 0xF : 0xE;
    duel_cmd((p ? 0x8000 : 0) | 0x31, z, (x & 0xF) << 8 | ev.zone, ev.uid);
    switch (id) {
    case 0x106D:
        if (!ban && x && z == 0xD) {
            f = 1;
        }
        break;
    case 0x1793:
    case 0x1794:
    case 0x1799:
        if (!ban && x && z == 0xB && o == p) {
            f = 1;
        }
        break;
    case 0x179A:
    case 0x1828:
    case 0x1946:
        if (!ban && z == 0xB) {
            f = 1;
        }
        break;
    case 0x1653:
    case 0x1655:
        if (!ban && x && o == p) {
            f = 1;
        }
        break;
    case 0x1185:
    case 0x12A2:
    case 0x1309:
    case 0x13E3:
    case 0x1687:
    case 0x16F5:
    case 0x17C5:
    case 0x19FE:
    case 0x14E9:
    case 0x163F:
    case 0x1A8F:
        if (!ban) {
            f = 1;
        }
        break;
    case 0x1A62:
        if (!ban) {
            duel_088C77D4(o, id, (unsigned short)((unsigned char)((o == duel_turn_player) + 1) << 4 | 1), CARD_UID(*c));
        }
        break;
    }
    if (id == 0x18AA && duel_d_find_name(p, 0x1445) >= 0) {
#ifdef __MWERKS__
        duel_0884F30C((p << 31 & 0x80000000) | 0x3C501445, 0, ev);
#else
        duel_0884F30C((p << 31 & 0x80000000) | 0x3C501445, 0, *(int *)&ev);
#endif
    }
    if (i && card_is_monster(id) && card_is_in_group_1911(id)) {
        for (q = 0; q < 2; q++) {
            if (duel_slot_e8170(q, 10, 0x169F)) {
                duel_0884F30C(((unsigned int)q << 31 & 0x80000000) | 0x3C000000 | ((o != q) << 24 & 0x1000000) | 0x2A169F,
#ifdef __MWERKS__
                              CARD_UID(duel.players[q & 1].field[10].card), ev);
#else
                              CARD_UID(duel.players[q & 1].field[10].card), *(int *)&ev);
#endif
            }
        }
    }
    if (i && card_attribute_icon_is3(id)) {
        for (q = 0; q < 2; q++) {
            if (duel_slot_e8170(q, 10, 0x187F)) {
                duel_0884F30C(((unsigned int)q << 31 & 0x80000000) | 0x3C000000 | ((o != q) << 24 & 0x1000000) | 0x2A187F,
#ifdef __MWERKS__
                              CARD_UID(duel.players[q & 1].field[10].card), ev);
#else
                              CARD_UID(duel.players[q & 1].field[10].card), *(int *)&ev);
#endif
            }
        }
    }
    if (card_get_type(id) == 9 && i) {
        for (q = 0; q <= 4; q++) {
            if (duel_slot_e8170(o, q, 0x1A8D)) {
                duel_088C7E68(o, q, 4, 1);
            }
        }
    }
    q = !duel_card_protected(id, CARD_UID(*c));
    switch (id) {
    case 0x14A5:
    case 0x16F9:
        if (q && i) {
            duel_088C4448(o, 0xE, CARD_UID(*c));
            duel_088C77D4(o, id, 1, CARD_UID(*c));
        }
        break;
    case 0x1466:
        if (p == o && i && x) {
            duel_088C77D4(p, c->id, 1, CARD_UID(*c));
        }
        break;
    case 0x1522:
        if (p == o && i && x) {
            k = o == duel_turn_player ? (duel_phase != 0) << 1 : 1;
            duel_088C77D4(o, id, (unsigned short)((k & 0xF) << 4 | 1), CARD_UID(*c));
        }
        break;
    case 0x185C:
        if (i) {
            k = o == duel_turn_player ? (duel_phase != 0) << 1 : 1;
            duel_088C77D4(o, id, (unsigned short)((k & 0xF) << 4 | 1), CARD_UID(*c));
        }
        break;
    }
    if (f) {
#ifdef __MWERKS__
        duel_0884F30C((o << 31 & 0x80000000 | 0x3C4E0000) | (id & 0xFFFF), CARD_UID(*c), ev);
#else
        duel_0884F30C((o << 31 & 0x80000000 | 0x3C4E0000) | (id & 0xFFFF), CARD_UID(*c), *(int *)&ev);
#endif
    }
    duel_088AFD5C(0x1E, ev);
}
#endif

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
