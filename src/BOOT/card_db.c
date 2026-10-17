// @range 0x0884C8D0-0x0884EA38
// @data 0x088EEBA0-0x088EEF50
// @rodata 0x088AEAE0-0x088AEB84
// @asm 0x0884CB1C-0x0884D0FC
/* Deck recipes (RCP%03d.ydc in the recipes pack) and the card database: cardinfo_<lang>.ehp
   holds the names/descriptions (CARD_Name/Indx/Desc), properties (CARD_Prop.bin), passwords,
   sort keys, genre bits, the alphabetical index (CARD_Top) and alternate-art groups. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"
#include "card_db.h"

void *ehp_find(void *pack, const char *name, int flags);
void ehp_mount(int slot, void *pack);
int ehfile_get_size(int dir, const char *name);
void ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
unsigned short *table_get_entry(void *table, int i);
int cfg_get_language(void);
int sysvar_get_a8c(void);
int inv_get_limit(unsigned short id);         /* u16 results used unmasked here: declared int */
int card_limit_default(unsigned short id);
int sysflag_test(unsigned int bit);
void *model_load(void *data, int flags);
void model_anim_reset(void *model, void *anim);
void model_anim_play(void *model, int idx, int flags);
void model_set_anim_ptr(void *model, void *anim);
void model_update(void *model, void *dt);
void model_draw(void *model, void *params);
int u16_cmp(const void *a, const void *b);
static int card_db_loaded(void *arg, int size);

const char lang_codes[6][8] = { "jpn", "eng", "ger", "fra", "ita", "spa" };

