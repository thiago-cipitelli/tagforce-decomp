/* libduel_mgr: duel scene (scene 6). Loads libduel_draw / libduel_eng helpers, runs the duel, the
   in-duel menu and the result screen. Module state and the BOOT/gmodule functions it calls that no
   shared header declares. Offsets are the ground truth; field names are partial. */
#ifndef LIBDUEL_MGR_H
#define LIBDUEL_MGR_H

#include "ehsys.h"
#include "duel_setup.h"
#include "menu.h"

typedef struct DmgrGameHdr {           /* view of GameState +0x108.. used by the duel scene */
    unsigned char deck[0x108];
    unsigned short extra[3];           /* +0x108 */
    unsigned short pad10e;
    int mode;                          /* +0x110 duel_setup_mode */
    int side;                          /* +0x114 player side / first player */
    unsigned char kind;                /* +0x118 1 story, 2 tutorial, 5 free duel ... */
    unsigned char kind2;               /* +0x119 2: deck swap before the duel */
    unsigned char partner_mode;        /* +0x11A 1 single, 2 tag duel */
    unsigned char tutorial;            /* +0x11B tutorial id */
    unsigned char bgm_set;             /* +0x11C index into dmgr_bgm_sets */
    unsigned char rematch;             /* +0x11D */
    unsigned char lp_level;            /* +0x11E starting life points */
    unsigned char time_level;          /* +0x11F answer time limit */
    PlayerSlot slot[4];                /* +0x120 */
} DmgrGameHdr;

/* the deck of a PlayerSlot; the original adds the offset as an integer (two addiu, not folded) */
#define SLOT_RECIPE(s) ((RecipeData *)((int)(s) + 0x80))

typedef struct DmgrBgm {               /* sound ids per duel kind */
    unsigned short a[3];
    unsigned short b[3];
} DmgrBgm;

typedef struct DmgrSeq {               /* dmgr +0x15C, zeroed at duel start */
    unsigned short opening;            /* +0x00 the first turn's cut-in is pending */
    unsigned short pending_card;       /* +0x02 card whose activation gets a cut-in */
    unsigned short set_card;           /* +0x04 card just set */
    unsigned short skip_card;          /* +0x06 */
    unsigned short attack_on;          /* +0x08 */
    unsigned short attack_player;      /* +0x0A */
    unsigned short turn_player;        /* +0x0C */
    unsigned short attrs;              /* +0x0E field attributes of the attacker */
    unsigned short act_player;         /* +0x10 */
    unsigned short attr_cutin;         /* +0x12 */
    unsigned short target;             /* +0x14 card id of the attack target */
    unsigned short pad16;
    unsigned short mood;               /* +0x18 0 even, 1 winning, 2 losing */
    unsigned short pad1a;
    unsigned short n_monsters[4];      /* +0x1C per slot */
    unsigned short n_spells[4];        /* +0x24 */
    DmgrBgm bgm;                       /* +0x2C */
    unsigned short cur_bgm;            /* +0x38 */
    unsigned short pad3a;
    unsigned char pad3c[0x48 - 0x3C];
    int bgm_started;                   /* +0x48 */
} DmgrSeq;

typedef struct DmgrEvent {             /* one queued presentation step, 0x10 */
    unsigned short type;
    unsigned short a, b, c, d;
    unsigned short pad[3];
} DmgrEvent;

typedef struct DmgrQueue {             /* dmgr +0x58, ring of 16 events */
    unsigned short head;
    unsigned short tail;
    DmgrEvent ev[16];
} DmgrQueue;

typedef struct DmgrDrawSetup {         /* passed to dd_api_set_param, 0x40 */
    int n;
    int type[4];                       /* +0x04 PlayerSlot.type */
    int type2[4];                      /* +0x14 */
    void *deck[4];                     /* +0x24 */
    int order[2];                      /* +0x34 slot of duel players 0 and 1 */
    int pad3c;
} DmgrDrawSetup;

typedef struct DmgrModArgs {           /* modmgr_load_ex option block */
    int size;
    int a, b, c, d;
} DmgrModArgs;

typedef struct DuelMgr {               /* dmgr 0x0931BCB0, 0x92C bytes zeroed by dmgr_main */
    unsigned char pad00[0x10];
    int frame;                         /* +0x010 draw frames */
    int timer;                         /* +0x014 frames waited for input on the result */
    int pad18;
    int answered;                      /* +0x01C */
    int quit;                          /* +0x020 */
    int pad24;
    int unk28;                         /* +0x028 */
    unsigned short state;              /* +0x02C */
    unsigned short next_state;         /* +0x02E */
    unsigned short sub;                /* +0x030 */
    unsigned short fade;               /* +0x032 */
    unsigned char pad34[0x58 - 0x34];
    DmgrQueue queue;                   /* +0x058 */
    DmgrSeq seq;                       /* +0x15C */
    DmgrModArgs mod_args;              /* +0x1A8 */
    int start_loaded;                  /* +0x1BC start/start_all.ehp read */
    void *start_buf;                   /* +0x1C0 */
    int busy;                          /* +0x1C4 a module is still loading */
    int draw_loaded;                   /* +0x1C8 libduel_draw */
    int mod8_loaded;                   /* +0x1CC module 8 */
    int codecs_loaded;                 /* +0x1D0 duel_modules_load */
    int swap_loaded;                   /* +0x1D4 libdeck_swap */
    int load_step;                     /* +0x1D8 */
    unsigned char cfg_save[8];         /* +0x1DC SysSave +0x5C during free duels */
    unsigned char gs_save[0x740];      /* +0x1E4 game state at scene start */
    void *heap_big;                    /* +0x924 */
    void *heap;                        /* +0x928 */
} DuelMgr;

