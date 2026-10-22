// @range 0x0886A6EC-0x0886B654
// @data 0x088F0908-0x088F0B74
// @rodata 0x088EB068-0x088EB0DC
/* Ad hoc matching: sceNetAdhocMatching context (host or guest), the matching callback that
   queues each event into a ring of Mbx packets, and the per-frame event handlers that keep the
   peer table (net_peers) up to date. */
#include "psp.h"
#include "libc.h"
#include "net.h"

int net_mbx = -1;

void dbg_printf(const char *fmt, ...);
int memcmp2(const void *a, const void *b, int n);
unsigned int rand_next(void);
int sceKernelCreateMbx(const char *name, unsigned int attr, void *opt);
int sceKernelDeleteMbx(int mbx);
int sceKernelSendMbx(int mbx, void *msg);
int sceKernelPollMbx(int mbx, void **msg);
int sceNetEtherNtostr(unsigned char *mac, char *out);
int sceNetAdhocctlGetNameByAddr(unsigned char *mac, void *name);
int sceNetAdhocctlGetParameter(void *param);
int sceNetAdhocMatchingInit(int memsize);
int sceNetAdhocMatchingTerm(void);
int sceNetAdhocMatchingCreate(int mode, int maxpeers, unsigned short port, int bufsize, unsigned int hellodelay, unsigned int pingdelay, int initcount, unsigned int msgdelay, void (*cb)(int, int, unsigned char *, int, void *));
int sceNetAdhocMatchingDelete(int id);
int sceNetAdhocMatchingStart(int id, int evthpri, int evthstack, int inthpri, int inthstack, int optlen, void *opt);
int sceNetAdhocMatchingStop(int id);
int sceNetAdhocMatchingSelectTarget(int id, unsigned char *mac, int optlen, void *opt);
int sceNetAdhocMatchingCancelTargetWithOpt(int id, unsigned char *mac, int optlen, void *opt);
int sceNetAdhocMatchingSetHelloOpt(int id, int optlen, void *opt);
void net_peer_flag(int i);
void net_match_stop(void);
void net_match_term(void);
int net_match_select(unsigned char *mac);
int net_peer_find(unsigned char *mac);
void net_event_guest(NetEvent *ev);
void net_event_host(NetEvent *ev);
int net_random_name(unsigned char *out);
int rand_range(int lo, int hi);
void net_match_handler(int id, int event, unsigned char *mac, int optlen, void *opt);

extern unsigned char net_group[8];     /* 0x08E20084 */

/* Guest side: matching in client mode. */
int net_match_start_client(void)
{
    int r;

    net.hello.role = 0;
    r = sceNetAdhocMatchingCreate(2, net.hello.max_peers, 1, 0x400, 2000000, 2000000, 3, 500000, net_match_handler);
    if (r < 0) {
        dbg_printf("sceNetAdhocMatchingCreate() failed. ret = 0x%x\n", r);
    } else {
        net.match_id = r;
        r = sceNetAdhocMatchingStart(r, 0x36, 0x1C00, 0x36, 0, 0x94, &net.hello);
        if (r < 0) {
            dbg_printf("sceNetAdhocMatchingStart() failed. ret = 0x%x\n", r);
        } else {
            return 0;
        }
    }
    net_match_stop();
    return r;
}

/* Host side: random group name, matching in host mode. */
int net_match_start_guest(void)
{
    int r;

    net_match_stop();
    net.hello.role = 1;
    net_random_name(net_group);
    r = sceNetAdhocMatchingCreate(1, net.hello.max_peers, 1, 0x400, 2000000, 2000000, 3, 500000, net_match_handler);
    if (r < 0) {
        dbg_printf("sceNetAdhocMatchingCreate() failed. ret = 0x%x\n", r);
    } else {
        net.match_id = r;
        net.hello.guests = 1;
        r = sceNetAdhocMatchingStart(r, 0x36, 0x1C00, 0x36, 0, 0x94, &net.hello);
        if (r < 0) {
            dbg_printf("sceNetAdhocMatchingStart() failed. ret = 0x%x\n", r);
        } else {
            return 0;
        }
    }
    net_match_stop();
    return r;
}

void net_match_stop(void)
{
    int r;

    if (net.match_id >= 0) {
        r = sceNetAdhocMatchingStop(net.match_id);
        if (r < 0) {
            dbg_printf("sceNetAdhocMatchingStop() failed. ret = 0x%x\n", r);
        }
        r = sceNetAdhocMatchingDelete(net.match_id);
        if (r < 0) {
            dbg_printf("sceNetAdhocMatchingDelete() failed. ret = 0x%x\n", r);
        }
        net.match_id = 0;
    }
}

