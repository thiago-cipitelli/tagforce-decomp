/* sceUtility dialogs on files, without any UI. Savedata keeps the bytes the game passes (no encryption, no
   PARAM.SFO) in TAGFORCE_SAVE/PSP/SAVEDATA/<gameName><saveName>/<fileName>; the message dialog logs its text and
   answers YES/OK. Status machine of every dialog: InitStart does the work and leaves the result in the common
   header (+0x1C); GetStatus then returns INIT(1), VISIBLE(2), QUIT(3) until the game calls ShutdownStart, which
   gives FINISHED(4) once and NONE(0) after (BOOT/msd.c waits on 1/2/4, shuts down on 3, reads the result on 0).
   Update is a no-op. Only the savedata modes BOOT/savedata.c uses are real: AUTOLOAD(0)/LOAD(2), AUTOSAVE(1)/SAVE(3),
   DELETE(7), SIZES(8), GETSIZE(22). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "savedata.h"

void host_save_dir(char *out, int n);
void host_make_parents(char *path);

enum { NONE, INIT, VISIBLE, QUIT, FINISHED };

#define ERR_LOAD_NO_DATA 0x80110307
#define ERR_RW_NO_DATA 0x80110327
#define ERR_RW_ACCESS 0x80110325
#define ERR_DELETE_NO_DATA 0x80110347
#define ERR_SAVE_ACCESS 0x80110385

static int sd_status, md_status;

static void sd_path(const SaveParam *p, const char *file, char *out, int n)
{
    char dir[512];

    host_save_dir(dir, sizeof dir);
    snprintf(out, n, "%s/PSP/SAVEDATA/%.13s%.20s/%s", dir, p->game_name, p->save_name, file ? file : "");
}

static long file_size(const char *path)
{
    struct stat st;

    return stat(path, &st) == 0 ? (long)st.st_size : -1;
}

static int sd_load(SaveParam *p)
{
    char path[640];
    FILE *f;
    long n;

    sd_path(p, p->file_name, path, sizeof path);
    if (!(f = fopen(path, "rb")))
        return ERR_LOAD_NO_DATA;
    n = fread(p->data_buf, 1, p->data_buf_size, f);
    fclose(f);
    p->data_size = (int)n;
    fprintf(stderr, "savedata: load %s (%ld bytes)\n", path, n);
    return 0;
}

static int sd_save(SaveParam *p)
{
    char path[640];
    FILE *f;

    sd_path(p, p->file_name, path, sizeof path);
    host_make_parents(path);
    if (!(f = fopen(path, "wb")))
        return ERR_SAVE_ACCESS;
    fwrite(p->data_buf, 1, p->data_size, f);
    fclose(f);
    fprintf(stderr, "savedata: save %s (%d bytes)\n", path, p->data_size);
    return 0;
}

static int sd_delete(SaveParam *p)
{
    char path[640];

    sd_path(p, p->file_name, path, sizeof path);
    if (file_size(path) < 0)
        return ERR_DELETE_NO_DATA;
    unlink(path);
    sd_path(p, 0, path, sizeof path);
    rmdir(path);
    fprintf(stderr, "savedata: delete %s\n", path);
    return 0;
}

/* SIZES: free space (msFree), the save's own use (msData, when it exists) and what this save would take
   (utilityData). The game copies 8 bytes from utilityData + 0x14 as the text it prints in "needs %s" messages
   (its buffer is 0x1C bytes), so the "xx KB" string goes there. */
static int sd_sizes(SaveParam *p)
{
    char path[640];
    int *free_info = p->ms_free, *data_info = p->ms_data;
    unsigned char *used = p->utility_data;
    long size;
    int kb = (p->data_size + 0x2000 + 1023) / 1024;   /* data + SFO/icons, rounded like the PSP's 32 KB clusters */

    kb = (kb + 31) / 32 * 32;
    if (free_info) {
        free_info[0] = 32768;
        free_info[1] = 65536;
        free_info[2] = 65536 * 32;
        snprintf((char *)&free_info[3], 8, "%d MB", 2048);
    }
    sd_path(p, p->file_name, path, sizeof path);
    size = file_size(path);
    if (data_info) {
        int *info = (int *)((char *)data_info + 0x24);
        if (size < 0)
            return ERR_RW_NO_DATA;
        info[0] = (int)((size + 32767) / 32768);
        info[1] = info[0];
        info[2] = kb;
        snprintf((char *)&info[3], 8, "%d KB", kb);
        info[5] = kb;
        snprintf((char *)&info[6], 8, "%d KB", kb);
    }
    if (used) {
        memset(used, 0, 0x1C);
        *(int *)(used + 0) = kb / 32;
        *(int *)(used + 4) = kb / 32;
        *(int *)(used + 8) = kb;
        snprintf((char *)used + 0xC, 8, "%d KB", kb);
        snprintf((char *)used + 0x14, 8, "%d KB", kb);
    }
    return 0;
}

