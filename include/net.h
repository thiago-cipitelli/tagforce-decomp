/* Ad hoc multiplayer (BOOT 0x08868DC8..0x0886C904): sceNetAdhocctl session, matching and PTP
   handshake, plus the system dialogs it shows. One state machine in net.state, stepped by
   net_update every frame. */
#ifndef NET_H
#define NET_H

typedef struct NetPeer {               /* 0xA4, 9 at 0x08E2008C (matching targets) */
    int state;                         /* +0x00 2 hello pending, 5 joined, 6 linked */
    int timeout;                       /* +0x04 */
    unsigned char mac[6];              /* +0x08 */
    unsigned char pad0e[2];
    unsigned char info[0x30];          /* +0x10 hello/opt data */
    unsigned char pad40[0xA2 - 0x40];
    unsigned short tag;                /* +0xA2 */
} NetPeer;

typedef struct NetSession {            /* 0x08E20650 */
    int state;                         /* +0x000 */
    unsigned char pad004[0x84 - 0x04]; /* +0x004 (+0x004..+0x097 cleared by netctl_init) */
    int role;                          /* +0x084 1 host */
    int unk088;                        /* +0x088 */
    int max_peers;                     /* +0x08C */
    unsigned char ver[6];              /* +0x090 */
    unsigned short ver_id;             /* +0x096 */
    int unk098;                        /* +0x098 */
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
    unsigned char pad0f6[0xFC - 0xF6];
    int ptp_peer;                      /* +0x0FC peer index of the PTP link */
    long long t_open;                  /* +0x100 */
    long long t_retry;                 /* +0x108 */
} NetSession;

extern NetPeer net_peers[9];           /* 0x08E2008C */
extern NetSession net;                 /* 0x08E20650 */

void net_state_enter(int s);
void net_fail(void);

#endif
