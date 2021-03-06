/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     rk_eq.c
*  Description:
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     06/12/29      1.0
*           HuWeiGuo     07/09/27      2.0          修改了接口函数
*
*******************************************************************************/

/*
 TODO:
 1. 将几个表格放到FLASH里，仅在初始化EQ读系数时使用
 2. 相应修改ROCKEQ_Get_Coeff()函数
*/
//#include "SysConfig.h"
#include "audio_main.h"
#include "effect.h"
#include "Rk_eq.h"
#define     _ATTR_RKEQ_TEXT_          __attribute__((section("RkEqCode")))
#define     _ATTR_RKEQ_DATA_          __attribute__((section("RkEqData")))
#define     _ATTR_RKEQ_BSS_           __attribute__((section("RkEqBss"),zero_init))

#include "AudioControl.h"
extern AudioInOut_Type	   AudioIOBuf;


/*
 * If SUPPORT_HIGH_PRECISION = 1, we support high precison, in other words, has no precision lose,
 * If SUPPORT_HIGH_PRECISION = 0, we lose some precison, but it is much quicker.　
 */
#define SUPPORT_HIGH_PRECISION 1
//#include "typedef.h"
//#include "ModuleInfoTab.h"
/*
 *  Struct
 */

__align(4) IIRFilterState g_FilterState;
__align(4) IIRFilterState_BASS g_bass_FilterState;

typedef struct
{
    short a1;
    short a2;
    short b1;
    short b2;
    short B;
}eqCoef_t;

typedef struct
{
    int a1;
    int a2;
    int b1;
    int b2;
    int B;
}bassCoef_t;

typedef struct
{
    short eqArray[5*5*25*5+1];
    int   bass_60_tbl[6*5*5];
    int   bass_120_tbl[150];
} eq_table_t;

