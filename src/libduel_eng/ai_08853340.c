// @range 0x08853340-0x08853CE8
/* Computer player: moving the queued actions (duel_chain.pad0) to the chain and the main step. */
#include "duel.h"

extern unsigned int duel_w_089147E0;   /* number of queued actions */
extern int duel_w_089148E8;
extern int duel_w_089148EC;
extern int duel_w_089148F0;
extern int duel_w_089149D4;
extern unsigned char duel_ai_step2;    /* 0x08914A2D */

void duel_memcpy(void *dst, void *src, int n);
int duel_card_e3070(int id);
int duel_08850164(void);
int duel_08850F44(void);
int duel_088C3F80(void);
int duel_088B0C88(void);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active);
int card_get_type(int id);
int card_is_monster(int id);

int duel_08853340(void)
{
    unsigned int i;
    unsigned int j;

    duel_chain.unk480 = 0;
    for (i = 0; i < duel_w_089147E0; i++) {
        duel_memcpy(&duel_chain.acts[i], &duel_chain.pending[i], 0x18);
        if (++duel_chain.unk480 >= 16) {
            break;
        }
    }
    if (i < duel_w_089147E0) {
        for (j = 0; j < duel_w_089147E0 - i; j++) {
            duel_memcpy(&((DuelAct *)duel_chain.pad0)[j], &((DuelAct *)duel_chain.pad0)[i + j], 0x18);
        }
    }
    duel_w_089147E0 -= i;
    duel_w_089148E8 = duel_chain.unk480 > 0;
    duel_w_089148EC = 0;
    return 1;
}

int duel_08853470(void)
{
    unsigned int i;

    duel_chain.unk480 = 0;
    for (i = 0; i < duel_w_089147E0; i++) {
        if (!duel_card_e3070(((DuelAct *)duel_chain.pad0)[i].id)) {
            continue;
        }
        duel_memcpy(&duel_chain.acts[duel_chain.unk480++], &((DuelAct *)duel_chain.pad0)[i], 0x18);
        for (; i < duel_w_089147E0 - 1; i++) {
            duel_memcpy(&((DuelAct *)duel_chain.pad0)[i], &((DuelAct *)duel_chain.pad0)[i + 1], 0x18);
        }
        duel_w_089147E0--;
        break;
    }
    duel_w_089148E8 = duel_chain.unk480 > 0;
    duel_w_089148EC = 0;
    return 1;
}

int duel_088535C0(void)
{
    if (duel_w_089148E8 != 0) {
        return duel_08850164();
    }
    if (duel_w_089148F0 != 0) {
        return duel_08850F44();
    }
    if (duel_088C3F80() != 0) {
        return 1;
    }
    if (duel_w_089147E0 != 0) {
        return duel_08853340();
    }
    if (duel_w_089149D4 != 0) {
        return duel_088B0C88();
    }
    return 0;
}

int duel_08853660(void)
{
    switch (duel_ai_step2) {
    case 0:
        if (duel_w_089149D4 != 0) {
            return duel_088B0C88();
        }
        duel_ai_step2++;
    case 1:
        if (duel_w_089148F0 != 0) {
            return duel_08850F44();
        }
        duel_ai_step2++;
    case 2:
        if (duel_w_089148E8 == 0 && duel_w_089147E0 != 0) {
            duel_08853470();
        }
        duel_ai_step2++;
    case 3:
        if (duel_w_089148E8 != 0) {
            return duel_08850164();
        }
        duel_ai_step2++;
    case 4:
        if (duel_w_089148F0 != 0) {
            return duel_08850F44();
        }
        duel_ai_step2++;
    }
    return 0;
}

int duel_088537B0(DuelAct *a, unsigned int p, int z)
{
    if (a->w[p] & 1 << z) {
        return 0;
    }
    return duel_0884E440(a, p, z);
}

int duel_088537F4(DuelAct *a, unsigned int p, int z)
{
    int id = duel.players[p & 1].field[z].card.id;

    if (id == 0) {
        return 0;
    }
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    if (z == 10) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_can_affect(a, p, z, 1) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].flags >> 8 & 1) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        return card_get_type(id) == 0x17;
    }
    return !card_is_monster(id);
}

int duel_0885395C(DuelAct *a, unsigned int p, int z)
{
    if (a->player == p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_08853A28(DuelAct *a, unsigned int p, int z)
{
    if (a->player == p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_act_immune(a, p, z) != 0) {
        return 0;
    }
    return duel_act_can_affect(a, p, z, 1) != 0;
}

int duel_08853B10(DuelAct *a, unsigned int p, int z)
{
    if (a->player == p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    return duel_act_can_target(a, p, z) != 0;
}

int duel_08853B98(DuelAct *a, unsigned int p, int z)
{
    if (a->player == p) {
        return 0;
    }
    if (z >= 5) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id == 0) {
        return 0;
    }
    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (duel_slot_ef098(p, z, a->id) == 0) {
        return 0;
    }
    return !duel_act_immune(a, p, z);
}

int duel_08853C80(DuelAct *a, unsigned int p, int z)
{
    unsigned short s = duel_slot_src_attr8(p, z, 0);
    int r = 1;

    if (s != 0xFFFF && s != (unsigned short)(a->player | a->zone << 8)) {
        r = 0;
    }
    return r;
}
