// @range 0x08865260-0x08866068
// @data 0x088F04AC-0x088F078C
// @rodata 0x088EAFE0-0x088EB030
// @asm 0x088657A8-0x0886591C
// @asm 0x08865A50-0x08865B50
// @asm 0x08865BC4-0x08865CB0
/* Game-side sound glue: heap and loader callbacks handed to the sound library (0x0885C688..0x08865260,
   still asm), the BGM/SE API exported to the gmodules (thin wrappers), character voice tables,
   per-player voice channels, movie_play with the configured volume, and the save-data path helpers
   (game id ULUS10136 + "0001", ms0:/PSP/SAVEDATA). */
#include "psp.h"
#include "libc.h"
#include "game.h"
#include "ehfile.h"
#include "ehsys.h"
#include "movie.h"

typedef struct SndCallbacks {          /* handed to snd_thread_start; snd_callbacks_default fills the rest */
    int (*read)(void *name, void *buf, int size, int off);
    void *(*alloc)(int size);
    void (*free)(void *p);
    void *unk0C;
    void *unk10;
} SndCallbacks;

typedef struct SndChannelSlot {        /* queued voice for one player slot, 4 per slot */
    int state;                         /* 0 queued, 1 playing */
    int se_id;
    int delay;                         /* frames before the next one */
    int pad;
} SndChannelSlot;

typedef struct SndChannel {            /* 0x80 */
    SndChannelSlot q[8];
} SndChannel;

extern unsigned int snd_flags;          /* 0x08C1FCF8 bit0: muted */
extern void *snd_heap;                  /* 0x08C1FCFC current heap for sound data */
extern void *snd_heap_main;             /* 0x08C1FD00 the 2 MB heap created by snd_init */
extern unsigned char snd_heap_mem[];    /* 0x08C1FD40 */
typedef struct SndChannelTab {         /* 0x08C1FAE8, 0x210: one table, indexed from its base in the original */
    int active[4];                     /* queue length per player slot */
    SndChannel ch[4];                  /* 0x08C1FAF8 */
} SndChannelTab;

extern SndChannelTab snd_channel_tab;
extern unsigned char snd_bank_cfg[];    /* 0x088B7DE0 "SHDS" sound bank configuration */

void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
void snd_group_release_all(int cmd, int flag);
unsigned int snd_bank_offset();
void snd_callbacks_default(SndCallbacks *cb);
void snd_thread_start(void *cfg, SndCallbacks *cb);
void snd_bank_open_async(int a, int b);
void snd_sas_init(int a, int b);
void snd_set_channel_volume(int ch, int vol, int time);
void snd_set_master_volume(int vol, int time);
void snd_group_load(int a, int b, int c, int d);
void snd_group_release(int x, int flag);
void bgm_start(int id, int id2, int pos, int loop, int flags);
int se_play_pan(int id, int pan, int c);
int snd_obj_base_id(int obj);
void snd_stop_fade(int x, int ms);
void snd_group_cmd(int cmd, int arg);
int snd_obj_count_bgm();
int snd_obj_count_se();
int snd_cur_set_volume();
int snd_obj_set_volume();
int snd_obj_query();
int snd_obj_status();
int snd_seq_entry();
int snd_group_read_done(void *obj, int a);
void snd_set_volume(int ch, int vol);
SysConfig *cfg_get(void);
unsigned short charmap_lookup(int c, int flag);
int charmap_lookup3(int c);
unsigned short region_pick_value(int flag);

int snd_read_cb(void *name, void *buf, int size, int off);
int snd_read_done_cb(void *arg, int result);
void *snd_heap_alloc_cb(int size);
void snd_heap_free_cb(void *p);
int snd_load_cb(void *buf, int result);
int voice_table_find(unsigned int chr, unsigned int variant);
int gamestate_partner_mode(void);
int partner_voice_id(unsigned int chr, unsigned int variant);
int voice_play(int obj, int idx);
int bgm_get_status();
void bgm_stop(int x);
void se_play(int id);

