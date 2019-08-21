
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_maclib.h"
/*Mod by Wei.Hisung 2007.03.05*/
//#include "APECompress.h"
//#include "APECompressCreate.h"
//#include "APECompressCore.h"
//#include "APECompress.h"
#include "ape_decompress.h"
#include "ape_Infodec.h"
#include "ape_globalvardeclaration.h"
/*Mod by Wei.Hisung 2007.03.05*/
//#include "APELink.h"

#ifdef BACKWARDS_COMPATIBILITY
    #include "ape_decompold.h"
#endif
#include "ape_globalvardeclaration.h"

void * ApeDecompressCoreCreate(CAPEInfo * pAPEInfo, ape_int32 nStartBlock, ape_int32 nFinishBlock, ape_int32 * pErrorCode)
{
  IAPEDecompress * pAPEDecompress = NULL;
  if (pAPEInfo != NULL && *pErrorCode == ERROR_SUCCESS)
  {
    if (pAPEInfo->GetInfo(pAPEInfo,APE_INFO_FILE_VERSION,0,0) >= 3930)
  	{
  		pAPEDecompress =(IAPEDecompress *) &Ape_gIApeDecompressMalloc;//malloc(sizeof(CAPEDecompress));
      pAPEDecompress->cIAPEDecompress =(void (*)(ape_int32 *,void *,ape_int32,ape_int32))ApeDecompressInitialize;
      pAPEDecompress->dIAPEDecompress =ApeDecompressDelete;
      pAPEDecompress->GetData =(ape_int32 (*)(void *,ape_int32,ape_int32 *))ApeDecompressGetData;
  		pAPEDecompress->GetInfo =ApeDecompressGetInfo;
  		pAPEDecompress->Seek =(ape_int32 (*)(void *,ape_int32))ApeDecompressSeek;
  		pAPEDecompress->cIAPEDecompress(pErrorCode, pAPEInfo, nStartBlock, nFinishBlock);//initalize ape decompress	param			
  	}
#ifdef BACKWARDS_COMPATIBILITY
  else
  {
  	pAPEDecompress =(IAPEDecompress *) &Ape_gIApeDecompressMalloc;//malloc(sizeof(CAPEDecompressOld));
  
  }
#endif

    if (pAPEDecompress == NULL || *pErrorCode != ERROR_SUCCESS)
    {
        //SAFE_DELETE(pAPEDecompress)
    }
      
  }
  
  return pAPEDecompress;
}

#if 0 //commented by hxd 20070710
void *  ApeDecompressCreate(ape_int32 * pErrorCode)
{
  // variables
  ape_int32 nErrorCode = ERROR_UNDEFINED;
  CAPEInfo * pAPEInfo = NULL;
  ape_int32 nStartBlock = -1; ape_int32 nFinishBlock = -1;
  IAPEDecompress * pAPEDecompress ;
  
  // error check the parameters
  
  {
   
  //pAPEInfo = (CAPEInfo*)malloc(sizeof(CAPEInfo));//(&nErrorCode, pFilename);
  pAPEInfo = &Ape_gDecompressInfoCreate;
  pAPEInfo->cCAPEInfoFile = (void (*)(void *,ape_int32 *,CAPETag *))ApeInfoFile;
  pAPEInfo->cCAPEInfoIO = (void (*)(void *,ape_int32 *,CIO *,CAPETag *))ApeInfoIo;
  pAPEInfo->CloseFile = (ape_int32 (*)(void *))ApeInfoCloseFile;
  //pAPEInfo->dCAPEInfo = (void (*)(void *))ApeInfoDelete;
  pAPEInfo->GetFileInformation = (ape_int32 (*)(void *,ape_int32))ApeInfoGetFileInfo;
  pAPEInfo->GetInfo = (ape_int32 (*)(void *,enum APE_DECOMPRESS_FIELDS,ape_int32,ape_int32))ApeInfoGetInfo;
  pAPEInfo->cCAPEInfoFile(pAPEInfo,&nErrorCode, NULL);
  }
  
  // fail if we couldn't get the file information
  if (pAPEInfo == NULL)
  {
      if (pErrorCode) *pErrorCode = ERROR_INVALID_INPUT_FILE;
      return NULL;
  }
  
  
  // create and return
  pAPEDecompress = (IAPEDecompress *)ApeDecompressCoreCreate(pAPEInfo, nStartBlock, nFinishBlock, &nErrorCode);
  if (pErrorCode) *pErrorCode = nErrorCode;
  return pAPEDecompress;
}
#endif

