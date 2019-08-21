/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  AudioControl.C
*
* Description:
*
* History:      <author>          <time>        <version>
*                 ZS      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_AUDIO_CONTROL_

#include "SysInclude.h"
#include "FileInfo.h"
#include "Rk_Eq.h"

#ifdef _MUSIC_
#include "audio_globals.h"
#include "audio_file_access.h"
#include "Effect.h"
#include "HoldonPlay.h"
#include "pmu.h"
#include "MainMenu.h"
#include "AudioControl.h"
#include "fade.h"
#include "ID3.h"
#include "Dma.h"
#include "myRandom.h"
#include "Spectrum.h"
#include "MediaBroWin.h"
#include "medialibwin.h"
#include "image_main.h"
#include "AddrSaveMacro.h"
#include "wma_parse.h"
#include "I2S.h"
#include "pcm.h"

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#include "SetBluetooth.h"
#include "rk_bt_Api.h"

#endif

#define _ATTR_AUDIO_SBC_ENCODE_TEXT_     __attribute__((section("SbcEnCodeCode")))
#define _ATTR_AUDIO_SBC_ENCODE_DATA_     __attribute__((section("SbcEnCodeData")))
#define _ATTR_AUDIO_SBC_ENCODE_BSS_      __attribute__((section("SbcEnCodeBss"),zero_init))


extern _ATTR_SYS_DATA_ DMA_CFGX AudioControlDmaCfg;
extern  unsigned long SRC_Num_Forehead;
extern unsigned int MP3_FORMAT_FLAG ;


#ifdef MP3_DEC_INCLUDE
extern volatile int is_synthing;
#endif

_ATTR_AUDIO_DATA_ uint8 EqMode[8] = {EQ_HEAVY, EQ_POP, EQ_JAZZ, EQ_UNIQUE, EQ_USER, EQ_USER, EQ_NOR, EQ_BASS};
_ATTR_AUDIO_DATA_ short UseEqTable[CUSTOMEQ_LEVELNUM] = { -10, -6, -3, 0, 3, 6, 10};
_ATTR_AUDIO_BSS_  int    server_ori_seed ;
_ATTR_AUDIO_BSS_  int    rand_first_flag;

_ATTR_AUDIO_BSS_  uint32 gDecCmd;
_ATTR_AUDIO_BSS_  uint32 gDecData;
_ATTR_AUDIO_BSS_  uint32 gDecDone;
_ATTR_AUDIO_BSS_  uint32 gSeekDone;
_ATTR_AUDIO_BSS_  uint32 gOpenDone;
_ATTR_AUDIO_BSS_  uint32 gGetBuffDone;
_ATTR_AUDIO_BSS_  uint32 gGetTimeDone;
_ATTR_AUDIO_BSS_  uint32 gCloseDone;
_ATTR_AUDIO_BSS_  uint32 gError;
_ATTR_AUDIO_BSS_  uint32 gACKDone;

_ATTR_AUDIO_BSS_  FILE_SEEK_OP_t    *gpFileSeekParm;
_ATTR_AUDIO_BSS_  FILE_READ_OP_t    *gpFileReadParm;
_ATTR_AUDIO_BSS_  FILE_WRITE_OP_t   *gpFileWriteParm;
_ATTR_AUDIO_BSS_  FLAC_SEEKFAST_OP_t *gpFlacSeekFastParm;

_ATTR_AUDIO_BSS_  MediaBlock    gpMediaBlock;
_ATTR_AUDIO_BSS_  FILE_HANDLE_t gFileHandle;

_ATTR_SYS_BSS_    ID3V2X_INFO    gAudioId3Info;
extern uint8 btAvrcpVolumeChanged;

#ifdef _A2DP_SOUCRE_
_ATTR_AUDIO_BSS_ uint sbcTotalCnt;
_ATTR_AUDIO_BSS_ uint FrameSize;


#define SBC_FRAME_NUM_PRE  5

extern uint8 BtWinStatus;

int avdtp_send_media(char * buf, int len, int frameCnt, void (*func)(void));

#define SBC_ENCODE_BUF_LEN  1024

//#define AUDIO_RESAMPLE_INPUT_PRE  128
#define AUDIO_RESAMPLE_INPUT_PRE  256

#define AUDIO_RESAMPLE_BUF_LEN  (AUDIO_RESAMPLE_INPUT_PRE*6)

_ATTR_AUDIO_DATA_ uint8 AudioSbcOutPut[SBC_ENCODE_BUF_LEN] = 0;
_ATTR_AUDIO_DATA_ uint8 AudioUnencodeBuf[512] = 0;

_ATTR_AUDIO_DATA_ uint8  AudioResampleOutBuf[AUDIO_RESAMPLE_BUF_LEN] = 0;

_ATTR_AUDIO_BSS_  uint16 AudioResampleIndex;


_ATTR_AUDIO_DATA_ uint16 AudioEncodeCnt = 0;
_ATTR_AUDIO_DATA_ uint16 AudioEncodeLeftCnt = 0;

_ATTR_AUDIO_DATA_ uint16 AudioDecodeCount = 0;

_ATTR_AUDIO_DATA_ char * AudioDecodeIntputPtr;

_ATTR_AUDIO_BSS_  uint8 AudioResampleInit;

_ATTR_AUDIO_TEXT_
uint16 *AudioGetId3Singer(void)
{
    return (uint16 *)&gAudioId3Info.id3_singer[0];
}

_ATTR_AUDIO_TEXT_
AudioSbcEncodeProc(void)
{
    uint TotalCntTemp;
    uint left_cnt;
    uint buf_need = 0;
    TotalCntTemp = 0;
    while(AudioDecodeCount)
    {
        if(SBC_ENCODE_BUF_LEN > sbcTotalCnt)
        {
            left_cnt = SBC_ENCODE_BUF_LEN- sbcTotalCnt;
        }
        else
        {
            left_cnt = 0;
        }

        if(FrameSize != 0)
        {
            buf_need = FrameSize;
        }
        if((left_cnt > 0) && (left_cnt >=  buf_need) )
        {
            if(AudioEncodeLeftCnt)
            {
                if(AudioDecodeCount >= (512- AudioEncodeLeftCnt) )
                {
                    memcpy(AudioUnencodeBuf+AudioEncodeLeftCnt, (uint8 *)AudioDecodeIntputPtr+AudioEncodeCnt, 512- AudioEncodeLeftCnt);
                    AudioEncodeCnt += (512- AudioEncodeLeftCnt);
                    AudioDecodeCount -= (512- AudioEncodeLeftCnt);
                    AudioEncodeLeftCnt = 0;
                    TotalCntTemp = sbc_enc(NULL,AudioUnencodeBuf,512 ,AudioSbcOutPut+sbcTotalCnt, &FrameSize);
                    sbcTotalCnt += TotalCntTemp;

                }
            }
            else
            {
                if(AudioDecodeCount >= 512)
                {
                    memcpy(AudioUnencodeBuf, (uint8 *)AudioDecodeIntputPtr+AudioEncodeCnt, 512);
                    TotalCntTemp = sbc_enc(NULL,AudioUnencodeBuf,512 ,AudioSbcOutPut+sbcTotalCnt, &FrameSize);
                    sbcTotalCnt += TotalCntTemp;
                    AudioEncodeCnt += 512;
                    AudioDecodeCount -=512;
                }
            }
        }
        if(AudioDecodeCount < 512)
        {
            if((512-AudioEncodeLeftCnt) > AudioDecodeCount)
            {
                memcpy(AudioUnencodeBuf+AudioEncodeLeftCnt, (uint8 *)AudioDecodeIntputPtr+AudioEncodeCnt, AudioDecodeCount);
                AudioEncodeLeftCnt += AudioDecodeCount;
                AudioDecodeCount = 0;
                AudioEncodeCnt = 0;

                if(pAudioRegKey->samplerate != 44100 && AudioResampleIndex < AudioLen*4)
                {
                    int out_len;
                    int int_len;
                    int_len = AUDIO_RESAMPLE_INPUT_PRE/2;

                    out_len = AUDIO_RESAMPLE_BUF_LEN/2;
                    resampler_process(NULL, (short *)AudioPtr+ AudioResampleIndex/2, &int_len, (short *)AudioResampleOutBuf, &out_len);
                    AudioResampleIndex += AUDIO_RESAMPLE_INPUT_PRE;

                    AudioDecodeIntputPtr = AudioResampleOutBuf;
                    AudioDecodeCount = out_len*2;
                }
            }

        }

        if(sbcTotalCnt > SBC_FRAME_NUM_PRE* FrameSize)
        {
            break;
        }
    }

}

_ATTR_AUDIO_TEXT_
void AudioSbcEncodeRequest()
{
    IntPendingSet(BT_SBC_PROCESS_INT_ID);
}

_ATTR_AUDIO_TEXT_
void AudioSbcSendComplete(void)
{
    if (AUDIO_STATE_PLAY == AudioPlayState)
    {
        AudioSbcEncodeRequest();
    }
}
_ATTR_AUDIO_TEXT_
void AudioSbcSendPro(void)
{
    uint16 TotalCntTemp;
    int ret = 0;
    if(sbcTotalCnt == 0 || gSysConfig.BtConfig.btConnected == 0 || AUDIO_STATE_PLAY != AudioPlayState || gSysConfig.AudioOutputMode != 1)
    {
        sbcTotalCnt = 0;
        return ;
    }

    if(sbcTotalCnt/FrameSize >= SBC_FRAME_NUM_PRE)
    {
        ret = avdtp_send_media(AudioSbcOutPut,SBC_FRAME_NUM_PRE*FrameSize, SBC_FRAME_NUM_PRE, AudioSbcSendComplete);

        if(ret == 0)
        {
            sbcTotalCnt -= SBC_FRAME_NUM_PRE*FrameSize;
            if(sbcTotalCnt > 0)
            memmove(AudioSbcOutPut,AudioSbcOutPut +FrameSize*SBC_FRAME_NUM_PRE, sbcTotalCnt);
        }
        AudioSbcEncodeProc();
    }


}

_ATTR_AUDIO_TEXT_
void AudioSbcEncodeInit(void)
{
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {

        uint frameCnt;
        uint TotalCntTemp = 0;
        uint left_cnt;
        uint buf_need = 0;
        uint i;
        short *vol_set_temp;
        int  pointCnt;
        extern BOOL avrcp_volume_manage;
        #ifdef MP3_INCLUDE
        if(CurrentCodec == CODEC_MP3)
        {
            //mp3_wait_synth();
        }
        #endif

        AudioResampleIndex = 0;
#ifdef _AVRCP_VERSION_1_4_
        if (avrcp_volume_manage == FALSE)
        {
            vol_set_temp = (short *)AudioPtr;
            pointCnt = AudioLen*2;

            for(i=0; i< pointCnt; i++)
            {
               vol_set_temp[i] = (int)vol_set_temp[i]*AudioPlayInfo.playVolume/MAX_VOLUME;
            }
        }
#else
#if 0
        vol_set_temp = (short *)AudioPtr;
        pointCnt = AudioLen*2;
        for(i=0; i< pointCnt; i++)
        {
            vol_set_temp[i] = (int)vol_set_temp[i] * MAX_VOLUME/(MAX_VOLUME+1);
        }
#endif
#endif
        if(pAudioRegKey->samplerate != 44100)
        {
            int out_len;
            int int_len;
            int_len = AUDIO_RESAMPLE_INPUT_PRE/2;

            out_len = AUDIO_RESAMPLE_BUF_LEN/2;

            if(AudioResampleInit == 0)
            {
                AudioResampleInit = 1;
                resample_init(pAudioRegKey->channels, pAudioRegKey->samplerate, 44100);
            }
            resampler_process(NULL, (short *)AudioPtr, &int_len, (short *)AudioResampleOutBuf, &out_len);
            AudioResampleIndex += AUDIO_RESAMPLE_INPUT_PRE;

            AudioDecodeIntputPtr = AudioResampleOutBuf;

            AudioDecodeCount = out_len*2;
        }
        else
        {
            AudioDecodeIntputPtr = (uint8 *)AudioPtr;

            AudioDecodeCount = AudioLen*4 ;
        }

        AudioEncodeCnt = 0;
        AudioSbcEncodeProc();

        if(sbcTotalCnt/FrameSize >= SBC_FRAME_NUM_PRE)
        {
            int ret = 0;
            ret = avdtp_send_media(AudioSbcOutPut,SBC_FRAME_NUM_PRE*FrameSize, SBC_FRAME_NUM_PRE, AudioSbcSendComplete);
            if(ret == 0)
            {
                sbcTotalCnt -= SBC_FRAME_NUM_PRE*FrameSize;
                if(sbcTotalCnt > 0)
                memmove(AudioSbcOutPut,AudioSbcOutPut +FrameSize*SBC_FRAME_NUM_PRE, sbcTotalCnt);
            }
            AudioSbcEncodeProc();
        }
    }
}


_ATTR_AUDIO_TEXT_
void AudioSbcEncodeStart(void)
{
    DEBUG("AudioSbcEncodeStart");
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        FREQ_EnterModule(FREQ_SBC_ENCODING);
        sbc_enc_init();

        if(pAudioRegKey->samplerate != 44100)
        {
            #ifdef SSRC
            AudioResampleIndex = 0;

            AudioResampleInit = 1;

            resample_init(pAudioRegKey->channels, pAudioRegKey->samplerate, 44100);

            #else
            SendMsg(MSG_AUDIO_DECODE_END);
            return;

            #endif
        }
        sbcTotalCnt = 0;

        AudioEncodeCnt = 0;
        AudioEncodeLeftCnt = 0;

        AudioDecodeCount = 0;

        avdtp_start();
    }
}


_ATTR_AUDIO_TEXT_
void AudioSbcEncodeStop(void)
{
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        avdtp_suspend();
        DelayMs(500);

        #ifdef SSRC
        if(pAudioRegKey->samplerate != 44100)
        {
            AudioResampleInit = 0;
        }
        #endif

        FREQ_ExitModule(FREQ_SBC_ENCODING);
    }
    DEBUG("AudioSbcEncodeStop");
}


_ATTR_AUDIO_TEXT_
void AudioSbcEncodeIntProecss(void)
{
    if(gSysConfig.BtConfig.btConnected)
    {
        AudioSbcSendPro();
    }
}

_ATTR_AUDIO_TEXT_
static void BluetoothReConnectResult(int result)
{
    BtWinStatus = BT_WIN_STATUS_IDLE;

    if (result == 0)
    {
        //DEBUG("###### ReConnect BT Success ######");
        //SendMsg(MSG_BLUETOOTH_CONNECT_SUCCEED);
        //连接成功
    }
    else
    {
        //DEBUG("****** ReConnect BT Fail ******");
        //SendMsg(MSG_BLUETOOTH_CONNECT_FAIL);
        //连接失败
    }
}

#endif


#if 1
_ATTR_AUDIO_DATA_
__align(4)
unsigned char DecDataBuf[2][AUDIO_FILE_PIPO_BUF_SIZE] = {0};
_ATTR_AUDIO_DATA_
unsigned char DecBufID = 0;
#endif

uint32 AudioCodecGetBufferSize(int codec, int samplerate);
BOOLEAN     AudioGetNextMusic_NP(UINT32 msg);
//extern void avctp_player_status_changed(uint8 player_status);
//extern void avctp_track_changed();

