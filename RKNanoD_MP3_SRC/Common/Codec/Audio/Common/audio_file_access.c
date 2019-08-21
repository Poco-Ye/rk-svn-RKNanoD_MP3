/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:   audio_file_access.c
*
* Description:  Audio File Operation Interface
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung    2009-01-08         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "audio_main.h"
#include "audio_file_access.h"
#include <stdio.h>
#include <string.h>
#include "FsInclude.h"
#include "File.h"

/*
*-------------------------------------------------------------------------------
*
*                           type define
*
*-------------------------------------------------------------------------------
*/
typedef unsigned int size_t;


size_t   (*RKFIO_FOpen)(uint8 * /*shortname*/, int32 /*DirClus*/, int32 /*Index*/, FS_TYPE /*FsType*/, uint8* /*Type*/) ;
size_t   (*RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
int      (*RKFIO_FSeek)(long int /*offset*/, int /*whence*/ , FILE * /*stream*/);
long int (*RKFIO_FTell)(FILE * /*stream*/);
size_t   (*RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);
unsigned long (*RKFIO_FLength)(FILE *in /*stream*/);
int      (*RKFIO_FClose)(FILE * /*stream*/);
int (*RKFIO_FEof)(FILE *);

FILE *pRawFileCache, *pFlacFileHandleBake, *pAacFileHandleSize, *pAacFileHandleOffset;

/*
*-------------------------------------------------------------------------------
*
*                           AudioFile Buffer define
*
*-------------------------------------------------------------------------------
*/
#ifdef A_CORE_DECODE
__align(4)
_ATTR_AUDIO_BSS_ uint8   AudioBuffer[AUDIO_BUF_SIZE];
_ATTR_AUDIO_BSS_ uint8  *AudioFileBuf;
_ATTR_AUDIO_BSS_ char   *AudioCodecBuf;
_ATTR_AUDIO_BSS_ uint32  CodecBufSize;
_ATTR_AUDIO_BSS_ uint32  AudioFilePIPOBufSize;     //分配给文件缓冲PIPO Buffer 的大小,总大小AudioFilePIPOBufSize x 2
                                                   //不同的文件，其大小不一样
_ATTR_AUDIO_BSS_ uint32  AudioFileBufSize[2];
_ATTR_AUDIO_BSS_ uint32  AudioFileBufPos;
_ATTR_AUDIO_BSS_ uint32  AudioFileRdBufID;
_ATTR_AUDIO_BSS_ uint32  AudioFileWrBufID;

#endif

/*
--------------------------------------------------------------------------------
  Function name : File access interface
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                                    2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
unsigned long RKFLength(FILE *in)
{
    return (FileInfo[(int)in].FileSize); // modified by huweiguo, 09/04/11

}

unsigned long RKFTell(FILE *in)
{
    return (FileInfo[(int)in].Offset);
}

void RKFileFuncInit(void)
{
    RKFIO_FOpen   = FileOpen;
    RKFIO_FLength = RKFLength;
    RKFIO_FRead   = FileRead;
    RKFIO_FWrite  = FileWrite;
    RKFIO_FSeek   = FileSeek;
    RKFIO_FTell   = RKFTell;
    RKFIO_FClose  = FileClose;
    RKFIO_FEof = FileEof;
}

 _ATTR_FLACDEC_TEXT_
int FLAC_FileSeekFast(int offset, int clus, FILE *in)
{
    FileInfo[(int)in].Offset = offset;
    FileInfo[(int)in].Clus   = clus;
    return 0;
}

_ATTR_FLACDEC_TEXT_
int FLAC_FileGetSeekInfo(int *pOffset, int *pClus, FILE *in)
{
    *pOffset = FileInfo[(int)in].Offset;
    *pClus   = FileInfo[(int)in].Clus;
    return 0;
}

#ifdef A_CORE_DECODE
/*
--------------------------------------------------------------------------------
  Function name : AudioFileBufferInit with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFileBufferInit(HANDLE Handle)
{
    AudioFileBufPos      = 0;
    AudioFileRdBufID     = 0;
    AudioFileBufSize[AudioFileRdBufID]  = FileRead((uint8*)(&AudioFileBuf[AudioFileRdBufID * AudioFilePIPOBufSize]), AudioFilePIPOBufSize, Handle);

    AudioFileWrBufID     = 1;
    AudioFileBufSize[AudioFileWrBufID]  = 0;

    SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
}

/*
--------------------------------------------------------------------------------
  Function name : AudioFileBufferSwitch with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFileBufferSwitch(HANDLE Handle)
{
    //if the second buffer empty, fill buffer
    AudioFileBufPos = 0;
    AudioFileBufSize[AudioFileRdBufID] = 0;
    AudioFileWrBufID = AudioFileRdBufID;
    AudioFileRdBufID = 1 - AudioFileRdBufID;

    if (AudioFileBufSize[AudioFileRdBufID] == 0)
    {
        AudioFileBufSize[AudioFileRdBufID]  = FileRead((uint8*)(&AudioFileBuf[AudioFileRdBufID * AudioFilePIPOBufSize]), AudioFilePIPOBufSize, Handle);
    }

    SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
}

/*
--------------------------------------------------------------------------------
  Function name : AudioFileInput with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint32 AudioFileInput(HANDLE Handle)
{
    if (GetMsg(MSG_AUDIO_DECODE_FILL_BUFFER))
    {
        AudioFileBufSize[AudioFileWrBufID] = FileRead((uint8*)(&AudioFileBuf[AudioFileWrBufID * AudioFilePIPOBufSize]), AudioFilePIPOBufSize, Handle);
    }
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : AudioFileRead with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint32 AudioFileRead(uint8 *pData, uint32 NumBytes, HANDLE Handle)
{
    uint32 remain;
    uint32 retval;
    uint32 readcnt;
    uint32 temp;

    readcnt = 0;
    while(NumBytes)
    {
        //printf("\nread = %d", NumBytes);

        /*
        if ((AudioFileBufSize[AudioFileRdBufID] < AudioFilePIPOBufSize) && (AudioFileBufPos >= AudioFileBufSize[AudioFileRdBufID]))
        {
            break;
        }
        */


    retry:
        remain = AudioFileBufSize[AudioFileRdBufID] - AudioFileBufPos;

        //printf("\nremain = %d", remain);

        if (NumBytes < remain)
        {
            memcpy(pData, &AudioFileBuf[AudioFileRdBufID * AudioFilePIPOBufSize + AudioFileBufPos], NumBytes);
            readcnt         += NumBytes;
            pData           += NumBytes;
            AudioFileBufPos += NumBytes;
            if (AudioFileBufPos >= AudioFileBufSize[AudioFileRdBufID])
            {
                AudioFileBufPos = 0;
                AudioFileBufferSwitch(Handle);
            }
            break;
        }
        else
        {
            //read buffer remain data, buffer empty
            memcpy(pData, &AudioFileBuf[AudioFileRdBufID * AudioFilePIPOBufSize + AudioFileBufPos], remain);
            NumBytes        -= remain;
            readcnt         += remain;
            pData           += remain;
            AudioFileBufPos += remain;
            if (AudioFileBufPos >= AudioFileBufSize[AudioFileRdBufID])
            {
                //printf("\nNumBytes = %d, remain = %d",NumBytes, remain);
                AudioFileBufPos = 0;
                AudioFileBufferSwitch(Handle);
                if(AudioFileBufSize[AudioFileRdBufID] == 0)
                {
                    break;
                }
            }

            goto retry;
        }

    }


    return readcnt;
}

