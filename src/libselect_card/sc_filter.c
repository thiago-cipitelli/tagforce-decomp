// @range 0x08810BF4-0x088119C8
// @asm 0x08810BF4-0x08810E74
// @asm 0x08810F84-0x08811130
// @asm 0x08811130-0x088112C0
// @asm 0x088115CC-0x08811730
// @asm 0x08811730-0x088118C0
// @data 0x0881B424-0x0881B454
/* Sorting a pane's visible order by one of the nine keys (stable, "no value" last), the key value
   of a card, the "new"-mark filter, the incremental text search and its word history, and the
   filters by kind, type and race. */
#include "libselect_card.h"

typedef struct ScKey {                 /* qsort record */
    unsigned short idx;                /* entry index */
    unsigned short pos;                /* position before sorting */
    int value;                         /* +0x04 sc_sort_key_value, -2 = none */
} ScKey;

typedef struct ScSearch {              /* sc_search 0x0881B98C */
    unsigned short pos;                /* entries scanned */
    unsigned short total;              /* +0x02 */
    unsigned short n;                  /* +0x04 matches */
    unsigned short pad06;
    unsigned short *out;               /* +0x08 */
    unsigned short *word;              /* +0x0C */
} ScSearch;

typedef int (*ScFilterFn)(unsigned short id, unsigned int arg);

extern ScSearch sc_search;

int cfg_get_byte0_hi2(void);
int wcslen(const unsigned short *s);
int wcscmp(const unsigned short *a, const unsigned short *b);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int sc_text_match(unsigned short id, unsigned short *word);
int sc_filter_attr(ScEnt *e, unsigned int *arg);
int sc_filter_kind(unsigned short id, unsigned int mask);
int sc_filter_type(unsigned short id, unsigned int mask);
int sc_filter_race(unsigned short id, unsigned int mask);
int sc_filter_level(unsigned short id, unsigned int mask);
int sc_filter_atk(unsigned short id, unsigned int mask);
int sc_filter_def(unsigned short id, unsigned int mask);
int sc_sort_key_value(ScEnt *e, unsigned short sort);
int sc_cmp_up(const void *pa, const void *pb);
int sc_cmp_down(const void *pa, const void *pb);

#ifdef NON_MATCHING /* o original tem o laço de cópia desenrolado com outra alocação */
/* sorts the visible order of l by key sort, dir 0 up, 1 down */
void sc_list_sort(ScList *l, int sort, unsigned short dir)
{
    unsigned int n = l->count;
    ScKey *k = heap_alloc(sc.heap, n << 3);
    int i;
    ScKey *p;

    for (i = 0, p = k; i < (int)n; i++, p++) {
        p->idx = l->order[i];
        p->pos = i;
        p->value = sc_sort_key_value(&l->ents[p->idx], sort);
    }
    if (dir == 0)
        qsort(k, n, 8, sc_cmp_up);
    else if (dir == 1)
        qsort(k, n, 8, sc_cmp_down);
    for (i = 0; i < (int)n; i++)
        l->order[i] = k[i].idx;
    if (k != 0)
        heap_set_level(sc.heap, k);
    l->sort = sort;
    l->dir = dir;
}
#endif

