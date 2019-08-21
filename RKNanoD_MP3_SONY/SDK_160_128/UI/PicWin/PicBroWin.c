/*
********************************************************************************
*          Copyright (C),2004-2005, tonyzyz, Fuzhou Rockchip Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : PicBroWin.c
* Author     : azg
* Description:
* History    :
*           <author>        <time>     <version>       <desc>
*           azg      05/11/24       1.0            ORG
*
********************************************************************************
*/
#define _IN_PICBRO_WIN_

#include "SysInclude.h"
#ifdef PIC_MEDIA	//<----sanshin_20150818
#ifdef _PICTURE_
#include "FsInclude.h"
#include "FileInfo.h"
#include "SortInfoGetMacro.h"
#include "AddrSaveMacro.h"


#include "PicBroWin.h"
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

//---->sanshin_20150625
#include "image_main.h"
#include "ImageControl.h"
#ifdef THUMB_DEC_INCLUDE
#include "thumbnail_parse.h"
#endif
//<----sanshin_20150625

UINT32 PicBroSaveAndPostJpegPlayInfo(void);
void PicBroFomatTheOtherItem(UINT16 *ucFileName);
UINT16 PicBroGetListItem(UINT16 *pListName, UINT16 uiListNO);


extern SORTINFO_STRUCT Subinfo;

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
_ATTR_PICBROWIN_INIT_CODE_
void PicBroSortInfoAddrInit(void)
{

     #ifdef PIC_MEDIA
     JpegSortInfoAddr.ulFileFullInfoSectorAddr = MediaInfoAddr + JPEG_SAVE_INFO_SECTOR_START;

    JpegSortInfoAddr.uiJpegSortInfoAddrOffset[0] = FILE_NAME_SAVE_ADDR_OFFSET;
    JpegSortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr + JPEG_TREE_ALL_SORT_INFO_SECTOR_START;			/*<--sanshin 0612*/
    #endif
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
_ATTR_PICBROWIN_CODE_
UINT16 PicBroGetCurItemNum()
{
    UINT16  i=0;
    UINT16  uiTotalItemNum;
    FDT     Fdt;
    FIND_DATA   FindData;
    SORTINFO_STRUCT Subinfo;

    //if(JpegDirTreeInfo.JpegDirDeep == 0)
    //{
    uiTotalItemNum = gSysConfig.MedialibPara.gJpegFileNum;
    //}
    //else //if(JpegDirTreeInfo.JpegDirDeep == 1)//;qH!W(<-;rRUJu<ROB5DND<~J}
    //{

//		if((JpegSortInfoAddr.uiJpegSortInfoAddrOffset[JpegDirTreeInfo.JpegDirDeep] == ID3_TITLE_SAVE_ADDR_OFFSET) && ((gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER) || (gMusicTypeSelID == SORT_TYPE_SEL_GENRE)))
//		{
//			uiTotalItemNum = GetSummaryInfo(JpegSortInfoAddr.ulSortSubInfoSectorAddr[JpegDirTreeInfo.JpegDirDeep-1], JpegDirTreeInfo.CurId[JpegDirTreeInfo.JpegDirDeep-1] - 1 + JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep-1], FIND_SUM_ITEMNUM);
//		}
//		else
//		{
//			uiTotalItemNum = GetSummaryInfo(JpegSortInfoAddr.ulSortSubInfoSectorAddr[JpegDirTreeInfo.JpegDirDeep-1], JpegDirTreeInfo.JpegCurId[JpegDirTreeInfo.JpegDirDeep-1]+JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep-1], FIND_SUM_ITEMNUM);
//		}
    // JpegDirTreeInfo.TotalItem[JpegDirTreeInfo.JpegDirDeep-1] = Subinfo.ItemNum;
    //}
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

_ATTR_PICBROWIN_CODE_

UINT16 PicBroGetBaseId()
{
    UINT16 uiBaseId;

    //if (JpegDirTreeInfo.JpegDirDeep == 0)
    //{
    uiBaseId = 0;
    //}
    /*  sch120418
     else if ((JpegDirTreeInfo.JpegDirDeep == 2)&&(gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER))
     {
         uiBaseId = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[JpegDirTreeInfo.JpegDirDeep-1], JpegDirTreeInfo.CurId[JpegDirTreeInfo.JpegDirDeep-1]+JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep-1]-1, FIND_SUM_SORTSTART);
     }
     */
//    else //if((JpegDirTreeInfo.JpegDirDeep == 1)||)
//    {
    //Rk Aaron.sun
//		if((JpegSortInfoAddr.uiJpegSortInfoAddrOffset[JpegDirTreeInfo.JpegDirDeep] == ID3_TITLE_SAVE_ADDR_OFFSET) && ((gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER) || (gMusicTypeSelID == SORT_TYPE_SEL_GENRE)))
//		{
//			uiBaseId = GetSummaryInfo(JpegSortInfoAddr.ulSortSubInfoSectorAddr[JpegDirTreeInfo.JpegDirDeep-1], JpegDirTreeInfo.CurId[JpegDirTreeInfo.JpegDirDeep-1] - 1 + JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep-1], FIND_SUM_SORTSTART);
//		}
//		else
//		{
//	 		uiBaseId = GetSummaryInfo(JpegSortInfoAddr.ulSortSubInfoSectorAddr[0/*JpegDirTreeInfo.JpegDirDeep-1*/], JpegDirTreeInfo.JpegCurId[JpegDirTreeInfo.JpegDirDeep-1] + JpegDirTreeInfo.JpegDirBaseSortId[0/*JpegDirTreeInfo.JpegDirDeep-1*/], FIND_SUM_SORTSTART);
//
//		}
//
//    }

    return uiBaseId;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordDBFileDelete(void)
  Author        : allen
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                allen               2013/09/25         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
extern  uint32   FlashSec[3];

/*
--------------------------------------------------------------------------------
  Function name : void DisplayNowPlayingIcon(void)
  Author        : allen
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                allen               2013/11/07         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
/*
_ATTR_PICBROWIN_CODE_
UINT16 PicBroDisplayNowPlayingIcon(UINT16 id)
{
    UINT16 ret;
    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;
    uint32 DirClus, Index;


    ret = 0;
    if ((TRUE == ThreadCheck(pMainThread, &MusicThread)) && (gMusicTypeSelID == SORT_TYPE_SEL_ID3TITLE))
    {
        GetSavedJpegDir(&stFindData, JpegSortInfoAddr.ulFileFullInfoSectorAddr, JpegSortInfoAddr.ulFileSortInfoSectorAddr, (id + JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep]));

        if(PicSysFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(PicSysFileInfo.FindData.Clus + PicSysFileInfo.FindData.Index - 1), 2, (uint8 *)&temp);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(PicSysFileInfo.FindData.Clus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
            if ((stFindData.Clus == DirClus) &&
                (stFindData.Index == Index))
            {
               ret = 1;
            }

        }
        else
        {
            if ((stFindData.Clus == PicSysFileInfo.FindData.Clus) &&
                (stFindData.Index == PicSysFileInfo.FindData.Index))
            {
               ret = 1;
            }
        }
    }

    return ret;
}
*/
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

_ATTR_PICBROWIN_INIT_CODE_

void PicBroVariableInit(void *pArg)
{
    UINT16 i;
	UINT16 curid, preid, keycnt;/*<--sanshin_20150630*/

/*-->sanshin_20150630*/
    curid = ((PICBRO_WIN_ARG*)pArg)->CurId;
    PicBroTitle = SID_PICTURE_FILE;

	//DEBUG("%s pre : curid=%d, preid=%d, keycnt=%d\n", __FUNCTION__, curid, gbPicCurid, gbPicKeyCnt);
    if (curid)
    {
    	curid -= 1;
    	preid = gbPicCurid;
    	keycnt = gbPicKeyCnt;

    	if(preid != curid)
    	{
    		if(preid > curid)
    		{
    			if(keycnt >= (preid - curid))
    			{
    				keycnt -= (preid - curid);
    			}
    			else
    			{
    				keycnt = 0;
    			}
    		}
    		else
    		{
    			keycnt += (curid - preid);
    			if(keycnt > MAX_ITEM_NUM_PICBRO_DISP-1)
    			{
    				keycnt = MAX_ITEM_NUM_PICBRO_DISP-1;
    			}
    		}
    	}

    	JpegDirTreeInfo.JpegCurId[0] = curid;
		JpegDirTreeInfo.KeyCounter = keycnt;
    }
	else
	{
		JpegDirTreeInfo.JpegCurId[0] = 0;
		JpegDirTreeInfo.KeyCounter = 0;
	}
	//DEBUG("%s post : curid=%d, keycnt=%d\n", __FUNCTION__, JpegDirTreeInfo.JpegCurId[0], JpegDirTreeInfo.KeyCounter);
/*<--sanshin_20150630*/
}
/*
--------------------------------------------------------------------------------
  Function name : void JpegDirValueInit(void)
  Author        : anzhiguo
  Description   : JpegDirValueInit

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
void PicBroDirValueInit(void)
{
    unsigned int i;
    unsigned int j = 0;

    //PicBroItem clear zero to long file name and file number of PicBroItem
    for (i=0; i<MAX_ITEM_NUM_PICBRO_DISP; i++)
    {
        memset(&(PicBroItem[i].LongFileName[j]),0,MAX_FILENAME_LEN*2);
        PicBroItem[i].ItemNumber = 0xFFFF;
    }
    //the items in PicBroItem build one double drection list
    for (i=0; i<MAX_ITEM_NUM_PICBRO_DISP-1; i++)
    {
        PicBroItem[i].pNext = &PicBroItem[i+1];
    }
    for (i=1; i<MAX_ITEM_NUM_PICBRO_DISP; i++)
    {
        PicBroItem[i].pPrev = &PicBroItem[i-1];
    }

    PicBroItem[0].pPrev = &PicBroItem[MAX_ITEM_NUM_PICBRO_DISP-1];
    PicBroItem[MAX_ITEM_NUM_PICBRO_DISP-1].pNext = &PicBroItem[0];

    JpegDirTreeInfo.PreCounter = 0;
    JpegDirTreeInfo.pPicBro     = &PicBroItem[0];
    JpegDirTreeInfo.JpegCurId[0]     = 0;//uiDivValueTemp;
    JpegDirTreeInfo.JpegCurId[1]     = 0;//uiDivValueTemp;
    JpegDirTreeInfo.JpegCurId[2]     = 0;//uiDivValueTemp;
    JpegDirTreeInfo.JpegCurId[3]     = 0;//uiDivValueTemp;
//    JpegDirTreeInfo.JpegDirDeep      = 0;
    JpegDirTreeInfo.JpegDirTotalItem = 0;
    JpegDirTreeInfo.KeyCounter   = 0;
    JpegDirTreeInfo.ItemStar = 0;
    JpegDirTreeInfo.JpegDirBaseSortId[0]=0;
    JpegDirTreeInfo.JpegDirBaseSortId[1]=0;
    JpegDirTreeInfo.JpegDirBaseSortId[2]=0;
    JpegDirTreeInfo.JpegDirBaseSortId[3]=0;

}
/*
--------------------------------------------------------------------------------
  Function name : void PicBroUpProc(UINT16 uiUpdateType)

  Author        : anzhiguo
  Description   : it is the handle child progarm of media libary module

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
void PicBroUpProc(UINT16 uiUpdateType)
{
    PICBRO_STRUCT  *pBro;

    UINT16  StartItem = 0;
    UINT16  i,j;

    StartItem = JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/] - JpegDirTreeInfo.KeyCounter;

    //StartItem = 10;

    switch (uiUpdateType)
    {
        case ALL_BROITEM_UPDATE_JPEG:
            for (i=0; i<MAX_ITEM_NUM_PICBRO_DISP; i++) //clear the items that is diaplay
            {
                for (j=0; j<MAX_FILENAME_LEN; j++)
                {
                    PicBroItem[i].LongFileName[j] = 0;
                }
                PicBroItem[i].ItemNumber = 0xFFFF;
                PicBroItem[i].FileType = 0xFFFF;
            }

            JpegDirTreeInfo.pPicBro = &PicBroItem[0];
            pBro = JpegDirTreeInfo.pPicBro;

            for (i=0;(i<MAX_ITEM_NUM_PICBRO_DISP)&&((i+StartItem)<JpegDirTreeInfo.JpegDirTotalItem);i++)
            {

                // Rk Aaron.sun
//		  if((JpegSortInfoAddr.uiJpegSortInfoAddrOffset[JpegDirTreeInfo.JpegDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (JpegDirTreeInfo.JpegDirDeep != 0) )
//		  {
//			if((StartItem == 0) && (i == 0))
//		       {
//				memcpy(pBro->LongFileName, L"All Album\0", 20);
//			}
//			else
//			{
//				GetListItem(pBro->LongFileName, StartItem - 1 + i);
//
//                		FomatTheOtherItem(pBro->LongFileName);
//			}
//		  }
//		  else
//		  {
                PicBroGetListItem(pBro->LongFileName, StartItem+i);
//DEBUG("%s   %d   %d",__FUNCTION__,StartItem,i);

                PicBroFomatTheOtherItem(pBro->LongFileName);
//		  }
                //printf("\n");
                //for (j=0;j < 30;j++)
                //{
                //    printf("%c",pBro->LongFileName[j]);
                //}
                //printf("\n");



                pBro->FileType = FileTypePicture;

                if (i >= (MAX_ITEM_NUM_PICBRO_DISP - 1))
                    break;


                pBro = pBro->pNext;
            }

            break;

        case UP_UPDATE_JPEG:
            JpegDirTreeInfo.pPicBro = JpegDirTreeInfo.pPicBro->pPrev;

            pBro = JpegDirTreeInfo.pPicBro;
            for (j=0; j<MAX_FILENAME_LEN; j++)
            {
                pBro->LongFileName[j] = 0;
            }

            //RK Aaron.sun
//		if((JpegSortInfoAddr.uiJpegSortInfoAddrOffset[JpegDirTreeInfo.JpegDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (JpegDirTreeInfo.JpegDirDeep != 0) )
//		{
//			if(StartItem == 0)
//			{
//				memcpy(pBro->LongFileName, L"All Album\0", 20);
//			}
//			else
//			{
//				GetListItem(pBro->LongFileName, StartItem - 1);
//
//			    	FomatTheOtherItem(pBro->LongFileName);
//			}
//		}
//		else
//		{
            PicBroGetListItem(pBro->LongFileName, StartItem);

            PicBroFomatTheOtherItem(pBro->LongFileName);
//		}


            pBro->FileType = FileTypePicture;	/*<--sanshin_20150630*/

            break;

        case DOWN_UPDATE_JPEG:

            pBro = JpegDirTreeInfo.pPicBro;

            for (j=0; j<MAX_FILENAME_LEN; j++)
            {
                pBro->LongFileName[j] = 0;
            }
//             if ((JpegSortInfoAddr.uiJpegSortInfoAddrOffset[JpegDirTreeInfo.JpegDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (JpegDirTreeInfo.JpegDirDeep != 0) )
//            {
//                GetListItem(pBro->LongFileName, JpegDirTreeInfo.JpegCurId[JpegDirTreeInfo.JpegDirDeep]-1);

//                FomatTheOtherItem(pBro->LongFileName);
//            }
//            else
            {
                PicBroGetListItem(pBro->LongFileName, JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/]);

                PicBroFomatTheOtherItem(pBro->LongFileName);
            }

            pBro->FileType = FileTypePicture;	/*<--sanshin_20150630*/

            JpegDirTreeInfo.pPicBro = JpegDirTreeInfo.pPicBro->pNext;

            break;
    }
}


