// @range 0x08820F88-0x08822B0C
// @asm 0x08820F88-0x088212B0
// @asm 0x088212C4-0x0882192C
// @asm 0x08821954-0x08821F38
// @asm 0x08821F70-0x08821FA0
// @asm 0x08821FA8-0x088228B8
// @asm 0x088228F0-0x08822B0C
/* The action window over a field location (summon, set, activate, attack...), the card-action icons, the message windows of kinds 0-2 (plain, yes/no, choice) and their menu hooks.
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

int func_08821460();
int func_088215A0();
int func_08822914();
int func_08825AFC();
extern int D_08C582B4;
extern int D_08C58358;
void func_088212B0(void);
void func_088212B8(int v);
int func_0882192C(int list);
void func_08821F38(int a, int b, int i);
void *func_08821FA0(void *list);
void func_088228B8(MenuWidget *w, int open);

#ifdef NON_MATCHING
// 08820f88 size=808
void func_08820F88(undefined4 *param_1)
{
  short sVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  undefined4 *puVar7;
  ushort local_4 [2];
  puVar7 = &D_08C582B8;
  (*(code *)*param_1)(local_4);
  D_08C582B8 = *param_1;
  D_08C582BC = 0;
  D_08C58340 = 0;
  D_08C58344 = 0;
  D_08C58350 = dv_zone_card_id((byte)local_4[0] & 1,((byte)local_4[0] & 0x3f) >> 1,local_4[0] >> 6);
  thunk_FUN_08821460();
  if ((D_08C582B4 == (code *)0x0) ||
     (iVar6 = (*D_08C582B4)((int)(short)local_4[0],&D_08C582C0,D_08C582BC), iVar6 < 1)) {
    if (D_08C582BC == 0) {
      se_play(0x6215);
    }
    else {
      iVar6 = 0;
      if (0 < D_08C582BC) {
        do {
          if ((puVar7[2] != 0xb) && (puVar7[2] != 0xd)) break;
          iVar6 = iVar6 + 1;
          puVar7 = puVar7 + 1;
        } while (iVar6 < D_08C582BC);
      }
      if ((iVar6 == D_08C582BC) && (D_08C582C0 == 0xb)) {
        func_088218CC((int)(short)local_4[0]);
        se_play(0x6213);
      }
      else {
        func_08825824(0,0,0x20,func_0882136C);
        dd_mode_push(4);
        iVar6 = func_08806D04();
        if (iVar6 == 0) {
          dd_mode_set_flag(0x31);
        }
        func_088259FC(func_08821868);
        func_088259E4(func_0882192C);
        func_08825A08(func_08821BAC);
        func_08825B7C(1);
        se_play(0x6216);
        uVar4 = (uint)(byte)local_4[0];
        uVar3 = (uVar4 & 0x3f) >> 1;
        if (uVar3 == 0xc) {
          local_4[0] = local_4[0] & 0x3f;
        }
        else if (uVar3 == 0xd) {
          local_4[0] = local_4[0] & 0x3f;
        }
        else if (uVar3 == 0xf) {
          sVar1 = dv_n_f(uVar4 & 1);
          local_4[0] = local_4[0] & 0x3f | (sVar1 + -1) * 0x40;
        }
        else if (uVar3 == 0xe) {
          sVar1 = dv_n_grave(uVar4 & 1);
          local_4[0] = local_4[0] & 0x3f | (sVar1 + -1) * 0x40;
        }
        iVar6 = func_0880B7B8((byte)local_4[0] & 1,((byte)local_4[0] & 0x3f) >> 1,local_4[0] >> 6);
        if (iVar6 != 0) {
          iVar2 = func_08806E58((int)(short)local_4[0]);
          if (iVar2 == 0) {
            if (-1 < (int)*(char *)(iVar6 + 0x8d) << 0x1a) {
              return;
            }
            uVar5 = *(undefined4 *)(iVar6 + 0x88);
          }
          else {
            uVar5 = *(undefined4 *)(iVar6 + 0x88);
          }
          func_088091C8(uVar5);
        }
      }
    }
  }
  else {
    func_088218CC((int)(short)local_4[0],iVar6 + -1);
  }
  return;
}
#endif

void func_088212B0(void)
{
    func_08821460();
}

void func_088212B8(int v)
{
    D_08C582B4 = v;
}

#ifdef NON_MATCHING
// 088212c4 size=168
void func_088212C4(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4)
{
  undefined4 uVar1;
  uVar1 = func_088044B0(0x40005);
  uVar1 = gim_set_texture(param_1,uVar1);
  uVar1 = draw_set_offsets(uVar1,0xd,0xd,0xd,0xd);
  draw_sprite(uVar1,param_2,param_3,0,*(undefined4 *)(&D_08876110 + param_4 * 0x10),
              *(undefined4 *)(&D_08876114 + param_4 * 0x10),0,0);
  return;
}
#endif

#ifdef NON_MATCHING
// 0882136c size=244
void func_0882136C(int param_1,int param_2)
{
  if (param_2 == 0) {
    *(short *)(param_1 + 0xf0) = (short)D_08C58340 + 0x12;
    *(undefined2 *)(param_1 + 0xf8) = *(undefined2 *)(param_1 + 0xf0);
    *(short *)(param_1 + 0x6c) =
         *(short *)(param_1 + 0x92) + *(short *)(param_1 + 0xf0) + 8 + *(short *)(param_1 + 0x90);
    D_08C58344 = param_1;
    if (*(short *)(param_1 + 0x5a) < 0) {
      *(undefined2 *)(param_1 + 0x5a) = 0;
    }
  }
  else {
    thunk_FUN_08821460();
    *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xfb;
    *(short *)(param_1 + 0xcc) = (short)D_08C582BC;
    *(char *)(param_1 + 0xce) = (char)D_08C582BC;
    *(undefined2 *)(param_1 + 0xf0) = 100;
    *(undefined2 *)(param_1 + 0xf2) = 0xf;
    *(code **)(param_1 + 0xfc) = func_08821954;
    *(undefined1 *)(param_1 + 0x56) = 1;
    *(undefined2 *)(param_1 + 0x94) = 2;
    *(undefined2 *)(param_1 + 0x96) = 2;
    *(undefined2 *)(param_1 + 0x90) = 2;
    *(undefined2 *)(param_1 + 0x92) = 2;
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08821460 size=320
void func_08821460(void)
{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined *puVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  undefined4 *puVar10;
  short local_4 [2];
  puVar10 = &D_08C582B8;
  (*D_08C582B8)(local_4);
  uVar1 = func_08821A3C((int)local_4[0]);
  iVar9 = 0;
  iVar8 = 0;
  D_08C5834C = uVar1;
  font_set_size(0xc,0xc);
  uVar6 = 0;
  puVar5 = &D_08876110;
  puVar4 = puVar10;
  while ((uVar6 < 0xf && (iVar9 < 0x21))) {
    if ((uVar1 & 1 << (*(uint *)(puVar5 + 0xc) & 0x1f)) != 0) {
      puVar4[2] = uVar6;
      puVar4 = puVar4 + 1;
      iVar9 = iVar9 + 1;
    }
    puVar5 = puVar5 + 0x10;
    uVar6 = uVar6 + 1;
  }
  iVar7 = 0;
  if (0 < iVar9) {
    do {
      uVar2 = func_08821C90(*(undefined4 *)(puVar10[2] * 0x10 + 0x8876118));
      iVar3 = font_measure_text(uVar2);
      iVar3 = iVar3 >> 6;
      if (iVar3 <= iVar8) {
        iVar3 = iVar8;
      }
      iVar7 = iVar7 + 1;
      puVar10 = puVar10 + 1;
      iVar8 = iVar3;
    } while (iVar7 < iVar9);
  }
  D_08C58340 = iVar8;
  D_08C582BC = iVar9;
  if (0x20 < iVar9) {
    D_08C582BC = 0x20;
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 088215a0 size=712
void func_088215A0(void)
{
  int iVar1;
  int iVar2;
  short sVar3;
  float fVar4;
  undefined1 auStack_140 [64];
  undefined1 auStack_100 [64];
  undefined1 auStack_c0 [64];
  undefined1 auStack_80 [64];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  float local_2c;
  float local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  ushort local_4 [2];
  if (D_08C58344 != 0) {
    (*D_08C582B8)(local_4);
    iVar2 = func_0880B7B8((byte)local_4[0] & 1,((byte)local_4[0] & 0x3f) >> 1,local_4[0] >> 6);
    if ((iVar2 == 0) || (((byte)local_4[0] & 0x3f) >> 1 != 0xb)) {
      func_0880A8E0(&local_20,&local_40,(int)(short)local_4[0]);
    }
    else {
      local_20 = *(undefined4 *)(iVar2 + 0x10);
      local_1c = *(undefined4 *)(iVar2 + 0x14);
      local_18 = *(undefined4 *)(iVar2 + 0x18);
      local_14 = *(undefined4 *)(iVar2 + 0x1c);
      local_40 = *(undefined4 *)(iVar2 + 0x40);
      local_3c = *(undefined4 *)(iVar2 + 0x44);
      local_38 = *(undefined4 *)(iVar2 + 0x48);
      local_34 = *(undefined4 *)(iVar2 + 0x4c);
    }
    mtx_set_mode(2);
    mtx_get34(auStack_c0);
    mtx_set_mode(1);
    mtx_get34(auStack_100);
    mtx_set_mode(0);
    mtx_get34(auStack_140);
    mat4_mul(auStack_80,auStack_100,auStack_c0);
    mat4_mul(auStack_80,auStack_140,auStack_80);
    mtx_set_mode(2);
    local_14 = 0x3f800000;
    mat4_transform_vec4(&local_30,auStack_80,&local_20);
    iVar1 = D_08C58344;
    fVar4 = ((local_30 * 480.0) / local_24 + 2122.0) - 1808.0;
    iVar2 = (int)*(short *)(D_08C58344 + 0x6c);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 1;
    }
    *(short *)(D_08C58344 + 0x58) = (short)(int)(fVar4 - (float)(iVar2 >> 1));
    *(short *)(iVar1 + 0x5a) =
         (short)(int)(((((local_2c * -272.0) / local_24 + 2044.0) - 1912.0) - 20.0) -
                     (float)(int)*(short *)(iVar1 + 0x6e));
    if ((int)*(short *)(iVar1 + 0x58) + (int)*(short *)(iVar1 + 0x6c) < 0x1e1) {
      sVar3 = *(short *)(iVar1 + 0x58);
    }
    else {
      *(short *)(iVar1 + 0x58) = 0x1e0 - *(short *)(iVar1 + 0x6c);
      sVar3 = *(short *)(iVar1 + 0x58);
    }
    if (sVar3 < 0x94) {
      *(undefined2 *)(iVar1 + 0x58) = 0x94;
      sVar3 = *(short *)(iVar1 + 0x5a);
    }
    else {
      sVar3 = *(short *)(iVar1 + 0x5a);
    }
    if (sVar3 < 0) {
      *(undefined2 *)(iVar1 + 0x5a) = 0;
      sVar3 = *(short *)(iVar1 + 0x58);
    }
    else {
      sVar3 = *(short *)(iVar1 + 0x58);
    }
    sVar3 = (short)(int)(fVar4 - (float)(int)sVar3);
    *(short *)(iVar1 + 0x74) = sVar3;
    if (sVar3 < 4) {
      *(undefined2 *)(iVar1 + 0x74) = 4;
      sVar3 = *(short *)(iVar1 + 0x6c);
    }
    else {
      sVar3 = *(short *)(iVar1 + 0x6c);
    }
    if (sVar3 + -4 < (int)*(short *)(iVar1 + 0x74)) {
      *(short *)(iVar1 + 0x74) = (short)(sVar3 + -4);
    }
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08821868 size=100
void func_08821868(void)
{
  int iVar1;
  short local_4 [2];
  if (-1 < D_08C58348) {
    iVar1 = (&D_08C582C0)[D_08C58348];
    (*D_08C582B8)(local_4);
    func_088218CC((int)local_4[0],(&D_0887611C)[iVar1 * 4]);
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 088218cc size=96
void func_088218CC(ushort param_1,int param_2)
{
  undefined1 local_4;
  if (param_2 < 0xd) {
    local_4 = (byte)param_1;
    dd_act(local_4 & 1,(local_4 & 0x3f) >> 1,param_1 >> 6);
  }
  else if (param_2 == 0xd) {
    dd_mode_push(5);
  }
  return;
}
#endif

int func_0882192C(int list)
{
    func_088215A0();
    return list;
}

#ifdef NON_MATCHING
// 08821954 size=232
void func_08821954(undefined4 param_1,uint param_2,int param_3,int param_4)
{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  iVar3 = (&D_08C582C0)[param_2 & 0xffff];
  uVar1 = dl_set_color(param_1,0xffffffff);
  uVar1 = dl_set_tex_filter(uVar1,0,1);
  uVar1 = func_088212C4(uVar1,param_3,param_4 + 1,(&D_08C582C0)[param_2 & 0xffff]);
  font_set_size(0xc,0xc);
  uVar2 = func_08821C90(*(undefined4 *)(iVar3 * 0x10 + 0x8876118));
  font_text_set(uVar1);
  font_set_color(0xff000000);
  font_draw_text((param_3 + 0xf) * 0x40,(param_4 + 1) * 0x40,(0x1e0 - (param_3 + 0xf)) * 0x40,uVar2)
  ;
  font_text_clear();
  return;
}
#endif

#ifdef NON_MATCHING
// 08821a3c size=368
uint func_08821A3C(ushort param_1)
{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  byte local_4;
  local_4 = (byte)param_1;
  uVar6 = (local_4 & 0x3f) >> 1;
  uVar5 = local_4 & 1;
  uVar1 = dv_08835AEC(uVar5,uVar6,param_1 >> 6);
  if (uVar6 == 0xc) {
    iVar2 = dv_zone_c_uid(uVar5,0);
    param_1 = param_1 & 0x3f;
  }
  else if (uVar6 == 0xd) {
    iVar2 = dv_zone_d_uid(uVar5,0);
    param_1 = param_1 & 0x3f;
  }
  else if (uVar6 == 0xf) {
    iVar2 = 0;
  }
  else if (uVar6 == 0xe) {
    iVar2 = 0;
  }
  else if (uVar6 == 0xb) {
    iVar2 = dv_hand_uid(uVar5,param_1 >> 6);
  }
  else {
    iVar2 = dv_slot_uid(uVar5,uVar6);
  }
  if ((0 < iVar2) && (iVar3 = func_08806E58((int)(short)param_1), iVar3 != 0)) {
    uVar1 = uVar1 | 0x2000;
    uVar4 = duel_088FEA90(iVar2);
    dd_mode_set_card(uVar4);
  }
  return uVar1;
}
#endif

#ifdef NON_MATCHING
// 08821bac size=228
bool func_08821BAC(void)
{
  int iVar1;
  int iVar2;
  int unaff_s0;
  undefined4 *puVar3;
  puVar3 = &D_08C582B8;
  iVar1 = dd_pad();
  iVar2 = dd_input_on();
  if (iVar2 != 0) {
    if ((((D_08C5834C & 0x1000) != 0) && ((*(uint *)(iVar1 + 4) & 0x200) != 0)) &&
       (*(char *)(D_08C58344 + 0xe9) == '\x03')) {
      iVar2 = 0;
      iVar1 = unaff_s0;
      if (0 < D_08C582BC) {
        do {
          iVar1 = iVar2;
          if (puVar3[2] == 0xc) break;
          iVar2 = iVar2 + 1;
          puVar3 = puVar3 + 1;
          iVar1 = unaff_s0;
        } while (iVar2 < D_08C582BC);
      }
      menu_set_cursor(D_08C58344,iVar1);
      se_play(0x6213);
    }
    D_08C58348 = menu_update(D_08C58344);
  }
  iVar1 = func_08825728();
  return iVar1 == 0;
}
#endif

#ifdef NON_MATCHING
// 08821c90 size=144
void func_08821C90(int param_1)
{
  int iVar1;
  if ((param_1 == 0x41) && ((D_08C5834C & 0xc0) != 0xc0)) {
    param_1 = 3;
  }
  if (((param_1 == 0x40) && (iVar1 = card_is_monster(D_08C58350), iVar1 == 0)) &&
     ((D_08C5834C & 0xc0) != 0xc0)) {
    param_1 = 3;
  }
  dd_text(param_1);
  return;
}
#endif

#ifdef NON_MATCHING
// 08821d20 size=156
void func_08821D20(undefined4 param_1,undefined4 param_2)
{
  memset(&D_08C58354,0,0xc);
  D_08C58354 = param_2;
  func_08825824(param_1,param_1,0x30,func_08821DBC);
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_088259FC(func_08821F70);
  func_088259E4(&func_08821FA0);
  func_088259F0(func_08821F38);
  se_play(0x6216);
  return;
}
#endif

#ifdef NON_MATCHING
// 08821dbc size=380
void func_08821DBC(int param_1,int param_2)
{
  byte bVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  if (param_2 == 0) {
    font_set_size(*(undefined1 *)(param_1 + 0x98),*(undefined1 *)(param_1 + 0x98));
    iVar5 = 0;
    do {
      iVar2 = dd_text(iVar5 + 0xe);
      if (iVar2 != 0) {
        font_measure_text(iVar2);
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < 5);
    iVar5 = (int)*(short *)(param_1 + 0x90) + (int)*(short *)(param_1 + 0x92) + 8;
    if (*(short *)(param_1 + 0x6c) < iVar5) {
      *(short *)(param_1 + 0x6c) = (short)iVar5;
      bVar1 = *(byte *)(param_1 + 0xd2);
    }
    else {
      bVar1 = *(byte *)(param_1 + 0xd2);
    }
    func_08821F38(param_1,0,(uint)*(ushort *)(param_1 + 0xd0) + (bVar1 & 0x1f));
  }
  else {
    uVar4 = 0;
    uVar3 = 1;
    while (((D_08C58354 & uVar3) == 0 && (uVar4 = uVar4 + 1, (int)uVar4 < 6))) {
      uVar3 = 1 << (uVar4 & 0x1f);
    }
    *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xfb;
    *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xdf | 0x20;
    *(char *)(param_1 + 0xd4) = (char)uVar4;
    *(undefined2 *)(param_1 + 0xcc) = 6;
    *(undefined1 *)(param_1 + 0xce) = 6;
    *(uint *)(param_1 + 0xdc) = ~D_08C58354;
    *(byte *)(param_1 + 0xc9) = *(byte *)(param_1 + 0xc9) & 0xfe | 1;
    *(undefined2 *)(param_1 + 0xf0) = 0x28;
    *(undefined2 *)(param_1 + 0xf2) = 0x28;
    *(code **)(param_1 + 0xfc) = func_08821FA8;
    *(undefined2 *)(param_1 + 0xf4) = 5;
    *(undefined2 *)(param_1 + 0xf8) = 0x1e;
    *(undefined2 *)(param_1 + 0xfa) = 0x28;
    D_08C5835C = param_1;
  }
  return;
}
#endif

void func_08821F38(int a, int b, int i)
{
    func_08825AFC(dd_text(i + 0xE));
    D_08C58358 = i;
}

#ifdef NON_MATCHING
void func_08821F70(void)
{
    if (func_08825A14() >= 0)
        dv_w624_set();
}
#endif

void *func_08821FA0(void *list)
{
    return list;
}

#ifdef NON_MATCHING
// 08821fa8 size=420
undefined4 func_08821FA8(undefined4 param_1,uint param_2,int param_3,int param_4)
{
  undefined4 uVar1;
  int iVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  undefined4 uVar6;
  uVar4 = param_2 & 0xffff;
  uVar5 = D_08C58354 & 1 << (param_2 & 0x1f);
  uVar1 = dl_set_color(param_1,0xffffffff);
  uVar1 = dl_set_tex_filter(uVar1,0,1);
  iVar2 = func_088044B0(0x40025);
  if (iVar2 != 0) {
    uVar1 = gim_set_texture(uVar1,iVar2);
  }
  if (uVar5 == 0) {
    uVar6 = 0x20;
  }
  else {
    uVar6 = 0;
  }
  uVar1 = draw_set_offsets(uVar1,0x14,0x20,0x14,0x20);
  uVar1 = draw_sprite(uVar1,param_3 + 10,param_4 + 4,0,uVar4 * 0x14,uVar6,0,0);
  if (uVar4 != 0) {
    if (uVar4 - 1 == (uint)*(byte *)(D_08C5835C + 0xd4)) {
      puVar3 = (uint *)gfx_get_state();
      uVar4 = ((uint)((ulonglong)*puVar3 * 0x88888889 >> 0x20) & 0xf) >> 3;
    }
    else {
      uVar4 = 1;
    }
    uVar1 = draw_set_offsets(uVar1,7,0xb,7,0xb);
    uVar1 = draw_sprite(uVar1,param_3 + -3,param_4 + 0xf,0,0x78,uVar4 << 5,0,0);
  }
  return uVar1;
}
#endif

#ifdef NON_MATCHING
// 0882214c size=120
void func_0882214C(undefined4 param_1)
{
  memset(&D_08C58360,0,0x4c);
  func_08822520();
  func_08825824(0,param_1,3,&func_088221C4);
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_088259FC(func_08822240);
  se_play(0x621a);
  return;
}
#endif

#ifdef NON_MATCHING
/* menu setup hook of the message window kind 2 (D_08C58360) */
void func_088221C4(MenuWidget *w, int open)
{
    int *st = (int *)&D_08C58360;

    if (open == 0)
        return;
    st[0] = (int)w;
    w->cancel_blocked = 0;
    w->cancel_resets = 1;
    w->initial = 3;
    w->draw_before = (MenuDrawRectFn)func_0882228C;
    if (st[0x44 / 4] > 0)
        w->def.h = 0x28;
    else
        w->def.h = 2;
}
#endif

