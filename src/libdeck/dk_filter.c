// @range 0x08817D18-0x088192C8
// @data 0x0882313C-0x08823178
// @rodata 0x0882266C-0x08822688
// @asm 0x08817FF8-0x0881815C
// @asm 0x0881815C-0x088182EC
// @asm 0x08818894-0x08818B44
// @asm 0x08819138-0x088192C8
/* Filtering and sorting a list's visible order: the "new"-mark reset, the incremental text search
   with its word history, the filters (kind, type, race, level ranges, spell/trap kinds, limit,
   genre/new/alternate picture), the text match (Boyer-Moore-Horspool over the normalised name and
   text), the stable sort by one of nine keys and the key values. */
#include "libdeck.h"

typedef struct DkIdList DkIdList;

typedef struct DkKey {                 /* qsort record */
    unsigned short idx;                /* entry index */
    unsigned short pos;                /* position before sorting */
    int value;                         /* +0x04 dk_sort_key_value, -2 = none */
} DkKey;

typedef struct DkSearch {              /* dk_search 0x095D2720 */
    unsigned short pos;                /* entries scanned */
    unsigned short total;              /* +0x02 */
    unsigned short n;                  /* +0x04 matches */
    unsigned short pad06;
    unsigned short *out;               /* +0x08 */
    unsigned short *word;              /* +0x0C */
} DkSearch;

typedef int (*DkFilterFn)(unsigned short id, unsigned int arg);

/* the search word history: 8 words of 32 characters at +0x116 of the dk_deck block (dk_profile) */
typedef struct DkWordsView {
    unsigned char pad[0x116];
    unsigned short words[8][32];
} DkWordsView;
#define DK_WORDS (((DkWordsView *)&dk_deck)->words)

extern DkSearch dk_search;

int cfg_get_byte0_hi2(void);
int wcslen(const unsigned short *s);
int wcscmp(const unsigned short *a, const unsigned short *b);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short towupper(unsigned short c);
void qsort(void *base, unsigned int n, unsigned int size, int (*cmp)(const void *, const void *));
unsigned short dk_id_count(unsigned short id, DkIdList *l);
int dk_kind_icon(unsigned short kind);
int dk_text_match(unsigned short id, unsigned short *word);
int dk_filter_attr(DkEnt *e, unsigned int *arg);
int dk_filter_kind(unsigned short id, unsigned int mask);
int dk_filter_type(unsigned short id, unsigned int mask);
int dk_filter_race(unsigned short id, unsigned int mask);
int dk_filter_level(unsigned short id, unsigned int mask);
int dk_filter_spelltrap(unsigned short id, unsigned int mask);
int dk_filter_limit(unsigned short id, unsigned int mask);
unsigned short *dk_find_text(unsigned short *text, unsigned short *word, int n);
void dk_fold_text(unsigned short *dst, unsigned short *src);
void dk_list_sort(DkList *l, int sort, unsigned short dir);
int dk_cmp_up(const void *pa, const void *pb);
int dk_cmp_down(const void *pa, const void *pb);
int dk_sort_key_value(DkEnt *e, int sort);

/* clears the filter of l (the trunk keeps only the marked or unmarked cards when the mark mode
   asks for it), re-sorted by key 1 */
unsigned short dk_list_filter_reset(DkList *l)
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
    dk_list_sort(l, 1, 0);
    return l->count;
}

/* the incremental search over the visible order (16 entries a frame) */
void dk_search_start(DkList *l, unsigned short *word)
{
    memset(&dk_search, 0, 0x10);
    dk_search.total = l->count;
    dk_search.word = word;
    if (dk_search.total > 0)
        dk_search.out = heap_alloc(dk_heap, dk_search.total << 1);
}

/* progress 0..1000 */
unsigned short dk_search_step(DkList *l)
{
    int i;
    unsigned short k;

    if (dk_search.out == 0)
        return 1000;
    if (dk_search.pos >= dk_search.total)
        return 1000;
    for (i = 0; i < 16 && dk_search.pos + i < dk_search.total; i++) {
        k = l->order[dk_search.pos + i];
        if (dk_text_match(l->ents[k].id, dk_search.word))
            dk_search.out[dk_search.n++] = k;
    }
    dk_search.pos += i;
    return dk_search.pos * 1000 / dk_search.total;
}


#ifdef NON_MATCHING /* 16 palavras: len, ponteiro do histórico e k em $s em rodízio (original s2/s0/s1); ordens de declaração e de instrução testadas */
/* the matches become the visible order; the word goes on top of the history */
void dk_search_finish(DkList *l)
{
    int len;
    int i;
    int k;

    if (dk_search.out != 0) {
        memcpy(l->order, dk_search.out, dk_search.n << 1);
        l->count = dk_search.n;
        heap_set_level(dk_heap, dk_search.out);
        dk_search.out = 0;
    }
    l->filter = 8;
    len = wcslen(dk_search.word);
    k = 7;
    for (i = 0; i < 8; i++) {
        if (DK_WORDS[i][0] == 0) {
            k = i;
            break;
        }
        if (len == wcslen(DK_WORDS[i]) && wcscmp(DK_WORDS[i], dk_search.word) == 0) {
            k = i;
            break;
        }
    }
    for (; k > 0; k--)
        wcscpy(DK_WORDS[k], DK_WORDS[k - 1]);
    wcscpy(DK_WORDS[0], dk_search.word);
}
#endif

