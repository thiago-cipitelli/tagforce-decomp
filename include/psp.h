/* Prototypes of the Sony kernel imports used by the reconstructed code (pspsdk names).
   Only what the C files need; grows on demand. sceIoLseek returns int here because the
   original code tests only $v0 after it. */
#ifndef PSP_H
#define PSP_H

typedef unsigned int SceSize;
typedef unsigned int SceUInt;
typedef int SceUID;
typedef long long SceOff;
typedef int (*SceKernelThreadEntry)(SceSize args, void *argp);

typedef struct ScePspDateTime {
    unsigned short year, month, day, hour, minute, second;
    unsigned int microsecond;
} ScePspDateTime;

typedef struct SceIoStat {
    int st_mode;
    unsigned int st_attr;
    SceOff st_size;
    ScePspDateTime st_ctime, st_atime, st_mtime;
    unsigned int st_private[6];
} SceIoStat; /* 0x58 */

typedef struct SceIoDirent {
    SceIoStat d_stat;
    char d_name[256];
    void *d_private;
    int dummy;
} SceIoDirent; /* 0x160 */

int sceIoDopen(const char *dirname);
int sceIoDread(int fd, SceIoDirent *dir);
int sceIoDclose(int fd);

SceUID sceKernelCreateThread(const char *name, SceKernelThreadEntry entry, int priority, int stack, SceUInt attr, void *opt);
int sceKernelStartThread(SceUID thid, SceSize args, void *argp);
SceUID sceKernelCreateSema(const char *name, SceUInt attr, int init, int max, void *opt);
int sceKernelDeleteSema(SceUID sema);
int sceKernelWaitSema(SceUID sema, int signal, SceUInt *timeout);
int sceKernelSignalSema(SceUID sema, int signal);
SceUID sceKernelCreateFpl(const char *name, int part, int attr, unsigned int size, unsigned int blocks, void *opt);
int sceKernelAllocateFpl(SceUID fpl, void **data, unsigned int *timeout);
SceUID sceKernelLoadModule(const char *path, int flags, void *opt);
int sceKernelStartModule(SceUID modid, SceSize argsize, void *argp, int *status, void *opt);
int sceKernelStopModule(SceUID modid, SceSize argsize, void *argp, int *status, void *opt);
int sceKernelUnloadModule(SceUID modid);
int sceKernelWakeupThread(SceUID thid);
int sceKernelSleepThread(void);
int sceKernelSuspendThread(SceUID thid);
int sceKernelResumeThread(SceUID thid);
SceUID sceKernelGetThreadId(void);
int sceKernelGetThreadCurrentPriority(void);

int sceUtilityLoadNetModule(int module);
int sceUtilityUnloadNetModule(int module);
int sceUtilityLoadAvModule(int module);
int sceUtilityUnloadAvModule(int module);
int sceUtilityLoadUsbModule(int module);
int sceUtilityUnloadUsbModule(int module);

SceUID sceIoOpen(const char *file, int flags, int mode);
int sceIoClose(SceUID fd);
int sceIoRead(SceUID fd, void *data, SceSize size);
int sceIoLseek(SceUID fd, SceOff offset, int whence);
int sceIoGetstat(const char *file, SceIoStat *stat);

int sceUmdGetDriveStat(void);
int sceUmdWaitDriveStat(int stat);
int sceUmdActivate(int unit, const char *drive);

typedef int (*SceKernelCallbackFunction)(int arg1, int arg2, void *arg);
SceUID sceKernelCreateCallback(const char *name, SceKernelCallbackFunction func, void *arg);
int sceKernelRegisterExitCallback(SceUID cb);
int sceKernelExitGame(void);
int sceKernelLoadExec(const char *file, void *param);
int sceKernelChangeCurrentThreadAttr(SceUInt clear, SceUInt set);
int sceRtcGetCurrentTick(unsigned long long *tick);

/* libgu (statically linked) */
void sceGuStart(int ctx, void *list, int size);
int sceGuFinish(void);


