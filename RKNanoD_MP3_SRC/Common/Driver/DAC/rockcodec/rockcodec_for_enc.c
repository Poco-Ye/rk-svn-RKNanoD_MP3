/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   Rockcodec.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo          2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "DriverInclude.h"


#include "rockcodec.h"
#include "AudioControl.h"
#define  MAX_OUTLEVEL            10
#define ReadAcodecReg(addr)                     (*(volatile uint32 *)(addr))
#define WriteAcodecReg(addr, data)              (*(volatile uint32 *)(addr) = data)

#define SetAcodecRegData(addr, databit)         WriteAcodecReg(addr, ReadAcodecReg(addr)|(databit))
#define ClrAcodecRegData(addr, databit)         WriteAcodecReg(addr, ReadAcodecReg(addr)&(~databit))

#define SetAcodecRegBit(addr,bit)               WriteAcodecReg(addr,(ReadAcodecReg(addr)|(1<<bit)))
#define ClrAcodecRegBit(addr,bit)               WriteAcodecReg(addr,(ReadAcodecReg(addr)&(~(1<<bit))))
#define GetAcodecRegBit(addr,bit)               (ReadAcodecReg(addr)&(1<<bit))

#define MaskAcodecRegBits(addr, y, z)           WriteAcodecReg(addr, (ReadAcodecReg(addr)&(~y))|(z))
ACodecI2S_mode_t Acodec_I2S_MODE;
eACodecPll_Target_Freq pll_old_target;
CodecMode_en_t Codecmode_Bak;
CodecFS_en_t CodecFS_Bak;
CodecPower_Status_t Codec_Power_Status;
uint32 Acodec_suspend_En;
void ACodec_Set_I2S_Mode(eACodecI2sFormat_t BUS_FORMAT,
              eACodecI2sDATA_WIDTH_t I2S_Data_width,
              ACodecI2S_BUS_MODE_t I2S_Bus_mode,
              ACodecI2S_mode_t I2S_mode);
typedef struct tagAPLL_APP
{
    uint32 F_source;    //KHz
    uint32 F_target;    //KHz
    uint32 PLL_POSDIV_L3;
    uint32 PLL_POSDIV_H8;
    uint32 PLL_PREDIV_BIT;
    uint32 PLL_OUTDIV;
 }ACodecPLL_APP,*pACodecPLL_APP;

static const ACodecPLL_APP ACodecpllTable_61440[8]=
{
    {2048  ,61440 , 0  , 45, 2,  6},
    {3072  ,61440 , 0  , 45, 3,  6},
    {4096  ,61440 , 0  , 45, 4,  6},
    {6000  ,61440 , 0  , 64, 10, 5},
    {6144  ,61440 , 0  , 45, 6,  6},
    {12000 ,61440 , 0  , 96, 25, 6},
    {12288 ,61440 , 0  , 45, 12, 6},
    {24000 ,61440 , 0  , 48, 25, 10},

};

static const ACodecPLL_APP ACodecpllTable_56448[8]=
{
    {2048  ,56448 , 3  , 41, 2,  6},
    {3072  ,56448 , 1  , 55, 4,  6},
    {4096  ,56448 , 3  , 41, 4,  6},
    {6000  ,56448 , 0  , 147,25, 5},
    {6144  ,56448 , 1  , 55, 8,  6},
    {12000 ,56448 , 4  , 73, 25, 5},
    {12288 ,56448 , 1  , 55, 16, 6},
    {24000 ,56448 , 6  , 36, 25, 3},

};

static const ACodecPLL_APP ACodecpllTable_40960[8]=
{
    {2048  ,40960 , 4   , 22, 1,  9},
    {3072  ,40960 , 0   , 45, 3,  9},
    {4096  ,40960 , 4   , 22, 2,  9},
    {6000  ,40960 , 0   , 192,25, 9},
    {6144  ,40960 , 0   , 40, 6,  9},
    {12000 ,40960 , 0   , 96, 25, 9},
    {12288 ,40960 , 0   , 45, 12, 9},
    {24000 ,40960 , 0   , 96, 50, 8},

};
_ATTR_DRIVER_CODE_ SRUCT_CODEC_CONFIG ACodec_LineOutVol_General[MAX_VOLUME + 1] =
{
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
 0, 140, 15, 1,  0, 165,   0, 157,      0, 124,   0, 124, //  1
 0, 135, 15, 1,  0, 153,   0, 145,      0, 132,   0, 132, //  2
 0, 123, 15, 1,  0, 148,   0, 140,      0, 127,   0, 127, //  3
 0, 123, 11, 1,  0, 145,   0, 137,      0, 124,   0, 124, //  4
 0, 123, 7,  1,  0, 143,   0, 135,      0, 122,   0, 122, //  5
 0, 123, 4,  1,  0, 144,   0, 136,      0, 123,   0, 123, //  6
 0, 123, 2,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  7
 0, 123, 0,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  8
 0, 117, 0,  1,  0, 134,   0, 126,      0, 113,   0, 113, //  9
 0, 111, 0,  1,  0, 127,   0, 119,      0, 106,   0, 106, //  10
 0, 107, 0,  1,  0, 122,   0, 114,      0, 101,   0, 101, //  11
 0, 101, 0,  1,  0, 116,   0, 108,      0, 95,    0, 95,  //  12
 0, 96,  0,  1,  0, 110,   0, 102,      0, 89,    0, 89,  //  13
 0, 91,  0,  1,  0, 103,   0, 95,       0, 82,    0, 82,  //  14
 0, 87,  0,  1,  0, 98,    0, 90,       0, 77,    0, 77,  //  15
 0, 81,  0,  1,  0, 93,    0, 85,       0, 72,    0, 72,  //  16
 0, 77,  0,  1,  0, 84,    0, 76,       0, 63,    0, 63,  //  17
 0, 72,  0,  1,  0, 80,    0, 72,       0, 59,    0, 59,  //  18
 0, 67,  0,  1,  0, 77,    0, 69,       0, 56,    0, 56,  //  19
 0, 62,  0,  1,  0, 69,    0, 61,       0, 48,    0, 48,  //  20
 0, 57,  0,  1,  0, 61,    0, 53,       0, 40,    0, 40,  //  21
 0, 52,  0,  1,  0, 56,    0, 48,       0, 35,    0, 35,  //  22
 0, 47,  0,  1,  0, 50,    0, 42,       0, 29,    0, 29,  //  23
 0, 43,  0,  1,  0, 45,    0, 37,       0, 24,    0, 24,  //  24
 0, 38,  0,  1,  0, 40,    0, 32,       0, 19,    0, 19,  //  25
 0, 34,  0,  1,  0, 33,    0, 25,       0, 12,    0, 12,  //  26
 0, 28,  0,  1,  0, 28,    0, 20,       0, 7,     0, 7,   //  27
 0, 24,  0,  1,  0, 22,    0, 14,       0, 1,     0, 1,    //  28
 0, 19,  0,  1,  0, 17,    0, 9,        1, 4,     1, 4,    //  29
 0, 14,  0,  1,  0, 14,    0, 6,        1, 1,     1, 1,    //  30
 0, 9,   0,  1,  0, 17,    0, 9,        1, 4,     1, 4,    //  31
 0, 4,   0,  1,  0, 14,    0, 6,        1, 1,     1, 1,    //  32
};


// HP_AMPVol;EQ_POP_HEAVY_HP_AMPVol;EQ_JAZZ_UNIQUE_HP_AMPVol;EQ_USER_HP_AMPVol;EQ_BASS_HP_AMPVol
// Description : increase the HP amplitude from 3dB to 9dB
//               0: 0 dB;1: 3 dB;2: 6 dB;3: 9 dB

// Dac_DigVol;EQ_POP_HEAVY_Dac_DigVol;EQ_JAZZ_UNIQUE_Dac_DigVol;EQ_USER_Dac_DigVol;EQ_BASS_Dac_DigVol
// Description : digital volume of DAC channel
//               0.375db/step: 0 dB - (-95) dB
//               0x00: 0dB; 0xff : -95dB

