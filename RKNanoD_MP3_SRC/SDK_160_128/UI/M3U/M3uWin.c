/*
********************************************************************************
*                   Copyright (c) 2008,CHENFEN
*                         All rights reserved.
*
* File Name:  M3uWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*
*    desc:      ORG.
********************************************************************************
*/

#define _IN_M3U_UI_

#include "SysInclude.h"

#ifdef  _M3U_

#include "FsInclude.h"

#include "M3uWin.h"
#include "M3uWinSub.h"

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
#include "AsicToUnicode.h"

void GetM3uListFileName(UINT16 start);
uint32 M3uGetDirClusIndex(uint8 *Path);

/*
--------------------------------------------------------------------------------
  Function name : void M3uWinInit(void)
  Author        : chenfen
  Description   : m3u window initialization func
                       currently use it to replace the Ebook Win ,and File type is set to text, cause m3u is a text file
  Input         :
  Return        :

  History:     <author>                    <time>         <version>
             chenfen / norton revises   2015/05/15      Ver1.1
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uWinInit(void *pArg)
{
    UINT8 from_where;
    HANDLE handle;
    TASK_ARG TaskArg;

    gM3uFindFileType = ((M3U_WIN_ARG*)pArg)->FileType;           //set in Mainmenu.c before TaskSwitch
    gM3uFildFileNum  = ((M3U_WIN_ARG*)pArg)->FileNum;            //file index
    from_where  = ((M3U_WIN_ARG*)pArg)->FromWhere;
    gM3uFsType = FS_FAT;

    FREQ_EnterModule(FREQ_MEDIAUPDATA);
    M3uValueInit(from_where);

    if(from_where == 0)
    {
        handle = FileOpen(M3uSysFileInfo.Fdt.Name, M3uSysFileInfo.FindData.Clus, M3uSysFileInfo.FindData.Index - 1, FS_FAT, FileOpenStringR);
        if ((int)handle == NOT_OPEN_FILE)
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 3;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            WinCreat(&M3uWin, &MessageBoxWin, &TaskArg);
            SendMsg(MSG_M3U_NOFIND_FILE);
        }
        else
        {
            M3uCreateGlobalNumList(handle);
            FileClose(handle);
            SendMsg(MSG_M3U_DIS_ALL);
        }
    }
    else
    {
        GetM3uListFileName(gM3uBrowserData.CurStartIndex[0]);
        SendMsg(MSG_M3U_DIS_ALL);
    }
    FREQ_ExitModule(FREQ_MEDIAUPDATA);
    KeyReset();
}

/*
--------------------------------------------------------------------------------
  Function name : void M3uWinInit(void)
  Author        : chenfen
  Description   : m3u window auti-initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uWinDeInit(void)
{
    //execute when you push the key,stop the scrolling file name
    ClearMsg(M3U_DISPFLAG_SCROLL_FILENAME);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 M3uWinService(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
uint32 M3uWinService(void)
{
    UINT32   i;
    UINT32   Retval = 0;
    TASK_ARG TaskArg;

    if (CheckMsg(MSG_M3U_NOFIND_FILE))
    {
        if (CheckMsg(MSG_MESSAGEBOX_DESTROY))
        {
            ClearMsg(MSG_M3U_NOFIND_FILE);
            ClearMsg(MSG_MESSAGEBOX_DESTROY);
            TaskArg.M3uBro.CurId= 1;
            TaskSwitch(TASK_ID_M3UBRO, &TaskArg);
            return 1;
        }
    }

    return (Retval);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 M3uWinKey(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
UINT32 M3uWinKey(void)
{
    INT32 KeyVal;
    INT16 RetVal = MSG_MODULE_OK;
    M3uFileStruct *pM3u;

    TASK_ARG TaskArg;

    KeyVal = GetKeyVal();

    //7 Key function
    switch(KeyVal)
    {
        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
            M3uKeyDownProc();
            break;

        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
            M3uKeyUpProc();
            break;

        case KEY_VAL_PLAY_SHORT_UP:
            break;

        case KEY_VAL_MENU_SHORT_UP:
            RetVal = M3uKeyEnterPro();
            break;

        case KEY_VAL_ESC_SHORT_UP:
            RetVal = M3uKeyExitPro();
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&M3uWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&M3uWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : M3uUIDisplay
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uWinPaint(void)
{
    UINT16  i,CurNum;
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode, TextMode;
    UINT16  TotalItem;
    UINT32   x,y;
    LCD_RECT             r;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;


    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_M3U_DIS_ALL)))
    {
        //display backgroud main window.
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        DisplayMenuStrWithIDNum(M3U_TITLE_TXT_X, M3U_TITLE_TXT_Y, M3U_TITLE_TXT_XSIZE,
                                M3U_TITLE_TXT_YSIZE, LCD_TEXTALIGN_CENTER, gM3uBrowserData.M3uTitleId);

        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+M3uBatteryLevel,105,146);
        SendMsg(MSG_M3U_DIS_ALL_ITEM);
        ClearMsg(M3U_DISPFLAG_SCROLL_FILENAME);
    }

     if(TRUE == GetMsg(MSG_BATTERY_UPDATE))
     {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+M3uBatteryLevel,105,146);
     }

    if (GetMsg(MSG_M3U_DIS_SELE_ITEM))
    {
        if (0/*page to next or prevous*/)
        {
            SendMsg(MSG_M3U_DIS_ALL_ITEM);
        }
        else
        {
            TotalItem = gM3uBrowserData.FolderInfo[0].TotalItems;
            ClearMsg(M3U_DISPFLAG_SCROLL_FILENAME);
            GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_MUSIC, &PicInfo1);
            GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
            for(i = 0; (i < M3U_ITEM_PER_PAGE) && (i < TotalItem); i++)
            {
                if((i == gM3uBrowserData.PrePointer) || (i == gM3uBrowserData.CurPointer))
                {
                    //deal with defferent case, the first case is PrePointer , set it as unselect state.
                    if (i == gM3uBrowserData.PrePointer)
                    {
                        //display a part(a line for an item) of a big picture. the background picture, is 128 * 160 pixel
                       DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
                    }

                    r.x0 = 20;
                    r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
                    r.x1 = r.x0 + PicInfo.xSize;
                    r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;

                    if (i == gM3uBrowserData.CurPointer)
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                    }

                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_MUSIC, PicInfo1.x, PicInfo.y+5+ 17 * i);

                    if(i == gM3uBrowserData.CurPointer)
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                        if(LCD_GetStringSize(M3uManage.M3u_Music[i].LongFileName) > PicInfo.xSize)
                        {
                            M3uPrintfBuf = M3uManage.M3u_Music[i].LongFileName;
                            M3uWinScrollInit(&r, IMG_ID_SEL_ICON, M3uManage.M3u_Music[i].LongFileName, 30);
                            SendMsg(M3U_DISPFLAG_SCROLL_FILENAME);
                        }
                        else
                        {
                            ClearMsg(M3U_DISPFLAG_SCROLL_FILENAME);
                        }
                    }

                    r.y0 += 2;
                    r.y1 =  r.y0+CH_CHAR_XSIZE_12-1;
                    TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
                    LCD_DispStringInRect(&r, &r, M3uManage.M3u_Music[i].LongFileName, LCD_TEXTALIGN_LEFT);
                    LCD_SetTextMode(TextMode);
                }
            }
        }
        SendMsg(MSG_M3U_DIS_SCLLO_ITEM);
    }


    if (GetMsg(MSG_M3U_DIS_ALL_ITEM))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3,143);
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_MUSIC, &PicInfo1);

        ClearMsg(M3U_DISPFLAG_SCROLL_FILENAME);
        TotalItem = gM3uBrowserData.FolderInfo[0].TotalItems;

        for(i = 0; ((i < M3U_ITEM_PER_PAGE) && (i < TotalItem )); i++)
        {
            //draw file type icon. add 2 in here is because the basic pictrue is smaller 4 than file icon in y coordinate,so get middle value.
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_MUSIC, PicInfo1.x, PicInfo.y+5+ 17 * i);

            r.x0 = 20;
            r.y0 = i*17+3;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0+17;

            if(i == gM3uBrowserData.CurPointer)
            {
                 DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                if(LCD_GetStringSize(M3uManage.M3u_Music[i].LongFileName) > PicInfo.xSize)
                {
                    M3uPrintfBuf = M3uManage.M3u_Music[i].LongFileName;
                    M3uWinScrollInit(&r, IMG_ID_SEL_ICON, M3uManage.M3u_Music[i].LongFileName, 30);

                    SendMsg(M3U_DISPFLAG_SCROLL_FILENAME);
                }
                else
                {
                    ClearMsg(M3U_DISPFLAG_SCROLL_FILENAME);
                }
            }

            r.y0 += 2;
            r.y1 =  r.y0+CH_CHAR_XSIZE_12-1;

            TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
            LCD_DispStringInRect(&r, &r, M3uManage.M3u_Music[i].LongFileName, LCD_TEXTALIGN_LEFT);
            LCD_SetTextMode(TextMode);
        }
        SendMsg(MSG_M3U_DIS_SCLLO_ITEM);
    }

    if (GetMsg(MSG_M3U_DIS_SCLLO_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL_BLOCK, &PicInfo1);

        CurNum = gM3uBrowserData.CurStartIndex[0]+gM3uBrowserData.CurPointer;

        if(CurNum > 0)
            y = (PicInfo.ySize - PicInfo1.ySize) * CurNum  /( gM3uBrowserData.FolderInfo[0].TotalItems -1);//+picInfo1.ySize/2;
        else
            y = 0;
        if((y+PicInfo1.ySize) >PicInfo.ySize)
            y = PicInfo.ySize - PicInfo1.ySize;

        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_SCOLL,122,0);
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_SCOLL_BLOCK,0+123, y);//scroll smal icon
    }

    if(TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNum(IMG_ID_BROWSER_BATTERY01 + M3uBatteryLevel);
    }

    if (CheckMsg(M3U_DISPFLAG_SCROLL_FILENAME))//an 4.21
    {
        TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(M3uPrintfBuf);
        LCD_SetTextMode(TextMode);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : M3uWinScrollInit
  Author        : norton
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 norton       2015/05/25         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;
    LCD_RECT           pRect1;

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
  Function name : M3uValueInit
  Author        : chenfen
  Description   : init an empty file list  gM3uBrowserData.pM3uFile for gM3uBrowserData. and Init the main gM3uBrowserData parameter
                such as: M3uTitleId(fileType)/RootDir/CurPoiter

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 norton    2015/05/15        Ver1.1
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uValueInit(UINT16 NeedInitAll)
{
    uint32 i, num;
    uint16 preid, curid;

    //if(NeedInitAll)
    {
        memset((UINT8*)&gM3uBrowserData,0,sizeof(gM3uBrowserData));
        gM3uBrowserData.pSearchFileInfo = M3uGetSearchFileInfo(gM3uFindFileType);//FileTypeText
        switch(gM3uFindFileType)
        {
            case FileTypeAudio:
                gM3uBrowserData.M3uTitleId = SID_MUSIC_FILE;
                break;

            case FileTypeText:
                gM3uBrowserData.M3uTitleId = SID_TEXT;
                break;

            case FileTypeVideo:
                gM3uBrowserData.M3uTitleId = SID_VIDEO_FILE;
                break;

            case FileTypePicture:
                gM3uBrowserData.M3uTitleId = SID_PICTURE_FILE;
                break;

            default:
                gM3uBrowserData.M3uTitleId = SID_EXPLORER;
                break;
        }

        gM3uBrowserData.FolderInfo[0].DirClus = GetRootDirClus(gM3uFsType);
    }
    if(NeedInitAll)
    {
        num = AudioFileInfo.M3uGlobalFileCnt;
        gM3uBrowserData.FolderInfo[0].TotalItems = num;
        for(i=0; i < num; i++)
        {
            gM3uGlobalFileNumBuf[i] = AudioFileInfo.M3uGlobalFileNumBuf[i];
        }

        curid = AudioFileInfo.CurrentFileNum - 1;
        preid = M3uSysFileInfo.WinStartId + M3uSysFileInfo.WinCurId;
        gM3uBrowserData.CurPointer = M3uSysFileInfo.WinCurId;
        gM3uBrowserData.PrePointer = M3uSysFileInfo.WinCurId;
        gM3uBrowserData.CurStartIndex[0] = M3uSysFileInfo.WinStartId;

        if(preid != curid)
        {
            if(preid > curid)
            {
                if(gM3uBrowserData.CurPointer >= (preid - curid))
                {
                    gM3uBrowserData.CurPointer -= (preid - curid);
                }
                else
                {
                    if(gM3uBrowserData.CurStartIndex[0] >= ((preid - curid) - gM3uBrowserData.CurPointer))
                    {
                        gM3uBrowserData.CurStartIndex[0] -= ((preid - curid) - gM3uBrowserData.CurPointer);
                    }
                    else
                    {
                        gM3uBrowserData.CurStartIndex[0] = 0;
                    }
                    gM3uBrowserData.CurPointer = 0;
                }
            }
            else
            {
                gM3uBrowserData.CurPointer += (curid - preid);
                if(gM3uBrowserData.CurPointer >= M3U_ITEM_PER_PAGE)
                {
                    gM3uBrowserData.CurStartIndex[0] += (gM3uBrowserData.CurPointer - (M3U_ITEM_PER_PAGE-1));
                    gM3uBrowserData.CurPointer = M3U_ITEM_PER_PAGE-1;
                }
            }
            gM3uBrowserData.PrePointer = gM3uBrowserData.CurPointer;
        }
    }
    else
    {
        gM3uBrowserData.CurStartIndex[0] = 0;
        gM3uBrowserData.PrePointer = 0;
        gM3uBrowserData.CurPointer = 0;
    }

    memset(gBrowserFileArray,0,sizeof(gBrowserFileArray));

    for(i = 0; i < M3U_ITEM_PER_PAGE - 1; i++)
    {
        gBrowserFileArray[i].pNext = &gBrowserFileArray[i + 1];
    }
    for(i = 1; i < M3U_ITEM_PER_PAGE; i++)
    {
        gBrowserFileArray[i].pPrev = &gBrowserFileArray[i - 1];
    }

    gBrowserFileArray[0].pPrev = &gBrowserFileArray[M3U_ITEM_PER_PAGE-1];
    gBrowserFileArray[M3U_ITEM_PER_PAGE-1].pNext = &gBrowserFileArray[0];
    gM3uBrowserData.pM3uFile     = &gBrowserFileArray[0];
}


/*
--------------------------------------------------------------------------------
  Function name : void M3uKeyDownProc(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uKeyDownProc(void)
{
    UINT16 TotalItems = gM3uBrowserData.FolderInfo[0].TotalItems;

    if (TotalItems == 0)
        return;

    if((gM3uBrowserData.CurPointer + gM3uBrowserData.CurStartIndex[0]) < TotalItems - 1)
    {
        gM3uBrowserData.PrePointer = gM3uBrowserData.CurPointer;

        if((gM3uBrowserData.CurPointer ) >= M3U_ITEM_PER_PAGE - 1)
        {
            gM3uBrowserData.CurStartIndex[0]++;
            GetM3uListFileName(gM3uBrowserData.CurStartIndex[0]);
            SendMsg(MSG_M3U_DIS_ALL_ITEM);
        }
        else
        {
            gM3uBrowserData.CurPointer++;
            SendMsg(MSG_M3U_DIS_SELE_ITEM);
        }
    }
    else
    {
        gM3uBrowserData.CurStartIndex[0] = 0;
        gM3uBrowserData.CurPointer = 0;
        GetM3uListFileName(gM3uBrowserData.CurStartIndex[0]);
        SendMsg(MSG_M3U_DIS_ALL_ITEM);
    }
}


/*
--------------------------------------------------------------------------------
  Function name : void M3uKeyUpProc(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uKeyUpProc(void)
{
    UINT16 CurDirTotalItems = gM3uBrowserData.FolderInfo[0].TotalItems;   // total item in cur dir

    if(CurDirTotalItems == 0)
        return;

      gM3uBrowserData.PrePointer = gM3uBrowserData.CurPointer;

      if(gM3uBrowserData.CurPointer > 0)
      {
        gM3uBrowserData.CurPointer--;
        SendMsg(MSG_M3U_DIS_SELE_ITEM);
      }
      else
      {
        if(gM3uBrowserData.CurStartIndex[0] > 0)
        {
            gM3uBrowserData.CurStartIndex[0]--;
            GetM3uListFileName(gM3uBrowserData.CurStartIndex[0]);
        }
        else
        {
            if(CurDirTotalItems <= M3U_ITEM_PER_PAGE)
            {
                gM3uBrowserData.CurStartIndex[0] = 0;
                gM3uBrowserData.CurPointer = CurDirTotalItems - 1;
            }
            else
            {
                gM3uBrowserData.CurStartIndex[0] = CurDirTotalItems - M3U_ITEM_PER_PAGE;
                gM3uBrowserData.CurPointer = M3U_ITEM_PER_PAGE - 1;
                GetM3uListFileName(gM3uBrowserData.CurStartIndex[0]);
            }
        }
        SendMsg(MSG_M3U_DIS_ALL_ITEM);   //display all the item, because it's a new page(next page or previous page)
      }
}


/*
--------------------------------------------------------------------------------
  Function name : int16 M3uKeyEnterPro(void)
  Author        : norton
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                   norton       2015/05/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
int16 M3uKeyEnterPro(void)
{
    UINT16      i, num;
    UINT16      temp;
    FileType    CurFileType;
    M3uFileStruct *pFileList;
    TASK_ARG    TaskArg;
    M3uFileInfo *FileExtName;

    if (TRUE == ThreadCheck(pMainThread, &MusicThread))
    {
        ThreadDelete(&pMainThread, &MusicThread);
    }
#ifdef AUDIOHOLDONPLAY
    gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif

    num = gM3uBrowserData.FolderInfo[0].TotalItems;
    temp = gM3uBrowserData.CurPointer + gM3uBrowserData.CurStartIndex[0];
    gwSaveDirClus = gM3uBrowserData.FolderInfo[0].DirClus;
    M3uSysFileInfo.WinCurId = gM3uBrowserData.CurPointer;
    M3uSysFileInfo.WinStartId = gM3uBrowserData.CurStartIndex[0];
    TaskArg.Music.FileNum = temp+1;
    AudioFileInfo.M3uCurSelGlobalFileNum = temp;
    AudioFileInfo.M3uGlobalFileCnt = num;
    for(i=0; i < num; i++)
    {
        AudioFileInfo.M3uGlobalFileNumBuf[i] = gM3uGlobalFileNumBuf[i];
    }
    AudioFileInfo.ucSelPlayType = SORT_TYPE_SEL_M3U_BROWSER;

    TaskSwitch(TASK_ID_MUSIC, &TaskArg);
    return 1;
}


/*
--------------------------------------------------------------------------------
  Function name : int16 M3uKeyExitPro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
int16 M3uKeyExitPro(void)
{
    TASK_ARG TaskArg;

    TaskArg.M3uBro.CurId= 1;
    TaskSwitch(TASK_ID_M3UBRO, &TaskArg);
    return 1;
}


/*
--------------------------------------------------------------------------------
  Function name :void M3uCreateGlobalNumList(HANDLE handle)
  Author        : norton
  Description   : You must have use M3uParseEntry() Already!And then the gM3uPlayListEntryInfo[] will
            have offset data.

  Input         : M3U file HANDLE
  Return        : null

  History:     <author>         <time>         <version>     <comment>
                    norton       2015/06/7            Ver2.0        chang function name from M3uShowPlaylist(handle), just do the job of get global num
                    norton       2015/05/17          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void M3uCreateGlobalNumList(HANDLE handle)
{

    int i=0, j=0, k=0;
    UINT16 FileNum,globalFileNum;
    UINT16 M3uPathOffset, M3uFileNameLen;
    UINT16 LineBuf[256*8];
    UINT8  buf[100];
    M3uFileInfo *FileExtName ;
    UINT8 readPoint=0;
    UINT8 M3uFileCodeType = CODE_TYPE_DEFAULT;
    TASK_ARG TaskArg;

    /// Parse_s ///
    FileRead(buf,3,handle);
   if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2]== 0xBF)
    {
        readPoint = 3;
        M3uFileCodeType = CODE_TYPE_UTF_8;
    }

    /* FilePointer Reset */
    FileSeek(readPoint, SEEK_SET, handle);

    {
        UINT8  find_flag=0;
        UINT8  zettai_flag=0;
        UINT8  skip_flag=0;
        UINT8  last_flag=0;
        UINT16 readNum;
        UINT16 remainNum;
        UINT16  charIndex/*, entryItemIndex=0 */;
        char curChar;
        int loop=0, loop1=0;

        do
        {
            readNum=FileRead(buf, 100, handle);     // File Read
            remainNum = readNum ;
            charIndex = 0;
            find_flag=0;
            zettai_flag=0;

            if(remainNum == 0)
            {
                last_flag = 1;
            }
            /*-------------------------*/
            /* M3U File Parse Proc     */
            /*-------------------------*/
            while(((remainNum > 0) && (i < M3U_LIST_FILE_MAX) ) || (last_flag == 1))
            {
                if(last_flag == 0)
                {
                remainNum--;
                curChar = buf[charIndex];
                charIndex++;

                switch(curChar){
                case '\r':
                {
                    /* End of Line (Windows or Mac) */

                    if(skip_flag == 1)
                    {
                        /* Comment */
                        skip_flag = 0;
                        break;
                    }
                    if(remainNum != 0)
                    {
                        if(buf[charIndex] == '\n')
                        {
                            /* \r\n偺Pattern */
                            charIndex++;
                            remainNum--;
                        }
                        if(j != 0)
                        {
                            LineBuf[j] = '\0';
                            if(M3uFileCodeType = CODE_TYPE_UTF_8)
                            {
                                M3uTransCodeFromUTF8ToUnicode(LineBuf); // Convert to UTF_8
                            }
                            M3uFileNameLen = j;
                            M3uPathOffset=0xFFFF;
                            if(LineBuf[0] == '\\')
                            {
                                zettai_flag = 1;    // Full Path
                            }else{
                                zettai_flag = 0;    // Relative Path
                            }
                            for(k=j; k != 0; k--)
                            {
                                if(LineBuf[k] == '\\')
                                {
                                    M3uPathOffset = k+1;
                                    break;
                                }
                            }
                            //DEBUG("/// TEST : Length=%d, PathOffset=%d\n", j, M3uPathOffset);
                            find_flag=1;    // 1Line Parse Fin
                            j=0;
                        }
                    }
                    break;
                }
                case '\n':
                    /* End of Line (Linux) */
                {
                    if(skip_flag == 1)
                    {
                        /* Comment */
                        skip_flag = 0;
                        break;
                    }
                    if(remainNum != 0)
                    {

                        if(j != 0)
                        {
                            LineBuf[j] = '\0';

                            if(M3uFileCodeType = CODE_TYPE_UTF_8)
                            {
                                M3uTransCodeFromUTF8ToUnicode(LineBuf); // Convert to UTF_8
                            }
                            M3uFileNameLen = j;
                            M3uPathOffset=0xFFFF;
                            if(LineBuf[0] == '\\')
                            {
                                zettai_flag = 1;    // Full Path
                            }else{
                                zettai_flag = 0;    // Relative Path
                            }
                            for(k=j; k != 0; k--)
                            {
                                if(LineBuf[k] == '\\')
                                {
                                    M3uPathOffset = k+1;
                                    break;
                                }
                            }
                            find_flag=1;    // 1Line Parse Fin
                            j=0;
                        }

                    }
                    break;
                }
                case '#':
                    /* Comment SKIP to next Line */
                    if(j == 0)
                    {
                        skip_flag = 1;
                        while(remainNum != 0)
                        {
                            if(buf[charIndex] == '\r')
                            {
                                skip_flag = 0;
                                charIndex++;
                                remainNum--;
                                if((buf[charIndex] == '\n') && (remainNum != 0))
                                {
                                    charIndex++;
                                    remainNum--;
                                }
                                break;
                            }
                            if(buf[charIndex] == '\n')
                            {
                                skip_flag = 0;
                                charIndex++;
                                remainNum--;
                                break;
                            }
                            charIndex++;
                            remainNum--;
                        }
                    }
                    else
                    {
                        LineBuf[j++] = (UINT16)curChar;
                    }
                    break;
                default:
                    /* ch -> Buff */
                    if(skip_flag == 1)
                    {
                        /* Comment */
                        j=0;
                        while(remainNum != 0)
                        {
                            if(buf[charIndex] == '\r')
                            {
                                skip_flag = 0;
                                charIndex++;
                                remainNum--;
                                if( (buf[charIndex] == '\n') && (remainNum != 0) )
                                {
                                    charIndex++;
                                    remainNum--;
                                }
                                break;
                            }
                            if(buf[charIndex] == '\n')
                            {
                                skip_flag = 0;
                                charIndex++;
                                remainNum--;
                                break;
                            }
                            charIndex++;
                            remainNum--;
                        }
                        break;
                    }
                    LineBuf[j++] = (UINT16)curChar;
                    break;
                }//end switch(curChar)
                }
                else
                {
                    last_flag = 0;
                    if(skip_flag == 1)
                    {
                        j=0;
                        skip_flag = 0;
                    }
                    else if(j != 0)
                    {
                        LineBuf[j] = '\0';
                        if(M3uFileCodeType = CODE_TYPE_UTF_8)
                        {
                            M3uTransCodeFromUTF8ToUnicode(LineBuf); // Convert to UTF_8
                        }
                        M3uFileNameLen = j;
                        M3uPathOffset=0xFFFF;
                        if(LineBuf[0] == '\\')
                        {
                            zettai_flag = 1;    // Full Path
                        }else{
                            zettai_flag = 0;    // Relative Path
                        }
                        for(k=j; k != 0; k--)
                        {
                            if(LineBuf[k] == '\\')
                            {
                                M3uPathOffset = k+1;
                                break;
                            }
                        }
                        find_flag=1;    // 1Line Parse Fin
                        j=0;
                    }
                }

                /*--------------------------*/
                /* Get GlobalFileNum Proc   */
                /*--------------------------*/
                if(find_flag == 1)              // M3U Path Found
                {

                    UINT32 Clus;
                    UINT16 PathBuf[8*256];
                    UINT16 FileName[256];
                    UINT16 ch, path_deep;
                    UINT16 offset = M3uPathOffset;
                    UINT16 len = M3uFileNameLen;
                    UINT16 pathlen;

                    FIND_DATA    FindDataInfo_Local;
                    FDT Fdt;
                    UINT16 M3uLongFileName[256];
                    UINT16 uiFindFileResult = 0 ;
                    UINT16 tmp, deep=0;
                    UINT8 judge_flag;

                    Clus = BAD_CLUS;
                    FileExtName  = M3uGetSearchFileInfo(FileTypeAudio);

                    /* Path Type Check */
                    if(zettai_flag == 1)
                    {
                        /* Full Path */
                        GotoRootDir(FileExtName->FileExtName,FS_FAT);   // Set RootDir Info
                        loop=1;
                    }
                    else
                    {
                        /* Relative Path */
                        gwSaveDirClus = M3uSysFileInfo.FindData.Clus;
                        GotoCurDir(FileExtName->FileExtName,FS_FAT);    // Set CurDir Info
                        loop=0;
                    }

                    /* Move First Dir */
                    memset(M3uLongFileName, 0 , sizeof(M3uLongFileName));
                    FindDataInfo_Local.Clus = CurDirClus;
                    uiFindFileResult = FindFirst(&Fdt, &FindDataInfo_Local, MusicFileExtString, FS_FAT_EX_VOICE);

                    if(M3uPathOffset != 0xFFFF)     // Have a Path
                    {
                        /*------------*/
                        /* Path Check */
                        /*------------*/

                        /* Get PathName */
                        offset = M3uPathOffset;
                        path_deep = 0;
                        for(loop1=0; loop < offset; loop++, loop1++)
                        {
                            ch = LineBuf[loop];
                            if(/*(UINT8)*/ch == '\\')               // Find Path char
                            {
                                PathBuf[loop1] = ch;
                                path_deep++;
                                loop1 = (256 * path_deep)-1;    // Path Divide
                            }
                            else
                            {
                                PathBuf[loop1] = ch;
                            }
                        }

                        while((uiFindFileResult == RETURN_OK) && (deep < path_deep))
                        {
                            judge_flag = 0;
                            if ((Fdt.Attr & ATTR_DIRECTORY))    // Type Dir
                            {
                                memset(M3uLongFileName, 0 , sizeof(M3uLongFileName));
                                M3uGetLongFileNameEraseSp(FindDataInfo_Local.Clus, FindDataInfo_Local.Index - 1, FS_FAT_EX_VOICE, M3uLongFileName);

                                judge_flag = 1;
                                for(tmp=0; /*(UINT8)*/(PathBuf[(deep * 256)+tmp]) != '\\'; tmp++)
                                {
                                    if(M3uLongFileName[tmp] != PathBuf[(deep * 256)+tmp])
                                    {
                                        /* Mismatch LongFileName */
                                        judge_flag = 0;
                                        break;
                                    }
                                }
                                if(M3uLongFileName[tmp] != '\0')    // Check Terminal char
                                {
                                    /* Mismatch LongFileName */
                                    judge_flag = 0;
                                }
                            }
                            if(judge_flag == 1)
                            {
                                /* Find Match Path */
                                deep++;
                                Clus = (((UINT32)Fdt.FstClusHI<<16) | Fdt.FstClusLO);       // Calc Clus
                                memset(M3uLongFileName, 0 , sizeof(M3uLongFileName));
                                FindDataInfo_Local.Clus = Clus;
                                uiFindFileResult = FindFirst(&Fdt, &FindDataInfo_Local, MusicFileExtString, FS_FAT_EX_VOICE);
                            }
                            else
                            {
                                /* Not Find Match Path */
                                /* Search Next */
                                uiFindFileResult = FindNext(&Fdt, &FindDataInfo_Local, MusicFileExtString, FS_FAT_EX_VOICE);
                            }
                        }
                        if(Clus == BAD_CLUS)
                        {
                            /* File Not Found */
                            find_flag = 0;
                        }
                    }
                    else
                    {
                        /* Not Have a Path */

                        if(zettai_flag == 1)
                        {
                            offset = 1;
                        }
                        else
                        {
                            offset = 0;
                        }
                    }


                    if(find_flag != 0)
                    {
                        /* Get FileName */
                        for(loop1=0, loop=offset; loop < len; loop++, loop1++)
                        {
                            FileName[loop1] = LineBuf[loop];
                        }
                        FileName[loop1]='\0';

                        /*------------*/
                        /* File Check */
                        /*------------*/

                        Clus = BAD_CLUS;

                        while(uiFindFileResult == RETURN_OK)
                        {
                            judge_flag = 0;
                            if((Fdt.Attr & ATTR_DIRECTORY) == 0)        // Type File
                            {
                                memset(M3uLongFileName, 0 , sizeof(M3uLongFileName));
                                GetLongFileName(FindDataInfo_Local.Clus, FindDataInfo_Local.Index - 1, FS_FAT_EX_VOICE, M3uLongFileName);   // Get LongFileName

                                judge_flag = 1;
                                for(tmp=0; /*(UINT8)*/(FileName[tmp]) != '\0'; tmp++)
                                {
                                    if(M3uLongFileName[tmp] != FileName[tmp])
                                    {
                                        /* Mismatch LongFileName */
                                        judge_flag = 0;
                                        break;
                                    }
                                }
                                if(M3uLongFileName[tmp] != '\0')    // Check Terminal char
                                {
                                    /* Mismatch LongFileName */
                                    judge_flag = 0;
                                }
                            }
                            if(judge_flag == 1)
                            {
                                /* Find Match File */
                                Clus = (((UINT32)Fdt.FstClusHI<<16) | Fdt.FstClusLO);       // Calc Clus
                                break;
                            }
                            else
                            {
                                /* Not Find Match Path */
                                /* Search Next */
                                uiFindFileResult = FindNext(&Fdt, &FindDataInfo_Local, MusicFileExtString, FS_FAT_EX_VOICE);
                            }
                        }
                        if(Clus != BAD_CLUS)
                        {
                            FileNum = M3uGetFileNumInFloderByShortName(Fdt.Name, FindDataInfo_Local.Clus, FileTypeAudio);   // Get FileNum
                            if(FileNum != 0)
                            {
                                globalFileNum = GetGlobeFileNum(FileNum, FindDataInfo_Local.Clus, FileExtName->FileExtName, gM3uFsType);    // Get GlobalFileNum
                                gM3uGlobalFileNumBuf[i] = globalFileNum;
                            }
                            else
                            {
                                /* File Not Found */
                                find_flag = 0;
                            }
                        }
                        else
                        {
                            /* File Not Found */
                            find_flag = 0;
                        }
                    }


                    /*------------------*/
                    /* Check Match File */
                    /*------------------*/
                    if(find_flag == 1)
                    {
                        /* File Found */
                        i++;
                        find_flag = 0;
                    }
                }

            }//end while
        }while((readNum > 0) && (i < M3U_LIST_FILE_MAX));

        gM3uBrowserData.FolderInfo[0].TotalItems = i;       // Num of Find File
        if(i == 0)
        {
            /* Not Found File */
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 3;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            WinCreat(&M3uWin, &MessageBoxWin, &TaskArg);
            SendMsg(MSG_M3U_NOFIND_FILE);
        }
        else
        {
            /* Get FileName */
            GetM3uListFileName(0);
        }
    }
}