/* Character voice table: (character, variant) -> base sound id; the list ends with character 0x98. */
unsigned short voice_pairs[190] = {   /* {chr, variant, id, 0} records ending with chr 0x98, then 4 bytes of padding */
    0x2, 0x0, 0x5D, 0x0, 0x3, 0x0, 0x60, 0x0,
    0x4, 0x0, 0x63, 0x0, 0x5, 0x0, 0x66, 0x0,
    0x6, 0x0, 0xE1, 0x0, 0x6, 0x1, 0x69, 0x0,
    0x7, 0x0, 0x6C, 0x0, 0x8, 0x0, 0x6F, 0x0,
    0x9, 0x0, 0x72, 0x0, 0xA, 0x0, 0x75, 0x0,
    0xB, 0x0, 0x78, 0x0, 0xC, 0x0, 0x7B, 0x0,
    0xC, 0x1, 0xDB, 0x0, 0xD, 0x0, 0x7E, 0x0,
    0xD, 0x1, 0xD2, 0x0, 0xE, 0x0, 0x81, 0x0,
    0xF, 0x0, 0x84, 0x0, 0x10, 0x0, 0x87, 0x0,
    0x10, 0x1, 0x87, 0x0, 0x11, 0x0, 0x8A, 0x0,
    0x12, 0x0, 0xD5, 0x0, 0x12, 0x1, 0x8D, 0x0,
    0x12, 0x2, 0xDE, 0x0, 0x13, 0x0, 0x90, 0x0,
    0x14, 0x0, 0x93, 0x0, 0x15, 0x0, 0x96, 0x0,
    0x16, 0x0, 0x99, 0x0, 0x17, 0x0, 0x9C, 0x0,
    0x18, 0x0, 0x9F, 0x0, 0x18, 0x1, 0xD8, 0x0,
    0x19, 0x0, 0xA2, 0x0, 0x1A, 0x0, 0xA5, 0x0,
    0x1B, 0x0, 0xA8, 0x0, 0x1C, 0x0, 0xAB, 0x0,
    0x1D, 0x0, 0xAE, 0x0, 0x1E, 0x0, 0xB1, 0x0,
    0x1F, 0x0, 0xB4, 0x0, 0x20, 0x0, 0xB7, 0x0,
    0x21, 0x0, 0xBA, 0x0, 0x22, 0x0, 0xBD, 0x0,
    0x23, 0x0, 0xC0, 0x0, 0x24, 0x0, 0xC3, 0x0,
    0x25, 0x0, 0xC6, 0x0, 0x26, 0x0, 0xC9, 0x0,
    0x27, 0x0, 0xCC, 0x0, 0x28, 0x0, 0xCF, 0x0,
    0x98, 0x0, 0x0, 0x0, 0x0, 0x0,
};
unsigned short voice_tbl_a[15][4] = {
    { 0xE4, 0x0, 0x1, 0x0 }, { 0xE7, 0x0, 0x2, 0x0 }, { 0xEA, 0x0, 0x3, 0x0 }, { 0xED, 0x0, 0x4, 0x0 },
    { 0xF0, 0x0, 0x5, 0x0 }, { 0xF3, 0x0, 0x6, 0x0 }, { 0xF6, 0x0, 0x7, 0x0 }, { 0xF9, 0x0, 0x8, 0x0 },
    { 0xFC, 0x0, 0x9, 0x0 }, { 0xFF, 0x0, 0xA, 0x0 }, { 0x102, 0x0, 0xB, 0x0 }, { 0x105, 0x0, 0xC, 0x0 },
    { 0x108, 0x0, 0xD, 0x0 }, { 0x10B, 0x0, 0xE, 0x0 }, { 0x10E, 0x0, 0x0, 0x0 },
};
unsigned short voice_tbl_b[15][4] = {
    { 0x111, 0x0, 0x1, 0x0 }, { 0x114, 0x0, 0x2, 0x0 }, { 0x117, 0x0, 0x3, 0x0 }, { 0x11A, 0x0, 0x4, 0x0 },
    { 0x11D, 0x0, 0x5, 0x0 }, { 0x120, 0x0, 0x6, 0x0 }, { 0x123, 0x0, 0x7, 0x0 }, { 0x126, 0x0, 0x8, 0x0 },
    { 0x129, 0x0, 0x9, 0x0 }, { 0x12C, 0x0, 0xA, 0x0 }, { 0x12F, 0x0, 0xB, 0x0 }, { 0x132, 0x0, 0xC, 0x0 },
    { 0x135, 0x0, 0xD, 0x0 }, { 0x138, 0x0, 0xE, 0x0 }, { 0x13B, 0x0, 0x0, 0x0 },
};
unsigned short voice_tbl_c[10] = { 0x13E, 0x0, 0x1, 0x0, 0x141, 0x0, 0x2, 0x0, 0x144, 0x0 };

void snd_heap_set(void *heap)
{
    snd_group_release_all(-0x105, 1);
    snd_heap = heap;
}

void snd_heap_reset(void)
{
    snd_group_release_all(-0x105, 1);
    snd_heap = snd_heap_main;
}

