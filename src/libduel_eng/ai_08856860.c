// @range 0x08856860-0x08856EE0
// @asm 0x08856860-0x08856EE0
/* Computer player: test by card id of a monster to tribute or send (duel_0884E690 callback). */
#include "duel.h"

#define LOC(p, z) (unsigned short)((unsigned char)(p) | (unsigned char)(z) << 8)

typedef int (*DuelZoneFn)(DuelAct *a, unsigned int p, int z);

int duel_tributable(unsigned int p);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_field_same_name(unsigned int p, int z, int id);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_bit_count(unsigned int m);
int card_is_attribute9(int id);
int card_is_in_group_1911(int id);
int duel_0884E690(DuelAct *a, DuelZoneFn fn);
int duel_0884E698(DuelAct *a, DuelZoneFn fn, unsigned int skip);
int duel_08855214(DuelAct *a, unsigned int p, int z);
int duel_08855DC8(DuelAct *a, unsigned int p, int z);
int duel_08855F54(DuelAct *a, unsigned int p, int z);
int duel_088580B8(DuelAct *a, unsigned int p, int z);
int duel_0885A15C(DuelAct *a, unsigned int p, int z);
int duel_0885F2E4(DuelAct *a, unsigned int p, int z);

#ifdef NON_MATCHING /* só o escalonamento do prólogo (lhu de a->kind) difere */
int duel_08856860(DuelAct *a, unsigned int p, int z)
{
    DuelStats st;

    if (a->kind == 0x23) {
        return 1;
    }
    if (duel_tributable(a->player) != 0 && (a->player == p || duel.players[p & 1].field[z].faceup != 0)) {
        duel_field_stats(p, z, &st);
        switch (a->id) {
        case 0x123B:
            return st.atk < 1001 && st.unk8 == 2;
        case 0x14C5:
            if (st.attr == 0xF) {
                return duel_0884E698(a, duel_08855214, LOC(p, z)) > 0;
            }
            break;
        case 0x14E4:
            if (st.attr == 1) {
                a->w[0] = st.atk;
                return duel_0884E698(a, duel_08855F54, LOC(p, z)) > 0;
            }
            break;
        case 0x14EE:
            return !card_is_attribute9(duel.players[p & 1].field[z].card.id);
        case 0x169B:
            if (card_is_in_group_1911(st.id) != 0) {
                return duel_0884E698(a, duel_0885A15C, LOC(p, z)) > 0;
            }
            break;
        case 0x1702:
            return st.attr == 0xA;
        case 0x1708:
            if (st.id == 0x1706) {
                return duel_0884E698(a, duel_08855DC8, LOC(p, z)) > 0;
            }
            if (st.id == 0x1709) {
                return duel_0884E690(a, duel_088580B8) > 0;
            }
            break;
        case 0x1753:
            if (st.attr == 0xD) {
                return duel_0884E698(a, duel_088580B8, LOC(p, z)) > 0;
            }
            break;
        case 0x1790:
            if (p != a->player) {
                return 0;
            }
            return duel.players[p & 1].field[z].faceup != 0 ? duel_field_same_name(p, z, a->id) : 0;
        case 0x1835:
            return a->uid == CARD_UID(duel.players[p & 1].field[z].card) ? 0 : st.attr == 4;
        case 0x1855:
            return st.level < 6;
        case 0x188C:
            return st.atk >= 2000 && st.unk8 == 2;
        case 0x188D:
            if (duel_bit_count(st.mask) > 1) {
                return 1;
            }
            switch (st.unk8) {
            case 6:
            case 3:
            case 4:
            case 5:
                return !(a->w[0] & 1 << st.unk8);
            }
            break;
        case 0x1928:
            return duel_field_attr_is(p, z, 3);
        case 0x1929:
            return duel_field_attr_is(p, z, 4);
        case 0x192A:
            return duel_field_attr_is(p, z, 6);
        case 0x1941:
            return a->uid == CARD_UID(duel.players[p & 1].field[z].card) ? 0 : st.attr == 0xA;
        case 0x195C:
            return a->w[2] < 2 ? duel_field_same_name(p, z, 0x194F) : duel_field_same_name(p, z, 0x1950);
        case 0x1A35:
            if (st.attr == 0x10) {
                return !(duel_0884E698(a, duel_0885F2E4, LOC(p, z)) < 2);
            }
            break;
        case 0x1AA3:
            return st.attr == 9;
        }
    }
    return 0;
}
#endif
