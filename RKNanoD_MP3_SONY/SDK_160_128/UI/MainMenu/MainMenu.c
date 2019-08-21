/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name¡êo  MainMenu.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_MAINMENU_

#include "SysInclude.h"
#include "FsInclude.h"
#include "MainMenu.h"
#include "MainMenuInterface.h"
#include "BrowserUI.h"
#include "Hold.h"
#include "AudioControl.h"
#include "MediaLibWin.h"

#ifdef _RECORD_
#include  "File.h"
#include  "FDT.h"

#include  "PCM.H"
#include  "pmu.h"

#include  "audio_main.h"

#include  "RecordControl.h"
#endif

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#include "SetBluetooth.h"
#endif

extern uint8  BtWinStatus;

extern THREAD MusicThread;
extern THREAD RecordThread;
extern THREAD FMThread;
_ATTR_SYS_DATA_  UINT16      MM_RecordQuality;   //quality of recording.
_ATTR_SYS_DATA_  UINT16      MM_RecordFmt;   //quality of recording.


_ATTR_SYS_DATA_ UINT8 MainIdFlag = 0;
/*
--------------------------------------------------------------------------------
  Function name : void MainMenuIntInit(void)
  Author        : ZHengYongzhi
  Description   : main menu interrupt initialization

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_INIT_CODE_
void MainMenuIntInit(void)
{
    //MainMenu  interrupt initialization
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuIntDeInit(void)
  Author        : ZHengYongzhi
  Description   : main menu interrupt auti-initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_DEINIT_CODE_
void MainMenuIntDeInit(void)
{
    //MainMenu
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuMsgInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_INIT_CODE_
void MainMenuMsgInit(void)
{
    SendMsg(MSG_MENU_DISPLAY_ALL);

}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuMsgDeInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message auti-initialization

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_DEINIT_CODE_
void MainMenuMsgDeInit(void)
{
    // ClearMsg(MSG_MENU_DISPLAY_BACK);
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuInit(void)
  Author        : ZHengYongzhi
  Description   : main menu initialization

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_CODE_
void MainMenuInit(void *pArg)
{
    DEBUG("MainMenu Enter");
    //For set backlight
    //BL_SetLevel(gSysConfig.BLevel);
    //Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_PWM0_OUT);
    //system content switch initial code

    MenuId = ((MAINMENU_WIN_ARG*)pArg)->MenuID;

#ifdef _BROWSER_
    if (MainIdFlag == MAINMENU_ID_BROWSER)
    {
        MenuId     = MAINMENU_ID_BROWSER;
        MainIdFlag = 0;
    }
#endif

    //service initial
    MainMenuMsgInit();      //message initial
    MainMenuIntInit();      //interrupt initial
    KeyReset();             //key initial

}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuDeInit(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_CODE_
void MainMenuDeInit(void)
{
    MainMenuIntDeInit();
    DEBUG("MainMenu Exit");
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuService(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_SERVICE_CODE_
UINT32 MainMenuService(void)
{
    return(RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuKey(void)
  Author        : ZHengYongzhi
  Description   : main menu key handle function.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_SERVICE_CODE_
UINT32 MainMenuKey(void)
{
    uint32 RetVal;
    UINT32 MainMenuKeyVal;
    WIN    *pWin;
    TASK_ARG TaskArg;

    RetVal = RETURN_OK;

    MainMenuKeyVal =  GetKeyVal();
    //7 key function
    switch (MainMenuKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP:
            RetVal = MainMenuModeKey();
            break;

        case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_FFD_PRESS:
        case KEY_VAL_UP_SHORT_UP:
        case KEY_VAL_UP_PRESS:
            if (++MenuId > (MAINMENU_ID_MAXNUM - 1))
            {
                MenuId = 0;
            }
            SendMsg(MSG_MENU_DISPLAY_ITEM);
            break;

        case KEY_VAL_FFW_SHORT_UP:
        case KEY_VAL_FFW_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
        case KEY_VAL_DOWN_PRESS:
            if (MenuId == 0)
            {
                MenuId = (MAINMENU_ID_MAXNUM - 1);
            }
            else
            {
                MenuId -- ;
            }
            SendMsg(MSG_MENU_DISPLAY_ITEM);
            break;

        case KEY_VAL_ESC_SHORT_UP:
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&MainMenuWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&MainMenuWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }
    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuDisplay(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_SERVICE_CODE_
void MainMenuDisplay(void)
{
    uint32 i;
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_MENU_DISPLAY_ALL)))
    {
        SendMsg(MSG_MENU_DISPLAY_ITEM);
        SendMsg(MSG_BATTERY_UPDATE);
    }

    if (TRUE == GetMsg(MSG_MENU_DISPLAY_ITEM))
    {

        for (i=0; i<(sizeof(MainConfig)/sizeof(Main_Config)); i++)
        {
            if (MainConfig[i].DispFlag && (MainConfig[i].TaskId == MenuId))
            {
                DispPictureWithIDNum(MainConfig[i].ImageId);
                DisplayMenuStrWithIDNum(0,128,127,144,LCD_TEXTALIGN_CENTER,MainConfig[i].TextId);
                break;
            }
        }
    }
    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        //DispPictureWithIDNum(IMG_ID_MAINMENU_BATTERYLEVEL0 + MainMenuBatteryLevel);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuModeKey(void)
  Author        : ZHengYongzhi
  Description   : main menu modules switch

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_SERVICE_CODE_
INT16 MainMenuModeKey(void)
{
    INT16  RetVal = 0;
    TASK_ARG TaskArg;

    switch (MenuId)
    {
#ifdef _MUSIC_
        case MAINMENU_ID_MUSIC:
#ifdef _MEDIA_MODULE_
            if (gSysConfig.MedialibPara.MediaUpdataFlag == 1)
            {
#ifdef _MUSIC_
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    ThreadDelete(&pMainThread, &MusicThread);
                }
                gSysConfig.MusicConfig.HoldOnPlaySaveFlag =0;
#endif

#ifdef _RADIO_
                if (TRUE == ThreadCheck(pMainThread, &FMThread))
                {
                    ThreadDelete(&pMainThread, &FMThread);
                }
#endif

                TaskArg.Mdb.TaskID = TASK_ID_MEDIALIB;
                TaskSwitch(TASK_ID_MDB, &TaskArg);
                RetVal = 1;
            }
            else
            {
                TaskArg.Medialib.CurId= 0;
                TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
                RetVal = 1;
            }
#else
            TaskArg.Browser.FileType = FileTypeAudio;
            TaskArg.Browser.FileNum   = 0;
            TaskArg.Browser.FromWhere = 0;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
#endif
            break;
#endif

#ifdef _RADIO_
        case MAINMENU_ID_RADIO:
            if (gSysConfig.FMEnable)
            {
                TaskSwitch(TASK_ID_RADIO, NULL);
                RetVal = 1;
            }
            break;
#endif

#ifdef _PICTURE_
        case MAINMENU_ID_PICTURE: //picture module
#ifdef PIC_MEDIA														/*<--sanshin 0612*/
/*sanshin 0612 --->*/                                                   /*<--sanshin 0612*/
			if (gSysConfig.MedialibPara.MediaUpdataFlag == 1)           /*<--sanshin 0612*/
    		{                                                           /*<--sanshin 0612*/
    			if (TRUE == ThreadCheck(pMainThread, &MusicThread))     /*<--sanshin 0612*/
    			{                                                       /*<--sanshin 0612*/
    				ThreadDelete(&pMainThread, &MusicThread);           /*<--sanshin 0612*/
    			}                                                       /*<--sanshin 0612*/
    			gSysConfig.MusicConfig.HoldOnPlaySaveFlag =0;           /*<--sanshin 0612*/
    			TaskArg.Mdb.TaskID = TASK_ID_BROWSER;                   /*<--sanshin 0612*/
    			                                                        /*<--sanshin 0612*/
    			TaskSwitch(TASK_ID_MDB, &TaskArg);                      /*<--sanshin 0612*/
    			BroswerFlag = FALSE;                                    /*<--sanshin 0612*/
    			RetVal = 1;                                             /*<--sanshin 0612*/
    		}                                                           /*<--sanshin 0612*/
            else /*if (gSysConfig.MedialibPara.gJpegFileNum)*/              /*<--sanshin 0612*/
            {                                                           /*<--sanshin 0612*/
                TaskArg.Browser.FileType = FileTypePicture;             /*<--sanshin 0612*/
                TaskArg.Browser.FileNum  = 0;                           /*<--sanshin 0612*/
                TaskArg.Browser.FromWhere = 2;                          /*<--sanshin 0612*/
                TaskSwitch(TASK_ID_BROWSER, &TaskArg);                  /*<--sanshin 0612*/
                BroswerFlag = FALSE;                                    /*<--sanshin 0612*/
                RetVal = 1;                                             /*<--sanshin 0612*/
            }															/*<--sanshin 0612*/