/* sceMpeg / UMD / async IO used by pmf_umd.c */
int sceMpegRingbufferAvailableSize(void *rb);
int sceMpegRingbufferPut(void *rb, int packets, int available);
int sceKernelDelayThread(SceUInt us);
int sceKernelExitThread(int status);
int sceKernelWaitSemaCB(SceUID sema, int signal, SceUInt *timeout);
int sceUmdCancelWaitDriveStat(void);
int sceUmdWaitDriveStatCB(int stat, SceUInt timeout);
int sceIoReadAsync(SceUID fd, void *data, SceSize size);
int sceIoWaitAsyncCB(SceUID fd, long long *res);
int sceKernelReferEventFlagStatus(SceUID evf, void *status);


/* movie.c (PSMF player) */
int sceMpegInit(void);
int sceMpegFinish(void);
int sceMpegRingbufferQueryMemSize(int packets);
int sceMpegRingbufferConstruct(void *rb, int packets, void *data, int size, int (*cb)(void *, int, void *), void *arg);
int sceMpegRingbufferDestruct(void *rb);
int sceMpegQueryMemSize(int mode);
int sceMpegCreate(void *mpeg, void *data, int size, void *rb, int frame_width, int mode, int ddrtop);
int sceMpegDelete(void *mpeg);
void *sceMpegRegistStream(void *mpeg, int type, int num);
int sceMpegUnRegistStream(void *mpeg, void *stream);
void *sceMpegMallocAvcEsBuf(void *mpeg);
int sceMpegFreeAvcEsBuf(void *mpeg, void *es);
int sceMpegQueryStreamOffset(void *mpeg, void *buf, int *offset);
int sceMpegQueryStreamSize(void *buf, int *size);
int sceMpegInitAu(void *mpeg, void *es, void *au);
int sceMpegQueryAtracEsSize(void *mpeg, int *es_size, int *out_size);
int sceMpegGetAvcAu(void *mpeg, void *stream, void *au, int *attr);
int sceMpegGetAtracAu(void *mpeg, void *stream, void *au, int *attr);
int sceMpegAvcDecode(void *mpeg, void *au, int frame_width, void **buffer, int *got);
int sceMpegAvcDecodeStop(void *mpeg, int frame_width, void **buffer, int *got);
int sceMpegAtracDecode(void *mpeg, void *au, void *buffer, int init);
int sceMpegFlushAllStream(void *mpeg);
int sceMpeg_4571CC64(void *mpeg); /* sceMpegAvcDecodeFlush */
int sceDisplaySetMode(int mode, int width, int height);
int sceDisplaySetFrameBuf(void *top, int buffer_width, int format, int sync);
float sceDisplayGetFramePerSec(void);
SceUID sceKernelCreateEventFlag(const char *name, int attr, int bits, void *opt);
int sceKernelDeleteEventFlag(SceUID evf);
int sceKernelSetEventFlag(SceUID evf, unsigned int bits);
int sceKernelClearEventFlag(SceUID evf, unsigned int bits);
int sceKernelWaitEventFlag(SceUID evf, unsigned int bits, unsigned int wait, unsigned int *out, SceUInt *timeout);
int sceCtrlReadBufferPositive(void *pad, int count);
int sceCtrlSetSamplingMode(int mode);
int sceDmacMemcpy(void *dst, const void *src, unsigned int size);
SceUID sceIoOpenAsync(const char *file, int flags, int mode);
int sceKernelPowerTick(int type);
int sceKernelDelayThreadCB(SceUInt us);
int sceKernelCheckCallback(void);
int sceKernelChangeThreadPriority(SceUID thid, int prio);
void *sceGeEdramGetAddr(void);
int sceKernelWaitThreadEnd(SceUID thid, SceUInt *timeout);
int sceKernelWaitThreadEndCB(SceUID thid, SceUInt *timeout);
int sceKernelDeleteThread(SceUID thid);

#endif
