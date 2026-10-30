// @range 0x08822B0C-0x0882308C
// @asm 0x08822B0C-0x08823068
/* Markers over field locations (count of cards in decks/graves, selectable marks) updated from the cursor.
   Functions under NON_MATCHING stay in asm; their C is the Ghidra pseudocode cleaned of the address
   names (Ghidra types: undefined4 = int-sized, code = function pointer), kept for the port. */
#include "dd_field_ui.h"

#ifdef NON_MATCHING
typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef void (*code)();
#endif

int dd_task_pause();
void func_08823068(int h);

#ifdef NON_MATCHING
// 08822b0c size=132
void func_08822B0C(void)
{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  puVar4 = &D_08C583B8;
  memset(&D_08C583B8,0,0x180);
  iVar3 = 0;
  do {
    iVar2 = 0;
    puVar1 = puVar4;
    do {
      *puVar1 = 0xffffffff;
      puVar1[1] = 0xffffffff;
      puVar1[2] = 0xffffffff;
      puVar1[3] = 0xffffffff;
      puVar1[4] = 0xffffffff;
      puVar1[5] = 0xffffffff;
      puVar1[6] = 0xffffffff;
      iVar2 = iVar2 + 8;
      puVar1[7] = 0xffffffff;
      puVar1 = puVar1 + 8;
    } while (iVar2 < 0x10);
    iVar3 = iVar3 + 1;
    puVar4 = puVar4 + 0x10;
  } while (iVar3 < 2);
  return;
}
#endif

#ifdef NON_MATCHING
// 08822b90 size=156
void func_08822B90(void)
{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  puVar2 = &D_08C583B8;
  iVar4 = 0;
  do {
    iVar3 = 0;
    iVar1 = 0;
    while( true ) {
      piVar5 = (int *)((int)puVar2 + iVar1);
      if (*piVar5 != -1) {
        dd_task_pause();
        *piVar5 = -1;
        piVar5[0x20] = 0;
      }
      iVar3 = iVar3 + 1;
      if (0xf < iVar3) break;
      iVar1 = iVar3 * 4;
    }
    iVar4 = iVar4 + 1;
    puVar2 = puVar2 + 0x10;
  } while (iVar4 < 2);
  return;
}
#endif

#ifdef NON_MATCHING
// 08822c2c size=516
void func_08822C2C(void)
{
  int iVar1;
  int iVar2;
  int *piVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined1 *puVar8;
  undefined1 local_30 [44];
  byte local_4 [4];
  puVar7 = &D_08C583B8;
  dd_cursor_get(local_4);
  dv_slot_markers(local_4[0] & 1,(local_4[0] & 0x3f) >> 1,local_30);
  uVar6 = 0;
  puVar8 = local_30;
  do {
    uVar5 = 0;
    iVar2 = 0;
    while( true ) {
      uVar4 = (uint)(byte)puVar8[iVar2];
      iVar2 = 0;
      if (((uVar4 != 0) && (iVar1 = func_0880B7B8(uVar6,uVar5,0), iVar1 != 0)) &&
         (*(float *)(iVar1 + 0x70) < *(float *)(iVar1 + 0x74))) {
        uVar4 = 0;
      }
      if ((uVar4 == 0) && (iVar1 = dv_zone_usable(uVar6,uVar5), iVar1 == 0)) {
        uVar4 = 7;
        iVar2 = 3;
      }
      if ((uVar4 == 0) ||
         (((uVar5 == (local_4[0] & 0x3f) >> 1 && (uVar6 == (local_4[0] & 1))) && (uVar4 != 7)))) {
        piVar3 = puVar7 + uVar5;
        if (*piVar3 != -1) {
          func_08823068();
          *piVar3 = -1;
          piVar3[0x20] = 0;
          piVar3[0x40] = 0;
        }
      }
      else {
        piVar3 = puVar7 + uVar5;
        if ((piVar3[0x20] != uVar4) || (piVar3[0x40] != iVar2)) {
          func_08823068(*piVar3);
          *piVar3 = -1;
          piVar3[0x20] = 0;
          piVar3[0x40] = 0;
        }
        if (*piVar3 == -1) {
          iVar1 = func_08822E30(uVar6,uVar5,uVar4,iVar2);
          *piVar3 = iVar1;
          piVar3[0x20] = uVar4;
          piVar3[0x40] = iVar2;
        }
      }
      uVar5 = uVar5 + 1;
      if (10 < (int)uVar5) break;
      iVar2 = uVar5 * 2;
    }
    uVar6 = uVar6 + 1;
    puVar8 = puVar8 + 0x16;
    puVar7 = puVar7 + 0x10;
  } while ((int)uVar6 < 2);
  return;
}
#endif