/*
--------------------------------------------------------------------------------
  Function name : AudioFileTell with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
unsigned long AudioFileTell(FILE *in)
{
    return (FileInfo[(int)in].Offset - ((AudioFileBufSize[0] + AudioFileBufSize[1]) - AudioFileBufPos));
}

/*
--------------------------------------------------------------------------------
  Function name :  with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
bool AudioFileEof(FILE *in)
{
    if(FileInfo[(int)in].Offset == FileInfo[(int)in].FileSize)
    {
        if(((AudioFileBufSize[0] + AudioFileBufSize[1]) - AudioFileBufPos) == 0)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}



/*
--------------------------------------------------------------------------------
  Function name : AudioFileSeek with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint8 AudioFileSeek(int32 offset, uint8 Whence, HANDLE Handle)
{
    uint8 ret = RETURN_OK;
    uint32 FileBuffStart, FileBufcurPos, FileBufferEnd;
    uint32 temp;

    if (Whence == SEEK_SET)
    {
        //printf("\n audio file seek = whence ---%d, %d",Whence, offset);

        FileBufferEnd = FileInfo[(int)Handle].Offset;
        FileBuffStart = FileInfo[(int)Handle].Offset - (AudioFileBufSize[0] + AudioFileBufSize[1]);

        if ((FileBuffStart <= offset) && (offset < FileBufferEnd))
        {
            temp = offset - FileBuffStart;
            if (temp < AudioFileBufSize[AudioFileRdBufID])
            {
                AudioFileBufPos = temp;
            }
            else
            {
                temp -= AudioFileBufSize[AudioFileRdBufID];
                AudioFileBufPos = temp;

                AudioFileBufSize[AudioFileRdBufID] = 0;
                AudioFileWrBufID = AudioFileRdBufID;
                AudioFileRdBufID = 1 - AudioFileRdBufID;
                SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
            }
        }
        else
        {
            ret = FileSeek((offset / 512) * 512, Whence, Handle);
            AudioFileBufferInit(Handle);
            AudioFileBufPos += (offset % 512);
        }

    }
    else if (Whence == SEEK_CUR)
    {
        temp = AudioFileBufPos + offset;
        if ((0 <= temp) && (temp < (AudioFileBufSize[0] + AudioFileBufSize[1])))
        {
            if (offset > 0)
            {
                AudioFileBufPos += offset;
                if (AudioFileBufPos > AudioFileBufSize[AudioFileRdBufID])
                {
                    AudioFileBufPos -= AudioFileBufSize[AudioFileRdBufID];
                    AudioFileBufSize[AudioFileRdBufID] = 0;
                    AudioFileWrBufID = AudioFileRdBufID;
                    AudioFileRdBufID = 1 - AudioFileRdBufID;
                    SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
                }
            }
            else if (offset < 0)
            {
                AudioFileBufPos += offset;
            }
        }
        else
        {
             FileBuffStart = FileInfo[(int)Handle].Offset - (AudioFileBufSize[0] + AudioFileBufSize[1]);

            FileBuffStart = FileBuffStart + temp;

            ret = FileSeek((FileBuffStart / 512) * 512, 0, Handle);
            AudioFileBufferInit(Handle);
            AudioFileBufPos += (FileBuffStart % 512);
        }
    }
    else
    {
        ret = FileSeek((offset / 512) * 512, Whence, Handle);
        AudioFileBufferInit(Handle);
        AudioFileBufPos += (offset % 512);
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : File access with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFileFuncInit(HANDLE Handle, uint32 CodecBufSize)
{
    RKFIO_FOpen   = FileOpen;
    RKFIO_FLength = RKFLength;
    RKFIO_FRead   = AudioFileRead;
    RKFIO_FSeek   = AudioFileSeek;
    RKFIO_FTell   = AudioFileTell;
    RKFIO_FClose  = FileClose;
    RKFIO_FEof = AudioFileEof;

    AudioFileBuf  = (uint8 *)AudioBuffer;
    AudioFilePIPOBufSize = ((AUDIO_BUF_SIZE - CodecBufSize) / 1024) * 1024;
    AudioFilePIPOBufSize = AudioFilePIPOBufSize >> 1;

    AudioFileBufferInit(Handle);
}


_ATTR_AUDIO_TEXT_
void AudioFileChangeBuf(HANDLE Handle, uint32 CodecBufSize)
{
    AudioFilePIPOBufSize = ((AUDIO_BUF_SIZE - CodecBufSize) / 1024) * 1024;
    AudioFilePIPOBufSize = AudioFilePIPOBufSize >> 1;
    FileSeek(((FileInfo[Handle].Offset - AudioFileBufSize[0] - AudioFileBufSize[1])), SEEK_SET, Handle);

    AudioFileBufSize[AudioFileRdBufID]  = FileRead((uint8*)(&AudioFileBuf[AudioFileRdBufID * AudioFilePIPOBufSize]), AudioFilePIPOBufSize, Handle);

    AudioFileBufSize[AudioFileWrBufID]  = 0;

    SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);

}
#endif

/*
********************************************************************************
*
*                         End of Audio_file_access.c
*
********************************************************************************
*/



