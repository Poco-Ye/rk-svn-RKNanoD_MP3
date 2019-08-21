/*
********************************************************************************
*                   Copyright (c) 2012,SunChuanHu
*                         All rights reserved.
*
* File Name?¨ºo   MDBBuildWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               SunChuanHu      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "SysConfig.h"

#define _IN_MDBBUILDWIN_

#include "SysInclude.h"
#include "FsInclude.h"
#include "MDBBuildWin.h"
#include "MainMenu.h"
#include "MediaBroWin.h"
#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#include "SetBluetooth.h"
#endif

#ifdef PIC_MEDIA
/*sanshin 0612 --->*/
#include "BrowserUI.h"  /*<--sanshin 0612*/
/*<--sanshin 0612*/
#endif

_ATTR_MDBBUILDWIN_BSS_ UINT32 LastTaskId;
_ATTR_MDBBUILDWIN_BSS_ uint32 LastDisplayUpdateCounter;
_ATTR_MDBBUILDWIN_BSS_ UINT16 MedialibUpdateImageFrameIndex;

extern uint8  BtWinStatus;

#ifdef _MEDIA_MODULE_
#ifdef MEDIA_UPDATE

/*
--------------------------------------------------------------------------------
  Function name : void MedialibUpdataDisplay(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
void MedialibUpdataDisplay(UINT16 PicId)
{
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;
    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    DispPictureWithIDNumAndXYoffset(IMG_ID_MEIDASORTBAK0 + PicId, 0, 0);

    DisplayMenuStrWithIDNum(MEDIA_UPDATA_TXT_X, MEDIA_UPDATA_TXT_Y,
                            MEDIA_UPDATA_TXT_XSIZE, MEDIA_UPDATA_TXT_YSIZE,
                            LCD_TEXTALIGN_CENTER, SID_UPDATEING_MEDIALIB);

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);

#ifdef _FRAME_BUFFER_
    Lcd_BuferTranfer();
#endif
}


_ATTR_SYS_CODE_ //PAGE hj
void MedialibUpdateDisplayHook(void)
{
    MedialibUpdataDisplay(0);
}

/*
--------------------------------------------------------------------------------
  Function name : void SysCpuInit(void)
  Author        : ZHengYongzhi
  Description   : PLL setting??¨¦Disable int??¨¦PWM Disable,etc

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_MDBBUILDWIN_CODE_
void UpdateMediaLibray(void)
{
    MakeDir("\\", "DCIM");//sanshin
    MakeDir("\\", "PICTURE");//sanshin
    MakeDir("\\", "PICTURES");//sanshin

    LastDisplayUpdateCounter = 0;
    MedialibUpdateImageFrameIndex = 0;
    MedialibUpdataDisplay(0);
    FREQ_EnterModule(FREQ_MEDIAUPDATA);
#ifdef _SDCARD_

    if (MemorySelect != FLASH0)
    {
        MemorySelect = FLASH0;
        FileSysSetup(FLASH0);
    }

#endif
    DEBUG("UpdateMediaLibray enter");
    MedialibUpdataDisplay(0);
    ModuleOverlay(MODULE_ID_FILEINFO_SAVE, MODULE_OVERLAY_ALL);
    SearchAndSaveMusicInfo();
#ifdef PIC_MEDIA
    SearchAndSaveJpegInfo();    //sanshin
#endif
#ifdef _M3U_                                                                                                                                                //<----sanshin_20150616
    SearchAndSaveM3uInfo();     //<----sanshin_20150616
#endif
#ifdef _RECORD_
    SearchAndSaveRecordFmInfo();
#endif
    MedialibUpdataDisplay(1);
    ModuleOverlay(MODULE_ID_FILEINFO_SORT, MODULE_OVERLAY_ALL);
    SortUpdateFun(&(gSysConfig.MedialibPara), MediaInfoAddr);
    MedialibUpdataDisplay(2);
    ModuleOverlay(MODULE_ID_FAVO_RESET, MODULE_OVERLAY_ALL);
    FavoReset();
    DEBUG("MDBUpdateFun done");
    MedialibUpdataDisplay(3);
    //SaveSysInformation();
    FREQ_ExitModule(FREQ_MEDIAUPDATA);
    MedialibUpdataDisplay(3);
}
#endif
#endif

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
/*
_ATTR_MDBBUILDWIN_CODE_
void FWUpgradeDisplay(void)
{
    UINT16 TextMode ;

    TextMode = LCD_SetTextMode(LCD_DRAWMODE_NORMAL);
    LCD_ClrSrc();

    DisplayMenuStrWithIDNum(MEDIA_UPDATA_TXT_X, MEDIA_UPDATA_TXT_Y,
                            MEDIA_UPDATA_TXT_XSIZE, MEDIA_UPDATA_TXT_YSIZE,
                            LCD_TEXTALIGN_CENTER, SID_UPDATEING_FIRMWARE);
    Lcd_BuferTranfer(0,0,LCD_MAX_XSIZE,LCD_MAX_YSIZE, NULL);
    BL_On();
    LCD_SetTextMode(TextMode);
}
*/
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
_ATTR_MDBBUILDWIN_CODE_
void FWRecoverDisplay(void)
{
#if 0
    UINT16 TextMode ;
    TextMode = LCD_SetTextMode(LCD_DRAWMODE_NORMAL);
    LCD_ClrSrc();
    DisplayMenuStrWithIDNum(MEDIA_UPDATA_TXT_X, MEDIA_UPDATA_TXT_Y,
                            MEDIA_UPDATA_TXT_XSIZE, MEDIA_UPDATA_TXT_YSIZE,
                            LCD_TEXTALIGN_CENTER, SID_RECOVER_FIRMWARE);
    Lcd_BuferTranfer(0, 0, LCD_MAX_XSIZE, LCD_MAX_YSIZE, NULL);
    BL_On();
    LCD_SetTextMode(TextMode);
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : void MdbBuildWinInit(void)
  Author        : SunChuanHu
  Description   :MDB Build Win Initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
void MdbBuildWinInit(void * pArg)
{
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);
    LastTaskId = ((MDB_WIN_ARG *)pArg)->TaskID;

    if ( gSysConfig.MedialibPara.MediaUpdataFlag )
    {
        DEBUG(" ***Media Need Update*** ");
        DispPictureWithIDNum(IMG_ID_MDB_BACKGROUND);
        // display title
        DisplayMenuStrWithIDNum(MDB_TITLE_TXT_X, MDB_TITLE_TXT_Y, MDB_TITLE_TXT_XSIZE,
                                MDB_TITLE_TXT_YSIZE, LCD_TEXTALIGN_CENTER, SID_MEDIALIB );
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + gBattery.Batt_Level, 105, 146);
    }

