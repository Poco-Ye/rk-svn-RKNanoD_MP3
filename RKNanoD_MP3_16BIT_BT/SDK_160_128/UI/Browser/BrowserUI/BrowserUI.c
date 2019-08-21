/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  BrowserUI.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chenfen          2008-3-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_BROSWER_UI_

#include "SysInclude.h"

#ifdef  _BROWSER_

#include "FsInclude.h"

#include "BrowserUI.h"
#include "BroCore.h"

#include "DialogBox.h"
#include "MessageBox.h"
#include "Hold.h"

#ifdef _MUSIC_
#include "audio_globals.h"
#include "audio_file_access.h"
#include "Effect.h"
#include "AudioControl.h"
#endif

#include "SysFindFile.h"

#include "MainMenu.h"
#include "FmControl.h"

#include "MediaBroWin.h"

#ifdef _FRAME_BUFFER_
#include "LcdInterface.h"
#endif

#include "image_main.h"
#include "ImageControl.h"
#ifdef THUMB_DEC_INCLUDE
#include "thumbnail_parse.h"
#endif

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#endif

int16 BroGetFileNumInFloder(BrowserFileStruct *pBro,UINT16 Flag);

/*
--------------------------------------------------------------------------------
  Function name : void BrowserWinInit(void)
  Author        : chenfen
  Description   : browser window initialization func

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef _FRAME_BUFFER_
_ATTR_BRO_UI_DATA_ static int gBrowserData_CurStartIndex = 0;
#endif

#ifdef THUMB_DEC_INCLUDE
_ATTR_BRO_UI_BSS_ static BOOL gIsImproveFreq;
_ATTR_BRO_UI_BSS_ static UINT32 gTickCounter;
#endif

_ATTR_BRO_UI_CODE_
void BrowserWinInit(void *pArg)
{
#ifdef THUMB_DEC_INCLUDE
    ModuleOverlay(MODULE_ID_PICTURE_CONTROL, MODULE_OVERLAY_ALL);
    gIsImproveFreq = FALSE;
    gTickCounter = SysTickCounter;
#endif
    gIsMusicWinFlg = FALSE;     //<----sanshin_20151026

    gBrowserFindFileType = ((BROWSER_WIN_ARG*)pArg)->FileType;           //task parameter transfer
    gBrowserFildFileNum  = ((BROWSER_WIN_ARG*)pArg)->FileNum;            //file index

    if ((gBrowserFindFileType == FileTypeALL))//to enter main interface directly.
    {
        SendMsg(MSG_BROW_FROM_MAINMENU);
    }
    else
    {
        ClearMsg(MSG_BROW_FROM_MAINMENU);
    }

#ifdef PIC_MEDIA
/*sanshin 0612 --->*/
    if(1 == (((BROWSER_WIN_ARG*)pArg)->FromWhere))              /*<--sanshin 0612*/
    {                                                           /*<--sanshin 0612*/
        gBrowSerFsType = MUSIC_DB;                              /*<--sanshin 0612*/
    }                                                           /*<--sanshin 0612*/
    else if(2 == (((BROWSER_WIN_ARG*)pArg)->FromWhere)){        /*<--sanshin 0612*/
        gBrowSerFsType = JPEG_DB;                               /*<--sanshin 0612*/
    }                                                           /*<--sanshin 0612*/
/*<--sanshin 0612*/
#else
    if (((BROWSER_WIN_ARG*)pArg)->FromWhere)
    {
        gBrowSerFsType = MUSIC_DB;
    }
#endif                                                          /*<--sanshin 0612*/
    else
    {
        gBrowSerFsType = FS_FAT;
    }

    //BrowserValueInit(1);/*<--sanshin_20150702*/

    /*-->sanshin_20150702*/
    if (gBrowserFildFileNum) //locate file basic index is 1
    {
        BrowserValueInit(1);
        BroGotoCurFile(gBrowserFildFileNum);//file that focus index is gBrowserFildFileNum.
    }
    else
    {
        ClearMsg(MSG_BROW_TASK_HAS_SWITCH);
        BrowserValueInit(1);
    }
    /*<--sanshin_20150702*/

    FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep],gBrowSerFsType);
    FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile, &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep],gBrowSerFsType);

    if (gBrowserFindFileType == FileTypeALL) //audio file,disk selet don't switch
    {
        BrowserMemSel         = FLASH0;//MemorySelect;
    }
    else
    {
        BrowserMemSel         = MemorySelect;
    }
    BrowserMemSelEn       = 0;
    BrowserData.TotleDisk = 1;

#if(defined (_SDCARD_) || defined (_MULT_DISK_))
    if (BroswerFlag == TRUE)//it do not enter browser from media.
    {
        if (gBrowserFildFileNum == 0)
        {
            BrowserMemSelEn = 1;
            BroLoadMemSelStr();
        }
    }
#endif

    if ((BrowserMemSelEn!=1)&&(BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems==0))
    {
        SendMsg(MSG_BROW_CHECK_FILE);
    }

    SendMsg(MSG_BROW_DIS_ALL);
    SendMsg(MSG_BROW_DIS_SCLLO_ITEM);//send scroll message

    KeyReset();             //key initial
}

/*
--------------------------------------------------------------------------------
  Function name : void BrowserWinInit(void)
  Author        : chenfen
  Description   : browser window auti-initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserWinDeInit(void)
{
#ifdef THUMB_DEC_INCLUDE
    if(gIsImproveFreq)
    {
        gIsImproveFreq = FALSE;
        FREQ_ExitModule(FREQ_JPG);
    }
#endif

    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BrowserWinService(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
uint32 BrowserWinService(void)
{
    UINT32   i;
    UINT32   Retval = 0;
    TASK_ARG TaskArg;

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))//message bos destory message.
    {
        BroKeyExitPro();
        switch (gBrowserFindFileType)
        {
            case FileTypeText:
#ifdef _EBOOK_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_EBOOK;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            case FileTypeVideo:
#ifdef _VIDEO_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            case FileTypePicture:
#ifdef _PICTURE_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            case FileTypeALL:
#ifdef _BROWSER_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_BROWSER;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            default:
                break;

        }
    }

    if (GetMsg(MSG_BROW_CHECK_FILE))
    {
        if (BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems==0)
        {
#if 0
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 3;
            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
#else
            UINT16 TextMode;
            TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
            ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
            DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);  //没有相应格式的文件, 也没有目录, 只显示背景图

            DisplayMenuStrWithIDNum(BROWSER_TITLE_TXT_X, BROWSER_TITLE_TXT_Y,
                                    BROWSER_TITLE_TXT_XSIZE,BROWSER_TITLE_TXT_YSIZE,
                                    LCD_TEXTALIGN_CENTER, BrowserData.BrowserTitleId);
            LCD_SetTextMode(TextMode);
#endif
        }
    }

#if 0
    if (GetMsg(MSG_BROW_DELETEING_FILE))                        //executive file be deleted
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            ThreadDelete(&pMainThread,&MusicThread);
        }

        BroDeleteFile();

        WinDestroy(&MessageBoxWin);

#ifdef _MEDIA_MODULE_
        gSysConfig.MedialibPara.MediaUpdataFlag = 1;//it should notice media update when file be deleted
#endif
        SendMsg(MSG_BROW_DIS_ALL);
        Retval = 0;
    }
    if (CheckMsg(MSG_BROW_DELETE_FILE)
            &&((CheckMsg(MSG_DIALOG_KEY_OK)||CheckMsg(MSG_DIALOG_KEY_CANCEL))))
    {
        if (GetMsg(MSG_DIALOG_KEY_OK))
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_DELETING_FILE;
            TaskArg.Message.HoldTime  = 3;
            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
            SendMsg(MSG_BROW_DELETEING_FILE);
            ClearMsg(MSG_BROW_DELETE_FILE);                     //he clear message in here should not turn two to one with below.

        }
        if (GetMsg(MSG_DIALOG_KEY_CANCEL))                      //to refresh screen when dialog return cancel message
        {
            SendMsg(MSG_BROW_DIS_ALL);
            ClearMsg(MSG_BROW_DELETE_FILE);
        }
    }
#endif

    if (GetMsg(MSG_SDCARD_UPDATE))
    {
#ifdef _SDCARD_
        BroSDCardCheck();
#endif
    }

    return (Retval);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BrowserWinKey(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
UINT32 BrowserWinKey(void)
{
    INT32 KeyVal;
    INT16 RetVal = MSG_MODULE_OK;
    BrowserFileStruct *pBro;

    TASK_ARG TaskArg;

    KeyVal = GetKeyVal();

    //5 Key function modify
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFD_DOWN:
                KeyVal = KEY_VAL_DOWN_DOWN;
                break;

            case KEY_VAL_FFD_PRESS:
                KeyVal = KEY_VAL_DOWN_PRESS;
                break;

            case KEY_VAL_FFW_DOWN:
                KeyVal = KEY_VAL_UP_DOWN;
                break;

            case KEY_VAL_FFW_PRESS:
                KeyVal = KEY_VAL_UP_PRESS;
                break;

            default:
                break;
        }
    }

    //6 Key modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFW_SHORT_UP:
                KeyVal = KEY_VAL_ESC_SHORT_UP;
                break;

            default:
                break;
        }
    }

    //7 Key function
    switch (KeyVal)
    {
        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
                BroMemSelKeyDown();
            }
            else
#endif
            {
                BroKeyDownProc();
            }
            break;

        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
#if(defined (_SDCARD_)|| defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
                BroMemSelKeyUp();
            }
            else
#endif
            {
                BroKeyUpProc();
            }
            break;

        case KEY_VAL_PLAY_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (!BrowserMemSelEn)
#endif
            {
                //BroDeleteFilePro();
            }
            break;

        case KEY_VAL_MENU_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
                BroMemSelKeyMenu();
            }
            else
#endif
            {
                RetVal = BroKeyEnterPro();
            }
            break;

        case KEY_VAL_ESC_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
                RetVal = BroMemSelKeyExit();
            }
            else
#endif
            {
                RetVal = BroKeyExitPro();
            }
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&BrowserWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&BrowserWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : BroUIDisplay
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserWinPaint(void)
{
    UINT16  i,CurNum;
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode, TextMode;
    UINT16  TotalItem;
    UINT16  *printfbuf;
    UINT32   x,y;
    LCD_RECT             r;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;

    BrowserFileStruct   *pBro = BrowserData.pBrowserFile;

#ifdef _FRAME_BUFFER_
    int CurStartIndex = BrowserData.CurStartIndex[BrowserData.CurDeep];
#endif

#ifdef THUMB_DEC_INCLUDE
    FILE* FileHandle;
    FileType    CurFileType;

    ImageMaxHeight = 12;
    ImageMaxWidth = 12;
    IsDisplayBackground(0);
#endif

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_BROW_DIS_ALL)))
    {
        //display backgroud main window.
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        DisplayMenuStrWithIDNum(BROWSER_TITLE_TXT_X, BROWSER_TITLE_TXT_Y,
                                BROWSER_TITLE_TXT_XSIZE,BROWSER_TITLE_TXT_YSIZE,
                                LCD_TEXTALIGN_CENTER, BrowserData.BrowserTitleId);

        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+BrowserBatteryLevel,105,146);
        //send other display message.
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+BrowserBatteryLevel,105,146);
    }

    if (GetMsg(MSG_BROW_DIS_SELE_ITEM))
    {
        if (0/*page to next or prevous*/)
        {
            SendMsg(MSG_BROW_DIS_ALL_ITEM);
        }
        else
        {
            ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
            GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
            GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);

            DisplayMenuStrWithIDNum(BROWSER_TITLE_TXT_X, BROWSER_TITLE_TXT_Y,
                                    BROWSER_TITLE_TXT_XSIZE,BROWSER_TITLE_TXT_YSIZE,
                                    LCD_TEXTALIGN_CENTER, BrowserData.BrowserTitleId);

            //DEBUG("##!!!  CurStartIndex = %d",CurStartIndex);
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
#ifdef _FRAME_BUFFER_
                CurStartIndex = 0;
