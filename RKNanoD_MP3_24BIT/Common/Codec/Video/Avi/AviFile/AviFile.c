/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name： VideoControl.C
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                 ZS      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#pragma arm section code = "AviDecCode", rodata = "AviDecCode", rwdata = "AviDecData", zidata = "AviDecBss"

#define _IN_AVI_FILE_


#include "SysInclude.h"
#ifdef _VIDEO_

#include "SysFindFile.h"
#include "AviFile.h"

#define NOT_OPEN_FILE -1
#define _IN_AVI_SERVICE

/*


*/
#define AVI_FAST_SEEK

#ifdef AVI_FAST_SEEK
int AviFastSeekInit(UINT32 fp)
{
	if(fp >= MAX_OPEN_FILES)
	{
		return -1;
	}
	
	FileInfo[fp].RefOffset = FileInfo[fp].Offset;
	FileInfo[fp].RefClus = FileInfo[fp].Clus;
	return 0;
}
#endif	

AUDIO_CHUNK_INFO        Audio_chunk_info;
VIDEO_CHUNK_INFO        Video_chunk_info;
STREAMFLAG              stream_supported_flag;
AVI_STREAM_INFO         AviStreamInfo;


unsigned long Byte2Long(UINT8 * data)
{
   return ((((unsigned long)data[3])<<24)
                         |(((unsigned long)data[2])<<16)
                         |(((unsigned long)data[1])<<8)
                         |(((unsigned long)data[0])));   
}

