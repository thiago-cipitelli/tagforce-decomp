// @range 0x08820138-0x08820F88
// @asm 0x08820138-0x088202A0
// @asm 0x088202E4-0x08820594
// @asm 0x0882059C-0x08820770
// @asm 0x088207A8-0x08820808
// @asm 0x08820840-0x0882092C
// @asm 0x08820988-0x08820F28
/* Field effects driven by events: LP change bars, deck shuffle, coin/dice, chain counters, wait-for-condition tasks, card angles from a matrix, the summon/move placement of cards and the action window opener.
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

int func_0880CE48();
int func_08820E34();
int func_08831788();
int func_08838068();
extern int D_08C582B0;
extern int D_08C582B4;
extern int D_08C582B8;
void func_088202A0(int *p);
void func_08820594(int *t, int arg);
void func_08820770(void *t, int h);
void func_08820808(void *t, int h);
int func_0882092C(void);
void func_08820938(int *t, int arg);
void func_08820980(DdCard *c);
void func_08820F28(void);
void func_08820F58(void);

#ifdef NON_MATCHING
// 08820138 size=360
void func_08820138(int param_1,int param_2)
{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 local_30 [4];
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18 [4];
  undefined4 local_8;
  undefined4 local_4;
  local_18[0] = D_088760D0;
  local_18[1] = D_088760D4;
  local_18[2] = D_088760D8;
  local_18[3] = D_088760DC;
  local_8 = D_088760E0;
  local_4 = D_088760E4;
  local_30[0] = D_088760E8;
  local_30[1] = D_088760EC;
  local_30[2] = D_088760F0;
  local_30[3] = D_088760F4;
  local_20 = D_088760F8;
  local_1c = D_088760FC;
  if (param_2 < 0) {
    uVar5 = 0xff;
    uVar4 = 0x3c;
    uVar3 = 0x3c;
  }
  else {
    uVar5 = 0;
    uVar4 = 0xef;
    uVar3 = 0xbb;
  }
  uVar1 = dd_flag_task(0xe);
  puVar2 = (undefined4 *)dd_alloc(0xc);
  *puVar2 = uVar1;
  puVar2[1] = param_1;
  puVar2[2] = param_2;
  func_088344E0(local_18 + param_1 * 3,param_2,uVar5,uVar4,uVar3,0xff,local_30 + param_1 * 3,
               dd_task_free_pair,puVar2);
  return;
}
#endif

void func_088202A0(int *p)
{
    func_0880CE48(p[1], 1);
    dd_task_remove(p[0]);
    dd_free(p);
    func_08820E34(0);
}

#ifdef NON_MATCHING
// 088202e4 size=96
void func_088202E4(int param_1)
{
  int iVar1;
  undefined4 uVar2;
  uVar2 = *(undefined4 *)(param_1 + 0x18);
  iVar1 = func_0880CB44();
  if ((iVar1 == 0) && (iVar1 = func_0880CC64(), iVar1 != 0)) {
    func_08820344(uVar2);
    dd_task_remove_cur();
  }
  else {
    req_flag_set(0x13);
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08820344 size=236
void func_08820344(int param_1)
{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined1 auStack_1c [12];
  code *local_10;
  int local_4;
  iVar1 = func_0880CB44();
  if ((iVar1 == 0) && (iVar1 = func_0880CC64(), iVar1 != 0)) {
    puVar2 = (undefined4 *)dd_alloc(8);
    uVar3 = func_0880B770(param_1,0xd);
    uVar4 = dd_flag_task(0x13);
    func_0880CE48(param_1,0);
    *puVar2 = uVar4;
    puVar2[1] = param_1;
    func_08835364(param_1 == 0,uVar3,func_088202A0,puVar2);
    func_08820E34(1);
  }
  else {
    memset(auStack_1c,0,0x1c);
    local_10 = func_088202E4;
    local_4 = param_1;
    dd_task_add(auStack_1c);
    req_flag_set(0x13);
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08820430 size=140
void func_08820430(undefined4 param_1,undefined4 param_2,undefined4 param_3,uint param_4)
{
  uint *puVar1;
  uint *puVar2;
  undefined4 uVar3;
  uint *puVar4;
  uint local_c;
  uint local_8;
  uint local_4;
  puVar2 = (uint *)0xc;
  puVar4 = &local_c;
  puVar1 = puVar4;
  while (puVar1 != (uint *)0x0) {
    *(undefined1 *)puVar4 = 0;
    puVar4 = (uint *)((int)puVar4 + 1);
    puVar2 = (uint *)((int)puVar2 + -1);
    puVar1 = puVar2;
  }
  local_c = (param_4 & 1) != 0 ^ 1;
  local_8 = (param_4 & 2) != 0 ^ 1;
  local_4 = (param_4 & 4) != 0 ^ 1;
  uVar3 = dd_flag_task(0x37);
  func_08834AFC(param_2,&local_c,&dd_flag_task_done,uVar3);
  return;
}
#endif

#ifdef NON_MATCHING
// 088204bc size=104
void func_088204BC(int param_1,undefined4 param_2)
{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  undefined4 local_8 [2];
  puVar2 = (undefined4 *)0x8;
  puVar4 = local_8;
  puVar1 = puVar4;
  while (puVar1 != (undefined4 *)0x0) {
    *(undefined1 *)puVar4 = 0;
    puVar4 = (undefined4 *)((int)puVar4 + 1);
    puVar2 = (undefined4 *)((int)puVar2 + -1);
    puVar1 = puVar2;
  }
  local_8[0] = param_2;
  uVar3 = dd_flag_task(0x38);
  func_08834F70(1,local_8,param_1 == 0,&dd_flag_task_done,uVar3);
  return;
}
#endif

#ifdef NON_MATCHING
// 08820524 size=112
void func_08820524(undefined4 param_1,undefined4 *param_2)
{
  int iVar1;
  req_flag_set(*param_2);
  if (param_2[2] == 0) {
    iVar1 = param_2[2];
  }
  else {
    iVar1 = (*(code *)param_2[1])();
    if (iVar1 != 0) goto D_0882057C;
    iVar1 = param_2[2];
  }
  if (iVar1 != 0) {
    return;
  }
  iVar1 = (*(code *)param_2[1])();
  if (iVar1 != 0) {
    return;
  }
D_0882057C:
  dd_task_remove_cur();
  return;
}
#endif

void func_08820594(int *t, int arg)
{
    func_08820938(t, arg);
}

#ifdef NON_MATCHING
// 0882059c size=220
void func_0882059C(undefined4 param_1,code *param_2,int param_3)
{
  int iVar1;
  undefined4 *puVar2;
  undefined1 auStack_1c [12];
  code *local_10;
  undefined1 *local_c;
  undefined4 *local_4;
  if (((param_3 == 0) || (iVar1 = (*param_2)(), iVar1 == 0)) &&
     ((param_3 != 0 || (iVar1 = (*param_2)(), iVar1 != 0)))) {
    req_flag_set(param_1);
    puVar2 = (undefined4 *)dd_alloc(0x10);
    memset(puVar2,0,0x10);
    *puVar2 = param_1;
    puVar2[1] = param_2;
    puVar2[2] = param_3;
    memset(auStack_1c,0,0x1c);
    local_10 = func_08820524;
    local_c = &func_08820594;
    local_4 = puVar2;
    dd_task_add(auStack_1c);
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08820678 size=92
void func_08820678(undefined4 param_1,undefined4 param_2,undefined4 param_3)
{
  undefined4 uVar1;
  uVar1 = dd_flag_task(0x35);
  func_088357B0(param_3,param_2,param_1,&dd_flag_task_done,uVar1);
  return;
}
#endif

#ifdef NON_MATCHING
// 088206d4 size=156
void func_088206D4(int param_1,int param_2)
{
  int iVar1;
  undefined4 uVar2;
  if (param_2 == 0) {
    uVar2 = dd_flag_task(0x3b);
    iVar1 = region_get();
    func_08837098(param_1 + 1,iVar1 == 0,&dd_flag_task_done,uVar2);
  }
  else {
    iVar1 = sysvar_get_a98();
    if (((iVar1 == 0) && (iVar1 = cfg_get_flag(0), iVar1 != 0)) && (param_1 == 4)) {
      dd_mode_set_card(0x1468);
      dd_mode_push(0xb);
    }
  }
  return;
}
#endif

void func_08820770(void *t, int h)
{
    if (func_08838068(h) != 0)
        dd_task_remove_cur();
    req_flag_set(0x3C);
}

#ifdef NON_MATCHING
// 088207a8 size=96
void func_088207A8(void)
{
  undefined4 uVar1;
  undefined1 auStack_1c [8];
  undefined4 local_14;
  code *local_10;
  undefined4 local_c;
  undefined4 local_4;
  uVar1 = func_088380BC(0x14,0,0);
  memset(auStack_1c,0,0x1c);
  local_10 = func_08820770;
  local_14 = 0;
  local_c = 0;
  local_4 = uVar1;
  dd_task_add(auStack_1c);
  return;
}
#endif

void func_08820808(void *t, int h)
{
    if (func_08831788(h) != 0)
        dd_task_remove_cur();
    req_flag_set(0x39);
}

#ifdef NON_MATCHING
// 08820840 size=112
void func_08820840(int param_1,undefined4 param_2)
{
  float fVar1;
  fVar1 = (float)func_08807034();
  if (1.0 / fVar1 <= *(float *)(param_1 + 0x14)) {
    func_088264D4(param_2);
    *(code **)(param_1 + 0xc) = func_08820808;
  }
  req_flag_set(0x39);
  return;
}
#endif

#ifdef NON_MATCHING
// 088208b0 size=124
void func_088208B0(void)
{
  undefined4 uVar1;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 auStack_1c [8];
  undefined4 local_14;
  code *local_10;
  undefined4 local_c;
  undefined4 local_4;
  local_28 = 0x13a;
  local_24 = 0x84;
  local_20 = 0;
  uVar1 = func_088317BC(&local_28,0,0);
  memset(auStack_1c,0,0x1c);
  local_10 = func_08820840;
  local_14 = 0;
  local_c = 0;
  local_4 = uVar1;
  dd_task_add(auStack_1c);
  req_flag_set(0x39);
  return;
}
#endif

int func_0882092C(void)
{
    return D_08C582B0;
}

/* default task leave: frees the task argument */
void func_08820938(int *t, int arg)
{
    if (arg != 0) {
        dd_free(arg);
        if (arg == t[6])
            t[6] = 0;
    }
}

