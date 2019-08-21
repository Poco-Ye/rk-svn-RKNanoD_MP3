
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE


#include "ape_all.h"
#include "ape_Infodec.h"
//#include "ape_globalvardeclaration.h"
#include "ape_globalvardefine.h"
#include "ape_headerdec.h"
#include "ape_decompress.h"
#include "ape_io1.h"
#include "pAPE_DEC.h"

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"


	static char apeversion[] = "Version:0.0.1 \nDate:2012.3.29 \nLib:ape_dec_lib" ;

	 char * ApeDecVersion()
	{
		
			return apeversion;
	}




extern void ApeDecodeInitialize(void);

#ifdef APE_USE_TABLE_ROOM

#define    TBL_OFFSET_Ape_gtPowersOfTwoMinusOne                           0xba98    //  132
#define    TBL_OFFSET_Ape_gtKSumMinBoundary                               0xbb1c    //  128
#define    TBL_OFFSET_Ape_gtRangeTotalOne                                 0xbb9c    //  260
#define    TBL_OFFSET_Ape_gtRangeWidthOne                                 0xbca0    //  256
#define    TBL_OFFSET_Ape_gtRangeTotalTwo                                 0xbda0    //  260
#define    TBL_OFFSET_Ape_gtRangeWidthTwo                                 0xbea4    //  256
#define    TBL_OFFSET_Ape_gtCRC32                                         0xbfa4    // 1024

extern unsigned long wma_table_room[];

// 初始化ape table 指针
void APETableRoomInit(void)
{   
	unsigned char *pTableBase = (unsigned char *)0x1000c400;
	//unsigned char *pTableBase = (unsigned char *)wma_table_room;
	/*
	Ape_gtPowersOfTwoMinusOne                    = (ape_uint32 *)(pTableBase + TBL_OFFSET_Ape_gtPowersOfTwoMinusOne);
	Ape_gtKSumMinBoundary                        = (ape_uint32 *)(pTableBase + TBL_OFFSET_Ape_gtKSumMinBoundary    );
	Ape_gtRangeTotalOne                          = (ape_uint32 *)(pTableBase + TBL_OFFSET_Ape_gtRangeTotalOne      );
	Ape_gtRangeWidthOne                          = (ape_uint32 *)(pTableBase + TBL_OFFSET_Ape_gtRangeWidthOne      );
	Ape_gtRangeTotalTwo                          = (ape_uint32 *)(pTableBase + TBL_OFFSET_Ape_gtRangeTotalTwo      );
	Ape_gtRangeWidthTwo                          = (ape_uint32 *)(pTableBase + TBL_OFFSET_Ape_gtRangeWidthTwo      );
	Ape_gtCRC32                                  = (ape_uint32 *)(pTableBase + TBL_OFFSET_Ape_gtCRC32              );	
		*/
	
	Ape_gtPowersOfTwoMinusOne					 = Ape_gtPowersOfTwoMinusOne_s;
	Ape_gtKSumMinBoundary						 = Ape_gtKSumMinBoundary_s;
	Ape_gtRangeTotalOne 						 =Ape_gtRangeTotalOne_s;
	Ape_gtRangeWidthOne 						 = Ape_gtRangeWidthOne_s;
	Ape_gtRangeTotalTwo 						 = Ape_gtRangeTotalTwo_s;
	Ape_gtRangeWidthTwo 						 = Ape_gtRangeWidthTwo_s;
	Ape_gtCRC32 								 = Ape_gtCRC32_s;


}  

#endif

//////////////////////////////
//#include "../include/globals.h"
//#include "../include/CommonCmd.h"
//#include "../buffer/buffer.h"
//////////////////////////////////
//#include "../include/file_access.h"

//mod by vincent
//tAPEFileIo    Ape_gFileOper;//定义文件操作结构体变量,成员为指向回调函数的函数指针


