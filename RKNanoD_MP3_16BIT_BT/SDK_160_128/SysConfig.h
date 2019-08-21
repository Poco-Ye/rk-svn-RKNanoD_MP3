/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   Main.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

/*
*-------------------------------------------------------------------------------
*
*                            Debug Config
*
*-------------------------------------------------------------------------------
*/
//debug compile switch.
#define _LOG_DEBUG_

#ifdef _LOG_DEBUG_
#define _UART_DEBUG_
//#define _BB_DEBUG_
//#define _BT_DEBUG_
#endif

#define USB_SERIAL_DEBUG
#ifndef USB_SERIAL_DEBUG
#define DEBUG_UART_PORT  1
#else
#define DEBUG_UART_PORT  0
#endif

#ifdef _LOG_DEBUG_
#define DEBUG(format,...)       rk_printf("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define DEBUG2(format,...)      rk_printf2("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)
#define USBDEBUG(format,...)    printf("\nUSB:"format, ##__VA_ARGS__)
#define bb_printf1              rk_printf2
#define BT_DEBUG(format,...)    printf("%s, L: %d: "format"\n", __MODULE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format,...)
#define DEBUG2(format,...)
#define USBDEBUG(format,...)
#define bb_printf1(format,...)
#define rk_printf2(format,...)
#define rk_print_string2(format,...)
#define BT_DEBUG(format,...)
#endif


/*
*-------------------------------------------------------------------------------
*
* Board & Chips Define
*
*-------------------------------------------------------------------------------
*/
#define RKNANOD_EVB_V10     1
#define RKNANOD_EVB_V20     2
#define RKNANOD_EVB_VER     RKNANOD_EVB_V20

#define RKNANOD_L           0
#define RKNANOD_G           1
#define RKNANOD_N           2
#define RKNANOD_M           3
#define RKNANOD_CHIP_TYPE   RKNANOD_G

/*
*-------------------------------------------------------------------------------
* Memory Device Option
* Select which memory device are used
*
*-------------------------------------------------------------------------------
*/
//3 Note: According to Flash Type,Pls Choose SPINOR or EMMC flash
//#define _SPINOR_          //SPI NOR Flash Support
#define _EMMC_            //EMMC Flash Support
//#define _SDCARD_              //SD CARD Support

#ifdef _EMMC_           //eMMC boot
#define FW_IN_DEV       3   //Firmware Stored in: 1:nandflash 2:sipnor 3:emmc 4:sd card
#define RES_IN_DEV      1   //Resources Stored in: 1 FW 2: filesystem
#define SDCARD_PORT     0   //SDCard Port Select: 0 SDIO 1: eMMC
#define FW_ALIGN_SIZE   (1024*1024) //must be 1MB size
#define FW_UPDATE
#endif

#ifdef _SPINOR_         //SPI nor boot
#define FW_IN_DEV       2   //Firmware Stored in: 1:nandflash 2:sipnor 3:emmc 4:sd card
#define RES_IN_DEV      2   //Resources Stored in: 1 FW 2: filesystem; if SPI flash is large enough to store resources, can config to 1
#define SDCARD_PORT     0   //SDCard Port Select: 0 SDIO 1: eMMC
#define FW_ALIGN_SIZE   (64*1024)      //must be multiple 64KB size
#define FW_UPDATE
#endif

#ifdef _SDCARD_         //SD Card boot
//#define FW_IN_DEV       4   //Firmware Stored in: 1:nandflash 2:sipnor 3:emmc 4:sd card
//#define RES_IN_DEV      1   //Resources Stored in: 1 FW 2: filesystem
#endif

//#define DISK_VOLUME
//#define _MULT_DISK_
#define ENABLE_MBR

//-------------------------------------------------------------------------------
//OPT config
//#define     OTP_DATA_ENABLE
#if defined(OTP_DATA_ENABLE)
#define OTP_PARTITION_SIZE      (512*1024)  //必须是block size (512KB)整数倍, 最大1MB
#else
#define OTP_PARTITION_SIZE      0
#endif

