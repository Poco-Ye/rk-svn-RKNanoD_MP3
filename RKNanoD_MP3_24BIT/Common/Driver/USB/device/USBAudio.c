/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USBAudio.c
Desc:

Author:
Date: 13-07-26
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/
#include "SysInclude.h"
#include "USBConfig.h"

#ifdef USB_AUDIO

#include <stdio.h>
#include <string.h>
#include "audio_file_access.h"
#include "HoldonPlay.h"
#include "Dma.h"
#include "I2S.h"

/*------------------------------------ Defines -------------------------------*/

/// Playback input terminal ID.
#define USB_AUDIO_INPUT_TERMINAL      1
/// Playback output terminal ID.
#define USB_AUDIO_OUTPUT_TERMINAL     2
/// Playback feature unit ID.
#define USB_AUDIO_FEATURE_UNIT        3


#define USB_AUDIO_EPOUT_NUM           2
#define USB_HID_EPIN_NUM              3
//#define USB_HID_EPOUT_NUM             2

#define MAX_USB_AUDIO_CUR             0x8080
#define MAX_USB_AUDIO_VOL             0x80ff //根据音量表进行映射调整
#define MIN_USB_AUDIO_VOL             0x8000
#define MAX_USB_AUDIO_RES             0x0001

/* Audio Class-Specific Request Codes */
#define UAC_RC_UNDEFINED				0x00
#define UAC_SET_CUR					    0x01
#define UAC_GET_INT                     0x0A
#define UAC_GET_CUR					    0x81
#define UAC_GET_MIN						0x82
#define UAC_GET_MAX					    0x83
#define UAC_GET_RES				        0x84

#define USB_AUDIO_OUT_BUF_SIZE 512

/*----------------------------------- Typedefs -------------------------------*/
typedef volatile struct tagUAS_DEVICE
{
    uint8  LastVolume;
    uint8  ConSel;
    uint8  connected;
    uint8  AlterInterface;
    USB_DEVICE   *pDev;

} UAS_DEVICE, *pUAS_DEVICE;

typedef enum _CONTROL_STATUS
{
    AUDIO_NO_CONTROL,
    AUDIO_VOLUME_SET,
    AUDIO_MUTE_STATUS
}
CONTROL_STATUS;

typedef union
{
  uint8   b8[USBAUDIO_BUFSIZE];
  uint16 b16[USBAUDIO_BUFSIZE/2];
  uint32 b32[USBAUDIO_BUFSIZE/4];
}USBBUFFER;

/*-------------------------- Forward Declarations ----------------------------*/

static int32 USBAudioSetup(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev);
static void  USBAudioReqest(uint8 epnum, uint32 event, uint32 param, USB_DEVICE *pDev);
static void  USBAudioResume(USB_DEVICE *pDev);
static void  USBAudioSuspend(USB_DEVICE *pDev);
static void  USBAudioDisconnect(USB_DEVICE *pDev);

static void   UsbMusicInit(void);
static UINT32 UsbMusicService(void);
static void   UsbMusicDeInit(void);


