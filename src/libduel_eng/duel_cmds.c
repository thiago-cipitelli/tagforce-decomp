// @range 0x088C77D4-0x088C7F18
// @asm 0x088C7954-0x088C7B90
/* Commands to the presentation (duel_cmd: command | 0x8000 for player 1, two halfwords) and the
   nesting counter of the effect being shown. */
#include "duel.h"

extern unsigned int duel_turn_player;
extern int duel_player_type[2];
extern int duel_w_089195EC;
extern unsigned int duel_089195C8;
extern int duel_w_08914918;            /* nesting depth of the effect being shown */
extern int duel_w_0891491C;
extern int duel_w_08914924;            /* nonzero: no commands */
extern int duel_w_089149D4;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, int c);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_hand_revealed(unsigned int p);
unsigned int duel_hand_locked(unsigned int p);
void duel_mode_set(unsigned int bit, int on);
void duel_player_flag_set(unsigned int p, unsigned int bit, int on);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
void duel_088DF554(unsigned int p, int z, unsigned int bit, int on);

void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b)
{
    duel_088C87A8(p, 11, id, a, b);
}

void duel_088C77EC(unsigned int p, int a)
{
    duel_cmd((p ? 0x8000 : 0) | 0x51, a, 0, 0);
}

void duel_088C7810(unsigned int p, unsigned int on)
{
    if (on == 0) {
        on = duel_hand_revealed(p);
    }
    if (on != duel_hand_locked(p)) {
        duel_cmd((p ? 0x8000 : 0) | 0x52, on, 0, 0);
    }
}

void duel_088C7880(unsigned int p)
{
    if (duel.players[p & 1].n_d != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x58, 0xD, 0, 0);
    }
}

void duel_088C78E0(unsigned int p)
{
    unsigned int v = duel_089195C8;

    duel_cmd((p ? 0x8000 : 0) | 0x54, v, v >> 16 & 0xFFFF, 0);
}

void duel_088C7910(unsigned int p)
{
    unsigned int v = duel_089195C8;

    duel_cmd((p ? 0x8000 : 0) | 0x4E, v, v >> 16 & 0xFFFF, 0);
}

void duel_088C7940(int a)
{
    duel_cmd(0x59, a, 0, 0);
}

int card_get_attribute_icon(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);

#ifdef NON_MATCHING /* 15 palavras: no segundo laço o original troca $s4/$s5 (q e o ponteiro do jogador) e faz
                       `or v0,v0,s1` no primeiro argumento do 0x3F */
/* command 0x3F for every attribute-3 card in both graveyards and every 0x12A1 in zone D that still has its
   effect, then 0x5B and the hand refresh of both players */
void duel_088C7954(void)
{
    unsigned int j;
    int i;

    for (i = 0; i < 2; i++) {
        unsigned int p = (duel_turn_player ^ i) & 1;

        for (j = 0; j < duel.players[p].n_grave; j++) {
            if (card_get_attribute_icon(duel.players[p].grave[j].id) == 3) {
                duel_cmd(0x3F, (CARD_UID(duel.players[p].grave[j]) & 0x3FF) << 6 | (p | 0x1C), p | 0x58, 0);
            }
        }
    }
    for (i = 0; i < 2; i++) {
        unsigned int q = duel_turn_player ^ i;
        unsigned int p = q & 1;

        for (j = 0; j < duel.players[p].n_d; j++) {
            if (duel.players[p].zone_d[j].id == 0x12A1
                && duel_slot_effect_with(q, 0xB, 0x12A1, CARD_UID(duel.players[p].zone_d[j]))) {
                duel_cmd(0x3F, (CARD_UID(duel.players[p].zone_d[j]) & 0x3FF) << 6 | (p | 0x1A), ((1 - q) & 1) | 0x5A, 0);
            }
        }
    }
    duel_cmd(0x5B, 0, 0, 0);
    duel_088C78E0(duel_turn_player);
    duel_088C78E0(1 - duel_turn_player);
}
#endif

void duel_088C7B90(int a)
{
    duel_cmd(10, a, 0, 0);
}

void duel_088C7BA4(void)
{
    unsigned short m;

    if (duel_w_08914924 == 0) {
        if (duel_w_089195EC == 0 || duel_player_type[duel_turn_player] != 2) {
            if (duel_w_08914918++ == 0) {
                duel_chain.n_kinds = 0;
                duel_w_089149D4 = 0;
                duel_w_0891491C++;
            }
        } else {
            m = 1;
            duel_msg_send(10, -1, &m, 2);
        }
    }
}

void duel_088C7C54(void)
{
    unsigned short m;

    if (duel_w_08914924 == 0) {
        if (duel_w_089195EC == 0 || duel_player_type[duel_turn_player] != 2) {
            if (duel_w_08914918++ == 0) {
                duel_w_0891491C++;
            }
        } else {
            m = 2;
            duel_msg_send(10, -1, &m, 2);
        }
    }
}

void duel_088C7CF0(void)
{
    unsigned short m;

    if (duel_w_08914924 == 0) {
        if (duel_w_089195EC == 0 || duel_player_type[duel_turn_player] != 2) {
            duel_w_08914918--;
        } else {
            m = 0;
            duel_msg_send(10, -1, &m, 2);
        }
    }
}

void duel_088C7D74(unsigned int bit, unsigned int on)
{
    if (on != (duel.mode >> bit & 1)) {
        duel_mode_set(bit, on);
        duel_cmd(9, bit, on, 0);
    }
}

void duel_088C7DD4(unsigned int p, unsigned int bit, unsigned int on)
{
    if (on != (duel.players[p & 1].flags >> bit & 1)) {
        duel_player_flag_set(p, bit, on);
        duel_cmd((p ? 0x8000 : 0) | 0x22, bit, on, 0);
    }
}

void duel_088C7E68(unsigned int p, unsigned int z, unsigned int bit, unsigned int on)
{
    if (on != (duel.players[p & 1].field[z].flags >> bit & 1)) {
        duel_088DF554(p, z, bit, on);
        duel_cmd((p ? 0x8000 : 0) | 0x2A, z, bit, on & 0xFFFF);
    }
}
