// @range 0x0885E634-0x0885F05C
// @asm 0x0885EB94-0x0885EC2C
// @data 0x088F03BC-0x088F0428
/* Sound library, engine side: initialization (state, lists, callbacks or heap, semaphores, SAS,
   object pool, threads), thread start, the bank file opened for direct reads, the per-frame
   pause/resume step and the frame update of every active sound (waiting for group data, 3D, start,
   fader end, stream end). */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndCallbacks {
    int (*read)(void *name, void *buf, int size, int off);
    void *(*alloc)(int size);
    void (*free)(void *p);
    void *heap;                        /* +0x0C group data goes to this buffer instead of alloc */
    int heap_size;                     /* +0x10 size of `heap`, or of a VPL created when heap is 0 */
} SndCallbacks;

typedef struct SndVoiceTab {
    unsigned int busy;                 /* mask of SAS voices in use */
    SndVoiceSlot v[32];                /* 0x08C1B058 snd_voice_slots */
} SndVoiceTab;

typedef struct SndCur {
    short slot;                        /* current slot, -1 none */
    short pad;
    SndObj *obj[4];
} SndCur;

typedef struct SndFxState {            /* 0x08C1AFC8 (SndFx in snd_out.c) */
    unsigned char type;
    unsigned char mode;
    unsigned char pad2[2];
    SndFader8 amount;                  /* +0x04 */
    int vpl;                           /* +0x0C */
    short *buf;                        /* +0x10 */
} SndFxState;

typedef struct SndGroup {
    int data;
    void *buf;
    int unk8;
    unsigned char state;               /* +0x0C 1 failed */
} SndGroup;

extern SndList snd_idle_list;          /* 0x08C1AFE0 */
extern SndList snd_stream_list;        /* 0x08C1AFEC */
extern SndList snd_sas_list;           /* 0x08C1AFF8 */
extern SndList snd_active;             /* 0x08C1B004 */
extern SndFader snd_bgm_master;        /* 0x08C1B010 */
extern SndFader snd_master;            /* 0x08C1B01C */
extern short snd_ch_volume[3];         /* 0x08C1B028 */
extern SndCur snd_cur_slot;            /* 0x08C1B030 */
extern int snd_sema;                   /* 0x08C1B044 */
extern int snd_atrac_sema;             /* 0x08C1B048 */
extern int snd_bank_sema;              /* 0x08C1B04C */
extern int snd_obj_sema;               /* 0x08C1B050 */
extern SndVoiceTab snd_voice_tab;      /* 0x08C1B054 */
extern SndObj snd_obj_pool[34];        /* 0x08C1B1D8 */
extern SndFxState snd_fx;              /* 0x08C1AFC8 */
extern int snd_3d;                     /* 0x08C1D750 */
extern unsigned int snd_rand_hist_tab[32][2]; /* 0x08C1D77C */
extern int snd_unused_87c;             /* 0x08C1D87C */
extern int snd_unused_880;             /* 0x08C1D880 */
extern int snd_voice_next;             /* 0x08C1D884 */
extern int snd_voice_cur;              /* 0x08C1D888 */
extern void *snd_bank;                 /* 0x08C1D88C */
extern unsigned char snd_out_mode[4];  /* 0x08C1D890 */
extern short snd_fx_type;              /* 0x08C1D89A */
extern int snd_vpl;                    /* 0x08C1D89C */
extern int snd_group_buf_size;              /* 0x08C1D8A0 */
extern void *snd_group_buf;                 /* 0x08C1D8A4 */
extern int snd_n_groups;               /* 0x08C1D8AC */
extern void *snd_cb_read;              /* 0x08C1D8B8 */
extern void *snd_cb_free;              /* 0x08C1D8BC */
extern void *snd_cb_alloc;             /* 0x08C1D8C0 */
extern int snd_idle_evf;               /* 0x08E1FD84 */
extern unsigned short snd_cfg_flags;   /* 0x08C1D894 0x2000 reads take an offset, 0x4000 3D enabled */
extern int snd_data_base;              /* 0x08C1D8B0 */
extern int snd_fd;                     /* 0x08C1D8B4 psp_snddat.bin, -1 when read through the callback */

