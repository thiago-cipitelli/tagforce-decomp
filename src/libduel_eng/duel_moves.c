// @range 0x088CB400-0x088CBB64
// @asm 0x088CB55C-0x088CB728
// @asm 0x088CB778-0x088CB8EC
/* Commands to the presentation for cards moving between the piles and the field (0x33, 0x3F, 0x41),
   attacks (0x29), the shuffle of the zones (0x49) and a card leaving a zone (0x43). */
#include "duel.h"
#include "card_db.h"

extern DuelCard duel_cards[512];
extern unsigned int duel_089195C8;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
unsigned int duel_f_find_uid(unsigned int p, int uid);
int duel_088CB55C(unsigned int p, DuelCard *c, int f);
void duel_088CBB64(unsigned int p, int z, int a);

/* 0x088CB55C, 0x088CB778: kept in asm. */

void duel_088CB400(unsigned int p, DuelCard *c, int a)
{
    int f = a != 0;

    duel_cmd((p ? 0x8000 : 0) | 0x33, 0xE, f, CARD_UID(*c));
}

void duel_088CB44C(unsigned int p, DuelCard *c, int a)
{
    duel_cmd((p ? 0x8000 : 0) | 0x33, 0xE, a, CARD_UID(*c));
}

void duel_088CB490(unsigned int p, unsigned int uid)
{
    duel_cmd(0x3F, (uid & 0x3FF) << 6 | ((p & 1) | 0x1E), (p & 1) | 0x5C, 0);
}

int duel_088CB4BC(unsigned int p, unsigned int uid)
{
    int k;

    if ((int)duel_f_find_uid(p, uid) >= 0) {
        k = card_attribute_icon_is3(duel.list[uid].id) ? 0xC : 0xD;
        duel_cmd(0x3F, (uid & 0x3FF) << 6 | ((p & 1) | 0x1E), (p & 1) | (k & 0x1F) << 1, 0);
        return 1;
    }
    return 0;
}

int duel_088CB554(unsigned int p, DuelCard *c)
{
    return duel_088CB55C(p, c, 1);
}

void duel_088CB728(int p, int z, int uid, unsigned int q, int qz, int a, int b)
{
    duel_cmd(0x41, (uid & 0x3FF) << 6 | ((p & 1) | (z & 0x1F) << 1), (q & 1) | (qz & 0x1F) << 1,
             (a & 0xFF) | (b & 0xFF) << 8);
}

void duel_088CB8EC(unsigned int p, int a, int b, int c)
{
    duel_cmd((p ? 0x8000 : 0) | 0x29, a, b, c + 1);
}

void duel_088CB914(unsigned int p, int a)
{
    duel_cmd((p ? 0x8000 : 0) | 0x29, a, 0, 0);
}

void duel_088CB938(unsigned int p, int a)
{
    duel_cmd((p ? 0x8000 : 0) | 0x29, a, 0, 0);
}

void duel_088CB95C(unsigned int p, int a, int b)
{
    duel_cmd((p ? 0x8000 : 0) | 0x29, a, b, 0);
}

void duel_088CB980(unsigned int p)
{
    duel_cmd((p ? 0x8000 : 0) | 0x29, 0xD, CARD_UID(duel.players[p & 1].zone_d[0]), 0);
}

void duel_088CB9E0(unsigned int p, unsigned int mask)
{
    unsigned int seed = duel_089195C8;
    int z;
    int n = 0;

    for (z = 0; z < 5; z++) {
        if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup == 0 &&
            duel.players[p & 1].field[z].pos != 0) {
            n++;
        } else {
            mask &= ~(1 << z);
        }
    }
    if (n > 1) {
        duel_cmd((p ? 0x8000 : 0) | 0x49, seed, seed >> 16, mask);
    }
}

void duel_088CBAA4(unsigned int p, int z, unsigned int uid)
{
    if (z < 11 && uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        duel_088CBB64(p, z, 1);
    }
    duel_cmd((p ? 0x8000 : 0) | 0x43, z, uid, 0);
}
