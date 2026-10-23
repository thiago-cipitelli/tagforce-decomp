// @range 0x088119C8-0x088121D0
// @asm 0x08811AA4-0x08811C04
// @asm 0x08811C3C-0x08811D28
// @asm 0x08811E68-0x08812118
// @asm 0x08812118-0x088121D0
// @data 0x0881B458-0x0881B460
/* The remaining card filters (level ranges, spell/trap kinds, limit, genre/new/alternate picture)
   and the text search: a Boyer-Moore-Horspool scan over the normalised card name and text. */
#include "libselect_card.h"

int inv_get_state(unsigned short id);
void card_get_genre(int id, unsigned short *out);
void wtext_strip_tags(unsigned short *dst, unsigned short *src);
void wtext_strip_tags2(unsigned short *dst, unsigned short *src);
unsigned short *card_get_desc(int id);
unsigned short towupper(unsigned short c);
int wcslen(const unsigned short *s);
unsigned short sc_id_count(unsigned short id, ScIdList *l);
unsigned short *sc_find_text(unsigned short *text, unsigned short *word, int n);
void sc_fold_text(unsigned short *dst, unsigned short *src);

/* level bits of the three level groups (1-4, 5-6, 7+) */
unsigned short sc_level_masks[3] = { 0x1E, 0x60, 0xFF80 };

int sc_filter_level(unsigned short id, unsigned int mask)
{
    unsigned short bits = 0;
    unsigned short level = card_get_level(id);

    if (mask & 1)
        bits |= sc_level_masks[0];
    if (mask & 2)
        bits |= sc_level_masks[1];
    if (mask & 4)
        bits |= sc_level_masks[2];
    if (card_is_monster(id) && (bits & 1 << level))
        return 1;
    return 0;
}

#ifdef NON_MATCHING /* cadeias de testes dos tipos com outra ordem */
/* spell and trap kinds */
int sc_filter_atk(unsigned short id, unsigned int mask)
{
    int a = card_get_attribute_icon(id);

    if (a == 8) {
        switch (card_get_prop5_hi(id)) {
        case 4:
            if (mask & 0x100)
                return 1;
            break;
        case 1:
            if (mask & 0x80)
                return 1;
            break;
        case 0:
            if (mask & 0x200)
                return 1;
            break;
        }
    } else {
        if (a != 7)
            return 0;
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
    }
    return 0;
}
#endif

int sc_filter_def(unsigned short id, unsigned int mask)
{
    return (mask & 1 << card_get_limit(id)) != 0;
}

#ifdef NON_MATCHING /* a contagem é comparada com blez depois da máscara u16; o mwcc atual usa beqz */
/* genre bits, the "new" cards and the cards with an alternate picture */
int sc_filter_attr(ScEnt *e, unsigned int *arg)
{
    unsigned short g[3];

    if ((arg[0] & 1) && inv_get_state(e->id) == 1)
        return 1;
    if ((arg[1] & 8) && inv_get_bit3(e->id) && sc_id_count(e->id, res_get(0x8032)) > 0)
        return 1;
    card_get_genre(e->id, g);
    if (g[0] & (arg[0] & 0xFFFF))
        return 1;
    if ((arg[0] >> 16 & 0xFFFF) & g[1])
        return 1;
    return (g[2] & (unsigned short)arg[1]) != 0;
}
#endif

/* word found in the card's name or text (with and without ruby) */
int sc_text_match(unsigned short id, unsigned short *word)
{
    unsigned short txt[0x800];
    unsigned short raw[0x800];
    unsigned short w[0x40];
    unsigned short n = wcslen(word);
    unsigned short *s;

    sc_fold_text(w, word);
    s = card_get_name(id);
    wtext_strip_tags(raw, s);
    sc_fold_text(txt, raw);
    if (sc_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    sc_fold_text(txt, raw);
    if (sc_find_text(txt, w, n))
        return 1;
    s = card_get_desc(id);
    wtext_strip_tags(raw, s);
    sc_fold_text(txt, raw);
    if (sc_find_text(txt, w, n))
        return 1;
    wtext_strip_tags2(raw, s);
    sc_fold_text(txt, raw);
    return sc_find_text(txt, w, n) != 0;
}

#ifdef NON_MATCHING /* laço da tabela de saltos desenrolado no original com outra alocação */
/* Horspool search of word (n chars) in text */
unsigned short *sc_find_text(unsigned short *text, unsigned short *word, int n)
{
    unsigned char *skip = heap_alloc(sc.heap, 0x10000);
    int i;
    int len;
    int j;
    int k;
    unsigned short *t;
    unsigned short *w;
    int d;

    if (skip == 0)
        return 0;
    memset(skip, n, 0x10000);
    for (i = 0; i < n - 1; i++)
        skip[word[i]] = n - i - 1;
    for (len = 0, t = text; *t != 0; t++)
        len++;
    for (k = n - 1; k < len; ) {
        t = &text[k];
        w = &word[n - 1];
        j = n - 1;
        if (*t == word[n - 1]) {
            do {
                if (j == 0) {
                    if (skip != 0)
                        heap_set_level(sc.heap, skip);
                    return &text[k];
                }
                t--;
                w--;
                k--;
                j--;
            } while (*t == *w);
        }
        d = skip[text[k]];
        if (d <= n - j)
            d = n - j;
        k += d;
    }
    if (skip != 0)
        heap_set_level(sc.heap, skip);
    return 0;
}
#endif

#ifdef NON_MATCHING /* teste do caractere no topo do laço com cópia do ponteiro */
/* upper case, full-width ASCII to half width, katakana to hiragana */
void sc_fold_text(unsigned short *dst, unsigned short *src)
{
    unsigned short c;

    for (c = *src; c != 0; c = *src) {
        src++;
        if (c > 0xFF00 && c < 0xFF5E)
            c += 0x120;
        if (c < 0x30A1 || c > 0x30F4)
            *dst = towupper(c);
        else
            *dst = c - 0x60;
        dst++;
    }
    *dst = 0;
}
#endif