/*-------------------------------- Local Statics: ----------------------------*/
_ATTR_USB_AUDIO_DATA_
static USB_AUDIO_CONFIGS_DESCRIPTOR HSAudioConfigs =
{
    {
    sizeof(USB_CONFIGURATION_DESCRIPTOR),                   // size of the descriptor 9
    USB_DT_CONFIG,                                          // type of the configuration descriptor02(1B)
    sizeof(USB_AUDIO_CONFIGS_DESCRIPTOR),                   //  wTotalLength  110 bytes
    0x03,                                                   // bNumInterfaces
    0x1,                                                    // bConfigurationValue
    0x0,                                                    // iConfiguration
    0xC0,                                                   // bmAttributes  BUS Powred
    0x32                                                    // bMaxPower = 100 mA
    },

    //Audio Control  interface descriptor
    {
    sizeof(USB_INTERFACE_DESCRIPTOR),                               //size of  usb interface
    USB_DT_INTERFACE,                                               //type of USB standard descriptor
    0x00,                                                           //interface number
    0x00,                                                           //bAlternateSetting
    0x00,                                                           //bNumEndpoints
    USB_DEVICE_CLASS_AUDIO,                                         //bInterfaceClass
    USB_SUBCLASS_CODE_AUDIO_CONTROL,                                //bInterfaceSubClass
    USB_AUDIO_CODE_INTERFACE_PROTOCOL_UNDEFINED,                    //bInterfaceProtocol
    0x00                                                            //iInterface
    },
    //String Descriptor Index

    //Audio Control specific header interface descriptor
    {
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_HEADER),                  // size of USB audio class specified interface header descriptor
    USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_INTERFACE,             // type of USB audio class specialied descriptor
    USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_HEADER,                        // subtype of usb audio class  control
    0x0100,                                                         // bcdADC
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_HEADER)+                  // 9 size of control descriptor
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_INPUT_TERMINAL)+          // 12
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_FEATURE_UNIT)+            // 10
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL),         // 9                                                    //wTotalLength
    0x01,                                                           //Number of Streaming interfaces
    0x01                                                            //bmControls the interface belong to this control
    },

    //Audio Control Specific input terminal interface descriptor
    {
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_INPUT_TERMINAL),          // 1 size of USB audio class specified interface input terminal descriptor
    USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_INTERFACE,             // 1 type of USB audio class specified   descriptor
    USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_INPUT_TERMINAL,                // 1 subtype of usb audio class control
    USB_AUDIO_INPUT_TERMINAL,                                       //1 bTerminalID
    0x0101,                                                         // 2 wTerminalType
    0x00,                                                           // 1 bAssocTerminal
    0x02,                                                           // 1 bNrChannels
    0x0003,                                                         // 2 bmChannelConfig
    0x00,                                                           // 1 iChannelNames
    0x00                                                            // 1 iTerminal
    },
    //iTerminal Index of a string descriptor

    //Audio Control Specific output terminal interface descriptor
    {
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL),         // 1 bLength
    USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_INTERFACE,             // 1 bDescriptorType
    USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_OUTPUT_TERMINAL,               // 1 bDescriptorSubType
    USB_AUDIO_OUTPUT_TERMINAL,                                      // 1 bTerminalID
    0x0301,                                                         // 2 wTerminalType
    0x00,                                                           // 1 bAssocTerminal
    USB_AUDIO_FEATURE_UNIT,                                         // 1 bSourceID
    0x00                                                            // 1 iTerminal
    },

    // Audio Control Feature Unit
    //Audio Control Specific feature terminal interface descriptor
    {
    sizeof(USB_CS_AC_INTERFACE_DESCRIPTOR_FEATURE_UNIT),            // 1 bLength
    USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_INTERFACE,             // 1 bDescriptorType
    USB_AUDIO_CLASS_CS_AC_ID_SUBTYPE_FEATURE_UNIT,                  // 1 bDescriptorSubType
    USB_AUDIO_FEATURE_UNIT,                                         // 1 bUnitID
    USB_AUDIO_INPUT_TERMINAL,                                       // bSourceId
    0x01,                                                           // 1 bControlSize
    0x03,                                                           // 2 bmaControls0
    0X00,                                                           // bmaControls1
    0X00,                                                           // bmaControls2
    0x00                                                            // 1 iFeature
    },

    //Audio streaming interface with 0 endpoints
    //Interface 1, Alternate Setting 0
    {
    sizeof(USB_INTERFACE_DESCRIPTOR),                               // size of  usb interface
    USB_DT_INTERFACE,                                               // type of USB standard descriptor
    0x01,                                                           // number of endpoint
    0x00,                                                           // replace of up
    0x00,                                                           // endpoint it used
    USB_DEVICE_CLASS_AUDIO,                                         // Class Type ,USB_DEVICE_CLASS_STORAGE=Mass Storage
    USB_SUBCLASS_CODE_AUDIO_STREAMING,                              // Class Sub Type(1B),"0x06=Reduced Block Commands(RBC)"
    USB_AUDIO_CODE_INTERFACE_PROTOCOL_UNDEFINED,                    // USB protocol type(1B),"0X50=Mass Storage Class Bulk-Only Transport"
    0x00                                                            // String Descriptor Index
    },

    //Audio streaming interface with data endpoint
    //Interface 1, Alternate Setting 1
    {
    sizeof(USB_INTERFACE_DESCRIPTOR),                               // size of  usb interface
    USB_DT_INTERFACE,                                               // type of USB standard descriptor
    0x01,                                                           // number of endpoint
    0x01,                                                           // replace of up
    0x01,                                                           // endpoint it used
    USB_DEVICE_CLASS_AUDIO,                                         // Class Type ,USB_DEVICE_CLASS_STORAGE=Mass Storage
    USB_SUBCLASS_CODE_AUDIO_STREAMING,                              // Class Sub Type(1B),"0x06=Reduced Block Commands(RBC)"
    USB_AUDIO_CODE_INTERFACE_PROTOCOL_UNDEFINED,                    // USB protocol type(1B),"0X50=Mass Storage Class Bulk-Only Transport"
    0x00                                                            //String Descriptor Index
    },

    // Audio Stream General
    //Audio Stream Specific general  interface descriptor
    {
    sizeof(USB_CS_AS_INTERFACE_GENERAL_DESCRIPTOR),                 // 1 bLength
    USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_INTERFACE,             // 1 bDescriptorType
    USB_AUDIO_CLASS_CS_AS_ID_SUBTYPE_AS_GENERAL,                    // 1 bDescriptorSubtype
    USB_AUDIO_INPUT_TERMINAL,                                       // bTerminalLink
    0x01,//00                                                       // No internal delay because of data path
    0x0001                                                          // wFormatTag PCM format
    },

    //Audio Stream Specific format  interface descriptor
    {
    sizeof(USB_CS_AS_INTERFACE_FORMAT_DESCRIPTOR),                  // 1 bLength
    USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_INTERFACE,             // 1 bDescriptorType
    USB_AUDIO_CLASS_CS_AS_ID_SUBTYPE_FORMAT_TYPE,                   // 1 bDescriptorSubType
    USB_AC_FORMAT_TYPE_I,                                           // 1 bFormatType
    0x02,                                                           // 1 bNrChannels
    0x02,                                                           // 1 bSubframeSize
    0x10,                                                           // 1 bBitResolution
    0x01,                                                           // 1 bSamFreqType
    {0x80,0xBB,0x00}                                                // 3 tSamFreq
    },

    // Audio streaming endpoint standard descriptor
    {
    sizeof(USB_AS_ENDPOINT_DESCRIPTOR),                             // 9bytes
    USB_DT_ENDPOINT,                                                // usb endpoint descriptor
    AS_OUT_EP,                                                      // endpoint number
    (USB_EPTYPE_ISOC | 0x08),                                       // type isochronous |0x08 for SynchronisationType
    //HS_BULK_TX_SIZE,                                              // HS_isochronous_RX_SIZE,
    192,
    1,                                                              // Polling interval = 2^(x-1) milliseconds (1 ms)
    0,                                                              // This is not a synchronization endpoint
    0                                                               // No associated synchronization endpoint                                                         // bulk trans invailed
    },

    //Audio Stream Specific   endpoint descriptor
    {
    sizeof(USB_AS_DATA_ENDPOINT_DESCRIPTOR),                                                               // bLength
    USB_AUDIO_CLASS_SPECIFIC_DESCRIPTOR_TYPE_ENDPOINT,              // bDescriptorType
    0x01,                                                           // Descriptor subtype for an Audio data endpoint.
    0x01,//0                                                           // bmAttributes
    0x01,//0                                                           // bLockDelayUnits
    0x0001,                                                            // wLockDelay

    },

    //HID Interface
    {
    sizeof(USB_INTERFACE_DESCRIPTOR),                               // blength
    USB_DT_INTERFACE,                                               // interface type
    0x02,                                                           // bInterfaceNumber: Number of Interface
    0x00,                                                           // bAlternateSetting: Alternate setting
    0x01,                                                           // bNumEndpoints
    0x03,                                                           // bInterfaceClass: HID
    0x00,                                                           // unkown  sub class
    0x00,                                                           // interface protol
    0x00                                                            // index
    },

    //HID Class Descriptor
    {
    sizeof(USB_HID_CLASS_DESCRIPTOR),                               // blength
    0x21,                                                           // bDescriptorType: HID
    0x0201,                                                         // bcdHid 1.2
    0x00,                                                           // bCountryCode: Hardware target country
    0x01,                                                           // bNumDescriptors: Number of HID class descriptors to follow
    0x22,                                                           // report descriptor type
    0x0021                                                          // report descriptor length
    },

    //HID In Endpoint
    {
    sizeof(USB_ENDPOINT_DESCRIPTOR),                                // blength
    USB_DT_ENDPOINT,                                                // endpoint type
    (HID_IN_EP|0x80),                                        // bEndpointAddress: Endpoint Address (IN)
    USB_EPTYPE_INTR,                                                // bmAttributes: Interrupt endpoint
    0x0002,                                                         // wMaxPacketSize: 2 Bytes max
    0x40,                                                           // bInterval: Polling Interval (64 ms)
    },
    //#if 1
    //HID Out Endpoint
    {
    sizeof(USB_ENDPOINT_DESCRIPTOR),                                // blength
    USB_DT_ENDPOINT,                                                // endpoint type
    HID_IN_EP,                                              // bEndpointAddress: Endpoint Address (OUT)
    USB_EPTYPE_INTR,                                                // bmAttributes: Interrupt endpoint
    0x0002,                                                         // wMaxPacketSize: 2 Bytes max
    0x40,                                                            // bInterval: Polling Interval (64 ms)
    }
   // #endif
};

