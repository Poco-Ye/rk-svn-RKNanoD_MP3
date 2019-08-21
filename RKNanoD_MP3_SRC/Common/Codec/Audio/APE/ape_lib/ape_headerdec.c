
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_headerdec.h"
#include "ape_maclib.h"
#include "ape_Infodec.h"
#include "ape_globalvardeclaration.h"

// TODO: should push and pop the file position

void ApeHeaderInitialize(CAPEHeader *aI,CIO * pIO)
{
    aI->Ape_pIO = pIO;
}

#if 0
void ApeHeaderDelete(CAPEHeader *aI)
{
}
#endif

#if 0 //commented by hxd 20070710
ape_int32 ApeHeaderFindDescriptor(CAPEHeader *aI,ape_BOOL bSeek)
{
  // store the original location and seek to the beginning
  ape_int32 nOriginalFileLocation = aI->Ape_pIO->GetPosition(aI->Ape_pIO);
  // set the default junk bytes to 0
  ape_int32 nJunkBytes = 0;
  // skip an ID3v2 tag (which we really don't support anyway...)
  ape_uint32 nBytesRead = 0; 
  ape_uchar cID3v2Header[10];
  
  ape_uint32 nGoalID = ((ape_uint32)' ' << 24) | ((ape_uint32)'C' << 16) | ((ape_uint32)'A' << 8) | ((ape_uint32)'M');
  //ape_uchar  ntmpReadID[4];
  ape_uint32 nReadID = 0;
  ape_int32 nRetVal = aI->Ape_pIO->Read(aI->Ape_pIO,&nReadID, 4, &nBytesRead);
  ape_int32 nScanBytes = 0;
  
  
  aI->Ape_pIO->Seek(aI->Ape_pIO,0, FILE_BEGIN);
  
  
  
  aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) cID3v2Header, 10, &nBytesRead);
  if (cID3v2Header[0] == 'I' && cID3v2Header[1] == 'D' && cID3v2Header[2] == '3') 
  {
    // why is it so hard to figure the lenght of an ID3v2 tag ?!?
    //ape_uint32 nLength = *((ape_uint32 *) &cID3v2Header[6]);
    
    ape_uint32 nSyncSafeLength = 0;
    ape_BOOL bHasTagFooter = FALSE;
    
    nSyncSafeLength = (cID3v2Header[6] & 127) << 21;
    nSyncSafeLength += (cID3v2Header[7] & 127) << 14;
    nSyncSafeLength += (cID3v2Header[8] & 127) << 7;
    nSyncSafeLength += (cID3v2Header[9] & 127);
    
    
    
    if (cID3v2Header[5] & 16) 
    {
        bHasTagFooter = TRUE;
        nJunkBytes = nSyncSafeLength + 20;
    }
    else 
    {
        nJunkBytes = nSyncSafeLength + 10;
    }
    
    // error check
    if (cID3v2Header[5] & 64)
    {
        // this ID3v2 length calculator algorithm can't cope with extended headers
        // we should be ok though, because the scan for the MAC header below should
        // really do the trick
    }
    
    aI->Ape_pIO->Seek(aI->Ape_pIO,nJunkBytes, FILE_BEGIN);
    
    // scan for padding (slow and stupid, but who cares here...)
    if (!bHasTagFooter)
    {
        ape_char cTemp = 0;
        aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) &cTemp, 1, &nBytesRead);
        while (cTemp == 0 && nBytesRead == 1)
        {
            nJunkBytes++;
            aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) &cTemp, 1, &nBytesRead);
        }
    }
  }
  aI->Ape_pIO->Seek(aI->Ape_pIO,nJunkBytes, FILE_BEGIN);
  
  // scan until we hit the APE_DESCRIPTOR, the end of the file, or 1 MB later
  nGoalID = (' ' << 24) | ('C' << 16) | ('A' << 8) | ('M');
  nReadID = 0;
  nRetVal = aI->Ape_pIO->Read(aI->Ape_pIO,&nReadID, 4, &nBytesRead);
  if (nRetVal != 0 || nBytesRead != 4)
  {
    return ERROR_UNDEFINED;
  }
  
  nBytesRead = 1;
  
  //while ((nGoalID != nReadID) && (nBytesRead == 1) && (nScanBytes < (1024 * 1024)))
  while ((nGoalID != nReadID) && (nBytesRead == 1) && (nScanBytes < ( 10240)))
  {
    ape_uchar cTemp;
    aI->Ape_pIO->Read(aI->Ape_pIO,&cTemp, 1, &nBytesRead);
    nReadID = (((ape_uint32) cTemp) << 24) | (nReadID >> 8);
    nJunkBytes++;
    nScanBytes++;
  }
  
  if (nGoalID != nReadID)
  {
    nJunkBytes = -1;
  }
  
  // seek to the proper place (depending on result and settings)
  if (bSeek && (nJunkBytes != -1))
  {
      // successfully found the start of the file (seek to it and return)
      aI->Ape_pIO->Seek(aI->Ape_pIO,nJunkBytes, FILE_BEGIN);
  }
  else
  {
      // restore the original file pointer
      aI->Ape_pIO->Seek(aI->Ape_pIO,nOriginalFileLocation, FILE_BEGIN);
  }
  
  return nJunkBytes;
}
#endif