/*	MOD BY VINCENT
///////////文件读写回调函数///////////////
int Ape_callbackfread(void* ptr,int size,int nitems,FILE* stream)
{
  int retval;
  retval = fread(ptr,size,nitems,stream);
  return retval;
}

int Ape_callbackfwrite(void* ptr, int size,int nitems,FILE* stream)
{
  int retval;
  retval = fwrite(ptr,size,nitems,stream);
  return retval;
}

int Ape_callbackfseek(FILE* stream,long offset,int fromwhere)
{
  int retval;
  retval = fseek(stream,offset,fromwhere);
  return retval;
}

int Ape_callbackftell(FILE* stream)
{
  int retval;
  
  retval = ftell(stream);
  
  return retval;
}
*/
////////////////////////////////////////////////////////////
void ApeDecompressCreate(ape_int32* pErrorCode)
{
  ape_int32 nErrorCode = ERROR_UNDEFINED;
  CAPEInfo* pAPEInfo = NULL;
  
  // error check the parameters
  {
    pAPEInfo = &Ape_gDecompressInfoCreate;
    pAPEInfo->cCAPEInfoFile = (void (*)(void *,ape_int32 *,CAPETag *))ApeInfoFile;
    pAPEInfo->cCAPEInfoIO = (void (*)(void *,ape_int32 *,CIO *,CAPETag *))ApeInfoIo;
    pAPEInfo->CloseFile = (ape_int32 (*)(void *))ApeInfoCloseFile;
    pAPEInfo->GetFileInformation = (ape_int32 (*)(void *,ape_int32))ApeInfoGetFileInfo;
    pAPEInfo->GetInfo = (ape_int32 (*)(void *,enum APE_DECOMPRESS_FIELDS,ape_int32,ape_int32))ApeInfoGetInfo;
    pAPEInfo->cCAPEInfoFile(pAPEInfo,&nErrorCode,NULL);
  }

  // fail if we couldn't get the file information
  if (pAPEInfo == NULL)
  {
      if (pErrorCode)
      {
        *pErrorCode = ERROR_INVALID_INPUT_FILE;
      }
      //return NULL;
  }
  if (pErrorCode) *pErrorCode = nErrorCode;
   
}
/********************************
*该函数作用是判断是否是APE文件
********************************/
int CheckAPE(void)
{
  ape_int32 nFunctionRetVal = ERROR_SUCCESS;
  tAPE *pAPE;

  // The first parameter is a pointer to the APE persistent state.
  pAPE = (tAPE *)&gAPEStruct;//ulEndOfRAM;
/*	mod by vincent */
  Ape_pInBuffer = (ape_uint32*)&pAPE->pcEncodedData[0];//输入Buffer指针初始化
/*
  Ape_gFileOper.ApeReadBase = Ape_callbackfread;
  Ape_gFileOper.ApeWriteBase = Ape_callbackfwrite;
  Ape_gFileOper.ApeTellBase = Ape_callbackftell;
  Ape_gFileOper.ApeSeekBase = Ape_callbackfseek;
*/    
  //全局变量初始化
  ApeDecodeInitialize();

#ifdef APE_USE_TABLE_ROOM
	APETableRoomInit();
#else
  memcpy(&Ape_gtPowersOfTwoMinusOne[0],&Ape_gtPowersOfTwoMinusOne_s[0],33*sizeof(ape_uint32));
	memcpy(&Ape_gtKSumMinBoundary[0],&Ape_gtKSumMinBoundary_s[0],32*sizeof(ape_uint32));
	memcpy(&Ape_gtRangeTotalOne[0],&Ape_gtRangeTotalOne_s[0],65*sizeof(ape_uint32));
	memcpy(&Ape_gtRangeWidthOne[0],&Ape_gtRangeWidthOne_s[0],64*sizeof(ape_uint32));
	memcpy(&Ape_gtRangeTotalTwo[0],&Ape_gtRangeTotalTwo_s[0],65*sizeof(ape_uint32));
	memcpy(&Ape_gtRangeWidthTwo[0],&Ape_gtRangeWidthTwo_s[0],64*sizeof(ape_uint32));
	/*
	ape_uint32 Ape_gtPowersOfTwoMinusOne[33];
	ape_uint32 Ape_gtKSumMinBoundary[32];
	ape_uint32 Ape_gtRangeTotalOne[65];
	ape_uint32 Ape_gtRangeWidthOne[64];
	ape_uint32 Ape_gtRangeTotalTwo[65];
	ape_uint32 Ape_gtRangeWidthTwo[64]; 
	*/
#endif
	
  //创建APE解码
  ApeDecompressCreate(&nFunctionRetVal);
  
  return nFunctionRetVal;
}

//APE文件信息获取
void ApeInfoFile(CAPEInfo* aI,ape_int32 * pErrorCode, CAPETag * pTag) 
{
  *pErrorCode = ERROR_SUCCESS;
  //aI->CloseFile(aI);
  // re-initialize variables
  aI->m_APEFileInfo.nSeekTableElements = 0;
  aI->m_bHasFileInformationLoaded = FALSE;
    
  
  aI->m_spIO= &Ape_gInfoIO;
  
  //结构体m_spIO成员初始化
  ApeIoInitialize(aI->m_spIO);
    
  //打开APE文件    
  if (aI->m_spIO->Open(aI->m_spIO) != 0)
  {
    aI->CloseFile(aI);
    *pErrorCode = ERROR_INVALID_INPUT_FILE;
    return;
  }
    
  // get the file information
  if (aI->GetFileInformation(aI,TRUE) != 0)
  {
      aI->CloseFile(aI);
      *pErrorCode = ERROR_INVALID_INPUT_FILE;
      return;
  }
 
  // get the tag (do this second so that we don't do it on failure)
  if (pTag == NULL)
  {
    // we don't want to analyze right away for non-local files
    // since a single I/O object is shared, we can't tag and read at the same time (i.e. in multiple threads)
    ape_BOOL bAnalyzeNow = TRUE;
    //if ((wcsnicmp(pFilename, L"http://", 7) == 0) || (wcsnicmp(pFilename, L"m01p://", 7) == 0))
    //bAnalyzeNow = FALSE;
    
    aI->m_spAPETag=&Ape_gInfoApeTag;
    ApeTagIo(aI->m_spAPETag,aI->m_spIO, bAnalyzeNow);	
  }
  else
  {
    /*Mod by Wei.Hisung 2007.03.06*/
    //m_spAPETag.Assign(pTag);
    aI->m_spAPETag=pTag;
  }

}