/*
--------------------------------------------------------------------------------
  Function name : M3uTransCodeFromUTF8ToUnicode(gM3uPlayListEntryInfo[i].longName);
  Author        : norton
  Description   :Transform char code from UTF8 to Unicode big-endian, the system is unicode big-endian
                    currently , only support 3 byte char, because Japaness and chiness is only use 3 byte. no need to design 4 -6 bytes.

   |  Unicode符号范围      |  UTF-8编码方式
 n |  (十六进制)           | (二进制)
---+-----------------------+------------------------------------------------------
 1 | 0000 0000 - 0000 007F |                                              0xxxxxxx
 2 | 0000 0080 - 0000 07FF |                                     110xxxxx 10xxxxxx
 3 | 0000 0800 - 0000 FFFF |                            1110xxxx 10xxxxxx 10xxxxxx
 4 | 0001 0000 - 0010 FFFF |                   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 5 | 0020 0000 - 03FF FFFF |          111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 6 | 0400 0000 - 7FFF FFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
  Input         :
  Return        :

  History:     <author>         <time>         <version>
               norton              2015/06/3        Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
int16 M3uTransCodeFromUTF8ToUnicode(UINT16 *longName){

    int i = 0;
    int size ;

       int unicodeNameIndex = 0;    //use the same array to store the result ,no need to use buf.

    while(longName[i] != 0){

        size = M3uGetUTF8Size(longName[i]);
        switch(size){

        case 1:

            //no need to transform
            longName[unicodeNameIndex] = longName[i];
            i += 1;
            unicodeNameIndex += 1;

        break;

        case 2:

            longName[unicodeNameIndex]  = 0x07FF & (longName[i]<<6 | 0x003F & longName[i+1]);  // 110xxxxx 10xxxxxx ; mask:is 0x003f for low 6bit, after combine, the unit16 mask is 0x07FF for low 11bits

            i += 2;
            unicodeNameIndex +=1;
        break;

        case 3:

            longName[unicodeNameIndex]  =  longName[i]<<12 | (0x003F & longName[i+1])<<6 |0x003F &  longName[i+2] ;  //     1110xxxx 10xxxxxx 10xxxxxx => mask: 0x000F 0x003f 0x003f,but the highest byte just need to shift 12bit, the highest 4bit will be clear.

            i += 3;
            unicodeNameIndex +=1;
        break;

        case 4:
            DEBUG("Error Four byte UTF8, need to add case in M3uTransCodeFromUTF8ToUnicode()");
            i += 4;
        case 5:
            DEBUG("Error Four byte UTF8, need to add case in M3uTransCodeFromUTF8ToUnicode()");
            i +=5;
        case 6:
            DEBUG("Error Four byte UTF8, need to add case in M3uTransCodeFromUTF8ToUnicode()");
            i += 6;
        break;
        }
    }

    longName[unicodeNameIndex] = 0;
    //clear the unwanted words to 0
    while(longName[++unicodeNameIndex] != 0){

        longName[unicodeNameIndex] = 0;
    }
}



/*
--------------------------------------------------------------------------------
  Function name : M3uGetUTF8Size
  Author        : norton
  Description   : get one utf8 char size , maybe 1 -6 byte

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               norton              2015/06/3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
int M3uGetUTF8Size(uint16 pInput)
{
    char c = (uint8)pInput;
    // 0xxxxxxx 返回1, 1byte
    // 10xxxxxx 不存在
    // 110xxxxx 返回2, 2byte
    // 1110xxxx 返回3, 3byte
    // 11110xxx 返回4, 4byte
    // 111110xx 返回5, 5byte
    // 1111110x 返回6, 6byte
    if(c< 0x80) return 1;
    if(c>=0x80 && c<0xC0) return -1;
    if(c>=0xC0 && c<0xE0) return 2;
    if(c>=0xE0 && c<0xF0) return 3;
    if(c>=0xF0 && c<0xF8) return 4;
    if(c>=0xF8 && c<0xFC) return 5;
    if(c>=0xFC) return 6;
}


/*
--------------------------------------------------------------------------------
  Function name : M3uGetFileNumInFloderByShortName
  Author        : norton
  Description   : Get File Num in Folder by Short Name , Dir Clus , FileType

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               norton              2015/05/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
int16 M3uGetFileNumInFloderByShortName(UINT8 *shortName, UINT32 DirClus , FileType FileType)
{
    int i,j;
    int16  ret;
    int16  FileNum = 0;
    int32  index   = 0;
    FDT    TempFDT;

    while (1)
    {
        //get TempFDT by dir clus and index ,for
        ret = GetFDTInfo(&TempFDT,  DirClus, index++);

        if( RETURN_OK != ret)
        {
            break;
        }

        if (TempFDT.Name[0]==FILE_NOT_EXIST)              //null direction item,there is no file in follow,
        {
            break;
        }
        if (TempFDT.Name[0]!=FILE_DELETED)
        {
            while(TempFDT.Attr==ATTR_LFN_ENTRY)          //long file name item should change to short file name firstly
            {
                GetFDTInfo(&TempFDT,  DirClus, index++);  //
            }
            if ( TempFDT.Attr & ATTR_VOLUME_ID )
                continue;

            if (!( TempFDT.Attr & ATTR_DIRECTORY ))//file
            {
                if( M3uCheckFileType( TempFDT.Name, FileType ))
                {
                    FileNum++;
                    for(i=0; i<11; i++)
                    {
                        // compare short name
                        if(shortName[i]!= TempFDT.Name[i])
                            break;
                    }

                    if(11 == i)
                        return FileNum;
                }
            }
        }
    }
    return 0;  //not found
}


/*
--------------------------------------------------------------------------------
  Function name : GetM3uListFileName
  Author        : norton
  Description   : Get File Num in Folder by Short Name , Dir Clus , FileType

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               norton              2015/05/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_UI_CODE_
void GetM3uListFileName(UINT16 start)
{
    SYS_FILE_INFO m3uFileInfo ;
    UINT16 i, num;
    UINT16 loop;
    UINT16 SelPlayType;

    UINT16 j;
    UINT32 AddrOffset;
    UINT8  FileInfoBuf[MEDIA_ID3_SAVE_CHAR_NUM *2];
    UINT16 Id3NameBuf[MEDIA_ID3_SAVE_CHAR_NUM];

    /* Calc DispNum */
    if(M3U_ITEM_PER_PAGE < gM3uBrowserData.FolderInfo[0].TotalItems){
        num = M3U_ITEM_PER_PAGE;
    }else{
        num = gM3uBrowserData.FolderInfo[0].TotalItems;
    }