#ifdef _FRAME_BUFFER_
    Lcd_BuferTranfer();
#endif
    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}


/*
--------------------------------------------------------------------------------
  Function name : void MainMenuIntDeInit(void)
  Author        : ZHengYongzhi
  Description   : ???2?|¨¬¡ê¡è?D???¡è??3?¡§o???£¤

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
void MdbBuildWinDeInit(void)
{
    //MainMenu?D???¡è??3?¡§o???£¤
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuService(void)
  Author        : ZHengYongzhi
  Description   : ???2?|¨¬¡ê¡è?¡èt??3¡§?D¡§¡ã,¡§??¡§?¡§2??|¡§¡è¡§a?????¡§2????¨¦??¨¦?????¡§2?¡èt??|¨¬¡§¡§¡§¡§???

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
UINT32 MdbBuildWinService(void)
{
    TASK_ARG TaskArg;
    uint32 LDOBack;

    FREQ_EnterModule(FREQ_MAX);

    if (GetMsg(MSG_SYS_FW_UPGRADE))
    {
        //FWUpgradeDisplay();

        //zyz:Backup SYSCONFIG
        BackupSysInformation();
        DelayMs(200);

        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
//        ModuleOverlay(MODULE_ID_FILE_ENCODE, MODULE_OVERLAY_ALL);
        ModuleOverlay(MODULE_ID_FW_UPGRADE, MODULE_OVERLAY_ALL);
        FwUpdate();

        #ifdef _FILE_DEBUG_
        SysDebugHookDeInit();
        #endif

        FREQ_ExitModule(FREQ_MAX);
        SysReboot(0x0000, 0);
    }
    if (GetMsg(MSG_SYS_FW_RECOVERY))
    {
        FWRecoverDisplay();
        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
//        ModuleOverlay(MODULE_ID_FILE_ENCODE, MODULE_OVERLAY_ALL);
        ModuleOverlay(MODULE_ID_FW_UPGRADE, MODULE_OVERLAY_ALL);
        FwUpgradeRecovery();

        #ifdef _FILE_DEBUG_
        SysDebugHookDeInit();
        #endif

        FREQ_ExitModule(FREQ_MAX);
        SysReboot(0x0000, 0);
    }

    if (gSysConfig.MedialibPara.MediaUpdataFlag)
    {
        gSysConfig.MedialibPara.MediaUpdataFlag   = 0;
#ifdef _MUSIC_
        gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;  //clear break point
#endif
#ifdef _RADIO_
        gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 0;
#endif

        /*before update meida libray,we must disconnect BT if BT is connect,
        becouse the code is overlay.
        */
        #ifdef _BLUETOOTH_
        BluetoothThreadDelete(gbBTConnected);
        #endif

#ifdef MEDIA_UPDATE
        UpdateMediaLibray();
