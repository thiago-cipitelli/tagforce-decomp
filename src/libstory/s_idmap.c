// @range 0x08810D24-0x08812728
// @data 0x0881C550-0x0881D15C
/* Remaps of the ids used by the story scripts to the indexes of the BOOT tables (characters, records,
   item and deck slots), one dense switch each; and the init/term of a 2-byte flag. */
#include "libstory.h"

int st_room_variant(void);

extern unsigned char st_flag_8ac9358[2];

void st_init_810d24(void)
{
    memset(st_flag_8ac9358, 0, 2);
}

void st_term_810d38(void)
{
    memset(st_flag_8ac9358, 0, 2);
}

int st_id_to_chr(int id)
{
    switch (id) {
    case 1: return 0;
    case 2: return 1;
    case 3: return 2;
    case 4: return 3;
    case 5: return 4;
    case 6: return 5;
    case 7: return 6;
    case 8: return 7;
    case 9: return 8;
    case 10: return 9;
    case 11: return 10;
    case 12: return 11;
    case 13: return 12;
    case 14: return 13;
    case 15: return 14;
    case 16: return 15;
    case 17: return 16;
    case 18: return 17;
    case 19: return 18;
    case 20: return 19;
    case 21: return 20;
    case 22: return 21;
    case 23: return 22;
    case 24: return 23;
    case 25: return 24;
    case 26: return 25;
    case 27: return 26;
    case 28: return 27;
    case 29: return 28;
    case 30: return 29;
    case 31: return 30;
    case 32: return 31;
    case 33: return 32;
    case 34: return 33;
    case 35: return 34;
    case 36: return 35;
    case 37: return 36;
    case 38: return 37;
    case 39: return 38;
    case 40: return 39;
    case 41: return 40;
    case 42: return 41;
    case 43: return 42;
    case 44: return 43;
    case 45: return 44;
    case 46: return 45;
    case 47: return 46;
    case 48: return 47;
    case 49: return 48;
    case 50: return 49;
    case 51: return 50;
    case 52: return 51;
    case 53: return 52;
    case 54: return 53;
    case 55: return 54;
    case 56: return 55;
    case 57: return 56;
    case 58: return 57;
    case 59: return 58;
    case 60: return 59;
    case 61: return 60;
    case 62: return 61;
    case 63: return 62;
    case 64: return 63;
    case 65: return 64;
    case 66: return 65;
    case 67: return 66;
    case 68: return 67;
    case 69: return 68;
    case 70: return 69;
    case 71: return 70;
    case 72: return 71;
    case 73: return 72;
    case 74: return 73;
    case 75: return 74;
    case 76: return 75;
    case 77: return 76;
    case 78: return 77;
    case 79: return 78;
    case 80: return 79;
    case 81: return 80;
    case 82: return 81;
    case 83: return 82;
    case 84: return 83;
    case 85: return 84;
    case 86: return 85;
    case 87: return 86;
    case 88: return 87;
    case 89: return 88;
    case 90: return 89;
    case 91: return 90;
    case 92: return 91;
    case 93: return 92;
    case 94: return 93;
    case 95: return 94;
    case 96: return 95;
    case 97: return 96;
    case 98: return 97;
    case 99: return 98;
    case 100: return 99;
    case 101: return 100;
    case 102: return 101;
    case 103: return 102;
    case 104: return 103;
    case 105: return 104;
    case 106: return 105;
    case 107: return 106;
    case 108: return 107;
    case 109: return 108;
    case 110: return 109;
    case 111: return 110;
    case 112: return 111;
    case 113: return 112;
    case 114: return 113;
    case 115: return 114;
    case 116: return 115;
    case 117: return 116;
    case 118: return 117;
    case 119: return 118;
    case 120: return 119;
    case 121: return 120;
    case 122: return 121;
    case 123: return 122;
    case 124: return 123;
    case 125: return 124;
    case 126: return 125;
    case 127: return 126;
    case 128: return 127;
    case 129: return 128;
    case 130: return 129;
    case 131: return 130;
    case 132: return 131;
    case 133: return 132;
    case 134: return 133;
    case 135: return 134;
    case 136: return 135;
    case 137: return 136;
    case 138: return 137;
    case 139: return 138;
    case 140: return 139;
    case 141: return 140;
    case 142: return 141;
    case 143: return 142;
    case 144: return 143;
    case 145: return 144;
    case 146: return 145;
    case 147: return 146;
    case 148: return 147;
    case 149: return 148;
    case 150: return 149;
    default: return 0;
    }
}

