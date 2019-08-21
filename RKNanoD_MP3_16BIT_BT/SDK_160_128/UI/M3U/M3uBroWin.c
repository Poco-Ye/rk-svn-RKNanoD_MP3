/*
********************************************************************************
*          Copyright (C),2004-2005, tonyzyz, Fuzhou Rockchip Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : M3uBroWin.c
* Author     : azg
* Description: 
* History    : 
*           <author>        <time>     <version>
*           sanshin        15/06/16       1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_M3UBRO_WIN_

#include "SysInclude.h"

#ifdef _M3U_
#include "FsInclude.h"
#include "FileInfo.h"
#include "SortInfoGetMacro.h"
#include "AddrSaveMacro.h"

#include "M3uBroWin.h"
#include "SysFindFile.h"
#include "PicInterface.h"
#include "BrowserUI.h"

#include "MessageBox.h"
#include "MainMenu.h"
#include "AudioControl.h"
#include "Hold.h"
#include "myRandom.h"
#include "DialogBox.h"
#include "AddrSaveMacro.h"

UINT32 M3uBroSaveAndPostM3uPlayInfo(void);
void M3uBroFomatTheOtherItem(UINT16 *ucFileName);
UINT16 M3uBroGetListItem(UINT16 *pListName, UINT16 uiListNO);


/*
--------------------------------------------------------------------------------
  Function name : void SortInfoAddrInit(void)
  Author        : anzhiguo
  Description   : the vaious informations saved address of media libary initial.
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         point to sec address of correspond information in flash.
--------------------------------------------------------------------------------
*/
_ATTR_M3UBROWIN_INIT_CODE_
void M3uBroSortInfoAddrInit(void)
{
    M3uSortInfoAddr.ulFileFullInfoSectorAddr = MediaInfoAddr + M3U_SAVE_INFO_SECTOR_START;

    M3uSortInfoAddr.uiM3uSortInfoAddrOffset[0] = FILE_NAME_SAVE_ADDR_OFFSET;
    M3uSortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr + M3U_TREE_SORT_INFO_SECTOR_START; 

}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 GetCurItemNum()
  Author        : anzhiguo
  Description   : ;qH!C=Le?bVP51G0S5SP5DLuD?J}
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/     
_ATTR_M3UBROWIN_CODE_
UINT16 M3uBroGetCurItemNum()
{
    UINT16  i=0;
    UINT16  uiTotalItemNum;
    FDT     Fdt;
    FIND_DATA   FindData;

    uiTotalItemNum = gSysConfig.MedialibPara.gM3uFileNum;

    return uiTotalItemNum;
}
/*
--------------------------------------------------------------------------------
  Function name : void SortInfoAddrInit(void)
  Author        : anzhiguo
  Description   : C=Le?bVP8wVVPEO"4f7E5XV71dA?5D3uJ<;/
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         V85=6TS&PEO"TZflashVP5Dsec5XV7
--------------------------------------------------------------------------------
*/     

_ATTR_M3UBROWIN_CODE_

UINT16 M3uBroGetBaseId()
{
    UINT16 uiBaseId;

    uiBaseId = 0;

    return uiBaseId;
}


/*
--------------------------------------------------------------------------------
  Function name : void MusicBroVariableInit(void)
  Author        : anzhiguo
  Description   : favourite module initial
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2008/07/21         Ver1.0
  desc:         according to favourite folder block flag 00fa,then to set or read out the file number 
                of favourite and favourite sec address.
--------------------------------------------------------------------------------
*/