/*****************************************************************************************
Close the file
*****************************************************************************************/
ape_int32 ApeInfoCloseFile(CAPEInfo* aI) 
{
  // re-initialize variables
  aI->m_APEFileInfo.nSeekTableElements = 0;
  aI->m_bHasFileInformationLoaded = FALSE;
  
  return ERROR_SUCCESS;
}

void ApeIoInitialize(CIO* aI)
{
  aI->m_hFile = NULL;
  memset(aI->m_cFileName, 0, LENGTH(MAX_PATH));
  aI->m_bReadOnly = FALSE;
  aI->Close =(ape_int32 (*)(void *))ApeIoClose;
  aI->Create =(ape_int32 (*)(void *,ape_char *))ApeIoCreate;
  aI->Delete =(ape_int32 (*)(void *))ApeIoDelete;
  aI->GetName =(ape_int32 (*)(void *,ape_char *))ApeIoGetName;
  aI->GetPosition =(ape_int32 (*)(void *))ApeIoGetPosition;
  aI->GetSize =(ape_int32 (*)(void *))ApeIoGetSize;
  aI->Open =(ape_int32 (*)(void *))ApeIoOpen;
  aI->Read =(ape_int32 (*)(void *,void *,unsigned int,unsigned int *))ApeIoRead;
  aI->Seek =(ape_int32 (*)(void *, ape_int32,ape_uint32))ApeIoSeek;
  aI->SetEOF =(ape_int32 (*)(void *))ApeIoSetEof;
  aI->Write =(ape_int32 (*)(void *, void *,unsigned int,unsigned int *))ApeIoWrite;
}

ape_int32 ApeIoOpen(CIO* aI)
{
  aI->Close(aI);
  aI->m_hFile = pApeRawFileCache;//fopen((const char*)pName, "rb");
  if (aI->m_hFile == NULL) 
  {
    aI->m_hFile = pApeRawFileCache;//fopen((const char*)pName, "rb");
    if (aI->m_hFile == NULL) 
    {
        return -1;
    }
    else 
    {
        aI->m_bReadOnly = TRUE;
    }
  }
  else
  {
      aI->m_bReadOnly = FALSE;
  }

  return 0;
}

ape_int32 ApeIoRead(CIO* aI,void * pBuffer, ape_uint32 nBytesToRead, ape_uint32 * pBytesRead)
{
  //*pBytesRead=Ape_gFileOper.ApeReadBase(pBuffer,1,nBytesToRead,aI->m_hFile);
  //*pBytesRead=RKFIO_FRead(aI->m_hFile,pBuffer,nBytesToRead);	//MOD BY VINCENT
  *pBytesRead = ApeFread(aI->m_hFile, pBuffer, nBytesToRead);
  if ((*pBytesRead)==nBytesToRead)
  {
    return 0;
  }
  else
  {
    return ERROR_IO_READ;
  }
}

ape_int32 ApeIoSeek(CIO* aI,ape_int32 nDistance, ape_uint32 nMoveMode)
{
  //Ape_gFileOper.ApeSeekBase(aI->m_hFile, nDistance, nMoveMode);
  //RKFIO_FSeek(aI->m_hFile, nDistance, nMoveMode);	//MOD BY VINCENT
  ApeFseek(aI->m_hFile, nDistance, nMoveMode);
  return 0;
}

ape_int32 ApeIoGetPosition(CIO* aI)
{
  //return Ape_gFileOper.ApeTellBase(aI->m_hFile);
  //return RKFIO_FTell(aI->m_hFile);	//MOD BY VINCENT
  return ApeFtell(aI->m_hFile);
}