int st_id_to_rec(int id)
{
    switch (id) {
    case 1: return 2;
    case 2: return 3;
    case 3: return 4;
    case 4: return 5;
    case 5: return 6;
    case 6: return 7;
    case 7: return 8;
    case 8: return 9;
    case 9: return 10;
    case 10: return 11;
    case 11: return 12;
    case 12: return 13;
    case 13: return 14;
    case 14: return 15;
    case 15: return 16;
    case 16: return 17;
    case 17: return 18;
    case 18: return 19;
    case 19: return 20;
    case 20: return 21;
    case 21: return 22;
    case 22: return 23;
    case 23: return 24;
    case 24: return 25;
    case 25: return 26;
    case 26: return 27;
    case 27: return 28;
    case 28: return 29;
    case 29: return 30;
    case 30: return 31;
    case 31: return 32;
    case 32: return 33;
    case 33: return 34;
    case 34: return 35;
    case 35: return 36;
    case 36: return 37;
    case 37: return 38;
    case 38: return 39;
    case 39: return 40;
    case 40: return 41;
    case 41: return 42;
    case 42: return 43;
    case 43: return 44;
    case 44: return 45;
    case 45: return 46;
    case 46: return 47;
    case 47: return 48;
    case 48: return 49;
    case 49: return 50;
    case 50: return 51;
    case 51: return 52;
    case 52: return 53;
    case 53: return 54;
    case 54: return 55;
    case 55: return 56;
    case 56: return 57;
    case 57: return 58;
    case 58: return 59;
    case 59: return 60;
    case 60: return 61;
    case 61: return 62;
    case 62: return 63;
    case 63: return 64;
    case 64: return 65;
    case 65: return 66;
    case 66: return 67;
    case 67: return 68;
    case 68: return 69;
    case 69: return 70;
    case 70: return 71;
    case 71: return 72;
    case 72: return 73;
    case 73: return 74;
    case 74: return 75;
    case 75: return 76;
    case 76: return 77;
    case 77: return 78;
    case 78: return 79;
    case 79: return 80;
    case 80: return 81;
    case 81: return 82;
    case 82: return 83;
    case 83: return 84;
    case 84: return 85;
    case 85: return 86;
    case 86: return 87;
    case 87: return 88;
    case 88: return 89;
    case 89: return 90;
    case 90: return 91;
    case 91: return 92;
    case 92: return 93;
    case 93: return 94;
    case 94: return 95;
    case 95: return 96;
    case 96: return 97;
    case 97: return 98;
    case 98: return 99;
    case 99: return 100;
    case 100: return 101;
    case 101: return 102;
    case 102: return 103;
    case 103: return 104;
    case 104: return 105;
    case 105: return 106;
    case 106: return 107;
    case 107: return 108;
    case 108: return 109;
    case 109: return 110;
    case 110: return 111;
    case 111: return 112;
    case 112: return 113;
    case 113: return 114;
    case 114: return 115;
    case 115: return 116;
    case 116: return 117;
    case 117: return 118;
    case 118: return 119;
    case 119: return 120;
    case 120: return 121;
    case 121: return 122;
    case 122: return 123;
    case 123: return 124;
    case 124: return 125;
    case 125: return 126;
    case 126: return 127;
    case 127: return 128;
    case 128: return 129;
    case 129: return 130;
    case 130: return 131;
    case 131: return 132;
    case 132: return 133;
    case 133: return 134;
    case 134: return 135;
    case 135: return 136;
    case 136: return 137;
    case 137: return 138;
    case 138: return 139;
    case 139: return 140;
    case 140: return 141;
    case 141: return 142;
    case 142: return 143;
    case 143: return 144;
    case 144: return 145;
    case 145: return 146;
    case 146: return 147;
    case 147: return 148;
    case 148: return 149;
    case 149: return 150;
    case 150: return 151;
    default: return 0;
    }
}

