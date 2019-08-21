/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   StartLoadTab.c
*
* Description:  ����ģ����Ϣ����ģ�����ʱ��Ҫʹ��
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2009-02-06         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_STARTLOAD_

#include "SysInclude.h"

extern uint32 Load$$MODULE_INFO$$Base;

//SysCode
extern uint32  Load$$SYS_CODE$$Base;
extern uint32 Image$$SYS_CODE$$Base;
extern uint32 Image$$SYS_CODE$$Length;
extern uint32  Load$$SYS_DATA$$Base;
extern uint32 Image$$SYS_DATA$$RW$$Base;
extern uint32 Image$$SYS_DATA$$RW$$Length;
extern uint32 Image$$SYS_DATA$$ZI$$Base;
extern uint32 Image$$SYS_DATA$$ZI$$Length;

#ifndef _JTAG_DEBUG_
extern uint32  Load$$SYS_INIT_CODE$$Base;
extern uint32 Image$$SYS_INIT_CODE$$Base;
extern uint32 Image$$SYS_INIT_CODE$$Length;
extern uint32  Load$$SYS_INIT_DATA$$Base;
extern uint32 Image$$SYS_INIT_DATA$$RW$$Base;
extern uint32 Image$$SYS_INIT_DATA$$RW$$Length;
extern uint32 Image$$SYS_INIT_DATA$$ZI$$Base;
extern uint32 Image$$SYS_INIT_DATA$$ZI$$Length;


extern uint32 Load$$LCD_DRIVER1_CODE$$Base;
extern uint32 Image$$LCD_DRIVER1_CODE$$Base;
extern uint32 Image$$LCD_DRIVER1_CODE$$Length;
extern uint32 Load$$LCD_DRIVER1_DATA$$Base;
extern uint32 Image$$LCD_DRIVER1_DATA$$RW$$Base;
extern uint32 Image$$LCD_DRIVER1_DATA$$RW$$Length;
extern uint32 Image$$LCD_DRIVER1_DATA$$ZI$$Base;
extern uint32 Image$$LCD_DRIVER1_DATA$$ZI$$Length;

extern uint32 Load$$LCD_DRIVER2_CODE$$Base;
extern uint32 Image$$LCD_DRIVER2_CODE$$Base;
extern uint32 Image$$LCD_DRIVER2_CODE$$Length;
extern uint32 Load$$LCD_DRIVER2_DATA$$Base;
extern uint32 Image$$LCD_DRIVER2_DATA$$RW$$Base;
extern uint32 Image$$LCD_DRIVER2_DATA$$RW$$Length;
extern uint32 Image$$LCD_DRIVER2_DATA$$ZI$$Base;
extern uint32 Image$$LCD_DRIVER2_DATA$$ZI$$Length;

//#ifdef _MEDIA_MODULE_
//fileinfosave
extern uint32  Load$$FILE_INFO_SAVE_CODE$$Base;
extern uint32 Image$$FILE_INFO_SAVE_CODE$$Base;
extern uint32 Image$$FILE_INFO_SAVE_CODE$$Length;
extern uint32  Load$$FILE_INFO_SAVE_DATA$$Base;
extern uint32 Image$$FILE_INFO_SAVE_DATA$$RW$$Base;
extern uint32 Image$$FILE_INFO_SAVE_DATA$$RW$$Length;
extern uint32 Image$$FILE_INFO_SAVE_DATA$$ZI$$Base;
extern uint32 Image$$FILE_INFO_SAVE_DATA$$ZI$$Length;

//fileinfosort
extern uint32  Load$$FILE_INFO_SORT_CODE$$Base;
extern uint32 Image$$FILE_INFO_SORT_CODE$$Base;
extern uint32 Image$$FILE_INFO_SORT_CODE$$Length;
extern uint32  Load$$FILE_INFO_SORT_DATA$$Base;
extern uint32 Image$$FILE_INFO_SORT_DATA$$RW$$Base;
extern uint32 Image$$FILE_INFO_SORT_DATA$$RW$$Length;
extern uint32 Image$$FILE_INFO_SORT_DATA$$ZI$$Base;
extern uint32 Image$$FILE_INFO_SORT_DATA$$ZI$$Length;

//FavoReset
extern uint32  Load$$FAVORESET_CODE$$Base;
extern uint32 Image$$FAVORESET_CODE$$Base;
extern uint32 Image$$FAVORESET_CODE$$Length;
extern uint32  Load$$FAVORESET_DATA$$Base;
extern uint32 Image$$FAVORESET_DATA$$RW$$Base;
extern uint32 Image$$FAVORESET_DATA$$RW$$Length;
extern uint32 Image$$FAVORESET_DATA$$ZI$$Base;
extern uint32 Image$$FAVORESET_DATA$$ZI$$Length;

//#ifdef _MEDIA_MODULE_
//ý���
extern uint32  Load$$MEDIALIBWIN_CODE$$Base;
extern uint32 Image$$MEDIALIBWIN_CODE$$Base;
extern uint32 Image$$MEDIALIBWIN_CODE$$Length;
extern uint32  Load$$MEDIALIBWIN_DATA$$Base;
extern uint32 Image$$MEDIALIBWIN_DATA$$RW$$Base;
extern uint32 Image$$MEDIALIBWIN_DATA$$RW$$Length;
extern uint32 Image$$MEDIALIBWIN_DATA$$ZI$$Base;
extern uint32 Image$$MEDIALIBWIN_DATA$$ZI$$Length;

//ý���mediabro
extern uint32  Load$$MEDIABROWIN_CODE$$Base;
extern uint32 Image$$MEDIABROWIN_CODE$$Base;
extern uint32 Image$$MEDIABROWIN_CODE$$Length;
extern uint32  Load$$MEDIABROWIN_DATA$$Base;
extern uint32 Image$$MEDIABROWIN_DATA$$RW$$Base;
extern uint32 Image$$MEDIABROWIN_DATA$$RW$$Length;
extern uint32 Image$$MEDIABROWIN_DATA$$ZI$$Base;
extern uint32 Image$$MEDIABROWIN_DATA$$ZI$$Length;

extern uint32  Load$$MEDIABROSUBWIN_CODE$$Base;
extern uint32 Image$$MEDIABROSUBWIN_CODE$$Base;
extern uint32 Image$$MEDIABROSUBWIN_CODE$$Length;
extern uint32  Load$$MEDIABROSUBWIN_DATA$$Base;
extern uint32 Image$$MEDIABROSUBWIN_DATA$$RW$$Base;
extern uint32 Image$$MEDIABROSUBWIN_DATA$$RW$$Length;
extern uint32 Image$$MEDIABROSUBWIN_DATA$$ZI$$Base;
extern uint32 Image$$MEDIABROSUBWIN_DATA$$ZI$$Length;

extern uint32  Load$$MEDIAFAVOSUBWIN_CODE$$Base;
extern uint32 Image$$MEDIAFAVOSUBWIN_CODE$$Base;
extern uint32 Image$$MEDIAFAVOSUBWIN_CODE$$Length;
extern uint32  Load$$MEDIAFAVOSUBWIN_DATA$$Base;
extern uint32 Image$$MEDIAFAVOSUBWIN_DATA$$RW$$Base;
extern uint32 Image$$MEDIAFAVOSUBWIN_DATA$$RW$$Length;
extern uint32 Image$$MEDIAFAVOSUBWIN_DATA$$ZI$$Base;
extern uint32 Image$$MEDIAFAVOSUBWIN_DATA$$ZI$$Length;


//ý���SVC
extern uint32  Load$$MEDIABRO_SORTGET_CODE$$Base;
extern uint32 Image$$MEDIABRO_SORTGET_CODE$$Base;
extern uint32 Image$$MEDIABRO_SORTGET_CODE$$Length;
extern uint32  Load$$MEDIABRO_SORTGET_DATA$$Base;
extern uint32 Image$$MEDIABRO_SORTGET_DATA$$RW$$Base;
extern uint32 Image$$MEDIABRO_SORTGET_DATA$$RW$$Length;
extern uint32 Image$$MEDIABRO_SORTGET_DATA$$ZI$$Base;
extern uint32 Image$$MEDIABRO_SORTGET_DATA$$ZI$$Length;


//USB Window UI code & data
extern uint32 Load$$USBWIN_CODE$$Base;
extern uint32 Image$$USBWIN_CODE$$Base;
extern uint32 Image$$USBWIN_CODE$$Length;
extern uint32 Load$$USBWIN_DATA$$Base;
extern uint32 Image$$USBWIN_DATA$$RW$$Base;
extern uint32 Image$$USBWIN_DATA$$RW$$Length;
extern uint32 Image$$USBWIN_DATA$$ZI$$Base;
extern uint32 Image$$USBWIN_DATA$$ZI$$Length;

//USB Control & driver code & data
extern uint32 Load$$USBCONTROL_CODE$$Base;
extern uint32 Image$$USBCONTROL_CODE$$Base;
extern uint32 Image$$USBCONTROL_CODE$$Length;
extern uint32 Load$$USBCONTROL_DATA$$Base;
extern uint32 Image$$USBCONTROL_DATA$$RW$$Base;
extern uint32 Image$$USBCONTROL_DATA$$RW$$Length;
extern uint32 Image$$USBCONTROL_DATA$$ZI$$Base;
extern uint32 Image$$USBCONTROL_DATA$$ZI$$Length;

//USB Audio code & data
extern uint32 Load$$USBAUDIO_CODE$$Base;
extern uint32 Image$$USBAUDIO_CODE$$Base;
extern uint32 Image$$USBAUDIO_CODE$$Length;
extern uint32 Load$$USBAUDIO_DATA$$Base;
extern uint32 Image$$USBAUDIO_DATA$$RW$$Base;
extern uint32 Image$$USBAUDIO_DATA$$RW$$Length;
extern uint32 Image$$USBAUDIO_DATA$$ZI$$Base;
extern uint32 Image$$USBAUDIO_DATA$$ZI$$Length;

//USB Serial
extern uint32 Load$$USBSERIAL_CODE$$Base;
extern uint32 Image$$USBSERIAL_CODE$$Base;
extern uint32 Image$$USBSERIAL_CODE$$Length;
extern uint32 Load$$USBSERIAL_DATA$$Base;
extern uint32 Image$$USBSERIAL_DATA$$RW$$Base;
extern uint32 Image$$USBSERIAL_DATA$$RW$$Length;
extern uint32 Image$$USBSERIAL_DATA$$ZI$$Base;
extern uint32 Image$$USBSERIAL_DATA$$ZI$$Length;

//USB MSC code & data
extern uint32 Load$$USBMSC_CODE$$Base;
extern uint32 Image$$USBMSC_CODE$$Base;
extern uint32 Image$$USBMSC_CODE$$Length;
extern uint32 Load$$USBMSC_DATA$$Base;
extern uint32 Image$$USBMSC_DATA$$RW$$Base;
extern uint32 Image$$USBMSC_DATA$$RW$$Length;
extern uint32 Image$$USBMSC_DATA$$ZI$$Base;
extern uint32 Image$$USBMSC_DATA$$ZI$$Length;

//Audio Decode
extern uint32  Load$$AUDIO_CONTROL_CODE$$Base;
extern uint32 Image$$AUDIO_CONTROL_CODE$$Base;
extern uint32 Image$$AUDIO_CONTROL_CODE$$Length;
extern uint32  Load$$AUDIO_CONTROL_DATA$$Base;
extern uint32 Image$$AUDIO_CONTROL_DATA$$RW$$Base;
extern uint32 Image$$AUDIO_CONTROL_DATA$$RW$$Length;
extern uint32 Image$$AUDIO_CONTROL_DATA$$ZI$$Base;
extern uint32 Image$$AUDIO_CONTROL_DATA$$ZI$$Length;

//Audio Decode
extern uint32  Load$$AUDIO_CONTROL_INIT_CODE$$Base;
extern uint32 Image$$AUDIO_CONTROL_INIT_CODE$$Base;
extern uint32 Image$$AUDIO_CONTROL_INIT_CODE$$Length;
extern uint32  Load$$AUDIO_CONTROL_INIT_DATA$$Base;
extern uint32 Image$$AUDIO_CONTROL_INIT_DATA$$RW$$Base;
extern uint32 Image$$AUDIO_CONTROL_INIT_DATA$$RW$$Length;
extern uint32 Image$$AUDIO_CONTROL_INIT_DATA$$ZI$$Base;
extern uint32 Image$$AUDIO_CONTROL_INIT_DATA$$ZI$$Length;

//Audio RKEq
extern uint32  Load$$AUDIO_RKEQ_CODE$$Base;
extern uint32 Image$$AUDIO_RKEQ_CODE$$Base;
extern uint32 Image$$AUDIO_RKEQ_CODE$$Length;
extern uint32  Load$$AUDIO_RKEQ_DATA$$Base;
extern uint32 Image$$AUDIO_RKEQ_DATA$$RW$$Base;
extern uint32 Image$$AUDIO_RKEQ_DATA$$RW$$Length;
extern uint32 Image$$AUDIO_RKEQ_DATA$$ZI$$Base;
extern uint32 Image$$AUDIO_RKEQ_DATA$$ZI$$Length;

//Audio EQ
extern uint32  Load$$AUDIO_EQ_CODE$$Base;
extern uint32 Image$$AUDIO_EQ_CODE$$Base;
extern uint32 Image$$AUDIO_EQ_CODE$$Length;
extern uint32  Load$$AUDIO_EQ_DATA$$Base;
extern uint32 Image$$AUDIO_EQ_DATA$$RW$$Base;
extern uint32 Image$$AUDIO_EQ_DATA$$RW$$Length;
extern uint32 Image$$AUDIO_EQ_DATA$$ZI$$Base;
extern uint32 Image$$AUDIO_EQ_DATA$$ZI$$Length;

extern uint32  Load$$AUDIO_ID3_CODE$$Base;
extern uint32 Image$$AUDIO_ID3_CODE$$Base;
extern uint32 Image$$AUDIO_ID3_CODE$$Length;
extern uint32  Load$$AUDIO_ID3_DATA$$Base;
extern uint32 Image$$AUDIO_ID3_DATA$$RW$$Base;
extern uint32 Image$$AUDIO_ID3_DATA$$RW$$Length;
extern uint32 Image$$AUDIO_ID3_DATA$$ZI$$Base;
extern uint32 Image$$AUDIO_ID3_DATA$$ZI$$Length;


//MP3
extern uint32  Load$$MP3_DECODE_CODE$$Base;
extern uint32 Image$$MP3_DECODE_CODE$$Base;
extern uint32 Image$$MP3_DECODE_CODE$$Length;
extern uint32  Load$$MP3_DECODE_DATA$$Base;
extern uint32 Image$$MP3_DECODE_DATA$$RW$$Base;
extern uint32 Image$$MP3_DECODE_DATA$$RW$$Length;
extern uint32 Image$$MP3_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$MP3_DECODE_DATA$$ZI$$Length;
//WMA
//common
extern uint32  Load$$WMA_COMMON_CODE$$Base;
extern uint32 Image$$WMA_COMMON_CODE$$Base;
extern uint32 Image$$WMA_COMMON_CODE$$Length;
extern uint32  Load$$WMA_COMMON_DATA$$Base;
extern uint32 Image$$WMA_COMMON_DATA$$RW$$Base;
extern uint32 Image$$WMA_COMMON_DATA$$RW$$Length;
extern uint32 Image$$WMA_COMMON_DATA$$ZI$$Base;
extern uint32 Image$$WMA_COMMON_DATA$$ZI$$Length;

//WAV
extern uint32  Load$$WAV_DECODE_CODE$$Base;
extern uint32 Image$$WAV_DECODE_CODE$$Base;
extern uint32 Image$$WAV_DECODE_CODE$$Length;
extern uint32  Load$$WAV_DECODE_DATA$$Base;
extern uint32 Image$$WAV_DECODE_DATA$$RW$$Base;
extern uint32 Image$$WAV_DECODE_DATA$$RW$$Length;
extern uint32 Image$$WAV_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$WAV_DECODE_DATA$$ZI$$Length;

//FLAC
extern uint32  Load$$FLAC_DECODE_CODE$$Base;
extern uint32 Image$$FLAC_DECODE_CODE$$Base;
extern uint32 Image$$FLAC_DECODE_CODE$$Length;
extern uint32  Load$$FLAC_DECODE_DATA$$Base;
extern uint32 Image$$FLAC_DECODE_DATA$$RW$$Base;
extern uint32 Image$$FLAC_DECODE_DATA$$RW$$Length;
extern uint32 Image$$FLAC_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$FLAC_DECODE_DATA$$ZI$$Length;

//AAC
extern uint32  Load$$AAC_DECODE_CODE$$Base;
extern uint32 Image$$AAC_DECODE_CODE$$Base;
extern uint32 Image$$AAC_DECODE_CODE$$Length;
extern uint32  Load$$AAC_DECODE_DATA$$Base;
extern uint32 Image$$AAC_DECODE_DATA$$RW$$Base;
extern uint32 Image$$AAC_DECODE_DATA$$RW$$Length;
extern uint32 Image$$AAC_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$AAC_DECODE_DATA$$ZI$$Length;

//APE
extern uint32  Load$$APE_DECODE_CODE$$Base;
extern uint32 Image$$APE_DECODE_CODE$$Base;
extern uint32 Image$$APE_DECODE_CODE$$Length;
extern uint32  Load$$APE_DECODE_DATA$$Base;
extern uint32 Image$$APE_DECODE_DATA$$RW$$Base;
extern uint32 Image$$APE_DECODE_DATA$$RW$$Length;
extern uint32 Image$$APE_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$APE_DECODE_DATA$$ZI$$Length;

//OGG

extern uint32  Load$$OGG_DECODE_CODE$$Base;
extern uint32 Image$$OGG_DECODE_CODE$$Base;
extern uint32 Image$$OGG_DECODE_CODE$$Length;
extern uint32  Load$$OGG_DECODE_DATA$$Base;
extern uint32 Image$$OGG_DECODE_DATA$$RW$$Base;
extern uint32 Image$$OGG_DECODE_DATA$$RW$$Length;
extern uint32 Image$$OGG_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$OGG_DECODE_DATA$$ZI$$Length;

