// @range 0x08820618-0x08824D34
// @asm 0x08820E08-0x08820F60
// @asm 0x08821124-0x08821888
// @asm 0x08822000-0x088230CC
// @asm 0x088230CC-0x088239EC
// @asm 0x088239EC-0x08824798
// @data 0x089078E8-0x08907908
/* The computer player's choice of a card to activate (second part): the tables of cards checked
   on the field and in the hand, and the step machine that goes through them. */
#include "duel.h"

typedef struct DuelAiCard {            /* 8 */
    unsigned int id;
    int (*cond)(DuelAct *a, DuelAct *b);
} DuelAiCard;

extern unsigned int duel_ai_player;    /* 0x08919648 */
extern int duel_w_0891963C;
extern int duel_w_08913EC4;
extern unsigned int duel_w_08913CFC;            /* step of duel_ai_step_08820E08 */
extern unsigned int duel_turn_player;
extern DuelAiCard duel_ai_field_cards[0x3D];   /* 0x089065E8 */
extern DuelAiCard duel_ai_summons[0xB6];       /* 0x089067D0 */
extern DuelAiCard duel_ai_cards_08906D80[4];
extern DuelAiCard duel_ai_cards_08906DA0[5];
extern DuelAiCard duel_ai_monster_cards[0x103]; /* 0x08906DC8 */
extern DuelAiCard duel_ai_st_cards[0x57];      /* 0x089075E0 */

int field_get_card(unsigned int p, int z);
int duel_0886981C(DuelAct *a, DuelAct *b);
int duel_08869ACC(DuelAct *a, DuelAct *b);
int duel_ai_0882054C(DuelAct *a);
void duel_08835F34(unsigned int p, int z, int i, int d);
int duel_hand_find_name(unsigned int p, int id);
int duel_0880AFC4(unsigned int p);
int card_is_monster(int id);
int duel_count_monsters(unsigned int p);
int duel_count_name(unsigned int p, int id);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_088079F8(unsigned int p, int uid, int b);
int duel_grave_count_monsters(unsigned int p);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
int duel_0880A8AC(unsigned int p, unsigned int q);
int duel_088BFFD8(unsigned int p, int id, int i);
unsigned int duel_08828740(unsigned int p, int uid);
int duel_088BFF78(unsigned int p, int id, int b);
int duel_0884C824(void);
unsigned int duel_0884C830(int i);
DuelCard *duel_0884C7EC(int i);
int duel_count_monsters_faceup_is(unsigned int p, unsigned int faceup);
void duel_088367A8(unsigned int p, int b, int id, int uid);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_free_st_zones(unsigned int p);
int duel_df664(unsigned int p);
int card_get_prop5_hi(int id);
int duel_card_e59b0(int id);
int duel_can_pay(unsigned int p, int id);
int duel_0880A934(unsigned int p, unsigned short id);
int card_get_type(int id);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
int duel_08853660(void);
int duel_d_find_name(unsigned int p, int id);
int duel_grave_find_name(unsigned int p, int id);
int duel_count_active(int id);
int duel_monster_can_act(unsigned int p, int z);
int duel_facedown_can_act(unsigned int p, int z);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_count_field_all(unsigned int p);
int duel_08809F2C();
int duel_08814EC8(unsigned int p, DuelAiCard *e, DuelAct *b);

int duel_08820618(unsigned int p, DuelAiCard *e)
{
    int z;
    int i;
    int r;
    DuelPlayer *pl;
    int id = e->id;

    pl = &duel.players[p & 1];
    for (z = 0; z <= 10; z++) {
        if (id == field_get_card(p, z) && !(pl->field[z].flags >> 5 & 1) && !(pl->field[z].flags >> 1 & 1)) {
            DuelAct t = {0};

            t.player = p;
            t.zone = z;
            t.opp = 0;
            t.uid = CARD_UID(pl->field[z].card);
            t.id = id;
            if (pl->field[z].faceup != 0) {
                r = duel_0886981C(&t, 0);
            } else {
                r = duel_08869ACC(&t, 0);
            }
            if (r != 0 && duel_ai_0882054C(&t) == 0 && e->cond(&t, 0) != 0) {
                duel_ai_player = p;
                duel_08835F34(p, z, 0, 3);
                duel_w_0891963C = 1;
                return 1;
            }
        }
    }
    i = duel_hand_find_name(p, e->id);
    if (i >= 0 && duel_0880AFC4(p) != 0) {
        DuelAct t = {0};

        t.id = pl->hand[i].id;
        t.player = p;
        t.zone = 0xB;
        t.opp = 0;
        t.uid = CARD_UID(pl->hand[i]);
        if (duel_08869ACC(&t, 0) != 0 && duel_ai_0882054C(&t) == 0 && e->cond(&t, 0) != 0) {
            duel_ai_player = p;
            duel_08835F34(p, 0xB, i, 3);
            duel_w_0891963C = 1;
            return 1;
        }
    }
    return 0;
}

