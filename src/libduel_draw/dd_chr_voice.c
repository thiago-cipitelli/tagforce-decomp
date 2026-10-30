// @range 0x08865060-0x08866074
// @asm 0x08865374-0x08865BB0
// @asm 0x08865BB0-0x08866028
/* Voice lines of the duel characters: for a model event (summon, attack, damage, ...) picks the
   clips of each step (named characters through their cast table, the generic ones by card kind),
   then plays the clips of a step when the animation reaches its trigger. Tables in .data (asm). */
#include "dd_duelui.h"

typedef struct VoiceStep {
    int trigger;                       /* animation trigger that starts the step */
    int f4;
} VoiceStep;

typedef struct VoiceEvent {            /* 0x0887965C[312], sorted by key */
    int key;
    VoiceStep steps[4];
} VoiceEvent;

typedef struct VoiceRange {            /* 0x0887C23C[29] / 0x0887C398[29]: clip range per event */
    int event;
    int first;
    int last;
} VoiceRange;

typedef struct VoiceCast {             /* 0x0887C4F4[41]: per character and costume */
    short chr;
    short costume;
    int f4;
    int f8;                            /* signature card */
    int cards[4];                      /* cards with their own line */
} VoiceCast;

extern VoiceEvent voice_events[312];
extern VoiceRange voice_ranges[29];
extern VoiceRange voice_ranges2[29];
extern VoiceCast voice_casts[41];
extern VoiceCast voice_cast_default;   /* 0x0887C954 */
extern VoiceEvent voice_event_default; /* 0x0887C970 */

int dd_chara_busy(void);
int func_088069B0(void);
int snd_channel_get_u32(int slot);
int partner_snd_channel(int slot, int se, int delay);
void snd_cmd_fef8_arg(int arg);
unsigned int rand_next(void);
unsigned int card_get_prop5_hi(int id);

void voice_pick_cast(ChrModel *m, int ev, unsigned int flags);
void voice_pick_generic(ChrModel *m, int ev, unsigned int flags);
VoiceCast *voice_cast_find(int chr, int costume);

void voice_tables_init(void)
{
    qsort(voice_events, 312, sizeof(VoiceEvent), dd_int_cmp);
    qsort(voice_ranges, 29, sizeof(VoiceRange), dd_int_cmp);
    qsort(voice_ranges2, 29, sizeof(VoiceRange), dd_int_cmp);
    qsort(voice_casts, 41, sizeof(VoiceCast), dd_int_cmp);
}

void voice_tables_term(void)
{
}

VoiceEvent *voice_event_find(int key)
{
    VoiceEvent k;

    k.key = key;
    return bsearch(&k, voice_events, 312, sizeof(VoiceEvent), dd_int_cmp);
}

/* model event: starts the next step when its trigger comes */
void voice_event(ChrModel *m, int ev, void *a, int trigger, int next)
{
    VoiceEvent *e = voice_event_find(ev);
    VoiceEvent def;
    int i, k;

    if (dd_chara_busy() != 0)
        return;
    if (e == 0) {
        def = voice_event_default;
        def.key = ev;
        e = &def;
    }
    if (m->voice_playing && snd_channel_get_u32(m->snd) == 0)
        m->voice_playing = 0;
    if (next != 0) {
        i = m->voice_step + 1;
        if (i < 4 && m->voices[i][0] >= 0 && e->steps[i].trigger == trigger) {
            m->voice_step = i;
            m->voice_a4 = 0;
            for (k = 0; k < 4 && m->voices[i][k] >= 0; k++)
                partner_snd_channel(m->snd, m->voices[i][k], 8);
            m->voice_playing = 1;
        }
    }
}

void voice_reset(ChrModel *m)
{
    m->voice_playing = 0;
    m->voice_step = -1;
    m->voice_a4 = 0;
    memset(m->voices, 0xFF, sizeof(m->voices));
}

void voice_start(ChrModel *m, int ev, unsigned int flags)
{
    voice_reset(m);
    if (charmap_lookup(m->chr_id, 0) < 0)
        voice_pick_cast(m, ev, flags);
    else
        voice_pick_generic(m, ev, flags);
}

int voice_is_playing(ChrModel *m)
{
    return m->voice_playing != 0;
}

void voice_stop(ChrModel *m)
{
    if (voice_is_playing(m)) {
        snd_cmd_fef8_arg(1);
        m->voice_playing = 0;
    }
}

#define RND(n) ((int)rand_next() % (n))

#ifdef NON_MATCHING /* funções grandes (switch de 29 casos com sorteios); ficam em asm */
/* characters with a cast entry: clips by event, card kind, the character's own cards and the
   flags of the event (clip numbers relative to 0x152) */
