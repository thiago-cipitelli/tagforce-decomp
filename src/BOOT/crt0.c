// @range 0x08804000-0x08804A0C
// @asm 0x08804000-0x08804394
// @asm 0x0880455C-0x08804714
// @asm 0x0880495C-0x08804998
/* crt0 and the newlib system-call layer of the Sony SDK libc (GCC). module_start starts the
   "user_main" thread, which splits the argument block into argv, sets up the thread's reentrancy
   struct and calls main() then exit(). The _open/_read/... stubs map newlib calls to sceIo*, with
   fds 0-2 translated to the kernel's stdin/stdout/stderr. All of it is kept as the original
   instructions (`asm` functions, or split asm where it loads globals); the C is the port's
   equivalent and calls the same sce* API, which platform/ provides. */
#include "psp.h"
#include "libc.h"

int sceKernelSetCompiledSdkVersion(unsigned int ver);
int SysMemUserForUser_F77D77CB(unsigned int ver);
SceUID sceKernelGetModuleIdByAddress(void *addr);
int sceKernelStdin(void);
int sceKernelStdout(void);
int sceKernelStderr(void);
int sceIoWrite(SceUID fd, const void *data, SceSize size);
int sceIoRename(const char *oldname, const char *newname);
SceUID sceKernelAllocPartitionMemory(SceUID part, const char *name, int type, SceSize size, void *addr);
void *sceKernelGetBlockHeadAddr(SceUID blockid);
int sceKernelFreePartitionMemory(SceUID blockid);
int sceKernelLibcClock(void);
int sceKernelLibcTime(int *t);
int sceKernelLibcGettimeofday(void *tp, void *tzp);
int sceKernelPrintf(const char *fmt, ...);
int ModuleMgrForUser_F0A26395(void);
int ModuleMgrForUser_8F2DF740(int, int, int, int, int);
int main(int argc, char **argv);
void exit(int status);

extern int crt0_heap_kb;               /* 0x088EB9C4 sce_newlib_heap_kb_size */
extern unsigned int sbrk_cur;          /* 0x088F0CC0 */

#ifdef NON_MATCHING /* GCC: loads the weak sce_newlib_* globals through %hi/%lo */
int user_main(SceSize args, void *argp);

extern char crt0_thread_name[];       /* 0x088EB9C8 "user_main" */
extern int crt0_thread_prio;           /* 0x088EB9D8 */
extern int crt0_stack_kb;              /* 0x088EB9D4 */
extern unsigned int crt0_thread_attr;  /* 0x088EB9DC */
extern void (*crt0_init_hook)(void);   /* 0x08872B44 weak */
extern unsigned int crt0_sbrk_base;    /* 0x088F0CC4 */
extern unsigned int crt0_sbrk_end;     /* 0x088F0CC8 */
extern SceUID crt0_sbrk_block;         /* 0x088F0CCC */

int module_start(SceSize args, void *argp)
{
    SceUID th;

    sceKernelSetCompiledSdkVersion(0x02080010);
    SysMemUserForUser_F77D77CB(0x00030306);
    th = sceKernelCreateThread(crt0_thread_name, user_main, crt0_thread_prio, crt0_stack_kb << 10,
                               crt0_thread_attr | 0x80000000, 0);
    sceKernelStartThread(th, args, argp);
    return 0;
}

void libc_init_reent(void *r);
int libc_set_module_reent(SceUID mod, void *r);
int atexit(void (*fn)(void));
unsigned int _sbrk(int incr);
void libc_fini(void);
void libc_init(void);

/* argv from the NUL-separated argument block (at most 20 entries), the thread's reent struct,
   then main() and exit(). */
int user_main(SceSize args, void *argp)
{
    char *argv[20];
    char reent[0x37C];
    char *p = argp;
    int argc = 0;

    _sbrk(0);
    if ((int)args > 0) {
        do {
            argv[argc++] = p;
            p += strlen(p) + 1;
        } while (argc < 20 && p - (char *)argp < (int)args);
    }
    argv[argc] = 0;
    libc_init_reent(reent);
    if (libc_set_module_reent(sceKernelGetModuleIdByAddress(module_start), reent) >= 0) {
        atexit(libc_fini);
        libc_init();
        exit(main(argc, argv));
    }
    sceKernelExitThread(1);
    return 1;
}

/* calls the weak init hook when it is linked in */
int crt0_run_init(void)
{
    if (&crt0_init_hook != 0) {
        crt0_init_hook();
    }
    return 0;
}
#endif

