// @range 0x088F71D8-0x088F7418
/* Messages from the rules engine to the presentation side: a command with optional argument and
   payload, and the count of commands still waiting for an answer. */
#include "duel.h"

typedef struct DuelMsgState {          /* 0x08914340 */
    unsigned int lo : 15;
    int pending : 4;                   /* commands still unanswered */
    unsigned int hi : 13;
} DuelMsgState;

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