#endif
            }
#endif

#ifdef _FRAME_BUFFER_
            for (i = CurStartIndex; i < BROWSER_SCREEN_PER_LINE + CurStartIndex; i++)
#else
            for (i = 0; i < BROWSER_SCREEN_PER_LINE; i++)
#endif
            {
                if ((i == BrowserData.PrePointer) || (i == BrowserData.CurPointer))
                {
#ifdef _FRAME_BUFFER_
                    DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+(i - CurStartIndex)*17,17);
#else
                    if (i == BrowserData.PrePointer)
                    {
                        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
                    }
#endif

                    r.x0 = 20;

#ifdef _FRAME_BUFFER_
                    r.y0 = (i - CurStartIndex)*17+3;
#else
                    r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
#endif
                    r.x1 = r.x0 + PicInfo.xSize;
                    r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;

                    if (i == BrowserData.CurPointer)
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                    }

#ifdef _FRAME_BUFFER_
#ifdef THUMB_DEC_INCLUDE
                    CurFileType = pBro->FileType;
                    if (/*CurFileType == FileTypeAudio || */CurFileType == FileTypePicture) /*<--sanshin_20150702*/
                    {
                        gTickCounter = SysTickCounter;
                        if(!gIsImproveFreq)
                        {
                            gIsImproveFreq = TRUE;
                            FREQ_EnterModule(FREQ_JPG); //避免缩略图解码反复提频、降频
                        }
                        FileHandle = (FILE*)FileOpen(NULL, pBro->FileLocInfo.Clus, pBro->FileLocInfo.Index, gBrowSerFsType, FileOpenStringR);
                        if ((int)FileHandle == NOT_OPEN_FILE)
                        {
                            DEBUG("=====FileOpen error=====");
                            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
                        }
                        else
                        {
                            ImageLeft = PicInfo1.x;
                            ImageTop = 5 + 17 * (i - CurStartIndex);

                            if (CurFileType == FileTypePicture)
                            {
                                SetPicFileType(IMAGE_PIC);
                            }

                            if (!ThumbParse(FileHandle))
                            {
                                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
                            }

                            FileClose((HANDLE)FileHandle);
                        }
                    }
                    else
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
                    }
#else
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
#endif
#else
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);
#endif

                    if (i == BrowserData.CurPointer)
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                        if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                        {
                            PrintfBuf = pBro->LongFileName;
                            BrowserWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 3);
                            SendMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                        }
                        else
                        {
                            ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                        }
                    }

                    r.y0 += 2;
                    r.y1 =  r.y0+CH_CHAR_XSIZE_12-1;
                    TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
                    LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
                    LCD_SetTextMode(TextMode);

#if 0
                    if (i == BrowserData.CurPointer)//an 4.21
                    {
                        LCD_SetTextMode(TextMode);
                    }
#endif
                }
                pBro = pBro->pNext;
            }
        }
        SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
    }

    if (GetMsg(MSG_BROW_DIS_ALL_ITEM))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3,143);
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);

        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);

#ifdef _FRAME_BUFFER_
        TotalItem = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems;
#else
        TotalItem = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems - BrowserData.CurStartIndex[BrowserData.CurDeep];
#endif

#if(defined (_SDCARD_) || defined (_MULT_DISK_))
        if (BrowserMemSelEn)
        {
            TotalItem = BrowserData.TotleDisk;

#ifdef _FRAME_BUFFER_
            CurStartIndex = 0;//BrowserData.CurPointer;
#endif

            DEBUG(" TotalItem = %d CurPoint = %d",TotalItem,BrowserData.CurPointer);
        }
#endif

#ifdef _FRAME_BUFFER_
        for (i = CurStartIndex; ((i < BROWSER_SCREEN_PER_LINE + CurStartIndex) && (i < TotalItem )); i++)
#else
        for (i = 0; ((i < BROWSER_SCREEN_PER_LINE) && (i < TotalItem )); i++)
#endif

        {
            //draw file type icon. add 2 in here is because the basic pictrue is smaller 4 than file icon in y coordinate,so get middle value.
#ifdef _FRAME_BUFFER_
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+(i - CurStartIndex)*17,17);

#ifdef THUMB_DEC_INCLUDE
            CurFileType = pBro->FileType;
            if (/*CurFileType == FileTypeAudio || */CurFileType == FileTypePicture)     /*<--sanshin_20150702*/
            {
                gTickCounter = SysTickCounter;
                if(!gIsImproveFreq)
                {
                    gIsImproveFreq = TRUE;
                    FREQ_EnterModule(FREQ_JPG); //避免缩略图解码反复提频、降频
                }

                FileHandle = (FILE*)FileOpen(NULL, pBro->FileLocInfo.Clus, pBro->FileLocInfo.Index, gBrowSerFsType, FileOpenStringR);
                if ((int)FileHandle == NOT_OPEN_FILE)
                {
                    DEBUG("=====FileOpen error=====");
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
                }
                else
                {
                    ImageLeft = PicInfo1.x;
                    ImageTop = 5 + 17 * (i - CurStartIndex);

                    if (CurFileType == FileTypePicture)
                    {
                        SetPicFileType(IMAGE_PIC);
                    }

                    if (!ThumbParse(FileHandle))
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
                    }

                    FileClose((HANDLE)FileHandle);
                }
            }
            else
            {
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
            }
