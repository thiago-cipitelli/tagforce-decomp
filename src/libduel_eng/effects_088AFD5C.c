// @range 0x088AFD5C-0x088B08B8
// @asm 0x088AFEE0-0x088B0504
/* Pending duel events (duel_chain.kinds / srcs) and their network echo. */
#include "duel.h"

extern int duel_w_089195EC;
extern unsigned int duel_turn_player;
extern int duel_player_type[2];
extern int duel_w_08914918;
extern int duel_w_0891491C;
extern unsigned int duel_w_08914920;
extern int duel_w_08914924;
extern int duel_w_089149CC;
extern unsigned int duel_w_089149D0;
extern int duel_w_089149D4;
extern int duel_w_089149D8;

void duel_msg_send(unsigned short cmd, int arg, void *data, int len);

void duel_088AFD5C(unsigned short kind, unsigned int ev)
{
    short m[3];
    unsigned int n;

    if (duel_w_089195EC != 0 && duel_player_type[duel_turn_player] == 2) {
        m[0] = kind;
        m[1] = ev;
        m[2] = ev >> 16;
        duel_msg_send(9, -1, m, 6);
        return;
    }
    if (duel_w_08914924 != 0) {
        return;
    }
    if (duel_w_08914918 != 0) {
        n = duel_chain.n_kinds + 1;
        if (n > 0x20) {
            n = 0x20;
        }
        duel_chain.n_kinds = n;
        duel_chain.kinds[n - 1] = kind;
        ((unsigned int *)&duel_chain.srcs[-1])[duel_chain.n_kinds] = ev;
    } else {
        duel_chain.n_kinds = 1;
        duel_w_0891491C++;
        duel_chain.kinds[0] = kind;
        ((unsigned int *)duel_chain.srcs)[0] = ev;
    }
    duel_w_089149D0 = duel_turn_player;
    duel_w_089149CC = 0;
    duel_w_089149D4 = 1;
    duel_w_089149D8 = 0;
    switch (kind) {
    case 3:
    case 4:
    case 0x17:
        duel_w_089149D0 = 1 - duel_turn_player;
    }
}

#ifdef NON_MATCHING /* o mwccpsp desenrola o laço quando o limite fica em registrador; o original não */
void duel_088AFEE0(int b)
{
    unsigned short m[97];
    unsigned int i;
    int n;
    unsigned short *q;

    if (duel_w_089195EC != 0) {
        n = 0;
        m[n++] = duel_w_08914920;
        q = &m[n];
        for (i = 0; i < duel_w_08914920; i++) {
            q[0] = duel_chain.kinds[i];
            q[1] = ((unsigned int *)duel_chain.srcs)[i];
            q[2] = ((unsigned int *)duel_chain.srcs)[i] >> 16;
            n += 3;
            q += 3;
        }
        duel_msg_send(b != 0 ? 0xC : 0xB, -1, m, n << 1);
    }
}
#endif


int card_is_monster(int id);
int card_is_in_group_1a60(int id);

