/* Ad hoc multiplayer (BOOT 0x08868DC8..0x0886C904): sceNetAdhocctl session, matching and PTP
   handshake, plus the system dialogs it shows. One state machine in net.state, stepped by
   net_update every frame. */
#ifndef NET_H
#define NET_H

typedef struct NetHello {              /* 0x94, our matching hello/opt data (net.hello) */
    unsigned char name[0x80];          /* +0x00 */
    int role;                          /* +0x80 1 host */
    int guests;                        /* +0x84 joined guests (host) */
    int max_peers;                     /* +0x88 */
    unsigned char ver[6];              /* +0x8C */
    unsigned short ver_id;             /* +0x92 */
} NetHello;

typedef struct NetPeer {               /* 0xA4, 9 at 0x08E2008C (matching targets) */
    int state;                         /* +0x00 1 seen, 2 hello, 3 requested, 4 selected, 5 joined, 6 linked */
    int timeout;                       /* +0x04 */
    unsigned char mac[6];              /* +0x08 */
    unsigned char pad0e[2];
    NetHello hello;                    /* +0x10 */
} NetPeer;

typedef struct NetEvent {              /* 0xAC, queued to the event Mbx */
    void *link;                        /* +0x00 SceKernelMsgPacket */
    int unk04;
    int event;                         /* +0x08 PSP_ADHOC_MATCHING_EVENT_* */
    unsigned char mac[6];              /* +0x0C */
    unsigned char pad12[2];
    int optlen;                        /* +0x14 */
    unsigned char opt[0x94];           /* +0x18 */
} NetEvent;

typedef struct NetSession {            /* 0x08E20650 */
    int state;                         /* +0x000 */
    NetHello hello;                    /* +0x004 cleared by netctl_init */
    NetHello *target;                  /* +0x098 hello of the joined host */
    int peer_sel;                      /* +0x09C */
    int unk0a0;                        /* +0x0A0 */
    int failed;                        /* +0x0A4 */
    unsigned char mac[6];              /* +0x0A8 own ether address */
    unsigned char pad0ae[2];
    int match_id;                      /* +0x0B0 */
    int peer_count;                    /* +0x0B4 */
    int self;                          /* +0x0B8 */
    int retry;                         /* +0x0BC */
    unsigned char macs[9][6];          /* +0x0C0 per player */
    unsigned char host_mac[6];         /* +0x0F6 */
    int ptp_peer;                      /* +0x0FC peer index of the PTP link */
    long long t_open;                  /* +0x100 */
    long long t_retry;                 /* +0x108 */
    unsigned char dialog[0x244];       /* +0x110 SceUtilityMsgDialogParams */
    unsigned char adhoc_conf[0xC];     /* +0x354 netconf adhoc param: group name, timeout */
    unsigned char netconf[0x40];       /* +0x360 SceUtilityNetconfData */
    int peers_seen;                    /* +0x3A0 */
    unsigned int ev_pos;               /* +0x3A4 write offset in net_events */
} NetSession;

typedef struct SceNetAdhocctlAdhocId {
    int type;
    char product[9];
    unsigned char pad[3];
} SceNetAdhocctlAdhocId;

extern SceNetAdhocctlAdhocId net_product; /* 0x088F07AC */
extern NetPeer net_peers[9];           /* 0x08E2008C */
extern NetSession net;                 /* 0x08E20650 */
extern unsigned char net_events[0x10000]; /* 0x08E209F8 ring of 0xB0-byte slots: "mbox" + NetEvent */

void net_state_enter(int s);
void net_fail(void);

#endif
