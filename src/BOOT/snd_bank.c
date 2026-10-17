// @range 0x0885DE90-0x0885E634
// @asm 0x0885DF70-0x0885DFE8
// @asm 0x0885E434-0x0885E4EC
// @asm 0x0885E524-0x0885E634
/* Sound library, bank side: the "SHDS" header of psp_snddat.bin (sound entries, groups, offsets and
   base ids), per-voice filter state, the 16-slot handle cache and the intrusive doubly linked lists
   used for active sound objects. */
#include "psp.h"
#include "libc.h"

typedef struct SndEntry {              /* 8 bytes per sound id */
    unsigned char kind;                /* low nibble: kind */
    unsigned char prio;                /* low 5 bits */
    unsigned char b2, b3;
    unsigned char flags4, flags5;
    unsigned char pad[2];
} SndEntry;

typedef struct SndGroup {              /* 0x10 per group */
    int data;                          /* loaded data or 0 */
    int unk4;
    int unk8;
    unsigned char state;
    unsigned char pad[3];
} SndGroup;

typedef struct SndBank {               /* "SHDS" */
    unsigned int magic;
    int unk4;
    unsigned short version;
    unsigned short pad0a;
    unsigned short n_groups_a;
    unsigned short n_entries_extra;
    unsigned short n_groups_b;
    unsigned short pad12;
    SndEntry *entries;                 /* +0x14 filled by snd_bank_init */
    SndGroup *groups;                  /* +0x18 */
    unsigned int *offsets;             /* +0x1C */
    unsigned short *base_ids;          /* +0x20 first sound id of each group */
    int pad24[4];
    unsigned char data[1];             /* +0x34 */
} SndBank;

typedef struct SndList {               /* intrusive list: links at node + link_off */
    void *head;
    void *tail;
    unsigned short count;
    unsigned short link_off;
} SndList;

typedef struct SndLink {
    void *prev;
    void *next;
} SndLink;

typedef struct SndHandle {             /* 16 cached handles */
    int data;
    int key;
    unsigned short id;
    signed char flags;                 /* bit 7: used */
    unsigned char pad;
    unsigned short refs;
    unsigned short pad2;
} SndHandle;

typedef struct SndCallbacks {
    int (*read)(void *name, void *buf, int size, int off);
    void *(*alloc)(int size);
    void (*free)(void *p);
    void *unk0C;
    void *unk10;
} SndCallbacks;

extern short snd_filter_hist[][16][2];     /* 0x08BF64E8 per voice */
extern unsigned char snd_voice_pcm[][0x6000]; /* 0x08BF6EC0 per voice */
extern SndBank *snd_bank;                  /* 0x08C1D88C */
extern int snd_n_entries;                  /* 0x08C1D8A8 */
extern int snd_n_groups;                   /* 0x08C1D8AC */
extern SndHandle snd_handles[16];          /* 0x08C1AEC0 */

SndGroup *snd_group_get(unsigned int g);
void *snd_list_next(SndList *l, void *node);

/* Clears the filter history and the PCM ring of the object's voice. */
void snd_voice_clear(unsigned char *obj)
{
    int v = (signed char)obj[0x30] - 0x1C;
    short (*h)[2] = snd_filter_hist[v];
    int i = 0;

    do {                                   /* `while (++i < n)` keeps -O4,p from unrolling the loop x8 */
        (*h)[1] = 0;
        (*h)[0] = 0;
        h++;
    } while (++i < 16);
    memset(snd_voice_pcm[v], 0, 0x4000);
}

int snd_entry_kind(unsigned int id)
{
    return snd_bank->entries[id & 0xFFFF].kind & 0xF;
}

int snd_entry_prio(unsigned int id)
{
    return snd_bank->entries[id & 0xFFFF].prio & 0x1F;
}

int snd_entry_b2(unsigned int id)
{
    return snd_bank->entries[id & 0xFFFF].b2;
}

int snd_entry_b3(unsigned int id)
{
    return snd_bank->entries[id & 0xFFFF].b3;
}

#ifdef NON_MATCHING /* register assignment of n/bank and the early-return layout */
/* Group containing sound id `id` (base_ids is sorted), -1 when none. */
int snd_group_of(unsigned int id)
{
    int n = snd_n_groups;
    SndBank *b = snd_bank;
    unsigned short key;
    int i;

    if (b == 0 || n == 0)
        return -1;
    i = -1;
    key = id;
    while (++i < n) {
        if (key < b->base_ids[i])
            return i - 1;
    }
    return i - 1;
}

#endif

int snd_group_data_of(unsigned int id)
{
    int g = snd_group_of(id);
    if (g < 0)
        return 0;
    return snd_bank->groups[g].data;
}

int snd_group_data(unsigned int g)
{
    return snd_bank->groups[g & 0xFFFF].data;
}

SndGroup *snd_group_get(unsigned int g)
{
    if (snd_bank == 0)
        return 0;
    if ((g & 0xFFFF) >= snd_n_groups)
        return 0;
    return &snd_bank->groups[g & 0xFFFF];
}

unsigned int snd_bank_offset(unsigned int g)
{
    if (snd_bank == 0)
        return 0;
    if ((g & 0xFFFF) >= snd_n_groups)
        return 0;
    return snd_bank->offsets[g & 0xFFFF];
}

int snd_group_unk8(unsigned int g)
{
    return ((SndGroup *)((g & 0xFFFF) * 0x10 + (unsigned char *)snd_bank->groups))->unk8;
}

int snd_group_base_id(unsigned int id)
{
    int g = snd_group_of(id);
    if (g < 0)
        return -1;
    return snd_bank->base_ids[g];
}

