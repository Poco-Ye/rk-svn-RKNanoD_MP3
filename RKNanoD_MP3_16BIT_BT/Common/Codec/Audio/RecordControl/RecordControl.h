/*
********************************************************************************
*                   Copyright (c) 2009, Rockchip
*                         All rights reserved.
*
* File Name:   RecordControl.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               WangBo          2009-4-16          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef RECORD_CONTROL_H_
#define RECORD_CONTROL_H_


#undef  EXT
#ifdef _IN_RECORD_CONTROL
#define EXT
#else
#define EXT extern
#endif

/******************************************************************************/
/*                                                                            */
/*                          Sections Define                                   */
/*                                                                            */
/******************************************************************************/

//-------------------------------------------Record Control-----------------------------------------------
#define     _ATTR_RECORD_CONTROL_CODE_            __attribute__( (section("RecordControlCode")))
#define     _ATTR_RECORD_CONTROL_DATA_            __attribute__( (section("RecordControlData")) )
#define     _ATTR_RECORD_CONTROL_BSS_             __attribute__( (section("RecordControlBss"),zero_init) )

/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/
#define     MIN_RECORD_RESERVE_MEMORY           ((UINT32)512 * 1024)  //the smallest memory space is 512k.

//RecordServiceFlag
#define     RECORD_STA_NULL                     0x0000
#define     RECORD_STA_PCMBUF_EMPTY             (0x0001 << 0)
#define     RECORD_STA_PCMBUF_FULL              (0x0001 << 1)
#define     RECORD_STA_DISP_INIT                (0X0001 << 2)
#define     RECORD_STA_TIME_UPDATE              (0X0001 << 3)
#define     RECORD_STA_FILEBUF_FULL             (0x0001 << 4)
#define     RECORD_STA_DISK_FULL                (0x0001 << 5)
#define     RECORD_STA_DECODE_MUTE_ON           (0x0001 << 6)
#define     RECORD_STA_DECODE_MUTE_OFF          (0x0001 << 7)

//RecordConFlag
#define     RECORD_CON_NULL                     0x0000
#define     RECORD_CON_ENCODE_INIT              (0x0001 << 0)
#define     RECORD_CON_ENCODE_ENABLE            (0x0001 << 1)

//RecordType
#define     RECORD_TYPE_MIC_STERO               0
#define     RECORD_TYPE_LINEIN1                 1
#define     RECORD_TYPE_LINEIN2                 2
#define     RECORD_TYPE_MIC1_MONO               3
#define     RECORD_TYPE_MIC2_MONO               4
#define     RECORD_TYPE_NULL                    5

#define     RECORD_CHANNEL_STERO              2
#define     RECORD_CHANNEL_MONO              1
#define     RECORD_DATAWIDTH_16BIT           0x0F
#define     RECORD_DATAWIDTH_24BIT           0x17
//record code type.
#define     RECORD_ENCODE_TYPE_WAV              0
#define     RECORD_ENCODE_TYPE_MP3              1
#define     RECORD_ENCODE_TYPE_PCM              2
//gbRecordQuality

//record status,reuse with play key.

#define    RECORD_STARTNOISEBLOCK    19
#define    RECORD_KEYNOISEBLOCK      15

#define     WAV_ENCODER_WRITE2FLASH_LENGTH      2048
#define     WAV_ENCODER_BUFFER_NUM              24
#define     WAV_ENCODER_WRITE_BUFFER_LENGTH     (WAV_ENCODER_WRITE2FLASH_LENGTH * WAV_ENCODER_BUFFER_NUM)
#define     WAV_HEAD_BUFFER_SIZE                4096
#define     WAV_AD_PIPO_BUFFER_SIZE             6144

//definition of sampling rate.
#define     RECORD_SAMPLE_FREQUENCY_8KHZ        8000    //8KHz
#define     RECORD_SAMPLE_FREQUENCY_11KHZ       11025   //11KHz
#define     RECORD_SAMPLE_FREQUENCY_16KHZ       16000   //16KHz
#define     RECORD_SAMPLE_FREQUENCY_22KHZ       22050   //22KHz
#define     RECORD_SAMPLE_FREQUENCY_32KHZ       32000   //32KHz
#define     RECORD_SAMPLE_FREQUENCY_44_1KHZ     44100   //44.1KHz
#define     RECORD_SAMPLE_FREQUENCY_48KHZ       48000   //48KHz

