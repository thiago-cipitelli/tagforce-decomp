/* IoFileMgrForUser and sceUmd on POSIX files (port step 4b, docs/port_survey.md).

   disc0:/ and umd0:/ map to the root of the extracted ISO (env TAGFORCE_ROOT, default the current directory),
   ms0:/ to the save directory (env TAGFORCE_SAVE, default $HOME/.local/share/tagforce/ms0). A path without a device
   is relative to disc0:/PSP_GAME/SYSDIR, the directory of the EBOOT.
   umd1: is the raw disc, read in 2048-byte sectors: ehfile_build_lbn_table reads the volume descriptor and the
   root directory from it to turn names into sector numbers. The host serves an empty volume (descriptor, path
   table and a root directory without entries), so every lookup misses and the game keeps opening files by name.
   The Async calls run synchronously and keep the result for sceIoWaitAsync/sceIoPollAsync. */
#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int dprintf(int fd, const char *fmt, ...);

/* out = a b c, truncated to n; not snprintf: the game's newlib one interposes glibc's */
static void cat3(char *out, int n, const char *a, const char *b, const char *c)
{
    const char *s[3] = { a, b, c };
    int i, k = 0;

    for (i = 0; i < 3; i++)
        for (; *s[i] && k < n - 1; s[i]++)
            out[k++] = *s[i];
    out[k] = 0;
}

typedef struct {   /* ScePspDateTime */
    unsigned short year, month, day, hour, minute, second;
    unsigned int microsecond;
} DateTime;

typedef struct {   /* SceIoStat, 0x58 */
    int st_mode;
    unsigned int st_attr;
    long long st_size;
    DateTime ctime_, atime_, mtime_;
    unsigned int st_private[6];
} IoStat;

typedef struct {   /* SceIoDirent, 0x160 */
    IoStat d_stat;
    char d_name[256];
    void *d_private;
    int dummy;
} IoDirent;

#define ERR_NOENT 0x80010002
#define ERR_BADF 0x80010009
#define ERR_INVAL 0x80010016
#define SECTOR 2048
#define UMD_FD 0x7FF0   /* umd1: is not a host fd */

typedef struct {
    int kind;   /* 0 free, 1 file, 2 dir, 3 umd1: */
    int fd;
    DIR *dir;
    long long sector;
    long long async;   /* result of the last Async call */
} File;

static File files[256];
static int trace = -1;

static const char *root(void)
{
    const char *r = getenv("TAGFORCE_ROOT");

    return r && *r ? r : ".";
}

void host_save_dir(char *out, int n)
{
    const char *s = getenv("TAGFORCE_SAVE");
    const char *home = getenv("HOME");

    if (s && *s)
        cat3(out, n, s, "", "");
    else
        cat3(out, n, home ? home : ".", "/.local/share/tagforce/ms0", "");
}

/* PSP path -> host path; returns 0 for a device the host does not have */
static int host_path(const char *psp, char *out, int n)
{
    const char *colon = strchr(psp, ':');
    char base[512];

    if (!colon) {
        cat3(base, sizeof base, root(), "/PSP_GAME/SYSDIR/", psp);
        cat3(out, n, base, "", "");
    } else if (!strncmp(psp, "disc0:", 6) || !strncmp(psp, "umd0:", 5)) {
        cat3(out, n, root(), colon[1] == '/' ? "" : "/", colon + 1);
    } else if (!strncmp(psp, "ms0:", 4) || !strncmp(psp, "fatms0:", 7)) {
        host_save_dir(base, sizeof base);
        cat3(out, n, base, colon[1] == '/' ? "" : "/", colon + 1);
    } else {
        return 0;
    }
    if (trace < 0)
        trace = getenv("TAGFORCE_TRACE_IO") != 0;
    if (trace)
        dprintf(2, "io: %s -> %s\n", psp, out);
    return 1;
}

/* mkdir -p of the directories above path (saves) */
void host_make_parents(char *path)
{
    char *p;

    for (p = path + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(path, 0755);
            *p = '/';
        }
    }
}

static int new_file(int kind)
{
    int i;

    for (i = 3; i < 256; i++) {
        if (!files[i].kind) {
            memset(&files[i], 0, sizeof files[i]);
            files[i].kind = kind;
            return i;
        }
    }
    return -1;
}

