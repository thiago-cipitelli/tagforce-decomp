// @range 0x08804AD0-0x088054F8
// @data 0x08809664-0x0880A5EC
/* Labo voice test (last entry of the debug menu): picks character / sub id / pack / voice, plays the
   matching BGM or voice, and shows the character name and the line from labo/voicetest.ehp. */
#include "ehsys.h"

void *memset(void *, int, unsigned int);
char *strcpy(char *, const char *);
int sprintf(char *, const char *, ...);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_file_alias(int dir, const char *name, void *buf, int size);
unsigned short region_pick_value(int flag);
void snd_cmd_fefb(void);
void bgm_fade(int x);
void se_play(int id);
unsigned short *chara_get_name(int i, int j);
void *ehp_find(void *pack, const char *name, int *size);
unsigned short partner_table_lookup(int flag, unsigned short mode, unsigned short idx, unsigned short *table);
unsigned short *table_get_entry(void *table, unsigned int i);
void font_set_size(int w, int h);
unsigned int font_set_color(unsigned int c);
void font_printf(int x, int y, const char *fmt, ...);
int font_draw_text(int x, int y, int w, unsigned short *text);
int font_draw_wtext_ex(int x, int y, int w, int h, unsigned short *text, int max_glyphs);

typedef struct VoiceTest {
    unsigned short row;    /* selected line: 0 chr, 1 sub, 2 pack, 3 voice */
    unsigned short chr;    /* character id; >= 50 are the generic students/teachers */
    unsigned short sub;
    unsigned short pack;
    unsigned short voice;
    unsigned short unused[3];
    void *ehp;             /* labo/voicetest.ehp, loaded into voicetest_buf */
} VoiceTest;

extern VoiceTest voicetest;        /* .bss 0x0890A610 */
extern char voicetest_buf[];       /* .bss 0x0880A610, 1 MB */


void voicetest_load(void)
{
    memset(&voicetest, 0, sizeof voicetest);
    voicetest.ehp = voicetest_buf;
    ehfile_read_file_alias(3, "labo/voicetest.ehp", voicetest.ehp, ehfile_get_size(3, "labo/voicetest.ehp"));
}

