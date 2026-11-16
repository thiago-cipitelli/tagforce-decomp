// @range 0x08804000-0x08804A0C
// @gcc 0x08804000-0x08804394
// @gcc 0x0880442C-0x08804624
// @asm 0x08804624-0x08804714
// @gcc 0x08804714-0x08804A0C
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

/* the sce_newlib_* settings and some libc hooks are weak in the SDK: crt0 tests their address */
#ifdef __GNUC__
#define CRT0_WEAK __attribute__((weak))
#else
#define CRT0_WEAK
#endif

#if defined(__mips__) && !defined(__MWERKS__)
/* GCC track: crt0's string literals are referenced at their place in its .rodata (0x08872B00), which the GCC
   object does not bring into the link; init/fini are the SDK's _init/_fini (names in symbols/BOOT.txt) */
extern char crt0_str_user_main[], crt0_str_C[], crt0_str_exit[], crt0_str_not_main[], crt0_str_user_sbrk[];
#define CRT0_S(name, lit) name
#define libc_init func_08818E64
#define libc_fini func_08818E6C
#else
#define CRT0_S(name, lit) lit
#endif

extern int crt0_heap_kb CRT0_WEAK;     /* 0x088EB9C4 sce_newlib_heap_kb_size */
extern unsigned int sbrk_cur;          /* 0x088F0CC0 */

#ifdef NON_MATCHING /* GCC: loads the weak sce_newlib_* globals through %hi/%lo */
int user_main(SceSize args, void *argp);

extern char crt0_thread_name[] CRT0_WEAK;       /* 0x088EB9C8 "user_main" */
extern int crt0_thread_prio CRT0_WEAK;           /* 0x088EB9D8 */
extern int crt0_stack_kb CRT0_WEAK;              /* 0x088EB9D4 */
extern unsigned int crt0_thread_attr CRT0_WEAK;  /* 0x088EB9DC */
extern void (*crt0_init_hook)(void);   /* 0x08872B44 weak */
extern unsigned int crt0_sbrk_base;    /* 0x088F0CC4 */
extern unsigned int crt0_sbrk_end;     /* 0x088F0CC8 */
extern SceUID crt0_sbrk_block;         /* 0x088F0CCC */

int module_start(SceSize args, void *argp)
{
    const char *name;
    int prio;
    int stack;
    unsigned int attr;
    SceUID th;

    sceKernelSetCompiledSdkVersion(0x02080010);
    SysMemUserForUser_F77D77CB(0x00030306);
    if (&crt0_thread_name != 0) {
        name = crt0_thread_name;
    } else {
        name = CRT0_S(crt0_str_user_main, "user_main");
    }
    prio = 32;
    if (&crt0_thread_prio != 0) {
        prio = crt0_thread_prio;
    }
    stack = 0x40000;
    if (&crt0_stack_kb != 0) {
        stack = crt0_stack_kb << 10;
    }
    if (&crt0_thread_attr != 0) {
        attr = crt0_thread_attr | 0x80000000;
    } else {
        attr = 0x80000000;
    }
    th = sceKernelCreateThread(name, user_main, prio, stack, attr, 0);
    sceKernelStartThread(th, args, argp);
    return 0;
}

void libc_init_reent(void *r) CRT0_WEAK;
int libc_set_module_reent(SceUID mod, void *r);
int atexit(void (*fn)(void));
unsigned int _sbrk(int incr);
void libc_fini(void);
void libc_init(void);
extern int crt0_heap_lazy CRT0_WEAK;   /* a setting the game does not define: _sbrk(0) makes the heap at start */

#if defined(__mips__) && !defined(__MWERKS__)
#include <sys/reent.h>

#include "sdk_reent.h"
#define CRT0_REENT_INIT(var) SDK_REENT_INIT_PTR(var, CRT0_S(crt0_str_C, "C"))
#else
#define CRT0_REENT_INIT(r) libc_init_reent(r)
struct _reent;
#endif

/* argv from the NUL-separated argument block (at most 20 entries), the thread's reent struct,
   then main() and exit(). */
int user_main(SceSize args, void *argp)
{
    char *argv[20];
    char reent[0x380];
    char *r = reent;
    char *p;
    int argc;

    if (&crt0_heap_lazy == 0 || crt0_heap_lazy == 0) {
        _sbrk(0);
    }
    for (argc = 0, p = argp; argc < 20 && p - (char *)argp < (int)args; argc++) {
        argv[argc] = p;
        p += strlen(p) + 1;
    }
    argv[argc] = 0;
    if (libc_init_reent != 0) {
        libc_init_reent(r);
        if (libc_set_module_reent(sceKernelGetModuleIdByAddress(module_start), r) < 0) {
            goto out;
        }
    } else {
        CRT0_REENT_INIT(((struct _reent *)r));
    }
#if defined(__mips__) && !defined(__MWERKS__)
    {
        register void **k0 __asm__("$26");

        k0[1] = r;                     /* the thread-local reent pointer that __getreent reads */
    }
#endif
    atexit(libc_fini);
    libc_init();
    exit(main(argc, argv));
out:
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
SceUID _open(const char *name, int flags, ...)
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
    switch (fd) {
    case 0:
        return sceKernelStdin();
    case 1:
        return sceKernelStdout();
    case 2:
        return sceKernelStderr();
    }
    return fd;
}
#endif

/* newlib _lseek; SEEK_SET/CUR/END are the same values on the PSP. -9 (EBADF) for a bad fd. */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _lseek(int fd, int offset, int whence)
{
    int w;

    fd = libc_fd(fd);
    if (fd < 0) {
        return -9;
    }
    switch (whence) {
    case 0:
        w = 0;
        break;
    case 1:
        w = 1;
        break;
    case 2:
        w = 2;
        break;
    default:
        w = whence;
        break;
    }
    return sceIoLseek(libc_fd(fd), offset, w);
}
#else
int _lseek(int fd, int offset, int whence);
#endif

