// @range 0x08810370-0x08811BDC
// @asm 0x08810944-0x08810AE0
// @asm 0x08810D08-0x08810EFC
// @asm 0x088119FC-0x08811B24
/* Conditions of the computer player: LP against the strongest ATK, monsters on the field, the chain. */
#include "duel.h"
int card_attribute_to_group(unsigned short id);
int card_get_special_group(unsigned short id);

extern int duel_phase;
extern unsigned int duel_turn_player;

int duel_08809FC4();
int duel_0880F8C4(DuelAct *a, int b);
int duel_is_cur_card(int id);
unsigned int duel_0884E4A0(DuelAct *a);
int duel_088159C8(unsigned int p, unsigned int mask, int (*fn)());
int duel_0881743C();
int duel_count_monsters(unsigned int p);
int duel_08807AB4(unsigned int p);
int duel_count_same_name_other(unsigned int p, int z);
int duel_act_can_target();
int duel_field_atk(unsigned int p, int z);
int duel_d_count_name(unsigned int p, unsigned short id);
int duel_08810AE0(DuelAct *a);
int duel_08809F2C();
int duel_field_def(unsigned int p, int z);
int duel_08845070(unsigned short id);
int duel_grave_count_monsters(unsigned int p);
int duel_field_level(unsigned int p, int z);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_count_name(unsigned int p, int id);
int duel_monster_can_act(unsigned int p, int z);
int duel_08807474(unsigned int p, int b, unsigned int mask);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_0880B07C(unsigned int p);
int duel_0880ABE0();
int duel_slot_has_effect(unsigned int p, int z, int id);
int card_get_atk(unsigned short id);
int duel_hand_count_monsters(unsigned int p);
void duel_act_0887E140(DuelAct *a, int d[2]);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_count_monsters_state(unsigned int p, int pos, int state);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int card_is_in_group_1a2f(unsigned short id);

int duel_08810370(DuelAct *a)
{
    int atk;
    int def;
    unsigned int p;

    duel_08809FC4(1 - a->player, &atk, &def, 1);
    if (atk > -1) {
        p = a->player;
        if (atk < duel.players[p & 1].lp) {
            if (atk >= duel.players[(1 - p) & 1].lp) {
                return 1;
            }
            if (atk >= 1500 && duel.players[(1 - p) & 1].lp - atk < duel.players[p & 1].lp) {
                return 1;
            }
        }
    }
    duel_08809FC4(a->player, &atk, &def, 1);
    if (atk > -1) {
        p = a->player;
        if (atk < duel.players[p & 1].lp && atk >= duel.players[(1 - p) & 1].lp) {
            return 1;
        }
    }
    return 0;
}

int duel_088104D0(DuelAct *a, int b)
{
    if (duel_0880F8C4(a, b) == 0) {
        return 0;
    }
    if (duel_is_cur_card(0x169F) != 0) {
        return 0;
    }
    return duel_088159C8(a->player, duel_0884E4A0(a), duel_0881743C) >= 0;
}

int duel_08810544(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].lp <= 1000) {
        return 0;
    }
    if (duel.players[(1 - p) & 1].lp <= 1000) {
        return 1;
    }
    return duel_count_monsters(p) > 2;
}

int duel_088105D0(DuelAct *a)
{
    unsigned int q = 1 - a->player;

    if (duel_count_monsters(q) * 500 >= duel.players[q & 1].lp) {
        return 1;
    }
    return duel_count_monsters(q) > 1;
}

int duel_0881065C(DuelAct *a)
{
    if (duel_count_monsters(1 - a->player) + duel_count_monsters(a->player) > 2) {
        return duel_08807AB4(a->player);
    }
    return 0;
}

int duel_088106C4(void)
{
    return !((unsigned int)duel_phase < 2);
}

int duel_088106D8(DuelAct *a)
{
    int i;
    unsigned int p;
    int z;
    int n = 0;
    int id;

    for (i = 0; i < 2; i++) {
        for (z = 0; z < 5; z++) {
            id = duel.players[i & 1].field[z].card.id;
            if (id != 0 && card_attribute_to_group(id) != 0) {
                n++;
            }
        }
    }
    if (duel.players[(1 - a->player) & 1].lp <= n * 500) {
        return 1;
    }
    n = 0;
    for (z = 0; z < 5; z++) {
        p = a->player;
        if (duel.players[(1 - p) & 1].field[z].card.id != 0 && duel.players[(1 - p) & 1].field[z].faceup == 0) {
            n++;
        }
        id = duel.players[p & 1].field[z].card.id;
        if (id != 0 && duel.players[p & 1].field[z].faceup == 0 && card_get_special_group(id) != 0) {
            return 0;
        }
    }
    return n > 0;
}