/*****************************************************************************************
Get the file information about the file
*****************************************************************************************/
ape_int32 ApeInfoGetFileInfo(CAPEInfo* aI,ape_BOOL bGetTagInformation) 
{
  ape_int32 nRetVal ;
  CAPEHeader APEHeader;
  // quit if there is no simple file
  if (aI->m_spIO == NULL)
  {
    return -1;
  }
  
  // quit if the file information has already been loaded
  if (aI->m_bHasFileInformationLoaded)
  {
    return ERROR_SUCCESS;
  }
  
  // use a CAPEHeader class to help us analyze the file
  //CAPEHeader APEHeader(aI->m_spIO);
  
  APEHeader.Analyze =(ape_int32 (*)(void *,struct APE_FILE_INFO *))ApeHeaderAnalyze ;
  APEHeader.AnalyzeCurrent =(ape_int32 (*)(void *,struct APE_FILE_INFO *))ApeHeaderAnalyzeCurrent;
  APEHeader.AnalyzeOld =(ape_int32 (*)(void *,struct APE_FILE_INFO *))ApeHeaderAnalyzeOld;
  APEHeader.cCAPEHeader =(void (*)(void *,CIO *))ApeHeaderInitialize;
  //APEHeader.dCAPEHeader =(void (*)(void *))ApeHeaderDelete ;
  APEHeader.FindDescriptor =(ape_int32 (*)(void *,ape_int32))ApeHeaderFindDescriptor;
  //APEHeader.cCAPEHeader(&APEHeader,aI->m_spIO);
  //ApeHeaderInitialize(&APEHeader,aI->m_spIO);
  APEHeader.Ape_pIO = aI->m_spIO;
  
  //nRetVal = APEHeader.Analyze(&APEHeader,&(aI->m_APEFileInfo));
  nRetVal = ApeHeaderAnalyze(&APEHeader,&(aI->m_APEFileInfo));
  
  // update our internal state
  if (nRetVal == ERROR_SUCCESS)
  {
    aI->m_bHasFileInformationLoaded = TRUE;
  }
  
  // return
  return nRetVal;
}

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
  aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void*)&CommonHeader, sizeof(struct APE_COMMON_HEADER), (unsigned int*)&nBytesRead);
  
  // make sure we're at the ID
  if (CommonHeader.cID[0] != 'M' || CommonHeader.cID[1] != 'A' 
      || CommonHeader.cID[2] != 'C' || CommonHeader.cID[3] != ' ')
  {
    return ERROR_UNDEFINED;
  }
  
   
   ape_DEBUG("version %d\n",CommonHeader.nVersion);
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
  ape_int32 nRetVal = aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void*)&nReadID, 4, (unsigned int*)&nBytesRead);
  ape_int32 nScanBytes = 0;
  
  
  aI->Ape_pIO->Seek(aI->Ape_pIO,0, FILE_BEGIN);
  
  
  
  aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) cID3v2Header, 10, (unsigned int*)&nBytesRead);
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
        aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) &cTemp, 1,(unsigned int*) &nBytesRead);
        while (cTemp == 0 && nBytesRead == 1)
        {
            nJunkBytes++;
            aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) &cTemp, 1,(unsigned int*) &nBytesRead);
        }
    }
  }
  aI->Ape_pIO->Seek(aI->Ape_pIO,nJunkBytes, FILE_BEGIN);
  
  // scan until we hit the APE_DESCRIPTOR, the end of the file, or 1 MB later
  nGoalID = (' ' << 24) | ('C' << 16) | ('A' << 8) | ('M');
  nReadID = 0;
  nRetVal = aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void*)&nReadID, 4,(unsigned int*) &nBytesRead);
  if (nRetVal != 0 || nBytesRead != 4)
  {
    return ERROR_UNDEFINED;
  }
  
  nBytesRead = 1;
  
  //while ((nGoalID != nReadID) && (nBytesRead == 1) && (nScanBytes < (1024 * 1024)))
  while ((nGoalID != nReadID) && (nBytesRead == 1) && (nScanBytes < ( 10240)))
  {
    ape_uchar cTemp;
    aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void*)&cTemp, 1,(unsigned int*) &nBytesRead);
    nReadID = (((ape_uint32) cTemp) << 24) | (nReadID >> 8);
    nJunkBytes++;
    nScanBytes++;
  }
  
  if (nGoalID != nReadID)
  {
    nJunkBytes = -1;
  }

  if (nJunkBytes != -1)
  {
      // successfully found the start of the file (seek to it and return)
      aI->Ape_pIO->Seek(aI->Ape_pIO,nJunkBytes, FILE_BEGIN);
  }
  // seek to the proper place (depending on result and settings)
  /*
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
  */
  
  return nJunkBytes;
}


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
    aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void*)pInfo->spAPEDescriptor, sizeof(struct APE_DESCRIPTOR),(unsigned int*) &nBytesRead);
    
    if ((pInfo->spAPEDescriptor->nDescriptorBytes - nBytesRead) > 0)
    {
        aI->Ape_pIO->Seek(aI->Ape_pIO,pInfo->spAPEDescriptor->nDescriptorBytes - nBytesRead, FILE_CURRENT);
    }
    
    // read the header
    aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void*)&APEHeader, sizeof(APEHeader),(unsigned int*) &nBytesRead);
    
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
    	return ERROR_UNDEFINED;// any.woo 13/7/02
    }
    
    pInfo->nFormatFlags            = (ape_int32)(APEHeader.nFormatFlags);
    pInfo->nTotalFrames            = (ape_int32)(APEHeader.nTotalFrames);
    pInfo->nFinalFrameBlocks    = (ape_int32)(APEHeader.nFinalFrameBlocks);
    pInfo->nBlocksPerFrame        = (ape_int32)(APEHeader.nBlocksPerFrame);
    pInfo->nChannels            = (ape_int32)(APEHeader.nChannels);
    pInfo->nSampleRate            = (ape_int32)(APEHeader.nSampleRate);
    pInfo->nBitsPerSample        = (ape_int32)(APEHeader.nBitsPerSample);
	if(pInfo->nBitsPerSample != 16)
	{
		return ERROR_UNDEFINED; // huweiguo, 09/04/08
	}

    pInfo->nBytesPerSample        = pInfo->nBitsPerSample / 8;
    pInfo->nBlockAlign            = pInfo->nBytesPerSample * pInfo->nChannels;
    pInfo->nTotalBlocks            = (APEHeader.nTotalFrames == 0) ? 0 : ((APEHeader.nTotalFrames -  1) * pInfo->nBlocksPerFrame) + APEHeader.nFinalFrameBlocks;
    pInfo->nWAVHeaderBytes        = (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER) ? sizeof(struct WAVE_HEADER) : pInfo->spAPEDescriptor->nHeaderDataBytes;
    pInfo->nWAVTerminatingBytes    = pInfo->spAPEDescriptor->nTerminatingDataBytes;
    pInfo->nWAVDataBytes        = pInfo->nTotalBlocks * pInfo->nBlockAlign;
    pInfo->nWAVTotalBytes        = pInfo->nWAVDataBytes + pInfo->nWAVHeaderBytes + pInfo->nWAVTerminatingBytes;
    
    pInfo->nAPETotalBytes        = aI->Ape_pIO->GetSize(aI->Ape_pIO);
    pInfo->nLengthMS            = (ape_int32)(((double)(pInfo->nTotalBlocks) * (double)(1000)) / (double)(pInfo->nSampleRate));
    pInfo->nAverageBitrate        = (pInfo->nLengthMS <= 0) ? 0 :((((long long)pInfo->nAPETotalBytes) * 8000) /(pInfo->nLengthMS));
    pInfo->nDecompressedBitrate = (pInfo->nBlockAlign * pInfo->nSampleRate * 8) / 1000;
    pInfo->nSeekTableElements    = pInfo->spAPEDescriptor->nSeekTableBytes / 4;

