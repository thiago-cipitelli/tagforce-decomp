// @range 0x08866068-0x088668C0
// @data 0x088F078C-0x088F07A4
// @asm 0x088662DC-0x0886651C
/* Save data through sceUtilitySavedata: fills the 0x600-byte param block (SFO texts from sysmsg,
   icon0/pic1 from msd.ehp, the fixed encryption key), message dialog, and the per-mode init
   helpers (autosave, save, load, delete, size query). The update state machines that follow stay
   in asm for now. */
#include "psp.h"
#include "libc.h"
#include "savedata.h"

extern unsigned char msd_code[8];      /* 0x08E1FD40 */
extern void *msd_heap;                 /* 0x08E1FD70 */
extern SaveParam *msd_param;           /* 0x08E1FD74 */
extern void *msd_data;                 /* 0x08E1FD78 */
extern void *msd_pack;                 /* 0x08E1FD7C */
extern void *msd_buffer;               /* 0x08E1FD80 */
extern unsigned char msd_key[16];      /* 0x088EAFD0 */
extern char savedata_data0[];          /* "DATA0", defined in sndapi.c */

unsigned short *sysmsg_get(int i);
void wcs_to_utf8(char *dst, int size, unsigned short *src);
int region_to_lang_group(void);
void *ehp_find(void *pack, const char *name, int flags);
int ehp_file_size(void *pack, const char *name);
void heap_set_level(void *heap, void *p);
void savedata_params_init(int *p, int size);
void savedata_set_gameid(char *p);
unsigned char *sysstate_get(void);
unsigned int sysstate_size(void);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
int sceUtilityMsgDialogInitStart(void *p);
int sceUtilitySavedataInitStart(void *p);

/* SFO texts: title, save title, detail from sysmsg 0xC..0xF. */
void savedata_fill_sfo(SaveSfo *sfo)
{
    wcs_to_utf8(sfo->title, 0x80, sysmsg_get(0xC));
    wcs_to_utf8(sfo->savedata_title, 0x80, sysmsg_get(0xD));
    wcs_to_utf8(msd_param->title_buf, 0x80, sysmsg_get(0xE));
    wcs_to_utf8(sfo->detail, 0x400, sysmsg_get(0xF));
    sfo->parental_level = region_to_lang_group();
    sfo->unk[0] = 4;
}

void savedata_load_icons(void)
{
    SaveParam *p = msd_param;
    SaveNewData *nd = &msd_param->new_buf;

    p->icon0.buf = ehp_find(msd_pack, "icon0.png", 0);
    p->icon0.size = p->icon0.bufsize = ehp_file_size(msd_pack, "icon0.png");
    p->pic1.buf = ehp_find(msd_pack, "pic1.png", 0);
    p->pic1.size = p->pic1.bufsize = ehp_file_size(msd_pack, "pic1.png");
    nd->icon0 = p->icon0;
    nd->title = msd_param->title_buf;
    p->new_data = nd;
}

void savedata_dialog_start(int msg)
{
    unsigned char *d = msd_param->dialog;

    memset(d, 0, 0x244);
    savedata_params_init((int *)d, 0x244);
    *(int *)(d + 0x34) = 0;
    *(int *)(d + 0x38) = msg;
    sceUtilityMsgDialogInitStart(d);
}

void savedata_free(void)
{
    if (msd_param != 0) {
        heap_set_level(msd_heap, msd_param);
        msd_param = 0;
    }
    if (msd_data != 0) {
        heap_set_level(msd_heap, msd_data);
        msd_data = 0;
    }
    if (msd_pack != 0) {
        heap_set_level(msd_heap, msd_pack);
        msd_pack = 0;
    }
    if (msd_buffer != 0) {
        heap_set_level(msd_heap, msd_buffer);
        msd_buffer = 0;
    }
}