_ATTR_USB_AUDIO_DATA_
USB_HID_REPORT_DESCRIPTOR USBHidReport =
{
    0x0c05,     //usage page(Consumer)
    0x0109,     //usage(consumer control)
    0x01a1,     //collection
    0x0015,     //logical minimum(0)
    0x0125,     //logical maxmum(1)
    0xe909,     //usage(volume up)
    0xea09,     //usage(volume down)
    0xe209,     //usage(volume mute)
    0xcd09,     //usage()
    0xb509,     //scan (next track)
    0xb609,     //scan (prev track)
    0xb309,     //fast forward
    0xb709,     //usage (stop)
    0x0175,     //report size(1)
    0x0895,     //report cnt(8)
    0x4281,     //input
    0xc0,       //end collection
};

_ATTR_USB_AUDIO_DATA_
USB_DEVICE_DESCRIPTOR HSAudioDeviceDescr =
{
    sizeof(USB_DEVICE_DESCRIPTOR),              //descriptor's size 18(1B)
    USB_DT_DEVICE,                              //descriptor's type 01(1B)
    0x0200,                                      //USB plan distorbution number (2B)
    0,                                          //1type code (point by USB)(1B),0x00
    0, 0,                                       //child type and protocal (usb alloc)(2B)
    EP0_PACKET_SIZE,                            //endpoint 0 max package length(1B)
    0x071b,
    0x3205,
    0x0200,								        // device serial number
    USB_STRING_MANUFACTURER,
    USB_STRING_PRODUCT,
    USB_STRING_SERIAL,                          //producter,produce,device serial number index(3B)
    1                                           //feasible configuration parameter(1B)
};

_ATTR_USB_AUDIO_BSS_
static UAS_DEVICE USBAudioDev;

_ATTR_USB_AUDIO_DATA_
static USB_DEVICE USBAudioDriver =
{
    USB_SPEED_FULL,
    USB_CLASS_TYPE_AUDIO,
    sizeof(USB_DEVICE_DESCRIPTOR),
    &HSAudioDeviceDescr,
    sizeof(USB_AUDIO_CONFIGS_DESCRIPTOR),
    &HSAudioConfigs,
    &HSAudioConfigs,
    USBAudioSetup,
    USBAudioSuspend,
    USBAudioResume,
    USBAudioDisconnect,
    USBAudioReqest,
    &USBAudioDev,
    NULL,
    NULL
};

/* ------------------------------- Globals ---------------------------------- */
_ATTR_USB_AUDIO_BSS_ USBAUDIOCONTROL UsbAudioPlayInfo;
_ATTR_USB_AUDIO_BSS_ uint32 usbaudioconfig;
_ATTR_USB_AUDIO_DATA_ int32 usbaudiosetvolume = 0;



/*-------------------------------- Local Statics: ----------------------------*/
_ATTR_USB_AUDIO_BSS_  USBBUFFER vs_buf;
_ATTR_USB_AUDIO_BSS_  unsigned int vs_bufhead, vs_buftail;
_ATTR_USB_AUDIO_BSS_  unsigned long vs_offset;
_ATTR_USB_AUDIO_BSS_  uint8 Usb_AudioPlayStatus;


_ATTR_USB_AUDIO_BSS_ track_info pUsbAudioRegKey;
_ATTR_USB_AUDIO_BSS_ UINT16  UsbAudioPlayState;
_ATTR_USB_AUDIO_BSS_ UINT16  UsbAudioPlayerState;
_ATTR_USB_AUDIO_BSS_ uint32  UsbAudioPtr;
_ATTR_USB_AUDIO_BSS_ uint32  UsbAudioLen;
_ATTR_USB_AUDIO_BSS_ uint32  UsbAudioDecodeing;
_ATTR_USB_AUDIO_BSS_ uint32  UsbDmaTransting;
_ATTR_USB_AUDIO_BSS_ int     UsbMusicStatus;


_ATTR_USB_AUDIO_DATA_ DMA_CFGX UsbAudioControlDmaCfg  =
{DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX,0};