_ATTR_RKEQ_DATA_
eq_table_t Eq_Table =
{
//_ATTR_RKEQ_DATA_
//short eqArray[]=
    {
        //采样 0
//采样 0频段 0
        0x3d01,0xffffe2e5,0xffffc0db,0x1f3f,0x1eea,
        0x3d29,0xffffe2be,0xffffc0e7,0x1f34,0x1f04,
        0x3d4f,0xffffe298,0xffffc0f3,0x1f28,0x1f1d,
        0x3d72,0xffffe274,0xffffc0ff,0x1f1b,0x1f35,
        0x3d94,0xffffe252,0xffffc10c,0x1f0e,0x1f4d,
        0x3db4,0xffffe232,0xffffc11b,0x1f00,0x1f64,
        0x3dd2,0xffffe214,0xffffc129,0x1ef1,0x1f7b,
        0x3def,0xffffe1f7,0xffffc139,0x1ee1,0x1f92,
        0x3e0a,0xffffe1dc,0xffffc14a,0x1ed0,0x1fa8,
        0x3e24,0xffffe1c2,0xffffc15b,0x1ebf,0x1fbe,
        0x3e3d,0xffffe1a9,0xffffc16e,0x1eac,0x1fd4,
        0x3e54,0xffffe192,0xffffc182,0x1e98,0x1fea,
        0x3e6a,0xffffe17c,0xffffc196,0x1e84,0x2000,
        0x3e7e,0xffffe168,0xffffc1ac,0x1e6e,0x2015,
        0x3e92,0xffffe154,0xffffc1c3,0x1e57,0x202b,
        0x3ea5,0xffffe141,0xffffc1dc,0x1e3e,0x2042,
        0x3eb6,0xffffe130,0xffffc1f6,0x1e24,0x2058,
        0x3ec7,0xffffe11f,0xffffc211,0x1e09,0x206f,
        0x3ed7,0xffffe10f,0xffffc22e,0x1dec,0x2086,
        0x3ee5,0xffffe100,0xffffc24c,0x1dce,0x209e,
        0x3ef4,0xffffe0f2,0xffffc26c,0x1dae,0x20b6,
        0x3f01,0xffffe0e5,0xffffc28e,0x1d8c,0x20cf,
        0x3f0d,0xffffe0d8,0xffffc2b1,0x1d68,0x20e9,
        0x3f19,0xffffe0cc,0xffffc2d7,0x1d42,0x2103,
        0x3f25,0xffffe0c1,0xffffc2ff,0x1d1b,0x211f,
//采样 0 频段 1
        0x36d4,0xffffe846,0xffffc34b,0x1db3,0x1ce7,
        0x373b,0xffffe7de,0xffffc36c,0x1d91,0x1d2c,
        0x379d,0xffffe77b,0xffffc38f,0x1d6e,0x1d71,
        0x37fa,0xffffe71c,0xffffc3b4,0x1d48,0x1db5,
        0x3854,0xffffe6c1,0xffffc3db,0x1d21,0x1df7,
        0x38a9,0xffffe66a,0xffffc403,0x1cf7,0x1e39,
        0x38fb,0xffffe617,0xffffc42e,0x1ccc,0x1e7b,
        0x3949,0xffffe5c7,0xffffc45c,0x1c9e,0x1ebc,
        0x3994,0xffffe57c,0xffffc48b,0x1c6d,0x1efd,
        0x39db,0xffffe534,0xffffc4bd,0x1c3a,0x1f3d,
        0x3a1e,0xffffe4ef,0xffffc4f2,0x1c05,0x1f7e,
        0x3a5f,0xffffe4ad,0xffffc52a,0x1bcc,0x1fbe,
        0x3a9c,0xffffe46f,0xffffc564,0x1b91,0x2000,
        0x3ad6,0xffffe434,0xffffc5a1,0x1b53,0x2041,
        0x3b0e,0xffffe3fb,0xffffc5e2,0x1b11,0x2083,
        0x3b43,0xffffe3c6,0xffffc625,0x1acc,0x20c7,
        0x3b75,0xffffe393,0xffffc66c,0x1a84,0x210b,
        0x3ba4,0xffffe362,0xffffc6b7,0x1a39,0x2150,
        0x3bd2,0xffffe334,0xffffc705,0x19e9,0x2197,
        0x3bfd,0xffffe309,0xffffc757,0x1996,0x21e0,
        0x3c25,0xffffe2df,0xffffc7ac,0x193f,0x222b,
        0x3c4c,0xffffe2b8,0xffffc806,0x18e4,0x2278,
        0x3c71,0xffffe292,0xffffc863,0x1885,0x22c7,
        0x3c94,0xffffe26f,0xffffc8c5,0x1822,0x2319,
        0x3cb5,0xffffe24d,0xffffc92c,0x17ba,0x236d,
//采样 0 频段 2
        0x2532,0xfffff3d2,0xffffcf93,0x1984,0x1894,
        0x25da,0xfffff30b,0xffffcfdd,0x192d,0x1929,
        0x267e,0xfffff248,0xffffd02a,0x18d1,0x19c0,
        0x271e,0xfffff18a,0xffffd07b,0x1871,0x1a57,
        0x27ba,0xfffff0d1,0xffffd0d0,0x180c,0x1af0,
        0x2852,0xfffff01c,0xffffd128,0x17a3,0x1b8b,
        0x28e5,0xffffef6d,0xffffd184,0x1736,0x1c27,
        0x2974,0xffffeec3,0xffffd1e4,0x16c4,0x1cc5,
        0x29ff,0xffffee1f,0xffffd248,0x164d,0x1d65,
        0x2a85,0xffffed7f,0xffffd2b0,0x15d1,0x1e07,
        0x2b07,0xffffece5,0xffffd31d,0x1551,0x1eac,
        0x2b85,0xffffec50,0xffffd38d,0x14cb,0x1f54,
        0x2bfe,0xffffebc0,0xffffd402,0x1440,0x2000,
        0x2c73,0xffffeb35,0xffffd47b,0x13b0,0x20af,
        0x2ce3,0xffffeaaf,0xffffd4f9,0x131b,0x2162,
        0x2d50,0xffffea2f,0xffffd57b,0x1281,0x2219,
        0x2db8,0xffffe9b3,0xffffd601,0x11e1,0x22d5,
        0x2e1c,0xffffe93c,0xffffd68c,0x113d,0x2397,
        0x2e7c,0xffffe8ca,0xffffd71b,0x1093,0x245f,
        0x2ed8,0xffffe85d,0xffffd7ae,0xfe4,0x252d,
        0x2f30,0xffffe7f4,0xffffd846,0xf2f,0x2602,
        0x2f85,0xffffe78f,0xffffd8e2,0xe76,0x26df,
        0x2fd6,0xffffe72f,0xffffd982,0xdb8,0x27c4,
        0x3023,0xffffe6d3,0xffffda26,0xcf5,0x28b1,
        0x306d,0xffffe67c,0xffffdace,0xc2e,0x29a8,
//采样 0 频段 3
        0xfffff823,0xfffffca7,0xbd5,0x152c,0x1545,
        0xfffff7ef,0xfffffbbf,0xbb7,0x14a5,0x1609,
        0xfffff7bc,0xfffffad8,0xb97,0x1419,0x16d2,
        0xfffff789,0xfffff9f4,0xb77,0x1387,0x17a1,
        0xfffff757,0xfffff911,0xb55,0x12f1,0x1874,
        0xfffff725,0xfffff832,0xb33,0x1255,0x194e,
        0xfffff6f4,0xfffff756,0xb0f,0x11b4,0x1a2e,
        0xfffff6c4,0xfffff67d,0xaea,0x110e,0x1b14,
        0xfffff694,0xfffff5a8,0xac4,0x1063,0x1c01,
        0xfffff666,0xfffff4d7,0xa9d,0xfb2,0x1cf5,
        0xfffff638,0xfffff40a,0xa74,0xefc,0x1df0,
        0xfffff60c,0xfffff341,0xa4b,0xe42,0x1ef3,
        0xfffff5e0,0xfffff27d,0xa20,0xd83,0x2000,
        0xfffff5b5,0xfffff1be,0x9f4,0xcbf,0x2115,
        0xfffff58c,0xfffff104,0x9c8,0xbf6,0x2233,
        0xfffff563,0xfffff04e,0x99a,0xb29,0x235c,
        0xfffff53c,0xffffef9d,0x96c,0xa58,0x2490,
        0xfffff516,0xffffeef2,0x93c,0x983,0x25d0,
        0xfffff4f1,0xffffee4c,0x90c,0x8aa,0x271d,
        0xfffff4cd,0xffffedab,0x8db,0x7ce,0x2876,
        0xfffff4ab,0xffffed0f,0x8a9,0x6ef,0x29de,
        0xfffff489,0xffffec79,0x877,0x60c,0x2b56,
        0xfffff469,0xffffebe7,0x844,0x528,0x2cdd,
        0xfffff449,0xffffeb5b,0x811,0x441,0x2e77,
        0xfffff42b,0xffffead4,0x7dd,0x359,0x3022,
//采样 0 频段 4
        0x6296,0xffffa9d1,0xffffc3aa,0x2854,0x3449,
        0x5e21,0xffffaf29,0xffffc332,0x28e4,0x3189,
        0x5a47,0xffffb3c7,0xffffc2b1,0x297f,0x2f1e,
        0x56ea,0xffffb7ce,0xffffc226,0x2a26,0x2cf7,
        0x53f7,0xffffbb58,0xffffc190,0x2ada,0x2b08,
        0x515c,0xffffbe78,0xffffc0ed,0x2b9c,0x2947,
        0x4f0b,0xffffc13f,0xffffc03e,0x2c6f,0x27ab,
        0x4cf9,0xffffc3ba,0xffffbf80,0x2d53,0x2630,
        0x4b1e,0xffffc5f4,0xffffbeb2,0x2e4a,0x24ce,
        0x4972,0xffffc7f5,0xffffbdd1,0x2f56,0x2382,
        0x47ef,0xffffc9c5,0xffffbcdd,0x307b,0x2249,
        0x4690,0xffffcb6a,0xffffbbd3,0x31ba,0x211e,
        0x4550,0xffffcce8,0xffffbab0,0x3318,0x2000,
        0x442d,0xffffce46,0xffffb970,0x3496,0x1eeb,
        0x4323,0xffffcf85,0xffffb811,0x363b,0x1ddd,
        0x422f,0xffffd0aa,0xffffb68e,0x380b,0x1cd5,
        0x414e,0xffffd1b6,0xffffb4e2,0x3a0c,0x1bd2,
        0x4080,0xffffd2ad,0xffffb307,0x3c46,0x1ad0,
        0x3fc2,0xffffd391,0xffffb0f5,0x3ec1,0x19cf,
        0x3f13,0xffffd464,0xffffaea4,0x4188,0x18ce,
        0x3e70,0xffffd526,0xffffac09,0x44a8,0x17cb,
        0x3dda,0xffffd5da,0xffffa916,0x4832,0x16c5,
        0x3d4f,0xffffd681,0xffffa5b9,0x4c39,0x15bb,
        0x3cce,0xffffd71c,0xffffa1df,0x50d7,0x14ab,
        0x3c56,0xffffd7ac,0xffff9d6a,0x562f,0x1395,
//采样 1 频段 0
        0x3e7e,0xffffe17c,0xffffc068,0x1f9f,0x1f72,
        0x3e93,0xffffe167,0xffffc06e,0x1f99,0x1f7f,
        0x3ea6,0xffffe153,0xffffc074,0x1f93,0x1f8c,
        0x3eb9,0xffffe141,0xffffc07a,0x1f8c,0x1f98,
        0x3eca,0xffffe12f,0xffffc081,0x1f86,0x1fa4,
        0x3edb,0xffffe11e,0xffffc088,0x1f7f,0x1fb0,
        0x3eeb,0xffffe10f,0xffffc090,0x1f77,0x1fbc,
        0x3efa,0xffffe100,0xffffc098,0x1f6f,0x1fc8,
        0x3f08,0xffffe0f2,0xffffc0a0,0x1f66,0x1fd3,
        0x3f15,0xffffe0e4,0xffffc0a9,0x1f5d,0x1fde,
        0x3f22,0xffffe0d8,0xffffc0b3,0x1f54,0x1fe9,
        0x3f2d,0xffffe0cc,0xffffc0bd,0x1f4a,0x1ff4,
        0x3f39,0xffffe0c1,0xffffc0c7,0x1f3f,0x2000,
        0x3f43,0xffffe0b6,0xffffc0d3,0x1f34,0x200b,
        0x3f4d,0xffffe0ac,0xffffc0de,0x1f28,0x2016,
        0x3f57,0xffffe0a3,0xffffc0eb,0x1f1c,0x2021,
        0x3f60,0xffffe09a,0xffffc0f8,0x1f0e,0x202c,
        0x3f68,0xffffe091,0xffffc106,0x1f00,0x2038,
        0x3f70,0xffffe089,0xffffc115,0x1ef1,0x2043,
        0x3f78,0xffffe081,0xffffc125,0x1ee2,0x204f,
        0x3f7f,0xffffe07a,0xffffc136,0x1ed1,0x205c,
        0x3f86,0xffffe074,0xffffc147,0x1ebf,0x2068,
        0x3f8c,0xffffe06d,0xffffc15a,0x1ead,0x2075,
        0x3f92,0xffffe067,0xffffc16d,0x1e99,0x2082,
        0x3f98,0xffffe061,0xffffc182,0x1e84,0x2090,
//采样 1 频段 1
        0x3b52,0xffffe471,0xffffc16e,0x1ed2,0x1e56,
        0x3b8d,0xffffe435,0xffffc180,0x1ec1,0x1e7d,
        0x3bc5,0xffffe3fd,0xffffc192,0x1eae,0x1ea3,
        0x3bfb,0xffffe3c7,0xffffc1a6,0x1e9b,0x1ec8,
        0x3c2e,0xffffe394,0xffffc1ba,0x1e86,0x1eec,
        0x3c5e,0xffffe364,0xffffc1d0,0x1e70,0x1f10,
        0x3c8c,0xffffe336,0xffffc1e7,0x1e59,0x1f33,
        0x3cb7,0xffffe30a,0xffffc1ff,0x1e41,0x1f55,
        0x3ce1,0xffffe2e0,0xffffc219,0x1e27,0x1f78,
        0x3d08,0xffffe2b9,0xffffc234,0x1e0c,0x1f9a,
        0x3d2d,0xffffe293,0xffffc250,0x1df0,0x1fbc,
        0x3d51,0xffffe270,0xffffc26e,0x1dd1,0x1fde,
        0x3d72,0xffffe24e,0xffffc28e,0x1db2,0x2000,
        0x3d92,0xffffe22f,0xffffc2af,0x1d90,0x2022,
        0x3db0,0xffffe210,0xffffc2d3,0x1d6d,0x2044,
        0x3dcc,0xffffe1f4,0xffffc2f8,0x1d47,0x2066,
        0x3de7,0xffffe1d9,0xffffc31f,0x1d20,0x208a,
        0x3e01,0xffffe1bf,0xffffc349,0x1cf6,0x20ad,
        0x3e19,0xffffe1a7,0xffffc374,0x1cca,0x20d2,
        0x3e30,0xffffe190,0xffffc3a2,0x1c9c,0x20f7,
        0x3e46,0xffffe17a,0xffffc3d2,0x1c6c,0x211d,
        0x3e5a,0xffffe165,0xffffc405,0x1c39,0x2144,
        0x3e6e,0xffffe152,0xffffc43b,0x1c03,0x216c,
        0x3e80,0xffffe13f,0xffffc473,0x1bcb,0x2195,
        0x3e92,0xffffe12e,0xffffc4ae,0x1b8f,0x21c0,
//采样 1 频段 2
        0x31c8,0xffffec20,0xffffc5fd,0x1c73,0x1b75,
        0x3251,0xffffeb92,0xffffc62e,0x1c40,0x1bd8,
        0x32d4,0xffffeb09,0xffffc661,0x1c0b,0x1c3a,
        0x3353,0xffffea85,0xffffc697,0x1bd3,0x1c9b,
        0x33cd,0xffffea06,0xffffc6cf,0x1b98,0x1cfb,
        0x3442,0xffffe98c,0xffffc70b,0x1b5a,0x1d5c,
        0x34b3,0xffffe916,0xffffc749,0x1b19,0x1dbb,
        0x351f,0xffffe8a6,0xffffc78b,0x1ad5,0x1e1b,
        0x3586,0xffffe83a,0xffffc7d0,0x1a8d,0x1e7b,
        0x35e9,0xffffe7d3,0xffffc818,0x1a42,0x1edb,
        0x3648,0xffffe770,0xffffc863,0x19f3,0x1f3c,
        0x36a3,0xffffe711,0xffffc8b3,0x19a0,0x1f9d,
        0x36fa,0xffffe6b6,0xffffc906,0x194a,0x2000,
        0x374d,0xffffe660,0xffffc95d,0x18ef,0x2063,
        0x379d,0xffffe60d,0xffffc9b8,0x1890,0x20c8,
        0x37e8,0xffffe5be,0xffffca17,0x182d,0x212f,
        0x3830,0xffffe573,0xffffca7a,0x17c6,0x2197,
        0x3875,0xffffe52b,0xffffcae1,0x175a,0x2202,
        0x38b7,0xffffe4e7,0xffffcb4d,0x16ea,0x2270,
        0x38f5,0xffffe4a6,0xffffcbbe,0x1674,0x22e0,
        0x3931,0xffffe468,0xffffcc33,0x15fa,0x2354,
        0x3969,0xffffe42d,0xffffccad,0x157b,0x23cb,
        0x399f,0xffffe3f5,0xffffcd2c,0x14f7,0x2446,
        0x39d2,0xffffe3c0,0xffffcdaf,0x146e,0x24c6,
        0x3a03,0xffffe38d,0xffffce38,0x13e0,0x254a,
//采样 1 频段 3
        0x182f,0xfffff937,0xffffddb3,0x1704,0x168f,
        0x18ba,0xfffff857,0xffffddfb,0x1690,0x1742,
        0x1944,0xfffff77a,0xffffde47,0x1617,0x17f9,
        0x19cb,0xfffff6a1,0xffffde95,0x1599,0x18b3,
        0x1a50,0xfffff5cb,0xffffdee7,0x1516,0x1971,
        0x1ad3,0xfffff4f9,0xffffdf3c,0x148e,0x1a32,
        0x1b54,0xfffff42c,0xffffdf94,0x1401,0x1af8,
        0x1bd1,0xfffff362,0xffffdfef,0x136f,0x1bc2,
        0x1c4c,0xfffff29e,0xffffe04d,0x12d7,0x1c90,
        0x1cc4,0xfffff1dd,0xffffe0af,0x123b,0x1d64,
        0x1d38,0xfffff122,0xffffe114,0x1199,0x1e3d,
        0x1daa,0xfffff06c,0xffffe17c,0x10f2,0x1f1b,
        0x1e19,0xffffefba,0xffffe1e7,0x1046,0x2000,
        0x1e84,0xffffef0e,0xffffe256,0xf94,0x20eb,
        0x1eec,0xffffee67,0xffffe2c8,0xede,0x21dd,
        0x1f51,0xffffedc5,0xffffe33c,0xe23,0x22d7,
        0x1fb3,0xffffed29,0xffffe3b4,0xd62,0x23d8,
        0x2011,0xffffec91,0xffffe42f,0xc9e,0x24e3,
        0x206c,0xffffebff,0xffffe4ac,0xbd4,0x25f7,
        0x20c4,0xffffeb72,0xffffe52d,0xb07,0x2716,
        0x2119,0xffffeaea,0xffffe5b0,0xa35,0x283f,
        0x216b,0xffffea67,0xffffe635,0x95f,0x2974,
        0x21b9,0xffffe9e9,0xffffe6bc,0x886,0x2ab6,
        0x2205,0xffffe970,0xffffe746,0x7a9,0x2c05,
        0x224d,0xffffe8fc,0xffffe7d1,0x6c9,0x2d63,
//采样 1 频段 4
        0xffffce8d,0xffffec5d,0x39d0,0x1c5e,0x1b5f,
        0xffffce03,0xffffebcd,0x399e,0x1c2a,0x1bc3,
        0xffffcd7d,0xffffeb41,0x396a,0x1bf4,0x1c27,
        0xffffccfd,0xffffeabb,0x3933,0x1bbb,0x1c89,
        0xffffcc81,0xffffea3a,0x38f9,0x1b7f,0x1cec,
        0xffffcc0a,0xffffe9be,0x38bd,0x1b3f,0x1d4e,
        0xffffcb98,0xffffe947,0x387d,0x1afd,0x1daf,
        0xffffcb2a,0xffffe8d4,0x383b,0x1ab7,0x1e11,
        0xffffcac1,0xffffe866,0x37f4,0x1a6e,0x1e73,
        0xffffca5c,0xffffe7fd,0x37ab,0x1a21,0x1ed5,
        0xffffc9fc,0xffffe798,0x375e,0x19d1,0x1f38,
        0xffffc9a0,0xffffe738,0x370d,0x197d,0x1f9b,
        0xffffc947,0xffffe6dc,0x36b9,0x1924,0x2000,
        0xffffc8f3,0xffffe683,0x3660,0x18c8,0x2065,
        0xffffc8a2,0xffffe62f,0x3604,0x1868,0x20cc,
        0xffffc855,0xffffe5df,0x35a4,0x1803,0x2135,
        0xffffc80c,0xffffe592,0x353f,0x179a,0x21a0,
        0xffffc7c5,0xffffe549,0x34d6,0x172c,0x220e,
        0xffffc783,0xffffe503,0x3468,0x16b9,0x227e,
        0xffffc743,0xffffe4c1,0x33f6,0x1642,0x22f1,
        0xffffc707,0xffffe481,0x337f,0x15c6,0x2367,
        0xffffc6cd,0xffffe445,0x3303,0x1545,0x23e1,
        0xffffc696,0xffffe40c,0x3283,0x14bf,0x245f,
        0xffffc662,0xffffe3d6,0x31fd,0x1433,0x24e1,
        0xffffc630,0xffffe3a2,0x3173,0x13a3,0x2569,
//采样 2 频段 0
        0x3ef5,0xffffe108,0xffffc047,0x1fbd,0x1f9d,
        0x3f04,0xffffe0f9,0xffffc04a,0x1fb9,0x1fa6,
        0x3f11,0xffffe0eb,0xffffc04f,0x1fb4,0x1faf,
        0x3f1e,0xffffe0de,0xffffc053,0x1fb0,0x1fb8,
        0x3f2b,0xffffe0d2,0xffffc058,0x1fab,0x1fc0,
        0x3f36,0xffffe0c7,0xffffc05d,0x1fa6,0x1fc9,
        0x3f41,0xffffe0bc,0xffffc062,0x1fa1,0x1fd1,
        0x3f4c,0xffffe0b1,0xffffc068,0x1f9c,0x1fd9,
        0x3f55,0xffffe0a8,0xffffc06d,0x1f96,0x1fe1,
        0x3f5f,0xffffe09e,0xffffc074,0x1f8f,0x1fe8,
        0x3f67,0xffffe096,0xffffc07a,0x1f89,0x1ff0,
        0x3f70,0xffffe08d,0xffffc081,0x1f82,0x1ff8,
        0x3f77,0xffffe085,0xffffc089,0x1f7b,0x2000,
        0x3f7f,0xffffe07e,0xffffc090,0x1f73,0x2007,
        0x3f86,0xffffe077,0xffffc099,0x1f6a,0x200f,
        0x3f8c,0xffffe071,0xffffc0a1,0x1f62,0x2017,
        0x3f93,0xffffe06a,0xffffc0ab,0x1f58,0x201e,
        0x3f98,0xffffe064,0xffffc0b4,0x1f4f,0x2026,
        0x3f9e,0xffffe05f,0xffffc0bf,0x1f44,0x202e,
        0x3fa3,0xffffe05a,0xffffc0ca,0x1f39,0x2037,
        0x3fa8,0xffffe055,0xffffc0d5,0x1f2e,0x203f,
        0x3fad,0xffffe050,0xffffc0e2,0x1f22,0x2048,
        0x3fb1,0xffffe04c,0xffffc0ef,0x1f15,0x2051,
        0x3fb6,0xffffe047,0xffffc0fc,0x1f07,0x205a,
        0x3fb9,0xffffe043,0xffffc10b,0x1ef8,0x2063,
//采样 2 频段 1
        0x3cc1,0xffffe321,0xffffc0f0,0x1f2f,0x1ed4,
        0x3cec,0xffffe2f7,0xffffc0fc,0x1f23,0x1eef,
        0x3d14,0xffffe2ce,0xffffc109,0x1f16,0x1f0a,
        0x3d3a,0xffffe2a8,0xffffc117,0x1f08,0x1f24,
        0x3d5f,0xffffe283,0xffffc125,0x1efa,0x1f3e,
        0x3d81,0xffffe260,0xffffc135,0x1eea,0x1f57,
        0x3da2,0xffffe240,0xffffc145,0x1eda,0x1f70,
        0x3dc1,0xffffe221,0xffffc156,0x1ec9,0x1f88,
        0x3ddf,0xffffe203,0xffffc168,0x1eb7,0x1fa1,
        0x3dfa,0xffffe1e7,0xffffc17b,0x1ea4,0x1fb8,
        0x3e15,0xffffe1cd,0xffffc18f,0x1e90,0x1fd0,
        0x3e2e,0xffffe1b4,0xffffc1a4,0x1e7b,0x1fe8,
        0x3e46,0xffffe19c,0xffffc1ba,0x1e64,0x2000,
        0x3e5c,0xffffe185,0xffffc1d2,0x1e4c,0x2017,
        0x3e71,0xffffe170,0xffffc1eb,0x1e33,0x202f,
        0x3e85,0xffffe15c,0xffffc206,0x1e19,0x2047,
        0x3e98,0xffffe149,0xffffc221,0x1dfd,0x2060,
        0x3eaa,0xffffe137,0xffffc23f,0x1ddf,0x2078,
        0x3ebb,0xffffe126,0xffffc25e,0x1dc0,0x2091,
        0x3ecb,0xffffe116,0xffffc27f,0x1da0,0x20ab,
        0x3edb,0xffffe106,0xffffc2a1,0x1d7d,0x20c6,
        0x3ee9,0xffffe0f8,0xffffc2c6,0x1d58,0x20e1,
        0x3ef7,0xffffe0ea,0xffffc2ec,0x1d32,0x20fc,
        0x3f04,0xffffe0dd,0xffffc314,0x1d09,0x2119,
        0x3f10,0xffffe0d1,0xffffc33f,0x1cdf,0x2137,
//采样 2 频段 2
        0x3602,0xffffe8ef,0xffffc3b0,0x1d7e,0x1ca7,
        0x366f,0xffffe880,0xffffc3d4,0x1d5a,0x1cf2,
        0x36d7,0xffffe816,0xffffc3fa,0x1d33,0x1d3c,
        0x373b,0xffffe7b0,0xffffc421,0x1d0b,0x1d85,
        0x379b,0xffffe74e,0xffffc44b,0x1ce0,0x1dcd,
        0x37f6,0xffffe6f1,0xffffc477,0x1cb3,0x1e14,
        0x384e,0xffffe698,0xffffc4a6,0x1c84,0x1e5b,
        0x38a1,0xffffe643,0xffffc4d7,0x1c52,0x1ea1,
        0x38f1,0xffffe5f2,0xffffc50a,0x1c1e,0x1ee7,
        0x393d,0xffffe5a4,0xffffc540,0x1be7,0x1f2d,
        0x3986,0xffffe55a,0xffffc579,0x1bad,0x1f73,
        0x39cb,0xffffe513,0xffffc5b5,0x1b70,0x1fb9,
        0x3a0d,0xffffe4d0,0xffffc5f3,0x1b30,0x2000,
        0x3a4b,0xffffe490,0xffffc635,0x1aed,0x2047,
        0x3a87,0xffffe453,0xffffc67a,0x1aa6,0x208f,
        0x3ac0,0xffffe419,0xffffc6c3,0x1a5c,0x20d8,
        0x3af6,0xffffe3e2,0xffffc70f,0x1a0e,0x2122,
        0x3b29,0xffffe3ae,0xffffc75f,0x19bd,0x216e,
        0x3b5a,0xffffe37c,0xffffc7b2,0x1968,0x21bb,
        0x3b89,0xffffe34d,0xffffc80a,0x190f,0x220a,
        0x3bb5,0xffffe320,0xffffc865,0x18b2,0x225c,
        0x3bdf,0xffffe2f5,0xffffc8c5,0x1850,0x22af,
        0x3c06,0xffffe2cd,0xffffc929,0x17ea,0x2306,
        0x3c2c,0xffffe2a6,0xffffc991,0x1780,0x235f,
        0x3c50,0xffffe282,0xffffc9fe,0x1711,0x23bb,
//采样 2 频段 3
        0x232f,0xfffff4d1,0xffffd17d,0x1916,0x1834,
        0x23d6,0xfffff405,0xffffd1c9,0x18b9,0x18d0,
        0x2479,0xfffff33c,0xffffd218,0x1858,0x196c,
        0x2519,0xfffff278,0xffffd26b,0x17f2,0x1a0a,
        0x25b4,0xfffff1b9,0xffffd2c1,0x1788,0x1aaa,
        0x264c,0xfffff0fe,0xffffd31b,0x171a,0x1b4c,
        0x26e0,0xfffff049,0xffffd379,0x16a7,0x1bf0,
        0x2770,0xffffef99,0xffffd3db,0x162f,0x1c96,
        0x27fb,0xffffeeed,0xffffd441,0x15b2,0x1d3e,
        0x2883,0xffffee47,0xffffd4ab,0x1530,0x1dea,
        0x2906,0xffffeda7,0xffffd519,0x14a9,0x1e98,
        0x2984,0xffffed0b,0xffffd58b,0x141d,0x1f4a,
        0x29ff,0xffffec75,0xffffd601,0x138b,0x2000,
        0x2a75,0xffffebe3,0xffffd67c,0x12f5,0x20b9,
        0x2ae7,0xffffeb57,0xffffd6fa,0x1259,0x2177,
        0x2b55,0xffffead0,0xffffd77d,0x11b9,0x223a,
        0x2bbf,0xffffea4e,0xffffd805,0x1113,0x2303,
        0x2c25,0xffffe9d1,0xffffd890,0x1067,0x23d1,
        0x2c87,0xffffe959,0xffffd920,0xfb7,0x24a6,
        0x2ce5,0xffffe8e6,0xffffd9b4,0xf02,0x2582,
        0x2d3f,0xffffe878,0xffffda4c,0xe47,0x2666,
        0x2d95,0xffffe80e,0xffffdae7,0xd88,0x2751,
        0x2de8,0xffffe7a8,0xffffdb87,0xcc4,0x2846,
        0x2e37,0xffffe747,0xffffdc2a,0xbfb,0x2944,
        0x2e83,0xffffe6ea,0xffffdcd1,0xb2f,0x2a4d,
//采样 2 频段 4
        0xfffff226,0xfffffbce,0x1487,0x15a6,0x1597,
        0xfffff1ce,0xfffffae7,0x1455,0x1523,0x1657,
        0xfffff177,0xfffffa02,0x1422,0x149c,0x171b,
        0xfffff120,0xfffff920,0x13ec,0x140f,0x17e5,
        0xfffff0ca,0xfffff841,0x13b4,0x137e,0x18b3,
        0xfffff076,0xfffff764,0x137a,0x12e7,0x1987,
        0xfffff023,0xfffff68b,0x133f,0x124b,0x1a60,
        0xffffefd1,0xfffff5b6,0x1301,0x11a9,0x1b3f,
        0xffffef81,0xfffff4e5,0x12c1,0x1103,0x1c25,
        0xffffef33,0xfffff417,0x127f,0x1057,0x1d10,
        0xffffeee6,0xfffff34f,0x123c,0xfa6,0x1e03,
        0xffffee9b,0xfffff28a,0x11f6,0xef0,0x1efd,
        0xffffee51,0xfffff1ca,0x11af,0xe36,0x2000,
        0xffffee0a,0xfffff110,0x1165,0xd76,0x210a,
        0xffffedc4,0xfffff05a,0x111a,0xcb1,0x221d,
        0xffffed81,0xffffefa9,0x10cd,0xbe9,0x233a,
        0xffffed3f,0xffffeefd,0x107f,0xb1b,0x2462,
        0xffffecff,0xffffee57,0x102f,0xa4a,0x2594,
        0xffffecc1,0xffffedb5,0xfdd,0x975,0x26d2,
        0xffffec86,0xffffed19,0xf8a,0x89c,0x281c,
        0xffffec4c,0xffffec82,0xf36,0x7bf,0x2974,
        0xffffec14,0xffffebf1,0xee0,0x6e0,0x2ada,
        0xffffebde,0xffffeb64,0xe89,0x5fe,0x2c50,
        0xffffebab,0xffffeadd,0xe32,0x519,0x2dd6,
        0xffffeb79,0xffffea5a,0xdda,0x432,0x2f6d,
//采样 3 频段 0
        0x3f3e,0xffffe0c0,0xffffc033,0x1fcf,0x1fb8,
        0x3f49,0xffffe0b6,0xffffc036,0x1fcc,0x1fbe,
        0x3f53,0xffffe0ac,0xffffc039,0x1fc9,0x1fc5,
        0x3f5c,0xffffe0a2,0xffffc03c,0x1fc6,0x1fcb,
        0x3f65,0xffffe099,0xffffc03f,0x1fc2,0x1fd2,
        0x3f6e,0xffffe091,0xffffc043,0x1fbf,0x1fd8,
        0x3f76,0xffffe089,0xffffc047,0x1fbb,0x1fde,
        0x3f7d,0xffffe081,0xffffc04b,0x1fb7,0x1fe3,
        0x3f84,0xffffe07a,0xffffc04f,0x1fb3,0x1fe9,
        0x3f8b,0xffffe073,0xffffc054,0x1fae,0x1fef,
        0x3f91,0xffffe06d,0xffffc058,0x1fa9,0x1ff4,
        0x3f97,0xffffe067,0xffffc05d,0x1fa4,0x1ffa,
        0x3f9d,0xffffe061,0xffffc063,0x1f9f,0x2000,
        0x3fa3,0xffffe05c,0xffffc069,0x1f99,0x2005,
        0x3fa8,0xffffe057,0xffffc06f,0x1f93,0x200b,
        0x3fac,0xffffe052,0xffffc075,0x1f8d,0x2010,
        0x3fb1,0xffffe04d,0xffffc07c,0x1f86,0x2016,
        0x3fb5,0xffffe049,0xffffc083,0x1f7f,0x201c,
        0x3fb9,0xffffe045,0xffffc08a,0x1f77,0x2022,
        0x3fbd,0xffffe041,0xffffc092,0x1f6f,0x2028,
        0x3fc1,0xffffe03e,0xffffc09b,0x1f67,0x202e,
        0x3fc4,0xffffe03a,0xffffc0a4,0x1f5e,0x2034,
        0x3fc7,0xffffe037,0xffffc0ad,0x1f54,0x203a,
        0x3fca,0xffffe034,0xffffc0b7,0x1f4a,0x2041,
        0x3fcd,0xffffe031,0xffffc0c2,0x1f40,0x2048,
//采样 3 频段 1
        0x3da2,0xffffe24e,0xffffc0a9,0x1f67,0x1f23,
        0x3dc2,0xffffe22e,0xffffc0b2,0x1f5f,0x1f37,
        0x3de0,0xffffe210,0xffffc0bb,0x1f55,0x1f4b,
        0x3dfd,0xffffe1f3,0xffffc0c5,0x1f4b,0x1f5f,
        0x3e18,0xffffe1d8,0xffffc0d0,0x1f41,0x1f72,
        0x3e32,0xffffe1be,0xffffc0db,0x1f35,0x1f84,
        0x3e4a,0xffffe1a6,0xffffc0e7,0x1f2a,0x1f96,
        0x3e61,0xffffe18f,0xffffc0f3,0x1f1d,0x1fa8,
        0x3e77,0xffffe179,0xffffc100,0x1f10,0x1fba,
        0x3e8b,0xffffe165,0xffffc10e,0x1f02,0x1fcb,
        0x3e9f,0xffffe151,0xffffc11d,0x1ef3,0x1fdd,
        0x3eb1,0xffffe13f,0xffffc12d,0x1ee3,0x1fee,
        0x3ec3,0xffffe12d,0xffffc13d,0x1ed3,0x2000,
        0x3ed3,0xffffe11d,0xffffc14f,0x1ec1,0x2011,
        0x3ee3,0xffffe10d,0xffffc161,0x1eaf,0x2022,
        0x3ef2,0xffffe0fe,0xffffc175,0x1e9b,0x2034,
        0x3f00,0xffffe0f0,0xffffc189,0x1e87,0x2046,
        0x3f0d,0xffffe0e3,0xffffc19f,0x1e71,0x2058,
        0x3f19,0xffffe0d6,0xffffc1b6,0x1e5a,0x206a,
        0x3f25,0xffffe0cb,0xffffc1ce,0x1e42,0x207d,
        0x3f30,0xffffe0bf,0xffffc1e8,0x1e28,0x2090,
        0x3f3b,0xffffe0b5,0xffffc203,0x1e0d,0x20a4,
        0x3f45,0xffffe0ab,0xffffc220,0x1df0,0x20b8,
        0x3f4e,0xffffe0a1,0xffffc23e,0x1dd2,0x20cd,
        0x3f57,0xffffe099,0xffffc25e,0x1db2,0x20e2,
//采样 3 频段 2
        0x38a9,0xffffe6c2,0xffffc27a,0x1e28,0x1d78,
        0x38ff,0xffffe66b,0xffffc294,0x1e0d,0x1db2,
        0x3952,0xffffe618,0xffffc2b1,0x1df0,0x1dea,
        0x39a0,0xffffe5c9,0xffffc2cf,0x1dd2,0x1e22,
        0x39eb,0xffffe57d,0xffffc2ee,0x1db2,0x1e59,
        0x3a33,0xffffe535,0xffffc30f,0x1d90,0x1e8f,
        0x3a77,0xffffe4f0,0xffffc332,0x1d6d,0x1ec4,
        0x3ab8,0xffffe4af,0xffffc357,0x1d47,0x1ef9,
        0x3af5,0xffffe470,0xffffc37e,0x1d20,0x1f2e,
        0x3b30,0xffffe435,0xffffc3a8,0x1cf7,0x1f62,
        0x3b68,0xffffe3fc,0xffffc3d3,0x1ccb,0x1f97,
        0x3b9d,0xffffe3c7,0xffffc400,0x1c9d,0x1fcb,
        0x3bd0,0xffffe394,0xffffc430,0x1c6c,0x2000,
        0x3c00,0xffffe363,0xffffc463,0x1c39,0x2034,
        0x3c2d,0xffffe335,0xffffc498,0x1c04,0x206a,
        0x3c58,0xffffe309,0xffffc4d0,0x1bcb,0x20a0,
        0x3c82,0xffffe2e0,0xffffc50b,0x1b90,0x20d7,
        0x3ca9,0xffffe2b9,0xffffc548,0x1b51,0x210e,
        0x3cce,0xffffe293,0xffffc589,0x1b10,0x2147,
        0x3cf1,0xffffe270,0xffffc5cd,0x1acb,0x2181,
        0x3d12,0xffffe24e,0xffffc615,0x1a83,0x21bd,
        0x3d31,0xffffe22e,0xffffc660,0x1a37,0x21fb,
        0x3d4f,0xffffe210,0xffffc6ae,0x19e8,0x223a,
        0x3d6c,0xffffe1f3,0xffffc701,0x1995,0x227b,
        0x3d86,0xffffe1d8,0xffffc757,0x193e,0x22bf,
//采样 3 频段 3
        0x2a60,0xfffff0f8,0xffffcb21,0x1aae,0x19a5,
        0x2b03,0xfffff043,0xffffcb64,0x1a65,0x1a29,
        0x2ba2,0xffffef92,0xffffcba9,0x1a18,0x1aad,
        0x2c3c,0xffffeee7,0xffffcbf2,0x19c7,0x1b31,
        0x2cd1,0xffffee41,0xffffcc3e,0x1972,0x1bb5,
        0x2d62,0xffffeda1,0xffffcc8e,0x1919,0x1c3a,
        0x2dee,0xffffed05,0xffffcce2,0x18bd,0x1cc0,
        0x2e76,0xffffec6f,0xffffcd39,0x185c,0x1d47,
        0x2ef8,0xffffebde,0xffffcd95,0x17f6,0x1dcf,
        0x2f76,0xffffeb52,0xffffcdf4,0x178c,0x1e58,
        0x2ff0,0xffffeacb,0xffffce57,0x171e,0x1ee3,
        0x3065,0xffffea4a,0xffffcebf,0x16ab,0x1f70,
        0x30d5,0xffffe9cd,0xffffcf2b,0x1633,0x2000,
        0x3141,0xffffe955,0xffffcf9b,0x15b6,0x2091,
        0x31a9,0xffffe8e2,0xffffd010,0x1535,0x2126,
        0x320c,0xffffe874,0xffffd08a,0x14ae,0x21be,
        0x326b,0xffffe80a,0xffffd108,0x1422,0x2259,
        0x32c7,0xffffe7a4,0xffffd18a,0x1391,0x22f9,
        0x331e,0xffffe743,0xffffd212,0x12fb,0x239d,
        0x3372,0xffffe6e7,0xffffd29e,0x125f,0x2445,
        0x33c2,0xffffe68e,0xffffd32f,0x11bf,0x24f4,
        0x340e,0xffffe639,0xffffd3c4,0x1119,0x25a8,
        0x3457,0xffffe5e8,0xffffd45e,0x106e,0x2662,
        0x349c,0xffffe59b,0xffffd4fd,0xfbd,0x2723,
        0x34df,0xffffe552,0xffffd5a0,0xf08,0x27ec,
//采样 3 频段 4
        0x51c,0xfffffce3,0xfffff849,0x150a,0x152f,
        0x53e,0xfffffbfa,0xfffff85c,0x1482,0x15f4,
        0x55f,0xfffffb13,0xfffff871,0x13f4,0x16be,
        0x581,0xfffffa2e,0xfffff886,0x1362,0x178e,
        0x5a2,0xfffff94b,0xfffff89c,0x12ca,0x1863,
        0x5c2,0xfffff86b,0xfffff8b3,0x122d,0x193e,
        0x5e2,0xfffff78e,0xfffff8cb,0x118a,0x1a20,
        0x602,0xfffff6b4,0xfffff8e3,0x10e3,0x1b08,
        0x621,0xfffff5de,0xfffff8fc,0x1036,0x1bf7,
        0x640,0xfffff50c,0xfffff916,0xf84,0x1ced,
        0x65e,0xfffff43e,0xfffff931,0xecd,0x1deb,
        0x67b,0xfffff374,0xfffff94c,0xe12,0x1ef1,
        0x698,0xfffff2af,0xfffff968,0xd51,0x2000,
        0x6b4,0xfffff1ee,0xfffff985,0xc8c,0x2118,
        0x6cf,0xfffff133,0xfffff9a2,0xbc2,0x2239,
        0x6ea,0xfffff07c,0xfffff9c0,0xaf4,0x2366,
        0x704,0xffffefca,0xfffff9df,0xa22,0x249d,
        0x71d,0xffffef1d,0xfffff9fe,0x94c,0x25e1,
        0x735,0xffffee76,0xfffffa1e,0x872,0x2731,
        0x74d,0xffffedd3,0xfffffa3e,0x795,0x288f,
        0x764,0xffffed36,0xfffffa5e,0x6b5,0x29fc,
        0x77a,0xffffec9e,0xfffffa7f,0x5d2,0x2b78,
        0x78f,0xffffec0c,0xfffffaa1,0x4ed,0x2d05,
        0x7a4,0xffffeb7e,0xfffffac2,0x406,0x2ea4,
        0x7b7,0xffffeaf6,0xfffffae4,0x31d,0x3055,
//采样 4 频段 0
        0x3f4e,0xffffe0b1,0xffffc02f,0x1fd3,0x1fbd,
        0x3f57,0xffffe0a7,0xffffc031,0x1fd0,0x1fc4,
        0x3f61,0xffffe09e,0xffffc034,0x1fcd,0x1fca,
        0x3f69,0xffffe095,0xffffc037,0x1fca,0x1fd0,
        0x3f72,0xffffe08d,0xffffc03a,0x1fc7,0x1fd5,
        0x3f79,0xffffe085,0xffffc03d,0x1fc4,0x1fdb,
        0x3f81,0xffffe07e,0xffffc041,0x1fc0,0x1fe0,
        0x3f88,0xffffe077,0xffffc045,0x1fbd,0x1fe6,
        0x3f8e,0xffffe070,0xffffc049,0x1fb9,0x1feb,
        0x3f95,0xffffe06a,0xffffc04d,0x1fb5,0x1ff0,
        0x3f9a,0xffffe064,0xffffc051,0x1fb0,0x1ff5,
        0x3fa0,0xffffe05f,0xffffc056,0x1fac,0x1ffa,
        0x3fa5,0xffffe059,0xffffc05b,0x1fa7,0x2000,
        0x3faa,0xffffe054,0xffffc060,0x1fa1,0x2005,
        0x3faf,0xffffe050,0xffffc066,0x1f9c,0x200a,
        0x3fb3,0xffffe04b,0xffffc06b,0x1f96,0x200f,
        0x3fb7,0xffffe047,0xffffc072,0x1f90,0x2014,
        0x3fbb,0xffffe043,0xffffc078,0x1f89,0x2019,
        0x3fbf,0xffffe040,0xffffc07f,0x1f82,0x201f,
        0x3fc3,0xffffe03c,0xffffc087,0x1f7b,0x2024,
        0x3fc6,0xffffe039,0xffffc08e,0x1f73,0x202a,
        0x3fc9,0xffffe036,0xffffc097,0x1f6b,0x2030,
        0x3fcc,0xffffe033,0xffffc09f,0x1f62,0x2036,
        0x3fcf,0xffffe030,0xffffc0a9,0x1f59,0x203c,
        0x3fd1,0xffffe02d,0xffffc0b2,0x1f4f,0x2042,
//采样 4 频段 1
        0x3dd3,0xffffe21f,0xffffc09a,0x1f74,0x1f34,
        0x3df1,0xffffe202,0xffffc0a2,0x1f6b,0x1f47,
        0x3e0c,0xffffe1e6,0xffffc0ab,0x1f63,0x1f59,
        0x3e27,0xffffe1cc,0xffffc0b4,0x1f5a,0x1f6b,
        0x3e40,0xffffe1b3,0xffffc0be,0x1f50,0x1f7d,
        0x3e57,0xffffe19b,0xffffc0c8,0x1f46,0x1f8e,
        0x3e6e,0xffffe185,0xffffc0d3,0x1f3b,0x1f9f,
        0x3e83,0xffffe16f,0xffffc0df,0x1f2f,0x1faf,
        0x3e97,0xffffe15b,0xffffc0eb,0x1f23,0x1fc0,
        0x3eaa,0xffffe148,0xffffc0f8,0x1f16,0x1fd0,
        0x3ebc,0xffffe136,0xffffc105,0x1f08,0x1fe0,
        0x3ecd,0xffffe125,0xffffc114,0x1efa,0x1ff0,
        0x3edd,0xffffe115,0xffffc123,0x1eeb,0x2000,
        0x3eec,0xffffe106,0xffffc133,0x1edb,0x200f,
        0x3efb,0xffffe0f8,0xffffc144,0x1eca,0x201f,
        0x3f08,0xffffe0ea,0xffffc156,0x1eb8,0x2030,
        0x3f15,0xffffe0dd,0xffffc169,0x1ea5,0x2040,
        0x3f21,0xffffe0d1,0xffffc17d,0x1e91,0x2051,
        0x3f2d,0xffffe0c5,0xffffc192,0x1e7b,0x2061,
        0x3f38,0xffffe0ba,0xffffc1a9,0x1e65,0x2073,
        0x3f42,0xffffe0b0,0xffffc1c0,0x1e4d,0x2084,
        0x3f4c,0xffffe0a6,0xffffc1d9,0x1e34,0x2096,
        0x3f55,0xffffe09d,0xffffc1f4,0x1e1a,0x20a9,
        0x3f5e,0xffffe095,0xffffc20f,0x1dfe,0x20bc,
        0x3f66,0xffffe08c,0xffffc22d,0x1de1,0x20d0,
//采样 4 频段 2
        0x393d,0xffffe644,0xffffc23c,0x1e4d,0x1da7,
        0x398e,0xffffe5f3,0xffffc255,0x1e34,0x1ddd,
        0x39db,0xffffe5a5,0xffffc26f,0x1e19,0x1e12,
        0x3a24,0xffffe55b,0xffffc28a,0x1dfd,0x1e45,
        0x3a6a,0xffffe515,0xffffc2a8,0x1de0,0x1e78,
        0x3aad,0xffffe4d1,0xffffc2c6,0x1dc1,0x1eaa,
        0x3aec,0xffffe491,0xffffc2e7,0x1da0,0x1edc,
        0x3b29,0xffffe454,0xffffc309,0x1d7d,0x1f0d,
        0x3b62,0xffffe41a,0xffffc32d,0x1d59,0x1f3e,
        0x3b99,0xffffe3e3,0xffffc353,0x1d33,0x1f6e,
        0x3bcd,0xffffe3af,0xffffc37c,0x1d0a,0x1f9f,
        0x3bff,0xffffe37d,0xffffc3a6,0x1cdf,0x1fcf,
        0x3c2d,0xffffe34e,0xffffc3d3,0x1cb2,0x2000,
        0x3c5a,0xffffe321,0xffffc401,0x1c83,0x2030,
        0x3c84,0xffffe2f6,0xffffc433,0x1c51,0x2062,
        0x3cad,0xffffe2cd,0xffffc467,0x1c1d,0x2093,
        0x3cd3,0xffffe2a7,0xffffc49e,0x1be6,0x20c6,
        0x3cf7,0xffffe283,0xffffc4d7,0x1bac,0x20f9,
        0x3d19,0xffffe260,0xffffc514,0x1b6f,0x212e,
        0x3d3a,0xffffe23f,0xffffc553,0x1b2f,0x2163,
        0x3d58,0xffffe220,0xffffc596,0x1aeb,0x219a,
        0x3d76,0xffffe203,0xffffc5dc,0x1aa5,0x21d3,
        0x3d91,0xffffe1e7,0xffffc625,0x1a5b,0x220d,
        0x3dab,0xffffe1cc,0xffffc672,0x1a0d,0x2249,
        0x3dc4,0xffffe1b3,0xffffc6c3,0x19bc,0x2287,
//采样 4 频段 3
        0x2bfb,0xffffefff,0xffffc9e5,0x1b0e,0x1a03,
        0x2c9b,0xffffef51,0xffffca24,0x1ac9,0x1a80,
        0x2d36,0xffffeea8,0xffffca66,0x1a81,0x1afd,
        0x2dcc,0xffffee04,0xffffcaac,0x1a35,0x1b7a,
        0x2e5d,0xffffed65,0xffffcaf4,0x19e6,0x1bf8,
        0x2eea,0xffffeccc,0xffffcb41,0x1993,0x1c75,
        0x2f72,0xffffec37,0xffffcb91,0x193b,0x1cf4,
        0x2ff5,0xffffeba8,0xffffcbe4,0x18e0,0x1d73,
        0x3073,0xffffeb1e,0xffffcc3b,0x1881,0x1df2,
        0x30ed,0xffffea9a,0xffffcc97,0x181d,0x1e73,
        0x3162,0xffffea1a,0xffffccf6,0x17b5,0x1ef6,
        0x31d2,0xffffe99f,0xffffcd5a,0x1748,0x1f7a,
        0x323e,0xffffe929,0xffffcdc2,0x16d7,0x2000,
        0x32a6,0xffffe8b8,0xffffce2e,0x1661,0x2088,
        0x330a,0xffffe84b,0xffffce9e,0x15e6,0x2112,
        0x3369,0xffffe7e3,0xffffcf13,0x1566,0x21a0,
        0x33c5,0xffffe77f,0xffffcf8d,0x14e2,0x2231,
        0x341c,0xffffe720,0xffffd00b,0x1458,0x22c5,
        0x346f,0xffffe6c5,0xffffd08e,0x13c9,0x235d,
        0x34bf,0xffffe66d,0xffffd116,0x1334,0x23fa,
        0x350c,0xffffe61a,0xffffd1a3,0x129b,0x249c,
        0x3554,0xffffe5cb,0xffffd234,0x11fc,0x2543,
        0x359a,0xffffe57f,0xffffd2ca,0x1158,0x25f0,
        0x35dc,0xffffe537,0xffffd365,0x10af,0x26a3,
        0x361b,0xffffe4f2,0xffffd405,0x1001,0x275d,
//采样 4 频段 4
        0x92f,0xfffffc82,0xfffff238,0x1541,0x1553,
        0x96b,0xfffffb99,0xfffff25b,0x14bb,0x1617,
        0x9a7,0xfffffab3,0xfffff27f,0x1430,0x16df,
        0x9e2,0xfffff9cf,0xfffff2a4,0x139f,0x17ac,
        0xa1d,0xfffff8ed,0xfffff2cb,0x130a,0x187f,
        0xa56,0xfffff80e,0xfffff2f3,0x126f,0x1958,
        0xa8f,0xfffff733,0xfffff31c,0x11ce,0x1a36,
        0xac7,0xfffff65b,0xfffff347,0x1129,0x1b1b,
        0xafe,0xfffff586,0xfffff373,0x107f,0x1c07,
        0xb34,0xfffff4b6,0xfffff3a1,0xfcf,0x1cf9,
        0xb69,0xfffff3e9,0xfffff3d0,0xf1a,0x1df3,
        0xb9d,0xfffff322,0xfffff400,0xe60,0x1ef5,
        0xbcf,0xfffff25e,0xfffff431,0xda2,0x2000,
        0xc00,0xfffff1a0,0xfffff463,0xcde,0x2113,
        0xc30,0xfffff0e6,0xfffff497,0xc17,0x222f,
        0xc5f,0xfffff031,0xfffff4cc,0xb4a,0x2356,
        0xc8d,0xffffef81,0xfffff502,0xa7a,0x2488,
        0xcb9,0xffffeed7,0xfffff539,0x9a5,0x25c6,
        0xce4,0xffffee32,0xfffff571,0x8cd,0x270f,
        0xd0d,0xffffed91,0xfffff5aa,0x7f2,0x2866,
        0xd35,0xffffecf6,0xfffff5e3,0x713,0x29cc,
        0xd5c,0xffffec61,0xfffff61e,0x631,0x2b40,
        0xd81,0xffffebd0,0xfffff659,0x54d,0x2cc5,
        0xda5,0xffffeb45,0xfffff695,0x467,0x2e5a,
        0xdc8,0xffffeabf,0xfffff6d1,0x37e,0x3003,

        0x00,
    },


    /*
     * Function
     */

//_ATTR_RKEQ_DATA_
//int bass_60_tbl[]=    //   一个采样率下60 Q1.2  12 : -2 : 2 的增益  共5个采样率  0
    {
//采样 0
        0x1f9a01,0xfff06144,0x106928,0xffe065ff,0xf3594,
        0x1f8e4d,0xfff06cfa,0x105261,0xffe071b3,0xf40a5,
        0x1f87f0,0xfff07357,0x104770,0xffe07810,0xf4538,
        0x1f7a18,0xfff08132,0x103246,0xffe085e8,0xf4c88,
        0x1f6a9e,0xfff090ae,0x101dd7,0xffe09562,0xf517b,
        0x1f6235,0xfff09917,0x1013d2,0xffe09dcb,0xf5317,
//采样 1

        0x1fcde1,0xfff030ef,0x1034e6,0xffe0321f,0xf9a2b,
        0x1fc7f3,0xfff036dc,0x102978,0xffe0380d,0xf9fab,
        0x1fc4b9,0xfff03a16,0x1023f9,0xffe03b47,0xfa1ef,
        0x1fbdb1,0xfff0411f,0x101957,0xffe0424f,0xfa58a,
        0x1fb5d2,0xfff048ff,0x100f0e,0xffe04a2e,0xfa7f2,
        0x1fb189,0xfff04d48,0x100a01,0xffe04e77,0xfa8b7,
//采样 2

        0x1fdda7,0xfff021c8,0x102485,0xffe02259,0xfb9b3,
        0x1fd98d,0xfff025e2,0x101ca2,0xffe02673,0xfbd7b,
        0x1fd751,0xfff0281e,0x1018d8,0xffe028af,0xfbf09,
        0x1fd273,0xfff02cfc,0x101181,0xffe02d8d,0xfc182,
        0x1fccff,0xfff03271,0x100a67,0xffe03301,0xfc328,
        0x1fca06,0xfff03569,0x1006ea,0xffe035fa,0xfc3ad,
//采样 3

        0x1fe729,0xfff0188a,0x101a87,0xffe018d7,0xfccee,
        0x1fe42e,0xfff01b86,0x1014ce,0xffe01bd2,0xfcfab,
        0x1fe28d,0xfff01d26,0x10120e,0xffe01d73,0xfd0cc,
        0x1fdf02,0xfff020b1,0x100cb8,0xffe020fe,0xfd296,
        0x1fdb0a,0xfff024aa,0x10078f,0xffe024f6,0xfd3c6,
        0x1fd8e1,0xfff026d3,0x100506,0xffe0271f,0xfd427,
//采样 4

        0x1fe932,0xfff0168e,0x101861,0xffe016ce,0xfd111,
        0x1fe674,0xfff0194b,0x10131e,0xffe0198c,0xfd396,
        0x1fe4f6,0xfff01aca,0x101097,0xffe01b0a,0xfd49e,
        0x1fe1b4,0xfff01e0c,0x100bb0,0xffe01e4c,0xfd644,
        0x1fde0d,0xfff021b2,0x1006f3,0xffe021f3,0xfd75b,
        0x1fdc10,0xfff023b0,0x10049e,0xffe023f0,0xfd7b3,

    },

//_ATTR_RKEQ_DATA_
//int bass_120_tbl[]= //120hz 的五个采样率  120hz 2.5 -9.5
    {	//采样 0
        //采样 0 频段 0
        0x1f41fe,0xfff0ab42,0x1042a4,0xffe0be02,0xf1219,
        0x1f3818,0xfff0b52e,0x1034fc,0xffe0c7e8,0xf15d5,
        0x1f2296,0xfff0cabe,0x101a3f,0xffe0dd6a,0xf1b03,
        0x1f2296,0xfff0cabe,0x101a3f,0xffe0dd6a,0xf1b03,
        0x1f2296,0xfff0cabe,0x101a3f,0xffe0dd6a,0xf1b03,
        0x1f0a96,0xfff0e2cb,0x100000,0xffe0f56a,0xf1d35,
        //采样 1

        0x1fa4ae,0xfff05695,0x1021b1,0xffe05b52,0xf87b9,
        0x1f9f9b,0xfff05ba8,0x101acd,0xffe06065,0xf898a,
        0x1f9492,0xfff066b3,0x100d4b,0xffe06b6e,0xf8c01,
        0x1f9492,0xfff066b3,0x100d4b,0xffe06b6e,0xf8c01,
        0x1f9492,0xfff066b3,0x100d4b,0xffe06b6e,0xf8c01,
        0x1f8839,0xfff0730e,0x100000,0xffe077c7,0xf8cf2,
        //采样 2

        0x1fc1e1,0xfff03bdd,0x10174b,0xffe03e1f,0xfacd6,
        0x1fbe5c,0xfff03f62,0x101288,0xffe041a4,0xfae14,
        0x1fb6b1,0xfff0470d,0x100933,0xffe0494f,0xfafc0,
        0x1fb6b1,0xfff0470d,0x100933,0xffe0494f,0xfafc0,
        0x1fb6b1,0xfff0470d,0x100933,0xffe0494f,0xfafc0,
        0x1fae1c,0xfff04fa3,0x100000,0xffe051e4,0xfb05d,
        //采样 3

        0x1fd347,0xfff02b88,0x1010ef,0xffe02cb9,0xfc388,
        0x1fd0b7,0xfff02e18,0x100d7a,0xffe02f49,0xfc46d,
        0x1fcb21,0xfff033ae,0x1006b0,0xffe034df,0xfc5a0,
        0x1fcb21,0xfff033ae,0x1006b0,0xffe034df,0xfc5a0,
        0x1fcb21,0xfff033ae,0x1006b0,0xffe034df,0xfc5a0,
        0x1fc4de,0xfff039f2,0x100000,0xffe03b22,0xfc60e,
        //采样 4

        0x1fd6fc,0xfff02803,0x100f91,0xffe02904,0xfc86b,
        0x1fd4a0,0xfff02a5f,0x100c64,0xffe02b60,0xfc93e,
        0x1fcf7c,0xfff02f82,0x100627,0xffe03084,0xfca57,
        0x1fcf7c,0xfff02f82,0x100627,0xffe03084,0xfca57,
        0x1fcf7c,0xfff02f82,0x100627,0xffe03084,0xfca57,
        0x1fc9bb,0xfff03544,0x100000,0xffe03645,0xfcabc,
    },
};