int st_id_to_item(int id)
{
    switch (id) {
    case 0: return 2;
    case 1: return 3;
    case 2: return 4;
    case 3: return 5;
    case 4: return 6;
    case 5: return 7;
    case 6: return 8;
    case 7: return 9;
    case 8: return 10;
    case 9: return 11;
    case 10: return 12;
    case 11: return 13;
    case 12: return 14;
    case 13: return 15;
    case 14: return 16;
    case 15: return 17;
    case 16: return 18;
    case 17: return 19;
    case 18: return 20;
    case 19: return 21;
    case 20: return 22;
    case 21: return 23;
    case 22: return 24;
    case 23: return 25;
    case 24: return 26;
    case 25: return 27;
    case 26: return 28;
    case 27: return 29;
    case 28: return 30;
    case 29: return 31;
    case 30: return 32;
    case 31: return 33;
    case 32: return 34;
    case 33: return 35;
    case 34: return 36;
    case 35: return 37;
    case 36: return 38;
    case 37: return 39;
    case 38: return 40;
    case 39: return 41;
    case 40: return 42;
    case 41: return 43;
    case 42: return 44;
    case 43: return 45;
    case 44: return 46;
    case 45: return 47;
    case 46: return 48;
    case 47: return 49;
    case 48: return 50;
    case 49: return 51;
    case 50: return 52;
    case 51: return 53;
    case 52: return 54;
    case 53: return 55;
    case 54: return 56;
    case 55: return 57;
    case 56: return 58;
    case 57: return 59;
    case 58: return 60;
    case 59: return 61;
    case 60: return 62;
    case 61: return 63;
    case 62: return 64;
    case 63: return 65;
    case 64: return 66;
    case 65: return 67;
    case 66: return 68;
    case 67: return 69;
    case 68: return 70;
    case 69: return 71;
    case 70: return 72;
    case 71: return 73;
    case 72: return 74;
    case 73: return 75;
    case 74: return 76;
    case 75: return 77;
    case 76: return 78;
    case 77: return 79;
    case 78: return 80;
    case 79: return 81;
    case 80: return 82;
    case 81: return 83;
    case 82: return 84;
    case 83: return 85;
    case 84: return 86;
    case 85: return 87;
    case 86: return 88;
    case 87: return 89;
    case 88: return 90;
    case 89: return 91;
    case 90: return 92;
    case 91: return 93;
    case 92: return 94;
    case 93: return 95;
    case 94: return 96;
    case 95: return 97;
    case 96: return 98;
    case 97: return 99;
    case 98: return 100;
    case 99: return 101;
    case 100: return 102;
    case 101: return 103;
    case 102: return 104;
    case 103: return 105;
    case 104: return 106;
    case 105: return 107;
    case 106: return 108;
    case 107: return 109;
    case 108: return 110;
    case 109: return 111;
    case 110: return 112;
    case 111: return 113;
    case 112: return 114;
    case 113: return 115;
    case 114: return 116;
    case 115: return 117;
    case 116: return 118;
    case 117: return 119;
    case 118: return 120;
    case 119: return 121;
    case 120: return 122;
    case 121: return 123;
    case 122: return 124;
    case 123: return 125;
    case 124: return 126;
    case 125: return 127;
    case 126: return 128;
    case 127: return 129;
    case 128: return 130;
    case 129: return 131;
    case 130: return 132;
    case 131: return 133;
    case 132: return 134;
    case 133: return 135;
    case 134: return 136;
    case 135: return 137;
    case 136: return 138;
    case 137: return 139;
    case 138: return 140;
    case 139: return 141;
    case 140: return 142;
    case 141: return 143;
    case 142: return 144;
    case 143: return 145;
    case 144: return 146;
    case 145: return 147;
    case 146: return 148;
    case 147: return 149;
    case 148: return 150;
    case 149: return 151;
    default: return 0;
    }
}

