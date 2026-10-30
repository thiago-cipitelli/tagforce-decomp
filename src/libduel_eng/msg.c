// @range 0x088F71D8-0x088F7EB4
// @asm 0x088F7418-0x088F7EB4
/* Messages between the two consoles of a link duel: a command with optional argument and
   payload, the count of commands still waiting for an answer, and duel_msg_poll, which reads
   the next command from the other console and applies it to the local state. */
#include "duel.h"

typedef union DuelMsgState {           /* 0x08914340 */
    struct {
        unsigned int lo : 15;
        int pending : 4;               /* commands still unanswered */
        unsigned int hi : 13;
    };
    struct {
        unsigned char wait : 1;        /* bit 0: the other console asked to wait (commands 2, 3) */
        unsigned char b1 : 1;          /* command 0x17 */
        unsigned char b2 : 1;
        unsigned char b3 : 1;          /* command 8 */
        unsigned char b4 : 1;          /* commands 0xC, 0xF */
        unsigned char b5 : 1;          /* commands 0xD, 0xE, 0x10, 0x11 */
        unsigned char b6 : 2;
    };
} DuelMsgState;

typedef struct DuelEvFlags {           /* 0x08914341: answers received from the other console */
    unsigned char b0 : 1;
    unsigned char b1 : 1;              /* command 0x1F */
    unsigned char b2 : 1;              /* command 0x1A */
    unsigned char b3 : 1;              /* command 0x1B */
    unsigned char b4 : 1;              /* command 0x1C */
    unsigned char b5 : 1;              /* command 0x1D */
    unsigned char b6 : 1;              /* command 0x18 */
    unsigned char b7 : 1;
} DuelEvFlags;

extern DuelMsgState duel_msg_state;

void duel_memcpy(void *dst, void *src, int n);
void duel_msg_post(unsigned short *buf, int size);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
void duel_msg_count(int cmd, int inc);

void duel_msg_send3(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c)
{
    unsigned short m[4];

    m[1] = a;
    m[2] = b;
    m[3] = c;
    m[0] = cmd;
    duel_msg_send(cmd, -1, &m[1], 6);
}

/* Commands that wait for an answer change the pending count. */
void duel_msg_count(int cmd, int inc)
{
    switch (cmd) {
    case 2:
    case 7:
    case 0xC:
    case 0xF:
    case 0x15:
    case 0x18:
    case 0x1A:
    case 0x1C:
    case 0x1E:
    case 3:
    case 8:
    case 0xD:
    case 0xE:
    case 0x10:
    case 0x11:
    case 0x17:
    case 0x19:
    case 0x1B:
    case 0x1D:
    case 0x1F:
        if (inc != 0) {
            duel_msg_state.pending++;
        } else {
            duel_msg_state.pending--;
        }
        break;
    }
}

void duel_msg_send(unsigned short cmd, int arg, void *data, int len)
{
    unsigned short buf[128];
    int n = 0;

    buf[n++] = cmd;
    if (arg != -1) {
        buf[n++] = arg;
    }
    if (len > 0) {
        duel_memcpy(&buf[n], data, len);
        n += (unsigned int)(len + 1) >> 1;
    }
    duel_msg_count(cmd, 1);
    duel_msg_post(buf, n * 2);
}

/* Commands received from the other console: 0x08914040, a command word and its payload. */
extern unsigned short duel_msg_buf[0x180];
extern DuelEvFlags duel_ev_flags;      /* 0x08914341 */
extern int duel_net_end;               /* 0x089195F0 the link was closed or the peer gave up */
extern int duel_w_089147D0;
extern int duel_w_089147D4;            /* chain link the last command refers to */
extern int duel_w_08914920;
extern int duel_w_089149D8;
extern int duel_w_089149E0;
extern int duel_w_089149E4;
extern int duel_w_0891968C;
extern int duel_step;
extern unsigned int duel_view_player;
extern int duel_player_type[2];
extern unsigned int duel_ai_player;
extern int duel_ev_sent;
extern unsigned char duel_b_0891434D;
extern unsigned char duel_b_0891434E;
extern unsigned char duel_b_0891434F;