/* Sound library read callback: reads from psp_snddat.bin (or the named file) in dir 7 (snd/). */
int snd_read_cb(void *name, void *buf, int size, int off)
{
    void *arg = name;

    if (off == -1 || (int)name >= 0x08000000)   /* a real file name: read it whole */
        off = 0;
    else
        name = "psp_snddat.bin";                   /* a bank id: read psp_snddat.bin at `off` */
    return ehfile_read_async(7, name, buf, off, size, (EhfileCallback)snd_read_done_cb, arg);
}

int snd_read_done_cb(void *arg, int result)
{
    if (result <= 0)
        return 0;
    snd_group_read_done(arg, 0);
    return 1;
}

void *snd_heap_alloc_cb(int size)
{
    return heap_alloc(snd_heap, size);
}

void snd_heap_free_cb(void *p)
{
    heap_set_level(snd_heap, p);
}

/* Loads the bank header of psp_snddat.bin into the sound heap. */
void snd_load_data(void)
{
    unsigned int off = snd_bank_offset();
    void *buf = heap_alloc(snd_heap, 0x40);
    ehfile_read_async(7, "psp_snddat.bin", buf, off, 0, (EhfileCallback)snd_load_cb, buf);
}

int snd_load_cb(void *buf, int result)
{
    if (result < 0)
        return 0;
    heap_set_level(snd_heap, buf);
    return 1;
}

/* Loads the codec modules, creates the 2 MB sound heap and starts the sound thread. */
void snd_init(void)
{
    SndCallbacks cb;

    modmgr_load_by_id(3);
    modmgr_load_by_id(7);
    modmgr_load_by_id(6);
    while (!modmgr_none_loading())
        sys_frame_end();
    snd_flags = 0;
    snd_heap = heap_create(snd_heap_mem, 0x200000);
    snd_heap_main = snd_heap;
    snd_callbacks_default(&cb);
    cb.read = snd_read_cb;
    cb.alloc = snd_heap_alloc_cb;
    cb.free = snd_heap_free_cb;
    snd_thread_start(snd_bank_cfg, &cb);
    snd_bank_open_async(0, 0);
    snd_sas_init(2, 2);
    snd_set_volume(0, 12);
    snd_set_volume(1, 12);
    snd_set_volume(2, 12);
}

int cfg_get_byte3(int i)
{
    SysConfig *c = cfg_get();

    return c->bytes3[i];
}

/* Config volume 0..15 for channel 0 BGM, 1 SE, 2 voice (muted outside Japanese). */
void snd_set_volume(int ch, int vol)
{
    SysConfig *c = cfg_get();

    c->bytes3[ch] = vol;
    if (vol != 0)
        vol = vol * 8 + 7;
    switch (ch) {
    case 0:
        snd_set_channel_volume(0, vol, 200);
        break;
    case 1:
        snd_set_channel_volume(1, vol, 200);
        break;
    case 2:
        if (cfg_get_language() == 0)
            snd_set_channel_volume(2, vol, 200);
        else
            snd_set_channel_volume(2, 0, 200);
        break;
    }
}

void snd_mute_on(void)
{
    snd_flags |= 1;
    snd_set_master_volume(0, 0);
}

void snd_mute_off(void)
{
    snd_flags &= ~1;
    snd_set_master_volume(0x7F, 0);
}

int snd_is_muted(void)
{
    return (snd_flags & 1) != 0;
}

void bgm_fade(int x)
{
    snd_group_load(x, 0, x, 0);
}

void bgm_cmd3(int x)
{
    snd_group_release(x, 1);
}

void snd_cmd_fefb(void)
{
    snd_group_release_all(-0x105, 1);
}

void snd_cmd_fefa(void)
{
    snd_group_release_all(-0x106, 1);
}

void bgm_play(int id)
{
    bgm_start(id, id, 0, -1, 0);
}

void bgm_play_from(int id, int pos)
{
    bgm_start(id, id, pos, -1, 0);
}

void se_play(int id)
{
    se_play_pan(id, 0, 0);
}

void se_play_at(int id, int pan)
{
    se_play_pan(id, pan, 0);
}

/* Plays voice `idx` of a sound object (base id from the bank). */
int voice_play(int obj, int idx)
{
    int base = snd_obj_base_id(obj);
    if (base < 0)
        return 0;
    return se_play_pan(base + idx, 0, 0);
}

void bgm_stop(int x)
{
    snd_stop_fade(x, 0);
}

void bgm_fade_out(int x, int ms)
{
    snd_stop_fade(x, ms);
}

