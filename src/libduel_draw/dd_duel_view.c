// @range 0x088666D8-0x08866EC8
// @asm 0x08866A34-0x08866A88
/* Duel engine seen from the screen: side 0 is the player at the bottom of the screen, side 1 the
   one at the top. The two tables translate between sides and engine players, and the wrappers
   call libduel_eng with the side translated to the player (most are tail calls). */
#include "libduel_draw.h"
#include "duel.h"
#include "allegrex.h"

extern int dv_side_player[2];        /* 0x08C5B978 side -> engine player */
extern int dv_player_side[2];        /* 0x08C5B980 engine player -> side */

unsigned int duel_mode_bit(int i);
unsigned int duel_turn_player_get(void);
int duel_phase_get(void);
unsigned int duel_turn_get(void);
int duel_list_id(int i);
int duel_hand_uid(unsigned int p, int i);
int duel_zone_d_uid(unsigned int p, int i);
int duel_zone_c_uid(unsigned int p, int i);
int duel_grave_uid(unsigned int p, int i);
int duel_zone_f_uid(unsigned int p, int i);
unsigned int duel_n_hand(unsigned int p);
int duel_n_d(unsigned int p);
int duel_n_18(unsigned int p);
unsigned int duel_n_grave(unsigned int p);
unsigned int duel_n_f(unsigned int p);
int duel_slot_uid(unsigned int p, int z);
unsigned int duel_slot_faceup(unsigned int p, int z);
unsigned int duel_slot_pos(unsigned int p, int z);
unsigned int duel_slot_link(unsigned int p, int z);
int duel_view_player_get(void);
int duel_is_view_player(int p);
int duel_player_is_type0(int p);
int duel_088FE810(int side);
void duel_w624_set(int v);
int *duel_list2_get(void);
int duel_must_answer(int p);
unsigned int duel_08835AEC(unsigned int p, int z, unsigned int i);
void duel_08835F34(unsigned int p, int z, int i, int k);
void duel_08836028(void);
void duel_0883603C(void);
int duel_08835924(void);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_field_locate(int uid);
int duel_hand_find_uid(unsigned int p, int uid);
int duel_can_attack_any(unsigned int p, int z, int arg);
int duel_slot_value_1909(unsigned int p, int z);
unsigned int duel_slot_min_1130(unsigned int p, int z);
int duel_slot_e68a0(unsigned int p, int z);
unsigned int duel_slot_link_1a0c(unsigned int p, int z);
int duel_slot_counter_1a90(unsigned int p, int z);
int duel_slot_counter_1513(unsigned int p, int z);
int duel_slot_counter_1439(unsigned int p, int z);
int field_get_card(unsigned int p, int z);
int duel_zone_usable(unsigned int p, unsigned int z);
int duel_zone_card_id(unsigned int p, int z, int i);
int duel_zone_pos(unsigned int p, int z, int i);
int duel_zone_faceup(unsigned int p, int z, int i);
unsigned int duel_zone_count(unsigned int p, int z);
int duel_zone_card_uid(unsigned int p, int z, int i);
int duel_list_card_level(unsigned int p, int i);
void duel_slot_markers(unsigned int p, int z, unsigned short (*out)[11]);
int duel_08832D08(void);
int duel_088B2094(void);
void duel_0884C7C0(int i);
int duel_0884C824(void);
unsigned int duel_0884C830(int i);
DuelCard *duel_0884C7EC(int i);
unsigned int duel_0884C8E4(int i);
void duel_0884CB8C(int i, int v);
void duel_088C936C(unsigned int p, int a, int b, int id, int opp);
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp);
void duel_088C452C(unsigned int p, int n);
unsigned int duel_47dc_get(void);
int duel_088695B8(unsigned int p, unsigned int q, int z, int a);
int duel_08869728(unsigned int p, int i, int a);
int duel_088BFFD8(unsigned int p, int id, int i);
DuelSpecial *duel_special_get(void);
DuelChain *duel_chain_get(void);
int duel_unk2c(unsigned int p);

#define P(side) dv_side_player[side]