/*
--------------------------------------------------------------------------------
  Function name : void PicBroJpegDirInit(void)
  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         call when key enter next layer,so permanent
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
void PicBroJpegDirInit(void)
{
    JpegDirTreeInfo.JpegDirTotalItem = PicBroGetCurItemNum();//51G0OTJ>D?B<OB5DND<~8vJ}
    JpegDirTreeInfo.JpegDirBaseSortId[0/*JpegDirTreeInfo.JpegDirDeep*/] = PicBroGetBaseId();//Ub8v1dA?JGJ2C4 ?????? JG51G0OTJ>FpJ<ND<~5DPr:E?

    if (JpegDirTreeInfo.JpegDirTotalItem > MAX_ITEM_NUM_PICBRO_DISP)
    {
        JpegDirTreeInfo.DispTotalItem = MAX_ITEM_NUM_PICBRO_DISP;
    }
    else
    {
        JpegDirTreeInfo.DispTotalItem = JpegDirTreeInfo.JpegDirTotalItem;
    }

    {
        PicBroUpProc(ALL_BROITEM_UPDATE_JPEG);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PicBroModule(void)

  Author        : anzhiguo
  Description   : meida's module entance

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
void PicBroInit(void *pArg)
{
    UINT16 i,j;
//---->sanshin_20150625
#ifdef THUMB_DEC_INCLUDE
    ModuleOverlay(MODULE_ID_PICTURE_CONTROL, MODULE_OVERLAY_ALL);
#endif
//<----sanshin_20150625
    //ModuleOverlay(MODULE_ID_PICBRO_INIT, MODULE_OVERLAY_ALL);
    PicBroSortInfoAddrInit();
    PicBroDirValueInit();
    PicBroVariableInit(pArg);
//	DEBUG("INIT 0!!!!");


    PicBroJpegDirInit();
//	DEBUG("INIT 1!!!!");


    KeyReset();

    SendMsg(MSG_PICBRO_DISPLAY_ALL);
    SendMsg(MSG_MEDIA_SCROLL_PAINT);

    //ModuleOverlay(MODULE_ID_PICBRO_SVC, MODULE_OVERLAY_ALL);
//	DEBUG("INIT 2!!!!");

}

/*
--------------------------------------------------------------------------------
  Function name : void PicBroDeInit(void)

  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
void PicBroDeInit(void)
{
    ClearMsg(MSG_PICBRO_DISPFLAG_SCROLL_FILENAME);
}



/***************************key handle***********************************/
/******************************************************************************

                            key handle child module

*******************************************************************************/
/*
--------------------------------------------------------------------------------
  Function name : void PicBroKey(void)

  Author        : anzhiguo
  Description   : entery function of meida module key handle

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_SERVICE_CODE_

UINT32 PicBroKey(void)
{

    uint32 RetVal;
    UINT32 KeyVal;
    UINT16 TempFileNum;
    TASK_ARG TaskArg;
    UINT16 TempDirTotalItem;

//	DEBUG("   !!!!");
    RetVal = RETURN_OK;

    KeyVal =  GetKeyVal();

    //5 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (KeyVal)
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
        switch (KeyVal)
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
        case KEY_VAL_MENU_SHORT_UP:

            if (JpegDirTreeInfo.JpegDirTotalItem == 0)
                break;

            //ModuleOverlay(MODULE_ID_PICBRO_SORTGET, MODULE_OVERLAY_ALL);
//            if(JpegDirTreeInfo.JpegDirDeep==0)
//            {
//                switch(gMusicTypeSelID)
//                {
//                    case SORT_TYPE_SEL_FILENAME:
            RetVal =PicBroSaveAndPostJpegPlayInfo();

//                        break;
//
//                    default:
//                        break;
//                }
//            }

            break;

            //--------------------- UP KEY ---------------------------
        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
            if ((JpegDirTreeInfo.JpegDirTotalItem) == 0)
                break;

            JpegDirTreeInfo.PreCounter = JpegDirTreeInfo.KeyCounter;

            if (JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.MusicDirDeep*/] > 0)
            {
                JpegDirTreeInfo.JpegCurId[0/*MusicDirTreeInfo.MusicDirDeep*/]--;

                if (JpegDirTreeInfo.KeyCounter == 0)
                {
                    {
                        PicBroUpProc(UP_UPDATE_JPEG);
                    }
#ifdef _FRAME_BUFFER_
                	//---->sanshin_20150625
                	JpegDirTreeInfo.PreCounter = 1;
                	LCD_Shift_Window(0, DIRECTION_DOWN, 17, 0, 3, 122, 105);
                	SendMsg(MSG_PICBRO_FRESH_ITEM);
                	SendMsg(MSG_PICBRO_FRESH_THUMBNAIL);
                	//<----sanshin_20150625
#else
                    SendMsg(MSG_PICBRO_ALL_ITEM);
#endif
                }
                else
                {
                    JpegDirTreeInfo.KeyCounter--;
                    SendMsg(MSG_PICBRO_FRESH_ITEM);
                }
            }
            else

            {
                TempFileNum = JpegDirTreeInfo.JpegDirTotalItem;
                {
                    JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/] = TempFileNum-1;
                    if ( TempFileNum > MAX_ITEM_NUM_PICBRO_DISP)
                    {
                        JpegDirTreeInfo.KeyCounter = MAX_ITEM_NUM_PICBRO_DISP -1;
                        PicBroJpegDirInit();
                        SendMsg(MSG_PICBRO_ALL_ITEM);
                    }
                    else
                    {
                        JpegDirTreeInfo.KeyCounter = TempFileNum-1;
                        SendMsg(MSG_PICBRO_FRESH_ITEM);
                    }
                }
            }

            SendMsg(MSG_MEDIA_SCROLL_PAINT);

            break;
            //------------------- DOWN KEY ---------------------------

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
            if ((JpegDirTreeInfo.JpegDirTotalItem) == 0)
                break;

            JpegDirTreeInfo.PreCounter = JpegDirTreeInfo.KeyCounter;

            {
                TempDirTotalItem = JpegDirTreeInfo.JpegDirTotalItem - 1;
            }

            if (JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/] < TempDirTotalItem)
            {
                JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/]++;

                if (JpegDirTreeInfo.KeyCounter >= MAX_ITEM_NUM_PICBRO_DISP - 1)
                {
                    {
                        PicBroUpProc(DOWN_UPDATE_JPEG);
                    }
#ifdef _FRAME_BUFFER_
                	//---->sanshin_20150625
                	JpegDirTreeInfo.PreCounter = 4;
                	LCD_Shift_Window(0, DIRECTION_UP, 17, 0, 3, 122, 105);
                	SendMsg(MSG_PICBRO_FRESH_ITEM);
                	SendMsg(MSG_PICBRO_FRESH_THUMBNAIL);
                	//<----sanshin_20150625
#else
                    SendMsg(MSG_PICBRO_ALL_ITEM);//display all screen
#endif
                }
                else
                {
                    JpegDirTreeInfo.KeyCounter++;
                    SendMsg(MSG_PICBRO_FRESH_ITEM);
                }
            }
            else
            {
                JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/] = 0;
                JpegDirTreeInfo.KeyCounter = 0;
                {
                    if (JpegDirTreeInfo.JpegDirTotalItem > MAX_ITEM_NUM_PICBRO_DISP)
                    {
                        PicBroJpegDirInit();
                        SendMsg(MSG_PICBRO_ALL_ITEM);
                    }
                    else
                    {
                        SendMsg(MSG_PICBRO_FRESH_ITEM);
                    }
                }
            }
            SendMsg(MSG_MEDIA_SCROLL_PAINT);



            break;

            //--------------------- FFW KEY --------------------------
        case KEY_VAL_PLAY_SHORT_UP://display sub window


