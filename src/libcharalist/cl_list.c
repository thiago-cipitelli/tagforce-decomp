// @range 0x08807F68-0x0880A34C
// @asm 0x08808264-0x0880981C
// @asm 0x08809914-0x08809B8C
// @asm 0x08809C54-0x0880A2D8
// @data 0x0880E138-0x0880EA74
/* The three tabs (the met duelists, the other ones, the deck-only ones), input of the list and of
   the detail page (costumes, profile scroll, voice test with its theme), the character table and the
   portrait of the duelists without a bust-up. */
#include "libcharalist.h"

typedef struct ClPair {        /* list_build output: number shown and character */
    unsigned short num;
    unsigned short id;
} ClPair;

typedef struct DeckSlot {      /* BOOT, SysSave +0x64 */
    unsigned short level;
    unsigned short count;
    unsigned short cards[6];
    unsigned short value;
} DeckSlot;

typedef struct CharmapPos {    /* BOOT charmap_lookup2 */
    unsigned char a;
    unsigned char b;
    unsigned char c : 4;
    unsigned char hi : 4;
} CharmapPos;

int value_to_tier(int v);
int charmap_lookup(int ch, int *kind_out);
int charmap_lookup2(int code, CharmapPos *out);
DeckSlot *deck_slot_get(int i);
int deck_slot_get_level(int i);
int sysflag_test(unsigned int bit);
void bgm_cmd(int x);
void bgm_cmd3(int x);
void bgm_fade(int x);
int snd_seq_get_u16();
int snd_count_se();
void snd_cmd_fef8(void);
void snd_set_volume(int ch, int vol);
unsigned char cfg_get_byte3_b(int i);
unsigned short partner_table_lookup(int flag, unsigned short mode, unsigned short idx, unsigned short *table);
int font_wtext_lines(int w, unsigned short *text);
unsigned short font_get_flags(void);
void font_set_flags(unsigned short flags);
int ehf_get_frame_count(void *e);
int ehf_get_data_ptr(void *e);
unsigned int *gim_set_texture(unsigned int *list, void *gim);
unsigned int *gim_set_texture_sub(unsigned int *list, void *g, int frame, int cell, int pframe, int pcell);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);

unsigned short cl_entries_build(ClEntry *out, ClPair *src, unsigned short n);
unsigned short cl_entries_simple(ClEntry *out, ClPair *src, unsigned short n);
void cl_costume_anim(void);
void cl_profile_measure(ClEntry *e);
void cl_voices_build(ClEntry *e, int alt);

/* theme (bgm), first and second voice-test se of each voice set */
ClBgm cl_bgm[47] = {
    { 0, 0, 0 }, { 0, 0, 0 }, { 93, 338, 481 }, { 96, 770, 913 },
    { 99, 1202, 1345 }, { 102, 1634, 1777 }, { 105, 2066, 2209 }, { 108, 2498, 2641 },
    { 111, 2930, 3073 }, { 114, 3362, 3505 }, { 117, 3794, 3937 }, { 120, 4226, 4369 },
    { 123, 4658, 4801 }, { 126, 5090, 5233 }, { 129, 5522, 5665 }, { 132, 5954, 6097 },
    { 135, 6386, 6529 }, { 138, 6818, 6961 }, { 141, 7250, 7393 }, { 144, 7682, 7825 },
    { 147, 8114, 8257 }, { 150, 8546, 8689 }, { 153, 8978, 9121 }, { 156, 9410, 9553 },
    { 159, 9842, 9985 }, { 162, 10274, 10417 }, { 165, 10706, 10849 }, { 168, 11138, 11281 },
    { 171, 11570, 11713 }, { 174, 12002, 12145 }, { 177, 12434, 12577 }, { 180, 12866, 13009 },
    { 183, 13298, 13441 }, { 186, 13730, 13873 }, { 189, 14162, 14305 }, { 192, 14594, 14737 },
    { 195, 15026, 15169 }, { 198, 15458, 15601 }, { 201, 15890, 16033 }, { 204, 16322, 16465 },
    { 207, 16754, 16897 }, { 210, 17186, 17329 }, { 213, 17618, 17761 }, { 216, 18050, 18193 },
    { 219, 18482, 18625 }, { 222, 18914, 19057 }, { 225, 19346, 19489 },
};

