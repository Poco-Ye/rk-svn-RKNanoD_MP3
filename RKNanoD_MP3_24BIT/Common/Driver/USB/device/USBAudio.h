/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USBAudio.h
Desc:

Author:
Date: 12-06-20
Notes:

$Log: $
 *
 *
*/

#ifndef _USBAUDIO_H
#define _USBAUDIO_H

/*-------------------------------- Includes ----------------------------------*/
#include "SysConfig.h"
#include "typedef.h"
#include "audio_globals.h"

/*------------------------------ Global Defines ------------------------------*/
#define USBAUDIO_BUFSIZE        (1024 * 12*3) // 30 kBytes
#define AUDIO_DMACHANNEL_IIS    (DMA_CHN_MAX - 1)

#define USBAUDIO_STOP_NORMAL       0
#define USBAUDIO_STOP_FORCE        1

#define FILE_MODE_LOCAL_MUSIC   (0)
#define FILE_MODE_STREAM_MEDIA  (1)

//   Usb Audio Interface Class Code
#define     AUDIO_CTRL_EP     0x00
#define     AS_OUT_EP         0x02
#define     HID_IN_EP         0x03

//define    USB Audio Interface Protocol Codes
#define     USB_AUDIO_CODE_INTERFACE_PROTOCOL_UNDEFINED             0x00

#define     USB_CLASS_CODE_AUDIO_INTERFACE                          0x01

//     USB AudioSubClass
#define     USB_SUBCLASS_CODE_INTERFACE_UNDEFINED                   0x00
#define     USB_SUBCLASS_CODE_AUDIO_CONTROL                         0x01
#define     USB_SUBCLASS_CODE_AUDIO_STREAMING                       0x02
#define     USB_SUBCLASS_CODE_AUDIO_MIDISTREAMING                   0x03

// USB Audio Class specific Descriptor type
#define     USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_UNDEFINED      0x20
#define     USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_DEVICE         0x21
#define     USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_CONFIGURATION  0x22
#define     USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_STRING         0x23
#define     USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_INTERFACE      0x24
#define     USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_ENDPOINT       0x25

// USB Audio Class-Specific AC(Audio Control) Interface Descriptor Subtype
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_UNDEFINED              0x00
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_HEADER                 0x01
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_INPUT_TERMINAL         0x02
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_OUTPUT_TERMINAL        0x03
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_MIXER_UNIT             0x04
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_SELECTOR_UNIT          0x05
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_FEATURE_UNIT           0x06
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_PROCESSING_UNIT        0x07
#define     USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_EXTENSION_UNIT         0x08

// USB Audio Class-Specific AS(Audio Stream) Interface Descripor Subtype
#define     USB_AUDIO_CLASS_CS_AS_ID_SUBTYPE_UNDEFINED              0x00
#define     USB_AUDIO_CLASS_CS_AS_ID_SUBTYPE_AS_GENERAL             0x01
#define     USB_AUDIO_CLASS_CS_AS_ID_SUBTYPE_FORMAT_TYPE            0x02
#define     USB_AUDIO_CLASS_CS_AS_ID_SUBTYPE_FORMAT_SPECIFIC        0x03

//USB Audio Class Format Type Codes
#define     USB_AC_FORMAT_TYPE_UNDEFINED                            0x00
#define     USB_AC_FORMAT_TYPE_I                                    0x01
#define     USB_AC_FORMAT_TYPE_II                                   0x02
#define     USB_AC_FORMAT_TYPE_III                                  0x03

//USB Audio State
#define     USB_AUDIO_ENUM_UNFINISH                                 0x00
#define     USB_AUDIO_ENUM_COMPLETED                                0x01

/*------------------------------ Global Typedefs -----------------------------*/
typedef enum _USB_AUDIO_CMD
{
    AUDIO_CMD_VOLUME_UP,
    AUDIO_CMD_VOLUME_DOWN,
    AUDIO_CMD_VOLUME_MUTE,
    AUDIO_CMD_STOP,
    AUDIO_CMD_SCAN_NEXT_TRACK,
    AUDIO_CMD_SCAN_PREV_TRACK,
    AUDIO_CMD_VOLUME_CD,
    AUDIO_CMD_FAST_FORWARD,
    AUDIO_CMD_NONE,

}USB_AUDIO_CMD;