unsigned short recipe_ids[0x160] = {
    1, 5, 6, 7, 8, 5, 9, 3, 4, 5, 6, 5, 9, 3, 4, 5,
    7, 6, 9, 4, 5, 6, 7, 5, 9, 5, 6, 7, 8, 5, 9, 4,
    5, 6, 7, 5, 9, 6, 7, 8, 9, 5, 10, 4, 5, 6, 7, 5,
    9, 3, 9, 0, 0, 0, 0, 8, 6, 7, 8, 5, 9, 8, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 6,
    0, 0, 0, 0, 0, 4, 6, 8, 0, 0, 0, 8, 9, 0, 0, 0,
    0, 8, 9, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 4, 0, 0,
    0, 0, 0, 3, 7, 0, 0, 0, 0, 3, 7, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
    0, 5, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 8, 8, 8, 8, 8, 0, 0, 0, 1, 1, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8,
    8, 8, 8, 0, 4, 4, 4, 4, 4, 8, 8, 0, 8, 8, 8, 8,
    8, 0, 4, 0, 8, 8, 8, 0, 8, 0, 8, 0, 0, 0, 0, 8,
    8, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* Recipe difficulty / tier id of recipe i (0 beyond the table). */
unsigned short recipe_get_id(int i)
{
    if (i < 0x160)
        return recipe_ids[i];
    return 0;
}

/* Name of recipe n from recipe_name_<l>.bin, or 0 when RCP<n>.ydc does not exist. */
unsigned short *recipe_get_name(int n)
{
    char name[32];
    char suffix[8] = "jegfis";

    sprintf(name, "RCP%03d.ydc", n);
    if (ehp_find(pack_recipes, name, 0) == 0)
        return 0;
    strcpy(name, "recipe_name_x.bin");
    name[12] = suffix[cfg_get_language()];
    return table_get_entry(ehp_find(pack_recipes, name, 0), n);
}

/* Reads RCP<n>.ydc (u16 count + ids for main, side, extra) into *out; zeroes it when invalid. */
void recipe_load(int n, RecipeData *out)
{
    char name[16];
    unsigned char *p;
    unsigned short *ids;
    unsigned int i;

    memset(out, 0, sizeof(RecipeData));
    sprintf(name, "RCP%03d.ydc", n);
    recipe_get_id(n);
    p = ehp_find(pack_recipes, name, 0);
    if (p == 0)
        return;
    ids = (unsigned short *)(p + 10);
    out->n_main = *(unsigned short *)(p + 8);
    if (out->n_main < 0x51) {
        for (i = 0; i < out->n_main; i++)
            out->main_ids[i] = *ids++;
        out->n_side = *ids++;
        if (out->n_side < 0x1F) {
            for (i = 0; i < out->n_side; i++)
                out->side_ids[i] = *ids++;
            out->n_extra = *ids++;
            if (out->n_extra < 0x10) {
                for (i = 0; i < out->n_extra; i++)
                    out->extra_ids[i] = *ids++;
                return;
            }
        }
    }
    memset(out, 0, sizeof(RecipeData));
}

int u16_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

/* Checks deck sizes and that the player owns enough copies of every card (partners' cards
   count once the flag is set). Returns 0 invalid, 1 valid, 2 valid using partner cards. */
/* Checks a deck against the inventory: 40..80 main cards, <=30 side, <=15 extra (when with_side),
   every card in game, copies owned (partner cards count once each) and per-card limit. 0 invalid,
   1 ok, 2 ok using partner cards. Structure matches; register allocation differs (s1/s2, s7/fp). */
#ifdef NON_MATCHING
int deck_validate(RecipeData *deck, int with_side)
{
    unsigned short ids[126];
    int n = 0;
    int used_partner = 0;
    unsigned int i;
    int k;
    unsigned short cur, run;
    unsigned short owned, limit;
    unsigned short id;
    int spare;

    if (deck->n_main < 0x28 || deck->n_main > 0x50)
        return 0;
    if (deck->n_side > 0x1E)
        return 0;
    if (with_side != 0 && deck->n_extra > 0xF)
        return 0;
    for (i = 0; i < deck->n_main; i++) {
        if (card_get_flag31(deck->main_ids[i]) != 0)
            ids[n++] = deck->main_ids[i];
    }
    for (i = 0; i < deck->n_side; i++) {
        if (card_get_flag31(deck->side_ids[i]) != 0)
            ids[n++] = deck->side_ids[i];
    }
    if (with_side != 0) {
        for (i = 0; i < deck->n_extra; i++) {
            if (card_get_flag31(deck->extra_ids[i]) != 0)
                ids[n++] = deck->extra_ids[i];
        }
    }
    qsort(ids, n, 2, u16_cmp);
    cur = 0;
    run = 0;
    spare = sysflag_test(0) != 0; /* one extra copy of a forbidden card allowed */
    for (k = 0; k < n; k++) {
        id = ids[k];
        if (cur == id) {
            run++;
            continue;
        }
        if (run > 0) {
            owned = inv_get_count(cur);
            limit = inv_get_limit(cur);
            if (owned < run) {
                for (i = 0; (int)i < partner_count_get(); i++) {
                    if (cur == partner_id_get(i)) {
                        used_partner = 1;
                        owned++;
                    }
                }
                if (owned < run)
                    return 0;
            }
            if (limit == 0) {
                spare -= run;
                if (spare < 0)
                    return 0;
            } else if (limit < run) {
                return 0;
            }
            run = 0;
        }
        cur = id;
        run++;
    }
    if (run > 0) {
        owned = inv_get_count(cur);
        limit = inv_get_limit(cur);
        if (owned < run) {
            for (i = 0; (int)i < partner_count_get(); i++) {
                if (cur == partner_id_get(i)) {
                    used_partner = 1;
                    owned++;
                }
            }
            if (owned < run)
                return 0;
        }
        if (limit == 0) {
            if (spare - run < 0)
                return 0;
        } else if (limit < run) {
            return 0;
        }
    }
    return used_partner ? 2 : 1;
}
#endif

/* For each partner card: 4 unused, 1 in main, 3 in side, 2 in extra (GameState +0x108). */
/* Marks each partner card: 1 missing from main, 3 missing from side, 2 missing from extra, 4 unused.
   Only the loop counter/pointer registers of the first loop differ (a1/a2). */
#ifdef NON_MATCHING
void gamestate_fill_partner_list(RecipeData *deck, int with_side)
{
    unsigned short *where = (unsigned short *)((char *)game_state_get() + 0x108);
    unsigned int j;
    int i, k;
    unsigned short id;
    unsigned short owned, used;

    memset(where, 4, 6);
    for (i = 0; i < partner_count_get(); i++) {
        id = partner_id_get(i);
        owned = inv_get_count(id);
        used = 0;
        for (k = 0; k < i; k++) {
            if (id == partner_id_get(k) && where[k] != 4)
                owned++;
        }
        for (j = 0; j < deck->n_main; j++) {
            if (deck->main_ids[j] == id)
                used++;
        }
        if (owned < used) {
            where[i] = 1;
            continue;
        }
        for (j = 0; j < deck->n_side; j++) {
            if (deck->side_ids[j] == id)
                used++;
        }
        if (owned < used) {
            where[i] = 3;
        } else if (with_side != 0) {
            for (j = 0; j < deck->n_extra; j++) {
                if (deck->extra_ids[j] == id)
                    used++;
            }
            if (owned < used)
                where[i] = 2;
        }
    }
}
#endif

/* Scene callback showing a card's 3D model (menu item kind 2); see the scene callback protocol. */
int cardinfo_model_cb(void **obj, void *scene, unsigned char *item, int msg)
{
    unsigned int anim[4];
    unsigned char params[0x34];
    void **p = (void **)(item + 0x10); /* {pos, rot, model name, anim1 name, anim2 name} */
    void *anim1;
    void *anim2;
    void *pack;
    void *data;

    if (item[3] < 3)
        return 0;
    switch (msg) {
    case 1:
        return 0xC;
    case 2:
        anim1 = 0;
        anim2 = 0;
        pack = *(void **)(*(int *)((char *)scene + 0xC) + 8);
        obj[0] = p[0];
        obj[1] = p[1];
        data = ehp_find(pack, p[2], 0);
        if (data == 0)
            return 0xC;
        if (item[3] >= 4 && p[3] != 0)
            anim1 = ehp_find(pack, p[3], 0);
        if (item[3] >= 5 && p[4] != 0)
            anim2 = ehp_find(pack, p[4], 0);
        obj[2] = model_load(data, 0);
        if (anim1 != 0) {
            model_anim_reset(obj[2], anim1);
            model_anim_play(obj[2], 0, 2);
            memset(anim, 0, sizeof anim);
            anim[0] = 0;
            model_update(obj[2], anim);
        }
        if (anim2 != 0)
            model_set_anim_ptr(obj[2], anim2);
        return 0xC;
    case 5:
        if (obj[2] == 0)
            return 0;
        return 0x80000000;
    case 6:
        memset(params, 0, sizeof params);
        *(void **)params = obj[0];
        *(void **)(params + 4) = obj[1];
        *(int *)(params + 0x24) = 1;
        model_draw(obj[2], params);
        model_update(obj[2], 0);
        return 0;
    }
    return 0;
}

/* Scene item type 0x26: a flag set/cleared by messages 0x101/0x102; "done" (msg 5) when clear. */
int cardinfo_flag_cb(void **obj, void *scene, unsigned char *item, int msg)
{
    switch (msg) {
    case 1:
        return 4;
    case 2:
        obj[0] = *(void **)(item + 0x10);
        return 4;
    case 5:
        return !obj[0];
    case 0x101:
        obj[0] = (void *)1;
        break;
    case 0x102:
        obj[0] = 0;
        break;
    }
    return 0;
}

/* Scene item type 0x27: like 0x26 but only reacts when the scene's current id matches item+0x10. */
int cardinfo_match_cb(int *obj, int **scene, unsigned char *item, int msg)
{
    switch (msg) {
    case 1:
        return 8;
    case 2:
        obj[0] = *(int *)(item + 0x10);
        obj[1] = *(int *)(item + 0x14);
        return 8;
    case 5:
        return !obj[1];
    case 0x101:
        if (obj[0] == **scene)
            obj[1] = 1;
        break;
    case 0x102:
        if (obj[0] == **scene)
            obj[1] = 0;
        break;
    }
    return 0;
}

CardDb *card_db_ptr = &card_db; /* .data slot between the recipe and the cardinfo literals */

/* Starts loading cardinfo_<lang>.ehp from directory 5 into card_db_buffer. */
void card_db_request_load(void)
{
    char name[32];
    int size;

    strcpy(name, "cardinfo_");
    strcat(name, lang_codes[cfg_get_language()]);
    strcat(name, ".ehp");
    size = ehfile_get_size(5, name);
    card_db_pack = card_db_buffer;
    card_db_ready = 0;
    ehfile_read_async_alias(5, name, card_db_buffer, size, card_db_loaded, card_db_buffer);
}

static int card_db_loaded(void *arg, int size)
{
    card_db_ready = 1;
    return 1;
}

/* Appends the language letter (J E G F I S) and ".bin" to a table name. */
void card_db_lang_suffix(char *name)
{
    char suffix[4];

    suffix[0] = lang_codes[cfg_get_language()][0] - 0x20;
    suffix[1] = 0;
    strcat(name, suffix);
    strcat(name, ".bin");
}

/* Mounts the loaded ehp and resolves every table. Returns 0 while the load is pending. */
int card_db_load(void)
{
    char name[32];
    void *pack;

    if (card_db_ready == 0)
        return 0;
    ehp_mount(0, card_db_pack);
    pack = card_db_pack;
    strcpy(name, "CARD_Name_");
    card_db_lang_suffix(name);
    card_db_ptr->name = ehp_find(pack, name, 0);
    card_db_ptr->unk08 = 0;
    strcpy(name, "CARD_Indx_");
    card_db_lang_suffix(name);
    card_db_ptr->index = ehp_find(pack, name, 0);
    card_db_ptr->unk10 = 0;
    strcpy(name, "CARD_Desc_");
    card_db_lang_suffix(name);
    card_db_ptr->desc = ehp_find(pack, name, 0);
    strcpy(name, "CARD_Sort_");
    card_db_lang_suffix(name);
    card_db_ptr->sort = ehp_find(pack, name, 0);
    strcpy(name, "CARD_Top_");
    card_db_lang_suffix(name);
    card_db_ptr->top = ehp_find(pack, name, 0);
    strcpy(name, "CARD_SamePict_");
    card_db_lang_suffix(name);
    card_db_ptr->samepict = ehp_find(pack, name, 0);
    strcpy(name, "DLG_Indx_");
    card_db_lang_suffix(name);
    card_db_ptr->dlg_index = ehp_find(pack, name, 0);
    strcpy(name, "DLG_Text_");
    card_db_lang_suffix(name);
    card_db_ptr->dlg_text = ehp_find(pack, name, 0);
    card_db_ptr->prop = ehp_find(pack, "CARD_Prop.bin", 0);
    card_db_ptr->password = ehp_find(pack, "CARD_Pass.bin", 0);
    card_db_ptr->int_id = ehp_find(pack, "CARD_IntID.bin", 0);
    card_db_ptr->genre = ehp_find(pack, "CARD_Genre.bin", 0);
    return 1;
}

int card_index_from_id2(int id)
{
    return card_index_from_id(id);
}

/* Table index of card id (ids start at 4000); 0 below that. */
int card_index_from_id(int id)
{
    if (id < 4000)
        return 0;
    return *(unsigned short *)((char *)card_db_ptr->int_id + id * 2 - 8000);
}

unsigned short *card_get_name(int id)
{
    CardDb *db = card_db_ptr;
    int i = card_index_from_id(id);

    return (unsigned short *)(db->name + db->index[i].name_off);
}

#pragma explicit_zero_data on /* zero-initialized, yet in .data in the original */
unsigned short card_empty_text[2] = { 0, 0 };
#pragma explicit_zero_data off

unsigned short *card_empty_text_get(void)
{
    return card_empty_text;
}

unsigned short *card_get_desc(int id)
{
    CardDb *db = card_db_ptr;
    int i = card_index_from_id(id);

    return (unsigned short *)(db->desc + *(unsigned int *)((char *)db->index + i * 8 + 4));
}

CardProp *card_prop(int id)
{
    return card_db_ptr->prop + card_index_from_id(id);
}

/* 0xFFFF when the card is in the game (bit 7 of byte 3), else 0. */
int card_get_flag31(int id)
{
    return card_prop(id)->in_game * 0xFFFF;
}

int card_get_sort(int id)
{
    return card_db_ptr->sort[card_index_from_id(id)];
}

unsigned int card_get_prop6_hi(int id)
{
    return card_prop(id)->prop6_hi;
}

unsigned int card_get_password(int id)
{
    return card_db_ptr->password[card_index_from_id(id)];
}

/* Card id for an 8-digit password (-1 when unknown). */
int card_id_from_password(unsigned int pass)
{
    int id = 0xFA7;

    if (pass == 0)
        return -1;
    do {
        if (card_get_password(id) == pass)
            return id;
        id++;
    } while (id < 0x1B2E);
    return -1;
}

unsigned short *dlg_get_text(int i)
{
    return (unsigned short *)(card_db_ptr->dlg_text + card_db_ptr->dlg_index[i]);
}

unsigned short *card_db_table_sel(int i)
{
    if (i != 0)
        return dlg_get_text(i + 500);
    return card_empty_text;
}

unsigned short *card_db_table_sel2(int i)
{
    if (i != 0)
        return dlg_get_text(i + 0x226);
    return card_empty_text;
}

unsigned short *dlg_table_sel(int i)
{
    if (i != 0)
        return dlg_get_text(i + 0x212);
    return card_empty_text;
}

unsigned short *dlg_table_sel2(int i)
{
    if (i != 0)
        return dlg_get_text(i + 0x23A);
    return card_empty_text;
}

unsigned int card_get_type(int id)
{
    return card_prop(id)->type;
}

int card_get_kind(int id)
{
    return card_prop(id)->kind;
}

unsigned int card_get_level(int id)
{
    return card_prop(id)->level;
}

unsigned int card_get_prop5_hi(int id)
{
    return card_prop(id)->prop5_hi;
}

unsigned int card_get_attribute(int id)
{
    return card_prop(id)->attribute;
}

int card_get_atk(int id)
{
    unsigned int v = card_prop(id)->atk;

    if (v == 0x1FF)
        return 0;
    return v * 10;
}

int card_get_def(int id)
{
    unsigned int v = card_prop(id)->def;

    if (v == 0x1FF)
        return 0;
    return v * 10;
}

/* Same as card_get_atk but "?" (0x1FF) stays 0xFFFF. */
int card_get_atk_q(int id)
{
    unsigned int v = card_prop(id)->atk;

    if (v == 0x1FF)
        return 0xFFFF;
    return v * 10;
}

int card_get_def_q(int id)
{
    unsigned int v = card_prop(id)->def;

    if (v == 0x1FF)
        return 0xFFFF;
    return v * 10;
}

/* Copies allowed per deck: the inventory's limit, or the default table in the alternate mode. */
int card_get_limit(int id)
{
    if (sysvar_get_a8c() != 0)
        return card_limit_default(id);
    return inv_get_limit(id);
}

unsigned short *dlg_get_text_bounded(int i)
{
    if (i < 0)
        return card_empty_text;
    if (i > 0x24)
        return card_empty_text;
    return dlg_get_text(i + 0x2BD);
}

void card_get_genre(int id, unsigned short *out)
{
    unsigned short *g = (unsigned short *)((char *)card_db_ptr->genre + card_index_from_id(id) * 6);

    out[0] = g[0];
    out[1] = g[1];
    out[2] = g[2];
}

int card_has_genre_bit(int id, unsigned int bit)
{
    unsigned short *g = (unsigned short *)((char *)card_db_ptr->genre + card_index_from_id(id) * 6);
    unsigned short w = g[(int)bit >> 4];

    return (w & (1 << (bit & 0xF))) != 0;
}

unsigned short card_top_count(void)
{
    return card_db_ptr->top[0];
}

unsigned short card_top_get_letter(unsigned short i)
{
    if (card_top_count() < i)
        return 0;
    return *(unsigned short *)((char *)card_db_ptr->top + i * 4 + 2);
}

unsigned short card_top_get_index(unsigned short i)
{
    if (card_top_count() < i)
        return 0;
    return *(unsigned short *)((char *)card_db_ptr->top + i * 4 + 4);
}

/* Letter group (CARD_Top entry) the card sorts into, -1 before the first. */
int card_get_sort_position(int id)
{
    int key = card_get_sort(id);
    int n = (unsigned short)card_top_count();
    int i;

    for (i = 0; i < n; i++) {
        if (key < card_get_sort(card_top_get_index(i)))
            break;
    }
    return i - 1;
}

/* Alternate-art picture number of the card (1 when it has none, 0 without the table). */
unsigned short card_get_samepict(unsigned short id)
{
    unsigned short *t = card_db_ptr->samepict;
    unsigned short r = 1;
    unsigned short idx = card_index_from_id(id);
    unsigned short *e;
    int lo, hi, mid;

    if (t == 0)
        return 0;
    e = t + 1; /* pairs {index, group} sorted by index; count in t[0] */
    hi = t[0] - 1;
    lo = 0;
    while (lo <= hi) {
        mid = (lo + hi) / 2;
        if (idx == e[mid * 2]) {
            r += e[mid * 2 + 1];
            break;
        }
        if (e[mid * 2] < idx)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return r;
}

/* Icon index for the attribute: 0 none/divine, 1 light-ish, 2 dark/earth, 3 water/fire, 4.. spells/traps. */
int card_get_attribute_icon(int id)
{
    switch (card_get_attribute(id)) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 3;
    case 3: return 3;
    case 4: return 2;
    case 5: return 2;
    case 6: return 1;
    case 7: return 1;
    case 8: return 1;
    case 9: return 0;
    case 10: return 4;
    case 11: return 5;
    case 12: return 6;
    case 13: return 7;
    case 14: return 8;
    case 15: return 9;
    default: return 0;
    }
}

/* Next card of an alternate-art cycle (Harpie Lady, Toon..., Dark Magician...). */
int card_id_remap(int id)
{
    int r;

    switch (id) {
    case 0xFA6: r = 0xFA7; break;
    case 0xFA7: r = 0xFA6; break;
    case 0x12E5: r = 0x1303; break;
    case 0x1303: r = 0x12E5; break;
    case 0xFC9: r = 0x142D; break;
    case 0x142D: r = 0xFC9; break;
    case 0xFE4: r = 0x1477; break;
    case 0x1477: r = 0x182A; break;
    case 0x182A: r = 0x182B; break;
    case 0x182B: r = 0x182C; break;
    case 0x182C: r = 0xFE4; break;
    case 0x10F4: r = 0x150B; break;
    case 0x150B: r = 0x10F4; break;
    case 0x14F9: r = 0x154F; break;
    case 0x154F: r = 0x1550; break;
    case 0x1550: r = 0x1551; break;
    case 0x1551: r = 0x14F9; break;
    case 0x1730: r = 0x1731; break;
    case 0x1731: r = 0x1730; break;
    case 0x1670: r = 0x1671; break;
    case 0x1671: r = 0x1672; break;
    case 0x1672: r = 0x1670; break;
    default: r = id; break;
    }
    return r;
}

/* Nonzero when a and b are the same card or alternate arts of each other. */
int card_same_name(int a, int b)
{
    if (a == b)
        return 1;
    switch (a) {
    case 0xFA6: case 0xFA7:
        if ((unsigned int)(b - 0xFA6) < 2)
            return 1;
        break;
    case 0x12E5: case 0x1303:
        if (b == 0x12E5 || b == 0x1303)
            return 1;
        break;
    case 0xFC9: case 0x142D:
        if (b == 0xFC9 || b == 0x142D)
            return 1;
        break;
    case 0xFE4: case 0x1477: case 0x182A: case 0x182B: case 0x182C:
        if (b == 0xFE4 || b == 0x1477 || (unsigned int)(b - 0x182A) < 3)
            return 1;
        break;
    case 0x10F4: case 0x150B:
        if (b == 0x10F4 || b == 0x150B)
            return 1;
        break;
    case 0x14F9: case 0x154F: case 0x1550: case 0x1551:
        if (b == 0x14F9 || (unsigned int)(b - 0x154F) < 3)
            return 1;
        break;
    case 0x1730: case 0x1731:
        if ((unsigned int)(b - 0x1730) < 2)
            return 1;
        break;
    case 0x1670: case 0x1671: case 0x1672:
        if ((unsigned int)(b - 0x1670) < 3)
            return 1;
        break;
    }
    return 0;
}

int card_is_monster(int id)
{
    int r = id != 0;

    if (r)
        r = card_get_kind(id) < 8;
    return r;
}

int card_is_attribute0(int id)
{
    return card_get_attribute(id) == 0;
}

/* 0 for monster attributes 0/2/4/9, 1 for the others. */
int card_attribute_to_group(int id)
{
    switch (card_get_attribute(id)) {
    case 1: case 3: case 5: case 6: case 7: case 8: case 10: case 11: case 12:
        return 1;
    default: /* 0, 2, 4, 9 */
        return 0;
    }
}

int card_attribute_icon_is3(int id)
{
    return card_get_attribute_icon(id) == 3;
}

int card_attribute_icon_is2(int id)
{
    return card_get_attribute_icon(id) == 2;
}

int card_is_attribute9(int id)
{
    return card_get_attribute(id) == 9;
}

int card_is_attribute6(int id)
{
    return card_get_attribute(id) == 6;
}

int card_is_attribute7(int id)
{
    return card_get_attribute(id) == 7;
}

int card_is_attribute8(int id)
{
    return card_get_attribute(id) == 8;
}

/* Hard-coded card families used by the story (archetype checks). */
int card_is_in_group_1566(int id)
{
    switch (id) {
    case 0x1123: case 0x127D: case 0x127E: case 0x127F: case 0x12A5: case 0x12BE: case 0x154A:
    case 0x1561: case 0x1562: case 0x1563: case 0x1564: case 0x1565: case 0x1566:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_1911(int id)
{
    switch (id) {
    case 0xFBC: case 0x1070: case 0x107F: case 0x10AB: case 0x10D6: case 0x127F: case 0x12B5:
    case 0x13E3: case 0x1441: case 0x14B7: case 0x14CD: case 0x14DA: case 0x165A: case 0x1682:
    case 0x168C: case 0x168D: case 0x168E: case 0x168F: case 0x1690: case 0x1691: case 0x1692:
    case 0x169D: case 0x16A4: case 0x1911:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_158d(int id)
{
    switch (id) {
    case 0x131D: case 0x1585: case 0x1586: case 0x1587: case 0x1588: case 0x1589: case 0x158A:
    case 0x158B: case 0x158C: case 0x158D:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_170b(int id)
{
    switch (id) {
    case 0xFFE: case 0x111C: case 0x11A7: case 0x1266: case 0x1452: case 0x164A: case 0x164B:
    case 0x164C: case 0x164D: case 0x164E: case 0x164F: case 0x18B0: case 0x18BA:
    case 0x152E: case 0x170B: /* appended after the sorted list: tests run in reverse source order */
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_16a3(int id)
{
    switch (id) {
    case 0x143F: case 0x1440: case 0x1531: case 0x161E: case 0x1656: case 0x1685: case 0x1686:
    case 0x169E: case 0x16A3:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_160f(int id)
{
    switch (id) {
    case 0x143E: case 0x144B: case 0x14A4: case 0x14A6: case 0x14AB: case 0x14AF: case 0x14B0:
    case 0x160D: case 0x160E: case 0x160F:
        return 1;
    default:
        return 0;
    }
}

int card_is_in_group_18be(int id)
{
    switch (id) {
    case 0x117B: case 0x16B8: case 0x16B9: case 0x1754: case 0x17DF: case 0x18BE:
        return 1;
    default:
        return 0;
    }
}