#if 0	
    // get the seek tables (really no reason to get the whole thing if there's extra)
    //pInfo->spSeekByteTable=(ape_uint32*)malloc(sizeof(ape_uint32)*pInfo->nSeekTableElements);
    //if (pInfo->spSeekByteTable == NULL) { return ERROR_UNDEFINED; }
    if (pInfo->nSeekTableElements-1>SEEKBYTE_TABLE_MAX)//if (pInfo->nTotalFrames-1>SEEKBYTE_TABLE_MAX)
    {
    	return ERROR_UNDEFINED;
    }
    else
    {
          pInfo->spSeekByteTable=(Ape_gHeaderSeekByteTable);
    }
    
    /*Mod by Wei.Hisung 2007.03.06*/
    aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) pInfo->spSeekByteTable,(unsigned int) 4 * pInfo->nSeekTableElements,(unsigned int*) &nBytesRead);
#else
	// 由于seek table所占的空间比较大，现在只取部分的seek table
	pInfo->spSeekByteTable=(Ape_gHeaderSeekByteTable);
    Ape_gHeaderSeekByteTableOffset = aI->Ape_pIO->GetPosition((void*)aI->Ape_pIO);
	if(pInfo->nSeekTableElements > SEEKBYTE_TABLE_MAX)
	{
		aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) pInfo->spSeekByteTable,(unsigned int) 4 * SEEKBYTE_TABLE_MAX, (unsigned int*) &nBytesRead);
		aI->Ape_pIO->Seek((void*)aI->Ape_pIO, (unsigned int) 4 * (pInfo->nSeekTableElements-SEEKBYTE_TABLE_MAX), FILE_CURRENT);
	}
	else
	{
		aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) pInfo->spSeekByteTable,(unsigned int) 4 * pInfo->nSeekTableElements,(unsigned int*) &nBytesRead);
	}
#endif
    
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