//HFLAC
extern uint32  Load$$FLAC_HDECODE_CODE$$Base;
extern uint32 Image$$FLAC_HDECODE_CODE$$Base;
extern uint32 Image$$FLAC_HDECODE_CODE$$Length;
extern uint32  Load$$FLAC_HDECODE_DATA$$Base;
extern uint32 Image$$FLAC_HDECODE_DATA$$RW$$Base;
extern uint32 Image$$FLAC_HDECODE_DATA$$RW$$Length;
extern uint32 Image$$FLAC_HDECODE_DATA$$ZI$$Base;
extern uint32 Image$$FLAC_HDECODE_DATA$$ZI$$Length;

//HALAC
extern uint32  Load$$ALAC_HDECODE_CODE$$Base;
extern uint32 Image$$ALAC_HDECODE_CODE$$Base;
extern uint32 Image$$ALAC_HDECODE_CODE$$Length;
extern uint32  Load$$ALAC_HDECODE_DATA$$Base;
extern uint32 Image$$ALAC_HDECODE_DATA$$RW$$Base;
extern uint32 Image$$ALAC_HDECODE_DATA$$RW$$Length;
extern uint32 Image$$ALAC_HDECODE_DATA$$ZI$$Base;
extern uint32 Image$$ALAC_HDECODE_DATA$$ZI$$Length;

//HAPE
extern uint32  Load$$APE_HDECODE_CODE$$Base;
extern uint32 Image$$APE_HDECODE_CODE$$Base;
extern uint32 Image$$APE_HDECODE_CODE$$Length;
extern uint32  Load$$APE_HDECODE_DATA$$Base;
extern uint32 Image$$APE_HDECODE_DATA$$RW$$Base;
extern uint32 Image$$APE_HDECODE_DATA$$RW$$Length;
extern uint32 Image$$APE_HDECODE_DATA$$ZI$$Base;
extern uint32 Image$$APE_HDECODE_DATA$$ZI$$Length;

//DSD
extern uint32  Load$$DSDIFF_DECODE_CODE$$Base;
extern uint32 Image$$DSDIFF_DECODE_CODE$$Base;
extern uint32 Image$$DSDIFF_DECODE_CODE$$Length;
extern uint32  Load$$DSDIFF_DECODE_DATA$$Base;
extern uint32 Image$$DSDIFF_DECODE_DATA$$RW$$Base;
extern uint32 Image$$DSDIFF_DECODE_DATA$$RW$$Length;
extern uint32 Image$$DSDIFF_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$DSDIFF_DECODE_DATA$$ZI$$Length;

//DSF
extern uint32  Load$$DSF_DECODE_CODE$$Base;
extern uint32 Image$$DSF_DECODE_CODE$$Base;
extern uint32 Image$$DSF_DECODE_CODE$$Length;
extern uint32  Load$$DSF_DECODE_DATA$$Base;
extern uint32 Image$$DSF_DECODE_DATA$$RW$$Base;
extern uint32 Image$$DSF_DECODE_DATA$$RW$$Length;
extern uint32 Image$$DSF_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$DSF_DECODE_DATA$$ZI$$Length;

//Audio Encode
extern uint32  Load$$ENCODE_CODE$$Base;
extern uint32 Image$$ENCODE_CODE$$Base;
extern uint32 Image$$ENCODE_CODE$$Length;
extern uint32  Load$$ENCODE_DATA$$Base;
extern uint32 Image$$ENCODE_DATA$$RW$$Base;
extern uint32 Image$$ENCODE_DATA$$RW$$Length;
extern uint32 Image$$ENCODE_DATA$$ZI$$Base;
extern uint32 Image$$ENCODE_DATA$$ZI$$Length;

//Encode MSADPCM
extern uint32  Load$$ENCODE_MSADPCM_CODE$$Base;
extern uint32 Image$$ENCODE_MSADPCM_CODE$$Base;
extern uint32 Image$$ENCODE_MSADPCM_CODE$$Length;
extern uint32  Load$$ENCODE_MSADPCM_DATA$$Base;
extern uint32 Image$$ENCODE_MSADPCM_DATA$$RW$$Base;
extern uint32 Image$$ENCODE_MSADPCM_DATA$$RW$$Length;
extern uint32 Image$$ENCODE_MSADPCM_DATA$$ZI$$Base;
extern uint32 Image$$ENCODE_MSADPCM_DATA$$ZI$$Length;

extern uint32  Load$$ENCODE_MP3_CODE$$Base;
extern uint32 Image$$ENCODE_MP3_CODE$$Base;
extern uint32 Image$$ENCODE_MP3_CODE$$Length;
extern uint32  Load$$ENCODE_MP3_DATA$$Base;
extern uint32 Image$$ENCODE_MP3_DATA$$RW$$Base;
extern uint32 Image$$ENCODE_MP3_DATA$$RW$$Length;
extern uint32 Image$$ENCODE_MP3_DATA$$ZI$$Base;
extern uint32 Image$$ENCODE_MP3_DATA$$ZI$$Length;

//RecordControl
extern uint32  Load$$RECORD_CONTROL_CODE$$Base;
extern uint32 Image$$RECORD_CONTROL_CODE$$Base;
extern uint32 Image$$RECORD_CONTROL_CODE$$Length;
extern uint32  Load$$RECORD_CONTROL_DATA$$Base;
extern uint32 Image$$RECORD_CONTROL_DATA$$RW$$Base;
extern uint32 Image$$RECORD_CONTROL_DATA$$RW$$Length;
extern uint32 Image$$RECORD_CONTROL_DATA$$ZI$$Base;
extern uint32 Image$$RECORD_CONTROL_DATA$$ZI$$Length;

//UI MainMenu
extern uint32  Load$$MAINMENU_CODE$$Base;
extern uint32 Image$$MAINMENU_CODE$$Base;
extern uint32 Image$$MAINMENU_CODE$$Length;
extern uint32  Load$$MAINMENU_DATA$$Base;
extern uint32 Image$$MAINMENU_DATA$$RW$$Base;
extern uint32 Image$$MAINMENU_DATA$$RW$$Length;
extern uint32 Image$$MAINMENU_DATA$$ZI$$Base;
extern uint32 Image$$MAINMENU_DATA$$ZI$$Length;

//UI Music
extern uint32  Load$$MUSICWIN_CODE$$Base;
extern uint32 Image$$MUSICWIN_CODE$$Base;
extern uint32 Image$$MUSICWIN_CODE$$Length;
extern uint32  Load$$MUSICWIN_DATA$$Base;
extern uint32 Image$$MUSICWIN_DATA$$RW$$Base;
extern uint32 Image$$MUSICWIN_DATA$$RW$$Length;
extern uint32 Image$$MUSICWIN_DATA$$ZI$$Base;
extern uint32 Image$$MUSICWIN_DATA$$ZI$$Length;

//UI Video
extern uint32  Load$$VIDEOWIN_CODE$$Base;
extern uint32 Image$$VIDEOWIN_CODE$$Base;
extern uint32 Image$$VIDEOWIN_CODE$$Length;
extern uint32  Load$$VIDEOWIN_DATA$$Base;
extern uint32 Image$$VIDEOWIN_DATA$$RW$$Base;
extern uint32 Image$$VIDEOWIN_DATA$$RW$$Length;
extern uint32 Image$$VIDEOWIN_DATA$$ZI$$Base;
extern uint32 Image$$VIDEOWIN_DATA$$ZI$$Length;

//videocontrol
extern uint32 Load$$VIDEO_CONTROL_CODE$$Base;
extern uint32 Image$$VIDEO_CONTROL_CODE$$Base;
extern uint32 Image$$VIDEO_CONTROL_CODE$$Length;
extern uint32 Load$$VIDEO_CONTROL_DATA$$Base;
extern uint32 Image$$VIDEO_CONTROL_DATA$$RW$$Base;
extern uint32 Image$$VIDEO_CONTROL_DATA$$RW$$Length;
extern uint32 Image$$VIDEO_CONTROL_DATA$$ZI$$Base;
extern uint32 Image$$VIDEO_CONTROL_DATA$$ZI$$Length;

// VIDEO_MP2
extern uint32  Load$$MP2_DECODE_CODE$$Base;
extern uint32 Image$$MP2_DECODE_CODE$$Base;
extern uint32 Image$$MP2_DECODE_CODE$$Length;
extern uint32  Load$$MP2_DECODE_DATA$$Base;
extern uint32 Image$$MP2_DECODE_DATA$$RW$$Base;
extern uint32 Image$$MP2_DECODE_DATA$$RW$$Length;
extern uint32 Image$$MP2_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$MP2_DECODE_DATA$$ZI$$Length;

//AVI
extern uint32  Load$$AVI_DECODE_CODE$$Base;
extern uint32 Image$$AVI_DECODE_CODE$$Base;
extern uint32 Image$$AVI_DECODE_CODE$$Length;
extern uint32  Load$$AVI_DECODE_DATA$$Base;
extern uint32 Image$$AVI_DECODE_DATA$$RW$$Base;
extern uint32 Image$$AVI_DECODE_DATA$$RW$$Length;
extern uint32 Image$$AVI_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$AVI_DECODE_DATA$$ZI$$Length;

//Radio
extern uint32  Load$$RADIOWIN_CODE$$Base;
extern uint32 Image$$RADIOWIN_CODE$$Base;
extern uint32 Image$$RADIOWIN_CODE$$Length;
extern uint32  Load$$RADIOWIN_DATA$$Base;
extern uint32 Image$$RADIOWIN_DATA$$RW$$Base;
extern uint32 Image$$RADIOWIN_DATA$$RW$$Length;
extern uint32 Image$$RADIOWIN_DATA$$ZI$$Base;
extern uint32 Image$$RADIOWIN_DATA$$ZI$$Length;

//Picture
extern uint32  Load$$PICWIN_CODE$$Base;
extern uint32 Image$$PICWIN_CODE$$Base;
extern uint32 Image$$PICWIN_CODE$$Length;
extern uint32  Load$$PICWIN_DATA$$Base;
extern uint32 Image$$PICWIN_DATA$$RW$$Base;
extern uint32 Image$$PICWIN_DATA$$RW$$Length;
extern uint32 Image$$PICWIN_DATA$$ZI$$Base;
extern uint32 Image$$PICWIN_DATA$$ZI$$Length;
//Picture ����
extern uint32  Load$$IMAGE_CONTROL_CODE$$Base;
extern uint32 Image$$IMAGE_CONTROL_CODE$$Base;
extern uint32 Image$$IMAGE_CONTROL_CODE$$Length;
extern uint32  Load$$IMAGE_CONTROL_DATA$$Base;
extern uint32 Image$$IMAGE_CONTROL_DATA$$RW$$Base;
extern uint32 Image$$IMAGE_CONTROL_DATA$$RW$$Length;
extern uint32 Image$$IMAGE_CONTROL_DATA$$ZI$$Base;
extern uint32 Image$$IMAGE_CONTROL_DATA$$ZI$$Length;
//JPG
extern uint32  Load$$JPG_DECODE_CODE$$Base;
extern uint32 Image$$JPG_DECODE_CODE$$Base;
extern uint32 Image$$JPG_DECODE_CODE$$Length;
extern uint32  Load$$JPG_DECODE_DATA$$Base;
extern uint32 Image$$JPG_DECODE_DATA$$RW$$Base;
extern uint32 Image$$JPG_DECODE_DATA$$RW$$Length;
extern uint32 Image$$JPG_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$JPG_DECODE_DATA$$ZI$$Length;

//BMP
extern uint32  Load$$BMP_DECODE_CODE$$Base;
extern uint32 Image$$BMP_DECODE_CODE$$Base;
extern uint32 Image$$BMP_DECODE_CODE$$Length;
extern uint32  Load$$BMP_DECODE_DATA$$Base;
extern uint32 Image$$BMP_DECODE_DATA$$RW$$Base;
extern uint32 Image$$BMP_DECODE_DATA$$RW$$Length;
extern uint32 Image$$BMP_DECODE_DATA$$ZI$$Base;
extern uint32 Image$$BMP_DECODE_DATA$$ZI$$Length;

//FM
extern uint32 Load$$FM_CONTROL_CODE$$Base;
extern uint32 Image$$FM_CONTROL_CODE$$Base;
extern uint32 Image$$FM_CONTROL_CODE$$Length;
extern uint32 Load$$FM_CONTROL_DATA$$Base;
extern uint32 Image$$FM_CONTROL_DATA$$RW$$Base;
extern uint32 Image$$FM_CONTROL_DATA$$RW$$Length;
extern uint32 Image$$FM_CONTROL_DATA$$ZI$$Base;
extern uint32 Image$$FM_CONTROL_DATA$$ZI$$Length;

// FM DRIVER
extern uint32 Load$$FM_DRIVER1_CODE$$Base;
extern uint32 Image$$FM_DRIVER1_CODE$$Base;
extern uint32 Image$$FM_DRIVER1_CODE$$Length;
extern uint32 Load$$FM_DRIVER1_DATA$$Base;
extern uint32 Image$$FM_DRIVER1_DATA$$RW$$Base;
extern uint32 Image$$FM_DRIVER1_DATA$$RW$$Length;
extern uint32 Image$$FM_DRIVER1_DATA$$ZI$$Base;
extern uint32 Image$$FM_DRIVER1_DATA$$ZI$$Length;

extern uint32 Load$$FM_DRIVER2_CODE$$Base;
extern uint32 Image$$FM_DRIVER2_CODE$$Base;
extern uint32 Image$$FM_DRIVER2_CODE$$Length;
extern uint32 Load$$FM_DRIVER2_DATA$$Base;
extern uint32 Image$$FM_DRIVER2_DATA$$RW$$Base;
extern uint32 Image$$FM_DRIVER2_DATA$$RW$$Length;
extern uint32 Image$$FM_DRIVER2_DATA$$ZI$$Base;
extern uint32 Image$$FM_DRIVER2_DATA$$ZI$$Length;


//Record
extern uint32  Load$$RECORDWIN_CODE$$Base;
extern uint32 Image$$RECORDWIN_CODE$$Base;
extern uint32 Image$$RECORDWIN_CODE$$Length;
extern uint32  Load$$RECORDWIN_DATA$$Base;
extern uint32 Image$$RECORDWIN_DATA$$RW$$Base;
extern uint32 Image$$RECORDWIN_DATA$$RW$$Length;
extern uint32 Image$$RECORDWIN_DATA$$ZI$$Base;
extern uint32 Image$$RECORDWIN_DATA$$ZI$$Length;

//Text
extern uint32  Load$$TEXTWIN_CODE$$Base;
extern uint32 Image$$TEXTWIN_CODE$$Base;
extern uint32 Image$$TEXTWIN_CODE$$Length;
extern uint32  Load$$TEXTWIN_DATA$$Base;
extern uint32 Image$$TEXTWIN_DATA$$RW$$Base;
extern uint32 Image$$TEXTWIN_DATA$$RW$$Length;
extern uint32 Image$$TEXTWIN_DATA$$ZI$$Base;
extern uint32 Image$$TEXTWIN_DATA$$ZI$$Length;

//Browser
extern uint32  Load$$BROWSER_CODE$$Base;
extern uint32 Image$$BROWSER_CODE$$Base;
extern uint32 Image$$BROWSER_CODE$$Length;
extern uint32  Load$$BROWSER_DATA$$Base;
extern uint32 Image$$BROWSER_DATA$$RW$$Base;
extern uint32 Image$$BROWSER_DATA$$RW$$Length;
extern uint32 Image$$BROWSER_DATA$$ZI$$Base;
extern uint32 Image$$BROWSER_DATA$$ZI$$Length;

#ifdef _M3U_
//M3UBRO
extern uint32  Load$$M3UBRO_SORTGET_CODE$$Base;
extern uint32 Image$$M3UBRO_SORTGET_CODE$$Base;
extern uint32 Image$$M3UBRO_SORTGET_CODE$$Length;
extern uint32  Load$$M3UBRO_SORTGET_DATA$$Base;
extern uint32 Image$$M3UBRO_SORTGET_DATA$$RW$$Base;
extern uint32 Image$$M3UBRO_SORTGET_DATA$$RW$$Length;
extern uint32 Image$$M3UBRO_SORTGET_DATA$$ZI$$Base;
extern uint32 Image$$M3UBRO_SORTGET_DATA$$ZI$$Length;

extern uint32  Load$$M3UBROWIN_CODE$$Base;
extern uint32 Image$$M3UBROWIN_CODE$$Base;
extern uint32 Image$$M3UBROWIN_CODE$$Length;
extern uint32  Load$$M3UBROWIN_DATA$$Base;
extern uint32 Image$$M3UBROWIN_DATA$$RW$$Base;
extern uint32 Image$$M3UBROWIN_DATA$$RW$$Length;
extern uint32 Image$$M3UBROWIN_DATA$$ZI$$Base;
extern uint32 Image$$M3UBROWIN_DATA$$ZI$$Length;
//M3U
extern uint32  Load$$M3U_CODE$$Base;
extern uint32 Image$$M3U_CODE$$Base;
extern uint32 Image$$M3U_CODE$$Length;
extern uint32  Load$$M3U_DATA$$Base;
extern uint32 Image$$M3U_DATA$$RW$$Base;
extern uint32 Image$$M3U_DATA$$RW$$Length;
extern uint32 Image$$M3U_DATA$$ZI$$Base;
extern uint32 Image$$M3U_DATA$$ZI$$Length;
#endif
//PICBRO
extern uint32  Load$$PICBROWIN_CODE$$Base;
extern uint32 Image$$PICBROWIN_CODE$$Base;
extern uint32 Image$$PICBROWIN_CODE$$Length;
extern uint32  Load$$PICBROWIN_DATA$$Base;
extern uint32 Image$$PICBROWIN_DATA$$RW$$Base;
extern uint32 Image$$PICBROWIN_DATA$$RW$$Length;
extern uint32 Image$$PICBROWIN_DATA$$ZI$$Base;
extern uint32 Image$$PICBROWIN_DATA$$ZI$$Length;

