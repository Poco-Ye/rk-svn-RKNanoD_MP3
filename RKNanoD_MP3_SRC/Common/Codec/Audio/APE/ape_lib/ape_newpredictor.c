
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
//#include "APECompress.h"
#include "ape_newpredictor.h"
#include "ape_globalvardeclaration.h"

/*****************************************************************************************
CPredictorDecompressNormal3930to3950
*****************************************************************************************/
ape_int32 ApeDecompressPredictor3930To3950(CPredictorDecompressNormal3930to3950 *aI,
                                      ape_int32 nCompressionLevel,
                                      ape_int32 nVersion,ape_int16 flag)     
{
  ape_int32 nRetVal = ERROR_SUCCESS;
  aI->m_pBuffer[0] = (ape_int32*)&Ape_gAdaptABufferFastMalloc[flag][0];
  
  if (nCompressionLevel == COMPRESSION_LEVEL_FAST)
  {
    aI->m_pNNFilter = NULL;
    aI->m_pNNFilter1 = NULL;
  }
  else if (nCompressionLevel == COMPRESSION_LEVEL_NORMAL)
  {
    aI->m_pNNFilter =(CNNFilter*) &Ape_gCNNFilterMalloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(aI->m_pNNFilter,16,11,nVersion,flag,0);
    aI->m_pNNFilter1 = NULL;
  }
  else if (nCompressionLevel == COMPRESSION_LEVEL_HIGH)
  {
    aI->m_pNNFilter =(CNNFilter*) &Ape_gCNNFilterMalloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(aI->m_pNNFilter,64,11,nVersion,flag,0);
    aI->m_pNNFilter1 = NULL;
  }
  else if (nCompressionLevel == COMPRESSION_LEVEL_EXTRA_HIGH)
  {
    aI->m_pNNFilter =(CNNFilter*) &Ape_gCNNFilterMalloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(aI->m_pNNFilter,256,13,nVersion,flag,0);
    aI->m_pNNFilter1 =(CNNFilter*) &Ape_gCNNFilter1Malloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(aI->m_pNNFilter1,32,10,nVersion,flag,1);
  }
  else
  {
    nRetVal = ERROR_DECOMPRESSING_FRAME;//exit(1);
  }
  return nRetVal;
}

#if 0
void ApePredictorDecompress3930To3950Delete(CPredictorDecompressNormal3930to3950 *aI)
{
  SAFE_DELETE(aI->m_pNNFilter)
  SAFE_DELETE(aI->m_pNNFilter1)
  SAFE_ARRAY_DELETE(aI->m_pBuffer[0])
}
#endif
    
ape_int32 ApePredictorFlush3930To3950(CPredictorDecompressNormal3930to3950 *aI)
{
  if (aI->m_pNNFilter)
  {
    aI->m_pNNFilter->Flush(aI->m_pNNFilter);
  }
  if (aI->m_pNNFilter1)
  {
    aI->m_pNNFilter1->Flush(aI->m_pNNFilter);
  }
  //ZeroMemory(aI->m_pBuffer[0], (HISTORY_ELEMENTS + 1) * sizeof(ape_int32));    
  //ZeroMemory(&(aI->m_aryM[0]), M_COUNT * sizeof(ape_int32));
  
  memset(aI->m_pBuffer[0],0, (HISTORY_ELEMENTS + 1) * sizeof(ape_int32));    
  memset(&(aI->m_aryM[0]),0, M_COUNT * sizeof(ape_int32));
  
  
  aI->m_aryM[0] = 360;
  aI->m_aryM[1] = 317;
  aI->m_aryM[2] = -109;
  aI->m_aryM[3] = 98;
  
  aI->m_pInputBuffer = &(aI->m_pBuffer[0][HISTORY_ELEMENTS]);
  
  aI->m_nLastValue = 0;
  aI->m_nCurrentIndex = 0;
  
  return ERROR_SUCCESS;
}

ape_int32 ApePredictorDecompressValue3930To3950(CPredictorDecompressNormal3930to3950 *aI,
                                                ape_int32 nInput, ape_int32 a)
{
  ape_int32 p1;
  ape_int32 p2;
  ape_int32 p3;
  ape_int32 p4;
  ape_int32 nRetVal;
  
  if (aI->m_nCurrentIndex == WINDOW_BLOCKS)
  {
    // copy forward and adjust pointers
    memcpy(&(aI->m_pBuffer[0][0]), &(aI->m_pBuffer[0][WINDOW_BLOCKS]), HISTORY_ELEMENTS * sizeof(ape_int32));
    aI->m_pInputBuffer = &(aI->m_pBuffer[0][HISTORY_ELEMENTS]);
    aI->m_nCurrentIndex = 0;
  }
  
  // stage 2: NNFilter
  if (aI->m_pNNFilter1)
  {	
    nInput = aI->m_pNNFilter1->Decompress(aI->m_pNNFilter1,nInput);
  }
  
  if (aI->m_pNNFilter)
  {
    nInput = aI->m_pNNFilter->Decompress(aI->m_pNNFilter,nInput);
  }
  
  // stage 1: multiple predictors (order 2 and offset 1)
  
  p1 = aI->m_pInputBuffer[-1];
  p2 = aI->m_pInputBuffer[-1] - aI->m_pInputBuffer[-2];
  p3 = aI->m_pInputBuffer[-2] - aI->m_pInputBuffer[-3];
  p4 = aI->m_pInputBuffer[-3] - aI->m_pInputBuffer[-4];
  
  aI->m_pInputBuffer[0] = nInput + (((p1 * aI->m_aryM[0]) + (p2 * aI->m_aryM[1])
              + (p3 * aI->m_aryM[2]) + (p4 * aI->m_aryM[3])) >> 9);
  
  if (nInput > 0) 
  {
    aI->m_aryM[0] -= ((p1 >> 30) & 2) - 1;
    aI->m_aryM[1] -= ((p2 >> 30) & 2) - 1;
    aI->m_aryM[2] -= ((p3 >> 30) & 2) - 1;
    aI->m_aryM[3] -= ((p4 >> 30) & 2) - 1;
  }
  else if (nInput < 0) 
  {
    aI->m_aryM[0] += ((p1 >> 30) & 2) - 1;
    aI->m_aryM[1] += ((p2 >> 30) & 2) - 1;
    aI->m_aryM[2] += ((p3 >> 30) & 2) - 1;
    aI->m_aryM[3] += ((p4 >> 30) & 2) - 1;
  }
  
  nRetVal = aI->m_pInputBuffer[0] + ((aI->m_nLastValue * 31) >> 5);
  aI->m_nLastValue = nRetVal;
  
  aI->m_nCurrentIndex++;
  aI->m_pInputBuffer++;
  
  return nRetVal;
}

