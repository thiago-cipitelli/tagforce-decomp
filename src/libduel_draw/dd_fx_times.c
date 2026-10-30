// @range 0x0884BA1C-0x0884BDB0
// @rodata 0x08870850-0x08870B90
/* Timing tables of the card effects: frame where the effect starts, frame of the hit (start + 8)
   and duration, by effect id (dd_fx_flare: 247-281; dd_fx_hit_se: 204-376). -1 = no visual. */
#include "libduel_draw.h"

void dd_fx_hit_times(int id, int *delay, int *hit, int *len)
{
    *len = 24;
    switch (id) {
    case 247:
        *delay = 112;
        break;
    case 248:
        *delay = 89;
        break;
    case 249:
        *delay = 185;
        break;
    case 250:
        *delay = 186;
        break;
    case 251:
        *delay = 197;
        break;
    case 253:
        *delay = -1;
        *hit = 167;
        return;
    case 254:
        *delay = 129;
        break;
    case 255:
        *delay = 167;
        break;
    case 256:
        *delay = 170;
        break;
    case 257:
        *delay = 202;
        break;
    case 258:
        *delay = 185;
        *hit = 190;
        return;
    case 259:
        *delay = 152;
        break;
    case 260:
        *delay = 318;
        break;
    case 261:
        *delay = 340;
        break;
    case 262:
        *delay = 182;
        break;
    case 263:
        *delay = 181;
        break;
    case 264:
        *delay = 127;
        break;
    case 266:
        *delay = 165;
        break;
    case 267:
        *delay = 162;
        break;
    case 268:
        *delay = 137;
        break;
    case 269:
        *delay = 165;
        break;
    case 270:
        *delay = 208;
        break;
    case 271:
        *delay = 207;
        break;
    case 272:
        *delay = 167;
        break;
    case 273:
        *delay = 162;
        break;
    case 274:
        *delay = 174;
        break;
    case 276:
        *delay = 177;
        break;
    case 277:
        *delay = 152;
        break;
    case 279:
        *delay = 222;
        break;
    case 280:
        *delay = 141;
        break;
    case 281:
        *delay = 141;
        break;
    default:
        *delay = 0;
        *hit = 0;
        *len = 0;
        return;
    }
    *hit = *delay + 8;
}

void dd_fx_hit2_times(int id, int *delay, int *hit, int *len)
{
    *len = 1;
    switch (id) {
    case 204:
    case 205:
    case 207:
    case 209:
    case 212:
    case 213:
    case 214:
    case 219:
    case 220:
    case 221:
    case 223:
    case 224:
    case 225:
    case 226:
    case 229:
    case 230:
    case 233:
    case 234:
    case 236:
    case 237:
    case 242:
    case 243:
    case 244:
    case 245:
        *delay = 7;
        break;
    case 206:
    case 211:
    case 228:
    case 231:
    case 232:
    case 235:
    case 239:
    case 240:
    case 241:
    case 246:
        *delay = 12;
        break;
    case 208:
    case 218:
        *delay = 10;
        break;
    case 210:
    case 215:
        *delay = -1;
        *hit = 10;
        return;
    case 216:
    case 227:
        *delay = 11;
        break;
    case 217:
    case 238:
        *delay = 2;
        break;
    case 290:
    case 296:
    case 298:
    case 307:
    case 308:
    case 311:
    case 312:
    case 320:
    case 321:
    case 330:
    case 331:
    case 333:
        *delay = 7;
        break;
    case 291:
    case 299:
    case 300:
    case 301:
    case 305:
    case 306:
    case 310:
    case 314:
    case 315:
    case 316:
    case 317:
    case 318:
    case 319:
    case 322:
    case 323:
    case 324:
    case 325:
    case 327:
    case 329:
        *delay = 12;
        break;
    case 292:
    case 328:
        *delay = 23;
        break;
    case 293:
        *delay = 17;
        break;
    case 297:
    case 302:
        *delay = -1;
        *hit = 14;
        return;
    case 303:
        *delay = 11;
        break;
    case 304:
        *delay = 8;
        break;
    case 313:
        *delay = 6;
        break;
    case 326:
        *delay = 37;
        break;
    case 334:
    case 344:
    case 348:
    case 353:
    case 354:
    case 355:
        *delay = 10;
        break;
    case 335:
        *delay = 39;
        break;
    case 336:
    case 337:
    case 351:
    case 359:
    case 360:
    case 376:
        *delay = 7;
        break;
    case 339:
    case 349:
    case 350:
    case 358:
    case 362:
    case 363:
    case 364:
    case 365:
    case 369:
    case 370:
    case 371:
    case 372:
    case 373:
    case 374:
    case 375:
        *delay = 12;
        break;
    case 340:
        *delay = -1;
        *hit = 18;
        return;
    case 341:
    case 342:
    case 343:
        *delay = 8;
        break;
    case 345:
        *delay = -1;
        *hit = 5;
        return;
    case 346:
    case 361:
        *delay = 11;
        break;
    case 347:
    case 356:
        *delay = 9;
        break;
    case 357:
        *delay = -1;
        *hit = 0;
        return;
    case 366:
        *delay = 2;
        break;
    case 367:
    case 368:
        *delay = 4;
        break;
    default:
        *delay = 0;
        *hit = 0;
        *len = 0;
        return;
    }
    *hit = *delay + 8;
}
