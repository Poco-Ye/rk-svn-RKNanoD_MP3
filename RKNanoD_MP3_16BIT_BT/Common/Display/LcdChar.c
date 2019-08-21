/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   LcdChar.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
//******************************************************************************
#define   _IN_LCDCHAR_
#include "FsInclude.h"
#include "LcdInclude.h"
#include "ModuleOverlay.h"

/*
--------------------------------------------------------------------------------
  Function name : void LCD_NFDispUnicodeChar(UINT32   c)
  Author        : yangwenjie
  Description   : charactor display

  Input        c: the charactor will be displayed.
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#include "Fontlib.h"
_ATTR_LCD_CODE_
uint16 * LCD_NFDispUnicodeChar(uint16 *pUnic)
{
    INT16    x,y,xsize,ysize;
    UINT32   i, D_mode, Old_mode;
    UINT16  *p, *pbuf;
    struct FontInfo Info;

    D_mode   = LcdContext.TextMode;
    Old_mode = LcdContext.DrawMode;
    LCD_SetDrawMode(D_mode);

    x        =  LcdContext.DispPosX;
    y        =  LcdContext.DispPosY;

    //printf("pUnicode = %x\n", *pUnic);

    p = getFontData(TYPE12_CJK, pUnic, &Info, MODE_ALLDATA);

    xsize = Info.width;
    ysize = Info.height;
    pbuf = (UINT16*)(Info.buff);

    if ((ScrollString.ShiftFlag == 1) && (ScrollString.FirstCharShiftBits != 0))
    {
        for (i = 0; i < Info.height; i++)
        {
            *pbuf = *pbuf << ScrollString.FirstCharShiftBits;
            pbuf ++;
        }
        xsize -= ScrollString.FirstCharShiftBits;
        ScrollString.FirstCharShiftBits = 0;
    }

    if ((ScrollString.ShiftFlag == 1) && (ScrollString.LastCharFlag != 0) && (ScrollString.LastCharShiftBits != 0))
    {
        xsize = ScrollString.LastCharShiftBits;
        ScrollString.LastCharFlag      = 0;
        ScrollString.LastCharShiftBits = 0;
    }

    LCD_DrawBmp(x, y, xsize, ysize, 1,(UINT16*)(Info.buff)); //+4 because one character lattice data
                                                                //first 4 bytes is width and height

    if((LcdContext.XDist > 0) || (LcdContext.YDist > 0))
    {
        if(D_mode != LCD_DRAWMODE_TRANS)
        {
            LCD_SetDrawMode(D_mode ^ LCD_DRAWMODE_REV);
            LCD_FillRect(x+xsize, y, x+xsize+LcdContext.XDist, y+ysize+LcdContext.YDist);
            LCD_FillRect(x, y+ysize, x+xsize,   y+ysize+LcdContext.YDist);
        }
    }

    LCD_SetDrawMode(Old_mode);

    if(LcdContext.TextMode != LCD_DRAWMODE_CIR90)
        LcdContext.DispPosX += (xsize + LcdContext.XDist);
    else
        LcdContext.DispPosY += (xsize + LcdContext.YDist);

    return p;

}

/*
--------------------------------------------------------------------------------
  Function name : INT32 LCD_GetCharXSize(UINT32 s)
  Author        : yangwenjie
  Description   : get the width of one charactor

  Input       s : charactor
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetCharXSize(UINT16 s)
{

    UINT16   iWidth;
    UINT16  stemp[2];

    struct FontInfo Info;

    stemp[0] = s;
    stemp[1] = 0;

    getFontData(TYPE12_CJK, stemp, &Info, MODE_SIZEONLY);

    return Info.width;

}

/*
--------------------------------------------------------------------------------
  Function name : INT32 LCD_GetCharsPerLine(LCD_RECT *r, UINT32   *s)
  Author        : yangwenjie
  Description   : compute the total number of charactors,these can display in one line of rect.

  Input         :   r   :  the rect that is used fo display charactors.
                    s   :  the pointer of string.
  Return        :   width of charactors.

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetCharsPerLine(LCD_RECT *r, UINT16 *s)
{
    INT32 NumChar;
    INT32 StrDistX;
    UINT16 CharSize;
    UINT16 temp;

    NumChar  = 0;
    StrDistX = 0;

    ScrollString.LastCharFlag = 0;
    while(*s)
    {
        if((*s == '\n') || (*s == '\r') || (*s == 0))
        {
            break;
        }

        CharSize = LCD_GetCharXSize(*s);
        StrDistX += (LcdContext.XDist + CharSize);

        if(StrDistX > (r->x1 - r->x0))
        {
            if ((ScrollString.ShiftFlag == 1)/* && (ScrollString.XShiftBits != 0)*/)
            {
                temp = (r->x1 - r->x0) - (StrDistX - CharSize) + ScrollString.XShiftBits;

                while(1)
                {
                    if (temp <= CharSize)
                    {
                        if (temp != 0)
                            NumChar++;

                        ScrollString.LastCharFlag = temp;
                        return(NumChar);
                    }
                    else //if (temp > CharSize)
                    {
                        temp -= CharSize;
                        NumChar++;
                        s++;
                        if((*s == '\n') || (*s == '\r') || (*s == 0))
                        {
                            ScrollString.LastCharFlag = 0;
                            return(NumChar);
                        }
                        CharSize = LCD_GetCharXSize(*s);
                    }
                }
            }
            break;
        }

        NumChar++;
        s++;

    }
    return(NumChar);
}


