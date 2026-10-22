// @range 0x08868DC8-0x0886A6EC
// @data 0x088F07AC-0x088F0908
// @rodata 0x088EB050-0x088EB068
// @asm 0x08868DC8-0x08868E40
// @asm 0x08869028-0x088690F8
// @asm 0x0886936C-0x088694F0
/* Ad hoc front end: the state-entry hook, the peer and scan lists shown by the lobby screens,
   the per-frame driver called from main (pad/UMD watchdogs, link watchdog), the functions the
   gmodules call, the netconf / message dialogs and the net module loading. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "snd.h"
#include "net.h"

typedef struct NetScanEntry {          /* 0x40: one joined player as listed by the host */
    int valid;
    int joined;                        /* also in net_scan.picked */
    int index;                         /* 1.. among the picked ones */
    int tag;                           /* hello.ver_id */
    unsigned char name[0x30];
} NetScanEntry;

typedef struct NetScan {               /* 0x08E1FDCC */
    int count;                         /* +0x000 */
    NetScanEntry e[9];                 /* +0x004 */
    unsigned char macs[9][6];          /* +0x244 */
    unsigned char pad27a[2];
    int npicked;                       /* +0x27C */
    unsigned char picked[9][6];        /* +0x280 */
} NetScan;

typedef struct NetPeerInfo {           /* 0x3C, lobby view of a NetPeer */
    int valid;
    unsigned char name[0x30];
    unsigned char ver[5];
    unsigned char pad35;
    unsigned short ver_id;
} NetPeerInfo;

typedef struct NetWatch {
    int seen;
    int miss;
} NetWatch;

typedef struct SceNetAdhocctlPeerInfo {
    void *next;
    char nickname[128];
    unsigned char mac[6];
    unsigned char pad[2];
    unsigned int unk;
    unsigned long long timestamp;
} SceNetAdhocctlPeerInfo;

extern int snd_idle_evf;               /* 0x08E1FD84 */
extern int net_ver_match;              /* 0x08E1FD88 */
extern int net_ver_seen;               /* 0x08E1FD8C */
extern unsigned int net_frame_count;   /* 0x08E1FD90 */
extern int net_hold_fpl;               /* 0x08E1FD94 */
extern int net_umd_count;              /* 0x08E1FD98 */
extern int net_pad_count;              /* 0x08E1FD9C */
extern int net_wait;                   /* 0x08E1FDA0 */
extern int net_alive;                  /* 0x08E1FDA4 */
extern int net_watch_on;               /* 0x08E1FDA8 */
extern NetWatch net_watch_tab[4];      /* 0x08E1FDAC */
extern NetScan net_scan;               /* 0x08E1FDCC */
extern unsigned char net_group[8];     /* 0x08E20084 */
extern SndList snd_idle_list;          /* 0x08C1AFE0 */
extern int snd_sema;                   /* 0x08C1B044 */

SceNetAdhocctlAdhocId net_product = { 0, "ULJM05151" };

int memcmp2(const void *a, const void *b, int n);
void dbg_printf(const char *fmt, ...);
long long sceKernelGetSystemTimeWide(void);
int sceKernelCreateFpl(const char *name, int part, int attr, unsigned int size, unsigned int blocks, void *opt);
int sceKernelDeleteFpl(int fpl);
int sceUmdGetDriveStat(void);
int sceNetAdhocctlGetPeerInfo(unsigned char *mac, int size, SceNetAdhocctlPeerInfo *info);
int sceUtilityNetconfInitStart(void *p);
int sceUtilityNetconfGetStatus(void);
int sceUtilityNetconfUpdate(int n);
int sceUtilityNetconfShutdownStart(void);
int sceUtilityMsgDialogInitStart(void *p);
int sceUtilityMsgDialogGetStatus(void);
int sceUtilityMsgDialogUpdate(int n);
int sceUtilityMsgDialogShutdownStart(void);
int sceUtilityLoadNetModule(int module);
int sceUtilityUnloadNetModule(int module);
void sceGuSync(int a, int b);
int sysvar_get_a98(void);
void sysvar_set_a98(int v);
int cfg_get_language(void);
unsigned short region_pick_value(int flag);
unsigned short *sysmsg_get(int i);
void wcs_to_utf8(char *dst, int size, unsigned short *src);
void *snd_list_next(SndList *l, void *node);
void ptp_close(void);
void net_update(void);
int net_match_request(int i);
int net_match_cancel(int i);
int net_match_reject(int i);
int net_peer_drop(int i);
int netctl_init(void);
void netctl_term(void);
void net_scan_prune(void);
int net_scan_pick_find(unsigned char *mac);
void net_scan_unpick(int i);
void net_watch(void);
int net_load_modules(void);
int net_unload_modules(void);
void net_hold_create(void);
void net_hold_delete(void);
int net_lang(void);
int net_swap_buttons(void);
int net_dialog_message(int msg);
void net_set_state(int s);
int net_get_state(void);
int netconf_update(void);
int net_dialog_update(void);

