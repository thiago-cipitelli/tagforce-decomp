// @range 0x088C4864-0x088C77D4
// @asm 0x088C4864-0x088C5424
// @asm 0x088C5510-0x088C5E70
// @asm 0x088C5E78-0x088C625C
// @asm 0x088C626C-0x088C6728
// @asm 0x088C67F0-0x088C69EC
// @asm 0x088C6BC8-0x088C7478
// @asm 0x088C7548-0x088C77D4
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
void duel_088C5E78(unsigned int p, int n, int x, int f);
int duel_088DFCC8(unsigned int uid);
void duel_088C626C(unsigned int p, unsigned int uid, int a);
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
DuelCard *duel_0884CCBC(int k);
void duel_088C6BC8(unsigned int p, int z, int i, DuelCard *c, int a);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);

/* 0x088C4864, 0x088C5510, 0x088C5E78, 0x088C626C, 0x088C67F0, 0x088C6BC8, 0x088C7548: kept in asm. */

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

void duel_088C625C(unsigned int p, int n, int x)
{
    duel_088C5E78(p, n, x, 0);
}

void duel_088C6264(unsigned int p, int n, int x)
{
    duel_088C5E78(p, n, x, 1);
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
