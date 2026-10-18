// @range 0x088FE638-0x088FF0DC
// @asm 0x088FE724-0x088FE810
// @asm 0x088FEF00-0x088FEF6C
/* Exported accessors of the duel state (library libduel_eng, imported by NID by libduel_draw),
   duel input log and the host callback. */
#include "duel.h"
#include "duel_setup.h"

typedef struct DuelLogEnt {
    short v[4];
} DuelLogEnt;

typedef struct DuelLog {               /* duel input log */
    int n;
    DuelLogEnt e[0x7FF];               /* +0x04 */
} DuelLog;

extern int duel_player_type[2];
extern int duel_view_player;
extern unsigned int duel_turn_player;
extern int duel_w_08919624;
extern unsigned short duel_w_0891968C[2];
extern int duel_w_0891524C;
extern unsigned int duel_w_08915250;
extern unsigned short *duel_w_089147DC;
extern int duel_log_mode;              /* 0x0891A174 1: recording */
extern int duel_log_n;                 /* 0x0891A178 */
extern DuelLog *duel_log_buf;          /* 0x0891A17C */
extern void (*duel_host_cb)(void);     /* 0x0891A180 */
extern int duel_host_args[4];          /* 0x0891A184 */

void duel_0884B368(RecipeData *d);
void duel_0884B184();
int duel_piles_swap(int a, RecipeData *d);
int duel_tag_slot(int side);
GameState *game_state_get(void);
int duel_setup_10_get(void);
unsigned int duel_turn_get(void);
int *game_state2_get(void);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_0880C60C();
int duel_08809F2C();
int req_flag_get(int i);

void duel_start_decks(int a)
{
    if (a != 0) {
        duel_0884B368(duel_decks2);
    } else {
        duel_0884B184();
    }
}

int duel_088FE66C(int a)
{
    return duel_piles_swap(a, duel_decks2);
}

unsigned int duel_unk3830_get(int i)
{
    return duel_w_0891968C[i];
}

int duel_setup_228_get(void)
{
    return duel_setup.unk228;
}

int duel_player_protected(unsigned int p)
{
    unsigned int f = duel.players[p & 1].flags;
    unsigned int b24 = f >> 24 & 1;
    int r = (f & 1) || b24;

    r |= duel_slot_has_effect(p, 0xB, 0x14FF);
    return r | duel_slot_has_effect(p, 0xB, 0x1548);
}

#ifdef NON_MATCHING /* the two tables are local arrays copied from templates at 0x08913530 kept in the asm */
/* Player slot (0-3) acting for side `side` in the current turn of a tag duel. */
int duel_tag_slot(int side)
{
    GameState *g = game_state_get();
    unsigned char t0[2][4] = { { 0, 0, 2, 2 }, { 3, 1, 1, 3 } };
    unsigned char t1[2][4] = { { 2, 0, 0, 2 }, { 1, 1, 3, 3 } };

    if (g->partner_mode == 1) {
        return side;
    }
    if (duel_setup_10_get() != 0) {
        return t1[side][duel_turn_get() & 3];
    }
    return t0[side][duel_turn_get() & 3];
}
#endif

int duel_088FE810(int side)
{
    int *g = game_state2_get();
    int v = g[duel_tag_slot(side)];

    if (v <= 0) {
        return v;
    }
    return v / 30 + 1;
}

int duel_088FE87C()
{
    return duel_0880C60C();
}

unsigned int duel_mode_bit(int i)
{
    return duel.mode >> i & 1;
}

unsigned int duel_turn_player_get(void)
{
    return duel_turn_player;
}

int duel_phase_get(void)
{
    return duel.phase;
}

int duel_phase_is_main(void)
{
    return duel.phase == 2 || duel.phase == 4;
}

unsigned int duel_turn_get(void)
{
    return duel.turn;
}

int duel_378c_is3(void)
{
    return duel.unk378c == 3;
}

int duel_3790_get(void)
{
    return duel.unk3790;
}

void duel_setup_mode_set(int v)
{
    duel_setup.mode = v;
}

int duel_setup_mode_get(void)
{
    return duel_setup.mode;
}

void duel_view_player_set(int p)
{
    duel_view_player = p;
}

int duel_view_player_get(void)
{
    return duel_view_player;
}

int duel_view_opponent_get(void)
{
    return duel_view_player ^ 1;
}

int duel_is_view_player(int p)
{
    return p == duel_view_player;
}

int duel_is_view_opponent(int p)
{
    return p == (duel_view_player ^ 1);
}

void duel_player_type_set(int p, int t)
{
    duel_player_type[p] = t;
}

int duel_player_is_type0(int p)
{
    return duel_player_type[p] == 0;
}

int duel_player_is_cpu(int p)
{
    return duel_player_type[p] == 1;
}

int duel_player_is_type2(int p)
{
    return duel_player_type[p] == 2;
}

int duel_player0_is_type3(void)
{
    return duel_player_type[0] == 3;
}

void duel_setup_10_set(int v)
{
    duel_setup.unk10 = v;
}

int duel_setup_10_get(void)
{
    return duel_setup.unk10;
}

void duel_setup_224_set(int v)
{
    duel_setup.unk224 = v;
}

int duel_setup_224_get(void)
{
    return duel_setup.unk224;
}