unsigned short Byte2Short(UINT8 * data)
{
   return (((unsigned short)data[1])<<8 |(((unsigned short)data[0])));   
}
/********************************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : AviFileParsing()
*  Description:  :  Parse an .AVI file      
*  Input:        :  Handle, avi_file_information
*  Output:       :  struct :avi_file_information
*  Return:       :     0: sucess;
                    other :error
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
*
********************************************************************************************/
INT16  AviFileParsing(INT32 Handle,AVI_STREAM_INFO *avi_file_information)
{
    UINT32  hAVIFile;
    unsigned 	char   	*TmpDataReadout;
    unsigned 	char   	ucReadoutBuffer[128];
    unsigned 	char   	ReadoutNum;
    unsigned 	int    	blHaveIndex = 0;
    unsigned 	int    	AudioVedioInfo = 0;     
    //unsigned 	long   	TotalFrames;
    unsigned 	int    	i;
    unsigned 	long   	SeekTmp;   
    unsigned 	long   	SeekSmall;       
    unsigned 	long   	MoviStartPos = 0;    
    unsigned 	long   	IndexStartPos = 0;
    AVI_FILE_PARSING   *Avi_File_HeaderData ;

    avi_file_information ->AviFileFormat = 0;         
        
    hAVIFile = Handle;

    Avi_File_HeaderData = ( AVI_FILE_PARSING   *)ucReadoutBuffer;
    TmpDataReadout = ucReadoutBuffer;
    
    ReadoutNum = AVI_FRead(ucReadoutBuffer, 88, hAVIFile);

    MoviStartPos += 88;

    if(ReadoutNum < 88)
    {
        return AVI_FILE_INDX_ERR;                                
    }
    
    if(Avi_File_HeaderData->AVI_SIGN_RIFF != SIGN_RIFF)
    {
        return AVI_FILE_FORMAT_ERR;
    }
    
    if(Avi_File_HeaderData->AVI_SIGN_AVI_ != SIGN_AVI_)
    {
        return AVI_FILE_FORMAT_ERR;
    }
    if(Avi_File_HeaderData->AVI_SIGN_LIST != SIGN_LIST)    
    {
        return AVI_FILE_FORMAT_ERR;
    }

    if(Avi_File_HeaderData->Avi_Sign_Hdrl != SIGN_HDRL)
    {
        return AVI_FILE_FORMAT_ERR;
    }
    
    if(Avi_File_HeaderData->Avi_Sign_Avih != SIGN_AVIH)
    {
       return AVI_FILE_FORMAT_ERR;
    }
   /*************************************************
   [2] Check if audio stream is included.
   **************************************************/
   
   stream_supported_flag.VideoSupportedFlag = TRUE;
   
   /* We judge by AVI stream number here. It may be wrong. */
   if (Avi_File_HeaderData->AviHeader.dwStreams == 1)
   {
       stream_supported_flag.AudioSupportedFlag = FALSE;
   }
   else
   {
       stream_supported_flag.AudioSupportedFlag = TRUE;
   }
   
    /**************************************************
    [3] Check if resolution and frame rate is supported.
    ***************************************************/    
   if((Avi_File_HeaderData->AviHeader.dwWidth != MAX_FRAME_WIDTH) || (Avi_File_HeaderData->AviHeader.dwHeight != MAX_FRAME_HEIGHT))// 屏的大小要和视频大小相匹配!!!zs 06.29
   {
        return AVI_FILE_FORMAT_ERR;       
   } 
    
   /* Max frame rate: 100fps. */
   if (Avi_File_HeaderData->AviHeader.dwMicroSecPerFrame < 10000)
   {
       return AVI_FILE_FORMAT_ERR;
   }
   Play_Frame_Rate = Avi_File_HeaderData->AviHeader.dwMicroSecPerFrame;    
   
   /*************************************************
   [4] Check if index is included.
   **************************************************/
   
   if ((Avi_File_HeaderData->AviHeader.dwFlags & FLAG_INDEX) == 0)
       return AVI_FILE_INDX_ERR;
   blHaveIndex = 1;
       
   /*************************************************
   [5] Read stream header.
   **************************************************/
   for(i = 0; i < Avi_File_HeaderData->AviHeader.dwStreams; i++)
   {
       ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);             // 'LIST' and list length
       MoviStartPos += 8;
       SeekTmp = Byte2Long(TmpDataReadout + 4);
                                    
       ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile); 
       MoviStartPos += 8;
       SeekTmp -= 8;
       
       if(Byte2Long(TmpDataReadout) != SIGN_STRL)
       {
           return AVI_FILE_FORMAT_ERR;        
       }
   
       if(Byte2Long(TmpDataReadout + 4) != SIGN_STRH)
       {
           return AVI_FILE_FORMAT_ERR;        
       }
       ReadoutNum = AVI_FRead(TmpDataReadout, 4, hAVIFile);            // Read out size of 'strh' 
       SeekSmall =  Byte2Long(TmpDataReadout);
   
       MoviStartPos += 4;
       SeekTmp -= 4;
                                
       ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);        
       MoviStartPos += 8; 
       SeekTmp -= 8;
       SeekSmall -= 8;
       
       if ((TmpDataReadout[0] == 'v') && (TmpDataReadout[1] == 'i')
        && (TmpDataReadout[2] == 'd') && (TmpDataReadout[3] == 's'))
      // if(Byte2Long(TmpDataReadout) == SIGN_VIDS)
      {
          if ((TmpDataReadout[4] != 'X') || (TmpDataReadout[5] != 'V')
            || (TmpDataReadout[6] != 'I') || (TmpDataReadout[7] != 'D'))
        // if(Byte2Long(TmpDataReadout + 4) != SIGN_XVID)
           {
               return AVI_FILE_FORMAT_ERR;                       // Video format is not supported
           }
           (avi_file_information ->AviFileFormat) |= 2;           // Have Video stream, set the flag
       }
       else if(Byte2Long(TmpDataReadout) == SIGN_AUDS)  
       {
           AVI_FSeek(SeekSmall, SEEK_CUR, hAVIFile);
           MoviStartPos += SeekSmall;
           SeekTmp -= SeekSmall;
           
           ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);
           MoviStartPos += 8;
           SeekTmp -= 8;
            
           if ((TmpDataReadout[0] != 's') || (TmpDataReadout[1] != 't')
            || (TmpDataReadout[2] != 'r') || (TmpDataReadout[3] != 'f'))
         // if(Byte2Long(TmpDataReadout) != SIGN_STRF)
            {
               return AVI_FILE_FORMAT_ERR;                // File format error
           }
           ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);
           MoviStartPos += 8;
           SeekTmp -= 8;    
                              
          if(Byte2Short(TmpDataReadout) != SIGN_WAVE_FORMAT_MPEG)
           {
               return AVI_FILE_FORMAT_ERR;                // Audio format is not supported
           }
           
           avi_file_information ->AudioSamplingRate = Byte2Long(TmpDataReadout + 4);
   
           (avi_file_information ->AviFileFormat) |= 1;           // Have audio stream, set the flag
          
       }
       else
       {
           return AVI_FILE_FORMAT_ERR;   
       }
       
       AVI_FSeek(SeekTmp, SEEK_CUR, hAVIFile);
       MoviStartPos += SeekTmp;
       
       while(1)
       {
           ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);
           if(ReadoutNum < 8)
           {
               return AVI_FILE_FORMAT_ERR;   
           }
           
           if(Byte2Long(TmpDataReadout) == SIGN_JUNK)
           {
               SeekTmp = Byte2Long(TmpDataReadout + 4);
               AVI_FSeek(SeekTmp, SEEK_CUR, hAVIFile);
               MoviStartPos += (SeekTmp + 8);                                                         
           }
           else
           {
               AVI_FSeek(-8, SEEK_CUR, hAVIFile);          
               break;
           }
       }        
   }
       
   //////////////  Find movi chunk ////////////////
   ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);
   MoviStartPos += 8;
   
   while(!AVI_FileEof(hAVIFile))
   {
       if(Byte2Long(TmpDataReadout)== SIGN_LIST)
       {
              SeekTmp = Byte2Long(TmpDataReadout +4);                                                    
              ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);
              MoviStartPos += 8; 
              SeekTmp -= 8;
              
           if(Byte2Long(TmpDataReadout) == SIGN_MOVI)
           {
               IndexStartPos = SeekTmp + MoviStartPos; 
               break;                      // Find FOURCC 'movi'
           }
           AVI_FSeek(SeekTmp, SEEK_CUR, hAVIFile);
           ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile); 
           MoviStartPos += (8 + SeekTmp);
       }      
       else  if(Byte2Long(TmpDataReadout) == SIGN_JUNK)
       {
             SeekTmp =  Byte2Long(TmpDataReadout +4);
             AVI_FSeek(SeekTmp, SEEK_CUR, hAVIFile);
             ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile); 
             MoviStartPos += (8 + SeekTmp);
       }
       else 
       {
             SeekTmp =  Byte2Long(TmpDataReadout +4);
             AVI_FSeek(SeekTmp, SEEK_CUR, hAVIFile);
             ReadoutNum = AVI_FRead(TmpDataReadout, 8, hAVIFile);
             MoviStartPos += (8 + SeekTmp);
       }  
   }
   
   if(AVI_FileEof(hAVIFile))
   {              
       return AVI_FILE_NO_MOVI_CHUNK;           // If find 'movi' in the end of file ,return error.
   }
   if (((avi_file_information ->AviFileFormat) & 0x3) == 0)
   {
       return AVI_FILE_NO_AUDIO_VIDEO_STREAM ;  // No audio and video stream, not support it
   }
   
   //'movi'的start addr
   avi_file_information->StartDataPos = MoviStartPos - 8;
             
   if(blHaveIndex == 1)
   {
   
       avi_file_information ->StartIndexPos = (IndexStartPos + 8);     
       
       AVI_FSeek(avi_file_information->StartIndexPos-4, SEEK_SET, Audio_chunk_info.hAudioIndex); 
       
       ReadoutNum = AVI_FRead(TmpDataReadout, 4,  Audio_chunk_info.hAudioIndex);
       //  FileRefSet( Audio_chunk_info.hAudioIndex);//by zs
       
       TotalAudioChunkNum = ((Byte2Long(TmpDataReadout))>>4)- Avi_File_HeaderData->AviHeader.dwTotalFrames;
       TotalVideoChunkNum = Avi_File_HeaderData->AviHeader.dwTotalFrames;
       
       if(TotalVideoChunkNum == 0)
           return AVI_FILE_NO_MOVI_CHUNK;
       
       if(TotalAudioChunkNum == 0)
           Only_Video_Mark = 1;
       else
           Only_Video_Mark = 0;
       
       AVI_FSeek(avi_file_information ->StartIndexPos, SEEK_SET, Video_chunk_info.hVideoIndex); 
           
       VideoIndexCount = 0;       
       AudioIndexCount = 0;
   }
   else
   {
       avi_file_information ->StartIndexPos = 0;  // AVI文件没有index 部分
       return AVI_FILE_INDX_ERR;          
   }
   

   Audio_chunk_info.Audio_FilePos = 4;
   Audio_chunk_info.Audio_ChunkLength = 0;
   Video_chunk_info.Video_FilePos = 4;     
   Video_chunk_info.Video_ChunkLength = 0;

   avi_file_information ->TotalFrameCount = Avi_File_HeaderData->AviHeader.dwTotalFrames;
   avi_file_information ->MicroSecondPerFrame = Avi_File_HeaderData->AviHeader.dwMicroSecPerFrame; // us

   return AVI_FILE_PARSING_OK;
}

