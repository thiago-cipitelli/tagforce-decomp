// @range 0x088C936C-0x088CA4E8
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

int duel_slot_immune(unsigned int p, int z, int opp);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);

extern DuelCard duel_cards[512];

/* card of p in zone z turned face up (by the card id, the opponent when opp); with n, a monster whose
   special group reaches n triggers its flip effect (0x18200000) */
void duel_088C936C(unsigned int p, int z, int n, int id, int opp)
{
    unsigned int cid = duel.players[p & 1].field[z].card.id;
    int noid = !id;
    int down;
    int k;
    struct DuelActX ev;

    if (card_get_type(id) == 0x17) {
        if (duel_slot_ref_immune(p, z, opp) != 0) {
            return;
        }
    } else if (card_get_type(id) == 0x16) {
        if (duel_slot_immune(p, z, opp) != 0) {
            return;
        }
    }
    if (cid == 0) {
        return;
    }
    down = !duel.players[p & 1].field[z].faceup;
    ev.s_player = p;
    ev.s_zone = z;
    ev.s_b14 = duel.players[p & 1].field[z].pos;
    ev.s_b15 = down;
    ev.s_uid = CARD_UID(duel.players[p & 1].field[z].card);
    ev.s_b16 = opp;
    ev.s_b17 = 0;
    if (n) {
        n = card_get_special_group(cid) >= n;
    }
    k = noid && z < 5 && down;
    duel_cmd((p ? 0x8000 : 0) | 0x35, z, !duel.players[p & 1].field[z].faceup, k);
    if (z < 5) {
        if (down) {
            switch (cid) {
            case 0x1174:
            case 0x133B:
                if ((noid || n) && duel_count_active(0x148E) == 0) {
                    duel_088C87A8(p, z, 0x1174, 2, 0);
                }
                break;
            }
            if (n) {
                duel.players[p & 1].field[z].faceup = 1;
                duel_0884F30C((cid & 0xFFFF) | ((p << 31 & 0x80000000) | 0x18200000 | (z << 16 & 0x1F0000)), ev.s_uid,
                              *(unsigned int *)&ev);
                duel.players[p & 1].field[z].faceup = 0;
            }
            duel_088AFD5C(0xC, *(unsigned int *)&ev);
        } else {
            duel_088AFD5C(9, *(unsigned int *)&ev);
        }
    }
}
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088C77D4(unsigned int p, unsigned short id, unsigned short a, unsigned short b);
void duel_088C89BC(unsigned int p, int z, int id, int c);

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

