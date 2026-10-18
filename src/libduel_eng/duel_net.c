// @range 0x088FE160-0x088FE638
// @asm 0x088FE160-0x088FE32C
/* Online duel: duel messages exchanged through the ad hoc records of BOOT's netsync, and accessors
   of the duel state for the other modules. */
#include "duel.h"
#include "libc.h"
#include "duel_setup.h"

typedef struct DuelNetMsg {            /* 0x100, record of one player; sent from +0x04 */
    int len;
    signed char kind;                  /* +0x04 6: duel message */
    signed char cls;                   /* +0x05 duel_msg_class: 0, 2 apply now, 1 wait for every peer */
    signed char mask;                  /* +0x06 players that must apply it */
    signed char side;                  /* +0x07 */
    signed char from;                  /* +0x08 sender */
    unsigned char seq;                 /* +0x09 */
    short pad0a;
    int size;                          /* +0x0C */
    unsigned char data[0xF0];          /* +0x10 */
} DuelNetMsg;

typedef struct DuelNetRec {            /* 0x104, copy of peer_get(i) */
    DuelNetMsg m;
    int pad100;
} DuelNetRec;

extern DuelNetRec duel_net_cur;        /* 0x089196A0 */
extern DuelNetRec duel_net_in[4];      /* 0x089197B0 */
extern int duel_net_done[4];           /* 0x08919BC0 */
extern unsigned char duel_net_last[4][8]; /* 0x08919BD0 */
extern unsigned char duel_net_seq;     /* 0x08919BF0 */
extern unsigned char D_08914A30[];

void *game_state2_get(void);
int peer_count_get(void);
int peer_self_get(void);
void *peer_get(int i);
int sysvar_get_a98(void);
int memcmp2(const void *a, const void *b, int n);
int netsync_local_side(void);
void netsync_queue_push(void *rec, int size);
int duel_msg_class(int cmd);
int duel_tag_slot(int side);
void duel_memcpy(void *dst, void *src, int n);
int duel_cmd();
int duel_088D4CD8();

#ifdef NON_MATCHING /* the original gives `found` and `i` the other $s registers and fills the delay slots of the last two returns differently */
int duel_net_poll(void)
{
    int n;
    int found;
    int sel;
    int i;
    int cnt;

    game_state_get();
    n = peer_count_get();
    found = 0;
    sel = -1;
    for (i = 0; i < n; i++) {
        if (i == peer_self_get()) {
            continue;
        }
        if (duel_net_in[i].m.kind == 6 && duel_net_in[i].m.from != peer_self_get() &&
            (duel_net_in[i].m.mask & 1 << peer_self_get()) && memcmp2(duel_net_last[i], &duel_net_in[i].m.kind, 8) != 0) {
            memcpy(duel_net_last[i], &duel_net_in[i].m.kind, 8);
            switch (duel_net_in[i].m.cls) {
            case 0:
                sel = i;
                goto clear;
            case 2:
                sel = i;
                goto clear;
            case 1:
                duel_net_done[i] = 1;
                sel = i;
                found++;
                break;
            }
        }
    }
    if (found != 0) {
        cnt = 0;
        for (i = 0; i < n; i++) {
            if (i == peer_self_get()) {
                continue;
            }
            if (duel_net_done[i] != 0) {
                cnt++;
            }
        }
        if (n - 1 == cnt) {
            goto clear;
        }
        return -1;
    }
    return -1;
clear:
    memset(duel_net_done, 0, 4);
    return sel;
}
#endif

int duel_net_sysvar(void)
{
    return sysvar_get_a98();
}

/* Queue a duel message (cmd words) for the other players. */
int duel_msg_post(unsigned short *buf, int size)
{
    GameState *g;
    int from;
    DuelNetMsg m;
    DuelNetMsg *p;

    p = &m;
    g = game_state_get();
    game_state2_get();
    memset(p, 0, 0x100);
    m.kind = 6;
    m.cls = duel_msg_class(buf[0]);
    switch (m.cls) {
    case 0:
        m.mask = 0xF;
        break;
    case 1:
        from = duel_net_cur.m.from;
        m.mask = 1 << from;
        if (from == peer_self_get()) {
            return 0;
        }
        break;
    case 2:
        /* g->slot[k].net_id; the original adds the base first */
        m.mask = 1 << *(unsigned short *)((char *)g + duel_tag_slot(netsync_local_side() ^ 1) * 0x188 + 0x128);
        break;
    }
    m.side = netsync_local_side();
    m.from = peer_self_get();
    m.seq = duel_net_seq++;
    m.size = size;
    memcpy(p->data, buf, size);
    netsync_queue_push(&p->kind, size + 0xC);
    return 1;
}

/* Copy every player's record; when a message is complete, keep it in duel_net_cur and copy its
   data to buf. Returns the data size or 0. */
int duel_net_recv(void *buf)
{
    int i;
    int k;
    DuelNetRec *r;

    for (i = 0; i < 4; i++) {
        memcpy(&duel_net_in[i], peer_get(i), 0x104);
    }
    k = duel_net_poll();
    if (k < 0) {
        return 0;
    }
    r = &duel_net_in[k];
    memcpy(&duel_net_cur, r, 0x104);
    memcpy(buf, r->m.data, r->m.size);
    return r->m.size;
}

int duel_net_cmd()
{
    return duel_cmd();
}

int duel_net_088D4CD8()
{
    return duel_088D4CD8();
}

Duel *duel_get(void)
{
    return &duel;
}

void *duel_get_14a30(void)
{
    return D_08914A30;
}

RecipeData *duel_deck_get(int i)
{
    return &duel_setup.deck[i];
}

void duel_deck_set(int i, void *src)
{
    duel_memcpy(&duel_setup.deck[i], src, 0x108);
}

void duel_deck_set4(int i, void *src)
{
    switch (i) {
    case 0:
    case 1:
        duel_memcpy(&duel_setup.deck[i], src, 0x108);
        break;
    case 2:
    case 3:
        duel_memcpy(&duel_decks2[i - 2], src, 0x108);
        break;
    }
}