_ATTR_USB_AUDIO_BSS_  uint32 g_BtBufIndex;
_ATTR_USB_AUDIO_BSS_  int32  g_outLength;
_ATTR_USB_AUDIO_BSS_  long   g_timePosCount;

_ATTR_USB_AUDIO_BSS_  int16   gUsbAudioOutputPtr [2][USB_AUDIO_OUT_BUF_SIZE * 2];

_ATTR_USB_AUDIO_BSS_ uint16 I2SbufferIndex;
_ATTR_USB_AUDIO_BSS_ uint16 USBbufferIndex;
_ATTR_USB_AUDIO_BSS_ uint32 DMAStarted;
_ATTR_USB_AUDIO_BSS_ uint32 *USBOutputBuf;
_ATTR_USB_AUDIO_BSS_ uint32 USBOutputLen;

_ATTR_USB_AUDIO_BSS_ uint8 UsbAudioControlStatus;
_ATTR_USB_AUDIO_BSS_ uint8 AddOdd ;
_ATTR_USB_AUDIO_BSS_ uint8 AudioEnumState;

_ATTR_USB_AUDIO_BSS_ uint32 UASBuffer[3*0x0f*192];


/*--------------------------- Local Function Prototypes ----------------------*/


/*------------------------ Function Implement --------------------------------*/

/*
Name:       USBAudioOutPkt
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioReInitOutEp(void)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if(AddOdd)
    {
        dev_regs->out_ep[AS_OUT_EP].doepctl &= ~(1ul<<29);
        dev_regs->out_ep[AS_OUT_EP].doepctl |= (1ul<<28);
        AddOdd = 0;
    }
    else
    {
        dev_regs->out_ep[AS_OUT_EP].doepctl |= (1ul<<29);
        dev_regs->out_ep[AS_OUT_EP].doepctl &= ~(1ul<<28);
        AddOdd = 1;
    }

    dev_regs->out_ep[AS_OUT_EP].doeptsiz |= (1ul<<19) | 192;
    dev_regs->out_ep[AS_OUT_EP].doepctl|= (1ul<<26);
}

/*
Name:       USBAudioOutPkt
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioOutPkt(pUAS_DEVICE pAud, uint16 len, uint8 SOF)
{
    DMA_CFGX DmaCfg = {DMA_CTLL_USB_RX, DMA_CFGL_USB_RX, DMA_CFGH_USB_RX,0};
	uint32 *buf = (uint32 *)USBOutputBuf;

    USBReadEp(AS_OUT_EP, len, (void *)USBOutputBuf);
    USBAudioReInitOutEp();
    usbAudio_buf_puts((void *)USBOutputBuf, len);
}

/*
Name:       USBAudioGetDesc
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static int32 USBAudioGetDesc(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    uint8 bTarget = (ctrl->bRequestType & 0x0F);
    int32 ret = -1;

    if (bTarget == USB_RECIP_INTERFACE)
    {
        uint8 type = (ctrl->wValue >> 8) & 0xff;
        uint8 IntfNum =  (ctrl->wIndex) & 0xff;

        if(IntfNum == 0x02) //HID Interface
        {
            if(type == 0x22) //report descriptor
            {
                uint32 len;
                USB_EP0_REQ *ep0req = USBGetEp0Req();

                len = MIN(sizeof(USB_HID_REPORT_DESCRIPTOR), ctrl->wLength);
                memcpy(ep0req->buf, (uint8*)&USBHidReport, len);
                ep0req->dir = 1;
                ep0req->NeedLen = len;
                USBStartEp0Xfer(ep0req);

                ret = 0;
            }
        }
    }
    return ret;
}

/*
Name:       USBAudioSetConfig
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioSetConfig(UAS_DEVICE *pAud, uint8 config)
{
    if (config)
    {
        USBEnableEp(AS_OUT_EP, (USB_ENDPOINT_DESCRIPTOR *)&HSAudioConfigs.ASOutEndp);
        USBEnableEp(HID_IN_EP|0x80, &HSAudioConfigs.HIDInEndp);
        pAud->connected = 1;
    }
    USBInEp0Ack();
}

/*
Name:       USBAudioStandardReq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static int32 USBAudioStandardReq(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    UAS_DEVICE *pAud = (UAS_DEVICE *)pDev->pFunDev;
    int32 ret = 0;

    switch (ctrl->bRequest)
    {
        case USB_REQ_GET_DESCRIPTOR:
            ret = USBAudioGetDesc(ctrl, pDev);
            break;

        /* One config, two speeds */
        case USB_REQ_SET_CONFIGURATION:
            USBAudioSetConfig(pAud, ctrl->wValue & 0x1);
            break;

         case USB_REQ_SET_INTERFACE:
            pAud->AlterInterface = ctrl->wValue & 0x1;
            USBInEp0Ack();
            break;


        case USB_REQ_GET_INTERFACE:
    	    {
                uint8 intf = pAud->AlterInterface;
                USBWriteEp0(1, &intf);
            }
            break;
        default:
            break;
    }

    return ret;
}

