// @range 0x088CB400-0x088CC178
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
void duel_088CBB64(unsigned int p, int z, unsigned int on);
extern unsigned int duel_w_08919620;
int field_get_card(unsigned int p, int z);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088C8AD0(unsigned int p, int z, int id, int v);
void duel_088CC178(unsigned int p, int z, int a);
void duel_088C8940(unsigned int p, int z);
void duel_088C929C(unsigned int p, int z, unsigned int kind);
void duel_088C1B88(unsigned int p, int z);
void duel_088CC284(unsigned int p, int z, int n);
void duel_088CC4B4(unsigned int p, int z, int n);

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

/* card (p, z) negated (on = 1) or no longer negated: the cards whose effect stops or starts with it */
void duel_088CBB64(unsigned int p, int z, unsigned int on)
{
    int id;

    if (on == (duel.players[p & 1].field[z].flags >> 5 & 1)) {
        return;
    }
    id = field_get_card(p, z);
    duel_088C7E68(p, z, 5, on);
    if (duel.players[p & 1].field[z].faceup == 0) {
        return;
    }
    if (z < 5) {
        if (on) {
            switch (id) {
            case 0x1296:
            case 0x15DA:
            case 0x172E:
            case 0x1812:
            case 0x1814:
            case 0x1816:
            case 0x181A:
            case 0x1955:
                if (++duel_w_08919620 >= 10) {
                    duel_w_08919620 = 0;
                    duel_088D1438(p, z, 0, 0);
                }
                break;
            case 0x10AC:
            case 0x10C6:
            case 0x1153:
            case 0x1184:
            case 0x12AC:
            case 0x1337:
            case 0x1397:
            case 0x147A:
            case 0x147D:
            case 0x1482:
            case 0x1527:
            case 0x154C:
            case 0x154D:
            case 0x1644:
            case 0x1645:
            case 0x164D:
            case 0x172B:
            case 0x1737:
            case 0x17C9:
            case 0x17ED:
            case 0x182D:
            case 0x1835:
            case 0x183B:
            case 0x188F:
            case 0x18AE:
            case 0x1996:
            case 0x19A5:
            case 0x19F7:
            case 0x19F8:
                duel_088C89BC(p, z, id, 1);
                break;
            case 0x13B2:
                duel_088C8AD0(p, z, id, 1);
                break;
            case 0x1009:
                duel_088C7E68(p, z, 4, 0);
                duel_088C89BC(p, z, id, 1);
                break;
            case 0x120E:
            case 0x12BB:
            case 0x1388:
            case 0x1654:
            case 0x1688:
            case 0x1838:
            case 0x1864:
            case 0x18B4:
            case 0x18FB:
            case 0x1A4E:
            case 0x1A79:
                duel_088CC178(p, z, 0);
                break;
            case 0x14FC:
            case 0x1957:
                duel_088C8940(p, z);
                break;
            case 0x13AD:
                duel_088C7E68(p, z, 4, 0);
                break;
            case 0x1281:
            case 0x1284:
                duel_088C929C(p, z, 0xD);
                break;
            case 0x12A6:
                duel_088C929C(p, z, 0xC);
                break;
            case 0x1A1D:
                duel_088C89BC(p, z, id, 1);
                duel_088CC178(p, z, 0);
                break;
            }
        }
    } else {
        switch (id) {
        case 0x1302:
        case 0x1360:
        case 0x1495:
            if (++duel_w_08919620 >= 10) {
                duel_w_08919620 = 0;
                duel_088D1438(p, z, 0, 0);
            }
            break;
        case 0x1368:
            if (!on) {
                duel_088C1B88(p, z);
            }
            break;
        case 0x17FF:
        case 0x184A:
            if (on) {
                duel_088C8940(p, z);
            }
            break;
        }
    }
    if (on) {
        if (card_get_group_value_1983(id)) {
            if (duel.players[p & 1].field[z].link != 0) {
                duel_088CC284(p, z, -duel.players[p & 1].field[z].link);
            }
        } else if (card_is_in_group_1a0c(id)) {
            if (duel.players[p & 1].field[z].link != 0) {
                duel_088CC4B4(p, z, -duel.players[p & 1].field[z].link);
            }
        }
    }
}
