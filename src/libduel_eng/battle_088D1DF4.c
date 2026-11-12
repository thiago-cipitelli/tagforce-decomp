// @range 0x088D1DF4-0x088D28F8
// @asm 0x088D1F1C-0x088D2288
/* Results of a battle: destroying or moving the cards that fought (duel_special.slot[i]: 0 attacker,
   1 target) and the commands to the presentation. */
#include "duel.h"
#include "card_db.h"

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
struct DuelActX1 {                     /* struct DuelActX with bit 23 as a 1-bit field */
    unsigned int s_uid : 9;
    unsigned int s_player : 1;
    unsigned int s_zone : 4;
    unsigned int s_b14 : 1;
    unsigned int s_b15 : 1;
    unsigned int s_b16 : 1;
    unsigned int s_b17 : 1;
    unsigned int s_b18 : 1;
    unsigned int s_zone2 : 4;
    unsigned int s_b23 : 1;
    unsigned int s_b24 : 8;
};

#ifdef __MWERKS__
void duel_088CC8EC(struct DuelActX x, DuelCard *c, int a, int b);
void duel_088AFD5C(int kind, struct DuelActX x);
#else
void duel_088CC8EC(struct DuelActX1 x, DuelSlot *s, int cause, unsigned int f);
void duel_088AFD5C(unsigned short kind, unsigned int ev);
/* port: the original passes the DuelActX word in a0/a1 as is; the definitions take it as DuelActX1 / unsigned int */
#define duel_088CC8EC(x, c, a, b) duel_088CC8EC(*(struct DuelActX1 *)&(x), (DuelSlot *)(c), a, b)
#define duel_088AFD5C(kind, x) duel_088AFD5C(kind, *(unsigned int *)&(x))
#endif
void duel_088CFBA4(DuelCard *c, unsigned int p, int z, unsigned int f);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_special_get_id(DuelSlot *s);
int duel_088DFCC8(unsigned int uid);
int duel_spec_act_ok(DuelSpecAct *s);

extern unsigned int duel_atk_tp;       /* 0x08913F14 */
extern int duel_atk_zone;              /* 0x08913F2C */
extern int duel_atk_tz;                /* 0x08913F30 */

/* player and zone of the card that fought: 0 the attacker, 1 the target */
#define BATTLE_PZ(i, q, z) ((q) = (i) == 0 ? duel_special.player[0] : duel_atk_tp, (z) = (i) == 0 ? duel_atk_zone : duel_atk_tz)

/* Card c (described by x) leaves the field to zone x.s_zone2 (0xE graveyard). */
void duel_088D1DF4(DuelCard *c, struct DuelActX x)
{
    unsigned int p = x.s_player;
    int z = x.s_zone;
    int opp = x.s_b18 != p;
    unsigned char to = x.s_zone2;
    int grave = to == 0xE;
    unsigned int f;

    if (c->id == 0 || z >= 5) {
        return;
    }
    {
        int o = opp != 0;
        unsigned int k;

        if (p) {
            k = 0x8000;
        } else {
            k = 0;
        }
        duel_cmd(k | 0x32, z, to, o);
        duel_088CC8EC(x, c, 0, 0);
        duel_088AFD5C(0x16, x);
        f = 6;
        if (opp) {
            f |= 1;
        }
        if (grave) {
            f |= 0x1000000;
        }
        duel_088CFBA4(c, x.s_player, z, f);
    }
}

#ifdef NON_MATCHING /* 8 palavras: lógica conferida; o original antecipa a carga de a->player para o delay slot antes de card_is_attribute9 e troca a0/v0 no laço de zeragem de x */
/* The card that fought (i) is destroyed by a's effect. */
int duel_088D1F1C(DuelAct *a, int i)
{
    unsigned int q;
    int z;
    DuelSlot *s;
    int id;
    int uid;
    int grave;
    unsigned int f;
    int opp;
    struct DuelActX1 x;

    BATTLE_PZ(i, q, z);
    s = &duel_special.slot[i];
    id = duel_special_get_id(s);
    uid = CARD_UID(s->card);
    if (duel_act_immune(a, q, z) != 0) {
        return 0;
    }
    if (id != 0) {
        f = 2;
        grave = !((duel_088DFCC8(uid) != 0 ^ 1) || card_is_attribute9(id) != 0);
        opp = a->player != q;
        {
            int n = 4;
            unsigned char *d = (unsigned char *)&x;

            if (d) {
                do {
                    *d++ = 0;
                } while (--n);
            }
        }
        if (id == 0x16F8 && duel_spec_act_ok(&duel_special.act[duel_special.player[0] ^ i]) != 0) {
            grave = 0;
        }
        x.s_uid = uid;
        x.s_player = q;
        x.s_zone = z;
        x.s_b14 = s->pos;
        x.s_b17 = s->faceup;
        x.s_b18 = q ^ opp;
        x.s_b15 = 1;
        x.s_b23 = 0;
        x.s_zone2 = grave ? 0xE : 0xF;
        if (opp) {
            f |= 1;
        }
        duel_cmd((q ? 0x8000 : 0) | 0x31, (z & 0xFF) | 0xFF00, (unsigned char)((opp & 0xF) | (x.s_b23 & 0xF) << 4) << 8 | x.s_zone2,
                 x.s_uid);
        duel_088CC8EC(*(struct DuelActX *)&x, &s->card, 0, 0);
        if (card_is_attribute9(id) == 0) {
            duel_088AFD5C(0x1B, *(struct DuelActX *)&x);
        }
        if (grave) {
            f |= 0x1000000;
        }
        duel_088CFBA4(&s->card, q, z, f);
        return 1;
    }
    return 0;
}
#endif