//define the play status.
typedef enum
{
    USBAUDIO_STATE_PLAY,
    USBAUDIO_STATE_FFD,
    USBAUDIO_STATE_FFW,
    USBAUDIO_STATE_PAUSE,
    USBAUDIO_STATE_STOP,

    //PAGE
    USBAUDIO_STATE_PLAY_FFD,
    USBAUDIO_STATE_PLAY_FFW,
    USBAUDIO_STATE_PAUSE_FFD,
    USBAUDIO_STATE_PAUSE_FFW,
    USBAUDIO_STATE_FFD_PLAY,
    USBAUDIO_STATE_FFW_PLAY

}USBAUDIOSTATE;

//reapeat mode
typedef enum
{
    USBAUDIO_FOLDER_ONCE,    //once directory.
    USBAUIDO_FOLDER_REPEAT,  //directory cycle
    USBAUDIO_REPEAT,         //repeat one song
    USBAUDIO_ALLONCE,        //repeat once all song.
    USBAUDIO_ALLREPEAT,      //cycle play all song.
    USBAUDIO_REPEAT1,
    USBAUDIO_ONCE,           //repeat once one song.
    USBAUDIO_TRY              //lyrics

}USBAUDIOREPEATMODE;

//define A-B status.
typedef enum
{
    USBAUDIO_AB_NULL,
    USBAUDIO_AB_A,
    USBAUDIO_AB_PLAY

}USBAUDIOABSTATE;

//EQ mode define.
//need be keep consisten with modes define in file effect.h.
typedef enum
{
    USBAUDIO_NORMAL,
    USBAUDIO_MSEQ,
    USBAUDIO_ROCK,
    USBAUDIO_POP,
    USBAUDIO_CLASSIC,
    USBAUDIO_BASS,
    USBAUDIO_JAZZ,
    USBAUDIO_USER

}USBAUDIOEQTYPE;

typedef enum
{
    USB_AUDIO_PALY_STATE_STOP,
    USB_AUDIO_PALY_STATE_PLAY,
    USB_AUDIO_PALY_STATE_PAUSE

}USB_AUDIOPALYSTATE;

// Class Specific Audio Control Interface Descriptor
//header
typedef __packed struct _USB_CS_AC_INTERFACE_DESCRIPTOR_HEADER
{
    uint8        bLength;               //9bytes
    uint8        bDescriptorType;       //CS_INTERFACE Descriptor Type;
    uint8        bDescriptorSubType;    //HEADER
    uint16       bcdADC;
    uint16       wTotalLength;
    uint8        bInCollection;
    uint8        baInterfaceNr0;
} USB_CS_AC_INTERFACE_DESCRIPTOR_HEADER,*PUSB_CS_AC_INTERFACE_DESCRIPTOR_HEADER;

//input terminal
typedef __packed struct _USB_CS_AC_INTERFACE_DESCRIPTOR_INPUT_TERMINAL
{
    uint8        bLength;               //17bytes
    uint8        bDescriptorType;       //CS_INTERFACE Descriptor Type;
    uint8        bDescriptorSubType;    //Input Terminal
    uint8        bTerminalID;           //Constant uniquely identifying the Terminal within the audio function. The value is used in all  requests to address this Terminal
    uint16       wTerminalType;         //Constant characterizing the type of Terminal. See Usb Audio Terminal Types.
    uint8        bAssocTerminal;        //ID of the Output Terminal to which this Input Terminal is associated;
    uint8        bNrChannels;           //Number of logical output channels in the Terminal's output audio channel cluster.
    uint16       bmChannelConfig;       //Bitmap Describes the spatial location of the logical channels;
    uint8        iChannelNames;        //Index of a string descriptor;
    uint8        iTerminal;            //Index of a string descriptor;
} USB_CS_AC_INTERFACE_DESCRIPTOR_INPUT_TERMINAL,*PUSB_CS_AC_INTERFACE_DESCRIPTOR_INPUT_TERMINAL;

//output  terminal
typedef __packed struct _USB_CS_AC_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL
{
    uint8        bLength;               //12bytes
    uint8        bDescriptorType;       //CS_INTERFACE Descriptor Type;
    uint8        bDescriptorSubType;    //Output Terminal
    uint8        bTerminalID;           //Constant uniquely identifying the Terminal within the audio function. The value is used in all  requests to address this Terminal
    uint16       wTerminalType;         //Constant characterizing the type of Terminal. See Usb Audio Terminal Types.
    uint8        bAssocTerminal;        //ID of the Output Terminal to which this Input Terminal is associated;
    uint8        bSourceID;             //ID of the Unit or Terminal to which this Terminal is connected;
    uint8        iTerminal;             //Index of a string descriptor;
} USB_CS_AC_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL,*PUSB_CS_AC_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL;