//            if(JpegDirTreeInfo.JpegDirTotalItem == 0)
//                break;
//
//            else if(gMusicTypeSelID != SORT_TYPE_PLAY_LIST)
//            {
//                break;
//            }
//
            break;

        case KEY_VAL_ESC_SHORT_UP:

//			if(JpegDirTreeInfo.JpegDirDeep==0)
//            {
//                {
//                    TaskArg.Medialib.CurId= /*gMusicTypeSelID*/;
//                }
//                TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
//                RetVal = 1;
            //TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;	/*<--sanshin_20150630*/
            //TaskSwitch(TASK_ID_MAINMENU, &TaskArg);			/*<--sanshin_20150630*/
/*-->sanshin_20150630*/
			TaskArg.Browser.FileType = FileTypePicture;
			TaskArg.Browser.FileNum  = 0;
			TaskArg.Browser.FromWhere = 2;
			TaskSwitch(TASK_ID_BROWSER, &TaskArg);
			BroswerFlag = FALSE;
/*<--sanshin_20150630*/
            RetVal = 1;
//            }
//            else
//            {
//                JpegDirTreeInfo.JpegDirDeep--;
//
//                if(JpegDirTreeInfo.JpegCurId[JpegDirTreeInfo.JpegDirDeep] >= MAX_ITEM_NUM_PICBRO_DISP-1)
//                {
//                    JpegDirTreeInfo.KeyCounter =  MAX_ITEM_NUM_PICBRO_DISP-1;
//                }
//                else
//                {
//                    JpegDirTreeInfo.KeyCounter = JpegDirTreeInfo.JpegCurId[JpegDirTreeInfo.JpegDirDeep];
//                }
//                PicBroJpegDirInit();
//                SendMsg(MSG_PICBRO_ALL_ITEM);
//               SendMsg(MSG_MEDIA_SCROLL_PAINT);
//            }

            break;

        case KEY_VAL_HOLD_ON://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&PicBroWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&PicBroWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return(RetVal);

}