int duel_0881089C(DuelAct *a)
{
    int z;

    for (z = 0; z < 5; z++) {
        unsigned int p = a->player;

        if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0
            && duel_count_same_name_other(p, z) > 0) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* a zona do índice passa por um byte; registradores */
int duel_08810944(DuelAct *a)
{
    unsigned int k;
    struct DuelActX *s;

    for (k = 0; k < duel_chain.n_kinds; k++) {
        switch (duel_chain.kinds[k]) {
        case 6:
        case 7:
        case 8:
            s = &duel_chain.srcs[k];
            if (a->player != s->s_player
                && duel.players[s->s_player & 1].field[s->s_zone].faceup != 0
                && duel_act_can_target(a) != 0
                && duel_field_atk(s->s_player, s->s_zone) <= 2000
                && duel_d_count_name(s->s_player, duel.players[s->s_player].field[s->s_zone].card.id) != 0) {
                return 1;
            }
            break;
        }
    }
    return 0;
}
#endif

int duel_08810AE0(DuelAct *a)
{
    unsigned int k;

    for (k = 0; k < duel_chain.n_kinds; k++) {
        switch (duel_chain.kinds[k]) {
        case 6:
        case 7:
        case 8:
        case 0x1F:
        case 0x20:
        case 0x21:
            if (a->player == (duel_chain.srcs[k].s_player ^ duel_chain.srcs[k].s_b16)) {
                return 0;
            }
            break;
        }
    }
    return 1;
}

int duel_08810BA0(DuelAct *a)
{
    if (duel_08810AE0(a) != 0) {
        return duel_08809F2C(a->player) < 0;
    }
    return 0;
}

int duel_08810BDC(DuelAct *a)
{
    int z;
    int m = 0;

    for (z = 0; z < 5; z++) {
        m = MAX(m, duel_field_atk(a->player, z));
        m = MAX(m, duel_field_def(a->player, z));
    }
    for (z = 0; z < 5; z++) {
        unsigned int q = 1 - a->player;

        if (duel.players[q & 1].field[z].faceup != 0 && duel_field_atk(q, z) > m) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* ordem dos testes do switch e registradores */
int duel_08810D08(DuelAct *a, int b)
{
    unsigned int i;
    unsigned int n;
    unsigned int id;
    int r;

    if (b != 0) {
        return 0;
    }
    switch (a->kind) {
    case 0x12:
    case 0x13:
        if (duel_count_monsters(a->player) == 0) {
            return 1;
        }
        if (duel_count_monsters(a->player) >= 3) {
            n = duel.players[a->player & 1].n_hand;
            for (i = 0; i < n; i++) {
                id = duel.players[a->player & 1].hand[i].id;
                switch (id) {
                case 4998:
                case 4999:
                case 5000:
                case 0x1905:
                case 0x13B5:
                    return 1;
                case 0x154B:
                    return 1;
                }
            }
        }
        break;
    case 0x17:
        for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
            r = !card_get_special_group(duel.players[a->player & 1].hand[i].id);
            if (!r) {
                return 1;
            }
        }
        break;
    }
    return 0;
}
#endif

int duel_08810EFC(DuelAct *a)
{
    return !duel_count_monsters(a->player);
}

int duel_08810F24(DuelAct *a)
{
    int z;

    if (duel_turn_player == a->player) {
        return 0;
    }
    for (z = 0; z <= 10; z++) {
        unsigned int id = duel.players[a->player & 1].field[z].card.id;

        if (id != 0 && !(duel.players[a->player & 1].field[z].flags >> 5 & 1) && duel_08845070(id) > 0) {
            return 1;
        }
    }
    return 0;
}

int duel_08810FEC(DuelAct *a)
{
    int z;
    int l;
    int n = duel_grave_count_monsters(a->player);

    if (n != 0) {
        for (z = 0; z < 5; z++) {
            unsigned int q = 1 - a->player;
            unsigned int id = duel.players[q & 1].field[z].card.id;

            if (duel.players[q & 1].field[z].faceup != 0 && id != 0
                && (l = duel_field_level(q, z)) != 0 && l <= n) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_088110C4(DuelAct *a)
{
    return duel_turn_player == a->player;
}

int duel_088110E0(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int q = 1 - p;
    int n = duel.players[q & 1].n_grave;

    if ((unsigned int)duel.players[p & 1].n_d <= 4 && (unsigned int)duel.players[p & 1].n_d < (unsigned int)duel.players[q & 1].n_d) {
        return 1;
    }
    if (n >= (unsigned int)duel.players[q & 1].n_d) {
        return 0;
    }
    if (n >= duel.players[p & 1].n_grave) {
        return 0;
    }
    if (n != 0) {
        if (duel_hand_count_name(p, 0x12F9) != 0 || duel_count_name(a->player, 0x12F9) != 0) {
            return 0;
        }
    }
    return 1;
}

int duel_088111F8(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int n = duel.players[(1 - p) & 1].n_grave;

    if (n >= (unsigned int)duel.players[p & 1].n_d) {
        return 0;
    }
    return (int)n >= 3;
}

int duel_08811264(DuelAct *a)
{
    int z;

    for (z = 0; z <= 4; z++) {
        if (card_get_special_group(duel.players[a->player & 1].field[z].card.id) != 0
            && duel.players[a->player & 1].field[z].faceup != 0) {
            unsigned int p = a->player;

            if (duel_turn_player == p && (unsigned int)duel_phase < 3 && duel_monster_can_act(p, z) != 0) {
                return 1;
            }
            if (duel_field_atk(a->player, z) <= 1000) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_08811380(DuelAct *a)
{
    int r = duel_08807474(a->player, 0, duel_0884E4A0(a));

    if (r < 0) {
        return 0;
    }
    a->n7 = 1;
    a->w[0] = duel_field_atk(r & 0xFF, ((unsigned short)r >> 8) & 0xFF);
    return duel_0884E948(a, 1 - a->player) > 0;
}

int duel_088113FC(DuelAct *a)
{
    if (duel_0880B07C(a->player) == 0) {
        return 0;
    }
    if (duel_0880ABE0(a, 1 - a->player, 1) != 0) {
        return 1;
    }
    return duel_08809F2C(a->player) < 0;
}

int duel_08811468(DuelAct *a)
{
    if (duel_0880B07C(a->player) == 0) {
        return 0;
    }
    if (duel_0880ABE0(a, 1 - a->player, 0) != 0) {
        return 1;
    }
    return duel_08809F2C(a->player) < 0;
}

int duel_088114D8(DuelAct *a)
{
    if (duel_slot_has_effect(1 - a->player, 0xB, a->id) != 0) {
        return 0;
    }
    return duel_08809F2C(a->player) < 0;
}

int duel_08811530(DuelAct *a)
{
    return card_get_atk(duel.list[a->src.uid].id) >= 1200;
}

int duel_08811570(DuelAct *a)
{
    if (duel_0880B07C(a->player) == 0) {
        return 0;
    }
    if (duel_0880ABE0(a, 1 - a->player, 0) != 0) {
        return 0;
    }
    return duel_count_monsters(1 - a->player) < 2;
}

int duel_088115E8(DuelAct *a)
{
    return duel.players[a->player & 1].n_hand <= duel_hand_count_monsters(a->player) * 2;
}

int duel_08811648(DuelAct *a)
{
    if (duel_08809F2C(a->player) < 0) {
        return 0;
    }
    return duel_count_monsters(a->player) > 1;
}

int duel_08811694(DuelAct *a)
{
    int d[2];
    unsigned int q;

    duel_act_0887E140(a, d);
    q = 1 - a->player;
    if (d[q] <= -1000) {
        return 1;
    }
    return duel.players[q & 1].lp + d[q] <= 0;
}

int duel_08811714(DuelAct *a)
{
    int n;

    if (duel_0880B07C(a->player) == 0) {
        return 0;
    }
    n = duel_0884E948(a, 1 - a->player);
    return duel_0884E948(a, a->player) < n;
}

int duel_08811784(DuelAct *a)
{
    if (duel_turn_player != a->player) {
        return 0;
    }
    if ((unsigned int)duel_phase > 1) {
        return 0;
    }
    return duel_08809F2C(a->player) < 0;
}

int duel_088117DC(DuelAct *a)
{
    return duel_0884E948(a, 1 - a->player) > 1;
}

int duel_0881180C(DuelAct *a)
{
    int z;

    for (z = 0; z < 5; z++) {
        unsigned short id = duel.players[a->player & 1].field[z].card.id;

        if (duel.players[a->player & 1].field[z].faceup != 0 && card_get_special_group(id) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_088118A8(DuelAct *a)
{
    int z;

    for (z = 0; z < 5; z++) {
        if (duel_0884E3E4(a, a->player, z) != 0
            && duel_088BFFD8(a->player, a->id, duel.players[a->player & 1].field[z].card.id) != 0) {
            return 1;
        }
    }
    return 0;
}

int duel_08811958(DuelAct *a)
{
    if (duel_turn_player == a->player) {
        return 0;
    } else if ((unsigned int)(a->kind - 0x12) < 2) {
        return duel_special.unk8;
    } else {
        return duel_phase == 5;
    }
}

int duel_088119B0(DuelAct *a)
{
    if (a->kind != 3) {
        return 0;
    }
    return duel_count_monsters_state(1 - a->player, 1, 0) > 0;
}

#ifdef NON_MATCHING /* registradores */
int duel_088119FC(DuelAct *a)
{
    int m = -1;
    unsigned int best;
    int p;
    int z;
    int v;
    unsigned int q;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel_0884E440(a, p, z) != 0) {
                v = duel_field_atk(p, z);
                if (m < 0 || v < m) {
                    best = p;
                    m = v;
                }
            }
        }
    }
    q = a->player;
    if (m / 2 < duel.players[q & 1].lp) {
        q = 1 - q;
        if (m / 2 < duel.players[q & 1].lp) {
            return best == q;
        }
        return 1;
    }
    return 0;
}
#endif

int duel_08811B24(DuelAct *a)
{
    if (a->kind == 0x16) {
        return card_get_atk(duel.list[a->src.uid].id) >= 1500;
    }
    return 0;
}

int duel_08811B7C(DuelAct *a)
{
    unsigned int id = duel.list[a->src.uid].id;

    if (card_is_in_group_1a2f(id) != 0) {
        return 1;
    }
    switch (id) {
    case 0x19BC:
        return 1;
    }
    return 0;
}