int net_match_init(void)
{
    int r;

    net.peers_seen = 0;
    r = sceKernelCreateMbx("Mevent", 0, 0);
    if (r < 0) {
        dbg_printf("sceKernelCreaeMbx() failed. ret = 0x%x\n", r);
    } else {
        net_mbx = r;
        memset(net_peers, 0, 0x5C4);
        r = sceNetAdhocMatchingInit(0x8000);
        if (r < 0) {
            dbg_printf("sceNetAdhocMatchingInit() failed. ret = 0x%x\n", r);
        } else {
            r = net_match_start_client();
            if (r >= 0) {
                return 0;
            }
        }
    }
    net_match_term();
    return r;
}

void net_match_term(void)
{
    void *msg;
    int r;

    net_match_stop();
    r = sceNetAdhocMatchingTerm();
    if (r < 0) {
        dbg_printf("sceNetAdhocMatchingTerm() failed. ret = 0x%x\n", r);
    }
    if (net_mbx >= 0) {
        while (sceKernelPollMbx(net_mbx, &msg) >= 0) {
        }
        sceKernelDeleteMbx(net_mbx);
        net_mbx = -1;
    }
    net.peers_seen = 0;
}

/* Guest: ask to join host `i`. */
int net_match_request(int i)
{
    int r = sceNetAdhocMatchingSelectTarget(net.match_id, net_peers[i].mac, 0x94, &net.hello);

    if (r < 0) {
        dbg_printf("sceNetAdhocMatchingSelectTarget() failed. ret = 0x%x\n", r);
    } else if (net_peers[i].state == 2) {
        net_peers[i].state = 4;
        net.peer_sel = i;
    }
    return 0;
}

int net_match_cancel(int i)
{
    int opt = 0;
    int r = sceNetAdhocMatchingCancelTargetWithOpt(net.match_id, net_peers[i].mac, 4, &opt);

    if (r < 0) {
        dbg_printf("sceNetAdhocMatchingCancelTargetWithOpt() failed. ret = 0x%x\n", r);
    } else {
        net_peers[i].state = 2;
        net.peer_sel = -1;
    }
    net_state_enter(0xB);
    return 0;
}

int net_match_reject(int i)
{
    int opt = 0;
    int r = sceNetAdhocMatchingCancelTargetWithOpt(net.match_id, net_peers[i].mac, 4, &opt);

    if (r < 0) {
        dbg_printf("sceNetAdhocMatchingCancelTargetWithOpt() failed. ret = 0x%x\n", r);
    } else {
        net_peers[i].state = 0;
    }
    return 0;
}

/* Matching callback (matching thread): queue the event for net_match_events. */
void net_match_handler(int id, int event, unsigned char *mac, int optlen, void *opt)
{
    static char *names[] = { "NONE", "HELLO", "REQUEST", "LEAVE", "DENY", "CANCEL", "ACCEPT",
                             "ESTABLISHED", "TIMEOUT", "ERROR", "BYE" };
    char str[20];
    unsigned char *slot;

    sceNetEtherNtostr(mac, str);
    dbg_printf("%s from %s\n", names[event], str);
    slot = net_events + net.ev_pos;
    if (memcmp2(slot, "\0\0\0\0", 4) != 0) {
        return;
    }
    memcpy(slot, "mbox", 4);
    memset(slot + 4, 0, 0xAC);
    net.ev_pos += 0xB0;
    if (net.ev_pos > 0xFF50) {
        net.ev_pos = 0;
    }
    ((NetEvent *)(slot + 4))->event = event;
    memcpy(((NetEvent *)(slot + 4))->mac, mac, 6);
    ((NetEvent *)(slot + 4))->optlen = optlen;
    switch (event) {
    case 1:
        if (optlen == 0x94) {
            memcpy(((NetEvent *)(slot + 4))->opt, opt, optlen);
        }
        break;
    case 6:
        if (optlen == 8) {
            memcpy(((NetEvent *)(slot + 4))->opt, opt, optlen);
        }
        break;
    case 3:
    case 4:
    case 5:
        if (optlen == 4) {
            memcpy(((NetEvent *)(slot + 4))->opt, opt, optlen);
        }
        break;
    case 2:
        if (optlen == 0x94) {
            memcpy(((NetEvent *)(slot + 4))->opt, opt, optlen);
        }
        break;
    }
    sceKernelSendMbx(net_mbx, slot + 4);
}

/* Per frame: handle every queued matching event. */
void net_match_events(void)
{
    NetEvent *ev;

    for (;;) {
        if (sceKernelPollMbx(net_mbx, (void **)&ev) < 0) {
            break;
        }
        if (net.hello.role == 1) {
            net_event_host(ev);
        } else {
            net_event_guest(ev);
        }
        memset((char *)ev - 4, 0, 0xB0);
    }
}

/* Removes peer i, shifting the rest down. */
int net_peer_drop(int i)
{
    NetPeer *p = &net_peers[i];

    memset(p, 0, 0xA4);
    p->state = 0;
    for (; i < 8; i++) {
        memcpy(&net_peers[i], &net_peers[i + 1], 0xA4);
    }
    memset(&net_peers[8], 0, 0xA4);
    net_peers[8].state = 0;
    net.peers_seen--;
    return 0;
}

