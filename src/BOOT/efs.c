// @range 0x0882923C-0x0882AD90
// @asm 0x088292C0-0x08829308
// @asm 0x08829798-0x08829AC4
// @asm 0x08829B10-0x08829C00
// @asm 0x08829C20-0x08829F60
// @asm 0x0882A538-0x0882A99C
// @data 0x088ED758-0x088ED770
/* EFS effects: .efs files hold trees of commands (the EHF command handlers) under named roots.
   Loading relocates a file and binds every command id to its handler; an instance of a root gets
   memory from the effect heap laid out by the handlers (phase 1 sizes, phase 2 lays out), is kept
   in a circular list and is run every frame (phase 5 draw, 6 after the children, 4 free; other
   phases are messages filtered by command id). Commands 1 and 2 nest another root by name. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

typedef struct EfsCmd {
    union {
        unsigned int id;               /* low 24 bits */
        struct {
            unsigned char pad[3];
            unsigned char argc;        /* +0x03 */
        } b;
    } h;
    unsigned int (*fn)(void *mem, EhfNode *node, struct EfsCmd *cmd, int phase); /* +0x04 */
    struct EfsCmd *child;              /* +0x08 */
    struct EfsCmd *next;               /* +0x0C a root: its instance */
    union {
        int i[1];
        void *p[1];
    } a;                               /* +0x10 */
} EfsCmd;

typedef struct EfsMem {                /* per command, followed by the handler's memory */
    struct EfsMem *child;
    struct EfsMem *next;               /* +0x04 */
    int pad[2];
    int data[1];                       /* +0x10 */
} EfsMem;

typedef struct EfsFile {
    int pad0[4];
    int tab;                           /* +0x10 sorted root table (offset, then pointers) */
    int pad14;
    int base;                          /* +0x18 relocation base */
    int pad1c;
    int str;                           /* +0x20 names */
    int pad24;
    int data;                          /* +0x28 */
    int pad2c;
    int reloc;                         /* +0x30 relocation byte code */
    int pad34;
    int n;                             /* +0x38 roots */
    int loaded;                        /* +0x3C */
} EfsFile;

typedef struct EfsNode {               /* EhfNode with its instance */
    int frame;
    int msg;                           /* +0x04 command id a message is for (0 all) */
    float time;                        /* +0x08 */
    struct EfsInst *inst;              /* +0x0C */
} EfsNode;

typedef struct EfsInst {               /* 0x30 */
    struct EfsInst *prev;
    struct EfsInst *next;              /* +0x04 */
    void *pack;                        /* +0x08 */
    void *heap;                        /* +0x0C */
    EfsFile *file;                     /* +0x10 */
    void *mem;                         /* +0x14 */
    EfsCmd *root;                      /* +0x18 */
    unsigned char argq;                /* +0x1C argument words, in 16-byte units */
    unsigned char memoff;              /* +0x1D command memory, in 16-byte units */
    unsigned char nargs;               /* +0x1E */
    unsigned char pad;
    EfsNode node;                      /* +0x20 */
} EfsInst;

typedef struct EfsRef {
    void *pack;
    EfsFile *file;
    EfsCmd *cmd;
} EfsRef;

typedef struct EfsCmdDef {
    unsigned int id;
    void *fn;
} EfsCmdDef;

typedef struct EfsMgr {               /* 0x088FA040 */
    EfsFile *files[48];
    void *packs[48];                   /* +0x0C0 */
    EfsInst insts[0x300];              /* +0x180 */
    void *heap;                        /* +0x9180 */
    int heap_ext;                      /* +0x9184 heap given by the caller */
    EfsInst *list;                     /* +0x9188 */
    EfsInst *cur;                      /* +0x918C next to run */
    int kill;                          /* +0x9190 the running instance ends */
    int hold;                          /* +0x9194 the running instance keeps its frame */
    int cmds_sorted;                   /* +0x9198 */
    int ncmds;                         /* +0x919C */
} EfsMgr;