int duel_08820948(unsigned int p, int b)
{
    unsigned int i;
    DuelPlayer *pl;
    unsigned int *n;
    int id;
    int q;
    DuelPlayer *o;

    int ok;

    i = 0;
    if (*(unsigned int *)((char *)&duel.players[0].n_hand + (p & 1) * 0xFAC) != 0) {
        q = 1 - p;
        pl = &duel.players[p & 1];
        o = &duel.players[q & 1];
        n = &pl->n_hand;
        do {
            id = pl->hand[i].id;
            if (card_is_monster(id) != 0) {
                ok = 1;
                switch (id) {
                case 0x18B4:
                    ok = duel_088BFFD8(p, id, 0) >= 4;
                    break;
                case 0x1578:
                    ok = o->lp < 1001 || duel_count_field_cards(p, 0, 1, 0) > 0 ||
                         (duel_0880A8AC(p, q) >= 3000 && !duel_count_monsters(p));
                    break;
                case 0x1488:
                    ok = !duel_grave_count_monsters(q);
                    break;
                case 0x127E:
                case 0x127F:
                case 0x127D:
                case 0x12A5:
                case 0x154A:
                    ok = b != 0 && duel_088079F8(p, CARD_UID(pl->hand[i]), 1) != 0;
                    break;
                case 0x1894:
                    ok = !duel_hand_count_name(p, 0x17F6);
                    break;
                case 0x19A5:
                    ok = !duel_count_name(p, id);
                    break;
                case 0x1A92:
                    ok = duel_count_monsters(p) > 0;
                    break;
                }
                if (ok && duel_08828740(p, CARD_UID(pl->hand[i])) != 0) {
                    duel_ai_player = p;
                    duel_08835F34(p, 0xB, i, 2);
                    duel_w_0891963C = 1;
                    duel_w_08913EC4 = 0;
                    return 1;
                }
            }
        } while (++i < *n);
    }
    if (duel_088BFF78(p, 0xFFFE, 0xC) != 0) {
        int c;

        for (q = 0; q < duel_0884C824(); q++) {
            c = duel_0884C830(q);
            ok = 1;
            switch (c) {
            case 0x15B1:
                ok = duel_count_field_cards(1 - p, 1, 0, 1) > 0;
                break;
            case 0x15F9:
                ok = duel_count_field_cards(1 - p, 0, 1, 1) > 0;
                break;
            case 0x15FA:
                ok = duel_count_monsters_faceup_is(1 - p, 0) > 0;
                break;
            }
            if (ok) {
                duel_088367A8(p, 0xC, c, CARD_UID(*duel_0884C7EC(q)));
                return 1;
            }
        }
    }
    return 0;
}

