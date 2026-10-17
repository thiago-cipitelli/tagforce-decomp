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

typedef struct SndFader {              /* 10 bytes: current/target value ramped over time */
    short cur;
    short target;
    short step;
    short time;
    short pad;
} SndFader;

typedef struct SndObj {                /* a playing sound */
    unsigned char pad00[0x1C];
    unsigned int id;                   /* +0x1C low 16: sound id; bits 16-19 kind; 0x100000 SE; 0x300000 channel */
    unsigned char pad20[4];
    int frames;                        /* +0x24 */
    unsigned short state;              /* +0x28 0x10 paused, 0x20 level meter, 0x80, 0x100 3D, 0x1000, 0x2000 playing */
    unsigned char pad2a[6];
    signed char voice;                 /* +0x30 SAS voice or <0 */
    unsigned char pad31[0x1F];
    SndFader fade[8];                  /* +0x50 */
    unsigned char padA0[2];
    unsigned short volA2;              /* +0xA2 */
    unsigned char padA4[2];
    unsigned short volA6;              /* +0xA6 */
    int atrac;                         /* +0xA8 sceAtrac id */
    int pos;                           /* +0xAC */
    int decoded;                       /* +0xB0 */
    unsigned char padB4[0x16];
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
SndObj *snd_obj_find(int handle);

#endif