//Decode Ape header information of 3.98v and before
ape_int32 ApeHeaderAnalyzeOld(CAPEHeader *aI,struct APE_FILE_INFO * pInfo)
{
    // variable declares
    ape_uint16 nBytesRead = 0;

    // read the MAC header from the file
    struct APE_HEADER_OLD APEHeader;
	  ape_int32 nPeakLevel = -1;

    aI->Ape_pIO->Seek(aI->Ape_pIO,pInfo->nJunkHeaderBytes, FILE_BEGIN);
    aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) &APEHeader, sizeof(APEHeader),(unsigned int*)&nBytesRead);

    // fail on 0 length APE files (catches non-finalized APE files)
    if (APEHeader.nTotalFrames == 0)
    {
        return ERROR_UNDEFINED;
    }


    if (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL)
    {
        aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) &nPeakLevel, 4, (unsigned int*)&nBytesRead);
    }

    if (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS)
    {
        aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) &pInfo->nSeekTableElements, 4,(unsigned int*)&nBytesRead);
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
	if(pInfo->nBitsPerSample != 16)
	{
		return ERROR_UNDEFINED; // huweiguo, 09/04/08
	}
	
    pInfo->nBytesPerSample        = pInfo->nBitsPerSample / 8;
    pInfo->nBlockAlign            = pInfo->nBytesPerSample * pInfo->nChannels;
    pInfo->nTotalBlocks            = (APEHeader.nTotalFrames == 0) ? 0 : ((APEHeader.nTotalFrames -  1) * pInfo->nBlocksPerFrame) + APEHeader.nFinalFrameBlocks;
    pInfo->nWAVHeaderBytes        = (APEHeader.nFormatFlags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER) ? sizeof(struct WAVE_HEADER) : APEHeader.nHeaderBytes;
    pInfo->nWAVTerminatingBytes    = (ape_int32)(APEHeader.nTerminatingBytes);
    pInfo->nWAVDataBytes        = pInfo->nTotalBlocks * pInfo->nBlockAlign;
    pInfo->nWAVTotalBytes        = pInfo->nWAVDataBytes + pInfo->nWAVHeaderBytes + pInfo->nWAVTerminatingBytes;
    pInfo->nAPETotalBytes        = aI->Ape_pIO->GetSize(aI->Ape_pIO);
    pInfo->nLengthMS            = (ape_int32)(((double)(pInfo->nTotalBlocks) * (double)(1000)) / (double)(pInfo->nSampleRate));
    pInfo->nAverageBitrate        = (pInfo->nLengthMS <= 0) ? 0 :((((long long)pInfo->nAPETotalBytes) * 8000) /(pInfo->nLengthMS));
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
   #if 0
    if (pInfo->nTotalFrames-1>SEEKBYTE_TABLE_MAX)
    {
      return ERROR_UNDEFINED;
    }
    else
    {
          pInfo->spSeekByteTable=(Ape_gHeaderSeekByteTable);
    }
    #else
      pInfo->spSeekByteTable=(Ape_gHeaderSeekByteTable);
    #endif 

	Ape_gHeaderSeekByteTableOffset = aI->Ape_pIO->GetPosition((void*)aI->Ape_pIO);
	if(pInfo->nSeekTableElements > SEEKBYTE_TABLE_MAX)
	{
		aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) pInfo->spSeekByteTable,(unsigned int) 4 * SEEKBYTE_TABLE_MAX, (unsigned int*) &nBytesRead);
		aI->Ape_pIO->Seek((void*)aI->Ape_pIO, (unsigned int) 4 * (pInfo->nSeekTableElements-SEEKBYTE_TABLE_MAX), FILE_CURRENT);
	}
	else 
	{
	    /*Mod by Wei.Hisung 2007.03.06*/
	    //Ape_pIO->Read((ape_uchar *) pInfo->spSeekByteTable.GetPtr(), 4 * pInfo->nSeekTableElements, &nBytesRead);
	    aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) pInfo->spSeekByteTable, 4 * pInfo->nSeekTableElements,(unsigned int*)&nBytesRead);
	}
	
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
      aI->Ape_pIO->Read((void*)aI->Ape_pIO,(void *) pInfo->spSeekBitTable, (unsigned int)pInfo->nTotalFrames, (unsigned int*)&nBytesRead);
      //aI->Ape_pIO->Read(aI->Ape_pIO,Ape_gReadBuffer, pInfo->nTotalFrames, &nBytesRead);
      //Byte2Word2(pInfo->spSeekBitTable,Ape_gReadBuffer,nBytesRead/2);
      if (pInfo->nSeekTableElements >pInfo->nTotalFrames)
      {
         aI->Ape_pIO->Seek(aI->Ape_pIO,(pInfo->nSeekTableElements - pInfo->nTotalFrames), FILE_CURRENT);
      }
    }
    
    return ERROR_SUCCESS;
}

