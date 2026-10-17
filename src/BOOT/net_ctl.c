// @range 0x0886B654-0x0886C904
// @data 0x088F0B74-0x088F0C94
// @rodata 0x088EB0DC-0x088EB110
// @asm 0x0886C59C-0x0886C790
/* Ad hoc control: sceNetAdhocctl init and handler, scan/connect/disconnect steps, the per-frame
   dispatcher net_update, and the PTP handshake in which the host sends the list of player MACs
   to every guest. */
#include "psp.h"
#include "libc.h"
#include "net.h"

extern unsigned int netctl_flags;      /* 0x08E309F8 handler events, bit per event */
extern int netctl_error;               /* 0x08E309FC */
extern unsigned char ptp_buf[];        /* 0x08E30A00 int count, then 6-byte MACs */
extern char net_product[];             /* 0x088F07AC SceNetAdhocctlAdhocId "ULJM05151" */
extern unsigned char net_group[8];     /* 0x08E20084 group name / matching opt data */

int netctl_handler_id = -1;
int ptp_listen_id = -1;
int ptp_conn_id = -1;

void dbg_printf(const char *fmt, ...);
int memcmp2(const void *a, const void *b, int n);
long long sceKernelGetSystemTimeWide(void);
int sceNetInit(int poolsize, int calloutprio, int calloutstack, int netintrprio, int netintrstack);
int sceNetTerm(void);
int sceNetAdhocInit(void);
int sceNetAdhocTerm(void);
int sceNetAdhocctlInit(int stacksize, int priority, void *product);
int sceNetAdhocctlTerm(void);
int sceNetAdhocctlAddHandler(void (*handler)(int, int, void *), void *arg);
int sceNetAdhocctlDelHandler(int id);
int sceNetAdhocctlScan(void);
int sceNetAdhocctlDisconnect(void);
int sceNetAdhocctlGetState(int *state);
int sceNetAdhocMatchingSelectTarget(int id, unsigned char *mac, int optlen, void *opt);
int sceNetAdhocPtpListen(unsigned char *mac, unsigned short port, unsigned int bufsize, unsigned int delay, int count, int queue, int unk);
int sceNetAdhocPtpOpen(unsigned char *src, unsigned short sport, unsigned char *dst, unsigned short dport, unsigned int bufsize, unsigned int delay, int count, int unk);
int sceNetAdhocPtpAccept(int id, unsigned char *mac, unsigned short *port, unsigned int timeout, int nonblock);
int sceNetAdhocPtpConnect(int id, unsigned int timeout, int nonblock);
int sceNetAdhocPtpSend(int id, void *data, int *len, unsigned int timeout, int nonblock);
int sceNetAdhocPtpRecv(int id, void *data, int *len, unsigned int timeout, int nonblock);
int sceNetAdhocPtpFlush(int id, unsigned int timeout, int nonblock);
int sceNetAdhocPtpClose(int id, int unk);
int sceWlanGetEtherAddr(unsigned char *mac);

void netctl_term(void);
int netconf_connect(void *arg);
int netconf_create(void *arg);
int netconf_join(void *arg);
int netconf_poll(int *result);
int net_dialog_error(int code);
void net_dialog_poll(int *result);
int net_match_init(void);
void net_match_term(void);
int net_match_start_guest(void);
void net_match_events(void);
void net_peer_drop(int i);
void net_finish(void);
int netsync_open(void);
int netsync_close(void);
int netsync_exchange(void);
void ptp_close(void);
void ptp_listen(void);
void ptp_open(void);
int ptp_connect(void);
int ptp_send_peers(void);
int ptp_flush(void);
int ptp_accept(void);
int ptp_update(void);

static int (*const netconf_start[3])(void *) = { netconf_connect, netconf_create, netconf_join };

void netctl_reset(void)
{
    netctl_flags = 0;
    netctl_error = 0;
}

void netctl_handler(int event, int error, void *arg)
{
    if (event == 0) {
        netctl_error = error;
        netctl_flags |= 1;
    } else if (event == 1) {
        netctl_error = 0;
        netctl_flags |= 2;
    } else if (event == 2) {
        netctl_error = 0;
        netctl_flags |= 4;
    } else if (event == 3) {
        netctl_error = 0;
        netctl_flags |= 8;
    } else if (event == 4) {
        netctl_error = 0;
        netctl_flags |= 0x10;
    }
}