#if (OTP_PARTITION_SIZE > (1*1024*1024))
#error Unsupported
#endif

#define OTP_PARTITION_SECS      (OTP_PARTITION_SIZE/512)


/*
*-------------------------------------------------------------------------------
*
*                            Application Modules Option
*
*-------------------------------------------------------------------------------*/
#define _MUSIC_
#define _RADIO_
#define _RECORD_
#define _BROWSER_
#define _SYSSET_
//#define _VIDEO_
#define _PICTURE_
#define _EBOOK_
#define _USB_

#ifndef _SPINOR_
#define _MEDIA_MODULE_
#endif

#define _WATCH_DOG_

//--------------------------------------------------------------------------------
#ifdef _MUSIC_

#define _MP3_DECODE_
#define _WMA_DECODE_
#define _WAV_DECODE_
#define _AAC_DECODE_
#define _FLAC_DECODE_

#define _RK_EQ_
#define _RK_SPECTRUM_
#define _FADE_PROCESS_

#define _RK_ID3_
#define AUDIOHOLDONPLAY             // 断点播放开关
#define FILE_ERROR_DIALOG           //文件格式不支持对话框//sen #20090803#1 all audio file is not support
#define SCROLL_LRC
#define FFW_FFD_PLAY

#ifdef _MEDIA_MODULE_
#define BROSUB
#define FAVOSUB
#define MEDIA_UPDATE
#endif

#endif  //#ifdef _MUSIC_

//--------------------------------------------------------------------------------
#ifdef _PICTURE_
#define _JPG_DECODE_
#define _BMP_DECODE_
#define PIC_MEDIA
#endif

//--------------------------------------------------------------------------------
//FM module compile switch.
#ifdef _RADIO_
#endif

//--------------------------------------------------------------------------------
//record module compile switch.
#ifdef _RECORD_
#define _FM_RECORD_
#define _WAV_ENCODE_
#define _MP3_ENCODE_
//#define _NS_

#define FM_ENC_WAV_HS       FS_48KHz
#define FM_ENC_WAV_FS       FS_44100Hz
#define FM_ENC_MP3_FS       FS_44100Hz

#define MP3_ENC_BITRATE     128
#endif

//--------------------------------------------------------------------------------
//book module compile switch.

//--------------------------------------------------------------------------------
//explorer module compile switch.
#ifdef _BROWSER_

#endif

//--------------------------------------------------------------------------------
//system setting module compile switch.
#ifdef _SYSSET_

#endif

/*
*-------------------------------------------------------------------------------
*
*                            Misc Option
*
*-------------------------------------------------------------------------------
*/

#ifdef B_CORE_DECODE
#define A_CORE_DECODE
#endif

//------------------------------------------------------------------------------
//Audio define
//#define HP_DET_CONFIG
#define _Vol_Tab_General
//#define _Vol_Tab_Europe

#define _BEEP_

//------------------------------------------------------------------------------
//Display define
//#define SUPPORT_YUV
#ifdef SUPPORT_YUV
#define USE_LLP
#endif
#define FILE_ERROR_DIALOG           //Support Dialog Display


//------------------------------------------------------------------------------
#define _M3U_ /*sanshin*/
//#define _M3U_DEBUG_/*sanshin*/

