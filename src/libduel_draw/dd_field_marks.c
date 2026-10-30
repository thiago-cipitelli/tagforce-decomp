// @range 0x08822B0C-0x0882308C
// @asm 0x08822B90-0x08822C2C
// @asm 0x08822C2C-0x08822E30
// @asm 0x08822E30-0x08823068
/* Marks over the field zones (dd_fx_mark: selectable zones, unusable zones), refreshed from the cursor
   and from the duel's slot markers. */
#include "dd_field_ui.h"

typedef struct DdMarks {               /* 0x08C583B8, 0x180 */
    int task[2][16];                   /* +0x000 dd_fx_mark task, -1 none */
    int kind[2][16];                   /* +0x080 marker kind of the duel (7: zone unusable) */
    int layer[2][16];                  /* +0x100 2D layer, 0: 3D */
} DdMarks;

extern DdMarks dd_marks;

int dd_task_pause(int h);
DdCobj *dd_cobj_at(int player, int zone, int index);
void dd_slot_xform(float *pos, float *rot, DdLoc at);
float dd_stack_y(int n);
int dv_n_d(int side);
int dv_n_18(int side);
unsigned int dv_n_grave(int side);
unsigned int dv_n_f(int side);
int dv_zone_usable(int side, unsigned int z);
void dv_slot_markers(int side, int z, unsigned short (*out)[11]);
void func_08820A8C(float *out, float *rot, int back);
int dd_fx_mark_start(float s, int kind, float *pos, float *rot, int dur, void *term, void *arg);
int dd_fx_mark_start_layer(float s, int layer, int kind, float *pos, float *rot, int dur, void *term, void *arg);
int func_08822E30(int player, int zone, unsigned char kind, int layer);
void func_08823068(int h);

/* clears the marks */
void func_08822B0C(void)
{
    int *t = dd_marks.task[0];
    int p, i;

    memset(&dd_marks, 0, sizeof(DdMarks));
    for (p = 0; p < 2; p++) {
        for (i = 0; i < 16; i++)
            t[i] = -1;
        t += 16;
    }
}

#ifdef NON_MATCHING /* laço interno com teste no topo (slti at/beqzl) não reproduzido */
/* removes every mark */
void func_08822B90(void)
{
    int *t = dd_marks.task[0];
    int p, i;

    for (p = 0; p < 2; p++) {
        for (i = 0; i < 16; i++) {
            if (t[i] != -1) {
                dd_task_pause(t[i]);
                t[i] = -1;
                t[i + 32] = 0;
            }
        }
        t += 16;
    }
}
#endif

#ifdef NON_MATCHING /* laço interno com teste no topo (slti at/beqzl) não reproduzido */
/* per frame: one mark per zone that the duel marks (or that can't be used), except under the cursor */
void func_08822C2C(void)
{
    int *t = dd_marks.task[0];
    unsigned short mk[2][11];
    DdLoc cur;
    unsigned short *row;
    int p, z;
    unsigned char kind;
    int layer;
    DdCobj *o;

    dd_cursor_get(&cur);
    dv_slot_markers(cur.player, cur.zone, mk);
    row = mk[0];
    for (p = 0; p < 2; p++) {
        for (z = 0; z < 11; z++) {
            kind = row[z];
            layer = 0;
            if (kind != 0 && (o = dd_cobj_at(p, z, 0)) != 0 && o->t < o->dur)
                kind = 0;           /* the card is still moving */
            if (kind == 0 && dv_zone_usable(p, z) == 0) {
                kind = 7;
                layer = 3;
            }
            if (kind == 0 || (z == cur.zone && p == cur.player && kind != 7)) {
                if (t[z] != -1) {
                    func_08823068(t[z]);
                    t[z] = -1;
                    t[z + 32] = 0;
                    t[z + 64] = 0;
                }
            } else {
                if (t[z + 32] != kind || t[z + 64] != layer) {
                    func_08823068(t[z]);
                    t[z] = -1;
                    t[z + 32] = 0;
                    t[z + 64] = 0;
                }
                if (t[z] == -1) {
                    t[z] = func_08822E30(p, z, kind, layer);
                    t[z + 32] = kind;
                    t[z + 64] = layer;
                }
            }
        }
        row += 11;
        t += 16;
    }
}
#endif

#ifdef NON_MATCHING /* DdLoc local montado com lhu/sh no original (o C gera lbu/sb) */
/* starts the mark of a zone; the piles are marked over their top card */
int func_08822E30(int player, int zone, unsigned char kind, int layer)
{
    float rot2[4];
    float rot[4];
    float pos[4];
    DdLoc at = { 0 };
    int fx;

    at.player = player;
    at.zone = zone;
    switch (kind) {
    default: fx = 0; break;
    case 2: fx = 1; break;
    case 3: fx = 2; break;
    case 4: fx = 3; break;
    case 5: fx = 4; break;
    case 6: fx = 5; break;
    case 7: fx = 6; break;
    }
    dd_slot_xform(pos, rot, at);
    pos[1] += 0.01f;
    if (at.zone == 12)
        pos[1] += dd_stack_y(dv_n_18(at.player));
    else if (at.zone == 15)
        pos[1] += dd_stack_y(dv_n_f(at.player));
    else if (at.zone == 13)
        pos[1] += dd_stack_y(dv_n_d(at.player));
    else if (at.zone == 14)
        pos[1] += dd_stack_y(dv_n_grave(at.player));
    func_08820A8C(rot2, rot, pos[2] < 0.0f);
    if (layer == 0)
        return dd_fx_mark_start(1.0f, fx, pos, rot2, -1, 0, 0);
    return dd_fx_mark_start_layer(1.0f, layer, fx, pos, rot2, -1, 0, 0);
}
#endif

/* stops a mark */
void func_08823068(int h)
{
    if (h != -1)
        dd_task_pause(h);
}