// IIR Filter Coefficient computation function
_ATTR_RKEQ_TEXT_
void ROCKEQ_Get_Coeff(short *pGain, long Fs)
{

    long r12;
    //int r14;
    short Gain[5];
    int max_gain = 0;
    eqCoef_t eqC[5][25]; /* [band][db] */
    bassCoef_t BASS_eqC[2];
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;
   // printf("\n 5 段EQ  %d %d\n",g_FilterState.i_32buff[0],g_FilterState.i_32buff[5]);
    /* dummy code , for keep eqArray . by Vincent @ Apr 14 , 2009 */
    //memcpy((char *)Gain, (char *)eqArray, 2*5);

    memcpy((char *)Gain, (char *)pGain, 2*5);  //short 型
    switch (pEffect->Mode)
    {
        case EQ_NOR:
        case EQ_ROCK:
        case EQ_CLASS:
            // max_gain = 0;
            // EQ_Last_maxGain = 0;
            break;
        case EQ_HEAVY:
        case EQ_POP:
           // printf("1 2\n");
            // max_gain = 3;//数值无实际意义，只是区分mode 类型
             break;
        case EQ_USER:
           //  printf("5\n");
           //  max_gain = 9; //数值无实际意义，只是区分mode 类型
             break;
        case EQ_BASS:
           //  max_gain = 0;
            break;
        case EQ_JAZZ:
        case EQ_UNIQUE:
           //  printf("3 4\n");
           //  max_gain = 5;//数值无实际意义，只是区分mode 类型
             Gain[1] -=1;
            break;
    }


    switch (Fs)
    {
//------------0---------------
        case 0: //8khz
        case 1://11khz
        case 2://12khz
            r12=0;
            Gain[4]=0x0c;
            break;
//------------1---------------
        case 3://16khz
        case 4://22khz
        case 5://24khz
            r12=1;
            Gain[4]=0x0c;
            break;
//------------2---------------
        case 6://32khz
            r12=2;
            Gain[4]=Gain[4]&0xff;
            break;
//------------3---------------
        case 7://44khz
            r12=3;
            Gain[4]=Gain[4]&0xff;
            break;
//------------4---------------
        case 8://48khz
            r12=4;
            Gain[4]=Gain[4]&0xff;
            break;
//----------------------------
        default:
            r12=3;
            Gain[4]=Gain[4]&0xff;
            break;

    }

#ifndef _JTAG_DEBUG_
    /*Todo*/
    //ReadModuleData(MODULE_ID_AUDIO_EQ,&eqC[0][0],r12 * sizeof(eqC),sizeof(eqC));
    ReadEqData(&eqC[0][0],r12 * sizeof(eqC),sizeof(eqC));//sizeof(eqC) = 2*5*5*25
    // printf("\neqC[0][0]= %x ",eqC[0][0]);
#else
    {
        char *p;
        p = (char *)&eqArray[0];
        p = p + r12 * sizeof(eqC);
        memcpy( &eqC[0][0] , p , sizeof(eqC) );//sizeof(eqC) = 2*5*5*25
    }
#endif

    /*********add by any.woo for bass*******/
    {
        //pEffect->max_DbGain
        if (pEffect->max_DbGain != 0)
        {
            ReadEqData(&BASS_eqC[0],6252+(r12 *6 +((12 - pEffect->max_DbGain)>>1))* sizeof(bassCoef_t) ,sizeof(bassCoef_t));//sizeof(eqC) = 2*5*5*25
            ReadEqData(&BASS_eqC[1],6852+(r12 *6 +((12 - pEffect->max_DbGain)>>1))* sizeof(bassCoef_t),sizeof(bassCoef_t));//sizeof(eqC) = 2*5*5*25

        }
    }
    /*********add by any.woo for bass*******/

    {		//loc_16c:
        int  i,temp,seg=0;



        for (i=0;i<5;i++)
        {
            seg=Gain[i];//<<1;
            //todo
            g_FilterState._as[i*2]=  eqC[i][seg].a1; //0x3fc4;//ROCKa[r12][i][seg];
            g_FilterState._as[i*2+1]= eqC[i][seg].a2; //0xe03b;//ROCKa[r12][i][seg+1];
        }


        for (i=0;i<5;i++)
        {
            seg=Gain[i];//<<1;
            //todo
            g_FilterState._bs[i*2]= eqC[i][seg].b1; //0xc03b;// ROCKb[r12][i][seg];
            g_FilterState._bs[i*2+1]= eqC[i][seg].b2; //0x1fc6; //ROCKb[r12][i][seg+1];
        }


        seg=Gain[0];
        //todo
        temp=0x2156;//ROCKB[r12][0][seg];  //8534

        for (i=1;i<5;i++)
        {
            seg=Gain[i];
            //todo
            //temp=(long)temp*eqC[i][seg].B; //0x2156/*ROCKB[r12][i][seg]*/>>13;
            temp=(((long)temp*eqC[i][seg].B)>>13); //0x2156/*ROCKB[r12][i][seg]*/;

        }
        g_FilterState.factored_BS = temp;

    }

    /*********add by any.woo for bass*******/
    if (pEffect->max_DbGain != 0)
    {
        //todo
        int i;

        for (i=0;i<2;i++)
        {
            //todo
            g_bass_FilterState._as[i*2]=  BASS_eqC[i].a1; //0x3fc4;//ROCKa[r12][i][seg];
            g_bass_FilterState._as[i*2+1]= BASS_eqC[i].a2; //0xe03b;//ROCKa[r12][i][seg+1];
            g_bass_FilterState._bs[i*3]= BASS_eqC[i].b1; //0xc03b;// ROCKb[r12][i][seg];
            g_bass_FilterState._bs[i*3+1]= BASS_eqC[i].b2; //0x1fc6; //ROCKb[r12][i][seg+1];
            g_bass_FilterState._bs[i*3+2]= BASS_eqC[i].B;
        }
    }
    /*********add by any.woo for bass*******/



}