#else
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * (i - CurStartIndex));
#endif

            r.x0 = 20;
            r.y0 = (i - CurStartIndex)*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;
#else
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);

            r.x0 = 20;
            r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;
#endif

            if (i == BrowserData.CurPointer)
            {
                DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                {
                    PrintfBuf = pBro->LongFileName;
                    BrowserWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 3);

                    SendMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                }
                else
                {
                    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                }
            }

            r.y0 += 2;
            r.y1 =  r.y0+CH_CHAR_XSIZE_12-1;

            TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
            LCD_SetTextMode(TextMode);

            pBro = pBro->pNext;
        }
        SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
    }

    if (GetMsg(MSG_BROW_DIS_SCLLO_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL_BLOCK, &PicInfo1);

#ifdef _FRAME_BUFFER_
        CurNum = BrowserData.CurPointer;
#else
        CurNum = BrowserData.CurStartIndex[BrowserData.CurDeep]+BrowserData.CurPointer;
#endif
        if (CurNum > 0)
            y = (PicInfo.ySize - PicInfo1.ySize) * CurNum  /( BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems -1);//+picInfo1.ySize/2;
        else
            y = 0;
        if ((y+PicInfo1.ySize) >PicInfo.ySize)
            y = PicInfo.ySize - PicInfo1.ySize;

        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_SCOLL,122,0);
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_SCOLL_BLOCK,0+123, y);//scroll smal icon
    }

    //if(CheckMsg(MSG_BROW_DELETE_FILE))//not scroll display when delete file
    //{
    //    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    //}

    if (CheckMsg(BROWSER_DISPFLAG_SCROLL_FILENAME))//an 4.21
    {
        TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(PrintfBuf);
        LCD_SetTextMode(TextMode);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);

#ifdef THUMB_DEC_INCLUDE
    if(SysTickCounter - gTickCounter > 500) //超过5s没有缩略图解码操作，则将频
    {
        if(gIsImproveFreq)
        {
            gIsImproveFreq = FALSE;
            FREQ_ExitModule(FREQ_JPG);
        }
    }
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : BrowserValueInit
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;
    LCD_RECT            pRect1;

    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize   = 16;

    pRect1.x0 = pRect->x0;
    pRect1.y0 = pRect->y0+2;
    pRect1.x1 = pRect->x1;
    pRect1.y1 = pRect->y0+CH_CHAR_XSIZE_12-1;;


    PicPartInfo.pictureIDNump = ImageID;
    SetScrollStringInfo(&pRect1, PicPartInfo, pstr, Speed);
}

/*
--------------------------------------------------------------------------------
  Function name : BrowserValueInit
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserValueInit(UINT16 NeedInitAll)
{
    uint32 i;

    if (NeedInitAll)
    {
#ifdef _FRAME_BUFFER_
        if (!CheckMsg(MSG_BROW_TASK_HAS_SWITCH))
#endif
        {
            memset((UINT8*)&BrowserData,0,sizeof(BrowserData));
        }
        BrowserData.pSearchFileInfo = GetSearchFileInfo(gBrowserFindFileType);
        switch (gBrowserFindFileType)
        {
            case FileTypeAudio:
                BrowserData.BrowserTitleId = SID_MUSIC_FILE;
                break;

            case FileTypeText:
                BrowserData.BrowserTitleId = SID_TEXT;
                break;

            case FileTypeVideo:
                BrowserData.BrowserTitleId = SID_VIDEO_FILE;
                break;

            case FileTypePicture:
                BrowserData.BrowserTitleId = SID_PICTURE_FILE;
                break;

            default:
                BrowserData.BrowserTitleId = SID_EXPLORER;
                break;
        }

        BrowserData.FolderInfo[BrowserData.CurDeep].DirClus = GetRootDirClus(gBrowSerFsType);
    }

    BrowserData.PrePointer = 0;
    BrowserData.CurPointer = 0;

    memset(BrowserFileItem,0,sizeof(BrowserFileItem));

    for (i = 0; i < BROWSER_SCREEN_PER_LINE - 1; i++)
    {
        BrowserFileItem[i].pNext = &BrowserFileItem[i + 1];
    }
    for (i = 1; i < BROWSER_SCREEN_PER_LINE; i++)
    {
        BrowserFileItem[i].pPrev = &BrowserFileItem[i - 1];
    }

    BrowserFileItem[0].pPrev = &BrowserFileItem[BROWSER_SCREEN_PER_LINE-1];
    BrowserFileItem[BROWSER_SCREEN_PER_LINE-1].pNext = &BrowserFileItem[0];
    BrowserData.pBrowserFile     = &BrowserFileItem[0];
}

/*
--------------------------------------------------------------------------------
  Function name : void BroGotoCurFile(UINT16 GlobeFileNum)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroGotoCurFile(UINT16 GlobeFileNum)
{
    UINT16 CurFileNum;
    UINT16 i;
    BroFileInfo *pSearchFileInfo;
    FIND_DATA FindData;

    pSearchFileInfo = GetSearchFileInfo(gBrowserFindFileType);
#ifdef PIC_MEDIA
/*sanshin 0612 --->*/
    if(                                         /*<--sanshin 0612*/
        (gBrowSerFsType == MUSIC_DB)            /*<--sanshin 0612*/
        ||                                      /*<--sanshin 0612*/
        (gBrowSerFsType == JPEG_DB)             /*<--sanshin 0612*/
    )                                           /*<--sanshin 0612*/
/*<--sanshin 0612*/
#else
    if (gBrowSerFsType == MUSIC_DB)
#endif
    {
        gwSaveDirClus = 0;
    }
    else
    {
        gwSaveDirClus = BootSector.BPB_RootClus;
    }

    CurFileNum      = GetCurFileNum(GlobeFileNum, &FindData, pSearchFileInfo->FileExtName, gBrowSerFsType);

    BrowserData.CurDeep = CurDirDeep;
    for (i = 0; i <= CurDirDeep; i++)
    {
        BrowserData.FolderInfo[i].DirClus = SubDirInfo[i].DirClus;
#ifdef _FRAME_BUFFER_
        if (!CheckMsg(MSG_BROW_TASK_HAS_SWITCH))    /*<--sanshin_20150702*/
#endif
        {
            BrowserData.CurPointerBack[i] = (SubDirInfo[i].CurDirNum) % BROWSER_SCREEN_PER_LINE;

            if (gBrowSerFsType == FS_FAT && BrowserData.CurPointerBack[i] > 0)
            {
                BrowserData.CurPointerBack[i]--;
            }
            BrowserData.CurStartIndex[i] = ((SubDirInfo[i].CurDirNum)  / BROWSER_SCREEN_PER_LINE) * BROWSER_SCREEN_PER_LINE;
        }
    }
#ifdef _FRAME_BUFFER_
    ClearMsg(MSG_BROW_TASK_HAS_SWITCH);             /*<--sanshin_20150702*/
#endif

    FolderSearchFile(pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep], gBrowSerFsType);

    if (CurFileNum>BrowserData.FolderInfo[CurDirDeep].TotalFiles)
    {
        CurFileNum = BrowserData.FolderInfo[CurDirDeep].TotalFiles;
    }

    //if( gSysConfig.MusicConfig.RepeatModeBak <= AUDIO_REPEAT)
    {
        CurFileNum += BrowserData.FolderInfo[CurDirDeep].TotalFolders;//add direction to locate file
    }

#ifdef _FRAME_BUFFER_
    BrowserData.CurPointer = CurFileNum - 1;

    if (gBrowserData_CurStartIndex > BrowserData.CurPointer)
        BrowserData.CurStartIndex[BrowserData.CurDeep] = BrowserData.CurPointer;
    else if ((BrowserData.CurPointer + 1 - BROWSER_SCREEN_PER_LINE) > gBrowserData_CurStartIndex)
        BrowserData.CurStartIndex[BrowserData.CurDeep] = BrowserData.CurPointer + 1 - BROWSER_SCREEN_PER_LINE;
    else
        BrowserData.CurStartIndex[BrowserData.CurDeep] = gBrowserData_CurStartIndex;
#else
    BrowserData.CurStartIndex[BrowserData.CurDeep] = ((CurFileNum-1)/BROWSER_SCREEN_PER_LINE)*BROWSER_SCREEN_PER_LINE;
    BrowserData.CurPointer = CurFileNum - BrowserData.CurStartIndex[BrowserData.CurDeep] - 1;
#endif

    BrowserData.PrePointer = BrowserData.CurPointer;

    FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile, &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);
}