//Set Menu
extern uint32  Load$$SETMENU_CODE$$Base;
extern uint32 Image$$SETMENU_CODE$$Base;
extern uint32 Image$$SETMENU_CODE$$Length;
extern uint32  Load$$SETMENU_DATA$$Base;
extern uint32 Image$$SETMENU_DATA$$RW$$Base;
extern uint32 Image$$SETMENU_DATA$$RW$$Length;
extern uint32 Image$$SETMENU_DATA$$ZI$$Base;
extern uint32 Image$$SETMENU_DATA$$ZI$$Length;

//Flash Write
extern uint32  Load$$FLASH_WRITE_CODE$$Base;
extern uint32 Image$$FLASH_WRITE_CODE$$Base;
extern uint32 Image$$FLASH_WRITE_CODE$$Length;
extern uint32  Load$$FLASH_WRITE_DATA$$Base;
extern uint32 Image$$FLASH_WRITE_DATA$$RW$$Base;
extern uint32 Image$$FLASH_WRITE_DATA$$RW$$Length;
extern uint32 Image$$FLASH_WRITE_DATA$$ZI$$Base;
extern uint32 Image$$FLASH_WRITE_DATA$$ZI$$Length;


//MDB WIN
extern uint32  Load$$MDB_WIN_CODE$$Base;
extern uint32 Image$$MDB_WIN_CODE$$Base;
extern uint32 Image$$MDB_WIN_CODE$$Length;
extern uint32  Load$$MDB_WIN_DATA$$Base;
extern uint32 Image$$MDB_WIN_DATA$$RW$$Base;
extern uint32 Image$$MDB_WIN_DATA$$RW$$Length;
extern uint32 Image$$MDB_WIN_DATA$$ZI$$Base;
extern uint32 Image$$MDB_WIN_DATA$$ZI$$Length;

//CHARGE WIN
extern uint32  Load$$CHARGE_WIN_CODE$$Base;
extern uint32 Image$$CHARGE_WIN_CODE$$Base;
extern uint32 Image$$CHARGE_WIN_CODE$$Length;
extern uint32  Load$$CHARGE_WIN_DATA$$Base;
extern uint32 Image$$CHARGE_WIN_DATA$$RW$$Base;
extern uint32 Image$$CHARGE_WIN_DATA$$RW$$Length;
extern uint32 Image$$CHARGE_WIN_DATA$$ZI$$Base;
extern uint32 Image$$CHARGE_WIN_DATA$$ZI$$Length;

//FS MEM GET
extern uint32  Load$$FS_MEM_GET_CODE$$Base;
extern uint32 Image$$FS_MEM_GET_CODE$$Base;
extern uint32 Image$$FS_MEM_GET_CODE$$Length;
extern uint32  Load$$FS_MEM_GET_DATA$$Base;
extern uint32 Image$$FS_MEM_GET_DATA$$RW$$Base;
extern uint32 Image$$FS_MEM_GET_DATA$$RW$$Length;
extern uint32 Image$$FS_MEM_GET_DATA$$ZI$$Base;
extern uint32 Image$$FS_MEM_GET_DATA$$ZI$$Length;

//Fwupgrade
extern uint32  Load$$FW_UPGRADE_CODE$$Base;
extern uint32 Image$$FW_UPGRADE_CODE$$Base;
extern uint32 Image$$FW_UPGRADE_CODE$$Length;
extern uint32  Load$$FW_UPGRADE_DATA$$Base;
extern uint32 Image$$FW_UPGRADE_DATA$$RW$$Base;
extern uint32 Image$$FW_UPGRADE_DATA$$RW$$Length;
extern uint32 Image$$FW_UPGRADE_DATA$$ZI$$Base;
extern uint32 Image$$FW_UPGRADE_DATA$$ZI$$Length;


//BEEP
extern uint32  Load$$SEID0000_8K_CODE$$Base;
extern uint32  Image$$SEID0000_8K_CODE$$Length;
extern uint32  Load$$SEID0001_8K_CODE$$Base;
extern uint32  Image$$SEID0001_8K_CODE$$Length;
extern uint32  Load$$SEID0002_8K_CODE$$Base;
extern uint32  Image$$SEID0002_8K_CODE$$Length;
extern uint32  Load$$SEID0003_8K_CODE$$Base;
extern uint32  Image$$SEID0003_8K_CODE$$Length;
extern uint32  Load$$SEID0004_8K_CODE$$Base;
extern uint32  Image$$SEID0004_8K_CODE$$Length;
extern uint32  Load$$SEID0005_8K_CODE$$Base;
extern uint32  Image$$SEID0005_8K_CODE$$Length;
extern uint32  Load$$SEID0006_8K_CODE$$Base;
extern uint32  Image$$SEID0006_8K_CODE$$Length;
extern uint32  Load$$SEID0007_8K_CODE$$Base;
extern uint32  Image$$SEID0007_8K_CODE$$Length;

extern uint32  Load$$SEID0000_11K_CODE$$Base;
extern uint32  Image$$SEID0000_11K_CODE$$Length;
extern uint32  Load$$SEID0001_11K_CODE$$Base;
extern uint32  Image$$SEID0001_11K_CODE$$Length;
extern uint32  Load$$SEID0002_11K_CODE$$Base;
extern uint32  Image$$SEID0002_11K_CODE$$Length;
extern uint32  Load$$SEID0003_11K_CODE$$Base;
extern uint32  Image$$SEID0003_11K_CODE$$Length;
extern uint32  Load$$SEID0004_11K_CODE$$Base;
extern uint32  Image$$SEID0004_11K_CODE$$Length;
extern uint32  Load$$SEID0005_11K_CODE$$Base;
extern uint32  Image$$SEID0005_11K_CODE$$Length;
extern uint32  Load$$SEID0006_11K_CODE$$Base;
extern uint32  Image$$SEID0006_11K_CODE$$Length;
extern uint32  Load$$SEID0007_11K_CODE$$Base;
extern uint32  Image$$SEID0007_11K_CODE$$Length;

extern uint32  Load$$SEID0000_12K_CODE$$Base;
extern uint32  Image$$SEID0000_12K_CODE$$Length;
extern uint32  Load$$SEID0001_12K_CODE$$Base;
extern uint32  Image$$SEID0001_12K_CODE$$Length;
extern uint32  Load$$SEID0002_12K_CODE$$Base;
extern uint32  Image$$SEID0002_12K_CODE$$Length;
extern uint32  Load$$SEID0003_12K_CODE$$Base;
extern uint32  Image$$SEID0003_12K_CODE$$Length;
extern uint32  Load$$SEID0004_12K_CODE$$Base;
extern uint32  Image$$SEID0004_12K_CODE$$Length;
extern uint32  Load$$SEID0005_12K_CODE$$Base;
extern uint32  Image$$SEID0005_12K_CODE$$Length;
extern uint32  Load$$SEID0006_12K_CODE$$Base;
extern uint32  Image$$SEID0006_12K_CODE$$Length;
extern uint32  Load$$SEID0007_12K_CODE$$Base;
extern uint32  Image$$SEID0007_12K_CODE$$Length;


extern uint32  Load$$SEID0000_16K_CODE$$Base;
extern uint32  Image$$SEID0000_16K_CODE$$Length;
extern uint32  Load$$SEID0001_16K_CODE$$Base;
extern uint32  Image$$SEID0001_16K_CODE$$Length;
extern uint32  Load$$SEID0002_16K_CODE$$Base;
extern uint32  Image$$SEID0002_16K_CODE$$Length;
extern uint32  Load$$SEID0003_16K_CODE$$Base;
extern uint32  Image$$SEID0003_16K_CODE$$Length;
extern uint32  Load$$SEID0004_16K_CODE$$Base;
extern uint32  Image$$SEID0004_16K_CODE$$Length;
extern uint32  Load$$SEID0005_16K_CODE$$Base;
extern uint32  Image$$SEID0005_16K_CODE$$Length;
extern uint32  Load$$SEID0006_16K_CODE$$Base;
extern uint32  Image$$SEID0006_16K_CODE$$Length;
extern uint32  Load$$SEID0007_16K_CODE$$Base;
extern uint32  Image$$SEID0007_16K_CODE$$Length;

extern uint32  Load$$SEID0000_22K_CODE$$Base;
extern uint32  Image$$SEID0000_22K_CODE$$Length;
extern uint32  Load$$SEID0001_22K_CODE$$Base;
extern uint32  Image$$SEID0001_22K_CODE$$Length;
extern uint32  Load$$SEID0002_22K_CODE$$Base;
extern uint32  Image$$SEID0002_22K_CODE$$Length;
extern uint32  Load$$SEID0003_22K_CODE$$Base;
extern uint32  Image$$SEID0003_22K_CODE$$Length;
extern uint32  Load$$SEID0004_22K_CODE$$Base;
extern uint32  Image$$SEID0004_22K_CODE$$Length;
extern uint32  Load$$SEID0005_22K_CODE$$Base;
extern uint32  Image$$SEID0005_22K_CODE$$Length;
extern uint32  Load$$SEID0006_22K_CODE$$Base;
extern uint32  Image$$SEID0006_22K_CODE$$Length;
extern uint32  Load$$SEID0007_22K_CODE$$Base;
extern uint32  Image$$SEID0007_22K_CODE$$Length;


extern uint32  Load$$SEID0000_24K_CODE$$Base;
extern uint32  Image$$SEID0000_24K_CODE$$Length;
extern uint32  Load$$SEID0001_24K_CODE$$Base;
extern uint32  Image$$SEID0001_24K_CODE$$Length;
extern uint32  Load$$SEID0002_24K_CODE$$Base;
extern uint32  Image$$SEID0002_24K_CODE$$Length;
extern uint32  Load$$SEID0003_24K_CODE$$Base;
extern uint32  Image$$SEID0003_24K_CODE$$Length;
extern uint32  Load$$SEID0004_24K_CODE$$Base;
extern uint32  Image$$SEID0004_24K_CODE$$Length;
extern uint32  Load$$SEID0005_24K_CODE$$Base;
extern uint32  Image$$SEID0005_24K_CODE$$Length;
extern uint32  Load$$SEID0006_24K_CODE$$Base;
extern uint32  Image$$SEID0006_24K_CODE$$Length;
extern uint32  Load$$SEID0007_24K_CODE$$Base;
extern uint32  Image$$SEID0007_24K_CODE$$Length;


extern uint32  Load$$SEID0000_32K_CODE$$Base;
extern uint32  Image$$SEID0000_32K_CODE$$Length;
extern uint32  Load$$SEID0001_32K_CODE$$Base;
extern uint32  Image$$SEID0001_32K_CODE$$Length;
extern uint32  Load$$SEID0002_32K_CODE$$Base;
extern uint32  Image$$SEID0002_32K_CODE$$Length;
extern uint32  Load$$SEID0003_32K_CODE$$Base;
extern uint32  Image$$SEID0003_32K_CODE$$Length;
extern uint32  Load$$SEID0004_32K_CODE$$Base;
extern uint32  Image$$SEID0004_32K_CODE$$Length;
extern uint32  Load$$SEID0005_32K_CODE$$Base;
extern uint32  Image$$SEID0005_32K_CODE$$Length;
extern uint32  Load$$SEID0006_32K_CODE$$Base;
extern uint32  Image$$SEID0006_32K_CODE$$Length;
extern uint32  Load$$SEID0007_32K_CODE$$Base;
extern uint32  Image$$SEID0007_32K_CODE$$Length;

extern uint32  Load$$SEID0000_44K_CODE$$Base;
extern uint32  Image$$SEID0000_44K_CODE$$Length;
extern uint32  Load$$SEID0001_44K_CODE$$Base;
extern uint32  Image$$SEID0001_44K_CODE$$Length;
extern uint32  Load$$SEID0002_44K_CODE$$Base;
extern uint32  Image$$SEID0002_44K_CODE$$Length;
extern uint32  Load$$SEID0003_44K_CODE$$Base;
extern uint32  Image$$SEID0003_44K_CODE$$Length;
extern uint32  Load$$SEID0004_44K_CODE$$Base;
extern uint32  Image$$SEID0004_44K_CODE$$Length;
extern uint32  Load$$SEID0005_44K_CODE$$Base;
extern uint32  Image$$SEID0005_44K_CODE$$Length;
extern uint32  Load$$SEID0006_44K_CODE$$Base;
extern uint32  Image$$SEID0006_44K_CODE$$Length;
extern uint32  Load$$SEID0007_44K_CODE$$Base;
extern uint32  Image$$SEID0007_44K_CODE$$Length;


extern uint32  Load$$SEID0000_48K_CODE$$Base;
extern uint32  Image$$SEID0000_48K_CODE$$Length;
extern uint32  Load$$SEID0001_48K_CODE$$Base;
extern uint32  Image$$SEID0001_48K_CODE$$Length;
extern uint32  Load$$SEID0002_48K_CODE$$Base;
extern uint32  Image$$SEID0002_48K_CODE$$Length;
extern uint32  Load$$SEID0003_48K_CODE$$Base;
extern uint32  Image$$SEID0003_48K_CODE$$Length;
extern uint32  Load$$SEID0004_48K_CODE$$Base;
extern uint32  Image$$SEID0004_48K_CODE$$Length;
extern uint32  Load$$SEID0005_48K_CODE$$Base;
extern uint32  Image$$SEID0005_48K_CODE$$Length;
extern uint32  Load$$SEID0006_48K_CODE$$Base;
extern uint32  Image$$SEID0006_48K_CODE$$Length;
extern uint32  Load$$SEID0007_48K_CODE$$Base;
extern uint32  Image$$SEID0007_48K_CODE$$Length;

extern uint32  Load$$CP1251_UNICODE_TABLE_CODE$$Base;
extern uint32  Image$$CP1251_UNICODE_TABLE_CODE$$Length;
extern uint32  Load$$CP932_UNICODE_TABLE_CODE$$Base;
extern uint32  Image$$CP932_UNICODE_TABLE_CODE$$Length;
extern uint32  Load$$CP932_UNICODE_TABLE1_CODE$$Base;
extern uint32  Image$$CP932_UNICODE_TABLE1_CODE$$Length;
extern uint32  Load$$CP932_UNICODE_TABLE2_CODE$$Base;
extern uint32  Image$$CP932_UNICODE_TABLE2_CODE$$Length;
extern uint32  Load$$CP950_UNICODE_TABLE_CODE$$Base;
extern uint32  Image$$CP950_UNICODE_TABLE_CODE$$Length;
extern uint32  Load$$CP949_UNICODE_TABLE_CODE$$Base;
extern uint32  Image$$CP949_UNICODE_TABLE_CODE$$Length;
extern uint32  Load$$CP949_UNICODE_TABLE1_CODE$$Base;
extern uint32  Image$$CP949_UNICODE_TABLE1_CODE$$Length;


/****************BB ��*************************/
extern uint32  Load$$BB_SYS_CODE$$Base;
extern uint32 Image$$BB_SYS_CODE$$Base;
extern uint32 Image$$BB_SYS_CODE$$Length;
extern uint32  Load$$BB_SYS_DATA$$Base;
extern uint32 Image$$BB_SYS_DATA$$RW$$Base;
extern uint32 Image$$BB_SYS_DATA$$RW$$Length;
extern uint32 Image$$BB_SYS_DATA$$ZI$$Base;
extern uint32 Image$$BB_SYS_DATA$$ZI$$Length;

extern uint32  Load$$MP3_ENCODE_BIN_CODE$$Base;
extern uint32 Image$$MP3_ENCODE_BIN_CODE$$Base;
extern uint32 Image$$MP3_ENCODE_BIN_CODE$$Length;
extern uint32  Load$$MP3_ENCODE_BIN_DATA$$Base;
extern uint32 Image$$MP3_ENCODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$MP3_ENCODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$MP3_ENCODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$MP3_ENCODE_BIN_DATA$$ZI$$Length;
//MP3
extern uint32  Load$$MP3_DECODE_BIN_CODE$$Base;
extern uint32 Image$$MP3_DECODE_BIN_CODE$$Base;
extern uint32 Image$$MP3_DECODE_BIN_CODE$$Length;
extern uint32  Load$$MP3_DECODE_BIN_DATA$$Base;
extern uint32 Image$$MP3_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$MP3_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$MP3_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$MP3_DECODE_BIN_DATA$$ZI$$Length;

//WMA
extern uint32  Load$$WMA_DECODE_BIN_CODE$$Base;
extern uint32 Image$$WMA_DECODE_BIN_CODE$$Base;
extern uint32 Image$$WMA_DECODE_BIN_CODE$$Length;
extern uint32  Load$$WMA_DECODE_BIN_DATA$$Base;
extern uint32 Image$$WMA_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$WMA_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$WMA_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$WMA_DECODE_BIN_DATA$$ZI$$Length;


//WAV
extern uint32  Load$$WAV_DECODE_BIN_CODE$$Base;
extern uint32 Image$$WAV_DECODE_BIN_CODE$$Base;
extern uint32 Image$$WAV_DECODE_BIN_CODE$$Length;
extern uint32  Load$$WAV_DECODE_BIN_DATA$$Base;
extern uint32 Image$$WAV_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$WAV_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$WAV_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$WAV_DECODE_BIN_DATA$$ZI$$Length;


//FLAC
extern uint32  Load$$FLAC_DECODE_BIN_CODE$$Base;
extern uint32 Image$$FLAC_DECODE_BIN_CODE$$Base;
extern uint32 Image$$FLAC_DECODE_BIN_CODE$$Length;
extern uint32  Load$$FLAC_DECODE_BIN_DATA$$Base;
extern uint32 Image$$FLAC_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$FLAC_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$FLAC_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$FLAC_DECODE_BIN_DATA$$ZI$$Length;