/*------------------------------------------------------------------------------
 Function:   EQ_ClearBuff
 Synopsis:
 Arguments:
 Returns:
------------------------------------------------------------------------------*/
_ATTR_RKEQ_TEXT_
void EQ_ClearBuff(void)
{
    int i;
    for (i = 0; i < 20; i++)
    {
        g_FilterState.i_32buff[i] = 0;
    }
    /*********add by any.woo for bass*******/

    for (i = 0;i<9;i++)
    {
        g_bass_FilterState.x[i] = 0;
        g_bass_FilterState.y[i] = 0;
    }
    g_bass_FilterState.max_DbGain[0] = 0;
    g_bass_FilterState.max_DbGain[1] = 0;

    g_FilterState.mode[0] = EQ_NOR;
    g_FilterState.mode[1] = EQ_NOR;

    /*********add by any.woo for bass*******/
}

/*------------------------------------------------------------------------------
 Function:   RockEQReduce9dB
 Synopsis:
 Arguments:
 Returns:
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 Function:   SubFiltering
 Synopsis:
 Arguments:
 Returns:
------------------------------------------------------------------------------*/
_ATTR_RKEQ_TEXT_
void SubFiltering(short *pwBuffer, long cwBuffer, long LR, short ps_factor, short index, long mode)
{
    short *pwbuffer;//ps_factor=0;  index = 2;
    long  cwbuffer;
    long lAccumulate;
    //long lbuff_1, lbuff_2;
    long y, y1, x, x1;
    //short a0, a1, b0, b1;

    //a0 = g_FilterState._a[index];
    //a1 = g_FilterState._a[index+1];
    //b0 = g_FilterState._b[index];
    //b1 = g_FilterState._b[index+1];

    pwbuffer = pwBuffer;
    cwbuffer = cwBuffer;

    //if(LR == 0)//left channel data
    {
        //g_FilterState.temp_mem = cwBuffer;
        //lbuff_1 = g_FilterState.i_32buff[index+LR*10];
        //lbuff_2 = g_FilterState.i_32buff[index+1+LR*10];

        //SubFilter_Core(cwbuffer,pwbuffer, &lbuff_1 , &lbuff_2 , a0 , a1 , b0 , b1);
        //SubFilter_Core(cwbuffer,pwbuffer, &lbuff_1 , &lbuff_2);

        SubFilter_Core(cwbuffer,pwbuffer, &g_FilterState , index , LR );

#if 0//1
        while (cwbuffer--)
        {
            /*
             * The difference equation is: y = x + a0*y' + a1*y" + b0*x' + b1*x"
             */
            x = *pwbuffer;//*pwbuffer>>ps_factor;//r5
            x1 = x;//r5
            y = lbuff_1 + ((unsigned long)x<<13);
            y >>= 13;
            y1 = y;
            if ( (y>>15) != (y>>31) )
            {
                y  >>= 31;
                y ^= 0x7fff;
            }
            *pwbuffer = y; // write the PCM data
            pwbuffer += mode;

            lbuff_1 = x1*b0 + y1*a0 + lbuff_2;

            // x2 = x1, y2 = y1
            lbuff_2 = x1*b1 + y1*a1;
        }
#endif

        //g_FilterState.i_32buff[index+LR*10] = lbuff_1;
        //g_FilterState.i_32buff[index+1+LR*10] = lbuff_2;
    }
}