void snd_fader_init(SndFader *f, short v);
void snd_fader8_init(SndFader8 *f, signed char v);
int snd_bank_init(void *cfg);
void snd_list_init(SndList *l, int link_off);
int sceWaveInit_wrap(void);
int wave_set_sample(int ch, int n);
int wave_set_volume(int ch, int l, int r);
int snd_sas_open(int grain);
void snd_out_mode_apply(unsigned char a, unsigned char b);
void snd_handles_reset(void);
void snd_fx_setup(int type, int a, int b);
void snd_rand_seed(unsigned int seed);
int snd_thread_main(unsigned int args, void *argp);
int snd_idle_thread(unsigned int args, void *argp);
int sceKernelCreateVpl(const char *name, int part, int attr, int size, void *opt);
int snd_group_of(unsigned int id);
SndGroup *snd_group_get(unsigned int g);
int snd_group_data(unsigned int g);
int snd_stop_fade(unsigned int h, int fade);
void snd_obj_3d(SndObj *o);
int snd_obj_begin(SndObj *o);
int snd_obj_stream_pos(unsigned int h);
void snd_obj_fade(SndObj *o, int which, short value, int time);
int snd_sas_set_pause(unsigned int mask, int on);
int sceIoOpenAsync(const char *path, int flags, int mode);
int sceIoWaitAsyncCB(int fd, long long *res);

/* Returns 0, or 1 bad bank header, 2 heap without size, 3 VPL, 4 semaphore, 5 sceWave, 6 SAS,
   7 thread creation, 8 thread start or event flag, 9 no read callback. The priorities are unused. */
