
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_globalfunc.h"
#include "ape_nnfilter.h"
#include "ape_globalvardeclaration.h"

void ApeCNNFilterInitialize(CNNFilter *aNNF,ape_int32 nOrder, ape_int32 nShift,
                            ape_int32 nVersion,ape_int16 xyflag,ape_int16 filterflag)
{
  //if ((nOrder <= 0) || ((nOrder % 16) != 0))
  //	  exit(1);
  
  
  aNNF->cCNNFilter=(void (*)(void *,ape_int32,ape_int32,ape_int32,ape_int16,ape_int16))ApeCNNFilterInitialize;
  //aNNF->dCNNFilter=(void (*)(void *))ApeCNNFilterDelete;
  aNNF->Decompress=(ape_int32 (*)(void *,ape_int32))ApeCNNFilterDecompress;
  aNNF->Flush=(void (*)(void *))ApeCNNFilterFlush;
  aNNF->GetSaturatedShortFromInt=(ape_int16 (*)(void *,ape_int32))ApeCNNFilterGetSaturateShortFromInt;
  aNNF->CalculateDotProductNoMMX=(ape_int32 (*)(void *,ape_int16 *,ape_int16 *,ape_int32))ApeCNNFilterCalculateDotProduct;
  aNNF->AdaptNoMMX=(void (*)(void *,ape_int16*,ape_int16*,ape_int32,ape_int32))ApeCNNFilterAdapt;
  
  aNNF->m_nOrder = nOrder;
  aNNF->m_nShift = nShift;
  aNNF->Ape_gVersion = nVersion;
  //aNNF->m_bMMXAvailable = GetMMXAvailable();
  
  //CRollBuffer m_rbInput;
   //CRollBuffer m_rbDeltaM;
  aNNF->m_rbInput.cCRollBuffer=ApeRollBufferInitialize; 
  aNNF->m_rbInput.Create=ApeRollBufferCreate_rbInput;
  //aNNF->m_rbInput.dCRollBuffer=ApeRollBufferDelete ;
  aNNF->m_rbInput.Flush=ApeRollBufferFlush ;
  aNNF->m_rbInput.IncrementFast=ApeRollBufferIncrementFast ;
  aNNF->m_rbInput.IncrementSafe=ApeRollBufferIncrementSafe ;
  aNNF->m_rbInput.Roll=ApeRollBufferRoll ;
  aNNF->m_rbInput.cCRollBuffer(&(aNNF->m_rbInput));
  
  aNNF->m_rbDeltaM.cCRollBuffer=ApeRollBufferInitialize; 
  aNNF->m_rbDeltaM.Create=ApeRollBufferCreate_rbDeltaM;
  //aNNF->m_rbDeltaM.dCRollBuffer=ApeRollBufferDelete ;
  aNNF->m_rbDeltaM.Flush=ApeRollBufferFlush ;
  aNNF->m_rbDeltaM.IncrementFast=ApeRollBufferIncrementFast ;
  aNNF->m_rbDeltaM.IncrementSafe=ApeRollBufferIncrementSafe ;
  aNNF->m_rbDeltaM.Roll=ApeRollBufferRoll ;
  aNNF->m_rbDeltaM.cCRollBuffer(&(aNNF->m_rbDeltaM));
    
  //aNNF->m_rbInput.Create(NN_WINDOW_ELEMENTS, aNNF->m_nOrder);
  aNNF->m_rbInput.Create(&(aNNF->m_rbInput),NN_WINDOW_ELEMENTS, aNNF->m_nOrder,xyflag,filterflag);
  //aNNF->m_rbDeltaM.Create(NN_WINDOW_ELEMENTS, aNNF->m_nOrder);
  aNNF->m_rbDeltaM.Create(&(aNNF->m_rbDeltaM),NN_WINDOW_ELEMENTS, aNNF->m_nOrder,xyflag,filterflag);
  aNNF->m_paryM =(ape_int32*)&Ape_gParyMMalloc[filterflag][xyflag][0];//malloc(sizeof(short)*(aNNF->m_nOrder));
}

#if 0
void ApeCNNFilterDelete(CNNFilter *aNNF)
{
    SAFE_ARRAY_DELETE(aNNF->m_paryM)
}
#endif

void ApeCNNFilterFlush(CNNFilter *aNNF)
{
  //memset(&aNNF->m_paryM[0], 0, aNNF->m_nOrder * sizeof(short)/2);
  memset(&aNNF->m_paryM[0], 0, aNNF->m_nOrder *sizeof(ape_int32) );
  aNNF->m_rbInput.Flush(&(aNNF->m_rbInput));
  aNNF->m_rbDeltaM.Flush(&(aNNF->m_rbDeltaM));
  aNNF->m_nRunningAverage = 0;
}