extern DuelMgr dmgr;

typedef struct DmgrRect {
    short x, y;
    unsigned short w, h;               /* percent scale of the sprite */
} DmgrRect;

typedef struct DmgrJkCard {            /* a janken hand card, 0x1A */
    unsigned char on : 1;
    unsigned char hand : 3;            /* 0 rock, 1 scissors, 2 paper */
    unsigned char pad0 : 4;
    unsigned char pad1;
    DmgrRect from;                     /* +0x02 */
    DmgrRect cur;                      /* +0x0A */
    DmgrRect to;                       /* +0x12 */
} DmgrJkCard;

typedef struct DmgrJkNet {             /* ad hoc state of the start menu */
    unsigned char peer : 4;            /* own peer index */
    unsigned char side : 2;
    unsigned char host : 1;            /* this peer plays the side's first slot */
    unsigned char b7 : 1;
    unsigned char c5_0 : 1;            /* +0x01 */
    unsigned char synced : 1;          /* partner decks received */
    unsigned char c5_2 : 6;
    unsigned char pad[2];
} DmgrJkNet;

typedef struct DmgrJk {                /* start menu (janken for the first turn), 0xE8 on the heap */
    unsigned short first : 2;          /* +0x00 0 janken, 1 chosen by the series, 2 free duel */
    unsigned short state : 8;
    unsigned short pad00 : 6;
    unsigned char sub;                 /* +0x02 */
    unsigned char won : 1;             /* +0x03 won the janken */
    unsigned char b3_1 : 7;
    unsigned char pick[2];             /* +0x04 hand of each side */
    unsigned char b6_0 : 3;            /* +0x06 */
    unsigned char choice : 2;          /* go first (0) or second (1) */
    unsigned char target : 2;          /* choice the cursor stops on */
    unsigned char closed : 1;          /* the menu was closed by the scene */
    unsigned char pad07;
    unsigned short timer;              /* +0x08 */
    unsigned char result : 4;          /* +0x0A 1 win, 2 lose, 3 draw */
    unsigned char pad0a : 4;
    unsigned char pad0b;
    unsigned short anim[2];            /* +0x0C per side */
    unsigned short t[2];               /* +0x10 */
    unsigned short wait[2];            /* +0x14 */
    void *text;                        /* +0x18 */
    DmgrJkCard card[2][3];             /* +0x1C */
    unsigned char bg_on : 1;           /* +0xB8 draw files[2] */
    unsigned char title_on : 1;        /* draw files[3] */
    unsigned char cursor_on : 1;
    unsigned char partner_on : 1;      /* draw the partner panel */
    unsigned char b8_4 : 4;
    unsigned char padb9;
    unsigned short res_anim[2];        /* +0xBA */
    unsigned char padbe[2];
    void *heap;                        /* +0xC0 */
    DmgrJkNet net;                     /* +0xC4 */
    unsigned char *netbuf;             /* +0xC8 */
    void *pack;                        /* +0xCC */
    void *files[6];                    /* +0xD0 */
} DmgrJk;

extern DmgrJk *dmgr_jk;

typedef struct DmgrAnim {              /* a frame counter that runs to len */
    unsigned char on;
    unsigned char pad;
    short cur;
    short len;
} DmgrAnim;

typedef struct DmgrPtEnt {             /* a player shown on the partner panel, 0x10 */
    short kind;                        /* PlayerSlot.type: 0 none, 1 player, character id */
    short type2;                       /* PlayerSlot.type2 */
    short src;                         /* deck source: 0 saved deck, 1 recipe, 2-3 own name */
    short deck;                        /* recipe id or saved deck index + 1 */
    void *name;                        /* +0x08 */
    void *deckname;                    /* +0x0C */
} DmgrPtEnt;

typedef struct DmgrPartner {           /* partner / versus panel, 0x6C on the heap */
    void *heap;
    void *pack;                        /* +0x04 */
    int timer;                         /* +0x08 */
    int state;                         /* +0x0C */
    unsigned char net;                 /* +0x10 */
    unsigned char mode;                /* +0x11 8 versus, 9 tag versus, 10 partner choice ... */
    unsigned char pad12[2];
    DmgrPtEnt ent[4];                  /* +0x14 */
    DmgrAnim anim;                     /* +0x54 */
    unsigned char pad5a[2];
    void *ehf[4];                      /* +0x5C */
} DmgrPartner;

extern DmgrPartner *dmgr_pt;