/* per voice set: {bgm id, se base, ?}; only the bgm id is read here */
unsigned short voicetest_bgm_table[47][3] = {
    {0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0},
    {0x5D, 0x152, 0x1E1},
    {0x60, 0x302, 0x391},
    {0x63, 0x4B2, 0x541},
    {0x66, 0x662, 0x6F1},
    {0x69, 0x812, 0x8A1},
    {0x6C, 0x9C2, 0xA51},
    {0x6F, 0xB72, 0xC01},
    {0x72, 0xD22, 0xDB1},
    {0x75, 0xED2, 0xF61},
    {0x78, 0x1082, 0x1111},
    {0x7B, 0x1232, 0x12C1},
    {0x7E, 0x13E2, 0x1471},
    {0x81, 0x1592, 0x1621},
    {0x84, 0x1742, 0x17D1},
    {0x87, 0x18F2, 0x1981},
    {0x8A, 0x1AA2, 0x1B31},
    {0x8D, 0x1C52, 0x1CE1},
    {0x90, 0x1E02, 0x1E91},
    {0x93, 0x1FB2, 0x2041},
    {0x96, 0x2162, 0x21F1},
    {0x99, 0x2312, 0x23A1},
    {0x9C, 0x24C2, 0x2551},
    {0x9F, 0x2672, 0x2701},
    {0xA2, 0x2822, 0x28B1},
    {0xA5, 0x29D2, 0x2A61},
    {0xA8, 0x2B82, 0x2C11},
    {0xAB, 0x2D32, 0x2DC1},
    {0xAE, 0x2EE2, 0x2F71},
    {0xB1, 0x3092, 0x3121},
    {0xB4, 0x3242, 0x32D1},
    {0xB7, 0x33F2, 0x3481},
    {0xBA, 0x35A2, 0x3631},
    {0xBD, 0x3752, 0x37E1},
    {0xC0, 0x3902, 0x3991},
    {0xC3, 0x3AB2, 0x3B41},
    {0xC6, 0x3C62, 0x3CF1},
    {0xC9, 0x3E12, 0x3EA1},
    {0xCC, 0x3FC2, 0x4051},
    {0xCF, 0x4172, 0x4201},
    {0xD2, 0x4322, 0x43B1},
    {0xD5, 0x44D2, 0x4561},
    {0xD8, 0x4682, 0x4711},
    {0xDB, 0x4832, 0x48C1},
    {0xDE, 0x49E2, 0x4A71},
    {0xE1, 0x4B92, 0x4C21},
};
/* [chr][sub] -> row of voicetest_bgm_table (0 = none) */
unsigned short voicetest_bgm_index[41][3] = {
    {0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0},
    {0x2, 0x0, 0x0},
    {0x3, 0x0, 0x0},
    {0x4, 0x0, 0x0},
    {0x5, 0x0, 0x0},
    {0x2E, 0x6, 0x0},
    {0x7, 0x0, 0x0},
    {0x8, 0x0, 0x0},
    {0x9, 0x0, 0x0},
    {0xA, 0x0, 0x0},
    {0xB, 0xB, 0x0},
    {0xC, 0x0, 0x0},
    {0xD, 0x0, 0x0},
    {0xE, 0xE, 0x0},
    {0xF, 0x0, 0x0},
    {0x10, 0x10, 0x0},
    {0x11, 0x0, 0x0},
    {0x2A, 0x12, 0x12},
    {0x13, 0x0, 0x0},
    {0x14, 0x0, 0x0},
    {0x15, 0x0, 0x0},
    {0x16, 0x0, 0x0},
    {0x17, 0x0, 0x0},
    {0x18, 0x2B, 0x0},
    {0x19, 0x0, 0x0},
    {0x1A, 0x0, 0x0},
    {0x1B, 0x0, 0x0},
    {0x1C, 0x0, 0x0},
    {0x0, 0x0, 0x0},
    {0x1E, 0x0, 0x0},
    {0x1F, 0x1F, 0x0},
    {0x20, 0x0, 0x0},
    {0x21, 0x0, 0x0},
    {0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0},
    {0x24, 0x0, 0x0},
    {0x25, 0x0, 0x0},
    {0x26, 0x0, 0x0},
    {0x0, 0x0, 0x0},
    {0x28, 0x0, 0x0},
};
char *voicetest_formats[4] = {
    "ChrID : %03d - ", /* ChrID : %03d -  */
    "SubID : %1d", /* SubID : %1d */
    "Pack : %s", /* Pack : %s */
    "VOICE : %03d : %s", /* VOICE : %03d : %s */
};
char *voicetest_pack_names[3] = {
    "\x82P\x82\x96\x82\x93\x82P", /* １ｖｓ１ */
    "\x8a\xcf\x90\xed", /* 観戦 */
    "\x83^\x83" "b\x83O", /* タッグ */
};
/* generic students/teachers (chr >= 50, 5 per kind) */
char *voicetest_npc_formats[6] = {
    "\x92j\x8eq\x90\xb6\x93kNOM%02d", /* 男子生徒NOM%02d */
    "\x92j\x8eq\x90\xb6\x93kCOL%02d", /* 男子生徒COL%02d */
    "\x92j\x8eq\x90\xb6\x93kHOT%02d", /* 男子生徒HOT%02d */
    "\x8f\x97\x8eq\x90\xb6\x93kNOM%02d", /* 女子生徒NOM%02d */
    "\x8f\x97\x8eq\x90\xb6\x93kCOL%02d", /* 女子生徒COL%02d */
    "\x8f\x97\x8eq\x90\xb6\x93kTIM%02d", /* 女子生徒TIM%02d */
};
/* voice labels: named characters (144 voices) and generic ones (53) */
char *voicetest_voice_names[144] = {
    "\x8aJ\x96\x8b\x82\xcc\x8c\xfb\x8f\xe3", /* 開幕の口上 */
    "\x83" "f\x83\x85\x83G\x83\x8b\x83R\x81[\x83\x8b", /* デュエルコール */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T1_1", /* ターンコール:余裕1_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T1_2", /* ターンコール:余裕1_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T2_1", /* ターンコール:余裕2_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T2_2", /* ターンコール:余裕2_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T3_1", /* ターンコール:余裕3_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T3_2", /* ターンコール:余裕3_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T4_1", /* ターンコール:余裕4_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T4_2", /* ターンコール:余裕4_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "1_1", /* ターンコール:通常1_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "1_2", /* ターンコール:通常1_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "2_1", /* ターンコール:通常2_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "2_2", /* ターンコール:通常2_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "3_1", /* ターンコール:通常3_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "3_2", /* ターンコール:通常3_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "4_1", /* ターンコール:通常4_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "4_2", /* ターンコール:通常4_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`1_1", /* ターンコール:ピンチ1_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`1_2", /* ターンコール:ピンチ1_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`2_1", /* ターンコール:ピンチ2_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`2_2", /* ターンコール:ピンチ2_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`3_1", /* ターンコール:ピンチ3_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`3_2", /* ターンコール:ピンチ3_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`4_1", /* ターンコール:ピンチ4_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`4_2", /* ターンコール:ピンチ4_2 */
    "\x83J\x81[\x83h\x83h\x83\x8d\x81[1", /* カードドロー1 */
    "\x83J\x81[\x83h\x83h\x83\x8d\x81[2", /* カードドロー2 */
    "\x83J\x81[\x83h\x83h\x83\x8d\x81[3", /* カードドロー3 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O:\x92\xca\x8f\xed" "1", /* 効果発動前:通常1 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O:\x92\xca\x8f\xed" "2", /* 効果発動前:通常2 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O:\x92\xca\x8f\xed" "3", /* 効果発動前:通常3 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O:\x92\xca\x8f\xed" "4", /* 効果発動前:通常4 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "1", /* 効果発動前：割込1 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "2", /* 効果発動前：割込2 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "3", /* 効果発動前：割込3 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "4", /* 効果発動前：割込4 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "5", /* 効果発動前：割込5 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "6", /* 効果発動前：割込6 */
    "\x8e\xe8\x8e" "D\x82\xa9\x82\xe7", /* 手札から */
    "\x96\x82\x96@\x94\xad\x93\xae" "1", /* 魔法発動1 */
    "\x96\x82\x96@\x94\xad\x93\xae" "2", /* 魔法発動2 */
    "\x96\x82\x96@\x94\xad\x93\xae" "3", /* 魔法発動3 */
    "\x96\x82\x96@\x94\xad\x93\xae" "4", /* 魔法発動4 */
    "\x91\xac\x8dU\x96\x82\x96@1", /* 速攻魔法1 */
    "\x91\xac\x8dU\x96\x82\x96@2", /* 速攻魔法2 */
    "\x91\xac\x8dU\x96\x82\x96@3", /* 速攻魔法3 */
    "\x89i\x91\xb1\x96\x82\x96@1", /* 永続魔法1 */
    "\x89i\x91\xb1\x96\x82\x96@2", /* 永続魔法2 */
    "\x89i\x91\xb1\x96\x82\x96@3", /* 永続魔法3 */
    "\x91\x95\x94\xf5\x96\x82\x96@1", /* 装備魔法1 */
    "\x91\x95\x94\xf5\x96\x82\x96@2", /* 装備魔法2 */
    "\x91\x95\x94\xf5\x96\x82\x96@3", /* 装備魔法3 */
    "\x8bV\x8e\xae\x96\x82\x96@1", /* 儀式魔法1 */
    "\x8bV\x8e\xae\x96\x82\x96@2", /* 儀式魔法2 */
    "\x8bV\x8e\xae\x96\x82\x96@3", /* 儀式魔法3 */
    "\xe3\xa9" "1", /* 罠1 */
    "\xe3\xa9" "2", /* 罠2 */
    "\xe3\xa9" "3", /* 罠3 */
    "\x89i\x91\xb1\xe3\xa9" "1", /* 永続罠1 */
    "\x89i\x91\xb1\xe3\xa9" "2", /* 永続罠2 */
    "\x89i\x91\xb1\xe3\xa9" "3", /* 永続罠3 */
    "\x83J\x83" "E\x83\x93\x83^\x81[\xe3\xa9" "1", /* カウンター罠1 */
    "\x83J\x83" "E\x83\x93\x83^\x81[\xe3\xa9" "2", /* カウンター罠2 */
    "\x83J\x83" "E\x83\x93\x83^\x81[\xe3\xa9" "3", /* カウンター罠3 */
    "\x83\x8a\x83o\x81[\x83X\x83I\x81[\x83v\x83\x93" "1", /* リバースオープン1 */
    "\x83\x8a\x83o\x81[\x83X\x83I\x81[\x83v\x83\x93" "2", /* リバースオープン2 */
    "\x83\x8a\x83o\x81[\x83X\x83I\x81[\x83v\x83\x93" "3", /* リバースオープン3 */
    "\x83\x82\x83\x93\x83X\x83^\x81[\x8c\xf8\x89\xca\x94\xad\x93\xae" "1", /* モンスター効果発動1 */
    "\x83\x82\x83\x93\x83X\x83^\x81[\x8c\xf8\x89\xca\x94\xad\x93\xae" "2", /* モンスター効果発動2 */
    "\x83\x82\x83\x93\x83X\x83^\x81[\x8c\xf8\x89\xca\x94\xad\x93\xae" "3", /* モンスター効果発動3 */
    "\x8b\xad\x97~\x82\xc8\x92\xd9", /* 強欲な壷 */
    "\x93V\x8eg\x82\xcc\x8e{\x82\xb5", /* 天使の施し */
    "\x83t\x83" "B\x81[\x83\x8b\x83h\x96\x82\x96@", /* フィールド魔法 */
    "\x8e\xe5\x97\xcd\x96\x82\x96@", /* 主力魔法 */
    "\x8f\xa2\x8a\xab\x91O1", /* 召喚前1 */
    "\x8f\xa2\x8a\xab\x91O2", /* 召喚前2 */
    "\x8f\xa2\x8a\xab\x91O3", /* 召喚前3 */
    "\x8f\xa2\x8a\xab\x91O4", /* 召喚前4 */
    "\x92\xca\x8f\xed\x8f\xa2\x8a\xab" "1", /* 通常召喚1 */
    "\x92\xca\x8f\xed\x8f\xa2\x8a\xab" "2", /* 通常召喚2 */
    "\x92\xca\x8f\xed\x8f\xa2\x8a\xab" "3", /* 通常召喚3 */
    "\x93\xc1\x8e\xea\x8f\xa2\x8a\xab" "1", /* 特殊召喚1 */
    "\x93\xc1\x8e\xea\x8f\xa2\x8a\xab" "2(\x8e\xe8\x8e" "D)", /* 特殊召喚2(手札) */
    "\x93\xc1\x8e\xea\x8f\xa2\x8a\xab" "3(\x83" "f\x83" "b\x83L)", /* 特殊召喚3(デッキ) */
    "\x93\xc1\x8e\xea\x8f\xa2\x8a\xab" "4(\x95\xe6\x92n)", /* 特殊召喚4(墓地) */
    "\x90\xb6\xe6\xd1\x83R\x81[\x83\x8b" "1", /* 生贄コール1 */
    "\x90\xb6\xe6\xd1\x83R\x81[\x83\x8b" "2", /* 生贄コール2 */
    "\x90\xb6\xe6\xd1\x83R\x81[\x83\x8b" "3", /* 生贄コール3 */
    "\x97Z\x8d\x87\x8f\xa2\x8a\xab" "1", /* 融合召喚1 */
    "\x97Z\x8d\x87\x8f\xa2\x8a\xab" "2", /* 融合召喚2 */
    "\x97Z\x8d\x87\x8f\xa2\x8a\xab" "3", /* 融合召喚3 */
    "\x8bV\x8e\xae\x8f\xa2\x8a\xab" "1", /* 儀式召喚1 */
    "\x8bV\x8e\xae\x8f\xa2\x8a\xab" "2", /* 儀式召喚2 */
    "\x8bV\x8e\xae\x8f\xa2\x8a\xab" "3", /* 儀式召喚3 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "1_1", /* 主力召喚1_1 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "1_2", /* 主力召喚1_2 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "2_1", /* 主力召喚2_1 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "2_2", /* 主力召喚2_2 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "3_1", /* 主力召喚3_1 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "3_2", /* 主力召喚3_2 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "4_1", /* 主力召喚4_1 */
    "\x8e\xe5\x97\xcd\x8f\xa2\x8a\xab" "4_2", /* 主力召喚4_2 */
    "\x8dU\x8c\x82" "1", /* 攻撃1 */
    "\x8dU\x8c\x82" "2", /* 攻撃2 */
    "\x92\xbc\x90\xda\x8dU\x8c\x82" "1", /* 直接攻撃1 */
    "\x92\xbc\x90\xda\x8dU\x8c\x82" "2", /* 直接攻撃2 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O1", /* 主力攻撃前1 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O2", /* 主力攻撃前2 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O3", /* 主力攻撃前3 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O4", /* 主力攻撃前4 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O5", /* 主力攻撃前5 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O6", /* 主力攻撃前6 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O7", /* 主力攻撃前7 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O8", /* 主力攻撃前8 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82" "1", /* 主力攻撃1 */
    "\x8bZ\x96\xbc", /* 技名 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82" "2", /* 主力攻撃2 */
    "\x8bZ\x96\xbc", /* 技名 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82" "3", /* 主力攻撃3 */
    "\x8bZ\x96\xbc", /* 技名 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82" "4", /* 主力攻撃4 */
    "\x8bZ\x96\xbc", /* 技名 */
    "\x83J\x81[\x83h\x83Z\x83" "b\x83g", /* カードセット */
    "\x83\x82\x83\x93\x83X\x83^\x81[\x83Z\x83" "b\x83g", /* モンスターセット */
    "\x83^\x81[\x83\x93\x83G\x83\x93\x83h1", /* ターンエンド1 */
    "\x83^\x81[\x83\x93\x83G\x83\x93\x83h2", /* ターンエンド2 */
    "\x83^\x81[\x83\x93\x83G\x83\x93\x83h3", /* ターンエンド3 */
    "\x83_\x83\x81\x81[\x83W\x8f\xac", /* ダメージ小 */
    "\x83_\x83\x81\x81[\x83W\x92\x86", /* ダメージ中 */
    "\x83_\x83\x81\x81[\x83W\x91\xe5", /* ダメージ大 */
    "\x82\xd0\x82\xe9\x82\xdd", /* ひるみ */
    "\x83_\x83\x81\x81[\x83W\x8c\xe3" "1", /* ダメージ後1 */
    "\x83_\x83\x81\x81[\x83W\x8c\xe3" "2", /* ダメージ後2 */
    "\x83_\x83\x81\x81[\x83W\x8c\xe3" "3", /* ダメージ後3 */
    "\x83g\x83h\x83\x81\x8b\xf2\x82\xe7\x82\xa4", /* トドメ喰らう */
    "\x83R\x83X\x83g\x91\xcf\x82\xa6(\x8c\xf8\x89\xca\x8e\xf3\x82\xaf\x81j", /* コスト耐え(効果受け） */
    "\x8f\x9f\x82\xbf", /* 勝ち */
    "\x95\x89\x82\xaf", /* 負け */
    "\x88\xf8\x82\xab\x95\xaa\x82\xaf", /* 引き分け */
    "\x83o\x83g\x83\x8b\x83t\x83" "F\x81[\x83Y\x93\xcb\x93\xfc" "1", /* バトルフェーズ突入1 */
    "\x83o\x83g\x83\x8b\x83t\x83" "F\x81[\x83Y\x93\xcb\x93\xfc" "2", /* バトルフェーズ突入2 */
    "\x83o\x83g\x83\x8b\x83t\x83" "F\x81[\x83Y\x93\xcb\x93\xfc" "3", /* バトルフェーズ突入3 */
    "\x8b\xc1\x82\xad", /* 驚く */
};
char *voicetest_npc_voice_names[53] = {
    "\x8aJ\x96\x8b\x82\xcc\x8c\xfb\x8f\xe3", /* 開幕の口上 */
    "\x83" "f\x83\x85\x83G\x83\x8b\x83R\x81[\x83\x8b", /* デュエルコール */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T1_1", /* ターンコール:余裕1_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x97]\x97T1_2", /* ターンコール:余裕1_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "1_1", /* ターンコール:通常1_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x92\xca\x8f\xed" "1_2", /* ターンコール:通常1_2 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`1_1", /* ターンコール:ピンチ1_1 */
    "\x83^\x81[\x83\x93\x83R\x81[\x83\x8b:\x83s\x83\x93\x83`1_2", /* ターンコール:ピンチ1_2 */
    "\x83J\x81[\x83h\x83h\x83\x8d\x81[1", /* カードドロー1 */
    "\x83J\x81[\x83h\x83h\x83\x8d\x81[2", /* カードドロー2 */
    "\x83J\x81[\x83h\x83h\x83\x8d\x81[3", /* カードドロー3 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O:\x92\xca\x8f\xed" "1", /* 効果発動前:通常1 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "1", /* 効果発動前：割込1 */
    "\x8c\xf8\x89\xca\x94\xad\x93\xae\x91O\x81" "F\x8a\x84\x8d\x9e" "2", /* 効果発動前：割込2 */
    "\x8e\xe8\x8e" "D\x82\xa9\x82\xe7", /* 手札から */
    "\x96\x82\x96@\x94\xad\x93\xae" "1", /* 魔法発動1 */
    "\x96\x82\x96@\x94\xad\x93\xae" "2", /* 魔法発動2 */
    "\x96\x82\x96@\x94\xad\x93\xae" "3", /* 魔法発動3 */
    "\x91\xac\x8dU\x96\x82\x96@1", /* 速攻魔法1 */
    "\x89i\x91\xb1\x96\x82\x96@1", /* 永続魔法1 */
    "\x91\x95\x94\xf5\x96\x82\x96@1", /* 装備魔法1 */
    "\x8bV\x8e\xae\x96\x82\x96@1", /* 儀式魔法1 */
    "\xe3\xa9" "1", /* 罠1 */
    "\x89i\x91\xb1\xe3\xa9" "1", /* 永続罠1 */
    "\x83J\x83" "E\x83\x93\x83^\x81[\xe3\xa9" "1", /* カウンター罠1 */
    "\x83\x8a\x83o\x81[\x83X\x83I\x81[\x83v\x83\x93" "1", /* リバースオープン1 */
    "\x83\x82\x83\x93\x83X\x83^\x81[\x8c\xf8\x89\xca\x94\xad\x93\xae" "1", /* モンスター効果発動1 */
    "\x8b\xad\x97~\x82\xc8\x92\xd9", /* 強欲な壷 */
    "\x93V\x8eg\x82\xcc\x8e{\x82\xb5", /* 天使の施し */
    "\x83t\x83" "B\x81[\x83\x8b\x83h\x96\x82\x96@", /* フィールド魔法 */
    "\x8f\xa2\x8a\xab\x91O1", /* 召喚前1 */
    "\x92\xca\x8f\xed\x8f\xa2\x8a\xab" "1", /* 通常召喚1 */
    "\x93\xc1\x8e\xea\x8f\xa2\x8a\xab" "1", /* 特殊召喚1 */
    "\x90\xb6\xe6\xd1\x83R\x81[\x83\x8b" "1", /* 生贄コール1 */
    "\x97Z\x8d\x87\x8f\xa2\x8a\xab" "1", /* 融合召喚1 */
    "\x8bV\x8e\xae\x8f\xa2\x8a\xab" "1", /* 儀式召喚1 */
    "\x8dU\x8c\x82" "1", /* 攻撃1 */
    "\x92\xbc\x90\xda\x8dU\x8c\x82" "1", /* 直接攻撃1 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O1", /* 主力攻撃前1 */
    "\x8e\xe5\x97\xcd\x8dU\x8c\x82\x91O5", /* 主力攻撃前5 */
    "\x83J\x81[\x83h\x83Z\x83" "b\x83g", /* カードセット */
    "\x83\x82\x83\x93\x83X\x83^\x81[\x83Z\x83" "b\x83g", /* モンスターセット */
    "\x83^\x81[\x83\x93\x83G\x83\x93\x83h1", /* ターンエンド1 */
    "\x83_\x83\x81\x81[\x83W\x8f\xac", /* ダメージ小 */
    "\x83_\x83\x81\x81[\x83W\x92\x86", /* ダメージ中 */
    "\x83_\x83\x81\x81[\x83W\x91\xe5", /* ダメージ大 */
    "\x82\xd0\x82\xe9\x82\xdd", /* ひるみ */
    "\x83g\x83h\x83\x81\x8b\xf2\x82\xe7\x82\xa4", /* トドメ喰らう */
    "\x83R\x83X\x83g\x91\xcf\x82\xa6(\x8c\xf8\x89\xca\x8e\xf3\x82\xaf\x81j", /* コスト耐え(効果受け） */
    "\x8f\x9f\x82\xbf", /* 勝ち */
    "\x95\x89\x82\xaf", /* 負け */
    "\x88\xf8\x82\xab\x95\xaa\x82\xaf", /* 引き分け */
    "\x83o\x83g\x83\x8b\x83t\x83" "F\x81[\x83Y\x93\xcb\x93\xfc" "1", /* バトルフェーズ突入1 */
};