//_ATTR_AUDIO_BSS_  FILE_SAVE_STRUCT AudioID3Info;
/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN AudioIsPlaying(void)
  Author        : zs
  Description   : judge whether is in playing status.
  Input         : null
  Return        : 1:playing;
                  0:no playing.
  History       : <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioGetNextMusic(UINT32 msg)
{
    BOOLEAN ret = TRUE;
    uint32  Playednumber;

    switch (pAudioRegKey->RepeatMode)
    {
        case AUDIO_REPEAT://single repeat
        case AUDIO_REPEAT1:
            if (AUDIO_STOP_FORCE == AudioStopMode)
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);
            }
            break;

        case AUDIO_TRY: //ok lyrics
        case AUDIO_ALLONCE://only once all song play cycle.
            Playednumber = AudioFileInfo.CurrentFileNum;
            if (pAudioRegKey->PlayOrder == AUDIO_RAND)
            {
                Playednumber = AudioPlayFileNum;
            }

            if ((Playednumber >= AudioFileInfo.TotalFiles ) && (AudioStopMode != AUDIO_STOP_FORCE))
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum = 1;
                }
                AudioStart();

                //SCH
                CodecSeek(0, 0);
                if (pAudioRegKey->CurrentTime >= pAudioRegKey->TotalTime)
                {
                    pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
                }

                AudioPause();

                CurrentTimeSecBk = 0xffffffff;
                return TRUE;
            }
            else
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum += (INT16)msg;
                    if (AudioPlayFileNum < 1)
                    {
                        AudioPlayFileNum = AudioFileInfo.TotalFiles;
                    }
                }
            }
            break;

        case AUDIO_ALLREPEAT://all cycle play
            AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_ALL;
            SysFindFile(&AudioFileInfo,(INT16) msg);
            break;

        case AUDIO_ONCE: //single once.
            if (AUDIO_STOP_FORCE == AudioStopMode)
            {
                SysFindFile(&AudioFileInfo,(INT16) msg);
                break;
            }
            else
            {
                AudioStart();
                //SCH
                CodecSeek(0, 0);
                if (pAudioRegKey->CurrentTime >= pAudioRegKey->TotalTime)
                {
                    pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
                }
                AudioPause();

                CurrentTimeSecBk = 0xffffffff;
                return TRUE;
            }
            break;

        case AUDIO_FOLDER_ONCE://directory once.
            AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_DIR;

            Playednumber = AudioFileInfo.CurrentFileNum;
            if (pAudioRegKey->PlayOrder == AUDIO_RAND)
            {
                Playednumber = AudioPlayFileNum;
            }
            if ((Playednumber >= AudioFileInfo.TotalFiles)&&(AudioStopMode != AUDIO_STOP_FORCE))
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum = 1;
                }

                AudioPlayerState = AUDIO_STATE_PAUSE;

                AudioStart();
                if(1 == AudioCodecOpenErr)
                {
                    printf("\n ######## AudioCodec Open Error ########\n");
                    SysFindFile(&AudioFileInfo,(INT16)msg);
                    return FALSE;
                }
                //SCH
                CodecSeek(0, 0);
                if (pAudioRegKey->CurrentTime >= pAudioRegKey->TotalTime)
                {
                    pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
                }
                AudioPause();
                CurrentTimeSecBk = 0xffffffff;

                return TRUE;
            }
            else
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum += (INT16)msg;
                    if (AudioPlayFileNum < 1)
                    {
                        AudioPlayFileNum = AudioFileInfo.TotalFiles;
                    }
                }
//                if(1 == AudioCodecOpenErr)
//                {
//                    if(AudioPlayerState == AUDIO_STATE_FFD ||AudioPlayerState == AUDIO_STATE_FFW)
//                    {
//                        AudioPlayerState = AUDIO_STATE_PLAY;
                        //printf("\n AudioGetNextMusic: change audio state: ffd /ffw ---> play\n");
//                    }
//                }
            }
            break;

        case AUIDO_FOLDER_REPEAT://directory cycle.
            AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_DIR;
            SysFindFile(&AudioFileInfo,(INT16)msg);
            break;

        default:
            ret = FALSE;
            break;
    }

    AudioStart();
    return ret;
}