int sc_cmp_up(const void *pa, const void *pb)
{
    const ScKey *a = pa;
    const ScKey *b = pb;

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

int sc_cmp_down(const void *pa, const void *pb)
{
    const ScKey *a = pa;
    const ScKey *b = pb;

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

unsigned char sc_spell_order[8] = { 9, 9, 2, 1, 4, 0, 3, 0 };
unsigned char sc_trap_order[8] = { 0x13, 0x0A, 0x13, 0x13, 0x0B, 0x13, 0x13, 0 };

#ifdef NON_MATCHING /* lógica conferida com o asm; 61 palavras: o original devolve -2 do case 7 direto (bnel) e o
                       case 8 e o default interno caem num li -2 comum, que o mwcc não reproduz */
/* value of key sort for a card (smaller sorts first), -2 none */
int sc_sort_key_value(ScEnt *e, unsigned short sort)
{
    int a;

    switch (sort) {
    case 1:
        return card_get_sort(e->id);
    case 2:
        return card_get_attribute_icon(e->id);
    case 9:
        return card_get_limit(e->id);
    }
    switch (card_get_attribute_icon(e->id)) {
    case 7:
        if (sort == 8)
            return sc_spell_order[card_get_prop5_hi(e->id)];
        return -2;
    case 8:
        if (sort == 8)
            return sc_trap_order[card_get_prop5_hi(e->id)];
        return -2;
    default:
        switch (sort) {
        case 3:
            return 0xFFFF - card_get_level(e->id);
        case 4:
            a = card_get_atk_q(e->id);
            if (a == 0xFFFF)
                return 0x10000;
            return 0xFFFF - a;
        case 5:
            a = card_get_def_q(e->id);
            if (a == 0xFFFF)
                return 0x10000;
            return 0xFFFF - a;
        case 6:
            return card_get_type(e->id);
        case 7:
            return card_get_kind(e->id);
        }
        return -2;
    }
}
#endif

#ifdef NON_MATCHING /* lógica conferida com o asm; 50 palavras: o original tem um "b fim" próprio em cada caso
                       (sem encadear os desvios ao epílogo); com return por caso piora para 92 */
/* the value shown for key sort (-1 = "?", -2 none) */
int sc_sort_value(ScEnt *e, unsigned short sort)
{
    int v;

    switch (sort) {
    case 1:
        v = card_get_sort_position(e->id);
        break;
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
            if (sort == 8) {
                v = card_get_prop5_hi(e->id);
                if (v == 0)
                    v = 7;
            } else {
                v = -2;
            }
            break;
        default:
            switch (sort) {
            case 3:
                v = card_get_level(e->id);
                break;
            case 4:
                v = card_get_atk_q(e->id);
                if (v == 0xFFFF)
                    v = -1;
                break;
            case 5:
                v = card_get_def_q(e->id);
                if (v == 0xFFFF)
                    v = -1;
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

/* clears the filter of l (keeping only the marked cards of the owned list when the mark mode
   asks for it), re-sorted by key 1 */
unsigned short sc_list_filter_reset(ScList *l)
{
    int only = 0;
    int want = 0;
    int none = -1;
    int m;
    int i;
    int n;

    switch (sc.kind) {
    case 1:
    case 2:
        none = 0;
        break;
    }
    if (l->tab == none) {
        m = cfg_get_byte0_hi2();
        if (m == 1)
            only = 1;
        if (m == 2) {
            only = 1;
            want = only;
        }
    }
    n = 0;
    for (i = 0; i < l->n; i++) {
        if (!only || want == inv_get_flag7(l->ents[i].id))
            l->order[n++] = i;
    }
    l->count = n;
    l->filter = 0;
    sc_list_sort(l, 1, 0);
    return l->count;
}

/* the incremental search over the visible order (16 entries a frame) */
void sc_search_start(ScList *l, unsigned short *word)
{
    memset(&sc_search, 0, 0x10);
    sc_search.total = l->count;
    sc_search.word = word;
    if (sc_search.total > 0)
        sc_search.out = heap_alloc(sc.heap, sc_search.total << 1);
}

/* progress 0..1000 */
unsigned short sc_search_step(ScList *l)
{
    int i;
    unsigned short k;

    if (sc_search.out == 0)
        return 1000;
    if (sc_search.pos >= sc_search.total)
        return 1000;
    for (i = 0; i < 16 && sc_search.pos + i < sc_search.total; i++) {
        k = l->order[sc_search.pos + i];
        if (sc_text_match(l->ents[k].id, sc_search.word))
            sc_search.out[sc_search.n++] = k;
    }
    sc_search.pos += i;
    return sc_search.pos * 1000 / sc_search.total;
}

#ifdef NON_MATCHING /* laços do histórico com outra alocação */
/* the matches become the visible order; the word goes on top of the history */
void sc_search_finish(ScList *l)
{
    int len;
    int i;
    int k;

    if (sc_search.out != 0) {
        memcpy(l->order, sc_search.out, sc_search.n << 1);
        l->count = sc_search.n;
        heap_set_level(sc.heap, sc_search.out);
        sc_search.out = 0;
    }
    l->filter = 8;
    len = wcslen(sc_search.word);
    for (i = 0; i < 8; i++) {
        k = i;
        if (sc.words[i][0] == 0)
            break;
        if (len == wcslen(sc.words[i]) && wcscmp(sc.words[i], sc_search.word) == 0)
            break;
        k = 7;
    }
    for (; k > 0; k--)
        wcscpy(sc.words[k], sc.words[k - 1]);
    wcscpy(sc.words[0], sc_search.word);
}
#endif

ScFilterFn sc_filters[8] = {
    0, (ScFilterFn)sc_filter_kind, (ScFilterFn)sc_filter_type, (ScFilterFn)sc_filter_race,
    (ScFilterFn)sc_filter_level, (ScFilterFn)sc_filter_atk, (ScFilterFn)sc_filter_def, 0,
};

#ifdef NON_MATCHING /* laço com a tabela de filtros; alocação diferente */
/* keeps the entries passing filter f (arg its parameter); returns the count */
unsigned short sc_list_filter(ScList *l, unsigned short f, unsigned int *arg)
{
    unsigned short *tmp = heap_alloc(sc.heap, l->count << 1);
    unsigned short n;
    int i;
    unsigned short k;
    ScEnt *e;
    int ok;
    unsigned short *p;

    if (tmp == 0)
        return l->count;
    n = 0;
    for (i = 0, p = tmp; i < l->count; i++) {
        k = l->order[i];
        e = &l->ents[k];
        ok = 1;
        if (sc_filters[f] != 0)
            ok = sc_filters[f](e->id, *arg);
        else if (f == 7)
            ok = sc_filter_attr(e, arg);
        if (ok) {
            *p++ = k;
            n++;
        }
    }
    memcpy(l->order, tmp, n << 1);
    l->count = n;
    l->filter = f;
    if (tmp != 0)
        heap_set_level(sc.heap, tmp);
    return n;
}
#endif

int sc_filter_kind(unsigned short id, unsigned int mask)
{
    return (mask & 1 << sc_kind_icon(card_get_attribute_icon(id))) != 0;
}

int sc_filter_type(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_type(id))
            return 1;
    }
    return 0;
}

int sc_filter_race(unsigned short id, unsigned int mask)
{
    if (card_is_monster(id)) {
        if (mask & 1 << card_get_kind(id))
            return 1;
    }
    return 0;
}
