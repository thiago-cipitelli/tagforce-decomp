// @range 0x0888F420-0x08890094
// @asm 0x0888F45C-0x0888F6B0
// @asm 0x0888F82C-0x08890094
/* Card effect handlers (DuelAct *a), after effects_zb.c. */
#include "duel.h"

extern int duel_step;
extern unsigned short duel_w_0891968C;

int duel_0887DB24(DuelAct *a);
void duel_088C7DD4(unsigned int p, int a, int b);
void duel_08832880(unsigned int p, int a, int b);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_pick_st_zone(unsigned int p);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
int duel_088BFF78(unsigned int p, int id, int b);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_0884C588(unsigned int p, int a, int id, int b);
int duel_0884C804(void);
DuelCard *duel_0884C7CC(void);
void duel_088CC178(unsigned int p, int z, int a);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088C44E4(unsigned int p, int id);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C78E0(unsigned int p);
void duel_088C6A10(unsigned int p, unsigned short id, int z);
void duel_088C7810(unsigned int p, unsigned int on);
void duel_088328FC(unsigned int p);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
void duel_088C7D74(int a, int b);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
int duel_can_direct_attack(unsigned int p, int z);
int duel_direct_attack_kind(unsigned int p, int z, int arg);
void duel_08832ADC(int a);

int duel_0888F420(DuelAct *a)
{
    duel_0887DB24(a);
    duel_088C7DD4(a->player, 0, 1);
    return 0;
}

#ifdef NON_MATCHING /* li 0; b; nop no segundo teste de 0x80 */
int duel_0888F45C(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel.players[0].n_hand != 0) {
            if (duel.players[1].n_hand == 0) {
                return 0;
            }
            duel_08832880(a->player, a->id, 1);
            return 0x7F;
        }
        return 0;
    case 0x7F:
        a->w[0] = CARD_UID(duel.players[1 - a->player & 1].hand[duel_w_0891968C]);
        duel_08832880(1 - a->player, a->id, 1);
        return 0x7E;
    case 0x7E:
        a->w[1] = CARD_UID(duel.players[a->player & 1].hand[duel_w_0891968C]);
        duel_cmd(0x3F, (1 - a->player & 1 | 0x16) | (a->w[0] & 0x3FF) << 6,
                 (a->player & 1 | 0x16) | (duel.players[a->player & 1].n_hand & 0x3FF) << 6, 0);
        return 0x7D;
    case 0x7D:
        duel_cmd(0x3F, (a->player & 1 | 0x16) | (a->w[1] & 0x3FF) << 6,
                 (1 - a->player & 1 | 0x16) | (duel.players[1 - a->player & 1].n_hand & 0x3FF) << 6, 0);
        return 0;
    }
    return 0;
}
#endif

