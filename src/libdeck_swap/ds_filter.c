// @range 0x0880D23C-0x0880DFFC
// @rodata 0x08819CA0-0x08819CBC
// @asm 0x0880D3F8-0x0880D6EC
// @asm 0x0880DC94-0x0880DF44
// @data 0x0881A1F8-0x0881A224
/* The incremental text search of a pane and its word history, the card filters (kind, type, race,
   level ranges, spell/trap kinds, limit, genre/new/alternate picture) and the text match: a
   Boyer-Moore-Horspool scan over the normalised card name and text (same source as libselect_card
   sc_filter.c and sc_match.c). */
#include "libdeck_swap.h"

typedef struct DsSearch {              /* ds_search 0x0881A40C */
    unsigned short pos;                /* entries scanned */
    unsigned short total;              /* +0x02 */
    unsigned short n;                  /* +0x04 matches */
    unsigned short pad06;
    unsigned short *out;               /* +0x08 */
    unsigned short *word;              /* +0x0C */
} DsSearch;

typedef int (*DsFilterFn)(unsigned short id, unsigned int arg);

extern DsSearch ds_search;

int wcslen(const unsigned short *s);
int wcscmp(const unsigned short *a, const unsigned short *b);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int inv_get_state(unsigned short id);
void card_get_genre(int id, unsigned short *out);
void wtext_strip_tags(unsigned short *dst, unsigned short *src);
void wtext_strip_tags2(unsigned short *dst, unsigned short *src);
unsigned short *card_get_desc(int id);
unsigned short towupper(unsigned short c);
unsigned short ds_id_count(unsigned short id, DsIdList *l);
unsigned short *ds_find_text(unsigned short *text, unsigned short *word, int n);
void ds_fold_text(unsigned short *dst, unsigned short *src);
int ds_text_match(unsigned short id, unsigned short *word);
int ds_filter_attr(DsEnt *e, unsigned int *arg);
int ds_filter_kind(unsigned short id, unsigned int mask);
int ds_filter_type(unsigned short id, unsigned int mask);
int ds_filter_race(unsigned short id, unsigned int mask);
int ds_filter_level(unsigned short id, unsigned int mask);
int ds_filter_atk(unsigned short id, unsigned int mask);
int ds_filter_def(unsigned short id, unsigned int mask);

/* the incremental search over the visible order (16 entries a frame) */
void ds_search_start(DsList *l, unsigned short *word)
{
    memset(&ds_search, 0, 0x10);
    ds_search.total = l->count;
    ds_search.word = word;
    if (ds_search.total > 0)
        ds_search.out = heap_alloc(ds.heap, ds_search.total << 1);
}

/* progress 0..1000 */
unsigned short ds_search_step(DsList *l)
{
    int i;
    unsigned short k;

    if (ds_search.out == 0)
        return 1000;
    if (ds_search.pos >= ds_search.total)
        return 1000;
    for (i = 0; i < 16 && ds_search.pos + i < ds_search.total; i++) {
        k = l->order[ds_search.pos + i];
        if (ds_text_match(l->ents[k].id, ds_search.word))
            ds_search.out[ds_search.n++] = k;
    }
    ds_search.pos += i;
    return ds_search.pos * 1000 / ds_search.total;
}

#ifdef NON_MATCHING /* 16 palavras: len, ponteiro do histórico e k em $s em rodízio (original s2/s0/s1), como dk_search_finish */
/* the matches become the visible order; the word goes on top of the history */
void ds_search_finish(DsList *l)
{
    int len;
    int i;
    int k;

    if (ds_search.out != 0) {
        memcpy(l->order, ds_search.out, ds_search.n << 1);
        l->count = ds_search.n;
        heap_set_level(ds.heap, ds_search.out);
        ds_search.out = 0;
    }
    l->filter = 8;
    len = wcslen(ds_search.word);
    k = 7;
    for (i = 0; i < 8; i++) {
        if (ds.words[i][0] == 0) {
            k = i;
            break;
        }
        if (len == wcslen(ds.words[i])) {
            if (wcscmp(ds.words[i], ds_search.word) == 0) {
                k = i;
                break;
            }
        }
    }
    for (; k > 0; k--)
        wcscpy(ds.words[k], ds.words[k - 1]);
    wcscpy(ds.words[0], ds_search.word);
}
#endif

DsFilterFn ds_filters[9] = {
    0, (DsFilterFn)ds_filter_kind, (DsFilterFn)ds_filter_type, (DsFilterFn)ds_filter_race,
    (DsFilterFn)ds_filter_level, (DsFilterFn)ds_filter_atk, (DsFilterFn)ds_filter_def, 0,
};

#ifdef NON_MATCHING /* 24 palavras: o original carrega e->id e calcula e (em a1) antes do teste do filtro, com ok = 1 no atraso do beqz; como dk_list_filter */
/* keeps the entries passing filter f (arg its parameter); returns the count */
unsigned short ds_list_filter(DsList *l, unsigned short f, unsigned int *arg)
{
    unsigned short *tmp = heap_alloc(ds.heap, l->count << 1);
    int n;
    int i;
    unsigned short k;
    DsEnt *e;
    int ok;

    if (tmp == 0)
        return l->count;
    n = 0;
    for (i = 0; i < l->count; i++) {
        k = l->order[i];
        e = &l->ents[k];
        ok = ds_filters[f] != 0 ? ds_filters[f](e->id, *arg) : f == 7 ? ds_filter_attr(e, arg) : 1;
        if (ok) {
            tmp[n++] = k;
        }
    }
    memcpy(l->order, tmp, n << 1);
    l->count = n;
    l->filter = f;
    if (tmp != 0)
        heap_set_level(ds.heap, tmp);
    return n;
}
#endif