/*
--------------------------------------------------------------------------------
  Function name : void PicBroSaveAndPostJpegPlayInfo(void)

  Author        : anzhiguo
  Description   : enter media player

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBRO_SORTGET_CODE_
UINT32 PicBroSaveAndPostJpegPlayInfo(void)
{
    UINT16   i;
    UINT16    uiFindResult;
    UINT16    temp1;
    UINT16    tempFileNum;
    UINT16    musicfilenum;
    UINT16    longname[SYS_SUPPROT_STRING_MAX_LEN + 1];
    TASK_ARG TaskArg;
    uint32 RetVal;
    RetVal = 0;

#ifdef _SDCARD_
    if (MemorySelect!=FLASH0)
    {
        MemorySelect = FLASH0;

        FileSysSetup(MemorySelect);
    }
#endif


//    {
//                ThreadDelete(&pMainThread, &MusicThread);
//    }

    //Rk Aaron.sun
//	if((JpegSortInfoAddr.uiJpegSortInfoAddrOffset[JpegDirTreeInfo.JpegDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (JpegDirTreeInfo.JpegDirDeep != 0) )
//	{
//	       {
//			uint16 i, TotalNum, BaseID;
//			TotalNum = 0;
//			for(i = 0; i < JpegDirTreeInfo.JpegDirTotalItem - 1; i++)
//		       {
//				TotalNum += (GetSummaryInfo(JpegSortInfoAddr.ulSortSubInfoSectorAddr[JpegDirTreeInfo.JpegDirDeep], JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep] + i, FIND_SUM_ITEMNUM) - 1);
//			}
//			printf("All Album TotalNum = %d", TotalNum);
//
//			#if 1
//			BaseID = GetSummaryInfo(JpegSortInfoAddr.ulSortSubInfoSectorAddr[JpegDirTreeInfo.JpegDirDeep],  JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep], FIND_SUM_SORTSTART); // first Song;
//			GetSavedJpegDir(&stFindData, JpegSortInfoAddr.ulFileFullInfoSectorAddr, JpegSortInfoAddr.ulFileSortInfoSectorAddr,BaseID);
//	              uiFindResult = RETURN_OK;
//	              printf("st.clus = %d, st.index = %d\n",stFindData.Clus, stFindData.Index);
//	              memcpy((uint8 *)&PicSysFileInfo.FindData , (uint8 *)&stFindData ,8);
//			JpegDirTreeInfo.JpegDirTotalItem =  TotalNum;
//
//			JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep] = BaseID;
//			#else
//			JpegDirTreeInfo.JpegCurId[JpegDirTreeInfo.JpegDirDeep]++;
//			JpegDirTreeInfo.JpegDirDeep++;
//
//			JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep] = PicBroGetBaseId();
//			JpegDirTreeInfo.JpegCurId[JpegDirTreeInfo.JpegDirDeep] = 0;
//
//			GetSavedJpegDir(&stFindData, JpegSortInfoAddr.ulFileFullInfoSectorAddr, JpegSortInfoAddr.ulFileSortInfoSectorAddr,JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep] );
//	              uiFindResult = RETURN_OK;
//	              printf("st.clus = %d, st.index = %d\n",stFindData.Clus, stFindData.Index);
//	              memcpy((uint8 *)&PicSysFileInfo.FindData , (uint8 *)&stFindData ,8);
//			JpegDirTreeInfo.JpegDirTotalItem =  TotalNum;
//			#endif
//
//		}
//	}
//	else
//	{
    #ifdef PIC_MEDIA
    PicBroGetSavedJpegDir(&stFindDataJpeg, JpegSortInfoAddr.ulFileFullInfoSectorAddr, JpegSortInfoAddr.ulFileSortInfoSectorAddr, (JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/]+ JpegDirTreeInfo.JpegDirBaseSortId[0/*JpegDirTreeInfo.JpegDirDeep*/]));
    #endif
    //uiFindResult = FindFileByShortPath(&PicSysFileInfo.Fdt,  &gPathBuffer[0], &gPathBuffer[PATH_SIZE]);
    uiFindResult = RETURN_OK;
    //printf("st.clus = %d, st.index = %d\n",stFindDataJpeg.Clus, stFindDataJpeg.Index);
    memcpy((uint8 *)&PicSysFileInfo.FindData , (uint8 *)&stFindDataJpeg ,8);