//feature unit
typedef __packed struct _USB_CS_AC_INTERFACE_DESCRIPTOR_FEATURE_UNIT
{
    uint8       bLength;                //7+(ch+1)*n=10
    uint8       bDescriptorType;        //CS_INTERFACE Descriptor Type;
    uint8       bDescriptorSubType;     //feature_unit
    uint8       bUnitID;                //Constant uniquely identifying the Unit within the audio function. This value is used in all request to address this unit;
    uint8       bSourceID;              //ID of the Unit or Terminal to which this Feature unit is connected.
    uint8       bControlSize;
    uint8       bmaControls0;           //Bitmap The Controls bitmap for master channel 0;
    uint8       bmaControls1;           //Bitmap The Controls bitmap for master channel 0;
    uint8       bmaControls2;           //Bitmap The Controls bitmap for master channel 0;
    uint8       iFeature;
    //n channels
} USB_CS_AC_INTERFACE_DESCRIPTOR_FEATURE_UNIT,*PUSB_CS_AC_INTERFACE_DESCRIPTOR_FEATURE_UNIT;

// Class Specific General Audio Stream Interface Descriptor
typedef __packed struct _USB_CS_AS_INTERFACE_GENERAL_DESCRIPTOR
{
    uint8       bLength;                //16bytes
    uint8       bDescriptorType;        //CS_INTERFACE Descriptor Type;
    uint8       bDescriptorSubtype;     //AS_GENERAL descriptor subtype;
    uint8       bTerminalLink;          //The Terminal ID of the Terminal to which this interface is connected
    uint8       bDelay;                 //Delay
    uint16      wFormatTag;
 }USB_CS_AS_INTERFACE_GENERAL_DESCRIPTOR,*PUSB_CS_AS_INTERFACE_GENERAL_DESCRIPTOR;

typedef __packed struct _USB_CS_AS_INTERFACE_FORMAT_DESCRIPTOR
{
    uint8      bLength;                 //6bytes
    uint8      bDescriptorType;         //CS_INTERFACE
    uint8      bDescriptorSubType;      //FORMAT_TYPE
    uint8      bFormatType;             //FORMAT_TYPE_I
    uint8      bNrChannels;             //The number of bytes occupied by one audio subslot,can be 1,2,3,4
    uint8      bSubframeSize;           //The number of effctively used bits from the available bits in an audio subslot
    uint8      bBitResolution;
    uint8      bSamFreqType;
    uint8      tSamFreq[3];
}USB_CS_AS_INTERFACE_FORMAT_DESCRIPTOR,*PUSB_CS_AS_INTERFACE_FORMAT_DESCRIPTOR;

typedef __packed struct _USB_AS_ENDPOINT_DESCRIPTOR
{
   /// Size of the descriptor in bytes.
   uint8 bLength;
   /// Descriptor type (USBGenericDescriptor_ENDPOINT).
   uint8 bDescriptorType;
   /// Address and direction of the endpoint.
   uint8 bEndpointAddress;
   /// Endpoint type and additional characteristics (for isochronous endpoints).
   uint8 bmAttributes;
   /// Maximum packet size (in bytes) of the endpoint.
   uint16 wMaxPacketSize;
   /// Polling rate of the endpoint.
   uint8 bInterval;
   /// Refresh rate for a feedback endpoint.
   uint8 bRefresh;
   /// Address of the associated feedback endpoint if any.
   uint8 bSyncAddress;
} USB_AS_ENDPOINT_DESCRIPTOR, *PUSB_AS_ENDPOINT_DESCRIPTOR;

typedef __packed struct _USB_AS_DATA_ENDPOINT_DESCRIPTOR
{
    /// Size of descriptor in bytes.
    uint8 bLength;
    /// Descriptor type (AUDGenericDescriptor_ENDPOINT).
    uint8 bDescriptorType;
    ///  Descriptor subtype  (AUDDataEndpointDescriptor_SUBTYPE).
    uint8 bDescriptorSubType;
    /// Indicates available controls and requirement on packet sizes.
    uint8 bmAttributes;
    /// Indicates the units of the wLockDelay fields.
    /// \sa "USB Audio Lock delay units"
    uint8 bLockDelayUnits;
    /// Time it takes for the endpoint to lock its internal clock circuitry.
    uint16 wLockDelay;
}
USB_AS_DATA_ENDPOINT_DESCRIPTOR, *PUSB_AS_DATA_ENDPOINT_DESCRIPTOR;