#ifdef NON_MATCHING
void func_08822240(void)
{
    int r = func_08825A14();
    int v;

    if (r == 2)
        v = 1;
    else if (r == 1)
        v = 1;
    else
        v = 0;
    dv_w624_set(v);
}
#endif

#ifdef NON_MATCHING
// 0882228c size=660
void func_0882228C(undefined4 param_1)
{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined4 *puVar8;
  int iVar9;
  undefined2 local_8;
  undefined2 local_6;
  undefined2 local_4;
  undefined2 local_2;
  puVar8 = &D_08C58360;
  iVar4 = (int)*(short *)(D_08C58360 + 0x6c);
  iVar1 = iVar4;
  if (iVar4 < 0) {
    iVar1 = iVar4 + 1;
  }
  iVar6 = (int)*(short *)(D_08C58360 + 0x58) + (iVar1 >> 1);
  iVar5 = (int)*(short *)(D_08C58360 + 0x5a) + (int)*(short *)(D_08C58360 + 0x6e);
  iVar1 = (iVar4 + -9) / 0x16;
  iVar4 = D_08C583A4;
  iVar9 = D_08C583A8;
  if (iVar1 < D_08C583A4) {
    iVar9 = 1;
    iVar4 = iVar1;
  }
  uVar2 = dl_enable(param_1,9);
  uVar2 = dl_set_color(uVar2,0xffffffff);
  uVar2 = dl_disable(uVar2,4);
  iVar1 = iVar4;
  if (iVar9 != 0) {
    iVar1 = iVar4 + -1;
  }
  iVar7 = 0;
  if (0 < iVar1) {
    do {
      uVar3 = duel_088FEA90(puVar8[1]);
      uVar2 = func_088046F0(uVar2,uVar3);
      uVar2 = draw_set_offsets(uVar2,0x40,0x40,0x14,0x1e);
      uVar2 = draw_sprite(uVar2,(int)((float)iVar6 + ((float)iVar7 - (float)iVar4 / 2.0) * 22.0 +
                                     1.0),iVar5 + -0x28,0,0,0,0,0);
      iVar7 = iVar7 + 1;
      puVar8 = puVar8 + 1;
    } while (iVar7 < iVar1);
  }
  uVar2 = dl_enable(uVar2,4);
  if (iVar9 != 0) {
    local_8 = D_08876200;
    local_6 = D_08876202;
    local_4 = D_08876204;
    local_2 = D_08876206;
    iVar1 = (int)((float)iVar6 + ((float)iVar7 - (float)iVar4 / 2.0) * 22.0 + 1.0);
    font_text_set(uVar2);
    font_set_size(6,0xc);
    font_set_color(0xff000000);
    font_draw_text(iVar1 << 6,(iVar5 + -0x1f) * 0x40,(0x1e0 - iVar1) * 0x40,&local_8);
    font_text_clear();
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08822520 size=416
void func_08822520(void)
{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  D_08C583A4 = 0;
  D_08C583A8 = 0;
  iVar1 = duel_088FEED0();
  if (*(int *)(iVar1 + 0x484) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = iVar1 + (*(int *)(iVar1 + 0x484) + -1) * 0x18 + 0x300;
  }
  iVar4 = 0;
  do {
    iVar5 = 0;
    do {
      iVar2 = dv_088695B8(0,iVar4,iVar5,iVar1);
      if (iVar2 != 0) {
        uVar3 = dv_slot_uid(iVar4,iVar5);
        (&D_08C58364)[D_08C583A4] = uVar3;
        D_08C583A4 = D_08C583A4 + 1;
        if (0xf < D_08C583A4) goto D_088226A4;
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < 0xb);
    iVar4 = iVar4 + 1;
  } while (iVar4 < 2);
  iVar4 = dv_n_hand(0);
  iVar5 = 0;
  if (0 < iVar4) {
    do {
      iVar2 = dv_08869728(0,iVar5,iVar1);
      if (iVar2 != 0) {
        uVar3 = dv_hand_uid(0,iVar5);
        (&D_08C58364)[D_08C583A4] = uVar3;
        D_08C583A4 = D_08C583A4 + 1;
        if (0xf < D_08C583A4) goto D_088226A4;
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < iVar4);
  }
  iVar1 = dv_088BFFD8(0,0xfffe,0xe);
  if (iVar1 != 0) {
D_088226A4:
    D_08C583A8 = 1;
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 088226c0 size=128
void func_088226C0(undefined4 param_1,int param_2,undefined4 param_3)
{
  D_08C583AC = param_3;
  if (param_2 == 0) {
    func_08825824(0,param_1,1,&func_08822740);
  }
  else {
    func_08825824(0,param_1,3,&func_08822740);
  }
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_088259FC(func_088227EC);
  se_play(0x621a);
  return;
}
#endif

#ifdef NON_MATCHING
/* menu setup hook of the yes/no window */
void func_08822740(MenuWidget *w, int open)
{
    if (open != 0) {
        w->cancel_blocked = 0;
        w->cancel_resets = 1;
        if (w->buttons == 1)
            w->initial = 1;
        else
            w->initial = 3;
        w->def.pad_b = 0;
    } else if (D_08C583AC != 0) {
        w->cursor = 1;
        w->button = 3;
    }
}
#endif

#ifdef NON_MATCHING
void func_088227EC(void)
{
    int r = func_08825A14();
    int v;

    if (r == 2)
        v = 1;
    else if (r == 1)
        v = 1;
    else
        v = 0;
    dv_w624_set(v);
}
#endif

#ifdef NON_MATCHING
// 08822838 size=128
void func_08822838(undefined4 param_1,undefined2 param_2,undefined4 param_3)
{
  D_08C583B4 = duel_088FEA90(param_2);
  D_08C583B0 = param_3;
  func_08825824(0,param_1,0x30,&func_088228B8);
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_088259FC(func_088228F0);
  se_play(0x621a);
  return;
}
#endif

void func_088228B8(MenuWidget *w, int open)
{
    if (open == 0)
        return;
    w->count = 2;
    w->rows = 2;
    w->item_w = 0x50;
    w->item_h = 0x5A;
    w->draw_item = (MenuDrawItemFn)func_08822914;
}

#ifdef NON_MATCHING
void func_088228F0(void)
{
    dv_w624_set(func_08825A14() != 0);
}
#endif

#ifdef NON_MATCHING
// 08822914 size=504
undefined4 func_08822914(undefined4 param_1,short param_2,int param_3,int param_4)
{
  undefined2 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  if (param_2 != 0) {
    iVar6 = 0x12;
    iVar5 = 7;
    uVar7 = 0x2c;
    uVar8 = 0x1f;
    uVar9 = 0x18;
  }
  else {
    iVar6 = 0x19;
    iVar5 = 0;
    uVar7 = 0x1f;
    uVar8 = 0x2c;
    uVar9 = 0x17;
  }
  uVar2 = dl_set_color(param_1,0xffffffff);
  uVar3 = dl_set_tex_filter(uVar2,0,1);
  uVar2 = D_08C583B4;
  if ((param_2 != 0) && (D_08C583B0 == 0)) {
    uVar2 = 0;
  }
  uVar2 = func_088046F0(uVar3,uVar2);
  uVar7 = draw_set_offsets(uVar2,0x40,0x40,uVar7,uVar8);
  uVar7 = draw_sprite(uVar7,param_3 + iVar6,param_4 + 0xc + iVar5,0,0,0,0,param_2 != 0);
  uVar1 = font_get_flags();
  font_text_set(uVar7);
  font_set_size(0xc,0xc);
  iVar5 = cfg_get_language();
  if (iVar5 != 0) {
    font_set_flag8(1);
  }
  uVar7 = dd_text(uVar9);
  uVar4 = font_measure_wtext2(0x1400,0x600,uVar7);
  iVar5 = -((uVar4 & 0xffff) >> 6);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 1;
  }
  font_set_color(0xff000000);
  font_draw_wtext_ex((param_3 + (0x28 - ((int)(uVar4 >> 0x16) >> 1))) * 0x40,
                 (param_4 + 0x4a + (iVar5 >> 1)) * 0x40,0x1400,0x600,uVar7,0xffffffff);
  uVar7 = font_text_clear();
  font_set_flags(uVar1);
  return uVar7;
}
#endif
