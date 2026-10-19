// @range 0x0880FDF4-0x08810370
// @asm 0x0880FE50-0x0880FEEC
// @asm 0x0880FFAC-0x08810098
// @asm 0x08810098-0x08810120
// @asm 0x08810120-0x088101B0
// @asm 0x088102DC-0x08810370
/* More conditions of the computer player (after the ones in ai_util.c). */
#include "duel.h"
#include "card_db.h"

extern int duel_phase;
extern unsigned int duel_turn_player;

int duel_count_monsters(unsigned int p);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
unsigned int duel_0884E4A0(DuelAct *a);
int duel_088159C8(unsigned int p, unsigned int mask, int (*fn)());
int duel_088171C8();
int duel_field_base_atk(unsigned int p, int z);
int duel_08809F2C();
int duel_0884E690(DuelAct *a, int (*fn)());
int duel_0885F0B0();
unsigned int duel_08807434(unsigned int p, int a);
int duel_hand_count_level(unsigned int p, int lvl);
int duel_08807AB4(unsigned int p);
int duel_0880F8C4(DuelAct *a, int b);
int duel_zone_f_count_monsters(unsigned int p);
int duel_find_effect_card(unsigned int p, int id, int z);

int duel_0880FDF4(DuelAct *a)
{
    if (duel_phase == 3) {
        return duel_count_monsters(1 - a->player) > 0;
    } else {
        return duel_count_monsters_pos(1 - a->player, 1, 0) > 0;
    }
}

#ifdef NON_MATCHING /* registradores/ordem de desvios */
int duel_0880FE50(DuelAct *a)
{

}
#endif

int duel_0880FEEC(DuelAct *a)
{
    if (duel_08809F2C(a->player) < 0) {
        return 0;
    }
    return duel_0884E690(a, duel_0885F0B0) > 1;
}

int duel_0880FF3C(DuelAct *a)
{
    if (duel_count_monsters(1 - a->player) == 0) {
        return 0;
    }
    if ((int)duel_08807434(a->player, 0) < 0) {
        return 0;
    }
    return duel_hand_count_level(a->player, 4) != 0;
}

#ifdef NON_MATCHING /* registradores/ordem de desvios */
int duel_0880FFAC(DuelAct *a)
{
    int k = 0;
    int z;

    for (z = 0; z < 5; z++) {
        unsigned int q = 1 - a->player;
        unsigned int id = duel.players[q & 1].field[z].card.id;

        if (id != 0 && duel.players[q & 1].field[z].faceup != 0 && card_get_kind(id) != k && card_get_kind(id) < 7) {
            if (k != 0) {
                return 1;
            }
            k = card_get_kind(id);
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* registradores/ordem de desvios */
int duel_08810098(DuelAct *a)
{
    if (duel_08807AB4(a->player) >= 2) {
        return 1;
    }
    return duel.players[a->player & 1].lp < 0x3E9 || *(unsigned short *)&a->idx > 0x5DB;
}
#endif

#ifdef NON_MATCHING /* registradores/ordem de desvios */
int duel_08810120(DuelAct *a)
{
    int r;

    if (duel_turn_player == 1 - a->player) {
        if (duel_phase == 0) {
            r = 0;
            if (duel_08809F2C() < 1) {
                r = duel_count_monsters_pos(1 - a->player, 1, 1) != 0;
            }
            return r;
        }
        return 0;
    }
    return 0;
}
#endif

int duel_088101B0(DuelAct *a, int b)
{
    if (duel_0880F8C4(a, b) != 0) {
        int n = duel_count_monsters(1 - a->player);

        return duel_count_monsters(a->player) < n;
    }
    return 0;
}

int duel_08810204(DuelAct *a, int b)
{
    if (duel_0880F8C4(a, b) != 0) {
        return duel_zone_f_count_monsters(1 - a->player);
    }
    return 0;
}

int duel_08810244(DuelAct *a)
{
    if (duel_find_effect_card(a->player, 0x1539, -1) != 0) {
        unsigned int p = a->player;

        if (duel_turn_player != p && duel_phase == 5) {
            return 1;
        }
        if (duel_turn_player == p && (unsigned int)duel_phase < 2) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores/ordem de desvios */
int duel_088102DC(DuelAct *a)
{
    int z;

    if (duel_phase == 5) {
        unsigned int q = 1 - a->player;

        for (z = 5; z <= 10; z++) {
            if (duel.players[q & 1].field[z].card.id != 0 && (duel.players[q & 1].field[z].flags >> 20 & 1)) {
                return 1;
            }
        }
    }
    return 0;
}
#endif
