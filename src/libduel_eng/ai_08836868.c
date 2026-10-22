// @range 0x08836868-0x08837500
// @asm 0x08837250-0x088373A4
/* Whether a player may still attack (battle phase locks, cards that force or forbid attacks). */
#include "duel.h"

extern unsigned int duel_turn;
extern int duel_phase;
extern int duel_w_0891960C;            /* battle step */
extern int duel_w_08919610;
extern int duel_w_08919614;

typedef struct AiCard {                /* view of DuelCard: owner and bit 14 together */
    unsigned int id : 13;
    unsigned int ob : 2;
    unsigned int rest : 17;
} AiCard;

typedef struct AiSnap {                /* 0x44: attacker and targets when the attack was declared */
    int uid;
    int ob;
    int uids[5];
    int obs[5];
    int can[5];
} AiSnap;

/* fields of duel_special used by the battle, addressed on their own */
extern unsigned int duel_atk_tp;       /* 0x08913F14 target player */
extern int duel_atk_direct;            /* 0x08913F18 */
extern int duel_atk_w28;               /* 0x08913F28 */
extern int duel_atk_zone;              /* 0x08913F2C attacker zone */
extern int duel_atk_tz;                /* 0x08913F30 target zone */
extern AiSnap duel_atk_snap;           /* 0x08913FFC */

#define SNAP (&duel_atk_snap)

int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_has_effect2(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_grave_type(unsigned int p, unsigned int type);
int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_can_attack(unsigned int p, int z, int arg);
int duel_can_attack_any(unsigned int p, int z, int arg);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
int duel_count_active(int id);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
int duel_slot_affected_ex(unsigned int p, int z, int id);
int duel_count_e17a0(unsigned int p, int id);
int duel_slot_immune(unsigned int p, int z, int opp);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, int c);
int memcmp2(const void *a, const void *b, int n);
int duel_can_direct_attack(unsigned int p, int z);
int duel_direct_attack_kind(unsigned int p, int z, int arg);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_host_call(int a, int b, int c, int d);
int duel_slot_effect12_arg(unsigned int p, int z, int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088C8AD0(unsigned int p, int z, int id, int v);
void duel_0884ACD8(unsigned int p, int msg);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);

int duel_08836868(unsigned int p)
{
    if (duel_turn == 0) {
        return 0;
    }
    if ((unsigned int)duel_phase > 3) {
        return 0;
    }
    if (duel_slot_has_effect(p, 0xB, 0x13EB)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 0xB, 0x1475)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 0xB, 0x15F0)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 0xB, 0x173F)) {
        return 0;
    }
    if (duel_find_effect_card(0, 0x16D4, -1) && !duel_count_grave_type(0, 0x16)) {
        return 0;
    }
    if (duel_find_effect_card(1, 0x16D4, -1) && !duel_count_grave_type(1, 0x16)) {
        return 0;
    }
    return 1;
}

int duel_0883698C(unsigned int p)
{
    if (duel_slot_has_effect(p, 0xB, 0x16A1)) {
        return 1;
    }
    return duel_slot_has_effect2(p, 0xB, 0x1502) != 0;
}

int duel_088369D8(unsigned int p)
{
    int i;
    int j;
    int k;

    if (duel_phase != 3 && !duel_08836868(p)) {
        return 1;
    }
    if (duel_0883698C(p)) {
        return 1;
    }
    if (duel_slot_has_effect(p, 0xB, 0x15FF)) {
        i = 1 - p;
        switch (duel_slot_effect_arg(i, 0xB, 0x15FF)) {
        case 0:
            break;
        case -1:
        case 1:
            for (j = 0; j < 5; j++) {
                if (!duel_can_attack(p, j, 1)) {
                    continue;
                }
                for (k = 0; k <= 4; k++) {
                    if (!duel_can_attack_target(p, j, i, k, 1)) {
                        continue;
                    }
                    return 0;
                }
            }
            break;
        }
    }
    if (duel_slot_has_effect(p, 0xB, 0x1669) || duel_slot_has_effect(p, 0xB, 0x14A6) || duel_count_active(0x1443)) {
        for (i = 0; i < 5; i++) {
            if (!duel_can_attack_any(p, i, 1)) {
                continue;
            }
            return 0;
        }
    }
    for (i = 0; i < 5; i++) {
        if (duel_slot_e8170(p, i, 0x16BF) && duel_can_attack_any(p, i, 1)) {
            return 0;
        }
    }
    for (i = 0; i < 5; i++) {
        k = 1 - p;
        if (duel_slot_e8170(p, i, 0x1A79) && duel.players[p & 1].field[i].link && duel_can_attack(p, i, 1)) {
            for (j = 0; j <= 4; j++) {
                if (!duel_can_attack_target(p, i, k, j, 1)) {
                    continue;
                }
                return 0;
            }
        }
    }
    return 1;
}