//	}


    if (RETURN_OK == uiFindResult)
    {
//        PicSysFileInfo.ucSelPlayType = gMusicTypeSelID;

        PicSysFileInfo.ulFullInfoSectorAddr = JpegSortInfoAddr.ulFileFullInfoSectorAddr;
        PicSysFileInfo.ulSortInfoSectorAddr = JpegSortInfoAddr.ulFileSortInfoSectorAddr;
//        for (i=0; i<=JpegDirTreeInfo.JpegDirDeep; i++)
//        {
//            PicSysFileInfo.uiCurId[i] = JpegDirTreeInfo.JpegCurId[i];
//           PicSysFileInfo.uiBaseSortId[i] = JpegDirTreeInfo.JpegDirBaseSortId[i];
//        }
        PicSysFileInfo.ucCurDeep = 0/*JpegDirTreeInfo.JpegDirDeep*/;
        //PicSysFileInfo.uiBaseSortId = JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep];

        PicSysFileInfo.TotalFiles = JpegDirTreeInfo.JpegDirTotalItem;
        {
            PicSysFileInfo.CurrentFileNum = JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/]+1;
        }
        PicSysFileInfo.pExtStr = PictureFileExtString;
        PicSysFileInfo.PlayedFileNum = PicSysFileInfo.CurrentFileNum;

        #ifdef PIC_MEDIA
        PicBroGetMediaItemInfo(gJpegTypeSelName, JpegSortInfoAddr, (JpegDirTreeInfo.JpegCurId[0]+JpegDirTreeInfo.JpegDirBaseSortId[0]), MEDIA_ID3_SAVE_CHAR_NUM, 0,0);
        #endif
				RetVal = 1;
    }
    else
    {
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&PicBroWin, &MessageBoxWin, &TaskArg);
        SendMsg(MSG_PICBRO_NOFIND_FILE);
        return 0;
    }