#if 1
ape_int32 ApeCNNFilterDecompress(CNNFilter *aNNF,ape_int32 nInput)
{    
  ape_int32  nDotProduct = 0;
  ape_int32  nOutput ;
  ape_int32  nOrderIndex;
  ape_int32* pInputCurrent;
  ape_int32* pDeltamCurrent;
  ape_int32* pParyMOne;
  ape_int32* pParyMTwo;
  ape_int32 lparym0,lparym1,lparym2;
  
  // figure a dot product
  //nDotProduct = ApeCNNFilterCalculateDotProduct(aNNF,&(aNNF->m_rbInput.m_pCurrent[-aNNF->m_nOrder]),
  //                                              &aNNF->m_paryM[0], aNNF->m_nOrder);		
  
  // adapt
  //ApeCNNFilterAdapt(aNNF,&aNNF->m_paryM[0], &(aNNF->m_rbDeltaM.m_pCurrent[-aNNF->m_nOrder]), nInput, aNNF->m_nOrder);
  pInputCurrent = &(aNNF->m_rbInput.m_pCurrent[-aNNF->m_nOrder]);
  pDeltamCurrent = &(aNNF->m_rbDeltaM.m_pCurrent[-aNNF->m_nOrder]);
  pParyMTwo = pParyMOne = &aNNF->m_paryM[0];
  
  nOrderIndex = (aNNF->m_nOrder >> 4);//将下面的运算展开,循环由aNNF->m_nOrder缩减为aNNF->m_nOrder>>1次

  ////以下使用循环展开 added by hxd 20070704  
  if (nInput == 0) 
  {
    do//for(; nOrderIndex!=0;nOrderIndex--)
    {
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;

    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;

    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 

    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	    	    	   	
      //EXPAND_16_TIMES(nDotProduct += (ape_int32)(*pInputCurrent++) * (ape_int32)(*pParyMOne++);)
      //nDotProduct += (ape_int32)(*pInputCurrent++) * (ape_int32)(*pParyMOne++);
    }while(--nOrderIndex != 0);
  }
  else if (nInput < 0) 
  {
    do//for(; nOrderIndex!=0;nOrderIndex--)
    {
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;

    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 

    	
    	lparym0 += (*pDeltamCurrent++);
    	lparym1 += (*pDeltamCurrent++);
    	lparym2 += (*pDeltamCurrent++);
    	nOutput += (*pDeltamCurrent++);

    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;

    	
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;

    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 += (*pDeltamCurrent++);
    	lparym1 += (*pDeltamCurrent++);
    	lparym2 += (*pDeltamCurrent++);
    	nOutput += (*pDeltamCurrent++);

    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;
    	
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 += (*pDeltamCurrent++);
    	lparym1 += (*pDeltamCurrent++);
    	lparym2 += (*pDeltamCurrent++);
    	nOutput += (*pDeltamCurrent++);
    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;

    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 += (*pDeltamCurrent++);
    	lparym1 += (*pDeltamCurrent++);
    	lparym2 += (*pDeltamCurrent++);
    	nOutput += (*pDeltamCurrent++);
    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;
    	    	    	
      //EXPAND_16_TIMES(nDotProduct += (ape_int32)(*pInputCurrent++) * (ape_int32)(*pParyMOne++);)
      //nDotProduct += (ape_int32)(*pInputCurrent++) * (ape_int32)(*pParyMOne++);
      
      //EXPAND_16_TIMES((*pParyMTwo++) += (*pDeltamCurrent++);)
      //(*pParyMTwo++) += (*pDeltamCurrent++);
    }while(--nOrderIndex != 0);
  }
  else
  {
    do//for(; nOrderIndex!=0;nOrderIndex--)
    {
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;

    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 

    	
    	lparym0 -= (*pDeltamCurrent++);
    	lparym1 -= (*pDeltamCurrent++);
    	lparym2 -= (*pDeltamCurrent++);
    	nOutput -= (*pDeltamCurrent++);

    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;

    	
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;

    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 -= (*pDeltamCurrent++);
    	lparym1 -= (*pDeltamCurrent++);
    	lparym2 -= (*pDeltamCurrent++);
    	nOutput -= (*pDeltamCurrent++);

    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;
    	
    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 -= (*pDeltamCurrent++);
    	lparym1 -= (*pDeltamCurrent++);
    	lparym2 -= (*pDeltamCurrent++);
    	nOutput -= (*pDeltamCurrent++);
    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;

    	lparym0 = *pParyMOne++;
    	lparym1 = *pParyMOne++;
    	lparym2 = *pParyMOne++;
    	nOutput = *pParyMOne++;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym0;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym1; 
    	nDotProduct += (ape_int32)(*pInputCurrent++) * lparym2;
    	nDotProduct += (ape_int32)(*pInputCurrent++) * nOutput; 
    	
    	lparym0 -= (*pDeltamCurrent++);
    	lparym1 -= (*pDeltamCurrent++);
    	lparym2 -= (*pDeltamCurrent++);
    	nOutput -= (*pDeltamCurrent++);
    	
    	*pParyMTwo++ = lparym0;
    	*pParyMTwo++ = lparym1;
    	*pParyMTwo++ = lparym2;
    	*pParyMTwo++ = nOutput;
    	    	    	    	
      //EXPAND_16_TIMES(nDotProduct += (ape_int32)(*pInputCurrent++) * (ape_int32)(*pParyMOne++);)
      //nDotProduct += (ape_int32)(*pInputCurrent++) * (ape_int32)(*pParyMOne++);
      
      //EXPAND_16_TIMES((*pParyMTwo++) -= (*pDeltamCurrent++);)
      //(*pParyMTwo++) -= (*pDeltamCurrent++);
    }while(--nOrderIndex != 0);
  }

  // store the output value
  nOutput = nInput + ((nDotProduct + ((ape_int32)1 << (aNNF->m_nShift - 1))) >> aNNF->m_nShift);
  
  pInputCurrent = aNNF->m_rbInput.m_pCurrent;
  lparym1 = abs(nOutput);
  pDeltamCurrent = aNNF->m_rbDeltaM.m_pCurrent;
  
  // update the input buffer
  //aNNF->m_rbInput.m_pCurrent[0] = ApeCNNFilterGetSaturateShortFromInt(aNNF,nOutput);
  pInputCurrent[0] = (ape_int32)((nOutput == ((ape_int16)(nOutput))) ?
                                  (nOutput) : ((nOutput >> (ape_int32)31) ^ 0x7FFF));
  lparym2 = 3 * lparym1;  
  if (aNNF->Ape_gVersion >= 3980)
  {
    //ape_int32  nTempABS;
    //ape_int32  nTempABSbak;  	
    
    //nTempABSbak = abs(nOutput);
    //nTempABS = nTempABSbak * 3;
    lparym0 = aNNF->m_nRunningAverage;
    
    if (lparym2 > ((lparym0 << 3) + lparym0))
    {
      pDeltamCurrent[0] = ((nOutput >> (ape_int32)25) & 64) - 32;
    }
    else if (lparym2 > (lparym0 << 2))
         {
           pDeltamCurrent[0] = ((nOutput >> (ape_int32)26) & 32) - 16;
         }
         else if (lparym2 > 0)      
         {
           pDeltamCurrent[0] = ((nOutput >> (ape_int32)27) & 16) - 8;          
         }
              else
              {
                pDeltamCurrent[0] = 0;
              }

    lparym0 += ((lparym1 - lparym0) / 16); 
    aNNF->m_nRunningAverage = lparym0; 
    pDeltamCurrent[-1] >>= 1;
    pDeltamCurrent[-2] >>= 1;
    pDeltamCurrent[-8] >>= 1;
  }
  else
  {
    pDeltamCurrent[0] = (nOutput == 0) ? 0 : ((nOutput >> (ape_int32)28) & 8) - 4; 
    pDeltamCurrent[-4] >>= 1;
    pDeltamCurrent[-8] >>= 1;
  }

  // increment and roll if necessary
  //aNNF->m_rbInput.IncrementSafe(&(aNNF->m_rbInput));
  //aNNF->m_rbDeltaM.IncrementSafe(&(aNNF->m_rbDeltaM));
  //the upper 2 functions is expanded.
  pInputCurrent++;
  //aNNF->m_rbInput.m_pCurrent++;
  lparym0 = aNNF->m_rbInput.m_nWindowElements;
  aNNF->m_rbInput.m_pCurrent = pInputCurrent;
  lparym1 = aNNF->m_rbInput.m_nHistoryElements;
  pDeltamCurrent++;//aNNF->m_rbDeltaM.m_pCurrent++;  
  aNNF->m_rbDeltaM.m_pCurrent = pDeltamCurrent;
  
  if (pInputCurrent/*aNNF->m_rbInput.m_pCurrent*/ == &(aNNF->m_rbInput.m_pData[lparym0 + lparym1]))	 	
  {
    ApeRollBufferRoll(&(aNNF->m_rbInput));
    ApeRollBufferRoll(&(aNNF->m_rbDeltaM));
  }  
  
  return nOutput;
}
#endif