//AAC
extern uint32  Load$$AAC_DECODE_BIN_CODE$$Base;
extern uint32 Image$$AAC_DECODE_BIN_CODE$$Base;
extern uint32 Image$$AAC_DECODE_BIN_CODE$$Length;
extern uint32  Load$$AAC_DECODE_BIN_DATA$$Base;
extern uint32 Image$$AAC_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$AAC_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$AAC_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$AAC_DECODE_BIN_DATA$$ZI$$Length;

//APE
extern uint32  Load$$APE_DECODE_BIN_CODE$$Base;
extern uint32 Image$$APE_DECODE_BIN_CODE$$Base;
extern uint32 Image$$APE_DECODE_BIN_CODE$$Length;
extern uint32  Load$$APE_DECODE_BIN_DATA$$Base;
extern uint32 Image$$APE_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$APE_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$APE_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$APE_DECODE_BIN_DATA$$ZI$$Length;


//OGG
extern uint32  Load$$OGG_DECODE_BIN_CODE$$Base;
extern uint32 Image$$OGG_DECODE_BIN_CODE$$Base;
extern uint32 Image$$OGG_DECODE_BIN_CODE$$Length;
extern uint32  Load$$OGG_DECODE_BIN_DATA$$Base;
extern uint32 Image$$OGG_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$OGG_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$OGG_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$OGG_DECODE_BIN_DATA$$ZI$$Length;

//HFLAC
extern uint32  Load$$FLAC_HDECODE_BIN_CODE$$Base;
extern uint32 Image$$FLAC_HDECODE_BIN_CODE$$Base;
extern uint32 Image$$FLAC_HDECODE_BIN_CODE$$Length;
extern uint32  Load$$FLAC_HDECODE_BIN_DATA$$Base;
extern uint32 Image$$FLAC_HDECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$FLAC_HDECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$FLAC_HDECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$FLAC_HDECODE_BIN_DATA$$ZI$$Length;

//HALAC
extern uint32  Load$$ALAC_HDECODE_BIN_CODE$$Base;
extern uint32 Image$$ALAC_HDECODE_BIN_CODE$$Base;
extern uint32 Image$$ALAC_HDECODE_BIN_CODE$$Length;
extern uint32  Load$$ALAC_HDECODE_BIN_DATA$$Base;
extern uint32 Image$$ALAC_HDECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$ALAC_HDECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$ALAC_HDECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$ALAC_HDECODE_BIN_DATA$$ZI$$Length;

//HAPE
extern uint32  Load$$APE_HDECODE_BIN_CODE$$Base;
extern uint32 Image$$APE_HDECODE_BIN_CODE$$Base;
extern uint32 Image$$APE_HDECODE_BIN_CODE$$Length;
extern uint32  Load$$APE_HDECODE_BIN_DATA$$Base;
extern uint32 Image$$APE_HDECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$APE_HDECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$APE_HDECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$APE_HDECODE_BIN_DATA$$ZI$$Length;

//DSD
extern uint32  Load$$DSDIFF_DECODE_BIN_CODE$$Base;
extern uint32 Image$$DSDIFF_DECODE_BIN_CODE$$Base;
extern uint32 Image$$DSDIFF_DECODE_BIN_CODE$$Length;
extern uint32  Load$$DSDIFF_DECODE_BIN_DATA$$Base;
extern uint32 Image$$DSDIFF_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$DSDIFF_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$DSDIFF_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$DSDIFF_DECODE_BIN_DATA$$ZI$$Length;

//DSF
extern uint32  Load$$DSF_DECODE_BIN_CODE$$Base;
extern uint32 Image$$DSF_DECODE_BIN_CODE$$Base;
extern uint32 Image$$DSF_DECODE_BIN_CODE$$Length;
extern uint32  Load$$DSF_DECODE_BIN_DATA$$Base;
extern uint32 Image$$DSF_DECODE_BIN_DATA$$RW$$Base;
extern uint32 Image$$DSF_DECODE_BIN_DATA$$RW$$Length;
extern uint32 Image$$DSF_DECODE_BIN_DATA$$ZI$$Base;
extern uint32 Image$$DSF_DECODE_BIN_DATA$$ZI$$Length;

//BT
extern uint32  Load$$BTCONTROL_CODE$$Base;
extern uint32 Image$$BTCONTROL_CODE$$Base;
extern uint32 Image$$BTCONTROL_CODE$$Length;
extern uint32  Load$$BTCONTROL_DATA$$Base;
extern uint32 Image$$BTCONTROL_DATA$$RW$$Base;
extern uint32 Image$$BTCONTROL_DATA$$RW$$Length;
extern uint32 Image$$BTCONTROL_DATA$$ZI$$Base;
extern uint32 Image$$BTCONTROL_DATA$$ZI$$Length;

//BT WIN
extern uint32  Load$$BTWIN_CODE$$Base;
extern uint32 Image$$BTWIN_CODE$$Base;
extern uint32 Image$$BTWIN_CODE$$Length;
extern uint32  Load$$BTWIN_DATA$$Base;
extern uint32 Image$$BTWIN_DATA$$RW$$Base;
extern uint32 Image$$BTWIN_DATA$$RW$$Length;
extern uint32 Image$$BTWIN_DATA$$ZI$$Base;
extern uint32 Image$$BTWIN_DATA$$ZI$$Length;

#ifdef _A2DP_SOUCRE_
//LWBT
extern uint32  Load$$LWBT_UARTIF_CODE$$Base;
extern uint32 Image$$LWBT_UARTIF_CODE$$Base;
extern uint32 Image$$LWBT_UARTIF_CODE$$Length;
extern uint32  Load$$LWBT_UARTIF_DATA$$Base;
extern uint32 Image$$LWBT_UARTIF_DATA$$RW$$Base;
extern uint32 Image$$LWBT_UARTIF_DATA$$RW$$Length;
extern uint32 Image$$LWBT_UARTIF_DATA$$ZI$$Base;
extern uint32 Image$$LWBT_UARTIF_DATA$$ZI$$Length;

//LWBT
extern uint32  Load$$LWBT_INIT_CODE$$Base;
extern uint32 Image$$LWBT_INIT_CODE$$Base;
extern uint32 Image$$LWBT_INIT_CODE$$Length;
extern uint32  Load$$LWBT_INIT_DATA$$Base;
extern uint32 Image$$LWBT_INIT_DATA$$RW$$Base;
extern uint32 Image$$LWBT_INIT_DATA$$RW$$Length;
extern uint32 Image$$LWBT_INIT_DATA$$ZI$$Base;
extern uint32 Image$$LWBT_INIT_DATA$$ZI$$Length;
#endif

//LWBT
extern uint32  Load$$LWBT_CODE$$Base;
extern uint32 Image$$LWBT_CODE$$Base;
extern uint32 Image$$LWBT_CODE$$Length;
extern uint32  Load$$LWBT_DATA$$Base;
extern uint32 Image$$LWBT_DATA$$RW$$Base;
extern uint32 Image$$LWBT_DATA$$RW$$Length;
extern uint32 Image$$LWBT_DATA$$ZI$$Base;
extern uint32 Image$$LWBT_DATA$$ZI$$Length;

//SBC
extern uint32  Load$$SBC_ENCODE_CODE$$Base;
extern uint32 Image$$SBC_ENCODE_CODE$$Base;
extern uint32 Image$$SBC_ENCODE_CODE$$Length;
extern uint32  Load$$SBC_ENCODE_DATA$$Base;
extern uint32 Image$$SBC_ENCODE_DATA$$RW$$Base;
extern uint32 Image$$SBC_ENCODE_DATA$$RW$$Length;
extern uint32 Image$$SBC_ENCODE_DATA$$ZI$$Base;
extern uint32 Image$$SBC_ENCODE_DATA$$ZI$$Length;

//ssrc
extern uint32  Load$$SRC_CODE$$Base;
extern uint32 Image$$SRC_CODE$$Base;
extern uint32 Image$$SRC_CODE$$Length;
extern uint32  Load$$SRC_DATA$$Base;
extern uint32 Image$$SRC_DATA$$RW$$Base;
extern uint32 Image$$SRC_DATA$$RW$$Length;
extern uint32 Image$$SRC_DATA$$ZI$$Base;
extern uint32 Image$$SRC_DATA$$ZI$$Length;

//ssrc table
extern uint32  Load$$SRC_TABLE_CODE_192_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_192_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_192_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_192_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_192_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_192_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_192_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_192_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_176_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_176_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_176_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_176_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_176_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_176_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_176_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_176_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_96_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_96_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_96_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_96_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_96_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_96_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_96_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_96_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_88_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_88_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_88_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_88_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_88_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_88_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_88_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_88_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_64_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_64_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_64_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_64_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_64_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_64_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_64_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_64_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_48_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_48_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_48_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_48_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_48_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_48_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_48_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_48_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_32_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_32_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_32_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_32_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_32_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_32_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_32_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_32_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_24_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_24_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_24_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_24_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_24_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_24_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_24_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_24_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_22_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_22_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_22_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_22_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_22_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_22_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_22_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_22_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_16_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_16_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_16_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_16_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_16_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_16_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_16_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_16_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_12_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_12_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_12_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_12_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_12_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_12_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_12_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_12_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_11_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_11_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_11_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_11_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_11_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_11_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_11_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_11_44$$ZI$$Length;

extern uint32  Load$$SRC_TABLE_CODE_8_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_8_44$$Base;
extern uint32 Image$$SRC_TABLE_CODE_8_44$$Length;
extern uint32  Load$$SRC_TABLE_DATA_8_44$$Base;
extern uint32 Image$$SRC_TABLE_DATA_8_44$$RW$$Base;
extern uint32 Image$$SRC_TABLE_DATA_8_44$$RW$$Length;
extern uint32 Image$$SRC_TABLE_DATA_8_44$$ZI$$Base;
extern uint32 Image$$SRC_TABLE_DATA_8_44$$ZI$$Length;

//cc2564 init script
extern uint32  Load$$BT_INIT_SCRIPT_CODE$$Base;
extern uint32 Image$$BT_INIT_SCRIPT_CODE$$Base;
extern uint32 Image$$BT_INIT_SCRIPT_CODE$$Length;

//test
extern uint32  Load$$MAILBOX_TEST_CODE$$Base;
extern uint32 Image$$MAILBOX_TEST_CODE$$Base;
extern uint32 Image$$MAILBOX_TEST_CODE$$Length;
extern uint32  Load$$MAILBOX_TEST_DATA$$Base;
extern uint32 Image$$MAILBOX_TEST_DATA$$RW$$Base;
extern uint32 Image$$MAILBOX_TEST_DATA$$RW$$Length;
extern uint32 Image$$MAILBOX_TEST_DATA$$ZI$$Base;
extern uint32 Image$$MAILBOX_TEST_DATA$$ZI$$Length;

extern uint32  Load$$MEMORY_TEST_CODE$$Base;
extern uint32 Image$$MEMORY_TEST_CODE$$Base;
extern uint32 Image$$MEMORY_TEST_CODE$$Length;
extern uint32  Load$$MEMORY_TEST_DATA$$Base;
extern uint32 Image$$MEMORY_TEST_DATA$$RW$$Base;
extern uint32 Image$$MEMORY_TEST_DATA$$RW$$Length;
extern uint32 Image$$MEMORY_TEST_DATA$$ZI$$Base;
extern uint32 Image$$MEMORY_TEST_DATA$$ZI$$Length;

extern uint32  Load$$TIMER_TEST_CODE$$Base;
extern uint32 Image$$TIMER_TEST_CODE$$Base;
extern uint32 Image$$TIMER_TEST_CODE$$Length;
extern uint32  Load$$TIMER_TEST_DATA$$Base;
extern uint32 Image$$TIMER_TEST_DATA$$RW$$Base;
extern uint32 Image$$TIMER_TEST_DATA$$RW$$Length;
extern uint32 Image$$TIMER_TEST_DATA$$ZI$$Base;
extern uint32 Image$$TIMER_TEST_DATA$$ZI$$Length;

extern uint32  Load$$WDT_TEST_CODE$$Base;
extern uint32 Image$$WDT_TEST_CODE$$Base;
extern uint32 Image$$WDT_TEST_CODE$$Length;
extern uint32  Load$$WDT_TEST_DATA$$Base;
extern uint32 Image$$WDT_TEST_DATA$$RW$$Base;
extern uint32 Image$$WDT_TEST_DATA$$RW$$Length;
extern uint32 Image$$WDT_TEST_DATA$$ZI$$Base;
extern uint32 Image$$WDT_TEST_DATA$$ZI$$Length;

extern uint32  Load$$EMMC_TEST_CODE$$Base;
extern uint32 Image$$EMMC_TEST_CODE$$Base;
extern uint32 Image$$EMMC_TEST_CODE$$Length;
extern uint32  Load$$EMMC_TEST_DATA$$Base;
extern uint32 Image$$EMMC_TEST_DATA$$RW$$Base;
extern uint32 Image$$EMMC_TEST_DATA$$RW$$Length;
extern uint32 Image$$EMMC_TEST_DATA$$ZI$$Base;
extern uint32 Image$$EMMC_TEST_DATA$$ZI$$Length;

extern uint32  Load$$SDMMC_TEST_CODE$$Base;
extern uint32 Image$$SDMMC_TEST_CODE$$Base;
extern uint32 Image$$SDMMC_TEST_CODE$$Length;
extern uint32  Load$$SDMMC_TEST_DATA$$Base;
extern uint32 Image$$SDMMC_TEST_DATA$$RW$$Base;
extern uint32 Image$$SDMMC_TEST_DATA$$RW$$Length;
extern uint32 Image$$SDMMC_TEST_DATA$$ZI$$Base;
extern uint32 Image$$SDMMC_TEST_DATA$$ZI$$Length;

extern uint32  Load$$SPI_TEST_CODE$$Base;
extern uint32 Image$$SPI_TEST_CODE$$Base;
extern uint32 Image$$SPI_TEST_CODE$$Length;
extern uint32  Load$$SPI_TEST_DATA$$Base;
extern uint32 Image$$SPI_TEST_DATA$$RW$$Base;
extern uint32 Image$$SPI_TEST_DATA$$RW$$Length;
extern uint32 Image$$SPI_TEST_DATA$$ZI$$Base;
extern uint32 Image$$SPI_TEST_DATA$$ZI$$Length;

extern uint32  Load$$UART_TEST_CODE$$Base;
extern uint32 Image$$UART_TEST_CODE$$Base;
extern uint32 Image$$UART_TEST_CODE$$Length;
extern uint32  Load$$UART_TEST_DATA$$Base;
extern uint32 Image$$UART_TEST_DATA$$RW$$Base;
extern uint32 Image$$UART_TEST_DATA$$RW$$Length;
extern uint32 Image$$UART_TEST_DATA$$ZI$$Base;
extern uint32 Image$$UART_TEST_DATA$$ZI$$Length;

extern uint32  Load$$PWM_TEST_CODE$$Base;
extern uint32 Image$$PWM_TEST_CODE$$Base;
extern uint32 Image$$PWM_TEST_CODE$$Length;
extern uint32  Load$$PWM_TEST_DATA$$Base;
extern uint32 Image$$PWM_TEST_DATA$$RW$$Base;
extern uint32 Image$$PWM_TEST_DATA$$RW$$Length;
extern uint32 Image$$PWM_TEST_DATA$$ZI$$Base;
extern uint32 Image$$PWM_TEST_DATA$$ZI$$Length;

extern uint32  Load$$GPIO_TEST_CODE$$Base;
extern uint32 Image$$GPIO_TEST_CODE$$Base;
extern uint32 Image$$GPIO_TEST_CODE$$Length;
extern uint32  Load$$GPIO_TEST_DATA$$Base;
extern uint32 Image$$GPIO_TEST_DATA$$RW$$Base;
extern uint32 Image$$GPIO_TEST_DATA$$RW$$Length;
extern uint32 Image$$GPIO_TEST_DATA$$ZI$$Base;
extern uint32 Image$$GPIO_TEST_DATA$$ZI$$Length;

extern uint32  Load$$I2C_TEST_CODE$$Base;
extern uint32 Image$$I2C_TEST_CODE$$Base;
extern uint32 Image$$I2C_TEST_CODE$$Length;
extern uint32  Load$$I2C_TEST_DATA$$Base;
extern uint32 Image$$I2C_TEST_DATA$$RW$$Base;
extern uint32 Image$$I2C_TEST_DATA$$RW$$Length;
extern uint32 Image$$I2C_TEST_DATA$$ZI$$Base;
extern uint32 Image$$I2C_TEST_DATA$$ZI$$Length;

extern uint32  Load$$ADC_TEST_CODE$$Base;
extern uint32 Image$$ADC_TEST_CODE$$Base;
extern uint32 Image$$ADC_TEST_CODE$$Length;
extern uint32  Load$$ADC_TEST_DATA$$Base;
extern uint32 Image$$ADC_TEST_DATA$$RW$$Base;
extern uint32 Image$$ADC_TEST_DATA$$RW$$Length;
extern uint32 Image$$ADC_TEST_DATA$$ZI$$Base;
extern uint32 Image$$ADC_TEST_DATA$$ZI$$Length;

extern uint32  Load$$PVTM_TEST_CODE$$Base;
extern uint32 Image$$PVTM_TEST_CODE$$Base;
extern uint32 Image$$PVTM_TEST_CODE$$Length;
extern uint32  Load$$PVTM_TEST_DATA$$Base;
extern uint32 Image$$PVTM_TEST_DATA$$RW$$Base;
extern uint32 Image$$PVTM_TEST_DATA$$RW$$Length;
extern uint32 Image$$PVTM_TEST_DATA$$ZI$$Base;
extern uint32 Image$$PVTM_TEST_DATA$$ZI$$Length;

extern uint32  Load$$I2S_TEST_CODE$$Base;
extern uint32 Image$$I2S_TEST_CODE$$Base;
extern uint32 Image$$I2S_TEST_CODE$$Length;
extern uint32  Load$$I2S_TEST_DATA$$Base;
extern uint32 Image$$I2S_TEST_DATA$$RW$$Base;
extern uint32 Image$$I2S_TEST_DATA$$RW$$Length;
extern uint32 Image$$I2S_TEST_DATA$$ZI$$Base;
extern uint32 Image$$I2S_TEST_DATA$$ZI$$Length;