/*****************************************************************************************
CPredictorDecompress3950toCurrent
*****************************************************************************************/

ape_int32 ApePredictorDecompress3950ToCurrent(void *aI,ape_int32 nCompressionLevel,
                                              ape_int32 nVersion,ape_int16 flag) 
{	
  ape_int32 nRetVal = ERROR_SUCCESS;
  ((CPredictorDecompress3950toCurrent*)aI)->Ape_gVersion = nVersion;
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA.Compress = ApeCScaledFirstOrderFilterCompress;
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA.Decompress = ApeCScaledFirstOrderFilterDecompress;
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA.Flush = ApeCScaledFirstOrderFilterFlush;
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA.SetMS = ApeCScaledFirstOrderFilterSetMS;
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB.Compress = ApeCScaledFirstOrderFilterCompress;
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB.Decompress = ApeCScaledFirstOrderFilterDecompress;
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB.Flush = ApeCScaledFirstOrderFilterFlush;
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB.SetMS = ApeCScaledFirstOrderFilterSetMS;
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA.SetMS(&(((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA),31,5);
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB.SetMS(&(((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB),31,5);
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.cCRollBufferFast =ApeRollBufferFastPredictionAInitialize ;
  //((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.dCRollBufferFast =ApeRollBufferFastDelete ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.Flush =ApeRollBufferFastFlush ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.IncrementFast =ApeRollBufferFastIncrementFast ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.IncrementSafe =ApeRollBufferFastIncrementSafe ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.Roll = ApeRollBufferFastRoll; 
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.cCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA),flag);
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.cCRollBufferFast =ApeRollBufferFastPredictionBInitialize ;
  //((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.dCRollBufferFast =ApeRollBufferFastDelete ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.Flush =ApeRollBufferFastFlush ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.IncrementFast =ApeRollBufferFastIncrementFast ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.IncrementSafe =ApeRollBufferFastIncrementSafe ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.Roll = ApeRollBufferFastRoll; 
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.cCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB),flag);
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.cCRollBufferFast =ApeRollBufferFastAdaptAInitialize ;
  //((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.dCRollBufferFast =ApeRollBufferFastDelete ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.Flush =ApeRollBufferFastFlush ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.IncrementFast =ApeRollBufferFastIncrementFast ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.IncrementSafe =ApeRollBufferFastIncrementSafe ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.Roll = ApeRollBufferFastRoll; 
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.cCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA),flag);
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.cCRollBufferFast =ApeRollBufferFastAdaptBInitialize ;
  //((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.dCRollBufferFast =ApeRollBufferFastDelete ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.Flush =ApeRollBufferFastFlush ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.IncrementFast =ApeRollBufferFastIncrementFast ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.IncrementSafe =ApeRollBufferFastIncrementSafe ;
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.Roll = ApeRollBufferFastRoll; 
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.cCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB),flag);

  if (nCompressionLevel == COMPRESSION_LEVEL_FAST)
  {
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter = NULL;
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1 = NULL;
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2 = NULL;
  }
  else if (nCompressionLevel == COMPRESSION_LEVEL_NORMAL)
  {
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter =(CNNFilter*) &Ape_gCNNFilterMalloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter,16,11,nVersion,flag,0);
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1 = NULL;
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2 = NULL;
  }
  else if (nCompressionLevel == COMPRESSION_LEVEL_HIGH)
  {
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter =(CNNFilter*) &Ape_gCNNFilterMalloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter,64,11,nVersion,flag,0);
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1 = NULL;
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2 = NULL;
  }
  else if (nCompressionLevel == COMPRESSION_LEVEL_EXTRA_HIGH)
  {
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter =(CNNFilter*) &Ape_gCNNFilterMalloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter,256,13,nVersion,flag,0);
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1 =(CNNFilter*) &Ape_gCNNFilter1Malloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1,32,10,nVersion,flag,1);
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2 = NULL;
  }
  else if (nCompressionLevel == COMPRESSION_LEVEL_INSANE)
  {
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter =(CNNFilter*) &Ape_gCNNFilterMalloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter,1024 + 256, 15, MAC_VERSION_NUMBER,flag,0);
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1 =(CNNFilter*) &Ape_gCNNFilter1Malloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1,256, 13, MAC_VERSION_NUMBER,flag,1);
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2 =(CNNFilter*) &Ape_gCNNFilter2Malloc[flag];//malloc(sizeof( CNNFilter));
    ApeCNNFilterInitialize(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2,16, 11, MAC_VERSION_NUMBER,flag,2);
  }
  else
  {
    nRetVal = ERROR_DECOMPRESSING_FRAME;//exit(1);
  }
  return nRetVal;
}

