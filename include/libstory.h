/* libstory: the story scene (scene 2): the story script interpreter, the story state (characters,
   decks, flags), the events that drive libfield, libshop and the sub-screen modules (charalist,
   cardalbum, recipe_viewer, ...). Module state and the BOOT (libehsys) calls no shared header
   declares. Offsets are the ground truth; field names are partial.
   Each block below belongs to the address range in its marker; keep every block compilable. */
#ifndef LIBSTORY_H
#define LIBSTORY_H

#include "ehsys.h"
#include "font.h"
#include "menu.h"
#include "game.h"
#include "libc.h"
#include "psp.h"

/* BOOT calls not declared by the shared headers */
void gfx_overlay_enable(int on);
void rand_seed_from_rtc(void);

/* imports of libfield */
void fld_sys_init(void);
void fld_sys_update(void);
int fld_sys_draw(void);
int fld_sys_term(void);
int fld_cmd_start(int kind, void *arg);
int fld_cmd_poll(int kind, void *arg);

/* --- 0x08804000-0x08806A18 --- */
typedef struct StoryChrSave {          /* state of a story character (save data, 150 of them) */
    unsigned short look;               /* +0x00 portrait/costume from the look table */
    unsigned short pad02;
    int x04;                           /* +0x04 */
    int x08;                           /* +0x08 */
    int x0c;                           /* +0x0C */
    int x10;                           /* +0x10 */
    unsigned char x14;                 /* +0x14 */
    unsigned char attr;                /* +0x15 chr_attr_remap of the look entry */
    unsigned char x16;                 /* +0x16 */
    unsigned char x17;                 /* +0x17 */
    unsigned char level;               /* +0x18 1..100 */
    unsigned char pad19[3];
    int x1c;                           /* +0x1C */
    int look_id;                       /* +0x20 key of the look table entry in use */
    int x24;                           /* +0x24 */
    int x28;                           /* +0x28 */
    int recipe;                        /* +0x2C deck recipe in use */
    unsigned short x30;                /* +0x30 */
    unsigned short x32;                /* +0x32 */
    unsigned short x34;                /* +0x34 */
    unsigned short x36;                /* +0x36 */
    unsigned short x38;                /* +0x38 */
    unsigned short pick_a[6];          /* +0x3A random pool (bit 2 of state) */
    unsigned short pick_b[6];          /* +0x46 */
    unsigned short x52;                /* +0x52 */
    int x54;                           /* +0x54 */
    int x58;                           /* +0x58 */
    int x5c;                           /* +0x5C */
    int x60;                           /* +0x60 */
    unsigned char x64;                 /* +0x64 */
    unsigned char x65;                 /* +0x65 */
    unsigned char x66;                 /* +0x66 */
    unsigned char x67;                 /* +0x67 */
    int x68;                           /* +0x68 -1 none */
    int x6c;                           /* +0x6C -1 none */
    unsigned short x70;                /* +0x70 65000 none */
    unsigned short x72;                /* +0x72 */
    unsigned short x74;                /* +0x74 */
    unsigned short x76;                /* +0x76 */
    unsigned short x78;                /* +0x78 */
    unsigned short x7a[6];             /* +0x7A */
    unsigned short pad86;
    unsigned int flags;                /* +0x88 bits 1..32 */
    unsigned char state;               /* +0x8C bit0 shown, bit2 re-pick */
    unsigned char pad8d[3];
    int day;                           /* +0x90 weekday of the last look update */
    int pick;                          /* +0x94 index picked in pick_a */
} StoryChrSave;

typedef struct StoryChrFlags {         /* 0x0881DA1C points here (save data) */
    unsigned int named[8];             /* +0x00 bit per character: has a real name */
    unsigned short recipes[150][6];    /* +0x20 recipes seen per character */
} StoryChrFlags;

typedef struct StoryChr {              /* script tables of a story character (Scr<i>_*.bin), 0x0881DA24[150] */
    unsigned short *gp;                /* +0x00 G: 6 u16 (date range, place, x24) */
    unsigned short *gs;                /* +0x04 G: 3 u16 (level, place, x24) */
    short *gc;                         /* +0x08 G: 3 u16 (chr flag, place, x24) */
    short *g;                          /* +0x0C G: 3 u16 (story flag, place, x24) */
    unsigned short *step;              /* +0x10 Step: (level, affinity max) pairs */
    unsigned short *deck;              /* +0x14 Deck: 8 u16 entries (date range, level, flag, story flag, recipe) */
    unsigned short *dh;                /* +0x18 DH: 9 u16 entries */
    unsigned short *dn;                /* +0x1C DN: 9 u16 entries */
    unsigned short *sche;              /* +0x20 Sche: 25 u16 entries */
    unsigned short *es;                /* +0x24 5 u16 entries */
    void *ei;                          /* +0x28 */
    void *el;                          /* +0x2C */
    int pad30;
    unsigned short name[0x40];         /* +0x34 */
    unsigned short deck_name[0x40];    /* +0xB4 */
} StoryChr;

