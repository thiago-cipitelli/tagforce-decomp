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

typedef struct SndObj {                /* a playing sound */
    unsigned char pad00[0x18];
    unsigned short sid;                /* +0x18 sound id looked up by snd_obj_find */
    unsigned char pad1a[2];
    unsigned int id;                   /* +0x1C low 16: sound id; bits 16-19 kind; 0x100000 SE; 0x300000 channel */
    unsigned char pad20[4];
    int frames;                        /* +0x24 */
    unsigned short state;              /* +0x28 0x10 paused, 0x20 level meter, 0x80, 0x100 3D, 0x1000, 0x2000 playing */
    unsigned char pad2a[6];
    signed char index;                 /* +0x30 index in snd_obj_pool, <0 when free */
    unsigned char pad31;
    signed char order;                 /* +0x32 start order among sounds of the same class, <0 none */
    unsigned char pad33;
    SndPos p3d;                        /* +0x34 */
    SndFader fade[8];                  /* +0x50 0 channel, 5 distance, 6 pan, 7 pitch, ... */
    short volA0;                       /* +0xA0 */
    short volA2;                       /* +0xA2 volume */
    unsigned short volA4;              /* +0xA4 */
    unsigned short volA6;              /* +0xA6 */
    int atrac;                         /* +0xA8 sceAtrac id */
    int stream_pos;                    /* +0xAC */
    int decoded;                       /* +0xB0 */
    unsigned char padB4[2];
    unsigned short send;               /* +0xB6 effect send level */
    unsigned char padB8[2];
    unsigned short send_pan;           /* +0xBA */
    unsigned char padBC[0xE];
    unsigned short sflags;             /* +0xCA 0x10 meter reset, 0x40, 0x80 */
    short meter;                       /* +0xCC */
    unsigned short level;              /* +0xCE */
    short peak;                        /* +0xD0 */
    short shown;                       /* +0xD2 */
    short step;                        /* +0xD4 */
    short acc;                         /* +0xD6 */
    signed char tick;                  /* +0xD8 */
} SndObj;

typedef struct SndVoiceSlot {          /* 32 SAS voices, 12 bytes each, after the busy mask */
    SndObj *obj;
    unsigned char b8;
    unsigned char flags;               /* 0x40 started */
    unsigned char pad[2];
    int unk;
} SndVoiceSlot;

void *snd_list_next(SndList *l, void *node);
SndObj *snd_obj_find(unsigned int handle);

#endif
