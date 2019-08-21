/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  VideoControl.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               ZS      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _AVI_FILE_H_
#define _AVI_FILE_H_


#undef  EXT
#ifdef _IN_AVI_FILE_
#define EXT
#else
#define EXT extern
#endif


typedef unsigned long       DWORD;

#define     ERROR_FRAME                 -1
#define     P_FRAME                      0
#define     I_FRAME                      1
#define     FILE_END                     2
#define     NOP_FRAME                    3

//max resolution:
#if (LCD_DIRECTION == LCD_VERTICAL)
 	#define     MAX_FRAME_WIDTH             LCD_HEIGHT
	#define     MAX_FRAME_HEIGHT            LCD_WIDTH
#else
 	#define     MAX_FRAME_WIDTH             LCD_WIDTH
	#define     MAX_FRAME_HEIGHT            LCD_HEIGHT
#endif	

/* 文件指针调整方式 */
#define 	SEEK_SET    				0           		//从文件开始处移动文件指针
#define 	SEEK_CUR    				1           		//从文件当前位置移动文件指针
#define 	SEEK_END    				2           		//从文件尾移动文件指针
#define 	SEEK_REF    				3           		//从文件参考点开始

//Signatures define
#define     SIGN_RIFF                   0x46464952          //'RIFF'
#define     SIGN_AVI_                   0x20495641          //'AVI '
#define     SIGN_LIST                   0x5453494C          //'LIST'
#define     SIGN_HDRL                   0x6C726468          //'hrdl'
#define     SIGN_AVIH                   0x68697661          //'avih'
#define     SIGN_STRL                   0x6C727473          //'strl'
#define     SIGN_STRH                   0x68727473          //'strh'
#define     SIGN_STRF                   0x66727473          //'strf'
#define     SIGN_STRD                   0x64727473          //'strd'
#define     SIGN_VIDS                   0x73646976          //'vids'
#define     SIGN_XVID                   0x58564944          //'XVID'
#define     SIGN_AUDS                   0x73647561          //'auds'
#define     SIGN_INFO                   0x4F464E49          //'INFO'
#define     SIGN_IDX1                   0x31786469          //'idx1'
#define     SIGN_MOVI                   0x69766F6D          //'movi'
#define     SIGN_JUNK                   0x4a554e4b          //'KUNK'

//Audio Codec Supported
#define SIGN_WAVE_FORMAT_MPEG           0x0050              //mp1,mp2
#define SIGN_WAVE_FORAMT_MPEGLAYER3     0x0055              //mp3
#define SIGN_WAVE_FORAMT_AC3            0x2000              //ac3
#define SIGN_WAVE_FORAMT_WMA            0x0161              //WMA
#define SIGN_WAVE_FORAMT_ADPCM          0x0002              //ADPCM
#define SIGN_WAVE_FORAMT_PCM            0x0001              //PCM

//index chunk id
#define AUDIO_INDEX                     0x6277              //'wb'
#define VIDEO_INDEX                     0x6364              //'dc'

//Index signatures define
#define SIGN_db                         0x62640000          //'db'
#define SIGN_dc                         0x63640000          //'dc'
#define SIGN_wc                         0x63770000          //'wc'

#define AVI_FILE_PARSING_OK              0
#define AVI_FILE_FORMAT_ERR             -1
#define AVI_FILE_INDX_ERR               -2
#define AVI_FILE_NO_MOVI_CHUNK          -3
#define AVI_FILE_RESOLUTION_ERR         -4
#define AVI_FILE_VIDEO_CODEC_ERR        -5
#define AVI_FILE_AUDIO_CODEC_ERR        -6
#define AVI_FILE_OPEN_ERR               -7
#define AVI_FILE_NO_AUDIO_VIDEO_STREAM  -8

#define FLAG_INDEX                      0x00000010          //Index flag



//AVI Format define
#define             AVI_AUDIO_ONLY                      1
#define             AVI_VIDEO_ONLY                      2
#define             AVI_AUDIO_VIDEO                     3

#define             NUMBER_OF_FRAME_RATE                9  //所支持的最多的码率

//Mp4 Decode Buffer define
//#define             MP4_PIPO_BUF_WORDSIZE              512  
//#define             MP4_PIPO_BUF_BYTESIZE              1024
//#define             MP4_BUF_WORDSIZE                   1024

//char                *pAudioFileReadBuf;
//char                AudioInputByteBuffer[MP4_PIPO_BUF_WORDSIZE];
//char                AudioInputBuffer[MP4_BUF_WORDSIZE];


//char                *pAviMp4FileReadBuf;
//char                VideoInputByteBuffer[MP4_PIPO_BUF_WORDSIZE];
//char                MP4InputBuffer[MP4_BUF_WORDSIZE];

EXT INT16               AviFrameRateIndex;
EXT INT32               Play_Frame_Rate;

EXT UINT32              TotalAudioChunkNum;
EXT UINT32              AudioIndexCount;

EXT INT16               AudioIndexBuff_ptr;
EXT INT16               AudioIndexBuff_valid;