//ape tag 解析
void ApeTagIo(CAPETag* aI,CIO * pIO, ape_BOOL bAnalyze)
{
  /*Mod by Wei.Hisung 2007.03.06*/
  //m_spIO.Assign(pIO, FALSE, FALSE); // we don't own the IO source
  aI->m_spIO=pIO; // we don't own the IO source
  aI->m_bAnalyzed = FALSE;
  aI->m_nFields = 0;
  aI->m_nTagBytes = 0;
  
  aI->Analyze =(ape_int32 (*)(void *))ApeTagAnalyze ;
  aI->cCAPETagFile =(void (*)(void *,ape_char *,ape_int32))ApeTagFileInitialize ;
  aI->cCAPETagIO =(void (*)(void *,CIO *,ape_BOOL))ApeTagIo ;
  //aI->dCAPETag =(void ( *)(void *))CAPETag_dCAPETag ;
  aI->GetAPETagVersion =(ape_int32 ( *)(void *))ApeTagGetApeTagVersion ;
  aI->GetHasAPETag =(ape_int32 ( *)(void *))ApeTagGetHasApeTag ;
  aI->GetHasID3Tag =(ape_int32 ( *)(void *))ApeTagGetHasId3Tag ;
  aI->GetTagBytes =(ape_int32 ( *)(void *))ApeTagGetTagBytes ;
  aI->LoadField =(ape_int32 ( *)(void *, ape_char *,ape_int32,ape_int32 *))ApeTagLoadField ;
    
  if (bAnalyze)
  {
     aI->Analyze(aI);
  }
}