typedef __packed struct _USB_HID_CLASS_DESCRIPTOR
{
    uint8 bLength;
    uint8 bDescriptorType;
    uint16 wBCDHID;
    uint8 bCountryCode;
    uint8 bNumDescriptors;
    uint8 bDescriptorClassType;
    uint16 wDescriptorLength;
}USB_HID_CLASS_DESCRIPTOR, * PUSB_HID_CLASS_DESCRIPTOR;

typedef __packed struct _USB_HID_REPORT_DESCRIPTOR
{
    uint16 wUsagePage;
    uint16 wUsage;
    uint16 wCollection;
    uint16 wLogicalMinimum;
    uint16 wLogicalMaxmum;
    uint16 wUsage1;
    uint16 wUsage2;
    uint16 wUsage3;
    uint16 wUsage4;
    uint16 wUsage5;
    uint16 wUsage6;
    uint16 wUsage7;
    uint16 wUsage8;
    uint16 wReportSize;
    uint16 wReportCnt;
    uint16 wInput;
    uint8 bEndCollect;

}USB_HID_REPORT_DESCRIPTOR, * P_USB_HID_REPORT_DESCRIPTOR;

//≈‰÷√√Ë ˆ∑˚ºØ∫œ√Ë ˆ∑˚Ω·ππ
typedef __packed struct _USB_AUDIO_CONFIGS_DESCRIPTOR
{
    USB_CONFIGURATION_DESCRIPTOR                    config;

    USB_INTERFACE_DESCRIPTOR                        control;//Audio Control

    USB_CS_AC_INTERFACE_DESCRIPTOR_HEADER           header;
    USB_CS_AC_INTERFACE_DESCRIPTOR_INPUT_TERMINAL   InputTerminal;
    USB_CS_AC_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL  OutputTerminal;
    USB_CS_AC_INTERFACE_DESCRIPTOR_FEATURE_UNIT     FeatureUnit;

    USB_INTERFACE_DESCRIPTOR                        StreamingOutNoIso;//Audio Stream
    USB_INTERFACE_DESCRIPTOR                        StreamingOut;//Audio Stream
    USB_CS_AS_INTERFACE_GENERAL_DESCRIPTOR          ASOutClass;
    USB_CS_AS_INTERFACE_FORMAT_DESCRIPTOR           ASFormatType;
    USB_AS_ENDPOINT_DESCRIPTOR                      ASOutEndp;
    USB_AS_DATA_ENDPOINT_DESCRIPTOR                 ASOutDataEndp;

    USB_INTERFACE_DESCRIPTOR                        HIDInterFace;
    USB_HID_CLASS_DESCRIPTOR                        HIDClassDescriptor;
    USB_ENDPOINT_DESCRIPTOR                         HIDInEndp;
    USB_ENDPOINT_DESCRIPTOR                         HIDOutEndp;
}USB_AUDIO_CONFIGS_DESCRIPTOR;

typedef struct
{
    short  *pPCMBuf;
    short  *EncOutBuf;
    long   flag;
    RKEffect EffectCtl;
}UsbAudioInOut_Type;

typedef struct
{
    uint32 ABRequire;
    uint32 AudioABStart;
    uint32 AudioABEnd;
    uint32 PlayDirect;
    uint8 playVolume;
    uint8 VolumeCnt;

}USBAUDIOCONTROL;

/*
*-------------------------------------------------------------------------------
*
*                           Function de
*
*-------------------------------------------------------------------------------
*/
BOOLEAN UsbAudioPause(void);
BOOLEAN UsbAudioResume(void);
extern int32 USBAudioInit(void *arg);
extern void  USBAudioDeInit(void);
extern uint8 USBAudioEnumCheck(void);
extern void  USBAudioOperCmd(uint8 bCmd) ;
extern uint8 USBAuidoTypeCheck(void);

//----- PROTOTYPES -----
void         usbAudio_buf_puts(const unsigned char *s, unsigned int len);
unsigned int usbAudio_buf_free_size(void);
unsigned int usbAudio_buf_len(void);
void         usbAudio_buf_reset(void);

int32        UsbAudioMsgProcess(void);

unsigned short RKUSB_FRead(unsigned char *b, unsigned short s, void *f);

/*
--------------------------------------------------------------------------------

  Description:  audio thread definition.

--------------------------------------------------------------------------------
*/

#endif