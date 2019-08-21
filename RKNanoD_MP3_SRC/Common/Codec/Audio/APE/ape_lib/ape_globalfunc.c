
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_globalfunc.h"
#include "ape_io1.h"

ape_int32 ApeReadSafe(CIO * pIO, void * pBuffer, ape_int32 nBytes)
{
  ape_uint32 nBytesRead = 0;
  ape_int32 nRetVal = pIO->Read((void*)pIO,pBuffer, (unsigned int)nBytes,(unsigned int*) &nBytesRead);
  if (nRetVal == ERROR_SUCCESS)
  {
    if (nBytes != (ape_int32)(nBytesRead))
    {
      nRetVal = ERROR_IO_READ;
    }
  }
  
  return nRetVal;
}

ape_int32 ApeWriteSafe(CIO * pIO, void * pBuffer, ape_int32 nBytes)
{
  ape_uint32 nBytesWritten = 0;
  ape_int32 nRetVal = pIO->Write((void*)pIO,pBuffer, (unsigned int)nBytes, (unsigned int*)&nBytesWritten);
  if (nRetVal == ERROR_SUCCESS)
  {
    if (nBytes != (ape_int32)(nBytesWritten))
    {
      nRetVal = ERROR_IO_WRITE;
    }
  }
  
  return nRetVal;
}

#pragma arm section code 

#endif
#endif
