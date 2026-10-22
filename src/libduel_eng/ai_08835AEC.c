// @range 0x08835AEC-0x08836868
// @data 0x08907A30-0x08907A9C
/* What the computer (or the local player) may pick for the current choice (duel_w_08919630). */
#include "duel.h"

extern unsigned int duel_view_player;
extern int duel_err;
extern int duel_w_08919630;            /* kind of choice */
extern int duel_w_08919634;
extern int (*duel_w_08919660)();       /* filter of the choice */
extern DuelAct *duel_w_089147DC;
extern unsigned int duel_turn_player;
extern int duel_phase;

typedef struct AiTab {                 /* 0x10 */
    int id;
    int zone;
    int (*check)(unsigned int p, int id);
    int (*apply)(unsigned int p, int id, int uid);
} AiTab;

extern AiTab duel_ai_tab[23];          /* 0x08907A9C */

typedef struct DuelCandList2 {         /* 0x0891524C */
    int pad0[3];
    unsigned int n;                    /* +0x0C */
    DuelCard cards[512];               /* +0x10 */
    unsigned short zones[512];         /* +0x810 */
} DuelCandList2;

extern DuelCandList2 duel_w_0891524C;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_w_08919640;
extern int duel_w_08919638;
extern int duel_w_08919644;
extern int duel_w_08919628;
extern int duel_w_0891962C;
extern struct {
    unsigned char b0 : 3;
    unsigned char b3 : 1;
    unsigned char b4 : 2;
    unsigned char b6 : 2;
} duel_w_08914A2A;

int field_get_card(unsigned int p, int z);
unsigned int duel_08834A84(int id, unsigned int p, int i);
unsigned int duel_08834E44(int id, unsigned int p, int z);
unsigned int duel_08835274(int id, unsigned int p, int z);
unsigned short duel_08835834(unsigned int p, int z);
unsigned short duel_088358F8(unsigned int p, int z);
int duel_08835620(unsigned int p, int z, unsigned int i);
int duel_08835740(DuelAct *a, unsigned int p, int z, unsigned int i);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
int duel_0886B0E4(unsigned int code, unsigned short uid, int b);
int duel_grave_find_name(unsigned int p, int id);
int duel_d_find_name(unsigned int p, int id);
int duel_c_find_name(unsigned int p, int id);
unsigned int duel_08828740(unsigned int p, int uid);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
void duel_0882AF5C(int a, int b, int c);
void duel_088C8AD0(unsigned int p, int z, int id, int v);
int duel_slot_has_effect12_arg(unsigned int p, int z, int id, int arg);
int duel_zone_card_id(unsigned int p, int z, int i);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_088362FC(unsigned int p, int id, int uid);
int duel_0883623C(unsigned int p, int id);
int duel_08827DA4(unsigned int p, int uid, int a, int b);

unsigned int duel_08835AEC(unsigned int p, int z, unsigned int i)
{
    unsigned int r;

    if (duel_w_08919634 == 0) {
        return 0;
    }
    switch (duel_w_08919630) {
    case 0:
        break;
    case 1:
        switch (z) {
        case 0xD:
        case 0xC:
        case 0xE:
        case 0xF:
            r = duel_08835834(p, z) | duel_088358F8(p, z);
            if (p == duel_view_player && z == 0xD) {
                r |= 0x1000;
            }
            return r;
        }
        break;
    case 2:
    case 3:
        switch (z) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            z += i;
            if (duel.players[p & 1].field[z].card.id) {
                r = duel_08834E44(field_get_card(p, z), p, z);
            } else {
                r = 0;
            }
            return r;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            z += i;
            if (duel.players[p & 1].field[z].card.id) {
                r = duel_08835274(field_get_card(p, z), p, z);
            } else {
                r = 0;
            }
            return r;
        case 0xB:
            if (i >= duel.players[p & 1].n_hand) {
                return 0;
            }
            return duel_08834A84(duel.players[p & 1].hand[i].id, p, i);
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
            r = duel_08835834(p, z) | duel_088358F8(p, z);
            if (p == duel_view_player && z == 0xD) {
                r |= 0x400;
            }
            return r;
        }
        break;
    case 4:
        return duel_08835620(p, z, i);
    case 5:
        return duel_08835740(&duel_chain.acts[duel_chain.len - 1], p, z, i);
    case 6:
        if (p == duel_view_player && z == 0xB) {
            if (i >= duel.players[p & 1].n_hand) {
                return 0;
            }
            return duel_08827DA4(p, CARD_UID(duel.players[p & 1].hand[i]), 1, duel_w_08914A2A.b4);
        }
        break;
    case 7:
    case 9:
        if (duel_w_08919660 != 0) {
            if (z == 0xB && i >= duel.players[p & 1].n_hand) {
                return 0;
            }
            return duel_w_08919660(p, z, i);
        }
        break;
    case 8:
    case 10:
        if (duel_w_08919660 != 0 && (int)(z + i) < 0xB) {
            if (duel_w_08919660(duel_w_089147DC, p, z + i)) {
                return 0x800;
            }
            duel_err = 9;
        }
        break;
    }
    return 0;
}

void duel_08835F34(unsigned int p, int z, int i, int k)
{
    duel_w_0891964C = p;
    duel_w_08919650 = z;
    duel_w_08919654 = i;
    duel_w_08919640 = k;
    duel_w_08919638 = 1;
    duel_w_08919644 = 0;
    duel_w_08919628 = duel_field_card_deb2c(p, z, i)->id;
    duel_w_0891962C = CARD_UID(*duel_field_card_deb2c(p, z, i));
    duel_w_08919630 = 0;
    if (z < 5) {
        duel_w_08919628 = field_get_card(p, z + i);
    }
}