void dv_init(void)
{
    if (duel_is_view_player(1)) {
        dv_player_side[0] = 1;
        dv_player_side[1] = 0;
    } else {
        dv_player_side[0] = 0;
        dv_player_side[1] = 1;
    }
    dv_side_player[0] = dv_player_side[0];
    dv_side_player[1] = dv_player_side[1];
}

int dv_player_to_side(int p)
{
    return dv_player_side[p];
}

int dv_side_to_player(int side)
{
    return dv_side_player[side];
}

unsigned int dv_mode_bit(int i)
{
    return duel_mode_bit(i);
}

int dv_turn_side(void)
{
    return dv_player_side[duel_turn_player_get()];
}

int dv_phase(void)
{
    return duel_phase_get();
}

unsigned int dv_turn(void)
{
    return duel_turn_get();
}

int dv_list_id(int i)
{
    return duel_list_id(i);
}

int dv_hand_uid(int side, int i)
{
    return duel_hand_uid(P(side), i);
}

int dv_zone_d_uid(int side, int i)
{
    return duel_zone_d_uid(P(side), i);
}

int dv_zone_c_uid(int side, int i)
{
    return duel_zone_c_uid(P(side), i);
}

int dv_grave_uid(int side, int i)
{
    return duel_grave_uid(P(side), i);
}

int dv_zone_f_uid(int side, int i)
{
    return duel_zone_f_uid(P(side), i);
}

unsigned int dv_n_hand(int side)
{
    return duel_n_hand(P(side));
}

int dv_n_d(int side)
{
    return duel_n_d(P(side));
}

int dv_n_18(int side)
{
    return duel_n_18(P(side));
}

unsigned int dv_n_grave(int side)
{
    return duel_n_grave(P(side));
}

unsigned int dv_n_f(int side)
{
    return duel_n_f(P(side));
}

int dv_slot_uid(int side, int z)
{
    return duel_slot_uid(P(side), z);
}

unsigned int dv_slot_faceup(int side, int z)
{
    return duel_slot_faceup(P(side), z);
}

unsigned int dv_slot_pos(int side, int z)
{
    return duel_slot_pos(P(side), z);
}

unsigned int dv_slot_link(int side, int z)
{
    return duel_slot_link(P(side), z);
}

/* 2 when the engine is watched from player 2 (replay/spectator), else 0 */
int dv_player_is2(void)
{
    return duel_view_player_get() == 2 ? 2 : 0;
}

int dv_is_view_side(int side)
{
    return duel_is_view_player(P(side));
}

int dv_is_type0(int side)
{
    return duel_player_is_type0(P(side));
}

int dv_088FE810(int side)
{
    return duel_088FE810(P(side));
}

void dv_w624_set(int v)
{
    duel_w624_set(v);
}

int *dv_list2(void)
{
    return duel_list2_get();
}

int dv_player_side_bit(int p)
{
    return dv_player_side[p & 1];
}

int dv_must_answer(int side)
{
    return duel_must_answer(P(side));
}

unsigned int dv_08835AEC(int side, int z, unsigned int i)
{
    return duel_08835AEC(P(side), z, i);
}

void dv_08835F34(int side, int z, int i, int k)
{
    duel_08835F34(P(side), z, i, k);
}

void dv_08836028(void)
{
    duel_08836028();
}

void dv_0883603C(void)
{
    duel_0883603C();
}

int dv_08835924(void)
{
    return duel_08835924();
}

void dv_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c)
{
    duel_cmd(cmd, a, b, c);
}

void dv_field_stats(int side, int z, DuelStats *st)
{
    duel_field_stats(P(side), z, st);
}

/* location of a card: low byte the side, high byte the zone (-1 when not on the field) */
#ifdef NON_MATCHING /* o original calcula o índice e a zona em outra ordem (ext depois do lbu) */
int dv_field_locate(int uid)
{
    int r = duel_field_locate(uid);
    unsigned short w;

    if (r < 0)
        return r;
    w = r;
    return (unsigned short)(dv_player_side[r & 0xFF] & 0xFF | ALLEGREX_EXT(w, 8, 8) << 8);
}
#else
int dv_field_locate(int uid);
#endif