// HP_ANTIPOPVol
// Description : decrease the HP amplitude from 0dB to -15dB
//               1 db/step: 0 dB - (-15) dB
//               0x00: 0dB; 0xf : -15dB

_ATTR_DRIVER_CODE_ SRUCT_CODEC_CONFIG ACodec_HPoutVol_General[MAX_VOLUME + 1] =
{
#ifdef VOL_700MV
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
 0, 144, 15, 1,  0, 134,   0, 125,      0, 112,   0, 112, //  1
 0, 140, 15, 1,  0, 130,   0, 121,      0, 108,   0, 108, //  2
 0, 137, 15, 1,  0, 127,   0, 118,      0, 105,   0, 105, //  3
 0, 135, 12, 1,  0, 125,   0, 116,      0, 103,   0, 103, //  4
 0, 135, 9,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  5
 0, 135, 6,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  6
 0, 135, 3,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  7
 0, 135, 0,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  8
 0, 132, 0,  1,  0, 122,   0, 113,      0, 100,   0, 100, //  9
 0, 129, 0,  1,  0, 119,   0, 110,      0, 97,    0, 97, //  10
 0, 122, 0,  1,  0, 112,   0, 103,      0, 90,    0, 90, //  11
 0, 115, 0,  1,  0, 105,   0, 96,       0, 83,    0, 83,  //  12
 0, 110, 0,  1,  0, 100,   0, 91,       0, 78,    0, 78,  //  13
 0, 104, 0,  1,  0, 94,    0, 85,       0, 72,    0, 72,  //  14
 0, 98,  0,  1,  0, 88,    0, 79,       0, 66,    0, 66,  //  15
 0, 92,  0,  1,  0, 82,    0, 73,       0, 60,    0, 60,  //  16
 0, 87,  0,  1,  0, 77,    0, 68,       0, 55,    0, 55,  //  17
 0, 81,  0,  1,  0, 71,    0, 62,       0, 49,    0, 49,  //  18
 0, 75,  0,  1,  0, 65,    0, 56,       0, 43,    0, 43,  //  19
 0, 70,  0,  1,  0, 60,    0, 51,       0, 38,    0, 38,  //  20
 0, 64,  0,  1,  0, 54,    0, 46,       0, 32,    0, 32,  //  21
 0, 58,  0,  1,  0, 48,    0, 40,       0, 26,    0, 26,  //  22
 0, 52,  0,  1,  0, 42,    0, 34,       0, 20,    0, 20,  //  23
 0, 47,  0,  1,  0, 37,    0, 29,       0, 15,    0, 15,  //  24
 0, 41,  0,  1,  0, 30,    0, 23,       0, 7,     0, 7,  //  25
 0, 35,  0,  1,  0, 23,    0, 17,       0, 2,     0, 2,  //  26
 0, 30,  0,  1,  0, 18,    0, 12,       1, 3,     1, 3,   //  27
 0, 24,  0,  1,  0, 14,    0, 6,        2, 8,     2, 5,    //  28
 0, 19,  0,  1,  0, 10,    0, 2,        2, 5,     2, 2,    //  29
 0, 16,  0,  1,  0, 7,     1, 7,        2, 2,     2, 0,    //  30
 0, 14,  0,  1,  0, 2,     1, 4,        2, 0,     3, 4,    //  31
 0, 13,  0,  1,  1, 4,     1, 2,        3, 7,     3, 2,    //  32
#else
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254,//  0
 0, 176, 15, 1,  0, 166,   0, 157,      0, 124,   0, 124,//  1
 0, 164, 15, 1,  0, 154,   0, 145,      0, 132,   0, 132,//  2
 0, 159, 15, 1,  0, 149,   0, 140,      0, 127,   0, 127,//  3
 0, 156, 11, 1,  0, 146,   0, 137,      0, 124,   0, 124,//  4
 0, 154, 9,  1,  0, 144,   0, 135,      0, 122,   0, 122,//  5
 0, 155, 5,  1,  0, 143,   0, 136,      0, 123,   0, 123,//  6
 0, 153, 3,  1,  0, 143,   0, 134,      0, 121,   0, 121,//  7
 0, 153, 0,  1,  0, 143,   0, 134,      0, 121,   0, 121,//  8
 0, 145, 0,  1,  0, 135,   0, 126,      0, 113,   0, 113,//  9
 0, 138, 0,  1,  0, 128,   0, 119,      0, 106,   0, 106,//  10
 0, 133, 0,  1,  0, 123,   0, 114,      0, 101,   0, 101,//  11
 0, 127, 0,  1,  0, 117,   0, 108,      0, 95,    0, 95,  //  12
 0, 121, 0,  1,  0, 111,   0, 102,      0, 89,    0, 89,  //  13
 0, 114, 0,  1,  0, 104,   0, 95,       0, 82,    0, 82,  //  14
 0, 109, 0,  1,  0, 99,    0, 90,       0, 77,    0, 77,  //  15
 0, 104, 0,  1,  0, 94,    0, 85,       0, 72,    0, 72,  //  16
 0, 95,  0,  1,  0, 85,    0, 76,       0, 63,    0, 63,  //  17
 0, 91,  0,  1,  0, 81,    0, 72,       0, 59,    0, 59,  //  18
 0, 88,  0,  1,  0, 78,    0, 69,       0, 56,    0, 56,  //  19
 0, 80,  0,  1,  0, 70,    0, 61,       0, 48,    0, 48,  //  20
 0, 72,  0,  1,  0, 62,    0, 53,       0, 40,    0, 40,  //  21
 0, 67,  0,  1,  0, 57,    0, 48,       0, 35,    0, 35,  //  22
 0, 61,  0,  1,  0, 51,    0, 42,       0, 29,    0, 29,  //  23
 0, 56,  0,  1,  0, 46,    0, 37,       0, 24,    0, 24,  //  24
 0, 51,  0,  1,  0, 41,    0, 32,       0, 19,    0, 19,  //  25
 0, 44,  0,  1,  0, 34,    0, 25,       0, 12,    0, 12,  //  26
 0, 39,  0,  1,  0, 29,    0, 20,       0, 7,     0, 7,   //  27
 0, 33,  0,  1,  0, 23,    0, 14,       0, 7,     0, 7,    //  28
 0, 28,  0,  1,  0, 18,    0, 9,        0, 7,     0, 7,    //  29
 0, 25,  0,  1,  0, 15,    0, 7,        0, 7,     0, 7,    //  30
 0, 25,  0,  1,  0, 15,    0, 7,        0, 7,     0, 7,    //  31
 0, 25,  0,  1,  0, 15,    0, 7,        0, 7,     0, 7,    //  32
#endif

};

// HP_AMPVol;EQ_POP_HEAVY_HP_AMPVol;EQ_JAZZ_UNIQUE_HP_AMPVol;EQ_USER_HP_AMPVol;EQ_BASS_HP_AMPVol
// Description : increase the HP amplitude from 3dB to 9dB
//               0: 0 dB;1: 3 dB;2: 6 dB;3: 9 dB

// Dac_DigVol;EQ_POP_HEAVY_Dac_DigVol;EQ_JAZZ_UNIQUE_Dac_DigVol;EQ_USER_Dac_DigVol;EQ_BASS_Dac_DigVol
// Description : digital volume of DAC channel
//               0.375db/step: 0 dB - (-95) dB
//               0x00: 0dB; 0xff : -95dB

// HP_ANTIPOPVol
// Description : decrease the HP amplitude from 0dB to -15dB
//               1 db/step: 0 dB - (-15) dB
//               0x00: 0dB; 0xf : -15dB