/* voice set of each costume, by character */
unsigned short cl_voice_set[41][3] = {
    { 0, 0, 0 }, { 0, 0, 0 }, { 2, 0, 0 }, { 3, 0, 0 }, { 4, 0, 0 }, { 5, 0, 0 },
    { 46, 6, 0 }, { 7, 0, 0 }, { 8, 0, 0 }, { 9, 0, 0 }, { 10, 0, 0 }, { 11, 11, 0 },
    { 12, 0, 0 }, { 13, 0, 0 }, { 14, 14, 0 }, { 15, 0, 0 }, { 16, 16, 0 }, { 17, 0, 0 },
    { 42, 18, 18 }, { 19, 0, 0 }, { 20, 0, 0 }, { 21, 0, 0 }, { 22, 0, 0 }, { 23, 0, 0 },
    { 24, 43, 0 }, { 25, 0, 0 }, { 26, 0, 0 }, { 27, 0, 0 }, { 28, 0, 0 }, { 0, 0, 0 },
    { 30, 0, 0 }, { 31, 31, 0 }, { 32, 0, 0 }, { 33, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
    { 36, 0, 0 }, { 37, 0, 0 }, { 38, 0, 0 }, { 0, 0, 0 }, { 40, 0, 0 },
};

/* voice-test entries: se offsets of the two halves (-1 none) */
short cl_voice_se[124][2] = {
    { 0, -1 }, { 1, -1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }, { 8, 9 }, { 10, 11 }, { 12, 13 },
    { 14, 15 }, { 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 }, { 24, 25 }, { 26, -1 }, { 27, -1 },
    { 28, -1 }, { 29, -1 }, { 30, -1 }, { 31, -1 }, { 32, -1 }, { 33, -1 }, { 34, -1 }, { 35, -1 },
    { 36, -1 }, { 37, -1 }, { 38, -1 }, { 39, -1 }, { 40, -1 }, { 41, -1 }, { 42, -1 }, { 43, -1 },
    { 44, -1 }, { 45, -1 }, { 46, -1 }, { 47, -1 }, { 48, -1 }, { 49, -1 }, { 50, -1 }, { 51, -1 },
    { 52, -1 }, { 53, -1 }, { 54, -1 }, { 55, -1 }, { 56, -1 }, { 57, -1 }, { 58, -1 }, { 59, -1 },
    { 60, -1 }, { 61, -1 }, { 62, -1 }, { 63, -1 }, { 64, -1 }, { 65, -1 }, { 66, -1 }, { 67, -1 },
    { 68, -1 }, { 69, -1 }, { 70, -1 }, { 71, -1 }, { 72, -1 }, { 73, -1 }, { 74, -1 }, { 75, -1 },
    { 76, -1 }, { 77, -1 }, { 78, -1 }, { 79, -1 }, { 80, -1 }, { 81, -1 }, { 82, -1 }, { 83, -1 },
    { 84, -1 }, { 85, -1 }, { 86, -1 }, { 87, -1 }, { 88, -1 }, { 89, -1 }, { 90, -1 }, { 91, -1 },
    { 92, -1 }, { 93, -1 }, { 94, -1 }, { 95, 96 }, { 97, 98 }, { 99, 100 }, { 101, 102 }, { 103, -1 },
    { 104, -1 }, { 105, -1 }, { 106, -1 }, { 107, -1 }, { 108, -1 }, { 109, -1 }, { 110, -1 }, { 111, -1 },
    { 112, -1 }, { 113, -1 }, { 114, -1 }, { 115, 116 }, { 117, 118 }, { 119, 120 }, { 121, 122 }, { 123, -1 },
    { 124, -1 }, { 125, -1 }, { 126, -1 }, { 127, -1 }, { 128, -1 }, { 129, -1 }, { 130, -1 }, { 131, -1 },
    { 132, -1 }, { 133, -1 }, { 134, -1 }, { 135, -1 }, { 136, -1 }, { 137, -1 }, { 138, -1 }, { 139, -1 },
    { 140, -1 }, { 141, -1 }, { 142, -1 }, { 143, -1 },
};

/* profile strings (second one for the alternate costume), by character */
unsigned short cl_profile_text[41][2] = {
    { 0, 0 }, { 0, 0 }, { 12, 0 }, { 13, 0 }, { 14, 0 }, { 15, 0 }, { 16, 17 }, { 18, 0 },
    { 19, 0 }, { 20, 0 }, { 21, 0 }, { 22, 0 }, { 23, 0 }, { 24, 0 }, { 25, 0 }, { 26, 0 },
    { 27, 28 }, { 29, 0 }, { 30, 0 }, { 31, 0 }, { 32, 0 }, { 33, 0 }, { 34, 0 }, { 35, 0 },
    { 37, 36 }, { 38, 0 }, { 39, 0 }, { 40, 0 }, { 41, 0 }, { 0, 0 }, { 43, 0 }, { 44, 0 },
    { 45, 0 }, { 46, 0 }, { 0, 0 }, { 0, 0 }, { 48, 0 }, { 49, 0 }, { 50, 0 }, { 0, 0 },
    { 51, 0 },
};

/* the 41 characters: id, rank, costumes and voice flag, bust-up names, face icon */
ClChara cl_charas[41] = {
    { { 0x0000 }, 0, { "", "", "" }, { -1 }, 0 },
    { { 0x0001 }, 0, { "", "", "" }, { 0 }, 0 },
    { { 0x2402 }, 0, { "jdi", "", "" }, { 1 }, 0 },
    { { 0x2403 }, 0, { "sho", "", "" }, { 2 }, 0 },
    { { 0x2404 }, 0, { "hyt", "", "" }, { 3 }, 0 },
    { { 0x2C05 }, 0, { "ask", "", "" }, { 4 }, 0 },
    { { 0xC406 }, 0, { "mab", "maa", "" }, { 5 }, 0 },
    { { 0x2807 }, 0, { "mis", "", "" }, { 7 }, 0 },
    { { 0x2C08 }, 0, { "ryo", "", "" }, { 8 }, 0 },
    { { 0x3009 }, 0, { "cro", "", "" }, { 9 }, 0 },
    { { 0x300A }, 0, { "dai", "", "" }, { 10 }, 0 },
    { { 0x4C0B }, 0, { "fba", "fbb", "" }, { 11 }, 0 },
    { { 0x340C }, 0, { "cmu", "", "" }, { 13 }, 0 },
    { { 0x340D }, 0, { "tny", "", "" }, { 14 }, 0 },
    { { 0x540E }, 0, { "zrg", "zmg", "" }, { 15 }, 0 },
    { { 0x340F }, 0, { "ad3", "", "" }, { 17 }, 0 },
    { { 0x5410 }, 0, { "tiy", "tit", "" }, { 18 }, 0 },
    { { 0x3411 }, 0, { "amu", "", "" }, { 20 }, 0 },
    { { 0xF812 }, 0, { "kgy", "kgo", "kgw" }, { 21 }, 0 },
    { { 0x2C13 }, 0, { "tai", "", "" }, { 24 }, 0 },
    { { 0x2014 }, 0, { "mtg", "", "" }, { -1 }, 0 },
    { { 0x2C15 }, 0, { "jun", "", "" }, { 26 }, 0 },
    { { 0x2C16 }, 0, { "moe", "", "" }, { 27 }, 0 },
    { { 0x2017 }, 0, { "sal", "", "" }, { -1 }, 0 },
    { { 0xC418 }, 0, { "rei", "rec", "" }, { 29 }, 0 },
    { { 0x3819 }, 0, { "ani", "", "" }, { 31 }, 0 },
    { { 0x381A }, 0, { "oto", "", "" }, { 32 }, 0 },
    { { 0x381B }, 0, { "bmg", "", "" }, { 33 }, 0 },
    { { 0x381C }, 0, { "sas", "", "" }, { 34 }, 0 },
    { { 0x201D }, 0, { "kmz", "", "" }, { -1 }, 0 },
    { { 0x381E }, 0, { "sei", "", "" }, { 36 }, 0 },
    { { 0x581F }, 0, { "tob", "tom", "" }, { 37 }, 0 },
    { { 0x3820 }, 0, { "cho", "", "" }, { 39 }, 0 },
    { { 0x3821 }, 0, { "shg", "", "" }, { 40 }, 0 },
    { { 0x0022 }, 0, { "", "", "" }, { -1 }, 0 },
    { { 0x2023 }, 0, { "kni", "", "" }, { -1 }, 0 },
    { { 0x3024 }, 0, { "ayu", "", "" }, { 43 }, 0 },
    { { 0x3825 }, 0, { "boy", "", "" }, { 44 }, 0 },
    { { 0x3826 }, 0, { "sra", "", "" }, { 45 }, 0 },
    { { 0x2027 }, 0, { "sam", "", "" }, { -1 }, 0 },
    { { 0x3428 }, 0, { "daa", "", "" }, { 47 }, 0 },
};

/* sorts the characters met so far into the three tabs (rank 1..6 only) */
void cl_list_build(void)
{
    ClPair *buf[3];
    int n[3];
    int i;
    int num;
    int rank;
    int ok;
    int id;

    for (i = 0; i < 3; i++) {
        buf[i] = heap_alloc(cl.heap, 0x260);
        n[i] = 0;
    }
    num = 1;
    for (id = 2; id < 0x98; id++) {
        rank = 0;
        ok = 0;
        switch (value_to_tier(id)) {
        case 1:
        case 2:
            rank = cl_chara_find(id)->u.f.rank;
            break;
        default:
            charmap_lookup(id, &rank);
            break;
        }
        if (rank > 0 && rank < 7) {
            switch (cl.mode) {
            case 3:
                switch (deck_slot_get_level(id)) {
                case 3:
                case 2:
                    ok = 1;
                    break;
                case 1:
                    if (cl.sub == 2)
                        ok = 1;
                    break;
                }
                break;
            case 0:
            case 1:
            case 2:
                if (deck_slot_get(id)->count > 0)
                    ok = 1;
                break;
            }
            if (ok) {
                switch (value_to_tier(id)) {
                case 1:
                    buf[0][n[0]].num = num;
                    buf[0][n[0]++].id = id;
                    break;
                case 2:
                    buf[1][n[1]].num = num;
                    buf[1][n[1]++].id = id;
                    break;
                default:
                    buf[2][n[2]].num = num;
                    buf[2][n[2]++].id = id;
                    break;
                }
            }
            num++;
        }
    }
    for (i = 0; i < 3; i++) {
        if (n[i] == 0)
            cl.list[i] = 0;
        else
            cl.list[i] = heap_alloc(cl.heap, n[i] << 4);
    }
    cl.count[0] = cl_entries_build(cl.list[0], buf[0], n[0]);
    cl.count[1] = cl_entries_build(cl.list[1], buf[1], n[1]);
    cl.count[2] = cl_entries_simple(cl.list[2], buf[2], n[2]);
    for (i = 0; i < 3; i++)
        heap_set_level(cl.heap, buf[i]);
}

#ifdef NON_MATCHING /* não testado para matching; forma igual à do original (pseudocódigo + asm) */
/* list input: cancel leaves, confirm opens the detail page (or picks, in the random/labo modes) */
void cl_input(void)
{
    PadState *pad = pad_get();
    unsigned int trig = pad->b.trig;
    ClEntry *e;
    unsigned int top;
    unsigned int cur;
    unsigned int count;
    int pos;
    ClInfo *info;

    if (trig & region_pick_value(1)) {
        fade_start_in(1, 0xFF000000, 16);
        cl.state = 5;
        se_play(0x6214);
        switch (cl.mode) {
        case 0:
        case 1:
            bgm_cmd(600);
            break;
        case 2:
        case 3:
            cl.cancelled = 1;
            break;
        }
        return;
    }
    if (trig & region_pick_value(0)) {
        e = cl_entry(cl.tab, cl.top[cl.tab] + cl.cursor[cl.tab]);
        if (e == 0) {
            se_play(0x6215);
            return;
        }
        switch (cl.mode) {
        case 2:
            cl.costume = 0;
            cl.chosen = e->id;
            if (!e->simple) {
                info = e->u.info;
                if (info->cur < info->n) {
                    cl.costume = info->cur;
                    if (info->sel)
                        cl.costume = info->sel;
                }
            }
            cl.timer = 0;
            cl.state = 4;
            se_play(0x6213);
            break;
        case 3:
            cl.costume = 0;
            cl.chosen = e->id;
            cl.timer = 0;
            cl.state = 4;
            se_play(0x6213);
            break;
        case 0:
        case 1:
            if (!e->simple) {
                cl.state = 3;
                cl.step = 0;
                se_play(0x6213);
            }
            break;
        }
        return;
    }
    if (trig & 0x100) {
        cl.tab = (cl.tab + 2) % 3;
        se_play(0x621B);
        return;
    }
    if (trig & 0x8200) {
        cl.tab = (cl.tab + 1) % 3;
        se_play(0x621B);
        return;
    }
    cur = cl.cursor[cl.tab];
    top = cl.top[cl.tab];
    count = cl.count[cl.tab];
    pos = top + cur;
    e = pos < count ? &cl.list[cl.tab][pos] : 0;
    if (pad->b.rep & 0x10) {
        if (pos > 0) {
            if (cur > 0)
                cur--;
            else if (top > 0)
                top--;
            se_play(0x6212);
        } else if (trig & 0x10) {
            if (count < 5) {
                cur = count - 1;
            } else {
                cur = 3;
                top = count - 4;
            }
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x40) {
        if (pos < count - 1) {
            if (cur < 3)
                cur++;
            else
                top++;
            se_play(0x6212);
        } else if (trig & 0x40) {
            top = 0;
            cur = 0;
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x80) {
        if (e && !e->simple && e->u.info->cur > 0) {
            e->u.info->cur--;
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x20) {
        if (e && !e->simple && e->u.info->cur < e->u.info->n - 1) {
            e->u.info->cur++;
            se_play(0x6212);
        }
    }
    pos = top + cur;
    if (pos >= 0 && pos < count) {
        cl.top[cl.tab] = top;
        cl.cursor[cl.tab] = cur;
    }
}

/* detail page: slide in, theme of the duelist, costume change, profile scroll, voice test */
void cl_detail_update(void)
{
    ClEntry *e = cl_entry(cl.tab, cl.top[cl.tab] + cl.cursor[cl.tab]);
    PadState *pad = pad_get();
    int vol;
    int lo;
    int d;
    ClInfo *info;
    ClBgm *b;
    int set;
    unsigned int voice;

    if (cl.bgm_wait && snd_seq_get_u16(cl.bgm))
        cl.bgm_wait = 0;
    if (cl.vol_fade) {
        vol = cl.volume;
        lo = vol / 3;
        cl.timer++;
        if (cl.vol_fade == 2) {
            if (cl.timer < 13) {
                lo += cl.timer * (vol - lo) / 12;
                if (lo <= vol)
                    vol = lo;
            } else {
                cl.vol_fade = 0;
            }
            snd_set_volume(0, vol);
        } else if (cl.vol_fade == 1) {
            if (cl.timer < 13) {
                vol -= cl.timer * (vol - lo) / 12;
                if (lo <= vol)
                    lo = vol;
            } else {
                cl.vol_fade = 0;
            }
            snd_set_volume(0, lo);
        }
    }
    switch (cl.step) {
    case 0:
        if (cl.bgm_wait)
            return;
        cl.voice = 0;
        cl.timer = 0;
        cl.anim = 0;
        cl.slide = 0;
        cl_profile_measure(e);
        info = e->u.info;
        if (info->voice) {
            if (snd_count_se())
                return;
            set = info->bgm[info->cur];
            if (set == 0) {
                info->voice = 0;
            } else {
                b = &cl_bgm[set];
                if (cl.bgm != b->bgm) {
                    if (cl.bgm >= 0)
                        bgm_cmd3(0);
                    bgm_fade(b->bgm);
                    cl.bgm = b->bgm;
                    cl.bgm_wait = 1;
                    cl_voices_build(e, 0);
                }
            }
        }
        cl.step++;
        return;
    case 1:
        if (++cl.timer < 2)
            return;
        cl.timer = 0;
        cl.step++;
        return;
    case 2:
        if (++cl.anim < 12)
            return;
        cl.step = 3;
        cl.anim = 12;
        return;
    case 3:
        break;
    case 4:
        if (cl.anim != 0) {
            cl.anim--;
            return;
        }
        cl.anim--;
        snd_set_volume(0, cl.volume);
        cl.state = 2;
        cl.step = 0;
        cl.timer = 0;
        cl.anim = 0;
        cl.slide = 0;
        cl.vol_fade = 0;
        return;
    default:
        return;
    }
    switch (cl.fade) {
    case 0:
        break;
    case 1:
        if (cl.alpha < 0x31) {
            cl_costume_anim();
            cl.alpha = 0x20;
            cl.fade = 2;
        } else {
            cl.alpha -= 0x10;
        }
        break;
    case 2:
        if (cl.alpha < 0xEF) {
            cl.alpha += 0x10;
        } else {
            cl.alpha = 0xFF;
            cl.fade = 0;
        }
        break;
    default:
        cl.fade = 0;
        break;
    }
    if (pad->b.trig & region_pick_value(1)) {
        cl.step++;
        se_play(0x6214);
        if (cl.volume != cfg_get_byte3_b(0)) {
            cl.vol_fade = 2;
            cl.timer = 0;
        }
    } else if (cl.fade == 0 && (pad->b.rep & 0x8000)) {
        cl.fade = 1;
        cl.alpha = 0xEF;
        se_play(0x621B);
    } else {
        d = 0;
        if (pad->b.analog_y > 0x9E)
            d = (pad->b.analog_y - 0x9E) * 4;
        if (pad->b.analog_y < 0x62)
            d += (pad->b.analog_y - 0x62) * 4;
        if (pad->b.down & 0x10)
            d -= 0x80;
        if (pad->b.down & 0x40)
            d += 0x80;
        cl.scroll += d;
        if (cl.scroll <= 0)
            cl.scroll = 0;
        else if (cl.scroll >= cl.scroll_max)
            cl.scroll = cl.scroll_max;
    }
    info = e->u.info;
    if (info->voice) {
        set = info->bgm[info->cur];
        voice = cl.voice;
        if (cl.se_wait && snd_count_se() == 0) {
            se_play(cl.se_next);
            cl.se_wait = 0;
        }
        if (pad->b.trig & region_pick_value(0)) {
            if (voice < cl.n_voices) {
                snd_cmd_fef8();
                cl.se_wait = 0;
                se_play(cl_voice_se[cl.voices[voice]][0] + cl_bgm[set].se);
                if (cl_voice_se[cl.voices[voice]][1] >= 0) {
                    cl.se_next = cl_voice_se[cl.voices[voice]][1] + cl_bgm[set].se;
                    cl.se_wait = 1;
                }
                if (cl.volume == cfg_get_byte3_b(0)) {
                    cl.vol_fade = 1;
                    cl.timer = 0;
                }
            }
        } else if (pad->b.rep & 0x100) {
            if (voice < 10)
                voice = 0;
            else
                voice -= 10;
        } else if (pad->b.rep & 0x200) {
            if ((int)voice < cl.n_voices - 10)
                voice += 10;
            else
                voice = cl.n_voices - 1;
        } else if (pad->b.rep & 0x80) {
            if (voice != 0)
                voice--;
        } else if (pad->b.rep & 0x20) {
            if ((int)voice < cl.n_voices - 1)
                voice++;
        }
        if (cl.voice != voice) {
            cl.voice = voice;
            se_play(0x6212);
        }
    }
}

/* tabs 0 and 1: one ClInfo per duelist from the character table, with the story-dependent costumes */
unsigned short cl_entries_build(ClEntry *out, ClPair *src, unsigned short n)
{
    ClInfo *alexis = 0;
    ClInfo *zane = 0;
    ClInfo *crowler = 0;
    unsigned short count = 0;
    int k;
    ClPair *p = src;
    ClChara *c;
    ClInfo *info;
    unsigned int v;
    int id;
    int i;
    ClEntry *e;

    for (k = 0; k < n; k++, p++) {
        id = p->id;
        c = cl_chara_find(id);
        if (c == 0)
            continue;
        info = heap_alloc(cl.heap, 0x28);
        memset(info, 0, 0x28);
        v = c->u.f.n;
        switch (cl.mode) {
        case 2:
            if (c->u.f.voice) {
                if (v > 2)
                    v = 2;
            } else {
                v = 1;
            }
            break;
        case 3:
            v = 1;
            break;
        case 0:
        case 1:
            if (region_get() == 0 && deck_slot_get_level(id) == 3)
                info->voice = 1;
            break;
        }
        if (v != 0)
            info->bustup[0] = c->bustup[0];
        if (v > 1)
            info->bustup[1] = c->bustup[1];
        if (v > 2)
            info->bustup[2] = c->bustup[2];
        info->n = v;
        for (i = 0; i < (int)v; i++) {
            info->rank[i] = c->u.f.rank;
            info->text[i] = table_get_entry(cl_res_get(CL_STRINGS), cl_profile_text[id][0]);
            if (info->voice)
                info->bgm[i] = cl_voice_set[id][i];
        }
        if (cl_profile_text[id][1] != 0)
            info->text[1] = table_get_entry(cl_res_get(CL_STRINGS), cl_profile_text[id][1]);
        info->face = c->face.face;
        switch (id) {
        case 0xE:
            info->rank[1] = 6;
            break;
        case 0x10:
            crowler = info;
            break;
        case 0x18:
            zane = info;
            break;
        case 6:
            alexis = info;
            break;
        }
        e = &out[count];
        e->id = id;
        e->num = p->num;
        count++;
        e->simple = 0;
        e->u.info = info;
    }
    if (alexis) {
        if (sysflag_test(4) == 0) {
            alexis->bustup[0] = cl_chara_find(6)->bustup[1];
            alexis->rank[0] = 3;
            alexis->text[0] = alexis->text[1];
            if (alexis->voice)
                alexis->bgm[0] = alexis->bgm[1];
            alexis->face++;
            alexis->sel = 1;
            if (alexis->n > 1) {
                alexis->bustup[1] = 0;
                alexis->text[1] = 0;
            }
            alexis->n = 1;
        } else if (alexis->n > 1) {
            alexis->rank[1] = 3;
        }
    }
    if (zane && sysflag_test(5) == 0) {
        zane->bustup[0] = cl_chara_find(0x18)->bustup[1];
        zane->text[0] = zane->text[1];
        if (zane->voice)
            zane->bgm[0] = zane->bgm[1];
        zane->face++;
        zane->sel = 1;
        if (zane->n > 1) {
            zane->bustup[1] = 0;
            zane->text[1] = 0;
        }
        zane->n = 1;
    }
    if (crowler) {
        if (sysflag_test(6) == 0) {
            crowler->bustup[0] = cl_chara_find(0x10)->bustup[1];
            crowler->rank[0] = 6;
            crowler->text[0] = crowler->text[1];
            crowler->face++;
            crowler->sel = 1;
            if (crowler->n > 1) {
                crowler->bustup[1] = 0;
                crowler->text[1] = 0;
            }
            crowler->n = 1;
        } else if (crowler->n > 1) {
            crowler->rank[1] = 6;
        }
    }
    return count;
}
#endif

/* tab 2: the deck-only duelists (rank and card picture) */
unsigned short cl_entries_simple(ClEntry *out, ClPair *src, unsigned short n)
{
    unsigned short count = 0;
    int k = 0;
    ClPair *p;
    ClEntry *e;
    int id;
    int rank;

    if (k < n) {
        p = src;
        do {
            e = &out[count];
            id = p->id;
            e->id = id;
            e->num = p->num;
            e->simple = 1;
            e->u.s.card = charmap_lookup(id, &rank);
            e->u.s.rank = rank;
            count++;
            k++;
            p++;
        } while (k < n);
    }
    return count;
}

#ifdef NON_MATCHING /* não testado para matching */
/* next costume animation frame; Jaden and Chazz swap their theme with the costume */
void cl_costume_anim(void)
{
    ClEntry *e = cl_entry(cl.tab, cl.top[cl.tab] + cl.cursor[cl.tab]);
    void *body = cl_bustup_get(e, 0);
    int n;
    int f;
    int set;
    int alt;
    ClBgm *b;

    if (body == 0)
        return;
    n = ehf_get_frame_count(body);
    f = n + 15 - (n + 15) % 15;
    if (f >= ehf_get_data_ptr(body))
        f = 0;
    ehf_set_anim(body, f, f + 1);
    ehf_set_anim(cl_bustup_get(e, 1), f, f + 1);
    if (!e->u.info->voice)
        return;
    set = -1;
    alt = 0;
    switch (e->id) {
    case 0xD:
        if (e->u.info->bgm[0] == 0xD && f == 0x2D) {
            set = 0x29;
            alt = 1;
        } else if (e->u.info->bgm[0] == 0x29 && f != 0x2D) {
            set = 0xD;
        }
        break;
    case 0xC:
        if (e->u.info->bgm[0] == 0xC && f == 0x1E) {
            set = 0x2C;
            alt = 1;
        } else if (e->u.info->bgm[0] == 0x2C && f != 0x1E) {
            set = 0xC;
        }
        break;
    }
    if (set < 0)
        return;
    e->u.info->bgm[0] = set;
    snd_cmd_fef8();
    b = &cl_bgm[set];
    if (cl.bgm != b->bgm) {
        if (cl.bgm >= 0)
            bgm_cmd3(0);
        bgm_fade(b->bgm);
        cl.bgm = b->bgm;
        cl.bgm_wait = 1;
        cl_voices_build(e, alt);
    }
}
#endif

/* height of the profile text, for its scroll bar */
void cl_profile_measure(ClEntry *e)
{
    unsigned short flags = font_get_flags();
    int h;

    font_set_flag4(1);
    if (cfg_get_language() == 0) {
        font_set_flag8000(1);
        font_set_flag1(1);
    } else {
        font_set_flag8(1);
    }
    font_set_size(14, 14);
    h = font_wtext_lines(0x4180, e->u.info->text[e->u.info->cur]);
    font_set_flags(flags);
    h -= 0x2640;                /* the part that does not fit the window */
    if (h > 0)
        cl.scroll_max = h;
    else
        cl.scroll_max = 0;
    cl.scroll = 0;
}

#ifdef NON_MATCHING /* lógica conferida com o asm; 133 palavras: o original guarda k e o ponteiro da tabela
                       na pilha e n em s8; switch do id na ordem do original */
/* voice test list: the voice-test entries whose sound differs in this duelist's voice table */
void cl_voices_build(ClEntry *e, int alt)
{
    unsigned short id;
    unsigned short var;
    unsigned short *tbl;
    unsigned short n;
    int k;
    short *v;
    int i;
    short a, b, c, d;
    unsigned short la;

    if (e->simple)
        return;
    id = e->id;
    var = e->u.info->cur;
    switch (id) {
    case 0x12:
        if (var == 2)
            var = 1;
        break;
    case 0xC:
        if (alt)
            id = 0x2C;
        break;
    case 0xD:
        if (alt)
            id = 0x29;
        break;
    case 0xB:
    case 0xE:
    case 0x10:
    case 0x1F:
        var = 0;
        break;
    }
    tbl = cl_voicetbl_get(id, var);
    if (tbl == 0)
        return;
    n = 0;
    for (k = 0, v = cl_voice_se[0]; k < 0x7C; k++, v += 2) {
        a = v[0];
        b = v[1];
        if (partner_table_lookup(1, 0, a, tbl) == partner_table_lookup(1, 0, b, tbl))
            continue;
        for (i = 0; i < n; i++) {
            c = cl_voice_se[cl.voices[i]][0];
            d = cl_voice_se[cl.voices[i]][1];
            if (partner_table_lookup(1, 0, a, tbl) != partner_table_lookup(1, 0, c, tbl))
                continue;
            if (b == d)
                break;
            if (b == -1 || d == -1)
                continue;
            if (partner_table_lookup(1, 0, b, tbl) == partner_table_lookup(1, 0, d, tbl))
                break;
        }
        if (i == n)
            cl.voices[n++] = k;
    }
    cl.n_voices = n;
    cl.voice = 0;
}

/* portrait of a deck-only duelist: a generic head for the special ids, else head + body from the
   charmap */
unsigned int *cl_draw_portrait(unsigned int *list, unsigned short id, int x, int y, int scale, void *head, void *body)
{
    int generic = 0;
    int code;
    int u = 0;
    int v = 0;
    CharmapPos pos;

    if (head == 0 || body == 0)
        return list;
    code = charmap_lookup(id, 0);
    switch (code) {
    case 0x3F4:
        u = 0x33;
        v = 0xA8;
        break;
    case 0x3F3:
        v = 0xA8;
        break;
    case 0x3F2:
        u = 0xCC;
        v = 0x54;
        break;
    case 0x3F1:
        u = 0x99;
        v = 0x54;
        break;
    case 0x3F0:
        u = 0x66;
        v = 0x54;
        break;
    case 0x3EF:
        u = 0x33;
        v = 0x54;
        break;
    case 1000:
        v = 0x54;
        break;
    default:
        if (charmap_lookup2(code, &pos) == 0)
            return list;
        goto draw;
    }
    generic = 1;
draw:
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 1, 1);
    if (generic) {
        list = gim_set_texture(list, body);
        list = draw_set_offsets(list, 0x33, 0x54, scale * 0x33 / 100, scale * 0x54 / 100);
        return draw_sprite(list, x, y - scale * 6 / 100, 0, u, v, 0, 0);
    }
    list = gim_set_texture(list, body);
    list = draw_set_offsets(list, 0x33, 0x40, scale * 0x33 / 100, (scale << 6) / 100);
    list = draw_sprite(list, x, y + (scale << 4) / 100, 0, pos.b * 0x33, 0, 0, 0);
    list = gim_set_texture_sub(list, head, 0, 0, 0, pos.c);
    list = draw_set_offsets(list, 0x33, 0x32, scale * 0x33 / 100, scale * 0x32 / 100);
    return draw_sprite(list, x, y, 0, pos.a % 5 * 0x33, pos.a / 5 * 0x32, 0, 0);
}
#endif

ClChara *cl_chara_find(unsigned short id)
{
    int lo = 0;
    int hi = CL_CHARAS - 1;
    int mid;
    ClChara *c;
    int v;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        c = &cl_charas[mid];
        v = c->u.f.id;
        if (v == id)
            return c;
        if (v < id)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}