#ifdef _PICTURE_

    {
        TaskArg.Pic.FileNum = JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/] + 1;
    }

    gSysConfig.JpegMediaDirTreeInfo.DirDeep = 0/*JpegDirTreeInfo.JpegDirDeep*/;
    //gSysConfig.JpegMediaDirTreeInfo.CurId[JpegDirTreeInfo.JpegDirDeep]= JpegDirTreeInfo.CurId[JpegDirTreeInfo.JpegDirDeep];/*Sanshin*/
    //gSysConfig.JpegMediaDirTreeInfo.DirBaseSortId[JpegDirTreeInfo.JpegDirDeep]= JpegDirTreeInfo.JpegDirBaseSortId[JpegDirTreeInfo.JpegDirDeep];/*Sanshin*/

    /* Sanshin$3$3$+$i */
    gSysConfig.JpegMediaDirTreeInfo.CurId[0]= JpegDirTreeInfo.JpegCurId[0];
    gSysConfig.JpegMediaDirTreeInfo.DirBaseSortId[0]= JpegDirTreeInfo.JpegDirBaseSortId[0];

    gSysConfig.JpegMediaDirTreeInfo.CurId[1]= JpegDirTreeInfo.JpegCurId[1];
    gSysConfig.JpegMediaDirTreeInfo.DirBaseSortId[1]= JpegDirTreeInfo.JpegDirBaseSortId[1];

    gSysConfig.JpegMediaDirTreeInfo.CurId[2]= JpegDirTreeInfo.JpegCurId[2];
    gSysConfig.JpegMediaDirTreeInfo.DirBaseSortId[2]= JpegDirTreeInfo.JpegDirBaseSortId[2];

    gSysConfig.JpegMediaDirTreeInfo.CurId[3]= JpegDirTreeInfo.JpegCurId[3];
    gSysConfig.JpegMediaDirTreeInfo.DirBaseSortId[3]= JpegDirTreeInfo.JpegDirBaseSortId[3];
    /*Sanshin$3$3$^$G*/

	GetLongFileName(PicSysFileInfo.FindData.Clus, PicSysFileInfo.FindData.Index - 1, FS_FAT, PicFileInfo.LongFileName);

	//printf("\n");
    //for (i =0; i < 10;i++)
    //{
    //    printf("%c ",PicFileInfo.LongFileName[i]);
    //}
    //printf("\n");

	gbPicCurid = JpegDirTreeInfo.JpegCurId[0];			/*<--sanshin_20150630*/
	gbPicKeyCnt = JpegDirTreeInfo.KeyCounter;			/*<--sanshin_20150630*/
    //TaskArg.Music.MediaTitleAdd= PicBroTitle;
	TaskArg.Pic.FsType = JPEG_ALL_DB/*FS_FAT*/;				/*<--sanshin 0706*/
	TaskArg.Pic.FromWhere = 1;							/*<--sanshin_20150630*/
	//TaskArg.Pic.StartFileNum = TaskArg.Pic.FileNum;		/*<--sanshin_20150630*/
	TaskArg.Pic.StartFileNum = 0;						/*<--sanshin_20150703*/
    TaskSwitch(TASK_ID_PICTURE, &TaskArg);
    return RetVal;
#endif
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
_ATTR_PICBROWIN_CODE_
void PicBroFomatTheOtherItem(UINT16 *ucFileName)
{
    unsigned int j;

    if (ucFileName[0]==0) // (ucFileName[0]==0x20)
    {
        for (j=0;j<MAX_FILENAME_LEN;j++)
        {
            if (ucFileName[j]!=0)
            {
                break;
            }
        }

        if (j==MAX_FILENAME_LEN)
        {
            //ucFileName[0]=0x003c; // '<'
            //ucFileName[1]=0x004F; // 'O'
            //ucFileName[2]=0x0074; // 't'
            //ucFileName[3]=0x0068; // 'h'
            //ucFileName[4]=0x0065; // 'e'
            //ucFileName[5]=0x0072; // 'r'
            //ucFileName[6]=0x003e; // '>'
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
_ATTR_PICBROWIN_CODE_
UINT16 PicBroGetListItem(UINT16 *pListName, UINT16 uiListNO)
{
    unsigned int i;
    FDT BroFDTBuf;
    FIND_DATA FindDataBro;
    UINT8 buf[PATH_SIZE_JPEG];

    //DEBUG("%s !! IN",__FUNCTION__);
    #ifdef PIC_MEDIA
    PicBroGetMediaItemInfo(pListName, JpegSortInfoAddr, uiListNO, MAX_FILENAME_LEN, 0/*JpegDirTreeInfo.JpegDirDeep*/,1);
    #endif
    FileExtNameRemove(pListName, PictureFileExtString);

    //DEBUG("%s !! OUT",__FUNCTION__);
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name : void GetListItemIconId()

  Author        : Allen
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                Allen              2013/09/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
UINT32 PicBroGetListItemIconId()
{
#if 0
    unsigned int i;
    FDT BroFDTBuf;
    FIND_DATA FindDataBro;
    UINT8 buf[PATH_SIZE];
    UINT16 longname[SYS_SUPPROT_STRING_MAX_LEN + 1];
    UINT32 RetVal = IMG_ID_MUSIC_MENU_SONG;

    switch (JpegDirTreeInfo.JpegDirDeep)
    {
        case 0:
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_FILENAME:
                        RetVal = IMG_ID_MUSIC_MENU_SONG;
                        break;

                    case SORT_TYPE_SEL_ID3SINGER:
                        RetVal = IMG_ID_MUSIC_MENU_ARTIST;
                        break;

                    case SORT_TYPE_SEL_ID3ALBUM:
                        RetVal = IMG_ID_MUSIC_MENU_ALBUM;
                        break;

                    case SORT_TYPE_SEL_GENRE:
                        RetVal = IMG_ID_MUSIC_MENU_GENRE;
                        break;

                    default:
                        break;
                }
                break;
            }
        case 1:
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_ID3SINGER:
                        RetVal = IMG_ID_MUSIC_MENU_ALBUM;
                        break;

                    case SORT_TYPE_SEL_ID3ALBUM:
                        RetVal = IMG_ID_MUSIC_MENU_SONG;
                        break;

                    case SORT_TYPE_SEL_GENRE:
                        RetVal = IMG_ID_MUSIC_MENU_ARTIST;
                        break;

                    default:
                        break;
                }
                break;
            }
        case 2:
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_ID3SINGER:
                        RetVal = IMG_ID_MUSIC_MENU_SONG;
                        break;

                    case SORT_TYPE_SEL_GENRE:
                        RetVal = IMG_ID_MUSIC_MENU_ALBUM;
                        break;

                    default:
                        break;
                }
                break;
            }
        case 3:
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_GENRE:
                        RetVal = IMG_ID_MUSIC_MENU_SONG;
                        break;

                    default:
                        break;
                }

                break;
            }

        default:
            break;
    }
    return RetVal;