_ATTR_M3UBROWIN_INIT_CODE_
void M3uBroVariableInit(void *pArg)
{
    UINT16 j,i;

    j = ((M3UBRO_WIN_ARG*)pArg)->CurId;  
    M3uBroTitle = SID_TEXT_FILE;//SID_M3U_FILE//<----sanshin_20150629
    if(j)
    {
        M3uDirTreeInfo.M3uCurId[0] = M3uSysFileInfo.BroCurId;   //<----sanshin_20150630
        M3uDirTreeInfo.KeyCounter = M3uSysFileInfo.BroKeyCnt;   //<----sanshin_20150630
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void M3uDirValueInit(void)
  Author        : anzhiguo
  Description   : M3uDirValueInit 
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/       
_ATTR_M3UBROWIN_CODE_
void M3uBroDirValueInit(void)
{
    unsigned int i;
    unsigned int j = 0;

    //M3uBroItem clear zero to long file name and file number of M3uBroItem
    for (i=0; i<MAX_ITEM_NUM_M3UBRO_DISP; i++)
    {
        memset(&(M3uBroItem[i].LongFileName[j]),0,MAX_FILENAME_LEN*2);
        M3uBroItem[i].ItemNumber = 0xFFFF;
    }
    //the items in M3uBroItem build one double drection list
    for (i=0; i<MAX_ITEM_NUM_M3UBRO_DISP-1; i++)
    {
        M3uBroItem[i].pNext = &M3uBroItem[i+1];
    }
    for (i=1; i<MAX_ITEM_NUM_M3UBRO_DISP; i++)
    {
        M3uBroItem[i].pPrev = &M3uBroItem[i-1];
    }

    M3uBroItem[0].pPrev = &M3uBroItem[MAX_ITEM_NUM_M3UBRO_DISP-1];
    M3uBroItem[MAX_ITEM_NUM_M3UBRO_DISP-1].pNext = &M3uBroItem[0];

    M3uDirTreeInfo.PreCounter = 0;
    M3uDirTreeInfo.pM3uBro     = &M3uBroItem[0];
    M3uDirTreeInfo.M3uCurId[0]     = 0;//uiDivValueTemp;
    M3uDirTreeInfo.M3uCurId[1]     = 0;//uiDivValueTemp;
    M3uDirTreeInfo.M3uCurId[2]     = 0;//uiDivValueTemp;
    M3uDirTreeInfo.M3uCurId[3]     = 0;//uiDivValueTemp;
    M3uDirTreeInfo.M3uDirTotalItem = 0;
    M3uDirTreeInfo.KeyCounter   = 0;
    M3uDirTreeInfo.ItemStar = 0;
    M3uDirTreeInfo.M3uDirBaseSortId[0]=0;
    M3uDirTreeInfo.M3uDirBaseSortId[1]=0;
    M3uDirTreeInfo.M3uDirBaseSortId[2]=0;
    M3uDirTreeInfo.M3uDirBaseSortId[3]=0;

}
/*
--------------------------------------------------------------------------------
  Function name : void M3uBroUpProc(UINT16 uiUpdateType)

  Author        : anzhiguo
  Description   : it is the handle child progarm of media libary module
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/ 
_ATTR_M3UBROWIN_CODE_
void M3uBroUpProc(UINT16 uiUpdateType)
{
    M3UBRO_STRUCT  *pBro;

    UINT16  StartItem = 0;
    UINT16  i,j;

    StartItem = M3uDirTreeInfo.M3uCurId[0] - M3uDirTreeInfo.KeyCounter; 

    switch(uiUpdateType)
    {
        case ALL_BROITEM_UPDATE_M3U:
            for(i=0; i<MAX_ITEM_NUM_M3UBRO_DISP; i++) //clear the items that is diaplay
            {
                for (j=0; j<MAX_FILENAME_LEN; j++)
                {
                    M3uBroItem[i].LongFileName[j] = 0;
                }
                M3uBroItem[i].ItemNumber = 0xFFFF;
                M3uBroItem[i].FileType = 0xFFFF;
            } 

            M3uDirTreeInfo.pM3uBro = &M3uBroItem[0];
            pBro = M3uDirTreeInfo.pM3uBro;

            for(i=0;(i<MAX_ITEM_NUM_M3UBRO_DISP)&&((i+StartItem)<M3uDirTreeInfo.M3uDirTotalItem);i++)
            {

                M3uBroGetListItem(pBro->LongFileName, StartItem+i);

                M3uBroFomatTheOtherItem(pBro->LongFileName);

                #if 0                           //<----sanshin_20150629
                printf("\n");
                for(j=0;j < 30;j++){
                    printf("%c",pBro->LongFileName[j]);
                }
                printf("\n");
                #endif                          //<----sanshin_20150629
                pBro->FileType = FileTypeM3u;   //<----sanshin_20150629

                if(i >= (MAX_ITEM_NUM_M3UBRO_DISP - 1))
                    break;  
                pBro = pBro->pNext;
            }
            break;

        case UP_UPDATE_M3U:
            M3uDirTreeInfo.pM3uBro = M3uDirTreeInfo.pM3uBro->pPrev;

            pBro = M3uDirTreeInfo.pM3uBro;
            for (j=0; j<MAX_FILENAME_LEN; j++)
            { 
                pBro->LongFileName[j] = 0;
            }

            M3uBroGetListItem(pBro->LongFileName, StartItem);
            M3uBroFomatTheOtherItem(pBro->LongFileName);
            pBro->FileType = FileTypeM3u;       //<----sanshin_20150629
            break;

        case DOWN_UPDATE_M3U:

            pBro = M3uDirTreeInfo.pM3uBro;

            for (j=0; j<MAX_FILENAME_LEN; j++)
            {
                pBro->LongFileName[j] = 0;
            }
            {
                M3uBroGetListItem(pBro->LongFileName, M3uDirTreeInfo.M3uCurId[0]); 

                M3uBroFomatTheOtherItem(pBro->LongFileName);
            }
            pBro->FileType = FileTypeM3u;       //<----sanshin_20150629
            M3uDirTreeInfo.pM3uBro = M3uDirTreeInfo.pM3uBro->pNext;
            break;
    }
}


/*
--------------------------------------------------------------------------------
  Function name : void M3uBroM3uDirInit(void)
  Author        : anzhiguo
  Description   : 
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         call when key enter next layer,so permanent
--------------------------------------------------------------------------------
*/     
_ATTR_M3UBROWIN_CODE_
void M3uBroM3uDirInit(void)
{
    M3uDirTreeInfo.M3uDirTotalItem = M3uBroGetCurItemNum();
    M3uDirTreeInfo.M3uDirBaseSortId[0] = M3uBroGetBaseId();

    if(M3uDirTreeInfo.M3uDirTotalItem > MAX_ITEM_NUM_M3UBRO_DISP)
    {
        M3uDirTreeInfo.DispTotalItem = MAX_ITEM_NUM_M3UBRO_DISP;
    }
    else
    {
        M3uDirTreeInfo.DispTotalItem = M3uDirTreeInfo.M3uDirTotalItem;
    } 
    
    {
        M3uBroUpProc(ALL_BROITEM_UPDATE_M3U);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void M3uBroModule(void)
  
  Author        : anzhiguo
  Description   : meida's module entance
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/ 
_ATTR_M3UBROWIN_CODE_    
void M3uBroInit(void *pArg)
{
    UINT16 i,j;

    //ModuleOverlay(MODULE_ID_M3UBRO_INIT, MODULE_OVERLAY_ALL);     //<----sanshin_20150629
    M3uBroSortInfoAddrInit();   
    M3uBroDirValueInit(); 
    M3uBroVariableInit(pArg);
    //DEBUG("%s : M3U_START\n", __FUNCTION__);

    M3uBroM3uDirInit();

    KeyReset();

    SendMsg(MSG_M3UBRO_DISPLAY_ALL);
    SendMsg(MSG_MEDIA_SCROLL_PAINT);

    //ModuleOverlay(MODULE_ID_M3UBRO_SVC, MODULE_OVERLAY_ALL);      //<----sanshin_20150629
}

/*
--------------------------------------------------------------------------------
  Function name : void M3uBroDeInit(void)
  
  Author        : anzhiguo
  Description   : 
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/ 
_ATTR_M3UBROWIN_CODE_ 
void M3uBroDeInit(void)
{
    ClearMsg(MSG_M3UBRO_DISPFLAG_SCROLL_FILENAME);
}



/***************************key handle***********************************/
/******************************************************************************

                            key handle child module
        
*******************************************************************************/
/*
--------------------------------------------------------------------------------
  Function name : void M3uBroKey(void)

  Author        : anzhiguo
  Description   : entery function of meida module key handle
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/ 
_ATTR_M3UBROWIN_SERVICE_CODE_

UINT32 M3uBroKey(void)
{

    uint32 RetVal;
    UINT32 KeyVal;
    UINT16 TempFileNum;
    TASK_ARG TaskArg;
    UINT16 TempDirTotalItem;

    RetVal = RETURN_OK;
    KeyVal =  GetKeyVal();

     //5 Key function modification    
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {            
        switch(KeyVal)
        {
        case KEY_VAL_FFW_DOWN:
        case KEY_VAL_FFW_PRESS:
            KeyVal = KEY_VAL_UP_DOWN;
            break;
            
        case KEY_VAL_FFD_DOWN:
        case KEY_VAL_FFD_PRESS:
            KeyVal = KEY_VAL_DOWN_DOWN;
            break;

        default:
            break;
        }
    }

    //6 Key function modification    
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch(KeyVal)
        {            
        case KEY_VAL_FFW_SHORT_UP:
            KeyVal = KEY_VAL_ESC_SHORT_UP;
            break;

        default:
            break;
        }
    }

    //7 Key function modification 
    switch (KeyVal) 
    {
    //--------------------- MENU KEY ---------------------------
        case KEY_VAL_MENU_SHORT_UP:

            if(M3uDirTreeInfo.M3uDirTotalItem == 0)
                break;

            ModuleOverlay(MODULE_ID_M3UBRO_SORTGET, MODULE_OVERLAY_ALL);
            M3uBroSaveAndPostM3uPlayInfo();
            TaskArg.M3u.FileType = FileTypeM3u;     //<----sanshin_20150707
            TaskArg.M3u.FileNum  = 0;
            TaskArg.M3u.FromWhere = 0;
            TaskSwitch(TASK_ID_M3UWIN, &TaskArg);
            RetVal = 1;

            break;

    //--------------------- UP KEY ---------------------------
        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
            if((M3uDirTreeInfo.M3uDirTotalItem) == 0)
                break;

            M3uDirTreeInfo.PreCounter = M3uDirTreeInfo.KeyCounter;

            if(M3uDirTreeInfo.M3uCurId[0/*M3uDirTreeInfo.MusicDirDeep*/] > 0) 
            {
                M3uDirTreeInfo.M3uCurId[0/*MusicDirTreeInfo.MusicDirDeep*/]--;

                if(M3uDirTreeInfo.KeyCounter == 0) 
                {
                    {
                        M3uBroUpProc(UP_UPDATE_M3U);
                    }
                    SendMsg(MSG_M3UBRO_ALL_ITEM);
                }
                else
                {
                    M3uDirTreeInfo.KeyCounter--;
                    SendMsg(MSG_M3UBRO_FRESH_ITEM);
                }
            }
            else
            {
                TempFileNum = M3uDirTreeInfo.M3uDirTotalItem;
                {
                    M3uDirTreeInfo.M3uCurId[0] = TempFileNum-1;
                    if( TempFileNum > MAX_ITEM_NUM_M3UBRO_DISP)
                    {
                        M3uDirTreeInfo.KeyCounter = MAX_ITEM_NUM_M3UBRO_DISP -1;
                        M3uBroM3uDirInit();
                        SendMsg(MSG_M3UBRO_ALL_ITEM);
                    }
                    else
                    {
                        M3uDirTreeInfo.KeyCounter = TempFileNum-1;
                        SendMsg(MSG_M3UBRO_FRESH_ITEM);
                    }
                }
            }

            SendMsg(MSG_MEDIA_SCROLL_PAINT);

            break;
    //------------------- DOWN KEY ---------------------------

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
            if((M3uDirTreeInfo.M3uDirTotalItem) == 0)
                break;
                
            M3uDirTreeInfo.PreCounter = M3uDirTreeInfo.KeyCounter;

            {
                TempDirTotalItem = M3uDirTreeInfo.M3uDirTotalItem - 1;
            }

            if(M3uDirTreeInfo.M3uCurId[0] < TempDirTotalItem)
            {
                M3uDirTreeInfo.M3uCurId[0]++; 

                if(M3uDirTreeInfo.KeyCounter >= MAX_ITEM_NUM_M3UBRO_DISP - 1) 
                {
                    {
                        M3uBroUpProc(DOWN_UPDATE_M3U);
                    }
                    SendMsg(MSG_M3UBRO_ALL_ITEM);//display all screen
                }
                else
                {
                    M3uDirTreeInfo.KeyCounter++;
                    SendMsg(MSG_M3UBRO_FRESH_ITEM);
                }  
            }
            else
            {
                M3uDirTreeInfo.M3uCurId[0] = 0;
                M3uDirTreeInfo.KeyCounter = 0;
                {
                    if(M3uDirTreeInfo.M3uDirTotalItem > MAX_ITEM_NUM_M3UBRO_DISP)
                    {
                        M3uBroM3uDirInit();
                        SendMsg(MSG_M3UBRO_ALL_ITEM);
                    }
                    else
                    {
                        SendMsg(MSG_M3UBRO_FRESH_ITEM);
                    }
                }
            }
            SendMsg(MSG_MEDIA_SCROLL_PAINT);
            
        
        
            break;

    //--------------------- FFW KEY --------------------------
        case KEY_VAL_PLAY_SHORT_UP://display sub window
            break;

        case KEY_VAL_ESC_SHORT_UP:
        
            TaskArg.MainMenu.MenuID = MAINMENU_ID_M3U;  //<-----sanshin_20150629//
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            RetVal = 1;
            break;

        case KEY_VAL_HOLD_ON://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&M3uBroWin, &HoldWin, &TaskArg);
        break;
            
        case KEY_VAL_HOLD_OFF://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&M3uBroWin, &HoldWin, &TaskArg);
        break;

        default:
            break;
    }
    return(RetVal);
}