/*
--------------------------------------------------------------------------------
  Function name : FolderSearchFile
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
uint16 FolderSearchFile(uint8 * ExtName, FolderInfoStruct *pFolderInfo, FS_TYPE FsType)
{
    uint16 ret = RETURN_OK;

    //DEBUG("DirClus = %d, FsType = %d ", pFolderInfo->DirClus, FsType);

    pFolderInfo->TotalFiles = GetTotalFiles(pFolderInfo->DirClus, ExtName, FsType);
    pFolderInfo->TotalFolders = GetTotalSubDir(pFolderInfo->DirClus, FsType);

    //RK Aaron.sun
#if 1
#ifdef PIC_MEDIA
/*sanshin 0612 --->*/
    if (                        /*<--sanshin 0612*/
        (FsType == MUSIC_DB)    /*<--sanshin 0612*/
        ||                      /*<--sanshin 0612*/
        (FsType == JPEG_DB)     /*<--sanshin 0612*/
        //((FsType == JPEG_DB) && (BrowserData.CurDeep == 0))   /*<--sanshin 0612*//*<--sanshin_20150702*/
    )                           /*<--sanshin 0612*/
/*<--sanshin 0612*/
#else
    if (FsType == MUSIC_DB)
#endif
    {
        if (pFolderInfo->TotalFolders >= 1)
        {
            pFolderInfo->TotalFolders++;
        }
    }
#endif

    pFolderInfo->TotalItems = pFolderInfo->TotalFiles + pFolderInfo->TotalFolders;
    //DEBUG("total file =%d, total dir = %d\n", pFolderInfo->TotalFiles, pFolderInfo->TotalFolders);
    if (FsType == MUSIC_DB){
        if(8192 < pFolderInfo->TotalItems){
            pFolderInfo->TotalItems = 8192;
        }
    }
    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : FolderGetItemInfo
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void FolderGetItemInfo(uint8 * ExtName, BrowserFileStruct*pBrowserFile,
                       FolderInfoStruct *pFolderInfo , UINT16 StartItem, FS_TYPE FsType)
{
    uint32 index;
    uint16 ret = RETURN_OK;
    FDT    TempFDT;
    FIND_DATA FindData;

    int16  NeedLoadFileNum;
    int16  NeedLoadFolderNum;
    int16  SkipLoadFileNum;
    int16  SkipLoadFolderNum;

    BrowserFileStruct* pBroFolder;
    BrowserFileStruct* pBroFile;

    if (pFolderInfo->TotalFolders > StartItem) //StartItem base 0
    {
        NeedLoadFolderNum = pFolderInfo->TotalFolders - StartItem;
        if (NeedLoadFolderNum >= BROWSER_SCREEN_PER_LINE)
        {
            NeedLoadFileNum = 0;
            NeedLoadFolderNum = BROWSER_SCREEN_PER_LINE;
        }
        else
        {
            NeedLoadFileNum = BROWSER_SCREEN_PER_LINE - NeedLoadFolderNum;
        }
        SkipLoadFileNum = 0;
        SkipLoadFolderNum = StartItem;
    }
    else
    {
        SkipLoadFolderNum = pFolderInfo->TotalFolders;
        NeedLoadFolderNum = 0;
        NeedLoadFileNum = pFolderInfo->TotalItems - StartItem;
        if (NeedLoadFileNum >= BROWSER_SCREEN_PER_LINE)
        {
            NeedLoadFileNum = BROWSER_SCREEN_PER_LINE;
        }
        SkipLoadFileNum = StartItem - pFolderInfo->TotalFolders;
    }

    pBroFolder = pBrowserFile;
    pBroFile = pBrowserFile + NeedLoadFolderNum;

    FindData.Clus = pFolderInfo->DirClus;
    FindData.Index = 0;
    FindData.TotalItem = pFolderInfo->TotalItems;


    if (FindFirst(&TempFDT, &FindData, ExtName,FsType) != RETURN_OK)
    {
        return;
    }

    //Rk Aaron.sun
#if 1
#ifdef PIC_MEDIA
/*sanshin 0612 --->*/
    if (                        /*<--sanshin 0612*/
        (FsType == MUSIC_DB)    /*<--sanshin 0612*/
        ||                      /*<--sanshin 0612*/
        (FsType == JPEG_DB)     /*<--sanshin 0612*/
        //((FsType == JPEG_DB) && (BrowserData.CurDeep == 0))       /*<--sanshin 0612*//*<--sanshin_20150702*/
    )                           /*<--sanshin 0612*/
#else
/*<--sanshin 0612*/
    if (FsType == MUSIC_DB)
#endif
    {
        if (NeedLoadFolderNum)
        {
            if (SkipLoadFolderNum == 0)
            {
                pBroFolder->FileType = FileTypeFolder;
                memcpy(pBroFolder->LongFileName, L"All files\0", 20);
                pBroFolder->FileLocInfo.Clus = FindData.Clus;
                pBroFolder->FileLocInfo.Index = FindData.Index - 1;
                pBroFolder = pBroFolder->pNext;
                NeedLoadFolderNum--;
            }
            else
            {
                SkipLoadFolderNum--;
            }
        }
    }
#endif


    do
    {
        if (TempFDT.Attr & ATTR_DIRECTORY)
        {
            if (NeedLoadFolderNum)
            {
                if (SkipLoadFolderNum==0)
                {
                    //get direction
                    pBroFolder->FileFDT =  TempFDT;
                    pBroFolder->FileType = FileTypeFolder;
                    pBroFolder->FileLocInfo.Clus = FindData.Clus;
                    pBroFolder->FileLocInfo.Index = FindData.Index - 1;

                    GetLongFileName(FindData.Clus, FindData.Index - 1, FsType, pBroFolder->LongFileName);
                    pBroFolder = pBroFolder->pNext;
                    NeedLoadFolderNum--;
                }
                else
                {
                    SkipLoadFolderNum--;
                }
            }
        }
        else if (NeedLoadFileNum)
        {
            if (SkipLoadFileNum == 0)
            {
                BroFileInfo * TempFileInfo = BroGetFileInfo(TempFDT.Name);
                //file get
                pBroFile->FileFDT =  TempFDT;
                if (FsType == MUSIC_DB)
                {
                    pBroFile->FileType = FileTypeAudio;
                }
#ifdef PIC_MEDIA                                            /*<--sanshin 0612*/
                /*sanshin 0612 -->*/                        /*<--sanshin 0612*/
                else if (FsType == JPEG_DB)                 /*<--sanshin 0612*/
                {                                           /*<--sanshin 0612*/
                    pBroFile->FileType = FileTypePicture;   /*<--sanshin 0612*/
                }                                           /*<--sanshin 0612*/
                /*<--sanshin 0612*/                         /*<--sanshin 0612*/
#endif                                                      /*<--sanshin 0612*/
                else
                {
                    pBroFile->FileType = TempFileInfo->FileType;
                }
                pBroFile->FileLocInfo.Clus = FindData.Clus;
                pBroFile->FileLocInfo.Index = FindData.Index-1;
                GetLongFileName(FindData.Clus, FindData.Index - 1,FsType ,pBroFile->LongFileName);

#ifdef PIC_MEDIA
/*sanshin 0612 --->*/
                if (FsType == MUSIC_DB)                                                 /*<--sanshin 0612*/
                {                                                                       /*<--sanshin 0612*/
                    FileExtNameRemove(pBroFile->LongFileName, AudioFileExtString);      /*<--sanshin 0612*/
                }                                                                       /*<--sanshin 0612*/
                else if (FsType == JPEG_DB)                                             /*<--sanshin 0612*/
                {                                                                       /*<--sanshin 0612*/
                    FileExtNameRemove(pBroFile->LongFileName, PictureFileExtString);    /*<--sanshin 0612*/
                }                                                                       /*<--sanshin 0612*/
#else
                FileExtNameRemove(pBroFile->LongFileName, AudioFileExtString);
/*<--sanshin 0612*/
#endif
                pBroFile = pBroFile->pNext;
                NeedLoadFileNum--;
            }
            else
            {
                SkipLoadFileNum--;
            }
        }

        if (NeedLoadFolderNum == 0 && NeedLoadFileNum == 0)
        {
            break;  //file find out.
        }

    }
    while (FindNext(&TempFDT, &FindData, ExtName,FsType) == RETURN_OK);

}