_ATTR_DRIVER_CODE_ SRUCT_CODEC_CONFIG CodecConfig_Europe[MAX_VOLUME + 1] =
{
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
 0, 144, 15, 1,  0, 165,   0, 157,      0, 124,   0, 124, //  1
 0, 140, 15, 1,  0, 153,   0, 145,      0, 132,   0, 132, //  2
 0, 137, 15, 1,  0, 148,   0, 140,      0, 127,   0, 127, //  3
 0, 135, 12, 1,  0, 145,   0, 137,      0, 124,   0, 124, //  4
 0, 135, 9,  1,  0, 143,   0, 135,      0, 122,   0, 122, //  5
 0, 135, 6,  1,  0, 144,   0, 136,      0, 123,   0, 123, //  6
 0, 135, 3,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  7
 0, 135, 0,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  8
 0, 132, 0,  1,  0, 134,   0, 126,      0, 113,   0, 113, //  9
 0, 129, 0,  1,  0, 127,   0, 119,      0, 106,   0, 106, //  10
 0, 122, 0,  1,  0, 122,   0, 114,      0, 101,   0, 101, //  11
 0, 115, 0,  1,  0, 116,   0, 108,      0, 95,    0, 95,  //  12
 0, 110, 0,  1,  0, 110,   0, 102,      0, 89,    0, 89,  //  13
 0, 104, 0,  1,  0, 103,   0, 95,       0, 82,    0, 82,  //  14
 0, 98,  0,  1,  0, 98,    0, 90,       0, 77,    0, 77,  //  15
 0, 92,  0,  1,  0, 93,    0, 85,       0, 72,    0, 72,  //  16
 0, 87,  0,  1,  0, 84,    0, 76,       0, 63,    0, 63,  //  17
 0, 81,  0,  1,  0, 80,    0, 72,       0, 59,    0, 59,  //  18
 0, 75,  0,  1,  0, 77,    0, 69,       0, 56,    0, 56,  //  19
 0, 70,  0,  1,  0, 69,    0, 61,       0, 48,    0, 48,  //  20
 0, 64,  0,  1,  0, 61,    0, 53,       0, 40,    0, 40,  //  21
 0, 58,  0,  1,  0, 56,    0, 48,       0, 35,    0, 35,  //  22
 0, 52,  0,  1,  0, 50,    0, 42,       0, 29,    0, 29,  //  23
 0, 47,  0,  1,  0, 45,    0, 37,       0, 24,    0, 24,  //  24
 0, 41,  0,  1,  0, 40,    0, 32,       0, 19,    0, 19,  //  25
 0, 35,  0,  1,  0, 33,    0, 25,       0, 12,    0, 12,  //  26
 0, 30,  0,  1,  0, 28,    0, 20,       0, 7,     0, 7,   //  27
 0, 24,  0,  1,  0, 22,    0, 14,       0, 7,     0, 7,    //  28
 0, 19,  0,  1,  0, 17,    0, 9,        0, 7,     0, 7,    //  29
 0, 16,  0,  1,  0, 14,    0, 6,        0, 7,     0, 7,    //  30
 0, 14,  0,  1,  0, 17,    0, 9,        0, 7,     0, 7,    //  31
 0, 13,  0,  1,  0, 14,    0, 6,        0, 7,     0, 7,    //  32

};
/*******************************************************************************
** Name: ACodec_po_dac_hp_test
** Input:
** Return:
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
void ACodec_dac_lineout_Deinit()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, LO_ANTIPOP_ENABLE); // set lo antipop_en

    for (i = 0; i < 16; i++) {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5);
        antipop_bit_tmp = i;
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, antipop_bit_tmp | LO_ANTIPOP_ENABLE);
        DelayMs(1);
    }
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_DOWN | PWD_DACL_DOWN| PWD_DACR_DOWN);
    DelayMs(5);

    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0xF | LO_ANTIPOP_ENABLE | PWD_LO_OSTG_DOWN);
    DelayMs(1);
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0xF | LO_ANTIPOP_ENABLE | PWD_LO_OSTG_DOWN | PWD_LO_BUF_DOWN);


}
/*******************************************************************************
** Name: ACodec_po_dac_hp_test
** Input:
** Return:
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
void ACodec_dac_lineout_init()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;
    if(Codec_Power_Status == Codec_Power_null)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0x60);           // pwd ostg and buf
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0x40); // pwo buf
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0x00); // pwo ostg
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, LO_ANTIPOP_ENABLE); // set lo antipop_en

        for (i = 0; i < 16; i++) {
            antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5);
            antipop_bit_tmp = i;
            WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, antipop_bit_tmp | LO_ANTIPOP_ENABLE);
            DelayMs(1);
        }
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, 0x4); // power on dac ibias/l/r
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_DISABLE);    //DAC unmute
        DelayMs(1);

        ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
        Codec_SetSampleRate(FS_8000Hz);
        Codec_SetMode(Codec_DACoutHP,FS_8000Hz);
        DelayMs(50);
        Codec_ExitMode(Codec_DACoutHP);
    }
}

/* add just for os ilde codec resume*/
void ACodec_dac_lineout_resume_init()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;
    if(Codec_Power_Status == Codec_Power_null)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0x60);           // pwd ostg and buf
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0x40); // pwo buf
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, 0x00); // pwo ostg
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, LO_ANTIPOP_ENABLE); // set lo antipop_en

        for (i = 0; i < 16; i++) {
            antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5);
            antipop_bit_tmp = i;
            WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, antipop_bit_tmp | LO_ANTIPOP_ENABLE);
            DelayMs(1);
        }
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, 0x4); // power on dac ibias/l/r
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_DISABLE);    //DAC unmute
        DelayMs(1);
    }
}


