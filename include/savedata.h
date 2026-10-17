#ifndef SAVEDATA_H
#define SAVEDATA_H

typedef struct SaveFileData {
    void *buf;
    int bufsize;
    int size;
    int unk;
} SaveFileData;

typedef struct SaveNewData {           /* SceUtilitySavedataSaveNewData */
    SaveFileData icon0;
    char *title;
} SaveNewData;

typedef struct SaveListEntry {
    int a;
    int b;
    char name[16];
} SaveListEntry;

typedef struct SaveSfo {               /* +0x80 */
    char title[0x80];
    char savedata_title[0x80];
    char detail[0x400];
    unsigned char parental_level;
    unsigned char unk[3];
} SaveSfo;

typedef struct SaveParam {             /* SceUtilitySavedataParam + game extras, 0x8CC+ */
    int base[12];                      /* +0x00 common header (savedata_params_init) */
    int mode;                          /* +0x30 */
    int bind;
    int overwrite;                     /* +0x38 */
    char game_name[13];                /* +0x3C */
    char pad49[3];
    char save_name[20];                /* +0x4C */
    void *save_name_list;              /* +0x60 */
    char file_name[13];                /* +0x64 */
    char pad71[3];
    void *data_buf;                    /* +0x74 */
    int data_buf_size;                 /* +0x78 */
    int data_size;                     /* +0x7C */
    SaveSfo sfo;                       /* +0x80 */
    SaveFileData icon0;                /* +0x584 */
    SaveFileData icon1;                /* +0x594 */
    SaveFileData pic1;                 /* +0x5A4 */
    SaveFileData snd0;                 /* +0x5B4 */
    void *new_data;                    /* +0x5C4 */
    int focus;                         /* +0x5C8 */
    int abort_status;
    void *ms_free;
    void *ms_data;
    void *utility_data;                /* +0x5D8 */
    unsigned char key[16];             /* +0x5DC */
    int secure_version;                /* +0x5EC */
    int multi_status;                  /* +0x5F0 */
    void *id_list;
    void *file_list;
    void *size_info;                   /* +0x5FC */
    SaveNewData new_buf;               /* +0x600 */
    unsigned char util_data[0x1C];     /* +0x614 */
    int size_info_buf[16];             /* +0x630 */
    SaveListEntry list;                /* +0x670 */
    unsigned char dialog[0x244];       /* +0x688 SceUtilityMsgDialogParams */
    char title_buf[0x80];              /* +0x8CC */
} SaveParam;

#endif
