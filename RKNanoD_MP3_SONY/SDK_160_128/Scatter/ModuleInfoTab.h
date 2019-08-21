/*
********************************************************************************
*                   Copyright (c) 2009,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   ModuleInfoTab.h
*
* Description:  define module struct information
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2009-02-06         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MODULEINFO_H
#define _MODULEINFO_H

#undef  EXT
#ifdef _IN_MODULEINFO_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Module Info
*
*-------------------------------------------------------------------------------
*/
//Module Section ID Define
typedef enum
{
    //SysCode
    MODULE_ID_SYSCODE = (UINT32)0,                  //00
    MODULE_ID_SYSINIT,                              //01
    MODULE_ID_PMU,
    MODULE_ID_FLASH_PROG,                           //50
    MODULE_ID_LCD_1,
    MODULE_ID_LCD_2,

    MODULE_ID_MDB_WIN,                              //55
    MODULE_ID_CHARGE_WIN,                           //56
    MODULE_ID_FS_MEM_GET,
    MODULE_ID_FW_UPGRADE,

    //USB
    MODULE_ID_USB,                                  //02
    MODULE_ID_USBCONTROL,
    MODULE_ID_USB_MSC,

    //IMAGE
    MODULE_ID_PICTURE,                              //13
    MODULE_ID_PICTURE_CONTROL,                      //14
    MODULE_ID_JPG_DECODE,                           //15
    MODULE_ID_BMP_DECODE,

    //UI
    MODULE_ID_MAINMENU,                             //03
    MODULE_ID_MEDIALIBWIN,   //6
    MODULE_ID_MEDIABRO,// 9
    MODULE_ID_MEDIABRO_SORTGET,//11
    MODULE_ID_MEDIABROSUB,//12
    MODULE_ID_MEDIAFAVOSUB,//13
    MODULE_ID_MUSIC,                                //06
    MODULE_ID_RADIOWIN,                             //09
    MODULE_ID_RECORDWIN,                            //16
    MODULE_ID_TEXTWIN,                              //19
    MODULE_ID_BROWSER,                              //23
#ifdef _M3U_
    MODULE_ID_M3UBRO,                               //<----sanshin_20150616
    MODULE_ID_M3UBRO_SORTGET,                       //<----sanshin_20150616
    MODULE_ID_M3U,                                  //<----sanshin_20150616
#endif
    MODULE_ID_PICBRO,                              //<---- sanshin //
    MODULE_ID_SETMENU,                              //24

    MODULE_ID_AUDIO_CONTROL,                        //31
    MODULE_ID_AUDIO_INIT,                           //31
    MODULE_ID_AUDIO_EQ,                             //34
    MODULE_ID_AUDIO_RKEQ,                           //33

    MODULE_ID_AUDIO_ID3,
    MODULE_ID_FILEINFO_SAVE,                        //38
    MODULE_ID_FILEINFO_SORT,                        //39
    MODULE_ID_FAVO_RESET,                           //40
    MODULE_ID_MP3_DECODE,                           //36

    MODULE_ID_WMA_COMMON,                           //37
    MODULE_ID_WAV_DECODE,                           //43
    MODULE_ID_AAC_DECODE,
    MODULE_ID_FLAC_DECODE,

    MODULE_ID_FM_CONTROL,                           //29

    MODULE_ID_FM_DRIVER1,                           //31
    MODULE_ID_FM_DRIVER2,

    MODULE_ID_RECORD_CONTROL,                       //32
    MODULE_ID_RECORD_MSADPCM,                       //33
    MODULE_ID_RECORD_MP3,

    MODULE_ID_SEID0000_8K,
    MODULE_ID_SEID0001_8K,
    MODULE_ID_SEID0002_8K,
    MODULE_ID_SEID0003_8K,
    MODULE_ID_SEID0004_8K,
    MODULE_ID_SEID0005_8K,
    MODULE_ID_SEID0006_8K,
    MODULE_ID_SEID0007_8K,

    MODULE_ID_SEID0000_11K,
    MODULE_ID_SEID0001_11K,
    MODULE_ID_SEID0002_11K,
    MODULE_ID_SEID0003_11K,
    MODULE_ID_SEID0004_11K,
    MODULE_ID_SEID0005_11K,
    MODULE_ID_SEID0006_11K,
    MODULE_ID_SEID0007_11K,

    MODULE_ID_SEID0000_12K,
    MODULE_ID_SEID0001_12K,
    MODULE_ID_SEID0002_12K,
    MODULE_ID_SEID0003_12K,
    MODULE_ID_SEID0004_12K,
    MODULE_ID_SEID0005_12K,
    MODULE_ID_SEID0006_12K,
    MODULE_ID_SEID0007_12K,

    MODULE_ID_SEID0000_16K,
    MODULE_ID_SEID0001_16K,
    MODULE_ID_SEID0002_16K,
    MODULE_ID_SEID0003_16K,
    MODULE_ID_SEID0004_16K,
    MODULE_ID_SEID0005_16K,
    MODULE_ID_SEID0006_16K,
    MODULE_ID_SEID0007_16K,

    MODULE_ID_SEID0000_22K,
    MODULE_ID_SEID0001_22K,
    MODULE_ID_SEID0002_22K,
    MODULE_ID_SEID0003_22K,
    MODULE_ID_SEID0004_22K,
    MODULE_ID_SEID0005_22K,
    MODULE_ID_SEID0006_22K,
    MODULE_ID_SEID0007_22K,

    MODULE_ID_SEID0000_24K,
    MODULE_ID_SEID0001_24K,
    MODULE_ID_SEID0002_24K,
    MODULE_ID_SEID0003_24K,
    MODULE_ID_SEID0004_24K,
    MODULE_ID_SEID0005_24K,
    MODULE_ID_SEID0006_24K,
    MODULE_ID_SEID0007_24K,

    MODULE_ID_SEID0000_32K,
    MODULE_ID_SEID0001_32K,
    MODULE_ID_SEID0002_32K,
    MODULE_ID_SEID0003_32K,
    MODULE_ID_SEID0004_32K,
    MODULE_ID_SEID0005_32K,
    MODULE_ID_SEID0006_32K,
    MODULE_ID_SEID0007_32K,

    MODULE_ID_SEID0000_44K,
    MODULE_ID_SEID0001_44K,
    MODULE_ID_SEID0002_44K,
    MODULE_ID_SEID0003_44K,
    MODULE_ID_SEID0004_44K,
    MODULE_ID_SEID0005_44K,
    MODULE_ID_SEID0006_44K,
    MODULE_ID_SEID0007_44K,

    MODULE_ID_SEID0000_48K,
    MODULE_ID_SEID0001_48K,
    MODULE_ID_SEID0002_48K,
    MODULE_ID_SEID0003_48K,
    MODULE_ID_SEID0004_48K,
    MODULE_ID_SEID0005_48K,
    MODULE_ID_SEID0006_48K,
    MODULE_ID_SEID0007_48K,

    MODULE_CP1251_UNICODE_TABLE,
    MODULE_CP932_UNICODE_TABLE,
    MODULE_CP932_UNICODE_TABLE1,
    MODULE_CP932_UNICODE_TABLE2,
    MODULE_CP950_UNICODE_TABLE,
    MODULE_CP949_UNICODE_TABLE,
    MODULE_CP949_UNICODE_TABLE1,

    // B core
    MODULE_ID_BB_CODE,
    MODULE_ID_MP3_ENCODE_BIN,
    MODULE_ID_MP3_DECODE_BIN,
    MODULE_ID_WMA_DECODE_BIN,
    MODULE_ID_WAV_DECODE_BIN,
    MODULE_ID_AAC_DECODE_BIN,
    MODULE_ID_FLAC_DECODE_BIN,

    //Bluetooth
    MODULE_ID_BT,
    MODULE_ID_LWBT_UARTIF,
    MODULE_ID_LWBT_INIT,
    MODULE_ID_LWBT,
    MODULE_ID_SBC_ENCODE,
    MODULE_ID_SRC,
    MODULE_ID_SRC_TABLE_48_44,
    MODULE_ID_SRC_TABLE_32_44,
    MODULE_ID_SRC_TABLE_24_44,
    MODULE_ID_SRC_TABLE_22_44,
    MODULE_ID_SRC_TABLE_16_44,
    MODULE_ID_SRC_TABLE_12_44,
    MODULE_ID_SRC_TABLE_11_44,
    MODULE_ID_SRC_TABLE_8_44,
    MODULE_ID_SRC_TABLE_48_44120,
    MODULE_ID_SRC_TABLE_44_44120,

    MODULE_ID_INIT_SCRIPT,

    MAX_MODULE_NUM

} MODULE_ID;