void ACodec_dac_hp_Deinit()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, HP_ANTIPOP_DISABLE, HP_ANTIPOP_ENABLE); // set hp antipop en

    for (i = 0; i < 16; i++) {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4);
        antipop_bit_tmp = i;
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, antipop_bit_tmp | HP_ANTIPOP_ENABLE);
        DelayMs(1);
    }
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG0,(0xff << 0),0xD7);
    DelayMs(5);

    #if 0
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0x9); // 1 0 1 1  clr hp vgnd pwd
    DelayMs(5);
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0xb); // 1 0 0 1 clr hp buf pwd
    DelayMs(5);
    #endif
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0xf); // 0 0 0 0 clr hp short out pwd
    DelayMs(5);

}
void ACodec_dac_hp_init()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;

    if(Codec_Power_Status == Codec_Power_null)
    {
        #if 0
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0xb); // 1 0 1 1  clr hp vgnd pwd
        DelayMs(5);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0x9); // 1 0 0 1 clr hp buf pwd
        DelayMs(5);
        #endif

        MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG0,(0x3f << 0),0x17);
        DelayMs(5);
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, HP_ANTIPOP_DISABLE, HP_ANTIPOP_ENABLE); // set hp antipop en

        for (i = 0; i < 16; i++) {
            antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4);
            antipop_bit_tmp = i;
            WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, antipop_bit_tmp | HP_ANTIPOP_ENABLE);
            DelayMs(1);
        }

        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0x0); // 1011 VGND PU
        #if 0
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0x1); // 0 0 0 1 clr hp ostg pwd
        DelayMs(1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3, 0x0); // 0 0 0 0 clr hp short out pwd
        DelayMs(5);
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, 0x4); // power on dac ibias/l/r
        #endif
    }

}
/*******************************************************************************
** Name: ACodec_pd_dac_lo_test
** Input:
** Return:
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
uint8 ACodec_get_over_current_value(void)
{
   uint8 config;
   config = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG2);
   return config;
}
/*******************************************************************************
** Name: ACodec_pd_dac_lo_test
** Input:
** Return:
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
void ACodec_get_dac_vol()
{
    uint8 L1,L2,R1,R2;

    while(1)
    {
        L1 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTL);
        R1 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTR);

        DelayMs(5);

        L2 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTL);
        R2 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTR);

        if((L1 == L2) && (R1 == R2))
            break;

    }
}
void ACodec_po_dac_lo()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
    DelayMs(1);

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_ON | PWD_DACL_ON | PWD_DACR_ON); // power down dac ibias/l/r
    DelayMs(1);
    for (i = 0; i < 16; i++) {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5);
        antipop_bit_tmp = (antipop_bit_tmp & 0xf0) | (0xf - i);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, antipop_bit_tmp | LO_ANTIPOP_ENABLE);
        DelayMs(1);
    }
    DelayMs(1);
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE, DACMTE_DISABLE); // unmute
    DelayMs(1);
}
void ACodec_pd_dac_lo()
{
    uint32 i = 0;
    uint32 antipop_bit_tmp = 0;
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
    DelayMs(1);
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, LO_ANTIPOP_ENABLE); // clr lo antipop_en
    DelayMs(1);
    for (i = 0; i < 16; i++) {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5);
        antipop_bit_tmp =  i;
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, antipop_bit_tmp | LO_ANTIPOP_ENABLE);
        DelayMs(1);
    }

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_DOWN | PWD_DACL_DOWN | PWD_DACR_DOWN); // power down dac ibias/l/r
    DelayMs(1);
}
void ACodec_po_dac_hp()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_ON | PWD_DACL_ON | PWD_DACR_ON); // power down dac ibias/l/r

    for (i = 0; i < 16; i++) {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4);
        antipop_bit_tmp = (antipop_bit_tmp & 0xf0) | (0xf - i);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, antipop_bit_tmp | HP_ANTIPOP_ENABLE);
        DelayMs(5);
    }

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_DISABLE);    //DAC unmute

}
void ACodec_pd_dac_hp()
{
    uint32 i = 0;
    uint32 antipop_bit_tmp = 0;


    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, HP_ANTIPOP_DISABLE, HP_ANTIPOP_ENABLE); // set hp antipop en

    for (i = 0; i < 16; i++) {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4);
        antipop_bit_tmp = (antipop_bit_tmp & 0xf0) | i;
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, antipop_bit_tmp | HP_ANTIPOP_ENABLE);
        DelayMs(1);
    }

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_DOWN | PWD_DACL_DOWN | PWD_DACR_DOWN); // power down dac ibias/l/r

}
/*******************************************************************************
** Name: ACodec_ADC2DAC_MIX
** Input:CodecMIX_Mode_t MIX_en 0-ENABLE;1-DISABLE
** Return: void
*******************************************************************************/
void ACodec_ADC2DAC_MIX(CodecMIX_Mode_t MIX_en)
{
    if(MIX_en == CodecMIX_ENABLE)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_MIXCTRL,MIXE_ENABLE);    //MIX
    }
    else
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_MIXCTRL,MIXE_DISABLE);    //NO MIX
    }

}
/*******************************************************************************
** Name: ACodec_Get_DAC_MTST
** Input:void
** Return: MUTE status
*******************************************************************************/
bool ACodec_Get_DAC_MTST(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_DACST);
    if(((config >> 4) & 0x1) == 0x1)
        return TRUE;
    else
        return FALSE;
}
/*******************************************************************************
** Name: ACodec_DACUnMute
** Input:void
** Return: void
*******************************************************************************/
void ACodec_DACUnMute(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_DISABLE);    //DAC Unmute
}
/*******************************************************************************
** Name: ACodec_DACMute
** Input:void
** Return: void
*******************************************************************************/
void ACodec_DACMute(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
}
/*******************************************************************************
** Name: ACodec_Get_DAC_DigVol_L
** Input:void
** Return: DigVol_L vol
*******************************************************************************/
uint8 ACodec_Get_DAC_DigVol_L(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTL);
    return config;
}
/*******************************************************************************
** Name: ACodec_Get_DAC_DigVol_R
** Input:void
** Return: DigVol_R vol
*******************************************************************************/
uint8 ACodec_Get_DAC_DigVol_R(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTR);
    return config;

}
/*******************************************************************************
** Name: ACodec_Set_DAC_DigVol
** Input:vol 0.375db/step: (0-0xff)0dB - -95dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_DAC_DigVol(uint8 vol)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACVCTLL,vol);    //DAC left channel volume
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACVCTLR,vol);    //DAC right channel volume
    return vol;
}
/*******************************************************************************
** Name: ACodec_Get_ADC_DigVol_L
** Input:void
** Return: ADC_DigVol_L
*******************************************************************************/
uint8 ACodec_Get_ADC_DigVol_L(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_ADCVSTL);
    return config;
}
/*******************************************************************************
** Name: ACodec_Get_ADC_DigVol_R
** Input:void
** Return: ADC_DigVol_R
*******************************************************************************/
uint8 ACodec_Get_ADC_DigVol_R(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_ADCVSTR);
    return config;

}
/*******************************************************************************
** Name: ACodec_Set_ADC_DigVol
** Input:vol 0.375db/step: (0-0xff)0dB - -95dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_ADC_DigVol(uint8 vol)
{
    uint32 config;
    //DEBUG("ACodec_Set_ADC_DigVol = %d",vol);
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCVCTLL,vol);    //ADC left channel volume
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCVCTLR,vol);    //ADC right channel volume
    return vol;

}
/*******************************************************************************
** Name: ACodec_Set_MIC_AnaVol
** Input:   0-3
         2'b00: 0dB
         2'b01: 10dB
         2'b10: 20dB
         2'b11: 30dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_MIC_AnaVol(uint8 vol)
{
    uint8 vol_l,vol_r;
    uint32 config;

    if(vol > 3)
        return 0;

    vol_r = vol << 0;
    vol_l = vol << 2;

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,0xF,vol_r | vol_l);
    return vol;

}
/*******************************************************************************
** Name: ACodec_Set_ADCMUX_Vol
** Input:   0-15  -18dB to 27dB, 3db/step.
** Return:
*******************************************************************************/
uint8 ACodec_Set_ADCMUX_Vol(uint8 vol)
{
    uint8 vol_l,vol_r;

    if(vol > 15)
        return 0;

    vol_r = vol << 0;
    vol_l = vol << 4;
    //DEBUG("ACodec_Set_ADCMUX_Vol = %d",vol);

    WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG2,vol_r | vol_l);
    return vol;

}
/*******************************************************************************
** Name: ACodec_Get_SRT_TIME
** Input:pll_target
** Return: void
*******************************************************************************/
uint32 ACodec_Get_SRT_TIME(CodecFS_en_t CodecFS)
{
    uint32 DACSRT_TIME = 0;
    switch (CodecFS)
    {
        case FS_8000Hz:
        case FS_11025Hz:
        case FS_12KHz:
            DACSRT_TIME = 0;
            break;

        case FS_16KHz:
        case FS_22050Hz:
        case FS_24KHz:
            DACSRT_TIME = 1;
            break;

        case FS_32KHz:
        case FS_44100Hz:
        case FS_48KHz:
            DACSRT_TIME = 2;
            break;
        case FS_64KHz:
        case FS_88200Hz:
        case FS_96KHz:
            DACSRT_TIME = 3;
            break;

        case FS_128KHz:
        case FS_1764KHz:
        case FS_192KHz:
            DACSRT_TIME = 4;
            break;
        default:
            break;
    }
    return DACSRT_TIME;

}
/*******************************************************************************
** Name: ACodec_Set_HP_Gain
** Input:vol 1db/step: (0-0xff)0dB - -15dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_HP_Gain(uint8 vol)
{
    //DEBUG("ACodec_Set_HP_Gain = %d",vol);
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, vol | HP_ANTIPOP_ENABLE);
    return vol;
}
/*******************************************************************************
** Name: ACodec_Set_LO_Gain
** Input:vol 1db/step: (0-0xff)0dB - -15dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_LO_Gain(uint8 vol)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, vol | LO_ANTIPOP_ENABLE);
    return vol;
}
/*******************************************************************************
** Name: ACodec_Set_HP_AMP
** Input:vol
** Return: void
*******************************************************************************/
void ACodec_Set_HP_AMP(eACodecHp_AMP vol)
{
    uint32 config;
    if(vol != 0)
    {
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, HP_ANTIPOP_ENABLE, HP_ANTIPOP_DISABLE);
    }
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG1,(0x3 << 3),vol << 3);
}
/*******************************************************************************
** Name: ACodec_Set_LO_AMP
** Input:vol
** Return: void
*******************************************************************************/
void ACodec_Set_LO_AMP(eACodecLo_AMP vol)
{
    uint32 config;
    if(vol != 0)
    {
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, LO_ANTIPOP_DISABLE, LO_ANTIPOP_ENABLE);
    }
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG0,(0x3 << 2),vol << 2);
}