#endif
}
/*
--------------------------------------------------------------------------------
  Function name : void   PicBroService(void)


  Author        : anzhiguo
  Description   : C=Le?bD#?iOTJ>D#?i

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_SERVICE_CODE_
UINT32 PicBroService(void)
{

    TASK_ARG TaskArg;
    UINT32 RetVal = 0;

//	DEBUG("   !!!!");

    if (CheckMsg(MSG_PICBRO_NOFIND_FILE))
    {
        if (CheckMsg(MSG_MESSAGEBOX_DESTROY))
        {
            ClearMsg(MSG_PICBRO_NOFIND_FILE);
            ClearMsg(MSG_MESSAGEBOX_DESTROY);
            SendMsg(MSG_PICBRO_DISPLAY_ALL);
        }
    }

    if (TRUE == GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
        {
            //TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;	/*<--sanshin_20150630*/
            //TaskSwitch(TASK_ID_MAINMENU, &TaskArg);		/*<--sanshin_20150630*/
/*-->sanshin_20150630*/
        	TaskArg.Browser.FileType = FileTypePicture;
			TaskArg.Browser.FileNum  = 0;
			TaskArg.Browser.FromWhere = 2;
			TaskSwitch(TASK_ID_BROWSER, &TaskArg);
			BroswerFlag = FALSE;
/*<--sanshin_20150630*/
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
_ATTR_PICBROWIN_SERVICE_CODE_
void PicBroWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
#if 1
    PicturePartInfo     PicPartInfo;
    LCD_RECT		   pRect1;

    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize	= 16;

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
  Function name : void   PicBroDisplay(void)


  Author        : anzhiguo
  Description   : the module of media display

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_SERVICE_CODE_
void   PicBroDisplay(void)
{
#if 1
    PICBRO_STRUCT  *pBro;
    JPEG_DIR_TREE_STRUCT *pJpegDirInfo;
    LCD_RECT      r;
    UINT32 PicBroNFAddr;
    UINT16  i,j,k,m,n;
    UINT16  color_temp;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;

//---->sanshin_20150625
#ifdef THUMB_DEC_INCLUDE
    FILE* JpegHandle;
    UINT16  StartItem = 0;
#endif
//<----sanshin_20150625

	UINT16  TotalItem, TempColor, TempCharSize, TextMode;
    pBro = JpegDirTreeInfo.pPicBro;

    TempColor = LCD_GetColor();
    LCD_SetColor(COLOR_BLACK);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);

//---->sanshin_20150625
#ifdef THUMB_DEC_INCLUDE
    ImageMaxHeight = 12;
    ImageMaxWidth = 12;

    SetPicFileType(IMAGE_PIC);
    IsDisplayBackground(0);
    StartItem = JpegDirTreeInfo.JpegCurId[0] - JpegDirTreeInfo.KeyCounter;
#endif
//<----sanshin_20150625

    //if(CheckMsg(MSG_NEED_PAINT_ALL)&& (PicBroWin.Son != NULL ))	 //Ub8vR*SP#,7qTrSPWS40?ZJ1;a3vOVIAFA 5.20 anzhiguo
    //    return;
    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_PICBRO_DISPLAY_ALL)))
    {
        //Display BackGround
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        //Display Title
        DisplayMenuStrWithIDNum(PICBRO_TITLE_TXT_X, PICBRO_TITLE_TXT_Y, PICBRO_TITLE_TXT_XSIZE,
                                PICBRO_TITLE_TXT_YSIZE, LCD_TEXTALIGN_CENTER, PicBroTitle );

        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+BrowserBatteryLevel,105,146);
        //Send other display message
        SendMsg(MSG_PICBRO_ALL_ITEM);
    }

    if (GetMsg(MSG_PICBRO_ALL_ITEM))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3,143);
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);

#ifdef THUMB_DEC_INCLUDE
    	//---->sanshin_20150625
    	{
    		JpegDirThumbTreeInfo.JpegCurId[0] = JpegDirTreeInfo.JpegCurId[0] - JpegDirTreeInfo.KeyCounter;
			JpegDirThumbTreeInfo.JpegDirBaseSortId[0] = JpegDirTreeInfo.JpegDirBaseSortId[0];
    	}
    	//<----sanshin_20150625