#ifdef NON_MATCHING
// 08822e30 size=568
void func_08822E30(ushort param_1,ushort param_2,undefined1 param_3,int param_4)
{
  ushort *puVar1;
  ushort *puVar2;
  undefined4 uVar3;
  ushort *puVar4;
  uint uVar5;
  uint uVar6;
  undefined4 uVar7;
  float fVar8;
  undefined1 auStack_40 [16];
  undefined1 auStack_30 [16];
  undefined1 auStack_20 [4];
  float local_1c;
  float local_18;
  ushort local_4 [2];
  puVar2 = (ushort *)0x2;
  puVar4 = local_4;
  puVar1 = puVar4;
  while (puVar1 != (ushort *)0x0) {
    *(undefined1 *)puVar4 = 0;
    puVar4 = (ushort *)((int)puVar4 + 1);
    puVar2 = (ushort *)((int)puVar2 + -1);
    puVar1 = puVar2;
  }
  local_4[0] = local_4[0] & 0xffc0 | param_1 & 1 | (param_2 & 0x1f) << 1;
  switch(param_3) {
  default:
    uVar7 = 0;
    break;
  case 2:
    uVar7 = 1;
    break;
  case 3:
    uVar7 = 2;
    break;
  case 4:
    uVar7 = 3;
    break;
  case 5:
    uVar7 = 4;
    break;
  case 6:
    uVar7 = 5;
    break;
  case 7:
    uVar7 = 6;
  }
  func_0880A8E0(auStack_20,auStack_30,(int)(short)local_4[0]);
  uVar6 = (uint)(byte)local_4[0];
  local_1c = local_1c + 0.01;
  uVar5 = (uVar6 & 0x3f) >> 1;
  if (uVar5 == 0xc) {
    uVar3 = dv_n_18(uVar6 & 1);
    fVar8 = (float)func_0880CC04(uVar3);
    local_1c = local_1c + fVar8;
  }
  else if (uVar5 == 0xf) {
    uVar3 = dv_n_f(uVar6 & 1);
    fVar8 = (float)func_0880CC04(uVar3);
    local_1c = local_1c + fVar8;
  }
  else if (uVar5 == 0xd) {
    uVar3 = dv_n_d(uVar6 & 1);
    fVar8 = (float)func_0880CC04(uVar3);
    local_1c = local_1c + fVar8;
  }
  else if (uVar5 == 0xe) {
    uVar3 = dv_n_grave(uVar6 & 1);
    fVar8 = (float)func_0880CC04(uVar3);
    local_1c = local_1c + fVar8;
  }
  func_08820A8C(auStack_40,auStack_30,local_18 < 0.0);
  if (param_4 == 0) {
    dd_fx_mark_start(0x3f800000,uVar7,auStack_20,auStack_40,0xffffffff,0,0);
  }
  else {
    dd_fx_mark_start_layer(0x3f800000,param_4,uVar7,auStack_20,auStack_40,0xffffffff,0,0);
  }
  return;
}
#endif

void func_08823068(int h)
{
    if (h != -1)
        dd_task_pause(h);
}