ape_int32 ApeTagAnalyze(CAPETag* aI)
{
  // clean-up
  //struct ID3_TAG Ape_gId3Tag;
  ape_int32 nOriginalPosition;
  ape_uint32 nBytesRead;
  ape_int32 nRetVal;
  //aI->ClearFields(aI);
  aI->m_nTagBytes = 0;
  
  aI->m_bAnalyzed = TRUE;
  
  // store the original location
  nOriginalPosition = aI->m_spIO->GetPosition(aI->m_spIO);
  
  // check for a tag
  
  aI->m_bHasID3Tag = FALSE;
  aI->m_bHasAPETag = FALSE;
  aI->m_nAPETagVersion = -1;
  aI->m_spIO->Seek(aI->m_spIO,-ID3_TAG_BYTES, FILE_END);
  nRetVal = aI->m_spIO->Read((void*)aI->m_spIO,(void *) &Ape_gId3Tag, sizeof(struct ID3_TAG),(unsigned int*) &nBytesRead);
  //nRetVal = aI->m_spIO->Read(aI->m_spIO,&Ape_gReadBuffer, sizeof(struct ID3_TAG)*2, &nBytesRead);
  //Byte2Word2(&Ape_gId3Tag,&Ape_gReadBuffer,sizeof(struct ID3_TAG));
  
  if ((nBytesRead == sizeof(struct ID3_TAG)) && (nRetVal == 0))
  {
    if (Ape_gId3Tag.Header[0] == 'T' && Ape_gId3Tag.Header[1] == 'A' && Ape_gId3Tag.Header[2] == 'G') 
    {
        aI->m_bHasID3Tag = TRUE;
        aI->m_nTagBytes += ID3_TAG_BYTES;
    }
  }
  
  // set the fields
  if (aI->m_bHasID3Tag)
  {
  /*
  char cTemp[16];
  
  aI->SetFieldID3String(aI,APE_TAG_FIELD_ARTIST, Ape_gId3Tag.Artist, 30);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_ALBUM, Ape_gId3Tag.Album, 30);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_TITLE, Ape_gId3Tag.Title, 30);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_COMMENT, Ape_gId3Tag.Comment, 28);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_YEAR, Ape_gId3Tag.Year, 4);
  
  sprintf(cTemp, "%d", Ape_gId3Tag.Track);
  aI->SetFieldStringCC(aI,APE_TAG_FIELD_TRACK, cTemp, FALSE);
  
  if ((Ape_gId3Tag.Genre == GENRE_UNDEFINED) || (Ape_gId3Tag.Genre >= GENRE_COUNT)) 
      aI->SetFieldStringUtf(aI,APE_TAG_FIELD_GENRE, APE_TAG_GENRE_UNDEFINED);
  else 
      aI->SetFieldStringUtf(aI,APE_TAG_FIELD_GENRE, g_ID3Genre[Ape_gId3Tag.Genre]);
  */
  }
  
  // try loading the APE tag
  if (aI->m_bHasID3Tag == FALSE)
  {
    APE_TAG_FOOTER APETagFooter;
    APETagFooter.cAPE_TAG_FOOTER =(void (*)(void *,ape_int32,ape_int32))ApeTagFooter ;
    APETagFooter.GetFieldBytes =(ape_int32 (*)(void *))ApeTagFooterGetFieldBytes; 
    APETagFooter.GetFieldsOffset= (ape_int32 (*)(void *))ApeTagFooterGetFieldsOffset;
    APETagFooter.GetHasHeader=(ape_int32 (*)(void *))ApeTagFooterGetHasHeader ;
    APETagFooter.GetIsHeader=(ape_int32 (*)(void *))ApeTagFooterGetIsHeader ;
    APETagFooter.GetIsValid=(ape_int32 (*)(void *,ape_int32))ApeTagFooterGetIsValid ;
    APETagFooter.GetNumberFields=(ape_int32 (*)(void *))ApeTagFooterGetNumberFields ;
    APETagFooter.GetTotalTagBytes=(ape_int32 (*)(void *))ApeTagFooterGetTotalTagBytes ;
    APETagFooter.GetVersion=(ape_int32 (*)(void *))ApeTagFooterGetVersion; 
    
    aI->m_spIO->Seek(aI->m_spIO,-(ape_int32)(APE_TAG_FOOTER_BYTES), FILE_END);
    nRetVal = aI->m_spIO->Read((void *)aI->m_spIO,(void *) &APETagFooter, APE_TAG_FOOTER_BYTES,(unsigned int*) &nBytesRead);
    //nRetVal = aI->m_spIO->Read(aI->m_spIO,&Ape_gReadBuffer, APE_TAG_FOOTER_BYTES, &nBytesRead);
    //Byte2Word2(&APETagFooter,&Ape_gReadBuffer,(ape_uint16)(APE_TAG_FOOTER_BYTES/2));
    if ((nBytesRead == APE_TAG_FOOTER_BYTES) && (nRetVal == 0))
    {
      if (APETagFooter.GetIsValid(&APETagFooter,FALSE))
      {
        ape_int32 nRawFieldBytes;
        ape_char* spRawTag;
        
        aI->m_bHasAPETag = TRUE;
        aI->m_nAPETagVersion = APETagFooter.GetVersion(&APETagFooter);
        
        nRawFieldBytes = APETagFooter.GetFieldBytes(&APETagFooter);
        aI->m_nTagBytes += APETagFooter.GetTotalTagBytes(&APETagFooter);
        
        spRawTag=Ape_gReadBuffer;
        
        aI->m_spIO->Seek(aI->m_spIO,-(APETagFooter.GetTotalTagBytes(&APETagFooter) - APETagFooter.GetFieldsOffset(&APETagFooter)), FILE_END);
        /*Mod by Wei.Hisung 2007.03.06*/
        //nRetVal = m_spIO->Read((ape_uchar *) spRawTag.GetPtr(), nRawFieldBytes, &nBytesRead);
        //nRetVal = aI->m_spIO->Read(aI->m_spIO,(ape_uchar *) spRawTag, nRawFieldBytes, &nBytesRead);
        
        //if ((nRetVal == 0) && (nRawFieldBytes == (ape_int32)(nBytesRead)))
        {
          // parse out the raw fields
          ape_int32 nLocation = 512;
          ape_int32 nBytesRead = 512;
          ape_int32 z=0;
          //add by Wei.hsiung
          aI->m_nFields=0;
          
          for (z = 0; z < APETagFooter.GetNumberFields(&APETagFooter); z++)
          {
            //ape_int32 nMaximumFieldBytes = nRawFieldBytes - nLocation;
            
            ape_int32 nBytes = 0;
            
            aI->m_spIO->Seek(aI->m_spIO,(nLocation-nBytesRead), FILE_CURRENT);
            
            aI->m_spIO->Read((void*)aI->m_spIO,(void *) spRawTag, 512 , (unsigned int*)&nBytesRead);
            
            //nLocation+=nBytesRead;
            
            nLocation = aI->LoadField(aI,spRawTag, nBytesRead, &nBytes);
            
            if ((nLocation == (-1))||(z>APETAG_FIELDS_NUM_MAX-1))
            {
              break;
            }
                    //if (aI->LoadField(aI,&spRawTag[nLocation], nMaximumFieldBytes, &nBytes) != ERROR_SUCCESS)
                    //{
                        // if LoadField(...) fails, it means that the tag is corrupt (accidently or intentionally)
                        // we'll just bail out -- leaving the fields we've already set
                        //break;
                    //}
                    //nLocation += nBytes;
          }
        }
      }
    }
  }

    // restore the file pointer
    aI->m_spIO->Seek(aI->m_spIO,nOriginalPosition, FILE_BEGIN);
    
    return ERROR_SUCCESS;
}

ape_BOOL ApeTagFooterGetIsValid(APE_TAG_FOOTER *aI,ape_BOOL bAllowHeader)
{
  ape_BOOL bValid = (aI->m_cID[0] == 'A') && (aI->m_cID[1] == 'P') && (aI->m_cID[2] == 'E') && (aI->m_cID[3] == 'T') &&
  (aI->m_cID[4] == 'A') && (aI->m_cID[5] == 'G') && (aI->m_cID[6] == 'E') && (aI->m_cID[7] == 'X') &&
  (aI->Ape_gVersion <= CURRENT_APE_TAG_VERSION) &&
  (aI->m_nFields <= (ape_int32)65536) &&
  (aI->GetFieldBytes(aI) <= ((ape_int32)1024 * (ape_int32)1024 * (ape_int32)16));
  
  if (bValid && (bAllowHeader == FALSE) && aI->GetIsHeader(aI))
  {
    bValid = FALSE;
  }
  
  return bValid ? TRUE : FALSE;
}

#pragma arm section code 

#endif
#endif
