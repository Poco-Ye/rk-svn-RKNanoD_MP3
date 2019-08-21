
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"

//#ifdef IO_USE_WIN_FILE_IO

#include "ape_io1.h"
//#include "CharacterHelper.h"
//#include <stdio.h>
//#include "../include/file_access.h"

//ape_int32 ApeGetFileSize(FILE * hFile);

#if 0 //commented by hxd 20070710
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
  aI->Read =(ape_int32 (*)(void *,void *,ape_uint32,ape_uint32 *))ApeIoRead;
  aI->Seek =(ape_int32 (*)(void *,ape_int32,ape_uint32))ApeIoSeek;
  aI->SetEOF =(ape_int32 (*)(void *))ApeIoSetEof;
  aI->Write =(ape_int32 (*)(void *, void *,ape_uint32,ape_uint32 *))ApeIoWrite;
}
#endif

#if 0 //commented by hxd 20070710
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
#endif

ape_int32 ApeIoClose(CIO* aI)
{
#if 0
	if (aI->m_hFile != NULL) 
	{
		fclose(aI->m_hFile);
		aI->m_hFile =NULL;
	}
    return 0;
#else
	return 0;
#endif
}

#if 0 //commented by hxd 20070710
ape_int32 ApeIoRead(CIO* aI,void * pBuffer, ape_uint32 nBytesToRead, ape_uint32 * pBytesRead)
{
	*pBytesRead=RKFIO_FRead(pBuffer,nBytesToRead,aI->m_hFile);//fread(pBuffer,1,nBytesToRead,aI->m_hFile);
	if ((*pBytesRead)==nBytesToRead)
	{
		return 0;
	}
	else
	{
		return ERROR_IO_READ;
	}
}
#endif

ape_int32 ApeIoWrite(CIO* aI, void * pBuffer, ape_uint32 nBytesToWrite, ape_uint32 * pBytesWritten)
{
#if 0
	*pBytesWritten=fwrite(pBuffer,1,nBytesToWrite,aI->m_hFile);
	//*pBytesWritten=fwrite_8bit(pBuffer,1,nBytesToWrite,aI->m_hFile);
	//*pBytesWritten=fwrite(pBuffer,nBytesToWrite/2,1,aI->m_hFile);

    //if ((*pBytesWritten != nBytesToWrite))
        //return ERROR_IO_WRITE;
    //else
        return 0;
#else
	return 0;
#endif	
}

#if 0 //commented by hxd 20070710
ape_int32 ApeIoSeek(CIO* aI,ape_int32 nDistance, ape_uint32 nMoveMode)
{
  RKFIO_FSeek(aI->m_hFile, nDistance, nMoveMode);//fseek(aI->m_hFile, nDistance, nMoveMode);
  return 0;
}
#endif

ape_int32 ApeIoSetEof(CIO* aI)
{
	return 0;
}

#if 0 //commented by hxd 20070710
ape_int32 ApeIoGetPosition(CIO* aI)
{
  return RKFIO_FLength(aI->m_hFile);//ftell(aI->m_hFile);
}
#endif

ape_int32 ApeIoGetSize(CIO* aI)
{
    return ApeGetFileSize(aI->m_hFile);
   // return 0;
}

ape_int32 ApeIoGetName(CIO* aI,ape_char * pBuffer)
{
    //wcscpy(pBuffer, aI->m_cFileName);
    return 0;
}

ape_int32 ApeIoCreate(CIO* aI,ape_char * pName)
{
#if 0
	aI->Close(aI);

	aI->m_hFile = fopen((const char*)pName,"wb+");
	if (aI->m_hFile == NULL) { return -1; }

    aI->m_bReadOnly = FALSE;
    
    //wcscpy(aI->m_cFileName, pName);
#else
    return 0;
#endif
}

ape_int32 ApeIoDelete(CIO* aI)
{
	return 0;
}

#if 0
ape_int32 ApeGetFileSize(FILE * hFile)
{

	return (RKFIO_FLength(hFile));
	/*
	ape_int32 OrgPos=ftell(hFile);
	ape_int32 fSize=0;
	
	fseek(hFile,0,SEEK_END);
	fSize=ftell(hFile);
	fseek(hFile,OrgPos,SEEK_SET);
	
//	fseek_8bit(hFile,0,SEEK_END);
	//fSize=ftell_8bit(hFile);
//	fseek_8bit(hFile,OrgPos,SEEK_SET);
	return(fSize);
	*/
}
#endif
//#endif // #ifdef IO_USE_WIN_FILE_IO

#pragma arm section code

#endif
#endif