#ifdef NON_MATCHING /* switch de um caso só não gera o bloco morto do original; agendamento do move a0 */
int duel_088AFF8C(unsigned int p, int id)
{
    unsigned int i;

    for (i = 0; i < duel_w_08914920; i++) {
        switch (duel_chain.kinds[i]) {
        case 1:
            return duel_player_type[p] == 1;
        case 2:
            switch (id) {
            case 0x131E:
            case 0x15F0:
            case 0x15AD:
                return 1;
            }
            break;
        case 3:
            switch (id) {
            case 0x1358:
            case 0x1472:
            case 0x17FC:
            case 0x1886:
                return 1;
            }
            break;
        case 4:
            switch (id) {
            case 0x1472:
                return 1;
            default:
                break;
            }
            break;
        case 6:
            switch (id) {
            case 0x124F:
            case 0x1250:
            case 0x12CD:
            case 0x12E4:
            case 0x13FA:
            case 0x1518:
            case 0x1572:
            case 0x15C6:
            case 0x15F3:
            case 0x15F8:
            case 0x16E0:
                return 1;
            }
            if (card_is_monster(id) && card_is_in_group_1a60(id) == 0) {
                return 1;
            }
            break;
        case 7:
            switch (id) {
            case 0x124F:
            case 0x1250:
            case 0x12CD:
            case 0x12E4:
            case 0x13FA:
            case 0x1518:
            case 0x1572:
            case 0x15C6:
            case 0x15F8:
            case 0x16E0:
                return 1;
            }
            if (card_is_monster(id)) {
                return 1;
            }
            break;
        case 8:
            switch (id) {
            case 0x12CD:
            case 0x13FA:
            case 0x140F:
            case 0x1518:
            case 0x15F8:
            case 0x16E0:
            case 0x18D8:
            case 0x18DC:
            case 0x192E:
            case 0x195D:
            case 0x198E:
            case 0x1AA1:
                return 1;
            }
            if (card_is_monster(id)) {
                return 1;
            }
            break;
        case 0xB:
            switch (id) {
            case 0x12D7:
                return 1;
            default:
                break;
            }
            break;
        case 9:
            switch (id) {
            case 0x140F:
            case 0x192E:
                return 1;
            }
            break;
        case 0x10:
            switch (id) {
            case 0x1352:
            case 0x135A:
                return 1;
            }
            break;
        case 0x12:
        case 0x13:
            return 1;
        case 0x18:
            switch (id) {
            case 0x134E:
                return 1;
            default:
                break;
            }
            break;
        case 0x19:
            switch (id) {
            case 0x1353:
            case 0x151C:
                return 1;
            }
            break;
        case 0x1B:
            switch (id) {
            case 0x134C:
            case 0x134D:
            case 0x135B:
            case 0x134B:
            case 0x15B5:
            case 0x195E:
            case 0x1A16:
            case 0x1A6B:
            case 0x1AA9:
                return 1;
            }
            break;
        case 0x1D:
            switch (id) {
            case 0x1354:
                return 1;
            default:
                break;
            }
            break;
        case 5:
            if (duel_player_type[p] == 1 || id == 0x1AA8) {
                return 1;
            }
            return p != duel_turn_player;
        case 0x17:
            if (p != duel_turn_player && id == 0x12F3) {
                return 1;
            }
            break;
        case 0x1F:
        case 0x20:
        case 0x21:
            return 1;
        case 0xD:
        case 0xE:
        case 0xF:
        case 0x14:
        case 0x15:
        case 0x16:
            return 1;
        }
    }
    return 0;
}
#endif

int field_get_card(unsigned int p, int z);
int duel_088695B8(unsigned int p, unsigned int q, int z, DuelAct *a);
int duel_08869728(unsigned int p, int i, DuelAct *a);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_088AFF8C(unsigned int p, int id);

int duel_088B0504(DuelAct *a, int id)
{
    switch (a->id) {
    case 0x17BC:
        return 1;
    }
    switch (id) {
    case 0x14E6:
    case 0x15A7:
    case 0x15AB:
    case 0x183E:
    case 0x198F:
    case 0x19B7:
        return 1;
    }
    return 0;
}

int duel_088B0570(unsigned int p, int b)
{
    int q;
    int z;
    int id;

    for (q = 0; q < 2; q++) {
        for (z = 0; z <= 0xA; z++) {
            id = field_get_card(q, z);
            if (id != 0 && duel_088695B8(p, q, z, 0) != 0) {
                if (b == 0 || duel_088AFF8C(p, id) != 0) {
                    return 1;
                }
            }
        }
    }
    for (z = 0; z < duel.players[p & 1].n_hand; z++) {
        id = duel.players[p & 1].hand[z].id;
        if (duel_08869728(p, z, 0) != 0) {
            if (b == 0 || duel_088AFF8C(p, id) != 0) {
                return 1;
            }
        }
    }
    if (duel_088BFFD8(p, 0xFFFE, 0xE) != 0) {
        return 1;
    }
    return 0;
}

int duel_088B06F0(DuelAct *a, unsigned int p, int b)
{
    int q;
    int z;
    int f;
    int id;

    f = 0;
    switch (a->b3_4) {
    case 0:
    case 3:
        if (a->player == p) {
            f = 1;
        }
    }
    for (q = 0; q < 2; q++) {
        for (z = 0; z <= 0xA; z++) {
            id = field_get_card(q, z);
            if (id != 0 && duel_088695B8(p, q, z, a) != 0) {
                if (b == 0 || !f || duel_088B0504(a, id) != 0) {
                    return 1;
                }
            }
        }
    }
    for (z = 0; z < duel.players[p & 1].n_hand; z++) {
        id = duel.players[p & 1].hand[z].id;
        if (duel_08869728(p, z, a) != 0) {
            if (b == 0 || !f || duel_088B0504(a, id) != 0) {
                return 1;
            }
        }
    }
    return 0;
}