#else
            TaskArg.Browser.FileType = FileTypePicture;
            TaskArg.Browser.FileNum  = 0;
            TaskArg.Browser.FromWhere = 0;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
#endif
            break;
#endif

#ifdef _EBOOK_
    case MAINMENU_ID_EBOOK:
        TaskArg.Browser.FileType = FileTypeText;
        TaskArg.Browser.FileNum  = 0;
        TaskArg.Browser.FromWhere = 0;
        BroswerFlag = TRUE;
        TaskSwitch(TASK_ID_BROWSER, &TaskArg);
        RetVal = 1;
        break;
#endif

#ifdef _RECORD_
        case MAINMENU_ID_RECORD:
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }
#ifdef _RADIO_
            if (TRUE == ThreadCheck(pMainThread, &FMThread))
            {
                ThreadDelete(&pMainThread, &FMThread);
            }
#endif

#ifdef _BLUETOOTH_
            if(TRUE == ThreadCheck(pMainThread, &BlueToothThread))
            {
                DEBUG("### Bluetooth thread exit ###");
                if(gbBTConnected)   //bt connected
                {
                    //disconnect BT
                    bt_a2dp_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, NULL);
                    gbBTConnected = 0;

                    BtWinStatus = BT_WIN_STATUS_BT_GO_TO_OFF;
                }

                //delete bt thread
                ThreadDelete(&pMainThread, &BlueToothThread);

                DEBUG("### Bluetooth thread deleted...###");
                BtWinStatus = BT_WIN_STATUS_IDLE;

                SendMsg(MSG_BLUETOOTH_RECONNECT);
            }