static File *get(int fd)
{
    return fd > 0 && fd < 256 && files[fd].kind ? &files[fd] : 0;
}

static int err(void)
{
    return errno == ENOENT || errno == ENOTDIR ? (int)ERR_NOENT : (int)(0x80010000 | errno);
}

int sceIoOpen(const char *file, int flags, int mode)
{
    char path[1024];
    int f = 0, h, fd;

    if (!strncmp(file, "umd1:", 5)) {
        fd = new_file(3);
        dprintf(2, "io: open %s (empty volume)\n", file);
        return fd;
    }
    if (!host_path(file, path, sizeof path)) {
        dprintf(2, "io: open %s: no such device\n", file);
        return ERR_NOENT;
    }
    switch (flags & 3) {
    case 1: f = O_RDONLY; break;
    case 2: f = O_WRONLY; break;
    default: f = O_RDWR; break;
    }
    if (flags & 0x100) f |= O_APPEND;
    if (flags & 0x200) f |= O_CREAT;
    if (flags & 0x400) f |= O_TRUNC;
    if (flags & 0x800) f |= O_EXCL;
    if (f & O_CREAT)
        host_make_parents(path);
    h = open(path, f, 0644);
    if (h < 0) {
        dprintf(2, "io: open %s: %s\n", file, strerror(errno));
        return err();
    }
    fd = new_file(1);
    files[fd].fd = h;
    dprintf(2, "io: open %s = %d\n", file, fd);
    return fd;
}

int sceIoClose(int fd)
{
    File *f = get(fd);

    if (!f)
        return ERR_BADF;
    if (f->kind == 1)
        close(f->fd);
    f->kind = 0;
    return 0;
}

/* the empty volume: PVD at sector 16 (path table of one sector at 18), the root directory at 19 */
static void umd_sector(long long s, unsigned char *b)
{
    memset(b, 0, SECTOR);
    if (s == 16) {
        b[0] = 1;
        memcpy(b + 1, "CD001", 5);
        b[0x84] = SECTOR & 0xFF;
        b[0x85] = SECTOR >> 8;
        b[0x8C] = 18;
    } else if (s == 18) {
        b[0] = 1;   /* root record: extent at +2 */
        b[2] = 19;
        b[6] = 1;
    }
}

int sceIoRead(int fd, void *data, unsigned int size)
{
    File *f = get(fd);
    int r;

    if (!f)
        return ERR_BADF;
    if (f->kind == 3) {   /* size in sectors */
        unsigned int i;

        for (i = 0; i < size; i++)
            umd_sector(f->sector++, (unsigned char *)data + i * SECTOR);
        return size;
    }
    r = read(f->fd, data, size);
    return r < 0 ? err() : r;
}

int sceIoWrite(int fd, const void *data, unsigned int size)
{
    File *f;
    int r;

    if (fd == 1 || fd == 2)
        return write(fd, data, size);
    f = get(fd);
    if (!f || f->kind != 1)
        return ERR_BADF;
    r = write(f->fd, data, size);
    return r < 0 ? err() : r;
}

long long sceIoLseek(int fd, long long offset, int whence)
{
    File *f = get(fd);
    off_t r;

    if (!f)
        return (int)ERR_BADF;
    if (f->kind == 3) {
        f->sector = whence == 0 ? offset : whence == 1 ? f->sector + offset : offset;
        return f->sector;
    }
    r = lseek(f->fd, offset, whence);
    return r < 0 ? err() : r;
}

int sceIoLseek32(int fd, int offset, int whence) { return (int)sceIoLseek(fd, offset, whence); }

static void to_stat(const struct stat *s, IoStat *o)
{
    struct tm tm;

    memset(o, 0, sizeof *o);
    o->st_mode = S_ISDIR(s->st_mode) ? 0x11FF : 0x21FF;   /* FIO_S_IFDIR / FIO_S_IFREG, rwx */
    o->st_attr = S_ISDIR(s->st_mode) ? 0x10 : 0x20;
    o->st_size = s->st_size;
    localtime_r(&s->st_mtime, &tm);
    o->mtime_.year = tm.tm_year + 1900;
    o->mtime_.month = tm.tm_mon + 1;
    o->mtime_.day = tm.tm_mday;
    o->mtime_.hour = tm.tm_hour;
    o->mtime_.minute = tm.tm_min;
    o->mtime_.second = tm.tm_sec;
    o->ctime_ = o->atime_ = o->mtime_;
}