void ApeCNNFilterAdapt(CNNFilter *aNNF,ape_int16 * pM, ape_int16 * pAdapt, ape_int32 nDirection, ape_int32 nOrder)
{
  nOrder >>= 4;
  
  if (nDirection < 0) 
  {    
    while (nOrder--)
    {
        EXPAND_16_TIMES(*pM++ += *pAdapt++;)
    }
  }
  else if (nDirection > 0)
  {
    while (nOrder--)
    {
        EXPAND_16_TIMES(*pM++ -= *pAdapt++;)
    }
  }
}

ape_int32 ApeCNNFilterCalculateDotProduct(CNNFilter *aNNF,ape_int16 * pA, ape_int16 * pB, ape_int32 nOrder)
{
  ape_int32 nDotProduct = 0;
  nOrder >>= 4;
  
  while (nOrder--)
  {
      EXPAND_16_TIMES(nDotProduct += (ape_int32)(*pA++) * (ape_int32)(*pB++);)
  }
  
  return nDotProduct;
}

ape_int16 ApeCNNFilterGetSaturateShortFromInt(CNNFilter *aNNF,ape_int32 nValue)
{
  return (short)((nValue == ((short)(nValue))) ? (nValue) : ((nValue >> (ape_int32)31) ^ 0x7FFF));
}

#pragma arm section code

#endif
#endif