/*
Name:       USBAudioEp0Cmpl
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioEp0Cmpl(void *ptr)
{
    USB_DEVICE *pDev = (USB_DEVICE *)ptr;
    pUAS_DEVICE pAud = (pUAS_DEVICE)pDev->pFunDev;
    USB_EP0_REQ *ep0req = USBGetEp0Req();
    int32 value = 0;
    int32 ValueLow = 0;

    if (pAud->ConSel == 0x02)   //VOLUME_CONTROL
    {
        value = __get_unaligned_be16(ep0req->buf);
        ValueLow = (value & 0x00ff);
        value = ((value  >> 8) | (ValueLow <<8));              //high and low exchange
        //DEBUG("the value is %x",value);
        if(value ==  MIN_USB_AUDIO_VOL || value ==  0x8000)
        {
            pAud->LastVolume = 0;
            gSysConfig.OutputVolume = 0;
			usbaudiosetvolume = 1;

        }
        else if(value < MIN_USB_AUDIO_VOL || value > MAX_USB_AUDIO_VOL)
        {
            return;
        }
        else if(value ==  MAX_USB_AUDIO_VOL)
        {

            pAud->LastVolume = MAX_VOLUME;
            gSysConfig.OutputVolume = MAX_VOLUME;
            usbaudiosetvolume = 1;
        }
        else
        {
            value = ((value - MIN_USB_AUDIO_VOL)*1000 /(MAX_USB_AUDIO_VOL - MIN_USB_AUDIO_VOL)*MAX_VOLUME/1000) + 1;
            gSysConfig.OutputVolume = value;
            pAud->LastVolume = value;

			usbaudiosetvolume = 1;
        }

    }
    else if (pAud->ConSel == 0x01) //MUTE_CONTROL
    {
        value = ep0req->buf[0];
        if(value)//mute
        {
			if (usbaudioconfig == 1)
            	Codec_DACMute();
        }
	    else //unmute
	    {
			if (usbaudioconfig == 1)
	        	Codec_DACUnMute();
	    }
    }

}

/*
Name:       USBAudioClassReq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioClassReq(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    uint32 tmp = 0;
    pUAS_DEVICE pAud = (pUAS_DEVICE)pDev->pFunDev;
    USB_EP0_REQ *ep0req = USBGetEp0Req();

    switch (ctrl->bRequestType)
    {
        case (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE):
        {
            switch(ctrl->bRequest)
            {
                case UAC_SET_CUR:
                {
				    uint8 ConSel = (ctrl->wValue >> 8) & 0xFF;

                    if (ConSel == 0x02 || ConSel == 0x01)           //VOLUME_CONTROL || MUTE_CONTROL
                    {
                        pAud->ConSel = ConSel;
                        ep0req->dir = 0;
                        ep0req->complete = USBAudioEp0Cmpl;
                        ep0req->NeedLen = ctrl->wLength;
                        USBStartEp0Xfer(ep0req);
					}
                    break;
                }

                case UAC_GET_INT:
                {
                    USBInEp0Ack();
                    break;
                }

                default:
                    break;
            }
            break;
        }

        case (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE):
        {
            switch(ctrl->bRequest)
            {
                case UAC_GET_CUR:
                {
                    if(ctrl->wLength == 0x01)
                        tmp = 0x00;
                    else if(ctrl->wLength == 0x02)
                        tmp = MAX_USB_AUDIO_CUR;                //当前值，留下根据系统的音量设置
                    USBWriteEp0(ctrl->wLength, &tmp);
                    break;
                }
                case UAC_GET_RES:                               //分辨率属性
                {
                    tmp = MAX_USB_AUDIO_RES;
                    USBWriteEp0(2, &tmp);
                    break;
                }
                case UAC_GET_MAX:
                {
                    //max volume
                    tmp = MAX_USB_AUDIO_VOL;
                    USBWriteEp0(2, &tmp);
                    break;
                }
                case UAC_GET_MIN:
                // min volume
                {
                    tmp = MIN_USB_AUDIO_VOL;
                    USBWriteEp0(2, &tmp);
                    break;
                }
                case 0x0A:
                {
                    USBInEp0Ack();
                    break;
                }
                default:
                break;
            }
            break;
        }

        case (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_ENDPOINT):
        {

            USBInEp0Ack();
            break;
        }

        case (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_ENDPOINT):
        {
            break;
        }

        default:
            break;
    }
}

/*
Name:       USBAudioSetup
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static int32 USBAudioSetup(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    uint8 type = ctrl->bRequestType & USB_TYPE_MASK;
    int32 ret = 0;

	if (type == USB_TYPE_STANDARD)
    {
    	ret = USBAudioStandardReq(ctrl, pDev);
    }
	else if (type == USB_TYPE_CLASS)
	{
		USBAudioClassReq(ctrl, pDev);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

/*
Name:       USBAudioReqest
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioReqest(uint8 epnum, uint32 event, uint32 param, USB_DEVICE *pDev)
{
    pUAS_DEVICE pAud = (pUAS_DEVICE)pDev->pFunDev;

    if (epnum == AS_OUT_EP)
    {
        if (event == UDC_EV_OUT_PKT_RCV)
        {
            USBAudioOutPkt(pAud, (uint16)param, (uint8)(param>>16));
        }
		if (event == UDC_EV_IN_XFER_COMPL)
        return;
    }

    if (event == UDC_EV_ENUM_DONE)
    {
        if ((uint8)param == USB_SPEED_FULL)
        {
            USB_AUDIO_CONFIGS_DESCRIPTOR *ptr = &HSAudioConfigs;
            ptr->ASOutEndp.wMaxPacketSize = FS_ISOC_TX_SIZE;
            AudioEnumState = USB_AUDIO_ENUM_COMPLETED;
        }
    }
}

/*
Name:       USBAudioResume
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioResume(USB_DEVICE *pDev)
{
    SendMsg(MSG_USB_RESUMED);
}

/*
Name:       USBAudioSuspend
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioSuspend(USB_DEVICE *pDev)
{
    pUAS_DEVICE pAud = (pUAS_DEVICE)pDev->pFunDev;
    pAud->connected = 0;
    SendMsg(MSG_USB_DISCONNECT);
}

/*
Name:       USBAudioDisconnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
static void USBAudioDisconnect(USB_DEVICE *pDev)
{
    pUAS_DEVICE pAud = (pUAS_DEVICE)pDev->pFunDev;

    pAud->connected = 0;
    SendMsg(MSG_USB_DISCONNECT);
}


/*
Name:       USBAudioOperCmd
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
void USBAudioOperCmd(uint8 bCmd)
{
    uint8 AudioOper = 0;

    switch(bCmd)
    {
        case AUDIO_CMD_NONE:
            AudioOper = 0;
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_VOLUME_UP:
            AudioOper |= (1 << AUDIO_CMD_VOLUME_UP);
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_VOLUME_DOWN:
            AudioOper |= (1 << AUDIO_CMD_VOLUME_DOWN);
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_VOLUME_MUTE:
            AudioOper |= (1 << AUDIO_CMD_VOLUME_MUTE);
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_VOLUME_CD:
            AudioOper |=  (1 <<AUDIO_CMD_VOLUME_CD);
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_SCAN_NEXT_TRACK:
            AudioOper |= (1 <<AUDIO_CMD_SCAN_NEXT_TRACK);
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_SCAN_PREV_TRACK:
            AudioOper |= (1 <<AUDIO_CMD_SCAN_PREV_TRACK);
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_FAST_FORWARD:
            AudioOper |= (1 <<AUDIO_CMD_FAST_FORWARD);
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;

        case AUDIO_CMD_STOP:
            AudioOper |= (1 <<AUDIO_CMD_STOP);             //standar
            USBWriteEp(HID_IN_EP, 1, &AudioOper);
            break;
    }
}

/*
Name:       USBAudioInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
int32 USBAudioInit(void *arg)
{
    uint32 i;
    pUAS_DEVICE pAud = &USBAudioDev;

    memset(pAud, 0, sizeof(UAS_DEVICE));

    AddOdd          = 0;
    USBOutputLen     = 0;
    DMAStarted       = 0;
    AudioEnumState = USB_AUDIO_ENUM_UNFINISH;
    UsbAudioControlStatus = AUDIO_NO_CONTROL;
    USBbufferIndex   = I2SbufferIndex = 0;
    USBOutputBuf     = UASBuffer;

    pAud->pDev = &USBAudioDriver;
    USBDriverProbe(&USBAudioDriver);

    return 0;
}

/*
Name:       USBAudioDeInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
void USBAudioDeInit(void)
{
    if (UsbMusicStatus == 1)
    {
        UsbMusicDeInit();
        UsbMusicStatus = 0;
    }
    AudioEnumState = USB_AUDIO_ENUM_UNFINISH;
}

/*
Name:       USBAudioDeInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
int32 USBAudioThread(void)
{
    TASK_ARG TaskArg;
    uint32 ret = 0;

    if (UsbMusicStatus == 0)
    {
        if(usbAudio_buf_len() > (USBAUDIO_BUFSIZE*3/4))
        {
            printf("\nusb audio start\n");
			UsbMusicStatus = 1;
            UsbMusicInit();
        }
    } else if (UsbMusicStatus == 1){
		if ((usbaudiosetvolume == 1)&&(usbaudioconfig == 1)){
			//printf("usbaudiovolume=%d\n", gSysConfig.OutputVolume);
			Codec_SetVolumet(gSysConfig.OutputVolume);
			usbaudiosetvolume = 0;
		}
	}

    return ret;
}

/*
Name:       USBAudioCheck
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
uint8 USBAudioEnumCheck(void)
{
    if(AudioEnumState != USB_AUDIO_ENUM_COMPLETED)
        AudioEnumState = USB_AUDIO_ENUM_UNFINISH;

    return  AudioEnumState;
}

/*
Name:       USBAuidoTypeCheck
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_AUDIO_CODE_
uint32 UsbAudioHidControl(uint32 CtrKey)
{
    if(USBAudioEnumCheck() == USB_AUDIO_ENUM_COMPLETED)
    {
        //7 key function
        switch (CtrKey)
        {
            case KEY_VAL_PLAY_SHORT_UP:
            {
                USBAudioOperCmd(AUDIO_CMD_STOP);
                USBAudioOperCmd(AUDIO_CMD_NONE);  //发送0清除标志
                break;
            }

            case KEY_VAL_FFD_SHORT_UP:
            {
                //Next
                USBAudioOperCmd(AUDIO_CMD_SCAN_NEXT_TRACK);
                USBAudioOperCmd(AUDIO_CMD_NONE);
                break;
            }

            case KEY_VAL_FFW_SHORT_UP:
            {
                //Prev
                USBAudioOperCmd( AUDIO_CMD_SCAN_PREV_TRACK);
                USBAudioOperCmd(AUDIO_CMD_NONE);
                break;
            }

            case KEY_VAL_UP_DOWN:                                   //volume increse
            case KEY_VAL_UP_PRESS:
            {

                USBAudioOperCmd(AUDIO_CMD_VOLUME_UP);
                USBAudioOperCmd(AUDIO_CMD_NONE);
                break;
            }
            case KEY_VAL_DOWN_DOWN:                                 //volume reduce
            case KEY_VAL_DOWN_PRESS:
            {
                USBAudioOperCmd(AUDIO_CMD_VOLUME_DOWN);
                USBAudioOperCmd(AUDIO_CMD_NONE);
                break;
            }

            default:
                break;

        }
    }


    return RETURN_OK;
}

/***************************************************************************/
_ATTR_USB_AUDIO_CODE_
void usbAudio_buf_puts(const unsigned char *s, unsigned int len)
{
    unsigned int head;
	int getlen = len;
    if(usbAudio_buf_free_size() < len)
    {
        return;
    }
    head = vs_bufhead;
    while(len--)
    {
        vs_buf.b8[head++] = *s++;
        if (head >= USBAUDIO_BUFSIZE)
        {
			head = 0;
		}
    }
    vs_bufhead = head;
}