void func_08820980(DdCard *c)
{
    c->fx = 0;
}

#ifdef NON_MATCHING
// 08820988 size=260
void func_08820988(undefined4 *param_1,undefined4 *param_2)
{
  undefined4 uVar1;
  float fVar2;
  uVar1 = fmaxf_vfpu(param_2[2],0xbf800000);
  fVar2 = (float)fminf_vfpu(uVar1,0x3f800000);
  fVar2 = (float)asinf_vfpu(-fVar2);
  param_1[1] = fVar2;
  if ((fVar2 < 1.5393804) || (1.6022123 < fVar2)) {
    if (-1.6022123 <= fVar2) {
      if (fVar2 <= -1.5393804) {
        *param_1 = 0;
        goto D_08820A3C;
      }
      uVar1 = param_2[6];
    }
    else {
      uVar1 = param_2[6];
    }
    uVar1 = atan2f_vfpu(uVar1,param_2[10]);
    *param_1 = uVar1;
    uVar1 = atan2f_vfpu(param_2[1],*param_2);
    param_1[2] = uVar1;
  }
  else {
    *param_1 = 0;
D_08820A3C:
    fVar2 = (float)atan2f_vfpu(param_2[4],param_2[5]);
    param_1[2] = -fVar2;
  }
  param_1[3] = 0;
  return;
}
#endif

