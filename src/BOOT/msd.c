// @range 0x088668C0-0x08868DC8
// @data 0x088F07A4-0x088F07AC
// @rodata 0x088EB030-0x088EB050
/* Save-data front end ("msd"): one state machine per operation, driven once per frame by
   msd_update. Each machine shows in-game message boxes (msgbox_*) around the
   sceUtilitySavedata / sceUtilityMsgDialog calls started in savedata.c. States 96..99 are
   shared: 96 waits msd_wait frames, 97 waits for the message box and jumps to
   msd_next[result], 98 frees everything, 99 returns msd_result. */
#include "psp.h"
#include "libc.h"
#include "savedata.h"

extern unsigned short msd_mode;        /* 0x08E1FD48 operation, see msd_update */
extern unsigned short msd_dialog;      /* 0x08E1FD4A 1 savedata utility, 2 message dialog */
extern unsigned short msd_box_result;  /* 0x08E1FD4C last msgbox_update result */
extern unsigned short msd_state;       /* 0x08E1FD4E */
extern unsigned short msd_wait;        /* 0x08E1FD50 */
extern unsigned short msd_next[6];     /* 0x08E1FD54 next state per msgbox result; [5] also after waits */
extern int msd_pack_ready;             /* 0x08E1FD60 */
extern int msd_box_busy;               /* 0x08E1FD64 */
extern int msd_status;                 /* 0x08E1FD68 utility status */
extern int msd_result;                 /* 0x08E1FD6C 2 ok, 3 failed/cancelled */
extern unsigned char msd_code[8];      /* 0x08E1FD40 */
extern void *msd_heap;                 /* 0x08E1FD70 */
extern SaveParam *msd_param;           /* 0x08E1FD74 */
extern void *msd_data;                 /* 0x08E1FD78 */
extern void *msd_pack;                 /* 0x08E1FD7C */
extern unsigned short *msd_buffer;     /* 0x08E1FD80 */

void *heap_alloc(void *heap, int size);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *), void *arg);
void ehp_mount(int slot, void *pack);
void sysvar_set_a90(int v);
void sysvar_set_a94(int v);
void *sysstate_get(void);
int sysstate_size(void);
void sysstate_checksum(void);
void sceGuSync(int a, int b);
int msgbox_open(unsigned short *text, void *heap, int type, int flag);
int msgbox_update(int *pad);
int msgbox_close(void);
void savedata_free(void);
void savedata_dialog_start(int msg);
void savedata_format_msg(int msg, unsigned short *out);
void savedata_start_listsave(void);
void savedata_start_sizes(void);
void savedata_start_autoload(void);
void savedata_start_save(void);
void savedata_start_delete(void);
int sceUtilitySavedataGetStatus(void);
int sceUtilitySavedataShutdownStart(void);
int sceUtilitySavedataUpdate(int n);
int sceUtilityMsgDialogGetStatus(void);
int sceUtilityMsgDialogShutdownStart(void);
int sceUtilityMsgDialogUpdate(int n);


#define SAVE_MAGIC 0x10002

/* Message box with sysmsg text `msg`. */
#define MSD_BOX(msg, type)                                  \
    memset(msd_buffer, 0, 0x200);                           \
    savedata_format_msg(msg, msd_buffer);                   \
    msgbox_open(msd_buffer, msd_heap, type, 0);             \
    msd_box_busy = 1

/* Mode 3: autoload at boot, no message unless the data is broken. */
int msd_run_autoload(void)
{
    SaveParam *p = msd_param;
    int err;

    switch (msd_state) {
    case 0:
        savedata_start_autoload();
        msd_dialog = 1;
        msd_state++;
        break;
    case 1:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7];
            if (err == 2) {
                msd_state = 0;
                break;
            }
            sysvar_set_a94(0);
            switch (err) {
            case 0:
                if (*(int *)p->data_buf != SAVE_MAGIC) {
                    msd_state = 98;
                    msd_result = 3;
                } else {
                    memcpy(sysstate_get(), p->data_buf, sysstate_size());
                    sysstate_checksum();
                    sysvar_set_a94(1);
                    msd_state = 98;
                    msd_result = 2;
                }
                break;
            case 0x80110327:
            case 0x80110321:
            case 0x80110322:
            case 0x8011032A:
            case 0x80110325:
            default:
                msd_state = 98;
                msd_result = 3;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 96:
        if (msd_wait != 0) {
            msd_wait--;
        } else {
            msd_state = msd_next[5];
        }
        break;
    case 97:
        if (msd_box_busy == 0) {
            msd_state = msd_next[msd_box_result];
        }
        break;
    case 98:
        savedata_free();
        msd_state = 99;
        return msd_result;
    case 99:
        return msd_result;
    }
    return 1;
}