extern uint32  Load$$ACODEC_TEST_CODE$$Base;
extern uint32 Image$$ACODEC_TEST_CODE$$Base;
extern uint32 Image$$ACODEC_TEST_CODE$$Length;
extern uint32  Load$$ACODEC_TEST_DATA$$Base;
extern uint32 Image$$ACODEC_TEST_DATA$$RW$$Base;
extern uint32 Image$$ACODEC_TEST_DATA$$RW$$Length;
extern uint32 Image$$ACODEC_TEST_DATA$$ZI$$Base;
extern uint32 Image$$ACODEC_TEST_DATA$$ZI$$Length;

extern uint32  Load$$SYSTICK_TEST_CODE$$Base;
extern uint32 Image$$SYSTICK_TEST_CODE$$Base;
extern uint32 Image$$SYSTICK_TEST_CODE$$Length;
extern uint32  Load$$SYSTICK_TEST_DATA$$Base;
extern uint32 Image$$SYSTICK_TEST_DATA$$RW$$Base;
extern uint32 Image$$SYSTICK_TEST_DATA$$RW$$Length;
extern uint32 Image$$SYSTICK_TEST_DATA$$ZI$$Base;
extern uint32 Image$$SYSTICK_TEST_DATA$$ZI$$Length;

#endif

extern void UC1604C_WriteRAM_Prepare(void);
extern void UC1604C_Init(UINT8 XMirror,UINT8 YMirror,UINT8 Rotary);
extern void UC1604C_SendData(UINT16 data);
extern void UC1604C_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1);
extern void UC1604C_Clear(UINT16 color);
extern void UC1604C_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *pSrc);
extern void UC1604C_Standby(void);
extern void UC1604C_WakeUp(void);
extern void UC1604C_SetPixel(UINT16 x, UINT16 y, UINT16 data);
extern void UC1604C_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1);
extern void UC1604C_ClrSrc(void);
extern void UC1604C_ClrRect(int x0, int y0, int x1, int y1);
extern void UC1604C_DEV_FillRect(int x0, int y0, int x1, int y1);


extern void SH1108G_SetPixel(UINT16 x, UINT16 y, UINT16 data);
extern void SH1108G_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1);
extern void SH1108G_DEV_FillRect(int x0, int y0, int x1, int y1);
extern void SH1108G_WriteRAM_Prepare(void);
extern void SH1108G_Init(void);
extern void SH1108G_SendData(UINT16 data);
extern void SH1108G_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1);
extern void SH1108G_Clear(UINT16 color);
extern int32 SH1108G_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT8 *pSrc);
extern void SH1108G_Standby(void);
extern void SH1108G_WakeUp(void);
extern void SH1108G_MP4_Init(void);
extern void SH1108G_MP4_DeInit(void);
extern void SH1108G_Clear_Screen(void);
extern void  SH1108G_ClrSrc(void);
extern void SH1108G_ClrRect(uint16 x0,uint16 y0,uint16 x1,int16 y1);

extern void ST7735S_WriteRAM_Prepare(void);
extern void ST7735S_Init(void);
extern void ST7735S_SendData(UINT16 data);
extern void ST7735S_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1);
extern void ST7735S_Clear(UINT16 color);
extern int32 ST7735S_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *pSrc);
extern void ST7735S_Standby(void);
extern void ST7735S_WakeUp(void);
extern void ST7735S_MP4_Init(void);
extern void ST7735S_MP4_DeInit(void);
extern void ST7735S_Clear_Screen(void);
extern void  ST7735S_ClrSrc(void);
extern void ST7735S_ClrRect(uint16 x0,uint16 y0,uint16 x1,int16 y1);

extern void ST7735_WriteRAM_Prepare(void);
extern void ST7735_Init(void);
extern void ST7735_SendData(UINT16 data);
extern void ST7735_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1);
extern void ST7735_Clear(UINT16 color);
extern int32 ST7735_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *pSrc);
extern void ST7735_Standby(void);
extern void ST7735_WakeUp(void);
extern void ST7735_MP4_Init(void);
extern void ST7735_MP4_DeInit(void);
extern void ST7735_SetPixel(UINT16 x, UINT16 y, UINT16 data);
extern void ST7735_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1);
extern void ST7735_Clear_Screen(void);
extern void ST7735_Clear_Rect(int x0, int y0, int x1, int y1);
extern void ST7735_Fill_Rect(int x0, int y0, int x1, int y1);
extern void  ST7735_ClrSrc(void);
extern void ST7735_ClrRect(uint16 x0,uint16 y0,uint16 x1,int16 y1);

extern void Qn8035_Tuner_SetInitArea(UINT8 area);
extern void Qn8035_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area);
extern void Qn8035_Tuner_SetStereo(BOOL bStereo);
extern void Qn8035_Tuner_Vol_Set(UINT8 gain);
extern void Qn8035_Tuner_PowerOffDeinit(void);
extern UINT16 Qn8035_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq);
extern void Qn8035_Tuner_PowerDown(void);
extern void Qn8035_Tuner_MuteControl(bool active);
extern BOOLEAN Qn8035_GetStereoStatus(void);

extern void FM5807_Tuner_SetInitArea(UINT8 area);
extern void FM5807_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area);
extern void FM5807_Tuner_SetStereo(BOOL bStereo);
extern void FM5807_Tuner_Vol_Set(UINT8 gain);
extern void FM5807_Tuner_PowerOffDeinit(void);
extern UINT16 FM5807_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq);
extern void FM5807_Tuner_PowerDown(void);
extern void FM5807_Tuner_MuteControl(bool active);
extern BOOLEAN FM5807_GetStereoStatus(void);