#ifdef NON_MATCHING /* GCC: loads globals through %hi/%lo */
SceUID libc_get_thread_module(void) CRT0_WEAK;
SceUID heap_get_current(void);
#if !(defined(__mips__) && !defined(__MWERKS__)) /* else newlib's, from <sys/reent.h> */
void _reclaim_reent(SceUID mod);
#endif
SceUID libc_module_of(SceUID mod);

/* newlib _exit: only the main thread may call it; unloads a sub-module or exits the game. */
void _exit(int status)
{
    if (&libc_get_thread_module != 0) {
        SceUID cur = libc_get_thread_module();
        SceUID mod;
        int res;

        if (cur != heap_get_current()) {
            sceKernelPrintf(CRT0_S(crt0_str_not_main, "libc:%s: should be called from main thread\n"), CRT0_S(crt0_str_exit, "_exit"));
            sceKernelExitThread(status);
        }
        _reclaim_reent(cur);
        mod = libc_module_of(cur);
        if (ModuleMgrForUser_F0A26395() != mod) {
            sceKernelStopModule(mod, 0, 0, &res, 0);
            sceKernelUnloadModule(mod);
            sceKernelExitThread(status);
        }
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

        size = 0x10000;
        if (&crt0_heap_kb != 0) {
            size = crt0_heap_kb << 10;
            if (crt0_heap_kb == 0) {
                return 0xFFFFFFFF;
            }
        }
        if (size != 0) {
            SceUID b = sceKernelAllocPartitionMemory(2, CRT0_S(crt0_str_user_sbrk, "UserSbrk"), 3, size, (void *)0x1000);

            crt0_sbrk_block = b;
            if (b > 0) {
                unsigned int h = (unsigned int)sceKernelGetBlockHeadAddr(b);

                crt0_sbrk_end = h + size;
                sbrk_cur = h;
                crt0_sbrk_base = h;
            }
        }
    }
    if (crt0_sbrk_base == 0) {
        return 0xFFFFFFFF;
    }
    old = sbrk_cur;
    next = old + incr;
    if (next < crt0_sbrk_base) {
        return 0xFFFFFFFF;
    }
    if (next > crt0_sbrk_end) {
        return 0xFFFFFFFF;
    }
    sbrk_cur = next;
    return old;
}
#endif

/* newlib _read */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _read(int fd, void *buf, int n)
{
    fd = libc_fd(fd);
    if (fd < 0) {
        return -9;
    }
    return sceIoRead(fd, buf, n);
}
#else
int _read(int fd, void *buf, int n);
#endif

/* newlib _write */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _write(int fd, const void *buf, int n)
{
    fd = libc_fd(fd);
    if (fd < 0) {
        return -9;
    }
    return sceIoWrite(fd, buf, n);
}
#else
int _write(int fd, const void *buf, int n);
#endif

/* newlib _close */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _close(int fd)
{
    fd = libc_fd(fd);
    if (fd < 0) {
        return -9;
    }
    return sceIoClose(fd);
}
#else
int _close(int fd);
#endif

/* newlib _fstat: everything is a character device (S_IFCHR) */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _fstat(int fd, int *st)
{
    st[1] = 0x2000;
    return 0;
}
#else
int _fstat(int fd, int *st);
#endif

/* newlib _isatty */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _isatty(int fd)
{
    return 1;
}
#else
int _isatty(int fd);
#endif

/* newlib _rename */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _rename(const char *oldname, const char *newname)
{
    return sceIoRename(oldname, newname);
}
#else
int _rename(const char *oldname, const char *newname);
#endif

/* newlib system calls without an implementation (_link, _unlink, _kill, _getpid, ...): -1 */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_08804914(void)
{
    return -1;
}
#else
int libc_nosys_08804914(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_0880491C(void)
{
    return -1;
}
#else
int libc_nosys_0880491C(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_08804924(void)
{
    return -1;
}
#else
int libc_nosys_08804924(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_0880492C(void)
{
    return -1;
}
#else
int libc_nosys_0880492C(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_08804934(void)
{
    return -1;
}
#else
int libc_nosys_08804934(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_0880493C(void)
{
    return -1;
}
#else
int libc_nosys_0880493C(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_08804944(void)
{
    return -1;
}
#else
int libc_nosys_08804944(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_0880494C(void)
{
    return -1;
}
#else
int libc_nosys_0880494C(void);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_08804954(void)
{
    return -1;
}
#else
int libc_nosys_08804954(void);
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
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _times(void *buf)
{
    return sceKernelLibcClock();
}
#else
int _times(void *buf);
#endif

/* newlib _time */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _time(int *t)
{
    return sceKernelLibcTime(t);
}
#else
int _time(int *t);
#endif

/* newlib _gettimeofday */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int _gettimeofday(void *tp, void *tzp)
{
    return sceKernelLibcGettimeofday(tp, tzp);
}
#else
int _gettimeofday(void *tp, void *tzp);
#endif

/* newlib _fork / _wait: -1 */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_088049EC(void)
{
    return -1;
}
#else
int libc_nosys_088049EC(void);
#endif

/* newlib _wait-like stub: *status = -1, returns 0 */
#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_088049F4(int *status)
{
    *status = -1;
    return 0;
}
#else
int libc_nosys_088049F4(int *status);
#endif

#ifndef __MWERKS__ /* PSP: crt0_gcc.c */
int libc_nosys_08804A04(void)
{
    return -1;
}
#else
int libc_nosys_08804A04(void);
#endif