/* Mode 2: check that the save exists (size query). */
int msd_run_check(void)
{
    SaveParam *p = msd_param;
    int err;

    switch (msd_state) {
    case 0:
        MSD_BOX(1, 0);
        msd_next[5] = 1;
        msd_wait = 0x14;
        msd_state = 96;
        break;
    case 1:
        savedata_start_sizes();
        msd_dialog = 1;
        msd_state++;
        break;
    case 2:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7] == 2 ? p->abort_status : p->base[7];
            sysvar_set_a94(0);
            switch (err) {
            case 0:
                sysvar_set_a94(1);
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 98;
                msd_result = 2;
                break;
            case 0x80110327:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 5;
                break;
            case 0x80110321:
            case 0x80110322:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 4;
                break;
            case 0x80110325:
            default:
                msgbox_close();
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 3;
                msd_next[5] = 6;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 3:
        msd_status = sceUtilityMsgDialogGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            msd_state = msd_next[5];
            break;
        case 3:
            sceUtilityMsgDialogShutdownStart();
            break;
        }
        break;
    case 4:
        MSD_BOX(10, 1);
        msd_next[5] = 98;
        msd_state = 97;
        msd_result = 3;
        break;
    case 5:
        MSD_BOX(0xB, 1);
        msd_next[5] = 98;
        msd_state = 97;
        msd_result = 3;
        break;
    case 6:
        MSD_BOX(0x19, 2);
        msd_next[3] = 98;
        msd_next[4] = 0;
        msd_state = 97;
        msd_result = 3;
        break;
    case 96:
        if (msd_wait != 0) {
            msd_wait--;
        } else {
            msd_state = msd_next[5];
        }
        break;
    case 97:
        if (msd_box_busy == 0) {
            msd_state = msd_next[msd_box_result];
        }
        break;
    case 98:
        savedata_free();
        msd_state = 99;
        return msd_result;
    case 99:
        return msd_result;
    }
    return 1;
}

/* Mode 1: start a new game. Lists the saves first to read the save code, then warns when a save
   already exists or the card is full. */
int msd_run_new(void)
{
    SaveParam *p = msd_param;
    int err;

    switch (msd_state) {
    case 0:
        MSD_BOX(1, 0);
        msd_next[5] = 94;
        msd_wait = 0x14;
        msd_state = 96;
        break;
    case 1:
        savedata_start_sizes();
        msd_dialog = 1;
        msd_state++;
        break;
    case 2:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7];
            if (err == 2) {
                msd_state = 1;
                break;
            }
            sysvar_set_a94(0);
            switch (err) {
            case 0:
                sysvar_set_a94(1);
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 98;
                msd_result = 2;
                break;
            case 0x80110327:
                if (((int *)p->size_info)[9] != 0) {
                    msd_next[2] = 4;
                } else {
                    msd_result = 2;
                    msd_next[2] = 98;
                }
                msgbox_close();
                msd_state = 97;
                break;
            case 0x8011032A:
                msgbox_close();
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 3;
                msd_next[5] = 0;
                break;
            case 0x80110321:
            case 0x80110322:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 5;
                break;
            case 0x80110325:
            default:
                msgbox_close();
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 3;
                msd_next[5] = 6;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 3:
        msd_status = sceUtilityMsgDialogGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            msd_state = msd_next[5];
            break;
        case 3:
            sceUtilityMsgDialogShutdownStart();
            break;
        }
        break;
    case 4:
        MSD_BOX(2, 2);
        msd_next[3] = 98;
        msd_next[4] = 0;
        msd_state = 97;
        msd_result = 2;
        break;
    case 5:
        MSD_BOX(4, 2);
        msd_next[3] = 98;
        msd_next[4] = 0;
        msd_state = 97;
        msd_result = 2;
        break;
    case 6:
        MSD_BOX(5, 2);
        msd_next[3] = 98;
        msd_next[4] = 0;
        msd_state = 97;
        msd_result = 2;
        break;
    case 94:
        savedata_start_listsave();
        msd_dialog = 1;
        msd_state++;
        break;
    case 95:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            if (p->base[7] == 2) {
                msd_state = 94;
            } else {
                memcpy(msd_code, (unsigned char *)p->utility_data + 0x14, 8);
                msd_state = 1;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 96:
        if (msd_wait != 0) {
            msd_wait--;
        } else {
            msd_state = msd_next[5];
        }
        break;
    case 97:
        if (msd_box_busy == 0) {
            msd_state = msd_next[msd_box_result];
        }
        break;
    case 98:
        savedata_free();
        msd_state = 99;
        return msd_result;
    case 99:
        return msd_result;
    }
    return 1;
}