int snd_thread_init(void *cfg, SndCallbacks *cb, int prio, int prio2)
{
    int i;
    int j;
    SndObj *o;
    SndCur *c;
    int th;

    snd_fd = -1;
    snd_data_base = 0;
    snd_fader_init(&snd_master, 0x4000);
    snd_fader_init(&snd_bgm_master, -0x8000);
    i = 0;
    do {
        snd_ch_volume[i] = 0x1000;
    } while (++i < 3);
    snd_cb_alloc = 0;
    snd_cb_free = 0;
    snd_cb_read = 0;
    snd_group_buf = 0;
    snd_group_buf_size = 0;
    snd_vpl = -1;
    snd_fx_type = -1;
    snd_cfg_flags = 0;
    snd_n_groups = 0;
    snd_unused_87c = -1;
    snd_3d = 0;
    snd_unused_880 = 3;
    snd_voice_next = 0;
    snd_voice_cur = 0;
    snd_bank = 0;
    snd_out_mode[1] = 4;
    snd_out_mode[3] = 4;
    snd_out_mode[0] = 1;
    snd_out_mode[2] = 1;
    if (snd_bank_init(cfg) == 0) {
        return 1;
    }
    snd_list_init(&snd_active, 8);
    snd_list_init(&snd_sas_list, 0);
    snd_list_init(&snd_stream_list, 0);
    snd_list_init(&snd_idle_list, 0);
    if (cb != 0) {
        snd_cb_read = cb->read;
        if (snd_cb_read == 0) {
            return 9;
        }
        snd_cb_alloc = cb->alloc;
        snd_cb_free = cb->free;
        snd_group_buf = cb->heap;
        snd_group_buf_size = cb->heap_size;
        if (snd_group_buf != 0) {
            if (snd_group_buf_size == 0) {
                return 2;
            }
            snd_cb_alloc = 0;
            snd_cb_free = 0;
        } else if (snd_group_buf_size != 0) {
            snd_vpl = sceKernelCreateVpl("SndHeapVPL", 2, 0, snd_group_buf_size, 0);
            if (snd_vpl < 0) {
                return 3;
            }
        }
    }
    snd_obj_sema = sceKernelCreateSema("SndSema", 0, 1, 1, 0);
    if (snd_obj_sema <= 0) {
        return 4;
    }
    snd_atrac_sema = sceKernelCreateSema("SndAtracDecodeSema", 0, 1, 1, 0);
    if (snd_atrac_sema <= 0) {
        return 4;
    }
    snd_bank_sema = sceKernelCreateSema("SndLoadSema", 0, 1, 1, 0);
    if (snd_bank_sema <= 0) {
        return 4;
    }
    snd_sema = sceKernelCreateSema("SndSeqSema", 0, 1, 1, 0);
    if (snd_sema <= 0) {
        return 4;
    }
    if (sceWaveInit_wrap() != 0) {
        return 5;
    }
    wave_set_sample(2, 0x100);
    wave_set_volume(2, 0x8000, 0x8000);
    if (snd_sas_open(0x100) < 0) {
        return 6;
    }
    o = snd_obj_pool;
    for (i = 0; i < 34; i++, o++) {
        o->index = -1;
        *(int *)&o->sid = -0x101;
        o->id = -0x101;
        for (j = 0; j < 8; j++) {
            snd_fader_init(&o->fade[j], 0);
        }
    }
    snd_voice_tab.busy = 0;
    i = 0;
    do {
        snd_voice_tab.v[i].obj = 0;
        snd_voice_tab.v[i].flags = 0;
    } while (++i < 32);
    c = &snd_cur_slot;
    c->slot = -1;
    i = 0;
    do {
        c->obj[i] = 0;
    } while (++i < 4);
    for (i = 0; i < 32; i++) {
        snd_rand_hist_tab[i][0] = 0;
    }
    snd_out_mode_apply(4, 1);
    snd_handles_reset();
    snd_fx_setup(-1, 0, 0);
    snd_fx.type = 0;
    snd_fx.vpl = -1;
    snd_fx.buf = 0;
    snd_fader8_init(&snd_fx.amount, 0);
    th = sceKernelCreateThread("snd_thread_main", snd_thread_main, 0x10, 0x3800, 0, 0);
    if (th < 0) {
        return 7;
    }
    if (sceKernelStartThread(th, 0, 0) != 0) {
        return 8;
    }
    th = sceKernelCreateEventFlag("SndSeqEvt", 0, 0, 0);
    if (th <= 0) {
        return 8;
    }
    snd_idle_evf = th;
    th = sceKernelCreateThread("snd_thread_seq", snd_idle_thread, 0x11, 0x1000, 0, 0);
    if (th < 0) {
        return 7;
    }
    if (sceKernelStartThread(th, 0, 0) != 0) {
        return 8;
    }
    snd_rand_seed(0x55555555);
    snd_cfg_flags = 0x8000;
    return 0;
}

int snd_thread_start(void *cfg, SndCallbacks *cb)
{
    return snd_thread_init(cfg, cb, 0x10, 0x6F);
}

/* Reads go straight to `fd` (or the file named by `fd` when it is a pointer) from offset `off`. */
#ifdef NON_MATCHING /* lógica conferida com o asm; o original entra nos ramos de open e wait por bgezl + b (padrão da cópia morta, docs/matching.md) e salva ra antes do primeiro teste: 31 palavras diferem */
void snd_bank_open_async(int fd, int off)
{
    long long res;
    int r = fd;

    if (off < 0) {
        off = 0;
    } else if (fd >= 0x8000000) {
        r = sceIoOpenAsync((const char *)fd, 1, 0);
        if (r < 0) {
            goto done;
        }
        r = sceIoWaitAsyncCB(r, &res);
        if (r == 0) {
            r = res;
        }
    done:
        if (r < 0) {
            return;
        }
        off = 0;
    }
    snd_fd = r;
    snd_data_base = off;
    snd_cfg_flags |= 0x2000;
}
#endif