/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN GetMusicUIState(void)
  Author        : zs
  Description   : get the audio window status.
  Input         : null
  Return        : 1: no presence
                  0: presence
  History       : <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN GetMusicUIState(void)
{
    return MusicUIState;
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN SetMusicUIState(void)
  Author        : zs
  Description   : set the audio window status.
  Input         : null
  Return        : 1: no presence
                  0: presence
  History       : <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void SetMusicUIState(bool State)
{
    MusicUIState = State;
}
/*
--------------------------------------------------------------------------------
  Function name : void AudioSetVolume(void)
  Author        : zs
  Description   : audio output volume set,because the audio volume is control by audio effect indenpently,
                  so package this function in here.
  Input         : null
  Return        : null
  History       :<author>         <time>         <version>
                   zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioSetVolume(void)
{
    AudioInOut_Type  *pAudio  = &AudioIOBuf;
    RKEffect         *pEffect = &pAudio->EffectCtl;

    OutputVolOffset = 0;


#ifdef _A2DP_SOUCRE_


    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1 && btAvrcpVolumeChanged == 0)
    {

        if(IsRemoteDevSupportV14VolumeSet())
        {
            #ifdef _AVRCP_VERSION_1_4_
            avrcp_set_absolute_volume(MusicOutputVol*127/32);
            #endif
        }
        else
        {
            if(AudioPlayInfo.playVolume > MusicOutputVol)
            {

                ct_volumedown();
            }
            else
            {

                ct_volumeup();
            }
        }
    }


#endif
    AudioPlayInfo.playVolume = MusicOutputVol;

}

/*
--------------------------------------------------------------------------------
  Function name : void AudioSetEQ(void)
  Author        : zs
  Description   : stop repeat read.to call it after audio initialization and every EQ changed.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioSetEQ(int vol)
{
    int max_DbGain = 0;
    AudioInOut_Type  *pAudio  = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;

#ifdef  _RK_EQ_
    if (VOLTAB_CONFIG == VOL_General)
    {
        pEffect->max_DbGain = 12;
        if (pEffect->Mode == EQ_BASS)
        {
            if (vol <= 27 )
            {
                pEffect->max_DbGain = 12;
            }
            else if (vol == 28 )
            {
                pEffect->max_DbGain = 10;
            }
            else if (vol == 29 )
            {
                pEffect->max_DbGain = 8;
            }
            else if (vol >= 30 )
            {
                pEffect->max_DbGain = 6;
            }
        }
    }
    else
    {
        pEffect->max_DbGain = 12;

        if (pEffect->Mode == EQ_BASS)
        {
            if (vol <= 22 )
            {
                pEffect->max_DbGain = 12;
            }
            else if (vol == 23 )
            {
                pEffect->max_DbGain = 10;
            }
            else if (vol == 24 )
            {
                pEffect->max_DbGain = 8;
            }
            else if (vol == 25 )
            {
                pEffect->max_DbGain = 6;
            }
            else if (vol == 26 )
            {
                pEffect->max_DbGain = 5;
            }
            else if (vol == 27 )
            {
                pEffect->max_DbGain = 3;
            }
            else if (vol == 28 )
            {
                pEffect->max_DbGain = 2;
            }
            else if (vol == 29 )
            {
                pEffect->max_DbGain = 1;
            }
            else if (vol >= 30 )
            {
                pEffect->max_DbGain = 0;
            }
        }

        if ((pEffect->Mode == EQ_JAZZ) || (pEffect->Mode == EQ_UNIQUE))
        {
            if (vol <= 25 )
            {
                pEffect->max_DbGain = 6;
            }
            else if (vol == 26 )
            {
                pEffect->max_DbGain = 5;
            }
            else if (vol == 27 )
            {
                pEffect->max_DbGain = 3;
            }
            else if (vol == 28 )
            {
                pEffect->max_DbGain = 2;
            }
            else if (vol == 29 )
            {
                pEffect->max_DbGain = 1;
            }
            else if (vol >= 30 )
            {
                pEffect->max_DbGain = 0;
            }
        }

        if (pEffect->Mode == EQ_POP)
        {
            if (vol <= 27 )
            {
                pEffect->max_DbGain = 3;
            }
            else if (vol == 28 )
            {
                pEffect->max_DbGain = 2;
            }
            else if (vol == 29 )
            {
                pEffect->max_DbGain = 1;
            }
            else if (vol >= 30 )
            {
                pEffect->max_DbGain = 0;
            }
        }
    }

    EffectAdjust();
    AudioSetVolume();
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : UINT8 GetFileType(UINT8 *pBuffer, UINT8 *pStr)
  Author        : zs
  Description   : get file type.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_AUDIO_INIT_TEXT_
_ATTR_FAT_FIND_CODE_
UINT8 GetFileType(UINT8 *pBuffer, UINT8 *pStr)
{
    UINT8 Len;
    UINT8 Retval = 0xff;
    UINT8 i;
    i = 0;
    Len = strlen((char*)pStr);

    while (i <= Len)
    {
        i += 3;

        if ((*(pBuffer + 0) == *(pStr + 0)) && (*(pBuffer + 1) == *(pStr + 1)) &&
            (*(pBuffer + 2) == *(pStr + 2)))
        {
            break;
        }

        pStr += 3;
    }

    if (i <= Len)
    {
        Retval = i / 3;
    }

    return (Retval);
}

/*
--------------------------------------------------------------------------------
  Function name : void AudioSetEQ(void)
  Author        : zs
  Description   : set current codec type.
  Input         : Codec_Type
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_INIT_TEXT_
void AudioCodec(UINT8 *pBuffer, UINT8 *pStr)
{
    UINT8 temp;
    temp = GetFileType(pBuffer, pStr);

    switch (temp)
    {
        case 1:     //mp3
        case 2:     //mp2
        case 3:     //mp1
#ifdef MP3_DEC_INCLUDE
            CurrentCodec = CODEC_MP3_DEC;
#endif
            break;

        case 4:     //wma
#ifdef  WMA_DEC_INCLUDE
            CurrentCodec = CODEC_WMA_DEC;
#endif
            break;

        case 5:     //wav
#ifdef WAV_DEC_INCLUDE
            CurrentCodec = CODEC_WAV_DEC;
#endif
            break;

        case 6:     //ape
            CurrentCodec = CODEC_REVERSED_4;
            break;

        case 7:     //flac
#ifdef FLAC_DEC_INCLUDE
            CurrentCodec = CODEC_FLAC_DEC;
#endif
            break;

        case 8:     //AAC
        case 9:     //M4A
        case 11:    //MP4
        case 12:    // 3GP
#ifdef AAC_DEC_INCLUDE
            CurrentCodec = CODEC_AAC_DEC;
#endif
            break;

        case 10:    //OGG
            CurrentCodec = CODEC_REVERSED_6;
            break;

        default:
            CurrentCodec = 0xff;
            break;
    }
}
_ATTR_AUDIO_INIT_TEXT_
static unsigned int CheckID3V2Tag(unsigned  char *pucBuffer)
{
    // The first three bytes of the tag should be "ID3".
    if ((pucBuffer[0] !=    'I') || (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next byte should be the value 3 (i.e. we support ID3v2.3.0).
    //if(pucBuffer[3]   != 3)
    if (pucBuffer[3] < 2 && pucBuffer[3] > 4)
    {
        return(0);
    }

    // The next byte should be less than 0xff.
    if (pucBuffer[4] == 0xff)
    {
        return(0);
    }

    // We don't care about the next byte.  The following four bytes should be
    // less than 0x80.
    if ((pucBuffer[6] >= 0x80) || (pucBuffer[7] >= 0x80)    ||
            (pucBuffer[8] >= 0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of the ID3v2 tag.
    return((pucBuffer[6] << 21) | (pucBuffer[7] << 14) |
           (pucBuffer[8] <<  7) |  pucBuffer[9]);
}

_ATTR_SYS_CODE_
void AudioCheckStreamType(UINT8 *pBuffer, FILE * hFile)
{
    uint8 char_buf[512];
    uint8 *buf = char_buf;
    int ID3_Length ;
    int  Redundancy_len = 0;
    RKFIO_FRead(char_buf, 512, hFile);
    ID3_Length = CheckID3V2Tag(buf);

    if (ID3_Length)
    {
        //printf("\naudio auto anlayse == id3\n");
        ID3_Length += 10;

        if (ID3_Length < (512 - 17))
        {
            buf += ID3_Length;
            Redundancy_len =  ID3_Length;
        }
        else
        {
            RKFIO_FSeek(ID3_Length, 0, hFile);
            RKFIO_FRead(char_buf, 512, hFile);
        }

        //printf("ID3 len = 0x%x \n",ID3_Length);
    }
    while (1)
    {
        if ((buf[0] == 0x30) && (buf[1] == 0x26) && (buf[2] == 0xB2))
        {
            //printf("\naudio auto anlayse == wma\n");
            *pBuffer++ = 'W';
            *pBuffer++ = 'M';
            *pBuffer++ = 'A';
            break;
        }
        else if (((*(uint16 *)buf) & 0xe0ff) == 0xe0ff &&((buf[15] !='a') &&(buf[16] !='a') &&(buf[17] !='c')))
        {
            uint32 framelen, framesec, frameoffset;

            uint32 i;
            uint32 hit_cnt = 0;
            uint32 framesumlen;
            framelen = ((uint16)((buf[3] & 0x03) << 11)) | ((uint16)((buf[4])  << 3)) | ((uint16)(buf[5] >> 5));
            framesumlen = framelen;

            //printf("\naudio auto anlayse == maybe no.id3.mp3, framesize = %d\n", framelen);
REPEAT:
            if (framelen == 0)
            {
                //printf("\naudio auto anlayse == no.id3.mp3\n");

                *pBuffer++ = 'M';
                *pBuffer++ = 'P';
                *pBuffer++ = '3';
                break;
            }
            else
            {
                RKFIO_FSeek(ID3_Length+framesumlen, SEEK_SET, hFile);
                RKFIO_FRead(char_buf, 512, hFile);
                buf = char_buf;
                frameoffset = 0;

                if (((buf[frameoffset] & 0xff) == 0xff)  && ((buf[frameoffset+1] & 0xf0) == 0xf0)&&((buf[frameoffset+3] & 0x03) !=0x03))
                {
                    //printf("\naudio auto anlayse == AAC\n");
                    *pBuffer++ = 'A';
                    *pBuffer++ = 'A';
                    *pBuffer++ = 'C';
                    pBuffer -= 3;
                    framelen = ((uint16)((buf[frameoffset+3] & 0x03) << 11)) | ((uint16)((buf[frameoffset+4])  << 3)) | ((uint16)(buf[frameoffset+5] >> 5));
                    framesumlen += framelen;
                    hit_cnt++;
                    //printf("\n ramesize = %d framesumlen=%d frameoffset=%d\n", framelen,framesumlen,frameoffset);
                    if(hit_cnt > 3)
                    {
                        printf("\n repeat three time ,audio auto anlayse == AAC\n");
                        *pBuffer++ = 'A';
                        *pBuffer++ = 'A';
                        *pBuffer++ = 'C';

                        break;
                    }

                    goto REPEAT;
                }
                else
                {
                    //printf("\naudio auto anlayse == no.id3.mp3\n");

                    *pBuffer++ = 'M';
                    *pBuffer++ = 'P';
                    *pBuffer++ = '3';
                    break;
                }
            }


        }
        else if ((buf[4] == 'f') && (buf[5] == 't') && (buf[6] == 'y') && (buf[7] == 'p'))
        {
            //printf("\naudio auto anlayse == M4A\n");
            *pBuffer++ = 'M';
            *pBuffer++ = '4';
            *pBuffer++ = 'A';
            break;
        }
        else if ((buf[9] == 0) && (buf[10] == 0) && (buf[11] == 'l') && (buf[12] == 'i') &&
                 (buf[13] == 'b') && (buf[14] == 'f') && (buf[15] == 'a') && (buf[16] == 'a') && (buf[17] == 'c'))
        {
            //printf("\naudio auto anlayse == AAC\n");
            *pBuffer++ = 'A';
            *pBuffer++ = 'A';
            *pBuffer++ = 'C';
            break;
        }
        else if ((buf[0] == 'R') && (buf[1] == 'I') && (buf[2] == 'F') && (buf[3] == 'F'))
        {
            //printf("\naudio auto anlayse == WAV\n");
            *pBuffer++ = 'W';
            *pBuffer++ = 'A';
            *pBuffer++ = 'V';
            break;
        }
        else if ((buf[0] == 'f') && (buf[1] == 'L') && (buf[2] == 'a') && (buf[3] == 'C'))
        {
            //printf("\naudio auto anlayse == flac\n");
            *pBuffer++ = 'F';
            *pBuffer++ = 'L';
            *pBuffer++ = 'A';
            break;
        }
        else if ((buf[0] == 'M') && (buf[1] == 'A') && (buf[2] == 'C'))
        {
            //printf("\naudio auto anlayse == ape\n");
            *pBuffer++ = 'A';
            *pBuffer++ = 'P';
            *pBuffer++ = 'E';
            break;
        }
        else if ((buf[0] == 'O') && (buf[1] == 'g') && (buf[2] == 'g') && (buf[3] == 'S'))
        {
            //printf("\naudio auto anlayse == ogg\n");
            *pBuffer++ = 'O';
            *pBuffer++ = 'G';
            *pBuffer++ = 'G';
            break;
        }
        else
        {
            buf++;
            Redundancy_len++;
            if (Redundancy_len == 512)
            {
                int ret = RKFIO_FRead(char_buf, 512, hFile);
                if (ret < 512)
                {
                    DEBUG("Don't know file type");
                    break;
                }
                buf = char_buf;
                Redundancy_len = 0;
            }
            else if (Redundancy_len == 1024)
            {
                DEBUG("Don't know file type");
                break;
            }

        }
    }

    RKFIO_FSeek(0, SEEK_SET, hFile);

}

/*
--------------------------------------------------------------------------------
  Function name :void AudioVariableInit(void)
  Author        : zyz
  Description   : initialization of variable.
  Input         : null
  Return        : null
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
//_ATTR_AUDIO_BSS_  DecodingError;
_ATTR_AUDIO_INIT_TEXT_
void AudioVariableInit(void)
{
    //AudioIOBuf.lOutLength = 1152;
    AudioPlayInfo.PlayDirect = 1;
    AudioStopMode = AUDIO_STOP_NORMAL;
    pAudioRegKey->CurrentTime = 0;
    pAudioRegKey->TotalTime = 1;
    pAudioRegKey->IsEQUpdate = 0;

    //DecodingError      = 0;
    AudioErrorFrameNum = 0;
    AudioFileSeekOffset = 0;
    OutputVolOffset = 0;

    AudioPlayInfo.VolumeCnt = 0;

    AudioEndFade = 0;

    CurrentCodec = 0xff;

//    gAudioPlayTime = 0;
    CurrentTimeSecBk = 0xffffffff;

    audio_dec_album_save = 0;
    audio_dec_album_done = 0;
    audio_have_album_pic = 0;
    AudioAlbumHandle = (FILE*)-1;

    memset(&gpMediaBlock,0,sizeof(MediaBlock));

    ClearMsg(MSG_AUDIO_DECODE_END);
}

/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioVariableDeInit(UINT16 ReqType);
  Author        :  zs
  Description   :
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_INIT_TEXT_
void AudioVariableDeInit(void)
{
    AudioPlayInfo.ABRequire = AUDIO_AB_NULL;

    ClearMsg(MSG_MUSIC_DISPFLAG_SCHED);
    ClearMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME);
    ClearMsg(MUSIC_UPDATESPECTRUM);
    ClearMsg(MSG_MUSIC_DISPFLAG_SCROLL_FILENAME);
}

/*
--------------------------------------------------------------------------------
  Function name :uint32 AudioFileOpen(void)
  Author        : zyz
  Description   :
  Input         : null
  Return        : null
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_INIT_TEXT_
uint32 AudioFileOpen(void)
{
    FS_TYPE FsType;
    int CurrentCodecSave;

    if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        FsType = MUSIC_DB;
    }
#ifdef _RECORD_
    else if (AudioFileInfo.ucSelPlayType == MUSIC_TYPE_SEL_RECORDFILE)
    {
        FsType = RECORD_DB;
    }
#endif
    else
    {
        FsType = FS_FAT;
    }

    pRawFileCache = (FILE*)-1;
    pRawFileCache = (FILE*)FileOpen(AudioFileInfo.Fdt.Name, AudioFileInfo.FindData.Clus, AudioFileInfo.FindData.Index - 1, FsType, FileOpenStringR);
    DEBUG("FileName = %s",AudioFileInfo.Fdt.Name);
    DEBUG("FileNum = %d", AudioFileInfo.CurrentFileNum);

    #ifdef _A2DP_SOUCRE_
    {
        HANDLE fd;
        int buf[512];
        int len;
        int utf8_len;
        ModuleOverlay(MODULE_ID_AUDIO_ID3,MODULE_OVERLAY_ALL);
        memset(buf,0,512);
        fd = FileOpen(AudioFileInfo.Fdt.Name, AudioFileInfo.FindData.Clus, AudioFileInfo.FindData.Index - 1, FsType, FileOpenStringR);
        if(fd != NOT_OPEN_FILE)
        {
            memset(&gAudioId3Info,0,sizeof(ID3V2X_INFO));
            GetAudioId3Info(fd, &gAudioId3Info, &AudioFileInfo.Fdt.Name[8]);
            #if 0
            len = get_unicode_len(&gAudioId3Info.id3_title, 256);
            utf8_len = UnicodeToUTF_8(&gAudioId3Info.id3_title, len, buf,512);
            printf("gAudioId3Info.title = %s\n",buf);
            len = get_unicode_len(&gAudioId3Info.id3_singer, 256);
            utf8_len = UnicodeToUTF_8(&gAudioId3Info.id3_singer, len, buf,512);
            printf("gAudioId3Info.id3_singer = %s\n",buf);
            len = get_unicode_len(&gAudioId3Info.id3_album, 256);
            utf8_len = UnicodeToUTF_8(&gAudioId3Info.id3_album, len, buf,512);
            printf("gAudioId3Info.id3_album = %s\n",buf);
            len = get_unicode_len(&gAudioId3Info.id3_genre, 256);
            utf8_len = UnicodeToUTF_8(&gAudioId3Info.id3_genre, len, buf,512);
            printf("gAudioId3Info.id3_genre = %s\n",buf);
            #endif
            FileClose(fd);
        }
        else
        {
            printf("id3 open file fail\n");
        }
    }
    #endif
    if ((uint32)pRawFileCache > MAX_OPEN_FILES)
    {
        DEBUG("ERROR!!! pRawFileCache = %d,Open fail ",pRawFileCache);
        return ERROR;
    }

    AudioCodec(&AudioFileInfo.Fdt.Name[8],(UINT8 *)AudioFileExtString);
    CurrentCodecSave = CurrentCodec;
    //DEBUG(" CurrentCodec = %d ",CurrentCodec);

    RKFileFuncInit();

    AudioCheckStreamType(&AudioFileInfo.Fdt.Name[8], pRawFileCache);
    AudioCodec(&AudioFileInfo.Fdt.Name[8],(UINT8 *)AudioFileExtString);

    if ((CurrentCodec == 0xff) && (CurrentCodecSave == 0xff))
    {
        printf("ERROR!!! CurrentCodec == 0x%02x \n",CurrentCodec);
        return ERROR;
    }

#ifdef FLAC_DEC_INCLUDE

    if (CurrentCodec == CODEC_FLAC_DEC)
    {
        ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
        RKFileFuncInit();
        FileSeek(0, SEEK_SET, (HANDLE)pRawFileCache);
    }

    if (CODEC_FLAC_DEC == CurrentCodec)
    {
        pFlacFileHandleBake = (FILE*)FileOpen(AudioFileInfo.Fdt.Name, AudioFileInfo.FindData.Clus, AudioFileInfo.FindData.Index - 1, FsType, FileOpenStringR);

        if ((uint32)pFlacFileHandleBake > MAX_OPEN_FILES)
        {
            DEBUG("ERROR!!! pFlacFileHandleBake = %d,Open fail", pFlacFileHandleBake);
            return ERROR;
        }
    }

#endif


#ifdef AAC_DEC_INCLUDE
    if (CurrentCodec == CODEC_AAC_DEC)
    {
        ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
        RKFileFuncInit();
        FileSeek(0, SEEK_SET, (HANDLE)pRawFileCache);
    }

    if (CODEC_AAC_DEC == CurrentCodec)
    {
        pAacFileHandleSize = (FILE*)FileOpen(AudioFileInfo.Fdt.Name, AudioFileInfo.FindData.Clus,AudioFileInfo.FindData.Index - 1, FsType, FileOpenStringR);
        pAacFileHandleOffset = (FILE*)FileOpen(AudioFileInfo.Fdt.Name, AudioFileInfo.FindData.Clus,AudioFileInfo.FindData.Index - 1, FsType, FileOpenStringR);
        if ((uint32)pAacFileHandleSize > MAX_OPEN_FILES)
        {
            DEBUG("ERROR!!! pAacFileHandleSize = %d, Open fail",pAacFileHandleSize);
            return ERROR;
        }
        if ((uint32)pAacFileHandleOffset > MAX_OPEN_FILES)
        {
            DEBUG("ERROR!!! pAacFileHandleOffset = %d, Open fail",pAacFileHandleOffset);
            return ERROR;
        }
    }
#endif

    GetLongFileName(AudioFileInfo.FindData.Clus, AudioFileInfo.FindData.Index - 1, FsType, MusicLongFileName);

    return OK;
}

/*
--------------------------------------------------------------------------------
  Function name :uint32 AudioFileClose(void)
  Author        : zyz
  Description   :
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_INIT_TEXT_
uint32 AudioFileClose(void)
{
    RKFIO_FClose((FILE*)pRawFileCache);
    pRawFileCache = (FILE*)-1;

#ifdef FLAC_DEC_INCLUDE

    if (CurrentCodec == CODEC_FLAC_DEC)
    {
        RKFIO_FClose((FILE*)pFlacFileHandleBake);
        pFlacFileHandleBake = (FILE*) - 1;
    }

#endif

#ifdef AAC_DEC_INCLUDE
    if (CurrentCodec == CODEC_AAC_DEC)
    {
        RKFIO_FClose((FILE*)pAacFileHandleSize);
        pAacFileHandleSize = (FILE*)-1;

        RKFIO_FClose((FILE*)pAacFileHandleOffset);
        pAacFileHandleOffset = (FILE*)-1;
    }
#endif

}

/*
--------------------------------------------------------------------------------
  Function name :void AudioCodecOpen(void)
  Author        : zyz
  Description   :
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint32 AudioCodecOpen(void)
{
    if (1 != CodecOpen(0, CODEC_OPEN_DECODE))
    {
        return ERROR;
    }

    CodecGetLength(&pAudioRegKey->TotalTime);
    CodecGetSampleRate(&pAudioRegKey->samplerate);

    CodecGetBitrate(&pAudioRegKey->bitrate);
    CodecGetChannels(&pAudioRegKey->channels);
    CodecGetBps(&pAudioRegKey->bps);
    rk_printf("bps = %d fs = %d bitrate = %d time=%dm:%ds %d ms",pAudioRegKey->bps,pAudioRegKey->samplerate,pAudioRegKey->bitrate,pAudioRegKey->TotalTime/60000,pAudioRegKey->TotalTime/1000%60,pAudioRegKey->TotalTime%1000);

    if ((pAudioRegKey->bitrate <= 0)||(pAudioRegKey->samplerate <= 0))
    {
        return ERROR;
    }

    if (pAudioRegKey->samplerate > 48000)
    {
        return ERROR;
    }

    return OK;
}

/*
--------------------------------------------------------------------------------
  Function name :void AudioWaitBBStart(void)
  Author        :
  Description   : wait for BB core file open
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioWaitBBStart(void)
{
    uint32 timeout = 5000000;

    gACKDone = 0;
    gFileHandle.codecType = CurrentCodec;
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteA2BData((int)&gFileHandle,MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    while (!gACKDone)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        BBDebug();
        //__WFI();
        DelayUs(1);
        if (--timeout == 0)
        {
            DEBUG("AudioWaitBBStart: timeout!!!");
            break;
        }
    }
    gACKDone = 0;
}

/*
--------------------------------------------------------------------------------
  Function name :void AudioWaitBBStop(void)
  Author        :
  Description   : wait for BB core file close
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioWaitBBStop(void)
{
    uint32 timeout = 20000;
    gACKDone = 0;
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteA2BData(/*(int)&gFileHandle*/ 1,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    while (!gACKDone)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        BBDebug();
        //__WFI();
        DelayUs(1);
        if (--timeout == 0)
        {
            DEBUG("AudioWaitBBStop: timeout!!!");
            break;
        }
    }
    gACKDone = 0;
}

