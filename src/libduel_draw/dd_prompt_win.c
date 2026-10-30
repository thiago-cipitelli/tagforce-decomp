// @range 0x08823674-0x08825088
// @asm 0x08823674-0x08823808
// @asm 0x0882382C-0x08823D84
// @asm 0x08823D8C-0x08823FF4
// @asm 0x0882401C-0x08824D18
// @asm 0x08824D7C-0x08825088
/* Prompt windows of the message kinds 3-10: card choice strips, attribute/type/number pickers, coin and dice choices, with their draw and menu hooks.
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

int func_08824D7C();
int func_08825A14();
int func_0882A294();
int func_08866988();
extern int D_08C5881C;
extern int D_08C58828;
void func_08823808(void);
void *func_08823D84(void *list);
void func_08823FF4(void);
void func_08824D18(MenuWidget *w, int open);
void func_08824D58(void);

#ifdef NON_MATCHING
// 08823674 size=176
void func_08823674(undefined4 param_1)
{
  undefined4 uVar1;
  D_08C58810 = 0;
  uVar1 = dd_text(0x1c);
  func_08825824(uVar1,param_1,0x30,func_08823724);
  if ((D_08C58814 == 0) || (D_08C58810 != 0)) {
    func_088259D8();
    duel_088FEE30(0);
  }
  else {
    dd_mode_push(4);
    dd_mode_set_flag(0x31);
    func_088259FC(func_08823808);
    func_08825A08(func_08823D8C);
    se_play(0x621a);
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08823724 size=228
void func_08823724(int param_1,int param_2)
{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  if (param_2 != 0) {
    iVar1 = duel_088FEE68();
    uVar5 = 0;
    iVar4 = 0;
    iVar2 = duel_088FEE68();
    iVar2 = *(int *)(iVar2 + 0xc);
    uVar3 = 0;
    if (0 < iVar2) {
      do {
        if ((int)((uint)*(byte *)(iVar1 + 0x11) << 0x19) < 0) {
          iVar4 = iVar4 + 1;
        }
        else {
          uVar5 = uVar5 | 1 << (uVar3 & 0x1f);
        }
        uVar3 = uVar3 + 1;
        iVar1 = iVar1 + 4;
      } while ((int)uVar3 < iVar2);
    }
    *(undefined2 *)(param_1 + 0x96) = 0x50;
    *(short *)(param_1 + 0xcc) = (short)iVar2;
    *(char *)(param_1 + 0xce) = (char)iVar2;
    *(uint *)(param_1 + 0xdc) = uVar5;
    *(undefined2 *)(param_1 + 0xf0) = 0x46;
    *(undefined2 *)(param_1 + 0xf2) = 0x12;
    *(code **)(param_1 + 0xfc) = func_0882382C;
    *(undefined1 **)(param_1 + 0x10c) = &func_08823D84;
    D_08C58814 = param_1;
    if (iVar4 == 0) {
      D_08C58810 = 1;
    }
  }
  return;
}
#endif

void func_08823808(void)
{
    func_08866988(func_08825A14());
}

#ifdef NON_MATCHING
// 0882382c size=1368
void func_0882382C(undefined4 param_1,uint param_2,int param_3,int param_4)
{
  ushort uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined1 auStack_e0 [128];
  float local_60;
  float local_5c;
  float local_54;
  float local_50;
  float local_48;
  float local_44;
  float local_3c;
  float local_38;
  iVar5 = D_08C58814;
  param_2 = param_2 & 0xffff;
  iVar8 = 0;
  if (param_2 == 0) {
    iVar6 = (uint)*(ushort *)(D_08C58814 + 0xf0) * (uint)*(ushort *)(D_08C58814 + 0xcc) + -9;
    iVar7 = ((((int)*(short *)(D_08C58814 + 0x5a) + (int)*(short *)(D_08C58814 + 0x6e)) -
             (int)*(short *)(D_08C58814 + 0x94)) + -0x1d) - (param_4 + 3);
    local_60 = (float)(param_3 + 4);
    local_5c = (float)(param_4 + 0x16);
    local_54 = local_60 + (float)iVar6;
    local_44 = local_5c + (float)iVar7;
    local_50 = local_5c;
    local_48 = local_54;
    local_3c = local_60;
    local_38 = local_44;
    uVar2 = dl_set_color(param_1,0xffffffff);
    uVar2 = func_08828DD0(uVar2,0,4,&local_60,1);
    font_text_set(uVar2);
    font_set_size(0xc,0xc);
    iVar3 = duel_088FEE68();
    iVar3 = iVar3 + (*(byte *)(D_08C58814 + 0xd2) & 0x1f) * 4;
    uVar2 = dlg_get_text(*(ushort *)(iVar3 + 0x10) & 0x1fff);
    if ((int)((uint)*(byte *)(iVar3 + 0x11) << 0x19) < 0) {
      font_set_color(0xff000000);
    }
    else {
      font_set_color(0xff666666);
    }
    font_draw_wtext_ex((int)local_60 << 6,(int)local_5c << 6,iVar6 * 0x40,iVar7 * 0x40,uVar2,0xffffffff)
    ;
    param_1 = font_text_clear();
  }
  iVar3 = func_088044B0(0x4001a);
  if (iVar3 != 0) {
    param_1 = gim_set_texture(param_1,iVar3);
  }
  if ((D_08C58814 == 0) || ((*(byte *)(D_08C58814 + 0xd2) & 0x1f) != param_2)) {
    uVar2 = dl_set_color(param_1,0xff808080);
    iVar8 = 2;
    uVar2 = draw_set_offsets(uVar2,8,0x12,8,0x12);
    if (param_2 == 0) {
      uVar2 = draw_sprite(uVar2,param_3,param_4 + 3,0,8,0,0,0);
      uVar1 = *(ushort *)(iVar5 + 0xcc);
    }
    else {
      uVar2 = draw_sprite(uVar2,param_3,param_4 + 3,0,0x18,0,0,0);
      uVar1 = *(ushort *)(iVar5 + 0xcc);
    }
    if (param_2 == uVar1 - 1) {
      uVar2 = draw_sprite(uVar2,param_3 + (uint)*(ushort *)(iVar5 + 0xf0) + -8,param_4 + 3,0,0x38,0,
                          0,0);
      uVar1 = *(ushort *)(iVar5 + 0xf0);
    }
    else {
      uVar2 = draw_sprite(uVar2,param_3 + (uint)*(ushort *)(iVar5 + 0xf0) + -8,param_4 + 3,0,0x28,0,
                          0,0);
      uVar1 = *(ushort *)(iVar5 + 0xf0);
    }
    uVar2 = draw_set_offsets(uVar2,1,0x12,uVar1 - 0x10,0x12);
    uVar2 = draw_sprite(uVar2,param_3 + 8,param_4 + 3,0,0xc,0,0,0);
  }
  else {
    uVar2 = dl_set_color(param_1,0xffffffff);
    uVar2 = draw_set_offsets(uVar2,8,0x12,8,0x12);
    if (param_2 == 0) {
      uVar2 = draw_sprite(uVar2,param_3,param_4 + 3,0,0,0,0,0);
      uVar1 = *(ushort *)(iVar5 + 0xcc);
    }
    else {
      uVar2 = draw_sprite(uVar2,param_3,param_4 + 3,0,0x10,0,0,0);
      uVar1 = *(ushort *)(iVar5 + 0xcc);
    }
    if (param_2 == uVar1 - 1) {
      uVar2 = draw_sprite(uVar2,param_3 + (uint)*(ushort *)(iVar5 + 0xf0) + -8,param_4 + 3,0,0x30,0,
                          0,0);
      uVar1 = *(ushort *)(iVar5 + 0xf0);
    }
    else {
      uVar2 = draw_sprite(uVar2,param_3 + (uint)*(ushort *)(iVar5 + 0xf0) + -8,param_4 + 3,0,0x20,0,
                          0,0);
      uVar1 = *(ushort *)(iVar5 + 0xf0);
    }
    uVar2 = draw_set_offsets(uVar2,1,0x12,uVar1 - 0x10,0x12);
    uVar2 = draw_sprite(uVar2,param_3 + 8,param_4 + 3,0,4,0,0,0);
  }
  uVar4 = dd_text(0x1b);
  wsprintf(auStack_e0,uVar4,param_2 + 1);
  font_set_size(0xc,0xc);
  iVar3 = font_measure_text(auStack_e0);
  iVar6 = font_line_height();
  iVar7 = iVar6 >> 7;
  if (iVar6 >> 6 < 0) {
    iVar7 = (iVar6 >> 6) + 1 >> 1;
  }
  iVar8 = iVar8 + (9 - iVar7);
  iVar6 = param_4 + 3 + iVar8;
  iVar5 = (uint)*(ushort *)(iVar5 + 0xf0) - (iVar3 >> 6);
  if (iVar5 < 0) {
    iVar5 = iVar5 + 1;
  }
  param_3 = param_3 + (iVar5 >> 1);
  uVar2 = dl_scissor(uVar2,param_3,iVar6,(iVar3 >> 6) + 1,0xf - iVar8);
  uVar2 = dl_enable(uVar2,2);
  font_set_color(0xff00354a);
  font_text_set(uVar2);
  font_draw_text(param_3 * 0x40,iVar6 * 0x40,(0x1e0 - param_3) * 0x40,auStack_e0);
  uVar2 = font_text_clear();
  dl_disable(uVar2,2);
  return;
}
#endif

void *func_08823D84(void *list)
{
    return list;
}

#ifdef NON_MATCHING
// 08823d8c size=168
bool func_08823D8C(void)
{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  iVar4 = D_08C58814;
  iVar2 = dd_input_on();
  if (iVar2 == 0) {
    cVar1 = *(char *)(iVar4 + 0xe9);
  }
  else {
    uVar3 = menu_update(iVar4);
    func_08825A70(uVar3);
    cVar1 = *(char *)(iVar4 + 0xe9);
  }
  if (cVar1 == '\x03') {
    iVar2 = dd_pad();
    if ((*(uint *)(iVar2 + 4) & 0x100) == 0) {
      if ((*(uint *)(iVar2 + 4) & 0x200) != 0) {
        menu_cursor_next(iVar4);
        se_play(0x6212);
      }
    }
    else {
      menu_cursor_prev(iVar4);
      se_play(0x6212);
    }
  }
  iVar4 = func_08825728();
  return iVar4 == 0;
}
#endif

#ifdef NON_MATCHING
// 08823e34 size=272
void func_08823E34(undefined4 param_1,undefined4 param_2,undefined4 param_3)
{
  undefined4 uVar1;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  code *local_1c;
  code *local_18;
  code *local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  uVar1 = dd_text(0x1c);
  D_08C58818 = param_3;
  memset(&local_2c,0,0x2c);
  local_2c = 1;
  local_28 = 7;
  local_24 = 6;
  local_20 = dd_text(0x1f);
  local_1c = dlg_table_sel;
  local_18 = func_08824374;
  local_14 = func_08824484;
  local_10 = 0x19;
  local_c = 0x19;
  local_8 = param_2;
  local_4 = param_2;
  func_088299D0(&local_2c);
  func_08825824(uVar1,param_1,0x30,func_08823F44);
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_08825A08(func_0882401C);
  func_088259FC(func_08823FF4);
  se_play(0x621a);
  return;
}
#endif

#ifdef NON_MATCHING
// 08823f44 size=176
void func_08823F44(int param_1,int param_2)
{
  byte bVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  if (param_2 == 0) {
    func_08829BA8();
  }
  else {
    D_08C5881C = param_1;
    func_08829AB4();
    *(code **)(param_1 + 0x100) = func_0882404C;
    *(int *)(param_1 + 0xdc) = ~(D_08C58818 >> 1);
    *(undefined4 *)(param_1 + 0x60) = 0;
    sVar2 = *(short *)(param_1 + 0x90);
    sVar3 = *(short *)(param_1 + 0x92);
    sVar4 = func_0882A1EC(param_1);
    *(short *)(param_1 + 0x6c) = sVar2 + sVar3 + sVar4 + 0x5c;
    sVar2 = *(short *)(param_1 + 0x94);
    sVar3 = *(short *)(param_1 + 0x96);
    bVar1 = *(byte *)(param_1 + 0x98);
    sVar4 = func_0882A22C(param_1);
    *(ushort *)(param_1 + 0x6e) = (ushort)bVar1 + sVar2 + sVar3 + sVar4 + 0x2a;
  }
  return;
}
#endif

void func_08823FF4(void)
{
    func_08866988(func_0882A294(D_08C5881C));
}

#ifdef NON_MATCHING
int func_0882401C(void)
{
    func_08829E2C(D_08C5881C);
    return func_08825728() == 0;
}
#endif

#ifdef NON_MATCHING
// 0882404c size=808
void func_0882404C(undefined4 param_1,int param_2,int param_3)
{
  float *pfVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  float *pfVar5;
  float *pfVar6;
  float fVar7;
  float fVar8;
  float local_48;
  float local_44;
  float local_3c;
  float local_38;
  float local_30;
  float local_2c;
  float local_24;
  float local_20;
  float local_18;
  float local_14;
  float local_c;
  float local_8;
  iVar2 = func_0882A1EC(D_08C5881C);
  iVar3 = func_0882A22C(D_08C5881C);
  pfVar5 = (float *)0x48;
  pfVar6 = &local_48;
  pfVar1 = pfVar6;
  while (pfVar1 != (float *)0x0) {
    *(undefined1 *)pfVar6 = 0;
    pfVar6 = (float *)((int)pfVar6 + 1);
    pfVar5 = (float *)((int)pfVar5 + -1);
    pfVar1 = pfVar5;
  }
  local_48 = (float)(param_2 + 0x1c);
  local_44 = (float)(param_3 + 7);
  local_3c = (float)(param_2 + 0x2d + iVar2);
  local_2c = (float)(param_3 + 0x1a + iVar3);
  local_24 = (float)(param_2 + 0x2d);
  local_14 = (float)(param_3 + 0x18);
  local_38 = local_44;
  local_30 = local_3c;
  local_20 = local_2c;
  local_18 = local_24;
  local_c = local_48;
  local_8 = local_14;
  uVar4 = syspack_get();
  iVar2 = ehp_find(uVar4,s_window01_gim_08876208,0);
  if (iVar2 != 0) {
    uVar4 = dl_enable(param_1,9);
    param_1 = gim_set_texture(uVar4,iVar2);
  }
  fVar8 = local_3c - local_48;
  fVar7 = local_8 - local_44;
  uVar4 = dl_set_color(param_1,0xffffffff);
  uVar4 = draw_set_offsets(uVar4,1,1,(int)(fVar8 + 2.0),(int)(fVar7 + 1.0));
  uVar4 = draw_sprite(uVar4,(int)(local_48 - 1.0),(int)(local_44 - 1.0),0,0x36,0x65,0,0);
  uVar4 = func_08828DD0(uVar4,0,6,&local_48,1);
  iVar2 = func_088044B0(0x40005);
  if (iVar2 != 0) {
    uVar4 = gim_set_texture(uVar4,iVar2);
  }
  uVar4 = draw_set_offsets(uVar4,0x11,0x11,0x11,0x11);
  uVar4 = draw_sprite(uVar4,(int)local_48,(int)local_44,0,0x55,0x20,0,0);
  iVar2 = dd_text(0x1d);
  if (iVar2 != 0) {
    font_set_size(0xc,0xc);
    font_set_color(0xff202020);
    font_text_set(uVar4);
    iVar3 = (int)(local_48 + 17.0 + 3.0);
    font_draw_text(iVar3 << 6,(int)(local_44 + 2.0) << 6,(0x1e0 - iVar3) * 0x40,iVar2);
    uVar4 = font_text_clear();
  }
  func_08829BBC(uVar4,D_08C5881C,param_2 + 0x2d,param_3 + 0x18,0xffffffff,0xffffffff);
  return;
}
#endif

#ifdef NON_MATCHING
// 08824374 size=272
void func_08824374(undefined4 param_1,undefined4 param_2,undefined4 param_3,uint param_4)
{
  int iVar1;
  undefined4 uVar2;
  iVar1 = func_088044B0(0x40004);
  if (iVar1 != 0) {
    uVar2 = dl_enable(param_1,9);
    param_1 = gim_set_texture(uVar2,iVar1);
  }
  if ((1 << (param_4 & 0x1f) & D_08C58818) == 0) {
    uVar2 = dl_set_color(param_1,0x80808080);
  }
  else {
    uVar2 = dl_set_color(param_1,0xffffffff);
  }
  uVar2 = draw_set_offsets(uVar2,0x11,0x11,0x11,0x11);
  draw_sprite(uVar2,param_2,param_3,0,(param_4 - 1) * 0x11,0x44,0,0);
  return;
}
#endif

#ifdef NON_MATCHING
// 08824484 size=192
void func_08824484(undefined4 param_1,undefined4 param_2,undefined4 param_3)
{
  int iVar1;
  undefined4 uVar2;
  iVar1 = func_088044B0(0x40005);
  if (iVar1 != 0) {
    uVar2 = dl_enable(param_1,9);
    param_1 = gim_set_texture(uVar2,iVar1);
  }
  uVar2 = dl_set_color(param_1,0xffffffff);
  uVar2 = draw_set_offsets(uVar2,0x19,0x18,0x19,0x18);
  draw_sprite(uVar2,param_2,param_3,0,0x1c,0x5d,0,0);
  return;
}
#endif

#ifdef NON_MATCHING
// 08824544 size=264
void func_08824544(undefined4 param_1,undefined4 param_2)
{
  undefined4 uVar1;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  code *local_1c;
  code *local_18;
  code *local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  uVar1 = dd_text(0x1c);
  memset(&local_2c,0,0x2c);
  local_2c = 1;
  local_28 = 0x15;
  local_24 = 5;
  local_20 = dd_text(0x1f);
  local_1c = card_db_table_sel;
  local_18 = func_08824ABC;
  local_14 = func_08824C08;
  local_10 = 0x19;
  local_c = 0x19;
  local_8 = 1;
  local_4 = 1;
  func_088299D0(&local_2c);
  D_08C58824 = param_2;
  func_08825824(uVar1,param_1,0x30,func_0882464C);
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_08825A08(func_08824764);
  func_088259FC(func_088246FC);
  se_play(0x621a);
  return;
}
#endif

#ifdef NON_MATCHING
// 0882464c size=176
void func_0882464C(int param_1,int param_2)
{
  byte bVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  if (param_2 == 0) {
    func_08829BA8();
  }
  else {
    D_08C58820 = param_1;
    func_08829AB4();
    *(code **)(param_1 + 0x100) = func_08824794;
    *(uint *)(param_1 + 0xdc) = ~(D_08C58824 >> 1);
    *(undefined4 *)(param_1 + 0x60) = 0;
    sVar2 = *(short *)(param_1 + 0x90);
    sVar3 = *(short *)(param_1 + 0x92);
    sVar4 = func_0882A1EC(param_1);
    *(short *)(param_1 + 0x6c) = sVar2 + sVar3 + sVar4 + 0x5c;
    sVar2 = *(short *)(param_1 + 0x94);
    sVar3 = *(short *)(param_1 + 0x96);
    bVar1 = *(byte *)(param_1 + 0x98);
    sVar4 = func_0882A22C(param_1);
    *(ushort *)(param_1 + 0x6e) = (ushort)bVar1 + sVar2 + sVar3 + sVar4 + 0x2a;
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 088246fc size=104
void func_088246FC(void)
{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uVar1 = func_0882A294(D_08C58820);
  uVar3 = 1;
  uVar2 = 2;
  while ((uVar4 = uVar3, (uVar1 & uVar2) == 0 && (uVar3 = uVar3 + 1, uVar4 = 0, (int)uVar3 < 0x15)))
  {
    uVar2 = 1 << (uVar3 & 0x1f);
  }
  duel_088FEE30(uVar4);
  return;
}
#endif

#ifdef NON_MATCHING
int func_08824764(void)
{
    func_08829E2C(D_08C58820);
    return func_08825728() == 0;
}
#endif

#ifdef NON_MATCHING
// 08824794 size=808
void func_08824794(undefined4 param_1,int param_2,int param_3)
{
  float *pfVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  float *pfVar5;
  float *pfVar6;
  float fVar7;
  float fVar8;
  float local_48;
  float local_44;
  float local_3c;
  float local_38;
  float local_30;
  float local_2c;
  float local_24;
  float local_20;
  float local_18;
  float local_14;
  float local_c;
  float local_8;
  iVar2 = func_0882A1EC(D_08C58820);
  iVar3 = func_0882A22C(D_08C58820);
  pfVar5 = (float *)0x48;
  pfVar6 = &local_48;
  pfVar1 = pfVar6;
  while (pfVar1 != (float *)0x0) {
    *(undefined1 *)pfVar6 = 0;
    pfVar6 = (float *)((int)pfVar6 + 1);
    pfVar5 = (float *)((int)pfVar5 + -1);
    pfVar1 = pfVar5;
  }
  local_48 = (float)(param_2 + 0x1c);
  local_44 = (float)(param_3 + 7);
  local_3c = (float)(param_2 + 0x2d + iVar2);
  local_2c = (float)(param_3 + 0x1a + iVar3);
  local_24 = (float)(param_2 + 0x2d);
  local_14 = (float)(param_3 + 0x18);
  local_38 = local_44;
  local_30 = local_3c;
  local_20 = local_2c;
  local_18 = local_24;
  local_c = local_48;
  local_8 = local_14;
  uVar4 = syspack_get();
  iVar2 = ehp_find(uVar4,s_window01_gim_08876218,0);
  if (iVar2 != 0) {
    uVar4 = dl_enable(param_1,9);
    param_1 = gim_set_texture(uVar4,iVar2);
  }
  fVar8 = local_3c - local_48;
  fVar7 = local_8 - local_44;
  uVar4 = dl_set_color(param_1,0xffffffff);
  uVar4 = draw_set_offsets(uVar4,1,1,(int)(fVar8 + 2.0),(int)(fVar7 + 1.0));
  uVar4 = draw_sprite(uVar4,(int)(local_48 - 1.0),(int)(local_44 - 1.0),0,0x36,0x65,0,0);
  uVar4 = func_08828DD0(uVar4,0,6,&local_48,1);
  iVar2 = func_088044B0(0x40005);
  if (iVar2 != 0) {
    uVar4 = gim_set_texture(uVar4,iVar2);
  }
  uVar4 = draw_set_offsets(uVar4,0x11,0x11,0x11,0x11);
  uVar4 = draw_sprite(uVar4,(int)local_48,(int)local_44,0,0x66,0x20,0,0);
  iVar2 = dd_text(0x1e);
  if (iVar2 != 0) {
    font_set_size(0xc,0xc);
    font_set_color(0xff202020);
    font_text_set(uVar4);
    iVar3 = (int)(local_48 + 17.0 + 3.0);
    font_draw_text(iVar3 << 6,(int)(local_44 + 2.0) << 6,(0x1e0 - iVar3) * 0x40,iVar2);
    uVar4 = font_text_clear();
  }
  func_08829BBC(uVar4,D_08C58820,param_2 + 0x2d,param_3 + 0x18,0xffffffff,0xffffffff);
  return;
}
#endif

#ifdef NON_MATCHING
// 08824abc size=332
void func_08824ABC(undefined4 param_1,undefined4 param_2,undefined4 param_3,uint param_4)
{
  int iVar1;
  undefined4 uVar2;
  iVar1 = func_088044B0(0x40004);
  if (iVar1 != 0) {
    uVar2 = dl_enable(param_1,9);
    param_1 = gim_set_texture(uVar2,iVar1);
  }
  if ((1 << (param_4 & 0x1f) & D_08C58824) == 0) {
    uVar2 = dl_set_color(param_1,0x80808080);
  }
  else {
    uVar2 = dl_set_color(param_1,0xffffffff);
  }
  uVar2 = draw_set_offsets(uVar2,0x11,0x11,0x11,0x11);
  draw_sprite(uVar2,param_2,param_3,0,((int)(param_4 - 1) % 7) * 0x11,
              ((int)(param_4 - 1) / 7) * 0x11,0,0);
  return;
}
#endif

#ifdef NON_MATCHING
// 08824c08 size=192
void func_08824C08(undefined4 param_1,undefined4 param_2,undefined4 param_3)
{
  int iVar1;
  undefined4 uVar2;
  iVar1 = func_088044B0(0x40005);
  if (iVar1 != 0) {
    uVar2 = dl_enable(param_1,9);
    param_1 = gim_set_texture(uVar2,iVar1);
  }
  uVar2 = dl_set_color(param_1,0xffffffff);
  uVar2 = draw_set_offsets(uVar2,0x19,0x18,0x19,0x18);
  draw_sprite(uVar2,param_2,param_3,0,0x1c,0x5d,0,0);
  return;
}
#endif

#ifdef NON_MATCHING
// 08824cc8 size=80
void func_08824CC8(undefined4 param_1)
{
  func_08825824(0,param_1,0x30,&func_08824D18);
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_088259FC(func_08824D58);
  se_play(0x621a);
  return;
}
#endif

void func_08824D18(MenuWidget *w, int open)
{
    if (open == 0)
        return;
    D_08C58828 = (int)w;
    w->count = 2;
    w->rows = 2;
    w->item_w = 0x64;
    w->item_h = 0x78;
    w->draw_all = (MenuDrawRectFn)func_08824D7C;
}

void func_08824D58(void)
{
    func_08866988(func_08825A14());
}

#ifdef NON_MATCHING
// 08824d7c size=448
void func_08824D7C(undefined4 param_1,int param_2,int param_3)
{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  iVar5 = D_08C58828;
  iVar2 = ((int)*(short *)(D_08C58828 + 0x6c) - (int)*(short *)(D_08C58828 + 0x90)) -
          (int)*(short *)(D_08C58828 + 0x92);
  iVar4 = iVar2 + -0x6d;
  if (iVar4 < 0) {
    iVar4 = iVar2 + -0x6c;
  }
  param_2 = param_2 + (iVar4 >> 1);
  bVar1 = (ushort)(*(short *)(D_08C58828 + 0xd0) + (*(byte *)(D_08C58828 + 0xd2) & 0x1f)) == 0;
  if (bVar1) {
    uVar6 = 0x1a;
  }
  else {
    uVar6 = 0x19;
  }
  uVar3 = menu_draw_cursor(param_1,D_08C58828,param_2 + 4,param_3 + 4,0x5c,0x5c);
  uVar3 = dl_set_color(uVar3,0xffffffff);
  uVar3 = dl_set_tex_filter(uVar3,0,1);
  uVar3 = func_0882A94C(0x3fc00000,uVar3,param_2,param_3,100,100,!bVar1);
  uVar3 = dl_disable(uVar3,1);
  uVar3 = gim_set_texture(uVar3,*(undefined4 *)(iVar5 + 0xe4));
  uVar3 = menu_draw_arrow(uVar3,param_2,param_3 + 0x30,2);
  uVar3 = menu_draw_arrow(uVar3,param_2 + 100,param_3 + 0x30,3);
  font_text_set(uVar3);
  font_set_size(0xc,0xc);
  uVar6 = dd_text(uVar6);
  iVar4 = font_measure_text(uVar6);
  font_set_color(0xff000000);
  iVar5 = iVar4 >> 7;
  if (iVar4 >> 6 < 0) {
    iVar5 = (iVar4 >> 6) + 1 >> 1;
  }
  param_2 = param_2 + (0x32 - iVar5);
  font_draw_text(param_2 * 0x40,(param_3 + 100) * 0x40,(0x1e0 - param_2) * 0x40,uVar6);
  uVar6 = font_text_clear();
  dl_enable(uVar6,1);
  return;
}
#endif

#ifdef NON_MATCHING
// 08824f3c size=180
void func_08824F3C(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5)
{
  undefined4 uVar1;
  uVar1 = dd_text(0x1c);
  D_08C58A2C = param_2;
  D_08C58A30 = param_3;
  D_08C58A34 = param_4;
  D_08C58A38 = param_5;
  func_08825824(uVar1,param_1,0x20,&func_08824FF0);
  dd_mode_push(4);
  dd_mode_set_flag(0x31);
  func_088259FC(func_0882504C);
  se_play(0x621a);
  return;
}
#endif

#ifdef NON_MATCHING
void func_08824FF0(MenuWidget *w, int open)
{
    int *st = (int *)&D_08C58A2C;

    if (open == 0)
        return;
    st[4] = (int)w;
    w->count = (st[1] - st[0]) / st[2] + 1;
    w->rows = 1;
    w->item_w = 0xC8;
    w->item_h = 0x28;
    w->draw_all = (MenuDrawRectFn)func_088250E8;
}
#endif

#ifdef NON_MATCHING
void func_0882504C(void)
{
    func_08866988(D_08C58A2C + func_08825A14() * D_08C58A34);
}
#endif
