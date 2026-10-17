/* ehfile: BOOT's file subsystem (worker thread + request ring + UMD LBN table). State lives in .bss
   (symbols/BOOT.txt: ehfile_state 0x08903410, ehfile_lbn 0x089033F8); see docs/architecture.md. */
#ifndef EHFILE_H
#define EHFILE_H

typedef int (*EhfileCallback)(void *arg, int result);

typedef struct EhfileReq {
    int active;
    int type;            /* 0 = read file, 1 = load module */
    int dir;             /* index into EhfileState.dirs */
    void *buf;
    int offset;
    int size;
    EhfileCallback cb;   /* called until it returns nonzero */
    void *arg;
    char name[32];
} EhfileReq; /* 0x40 */

typedef struct EhfileState {
    const char **dirs;
    int thread;
    EhfileReq queue[16];
    unsigned int head;
    int sema;
    int queue_sema;
    int unused[2];
} EhfileState; /* 0x41C */

typedef struct LbnEntry {
    const char *name;
    unsigned int lbn;
    unsigned int size;
    unsigned int pad;
} LbnEntry; /* 0x10 */

typedef struct EhfileLbn {
    LbnEntry *table;
    LbnEntry *table_end;
    char *names_end;     /* names grow downwards from the end of the pool */
    int fpl;
    char *buf;           /* fixed pool: LbnEntry table grows up from the start, names down from the end */
    int ready;
} EhfileLbn;

extern EhfileState ehfile_state;
extern EhfileLbn ehfile_lbn;

int ehfile_init(const char ***dirs);
int ehfile_read_file(int dir, const char *name, void *buf, int offset, int size);
int ehfile_make_path(char *out, int dir, const char *name, int raw);
int ehfile_lookup_lbn(const char *name, unsigned int *size_out);
int ehfile_is_disc_path(const char *path);
int ehfile_busy(void);

#endif