#if 0
//CPredictorDecompress3950toCurrent::~CPredictorDecompress3950toCurrent()
void ApePredictorDecompress3950ToCurrentDelete(void *aI)
{
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.dCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA));
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.dCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB));
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.dCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA));
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.dCRollBufferFast(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB));
  
  SAFE_DELETE(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter)
  SAFE_DELETE(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1)
  SAFE_DELETE(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2)
}
#endif    
ape_int32 ApePredictorFlush3950ToCurrent(void *aI)
{	
  if (((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter) 
  {
    ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter->	Flush( ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter );
  }
  if (((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1)
  {	
  	 ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1->Flush(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter1);
  }
  if (((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2)
  {
  	 ((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2->Flush(((CPredictorDecompress3950toCurrent*)aI)->m_pNNFilter2);
  }
  
  //ZeroMemory(((CPredictorDecompress3950toCurrent*)aI)->m_aryMA, sizeof(((CPredictorDecompress3950toCurrent*)aI)->m_aryMA));
  //ZeroMemory(((CPredictorDecompress3950toCurrent*)aI)->m_aryMB, sizeof(((CPredictorDecompress3950toCurrent*)aI)->m_aryMB));
  memset(((CPredictorDecompress3950toCurrent*)aI)->m_aryMA,0, sizeof(((CPredictorDecompress3950toCurrent*)aI)->m_aryMA));
  memset(((CPredictorDecompress3950toCurrent*)aI)->m_aryMB,0, sizeof(((CPredictorDecompress3950toCurrent*)aI)->m_aryMB));
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA.Flush(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionA));
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB.Flush(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbPredictionB));
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA.Flush(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptA));
  ((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB.Flush(&(((CPredictorDecompress3950toCurrent*)aI)->m_rbAdaptB));
  
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_aryMA[0] = 360;
  ((CPredictorDecompress3950toCurrent*)aI)->m_aryMA[1] = 317;
  ((CPredictorDecompress3950toCurrent*)aI)->m_aryMA[2] = -109;
  ((CPredictorDecompress3950toCurrent*)aI)->m_aryMA[3] = 98;
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA.Flush(&(((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterA));
  ((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB.Flush(&(((CPredictorDecompress3950toCurrent*)aI)->m_Stage1FilterB));
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_nLastValueA = 0;
  
  ((CPredictorDecompress3950toCurrent*)aI)->m_nCurrentIndex = 0;
  
  return ERROR_SUCCESS;
}


ape_int32 ApePredictorDecompressValue3950ToCurrent(void *aI,ape_int32 nA, ape_int32 nB)
{
  ape_int32 nPredictionA;
  ape_int32 nPredictionB;
  
  ape_int32 nCurrentA;
  ape_int32 nRetVal;
  ape_int32 *lpPredACurrent;
  ape_int32 *lpPredBCurrent; 
  ape_int32 *lpAdaptACurrent;
  ape_int32 *lpAdaptBCurrent; 

  
  CPredictorDecompress3950toCurrent* tmpaI=(CPredictorDecompress3950toCurrent*)aI;
  lpPredACurrent = (&((tmpaI)->m_rbPredictionA.m_pCurrent[0]));
  lpPredBCurrent = (&((tmpaI)->m_rbPredictionB.m_pCurrent[0]));
	lpAdaptACurrent  =(&((tmpaI)->m_rbAdaptA.m_pCurrent[0]));
	lpAdaptBCurrent  =(&((tmpaI)->m_rbAdaptB.m_pCurrent[0]));
	    
  if (tmpaI->m_nCurrentIndex == WINDOW_BLOCKS)
  {
    // copy forward and adjust pointers
    (tmpaI)->m_rbPredictionA.Roll(&((tmpaI)->m_rbPredictionA));    
    (tmpaI)->m_rbPredictionB.Roll(&((tmpaI)->m_rbPredictionB));
    (tmpaI)->m_rbAdaptA.Roll(&((tmpaI)->m_rbAdaptA)); 
    (tmpaI)->m_rbAdaptB.Roll(&((tmpaI)->m_rbAdaptB));
    lpPredACurrent = (&((tmpaI)->m_rbPredictionA.m_pCurrent[0]));
    lpPredBCurrent = (&((tmpaI)->m_rbPredictionB.m_pCurrent[0])); 
	  lpAdaptACurrent  =(&((tmpaI)->m_rbAdaptA.m_pCurrent[0]));
	  lpAdaptBCurrent  =(&((tmpaI)->m_rbAdaptB.m_pCurrent[0]));       
    (tmpaI)->m_nCurrentIndex = 0;
  }
  
  // stage 2: NNFilter
  if ((tmpaI)->m_pNNFilter2)
  {	
    nA = (tmpaI)->m_pNNFilter2->Decompress((tmpaI)->m_pNNFilter2,nA);
  } 
  if ((tmpaI)->m_pNNFilter1)
  {
    nA = (tmpaI)->m_pNNFilter1->Decompress((tmpaI)->m_pNNFilter1,nA);
  }
  if ((tmpaI)->m_pNNFilter)
  {
    nA = (tmpaI)->m_pNNFilter->Decompress((tmpaI)->m_pNNFilter,nA);
  }
  
  // stage 1: multiple predictors (order 2 and offset 1)
  lpPredACurrent[0]  = (tmpaI)->m_nLastValueA;
  lpPredACurrent[-1] = lpPredACurrent[0] - lpPredACurrent[-1];
  
  //(tmpaI)->m_rbPredictionB.m_pCurrent[0]  = (tmpaI)->m_Stage1FilterB.Compress(&((tmpaI)->m_Stage1FilterB),nB);
  lpPredBCurrent[0]  = nB 
       - ((((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterB))->m_nLastValue * 31) >>  5);
  	                           
  ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterB))->m_nLastValue = nB;
  
  lpPredBCurrent[-1] = lpPredBCurrent[0] - lpPredBCurrent[-1];	
  
  
  nPredictionA = (ape_int32)(lpPredACurrent[0] * (ape_int32)(tmpaI)->m_aryMA[0])
                 + (ape_int32)(lpPredACurrent[-1] * (ape_int32)(tmpaI)->m_aryMA[1])
                 + (ape_int32)(lpPredACurrent[-2] * (ape_int32)(tmpaI)->m_aryMA[2])
                 + (ape_int32)(lpPredACurrent[-3] * (ape_int32)(tmpaI)->m_aryMA[3]);
  nPredictionB = (ape_int32)(lpPredBCurrent[0] * (ape_int32)(tmpaI)->m_aryMB[0])
                 + (ape_int32)(lpPredBCurrent[-1] * (ape_int32)(tmpaI)->m_aryMB[1])
                 + (ape_int32)(lpPredBCurrent[-2] * (ape_int32)(tmpaI)->m_aryMB[2])
                 + (ape_int32)(lpPredBCurrent[-3] * (ape_int32)(tmpaI)->m_aryMB[3])
                 + (ape_int32)(lpPredBCurrent[-4] * (ape_int32)(tmpaI)->m_aryMB[4]);
  
  nCurrentA = nA + ((nPredictionA + (nPredictionB >> (ape_int32)1)) >> (ape_int32)10);
  
  lpAdaptACurrent[0] =  (lpPredACurrent[0]) ?  
                                       (((ape_int32)lpPredACurrent[0] >> (ape_int32)30) & 2) - 1 : 0;
  lpAdaptACurrent[-1] = (lpPredACurrent[-1]) ? 
                                       (((ape_int32)lpPredACurrent[-1] >> (ape_int32)30) & 2) - 1 : 0;
  
  lpAdaptBCurrent[0] =  (lpPredBCurrent[0]) ?  
                                       (((ape_int32)lpPredBCurrent[0] >> (ape_int32)30) & 2) - 1 : 0;
  lpAdaptBCurrent[-1] = (lpPredBCurrent[-1]) ? 
                                       (((ape_int32)lpPredBCurrent[-1] >> (ape_int32)30) & 2) - 1 : 0;
  
  if (nA > 0) 
  {
    (tmpaI)->m_aryMA[0] -= lpAdaptACurrent[0];
    (tmpaI)->m_aryMA[1] -= lpAdaptACurrent[-1];
    (tmpaI)->m_aryMA[2] -= lpAdaptACurrent[-2];
    (tmpaI)->m_aryMA[3] -= lpAdaptACurrent[-3];
    
    (tmpaI)->m_aryMB[0] -= lpAdaptBCurrent[0];
    (tmpaI)->m_aryMB[1] -= lpAdaptBCurrent[-1];
    (tmpaI)->m_aryMB[2] -= lpAdaptBCurrent[-2];
    (tmpaI)->m_aryMB[3] -= lpAdaptBCurrent[-3];
    (tmpaI)->m_aryMB[4] -= lpAdaptBCurrent[-4];
  }
  else if (nA < 0) 
  {				
    (tmpaI)->m_aryMA[0] += lpAdaptACurrent[0];
    (tmpaI)->m_aryMA[1] += lpAdaptACurrent[-1];
    (tmpaI)->m_aryMA[2] += lpAdaptACurrent[-2];
    (tmpaI)->m_aryMA[3] += lpAdaptACurrent[-3];
    
    (tmpaI)->m_aryMB[0] += lpAdaptBCurrent[0];
    (tmpaI)->m_aryMB[1] += lpAdaptBCurrent[-1];
    (tmpaI)->m_aryMB[2] += lpAdaptBCurrent[-2];
    (tmpaI)->m_aryMB[3] += lpAdaptBCurrent[-3];
    (tmpaI)->m_aryMB[4] += lpAdaptBCurrent[-4];
  }
  
  //nRetVal = (tmpaI)->m_Stage1FilterA.Decompress(&((tmpaI)->m_Stage1FilterA),nCurrentA);
  //(tmpaI)->m_Stage1FilterA.Decompress() is expanded.
  ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterA))->m_nLastValue = nCurrentA 
    + ((((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterA))->m_nLastValue * 31) >> 5);
  nRetVal = ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterA))->m_nLastValue;
  
  (tmpaI)->m_nLastValueA = nCurrentA;
  /*
  (tmpaI)->m_rbPredictionA.IncrementFast(&((tmpaI)->m_rbPredictionA)); 
  (tmpaI)->m_rbPredictionB.IncrementFast(&((tmpaI)->m_rbPredictionB));
  (tmpaI)->m_rbAdaptA.IncrementFast(&((tmpaI)->m_rbAdaptA)); 
  (tmpaI)->m_rbAdaptB.IncrementFast(&((tmpaI)->m_rbAdaptB));
  */
  //upper 4 functions is expanded.
  ((CRollBufferFast*)&((tmpaI)->m_rbPredictionA))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbPredictionB))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbAdaptA))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbAdaptB))->m_pCurrent++;
  
  (tmpaI)->m_nCurrentIndex++;
  
  return nRetVal;
}