/*******************************************************************************
** Name: ACodec_I2s_SetSampleRate
** Input:Sck_div
** Return: void
*******************************************************************************/
void ACodec_I2s_SetSampleRate(uint32 Sck_div)
{
    uint32 config;
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_I2SCKM,(0xF << 4),(Sck_div << 4));
}
/*******************************************************************************
** Name: ACodec_PLL_Set
** Input:pll_target
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_Soft_Reset(void)
{
    uint32 timeout = 200000;
    WriteAcodecReg(ACODEC_BASE+ACODEC_SRST,SOFT_RESET);

    while(ReadAcodecReg(ACODEC_BASE+ACODEC_SRST))
    {
        if (--timeout == 0)
        {
            break;
        }
    }
    //DEBUG("ACodec_Soft_Reset");
}
/*******************************************************************************
** Name: ACodec_PLL_Set
** Input:pll_target
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_PLL_Set(eF_SOURCE_ID F_SOURCE,eACodecPll_Target_Freq pll_target)
{
    uint32 MHz,POSDIV_L3,POSDIV_H8,PREDIV_BIT,OUTDIV;
    uint32 config,i;

    if (F_SOURCE >= F_SOURCE_MAX)
    {
        return;
    }

    if(pll_old_target == pll_target)
    {
       return;  //如果PLL一样就不需要重新设置，只需要配置SCK_DIV即可
    }

    if(ACodec_I2S_MASTER_MODE == Acodec_I2S_MODE)
    {
        switch (pll_target)
        {
            case Pll_Target_Freq_40960:
                POSDIV_L3 = ACodecpllTable_40960[F_SOURCE].PLL_POSDIV_L3;
                POSDIV_H8 = ACodecpllTable_40960[F_SOURCE].PLL_POSDIV_H8;
                PREDIV_BIT = ACodecpllTable_40960[F_SOURCE].PLL_PREDIV_BIT;
                OUTDIV = ACodecpllTable_40960[F_SOURCE].PLL_OUTDIV;
                break;

            case Pll_Target_Freq_56448:
                POSDIV_L3 = ACodecpllTable_56448[F_SOURCE].PLL_POSDIV_L3;
                POSDIV_H8 = ACodecpllTable_56448[F_SOURCE].PLL_POSDIV_H8;
                PREDIV_BIT = ACodecpllTable_56448[F_SOURCE].PLL_PREDIV_BIT;
                OUTDIV = ACodecpllTable_56448[F_SOURCE].PLL_OUTDIV;
                break;

            case Pll_Target_Freq_61440:
                POSDIV_L3 = ACodecpllTable_61440[F_SOURCE].PLL_POSDIV_L3;
                POSDIV_H8 = ACodecpllTable_61440[F_SOURCE].PLL_POSDIV_H8;
                PREDIV_BIT = ACodecpllTable_61440[F_SOURCE].PLL_PREDIV_BIT;
                OUTDIV = ACodecpllTable_61440[F_SOURCE].PLL_OUTDIV;
                break;

            default:
                break;
        }
        pll_old_target = pll_target;

        //disable out vco
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,PLL_OUTDIV_ENABLE,PLL_OUTDIV_DISABLE);

        //PLL RESET pwd down
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,APLL_RELEASE_RESET,APLL_RESET);
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_UP,PLL_PW_DOWN);

        if(F_SOURCE == F_SOURCE_24000KHz)
        {
            //config = ReadAcodecReg(ACODEC_BASE+ACODEC_PLLCFG0);
            //Select 24M as the source clock of PLL
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,(0x03 << 3),PLL_CLKIN_SEL_S_CLOCK);

            WriteAcodecReg(ACODEC_BASE+ACODEC_PLLCFG1,POSDIV_L3);
            WriteAcodecReg(ACODEC_BASE+ACODEC_PLLCFG2,POSDIV_H8);
            WriteAcodecReg(ACODEC_BASE+ACODEC_PLLCFG3,PREDIV_BIT);
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG4,(0xf << 4),(OUTDIV << 4));

            //PLL power on

            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_DOWN,PLL_PW_UP);
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,APLL_RESET,APLL_RELEASE_RESET);


            //config = ReadAcodecReg(ACODEC_BASE+ACODEC_PLLCFG0);
            DelayMs(5);
            //enable out vco
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,PLL_OUTDIV_DISABLE,PLL_OUTDIV_ENABLE);
        }

    }
    else
    {
        //考虑到I2S无法分出全部的采样率，codec推荐做主，做从暂时不考虑
    }
}
/*******************************************************************************
** Name: ACodec_I2S_RX_Start
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_RX_Start(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCMD,RX_START);
}
/*******************************************************************************
** Name: ACodec_I2S_RX_Stop
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_RX_Stop(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCMD,RX_STOP|RX_CLEAR);
    DelayMs(5);
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCMD,RX_STOP);
}
/*******************************************************************************
** Name: ACodec_I2S_RX_Init
** Input:
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_RX_Init(eACodecI2sFormat_t BUS_FORMAT,
              eACodecI2sDATA_WIDTH_t I2S_Data_width,
              ACodecI2S_BUS_MODE_t I2S_Bus_mode)
{
    uint32 timeout = 20000;
    uint32 cofig_data = 0;

    ACodec_I2S_RX_Stop();

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCR2,I2S_Data_width);

    cofig_data = LSB_RX_MSB | EXRL_RX_NORMAL | PBM_RX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCR1,cofig_data);
    cofig_data = 0;

    cofig_data = RXRL_P_NORMAL | SCKD_RX_64_DIV;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCR0,cofig_data);
    cofig_data = 0;


}
/*******************************************************************************
** Name: ACodec_I2S_TX_Start
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_TX_Start(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCMD,TX_START);
}
/*******************************************************************************
** Name: ACodec_I2S_TX_Stop
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_TX_Stop(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCMD,TX_STOP|TX_CLEAR);
    //DEBUG("ACODEC_ADCCFG1 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_I2STXCMD));
    DelayMs(5);
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCMD,TX_STOP);
}

/*******************************************************************************
** Name: ACodec_I2S_TX_Init
** Input:
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_TX_Init(eACodecI2sFormat_t BUS_FORMAT,
              eACodecI2sDATA_WIDTH_t I2S_Data_width,
              ACodecI2S_BUS_MODE_t I2S_Bus_mode)
{
    uint32 timeout = 20000;
    uint32 cofig_data = 0;

    ACodec_I2S_TX_Stop();

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR3,0);  //RCNVT_TX = 0

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR2,I2S_Data_width);  //24bit/16bit

    cofig_data = LSB_TX_MSB | EXRL_TX_NORMAL | PBM_TX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR1,cofig_data);
    cofig_data = 0;

    cofig_data = TXRL_P_NORMAL | SCKD_TX_64_DIV;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR0,cofig_data);
    cofig_data = 0;


}
/*******************************************************************************
** Name: ACodec_Set_LineIn_Limiter
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_LineIn_Limiter()
{

}
/*******************************************************************************
** Name: ACodec_Set_DAC_Limiter
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_DAC_Limiter()
{

}
/*******************************************************************************
** Name: ACodec_Set_ADC_HighPassFilter
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_HighPassFilter()
{

}
/*******************************************************************************
** Name: ACodec_Set_ADC_NoiseGate
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_NoiseGate()
{

}
/*******************************************************************************
** Name: ACodec_Set_ADC_ALC
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_ALC()
{

}
/*******************************************************************************
** Name: ACodec_Exit_ADC_Mode
** Input:
** Return: void
*******************************************************************************/
void ACodec_Exit_ADC_Mode()
{
    uint32 config = 0;
    uint32 ADCSRT_NUM;
    uint32 timeout = 200000;

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_ADCCFG1,ADC_MUTE_L_DISABLE | ADC_MUTE_R_DISABLE,
                                                    ADC_MUTE_L_ENABLE | ADC_MUTE_R_ENABLE);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,ADC_CKE_ENABLE | I2STX_CKE_ENABLE,
                                                ADC_CKE_DISABLE | I2STX_CKE_DISABLE);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN , I2STX_EN_ENABLE, I2STX_EN_DISABLE );
    ACodec_I2S_TX_Stop();
    DelayMs(1);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN , ADC_EN_ENABLE, ADC_EN_DISABLE);
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0xD8);
    DelayMs(1);
    ACodec_Set_ADC_ALC();
}