/*
--------------------------------------------------------------------------------
  Function name : BroGetFileNumInFloder
  Author        : ChenFen
  Description   :

  Input         : when Flag =0 ,it go to get file number of all kinds of file.
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroGetFileNumInFloder(BrowserFileStruct *pBro, UINT16 Flag)
{
    int16  i;
    int16  ret;
    int16  FileNum = 0;
    int32  index   = 0;
    FDT    TempFDT;
    FileType FileType ;//= pBro->FileType;
    UINT32 DirClus    = pBro->FileLocInfo.Clus;

    if (Flag)
    {
        FileType = pBro->FileType;
    }
    else
    {
        FileType = FileTypeALL;
    }

    while (1)
    {
        ret = GetFDTInfo(&TempFDT,  DirClus, index++);
        if ( RETURN_OK != ret)
        {
            break;
        }
        if (TempFDT.Name[0]==FILE_NOT_EXIST)              //null direction item,there is no file in follow,
        {
            break;
        }
        if (TempFDT.Name[0]!=FILE_DELETED)
        {
            while (TempFDT.Attr==ATTR_LFN_ENTRY)         //long file name item should find short file name
            {
                GetFDTInfo(&TempFDT,  DirClus, index++);
            }
            if ( TempFDT.Attr & ATTR_VOLUME_ID )
                continue;
            
            if ( TempFDT.Attr & ATTR_HIDDEN )
            {
                DEBUG("Dir or File is Hidden attr...");
                continue;
            }

            if (!( TempFDT.Attr & ATTR_DIRECTORY ))//file
            {
                if ( BroCheckFileType( TempFDT.Name, FileType ))
                {
                    FileNum++;
                    for (i=0; i<11; i++)
                    {
                        if (pBro->FileFDT.Name[i]!= TempFDT.Name[i])
                            break;
                    }
                    if (11 == i)
                        break;
                }
            }
        }
    }
    return (FileNum);
}

/*
--------------------------------------------------------------------------------
  Function name : void BroKeyDownProc(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef _FRAME_BUFFER_
extern void LCD_Shift_Window(UINT16 frame_index,
                                 BUFFER_SHIFT_DIRECTION  direction,
                                 UINT16 distance,
                                 UINT16 x0,UINT16 y0,
                                 UINT16 x1,UINT16 y1);
#endif

_ATTR_BRO_UI_CODE_
void BroKeyDownProc(void)
{
    UINT16 TotalItems = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems;

    if (TotalItems == 0)
        return;

#ifdef _FRAME_BUFFER_
    if (BrowserData.CurPointer < (TotalItems - 1))
#else
    if ((BrowserData.CurPointer + BrowserData.CurStartIndex[BrowserData.CurDeep]) < TotalItems - 1)
#endif
    {
        BrowserData.PrePointer = BrowserData.CurPointer;

        if ((BrowserData.CurPointer ) >= BROWSER_SCREEN_PER_LINE - 1)
        {
#ifdef _FRAME_BUFFER_
            BrowserData.CurPointer++;

            if ((BrowserData.CurPointer - BrowserData.CurStartIndex[BrowserData.CurDeep])
                    == BROWSER_SCREEN_PER_LINE)
            {
                LCD_Shift_Window(0, DIRECTION_UP, 17, 0, 3, 122, 139);
                BrowserData.CurStartIndex[BrowserData.CurDeep] += 1;

                FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                                  BrowserData.pBrowserFile,
                                  &BrowserData.FolderInfo[BrowserData.CurDeep],
                                  BrowserData.CurStartIndex[BrowserData.CurDeep],
                                  gBrowSerFsType);
            }

            SendMsg(MSG_BROW_DIS_SELE_ITEM);
#else
            BrowserData.CurStartIndex[BrowserData.CurDeep] += BROWSER_SCREEN_PER_LINE;
            BrowserData.CurPointer = 0;
            FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                              BrowserData.pBrowserFile,
                              &BrowserData.FolderInfo[BrowserData.CurDeep],
                              BrowserData.CurStartIndex[BrowserData.CurDeep],
                              gBrowSerFsType);

            SendMsg(MSG_BROW_DIS_ALL_ITEM);
#endif

        }
        else
        {
            BrowserData.CurPointer++;
            SendMsg(MSG_BROW_DIS_SELE_ITEM);
        }
    }
    else
    {
        BrowserData.CurStartIndex[BrowserData.CurDeep] = 0;
        BrowserData.CurPointer = 0;

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                          BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep],
                          BrowserData.CurStartIndex[BrowserData.CurDeep],
                          gBrowSerFsType);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }

}

/*
--------------------------------------------------------------------------------
  Function name : void BroKeyUpProc(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroKeyUpProc(void)
{
    UINT16 TotalItems = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems;

    if (TotalItems == 0)
        return;

    BrowserData.PrePointer = BrowserData.CurPointer;

#ifdef _FRAME_BUFFER_
    if (BrowserData.CurPointer > 0)
    {
        BrowserData.CurPointer--;

        if (BrowserData.CurPointer < BrowserData.CurStartIndex[BrowserData.CurDeep])
        {
            LCD_Shift_Window(0, DIRECTION_DOWN, 17, 0, 3, 122, 139);
            BrowserData.CurStartIndex[BrowserData.CurDeep] -= 1;

            FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,BrowserData.pBrowserFile,
                              &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep],gBrowSerFsType);
        }

        SendMsg(MSG_BROW_DIS_SELE_ITEM);
    }
    else
    {
        //return to the last page.
        if (TotalItems >= BROWSER_SCREEN_PER_LINE)
        {
            BrowserData.CurStartIndex[BrowserData.CurDeep] = TotalItems - BROWSER_SCREEN_PER_LINE;
        }
        else
        {
            BrowserData.CurStartIndex[BrowserData.CurDeep] = 0;
        }

        BrowserData.CurPointer = TotalItems - 1;

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep],gBrowSerFsType);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }
#else
    if (BrowserData.CurPointer > 0)
    {
        BrowserData.CurPointer--;
        SendMsg(MSG_BROW_DIS_SELE_ITEM);
    }
    else
    {
        if (BrowserData.CurStartIndex[BrowserData.CurDeep] > 0)
        {
            //previous
            BrowserData.CurStartIndex[BrowserData.CurDeep] -= BROWSER_SCREEN_PER_LINE;
            BrowserData.CurPointer = BROWSER_SCREEN_PER_LINE-1;
        }
        else
        {
            //return to the last page.
            BrowserData.CurStartIndex[BrowserData.CurDeep] = ((TotalItems-1)/BROWSER_SCREEN_PER_LINE)*BROWSER_SCREEN_PER_LINE;
            BrowserData.CurPointer = TotalItems - BrowserData.CurStartIndex[BrowserData.CurDeep] - 1;
        }

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep],gBrowSerFsType);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }
#endif
}



/*
--------------------------------------------------------------------------------
  Function name : int16 BroKeyEnterPro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroKeyEnterPro(void)
{
    UINT16      i;
    UINT16      temp;
    FileType    CurFileType;
    BrowserFileStruct *pBro;
    TASK_ARG    TaskArg;
    BroFileInfo *FileExtName;

    pBro = BrowserData.pBrowserFile;

#ifdef _FRAME_BUFFER_
    for (i = BrowserData.CurStartIndex[BrowserData.CurDeep]; i < BrowserData.CurPointer; i++)
    {
        pBro = pBro->pNext;
    }
#else
    for (i = 0;i < BrowserData.CurPointer;i++)
    {
        pBro = pBro->pNext;
    }
#endif

    CurFileType = pBro->FileType;

    if (CurFileType == FileTypeFolder)
    {
        if (0 == BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems)
        {
            return 0 ;//it go to do nothing if is null.
        }

        if ((BrowserData.CurStartIndex[BrowserData.CurDeep] == 0)  &&  (BrowserData.CurPointer == 0) && (gBrowSerFsType == MUSIC_DB))
        {
            i = 1;

            FileExtName  = GetSearchFileInfo(FileTypeAudio);

#ifdef _MUSIC_
            //printf("From All File Clus = %d, FileExName = %d", pBro->FileLocInfo.Clus, FileExtName->FileExtName);

            TaskArg.Music.FileNum = GetGlobeFileNum(i,pBro->FileLocInfo.Clus,FileExtName->FileExtName, gBrowSerFsType);

            //printf("From All File: AudioFileNum = %d\n", TaskArg.Music.FileNum);

            if (BrowserData.CurDeep == 0)
            {
                GetResourceStr(SID_DIR_LIST, gMusicTypeSelName, MAX_FILENAME_LEN);
            }
            else
            {
                GetLongFileName(SubDirInfo[CurDirDeep - 1].DirClus, SubDirInfo[CurDirDeep - 1].CurDirNum - 1, MUSIC_DB, gMusicTypeSelName);
            }

            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

            gSysConfig.MusicConfig.RepeatModeBak =  gSysConfig.MusicConfig.RepeatMode + AUDIO_ALLONCE;
            AudioFileInfo.ucSelPlayType = SORT_TYPE_SEL_FOLDER;

            gwSaveDirClus = BrowserData.FolderInfo[BrowserData.CurDeep].DirClus;

            //ThreadDeleteAll(&pMainThread);

#ifdef AUDIOHOLDONPLAY  //4.debug,it can not switch music when in brower.
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;
#endif

            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_MUSIC, &TaskArg);
            return 1;
#endif

        }
#ifdef PIC_MEDIA                                                                    /*<--sanshin 0612*/
    /* sanshin 0612 -->*/                                                           /*<--sanshin 0612*/
     if ((BrowserData.CurStartIndex[BrowserData.CurDeep] == 0) &&                   /*<--sanshin 0612*/
         (BrowserData.CurPointer == 0) &&                                           /*<--sanshin 0612*/
         (gBrowSerFsType == JPEG_DB)                                                /*<--sanshin 0612*/
         //(BrowserData.CurDeep == 0)                                               /*<--sanshin_20150702*/
     ){                                                                             /*<--sanshin 0612*/
                TaskArg.MediaBro.CurId= 0;                                          /*<--sanshin 0612*/
                //TaskArg.MediaBro.TitleAdd = MediaItem[gMusicTypeSelID][1];        /*<--sanshin 0612*/
                TaskSwitch(TASK_ID_PICBRO, &TaskArg);                               /*<--sanshin 0612*/
                return 1;                                                           /*<--sanshin 0612*/
     }                                                                              /*<--sanshin 0612*/
    /*<--sanshin 0612*/                                                             /*<--sanshin 0612*/