int net_peer_find(unsigned char *mac)
{
    NetPeer *p;
    int i;

    for (i = 0, p = net_peers; i < 9; i++, p++) {
        if (memcmp2(p->mac, mac, 6) == 0) {
            return i;
        }
        if (p->state == 0) {
            p = &net_peers[i];
            p->state = 1;
            memcpy(p->mac, mac, 6);
            sceNetAdhocctlGetNameByAddr(mac, &p->hello);
            net.peers_seen++;
            return i;
        }
    }
    return -1;
}

#ifdef NON_MATCHING /* the original clears i before finishing the address of net_peers */
/* Index of the peer with `mac`, adding it when there is room; -1 when full. */
#endif

void net_event_guest(NetEvent *ev)
{
    NetPeer *p;
    int i;

    if (ev->event == 1 && ev->optlen == 0x94 && ((NetHello *)ev->opt)->role != 1) {
        return;
    }
    if ((i = net_peer_find(ev->mac)) >= 0) {
        switch (ev->event) {
        case 1:
            p = &net_peers[i];
            if (p->state != 5) {
                p->state = 2;
                memcpy(p->mac, ev->mac, 6);
                if (ev->optlen == 0x94) {
                    memcpy(&p->hello, ev->opt, 0x94);
                }
                net_peers[i].timeout = 0xF0;
            }
            break;
        case 4:
            if (net.state == 0xD && i == net.peer_sel) {
                net_peer_drop(i);
                net_state_enter(0x11);
            }
            break;
        case 6:
            if (ev->optlen == 8) {
                memcpy(net_group, ev->opt, 8);
            }
            break;
        case 7:
            memcpy(net.host_mac, ev->mac, 6);
            p = &net_peers[i];
            net.target = &p->hello;
            p->state = 5;
            net_state_enter(0x14);
            break;
        case 10:
            if (net.state - 0x14U < 2 && i == net.peer_sel) {
                net_peer_drop(i);
                net_state_enter(0x11);
                break;
            }
            if (net_peers[i].state == 5) {
                break;
            }
        case 8:
            net_peer_drop(i);
            if (i == net.peer_sel) {
                switch (net.state) {
                case 0xD:
                case 0x14:
                case 0x15:
                    net_state_enter(0x11);
                    break;
                }
            }
            break;
        case 2:
        case 3:
        case 5:
        case 9:
            break;
        }
    }
}

void net_event_host(NetEvent *ev)
{
    int opt1;
    int opt2;
    NetPeer *p;
    int i;

    if (net.state - 0x16U <= 3) {
        return;
    }
    if ((i = net_peer_find(ev->mac)) < 0) {
        if (ev->event == 2) {
            opt1 = 2;
            sceNetAdhocMatchingCancelTargetWithOpt(net.match_id, ev->mac, 4, &opt1);
        }
        return;
    }
    switch (ev->event) {
    case 2:
        if (net.state != 0x1E) {
            opt2 = 2;
            sceNetAdhocMatchingCancelTargetWithOpt(net.match_id, net_peers[i].mac, 4, &opt2);
        } else {
            p = &net_peers[i];
            memcpy(p->mac, ev->mac, 6);
            memcpy(&p->hello, ev->opt, 0x94);
            net_match_select(ev->mac);
            p->state = 3;
        }
        break;
    case 7:
        net.target = &net.hello;
        net_peers[i].state = 5;
        net_peer_flag(i);
        net.hello.guests++;
        sceNetAdhocMatchingSetHelloOpt(net.match_id, 0x94, &net.hello);
        break;
    case 3:
    case 5:
        if (net_peers[i].state == 3) {
            net_peer_drop(i);
            break;
        }
    case 8:
    case 10:
        if (net_peers[i].state != 6) {
            if (net_peers[i].state == 5) {
                net.hello.guests--;
                sceNetAdhocMatchingSetHelloOpt(net.match_id, 0x94, &net.hello);
            }
            net_peer_drop(i);
        }
        break;
    case 0:
    case 1:
    case 4:
    case 6:
    case 9:
        break;
    }
}

int rand_range(int lo, int hi)
{
    unsigned int r;

    if (hi == lo) {
        return lo;
    }
    r = rand_next() / (0x7FFFFFFFU / (hi - lo + 1) + 1);
    return r + lo;
}

#ifdef NON_MATCHING /* the original adds lo straight into v0; this keeps it in s0 and moves */
/* Uniform integer in [lo, hi]. */
#endif

int net_get_param(unsigned char *mac)
{
    char str[8];
    unsigned char param[0x8C];
    int r = sceNetAdhocctlGetParameter(param);

    if (r >= 0) {
        sceNetEtherNtostr((unsigned char *)str, (char *)mac);
    }
    return r;
}

/* 1..8 random capital letters. */
int net_random_name(unsigned char *out)
{
    unsigned int n = rand_range(1, 8);
    unsigned int i;

    memset(out, 0, 8);
    for (i = 0; i < n; i++) {
        out[i] = rand_range('A', 'Z');
    }
    return 0;
}
