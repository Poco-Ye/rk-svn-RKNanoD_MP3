
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_rollbuffer.h"
#include "ape_all.h"

#define HISTORY_ELEMENTS 8
#include "ape_globalvardeclaration.h"

void ApeRollBufferInitialize(void *aI)
{	
  ((CRollBuffer*)aI)->m_pData = NULL;
  ((CRollBuffer*)aI)->m_pCurrent = NULL;
}

#if 0
void ApeRollBufferDelete(void *aI)
{		
	//SAFE_ARRAY_DELETE(((CRollBuffer*)aI)->m_pData);
}
#endif

ape_int32 ApeRollBufferCreate_rbInput(void *aI,ape_int32 nWindowElements, ape_int32 nHistoryElements,ape_int16 xyflag,ape_int16 filterflag)
{
  //SAFE_ARRAY_DELETE(((CRollBuffer*)aI)->m_pData)	
  ((CRollBuffer*)aI)->m_nWindowElements = nWindowElements;
  ((CRollBuffer*)aI)->m_nHistoryElements = nHistoryElements;
  
  ((CRollBuffer*)aI)->m_pData =(ape_int32*)&Ape_gInputCreateMalloc[filterflag][xyflag][0];//malloc(sizeof(short)*(((CRollBuffer*)aI)->m_nWindowElements + ((CRollBuffer*)aI)->m_nHistoryElements));
  //((CRollBuffer*)aI)->m_pData =(ape_int16*)&Ape_gInputCreateMalloc[filterflag][xyflag][0];//malloc(sizeof(short)*(((CRollBuffer*)aI)->m_nWindowElements + ((CRollBuffer*)aI)->m_nHistoryElements));
   if (((CRollBuffer*)aI)->m_pData == NULL)
   {
     return ERROR_INSUFFICIENT_MEMORY;
   }
   ApeRollBufferFlush(aI);
   return 0;
}

ape_int32 ApeRollBufferCreate_rbDeltaM(void *aI,ape_int32 nWindowElements, ape_int32 nHistoryElements,ape_int16 xyflag,ape_int16 filterflag)
{
  //SAFE_ARRAY_DELETE(((CRollBuffer*)aI)->m_pData)	
  ((CRollBuffer*)aI)->m_nWindowElements = nWindowElements;
  ((CRollBuffer*)aI)->m_nHistoryElements = nHistoryElements;
  //((CRollBuffer*)aI)->m_pData = new TYPE[m_nWindowElements + m_nHistoryElements];
  //((CRollBuffer*)aI)->m_pData = new short[((CRollBuffer*)aI)->m_nWindowElements + ((CRollBuffer*)aI)->m_nHistoryElements];
  //((CRollBuffer*)aI)->m_pData =(short*)&Ape_gDeltaMCreateMalloc[filterflag][xyflag][0];//malloc(sizeof(short)*(((CRollBuffer*)aI)->m_nWindowElements + ((CRollBuffer*)aI)->m_nHistoryElements));
  ((CRollBuffer*)aI)->m_pData =(ape_int32*)&Ape_gDeltaMCreateMalloc[filterflag][xyflag][0];//malloc(sizeof(short)*(((CRollBuffer*)aI)->m_nWindowElements + ((CRollBuffer*)aI)->m_nHistoryElements));
  if (((CRollBuffer*)aI)->m_pData == NULL)
  {
    return ERROR_INSUFFICIENT_MEMORY;
  }
  ApeRollBufferFlush(aI);
  return 0;
}


void ApeRollBufferFlush(void *aI)
{
  memset(((CRollBuffer*)aI)->m_pData,0, (((CRollBuffer*)aI)->m_nHistoryElements + 1) * sizeof(ape_int32));
  ((CRollBuffer*)aI)->m_pCurrent = &(((CRollBuffer*)aI)->m_pData[((CRollBuffer*)aI)->m_nHistoryElements]);
}

void ApeRollBufferRoll(void *aI)
{
  memcpy(&(((CRollBuffer*)aI)->m_pData[0]), &(((CRollBuffer*)aI)->m_pCurrent[-((CRollBuffer*)aI)->m_nHistoryElements]), ((CRollBuffer*)aI)->m_nHistoryElements * sizeof(ape_int32) );
  ((CRollBuffer*)aI)->m_pCurrent = &(((CRollBuffer*)aI)->m_pData[((CRollBuffer*)aI)->m_nHistoryElements]);
}

