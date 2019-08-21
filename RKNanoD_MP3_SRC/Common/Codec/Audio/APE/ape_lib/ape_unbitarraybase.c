
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_unbitarraybase.h"
#include "ape_Infodec.h"
#include "ape_unbitarray.h"
#ifdef BACKWARDS_COMPATIBILITY
    #include "ape_decompold.h"
    #include "ape_unbitarrayold.h"
#endif
#include "ape_globalvardeclaration.h"


void ApeUnBitArrayBaseDelete()
{
}

void ApeUnBitArrayBaseFinalize()
{
}

void ApeUnBitArrayBaseFlushBitArray()
{
}

ape_int32 ApeUnBitArrayBaseDecodeValueRange(struct UNBIT_ARRAY_STATE * BitArrayState) 
{
	return 0; 
}

void ApeUnBitArrayBaseFlushState(struct UNBIT_ARRAY_STATE * BitArrayState) 
{
}

void ApeUnBitArrayBaseGenerateArray(ape_int32 * pOutputArray, ape_int32 nElements, ape_int32 nBytesRequired ) 
{
}

ape_uint32 ApeUnBitArrayBaseDecodeValue(enum DECODE_VALUE_METHOD DecodeMethod, ape_int32 nParam1 , ape_int32 nParam2 ) 
{
	return 0; 
}

//CUnBitArrayBase * ApeUnBitArrayCreate(IAPEDecompress * pAPEDecompress, ape_int32 nVersion)
CUnBitArrayBase * ApeUnBitArrayCreate(void * pAPEDecompress, ape_int32 nVersion)
{
  if (nVersion >= 3900)
	{
    /*Mod by Wei.Hisung 2007.03.06*/
    CUnBitArrayBase * pTmp = (CUnBitArrayBase * )&Ape_gUnBitArrayBaseMalloc;//malloc(sizeof(CUnBitArray));added by hxd 20070321
    pTmp->AdvanceToByteBoundary = ApeUnBitArrayAdvanceToByteBoundary;
    pTmp->CreateHelper = ApeUnBitArrayCreateHelper ;
    //pTmp->dCUnBitArrayBase =ApeUnBitArrayDelete;
    pTmp->DecodeValue = ApeUnBitArrayDecodeValue;
    pTmp->DecodeValueRange = ApeUnBitArrayDecodeValueRange;
    pTmp->DecodeValueXBits = ApeUnBitArrayDecodeValueXBits;
    pTmp->FillAndResetBitArray = ApeUnBitArrayFillAndResetBitArray;
    pTmp->FillBitArray =ApeUnBitArrayFillBitArray;
    pTmp->Finalize =ApeUnBitArrayFinalize;
    pTmp->FlushBitArray =ApeUnBitArrayFlushBitArray;
    pTmp->FlushState = ApeUnBitArrayFlushState;
  //  pTmp->GenerateArray =ApeUnBitArrayGenerateArray;
    //#define GET_IO(APE_INFO) ((CIO *) (APE_INFO)->GetInfo(APE_INFO_IO_SOURCE))
    //ApeUnBitArrayInitialize(GET_IO(pAPEDecompress), nVersion);
    ApeUnBitArrayInitialize((CIO *)((IAPEDecompress *)pAPEDecompress)->GetInfo(APE_INFO_IO_SOURCE,0,0), nVersion);
    return pTmp;
	}
  else
	{
    /*Mod by Wei.Hisung 2007.03.06*/
    CUnBitArrayBase * pTmp = (CUnBitArrayBase * )&Ape_gUnBitArrayBaseMalloc;//malloc(sizeof(CUnBitArrayOld));added by hxd 20070321
    //pTmp->AdvanceToByteBoundary = ApeUnBitArrayOldAdvanceToByteBoundary;
    //pTmp->CreateHelper = ApeUnBitArrayOldCreateHelper ;
    //pTmp->dCUnBitArrayBase =ApeUnBitArrayOldDelete;
    //pTmp->DecodeValue = ApeUnBitArrayOldDecodeValue;
    //pTmp->DecodeValueRange = ApeUnBitArrayOldDecodeValueRange;
    //pTmp->DecodeValueXBits = ApeUnBitArrayOldDecodeValueXBits;
    //pTmp->FillAndResetBitArray = ApeUnBitArrayOldFillAndResetBitArray;
    //pTmp->FillBitArray =ApeUnBitArrayOldFillBitArray;
    //pTmp->Finalize =ApeUnBitArrayOldFinalize;
    //pTmp->FlushBitArray =ApeUnBitArrayOldFlushBitArray;
    //pTmp->FlushState = ApeUnBitArrayOldFlushState;
    //pTmp->GenerateArray =ApeUnBitArrayOldGenerateArray;
    //ApeUnBitArrayOldInitialize((IAPEDecompress *)pAPEDecompress, nVersion);
    return pTmp;
	}
}

