// @range 0x088CA4E8-0x088CAF24
// @asm 0x088CA4E8-0x088CAB04
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

/* 0x088CA4E8: 1564 bytes kept in asm. */

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
