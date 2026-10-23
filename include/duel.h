/* libduel_eng: duel state (see src/libduel_eng/). Names of the zone arrays are provisional. */
#ifndef DUEL_H
#define DUEL_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct DuelCard {              /* 4: a card reference */
    unsigned int id : 13;
    unsigned int owner : 1;          /* also bit 0 of the instance id */
    unsigned int b14 : 1;
    unsigned int b15 : 1;
    unsigned int b16 : 1;
    unsigned int b17 : 1;
    unsigned int b18 : 1;
    unsigned int b19 : 1;
    unsigned int b20 : 1;
    unsigned int b21 : 1;
    unsigned int uid_hi : 8;         /* instance id bits 1-8 */
    unsigned int f30 : 2;
} DuelCard;

/* instance id of a card reference */

typedef struct DuelSlot {              /* 0x14: a field zone */
    DuelCard card;
    unsigned short unk4;               /* +0x04 */
    unsigned short pos;                /* +0x06 */
    unsigned short faceup;             /* +0x08 */
    unsigned short effects;            /* +0x0A head of its effect list */
    unsigned int link;                 /* +0x0C */
    unsigned int flags;                /* +0x10 */
} DuelSlot;

/* face up and not negated (bits 1 and 5 of the flags) */
#define CARD_UID(c) ((c).owner + ((c).uid_hi << 1))
#define SLOT_ACTIVE(s) ((s)->faceup & ~((s)->flags >> 5 & 1) & ~((s)->flags >> 1 & 1))

#define EFF_PLAYER(e) ((e)->src & 0xFF)
#define EFF_ZONE(e) ((e)->src >> 8 & 0xFF)

typedef struct DuelPlayer {            /* 0xFAC, two from 0x08915E5C */
    int lp;
    int unk4;                          /* +0x004 */
    int unk8;                          /* +0x008 */
    unsigned int n_hand;               /* +0x00C zone 0xB */
    int n_d;                           /* +0x010 zone 0xD */
    unsigned int n_grave;              /* +0x014 zone 0xE */
    int n_18;                          /* +0x018 */
    unsigned int n_f;                  /* +0x01C zone 0xF and marks[] */
    int unk20[4];                      /* +0x020 */
    DuelSlot field[11];                /* +0x030 0-4 monsters, 5-9 spells/traps, 10 field spell */
    unsigned short unk10c[5];          /* +0x10C */
    unsigned short effects;            /* +0x116 head of the player's effect list */
    int unk118;                        /* +0x118 */
    unsigned int flags;                /* +0x11C */
    DuelCard hand[160];                /* +0x120 zone 0xB */
    DuelCard zone_d[160];              /* +0x3A0 zone 0xD */
    DuelCard zone_c[60];               /* +0x620 zone 0xC */
    DuelCard grave[220];               /* +0x710 zone 0xE */
    DuelCard zone_f[220];              /* +0xA80 zone 0xF */
    unsigned short marks[222];         /* +0xDF0 parallel to zone_f: kind | player << 8 | zone << 12 */
} DuelPlayer;

typedef struct DuelEffect {            /* 8: entry of the effect lists */
    unsigned short src;                /* source card: player | zone << 8 */
    unsigned char kind;                /* +0x02 low nibble: kind */
    unsigned char pad3;
    unsigned short arg;                /* +0x04 */
    unsigned short next;               /* +0x06 */
} DuelEffect;

typedef struct DuelEffectBits {        /* view of DuelEffect with the kind byte as nibbles */
    unsigned short src;
    unsigned char kind : 4;
    unsigned char kind_hi : 4;
    unsigned char pad3;
    unsigned short arg;
    unsigned short next;
} DuelEffectBits;

typedef struct DuelStats {             /* duel_field_stats */
    int id;
    int attr;                          /* +0x04 type */
    int unk8;
    unsigned int mask;                 /* +0x0C attribute bits */
    int level;                         /* +0x10 */
    int atk;                           /* +0x14 */
    int def;                           /* +0x18 */
    int batk;                          /* +0x1C base ATK */
    int bdef;                          /* +0x20 base DEF */
    unsigned int imm;                  /* +0x24 */
    unsigned int imm2;                 /* +0x28 */
} DuelStats;

typedef struct DuelSrc {
    unsigned short uid : 9;
    unsigned short player : 1;
    unsigned short zone : 4;
    unsigned short b14 : 2;
} DuelSrc;

