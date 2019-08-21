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

#define _IN_DIALOG_VOLUME_

#include "SysInclude.h"
#include "Volume.h"
#include "Hold.h"
#include "FMControl.h"
#include "AudioControl.h"
#include "VideoControl.h"

#define VolumeWinVol gSysConfig.OutputVolume

BOOL ShowAvlsIcon = FALSE;
BOOL ShowAvlsWarning = FALSE;
BOOL ShowEuropeVolumeLimitAlarm = FALSE;
BOOL EuropeVolumeLimitAlarmConfirmed = FALSE;

/*
--------------------------------------------------------------------------------
  Function name : void DialogVolumeInit(void *pArg)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VOLUME_CODE_
void DialogVolumeInit(void *pArg)
{
    uint16 VolumeAction;
    UINT8 MaxVolumeLimit = MAX_VOLUME;

    if (gSysConfig.AvlsEnabled)
    {
        MaxVolumeLimit = MAX_VOLUME_AVLS_WORLDWIDE;
    }
    else
    {
        MaxVolumeLimit = MAX_VOLUME;
    }

    ShowAvlsIcon = FALSE;
    ShowAvlsWarning = FALSE;
    ShowEuropeVolumeLimitAlarm = FALSE;
    EuropeVolumeLimitAlarmConfirmed = FALSE;

    //initial timing
    VolumeSystickCounterBack = SysTickCounter;

    if (pArg != NULL)
    {
        VolumeAction = ((VOLUME_WIN_ARG*)pArg)->VolumeAction;

        switch (VolumeAction)
        {
        case VOLUME_UP:
            if (VolumeWinVol < MaxVolumeLimit)
            {
                VolumeWinVol++;

#ifdef _MUSIC_
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    MusicDecodeProc(MSG_AUDIO_VOLUMESET,NULL);
                }
#endif
#ifdef _RADIO_
                if (TRUE == ThreadCheck(pMainThread, &FMThread))
                {
                    FM_Process(MSG_RADIO_VOLUMESET,NULL);
                }
#endif
#ifdef _VIDEO_
                if (TRUE == ThreadCheck(pMainThread, &VideoThread))
                {
                    VideoControlProc(MSG_XVID_VOLUMESET,VolumeWinVol);
                }
#endif

                if (ShowEuropeVolumeLimitAlarm)
                {
                    #ifdef _BEEP_
	            if (!gSysConfig.BeepEnabled) {
                        BeepPlay(BEEP_LOW_BATTERY, -1, BEEP_VOL_SET, 0, 0);
                    }
                    #endif
                }
            }
            else
            {
                if (gSysConfig.AvlsEnabled)
                {
                    ShowAvlsIcon = TRUE;
                }
            }
            break;

        case VOLUME_DOWN:
            if (VolumeWinVol)
            {
                VolumeWinVol--;
#ifdef _MUSIC_
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    MusicDecodeProc(MSG_AUDIO_VOLUMESET,NULL);
                }
#endif
#ifdef _RADIO_
                if (TRUE == ThreadCheck(pMainThread, &FMThread))
                {
                    FM_Process(MSG_RADIO_VOLUMESET,NULL);
                }
#endif
#ifdef _VIDEO_
                if (TRUE == ThreadCheck(pMainThread, &VideoThread))
                {
                    VideoControlProc(MSG_XVID_VOLUMESET,VolumeWinVol);
                }
#endif
            }
            break;

        case VOLUME_CHECK_ALVS_SET_ON:
            if ((gSysConfig.AvlsEnabled) && (VolumeWinVol > MaxVolumeLimit))
            {
                ShowAvlsWarning = TRUE;

                VolumeWinVol = MaxVolumeLimit;
#ifdef _MUSIC_
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    MusicDecodeProc(MSG_AUDIO_VOLUMESET,NULL);
                }
#endif
#ifdef _RADIO_
                if (TRUE == ThreadCheck(pMainThread, &FMThread))
                {
                    FM_Process(MSG_RADIO_VOLUMESET,NULL);
                }
#endif
#ifdef _VIDEO_
                if (TRUE == ThreadCheck(pMainThread, &VideoThread))
                {
                    VideoControlProc(MSG_XVID_VOLUMESET,VolumeWinVol);
                }
#endif
            }
            else
            {
                /* Force Volume UI destory soon */
                VolumeSystickCounterBack = (VolumeSystickCounterBack > (VOLUME_DISPLAY_TIME * 2 * 100)) ?
                                               (VolumeSystickCounterBack - (VOLUME_DISPLAY_TIME * 2 * 100)) : 0;
                return;
            }
            break;

        case VOLUME_SOUND_PRESSURE_COUNTER_EXPIRED:

            break;

        default:
            break;
        }
    }

    KeyReset();
    SendMsg(MSG_VOLUME_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void MessageBoxDeInit(void)
  Author        :
  Description   : auti-initial function.

  Input         :
  Return        :

  History:     <author>         <time>         <version>       

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VOLUME_CODE_
void DialogVolumeDeInit(void)
{
    
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 DialogVolumeService(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>       

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VOLUME_CODE_
uint32 DialogVolumeService(void)
{
    INT16 Retval = 0;
    UINT32 Count;

    if (ShowEuropeVolumeLimitAlarm)
    {
        if (EuropeVolumeLimitAlarmConfirmed)
        {
            Count = SysTickCounter - VolumeSystickCounterBack;
            if (Count > EUROPE_VOLUME_LIMIT_ALARM_TIME * 100)
            {
                #ifdef _BEEP_
                BeepStop();
                #endif

                //WinDestroy(&VolumeWin);
                SendMsg(MSG_VOLUMEDIALOG_DESTROY);
                SendMsg(MSG_NEED_PAINT_ALL);
            }
        }
    }
    else
    {
        //check regular time, timing is coming
        Count = SysTickCounter - VolumeSystickCounterBack;
        if (Count > VOLUME_DISPLAY_TIME * 100)
        {
            //WinDestroy(&VolumeWin);
            SendMsg(MSG_VOLUMEDIALOG_DESTROY);
            SendMsg(MSG_NEED_PAINT_ALL);
        }
    }

    return(Retval);
}
   
/*
--------------------------------------------------------------------------------
  Function name : UINT32 DialogVolumeKey(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>       

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VOLUME_CODE_
uint32 DialogVolumeKey(void)
{
    UINT32 VolumeKeyVal;
    TASK_ARG TaskArg; 
    UINT8 MaxVolumeLimit = MAX_VOLUME;

    VolumeKeyVal =  GetKeyVal();

    if ((ShowEuropeVolumeLimitAlarm) &&
        (VolumeKeyVal != KEY_VAL_NONE) && (VolumeKeyVal != KEY_VAL_UP_PRESS_START) && (VolumeKeyVal != KEY_VAL_UP_SHORT_UP))
    {
        if (!EuropeVolumeLimitAlarmConfirmed)
        {
            EuropeVolumeLimitAlarmConfirmed = TRUE;
            VolumeSystickCounterBack = SysTickCounter;
            if (VolumeKeyVal == KEY_VAL_HOLD_ON)
            {
                TaskArg.Hold.HoldAction = HOLD_STATE_ON;
                //WinCreat(&VolumeWin, &HoldWin, &TaskArg);
            }
        }
        return;
    }

    if (gSysConfig.AvlsEnabled)
    {
        MaxVolumeLimit = MAX_VOLUME_AVLS_WORLDWIDE;
    }
    else
    {
        MaxVolumeLimit = MAX_VOLUME;
    }

    switch (VolumeKeyVal) 
    {
    case KEY_VAL_UP_DOWN:
    case KEY_VAL_UP_PRESS:
    //case KEY_VAL_UP_SHORT_UP:
        ShowAvlsIcon = FALSE;
        ShowAvlsWarning = FALSE;
		
        if (VolumeWinVol < MaxVolumeLimit)
        {
            VolumeWinVol++;     

#ifdef _MUSIC_
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                MusicDecodeProc(MSG_AUDIO_VOLUMESET,NULL);
            }
#endif
#ifdef _RADIO_
            if (TRUE == ThreadCheck(pMainThread, &FMThread))
            {
                FM_Process(MSG_RADIO_VOLUMESET,NULL);
            }
#endif
       		 
            if (ShowEuropeVolumeLimitAlarm)
            {
                #ifdef _BEEP_
                if (!gSysConfig.BeepEnabled) {
                    BeepPlay(BEEP_LOW_BATTERY, -1, BEEP_VOL_SET, 0, 0);
                }
                #endif
            }
        }
        else
        {
            if (gSysConfig.AvlsEnabled)
            {
                ShowAvlsIcon = TRUE;
            }
        }

        VolumeSystickCounterBack = SysTickCounter;
        SendMsg(MSG_VOLUME_DISPLAY_ALL);
        break;
        
    case KEY_VAL_DOWN_DOWN:
    case KEY_VAL_DOWN_PRESS:
    //case KEY_VAL_DOWN_SHORT_UP:
        ShowAvlsIcon = FALSE;
        ShowAvlsWarning = FALSE;

        if (VolumeWinVol)
        {
            VolumeWinVol--;
#ifdef _MUSIC_
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                MusicDecodeProc(MSG_AUDIO_VOLUMESET,NULL);
            }
#endif
#ifdef _RADIO_
            if (TRUE == ThreadCheck(pMainThread, &FMThread))
            {
                FM_Process(MSG_RADIO_VOLUMESET,NULL);
            }
#endif
#ifdef _VIDEO_
            if (TRUE == ThreadCheck(pMainThread, &VideoThread))
            {
                VideoControlProc(MSG_XVID_VOLUMESET,VolumeWinVol);
            }
#endif
        }
        VolumeSystickCounterBack = SysTickCounter;
        SendMsg(MSG_VOLUME_DISPLAY_ALL);
        break;

    default:        
        break;
    }
    
    return (0);
}    

/*
--------------------------------------------------------------------------------
  Function name : void DialogVolumeDisplay(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>       

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VOLUME_CODE_
void DialogVolumeDisplay(void)
{

    UINT16              color_tmp, TempCharSize;
    LCD_RECT            r;
    UINT16              pVolumeDisBuf[2];
    PICTURE_INFO_STRUCT picInfo;

    if(CheckMsg(MSG_NEED_PAINT_ALL) || GetMsg(MSG_VOLUME_DISPLAY_ALL))
    {
        LCD_ClrSrc();

        if (ShowEuropeVolumeLimitAlarm)
        {
            //DispPictureWithIDNum(IMG_ID_CAUTION);
            //GetPictureInfoWithIDNum(IMG_ID_CAUTION, &picInfo);

			DispPictureWithIDNum(IMG_ID_SETMENU_BACKGROUND1);
            DisplayMenuStrWithIDNum(picInfo.x + picInfo.xSize + 1, 4, 90, 12, LCD_TEXTALIGN_LEFT, SID_VOLUME_LIMIT_LINE1);
            DisplayMenuStrWithIDNum(picInfo.x + picInfo.xSize + 1, 20, 90, 12, LCD_TEXTALIGN_LEFT, SID_VOLUME_LIMIT_LINE2);
        }
        else if (ShowAvlsWarning)
        {
            DispPictureWithIDNumAndXY(IMG_ID_WARNING_BOX, 48, 2);
            DisplayMenuStrWithIDNum(0, 24, 128, 12, LCD_TEXTALIGN_CENTER, SID_AVLS);
        }
        else
        {
            color_tmp = LCD_GetColor();
            LCD_SetColor(COLOR_BLACK);
            TempCharSize = LCD_SetCharSize(FONT_12x12);

            DisplayMenuStrWithIDNum(0, 4, 128, 12, LCD_TEXTALIGN_CENTER, SID_VOLUME);
            //DispPictureWithIDNum(IMG_ID_VOLUME_ICON);

            if (ShowAvlsIcon)
            {
                {
                    //DispPictureWithIDNum(IMG_ID_VOLUME_LEVEL_AVLS_WW);
                    //GetPictureInfoWithIDNum(IMG_ID_VOLUME_LEVEL_AVLS_WW, &picInfo);
                    DispPictureWithIDNumAndXY(IMG_ID_MUSIC_SCROLLGROUND00 + VolumeWinVol,0,21);
                }
                //DispPictureWithIDNumAndXY(IMG_ID_AVLS_ICON, picInfo.x + picInfo.xSize + 1, 21);
                DispPictureWithIDNumAndXY(IMG_ID_VIDEO_VOL, picInfo.x + picInfo.xSize + 1, 21);
            }
            else
            {
                //DispPictureWithIDNum(IMG_ID_VOLUME_LEVEL_0 + VolumeWinVol);
				DispPictureWithIDNumAndXY(IMG_ID_MUSIC_SCROLLGROUND00 + VolumeWinVol,0,21);
				
            }

            r.x0 = 130;//100;
            r.y0 = 20;
            r.x1 = r.x0 + 16;
            r.y1 = r.y0 + 12;
            pVolumeDisBuf[0] = VolumeWinVol / 10;
            pVolumeDisBuf[0] += (pVolumeDisBuf[0] ? '0' : ' ' );
            pVolumeDisBuf[1] = (VolumeWinVol % 10) + '0';
            pVolumeDisBuf[2] = '\0';
            LCD_DispStringInRect(&r, &r, pVolumeDisBuf, LCD_TEXTALIGN_RIGHT);

            LCD_SetColor(color_tmp);
            LCD_SetCharSize(TempCharSize);
        }
    }

}

/*
********************************************************************************
*
*                         End of LowPower.c
*
********************************************************************************
*/

