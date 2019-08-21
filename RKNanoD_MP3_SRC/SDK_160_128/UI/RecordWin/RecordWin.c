/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                      All rights reserved.
*
* File Name：   RecordWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*                WangBo         2009-4-10          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_RECORDWIN_

#include "SysInclude.h"

#ifdef _RECORD_

#include "RecordWinInterface.h"
#include "RecordWin.h"
#include "MainMenu.h"
#include "Hold.h"

#ifdef _RECODE_IMAGE_DIAPLAY_

/*
********************************************************************************
*  Function name :  DispPictureWithIDNumAndXY()
*  Author:          anzhiguo
*  Description:     display the picture by picture id and coordinate,the parameters that function setting
                    have any relation with parameter that is generated by tool.
*
*  Input:           pictureIDNum     :  picture resource ID.
*                   x, y ：             the coordinate to display picture.
*  Output:          null
*  Return:          null
*
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo       2009-2-24     Ver1.0       ORG
********************************************************************************
*/
_ATTR_RECORDWIN_CODE_
void  RecordGetPictureResourceData(UINT16 pictureIDNum, uint8 *pBuf)
{
    UINT32 i;
    PICTURE_INFO_STRUCT  picInfo;

    GetPictureInfoWithIDNum(pictureIDNum, &picInfo);
    LcdGetResourceData(picInfo.offsetAddr + ImageLogicAddress, pBuf, RECORD_PIC_BUF_SIZE);

#if 0
    for (i = 0; i < RECORD_PIC_BUF_NUM; i++)
    {
        LcdGetResourceData(picInfo.offsetAddr + ImageLogicAddress + i * RECORD_PIC_BUF_SIZE,
                           (UINT8*) RecordDispPicBuf[i], RECORD_PIC_BUF_SIZE);
    }
#endif
}

/*
********************************************************************************
*  Function name :  DispPictureWithIDNumAndXY()
*  Author:          anzhiguo
*  Description:     display the picture by picture id and coordinate,the parameters that function setting
                    have any relation with parameter that is generated by tool.
*
*  Input:           pictureIDNum     :  picture resource ID.
*                   x, y ：             the coordinate to display picture.
*  Output:          null
*  Return:          null
*
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo       2009-2-24     Ver1.0       ORG
********************************************************************************
*/
_ATTR_RECORDWIN_CODE_
void RecordDisplayPicture(INT16 x, INT16 y, INT16 xsize, INT16 ysize, UINT16 *pBuf)
{
#if (LCD_PIXEL == LCD_PIXEL_16)
    {
        LCD_DrawBmp(x, y, x + xsize - 1,y + ysize - 1, 16, pBuf);
    }
#elif (LCD_PIXEL == LCD_PIXEL_1)
    {
        LCD_DrawBmp(x, y, xsize, ysize, 16, pBuf);
    }
#endif
}

/*
********************************************************************************
*  Function name :  void  RecordGetPicIfor(UINT16 pictureIDNum)
*  Author:          anzhiguo
*  Description:
*
*  Input:           pictureIDNum     :  picture resource ID.
*                   x, y ：             the coordinate to display picture.
*  Output:          null
*  Return:          null
*
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo       2009-2-24     Ver1.0       ORG
********************************************************************************
*/
_ATTR_RECORDWIN_CODE_
void RecordGetPicIfor(UINT16 pictureIDNum, PICTURE_INFO_STRUCT *pInfo)
{
    UINT32 i;
    PICTURE_INFO_STRUCT  picInfo;

    GetPictureInfoWithIDNum(pictureIDNum, &picInfo);//获取图片资源的结构信息

    pInfo->x          = picInfo.x;
    pInfo->y          = picInfo.y;
    pInfo->xSize      = picInfo.xSize;
    pInfo->ySize      = picInfo.ySize;
    pInfo->offsetAddr = picInfo.offsetAddr;
    pInfo->totalSize  = picInfo.totalSize;
}