EXT UINT32              TotalVideoChunkNum;
EXT UINT32              VideoIndexCount;

EXT INT16               Mp4DecodeState;
EXT int                 Only_Video_Mark;

EXT UINT32 				AviAudioIndexRead;
EXT UINT32 				AviVideoIndexRead;
//-----------------------------------------------------------------------------

typedef struct
{
    DWORD  dwMicroSecPerFrame; // 单位为us
    DWORD  dwMaxBytesPerSec;
    DWORD  dwReserved1;
    DWORD  dwFlags;
    DWORD  dwTotalFrames;
    DWORD  dwInitialFrames;
    DWORD  dwStreams;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwWidth;
    DWORD  dwHeight;
    DWORD  dwScale;
    DWORD  dwRate;
    DWORD  dwStart;
    DWORD  dwLength;
} MainAVIHeader; //avi header, from "Video for Windows Programmer's Guide"

typedef struct {

unsigned long AVI_SIGN_RIFF;
unsigned long AVI_TotalSize;
unsigned long AVI_SIGN_AVI_;
unsigned long AVI_SIGN_LIST;
unsigned long Avi_List_Length;
unsigned long Avi_Sign_Hdrl;
unsigned long Avi_Sign_Avih;
unsigned long Avi_Avih_Length;
MainAVIHeader AviHeader;

}AVI_FILE_PARSING;

typedef struct
{
    char    VideoCodecLib;
    char    VideoSupportedFlag;
    char    AudioCodecLib;
    char    AudioSupportedFlag;
}STREAMFLAG;

typedef struct
{
    
    UINT16  playfreq;                               //Cpu Freq
    UINT16  fps;                                    //frame number per second
    UINT32  mspf;                                   //micro-second per frame
    
} AVI_FRAME_RATE_TABLE;

#ifdef  _IN_AVI_SERVICE     
 AVI_FRAME_RATE_TABLE    AviFrameRateTab[NUMBER_OF_FRAME_RATE]  = 
{
                           
    {72, 15, (UINT32)66666},
    {74, 16, (UINT32)62500},
    {76, 18, (UINT32)55555},
    {78, 20, (UINT32)50000},
    {78, 22, (UINT32)45454},
    {80, 23, (UINT32)41708},
    {80, 24, (UINT32)41666},
    {82, 25, (UINT32)40000},
    {82, 30, (UINT32)33333}
};                         
#else                      
EXT AVI_FRAME_RATE_TABLE    AviFrameRateTab[NUMBER_OF_FRAME_RATE];
#endif                     
//----------------------------------------------------------------------
//音视频INDEX的缓冲buffer大小,此buffer越大，越可以减少读Flash的次数,加快快进和快退的速度,
#define             AviIndexBufferSize                 256 //这个值必须为16的整数倍

typedef struct
{
    DWORD ReadSize;     /* size read out. */
    DWORD CurOffset;
    DWORD CurChunkSize;
}CurChunk;

typedef struct {
    UINT32          hAudioData;
    UINT32          hAudioIndex;
    CurChunk        Audio;                  /* current video chunk information. */
    UINT32          Audio_Current_FrameNum;
    UINT32          Audio_FilePos;
    UINT32          Audio_ChunkLength;
    unsigned int    AviAudioIndexBufferOffset;
    unsigned char   AviAudioIndexBuffer[AviIndexBufferSize];

}AUDIO_CHUNK_INFO;

typedef struct {
    UINT32              hVideoData;
    UINT32              hVideoIndex;
    CurChunk           Video;                  /* current video chunk information. */
    UINT32              Video_Current_FrameNum;
    UINT32              Video_FilePos;
    UINT32              Video_ChunkLength;
    unsigned int        AviVideoIndexBufferOffset;
    unsigned char       AviVideoIndexBuffer[AviIndexBufferSize];

}VIDEO_CHUNK_INFO;


extern AUDIO_CHUNK_INFO        Audio_chunk_info;
extern VIDEO_CHUNK_INFO        Video_chunk_info;



typedef struct {
    
    UINT16  AviFileFormat;                          // 1：音频流
                                                    // 2：视频流
                                                    // 3：音视频流 
                                                    // ：非法流格式
                                                    
    UINT32  MicroSecondPerFrame;                    //   每贞的播放时间，以uS为单位
    UINT32  TotalFrameCount;                        //   总贞数
                                                    
    UINT32  StartDataPos;                           //   音视频数据块的起始文件偏移地址
    UINT32  StartIndexPos;                          //   音视频索引块的起始文件偏移地址
    UINT32  AudioSamplingRate;                      //   音频的采样率
    
} AVI_STREAM_INFO;

extern STREAMFLAG              stream_supported_flag;
extern AVI_STREAM_INFO         AviStreamInfo;


extern void AviDecodeInit(void);
extern int VideoDecodeUninit(void);

extern int AviGetVideoData(char *pbuf, int size, int *end_frame);
extern int AviGetAudioData(char *pbuf, int size);
extern int AviVideoSeek(int nFrame, int direction);
extern int SyncAudio2Video(void);
extern int SyncVideo2Audio(void);


#endif 


