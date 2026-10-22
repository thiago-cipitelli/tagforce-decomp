// @range 0x08820618-0x08822000
// @asm 0x08820E08-0x08820F60
// @asm 0x08821124-0x08821888
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

extern struct {
    unsigned char player : 1;
    unsigned char zone : 4;
} duel_b_08914A14;                     /* attacking card */
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