void snd_cmd_fefb_0(void)        { snd_group_cmd(-0x105, 0); }
void snd_cmd_fefb_arg(int arg)   { snd_group_cmd(-0x105, arg); }
void snd_cmd_fefa_0(void)        { snd_group_cmd(-0x106, 0); }
void bgm_cmd(int arg)            { snd_group_cmd(-0x106, arg); }
void snd_cmd_fef7_0(void)        { snd_group_cmd(-0x109, 0); }
void snd_cmd_fef7_arg(int arg)   { snd_group_cmd(-0x109, arg); }
void snd_cmd_fef9(void)          { snd_group_cmd(-0x107, 0); }
void snd_cmd_fef9_arg(int arg)   { snd_group_cmd(-0x107, arg); }
void snd_cmd_fef8(void)          { snd_group_cmd(-0x108, 0); }
void snd_cmd_fef8_arg(int arg)   { snd_group_cmd(-0x108, arg); }
int snd_count_bgm()              { return snd_obj_count_bgm(); }
int snd_count_se()               { return snd_obj_count_se(); }
int bgm_set_pan()                { return snd_cur_set_volume(); }
int bgm_set_volume()             { return snd_obj_set_volume(); }
int bgm_query()                  { return snd_obj_query(); }
int bgm_get_status()             { return snd_obj_status(); }
int snd_seq_get_u16()            { return snd_seq_entry(); }
int snd_get_base_id(int obj)     { return snd_obj_base_id(obj); }

#ifdef NON_MATCHING /* voice_table_find: loop and range-test layout; gamestate_partner_mode: result kept in a0 across the tests */
/* Voice base id for a character: 0..0x28 use the pair list, 0x29.. derive from the charmap group. */
int voice_table_find(unsigned int chr, unsigned int variant)
{
    unsigned short *p;
    unsigned int c;
    int group, sub;

    if ((int)chr < 0x29) {
        c = voice_pairs[0];
        p = voice_pairs;
        if (c != 0x98) {
            while (c != chr || p[1] != variant) {
                p += 4;
                c = *p;
                if (c == 0x98)
                    return 0;
            }
            return p[2];
        }
    } else {
        group = charmap_lookup(chr, 0);
        sub = charmap_lookup3(chr);
        if (group == 1000) {
            if (sub < 3)
                return voice_tbl_c[sub * 4];
            return 0;
        }
        if (group < 100)
            return 0;
        if (sub >= 15)
            return 0;
        if (group < 400)
            return voice_tbl_a[sub][0];
        if (group < 500)
            return voice_tbl_b[sub][0];
    }
    return 0;
}

/* 0 single, 1 both partners set, 2 tag mode flag. */
int gamestate_partner_mode(void)
{
    GameState *gs = game_state_get();

    if (gs->partner_mode == 2)
        return 2;
    if (gs->slot[0].type < 2)
        return 0;
    if (gs->slot[1].type < 2)
        return 0;
    return 1;
}

#endif

int partner_voice_id(unsigned int chr, unsigned int variant)
{
    int id = voice_table_find(chr, variant);
    if (id == 0)
        return 0;
    return id + gamestate_partner_mode();
}

int partner_voice_play(int slot, int arg)
{
    PlayerSlot *p = &game_state_get()->slot[slot];
    int id = partner_voice_id(p->type, p->type2);
    if (id == 0)
        return 0;
    id = snd_obj_base_id(id);
    if (id != 0)
        return id + arg;
    return 0;
}

#ifdef NON_MATCHING /* sense of the likely branch on the base id */
/* Sound id for voice `arg` of the character in player slot `slot`, 0 when none. */

#endif

int gamestate_slot_snd(int slot, int arg)
{
    int obj = game_state_get()->slot[slot].voice_obj;
    if (obj == 0)
        return 0;
    return voice_play(obj, arg);
}

void snd_channels_reset(void)
{
    memset(&snd_channel_tab, 0, sizeof(SndChannelTab));
}

#ifdef NON_MATCHING /* the original indexes the queue from the table base (base + slot*0x80 + n*0x10 + 0x10) */
/* Queues voice `se` of the slot's character with a delay (frames) after the previous one. */
int partner_snd_channel(int slot, int se, int delay)
{
    PlayerSlot *p = &game_state_get()->slot[slot];
    int n;

    if (p->voice_obj == 0)
        return 0;
    n = snd_channel_tab.active[slot];
    if (n >= 8)
        return 0;
    snd_channel_tab.ch[slot].q[n].state = 0;
    snd_channel_tab.ch[slot].q[n].delay = delay;
    snd_channel_tab.ch[slot].q[n].se_id = se + snd_obj_base_id(p->voice_obj);
    snd_channel_tab.active[slot]++;
    return 1;
}