/*
--------------------------------------------------------------------------------
  Function name : INT32   LCD_GetLineDistX(UINT32   * s,    INT32 LineNumChar)
  Author        : yangwenjie
  Description   : get the width of charactors according to number of charactors.更据字符数计算字符的宽度

  Input         :  s                :   string pointer.
                   LineNumChar      :   charactors number
  Return        :  width of charactors.

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetLineDistX(UINT16 * s,    UINT32 LineNumChar)
{
    INT32 DistX;

    DistX = 0;
    for(;LineNumChar > 0; LineNumChar--)
    {
        DistX += (LcdContext.XDist + LCD_GetCharXSize(*s++));
    }

    return(DistX);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_DispLineChar(UINT32 *s, INT32 LineNumChar)

  Author        : yangwenjie
  Description   : display string.

  Input         :  s            :   the string pointer.
                   LineNumChar  :   charactor number.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
 _ATTR_LCD_CODE_
void LCD_DispLineChar(UINT16 *s, UINT32 LineNumChar)
{

    uint16 * p;
    uint16 i, CharSize;

    i = 0;
    do
    {

        if ((i == 0) && (ScrollString.ShiftFlag == 1))
        {
            ScrollString.FirstCharShiftBits = ScrollString.XShiftBits;
        }
        if ((i == (LineNumChar-1)) && (ScrollString.ShiftFlag == 1) && (ScrollString.LastCharFlag != 0))
        {
            ScrollString.LastCharShiftBits = ScrollString.LastCharFlag;
        }

        p = s;
        s = LCD_NFDispUnicodeChar(s);
        i += (s-p);

    }while(i < LineNumChar);

}

/*
--------------------------------------------------------------------------------
  Function name : INT32   LCD_GetStringSize(UINT32 *s)
  Author        : yangwenjie
  Description   : get the width of the string.

  Input         :  s       :   string pointer.

  Return        :  string width.

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT32 LCD_GetStringSize(UINT16 *s)
{
    UINT32    CharSize = 0;

    while(*s)
    {
        CharSize += (LcdContext.XDist + LCD_GetCharXSize(*s++));
    }
    return(CharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_NFDispString(UINT8 *pStr)
  Author        : yangwenjie
  Description   : string display.

  Input         : pStr :string pointer.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_NFDispString(UINT16 *pStr)
{
    UINT32     StringSize = 0;
    INT32      xAdjust    = 0;
    INT32      Old_DispPosX;

    Old_DispPosX = LcdContext.DispPosX;

    StringSize = LCD_GetStringSize(pStr);
    switch(LcdContext.TextAlign & LCD_TEXTALIGN_HORIZONTAL) {

        case LCD_TEXTALIGN_RIGHT:
            xAdjust = StringSize;
            break;
        case LCD_TEXTALIGN_CENTER:
            xAdjust = StringSize/2;
            break;
        default:
            xAdjust = 0;
    }
    LcdContext.DispPosX -= xAdjust;

    while(*pStr)
    {
        if(*pStr == '\n')
        {
            switch(LcdContext.TextAlign & LCD_TEXTALIGN_HORIZONTAL)
            {
                case LCD_TEXTALIGN_RIGHT:
                case LCD_TEXTALIGN_CENTER:
                    LcdContext.DispPosX = Old_DispPosX;
                    break;
                default :
                    LcdContext.DispPosX = LcdContext.LBorder;
                    break;
            }
            if(LcdContext.TextFort == FONT_12x12)
            {
                LcdContext.DispPosY += (LcdContext.YDist + CH_CHAR_YSIZE_12);
            }
            else //if(LcdContext.TextFort == FONT_16x16)
            {
                LcdContext.DispPosY += (LcdContext.YDist + CH_CHAR_YSIZE_16);
            }
        }
        else
        {
            pStr = LCD_NFDispUnicodeChar(pStr);
            continue;
        }
        pStr++;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void LCD_NFDispStringAt(INT32   x, INT32 y,   UINT32 *pStr)
  Author        : yangwenjie
  Description   : display string in specified postion.

  Input         :   x,y  : coordinate of display string.
                    pStr : string pointer.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_NFDispStringAt(UINT32 x, UINT32 y, UINT16 *pStr)
{
    UINT32 bIsBidirectionalString;
    UINT16 StringBuffer[2*TXT_PER_LINE_CHARS];

    LcdContext.DispPosX = x;
    LcdContext.DispPosY = y;
    if(LcdContext.TextLanguage == LANGUAGE_RABBINIC)
    {
        bIsBidirectionalString = IsBidirectionalString( pStr);
        if (1 == bIsBidirectionalString)
        {
            //ReverseBidirectionalString(pStr, StringBuffer,(TXT_PER_LINE_CHARS - 1));
            BidirectionalStringPro(pStr, StringBuffer);
            LCD_NFDispString(StringBuffer);

            return;
        }
    }
    LCD_NFDispString(pStr);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_DispStringInRect(LCD_RECT *pDr, LCD_RECT *pSr,   UINT32 *pStr, UINT32 AlignMode)
  Author        : yangwenjie
  Description   : display string in specified postion.

  Input         :   pDr         : target area.
                    pSr         : source area.
                    pStr        : string pointer.
                    AlignMode   : align mode.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_DispStringLine(LCD_RECT *pDr, LCD_RECT   *pSr,   UINT16 *pStr, UINT32 AlignMode,UINT8 YSymbol)
{

     LCD_RECT    old_r;
     INT16       NumChar;
     INT16       xDist;
     INT16       old_DispPosX;
     INT16       xAdjust;
     UINT16      bIsBidirectionalString;
     UINT16      StringBuffer[TXT_PER_LINE_CHARS*2];
     UINT16      *pString;

     pString=pStr;

     old_r= LcdContext.ClipRect;


     LcdContext.ClipRect = *pDr;

     if(YSymbol)
     {

        if(LcdContext.TextFort == FONT_12x12)
        {
           LcdContext.DispPosY =(pSr->y1-pSr->y0-CH_CHAR_YSIZE_12)/2 + pSr->y0;
        }
        else //if(LcdContext.TextFort == FONT_16x16)
        {
           LcdContext.DispPosY =(pSr->y1-pSr->y0-CH_CHAR_YSIZE_16)/2 + pSr->y0;
        }
     }
     else
     {
        LcdContext.DispPosY = pSr->y0;
     }

     bIsBidirectionalString = IsBidirectionalString (pStr);

     if (1 == bIsBidirectionalString)
     {
         if(IsSetmenu == FALSE)
         {
             //if (LCD_TEXTALIGN_LEFT == AlignMode)
             //{
                 BidirectionalStringPro(pStr,StringBuffer);
                 //AlignMode   = LCD_TEXTALIGN_RIGHT;
                 pString = StringBuffer;

             //}
         }
         else
         {
             bIsBidirectionalString=0;
             IsSetmenu = FALSE;
         }
     }

     switch(AlignMode & LCD_TEXTALIGN_HORIZONTAL)
     {
         case LCD_TEXTALIGN_RIGHT:
             LcdContext.DispPosX = pSr->x1;
             break;
         case LCD_TEXTALIGN_CENTER:
             LcdContext.DispPosX = pSr->x0 + (pSr->x1 - pSr->x0)/2;
             break;
         default:
             LcdContext.DispPosX = pSr->x0;
             break;
     }

     old_DispPosX = LcdContext.DispPosX;

     NumChar =   LCD_GetCharsPerLine(pSr, pString);//更据字符宽度计算出在矩形中一行能显示的字符数
     xDist   =   LCD_GetLineDistX(pString, NumChar);//更据字符数计算字符的宽度

     switch(AlignMode & LCD_TEXTALIGN_HORIZONTAL)
     {
         case LCD_TEXTALIGN_RIGHT:
             xAdjust =   xDist;
             break;
         case LCD_TEXTALIGN_CENTER:
             xAdjust =   xDist/2;
             break;
         default:
             xAdjust =   0;
             break;
     }
     LcdContext.DispPosX -= xAdjust;

     LCD_DispLineChar(pString, NumChar);

     LcdContext.DispPosX  =  old_DispPosX;
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_DispStringInRect(LCD_RECT *pDr, LCD_RECT *pSr,   UINT32 *pStr, UINT32 AlignMode)
  Author        : yangwenjie
  Description   : display string in specified postion.

  Input         :   pDr         : target area.
                    pSr         : source area.
                    pStr        : string pointer.
                    AlignMode   : align mode.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_DispStringInRect(LCD_RECT *pDr, LCD_RECT   *pSr,   UINT16 *pStr, UINT32 AlignMode)
{
    LCD_RECT    old_r;
    INT16       NumChar;
    INT16       xDist;
    INT16       old_DispPosX;
    INT16       xAdjust;
    UINT16      bIsBidirectionalString;
    UINT16      StringBuffer[TXT_PER_LINE_CHARS*2];
    UINT16      *pString;
    UINT16      CharYSize;

    pString = pStr;

    old_r = LcdContext.ClipRect;

    LcdContext.ClipRect = *pDr;

    LcdContext.DispPosY = pSr->y0 + LcdContext.YDist;

    bIsBidirectionalString = IsBidirectionalString(pStr);
    if (1 == bIsBidirectionalString)
    {
        if (IsSetmenu == FALSE)
        {
            //if (LCD_TEXTALIGN_LEFT == AlignMode)
            //{
                BidirectionalStringPro(pStr, StringBuffer);
                //AlignMode   = LCD_TEXTALIGN_RIGHT;
                pString = StringBuffer;
            //}
        }
        else
        {
            bIsBidirectionalString=0;
            IsSetmenu = FALSE;
        }
    }

    switch (AlignMode & LCD_TEXTALIGN_HORIZONTAL)
    {
        case LCD_TEXTALIGN_RIGHT:
            LcdContext.DispPosX = pSr->x1;
            break;

        case LCD_TEXTALIGN_CENTER:
            LcdContext.DispPosX = pSr->x0 + ((pSr->x1 - pSr->x0) >> 1);
            break;

        default:
            LcdContext.DispPosX = pSr->x0;
            break;
    }
    LcdContext.DispPosX += LcdContext.XDist;
    old_DispPosX = LcdContext.DispPosX;

    while(*pString)
    {
        NumChar = LCD_GetCharsPerLine(pSr, pString);//更据字符宽度计算出在矩形中一行能显示的字符数
        xDist   = LCD_GetLineDistX(pString, NumChar);//更据字符数计算字符的宽度

        switch(AlignMode & LCD_TEXTALIGN_HORIZONTAL)
        {
            case LCD_TEXTALIGN_RIGHT:
                xAdjust = xDist;
                break;

            case LCD_TEXTALIGN_CENTER:
                xAdjust = (xDist >> 1);
                break;

            default:
                xAdjust = 0;
                break;
        }
        LcdContext.DispPosX -= xAdjust;

        if (*pString >= 0x20)        //zyz:不可见字符不显示zyz
        {
            LCD_DispLineChar(pString, NumChar);
        }
        pString += NumChar;
        if (*pString == '\n' )
        {
            pString++;
            //break;        //if comment this line code ,system info display error.
        }

        LcdContext.DispPosX = old_DispPosX;

        if (LcdContext.TextFort == FONT_12x12)
        {
            CharYSize = CH_CHAR_YSIZE_12;
        }
        else //if(LcdContext.TextFort == FONT_16x16)
        {
            CharYSize = CharYSize = CH_CHAR_YSIZE_12;;
        }

        LcdContext.DispPosY += CharYSize;

        /* check next line hight is enough, if not break. */
        if (LcdContext.DispPosY + CharYSize > LcdContext.ClipRect.y1)
        {
            break;
        }
    }

    LcdContext.ClipRect = old_r;
}

