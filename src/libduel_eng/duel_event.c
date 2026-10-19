// @range 0x088324F0-0x08832B54
/* Duel events for the presentation: one pending record (player, kind, two halfwords or a short
   list) that duel_ev_flush sends to the other player as message 0x1E when it is the local side's. */
#include "duel.h"

typedef struct DuelEvFlags {
    unsigned char b0 : 1;
    unsigned char b1 : 1;              /* cleared when an event is sent */
    unsigned char b2 : 6;
} DuelEvFlags;

extern unsigned int duel_view_player;
extern unsigned int duel_turn_player;
extern int duel_ev_sent;               /* 0x08919668 */
extern int duel_ev_pending;            /* 0x0891966C */
extern unsigned int duel_w_08919670;   /* player of the event */
extern int duel_ev_kind;               /* 0x08919674 */
extern int duel_ev_unk;                /* 0x08919678 */
extern unsigned short duel_ev_data[8]; /* 0x0891967C */
extern int duel_w_089195EC;            /* nonzero: duel against another console */
extern DuelEvFlags duel_ev_flags;      /* 0x08914341 */

void duel_memcpy(void *dst, void *src, int n);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);

void duel_ev_flush(void)
{
    unsigned short msg[9];

    duel_ev_pending = 1;
    duel_ev_unk = 0;
    duel_ev_sent = 0;
    if (duel_w_08919670 == (duel_view_player ^ 1) && duel_w_089195EC != 0) {
        msg[0] = duel_ev_kind;
        duel_memcpy(&msg[1], duel_ev_data, 0x10);
        duel_msg_send(0x1E, -1, msg, 0x12);
        duel_ev_flags.b1 = 0;
        duel_ev_sent = 1;
    }
}

void duel_ev_set(unsigned int p, int kind, unsigned short a, unsigned short b)
{
    duel_w_08919670 = p;
    duel_ev_kind = kind;
    duel_ev_data[0] = a;
    duel_ev_data[1] = b;
    duel_ev_flush();
}

void duel_ev_set_list(unsigned int p, int kind, unsigned short *list, int n)
{
    if (n > 8) {
        n = 8;
    }
    duel_w_08919670 = p;
    duel_ev_kind = kind;
    duel_memcpy(duel_ev_data, list, n << 1);
    duel_ev_flush();
}

void duel_08832624(unsigned int p, unsigned int id, int a, int b)
{
    unsigned short f = 0;

    if (id != 0) {
        if (a != 0) {
            f |= 1;
        }
        if (b != 0) {
            f |= 2;
        }
        duel_ev_set(p, 1, id, f);
    }
}

void duel_08832670(unsigned int p, unsigned int id, int a, int b)
{
    unsigned short f = 0;

    if (id != 0) {
        if (a == 1) {
            f |= 1;
        }
        if (b != 0) {
            f |= 2;
        }
        if (a == 8) {
            f |= 4;
        }
        if (a == 7) {
            f |= 8;
        }
        duel_ev_set(p, 1, id, f);
    }
}

void duel_088326E8(unsigned int p, unsigned int id)
{
    if (id != 0) {
        duel_ev_set(p, 1, id, 0x80);
    }
}

void duel_08832734(unsigned int p, int id, int a);

void duel_08832710(unsigned int p, int id, int a, int b)
{
    int f = 0;

    if (a != 0) {
        f = 1;
    }
    if (b != 0) {
        f = 7;
    }
    duel_08832734(p, id, f);
}

void duel_08832734(unsigned int p, int id, int a)
{
    unsigned short f = 0;

    if (a == 1) {
        f |= 1;
    }
    if (a == 8) {
        f |= 4;
    }
    if (a == 7) {
        f |= 8;
    }
    duel_ev_set(p, 2, id, f | 0x10);
}

void duel_08832788(unsigned int p, int id, int a, int b)
{
    unsigned short f = 0;

    if (a != 0) {
        f |= 1;
    }
    if (b != 0) {
        f |= 8;
    }
    duel_ev_set(p, 2, id, f | 0x30);
}

void duel_088327C0(unsigned int p, int id, int a, int b)
{
    unsigned short f = 0;

    if (a != 0) {
        f |= 1;
    }
    if (b != 0) {
        f |= 8;
    }
    duel_ev_set(p, 2, id, f | 0x20);
}

void duel_088327F8(unsigned int p, int id, int n)
{
    if (duel_ev_pending != 0 && duel_w_08919670 == p && duel_ev_kind == 3) {
        duel_ev_data[1] += (unsigned short)n;
    } else {
        duel_ev_set(p, 3, id, n);
    }
}

void duel_08832870(unsigned int p, int b)
{
    duel_ev_set(p, 4, 0, b);
}

void duel_08832880(unsigned int p, int a, int b)
{
    duel_ev_set(p, 5, a, b);
}

void duel_08832894(unsigned int p, int a)
{
    duel_ev_set(p, 6, a, 0);
}

void duel_088328A4(unsigned int p, int a, int b)
{
    duel_ev_set(p, 6, a, b);
}

void duel_088328B8(unsigned int p, unsigned short a, int b)
{
    duel_ev_set(p, 8, a, b);
}

void duel_088328C8(unsigned int p, unsigned short a, unsigned int x, unsigned int y)
{
    duel_ev_set(p, 8, a, (y & 0xFF) | (x & 0xFF) << 8);
}

void duel_088328E8(unsigned int p, int a, int b)
{
    duel_ev_set(p, 9, a, b);
}

void duel_088328FC(unsigned int p)
{
    if (duel.players[(1 - p) & 1].n_d != 0) {
        duel_ev_set(p, 7, 0, 0);
    }
}

void duel_08832950(unsigned int p, int a)
{
    duel_ev_set(p, 0xF, a, 0);
}

void duel_08832960(unsigned int p, int a, int b)
{
    duel_ev_set(p, 0xF, a, b);
}

void duel_08832974(unsigned int p, int a, int b)
{
    duel_ev_set(p, 0x10, a, b);
}

void duel_08832988(unsigned int p, int a, unsigned int x, unsigned int y)
{
    duel_ev_set(p, 0xB, a, (x & 0xFF) | (y & 0xFF) << 8);
}

void duel_088329AC(unsigned int p, int a)
{
    duel_ev_set(p, 10, a, 0);
}

void duel_088329BC(unsigned int p, int a, int b)
{
    duel_ev_set(p, 0xE, a, b);
}

void duel_088329D0(unsigned int p, int a, int b)
{
    duel_ev_set(p, 0xC, a, b);
}

void duel_088329E4(unsigned int p, int a, unsigned short *list, int n)
{
    unsigned short buf[8];
    int i;

    buf[0] = a;
    for (i = 0; i < n; i++) {
        buf[i + 1] = list[i];
    }
    duel_ev_set_list(p, 0xC, buf, n + 1);
}

void duel_08832ADC(int a)
{
    duel_ev_set(duel_turn_player, 0x11, a, 0);
}

void duel_08832AF4(unsigned int p, int a, unsigned int x, unsigned int y)
{
    duel_ev_set(p, 0x12, a, (x & 0xFF) | (y & 0xFF) << 8);
}

void duel_08832B18(unsigned int p, int a)
{
    duel_ev_set(p, 0x13, a, 0);
}

void duel_08832B28(unsigned int p, int a, int b)
{
    duel_ev_set(p, 0x13, a, b - 1);
}

void duel_08832B40(unsigned int p, int a, int b)
{
    duel_ev_set(p, 0x14, a, b);
}