#endif

    	TotalItem = JpegDirTreeInfo.DispTotalItem;

        for (i = 0; ((i < MAX_ITEM_NUM_PICBRO_DISP) && (i < TotalItem )); i++)
        {
#ifdef THUMB_DEC_INCLUDE
        	//---->sanshin_20150625
        	PicBroGetSavedJpegDir(&stFindDataJpeg, JpegSortInfoAddr.ulFileFullInfoSectorAddr, JpegSortInfoAddr.ulFileSortInfoSectorAddr,
					(JpegDirThumbTreeInfo.JpegCurId[0]+JpegDirThumbTreeInfo.JpegDirBaseSortId[0]));
        	//DEBUG("ALL : i=%d, CurId=%d, Clus=0x%x, Index=%d\n", i, JpegDirThumbTreeInfo.JpegCurId[0], stFindDataJpeg.Clus, stFindDataJpeg.Index);//hoshi
        	JpegHandle = (FILE*)FileOpen(NULL, stFindDataJpeg.Clus, stFindDataJpeg.Index - 1, FS_FAT, FileOpenStringR);
            if ((int)JpegHandle == NOT_OPEN_FILE)
            {
                DEBUG("=====FileOpen error=====\n");
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);
            }
            else
            {
                ImageLeft = PicInfo1.x;
                ImageTop = 5 + 17 * i;

                if (!ThumbParse(JpegHandle))
                {
                    //Display file Type Icon
            		DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);
                }

                FileClose((HANDLE)JpegHandle);
            }
        	//<----sanshin_20150625
#else
        	//Display file Type Icon
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);
#endif
            r.x0 = 20;
            r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;

            if (i == JpegDirTreeInfo.KeyCounter)
            {
                DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                {
                    PicBroPrintfBuf = pBro->LongFileName;
                    PicBroWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 3);

                    SendMsg(MSG_PICBRO_DISPFLAG_SCROLL_FILENAME);
                }
                else
                {
                    ClearMsg(MSG_PICBRO_DISPFLAG_SCROLL_FILENAME);
                }
            }

            r.y0 += 2;
            r.y1 =	r.y0+CH_CHAR_XSIZE_12-1;

            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
            pBro = pBro->pNext;

#ifdef THUMB_DEC_INCLUDE
        	//---->sanshin_20150625
        	{
        		JpegDirThumbTreeInfo.JpegCurId[0]++;
        	}
        	//<----sanshin_20150625
#endif
        }
        SendMsg(MSG_MEDIA_SCROLL_PAINT);
    }

    if (GetMsg(MSG_PICBRO_FRESH_ITEM))
    {

        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);

        for (i = 0; i < BROWSER_SCREEN_PER_LINE; i++)
        {
            if ((i == JpegDirTreeInfo.KeyCounter) || (i == JpegDirTreeInfo.PreCounter))
            {
                r.x0 = 20;
                r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
                r.x1 = r.x0 + PicInfo.xSize;
                r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;

                if (i == JpegDirTreeInfo.PreCounter)
                {
                    //DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
                	DisplayPicture_part_xoffset(IMG_ID_BROWSER_BACKGROUND,0,17,128-17,0,3+i*17,17);//<----sanshin_20150625
                }
                //DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);//<----sanshin_20150625
                if (i == JpegDirTreeInfo.KeyCounter)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                    if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                    {
                        PicBroPrintfBuf = pBro->LongFileName;
                        PicBroWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 3);
                        SendMsg(MSG_PICBRO_DISPFLAG_SCROLL_FILENAME);
                    }
                    else
                    {
                        ClearMsg(MSG_PICBRO_DISPFLAG_SCROLL_FILENAME);
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

#ifdef THUMB_DEC_INCLUDE
	//---->sanshin_20150625
	if (GetMsg(MSG_PICBRO_FRESH_THUMBNAIL))
	{
    	JpegDirThumbTreeInfo.JpegCurId[0] = JpegDirTreeInfo.JpegCurId[0];
		JpegDirThumbTreeInfo.JpegDirBaseSortId[0] = JpegDirTreeInfo.JpegDirBaseSortId[0];

		PicBroGetSavedJpegDir(&stFindDataJpeg, JpegSortInfoAddr.ulFileFullInfoSectorAddr, JpegSortInfoAddr.ulFileSortInfoSectorAddr,
				(JpegDirThumbTreeInfo.JpegCurId[0]+JpegDirThumbTreeInfo.JpegDirBaseSortId[0]));
        //DEBUG("FRESH : KeyCounter=%d, CurId=%d, Clus=0x%x, Index=%d\n", JpegDirTreeInfo.KeyCounter, JpegDirThumbTreeInfo.JpegCurId[0], stFindDataJpeg.Clus, stFindDataJpeg.Index);//hoshi
        JpegHandle = (FILE*)FileOpen(NULL, stFindDataJpeg.Clus, stFindDataJpeg.Index - 1, FS_FAT, FileOpenStringR);
        if ((int)JpegHandle == NOT_OPEN_FILE)
        {
            DEBUG("=====FileOpen error=====\n");
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * JpegDirTreeInfo.KeyCounter);
        }
        else
        {
            ImageLeft = PicInfo1.x;
            ImageTop = 5 + 17 * JpegDirTreeInfo.KeyCounter;

            if (!ThumbParse(JpegHandle))
            {
                //Display file Type Icon
        		DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * JpegDirTreeInfo.KeyCounter);
            }

            FileClose((HANDLE)JpegHandle);
        }
	}
	//<----sanshin_20150625
#endif

	if (CheckMsg(MSG_PICBRO_DISPFLAG_SCROLL_FILENAME))//an 4.21
    {
        TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(PicBroPrintfBuf);
        LCD_SetTextMode(TextMode);
    }

    if (GetMsg(MSG_MEDIA_SCROLL_PAINT))
    {

        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL_BLOCK, &PicInfo1);

        if (JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/] > 0)
        {
            i = (PicInfo.ySize - PicInfo1.ySize) * JpegDirTreeInfo.JpegCurId[0/*JpegDirTreeInfo.JpegDirDeep*/]  /( JpegDirTreeInfo.JpegDirTotalItem-1);//+picInfo1.ySize/2;
        }
        else
        {
            i = 0;
        }

        if ((i +PicInfo1.ySize)>PicInfo.ySize)//an 4.22
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

#endif	//<----sanshin_20150818