void duel_08836028(void)
{
    duel_w_08919630 = 0;
    duel_w_08919640 = 0;
}

void duel_0883603C(void)
{
    if (duel_w_08919630 != 0) {
        duel_w_08919630 = 0;
        duel_w_08919638 = 1;
        duel_w_08919640 = 0xD;
    }
}

int duel_08836074(unsigned int p, int id)
{
    return duel_0886B0E4((p << 31 & 0x80000000) | 0x500000 | (id & 0xFFFF), 0, 0);
}

int duel_0883609C(unsigned int p, int id)
{
    unsigned int code = (p << 31 & 0x80000000) | 0x4E0000 | (id & 0xFFFF);
    int i = duel_grave_find_name(p, id);

    if (i < 0) {
        return 0;
    }
    return duel_0886B0E4(code, CARD_UID(duel.players[p & 1].grave[i]), 0);
}

unsigned int duel_08836144(unsigned int p, int id)
{
    int i = duel_d_find_name(p, id);

    if (i < 0) {
        return 0;
    }
    return duel_08828740(p, CARD_UID(duel.players[p & 1].zone_d[i]));
}

unsigned int duel_088361C0(unsigned int p, int id)
{
    int i = duel_c_find_name(p, id);

    if (i < 0) {
        return 0;
    }
    return duel_08828740(p, CARD_UID(duel.players[p & 1].zone_c[i]));
}

int duel_0883623C(unsigned int p, int id)
{
    return duel_0884F30C((p << 31 & 0x80000000) | 0x500000 | (id & 0xFFFF), 0, 0);
}

int duel_08836264(unsigned int p, int id, int uid)
{
    return duel_0884F30C((p << 31 & 0x80000000) | 0x4E0000 | (id & 0xFFFF), uid, 0);
}

int duel_0883628C(unsigned int p, int a, int b)
{
    duel_w_0891964C = p;
    duel_w_0891962C = b;
    duel_0882AF5C(1, 1, 0);
    return 1;
}

int duel_088362C4(unsigned int p, int a, int b)
{
    duel_w_0891964C = p;
    duel_w_0891962C = b;
    duel_0882AF5C(1, 1, 0);
    return 1;
}

int duel_088362FC(unsigned int p, int id, int uid)
{
    if (duel_0884F30C((p << 31 & 0x80000000) | 0x464E0000 | (id & 0xFFFF), uid, 0) != 0) {
        duel_088C8AD0(p, 0xB, 0x1A04, uid);
        return 1;
    }
    return 0;
}

void duel_08836370(unsigned int p, int a, int z)
{
    int n = 0;
    int big = 0;
    unsigned int i;
    AiTab *t;
    int list[32];
    int k;
    int id;
    int v;
    int n2;

    for (i = 0; i < duel_chain.n_kinds; i++) {
        if (duel_chain.kinds[i] > 5) {
            big = 1;
        }
    }
    for (i = 0, t = duel_ai_tab; i < 0x17; i++, t++) {
        if (z != t->zone) {
            continue;
        }
        id = t->id;
        if (!big || id == 0x1AAC || id == 0x1B2C) {
            if (t->check(p, id)) {
                list[n++] = i;
            }
        }
    }
    if (duel_turn_player == p && (duel_phase == 2 || duel_phase == 4) && !big && z == 0xE &&
        (k = duel.players[p & 1].effects) != 0) {
        do {
            DuelEffectBits *e = (DuelEffectBits *)&duel.effects[k];
            int uid;

            k = e->next;
            if (e->kind == 1 && e->src == 0x1A04 && e->kind_hi == 0) {
                uid = e->arg;
                if (duel_grave_find_uid(p, uid) >= 0 && duel_0886B0E4((p << 31 & 0x80000000) | 0x464E0000 | duel.list[uid].id, uid, 0)) {
                    list[n++] = -uid;
                }
            }
        } while (k != 0);
    }
    for (big = 0; big < n; big++) {
        v = list[big];
        if (v < 0) {
            duel_w_0891524C.cards[duel_w_0891524C.n] = *duel_field_card_deb2c(p, z, duel_grave_find_uid(p, -v));
            duel_w_0891524C.zones[duel_w_0891524C.n] = z;
            duel_w_0891524C.n += duel_w_0891524C.n < 0x1FF;
        } else {
            id = duel_ai_tab[v].id;
            if (duel_ai_tab[v].apply != (void *)duel_0883623C) {
                for (n2 = duel_zone_count(p, z) - 1; n2 >= 0; n2--) {
                    if (id == duel_zone_card_id(p, z, n2)) {
                        duel_w_0891524C.cards[duel_w_0891524C.n] = *duel_field_card_deb2c(p, z, n2);
                        duel_w_0891524C.zones[duel_w_0891524C.n] = z;
                        duel_w_0891524C.n += duel_w_0891524C.n < 0x1FF;
                    }
                }
            } else {
                DuelCard c = {0};

                c.id = id;
                c.owner = p;
                duel_w_0891524C.cards[duel_w_0891524C.n] = c;
                duel_w_0891524C.zones[duel_w_0891524C.n] = 0x10;
                duel_w_0891524C.n += duel_w_0891524C.n < 0x1FF;
            }
        }
    }
}

void duel_088367A8(unsigned int p, int z, int id, int uid)
{
    unsigned int i;
    AiTab *t;

    if (z == 0xE && duel_slot_has_effect12_arg(p, 0xB, 0x1A04, uid)) {
        duel_088362FC(p, id, uid);
        return;
    }
    for (i = 0, t = duel_ai_tab; i < 0x17; i++, t++) {
        if (id == t->id && t->apply(p, id, uid)) {
            return;
        }
    }
}