typedef struct StoryPlayer {           /* 0x15C, save data */
    int room;                          /* +0x00 */
    int prev_room;                     /* +0x04 */
    float x;                           /* +0x08 */
    float z;                           /* +0x0C */
    int dir;                           /* +0x10 */
    int x14;                           /* +0x14 */
    int x18;                           /* +0x18 */
    int x1c;                           /* +0x1C */
    int x20;                           /* +0x20 */
    int x24;                           /* +0x24 */
    int points;                        /* +0x28 0..999 */
    int x2c;                           /* +0x2C 0..999 */
    int items[21];                     /* +0x30 1..20 */
    int cards[51];                     /* +0x84 1..50 */
    int x150;                          /* +0x150 */
    unsigned short x154;               /* +0x154 */
    unsigned short x156;               /* +0x156 */
    int x158;                          /* +0x158 */
} StoryPlayer;

extern StoryChrFlags *st_chr_flags;    /* 0x0881DA1C */
extern StoryChrSave *st_chr_save;      /* 0x0881DA20 */
extern StoryChr st_chr[150];           /* 0x0881DA24 */
extern StoryPlayer *st_pl;             /* 0x08828E9C */
extern unsigned int *st_pl_bits;       /* 0x08828EA0 */

int st_chr_level(int i);
int st_chr_flag_get(int i, unsigned int bit);
int story_mode_get(void);
int story_mode_is1(void);
int sdate_is_daytime(void);
int sdate_weekday_flag(int d);
int sdate_check(int n);
int sdate_get_0c(void);
int sdate_weekday(void);
unsigned short *stext_get(int i);
void se_play_delta(int d);
int chr_attr_remap(int a);

/* --- 0x08806A18-0x0880B3E4 --- */
typedef struct StoryCmd {              /* one instruction of a story script, 10 bytes */
    unsigned short op;                 /* +0x0 opcode 0xC351..0xC3F5, 0 = end */
    unsigned short a;                  /* +0x2 */
    unsigned short b;                  /* +0x4 */
    unsigned short c;                  /* +0x6 */
    unsigned short d;                  /* +0x8 */
} StoryCmd;

typedef struct StoryVm {               /* 0x08828EA8, 0x298: the script interpreter */
    StoryCmd *script;                  /* +0x000 running script (0 = idle) */
    int next;                          /* +0x004 script queued to run next */
    int pc;                            /* +0x008 index of the current instruction */
    int var[2];                        /* +0x00C */
    int status;                        /* +0x014 */
    unsigned short text[5][0x40];      /* +0x018 lines given to the choice window */
} StoryVm;

extern int *story_vm_slot;             /* 0x08828EA4 where the id of the running script is kept */
extern StoryVm story_vm;               /* 0x08828EA8 */

/* --- 0x0880B3E4-0x0880DE58 --- */

typedef struct ScalDate {              /* story date (scal_date points into the save), 0x24 */
    int tick;                          /* +0x00 (day - 1) * 73 + slot, day counted from month 1 day 1 */
    int month;                         /* +0x04 story month 1..8 */
    int day;                           /* +0x08 */
    int slot;                          /* +0x0C time slot of the day 1..73 (15 minutes each from 5:45) */
    int cal_month;                     /* +0x10 calendar month 5..12 */
    int hour;                          /* +0x14 */
    int minute;                        /* +0x18 */
    int wday;                          /* +0x1C 0 Sunday .. 6 */
    int days_left;                     /* +0x20 92 - days elapsed, not below 0 */
} ScalDate;

typedef struct ScalPeriod {            /* chapter of the story by date, 0x0881D1E0 */
    int month;                         /* +0x00 */
    int day;                           /* +0x04 first day (0: rest of the month) */
    int a;                             /* +0x08 */
    int b;                             /* +0x0C */
} ScalPeriod;

typedef struct SttTables {             /* 0x0882914C text tables (base + offset table), 0x28 */
    int count_a;                       /* +0x00 */
    unsigned short *base_a;            /* +0x04 */
    int *off_a;                        /* +0x08 */
    int count_b;                       /* +0x0C */
    unsigned short *base_b;            /* +0x10 */
    int *off_b;                        /* +0x14 */
    unsigned short *base_c;            /* +0x18 */
    int *off_c;                        /* +0x1C indexed from 1 */
    unsigned short *branch;            /* +0x20 branch table, records of 13 halfwords */
    int branch_x;                      /* +0x24 */
} SttTables;