_ATTR_USB_AUDIO_CODE_
unsigned int usbAudio_buf_free_size(void)
{
	unsigned int head, tail;

	head = vs_bufhead;//the vaild data begin form positon 'vs_buftail' and end in position 'vs_bufhead'.
	tail = vs_buftail;

	if (head > tail)
	{
		return (USBAUDIO_BUFSIZE - (head - tail) - 1);
	}
	else if(head < tail)
	{
		return (tail - head - 1);
	}

	return (USBAUDIO_BUFSIZE - 1);
}

_ATTR_USB_AUDIO_CODE_
unsigned int usbAudio_buf_len(void)
{
	unsigned int head, tail;

	head = vs_bufhead;//the vaild data begin form positon 'vs_buftail' and end in position 'vs_bufhead'.
	tail = vs_buftail;

	if(head > tail)
	{
		return (head-tail);
	}
	else if(head < tail)
	{
		return (USBAUDIO_BUFSIZE-(tail-head));
	}

	return 0;
}

_ATTR_USB_AUDIO_CODE_
void usbAudio_buf_reset(void)
{
	vs_bufhead = 0;
	vs_buftail = 0;
    vs_offset = 0;
	return;
}

/*
 * audio file system interface..
 */
_ATTR_USB_AUDIO_CODE_
unsigned short RKUSB_FRead(unsigned char *b, unsigned short s, void *f)
{
    unsigned int tail;
    int readed, i;

    readed = usbAudio_buf_len();

    if (readed > s)
    	readed = s;

    tail = vs_buftail;

    i = 0;
    while (i < readed)
    {
	    b[i++] = vs_buf.b8[tail++];
	    if (tail >= USBAUDIO_BUFSIZE)
	    {
		      tail = 0;
	    }
    }
    vs_buftail = tail;

    vs_offset += readed;
  return readed;
}