#endif
        SendMsg(MSG_FLASH_MEM0_UPDATE);
    }

    if (CheckMsg(MSG_FLASH_MEM0_UPDATE)
        || CheckMsg(MSG_FLASH_MEM1_UPDATE)
        || CheckMsg(MSG_SDCARD_MEM_UPDATE))
    {
        ModuleOverlay(MODULE_ID_FS_MEM_GET, MODULE_OVERLAY_ALL);
    }

    if (GetMsg(MSG_FLASH_MEM0_UPDATE))
    {
        GetFreeMem(FLASH0, &sysTotalMemeryFlash0, &sysFreeMemeryFlash0);
    }

    if (GetMsg(MSG_FLASH_MEM1_UPDATE))
    {
        GetFreeMem(FLASH1, &sysTotalMemeryFlash1, &sysFreeMemeryFlash1);
    }

    if (GetMsg(MSG_SDCARD_MEM_UPDATE))
    {
        GetFreeMem(CARD, &sysTotalMemeryCard, &sysFreeMemeryCard);
    }

    FREQ_ExitModule(FREQ_MAX);

    if (LastTaskId == TASK_ID_MAINMENU)
    {
        TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
    }

#ifdef _MUSIC_
    else if (LastTaskId == TASK_ID_MEDIALIB)
    {
        TaskArg.Medialib.CurId = 0;

        if (gSysConfig.MusicConfig.HoldOnPlaySaveFlag == 1)
        {
            TaskArg.Medialib.CurId = MEDIA_MUSIC_BREAKPOINT;
        }
    }

#endif
#ifdef _RADIO_
    else if (LastTaskId == TASK_ID_RADIO)
    {
#ifdef _BEEP_

        if (!gSysConfig.BeepEnabled)
        {
            BeepPlay(BEEP_PLAY, 1, BEEP_VOL_SET, 0, 0 );
        }

#endif
        TaskArg.Radio.Freq = gSysConfig.RadioConfig.FmFreq;
        TaskArg.Radio.AutoPreset = 0;
        SendMsg(MSG_RADIOSUBWIN_HOLDONPLAYSAVE);
    }

#endif
#ifdef PIC_MEDIA                                            /*<--sanshin 0612*/
    /*sanshin 0612 --->*/                                       /*<--sanshin 0612*/
    else if (LastTaskId == TASK_ID_BROWSER)                 /*<--sanshin 0612*/
    {
        TaskArg.Browser.FileType = FileTypePicture;         /*<--sanshin 0612*/
        TaskArg.Browser.FileNum  = 0;                       /*<--sanshin 0612*/
        TaskArg.Browser.FromWhere = 2;                      /*<--sanshin 0612*/
    }                                                       /*<--sanshin 0612*/

    /*<--sanshin 0612*/                                         /*<--sanshin 0612*/
#endif                                                      /*<--sanshin 0612*/
#ifdef  _M3U_                                               //<-----sanshin_20150616//
//----->sanshin_20150616//                                  //<-----sanshin_20150616//
    else if (LastTaskId == TASK_ID_M3UBRO)                  //<-----sanshin_20150616//
    {
        if (gSysConfig.MedialibPara.gM3uFileNum)            //<-----sanshin_20150616//
        {
            //<-----sanshin_20150616//
            TaskArg.M3uBro.CurId = 0;                       //<-----sanshin_20150616////<-----sanshin_20150629//
        }                                                   //<-----sanshin_20150616//
        else                                                //<-----sanshin_20150616//
        {
            //<-----sanshin_20150616//
            LastTaskId = TASK_ID_MAINMENU;                  //<-----sanshin_20150616//
            TaskArg.MainMenu.MenuID = MAINMENU_ID_M3U;      //<-----sanshin_20150616////<-----sanshin_20150629//
        }                                                   //<-----sanshin_20150616//
    }                                                       //<-----sanshin_20150616//

//<-----sanshin_20150616//                                  //<-----sanshin_20150616//
#endif                                                      //<-----sanshin_20150616//
    TaskSwitch(LastTaskId, &TaskArg);
    return (RETURN_FAIL);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuKey(void)
  Author        : ZHengYongzhi
  Description   : ???2?|¨¬¡ê¡è?????¡§1??|¡§¡è¡§a3¡§?D¡§¡ã

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
UINT32 MdbBuildWinKey(void)
{
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuDisplay(void)
  Author        : ZHengYongzhi
  Description   : ???2?|¨¬¡ê¡è??¡§o???|¡§¡è¡§a3¡§?D¡§¡ã

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
void MdbBuildWinDisplay(void)
{
    DEBUG();
}

/*
--------------------------------------------------------------------------------
  Function name : void MedialibUpdateDisplayHook(void)
  Author        :
  Description   : Medialib update display hook called by external

  Input         :
  Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_MDBBUILDWIN_CODE_
//void MedialibUpdateDisplayHook(void)
//{
    //MedialibUpdataDisplay();
//}

