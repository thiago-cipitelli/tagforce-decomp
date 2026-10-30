// @range 0x088CA4E8-0x088CAF24
// @asm 0x088CAB04-0x088CAB5C
/* Cards leaving the hand: to the graveyard, the D zone or the field, with the command to the
   presentation (0x33) and the event of the move. */
#include "duel.h"
#include "card_db.h"

typedef struct DuelMoveEv {            /* event argument of a card leaving a zone */
    unsigned short uid : 9;
    unsigned short player : 1;
    unsigned short b10 : 1;
    unsigned short opp : 1;
    unsigned short zone : 4;
    unsigned short pad;
} DuelMoveEv;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_088CA4E8(unsigned int p, DuelCard *c, int a, int z, int b, int d);
int duel_count_active(int id);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
void duel_088AFD5C(int kind, DuelMoveEv ev);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088C44E4(unsigned int p, int id);
void duel_088C4730(unsigned int p, int n, int a, int id);

int duel_088DFCC8(unsigned int uid);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C050C(unsigned int p, int id);
void duel_088C01F8(unsigned int p, int id, int a);
void duel_088C1884(unsigned int p);
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_0884F30C(unsigned int a, unsigned short uid, DuelMoveEv ev);

/* card c leaves the hand of p for zone z (0xE graveyard, 0xF if it cannot go there, else D): a is the command
   argument (bit 0: for the opponent, bit 12: shown with 0x46), b: by an effect (event 0x1D, else 0x1E),
   d: 0 normal, 1 discard (0x1379), 2 [?] */
void duel_088CA4E8(unsigned int p, DuelCard *c, int a, int z, int b, int d)
{
    int opp = (a & 1) > 0;
    int k = b ? 0x1D : 0x1E;
    int f;
    int uid;
    DuelMoveEv ev;

    {
        char *q = (char *)&ev;
        int n = 4;

        if (q) {
            do {
                *q++ = 0;
            } while (--n);
        }
    }
    if (z == 0xE) {
        if (duel_088DFCC8(CARD_UID(*c)) == 0) {
            z = 0xF;
        }
    }
    uid = CARD_UID(*c);
    ev.uid = uid;
    ev.player = p;
    ev.b10 = 0;
    ev.opp = p ^ opp;
    ev.zone = z;
    if (a & 0x1000) {
        duel_cmd((p ? 0x8000 : 0) | 0x46, 0xB, z, ev.uid);
    } else if (z == 0xF) {
        duel_cmd((p ? 0x8000 : 0) | 0x33, 0xB, a, CARD_UID(*c));
    } else {
        duel_cmd(0x3F, (uid & 0x3FF) << 6 | ((p & 1) | 0x16), (c->owner & 1) | 0x5C, a);
    }
    if (z == 0xE) {
        f = 0;
        switch (c->id) {
        case 0x14A5:
        case 0x16F9:
            duel_088C4448((int)c->owner, 0xE, CARD_UID(*c));
            duel_088C77D4((int)c->owner, c->id, 1, CARD_UID(*c));
            break;
        case 0x1175:
        case 0x11BC:
        case 0x1653:
        case 0x1655:
        case 0x1793:
        case 0x1794:
        case 0x1799:
            if (opp && c->owner == p) {
                f = 1;
            }
            break;
        case 0x179A:
        case 0x1828:
        case 0x1946:
        case 0x1185:
        case 0x12A2:
        case 0x1309:
        case 0x13E3:
        case 0x1687:
        case 0x16F5:
        case 0x17C5:
        case 0x19FE:
            f = 1;
            break;
        case 0x1965:
        case 0x1966:
        case 0x1968:
        case 0x1969:
            f = !d && b != 0;
            break;
        }
        if (f) {
            unsigned char o = (int)c->owner;

            duel_0884F30C(c->id | ((o << 31 & 0x80000000) | (k << 25 & 0x7E000000) | 0x4E0000), o + (c->uid_hi << 1), ev);
        }
        if (opp && !d && b && c->owner == p) {
            duel_088C050C(p, 0x17A4);
        }
        if (card_get_type(c->id) == 9) {
            for (f = 0; f <= 4; f++) {
                if (duel_slot_e8170(c->owner, f, 0x1A8D)) {
                    duel_088C7E68(c->owner, f, 4, 1);
                }
            }
        }
    }
    duel_088AFD5C((unsigned short)k, ev);
    if (b) {
        if (p == c->owner && z == 0xE) {
            duel_088C01F8(1 - p, 0x1306, 1);
        }
        duel_088C1884(p);
        if (d != 2 && card_get_type(c->id) == 1) {
            duel_088C77D4(p, 0x14E2, 2, 0);
        }
    }
    if (d == 1 && duel_slot_effect_with(p, 0xB, 0x1379, CARD_UID(*c))) {
        duel_088C44E4(p, 0x1379);
        duel_088C4730(p, 2000, 0, 0x1379);
    }
}

#ifdef NON_MATCHING /* the original computes x != 0 in the delay slot of the index test */
void duel_088CAB04(unsigned int p, int i, int x)
{
    if (i >= 0) {
        int f = x != 0;

        duel_088CA4E8(p, duel_field_card_deb2c(p, 11, i), f, 0xE, 1, 0);
    }
}
#endif

void duel_088CAB5C(unsigned int p, int i)
{
    if (i >= 0) {
        duel_088CA4E8(p, duel_field_card_deb2c(p, 11, i), 0, 0xE, 1, 2);
    }
}

void duel_088CABA8(unsigned int p, int i)
{
    if (i >= 0) {
        duel_088CA4E8(p, duel_field_card_deb2c(p, 11, i), 0, 0xE, 1, 1);
    }
}

void duel_088CABF4(unsigned int p, int i, int x)
{
    int z = 0xE;

    if (x != 0 && duel_count_active(0x19DA) != 0) {
        z = card_attribute_icon_is3(duel.players[p & 1].hand[i].id) ? 0xC : 0xD;
    }
    duel_088CA4E8(p, &duel.players[p & 1].hand[i], 0x1000, z, 0, 0);
    if (card_get_type(duel.players[p & 1].hand[i].id) == 1) {
        duel_088C77D4(p, 0x14E2, 2, 0);
    }
}

void duel_088CACF0(unsigned int p, DuelCard *c, int n)
{
    DuelMoveEv ev = {0};

    ev.uid = CARD_UID(*c);
    ev.player = p;
    ev.b10 = 0;
    ev.opp = p ^ ((n & 1) > 0);
    ev.zone = 0xF;
    duel_cmd((p ? 0x8000 : 0) | 0x33, 0xB, n, CARD_UID(*c));
    duel_088AFD5C(0x1E, ev);
}

void duel_088CAE24(unsigned int p, DuelCard *c, int a)
{
    duel_088CA4E8(p, c, a, 0xE, 0, 0);
}

void duel_088CAE34(unsigned int p, int i)
{
    if (i >= 0) {
        duel_088CA4E8(p, duel_field_card_deb2c(p, 11, i), 0, 0xE, 0, 1);
    }
}

void duel_088CAE80(unsigned int p, int i)
{
    if (i >= 0) {
        DuelCard *c = duel_field_card_deb2c(p, 11, i);

        duel_088CACF0(p, c, 0);
        if (duel_slot_effect_with(p, 11, 0x1379, CARD_UID(*c)) != 0) {
            duel_088C44E4(p, 0x1379);
            duel_088C4730(p, 2000, 0, 0x1379);
        }
    }
}
