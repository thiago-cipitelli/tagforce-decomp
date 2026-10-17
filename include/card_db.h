/* Card database (cardinfo_<lang>.ehp) and deck recipes (RCP%03d.ydc). BOOT 0x0884C8D0.. */
#ifndef CARD_DB_H
#define CARD_DB_H

#include "game.h"

typedef struct CardIndex {             /* CARD_Indx_<l>.bin[idx] */
    unsigned int name_off;             /* into CARD_Name_<l>.bin */
    unsigned int desc_off;             /* into CARD_Desc_<l>.bin */
} CardIndex;

typedef struct CardProp {              /* CARD_Prop.bin[idx], 8 bytes */
    unsigned int unk0 : 13;
    unsigned int atk : 9;              /* x10; 0x1FF = "?" */
    unsigned int def : 9;
    unsigned int in_game : 1;
    unsigned int attribute : 4;        /* 0..9 monster attributes, 10+ spell/trap kinds */
    unsigned int kind : 4;             /* < 8 monster */
    unsigned int level : 4;
    unsigned int prop5_hi : 3;
    unsigned int type : 5;
    unsigned int prop6_hi : 3;
    unsigned int pad : 1;
    unsigned int byte7 : 8;
} CardProp;

typedef struct CardDb {                /* 0x3C, in .bss; the ehp stays mounted in card_db_buffer */
    int unk00;
    unsigned char *name;               /* +0x04 UCS-2 names blob */
    int unk08;
    CardIndex *index;                  /* +0x0C */
    int unk10;
    unsigned char *desc;               /* +0x14 */
    unsigned short *sort;              /* +0x18 sort key per index */
    unsigned int *dlg_index;           /* +0x1C */
    unsigned char *dlg_text;           /* +0x20 */
    unsigned short *genre;             /* +0x24 3 x u16 bits per index */
    unsigned short *top;               /* +0x28 count, then {letter, index} pairs */
    unsigned short *samepict;          /* +0x2C count, then {index, alt} pairs sorted by index */
    CardProp *prop;                    /* +0x30 */
    unsigned int *password;            /* +0x34 */
    unsigned short *int_id;            /* +0x38 id - 4000 -> index */
} CardDb;

extern CardDb *card_db_ptr;
extern CardDb card_db;
extern int card_db_ready;
extern void *card_db_pack;
extern unsigned char card_db_buffer[];
extern unsigned char pack_recipes[];
extern unsigned short recipe_ids[0x160];

unsigned short recipe_get_id(int i);
unsigned short *recipe_get_name(int n);
void recipe_load(int n, RecipeData *out);
int deck_validate(RecipeData *deck, int with_side);
void gamestate_fill_partner_list(RecipeData *deck, int with_side);
void card_db_request_load(void);
void card_db_lang_suffix(char *name);
int card_db_load(void);
int card_index_from_id2(int id);
int card_index_from_id(int id);
unsigned short *card_get_name(int id);
unsigned short *card_empty_text_get(void);
unsigned short *card_get_desc(int id);
CardProp *card_prop(int id);
int card_get_flag31(int id);
int card_get_sort(int id);
unsigned int card_get_prop6_hi(int id);
unsigned int card_get_password(int id);
int card_id_from_password(unsigned int pass);
unsigned short *dlg_get_text(int i);
unsigned short *card_db_table_sel(int i);
unsigned short *card_db_table_sel2(int i);
unsigned short *dlg_table_sel(int i);
unsigned short *dlg_table_sel2(int i);
unsigned int card_get_type(int id);
int card_get_kind(int id);
unsigned int card_get_level(int id);
unsigned int card_get_prop5_hi(int id);
unsigned int card_get_attribute(int id);
int card_get_atk(int id);
int card_get_def(int id);
int card_get_atk_q(int id);
int card_get_def_q(int id);
int card_get_limit(int id);
unsigned short *dlg_get_text_bounded(int i);
void card_get_genre(int id, unsigned short *out);
int card_has_genre_bit(int id, unsigned int bit);
unsigned short card_top_count(void);
unsigned short card_top_get_letter(unsigned short i);
unsigned short card_top_get_index(unsigned short i);
int card_get_sort_position(int id);
unsigned short card_get_samepict(unsigned short id);
int card_get_attribute_icon(int id);
int card_id_remap(int id);
int card_same_name(int a, int b);
int card_is_monster(int id);
int card_is_attribute0(int id);
int card_attribute_to_group(int id);
int card_attribute_icon_is3(int id);
int card_attribute_icon_is2(int id);
int card_is_attribute9(int id);
int card_is_attribute6(int id);
int card_is_attribute7(int id);
int card_is_attribute8(int id);
int card_is_in_group_1566(int id);
int card_is_in_group_1911(int id);
int card_is_in_group_158d(int id);
int card_is_in_group_170b(int id);
int card_is_in_group_16a3(int id);
int card_is_in_group_160f(int id);
int card_is_in_group_18be(int id);

#endif