/* 0x20-byte descriptor of sound `id` inside its loaded group data. */
unsigned char *snd_desc(unsigned int id)
{
    int base = snd_group_base_id(id);
    int data = snd_group_data_of(id);

    if (base < 0 || data == 0)
        return 0;
    return (unsigned char *)data + *(int *)(data + 8) + ((id & 0xFFFF) - base) * 0x20;
}

unsigned char *snd_data_ptr_c(unsigned char *data, unsigned char *desc, int i)
{
    return data + *(int *)(data + 0xC) + *(int *)(desc + 8) + i * 8;
}

unsigned char *snd_data_ptr_18(unsigned char *data, unsigned char *desc)
{
    return data + *(int *)(data + 0x18) + *(int *)(desc + 4);
}

unsigned char *snd_data_ptr_10(unsigned char *data, int *desc)
{
    return data + *(int *)(data + 0x10) + *desc;
}

int snd_entry_flags4(unsigned int id, int mask)
{
    return mask & snd_bank->entries[id & 0xFFFF].flags4;
}

int snd_desc_type(unsigned int id)
{
    signed char *d = (signed char *)snd_desc(id);
    if (d == 0)
        return 0;
    return *d;
}

int snd_desc_b2(unsigned int id)
{
    unsigned char *d = snd_desc(id);
    if (d == 0)
        return -1;
    return d[2];
}

int snd_entry_flags5(unsigned int id, int mask)
{
    return mask & snd_bank->entries[id & 0xFFFF].flags5;
}

int snd_entry_b5(unsigned int id)
{
    return snd_bank->entries[id & 0xFFFF].flags5;
}

int snd_desc_b7(unsigned int id)
{
    unsigned char *d = snd_desc(id);
    if (d == 0)
        return -1;
    return d[7];
}

void snd_handles_reset(void)
{
    SndHandle *h = snd_handles;
    int i = 0;

    do {
        h->key = 0;
        h->id = 0;
        h++->flags = 0;
    } while (++i < 16);
}

SndHandle *snd_handle_find(int data)
{
    SndHandle *h = snd_handles;
    int i = 0;

    do {
        if ((h->flags & 0x80) && h->data == data)
            return h;
        h++;
    } while (++i < 16);
    return 0;
}

/* Finds the handle for (id, key) or takes the lowest free one. */
SndHandle *snd_handle_get(int id, int key, int data)
{
    SndHandle *free = 0;
    SndHandle *h = &snd_handles[15];
    int i = 15;

    do {
        if (!(h->flags & 0x80))
            free = h;
        else if (h->id == id && h->key == key)
            return h;
        h--;
    } while (--i >= 0);
    if (free == 0)
        return 0;
    free->data = data;
    free->key = key;
    free->id = id;
    free->refs = 1;
    free->flags = 0x80;
    return free;
}

void snd_handle_release(SndHandle *h)
{
    h->flags = 0;
}

void snd_list_init(SndList *l, unsigned short link_off)
{
    l->head = 0;
    l->tail = 0;
    l->count = 0;
    l->link_off = link_off;
}

int snd_list_contains(SndList *l, void *node)
{
    void *p = 0;

    do {
        p = snd_list_next(l, p);
        if (p == 0)
            return 0;
    } while (p != node);
    return 1;
}

#ifdef NON_MATCHING /* load order of tail/link_off (one addu operand swap) */
void snd_list_append(SndList *l, void *node)
{
    SndLink *k = (SndLink *)((unsigned char *)node + l->link_off);

    if (l->head == 0) {
        k->next = 0;
        k->prev = 0;
        l->head = node;
        l->tail = node;
    } else {
        k->prev = l->tail;
        k->next = 0;
        ((SndLink *)((unsigned char *)l->tail + l->link_off))->next = node;
        l->tail = node;
    }
    l->count++;
}

void snd_list_remove(SndList *l, void *node)
{
    SndLink *k = (SndLink *)((unsigned char *)node + l->link_off);

    if (k->prev == 0)
        l->head = k->next;
    else
        ((SndLink *)((unsigned char *)k->prev + l->link_off))->next = k->next;
    if (k->next == 0)
        l->tail = k->prev;
    else
        ((SndLink *)((unsigned char *)k->next + l->link_off))->prev = k->prev;
    k->next = 0;
    k->prev = 0;
    l->count--;
}

#endif

void *snd_list_next(SndList *l, void *node)
{
    if (node == 0)
        return l->head;
    return ((SndLink *)((unsigned char *)node + l->link_off))->next;
}

void snd_callbacks_default(SndCallbacks *cb)
{
    cb->read = 0;
    cb->alloc = 0;
    cb->free = 0;
    cb->unk0C = 0;
    cb->unk10 = 0;
}

#ifdef NON_MATCHING /* table pointer arithmetic order */
/* Validates the "SHDS" header (version 0x2x), resolves its tables and clears the groups. */
int snd_bank_init(SndBank *b)
{
    SndGroup *g;
    unsigned char *p;
    int i;

    if (b->magic != 0x53444853)
        return 0;
    if ((b->version & 0xFFF0) != 0x20)
        return 0;
    snd_n_groups = b->n_groups_a + b->n_groups_b;
    p = b->data;
    snd_n_entries = snd_n_groups + b->n_entries_extra;
    b->entries = (SndEntry *)p;
    p += snd_n_entries * 8;
    b->groups = (SndGroup *)p;
    b->offsets = (unsigned int *)(p + snd_n_groups * 0x10);
    b->base_ids = (unsigned short *)(p + snd_n_groups * 0x10 + snd_n_groups * 4);
    snd_bank = b;
    for (i = 0; i < snd_n_groups; i++) {
        g = snd_group_get(i);
        if (g != 0) {
            g->unk4 = 0;
            g->data = 0;
            g->state = 0;
        }
    }
    return 1;
}
#endif