/*
--------------------------------------------------------------------------------
  Function name :void AudioCodecOpen(void)
  Author        : zyz
  Description   :
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioHoldonInit(void)
{
#ifdef AUDIOHOLDONPLAY
    if (gSysConfig.MusicConfig.HoldOnPlaySaveFlag == 1)
    {
        if (0 != gSysConfig.MusicConfig.CurTime)
        {
            pAudioRegKey->CurrentTime = gSysConfig.MusicConfig.CurTime;
            CodecSeek(pAudioRegKey->CurrentTime, 0);
        }
        gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
    }
#endif
}


/*
--------------------------------------------------------------------------------
  Function name : void AudioHWInit(void)
  Author        : zs
  Description   : audio hareware accelerator and CODEC initialization.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioHWInit(void)
{
    switch (CurrentCodec)
    {
#ifdef MP3_DEC_INCLUDE
        case CODEC_MP3_DEC:

            ModuleOverlay(MODULE_ID_MP3_DECODE, MODULE_OVERLAY_ALL);

            //clock gate&Reset Init
            ScuClockGateCtr(HCLK_SYNTH_GATE,1);
            ScuClockGateCtr(HCLK_IMDCT_GATE,1);
            ScuSoftResetCtr(IMDCT_SRST, 0);
            ScuSoftResetCtr(SYNTH_SRST, 0);

            {
                StartBBSystem(MODULE_ID_MP3_DECODE_BIN);

                //Others
                memset(&gFileHandle,0,sizeof(gFileHandle));
                gFileHandle.handle1 = (unsigned char)pRawFileCache;
                gFileHandle.filesize = RKFIO_FLength(pRawFileCache);
                gFileHandle.curfileoffset[0] = FileInfo[gFileHandle.handle1].Offset;

                AudioWaitBBStart();
            }
            break;
#endif

#ifdef  WMA_DEC_INCLUDE
        case CODEC_WMA_DEC:
            ModuleOverlay(MODULE_ID_WMA_COMMON, MODULE_OVERLAY_ALL);
            {
                //
                StartBBSystem(MODULE_ID_WMA_DECODE_BIN);

                //Others
                memset(&gFileHandle,0,sizeof(gFileHandle));
                gFileHandle.handle1 = (unsigned char)pRawFileCache;
                gFileHandle.filesize = RKFIO_FLength(pRawFileCache);
                gFileHandle.curfileoffset[0] = FileInfo[gFileHandle.handle1].Offset;

                AudioWaitBBStart();
            }
            break;
#endif

#ifdef  AAC_DEC_INCLUDE
        case CODEC_AAC_DEC:
            ModuleOverlay(MODULE_ID_AAC_DECODE, MODULE_OVERLAY_ALL);
            {
                //
                StartBBSystem(MODULE_ID_AAC_DECODE_BIN);

                //Others
                memset(&gFileHandle,0,sizeof(gFileHandle));
                gFileHandle.handle1 = (unsigned char)pRawFileCache;
                gFileHandle.filesize = RKFIO_FLength(pRawFileCache);
                gFileHandle.curfileoffset[0] = FileInfo[gFileHandle.handle1].Offset;

                gFileHandle.handle2 = (unsigned char)pAacFileHandleSize;
                gFileHandle.handle3 = (unsigned char)pAacFileHandleOffset;
                gFileHandle.curfileoffset[1] = FileInfo[gFileHandle.handle2].Offset;
                gFileHandle.curfileoffset[2] = FileInfo[gFileHandle.handle3].Offset;

                AudioWaitBBStart();
            }
            break;
#endif

#ifdef WAV_DEC_INCLUDE
        case CODEC_WAV_DEC:
            ModuleOverlay(MODULE_ID_WAV_DECODE, MODULE_OVERLAY_ALL);
            {
                //
                StartBBSystem(MODULE_ID_WAV_DECODE_BIN);

                //Others
                memset(&gFileHandle,0,sizeof(gFileHandle));
                gFileHandle.handle1 = (unsigned char)pRawFileCache;
                gFileHandle.filesize = RKFIO_FLength(pRawFileCache);
                gFileHandle.curfileoffset[0] = FileInfo[gFileHandle.handle1].Offset;

                AudioWaitBBStart();
            }
            break;
#endif

#ifdef FLAC_DEC_INCLUDE

        case CODEC_FLAC_DEC:
            ModuleOverlay(MODULE_ID_FLAC_DECODE, MODULE_OVERLAY_ALL);
            {
                //
                StartBBSystem(MODULE_ID_FLAC_DECODE_BIN);

                //Others
                memset(&gFileHandle, 0, sizeof(gFileHandle));
                gFileHandle.handle1 = (unsigned char)pRawFileCache;
                gFileHandle.filesize = RKFIO_FLength(pRawFileCache);
                gFileHandle.curfileoffset[0] = FileInfo[gFileHandle.handle1].Offset;
                gFileHandle.handle2 = (unsigned char)pFlacFileHandleBake;
                gFileHandle.curfileoffset[1] = FileInfo[gFileHandle.handle2].Offset;
                AudioWaitBBStart();
            }
            break;
#endif


        default:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void AudioHWInit(void)
  Author        : zs
  Description   : audio hareware accelerator and CODEC initialization.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioDeHWInit(void)
{
    switch (CurrentCodec)
    {
#ifdef MP3_DEC_INCLUDE
        case CODEC_MP3_DEC:
            {
                //...
                AudioWaitBBStop();
                ShutOffBBSystem();
            }

            ScuSoftResetCtr(IMDCT_SRST, 1);
            ScuSoftResetCtr(SYNTH_SRST, 1);
            ScuClockGateCtr(HCLK_SYNTH_GATE,0);
            ScuClockGateCtr(HCLK_IMDCT_GATE,0);

            break;
#endif

#ifdef  WMA_DEC_INCLUDE
        case CODEC_WMA_DEC:
            {
                //...
                AudioWaitBBStop();
                ShutOffBBSystem();
            }
            break;
#endif

#ifdef  AAC_DEC_INCLUDE
        case CODEC_AAC_DEC:
            {
                //...
                AudioWaitBBStop();
                ShutOffBBSystem();
            }
            break;
#endif

#ifdef WAV_DEC_INCLUDE
        case CODEC_WAV_DEC:
            {
                //...
                AudioWaitBBStop();
                ShutOffBBSystem();
            }
            break;
#endif


#ifdef FLAC_DEC_INCLUDE
        case CODEC_FLAC_DEC:
        {
            //...
            AudioWaitBBStop();
            ShutOffBBSystem();
        }
        break;
#endif

        default:
            break;
    }

}

/*
--------------------------------------------------------------------------------
  Function name :void AudioFREQInit(void)
  Author        : zyz
  Description   :
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFREQInit(void)
{
    switch (CurrentCodec)
    {
#ifdef MP3_DEC_INCLUDE
        case (CODEC_MP3_DEC):
            {
                if ((pAudioRegKey->bitrate <= 128000)&&(pAudioRegKey->samplerate <= 44100))
                {
                    FREQ_EnterModule(FREQ_MP3);
                }
                else
                {
                    FREQ_EnterModule(FREQ_MP3H);

                }
                break;
            }
#endif

#ifdef WMA_DEC_INCLUDE
        case (CODEC_WMA_DEC):
            {
                if (pAudioRegKey->bitrate < 128000)
                {
                    if ((pAudioRegKey->samplerate == FS_32KHz) &&(pAudioRegKey->bitrate/1000 == 22))
                    {
                        FREQ_EnterModule(FREQ_WMAH);
                    }
                    else if ((pAudioRegKey->samplerate == FS_44100Hz) &&(pAudioRegKey->bitrate/1000 == 48))
                    {
                        FREQ_EnterModule(FREQ_WMAH);
                    }
                    else
                        FREQ_EnterModule(FREQ_WMA);
                }
                else
                {
                    FREQ_EnterModule(FREQ_WMAH);
                }
                break;
            }
#endif

#ifdef FLAC_DEC_INCLUDE

        case (CODEC_FLAC_DEC):
            {
                DEBUG("ENTER FREQ_FLAC");
                FREQ_EnterModule(FREQ_FLAC);
                break;
            }

#endif


#ifdef AAC_DEC_INCLUDE
        case (CODEC_AAC_DEC):
            {
                if ((pAudioRegKey->bitrate <= 128000)&&(pAudioRegKey->samplerate <= 44100))
                {
                    FREQ_EnterModule(FREQ_AACL);
                }
                else
                {
                    FREQ_EnterModule(FREQ_AAC);
                }
                break;
            }
#endif

#ifdef WAV_DEC_INCLUDE
        case (CODEC_WAV_DEC):
            {
                FREQ_EnterModule(FREQ_WAV);
                break;
            }
#endif

    }
}

/*
--------------------------------------------------------------------------------
  Function name :void AudioFREQDeInit(void)
  Author        : zyz
  Description   : FREQ auti-initialization.
  Input         : null
  Return        : null
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_INIT_TEXT_
void AudioFREQDeInit(void)
{
    switch (CurrentCodec)
    {
#ifdef MP3_DEC_INCLUDE
        case (CODEC_MP3_DEC):
            {
                if ((pAudioRegKey->bitrate <= 128000)&&(pAudioRegKey->samplerate <= 44100))
                {
                    FREQ_ExitModule(FREQ_MP3);
                }
                else
                {
                    FREQ_ExitModule(FREQ_MP3H);
                }
                break;
            }
#endif

#ifdef WMA_DEC_INCLUDE
        case (CODEC_WMA_DEC):
            {
                if (pAudioRegKey->bitrate < 128000)
                {
                    if ((pAudioRegKey->samplerate == FS_32KHz) &&(pAudioRegKey->bitrate/1000 == 22))
                    {
                        FREQ_ExitModule(FREQ_WMAH);
                    }
                    else if ((pAudioRegKey->samplerate == FS_44100Hz) &&(pAudioRegKey->bitrate/1000 == 48))
                    {
                        FREQ_ExitModule(FREQ_WMAH);
                    }
                    else
                        FREQ_ExitModule(FREQ_WMA);
                }
                else
                {
                    FREQ_ExitModule(FREQ_WMAH);
                }
                break;
            }
#endif

#ifdef AAC_DEC_INCLUDE
        case (CODEC_AAC_DEC):
            {
                if ((pAudioRegKey->bitrate <= 128000)&&(pAudioRegKey->samplerate <= 44100))
                {
                    FREQ_ExitModule(FREQ_AACL);
                }
                else
                {
                    FREQ_ExitModule(FREQ_AAC);
                }
                break;
            }
#endif

#ifdef WAV_DEC_INCLUDE
        case (CODEC_WAV_DEC):
            {
                FREQ_ExitModule(FREQ_WAV);
                break;
            }
#endif

#ifdef FLAC_DEC_INCLUDE
        case (CODEC_FLAC_DEC):
        {
            FREQ_ExitModule(FREQ_FLAC);
            break;
        }

#endif

    }
}

#ifdef _LOG_DEBUG_
//this fun just for debuging
static uint8* Cmd2String(uint32 cmd)
{
    uint8* pstr;

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_DEC_OPEN_ERR:
            pstr = "[DEC OPEN ERROR]";
            break;

        case MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL:
            pstr = "[DEC OPEN CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CMPL:
            pstr = "[DECODE CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_DECODE_ERR:
            pstr = "[DECODE ERROR]";
            break;

        case MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL:
            pstr = "[DECODE GET_BUF CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL:
            pstr = "[DECODE SEEK CMPL";
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL:
            pstr = "[DECODE CLOSE CMPL]";
            break;
#if 0
        case MEDIA_MSGBOX_CMD_FILE_SEEK:
            pstr = "[FILE SEEK]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL:
            pstr = "[FILE SEEK CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_READ:
            pstr = "[FILE READ]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_READ_CMPL:
            pstr = "[FILE READ CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_TELL:
            pstr = "[FILE TELL]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_TELL_CMPL:
            pstr = "[FILE TELL CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH:
            pstr = "[FILE GET_LEN]";
            break;
        case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL:
            pstr = "[FILE GET_LEN CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL:
            pstr = "[FILE CLOSE CMPL]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_OPEN_HANDSHK:
            pstr = "[OPEN HANDSHAKE]";
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE_HANDSHK:
            pstr = "[CLOSE HANDSHAKE]";
            break;

        case MEDIA_MSGBOX_CMD_BBSYSTEM_OK:
            pstr = "[BB SYSTEM START OK]";
            break;
#endif
        default:
            pstr = "[NO FOUNDD CMD]";
            break;
    }

    return pstr;
}



#endif

/*
--------------------------------------------------------------------------------
  Function name :void AudioDecodingGetOutBuffer(void)
  Author        :
  Description   : Get decoding output data from calculate core
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    mlc             2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
ExecFunPtr AudioDecodingGetOutBuffer(void)
{
    uint32 cmd;
    uint32 data;

    cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    gDecCmd  = cmd;
    gDecData = data;

    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
    //rk_printf("B2A Decode Service cmd = %s,val = %d",Cmd2String(cmd),cmd);

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_DEC_OPEN_ERR:
            gError   = 1;
            gOpenDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL:
            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock));
            gpMediaBlock.DecodeOver = 1;
            gError   = 0;
            gOpenDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CMPL:
            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock));

            gpMediaBlock.DecodeOver = 1;
            gpMediaBlock.Decoding   = 0;

            if (AudioNeedDecode == 1)
            {
                UserIsrRequest();
            }

            AudioNeedDecode = 0;
            gDecDone = 1;

            break;

        case MEDIA_MSGBOX_CMD_DECODE_ERR:
            gpMediaBlock.DecodeOver = 1;
            gpMediaBlock.DecodeErr = 1;
            if (AudioNeedDecode == 1)
            {
                UserIsrRequest();
            }
            AudioNeedDecode = 0;
            gDecDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL:
            gSeekDone = 1;
            break;


        case MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL:
            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock));
            gGetTimeDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL:
            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock));
            gGetBuffDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL:
            gCloseDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FLAC_SEEKFAST:
            {
                FILE* fp;
                gpFlacSeekFastParm = (FLAC_SEEKFAST_OP_t*)data;
                fp = (FILE*)gpFlacSeekFastParm->fp;
                FileInfo[(int)fp].Offset = gpFlacSeekFastParm->offset;
                FileInfo[(int)fp].Clus   = gpFlacSeekFastParm->clus;

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(0,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;

        case MEDIA_MSGBOX_CMD_FLAC_GETSEEK_INFO:
            {
                gpFlacSeekFastParm->clus = FileInfo[(int)data].Clus;
                gpFlacSeekFastParm->offset = FileInfo[(int)data].Offset;

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_INFO_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData((uint32)gpFlacSeekFastParm,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;


        default:
            return;
    }
}

void RegMBoxDecodeSvc()
{
    IntRegister(INT_ID_MAILBOX1, AudioDecodingGetOutBuffer);
    IntPendingClear(INT_ID_MAILBOX1);
    MailBoxClearB2AInt(MAILBOX_ID_0,  MAILBOX_INT_1);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
    IntEnable(INT_ID_MAILBOX1);
}

void DeRegMBoxDecodeSvc()
{
    IntUnregister(INT_ID_MAILBOX1);
    IntPendingClear(INT_ID_MAILBOX1);
    IntDisable(INT_ID_MAILBOX1);
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
}

/*
--------------------------------------------------------------------------------
  Function name :void AudioDecodingInputFileBuffer(void)
  Author        :
  Description   : input file data stream to calculate core
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    mlc             2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
ExecFunPtr AudioDecodingInputFileBuffer(void)
{
    uint32 cmd;
    uint32 data;

    cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
    //printf("\n B2A File Service cmd = %s,cmd = %d",Cmd2String(cmd),cmd);

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_OPEN_HANDSHK:
            gACKDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_SEEK:
            {
                uint8 ret;
                gpFileSeekParm = (FILE_SEEK_OP_t*)data;
                ret = FileSeek(gpFileSeekParm->offset,gpFileSeekParm->whence, gpFileSeekParm->handle);
                gpFileSeekParm->offset = FileInfo[gpFileSeekParm->handle].Offset;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(ret, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;

        case MEDIA_MSGBOX_CMD_FILE_READ:
            {
                uint32 ret;
                uint8  handle;
                uint8 *dataptr;
                uint32 num;
                gpFileReadParm = (FILE_READ_OP_t*)data;

                handle  = gpFileReadParm->handle;
                dataptr = gpFileReadParm->pData;
                num     = gpFileReadParm->NumBytes;

                //printf("\n gpFileReadParm = 0x%08x ",gpFileReadParm);
                //printf("\n gpFileReadParm->pData = 0x%08x ",dataptr);
                //printf("\n gpFileReadParm->NumBytes = 0x%08x ",num);
                //printf("\n gpFileReadParm->handle = 0x%08x ",handle);

                ret = FileRead(gpFileReadParm->pData,num,handle);

                gpFileReadParm->NumBytes = FileInfo[gpFileReadParm->handle].Offset;

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_READ_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(ret, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

                //dumpMemoryA(AudioFileBuffer,32);
            }
            break;

        case MEDIA_MSGBOX_CMD_FILE_WRITE:
            {
                uint32 ret;
                gpFileWriteParm = (FILE_WRITE_OP_t*)data;
                ret = FileWrite(gpFileWriteParm->buf,gpFileWriteParm->fileOffset,
                                gpFileWriteParm->size,gpFileWriteParm->handle);

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(ret,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;

        case MEDIA_MSGBOX_CMD_FILE_TELL:
            {
                uint32 offset;
                offset = (FileInfo[(int)data].Offset);

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_TELL_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(offset,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;

        case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH:
            {
                uint32 filesize;
                filesize = (FileInfo[(int)data].FileSize);
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(filesize,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;


        case MEDIA_MSGBOX_CMD_FILE_CLOSE:
            {
                uint32 ret;
                ret = FileClose((HANDLE)data);
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(ret,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE_HANDSHK:
            gACKDone = 1;
            break;

        default:

            break;
    }

}

void RegMBoxFileSvc()
{
    IntRegister(INT_ID_MAILBOX2, AudioDecodingInputFileBuffer);
    IntPendingClear(INT_ID_MAILBOX2);
    MailBoxClearB2AInt(MAILBOX_ID_0,  MAILBOX_INT_2);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
    IntEnable(INT_ID_MAILBOX2);
}


void DeRegMBoxFileSvc()
{
    IntUnregister(INT_ID_MAILBOX2);
    IntPendingClear(INT_ID_MAILBOX2);
    IntDisable(INT_ID_MAILBOX2);
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
}



/*
--------------------------------------------------------------------------------
  Function name :void AudioStart(void)
  Author        : zs
  Description   :call related initialization,then start audio.
  Input         : null
  Return        : null
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioStart(void)
{
    int ret = 0;
    uint32 timeout = 200;

#ifdef _BEEP_
    BeepStop();
#endif

    FREQ_EnterModule(FREQ_AUDIO_INIT);

    ModuleOverlay(MODULE_ID_AUDIO_INIT,MODULE_OVERLAY_ALL);

    AudioVariableInit();

    #ifdef _AUDIO_DECODE_PCM_DUMP_
    hPlayRecordFile = FileOpenW(NULL,L"C:\\",L"playrecord.pcm", "W");
    rk_printf("hPlayRecordFile = %d", hPlayRecordFile);
    #endif

    if (ERROR == AudioFileOpen())
    {
        if((AUDIO_STATE_FFD == AudioPlayerState) || (AUDIO_STATE_FFW == AudioPlayerState))
        {
            SendMsg(MSG_AUDIO_FFW_FFD_END);
        }
        else
        {
            MusicNextFile = 1;
            SendMsg(MSG_AUDIO_FILE_ERROR);
        }
        FREQ_ExitModule(FREQ_AUDIO_INIT);
        DEBUG("Audio File Open Error");
        return;
    }

    SendMsg(MSG_AUDIO_INIT_SUCESS);

    AudioHWInit();

    AudioCodecOpenErr = 0;
    if (ERROR == AudioCodecOpen())
    {
        AudioCodecOpenErr = 1;

        if (AudioPlayerState == AUDIO_STATE_PLAY)
        {
            MusicNextFile = 1;
            SendMsg(MSG_AUDIO_FILE_ERROR);
        }
        else if((AudioPlayerState == AUDIO_STATE_FFD) ||
            (AudioPlayerState == AUDIO_STATE_FFW))
        {
            SendMsg(MSG_AUDIO_FFW_FFD_END);
        }
        FREQ_ExitModule(FREQ_AUDIO_INIT);
        DEBUG("Codec Open Error1");
        return;
    }

#ifdef AUDIOHOLDONPLAY
    AudioHoldonInit();
#endif

    AudioFREQInit();
    FREQ_ExitModule(FREQ_AUDIO_INIT);

    I2SInit(I2S_CH, I2S_PORT,I2S_MODE,
            pAudioRegKey->samplerate, I2S_FORMAT,
            I2S_DATA_WIDTH16, I2S_NORMAL_MODE);

#if (I2S_MODE == I2S_SLAVE_MODE)
    switch (pAudioRegKey->samplerate)
    {
        case FS_8000Hz:
        case FS_16KHz:
        case FS_32KHz:
        case FS_64KHz:
        case FS_128KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_40960);
            break;

        case FS_12KHz:
        case FS_24KHz:
        case FS_48KHz:
        case FS_96KHz:
        case FS_192KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_61440);
            break;
        case FS_11025Hz:
        case FS_22050Hz:
        case FS_44100Hz:
        case FS_88200Hz:
        case FS_1764KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_56448);
            break;

        default:
            break;
    }
#endif

    Codec_SetSampleRate(pAudioRegKey->samplerate);

    CodecGetCaptureBuffer((short *)&AudioPtr, &AudioLen);
    memset((uint8*)AudioPtr, 0, AudioLen * 4);
    Bit_Convertor((short*)AudioPtr, &AudioLen,  pAudioRegKey->bps);

#ifdef _RK_EQ_
    EffectInit();
    EQ_ClearBuff();
    AudioSetEQ(AudioPlayInfo.playVolume);
#endif

    AudioDecodeing = 0;
    DmaTransting = 0;
    AudioNeedDecode = 0;

    timeout = 200;
    while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
    {
        DelayMs(1);
        if (--timeout == 0)
        {
            break;
        }
    }

#ifdef _A2DP_SOUCRE_
    AudioSbcEncodeStart();
#ifdef BT_VENDORDEP_ENABLE
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        DEBUG("avrcp_player_status_changed");
        avrcp_track_changed();
        avrcp_player_status_changed(AVRCP_PLAY_STATUS_PLAYING);


    }
#endif
#endif

    if (AudioPlayerState == AUDIO_STATE_PLAY)
    {
        DmaTransting = 1;
        DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),
                 AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);

        AudioPlayState = AUDIO_STATE_PLAY;
        ClearMsg(MSG_MUSIC_FADE_OK);

        AudioDecodeing = 1;
        UserIsrRequest();

        I2SStart(I2S_CH, I2S_START_DMA_TX);
    }
    else if(AudioPlayerState == AUDIO_STATE_PAUSE)
    {
        CurrentTimeSecBk = 0xffffffff;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN AudioPlay(void)
  Author        : zs
  Description   : audio decode,decode next frame.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                   zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
int32 FadeFinishedWait(uint32 timeout, int type)
{
    uint32 tmp;
    tmp = timeout;
    while (!FadeIsFinished())
    {
        if (AUDIO_STATE_PLAY != AudioPlayState)
            break;

        BBDebug();
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        DelayMs(1);
        if (--timeout == 0)
        {
            //DEBUG("FadeFinishedWait Timeout,delayed %d ms",tmp);
            break;
        }
    }
    if (type == FADE_OUT)
    {
        AudioEndFade = 1;
    }
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioPause(void)
  Author        : zs
  Description   : audio decode pause,to changed paly status to comply temporarily.
  Input         : null
  Return        : TRUE/FALSE
  History       : <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :    ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioPause(void)
{
    uint32 timeout = 20000;//20000;
    //DEBUG("=== AudioPause in ===");

    if (AUDIO_STATE_PLAY == AudioPlayerState)
    {
#ifdef _FADE_PROCESS_
        AudioEndFade = 0;
        FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 32, FADE_OUT);
        FadeFinishedWait(pAudioRegKey->samplerate, FADE_OUT);
#endif

        AudioPlayerState = AUDIO_STATE_PAUSE;
        AudioPlayState = AUDIO_STATE_STOP;
#ifdef _BEEP_
        if (BeepPlayState != Voice_PLAY)
#endif
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
            AudioPlayInfo.VolumeCnt = 0;
        }

#ifdef _BEEP_
        if (!gSysConfig.BeepEnabled)
        {
            BeepPlay(BEEP_STOP, 1, BEEP_VOL_SET, 0, 0 );
        }
#endif

        if (CheckMsg(MSG_MUSIC_FADE_IN))
        {
            ClearMsg(MSG_MUSIC_FADE_IN);
        }

        AutoPowerOffEnable();
#ifdef _BEEP_
        if (BeepPlayState != Voice_PLAY)
#endif
        {
            AudioDecodeing = 0;
            DmaTransting = 0;
        }
    }

    else
    {
#ifdef _BEEP_
        if (BeepPlayState != Voice_PLAY)
#endif
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
            AudioPlayInfo.VolumeCnt = 0;
            //Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
        }

#ifdef _BEEP_
        if (!gSysConfig.BeepEnabled)
        {
            BeepPlay(BEEP_STOP, 1, BEEP_VOL_SET, 0, 0 );
        }
#endif
    }

#ifdef _A2DP_SOUCRE_
    //DEBUG("btConnected=%d,AudioOutputMode=%d\n", gSysConfig.BtConfig.btConnected, gSysConfig.AudioOutputMode);
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        avdtp_suspend();
#ifdef BT_VENDORDEP_ENABLE
        avrcp_player_status_changed(AVRCP_PLAY_STATUS_PAUSED);
#endif
    }
#endif

    //DEBUG("=== AudioPause out ===");
    return TRUE;
}


/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioFFPause(void)
  Author        : zs
  Description   : audio decode pause,to changed paly status to comply temporarily.
  Input         : null
  Return        : TRUE/FALSE
  History       : <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :    ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioFFPause(void)
{
    uint32 timeout = 20000;//20000;
    //DEBUG("==AudioFFPause==");
    if(CheckMsg(MSG_AUDIO_FFW_FFD_END) == TRUE)
    {
        return FALSE;
    }
    if (AudioPlayState != AUDIO_STATE_STOP)
    {
#ifdef _FADE_PROCESS_
        AudioEndFade = 0;
        FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 32, FADE_OUT);
        FadeFinishedWait(pAudioRegKey->samplerate, FADE_OUT);
#endif
        AudioPlayState = AUDIO_STATE_STOP;

        while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
        {
            if (--timeout == 0)
            {
                break;
            }

            DelayMs(1);
        }

        AudioDecodeing = 0;
        DmaTransting = 0;
    }

#ifdef _A2DP_SOUCRE_
    //DEBUG("AudioFFPause:btConnected=%d,AudioOutputMode=%d\n", gSysConfig.BtConfig.btConnected, gSysConfig.AudioOutputMode);
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        avdtp_suspend();
    }
#endif
    return TRUE;
}


/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN AudioResume(void)
  Author        :  zs
  Description   :  audio decode resume.
                   changed play status.
                   send decode message.
                   start DMA
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioResume(void)
{
    //DEBUG("=== AudioResume in ===  ");
    if (AUDIO_STATE_PLAY == AudioPlayerState)
    {
        return;
    }

    if (AudioPlayerState == AUDIO_STATE_PAUSE)
    {
        AutoPowerOffDisable();
    }

#ifdef _BEEP_
    BeepStop();
#endif

    if (AudioCodecOpenErr)
    {
        AudioPlayerState = AUDIO_STATE_PLAY;
        AudioPlayState = AUDIO_STATE_STOP;
        SendMsg(MSG_AUDIO_FILE_ERROR);
        return TRUE;
    }

    AudioErrorFrameNum = 0;
    SendMsg(MSG_MUSIC_DISPFLAG_STATUS);//cw 2009-5-11
    CodecGetCaptureBuffer((short*)&AudioPtr,&AudioLen);
    memset((uint8*)AudioPtr, 0, AudioLen * sizeof(int16) * 2);

    DmaTransting = 1;
    ClearMsg(MSG_MUSIC_FADE_OK);
    DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),  AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);
    I2SStart(I2S_CH, I2S_START_DMA_TX);
#ifdef _A2DP_SOUCRE_
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        sbcTotalCnt = 0;
        avdtp_start();
        avrcp_player_status_changed(AVRCP_PLAY_STATUS_PLAYING);
    }
#endif
    AudioPlayerState = AUDIO_STATE_PLAY;
    AudioPlayState   = AUDIO_STATE_PLAY;

    AudioDecodeing = 1;
    UserIsrRequest();

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN AudioResume(void)
  Author        :  zs
  Description   :  audio decode resume.
                   changed play status.
                   send decode message.
                   start DMA
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioFFResume(void)
{
    uint32 timeout;

    if(CheckMsg(MSG_AUDIO_FFW_FFD_END) == TRUE)
    {
        return FALSE;
    }
#ifdef _BEEP_
    BeepStop();
#endif

    if(AudioCodecOpenErr)
    {
        if(AudioPlayerState == AUDIO_STATE_FFW)
            MusicNextFile = -1;
        else if(AudioPlayerState == AUDIO_STATE_FFD)
            MusicNextFile = 1;

        SendMsg(MSG_MUSIC_DISPLAY_ALL);
        return FALSE;
    }

    CodecSeek(pAudioRegKey->CurrentTime, 0);

    gDecDone = 0;
    CodecDecode();

    timeout = 10000;
    while(!gDecDone)        //wait decod over
    {
    #ifdef _WATCH_DOG_
        WatchDogReload();
    #endif
        BBDebug();

        DelayMs(1);
        if (--timeout == 0)
        {
            DEBUG("FFResume CodecDecode: timeout!!!");
            break;
        }
    }
    gDecDone = 0;

    AudioErrorFrameNum = 3;
    CodecGetCaptureBuffer((short*)&AudioPtr,&AudioLen);
    memset((uint8*)AudioPtr, 0, AudioLen * sizeof(int16) * 2);
    AudioPlayState   = AUDIO_STATE_PLAY;

    DmaTransting = 1;
    ClearMsg(MSG_MUSIC_FADE_OK);
    DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),  AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);
    I2SStart(I2S_CH, I2S_START_DMA_TX);

    AudioDecodeing = 1;
    UserIsrRequest();

#ifdef _A2DP_SOUCRE_
    //DEBUG("==AudioFFResume:btConnected=%d,AudioOutputMode=%d\n", gSysConfig.BtConfig.btConnected, gSysConfig.AudioOutputMode);
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        sbcTotalCnt = 0;
        avdtp_start();
    }
#endif

    //DEBUG("==AudioFFResume out==\n");
    return TRUE;
}


/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioFFW(unsigned long StepLen)
  Author        :  zs
  Description   :  audio FFD
  Input         :  StepLen:The step of FFD.
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioFFW(unsigned long StepLen)
{
    uint32 timeout = 200;

    //DEBUG("AudioFFW");
    if (AUDIO_STATE_STOP == AudioPlayerState)
    {
        return TRUE;
    }

    if(CheckMsg(MSG_AUDIO_FFW_FFD_END) == TRUE)
    {
        return FALSE;
    }

    if (AUDIO_STATE_FFW != AudioPlayerState)  //wait for mute
    {
        if (AUDIO_STATE_PLAY == AudioPlayState)
        {
#ifdef _FADE_PROCESS_
            AudioEndFade = 0;
            FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 32, FADE_OUT);
            FadeFinishedWait(pAudioRegKey->samplerate, FADE_OUT);
#endif
        }

        AudioPlayerState = AUDIO_STATE_FFW;
        AudioPlayState   = AUDIO_STATE_STOP;

#ifdef _BEEP_
        if (BeepPlayState != Voice_PLAY)
#endif
        {
            while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    break;
                }
            }
            AudioPlayInfo.VolumeCnt = 0;
            //Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
        }

#ifdef _BEEP_
        if (!gSysConfig.BeepEnabled)
        {
            BeepPlay(BEEP_AMS_PLUS,1,BEEP_VOL_SET, 0, 0);
        }
#endif

#ifdef _A2DP_SOUCRE_
        //DEBUG("AudioFFW:gSysConfig.BtConfig.btConnected=%d\n", gSysConfig.BtConfig.btConnected);
        if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
        {
            avdtp_suspend();
            //avrcp_player_status_changed(AVRCP_PLAY_STATUS_FWD_SEEK);
        }
#endif

    }
    else //if (AUDIO_STATE_FFW == AudioPlayState)   //FFW
    {
        pAudioRegKey->CurrentTime = (pAudioRegKey->CurrentTime > StepLen) ? (pAudioRegKey->CurrentTime - StepLen) : 0;
//        gAudioPlayTime = pAudioRegKey->CurrentTime;
    }

    return TRUE;
}

_ATTR_AUDIO_TEXT_
void AudioFFWStop(void)
{
    if(CheckMsg(MSG_AUDIO_FFW_FFD_END) == TRUE)
    {
        return;
    }
    CodecSeek(pAudioRegKey->CurrentTime, 0);

#ifdef _A2DP_SOUCRE_
    AudioSbcEncodeStop();
#endif
}

/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioFFD(unsigned long StepLen)
  Author        :  zs
  Description   :  audio FFW
  Input         :  StepLen:The step of FFW
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioFFD(unsigned long StepLen)
{
    uint32 timeout = 200;
    //DEBUG("AudioFFD ");

    if (AUDIO_STATE_STOP == AudioPlayerState)
    {
        return TRUE;
    }

    if(CheckMsg(MSG_AUDIO_FFW_FFD_END) == TRUE)
    {
        return FALSE;
    }

    if (AUDIO_STATE_FFD != AudioPlayerState)  //wait for mute
    {
        if (AUDIO_STATE_PLAY == AudioPlayState)
        {
#ifdef _FADE_PROCESS_
            AudioEndFade = 0;
            FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 32, FADE_OUT);
            FadeFinishedWait(pAudioRegKey->samplerate, FADE_OUT);
#endif
        }

        AudioPlayerState = AUDIO_STATE_FFD; // audio player state
        AudioPlayState    = AUDIO_STATE_STOP; // play state

#ifdef _BEEP_
        if (BeepPlayState != Voice_PLAY)
#endif
        {
            while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    break;
                }
            }

            AudioPlayInfo.VolumeCnt = 0;
            //Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
        }

#ifdef _BEEP_
        if (!gSysConfig.BeepEnabled)
        {
            BeepPlay(BEEP_AMS_PLUS,1, BEEP_VOL_SET, 0, 0);
        }
#endif

#ifdef _A2DP_SOUCRE_
        if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
        {
            avdtp_suspend();
            //avrcp_player_status_changed(AVRCP_PLAY_STATUS_FWD_SEEK);
        }
#endif

    }
    else //if (AUDIO_STATE_FFD == AudioPlayState)   //FFD
    {
        pAudioRegKey->CurrentTime = (pAudioRegKey->CurrentTime < pAudioRegKey->TotalTime - StepLen) ? (pAudioRegKey->CurrentTime + StepLen) : pAudioRegKey->TotalTime;
//        gAudioPlayTime = pAudioRegKey->CurrentTime;
    }
    //DEBUG("AudioFFD out");
    return TRUE;
}

_ATTR_AUDIO_TEXT_
void AudioFFDStop(void)
{
    if (pAudioRegKey->CurrentTime >= pAudioRegKey->TotalTime)
    {
        pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
        FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
    }

    if(CheckMsg(MSG_AUDIO_FFW_FFD_END) == TRUE)
    {
        return;
    }

    CodecSeek(pAudioRegKey->CurrentTime, 0);

#ifdef _A2DP_SOUCRE_
    AudioSbcEncodeStop();
#endif
    //DEBUG("Leaving AudioFFDStop\n");
}

/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioFFW(unsigned long StepLen)
  Author        :  zs
  Description   :  audio FFW
  Input         :  StepLen:The step of FFW
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioNext(uint32 msg)
{
    UINT32 ANextKeyVal;
    INT16 i;

    AudioStop(AUDIO_STOP_FORCE);

#if 0
    if (msg)
    {
#ifdef _BEEP_
        if (!gSysConfig.BeepEnabled)
        {
            BeepPlay(BEEP_AMS_PLUS,  1, BEEP_VOL_SET, 0, 0);
        }
#endif
    }
    else
    {
#ifdef _BEEP_
        if (!gSysConfig.BeepEnabled)
        {
            BeepPlay(BEEP_AMS_MINUS,  1, BEEP_VOL_SET, 0, 0);
        }
#endif
    }
#endif

    AudioGetNextMusic_NP((UINT32)msg);

    i = 0;
    ANextKeyVal =  GetKeyVal();
    if (!gSysConfig.BeepEnabled)
    {
        BeepStop();
    }
    else
    {
        DelayMs(40);
    }
    while (i < 5)
    {
        WatchDogReload();
        if(
            (KEY_VAL_FFD_DOWN == ANextKeyVal) ||
            (KEY_VAL_FFW_DOWN == ANextKeyVal)
        )
        {
            ANextKeyVal =  GetKeyVal();
        }
        if(KEY_VAL_FFD_SHORT_UP == ANextKeyVal)
        {
            if (!gSysConfig.BeepEnabled)
            {
                BeepPlay(BEEP_AMS_PLUS,  1, BEEP_VOL_SET, 0, 0);
            }
            AudioGetNextMusic_NP((UINT32)1);
        }
        else if (KEY_VAL_FFW_SHORT_UP == ANextKeyVal)
        {
            if (!gSysConfig.BeepEnabled)
            {
                BeepPlay(BEEP_AMS_PLUS,  1, BEEP_VOL_SET, 0, 0);
            }
            AudioGetNextMusic_NP((UINT32)-1);
        }
        else
        {
            break;
        }
        if (!gSysConfig.BeepEnabled)
        {
            BeepStop();
        }
        ANextKeyVal =  GetKeyVal();
        DelayMs(40);
        i++;
    }
    AudioStart();
    SendMsg(MSG_AUDIO_NEXTFILE);
}
_ATTR_AUDIO_TEXT_
BOOLEAN     AudioGetNextMusic_NP(UINT32 msg)
{
    uint32  Playednumber;

    switch (pAudioRegKey->RepeatMode)
    {
        case AUDIO_REPEAT://single repeat
        case AUDIO_REPEAT1:
            if (AUDIO_STOP_FORCE == AudioStopMode)
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);
            }
            break;

        case AUDIO_TRY: //ok lyrics
        case AUDIO_ALLONCE://only once all song play cycle.
            Playednumber = AudioFileInfo.CurrentFileNum;
            if (pAudioRegKey->PlayOrder == AUDIO_RAND)
            {
                Playednumber = AudioPlayFileNum;
            }

            if ((Playednumber >=AudioFileInfo.TotalFiles )&&(AudioStopMode != AUDIO_STOP_FORCE))
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum = 1;
                }
                AudioStart();

                CodecSeek(0, 0);
                if (pAudioRegKey->CurrentTime >= pAudioRegKey->TotalTime)
                {
                    pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
                }

                AudioPause();

                CurrentTimeSecBk = 0xffffffff;
            }
            else
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum += (INT16)msg;
                    if (AudioPlayFileNum < 1)
                    {
                        AudioPlayFileNum = AudioFileInfo.TotalFiles;
                    }
                }
            }
            break;

        case AUDIO_ALLREPEAT://all cycle play
            AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_ALL;
            SysFindFile(&AudioFileInfo,(INT16) msg);
            break;

        case AUDIO_ONCE: //single once.
            if (AUDIO_STOP_FORCE == AudioStopMode)
            {
                SysFindFile(&AudioFileInfo,(INT16) msg);
                break;
            }
            else
            {
                AudioStart();
                CodecSeek(0, 0);
                if (pAudioRegKey->CurrentTime >= pAudioRegKey->TotalTime)
                {
                    pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
                }
                AudioPause();

                CurrentTimeSecBk = 0xffffffff;
            }
            break;

        case AUDIO_FOLDER_ONCE://directory once.
            AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_DIR;

            Playednumber = AudioFileInfo.CurrentFileNum;
            if (pAudioRegKey->PlayOrder == AUDIO_RAND)
            {
                Playednumber = AudioPlayFileNum;
            }
            if ((Playednumber >= AudioFileInfo.TotalFiles)&&(AudioStopMode != AUDIO_STOP_FORCE))
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum = 1;
                }

                AudioPlayerState = AUDIO_STATE_PAUSE;

                AudioStart();
                if(1 == AudioCodecOpenErr)
                {
                    printf("\n ######## AudioCodec Open Error ########\n");
                    SysFindFile(&AudioFileInfo,(INT16)msg);
                }
                CodecSeek(0, 0);
                if (pAudioRegKey->CurrentTime >= pAudioRegKey->TotalTime)
                {
                    pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
                }
                AudioPause();
                CurrentTimeSecBk = 0xffffffff;

            }
            else
            {
                SysFindFile(&AudioFileInfo,(INT16)msg);

                if (pAudioRegKey->PlayOrder == AUDIO_RAND)
                {
                    AudioPlayFileNum += (INT16)msg;
                    if (AudioPlayFileNum < 1)
                    {
                        AudioPlayFileNum = AudioFileInfo.TotalFiles;
                    }
                }
//                if(1 == AudioCodecOpenErr)
//                {
//                    if(AudioPlayerState == AUDIO_STATE_FFD ||AudioPlayerState == AUDIO_STATE_FFW)
//                    {
//                        AudioPlayerState = AUDIO_STATE_PLAY;
//                    }
//                }
            }
            break;

        case AUIDO_FOLDER_REPEAT://directory cycle.
            AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_DIR;
            SysFindFile(&AudioFileInfo,(INT16)msg);
            break;

        default:
            break;
    }
}
/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioStop(UINT16 ReqType);
  Author        :  zs
  Description   :  audio decode over,change paly status,and close file.
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioStop(UINT16 ReqType)
{
    TASK_ARG TaskArg;
    uint32 timeout = 20000;

    DEBUG("Audio Stop,ReqType = %d",ReqType);

    #ifdef _AUDIO_DECODE_PCM_DUMP_
    FileClose(hPlayRecordFile);
    #endif

    if ((AUDIO_STATE_PLAY == AudioPlayState) && ((AUDIO_STOP_NORMAL != ReqType)))
    {
#ifdef _FADE_PROCESS_
        AudioEndFade = 0;
        //FadeInit(0, pAudioRegKey->samplerate / 32, FADE_OUT);
        FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 32, FADE_OUT);
        FadeFinishedWait(pAudioRegKey->samplerate, FADE_OUT);
#endif
    }

    AudioPlayState = AUDIO_STATE_STOP;
#ifdef _BEEP_
    if (BeepPlayState != Voice_PLAY)
#endif
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
    }

#ifdef _BEEP_
    if (AUDIO_STOP_FORCE == ReqType)
    {
        if (MusicNextFile)
        {
            if (!gSysConfig.BeepEnabled)
            {
                BeepPlay(BEEP_AMS_PLUS,  1, BEEP_VOL_SET, 0, 0);
            }
        }
        else
        {
            if (!gSysConfig.BeepEnabled)
            {
                BeepPlay(BEEP_AMS_MINUS,  1, BEEP_VOL_SET, 0, 0);
            }
        }
        BeepStop();
    }
#endif

#ifdef _BEEP_
    if (BeepPlayerState != Voice_PLAY)
#endif
    {
        I2SStop(I2S_CH, I2S_START_DMA_TX);
        I2SDeInit(I2S_CH, I2S_PORT);

        AudioDecodeing = 0;
        DmaTransting = 0;
    }

    if((AUDIO_STATE_FFD == AudioPlayerState)
        || (AUDIO_STATE_FFW== AudioPlayerState))
    {
    }
    else
    {
        if(FALSE == CheckMsg(MSG_BL_OFF))
        {
            if (TRUE == GetMusicUIState())
            {
                if ((pAudioRegKey->TotalTime - pAudioRegKey->CurrentTime) <= 1000)
                {
                    if (!AudioCodecOpenErr) //no open error happen
                    {
                        pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    }

                    SendMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME);
                    MusicWinPaint();
                    Lcd_BuferTranfer();
                }
            }
        }
    }

    if (CheckMsg(MSG_MUSIC_FADE_IN))
    {
        ClearMsg(MSG_MUSIC_FADE_IN);
    }

#ifdef _RK_EQ_
    EffectEnd(); //audio effect finish.
#endif

    ModuleOverlay(MODULE_ID_AUDIO_INIT,MODULE_OVERLAY_ALL);

    AudioVariableDeInit();

    CloseTrack();

    AudioDeHWInit();

    AudioFileClose();

#ifdef THUMB_DEC_INCLUDE
    if (AudioAlbumHandle != (FILE*)-1)
    {
        FileClose((HANDLE)AudioAlbumHandle);
        AudioAlbumHandle = (FILE*)-1;
    }
#endif

    AudioFREQDeInit();

#ifdef _SDCARD_ //by zs 05.18 slove the machine crash as pull out sd card.
    if ((gSysConfig.Memory == CARD) && (CheckCard() == 0))
    {
        if (FALSE == GetMusicUIState())
        {
            SendMsg(MSG_AUDIO_CARD_CHECK);

            gSysConfig.Memory = FLASH0;

            FileSysSetup(gSysConfig.Memory);
            SDCardDisable();

        }
    }
#endif

#ifdef _A2DP_SOUCRE_
    AudioSbcEncodeStop();
#ifdef BT_VENDORDEP_ENABLE
    if(gSysConfig.BtConfig.btConnected && gSysConfig.AudioOutputMode == 1)
    {
        DEBUG("avrcp_player_status_changed");
        avrcp_player_status_changed(AVRCP_PLAY_STATUS_STOPPED);
    }
#endif
#endif

    if (AUDIO_STOP_NORMAL == ReqType)
    {
        AudioStopMode = AUDIO_STOP_NORMAL;//by zs 0512
        AudioGetNextMusic(MusicNextFile);
        SendMsg(MSG_AUDIO_NEXTFILE);
        return FALSE;
    }

    return TRUE;
}

#if 0
/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioSetPosition(unsigned long Desti_Posi)
  Author        :  zs
  Description   :  paly in any position,use for touch panel and hold on point(break point) play.
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioSetPosition(unsigned long Desti_Posi)
{
    CodecSeek((Desti_Posi > pAudioRegKey->TotalTime) ? pAudioRegKey->TotalTime : Desti_Posi, 0);

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name :  void AudioSetAB_A(void)
  Author        :  zs
  Description   :  set the start point A for AB repeat read.
  Input         :  null
  Return        :  null
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioSetAB_A(void)
{
    //.1th set the repeat start.
    CodecGetTime(&pAudioRegKey->CurrentTime);

    AudioPlayInfo.ABRequire = AUDIO_AB_A;
    AudioPlayInfo.AudioABStart = pAudioRegKey->CurrentTime;
}

/*
--------------------------------------------------------------------------------
  Function name :  void AudioSetAB(void)
  Author        :  zs
  Description   :  set the end point of AB repeat, and start repeat.
  Input         :  null
  Return        :  null
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioSetAB(void)
{
    uint32 timeout = 100;

    if (AudioPlayInfo.ABRequire == AUDIO_AB_A)    //set the end point of repeat.
    {
        CodecGetTime(&pAudioRegKey->CurrentTime);
        AudioPlayInfo.AudioABEnd = pAudioRegKey->CurrentTime;
        AudioPlayInfo.ABRequire = AUDIO_AB_PLAY;

        /*
        //wait for fade out finished
        if (AUDIO_STATE_PLAY == AudioPlayState)
        {
            #ifdef _FADE_PROCESS_
            FadeInit(0,pAudioRegKey->samplerate / 2,FADE_OUT);
            FadeFinishedWait(pAudioRegKey->samplerate/2);
            #endif
        }

        //wait DMA finished
        AudioPlayState = AUDIO_STATE_FFW;
        while(DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
        {
            DelayMs(1);
            if (--timeout == 0)
            {
                break;
            }
        }

        AudioPlayInfo.VolumeCnt = 0;
        Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
        */

        // Set to A Point
        AudioFFW((pAudioRegKey->CurrentTime - AudioPlayInfo.AudioABStart)); //first pause music
        AudioFFW((pAudioRegKey->CurrentTime - AudioPlayInfo.AudioABStart)); //second

        //Start Fadein and Play
        AudioPlayState = AUDIO_STATE_PLAY;
        AudioPlayerState = AUDIO_STATE_PLAY;

