// @range 0x0886E0A4-0x0886ED2C
// @asm 0x0886E33C-0x0886E54C
// @asm 0x0886E988-0x0886EBA0
/* Ad hoc state exchange over one PDP port (0x315): every player owns a 0x100-byte record;
   the local record is sent to the other PSP and the received one copied into peer_records.
   In queued mode (netsync_mode) packets go through a ring of 32 heap copies, one per 5 frames.
   netsync_transfer moves a block of 0x80-byte chunks from the host to the guest. */
#include "psp.h"
#include "libc.h"
#include "game.h"
#include "net.h"

extern unsigned char peer_records[4][0x100];   /* 0x08E34BF0 */
extern unsigned char local_record[0x100];      /* 0x08E34FF0 */
extern unsigned char netsync_heap_mem[0x800];  /* 0x08E350F0 */
extern unsigned char netsync_mac[6];           /* 0x08E358F0 own ether address */
extern unsigned char netsync_host_mac[6];      /* 0x08E358F6 */
extern int netsync_pdp;                        /* 0x08E358FC */
extern int netsync_is_host;                    /* 0x08E35900 */
extern int netsync_dirty;                      /* 0x08E35904 */
extern int netsync_mode;                       /* 0x08E35908 1: queued packets */
typedef struct NetXfer {
    int size;                          /* bytes per player */
    unsigned char *src;
    unsigned char *dst;                /* size bytes per player */
    int ready;                         /* every player answered the start packet */
} NetXfer;

typedef struct NetQueue {              /* 0x10C */
    int head;
    int tail;
    int size[32];
    void *buf[32];
    void *heap;                        /* +0x108 */
} NetQueue;

extern NetXfer netsync_xfer;                   /* 0x08E35910 */
extern NetQueue netsync_q;                     /* 0x08E35920 */
extern int netsync_q_delay;                    /* 0x08E35A2C */
extern int netsync_send_size;                  /* 0x08E35A30 */
extern unsigned int net_frame_count;           /* 0x08E1FD90 */

void *heap_create(void *mem, int size);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
int sysvar_get_a98(void);
int memcmp2(const void *a, const void *b, int n);
int sceNetGetLocalEtherAddr(unsigned char *mac);
int sceNetAdhocPdpCreate(unsigned char *mac, unsigned short port, unsigned int bufsize, int unk);
int sceNetAdhocPdpDelete(int id, int unk);
int sceNetAdhocPdpSend(int id, unsigned char *mac, unsigned short port, void *data, unsigned int len, unsigned int timeout, int nonblock);
int sceNetAdhocPdpRecv(int id, unsigned char *mac, unsigned short *port, void *data, int *len, unsigned int timeout, int nonblock);

void netsync_init(void);
int netsync_exchange_queued(void);
int peer_count_get(void);
int peer_self_get(void);
unsigned char *peer_get();
unsigned char *local_record_get(void);
void local_record_set(unsigned char *rec);
void local_record_set1(unsigned char v);
int peers_all_match(int a, int b, int c);
int peer_self_byte1(void);
int peer_self_byte2(void);
int peer0_byte2(void);

int netsync_open(void)
{
    netsync_pdp = -1;
    sceNetGetLocalEtherAddr(netsync_mac);
    if (net.hello.role == 1) {
        netsync_is_host = 1;
    } else {
        netsync_is_host = 0;
        memcpy(netsync_host_mac, net_peers[net.peer_sel].mac, 6);
    }
    netsync_pdp = sceNetAdhocPdpCreate(netsync_mac, 0x315, 0x800, 0);
    memset(local_record, 0, 0x100);
    memset(peer_records, 0, 0x400);
    netsync_init();
    return 0;
}

int netsync_close(void)
{
    int r = 0;

    if (netsync_pdp >= 0) {
        r = sceNetAdhocPdpDelete(netsync_pdp, 0);
        netsync_pdp = -1;
    }
    return r;
}