/* newlib _open: O_RDONLY/O_WRONLY/O_RDWR -> PSP_O_RDONLY(1)/WRONLY(2)/RDWR(3), O_APPEND(8) ->
   0x100, O_CREAT/O_TRUNC/O_EXCL (0x200/0x400/0x800) as is, O_NONBLOCK(0x4000) -> 4. */
#ifdef __MWERKS__
asm SceUID _open(const char *name, int flags)
{
    addiu sp, sp, -48
    move t5, a1
    andi v1, a1, 0x3
    addiu a1, zero, 1
    sw ra, 0(sp)
    move t4, zero
    sw a2, 24(sp)
    sw a3, 28(sp)
    sw t0, 32(sp)
    sw t1, 36(sp)
    sw t2, 40(sp)
    beq v1, a1, @90
    sw t3, 44(sp)
    beqzl v1, @48
    addiu t4, zero, 1
    addiu a1, zero, 2
    beql v1, a1, @48
    addiu t4, zero, 3
@48:
    andi t8, t5, 0x8
    ori t7, t4, 0x100
    andi t6, t5, 0x200
    movn t4, t7, t8
    andi t3, t5, 0x400
    or t0, t6, t4
    andi t1, t5, 0x800
    or v0, t3, t0
    or v1, t1, v0
    andi t2, t5, 0x4000
    ori a3, v1, 0x4
    movn v1, a3, t2
    addiu a2, zero, 420
    jal sceIoOpen
    move a1, v1
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 48
@90:
    j @48
    addiu t4, zero, 2
}
#else
SceUID _open(const char *name, int flags)
{
    int f = 0;

    switch (flags & 3) {
    case 0:
        f = 1;
        break;
    case 1:
        f = 2;
        break;
    case 2:
        f = 3;
        break;
    }
    if (flags & 8) {
        f |= 0x100;
    }
    f |= flags & 0x800;
    f |= flags & 0x400;
    f |= flags & 0x200;
    if (flags & 0x4000) {
        f |= 4;
    }
    return sceIoOpen(name, f, 0x1A4);
}
#endif

#ifndef __MWERKS__
static int libc_fd(int fd)
{
    if (fd == 0) {
        return sceKernelStdin();
    }
    if (fd == 1) {
        return sceKernelStdout();
    }
    if (fd == 2) {
        return sceKernelStderr();
    }
    return fd;
}
#endif

/* newlib _lseek; SEEK_SET/CUR/END are the same values on the PSP. -9 (EBADF) for a bad fd. */
#ifdef __MWERKS__
asm int _lseek(int fd, int offset, int whence)
{
    addiu sp, sp, -16
    addiu v1, zero, 1
    sw s2, 8(sp)
    move s2, a1
    sw s0, 0(sp)
    move s0, a2
    sw ra, 12(sp)
    beq a0, v1, @120
    sw s1, 4(sp)
    slti v1, a0, 2
    bnez v1, @108
    addiu a1, zero, 2
    beq a0, a1, @f8
    nop
@38:
    bltz a0, @98
    addiu v0, zero, -9
    addiu a2, zero, 1
    beq s0, a2, @64
    addiu s1, zero, 1
    slti a3, s0, 2
    bnez a3, @e8
    addiu t0, zero, 2
    beq s0, t0, @64
    addiu s1, zero, 2
    move s1, s0
@64:
    addiu t1, zero, 1
    beq a0, t1, @d8
    slti t2, a0, 2
    bnez t2, @c0
    addiu t3, zero, 2
    beq a0, t3, @b0
    nop
    move v0, a0
@84:
    move a2, s2
@88:
    sra a3, s2, 0x1f
    move a0, v0
    jal sceIoLseek
    move t0, s1
@98:
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
@b0:
    jal sceKernelStderr
    nop
    j @88
    move a2, s2
@c0:
    bnezl a0, @84
    move v0, a0
    jal sceKernelStdin
    nop
    j @88
    move a2, s2
@d8:
    jal sceKernelStdout
    nop
    j @88
    move a2, s2
@e8:
    beqz s0, @64
    move s1, zero
    j @64
    move s1, s0
@f8:
    jal sceKernelStderr
    nop
    j @38
    move a0, v0
@108:
    bnez a0, @38
    nop
    jal sceKernelStdin
    nop
    j @38
    move a0, v0
@120:
    jal sceKernelStdout
    nop
    j @38
    move a0, v0
}
#else
int _lseek(int fd, int offset, int whence)
{
    int kfd = libc_fd(fd);

    if (kfd < 0) {
        return -9;
    }
    return sceIoLseek(libc_fd(fd), (SceOff)offset, whence);
}
#endif