#if 1
    /* LongFileName */
    SelPlayType = AudioFileInfo.ucSelPlayType;
    AudioFileInfo.ucSelPlayType = 0;
    for(i=0; i < num; i++)
    {
        gwSaveDirClus = BootSector.BPB_RootClus;
        GotoRootDir(MusicFileExtString,FS_FAT);
        SysFindFileInit(&m3uFileInfo, gM3uGlobalFileNumBuf[start + i], FIND_FILE_RANGE_ALL,
                        gSysConfig.MusicConfig.PlayOrder, (UINT8*)MusicFileExtString);
        GetLongFileName(m3uFileInfo.FindData.Clus, m3uFileInfo.FindData.Index - 1, FS_FAT, M3uManage.M3u_Music[i].LongFileName);
    }
    AudioFileInfo.ucSelPlayType = SelPlayType;

#else
    /* ID3 TITLE */

    for(i=0; i < num; i++)
    {
        AddrOffset = (UINT32)(gM3uGlobalFileNumBuf[start+i]-1)*BYTE_NUM_SAVE_PER_FILE + ID3_TITLE_SAVE_ADDR_OFFSET;
        MDReadData(DataDiskID,((MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START)<<9)+AddrOffset, MEDIA_ID3_SAVE_CHAR_NUM*2, FileInfoBuf);

        //DEBUG("Id3NameBuf[%d] = ", i);//hoshi
        for(j=0; j<MEDIA_ID3_SAVE_CHAR_NUM; j++)
        {
            M3uManage.M3u_Music[i].LongFileName[j] = (UINT16)FileInfoBuf[2*j]+((UINT16)FileInfoBuf[2*j+1]<<8);
            //DEBUG("%c", M3uManage.M3u_Music[i].LongFileName[j]);//hoshi
        }
        //DEBUG("\n");//hoshi
    }