extern unsigned int *scal_flags;       /* 0x08829140 flags 129..192 ([0], [1]) and 1..32 ([2], scal_flag2) */
extern ScalDate *scal_date;            /* 0x08829144 */
extern int *scal_state;                /* 0x08829148 [0] day number seen, [1]/[2] flags 1..64 */
extern SttTables stt;                  /* 0x0882914C */

int story_flag_get(unsigned int id);
int story_date_in_range(int m0, int d0, int m1, int d1);

typedef struct StRes {                 /* 0x08AC9180 heaps and the file loader, 0x1A8 */
    unsigned char x000[0x18C];
    void *heap[3];                     /* +0x18C 0: 2.25 MB (files), 1: 256 KB, 2: 128 KB */
    void *file;                        /* +0x198 file being read by st_file_load */
    int file_size;                     /* +0x19C */
    int file_done;                     /* +0x1A0 set by the read callback */
    int x1a4;
} StRes;

extern StRes st_res;                   /* 0x08AC9180 */

void *st_save_buf(void);
void *st_file_load(const char *path, int heap);
void st_res_free(void *p, int heap);
void *st_ehp_load(void *ehp, const char *name, int heap);

/* --- 0x0880DE58-0x08810D24 --- */

typedef struct StoryCtx {              /* 0x08AC9328 story_ctx, 0x1C (cleared by st_init_80c9e4) */
    int x00;                           /* +0x00 */
    int prev;                          /* +0x04 sub-screen the story came back from */
    int state;                         /* +0x08 story mode state (dispatched by story_update) */
    int movie_fpl;                     /* +0x0C HeapMovie pool of the staff roll */
    void *movie_mem;                   /* +0x10 */
    void (*step)(void);                /* +0x14 multi-frame step run by story_update */
    int x18;                           /* +0x18 */
} StoryCtx;
extern StoryCtx story_ctx;             /* 0x08AC9328 */

/* --- 0x08810D24-0x088138AC --- */

/* --- 0x088138AC-0x088156A4 --- */

typedef struct SevEntry {              /* map event of the story event table, 0x1E */
    unsigned short from_month;         /* +0x00 date range (all four 0: always) */
    unsigned short from_day;           /* +0x02 */
    unsigned short to_month;           /* +0x04 */
    unsigned short to_day;             /* +0x06 */
    unsigned short chr;                /* +0x08 character whose level must equal level (0: none) */
    unsigned short level;              /* +0x0A */
    unsigned short flag;               /* +0x0C story flag tested (0: none) */
    unsigned short flag_cond;          /* +0x0E 3 needs it set, 4 needs it clear */
    unsigned short chr2;               /* +0x10 character whose flag bit is tested (0: none) */
    unsigned short bit;                /* +0x12 */
    unsigned short bit_cond;           /* +0x14 3 needs it set, 4 needs it clear */
    unsigned short kind;               /* +0x16 0 with x/y/z 0 ends the table */
    unsigned short x;                  /* +0x18 */
    unsigned short y;                  /* +0x1A */
    unsigned short z;                  /* +0x1C */
} SevEntry;

typedef struct SevSlot {               /* event active this frame, 0x10 */
    int id;                            /* +0x00 index in the table + 1 (0: free) */
    int x, y, z;
} SevSlot;

typedef struct SevState {              /* 0x08ACB030, 0x84 */
    SevEntry *table;                   /* +0x00 */
    SevSlot slot[8];                   /* +0x04 */
} SevState;

extern SevState sev;                   /* 0x08ACB030 */
extern unsigned int *sev_done;         /* 0x08ACB02C 128 bits: event already seen */

typedef struct StalkList {             /* argument of the libfield list commands: array and count */
    void *items;
    int n;
} StalkList;

typedef struct StalkBu {               /* bust-up shown by the field (item of field command 0xE) */
    int chara;                         /* +0x00 costume */
    int index;                         /* +0x04 screen slot: 0 left, 1 centre, 2 right */
    int anim;                          /* +0x08 */
} StalkBu;

typedef struct StalkBuAnim {           /* item of field command 0xF (bust-up animation) */
    int index;
    int anim;
} StalkBuAnim;

typedef struct StalkMsg {              /* argument of field command 0x13 (message window) */
    unsigned short *text;              /* 0: close the window */
    unsigned short *title;             /* speaker name */
    int speaker;                       /* slot of the speaking bust-up, -1 none */
} StalkMsg;

typedef struct StalkState {            /* 0x08ACB0B4, 0x224 */
    unsigned short text[256];          /* +0x000 expanded text of the message */
    int chr[2];                        /* +0x200 script id of the character in each bust-up */
    StalkBu bu[2];                     /* +0x208 */
    int talk_pos;                      /* +0x220 slot of the one speaking, -1 none */
} StalkState;

extern StalkState stalk;               /* 0x08ACB0B4 */

/* --- 0x088156A4-0x08817E64 --- */

#endif