ape_int32  ApePredictorDecompressValue3950ToCurrentMergeXAndY(ape_int32 nY,ape_int32 llastxvalue,ape_int32 nX)
{
  ape_int32 nPredictionA;
  ape_int32 nPredictionB;
  
  ape_int32 nCurrentA;
  //ape_int32 nRetVal;
  ape_int32 *lpPredACurrent;
  ape_int32 *lpPredBCurrent; 
  ape_int32 *lpAdaptACurrent;
  ape_int32 *lpAdaptBCurrent; 
  ape_int32 lppredacurrent0;
  ape_int32 lppredacurrentn1;
  ape_int32 lppredbcurrent0;
  ape_int32 lppredbcurrentn1;
  
  CPredictorDecompress3950toCurrent* tmpaI=(CPredictorDecompress3950toCurrent*)Ape_pNewPredictorY;
  //lpPredACurrent = (&((tmpaI)->m_rbPredictionA.m_pCurrent[0]));
  //lpPredBCurrent = (&((tmpaI)->m_rbPredictionB.m_pCurrent[0]));
	//lpAdaptACurrent  =(&((tmpaI)->m_rbAdaptA.m_pCurrent[0]));
	//lpAdaptBCurrent  =(&((tmpaI)->m_rbAdaptB.m_pCurrent[0]));
	    
  if (tmpaI->m_nCurrentIndex == WINDOW_BLOCKS)
  {
    // copy forward and adjust pointers
    (tmpaI)->m_rbPredictionA.Roll(&((tmpaI)->m_rbPredictionA));    
    (tmpaI)->m_rbPredictionB.Roll(&((tmpaI)->m_rbPredictionB));
    (tmpaI)->m_rbAdaptA.Roll(&((tmpaI)->m_rbAdaptA)); 
    (tmpaI)->m_rbAdaptB.Roll(&((tmpaI)->m_rbAdaptB));
    //lpPredACurrent = (&((tmpaI)->m_rbPredictionA.m_pCurrent[0]));
    //lpPredBCurrent = (&((tmpaI)->m_rbPredictionB.m_pCurrent[0])); 
	  //lpAdaptACurrent  =(&((tmpaI)->m_rbAdaptA.m_pCurrent[0]));
	  //lpAdaptBCurrent  =(&((tmpaI)->m_rbAdaptB.m_pCurrent[0]));       
    (tmpaI)->m_nCurrentIndex = 0;
  }

	    
  // stage 2: NNFilter
  //if ((tmpaI)->m_pNNFilter2)
  //{	
  //  nY = (tmpaI)->m_pNNFilter2->Decompress((tmpaI)->m_pNNFilter2,nY);
  //} 
  //if ((tmpaI)->m_pNNFilter1)
  //{
  //  nY = (tmpaI)->m_pNNFilter1->Decompress((tmpaI)->m_pNNFilter1,nY);
  //}
  if ((tmpaI)->m_pNNFilter)
  {
    nY = (tmpaI)->m_pNNFilter->Decompress((tmpaI)->m_pNNFilter,nY);
  }
  
    lpPredACurrent = (&((tmpaI)->m_rbPredictionA.m_pCurrent[0]));
    lpPredBCurrent = (&((tmpaI)->m_rbPredictionB.m_pCurrent[0])); 

	    
  // stage 1: multiple predictors (order 2 and offset 1)
  lppredacurrent0 = lpPredACurrent[0]  = (tmpaI)->m_nLastValueA;
  lppredacurrentn1 = lpPredACurrent[-1] = lppredacurrent0 - lpPredACurrent[-1];
  

  {
  ape_int32* lmarymamb;
  lmarymamb = &(tmpaI)->m_aryMA[0];
  nPredictionA = (ape_int32)(lppredacurrent0 * lmarymamb[0])
                 + (ape_int32)(lppredacurrentn1 * lmarymamb[1])
                 + (ape_int32)(lpPredACurrent[-2] * lmarymamb[2])
                 + (ape_int32)(lpPredACurrent[-3] * lmarymamb[3]);
                 
   }              
   //(tmpaI)->m_rbPredictionB.m_pCurrent[0]  = (tmpaI)->m_Stage1FilterB.Compress(&((tmpaI)->m_Stage1FilterB),llastxvalue);
  lppredbcurrent0 = lpPredBCurrent[0]  = llastxvalue 
       - ((((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterB))->m_nLastValue * 31) >>  5);
  	                           
  ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterB))->m_nLastValue = llastxvalue;
  
  lppredbcurrentn1 = lpPredBCurrent[-1] = lppredbcurrent0 - lpPredBCurrent[-1];	
         
  {
  ape_int32* lmarymamb; 
  lmarymamb = &(tmpaI)->m_aryMB[0];             
  nPredictionB = (ape_int32)(lppredbcurrent0 * lmarymamb[0])
                 + (ape_int32)(lppredbcurrentn1 * lmarymamb[1])
                 + (ape_int32)(lpPredBCurrent[-2] * lmarymamb[2])
                 + (ape_int32)(lpPredBCurrent[-3] * lmarymamb[3])
                 + (ape_int32)(lpPredBCurrent[-4] * lmarymamb[4]);
  
  }
  nCurrentA = nY + ((nPredictionA + (nPredictionB >> (ape_int32)1)) >> (ape_int32)10);


	  lpAdaptACurrent  =(&((tmpaI)->m_rbAdaptA.m_pCurrent[0]));
	  lpAdaptBCurrent  =(&((tmpaI)->m_rbAdaptB.m_pCurrent[0])); 
	  
  {
  //ape_int32* lmarymamb; 	    
  lppredacurrent0 = lpAdaptACurrent[0] =  (lppredacurrent0) ?  
                                       ((lppredacurrent0 >> (ape_int32)30) & 2) - 1 : 0;
  lppredacurrentn1 = lpAdaptACurrent[-1] = (lppredacurrentn1) ? 
                                       ((lppredacurrentn1 >> (ape_int32)30) & 2) - 1 : 0;
  }
  
  {
  //ape_int32* lmarymamb;   
  lppredbcurrent0 = lpAdaptBCurrent[0] =  (lppredbcurrent0) ?  
                                       ((lppredbcurrent0 >> (ape_int32)30) & 2) - 1 : 0;
  lppredbcurrentn1 = lpAdaptBCurrent[-1] = (lppredbcurrentn1) ? 
                                       ((lppredbcurrentn1 >> (ape_int32)30) & 2) - 1 : 0;
  }
  
  if (nY > 0) 
  {
  	ape_int32 mamb0,mamb1,mamb2,mamb3,mamb4;
    mamb0 = (tmpaI)->m_aryMA[0];
    mamb1 = (tmpaI)->m_aryMA[1];
    mamb2 = (tmpaI)->m_aryMA[2];
    mamb3 = (tmpaI)->m_aryMA[3];
  	
    mamb0 -= lppredacurrent0;    
    mamb1 -= lppredacurrentn1;   
    mamb2 -= lpAdaptACurrent[-2];
    mamb3 -= lpAdaptACurrent[-3];
    (tmpaI)->m_aryMA[0] = mamb0;
    (tmpaI)->m_aryMA[1] = mamb1;
    (tmpaI)->m_aryMA[2] = mamb2;
    (tmpaI)->m_aryMA[3] = mamb3;
    //(tmpaI)->m_aryMA[0] -= lppredacurrent0;    
    //(tmpaI)->m_aryMA[1] -= lppredacurrentn1;   
    //(tmpaI)->m_aryMA[2] -= lpAdaptACurrent[-2];
    //(tmpaI)->m_aryMA[3] -= lpAdaptACurrent[-3];

    mamb0 = (tmpaI)->m_aryMB[0];
    mamb1 = (tmpaI)->m_aryMB[1];
    mamb2 = (tmpaI)->m_aryMB[2];
    mamb3 = (tmpaI)->m_aryMB[3];
    mamb4 = (tmpaI)->m_aryMB[4];
       
    mamb0 -= lppredbcurrent0;    
    mamb1 -= lppredbcurrentn1;   
    mamb2 -= lpAdaptBCurrent[-2];
    mamb3 -= lpAdaptBCurrent[-3];
    mamb4 -= lpAdaptBCurrent[-4];
    (tmpaI)->m_aryMB[0] = mamb0;
    (tmpaI)->m_aryMB[1] = mamb1;
    (tmpaI)->m_aryMB[2] = mamb2;
    (tmpaI)->m_aryMB[3] = mamb3;
    (tmpaI)->m_aryMB[4] = mamb4;
    //(tmpaI)->m_aryMB[0] -= lppredbcurrent0;     
    //(tmpaI)->m_aryMB[1] -= lppredbcurrentn1;    
    //(tmpaI)->m_aryMB[2] -= lpAdaptBCurrent[-2]; 
    //(tmpaI)->m_aryMB[3] -= lpAdaptBCurrent[-3]; 
    //(tmpaI)->m_aryMB[4] -= lpAdaptBCurrent[-4]; 
  }
  else if (nY < 0) 
  {				
  	ape_int32 mamb0,mamb1,mamb2,mamb3,mamb4;
    mamb0 = (tmpaI)->m_aryMA[0];
    mamb1 = (tmpaI)->m_aryMA[1];
    mamb2 = (tmpaI)->m_aryMA[2];
    mamb3 = (tmpaI)->m_aryMA[3];
      	  				
    mamb0 += lppredacurrent0;    
    mamb1 += lppredacurrentn1;   
    mamb2 += lpAdaptACurrent[-2];
    mamb3 += lpAdaptACurrent[-3];
    (tmpaI)->m_aryMA[0] = mamb0; 
    (tmpaI)->m_aryMA[1] = mamb1; 
    (tmpaI)->m_aryMA[2] = mamb2; 
    (tmpaI)->m_aryMA[3] = mamb3; 
    //(tmpaI)->m_aryMA[0] += lppredacurrent0;    
    //(tmpaI)->m_aryMA[1] += lppredacurrentn1;   
    //(tmpaI)->m_aryMA[2] += lpAdaptACurrent[-2];
    //(tmpaI)->m_aryMA[3] += lpAdaptACurrent[-3];


    mamb0 = (tmpaI)->m_aryMB[0];
    mamb1 = (tmpaI)->m_aryMB[1];
    mamb2 = (tmpaI)->m_aryMB[2];
    mamb3 = (tmpaI)->m_aryMB[3];
    mamb4 = (tmpaI)->m_aryMB[4];
                                     
    mamb0 += lppredbcurrent0;    
    mamb1 += lppredbcurrentn1;   
    mamb2 += lpAdaptBCurrent[-2];
    mamb3 += lpAdaptBCurrent[-3];
    mamb4 += lpAdaptBCurrent[-4];
    (tmpaI)->m_aryMB[0] = mamb0; 
    (tmpaI)->m_aryMB[1] = mamb1; 
    (tmpaI)->m_aryMB[2] = mamb2; 
    (tmpaI)->m_aryMB[3] = mamb3; 
    (tmpaI)->m_aryMB[4] = mamb4; 
    //(tmpaI)->m_aryMB[0] += lppredbcurrent0;    
    //(tmpaI)->m_aryMB[1] += lppredbcurrentn1;   
    //(tmpaI)->m_aryMB[2] += lpAdaptBCurrent[-2];
    //(tmpaI)->m_aryMB[3] += lpAdaptBCurrent[-3];
    //(tmpaI)->m_aryMB[4] += lpAdaptBCurrent[-4];
  }
  
  //nRetVal = (tmpaI)->m_Stage1FilterA.Decompress(&((tmpaI)->m_Stage1FilterA),nCurrentA);
  //(tmpaI)->m_Stage1FilterA.Decompress() is expanded.
  nPredictionA = ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterA))->m_nLastValue;
   nPredictionA = nCurrentA 
    + ((nPredictionA * 31) >> 5);

  /*
  (tmpaI)->m_rbPredictionA.IncrementFast(&((tmpaI)->m_rbPredictionA)); 
  (tmpaI)->m_rbPredictionB.IncrementFast(&((tmpaI)->m_rbPredictionB));
  (tmpaI)->m_rbAdaptA.IncrementFast(&((tmpaI)->m_rbAdaptA)); 
  (tmpaI)->m_rbAdaptB.IncrementFast(&((tmpaI)->m_rbAdaptB));
  */
  //upper 4 functions is expanded.
  ((CRollBufferFast*)&((tmpaI)->m_rbPredictionA))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbPredictionB))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbAdaptA))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbAdaptB))->m_pCurrent++;
  Ape_gYValue = llastxvalue = nPredictionA;
  
  (tmpaI)->m_nLastValueA = nCurrentA;
  ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterA))->m_nLastValue = nPredictionA;  
  (tmpaI)->m_nCurrentIndex++;
  
  //return nRetVal;	