/*
********************************************************************************
*  Function name :  DispPictureWithIDNumAndXY()
*  Author:          anzhiguo
*  Description:     display the picture by picture id and coordinate,the parameters that function setting
                    have any relation with parameter that is generated by tool.
*
*  Input:           pictureIDNum     :  picture resource ID.
*                   x, y ：             the coordinate to display picture.
*  Output:          null
*  Return:          null
*
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo       2009-2-24     Ver1.0       ORG
********************************************************************************
*/
_ATTR_RECORDWIN_CODE_
void RecordDispPictureWithIDNumAndXYoffset(UINT16 pictureIDNum, UINT16 xOffset, UINT16 yOffset)
{
    uint8 i;

    //find the buffer pointer by buffer postion that have picture in there.
    //if picture's size is different,then it need to change by the definition of the picture buffer.
    i = pictureIDNum - RECORD_PIC_FIRSTNUM;
    RecordDisplayPicture((RecCurTimePicInfo.x + xOffset), (RecCurTimePicInfo.y + yOffset), RecCurTimePicInfo.xSize, RecCurTimePicInfo.ySize, (uint16*)&RecordDispPicBuf[i][0]);
}
#endif


#ifdef _RECODE_CHAR_DIAPLAY_

/*
********************************************************************************
*  Function name :  RecordGetCharResourceData()
*  Author:          yangwenjie
*  Description:    get string(0~9,:) resource and save it to memory
*
*  Input:
*
*  Output:          null
*  Return:          null
*
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo       2009-2-24     Ver1.0       ORG
********************************************************************************
*/
_ATTR_RECORDWIN_CODE_
void  RecordGetStrResourceData(char Data, uint8 *pBuf)
{
    UINT16 i;
    UINT32   CharInNFAddr;

    if (LcdContext.TextFort == FONT_12x12)
    {

        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(Data) * 32);
        LcdGetResourceData(CharInNFAddr, pBuf, 24);

    }
    else
    {

        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(Data) * 33);
        LcdGetResourceData(CharInNFAddr, pBuf, 32);
    }

#if 0
    for (i=0;i<11;i++)
    {
        if (LcdContext.TextFort == FONT_12x12)
        {
            CharInNFAddr = Font12LogicAddress + (UINT32)((UINT32)(i+'0')<<5);
            LcdGetResourceData(CharInNFAddr, RecordDispCharBuf[i],24);

        }
        else
        {
            CharInNFAddr = Font16LogicAddress + (UINT32)((UINT32)(i+'0')*33);
            LcdGetResourceData(CharInNFAddr, RecordDispCharBuf[i],32);
        }
    }
#endif
}