#ifdef _M3U_DEBUG_
    #define DEBUG_M3U(format,...)    rk_printf("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)

    #define DEBUG_PrintShortNameCodeNum(...)      rk_printf("FILE: %s, LINE: %d: ShortNameCodeNum :", __FILE__, __LINE__); \
		for(j = 0 ; j < 11 ; j++){ \
			printf("%x ",##__VA_ARGS__[j]);\
		} \
	   //    rk_printf("\n")

    #define DEBUG_PrintShortName(...)      rk_printf("FILE: %s, LINE: %d: ShortName :  ", __FILE__, __LINE__); \
		for(j = 0 ; j < 11 ; j++){ \
			printf("%c",##__VA_ARGS__[j]);\
		} \
	   //    rk_printf("\n")

    #define DEBUG_PrintLongName(...)   rk_printf("FILE: %s, LINE: %d: LongName :  ", __FILE__, __LINE__); \
		for(j = 0 ; j < StrLenW(##__VA_ARGS__) ; j++){\
			printf("%c",##__VA_ARGS__[j]);\
		}\
	   //    rk_printf("\n")

    #define DEBUG_PrintLongNameCodeNum(...)   rk_printf("FILE: %s, LINE: %d: LongNameCodeNum :  ", __FILE__, __LINE__); \
		for(j = 0 ; j < StrLenW(##__VA_ARGS__)  ; j++){\
			printf("%x ",##__VA_ARGS__[j]);\
		}\
	   //    rk_printf("\n")


#else
     #define DEBUG_M3U(format,...)
	 #define DEBUG_PrintShortName(...)
	 #define DEBUG_PrintLongName(...)
	 #define DEBUG_PrintShortNameCodeNum(...)
	 #define DEBUG_PrintLongNameCodeNum(...)
#endif

/*
*-------------------------------------------------------------------------------
*
*                            LCD Driver Config
*
*-------------------------------------------------------------------------------
*/
#define _FRAME_BUFFER_
#define _DISP_FROM_RAM_             //Support Display Pictures from buffer when recording

//LCD Pixel Define
//#define _LCD_SIZE_20_
#define COLOR_LCD
#define LCD_PIXEL_1         1
#define LCD_PIXEL_16        16

//Horizontal or Verital LCD Configuration.
#define LCD_HORIZONTAL      0
#define LCD_VERTICAL        1

//LCD Type Define
#define LCD_PIXEL           LCD_PIXEL_16//LCD_PIXEL_1
#define LCD_DIRECTION       LCD_VERTICAL

#define LCD_WIDTH 	        128
#define LCD_HEIGHT 	        160

#define LCD_BUF_XSIZE       128
#define LCD_BUF_YSIZE       5

/*
*-------------------------------------------------------------------------------
*
*                            Key Driver Configuration
*
*-------------------------------------------------------------------------------
*/
//Key Num Define
#define KEY_NUM_4		    4
#define KEY_NUM_5		    5
#define KEY_NUM_6		    6
#define KEY_NUM_7		    7
#define KEY_NUM_8		    8

//Config KeyDriver
#define KEY_NUM  	        KEY_NUM_7

//Key Val Define
#define KEY_VAL_NONE        ((UINT32)0x0000)
#define KEY_VAL_PLAY        ((UINT32)0x0001 << 0)
#define KEY_VAL_MENU        ((UINT32)0x0001 << 1)
#define KEY_VAL_FFD         ((UINT32)0x0001 << 2)
#define KEY_VAL_FFW         ((UINT32)0x0001 << 3)
#define KEY_VAL_UP          ((UINT32)0x0001 << 4)
#define KEY_VAL_DOWN        ((UINT32)0x0001 << 5)
#define KEY_VAL_ESC         ((UINT32)0x0001 << 6)
#define KEY_VAL_UNHOLD      ((UINT32)0x0001 << 8)

#define KEY_VAL_UPGRADE     KEY_VAL_MENU
#define KEY_VAL_POWER       KEY_VAL_PLAY
#define KEY_VAL_HOLD        (KEY_VAL_MENU | KEY_VAL_PLAY)
#define KEY_VAL_VOL         KEY_VAL_ESC

#define KEY_VAL_MASK        ((UINT32)0x0fffffff)
#define KEY_VAL_UNMASK      ((UINT32)0xf0000000)

//Bit position define for AD keys.
#define KEY_VAL_ADKEY2		KEY_VAL_MENU
#define KEY_VAL_ADKEY3		KEY_VAL_UP
#define KEY_VAL_ADKEY4      KEY_VAL_FFW
#define KEY_VAL_ADKEY5      KEY_VAL_FFD
#define KEY_VAL_ADKEY6      KEY_VAL_DOWN
#define KEY_VAL_ADKEY7      KEY_VAL_ESC

//AdKey Reference Voltage define
// adc_val = 0 / 2.5 *1024
#define ADKEY2_MIN          ((0   +   0) / 2)
#define ADKEY2_MAX          ((0   + 147) / 2)

#define ADKEY3_MIN          ((147 +   0) / 2) //up
#define ADKEY3_MAX          ((147 + 330) / 2)

#define ADKEY4_MIN          ((330 + 147) / 2) //FFD
#define ADKEY4_MAX          ((330 + 522) / 2)

#define ADKEY5_MIN          ((522 + 330) / 2) //FFW
#define ADKEY5_MAX          ((522 + 780) / 2)

#define ADKEY6_MIN          ((780 + 522) / 2) //Down
#define ADKEY6_MAX          ((780 + 956) / 2)

#define ADKEY7_MIN          ((956 + 780) / 2) //ESC
#define ADKEY7_MAX          ((956 + 1024) / 2)

/*
*-------------------------------------------------------------------------------
*
*                            Voltage About
*
*-------------------------------------------------------------------------------
*/
//degree for battery power.
#define BATTERY_LEVEL       5

//bind of PWM output.
#define BL_LEVEL_MAX        5       //backlight 5 degree.

#define BL_PWM_RATE_MIN     30
#define BL_PWM_RATE_MAX     80
#define BL_PWM_RATE_STEP    ((BL_PWM_RATE_MAX - BL_PWM_RATE_MIN) / (BL_LEVEL_MAX))


/*
*-------------------------------------------------------------------------------
*
*                            EVK Board Configuration
*
*-------------------------------------------------------------------------------
*/
//#define RKNANOC_EVK_V20
#define RKNANOC_BT_EVB_V10
#define _USE_SHELL_

/*
*-------------------------------------------------------------------------------
*
*                            GPIO Config
*
*-------------------------------------------------------------------------------
*/
#define HP_DET              GPIOPortB_Pin3
/*
*-------------------------------------------------------------------------------
*
*                            Language Define
*
*-------------------------------------------------------------------------------
*/
//zhuzhe
#define LANGUAGE_CHINESE_S               0      //Simplified Chinese.
#define LANGUAGE_CHINESE_T               1      //Traditional Chinese
#define LANGUAGE_ENGLISH                 2      //Englis
#define LANGUAGE_KOREAN                  3      //Korean
#define LANGUAGE_JAPANESE                4      //Japanese
#define LANGUAGE_SPAISH                  9      //Spanish
#define LANGUAGE_FRENCH                  5      //French
#define LANGUAGE_GERMAN                  6      //German
#define LANGUAGE_ITALIAN                 10      //Italian
#define LANGUAGE_PORTUGUESE              7      //Portuguess
#define LANGUAGE_RUSSIAN                 8      //Russian

#define LANGUAGE_SWEDISH                11      //Swedish
#define LANGUAGE_THAI                   12      //Thai
#define LANGUAGE_POLAND                 13      //Polish
#define LANGUAGE_DENISH                 14      //Danish
#define LANGUAGE_DUTCH                  15      //Dutch
#define LANGUAGE_HELLENIC               16      //Greek
#define LANGUAGE_CZECHIC                17      //Czech
#define LANGUAGE_TURKIC                 18      //Turkish
#define LANGUAGE_RABBINIC               19      //Hebrew
#define LANGUAGE_ARABIC                 20      //Arabic
#define LANGUAGE_MAX_COUNT              21
#define EVK_LANGUAGE_MAX_COUNT          9
#define DEFAULT_LANGUE                  LANGUAGE_CHINESE_S

//-----------------------------------------------------------------------------
//configer bluetooth function.
#define BT_CHIP_CC2564      0
#define BT_CHIP_CC2564B     1
#define BT_CHIP_RTL8761     2

#define BT_CHIP_CONFIG    BT_CHIP_RTL8761
//#define BT_CHIP_CONFIG      BT_CHIP_CC2564
#define BT_ENABLE_SET_MAC
#define BT_UART_INTERFACE_H4   1
#define BT_UART_INTERFACE_H5   2
#define BT_UART_INTERFACE_CONFIG BT_UART_INTERFACE_H5

#define _BLUETOOTH_
#ifdef _BLUETOOTH_

#define BT_VENDORDEP_ENABLE

    #define _A2DP_SOUCRE_  //_A2DP_SINK_ and _A2DP_SOUCRE_ can't support at same project, choose one of them
    #ifdef _A2DP_SOUCRE_
		#define _AVRCP_
        #define _SBC_ENCODE_
        #define BT_OFF_TIME_OUT         500//tick
        #define BT_CONNECT_TIME_OUT     2000//tick
        #define BT_LINK_SUPERVISION_TIMEOUT 20//second MAX is 40s
        #define SSRC                    //for bluetooth function
        #define PIN_CODE_WIN
        #define NUMERIC_REQ_WIN
        #define _AVRCP_VERSION_1_4_
        //#define BT_HOST_SNIFF
        //#define HAVE_BLE
    #endif

    #ifdef HAVE_BLE
        //#define BLE_SMALL_MEMROY
        //#define BLE_ENABLE_SM
    #endif

    #ifdef _OBEX_
        #define _OPP_               //for build lib
    #endif
    //#define ENABLE_BQB_RF_TEST               // when BQB RF test , define the ENABLE_BQB_RF_TEST
    #define ENABLE_BQB_PTS_TEST               // when BQB RF test , define the ENABLE_BQB_RF_TEST
    #ifndef ENABLE_BQB_RF_TEST          // when BQB RF test , define the ENABLE_BQB_RF_TEST
        #ifndef _A2DP_SOUCRE_
            //#define ENABLE_PAIR_TIMER
            //#define PAIR_TIME_OUT       6000    //60s ,when time out ,bt discoverable_disable and go to sleep
            //#define ENABLE_DEEP_SLEEP
            //#define ENABLE_NFC
        #endif
    #endif

    //#define BT_VOICENOTIFY
    #ifdef BT_VOICENOTIFY
        #define VOICE_NOTIFY_VOL        24
    #endif

//#define BT_VCC_ON_GPIO_CH           0xFF//no use
//#define BT_VCC_ON_GPIO_PIN          0xFF//no use
#define BT_VCC_ON_GPIO_CH           GPIO_CH2
#define BT_VCC_ON_GPIO_PIN          GPIOPortA_Pin2

#define BT_POWER_GPIO_CH            GPIO_CH0
#define BT_POWER_GPIO_PIN           GPIOPortB_Pin5
#define BT_HOST_RX_CH               GPIO_CH2
#define BT_HOST_RX_PIN              GPIOPortC_Pin1
#define BT_HOST_TX_CH               GPIO_CH2
#define BT_HOST_TX_PIN              GPIOPortC_Pin0
#define BT_HOST_CTS_CH              GPIO_CH2
#define BT_HOST_CTS_PIN             GPIOPortB_Pin7
#define BT_HOST_RTS_CH              GPIO_CH2
#define BT_HOST_RTS_PIN             GPIOPortB_Pin6

#define BT_HCI_UART_ID  			UART_CH1
#define BT_UART_CH      			UART_CH1_PA

#define BT_UART_INT_ID 				INT_ID_UART1
#define BT_GPIO_INT_ID 				INT_ID_GPIO0
#define BT_HCI_SERVER_INT_ID 		INT_ID_UART5

#define BT_H5_TX_INT_ID 			INT_ID_UART3
//#define BT_HOST_SNIFF
#define BT_SBC_PROCESS_INT_ID       INT_ID_UART2
#endif

//-----------------------------------------------------------------------------

/*
*-------------------------------------------------------------------------------
*
*                            Misc Define
*
*-------------------------------------------------------------------------------
*/
//#define DEEP_SLEEP
//#define KEY_RESUME_ENABLE

/*
********************************************************************************
*
*                         End of Include.h
*
********************************************************************************
*/
#endif //USE_DRIVER_TEST_CONFIG