#define MSD_POLL_DIALOG()                                   \
    msd_status = sceUtilityMsgDialogGetStatus();            \
    if (msd_status == 4 || msd_status == 1 || msd_status == 2) { \
        break;                                              \
    }                                                       \
    switch (msd_status) {                                   \
    case 0:                                                 \
        msd_dialog = 0;                                     \
        msd_state = msd_next[5];                            \
        break;                                              \
    case 3:                                                 \
        sceUtilityMsgDialogShutdownStart();                 \
        break;                                              \
    }

/* Modes 4 and 7: load the save (7 skips the closing "loaded" message). */
int msd_run_load(void)
{
    SaveParam *p = msd_param;
    int err;

    switch (msd_state) {
    case 0:
        MSD_BOX(1, 0);
        msd_next[5] = 1;
        msd_wait = 0x14;
        msd_state = 96;
        break;
    case 1:
        savedata_start_sizes();
        msd_dialog = 1;
        msd_state++;
        break;
    case 2:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7] == 2 ? p->abort_status : p->base[7];
            sysvar_set_a94(0);
            switch (err) {
            case 0:
                sysvar_set_a94(1);
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 9;
                break;
            case 0x80110327:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 8;
                break;
            case 0x80110321:
            case 0x80110322:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 7;
                break;
            case 0x8011032A:
            case 0x80110325:
            default:
                msgbox_close();
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 5;
                msd_next[5] = 6;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 3:
        savedata_start_autoload();
        msd_dialog = 1;
        msd_state++;
        break;
    case 4:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7] == 2 ? p->abort_status : p->base[7];
            sysvar_set_a94(0);
            switch (err) {
            case 0:
                if (*(int *)p->data_buf != SAVE_MAGIC) {
                    msgbox_close();
                    savedata_dialog_start(0x80110326);
                    msd_dialog = 2;
                    msd_state = 5;
                    msd_next[5] = 6;
                    break;
                }
                memcpy(sysstate_get(), p->data_buf, sysstate_size());
                sysstate_checksum();
                sysvar_set_a94(1);
                msgbox_close();
                msd_next[2] = 10;
                msd_state = 97;
                if (msd_mode == 7) {
                    msd_result = 2;
                    msd_next[2] = 98;
                }
                break;
            case 0x80110327:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 8;
                break;
            case 0x80110321:
            case 0x80110322:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 7;
                break;
            case 0x8011032A:
            case 0x80110325:
            default:
                msgbox_close();
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 5;
                msd_next[5] = 6;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 5:
        MSD_POLL_DIALOG();
        break;
    case 6:
        MSD_BOX(6, 2);
        msd_next[3] = 98;
        msd_next[4] = 0;
        msd_state = 97;
        msd_result = 3;
        break;
    case 7:
        MSD_BOX(10, 1);
        msd_next[5] = 6;
        msd_state = 97;
        break;
    case 8:
        MSD_BOX(0xB, 1);
        msd_next[5] = 6;
        msd_state = 97;
        break;
    case 9:
        MSD_BOX(8, 0);
        msd_next[5] = 3;
        msd_wait = 0x14;
        msd_state = 96;
        break;
    case 10:
        MSD_BOX(9, 1);
        msd_next[5] = 98;
        msd_state = 97;
        msd_result = 2;
        break;
    case 96:
        if (msd_wait != 0) {
            msd_wait--;
        } else {
            msd_state = msd_next[5];
        }
        break;
    case 97:
        if (msd_box_busy == 0) {
            msd_state = msd_next[msd_box_result];
        }
        break;
    case 98:
        savedata_free();
        msd_state = 99;
        return msd_result;
    case 99:
        return msd_result;
    }
    return 1;
}