#if 0
void *  CreateIAPEDecompressEx(CIO * pIO, ape_int32 * pErrorCode)
{
    ape_int32 nErrorCode = ERROR_UNDEFINED;
	IAPEDecompress * pAPEDecompress ;
	CAPEInfo * pAPEInfo = (CAPEInfo*)malloc(sizeof(CAPEInfo));//(&nErrorCode, pFilename);
	pAPEInfo->cCAPEInfoFile = (void (*)(CAPEInfo *,ape_int32 *,CAPETag *))ApeInfoFile;
    pAPEInfo->cCAPEInfoIO = (void (*)(CAPEInfo *,ape_int32 *,CIO *,CAPETag *))ApeInfoIo;
    pAPEInfo->CloseFile = (ape_int32 (*)(CAPEInfo *))ApeInfoCloseFile;
	//pAPEInfo->dCAPEInfo = (void (*)(void *))ApeInfoDelete;
	pAPEInfo->GetFileInformation = (ape_int32 (*)(CAPEInfo *,ape_int32))ApeInfoGetFileInfo;
	pAPEInfo->GetInfo = (ape_int32 (*)(void *,enum APE_DECOMPRESS_FIELDS,ape_int32,ape_int32))ApeInfoGetInfo;
	pAPEInfo->cCAPEInfoIO(pAPEInfo,&nErrorCode, pIO,NULL);

    pAPEDecompress = (IAPEDecompress *)ApeDecompressCoreCreate(pAPEInfo, -1, -1, &nErrorCode);
    if (pErrorCode) *pErrorCode = nErrorCode;
    return pAPEDecompress;
}
#endif

#if 0
IAPEDecompress *  CreateIAPEDecompressEx2(CAPEInfo * pAPEInfo, ape_int32 nStartBlock, ape_int32 nFinishBlock, ape_int32 * pErrorCode)
{
    ape_int32 nErrorCode = ERROR_SUCCESS;
    IAPEDecompress * pAPEDecompress = (IAPEDecompress *)ApeDecompressCoreCreate(pAPEInfo, nStartBlock, nFinishBlock, &nErrorCode);
    if (pErrorCode) *pErrorCode = nErrorCode;
    return pAPEDecompress;
}
#endif

ape_int32  ApeFillWaveFormatEx(WAVEFORMATEX * pWaveFormatEx, ape_int32 nSampleRate, ape_int32 nBitsPerSample, ape_int32 nChannels)
{
    pWaveFormatEx->cbSize = 0;
    pWaveFormatEx->nSamplesPerSec = nSampleRate;
    pWaveFormatEx->wBitsPerSample = nBitsPerSample;
    pWaveFormatEx->nChannels = nChannels;
    pWaveFormatEx->wFormatTag = 1;

    pWaveFormatEx->nBlockAlign = (pWaveFormatEx->wBitsPerSample / 8) * pWaveFormatEx->nChannels;
    pWaveFormatEx->nAvgBytesPerSec = pWaveFormatEx->nBlockAlign * pWaveFormatEx->nSamplesPerSec;

    return ERROR_SUCCESS;
}

ape_int32 ApeFillWaveHeader(struct WAVE_HEADER * pWAVHeader, ape_int32 nAudioBytes, WAVEFORMATEX * pWaveFormatEx, ape_int32 nTerminatingBytes)
{
  // RIFF header
  memcpy(pWAVHeader->cRIFFHeader, "RIFF", LENGTH(4));
  pWAVHeader->nRIFFBytes = (nAudioBytes + 44) - 8 + nTerminatingBytes;
  
  // format header
  memcpy(pWAVHeader->cDataTypeID, "WAVE", LENGTH(4));
  memcpy(pWAVHeader->cFormatHeader, "fmt ", LENGTH(4));
  
  // the format chunk is the first 16 bytes of a waveformatex
  pWAVHeader->nFormatBytes = 16;
  memcpy(&pWAVHeader->nFormatTag, pWaveFormatEx, LENGTH(16));
  
  // the data header
  memcpy(pWAVHeader->cDataHeader, "data", LENGTH(4));
  pWAVHeader->nDataBytes = nAudioBytes;
  
  return ERROR_SUCCESS;
    
}

#pragma arm section code

#endif
#endif