#ifdef NON_MATCHING /* the original is 0x30 bytes longer: each wsprintf call site keeps its own sysmsg_get */
/* Message text for dialog `msg`, with the save code (8 bytes) or "!" where the message has %s/%c. */
void savedata_format_msg(int msg, unsigned short *out)
{
    unsigned short code[8];

    if (msg == 0x19 || msg == 0xB || msg == 9 || msg == 7 || msg == 6) {
        wsprintf(out, sysmsg_get(msg));
    } else if (msg == 10 || msg == 8) {
        wsprintf(out, sysmsg_get(msg), 0x2122);
    } else if (msg == 5) {
        memset(code, 0, 0x10);
        code[0] = msd_code[0];
        code[1] = msd_code[1];
        code[2] = msd_code[2];
        code[3] = msd_code[3];
        code[4] = msd_code[4];
        code[5] = msd_code[5];
        code[6] = msd_code[6];
        code[7] = msd_code[7];
        wsprintf(out, sysmsg_get(5), code);
    } else if (msg == 4 || msg == 3 || msg == 2) {
        memset(code, 0, 0x10);
        code[0] = msd_code[0];
        code[1] = msd_code[1];
        code[2] = msd_code[2];
        code[3] = msd_code[3];
        code[4] = msd_code[4];
        code[5] = msd_code[5];
        code[6] = msd_code[6];
        code[7] = msd_code[7];
        wsprintf(out, sysmsg_get(msg), 0x2122, code);
    } else if (msg == 1) {
        wsprintf(out, sysmsg_get(msg), 0x2122, 0x2122);
    }
}

#endif

#define SAVE_COMMON(MODE, OVERWRITE)                              \
    memset(msd_param, 0, 0x600);                                  \
    savedata_params_init((int *)p, 0x600);                        \
    p->mode = MODE;                                               \
    p->overwrite = OVERWRITE;                                     \
    savedata_set_gameid((char *)p);                               \
    p->data_buf = msd_data;                                       \
    p->data_size = 0x10000;                                       \
    p->data_buf_size = 0x10000

#define SAVE_LIST_TAIL()                                          \
    q = msd_param;                                                \
    p->size_info = q->size_info_buf;                              \
    q->size_info_buf[0] = 1;                                      \
    q->size_info_buf[1] = 0;                                      \
    r = msd_param;                                                \
    e = &r->list;                                                 \
    *(SaveListEntry **)&q->size_info_buf[2] = e;                  \
    r->list.a = 0x10000;                                          \
    r->list.b = 0;                                                \
    strcpy(e->name, savedata_data0)

/* Mode 8: list save. */
void savedata_start_listsave(void)
{
    SaveParam *p = msd_param;
    SaveParam *q, *r;
    SaveListEntry *e;

    SAVE_COMMON(8, 0);
    savedata_fill_sfo(&p->sfo);
    savedata_load_icons();
    p->utility_data = msd_param->util_data;
    memcpy(p->key, msd_key, 0x10);
    p->secure_version = 0;
    p->multi_status = 0;
    SAVE_LIST_TAIL();
    sceUtilitySavedataInitStart(p);
}

/* Mode 0x16: size query of the list save. */
void savedata_start_sizes(void)
{
    SaveParam *p = msd_param;
    SaveParam *q, *r;
    SaveListEntry *e;

    SAVE_COMMON(0x16, 0);
    savedata_fill_sfo(&p->sfo);
    savedata_load_icons();
    memcpy(p->key, msd_key, 0x10);
    p->secure_version = 0;
    p->multi_status = 0;
    SAVE_LIST_TAIL();
    sceUtilitySavedataInitStart(p);
}

/* Mode 0: autoload. */
void savedata_start_autoload(void)
{
    SaveParam *p = msd_param;

    SAVE_COMMON(0, 0);
    savedata_fill_sfo(&p->sfo);
    savedata_load_icons();
    p->focus = 3;
    memcpy(p->key, msd_key, 0x10);
    p->secure_version = 0;
    p->multi_status = 0;
    sceUtilitySavedataInitStart(p);
}

/* Mode 3: save, copying the whole system state into the data buffer. */
void savedata_start_save(void)
{
    SaveParam *p = msd_param;

    SAVE_COMMON(3, 1);
    memcpy(p->data_buf, sysstate_get(), sysstate_size());
    savedata_fill_sfo(&p->sfo);
    savedata_load_icons();
    memcpy(p->key, msd_key, 0x10);
    p->secure_version = 0;
    p->multi_status = 0;
    sceUtilitySavedataInitStart(p);
}

/* Mode 7: delete. */
void savedata_start_delete(void)
{
    SaveParam *p = msd_param;

    memset(msd_param, 0, 0x600);
    savedata_params_init((int *)p, 0x600);
    p->mode = 7;
    p->secure_version = 0;
    p->multi_status = 0;
    sceUtilitySavedataInitStart(p);
}
