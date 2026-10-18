// @range 0x088F7EB4-0x088F85D4
// @asm 0x088F84A4-0x088F8564
// @data 0x08911FC0-0x0891205C
/* Message state queries, the class of each message command, and the first card effect handlers:
   each handler gets the activated card and returns the next step of the chain (0x7A-0x80) or 0. */
#include "duel.h"

typedef struct DuelMsgState {          /* 0x08914340 */
    unsigned int lo : 15;
    int pending : 4;
    unsigned int hi : 13;
} DuelMsgState;

extern DuelMsgState duel_msg_state;
extern int duel_player_type[2];        /* 0x089139F8 2: remote */
extern int duel_step;                  /* 0x089148F8 */

void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088CA384(unsigned int p, int id, int a);
void duel_08832894(unsigned int p, unsigned short id);
void duel_08832950(unsigned int p, unsigned short id);
int duel_0886CE38(void);
void duel_088998D8(DuelAct *a, int b);
void duel_08832624(unsigned int p, int a, int b, int c);
void duel_088824E4(void);
void duel_088C5E70(unsigned int p, int a);
void duel_088C78E0(unsigned int p);
unsigned int duel_act_target2(DuelAct *a, int i);
int duel_grave_find_uid(unsigned int p, unsigned int uid);
void duel_088CA428(unsigned int p, unsigned int uid, int a);
int duel_0887DB24(void);
void duel_08883FBC(void);
void duel_088C7DD4(unsigned int p, int a, int b);
void duel_088C625C(unsigned int p, int a, int b);

int duel_msg_done(void)
{
    return duel.msg_done;
}

int duel_msg_busy(void)
{
    int n = duel.unk3790 != 0 && duel_player_type[duel.turn_player] == 2;

    return duel_msg_state.pending + n > 0;
}

/* 0: no answer, 1: answered by the presentation, 2: answered by the player. */
int duel_msg_class(int cmd)
{
    switch (cmd) {
    case 1:
        return 0;
    case 2:
        return 0;
    case 3:
        return 1;
    case 6:
        return 0;
    case 7:
        return 0;
    case 8:
        return 1;
    case 9:
        return 0;
    case 10:
        return 0;
    case 11:
        return 0;
    case 12:
        return 2;
    case 13:
        return 2;
    case 14:
        return 2;
    case 15:
        return 2;
    case 16:
        return 2;
    case 17:
        return 2;
    case 18:
        return 0;
    case 19:
        return 0;
    case 20:
        return 0;
    case 21:
        return 2;
    case 22:
        return 2;
    case 23:
        return 2;
    case 24:
        return 2;
    case 25:
        return 2;
    case 26:
        return 2;
    case 27:
        return 2;
    case 28:
        return 2;
    case 29:
        return 2;
    case 30:
        return 2;
    case 31:
        return 2;
    default:
        return 0;
    }
}

int duel_eff_f8030(DuelAct *a)
{
    duel_08832624(a->player, 1, 0, 0);
    return 0;
}

int duel_eff_f8060(DuelAct *a, int b)
{
    unsigned int q;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38() == 0) {
            return 0;
        }
        duel_088998D8(a, b);
        if (a->player != a->b15_3 && a->player == a->b15_1 && duel.players[(1 - a->player) & 1].n_hand >= 2) {
            return 0x7F;
        }
        return 0;
    case 0x7F:
        duel_088C7BA4();
        duel_08832950(1 - a->player, a->id);
        return 0x7E;
    case 0x7C:
    case 0x7E:
        q = 1 - a->player;
        if (duel.players[q & 1].n_hand != 0) {
            duel_08832894(q, a->id);
        }
        return duel_step - 1;
    case 0x7B:
    case 0x7D:
        q = 1 - a->player;
        if (duel.players[q & 1].n_hand != 0) {
            duel_088CA384(q, duel.unk3830, 0);
        }
        return duel_step - 1;
    case 0x7A:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_eff_f8234(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38() == 0) {
            return 0;
        }
        duel_088998D8(a, b);
        return 0x7F;
    case 0x7F:
        duel_088824E4();
        return 0;
    }
    return 0;
}

int duel_eff_f82b8(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        for (i = 0; i < a->n7; i++) {
            if (duel_grave_find_uid(a->player, duel_act_target2(a, i)) < 0) {
                return 0;
            }
        }
        return 0x7F;
    case 0x7F:
        if (a->n7 != 0) {
            int j = duel_grave_find_uid(a->player, duel_act_target2(a, (a->n7 & 0xFF) - 1));

            if (j >= 0) {
                duel_088CA428(a->player, CARD_UID(duel.players[a->player & 1].grave[j]), 1);
            }
            a->n7--;
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C78E0(a->player);
        return 0x7D;
    case 0x7D:
        duel_088C5E70(a->player, 2);
        return 0;
    }
    return 0;
}

int duel_eff_f8438(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0887DB24()) {
            return 0x7F;
        }
        break;
    case 0x7F:
        duel_08832624(a->player, 1, 0, 0);
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* same code; the original loads the player into a2 instead of a0 */
int duel_eff_f84a4(DuelAct *a)
{
    if (!a->b4_2) {
        if (duel.players[a->player & 1].flags >> 16 & 1) {
            return 0;
        }
        duel_08883FBC();
    }
    duel_088C7DD4(a->player, 0x15, 1);
    duel_088C7DD4(a->player, 0x16, 1);
    return 0;
}
#endif

int duel_eff_f8564(DuelAct *a)
{
    unsigned int i;

    duel_088C7BA4();
    for (i = 0; (int)i < 2; i++) {
        unsigned int q = duel.turn_player ^ i;

        duel_088C625C(q, 1, q != a->player);
    }
    duel_088C7CF0();
    return 0;
}