/*------------------------------------------------------------------------------
 Function:   SubFiltering1
 Synopsis:
 Arguments:
 Returns:
------------------------------------------------------------------------------*/
_ATTR_RKEQ_TEXT_
void SubFiltering1(short *pwBuffer, long cwBuffer, long LR, long mode)
{
    short *pwbuffer;
    long  cwbuffer;
    long lAccumulate;
    long lbuff_1, lbuff_2;
    long y, y1, x, x1;
    short factored_B;

    pwbuffer = pwBuffer;
    cwbuffer = cwBuffer;

    if(LR == 0)
    {
        factored_B = g_FilterState.factored_BL; // may cause overflow!!!!
    }
    else
    {
        factored_B = g_FilterState.factored_BS; // may cause overflow!!!!
    }

    //if(LR == 0)//left channel data
    {
        //g_FilterState.temp_mem = cwBuffer;
        lbuff_1 = g_FilterState.i_32buff[0+LR*10];
        lbuff_2 = g_FilterState.i_32buff[0+1+LR*10];

        while (cwbuffer--)
        {
            /*
             * The difference equation is: y = x + a0*y' + a1*y" + b0*x' + b1*x"
             */
            x = *pwbuffer*factored_B;//r5
            x1 = x>>13;//r5
            y = lbuff_1 + x;
            y >>= 13;
            y1 = y;
            if ( (y>>15) != (y>>31) )
            {
                y  >>= 31;
                y ^= 0x7fff;
            }
            *pwbuffer = y; // write the PCM data
            pwbuffer += mode;

            if(LR == 0)
            {
                lbuff_1 = x1*g_FilterState._bl[0] + y1*g_FilterState._al[0] + lbuff_2;

                // x2 = x1, y2 = y1
                lbuff_2 = x1*g_FilterState._bl[0+1] + y1*g_FilterState._al[0+1];
            }
            else
            {
                lbuff_1 = x1*g_FilterState._bs[0] + y1*g_FilterState._as[0] + lbuff_2;

                // x2 = x1, y2 = y1
                lbuff_2 = x1*g_FilterState._bs[0+1] + y1*g_FilterState._as[0+1];

            }
        }

        g_FilterState.i_32buff[0+LR*10] = lbuff_1;
        g_FilterState.i_32buff[0+1+LR*10] = lbuff_2;
    }
}

