// @range 0x088676D8-0x08867EE0
// @asm 0x08867890-0x08867978
// @asm 0x08867B40-0x08867C28
// @asm 0x08867DF0-0x08867ED8
// @data 0x0887C9D4-0x0887CBD0
// @rodata 0x08872A04-0x08872B98
/* Per-character model handlers of the duel scene (the opponent's 3D model): each block is the
   setup that fills a ChrVt with the character's callbacks, the init/event/update/motion overrides
   (most only forward to the base model), the map from bone slot to bone id and the rest pose of
   the bones (table sorted by id on first use and searched with bsearch). */
#include "dd_duelui.h"

int chr15_pairs[7] = { 40, 34 }; /* 0x0887C9D4 */
ChrBone chr15_bones[5] = { /* 0x0887C9F0 */
    { 40, -12.269158f, -0.07377695f, 0.7103463f, { -0.011740291f, -0.056571048f, -0.0030753254f, 0.9983248f } },
    { 37, -8.041381f, -1.868716f, -0.6764291f, { 0.05526083f, -0.7058925f, -0.055408515f, -0.7039829f } },
    { 38, -0.16214149f, 3.6154706f, -0.016765239f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr40_bones[5] = { /* 0x0887CA90 */
    { 40, -44.27786f, 2.2241292f, -11.201814f, { -0.0065734657f, 0.06163295f, -0.0015248023f, 0.9980761f } },
    { 37, -7.7850266f, -1.8097235f, -0.7587733f, { -0.05632787f, 0.7048597f, 0.056328032f, 0.7048597f } },
    { 38, -25.285643f, 3.6355724f, -11.901963f, { -6.947231e-05f, 0.00066360744f, 0.001071708f, 0.9999992f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr1000_bones[5] = { /* 0x0887CB30 */
    { 40, -44.261738f, 2.2172053f, -11.249005f, { -0.0012811334f, 0.060936045f, -0.0018722527f, 0.9981391f } },
    { 37, -8.063457f, -2.2417245f, -0.7449553f, { -0.055478957f, 0.7049269f, 0.055478945f, 0.7049271f } },
    { 38, -25.317606f, 3.6154706f, -12.11088f, { -0.00023878129f, -2.454672e-05f, 0.002278113f, 0.9999974f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};

/* character 15 */
int chr15_init(void *self, void *a, void *b);
int chr15_event(void *self, int ev, void *a, void *b);
int chr15_update(void *self, void *a, void *b, void *c, void *d);
int chr15_bone_id(void *self, int i);
int chr15_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr15_motion(void *self, void *a, void *b, void *c, int d);

void chr15_setup(ChrVt *vt)
{
    vt->init = chr15_init;
    vt->event = chr15_event;
    vt->update = chr15_update;
    vt->bone_id = chr15_bone_id;
    vt->bone_rest = chr15_bone_rest;
    vt->motion = chr15_motion;
    vt->pairs = chr15_pairs;
}

int chr15_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr15_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr15_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr15_bone_id(void *self, int i)
{
    switch (i) {
    case 29:
        i = 21;
        break;
    }
    switch (i) {
    case 14: return 5;
    case 7: return 66;
    case 4: return 61;
    case 20: return 17;
    case 12: return 71;
    case 19: return 7;
    case 26: return 13;
    case 11: return 70;
    case 6: return 65;
    case 16: return 2;
    case 31: return 53;
    case 8: return 67;
    case 34: return 28;
    case 5: return 64;
    case 15: return 3;
    case 32: return 54;
    case 13: return 0;
    case 25: return 40;
    case 22: return 15;
    case 23: return 42;
    case 18: return 8;
    case 10: return 69;
    case 21: return 16;
    case 24: return 41;
    case 9: return 68;
    case 17: return 10;
    case 0: return 43;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr15_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B9AC */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr15_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr15_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr15_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 40 */
int chr40_init(void *self, void *a, void *b);
int chr40_event(void *self, int ev, void *a, void *b);
int chr40_update(void *self, void *a, void *b, void *c, void *d);
int chr40_bone_id(void *self, int i);
int chr40_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr40_motion(void *self, void *a, void *b, void *c, int d);

void chr40_setup(ChrVt *vt)
{
    vt->init = chr40_init;
    vt->event = chr40_event;
    vt->update = chr40_update;
    vt->bone_id = chr40_bone_id;
    vt->bone_rest = chr40_bone_rest;
    vt->motion = chr40_motion;
}

int chr40_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr40_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr40_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr40_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 42;
    case 4: return 48;
    case 2: return 62;
    case 1: return 60;
    case 20: return 58;
    case 12: return 47;
    case 19: return 7;
    case 26: return 13;
    case 11: return 46;
    case 6: return 41;
    case 16: return 2;
    case 31: return 16;
    case 8: return 43;
    case 5: return 40;
    case 15: return 3;
    case 32: return 17;
    case 30: return 65;
    case 13: return 0;
    case 29: return 61;
    case 25: return 70;
    case 3: return 63;
    case 22: return 56;
    case 23: return 72;
    case 18: return 8;
    case 10: return 45;
    case 21: return 57;
    case 24: return 71;
    case 9: return 44;
    case 17: return 10;
    case 0: return 73;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr40_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B9B0 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr40_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr40_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr40_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* chr1000 */
int chr1000_init(void *self, void *a, void *b);
int chr1000_event(void *self, int ev, void *a, void *b);
int chr1000_update(void *self, void *a, void *b, void *c, void *d);
int chr1000_bone_id(void *self, int i);
int chr1000_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr1000_motion(void *self, void *a, void *b, void *c, int d);

void chr1000_setup(ChrVt *vt)
{
    vt->init = chr1000_init;
    vt->event = chr1000_event;
    vt->update = chr1000_update;
    vt->bone_id = chr1000_bone_id;
    vt->bone_rest = chr1000_bone_rest;
    vt->motion = chr1000_motion;
}

int chr1000_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr1000_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr1000_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr1000_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 29;
    case 7: return 65;
    case 4: return 53;
    case 2: return 71;
    case 1: return 57;
    case 20: return 41;
    case 12: return 70;
    case 19: return 3;
    case 26: return 19;
    case 11: return 69;
    case 6: return 64;
    case 16: return 2;
    case 31: return 45;
    case 8: return 66;
    case 5: return 63;
    case 15: return 11;
    case 32: return 46;
    case 30: return 75;
    case 13: return 0;
    case 29: return 62;
    case 25: return 34;
    case 3: return 72;
    case 22: return 32;
    case 23: return 43;
    case 18: return 12;
    case 10: return 68;
    case 21: return 38;
    case 24: return 39;
    case 9: return 67;
    case 17: return 30;
    case 0: return 58;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr1000_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B9B4 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr1000_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr1000_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr1000_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}