/*--------------------------- Local Function Prototypes ----------------------*/
_ATTR_USB_AUDIO_CODE_
int UsbAudio_open(void)
{
    int ret;

    g_outLength = USB_AUDIO_OUT_BUF_SIZE;
    g_BtBufIndex = 0;
	g_timePosCount += g_outLength;;

    memset(&gUsbAudioOutputPtr[0][0], 0, 2*2*USB_AUDIO_OUT_BUF_SIZE*sizeof(int16));

    return 1;
}

_ATTR_USB_AUDIO_CODE_
int UsbAudio_get_buffer(unsigned long ulParam1 , unsigned long ulParam2)
{
	*(int *)ulParam1 = (unsigned long)(&gUsbAudioOutputPtr[g_BtBufIndex^1][0]);
	*(int *)ulParam2 = g_outLength;

	return 1;
}

_ATTR_USB_AUDIO_CODE_
int UsbAudio_decode(void)
{
    int32  i;
    uint32 len;
    uint32 readedbyte;
    int16 *pbuf = (int16*)gUsbAudioOutputPtr[g_BtBufIndex];

    readedbyte = RKUSB_FRead((unsigned char *)pbuf, USB_AUDIO_OUT_BUF_SIZE*2*2, NULL);
    if(readedbyte <= 0)
    {
         memset(&gUsbAudioOutputPtr[0][0], 0, 2*2*USB_AUDIO_OUT_BUF_SIZE*sizeof(int16));
         return 0;
    }

    g_timePosCount += g_outLength;
    g_BtBufIndex ^= 1;

    return 1;
}

_ATTR_USB_AUDIO_CODE_
int UsbAudio_get_time(void)
{
	return g_timePosCount * 1000 / 48000;
}

_ATTR_USB_AUDIO_CODE_
uint32 UsbAudioCodecOpen(void)
{
    UsbAudio_open();

	pUsbAudioRegKey.bitrate = 192000;
	pUsbAudioRegKey.bps = 16;
	pUsbAudioRegKey.samplerate = 48000;
	pUsbAudioRegKey.channels = 2;
    printf("bps = %d s = %d bitrate = %d\n",
		pUsbAudioRegKey.bps,
		pUsbAudioRegKey.samplerate,
		pUsbAudioRegKey.bitrate);

    return OK;
}

_ATTR_USB_AUDIO_CODE_
void UsbAudioDmaIsrHandler(void)
{
    UsbAudio_decode();
	UsbAudio_get_buffer((unsigned long)&UsbAudioPtr, (unsigned long)&UsbAudioLen);
    //DEBUG("UsbAudioPtr = 0x%x, UsbAudioLen = %d", UsbAudioPtr,UsbAudioLen);

    if (USBAUDIO_STATE_PLAY == UsbAudioPlayState)
    {
        DmaStart(AUDIO_DMACHANNEL_IIS,
				(UINT32)UsbAudioPtr,
				(uint32)(&(I2s_Reg->I2S_TXDR)),
				UsbAudioLen,
				&UsbAudioControlDmaCfg,
				UsbAudioDmaIsrHandler);
    }
}

_ATTR_USB_AUDIO_CODE_
void UsbAudioStart(void)
{
    int ret = 0;
    uint32 timeout = 200;

    FREQ_EnterModule(FREQ_AUDIO_INIT);
    
    if (ERROR == UsbAudioCodecOpen())
    {
        FREQ_ExitModule(FREQ_AUDIO_INIT);

        DEBUG("Codec Open Error1");
        return;
    }
    
    printf("bps111 = %d s = %d bitrate = %d\n",
		pUsbAudioRegKey.bps,
		pUsbAudioRegKey.samplerate,
		pUsbAudioRegKey.bitrate);

    DEBUG("audio codec open success.");

    FREQ_ExitModule(FREQ_AUDIO_INIT);

    I2SInit(I2S_CH, I2S_PORT,I2S_MODE,
            pUsbAudioRegKey.samplerate, I2S_FORMAT,
            I2S_DATA_WIDTH16, I2S_NORMAL_MODE);

    Codec_SetMode(Codec_DACoutHP, ACodec_I2S_DATA_WIDTH16);
    Codec_SetSampleRate(pUsbAudioRegKey.samplerate);
   	UsbAudio_get_buffer((unsigned long)&UsbAudioPtr, (unsigned long)&UsbAudioLen);
    memset((uint8*)UsbAudioPtr, 0, UsbAudioLen * 2);
	Codec_SetVolumet(27);

    UsbAudioDecodeing = 0;
    UsbDmaTransting = 0;

    timeout = 200;

    while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
    {
        DelayMs(1);
        if (--timeout == 0)
        {
            break;
        }
    }

    if (UsbAudioPlayerState == USBAUDIO_STATE_PLAY)
    {
        UsbDmaTransting = 1;
        DmaStart(AUDIO_DMACHANNEL_IIS,
				(UINT32)UsbAudioPtr,
				(uint32)(&(I2s_Reg->I2S_TXDR)),
                 UsbAudioLen,
                 &UsbAudioControlDmaCfg,
                 UsbAudioDmaIsrHandler);
        UsbAudioPlayState = USBAUDIO_STATE_PLAY;
        UsbAudioDecodeing = 0;
        I2SStart(I2S_CH, I2S_START_DMA_TX);
		Codec_SetVolumet(27);
		usbaudioconfig = 1;
        printf("Audio DMA & I2S start...\n");
    }
}