void voice_pick_cast(ChrModel *m, int ev, unsigned int flags)
{
    int own[4];
    int clips[4][4];
    int key;
    VoiceRange *r;
    VoiceCast *cast;
    int n, i, k, pick;
    int *p = clips[0];

    pick = -1;
    memset(clips, 0xFF, sizeof(clips));
    key = ev;
    r = bsearch(&key, voice_ranges, 29, sizeof(VoiceRange), dd_int_cmp);
    if (r == 0)
        return;
    cast = voice_cast_find(m->chr_id, m->f4);
    n = 0;
    for (i = 0; i < 4; i++) {
        if (m->card_id == cast->cards[i])
            own[n++] = i;
    }
    if (n > 0)
        pick = own[RND(n)];
    switch (ev) {
    case 0: case 1: case 6: case 9: case 18: case 22: case 23: case 24: case 25: case 26: case 27: case 28:
        clips[0][0] = r->first + RND(r->last - r->first + 1);
        break;
    case 2:
        i = RND(4);
        if (flags & 1) {
            clips[0][0] = i * 2 + 0x154;
            clips[0][1] = i * 2 + 0x155;
        } else if (flags & 2) {
            clips[0][0] = i * 2 + 0x164;
            clips[0][1] = i * 2 + 0x165;
        } else {
            clips[0][0] = i * 2 + 0x15C;
            clips[0][1] = i * 2 + 0x15D;
        }
        break;
    case 3:
        if (flags & 1)
            clips[0][0] = 0x16C;
        else if (flags & 2)
            clips[0][0] = 0x16E;
        else
            clips[0][0] = 0x16D;
        break;
    case 4: case 5: case 7:
        if (flags & 0x10) {
            *p++ = 0x173;
        } else if (flags & 8) {
            *p++ = RND(6) + 0x173;
        }
        if (flags & 0x40)
            *p++ = RND(3) + 0x193;
        else if (flags & 0x20)
            *p++ = 0x179;
        if (card_is_monster(m->card_id) != 0) {
            *p = RND(3) + 0x196;
        } else if (m->card_id == cast->f8) {
            *p = 0x19C;
        } else if (card_get_attribute_icon(m->card_id) == 8) {
            switch (card_get_prop5_hi(m->card_id)) {
            case 4: *p = RND(3) + 0x18D; break;
            case 1: *p = RND(3) + 400; break;
            case 0: *p = RND(3) + 0x18A; break;
            }
        } else if (card_get_attribute_icon(m->card_id) == 7) {
            switch (card_get_prop5_hi(m->card_id)) {
            case 0:
                *p = RND(4) + 0x17A;
                if (func_088069B0() == 0) {
                    if (m->card_id == 0x12EC)
                        p[1] = 0x199;
                    else if (m->card_id == 0x12CC)
                        p[1] = 0x19A;
                }
                break;
            case 2: *p = 0x19B; break;
            case 3: *p = RND(3) + 0x184; break;
            case 4: *p = RND(3) + 0x181; break;
            case 5: *p = RND(3) + 0x17E; break;
            case 6: *p = RND(3) + 0x187; break;
            }
        }
        break;
    case 8: case 10: case 11: case 12: case 13: case 14: case 15:
        if (pick < 0) {
            if (flags & 0x100)
                clips[0][0] = RND(3) + 0x1A8;
            else
                clips[0][0] = RND(4) + 0x19D;
            switch (ev) {
            case 8:
                if (!(flags & 0x200))
                    clips[0][1] = RND(3) + 0x1A1;
                break;
            case 10: clips[0][1] = 0x1A4; break;
            case 11: clips[0][1] = 0x1A5; break;
            case 12: clips[0][1] = 0x1A6; break;
            case 13: clips[0][1] = 0x1A7; break;
            case 14: clips[0][1] = RND(3) + 0x1AB; break;
            case 15: clips[0][1] = RND(3) + 0x1AE; break;
            }
        } else {
            clips[0][0] = pick * 2 + 0x1B1;
            clips[0][1] = pick * 2 + 0x1B2;
        }
        break;
    case 16:
    case 17:
        if (pick < 0) {
            if (flags & 0x80)
                clips[0][0] = RND(4) + 0x1C1;
            else
                clips[0][0] = RND(4) + 0x1BD;
            clips[0][1] = RND(2) + (ev == 16 ? 0x1B9 : 0x1BB);
        } else {
            clips[0][0] = pick * 2 + 0x1C5;
            clips[0][1] = pick * 2 + 0x1C6;
        }
        break;
    case 19:
        clips[0][0] = 0x1D2;
        clips[1][0] = 0x1D6;
        break;
    case 20:
        clips[0][0] = 0x1D3;
        clips[1][0] = 0x1D7;
        break;
    case 21:
        clips[0][0] = 0x1D4;
        clips[1][0] = 0x1D8;
        if (m->chr_id == 0x18 && m->f4 == 1)
            clips[1][0] = 0x1D6;
        break;
    }
    memset(m->voices, 0xFF, sizeof(m->voices));
    for (i = 0; i < 4; i++)
        for (k = 0; k < 4; k++)
            m->voices[k][i] = clips[k][i] - 0x152;
}