void duel_setup_228_set(int v)
{
    duel_setup.unk228 = v;
}

int duel_setup_228_get2(void)
{
    return duel_setup.unk228;
}

int duel_lp_get(unsigned int p)
{
    return duel.players[p & 1].lp;
}

void duel_lp_set(unsigned int p, int lp)
{
    duel.players[p & 1].lp = lp;
}

int duel_list_id(int i)
{
    return duel.list[i].id;
}

int duel_hand_uid(unsigned int p, int i)
{
    return CARD_UID(duel.players[p & 1].hand[i]);
}

int duel_zone_d_uid(unsigned int p, int i)
{
    return CARD_UID(duel.players[p & 1].zone_d[i]);
}

int duel_zone_c_uid(unsigned int p, int i)
{
    return CARD_UID(duel.players[p & 1].zone_c[i]);
}

int duel_grave_uid(unsigned int p, int i)
{
    return CARD_UID(duel.players[p & 1].grave[i]);
}

int duel_zone_f_uid(unsigned int p, int i)
{
    return CARD_UID(duel.players[p & 1].zone_f[i]);
}

unsigned int duel_n_hand(unsigned int p)
{
    return duel.players[p & 1].n_hand;
}

int duel_n_d(unsigned int p)
{
    return duel.players[p & 1].n_d;
}

int duel_n_18(unsigned int p)
{
    return duel.players[p & 1].n_18;
}

unsigned int duel_n_grave(unsigned int p)
{
    return duel.players[p & 1].n_grave;
}

unsigned int duel_n_f(unsigned int p)
{
    return duel.players[p & 1].n_f;
}

int duel_slot_id(unsigned int p, int z)
{
    return duel.players[p & 1].field[z].card.id;
}

int duel_slot_uid(unsigned int p, int z)
{
    return CARD_UID(duel.players[p & 1].field[z].card);
}

unsigned int duel_slot_faceup(unsigned int p, int z)
{
    return duel.players[p & 1].field[z].faceup;
}

unsigned int duel_slot_pos(unsigned int p, int z)
{
    return duel.players[p & 1].field[z].pos;
}

unsigned int duel_slot_flag5(unsigned int p, int z)
{
    return duel.players[p & 1].field[z].flags >> 5 & 1;
}

unsigned int duel_slot_link(unsigned int p, int z)
{
    return duel.players[p & 1].field[z].link;
}

int duel_w624_get(void)
{
    return duel_w_08919624;
}

void duel_w624_set(int v)
{
    duel_w_08919624 = v;
}

int duel_5250_in_range(void)
{
    int r = 0;
    unsigned int x = duel_w_08915250;

    if (x <= 6) {
        goto end;
    }
    if (x >= 0x49) {
        goto end;
    }
    r = 1;
end:
    return r;
}

int *duel_list2_get(void)
{
    return &duel_w_0891524C;
}

int duel_088FEE74()
{
    return duel_08809F2C();
}

unsigned int duel_player_flag(unsigned int p, int i)
{
    return duel.players[p & 1].flags >> i & 1;
}

DuelSpecial *duel_special_get(void)
{
    return &duel_special;
}

unsigned int duel_47dc_get(void)
{
    if (duel_w_089147DC != 0) {
        return *duel_w_089147DC;
    }
    return 0;
}

DuelChain *duel_chain_get(void)
{
    return &duel_chain;
}

int duel_unk2c(unsigned int p)
{
    return duel.players[p & 1].unk20[3];
}

#ifdef NON_MATCHING /* the original keeps the count in a3 and loads the buffer pointer before storing it back */
void duel_log_push(DuelLogEnt *v)
{
    if (duel_log_mode == 1 && duel_log_n < 0x7FF) {
        DuelLogEnt *e = &duel_log_buf->e[duel_log_n];

        duel_log_n++;
        *e = *v;
    }
}
#endif

DuelLogEnt *duel_log_cur(void)
{
    return &duel_log_buf->e[duel_log_n];
}

void duel_log_next(void)
{
    duel_log_n++;
}

void duel_log_rewind(void)
{
    duel_log_n = 0;
}

int duel_log_at_end(void)
{
    return !(duel_log_n < duel_log_buf->n);
}

void duel_log_init(int mode, DuelLog *buf)
{
    duel_log_mode = mode;
    duel_log_n = 0;
    duel_log_buf = buf;
}

DuelLog *duel_log_end(void)
{
    if (duel_log_mode == 1) {
        duel_log_buf->n = duel_log_n;
    }
    return duel_log_buf;
}

int duel_host_call(int a, int b, int c, int d)
{
    duel_host_args[0] = a;
    duel_host_args[1] = b;
    duel_host_args[2] = c;
    duel_host_args[3] = d;
    duel_host_cb();
    return 1;
}

int duel_req_flag(int i)
{
    if (i < 0 || i >= 0x60) {
        return 0;
    }
    return req_flag_get(i);
}

void duel_host_cb_set(void (*cb)(void))
{
    duel_host_cb = cb;
}

void duel_host_args_get(int *out)
{
    out[0] = duel_host_args[0];
    out[1] = duel_host_args[1];
    out[2] = duel_host_args[2];
    out[3] = duel_host_args[3];
}