/* Per frame: send our record when it changed, take every pending packet of the other PSP. */
int netsync_exchange(void)
{
    unsigned short port;
    unsigned char mac[6];
    unsigned char data[0x400];
    int len = 0;
    int self;
    int other;

    if (netsync_mode != 0) {
        return netsync_exchange_queued();
    }
    self = peer_self_get();
    other = self ^ 1;
    if (net_frame_count & 1) {
        memcpy(peer_records[self], local_record, 0x100);
        memcpy(mac, net.macs[other], 6);
        sceNetAdhocPdpSend(netsync_pdp, mac, 0x315, local_record, 0x100, 0, 1);
        netsync_dirty = 0;
    }
    for (;;) {
        len = 0x100;
        if (sceNetAdhocPdpRecv(netsync_pdp, mac, &port, data, &len, 0, 1) == 0x80410709) {
            break;
        }
        if (port == 0x315 && memcmp2(net.macs[other], mac, 6) == 0) {
            memcpy(peer_records[other], data, 0x100);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* queue_pop: lógica conferida; só a alocação difere (o original põe head em a3 e reaproveita a1 para head+1): 11 palavras. exchange_queued loads the pdp id and the size in the other order */
/* Moves the oldest queued packet into the local record after 5 frames. */
void netsync_queue_pop(void)
{
    NetQueue *q = (NetQueue *)(char *)&netsync_q;
    unsigned char *rec;
    int i;
    int size;

    if (netsync_q_delay < 5) {
        netsync_q_delay++;
    }
    if (q->head != q->tail && netsync_q_delay >= 5) {
        rec = local_record_get();
        memcpy(rec + 4, q->buf[q->head], q->size[q->head]);
        heap_set_level(q->heap, q->buf[q->head]);
        i = q->head;
        size = q->size[i];
        netsync_q_delay = 0;
        q->head = (i + 1) & 0x1F;
        netsync_send_size = size + 4;
    }
}

int netsync_exchange_queued(void)
{
    unsigned short port;
    unsigned char mac[6];
    unsigned char data[0x400];
    int len = 0;
    int self;
    int other;

    self = peer_self_get();
    other = self ^ 1;
    netsync_queue_pop();
    memcpy(peer_records[self], local_record, netsync_send_size);
    memcpy(mac, net.macs[other], 6);
    sceNetAdhocPdpSend(netsync_pdp, mac, 0x315, local_record, netsync_send_size, 0, 1);
    for (;;) {
        len = 0x100;
        if (sceNetAdhocPdpRecv(netsync_pdp, mac, &port, data, &len, 0, 1) == 0x80410709) {
            break;
        }
        if (port == 0x315 && memcmp2(net.macs[other], mac, 6) == 0) {
            memcpy(peer_records[other], data, len);
        }
    }
    return 0;
}
#endif

void netsync_set_mode(int mode)
{
    netsync_mode = mode;
}

void netsync_init(void)
{
    NetQueue *q = (NetQueue *)(char *)&netsync_q;

    memset(q, 0, 0x10C);
    q->heap = heap_create(netsync_heap_mem, 0x800);
}

/* Exported: queues a packet of `size` bytes (sent in queued mode). */
void netsync_queue_push(void *data, int size)
{
    NetQueue *q = (NetQueue *)(char *)&netsync_q;

    if (sysvar_get_a98() == 0) {
        return;
    }
    if (((q->tail + 1) & 0x1F) == q->head) {
        return;
    }
    q->size[q->tail] = size;
    q->buf[q->tail] = heap_alloc(q->heap, size);
    if (q->buf[q->tail] != 0) {
        memcpy(q->buf[q->tail], data, size);
        q->tail = (q->tail + 1) & 0x1F;
    } else {
        net_fail();
    }
}

unsigned char *local_record_get(void)
{
    return local_record;
}

unsigned char *peer_get(i)
int i;
{
    return peer_records[i];
}

void local_record_set(unsigned char *rec)
{
    memcpy(local_record_get(), rec, 0x100);
    netsync_dirty = 1;
}

void local_record_set0(unsigned char v)
{
    *local_record_get() = v;
    netsync_dirty = 1;
}

void local_record_set1(unsigned char v)
{
    local_record_get()[1] = v;
    netsync_dirty = 1;
}

void local_record_set2(unsigned char v)
{
    local_record_get()[2] = v;
    netsync_dirty = 1;
}

void local_record_set_u32(int v)
{
    *(int *)local_record_get() = v;
    netsync_dirty = 1;
}

int peer_count_get(void)
{
    return net.peer_count;
}

int peer_self_get(void)
{
    return net.self;
}

/* 1 when every player's record has byte 0 == a, byte 1 == b (4: any), byte 2 == c (0xFF: any). */
int peers_all_match(int a, int b, int c)
{
    signed char *r;
    int i;

    for (i = 0; i < peer_count_get(); i++) {
        r = (signed char *)peer_get(i);
        if (r[0] != a) {
            return 0;
        }
        if (b != 4 && r[1] != b) {
            return 0;
        }
        if (c != 0xFF && r[2] != c) {
            return 0;
        }
    }
    return 1;
}

int peer_self_byte0(void)
{
    return ((signed char *)peer_get(peer_self_get()))[0];
}

int peer_self_byte1(void)
{
    return ((signed char *)peer_get(peer_self_get()))[1];
}

int peer_self_byte2(void)
{
    return ((signed char *)peer_get(peer_self_get()))[2];
}

int peer0_byte0(void)
{
    return ((signed char *)peer_get(0))[0];
}

int peer0_byte1(void)
{
    return ((signed char *)peer_get(0))[1];
}

int peer0_byte2(void)
{
    return ((signed char *)peer_get(0))[2];
}

/* The original calls peer_get without an argument (a0 is whatever the caller left), which
   compiles because peer_get has an old-style definition. */
int peer_self_u32(void)
{
    return *(int *)peer_get();
}

void netsync_transfer_setup(unsigned char *dst, unsigned char *src, int size)
{
    NetXfer *x = (NetXfer *)(char *)&netsync_xfer;

    x->size = size;
    x->src = src;
    x->dst = dst;
    x->ready = 0;
}

#ifdef NON_MATCHING /* one addu with its operands swapped */
/* Exported: one step of the chunked transfer; returns 1 when every player has every chunk. */
int netsync_transfer(void)
{
    int pkt[0x40];
    NetXfer *x = (NetXfer *)(char *)&netsync_xfer;
    unsigned char *p;
    int chunk;
    int done;
    int i;

    if (x->ready == 0 && (x->ready = peers_all_match(7, 4, 0xFF)) == 0) {
        p = (unsigned char *)pkt;
        memset(p, 0, 0x100);
        p[0] = 7;
        p[1] = 2;
        p[2] = 0;
        local_record_set(p);
        return 0;
    }
    if (peer_self_byte2() << 7 >= x->size && peer_self_byte1() == 2) {
        return 1;
    }
    if (peer_self_get() == 0) {
        done = peers_all_match(7, 2, peer_self_byte2());
    } else {
        chunk = peer0_byte2();
        if (peer_self_byte2() < chunk) {
            done = 1;
        } else {
            done = 0;
        }
    }
    if (done) {
        p = (unsigned char *)pkt;
        chunk = peer_self_byte2();
        p[0] = 7;
        p[1] = 1;
        p[2] = chunk + 1;
        memcpy(p + 4, x->src + (chunk << 7), 0x80);
        local_record_set(p);
        return 0;
    }
    if (peers_all_match(7, 4, peer_self_byte2()) != 0) {
        if (peer_self_byte1() == 1) {
            chunk = peer_self_byte2();
            if (chunk != 0) {
                for (i = 0; i < peer_count_get(); i++) {
                    memcpy(x->dst + (((chunk - 1) << 7) + x->size * i), peer_get(i) + 4, 0x80);
                }
            }
            local_record_set1(2);
        }
    }
    return 0;
}
#endif

/* Which side the local player is on: 1 for slots 1 and 3, 0 for 0 and 2, 2 when absent. */
int netsync_local_side(void)
{
    int i;
    GameState *g = game_state_get();
    int n;

    if (g->partner_mode == 2) {
        n = 4;
    } else {
        n = 2;
    }
    for (i = 0; i < n; i++) {
        if (g->slot[i].net_id == peer_self_get()) {
            switch (i) {
            case 0:
            case 2:
                return 0;
            case 1:
            case 3:
                return 1;
            }
        }
    }
    return 2;
}

/* net_id of the first human player of side `which`, 4 if none. */
int netsync_side_player(int which)
{
    GameState *g = game_state_get();
    PlayerSlot *s;

    switch (which) {
    case 0:
        s = &g->slot[0];
        if (s->type < 2) {
            return s->net_id;
        }
        if (g->partner_mode == 2) {
            s = &g->slot[2];
            if (s->type < 2) {
                return s->net_id;
            }
        }
        break;
    case 1:
        s = &g->slot[1];
        if (s->type < 2) {
            return s->net_id;
        }
        if (g->partner_mode == 2) {
            s = &g->slot[3];
            if (s->type < 2) {
                return s->net_id;
            }
        }
        break;
    }
    return 4;
}
