// @range 0x0880D4F4-0x0880F228
// @asm 0x0880D4F4-0x0880ED50
// @rodata 0x08838384-0x0883860C
/* Index tables: chara id -> SD model (asm), bust-up, room, room group and voice file indices. */
#include "libfield.h"

int fld_bu_index(int x)
{
    int r = 0;

    switch (x) {
    case 1:
        r = 0;
        break;
    case 2:
        r = 1;
        break;
    case 3:
        r = 2;
        break;
    case 4:
        r = 3;
        break;
    case 5:
        r = 4;
        break;
    case 6:
        r = 5;
        break;
    case 7:
        r = 6;
        break;
    case 8:
        r = 7;
        break;
    case 9:
        r = 8;
        break;
    case 10:
        r = 9;
        break;
    case 11:
        r = 10;
        break;
    case 12:
        r = 11;
        break;
    case 13:
        r = 12;
        break;
    case 14:
        r = 13;
        break;
    case 15:
        r = 14;
        break;
    case 16:
        r = 15;
        break;
    case 17:
        r = 16;
        break;
    case 18:
        r = 17;
        break;
    case 19:
        r = 18;
        break;
    case 20:
        r = 19;
        break;
    case 21:
        r = 20;
        break;
    case 22:
        r = 21;
        break;
    case 23:
        r = 22;
        break;
    case 24:
        r = 23;
        break;
    case 25:
        r = 24;
        break;
    case 26:
        r = 25;
        break;
    case 27:
        r = 26;
        break;
    case 28:
        r = 27;
        break;
    case 30:
        r = 28;
        break;
    case 31:
        r = 29;
        break;
    case 32:
        r = 30;
        break;
    case 33:
        r = 31;
        break;
    case 34:
        r = 32;
        break;
    case 35:
        r = 33;
        break;
    case 36:
        r = 34;
        break;
    case 37:
        r = 35;
        break;
    case 38:
        r = 36;
        break;
    case 39:
        r = 37;
        break;
    case 40:
        r = 38;
        break;
    case 41:
        r = 39;
        break;
    case 42:
        r = 40;
        break;
    case 43:
        r = 41;
        break;
    case 44:
        r = 42;
        break;
    case 45:
        r = 43;
        break;
    case 46:
        r = 44;
        break;
    case 47:
        r = 45;
        break;
    case 48:
        r = 46;
        break;
    case 49:
        r = 47;
        break;
    case 50:
        r = 48;
        break;
    case 55:
        r = 49;
        break;
    case 56:
        r = 50;
        break;
    case 57:
        r = 51;
        break;
    case 58:
        r = 52;
        break;
    case 59:
        r = 53;
        break;
    }
    return r;
}

int fld_bg_index(int x)
{
    int r = 0;

    switch (x) {
    case 1:
        r = 0;
        break;
    case 2:
        r = 1;
        break;
    case 3:
        r = 2;
        break;
    case 4:
        r = 3;
        break;
    case 5:
        r = 4;
        break;
    case 12:
        r = 5;
        break;
    case 13:
        r = 6;
        break;
    case 14:
        r = 7;
        break;
    case 15:
        r = 8;
        break;
    case 16:
        r = 9;
        break;
    case 17:
        r = 10;
        break;
    case 18:
        r = 11;
        break;
    case 19:
        r = 12;
        break;
    case 20:
        r = 13;
        break;
    case 21:
        r = 14;
        break;
    case 22:
        r = 15;
        break;
    case 23:
        r = 16;
        break;
    case 24:
        r = 17;
        break;
    case 25:
        r = 18;
        break;
    case 26:
        r = 19;
        break;
    case 27:
        r = 20;
        break;
    case 29:
        r = 21;
        break;
    case 30:
        r = 22;
        break;
    case 31:
        r = 23;
        break;
    case 32:
        r = 24;
        break;
    case 36:
        r = 25;
        break;
    case 37:
        r = 26;
        break;
    case 38:
        r = 27;
        break;
    case 39:
        r = 28;
        break;
    }
    return r;
}

int fld_bg_group(int x)
{
    int r = 0;

    switch (x) {
    case 1:
        r = 15;
        break;
    case 2:
        r = 16;
        break;
    case 3:
        r = 17;
        break;
    case 4:
        r = 18;
        break;
    case 5:
        r = 19;
        break;
    case 12:
        r = 1;
        break;
    case 13:
        r = 2;
        break;
    case 14:
        r = 3;
        break;
    case 15:
        r = 4;
        break;
    case 16:
        r = 5;
        break;
    case 17:
        r = 6;
        break;
    case 18:
        r = 7;
        break;
    case 19:
        r = 8;
        break;
    case 20:
        r = 9;
        break;
    case 21:
        r = 10;
        break;
    case 22:
        r = 11;
        break;
    case 23:
        r = 12;
        break;
    case 24:
        r = 13;
        break;
    case 25:
        r = 23;
        break;
    case 26:
        r = 22;
        break;
    case 27:
        r = 24;
        break;
    case 29:
        r = 26;
        break;
    case 30:
        r = 27;
        break;
    case 31:
        r = 28;
        break;
    case 32:
        r = 29;
        break;
    case 36:
        r = 30;
        break;
    case 37:
        r = 1;
        break;
    case 38:
        r = 8;
        break;
    case 39:
        r = 30;
        break;
    }
    return r;
}

int fld_voice_index(int x)
{
    int r = 0;

    switch (x) {
    case 1:
        r = 0;
        break;
    case 2:
        r = 1;
        break;
    case 3:
        r = 2;
        break;
    case 4:
        r = 3;
        break;
    case 5:
        r = 4;
        break;
    case 6:
        r = 5;
        break;
    case 7:
        r = 6;
        break;
    case 8:
        r = 7;
        break;
    case 9:
        r = 8;
        break;
    case 10:
        r = 9;
        break;
    case 11:
        r = 10;
        break;
    case 12:
        r = 11;
        break;
    case 13:
        r = 12;
        break;
    case 14:
        r = 13;
        break;
    case 15:
        r = 14;
        break;
    case 16:
        r = 15;
        break;
    case 17:
        r = 16;
        break;
    case 18:
        r = 17;
        break;
    case 19:
        r = 18;
        break;
    case 20:
        r = 19;
        break;
    case 21:
        r = 20;
        break;
    }
    return r;
}