/* characters of the charmap: the same events with fixed clip numbers */
void voice_pick_generic(ChrModel *m, int ev, unsigned int flags)
{
    int clips[4][4];
    int key;
    VoiceRange *r;
    int i, k;
    int *p = clips[0];

    memset(clips, 0xFF, sizeof(clips));
    key = ev;
    r = bsearch(&key, voice_ranges2, 29, sizeof(VoiceRange), dd_int_cmp);
    if (r == 0)
        return;
    switch (ev) {
    case 0: case 1: case 6: case 9: case 18: case 22: case 23: case 24: case 25: case 26: case 27: case 28:
        clips[0][0] = r->first + RND(r->last - r->first + 1);
        break;
    case 2:
        if (flags & 1) {
            clips[0][0] = 2;
            clips[0][1] = 3;
        } else if (flags & 2) {
            clips[0][0] = 6;
            clips[0][1] = 7;
        } else {
            clips[0][0] = 4;
            clips[0][1] = 5;
        }
        break;
    case 3:
        if (flags & 1)
            clips[0][0] = 8;
        else if (flags & 2)
            clips[0][0] = 10;
        else
            clips[0][0] = 9;
        break;
    case 4: case 5: case 7:
        if (flags & 0x10)
            *p++ = 0xC;
        else if (flags & 8)
            *p++ = RND(2) + 0xC;
        if (flags & 0x40)
            *p++ = 0x19;
        else if (flags & 0x20)
            *p++ = 0xE;
        if (card_is_monster(m->card_id) != 0) {
            *p = 0x1A;
        } else if (card_get_attribute_icon(m->card_id) == 8) {
            switch (card_get_prop5_hi(m->card_id)) {
            case 4: *p = 0x17; break;
            case 1: *p = 0x18; break;
            case 0: *p = 0x16; break;
            }
        } else if (card_get_attribute_icon(m->card_id) == 7) {
            switch (card_get_prop5_hi(m->card_id)) {
            case 0:
                *p = 0xF;
                if (func_088069B0() == 0) {
                    if (m->card_id == 0x12EC)
                        p[1] = 0x1B;
                    else if (m->card_id == 0x12CC)
                        p[1] = 0x1C;
                }
                break;
            case 2: *p = 0x1D; break;
            case 3: *p = 0x14; break;
            case 4: *p = 0x13; break;
            case 5: *p = 0x12; break;
            case 6: *p = 0x15; break;
            }
        }
        break;
    case 8: case 10: case 11: case 12: case 13: case 14: case 15:
        if (flags & 0x100)
            clips[0][0] = 0x21;
        else
            clips[0][0] = 0x1E;
        switch (ev) {
        case 8:
            if (!(flags & 0x200))
                clips[0][1] = 0x1F;
            break;
        case 10: case 11: case 12: case 13:
            clips[0][1] = 0x20;
            break;
        case 14:
            clips[0][1] = 0x22;
            break;
        case 15:
            clips[0][1] = 0x23;
            break;
        }
        break;
    case 16:
        clips[0][0] = (flags & 0x80) ? 0x27 : 0x26;
        clips[0][1] = 0x24;
        break;
    case 17:
        clips[0][0] = (flags & 0x80) ? 0x27 : 0x26;
        clips[0][1] = 0x25;
        break;
    case 19:
        clips[0][0] = 0x2B;
        break;
    case 20:
        clips[0][0] = 0x2C;
        break;
    case 21:
        clips[0][0] = 0x2D;
        break;
    }
    memset(m->voices, 0xFF, sizeof(m->voices));
    for (i = 0; i < 4; i++)
        for (k = 0; k < 4; k++)
            m->voices[k][i] = clips[k][i];
}
#else
void voice_pick_cast(ChrModel *m, int ev, unsigned int flags);
void voice_pick_generic(ChrModel *m, int ev, unsigned int flags);
#endif

/* cast entry of a character and costume (default entry when absent) */
VoiceCast *voice_cast_find(int chr, int costume)
{
    VoiceCast key;
    VoiceCast *c;

    key.chr = chr;
    key.costume = costume;
    c = bsearch(&key, voice_casts, 41, sizeof(VoiceCast), dd_int_cmp);
    if (c == 0)
        c = &voice_cast_default;
    return c;
}