int netctl_take_error(void)
{
    if (netctl_flags & 1) {
        netctl_flags &= ~1;
        return netctl_error;
    }
    return 0;
}

int netctl_init(void)
{
    int r;

    r = sceNetInit(0x20000, 0x28, 0, 0x28, 0);
    if (r >= 0 && (r = sceNetAdhocInit()) >= 0 && (r = sceNetAdhocctlInit(0x1C00, 0x30, net_product)) >= 0) {
        netctl_reset();
        r = sceNetAdhocctlAddHandler(netctl_handler, 0);
        if (r >= 0) {
            netctl_handler_id = r;
            memset(&net.hello, 0, 0x94);
            net.hello.max_peers = 9;
            return 0;
        }
    }
    netctl_term();
    return -1;
}

void netctl_term(void)
{
    if (netctl_handler_id >= 0) {
        sceNetAdhocctlDelHandler(netctl_handler_id);
        netctl_handler_id = -1;
    }
    sceNetAdhocctlTerm();
    sceNetAdhocTerm();
    sceNetTerm();
}

int netctl_scan(void)
{
    int r;

    netctl_flags = 0;
    r = sceNetAdhocctlScan();
    if (r < 0) {
        net_state_enter(0);
    } else {
        switch (net.state) {
        case 1:
            net_state_enter(2);
            break;
        }
    }
    return r;
}

int netctl_connect(void)
{
    int r = netconf_start[0](0);

    if (r < 0) {
        net_state_enter(-1);
    } else {
        net_state_enter(5);
    }
    return r;
}

int netctl_disconnect(void)
{
    ptp_close();
    net_match_term();
    netsync_close();
    if (sceNetAdhocctlDisconnect() >= 0) {
        switch (net.state) {
        case 0x12:
            net_state_enter(7);
            break;
        case 0x13:
            net_state_enter(9);
            break;
        case 0x21:
            net_state_enter(0x22);
            break;
        case 0x24:
            net_state_enter(0x25);
            break;
        }
    }
    return 0;
}

int netctl_wait_scan(void)
{
    int r = 0;
    int st;

    if ((netctl_flags & 1) || (netctl_flags & 8)) {
        r = sceNetAdhocctlGetState(&st);
        if (r >= 0 && st != 2) {
            r = netctl_take_error();
            if (r < 0) {
                r = net_dialog_error(r);
                if (r >= 0) {
                    net_state_enter(10);
                }
            } else {
                net_state_enter(3);
            }
        }
    }
    return r;
}

int netctl_wait_netconf(void)
{
    unsigned char mac[8];
    int res = 0;
    int r = 0;

    netconf_poll(&res);
    if (res > 0) {
        switch (net.state) {
        case 5:
            sceWlanGetEtherAddr(mac);
            memcpy(net.mac, mac, 6);
            r = net_match_init();
            if (r < 0) {
                net_state_enter(0x21);
            } else {
                net_state_enter(0xB);
            }
            break;
        case 8:
            r = netsync_open();
            if (r < 0) {
                net_state_enter(0x21);
            } else {
                net_state_enter(0x1F);
            }
            break;
        }
    } else if (res < 0) {
        net.failed = 1;
    }
    return r;
}

int netctl_wait_disconnect(void)
{
    int r;
    int st;

    r = sceNetAdhocctlGetState(&st);
    if (r >= 0 && st == 0) {
        switch (net.state) {
        case 9:
            net_state_enter(0);
            break;
        case 0x22:
            net_state_enter(-1);
            break;
        case 0x25:
            net_finish();
            break;
        case 7:
            if (net.hello.role == 1) {
                r = netconf_start[1](net_group);
            } else {
                r = netconf_start[2](net_group);
            }
            if (r < 0) {
                net_state_enter(0);
            } else {
                net_state_enter(8);
            }
            break;
        }
    }
    return r;
}

int netctl_check_error(void)
{
    int r = netctl_take_error();

    if (r != 0) {
        r = net_dialog_error(r);
        if (r >= 0) {
            net_state_enter(10);
        }
    }
    return r;
}