/* The card that fought (i) is returned to the hand by a's effect (f: command flags). */
int duel_088D2288(DuelAct *a, int i, unsigned int f)
{
    unsigned int q;
    int z;
    DuelSlot *s;
    int id;
    int uid;

    BATTLE_PZ(i, q, z);
    s = &duel_special.slot[i];
    id = duel_special_get_id(s);
    uid = CARD_UID(s->card);
    if (duel_act_immune(a, q, z) != 0) {
        return 0;
    }
    switch (id) {
    case 0x1A8D:
        if (duel_spec_act_ok(&duel_special.act[duel_special.player[0] ^ i]) != 0) {
            return 0;
        }
    }
    if (id != 0) {
        if (card_is_attribute9(id) != 0) {
            duel_cmd((q ? 0x8000 : 0) | 0x31, (z & 0xFF) | 0xFF00, 0x10, uid);
        } else {
            if (a->player != q) {
                f |= 1;
            }
            duel_cmd((q ? 0x8000 : 0) | 0x33, (z & 0xFF) | 0xFF00, f, uid);
        }
        duel_088CFBA4(&s->card, q, z, f);
        return 1;
    }
    return 0;
}

/* The card that fought (i) is destroyed by the battle. */
int duel_088D2468(int i)
{
    unsigned int q;
    int z;
    DuelSlot *s;
    int id;
    int uid;
    unsigned int f;
    int grave;
    struct DuelActX1 x;

    BATTLE_PZ(i, q, z);
    s = &duel_special.slot[i];
    id = duel_special_get_id(s);
    uid = CARD_UID(s->card);
    if (id != 0) {
        f = 0;
        grave = !((duel_088DFCC8(uid) != 0 ^ 1) || card_is_attribute9(id) != 0);
        {
            unsigned char *d = (unsigned char *)&x;
            int n = 4;

            if (d) {
                do {
                    *d++ = 0;
                } while (--n);
            }
        }
        if (id == 0x16F8 && duel_spec_act_ok(&duel_special.act[duel_special.player[0] ^ i]) != 0) {
            grave = 0;
        }
        x.s_uid = uid;
        x.s_player = q;
        x.s_zone = z;
        x.s_b14 = s->pos;
        x.s_b17 = s->faceup;
        x.s_b18 = q;
        x.s_b15 = 1;
        x.s_b23 = 0;
        x.s_zone2 = grave ? 0xE : 0xF;
        duel_cmd((q ? 0x8000 : 0) | 0x46, z, x.s_zone2, x.s_uid);
        duel_088CC8EC(*(struct DuelActX *)&x, &s->card, 0, 0);
        if (card_is_attribute9(id) == 0) {
            duel_088AFD5C(0x1B, *(struct DuelActX *)&x);
        }
        if (grave) {
            f |= 0x1000000;
        }
        duel_088CFBA4(&s->card, q, z, f);
        return 1;
    }
    return 0;
}

/* The card that fought (i) is banished/moved away by a's effect. */
int duel_088D2760(DuelAct *a, int i)
{
    int opp = 0;
    unsigned int q;
    int z;
    DuelSlot *s;
    int id;
    int uid;

    BATTLE_PZ(i, q, z);
    s = &duel_special.slot[i];
    id = duel_special_get_id(s);
    uid = CARD_UID(s->card);
    if (duel_act_immune(a, q, z) != 0) {
        return 0;
    }
    if (id != 0) {
        if (card_is_attribute9(id) != 0) {
            duel_cmd((q ? 0x8000 : 0) | 0x31, (z & 0xFF) | 0xFF00, 0x10, uid);
        } else {
            unsigned int o = s->card.owner;

            if (a->player != q) {
                opp |= 1;
            }
            duel_cmd(0x40, ((z & 0x1F) * 2 | ((int)q & 1)) | 0xFFC0, (o & 1) | 0x5A, opp);
        }
        duel_088CFBA4(&s->card, q, z, opp);
        return 1;
    }
    return 0;
}
