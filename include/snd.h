/* Sound library (BOOT 0x0885C688-0x08865260): bank, objects, voices. Offsets are the ground truth;
   field names are partial. */
#ifndef SND_H
#define SND_H

typedef struct SndList {               /* intrusive list: links at node + link_off */
    void *head;
    void *tail;
    unsigned short count;
    unsigned short link_off;
} SndList;

typedef struct SndFader {              /* 10 bytes: value ramped towards target over `count` ticks */
    short target;
    short cur;                         /* cur << 8 | frac is the 24.8 running value */
    unsigned char frac;
    unsigned char pad5;
    short step;
    short count;
} SndFader;

typedef struct SndFader8 {             /* 8 bytes, same with 8-bit values */
    signed char target;
    signed char cur;
    unsigned char frac;
    unsigned char pad3;
    short step;
    short count;
} SndFader8;

typedef struct SndPos {                /* 3D position of an object, 0x1C */
    float pos[3];
    int pad0c;
    float *src;                        /* +0x10 followed position */
    int pad14;
    unsigned short flags;              /* +0x18 0x20 already in view space, 0x40 follow src, 0x80 has previous position */
    unsigned short pad1a;
} SndPos;

typedef struct SndStream {             /* SndObj +0xA8: decoder and level-meter state */
    int atrac;                         /* +0x00 sceAtrac id (streams) or 0 */
    int pos;                           /* +0x04 stream position, or SAS voice mask */
    int decoded;                       /* +0x08 */
    union {
        struct {                       /* kinds 3-4 */
            int start;                 /* +0x0C */
            int end;                   /* +0x10 */
            int loop;                  /* +0x14 <0 no loop */
            int loop2;                 /* +0x18 */
            unsigned short c4;         /* +0x1C */
        } st;
        struct {                       /* kinds 0-2 */
            unsigned short b4;
            unsigned short send;       /* +0x0E effect send level */
            unsigned short b8;
            unsigned short send_pan;   /* +0x12 */
        } v;
    } u;
    unsigned char pad20[2];
    unsigned short sflags;             /* +0x22 0x10 meter reset, 0x40, 0x80 */
    unsigned short meter;              /* +0x24 */
    unsigned short level;              /* +0x26 */
    short peak;                        /* +0x28 */
    short shown;                       /* +0x2A */
    short step;                        /* +0x2C */
    short acc;                         /* +0x2E */
    signed char tick;                  /* +0x30 */
    unsigned char pad31[3];
    SndFader8 fade8;                   /* +0x34 */
    unsigned char e4, e5, e6, e7;      /* +0x3C */
} SndStream;

typedef struct SndObj {                /* a playing sound, 0x118 per slot of snd_obj_pool */
    unsigned char pad00[0x14];
    void (*on_end)(int handle, unsigned short id); /* +0x14 called when the sound is released */
    unsigned short sid;                /* +0x18 sound id looked up by snd_obj_find */
    unsigned short pad1a;
    unsigned int id;                   /* +0x1C low 16: sound id; bits 16-19 kind; 0x100000 SE; 0x300000 channel */
    void *name;                        /* +0x20 read request of the group data */
    int frames;                        /* +0x24 */
    unsigned short state;              /* +0x28 0x10 paused, 0x20 level meter, 0x40 suspended, 0x80 stop at fade end, 0x100 faders moving, 0x400 owns group data, 0x1000 waiting for data, 0x2000 playing, 0x4000 */
    unsigned short pad2a;
    unsigned short fade_in;            /* +0x2C fader ticks, kept to restart a suspended BGM */
    unsigned short fade_out;           /* +0x2E */
    signed char index;                 /* +0x30 index in snd_obj_pool, <0 when free */
    unsigned char pad31;
    signed char order;                 /* +0x32 start order among sounds of the same class, <0 none */
    unsigned char kind;                /* +0x33 (id >> 16) & 0xF: 0 sequence, 1-2 SAS, 3-4 Atrac stream */
    SndPos p3d;                        /* +0x34 */
    SndFader fade[8];                  /* +0x50 0 channel, 1 stop, 2 pause, 3 volume, 5 distance, 6 pan, 7 pitch */
    short volA0;                       /* +0xA0 */
    short volA2;                       /* +0xA2 volume */
    unsigned short volA4;              /* +0xA4 pitch */
    unsigned short volA6;              /* +0xA6 */
    SndStream s;                       /* +0xA8 */
    int e8;                            /* +0xE8 */
    unsigned char padEC[0x24];
    unsigned char b110;                /* +0x110 */
    unsigned char pad111[2];
    unsigned char b113;                /* +0x113 */
    unsigned short pause_time;         /* +0x114 fader ticks of pause/resume */
    short pan_arg;                     /* +0x116 */
} SndObj;

typedef struct SndVoiceSlot {          /* 32 SAS voices, 12 bytes each, after the busy mask */
    SndObj *obj;
    unsigned char b8;
    unsigned char flags;               /* 0x40 started */
    signed char angle;                 /* +0x06 pan angle */
    unsigned char pad7;
    unsigned char voice;               /* +0x08 SAS voice number */
    unsigned char pad9[3];
} SndVoiceSlot;

void *snd_list_next(SndList *l, void *node);
SndObj *snd_obj_find(unsigned int handle);

#endif
