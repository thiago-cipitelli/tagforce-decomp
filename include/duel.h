/* libduel_eng: duel state (see src/libduel_eng/). Names of the zone arrays are provisional. */
#ifndef DUEL_H
#define DUEL_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct DuelCard {              /* 4: a card reference */
    unsigned short id : 13;
    unsigned short owner : 1;          /* also bit 0 of the instance id */
    unsigned short b14 : 1;
    unsigned short b15 : 1;
    unsigned short lo5 : 5;
    unsigned short b21 : 1;
    unsigned short uid_hi : 8;         /* instance id bits 1-8 */
    unsigned short f30 : 2;
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
    };
} DuelAct;

typedef struct DuelChain {             /* 0x08914350 */
    unsigned char pad0[0x300];
    DuelAct acts[16];                  /* +0x300 */
    int unk480;
    int len;                           /* +0x484 */
} DuelChain;

typedef struct DuelSpecial {           /* 0x08913F10, 0x130: two cards held outside the zones */
    int player[2];
    int pad8[5];
    int zone[2];                       /* +0x1C */
    int pad24;
    int uid;                           /* +0x28 card uid (owner + uid_hi * 2) */
    int pad2c[38];
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
    DuelCard list[64];                 /* +0x1F6C */
    unsigned char pad206c[0x276C - 0x206C];
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
    unsigned char pad3798[0x3830 - 0x3798];
    unsigned short unk3830;
    unsigned short unk3832;
    unsigned char pad3834[0x3840 - 0x3834];
} Duel;

extern Duel duel;
extern DuelChain duel_chain;
extern DuelSpecial duel_special;       /* 0x08913F10 */
extern DuelEffect duel_effects[];      /* 0x089185C8 */

#endif
