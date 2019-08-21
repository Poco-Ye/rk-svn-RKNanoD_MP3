/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��   PowerManager.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _POWER_H
#define _POWER_H

#undef  EXT
#ifdef _IN_POWER
#define EXT
#else
#define EXT extern
#endif

#define _ATTR_VECTT_PMU_             __attribute__((section("pmu_vect")))
#define _ATTR_PMU_CODE_              __attribute__((section("pmucode")))
#define _ATTR_PMU_DATA_              __attribute__((section("pmudata")))
#define _ATTR_PMU_BSS_              __attribute__((section("pmudata"),zero_init))


/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
typedef enum _FREQ_APP
{
    FREQ_IDLE = 0,
    FREQ_MIN,
    FREQ_INIT,
    FREQ_BLON,
    FREQ_AUDIO_INIT,
    FREQ_MP3,       //5
    FREQ_MP3H,
    FREQ_WMA,
    FREQ_WMAH,
    FREQ_WAV,
    FREQ_AACL,
    FREQ_AAC,       //10
    FREQ_FLAC,
    FREQ_SBC_ENCODING,
    FREQ_BLUETOOTH,
    FREQ_SSRC,
    FREQ_EQ,        //20
    FREQ_JPG,
    FREQ_FM,
    FREQ_RECORDADPCM,
    FREQ_RECORDMP3,
    FREQ_FMAUTOSEARCH,
    FREQ_MEDIAUPDATA,//25
    FREQ_USB,
    FREQ_BEEP,
    FREQ_MEDIA_INIT, //30
    FREQ_MAX,       //32

    FREQ_APP_MAX

}eFREQ_APP;

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
typedef struct tagCRU_CLK_INFO
{
    uint32 armFreq;		//ARM PLL FREQ
    uint32 armFreqLast;

} CRU_CLK_INFO,*pCRU_CLK_INFO;

typedef struct tagFREQ_APP_TABLE
{
    uint8  scuAppId;
    uint8  counter;

    uint32 PllFreq;
    uint32 syshclk; //sysfclk
    uint32 sysstclk;
    uint32 syspclk;

    uint32 calhclk; //calfclk
    uint32 calstclk;

}FREQ_APP_TABLE,*pFREQ_APP_TABLE;


/*
*-------------------------------------------------------------------------------
*
*                           Functon Declaration
*
*-------------------------------------------------------------------------------
*/
extern int32 FREQ_Enable(void);
extern int32 FREQ_Disable(void);

extern int32 FREQ_EnterModule(eFREQ_APP modulename);
extern int32 FREQ_ExitModule(eFREQ_APP modulename);

/*
********************************************************************************
*
*                         End of Pmu.h
*
********************************************************************************
*/
#endif