extern EfsMgr efs;
extern EfsCmdDef efs_cmd_table[];      /* 0x088ED4D0 */
extern char efs_heap_hdr[];            /* 0x088FA030 */

int ehp_add_handler(char *ext, void *fn);
void *heap_create(void *mem, int size);
int heap_free_block(void *heap);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
void heap_stats(void *heap, int *used, int *freed);
void mtx_push(void);

int efs_load(int load, EfsFile *f, int size, void *pack);
int efs_stub_init(void);
int efs_inst_init(EfsInst *inst, EfsRef *r, char *fmt, void *args);
int efs_set_args(void **dst, char *fmt, int *src);
unsigned int efs_run(EfsMem *mem, EfsNode *node, EfsCmd *cmd, int phase);
void efs_resolve(EfsCmd *cmd, EfsNode *node);
void *efs_cmd_lookup(unsigned int id);
int efs_find(EfsRef *out, char *name);
int efs_inst_free(EfsInst *inst);

int efs_init(void)
{
    ehp_add_handler(".efs", efs_load);
    efs_stub_init();
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    return 1;
}

int efs_heap_init(void)
{
    EfsMgr *e = &efs;
    memset(e, 0, 0x91A0);
    e->heap = heap_create(efs_heap_hdr, 0x10);
    return 1;
}

#ifdef NON_MATCHING /* the constant 1 lands in $v0 instead of $a0 */
void efs_set_heap(void *heap)
{
    EfsMgr *e = &efs;
    memset(e, 0, 0x91A0);
    e->heap_ext = 1;
    e->heap = heap;
}
#endif

int efs_term(void)
{
    int i;

    for (i = 0; i < 0x300; i++) {
        EfsInst *inst = &efs.insts[i];

        if (inst->mem != 0) {
            efs_inst_free(inst);
        }
    }
    if (efs.heap != 0 && efs.heap_ext == 0) {
        heap_free_block(efs.heap);
    }
    memset(&efs, 0, 0x91A0);
    return 1;
}

/* Runs every instance for the frame; an instance that set efs.kill is freed. */
int efs_update(void)
{
    EfsMgr *e = &efs;

    if (e->list != 0) {
        EfsInst *inst;
        int last;

        mtx_set_mode(2);
        mtx_push();
        mtx_identity();
        inst = e->list;
        last = 0;
        do {
            e->kill = 0;
            e->hold = 0;
            if (inst->root->h.b.argc > 0 && inst->argq != 0) {
                efs_set_args(&inst->root->a.p[0], (char *)inst->mem + inst->argq * 16, inst->mem);
            }
            efs_run((EfsMem *)((char *)inst->mem + inst->memoff * 16), &inst->node, inst->root->child, 5);
            if (e->hold == 0) {
                float t = inst->node.time + 1.0f;

                inst->node.msg = t - inst->node.frame;
                if (t >= 65536.0f) {
                    t -= 65536.0f;
                }
                inst->node.time = t;
                inst->node.frame = t;
            }
            if (inst->next == e->list) {
                last = 1;
            }
            e->cur = inst->next;
            if (efs.kill != 0) {
                efs_inst_free(inst);
            }
        inst = e->cur;
        } while (!last && e->list != 0);
        mtx_pop();
    }
    return 1;
}

/* New instance of the root `name` (arguments as for efs_set_args), linked at the end of the list. */
EfsInst *efs_start(char *name, char *fmt, void *args)
{
    int i;
    EfsInst *inst;
    EfsRef r;

    for (i = 0; i < 0x300; i++) {
        if (efs.insts[i].mem == 0) {
            goto found;
        }
    }
    return 0;
found:
    inst = &efs.insts[i];
    memset(inst, 0, 0x30);
    if (efs_find(&r, name) == 0) {
        return 0;
    }
    if (fmt == 0) {
        fmt = "";
    }
    if (efs_inst_init(inst, &r, fmt, args) < 0) {
        return 0;
    }
    if (efs.list == 0) {
        efs.list = inst;
        inst->prev = inst;
        inst->next = inst;
    } else {
        inst->prev = efs.list->prev;
        inst->next = efs.list;
        inst->prev->next = inst;
        inst->next->prev = inst;
    }
    return inst;
}

