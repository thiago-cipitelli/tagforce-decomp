// @range 0x088A6464-0x088A67D8
/* Card effect helpers on the duel_chain + 0x698 list (after effects_088A49F8.c). */
#include "duel.h"

extern int duel_w_089149E8;              /* duel_chain + 0x698 */
extern int duel_w_089149F0;              /* duel_chain + 0x6A0 */
extern int duel_w_089149F4;              /* duel_chain + 0x6A4 */
extern int duel_w_089149FC;              /* duel_chain + 0x6AC */

int duel_088A6264(int k, int id);
int duel_088A62AC(int k, unsigned int p, int z);
int card_is_in_group_1872(int id);
int field_get_card(unsigned int p, int z);

#define CHAIN_LIST ((int *)((char *)&duel_chain + 0x6B0))
#define CHAIN_W(i) (((int *)&duel_chain)[0x6B0 / 4 + (i)])

int duel_088A6464(int id, int f)
{
    int n = duel_w_089149F0;
    int last = -1;
    int i;
    int cnt = 0;
    int cnt2 = 0;
    int k;

    if (duel_w_089149E8 == 0x1A4E && f == 2) {
        n = 2;
        CHAIN_LIST[1] = 7;
    }
    for (i = 0; i < n; i++) {
        k = CHAIN_W(i);
        if (duel_088A6264(k, id)) {
            last = i;
            cnt++;
        } else if (k > 0x14 && duel_w_089149FC == 0 && card_is_in_group_1872(id)) {
            last = i;
            cnt++;
            cnt2++;
        }
    }
    if (cnt != 0) {
        if (f != 0) {
            if (cnt > 1) {
                duel_w_089149F4++;
                if (cnt == cnt2) {
                    duel_w_089149FC = 1;
                }
            } else {
                CHAIN_LIST[last] = 0;
                if (cnt2 != 0) {
                    duel_w_089149FC = 1;
                }
            }
        }
        return 1;
    }
    return 0;
}

int duel_088A65EC(unsigned int p, int z, int f)
{
    int n = duel_w_089149F0;
    int last = -1;
    int i;
    int cnt = 0;
    int cnt2 = 0;
    int k;

    if (duel_w_089149E8 == 0x1A4E && f == 2) {
        n = 2;
        CHAIN_LIST[1] = 7;
    }
    for (i = 0; i < n; i++) {
        k = CHAIN_W(i);
        if (duel_088A62AC(k, p, z)) {
            last = i;
            cnt++;
        } else if (k > 0x14 && duel_w_089149FC == 0 && card_is_in_group_1872(field_get_card(p, z)) &&
                   !(duel.players[p & 1].field[z].flags >> 5 & 1)) {
            last = i;
            cnt++;
            cnt2++;
        }
    }
    if (cnt != 0) {
        if (f != 0) {
            if (cnt > 1) {
                duel_w_089149F4++;
                if (cnt == cnt2) {
                    duel_w_089149FC = 1;
                }
            } else {
                CHAIN_LIST[last] = 0;
                if (cnt2 != 0) {
                    duel_w_089149FC = 1;
                }
            }
        }
        return 1;
    }
    return 0;
}