/*
********************************************************************************
*  Function name :  RecordGetCharResourceData()
*  Author:          yangwenjie
*  Description:   display the string in memory(0~9,:)
*
*  Input:
*
*  Output:          null
*  Return:          null
*
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo       2009-2-24     Ver1.0       ORG
********************************************************************************
*/
_ATTR_RECORDWIN_CODE_
void RecordDisplayStrWithIDNum(UINT32 numberID,UINT16 x,UINT16 y)
{
    if (LcdContext.TextFort == FONT_12x12)
    {
        LCD_DrawBmp(x,y,AS_CHAR_XSIZE_12,AS_CHAR_YSIZE_12,1,(UINT16*)RecordDispCharBuf[numberID]);
    }
    else
    {
        LCD_DrawBmp(x,y,AS_CHAR_XSIZE_16,AS_CHAR_YSIZE_16,1,(UINT16*)RecordDispCharBuf[numberID]);
    }
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinIntInit(void)
  Author        : WangBo
  Description   : record interrupt initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_INIT_CODE_
void RecordWinIntInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinIntDeInit(void)
  Author        : WangBo
  Description   : record interrupt auti-initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_DEINIT_CODE_
void RecordWinIntDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinMsgInit(void)
  Author        : WangBo
  Description   : record message initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_INIT_CODE_
void RecordWinMsgInit(void)
{
    ClearMsg(MSG_MESSAGEBOX_DESTROY);
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinMsgDeInit(void)
  Author        : WangBo
  Description   : record message auti-initial


  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_DEINIT_CODE_
void RecordWinMsgDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinInit(void)
  Author        : WangBo
  Description   : record window initial.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_CODE_
void RecordWinInit(void *pArg)
{
    uint32 i;
    UINT16 FortTemp;

    RecordWinMsgInit();

    KeyReset();

    //DEBUG("### RecordWinInit In ###");

    if (((RECORD_WIN_ARG*)pArg)->RecordType == RECORD_TYPE_LINEIN1)
    {
        //add the I/O control of FM recording in here
        //...
    }
    else
    {
        //add the I/O control of MIC recording in here
        //...
    }

#ifdef _RECODE_IMAGE_DIAPLAY_
    RecordGetPicIfor(IMG_ID_RECORD_CURRTIME_BACK, &RecCurTimePicInfo);
    //RecordGetPictureResourceData(IMG_ID_RECORD_CURRTIME_BACK, &RecordDispPicBuf[0][0]);
#endif

#ifdef _RECODE_CHAR_DIAPLAY_
    FortTemp = LCD_SetCharSize(FONT_12x12);
    for (i = 0; i < 10; i++)
    {
        RecordGetStrResourceData('0' + i, &RecordDispCharBuf[i][0]);
    }

    RecordGetStrResourceData(':', &RecordDispCharBuf[i][0]);
    LCD_SetCharSize(FortTemp);
#endif

    REC_Flag = RECORD_PREPARE;

    RecordWinSvcStart(pArg);
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinDeInit(void)
  Author        : WangBo
  Description   : record window auti-initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_CODE_
void RecordWinDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RecordWinService(void)
  Author        : WangBo
  Description   : record window service function,it is used to window message handle and window service etc.
                  the order to handle message can refer the order of sending message,the question may happen if
                  the order is inproper.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_SERVICE_CODE_
UINT32 RecordWinService(void)
{
    UINT32 Retval = 0;
    TASK_ARG TaskArg;

    //the destination of setting the message is display the backgroud basic picture at frist when enter recording interface.
    //the recording file and total time will be got by backgroud service,as it need time to get disk capacity,
    //if display the all content at beginning.it will take more long time to enter recording interface.

    if (TRUE == GetMsg(MSG_RECORD_ENCODESTART))
    {
        RecordEncodeProc(MSG_RECORD_STOP,NULL);
        SendMsg(MSG_RECORD_SERVICESTART);   //start recording again
    }

    if (TRUE == GetMsg(MSG_RECORD_INITUI))
    {
        RecordEncodeProc(MSG_RECORD_INIT,NULL);
        SendMsg(MSG_RECORD_SERVICESTART);
        //return Retval;
    }

    if (TRUE == GetMsg(MSG_RECORD_SERVICESTART))
    {
        //reserved interface,select record type (wav mp3)
        RecordEncodeProc(MSG_RECORD_TYPE,NULL);
        RecordEncodeProc(MSG_RECORD_GETINFO,NULL);
        REC_Flag = RECORD_PREPARE;

        //return Retval;
    }

    if (TRUE == GetMsg(MSG_RECORD_STATUS_DISK_FULL))
    {
        //it can get the disk capacity after start RecordGetInfo,if disk is full,popup the messagebox to display message,
        //exit record interface, it take the same handle process if the disk full is found in recording.
        SendMsg(MSG_SYS_RESUME);
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_DISK_IS_FULL;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&RecordWin, &MessageBoxWin, &TaskArg);

        //return Retval;
    }

    //it fail to create or display file name.
    if (TRUE == GetMsg(MSG_RECORD_STATUS_GETFILE_FAIL) || TRUE == GetMsg(MSG_RECORD_STATUS_CREATEFILE_FAIL))
    {
        TaskArg.Message.TitleID    = SID_WARNING;
        TaskArg.Message.ContentID  = SID_CREAT_FILE_ERROR;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&RecordWin, &MessageBoxWin,&TaskArg);

        //return Retval;
    }

    //if fail to write file in recording
    if (TRUE == GetMsg(MSG_RECORD_STATUS_WRITEFILE_FAIL))
    {
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_WRITE_FILE_ERROR;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&RecordWin, &MessageBoxWin, &TaskArg);  //new mechanism to create window

        //return Retval;
    }

    if ((TRUE == GetMsg(MSG_MESSAGEBOX_DESTROY)))
    {
        RecordWinSvcStop();
        if(RecordExitTaskID == TASK_ID_MAINMENU)
        {
            RecordType = RECORD_TYPE_NULL;
            TaskArg.MainMenu.MenuID = MAINMENU_ID_RECORD;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);        //now is not in recording status,exit recording main window,return to MainMenu
        Retval = 1;
        }
        else
        {
            RecordType = RECORD_TYPE_NULL;
            TaskSwitch(RecordExitTaskID, NULL);
            Retval = 1;
        }
        //return Retval;
    }

    if (TRUE == GetMsg(MSG_RECORD_WAVENCODESTOP))
    {
        RecordWinSvcStop();
        if(RecordExitTaskID == TASK_ID_MAINMENU)
        {
            RecordType = RECORD_TYPE_NULL;
            TaskArg.MainMenu.MenuID = MAINMENU_ID_RECORD;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);        //now is not in recording status,exit recording main window,return to MainMenu
            Retval = 1;
        }
        else
        {
            RecordType = RECORD_TYPE_NULL;
            TaskSwitch(RecordExitTaskID, NULL);
            Retval = 1;
        }
    }

    return Retval;

    //it happen such as USB insert,low power,etc abort event ,handle these in here.

}