/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : ģ����Ϣ�����ɹ̼�ģ��ͷ��Ϣ�����ڴ������

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
__attribute__((section("ModuleInfo")))
FIRMWARE_INFO_T const ModuleInfo =
{

    (uint32)(&Load$$MODULE_INFO$$Base),

    //CODE_INFO_TABLE
    {
        (uint32)(MAX_MODULE_NUM),
#ifndef _JTAG_DEBUG_
        {
            //ϵͳ��
            {
                (uint32)(&Load$$SYS_CODE$$Base),
                (uint32)(&Image$$SYS_CODE$$Base),
                (uint32)(&Image$$SYS_CODE$$Length),

                (uint32)(&Load$$SYS_DATA$$Base),
                (uint32)(&Image$$SYS_DATA$$RW$$Base),
                (uint32)(&Image$$SYS_DATA$$RW$$Length),

                (uint32)(&Image$$SYS_DATA$$ZI$$Base),
                (uint32)(&Image$$SYS_DATA$$ZI$$Length),
            },

            //ϵͳ��ʼ����
            {
                (uint32)(&Load$$SYS_INIT_CODE$$Base),
                (uint32)(&Image$$SYS_INIT_CODE$$Base),
                (uint32)(&Image$$SYS_INIT_CODE$$Length),

                (uint32)(&Load$$SYS_INIT_DATA$$Base),
                (uint32)(&Image$$SYS_INIT_DATA$$RW$$Base),
                (uint32)(&Image$$SYS_INIT_DATA$$RW$$Length),

                (uint32)(&Image$$SYS_INIT_DATA$$ZI$$Base),
                (uint32)(&Image$$SYS_INIT_DATA$$ZI$$Length),
            },

            //Flash���
            {
                (uint32)(&Load$$FLASH_WRITE_CODE$$Base),
                (uint32)(&Image$$FLASH_WRITE_CODE$$Base),
                (uint32)(&Image$$FLASH_WRITE_CODE$$Length),

                (uint32)(&Load$$FLASH_WRITE_DATA$$Base),
                (uint32)(&Image$$FLASH_WRITE_DATA$$RW$$Base),
                (uint32)(&Image$$FLASH_WRITE_DATA$$RW$$Length),

                (uint32)(&Image$$FLASH_WRITE_DATA$$ZI$$Base),
                (uint32)(&Image$$FLASH_WRITE_DATA$$ZI$$Length),
            },


            //lcd������driver1
            {
                (uint32)(&Load$$LCD_DRIVER1_CODE$$Base),
                (uint32)(&Image$$LCD_DRIVER1_CODE$$Base),
                (uint32)(&Image$$LCD_DRIVER1_CODE$$Length),

                (uint32)(&Load$$LCD_DRIVER1_DATA$$Base),
                (uint32)(&Image$$LCD_DRIVER1_DATA$$RW$$Base),
                (uint32)(&Image$$LCD_DRIVER1_DATA$$RW$$Length),

                (uint32)(&Image$$LCD_DRIVER1_DATA$$ZI$$Base),
                (uint32)(&Image$$LCD_DRIVER1_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$LCD_DRIVER2_CODE$$Base),
                (uint32)(&Image$$LCD_DRIVER2_CODE$$Base),
                (uint32)(&Image$$LCD_DRIVER2_CODE$$Length),

                (uint32)(&Load$$LCD_DRIVER2_DATA$$Base),
                (uint32)(&Image$$LCD_DRIVER2_DATA$$RW$$Base),
                (uint32)(&Image$$LCD_DRIVER2_DATA$$RW$$Length),

                (uint32)(&Image$$LCD_DRIVER2_DATA$$ZI$$Base),
                (uint32)(&Image$$LCD_DRIVER2_DATA$$ZI$$Length),
            },


            //MDB WIN
            {
                (uint32)(&Load$$MDB_WIN_CODE$$Base),
                (uint32)(&Image$$MDB_WIN_CODE$$Base),
                (uint32)(&Image$$MDB_WIN_CODE$$Length),

                (uint32)(&Load$$MDB_WIN_DATA$$Base),
                (uint32)(&Image$$MDB_WIN_DATA$$RW$$Base),
                (uint32)(&Image$$MDB_WIN_DATA$$RW$$Length),

                (uint32)(&Image$$MDB_WIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MDB_WIN_DATA$$ZI$$Length),
            },

            //CHARGE WIN
            {
                (uint32)(&Load$$CHARGE_WIN_CODE$$Base),
                (uint32)(&Image$$CHARGE_WIN_CODE$$Base),
                (uint32)(&Image$$CHARGE_WIN_CODE$$Length),

                (uint32)(&Load$$CHARGE_WIN_DATA$$Base),
                (uint32)(&Image$$CHARGE_WIN_DATA$$RW$$Base),
                (uint32)(&Image$$CHARGE_WIN_DATA$$RW$$Length),

                (uint32)(&Image$$CHARGE_WIN_DATA$$ZI$$Base),
                (uint32)(&Image$$CHARGE_WIN_DATA$$ZI$$Length),
            },

            //FS MEM GET
            {
                (uint32)(&Load$$FS_MEM_GET_CODE$$Base),
                (uint32)(&Image$$FS_MEM_GET_CODE$$Base),
                (uint32)(&Image$$FS_MEM_GET_CODE$$Length),

                (uint32)(&Load$$FS_MEM_GET_DATA$$Base),
                (uint32)(&Image$$FS_MEM_GET_DATA$$RW$$Base),
                (uint32)(&Image$$FS_MEM_GET_DATA$$RW$$Length),

                (uint32)(&Image$$FS_MEM_GET_DATA$$ZI$$Base),
                (uint32)(&Image$$FS_MEM_GET_DATA$$ZI$$Length),
            },

            //FW Upgrade
            {
                (uint32)(&Load$$FW_UPGRADE_CODE$$Base),
                (uint32)(&Image$$FW_UPGRADE_CODE$$Base),
                (uint32)(&Image$$FW_UPGRADE_CODE$$Length),

                (uint32)(&Load$$FW_UPGRADE_DATA$$Base),
                (uint32)(&Image$$FW_UPGRADE_DATA$$RW$$Base),
                (uint32)(&Image$$FW_UPGRADE_DATA$$RW$$Length),

                (uint32)(&Image$$FW_UPGRADE_DATA$$ZI$$Base),
                (uint32)(&Image$$FW_UPGRADE_DATA$$ZI$$Length),
            },

            //USB window UI
            {
                (uint32)(&Load$$USBWIN_CODE$$Base),
                (uint32)(&Image$$USBWIN_CODE$$Base),
                (uint32)(&Image$$USBWIN_CODE$$Length),

                (uint32)(&Load$$USBWIN_DATA$$Base),
                (uint32)(&Image$$USBWIN_DATA$$RW$$Base),
                (uint32)(&Image$$USBWIN_DATA$$RW$$Length),

                (uint32)(&Image$$USBWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$USBWIN_DATA$$ZI$$Length),
            },

            //USB Control & driver
            {
                (uint32)(&Load$$USBCONTROL_CODE$$Base),
                (uint32)(&Image$$USBCONTROL_CODE$$Base),
                (uint32)(&Image$$USBCONTROL_CODE$$Length),

                (uint32)(&Load$$USBCONTROL_DATA$$Base),
                (uint32)(&Image$$USBCONTROL_DATA$$RW$$Base),
                (uint32)(&Image$$USBCONTROL_DATA$$RW$$Length),

                (uint32)(&Image$$USBCONTROL_DATA$$ZI$$Base),
                (uint32)(&Image$$USBCONTROL_DATA$$ZI$$Length),
            },

            //USB Audio
            {
                (uint32)(&Load$$USBAUDIO_CODE$$Base),
                (uint32)(&Image$$USBAUDIO_CODE$$Base),
                (uint32)(&Image$$USBAUDIO_CODE$$Length),

                (uint32)(&Load$$USBAUDIO_DATA$$Base),
                (uint32)(&Image$$USBAUDIO_DATA$$RW$$Base),
                (uint32)(&Image$$USBAUDIO_DATA$$RW$$Length),

                (uint32)(&Image$$USBAUDIO_DATA$$ZI$$Base),
                (uint32)(&Image$$USBAUDIO_DATA$$ZI$$Length),
            },

            //USB MSC
            {
                (uint32)(&Load$$USBMSC_CODE$$Base),
                (uint32)(&Image$$USBMSC_CODE$$Base),
                (uint32)(&Image$$USBMSC_CODE$$Length),

                (uint32)(&Load$$USBMSC_DATA$$Base),
                (uint32)(&Image$$USBMSC_DATA$$RW$$Base),
                (uint32)(&Image$$USBMSC_DATA$$RW$$Length),

                (uint32)(&Image$$USBMSC_DATA$$ZI$$Base),
                (uint32)(&Image$$USBMSC_DATA$$ZI$$Length),
            },

            //USB Serial
            {
                (uint32)(&Load$$USBSERIAL_CODE$$Base),
                (uint32)(&Image$$USBSERIAL_CODE$$Base),
                (uint32)(&Image$$USBSERIAL_CODE$$Length),

                (uint32)(&Load$$USBSERIAL_DATA$$Base),
                (uint32)(&Image$$USBSERIAL_DATA$$RW$$Base),
                (uint32)(&Image$$USBSERIAL_DATA$$RW$$Length),

                (uint32)(&Image$$USBSERIAL_DATA$$ZI$$Base),
                (uint32)(&Image$$USBSERIAL_DATA$$ZI$$Length),
            },

            //VIDEO
            {
                (uint32)(&Load$$VIDEOWIN_CODE$$Base),
                (uint32)(&Image$$VIDEOWIN_CODE$$Base),
                (uint32)(&Image$$VIDEOWIN_CODE$$Length),

                (uint32)(&Load$$VIDEOWIN_DATA$$Base),
                (uint32)(&Image$$VIDEOWIN_DATA$$RW$$Base),
                (uint32)(&Image$$VIDEOWIN_DATA$$RW$$Length),

                (uint32)(&Image$$VIDEOWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$VIDEOWIN_DATA$$ZI$$Length),
            },

            //VIDEO CONTROL
            {
                (uint32)(&Load$$VIDEO_CONTROL_CODE$$Base),
                (uint32)(&Image$$VIDEO_CONTROL_CODE$$Base),
                (uint32)(&Image$$VIDEO_CONTROL_CODE$$Length),

                (uint32)(&Load$$VIDEO_CONTROL_DATA$$Base),
                (uint32)(&Image$$VIDEO_CONTROL_DATA$$RW$$Base),
                (uint32)(&Image$$VIDEO_CONTROL_DATA$$RW$$Length),

                (uint32)(&Image$$VIDEO_CONTROL_DATA$$ZI$$Base),
                (uint32)(&Image$$VIDEO_CONTROL_DATA$$ZI$$Length),
            },

            //VIDEO_Mp2 Decode
            {
                (uint32)(&Load$$MP2_DECODE_CODE$$Base),
                (uint32)(&Image$$MP2_DECODE_CODE$$Base),
                (uint32)(&Image$$MP2_DECODE_CODE$$Length),

                (uint32)(&Load$$MP2_DECODE_DATA$$Base),
                (uint32)(&Image$$MP2_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$MP2_DECODE_DATA$$RW$$Length),

                (uint32)(&Image$$MP2_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$MP2_DECODE_DATA$$ZI$$Length),
            },

            //AVI Decode
            {
                (uint32)(&Load$$AVI_DECODE_CODE$$Base),
                (uint32)(&Image$$AVI_DECODE_CODE$$Base),
                (uint32)(&Image$$AVI_DECODE_CODE$$Length),

                (uint32)(&Load$$AVI_DECODE_DATA$$Base),
                (uint32)(&Image$$AVI_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$AVI_DECODE_DATA$$RW$$Length),

                (uint32)(&Image$$AVI_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$AVI_DECODE_DATA$$ZI$$Length),
            },

            //PicWin
            {
                (uint32)(&Load$$PICWIN_CODE$$Base),
                (uint32)(&Image$$PICWIN_CODE$$Base),
                (uint32)(&Image$$PICWIN_CODE$$Length),

                (uint32)(&Load$$PICWIN_DATA$$Base),
                (uint32)(&Image$$PICWIN_DATA$$RW$$Base),
                (uint32)(&Image$$PICWIN_DATA$$RW$$Length),

                (uint32)(&Image$$PICWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$PICWIN_DATA$$ZI$$Length),
            },

            //PicWin jin cheng
            {
                (uint32)(&Load$$IMAGE_CONTROL_CODE$$Base),
                (uint32)(&Image$$IMAGE_CONTROL_CODE$$Base),
                (uint32)(&Image$$IMAGE_CONTROL_CODE$$Length),

                (uint32)(&Load$$IMAGE_CONTROL_DATA$$Base),
                (uint32)(&Image$$IMAGE_CONTROL_DATA$$RW$$Base),
                (uint32)(&Image$$IMAGE_CONTROL_DATA$$RW$$Length),

                (uint32)(&Image$$IMAGE_CONTROL_DATA$$ZI$$Base),
                (uint32)(&Image$$IMAGE_CONTROL_DATA$$ZI$$Length),
            },

            //JPG Decode
            {
                (uint32)( &Load$$JPG_DECODE_CODE$$Base),
                (uint32)(&Image$$JPG_DECODE_CODE$$Base),
                (uint32)(&Image$$JPG_DECODE_CODE$$Length),

                (uint32)( &Load$$JPG_DECODE_DATA$$Base),
                (uint32)(&Image$$JPG_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$JPG_DECODE_DATA$$RW$$Length),

                (uint32)(&Image$$JPG_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$JPG_DECODE_DATA$$ZI$$Length),
            },

            //BMP Decode
            {
                (uint32)( &Load$$BMP_DECODE_CODE$$Base),
                (uint32)(&Image$$BMP_DECODE_CODE$$Base),
                (uint32)(&Image$$BMP_DECODE_CODE$$Length),

                (uint32)( &Load$$BMP_DECODE_DATA$$Base),
                (uint32)(&Image$$BMP_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$BMP_DECODE_DATA$$RW$$Length),

                (uint32)(&Image$$BMP_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$BMP_DECODE_DATA$$ZI$$Length),
            },


            //UI ���˵�
            {
                (uint32)(&Load$$MAINMENU_CODE$$Base),
                (uint32)(&Image$$MAINMENU_CODE$$Base),
                (uint32)(&Image$$MAINMENU_CODE$$Length),

                (uint32)(&Load$$MAINMENU_DATA$$Base),
                (uint32)(&Image$$MAINMENU_DATA$$RW$$Base),
                (uint32)(&Image$$MAINMENU_DATA$$RW$$Length),

                (uint32)(&Image$$MAINMENU_DATA$$ZI$$Base),
                (uint32)(&Image$$MAINMENU_DATA$$ZI$$Length),
            },

            //ý���
            {
                (uint32)(&Load$$MEDIALIBWIN_CODE$$Base),
                (uint32)(&Image$$MEDIALIBWIN_CODE$$Base),
                (uint32)(&Image$$MEDIALIBWIN_CODE$$Length),

                (uint32)(&Load$$MEDIALIBWIN_DATA$$Base),
                (uint32)(&Image$$MEDIALIBWIN_DATA$$RW$$Base),
                (uint32)(&Image$$MEDIALIBWIN_DATA$$RW$$Length),

                (uint32)(&Image$$MEDIALIBWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MEDIALIBWIN_DATA$$ZI$$Length),
            },

            //ý������
            {
                (uint32)(&Load$$MEDIABROWIN_CODE$$Base),
                (uint32)(&Image$$MEDIABROWIN_CODE$$Base),
                (uint32)(&Image$$MEDIABROWIN_CODE$$Length),

                (uint32)(&Load$$MEDIABROWIN_DATA$$Base),
                (uint32)(&Image$$MEDIABROWIN_DATA$$RW$$Base),
                (uint32)(&Image$$MEDIABROWIN_DATA$$RW$$Length),

                (uint32)(&Image$$MEDIABROWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MEDIABROWIN_DATA$$ZI$$Length),
            },

            {//ý��� mediasortget
                (uint32)(&Load$$MEDIABRO_SORTGET_CODE$$Base),
                (uint32)(&Image$$MEDIABRO_SORTGET_CODE$$Base),
                (uint32)(&Image$$MEDIABRO_SORTGET_CODE$$Length),

                (uint32)(&Load$$MEDIABRO_SORTGET_DATA$$Base),
                (uint32)(&Image$$MEDIABRO_SORTGET_DATA$$RW$$Base),
                (uint32)(&Image$$MEDIABRO_SORTGET_DATA$$RW$$Length),

                (uint32)(&Image$$MEDIABRO_SORTGET_DATA$$ZI$$Base),
                (uint32)(&Image$$MEDIABRO_SORTGET_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$MEDIABROSUBWIN_CODE$$Base),
                (uint32)(&Image$$MEDIABROSUBWIN_CODE$$Base),
                (uint32)(&Image$$MEDIABROSUBWIN_CODE$$Length),

                (uint32)(&Load$$MEDIABROSUBWIN_DATA$$Base),
                (uint32)(&Image$$MEDIABROSUBWIN_DATA$$RW$$Base),
                (uint32)(&Image$$MEDIABROSUBWIN_DATA$$RW$$Length),

                (uint32)(&Image$$MEDIABROSUBWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MEDIABROSUBWIN_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$MEDIAFAVOSUBWIN_CODE$$Base),
                (uint32)(&Image$$MEDIAFAVOSUBWIN_CODE$$Base),
                (uint32)(&Image$$MEDIAFAVOSUBWIN_CODE$$Length),

                (uint32)(&Load$$MEDIAFAVOSUBWIN_DATA$$Base),
                (uint32)(&Image$$MEDIAFAVOSUBWIN_DATA$$RW$$Base),
                (uint32)(&Image$$MEDIAFAVOSUBWIN_DATA$$RW$$Length),

                (uint32)(&Image$$MEDIAFAVOSUBWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MEDIAFAVOSUBWIN_DATA$$ZI$$Length),
            } ,

            //UI ���ֲ���
            {
                (uint32)(&Load$$MUSICWIN_CODE$$Base),
                (uint32)(&Image$$MUSICWIN_CODE$$Base),
                (uint32)(&Image$$MUSICWIN_CODE$$Length),

                (uint32)(&Load$$MUSICWIN_DATA$$Base),
                (uint32)(&Image$$MUSICWIN_DATA$$RW$$Base),
                (uint32)(&Image$$MUSICWIN_DATA$$RW$$Length),

                (uint32)(&Image$$MUSICWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MUSICWIN_DATA$$ZI$$Length),
            },

            //UI������
            {
                (uint32)(&Load$$RADIOWIN_CODE$$Base),
                (uint32)(&Image$$RADIOWIN_CODE$$Base),
                (uint32)(&Image$$RADIOWIN_CODE$$Length),

                (uint32)(&Load$$RADIOWIN_DATA$$Base),
                (uint32)(&Image$$RADIOWIN_DATA$$RW$$Base),
                (uint32)(&Image$$RADIOWIN_DATA$$RW$$Length),

                (uint32)(&Image$$RADIOWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$RADIOWIN_DATA$$ZI$$Length),
            },

            //UI ¼������
            {
                (uint32)(&Load$$RECORDWIN_CODE$$Base),
                (uint32)(&Image$$RECORDWIN_CODE$$Base),
                (uint32)(&Image$$RECORDWIN_CODE$$Length),

                (uint32)(&Load$$RECORDWIN_DATA$$Base),
                (uint32)(&Image$$RECORDWIN_DATA$$RW$$Base),
                (uint32)(&Image$$RECORDWIN_DATA$$RW$$Length),

                (uint32)(&Image$$RECORDWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$RECORDWIN_DATA$$ZI$$Length),
            },

            //UI������
            {
                (uint32)(&Load$$TEXTWIN_CODE$$Base),
                (uint32)(&Image$$TEXTWIN_CODE$$Base),
                (uint32)(&Image$$TEXTWIN_CODE$$Length),

                (uint32)(&Load$$TEXTWIN_DATA$$Base),
                (uint32)(&Image$$TEXTWIN_DATA$$RW$$Base),
                (uint32)(&Image$$TEXTWIN_DATA$$RW$$Length),

                (uint32)(&Image$$TEXTWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$TEXTWIN_DATA$$ZI$$Length),
            },

            //UI��Դ������
            {
                (uint32)(&Load$$BROWSER_CODE$$Base),
                (uint32)(&Image$$BROWSER_CODE$$Base),
                (uint32)(&Image$$BROWSER_CODE$$Length),

                (uint32)(&Load$$BROWSER_DATA$$Base),
                (uint32)(&Image$$BROWSER_DATA$$RW$$Base),
                (uint32)(&Image$$BROWSER_DATA$$RW$$Length),

                (uint32)(&Image$$BROWSER_DATA$$ZI$$Base),
                (uint32)(&Image$$BROWSER_DATA$$ZI$$Length),
            },

#ifdef _M3U_
            {
                (uint32)(&Load$$M3UBROWIN_CODE$$Base),
                (uint32)(&Image$$M3UBROWIN_CODE$$Base),
                (uint32)(&Image$$M3UBROWIN_CODE$$Length),

                (uint32)(&Load$$M3UBROWIN_DATA$$Base),
                (uint32)(&Image$$M3UBROWIN_DATA$$RW$$Base),
                (uint32)(&Image$$M3UBROWIN_DATA$$RW$$Length),

                (uint32)(&Image$$M3UBROWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$M3UBROWIN_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$M3UBRO_SORTGET_CODE$$Base),
                (uint32)(&Image$$M3UBRO_SORTGET_CODE$$Base),
                (uint32)(&Image$$M3UBRO_SORTGET_CODE$$Length),

                (uint32)(&Load$$M3UBRO_SORTGET_DATA$$Base),
                (uint32)(&Image$$M3UBRO_SORTGET_DATA$$RW$$Base),
                (uint32)(&Image$$M3UBRO_SORTGET_DATA$$RW$$Length),

                (uint32)(&Image$$M3UBRO_SORTGET_DATA$$ZI$$Base),
                (uint32)(&Image$$M3UBRO_SORTGET_DATA$$ZI$$Length),
            },

            //UI
            {
                (uint32)(&Load$$M3U_CODE$$Base),
                (uint32)(&Image$$M3U_CODE$$Base),
                (uint32)(&Image$$M3U_CODE$$Length),

                (uint32)(&Load$$M3U_DATA$$Base),
                (uint32)(&Image$$M3U_DATA$$RW$$Base),
                (uint32)(&Image$$M3U_DATA$$RW$$Length),

                (uint32)(&Image$$M3U_DATA$$ZI$$Base),
                (uint32)(&Image$$M3U_DATA$$ZI$$Length),
            },
#endif

            {
                (uint32)(&Load$$PICBROWIN_CODE$$Base),
                (uint32)(&Image$$PICBROWIN_CODE$$Base),
                (uint32)(&Image$$PICBROWIN_CODE$$Length),

                (uint32)(&Load$$PICBROWIN_DATA$$Base),
                (uint32)(&Image$$PICBROWIN_DATA$$RW$$Base),
                (uint32)(&Image$$PICBROWIN_DATA$$RW$$Length),

                (uint32)(&Image$$PICBROWIN_DATA$$ZI$$Base),
                (uint32)(&Image$$PICBROWIN_DATA$$ZI$$Length),
            },

            //UI���ò˵�
            {
                (uint32)(&Load$$SETMENU_CODE$$Base),
                (uint32)(&Image$$SETMENU_CODE$$Base),
                (uint32)(&Image$$SETMENU_CODE$$Length),

                (uint32)(&Load$$SETMENU_DATA$$Base),
                (uint32)(&Image$$SETMENU_DATA$$RW$$Base),
                (uint32)(&Image$$SETMENU_DATA$$RW$$Length),

                (uint32)(&Image$$SETMENU_DATA$$ZI$$Base),
                (uint32)(&Image$$SETMENU_DATA$$ZI$$Length),
            },

            //CONSOLE ���ֲ��ź�̨
            {
                (uint32)(&Load$$AUDIO_CONTROL_CODE$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_CODE$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_CODE$$Length),

                (uint32)(&Load$$AUDIO_CONTROL_DATA$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_DATA$$RW$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_DATA$$RW$$Length),

                (uint32)(&Image$$AUDIO_CONTROL_DATA$$ZI$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$AUDIO_CONTROL_INIT_CODE$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_INIT_CODE$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_INIT_CODE$$Length),

                (uint32)(&Load$$AUDIO_CONTROL_INIT_DATA$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_INIT_DATA$$RW$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_INIT_DATA$$RW$$Length),

                (uint32)(&Image$$AUDIO_CONTROL_INIT_DATA$$ZI$$Base),
                (uint32)(&Image$$AUDIO_CONTROL_INIT_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$AUDIO_EQ_CODE$$Base),
                (uint32)(&Image$$AUDIO_EQ_CODE$$Base),
                (uint32)(&Image$$AUDIO_EQ_CODE$$Length),

                (uint32)(&Load$$AUDIO_EQ_DATA$$Base),
                (uint32)(&Image$$AUDIO_EQ_DATA$$RW$$Base),
                (uint32)(&Image$$AUDIO_EQ_DATA$$RW$$Length),

                (uint32)(&Image$$AUDIO_EQ_DATA$$ZI$$Base),
                (uint32)(&Image$$AUDIO_EQ_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$AUDIO_RKEQ_CODE$$Base),
                (uint32)(&Image$$AUDIO_RKEQ_CODE$$Base),
                (uint32)(&Image$$AUDIO_RKEQ_CODE$$Length),

                (uint32)(&Load$$AUDIO_RKEQ_DATA$$Base),
                (uint32)(&Image$$AUDIO_RKEQ_DATA$$RW$$Base),
                (uint32)(&Image$$AUDIO_RKEQ_DATA$$RW$$Length),

                (uint32)(&Image$$AUDIO_RKEQ_DATA$$ZI$$Base),
                (uint32)(&Image$$AUDIO_RKEQ_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$AUDIO_ID3_CODE$$Base),
                (uint32)(&Image$$AUDIO_ID3_CODE$$Base),
                (uint32)(&Image$$AUDIO_ID3_CODE$$Length),

                (uint32)(&Load$$AUDIO_ID3_DATA$$Base),
                (uint32)(&Image$$AUDIO_ID3_DATA$$RW$$Base),
                (uint32)(&Image$$AUDIO_ID3_DATA$$RW$$Length),

                (uint32)(&Image$$AUDIO_ID3_DATA$$ZI$$Base),
                (uint32)(&Image$$AUDIO_ID3_DATA$$ZI$$Length),
            },

            //CONSOLE ý���
            {
                (uint32)(&Load$$FILE_INFO_SAVE_CODE$$Base),
                (uint32)(&Image$$FILE_INFO_SAVE_CODE$$Base),
                (uint32)(&Image$$FILE_INFO_SAVE_CODE$$Length),

                (uint32)(&Load$$FILE_INFO_SAVE_DATA$$Base),
                (uint32)(&Image$$FILE_INFO_SAVE_DATA$$RW$$Base),
                (uint32)(&Image$$FILE_INFO_SAVE_DATA$$RW$$Length),

                (uint32)(&Image$$FILE_INFO_SAVE_DATA$$ZI$$Base),
                (uint32)(&Image$$FILE_INFO_SAVE_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$FILE_INFO_SORT_CODE$$Base),
                (uint32)(&Image$$FILE_INFO_SORT_CODE$$Base),
                (uint32)(&Image$$FILE_INFO_SORT_CODE$$Length),

                (uint32)(&Load$$FILE_INFO_SORT_DATA$$Base),
                (uint32)(&Image$$FILE_INFO_SORT_DATA$$RW$$Base),
                (uint32)(&Image$$FILE_INFO_SORT_DATA$$RW$$Length),

                (uint32)(&Image$$FILE_INFO_SORT_DATA$$ZI$$Base),
                (uint32)(&Image$$FILE_INFO_SORT_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$FAVORESET_CODE$$Base),
                (uint32)(&Image$$FAVORESET_CODE$$Base),
                (uint32)(&Image$$FAVORESET_CODE$$Length),

                (uint32)(&Load$$FAVORESET_DATA$$Base),
                (uint32)(&Image$$FAVORESET_DATA$$RW$$Base),
                (uint32)(&Image$$FAVORESET_DATA$$RW$$Length),

                (uint32)(&Image$$FAVORESET_DATA$$ZI$$Base),
                (uint32)(&Image$$FAVORESET_DATA$$ZI$$Length),
            },

//-------------------------decode segment---------------------------------------
            //MP3
            {
                (uint32)(&Load$$MP3_DECODE_CODE$$Base),
                (uint32)(&Image$$MP3_DECODE_CODE$$Base),
                (uint32)(&Image$$MP3_DECODE_CODE$$Length),

                (uint32)(&Load$$MP3_DECODE_DATA$$Base),
                (uint32)(&Image$$MP3_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$MP3_DECODE_DATA$$RW$$Length),

                (uint32)(&Image$$MP3_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$MP3_DECODE_DATA$$ZI$$Length),
            },

            //WMA
            {
                (uint32)(& Load$$WMA_COMMON_CODE$$Base),
                (uint32)(&Image$$WMA_COMMON_CODE$$Base),
                (uint32)(&Image$$WMA_COMMON_CODE$$Length),
                (uint32)(& Load$$WMA_COMMON_DATA$$Base),
                (uint32)(&Image$$WMA_COMMON_DATA$$RW$$Base),
                (uint32)(&Image$$WMA_COMMON_DATA$$RW$$Length),
                (uint32)(&Image$$WMA_COMMON_DATA$$ZI$$Base),
                (uint32)(&Image$$WMA_COMMON_DATA$$ZI$$Length),
            },

            //WAV
            {
                (uint32)(&Load$$WAV_DECODE_CODE$$Base),
                (uint32)(&Image$$WAV_DECODE_CODE$$Base),
                (uint32)(&Image$$WAV_DECODE_CODE$$Length),

                (uint32)(&Load$$WAV_DECODE_DATA$$Base),
                (uint32)(&Image$$WAV_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$WAV_DECODE_DATA$$RW$$Length),

                (uint32)(&Image$$WAV_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$WAV_DECODE_DATA$$ZI$$Length),
            },
            //FALC
            {
                (uint32)(&Load$$FLAC_DECODE_CODE$$Base),
                (uint32)(&Image$$FLAC_DECODE_CODE$$Base),
                (uint32)(&Image$$FLAC_DECODE_CODE$$Length),
                (uint32)(&Load$$FLAC_DECODE_DATA$$Base),
                (uint32)(&Image$$FLAC_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$FLAC_DECODE_DATA$$RW$$Length),
                (uint32)(&Image$$FLAC_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$FLAC_DECODE_DATA$$ZI$$Length),
            },

            //AAC
            {
                (uint32)(&Load$$AAC_DECODE_CODE$$Base),
                (uint32)(&Image$$AAC_DECODE_CODE$$Base),
                (uint32)(&Image$$AAC_DECODE_CODE$$Length),
                (uint32)(&Load$$AAC_DECODE_DATA$$Base),
                (uint32)(&Image$$AAC_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$AAC_DECODE_DATA$$RW$$Length),
                (uint32)(&Image$$AAC_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$AAC_DECODE_DATA$$ZI$$Length),
            },
            //APE
            {
                (uint32)(&Load$$APE_DECODE_CODE$$Base),
                (uint32)(&Image$$APE_DECODE_CODE$$Base),
                (uint32)(&Image$$APE_DECODE_CODE$$Length),
                (uint32)(&Load$$APE_DECODE_DATA$$Base),
                (uint32)(&Image$$APE_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$APE_DECODE_DATA$$RW$$Length),
                (uint32)(&Image$$APE_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$APE_DECODE_DATA$$ZI$$Length),
            },
            //OGG
            {
                (uint32)(&Load$$OGG_DECODE_CODE$$Base),
                (uint32)(&Image$$OGG_DECODE_CODE$$Base),
                (uint32)(&Image$$OGG_DECODE_CODE$$Length),
                (uint32)(&Load$$OGG_DECODE_DATA$$Base),
                (uint32)(&Image$$OGG_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$OGG_DECODE_DATA$$RW$$Length),
                (uint32)(&Image$$OGG_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$OGG_DECODE_DATA$$ZI$$Length),
            },

            //HIFI FLAC
            {
                (uint32)(&Load$$FLAC_HDECODE_CODE$$Base),
                (uint32)(&Image$$FLAC_HDECODE_CODE$$Base),
                (uint32)(&Image$$FLAC_HDECODE_CODE$$Length),
                (uint32)(&Load$$FLAC_HDECODE_DATA$$Base),
                (uint32)(&Image$$FLAC_HDECODE_DATA$$RW$$Base),
                (uint32)(&Image$$FLAC_HDECODE_DATA$$RW$$Length),
                (uint32)(&Image$$FLAC_HDECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$FLAC_HDECODE_DATA$$ZI$$Length),
            },

            //HIFI ALAC
            {
                (uint32)(&Load$$ALAC_HDECODE_CODE$$Base),
                (uint32)(&Image$$ALAC_HDECODE_CODE$$Base),
                (uint32)(&Image$$ALAC_HDECODE_CODE$$Length),
                (uint32)(&Load$$ALAC_HDECODE_DATA$$Base),
                (uint32)(&Image$$ALAC_HDECODE_DATA$$RW$$Base),
                (uint32)(&Image$$ALAC_HDECODE_DATA$$RW$$Length),
                (uint32)(&Image$$ALAC_HDECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$ALAC_HDECODE_DATA$$ZI$$Length),
            },

            //HIFI APE
            {
                (uint32)(&Load$$APE_HDECODE_CODE$$Base),
                (uint32)(&Image$$APE_HDECODE_CODE$$Base),
                (uint32)(&Image$$APE_HDECODE_CODE$$Length),
                (uint32)(&Load$$APE_HDECODE_DATA$$Base),
                (uint32)(&Image$$APE_HDECODE_DATA$$RW$$Base),
                (uint32)(&Image$$APE_HDECODE_DATA$$RW$$Length),
                (uint32)(&Image$$APE_HDECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$APE_HDECODE_DATA$$ZI$$Length),
            },

            //DSD
            {
                (uint32)(&Load$$DSDIFF_DECODE_CODE$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_CODE$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_CODE$$Length),
                (uint32)(&Load$$DSDIFF_DECODE_DATA$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_DATA$$RW$$Length),
                (uint32)(&Image$$DSDIFF_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_DATA$$ZI$$Length),
            },

           //DSF
            {
                (uint32)(&Load$$DSF_DECODE_CODE$$Base),
                (uint32)(&Image$$DSF_DECODE_CODE$$Base),
                (uint32)(&Image$$DSF_DECODE_CODE$$Length),
                (uint32)(&Load$$DSF_DECODE_DATA$$Base),
                (uint32)(&Image$$DSF_DECODE_DATA$$RW$$Base),
                (uint32)(&Image$$DSF_DECODE_DATA$$RW$$Length),
                (uint32)(&Image$$DSF_DECODE_DATA$$ZI$$Base),
                (uint32)(&Image$$DSF_DECODE_DATA$$ZI$$Length),
            },


            //CONSOLE ������
            {
                (uint32)(&Load$$FM_CONTROL_CODE$$Base),
                (uint32)(&Image$$FM_CONTROL_CODE$$Base),
                (uint32)(&Image$$FM_CONTROL_CODE$$Length),

                (uint32)(&Load$$FM_CONTROL_DATA$$Base),
                (uint32)(&Image$$FM_CONTROL_DATA$$RW$$Base),
                (uint32)(&Image$$FM_CONTROL_DATA$$RW$$Length),

                (uint32)(&Image$$FM_CONTROL_DATA$$ZI$$Base),
                (uint32)(&Image$$FM_CONTROL_DATA$$ZI$$Length),
            },

            //FM ����
            {
                (uint32)(&Load$$FM_DRIVER1_CODE$$Base),
                (uint32)(&Image$$FM_DRIVER1_CODE$$Base),
                (uint32)(&Image$$FM_DRIVER1_CODE$$Length),

                (uint32)(&Load$$FM_DRIVER1_DATA$$Base),
                (uint32)(&Image$$FM_DRIVER1_DATA$$RW$$Base),
                (uint32)(&Image$$FM_DRIVER1_DATA$$RW$$Length),

                (uint32)(&Image$$FM_DRIVER1_DATA$$ZI$$Base),
                (uint32)(&Image$$FM_DRIVER1_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$FM_DRIVER2_CODE$$Base),
                (uint32)(&Image$$FM_DRIVER2_CODE$$Base),
                (uint32)(&Image$$FM_DRIVER2_CODE$$Length),

                (uint32)(&Load$$FM_DRIVER2_DATA$$Base),
                (uint32)(&Image$$FM_DRIVER2_DATA$$RW$$Base),
                (uint32)(&Image$$FM_DRIVER2_DATA$$RW$$Length),

                (uint32)(&Image$$FM_DRIVER2_DATA$$ZI$$Base),
                (uint32)(&Image$$FM_DRIVER2_DATA$$ZI$$Length),
            },

            //CONSOLE ¼��
            {
                (uint32)(&Load$$RECORD_CONTROL_CODE$$Base),
                (uint32)(&Image$$RECORD_CONTROL_CODE$$Base),
                (uint32)(&Image$$RECORD_CONTROL_CODE$$Length),

                (uint32)(&Load$$RECORD_CONTROL_DATA$$Base),
                (uint32)(&Image$$RECORD_CONTROL_DATA$$RW$$Base),
                (uint32)(&Image$$RECORD_CONTROL_DATA$$RW$$Length),

                (uint32)(&Image$$RECORD_CONTROL_DATA$$ZI$$Base),
                (uint32)(&Image$$RECORD_CONTROL_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$ENCODE_MSADPCM_CODE$$Base),
                (uint32)(&Image$$ENCODE_MSADPCM_CODE$$Base),
                (uint32)(&Image$$ENCODE_MSADPCM_CODE$$Length),

                (uint32)(&Load$$ENCODE_MSADPCM_DATA$$Base),
                (uint32)(&Image$$ENCODE_MSADPCM_DATA$$RW$$Base),
                (uint32)(&Image$$ENCODE_MSADPCM_DATA$$RW$$Length),

                (uint32)(&Image$$ENCODE_MSADPCM_DATA$$ZI$$Base),
                (uint32)(&Image$$ENCODE_MSADPCM_DATA$$ZI$$Length),
            },

            //MP3 ENCODE
            {
                (uint32)(&Load$$ENCODE_MP3_CODE$$Base),
                (uint32)(&Image$$ENCODE_MP3_CODE$$Base),
                (uint32)(&Image$$ENCODE_MP3_CODE$$Length),

                (uint32)(&Load$$ENCODE_MP3_DATA$$Base),
                (uint32)(&Image$$ENCODE_MP3_DATA$$RW$$Base),
                (uint32)(&Image$$ENCODE_MP3_DATA$$RW$$Length),

                (uint32)(&Image$$ENCODE_MP3_DATA$$ZI$$Base),
                (uint32)(&Image$$ENCODE_MP3_DATA$$ZI$$Length),
            },

            //BEEP
            {
                (uint32)(&Load$$SEID0000_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_8K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_8K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0000_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_11K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_11K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0000_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_12K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_12K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },


            {
                (uint32)(&Load$$SEID0000_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_16K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_16K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0000_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_22K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_22K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0000_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_24K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_24K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0000_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_32K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_32K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0000_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_44K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_44K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0000_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0000_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0001_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0001_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0002_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0002_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0003_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0003_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0004_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0004_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0005_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0005_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0006_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0006_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            {
                (uint32)(&Load$$SEID0007_48K_CODE$$Base),
                NULL,
                (uint32)(&Image$$SEID0007_48K_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

            // asic to unicode
            {
                (uint32)(&Load$$CP1251_UNICODE_TABLE_CODE$$Base),
                NULL,
                (uint32)(&Image$$CP1251_UNICODE_TABLE_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },
            {
                (uint32)(&Load$$CP932_UNICODE_TABLE_CODE$$Base),
                NULL,
                (uint32)(&Image$$CP932_UNICODE_TABLE_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },
            {
                (uint32)(&Load$$CP932_UNICODE_TABLE1_CODE$$Base),
                NULL,
                (uint32)(&Image$$CP932_UNICODE_TABLE1_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },
            {
                (uint32)(&Load$$CP932_UNICODE_TABLE2_CODE$$Base),
                NULL,
                (uint32)(&Image$$CP932_UNICODE_TABLE2_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },
            {
                (uint32)(&Load$$CP950_UNICODE_TABLE_CODE$$Base),
                NULL,
                (uint32)(&Image$$CP950_UNICODE_TABLE_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },
            {
                (uint32)(&Load$$CP949_UNICODE_TABLE_CODE$$Base),
                NULL,
                (uint32)(&Image$$CP949_UNICODE_TABLE_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },
            {
                (uint32)(&Load$$CP949_UNICODE_TABLE1_CODE$$Base),
                NULL,
                (uint32)(&Image$$CP949_UNICODE_TABLE1_CODE$$Length),

                NULL,
                NULL,
                NULL,

                NULL,
                NULL,
            },

//----------------------------BB Module ---------------------------------------
            //bb_core
            {
                (uint32)(&Load$$BB_SYS_CODE$$Base),
                (uint32)(&Image$$BB_SYS_CODE$$Base),
                (uint32)(&Image$$BB_SYS_CODE$$Length),

                (uint32)(&Load$$BB_SYS_DATA$$Base),
                (uint32)(&Image$$BB_SYS_DATA$$RW$$Base),
                (uint32)(&Image$$BB_SYS_DATA$$RW$$Length),

                (uint32)(&Image$$BB_SYS_DATA$$ZI$$Base),
                (uint32)(&Image$$BB_SYS_DATA$$ZI$$Length),
            },

            //mp3 encode
            {
                (uint32)(&Load$$MP3_ENCODE_BIN_CODE$$Base),
                (uint32)(&Image$$MP3_ENCODE_BIN_CODE$$Base),
                (uint32)(&Image$$MP3_ENCODE_BIN_CODE$$Length),

                (uint32)(&Load$$MP3_ENCODE_BIN_DATA$$Base),
                (uint32)(&Image$$MP3_ENCODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$MP3_ENCODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$MP3_ENCODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MP3_ENCODE_BIN_DATA$$ZI$$Length),
            },
            //MP3 decode
            {
                (uint32)(&Load$$MP3_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$MP3_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$MP3_DECODE_BIN_CODE$$Length),

                (uint32)(&Load$$MP3_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$MP3_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$MP3_DECODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$MP3_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$MP3_DECODE_BIN_DATA$$ZI$$Length),
            },

            //WMA
            {
                (uint32)(&Load$$WMA_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$WMA_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$WMA_DECODE_BIN_CODE$$Length),

                (uint32)(&Load$$WMA_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$WMA_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$WMA_DECODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$WMA_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$WMA_DECODE_BIN_DATA$$ZI$$Length),
            },

            //WAV
            {
                (uint32)(&Load$$WAV_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$WAV_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$WAV_DECODE_BIN_CODE$$Length),

                (uint32)(&Load$$WAV_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$WAV_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$WAV_DECODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$WAV_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$WAV_DECODE_BIN_DATA$$ZI$$Length),
            },


            //FLAC
            {
                (uint32)(&Load$$FLAC_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$FLAC_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$FLAC_DECODE_BIN_CODE$$Length),
                (uint32)(&Load$$FLAC_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$FLAC_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$FLAC_DECODE_BIN_DATA$$RW$$Length),
                (uint32)(&Image$$FLAC_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$FLAC_DECODE_BIN_DATA$$ZI$$Length),
            },


            //AAC
            {
                (uint32)(&Load$$AAC_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$AAC_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$AAC_DECODE_BIN_CODE$$Length),
                (uint32)(&Load$$AAC_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$AAC_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$AAC_DECODE_BIN_DATA$$RW$$Length),
                (uint32)(&Image$$AAC_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$AAC_DECODE_BIN_DATA$$ZI$$Length),
            },

            //APE
            {
                (uint32)(&Load$$APE_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$APE_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$APE_DECODE_BIN_CODE$$Length),
                (uint32)(&Load$$APE_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$APE_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$APE_DECODE_BIN_DATA$$RW$$Length),
                (uint32)(&Image$$APE_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$APE_DECODE_BIN_DATA$$ZI$$Length),
            },

            //OGG
            {
                (uint32)(&Load$$OGG_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$OGG_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$OGG_DECODE_BIN_CODE$$Length),

                (uint32)(&Load$$OGG_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$OGG_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$OGG_DECODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$OGG_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$OGG_DECODE_BIN_DATA$$ZI$$Length),
            },

            //HIFI FLAC
            {
                (uint32)(&Load$$FLAC_HDECODE_BIN_CODE$$Base),
                (uint32)(&Image$$FLAC_HDECODE_BIN_CODE$$Base),
                (uint32)(&Image$$FLAC_HDECODE_BIN_CODE$$Length),

                (uint32)(&Load$$FLAC_HDECODE_BIN_DATA$$Base),
                (uint32)(&Image$$FLAC_HDECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$FLAC_HDECODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$FLAC_HDECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$FLAC_HDECODE_BIN_DATA$$ZI$$Length),
            },

            //HIFI ALAC
            {
                (uint32)(&Load$$ALAC_HDECODE_BIN_CODE$$Base),
                (uint32)(&Image$$ALAC_HDECODE_BIN_CODE$$Base),
                (uint32)(&Image$$ALAC_HDECODE_BIN_CODE$$Length),

                (uint32)(&Load$$ALAC_HDECODE_BIN_DATA$$Base),
                (uint32)(&Image$$ALAC_HDECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$ALAC_HDECODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$ALAC_HDECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$ALAC_HDECODE_BIN_DATA$$ZI$$Length),
            },

            //HIFI APE
            {
                (uint32)(&Load$$APE_HDECODE_BIN_CODE$$Base),
                (uint32)(&Image$$APE_HDECODE_BIN_CODE$$Base),
                (uint32)(&Image$$APE_HDECODE_BIN_CODE$$Length),

                (uint32)(&Load$$APE_HDECODE_BIN_DATA$$Base),
                (uint32)(&Image$$APE_HDECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$APE_HDECODE_BIN_DATA$$RW$$Length),

                (uint32)(&Image$$APE_HDECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$APE_HDECODE_BIN_DATA$$ZI$$Length),
            },

            //DSD
            {
                (uint32)(&Load$$DSDIFF_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_BIN_CODE$$Length),
                (uint32)(&Load$$DSDIFF_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_BIN_DATA$$RW$$Length),
                (uint32)(&Image$$DSDIFF_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$DSDIFF_DECODE_BIN_DATA$$ZI$$Length),
            },

             //DSF
            {
                (uint32)(&Load$$DSF_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$DSF_DECODE_BIN_CODE$$Base),
                (uint32)(&Image$$DSF_DECODE_BIN_CODE$$Length),
                (uint32)(&Load$$DSF_DECODE_BIN_DATA$$Base),
                (uint32)(&Image$$DSF_DECODE_BIN_DATA$$RW$$Base),
                (uint32)(&Image$$DSF_DECODE_BIN_DATA$$RW$$Length),
                (uint32)(&Image$$DSF_DECODE_BIN_DATA$$ZI$$Base),
                (uint32)(&Image$$DSF_DECODE_BIN_DATA$$ZI$$Length),
            },

            //BT Control
            {
                (uint32)(&Load$$BTCONTROL_CODE$$Base),
                (uint32)(&Image$$BTCONTROL_CODE$$Base),
                (uint32)(&Image$$BTCONTROL_CODE$$Length),

                (uint32)(&Load$$BTCONTROL_DATA$$Base),
                (uint32)(&Image$$BTCONTROL_DATA$$RW$$Base),
                (uint32)(&Image$$BTCONTROL_DATA$$RW$$Length),

                (uint32)(&Image$$BTCONTROL_DATA$$ZI$$Base),
                (uint32)(&Image$$BTCONTROL_DATA$$ZI$$Length),
            },

            #ifdef _A2DP_SOUCRE_
            //lwbt_uartif
            {
                (uint32)(&Load$$LWBT_UARTIF_CODE$$Base),
                (uint32)(&Image$$LWBT_UARTIF_CODE$$Base),
                (uint32)(&Image$$LWBT_UARTIF_CODE$$Length),

                (uint32)(&Load$$LWBT_UARTIF_DATA$$Base),
                (uint32)(&Image$$LWBT_UARTIF_DATA$$RW$$Base),
                (uint32)(&Image$$LWBT_UARTIF_DATA$$RW$$Length),

                (uint32)(&Image$$LWBT_UARTIF_DATA$$ZI$$Base),
                (uint32)(&Image$$LWBT_UARTIF_DATA$$ZI$$Length),
            },

            //lwbt_init
            {
                (uint32)(&Load$$LWBT_INIT_CODE$$Base),
                (uint32)(&Image$$LWBT_INIT_CODE$$Base),
                (uint32)(&Image$$LWBT_INIT_CODE$$Length),

                (uint32)(&Load$$LWBT_INIT_DATA$$Base),
                (uint32)(&Image$$LWBT_INIT_DATA$$RW$$Base),
                (uint32)(&Image$$LWBT_INIT_DATA$$RW$$Length),

                (uint32)(&Image$$LWBT_INIT_DATA$$ZI$$Base),
                (uint32)(&Image$$LWBT_INIT_DATA$$ZI$$Length),
            },
            #else
            {
                (uint32)0,
                (uint32)0,
                (uint32)0,

                (uint32)0,
                (uint32)0,
                (uint32)0,

                (uint32)0,
                (uint32)0,
            },
            {
                (uint32)0,
                (uint32)0,
                (uint32)0,

                (uint32)0,
                (uint32)0,
                (uint32)0,

                (uint32)0,
                (uint32)0,
            },

            #endif

            //lwbt
            {
                (uint32)(&Load$$LWBT_CODE$$Base),
                (uint32)(&Image$$LWBT_CODE$$Base),
                (uint32)(&Image$$LWBT_CODE$$Length),

                (uint32)(&Load$$LWBT_DATA$$Base),
                (uint32)(&Image$$LWBT_DATA$$RW$$Base),
                (uint32)(&Image$$LWBT_DATA$$RW$$Length),

                (uint32)(&Image$$LWBT_DATA$$ZI$$Base),
                (uint32)(&Image$$LWBT_DATA$$ZI$$Length),
            },

            //sbc encode
            {
                (uint32)(&Load$$SBC_ENCODE_CODE$$Base),
                (uint32)(&Image$$SBC_ENCODE_CODE$$Base),
                (uint32)(&Image$$SBC_ENCODE_CODE$$Length),

                (uint32)(&Load$$SBC_ENCODE_DATA$$Base),
                (uint32)(&Image$$SBC_ENCODE_DATA$$RW$$Base),
                (uint32)(&Image$$SBC_ENCODE_DATA$$RW$$Length),

                (uint32)(&Image$$SBC_ENCODE_DATA$$ZI$$Base),
                (uint32)(&Image$$SBC_ENCODE_DATA$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_CODE$$Base),
                (uint32)(&Image$$SRC_CODE$$Base),
                (uint32)(&Image$$SRC_CODE$$Length),

                (uint32)(&Load$$SRC_DATA$$Base),
                (uint32)(&Image$$SRC_DATA$$RW$$Base),
                (uint32)(&Image$$SRC_DATA$$RW$$Length),

                (uint32)(&Image$$SRC_DATA$$ZI$$Base),
                (uint32)(&Image$$SRC_DATA$$ZI$$Length),
            },
            
            {
                (uint32)(&Load$$SRC_TABLE_CODE_192_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_192_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_192_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_192_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_192_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_192_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_192_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_192_44$$ZI$$Length),
            },
                
            {
                (uint32)(&Load$$SRC_TABLE_CODE_176_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_176_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_176_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_176_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_176_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_176_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_176_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_176_44$$ZI$$Length),
            },
                
            {
                (uint32)(&Load$$SRC_TABLE_CODE_96_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_96_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_96_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_96_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_96_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_96_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_96_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_96_44$$ZI$$Length),
            },
            
            {
                (uint32)(&Load$$SRC_TABLE_CODE_88_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_88_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_88_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_88_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_88_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_88_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_88_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_88_44$$ZI$$Length),
            },
                
            {
                (uint32)(&Load$$SRC_TABLE_CODE_64_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_64_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_64_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_64_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_64_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_64_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_64_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_64_44$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_TABLE_CODE_48_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_48_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_48_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_48_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_48_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_48_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_48_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_48_44$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_TABLE_CODE_32_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_32_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_32_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_32_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_32_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_32_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_32_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_32_44$$ZI$$Length),
            },


            {
                (uint32)(&Load$$SRC_TABLE_CODE_24_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_24_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_24_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_24_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_24_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_24_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_24_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_24_44$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_TABLE_CODE_22_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_22_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_22_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_22_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_22_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_22_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_22_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_22_44$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_TABLE_CODE_16_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_16_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_16_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_16_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_16_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_16_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_16_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_16_44$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_TABLE_CODE_12_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_12_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_12_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_12_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_12_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_12_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_12_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_12_44$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_TABLE_CODE_11_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_11_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_11_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_11_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_11_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_11_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_11_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_11_44$$ZI$$Length),
            },

            {
                (uint32)(&Load$$SRC_TABLE_CODE_8_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_8_44$$Base),
                (uint32)(&Image$$SRC_TABLE_CODE_8_44$$Length),

                (uint32)(&Load$$SRC_TABLE_DATA_8_44$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_8_44$$RW$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_8_44$$RW$$Length),

                (uint32)(&Image$$SRC_TABLE_DATA_8_44$$ZI$$Base),
                (uint32)(&Image$$SRC_TABLE_DATA_8_44$$ZI$$Length),
            },

            //BT init script
            {
                (uint32)(&Load$$BT_INIT_SCRIPT_CODE$$Base),
                (uint32)(&Image$$BT_INIT_SCRIPT_CODE$$Base),
                (uint32)(&Image$$BT_INIT_SCRIPT_CODE$$Length),

                (uint32)(0),
                (uint32)(0),
                (uint32)(0),

                (uint32)(0),
                (uint32)(0),
            },

            //TEST
            {
                (uint32)(&Load$$MAILBOX_TEST_CODE$$Base),
                (uint32)(&Image$$MAILBOX_TEST_CODE$$Base),
                (uint32)(&Image$$MAILBOX_TEST_CODE$$Length),
                (uint32)(&Load$$MAILBOX_TEST_DATA$$Base),
                (uint32)(&Image$$MAILBOX_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$MAILBOX_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$MAILBOX_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$MAILBOX_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$MEMORY_TEST_CODE$$Base),
                (uint32)(&Image$$MEMORY_TEST_CODE$$Base),
                (uint32)(&Image$$MEMORY_TEST_CODE$$Length),
                (uint32)(&Load$$MEMORY_TEST_DATA$$Base),
                (uint32)(&Image$$MEMORY_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$MEMORY_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$MEMORY_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$MEMORY_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$TIMER_TEST_CODE$$Base),
                (uint32)(&Image$$TIMER_TEST_CODE$$Base),
                (uint32)(&Image$$TIMER_TEST_CODE$$Length),
                (uint32)(&Load$$TIMER_TEST_DATA$$Base),
                (uint32)(&Image$$TIMER_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$TIMER_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$TIMER_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$TIMER_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$WDT_TEST_CODE$$Base),
                (uint32)(&Image$$WDT_TEST_CODE$$Base),
                (uint32)(&Image$$WDT_TEST_CODE$$Length),
                (uint32)(&Load$$WDT_TEST_DATA$$Base),
                (uint32)(&Image$$WDT_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$WDT_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$WDT_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$WDT_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$EMMC_TEST_CODE$$Base),
                (uint32)(&Image$$EMMC_TEST_CODE$$Base),
                (uint32)(&Image$$EMMC_TEST_CODE$$Length),
                (uint32)(&Load$$EMMC_TEST_DATA$$Base),
                (uint32)(&Image$$EMMC_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$EMMC_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$EMMC_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$EMMC_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$SDMMC_TEST_CODE$$Base),
                (uint32)(&Image$$SDMMC_TEST_CODE$$Base),
                (uint32)(&Image$$SDMMC_TEST_CODE$$Length),
                (uint32)(&Load$$SDMMC_TEST_DATA$$Base),
                (uint32)(&Image$$SDMMC_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$SDMMC_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$SDMMC_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$SDMMC_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$SPI_TEST_CODE$$Base),
                (uint32)(&Image$$SPI_TEST_CODE$$Base),
                (uint32)(&Image$$SPI_TEST_CODE$$Length),
                (uint32)(&Load$$SPI_TEST_DATA$$Base),
                (uint32)(&Image$$SPI_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$SPI_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$SPI_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$SPI_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$UART_TEST_CODE$$Base),
                (uint32)(&Image$$UART_TEST_CODE$$Base),
                (uint32)(&Image$$UART_TEST_CODE$$Length),
                (uint32)(&Load$$UART_TEST_DATA$$Base),
                (uint32)(&Image$$UART_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$UART_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$UART_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$UART_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$PWM_TEST_CODE$$Base),
                (uint32)(&Image$$PWM_TEST_CODE$$Base),
                (uint32)(&Image$$PWM_TEST_CODE$$Length),
                (uint32)(&Load$$PWM_TEST_DATA$$Base),
                (uint32)(&Image$$PWM_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$PWM_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$PWM_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$PWM_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$GPIO_TEST_CODE$$Base),
                (uint32)(&Image$$GPIO_TEST_CODE$$Base),
                (uint32)(&Image$$GPIO_TEST_CODE$$Length),
                (uint32)(&Load$$GPIO_TEST_DATA$$Base),
                (uint32)(&Image$$GPIO_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$GPIO_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$GPIO_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$GPIO_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$I2C_TEST_CODE$$Base),
                (uint32)(&Image$$I2C_TEST_CODE$$Base),
                (uint32)(&Image$$I2C_TEST_CODE$$Length),
                (uint32)(&Load$$I2C_TEST_DATA$$Base),
                (uint32)(&Image$$I2C_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$I2C_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$I2C_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$I2C_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$ADC_TEST_CODE$$Base),
                (uint32)(&Image$$ADC_TEST_CODE$$Base),
                (uint32)(&Image$$ADC_TEST_CODE$$Length),
                (uint32)(&Load$$ADC_TEST_DATA$$Base),
                (uint32)(&Image$$ADC_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$ADC_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$ADC_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$ADC_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$PVTM_TEST_CODE$$Base),
                (uint32)(&Image$$PVTM_TEST_CODE$$Base),
                (uint32)(&Image$$PVTM_TEST_CODE$$Length),
                (uint32)(&Load$$PVTM_TEST_DATA$$Base),
                (uint32)(&Image$$PVTM_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$PVTM_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$PVTM_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$PVTM_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$I2S_TEST_CODE$$Base),
                (uint32)(&Image$$I2S_TEST_CODE$$Base),
                (uint32)(&Image$$I2S_TEST_CODE$$Length),
                (uint32)(&Load$$I2S_TEST_DATA$$Base),
                (uint32)(&Image$$I2S_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$I2S_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$I2S_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$I2S_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$ACODEC_TEST_CODE$$Base),
                (uint32)(&Image$$ACODEC_TEST_CODE$$Base),
                (uint32)(&Image$$ACODEC_TEST_CODE$$Length),
                (uint32)(&Load$$ACODEC_TEST_DATA$$Base),
                (uint32)(&Image$$ACODEC_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$ACODEC_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$ACODEC_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$ACODEC_TEST_DATA$$ZI$$Length),
            },
            {
                (uint32)(&Load$$SYSTICK_TEST_CODE$$Base),
                (uint32)(&Image$$SYSTICK_TEST_CODE$$Base),
                (uint32)(&Image$$SYSTICK_TEST_CODE$$Length),
                (uint32)(&Load$$SYSTICK_TEST_DATA$$Base),
                (uint32)(&Image$$SYSTICK_TEST_DATA$$RW$$Base),
                (uint32)(&Image$$SYSTICK_TEST_DATA$$RW$$Length),
                (uint32)(&Image$$SYSTICK_TEST_DATA$$ZI$$Base),
                (uint32)(&Image$$SYSTICK_TEST_DATA$$ZI$$Length),
            },
        },
#endif
    },

    //ϵͳĬ�ϲ���
    {
        //ϵͳ����
#ifdef _SDCARD_
        (uint32)1,      //UINT32 SDEnable;
#else
        (uint32)0,
#endif

#ifdef _RADIO_
        (uint32)1,      //UINT32 FMEnable;
#else
        (uint32)1,
#endif

        (uint32)KEY_NUM,//UINT32 KeyNum


        (uint32)0,      //UINT32 Langule;
        (uint32)13,     //UINT32 Volume;

        (uint32)1,      //UINT32 BLMode;
        (uint32)2,      //UINT32 BLevel;
        (uint32)1,      //UINT32 BLtime;

        (uint32)0,      //UINT32 SetPowerOffTime;
        (uint32)0,      //UINT32 IdlePowerOffTime;

        (uint32)0,      //UINT32 AvlsEnabled;
        (uint32)0,      //UINT32 BeepEnabled;
        (uint32)1,      //UINT32 FactoryVolLimit;

        //Music����
        (uint32)0,      //UINT32 MusicRepMode;
        (uint32)0,      //UINT32 MusicPlayOrder;
        (uint32)0,      //UINT32 MusicBassBoost;
        (uint32)7,      //UINT32 MusicEqSel;
        (uint32)0,      //UINT32 MusicPlayFxSel;

        //Video����

        //Radio����
        (uint32)0,      //UINT32 FMArea;
        (uint32)0,      //UINT32 FMStereo;
        (uint32)0,      //UINT32 FMScanSensitivity;

        //Record����

        //Picture����
        (uint32)2,      //UINT32 PicAutoPlayTime;

        //Text����
        (uint32)2,      //UINT32 TextAutoPlayTime;

        //Image ID
        (uint32)0,

        //#ifdef _Vol_Tab_General
        //(uint32)1,      //UINT32 VolumeTableIndex;
        //#else
        //(uint32)0,
        //#endif            //VolumeTableIndex will not be reset when reset default setting - weslly

#ifdef _BLUETOOTH_
        //BT����
        "RKNANOD_Source",   //local bt device name.
#endif

        //�������ѡ��
        (uint32)TOTAL_MENU_ITEM,    //���˵���
        (uint32)TOTAL_LANAUAGE_NUM, //uint32 LanguageTotle, ʵ����������Ը���
        //�������ñ�                //����֧��64������
        {
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //8
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //16
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //24
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //32
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //40
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //48
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //56
            (uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1,(uint32)1, //64
        },
    },

    //FM ���������������б�
    {
        (uint32)1,      //FMDriverID;
        (uint32)FM_DRIVER_MAX,

        //FM Driver List
#ifdef _RADIO_
        {
            //FM5767
            {
                Qn8035_Tuner_SetInitArea,
                Qn8035_Tuner_SetFrequency,
                Qn8035_Tuner_SetStereo,
                Qn8035_Tuner_Vol_Set,

                Qn8035_Tuner_PowerOffDeinit,
                Qn8035_Tuner_SearchByHand,
                Qn8035_Tuner_PowerDown,
                Qn8035_Tuner_MuteControl,
                Qn8035_GetStereoStatus,
            },

            //FM5807
            {
                FM5807_Tuner_SetInitArea,
                FM5807_Tuner_SetFrequency,
                FM5807_Tuner_SetStereo,
                FM5807_Tuner_Vol_Set,

                FM5807_Tuner_PowerOffDeinit,
                FM5807_Tuner_SearchByHand,
                FM5807_Tuner_PowerDown,
                FM5807_Tuner_MuteControl,
                FM5807_GetStereoStatus,

            },
        },
#else
        {
            //FM5767
            {
                0,
                0,
                0,
                0,

                0,
                0,
                0,
                0,
                0,
            },

            //FM5807
            {
                0,
                0,
                0,
                0,

                0,
                0,
                0,
                0,
                0,

            },
        },
#endif
    },

    //LCD ���������������б�
    {
        (uint32)1,      //LcdDriverID;

        (uint32)LCD_DRIVER_MAX,

        //LCD Driver List
        {
            {
                SH1108G_WriteRAM_Prepare,
                SH1108G_Init,
                SH1108G_SendData,
                SH1108G_SetWindow,
                0,
                SH1108G_DMATranfer,
                SH1108G_Standby,
                SH1108G_WakeUp,
                0,
                0,
                SH1108G_SetPixel,
                SH1108G_Buffer_Display1,
                SH1108G_ClrSrc,
                SH1108G_ClrRect,
                SH1108G_DEV_FillRect,
            },

            {
                ST7735S_WriteRAM_Prepare,
                ST7735S_Init,
                ST7735S_SendData,
                ST7735S_SetWindow,
                ST7735S_Clear,
                ST7735S_DMATranfer,
                ST7735S_Standby,
                ST7735S_WakeUp,
                ST7735S_MP4_Init,
                ST7735S_MP4_DeInit,
                0,
                0,
                ST7735S_ClrSrc,
                ST7735S_ClrRect,
                0,
            },
        },
    }
};

/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : ģ����Ϣ��

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
__attribute__((section("START_CODE")))
void ScatterLoader(void)
{
    uint32 i,len;
    uint8  *pDest;

    //���Bss��
    pDest = (uint8*)((uint32)(&Image$$SYS_DATA$$ZI$$Base));
    len = (uint32)((uint32)(&Image$$SYS_DATA$$ZI$$Length));
    for (i = 0; i < len; i++)
    {
        *pDest++ = 0;
    }
}
/*
********************************************************************************
*
*                         End of StartLoadTab.c
*
********************************************************************************
*/