#ifdef _FADE_PROCESS_
        FadeInit(0,pAudioRegKey->samplerate/2,FADE_IN);
#endif

        AudioErrorFrameNum = 0;
        SendMsg(MSG_MUSIC_DISPFLAG_STATUS);//cw 2009-5-11

        CodecGetCaptureBuffer((short*)&AudioPtr, &AudioLen);
        memset((uint8*)AudioPtr, 0, AudioLen * sizeof(int16) * 2);
        DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),  AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);

    }
}

/*
--------------------------------------------------------------------------------
  Function name :  void AudioABStop(void)
  Author        :  zs
  Description   :  stop repeat.
  Input         :  null
  Return        :  null
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioABStop(void)
{
    AudioPlayInfo.ABRequire = AUDIO_AB_NULL;
}
#endif

/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioDecodeProc(MSG_ID id,void *msg)
  Author        :  zs
  Description   :  audio handle function, remember to set vaule for variable AudioProcId and AudioProcMsg when call
                   this function.
                   AudioProcId  -- id of msg、AudioProcMsg -- the content of message.
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN AudioDecodeProc(MSG_ID id,void * msg)
{
    BOOLEAN ret = TRUE;
    unsigned long  HoldOnTimeTemp;

    switch (id)
    {
        case MSG_AUDIO_DECSTART:
            AudioStart();
            break;

        case MSG_AUDIO_PAUSE:
            AudioPause();
            break;

        case MSG_AUDIO_RESUME:
            AudioResume();
            break;

        case MSG_AUDIO_STOP:
            if (AudioStop((int)msg))
            {
#ifdef AUDIOHOLDONPLAY
#ifdef _MEDIA_MODULE_
                if (gSysConfig.Memory == FLASH0) //the music can not breakpoint play when the music in sd card.
#endif
                {
                    MusicHoldOnInforSave(pAudioRegKey->CurrentTime, &AudioFileInfo);
                }
#endif
            }
            break;

        case MSG_AUDIO_FFD:     //FFW
            {
                AudioFFD((unsigned long)msg);
            }
            break;

        case MSG_AUDIO_FFW:     //FFD
            {
                AudioFFW((unsigned long)msg);
            }
            break;

#if 0
        case MSG_AUDIO_GETPOSI:
            CodecGetTime(&pAudioRegKey->CurrentTime);
            break;

        case MSG_AUDIO_SETPOSI:
            {
                AudioSetPosition((unsigned long)msg);
            }
            break;

        case MSG_AUDIO_ABPLAY:
            AudioSetAB();
            break;

        case MSG_AUDIO_ABSETA:
            AudioSetAB_A();
            break;

        case MSG_AUDIO_ABSTOP:
            AudioABStop();
            break;
#endif

        case MSG_AUDIO_VOLUMESET:
        {
            AudioInOut_Type  *pAudio = &AudioIOBuf;
            RKEffect   *pEffect = &pAudio->EffectCtl;
            AudioSetVolume();
#if 0

            if (((AudioPlayInfo.playVolume > 27) || ((AudioPlayInfo.playVolume == 27) && (pEffect->max_DbGain == 10)))  && (pEffect->Mode == EQ_BASS))
            {
                AudioSetEQ(AudioPlayInfo.playVolume);
            }
            else
            {
                if (AUDIO_STATE_PLAY == AudioPlayState)
                {
                    Codec_SetVolumet(AudioPlayInfo.playVolume);
                }
            }

#else

            if (VOLTAB_CONFIG == VOL_General)
            {
                if (pEffect->Mode == EQ_BASS)
                {
                    if ((AudioPlayInfo.playVolume > 27) || ((AudioPlayInfo.playVolume == 27) && (pEffect->max_DbGain == 10)))
                    {
                        AudioSetEQ(AudioPlayInfo.playVolume);
                    }
                }
            }
            else
            {
                if (pEffect->Mode == EQ_BASS)
                {
                    if ((AudioPlayInfo.playVolume > 22) || ((AudioPlayInfo.playVolume == 22) && (pEffect->max_DbGain == 10)))
                    {
                        AudioSetEQ(AudioPlayInfo.playVolume);
                    }
                }

                if ((pEffect->Mode == EQ_JAZZ) || (pEffect->Mode == EQ_UNIQUE))
                {
                    if ((AudioPlayInfo.playVolume > 25) || ((AudioPlayInfo.playVolume == 25) && (pEffect->max_DbGain == 5)))
                    {
                        AudioSetEQ(AudioPlayInfo.playVolume);
                    }
                }

                if (pEffect->Mode == EQ_POP)
                {
                    if ((AudioPlayInfo.playVolume > 27) || ((AudioPlayInfo.playVolume == 27) && (pEffect->max_DbGain == 1)))
                    {
                        AudioSetEQ(AudioPlayInfo.playVolume);
                    }
                }
            }

            if (AUDIO_STATE_PLAY == AudioPlayState)
            {
                Codec_SetVolumet(AudioPlayInfo.playVolume);
            }

#endif
        }
        break;

        case MSG_AUDIO_EQSET:
            if (msg)
            {
                memcpy(&AudioIOBuf.EffectCtl, &pAudioRegKey->UserEQ, sizeof(RKEffect));
            }

            AudioSetEQ(AudioPlayInfo.playVolume);
            break;

        case MSG_AUDIO_UNMUTE:
        {
            Codec_DACUnMute();
        }
        break;

        case MSG_AUDIO_NEXTFILE:
            AudioNext((UINT32)msg);
            break;

        case MSG_AUDIO_FF_PAUSE:
            AudioFFPause();
            break;

        case MSG_AUDIO_FF_RESUME:
            AudioFFResume();
            break;

        default:
            ret = FALSE;
            break;
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Description   :  if no file in current disk, when enter the audio module in the situation
                   of more than one disk(no media libary),then poll the following disk.
  Input         :  null
  Return        :  0 -- find file from disk， 1-- no fine in disk
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
_ATTR_AUDIO_TEXT_
UINT16 DiskChange(FS_TYPE FsType)
{
    UINT16 RetVal = 0;

    if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FsType)))
    {
        switch (gSysConfig.Memory)
        {
            case FLASH0:
            {
#ifdef _MULT_DISK_

                if (bShowFlash1)
                {
                    gSysConfig.Memory = FLASH1;//to switch to flash1 at frist.
                    FileSysSetup(gSysConfig.Memory);
                }

                if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FsType)))
#endif
#ifdef _SDCARD_
                {
                    if (gSysConfig.SDEnable)
                    {
                        if (CheckCard() == 1) //have sd card.
                        {
                            gSysConfig.Memory = CARD;
                            SDCardEnable();
                            FileSysSetup(gSysConfig.Memory);
                        }
                    }
                }

                if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FsType)))
#endif
                {
                    gSysConfig.Memory = FLASH0;
                    FileSysSetup(gSysConfig.Memory);
#ifdef _SDCARD_
                    SDCardDisable();
#endif
                    RetVal = 1;
                    SendMsg(MSG_AUDIO_NOFILE);
                }
            }
            break;
#ifdef _MULT_DISK_

            case FLASH1:
            {
                gSysConfig.Memory = FLASH0;//switch to flash0 at frist.
                FileSysSetup(gSysConfig.Memory);

                if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FsType)))
#ifdef _SDCARD_
                {
                    if (gSysConfig.SDEnable)
                    {
                        if (CheckCard() == 1) //have card.
                        {
                            gSysConfig.Memory = CARD;
                            SDCardEnable();
                            FileSysSetup(gSysConfig.Memory);
                        }
                    }
                }

                if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FsType)))
#endif
                {
                    gSysConfig.Memory = FLASH0;
                    FileSysSetup(gSysConfig.Memory);
#ifdef _SDCARD_
                    SDCardDisable();
#endif
                    RetVal = 1;
                    SendMsg(MSG_AUDIO_NOFILE);
                }
            }
            break;
#endif
#ifdef _SDCARD_

            case CARD:
            {
                if (gSysConfig.SDEnable)
                {
                    gSysConfig.Memory = FLASH0;///switch to flash0 at frist.
                    FileSysSetup(gSysConfig.Memory);
                    SDCardDisable();

                    if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FsType)))
#ifdef _MULT_DISK_
                    {
                        if (bShowFlash1)
                        {
                            gSysConfig.Memory = FLASH1;//switch to flash1 at frist.
                            FileSysSetup(gSysConfig.Memory);
                        }
                    }

                    if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FsType)))
#endif
                    {
                        gSysConfig.Memory = FLASH0;
                        FileSysSetup(gSysConfig.Memory);
#ifdef _SDCARD_
                        SDCardDisable();
#endif
                        RetVal = 1;
                        SendMsg(MSG_AUDIO_NOFILE);
                    }
                }
            }
            break;
#endif
        }
    }

    return (RetVal);
}
#endif

/*
--------------------------------------------------------------------------------
  Function name :  void MusicInit(void)
  Author        :  zs
  Description   :  audion control initialization.

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void MusicInit(void *pArg)
{
    int i;
    //DEBUG("Music Thread Enter");
    FREQ_EnterModule(FREQ_MAX);
//    ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
    AudioErrorFileCount = 0;
    AudioPlayState      = AUDIO_STATE_STOP;
    ClearMsg(MSG_SERVICE_MUSIC_ORDER_UPDATE);

    if (pArg == NULL)
    {
        gbAudioFileNum = 1;
    }
    else
    {
        gbAudioFileNum = ((AUDIO_THREAD_ARG*)pArg)->FileNum;
    }

    if ((gbAudioFileNum == 0xFFFF) || (gbAudioFileNum == 0)) //by zs 05.13 fix bug
    {
        gbAudioFileNum = 1;
    }

    AudioPlayFileNumSave = gbAudioFileNum - 1;
#if(defined (_SDCARD_) || defined (_MULT_DISK_))

    if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER)
    {
#if (2 == FW_IN_DEV) // spi nor

        if (SDC_CheckCard(SDM_SD_ID, MD_EVENT_CHANGE))
        {
            if (CheckCard())
            {
                DEBUG("find sd changed!!");
                gSysConfig.Memory = FLASH0;//to switch to flash1 at frist.
                FileSysSetup(gSysConfig.Memory);
            }
        }

        AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FS_FAT);

        if (0 == AudioFileInfo.TotalFiles)
        {
            if (0 == CheckCard())
            {
                SendMsg(MSG_AUDIO_CARD_CHECK);
            }

            goto MusicInit_ERR;
        }

#else

        if (DiskChange(FS_FAT))
        {
            goto MusicInit_ERR;
        }

#endif
    }
    else if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        //remove record file
        if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, MUSIC_DB)))
        {
            SendMsg(MSG_AUDIO_NOFILE);
            goto MusicInit_ERR;
        }
    }

#else

    if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER)
    {
        if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FS_FAT)))
        {
            SendMsg(MSG_AUDIO_NOFILE);
            goto MusicInit_ERR;
        }
    }
    else if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        //remove record file
        if (0 == (AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, MUSIC_DB)))
        {
            SendMsg(MSG_AUDIO_NOFILE);
            goto MusicInit_ERR;
        }
    }

#ifdef _M3U_
    else if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_M3U_BROWSER)
    {
        AudioFileInfo.TotalFiles = AudioFileInfo.M3uGlobalFileCnt;  //<----sanshin_20150619
        if (0 == AudioFileInfo.TotalFiles)                          //<----sanshin_20150619
        {
            SendMsg(MSG_AUDIO_NOFILE);
            goto MusicInit_ERR;
        }

        if(CheckMsg(MSG_SHUFFLE_FIRST_PREV))
        {
            ClearMsg(MSG_SHUFFLE_FIRST_PREV);
        }
    }

#endif
#endif

    pAudioRegKey   = &gRegAudioConfig;
    AudioPlayerState = AUDIO_STATE_PLAY;

    if (AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_FOLDER)
    {
#ifdef _M3U_

        if (AudioFileInfo.ucSelPlayType  == SORT_TYPE_SEL_M3U_BROWSER)
        {
            //when showing m3u list; norton
            if (gSysConfig.MusicConfig.RepeatMode == AUDIO_FOLDER_ONCE
                || gSysConfig.MusicConfig.RepeatMode == AUIDO_FOLDER_REPEAT
                || gSysConfig.MusicConfig.RepeatMode == AUDIO_REPEAT)
            {
                pAudioRegKey->RepeatMode    =  gSysConfig.MusicConfig.RepeatMode + AUDIO_ALLONCE; //make sure it don't repeat in folder,just use globalnum   //<----sanshin_20150619
            }
            else
            {
                pAudioRegKey->RepeatMode    =  gSysConfig.MusicConfig.RepeatMode;
            }
        }
        else
#endif
        {
            pAudioRegKey->RepeatMode    =  gSysConfig.MusicConfig.RepeatMode;
        }
    }
    else
    {
        pAudioRegKey->RepeatMode    =  gSysConfig.MusicConfig.RepeatModeBak;

        if ( gSysConfig.MusicConfig.RepeatModeBak > AUDIO_REPEAT)
        {
            gbAudioFileNum = 1;
        }
    }

    pAudioRegKey->PlayOrder     =   gSysConfig.MusicConfig.PlayOrder;

#ifdef _RK_EQ_

    if (!gSysConfig.MusicConfig.BassBoost)  //BASS ON
    {
        pAudioRegKey->UserEQ.Mode = EqMode[7];  //BASS
        AudioIOBuf.EffectCtl.Mode = EqMode[7];
    }
    else
    {
        pAudioRegKey->UserEQ.Mode   = EqMode[gSysConfig.MusicConfig.Eq.Mode];
        AudioIOBuf.EffectCtl.Mode =  EqMode[gSysConfig.MusicConfig.Eq.Mode];

        for (i = 0; i < 5; i++)
        {
            pAudioRegKey->UserEQ.RKCoef.dbGain[i] = UseEqTable[gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i]];
        }

        memcpy(&AudioIOBuf.EffectCtl, &pAudioRegKey->UserEQ, sizeof(RKEffect));
    }

#endif

    if ((pAudioRegKey->RepeatMode == AUDIO_FOLDER_ONCE)||(pAudioRegKey->RepeatMode == AUIDO_FOLDER_REPEAT)||(pAudioRegKey->RepeatMode == AUDIO_REPEAT))
    {
        pAudioRegKey->AudioFileDirOrAll =  FIND_FILE_RANGE_DIR;//pAudioRegKey->RepeatMode;//by zs 06.01 解决目录一次的问题
    }

#ifdef _M3U_

    if ((AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_BROWSER) && (AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_FOLDER) && (AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_M3U_BROWSER))
#else                                                                                                                       //<----sanshin_20150619
    if ((AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_BROWSER) && (AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_FOLDER))    //<----sanshin_20150619
#endif                                                                                                                      //<----sanshin_20150619
    {
        //dgl audio AudioFileInfo.PlayOrder     = pAudioRegKey->AudioFileDirOrAll;
        AudioFileInfo.PlayOrder     = gSysConfig.MusicConfig.PlayOrder;//debug the bug:the play type is order play type when power off then start machine again.
        AudioFileInfo.pExtStr = RecordFileExtString;//SCH 11.04
    }
    else
    {
        SysFindFileInit(&AudioFileInfo, gbAudioFileNum, pAudioRegKey->AudioFileDirOrAll,
                        pAudioRegKey->PlayOrder, (UINT8*)MusicFileExtString);
    }

    if (AudioFileInfo.TotalFiles > SORT_FILENUM_DEFINE)
    {
        AudioFileInfo.TotalFiles = SORT_FILENUM_DEFINE;

        if (AudioFileInfo.CurrentFileNum > AudioFileInfo.TotalFiles)
            AudioFileInfo.CurrentFileNum = AudioFileInfo.TotalFiles - 1;
    }

    if (pAudioRegKey->PlayOrder == AUDIO_RAND)
    {
//        AudioPlayFileNum = 0;
        AudioPlayFileNum = 1;
        CreateRandomList(AudioFileInfo.TotalFiles, AudioFileInfo.CurrentFileNum - 1);
    }

    FREQ_ExitModule(FREQ_MAX);
    AutoPowerOffDisable();

    Codec_SetMode(Codec_DACoutHP, ACodec_I2S_DATA_WIDTH16);

    BBSystemInit();

    RegMBoxDecodeSvc();
    RegMBoxFileSvc();
    AudioDecodeProc(MSG_AUDIO_DECSTART, NULL);
    return;
MusicInit_ERR:
    FREQ_ExitModule(FREQ_MAX);
}

/*
--------------------------------------------------------------------------------
  Function name :  UINT32 MusicService(void)
  Author        :  zs
  Description   :
  Input         :
  Return        :
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
UINT32 MusicService(void)
{
    int ret = 0;
    UINT16 i, j;
    UINT16 tempFileNum;
    INT16  FindFileResult;
    //AudioFileInput(pRawFileCache);

    //error end
    if (Task.TaskID != TASK_ID_MUSIC)
    {
        if (GetMsg(MSG_AUDIO_FILE_ERROR))
        {
            AudioErrorFileCount++;

            if ((AudioFileInfo.TotalFiles <= AudioErrorFileCount) ||
                (AUDIO_ONCE == pAudioRegKey->RepeatMode) ||
                (AUDIO_REPEAT == pAudioRegKey->RepeatMode) ||
                (AUDIO_REPEAT1 == pAudioRegKey->RepeatMode))
            {
                AudioStop(AUDIO_STOP_FORCE);
            }
            else
            {
                AudioStop(AUDIO_STOP_NORMAL);
            }
        }
    }

    //normal end
    if (GetMsg(MSG_AUDIO_DECODE_END))
    {
        AudioErrorFileCount = 0;

        if (gpMediaBlock.DecodeErr == 1 && (FileInfo[(uint32)pRawFileCache].Offset <= FileInfo[(uint32)pRawFileCache].FileSize / 2)) // by chad.ma 20151016
        {
            //decode error
            //AudioStop(Audio_Stop_Force);
            SendMsg(MSG_AUDIO_DECODE_ERROR);
        }
        else
        {
            //decode end
            AudioStop(AUDIO_STOP_NORMAL);
        }
        return ret;
    }

    if (pAudioRegKey)
    {
        if (AudioPlayState == AUDIO_STATE_PLAY)
        {
            CodecGetTime(&pAudioRegKey->CurrentTime);
        }
    }

    if (GetMsg(MSG_SERVICE_MUSIC_MODE_UPDATE)) //order play
    {
//        ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
        if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER)
        {
            if ((AUDIO_FOLDER_ONCE == gSysConfig.MusicConfig.RepeatMode)||(AUIDO_FOLDER_REPEAT == gSysConfig.MusicConfig.RepeatMode)||(AUDIO_REPEAT == gSysConfig.MusicConfig.RepeatMode))
            {
                AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_DIR;
                pAudioRegKey->AudioFileDirOrAll = AudioFileInfo.Range;
                if ((pAudioRegKey->RepeatMode != AUDIO_FOLDER_ONCE) && (pAudioRegKey->RepeatMode != AUIDO_FOLDER_REPEAT) && (pAudioRegKey->RepeatMode != AUDIO_REPEAT))
                {
                    tempFileNum = GetCurFileNum(AudioFileInfo.CurrentFileNum, &AudioFileInfo.FindData, (UINT8*)MusicFileExtString, FS_FAT);
                    AudioFileInfo.TotalFiles = GetTotalFiles(AudioFileInfo.FindData.Clus,(UINT8*)MusicFileExtString, FS_FAT);
                }
                else
                {
                    tempFileNum = AudioFileInfo.CurrentFileNum;
                }
            }
            else
            {
                AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_ALL;
                pAudioRegKey->AudioFileDirOrAll = AudioFileInfo.Range;

                if ((pAudioRegKey->RepeatMode == AUDIO_FOLDER_ONCE) || (pAudioRegKey->RepeatMode == AUIDO_FOLDER_REPEAT) || (pAudioRegKey->RepeatMode == AUDIO_REPEAT))
                {
                    tempFileNum = GetGlobeFileNum(AudioFileInfo.CurrentFileNum, AudioFileInfo.FindData.Clus,(UINT8*)MusicFileExtString, FS_FAT);
                    AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, FS_FAT);
                }
                else
                {
                    tempFileNum = AudioFileInfo.CurrentFileNum;
                }
            }
            AudioFileInfo.CurrentFileNum = tempFileNum;
            AudioFileInfo.PlayedFileNum = AudioFileInfo.CurrentFileNum;

        }
        else if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
        {
            if ((AUDIO_FOLDER_ONCE == gSysConfig.MusicConfig.RepeatMode)||(AUIDO_FOLDER_REPEAT == gSysConfig.MusicConfig.RepeatMode)||(AUDIO_REPEAT == gSysConfig.MusicConfig.RepeatMode))
            {
                AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_DIR;
                pAudioRegKey->AudioFileDirOrAll = AudioFileInfo.Range;
                if ((pAudioRegKey->RepeatMode != AUDIO_FOLDER_ONCE) && (pAudioRegKey->RepeatMode != AUIDO_FOLDER_REPEAT) && (pAudioRegKey->RepeatMode != AUDIO_REPEAT))
                {
                    tempFileNum = GetCurFileNum(AudioFileInfo.CurrentFileNum, &AudioFileInfo.FindData, (UINT8*)MusicFileExtString, MUSIC_DB);
                    AudioFileInfo.TotalFiles = GetTotalFiles(AudioFileInfo.FindData.Clus,(UINT8*)MusicFileExtString, MUSIC_DB);
                }
                else
                {
                    tempFileNum = AudioFileInfo.CurrentFileNum;
                }
            }
            else
            {
                AudioFileInfo.Range = (INT16)FIND_FILE_RANGE_ALL;
                pAudioRegKey->AudioFileDirOrAll = AudioFileInfo.Range;

                if ((pAudioRegKey->RepeatMode == AUDIO_FOLDER_ONCE) || (pAudioRegKey->RepeatMode == AUIDO_FOLDER_REPEAT) || (pAudioRegKey->RepeatMode == AUDIO_REPEAT))
                {
                    tempFileNum = GetGlobeFileNum(AudioFileInfo.CurrentFileNum, AudioFileInfo.FindData.Clus,(UINT8*)MusicFileExtString, MUSIC_DB);
                    AudioFileInfo.TotalFiles = BuildDirInfo((UINT8*)MusicFileExtString, MUSIC_DB);
                }
                else
                {
                    tempFileNum = AudioFileInfo.CurrentFileNum;
                }
            }
            AudioFileInfo.CurrentFileNum = tempFileNum;
            AudioFileInfo.PlayedFileNum = AudioFileInfo.CurrentFileNum;

            //Rk Aaron.sun
            if (pAudioRegKey->RepeatMode > AUDIO_REPEAT)
            {
                pAudioRegKey->RepeatMode =  gSysConfig.MusicConfig.RepeatMode +  AUDIO_ALLONCE;
            }
            else
            {
                pAudioRegKey->RepeatMode =  gSysConfig.MusicConfig.RepeatMode ;
            }

        }
        else
        {
            pAudioRegKey->RepeatMode = gSysConfig.MusicConfig.RepeatMode;
        }
        if (0 == AudioFileInfo.TotalFiles)
        {
            return -1;
        }


#if 0  //if reat mode change need change shuffle list, please open marco
        if (AudioFileInfo.PlayOrder == AUDIO_RAND)//switch to order mode
        {
            AudioPlayFileNum = 0;
            CreateRandomList(AudioFileInfo.TotalFiles, AudioFileInfo.CurrentFileNum - 1);
        }

#endif
    }

    if (GetMsg(MSG_SERVICE_MUSIC_ORDER_UPDATE))//random or order
    {
        pAudioRegKey->PlayOrder = gSysConfig.MusicConfig.PlayOrder;// SetMusicPlayOrder;
        AudioFileInfo.PlayOrder = pAudioRegKey->PlayOrder;
        gRegAudioConfig.PlayOrder = pAudioRegKey->PlayOrder;

        if (AudioFileInfo.PlayOrder == AUDIO_RAND)//switch to order mode
        {
            AudioPlayFileNum = 0;
            CreateRandomList(AudioFileInfo.TotalFiles, AudioFileInfo.CurrentFileNum - 1);
        }
    }

#ifdef _RK_EQ_

    if ((GetMsg(MSG_AUDIO_EQSET_UPDATA)) || (GetMsg(MSG_AUDIO_EQSET_UPDATE_USER_EQ))) //update EQ
    {
        uint32 timeout;
        uint32 PlayState = AudioPlayState;
        //fade out
        if (AUDIO_STATE_PLAY == AudioPlayState)
        {
#ifdef _FADE_PROCESS_
            AudioEndFade = 0;
#if 0
            FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 32, FADE_OUT);
            FadeFinishedWait(pAudioRegKey->samplerate, FADE_OUT);
#else
            Codec_SetVolumet(0);  //fadeout --> 0
#endif
#endif
            AudioPlayState = AUDIO_STATE_PAUSE;
            timeout = 20000;
            while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    DEBUG("dma busy out");
                    break;
                }
            }

        }

        //Eq update
        {
            pAudioRegKey->UserEQ.Mode =  EqMode[gSysConfig.MusicConfig.Eq.Mode];
            AudioIOBuf.EffectCtl.Mode =  EqMode[gSysConfig.MusicConfig.Eq.Mode];
            gRegAudioConfig.UserEQ.Mode =  pAudioRegKey->UserEQ.Mode;

            if (AudioIOBuf.EffectCtl.Mode == EQ_USER)
            {
                for (i = 0; i < 5; i++)
                {
                    AudioIOBuf.EffectCtl.RKCoef.dbGain[i] = UseEqTable[gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i]];
                }
            }

            AudioDecodeProc(MSG_AUDIO_EQSET, NULL);
        }

        // Fade in
        if (AUDIO_STATE_PLAY == PlayState)
        {
            AudioErrorFrameNum = 0;
            CodecGetCaptureBuffer((short*)&AudioPtr,&AudioLen);
            memset((uint8*)AudioPtr, 0, AudioLen * sizeof(int16) * 2);
            DmaTransting = 1;
            DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),  AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);
            I2SStart(I2S_CH, I2S_START_DMA_TX);
            AudioPlayState   = AUDIO_STATE_PLAY;
            AudioDecodeing = 1;
            UserIsrRequest();
        }
    }

#endif

    if (GetMsg(MSG_MUSIC_FADE_IN))
    {
#ifdef _FADE_PROCESS_
        FadeFinishedWait(pAudioRegKey->samplerate, FADE_IN);
#else
        AudioPlayInfo.VolumeCnt = AudioPlayInfo.playVolume;
        Codec_SetVolumet(AudioPlayInfo.playVolume);
        UserIsrDisable();
        SendMsg(MSG_MUSIC_FADE_OK);
        UserIsrEnable();
#endif
    }

    /*chad.ma add for reconnect bt*/
