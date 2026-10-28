// @range 0x08810AE4-0x08810D24
/* Story save variables (st_sv, a 0x18 block of the save: chapter, alternate time flag, crowd kind...)
   and the three script tables of the event pack (ES, EI, EL) with the lookup of a script by id. */
#include "libstory.h"

typedef struct StSv {                  /* *st_sv, 0x18 (in the save) */
    int chapter;                       /* +0x00 0..199 */
    int alt;                           /* +0x04 alternate placement of the characters */
    int x08;                           /* +0x08 */
    int x0c;                           /* +0x0C */
    unsigned short crowd;              /* +0x10 kind of crowd day (1 festival / tournament) */
    int x14;                           /* +0x14 */
} StSv;

typedef struct StIso {                 /* 0x08AC9348 script tables of the event pack, 0x10 */
    StoryCmd *es;                      /* +0x0 */
    StoryCmd *es_cur;                  /* +0x4 last script found */
    StoryCmd *ei;                      /* +0x8 */
    StoryCmd *el;                      /* +0xC */
} StIso;

extern StSv *st_sv;                    /* 0x08AC9344 */
extern StIso st_iso;                   /* 0x08AC9348 */

void st_init_810ae4(void)
{
    memset(&st_iso, 0, sizeof(StIso));
}

void st_term_810af8(void)
{
    memset(&st_iso, 0, sizeof(StIso));
}

void st_sv_set_ptr(StSv *p)
{
    st_sv = p;
}

void st_sv_clear(void)
{
    memset(st_sv, 0, sizeof(StSv));
}

void st_iso_set_es(StoryCmd *p)
{
    st_iso.es = p;
}

StoryCmd *st_iso_get_es(void)
{
    StIso *p = &st_iso;

    if (st_iso.es == 0)
        return 0;
    return p->es;
}

/* the script of the ES table whose first instruction has op == id (the table must hold it) */
StoryCmd *st_iso_find_es(int id)
{
    StIso *p = &st_iso;
    int i;
    int off;

    if (st_iso.es == 0)
        return 0;
    for (i = 0, off = 0;; off += sizeof(StoryCmd), i++) {
        unsigned short op = (p->es_cur = (StoryCmd *)((char *)p->es + off))->op;
        if (op == id)
            return p->es_cur = p->es + i;
    }
}

void st_iso_set_ei(StoryCmd *p)
{
    st_iso.ei = p;
}

StoryCmd *st_iso_get_ei(void)
{
    StIso *p = &st_iso;

    if (st_iso.ei == 0)
        return 0;
    return p->ei;
}

void st_iso_set_el(StoryCmd *p)
{
    st_iso.el = p;
}

StoryCmd *st_iso_get_el(void)
{
    StIso *p = &st_iso;

    if (st_iso.el == 0)
        return 0;
    return p->el;
}

void st_sv_set_chapter(int ch)
{
    StSv *s = st_sv;

    if (ch < 0 || ch >= 200)
        return;
    s->chapter = ch;
}

int st_sv_chapter(void)
{
    int ch = st_sv->chapter;

    if (ch < 0 || ch >= 200)
        ch = 0;
    return ch;
}

void st_sv_set_alt(int v)
{
    st_sv->alt = v;
}

int st_sv_alt(void)
{
    return st_sv->alt;
}

void st_sv_set_08(int v)
{
    st_sv->x08 = v;
}

int st_sv_get_08(void)
{
    return st_sv->x08;
}

void st_sv_set_0c(int v)
{
    st_sv->x0c = v;
}

void st_sv_set_crowd(int v)
{
    st_sv->crowd = v;
}

int st_sv_get_0c(void)
{
    return st_sv->x0c;
}

unsigned short st_sv_crowd(void)
{
    return st_sv->crowd;
}

void st_sv_set_14(int v)
{
    st_sv->x14 = v;
}

int st_sv_get_14(void)
{
    return st_sv->x14;
}