int dv_hand_find_uid(int side, int uid)
{
    return duel_hand_find_uid(P(side), uid);
}

int dv_can_attack_any(int side, int z, int arg)
{
    return duel_can_attack_any(P(side), z, arg);
}

int dv_slot_value_1909(int side, int z)
{
    return duel_slot_value_1909(P(side), z);
}

unsigned int dv_slot_min_1130(int side, int z)
{
    return duel_slot_min_1130(P(side), z);
}

int dv_slot_e68a0(int side, int z)
{
    return duel_slot_e68a0(P(side), z);
}

unsigned int dv_slot_link_1a0c(int side, int z)
{
    return duel_slot_link_1a0c(P(side), z);
}

int dv_slot_counter_1a90(int side, int z)
{
    return duel_slot_counter_1a90(P(side), z);
}

int dv_slot_counter_1513(int side, int z)
{
    return duel_slot_counter_1513(P(side), z);
}

int dv_slot_counter_1439(int side, int z)
{
    return duel_slot_counter_1439(P(side), z);
}

int dv_field_card(int side, int z)
{
    return field_get_card(P(side), z);
}

int dv_zone_usable(int side, unsigned int z)
{
    return duel_zone_usable(P(side), z);
}

int dv_zone_card_id(int side, int z, int i)
{
    return duel_zone_card_id(P(side), z, i);
}

int dv_zone_pos(int side, int z, int i)
{
    return duel_zone_pos(P(side), z, i);
}

int dv_zone_faceup(int side, int z, int i)
{
    return duel_zone_faceup(P(side), z, i);
}

unsigned int dv_zone_count(int side, int z)
{
    return duel_zone_count(P(side), z);
}

int dv_zone_card_uid(int side, int z, int i)
{
    return duel_zone_card_uid(P(side), z, i);
}

int dv_list_card_level(int side, int i)
{
    return duel_list_card_level(P(side), i);
}

/* markers of both sides, the rows reordered from players to sides */
void dv_slot_markers(int side, int z, unsigned short (*out)[11])
{
    unsigned short m[2][11];
    int s, i;

    duel_slot_markers(P(side), z, m);
    for (s = 0; s < 2; s++)
        for (i = 0; i < 11; i++)
            out[s][i] = m[dv_side_player[s]][i];
}

int dv_08832D08(void)
{
    return duel_08832D08();
}

int dv_088B2094(void)
{
    return duel_088B2094();
}

void dv_0884C7C0(int i)
{
    duel_0884C7C0(i);
}

int dv_0884C824(void)
{
    return duel_0884C824();
}

unsigned int dv_0884C830(int i)
{
    return duel_0884C830(i);
}

DuelCard *dv_0884C7EC(int i)
{
    return duel_0884C7EC(i);
}

unsigned int dv_0884C8E4(int i)
{
    return duel_0884C8E4(i);
}

void dv_0884CB8C(int i, int v)
{
    duel_0884CB8C(i, v);
}

void dv_088C936C(int side, int a, int b, int id, int opp)
{
    duel_088C936C(P(side), a, b, id, opp);
}

int dv_088C9878(int side, int z, int x, int y, int id, int opp)
{
    return duel_088C9878(P(side), z, x, y, id, opp);
}

void dv_088C452C(int side, int n)
{
    duel_088C452C(P(side), n);
}

unsigned int dv_47dc(void)
{
    return duel_47dc_get();
}

int dv_088695B8(int side, int other, int z, int a)
{
    return duel_088695B8(P(side), P(other), z, a);
}

int dv_08869728(int side, int i, int a)
{
    return duel_08869728(P(side), i, a);
}

int dv_088BFFD8(int side, int id, int i)
{
    return duel_088BFFD8(P(side), id, i);
}

DuelSpecial *dv_special(void)
{
    return duel_special_get();
}

DuelChain *dv_chain(void)
{
    return duel_chain_get();
}

int dv_unk2c(int side)
{
    return duel_unk2c(P(side));
}

void popup_init(void)
{
    memset(&popup, 0, sizeof(popup));
}

void popup_term(void)
{
    memset(&popup, 0, sizeof(popup));
}