/*
--------------------------------------------------------------------------------
  Function name : void M3uBroSaveAndPostM3uPlayInfo(void)

  Author        : anzhiguo
  Description   : enter media player
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/ 
_ATTR_M3UBRO_SORTGET_CODE_
UINT32 M3uBroSaveAndPostM3uPlayInfo(void)
{
    UINT16   i;   
    UINT16    uiFindResult;
    UINT16    temp1;      
    UINT16    tempFileNum;
    UINT16    musicfilenum;
    UINT16    longname[SYS_SUPPROT_STRING_MAX_LEN + 1];
    TASK_ARG TaskArg;
    uint32 RetVal;

    RetVal = RETURN_OK;

    #ifdef _SDCARD_  
    if(MemorySelect!=FLASH0)
    {
        MemorySelect = FLASH0;

        FileSysSetup(MemorySelect);
    }
    #endif


    M3uBroGetSavedM3uDir(&stFindDataM3u, M3uSortInfoAddr.ulFileFullInfoSectorAddr, M3uSortInfoAddr.ulFileSortInfoSectorAddr, (M3uDirTreeInfo.M3uCurId[0]+ M3uDirTreeInfo.M3uDirBaseSortId[0]));
    //printf("st.clus = %d, st.index = %d\n",stFindDataM3u.Clus, stFindDataM3u.Index);
    memcpy((uint8 *)&M3uSysFileInfo.FindData , (uint8 *)&stFindDataM3u ,8);

    M3uSysFileInfo.BroCurId = M3uDirTreeInfo.M3uCurId[0];   //<----sanshin_20150630
    M3uSysFileInfo.BroKeyCnt = M3uDirTreeInfo.KeyCounter;   //<----sanshin_20150630
//---->sanshin_20150630
    //M3uSysFileInfo.ulFullInfoSectorAddr = M3uSortInfoAddr.ulFileFullInfoSectorAddr;
    //M3uSysFileInfo.ulSortInfoSectorAddr = M3uSortInfoAddr.ulFileSortInfoSectorAddr;
    //M3uSysFileInfo.ucCurDeep = 0;
    //
    //M3uSysFileInfo.TotalFiles = M3uDirTreeInfo.M3uDirTotalItem;
    //{
    //    M3uSysFileInfo.CurrentFileNum = M3uDirTreeInfo.M3uCurId[0]+1;  
    //}
    //M3uSysFileInfo.pExtStr = PictureFileExtString;
    //M3uSysFileInfo.PlayedFileNum = M3uSysFileInfo.CurrentFileNum;
    //
    //M3uBroGetMediaItemInfo(gM3uTypeSelName, M3uSortInfoAddr, (M3uDirTreeInfo.M3uCurId[0]+M3uDirTreeInfo.M3uDirBaseSortId[0]), MEDIA_ID3_SAVE_CHAR_NUM, 0,0);
    //{
    //     TaskArg.Pic.FileNum = M3uDirTreeInfo.M3uCurId[0] + 1;
    //}
//<----sanshin_20150630
    return RetVal;
}
/*
--------------------------------------------------------------------------------
  Function name : void FomatTheOtherItem(UINT16 *ucFileName)

  Author        : anzhiguo
  Description   :
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/ 
_ATTR_M3UBROWIN_CODE_
void M3uBroFomatTheOtherItem(UINT16 *ucFileName)
{
    unsigned int j;

    if(ucFileName[0]==0)  // (ucFileName[0]==0x20)
    {
        for(j=0;j<MAX_FILENAME_LEN;j++)
        {
            if(ucFileName[j]!=0)  
            {
                break;
            }
        }
         
        if(j==MAX_FILENAME_LEN)
        {
            ucFileName[0]='U'; // '<'
            ucFileName[1]='n'; // 'O'
            ucFileName[2]='K'; // 't'
            ucFileName[3]='n'; // 'h'
            ucFileName[4]='o'; // 'e'
            ucFileName[5]='w'; // 'r'
            ucFileName[6]='n'; // '>'
        }
    }    
}

/*
--------------------------------------------------------------------------------
  Function name : void FomatTheOtherItem(UINT16 *ucFileName)

  Author        : anzhiguo
  Description   :
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/ 
_ATTR_M3UBROWIN_CODE_
UINT16 M3uBroGetListItem(UINT16 *pListName, UINT16 uiListNO)
{
    unsigned int i;
    FDT BroFDTBuf;
    FIND_DATA FindDataBro;
    UINT8 buf[PATH_SIZE_M3U];

    M3uBroGetMediaItemInfo(pListName, M3uSortInfoAddr, uiListNO, MAX_FILENAME_LEN, 0, 1);

    FileExtNameRemove(pListName, PictureFileExtString);
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name : void   M3uBroService(void)
  
  
  Author        : anzhiguo
  Description   : C=Le?bD#?iOTJ>D#?i
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/
_ATTR_M3UBROWIN_SERVICE_CODE_
UINT32 M3uBroService(void)
{

    TASK_ARG TaskArg;
    UINT32 RetVal = 0;

    if(CheckMsg(MSG_M3UBRO_NOFIND_FILE))
    {
        DEBUG("%s : ERROR0\n", __FUNCTION__);
        if(CheckMsg(MSG_MESSAGEBOX_DESTROY))
        {
            ClearMsg(MSG_M3UBRO_NOFIND_FILE);
            ClearMsg(MSG_MESSAGEBOX_DESTROY);
            SendMsg(MSG_M3UBRO_DISPLAY_ALL);
        }
    }

    if(TRUE == GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
        {
            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            RetVal = 1;
        }
    }
    return RetVal;
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
_ATTR_M3UBROWIN_SERVICE_CODE_
void M3uBroWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
#if 1
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
#endif
}


/*
--------------------------------------------------------------------------------
  Function name : void   M3uBroDisplay(void)
  
  
  Author        : anzhiguo
  Description   : the module of media display
                  
  Input         : 
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009/06/02         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/     
_ATTR_M3UBROWIN_SERVICE_CODE_
void   M3uBroDisplay(void)
{
#if 1
    M3UBRO_STRUCT  *pBro;
    M3UBRO_DIR_TREE_STRUCT *pM3uDirInfo;
    LCD_RECT      r;
    UINT32 M3uBroNFAddr;
    UINT16  i,j,k,m,n;
    UINT16  color_temp;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;

    UINT16  TotalItem, TempColor, TempCharSize, TextMode;
    pBro = M3uDirTreeInfo.pM3uBro;

    TempColor = LCD_GetColor();
    LCD_SetColor(COLOR_BLACK);
    TempCharSize = LCD_SetCharSize(FONT_12x12); 
    TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_M3UBRO_DISPLAY_ALL)))
    {
        //Display BackGround
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);

        //Display Title
        DisplayMenuStrWithIDNum(M3UBRO_TITLE_TXT_X, M3UBRO_TITLE_TXT_Y, M3UBRO_TITLE_TXT_XSIZE, 
                            M3UBRO_TITLE_TXT_YSIZE, LCD_TEXTALIGN_CENTER, M3uBroTitle );

        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+BrowserBatteryLevel,105,146);

        //Send other display message
        SendMsg(MSG_M3UBRO_ALL_ITEM);
    }

    if(GetMsg(MSG_M3UBRO_ALL_ITEM))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3,143);
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);

        TotalItem = M3uDirTreeInfo.DispTotalItem;

        for(i = 0; ((i < MAX_ITEM_NUM_M3UBRO_DISP) && (i < TotalItem )); i++)
        {
            //Display file Type Icon
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);

            r.x0 = 20;
            r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;

            if(i == M3uDirTreeInfo.KeyCounter)
            {
                DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                if(LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                {
                    M3uBroPrintfBuf = pBro->LongFileName;
                    M3uBroWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 3);

                    SendMsg(MSG_M3UBRO_DISPFLAG_SCROLL_FILENAME);
                }
                else
                {
                    ClearMsg(MSG_M3UBRO_DISPFLAG_SCROLL_FILENAME);
                }
            }

            r.y0 += 2;
            r.y1 =  r.y0+CH_CHAR_XSIZE_12-1;
            
            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);           
            pBro = pBro->pNext;
        }   
        SendMsg(MSG_MEDIA_SCROLL_PAINT);
    }

    if(GetMsg(MSG_M3UBRO_FRESH_ITEM))
    {
    
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);

        for(i = 0; i < BROWSER_SCREEN_PER_LINE; i++)
        {
            if((i == M3uDirTreeInfo.KeyCounter) || (i == M3uDirTreeInfo.PreCounter))
            {
                r.x0 = 20;
                r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
                r.x1 = r.x0 + PicInfo.xSize;
                r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;
                
                if (i == M3uDirTreeInfo.PreCounter)
                {
                    DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
                }
                
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);

                if(i == M3uDirTreeInfo.KeyCounter)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                    if(LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                    {
                        M3uBroPrintfBuf = pBro->LongFileName;
                        M3uBroWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 3);
                        SendMsg(MSG_M3UBRO_DISPFLAG_SCROLL_FILENAME);
                    }
                    else
                    {
                        ClearMsg(MSG_M3UBRO_DISPFLAG_SCROLL_FILENAME);
                    }
                }
                
                r.y0 += 2;
                r.y1 = r.y0+CH_CHAR_XSIZE_12-1;
                LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);           

            }
            pBro = pBro->pNext;
        }
        SendMsg(MSG_MEDIA_SCROLL_PAINT);
    }

    if (CheckMsg(MSG_M3UBRO_DISPFLAG_SCROLL_FILENAME))//an 4.21
    {
        TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(M3uBroPrintfBuf);
        LCD_SetTextMode(TextMode);
    }

    if(GetMsg(MSG_MEDIA_SCROLL_PAINT))
    {

        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL_BLOCK, &PicInfo1);

        if(M3uDirTreeInfo.M3uCurId[0] > 0)
        {
            i = (PicInfo.ySize - PicInfo1.ySize) * M3uDirTreeInfo.M3uCurId[0]  /( M3uDirTreeInfo.M3uDirTotalItem-1);//+picInfo1.ySize/2;
        }
        else
        {
            i = 0;
        }

        if((i +PicInfo1.ySize)>PicInfo.ySize)//an 4.22
        {
            i = PicInfo.ySize - PicInfo1.ySize;
        }   
        
        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_SCOLL,122,0);            
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_SCOLL_BLOCK,0+123, i);//scroll small icon

    }

    LCD_SetTextMode(TextMode);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
#endif
}


#endif