int snd_channel_get_u32(int slot)
{
    if (snd_channel_tab.active[slot] == 0)
        return 0;
    return snd_channel_tab.ch[slot].q[0].se_id;
}

#endif

/* Stops the slot's current voice. */
void snd_channel_update(int slot)
{
    int *act = &snd_channel_tab.active[slot];
    SndChannel *c;

    if (*act == 0)
        return;
    c = &snd_channel_tab.ch[slot];
    if (c->q[0].state != 0 && bgm_get_status(c->q[0].se_id) != 0)
        bgm_stop(c->q[0].se_id);
    *act = 0;
}

#ifdef NON_MATCHING /* s-register allocation of the slot pointers */
/* Per frame: starts the queued voice of each slot, advances the queue when it ends. */
void snd_channels_update(void)
{
    SndChannel *c;
    int slot, i;

    for (slot = 0; slot < 4; slot++) {
        c = &snd_channel_tab.ch[slot];
        if (snd_channel_tab.active[slot] == 0)
            continue;
        if (c->q[0].state == 0) {
            se_play(c->q[0].se_id);
            c->q[0].state = 1;
        } else if (bgm_get_status(c->q[0].se_id) == 0) {
            if (c->q[0].delay == 0) {
                for (i = 1; i < 8; i++)
                    memcpy(&c->q[i - 1], &c->q[i], 0x10);
                snd_channel_tab.active[slot]--;
            } else {
                c->q[0].delay--;
            }
        }
    }
}

#endif

/* Plays a PMF with the configured BGM volume (silent when muted). */
void movie_play(const char *path, unsigned int flags, int arg)
{
    int vol = cfg_get_byte3(0) * 0x8000 / 15;
    if (snd_is_muted())
        vol = 0;
    movie_run(path, flags | 1, vol, arg);
}

/* Save-data game id: region code + "0001". */
void region_game_id(char *out)
{
    switch (region_get()) {
    case 0:
        strcpy(out, "ULJM05151");
        break;
    case 1:
        strcpy(out, "ULUS10136");
        break;
    case 2: case 3: case 4: case 5: case 6:
        strcpy(out, "SLPS10000");
        break;
    }
    strcat(out, "0001");
}

void savedata_dir_path(char *out)
{
    char id[32];

    strcpy(out, "ms0:PSP/SAVEDATA/");
    region_game_id(id);
    strcat(out, id);
}

/* Reads a whole file from the save-data directory; 1 when `size` bytes were read. */
int savedata_read_file(const char *name, void *buf, int size)
{
    char path[64];
    int fd, n;

    savedata_dir_path(path);
    strcat(path, "/");
    strcat(path, name);
    strcat(path, "");
    fd = sceIoOpen(path, 0x2030001, 0);
    if (fd < 0)
        return 0;
    n = sceIoRead(fd, buf, size);
    sceIoClose(fd);
    if (n < 0 || n != size)
        return 0;
    return 1;
}

/* Common sceUtility dialog parameters: language, button swap, sysmsg ids of the four buttons. */
void savedata_params_init(int *p, int base)
{
    int lang;

    p[0] = base;
    switch (cfg_get_language()) {
    case 0: lang = 0; break;
    case 2: lang = 4; break;
    case 3: lang = 2; break;
    case 4: lang = 5; break;
    case 5: lang = 3; break;
    case 1:
    default: lang = 1; break;
    }
    p[1] = lang;
    if (region_pick_value(0) == 0x2000)
        p[2] = 0;
    else
        p[2] = 1;
    p[3] = 0x11;
    p[4] = 0x13;
    p[5] = 0x12;
    p[6] = 0x10;
}

char savedata_data0[] = "DATA0";   /* named: also read by the save-data asm after this file */

/* Fills the save-data game name / save name / file name of a sceUtilitySavedata param block. */
void savedata_set_gameid(char *p)
{
    switch (region_get()) {
    case 0:
        strcat(p + 0x3C, "ULJM05151");
        break;
    case 1:
        strcat(p + 0x3C, "ULUS10136");
        break;
    case 2: case 3: case 4: case 5: case 6:
        strcat(p + 0x3C, "SLPS10000");
        break;
    default:
        strcat(p + 0x3C, "ULUS10136");
        break;
    }
    strcpy(p + 0x4C, "0001");
    *(int *)(p + 0x60) = 0;
    strcpy(p + 0x64, savedata_data0);
}