#ifdef NON_MATCHING
// 08820a8c size=564
void func_08820A8C(float *param_1,float *param_2)
{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  fVar1 = *param_2;
  fVar2 = param_2[1];
  fVar7 = fVar1 + fVar1;
  fVar5 = param_2[2] + param_2[2];
  fVar8 = fVar2 + fVar2;
  fVar1 = -(fVar1 * fVar5 - param_2[3] * fVar8);
  if (1.0 <= fVar1) {
    fVar1 = 1.0;
  }
  else if (fVar1 <= -1.0) {
    fVar1 = -1.0;
  }
  if ((fVar1 == 0.0) || (fVar1 == -0.0)) {
    if ((fVar2 == 1.0) || (fVar2 == -1.0)) {
      fVar1 = 3.1415927;
    }
    else {
      fVar1 = 0.0;
    }
  }
  else {
    fVar1 = (float)asinf_vfpu();
  }
  param_1[1] = fVar1;
  fVar3 = param_2[2] * fVar5;
  fVar2 = *param_2 * fVar7;
  fVar6 = *param_2 * fVar8;
  fVar4 = param_2[3] * fVar5;
  if (fVar1 < 1.5707964) {
    if (-1.5707964 < fVar1) {
      fVar8 = param_2[1] * fVar8;
      if (fVar6 + fVar4 == 0.0) {
        fVar4 = 1.1754944e-38;
      }
      fVar1 = (float)atan2f_vfpu(param_2[1] * fVar5 + param_2[3] * fVar7,1.0 - (fVar2 + fVar8));
      *param_1 = fVar1;
      fVar1 = (float)atan2f_vfpu(fVar6 + fVar4,1.0 - (fVar8 + fVar3));
      param_1[2] = fVar1;
    }
    else {
      fVar1 = (float)atan2f_vfpu(fVar6 - fVar4,1.0 - (fVar2 + fVar3));
      *param_1 = -fVar1;
      param_1[2] = 0.0;
    }
  }
  else {
    fVar1 = (float)atan2f_vfpu(fVar6 - fVar4,1.0 - (fVar2 + fVar3));
    *param_1 = fVar1;
    param_1[2] = 0.0;
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08820cc0 size=128
void func_08820CC0(undefined1 *param_1,undefined1 *param_2,short param_3)
{
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 auStack_20 [16];
  undefined1 auStack_10 [16];
  if (param_1 == (undefined1 *)0x0) {
    param_1 = auStack_10;
  }
  if (param_2 == (undefined1 *)0x0) {
    param_2 = auStack_20;
  }
  func_0880A8E0(param_1,param_2,(int)param_3);
  local_30 = D_08876100;
  local_2c = D_08876104;
  local_28 = D_08876108;
  local_24 = D_0887610C;
  quat_mul(param_2,param_2,&local_30);
  return;
}
#endif

#ifdef NON_MATCHING
// 08820d40 size=244
undefined4 func_08820D40(int param_1)
{
  int iVar1;
  undefined1 auStack_90 [16];
  undefined1 auStack_80 [64];
  undefined1 auStack_40 [64];
  if ((*(int *)(param_1 + 0x94) != -1) || (*(int *)(param_1 + 0x90) != -1)) {
    mat4_identity(auStack_40);
    quat_to_mat4(auStack_80,param_1 + 0x40);
    mat4_mul(auStack_40,auStack_80,auStack_40);
    mat4_translate(auStack_40,auStack_40,param_1 + 0x10);
    func_08820A8C(auStack_90,param_1 + 0x40,*(float *)(param_1 + 0x18) < 0.0);
    if (*(int *)(param_1 + 0x94) == -1) {
      iVar1 = *(int *)(param_1 + 0x90);
    }
    else {
      func_08832200((float)dd_glow_kind,*(int *)(param_1 + 0x94),param_1 + 0x10,auStack_90);
      iVar1 = *(int *)(param_1 + 0x90);
    }
    if (iVar1 != -1) {
      func_08832200((float)dd_glow_kind,iVar1,param_1 + 0x10,auStack_90);
    }
  }
  return 0;
}
#endif

#ifdef NON_MATCHING
void func_08820E34(int on)
{
    D_08C582B0 = on != 0;
}
#endif

#ifdef NON_MATCHING
// 08820e58 size=80
void func_08820E58(void)
{
  undefined4 uVar1;
  uVar1 = dd_text(0x37);
  func_088226C0(uVar1,1,0);
  func_08825B88(0);
  func_088257E4(0,100);
  func_08825818();
  func_08809508(1);
  return;
}
#endif

#ifdef NON_MATCHING
// 08820ea8 size=128
void func_08820EA8(int param_1,undefined4 param_2)
{
  int iVar1;
  func_08809508(0);
  if ((0 < param_1) && ((iVar1 = func_08825A14(), iVar1 == 2 || (iVar1 == 1)))) {
    dd_mode_set_card(param_1);
    dd_mode_push(5);
    dd_mode_set_flag(param_2);
  }
  return;
}
#endif

void func_08820F28(void)
{
    memset(&D_08C582B8, 0, 0x9C);
    D_08C582B4 = 0;
}

void func_08820F58(void)
{
    memset(&D_08C582B8, 0, 0x9C);
    D_08C582B4 = 0;
}