#define RECORD_PCM     1
#define RECORD_WAV     2
#define RECORD_TYPE_CONFIG     RECORD_PCM
/******************************************************************************/
/*                                                                            */
/*                          Struct Define                                     */
/*                                                                            */
/******************************************************************************/

typedef struct
{
    UINT16      TotalFiles;     //the total file number in current directory.
    UINT16      CurrentFileNum;
    UINT16      PlayedFileNum;
    UINT16      DiskTotalFiles; //the total file number in all disk.

    UINT16      Range;
    UINT16      Mode;

    UINT16      *pExtStr;       //type of file.

    FDT         Fdt;
    FIND_DATA   FindData;

    UINT16      RandomBuffer[16];
    UINT8       Path[3 + (MAX_DIR_DEPTH - 1) * 12 + 1];

} RECORD_SYS_FILE_INFO;

// д�ļ�������.
typedef struct
{
    //write buffer must algin 4
    UINT8   Data[WAV_ENCODER_WRITE_BUFFER_LENGTH]; //WriteBuffer memory space.

    UINT16  uHead;        //the head of cycle.
    UINT16  uTail;        //the end of cycle.
    UINT16  uCounter;     //the valid data in current WriteBuffer.

} WAV_WRITE_BUFFER_STRUCT;

typedef enum Record_FS
{
    RECORD_FS_8000Hz   = 8000,
    RECORD_FS_11025Hz  = 11025,
    RECORD_FS_12KHz    = 12000,
    RECORD_FS_16KHz    = 16000,
    RECORD_FS_22050Hz  = 22050,
    RECORD_FS_24KHz    = 24000,
    RECORD_FS_32KHz    = 32000,
    RECORD_FS_44100Hz  = 44100,
    RECORD_FS_48KHz    = 48000,
    RECORD_FS_64KHz    = 64000,
    RECORD_FS_88200Hz   = 88200,
    RECORD_FS_96KHz    = 96000,
    RECORD_FS_128KHz   = 128000,
    RECORD_FS_1764KHz  = 176400,
    RECORD_FS_192KHz   = 192000,
    RECORD_FSSTOP      = 192000
} RecordFS_en_t;
typedef enum _MEDIA_MSGBOX_ENCODE_CMD
{
    MEDIA_MSGBOX_CMD_ENCODE_NULL = 200,

    MEDIA_MSGBOX_CMD_ENC_OPEN,
    MEDIA_MSGBOX_CMD_ENC_OPEN_CMPL,

    //record cmd
    MEDIA_MSGBOX_CMD_ENCODE,
    MEDIA_MSGBOX_CMD_ENCODE_PREPARE,
    MEDIA_MSGBOX_CMD_ENCODE_INIT,
    MEDIA_MSGBOX_CMD_ENCODE_INIT_CMPL,
//    MEDIA_MSGBOX_CMD_ENCODE_DATA_REQ,
    MEDIA_MSGBOX_CMD_ENCODE_CMPL,      /*���һ�α������*/
    MEDIA_MSGBOX_CMD_ENCODE_ERR,       /*�������*/

    MEDIA_MSGBOX_CMD_REC_FILE_CLOSE,
    MEDIA_MSGBOX_CMD_REC_FILE_CLOSE_CMPL,
    MEDIA_MSGBOX_CMD_REC_FILE_CLOSE_HANDSHK,

}MEDIA_MSGBOX_ENCODE_CMD;

/******************************************************************************/
/*                                                                            */
/*                          Variable Define                                   */
/*                                                                            */
/******************************************************************************/
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordExitTaskID;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordType;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordSampleRate;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordBitrate;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordChannel;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordDataWidth;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordEncodeType;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordCodecConfig;//enter FM module and to record directly.
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordFilterFlag;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordDataShiftBit;