int sceIoGetstat(const char *file, IoStat *st)
{
    char path[1024];
    struct stat s;

    if (!host_path(file, path, sizeof path) || stat(path, &s) < 0)
        return ERR_NOENT;
    to_stat(&s, st);
    return 0;
}

int sceIoDopen(const char *dirname)
{
    char path[1024];
    DIR *d;
    int fd;

    if (!host_path(dirname, path, sizeof path) || !(d = opendir(path)))
        return ERR_NOENT;
    fd = new_file(2);
    files[fd].dir = d;
    return fd;
}

int sceIoDread(int fd, IoDirent *dir)
{
    File *f = get(fd);
    struct dirent *e;

    if (!f || f->kind != 2)
        return ERR_BADF;
    e = readdir(f->dir);
    if (!e)
        return 0;
    memset(dir, 0, sizeof *dir);
    cat3(dir->d_name, sizeof dir->d_name, e->d_name, "", "");
    {
        struct stat s;

        if (fstatat(dirfd(f->dir), e->d_name, &s, 0) == 0)
            to_stat(&s, &dir->d_stat);
    }
    return 1;
}

int sceIoDclose(int fd)
{
    File *f = get(fd);

    if (!f || f->kind != 2)
        return ERR_BADF;
    closedir(f->dir);
    f->kind = 0;
    return 0;
}

int sceIoMkdir(const char *dir, int mode)
{
    char path[1024];

    if (!host_path(dir, path, sizeof path))
        return ERR_NOENT;
    host_make_parents(path);
    return mkdir(path, 0755) < 0 && errno != EEXIST ? err() : 0;
}

int sceIoRemove(const char *file)
{
    char path[1024];

    if (!host_path(file, path, sizeof path))
        return ERR_NOENT;
    return unlink(path) < 0 ? err() : 0;
}

int sceIoChdir(const char *dir) { return 0; }

/* --- Async: done at once, the result waits in the File --- */

int sceIoOpenAsync(const char *file, int flags, int mode)
{
    int r = sceIoOpen(file, flags, mode);

    if (r >= 0)
        files[r].async = r;
    return r;
}

static int async(int fd, long long r)
{
    File *f = get(fd);

    if (!f)
        return ERR_BADF;
    f->async = r;
    return 0;
}

int sceIoReadAsync(int fd, void *data, unsigned int size) { return async(fd, sceIoRead(fd, data, size)); }
int sceIoWriteAsync(int fd, const void *data, unsigned int size) { return async(fd, sceIoWrite(fd, data, size)); }
int sceIoLseekAsync(int fd, long long offset, int whence) { return async(fd, sceIoLseek(fd, offset, whence)); }
int sceIoLseek32Async(int fd, int offset, int whence) { return async(fd, sceIoLseek(fd, offset, whence)); }

int sceIoCloseAsync(int fd)
{
    File *f = get(fd);

    if (!f)
        return ERR_BADF;
    if (f->kind == 1)
        close(f->fd);
    f->kind = 4;   /* closed, result pending: freed by the wait */
    f->async = 0;
    return 0;
}

int sceIoWaitAsync(int fd, long long *res)
{
    File *f = get(fd);

    if (!f)
        return ERR_BADF;
    if (res)
        *res = f->async;
    if (f->kind == 4)
        f->kind = 0;
    return 0;
}

int sceIoWaitAsyncCB(int fd, long long *res) { return sceIoWaitAsync(fd, res); }
int sceIoPollAsync(int fd, long long *res) { return sceIoWaitAsync(fd, res); }
int sceIoChangeAsyncPriority(int fd, int prio) { return 0; }
int sceIoSetAsyncCallback(int fd, int cb, void *arg) { return 0; }

/* --- UMD: always inserted and ready --- */

int sceUmdActivate(int unit, const char *drive) { return 0; }
int sceUmdGetDriveStat(void) { return 0x32; }   /* PRESENT | INITED | READY */
int sceUmdWaitDriveStat(int stat) { return 0; }
int sceUmdWaitDriveStatCB(int stat, unsigned int timeout) { return 0; }
int sceUmdCancelWaitDriveStat(void) { return 0; }
