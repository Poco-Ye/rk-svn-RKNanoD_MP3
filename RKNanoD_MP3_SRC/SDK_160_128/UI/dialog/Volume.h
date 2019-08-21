/*
********************************************************************************
*                   Copyright (c) 2013,Pegatron
*                         All rights reserved.
*
* File Name:  Volume.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               Pegatron        2013-10-01         1.0
*    desc:      ORG.
********************************************************************************
*/

#ifndef  _DIALOG_VOLUME_H
#define  _DIALOG_VOLUME_H

#undef  EXT
#ifdef  _IN_DIALOG_VOLUME_
#define EXT 
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*  
*                           Macro define
*  
*-------------------------------------------------------------------------------
*/
#define _ATTR_VOLUME_CODE_         __attribute__((section("DialogVolumeCode")))
#define _ATTR_VOLUME_DATA_         __attribute__((section("DialogVolumeData")))
#define _ATTR_VOLUME_BSS_          __attribute__((section("DialogVolumeBss"),zero_init))

#define VOLUME_DISPLAY_TIME                          1.5

#define VOLUME_UP                                    0
#define VOLUME_DOWN                                  1
#define VOLUME_CHECK_ALVS_SET_ON                     2
#define VOLUME_SOUND_PRESSURE_COUNTER_EXPIRED        3

#define MAX_VOLUME_AVLS_WORLDWIDE                    19          /* except Europe */
#define MAX_VOLUME_AVLS_EUROPE                       19
#define EUROPE_FADEDOWN_VOLUME                       10
#define EUROPE_SOUND_PRESSURE_THRESHOLD              19
#define EUROPE_VOLUME_LIMIT_ALARM_TIME               2.5         /* One alarm 3 seconds, set to 2.5 seconds for latency */

/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
_ATTR_VOLUME_BSS_ EXT UINT32      VolumeSystickCounterBack;

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
extern void DialogVolumeInit(void *pArg);
extern void DialogVolumeDeInit(void);
extern uint32 DialogVolumeService(void);
extern uint32 DialogVolumeKey(void);
extern void DialogVolumeDisplay(void);


/*
********************************************************************************
*
*                         End of Volume.h
*
********************************************************************************
*/
#endif