/* language suffix of the wordstbl files, indexed by cfg_get_language() */
char voicetest_lang_chars[] = "jegfis";

/* number of values of a row */
inline int voicetest_row_count(unsigned short row)
{
    int n = 0;

    switch (row) {
    case 0:
        n = 0x53;
        break;
    case 1:
        n = 3;
        break;
    case 2:
        n = 3;
        break;
    case 3:
        if (voicetest.chr < 50)
            n = 0x90;
        else
            n = 0x35;
        break;
    }
    return n;
}

void voicetest_play_bgm(void)
{
    unsigned short bgm = 0;

    snd_cmd_fefb();
    if (voicetest.chr >= 50) {
        bgm = (voicetest.chr - 50) * 3 + 228;
    } else if (voicetest.chr > 40) {
        bgm = (voicetest.chr - 2) * 3 + 93;
    } else {
        int k = voicetest_bgm_index[voicetest.chr][voicetest.sub];
        if (k)
            bgm = voicetest_bgm_table[k][0];
        if (voicetest.chr == 18 && voicetest.sub == 2)
            bgm = 222;
    }
    if (bgm > 0)
        bgm_fade(bgm + voicetest.pack);
}

int voicetest_update(void)
{
    if (pad_get()->b.trig & region_pick_value(0)) {
        unsigned short id;
        if (voicetest.chr >= 50) {
            id = (voicetest.pack + (voicetest.chr - 50) * 3) * 53 + 19778;
        } else {
            unsigned short c = voicetest.chr;
            switch (c) {
            case 6:
                if (voicetest.sub == 0)
                    c = 46;
                break;
            case 18:
                if (voicetest.sub == 0)
                    c = 42;
                else if (voicetest.sub == 2)
                    c = 45;
                break;
            case 24:
                if (voicetest.sub == 1)
                    c = 43;
                break;
            }
            id = (voicetest.pack + (c - 2) * 3) * 144 + 338;
        }
        se_play(id + voicetest.voice);
    } else if (pad_get()->b.trig & region_pick_value(1)) {
        return 1;
    } else if (pad_get()->b.trig & 0x1000) {
        voicetest_play_bgm();
    } else if (pad_get()->b.trig & 0x100) {
        (&voicetest.chr)[voicetest.row] = 0;
    } else if (pad_get()->b.trig & 0x200) {
        unsigned short row = voicetest.row;
        (&voicetest.chr)[row] = voicetest_row_count(row) - 1;
    } else if (pad_get()->b.rep & 0x80) {
        if ((&voicetest.chr)[voicetest.row] > 0)
            (&voicetest.chr)[voicetest.row]--;
    } else if (pad_get()->b.rep & 0x20) {
        unsigned short row = voicetest.row;
        int n = voicetest_row_count(row);
        if ((&voicetest.chr)[row] < n - 1)
            (&voicetest.chr)[row]++;
    } else if (pad_get()->b.rep & 0x10) {
        if (voicetest.row > 0)
            voicetest.row--;
        else
            voicetest.row = 3;
    } else if (pad_get()->b.rep & 0x40) {
        if (voicetest.row < 3)
            voicetest.row++;
        else
            voicetest.row = 0;
    }
    return 0;
}