#ifdef NON_MATCHING /* GCC: loads globals through %hi/%lo */
SceUID libc_get_thread_module(void);
SceUID heap_get_current(void);
void libc_run_atexit(SceUID mod);
SceUID libc_module_of(SceUID mod);

/* newlib _exit: only the main thread may call it; unloads a sub-module or exits the game. */
void _exit(int status)
{
    SceUID cur = libc_get_thread_module();
    SceUID mod;
    int res;

    if (cur != heap_get_current()) {
        sceKernelPrintf("libc:%s: should be called from main thread\n", "_exit");
        sceKernelExitThread(status);
    }
    libc_run_atexit(cur);
    mod = libc_module_of(cur);
    if (ModuleMgrForUser_F0A26395() != mod) {
        sceKernelStopModule(mod, 0, 0, &res, 0);
        sceKernelUnloadModule(mod);
        sceKernelExitThread(status);
    }
    ModuleMgrForUser_8F2DF740(1, 0, 0, 0, 0);
}

/* newlib _sbrk over a "UserSbrk" partition block of crt0_heap_kb KB allocated on first use. */
unsigned int _sbrk(int incr)
{
    unsigned int old;
    unsigned int next;

    if (crt0_sbrk_base == 0) {
        int size;

        if (crt0_heap_kb == 0) {
            return 0xFFFFFFFF;
        }
        size = crt0_heap_kb * 1024;
        if (size != 0) {
            crt0_sbrk_block = sceKernelAllocPartitionMemory(2, "UserSbrk", 3, size, (void *)0x1000);
            if (crt0_sbrk_block > 0) {
                sbrk_cur = (unsigned int)sceKernelGetBlockHeadAddr(crt0_sbrk_block);
                crt0_sbrk_end = sbrk_cur + size;
                crt0_sbrk_base = sbrk_cur;
            }
        }
    }
    if (crt0_sbrk_base == 0) {
        return 0xFFFFFFFF;
    }
    old = sbrk_cur;
    next = sbrk_cur + incr;
    if (next < crt0_sbrk_base || next > crt0_sbrk_end) {
        return 0xFFFFFFFF;
    }
    sbrk_cur = next;
    return old;
}
#endif

/* newlib _read */
#ifdef __MWERKS__
asm int _read(int fd, void *buf, int n)
{
    addiu sp, sp, -16
    addiu v1, zero, 1
    sw s1, 4(sp)
    move s1, a2
    sw s0, 0(sp)
    move s0, a1
    beq a0, v1, @94
    sw ra, 8(sp)
    slti v1, a0, 2
    bnez v1, @7c
    addiu a1, zero, 2
    beq a0, a1, @6c
    nop
    move v0, a0
@38:
    bltz v0, @54
    addiu v1, zero, -9
    move a0, v0
    move a1, s0
    jal sceIoRead
    move a2, s1
    move v1, v0
@54:
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 16
@6c:
    jal sceKernelStderr
    nop
    j @38
    nop
@7c:
    bnezl a0, @38
    move v0, a0
    jal sceKernelStdin
    nop
    j @38
    nop
@94:
    jal sceKernelStdout
    nop
    j @38
    nop
}
#else
int _read(int fd, void *buf, int n)
{
    fd = libc_fd(fd);
    if (fd < 0) {
        return -9;
    }
    return sceIoRead(fd, buf, n);
}
#endif

/* newlib _write */
#ifdef __MWERKS__
asm int _write(int fd, const void *buf, int n)
{
    addiu sp, sp, -16
    addiu v1, zero, 1
    sw s1, 4(sp)
    move s1, a2
    sw s0, 0(sp)
    move s0, a1
    beq a0, v1, @94
    sw ra, 8(sp)
    slti v1, a0, 2
    bnez v1, @7c
    addiu a1, zero, 2
    beq a0, a1, @6c
    nop
    move v0, a0
@38:
    bltz v0, @54
    addiu v1, zero, -9
    move a0, v0
    move a1, s0
    jal sceIoWrite
    move a2, s1
    move v1, v0
@54:
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 16
@6c:
    jal sceKernelStderr
    nop
    j @38
    nop
@7c:
    bnezl a0, @38
    move v0, a0
    jal sceKernelStdin
    nop
    j @38
    nop
@94:
    jal sceKernelStdout
    nop
    j @38
    nop
}
#else
int _write(int fd, const void *buf, int n)
{
    fd = libc_fd(fd);
    if (fd < 0) {
        return -9;
    }
    return sceIoWrite(fd, buf, n);
}
#endif