/* Mode 5: save. Asks before overwriting; on a full card offers to delete the old save. */
int msd_run_save(void)
{
    SaveParam *p = msd_param;
    int res;
    int err;

    switch (msd_state) {
    case 0:
        MSD_BOX(1, 0);
        msd_next[5] = 1;
        msd_wait = 0x14;
        msd_state = 96;
        break;
    case 1:
        savedata_start_sizes();
        msd_dialog = 1;
        msd_state++;
        break;
    case 2:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7] == 2 ? p->abort_status : p->base[7];
            sysvar_set_a94(0);
            switch (err) {
            case 0:
                sysvar_set_a94(1);
                msgbox_close();
                msd_state = 3;
                break;
            case 0x80110327:
                if (((int *)p->size_info)[9] != 0) {
                    msd_next[2] = 8;
                } else {
                    msd_next[2] = 3;
                }
                msgbox_close();
                msd_state = 97;
                break;
            case 0x80110321:
            case 0x80110322:
                msgbox_close();
                msd_state = 97;
                msd_next[2] = 7;
                break;
            case 0x8011032A:
            case 0x80110325:
            default:
                msgbox_close();
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 5;
                msd_next[5] = 6;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 3:
        savedata_start_save();
        msd_dialog = 1;
        msd_state++;
        break;
    case 4:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            res = p->base[7];
            err = res == 2 ? p->abort_status : res;
            switch (err) {
            case 0:
                sysvar_set_a94(1);
                sysstate_checksum();
                msd_result = 2;
                msd_state = 98;
                break;
            case 1:
                if (res == 2) {
                    msd_box_busy = 1;
                    msd_next[5] = 6;
                    msd_wait = 0x10;
                    msd_state = 96;
                } else {
                    msd_result = 3;
                    msd_state = 98;
                }
                break;
            default:
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 5;
                if (p->base[7] == 2) {
                    msd_next[5] = 6;
                } else {
                    msd_next[5] = 98;
                    msd_result = 3;
                }
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 5:
        MSD_POLL_DIALOG();
        break;
    case 6:
        MSD_BOX(7, 2);
        msd_result = 3;
        msd_next[3] = 98;
        msd_next[4] = 0;
        msd_state = 97;
        break;
    case 7:
        MSD_BOX(10, 1);
        msd_next[5] = 6;
        msd_state = 97;
        break;
    case 8:
        MSD_BOX(3, 2);
        msd_next[3] = 10;
        msd_next[4] = 6;
        msd_state = 97;
        break;
    case 10:
        savedata_start_delete();
        msd_dialog = 1;
        msd_state++;
        break;
    case 11:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7] == 2 ? p->abort_status : p->base[7];
            switch (err) {
            case 0:
            case 0x80110347:
            case 1:
                msd_state = 6;
                break;
            default:
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 5;
                msd_next[5] = 6;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 96:
        if (msd_wait != 0) {
            msd_wait--;
        } else {
            msd_state = msd_next[5];
        }
        break;
    case 97:
        if (msd_box_busy == 0) {
            msd_state = msd_next[msd_box_result];
        }
        break;
    case 98:
        savedata_free();
        msd_state = 99;
        return msd_result;
    case 99:
        return msd_result;
    }
    return 1;
}

/* Mode 6: delete the save. */
int msd_run_delete(void)
{
    SaveParam *p = msd_param;
    int err;

    switch (msd_state) {
    case 0:
        MSD_BOX(1, 0);
        msd_next[5] = 1;
        msd_wait = 0x14;
        msd_state = 96;
        break;
    case 1:
        msgbox_close();
        savedata_start_delete();
        msd_dialog = 1;
        msd_state++;
        break;
    case 2:
        msd_status = sceUtilitySavedataGetStatus();
        if (msd_status == 4 || msd_status == 1 || msd_status == 2) {
            break;
        }
        switch (msd_status) {
        case 0:
            msd_dialog = 0;
            err = p->base[7] == 2 ? p->abort_status : p->base[7];
            msd_result = 2;
            switch (err) {
            case 0:
            case 0x80110347:
            case 1:
                msd_result = 2;
                msd_state = 98;
                break;
            default:
                savedata_dialog_start(err);
                msd_dialog = 2;
                msd_state = 5;
                msd_next[5] = 98;
                break;
            }
            break;
        case 3:
            sceUtilitySavedataShutdownStart();
            break;
        }
        break;
    case 5:
        MSD_POLL_DIALOG();
        break;
    case 96:
        if (msd_wait != 0) {
            msd_wait--;
        } else {
            msd_state = msd_next[5];
        }
        break;
    case 97:
        if (msd_box_busy == 0) {
            msd_state = msd_next[msd_box_result];
        }
        break;
    case 98:
        savedata_free();
        msd_state = 99;
        return msd_result;
    case 99:
        return msd_result;
    }
    return 1;
}

static int msd_pack_loaded(void *arg)
{
    msd_pack_ready = 1;
    ehp_mount(0, msd_pack);
    return 1;
}

/* Exported: start save-data operation `mode` (see msd_update) with buffers from `heap`. */
int msd_open(int mode, void *heap)
{
    int size;

    memset(&msd_mode, 0, 0x3C);
    msd_mode = mode;
    msd_heap = heap;
    msd_buffer = heap_alloc(heap, 0x200);
    if (msd_buffer == 0) {
        return 0;
    }
    size = ehfile_get_size(6, "msd.ehp");
    msd_pack = heap_alloc(msd_heap, size);
    if (msd_pack == 0) {
        savedata_free();
        return 0;
    }
    msd_param = heap_alloc(msd_heap, 0x950);
    if (msd_param == 0) {
        savedata_free();
        return 0;
    }
    memset(msd_param, 0, 0x950);
    msd_data = heap_alloc(msd_heap, 0x10000);
    if (msd_data == 0) {
        savedata_free();
        return 0;
    }
    if (msd_pack != 0) {
        ehfile_read_async_alias(6, "msd.ehp", msd_pack, size, msd_pack_loaded, msd_pack);
    }
    msd_state = 0;
    return 1;
}

/* Exported: run one frame of the operation; returns 1 while busy, then msd_result. */
int msd_update(void)
{
    int r;

    if (msd_pack != 0 && msd_pack_ready == 0) {
        r = 1;
    } else {
        switch (msd_mode) {
        case 1:
            r = msd_run_new();
            break;
        case 2:
            r = msd_run_check();
            break;
        case 3:
            r = msd_run_autoload();
            break;
        case 4:
        case 7:
            r = msd_run_load();
            break;
        case 5:
            r = msd_run_save();
            break;
        case 6:
            r = msd_run_delete();
            break;
        default:
            r = 1;
            break;
        }
    }
    if (r == 1) {
        sysvar_set_a90(1);
    } else {
        sysvar_set_a90(0);
    }
    return r;
}

/* Exported: per-frame draw-side update of the message box and the utility dialogs. */
void msd_update_dialog(void)
{
    if (msd_box_busy != 0) {
        msd_box_result = msgbox_update(0);
        if (msd_box_result != 1) {
            msd_box_busy = 0;
        }
    }
    sceGuSync(0, 0);
    switch (msd_status) {
    case 2:
        if (msd_dialog == 1) {
            sceUtilitySavedataUpdate(2);
        } else {
            sceUtilityMsgDialogUpdate(2);
        }
        break;
    }
}