void AviVideoVariableInit(void)
{
	memset(&AviStreamInfo, 0, sizeof(AVI_STREAM_INFO));
	memset(&Video_chunk_info, 0, sizeof(VIDEO_CHUNK_INFO));
	memset(&Audio_chunk_info, 0, sizeof(AUDIO_CHUNK_INFO));
}

void AviDecodeInit(void)
{
   // Play_Frame_Rate     = 20;    

	AviAudioIndexRead = AviIndexBufferSize;
	AviVideoIndexRead = AviIndexBufferSize;
	
    Video_chunk_info.Video_Current_FrameNum = 0;
    Audio_chunk_info.Audio_Current_FrameNum = 0;
    Video_chunk_info.AviVideoIndexBufferOffset = AviIndexBufferSize;
    Audio_chunk_info.AviAudioIndexBufferOffset= AviIndexBufferSize;   
    
  //  Play_Frame_Rate = (INT32)AviFrameRateTab[AviFrameRateIndex].fps;
    
  //  pAviMp4FileReadBuf = VideoInputByteBuffer;

}

/*
--------------------------------------------------------------------------------
  Function name :  void VideoThreadInit(void *pArg)  
  Author        :  zs
  Description   :  
                    
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>       
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/

int VideoDecodeUninit(void)
{
	if(Video_chunk_info.hVideoData != (UINT32)NOT_OPEN_FILE)
    	AVI_FClose(Video_chunk_info.hVideoData);
	if(Audio_chunk_info.hAudioData != (UINT32)NOT_OPEN_FILE)
    	AVI_FClose( Audio_chunk_info.hAudioData);
	if(Video_chunk_info.hVideoIndex != (UINT32)NOT_OPEN_FILE)
    	AVI_FClose(Video_chunk_info.hVideoIndex);
	if(Audio_chunk_info.hAudioIndex != (UINT32)NOT_OPEN_FILE)
    	AVI_FClose( Audio_chunk_info.hAudioIndex);	

	return 0;
}

/*
--------------------------------------------------------------------------------
  Function name :  void VideoThreadInit(void *pArg)  
  Author        :  zs
  Description   :  
                    
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>       
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/

int VideoDecodeInit(void)
{
    INT16  Temp;
    extern char FileOpenStringR[];
    char *path = VideoFileInfo.Path;

	char *pAviFileName = VideoFileInfo.Fdt.Name;//"OPPO1   AVI";

	AviVideoVariableInit();

    Video_chunk_info.hVideoData  = FileOpenA(path, pAviFileName, FileOpenStringR);//AVI_FOpen(pAviFileName, (UINT8 *)"R");

    Audio_chunk_info.hAudioData  = FileOpenA(path, pAviFileName, FileOpenStringR);//AVI_FOpen(pAviFileName, (UINT8 *)"R");

    Audio_chunk_info.hAudioIndex = FileOpenA(path, pAviFileName, FileOpenStringR);//AVI_FOpen(pAviFileName, (UINT8 *)"R");
    Video_chunk_info.hVideoIndex = FileOpenA(path, pAviFileName, FileOpenStringR);//AVI_FOpen(pAviFileName, (UINT8 *)"R");
        
    if ((NOT_OPEN_FILE == Video_chunk_info.hVideoData) || (NOT_OPEN_FILE ==  Audio_chunk_info.hAudioData) || (NOT_OPEN_FILE ==  Audio_chunk_info.hAudioIndex) || (NOT_OPEN_FILE == Video_chunk_info.hVideoIndex)) {
        AVI_FClose(Video_chunk_info.hVideoData);
        AVI_FClose( Audio_chunk_info.hAudioData);
        AVI_FClose(Video_chunk_info.hVideoIndex);
        AVI_FClose( Audio_chunk_info.hAudioIndex);
        return -1;
    }

    Temp = AviFileParsing(Video_chunk_info.hVideoData,&AviStreamInfo);
    
    if ((0 != Temp) || (AVI_AUDIO_VIDEO < AviStreamInfo.AviFileFormat)) {
        AVI_FClose(Video_chunk_info.hVideoData);
        AVI_FClose( Audio_chunk_info.hAudioData);
        AVI_FClose(Video_chunk_info.hVideoIndex);
        AVI_FClose( Audio_chunk_info.hAudioIndex);
        return -1;
    }

#ifdef AVI_FAST_SEEK

	AviFastSeekInit(Video_chunk_info.hVideoIndex);
	AviFastSeekInit(Audio_chunk_info.hAudioIndex);
#endif	

  //  AviFrameRateIndex = 0;
  //  for(AviFrameRateIndex = 0; AviFrameRateIndex < NUMBER_OF_FRAME_RATE; AviFrameRateIndex++)
    {
       // if(AviStreamInfo.MicroSecondPerFrame == (AviFrameRateTab[AviFrameRateIndex].mspf/10))
          //  break;
    }
#if 0
  if (NUMBER_OF_FRAME_RATE <= AviFrameRateIndex) {
        AVI_FClose(Video_chunk_info.hVideoData);
        AVI_FClose( Audio_chunk_info.hAudioData);
        AVI_FClose(Video_chunk_info.hVideoIndex);
        AVI_FClose( Audio_chunk_info.hAudioIndex);
     //   SysServiceMsg |= MSG_SERVICE_FILE_OPEN_ERROR;
        return;
    }
#endif 
    AVI_FSeek(AviStreamInfo.StartDataPos,SEEK_SET,Video_chunk_info.hVideoData);
    AVI_FSeek(AviStreamInfo.StartDataPos,SEEK_SET, Audio_chunk_info.hAudioData);

    AviDecodeInit();

	return 0;

}

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : GetNextVideoChunk()
*  Description:  
*  Input:                         
*  Output:         
*  Return:        
*  Others:        
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int GetNextVideoChunk(VIDEO_CHUNK_INFO *Video_chunk_info)
{
	unsigned char   *TmpAviIndexValue;
    unsigned long   tmp = 0;    
    unsigned int    uiReadoutLen = AviIndexBufferSize;
    unsigned long   Fileoffset = 0;    

	Video_chunk_info->Video.CurOffset = 0;

	if(Video_chunk_info->Video_Current_FrameNum >= TotalVideoChunkNum)
	{
		return FILE_END; 
	}
    
	do
    {         
        if(Video_chunk_info->AviVideoIndexBufferOffset >= uiReadoutLen)
        {
            uiReadoutLen = AVI_FRead(Video_chunk_info->AviVideoIndexBuffer, AviIndexBufferSize, Video_chunk_info->hVideoIndex);
            if(uiReadoutLen < 16)
            {
                return FILE_END;    
            }

			AviVideoIndexRead = uiReadoutLen;
            Video_chunk_info->AviVideoIndexBufferOffset = 16;
            TmpAviIndexValue = Video_chunk_info->AviVideoIndexBuffer;
        }
        else
        {
            TmpAviIndexValue = (Video_chunk_info->AviVideoIndexBuffer + Video_chunk_info->AviVideoIndexBufferOffset);
            Video_chunk_info->AviVideoIndexBufferOffset +=16;
        }

        VideoIndexCount++;

        if( Byte2Short(TmpAviIndexValue + 2) == VIDEO_INDEX)
        {
        	long distance;
			unsigned long Movi;
			//unsigned long CurVideoChunkBeginOffset;
			unsigned long CurVideoChunkUsedOffset;
			unsigned long NextVideoChunkBeginOffset;
			
            Video_chunk_info->Video_Current_FrameNum++;
			if(Video_chunk_info->Video_Current_FrameNum > TotalVideoChunkNum)
			{
				return FILE_END; 
			}
			
			//
			// get file offset - nextVideoChunk，for seek to NextVideChunk postion:
			//                                      
			//                                   distance
			//                              |               |
			//     ____________ ____________ _______________ ____________________
			//    |            |            |               |
			//  Movi           |            |               |
			//                 |            |               |
			//     CurVideoChunkBeginOffset |               |
			//                    CurVideoChunkUsedOffset   |                         
			//                                  NextVideoChunkBeginOffset        
			//
			// Movi:                     MOVI start position
			// CurVideoChunkBeginOffset: 
			// CurVideoChunkUsedOffset:  
			// NextVideoChunkBeginOffset:
			// distance:                 
			//

			Movi = AviStreamInfo.StartDataPos;
			CurVideoChunkUsedOffset = AVI_FTell(Video_chunk_info->hVideoData);
			if(CurVideoChunkUsedOffset >= Movi)
			{
				CurVideoChunkUsedOffset -= Movi;
			}
			else
			{
				CurVideoChunkUsedOffset = 0;
			}

			NextVideoChunkBeginOffset= Byte2Long(TmpAviIndexValue + 8);

			// get nextVideoChunk information
			Video_chunk_info->Video_FilePos = Byte2Long(TmpAviIndexValue + 8);
			Video_chunk_info->Video_ChunkLength = Byte2Long(TmpAviIndexValue + 12);


			if(NextVideoChunkBeginOffset >= CurVideoChunkUsedOffset)
			{
				distance = NextVideoChunkBeginOffset - CurVideoChunkUsedOffset;
			}
			else
			{
				distance = -8;
				continue; //go on search next frame
			}
			
            Video_chunk_info->Video.CurOffset = distance + 8; 
            Video_chunk_info->Video.CurChunkSize = Video_chunk_info->Video_ChunkLength ;
			if(Video_chunk_info->Video.CurChunkSize == 0)
			{
				return NOP_FRAME;
			}
            
            if((Byte2Long(TmpAviIndexValue + 4)) != 0)
            {
				return I_FRAME;       // I
            }
            else
            {
                // P/N, 
            }
        }
       else if( Byte2Short(TmpAviIndexValue + 2) != AUDIO_INDEX)
       {
         return ERROR_FRAME;
       }
    }while(TotalVideoChunkNum != Video_chunk_info->Video_Current_FrameNum);
    
    return FILE_END;      //no find index until file end
}

/******************************************************************************
*  Copyright (C),2009-2010, Fuzhou Rockchip Co.,Ltd.
*  Function name : GetPreVideoChunk()
*  Description:   
*  Input:                           
*  Output:         
*  Return:         
*  Others:         
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int GetPreVideoChunk(VIDEO_CHUNK_INFO *Video_chunk_info)
{
    unsigned char   *tmpAviIndexValue;
	unsigned int    uiReadoutLen; 	
     
    Video_chunk_info->Video.CurOffset = 0;

	if(Video_chunk_info->Video_Current_FrameNum == 0)
	{
		return FILE_END; // seek to first frame
	}	
    
    while(VideoIndexCount > 0) // is back to first frame
    {
        if(Video_chunk_info->AviVideoIndexBufferOffset < 16)
        {  
			//read data to index buffer from file

#ifdef AVI_FAST_SEEK	
			AVI_FSeek(-(int)(AviIndexBufferSize+AviVideoIndexRead), SEEK_REF, Video_chunk_info->hVideoIndex);
#else
			AVI_FSeek(-(int)(AviIndexBufferSize+AviVideoIndexRead), SEEK_CUR, Video_chunk_info->hVideoIndex);
#endif

		   	uiReadoutLen = AVI_FRead(Video_chunk_info->AviVideoIndexBuffer, AviIndexBufferSize, Video_chunk_info->hVideoIndex);
            if(uiReadoutLen < 16)
            {
                return FILE_END;    
            }

			AviVideoIndexRead = uiReadoutLen;
            Video_chunk_info->AviVideoIndexBufferOffset = uiReadoutLen - 16;
            tmpAviIndexValue = Video_chunk_info->AviVideoIndexBuffer + Video_chunk_info->AviVideoIndexBufferOffset;
        }
        else
        {
            Video_chunk_info->AviVideoIndexBufferOffset -= 16;
            tmpAviIndexValue = Video_chunk_info->AviVideoIndexBuffer + Video_chunk_info->AviVideoIndexBufferOffset;
        }

        VideoIndexCount--;
        
        if( Byte2Short(tmpAviIndexValue + 2) == VIDEO_INDEX)
        {
        	unsigned long distance;
			unsigned long Movi;
			//unsigned long CurVideoChunkBeginOffset;
			unsigned long CurVideoChunkUsedOffset;
			unsigned long PreVideoChunkBeginOffset;  

			if(Video_chunk_info->Video_Current_FrameNum == 0)
			{
				return FILE_END; // seek to first index
			}			
            Video_chunk_info->Video_Current_FrameNum--;
			

			Movi = AviStreamInfo.StartDataPos;
			CurVideoChunkUsedOffset  = AVI_FTell(Video_chunk_info->hVideoData);
			if(CurVideoChunkUsedOffset >= Movi)
			{
				CurVideoChunkUsedOffset  -= Movi;	
			}
			else
			{
				CurVideoChunkUsedOffset = 0;
			}

			PreVideoChunkBeginOffset= Byte2Long(tmpAviIndexValue + 8);


			Video_chunk_info->Video_FilePos = Byte2Long(tmpAviIndexValue + 8);
			Video_chunk_info->Video_ChunkLength = Byte2Long(tmpAviIndexValue + 12);			


			if(CurVideoChunkUsedOffset >= PreVideoChunkBeginOffset+8)
			{
				distance = CurVideoChunkUsedOffset - PreVideoChunkBeginOffset;
			}
			else
			{
				distance = 8;
			}

            Video_chunk_info->Video.CurOffset = distance - 8; 
            Video_chunk_info->Video.CurChunkSize = Video_chunk_info->Video_ChunkLength;
			if(Video_chunk_info->Video.CurChunkSize == 0)
			{
				return NOP_FRAME;
			}			

			if((Byte2Long(tmpAviIndexValue + 4)) != 0)	
			{        
                return I_FRAME; 
            }
            else
            {
                // P/N, 
            }			
        }
        else if( Byte2Short(tmpAviIndexValue + 2) != AUDIO_INDEX)
        {
            return ERROR_FRAME;
        }
    }

    return ERROR_FRAME;  
}

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : GetNextAudioChunk()
*  Description:     
*  Input:        
*  Output:      
*  Return:         
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int GetNextAudioChunk(AUDIO_CHUNK_INFO *Audio_chunk_info)
{
    unsigned char   *TmpAviIndexValue;
    unsigned long   tmp = 0;
    unsigned int    uiReadoutLen = AviIndexBufferSize;
     
    Audio_chunk_info->Audio.CurOffset = 0;

	if(Audio_chunk_info->Audio_Current_FrameNum >= TotalAudioChunkNum)
	{
		return FILE_END; // audio frame is over
	}
	
    do
    {       
        if(Audio_chunk_info->AviAudioIndexBufferOffset >= uiReadoutLen)
        {
            uiReadoutLen = AVI_FRead(Audio_chunk_info->AviAudioIndexBuffer, AviIndexBufferSize,  Audio_chunk_info->hAudioIndex);
            if(uiReadoutLen < 16)
            {
                return FILE_END;    
            }

			AviAudioIndexRead = uiReadoutLen;
            Audio_chunk_info->AviAudioIndexBufferOffset = 16;
            TmpAviIndexValue = Audio_chunk_info->AviAudioIndexBuffer;
        }
        else
        {
            TmpAviIndexValue = Audio_chunk_info->AviAudioIndexBuffer + Audio_chunk_info->AviAudioIndexBufferOffset;
            Audio_chunk_info->AviAudioIndexBufferOffset +=16;
        }
        
        AudioIndexCount++;
        
		if( Byte2Short(TmpAviIndexValue + 2) == AUDIO_INDEX)
		{
			long distance;
			unsigned long Movi;
			//unsigned long CurAudioChunkBeginOffset;
			unsigned long CurAudioChunkUsedOffset;
			unsigned long NextAudioChunkBeginOffset;

			Audio_chunk_info->Audio_Current_FrameNum++;
			if(Audio_chunk_info->Audio_Current_FrameNum > TotalAudioChunkNum)
			{
				return FILE_END; // frame is over
			}


			Movi = AviStreamInfo.StartDataPos;
			CurAudioChunkUsedOffset = AVI_FTell(Audio_chunk_info->hAudioData);
			if(CurAudioChunkUsedOffset >= Movi)
			{
				CurAudioChunkUsedOffset -= Movi;
			}
			else
			{
				CurAudioChunkUsedOffset = 0;
			}

			NextAudioChunkBeginOffset= Byte2Long(TmpAviIndexValue + 8);

			Audio_chunk_info->Audio_FilePos = Byte2Long(TmpAviIndexValue + 8);
			Audio_chunk_info->Audio_ChunkLength = Byte2Long(TmpAviIndexValue + 12);

			if(NextAudioChunkBeginOffset >= CurAudioChunkUsedOffset)
			{
				distance = NextAudioChunkBeginOffset - CurAudioChunkUsedOffset;
			}
			else
			{
				distance = -8;
			}
			
		    Audio_chunk_info->Audio.CurOffset = distance + 8; 
		    Audio_chunk_info->Audio.CurChunkSize = Audio_chunk_info->Audio_ChunkLength ;
		    return I_FRAME;
		}
		else if( Byte2Short(TmpAviIndexValue + 2) != VIDEO_INDEX)
		{
		    return ERROR_FRAME;
		}
    }while(TotalAudioChunkNum != Audio_chunk_info->Audio_Current_FrameNum);
    
    return FILE_END;      
}

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : GetPreAudioChunk()
*  Description:     
*  Input:        
*  Output:      
*  Return:         
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int GetPreAudioChunk(AUDIO_CHUNK_INFO *Audio_chunk_info)
{
    unsigned char   *tmpAviIndexValue;
	unsigned int    uiReadoutLen;	
     
    Audio_chunk_info->Audio.CurOffset = 0;

	if(Audio_chunk_info->Audio_Current_FrameNum == 0)
	{
		return FILE_END; 
	}	
    
    while(AudioIndexCount > 0) 
    {
        if(Audio_chunk_info->AviAudioIndexBufferOffset < 16)
        {  
#ifdef AVI_FAST_SEEK
			AVI_FSeek(-(int)(AviIndexBufferSize+AviAudioIndexRead), SEEK_REF, Audio_chunk_info->hAudioIndex);
#else
			AVI_FSeek(-(int)(AviIndexBufferSize+AviAudioIndexRead), SEEK_CUR, Audio_chunk_info->hAudioIndex);
#endif
		   	uiReadoutLen = AVI_FRead(Audio_chunk_info->AviAudioIndexBuffer, AviIndexBufferSize, Audio_chunk_info->hAudioIndex);
            if(uiReadoutLen < 16)
            {
                return FILE_END;    
            }

			AviAudioIndexRead = uiReadoutLen;
            Audio_chunk_info->AviAudioIndexBufferOffset = uiReadoutLen - 16;
            tmpAviIndexValue = Audio_chunk_info->AviAudioIndexBuffer + Audio_chunk_info->AviAudioIndexBufferOffset;
        }
        else
        {
            Audio_chunk_info->AviAudioIndexBufferOffset -= 16;
            tmpAviIndexValue = Audio_chunk_info->AviAudioIndexBuffer + Audio_chunk_info->AviAudioIndexBufferOffset;
        }

        AudioIndexCount--;
        
        if( Byte2Short(tmpAviIndexValue + 2) == AUDIO_INDEX)
        {
        	unsigned long distance;
			unsigned long Movi;
			//unsigned long CurAudioChunkBeginOffset;
			unsigned long CurAudioChunkUsedOffset;
			unsigned long PreAudioChunkBeginOffset;  

			if(Audio_chunk_info->Audio_Current_FrameNum == 0)
			{
				return FILE_END; 
			}			
            Audio_chunk_info->Audio_Current_FrameNum--;
			

			Movi = AviStreamInfo.StartDataPos;
			CurAudioChunkUsedOffset  = AVI_FTell(Audio_chunk_info->hAudioData);
			if(CurAudioChunkUsedOffset >= Movi)
			{
				CurAudioChunkUsedOffset  -= Movi;	
			}
			else
			{
				CurAudioChunkUsedOffset = 0;
			}

			PreAudioChunkBeginOffset= Byte2Long(tmpAviIndexValue + 8);

			Audio_chunk_info->Audio_FilePos = Byte2Long(tmpAviIndexValue + 8);
			Audio_chunk_info->Audio_ChunkLength = Byte2Long(tmpAviIndexValue + 12);			

			if(CurAudioChunkUsedOffset >= PreAudioChunkBeginOffset+8)
			{
				distance = CurAudioChunkUsedOffset - PreAudioChunkBeginOffset;
			}
			else
			{
				distance = 8;
			}

            Audio_chunk_info->Audio.CurOffset = distance - 8; 
            Audio_chunk_info->Audio.CurChunkSize = Audio_chunk_info->Audio_ChunkLength;
			return I_FRAME;
        }
        else if( Byte2Short(tmpAviIndexValue + 2) != VIDEO_INDEX)
        {
            return ERROR_FRAME;
        }
    }

    return FILE_END;  
}

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : AviGetVideoData()
*  Description:    get a chunk video data 
*  Input:          [pbuf]: a pointer to video data
*                  [size]: the length of video data buffer
*  Output:         [end_frame]: 
*  Return:         real get video data length
*  Others:         
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int AviGetVideoData(char *pbuf, int size, int *end_frame)
{
    UINT16 len,FileReadDataLen;
    INT16  ReadResult = I_FRAME;
	VIDEO_CHUNK_INFO *pVideo_chunk_info = &Video_chunk_info;
     
    len = size;//(size & 0xfffe);

    pVideo_chunk_info->Video.ReadSize = 0;
    *end_frame = 0;
    
    do {
        if(pVideo_chunk_info->Video.CurChunkSize == 0)  
        {
            ReadResult = GetNextVideoChunk(pVideo_chunk_info);
            AVI_FSeek(pVideo_chunk_info->Video.CurOffset,SEEK_CUR,pVideo_chunk_info->hVideoData);
        }
        if(ReadResult == NOP_FRAME)
        {
            return -1; // this frame no data
        }
        
		if (ReadResult != I_FRAME) 
		{
			break;
		}                              
        
        if (pVideo_chunk_info->Video.CurChunkSize > (len - pVideo_chunk_info->Video.ReadSize)) // chunk的长度大于所需要数据的长度
        {
            FileReadDataLen = AVI_FRead((UINT8*)pbuf + pVideo_chunk_info->Video.ReadSize, len - pVideo_chunk_info->Video.ReadSize, pVideo_chunk_info->hVideoData);
            if (FileReadDataLen < (len - pVideo_chunk_info->Video.ReadSize)) 
            {
		        break;
            }
        }
         else // chunk length less than need length
        {
            FileReadDataLen = AVI_FRead((UINT8*)pbuf + pVideo_chunk_info->Video.ReadSize, pVideo_chunk_info->Video.CurChunkSize, pVideo_chunk_info->hVideoData);
            if (FileReadDataLen < pVideo_chunk_info->Video.CurChunkSize)
            {
                break;
            }
        }
        
        pVideo_chunk_info->Video.CurChunkSize -= FileReadDataLen;
        pVideo_chunk_info->Video.ReadSize += FileReadDataLen;
		
		if(pVideo_chunk_info->Video.CurChunkSize == 0)
		{
		    *end_frame = 1;
			break; // chunk data is over
		}
            
    } while (len != pVideo_chunk_info->Video.ReadSize);     

    return(pVideo_chunk_info->Video.ReadSize);
}

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : AviGetAudioData()
*  Description:    get audio buf
*  Input:          [pbuf]: aduio data buffer
*                  [size]: audio data length
*  Output:      
*  Return:         real get audio length
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int AviGetAudioData(char *pbuf, int size)
{
    UINT16 len,FileReadDataLen;
    INT16  ReadResult = I_FRAME;
	AUDIO_CHUNK_INFO *pAudio_chunk_info = &Audio_chunk_info;
     
    len = size;//(size & 0xfffe);

    pAudio_chunk_info->Audio.ReadSize = 0;
    
    do {
    
        if(pAudio_chunk_info->Audio.CurChunkSize == 0)  
        {
            ReadResult = GetNextAudioChunk(pAudio_chunk_info);
            AVI_FSeek(pAudio_chunk_info->Audio.CurOffset,SEEK_CUR,pAudio_chunk_info->hAudioData);
        }
        
		if(ReadResult != I_FRAME) 
		{
		   break;
		}
                                        
        if (pAudio_chunk_info->Audio.CurChunkSize > (len - pAudio_chunk_info->Audio.ReadSize)) // chunk的长度大于所需要数据的长度
        {
            FileReadDataLen = AVI_FRead((UINT8*)pbuf + pAudio_chunk_info->Audio.ReadSize, len - pAudio_chunk_info->Audio.ReadSize, pAudio_chunk_info->hAudioData);
            if (FileReadDataLen < (len - pAudio_chunk_info->Audio.ReadSize)) 
            {
                break;
            }
        }
        else 
        {
            FileReadDataLen = AVI_FRead((UINT8*)pbuf + pAudio_chunk_info->Audio.ReadSize, pAudio_chunk_info->Audio.CurChunkSize, pAudio_chunk_info->hAudioData);
            if (FileReadDataLen < pAudio_chunk_info->Audio.CurChunkSize)
            {
                break;
            }
        }
        
        pAudio_chunk_info->Audio.CurChunkSize -= FileReadDataLen;
        pAudio_chunk_info->Audio.ReadSize += FileReadDataLen;
            
    } while (len != pAudio_chunk_info->Audio.ReadSize);   

    return(pAudio_chunk_info->Audio.ReadSize);  
}

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : AviVideoSeek()
*  Description:    
*  Input:          [nFrame]:   
*                  [direction]:    
*  Output:         
*  Return:         
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int AviVideoSeek(int nFrame, int direction)
{
	int i, ret = -1;
	VIDEO_CHUNK_INFO *pVideo_chunk_info = &Video_chunk_info;
	
	if(direction == 1)     // FFD
	{ 
		for(i = 0; i < nFrame; i++)
		{
			ret = GetNextVideoChunk(pVideo_chunk_info);
		}

		pVideo_chunk_info->Video.CurChunkSize = 0;
		AVI_FSeek(pVideo_chunk_info->Video.CurOffset, SEEK_CUR, pVideo_chunk_info->hVideoData);
	}
	else if(direction == -1) // FFW
	{
		for(i = 0; i < nFrame; i++)
		{
			ret = GetPreVideoChunk(pVideo_chunk_info);
		}

		pVideo_chunk_info->Video.CurChunkSize = 0;        
		AVI_FSeek(-(int)pVideo_chunk_info->Video.CurOffset, SEEK_CUR, pVideo_chunk_info->hVideoData);
	}

	if(ret == I_FRAME)
	{
		return 0;
	}
	else if(ret == FILE_END)
	{
		return 1;
	}
	return -1;
}

UINT32 Avi_GetTotalTime()//以s为单位
{
       return (((__int64)AviStreamInfo.MicroSecondPerFrame * TotalVideoChunkNum)/1000000);
}


UINT32 Avi_GetCurrentTime()
{
    return  (((__int64)AviStreamInfo.MicroSecondPerFrame * Video_chunk_info.Video_Current_FrameNum) / 1000000);
}


#if 0 
/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : AviAudioSeek()
*  Description:    
*  Input:          [nFrame]:    
*                  [direction]: 
*  Output:      
*  Return:         
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int AviAudioSeek(int nFrame, int direction)
{
	int i, ret = -1;
	AUDIO_CHUNK_INFO *pAudio_chunk_info = &Audio_chunk_info;
	
	if(direction == 1)     // FFD
	{ 
		for(i = 0; i < nFrame; i++)
		{
			ret = GetNextAudioChunk(pAudio_chunk_info);
		}

		pAudio_chunk_info->Audio.CurChunkSize = 0;
		AVI_FSeek(pAudio_chunk_info->Audio.CurOffset, SEEK_CUR, pAudio_chunk_info->hAudioData);
	}
	else if(direction == -1) // FFW
	{
		for(i = 0; i < nFrame; i++)
		{
			ret = GetPreAudioChunk(pAudio_chunk_info);
		}

		pAudio_chunk_info->Audio.CurChunkSize = 0;        
		AVI_FSeek(-(int)pAudio_chunk_info->Audio.CurOffset, SEEK_CUR, pAudio_chunk_info->hAudioData);
	}

	if(ret == I_FRAME)
	{
		return 0;
	}
	else if(ret == FILE_END)
	{
		return 1;
	}
	return -1;
}
#endif


#define AVI_AUDIO_FRAME_TIME    (26122) // a frame audio data time, unit is us

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : SyncAudio2Video()
*  Description:    
*  Input:          
*  Output:      
*  Return:         
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int SyncAudio2Video(void)
{
	AVI_STREAM_INFO  *pAviStreamInfo    = &AviStreamInfo;
	AUDIO_CHUNK_INFO *pAudio_chunk_info = &Audio_chunk_info;
	VIDEO_CHUNK_INFO *pVideo_chunk_info = &Video_chunk_info;	

	__int64 CurVideoTime;
	UINT32 DstAudioFrm;
	int i, ret = 0, nFrame;

	
	CurVideoTime = (__int64)pAviStreamInfo->MicroSecondPerFrame * pVideo_chunk_info->Video_Current_FrameNum;

	DstAudioFrm = (UINT32)(CurVideoTime/AVI_AUDIO_FRAME_TIME);
	if(DstAudioFrm > TotalAudioChunkNum)
	{
		DstAudioFrm = TotalAudioChunkNum;
	}

	if(DstAudioFrm > pAudio_chunk_info->Audio_Current_FrameNum)
	{
		// seek ffd
		nFrame = DstAudioFrm-pAudio_chunk_info->Audio_Current_FrameNum;
		for(i = 0; i < nFrame; i++)
		{
			ret = GetNextAudioChunk(pAudio_chunk_info);
		}
		
		pAudio_chunk_info->Audio.CurChunkSize = 0;        
		AVI_FSeek(pAudio_chunk_info->Audio.CurOffset, SEEK_CUR, pAudio_chunk_info->hAudioData);		
	}
	else if(DstAudioFrm < pAudio_chunk_info->Audio_Current_FrameNum)
	{
		// seek ffw
		nFrame = pAudio_chunk_info->Audio_Current_FrameNum-DstAudioFrm;
		for(i = 0; i < nFrame; i++)
		{
			ret = GetPreAudioChunk(pAudio_chunk_info);
		}		

		pAudio_chunk_info->Audio.CurChunkSize = 0;        
		AVI_FSeek(-(int)pAudio_chunk_info->Audio.CurOffset, SEEK_CUR, pAudio_chunk_info->hAudioData);			
	}
	else
	{   

		return 0;
	}

	if(ret == I_FRAME)
	{
		return 0;
	}
	else if(ret == FILE_END)
	{
		return 1;
	}
	return -1;
}

/******************************************************************************
*  Copyright (C),2004-2005, Fuzhou Rockchip Co.,Ltd.
*  Function name : SyncVideo2Audio()
*  Description:    
*  Input:          
*  Output:      
*  Return:         
*  Others: 
*  History:        
*           <author>      <time>     <version>       <desc>
******************************************************************************/
int SyncVideo2Audio(void)
{
	AVI_STREAM_INFO  *pAviStreamInfo    = &AviStreamInfo;
	AUDIO_CHUNK_INFO *pAudio_chunk_info = &Audio_chunk_info;
	VIDEO_CHUNK_INFO *pVideo_chunk_info = &Video_chunk_info;	

	__int64 CurAduioTime;
	UINT32 DstVideoFrm, CurVideoFrm;
	int i, ret = 0, nFrame;

	if(pAudio_chunk_info->Audio_Current_FrameNum >= TotalAudioChunkNum)
	{
		return 2; 
	}

	CurAduioTime = (__int64)AVI_AUDIO_FRAME_TIME * pAudio_chunk_info->Audio_Current_FrameNum;

	DstVideoFrm = (UINT32)(CurAduioTime/pAviStreamInfo->MicroSecondPerFrame);
	CurVideoFrm = pVideo_chunk_info->Video_Current_FrameNum;
	if(DstVideoFrm > TotalVideoChunkNum)
	{
		DstVideoFrm = TotalVideoChunkNum;
	}

	#define NEED_SYNC_DISTANCE    3

	if(DstVideoFrm >= CurVideoFrm+NEED_SYNC_DISTANCE)
	{
		nFrame = DstVideoFrm - CurVideoFrm;
		for(i = 0; i < nFrame; i++)
		{
			ret = GetNextVideoChunk(pVideo_chunk_info);
		}
		
		//pVideo_chunk_info->Video.CurChunkSize = 0;        
		AVI_FSeek(pVideo_chunk_info->Video.CurOffset, SEEK_CUR, pVideo_chunk_info->hVideoData);		
	}
	else if(CurVideoFrm >= DstVideoFrm+NEED_SYNC_DISTANCE)
	{
		nFrame = CurVideoFrm - DstVideoFrm;
		for(i = 0; i < nFrame; i++)
		{
			ret = GetPreVideoChunk(pVideo_chunk_info);
		}		

		pVideo_chunk_info->Video.CurChunkSize = 0;        
		AVI_FSeek(-(int)pVideo_chunk_info->Video.CurOffset, SEEK_CUR, pVideo_chunk_info->hVideoData);			
	}
	else
	{   
		return 2;
	}

	if(ret == I_FRAME)
	{
		return 0;
	}
	else if(ret == FILE_END)
	{
		return 1;
	}
	return -1;
}

#endif
#pragma arm section code

