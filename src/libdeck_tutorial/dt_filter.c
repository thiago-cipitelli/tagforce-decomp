// @range 0x088135A8-0x08814460
// @asm 0x08813888-0x088139EC
// @asm 0x088139EC-0x08813B7C
// @asm 0x08813D60-0x08813EC0
// @asm 0x088140F8-0x088143A8
// @asm 0x088143A8-0x08814460
// @data 0x08822024-0x08822050
/* (The code of libdeck dk_filter.c, first half; the lesson has no "new" filter.) Filtering and sorting a list's visible order: the "new"-mark reset, the incremental text search
   with its word history, the filters (kind, type, race, level ranges, spell/trap kinds, limit,
   genre/new/alternate picture), the text match (Boyer-Moore-Horspool over the normalised name and
   text), the stable sort by one of nine keys and the key values. */
#include "libdeck_tutorial.h"

typedef struct DtIdList DtIdList;

typedef struct DtKey {                 /* qsort record */
    unsigned short idx;                /* entry index */
    unsigned short pos;                /* position before sorting */
    int value;                         /* +0x04 dt_sort_key_value, -2 = none */
} DtKey;

typedef struct DtSearch {              /* dt_search 0x095D2720 */
    unsigned short pos;                /* entries scanned */
    unsigned short total;              /* +0x02 */
    unsigned short n;                  /* +0x04 matches */
    unsigned short pad06;
    unsigned short *out;               /* +0x08 */
    unsigned short *word;              /* +0x0C */
} DtSearch;

typedef int (*DtFilterFn)(unsigned short id, unsigned int arg);

/* the search word history: 8 words of 32 characters at +0x116 of the dt_deck block (dt_profile) */
typedef struct DtWordsView {
    unsigned char pad[0x116];
    unsigned short words[8][32];
} DtWordsView;
#define DK_WORDS (((DtWordsView *)&dts)->words)

extern DtSearch dt_search;             /* 0x08824704 */

void *dt_eres_get(unsigned short id);
int inv_get_state(unsigned short id);
void card_get_genre(int id, unsigned short *g);

int cfg_get_byte0_hi2(void);
int wcslen(const unsigned short *s);
int wcscmp(const unsigned short *a, const unsigned short *b);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short towupper(unsigned short c);
void qsort(void *base, unsigned int n, unsigned int size, int (*cmp)(const void *, const void *));
unsigned short dt_id_count(unsigned short id, DtIdList *l);
int dt_kind_icon(unsigned short kind);
int dt_text_match(unsigned short id, unsigned short *word);
int dt_filter_attr(DtEnt *e, unsigned int *arg);
int dt_filter_kind(unsigned short id, unsigned int mask);
int dt_filter_type(unsigned short id, unsigned int mask);
int dt_filter_race(unsigned short id, unsigned int mask);
int dt_filter_level(unsigned short id, unsigned int mask);
int dt_filter_spelltrap(unsigned short id, unsigned int mask);
int dt_filter_limit(unsigned short id, unsigned int mask);
unsigned short *dt_find_text(unsigned short *text, unsigned short *word, int n);
void dt_fold_text(unsigned short *dst, unsigned short *src);
void dt_list_sort(DtList *l, int sort, unsigned short dir);
int dt_cmp_up(const void *pa, const void *pb);
int dt_cmp_down(const void *pa, const void *pb);
int dt_sort_key_value(DtEnt *e, int sort);

/* clears the filter of l (the trunk keeps only the marked or unmarked cards when the mark mode
   asks for it), re-sorted by key 1 */
unsigned short dt_list_filter_reset(DtList *l)
{
    int only = 0;
    unsigned int want = 0;
    int m;
    int i;
    int n;

    if (l->tab == 3) {
        m = cfg_get_byte0_hi2();
        if (m == 1)
            only = 1;
        if (m == 2) {
            only = 1;
            want = 1;
        }
    }
    n = 0;
    for (i = 0; i < l->n; i++) {
        if (!only || want == inv_get_flag7(l->ents[i].id))
            l->order[n++] = i;
    }
    l->count = n;
    l->filter = 0;
    dt_list_sort(l, 1, 0);
    return l->count;
}

/* the incremental search over the visible order (16 entries a frame) */
void dt_search_start(DtList *l, unsigned short *word)
{
    memset(&dt_search, 0, 0x10);
    dt_search.total = l->count;
    dt_search.word = word;
    if (dt_search.total > 0)
        dt_search.out = heap_alloc(dts.heap, dt_search.total << 1);
}

/* progress 0..1000 */
unsigned short dt_search_step(DtList *l)
{
    int i;
    unsigned short k;

    if (dt_search.out == 0)
        return 1000;
    if (dt_search.pos >= dt_search.total)
        return 1000;
    for (i = 0; i < 16 && dt_search.pos + i < dt_search.total; i++) {
        k = l->order[dt_search.pos + i];
        if (dt_text_match(l->ents[k].id, dt_search.word))
            dt_search.out[dt_search.n++] = k;
    }
    dt_search.pos += i;
    return dt_search.pos * 1000 / dt_search.total;
}


#ifdef NON_MATCHING /* laços do histórico com outra alocação ($s trocados) */
* the matches become the visible order; the word goes on top of the history */
void dt_search_finish(DtList *l)
{
    int len;
    int i;
    int k;

    if (dt_search.out != 0) {
        memcpy(l->order, dt_search.out, dt_search.n << 1);
        l->count = dt_search.n;
        heap_set_level(dts.heap, dt_search.out);
        dt_search.out = 0;
    }
    l->filter = 8;
    k = 7;
    len = wcslen(dt_search.word);
    for (i = 0; i < 8; i++) {
        if (DK_WORDS[i][0] == 0) {
            k = i;
            break;
        }
        if (len == wcslen(DK_WORDS[i]) && wcscmp(DK_WORDS[i], dt_search.word) == 0) {
            k = i;
            break;
        }
    }
    for (; k > 0; k--)
        wcscpy(DK_WORDS[k], DK_WORDS[k - 1]);
    wcscpy(DK_WORDS[0], dt_search.word);
}
#endif

