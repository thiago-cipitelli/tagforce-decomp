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

#endif