#endif                                                                              /*<--sanshin 0612*/

        if (BrowserData.CurDeep < MAX_DIR_DEPTH-1)
        {

            BrowserData.CurPointerBack[BrowserData.CurDeep] = BrowserData.CurPointer;
            BrowserData.CurDeep++;
            BrowserData.CurStartIndex[BrowserData.CurDeep] = 0;
            BrowserData.CurPointer = 0;
#ifdef PIC_MEDIA
/*sanshin 0612 --->*/
            if (                                    /*<--sanshin 0612*/
                (gBrowSerFsType == MUSIC_DB)        /*<--sanshin 0612*/
                ||                                  /*<--sanshin 0612*/
                (gBrowSerFsType == JPEG_DB)         /*<--sanshin 0612*/
            )                                       /*<--sanshin 0612*/
/*<--sanshin 0612*/
#else
            if (gBrowSerFsType == MUSIC_DB)
#endif
            {
                BrowserData.FolderInfo[BrowserData.CurDeep].DirClus = GetCurDir((pBro->FileLocInfo).Clus, (pBro->FileLocInfo).Index, gBrowSerFsType);
            }
            else
            {
                BrowserData.FolderInfo[BrowserData.CurDeep].DirClus = (((UINT32)pBro->FileFDT.FstClusHI<<16) |pBro->FileFDT.FstClusLO);
            }
            FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep], gBrowSerFsType);
            FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile, &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep],gBrowSerFsType);

            if (0 == BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems)
            {
                //if there is not file,browser interface is null,it do not display dialog.
                SendMsg(MSG_BROW_CHECK_FILE);
            }
            else
            {
                SendMsg(MSG_BROW_DIS_ALL_ITEM);
                ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);//clear scroll message,if not,it still fresh scrolllist when the long file name folder is empty.
            }
        }
    }
    else
    {
        FileExtName = GetSearchFileInfo(pBro->FileType);
        if (gBrowSerFsType == FS_FAT)
        {
            if ((CurFileType != FileTypeAudio) && (CurFileType != FileTypePicture))
            {
                i = BroGetFileNumInFloder(pBro,0);
                GlobalFilenum = GetGlobeFileNum(i,pBro->FileLocInfo.Clus,"*", gBrowSerFsType);
                i = BroGetFileNumInFloder(pBro,1);
            }
            else
            {
                i = BroGetFileNumInFloder(pBro,1);
            }
        }
        else
        {
#ifdef _FRAME_BUFFER_
            i = BrowserData.CurPointer - BrowserData.FolderInfo[BrowserData.CurDeep].TotalFolders + 1;
#else
            i = BrowserData.CurStartIndex[BrowserData.CurDeep] + BrowserData.CurPointer - BrowserData.FolderInfo[BrowserData.CurDeep].TotalFolders + 1;
#endif
        }

        if (i == 0) //delete all file.then do not exit to brower,but to key menu the device crash.
        {
            return 0;
        }

#ifdef _FRAME_BUFFER_
        gBrowserData_CurStartIndex = BrowserData.CurStartIndex[BrowserData.CurDeep];
#endif

        if (CurFileType == FileTypeAudio)
        {
#ifdef _MUSIC_
            //因为蓝牙线程也有使用
            //如果有蓝牙线程工作，先关闭蓝牙线程
#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_
#else
            if (TRUE == ThreadCheck(pMainThread, &BlueToothThread))
            {
                ThreadDeleteAll(&pMainThread);
            }
#endif
#endif
            TaskArg.Music.FileNum = GetGlobeFileNum(i,pBro->FileLocInfo.Clus,FileExtName->FileExtName, gBrowSerFsType);

            //printf("From File: AudioFileNum = %d\n", TaskArg.Music.FileNum);

            if (BrowserData.CurDeep == 0)
            {
                GetResourceStr(SID_DIR_LIST, gMusicTypeSelName, MAX_FILENAME_LEN);
            }
            else
            {
                GetLongFileName(SubDirInfo[CurDirDeep - 1].DirClus, SubDirInfo[CurDirDeep - 1].CurDirNum - 1, MUSIC_DB, gMusicTypeSelName);
            }

            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                if ((AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER) && (gBrowSerFsType == FS_FAT))
                {
                    //------------------------------------------------------------------------------
                    //this part has called music backgroud variables,only backgroud music playing,it can process the part code.
                    temp  = AudioFileInfo.CurrentFileNum;
                    if (FIND_FILE_RANGE_DIR == AudioFileInfo.Range)
                    {
                        temp = GetGlobeFileNum(AudioFileInfo.CurrentFileNum, AudioFileInfo.FindData.Clus, FileExtName->FileExtName, gBrowSerFsType);
                    }

                    if (TaskArg.Music.FileNum == temp)
                    {
                        SendMsg(MSG_BROW_TASK_HAS_SWITCH);
                        TaskSwitch(TASK_ID_MUSIC, NULL);
                        return 1;
                    }
                }
                else if ((AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER) && (gBrowSerFsType == MUSIC_DB))
                {
                    //------------------------------------------------------------------------------
                    //this part has called music backgroud variables,only backgroud music playing,it can process the part code.
                    temp  = AudioFileInfo.CurrentFileNum;
                    if (FIND_FILE_RANGE_DIR == AudioFileInfo.Range)
                    {
                        temp = GetGlobeFileNum(AudioFileInfo.CurrentFileNum, AudioFileInfo.FindData.Clus, FileExtName->FileExtName, gBrowSerFsType);
                    }

                    if (TaskArg.Music.FileNum == temp)
                    {
                        SendMsg(MSG_BROW_TASK_HAS_SWITCH);
                        TaskSwitch(TASK_ID_MUSIC, NULL);
                        return 1;
                    }
                }

                //------------------------------------------------------------------------------
            }

            ThreadDelete(&pMainThread, &MusicThread);

            if (gBrowSerFsType == FS_FAT)
            {
                AudioFileInfo.ucSelPlayType = SORT_TYPE_SEL_BROWSER; //the macro SORT_TYPE_SEL_BROWSER is consist wich meida.
                gwSaveDirClus = 0;
            }
            else
            {
                gSysConfig.MusicConfig.RepeatModeBak =  gSysConfig.MusicConfig.RepeatMode;
                gwSaveDirClus = 0;
                AudioFileInfo.ucSelPlayType = SORT_TYPE_SEL_FOLDER;
            }

#ifdef AUDIOHOLDONPLAY  //4.debug,it can not switch music when in brower.
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_MUSIC, &TaskArg);
            return 1;