DkFilterFn dk_filters[9] = {
    0, (DkFilterFn)dk_filter_kind, (DkFilterFn)dk_filter_type, (DkFilterFn)dk_filter_race,
    (DkFilterFn)dk_filter_level, (DkFilterFn)dk_filter_spelltrap, (DkFilterFn)dk_filter_limit, 0, 0,
};


#ifdef NON_MATCHING /* 22 palavras: o original carrega e->id e calcula e (em a1) antes do teste do filtro, com ok = 1 no atraso do beqz; o ternário chega mais perto que if/else */
/* keeps the entries passing filter f (arg its parameter); returns the count */
int dk_list_filter(DkList *l, unsigned short f, unsigned int *arg)
{
    unsigned short *tmp = heap_alloc(dk_heap, l->count << 1);
    int n;
    int i;
    unsigned short k;
    DkEnt *e;
    int ok;

    if (tmp == 0)
        return l->count;
    n = 0;
    for (i = 0; i < l->count; i++) {
        k = l->order[i];
        e = &l->ents[k];
        ok = dk_filters[f] != 0 ? dk_filters[f](e->id, *arg) : f == 7 ? dk_filter_attr(e, arg) : 1;
        if (ok) {
            tmp[n++] = k;
        }
    }
    memcpy(l->order, tmp, n << 1);
    l->count = n;
    l->filter = f;
    if (tmp != 0)
        heap_set_level(dk_heap, tmp);
    return n;
}
#endif

int dk_filter_kind(unsigned short id, unsigned int mask)
{
    return (mask & 1 << dk_kind_icon(card_get_attribute_icon(id))) != 0;
}

int dk_filter_type(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_type(id))
            return 1;
    }
    return 0;
}

int dk_filter_race(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_kind(id))
            return 1;
    }
    return 0;
}

/* level bits of the three level groups (1-4, 5-6, 7+) */
unsigned short dk_level_masks[3] = { 0x1E, 0x60, 0xFF80 };

int dk_filter_level(unsigned short id, unsigned int mask)
{
    unsigned short bits = 0;
    unsigned short level = card_get_level(id);

    if (mask & 1)
        bits |= dk_level_masks[0];
    if (mask & 2)
        bits |= dk_level_masks[1];
    if (mask & 4)
        bits |= dk_level_masks[2];
    if (card_is_monster(id) && (bits & 1 << level))
        return 1;
    return 0;
}


/* spell and trap kinds */
int dk_filter_spelltrap(unsigned short id, unsigned int mask)
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

int dk_filter_limit(unsigned short id, unsigned int mask)
{
    return (mask & 1 << card_get_limit(id)) != 0;
}

/* genre bits, the "new" cards and the cards with an alternate picture */
int dk_filter_attr(DkEnt *e, unsigned int *arg)
{
    unsigned short g[3];

    if ((arg[0] & 1) && inv_get_state(e->id) == 1)
        return 1;
    if ((arg[1] & 8) && inv_get_bit3(e->id) && dk_id_count(e->id, dk_res_get(0x8032)) > 0)
        return 1;
    card_get_genre(e->id, g);
    if (g[0] & (arg[0] & 0xFFFF))
        return 1;
    if (g[1] & (unsigned short)(arg[0] >> 16))
        return 1;
    return (g[2] & (unsigned short)arg[1]) != 0;
}