/* monster of p in zone z changes to face up (x) or face down, by the card id (the opponent when opp); with y, a
   flip to face up of a monster whose special group reaches y triggers its flip effect */
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp)
{
    unsigned int cid = field_get_card(p, z);
    unsigned int up;
    int def;
    struct DuelActX ev;

    if (z >= 5) {
        return 0;
    }
    if (cid != 0) {
        up = duel.players[p & 1].field[z].faceup;
        def = !duel.players[p & 1].field[z].pos;
        ev.s_player = p;
        ev.s_zone = z;
        ev.s_b14 = def;
        ev.s_b15 = up ^ x;
        ev.s_uid = CARD_UID(duel.players[p & 1].field[z].card);
        ev.s_b16 = opp;
        ev.s_b17 = 0;
        if (y) {
            y = card_get_special_group(cid) >= y;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x34, z, x, !id);
        if (id == 0) {
            duel_088C7E68(p, z, 0x15, 1);
        }
        if ((x == 0 && up != 0) || (x != 0 && up == 0)) {
            if (!def) {
                switch (cid) {
                case 0x1005:
                    duel_0884F30C((cid & 0xFFFF) | ((p << 31 & 0x80000000) | 0x16200000 | (z << 16 & 0x1F0000)), ev.s_uid,
                                  *(unsigned int *)&ev);
                    break;
                }
            } else {
                switch (cid) {
                case 0x101E:
                case 0x1048:
                case 0x1197:
                case 0x1868:
                    duel_0884F30C((cid & 0xFFFF) | ((p << 31 & 0x80000000) | 0x16200000 | (z << 16 & 0x1F0000)), ev.s_uid,
                                  *(unsigned int *)&ev);
                    break;
                }
            }
        }
        if (y && x && up == 0 && card_get_special_group(cid) != 0) {
            switch (cid) {
            case 0x1174:
            case 0x133B:
                if (duel_count_active(0x148E) == 0) {
                    duel_088C87A8(p, z, 0x1174, 2, 0);
                }
                break;
            }
            duel.players[p & 1].field[z].faceup = 1;
            duel_0884F30C((cid & 0xFFFF) | ((p << 31 & 0x80000000) | 0x16200000 | (z << 16 & 0x1F0000)), ev.s_uid,
                          *(unsigned int *)&ev);
            duel.players[p & 1].field[z].faceup = 0;
        }
        if (def) {
            unsigned int e = duel.players[p & 1].field[z].effects;

            while (e != 0) {
                DuelEffect *ef = &duel.effects[e];

                e = ef->next;
                if ((ef->kind & 0xF) < 6 && ef->src == 0x1A0B) {
                    duel_088C77D4(ef->arg, 0x1A0B, 1, 0);
                }
            }
            duel_088C89BC(p, z, 0x1A0B, 1);
        }
        if (x != 0 && up != 0) {
            duel_088AFD5C(9, *(unsigned int *)&ev);
        } else {
            duel_088AFD5C(0xB, *(unsigned int *)&ev);
        }
        return 1;
    }
    return 0;
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

void duel_088C9E88(int loc, int t)
{
    unsigned int p = loc & 0xFF;
    unsigned int z = (unsigned short)loc >> 8 & 0xFF;
    unsigned int s;

    if (p != (t & 0xFF) && duel.players[p & 1].field[z].faceup != 0) {
        switch (field_get_card(p, z)) {
        case 0x14AC:
            s = (unsigned short)duel_slot_e1854(p, z);
            if (s != 0xFFFF) {
                duel_088C88B8(s & 0xFF, (unsigned short)s >> 8 & 0xFF, (unsigned short)((p & 0xFF) | (z & 0xFF) << 8), 1);
            }
            break;
        }
    }
}

void duel_088C9F58(int loc, int t)
{
    unsigned int p = loc & 0xFF;
    unsigned int z = (unsigned short)loc >> 8 & 0xFF;
    unsigned int tp = t & 0xFF;
    int tz = (unsigned short)t >> 8 & 0xFF;
    unsigned int e;

    if (p != tp && duel.players[p & 1].field[z].faceup != 0) {
        e = duel.players[p & 1].field[z].effects;
        while (e != 0) {
            DuelEffect *ef = &duel.effects[e];
            unsigned short src = ef->src;
            unsigned short sp = src & 0xFF;
            unsigned short sz = src >> 8 & 0xFF;

            e = ef->next;
            switch (ef->kind & 0xF) {
            case 10:
                switch (duel.players[sp & 1].field[sz].card.id) {
                case 0x149D:
                    duel_088CC1D0(sp, sz, 0);
                    break;
                }
                break;
            }
        }
        switch (field_get_card(p, z)) {
        case 0x118A:
        case 0x11C9:
            if (tz < 5 && duel.players[p & 1].field[z].link == 0 &&
                duel_0884F30C((tp << 31 & 0x80000000) | 0x200000 | (tz << 16 & 0x1F0000) | (field_get_card(p, z) & 0xFFFF),
                              (unsigned short)CARD_UID(duel.players[p & 1].field[z].card), 0) != 0) {
                duel_088CC178(tp, tz, 1);
            }
            break;
        }
    }
}

void duel_088CA184(int a, int b)
{
    unsigned int pa = a & 0xFF;
    unsigned int za = (unsigned short)a >> 8 & 0xFF;
    unsigned int pb = b & 0xFF;
    unsigned int zb = (unsigned short)b >> 8 & 0xFF;

    if (duel.players[pa & 1].field[za].card.id != 0) {
        if (duel.players[pb & 1].field[zb].card.id == 0) {
            duel_088C9E88(a, b);
            {
                extern void duel_cmd(unsigned short, int, int, int);
                duel_cmd((pa ? 0x8000 : 0) | 0x3D, a, b, 0);
            }
            duel_088C9F58(a, b);
        }
    }
}

void duel_088CA270(unsigned int p, int a, int b)
{
    unsigned short ib = duel.players[(unsigned char)b & 1].field[(unsigned short)b >> 8 & 0xFF].card.id;

    if ((int)duel.players[(unsigned char)a & 1].field[(unsigned short)a >> 8 & 0xFF].card.id > 0 && ib > 0) {
        duel_088C9E88(a, b);
        duel_088C9E88(b, a);
        {
            extern void duel_cmd(unsigned short, int, int, int);
            duel_cmd((p ? 0x8000 : 0) | 0x3E, a, b, 0);
        }
        duel_088C9F58(a, b);
        duel_088C9F58(b, a);
    }
}

void duel_088CA384(unsigned int p, int i, int f)
{
    DuelCard *c = duel_field_card_deb2c(p, 11, i);
    unsigned int uid = CARD_UID(*c);

    duel_cmd(0x3F, (uid & 0x3FF) << 6 | ((p & 1) | 0x16), !f << 6 | ((c->owner & 1) | 0x1A), 0);
    duel_088AFD5C(0x24, 0);
}

int duel_088CA428(unsigned int p, unsigned int uid, int f)
{
    int k;

    if (duel_grave_find_uid(p, uid) >= 0) {
        k = card_attribute_icon_is3(duel.list[uid].id) ? 0xC : 0xD;
        duel_cmd(0x3F, (uid & 0x3FF) << 6 | ((p & 1) | 0x1C), !f << 6 | ((p & 1) | (k & 0x1F) << 1), 0);
        duel_088AFD5C(0x24, 0);
        return 1;
    }
    return 0;
}

