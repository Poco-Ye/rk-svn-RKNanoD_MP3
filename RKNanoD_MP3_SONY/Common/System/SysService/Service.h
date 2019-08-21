/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   SysService.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SYSSERVICE_H_
#define _SYSSERVICE_H_

#undef  EXT
#ifdef _IN_SYSSERVICE_
#define EXT
#else
#define EXT extern
#endif

typedef enum
{
    Voice_PLAY,
    Voice_STOP
}VoiceNotifySTATE;

typedef enum
{
	BEEP_PLAY = 0,
	BEEP_AMS_PLUS,
	BEEP_AMS_MINUS,
	BEEP_FM_PRESET_REGIST,
	BEEP_FM_PRESET_DELETE,
	BEEP_LOW_BATTERY,
	BEEP_AMS_RETURN,
	BEEP_STOP,
	BEEP_NUM

}BEEP_ID;


_ATTR_SYS_BSS_ EXT  UINT8   SetPowerOffFlag;
_ATTR_SYS_BSS_ EXT  INT8    AutoPowerOffDisableCounter;
_ATTR_SYS_BSS_ EXT  uint32 DemoModePauseSystickCounterBack;//PAGE

_ATTR_SYS_BSS_ EXT  UINT16  AudioPlayState;
_ATTR_SYS_BSS_ EXT  UINT16  AudioPlayerState;
_ATTR_SYS_BSS_ EXT  UINT16  hPlayRecordFile;
_ATTR_SYS_BSS_ EXT  UINT16  AudioCodecOpenErr;
_ATTR_SYS_BSS_ EXT  uint32  AudioPtr;
_ATTR_SYS_BSS_ EXT  uint32  AudioDecodeing;
_ATTR_SYS_BSS_ EXT  uint32  DmaTransting;
_ATTR_SYS_BSS_ EXT  uint32  AudioNeedDecode;
_ATTR_SYS_BSS_ EXT  uint32  AudioLen;
_ATTR_SYS_BSS_ EXT  UINT32  Acodec_over_cur_value;
_ATTR_SYS_BSS_ EXT  UINT32  Acodec_over_cur_count;
_ATTR_SYS_BSS_ EXT  UINT16      REC_Flag;                   // flag to store updating status for recording;
#ifdef _BEEP_
_ATTR_SYS_BSS_ EXT uint16 BeepPlayState;
_ATTR_SYS_BSS_ EXT uint32 BeepPlayerState;
_ATTR_SYS_BSS_ EXT uint32 BeepSampleRateBK;
_ATTR_SYS_BSS_ EXT uint32 BeepCodecmodeBK;
_ATTR_SYS_BSS_ EXT uint32 BeepCodecdatawidthBK;

#define BEEP_VOL_SET    12
#endif


#ifdef _RADIO_
//HJ
_ATTR_SYS_BSS_  EXT UINT32   FmFunctionSele;
_ATTR_SYS_BSS_  EXT UINT16   FmListSaveAndDel;
#endif


/*-----------------------------------------------------------------------------*/
//Service.c
extern UINT32 VolumeLimit;
extern UINT8 FactoryVolumeLimit;

extern void SysServiceInit(void);
extern UINT32 SysService(void);

extern void AutioPowerOffTimerRest(void);
extern void AutoPowerOffEnable(void);
extern void AutoPowerOffDisable(void);
extern void SetPowerOffTimerEnable(void);
extern void SetPowerOffTimerDisable(void);

extern void AudioDmaIsrHandler(void);
extern void AudioDecoding(void);
extern UINT8 Audio_HP_DET(void);

#ifdef _BEEP_
extern void BeepPlay(uint8 BeepId, int LoopCnt, uint8 BeepVolume, uint8 MusicFade, uint8 MusicFadeDeep);
extern void BeepStop(void);
extern void BeepInit(void);
#endif

/*
********************************************************************************
*
*                         End of SysService.h
*
********************************************************************************
*/
#endif
