// @range 0x08859F18-0x0885F76C
// @asm 0x0885A0D8-0x0885A1C0
// @asm 0x0885A388-0x0885A470
// @asm 0x0885AB38-0x0885AC20
// @asm 0x0885ADE8-0x0885AED0
// @asm 0x0885B098-0x0885B180
// @asm 0x0885B348-0x0885B430
// @asm 0x0885B5F8-0x0885B6E0
// @asm 0x0885B90C-0x0885B9F4
// @asm 0x0885BC90-0x0885BD78
// @asm 0x0885C008-0x0885C0F0
// @asm 0x0885C2B0-0x0885C398
// @asm 0x0885C560-0x0885C648
// @asm 0x0885C810-0x0885C8F8
// @asm 0x0885CAC0-0x0885CBA8
// @asm 0x0885CD68-0x0885CE50
// @asm 0x0885D018-0x0885D100
// @asm 0x0885D32C-0x0885D414
// @asm 0x0885D5DC-0x0885D6C4
// @asm 0x0885D88C-0x0885D974
// @asm 0x0885DBB0-0x0885DC98
// @asm 0x0885DE60-0x0885DF48
// @asm 0x0885E110-0x0885E1F8
// @asm 0x0885E3C0-0x0885E4A8
// @asm 0x0885E654-0x0885E73C
// @asm 0x0885E914-0x0885E9FC
// @asm 0x0885EBC4-0x0885ECAC
// @asm 0x0885EE6C-0x0885EF54
// @asm 0x0885F11C-0x0885F204
// @asm 0x0885F3CC-0x0885F4B4
// @asm 0x0885F67C-0x0885F764
// @data 0x08877980-0x08878CC0
// @rodata 0x08870EC8-0x08871ED0
/* Per-character model handlers of the duel scene (the opponent's 3D model): each block is the
   setup that fills a ChrVt with the character's callbacks, the init/event/update/motion overrides
   (most only forward to the base model), the map from bone slot to bone id and the rest pose of
   the bones (table sorted by id on first use and searched with bsearch). */
#include "libduel_draw.h"