/*------------------------------------------------------------------------------
 Function:   I32_MUL
 Synopsis:   Calculate I32*I32 to get I64
 Arguments:  [a]    -- the frist multipier
             [b]    -- the second multipier
             [high] -- the high 32 bit of the I64
             [low]  -- the low 32 bit of the I64
 Returns:
 Attentions: The result may lost some precisions!!!, but we need not to care.
------------------------------------------------------------------------------*/




/*c[2] = (short)lvalue;
c[3] = lvalue>>16;*/

//*high = *((long*)&c[2]);

/*------------------------------------------------------------------------------
 Function:   SubFiltering2
 Synopsis:
 Arguments:
 Returns:
------------------------------------------------------------------------------*/
#if SUPPORT_HIGH_PRECISION
_ATTR_RKEQ_TEXT_
void SubFiltering2(short *pwBuffer, long cwBuffer, long LR, long mode)
{
    short *pwbuffer;
    long  cwbuffer;
    long lbuff_1, lbuff_2;
    long y, y1, x, x1;
    long lData1;
    //long lhigh32, llow32;
    __int64 sum;
    short factB;
    short a0, a1, b0, b1;

    if(LR == 0)
    {
        factB = g_FilterState.factored_BL;
        a0 = g_FilterState._al[0];
        a1 = g_FilterState._al[1];
        b0 = g_FilterState._bl[0];
        b1 = g_FilterState._bl[1];
    }
    else
    {
        factB = g_FilterState.factored_BR;
        a0 = g_FilterState._ar[0];
        a1 = g_FilterState._ar[1];
        b0 = g_FilterState._br[0];
        b1 = g_FilterState._br[1];

    }

    pwbuffer = pwBuffer;
    cwbuffer = cwBuffer;

    //if(LR == 0)//left channel data
    {
        lbuff_1 = g_FilterState.i_32buff[0+LR*10];
        lbuff_2 = g_FilterState.i_32buff[1+LR*10];

        while (cwbuffer--)
        {
            x = *pwbuffer * factB;
            x1 = x;
            y = x + lbuff_1;
            y1 = y;
            y >>= 13;

            if ( (y>>15) != (y>>31) )
            {
                y  >>= 31;
                y ^= 0x7fff;
            }

            *pwbuffer = y; // write the PCM data
            pwbuffer += mode;

#if 1
            if(LR == 0)
            {
                sum = (__int64)(x1)*(g_FilterState._bl[0]);
                lData1 = (long)(sum>>13);
                lData1 += lbuff_2;

                sum = (__int64)(y1)*(g_FilterState._al[0]);
                lbuff_1 = (long)(sum>>13);
                lbuff_1 += lData1;

                sum = (__int64)(x1)*(g_FilterState._bl[1]);
                lData1 = (long)(sum>>13);

                sum = (__int64)(y1)*(g_FilterState._al[1]);
                lbuff_2 = (long)(sum>>13);
                lbuff_2 += lData1;
            }
            else
            {
                sum = (__int64)(x1)*(g_FilterState._bs[0]);
                lData1 = (long)(sum>>13);
                lData1 += lbuff_2;

                sum = (__int64)(y1)*(g_FilterState._as[0]);
                lbuff_1 = (long)(sum>>13);
                lbuff_1 += lData1;

                sum = (__int64)(x1)*(g_FilterState._bs[1]);
                lData1 = (long)(sum>>13);

                sum = (__int64)(y1)*(g_FilterState._as[1]);
                lbuff_2 = (long)(sum>>13);
                lbuff_2 += lData1;

            }
#else // 会存在误差

            lbuff_1 = (long)(((__int64)(x1)*(b0)  + (__int64)(y1)*(a0)) >> 13) + lbuff_2;

            lbuff_2 = (long)(((__int64)(x1)*(b1) + (__int64)(y1)*(a1)) >> 13);

#endif
        }

        g_FilterState.i_32buff[0+LR*10] = lbuff_1;
        g_FilterState.i_32buff[1+LR*10] = lbuff_2;
    }
}
#endif