void ApeUnBitArrayBase_AdvanceToByteBoundary() 
{
  ape_int32 nMod = Ape_gCurrentBitIndexBase % 8;
  if (nMod != 0)
  {
    Ape_gCurrentBitIndexBase += 8 - nMod;
  }
}

ape_uint32 ApeUnBitArrayBase_DecodeValueXBits(ape_uint32 nBits) 
{
  ape_uint32 nLeftBits;
  ape_uint32 nBitArrayIndex;
  ape_int32 nRightBits;
  ape_uint32 nLeftValue;
  ape_uint32 nRightValue;
  
  // get more data if necessary
  if ((Ape_gCurrentBitIndexBase + nBits) >= Ape_gBitsValueBase)
  {
    ApeUnBitArrayBaseFillBitArray();
  }
  
  // variable declares
  nLeftBits = 32 - (Ape_gCurrentBitIndexBase & 31);
  nBitArrayIndex = Ape_gCurrentBitIndexBase >> 5;
  Ape_gCurrentBitIndexBase += nBits;
  
  // if their isn't an overflow to the right value, get the value and exit
  if (nLeftBits >= nBits)
  {
   return (Ape_pBitArrayBase[nBitArrayIndex] & (Ape_gtPowersOfTwoMinusOne[nLeftBits])) >> (nLeftBits - nBits);
  }
  
  // must get the "split" value from left and right
  nRightBits = nBits - nLeftBits;
  
  nLeftValue = ((Ape_pBitArrayBase[nBitArrayIndex] & Ape_gtPowersOfTwoMinusOne[nLeftBits]) << nRightBits);
  nRightValue = (Ape_pBitArrayBase[nBitArrayIndex + 1] >> (32 - nRightBits));
  return (nLeftValue | nRightValue);
}

ape_int32 ApeUnBitArrayBase_FillAndResetBitArray(ape_int32 nFileLocation, ape_int32 nNewBitIndex) 
{
  ape_uint32 nBytesRead;
  // reset the bit index
  Ape_gCurrentBitIndexBase = nNewBitIndex;
  
  // seek if necessary
  if (nFileLocation != -1)
  {
    if (Ape_pIOBase->Seek(Ape_pIOBase,nFileLocation, FILE_BEGIN) != 0)
    {
      return ERROR_IO_READ;
    }
  }
      
  // read the new data into the bit array
  nBytesRead = 0;
  if (Ape_pIOBase->Read(Ape_pIOBase,((ape_uchar *) Ape_pBitArrayBase), Ape_gBytesValueBase,(unsigned int*) &nBytesRead) != 0)
  {
    return ERROR_IO_READ;
  }
  
  return 0;
}

ape_int32 ApeUnBitArrayBaseFillBitArray() 
{
  ape_int32 nBytesToRead;
  ape_uint32 nBytesRead;
  ape_int32 nRetVal;
  
  // get the bit array index
  ape_uint32 nBitArrayIndex = Ape_gCurrentBitIndexBase >> 5;
  
  // move the remaining data to the front
  memmove((void *) (Ape_pBitArrayBase), ( void *) (Ape_pBitArrayBase + nBitArrayIndex), LENGTH(Ape_gBytesValueBase - (nBitArrayIndex * 4)));
  
  // read the new data
  nBytesToRead = nBitArrayIndex * 4;
  nBytesRead = 0;
  nRetVal = Ape_pIOBase->Read(Ape_pIOBase,(ape_uchar *) (Ape_pBitArrayBase + Ape_gElementsBase - nBitArrayIndex), nBytesToRead, (unsigned int*)&nBytesRead);
  
  // adjust the m_Bit pointer
  Ape_gCurrentBitIndexBase = Ape_gCurrentBitIndexBase & 31;
  
  // return
  return (nRetVal == 0) ? 0 : ERROR_IO_READ;
}

//ape_int32 ApeUnBitArrayBaseCreateHelper(CIO * pIO, ape_int32 nBytes, ape_int32 nVersion)
//{
//    // check the parameters
//    if ((pIO == NULL) || (nBytes <= 0)) { return ERROR_BAD_PARAMETER; }
//
//    // save the size
//    Ape_gElementsBase = nBytes / 4;
//    Ape_gBytesValueBase = Ape_gElementsBase * 4;
//    Ape_gBitsValueBase = Ape_gBytesValueBase * 8;
//    
//    // set the variables
//    Ape_pIOBase = pIO;
//    Ape_gVersionBase = nVersion;
//    Ape_gCurrentBitIndexBase = 0;
//    
//    // create the bitarray
//    Ape_pBitArrayBase =(uint32*)malloc(sizeof(uint32)*Ape_gElementsBase);
//    
//    return (Ape_pBitArrayBase != NULL) ? 0 : ERROR_INSUFFICIENT_MEMORY;


#pragma arm section code

#endif
#endif