#ifdef _BLUETOOTH_
    if(TRUE == GetMsg(MSG_BLUETOOTH_RECONNECT))
    {
        DEBUG("Get msg : BLUETOOTH RECONNECT ");
        if (!gbBTConnected && gSysConfig.BtConfig.PairedDevCnt && BtWinStatus == BT_WIN_STATUS_IDLE )
        {
            DEBUG("###### here Will ReConnect BT ######");
            BtWinStatus = BT_WIN_STATUS_CONNECTING;
            bt_a2dp_connect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothReConnectResult);
        }
    }
#endif

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name :  void MusicDeInit(void)
  Author        :
  Description   :
  Input         :
  Return        :
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void MusicDeInit(void)
{
    int i ;

    if (0 == AudioFileInfo.TotalFiles)
    {
        return;
    }

    AudioDecodeProc(MSG_AUDIO_STOP, (void*)AUDIO_STOP_FORCE);

    if (AudioPlayState != AUDIO_STATE_PAUSE)
    {
        AutoPowerOffEnable();
    }

    AudioPlayState = AUDIO_STATE_STOP;

    Codec_ExitMode(Codec_DACoutHP);

    DeRegMBoxDecodeSvc();
    DeRegMBoxFileSvc();
    BBSystemDeInit();
    DEBUG("Music Thread Exit");
}
#endif

/*
********************************************************************************
*
*                         End of AudioControl.c
*
********************************************************************************
*/