#if 0 //commented by hxd 20070710
//identity true ape file
ape_int32 ApeHeaderAnalyze(CAPEHeader *aI,struct APE_FILE_INFO * pInfo)
{
  // variables
  ape_uint32 nBytesRead = 0;
  struct APE_COMMON_HEADER CommonHeader; 
  
  ape_int32 nRetVal = ERROR_UNDEFINED;
  // error check
  if ((aI->Ape_pIO == NULL) || (pInfo == NULL))
  {
      //return ERROR_INVALID_PARAMETER;
    return (-1);
  }
  
  // find the descriptor 'MAC'
  pInfo->nJunkHeaderBytes = aI->FindDescriptor(aI,TRUE);
  if (pInfo->nJunkHeaderBytes < 0)
  {
    return ERROR_UNDEFINED;
  }
  
  // read the first 8 bytes of the descriptor (ID and version)
  memset(&CommonHeader, 0, sizeof(struct APE_COMMON_HEADER));
  aI->Ape_pIO->Read(aI->Ape_pIO,&CommonHeader, sizeof(struct APE_COMMON_HEADER), &nBytesRead);
  
  // make sure we're at the ID
  if (CommonHeader.cID[0] != 'M' || CommonHeader.cID[1] != 'A' 
  	  || CommonHeader.cID[2] != 'C' || CommonHeader.cID[3] != ' ')
  {
    return ERROR_UNDEFINED;
  }
  
  
  
  if (CommonHeader.nVersion >= 3980)
  {
      // current header format
      nRetVal = aI->AnalyzeCurrent(aI,pInfo);
  }
  else
  {
      // legacy support
      nRetVal = aI->AnalyzeOld(aI,pInfo);
  }
  
  return nRetVal;
}
#endif