/*
--------------------------------------------------------------------------------
  Function name : void ScrollStringForMusic(UINT16 *pstr )
  Author        : yangwenjie
  Description   : audio string display as a rolling mode.

  Input         : r            :  display area.
                  pstr         :  diaplay string pointer.
                  PictureInfo  :  the information of display image.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void ScrollStringForMusic(UINT16 *pstr )
{
    UINT16    *pCurternStr,*pCurternStr1;
    UINT16     TextMode;
    UINT16     Count;
    struct FontInfo Info;


    if(IsBidirectionalString(pstr))
    {
      return;
    }

    Count = SysTickCounter - ScrollString.SystickCounterBack;
    if(Count > ScrollString.ScrollSpeed)
    {
        if((ScrollString.DiplayRect.x0 > ScrollString.StartX))
        {
            pCurternStr = pstr;
            if((ScrollString.DiplayRect.x0-ScrollString.StartX)<ScrollString.TextFort)
            {
               ScrollString.DiplayRect.x0 = ScrollString.StartX;
            }
            else
            {
                ScrollString.DiplayRect.x0 = ScrollString.StartX + ScrollString.DiplayRect.x1- ScrollString.ScrollNumber*ScrollString.TextFort;//ScrollString.CharSizePerLine - ScrollString.ScrollNumber*ScrollString.TextFort;
            }
        }
        else
        {
            if(ScrollString.TotalCharNum >= ScrollString.CharNumber)
            {
                pCurternStr = pstr + ScrollString.CharNumber;

                 pCurternStr1 = getFontData(TYPE12_CJK, pCurternStr, &Info, MODE_SIZEONLY);
                 ScrollString.CharNumber += (pCurternStr1 - pCurternStr);
            }
            else
            {
               ScrollString.ScrollNumber = 1;
               ScrollString.DiplayRect.x0 = ScrollString.StartX +
               ScrollString.DiplayRect.x1-ScrollString.ScrollNumber*ScrollString.TextFort;//ScrollString.CharSizePerLine-ScrollString.ScrollNumber*ScrollString.TextFort;
               pCurternStr = pstr;
               ScrollString.CharNumber=1;
            }
        }

        #if (LCD_PIXEL == LCD_PIXEL_1)
        if(ScrollString.pictureID == NULL)
        {
             LCD_ClrRect(ScrollString.DiplayRect.x0,ScrollString.DiplayRect.y0,ScrollString.DiplayRect.x1,ScrollString.DiplayRect.y1);
        }
        else
        #endif
        {
            DisplayPicture_part(ScrollString.pictureID,ScrollString.pictureInfo.x,ScrollString.pictureInfo.y,ScrollString.pictureInfo.yoffset,ScrollString.pictureInfo.ysize);
        }
        LCD_DispStringInRect(&(ScrollString.DiplayRect),&(ScrollString.DiplayRect), pCurternStr,LCD_TEXTALIGN_LEFT);
        ScrollString.ScrollNumber++;
        ScrollString.SystickCounterBack = SysTickCounter;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void ScrollStringCommon(UINT16 *pstr )
  Author        : yangwenjie
  Description   : audio string display as a rolling mode.

  Input         : r            :  display area.
                  pstr         :  diaplay string pointer.
                  PictureInfo  :  the information of display image.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void ScrollStringCommon(UINT16 *pstr )
{
    UINT16     TextMode;
    UINT16     Count;
    struct FontInfo Info;

    if (IsBidirectionalString(pstr))
    {
        return;
    }

    Count = SysTickCounter -  ScrollString.SystickCounterBack;

    if (Count >= ScrollString.ScrollSpeed)
    {
        if(ScrollString.TotalCharNum > ScrollString.CharNumber)
        {
            if (ScrollString.XShiftBits == 0)
            {
                ScrollString.CharNumber += (ScrollString.pNextStr - ScrollString.pCurStr);

                ScrollString.pCurStr = pstr+ScrollString.CharNumber;
                ScrollString.pNextStr = getFontData(TYPE12_CJK, ScrollString.pCurStr, &Info, MODE_SIZEONLY);
                ScrollString.XShiftCharSize = Info.width;
            }
        }
        else
        {
            ScrollString.pCurStr  = pstr;
            ScrollString.pNextStr = getFontData(TYPE12_CJK, ScrollString.pCurStr, &Info, MODE_SIZEONLY);
            ScrollString.XShiftCharSize = Info.width;

            ScrollString.CharNumber = 0;
            ScrollString.XShiftBits = 0;
        }

#if (LCD_PIXEL == LCD_PIXEL_1)
        if (ScrollString.pictureID == NULL)
        {
            if (LcdContext.TextMode == LCD_DRAWMODE_REV)
            {
                LCDDEV_FillRect(ScrollString.DiplayRect.x0, ScrollString.DiplayRect.y0, ScrollString.DiplayRect.x1, ScrollString.DiplayRect.y1);
            }
            else
            {
                LCD_ClrRect(ScrollString.DiplayRect.x0, ScrollString.DiplayRect.y0, ScrollString.DiplayRect.x1, ScrollString.DiplayRect.y1);
            }
        }
        else
#endif
        {
            DisplayPicture_part(ScrollString.pictureID,ScrollString.pictureInfo.x,ScrollString.pictureInfo.y,ScrollString.pictureInfo.yoffset,ScrollString.pictureInfo.ysize);
        }

        ScrollString.ShiftFlag  = 1;
        LCD_DispStringInRect(&(ScrollString.DiplayRect), &(ScrollString.DiplayRect), ScrollString.pCurStr, LCD_TEXTALIGN_LEFT);
        ScrollString.ShiftFlag  = 0;

        ScrollString.XShiftBits++;
        if (ScrollString.XShiftBits >= ScrollString.XShiftCharSize)
        {
            ScrollString.XShiftBits = 0;
        }

        ScrollString.SystickCounterBack = SysTickCounter;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void void SetScrollStringInfo(LCD_RECT *r, PicturePartInfo PictureInfo,UINT16 *pstr,UINT16 Speed)
  Author        : yangwenjie
  Description   : set rolling string information.

  Input         : r            :  display area.
                  PictureInfo  :  display string pointer.
                  PictureInfo  :  display image's information.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void SetScrollStringInfo(LCD_RECT *r, PicturePartInfo PictureInfo,UINT16 *pstr,UINT16 Speed)
{
    UINT16 CharNumPerLine;

    ScrollString.TotalSize   =  LCD_GetStringSize(pstr);
    ScrollString.TotalCharNum  =   LCD_GetStringNum(pstr) ;
    ScrollString.pictureID   =  PictureInfo.pictureIDNump;
    CharNumPerLine = LCD_GetCharsPerLine(r,pstr);
    ScrollString.CharSizePerLine = LCD_GetLineDistX(pstr,CharNumPerLine);
    if(LcdContext.TextFort==FONT_12x12)
    {
        ScrollString.TextFort  = CH_CHAR_XSIZE_12;
    }
    else
    {
        ScrollString.TextFort  = CH_CHAR_XSIZE_16;
    }
    ScrollString.ScrollNumber   = 1;
    ScrollString.CharNumber     = 0;

    ScrollString.ShiftFlag      = 0;
    ScrollString.XShiftBits     = 0;
    ScrollString.XShiftCharSize = CH_CHAR_XSIZE_12;
    ScrollString.LastCharFlag   = 0;
    ScrollString.LastCharShiftBits  = 0;
    ScrollString.FirstCharShiftBits = 0;
    ScrollString.pNextStr       = NULL;
    ScrollString.pCurStr        = NULL;

    ScrollString.pictureInfo.x  = PictureInfo.x;
    ScrollString.pictureInfo.y  = PictureInfo.y;
    ScrollString.pictureInfo.yoffset  = PictureInfo.yoffset;
    ScrollString.pictureInfo.ysize    = PictureInfo.ysize;

    ScrollString.DiplayRect.x0  = r->x0;
    ScrollString.DiplayRect.y0  = r->y0;
    ScrollString.DiplayRect.x1  = r->x1;
    ScrollString.DiplayRect.y1  = r->y1;

    ScrollString.StartX  = ScrollString.DiplayRect.x0;
    ScrollString.SystickCounterBack = SysTickCounter;
    ScrollString.ScrollSpeed = Speed;
}
/*
--------------------------------------------------------------------------------
  Function name : void DisplayString(UINT32   x, UINT32 y, UINT32 xsize,
                                        UINT32 ysize, UINT32 mode, UINT32 flash_addr)
  Author        : yangwenjie
  Description   : get string resource from flash.

  Input         :x,y    :     the coordinate of display string.
                 xsize  :     the length of tring(the pixels number.)
                 ysize  :     the height of tring(the pixels number.)
                 mode   :     display mode
                 flash_addr : the address of string that is in flash.
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void DisplayString(UINT32 x, UINT32 y, UINT32 xsize, UINT32 ysize,  UINT32 mode, UINT16 TextID)
{
    LCD_RECT        r;
    UINT16          StringBuf[64];

   //GetResourceInfo(TextID,StringBuf,64);
    GetResourceStr(TextID,StringBuf,64);
    r.x0 = x;
    r.y0 = y;
    r.x1 = x+xsize-1;
    r.y1 = y+ysize-1;

    LCD_DispStringInRect(&r,&r,StringBuf, mode);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_SetCharSize(UINT16 size)
  Author        : yangwenjie
  Description   : set font size

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void LCD_SetCharSizeInit(void)
{
    uint8  FlashBuf[512];

    FIRMWARE_HEADER *pFWHead = (FIRMWARE_HEADER *)FlashBuf;

    MDReadData(SysDiskID, 0, 512,FlashBuf);
    Font16LogicAddress = pFWHead->FontInfo.FontInfoTbl[FONT16].ModuleOffset;
    Font12LogicAddress = pFWHead->FontInfo.FontInfoTbl[FONT12].ModuleOffset;
}

_ATTR_LCD_CODE_
UINT16 LCD_SetCharSize(UINT16 size)
{
    UINT16 TextFortBack;

    TextFortBack = LCD_TEXTFORT;

    if(LCD_TEXTFORT != size)
    {
        LCD_TEXTFORT = size;

        if (LCD_TEXTFORT == FONT_16x16)
        {
            FontLogicAddress = Font16LogicAddress;//pFWHead->FontInfo.FontInfoTbl[FONT16].ModuleOffset;
        }
        else
        {
            FontLogicAddress = Font12LogicAddress;//pFWHead->FontInfo.FontInfoTbl[FONT12].ModuleOffset;
        }
    }
    return TextFortBack;
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetColor(UINT16 color)
  Author        : ZHengYongzhi
  Description   : set front color.

  Input         : color
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#if 0
_ATTR_LCD_CODE_
void LCD_SetColor(UINT16 color)
{
    LCD_COLOR = color;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetColor(void)
  Author        : ZHengYongzhi
  Description   : 获取前景色

  Input         :
  Return        : 当前前景色

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetColor(void)
{
    return(LCD_COLOR);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetBkColor(UINT16 color)
  Author        : ZHengYongzhi
  Description   : 设置背景色

  Input         : color —— 背景色
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetBkColor(UINT16 color)
{
    LCD_BKCOLOR = color;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetBkColor(void)
  Author        : ZHengYongzhi
  Description   : 获取背景色

  Input         :
  Return        : 当前背景色

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetBkColor(void)
{
    return(LCD_BKCOLOR);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_SetDrawMode(UINT16 mode)
  Author        : ZHengYongzhi
  Description   : 设置系统绘图模式

  Input         : mode —— 新的绘图模式
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetDrawMode(UINT16 mode)
{
    LCD_DRAWMODE = mode;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetDrawMode(void)
  Author        : ZHengYongzhi
  Description   : 获取绘图模式

  Input         :
  Return        : 当前绘图模式

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetDrawMode(void)
{
    return(LCD_DRAWMODE);
}
#endif
/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetTextMode(UINT16 mode)
  Author        : ZHengYongzhi
  Description   : set text display mode.

  Input         : mode ——
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_SetTextMode(UINT16 mode)
{
    //LCD_TEXTMODE = mode;
    UINT16 mode_temp;

    #ifdef COLOR_LCD
    if (mode == LCD_DRAWMODE_REV)
    {
        mode = LCD_DRAWMODE_NORMAL;
    }
    else if (mode == LCD_DRAWMODE_NORMAL)
    {
        mode = LCD_DRAWMODE_REV;
    }
    #endif

    mode_temp = LcdContext.TextMode;
    LcdContext.TextMode = mode;

    return(mode_temp);

}

#if 0
/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetTextMode(void)
  Author        : ZHengYongzhi
  Description   : 获取文本显示模式

  Input         :
  Return        : 文本显示模式

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetTextMode(void)
{
    return(LCD_TEXTMODE);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetTextAlign(UINT16 mode)
  Author        : ZHengYongzhi
  Description   : 设置文本对齐模式

  Input         : mode —— 文本对齐模式
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetTextAlign(UINT16 mode)
{
    LCD_TEXTALIGN = mode;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetTextAlign(void)
  Author        : ZHengYongzhi
  Description   : 获取文本对齐模式

  Input         :
  Return        : 文本对齐模式

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetTextAlign(void)
{
    return(LCD_TEXTALIGN);
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetDiaplayMode(INT16 mode)
  Author        : ZHengYongzhi
  Description   : set display mode

  Input         : mode ——
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetDiaplayMode(INT16 mode)
{
    LCD_DISPMODE = mode;

    if(mode == LCD_MODE_0)
    {

        LcdContext.LcdMaxWidth = LCD_WIDTH;
        LcdContext.LcdMaxHeight = LCD_HEIGHT;

        #ifdef _FRAME_BUFFER_
        Lcd_SetFrameMode(0);
        #endif

    }
    else
    {

        LcdContext.LcdMaxWidth = LCD_HEIGHT;
        LcdContext.LcdMaxHeight = LCD_WIDTH;

        #ifdef _FRAME_BUFFER_
        Lcd_SetFrameMode(1);
        #endif

    }

    LcdContext.ClipRect.x0 = 0;
    LcdContext.ClipRect.y0 = 0;
    LcdContext.ClipRect.x1 = LcdContext.LcdMaxWidth - 1;
    LcdContext.ClipRect.y1 = LcdContext.LcdMaxHeight - 1;


}



/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetLanguage(INT16 mode)
  Author        : yangwenjie
  Description   : get and set current language.

  Input         :
  Return        : current language.

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT8 LCD_SetLanguage(UINT8 SysLanguage)
{
    UINT8 LanguageBack;

    LanguageBack =  Language;

    if(Language!=SysLanguage)
    {
       Language = SysLanguage;
    }

    return(LanguageBack);
}


/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetDispRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1)
  Author        : ZHengYongzhi
  Description   : set display area in lcd.

  Input         : x0,y0 —— the upper left coordinate.
                  x1,y1 —— the lower right coordinate.
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetDispRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1)
{
    LcdContext.ClipRect.x0 = x0;
    LcdContext.ClipRect.y0 = y0;

    if(LCD_DISPMODE == LCD_MODE_0){

        LcdContext.ClipRect.x1 = x1;
        LcdContext.ClipRect.y1 = y1;

    } else {

        LcdContext.ClipRect.x1 = y1;
        LcdContext.ClipRect.y1 = x1;

    }
}


/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetPixel(UINT16 x, UINT16 y, UINT16 color)
  Author        : ZHengYongzhi
  Description   : set display pixel

  Input         : x0,y0 —— the upper left coordinate
                  x1,y1 —— the lower right coordinate
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetPixel(UINT16 x, UINT16 y, UINT16 color)
{
    #if (LCD_PIXEL == LCD_PIXEL_16)
    {
        Lcd_SetWindow(x, y, x, y);
        Lcd_SendData (color);
    }
    #else
    {
        LCDDEV_SetPixel(x,y,color);
    }
    #endif
}

/*
--------------------------------------------------------------------------------
  Function name : void GetResourceStr(UINT16 menuTextID , UINT16 *pMenuStr ,UINT16 StrLen)
  Author        : yangwenjie
  Description   : get the resource of string.

  Input         : menuTextID：string ID
                  pMenuStr  ：the unicode code of charactor group.
                  StrLen    ：get the length of string.
  Return        : null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void GetResourceStr(UINT16 menuTextID , UINT16 *pMenuStr ,UINT16 StrLen)
{

    UINT32   CharInNFAddr;
    UINT32   languegeOffsetAddr;

    languegeOffsetAddr = MenuLogicAddress +Language*MENU_ITEM_LENGTH*TOTAL_MENU_ITEM+TOTAL_LANAUAGE_NUM*4+2;
    if(!gSysConfig.FMEnable)
    {
        languegeOffsetAddr = MenuLogicAddress +Language*MENU_ITEM_LENGTH*(TOTAL_MENU_ITEM - 9)+TOTAL_LANAUAGE_NUM*4+2;
    }

   //  LcdGetResourceInfo(MenuLogicAddress,(UINT8*)&languegeInfo,sizeof(languegeInfo));
    CharInNFAddr = languegeOffsetAddr + menuTextID*MENU_ITEM_LENGTH+MENU_CONTENT_OFFSET;
    LcdGetResourceInfo(CharInNFAddr, (UINT8*)pMenuStr, StrLen);

}
/*
--------------------------------------------------------------------------------
  Function name : void GetMenuTextInfoWithIDNum(UINT32 menuTextID,
                              MENU_TEXT_INFO_STRUCT *pMenuTextInfo)
  Author        : yangwenjie
  Description   : get menu resource structure information(string id number).it be use for muiti-country.

  Input         : menuTextID    ：menu text id.
                  pMenuTextInfo ：save menu resource text structrue information pointer.
  Return        : null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void GetMenuTextInfoWithIDNum(UINT32 menuTextID,
                              MENU_TEXT_INFO_STRUCT *pMenuTextInfo)
{
    UINT32   CharInNFAddr;
    UINT32   languegeOffsetAddr;

    languegeOffsetAddr = MenuLogicAddress +Language*MENU_ITEM_LENGTH*(TOTAL_MENU_ITEM-9)+TOTAL_LANAUAGE_NUM*4+2;
    if(gSysConfig.FMEnable)
    {
        languegeOffsetAddr = MenuLogicAddress + Language*MENU_ITEM_LENGTH*(TOTAL_MENU_ITEM)+TOTAL_LANAUAGE_NUM*4+2;
    }
    CharInNFAddr = languegeOffsetAddr + menuTextID * MENU_ITEM_LENGTH;
    LcdGetResourceInfo(CharInNFAddr, (UINT8*)pMenuTextInfo, sizeof(MENU_TEXT_INFO_STRUCT));
}

/*
--------------------------------------------------------------------------------
  Function name : void DisplayMenuStrWithIDNum(UINT16 x, UINT16 y,
                             UINT16 xsize, UINT16 ysize,
                             UINT16 alignMode, UINT16 menuTextID)
  Author        : yangwenjie
  Description   : display menu resource text.(use text id)

  Input         : x,y        : coordinate of display string.
                  xsize      : string length(pixels)
                  ysize      : string height(pixels)
                  alignMode  : align mode
                  menuTextID ：menu text id.
  Return        : null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void DisplayMenuStrWithIDNum(UINT16 x, UINT16 y,
                             UINT16 xsize, UINT16 ysize,
                             UINT16 alignMode, UINT16 menuTextID)
{
    UINT16  tempBuf[((MENU_ITEM_LENGTH - MENU_CONTENT_OFFSET) >> 1)];
    LCD_RECT      r;

    GetResourceStr(menuTextID,tempBuf,(UINT16)((MENU_ITEM_LENGTH - MENU_CONTENT_OFFSET) >> 1));
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + xsize - 1;
    r.y1 = y + ysize - 1;

    IsSetmenu = TRUE;

    LCD_DispStringInRect(&r,&r,tempBuf, alignMode);

}

/*
********************************************************************************
*
*                         End of LcdChar.c
*
********************************************************************************
*/