int duel_0888F6B0(DuelAct *a)
{
    unsigned int t;
    unsigned int q;
    unsigned int z;
    int s;

    if (duel_grave_find_uid(a->player, a->uid) < 0) {
        return 0;
    }
    if (duel_pick_st_zone(a->player) == -1) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        q = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (q != a->player && duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup != 0) {
            s = duel_pick_st_zone(a->player);
            duel_088CB728(a->player, 0xE, a->uid, a->player, s, 1, 0);
            duel_088C8B70(a->player, a->player | (s & 0xFF) << 8, (q & 0xFF) | (z & 0xFF) << 8);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* alocação dos $s (uid reaproveita o registrador da zona) */
int duel_0888F82C(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int z = a->zone;
    int s = duel_pick_st_zone(p);
    int n = duel.players[p & 1].field[z].link;
    int cid;
    int uid;

    if (a->b4_2) {
        return 0;
    }
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[(unsigned char)a->zone].card)) {
        return 0;
    }
    if (a->b3_4 == 0) {
        duel_cmd((a->player ? 0x8000 : 0) | 0x5F, 0, (unsigned char)a->zone, 0);
        return 0;
    }
    switch (n) {
    case 0:
        cid = 0x1497;
        break;
    case 1:
        cid = 0x1498;
        break;
    case 2:
        cid = 0x1499;
        break;
    case 3:
        cid = 0x149A;
        break;
    default:
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, cid) == 0) {
            duel_0884ACD8(a->player, (void *)0xD);
            return 0x78;
        }
        duel_0884ACD8(p, (void *)0xF6);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, cid);
        return 0x7E;
    case 0x7E:
        switch (duel_0884C804()) {
        case 0xB:
            return 0x7D;
        case 0xD:
            return 0x7C;
        }
        return 0;
    case 0x7D:
        uid = CARD_UID(*duel_0884C7CC());
        duel_088CC178(p, z, n + 1);
        duel_088CB728(p, 0xB, uid, p, s, 1, 0);
        duel_cmd((p ? 0x8000 : 0) | 0x5F, n + 1, s, 0);
        if (duel_slot_effect_with(p, 0xB, 0x1379, uid) != 0) {
            duel_088C44E4(p, 0x1379);
            duel_088C4604(a, p, 2000);
        }
        if (n != 3) {
            duel_088C78E0(p);
        }
        return 0;
    case 0x7C:
        duel_088CC178(p, z, n + 1);
        duel_088C6A10(p, cid, s);
        duel_cmd((p ? 0x8000 : 0) | 0x5F, n + 1, s, 0);
        if (n != 3) {
            duel_088C78E0(p);
        }
        return 0;
    case 0x78:
        duel_088C7810(p, 1);
        return 0x77;
    case 0x77:
        duel_088328FC(1 - p);
        return 0x76;
    case 0x76:
        duel_088C7810(p, 0);
        return 100;
    case 100:
        duel_088C78E0(p);
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* ordem zona/jogador do primeiro alvo e $s3/$s4 no caso 0x7F */
int duel_0888FC28(DuelAct *a)
{
    int y;
    unsigned int t;
    unsigned int p;
    unsigned int z;
    int ok;
    int q;

    switch (duel_step) {
    case 0x80:
        if (a->b4_2) {
            return 0;
        }
        if (duel.mode & 1) {
            return 0;
        }
        if (!(duel.mode >> 1 & 1)) {
            return 0;
        }
        ok = 0;
        if (duel_088A9008(a, 1, 0) != 0) {
            t = duel_act_target(a, 1);
            y = (unsigned short)t >> 8 & 0xFF;
            q = t & 0xFF;
            if (duel.players[q & 1].field[y].faceup != 0) {
                ok = 1;
            }
        }
        if (!ok) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel.players[p & 1].field[z].faceup != 0 && duel_act_immune(a, p, z) == 0) {
                duel_cmd((p ? 0x8000 : 0) | 0x1B, z, 1, 0);
                duel_088C7D74(1, 0);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        if (duel_special.zone[1] < 5 &&
            duel_special.act[1 - duel_special.player[0]].uid != CARD_UID(duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].card)) {
            return 0;
        }
        if (duel_088A9008(a, 1, 0) != 0) {
            t = duel_act_target(a, 1);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel.players[p & 1].field[z].faceup != 0 && duel_act_immune(a, p, z) == 0) {
                if (duel.players[p & 1].field[z].pos != 0) {
                    duel_088C97F0(a, p, z, 0, 0);
                }
                return 0x7E;
            }
        }
        return 0;
    case 0x7E:
        t = duel_act_target(a, 1);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_attack_check(p, z, 0) == 0) {
            return 0;
        }
        if (duel_special.zone[1] >= 5) {
            if (duel_can_direct_attack(p, z) == 0 && duel_direct_attack_kind(p, z, 1) == 0) {
                return 0;
            }
            duel_cmd((p ? 0x8000 : 0) | 0x1A, z, 1, 0);
        } else {
            if (duel_can_attack_target(p, z, duel_special.player[1], duel_special.zone[1], 0) == 0) {
                return 0;
            }
            duel_cmd((p ? 0x8000 : 0) | 0x19, z, duel_special.zone[1], duel_special.player[1]);
        }
        return 0x7D;
    case 0x7D:
        duel_08832ADC(0);
        return 0;
    }
    return 0;
}
#endif