typedef struct DmgrResItem {           /* an entry of the result text tables */
    unsigned short on : 1;             /* shown on the result screen */
    unsigned short text : 15;          /* line in strTbl.bin */
} DmgrResItem;

typedef struct DmgrResRec {            /* DuelResults +0x3C: what this duel earned */
    int rec[0x28];                     /* +0x00 records */
    int bonus[0x53];                   /* +0xA0 bonus points, [0x52] total */
    unsigned char title[0x76];         /* +0x1EC titles won */
} DmgrResRec;

typedef struct DmgrResTab {            /* a page of the result screen, 0x0C */
    unsigned short scroll;
    unsigned short count;
    unsigned short *ids;               /* +0x04 indices into the text table */
    unsigned short *lines;             /* +0x08 height of each entry in lines */
} DmgrResTab;

typedef struct DmgrRes {               /* result screen, 0x1A4 on libduel_draw's heap */
    unsigned short state;              /* +0x000 */
    unsigned short sub;                /* +0x002 */
    unsigned char tab : 4;             /* +0x004 current page */
    unsigned char b4_hi : 4;
    unsigned char pad05[3];
    int total[4];                      /* +0x008 points per page: bonus, records, titles, money */
    unsigned char bar : 4;             /* +0x018 page shown on the tab bar */
    unsigned char bg : 2;              /* bg animation */
    unsigned char b18_6 : 2;
    unsigned char pad19;
    short result_ehf;                  /* +0x01A win/lose/draw file, <0 none */
    unsigned char result_on : 1;       /* +0x01C */
    unsigned char prev_tab : 4;
    unsigned char slide : 2;           /* direction of the page change */
    unsigned char b1c_7 : 1;
    unsigned char pad1d;
    unsigned short timer;              /* +0x01E */
    DmgrResTab tabs[4];                /* +0x020 */
    DmgrResRec *rec;                   /* +0x050 */
    void *heap;                        /* +0x054 */
    MenuWidget menu;                   /* +0x058 */
    unsigned char menu_kind : 2;       /* +0x178 1 continue, 2 net wait, 3 net lost */
    unsigned char menu_done : 1;
    unsigned char b178 : 5;
    unsigned char pad179;
    unsigned short n_peers;            /* +0x17A */
    unsigned char net : 1;             /* +0x17C */
    unsigned char loading : 1;
    unsigned char b17c : 6;
    unsigned char pad17d[3];
    void *pack;                        /* +0x180 duel/result/result_?.ehp */
    void *files[8];                    /* +0x184 */
} DmgrRes;

extern DmgrRes *dmgr_res;

/* &gs->slot[k] the way the original adds it (base first): an accessor, not the inline expression */
inline PlayerSlot *dmgr_slot(DmgrGameHdr *gs, int k)
{
    return &gs->slot[k];
}

/* BOOT */
void *memset(void *, int, unsigned int);
void *memcpy(void *, const void *, unsigned int);
void *game_state2_get(void);
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
unsigned short scene_stack_peek_arg(unsigned short scene);
void scene_reset_to(unsigned short from, unsigned short scene, unsigned short arg);
void scene_pop_to(unsigned short from, unsigned short arg);
int sysvar_get_a98(void);
void sysvar_set_a8c(int v);
void cfg_set_bits_1_3_4(int v);
void cfg_set_flag(int which, int on);
void gfx_overlay_enable(int on);
void rand_seed_from_rtc(void);
void sceKernelChangeCurrentThreadAttr(int unk, int attr);
int net_get_state(void);
void net_set_state(int s);
int net_adhoc_stop(void);
void netsync_init(void);
void netsync_set_mode(int mode);
int netsync_local_side(void);
void local_record_set0(unsigned char v);
void local_record_set_u32(int v);
int peer_self_get(void);
int peer_self_u32();
void req_flags_clear(void);
int partner_count_get(void);
int partner_voice_id(unsigned int chr, unsigned int variant);
void bgm_fade(int x);
void bgm_cmd3(int x);
void bgm_cmd(int x);
void snd_cmd_fefa(void);
void snd_cmd_fefb(void);
void snd_cmd_fefb_0(void);
int snd_count_bgm(void);
int snd_count_se(void);
void snd_channel_update(int slot);
void snd_mute_off(void);
int duel_modules_load();
int duel_modules_unload(void);
int modmgr_load_by_id(int id);
int modmgr_load_ex(int id, int argsize, void *argp, int *status, void *opt);
int modmgr_update(int id);
int modmgr_none_loading(void);
int modmgr_none_loading_alias(void);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehfile_busy(void);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
void dl_set_cur_list_base(void *p);
void *dl_get_cur_list_base(void);
unsigned int *dl_disable(unsigned int *list, int state);
int sceGuStart(int cid, void *list, int arg);
int sceGuFinish(void);
void gu_set_color_mask(unsigned int color);
void gu_set_param_50(unsigned int depth);
void gu_set_param_4c(unsigned int stencil);
void gu_set_param_494(int flags);
unsigned int font_set_color(unsigned int c);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_apply(unsigned int **list);

#endif
