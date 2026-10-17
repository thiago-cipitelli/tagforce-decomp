#ifndef MOVIE_H
#define MOVIE_H
/* PMF (PSMF) movie player, from Sony's sample code: a control block followed by the display,
   sound, reader and video objects, all carved from the memory block set by movie_set_memory. */
#include "psp.h"

typedef struct MovieCtrl {      /* first object of the movie block; every object is 64-byte aligned */
    SceUID control_flag;        /* 1 start display, 2 start sound, 4 pause, 8 stop, 0x10/0x20 thread done, 0x800 error, 0xFF abort */
    SceUID wait_flag;           /* bits 1|2 set by the vblank handler, waited by the display/main loops */
} MovieCtrl;

typedef struct MovieDisp {      /* decoded frame ring (0x28) */
    SceUID sema;
    SceUID delay_flag;
    void **frames;              /* +08 nframes pointers to 512x272x4 buffers */
    int *pts;                   /* +0C */
    int wr;                     /* +10 */
    int rd;                     /* +14 */
    int count;                  /* +18 */
    int nframes;                /* +1C */
    int state;                  /* +20 1 normal, 2 ahead, 4 behind, 0xFF abort (vid_sync_state) */
    MovieCtrl *ctrl;            /* +24 */
} MovieDisp;

typedef struct MovieSnd {       /* decoded Atrac3+ frame ring (0x28) */
    SceUID sema;
    void **bufs;                /* +04 */
    int *pts;                   /* +08 */
    int wr;                     /* +0C */
    int rd;                     /* +10 */
    int count;                  /* +14 */
    int nbufs;                  /* +18 */
    unsigned int bufsize;       /* +1C */
    MovieCtrl *ctrl;            /* +20 */
    int last_pts;               /* +24 */
} MovieSnd;

typedef struct PmfReader {      /* ring-buffer feeder state (0x10, pmf_umd.c) */
    int total;                  /* bytes to feed (stream size) */
    int state;                  /* 0 idle, 1 running, 2 error, 3 done */
    void *rb;                   /* SceMpegRingbuffer */
    MovieCtrl *ctrl;
} PmfReader;

typedef struct MovieVid {       /* video pts queue and A/V sync (0x20) */
    MovieDisp *disp;
    MovieSnd *snd;
    int *pts;                   /* +08 */
    int wr;                     /* +0C */
    int rd;                     /* +10 */
    int count;                  /* +14 */
    int n;                      /* +18 */
    int last_pts;               /* +1C */
} MovieVid;

typedef struct MovieFile {
    SceUID fd;
    char dev[12];               /* "host0:", "disc0:" or "ms0:" */
} MovieFile;

typedef struct MovieAu {        /* at the start of the memory block, before MovieCtrl */
    void *avc_stream;
    void *avc_au;
    void *atrac_stream;
    void *atrac_au;
} MovieAu;

/* pmf_umd.c */
int pmf_reader_attach(PmfReader *s, void *rb, MovieCtrl *ctrl);
int pmf_reader_start(PmfReader *s, int total);
int pmf_reader_term(void);
int pmf_reader_get_state(PmfReader *s);
int pmf_reader_set_state(PmfReader *s, int state);
int pmf_reader_is_paused(PmfReader *s);
int pmf_reader_idle(PmfReader *s);
int pmf_reader_available(PmfReader *s);
int pmf_reader_thread(SceSize args, void *argp);
int umd_retry_init(void);
int umd_retry_term(void);
int DvdUmdRetryOpenCB(const char *path, int flags, int mode);
int DvdUmdRetryRead(SceUID fd, void *buf, SceSize size);
int DvdUmdRetryReadCB(SceUID fd, void *buf, SceSize size);
int pmf_read_blocks(void *buf, int blocks, void *file);
void movie_set_memory(unsigned int base, unsigned int size);

/* movie.c */
int movie_evf_status(MovieCtrl *c, unsigned int *out);
int movie_evf_set(MovieCtrl *c, unsigned int bits);
int movie_read(MovieFile *f, void *buf, int size);
int movie_run(const char *path, unsigned int flags, int volume, int packets);
void movie_exit_cleanup(void);

#endif