int netctl_wait_dialog(void)
{
    int res = 0;

    net_dialog_poll(&res);
    if (res != 0) {
        if (net.state == 0x26) {
            net_state_enter(-1);
        } else {
            net_state_enter(0x21);
        }
        net.failed = 1;
    }
    return 0;
}

/* Exported: one frame of the ad hoc state machine. */
int net_update(void)
{
    switch (net.state) {
    case 1:
        netctl_scan();
        break;
    case 2:
        netctl_wait_scan();
        break;
    case 4:
        netctl_connect();
        break;
    case 5:
    case 8:
        netctl_wait_netconf();
        break;
    case 0x13:
    case 0x12:
    case 0x21:
    case 0x24:
        netctl_disconnect();
        break;
    case 7:
    case 9:
    case 0x22:
    case 0x25:
        netctl_wait_disconnect();
        break;
    case 0x1F:
        netsync_exchange();
        break;
    case 10:
    case 0x26:
        netctl_wait_dialog();
        break;
    default:
        ptp_update();
        break;
    }
    if (net.state >= 11) {
        net_match_events();
        netctl_check_error();
    }
    return 0;
}

int net_match_select(unsigned char *mac)
{
    int r = sceNetAdhocMatchingSelectTarget(net.match_id, mac, 8, net_group);

    if (r < 0) {
        dbg_printf("sceNetAdhocMatchingSelectTarget() failed. ret = 0x%x\n", r);
    }
    return r;
}

void ptp_listen(void)
{
    if (ptp_listen_id < 0) {
        ptp_listen_id = sceNetAdhocPtpListen(net.mac, 0x42, 0x4000, 200000, 200000, 1, 0);
        if (ptp_listen_id < 0) {
            dbg_printf("sceNetAdhocPtpListen() failed. ret = 0x%x\n", ptp_listen_id);
            net_fail();
        } else {
            net_state_enter(0x15);
        }
    }
}

void ptp_open(void)
{
    int i;

    if (ptp_listen_id < 0) {
        for (i = 0; i < 9; i++) {
            if (net_peers[i].state == 5) {
                break;
            }
        }
        if (i == 9) {
            net_state_enter(0x12);
        } else {
            net.ptp_peer = i;
            ptp_listen_id = sceNetAdhocPtpOpen(net.mac, 0, net_peers[i].mac, 0x42, 0x4000, 200000, 200000, 0);
            if (ptp_listen_id < 0) {
                dbg_printf("sceNetAdhocPtpOpen() failed. ret = 0x%x\n", ptp_listen_id);
                net_fail();
            } else {
                net_state_enter(0x17);
            }
            net.t_open = sceKernelGetSystemTimeWide();
        }
    }
}

void ptp_close(void)
{
    if (ptp_conn_id >= 0) {
        sceNetAdhocPtpClose(ptp_conn_id, 0);
    }
    ptp_conn_id = -1;
    if (ptp_listen_id >= 0) {
        sceNetAdhocPtpClose(ptp_listen_id, 0);
    }
    ptp_listen_id = -1;
}

int ptp_connect(void)
{
    int r;

    if (net.state != 0x17) {
        return 0;
    }
    if ((unsigned long long)(sceKernelGetSystemTimeWide() - net.t_open) > 5000000) {
        net_peer_drop(net.ptp_peer);
        ptp_close();
        net_state_enter(0x16);
        return 0;
    }
    r = sceNetAdhocPtpConnect(ptp_listen_id, 0, 1);
    if (r == 0x80410709) {
        return 0;
    }
    if (r == 0x80410718) {
        ptp_close();
        net_state_enter(0x1C);
        net.t_retry = sceKernelGetSystemTimeWide();
        return 0;
    }
    if (r < 0) {
        dbg_printf("sceNetAdhocPtpConnect() failed. ret = 0x%x\n", r);
    }
    if (r < 0) {
        net_fail();
    } else {
        net_state_enter(0x18);
    }
    return r;
}