/////////////////////nX Predictor////////////////////////	
  tmpaI=(CPredictorDecompress3950toCurrent*)Ape_pNewPredictorX;
  //lpPredACurrent = (&((tmpaI)->m_rbPredictionA.m_pCurrent[0]));
  //lpPredBCurrent = (&((tmpaI)->m_rbPredictionB.m_pCurrent[0]));
	//lpAdaptACurrent  =(&((tmpaI)->m_rbAdaptA.m_pCurrent[0]));
	//lpAdaptBCurrent  =(&((tmpaI)->m_rbAdaptB.m_pCurrent[0]));
	    
  if (tmpaI->m_nCurrentIndex == WINDOW_BLOCKS)
  {
    // copy forward and adjust pointers
    (tmpaI)->m_rbPredictionA.Roll(&((tmpaI)->m_rbPredictionA));    
    (tmpaI)->m_rbPredictionB.Roll(&((tmpaI)->m_rbPredictionB));
    (tmpaI)->m_rbAdaptA.Roll(&((tmpaI)->m_rbAdaptA)); 
    (tmpaI)->m_rbAdaptB.Roll(&((tmpaI)->m_rbAdaptB));
       
    (tmpaI)->m_nCurrentIndex = 0;
  }
    lpPredACurrent = (&((tmpaI)->m_rbPredictionA.m_pCurrent[0]));
    lpPredBCurrent = (&((tmpaI)->m_rbPredictionB.m_pCurrent[0])); 

  // stage 2: NNFilter
  //if ((tmpaI)->m_pNNFilter2)
  //{	
  //  nX = (tmpaI)->m_pNNFilter2->Decompress((tmpaI)->m_pNNFilter2,nX);
 // } 
  //if ((tmpaI)->m_pNNFilter1)
  //{
  //  nX = (tmpaI)->m_pNNFilter1->Decompress((tmpaI)->m_pNNFilter1,nX);
 // }
  if ((tmpaI)->m_pNNFilter)
  {
    nX = (tmpaI)->m_pNNFilter->Decompress((tmpaI)->m_pNNFilter,nX);
  }
  
  // stage 1: multiple predictors (order 2 and offset 1)
  lppredacurrent0 = lpPredACurrent[0]  = (tmpaI)->m_nLastValueA;
  lppredacurrentn1 = lpPredACurrent[-1] = lppredacurrent0 - lpPredACurrent[-1];

  {
  ape_int32* lmarymamb;
  lmarymamb = &(tmpaI)->m_aryMA[0];  
  nPredictionA = (ape_int32)(lppredacurrent0 * lmarymamb[0])
                 + (ape_int32)(lppredacurrentn1 * lmarymamb[1])
                 + (ape_int32)(lpPredACurrent[-2] * lmarymamb[2])
                 + (ape_int32)(lpPredACurrent[-3] * lmarymamb[3]);
   }
                 
  //(tmpaI)->m_rbPredictionB.m_pCurrent[0]  = (tmpaI)->m_Stage1FilterB.Compress(&((tmpaI)->m_Stage1FilterB),llastxvalue);
  lppredbcurrent0 = lpPredBCurrent[0]  = llastxvalue 
       - ((((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterB))->m_nLastValue * 31) >>  5);
  	                           
  ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterB))->m_nLastValue = llastxvalue;
  
  lppredbcurrentn1 = lpPredBCurrent[-1] = lppredbcurrent0 - lpPredBCurrent[-1];	

  {
  ape_int32* lmarymamb; 
  lmarymamb = &(tmpaI)->m_aryMB[0];                   
  nPredictionB = (ape_int32)(lppredbcurrent0 * lmarymamb[0])
                 + (ape_int32)(lppredbcurrentn1 * lmarymamb[1])
                 + (ape_int32)(lpPredBCurrent[-2] * lmarymamb[2])
                 + (ape_int32)(lpPredBCurrent[-3] * lmarymamb[3])
                 + (ape_int32)(lpPredBCurrent[-4] * lmarymamb[4]);
   }
  
  nCurrentA = nX + ((nPredictionA + (nPredictionB >> (ape_int32)1)) >> (ape_int32)10);

	  lpAdaptACurrent  =(&((tmpaI)->m_rbAdaptA.m_pCurrent[0]));
	  lpAdaptBCurrent  =(&((tmpaI)->m_rbAdaptB.m_pCurrent[0]));  
	    
  lppredacurrent0 = lpAdaptACurrent[0] =  (lppredacurrent0) ?  
                                       ((lppredacurrent0 >> (ape_int32)30) & 2) - 1 : 0;
  lppredacurrentn1 = lpAdaptACurrent[-1] = (lppredacurrentn1) ? 
                                       ((lppredacurrentn1 >> (ape_int32)30) & 2) - 1 : 0;
  
  lppredbcurrent0 = lpAdaptBCurrent[0] =  (lppredbcurrent0) ?  
                                       ((lppredbcurrent0 >> (ape_int32)30) & 2) - 1 : 0;
  lppredbcurrentn1 = lpAdaptBCurrent[-1] = (lppredbcurrentn1) ? 
                                       ((lppredbcurrentn1 >> (ape_int32)30) & 2) - 1 : 0;
  
  if (nX > 0) 
  { 
  	ape_int32 mamb0,mamb1,mamb2,mamb3,mamb4;
    mamb0 = (tmpaI)->m_aryMA[0];
    mamb1 = (tmpaI)->m_aryMA[1];
    mamb2 = (tmpaI)->m_aryMA[2];
    mamb3 = (tmpaI)->m_aryMA[3];
  	
    mamb0 -= lppredacurrent0;    
    mamb1 -= lppredacurrentn1;   
    mamb2 -= lpAdaptACurrent[-2];
    mamb3 -= lpAdaptACurrent[-3];
    (tmpaI)->m_aryMA[0] = mamb0;
    (tmpaI)->m_aryMA[1] = mamb1;
    (tmpaI)->m_aryMA[2] = mamb2;
    (tmpaI)->m_aryMA[3] = mamb3;
    //(tmpaI)->m_aryMA[0] -= lppredacurrent0;    
    //(tmpaI)->m_aryMA[1] -= lppredacurrentn1;   
    //(tmpaI)->m_aryMA[2] -= lpAdaptACurrent[-2];
    //(tmpaI)->m_aryMA[3] -= lpAdaptACurrent[-3];

    mamb0 = (tmpaI)->m_aryMB[0];
    mamb1 = (tmpaI)->m_aryMB[1];
    mamb2 = (tmpaI)->m_aryMB[2];
    mamb3 = (tmpaI)->m_aryMB[3];
    mamb4 = (tmpaI)->m_aryMB[4];
       
    mamb0 -= lppredbcurrent0;    
    mamb1 -= lppredbcurrentn1;   
    mamb2 -= lpAdaptBCurrent[-2];
    mamb3 -= lpAdaptBCurrent[-3];
    mamb4 -= lpAdaptBCurrent[-4];
    (tmpaI)->m_aryMB[0] = mamb0;
    (tmpaI)->m_aryMB[1] = mamb1;
    (tmpaI)->m_aryMB[2] = mamb2;
    (tmpaI)->m_aryMB[3] = mamb3;
    (tmpaI)->m_aryMB[4] = mamb4;
    //(tmpaI)->m_aryMB[0] -= lppredbcurrent0;     
    //(tmpaI)->m_aryMB[1] -= lppredbcurrentn1;    
    //(tmpaI)->m_aryMB[2] -= lpAdaptBCurrent[-2]; 
    //(tmpaI)->m_aryMB[3] -= lpAdaptBCurrent[-3]; 
    //(tmpaI)->m_aryMB[4] -= lpAdaptBCurrent[-4]; 
  } 
  else if (nX < 0) 
  {	
  	ape_int32 mamb0,mamb1,mamb2,mamb3,mamb4;
    mamb0 = (tmpaI)->m_aryMA[0];
    mamb1 = (tmpaI)->m_aryMA[1];
    mamb2 = (tmpaI)->m_aryMA[2];
    mamb3 = (tmpaI)->m_aryMA[3];
      	  				
    mamb0 += lppredacurrent0;    
    mamb1 += lppredacurrentn1;   
    mamb2 += lpAdaptACurrent[-2];
    mamb3 += lpAdaptACurrent[-3];
    (tmpaI)->m_aryMA[0] = mamb0; 
    (tmpaI)->m_aryMA[1] = mamb1; 
    (tmpaI)->m_aryMA[2] = mamb2; 
    (tmpaI)->m_aryMA[3] = mamb3; 
    //(tmpaI)->m_aryMA[0] += lppredacurrent0;    
    //(tmpaI)->m_aryMA[1] += lppredacurrentn1;   
    //(tmpaI)->m_aryMA[2] += lpAdaptACurrent[-2];
    //(tmpaI)->m_aryMA[3] += lpAdaptACurrent[-3];


    mamb0 = (tmpaI)->m_aryMB[0];
    mamb1 = (tmpaI)->m_aryMB[1];
    mamb2 = (tmpaI)->m_aryMB[2];
    mamb3 = (tmpaI)->m_aryMB[3];
    mamb4 = (tmpaI)->m_aryMB[4];
                                     
    mamb0 += lppredbcurrent0;    
    mamb1 += lppredbcurrentn1;   
    mamb2 += lpAdaptBCurrent[-2];
    mamb3 += lpAdaptBCurrent[-3];
    mamb4 += lpAdaptBCurrent[-4];
    (tmpaI)->m_aryMB[0] = mamb0; 
    (tmpaI)->m_aryMB[1] = mamb1; 
    (tmpaI)->m_aryMB[2] = mamb2; 
    (tmpaI)->m_aryMB[3] = mamb3; 
    (tmpaI)->m_aryMB[4] = mamb4; 
    //(tmpaI)->m_aryMB[0] += lppredbcurrent0;    
    //(tmpaI)->m_aryMB[1] += lppredbcurrentn1;   
    //(tmpaI)->m_aryMB[2] += lpAdaptBCurrent[-2];
    //(tmpaI)->m_aryMB[3] += lpAdaptBCurrent[-3];
    //(tmpaI)->m_aryMB[4] += lpAdaptBCurrent[-4];
  } 
    
  //nRetVal = (tmpaI)->m_Stage1FilterA.Decompress(&((tmpaI)->m_Stage1FilterA),nCurrentA);
  //(tmpaI)->m_Stage1FilterA.Decompress() is expanded.
  nPredictionA = ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterA))->m_nLastValue;
  nPredictionA = nCurrentA 
    + ((nPredictionA * 31) >> 5);
  //nRetVal = nPredictionA;
    
  /*
  (tmpaI)->m_rbPredictionA.IncrementFast(&((tmpaI)->m_rbPredictionA)); 
  (tmpaI)->m_rbPredictionB.IncrementFast(&((tmpaI)->m_rbPredictionB));
  (tmpaI)->m_rbAdaptA.IncrementFast(&((tmpaI)->m_rbAdaptA)); 
  (tmpaI)->m_rbAdaptB.IncrementFast(&((tmpaI)->m_rbAdaptB));
  */
  //upper 4 functions is expanded.
  ((CRollBufferFast*)&((tmpaI)->m_rbPredictionA))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbPredictionB))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbAdaptA))->m_pCurrent++;
  ((CRollBufferFast*)&((tmpaI)->m_rbAdaptB))->m_pCurrent++;
  ((CScaledFirstOrderFilter*)&((tmpaI)->m_Stage1FilterA))->m_nLastValue = nPredictionA;  
  (tmpaI)->m_nLastValueA = nCurrentA;  
  (tmpaI)->m_nCurrentIndex++;
  
  return nPredictionA;	
}

#pragma arm section code

#endif
#endif
