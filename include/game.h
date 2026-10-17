/* BOOT's game-level globals: one 0x107A4-byte block at 0x08A66A80 zeroed by loading_screen_init, holding
   runtime flags, the save state ("sysstate", 0x08A66B98) and the per-session game state. Field names
   are partial; offsets are the ground truth. */
#ifndef GAME_H
#define GAME_H

typedef struct DuelOppRec {            /* per-opponent counters, 8 bytes */
    unsigned short count[3];           /* kind 1..3 (999 cap) */
    unsigned short pad;
} DuelOppRec;

typedef struct DuelRecords {           /* SysSave +0xEA4 */
    int total[3];                      /* +0x00 kind 1..3 (999999 cap) */
    int streak;                        /* +0x0C kind-1 streak */
    int best_streak;                   /* +0x10 */
    int streak2;                       /* +0x14 kind-2 streak */
    int best_streak2;                  /* +0x18 */
    DuelOppRec opp[160];               /* +0x1C */
} DuelRecords;                         /* 0x51C */

typedef struct SaveExtra {             /* SysSave +0x13C0 (DuelRecords + 0x51C) */
    unsigned char unk0[0x94];
    int unk94;                         /* newgame: 0x92784 */
    int unk98;                         /* newgame: 999 */
} SaveExtra;

typedef struct InvRec {                /* per-card inventory record, 4 bytes, SysSave +0x1558 */
    unsigned char flag0 : 1;
    unsigned char state : 2;           /* 0 none, 1 seen, 2 owned */
    unsigned char flag3 : 1;
    unsigned char hi : 3;
    unsigned char bit7 : 1;
    unsigned char count : 7;           /* 0..99 */
    unsigned char flag7b : 1;
    unsigned char limit : 2;           /* 0 forbidden .. 3 unlimited */
    unsigned char pad2 : 6;
    unsigned char pad3;
} InvRec;

typedef struct RecipeData {            /* recipe_load output / current deck, 0x108 */
    unsigned int n_main, n_extra, n_side;
    unsigned short main_ids[80];
    unsigned short extra_ids[15];
    unsigned short side_ids[31];
} RecipeData;

typedef struct SysSave {               /* 0x08A66B98, 0xFCA4 bytes used (+4 pad in GameGlobals) */
    int version;                       /* +0x00 0x10002 */
    unsigned int flags;                /* +0x04 bit flags (sysflag_test/set) */
    unsigned int play_counter;         /* +0x08 frames played, saturating */
    unsigned char psid[16];            /* +0x0C sceOpenPSIDGetOpenPSID; checksum covers from here */
    unsigned short name[24];           /* +0x1C player name, UTF-16, '$' doubled */
    int day;                           /* +0x4C 0..99 */
    int day_progress;                  /* +0x50 */
    int money;                         /* +0x54 0..999999999 */
    int money_total;                   /* +0x58 */
    unsigned char unk5c[0xEA4 - 0x5C];
    DuelRecords records;               /* +0xEA4 */
    SaveExtra extra;                   /* +0x13C0 */
    unsigned char unk145c[0x1558 - 0x145C];
    InvRec inv[2600];                  /* +0x1558 indexed by card index */
    unsigned char event_seen[0x40];    /* +0x3DF8 stat_event_record */
    unsigned char flag_3e38[0x4820 - 0x3E38];
    RecipeData deck;                   /* +0x4820 current deck */
    unsigned short partner_count;      /* +0x4928 < 4 */
    unsigned short partner_ids[3];     /* +0x492A */
    unsigned short partner_state[3];   /* +0x4930 1..3 */
    unsigned short deck_name[32];      /* +0x4936 */
    unsigned char blob[0x200];         /* +0x4976 */
    unsigned short player_name[64];    /* +0x4B76 */
    unsigned char unk4bf6[0x8FF0 - 0x4BF6];
    unsigned char flagrec[48][0x2A];   /* +0x8FF0 sysflag2 records */
    unsigned char tail[0xFCA8 - 0x97D0];
} SysSave;

typedef struct GameGlobals {           /* 0x08A66A80 */
    unsigned int sysflags;             /* +0x000 */
    unsigned int rand_state;           /* +0x004 MSVC LCG */
    int unk08;
    int var_a8c;                       /* +0x00C set to 1 after config changes */
    int var_a90;                       /* +0x010 nonzero inhibits the soft reset */
    int var_a94;
    int var_a98;                       /* +0x018 gates net start */
    int overlay_enabled;               /* +0x01C loading overlay drawn each frame */
    unsigned int save_checksum;        /* +0x020 sysstate_checksum */
    void *loading_ehf;                 /* +0x024 "loading.ehf" from pack_loading */
    unsigned char info[0x8C];          /* +0x028 sys_info_get() */
    unsigned int frame_counter;        /* +0x0B4 */
    unsigned int req_flags[24];        /* +0x0B8 */
    SysSave save;                      /* +0x118 */
    unsigned char game_state[0x740];   /* +0xFDC0 slots of 0x188 */
    unsigned char game_state2[0x2A0];  /* +0x10500 */
    void *heap;                        /* +0x107A0 heap_create(game_heap_mem, 0x4000) */
} GameGlobals;

extern GameGlobals g_game;
extern unsigned char game_heap_mem[0x4000];   /* 0x08A62A80 */
extern unsigned char dl_heap[];               /* 0x08906A80, 0x15C000 bytes */

/* EHP archives embedded in BOOT's .data */
extern unsigned char pack_chr_name[];   /* chr_name_?.bin */
extern unsigned char pack_sysgfx[];     /* icons, fonts */
extern unsigned char pack_recipes[];    /* RCP*.ydc */
extern unsigned char pack_loading[];    /* loading.ehf + gims */
extern unsigned char pack_sysmsg[];     /* sysmsg_?.bin */

SysSave *sysstate_get(void);
RecipeData *deck_get(void);
int deck_count(int which);
unsigned short deck_get_card(int which, int i);
int partner_count_get(void);
unsigned short partner_id_get(int i);
int partner_state_get(int i);
int inv_get_count(unsigned short id);
void inv_set_limit(unsigned short id, unsigned short v);
void sysstate_set_player_name(const unsigned short *name);
extern const unsigned short card_limit_table[0xA6];   /* 0x0887544C: (card id, limit) pairs */
DuelRecords *duel_records_get(void);
SaveExtra *sysstate2_get_ptr_51c(void);
int sysstate_checksum(void);
unsigned int rand_next(void);
void srand(unsigned int seed);
unsigned int rand(void);               /* MSVC LCG on g_game.rand_state */
int day_threshold(int day);
int day_advance(int delta);
void money_add(int delta);
void duel_record_add(int kind, int opp1, int opp2);

#endif