/* GETSIZE: whether the save exists; sizeInfo gets the free space and neededKB = 0 (always room). */
static int sd_getsize(SaveParam *p)
{
    char path[640];
    int *si = p->size_info;

    if (si) {
        si[4] = 32768;
        si[5] = 65536;
        si[6] = 65536 * 32;
        snprintf((char *)&si[7], 8, "%d MB", 2048);
        si[9] = 0;
        snprintf((char *)&si[10], 8, "0 KB");
        si[12] = 0;
        snprintf((char *)&si[13], 8, "0 KB");
    }
    sd_path(p, p->file_name, path, sizeof path);
    return file_size(path) < 0 ? ERR_RW_NO_DATA : 0;
}

int sceUtilitySavedataInitStart(SaveParam *p)
{
    int r;

    switch (p->mode) {
    case 0: case 2: r = sd_load(p); break;
    case 1: case 3: r = sd_save(p); break;
    case 7: case 9: r = sd_delete(p); break;
    case 8: r = sd_sizes(p); break;
    case 22: r = sd_getsize(p); break;
    default:
        fprintf(stderr, "savedata: mode %d not implemented\n", p->mode);
        r = ERR_RW_ACCESS;
        break;
    }
    fprintf(stderr, "savedata: mode %d %.13s%.20s/%.13s -> %08x\n", p->mode, p->game_name, p->save_name,
            p->file_name, r);
    p->base[7] = r;
    sd_status = INIT;
    return 0;
}

static int step(int *status)
{
    int r = *status;

    if (r == INIT || r == VISIBLE)
        (*status)++;
    else if (r == FINISHED)
        *status = NONE;
    return r;
}

int sceUtilitySavedataGetStatus(void) { return step(&sd_status); }
int sceUtilitySavedataUpdate(int n) { (void)n; return 0; }
int sceUtilitySavedataShutdownStart(void) { sd_status = FINISHED; return 0; }

/* SceUtilityMsgDialogParams: +0x34 mode (0 error code, 1 text), +0x38 errorValue, +0x3C message[512],
   +0x23C options, +0x240 buttonPressed (1 yes/ok, 2 no, 3 back) */
int sceUtilityMsgDialogInitStart(unsigned char *d)
{
    if (*(int *)(d + 0x34) == 0)
        fprintf(stderr, "msgdialog: error %08x\n", *(unsigned int *)(d + 0x38));
    else
        fprintf(stderr, "msgdialog: \"%.512s\"\n", (char *)d + 0x3C);
    *(int *)(d + 0x1C) = 0;
    *(int *)(d + 0x240) = 1;
    md_status = INIT;
    return 0;
}

int sceUtilityMsgDialogGetStatus(void) { return step(&md_status); }
int sceUtilityMsgDialogUpdate(int n) { (void)n; return 0; }
int sceUtilityMsgDialogShutdownStart(void) { md_status = FINISHED; return 0; }
int sceUtilityMsgDialogAbort(void) { md_status = QUIT; return 0; }

/* SceUtilityOskParams: +0x30 datacount, +0x34 data; SceUtilityOskData: +0x1C desc, +0x20 intext, +0x24
   outtextlength, +0x28 outtext, +0x2C result (0 unchanged, 1 cancelled, 2 changed), +0x30 outtextlimit.
   No keyboard: TAGFORCE_OSK (ASCII) is "typed" when set, else the text is left as it was, or "Player" when it
   was empty (step 6: a new game starts with an empty name, the PSP user types one here; the TTF renders ASCII). */
static int osk_status;

int sceUtilityOskInitStart(unsigned char *p)
{
    unsigned char *d = *(unsigned char **)(p + 0x34);
    const unsigned short *in = *(unsigned short **)(d + 0x20);
    unsigned short *out = *(unsigned short **)(d + 0x28);
    int max = *(int *)(d + 0x24), i = 0;
    const char *typed = getenv("TAGFORCE_OSK");

    if (!typed && (!in || !in[0]))
        typed = "Player";
    if (typed) {
        for (; typed[i] && i < max - 1; i++)
            out[i] = (unsigned char)typed[i];
        *(int *)(d + 0x2C) = 2;
    } else {
        for (; in && in[i] && i < max - 1; i++)
            out[i] = in[i];
        *(int *)(d + 0x2C) = 0;
    }
    out[i] = 0;
    fprintf(stderr, "osk: %s -> %d chars\n", typed ? "TAGFORCE_OSK" : "unchanged", i);
    *(int *)(p + 0x1C) = 0;
    osk_status = INIT;
    return 0;
}

int sceUtilityOskGetStatus(void) { return step(&osk_status); }
int sceUtilityOskUpdate(int n) { (void)n; return 0; }
int sceUtilityOskShutdownStart(void) { osk_status = FINISHED; return 0; }
