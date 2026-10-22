// @range 0x08860A88-0x0886191C
/* The computer player: the life point damage it expects from a card. */
#include "duel.h"

int card_get_type(int id);
int duel_find_effect_card(unsigned int p, int id, int skip);
int duel_list_counter(int uid, int id);

int duel_08860A88(DuelAct *a)
{
    unsigned int q = a->player ^ a->opp;
    int r = 0;

    if (card_get_type(a->id) == 0x16 && a->b3_4 == 0 && duel_find_effect_card(q, 0x17A3, -1) != 0) {
        return 0;
    }
    if (a->kind == 0x23) {
        return 0;
    }
    switch (a->id) {
    case 0x14BE:
        r = 100;
        break;
    case 0x11CF:
    case 0x12F3:
    case 0x14C2:
    case 0x16A4:
    case 0x1741:
    case 0x1775:
    case 0x17C9:
    case 0x184D:
    case 0x18CC:
    case 0x1977:
    case 0x19E1:
    case 0x19E2:
        r = 500;
        break;
    case 0x1295:
    case 0x12C3:
    case 0x1366:
    case 0x1517:
    case 0x1684:
    case 0x17A7:
    case 0x1880:
    case 0x1881:
        r = 800;
        break;
    case 0x1138:
    case 0x12BE:
    case 0x12FF:
    case 0x1324:
    case 0x1325:
    case 0x1393:
    case 0x14B6:
    case 0x14C7:
    case 0x14F0:
    case 0x159F:
    case 0x15B5:
    case 0x15FF:
    case 0x1619:
    case 0x166C:
    case 0x16E4:
    case 0x1766:
    case 0x17F4:
    case 0x18EC:
    case 0x1922:
    case 0x1932:
    case 0x1975:
        r = 1000;
        break;
    case 0x14AB:
    case 0x15E6:
    case 0x162B:
        r = 1500;
        break;
    case 0x1190:
    case 0x13A7:
    case 0x156A:
    case 0x169C:
    case 0x1712:
    case 0x19D5:
        r = 2000;
        break;
    case 0x11A0:
        r = 3000;
        break;
    case 0x114A:
        r = 5000;
        break;
    case 0x12FD:
    case 0x12DE:
    case 0x1679:
    case 0x17BC:
    case 0x17BE:
        r = duel.players[q & 1].lp >> 1;
        break;
    case 0x1628:
    case 0x143E:
        if (a->b3_4 != 0) {
            r = 100;
        }
        break;
    case 0x1599:
    case 0x16D5:
    case 0x18D2:
        if (a->b3_4 != 0) {
            r = 300;
        }
        break;
    case 0x10DF:
    case 0x10E7:
    case 0x1367:
    case 0x169D:
        if (a->b3_4 != 0) {
            r = 500;
        }
        break;
    case 0x175D:
        if (a->b3_4 != 0) {
            r = 700;
        }
        break;
    case 0x13C7:
        if (a->kind == 0x16) {
            r = 1000;
        }
        break;
    case 0x188F:
        if (a->kind == 0x16) {
            r = 500;
        }
        break;
    case 0x1445:
        if (a->b3_4 == 0 && duel_list_counter(a->uid, a->id) < 3) {
            r = duel.players[q & 1].lp >> 1;
        }
        break;
    }
    return r;
}

int duel_08860FDC(DuelAct *a)
{
    switch (a->id) {
    case 0x12F8:
    case 0x12FE:
    case 0x13F5:
    case 0x1461:
    case 0x1470:
    case 0x14DC:
    case 0x14DE:
    case 0x15A6:
    case 0x15A8:
    case 0x15AD:
    case 0x15B1:
    case 0x15B4:
    case 0x15C7:
    case 0x15E9:
    case 0x15F9:
    case 0x15FA:
    case 0x15FC:
    case 0x161C:
    case 0x179E:
    case 0x1805:
    case 0x1844:
    case 0x1845:
    case 0x1848:
    case 0x1849:
    case 0x1851:
    case 0x187A:
    case 0x1887:
    case 0x188E:
    case 0x18C9:
    case 0x190E:
    case 0x192D:
    case 0x1953:
    case 0x19AE:
    case 0x19B6:
    case 0x19DB:
    case 0x19F3:
    case 0x1A09:
    case 0x1A42:
    case 0x1A6E:
    case 0x1A70:
    case 0x1A9B:
        return 1;
    case 0x14A7:
    case 0x183E:
    case 0x1446:
        return 1;
    case 0x1326:
    case 0x1638:
    case 0x16A6:
        return 2;
    case 0x1321:
        return 5;
    case 0xFE0:
    case 0x114F:
    case 0x1421:
    case 0x147F:
    case 0x14D7:
    case 0x1594:
    case 0x15FD:
    case 0x16B2:
    case 0x16DD:
    case 0x1727:
    case 0x1754:
    case 0x1758:
    case 0x1764:
    case 0x1769:
    case 0x179C:
    case 0x1895:
    case 0x18C6:
    case 0x18CB:
    case 0x1900:
    case 0x191E:
    case 0x1936:
    case 0x1A60:
        return 1;
    }
    return 0;
}