#endif

            MM_RecordQuality = gSysConfig.RecordConfig.RecordQuality;
            MM_RecordFmt     = gSysConfig.RecordConfig.RecordFmt;
            {
                if (RECORD_FMT_MP3 == MM_RecordFmt)  //quality record.
                {
                    TaskArg.Record.RecordExitTaskID = TASK_ID_MAINMENU;
                    TaskArg.Record.RecordType = RECORD_TYPE_MIC_STERO;
                    TaskArg.Record.RecordCodecConfig = 0;
                    TaskArg.Record.RecordSampleRate = FM_ENC_MP3_FS;
                    TaskArg.Record.RecordChannel = RECORD_CHANNEL_STERO;
                    TaskArg.Record.RecordDataWidth = RECORD_DATAWIDTH_16BIT;
                    TaskArg.Record.RecordEncodeType = RECORD_ENCODE_TYPE_MP3;
                }
                else
                {
                    TaskArg.Record.RecordExitTaskID = TASK_ID_MAINMENU;
                    TaskArg.Record.RecordType = RECORD_TYPE_MIC_STERO;
                    TaskArg.Record.RecordCodecConfig = 0;
                    TaskArg.Record.RecordSampleRate = FM_ENC_WAV_FS;
                    TaskArg.Record.RecordChannel = RECORD_CHANNEL_STERO;
                    TaskArg.Record.RecordDataWidth = RECORD_DATAWIDTH_16BIT;
                    TaskArg.Record.RecordEncodeType = RECORD_ENCODE_TYPE_WAV;
                }
            }

            TaskSwitch(TASK_ID_RECORD, &TaskArg);
            RetVal = 1;
            break;
#endif

#ifdef _BROWSER_
        case MAINMENU_ID_BROWSER:
            MainIdFlag = MAINMENU_ID_BROWSER;
            TaskArg.Browser.FileType = FileTypeALL;
            TaskArg.Browser.FileNum  = 0;
            TaskArg.Browser.FromWhere = 0;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
            break;
#endif

#ifdef  _M3U_	/*sanshin*/
    case MAINMENU_ID_M3U:
    	if (gSysConfig.MedialibPara.MediaUpdataFlag == 1)           //<----sanshin_20150616
    	{                                                           //<----sanshin_20150616
    		if (TRUE == ThreadCheck(pMainThread, &MusicThread))     //<----sanshin_20150616
    		{                                                       //<----sanshin_20150616
    			ThreadDelete(&pMainThread, &MusicThread);           //<----sanshin_20150616
    		}                                                       //<----sanshin_20150616
    		//gSysConfig.MusicConfig.HoldOnPlaySaveFlag =0;         //<----sanshin_20150616
    		TaskArg.Mdb.TaskID = TASK_ID_M3UBRO;                    //<----sanshin_20150616
    		TaskSwitch(TASK_ID_MDB, &TaskArg);                      //<----sanshin_20150616
    	}                                                           //<----sanshin_20150616
    	else                                                        //<----sanshin_20150616
    	{                                                           //<----sanshin_20150616
    		TaskArg.M3uBro.CurId= 0;                                //<----sanshin_20150616
	    	TaskSwitch(TASK_ID_M3UBRO, &TaskArg);                   //<----sanshin_20150616
    	}                                                           //<----sanshin_20150616
    	RetVal = 1;                                                 //<----sanshin_20150616
	    break;                                                      //<----sanshin_20150616
#endif			/*sanshin*/

#ifdef  _SYSSET_
        case MAINMENU_ID_SETMENU:
            TaskSwitch(TASK_ID_SYSSET, NULL);
            RetVal = 1;
            break;
#endif

        default:
            break;
    }

    return (RetVal);
}
/*
********************************************************************************
*
*                         End of MainMenu.c
*
********************************************************************************
*/