#if 0 //commented by hxd 20070710
//Decode ape header information of 3.99v and later
ape_int32 ApeHeaderAnalyzeCurrent(CAPEHeader *aI,struct APE_FILE_INFO * pInfo)
{
    // variable declares
    ape_uint32 nBytesRead = 0;
    struct APE_HEADER APEHeader;
    
    //pInfo->spAPEDescriptor=(struct APE_DESCRIPTOR*)malloc(sizeof(struct APE_DESCRIPTOR)); 
    //memset(pInfo->spAPEDescriptor, 0, sizeof(struct APE_DESCRIPTOR));
    //memset(&APEHeader, 0, sizeof(APEHeader));
    pInfo->spAPEDescriptor=&Ape_gHeaderDescriptor;
    
    // read the descriptor
    aI->Ape_pIO->Seek(aI->Ape_pIO,pInfo->nJunkHeaderBytes, FILE_BEGIN);
    aI->Ape_pIO->Read(aI->Ape_pIO,pInfo->spAPEDescriptor, sizeof(struct APE_DESCRIPTOR), &nBytesRead);
    
    if ((pInfo->spAPEDescriptor->nDescriptorBytes - nBytesRead) > 0)
    {
        aI->Ape_pIO->Seek(aI->Ape_pIO,pInfo->spAPEDescriptor->nDescriptorBytes - nBytesRead, FILE_CURRENT);
    }
    
    // read the header
    aI->Ape_pIO->Read(aI->Ape_pIO,&APEHeader, sizeof(APEHeader), &nBytesRead);
    
    if ((pInfo->spAPEDescriptor->nHeaderBytes - nBytesRead) > 0)
    {	
        aI->Ape_pIO->Seek(aI->Ape_pIO,pInfo->spAPEDescriptor->nHeaderBytes - nBytesRead, FILE_CURRENT);
    }
    
    // fill the APE info structure
    pInfo->nVersion                = (ape_int32)(pInfo->spAPEDescriptor->nVersion);
    if(pInfo->nVersion < 3930)
    {
    	return ERROR_UNDEFINED;
    }    
    pInfo->nCompressionLevel    = (ape_int32)(APEHeader.nCompressionLevel);
    if((pInfo->nCompressionLevel != COMPRESSION_LEVEL_FAST) && (pInfo->nCompressionLevel != COMPRESSION_LEVEL_NORMAL))
    {
    	return ERROR_UNDEFINED;
    }
    
    pInfo->nFormatFlags            = (ape_int32)(APEHeader.nFormatFlags);
    pInfo->nTotalFrames            = (ape_int32)(APEHeader.nTotalFrames);
    pInfo->nFinalFrameBlocks    = (ape_int32)(APEHeader.nFinalFrameBlocks);
    pInfo->nBlocksPerFrame        = (ape_int32)(APEHeader.nBlocksPerFrame);
    pInfo->nChannels            = (ape_int32)(APEHeader.nChannels);
    pInfo->nSampleRate            = (ape_int32)(APEHeader.nSampleRate);
    pInfo->nBitsPerSample        = (ape_int32)(APEHeader.nBitsPerSample);
    pInfo->nBytesPerSample        = pInfo->nBitsPerSample / 8;
    pInfo->nBlockAlign            = pInfo->nBytesPerSample * pInfo->nChannels;
    pInfo->nTotalBlocks            = (APEHeader.nTotalFrames == 0) ? 0 : ((APEHeader.nTotalFrames -  1) * pInfo->nBlocksPerFrame) + APEHeader.nFinalFrameBlocks;
    pInfo->nWAVHeaderBytes        = (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER) ? sizeof(struct WAVE_HEADER) : pInfo->spAPEDescriptor->nHeaderDataBytes;
    pInfo->nWAVTerminatingBytes    = pInfo->spAPEDescriptor->nTerminatingDataBytes;
    pInfo->nWAVDataBytes        = pInfo->nTotalBlocks * pInfo->nBlockAlign;
    pInfo->nWAVTotalBytes        = pInfo->nWAVDataBytes + pInfo->nWAVHeaderBytes + pInfo->nWAVTerminatingBytes;
    
    pInfo->nAPETotalBytes        = aI->Ape_pIO->GetSize(aI->Ape_pIO);
    pInfo->nLengthMS            = (ape_int32)(((double)(pInfo->nTotalBlocks) * (double)(1000)) / (double)(pInfo->nSampleRate));
    pInfo->nAverageBitrate        = (pInfo->nLengthMS <= 0) ? 0 : (long)(((double)(pInfo->nAPETotalBytes) * (double)(8)) / (double)(pInfo->nLengthMS));
    pInfo->nDecompressedBitrate = (pInfo->nBlockAlign * pInfo->nSampleRate * 8) / 1000;
    pInfo->nSeekTableElements    = pInfo->spAPEDescriptor->nSeekTableBytes / 4;
    
    // get the seek tables (really no reason to get the whole thing if there's extra)
    //pInfo->spSeekByteTable=(ape_uint32*)malloc(sizeof(ape_uint32)*pInfo->nSeekTableElements);
    //if (pInfo->spSeekByteTable == NULL) { return ERROR_UNDEFINED; }
    
    //pInfo->nSeekTableElements表示seektalbe数目 repaired by hxd 20071205
    if (pInfo->nSeekTableElements-1>SEEKBYTE_TABLE_MAX)//if (pInfo->nTotalFrames-1>SEEKBYTE_TABLE_MAX)
    {
    	return ERROR_UNDEFINED;
    }
    else
    {
          pInfo->spSeekByteTable=(Ape_gHeaderSeekByteTable);
    }
    
    /*Mod by Wei.Hisung 2007.03.06*/
    aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) pInfo->spSeekByteTable, 4 * pInfo->nSeekTableElements, &nBytesRead);
    
    
    // get the wave header
    if (!(APEHeader.nFormatFlags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
    {
      /*Mod by Wei.Hisung 2007.03.06*/
      //pInfo->spWaveHeaderData=(ape_uchar*)malloc(sizeof(ape_uchar)*pInfo->nWAVHeaderBytes);
      //if (pInfo->spWaveHeaderData == NULL) { return ERROR_UNDEFINED; }
      //aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) pInfo->spWaveHeaderData, pInfo->nWAVHeaderBytes, &nBytesRead);
      aI->Ape_pIO->Seek(aI->Ape_pIO,pInfo->nWAVHeaderBytes, FILE_CURRENT);
    }
    
    return ERROR_SUCCESS;
}
#endif

#if 0 //commented by hxd 20070710
//Decode Ape header information of 3.98v and before
ape_int32 ApeHeaderAnalyzeOld(CAPEHeader *aI,struct APE_FILE_INFO * pInfo)
{
    // variable declares
    ape_uint16 nBytesRead = 0;

    // read the MAC header from the file
    struct APE_HEADER_OLD APEHeader;
	  ape_int32 nPeakLevel = -1;

    aI->Ape_pIO->Seek(aI->Ape_pIO,pInfo->nJunkHeaderBytes, FILE_BEGIN);
    aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) &APEHeader, sizeof(APEHeader), (ape_uint32*)&nBytesRead);

    // fail on 0 length APE files (catches non-finalized APE files)
    if (APEHeader.nTotalFrames == 0)
    {
        return ERROR_UNDEFINED;
    }


    if (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL)
    {
        aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) &nPeakLevel, 4, (ape_uint32*)&nBytesRead);
    }

    if (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS)
    {
        aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) &pInfo->nSeekTableElements, 4, (ape_uint32*)&nBytesRead);
    }
    else
    {
        pInfo->nSeekTableElements = APEHeader.nTotalFrames;
    }
    // fill the APE info structure
    pInfo->nVersion                = (ape_int32)(APEHeader.nVersion);
    if(pInfo->nVersion < 3930)
    {
    	return ERROR_UNDEFINED;
    }
    pInfo->nCompressionLevel    = (ape_int32)(APEHeader.nCompressionLevel);
    if((pInfo->nCompressionLevel != COMPRESSION_LEVEL_FAST) && (pInfo->nCompressionLevel != COMPRESSION_LEVEL_NORMAL))
    {
    	return ERROR_UNDEFINED;
    }
        
    pInfo->nFormatFlags            = (ape_int32)(APEHeader.nFormatFlags);
    pInfo->nTotalFrames            = (ape_int32)(APEHeader.nTotalFrames);
    pInfo->nFinalFrameBlocks    = (ape_int32)(APEHeader.nFinalFrameBlocks);
    pInfo->nBlocksPerFrame        = ((APEHeader.nVersion >= 3900) || ((APEHeader.nVersion >= 3800) && (APEHeader.nCompressionLevel == COMPRESSION_LEVEL_EXTRA_HIGH))) ? 73728 : 9216;
    if ((APEHeader.nVersion >= 3950))
    {
    	pInfo->nBlocksPerFrame = 73728 * 4;
    }
    pInfo->nChannels            = (ape_int32)(APEHeader.nChannels);
    pInfo->nSampleRate            = (ape_int32)(APEHeader.nSampleRate);
    pInfo->nBitsPerSample        = (pInfo->nFormatFlags & MAC_FORMAT_FLAG_8_BIT) ? 8 : ((pInfo->nFormatFlags & MAC_FORMAT_FLAG_24_BIT) ? 24 : 16);
    pInfo->nBytesPerSample        = pInfo->nBitsPerSample / 8;
    pInfo->nBlockAlign            = pInfo->nBytesPerSample * pInfo->nChannels;
    pInfo->nTotalBlocks            = (APEHeader.nTotalFrames == 0) ? 0 : ((APEHeader.nTotalFrames -  1) * pInfo->nBlocksPerFrame) + APEHeader.nFinalFrameBlocks;
    pInfo->nWAVHeaderBytes        = (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER) ? sizeof(struct WAVE_HEADER) : APEHeader.nHeaderBytes;
    pInfo->nWAVTerminatingBytes    = (ape_int32)(APEHeader.nTerminatingBytes);
    pInfo->nWAVDataBytes        = pInfo->nTotalBlocks * pInfo->nBlockAlign;
    pInfo->nWAVTotalBytes        = pInfo->nWAVDataBytes + pInfo->nWAVHeaderBytes + pInfo->nWAVTerminatingBytes;
    pInfo->nAPETotalBytes        = aI->Ape_pIO->GetSize(aI->Ape_pIO);
    pInfo->nLengthMS            = (ape_int32)(((double)(pInfo->nTotalBlocks) * (double)(1000)) / (double)(pInfo->nSampleRate));
    pInfo->nAverageBitrate        = (pInfo->nLengthMS <= 0) ? 0 : (ape_int32)(((double)(pInfo->nAPETotalBytes) * (double)(8)) / (double)(pInfo->nLengthMS));
    pInfo->nDecompressedBitrate = (pInfo->nBlockAlign * pInfo->nSampleRate * 8) / 1000;

    // get the wave header
    if (!(APEHeader.nFormatFlags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
    {
		    //pInfo->spWaveHeaderData=(ape_uchar*)malloc(sizeof(ape_uchar)*APEHeader.nHeaderBytes);
        //if (pInfo->spWaveHeaderData == NULL) { return ERROR_UNDEFINED; }
        //aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) pInfo->spWaveHeaderData, APEHeader.nHeaderBytes, &nBytesRead);
        //aI->Ape_pIO->Read(aI->Ape_pIO,Ape_gReadBuffer, APEHeader.nHeaderBytes, &nBytesRead);
        aI->Ape_pIO->Seek(aI->Ape_pIO, APEHeader.nHeaderBytes, FILE_CURRENT);
        //Byte2Word2(pInfo->spWaveHeaderData,Ape_gReadBuffer,nBytesRead/2);
    }

    // get the seek tables (really no reason to get the whole thing if there's extra)
    /*Mod by Wei.Hisung 2007.03.06*/
    //pInfo->spSeekByteTable=(ape_uint32*)malloc(sizeof(ape_uint32)*pInfo->nSeekTableElements);
    //if (pInfo->spSeekByteTable == NULL) { return ERROR_UNDEFINED; }
    //pInfo->nSeekTableElements表示seektalbe数目 repaired by hxd 20071205
    if (pInfo->nSeekTableElements-1>SEEKBYTE_TABLE_MAX)//if (pInfo->nTotalFrames-1>SEEKBYTE_TABLE_MAX)
    {
    	return ERROR_UNDEFINED;
    }
    else
    {
          pInfo->spSeekByteTable=(Ape_gHeaderSeekByteTable);
    }
    
    /*Mod by Wei.Hisung 2007.03.06*/
    //Ape_pIO->Read((ape_uchar *) pInfo->spSeekByteTable.GetPtr(), 4 * pInfo->nSeekTableElements, &nBytesRead);
    aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) pInfo->spSeekByteTable, 4 * pInfo->nSeekTableElements, (ape_uint32*)&nBytesRead);
	
    //commented by hxd 20070615
    //if (pInfo->nSeekTableElements >pInfo->nTotalFrames)
    //  {
    //	  aI->Ape_pIO->Seek(aI->Ape_pIO,4 * (pInfo->nSeekTableElements - pInfo->nTotalFrames), FILE_CURRENT);
    //  }
    
    if (APEHeader.nVersion <= 3800) 
    {
      //pInfo->spSeekBitTable=(ape_uchar*)malloc(sizeof(ape_uchar)*(pInfo->nSeekTableElements));
      //if (pInfo->spSeekBitTable == NULL) { return ERROR_UNDEFINED; }
      
      //aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) pInfo->spSeekBitTable, pInfo->nSeekTableElements, &nBytesRead);
      if (pInfo->nSeekTableElements-1>SEEKBYTE_TABLE_MAX)
      {
          return ERROR_UNDEFINED;
      }
      else
      {
           pInfo->spSeekBitTable=(Ape_gHeaderSeekBitTable);
      }
      aI->Ape_pIO->Read(aI->Ape_pIO,(ape_uchar *) pInfo->spSeekBitTable, pInfo->nTotalFrames, (ape_uint32*)&nBytesRead);
      //aI->Ape_pIO->Read(aI->Ape_pIO,Ape_gReadBuffer, pInfo->nTotalFrames, &nBytesRead);
      //Byte2Word2(pInfo->spSeekBitTable,Ape_gReadBuffer,nBytesRead/2);
      if (pInfo->nSeekTableElements >pInfo->nTotalFrames)
      {
         aI->Ape_pIO->Seek(aI->Ape_pIO,(pInfo->nSeekTableElements - pInfo->nTotalFrames), FILE_CURRENT);
      }
    }
    
    return ERROR_SUCCESS;
}
#endif

#pragma arm section code 

#endif
#endif
