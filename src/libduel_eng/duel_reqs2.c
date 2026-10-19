// @range 0x088C936C-0x088CA4E8
// @asm 0x088C936C-0x088C96EC
// @asm 0x088C9878-0x088C9D24
// @asm 0x088C9E88-0x088C9F58
// @asm 0x088C9F58-0x088CA184
// @asm 0x088CA184-0x088CA270
// @asm 0x088CA270-0x088CA384
// @asm 0x088CA384-0x088CA428
// @asm 0x088CA428-0x088CA4E8
/* More commands to the presentation: a card is flipped or changes position (0x35), moves between
   zones (0x3D, 0x3E), goes from the hand or the graveyard to the field (0x3F). */
#include "duel.h"
#include "card_db.h"

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_count_active(int id);
int field_get_card(unsigned int p, int z);
unsigned short duel_slot_e1854(unsigned int p, int z);
void duel_088C88B8(unsigned int p, int z, int v, int c);
void duel_088CC178();
void duel_088CC1D0(unsigned int p, int z, int a);
int duel_0884F30C(unsigned int a, int uid, unsigned int b);
void duel_088AFD5C(int kind, unsigned int a);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088C9344(DuelAct *a, unsigned int p, int x, int y);
void duel_088C9E88(int loc, int t);
void duel_088C9F58(int loc, int t);
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp);

extern DuelCard duel_cards[512];

/* 0x088C936C: 896 bytes kept in asm. */
/* 0x088C9878: 1196 bytes kept in asm. */

void duel_088C96EC(unsigned int p, int z, int v)
{
    if (duel.players[p & 1].field[z].card.id != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x35, z, v, 8);
    }
}

void duel_088C9760(unsigned int p, int z)
{
    if (duel.players[p & 1].field[z].card.id != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x35, z, !duel.players[p & 1].field[z].faceup, 4);
    }
}

int duel_088C97F0(DuelAct *a, unsigned int p, int z, int x, int y)
{
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel_088C9878(p, z, x, y, a->id, p != a->player);
}

void duel_088C9D24(DuelAct *a, unsigned int p, int z)
{
    unsigned int q = p & 1;
    int x = !card_is_attribute9(duel.players[q].field[z].card.id);

    if (duel_count_active(0x135D) == 0 && duel_act_immune(a, p, z) == 0) {
        if (x && duel.players[q].field[z].faceup != 0) {
            if (duel.players[q].field[z].pos != 0) {
                duel_088C9344(a, p, z, 0);
            } else {
                duel_088C97F0(a, p, z, 1, 0);
            }
        } else if (duel.players[q].field[z].pos == 0) {
            duel_088C97F0(a, p, z, 0, 0);
        }
    }
}

#ifdef NON_MATCHING /* branch layout and argument packing not reproduced */
void duel_088C9E88(int loc, int t)
{
    unsigned int p = loc & 0xFF;
    unsigned int z = (unsigned short)loc >> 8 & 0xFF;
    unsigned short s;

    if (p != (t & 0xFF) && duel.players[p & 1].field[z].faceup != 0) {
        if (field_get_card(p, z) == 0x14AC) {
            s = duel_slot_e1854(p, z);
            if (s != 0xFFFF) {
                duel_088C88B8(s & 0xFF, s >> 8, (p & 0xFF) | (z & 0xFF) << 8, 1);
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* branch layout and argument packing not reproduced */
void duel_088C9F58(int loc, int t)
{
    unsigned int z = (unsigned short)loc >> 8 & 0xFF;
    unsigned int p = loc & 0xFF;
    unsigned int tz = (unsigned short)t >> 8 & 0xFF;
    unsigned int q;

    if (p != (t & 0xFF)) {
        q = loc & 1;
        if (duel.players[q].field[z].faceup != 0) {
            unsigned int e = duel.players[q].field[z].effects;

            while (e != 0) {
                DuelEffect *ef = &duel.effects[e];
                unsigned short src = ef->src;

                e = ef->next;
                if ((ef->kind & 0xF) == 10 && duel.players[src & 1].field[src >> 8].card.id == 0x149D) {
                    duel_088CC1D0(src & 0xFF, src >> 8, 0);
                }
            }
            {
                int id = field_get_card(p, z);

                if ((id == 0x11C9 || id == 0x118A) && tz < 5 && duel.players[q].field[z].link == 0 &&
                    duel_0884F30C(t << 31 | 0x200000 | (tz & 0x1F) << 16 | (field_get_card(p, z) & 0xFFFF),
                                  CARD_UID(duel.players[q].field[z].card), 0) != 0) {
                    duel_088CC178(t & 0xFF, tz, 1);
                }
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* branch layout and argument packing not reproduced */
void duel_088CA184(int a, int b)
{
    if (duel.players[(a & 0xFF) & 1].field[(unsigned short)a >> 8 & 0xFF].card.id != 0 &&
        duel.players[(b & 0xFF) & 1].field[(unsigned short)b >> 8 & 0xFF].card.id == 0) {
        duel_088C9E88(a, b);
        duel_cmd(((a & 0xFF) ? 0x8000 : 0) | 0x3D, a, b, 0);
        duel_088C9F58(a, b);
    }
}
#endif

#ifdef NON_MATCHING /* branch layout and argument packing not reproduced */
void duel_088CA270(unsigned int p, int a, int b)
{
    if (duel.players[(a & 0xFF) & 1].field[(unsigned short)a >> 8 & 0xFF].card.id != 0 &&
        duel.players[(b & 0xFF) & 1].field[(unsigned short)b >> 8 & 0xFF].card.id != 0) {
        duel_088C9E88(a, b);
        duel_088C9E88(b, a);
        duel_cmd((p ? 0x8000 : 0) | 0x3E, a, b, 0);
        duel_088C9F58(a, b);
        duel_088C9F58(b, a);
    }
}
#endif

#ifdef NON_MATCHING /* branch layout and argument packing not reproduced */
void duel_088CA384(unsigned int p, int i, int f)
{
    DuelCard *c = duel_field_card_deb2c(p, 11, i);

    duel_cmd(0x3F, CARD_UID(*c) << 6 | (p & 1) | 0x16, !(f != 0) << 6 | c->owner | 0x1A, 0);
    duel_088AFD5C(0x24, 0);
}
#endif

#ifdef NON_MATCHING /* branch layout and argument packing not reproduced */
int duel_088CA428(unsigned int p, unsigned int uid, int f)
{
    int k;

    if (duel_grave_find_uid(p, uid) < 0) {
        return 0;
    }
    k = card_attribute_icon_is3(duel_cards[uid].id) ? 0xC : 0xD;
    duel_cmd(0x3F, (uid & 0x3FF) << 6 | (p & 1) | 0x1C, !(f != 0) << 6 | (p & 1) | k << 1, 0);
    duel_088AFD5C(0x24, 0);
    return 1;
}
#endif