int duel_08836C98(unsigned int p, int z)
{
    unsigned int q;

    if (!duel_can_attack_any(p, z, 0)) {
        return 1;
    }
    if (duel_slot_has_effect(p, 0xB, 0x15FF)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 0xB, 0x14A6)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 0xB, 0x1669)) {
        return 0;
    }
    if (duel_count_active(0x1443)) {
        return 0;
    }
    if (duel_slot_e8170(p, z, 0x16BF)) {
        return 0;
    }
    if (duel_slot_e8170(p, z, 0x1A79) &&
        *(unsigned int *)((char *)&duel.players[0].field[0].link + (p & 1) * 0xFAC + z * 20)) {
        return 0;
    }
    if (duel_slot_effect_kind(p, z, 0x16CB, 4)) {
        return 0;
    }
    q = 1 - p;
    if (duel_slot_has_effect(q, 0xB, 0x177A)) {
        return 0;
    }
    if (duel_slot_affected_ex(p, z, 0x1561)) {
        return 0;
    }
    if (duel_slot_affected_ex(p, z, 0x1852)) {
        return 0;
    }
    if (duel_count_e17a0(q, 0x1318) && !duel_slot_immune(p, z, 1)) {
        return 0;
    }
    return 1;
}

void duel_08836E74(unsigned int p, int z)
{
    duel_w_0891960C = 1;
    duel_w_08919610 = 0;
    if (duel_atk_w28 != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x1B, z, 1, 1);
    }
}

void duel_08836ED0(AiSnap *s)
{
    int i;

    s->uid = CARD_UID(duel.players[duel_special.player[0] & 1].field[duel_atk_zone].card);
    s->ob = ((AiCard *)&duel.list[s->uid])->ob;
    for (i = 0; i < 5; i++) {
        s->uids[i] = CARD_UID(duel.players[duel_atk_tp & 1].field[i].card);
        s->obs[i] = ((AiCard *)&duel.list[s->uids[i]])->ob;
        s->can[i] = duel_can_attack_target(duel_special.player[0], duel_atk_zone, duel_atk_tp, i, 0);
    }
}

int duel_08837020(void)
{
    int bad = 0;
    AiSnap s;

    duel_08836ED0(&s);
    if (SNAP->uid != s.uid || SNAP->ob != s.ob) {
        duel_atk_direct = 0;
        duel_w_08919610 = 0;
        duel_w_0891960C = 1;
        return 1;
    }
    if (!duel_can_attack(duel_special.player[0], duel_atk_zone, 0)) {
        bad = 1;
    }
    if (memcmp2(SNAP, &s, 0x44)) {
        bad = 1;
    }
    if (duel_atk_direct != 0) {
        if (!duel_can_direct_attack(duel_special.player[0], duel_atk_zone) &&
            !duel_direct_attack_kind(duel_special.player[0], duel_atk_zone, 1)) {
            bad = 1;
        }
    } else if (!duel_can_attack_target(duel_special.player[0], duel_atk_zone, duel_atk_tp, duel_atk_tz, 0)) {
        bad = 1;
    }
    if (duel_slot_has_effect(duel_atk_tp, 0xB, 0x177A)) {
        bad = 1;
    }
    if (bad) {
        if (!duel_can_attack_any(duel_special.player[0], duel_atk_zone, 0)) {
            duel_cmd((duel_special.player[0] ? 0x8000 : 0) | 0x1B, duel_atk_zone, 1, 0);
            duel_atk_direct = 0;
            duel_w_08919610 = 0;
            duel_w_0891960C = 1;
            return 1;
        }
        duel_atk_direct = 0;
        duel_w_08919610 = 0;
        duel_w_0891960C = 2;
        duel_atk_w28 = 1;
        duel_host_call(0xB, duel_atk_zone, 1 - duel_special.player[0], -1);
        duel_088C7E68(duel_special.player[0], duel_atk_zone, 0x15, 1);
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* leiaute dos retornos 0/1 depois do teste duel_atk_direct não reproduzido */
int duel_08837250(void)
{
    if (SNAP->uid == CARD_UID(duel.players[duel_special.player[0] & 1].field[duel_atk_zone].card)) {
        if (SNAP->ob != ((AiCard *)&duel.list[SNAP->uid])->ob) {
            return 1;
        }
        if (duel_atk_direct != 0) {
            return 0;
        }
        if (SNAP->uids[duel_atk_tz] == CARD_UID(duel.players[duel_atk_tp & 1].field[duel_atk_tz].card)) {
            if (SNAP->obs[duel_atk_tz] != ((AiCard *)&duel.list[SNAP->uids[duel_atk_tz]])->ob) {
                return 1;
            }
            return 0;
        }
    }
    return 1;
}
#endif

int duel_088373A4(unsigned int p)
{
    int i;

    switch (duel_w_08919610) {
    case 0:
        if (duel_0883698C(p)) {
            duel_088C89BC(p, 0xB, 0x16A1, 1);
            duel_0884ACD8(p, 0x136);
            duel_w_0891960C = 0xD;
            duel_w_08919610 = 0;
            duel_w_08919614 = 1;
            return 0;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x18, 1, 0, 0);
        duel_cmd((p ? 0x8000 : 0) | 0xF, 0, 0, 0);
        duel_w_08919610++;
    case 1:
        i = duel_slot_effect12_arg(p, 0xB, 0x1A7A);
        if (i >= 0) {
            duel_088C8AD0(p, 0xB, 0x1A7A, i);
            duel_0884F30C((p << 31 & 0x80000000) | 0xA501A7A, 0, i);
            return 0;
        }
        break;
    }
    return 1;
}