#endif
        }

        else if (CurFileType == FileTypePicture)
        {  //pictrue file
#ifdef _PICTURE_
            //ThreadDeleteAll(&pMainThread);//by zs 05.25
            //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
#ifdef PIC_MEDIA                                                                                                                            /*<--sanshin 0612*/
/*sanshin 0612 --->*/                                                                                                                       /*<--sanshin 0612*/
            pBro->FileLocInfo.Index = pBro->FileLocInfo.Index + 1   ;                                                                       /*<--sanshin 0612*/
                                                                                                                                            /*<--sanshin 0612*/
            //DEBUG("Clus = %d index = %d",pBro->FileLocInfo.Clus,pBro->FileLocInfo.Index);                                                 /*<--sanshin 0612*/
            PicSysFileInfo.FindData.Clus = pBro->FileLocInfo.Clus;                                                                          /*<--sanshin 0612*/
            PicSysFileInfo.FindData.Index = pBro->FileLocInfo.Index;                                                                        /*<--sanshin 0612*/
                                                                                                                                            /*<--sanshin 0612*/
            PicSysFileInfo.pExtStr = PictureFileExtString;                                                                                  /*<--sanshin 0612*/
            PicSysFileInfo.ulFullInfoSectorAddr = MediaInfoAddr + JPEG_SAVE_INFO_SECTOR_START;                                              /*<--sanshin 0612*/
            PicSysFileInfo.ulSortInfoSectorAddr = MediaInfoAddr + JPEG_TREE_ALL_SORT_INFO_SECTOR_START;                                     /*<--sanshin 0612*/
            PicSysFileInfo.ucCurDeep = BrowserData.CurDeep;                                                                                 /*<--sanshin 0612*//*<--sanshin_20150702*/
            PicSysFileInfo.TotalFiles = GetTotalFiles(PicSysFileInfo.FindData.Clus, FileExtName->FileExtName, gBrowSerFsType);              /*<--sanshin 0612*/
            TaskArg.Pic.FileNum = pBro->FileLocInfo.Index;                                                                                  /*<--sanshin 0612*/
            TaskArg.Pic.FromWhere = 0;                                                                                                      /*<--sanshin_20150630*/
            TaskArg.Pic.FsType = gBrowSerFsType;                                                                                            /*<--sanshin 0612*//*<--sanshin_20150702*/

            if(gBrowSerFsType == JPEG_DB){
                /*<--sanshin 0612*/
                TaskArg.Pic.StartFileNum = GetTotalSubDir(PicSysFileInfo.FindData.Clus, gBrowSerFsType);                                        /*<--sanshin 0612*/
                //if(BrowserData.CurDeep == 0)                                                                                                  /*<--sanshin_20150702*/
                {
                    TaskArg.Pic.StartFileNum++;// all file                                                                                      /*<--sanshin 0612*/
                }
            }else{
                TaskArg.Pic.StartFileNum = (PicSysFileInfo.FindData.Index - (i - 1));
            }
                                                                                                                                            /*<--sanshin 0612*/
#else                                                                                                                                       /*<--sanshin 0612*/
            TaskArg.Pic.FileNum = GetGlobeFileNum(i,pBro->FileLocInfo.Clus,FileExtName->FileExtName, gBrowSerFsType);                       /*<--sanshin 0612*/
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);                                                                                              /*<--sanshin 0612*/
/*<--sanshin 0612*/                                                                                                                         /*<--sanshin 0612*/
#endif                                                                                                                                      /*<--sanshin 0612*/
            //if(!CheckMsg(MSG_BROW_FROM_MAINMENU))                                                                                         /*<--sanshin_20150702*/
            {
                SendMsg(MSG_BROW_TASK_HAS_SWITCH);                                                                                          /*<--sanshin_20150702*/
            }
            TaskSwitch(TASK_ID_PICTURE, &TaskArg);
            return 1;
#endif
        }
        else if (CurFileType == FileTypeVideo)
        {
#ifdef _VIDEO_
            ThreadDeleteAll(&pMainThread);//by zs 05.25
            //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
            TaskArg.Video.FileNum = GetGlobeFileNum(i,pBro->FileLocInfo.Clus,FileExtName->FileExtName, gBrowSerFsType);
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_VIDEO, &TaskArg);
            return 1;
#endif
        }

        else if (CurFileType == FileTypeText)
        {
#ifdef _EBOOK_
            TaskArg.Text.FileNum = GetGlobeFileNum(i,pBro->FileLocInfo.Clus,FileExtName->FileExtName, gBrowSerFsType);
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_EBOOK, &TaskArg);
            return 1;
#endif
        }
        else //other file format not support
        {
            DEBUG(" ## NOT SUPPORT FORMAT ##");
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
            TaskArg.Message.HoldTime  = 3;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;

            gBrowserKeep  = 1;
            //HorizontalDisplay();
            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
        }
    }

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void BroDeleteFilePro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroDeleteFilePro(void)
{
    uint32 i;
    BrowserFileStruct *pBro;
    TASK_ARG TaskArg;

    pBro = BrowserData.pBrowserFile;
    for (i = 0;i < BrowserData.CurPointer; i++)
    {
        pBro = pBro->pNext;
    }

    if ((pBro->FileType == FileTypeFolder) ||(gBrowserFindFileType!= FileTypeALL))
    {
        return ;
    }

    i = BroGetFileNumInFloder(pBro,1);
    if (i == 0)  //no file return.
    {
        return;
    }

    TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
    TaskArg.Dialog.TitleID = SID_WARNING;
    TaskArg.Dialog.ContentID = SID_DELETE_FILE_ASK;
    WinCreat(&BrowserWin, &DialogWin, &TaskArg);

    SendMsg(MSG_BROW_DELETE_FILE);//file delete message.
    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    return;
}


/*
--------------------------------------------------------------------------------
  Function name : int16 BroKeyExitPro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroKeyExitPro(void)
{
    uint32   i;
    FileType CurFileType;
    TASK_ARG TaskArg;
    BrowserFileStruct *pBro;

    if (BrowserData.CurDeep > 0)
    {
        BrowserData.CurDeep--;
        BrowserData.CurPointer = BrowserData.CurPointerBack[BrowserData.CurDeep];
        FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName,
                         &BrowserData.FolderInfo[BrowserData.CurDeep],
                         gBrowSerFsType);

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                          BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep],
                          BrowserData.CurStartIndex[BrowserData.CurDeep],
                          gBrowSerFsType);

        SendMsg(MSG_BROW_DIS_ALL_ITEM);
        return (MSG_MODULE_OK);
    }
    else
    {
#ifdef _MEDIA_MODULE_
        if (gBrowSerFsType == MUSIC_DB)
        {
            if (gBrowserFindFileType == FileTypeAudio)
            {
                if ( BroswerFlag == FALSE)
                {
#ifdef _MUSIC_
                    if ((FALSE == ThreadCheck(pMainThread, &MusicThread)))//&& (HoldOnPlayInfo.HoldMusicGetSign == 0))
                    {
#if(MEDIALIBTYPE == 0)
                        {
                            TaskArg.Medialib.CurId = gMusicTypeSelID- 1;
                        }
#else
                        {
                            TaskArg.Medialib.CurId =gMusicTypeSelID;
                        }
#endif
                    }
                    else
#endif
                    {
                        TaskArg.Medialib.CurId =gMusicTypeSelID;
                    }
                    TaskSwitch(TASK_ID_MEDIALIB, &TaskArg); //exit to media
                    return (MSG_MODULE_EXIT);
                }
            }
        }
        else
#endif
        {
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (BrowserMemSelEn == 0 && !gBrowserKeep)
            {
                BrowserMemSelEn = 1;
                BroLoadMemSelStr();

                SendMsg(MSG_BROW_DIS_ALL_ITEM);
                return (MSG_MODULE_OK);
            }
            else
#endif
            {
                gBrowserKeep = 0;
                switch (gBrowserFindFileType)
                {
                    case FileTypeAudio:
#ifdef _MUSIC_
#ifdef _MEDIA_MODULE_
                        if (BroswerFlag ==FALSE)
                        {
                            TaskSwitch(TASK_ID_MEDIALIB, NULL); //exit to media
                        }
                        else
#endif
                        {
                            //TaskArg.MainMenu.MenuID = MAINMENU_ID_BROWSER;/*Sanshin*/
                            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);/*Sanshin*/
                        }
#endif
                        break;

                    case FileTypeText:
#ifdef _EBOOK_
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_EBOOK;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                        break;

                    case FileTypeVideo:
                        break;

                    case FileTypePicture:
#ifdef _PICTURE_
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                        break;

                    default:
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_BROWSER;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                        break;
                }
                return (MSG_MODULE_EXIT);
            }
        }
    }
}

