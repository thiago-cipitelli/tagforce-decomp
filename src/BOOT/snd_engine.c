// @range 0x0885EB88-0x0885F05C
// @asm 0x0885EB94-0x0885EC2C
/* Sound library, engine side: thread start, the bank file opened for direct reads, the per-frame
   pause/resume step and the frame update of every active sound (waiting for group data, 3D, start,
   fader end, stream end). */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndGroup {
    int data;
    void *buf;
    int unk8;
    unsigned char state;               /* +0x0C 1 failed */
} SndGroup;

extern SndList snd_active;             /* 0x08C1B004 */
extern unsigned short snd_cfg_flags;   /* 0x08C1D894 0x2000 reads take an offset, 0x4000 3D enabled */
extern int snd_data_base;              /* 0x08C1D8B0 */
extern int snd_fd;                     /* 0x08C1D8B4 psp_snddat.bin, -1 when read through the callback */

int snd_thread_init(void *cfg, void *cb, int prio, int prio2);
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

int snd_thread_start(void *cfg, void *cb)
{
    return snd_thread_init(cfg, cb, 0x10, 0x6F);
}

/* Reads go straight to `fd` (or the file named by `fd` when it is a pointer) from offset `off`. */
#ifdef NON_MATCHING /* the original branches forward (branch-likely) into the open and wait paths */
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