#ifdef NON_MATCHING /* the original walks the list with the test at the top and a jump back */
/* Sound side: wakes on the event flag and walks the idle list under the sound semaphore. */
void snd_idle_thread(void)
{
    void *p;

    for (;;) {
        if (sceKernelWaitEventFlag(snd_idle_evf, 1, 0x11, 0, 0) != 0) {
            continue;
        }
        sceKernelWaitSema(snd_sema, 1, 0);
        p = 0;
        for (;;) {
            p = snd_list_next(&snd_idle_list, p);
            if (p == 0) {
                break;
            }
        }
        sceKernelSignalSema(snd_sema, 1);
    }
}
#endif

void net_state_enter(int s)
{
    int i;

    switch (s) {
    case 0xB:
        ptp_close();
        break;
    case 0x1F:
        for (i = 0; i < net.peer_count; i++) {
            if (memcmp2(net.mac, net.macs[i], 6) == 0) {
                net.self = i;
            }
        }
        memset(net_watch_tab, 0, 0x20);
        net_watch_on = 1;
        net_alive = net.peer_count;
        break;
    }
    net.state = s;
    net.unk0a0 = 0;
}

/* Counts guests that run our version (host side). */
void net_peer_flag(int i)
{
    if (net.hello.ver[0] == 1) {
        net_ver_seen++;
        if (net.hello.ver_id == net_peers[i].hello.ver_id) {
            net_ver_match++;
        }
    }
}