/* newlib _close */
#ifdef __MWERKS__
asm int _close(int fd)
{
    addiu sp, sp, -16
    addiu v1, zero, 1
    beq a0, v1, @74
    sw ra, 0(sp)
    slti v1, a0, 2
    bnez v1, @5c
    addiu a1, zero, 2
    beq a0, a1, @4c
    nop
    move v0, a0
@28:
    bltz v0, @3c
    addiu v1, zero, -9
    jal sceIoClose
    move a0, v0
    move v1, v0
@3c:
    lw ra, 0(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 16
@4c:
    jal sceKernelStderr
    nop
    j @28
    nop
@5c:
    bnezl a0, @28
    move v0, a0
    jal sceKernelStdin
    nop
    j @28
    nop
@74:
    jal sceKernelStdout
    nop
    j @28
    nop
}
#else
int _close(int fd)
{
    fd = libc_fd(fd);
    if (fd < 0) {
        return -9;
    }
    return sceIoClose(fd);
}
#endif

/* newlib _fstat: everything is a character device (S_IFCHR) */
#ifdef __MWERKS__
asm int _fstat(int fd, int *st)
{
    addiu v1, zero, 8192
    move v0, zero
    jr ra
    sw v1, 4(a1)
}
#else
int _fstat(int fd, int *st)
{
    st[1] = 0x2000;
    return 0;
}
#endif

/* newlib _isatty */
#ifdef __MWERKS__
asm int _isatty(int fd)
{
    jr ra
    addiu v0, zero, 1
}
#else
int _isatty(int fd)
{
    return 1;
}
#endif

/* newlib _rename */
#ifdef __MWERKS__
asm int _rename(const char *oldname, const char *newname)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal sceIoRename
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int _rename(const char *oldname, const char *newname)
{
    return sceIoRename(oldname, newname);
}
#endif

/* newlib system calls without an implementation (_link, _unlink, _kill, _getpid, ...): -1 */
#ifdef __MWERKS__
asm int libc_nosys_08804914(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_08804914(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_0880491C(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_0880491C(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_08804924(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_08804924(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_0880492C(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_0880492C(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_08804934(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_08804934(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_0880493C(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_0880493C(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_08804944(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_08804944(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_0880494C(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_0880494C(void)
{
    return -1;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_08804954(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_08804954(void)
{
    return -1;
}
#endif

#ifdef NON_MATCHING /* GCC: loads globals through %hi/%lo */
/* frees the UserSbrk block */
void _sbrk_free(void)
{
    if (crt0_sbrk_block != 0) {
        sceKernelFreePartitionMemory(crt0_sbrk_block);
        crt0_sbrk_block = 0;
    }
}
#endif

/* newlib _times -> clock() */
#ifdef __MWERKS__
asm int _times(void *buf)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal sceKernelLibcClock
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int _times(void *buf)
{
    return sceKernelLibcClock();
}
#endif

/* newlib _time */
#ifdef __MWERKS__
asm int _time(int *t)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal sceKernelLibcTime
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int _time(int *t)
{
    return sceKernelLibcTime(t);
}
#endif

/* newlib _gettimeofday */
#ifdef __MWERKS__
asm int _gettimeofday(void *tp, void *tzp)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal sceKernelLibcGettimeofday
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int _gettimeofday(void *tp, void *tzp)
{
    return sceKernelLibcGettimeofday(tp, tzp);
}
#endif

/* newlib _fork / _wait: -1 */
#ifdef __MWERKS__
asm int libc_nosys_088049EC(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_088049EC(void)
{
    return -1;
}
#endif

/* newlib _wait-like stub: *status = -1, returns 0 */
#ifdef __MWERKS__
asm int libc_nosys_088049F4(int *status)
{
    addiu v1, zero, -1
    move v0, zero
    jr ra
    sw v1, 0(a0)
}
#else
int libc_nosys_088049F4(int *status)
{
    *status = -1;
    return 0;
}
#endif

#ifdef __MWERKS__
asm int libc_nosys_08804A04(void)
{
    jr ra
    addiu v0, zero, -1
}
#else
int libc_nosys_08804A04(void)
{
    return -1;
}
#endif
