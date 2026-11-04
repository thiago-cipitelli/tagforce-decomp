// @range 0x0880D4F4-0x0880F228
// @rodata 0x08838384-0x0883860C
/* Index tables: chara id -> SD model (asm), bust-up, room, room group and voice file indices. */
#include "libfield.h"

/* SD model index of a chara id (0 when it has none) */
int fld_chr_index(int x)
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
    case 1001:
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
    case 1004:
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
    case 1002:
        r = 45;
        break;
    case 48:
        r = 46;
        break;
    case 49:
        r = 47;
        break;
    case 1003:
        r = 48;
        break;
    case 51:
        r = 49;
        break;
    case 58:
        r = 50;
        break;
    case 1006:
        r = 51;
        break;
    case 1005:
        r = 52;
        break;
    case 1000:
        r = 53;
        break;
    case 1007:
        r = 54;
        break;
    case 1008:
        r = 55;
        break;
    case 1009:
        r = 56;
        break;
    case 1010:
        r = 57;
        break;
    case 1011:
        r = 58;
        break;
    case 1012:
        r = 59;
        break;
    case 100:
        r = 60;
        break;
    case 101:
        r = 61;
        break;
    case 102:
        r = 62;
        break;
    case 103:
        r = 63;
        break;
    case 104:
        r = 64;
        break;
    case 105:
        r = 65;
        break;
    case 106:
        r = 66;
        break;
    case 107:
        r = 67;
        break;
    case 108:
        r = 68;
        break;
    case 109:
        r = 69;
        break;
    case 110:
        r = 70;
        break;
    case 111:
        r = 71;
        break;
    case 112:
        r = 72;
        break;
    case 113:
        r = 73;
        break;
    case 114:
        r = 74;
        break;
    case 115:
        r = 75;
        break;
    case 116:
        r = 76;
        break;
    case 117:
        r = 77;
        break;
    case 118:
        r = 78;
        break;
    case 119:
        r = 79;
        break;
    case 120:
        r = 80;
        break;
    case 121:
        r = 81;
        break;
    case 122:
        r = 82;
        break;
    case 123:
        r = 83;
        break;
    case 124:
        r = 84;
        break;
    case 125:
        r = 85;
        break;
    case 126:
        r = 86;
        break;
    case 127:
        r = 87;
        break;
    case 128:
        r = 88;
        break;
    case 129:
        r = 89;
        break;
    case 130:
        r = 90;
        break;
    case 131:
        r = 91;
        break;
    case 132:
        r = 92;
        break;
    case 133:
        r = 93;
        break;
    case 134:
        r = 94;
        break;
    case 135:
        r = 95;
        break;
    case 136:
        r = 96;
        break;
    case 137:
        r = 97;
        break;
    case 138:
        r = 98;
        break;
    case 139:
        r = 99;
        break;
    case 140:
        r = 100;
        break;
    case 141:
        r = 101;
        break;
    case 142:
        r = 102;
        break;
    case 143:
        r = 103;
        break;
    case 144:
        r = 104;
        break;
    case 145:
        r = 105;
        break;
    case 146:
        r = 106;
        break;
    case 147:
        r = 107;
        break;
    case 148:
        r = 108;
        break;
    case 149:
        r = 109;
        break;
    case 200:
        r = 110;
        break;
    case 201:
        r = 111;
        break;
    case 202:
        r = 112;
        break;
    case 203:
        r = 113;
        break;
    case 204:
        r = 114;
        break;
    case 205:
        r = 115;
        break;
    case 206:
        r = 116;
        break;
    case 207:
        r = 117;
        break;
    case 208:
        r = 118;
        break;
    case 209:
        r = 119;
        break;
    case 210:
        r = 120;
        break;
    case 211:
        r = 121;
        break;
    case 212:
        r = 122;
        break;
    case 213:
        r = 123;
        break;
    case 214:
        r = 124;
        break;
    case 215:
        r = 125;
        break;
    case 216:
        r = 126;
        break;
    case 217:
        r = 127;
        break;
    case 218:
        r = 128;
        break;
    case 219:
        r = 129;
        break;
    case 220:
        r = 130;
        break;
    case 221:
        r = 131;
        break;
    case 222:
        r = 132;
        break;
    case 223:
        r = 133;
        break;
    case 224:
        r = 134;
        break;
    case 225:
        r = 135;
        break;
    case 226:
        r = 136;
        break;
    case 227:
        r = 137;
        break;
    case 228:
        r = 138;
        break;
    case 229:
        r = 139;
        break;
    case 230:
        r = 140;
        break;
    case 231:
        r = 141;
        break;
    case 232:
        r = 142;
        break;
    case 233:
        r = 143;
        break;
    case 234:
        r = 144;
        break;
    case 235:
        r = 145;
        break;
    case 236:
        r = 146;
        break;
    case 237:
        r = 147;
        break;
    case 238:
        r = 148;
        break;
    case 239:
        r = 149;
        break;
    case 240:
        r = 150;
        break;
    case 241:
        r = 151;
        break;
    case 242:
        r = 152;
        break;
    case 243:
        r = 153;
        break;
    case 244:
        r = 154;
        break;
    case 245:
        r = 155;
        break;
    case 246:
        r = 156;
        break;
    case 247:
        r = 157;
        break;
    case 248:
        r = 158;
        break;
    case 249:
        r = 159;
        break;
    case 300:
        r = 160;
        break;
    case 301:
        r = 161;
        break;
    case 302:
        r = 162;
        break;
    case 303:
        r = 163;
        break;
    case 304:
        r = 164;
        break;
    case 305:
        r = 165;
        break;
    case 306:
        r = 166;
        break;
    case 307:
        r = 167;
        break;
    case 308:
        r = 168;
        break;
    case 309:
        r = 169;
        break;
    case 310:
        r = 170;
        break;
    case 311:
        r = 171;
        break;
    case 312:
        r = 172;
        break;
    case 313:
        r = 173;
        break;
    case 314:
        r = 174;
        break;
    case 315:
        r = 175;
        break;
    case 316:
        r = 176;
        break;
    case 317:
        r = 177;
        break;
    case 318:
        r = 178;
        break;
    case 319:
        r = 179;
        break;
    case 320:
        r = 180;
        break;
    case 321:
        r = 181;
        break;
    case 322:
        r = 182;
        break;
    case 323:
        r = 183;
        break;
    case 324:
        r = 184;
        break;
    case 325:
        r = 185;
        break;
    case 326:
        r = 186;
        break;
    case 327:
        r = 187;
        break;
    case 328:
        r = 188;
        break;
    case 329:
        r = 189;
        break;
    case 330:
        r = 190;
        break;
    case 331:
        r = 191;
        break;
    case 332:
        r = 192;
        break;
    case 333:
        r = 193;
        break;
    case 334:
        r = 194;
        break;
    case 335:
        r = 195;
        break;
    case 336:
        r = 196;
        break;
    case 337:
        r = 197;
        break;
    case 338:
        r = 198;
        break;
    case 339:
        r = 199;
        break;
    case 340:
        r = 200;
        break;
    case 341:
        r = 201;
        break;
    case 342:
        r = 202;
        break;
    case 343:
        r = 203;
        break;
    case 344:
        r = 204;
        break;
    case 345:
        r = 205;
        break;
    case 346:
        r = 206;
        break;
    case 347:
        r = 207;
        break;
    case 348:
        r = 208;
        break;
    case 349:
        r = 209;
        break;
    case 400:
        r = 210;
        break;
    case 401:
        r = 211;
        break;
    case 402:
        r = 212;
        break;
    case 403:
        r = 213;
        break;
    case 404:
        r = 214;
        break;
    case 405:
        r = 215;
        break;
    case 406:
        r = 216;
        break;
    case 407:
        r = 217;
        break;
    case 408:
        r = 218;
        break;
    case 409:
        r = 219;
        break;
    case 410:
        r = 220;
        break;
    case 411:
        r = 221;
        break;
    case 412:
        r = 222;
        break;
    case 413:
        r = 223;
        break;
    case 414:
        r = 224;
        break;
    case 415:
        r = 225;
        break;
    case 416:
        r = 226;
        break;
    case 417:
        r = 227;
        break;
    case 418:
        r = 228;
        break;
    case 419:
        r = 229;
        break;
    case 420:
        r = 230;
        break;
    case 421:
        r = 231;
        break;
    case 422:
        r = 232;
        break;
    case 423:
        r = 233;
        break;
    case 424:
        r = 234;
        break;
    case 425:
        r = 235;
        break;
    case 426:
        r = 236;
        break;
    case 427:
        r = 237;
        break;
    case 428:
        r = 238;
        break;
    case 429:
        r = 239;
        break;
    case 430:
        r = 240;
        break;
    case 431:
        r = 241;
        break;
    case 432:
        r = 242;
        break;
    case 433:
        r = 243;
        break;
    case 434:
        r = 244;
        break;
    case 435:
        r = 245;
        break;
    case 436:
        r = 246;
        break;
    case 437:
        r = 247;
        break;
    case 438:
        r = 248;
        break;
    case 439:
        r = 249;
        break;
    case 440:
        r = 250;
        break;
    case 441:
        r = 251;
        break;
    case 442:
        r = 252;
        break;
    case 443:
        r = 253;
        break;
    case 444:
        r = 254;
        break;
    case 445:
        r = 255;
        break;
    case 446:
        r = 256;
        break;
    case 447:
        r = 257;
        break;
    case 448:
        r = 258;
        break;
    case 449:
        r = 259;
        break;
    }
    return r;
}

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
