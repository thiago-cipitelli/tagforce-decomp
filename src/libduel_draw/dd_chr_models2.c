// @range 0x0885F980-0x08861238
// @asm 0x08860BE8-0x08860CD0
// @asm 0x08860E98-0x08860F80
// @asm 0x08861148-0x08861230
// @data 0x08878CC0-0x08878EA0
// @rodata 0x08871ED0-0x08872794
/* Per-character model handlers of the duel scene (the opponent's 3D model): each block is the
   setup that fills a ChrVt with the character's callbacks, the init/event/update/motion overrides
   (most only forward to the base model), the map from bone slot to bone id and the rest pose of
   the bones (table sorted by id on first use and searched with bsearch). */
#include "libduel_draw.h"

ChrBone chr_generic_bones[5] = { /* 0x08878CC0 */
    { 40, -44.261738f, 2.2172053f, -11.249005f, { -0.0012811334f, 0.060936045f, -0.0018722527f, 0.9981391f } },
    { 37, -8.063457f, -2.2417245f, -0.7449553f, { -0.055478957f, 0.7049269f, 0.055478945f, 0.7049271f } },
    { 38, -25.365465f, 3.6239831f, -12.089487f, { -0.00023878129f, -2.454672e-05f, 0.002278113f, 0.9999974f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr38_bones[5] = { /* 0x08878D60 */
    { 40, -44.31473f, 2.305477f, -11.138741f, { -0.0061680437f, 0.06288665f, -0.0015504871f, 0.9980004f } },
    { 37, -7.785446f, -2.2130919f, -0.7573639f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.414389f, 3.6413865f, -12.05114f, { 0.00033869813f, 0.0019179317f, 0.0022188486f, 0.99999565f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr19_bones[5] = { /* 0x08878E00 */
    { 40, -44.421146f, 2.2944636f, -10.913509f, { -0.0065734535f, 0.061632924f, -0.0015248026f, 0.9980761f } },
    { 37, -8.05233f, -0.49729863f, -0.7631136f, { -3.821859e-05f, 0.7087757f, -0.00021043376f, 0.7054339f } },
    { 38, -25.335016f, 3.6235454f, -12.089307f, { -6.945993e-05f, 0.00066358334f, 0.001071707f, 0.9999992f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};

/* chr_generic */
int chr_generic_init(void *self, void *a, void *b);
int chr_generic_event(void *self, int ev, void *a, void *b);
int chr_generic_update(void *self, void *a, void *b, void *c, void *d);
int chr_generic_bone_id(void *self, int i);
int chr_generic_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr_generic_motion(void *self, void *a, void *b, void *c, int d);

void chr_generic_setup(ChrVt *vt)
{
    vt->init = chr_generic_init;
    vt->event = chr_generic_event;
    vt->update = chr_generic_update;
    vt->bone_id = chr_generic_bone_id;
    vt->bone_rest = chr_generic_bone_rest;
    vt->motion = chr_generic_motion;
}

int chr_generic_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr_generic_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr_generic_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr_generic_bone_id(void *self, int i)
{
    DdCharPos pos = { 0 };

    charmap_lookup2(charmap_lookup(((ChrModel *)self)->chr_id, 0), &pos);
    switch (pos.sub) {
    case 0:
        switch (pos.kind) {
        case 0:
            switch (i) {
                case 14: return 15;
                case 7: return 51;
                case 4: return 40;
                case 2: return 57;
                case 1: return 43;
                case 20: return 28;
                case 12: return 56;
                case 19: return 3;
                case 26: return 7;
                case 11: return 55;
                case 6: return 50;
                case 16: return 2;
                case 31: return 32;
                case 8: return 52;
                case 5: return 49;
                case 15: return 5;
                case 32: return 33;
                case 30: return 61;
                case 13: return 0;
                case 29: return 48;
                case 25: return 20;
                case 3: return 58;
                case 22: return 18;
                case 23: return 30;
                case 18: return 6;
                case 10: return 54;
                case 21: return 25;
                case 24: return 26;
                case 9: return 53;
                case 17: return 16;
                case 0: return 44;
            }
            break;
        case 1:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        case 2:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        case 3:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        case 4:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        }
        break;
    case 1:
        switch (pos.kind) {
        case 0:
            switch (i) {
                case 14: return 15;
                case 7: return 51;
                case 4: return 40;
                case 2: return 57;
                case 1: return 43;
                case 20: return 28;
                case 12: return 56;
                case 19: return 3;
                case 26: return 7;
                case 11: return 55;
                case 6: return 50;
                case 16: return 2;
                case 31: return 32;
                case 8: return 52;
                case 5: return 49;
                case 15: return 5;
                case 32: return 33;
                case 30: return 61;
                case 13: return 0;
                case 29: return 48;
                case 25: return 20;
                case 3: return 58;
                case 22: return 18;
                case 23: return 30;
                case 18: return 6;
                case 10: return 54;
                case 21: return 25;
                case 24: return 26;
                case 9: return 53;
                case 17: return 16;
                case 0: return 44;
            }
            break;
        case 1:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        case 2:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        case 3:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        case 4:
            switch (i) {
                case 14: return 15;
                case 7: return 50;
                case 4: return 39;
                case 2: return 56;
                case 1: return 42;
                case 20: return 27;
                case 12: return 55;
                case 19: return 3;
                case 26: return 7;
                case 11: return 54;
                case 6: return 49;
                case 16: return 2;
                case 31: return 31;
                case 8: return 51;
                case 5: return 48;
                case 15: return 5;
                case 32: return 32;
                case 30: return 60;
                case 13: return 0;
                case 29: return 47;
                case 25: return 20;
                case 3: return 57;
                case 22: return 18;
                case 23: return 29;
                case 18: return 6;
                case 10: return 53;
                case 21: return 24;
                case 24: return 25;
                case 9: return 52;
                case 17: return 16;
                case 0: return 43;
            }
            break;
        }
        break;
    case 2:
        switch (pos.kind) {
        case 0:
            switch (i) {
                case 14: return 29;
                case 7: return 64;
                case 4: return 53;
                case 2: return 70;
                case 1: return 56;
                case 20: return 41;
                case 12: return 69;
                case 19: return 3;
                case 26: return 19;
                case 11: return 68;
                case 6: return 63;
                case 16: return 2;
                case 31: return 45;
                case 8: return 65;
                case 5: return 62;
                case 15: return 11;
                case 32: return 46;
                case 30: return 74;
                case 13: return 0;
                case 29: return 61;
                case 25: return 34;
                case 3: return 71;
                case 22: return 32;
                case 23: return 43;
                case 18: return 12;
                case 10: return 67;
                case 21: return 38;
                case 24: return 39;
                case 9: return 66;
                case 17: return 30;
                case 0: return 57;
            }
            break;
        case 1:
            switch (i) {
                case 14: return 29;
                case 7: return 64;
                case 4: return 53;
                case 2: return 70;
                case 1: return 56;
                case 20: return 41;
                case 12: return 69;
                case 19: return 3;
                case 26: return 19;
                case 11: return 68;
                case 6: return 63;
                case 16: return 2;
                case 31: return 45;
                case 8: return 65;
                case 5: return 62;
                case 15: return 11;
                case 32: return 46;
                case 30: return 74;
                case 13: return 0;
                case 29: return 61;
                case 25: return 34;
                case 3: return 71;
                case 22: return 32;
                case 23: return 43;
                case 18: return 12;
                case 10: return 67;
                case 21: return 38;
                case 24: return 39;
                case 9: return 66;
                case 17: return 30;
                case 0: return 57;
            }
            break;
        case 2:
            switch (i) {
                case 14: return 29;
                case 7: return 64;
                case 4: return 53;
                case 2: return 70;
                case 1: return 56;
                case 20: return 41;
                case 12: return 69;
                case 19: return 3;
                case 26: return 19;
                case 11: return 68;
                case 6: return 63;
                case 16: return 2;
                case 31: return 45;
                case 8: return 65;
                case 5: return 62;
                case 15: return 11;
                case 32: return 46;
                case 30: return 74;
                case 13: return 0;
                case 29: return 61;
                case 25: return 34;
                case 3: return 71;
                case 22: return 32;
                case 23: return 43;
                case 18: return 12;
                case 10: return 67;
                case 21: return 38;
                case 24: return 39;
                case 9: return 66;
                case 17: return 30;
                case 0: return 57;
            }
            break;
        case 3:
            switch (i) {
                case 14: return 29;
                case 7: return 64;
                case 4: return 53;
                case 2: return 70;
                case 1: return 56;
                case 20: return 41;
                case 12: return 69;
                case 19: return 3;
                case 26: return 19;
                case 11: return 68;
                case 6: return 63;
                case 16: return 2;
                case 31: return 45;
                case 8: return 65;
                case 5: return 62;
                case 15: return 11;
                case 32: return 46;
                case 30: return 74;
                case 13: return 0;
                case 29: return 61;
                case 25: return 34;
                case 3: return 71;
                case 22: return 32;
                case 23: return 43;
                case 18: return 12;
                case 10: return 67;
                case 21: return 38;
                case 24: return 39;
                case 9: return 66;
                case 17: return 30;
                case 0: return 57;
            }
            break;
        case 4:
            switch (i) {
                case 14: return 29;
                case 7: return 64;
                case 4: return 53;
                case 2: return 70;
                case 1: return 56;
                case 20: return 41;
                case 12: return 69;
                case 19: return 3;
                case 26: return 19;
                case 11: return 68;
                case 6: return 63;
                case 16: return 2;
                case 31: return 45;
                case 8: return 65;
                case 5: return 62;
                case 15: return 11;
                case 32: return 46;
                case 30: return 74;
                case 13: return 0;
                case 29: return 61;
                case 25: return 34;
                case 3: return 71;
                case 22: return 32;
                case 23: return 43;
                case 18: return 12;
                case 10: return 67;
                case 21: return 38;
                case 24: return 39;
                case 9: return 66;
                case 17: return 30;
                case 0: return 57;
            }
            break;
        }
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr_generic_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B440 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr_generic_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr_generic_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr_generic_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 38 */
int chr38_init(void *self, void *a, void *b);
int chr38_event(void *self, int ev, void *a, void *b);
int chr38_update(void *self, void *a, void *b, void *c, void *d);
int chr38_bone_id(void *self, int i);
int chr38_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr38_motion(void *self, void *a, void *b, void *c, int d);

void chr38_setup(ChrVt *vt)
{
    vt->init = chr38_init;
    vt->event = chr38_event;
    vt->update = chr38_update;
    vt->bone_id = chr38_bone_id;
    vt->bone_rest = chr38_bone_rest;
    vt->motion = chr38_motion;
}

int chr38_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr38_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr38_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr38_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 22;
    case 7: return 68;
    case 4: return 42;
    case 2: return 75;
    case 1: return 64;
    case 20: return 57;
    case 12: return 73;
    case 19: return 3;
    case 26: return 10;
    case 11: return 72;
    case 6: return 67;
    case 16: return 2;
    case 31: return 33;
    case 8: return 69;
    case 5: return 66;
    case 15: return 8;
    case 32: return 34;
    case 30: return 79;
    case 13: return 0;
    case 29: return 74;
    case 25: return 28;
    case 3: return 76;
    case 22: return 25;
    case 23: return 60;
    case 18: return 9;
    case 10: return 71;
    case 21: return 46;
    case 24: return 48;
    case 9: return 70;
    case 17: return 23;
    case 0: return 65;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr38_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B444 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr38_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr38_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr38_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 19 */
int chr19_init(void *self, void *a, void *b);
int chr19_event(void *self, int ev, void *a, void *b);
int chr19_update(void *self, void *a, void *b, void *c, void *d);
int chr19_bone_id(void *self, int i);
int chr19_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr19_motion(void *self, void *a, void *b, void *c, int d);

void chr19_setup(ChrVt *vt)
{
    vt->init = chr19_init;
    vt->event = chr19_event;
    vt->update = chr19_update;
    vt->bone_id = chr19_bone_id;
    vt->bone_rest = chr19_bone_rest;
    vt->motion = chr19_motion;
}

int chr19_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr19_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr19_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr19_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 39;
    case 4: return 45;
    case 2: return 52;
    case 1: return 50;
    case 20: return 49;
    case 12: return 44;
    case 19: return 7;
    case 26: return 13;
    case 11: return 43;
    case 6: return 38;
    case 16: return 2;
    case 31: return 16;
    case 8: return 40;
    case 5: return 37;
    case 15: return 3;
    case 32: return 17;
    case 30: return 55;
    case 13: return 0;
    case 29: return 51;
    case 25: return 61;
    case 3: return 53;
    case 22: return 47;
    case 23: return 63;
    case 18: return 8;
    case 10: return 42;
    case 21: return 48;
    case 24: return 62;
    case 9: return 41;
    case 17: return 10;
    case 0: return 64;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr19_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B448 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr19_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr19_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr19_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}