/* character name: special variants by text, generic students/teachers by format, else the game's name */
void voicetest_draw_name(void)
{
    char buf[1024];
    int chr = voicetest.chr;
    unsigned short sub = voicetest.sub;
    int text = 0;

    if (chr >= 2) {
        if (chr == 6) {
            if (sub == 1) {
                strcpy(buf, "\x96\x9c\x8f\xe4\x96\xda\x81@\x8f\x80(\x83u\x83\x8b\x81[)"); /* 万丈目　準(ブルー) */
                text = 1;
            }
        } else if (chr == 18) {
            if (sub == 1) {
                strcpy(buf, "\x89" "e\x8a\xdb(\x98V\x90l\x81" "E\x83t\x83\x8a\x81[)"); /* 影丸(老人・フリー) */
                text = 1;
            } else if (sub == 2) {
                strcpy(buf, "\x89" "e\x8a\xdb(\x98V\x90l\x81" "E\x83X\x83g\x81[\x83\x8a\x81[)"); /* 影丸(老人・ストーリー) */
                text = 1;
            }
        } else if (chr == 24) {
            if (sub == 1) {
                strcpy(buf, "\x91\x81\x89\xb3\x8f\x97\x81@\x83\x8c\x83" "C(\x92j\x91\x95)"); /* 早乙女　レイ(男装) */
                text = 1;
            }
        } else if (chr == 41) {
            strcpy(buf, "\x83^\x83j\x83\x84(\x8a\xc3)"); /* タニヤ(甘) */
            text = 1;
        } else if (chr == 44) {
            strcpy(buf, "\x83J\x83~\x83\x85\x81[\x83\x89(\x93{)"); /* カミューラ(怒) */
            text = 1;
        } else if (chr >= 80) {
            switch (chr) {
            case 80:
                strcpy(buf, "\x8b\xb3\x8etNOM"); /* 教師NOM */
                break;
            case 81:
                strcpy(buf, "\x8b\xb3\x8etCOL"); /* 教師COL */
                break;
            case 82:
                strcpy(buf, "\x8b\xb3\x8etHOT"); /* 教師HOT */
                break;
            default:
                return;
            }
            text = 1;
        } else if (chr >= 50) {
            sprintf(buf, voicetest_npc_formats[(chr - 50) / 5], (chr - 50) % 5);
            text = 1;
        } else if (chr > 40) {
            return;
        }
        if (text)
            font_printf(197, 10, buf);
        else
            font_draw_text(12608, 640, 18112, chara_get_name(0, chr));
    }
}