ChrBone chr37_bones[5] = { /* 0x08877980 */
    { 40, -44.264927f, 2.218205f, -11.242474f, { -0.0012936485f, 0.06105304f, -0.0018728677f, 0.99813193f } },
    { 37, -8.0562525f, -2.2415793f, -0.81019276f, { -0.054659177f, 0.703761f, 0.0544678f, 0.70623374f } },
    { 38, -25.317724f, 3.6167965f, -12.1096735f, { -0.000283532f, 0.0004056494f, 0.0010726509f, 0.9999993f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr32_bones[5] = { /* 0x08877A20 */
    { 40, -44.261734f, 2.2172098f, -11.249008f, { -0.0012811306f, 0.06093604f, -0.0018722526f, 0.9981391f } },
    { 37, -8.053032f, -2.2429078f, -0.75736207f, { -0.05456357f, 0.70499855f, 0.054563627f, 0.7049984f } },
    { 38, -25.306566f, 3.6166131f, -12.11399f, { -0.0002387486f, -2.484116e-05f, 0.0010723457f, 0.9999994f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr_kind3_bones[5] = { /* 0x08877AC0 */
    { 40, -44.26263f, 2.2257454f, -11.231956f, { -0.0065024323f, 0.06097078f, -0.0015242406f, 0.9981172f } },
    { 37, -7.794098f, -2.0848489f, -0.7342731f, { -0.055478957f, 0.7049269f, 0.055478945f, 0.7049271f } },
    { 38, -25.366402f, 3.6329231f, -12.072714f, { 8.37e-09f, -1.08e-09f, 0.0022780737f, 0.9999974f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr10_bones[5] = { /* 0x08877B60 */
    { 40, -44.275665f, 2.2243454f, -11.206177f, { -0.0011038929f, 0.061527904f, -0.0018613136f, 0.998103f } },
    { 37, -8.045924f, -2.0097036f, -0.7583708f, { -0.054563582f, 0.7049984f, 0.054563615f, 0.70499855f } },
    { 38, -25.331688f, 3.6230187f, -12.0902605f, { -5.879246e-05f, 0.00056768634f, 0.0022775424f, 0.99999726f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr11_bones[5] = { /* 0x08877C00 */
    { 40, -44.288723f, 2.3274956f, -11.201816f, { -0.0011141466f, 0.061623685f, -0.0018618958f, 0.9980971f } },
    { 37, -7.785797f, -1.9424748f, -0.7573644f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.382761f, 3.6311646f, -12.067847f, { -6.867206e-05f, 0.0006636907f, 0.002277475f, 0.9999972f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr4_bones[5] = { /* 0x08877CA0 */
    { 40, -44.17504f, 2.2398725f, -10.619673f, { -0.00674068f, 0.060945373f, -0.0015389201f, 0.99811715f } },
    { 37, -10.782956f, -3.9173882f, -0.52956915f, { -0.04584575f, 0.7056189f, 0.045845743f, 0.7056191f } },
    { 38, -25.317608f, 3.6154764f, -12.095605f, { -0.00023875154f, -2.483461e-05f, 0.0010723457f, 0.9999994f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr21_bones[5] = { /* 0x08877D40 */
    { 40, -44.26263f, 2.2257524f, -11.231956f, { -0.0065024323f, 0.06097078f, -0.0015242406f, 0.9981172f } },
    { 37, -7.785797f, -1.9791522f, -0.75736433f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.366404f, 3.63293f, -12.072714f, { 8.37e-09f, -1.08e-09f, 0.0022780737f, 0.9999974f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
int chr18_alt_pairs[4] = { 40, 3 }; /* 0x08877DE0 */
ChrBone chr18_alt_bones[5] = { /* 0x08877DF0 */
    { 40, 58.46166f, -9.396748f, -79.40915f, { 0.007261145f, -0.32632694f, 0.0007476742f, 0.94522876f } },
    { 37, -2.3940504f, -0.03579547f, 2.1968362f, { -4.833061e-05f, -0.9156897f, 3.918602e-05f, -0.40188602f } },
    { 38, -8.14016f, 7.8293633f, -23.966043f, { 0.0037036315f, -0.36583713f, -0.012525376f, 0.93058723f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr18_bones[5] = { /* 0x08877E90 */
    { 40, -45.86641f, 2.7453024f, -11.554769f, { -0.006573455f, 0.061632942f, -0.0015248026f, 0.9980761f } },
    { 37, -8.052582f, -2.1561172f, -0.7631584f, { -0.049695164f, 0.70535827f, 0.04969515f, 0.70535845f } },
    { 38, -24.816685f, 4.02618f, -13.432565f, { -6.946164e-05f, 0.00066359976f, 0.001071707f, 0.9999992f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
int chr12_pairs[8] = { 38, 28, 40, 27 }; /* 0x08877F30 */
ChrBone chr12_bones[5] = { /* 0x08877F50 */
    { 40, -0.036218207f, 0.58098155f, -60.227158f, { -0.03441545f, -0.0008502717f, 0.001863528f, 0.9994055f } },
    { 37, -7.785797f, -2.1699655f, -0.75736326f, { -0.054562315f, 0.7049985f, 0.0545623f, 0.7049986f } },
    { 38, 0.46619627f, 3.4009764f, 9.537e-08f, { 0.0f, 0.0f, -0.00010545413f, 1.0f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
int chr9_pairs[8] = { 38, 26, 40, 35 }; /* 0x08877FF0 */
ChrBone chr9_bones[5] = { /* 0x08878010 */
    { 40, -79.81217f, -83.121346f, 1.7199306f, { 0.25402498f, 0.66014487f, 0.6587926f, 0.2562663f } },
    { 37, -6.973999f, -2.0265183f, -0.62047076f, { -0.05456359f, 0.70499855f, 0.054563608f, 0.7049984f } },
    { 38, 6.1808763f, 8.236723f, 6.6636043f, { 0.59350586f, 0.13978745f, -0.1216937f, 0.7831992f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr26_bones[5] = { /* 0x088780B0 */
    { 40, -44.35683f, 2.499217f, -11.173527f, { -0.0011798928f, 0.06224206f, -0.0018645945f, 0.9980586f } },
    { 37, -9.052797f, -2.737388f, -0.7504136f, { -0.05456475f, 0.7049834f, 0.05456242f, 0.7050135f } },
    { 38, -25.343836f, 3.6217628f, -12.086877f, { -0.00014297136f, 0.0013745351f, 0.0010721583f, 0.99999845f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr25_bones[5] = { /* 0x08878150 */
    { 40, -44.304413f, 2.3323526f, -11.173642f, { -0.0011798928f, 0.06224206f, -0.0018645945f, 0.9980586f } },
    { 37, -9.052797f, -2.737388f, -0.7504136f, { -0.05456475f, 0.7049834f, 0.05456242f, 0.7050135f } },
    { 38, -25.338203f, 3.622319f, -12.088512f, { -0.0001250561f, 0.0012023094f, 0.0010723827f, 0.9999987f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr7_bones[5] = { /* 0x088781F0 */
    { 40, -44.294006f, 2.222455f, -11.17116f, { -0.001185711f, 0.06229644f, -0.0018648808f, 0.9980552f } },
    { 37, -8.19726f, -2.2333298f, -0.761175f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.400093f, 3.6293893f, -12.062866f, { -0.00013754312f, 0.0013380285f, 0.0022780776f, 0.9999965f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr20_bones[5] = { /* 0x08878290 */
    { 40, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 37, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 38, -0.16214149f, 3.6154706f, -0.016765239f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr22_bones[5] = { /* 0x08878330 */
    { 40, -44.269672f, 2.2927713f, -11.231956f, { -0.0065024327f, 0.06097078f, -0.0015242293f, 0.9981172f } },
    { 37, -8.0304365f, -2.1105204f, -0.75736433f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.3664f, 3.632931f, -12.072714f, { 8.41e-09f, -1.48e-09f, 0.0022780849f, 0.9999974f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr28_bones[5] = { /* 0x088783D0 */
    { 40, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 37, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 38, -0.16214149f, 3.6154706f, -0.016765239f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr24_bones[5] = { /* 0x08878470 */
    { 40, -44.260563f, 2.2211516f, -10.9865055f, { -7.743301e-05f, 0.0629185f, 0.0012746147f, 0.99801785f } },
    { 37, -9.768001f, -3.3223882f, -0.36987147f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.344532f, 3.6216934f, -12.086502f, { -0.00012712595f, 0.0012366961f, 0.0022781158f, 0.9999966f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr8_bones[6] = { /* 0x08878510 */
    { 42, -9.160106f, -1.1136465f, -0.36120236f, { 0.5073871f, 0.48525852f, 0.514905f, 0.49188957f } },
    { 40, -44.275665f, 2.2243607f, -11.206178f, { -0.0011038926f, 0.061527908f, -0.0018613136f, 0.998103f } },
    { 37, -8.045924f, -2.0097036f, -0.75837094f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.380402f, 3.6314158f, -12.068554f, { -5.879215e-05f, 0.00056768884f, 0.0022775424f, 0.99999726f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr23_bones[5] = { /* 0x088785D0 */
    { 40, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 37, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 38, -0.16214149f, 3.6154706f, -0.016765239f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr30_bones[5] = { /* 0x08878670 */
    { 40, -44.26263f, 2.2257524f, -11.231956f, { -0.0065024323f, 0.06097078f, -0.0015242406f, 0.9981172f } },
    { 37, -7.785797f, -1.9791522f, -0.75736433f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.366404f, 3.63293f, -12.072714f, { 8.37e-09f, -1.08e-09f, 0.0022780737f, 0.9999974f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr3_bones[5] = { /* 0x08878710 */
    { 40, -44.260563f, 2.2211607f, -10.986503f, { -7.743301e-05f, 0.0629185f, 0.0012746147f, 0.99801785f } },
    { 37, -9.877696f, -2.8616679f, -0.9449287f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.344532f, 3.6216934f, -12.086502f, { -0.00012712595f, 0.0012366961f, 0.0022781158f, 0.9999966f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr33_bones[5] = { /* 0x088787B0 */
    { 40, -44.261734f, 2.2172046f, -11.249005f, { -0.0012811343f, 0.060936045f, -0.0018722527f, 0.9981391f } },
    { 37, -8.05303f, -2.2429101f, -0.75736344f, { -0.05456352f, 0.70499843f, 0.05456368f, 0.70499843f } },
    { 38, -25.30657f, 3.6166127f, -12.113987f, { -0.00023875237f, -2.483464e-05f, 0.0010723457f, 0.9999994f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr13_bones[5] = { /* 0x08878850 */
    { 40, 3.8745577f, 5.939133f, -15.96011f, { 0.00036854838f, 0.00010461747f, -0.036732215f, 0.9993251f } },
    { 37, -7.785797f, -2.1699653f, -0.75736296f, { -0.054563522f, 0.7049985f, 0.054563675f, 0.7049984f } },
    { 38, 13.746393f, 11.035532f, 0.10999952f, { 1.482e-08f, -5.2e-10f, -0.03487172f, 0.9993918f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
int chr16_pairs[4] = { 40, 33 }; /* 0x088788F0 */
ChrBone chr16_bones[5] = { /* 0x08878900 */
    { 40, 18.988037f, 0.039029717f, -75.75673f, { -0.023310736f, -0.13557746f, -0.0033985327f, 0.9904867f } },
    { 37, -7.975506f, -1.1035748f, 0.17324257f, { -0.07194128f, 0.7029405f, 0.07183864f, 0.7039448f } },
    { 38, -0.16214149f, 3.6154706f, -0.016765239f, { 0.0f, 0.0f, 0.0f, 1.0f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr31_bones[5] = { /* 0x088789A0 */
    { 40, -44.252903f, 2.2562797f, -11.077101f, { -0.00674068f, 0.060945373f, -0.0015389201f, 0.99811715f } },
    { 37, -10.783499f, -4.0833383f, -0.52232665f, { -0.054563604f, 0.7049984f, 0.054563593f, 0.70499855f } },
    { 38, -25.317606f, 3.6154778f, -12.095605f, { -0.00023878131f, -2.454672e-05f, 0.002278113f, 0.9999974f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr14_bones[5] = { /* 0x08878A40 */
    { 40, -42.817974f, 2.298372f, -11.551839f, { -0.006264195f, 0.06099617f, -0.0015095252f, 0.9981172f } },
    { 37, -7.7582197f, -2.730565f, -0.74885136f, { -0.087844275f, 0.7016185f, 0.08818934f, 0.7015964f } },
    { 38, -23.745214f, 3.6316082f, -12.097479f, { 0.00023875973f, 2.481937e-05f, 0.0010723033f, 0.9999994f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr17_bones[5] = { /* 0x08878AE0 */
    { 40, -44.275665f, 2.2243454f, -11.206177f, { -0.0011038929f, 0.061527904f, -0.0018613136f, 0.998103f } },
    { 37, -8.046413f, -1.9961519f, -0.8448467f, { -0.054563567f, 0.7049983f, 0.05456355f, 0.70499855f } },
    { 38, -25.331688f, 3.6230187f, -12.0902605f, { -5.879246e-05f, 0.00056768634f, 0.0022775424f, 0.99999726f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr36_bones[5] = { /* 0x08878B80 */
    { 40, -44.307423f, 2.2358606f, -11.138815f, { -0.0061680437f, 0.06288665f, -0.0015504871f, 0.9980004f } },
    { 37, -7.785446f, -2.2130919f, -0.7573639f, { -0.054563604f, 0.70499855f, 0.05456363f, 0.7049984f } },
    { 38, -25.414389f, 3.6413865f, -12.05114f, { 0.00033869813f, 0.0019179317f, 0.0022188486f, 0.99999565f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};
ChrBone chr27_bones[5] = { /* 0x08878C20 */
    { 40, -44.262283f, 2.2257817f, -11.232638f, { -0.0065008313f, 0.06095577f, -0.001524242f, 0.99811816f } },
    { 37, -7.7835608f, -2.086039f, -0.74526936f, { 0.05456141f, -0.70503086f, -0.054566428f, -0.704966f } },
    { 38, -25.307138f, 3.6256113f, -12.097329f, { 1.57301e-06f, -1.50427e-05f, 0.0010723063f, 0.9999994f } },
    { 39, 0.0f, 0.0f, 44.0f, { 1.0f, 0.0f, 0.0f, 0.0f } },
    { 41, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 1.0f } },
};

/* character 37 */
int chr37_init(void *self, void *a, void *b);
int chr37_event(void *self, int ev, void *a, void *b);
int chr37_update(void *self, void *a, void *b, void *c, void *d);
int chr37_bone_id(void *self, int i);
int chr37_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr37_motion(void *self, void *a, void *b, void *c, int d);

void chr37_setup(ChrVt *vt)
{
    vt->init = chr37_init;
    vt->event = chr37_event;
    vt->update = chr37_update;
    vt->bone_id = chr37_bone_id;
    vt->bone_rest = chr37_bone_rest;
    vt->motion = chr37_motion;
}

int chr37_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr37_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr37_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr37_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 33;
    case 4: return 28;
    case 2: return 51;
    case 1: return 49;
    case 20: return 47;
    case 12: return 38;
    case 19: return 7;
    case 26: return 19;
    case 11: return 37;
    case 6: return 32;
    case 16: return 2;
    case 31: return 42;
    case 8: return 34;
    case 5: return 31;
    case 15: return 3;
    case 32: return 43;
    case 30: return 54;
    case 13: return 0;
    case 29: return 50;
    case 25: return 59;
    case 3: return 52;
    case 22: return 45;
    case 23: return 61;
    case 18: return 8;
    case 10: return 36;
    case 21: return 46;
    case 24: return 60;
    case 9: return 35;
    case 17: return 10;
    case 0: return 62;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr37_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3C0 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr37_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr37_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr37_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 32 */
int chr32_init(void *self, void *a, void *b);
int chr32_event(void *self, int ev, void *a, void *b);
int chr32_update(void *self, void *a, void *b, void *c, void *d);
int chr32_bone_id(void *self, int i);
int chr32_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr32_motion(void *self, void *a, void *b, void *c, int d);

void chr32_setup(ChrVt *vt)
{
    vt->init = chr32_init;
    vt->event = chr32_event;
    vt->update = chr32_update;
    vt->bone_id = chr32_bone_id;
    vt->bone_rest = chr32_bone_rest;
    vt->motion = chr32_motion;
}

int chr32_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr32_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr32_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr32_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 26;
    case 4: return 22;
    case 2: return 46;
    case 1: return 44;
    case 20: return 42;
    case 12: return 32;
    case 19: return 7;
    case 26: return 13;
    case 11: return 30;
    case 6: return 25;
    case 16: return 2;
    case 31: return 37;
    case 8: return 28;
    case 5: return 33;
    case 15: return 3;
    case 32: return 38;
    case 30: return 49;
    case 13: return 0;
    case 29: return 45;
    case 25: return 54;
    case 3: return 47;
    case 22: return 40;
    case 23: return 56;
    case 18: return 8;
    case 10: return 31;
    case 21: return 41;
    case 24: return 55;
    case 9: return 29;
    case 17: return 10;
    case 0: return 57;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr32_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3C4 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr32_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr32_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr32_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* chr_kind3 */
int chr_kind3_init(void *self, void *a, void *b);
int chr_kind3_event(void *self, int ev, void *a, void *b);
int chr_kind3_update(void *self, void *a, void *b, void *c, void *d);
int chr_kind3_bone_id(void *self, int i);
int chr_kind3_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr_kind3_motion(void *self, void *a, void *b, void *c, int d);

void chr_kind3_setup(ChrVt *vt)
{
    vt->init = chr_kind3_init;
    vt->event = chr_kind3_event;
    vt->update = chr_kind3_update;
    vt->bone_id = chr_kind3_bone_id;
    vt->bone_rest = chr_kind3_bone_rest;
    vt->motion = chr_kind3_motion;
}

int chr_kind3_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr_kind3_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr_kind3_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr_kind3_bone_id(void *self, int i)
{
    DdCharPos pos = { 0 };

    charmap_lookup2(charmap_lookup(((ChrModel *)self)->chr_id, 0), &pos);
    switch (pos.kind) {
    case 5:
        switch (i) {
            case 14: return 5;
            case 7: return 29;
            case 4: return 35;
            case 2: return 47;
            case 1: return 45;
            case 20: return 43;
            case 12: return 34;
            case 19: return 7;
            case 26: return 13;
            case 11: return 33;
            case 6: return 28;
            case 16: return 2;
            case 31: return 16;
            case 8: return 30;
            case 5: return 27;
            case 15: return 3;
            case 32: return 17;
            case 30: return 50;
            case 13: return 0;
            case 29: return 46;
            case 25: return 55;
            case 3: return 48;
            case 22: return 41;
            case 23: return 57;
            case 18: return 8;
            case 10: return 32;
            case 21: return 42;
            case 24: return 56;
            case 9: return 31;
            case 17: return 10;
            case 0: return 58;
        }
        break;
    case 6:
        switch (i) {
            case 14: return 5;
            case 7: return 29;
            case 4: return 35;
            case 2: return 47;
            case 1: return 45;
            case 20: return 43;
            case 12: return 34;
            case 19: return 7;
            case 26: return 13;
            case 11: return 33;
            case 6: return 28;
            case 16: return 2;
            case 31: return 16;
            case 8: return 30;
            case 5: return 27;
            case 15: return 3;
            case 32: return 17;
            case 30: return 50;
            case 13: return 0;
            case 29: return 46;
            case 25: return 55;
            case 3: return 48;
            case 22: return 41;
            case 23: return 57;
            case 18: return 8;
            case 10: return 32;
            case 21: return 42;
            case 24: return 56;
            case 9: return 31;
            case 17: return 10;
            case 0: return 58;
        }
        break;
    case 7:
        switch (i) {
            case 14: return 16;
            case 7: return 39;
            case 4: return 45;
            case 2: return 59;
            case 1: return 55;
            case 20: return 47;
            case 12: return 44;
            case 19: return 3;
            case 26: return 7;
            case 11: return 43;
            case 6: return 38;
            case 16: return 2;
            case 31: return 25;
            case 8: return 40;
            case 5: return 37;
            case 15: return 5;
            case 32: return 26;
            case 30: return 63;
            case 13: return 0;
            case 29: return 58;
            case 25: return 21;
            case 3: return 60;
            case 22: return 19;
            case 23: return 49;
            case 18: return 6;
            case 10: return 42;
            case 21: return 34;
            case 24: return 35;
            case 9: return 41;
            case 17: return 17;
            case 0: return 56;
        }
        break;
    case 8:
        switch (i) {
            case 14: return 16;
            case 7: return 39;
            case 4: return 45;
            case 2: return 59;
            case 1: return 55;
            case 20: return 47;
            case 12: return 44;
            case 19: return 3;
            case 26: return 7;
            case 11: return 43;
            case 6: return 38;
            case 16: return 2;
            case 31: return 25;
            case 8: return 40;
            case 5: return 37;
            case 15: return 5;
            case 32: return 26;
            case 30: return 63;
            case 13: return 0;
            case 29: return 58;
            case 25: return 21;
            case 3: return 60;
            case 22: return 19;
            case 23: return 49;
            case 18: return 6;
            case 10: return 42;
            case 21: return 34;
            case 24: return 35;
            case 9: return 41;
            case 17: return 17;
            case 0: return 56;
        }
        break;
    case 9:
        switch (i) {
            case 14: return 5;
            case 7: return 30;
            case 4: return 36;
            case 2: return 48;
            case 1: return 46;
            case 20: return 44;
            case 12: return 35;
            case 19: return 7;
            case 26: return 13;
            case 11: return 34;
            case 6: return 29;
            case 16: return 2;
            case 31: return 16;
            case 8: return 31;
            case 5: return 28;
            case 15: return 3;
            case 32: return 17;
            case 30: return 51;
            case 13: return 0;
            case 29: return 47;
            case 25: return 56;
            case 3: return 49;
            case 22: return 42;
            case 23: return 58;
            case 18: return 8;
            case 10: return 33;
            case 21: return 43;
            case 24: return 57;
            case 9: return 32;
            case 17: return 10;
            case 0: return 59;
        }
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr_kind3_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3C8 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr_kind3_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr_kind3_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr_kind3_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 10 */
int chr10_init(void *self, void *a, void *b);
int chr10_event(void *self, int ev, void *a, void *b);
int chr10_update(void *self, void *a, void *b, void *c, void *d);
int chr10_bone_id(void *self, int i);
int chr10_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr10_motion(void *self, void *a, void *b, void *c, int d);

void chr10_setup(ChrVt *vt)
{
    vt->init = chr10_init;
    vt->event = chr10_event;
    vt->update = chr10_update;
    vt->bone_id = chr10_bone_id;
    vt->bone_rest = chr10_bone_rest;
    vt->motion = chr10_motion;
}

int chr10_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr10_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr10_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr10_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 17;
    case 7: return 48;
    case 4: return 38;
    case 2: return 63;
    case 1: return 59;
    case 20: return 54;
    case 12: return 53;
    case 19: return 3;
    case 26: return 7;
    case 11: return 52;
    case 6: return 47;
    case 16: return 2;
    case 31: return 27;
    case 8: return 49;
    case 5: return 46;
    case 15: return 5;
    case 32: return 28;
    case 30: return 67;
    case 13: return 0;
    case 29: return 62;
    case 25: return 22;
    case 3: return 64;
    case 22: return 20;
    case 23: return 56;
    case 18: return 6;
    case 10: return 51;
    case 21: return 43;
    case 24: return 44;
    case 9: return 50;
    case 17: return 18;
    case 0: return 60;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr10_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3CC */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr10_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr10_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr10_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 11 */
int chr11_init(void *self, void *a, void *b);
int chr11_event(void *self, int ev, void *a, void *b);
int chr11_update(void *self, void *a, void *b, void *c, void *d);
int chr11_bone_id(void *self, int i);
int chr11_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr11_motion(void *self, void *a, void *b, void *c, int d);

void chr11_setup(ChrVt *vt)
{
    vt->init = chr11_init;
    vt->event = chr11_event;
    vt->update = chr11_update;
    vt->bone_id = chr11_bone_id;
    vt->bone_rest = chr11_bone_rest;
    vt->motion = chr11_motion;
}

int chr11_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr11_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr11_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr11_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 14;
    case 7: return 52;
    case 4: return 58;
    case 2: return 69;
    case 1: return 65;
    case 20: return 60;
    case 12: return 57;
    case 19: return 3;
    case 26: return 7;
    case 11: return 56;
    case 6: return 51;
    case 16: return 2;
    case 31: return 23;
    case 8: return 53;
    case 5: return 50;
    case 15: return 5;
    case 32: return 24;
    case 30: return 73;
    case 13: return 0;
    case 29: return 68;
    case 25: return 19;
    case 3: return 70;
    case 22: return 17;
    case 23: return 62;
    case 18: return 6;
    case 10: return 55;
    case 21: return 38;
    case 24: return 39;
    case 9: return 54;
    case 17: return 15;
    case 0: return 66;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr11_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3D0 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr11_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr11_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr11_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 4 */
int chr4_init(void *self, void *a, void *b);
int chr4_event(void *self, int ev, void *a, void *b);
int chr4_update(void *self, void *a, void *b, void *c, void *d);
int chr4_bone_id(void *self, int i);
int chr4_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr4_motion(void *self, void *a, void *b, void *c, int d);

void chr4_setup(ChrVt *vt)
{
    vt->init = chr4_init;
    vt->event = chr4_event;
    vt->update = chr4_update;
    vt->bone_id = chr4_bone_id;
    vt->bone_rest = chr4_bone_rest;
    vt->motion = chr4_motion;
}

int chr4_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr4_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr4_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr4_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 27;
    case 4: return 22;
    case 2: return 43;
    case 1: return 41;
    case 20: return 39;
    case 12: return 32;
    case 19: return 7;
    case 26: return 13;
    case 11: return 31;
    case 6: return 26;
    case 16: return 2;
    case 31: return 16;
    case 8: return 28;
    case 5: return 25;
    case 15: return 3;
    case 32: return 17;
    case 30: return 46;
    case 13: return 0;
    case 29: return 42;
    case 25: return 51;
    case 3: return 44;
    case 22: return 37;
    case 23: return 53;
    case 18: return 8;
    case 10: return 30;
    case 21: return 38;
    case 24: return 52;
    case 9: return 29;
    case 17: return 10;
    case 0: return 54;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr4_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3D4 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr4_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr4_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr4_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 21 */
int chr21_init(void *self, void *a, void *b);
int chr21_event(void *self, int ev, void *a, void *b);
int chr21_update(void *self, void *a, void *b, void *c, void *d);
int chr21_bone_id(void *self, int i);
int chr21_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr21_motion(void *self, void *a, void *b, void *c, int d);

void chr21_setup(ChrVt *vt)
{
    vt->init = chr21_init;
    vt->event = chr21_event;
    vt->update = chr21_update;
    vt->bone_id = chr21_bone_id;
    vt->bone_rest = chr21_bone_rest;
    vt->motion = chr21_motion;
}

int chr21_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr21_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr21_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr21_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 16;
    case 7: return 40;
    case 4: return 46;
    case 2: return 57;
    case 1: return 53;
    case 20: return 48;
    case 12: return 45;
    case 19: return 3;
    case 26: return 7;
    case 11: return 44;
    case 6: return 39;
    case 16: return 2;
    case 31: return 25;
    case 8: return 41;
    case 5: return 38;
    case 15: return 5;
    case 32: return 26;
    case 30: return 61;
    case 13: return 0;
    case 29: return 56;
    case 25: return 21;
    case 3: return 58;
    case 22: return 19;
    case 23: return 50;
    case 18: return 6;
    case 10: return 43;
    case 21: return 34;
    case 24: return 35;
    case 9: return 42;
    case 17: return 17;
    case 0: return 54;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr21_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3D8 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr21_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr21_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr21_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* chr18_alt */
int chr18_alt_init(void *self, void *a, void *b);
int chr18_alt_event(void *self, int ev, void *a, void *b);
int chr18_alt_update(void *self, void *a, void *b, void *c, void *d);
int chr18_alt_bone_id(void *self, int i);
int chr18_alt_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr18_alt_motion(void *self, void *a, void *b, void *c, int d);

void chr18_alt_setup(ChrVt *vt)
{
    vt->init = chr18_alt_init;
    vt->event = chr18_alt_event;
    vt->update = chr18_alt_update;
    vt->bone_id = chr18_alt_bone_id;
    vt->bone_rest = chr18_alt_bone_rest;
    vt->motion = chr18_alt_motion;
    vt->pairs = chr18_alt_pairs;
}

int chr18_alt_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr18_alt_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr18_alt_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr18_alt_bone_id(void *self, int i)
{
    switch (i) {
    case 2: return 8;
    case 1: return 6;
    case 20: return 4;
    case 26: return 27;
    case 31: return 30;
    case 32: return 31;
    case 30: return 11;
    case 13: return 38;
    case 29: return 7;
    case 25: return 14;
    case 3: return 9;
    case 22: return 1;
    case 23: return 17;
    case 21: return 2;
    case 24: return 15;
    case 0: return 18;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr18_alt_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3DC */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr18_alt_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr18_alt_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr18_alt_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 18 */
int chr18_init(void *self, void *a, void *b);
int chr18_event(void *self, int ev, void *a, void *b);
int chr18_update(void *self, void *a, void *b, void *c, void *d);
int chr18_bone_id(void *self, int i);
int chr18_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr18_motion(void *self, void *a, void *b, void *c, int d);

void chr18_setup(ChrVt *vt)
{
    vt->init = chr18_init;
    vt->event = chr18_event;
    vt->update = chr18_update;
    vt->bone_id = chr18_bone_id;
    vt->bone_rest = chr18_bone_rest;
    vt->motion = chr18_motion;
}

int chr18_init(void *self, void *a, void *b)
{
    func_08839240(self, a, b);
    func_0882837C(self, a, b);
    return 0;
}

int chr18_event(void *self, int ev, void *a, void *b)
{
    func_0883936C(self, ev, a, b);
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr18_update(void *self, void *a, void *b, void *c, void *d)
{
    func_088393B0(self, a, b, c, d);
    return func_08828470(self, a, b, c, d);
}

int chr18_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 27;
    case 4: return 33;
    case 2: return 64;
    case 1: return 62;
    case 20: return 60;
    case 12: return 32;
    case 19: return 7;
    case 26: return 13;
    case 11: return 31;
    case 6: return 26;
    case 16: return 2;
    case 31: return 16;
    case 8: return 28;
    case 5: return 25;
    case 15: return 3;
    case 32: return 17;
    case 30: return 67;
    case 13: return 0;
    case 29: return 63;
    case 25: return 51;
    case 3: return 65;
    case 22: return 58;
    case 23: return 53;
    case 18: return 8;
    case 10: return 30;
    case 21: return 59;
    case 24: return 52;
    case 9: return 29;
    case 17: return 10;
    case 0: return 54;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr18_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3E0 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr18_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr18_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr18_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 12 */
int chr12_init(void *self, void *a, void *b);
int chr12_event(void *self, int ev, void *a, void *b);
int chr12_update(void *self, void *a, void *b, void *c, void *d);
int chr12_bone_id(void *self, int i);
int chr12_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr12_motion(void *self, void *a, void *b, void *c, int d);

void chr12_setup(ChrVt *vt)
{
    vt->init = chr12_init;
    vt->event = chr12_event;
    vt->update = chr12_update;
    vt->bone_id = chr12_bone_id;
    vt->bone_rest = chr12_bone_rest;
    vt->motion = chr12_motion;
    vt->pairs = chr12_pairs;
}

int chr12_init(void *self, void *a, void *b)
{
    func_08839240(self, a, b);
    func_0882837C(self, a, b);
    return 0;
}

int chr12_event(void *self, int ev, void *a, void *b)
{
    func_0883936C(self, ev, a, b);
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr12_update(void *self, void *a, void *b, void *c, void *d)
{
    func_088393B0(self, a, b, c, d);
    return func_08828470(self, a, b, c, d);
}

int chr12_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 18;
    case 7: return 83;
    case 4: return 45;
    case 20: return 53;
    case 12: return 88;
    case 19: return 3;
    case 26: return 11;
    case 11: return 87;
    case 28: return 78;
    case 6: return 82;
    case 16: return 2;
    case 31: return 38;
    case 8: return 84;
    case 5: return 81;
    case 15: return 5;
    case 32: return 39;
    case 13: return 0;
    case 25: return 24;
    case 22: return 21;
    case 27: return 96;
    case 23: return 55;
    case 18: return 8;
    case 10: return 86;
    case 21: return 33;
    case 24: return 34;
    case 9: return 85;
    case 17: return 19;
    case 0: return 66;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr12_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3E4 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr12_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr12_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr12_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 9 */
int chr9_init(void *self, void *a, void *b);
int chr9_event(void *self, int ev, void *a, void *b);
int chr9_update(void *self, void *a, void *b, void *c, void *d);
int chr9_bone_id(void *self, int i);
int chr9_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr9_motion(void *self, void *a, void *b, void *c, int d);

void chr9_setup(ChrVt *vt)
{
    vt->init = chr9_init;
    vt->event = chr9_event;
    vt->update = chr9_update;
    vt->bone_id = chr9_bone_id;
    vt->bone_rest = chr9_bone_rest;
    vt->motion = chr9_motion;
    vt->pairs = chr9_pairs;
}

int chr9_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr9_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr9_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr9_bone_id(void *self, int i)
{
    switch (i) {
    case 29:
        i = 26;
        break;
    }
    switch (i) {
    case 14: return 31;
    case 7: return 88;
    case 4: return 54;
    case 20: return 75;
    case 12: return 93;
    case 19: return 3;
    case 26: return 19;
    case 11: return 92;
    case 6: return 87;
    case 16: return 2;
    case 31: return 46;
    case 8: return 89;
    case 35: return 85;
    case 5: return 86;
    case 15: return 11;
    case 32: return 47;
    case 13: return 0;
    case 25: return 37;
    case 22: return 34;
    case 23: return 80;
    case 18: return 12;
    case 10: return 91;
    case 21: return 60;
    case 24: return 64;
    case 9: return 90;
    case 17: return 32;
    case 0: return 97;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr9_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3E8 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr9_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr9_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr9_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 26 */
int chr26_init(void *self, void *a, void *b);
int chr26_event(void *self, int ev, void *a, void *b);
int chr26_update(void *self, void *a, void *b, void *c, void *d);
int chr26_bone_id(void *self, int i);
int chr26_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr26_motion(void *self, void *a, void *b, void *c, int d);

void chr26_setup(ChrVt *vt)
{
    vt->init = chr26_init;
    vt->event = chr26_event;
    vt->update = chr26_update;
    vt->bone_id = chr26_bone_id;
    vt->bone_rest = chr26_bone_rest;
    vt->motion = chr26_motion;
}

int chr26_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr26_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr26_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr26_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 30;
    case 4: return 25;
    case 2: return 44;
    case 1: return 42;
    case 20: return 40;
    case 12: return 35;
    case 19: return 7;
    case 26: return 13;
    case 11: return 34;
    case 6: return 29;
    case 16: return 2;
    case 31: return 16;
    case 8: return 31;
    case 5: return 28;
    case 15: return 3;
    case 32: return 17;
    case 30: return 47;
    case 13: return 0;
    case 29: return 43;
    case 25: return 52;
    case 3: return 45;
    case 22: return 38;
    case 23: return 54;
    case 18: return 8;
    case 10: return 33;
    case 21: return 39;
    case 24: return 53;
    case 9: return 32;
    case 17: return 10;
    case 0: return 55;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr26_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3EC */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr26_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr26_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr26_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 25 */
int chr25_init(void *self, void *a, void *b);
int chr25_event(void *self, int ev, void *a, void *b);
int chr25_update(void *self, void *a, void *b, void *c, void *d);
int chr25_bone_id(void *self, int i);
int chr25_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr25_motion(void *self, void *a, void *b, void *c, int d);

void chr25_setup(ChrVt *vt)
{
    vt->init = chr25_init;
    vt->event = chr25_event;
    vt->update = chr25_update;
    vt->bone_id = chr25_bone_id;
    vt->bone_rest = chr25_bone_rest;
    vt->motion = chr25_motion;
}

int chr25_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr25_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr25_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr25_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 30;
    case 4: return 25;
    case 2: return 44;
    case 1: return 42;
    case 20: return 40;
    case 12: return 35;
    case 19: return 7;
    case 26: return 13;
    case 11: return 34;
    case 6: return 29;
    case 16: return 2;
    case 31: return 16;
    case 8: return 31;
    case 5: return 28;
    case 15: return 3;
    case 32: return 17;
    case 30: return 47;
    case 13: return 0;
    case 29: return 43;
    case 25: return 52;
    case 3: return 45;
    case 22: return 38;
    case 23: return 54;
    case 18: return 8;
    case 10: return 33;
    case 21: return 39;
    case 24: return 53;
    case 9: return 32;
    case 17: return 10;
    case 0: return 55;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr25_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3F0 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr25_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr25_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr25_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 7 */
int chr7_init(void *self, void *a, void *b);
int chr7_event(void *self, int ev, void *a, void *b);
int chr7_update(void *self, void *a, void *b, void *c, void *d);
int chr7_bone_id(void *self, int i);
int chr7_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr7_motion(void *self, void *a, void *b, void *c, int d);

void chr7_setup(ChrVt *vt)
{
    vt->init = chr7_init;
    vt->event = chr7_event;
    vt->update = chr7_update;
    vt->bone_id = chr7_bone_id;
    vt->bone_rest = chr7_bone_rest;
    vt->motion = chr7_motion;
}

int chr7_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr7_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr7_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr7_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 15;
    case 7: return 45;
    case 4: return 30;
    case 2: return 54;
    case 1: return 51;
    case 20: return 39;
    case 12: return 50;
    case 19: return 3;
    case 26: return 7;
    case 11: return 49;
    case 6: return 44;
    case 16: return 2;
    case 31: return 24;
    case 8: return 46;
    case 5: return 43;
    case 15: return 5;
    case 32: return 25;
    case 30: return 58;
    case 13: return 0;
    case 29: return 53;
    case 25: return 20;
    case 3: return 55;
    case 22: return 18;
    case 23: return 41;
    case 18: return 6;
    case 10: return 48;
    case 21: return 33;
    case 24: return 34;
    case 9: return 47;
    case 17: return 16;
    case 0: return 52;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr7_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3F4 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr7_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr7_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr7_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 20 */
int chr20_init(void *self, void *a, void *b);
int chr20_event(void *self, int ev, void *a, void *b);
int chr20_update(void *self, void *a, void *b, void *c, void *d);
int chr20_bone_id(void *self, int i);
int chr20_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr20_motion(void *self, void *a, void *b, void *c, int d);

void chr20_setup(ChrVt *vt)
{
    vt->init = chr20_init;
    vt->event = chr20_event;
    vt->update = chr20_update;
    vt->bone_id = chr20_bone_id;
    vt->bone_rest = chr20_bone_rest;
    vt->motion = chr20_motion;
}

int chr20_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr20_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr20_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr20_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 16;
    case 7: return 72;
    case 2: return 85;
    case 1: return 66;
    case 20: return 46;
    case 12: return 81;
    case 19: return 3;
    case 26: return 7;
    case 11: return 80;
    case 6: return 71;
    case 16: return 2;
    case 31: return 29;
    case 8: return 75;
    case 5: return 68;
    case 15: return 5;
    case 32: return 30;
    case 30: return 89;
    case 13: return 0;
    case 29: return 84;
    case 25: return 23;
    case 3: return 86;
    case 22: return 19;
    case 23: return 48;
    case 18: return 6;
    case 10: return 77;
    case 21: return 38;
    case 24: return 41;
    case 9: return 76;
    case 17: return 17;
    case 0: return 67;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr20_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3F8 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr20_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr20_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr20_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 22 */
int chr22_init(void *self, void *a, void *b);
int chr22_event(void *self, int ev, void *a, void *b);
int chr22_update(void *self, void *a, void *b, void *c, void *d);
int chr22_bone_id(void *self, int i);
int chr22_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr22_motion(void *self, void *a, void *b, void *c, int d);

void chr22_setup(ChrVt *vt)
{
    vt->init = chr22_init;
    vt->event = chr22_event;
    vt->update = chr22_update;
    vt->bone_id = chr22_bone_id;
    vt->bone_rest = chr22_bone_rest;
    vt->motion = chr22_motion;
}

int chr22_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr22_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr22_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr22_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 16;
    case 7: return 40;
    case 4: return 46;
    case 2: return 58;
    case 1: return 55;
    case 20: return 51;
    case 12: return 45;
    case 19: return 3;
    case 26: return 7;
    case 11: return 44;
    case 6: return 39;
    case 16: return 2;
    case 31: return 25;
    case 8: return 41;
    case 5: return 38;
    case 15: return 5;
    case 32: return 26;
    case 30: return 62;
    case 13: return 0;
    case 29: return 57;
    case 25: return 21;
    case 3: return 59;
    case 22: return 19;
    case 23: return 53;
    case 18: return 6;
    case 10: return 43;
    case 21: return 34;
    case 24: return 35;
    case 9: return 42;
    case 17: return 17;
    case 0: return 56;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr22_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B3FC */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr22_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr22_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr22_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 28 */
int chr28_init(void *self, void *a, void *b);
int chr28_event(void *self, int ev, void *a, void *b);
int chr28_update(void *self, void *a, void *b, void *c, void *d);
int chr28_bone_id(void *self, int i);
int chr28_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr28_motion(void *self, void *a, void *b, void *c, int d);

void chr28_setup(ChrVt *vt)
{
    vt->init = chr28_init;
    vt->event = chr28_event;
    vt->update = chr28_update;
    vt->bone_id = chr28_bone_id;
    vt->bone_rest = chr28_bone_rest;
    vt->motion = chr28_motion;
}

int chr28_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr28_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr28_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr28_bone_id(void *self, int i)
{
    switch (i) {
    case 29:
        i = 0;
        break;
    default:
        break;
    }
    switch (i) {
    case 14: return 4;
    case 4: return 15;
    case 20: return 24;
    case 19: return 6;
    case 26: return 12;
    case 16: return 1;
    case 15: return 2;
    case 13: return 40;
    case 25: return 17;
    case 22: return 22;
    case 23: return 19;
    case 18: return 7;
    case 21: return 23;
    case 24: return 18;
    case 17: return 9;
    case 0: return 20;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr28_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B400 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr28_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr28_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr28_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 24 */
int chr24_init(void *self, void *a, void *b);
int chr24_event(void *self, int ev, void *a, void *b);
int chr24_update(void *self, void *a, void *b, void *c, void *d);
int chr24_bone_id(void *self, int i);
int chr24_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr24_motion(void *self, void *a, void *b, void *c, int d);

void chr24_setup(ChrVt *vt)
{
    vt->init = chr24_init;
    vt->event = chr24_event;
    vt->update = chr24_update;
    vt->bone_id = chr24_bone_id;
    vt->bone_rest = chr24_bone_rest;
    vt->motion = chr24_motion;
}

int chr24_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr24_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr24_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr24_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 15;
    case 7: return 47;
    case 4: return 31;
    case 2: return 58;
    case 1: return 54;
    case 20: return 41;
    case 12: return 52;
    case 19: return 3;
    case 26: return 7;
    case 11: return 51;
    case 6: return 46;
    case 16: return 2;
    case 31: return 24;
    case 8: return 48;
    case 5: return 45;
    case 15: return 5;
    case 32: return 25;
    case 30: return 62;
    case 13: return 0;
    case 29: return 57;
    case 25: return 20;
    case 3: return 59;
    case 22: return 18;
    case 23: return 43;
    case 18: return 6;
    case 10: return 50;
    case 21: return 35;
    case 24: return 36;
    case 9: return 49;
    case 17: return 16;
    case 0: return 55;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr24_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B404 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr24_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr24_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr24_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 8 */
int chr8_init(void *self, void *a, void *b);
int chr8_event(void *self, int ev, void *a, void *b);
int chr8_update(void *self, void *a, void *b, void *c, void *d);
int chr8_bone_id(void *self, int i);
int chr8_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr8_motion(void *self, void *a, void *b, void *c, int d);

void chr8_setup(ChrVt *vt)
{
    vt->init = chr8_init;
    vt->event = chr8_event;
    vt->update = chr8_update;
    vt->bone_id = chr8_bone_id;
    vt->bone_rest = chr8_bone_rest;
    vt->motion = chr8_motion;
}

int chr8_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr8_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr8_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr8_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 22;
    case 7: return 46;
    case 4: return 52;
    case 2: return 63;
    case 1: return 59;
    case 20: return 54;
    case 12: return 51;
    case 19: return 3;
    case 26: return 9;
    case 11: return 50;
    case 6: return 45;
    case 16: return 2;
    case 31: return 31;
    case 8: return 47;
    case 5: return 44;
    case 15: return 7;
    case 32: return 32;
    case 30: return 67;
    case 13: return 0;
    case 29: return 62;
    case 25: return 27;
    case 3: return 64;
    case 22: return 25;
    case 23: return 56;
    case 18: return 8;
    case 10: return 49;
    case 21: return 40;
    case 24: return 41;
    case 9: return 48;
    case 17: return 23;
    case 0: return 60;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr8_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B408 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr8_bones, 6, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr8_bones, 6, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr8_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 23 */
int chr23_init(void *self, void *a, void *b);
int chr23_event(void *self, int ev, void *a, void *b);
int chr23_update(void *self, void *a, void *b, void *c, void *d);
int chr23_bone_id(void *self, int i);
int chr23_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr23_motion(void *self, void *a, void *b, void *c, int d);

void chr23_setup(ChrVt *vt)
{
    vt->init = chr23_init;
    vt->event = chr23_event;
    vt->update = chr23_update;
    vt->bone_id = chr23_bone_id;
    vt->bone_rest = chr23_bone_rest;
    vt->motion = chr23_motion;
}

int chr23_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr23_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr23_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr23_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 2: return 28;
    case 1: return 26;
    case 20: return 24;
    case 19: return 7;
    case 26: return 14;
    case 16: return 1;
    case 15: return 3;
    case 30: return 31;
    case 29: return 27;
    case 25: return 48;
    case 3: return 29;
    case 22: return 22;
    case 23: return 50;
    case 18: return 9;
    case 21: return 23;
    case 24: return 49;
    case 17: return 11;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr23_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B40C */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr23_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr23_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr23_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 30 */
int chr30_init(void *self, void *a, void *b);
int chr30_event(void *self, int ev, void *a, void *b);
int chr30_update(void *self, void *a, void *b, void *c, void *d);
int chr30_bone_id(void *self, int i);
int chr30_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr30_motion(void *self, void *a, void *b, void *c, int d);

void chr30_setup(ChrVt *vt)
{
    vt->init = chr30_init;
    vt->event = chr30_event;
    vt->update = chr30_update;
    vt->bone_id = chr30_bone_id;
    vt->bone_rest = chr30_bone_rest;
    vt->motion = chr30_motion;
}

int chr30_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr30_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr30_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr30_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 16;
    case 7: return 42;
    case 4: return 48;
    case 2: return 59;
    case 1: return 55;
    case 20: return 50;
    case 12: return 47;
    case 19: return 3;
    case 26: return 7;
    case 11: return 46;
    case 6: return 41;
    case 16: return 2;
    case 31: return 25;
    case 8: return 43;
    case 5: return 40;
    case 15: return 5;
    case 32: return 26;
    case 30: return 63;
    case 13: return 0;
    case 29: return 58;
    case 25: return 21;
    case 3: return 60;
    case 22: return 19;
    case 23: return 52;
    case 18: return 6;
    case 10: return 45;
    case 21: return 36;
    case 24: return 37;
    case 9: return 44;
    case 17: return 17;
    case 0: return 56;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr30_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B410 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr30_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr30_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr30_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 3 */
int chr3_init(void *self, void *a, void *b);
int chr3_event(void *self, int ev, void *a, void *b);
int chr3_update(void *self, void *a, void *b, void *c, void *d);
int chr3_bone_id(void *self, int i);
int chr3_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr3_motion(void *self, void *a, void *b, void *c, int d);

void chr3_setup(ChrVt *vt)
{
    vt->init = chr3_init;
    vt->event = chr3_event;
    vt->update = chr3_update;
    vt->bone_id = chr3_bone_id;
    vt->bone_rest = chr3_bone_rest;
    vt->motion = chr3_motion;
}

int chr3_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr3_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr3_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr3_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 16;
    case 7: return 52;
    case 4: return 36;
    case 2: return 63;
    case 1: return 59;
    case 20: return 46;
    case 12: return 57;
    case 19: return 3;
    case 26: return 7;
    case 11: return 56;
    case 6: return 51;
    case 16: return 2;
    case 31: return 25;
    case 8: return 53;
    case 5: return 50;
    case 15: return 5;
    case 32: return 26;
    case 30: return 67;
    case 13: return 0;
    case 29: return 62;
    case 25: return 21;
    case 3: return 64;
    case 22: return 19;
    case 23: return 48;
    case 18: return 6;
    case 10: return 55;
    case 21: return 40;
    case 24: return 41;
    case 9: return 54;
    case 17: return 17;
    case 0: return 60;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr3_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B414 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr3_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr3_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr3_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 33 */
int chr33_init(void *self, void *a, void *b);
int chr33_event(void *self, int ev, void *a, void *b);
int chr33_update(void *self, void *a, void *b, void *c, void *d);
int chr33_bone_id(void *self, int i);
int chr33_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr33_motion(void *self, void *a, void *b, void *c, int d);

void chr33_setup(ChrVt *vt)
{
    vt->init = chr33_init;
    vt->event = chr33_event;
    vt->update = chr33_update;
    vt->bone_id = chr33_bone_id;
    vt->bone_rest = chr33_bone_rest;
    vt->motion = chr33_motion;
}

int chr33_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr33_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr33_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr33_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 27;
    case 4: return 22;
    case 2: return 45;
    case 1: return 43;
    case 20: return 41;
    case 12: return 32;
    case 19: return 7;
    case 26: return 13;
    case 11: return 31;
    case 6: return 26;
    case 16: return 2;
    case 31: return 36;
    case 8: return 28;
    case 5: return 25;
    case 15: return 3;
    case 32: return 37;
    case 30: return 48;
    case 13: return 0;
    case 29: return 44;
    case 25: return 53;
    case 3: return 46;
    case 22: return 39;
    case 23: return 55;
    case 18: return 8;
    case 10: return 30;
    case 21: return 40;
    case 24: return 54;
    case 9: return 29;
    case 17: return 10;
    case 0: return 56;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr33_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B418 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr33_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr33_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr33_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 13 */
int chr13_init(void *self, void *a, void *b);
int chr13_event(void *self, int ev, void *a, void *b);
int chr13_update(void *self, void *a, void *b, void *c, void *d);
int chr13_bone_id(void *self, int i);
int chr13_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr13_motion(void *self, void *a, void *b, void *c, int d);

void chr13_setup(ChrVt *vt)
{
    vt->init = chr13_init;
    vt->event = chr13_event;
    vt->update = chr13_update;
    vt->bone_id = chr13_bone_id;
    vt->bone_rest = chr13_bone_rest;
    vt->motion = chr13_motion;
}

int chr13_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr13_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr13_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr13_bone_id(void *self, int i)
{
    switch (i) {
    case 29:
        i = 21;
        break;
    }
    switch (i) {
    case 14: return 13;
    case 7: return 57;
    case 4: return 33;
    case 20: return 37;
    case 12: return 62;
    case 19: return 3;
    case 26: return 7;
    case 11: return 61;
    case 6: return 56;
    case 16: return 2;
    case 31: return 26;
    case 8: return 58;
    case 5: return 55;
    case 15: return 5;
    case 32: return 27;
    case 13: return 0;
    case 25: return 18;
    case 22: return 16;
    case 23: return 39;
    case 18: return 6;
    case 10: return 60;
    case 21: return 24;
    case 24: return 25;
    case 9: return 59;
    case 17: return 14;
    case 0: return 45;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr13_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B41C */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr13_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr13_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr13_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 16 */
int chr16_init(void *self, void *a, void *b);
int chr16_event(void *self, int ev, void *a, void *b);
int chr16_update(void *self, void *a, void *b, void *c, void *d);
int chr16_bone_id(void *self, int i);
int chr16_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr16_motion(void *self, void *a, void *b, void *c, int d);

void chr16_setup(ChrVt *vt)
{
    vt->init = chr16_init;
    vt->event = chr16_event;
    vt->update = chr16_update;
    vt->bone_id = chr16_bone_id;
    vt->bone_rest = chr16_bone_rest;
    vt->motion = chr16_motion;
    vt->pairs = chr16_pairs;
}

int chr16_init(void *self, void *a, void *b)
{
    ChrModel *m = self;

    if (m->f4 == 0)
        m->f178 = 0x20000000;
    else
        m->f178 = 0x10000000;
    m->flags |= 0x30000000;
    func_0882837C(self, a, b);
    return 0;
}

int chr16_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr16_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr16_bone_id(void *self, int i)
{
    switch (i) {
    case 29:
        i = 26;
        break;
    }
    switch (i) {
    case 14: return 5;
    case 7: return 23;
    case 4: return 18;
    case 20: return 32;
    case 12: return 28;
    case 19: return 7;
    case 26: return 13;
    case 11: return 27;
    case 6: return 22;
    case 16: return 2;
    case 8: return 24;
    case 5: return 21;
    case 15: return 3;
    case 33: return 40;
    case 13: return 0;
    case 25: return 44;
    case 22: return 30;
    case 23: return 46;
    case 18: return 8;
    case 10: return 26;
    case 21: return 31;
    case 24: return 45;
    case 9: return 25;
    case 17: return 10;
    case 0: return 47;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr16_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B420 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr16_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr16_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr16_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 31 */
int chr31_init(void *self, void *a, void *b);
int chr31_event(void *self, int ev, void *a, void *b);
int chr31_update(void *self, void *a, void *b, void *c, void *d);
int chr31_bone_id(void *self, int i);
int chr31_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr31_motion(void *self, void *a, void *b, void *c, int d);

void chr31_setup(ChrVt *vt)
{
    vt->init = chr31_init;
    vt->event = chr31_event;
    vt->update = chr31_update;
    vt->bone_id = chr31_bone_id;
    vt->bone_rest = chr31_bone_rest;
    vt->motion = chr31_motion;
}

int chr31_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr31_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr31_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr31_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 13;
    case 7: return 53;
    case 4: return 35;
    case 2: return 62;
    case 1: return 49;
    case 20: return 41;
    case 12: return 58;
    case 19: return 3;
    case 26: return 7;
    case 11: return 57;
    case 6: return 52;
    case 16: return 2;
    case 31: return 31;
    case 8: return 54;
    case 5: return 51;
    case 15: return 5;
    case 32: return 32;
    case 30: return 66;
    case 13: return 0;
    case 29: return 60;
    case 25: return 19;
    case 3: return 63;
    case 22: return 16;
    case 23: return 43;
    case 18: return 6;
    case 10: return 56;
    case 21: return 27;
    case 24: return 29;
    case 9: return 55;
    case 17: return 14;
    case 0: return 50;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr31_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B424 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr31_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr31_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr31_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 14 */
int chr14_init(void *self, void *a, void *b);
int chr14_event(void *self, int ev, void *a, void *b);
int chr14_update(void *self, void *a, void *b, void *c, void *d);
int chr14_bone_id(void *self, int i);
int chr14_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr14_motion(void *self, void *a, void *b, void *c, int d);

void chr14_setup(ChrVt *vt)
{
    vt->init = chr14_init;
    vt->event = chr14_event;
    vt->update = chr14_update;
    vt->bone_id = chr14_bone_id;
    vt->bone_rest = chr14_bone_rest;
    vt->motion = chr14_motion;
}

int chr14_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr14_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr14_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr14_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 5;
    case 7: return 26;
    case 4: return 32;
    case 2: return 41;
    case 1: return 39;
    case 20: return 36;
    case 12: return 31;
    case 19: return 7;
    case 26: return 13;
    case 11: return 30;
    case 6: return 25;
    case 16: return 2;
    case 31: return 16;
    case 8: return 27;
    case 5: return 24;
    case 15: return 3;
    case 30: return 44;
    case 13: return 0;
    case 29: return 40;
    case 25: return 49;
    case 3: return 42;
    case 22: return 34;
    case 23: return 51;
    case 18: return 8;
    case 10: return 29;
    case 21: return 35;
    case 24: return 50;
    case 9: return 28;
    case 17: return 10;
    case 0: return 52;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr14_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B428 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr14_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr14_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr14_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 17 */
int chr17_init(void *self, void *a, void *b);
int chr17_event(void *self, int ev, void *a, void *b);
int chr17_update(void *self, void *a, void *b, void *c, void *d);
int chr17_bone_id(void *self, int i);
int chr17_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr17_motion(void *self, void *a, void *b, void *c, int d);

void chr17_setup(ChrVt *vt)
{
    vt->init = chr17_init;
    vt->event = chr17_event;
    vt->update = chr17_update;
    vt->bone_id = chr17_bone_id;
    vt->bone_rest = chr17_bone_rest;
    vt->motion = chr17_motion;
}

int chr17_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr17_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr17_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr17_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 29;
    case 7: return 83;
    case 4: return 64;
    case 2: return 92;
    case 1: return 79;
    case 20: return 68;
    case 12: return 88;
    case 19: return 3;
    case 26: return 12;
    case 11: return 87;
    case 6: return 82;
    case 16: return 2;
    case 31: return 55;
    case 8: return 84;
    case 5: return 81;
    case 15: return 10;
    case 32: return 56;
    case 30: return 96;
    case 13: return 0;
    case 29: return 90;
    case 25: return 35;
    case 3: return 93;
    case 22: return 32;
    case 23: return 71;
    case 18: return 11;
    case 10: return 86;
    case 21: return 46;
    case 24: return 48;
    case 9: return 85;
    case 17: return 30;
    case 0: return 80;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr17_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B42C */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr17_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr17_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr17_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 36 */
int chr36_init(void *self, void *a, void *b);
int chr36_event(void *self, int ev, void *a, void *b);
int chr36_update(void *self, void *a, void *b, void *c, void *d);
int chr36_bone_id(void *self, int i);
int chr36_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr36_motion(void *self, void *a, void *b, void *c, int d);

void chr36_setup(ChrVt *vt)
{
    vt->init = chr36_init;
    vt->event = chr36_event;
    vt->update = chr36_update;
    vt->bone_id = chr36_bone_id;
    vt->bone_rest = chr36_bone_rest;
    vt->motion = chr36_motion;
}

int chr36_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr36_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr36_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr36_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 16;
    case 7: return 55;
    case 4: return 34;
    case 2: return 66;
    case 1: return 61;
    case 20: return 48;
    case 12: return 60;
    case 19: return 3;
    case 26: return 7;
    case 11: return 59;
    case 6: return 54;
    case 16: return 2;
    case 31: return 25;
    case 8: return 56;
    case 5: return 53;
    case 15: return 5;
    case 32: return 26;
    case 30: return 70;
    case 13: return 0;
    case 29: return 64;
    case 25: return 21;
    case 3: return 67;
    case 22: return 19;
    case 23: return 50;
    case 18: return 6;
    case 10: return 58;
    case 21: return 39;
    case 24: return 40;
    case 9: return 57;
    case 17: return 17;
    case 0: return 62;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr36_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B430 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr36_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr36_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr36_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}

/* character 27 */
int chr27_init(void *self, void *a, void *b);
int chr27_event(void *self, int ev, void *a, void *b);
int chr27_update(void *self, void *a, void *b, void *c, void *d);
int chr27_bone_id(void *self, int i);
int chr27_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id);
int chr27_motion(void *self, void *a, void *b, void *c, int d);

void chr27_setup(ChrVt *vt)
{
    vt->init = chr27_init;
    vt->event = chr27_event;
    vt->update = chr27_update;
    vt->bone_id = chr27_bone_id;
    vt->bone_rest = chr27_bone_rest;
    vt->motion = chr27_motion;
}

int chr27_init(void *self, void *a, void *b)
{
    func_0882837C(self, a, b);
    return 0;
}

int chr27_event(void *self, int ev, void *a, void *b)
{
    func_088283D4(self, ev, a, b);
    return 0;
}

int chr27_update(void *self, void *a, void *b, void *c, void *d)
{
    return func_08828470(self, a, b, c, d);
}

int chr27_bone_id(void *self, int i)
{
    switch (i) {
    case 14: return 4;
    case 7: return 29;
    case 4: return 35;
    case 2: return 46;
    case 1: return 44;
    case 20: return 42;
    case 12: return 34;
    case 19: return 8;
    case 26: return 15;
    case 11: return 33;
    case 6: return 28;
    case 16: return 2;
    case 31: return 18;
    case 8: return 30;
    case 5: return 27;
    case 15: return 3;
    case 32: return 19;
    case 30: return 49;
    case 13: return 0;
    case 29: return 45;
    case 25: return 56;
    case 3: return 47;
    case 22: return 40;
    case 23: return 58;
    case 18: return 9;
    case 10: return 32;
    case 21: return 41;
    case 24: return 57;
    case 9: return 31;
    case 17: return 10;
    case 0: return 59;
    }
    return 0;
}

#ifdef NON_MATCHING /* a flag static local (o original) vai para o .bss do objeto */
int chr27_bone_rest(void *self, DdVec4 *pos, DdVec4 *rot, int id)
{
    static int sorted; /* 0x08C5B434 */
    ChrBone *e;
    int key;

    if (sorted == 0) {
        qsort(chr27_bones, 5, sizeof(ChrBone), func_0881DF4C);
        sorted = 1;
    }
    key = id;
    e = bsearch(&key, chr27_bones, 5, sizeof(ChrBone), func_0881DF4C);
    if (e == 0)
        return 0;
    *pos = *(DdVec4 *)&e->px;
    pos->w = 1.0f;
    *rot = e->rot;
    return 1;
}
#endif

int chr27_motion(void *self, void *a, void *b, void *c, int d)
{
    return func_08828528(self, a, b, c, d);
}