/* 1 when `mac` belongs to a joined peer. */
int net_peer_joined(unsigned char *mac)
{
    int i;
    NetPeer *p;

    for (i = 0, p = net_peers; i < 9; i++, p++) {
        if (p->state == 5 && memcmp2(mac, p->mac, 6) == 0) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* the original copies i to s2 only after the count tests */
void net_scan_unpick(int i)
{
    if (net_scan.npicked != 0 && i < net_scan.npicked) {
        for (; i < 8; i++) {
            memcpy(net_scan.picked[i], net_scan.picked[i + 1], 6);
        }
        memset(net_scan.picked[8], 0, 6);
        net_scan.npicked--;
    }
}
#endif

int net_scan_pick_find(unsigned char *mac)
{
    int i;

    for (i = 0; i < net_scan.npicked; i++) {
        if (memcmp2(mac, net_scan.picked[i], 6) == 0) {
            return i;
        }
    }
    return -1;
}

void net_scan_prune(void)
{
    int i;

    for (i = 0; i < net_scan.npicked; i++) {
        if (net_peer_joined(net_scan.picked[i]) == 0) {
            net_scan_unpick(i);
        }
    }
}

void net_peer_timeouts(void)
{
    int i = 0;
    NetPeer *p = net_peers;

    do {
        if (p->state == 2) {
            if (p->timeout <= 0) {
                net_peer_drop(i);
            } else {
                p->timeout--;
            }
        }
        i++;
        p++;
    } while (i < 9);
}

/* Host lobby: rebuild net_scan from the joined peers. */
void net_scan_build(void)
{
    NetScanEntry *e;
    int i;
    int picked;
    NetPeer *p;

    net_scan_prune();
    net_scan.count = 0;
    picked = 1;
    for (i = 0, p = net_peers; i < 9; i++, p++) {
        if (p->state == 5) {
            e = &net_scan.e[net_scan.count];
            e->valid = 1;
            if (net_scan_pick_find(p->mac) >= 0) {
                e->joined = 1;
                e->index = picked;
                picked++;
            } else {
                e->joined = 0;
                e->index = 0;
            }
            e->tag = p->hello.ver_id;
            memcpy(e->name, p->hello.name, 0x30);
            memcpy(net_scan.macs[net_scan.count], p->mac, 6);
            net_scan.count++;
        }
    }
}

#ifdef NON_MATCHING /* MAC and watch pointers in swapped registers */
/* Link watchdog while the game runs: every other player must stay visible. */
void net_watch(void)
{
    SceNetAdhocctlPeerInfo info;
    int i;
    NetWatch *w;
    int r;

    if (net.peer_count < 2) {
        goto fail;
    }
    net_alive = 0;
    for (i = 0, w = net_watch_tab; i < net.peer_count; i++, w++) {
        if (i == net.self) {
            net_alive++;
        } else {
            r = sceNetAdhocctlGetPeerInfo(net.macs[i], 0x98, &info);
            if (r == 0x80410716) {
                w->miss++;
                if (w->seen != 0) {
                    if (w->miss >= 150) {
                        goto fail;
                    }
                } else if (w->miss >= 300) {
                    goto fail;
                }
            } else {
                if (r != 0) {
                    goto fail;
                }
                w->seen = 1;
                w->miss = 0;
                if ((unsigned long long)(sceKernelGetSystemTimeWide() - info.timestamp) >= 5000000) {
                    goto fail;
                }
            }
            net_alive++;
        }
    }
    return;
fail:
    if (net_watch_on != 0) {
        net_fail();
    }
}
#endif

/* Called from main every frame while ad hoc is on. */
void net_frame(void)
{
    PadState *pad = pad_get();

    if (sysvar_get_a98() == 0) {
        return;
    }
    net_frame_count++;
    sceGuSync(0, 0);
    switch (net.state) {
    case 5:
    case 8:
        netconf_update();
        break;
    case 10:
    case 0x26:
        net_dialog_update();
        break;
    case 0x24:
    case 0x25:
    case 0x21:
    case 0x22:
    case -1:
        break;
    default:
        if (pad->b.down & 1) {
            if (++net_pad_count >= 90) {
                net_state_enter(0x24);
                net_pad_count = 0;
            }
        } else {
            net_pad_count = 0;
        }
        if (sceUmdGetDriveStat() & 0xD) {
            if (++net_umd_count >= 900) {
                net_state_enter(0x24);
                net_umd_count = 0;
            }
        } else {
            net_umd_count = 0;
        }
        break;
    }
    net_update();
    switch (net.state) {
    case 0xB:
        net_peer_timeouts();
        break;
    case 0x1E:
        net_scan_build();
        break;
    case 0x15:
        net_wait++;
        switch (net_peers[net.peer_sel].state) {
        case 0:
            if (net_wait++ >= 150) {
                net_state_enter(0x11);
            }
            break;
        case 5:
            net_wait = 0;
            break;
        }
        break;
    case 0x1F:
        net_watch();
        break;
    }
}

int net_get_param(unsigned char *mac);

int net_get_param_export(unsigned char *mac)
{
    return net_get_param(mac);
}

/* Exported. */
void net_get_mac_str(int i, char *out)
{
    strcpy(out, (char *)net.macs[i]);
}

void net_get_own_mac_str(char *out)
{
    strcpy(out, (char *)net.mac);
}

int net_get_alive(void)
{
    return net_alive;
}

void net_set_watch(int on)
{
    net_watch_on = on;
}

/* Exported: load the net modules and start sceNetAdhocctl; 1 on success. */
int net_adhoc_start(void)
{
    if (sysvar_get_a98() != 0) {
        return 0;
    }
    net_hold_delete();
    if (net_load_modules() < 0) {
        return 0;
    }
    memset(net_group, 0, 8);
    memset(&net, 0, 0x110);
    memset(net_peers, 0, 0x33E4);
    net_state_enter(0);
    if (netctl_init() < 0) {
        return 0;
    }
    sysvar_set_a98(1);
    net_umd_count = 0;
    net_pad_count = 0;
    return 1;
}

/* Exported: stop ad hoc and unload the modules. */
int net_adhoc_stop(void)
{
    int r;

    if (sysvar_get_a98() == 0) {
        return 0;
    }
    sysvar_set_a98(0);
    netctl_term();
    r = net_unload_modules();
    net_hold_create();
    return r >= 0;
}

int net_get_state(void)
{
    return net.state;
}

void net_set_state(int s)
{
    net_state_enter(s);
}

/* Exported: hosts in hello state (guest lobby). */
void net_get_hosts(int *out)
{
    NetPeerInfo *e;
    int i;
    NetPeer *p;
    int *q;

    memset(out, 0, 0x220);
    if (net_get_state() != 0xB) {
        return;
    }
    for (i = 0, p = net_peers, q = out; i < 9; i++, p++, q += 15) {
        if (p->state == 2) {
            e = (NetPeerInfo *)(q + 1);
            e->valid = 1;
            memcpy(e->name, p->hello.name, 0x30);
            e->ver[0] = p->hello.ver[0];
            e->ver[1] = p->hello.ver[1];
            e->ver[2] = p->hello.ver[2];
            e->ver[3] = p->hello.ver[3];
            e->ver[4] = p->hello.ver[4];
            e->ver_id = p->hello.ver_id;
            (*out)++;
        }
    }
}

/* Exported: become host with our lobby info. */
int net_host(NetPeerInfo *me)
{
    if (net_get_state() != 0xB) {
        return 0;
    }
    memcpy(net.hello.name, me->name, 0x30);
    net.hello.ver[0] = me->ver[0];
    net.hello.ver[1] = me->ver[1];
    net.hello.ver[2] = me->ver[2];
    net.hello.ver[3] = me->ver[3];
    net.hello.ver[4] = me->ver[4];
    net.hello.ver_id = me->ver_id;
    memset(&net_scan, 0, 0x2B8);
    net_ver_match = 0;
    net_ver_seen = 0;
    net_set_state(0x1D);
    return 1;
}

/* Exported: ask host i to let us in. */
int net_join(int i, NetScanEntry *me)
{
    if (net_get_state() != 0xB) {
        return 0;
    }
    if (net_peers[i].state != 2) {
        return 0;
    }
    memset(net.hello.name, 0, 0x80);
    memcpy(net.hello.name, me->name, 0x30);
    net_match_request(i);
    net_state_enter(0xD);
    return 1;
}

int net_join_tagged(int i, unsigned short *me)
{
    if (net_get_state() != 0xB) {
        return 0;
    }
    if (net_peers[i].state != 2) {
        return 0;
    }
    memset(net.hello.name, 0, 0x80);
    memcpy(net.hello.name, me + 2, 0x30);
    net.hello.ver_id = *me;
    net_match_request(i);
    net_state_enter(0xD);
    return 1;
}

/* Exported (host lobby): the scan list. */
void net_get_scan(int *out)
{
    NetScanEntry *d;
    int i;

    memset(out, 0, 0x244);
    if (net_get_state() != 0x1E) {
        return;
    }
    for (i = 0; i < net_scan.count; i++) {
        d = (NetScanEntry *)(out + 1);
        d->valid = net_scan.e[i].valid;
        d->joined = net_scan.e[i].joined;
        d->index = net_scan.e[i].index;
        d->tag = net_scan.e[i].tag;
        memcpy(d->name, net_scan.e[i].name, 0x30);
        out += 0x10;
    }
}

/* Exported (host lobby): toggle player i in the picked list. */
void net_toggle_pick(int i)
{
    unsigned char *mac;
    int r;
    int n;

    if (net_get_state() != 0x1E || net_scan.count <= i) {
        return;
    }
    mac = net_scan.macs[i];
    r = net_scan_pick_find(mac);
    if (r >= 0) {
        net_scan_unpick(r);
    } else if (net_scan.npicked < 8) {
        n = net_scan.npicked;
        net_scan.npicked = n + 1;
        memcpy(net_scan.picked[n], mac, 6);
    }
}

/* Exported (host): drop joined players that were not picked and start the PTP handshake. */
void net_start_game(void)
{
    int i;
    NetPeer *p;

    net_scan_prune();
    for (i = 0, p = net_peers; i < 9; i++, p++) {
        if (p->state == 5 && net_scan_pick_find(p->mac) < 0) {
            net_match_reject(i);
        }
    }
    net.retry = 0;
    net_set_state(0x16);
}

/* Exported (guest): which players the host reports, as a scan list. */
void net_get_host_players(int *out)
{
    int *e;
    int i;
    NetPeer *p;
    int j;
    int *q;

    memset(out, 0, 0x244);
    if (net_get_state() != 0x15) {
        return;
    }
    for (i = 0, p = net_peers; i < 9; i++, p++) {
        if (memcmp2(net.host_mac, p->mac, 6) == 0) {
            for (j = 0, q = out; j < 9; j++, q += 0x10) {
                e = q + 1;
                if (j < p->hello.guests) {
                    (*out)++;
                    *e = 1;
                }
            }
        }
    }
}

/* Exported (guest): our host's lobby info. */
void net_get_host_info(NetPeerInfo *out)
{
    memset(out, 0, 0x3C);
    out->valid = 1;
    memcpy(out->name, net_peers[net.peer_sel].hello.name, 0x30);
    out->ver[0] = net_peers[net.peer_sel].hello.ver[0];
    out->ver[1] = net_peers[net.peer_sel].hello.ver[1];
    out->ver[2] = net_peers[net.peer_sel].hello.ver[2];
    out->ver[3] = net_peers[net.peer_sel].hello.ver[3];
    out->ver[4] = net_peers[net.peer_sel].hello.ver[4];
}

int net_leave(void)
{
    net_match_cancel(net.peer_sel);
    return 1;
}

void net_fail(void)
{
    if (net_dialog_message(0x16) < 0) {
        net_state_enter(0x21);
    } else {
        net_state_enter(10);
    }
}

void net_finish(void)
{
    if (net_dialog_message(0x1A) < 0) {
        net_state_enter(0x26);
    } else {
        net_state_enter(10);
    }
}

/* Reserve the memory the net modules need while ad hoc is off. */
void net_hold_create(void)
{
    int opt[2];

    opt[0] = 8;
    opt[1] = 0x40;
    net_hold_fpl = sceKernelCreateFpl("ygadh_hold", 2, 0, 0x5F000, 1, opt);
}

void net_hold_delete(void)
{
    sceKernelDeleteFpl(net_hold_fpl);
}

void net_get_ver_counts(int *seen, int *match)
{
    *seen = net_ver_seen;
    *match = net_ver_match;
}

/* sceUtility language from the game language. */
int net_lang(void)
{
    switch (cfg_get_language()) {
    case 0:
        return 0;
    case 2:
        return 4;
    case 3:
        return 2;
    case 4:
        return 5;
    case 5:
        return 3;
    case 1:
    default:
        return 1;
    }
}

int net_swap_buttons(void)
{
    return region_pick_value(0) != 0x2000;
}

typedef struct NetconfAdhoc {
    char group[8];
    int timeout;
} NetconfAdhoc;

typedef struct UtilBase {              /* pspUtilityDialogCommon */
    int size;
    int language;
    int buttonSwap;
    int graphicsThread;
    int accessThread;
    int fontThread;
    int soundThread;
    int result;
    int reserved[4];
} UtilBase;

typedef struct NetconfData {           /* 0x40 */
    UtilBase base;
    int action;                        /* +0x30 */
    NetconfAdhoc *adhoc;               /* +0x34 */
    int unk38[2];
} NetconfData;

typedef struct MsgDialog {             /* 0x244 */
    UtilBase base;
    int unk30;
    int mode;                          /* +0x34 0 error code, 1 text */
    int error;                         /* +0x38 */
    char text[0x200];                  /* +0x3C */
    int options;
    int button;
} MsgDialog;

#define NETCONF ((NetconfData *)net.netconf)
#define CONF ((NetconfAdhoc *)net.adhoc_conf)
#define DIALOG ((MsgDialog *)net.dialog)

int netconf_start(int action, char *group)
{
    memset(NETCONF, 0, 0x40);
    NETCONF->base.size = 0x40;
    NETCONF->base.buttonSwap = net_swap_buttons();
    NETCONF->base.language = net_lang();
    NETCONF->base.graphicsThread = 0x11;
    NETCONF->base.accessThread = 0x13;
    NETCONF->base.fontThread = 0x12;
    NETCONF->base.soundThread = 0x10;
    NETCONF->action = action;
    memset(CONF, 0, 0xC);
    if (group != 0) {
        memcpy(CONF->group, group, 8);
    } else {
        strcpy(CONF->group, "YuGiOhVs");
    }
    CONF->timeout = 0x14;
    NETCONF->adhoc = CONF;
    return sceUtilityNetconfInitStart(NETCONF);
}

int netconf_update(void)
{
    int r = 0;

    switch (sceUtilityNetconfGetStatus()) {
    case 2:
        r = sceUtilityNetconfUpdate(2);
        break;
    }
    return r;
}

int netconf_connect(char *group)
{
    return netconf_start(2, group);
}

int netconf_create(char *group)
{
    return netconf_start(4, group);
}

int netconf_join(char *group)
{
    return netconf_start(5, group);
}

int netconf_poll(int *result)
{
    int st = sceUtilityNetconfGetStatus();

    if (st >= 0) {
        switch (st) {
        case 3:
            st = sceUtilityNetconfShutdownStart();
            break;
        case 0:
            switch (NETCONF->base.result) {
            case 0:
                *result = 1;
                break;
            case 2:
                *result = -1;
                net_fail();
                break;
            default:
                *result = -1;
                net_state_enter(-1);
                break;
            }
            break;
        }
    }
    return st;
}

int net_dialog_error(int code)
{
    memset(DIALOG, 0, 0x244);
    DIALOG->base.size = 0x244;
    DIALOG->base.buttonSwap = net_swap_buttons();
    DIALOG->base.language = net_lang();
    DIALOG->base.graphicsThread = 0x11;
    DIALOG->base.accessThread = 0x13;
    DIALOG->base.fontThread = 0x12;
    DIALOG->base.soundThread = 0x10;
    DIALOG->error = code;
    DIALOG->mode = 0;
    return sceUtilityMsgDialogInitStart(DIALOG);
}

int net_dialog_update(void)
{
    int r = 0;

    switch (sceUtilityMsgDialogGetStatus()) {
    case 2:
        r = sceUtilityMsgDialogUpdate(2);
        break;
    }
    return r;
}

void net_dialog_poll(int *result)
{
    int st = sceUtilityMsgDialogGetStatus();

    if (st >= 0) {
        switch (st) {
        case 3:
            sceUtilityMsgDialogShutdownStart();
            break;
        case 0:
            if (DIALOG->base.result == 0) {
                *result = 1;
            } else {
                *result = -1;
            }
            break;
        }
    }
}

int net_dialog_message(int msg)
{
    memset(DIALOG, 0, 0x244);
    DIALOG->base.size = 0x244;
    DIALOG->base.buttonSwap = net_swap_buttons();
    DIALOG->base.language = net_lang();
    DIALOG->base.graphicsThread = 0x11;
    DIALOG->base.accessThread = 0x13;
    DIALOG->base.fontThread = 0x12;
    DIALOG->base.soundThread = 0x10;
    DIALOG->error = 0;
    DIALOG->mode = 1;
    wcs_to_utf8(DIALOG->text, 0x40, sysmsg_get(msg));
    return sceUtilityMsgDialogInitStart(DIALOG);
}

int net_load_modules(void)
{
    int r = sceUtilityLoadNetModule(1);

    if (r < 0) {
        dbg_printf("sceUtilityLoadNetModule(SCE_UTILITY_NET_MODULE_COMMON) failed. ret = 0x%x\n", r);
    } else {
        r = sceUtilityLoadNetModule(2);
        if (r < 0) {
            dbg_printf("sceUtilityLoadNetModule(SCE_UTILITY_NET_MODULE_ADHOC) failed. ret = 0x%x\n", r);
        } else {
            return r;
        }
    }
    net_unload_modules();
    return r;
}

int net_unload_modules(void)
{
    int r = sceUtilityUnloadNetModule(2);

    if (r < 0) {
        dbg_printf("sceUtilityUnloadNetModule(SCE_UTILITY_NET_MODULE_ADHOC) failed. ret = 0x%x\n", r);
    }
    r = sceUtilityUnloadNetModule(1);
    if (r < 0) {
        dbg_printf("sceUtilityUnloadNetModule(SCE_UTILITY_NET_MODULE_COMMON) failed. ret = 0x%x\n", r);
    }
    return r;
}
