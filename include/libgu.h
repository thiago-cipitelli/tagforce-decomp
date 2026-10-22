#ifndef LIBGU_H
#define LIBGU_H
/* State of the Sony SDK libgu (src/BOOT/libgu*.c): the display-list contexts and the global
   settings, as the library keeps them at 0x088F0CE0-0x088F1258. Only the port C (NON_MATCHING)
   uses these definitions; the PSP build keeps the original data. */

typedef struct GuList {                /* 0x088F0D5C + i * 0xFC, one per context (0..4) */
    int pad0;
    unsigned int *start;               /* +0x04 */
    unsigned int *cur;                 /* +0x08 */
    int arg;                           /* +0x0C */
    int pad10[2];
    int fbp_psm;                       /* +0x18 */
    int disp_w;                        /* +0x1C */
    int disp_h;                        /* +0x20 */
    int scissor_on;                    /* +0x24 */
    int scissor[4];                    /* +0x28 x0, y0, x1, y1 */
    int pad38[15];
    int obj_depth;                     /* +0x74 sceGuBeginObject nesting */
    unsigned int *obj_stack[32];       /* +0x78 */
    int next;                          /* +0xF8 */
} GuList;

typedef struct GuSettings {            /* 0x088F0D10 */
    int initialized;                   /* +0x00 */
    int started;                       /* +0x04 first sceGuStart done */
    int display_on;                    /* +0x08 */
    int cid;                           /* +0x0C current context */
    int call_mode;                     /* +0x10 */
    unsigned int edram;                /* +0x14 sceGeEdramGetAddr */
    int ge_list;                       /* +0x18 */
    int send_list;                     /* +0x1C */
    int ge_cb;                         /* +0x20 */
    int psm;                           /* +0x24 */
    int fbw;                           /* +0x28 */
    unsigned int draw_fbp;             /* +0x2C */
    unsigned int disp_fbp;             /* +0x30 */
    unsigned int depth_fbp;            /* +0x34 */
    int depth_w;                       /* +0x38 */
    int width;                         /* +0x3C */
    int height;                        /* +0x40 */
    unsigned int states;               /* +0x44 */
    GuList *list;                      /* +0x48 */
    GuList lists[5];                   /* +0x4C */
} GuSettings;

typedef struct GuCallbacks {           /* 0x088F0CE0 */
    void (*signal)(int);
    void (*finish)(int);
    short signals[16];                 /* +0x08 */
    int signal_count;                  /* +0x28 */
    int event_flag;                    /* +0x2C */
} GuCallbacks;

extern GuSettings gu;                  /* 0x088F0D10 */
extern GuCallbacks gu_cb;              /* 0x088F0CE0 */

/* context-level command builders (libgu_ctx.c), shared by the sceGu* wrappers */
void gu_ctx_finish(GuList *l, int mode, unsigned int arg);
void gu_ctx_state(GuList *l, int state, int on);
void gu_ctx_matrix(GuList *l, int type, const float *m);
void gu_update_stall(void);

/* float -> GE float24 argument */
#define GU_F24(f) (gu_float_bits(f) >> 8)
static unsigned int gu_float_bits(float f)
{
    union {
        float f;
        unsigned int i;
    } u;

    u.f = f;
    return u.i;
}

#define GU_PUSH(w) (*gu.list->cur++ = (w))
#endif
