// @range 0x088D1DF4-0x088D28F8
// @asm 0x088D1DF4-0x088D1F1C
// @asm 0x088D1F1C-0x088D2288
// @asm 0x088D2468-0x088D2760
// @asm 0x088D2760-0x088D28F8
/* Results of a battle: destroying or moving the cards that fought (duel_special.slot[i]: 0 attacker,
   1 target) and the commands to the presentation. */
#include "duel.h"
#include "card_db.h"

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088CC8EC(struct DuelActX x, DuelCard *c, int a, int b);
void duel_088AFD5C(int kind, struct DuelActX x);
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

#ifdef NON_MATCHING /* jogador e zona extraídos de x trocam s2/s3; o teste z < 5 sai com slti at em vez de slti v1 */
/* Card c (described by x) leaves the field to zone x.s_zone2 (0xE graveyard). */
void duel_088D1DF4(DuelCard *c, struct DuelActX x)
{
    unsigned int p = x.s_player;
    int z = x.s_zone;
    int opp = x.s_b18 != p;
    unsigned char to = x.s_zone2;
    int grave = to == 0xE;
    unsigned int f;

    if (c->id != 0) {
        if (z < 5) {
            duel_cmd((p ? 0x8000 : 0) | 0x32, z, to, opp != 0);
            duel_088CC8EC(x, c, 0, 0);
            duel_088AFD5C(0x16, x);
            f = 6;
            if (opp) {
                f |= 1;
            }
            if (grave) {
                f |= 0x1000000;
            }
            duel_088CFBA4(c, p, z, f);
        }
    }
}
#endif

#ifdef NON_MATCHING /* montagem do evento (struct DuelActX) campo a campo e cálculo de grave não reproduzidos */
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

    BATTLE_PZ(i, q, z);
    s = &duel_special.slot[i];
    id = duel_special_get_id(s);
    uid = CARD_UID(s->card);
    if (duel_act_immune(a, q, z) != 0) {
        return 0;
    }
    if (id != 0) {
        struct DuelActX x = {0};

        f = 2;
        grave = !(duel_088DFCC8(uid) == 0 || card_is_attribute9(id) != 0);
        opp = a->player != q;
        if (id == 0x16F8 && duel_spec_act_ok(&duel_special.act[duel_special.player[0] ^ i]) != 0) {
            grave = 0;
        }
        x.s_uid = uid;
        x.s_player = q;
        x.s_zone = z;
        x.s_b14 = s->pos;
        x.s_b17 = s->faceup;
        x.s_b18 = q ^ opp;
        x.s_b16 = 0;
        x.s_b23 = 0;
        x.s_zone2 = grave ? 0xE : 0xF;
        x.s_b15 = 1;
        if (opp) {
            f = 3;
        }
        duel_cmd((q ? 0x8000 : 0) | 0x31, (z & 0xFF) | 0xFF00, opp << 8 | x.s_zone2, uid & 0x1FF);
        duel_088CC8EC(x, &s->card, 0, 0);
        if (card_is_attribute9(id) == 0) {
            duel_088AFD5C(0x1B, x);
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

#ifdef NON_MATCHING /* montagem do evento (struct DuelActX) campo a campo; não reproduzida */
/* The card that fought (i) is destroyed by the battle. */
int duel_088D2468(int i)
{
    unsigned int q;
    int z;
    DuelSlot *s = &duel_special.slot[i];
    int id;
    int uid;
    int grave;
    unsigned int f;
    struct DuelActX x;

    BATTLE_PZ(i, q, z);
    id = duel_special_get_id(s);
    uid = CARD_UID(s->card);
    if (id == 0) {
        return 0;
    }
    f = 0;
    grave = duel_088DFCC8(uid) != 0 && card_is_attribute9(id) == 0;
    *(unsigned int *)&x = 0;
    if (id == 0x16F8) {
        grave = duel_spec_act_ok(&duel_special.act[duel_special.player[0] ^ i]) == 0 && grave;
    }
    x.s_uid = uid;
    x.s_player = q;
    x.s_zone = z;
    x.s_b14 = s->pos & 1;
    x.s_b17 = s->faceup & 1;
    x.s_b18 = q;
    x.s_b15 = 1;
    x.s_zone2 = grave ? 0xE : 0xF;
    duel_cmd((q ? 0x8000 : 0) | 0x46, z, x.s_zone2, uid & 0x1FF);
    duel_088CC8EC(x, &s->card, 0, 0);
    if (card_is_attribute9(id) == 0) {
        duel_088AFD5C(0x1B, x);
    }
    if (grave) {
        f = 0x1000000;
    }
    duel_088CFBA4(&s->card, q, z, f);
    return 1;
}
#endif

#ifdef NON_MATCHING /* só a ordem de (q & 1) e (z & 0x1F) << 1 no argumento do comando 0x40 difere */
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
            duel_cmd(0x40, (z & 0x1F) << 1 | (q & 1) | 0xFFC0, (o & 1) | 0x5A, opp);
        }
        duel_088CFBA4(&s->card, q, z, opp);
        return 1;
    }
    return 0;
}
#endif