/* the selected voice's line: voice id -> line index (voiceidchgtbl), line text (wordstbl, per language) */
void voicetest_draw_line(void)
{
    char buf[256];
    unsigned short *chg;
    void *words;
    unsigned short line;

    sprintf(buf, "chr%03d%02d_voiceidchgtbl.bin", voicetest.chr, voicetest.sub);
    chg = ehp_find(voicetest.ehp, buf, 0);
    sprintf(buf, "chr%03d%02d_wordstbl_%c.bin", voicetest.chr, voicetest.sub, voicetest_lang_chars[cfg_get_language()]);
    words = ehp_find(voicetest.ehp, buf, 0);
    if (chg == 0 || words == 0)
        return;
    line = partner_table_lookup(voicetest.chr < 50, voicetest.pack, voicetest.voice, chg);
    sprintf(buf, "\x91\xe4\x8e\x8cID : %d", line - 1); /* 台詞ID : %d */
    font_printf(100, 90, buf);
    font_draw_wtext_ex(3200, 7680, 24320, 8320, table_get_entry(words, line), -1);
}

void voicetest_draw(void)
{
    char buf[1024];

    font_set_color(0xFFFFFFFF);
    font_set_size(14, 14);
    font_printf(80, voicetest.row * 20 + 10, "\x81\x9e"); /* ◇ */
    sprintf(buf, voicetest_formats[0], voicetest.chr);
    font_printf(100, 10, buf);
    sprintf(buf, voicetest_formats[1], voicetest.sub);
    font_printf(100, 30, buf);
    sprintf(buf, voicetest_formats[2], voicetest_pack_names[voicetest.pack]);
    font_printf(100, 50, buf);
    if (voicetest.chr < 50)
        sprintf(buf, voicetest_formats[3], voicetest.voice, voicetest_voice_names[voicetest.voice]);
    else
        sprintf(buf, voicetest_formats[3], voicetest.voice, voicetest_npc_voice_names[voicetest.voice]);
    font_printf(100, 70, buf);
    voicetest_draw_name();
    voicetest_draw_line();
    font_printf(10, 250,
                "\x81\x9b\x81" "F\x8d\xc4\x90\xb6\x81@\x81\xa2\x81" "FSE\x83p\x83" "b\x83N\x83\x8d\x81[\x83h\x81@\x81~\x81" "F\x96\xdf\x82\xe9"); /* ○：再生　△：SEパックロード　×：戻る */
}