int duel_08861360(DuelAct *a)
{
    switch (a->id) {
    case 0x15F1:
    case 0x165F:
    case 0x1661:
    case 0x198C:
        return 1;
    case 0x14EA:
    case 0x1907:
    case 0x19AF:
        return 2;
    }
    return 0;
}

int duel_088613D8(DuelAct *a)
{
    switch (a->id) {
    case 0x128E:
    case 0x1617:
        return 1;
    case 0x1615:
    case 0x1631:
    case 0x1634:
        return 2;
    case 0x1610:
    case 0x1611:
    case 0x161A:
    case 0x1695:
        return 3;
    case 0x162D:
        return 10;
    }
    return 0;
}

int duel_08861480(int id, int n)
{
    switch (id) {
    case 0x10F8:
        switch (n) {
        case 0:
            return 0x196;
        case 1:
            return 0x197;
        }
        break;
    case 0x1298:
        switch (n) {
        case 0:
            return 0x1C6;
        case 1:
            return 0x1C7;
        }
        break;
    case 0x12C6:
    case 0x153D:
    case 0x175A:
        switch (n) {
        case 0:
            return 0x198;
        case 1:
            return 0x199;
        }
        break;
    case 0x1388:
        switch (n) {
        case 0:
            return 0x1AD;
        case 1:
            return 0x1AE;
        }
        break;
    case 0x14E0:
        switch (n) {
        case 0:
            return 0x1A4;
        case 1:
            return 0x1AD;
        }
        break;
    case 0x14FD:
        switch (n) {
        case 0:
            return 0x1B6;
        case 1:
            return 0x1B7;
        }
        break;
    case 0x1519:
    case 0x1522:
    case 0x153F:
    case 0x15A5:
    case 0x1746:
        switch (n) {
        case 0:
            return 0x1B8;
        case 1:
            return 0x1B9;
        case 2:
            return 0x1BA;
        }
        break;
    case 0x1532:
        switch (n) {
        case 0:
            return 0x1A6;
        case 1:
            return 0x1A7;
        }
        break;
    case 0x1544:
        switch (n) {
        case 0:
            return 0x1AA;
        case 1:
            return 0x1AB;
        }
        break;
    case 0x1581:
        switch (n) {
        case 0:
            return 0x1BE;
        case 1:
            return 0x1BF;
        }
        break;
    case 0x1599:
        switch (n) {
        case 0:
            return 0x1A8;
        case 1:
            return 0x1A9;
        }
        break;
    case 0x15CF:
        switch (n) {
        case 0:
            return 0x1C5;
        case 1:
            return 0x1CD;
        }
        break;
    case 0x15D3:
    case 0x1916:
        switch (n) {
        case 0:
            return 0x1C5;
        case 1:
            return 0x1CC;
        }
        break;
    case 0x15E7:
        switch (n) {
        case 0:
            return 0x196;
        case 1:
            return 0x1A4;
        }
        break;
    case 0x15F1:
        switch (n) {
        case 0:
            return 0x1C2;
        case 1:
            return 0x1C3;
        }
        break;
    case 0x15F7:
        switch (n) {
        case 0:
            return 0x1C4;
        case 1:
            return 0x1C5;
        }
        break;
    case 0x161E:
        switch (n) {
        case 0:
            return 0x1AF;
        case 1:
            return 0x1A7;
        }
        break;
    case 0x1656:
        switch (n) {
        case 0:
            return 0x1B0;
        case 1:
            return 0x1B1;
        case 3:
            return 0x1B6;
        case 4:
            return 0x1B7;
        }
        break;
    case 0x1679:
        switch (n) {
        case 0:
            return 0x1C8;
        case 1:
            return 0x1C9;
        }
        break;
    case 0x1685:
        switch (n) {
        case 0:
            return 0x1B2;
        case 1:
            return 0x1B3;
        }
        break;
    case 0x1686:
        switch (n) {
        case 0:
            return 0x1B4;
        case 1:
            return 0x1B5;
        }
        break;
    case 0x1776:
        switch (n) {
        case 0:
            return 0x1C0;
        case 1:
            return 0x1C1;
        }
        break;
    case 0x1841:
        switch (n) {
        case 0:
            return 0x1C0;
        case 1:
            return 0x1A6;
        }
        break;
    case 0x184E:
        switch (n) {
        case 0:
            return 0x1CA;
        case 1:
            return 0x1CB;
        case 2:
            return 0x1A5;
        case 3:
            return 0x1C1;
        }
        break;
    case 0x1885:
        switch (n) {
        case 0:
            return 0x1D1;
        case 1:
            return 0x1D2;
        }
        break;
    case 0x19D3:
        switch (n) {
        case 0:
            return 0x1CE;
        case 1:
            return 0x1CF;
        case 2:
            return 0x1D0;
        }
        break;
    case 0x1A1C:
    case 0x1A32:
        switch (n) {
        case 0:
            return 0x1D3;
        case 1:
            return 0x1D4;
        case 2:
            return 0x1D5;
        }
        break;
    }
    return 0x1AC;
}