//Code Info Table
typedef struct _CODE_INFO_T
{
    UINT32 CodeLoadBase;
    UINT32 CodeImageBase;
    UINT32 CodeImageLength;

    UINT32 DataLoadBase;
    UINT32 DataImageBase;
    UINT32 DataImageLength;

    UINT32 BssImageBase;
    UINT32 BssImageLength;

} CODE_INFO_T;

//Module Info
typedef struct _CODE_INFO_TABLE
{
    UINT32                  ModuleNum;
    CODE_INFO_T             Module[MAX_MODULE_NUM];

} CODE_INFO_TABLE;

/*
*-------------------------------------------------------------------------------
*
*                           FM Driver Info
*
*-------------------------------------------------------------------------------
*/
//FM Driver Index
typedef enum
{
    MODULE_ID_FM_1    = (UINT32)0,
    MODULE_ID_FM_2,

    FM_DRIVER_MAX

} FM_DRIVER_ID;

//FM driver
typedef struct _FM_DRIVER_TABLE_T
{
    void   (*Tuner_SetInitArea)(UINT8 );
    void   (*Tuner_SetFrequency)(UINT32 , UINT8 , BOOL ,UINT16 );
    void   (*Tuner_SetStereo)(BOOL );
    void   (*Tuner_SetVolume)(uint8);

    void   (*Tuner_PowerOffDeinit)(void);
    UINT16 (*Tuner_SearchByHand)(UINT16 , UINT32 * );
    void   (*Tuner_PowerDown)(void);
    void   (*Tuner_MuteControl)(bool );
    BOOLEAN (*GetStereoStatus)(void);

} FM_DRIVER_TABLE_T;