/*------------------------------------------------------------------------------
 Function:   EQ_Filtering
 Synopsis:   IIR filting a frame of PCM data
 Arguments:  [pwBuffer] -- the PCM data buffer
             [cwBuffer] -- the length of the buffer
             [LR]       -- left or right channel PCM data
                           if is left channel, LR = 0, else LR = 1
             [mode]   --  PCM 存放模式: 1 表示LLL... RRR...;　2 表示LRLRLR...
 Returns:
------------------------------------------------------------------------------*/
_ATTR_RKEQ_TEXT_
void RockEQFiltering(short *pwBuffer, long cwBuffer, long LR, long mode)
{

    short *pwbuffer = pwBuffer;
    long cwbuffer = cwBuffer;
#if SUPPORT_HIGH_PRECISION
    long lAccumulate;
    long lbuff_1, lbuff_2;
    long y, y1, x, x1;
    //long lhigh32, llow32;
    __int64 sum;
#else
    long y;
#endif

    if (cwbuffer < 1) return;

    if(g_FilterState.mode[LR] == EQ_NOR)
    {
        return;
    }

    RockEQReduce9dB(pwBuffer, cwBuffer, LR, mode);

    //-----------------------------------------------------------------
    //Filter 5 [10K]
    //-----------------------------------------------------------------
    //SubFiltering(pwBuffer, cwBuffer, LR, g_FilterState.EQ_ps_factor, 8, mode);
    SubFilter_Core(cwbuffer,pwbuffer, &g_FilterState , 8 , LR );

    //-----------------------------------------------------------------
    //Filter 4 [3.15K]
    //-----------------------------------------------------------------
    //SubFiltering(pwBuffer, cwBuffer, LR, 0, 6, mode);
    SubFilter_Core(cwbuffer,pwbuffer, &g_FilterState , 6 , LR );

    //-----------------------------------------------------------------
    //Filter 3 [1K]
    //-----------------------------------------------------------------
    //SubFiltering(pwBuffer, cwBuffer, LR, 0, 4, mode);
    SubFilter_Core(cwbuffer,pwbuffer, &g_FilterState , 4 , LR );

    //-----------------------------------------------------------------
    //Filter 2 [315Hz]
    //-----------------------------------------------------------------
#if SUPPORT_HIGH_PRECISION
    pwbuffer = pwBuffer;
    cwbuffer = cwBuffer;
    //if(LR == 0)//left channel data
    {
        short a0, a1, b0, b1;

        if(LR == 0)
        {
            a0 = g_FilterState._al[2];
            a1 = g_FilterState._al[3];
            b0 = g_FilterState._bl[2];
            b1 = g_FilterState._bl[3];
        }
        else
        {
            a0 = g_FilterState._as[2];
            a1 = g_FilterState._as[3];
            b0 = g_FilterState._bs[2];
            b1 = g_FilterState._bs[3];

        }
        //g_FilterState.temp_mem = cwBuffer;
        lbuff_1 = g_FilterState.i_32buff[2+LR*10];
        lbuff_2 = g_FilterState.i_32buff[3+LR*10];

        while (cwbuffer--)
        {
            x = *pwbuffer;
            x1 = x;
            y = lbuff_1 + ((unsigned long)x<<13);
            y1 = y;//r8
            y >>= 13;
            if ( (y>>15) != (y>>31) )
            {
                y  >>= 31;
                y ^= 0x7fff;
            }
            *pwbuffer = y; // write the PCM data
            pwbuffer += mode;

#if 0
            lAccumulate = x1 * g_FilterState._b[2] + lbuff_2;//r6
            I32_MUL(y1, g_FilterState._a[2], &lhigh32, &llow32);
            lhigh32 = (unsigned long)lhigh32<<19;
            lbuff_1 = (unsigned long)llow32>>13;
            lbuff_1 |= lhigh32;
            lbuff_1 += lAccumulate;

            lAccumulate = x1 * g_FilterState._b[3];
            I32_MUL(y1, g_FilterState._a[3], &lhigh32, &llow32);
            lhigh32 = (unsigned long)lhigh32<<19;
            lbuff_2 = (unsigned long)llow32>>13;
            lbuff_2 |= lhigh32;
            lbuff_2 += lAccumulate;
#else
            lAccumulate = x1 * b0 + lbuff_2;//r6
            sum = (__int64)(y1)*(a0);
            lbuff_1 = (long)(sum>>13);
            lbuff_1 += lAccumulate;

            lAccumulate = x1 * b1;
            sum = (__int64)(y1)*(a1);
            lbuff_2 = (long)(sum>>13);
            lbuff_2 += lAccumulate;
#endif
        }

        g_FilterState.i_32buff[2+LR*10] = lbuff_1;
        g_FilterState.i_32buff[3+LR*10] = lbuff_2;
    }
#else
    /* A method may lose some precision, but more quick.
     */
    SubFiltering(pwBuffer, cwBuffer, LR, 0, 2, mode);
#endif

    //-----------------------------------------------------------------
    //Filter 1 [100Hz]
    //-----------------------------------------------------------------


    SubFiltering2(pwBuffer, cwBuffer, LR, mode);

}


_ATTR_RKEQ_TEXT_
long long  signed_Rshift(long long in)
{
    if (in >= 0)
    {
        in = in >>20;
    }
    else
    {
        in =  -((-in)>>20);
    }
    return in;
}
_ATTR_RKEQ_TEXT_
void filter_2_int_bass(short *pwBuffer, unsigned short frameLen,int index,int L)
{
    long long y0;
    long b0;
    long b1;
    long b2;
    long a1;
    long a2;
    short x0;
    short x1;
    short x2;
    long long y1;
    long long y2;
    short i;

    if(L == 0)
    {
        a1 = g_bass_FilterState._al[index*2];
        a2 = g_bass_FilterState._al[index*2+1];
        b0 = g_bass_FilterState._bl[index*3];
        b1 = g_bass_FilterState._bl[index*3+1];
        b2 = g_bass_FilterState._bl[index*3+2];
    }
    else
    {
        a1 = g_bass_FilterState._ar[index*2];
        a2 = g_bass_FilterState._ar[index*2+1];
        b0 = g_bass_FilterState._br[index*3];
        b1 = g_bass_FilterState._br[index*3+1];
        b2 = g_bass_FilterState._br[index*3+2];
    }

    x1 =g_bass_FilterState.x[index*2+4*L];
    x2 =g_bass_FilterState.x[1+index*2+4*L];
    y1 = g_bass_FilterState.y[index*2+4*L];
    y2 = g_bass_FilterState.y[1+index*2+4*L];
    for (i = 0; i < frameLen; i++)
    {
        x0 = *pwBuffer;
        y0 = ((long long)b0 * (long long)x0) + ((long long)b1 * (long long)x1) + ((long long)b2 * (long long)x2) + signed_Rshift(((long long)a1 * (long long)y1))+ signed_Rshift(((long long)a2 * (long long)y2));
        y2 = y1;
        y1 = y0;
        if (y0 <0)
        {
            if ( y0 < - 34359738368)
            {
                y0 = -32768;
            }
            else
            {
                y0 = -((-y0)>>20);
            }
        }
        else
        {
            if (y0 > 34359738368)
            {
                y0 = 32767;
            }
            else
            {
                y0 = y0 >>20;
            }
        }
        x2 = x1;
        x1 = x0;
        *pwBuffer = y0;
        pwBuffer += 2;
    }
    g_bass_FilterState.x[index*2+4*L] = x1;
    g_bass_FilterState.x[1+index*2+4*L] = x2;
    g_bass_FilterState.y[index*2+4*L] = y1;
    g_bass_FilterState.y[1+index*2+4*L] = y2;
}
void RockBassFiltering(short *pwBuffer, long cwBuffer, long LR, long mode)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;

    //-----------------------------------------------------------------
    //Filter 2 [60]
    //-----------------------------------------------------------------
    // SubFiltering_bass(pwBuffer, cwBuffer, LR, 0, 2, mode);
    //SubFilter_Core(cwbuffer,pwbuffer, &g_bass_FilterState , 2 , LR );
    filter_2_int_bass(pwBuffer, cwBuffer, 1, LR);

    //-----------------------------------------------------------------
    //Filter 1 [30]
    //-----------------------------------------------------------------
    // SubFiltering_bass(pwBuffer, cwBuffer, LR, 0, 0, mode);
    filter_2_int_bass(pwBuffer, cwBuffer, 0, LR);

}

/*------------------------------------------------------------------------------
 Function:   RockEQAdjust
 Synopsis:
 Arguments:
 Returns:
------------------------------------------------------------------------------*/
_ATTR_RKEQ_TEXT_
void RockEQAdjust(long SmpRate, short *g, short db) //db=1
{
    long iFs;
    short dbReduce;

    switch (SmpRate)
    {
        case(8000):
            iFs = 0;
            break;
        case(11025):
            iFs = 1;
            break;
        case(12000):
            iFs = 2;
            break;
        case(16000):
            iFs = 3;
            break;
        case(22050):
            iFs = 4;
            break;
        case(24000):
            iFs = 5;
            break;
        case(32000):
            iFs = 6;
            break;
        case(44100):
            iFs = 7;
            break;
        case(48000):
            iFs = 8;
            break;
        default :
            iFs = 7;
            break;
    }

    /* IIR Filter Delay buffer clear */
   // EQ_ClearBuff();

    /* IIR Filter Coefficient computation */
    ROCKEQ_Get_Coeff(g, iFs);

    //////////////////////////////////////
    dbReduce = db; // 正确
    //////////////////////////////////////
}