_ATTR_RECORDWIN_SERVICE_CODE_
void RecordDisplayTime(UINT16 Hour,UINT16 Min,UINT16 Sec,UINT16 mode)
{
    UINT16 TxtDrawMode;
    UINT16 DisplayBuf[10];

    if(Hour < 100)
    {
        DisplayBuf[0] = Hour/10+'0';
        DisplayBuf[1] = Hour%10+'0';
        DisplayBuf[2] = ':';
        DisplayBuf[3] = Min/10+'0';
        DisplayBuf[4] = Min%10+'0';
        DisplayBuf[5] = ':';
        DisplayBuf[6] = Sec/10+'0';
        DisplayBuf[7] = Sec%10+'0';
        DisplayBuf[8] = 0;
        DisplayBuf[9] = 0;
    }
    else    //hour >= 100
    {
        uint16 tmp;

        DisplayBuf[0] = Hour/100+'0';
        tmp = Hour/10;
        DisplayBuf[1] = tmp%10 +'0';

        DisplayBuf[2] = Hour%10+'0';
        DisplayBuf[3] = ':';
        DisplayBuf[4] = Min/10+'0';
        DisplayBuf[5] = Min%10+'0';
        DisplayBuf[6] = ':';
        DisplayBuf[7] = Sec/10+'0';
        DisplayBuf[8] = Sec%10+'0';
        DisplayBuf[9] = 0;
    }

    TxtDrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    if (mode==0)
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_CURRTIME_BACK,4,127);
        LCD_NFDispStringAt(/*2*/4, 127, DisplayBuf);
    }
    else
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_CURRTIME_BACK,64,127);

        if(Hour < 100)
            LCD_NFDispStringAt(81, 127, DisplayBuf);
        else // >= 100
            LCD_NFDispStringAt(75, 127, DisplayBuf);
    }

    LCD_SetTextMode(TxtDrawMode);
}

_ATTR_RECORDWIN_SERVICE_CODE_
void RecordDisplayQualityInfo(UINT32 sampleRate,UINT16 channel,UINT16 dataWidth)
{
    UINT16 mono[5] = {'M','o','n','o'};
    UINT16 stereo[7] = {'S','t','e','r','e','o'};
    UINT16 srBuf[7];
    UINT16 dwBuf[6];
    uint32 sr;

    sr = sampleRate / 1000;
    if(sr > 100)
    {
        uint16 tmp;

        srBuf[0] = sr/100+'0';
        tmp = sr/10;
        srBuf[1] = tmp%10 +'0';
        srBuf[2] = sr%10+'0';
        srBuf[3] = 'K';
        srBuf[4] = 'h';
        srBuf[5] = 'z';
        srBuf[6] = 0;
    }
    else if(sr > 10)
    {
        srBuf[0] = sr/10+'0';
        srBuf[1] = sr%10+'0';
        srBuf[2] = 'K';
        srBuf[3] = 'h';
        srBuf[4] = 'z';
        srBuf[5] = 0;
    }

    if(RECORD_DATAWIDTH_24BIT == dataWidth)
    {
        uint16 dw;
        dw = dataWidth + 1;
        dwBuf[0] = dw / 10 + '0';
        dwBuf[1] = dw % 10 + '0';
        dwBuf[2] = 'b';
        dwBuf[3] = 'i';
        dwBuf[4] = 't';
        dwBuf[5] = 0;
    }
    else if(RECORD_DATAWIDTH_16BIT == dataWidth)
    {
        uint16 dw;
        dw = dataWidth + 1;
        dwBuf[0] = dw / 10 + '0';
        dwBuf[1] = dw % 10 + '0';
        dwBuf[2] = 'b';
        dwBuf[3] = 'i';
        dwBuf[4] = 't';
        dwBuf[5] = 0;
    }

    LCD_NFDispStringAt(4, 148, srBuf);

    if(RECORD_CHANNEL_MONO == channel)
    {
        LCD_NFDispStringAt(48, 148, mono);
    }
    else if(RECORD_CHANNEL_STERO == channel)
    {
        LCD_NFDispStringAt(46, 148, stereo);
    }

    LCD_NFDispStringAt(100, 148, dwBuf);
}