/* word found in the card's name or text (with and without ruby) */
int dk_text_match(unsigned short id, unsigned short *word)
{
    unsigned short txt[0x800];
    unsigned short raw[0x800];
    unsigned short w[0x40];
    unsigned short n = wcslen(word);
    unsigned short *s;

    dk_fold_text(w, word);
    s = card_get_name(id);
    wtext_strip_tags(raw, s);
    dk_fold_text(txt, raw);
    if (dk_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    dk_fold_text(txt, raw);
    if (dk_find_text(txt, w, n))
        return 1;
    s = card_get_desc(id);
    wtext_strip_tags(raw, s);
    dk_fold_text(txt, raw);
    if (dk_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    dk_fold_text(txt, raw);
    return dk_find_text(txt, w, n) != 0;
}


#ifdef NON_MATCHING /* 2 palavras: o original põe "j = n - 1" (move t2,a3) no atraso do bne do primeiro caractere; nenhuma ordem das três atribuições reproduz */
/* Horspool search of word (n chars) in text */
unsigned short *dk_find_text(unsigned short *text, unsigned short *word, int n)
{
    int i;
    int len;
    int j;
    int k;
    unsigned short *t;
    unsigned short *w;

    unsigned char *skip = heap_alloc(dk_heap, 0x10000);

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
                        heap_set_level(dk_heap, skip);
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
        heap_set_level(dk_heap, skip);
    return 0;
}
#endif


/* upper case, full-width ASCII to half width, katakana to hiragana */
void dk_fold_text(unsigned short *dst, unsigned short *src)
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


/* sorts the visible order of l by key sort, dir 0 up, 1 down */
void dk_list_sort(DkList *l, int sort, unsigned short dir)
{
    unsigned short n;
    DkKey *k;
    int i;
    unsigned short id;

    n = l->count;
    k = heap_alloc(dk_heap, n << 3);
    for (i = 0; i < n; i++) {
        id = l->order[i];
        k[i].idx = id;
        k[i].pos = i;
        k[i].value = dk_sort_key_value(&l->ents[id], sort);
    }
    if (dir == 0)
        qsort(k, (int)n, sizeof(DkKey), dk_cmp_up);
    else if (dir == 1)
        qsort(k, (int)n, sizeof(DkKey), dk_cmp_down);
    for (i = 0; i < n; i++)
        l->order[i] = k[i].idx;
    if (k != 0)
        heap_set_level(dk_heap, k);
    l->sort = sort;
    l->dir = dir;
}


int dk_cmp_up(const void *pa, const void *pb)
{
    const DkKey *a = pa;
    const DkKey *b = pb;

    if (a->value == b->value) {
        if (a->pos < b->pos)
            return -1;
        if (b->pos < a->pos)
            return 1;
    } else {
        if (a->value == -2)
            return 1;
        if (b->value == -2)
            return -1;
        if (a->value < b->value)
            return -1;
        if (b->value < a->value)
            return 1;
    }
    return 0;
}


int dk_cmp_down(const void *pa, const void *pb)
{
    const DkKey *a = pa;
    const DkKey *b = pb;

    if (a->value == b->value) {
        if (a->pos > b->pos)
            return -1;
        if (a->pos < b->pos)
            return 1;
    } else {
        if (a->value == -2)
            return 1;
        if (b->value == -2)
            return -1;
        if (a->value < b->value)
            return 1;
        if (b->value < a->value)
            return -1;
    }
    return 0;
}

unsigned char dk_spell_order[8] = { 9, 9, 2, 1, 4, 0, 3, 0 };
unsigned char dk_trap_order[8] = { 0x13, 0x0A, 0x13, 0x13, 0x0B, 0x13, 0x13, 0 };


/* value of key sort for a card (smaller sorts first), -2 none */
int dk_sort_key_value(DkEnt *e, int key)
{
    unsigned short sort = key;
    int v;

    switch (sort) {
    case 1:
        return card_get_sort(e->id);
    case 2:
        v = card_get_attribute_icon(e->id);
        break;
    case 9:
        v = card_get_limit(e->id);
        break;
    default:
        switch (card_get_attribute_icon(e->id)) {
        case 7:
            if (sort == 8)
                return dk_spell_order[card_get_prop5_hi(e->id)];
            break;
        case 8:
            if (sort != 8)
                break;
            return dk_trap_order[card_get_prop5_hi(e->id)];
        default:
            switch (sort) {
            case 3:
                return 0xFFFF - card_get_level(e->id);
            case 4:
                v = card_get_atk_q(e->id);
                if (v == 0xFFFF)
                    return 0x10000;
                return 0xFFFF - v;
            case 5:
                v = card_get_def_q(e->id);
                if (v == 0xFFFF)
                    return 0x10000;
                return 0xFFFF - v;
            case 6:
                return card_get_type(e->id);
            case 7:
                return card_get_kind(e->id);
            }
            break;
        }
        v = -2;
        break;
    }
    return v;
}


#ifdef NON_MATCHING /* 16 palavras: o "default: v = -2" do switch(sort) interno vai para o atraso do b; no original fica no fim (b+nop), alvo comum com o "sort != 8" dos ícones 7/8. goto, retornos e default sem corpo não reproduzem */
/* the value shown for key sort (-1 = "?", -2 none) */
int dk_sort_value(DkEnt *e, unsigned short sort)
{
    int v;

    switch (sort) {
    case 1:
        return card_get_sort_position(e->id);
    case 2:
        v = card_get_attribute_icon(e->id);
        break;
    case 9:
        v = card_get_limit(e->id);
        break;
    default:
        switch (card_get_attribute_icon(e->id)) {
        case 7:
        case 8:
            if (sort != 8)
                return -2;
            v = card_get_prop5_hi(e->id);
            v = v == 0 ? 7 : v;
            break;
        default:
            switch (sort) {
            case 3:
                v = card_get_level(e->id);
                break;
            case 4:
                v = card_get_atk_q(e->id);
                v = v == 0xFFFF ? -1 : v;
                break;
            case 5:
                v = card_get_def_q(e->id);
                v = v == 0xFFFF ? -1 : v;
                break;
            case 6:
                v = card_get_type(e->id);
                break;
            case 7:
                v = card_get_kind(e->id);
                break;
            default:
                v = -2;
                break;
            }
            break;
        }
        break;
    }
    return v;
}
#endif