_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      gbRecordQuality;   //quality of recording.
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      gbRecordFormat;    //the format of recording.

_ATTR_RECORD_CONTROL_BSS_       EXT INT16       hRecordFile;
_ATTR_RECORD_CONTROL_BSS_       EXT uint32      RecordLdoBack;

_ATTR_RECORD_CONTROL_BSS_       EXT  uint32  RecordEncodeing;
_ATTR_RECORD_CONTROL_BSS_       EXT  uint32  RecordDmaTransting;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordStaFlag;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordConFlag;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordCpuFreqBackup;

_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordInputBufferIndex;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordInputBufferLength;    //

_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      *pRecordEncodeInputBuffer;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      *pRecordPCMInputBuffer;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      *pRecordInputBuffer;

_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordFreeMemory;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordFileOffset;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordWriteFileCounter;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      uSampleCounter;    //compare with sampling rate,decide the change of time.

_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordTotalTime;              //total time of record.
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordRemainTime;             //left time can be use for recording.
_ATTR_RECORD_CONTROL_BSS_       EXT UINT32      RecordCurrTime;               //the current record time.

_ATTR_RECORD_CONTROL_BSS_       EXT UINT16      RecordNOISECount;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT8       *WavEncodeBuf;
_ATTR_RECORD_CONTROL_BSS_       EXT UINT8       AdcSampleFlag;

_ATTR_RECORD_CONTROL_BSS_       EXT  UINT16     RecordLongFileName[MAX_FILENAME_LEN]; //record file name.
_ATTR_RECORD_CONTROL_BSS_       EXT  RECORD_SYS_FILE_INFO   RecordFileInfo;
_ATTR_RECORD_CONTROL_CODE_      EXT  WAV_WRITE_BUFFER_STRUCT  __align(4) WriteBuffer;

_ATTR_RECORD_CONTROL_BSS_       EXT UINT8*      WavEncodeHeadBuffer;
_ATTR_RECORD_CONTROL_CODE_      EXT UINT32 __align(4) WavEncodeInputBuffer[WAV_AD_PIPO_BUFFER_SIZE];  //Buffer must align 4
#define RECORD_DMACHANNEL_IIS    (DMA_CHN_MAX - 1)

/******************************************************************************/
/*                                                                            */
/*                         Function Declare                                   */
/*                                                                            */
/******************************************************************************/
void RecordAdcInit(void);
void RecordAdcDeInit(void);
void RecordCpuInit(void);
void RecordServiceCpuResume(void);
BOOL RecordGetFileName(void);
BOOL RecordServiceGetTotalTime (void);
void RecordGetCurTime(void);
void RecordGetInfo(void);
void GetEncodeType(void);
void RecordInit(void);
BOOLEAN RecordCreateFile (void);
int RecordStart(void);
void RecordInputSwitchBuffer(void);
BOOLEAN RecordWriteFile (void);
void RecordWriteFileTail (void);
void RecordCodecInit(void);
void RecordCodecDeInit(void);
void RecordEnable(void);
void RecordDisable(void);
void RecordStop(void);
BOOLEAN RecordEncodeProc(MSG_ID id,void * msg);

void WavEncodeVariableInit(void);
ExecFunPtr DataEncodeIsr(void);
void WavEncodeHeaderInit();

void RecInit(void *pArg);
UINT32 RecService(void);
void RecDeInit(void);

void RecordGetDataFromFM(UINT32 *DataBuf,uint32 encodeDataLen);




/*
--------------------------------------------------------------------------------

  Description:  the definition of recording.

--------------------------------------------------------------------------------
*/
#ifdef _IN_RECORD_CONTROL
_ATTR_RECORD_CONTROL_DATA_ EXT THREAD RecordThread = {

    NULL,
    NULL,

    RecInit,
    RecService,
    RecDeInit,

    NULL
};
#else
_ATTR_RECORD_CONTROL_DATA_ EXT THREAD RecordThread;
#endif


#endif