int duel_net_recv(void *buf);
int duel_net_sysvar(void);
int duel_msg_done(void);
int duel_0884B050(void);
void duel_088D4D70(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088AFD5C(int kind, unsigned int a);
void duel_088C7BA4(void);
void duel_088C7C54(void);
void duel_088C7CF0(void);
void duel_08835F34(unsigned int p, int z, int i, int d);
void duel_0884F07C(int which, unsigned int code, unsigned short uid, unsigned int arg);
void duel_atk_reset(void);
void duel_ev_set_list(unsigned int p, int kind, unsigned short *list, int n);
int card_is_monster(int id);
void duel_0884DBAC(DuelAct *a, int i);
int duel_0886B1F4(DuelAct *a);

#ifdef NON_MATCHING /* forma igual; o original não carrega $a1 antes de duel_msg_send3(3, 0, 0, 0) (4 bytes a menos) */
/* Reads one command from the other console and applies it. Returns 0 when the link is over. */
int duel_msg_poll(void)
{
    unsigned int size;
    int i;
    unsigned short *m;
    DuelAct *a;
    int n;

    if (duel_msg_state.wait) {
        return 1;
    }
    duel_msg_buf[0] = 0;
    size = duel_net_recv(duel_msg_buf); /* the original reads it back as the $v0 left by duel_msg_count */
    if (size == 0) {
        if (!duel_net_sysvar() || duel_msg_done()) {
            duel_net_end = 1;
            return 0;
        }
        return 1;
    }
    duel_msg_count(duel_msg_buf[0], 0);
    switch (duel_msg_buf[0]) {
    case 0:
        return 1;
    case 2:                            /* the other side waits for this one */
        duel_msg_state.wait = 1;
        duel_msg_send3(3, 0, 0, 0);
        if (duel_player_type[duel_view_player] == 0) {
            duel_0884B050();
        }
        return 1;
    case 3:
        duel_msg_state.wait = 1;
        return 1;
    case 0x15:                         /* attack record, then restart the attack */
        duel_memcpy(&duel_summon, &duel_msg_buf[1], 0x1C);
        duel_atk_reset();
        return 1;
    case 0x16:
        duel_memcpy(&duel_summon, &duel_msg_buf[1], 0x1C);
        return 1;
    case 0x17:
        duel_msg_state.b1 = 1;
        return 1;
    case 9:
        n = duel_msg_buf[2] | duel_msg_buf[3] << 16;
        duel_088AFD5C(duel_msg_buf[1], n);
        return 1;
    case 0xA:
        switch (duel_msg_buf[1]) {
        case 1:
            duel_088C7BA4();
            break;
        case 2:
            duel_088C7C54();
            break;
        case 0:
            duel_088C7CF0();
            break;
        }
        return 1;
    case 0xC:
        duel_w_089147D4 = 0;
        duel_w_089149E0 = 0;
        duel_w_089149E4 = 0;
        duel_msg_state.b4 = 1;
    case 0xB:                          /* kinds and sources of the pending chain entries */
        m = &duel_msg_buf[1];
        n = *m++;
        i = 0;
        duel_w_08914920 = n;
        for (; i < n; i++) {
            duel_chain.kinds[i] = m[0];
            *(unsigned int *)&duel_chain.srcs[i] = m[1] | m[2] << 16;
            m += 3;
        }
        return 1;
    case 0xD:
        duel_msg_state.b5 = 1;
        return 1;
    case 0xE:
        m = &duel_msg_buf[1];
        duel_ai_player = duel_view_player ^ 1;
        duel_08835F34(m[0], m[1], m[2], 3);
        duel_w_089149D8 = 1;
        duel_msg_state.b5 = 1;
        return 1;
    case 0x12:
        duel_memcpy(&duel_chain.acts[duel_msg_buf[1]], &duel_msg_buf[2], 0x18);
        return 1;
    case 0x13:
        duel_w_089147D4 = duel_msg_buf[1];
        return 1;
    case 0x14:
        duel_0884F07C(duel_msg_buf[1], duel_msg_buf[2] | duel_msg_buf[3] << 16, duel_msg_buf[6],
                      duel_msg_buf[4] | duel_msg_buf[5] << 16);
        return 1;
    case 0xF:
        duel_w_089147D4 = duel_msg_buf[1];
        duel_w_089149E0 = 0;
        duel_w_089149E4 = 1;
        duel_msg_state.b4 = 1;
        return 1;
    case 0x10:
        duel_msg_state.b5 = 1;
        return 1;
    case 0x11:
        m = &duel_msg_buf[1];
        duel_ai_player = duel_view_player ^ 1;
        duel_08835F34(m[0], m[1], m[2], 3);
        duel_w_089149D8 = 1;
        duel_msg_state.b5 = 1;
        return 1;
    case 0x1E:
        duel_ev_set_list(duel_view_player, duel_msg_buf[1], &duel_msg_buf[2], 8);
        duel_ev_sent = 1;
        return 1;
    case 0x1F:
        duel_memcpy(&duel_w_0891968C, &duel_msg_buf[1], 0x10);
        duel_ev_flags.b1 = 1;
        return 1;
    case 0x1C:
        duel_w_089147D4 = duel_msg_buf[1];
        duel_memcpy(&duel_chain.acts[duel_w_089147D4], &duel_msg_buf[2], 0x18);
        duel_b_0891434D = 0;
        duel_ev_flags.b4 = 1;
        return 1;
    case 0x1D:
        duel_ev_flags.b5 = 1;
        duel_memcpy(&duel_chain.acts[duel_w_089147D4], &duel_msg_buf[1], 0x18);
        return 1;
    case 0x1A:
        duel_w_089147D4 = duel_msg_buf[1];
        duel_memcpy(&duel_chain.acts[duel_w_089147D4], &duel_msg_buf[2], 0x18);
        duel_b_0891434E = 0;
        duel_ev_flags.b2 = 1;
        return 1;
    case 0x1B:
        duel_ev_flags.b3 = 1;
        duel_memcpy(&duel_chain.acts[duel_w_089147D4], &duel_msg_buf[1], 0x18);
        return 1;
    case 0x18:                         /* a chain link chosen by the other console */
        duel_w_089147D4 = duel_msg_buf[1];
        a = &duel_chain.acts[duel_w_089147D4];
        duel_memcpy(a, &duel_msg_buf[2], 0x18);
        if (!a->b4_1) {
            if (card_is_monster(a->id) == 0 && duel_view_player == (a->player ^ a->opp)) {
                duel.list[a->uid].b15 = 1;
            }
            duel_0884DBAC(a, duel_w_089147D4);
        }
        if (duel_0886B1F4(a) == 0) {
            duel_msg_send3(0x19, 0, 0, 0);
            return 1;
        }
        duel_b_0891434F = 0;
        duel_ev_flags.b6 = 1;
        return 1;
    case 0x19:
        duel_step = 0;
        if (size < 0x18) {
            return 1;
        }
        if (duel_w_089147D0 > 1) {         /* the last two links */
            duel_memcpy(&duel_chain.acts[duel_w_089147D0 - 2], &duel_msg_buf[1], 0x30);
        } else {
            duel_memcpy(&duel_chain.acts[duel_w_089147D0 - 1], &duel_msg_buf[1], 0x18);
        }
        return 1;
    case 6:
        return 1;
    case 7:
        duel_088D4D70(duel_msg_buf[1], duel_msg_buf[2], duel_msg_buf[3], duel_msg_buf[4]);
        return 1;
    case 8:
        duel_msg_state.b3 = 1;
        return 1;
    case 1:                            /* the other console ended the duel */
        duel_net_end = 1;
        return 0;
    default:
        duel_msg_send3(1, 0, 0, 0);
        duel_net_end = 1;
        return 0;
    }
}
#endif