#if(defined (_SDCARD_) || defined (_MULT_DISK_))
/*
--------------------------------------------------------------------------------
  Function name : BroLoadMemSelStr
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroSDCardCheck(void)
{
    //it is to update display item information only when card select interface
    //and brower display the content of sd card
    if ((BrowserMemSel == CARD)||(BrowserMemSelEn == 1))
    {
        BrowserValueInit(1);
        FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName,
                         &BrowserData.FolderInfo[BrowserData.CurDeep],
                         gBrowSerFsType);
        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                          BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep],
                          BrowserData.CurStartIndex[BrowserData.CurDeep],
                          gBrowSerFsType);

        BrowserMemSel         = FLASH0;
        BrowserData.TotleDisk = 1;

        BrowserMemSelEn = 1;
        BroLoadMemSelStr();

        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BroLoadMemSelStr
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroLoadMemSelStr(void)
{
    BrowserFileStruct *pBro = BrowserData.pBrowserFile;

    BrowserData.CurPointer = 0;

#ifdef _MULT_DISK_
    if (BrowserMemSel == CARD)
    {
        BrowserData.CurPointer = 1;
        if (bShowFlash1)
        {
            BrowserData.CurPointer = 2;
        }
    }
    else if (BrowserMemSel == FLASH1)
    {
        BrowserData.CurPointer = 1;
    }
#else
    if (BrowserMemSel == CARD)
    {
        BrowserData.CurPointer = 1;
    }
#endif

    BrowserData.PrePointer = BrowserData.CurPointer;

#ifdef _MULT_DISK_
    if (bShowFlash1)
        BrowserData.TotleDisk = 2;
    else
        BrowserData.TotleDisk = 1;

    if (gSysConfig.SDEnable)
    {
        if (CheckCard())
        {
            BrowserData.TotleDisk += 1;
        }
    }
#else
    BrowserData.TotleDisk = 1;
    if (gSysConfig.SDEnable)
    {
        if (CheckCard())
        {
            BrowserData.TotleDisk += 1;
        }
    }
#endif

    pBro->FileType   = FileTypeFolder;
    GetResourceStr(SID_C_FLASH, pBro->LongFileName, MAX_FILENAME_LEN);

#ifdef _MULT_DISK_
    if (bShowFlash1)
    {
        pBro = pBro->pNext;
        pBro->FileType   = FileTypeFolder;
        GetResourceStr(SID_D_FLASH, pBro->LongFileName, MAX_FILENAME_LEN);
    }
#endif

#ifdef _SDCARD_
    if (gSysConfig.SDEnable)
    {
        pBro = pBro->pNext;
        pBro->FileType   = FileTypeFolder;
#ifdef _MULT_DISK_
        if (bShowFlash1)
        {
            GetResourceStr(SID_E_SD_CARD, pBro->LongFileName, MAX_FILENAME_LEN);
        }
        else
        {
            GetResourceStr(SID_D_SD_CARD, pBro->LongFileName, MAX_FILENAME_LEN);
        }
#else
        GetResourceStr(SID_D_SD_CARD, pBro->LongFileName, MAX_FILENAME_LEN);
#endif
    }
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyDown(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroMemSelKeyDown(void)
{
    BrowserData.PrePointer = BrowserData.CurPointer;
    if (++BrowserData.CurPointer >= BrowserData.TotleDisk)
    {
        BrowserData.CurPointer = 0;
    }

    SendMsg(MSG_BROW_DIS_SELE_ITEM);
}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyUp(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroMemSelKeyUp(void)
{
    BrowserData.PrePointer = BrowserData.CurPointer;
    if (BrowserData.CurPointer == 0)
    {
        BrowserData.CurPointer = BrowserData.TotleDisk - 1;
    }
    else
    {
        BrowserData.CurPointer--;
    }

    SendMsg(MSG_BROW_DIS_SELE_ITEM);
}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyUp(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroMemSelKeyMenu(void)
{
    uint8 FileSysSetupRet;

    if (MemorySelect != BrowserMem[BrowserData.CurPointer])
    {
        if (ThreadCheck(pMainThread, &MusicThread) == TRUE)
        {
            ThreadDelete(&pMainThread, &MusicThread);
        }

        //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);

        MemorySelect = BrowserMem[BrowserData.CurPointer];

#ifdef _MULT_DISK_
        if ((MemorySelect == FLASH1)&&(bShowFlash1==0))
            MemorySelect = CARD;
#endif

#ifdef _SDCARD_
        if (MemorySelect == CARD)
        {
            if (gSysConfig.SDEnable)
            {
                SDCardEnable();
                MDScanDev(1);
            }
            else
            {
                MemorySelect = FLASH0;
            }
        }
#endif
        FileSysSetupRet =  FileSysSetup(MemorySelect);

#ifdef _SDCARD_
        if (FileSysSetupRet == OK)
        {

        }
        else
        {
            TASK_ARG TaskArg;
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_COULD_NOT_CONTINUE;
            TaskArg.Message.HoldTime  = 3;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;

            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
            SDCardDisable();
            MemorySelect = FLASH0;
            FileSysSetup(MemorySelect);
        }
#endif
    }

#if 0//def _SDCARD_
        if (MemorySelect == CARD)
        {
            if (gSysConfig.SDEnable)
            {
                SDCardEnable();
                MDScanDev(1);
            }
            else
            {
                MemorySelect = FLASH0;
            }

            FileSysSetupRet =  FileSysSetup(MemorySelect);
            if (FileSysSetupRet == OK)
            {
                SDCardDisable();
                MemorySelect = FLASH0;
                FileSysSetup(MemorySelect);
            }
            else
            {
                TASK_ARG TaskArg;
                TaskArg.Message.TitleID   = SID_WARNING;
                TaskArg.Message.ContentID = SID_COULD_NOT_CONTINUE;
                TaskArg.Message.HoldTime  = 3;
                TaskArg.Message.CurDisFrameIndex = 0;
                TaskArg.Message.UnsupportFrameIndex = 1;

                WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
            }
        }
#endif

    BrowserMemSel = MemorySelect;

    BrowserMemSelEn = 0;
    BrowserValueInit(1);
    FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep], gBrowSerFsType);
    FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile, &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep],gBrowSerFsType);

    SendMsg(MSG_BROW_DIS_ALL);
    SendMsg(MSG_BROW_CHECK_FILE);

}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyUp(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroMemSelKeyExit(void)
{
    uint32 i;
    FileType CurFileType;
    TASK_ARG TaskArg;

    switch (gBrowserFindFileType)
    {
        case FileTypeAudio:
#ifdef _MUSIC_
#if 0
            TaskSwitch(TASK_ID_MUSIC, NULL);
#endif
            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
            break;

        case FileTypeText:
#ifdef _EBOOK_
#if 0
            TaskArg.Text.FileNum = gSysConfig.TextConfig.FileNum;
            TaskSwitch(TASK_ID_EBOOK, &TaskArg);
#else
            TaskArg.MainMenu.MenuID = MAINMENU_ID_EBOOK;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
#endif
            break;

        case FileTypeVideo:
#ifdef _VIDEO_
            TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
            break;

        case FileTypePicture:
#ifdef _PICTURE_
            TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
            break;

        default:
            TaskArg.MainMenu.MenuID = MAINMENU_ID_BROWSER;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            break;
    }
    return (MSG_MODULE_EXIT);
}
#endif


/*
--------------------------------------------------------------------------------
  Function name : void BroDeleteFilePro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
UINT32 BroDeleteFile(void)
{
    UINT32 i;
    UINT32 Retval = 0;
    BrowserFileStruct *pBro = BrowserData.pBrowserFile;

    for (i = 0;i<BrowserData.CurPointer;i++)
    {
        pBro = pBro->pNext;
    }

    if (pBro->FileType == FileTypeFolder)
    {
        return Retval;
    }

//    ModuleOverlay(MODULE_ID_FILE_ENCODE, MODULE_OVERLAY_ALL);
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);

    FREQ_EnterModule(FREQ_MAX);//frist to improve the cpu frequency before to delete file.

    if (RETURN_OK == BroFileDelete(pBro->FileLocInfo.Clus,pBro->FileFDT.Name))
    {
//        ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);

        FileSysSetup(MemorySelect);

#ifdef AUDIOHOLDONPLAY  //clear breakpoint
        {
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;//zyz 2009-6-16
        }
#endif


        FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep], gBrowSerFsType);
        if (BrowserData.CurPointer > 0)
        {
            BrowserData.CurPointer--;
        }
        else
        {
            if (BrowserData.CurStartIndex[BrowserData.CurDeep] > 0)
            {
                BrowserData.CurStartIndex[BrowserData.CurDeep] -= BROWSER_SCREEN_PER_LINE;
                BrowserData.CurPointer = BROWSER_SCREEN_PER_LINE-1;
            }
        }
        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile, &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep],gBrowSerFsType);

        FREQ_ExitModule(FREQ_MAX);//retore cpu frequncy after delete file

        return(RETURN_OK);
    }

    FREQ_ExitModule(FREQ_MAX);//retore cpu frequncy after delete file

    return(RETURN_FAIL);
}
/*
********************************************************************************
*
*                         End of BrowserUI.c
*
********************************************************************************
*/
#endif