_ATTR_USB_AUDIO_CODE_
BOOLEAN UsbAudioPause(void)
{
    uint32 timeout = 20000;//20000;
    printf("=== AudioPause in ===\n");

    if (USBAUDIO_STATE_PLAY == UsbAudioPlayerState)
    {
        UsbAudioPlayerState = USBAUDIO_STATE_PAUSE;
        UsbAudioPlayState = USBAUDIO_STATE_STOP;

        while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
        {
            DelayMs(1);

            if (--timeout == 0)
            {
                DEBUG("dma busy out");
                break;
            }
        }

        UsbAudioPlayInfo.VolumeCnt = 0;

        AutoPowerOffEnable();

        UsbAudioDecodeing = 0;
        UsbDmaTransting = 0;
    }
    else
    {
        while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
        {
            DelayMs(1);

            if (--timeout == 0)
            {
                DEBUG("dma busy out");
                break;
            }
        }

        UsbAudioPlayInfo.VolumeCnt = 0;
    }

    printf("=== AudioPause out ===\n");
    return TRUE;
}

_ATTR_USB_AUDIO_CODE_
BOOLEAN UsbAudioResume(void)
{
    printf("=== AudioResume in ===  \n");

    if (USBAUDIO_STATE_PLAY == UsbAudioPlayerState)
    {
        return;
    }

    if (UsbAudioPlayerState == USBAUDIO_STATE_PAUSE)
    {
        AutoPowerOffDisable();
    }
	UsbAudio_decode();
	UsbAudio_get_buffer((unsigned long)&UsbAudioPtr, (unsigned long)&UsbAudioLen);  

    memset((uint8*)UsbAudioPtr, 0, UsbAudioLen * 2);

    UsbDmaTransting = 1;
    DmaStart(AUDIO_DMACHANNEL_IIS,
			(UINT32)UsbAudioPtr,
			(uint32)(&(I2s_Reg->I2S_TXDR)),
			UsbAudioLen, &UsbAudioControlDmaCfg,
			UsbAudioDmaIsrHandler);
    I2SStart(I2S_CH, I2S_START_DMA_TX);
	Codec_SetVolumet(27);

    UsbAudioPlayerState = USBAUDIO_STATE_PLAY;
    UsbAudioPlayState   = USBAUDIO_STATE_PLAY;
    UsbAudioDecodeing = 0;
    printf("=== AudioResume out ===\n");

    return TRUE;
}


_ATTR_USB_AUDIO_CODE_
BOOLEAN UsbAudioDecodeProc(MSG_ID id, void * msg)
{
    BOOLEAN ret = TRUE;
    unsigned long  HoldOnTimeTemp;

    switch (id)
    {
        case MSG_AUDIO_DECSTART:
            UsbAudioStart();
            break;

        case MSG_AUDIO_PAUSE:
            AudioPause();
            break;

        case MSG_AUDIO_RESUME:
            AudioResume();
            break;

		case MSG_AUDIO_STOP:
        	//AudioStop((int)msg);
            break;

        default:
            ret = FALSE;
            break;
    }

    return ret;
}

_ATTR_USB_AUDIO_CODE_
int32 UsbAudioTrackInit(void)
{
    Usb_AudioPlayStatus = USB_AUDIO_PALY_STATE_PLAY;

    return RETURN_OK;
}

_ATTR_USB_AUDIO_CODE_
int32 UsbAudioMsgProcess(void)
{
    if(Usb_AudioPlayStatus == USB_AUDIO_PALY_STATE_PLAY)
    {
        if(usbAudio_buf_len() < (USBAUDIO_BUFSIZE/3))
        {
            Usb_AudioPlayStatus = USB_AUDIO_PALY_STATE_PAUSE;
            printf("usb audio pause\n");
            UsbAudioPause();
            usbAudio_buf_reset();
        }
    }
    else if(Usb_AudioPlayStatus == USB_AUDIO_PALY_STATE_PAUSE)
    {
        if(usbAudio_buf_len() > (USBAUDIO_BUFSIZE*3/4))
        {
            Usb_AudioPlayStatus = USB_AUDIO_PALY_STATE_PLAY;
            printf("usb audio resume\n");
            UsbAudioResume();
        }
    }

    return RETURN_OK;
}

_ATTR_USB_AUDIO_CODE_
void UsbMusicInit(void)
{
    int i;

	usbaudioconfig = 0;
	UsbAudioTrackInit();

    Codec_SetMode(Codec_DACoutHP, ACodec_I2S_DATA_WIDTH16);

    AutoPowerOffDisable();

    UsbAudioPlayerState = USBAUDIO_STATE_PLAY;
    UsbAudioDecodeProc(MSG_AUDIO_DECSTART, NULL);

    return;
}

_ATTR_USB_AUDIO_CODE_
UINT32 UsbMusicService(void)
{
	int ret = 0;

	UsbAudioMsgProcess();

    return ret;
}

_ATTR_USB_AUDIO_CODE_
void UsbMusicDeInit(void)
{
    int i ;

    //UsbAudioDecodeProc(MSG_AUDIO_STOP, (void*)USBAUDIO_STOP_FORCE);

    if (UsbAudioPlayState != USBAUDIO_STATE_PAUSE)
    {
        AutoPowerOffEnable();
    }

    UsbAudioPlayState = USBAUDIO_STATE_STOP;
	usbaudioconfig = 0;

    Codec_ExitMode(Codec_DACoutHP);

    DEBUG("Music Thread Exit");
}

#endif

/***************************************************************************/