/* State 0x8 requests pause, 0x4 resume (or the reverse while 0x10 is set): ramps fader 2 and, at the
   end of the ramp, pauses the SAS voices or remembers the stream position. 1 while paused. */
int snd_obj_pause_update(SndObj *o)
{
    unsigned short st = o->state;
    unsigned int kind;
    unsigned int m;
    unsigned short v;
    SndFader *f;

    if (!(st & 0x18)) {
        return 0;
    }
    if ((st & 0x10) && !(st & 8)) {
        return 1;
    }
    kind = (o->id >> 16) & 0xF;
    if (!(st & 2)) {
        o->state |= 2;
        v = 0;
        if (!(o->state & 4)) {
            if (o->state & 0x10) {
                switch (kind) {
                case 2:
                case 1:
                    m = o->s.pos ^ o->s.atrac;
                    if (m != 0) {
                        snd_sas_set_pause(m, 0);
                    }
                    break;
                }
                o->state &= ~0x10;
            }
            v = 0x1000;
        }
        snd_obj_fade(o, 2, v, o->pause_time);
    }
    f = &o->fade[2];
    if (o->fade[2].cur == o->fade[2].target) {
        if (f->count == 0) {
            if (o->state & 4) {
                switch (kind) {
                case 2:
                case 1:
                    m = o->s.pos ^ o->s.atrac;
                    if (m != 0) {
                        snd_sas_set_pause(m, 1);
                    }
                    break;
                case 3:
                case 4:
                    o->e8 = snd_obj_stream_pos((unsigned int)o);
                    break;
                }
                o->state |= 0x10;
            }
            o->state &= 0xFFF3;
            return 1;
        }
    }
    return 0;
}

void snd_stream_update(void)
{
    unsigned int do3d = snd_cfg_flags & 0x4000;
    int i;
    SndObj *cur;
    SndStream *s;
    SndObj *o = snd_list_next(&snd_active, 0);
    unsigned int g;
    SndGroup *gr;
    int moving;
    SndFader *f;
    int r;

    for (;;) {
        if (o == 0) {
            break;
        }
        cur = o;
        o = snd_list_next(&snd_active, o);
        if (cur->state & 0x40) {
            continue;
        }
        if (cur->state & 0x1000) {
            g = snd_group_of(cur->id);
            gr = snd_group_get(g);
            if (gr == 0 || gr->state == 1) {
                snd_stop_fade((unsigned int)cur, 0);
                continue;
            }
            if (snd_group_data(g) == 0) {
                continue;
            }
            cur->state &= ~0x1000;
        }
        if (cur->state & 0x800) {
            continue;
        }
        if ((cur->state & 0x200) && do3d) {
            snd_obj_3d(cur);
        }
        if (!(cur->state & 0x2000)) {
            r = snd_obj_begin(cur);
            if (r < 0) {
                snd_stop_fade((unsigned int)cur, 0);
            } else if (r > 0) {
                cur->state |= 0x2100;
            }
            continue;
        }
        if (snd_obj_pause_update(cur)) {
            continue;
        }
        if (cur->state & 0x100) {
            f = cur->fade;
            moving = 0;
            for (i = 0; i < 8; i++, f++) {
                if (f->cur != f->target || f->count != 0) {
                    moving = 1;
                    break;
                }
            }
            if (!moving) {
                cur->state &= ~0x100;
                if (cur->state & 0x80) {
                    snd_stop_fade((unsigned int)cur, 0);
                }
            }
        }
        if (cur->kind < 3) {
            if ((cur->s.pos ^ cur->s.atrac) == 0) {
                snd_stop_fade((unsigned int)cur, 0);
            }
            continue;
        }
        s = &cur->s;
        if (cur->state & 0x20) {
            s->level = s->meter;
            s->sflags |= 0x10;
        }
        if (s->u.st.loop < 0 && s->u.st.end != 0 && s->pos >= s->u.st.end) {
            snd_stop_fade((unsigned int)cur, 0);
        }
    }
}