int st_id_to_deck(int id)
{
    switch (id) {
    case 2: return 0;
    case 3: return 1;
    case 4: return 2;
    case 5: return 3;
    case 6: return 4;
    case 7: return 5;
    case 8: return 6;
    case 9: return 7;
    case 10: return 8;
    case 11: return 9;
    case 12: return 10;
    case 13: return 11;
    case 14: return 12;
    case 15: return 13;
    case 16: return 14;
    case 17: return 15;
    case 18: return 16;
    case 19: return 17;
    case 20: return 18;
    case 21: return 19;
    case 22: return 20;
    case 23: return 21;
    case 24: return 22;
    case 25: return 23;
    case 26: return 24;
    case 27: return 25;
    case 28: return 26;
    case 29: return 27;
    case 30: return 28;
    case 31: return 29;
    case 32: return 30;
    case 33: return 31;
    case 34: return 32;
    case 35: return 33;
    case 36: return 34;
    case 37: return 35;
    case 38: return 36;
    case 39: return 37;
    case 40: return 38;
    case 41: return 39;
    case 42: return 40;
    case 43: return 41;
    case 44: return 42;
    case 45: return 43;
    case 46: return 44;
    case 47: return 45;
    case 48: return 46;
    case 49: return 47;
    case 50: return 48;
    case 51: return 49;
    case 52: return 50;
    case 53: return 51;
    case 54: return 52;
    case 55: return 53;
    case 56: return 54;
    case 57: return 55;
    case 58: return 56;
    case 59: return 57;
    case 60: return 58;
    case 61: return 59;
    case 62: return 60;
    case 63: return 61;
    case 64: return 62;
    case 65: return 63;
    case 66: return 64;
    case 67: return 65;
    case 68: return 66;
    case 69: return 67;
    case 70: return 68;
    case 71: return 69;
    case 72: return 70;
    case 73: return 71;
    case 74: return 72;
    case 75: return 73;
    case 76: return 74;
    case 77: return 75;
    case 78: return 76;
    case 79: return 77;
    case 80: return 78;
    case 81: return 79;
    case 82: return 80;
    case 83: return 81;
    case 84: return 82;
    case 85: return 83;
    case 86: return 84;
    case 87: return 85;
    case 88: return 86;
    case 89: return 87;
    case 90: return 88;
    case 91: return 89;
    case 92: return 90;
    case 93: return 91;
    case 94: return 92;
    case 95: return 93;
    case 96: return 94;
    case 97: return 95;
    case 98: return 96;
    case 99: return 97;
    case 100: return 98;
    case 101: return 99;
    case 102: return 100;
    case 103: return 101;
    case 104: return 102;
    case 105: return 103;
    case 106: return 104;
    case 107: return 105;
    case 108: return 106;
    case 109: return 107;
    case 110: return 108;
    case 111: return 109;
    case 112: return 110;
    case 113: return 111;
    case 114: return 112;
    case 115: return 113;
    case 116: return 114;
    case 117: return 115;
    case 118: return 116;
    case 119: return 117;
    case 120: return 118;
    case 121: return 119;
    case 122: return 120;
    case 123: return 121;
    case 124: return 122;
    case 125: return 123;
    case 126: return 124;
    case 127: return 125;
    case 128: return 126;
    case 129: return 127;
    case 130: return 128;
    case 131: return 129;
    case 132: return 130;
    case 133: return 131;
    case 134: return 132;
    case 135: return 133;
    case 136: return 134;
    case 137: return 135;
    case 138: return 136;
    case 139: return 137;
    case 140: return 138;
    case 141: return 139;
    case 142: return 140;
    case 143: return 141;
    case 144: return 142;
    case 145: return 143;
    case 146: return 144;
    case 147: return 145;
    case 148: return 146;
    case 149: return 147;
    case 150: return 148;
    case 151: return 149;
    default: return 0;
    }
}

/* room of the field menu (field_bg_of_room order) of a bg id */
int st_bg_to_room(int id)
{
    switch (id) {
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 5;
    case 12:
        return 6;
    case 13:
        return 7;
    case 14:
        return 8;
    case 15:
        return 9;
    case 16:
        return 10;
    case 17:
        return 11;
    case 18:
        return 12;
    case 19:
        return 13;
    case 20:
        return 14;
    case 21:
        return 15;
    case 22:
        return 16;
    case 23:
        return 17;
    case 24:
        return 18;
    case 25:
        return 19;
    case 26:
        return 20;
    case 27:
        return 21;
    case 29:
        return 22;
    case 30:
        return 23;
    case 31:
        return 24;
    case 32:
        return 25;
    case 36:
        return 26;
    case 37:
        return 6;
    case 38:
        return 13;
    case 39:
        return 27;
    default:
        return 0;
    }
}