int ptp_send_peers(void)
{
    int len = 0;
    int n;
    NetPeer *p;
    int r;
    int i;

    if (net.state != 0x18) {
        return 0;
    }
    memset(net.macs, 0, 0x36);
    memcpy(ptp_buf + 4, net.mac, 6);
    memcpy(net.macs[0], net.mac, 6);
    n = 1;
    for (i = 0; i < 9; i++) {
        if (net_peers[i].state != 5 && net_peers[i].state != 6) {
        } else {
            memcpy(ptp_buf + 4 + n * 6, net_peers[i].mac, 6);
            memcpy(net.macs[n], net_peers[i].mac, 6);
            n++;
        }
    }
    if (n < 1) {
        return 0;
    }
    memcpy(ptp_buf, &n, 4);
    net.peer_count = n;
    len = n * 6 + 4;
    r = sceNetAdhocPtpSend(ptp_listen_id, ptp_buf, &len, 0, 1);
    if (r == 0x80410709) {
        return 0;
    }
    if (r < 0) {
        dbg_printf("sceNetAdhocPtpSend() failed. ret = 0x%x\n", r);
    } else {
        net.t_open = sceKernelGetSystemTimeWide();
    }
    if (r < 0) {
        net_fail();
    } else {
        net_state_enter(0x19);
    }
    return r;
}

int ptp_flush(void)
{
    int r;

    if (net.state != 0x19) {
        return 0;
    }
    if ((unsigned long long)(sceKernelGetSystemTimeWide() - net.t_open) > 5000000) {
        net_peer_drop(net.ptp_peer);
        ptp_close();
        net_state_enter(0x16);
        return 0;
    } else {
        r = sceNetAdhocPtpFlush(ptp_listen_id, 0, 1);
        if (r == 0x80410709) {
            return 0;
        }
        if (r < 0) {
            dbg_printf("sceNetAdhocPtpFlush() failed. ret = 0x%x\n", r);
        } else {
            net_peers[net.ptp_peer].state = 6;
        }
        ptp_close();
        if (r < 0) {
            net_fail();
        } else {
            net_state_enter(0x16);
        }
    }
    return r;
}

#ifdef NON_MATCHING /* the original recomputes ptp_buf + 4 + i * 6 in the copy loop instead of stepping a pointer */
int ptp_accept(void)
{
    int n;
    int len;
    unsigned char mac[8];
    int r;
    int i;

    if (net.state != 0x15) {
        return 0;
    }
    if (ptp_conn_id < 0) {
        r = sceNetAdhocPtpAccept(ptp_listen_id, mac, 0, 0, 1);
        if (r == 0x80410709) {
            return 0;
        }
        if (r < 0) {
            goto done;
        }
        if (memcmp2(net_peers[net.peer_sel].mac, mac, 6) != 0) {
            return 0;
        }
        ptp_conn_id = r;
    }
    len = 0x4000;
    r = sceNetAdhocPtpRecv(ptp_conn_id, ptp_buf, &len, 0, 1);
    if (r == 0x80410709) {
        return 0;
    }
    if (r < 0) {
        goto done;
    }
    if (len < 4) {
        return 0;
    }
    memcpy(&n, ptp_buf, 4);
    r = n;
    if (r <= 0) {
        return 0;
    }
    memset(net.macs, 0, 0x36);
    for (i = 0; i < n; i++) {
        memcpy(net.macs[i], ptp_buf + 4 + i * 6, 6);
    }
    net.peer_count = n;
done:
    if (r < 0) {
        net_fail();
    } else {
        net_state_enter(0x12);
    }
    return r;
}
#endif

int ptp_update(void)
{
    switch (net.state) {
    case 0x1D:
        if (net_match_start_guest() < 0) {
            net_state_enter(0x13);
        } else {
            net_state_enter(0x1E);
        }
        break;
    case 0x14:
        ptp_listen();
        break;
    case 0x15:
        ptp_accept();
        break;
    case 0x16:
        ptp_open();
        break;
    case 0x17:
        ptp_connect();
        break;
    case 0x18:
        ptp_send_peers();
        break;
    case 0x19:
        ptp_flush();
        break;
    case 0x1A:
    case 0x1B:
        break;
    case 0x1C:
        if (++net.retry >= 0x1E) {
            net_state_enter(0x23);
        } else if ((unsigned long long)(sceKernelGetSystemTimeWide() - net.t_retry) > 200000) {
            net_state_enter(0x16);
            net.t_open = sceKernelGetSystemTimeWide();
        }
        break;
    }
    return 0;
}
