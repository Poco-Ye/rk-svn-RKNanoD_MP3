/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   MainMenu.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MAINMENU_H_
#define _MAINMENU_H_

#undef  EXT
#ifdef _IN_MAINMENU_
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
//section define
//main menu permanent code
#define _ATTR_MAIN_MENU_CODE_         __attribute__((section("MainMenuCode")))
#define _ATTR_MAIN_MENU_DATA_         __attribute__((section("MainMenuData")))
#define _ATTR_MAIN_MENU_BSS_          __attribute__((section("MainMenuBss"),zero_init))

//main menu initial code
#define _ATTR_MAIN_MENU_INIT_CODE_    __attribute__((section("MainMenuInitCode")))
#define _ATTR_MAIN_MENU_INIT_DATA_    __attribute__((section("MainMenuInitData")))
#define _ATTR_MAIN_MENU_INIT_BSS_     __attribute__((section("MainMenuInitBss"),zero_init))

//main menu auti-initial code
#define _ATTR_MAIN_MENU_DEINIT_CODE_  __attribute__((section("MainMenuDeInitCode")))
#define _ATTR_MAIN_MENU_DEINIT_DATA_  __attribute__((section("MainMenuDeInitData")))
#define _ATTR_MAIN_MENU_DEINIT_BSS_   __attribute__((section("MainMenuDeInitBss"),zero_init))

//main menu content switch code
#define _ATTR_MAIN_MENU_SERVICE_CODE_ __attribute__((section("MainMenuServiceCode")))
#define _ATTR_MAIN_MENU_SERVICE_DATA_ __attribute__((section("MainMenuServiceData")))
#define _ATTR_MAIN_MENU_SERVICE_BSS_  __attribute__((section("MainMenuServiceBss"),zero_init))

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
typedef enum
{
    #if 1//def _MUSIC_
    MAINMENU_ID_MUSIC= (UINT32)0,//audio module
    #endif

    #if 1//def _RADIO_
    MAINMENU_ID_RADIO,           //FM
    #endif

    #if 1//def _PICTURE_
    MAINMENU_ID_PICTURE,         //picture
    #endif

    #if 1//def _EBOOK_
    MAINMENU_ID_EBOOK,          //picture
    #endif

    #if 1//def _RECORD_
    MAINMENU_ID_RECORD,
    #endif

    #if 1//def _BROWSER_
    MAINMENU_ID_BROWSER,         //brower
    #endif

    #if 1//def _GAME_
    MAINMENU_ID_M3U,          //picture
    #endif

    #if 1//def _SYSSET_
    MAINMENU_ID_SETMENU,          //setting module
    #endif

    MAINMENU_ID_MAXNUM,

} MAINMENU_ID;

typedef struct _MainConfig
{
	BOOL       DispFlag;	//����Ŀ��ʾ��
	UINT8          Item;		//�����Ŀ�����˵��еĵڼ��� /������  /���Ը���MAINMENU_ID���ж�˳��
	MAINMENU_ID  TaskId;	 //����Ŀִ�е�������
	UINT16 ImageId; //����Ŀ����ʾͼƬID
	UINT16 TextId;	//����Ŀ����ʾ����ID
} Main_Config;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
#define MainMenuBatteryLevel       gBattery.Batt_Level
#define MainmenuHoldState          HoldState
//#define MAINMENU_ID_BLUETOOTH      MAINMENU_ID_RADIO
_ATTR_MAIN_MENU_BSS_ EXT UINT16    MenuId; //moduel id of main menu interface
_ATTR_MAIN_MENU_BSS_ EXT UINT16    HomeDemoMode; //moduel id of main menu interface

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
extern void MainMenuInit(void *pArg);
extern void MainMenuDeInit(void);

extern UINT32 MainMenuService(void);
extern UINT32 MainMenuKey(void);
extern void MainMenuDisplay(void);

INT16 MainMenuModeKey(void);


/*
--------------------------------------------------------------------------------

  Description:  window sturcture definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_MAINMENU_
_ATTR_MAIN_MENU_DATA_ WIN MainMenuWin = {

    NULL,
    NULL,

    MainMenuService,               //window service handle function.
    MainMenuKey,                   //window key service handle function.
    MainMenuDisplay,               //window display service handle function.

    MainMenuInit,                  //window initial handle function.
    MainMenuDeInit                 //window auti-initial handle function.

};
#else
_ATTR_MAIN_MENU_DATA_ EXT WIN MainMenuWin;
#endif


#ifdef _IN_MAINMENU_
_ATTR_MAIN_MENU_DATA_   Main_Config MainConfig[]= {
	{1, 	MAINMENU_ID_MUSIC,			MAINMENU_ID_MUSIC,			IMG_ID_MAINMENU_MUSIC,		SID_MUSIC}, 		//����

#if 1//def _RADIO_
    {1,     MAINMENU_ID_RADIO,          MAINMENU_ID_RADIO,          IMG_ID_MAINMENU_FM,         SID_RADIO},         //������
#endif

#if 1//def _PICTURE_
	{1, 	MAINMENU_ID_PICTURE,		MAINMENU_ID_PICTURE,		IMG_ID_MAINMENU_PHOTO,		SID_PHOTO}, 		//ͼƬ
#endif

#if 1//def _EBOOK_
	{1, 	MAINMENU_ID_EBOOK,			MAINMENU_ID_EBOOK,			IMG_ID_MAINMENU_BOOK,		SID_TEXT},			//������
#endif

#if 1//def _RECORD_
	{1, 	MAINMENU_ID_RECORD,			MAINMENU_ID_RECORD, 		IMG_ID_MAINMENU_RECOD,		SID_RECORD},		//¼��
#endif

#if 1// def _BROWSER_
	{1, 	MAINMENU_ID_BROWSER,		MAINMENU_ID_BROWSER,		IMG_ID_MAINMENU_BROWSER,	SID_EXPLORER},		//�����
#endif

#if 1//def _M3U_
	{1, 	MAINMENU_ID_M3U,			MAINMENU_ID_M3U,			IMG_ID_MAINMENU_GAME,		SID_GAME},			//��Ϸ
#endif

#if 1//def _SYSSET_
	{1, 	MAINMENU_ID_SETMENU,		MAINMENU_ID_SETMENU,		IMG_ID_MAINMENU_SETMENU,	SID_SETTINGS},		//����
#endif

};
#else
_ATTR_MAIN_MENU_DATA_ EXT  Main_Config MainConfig;
#endif

/*
********************************************************************************
*
*                         End of Win.h
*
********************************************************************************
*/
#endif