/* bg id of a room; rooms 6 and 13 have a second bg once st_room_variant() is set */
int st_room_to_bg(int room)
{
    switch (room) {
    default:
        return 0x1E;
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 5;
    case 6:
        if (st_room_variant() != 0)
            return 0x25;
        return 0xC;
    case 7:
        return 0xD;
    case 8:
        return 0xE;
    case 9:
        return 0xF;
    case 10:
        return 0x10;
    case 11:
        return 0x11;
    case 12:
        return 0x12;
    case 13:
        if (st_room_variant() != 0)
            return 0x26;
        return 0x13;
    case 14:
        return 0x14;
    case 15:
        return 0x15;
    case 16:
        return 0x16;
    case 17:
        return 0x17;
    case 18:
        return 0x18;
    case 19:
        return 0x19;
    case 20:
        return 0x1A;
    case 21:
        return 0x1B;
    case 22:
        return 0x1D;
    case 23:
        return 0x1E;
    case 24:
        return 0x1F;
    case 25:
        return 0x20;
    case 26:
        return 0x24;
    case 27:
        return 0x27;
    }
}

int st_remap_812334(int id)
{
    switch (id) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 3:
        return 2;
    case 4:
        return 3;
    case 5:
        return 4;
    case 6:
        return 5;
    case 7:
        return 6;
    case 8:
        return 7;
    case 9:
        return 8;
    case 10:
        return 9;
    case 11:
        return 10;
    case 12:
        return 11;
    case 13:
        return 12;
    case 14:
        return 13;
    case 15:
        return 14;
    case 16:
        return 15;
    case 17:
        return 16;
    case 18:
        return 17;
    case 19:
        return 18;
    default:
        return 0;
    }
}

int st_remap_812400(int id)
{
    switch (id) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 5;
    }
}

int chr_attr_remap(int a)
{
    switch (a) {
    case 8:
        return 1;
    case 9:
        return 3;
    case 10:
        return 5;
    case 11:
        return 7;
    }
    return 0;
}

int st_remap_812498(int id)
{
    switch (id) {
    case 1:
        return 0x6265;
    }
    return 0;
}

int st_remap_8124b0(int id)
{
    switch (id) {
    case 1:
        return 15;
    case 2:
        return 16;
    case 3:
        return 17;
    case 4:
        return 18;
    case 5:
        return 19;
    case 6:
        return 20;
    case 7:
        return 21;
    case 8:
        return 22;
    case 9:
        return 24;
    case 10:
        return 26;
    case 11:
        return 27;
    case 12:
        return 28;
    case 13:
        return 29;
    case 14:
        return 30;
    case 15:
        return 31;
    case 16:
        return 32;
    case 17:
        return 33;
    case 18:
        return 34;
    case 19:
        return 35;
    case 20:
        return 36;
    case 21:
        return 37;
    case 22:
        return 38;
    case 23:
        return 39;
    default:
        return 0;
    }
}

int st_remap_81259c(int id)
{
    switch (id) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        return 16;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return 15;
    case 16:
        return 21;
    case 17:
    case 18:
        return 20;
    case 19:
    case 20:
        return 21;
    case 21:
        return 19;
    case 22:
    case 23:
        return 18;
    case 24:
        return 21;
    case 25:
        return 17;
    case 26:
        return 22;
    case 27:
        return 25;
    default:
        return 0;
    }
}

int st_remap_812628(int id)
{
    switch (id) {
    case 1:
        return 8;
    case 2:
        return 9;
    case 3:
        return 10;
    case 4:
        return 11;
    case 5:
        return 12;
    case 6:
        return 13;
    case 7:
        return 14;
    case 8:
        return 15;
    case 9:
        return 16;
    case 10:
        return 17;
    default:
        return 0;
    }
}

int st_remap_8126ac(int id)
{
    switch (id) {
    case 1:
        return -1;
    case 2:
        return 0;
    case 3:
        return 1;
    case 4:
        return 2;
    case 5:
        return 3;
    case 6:
        return 4;
    case 7:
        return 5;
    case 8:
        return 6;
    case 9:
        return 7;
    default:
        return 0;
    }
}