int efs_inst_free(EfsInst *inst)
{
    EfsMgr *e = &efs;

    if (inst == 0 || inst->mem == 0) {
        return 1;
    }
    efs_run((EfsMem *)((char *)inst->mem + inst->memoff * 16), &inst->node, inst->root->child, 4);
    heap_set_level(inst->heap, inst->mem);
    inst->root = 0;
    inst->mem = 0;
    if (inst == e->cur) {
        if (inst->next == inst) {
            e->cur = 0;
        } else {
            e->cur = inst->next;
        }
    }
    if (inst->next != 0) {
        if (inst->next == inst) {
            inst->prev = 0;
            inst->next = 0;
            efs.list = 0;
        } else if (inst == efs.list) {
            efs.list = inst->next;
            inst->prev->next = inst->next;
            inst->next->prev = inst->prev;
            inst->next = 0;
            inst->prev = 0;
        } else {
            inst->prev->next = inst->next;
            inst->next->prev = inst->prev;
            inst->next = 0;
            inst->prev = 0;
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* the instance goes to $s1 through a handle parameter; one store order differs */
/* Sends message `phase` (with argument words a, b) to the commands of the instance. */
int efs_send(void *h, int phase, int a, int b)
{
    EfsInst *inst = h;
    EfsNode n;
    int kill;

    if (inst == 0) {
        return 0;
    }
    if (inst->mem == 0) {
        return 0;
    }
    kill = efs.kill;
    n.frame = b;
    n.msg = a;
    n.time = 0;
    n.inst = inst;
    efs.kill = 0;
    if (inst->root->h.b.argc > 0 && inst->argq != 0) {
        efs_set_args(&inst->root->a.p[0], (char *)inst->mem + inst->argq * 16, inst->mem);
    }
    efs_run((EfsMem *)((char *)inst->mem + inst->memoff * 16), &n, inst->root->child, phase);
    if (efs.kill != 0) {
        efs_inst_free(inst);
    }
    efs.kill = kill;
    return 1;
}
#endif

#ifdef NON_MATCHING /* one addu operand order (index + table) */
/* Root `name` in the loaded files (binary search in each file's sorted table). */
int efs_find(EfsRef *out, char *name)
{
    int i;

    for (i = 0; i < 48; i++) {
        EfsFile *f = efs.files[i];

        if (f != 0) {
            EfsCmd **tab = (EfsCmd **)((char *)f + f->tab);
            int lo = 0;
            int hi = f->n + 1;
            char *str = (char *)f + f->str;
            int mid;

            for (mid = hi / 2; lo < mid; mid = (lo + hi) / 2) {
                int c = strcmp(str + (tab[mid - 1]->h.id & 0xFFFFFF), name);

                if (c == 0) {
                    out->pack = efs.packs[i];
                    out->file = efs.files[i];
                    out->cmd = tab[mid - 1];
                    return 1;
                }
                if (c < 0) {
                    lo = mid;
                } else {
                    hi = mid;
                }
            }
        }
    }
    memset(out, 0, 0xC);
    return 0;
}
#endif

#ifdef NON_MATCHING /* the name offset is loaded before the table offset */
/* Calls fn(name, arg) for every root of the loaded files. */
void efs_foreach(void (*fn)(char *name, void *arg), void *arg)
{
    int i;

    for (i = 0; i < 48; i++) {
        EfsFile *f = efs.files[i];

        if (f != 0) {
            char *str = (char *)f + f->str;
            int k;
            EfsCmd **tab = (EfsCmd **)((char *)f + f->tab);

            for (k = 0; k < f->n; k++, tab++) {
                fn(str + ((*tab)->h.id & 0xFFFFFF), arg);
            }
        }
    }
}
#endif

int efs_heap_used(void)
{
    int used;
    EfsMgr *e = &efs;

    if (efs.heap == 0) {
        return 0;
    }
    heap_stats(e->heap, &used, 0);
    return used;
}

#ifdef NON_MATCHING /* register allocation of the search (id in $t0) */
/* Replaces handlers of the command table: {id, fn} pairs up to id -1 or a null fn. */
void efs_cmds_override(EfsCmdDef *d)
{
    EfsMgr *e = &efs;
    efs_cmd_lookup(1);
    for (; d->id != -1 && d->fn != 0; d++) {
        int lo = -1;
        int hi = e->ncmds;
        int mid = (hi - 1) / 2;

        while (lo < mid && mid < hi) {
            if (d->id == efs_cmd_table[mid].id) {
                efs_cmd_table[mid].fn = d->fn;
                break;
            }
            if (d->id < efs_cmd_table[mid].id) {
                hi = mid;
            } else {
                lo = mid;
            }
            mid = (lo + hi) / 2;
        }
    }
}
#endif

void efs_set_kill(void)
{
    EfsMgr *e = &efs;
    e->kill = 1;
}

void efs_set_hold(void)
{
    EfsMgr *e = &efs;
    e->hold = 1;
}

#ifdef NON_MATCHING /* register allocation (phase in $s1, the instance shares $s0 with the argument pointer) */
/* 1: instance of the root named by arg0, with argument format arg1 and words from arg2. */
int efs_cmd_inst(EfsInst *inst, EfsNode *node, EfsCmd *cmd, int phase)
{
    EfsRef r;

    switch (phase) {
    case 0: {
        void **a = cmd->a.p;

        if (efs_find(&r, a[0]) == 0) {
            a[0] = 0;
            return 0;
        }
        a[0] = r.cmd;
        return 0;
    }
    case 1:
        return 0x30;
    case 2: {
        void **a = cmd->a.p;

        memset(inst, 0, 0x30);
        if (a[0] == 0) {
            return 0x30;
        }
        r.pack = node->inst->pack;
        r.file = node->inst->file;
        r.cmd = a[0];
        if (cmd->h.b.argc < 3) {
            efs_inst_init(inst, &r, 0, 0);
            return 0x30;
        }
        efs_inst_init(inst, &r, a[1], &a[2]);
        return 0x30;
    }
    case 4:
        if (inst->root != 0) {
            efs_inst_free(inst);
            return 0;
        }
        return 0;
    case 5:
    default:
        if (inst->root == 0) {
            return 0;
        }
        if (inst->root->h.b.argc > 0) {
            efs_set_args(&inst->root->a.p[0], cmd->a.p[1], (int *)&cmd->a.p[2]);
        }
        efs_run((EfsMem *)((char *)inst->mem + inst->memoff * 16), node, inst->root->child, phase);
        return 0;
    }
}
#endif

#ifdef NON_MATCHING /* case 2 stores the instance from a separate block */
/* 2: runs the commands of the existing instance of the root named by arg0. */
int efs_cmd_ref(EfsInst **p, EfsNode *node, EfsCmd *cmd, int phase)
{
    switch (phase) {
    case 0: {
        void **a = cmd->a.p;
        EfsRef r;

        if (efs_find(&r, a[0]) == 0) {
            a[0] = 0;
            return 0;
        }
        a[0] = r.cmd;
        return 0;
    }
    case 1:
        return 4;
    case 2: {
        EfsCmd *root = cmd->a.p[0];

        if (root == 0) {
            return 4;
        }
        if (root->next != 0) {
            *p = (EfsInst *)root->next;
        }
        return 4;
    }
    case 4:
        return 0;
    case 5: {
        EfsNode n = *node;

        if ((*p)->root->h.b.argc > 0) {
            efs_set_args(&(*p)->root->a.p[0], cmd->a.p[1], (int *)&cmd->a.p[2]);
        }
        n.msg = 0;
        efs_run((EfsMem *)((char *)(*p)->mem + (*p)->memoff * 16), &n, (*p)->root->child, phase);
        return 0;
    }
    default:
        if ((*p)->root->h.b.argc > 0) {
            efs_set_args(&(*p)->root->a.p[0], cmd->a.p[1], (int *)&cmd->a.p[2]);
        }
        efs_run((EfsMem *)((char *)(*p)->mem + (*p)->memoff * 16), node, (*p)->root->child, phase);
        return 0;
    }
}
#endif

/* 3: ends the instance at frame arg0 (< 0: at once). */
int efs_cmd_end(void *mem, EfsNode *node, EfsCmd *cmd, int phase)
{
    switch (phase) {
    case 5:
        if (cmd->a.i[0] < 0 || cmd->a.i[0] == node->frame) {
            efs_set_kill();
        }
        return 0;
    }
    return 0;
}

/* .efs handler: relocates the file, registers it and binds its commands; unloading frees its
   instances. */
int efs_load(int load, EfsFile *f, int size, void *pack)
{
    int i;

    if (load != 0) {
        unsigned int *end;
        unsigned char *bc;
        char *str;
        char *data;
        unsigned int *p;
        unsigned int *base;
        unsigned int c;
        unsigned int skip;
        EfsInst fake;

        if (f->loaded != 0) {
            return 1;
        }
        f->loaded = 1;
        end = (unsigned int *)((char *)f + f->tab) + f->n;
        base = (unsigned int *)((char *)f + f->base);
        str = (char *)f + f->str;
        data = (char *)f + f->data;
        bc = (unsigned char *)f + f->reloc;
        p = base;
        while ((c = *bc) != 0) {
            unsigned int v;

            if (c & 0x80) {
                skip = bc[1] + ((c & ~0x80) << 8);
                bc += 2;
            } else {
                skip = c & ~0x80;
                bc++;
            }
            p += skip;
            v = *p;
            switch (v & 0xF0000000) {
            case 0x10000000:
                *p = (unsigned int)p + (v & 0xFFFFFFF);
                break;
            case 0x20000000:
                *p = (unsigned int)str + (v & 0xFFFFFFF);
                break;
            case 0x30000000:
                *p = (unsigned int)data + (v & 0xFFFFFFF);
                break;
            }
        }
        for (i = 0; i < 48; i++) {
            if (efs.files[i] == 0) {
                efs.files[i] = f;
                efs.packs[i] = pack;
                break;
            }
        }
        if (i == 48) {
            return 0;
        }
        for (p = (unsigned int *)((char *)f + f->tab); p < end; p++) {
            *p = (unsigned int)base + (*p & 0xFFFFFFF);
        }
        memset(&fake, 0, 0x30);
        fake.node.inst = &fake;
        fake.pack = pack;
        fake.file = f;
        for (p = (unsigned int *)((char *)f + f->tab); p < end; p++) {
            fake.root = (EfsCmd *)*p;
            efs_resolve(fake.root->child, &fake.node);
        }
        return 1;
    }
    for (i = 0; i < 48; i++) {
        if (efs.files[i] == f) {
            int k;

            for (k = 0; k < 0x300; k++) {
                EfsInst *inst = &efs.insts[k];

                if (inst->mem != 0 && inst->file == f) {
                    efs_inst_free(inst);
                }
            }
            efs.files[i] = 0;
            return 1;
        }
    }
    return 0;
}

/* Copies argument words from src into the places the root's argument pointers name, by format:
   M matrix, V/I vector, S/F/N word inline; lower case: pointer to it. Returns the words used. */
int efs_set_args(void **dst, char *fmt, int *src)
{
    int n = 0;

    for (; *fmt != 0; fmt++) {
        switch (*fmt) {
        case 'n':
        case 'f':
        case 's':
            *(int *)*dst = *(int *)*src;
            dst++;
            src++;
            n++;
            break;
        case 'i':
        case 'v':
            memcpy(*dst, (void *)*src, 0x10);
            dst++;
            src++;
            n++;
            break;
        case 'm':
            memcpy(*dst, (void *)*src, 0x40);
            dst++;
            src++;
            n++;
            break;
        case 'N':
        case 'F':
        case 'S':
            *(int *)*dst = *src;
            dst++;
            src++;
            n++;
            break;
        case 'I':
        case 'V':
            memcpy(*dst, src, 0x10);
            dst++;
            src += 4;
            n += 4;
            break;
        case 'M':
            memcpy(*dst, src, 0x40);
            dst++;
            src += 16;
            n += 16;
            break;
        }
    }
    return n;
}

#ifdef NON_MATCHING /* the original keeps the blocks emptied by branch-likely filling (case entries and loop continuations) */
/* Runs a command list with its memory: phase 1 returns the memory size, 2 lays it out, 4 frees,
   5 draws (handler flags: 1 skip children, 2 pop the matrix, 8 pop the projection too, 0x80000000
   call again with 6 after the children); other phases are messages for node->msg. */
unsigned int efs_run(EfsMem *mem, EfsNode *node, EfsCmd *cmd, int phase)
{
    EfsMem *m = mem;
    EfsNode n;

    switch (phase) {
    case 1: {
        unsigned int size = 0;

        for (;;) {
            n = *node;
            size = (size + 0x10 + 0xF) & ~0xF;
            if (cmd->fn != 0) {
                int r = cmd->fn(0, (EhfNode *)&n, cmd, 1);

                if (r < 0) {
                    return -1;
                }
                size += r;
            }
            size = (size + 0xF) & ~0xF;
            if (cmd->child != 0) {
                int r = efs_run(0, &n, cmd->child, 1);

                if (r < 0) {
                    return -1;
                }
                size = (size + r + 0xF) & ~0xF;
            }
            cmd = cmd->next;
            if (cmd == 0) {
                return size;
            }
        }
    }
    case 2: {
        unsigned int size = 0;

        for (;;) {
            unsigned int r;

            n = *node;
            size = (size + 0x10 + 0xF) & ~0xF;
            r = cmd->fn(m->data, (EhfNode *)&n, cmd, 2);
            size = (size + (r & 0x7FFFFFFF) + 0xF) & ~0xF;
            if (cmd->child != 0) {
                m->child = (EfsMem *)((char *)mem + size);
                size = (size + efs_run(m->child, &n, cmd->child, 2) + 0xF) & ~0xF;
            } else {
                m->child = 0;
            }
            if (r & 0x80000000) {
                cmd->fn(m->data, (EhfNode *)&n, cmd, 3);
            }
            if (cmd->next == 0) {
                m->next = 0;
                return size;
            }
            m->next = (EfsMem *)((char *)mem + size);
            cmd = cmd->next;
            m = (EfsMem *)((char *)mem + size);
        }
    }
    case 4:
        for (;;) {
            n = *node;
            if (cmd->child != 0) {
                efs_run(m->child, &n, cmd->child, phase);
            }
            cmd->fn(m->data, (EhfNode *)&n, cmd, phase);
            cmd = cmd->next;
            if (cmd == 0) {
                return 0;
            }
            m = m->next;
        }
    case 5:
        for (;;) {
            unsigned int r;

            n = *node;
            r = cmd->fn(m->data, (EhfNode *)&n, cmd, phase);
            if (cmd->child != 0 && !(r & 1)) {
                efs_run(m->child, &n, cmd->child, phase);
            }
            if (r & 2) {
                mtx_pop();
            }
            if (r & 8) {
                mtx_set_mode(1);
                mtx_pop();
                mtx_set_mode(2);
            }
            if (r & 0x80000000) {
                cmd->fn(m->data, (EhfNode *)&n, cmd, 6);
            }
            cmd = cmd->next;
            if (cmd == 0) {
                return 0;
            }
            m = m->next;
        }
    default:
        for (;;) {
            EfsNode n2 = *node;
            unsigned int r = 0;

            if (n2.msg == 0 || n2.msg == (cmd->h.id & 0xFFFFFF) || (cmd->h.id & 0xFFFFFF) - 1 < 2) {
                r = cmd->fn(m->data, (EhfNode *)&n2, cmd, phase);
            }
            if (cmd->child != 0 && !(r & 1)) {
                efs_run(m->child, &n2, cmd->child, phase);
            }
            if (r & 0x80000000) {
                cmd->fn(m->data, (EhfNode *)&n2, cmd, 7);
            }
            cmd = cmd->next;
            if (cmd == 0) {
                return 0;
            }
            m = m->next;
        }
    }
}
#endif

/* Instance memory: argument words and format kept after the commands' memory when given. */
int efs_inst_init(EfsInst *inst, EfsRef *r, char *fmt, void *args)
{
    int size;
    int total;
    int n;

    inst->pack = r->pack;
    inst->file = r->file;
    inst->root = r->cmd;
    inst->node.frame = 0;
    inst->node.inst = inst;
    inst->node.msg = 1;
    if (fmt != 0 && args != 0) {
        n = efs_set_args(&inst->root->a.p[0], fmt, args);
    } else {
        n = 0;
    }
    size = efs_run(0, &inst->node, inst->root->child, 1);
    if (size < 0) {
        return -1;
    }
    total = size + ((n + 3) & ~3) * 4;
    if (fmt != 0) {
        total += (strlen(fmt) + 0x10) & ~0xF;
    }
    inst->mem = heap_alloc(efs.heap, total);
    inst->heap = efs.heap;
    inst->nargs = n;
    if (inst->mem == 0) {
        inst->root = 0;
        return -1;
    }
    if (inst->root->h.b.argc > 0 && args != 0) {
        inst->argq = ((inst->nargs + 3) & ~3) >> 2;
        inst->memoff = inst->argq + (((strlen(fmt) + 0x10) & ~0xF) >> 4);
        memcpy(inst->mem, args, inst->nargs << 2);
        strcpy((char *)inst->mem + inst->argq * 16, fmt);
        efs_set_args(&inst->root->a.p[0], (char *)inst->mem + inst->argq * 16, inst->mem);
    } else {
        inst->argq = 0;
        inst->memoff = 0;
    }
    efs_run((EfsMem *)((char *)inst->mem + inst->memoff * 16), &inst->node, inst->root->child, 2);
    r->cmd->next = (EfsCmd *)inst;
    return size;
}

/* Binds every command of the list to its handler and lets it see the file (phase 0). */
void efs_resolve(EfsCmd *cmd, EfsNode *node)
{
    for (; cmd != 0; cmd = cmd->next) {
        cmd->fn = efs_cmd_lookup(cmd->h.id & 0xFFFFFF);
        cmd->fn(0, (EhfNode *)node, cmd, 0);
        if (cmd->child != 0) {
            efs_resolve(cmd->child, node);
        }
    }
}

int efs_cmd_cmp(const void *a, const void *b)
{
    return ((EfsCmdDef *)a)->id - ((EfsCmdDef *)b)->id;
}

/* Unknown commands: skip their children when drawing. */
unsigned int efs_cmd_none(void *mem, EhfNode *node, EfsCmd *cmd, int phase)
{
    switch (phase) {
    case 5:
        return 1;
    }
    return 0;
}

/* Handler of a command id (the table is sorted on first use). */
void *efs_cmd_lookup(unsigned int id)
{
    EfsMgr *e = &efs;
    int lo;
    int hi;
    int mid;

    if (efs.cmds_sorted == 0) {
        int n = 0;

        e->cmds_sorted = 1;
        while (efs_cmd_table[n].fn != 0) {
            n++;
        }
        e->ncmds = n;
        qsort(efs_cmd_table, n, 8, efs_cmd_cmp);
    }
    lo = -1;
    hi = e->ncmds;
    mid = (hi - 1) / 2;
    while (lo < mid && mid < hi) {
        if (id == efs_cmd_table[mid].id) {
            return efs_cmd_table[mid].fn;
        }
        if (id < efs_cmd_table[mid].id) {
            hi = mid;
        } else {
            lo = mid;
        }
        mid = (lo + hi) / 2;
    }
    return efs_cmd_none;
}

int efs_stub_init(void)
{
    return 1;
}