int ds_filter_kind(unsigned short id, unsigned int mask)
{
    return (mask & 1 << ds_kind_icon(card_get_attribute_icon(id))) != 0;
}

int ds_filter_type(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_type(id))
            return 1;
    }
    return 0;
}

int ds_filter_race(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_kind(id))
            return 1;
    }
    return 0;
}

void ds_fold_text(unsigned short *dst, unsigned short *src);

/* level bits of the three level groups (1-4, 5-6, 7+) */
unsigned short ds_level_masks[3] = { 0x1E, 0x60, 0xFF80 };

int ds_filter_level(unsigned short id, unsigned int mask)
{
    unsigned short bits = 0;
    unsigned short level = card_get_level(id);

    if (mask & 1)
        bits |= ds_level_masks[0];
    if (mask & 2)
        bits |= ds_level_masks[1];
    if (mask & 4)
        bits |= ds_level_masks[2];
    if (card_is_monster(id) && (bits & 1 << level))
        return 1;
    return 0;
}

/* spell and trap kinds */
int ds_filter_atk(unsigned short id, unsigned int mask)
{
    switch (card_get_attribute_icon(id)) {
    case 7:
        switch (card_get_prop5_hi(id)) {
        case 0:
            if (mask & 0x40)
                return 1;
            break;
        case 2:
            if (mask & 2)
                return 1;
            break;
        case 3:
            if (mask & 4)
                return 1;
            break;
        case 4:
            if (mask & 8)
                return 1;
            break;
        case 5:
            if (mask & 0x10)
                return 1;
            break;
        case 6:
            if (mask & 0x20)
                return 1;
            break;
        }
        break;
    case 8:
        switch (card_get_prop5_hi(id)) {
        case 0:
            if (mask & 0x200)
                return 1;
            break;
        case 1:
            if (mask & 0x80)
                return 1;
            break;
        case 4:
            if (mask & 0x100)
                return 1;
            break;
        }
        break;
    }
    return 0;
}

int ds_filter_def(unsigned short id, unsigned int mask)
{
    return (mask & 1 << card_get_limit(id)) != 0;
}


/* genre bits, the "new" cards and the cards with an alternate picture */
int ds_filter_attr(DsEnt *e, unsigned int *arg)
{
    unsigned short g[3];

    if ((arg[0] & 1) && inv_get_state(e->id) == 1)
        return 1;
    if ((arg[1] & 8) && inv_get_bit3(e->id) && ds_id_count(e->id, res_get(0x8032)) > 0)
        return 1;
    card_get_genre(e->id, g);
    if (g[0] & (arg[0] & 0xFFFF))
        return 1;
    if (g[1] & (unsigned short)(arg[0] >> 16))
        return 1;
    return (g[2] & (unsigned short)arg[1]) != 0;
}

/* word found in the card's name or text (with and without ruby) */
int ds_text_match(unsigned short id, unsigned short *word)
{
    unsigned short txt[0x800];
    unsigned short raw[0x800];
    unsigned short w[0x40];
    unsigned short n = wcslen(word);
    unsigned short *s;

    ds_fold_text(w, word);
    s = card_get_name(id);
    wtext_strip_tags(raw, s);
    ds_fold_text(txt, raw);
    if (ds_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    ds_fold_text(txt, raw);
    if (ds_find_text(txt, w, n))
        return 1;
    s = card_get_desc(id);
    wtext_strip_tags(raw, s);
    ds_fold_text(txt, raw);
    if (ds_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    ds_fold_text(txt, raw);
    return ds_find_text(txt, w, n) != 0;
}

#ifdef NON_MATCHING /* 2 palavras: o original põe "j = n - 1" (move t2,a3) no atraso do bne do primeiro caractere; como dk_find_text */
/* Horspool search of word (n chars) in text */
unsigned short *ds_find_text(unsigned short *text, unsigned short *word, int n)
{
    int i;
    int len;
    int j;
    int k;
    unsigned short *t;
    unsigned short *w;

    unsigned char *skip = heap_alloc(ds.heap, 0x10000);

    if (skip == 0)
        return 0;
    memset(skip, n, 0x10000);
    for (i = 0; i < n - 1; i++)
        skip[word[i]] = n - i - 1;
    len = 0;
    if (*text != 0) {
        t = text;
        do {
            t++;
            len++;
        } while (*t != 0);
    }
    for (k = n - 1; k < len; ) {
        t = &text[k];
        j = n - 1;
        w = &word[j];
        if (*t == *w) {
            do {
                if (j == 0) {
                    if (skip != 0)
                        heap_set_level(ds.heap, skip);
                    return &text[k];
                }
                t--;
                w--;
                k--;
                j--;
            } while (*t == *w);
        }
        if (skip[text[k]] > n - j)
            k += skip[text[k]];
        else
            k += n - j;
    }
    if (skip != 0)
        heap_set_level(ds.heap, skip);
    return 0;
}
#endif

/* upper case, full-width ASCII to half width, katakana to hiragana */
void ds_fold_text(unsigned short *dst, unsigned short *src)
{
    unsigned short c;

    while ((c = *src++) != 0) {
        if (c >= 0xFF01 && c < 0xFF5E)
            c += 0x120;
        if (c >= 0x30A1 && c <= 0x30F4)
            c += 0xFFA0;
        else
            c = towupper(c);
        *dst++ = c;
    }
    *dst = 0;
}