/*
--------------------------------------------------------------------------------
  Function name : void RecordWinPaint(void)
  Author        : WangBo
  Description   : recording window display function,adopt new MSG mechanism

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_SERVICE_CODE_
void RecordWinPaint(void)
{
    uint8  j, k, uMinute, uSecond;
    uint16 i, uHour;
    UINT16 TempColor,TempBkColor,TempTxtMode,TempCharSize;
    UINT16 *pString;
    LCD_RECT    r;
    UINT16  DisplayBuffer[12];
    UINT32  RecordCurrTimerTmb;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    //to display all again when restore from backgroud light or the other window.
    if (TRUE == GetMsg(MSG_NEED_PAINT_ALL))
    {
        SendMsg(MSG_RECORDWIN_DISPLAY_ALL);
        SendMsg(MSG_RECORDWIN_DISPFLAG_RECQUALITY);
        SendMsg(MSG_RECORDWIN_DISPFLAG_TIME);
        SendMsg(MSG_RECORDWIN_DISPFLAG_FILENAME);
        //to send message by current recording status.
        if (REC_Flag == RECORD_PREPARE)
        {
            SendMsg(MSG_RECORD_STATUS_PREPARE);
        }
        else if (REC_Flag == RECORD_BEING)
        {
            SendMsg(MSG_RECORD_STATUS_PLAY);
        }
        else if (REC_Flag == RECORD_PAUSE)
        {
            SendMsg(MSG_RECORD_STATUS_PAUSE);
        }

        SendMsg(MSG_RECORDWIN_DISPFLAG_TOTALTIME);

    }

    //to backgroud record in other window,restore current recording status when enter recording window again.
    if (TRUE == GetMsg(MSG_RECORDWIN_DISPFLAG_REFRESH))
    {
        SendMsg(MSG_RECORDWIN_DISPLAY_ALL);
        SendMsg(MSG_RECORDWIN_DISPFLAG_RECQUALITY);
        SendMsg(MSG_RECORDWIN_DISPFLAG_TIME);
        SendMsg(MSG_RECORDWIN_DISPFLAG_FILENAME);
        //to send message by current recording status.
        if (REC_Flag == RECORD_PREPARE)
        {
            SendMsg(MSG_RECORD_STATUS_PREPARE);
        }
        else if (REC_Flag == RECORD_BEING)
        {
            SendMsg(MSG_RECORD_STATUS_PLAY);
        }
        else if (REC_Flag == RECORD_PAUSE)
        {
            SendMsg(MSG_RECORD_STATUS_PAUSE);
        }

        SendMsg(MSG_RECORDWIN_DISPFLAG_TOTALTIME);

    }

    if ( (TRUE == GetMsg(MSG_RECORDWIN_DISPLAY_ALL)))                             //initial interface,staic part display
    {
        DispPictureWithIDNum(IMG_ID_MIC_BACKGROUND);                             //display recording main background
        //DispPictureWithIDNum(IMG_ID_MUSIC_BATTERY01 + RecordBatteryLevel);      //display dynamicly bettary status
        if ((RECORD_TYPE_MIC_STERO == RecordType)
            || (RECORD_TYPE_MIC1_MONO == RecordType)
            || (RECORD_TYPE_MIC2_MONO == RecordType))
        {
            DisplayMenuStrWithIDNum(0,19,128,12,LCD_TEXTALIGN_CENTER,SID_MIC_REC); 		//display title, MIC REC
        }
        else if((RECORD_TYPE_LINEIN1 == RecordType)
            || (RECORD_TYPE_LINEIN2 == RecordType))
        {
            DisplayMenuStrWithIDNum(0,19,128,12,LCD_TEXTALIGN_CENTER,SID_FM_REC); 		//display title MIC REC
        }
        //SendMsg(MSG_RECORDWIN_DISPFLAG_BATT);
        //SendMsg(MSG_RECORDWIN_DISPFLAG_HOLD);
        //SendMsg(MSG_RECORDWIN_DISPFLAG_VOL);
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNum(IMG_ID_MUSIC_BATTERY01 + RecordBatteryLevel);      //display battery status dynamically
    }


    if (TRUE == GetMsg(MSG_RECORDWIN_DISPFLAG_VOL))
    {
        //DispPictureWithIDNumAndXYoffset(IMG_ID_RECORD_VOL4,0,0);                  //display small speaker, display volume dynamically
    }

    if (TRUE == GetMsg(MSG_RECORDWIN_DISPFLAG_HOLD))
    {
        //DispPictureWithIDNum(IMG_ID_RECORD_HOLDOFF);                              //HOLD 显示
    }

    if (TRUE == GetMsg(MSG_RECORD_STATUS_PREPARE))
    {   //准备录音
        DisplayPicture_part(IMG_ID_MIC_BACKGROUND,0,0,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_HEIGHT);
        DisplayMenuStrWithIDNum(STR_RECORD_STATUS_X,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_WIDTH,
                                STR_RECORD_STATUS_HEIGHT,LCD_TEXTALIGN_CENTER,SID_RECORD_PREPARE);
    }

    if (TRUE == GetMsg(MSG_RECORD_STATUS_PLAY))
    {   //正在录音
        DisplayPicture_part(IMG_ID_MIC_BACKGROUND,0,0,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_HEIGHT);
        DisplayMenuStrWithIDNum(STR_RECORD_STATUS_X,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_WIDTH,
                                STR_RECORD_STATUS_HEIGHT,LCD_TEXTALIGN_CENTER,SID_RECORDING);
    }

    if (TRUE == GetMsg(MSG_RECORD_STATUS_PAUSE))
    { //暂停录音
        DisplayPicture_part(IMG_ID_MIC_BACKGROUND,0,0,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_HEIGHT);
        DisplayMenuStrWithIDNum(STR_RECORD_STATUS_X,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_WIDTH,
                                STR_RECORD_STATUS_HEIGHT,LCD_TEXTALIGN_CENTER,SID_RECORD_PAUSE);
    }

    if (TRUE == GetMsg(MSG_RECORD_STATUS_SAVE))
    { //保存录音文件
        DisplayPicture_part(IMG_ID_MIC_BACKGROUND,0,0,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_HEIGHT);
        DisplayMenuStrWithIDNum(STR_RECORD_STATUS_X,STR_RECORD_STATUS_Y,STR_RECORD_STATUS_WIDTH,
                                STR_RECORD_STATUS_HEIGHT,LCD_TEXTALIGN_CENTER,SID_RECORD_SAVE_FILE);
    }

    if (TRUE == GetMsg(MSG_RECORDWIN_DISPFLAG_TIME))
    {
        if(RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
        {
            RecordCurrTimerTmb = (RecordFileOffset) / ((UINT32)RecordBitrate * 125);   //Bitrate / 8 * 1000 the other arithmetic record
        }
        else if(RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
        {
            if(RECORD_DATAWIDTH_24BIT == RecordDataWidth)
            {
                RecordCurrTimerTmb = RecordFileOffset  / ((UINT32)RecordSampleRate * RecordChannel * 3);
            }
            else
            {
                RecordCurrTimerTmb = RecordFileOffset / ((UINT32)RecordSampleRate * RecordChannel * 2);
            }
        }
        else if(RecordEncodeType == RECORD_ENCODE_TYPE_MP3)
        {
            RecordCurrTimerTmb = (RecordFileOffset) / ((UINT32)RecordBitrate * 125);   //Bitrate / 8 * 1000 the other arithmetic record
        }

        RecordRemainTime = RecordTotalTime - RecordCurrTimerTmb;                  //record total remain time

        if ((RecordCurrTimerTmb != RecordCurrTime))
        {
            RecordCurrTime = RecordCurrTimerTmb;
            GetTimeHMS(RecordCurrTime,&i,&j,&k);
            RecordDisplayTime(i,j,k,0);

            GetTimeHMS(RecordRemainTime, &i, &j, &k);         //get RecordRemainTime
            RecordDisplayTime(i,j,k,1);
        }
    }

    if (TRUE == GetMsg(MSG_RECORDWIN_DISPFLAG_FILENAME))
    {
        // display file name
        r.x0    = STR_RECORD_FILENAME_LEFT_X;
        r.y0    = STR_RECORD_FILENAME_LEFT_Y;
        r.x1    = STR_RECORD_FILENAME_RIGHT_X;
        r.y1    = STR_RECORD_FILENAME_RIGHT_Y;
        pString = RecordLongFileName;
        DisplayPicture_part(IMG_ID_MIC_BACKGROUND,0,0,STR_RECORD_FILENAME_LEFT_Y,16);
        LCD_DispStringInRect(&r,&r, pString, LCD_TEXTALIGN_CENTER);
    }

    if  (TRUE == GetMsg(MSG_RECORDWIN_DISPFLAG_RECQUALITY))
    {
        // DispPictureWithIDNum(IMG_ID_RECORD_QUALITYHIGH + gbRecordQuality);      //display record quality, it can be get from setting menu or system backup.
        RecordDisplayQualityInfo(RecordSampleRate,RecordChannel,RecordDataWidth);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RecordWinKeyProc(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-10          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_SERVICE_CODE_
UINT32 RecordWinKeyProc(void)
{
    UINT32 Retval = 0;
    UINT32 RecordKeyVal;
    TASK_ARG TaskArg;

    RecordKeyVal =  GetKeyVal();
    /* lml
        //5 key function modification
        if (gSysConfig.KeyNum == KEY_NUM_5)
        {
        }

        //6 key function modification
        if (gSysConfig.KeyNum == KEY_NUM_6)
        {
        }
    */
    //7 key function
    switch (RecordKeyVal)
    {
        case KEY_VAL_PLAY_SHORT_UP:                                  //both play and pause share
            {
                //DEBUG("REC_Flag=%d\n", REC_Flag);
                if (REC_Flag == RECORD_PREPARE)                      //status from ready turn to recording
                {
                    RecordEncodeProc(MSG_RECORD_STARTE,NULL);        //build file in disk,start timer interrupt,start AD sampling,trnsfer data.
                    REC_Flag = RECORD_BEING;
                    SendMsg(MSG_RECORD_STATUS_PLAY);
                    SendMsg(MSG_RECORDWIN_DISPFLAG_REFRESH);
                    //printf("MSG_RECORD_STATUS_PLAY\n");
                    break;
                }
                else if (REC_Flag == RECORD_BEING)                    //status turn pause from reording that is stop sampling
                {
                    RecordEncodeProc(MSG_RECORD_PAUSE,NULL);          //close timer interrupt,stop sampling.
                    REC_Flag = RECORD_PAUSE;
                    SendMsg(MSG_RECORD_STATUS_PAUSE);
                    break;
                }
                else if (REC_Flag == RECORD_PAUSE)                    //status from pause turn to recording
                {
                    RecordEncodeProc(MSG_RECORD_RESUME,NULL);         //start timer interrupt,receive sampling data continue
                    REC_Flag = RECORD_BEING;
                    SendMsg(MSG_RECORD_STATUS_PLAY);
                    break;
                }
            }

        case KEY_VAL_ESC_SHORT_UP:
            {
                if ((REC_Flag == RECORD_BEING) || (REC_Flag == RECORD_PAUSE))
                {
                    //it need to save record file,frist display save recording file at frist,then take handle to
                    //WaveEncodeStop,exit recording main window.
                    //SendMsg(MSG_RECORD_STATUS_SAVE);    //UI display save file string.
                    SendMsg(MSG_RECORD_WAVENCODESTOP);  //send service message,to stop coding and sampling
                }
                else
                {
                    RecordWinSvcStop();
                    if(RecordExitTaskID == TASK_ID_MAINMENU)
                    {
                        RecordType = RECORD_TYPE_NULL;
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_RECORD;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);//current is in recording status,it exit the recording main interface to MainMenu,
                        Retval = 1;
                    }
                    else
                    {
                        RecordType = RECORD_TYPE_NULL;
                        TaskSwitch(RecordExitTaskID, NULL);
                        Retval = 1;
                    }
                }
            }
            break;

        case KEY_VAL_MENU_SHORT_UP:              //STOP,stop recording ,start next recording.
            {
                if ((REC_Flag == RECORD_BEING) || (REC_Flag == RECORD_PAUSE))
                {
                    SendMsg(MSG_RECORD_STATUS_SAVE);   //UI display save file string.
                    RecordEncodeProc(MSG_RECORD_STOP,NULL);
                }
                SendMsg(MSG_RECORD_SERVICESTART);   //start recording again
            }
            break;

            //add hold handle.
        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&RecordWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&RecordWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (Retval);
}


/*
********************************************************************************
*
*                         End of Recordwin.c
*
********************************************************************************
*/
#endif