typedef struct _FM_DRIVER_INFO_T
{
    //FM function list
    UINT32 FMDriverIndex;
    UINT32 FMDriverMax;
    FM_DRIVER_TABLE_T FmDriver[FM_DRIVER_MAX];

} FM_DRIVER_INFO_T;

/*
*-------------------------------------------------------------------------------
*
*                           LCD Driver Info
*
*-------------------------------------------------------------------------------
*/
//LCD Driver Index
typedef enum
{
    LCD_DRIVER_LCD_1 = (UINT32)0,
    LCD_DRIVER_LCD_2,
    LCD_DRIVER_MAX

} LCD_DRIVER_ID;

//Lcd driver
typedef struct _LCD_DRIVER_TABLE_T
{
    //Lcd function list
    void(*pLcd_WriteRAM_Prepare)(void);
    void(*pLcd_Init)(void);
    void(*pLcd_SendData)(UINT16);
    void(*pLcd_SetWindow)(UINT16 ,UINT16 ,UINT16,INT16);
    void(*pLcd_Clear)(UINT16);
    int32(*pLcd_DMATranfer)(UINT16,UINT16,UINT16,UINT16,UINT16*);
    void(*pLcd_Standby)(void);
    void(*pLcd_WakeUp)(void);
    void(*pLcd_MP4_Init)(void);
    void(*pLcd_MP4_DeInit)(void);
    void(*pLcd_SetPixel)(UINT16 x, UINT16 y, UINT16 data);
    void(*pLcd_Buffer_Display1)(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1);
    void(*pLcd_ClrSrc)(void);
    void(*pLcd_ClrRect)(int x0, int y0, int x1, int y1);
    void(*LCDDEV_FillRect)(int x0, int y0, int x1, int y1);
     // UINT32 reserve;

} LCD_DRIVER_TABLE_T;

typedef struct _LCD_DRIVER_INFO_T
{
    //Lcd driver function
    UINT32 LcdDriverIndex;
    UINT32 LcdDriverMax;
    LCD_DRIVER_TABLE_T LcdDriver[LCD_DRIVER_MAX];

} LCD_DRIVER_INFO_T;

/*
*-------------------------------------------------------------------------------
*
*                           Default Para
*
*-------------------------------------------------------------------------------
*/
//System Default Para
typedef struct _SYSTEM_DEFAULT_PARA_T
{
    //system default setting
    UINT32 SDEnable;
    UINT32 FMEnable;
    UINT32 KeyNumIndex;

    UINT32 gLanguage;
    UINT32 Volume;

    UINT32 BLMode;
    UINT32 BLevel;
    UINT32 BLtime;

    UINT32 SettingPowerOffTime;
    UINT32 IdlePowerOffTime;

    UINT32 AvlsEnabled;
    UINT32 BeepEnabled;
    UINT32 FactoryVolLimit;

    //Music default setting
    UINT32 MusicRepMode;
    UINT32 MusicPlayOrder;
    UINT32 MusicBassBoost;
    UINT32 MusicEqSel;
    UINT32 MusicPlayFxSel;

    //Video default setting

    //Radio default setting
    UINT32 FMArea;
    UINT32 FMStereo;
    UINT32 FMScanSensitivity;

    //Record default setting

    //Picture default setting
    UINT32 PicAutoPlayTime;

    //Text default setting
    UINT32 TextAutoPlayTime;

    //Image ID
    uint32 ImageIndex;

    //VolumeTableIndex ID
    //uint32 VolumeTableIndex; // 1 : General  ;  0 : Europe  //VolumeTableIndex will not be reset when reset default setting - weslly

#ifdef _BLUETOOTH_
    char BTDevName[BT_LOCAL_DEV_NAME_MAX + 2];
#endif
    //mutiple language
    uint32 MenuMaxItem;
    uint32 LanguageTotle;   //TOTAL_LANAUAGE_NUM,LANGUAGE_MAX_COUNT
    uint32 LanguagesEnableTbl[64];//support 256 languange

} SYSTEM_DEFAULT_PARA_T;

/*
*-------------------------------------------------------------------------------
*
*                           Module Info
*
*-------------------------------------------------------------------------------
*/
//Firmware Info
typedef struct _FIRMWARE_INFO_T
{
    UINT32                  LoadStartBase;

    CODE_INFO_TABLE         ModuleInfo;

    SYSTEM_DEFAULT_PARA_T   SysDefaultPara;

    FM_DRIVER_INFO_T        FMDriverTable;

    LCD_DRIVER_INFO_T       LCDDriverTable;

} FIRMWARE_INFO_T;

/*
********************************************************************************
*
*                         End of ModuleInfoTab.h
*
********************************************************************************
*/
#endif