int duel_08820D44(unsigned int p)
{
    int z;
    DuelPlayer *pl;

    if (duel_find_effect_card(p, 0x145D, -1) != 0) {
        return 1;
    }
    if (duel_find_effect_card(p, 0x146F, -1) != 0) {
        return 1;
    }
    if (duel_find_effect_card(p, 0x1A6F, -1) != 0) {
        return 1;
    }
    pl = &duel.players[p & 1];
    for (z = 0; z < 5; z++) {
        if (pl->field[z].faceup == 0) {
            switch (pl->field[z].card.id) {
            case 0x11F5:
                return 1;
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* o original não sabe que v0 já vale 1 no default (b fim; li v0,1) e grava o passo antes do li; o mwcc salta direto ao epílogo */
int duel_ai_step_08820E08(void)
{
    unsigned int p = duel_turn_player;
    unsigned int i;
    int r;

    switch (duel_w_08913CFC) {
    default:
        return 1;
    case 0:
        for (i = 0; i < 0x3D; i++) {
            r = !duel_08820618(p, &duel_ai_field_cards[i]);
            if (!r) {
                return 0;
            }
        }
        duel_w_08913CFC++;
    case 1:
        if (duel_08820D44(p) != 0 && duel_08821888(p) != 0) {
            return 0;
        }
        duel_w_08913CFC++;
    case 2:
        if (duel_08821124(p) != 0) {
            return 0;
        }
        duel_w_08913CFC++;
    case 3:
        if (duel_08820948(p, 0) != 0) {
            return 0;
        }
        duel_w_08913CFC++;
        return 1;
    }
}
#endif

int duel_08820F60(unsigned int p)
{
    unsigned int i;
    int r;

    for (i = 0; i < 0xB6; i++) {
        r = !duel_08820618(p, &duel_ai_summons[i]);
        if (!r) {
            return 1;
        }
    }
    return 0;
}

int duel_08820FD0(unsigned int p, int b)
{
    unsigned int i;
    int r;

    for (i = 0; i < 4; i++) {
        r = !duel_08820618(p, &duel_ai_cards_08906D80[i]);
        if (!r) {
            return 1;
        }
    }
    if (b != 0) {
        return 0;
    }
    for (i = 0; i < 5; i++) {
        r = !duel_08820618(p, &duel_ai_cards_08906DA0[i]);
        if (!r) {
            return 1;
        }
    }
    return 0;
}

int duel_08821094(unsigned int p)
{
    unsigned int i;
    DuelAiCard *e = duel_ai_monster_cards;

    for (i = 0; i < 0x103; e++, i++) {
        if (card_is_monster(e->id) != 0 && duel_08814EC8(p, &duel_ai_monster_cards[i], 0) != 0) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* forma igual; o mwcc troca os registradores $s de p, do id e do ponteiro da tabela (s0/s1) */
int duel_08821124(unsigned int p)
{
    unsigned int i;
    DuelAiCard *e;
    int id;
    int z;
    int q;
    DuelPlayer *pl;

    unsigned int *n;
    int r;
    int uid;
    int ok;
    int k;

    for (i = 0, e = duel_ai_monster_cards; i < 0x103; i++, e++) {
        r = !duel_08820618(p, e);
        if (!r) {
            return 1;
        }
    }
    if (duel_088BFF78(p, 0xFFFE, 0xD) != 0) {
        for (i = 0; (int)i < duel_0884C824(); i++) {
        id = duel_0884C830(i);
        k = duel_d_find_name(p, id);
        if (k >= 0) {
            uid = CARD_UID(duel.players[p & 1].zone_d[k]);
        } else {
            uid = 0;
        }
        duel_088367A8(p, 0xD, id, uid);
        return 1;
        }
    }
    if (duel_088BFF78(p, 0xFFFE, 0xE) != 0) {
        for (i = 0; (int)i < duel_0884C824(); i++) {
        id = duel_0884C830(i);
        k = duel_grave_find_name(p, id);
        if (k >= 0) {
            uid = CARD_UID(duel.players[p & 1].grave[k]);
        } else {
            uid = 0;
        }
        duel_088367A8(p, 0xE, id, uid);
        return 1;
        }
    }
    if (duel_count_active(0x166C) != 0) {
        return 0;
    }
    pl = &duel.players[p & 1];
    q = 1 - p;
    n = &pl->n_hand;
    for (z = 0; z < 5; z++) {
        id = pl->field[z].card.id;
        if (duel_monster_can_act(p, z) == 0) {
            continue;
        }
        if (duel_facedown_can_act(p, z) != 0) {
            ok = 0;
            switch (id) {
            case 0x11D8:
            case 0x12A1:
            case 0x16FA:
            case 0x1533:
            case 0x19F1:
                ok = 1;
                break;
            case 0x119B:
            case 0x11C3:
            case 0x1200:
            case 0x1227:
            case 0x152A:
            case 0x152B:
            case 0xFFA:
            case 0x1086:
            case 0x13AB:
            case 0x179A:
            case 0x13B0:
            case 0x1413:
            case 0x152E:
            case 0x186E:
            case 0x18BA:
            case 0x18BB:
            case 0x16BA:
            case 0x14CD:
            case 0x14CE:
            case 0x18BF:
            case 0x18C0:
            case 0x18C1:
            case 0x18C2:
            case 0x18B5:
            case 0x1877:
            case 0x17E0: {
                DuelAct t = {0};

                t.player = p;
                t.zone = z;
                t.uid = CARD_UID(pl->field[z].card);
                t.b3_4 = 1;
                t.id = id;
                ok = duel_0884E948(&t, q) > 0;
                break;
            }
            case 0x1331:
            case 0x1369:
                ok = duel_08809F2C(p) < 0;
                break;
            case 0x11F5:
                ok = *n < 5;
                break;
            case 0x11F0:
            case 0x1489:
                ok = duel_count_field_all(q) >= 2;
                break;
            }
            if (ok) {
                duel_ai_player = p;
                duel_w_0891963C = 1;
                duel_08835F34(p, z, 0, 5);
                return 1;
            }
        } else if (pl->field[z].pos != 0) {
            ok = 0;
            switch (id) {
            case 0x1005: {
                DuelAct t = {0};

                t.player = p;
                t.zone = z;
                t.uid = CARD_UID(pl->field[z].card);
                t.b3_4 = 1;
                t.id = id;
                ok = duel_0884E948(&t, q) > 0;
                break;
            }
            }
            if (ok) {
                duel_ai_player = p;
                duel_w_0891963C = 1;
                duel_08835F34(p, z, 0, 8);
                return 1;
            }
        } else {
            ok = 0;
            switch (id) {
            case 0x101E:
            case 0x1868: {
                DuelAct t = {0};

                t.player = p;
                t.zone = z;
                t.uid = CARD_UID(pl->field[z].card);
                t.b3_4 = 1;
                t.id = id;
                ok = duel_0884E948(&t, q) > 0;
                break;
            }
            }
            if (ok) {
                duel_ai_player = p;
                duel_w_0891963C = 1;
                duel_08835F34(p, z, 0, 9);
                return 1;
            }
        }
    }
    return 0;
}
#endif

int duel_08821888(unsigned int p)
{
    int pick = -1;
    int last = pick;
    unsigned int i;
    unsigned int *n;
    unsigned int *m;
    DuelPlayer *pl;
    int id;
    int ok;

    if (duel_count_free_st_zones(p) >= 2 && duel_df664(p) != 0 && duel_0880AFC4(p) != 0) {
        n = (unsigned int *)((char *)&duel.players[0].n_hand + (p & 1) * 0xFAC);
        i = 0;
        if (*n != 0) {
            pl = &duel.players[p & 1];
            m = &pl->n_hand;
            do {
                id = pl->hand[i].id;
                if (card_is_monster(id) == 0 && duel_can_pay(p, id) != 0 && duel_0880A934(p, id) != 0 &&
                    duel_card_e59b0(id) == 0) {
                    ok = 1;
                    if (card_get_type(id) == 0x16) {
                        ok = card_get_prop5_hi(id) == 5;
                        if (!ok) {
                            ok = duel_count_active(0x49C) != 0;
                        }
                    }
                    if (p != pl->hand[i].owner) {
                        ok = 0;
                    }
                    switch (id) {
                    case 0x10DD:
                        ok = 1;
                        break;
                    case 0x131E:
                        ok = 1;
                        break;
                    case 0x138F:
                        ok = 0;
                        break;
                    case 0x1435:
                        ok = 1;
                        break;
                    }
                    last = i;
                    if (ok) {
                        pick = i;
                        break;
                    }
                }
            } while (++i < *m);
        }
        if (pick < 0 && last >= 0 && duel_count_field_all(p) == 0 && *n >= 5) {
            pick = last;
        }
        if (pick >= 0) {
            duel_ai_player = p;
            duel_08835F34(p, 0xB, pick, 7);
            duel_w_0891963C = 1;
            return 1;
        }
    }
    return 0;
}

int duel_ai_step_08821ABC(void)
{
    unsigned int p = duel_turn_player;
    unsigned int i;
    int r;
    int n;
    int z;

    switch (duel_w_08913CFC) {
    case 0:
        for (i = 0; i < 0x57; i++) {
            r = !duel_08820618(p, &duel_ai_st_cards[i]);
            if (!r) {
                return 0;
            }
        }
        duel_w_08913CFC++;
    case 1:
        if (duel_count_free_st_zones(p) >= 2 && duel_df664(p) != 0 && duel_0880AFC4(p) != 0) {
            n = 0;
            for (z = 5; z <= 10; z++) {
                if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup == 0 &&
                    (duel.players[p & 1].field[z].flags >> 20 & 1)) {
                    n++;
                }
            }
            if ((n < 2 || duel_08820D44(p) != 0) && duel_08821888(p) != 0) {
                return 0;
            }
        }
        return 1;
    default:
        return 1;
    }
}

static int duel_08821C3C(int id)
{
    switch (id) {
    case 0x1578:
    case 0x166A:
    case 0x19F4:
    case 0x1A92:
        return 1;
    }
    return 0;
}

typedef struct DuelAtkEv {             /* event argument of an attack (duel_088AFD5C) */
    unsigned int uid : 9;
    unsigned int player : 1;
    unsigned int zone : 4;
    unsigned int pos : 1;
    unsigned int faceup : 1;
    unsigned int b16 : 1;
    unsigned int b17 : 1;
    unsigned int b18 : 2;
} DuelAtkEv;

typedef struct DuelAtkRec {            /* 0x08914A14, 0x1C: the attack being resolved (sent as is to the peer) */
    unsigned int player : 1;           /* attacking card */
    unsigned int zone : 4;
    unsigned int b5 : 1;               /* the attack goes on (not cancelled) [?] */
    unsigned int kind : 2;             /* bits 6-7 */
    unsigned int id : 15;
    unsigned int uid : 9;
    unsigned short h18;                /* +0x04 duel_h_08914A18 */
    unsigned short atk_id;             /* +0x06 duel_h_08914A1A */
    unsigned char n;                   /* +0x08 number of targets */
    unsigned char count;               /* +0x09 distinct targets */
    unsigned short uids[3];            /* +0x0A */
    unsigned short locs[3];            /* +0x10 player | zone << 8 */
    unsigned char f0 : 1;              /* +0x16 duel_w_08914A2A */
    unsigned char f1 : 1;
    unsigned char owner : 1;           /* bit 0 of the uid of the attacking card */
    unsigned char f3 : 1;
    unsigned char f4 : 2;
    unsigned char f6 : 1;
    unsigned char f7 : 1;
    unsigned char flags2;              /* +0x17 */
} DuelAtkRec;

extern DuelAtkRec duel_b_08914A14;
extern unsigned short duel_h_08914A18;
extern unsigned short duel_h_08914A1A;
extern unsigned char duel_b_08914A1D;
extern struct {
    unsigned char b0 : 1;
    unsigned char b1 : 5;
    unsigned char b6 : 1;
} duel_w_08914A2A;
extern unsigned char duel_ai_atk_step;  /* 0x08914A2C */
extern unsigned char duel_ai_step2;     /* 0x08914A2D */
extern unsigned char duel_b_08914A2E;
extern int duel_w_089147D0;
extern int duel_w_089195EC;
extern int duel_w_08914920;
extern int duel_player_type[2];

void duel_088AFD5C(unsigned short kind, DuelAtkEv ev);

int duel_ai_atk_08821C80(int kind)
{
    DuelAtkEv ev;

    switch (duel_ai_atk_step) {
    case 3:
        if (duel_w_089147D0 > 1) {
            duel_ai_atk_step = 8;
            return 0;
        }
        if (duel.players[duel_b_08914A14.player & 1].field[duel_b_08914A14.zone].card.id == 0) {
            return 1;
        }
        duel_ai_step2 = 0;
        duel_b_08914A2E = 0;
        duel_ai_atk_step++;
        if (duel_w_089195EC != 0 && duel_player_type[duel_turn_player] == 2) {
            duel_msg_send(0x16, -1, &duel_b_08914A14, 0x1C);
            duel_w_08914A2A.b0 = 0;
            duel_w_08914A2A.b6 = 0;
            return 0;
        }
    case 4:
        duel_w_08914920 = 0;
        ev.player = duel_b_08914A14.player;
        ev.zone = duel_b_08914A14.zone;
        ev.uid = CARD_UID(duel.players[ev.player & 1].field[ev.zone].card);
        ev.faceup = duel.players[ev.player & 1].field[ev.zone].faceup;
        ev.pos = duel.players[ev.player & 1].field[ev.zone].pos;
        ev.b16 = duel_08821C3C(duel_h_08914A1A);
        ev.b17 = (duel_h_08914A18 & 1) > 0;
        ev.b18 = duel_b_08914A1D;
        duel_088AFD5C(kind, ev);
        duel_ai_atk_step = 7;
    case 7:
        if (duel_08853660() == 0) {
            duel_ai_atk_step = 8;
        }
        return 0;
    default:
        return 1;
    }
}

extern int duel_w_08919624;

int duel_0880F1D0(unsigned int p, int id, int b, int n, unsigned short *locs, int c);
void duel_0884AF68(int msg, int uid, int a);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088C44E4(unsigned int p, int id);
void duel_088D1B20(unsigned int p, unsigned int mask);
void duel_088CC178(unsigned int p, int z, unsigned int v);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int card_is_in_group_1a2f(int id);
int card_get_kind(int id);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_field_base_atk(unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_0886BCDC(void);
unsigned int duel_card_locate(unsigned int uid);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088CC4B4(unsigned int p, int z, int n);
void duel_088C0BB0(unsigned int p, int z);
int duel_slot_faceup_live(unsigned int p, int z);
void duel_088C4448(unsigned int p, int z, unsigned int uid);

/* Steps of the attack: announce, targets, damage calculation (duel_ai_atk_08821C80), end. */
#ifdef NON_MATCHING /* forma quase igual (~330 palavras): alocação dos $s no passo 9 e ordem zona/jogador no 0x164F */
int duel_ai_atk_08822000(void)
{
    DuelAtkRec *const a = &duel_b_08914A14;
    int t[3];
    DuelStats st;
    DuelAtkEv ev;
    int i;
    int k;
    int cnt;
    unsigned int mask;
    unsigned int id;
    int atk;
    int def;
    unsigned int p;
    unsigned char z;

    switch (duel_ai_atk_step) {
    case 0:
        if (a->kind == 2) {
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = duel_0880F1D0(a->player, a->id, a->b5, a->n, a->locs, 0);
            } else {
                duel_0884AF68(0xF7, a->uid, a->b5);
            }
        }
        duel_ai_atk_step++;
        return 0;
    case 1:
        if (a->kind == 2) {
            a->kind = duel_w_08919624;
            if (a->kind == 0) {
                a->b5 = 1;
            }
        }
        if (a->b5) {
            duel_cmd((a->player ? 0x8000 : 0) | 0x60, a->id, a->kind, a->n);
        }
        a->count = 0;
        mask = 0;
        for (i = 0; i < a->n; i++) {
            duel_cmd((a->locs[i] & 0xFF ? 0x8000 : 0) | 0x4A, (unsigned char)((a->locs[i] & 0xFFFF) >> 8), 0, 0);
            if (i == 0 || a->locs[i] != a->locs[i - 1]) {
                a->count++;
            }
        }
        for (i = 0; i < a->n; i++) {
            int loc = a->locs[i];
            unsigned int q = loc & 0xFF;
            unsigned int qz = (unsigned char)((loc & 0xFFFF) >> 8);

            mask |= 1 << (qz + q * 16);
            if (i > 0 && loc == a->locs[i - 1]) {
                duel_088C44E4(a->player, duel.players[q & 1].field[qz].card.id);
            }
        }
        duel_088D1B20(a->player, mask);
        duel_cmd((a->player ? 0x8000 : 0) | 0x4F, a->id,
                 (a->player & 1) | (a->zone & 0x1F) << 1 | (a->b5 & 1) << 14 | (a->kind & 1) << 15,
                 a->owner + (a->uid << 1));
        cnt = 0;
        for (i = 0; i < a->n; i++) {
            if (i == 0 || a->locs[i] != a->locs[i - 1]) {
                t[cnt++] = a->locs[i];
            }
        }
        id = a->id;
        switch (id) {
        case 0x1388:
            if (a->b5 && cnt == 3) {
                atk = 0;
                def = 0;
                for (i = 0; i < cnt; i++) {
                    duel_field_stats(t[i] & 0xFF, (unsigned char)((t[i] & 0xFFFF) >> 8), &st);
                    atk += st.batk;
                    def += st.bdef;
                }
                duel_088CC178(a->player, a->zone,
                              ((atk > 0xFFFF ? 0xFFFF : atk) & 0xFFFF) | ((def > 0xFFFF ? 0xFFFF : def) & 0xFFFF) << 16);
            }
            break;
        case 0x164F:
            if (cnt > 0) {
                unsigned int qz = (unsigned char)((t[0] & 0xFFFF) >> 8);
                unsigned int q = (unsigned char)t[0];

                duel_088C87A8(a->player, a->zone, id, 5, CARD_UID(duel.players[q & 1].field[qz].card));
            }
            break;
        case 0x1654:
            if (a->b5 && cnt == 2) {
                duel_088CC178(a->player, a->zone,
                              duel_field_base_atk(t[0] & 0xFF, (unsigned char)((t[0] & 0xFFFF) >> 8)) +
                              duel_field_base_atk(t[1] & 0xFF, (unsigned char)((t[1] & 0xFFFF) >> 8)));
            }
            break;
        case 0x1688:
            if (a->b5 && cnt == 1) {
                duel_088CC178(a->player, a->zone,
                              duel_field_base_atk(t[0] & 0xFF, (unsigned char)((t[0] & 0xFFFF) >> 8)) * 2);
            }
            break;
        case 0x1838:
        case 0x1864:
            if (a->atk_id != 0) {
                duel_088CC178(a->player, a->zone, 1);
            }
            break;
        case 0x1A1D:
            if (a->n != 0 && card_is_in_group_1a2f(duel.list[a->uids[0]].id) != 0) {
                duel_088CC178(a->player, a->zone, duel.list[a->uids[0]].id * 10);
            }
            break;
        case 0x1A20:
            if (a->n == 0) {
                duel_088C87A8(a->player, a->zone, id, 5, 0);
            }
            break;
        case 0x1A21:
            if (cnt == 1) {
                duel_088C87A8(a->player, a->zone, id, 5, 0);
            }
            break;
        case 0x1A30:
            if (a->n != 0) {
                k = a->locs[0];
                if (duel_field_attr_is(k & 0xFF, (unsigned char)((k & 0xFFFF) >> 8), card_get_kind(id)) != 0) {
                    duel_088CC178(a->player, a->zone, 1);
                }
            }
            break;
        }
        duel_ai_atk_step++;
        return 0;
    case 2:
        if (!a->b5) {
            duel_ai_atk_step = 9;
            return 0;
        } else {
            duel_cmd((a->player ? 0x8000 : 0) | 0x2C, a->id, a->zone, 0);
            duel_ai_atk_step = 3;
            return 0;
        }
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return duel_ai_atk_08821C80(0x1F);
    case 8:
        z = a->zone;
        p = a->player & 1;
        if (duel.players[p & 1].field[z].card.id == 0) {
            return 1;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x45, z, duel_h_08914A18, a->n);
        duel_ai_atk_step++;
        return 0;
    case 9:
        if (duel_0886BCDC() != 0) {
            return 0;
        }
        for (i = 0; i < a->n; i++) {
            id = duel.list[a->uids[i]].id;
            switch (id) {
            case 0x14BA:
                if (a->player != (a->locs[i] & 0xFF) || card_get_type(a->id) != card_get_type(id)) {
                    break;
                }
            case 0x14F3: {
                unsigned int o = a->uids[i] & 1;
                unsigned int loc = duel_card_locate(a->uids[i]);

                duel_0884F30C((unsigned short)id | ((unsigned char)((loc & 0xFFFF) >> 8) << 16 & 0x1F0000 |
                                                    ((o << 31 & 0x80000000 | (a->player != o) << 24 & 0x1000000) | 0x400000)),
                              a->uids[i], 0);
                break;
            }
            }
        }
        cnt = 0;
        for (i = 0; i < a->n; i++) {
            if (i == 0 || a->locs[i] != a->locs[i - 1]) {
                cnt++;
            }
        }
        if (cnt >= 2) {
            for (i = 5; i < 10; i++) {
                if (duel_slot_e8170(a->player, i, 0x1662) != 0) {
                    duel_0884F30C(a->player << 31 | 0x200000 | (unsigned int)i << 16 & 0x1F0000 | 0x1662,
                                  CARD_UID(duel.players[a->player & 1].field[i].card), 0);
                }
            }
        }
        for (k = 0; k < 2; k++) {
            p = duel_turn_player ^ k;
            for (i = 5; i <= 9; i++) {
                if (duel_slot_e8170(p, i, 0x19B2) != 0) {
                    duel_088CC4B4(p, i, 1);
                }
            }
        }
        if (a->b5) {
            duel_088C0BB0(a->player, a->zone);
            if (duel_slot_faceup_live(a->player, a->zone) != 0) {
                ev.player = a->player;
                ev.zone = a->zone;
                ev.faceup = 1;
                ev.pos = a->kind & 1;
                ev.uid = CARD_UID(duel.players[a->player & 1].field[a->zone].card);
                ev.b16 = 0;
                ev.b17 = (duel_h_08914A18 & 1) > 0;
                ev.b18 = a->count;
                switch (a->id) {
                case 0x13B5:
                case 0x154B:
                    if (a->n < 3) {
                        break;
                    }
                case 0x156A: case 0x158B: case 0x172D: case 0x1737: case 0x174D: case 0x17E2: case 0x182E:
                case 0x185F: case 0x1864: case 0x18E0: case 0x1918: case 0x1A0A: case 0x1A1D: case 0x1A30:
                    if (a->n == 0) {
                        break;
                    }
                case 0x119A: case 0x120E: case 0x12B4: case 0x1334: case 0x1341: case 0x128E: case 0x1293:
                case 0x128A: case 0x14AC: case 0x14C6: case 0x14CC: case 0x14FD: case 0x1503: case 0x1526:
                case 0x158E: case 0x15B6: case 0x15D0: case 0x15D4: case 0x1617: case 0x164A: case 0x1694:
                case 0x16F8: case 0x16FD: case 0x170C: case 0x17EE: case 0x1807: case 0x180B: case 0x180C:
                case 0x189E: case 0x1900: case 0x1938: case 0x1996: case 0x19C0: case 0x19C6: case 0x19D3:
                    duel_0884F30C(a->id | a->player << 31 | 0xC200000 | a->zone << 16 & 0x1F0000, ev.uid, *(unsigned int *)&ev);
                    break;
                }
            }
            z = a->zone;
            p = a->player;
            if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
                switch (a->id) {
                case 0x1837:
                case 0x1913:
                case 0x1962:
                    duel_088C4448(p, z, a->uid);
                    duel_088CC4B4(a->player, a->zone, 3);
                    break;
                case 0x19BF:
                    duel_088C4448(p, z, a->uid);
                    duel_088CC4B4(a->player, a->zone, 2);
                    break;
                }
            }
        }
        duel_ai_atk_step++;
        return 0;
    default:
        return 1;
    }
}
#endif

extern unsigned char duel_b_08914A2F;

int card_is_attribute6(int id);
int card_attribute_icon_in_2_3(int id);
int card_get_special_group(int id);
int duel_field_level(unsigned int p, int z);
void duel_088C448C(int p, int a);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_0886B0E4(unsigned int a, unsigned short uid, int c);

#define DUEL_EV(p, z, f, id) ((p) << 31 & 0x80000000 | (f) | (unsigned int)(z) << 16 & 0x1F0000 | (id))

/* Steps of a direct attack [?]: same machine as duel_ai_atk_08822000 with the damage step 0x20. */
#ifdef NON_MATCHING /* 3 palavras: o original põe o move a2 (terceiro argumento de duel_0886B0E4) no atraso do jal */
int duel_ai_atk_088230CC(void)
{
    DuelAtkRec *const a = &duel_b_08914A14;
    DuelAtkEv ev;
    int f;
    int i;
    unsigned int q;
    int k;
    unsigned int code;
    unsigned char z;
    unsigned int p;

    switch (duel_ai_atk_step) {
    case 0:
        duel_cmd((a->player ? 0x8000 : 0) | 0x60, a->id, 0, 4);
        z = a->zone;
        p = a->player;
        if (duel.players[p & 1].field[z].pos != 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x34, z, 1, 1);
            duel_b_08914A2F = 1;
        } else {
            duel_cmd((p ? 0x8000 : 0) | 0x35, z, 1, 2);
            duel_b_08914A2F = 0;
        }
        duel_ai_atk_step++;
        return 0;
    case 1:
        f = 0;
        if (card_is_attribute6(a->id) != 0 && card_attribute_icon_in_2_3(a->id) != 0) {
            f = 1;
        }
        duel_cmd((a->player ? 0x8000 : 0) | 0x2C, a->id, a->zone, f + 1);
        duel_ai_atk_step++;
        return 0;
    case 2:
        if (duel_field_level(a->player, a->zone) < 3 && duel_count_active(0x1255) > 0) {
            duel_088C448C(-1, 0x1255);
            duel_088D1438(a->player, a->zone, duel_find_effect_card(1 - a->player, 0x1255, -1) > 0, a->id);
            return 1;
        }
        duel_ai_atk_step++;
        return 0;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return duel_ai_atk_08821C80(0x20);
    case 8:
        z = a->zone;
        p = a->player;
        if (duel.players[p & 1].field[z].card.id == 0) {
            return 1;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x45, z, duel_h_08914A18, a->n | 0x100);
        duel_ai_atk_step++;
        return 0;
    case 9:
        if (duel_0886BCDC() != 0) {
            return 0;
        }
        duel_088C7E68(a->player, a->zone, 0x15, 1);
        duel_088C0BB0(a->player, a->zone);
        for (i = 0; i < 5; i++) {
            if (i != a->zone && duel_slot_e8170(a->player, i, 0x18B6) != 0) {
                duel_0884F30C(DUEL_EV(a->player, i, 0xE200000, 0x18B6),
                              CARD_UID(duel.players[a->player & 1].field[i].card), 0);
            }
        }
        if (duel_field_attr_is(a->player, a->zone, 5) != 0) {
            int j;
            int m;

            for (m = 0; m < 2; m++) {
                q = duel_turn_player ^ m;
                for (j = 0; j < 5; j++) {
                    if ((q != a->player || j != a->zone) && duel_slot_e8170(q, j, 0x19F0) != 0) {
                        duel_0884F30C(DUEL_EV(q, j, 0xE200000, 0x19F0), CARD_UID(duel.players[q & 1].field[j].card), 0);
                    }
                }
            }
        }
        if (duel_slot_faceup_live(a->player, a->zone) != 0) {
            ev.player = a->player;
            ev.zone = a->zone;
            ev.faceup = 1;
            ev.pos = a->kind;
            ev.uid = CARD_UID(duel.players[a->player & 1].field[a->zone].card);
            ev.b16 = 0;
            ev.b17 = (duel_h_08914A18 & 1) > 0;
            ev.b18 = 0;
            switch (a->id) {
            case 0x1005:
                if (duel_b_08914A2F == 0) {
                    break;
                }
            case 0x119A: case 0x120E: case 0x12B4: case 0x1334: case 0x1341: case 0x1293: case 0x128A:
            case 0x152A: case 0x152B: case 0x152E: case 0x1533: case 0x16BC: case 0x16FA: case 0x16FD:
            case 0x170C: case 0x186E: case 0x18BA: case 0x18BB: case 0x1900: case 0x19C6: case 0x19F1:
            case 0x1A9F:
                duel_0884F30C((unsigned short)a->id | ((a->player << 31 & 0x80000000 | 0xE200000) | a->zone << 16 & 0x1F0000),
                              ev.uid, *(unsigned int *)&ev);
                break;
            }
            if (card_get_special_group(a->id) != 0) {
                int e = *(int *)&ev;

                code = (unsigned short)a->id | ((a->player << 31 & 0x80000000 | 0xE200000) | a->zone << 16 & 0x1F0000);
                if (duel_0886B0E4(code, ev.uid, e) != 0) {
                    duel_cmd((a->player ? 0x8000 : 0) | 0x61, a->id, 0, 0);
                    duel_0884F30C(code, ev.uid, e);
                }
            }
        }
        duel_ai_atk_step++;
        return 0;
    default:
        return 1;
    }
}
#endif

extern unsigned int duel_w_089149F0;
extern unsigned int duel_w_08914A00;

int duel_no_1a1b(unsigned int p);
int card_is_attribute9(int id);
void duel_088C77D4(unsigned int p, unsigned short id, unsigned short a, unsigned short b);
int duel_field_atk(unsigned int p, int z);
void duel_088328C8(unsigned int p, unsigned short a, unsigned int x, unsigned int y);
int duel_ai_sum_088239EC(void);

#ifdef NON_MATCHING /* o original deixa cópias mortas depois de beql/b nos ?: de duel_cmd (problema aberto) */
/* Steps of a summon (or of the activation of a card from the hand) [?]. */
int duel_ai_sum_088239EC(void)
{
    DuelAtkRec *const a = &duel_b_08914A14;
    DuelAtkEv ev;
    unsigned int p;
    unsigned char z;
    unsigned int v;
    int ok;

    switch (duel_ai_atk_step) {
    case 0:
        if (a->kind == 2) {
            if (duel_no_1a1b(a->player) == 0) {
                a->kind = 0;
            } else {
                p = a->player;
                if (duel_player_type[p] == 1) {
                    duel_w_08919624 = duel_0880F1D0(p, a->id, a->b5, a->n, a->locs, 1);
                } else {
                    if (((int)a->uid >> 1) == 0) {
                        a->uid = p + duel.players[p & 1].unk4 * 2;
                        duel.list[a->uid].id = a->id;
                    }
                    duel_0884AF68(0xF7, a->uid, a->b5);
                }
            }
        }
        duel_ai_atk_step++;
        return 0;
    case 1:
        if (a->kind == 2) {
            a->kind = duel_w_08919624;
            if (a->kind == 0) {
                a->b5 = 1;
            }
        }
        if (a->b5) {
            duel_cmd((a->player != duel_08821C3C(a->atk_id) ? 0x8000 : 0) | 0x60, a->id, a->kind,
                     ((a->h18 & 1) != 0) + 5);
        }
        if (a->id == 0x14D5 && (duel_h_08914A18 & 0x20)) {
            duel_088328C8(a->player, 0xFFFF, 1, 1);
        }
        duel_ai_atk_step++;
        return 0;
    case 2:
        if (card_is_attribute9(a->id) != 0) {
            duel_cmd((a->player ? 0x8000 : 0) | 0x48, (unsigned char)(a->b5 | a->kind << 1 | a->zone << 4) << 8, a->id, 0);
        } else {
            if (a->id == 0x1578) {
                duel_088C77D4(a->player, 0x1578, 1, 0);
            }
            duel_cmd(0x2D, a->player | a->zone << 8, a->uid, a->b5 | a->kind << 8);
            if (a->h18 & 1) {
                switch (a->id) {
                case 0x13A6:
                    duel_088C87A8(a->player, a->zone, 0x13A6, 5, 0);
                    break;
                case 0x1472:
                case 0x1636:
                case 0x172F:
                case 0x1809:
                    duel_088C87A8(a->player, a->zone, 0x1472, 5, duel_card_locate(a->uid));
                    duel_088C7E68(a->player, a->zone, 9, 1);
                    break;
                case 0x18FB:
                    duel_088CC178(a->player, a->zone, duel_w_08914A00 >= 0x10000 ? 0xFFFF : duel_w_08914A00);
                    break;
                case 0x1A4E:
                    duel_088CC178(a->player, a->zone, duel_w_089149F0);
                    break;
                }
            }
        }
        duel_ai_atk_step = 8;
        return 0;
    case 8:
        z = a->zone;
        p = a->player;
        if (duel.players[p & 1].field[z].card.id == 0) {
            return 1;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x45, z, duel_h_08914A18, a->n);
        duel_ai_atk_step++;
        return 0;
    case 9:
        if (duel_0886BCDC() != 0) {
            return 0;
        }
        switch (a->atk_id) {
        case 0x1A65:
            duel_088C87A8(a->player, a->zone, 0x1A65, ((duel_turn_player != a->player) + 1) * 16 | 2, 0);
            break;
        case 0x1A31:
            v = duel_field_atk(a->player, a->zone) / 2;
            if ((int)v > 0xFFFF) {
                v = 0xFFFF;
            }
            duel_088C87A8(a->player, a->zone, a->atk_id, 2, v);
            break;
        case 0x18FE:
            duel_088C87A8(a->player, a->zone, 0x18FE, 3, 0);
            duel_088C77D4(a->player, a->atk_id, 1, duel_field_base_atk(a->player, a->zone));
            break;
        case 0x19D2:
            duel_088CC178(a->player, a->zone, 1);
            break;
        case 0x1595:
            duel_088CC178(a->player, a->zone, 1 - a->player);
            break;
        case 0x17D1:
        case 0x1822:
        case 0x185E:
            duel_088C87A8(a->player, a->zone, a->id, 3, 0);
            break;
        case 0x1481: case 0x156A: case 0x15ED: case 0x1619: case 0x16A4: case 0x17BE: case 0x1876:
        case 0x1900: case 0x1944: case 0x1A7B:
            duel_088C87A8(a->player, a->zone, a->atk_id, 2, 0);
            break;
        case 0x151E:
            duel_088C87A8(a->player, a->zone, 0x151E, 1, 1);
            break;
        case 0x19F7:
        case 0x19F8:
            if (duel_h_08914A18 & 0x20) {
                duel_088C87A8(a->player, a->zone, a->atk_id, 3, 0);
            }
            break;
        case 0x14A7: case 0x14F7: case 0x188F: case 0x196B: case 0x196C: case 0x196D: case 0x196E:
        case 0x1990: case 0x1A5C:
            duel_088C87A8(a->player, a->zone, a->atk_id, 3, 0);
            break;
        case 0x12EA: case 0x17D5: case 0x1A2B: case 0x1A2C: case 0x1A2D: case 0x1A2E:
            duel_088C87A8(a->player, a->zone, a->atk_id, 5, 0);
            break;
        }
        if (duel_slot_faceup_live(a->player, a->zone) != 0) {
            ok = 0;
            switch (a->id) {
            case 0x1A05:
                ok = duel_h_08914A1A == 0x1A0C;
                break;
            case 0x19EF:
            case 0x1A4E:
                ok = (duel_h_08914A18 & 1) != 0;
                break;
            case 0x190A:
                ok = 0;
                if (duel_h_08914A1A == 0x190A) {
                    ok = (duel_h_08914A18 & 0x20) == 0;
                }
                break;
            case 0x176C:
            case 0x185C:
            case 0x1A28:
            case 0x1A92:
                ok = duel_h_08914A1A == a->id;
                break;
            case 0x16F7:
                ok = (duel_h_08914A18 & 0x20) != 0;
                break;
            case 0x19F4:
                ok = 1;
                break;
            case 0x12B1: case 0x1388: case 0x14FC: case 0x164A: case 0x167C: case 0x16F8: case 0x16FB:
            case 0x1745: case 0x1788: case 0x17C9: case 0x17EE: case 0x1807: case 0x180B: case 0x180C:
            case 0x19A7: case 0x1996:
                ok = 1;
                break;
            }
            if (ok) {
                ev.player = a->player;
                ev.zone = a->zone;
                ev.faceup = 1;
                ev.pos = a->kind;
                ev.uid = CARD_UID(duel.players[a->player & 1].field[a->zone].card);
                ev.b16 = 0;
                ev.b17 = (duel_h_08914A18 & 1) > 0;
                ev.b18 = duel_b_08914A1D;
                duel_0884F30C((unsigned short)a->id | ((a->player << 31 & 0x80000000 | 0x10200000) | a->zone << 16 & 0x1F0000),
                              ev.uid, *(unsigned int *)&ev);
            }
        }
        duel_ai_atk_step++;
        return 0;
    default:
        return 1;
    }
}
#endif

int card_is_group_c(int id);
void duel_088C78E0(unsigned int p);

/* Steps of a special summon [?]: targets, then the summon steps of duel_ai_sum_088239EC. */
int duel_ai_atk_08824798(void)
{
    DuelAtkRec *const a = &duel_b_08914A14;
    int i;
    unsigned int mask;

    switch (duel_ai_atk_step) {
    case 0:
        if (a->kind == 2) {
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = duel_0880F1D0(a->player, a->id, a->b5, a->n, a->locs, 1);
            } else {
                duel_0884AF68(0xF7, a->uid, a->b5);
            }
        }
        duel_ai_atk_step++;
        return 0;
    case 1:
        if (a->kind == 2) {
            a->kind = duel_w_08919624;
            if (a->kind == 0) {
                a->b5 = 1;
            }
        }
        if (a->b5) {
            duel_cmd((a->player ? 0x8000 : 0) | 0x60, a->id, a->kind, 6);
        }
        a->count = 0;
        mask = 0;
        for (i = 0; i < a->n; i++) {
            duel_cmd((a->locs[i] & 0xFF ? 0x8000 : 0) | 0x4A, (unsigned char)((a->locs[i] & 0xFFFF) >> 8), 0, 0);
            if (i == 0 || a->locs[i] != a->locs[i - 1]) {
                a->count++;
            }
        }
        for (i = 0; i < a->n; i++) {
            mask |= 1 << ((unsigned char)((a->locs[i] & 0xFFFF) >> 8) + (a->locs[i] & 0xFF) * 16);
        }
        duel_088D1B20(a->player, mask);
        duel_ai_atk_step++;
        return 0;
    case 2:
        if (a->atk_id == 0x1578 || a->atk_id == 0x1A92) {
            duel_088C77D4(a->player, a->id, 1, 0);
            a->player ^= 1;
        }
        duel_cmd(0x2D, a->player | a->zone << 8, a->uid, a->b5 | a->kind << 8);
        switch (a->id) {
        case 0x18B4:
            duel_088CC178(a->player, a->zone, duel_b_08914A14.locs[0] * 700);
            break;
        }
        if (card_is_group_c(a->id) != 0) {
            duel_ai_atk_step = 3;
        } else {
            duel_ai_atk_step = 8;
        }
        return 0;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return duel_ai_atk_08821C80(0x21);
    default:
        if (duel_ai_sum_088239EC() != 0) {
            switch (a->id) {
            case 0x112E:
            case 0x128C:
            case 0x196B:
            case 0x196C:
            case 0x196D:
            case 0x196E:
                duel_088C78E0(a->player);
                break;
            }
            break;
        }
        return 0;
    }
    return 1;
}