/*******************************************************************************
** Name: ACodec_Set_ADC_Mode
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_MICLI_Mode(CodecIn_sel_t In_Mode,CodecMicBias_sel_t Mic_Bias,CodecMic_Mode_t MIC_Mode)
{
    uint32 config,Mic_Bias_old;

    switch (In_Mode)
    {
        case Codecin_Sel_MIC1_MONO:
            if(MIC_Mode == CodecMIC_Normal_Mode)
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC);
            }
            else
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC | MIC_L_DIFF_EN_ENABLE);
            }
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MIC_L_PD_ON  | MUX_L_PD_ON );
            Mic_Bias_old = ReadAcodecReg(ACODEC_BASE+ACODEC_LICFG0);
            Mic_Bias_old &= 0x70;
            if(Mic_Bias_old != Mic_Bias)
            {
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,MICBIAS_SEL_MASK,Mic_Bias);
                DelayMs(50);
            }
            break;
        case Codecin_Sel_MIC2_MONO:
            if(MIC_Mode == CodecMIC_Normal_Mode)
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_R_IN_SEL_MIC);
            }
            else
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_R_IN_SEL_MIC | MIC_R_DIFF_EN_ENABLE);
            }
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MIC_R_PD_ON | MUX_R_PD_ON);
            Mic_Bias_old = ReadAcodecReg(ACODEC_BASE+ACODEC_LICFG0);
            Mic_Bias_old &= 0x70;
            if(Mic_Bias_old != Mic_Bias)
            {
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,MICBIAS_SEL_MASK,Mic_Bias);
                DelayMs(50);
            }
            break;
        case Codecin_Sel_MIC_STERO:
            if(MIC_Mode == CodecMIC_Normal_Mode)
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC | MUX_R_IN_SEL_MIC);
            }
            else
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC | MUX_R_IN_SEL_MIC | MIC_R_DIFF_EN_ENABLE | MIC_L_DIFF_EN_ENABLE);
            }
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MIC_L_PD_ON | MIC_R_PD_ON | MUX_L_PD_ON | MUX_R_PD_ON);
            //select the output voltage of MIC bias
            Mic_Bias_old = ReadAcodecReg(ACODEC_BASE+ACODEC_LICFG0);
            Mic_Bias_old &= 0x70;
            if(Mic_Bias_old != Mic_Bias)
            {
                MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,MICBIAS_SEL_MASK,Mic_Bias);
                DelayMs(50);
            }
            break;

        case Codecin_Sel_LINE1:
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_LINE_1 | MUX_R_IN_SEL_LINE_1);
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MUX_L_PD_ON | MUX_R_PD_ON | MIC_L_PD_DOWN | MIC_R_PD_DOWN);
            //ACodec_Set_ADCMUX_Vol(12);

            break;
        case Codecin_Sel_LINE2:
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_LINE_2 | MUX_R_IN_SEL_LINE_2);
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MUX_L_PD_ON | MUX_R_PD_ON | MIC_L_PD_DOWN | MIC_R_PD_DOWN);

            break;
        default:
            break;
    }
}

/*******************************************************************************
** Name: ACodec_Set_ADC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_Mode(CodecIn_sel_t In_Mode,CodecFS_en_t CodecFS)
{
    uint32 config = 0;
    uint32 ADCSRT_NUM;
    uint32 timeout = 200000;
    //mute the left/right channel of ADC
    //WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1,0xc0);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_ADCCFG1,ADC_MUTE_L_DISABLE | ADC_MUTE_R_DISABLE,
                                                    ADC_MUTE_L_ENABLE | ADC_MUTE_R_ENABLE);

    ACodec_Set_MICLI_Mode(In_Mode,MicBias_sel_20V,CodecMIC_Diff_Mode); //mic/line config


    config |= ADCBYPS_DISABLE; //ADCBYPS DISABLE
    config |= ADCFADE_AS_ADCCZDT; //ADCFADE = 1
    config |= ADCCZDT_1; //volume adjusts only when audio waveform crosses zero or volume-control time-limit condition meets

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_VCTL,ADCBYPS_ENABLE,config);
    config = 0;

    //VCTIME NO CONFIG

    //ADCSR SET
    ADCSRT_NUM = ACodec_Get_SRT_TIME(CodecFS);
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCSR,ADCSRT_NUM);

    //ADC L/R power on
    if(In_Mode == Codecin_Sel_MIC1_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0x18 | ADC_R_PWD_DOWN);
    }
    else if(In_Mode == Codecin_Sel_MIC2_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0x18 | ADC_L_PWD_DOWN);
    }
    else
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0x18);
    }

    //enable ADC and i2s tx clk
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,ADC_CKE_DISABLE | I2STX_CKE_DISABLE,
                                                    ADC_CKE_ENABLE | I2STX_CKE_ENABLE);

    //enbale i2s_tx
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN, I2STX_EN_DISABLE , I2STX_EN_ENABLE );

    // tx transfer start
    ACodec_I2S_TX_Start();

    //wait 1ms
    //DelayMs(1);

    //enable adc
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN, ADC_EN_DISABLE , ADC_EN_ENABLE );

    //wait 1ms
    DelayMs(1);

    //unmute the left/right channel of ADC
    if(In_Mode == Codecin_Sel_MIC1_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1, 0x00 | ADC_MUTE_R_ENABLE);
    }
    else if(In_Mode == Codecin_Sel_MIC2_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1, 0x00 | ADC_MUTE_L_ENABLE);
    }
    else
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1, 0x00);
    }
    DelayMs(1);
    //SET ALC
    ACodec_Set_ADC_ALC();
}
/*******************************************************************************
** Name: ACodec_Set_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_WriteReg(uint32 regaddr,uint32 data)
{
    WriteAcodecReg(regaddr,data);
    ACodec_ReadReg_Debug(regaddr);
}
/*******************************************************************************
** Name: ACodec_Set_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_ReadReg_Debug(uint32 regaddr)
{
    //DEBUG("reg data = %d",ReadAcodecReg(regaddr));
}
/*******************************************************************************
** Name: ACodec_Exit_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Exit_DAC_Mode()
{
    //auto power down
    if(ACODEC_OUT_CONFIG == ACODEC_OUT_HP)
    {
        ACodec_pd_dac_hp();
    }
    else if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
    {
        ACodec_pd_dac_lo();
    }

    //disable dac
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,DAC_EN_ENABLE,DAC_EN_DISABLE);

    //disable I2S RX
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,I2SRX_EN_ENABLE,I2SRX_EN_DISABLE);

    ACodec_I2S_RX_Stop();

    //disable dac clk ;disable i2s_rx clk
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,I2SRX_CKE_ENABLE,I2SRX_CKE_DISABLE);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,DAC_CKE_ENABLE,DAC_CKE_DISABLE);

}
/*******************************************************************************
** Name: ACodec_Set_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_DAC_Mode(CodecOut_sel_t OUT_Mode,CodecFS_en_t CodecFS)
{
    uint32 config = 0;
    uint32 DACSRT_NUM;
    uint32 timeout = 200000;

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute

    config |= DACBYPS_DISABLE; //DACBYPS DISABLE
    config |= DACFADE_AS_DACCZDT; //DACFADE = 1
    config |= DACCZDT_1; //volume adjusts only when audio waveform crosses zero or volume-control time-limit condition meets

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_VCTL,DACBYPS_ENABLE,config);
    config = 0;

    //VCTIME NO CONFIG

    DACSRT_NUM = ACodec_Get_SRT_TIME(CodecFS);

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACSR,DACSRT_NUM);

    //DAC L/R power on
    //enable dac clk ;enbale i2s_rx clk
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,DAC_CKE_DISABLE | I2SRX_CKE_DISABLE,
                                                DAC_CKE_ENABLE | I2SRX_CKE_ENABLE);


    //enbale i2s_rx
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,I2SRX_EN_DISABLE,I2SRX_EN_ENABLE);

    ACodec_I2S_RX_Start();

    DelayMs(5);

    //enbale dac
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,DAC_EN_DISABLE,DAC_EN_ENABLE);
    DelayMs(1);


    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG1,(0x1 << 0),0x1);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG0,(0xff << 0),0x1);
    DelayMs(1);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG1,(0x7 << 0),0x1);
    DelayMs(1);

    Codec_Power_Status = Codec_Power_on;
    //power on analog part of DAC
    if(ACODEC_OUT_CONFIG == ACODEC_OUT_HP)
    {
        ACodec_po_dac_hp();
    }
    else if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
    {
        ACodec_po_dac_lo();
    }


}
/*******************************************************************************
** Name: ACodec_Set_HPLO_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_HPLO_Mode(CodecOut_sel_t OUT_Mode)
{
    uint32 config;

    switch (OUT_Mode)
    {
        case CodecOut_Sel_HP:

            break;

        case CodecOut_Sel_LINE:

            break;

        default:
            break;
    }
}
/*******************************************************************************
** Name: ACodec_I2S_Init
** Input:
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_Set_I2S_Mode(eACodecI2sFormat_t BUS_FORMAT,
              eACodecI2sDATA_WIDTH_t I2S_Data_width,
              ACodecI2S_BUS_MODE_t I2S_Bus_mode,
              ACodecI2S_mode_t I2S_mode)
{
    Acodec_I2S_MODE = I2S_mode;
    ACodec_I2S_TX_Init(BUS_FORMAT,I2S_Data_width,I2S_Bus_mode);
    ACodec_I2S_RX_Init(BUS_FORMAT,I2S_Data_width,I2S_Bus_mode);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_I2SCKM,(I2S_MST_MASTER) | (SCK_EN_DISABLE),
                                                I2S_mode | SCK_EN_ENABLE);

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_PowerOnInitial(void)
  Author        : yangwenjie
  Description   : Codec power on initial

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
//_ATTR_SYS_INIT_CODE_
void Codec_PowerOnInitial(void)
{
    ScuClockGateCtr(CLK_ACODEC_GATE, 1);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 1);     //PCLK ACODEC gating open
    ScuSoftResetCtr(ACODEC_SRST0, 0);

    Acodec_I2S_MODE    = ACodec_I2S_MASTER_MODE;
    pll_old_target     = Pll_Target_Freq_NULL;
    Acodec_suspend_En  = 0;
    Codec_Power_Status = Codec_Power_null;
    ACodec_Soft_Reset();

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x7);
    DelayMs(5);

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_DOWN | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(200);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(200);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    DelayMs(500);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_ON);

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x1);

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACPOPD,ATPCE_DISABLE | SMTPO_DOWN | ANTIPOP_DISABLE);

}

void Codec_PowerOnInit_Start(void)
{
    ScuClockGateCtr(CLK_ACODEC_GATE, 1);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 1);     //PCLK ACODEC gating open
    ScuSoftResetCtr(ACODEC_SRST0, 0);

    Acodec_I2S_MODE    = ACodec_I2S_MASTER_MODE;
    pll_old_target     = Pll_Target_Freq_NULL;
    Acodec_suspend_En  = 0;
    Codec_Power_Status = Codec_Power_null;
    ACodec_Soft_Reset();

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x7);
    DelayMs(5);

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_DOWN | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    //DelayMs(200);
}

void Codec_PowerOnInit_Step2(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    //DelayMs(200);
}

void Codec_PowerOnInit_Step3(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    //DelayMs(500);
}
void Codec_PowerOnInit_End(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_ON);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x1);
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACPOPD,ATPCE_DISABLE | SMTPO_DOWN | ANTIPOP_DISABLE);
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DeIitial(void)
  Author        : yangwenjie
  Description   : close Codec

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DeInitial(void)
{
#if 1
    Acodec_suspend_En = 0;
    Codec_Suspend();
#else
    ACodec_Exit_DAC_Mode();

#ifdef HP_DET_CONFIG
    ACodec_dac_hp_Deinit();
    ACodec_dac_lineout_Deinit();
#else
    if (ACODEC_OUT_CONFIG == ACODEC_OUT_HP)
    {
       ACodec_dac_hp_Deinit();
    }
    else if (ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
    {
       ACodec_dac_lineout_Deinit();
    }
#endif

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    DelayMs(50);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(50);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_DOWN | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(50);

    //PLL RESET pwd down
    Codec_Power_Status = Codec_Power_null;
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,PLL_OUTDIV_ENABLE,PLL_OUTDIV_DISABLE);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,APLL_RELEASE_RESET,APLL_RESET);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_UP,PLL_PW_DOWN);

    ScuSoftResetCtr(ACODEC_SRST0, 1);
    ScuClockGateCtr(CLK_ACODEC_GATE, 0);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 0);     //PCLK ACODEC gating open
#endif
}
_ATTR_DRIVER_CODE_
void Codec_PowerOn(void)
{
    if(Codec_Power_Status == Codec_Power_down)
    {
        Codec_Power_Status = Codec_Power_on;
        //DEBUG("Codec_PowerOn");
        ACodec_pd_dac_lo();
        ACodec_po_dac_hp();
    }
}
_ATTR_DRIVER_CODE_
void Codec_PowerDown(void)
{
    if(Codec_Power_Status == Codec_Power_on)
    {
        Codec_Power_Status = Codec_Power_down;
        ACodec_pd_dac_hp();
        ACodec_po_dac_lo();
        //DEBUG("Codec_PowerDown");
    }
}
_ATTR_DRIVER_CODE_
void Codec_Suspend(void)
{
   //ACODEC CLOCK gate close
   //G0_14 and G9_16
   //DEBUG("Codec_Suspend");

   if(Acodec_suspend_En == 0)
   {
       Acodec_suspend_En = 1;

       MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG0,(0xff << 0),0xD7);

       ACodec_Exit_DAC_Mode();

#ifdef HP_DET_CONFIG
       ACodec_dac_hp_Deinit();
       ACodec_dac_lineout_Deinit();
#else
       if (ACODEC_OUT_CONFIG == ACODEC_OUT_HP)
       {
           ACodec_dac_hp_Deinit();
       }
       else if (ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
       {
           ACodec_dac_lineout_Deinit();
       }
#endif

       WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
       DelayMs(50);
       WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
       DelayMs(50);
       WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_DOWN | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
       DelayMs(50);

       //PLL RESET pwd down
       Codec_Power_Status = Codec_Power_null;
       MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,PLL_OUTDIV_ENABLE,PLL_OUTDIV_DISABLE);

       MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,APLL_RELEASE_RESET,APLL_RESET);
       MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_UP,PLL_PW_DOWN);

        //ScuSoftResetCtr(ACODEC_SRST0, 1);
       ScuClockGateCtr(CLK_ACODEC_GATE, 0);      //ACODEC gating open
       ScuClockGateCtr(PCLK_ACODEC_GATE, 0);     //PCLK ACODEC gating open
   }

}

_ATTR_DRIVER_CODE_
void Codec_Resume(void)
{
    //DEBUG("Codec_Resume");

   if(Acodec_suspend_En == 1)
   {
        Acodec_suspend_En = 0;
        ScuClockGateCtr(CLK_ACODEC_GATE, 1);      //ACODEC gating open
        ScuClockGateCtr(PCLK_ACODEC_GATE, 1);     //PCLK ACODEC gating open
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_DOWN,PLL_PW_UP);
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,APLL_RESET,APLL_RELEASE_RESET);

        //config = ReadAcodecReg(ACODEC_BASE+ACODEC_PLLCFG0);
        //DelayMs(5);
        //enable out vco
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,PLL_OUTDIV_DISABLE,PLL_OUTDIV_ENABLE);
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG0,(0x3f << 0),0x01);

        WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x7);
        DelayMs(5);
        WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
        DelayMs(200);
        WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
        DelayMs(500);
        WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_ON);

        WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x1);
        WriteAcodecReg(ACODEC_BASE+ACODEC_DACPOPD,ATPCE_DISABLE | SMTPO_DOWN | ANTIPOP_DISABLE);

        #if 1
        #ifdef HP_DET_CONFIG
        ACodec_dac_hp_init();
        ACodec_dac_lineout_init();
        #else
        if (ACODEC_OUT_CONFIG == ACODEC_OUT_HP)
        {
            ACodec_dac_hp_init();
        }
        else if (ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
        {
            ACodec_dac_lineout_resume_init();
        }
        #endif

        ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);

        Codec_SetSampleRate(CodecFS_Bak);
        Codec_SetMode(Codecmode_Bak,CodecFS_Bak);
        #endif

   }


}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   :

  Input         : Codecmode：

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetMode(CodecMode_en_t Codecmode,CodecFS_en_t CodecFS)
{

    Codecmode_Bak = Codecmode;
    CodecFS_Bak = CodecFS;

    switch (Codecmode)
    {
        case Codec_DACoutHP:
            ACodec_Set_DAC_Mode(CodecOut_Sel_HP,CodecFS);
            break;
        case Codec_DACoutLINE:
            ACodec_Set_DAC_Mode(CodecOut_Sel_LINE,CodecFS);
            break;

        case Codec_Line1ADC:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE1,CodecFS);
            //ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;

        case Codec_Line1in:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE1,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_ENABLE);
            break;
        case Codec_Line2ADC:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE2,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
        case Codec_Line2in:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE2,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_ENABLE);
            break;
        case Codec_MicStero:
            ACodec_Exit_DAC_Mode();
            Codec_Power_Status = Codec_Power_null;
            ACodec_pd_dac_hp();
            ACodec_pd_dac_lo();
            ACodec_dac_hp_Deinit();
            ACodec_dac_lineout_Deinit();

            ACodec_Set_ADC_Mode(Codecin_Sel_MIC_STERO,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
        case Codec_Mic1Mono:
            ACodec_Exit_DAC_Mode();
            Codec_Power_Status = Codec_Power_null;
            ACodec_pd_dac_hp();
            ACodec_pd_dac_lo();
            ACodec_dac_hp_Deinit();
            ACodec_dac_lineout_Deinit();

            ACodec_Set_ADC_Mode(Codecin_Sel_MIC1_MONO,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
         case Codec_Mic2Mono:
            ACodec_Exit_DAC_Mode();
            Codec_Power_Status = Codec_Power_null;
            ACodec_pd_dac_hp();
            ACodec_pd_dac_lo();
            ACodec_dac_hp_Deinit();
            ACodec_dac_lineout_Deinit();

            ACodec_Set_ADC_Mode(Codecin_Sel_MIC2_MONO,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
        default:

            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_ExitMode(CodecMode_en_t Codecmode)
{

    switch (Codecmode)
    {
        case Codec_DACoutHP:
            ACodec_Exit_DAC_Mode();
            break;
        case Codec_DACoutLINE:
            ACodec_Exit_DAC_Mode();
            break;

        case Codec_Line1ADC:
            ACodec_Exit_ADC_Mode();
            break;

        case Codec_Line1in:
            ACodec_Exit_ADC_Mode();
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
        case Codec_Line2ADC:
            ACodec_Exit_ADC_Mode();
            break;
        case Codec_Line2in:
            ACodec_Exit_ADC_Mode();
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
        case Codec_MicStero:
        case Codec_Mic1Mono:
        case Codec_Mic2Mono:
            ACodec_Exit_ADC_Mode();
            break;
        default:

            break;
    }

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetSampleRate(CodecFS_en_t CodecFS)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetSampleRate(CodecFS_en_t CodecFS)
{
    uint32 Sck_div,FS_value;
    FS_value = CodecFS;

    switch (CodecFS)
    {
        case FS_8000Hz:
        case FS_16KHz:
        case FS_32KHz:
        case FS_64KHz:
        case FS_128KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_40960);
            Sck_div = (128000 / FS_value) - 1 ;
            ACodec_I2s_SetSampleRate(Sck_div);
            break;

        case FS_12KHz:
        case FS_24KHz:
        case FS_48KHz:
        case FS_96KHz:
        case FS_192KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_61440);
            Sck_div = (192000 / FS_value) - 1;
            ACodec_I2s_SetSampleRate(Sck_div);
            break;
        case FS_11025Hz:
        case FS_22050Hz:
        case FS_44100Hz:
        case FS_88200Hz:
        case FS_1764KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_56448);
            Sck_div = (176400 / FS_value) - 1;
            ACodec_I2s_SetSampleRate(Sck_div);
            break;

        default:

            break;

    }
}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetVolumet(unsigned int Volume)
  Author        : yangwenjie
  Description   : codec control volume

  Input         : Volume

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      volume = 0 mean mute,
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetVolumetTable(unsigned int Volume)
{

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetVolumet(unsigned int Volume)
  Author        : yangwenjie
  Description   : codec control volume

  Input         : Volume

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      volume = 0 mean mute,
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetVolumet(unsigned int Volume)
{
   uint32 VolumeMode = EQ_NOR;

    if (Volume == 0)
    {
        if(VOLTAB_CONFIG == VOL_Europe)
        {
            if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
            {
                ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].HP_AMPVol);
                ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].Dac_DigVol);
                ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
            }
            else
            {
                ACodec_Set_HP_AMP(CodecConfig_Europe[Volume].HP_AMPVol);
                ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].Dac_DigVol);
                ACodec_Set_HP_Gain(CodecConfig_Europe[Volume].HP_ANTIPOPVol);
            }
        }
        else
        {
            if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
            {
                ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].HP_AMPVol);
                ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].Dac_DigVol);
                ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
            }
            else
            {
                ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].HP_AMPVol);
                ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].Dac_DigVol);
                ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
            }
        }
        Codec_DACMute();
    }
    else
    {
       // if(ACodec_Get_DAC_MTST())
        {
            Codec_DACUnMute();
        }
        #if 1
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            AudioInOut_Type  *pAudio  = &AudioIOBuf;
            RKEffect         *pEffect = &pAudio->EffectCtl;
            VolumeMode = pEffect->Mode;
        }
        #endif
        switch (VolumeMode)
        {
            case EQ_NOR:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].Dac_DigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].Dac_DigVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(CodecConfig_Europe[Volume].HP_AMPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].Dac_DigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].Dac_DigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].HP_AMPVol);
                    }
                }

                break;
            case EQ_POP:
            case EQ_HEAVY:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_POP_HEAVY_HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(CodecConfig_Europe[Volume].EQ_POP_HEAVY_HP_AMPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_POP_HEAVY_HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_POP_HEAVY_HP_AMPVol);
                    }
                }
                break;
            case EQ_JAZZ:
            case EQ_UNIQUE:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                   if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                   {
                       ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_JAZZ_UNIQUE_HP_AMPVol);
                       ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_JAZZ_UNIQUE_DacDigVol);
                       ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                       ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_JAZZ_UNIQUE_HP_AMPVol);
                   }
                   else
                   {
                       ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].EQ_JAZZ_UNIQUE_DacDigVol);
                       ACodec_Set_HP_Gain(CodecConfig_Europe[Volume].HP_ANTIPOPVol);
                       ACodec_Set_HP_AMP(CodecConfig_Europe[Volume].EQ_JAZZ_UNIQUE_HP_AMPVol);
                   }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_JAZZ_UNIQUE_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_JAZZ_UNIQUE_HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_JAZZ_UNIQUE_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_JAZZ_UNIQUE_HP_AMPVol);
                    }
                }
                break;

            case EQ_USER:

                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_USER_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_USER_HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].EQ_USER_DacDigVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(CodecConfig_Europe[Volume].EQ_USER_HP_AMPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_USER_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_USER_HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_USER_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_USER_HP_AMPVol);
                    }
                }
                break;

            case EQ_BASS:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_BASS_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_BASS_HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].EQ_BASS_DacDigVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(CodecConfig_Europe[Volume].EQ_BASS_HP_AMPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_BASS_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_BASS_HP_AMPVol);
                    }
                    else
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_BASS_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_BASS_HP_AMPVol);
                    }
                }

                break;

            default:
                break;
        }
    }

    DelayUs(10);

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACMute(void)
  Author        : yangwenjie
  Description   : set codec mute

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DACMute(void)
{
    ACodec_DACMute();
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DACUnMute(void)
{
    ACodec_DACUnMute();
}

/*
********************************************************************************
*
*                         End of Rockcodec.c
*
********************************************************************************
*/

