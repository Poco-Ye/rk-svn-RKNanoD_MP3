
#include  "SysInclude.h"
#include  "audio_main.h"


#include  "FsInclude.h"
#include  "File.h"
#include  "FDT.h"
#include  "audio_globals.h"




#ifdef MP3_ENC_INCLUDE
#include  "RecordControl.h"
#include "mp3_enc_types.h"



//extern void L3_compress(void);
_ATTR_RECORD_CONTROL_BSS_ config_t config_A;

extern RecordBlock  gRecordBlock;


extern UINT32 gEncodeDone;

_ATTR_RECORD_CONTROL_CODE_
void MP3_Init(void * ptr)
{
#ifndef A_CORE_DECODE
    gEncodeDone = 0;
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_ENCODE_INIT, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteA2BData((uint32)ptr, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    while (!gEncodeDone)
    {
#ifdef _LOG_DEBUG_
        BBDebug();
#endif
        __WFI();
    }
    gEncodeDone = 0;
#endif
}
/*
--------------------------------------------------------------------------------
  Function name : void WavEncodeVariableInit()
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void Mp3VariableInit(void)
{
    UINT16 i;
    RecordInputBufferIndex   = 0;   //used for buffer switch.
    pRecordInputBuffer       = (UINT16*)WavEncodeInputBuffer;
    pRecordPCMInputBuffer    = pRecordInputBuffer;
    pRecordEncodeInputBuffer = pRecordInputBuffer + WAV_AD_PIPO_BUFFER_SIZE;
    for (i = 0; i < WAV_AD_PIPO_BUFFER_SIZE * 2; i++)
    {
        *(pRecordInputBuffer + i) = 0;
    }
    WriteBuffer.uHead     = 0;
    WriteBuffer.uTail     = 0;
    WriteBuffer.uCounter  = 0;
    for (i = 0; i < WAV_ENCODER_WRITE_BUFFER_LENGTH; i++)
    {
        WriteBuffer.Data[i] = 0;
    }
    printf("Mp3 VariableInit  \n");
    RecordFileOffset   = 0;
    uSampleCounter     = 0;
    RecordConFlag = RECORD_CON_NULL;                     //the flag that be used to control coding.
    RecordStaFlag = RECORD_STA_PCMBUF_EMPTY;             //the flag that be used to control input buffer empty or full.
    RecordNOISECount = RECORD_STARTNOISEBLOCK;
}
_ATTR_RECORD_CONTROL_CODE_
void Mp3EncodeVariableInit(void)
{
    UINT16 i;


    RecordInputBufferIndex   = 0;   //used for buffer switch.
    pRecordInputBuffer       = (UINT16*)WavEncodeInputBuffer;
    pRecordPCMInputBuffer    = pRecordInputBuffer;
    pRecordEncodeInputBuffer = pRecordInputBuffer + WAV_AD_PIPO_BUFFER_SIZE;

    for (i = 0; i < WAV_AD_PIPO_BUFFER_SIZE * 2; i++)
    {
        *(pRecordInputBuffer + i) = 0;
    }

    //WriteBuffer initialization.
    WriteBuffer.uHead     = 0;
    WriteBuffer.uTail     = 0;
    WriteBuffer.uCounter  = 0;

    //cache buffer for write initialization.
    for (i = 0; i < WAV_ENCODER_WRITE_BUFFER_LENGTH; i++)
    {
        WriteBuffer.Data[i] = 0;
    }
    memset((uint8*)&config_A,0,sizeof(config_t));

    printf("Mp3 EncodeVariableInit  \n");
    #if 0

    config.mpeg.type = MPEG2_5;
    config.mpeg.layr = LAYER_3;
    config.mpeg.mode = MODE_MONO;
    config.mpeg.psyc = 0;
    config.mpeg.emph = 0;
    config.mpeg.crc  = 0;
    config.mpeg.ext  = 0;
    config.mpeg.mode_ext  = 0;
    config.mpeg.copyright = 0;
    config.mpeg.original  = 1;
    config.mpeg.granules = 1; //与MP3格式有关(标准)

    config.mpeg.samplerate_index = find_samplerate_index(config.mpeg.samplerate);
    config.mpeg.bitrate_index    =find_bitrate_index(config.mpeg.bitr);
    config.mpeg.cutoff = set_cutoff();
    #endif

    memset((uint8*)&gRecordBlock,0,sizeof(RecordBlock));

    /////////////////////////////////////////////////////////////////////////
    //zyz
    gRecordBlock.channel        = RecordChannel;
    gRecordBlock.dataWidth      = RecordDataWidth;
    gRecordBlock.encodeType     = RecordEncodeType;
    gRecordBlock.sampleRate     = RecordSampleRate;
    gRecordBlock.Bitrate        = RecordBitrate;
    gRecordBlock.PCM_source     = 0;
    gRecordBlock.MP3_samples_per_frame     = 0;


    #ifdef _NS_
    if((RecordType == RECORD_TYPE_MIC_STERO)
        || (RecordType == RECORD_TYPE_MIC1_MONO)
        || (RecordType == RECORD_TYPE_MIC2_MONO))
    {
        gRecordBlock.FilterFlag = 1;
    }
    else
    {
        gRecordBlock.FilterFlag = 0;
    }
    #else
    gRecordBlock.FilterFlag = 0;
    #endif
    DEBUG("encodeType = %d",gRecordBlock.encodeType);

    MP3_Init(&gRecordBlock);

    if(RecordChannel == RECORD_CHANNEL_MONO)
    {
        RecordInputBufferLength = gRecordBlock.MP3_samples_per_frame * RecordChannel*2;     //the number of AD sampling.
    }
    else
    {
        RecordInputBufferLength = gRecordBlock.MP3_samples_per_frame * RecordChannel;     //the number of AD sampling.
    }

    DEBUG("MP3_samples_per_frame = %d",gRecordBlock.MP3_samples_per_frame);
    DEBUG("RecordInputBufferLength = %d",RecordInputBufferLength);
    RecordFileOffset   = 0;
    uSampleCounter     = 0;
    RecordConFlag      = RECORD_CON_NULL;                     //the flag that be used to control coding.
    RecordStaFlag      = RECORD_STA_PCMBUF_EMPTY;             //the flag that be used to control input buffer empty or full.
    RecordNOISECount   = RECORD_STARTNOISEBLOCK;

}


#endif