void ApeRollBufferIncrementSafe(void *aI)
{
  ((CRollBuffer*)aI)->m_pCurrent++;
  if (((CRollBuffer*)aI)->m_pCurrent 
  	 == &(((CRollBuffer*)aI)->m_pData[((CRollBuffer*)aI)->m_nWindowElements + ((CRollBuffer*)aI)->m_nHistoryElements]))
  {
    ApeRollBufferRoll(aI);
  }
}

void ApeRollBufferIncrementFast(void *aI)
{
  ((CRollBuffer*)aI)->m_pCurrent++;
}

void ApeRollBufferFastPredictionAInitialize(void *aI,ape_int16 flag)
{	
  ((CRollBufferFast*)aI)->m_pData = (ape_int32*)&Ape_gPredABufferFastMalloc[flag][0];//malloc(sizeof(ape_int32)*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  ApeRollBufferFastFlush(aI);
}

void ApeRollBufferFastPredictionBInitialize(void *aI,ape_int16 flag)
{	
  ((CRollBufferFast*)aI)->m_pData = (ape_int32*)&Ape_gPredBBufferFastMalloc[flag][0];//malloc(sizeof(ape_int32)*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  ApeRollBufferFastFlush(aI);
}

void ApeRollBufferFastAdaptAInitialize(void *aI,ape_int16 flag)
{	
  ((CRollBufferFast*)aI)->m_pData = (ape_int32*)&Ape_gAdaptABufferFastMalloc[flag][0];//malloc(sizeof(ape_int32)*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  ApeRollBufferFastFlush(aI);
}

void ApeRollBufferFastAdaptBInitialize(void *aI,ape_int16 flag)
{	
  ((CRollBufferFast*)aI)->m_pData = (ape_int32*)&Ape_gAdaptBBufferFastMalloc[flag][0];//malloc(sizeof(ape_int32)*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  ApeRollBufferFastFlush(aI);
}

#if 0
void ApeRollBufferFastDelete(void *aI)
{
	//SAFE_ARRAY_DELETE(((CRollBufferFast*)aI)->m_pData);
}
#endif

void ApeRollBufferFastFlush(void *aI)
{
  //ZeroMemory(m_pData, (HISTORY_ELEMENTS + 1) * sizeof(TYPE));
  //ZeroMemory(((CRollBufferFast*)aI)->m_pData, (HISTORY_ELEMENTS + 1) * sizeof(ape_int32));
  //memset(((CRollBufferFast*)aI)->m_pData, 0, (HISTORY_ELEMENTS + 1) * sizeof(ape_int32)/2);
  memset(((CRollBufferFast*)aI)->m_pData, 0, (HISTORY_ELEMENTS + 1) * sizeof(ape_int32));
  ((CRollBufferFast*)aI)->m_pCurrent = &(((CRollBufferFast*)aI)->m_pData[HISTORY_ELEMENTS]);
}

void ApeRollBufferFastRoll(void *aI)
{
  //memcpy(&m_pData[0], &m_pCurrent[-HISTORY_ELEMENTS], HISTORY_ELEMENTS * sizeof(TYPE));
  //!!!ÓÐ†–î}!!
  memcpy(&(((CRollBufferFast*)aI)->m_pData[0]), &(((CRollBufferFast*)aI)->m_pCurrent[-HISTORY_ELEMENTS]), HISTORY_ELEMENTS * sizeof(ape_int32));
  ((CRollBufferFast*)aI)->m_pCurrent = &(((CRollBufferFast*)aI)->m_pData[HISTORY_ELEMENTS]);
}

void ApeRollBufferFastIncrementSafe(void *aI)
{
  ((CRollBufferFast*)aI)->m_pCurrent++;
  if (((CRollBufferFast*)aI)->m_pCurrent == &(((CRollBufferFast*)aI)->m_pData[WINDOW_ELEMENTS + HISTORY_ELEMENTS]))
  {
    ApeRollBufferFastRoll(aI);
  }
}

void ApeRollBufferFastIncrementFast(void *aI)
{
  ((CRollBufferFast*)aI)->m_pCurrent++;
}

#pragma arm section code

#endif
#endif