#endif
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  M3uGetLongFileNameEraseSp()
*  Author:
*  Description:     Get LongFileName And Erase Last Space
*
*  Input:   DirClus
*           Index
*           FsType
*  Output:  lfName
*  Return:
*  Calls:
*
*  History:     <author>         <time>         <version>
*
*     desc: ORG
********************************************************************************
*/
_ATTR_M3U_UI_CODE_
void M3uGetLongFileNameEraseSp(int32 DirClus, int32 Index, FS_TYPE FsType, uint16* lfName )
{
    uint16 i;
    uint16 Item = 1;
    FDT     TempFDT;
    uint16 *buf = lfName;
    uint16 offset;
    uint16 StringCnt;

    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;

    if (FsType == MUSIC_DB)
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
        Index--;
    }
    else if (FsType == RECORD_DB)
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
        Index--;
    }
#ifdef PIC_MEDIA
    else if(FsType == JPEG_DB)
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
        Index--;
    }
#endif
    memset((void*)lfName, 0, (MAX_FILENAME_LEN * 2));
    GetFDTInfo(&TempFDT, DirClus, Index);
    for (i=0; i<8; i++)
    {
        if (TempFDT.Name[i] == ' ')
            break;
        if ((TempFDT.NTRes & 0x08) && ((TempFDT.Name[i] >= 'A')&&(TempFDT.Name[i] <= 'Z')))
            *buf= TempFDT.Name[i]+32;
        else
            *buf= TempFDT.Name[i];
        buf++;
    }

    if (TempFDT.Name[8] != ' ')
    {
        *buf++ = '.';

        for (i=8; i<11; i++)
        {
            if ((TempFDT.Name[i] == ' ') && ((TempFDT.Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY))
                break;
            if ((TempFDT.NTRes & 0x10) && ((TempFDT.Name[i] >= 'A')&&(TempFDT.Name[i] <= 'Z')))
                *buf= TempFDT.Name[i]+32;
            else
                *buf= TempFDT.Name[i];
            buf++;
        }
    }
    if (Index == 0)
        return;

    StringCnt = 0;
    while (Item <= MAX_LFN_ENTRIES)
    {
        uint16 * plfName;
        uint8 *buf;
        GetFDTInfo(&TempFDT, DirClus, Index - Item);
        if (ATTR_LFN_ENTRY != TempFDT.Attr)
            break;

        buf = (uint8 *)&TempFDT;

        if ((buf[0] & LFN_SEQ_MASK) <= MAX_LFN_ENTRIES)
        {
            plfName = lfName + 13 * ((buf[0] & LFN_SEQ_MASK) - 1);
            buf++;
            for (i = 0;i<5;i++)
            {
                *plfName = (uint16)*buf++;
                *plfName |= ((uint16)(*buf++))<<8;
                plfName++;
                StringCnt++;

                if (StringCnt >= MAX_FILENAME_LEN)
                {
                    *plfName = 0;
                    return;
                }
            }
            buf += 3;
            for (i = 0;i<6;i++)
            {
                *plfName = (uint16)*buf++;
                *plfName |= ((uint16)(*buf++))<<8;
                plfName++;
                StringCnt++;

                if (StringCnt >= MAX_FILENAME_LEN)
                {
                    *plfName = 0;
                    return;
                }
            }
            buf += 2;
            for (i = 0;i<2;i++)
            {
                *plfName = (uint16)*buf++;
                *plfName |= ((uint16)(*buf++))<<8;
                plfName++;
                StringCnt++;

                if (StringCnt >= MAX_FILENAME_LEN)
                {
                    *plfName = 0;
                    return;
                }
            }
        }

        if ((Index - Item) == 0)
        {
            *plfName = 0;
            return;
        }

        Item++;
    };
}

/*
********************************************************************************
*
*                         End of M3uWin.c
*
********************************************************************************
*/
#endif