DtFilterFn dt_filters[9] = {
    0, (DtFilterFn)dt_filter_kind, (DtFilterFn)dt_filter_type, (DtFilterFn)dt_filter_race,
    (DtFilterFn)dt_filter_level, (DtFilterFn)dt_filter_spelltrap, (DtFilterFn)dt_filter_limit, 0, 0,
};


#ifdef NON_MATCHING /* laço com a tabela de filtros; alocação diferente */
* keeps the entries passing filter f (arg its parameter); returns the count */
int dt_list_filter(DtList *l, unsigned short f, unsigned int *arg)
{
    unsigned short *tmp = heap_alloc(dts.heap, l->count << 1);
    int n;
    int i;
    unsigned short k;
    DtEnt *e;
    int ok;
    unsigned short *p;

    if (tmp == 0)
        return l->count;
    n = 0;
    p = tmp;
    for (i = 0; i < l->count; i++) {
        k = l->order[i];
        e = &l->ents[k];
        ok = 1;
        if (dt_filters[f] != 0)
            ok = dt_filters[f](e->id, *arg);
        else if (f == 7)
            ok = dt_filter_attr(e, arg);
        if (ok) {
            *p++ = k;
            n++;
        }
    }
    memcpy(l->order, tmp, n << 1);
    l->count = n;
    l->filter = f;
    if (tmp != 0)
        heap_set_level(dts.heap, tmp);
    return n;
}
#endif

int dt_filter_kind(unsigned short id, unsigned int mask)
{
    return (mask & 1 << dt_kind_icon(card_get_attribute_icon(id))) != 0;
}

int dt_filter_type(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_type(id))
            return 1;
    }
    return 0;
}

int dt_filter_race(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_kind(id))
            return 1;
    }
    return 0;
}

/* level bits of the three level groups (1-4, 5-6, 7+) */
unsigned short dt_level_masks[3] = { 0x1E, 0x60, 0xFF80 };

int dt_filter_level(unsigned short id, unsigned int mask)
{
    unsigned short bits = 0;
    unsigned short level = card_get_level(id);

    if (mask & 1)
        bits |= dt_level_masks[0];
    if (mask & 2)
        bits |= dt_level_masks[1];
    if (mask & 4)
        bits |= dt_level_masks[2];
    if (card_is_monster(id) && (bits & 1 << level))
        return 1;
    return 0;
}


#ifdef NON_MATCHING /* o default da tabela de saltos interna sai com beqz+nop no original */
* spell and trap kinds */
int dt_filter_spelltrap(unsigned short id, unsigned int mask)
{
    switch (card_get_attribute_icon(id)) {
    default:
        return 0;
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
#endif

int dt_filter_limit(unsigned short id, unsigned int mask)
{
    return (mask & 1 << card_get_limit(id)) != 0;
}

/* genre bits, the "new" cards and the cards with an alternate picture */
int dt_filter_attr(DtEnt *e, unsigned int *arg)
{
    unsigned short g[3];

    if ((arg[1] & 8) && inv_get_bit3(e->id) && dt_id_count(e->id, dt_eres_get(0x8032)) > 0)
        return 1;
    card_get_genre(e->id, g);
    if (g[0] & (arg[0] & 0xFFFF))
        return 1;
    if (g[1] & (unsigned short)(arg[0] >> 16))
        return 1;
    return (g[2] & (unsigned short)arg[1]) != 0;
}

/* word found in the card's name or text (with and without ruby) */
int dt_text_match(unsigned short id, unsigned short *word)
{
    unsigned short txt[0x800];
    unsigned short raw[0x800];
    unsigned short w[0x40];
    unsigned short n = wcslen(word);
    unsigned short *s;

    dt_fold_text(w, word);
    s = card_get_name(id);
    wtext_strip_tags(raw, s);
    dt_fold_text(txt, raw);
    if (dt_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    dt_fold_text(txt, raw);
    if (dt_find_text(txt, w, n))
        return 1;
    s = card_get_desc(id);
    wtext_strip_tags(raw, s);
    dt_fold_text(txt, raw);
    if (dt_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    dt_fold_text(txt, raw);
    return dt_find_text(txt, w, n) != 0;
}


#ifdef NON_MATCHING /* ordem de um bne no laço de busca */
* Horspool search of word (n chars) in text */
unsigned short *dt_find_text(unsigned short *text, unsigned short *word, int n)
{
    int i;
    int len;
    int j;
    int k;
    unsigned short *t;
    unsigned short *w;

    unsigned char *skip = heap_alloc(dts.heap, 0x10000);

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
                        heap_set_level(dts.heap, skip);
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
        heap_set_level(dts.heap, skip);
    return 0;
}
#endif


#ifdef NON_MATCHING /* teste do caractere e máscaras u16 com outra alocação */
* upper case, full-width ASCII to half width, katakana to hiragana */
void dt_fold_text(unsigned short *dst, unsigned short *src)
{
    unsigned short c = *src++;

    while (c != 0) {
        if (c >= 0xFF01 && c < 0xFF5E)
            c += 0x120;
        if (c >= 0x30A1 && c <= 0x30F4)
            c += (unsigned short)(0x3041 - 0x30A1);
        else
            c = towupper(c);
        *dst = c;
        dst++;
        c = *src++;
    }
    *dst = 0;
}
#endif