typedef struct DuelAct {               /* 0x18: card being activated or resolved */
    unsigned short id;
    unsigned short player : 1;         /* +0x02 */
    unsigned short zone : 5;
    unsigned short kind : 6;
    unsigned short b3_4 : 2;
    unsigned short opp : 1;            /* acts for the opponent of `player` */
    unsigned short b15 : 1;
    unsigned short b4_0 : 1;           /* +0x04 */
    unsigned short b4_1 : 1;
    unsigned short b4_2 : 1;
    unsigned short b4_3 : 1;
    unsigned short b4_4 : 1;
    unsigned short b4_5 : 1;
    unsigned short uid : 9;
    unsigned short b4_15 : 1;
    unsigned char n_targets;           /* +0x06 */
    unsigned char n7;                  /* +0x07 */
    unsigned short w[6];               /* +0x08 */
    union {
        struct {
            unsigned char b14;
            unsigned char b15_0 : 1;   /* +0x15 */
            unsigned char b15_1 : 1;
            unsigned char b15_2 : 1;
            unsigned char b15_3 : 1;
            unsigned char b15_4 : 4;
        };
        DuelSrc src;                   /* +0x14 card that caused the action */
        int idx;                       /* +0x14 index into duel.list for some handlers */
        struct DuelActX {              /* +0x14 as a word */
            unsigned int s_uid : 9;
            unsigned int s_player : 1;
            unsigned int s_zone : 4;
            unsigned int s_b14 : 1;
            unsigned int s_b15 : 1;
            unsigned int s_b16 : 1;
            unsigned int s_b17 : 1;
            unsigned int s_b18 : 1;
            unsigned int s_zone2 : 4;      /* bits 19-22 */
            unsigned int s_b23 : 9;
        } x;
        struct DuelActY {              /* +0x14 by bytes */
            unsigned char y14;
            unsigned char y15_0 : 1;
            unsigned char y15_1 : 1;
            unsigned char y15_2 : 1;
            unsigned char y15_3 : 1;
            unsigned char y15_4 : 1;
            unsigned char y15_5 : 1;
            unsigned char y15_6 : 1;
            unsigned char y15_7 : 1;
            unsigned char y16_0 : 1;
            unsigned char y16_1 : 1;
            unsigned char y16_2 : 1;
            unsigned char y16_3 : 1;
            unsigned char y16_4 : 1;
            unsigned char y16_5 : 1;
            unsigned char y16_6 : 1;
            unsigned char y16_7 : 1;
        } y;
    };
} DuelAct;

typedef struct DuelChain {             /* 0x08914350 */
    union {
        unsigned char pad0[0x300];
        DuelAct pending[32];           /* actions queued by the computer player (duel_w_089147E0) */
    };
    DuelAct acts[16];                  /* +0x300 */
    int unk480;
    int len;                           /* +0x484 */
    unsigned char pad488[0x494 - 0x488];
    unsigned int uids_head;            /* +0x494 ring of card uids: uids[head & 0x7F] = count, then the uids */
    unsigned short uids[128];          /* +0x498 */
    unsigned char pad598[0x5D0 - 0x598];
    unsigned int n_kinds;              /* +0x5D0 */
    int unk5d4;
    unsigned char kinds[16];           /* +0x5D8 kind of each pending entry [?] */
    unsigned char pad5e8[0x5F8 - 0x5E8];
    struct DuelActX srcs[16];          /* +0x5F8 source card of each pending entry [?] */
} DuelChain;

typedef struct DuelSpecAct {           /* 0x38 */
    int unk0;
    int unk4;
    int unk8;                          /* +0x08 */
    int uid;                           /* +0x0C */
    int id;                            /* +0x10 */
    int pad14[8];
    int unk34;                         /* +0x34 */
} DuelSpecAct;

typedef struct DuelSpecRec {           /* 0x14: last target answered by each special card [?] */
    int w0;
    int a[2];                          /* player, zone */
    int b[2];
} DuelSpecRec;

typedef struct DuelSpecial {           /* 0x08913F10, 0x130: two cards held outside the zones */
    int player[2];
    int unk8;                          /* +0x08 */
    int padc[4];
    int zone[2];                       /* +0x1C */
    int unk24;                         /* +0x24 */
    int uid;                           /* +0x28 card uid (owner + uid_hi * 2) */
    DuelSpecAct act[2];                /* +0x2C by player */
    DuelSpecRec recs[2];               /* +0x9C */
    DuelSlot slot[2];                  /* +0xC4 */
    int padec[17];
} DuelSpecial;

typedef struct Duel {                  /* 0x08915E5C, 0x3840 */
    DuelPlayer players[2];
    unsigned int mode;                 /* +0x1F58 bit 0: the special cards count as on the field */
    unsigned int zone_off;             /* +0x1F5C zone bits (player * 16 + zone) of disabled zones */
    int unk1f60;
    int cur_card;                      /* +0x1F64 */
    int unk1f68;
    DuelCard list[512];                /* +0x1F6C every card of the duel by uid */
    DuelEffect effects[513];           /* +0x276C, entry 0 unused */
    unsigned int turn_player;          /* +0x3774 [?] */
    unsigned int turn;                 /* +0x3778 [?] */
    int unk377c;
    int phase;                         /* +0x3780 [?] */
    int unk3784;
    int unk3788;
    int unk378c;
    int unk3790;
    int msg_done;                      /* +0x3794 */
    int unk3798;                       /* +0x3798 */
    int unk379c;
    int unk37a0;                       /* +0x37A0 */
    int unk37a4;                       /* +0x37A4 */
    unsigned char pad37a8[0x37D4 - 0x37A8];
    int unk37d4;                       /* +0x37D4 */
    unsigned char pad37d8[0x3822 - 0x37D8];
    unsigned short unk3822[3];         /* +0x3822 card ids [?] */
    unsigned char pad3828[0x3830 - 0x3828];
    unsigned short unk3830;            /* +0x3830 count of unk3832 (also duel_w_0891968C) */
    unsigned short unk3832[7];         /* +0x3832 */
} Duel;

extern Duel duel;
extern DuelChain duel_chain;
extern DuelSpecial duel_special;       /* 0x08913F10 */
extern DuelEffect duel_effects[];      /* 0x089185C8 */

#endif