/*------------------------------------------------------------------------------
 Function:   RockEQProcess
 Synopsis:
 Arguments:
 Returns:
------------------------------------------------------------------------------*/
_ATTR_RKEQ_TEXT_
void RockEQProcess(short *pData, long PcmLen)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;
    uint32 i , j[2];

    if(pEffect->Mode== EQ_BASS)
    {
        if (pEffect->max_DbGain != 0)
        {
            if(g_bass_FilterState.EqAjustOk[0])
            {
                g_bass_FilterState.EqAjustOk[0]++;
            }

            if(g_bass_FilterState.EqAjustOk[1])
            {
                g_bass_FilterState.EqAjustOk[1]++;
            }

            if((g_bass_FilterState.EqAjustOk[0] > 2) && (g_bass_FilterState.EqAjustOk[1] > 2))
            {
                EqAdjustVolume();
                g_bass_FilterState.EqAjustOk[0] = 0;
                g_bass_FilterState.EqAjustOk[1] = 0;
            }

            j[0] = 0;
            j[1] = 0;

            if(g_bass_FilterState.max_DbGain[0] != pEffect->max_DbGain)
            {
                for(i = 0; i < PcmLen; i++)
                {
                    if((pData[i * 2] < 200) && (pData[i * 2] > -200))
                    {
                        #if 0
                        printf("\nj[0]  = %d\n", j[0]);

                        if(j[0] >= 2)
                        {
                            g_bass_FilterState.ls[0] = pData[j[0] * 2 - 4];
                            g_bass_FilterState.ls[1] = pData[j[0] * 2 - 2];
                        }
                        else if(j[0] == 1)
                        {
                            g_bass_FilterState.ls[1] = pData[j[0] * 2 - 2];
                        }

                        printf("sl %d, %d\n", g_bass_FilterState.max_DbGain[0], pEffect->max_DbGain);
                        printf("Ls:%d, %d, %d, %d\n", g_bass_FilterState.ls[0], g_bass_FilterState.ls[1], pData[i * 2], pData[i * 2 + 2]);

                        #endif

                        if(g_bass_FilterState.max_DbGain[0])
                        {
                            RockEQReduce9dB(pData, j[0], 0, 2);
                            RockBassFiltering(pData, j[0], 0, 2); // left channel;
                        }

                        #if 0
                        if(j[0] >= 2)
                        {
                            g_bass_FilterState.l[0] = pData[j[0] * 2 - 4];
                            g_bass_FilterState.l[1] = pData[j[0] * 2 - 2];
                        }
                        else if(j[0] == 1)
                        {
                            g_bass_FilterState.l[1] = pData[j[0] * 2 - 2];
                        }
                        #endif

                        g_bass_FilterState._al[0] = g_bass_FilterState._as[0];
                        g_bass_FilterState._al[1] = g_bass_FilterState._as[1];
                        g_bass_FilterState._al[2] = g_bass_FilterState._as[2];
                        g_bass_FilterState._al[3] = g_bass_FilterState._as[3];

                        g_bass_FilterState._bl[0] = g_bass_FilterState._bs[0];
                        g_bass_FilterState._bl[1] = g_bass_FilterState._bs[1];
                        g_bass_FilterState._bl[2] = g_bass_FilterState._bs[2];
                        g_bass_FilterState._bl[3] = g_bass_FilterState._bs[3];
                        g_bass_FilterState._bl[4] = g_bass_FilterState._bs[4];
                        g_bass_FilterState._bl[5] = g_bass_FilterState._bs[5];

                        g_bass_FilterState.max_DbGain[0] = pEffect->max_DbGain;
                        g_bass_FilterState.EqAjustOk[0] = 1;

                        break;

                    }
                    j[0]++;

                }

                #if 0
                if(i == PcmLen)
                {
                    g_bass_FilterState.ls[0] = pData[PcmLen * 2 - 4];
                    g_bass_FilterState.ls[1] = pData[PcmLen * 2 - 2];
                }
                #endif
            }

            if(g_bass_FilterState.max_DbGain[1] != pEffect->max_DbGain)
            {
                for(i = 0; i < PcmLen; i++)
                {
                    if((pData[i * 2 + 1] < 200) && (pData[i * 2 + 1] > -200))
                    {

                        #if 0
                        printf("\nj[1]  = %d\n", j[1]);

                        if(j[1] >= 2)
                        {
                            g_bass_FilterState.rs[0] = pData[j[1] * 2 - 3];
                            g_bass_FilterState.rs[1] = pData[j[1] * 2 - 1];
                        }
                        else if(j[1] == 1)
                        {
                            g_bass_FilterState.rs[1] = pData[j[1] * 2 - 1];
                        }

                        printf("sr %d, %d\n", g_bass_FilterState.max_DbGain[1], pEffect->max_DbGain);
                        printf("Rs:%d, %d, %d, %d\n", g_bass_FilterState.rs[0], g_bass_FilterState.rs[1], pData[i * 2 + 1], pData[i * 2 + 3]);
                        #endif

                        if(g_bass_FilterState.max_DbGain[1])
                        {
                            RockEQReduce9dB(pData + 1, j[1], 1, 2);
                            RockBassFiltering(pData+1, j[1], 1, 2); // right channel
                        }

                        #if 0
                        if(j[1] >= 2)
                        {
                            g_bass_FilterState.r[0] = pData[j[1] * 2 - 3];
                            g_bass_FilterState.r[1] = pData[j[1] * 2 - 1];
                        }
                        else if(j[1] == 1)
                        {
                            g_bass_FilterState.r[1] = pData[j[1] * 2 - 1];
                        }
                        #endif

                        g_bass_FilterState._ar[0] = g_bass_FilterState._as[0];
                        g_bass_FilterState._ar[1] = g_bass_FilterState._as[1];
                        g_bass_FilterState._ar[2] = g_bass_FilterState._as[2];
                        g_bass_FilterState._ar[3] = g_bass_FilterState._as[3];

                        g_bass_FilterState._br[0] = g_bass_FilterState._bs[0];
                        g_bass_FilterState._br[1] = g_bass_FilterState._bs[1];
                        g_bass_FilterState._br[2] = g_bass_FilterState._bs[2];
                        g_bass_FilterState._br[3] = g_bass_FilterState._bs[3];
                        g_bass_FilterState._br[4] = g_bass_FilterState._bs[4];
                        g_bass_FilterState._br[5] = g_bass_FilterState._bs[5];

                        g_bass_FilterState.max_DbGain[1] = pEffect->max_DbGain;
                        g_bass_FilterState.EqAjustOk[1] = 1;

                        break;

                    }

                    j[1]++;

                }

                #if 0

                if(i == PcmLen)
                {
                    g_bass_FilterState.rs[0] = pData[PcmLen * 2 - 3];
                    g_bass_FilterState.rs[1] = pData[PcmLen * 2 - 1];
                }
                #endif

            }

            RockEQReduce9dB(pData + j[0] * 2, PcmLen - j[0], 0, 2);
            RockBassFiltering(pData + j[0] * 2, PcmLen - j[0], 0, 2); // left channel;


            RockEQReduce9dB(pData + 1 + j[1] * 2, PcmLen - j[1], 1, 2);
            RockBassFiltering(pData+ 1 + j[1] * 2, PcmLen - j[1], 1, 2); // right channel

            #if 0
            if(j[0])
            {
                printf("\nL:%d, %d, %d, %d", g_bass_FilterState.l[0], g_bass_FilterState.l[1], pData[j[0] * 2], pData[j[0] * 2 + 2]);
            }

            if(j[1])
            {
                printf("\nR:%d, %d, %d, %d", g_bass_FilterState.r[0], g_bass_FilterState.r[1], pData[j[1] * 2 + 1], pData[j[1] * 2 + 3]);
            }

            g_bass_FilterState.l[0] = pData[PcmLen * 2 - 4];
            g_bass_FilterState.l[1] = pData[PcmLen * 2 - 2];
            g_bass_FilterState.r[0] = pData[PcmLen * 2 - 3];
            g_bass_FilterState.r[1] = pData[PcmLen * 2 - 1];
            #endif

       }

    }
    else
    {

        if(g_FilterState.EqAjustOk[0])
        {
            g_FilterState.EqAjustOk[0]++;
        }

        if(g_FilterState.EqAjustOk[1])
        {
            g_FilterState.EqAjustOk[1]++;
        }

        if((g_FilterState.EqAjustOk[0] > 2) && (g_FilterState.EqAjustOk[1] > 2))
        {
            //EqAdjustVolume();
            g_FilterState.EqAjustOk[0] = 0;
            g_FilterState.EqAjustOk[1] = 0;
        }

        j[0] = 0;
        j[1] = 0;

        #if 1

        if((g_FilterState.mode[0] != pEffect->Mode)
            || (g_FilterState._al[0] != g_FilterState._as[0])
            || (g_FilterState._al[1] != g_FilterState._as[1])
            || (g_FilterState._al[2] != g_FilterState._as[2])
            || (g_FilterState._al[3] != g_FilterState._as[3])
            || (g_FilterState._al[4] != g_FilterState._as[4])
            || (g_FilterState._al[5] != g_FilterState._as[5])
            || (g_FilterState._al[6] != g_FilterState._as[6])
            || (g_FilterState._al[7] != g_FilterState._as[7])
            || (g_FilterState._al[8] != g_FilterState._as[8])
            || (g_FilterState._al[9] != g_FilterState._as[9])
            || (g_FilterState._bl[0] != g_FilterState._bs[0])
            || (g_FilterState._bl[1] != g_FilterState._bs[1])
            || (g_FilterState._bl[2] != g_FilterState._bs[2])
            || (g_FilterState._bl[3] != g_FilterState._bs[3])
            || (g_FilterState._bl[4] != g_FilterState._bs[4])
            || (g_FilterState._bl[5] != g_FilterState._bs[5])
            || (g_FilterState._bl[6] != g_FilterState._bs[6])
            || (g_FilterState._bl[7] != g_FilterState._bs[7])
            || (g_FilterState._bl[8] != g_FilterState._bs[8])
            || (g_FilterState._bl[9] != g_FilterState._bs[9]))
        {
            for(i = 0; i < PcmLen; i++)
            {
                if((pData[i * 2] < 100) && (pData[i * 2] > -100))
                {

                    #if 0
                    printf("\nj[0]  = %d\n", j[0]);

                    if(j[0] >= 2)
                    {
                        g_FilterState.ls[0] = pData[j[0] * 2 - 4];
                        g_FilterState.ls[1] = pData[j[0] * 2 - 2];
                    }
                    else if(j[0] == 1)
                    {
                        g_FilterState.ls[1] = pData[j[0] * 2 - 2];
                    }

                    printf("sl %d, %d\n", g_FilterState.mode[0], pEffect->Mode);
                    printf("Ls:%d, %d, %d, %d\n", g_FilterState.ls[0], g_FilterState.ls[1], pData[i * 2], pData[i * 2 + 2]);
                    #endif

                    RockEQFiltering(pData, j[0], 0, 2); // left channel;

                    #if 0
                    if(j[0] >= 2)
                    {
                        g_FilterState.l[0] = pData[j[0] * 2 - 4];
                        g_FilterState.l[1] = pData[j[0] * 2 - 2];
                    }
                    else if(j[0] == 1)
                    {
                        g_FilterState.l[1] = pData[j[0] * 2 - 2];
                    }
                    #endif

                    //if(g_FilterState.mode[0] == EQ_NOR)
                    if(1)
                    {
                    g_FilterState._al[0] = g_FilterState._as[0];
                    g_FilterState._al[1] = g_FilterState._as[1];
                    g_FilterState._al[2] = g_FilterState._as[2];
                    g_FilterState._al[3] = g_FilterState._as[3];
                    g_FilterState._al[4] = g_FilterState._as[4];
                    g_FilterState._al[5] = g_FilterState._as[5];
                    g_FilterState._al[6] = g_FilterState._as[6];
                    g_FilterState._al[7] = g_FilterState._as[7];
                    g_FilterState._al[8] = g_FilterState._as[8];
                    g_FilterState._al[9] = g_FilterState._as[9];

                    g_FilterState._bl[0] = g_FilterState._bs[0];
                    g_FilterState._bl[1] = g_FilterState._bs[1];
                    g_FilterState._bl[2] = g_FilterState._bs[2];
                    g_FilterState._bl[3] = g_FilterState._bs[3];
                    g_FilterState._bl[4] = g_FilterState._bs[4];
                    g_FilterState._bl[5] = g_FilterState._bs[5];
                    g_FilterState._bl[6] = g_FilterState._bs[6];
                    g_FilterState._bl[7] = g_FilterState._bs[7];
                    g_FilterState._bl[8] = g_FilterState._bs[8];
                    g_FilterState._bl[9] = g_FilterState._bs[9];

                    g_FilterState.factored_BL = g_FilterState.factored_BS;

                    g_FilterState.mode[0] = pEffect->Mode;
                    //g_FilterState.EqAjustOk[0] = 1;
                    }
                    else
                    {
                    //pEffect->Mode = g_FilterState.mode[0];
                    }
                    break;

                }

                j[0]++;

            }

            #if 0
            if(i == PcmLen)
            {
                g_FilterState.ls[0] = pData[PcmLen * 2 - 4];
                g_FilterState.ls[1] = pData[PcmLen * 2 - 2];
            }
            #endif
        }

        if((g_FilterState.mode[1] != pEffect->Mode)
            || (g_FilterState._ar[0] != g_FilterState._as[0])
            || (g_FilterState._ar[1] != g_FilterState._as[1])
            || (g_FilterState._ar[2] != g_FilterState._as[2])
            || (g_FilterState._ar[3] != g_FilterState._as[3])
            || (g_FilterState._ar[4] != g_FilterState._as[4])
            || (g_FilterState._ar[5] != g_FilterState._as[5])
            || (g_FilterState._ar[6] != g_FilterState._as[6])
            || (g_FilterState._ar[7] != g_FilterState._as[7])
            || (g_FilterState._ar[8] != g_FilterState._as[8])
            || (g_FilterState._ar[9] != g_FilterState._as[9])
            || (g_FilterState._br[0] != g_FilterState._bs[0])
            || (g_FilterState._br[1] != g_FilterState._bs[1])
            || (g_FilterState._br[2] != g_FilterState._bs[2])
            || (g_FilterState._br[3] != g_FilterState._bs[3])
            || (g_FilterState._br[4] != g_FilterState._bs[4])
            || (g_FilterState._br[5] != g_FilterState._bs[5])
            || (g_FilterState._br[6] != g_FilterState._bs[6])
            || (g_FilterState._br[7] != g_FilterState._bs[7])
            || (g_FilterState._br[8] != g_FilterState._bs[8])
            || (g_FilterState._br[9] != g_FilterState._bs[9]))
        {
            for(i = 0; i < PcmLen; i++)
            {
                if((pData[i * 2 + 1] < 100) && (pData[i * 2 + 1] > -100))
                {
                    #if 0
                    printf("\nj[1]  = %d\n", j[1]);

                    if(j[1] >= 2)
                    {
                        g_FilterState.rs[0] = pData[j[1] * 2 - 3];
                        g_FilterState.rs[1] = pData[j[1] * 2 - 1];
                    }
                    else if(j[1] == 1)
                    {
                        g_FilterState.rs[1] = pData[j[1] * 2 - 1];
                    }

                    printf("sr %d, %d\n", g_FilterState.mode[1], pEffect->Mode);
                    printf("Rs:%d, %d, %d, %d\n", g_FilterState.rs[0], g_FilterState.rs[1], pData[i * 2 + 1], pData[i * 2 + 3]);
                    #endif

                    RockEQFiltering(pData + 1, j[1], 1, 2); // left channel;

                    #if 0
                    if(j[1] >= 2)
                    {
                        g_FilterState.r[0] = pData[j[1] * 2 - 3];
                        g_FilterState.r[1] = pData[j[1] * 2 - 1];
                    }
                    else if(j[1] == 1)
                    {
                        g_FilterState.r[1] = pData[j[1] * 2 - 1];
                    }
                    #endif

                    //if(g_FilterState.mode[1] == EQ_NOR)
                    if(1)
                    {
                    g_FilterState._ar[0] = g_FilterState._as[0];
                    g_FilterState._ar[1] = g_FilterState._as[1];
                    g_FilterState._ar[2] = g_FilterState._as[2];
                    g_FilterState._ar[3] = g_FilterState._as[3];
                    g_FilterState._ar[4] = g_FilterState._as[4];
                    g_FilterState._ar[5] = g_FilterState._as[5];
                    g_FilterState._ar[6] = g_FilterState._as[6];
                    g_FilterState._ar[7] = g_FilterState._as[7];
                    g_FilterState._ar[8] = g_FilterState._as[8];
                    g_FilterState._ar[9] = g_FilterState._as[9];

                    g_FilterState._br[0] = g_FilterState._bs[0];
                    g_FilterState._br[1] = g_FilterState._bs[1];
                    g_FilterState._br[2] = g_FilterState._bs[2];
                    g_FilterState._br[3] = g_FilterState._bs[3];
                    g_FilterState._br[4] = g_FilterState._bs[4];
                    g_FilterState._br[5] = g_FilterState._bs[5];
                    g_FilterState._br[6] = g_FilterState._bs[6];
                    g_FilterState._br[7] = g_FilterState._bs[7];
                    g_FilterState._br[8] = g_FilterState._bs[8];
                    g_FilterState._br[9] = g_FilterState._bs[9];

                    g_FilterState.factored_BR = g_FilterState.factored_BS;
                    g_FilterState.mode[1] = pEffect->Mode;
                    //g_FilterState.EqAjustOk[1] = 1;
                    }
                    else
                    {

                    pEffect->Mode = g_FilterState.mode[1];
                    }
                    break;

                }

                j[1]++;

            }

            #if 0
            if(i == PcmLen)
            {
                g_FilterState.rs[0] = pData[PcmLen * 2 - 3];
                g_FilterState.rs[1] = pData[PcmLen * 2 - 1];
            }
            #endif

        }
        #endif

        RockEQFiltering(pData + j[0] * 2, PcmLen - j[0], 0, 2); // left channel;
        RockEQFiltering(pData + 1 + j[1] * 2, PcmLen - j[1], 1, 2); // right channel

        #if 0
        if(j[0])
        {
            printf("\nL:%d, %d, %d, %d", g_FilterState.l[0], g_FilterState.l[1], pData[j[0] * 2], pData[j[0] * 2 + 2]);
        }

        if(j[1])
        {
            printf("\nR:%d, %d, %d, %d", g_FilterState.r[0], g_FilterState.r[1], pData[j[1] * 2 + 1], pData[j[1] * 2 + 3]);
        }

        g_FilterState.l[0] = pData[PcmLen * 2 - 4];
        g_FilterState.l[1] = pData[PcmLen * 2 - 2];
        g_FilterState.r[0] = pData[PcmLen * 2 - 3];
        g_FilterState.r[1] = pData[PcmLen * 2 - 1];
        #endif
    }
}
