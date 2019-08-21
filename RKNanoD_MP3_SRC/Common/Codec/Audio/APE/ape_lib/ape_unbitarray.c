
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_Infodec.h"
#include "ape_unbitarray.h"
#include "ape_bitarray.h"
#include "ape_globalvardeclaration.h"

#define RANGE_OVERFLOW_TOTAL_WIDTH        65536
#define RANGE_OVERFLOW_SHIFT            16

#define CODE_BITS 32
#define TOP_VALUE ((ape_uint32 ) 1 << (CODE_BITS - 1))
#define SHIFT_BITS (CODE_BITS - 9)
#define EXTRA_BITS (((CODE_BITS - 2) & 7) + 1)//((CODE_BITS - 2) % 8 + 1)
#define BOTTOM_VALUE (TOP_VALUE >> 8)

#define MODEL_ELEMENTS 64

/***********************************************************************************
Construction
***********************************************************************************/
void ApeUnBitArrayInitialize(CIO * pIO, ape_int32 nVersion) 
{
  ApeUnBitArrayCreateHelper(pIO, (ape_int32)(BLOCKS_PER_BITSTREAMREAD + 16)*4/*4384*/, nVersion);//allocate space for inputbuffer,Ape_pBitArray,modified by hxd 20070316
  Ape_gFlushCounter = 0;
  Ape_gFinalizeCounter = 0;
}

#if 0
void ApeUnBitArrayDelete()
{
  SAFE_ARRAY_DELETE(Ape_pBitArray)
}
#endif

ape_uint32 ApeUnBitArrayDecodeValue(enum DECODE_VALUE_METHOD DecodeMethod, ape_int32 nParam1, ape_int32 nParam2)
{
  switch (DecodeMethod)
  {
  case DECODE_VALUE_METHOD_UNSIGNED_INT:
      return ApeUnBitArrayDecodeValueXBits(32);
  }
  
  return 0;
}

void ApeUnBitArrayGenerateArray(ape_int32 * pOutputArray, ape_int32 nElements, ape_int32 nBytesRequired) 
{
  ApeUnBitArrayGenerateArrayRange(pOutputArray, nElements);
}

ape_uchar ApeUnBitArrayGetC()
{
  ape_uchar nValue = (ape_uchar) (Ape_pBitArray[Ape_gCurrentBitIndex >> 5] >> (24 - (Ape_gCurrentBitIndex & 31)));
  Ape_gCurrentBitIndex += 8;
  return nValue;
}    

#if 0
__inline ape_int32 ApeUnBitArrayRangeDecodeFast(ape_int32 nShift)
{
  while (Ape_gRangeValue <= BOTTOM_VALUE)
  {   
    Ape_gBufferValue = (Ape_gBufferValue << 8)
                         | ((Ape_pBitArray[Ape_gCurrentBitIndex >> 5] >> (24 - (Ape_gCurrentBitIndex & 31))) & 0xFF);
    Ape_gCurrentBitIndex += 8;
    Ape_gLowValue = (Ape_gLowValue << 8) | ((Ape_gBufferValue >> 1) & 0xFF);
    Ape_gRangeValue <<= 8;
  }
  
  // decode
  Ape_gRangeValue = Ape_gRangeValue >> nShift;
  return Ape_gLowValue / Ape_gRangeValue;
}

__inline ape_int32 ApeUnBitArrayRangeDecodeFastWithUpdate(ape_int32 nShift)
{
  ape_int32 nRetVal ;
  while (Ape_gRangeValue <= BOTTOM_VALUE)
  {   
      Ape_gBufferValue = (Ape_gBufferValue << 8)
                  | ((Ape_pBitArray[Ape_gCurrentBitIndex >> 5] >> (24 - (Ape_gCurrentBitIndex & 31))) & 0xFF);
      Ape_gCurrentBitIndex += 8;
      Ape_gLowValue = (Ape_gLowValue << 8) | ((Ape_gBufferValue >> 1) & 0xFF);
      Ape_gRangeValue <<= 8;
  }
  
  // decode
  Ape_gRangeValue = Ape_gRangeValue >> nShift;
  nRetVal = Ape_gLowValue / Ape_gRangeValue;
  Ape_gLowValue = Ape_gLowValue % Ape_gRangeValue;//Ape_gLowValue -= Ape_gRangeValue * nRetVal;
  return nRetVal;
}
#endif

#if 0
extern ape_uint32 udiv_32by32_arm9e(ape_uint32,ape_uint32);
#else
#define udiv_32by32_arm9e(a, b) (b/a)
#endif

ape_int32 ApeUnBitArrayDecodeValueRange(struct UNBIT_ARRAY_STATE * BitArrayState)
{
  ape_int32 nValue = 0;
  ape_int32 nPivotValue;
  // get the overflow
  ape_int32 nOverflow = 0;
  ape_int32 nBase = 0;
  ape_int32 nPivotValueBits = 0;
  ape_int32 nPivotValueA ;
  ape_int32 nBaseB ;
  ape_int32 nBaseLower;
  //ape_int32 nTempK;
  ape_uint32 RangeLastValue;
  ape_uint32 RangeTempValue;
  ape_uint32 lrangeval = Ape_gRangeValue;
  ape_uint32 lbuffervalue = Ape_gBufferValue;
  ape_uint32 lcurrentbitindex = Ape_gCurrentBitIndex;
  ape_uint32 llowvalue = Ape_gLowValue;
  
  // make sure there is room for the data
  // this is a little slower than ensuring a huge block to start with, but it's safer
  //if (Ape_gCurrentBitIndex > Ape_gRefillBitThreshold)
  //{
  //  ApeUnBitArrayFillBitArray();
  //}
  
  if (Ape_gVersion >= 3990)
  {
    // figure the pivot value
    nPivotValue = max((BitArrayState->nKSum >> 5), 1);//max(BitArrayState->nKSum / 32, 1);
    
    
    {
        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
        if(lrangeval == 0)
        {
          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
          return 0;
        }
        
        // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
        while (lrangeval <= BOTTOM_VALUE)
        {   
          lbuffervalue = (lbuffervalue << 8)
                         | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
        }
  
        // decode
        lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
        
        // lookup the symbol (must be a faster way than this)
        nOverflow = 0;
        RangeLastValue = 0;
        while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalTwo[nOverflow + 1]))
        {
          RangeLastValue = RangeTempValue;
          nOverflow++;
        }
        
        // update
        llowvalue -= RangeLastValue;
        lrangeval = lrangeval * Ape_gtRangeWidthTwo[nOverflow];
        
        // get the working k
        if (nOverflow == (MODEL_ELEMENTS - 1))
        {
          //nOverflow = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
          while (lrangeval <= BOTTOM_VALUE)
          {   
              lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
              lcurrentbitindex += 8;
              llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
              lrangeval <<= 8;
          }
          
          // decode
          lrangeval = lrangeval >> 16;
          nOverflow = udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow = llowvalue / lrangeval;
          llowvalue -= lrangeval * nOverflow;          
          
          nOverflow <<= 16;
          //nOverflow |= ApeUnBitArrayRangeDecodeFastWithUpdate(16);
          while (lrangeval <= BOTTOM_VALUE)
          {   
              lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
              lcurrentbitindex += 8;
              llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
              lrangeval <<= 8;
          }
          
          // decode
          lrangeval = lrangeval >> 16;
          nOverflow |= udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow |= llowvalue / lrangeval;
          llowvalue -= lrangeval * nOverflow;          
                    
        }
    }
  
    // get the value
    {
        //ape_int32 nShift = 0;
        if (nPivotValue > 65535 /*>= ((ape_int32)1 << (ape_int32)16)*/)
        {
          nPivotValueBits = 0;
          while ((nPivotValue >> nPivotValueBits) > 0)
          {
            nPivotValueBits++;
          }

          nPivotValueBits -= 16;
          nPivotValueA = (nPivotValue >> (nPivotValueBits)) + 1;
          
          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
        
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          lrangeval = lrangeval / nPivotValueA;
          nBase = llowvalue / lrangeval;
          llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBase;

          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
                    
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          lrangeval = (lrangeval >> (nPivotValueBits));
          nBaseB = llowvalue / lrangeval;
          llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBaseB;
          
          nBase = (nBase << nPivotValueBits) + nBaseB;
        }
        else
        {
          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
                  
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                       | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          // decode
          lrangeval = udiv_32by32_arm9e((ape_uint32) nPivotValue,lrangeval);//lrangeval = lrangeval / nPivotValue;
          nBaseLower = udiv_32by32_arm9e(lrangeval,llowvalue);//nBaseLower = llowvalue / lrangeval;
          llowvalue -= lrangeval * nBaseLower;
          
          nBase = nBaseLower;
        }
    }
    
    // build the value
    nValue = nBase + (nOverflow * nPivotValue);
  }
  else//the following is ape version < 3990
  {
    //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
    if(lrangeval == 0)
    {
      Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
      return 0;
    }
            
    // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
    while (lrangeval <= BOTTOM_VALUE)
    {   
      lbuffervalue = (lbuffervalue << 8)
                     | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
      lcurrentbitindex += 8;
      llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
      lrangeval <<= 8;
    }
  
    // decode
    lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
    
    // lookup the symbol (must be a faster way than this)
    nOverflow = 0;
    RangeLastValue = 0;
    while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalOne[nOverflow + 1]))
    {
      RangeLastValue = RangeTempValue;
      nOverflow++;
    }
    
    // update
    llowvalue -= RangeLastValue;
    lrangeval = lrangeval * Ape_gtRangeWidthOne[nOverflow];
    
    // get the working k
    
    if (nOverflow == (MODEL_ELEMENTS - 1))
    {
      //RangeTempValue = ApeUnBitArrayRangeDecodeFastWithUpdate(5);
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> 5;
      RangeTempValue = llowvalue / lrangeval;
      llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nRetVal;  
      nOverflow = 0;
    }
    else
    {
      RangeTempValue = (BitArrayState->k < 1) ? (0) : (BitArrayState->k - 1);
    }
    
    // figure the extra bits on the left and the left value
    if (RangeTempValue <= 16 || Ape_gVersion < 3910)
    {
      //nValue = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue);
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> RangeTempValue;
      nValue = llowvalue / lrangeval;
      llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nRetVal;        
    }                    
    else
    {    
      //ape_int32 nX1 = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
      //ape_int32 nX2 = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue - 16);
      ape_int32 nX1;
      ape_int32 nX2;

      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> 16;
      nX1 = llowvalue / lrangeval;
      llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nRetVal; 
      
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> (RangeTempValue - 16);
      nX2 = llowvalue / lrangeval;
      llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nRetVal;             
      
      nValue = nX1 | (nX2 << 16);
    }
        
    // build the value and output it
    nValue += (nOverflow << RangeTempValue);
  }
  
  // update nKSum
  BitArrayState->nKSum += ((nValue + 1) >> 1) - ((BitArrayState->nKSum + 16) >> 5);
  
  // update k
  if (BitArrayState->nKSum < Ape_gtKSumMinBoundary[BitArrayState->k])
  {	 
    BitArrayState->k--;
  }
  else if (BitArrayState->nKSum >= Ape_gtKSumMinBoundary[BitArrayState->k + 1]) 
  	   {
         BitArrayState->k++;
       }
  Ape_gRangeValue = lrangeval;
  Ape_gBufferValue = lbuffervalue;
  Ape_gCurrentBitIndex = lcurrentbitindex;
  Ape_gLowValue = llowvalue;  
  
  // output the value (converted to signed)
  return (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));
}

ape_int32 ApeUnBitArrayDecodeValueRangeMergeXAndY(struct UNBIT_ARRAY_STATE * BitArrayStateY,struct UNBIT_ARRAY_STATE * BitArrayStateX)
{
  ape_int32 nValue = 0;
  ape_int32 nPivotValue;
  // get the overflow
  ape_int32 nOverflow = 0;
  ape_int32 nBase = 0;
  ape_int32 nPivotValueBits = 0;
  ape_int32 nPivotValueA ;
  ape_int32 nBaseB ;
  ape_int32 nBaseLower;
  //ape_int32 nTempK;
  ape_uint32 RangeLastValue;
  ape_uint32 RangeTempValue;
  ape_uint32 lrangeval = Ape_gRangeValue;
  ape_uint32 lbuffervalue = Ape_gBufferValue;
  ape_uint32 lcurrentbitindex = Ape_gCurrentBitIndex;
  ape_uint32 llowvalue = Ape_gLowValue;
  
  // make sure there is room for the data
  // this is a little slower than ensuring a huge block to start with, but it's safer
  //if (Ape_gCurrentBitIndex > Ape_gRefillBitThreshold)
  //{
  //  ApeUnBitArrayFillBitArray();
  //}
  
  if (Ape_gVersion >= 3990)
  {
    // figure the pivot value
    nPivotValue = max((BitArrayStateY->nKSum >> 5), 1);//max(BitArrayStateY->nKSum / 32, 1);
    
    
    {
        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
        if(lrangeval == 0)
        {
          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
          return 0;
        }
        
        // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
        while (lrangeval <= BOTTOM_VALUE)
        {   
          lbuffervalue = (lbuffervalue << 8)
                         | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
        }
  
        // decode
        lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
        
        // lookup the symbol (must be a faster way than this)
        nOverflow = 0;
        RangeLastValue = 0;
        while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalTwo[nOverflow + 1]))
        {
          RangeLastValue = RangeTempValue;
          nOverflow++;
        }
        
        // update
        llowvalue -= RangeLastValue;
        lrangeval = lrangeval * Ape_gtRangeWidthTwo[nOverflow];
        
        // get the working k
        if (nOverflow == (MODEL_ELEMENTS - 1))
        {
          //nOverflow = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
          while (lrangeval <= BOTTOM_VALUE)
          {   
              lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
              lcurrentbitindex += 8;
              llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
              lrangeval <<= 8;
          }
          
          // decode
          lrangeval = lrangeval >> 16;
          nOverflow = udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow = llowvalue / lrangeval;
          llowvalue -= lrangeval * nOverflow;          
          
          nOverflow <<= 16;
          //nOverflow |= ApeUnBitArrayRangeDecodeFastWithUpdate(16);
          while (lrangeval <= BOTTOM_VALUE)
          {   
              lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
              lcurrentbitindex += 8;
              llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
              lrangeval <<= 8;
          }
          
          // decode
          lrangeval = lrangeval >> 16;
          nOverflow |= udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow |= llowvalue / lrangeval;
          llowvalue -= lrangeval * nOverflow;          
                    
        }
    }
  
    // get the value
    {
        //ape_int32 nShift = 0;
        if (nPivotValue > 65535 /*>= ((ape_int32)1 << (ape_int32)16)*/)
        {
          nPivotValueBits = 0;
          while ((nPivotValue >> nPivotValueBits) > 0)
          {
            nPivotValueBits++;
          }

          nPivotValueBits -= 16;
          nPivotValueA = (nPivotValue >> (nPivotValueBits)) + 1;
          
          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
        
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          lrangeval = lrangeval / nPivotValueA;
          nBase = llowvalue / lrangeval;
          llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBase;

          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
                    
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          lrangeval = (lrangeval >> (nPivotValueBits));
          nBaseB = llowvalue / lrangeval;
          llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBaseB;
          
          nBase = (nBase << nPivotValueBits) + nBaseB;
        }
        else
        {
          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
                  
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                       | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          // decode
          lrangeval = udiv_32by32_arm9e((ape_uint32) nPivotValue,lrangeval);//lrangeval = lrangeval / nPivotValue;
          nBaseLower = udiv_32by32_arm9e(lrangeval,llowvalue);//nBaseLower = llowvalue / lrangeval;
          llowvalue -= lrangeval * nBaseLower;
          
          nBase = nBaseLower;
        }
    }
    
    // build the value
    nValue = nBase + (nOverflow * nPivotValue);
  }
  else//the following is ape version < 3990
  {
    //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
    if(lrangeval == 0)
    {
      Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
      return 0;
    }
            
    // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
    while (lrangeval <= BOTTOM_VALUE)
    {   
      lbuffervalue = (lbuffervalue << 8)
                     | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
      lcurrentbitindex += 8;
      llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
      lrangeval <<= 8;
    }
  
    // decode
    lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
    
    // lookup the symbol (must be a faster way than this)
    nOverflow = 0;
    RangeLastValue = 0;
    while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalOne[nOverflow + 1]))
    {
      RangeLastValue = RangeTempValue;
      nOverflow++;
    }
    
    // update
    llowvalue -= RangeLastValue;
    lrangeval = lrangeval * Ape_gtRangeWidthOne[nOverflow];
    
    // get the working k
    
    if (nOverflow == (MODEL_ELEMENTS - 1))
    {
      //RangeTempValue = ApeUnBitArrayRangeDecodeFastWithUpdate(5);
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> 5;
      RangeTempValue = udiv_32by32_arm9e(lrangeval,llowvalue);//RangeTempValue = llowvalue / lrangeval;
      llowvalue -= lrangeval * RangeTempValue;  
      nOverflow = 0;
    }
    else
    {
      RangeTempValue = (BitArrayStateY->k < 1) ? (0) : (BitArrayStateY->k - 1);
    }
    
    // figure the extra bits on the left and the left value
    if (RangeTempValue <= 16 || Ape_gVersion < 3910)
    {
      //nValue = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue);
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> RangeTempValue;
      nValue = udiv_32by32_arm9e(lrangeval,llowvalue);//nValue = llowvalue / lrangeval;
      llowvalue -= lrangeval * nValue;        
    }                    
    else
    {    
      //ape_int32 nX1 = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
      //ape_int32 nX2 = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue - 16);
      ape_int32 nX1;
      ape_int32 nX2;

      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> 16;
      nX1 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX1 = llowvalue / lrangeval;
      llowvalue -= lrangeval * nX1; 
      
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> (RangeTempValue - 16);
      nX2 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX2 = llowvalue / lrangeval;
      llowvalue -= lrangeval * nX2;             
      
      nValue = nX1 | (nX2 << 16);
    }
        
    // build the value and output it
    nValue += (nOverflow << RangeTempValue);
  }
  RangeLastValue = BitArrayStateY->nKSum;
  // update nKSum
  RangeLastValue += ((nValue + 1) >> 1) - ((RangeLastValue + 16) >> 5);
  RangeTempValue = BitArrayStateY->k;
  // update k
  if (RangeLastValue < Ape_gtKSumMinBoundary[RangeTempValue])
  {	 
    RangeTempValue--;
  }
  else if (RangeLastValue >= Ape_gtKSumMinBoundary[RangeTempValue + 1]) 
  	   {
         RangeTempValue++;
       }

  BitArrayStateY->nKSum = RangeLastValue;
  BitArrayStateY->k = RangeTempValue;
  // output the value (converted to signed)
  Ape_gnYValue = (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));
  	
////////////////////nX///////////////////////////////////////////
  //nValue = 0;
  //nOverflow = 0;
  //nBase = 0;
  //nPivotValueBits = 0;
  if (Ape_gVersion >= 3990)
  {
    // figure the pivot value
    nPivotValue = max((BitArrayStateX->nKSum >> 5), 1);//max(BitArrayStateX->nKSum / 32, 1);
    
    
    {
        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
        if(lrangeval == 0)
        {
          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
          return 0;
        }
        
        // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
        while (lrangeval <= BOTTOM_VALUE)
        {   
          lbuffervalue = (lbuffervalue << 8)
                         | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
        }
  
        // decode
        lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
        
        // lookup the symbol (must be a faster way than this)
        nOverflow = 0;
        RangeLastValue = 0;
        while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalTwo[nOverflow + 1]))
        {
          RangeLastValue = RangeTempValue;
          nOverflow++;
        }
        
        // update
        llowvalue -= RangeLastValue;
        lrangeval = lrangeval * Ape_gtRangeWidthTwo[nOverflow];
        
        // get the working k
        if (nOverflow == (MODEL_ELEMENTS - 1))
        {
          //nOverflow = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
          while (lrangeval <= BOTTOM_VALUE)
          {   
              lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
              lcurrentbitindex += 8;
              llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
              lrangeval <<= 8;
          }
          
          // decode
          lrangeval = lrangeval >> 16;
          nOverflow = udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow = llowvalue / lrangeval;
          llowvalue -= lrangeval * nOverflow;          
          
          nOverflow <<= 16;
          //nOverflow |= ApeUnBitArrayRangeDecodeFastWithUpdate(16);
          while (lrangeval <= BOTTOM_VALUE)
          {   
              lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
              lcurrentbitindex += 8;
              llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
              lrangeval <<= 8;
          }
          
          // decode
          lrangeval = lrangeval >> 16;
          nOverflow |= udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow |= llowvalue / lrangeval;
          llowvalue -= lrangeval * nOverflow;          
                    
        }
    }
  
    // get the value
    {
        //ape_int32 nShift = 0;
        if (nPivotValue > 65535 /*>= ((ape_int32)1 << (ape_int32)16)*/)
        {
          nPivotValueBits = 0;
          while ((nPivotValue >> nPivotValueBits) > 0)
          {
            nPivotValueBits++;
          }

          nPivotValueBits -= 16;
          nPivotValueA = (nPivotValue >> (nPivotValueBits)) + 1;
          
          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
        
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          lrangeval = lrangeval / nPivotValueA;
          nBase = llowvalue / lrangeval;
          llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBase;

          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
                    
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                          | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          lrangeval = (lrangeval >> (nPivotValueBits));
          nBaseB = llowvalue / lrangeval;
          llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBaseB;
          
          nBase = (nBase << nPivotValueBits) + nBaseB;
        }
        else
        {
          //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
          if(lrangeval == 0)
          {
            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
            return 0;
          }
                  
          while (lrangeval <= BOTTOM_VALUE)
          {   
            lbuffervalue = (lbuffervalue << 8)
                       | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
            lcurrentbitindex += 8;
            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
            lrangeval <<= 8;
          }
          
          // decode
          lrangeval = udiv_32by32_arm9e((ape_uint32) nPivotValue,lrangeval);//lrangeval = lrangeval / nPivotValue;
          nBaseLower = udiv_32by32_arm9e(lrangeval,llowvalue);//nBaseLower = llowvalue / lrangeval;
          llowvalue -= lrangeval * nBaseLower;
          
          nBase = nBaseLower;
        }
    }
    
    // build the value
    nValue = nBase + (nOverflow * nPivotValue);
  }
  else//the following is ape version < 3990
  {
    //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
    if(lrangeval == 0)
    {
      Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
      return 0;
    }
            
    // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
    while (lrangeval <= BOTTOM_VALUE)
    {   
      lbuffervalue = (lbuffervalue << 8)
                     | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
      lcurrentbitindex += 8;
      llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
      lrangeval <<= 8;
    }
  
    // decode
    lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
    
    // lookup the symbol (must be a faster way than this)
    nOverflow = 0;
    RangeLastValue = 0;
    while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalOne[nOverflow + 1]))
    {
      RangeLastValue = RangeTempValue;
      nOverflow++;
    }
    
    // update
    llowvalue -= RangeLastValue;
    lrangeval = lrangeval * Ape_gtRangeWidthOne[nOverflow];
    
    // get the working k
    
    if (nOverflow == (MODEL_ELEMENTS - 1))
    {
      //RangeTempValue = ApeUnBitArrayRangeDecodeFastWithUpdate(5);
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> 5;
      RangeTempValue = udiv_32by32_arm9e(lrangeval,llowvalue);//RangeTempValue = llowvalue / lrangeval;
      llowvalue -= lrangeval * RangeTempValue;  
      nOverflow = 0;
    }
    else
    {
      RangeTempValue = (BitArrayStateX->k < 1) ? (0) : (BitArrayStateX->k - 1);
    }
    
    // figure the extra bits on the left and the left value
    if (RangeTempValue <= 16 || Ape_gVersion < 3910)
    {
      //nValue = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue);
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> RangeTempValue;
      nValue = udiv_32by32_arm9e(lrangeval,llowvalue);//nValue = llowvalue / lrangeval;
      llowvalue -= lrangeval * nValue;        
    }                    
    else
    {    
      //ape_int32 nX1 = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
      //ape_int32 nX2 = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue - 16);
      ape_int32 nX1;
      ape_int32 nX2;

      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> 16;
      nX1 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX1 = llowvalue / lrangeval;
      llowvalue -= lrangeval * nX1;  
      
      while (lrangeval <= BOTTOM_VALUE)
      {   
          lbuffervalue = (lbuffervalue << 8)
                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
          lcurrentbitindex += 8;
          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
          lrangeval <<= 8;
      }
      
      // decode
      lrangeval = lrangeval >> (RangeTempValue - 16);
      nX2 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX2 = llowvalue / lrangeval;
      llowvalue -= lrangeval * nX2;            
      
      nValue = nX1 | (nX2 << 16);
    }
        
    // build the value and output it
    nValue += (nOverflow << RangeTempValue);
  }
  
  RangeLastValue = BitArrayStateX->nKSum;
  // update nKSum
  RangeLastValue += ((nValue + 1) >> 1) - ((RangeLastValue + 16) >> 5);
  RangeTempValue = BitArrayStateX->k;
  // update k
  if (RangeLastValue < Ape_gtKSumMinBoundary[RangeTempValue])
  {	 
    RangeTempValue--;
  }
  else if (RangeLastValue >= Ape_gtKSumMinBoundary[RangeTempValue + 1]) 
  	   {
         RangeTempValue++;
       }
       
  BitArrayStateX->nKSum = RangeLastValue;
  BitArrayStateX->k = RangeTempValue;  
     
  Ape_gRangeValue = lrangeval;
  Ape_gBufferValue = lbuffervalue;
  Ape_gCurrentBitIndex = lcurrentbitindex;
  Ape_gLowValue = llowvalue;  
  
  // output the value (converted to signed)
  //*nX = (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));
  return  (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));
  	
}


void ApeUnBitArrayFlushState(struct UNBIT_ARRAY_STATE * BitArrayState)
{
  BitArrayState->k = 10;
  BitArrayState->nKSum = ((ape_int32)1 << BitArrayState->k) * 16;
}

void ApeUnBitArrayFlushBitArray()
{
  ApeUnBitArrayAdvanceToByteBoundary();
  Ape_gCurrentBitIndex += 8; // ignore the first byte... (slows compression too much to not output this dummy byte)
  Ape_gBufferValue = ApeUnBitArrayGetC();
  Ape_gLowValue = Ape_gBufferValue >> (8 - EXTRA_BITS);
  Ape_gRangeValue = (ape_uint32) 1 << EXTRA_BITS;
  
  Ape_gRefillBitThreshold = (Ape_gBitsNum - 512);
}

void ApeUnBitArrayFinalize()
{
  // normalize
  while (Ape_gRangeValue <= BOTTOM_VALUE)
  {   
     Ape_gCurrentBitIndex += 8;
     Ape_gRangeValue <<= 8;
  }
  
  // used to back-pedal the last two bytes out
  // this should never have been a problem because we've outputted and normalized beforehand
  // but stopped doing it as of 3.96 in case it accounted for rare decompression failures
  if (Ape_gVersion <= 3950)
    Ape_gCurrentBitIndex -= 16;
}

void ApeUnBitArrayGenerateArrayRange(ape_int32 * pOutputArray, ape_int32 nElements)
{
  struct UNBIT_ARRAY_STATE BitArrayState;
  ape_int32 z;
  ApeUnBitArrayFlushState(&BitArrayState);
  ApeUnBitArrayFlushBitArray();
  
  for (z = 0; z < nElements; z++)
  {
    pOutputArray[z] = ApeUnBitArrayDecodeValueRange(&BitArrayState);
  }
  
  ApeUnBitArrayFinalize();    
}
//============================================

ape_int32 ApeUnBitArrayCreateHelper(CIO * pIO, ape_int32 nBytes, ape_int32 nVersion)
{
  // check the parameters
  if ((pIO == NULL) || (nBytes <= 0))
  {
    return ERROR_BAD_PARAMETER;
  }
  
  // save the size
  Ape_gElements = nBytes / 4;
  Ape_gBytesNum = Ape_gElements * 4;
  Ape_gBitsNum = Ape_gBytesNum * 8;
  
  // set the variables
  Ape_pIO = pIO;
  Ape_gVersion = nVersion;
  Ape_gCurrentBitIndex = 0;
  
  // create the bitarray
  Ape_pBitArray = Ape_pInBuffer;//&Ape_gInBuffer[0];//(uint32*)malloc(sizeof(ape_uint32)*Ape_gElements);
  //add by XW 20070324
  Ape_pIOBase=pIO;
  Ape_pBitArrayBase = Ape_pInBuffer;//&Ape_gInBuffer[0];
  
  return (Ape_pBitArray != NULL) ? 0 : ERROR_INSUFFICIENT_MEMORY;
}

ape_uint32 ApeUnBitArrayDecodeValueXBits(ape_uint32 nBits) 
{
  ape_uint32 nLeftBits ;
  ape_uint32 nBitArrayIndex ;
  ape_int32 nRightBits ;
  
  ape_uint32 nLeftValue ;
  ape_uint32 nRightValue ;
  
  
  // get more data if necessary,当解码新帧时,判断以解码比特数目+32是否超过输入缓冲区总比特数目,若超过就填新比特流
  if ((Ape_gCurrentBitIndex + nBits) >= Ape_gBitsNum)
  {
    ApeUnBitArrayBaseFillBitArray();
  }
  
  // variable declares
  nLeftBits = 32 - (Ape_gCurrentBitIndex & 31);
  nBitArrayIndex = Ape_gCurrentBitIndex >> 5;
  Ape_gCurrentBitIndex += nBits;
  
  // if their isn't an overflow to the right value, get the value and exit
  if (nLeftBits >= nBits)
  {
    return (Ape_pBitArray[nBitArrayIndex] & (Ape_gtPowersOfTwoMinusOne[nLeftBits])) >> (nLeftBits - nBits);
  }
  
  // must get the "split" value from left and right
  nRightBits = nBits - nLeftBits;
  
  nLeftValue = ((Ape_pBitArray[nBitArrayIndex] & Ape_gtPowersOfTwoMinusOne[nLeftBits]) << nRightBits);
  nRightValue = (Ape_pBitArray[nBitArrayIndex + 1] >> (32 - nRightBits));
  return (nLeftValue | nRightValue);
}

ape_int32 ApeUnBitArrayFillBitArray() 
{
  // get the bit array index
  ape_uint32 nBitArrayIndex = Ape_gCurrentBitIndex >> 5;
  ape_int32  nBytesToRead ;
  ape_uint32 nBytesRead ;
  ape_int32  nRetVal ;
  
  //ape_uint32 * pTmp;
  //ape_uint32 i = 0;
  //ape_uint32 nPassToRead = ((BLOCKS_PER_BITSTREAMREAD)*4/512);
  
  
  // move the remaining data to the front
  memmove((void *) (Ape_pBitArray), ( void *) (Ape_pBitArray + nBitArrayIndex),LENGTH(( Ape_gBytesNum - (nBitArrayIndex * 4))));
  
  // read the new data
  nBytesToRead = nBitArrayIndex * 4;
  nBytesRead = 0;
  nRetVal = Ape_pIO->Read((void*)Ape_pIO,(void *) (Ape_pBitArray + Ape_gElements - nBitArrayIndex), (unsigned int)nBytesToRead,(unsigned int*) &nBytesRead);
  //1080 *4 = 4320 Bs
  //pTmp=(Ape_pBitArray + Ape_gElements - nBitArrayIndex);
  
  // adjust the m_Bit pointer
  Ape_gCurrentBitIndex = Ape_gCurrentBitIndex & 31;
  
  // return
  return (nRetVal == 0) ? 0 : ERROR_IO_READ;
}

//remove bitstream used and append new bitstream added by hxd 20070618
ape_int32 ApeUnBitArrayFillBitArrayOutside() 
{
  // get the bit array index
  ape_uint32 nBitArrayIndex = Ape_gCurrentBitIndex >> 5;
  ape_int32  nBytesToRead ;
  ape_uint32 nBytesRead ;
  ape_int32  nRetVal ;
  
  //ape_uint32 * pTmp;
  //ape_uint32 i = 0;
  //ape_uint32 nPassToRead = ((BLOCKS_PER_BITSTREAMREAD)*4/512);
  /*bug fix*/
  if ((long)(Ape_gBytesNum - (nBitArrayIndex * 4))<0)
  {
	nBitArrayIndex = 0;//Ape_gBytesNum/4;
	Ape_gCurrentBitIndex = 0;//(nBitArrayIndex << 5);
  }
  
  
  // move the remaining data to the front
  memmove((void *) (Ape_pBitArray), ( void *) (Ape_pBitArray + nBitArrayIndex),LENGTH(( Ape_gBytesNum - (nBitArrayIndex * 4))));
  
  // read the new data
  nBytesToRead = nBitArrayIndex * 4;
  nBytesRead = 0;
  nRetVal = Ape_pIO->Read((void *)Ape_pIO,(void *) (Ape_pBitArray + Ape_gElements - nBitArrayIndex), (unsigned int)nBytesToRead,(unsigned int*) &nBytesRead);
  //1080 *4 = 4320 Bs
  //pTmp=(Ape_pBitArray + Ape_gElements - nBitArrayIndex);
  
  // adjust the m_Bit pointer
  Ape_gCurrentBitIndex = Ape_gCurrentBitIndex & 31;
  
  // return
  return (nRetVal == 0) ? 0 : ERROR_IO_READ;
}

void ApeUnBitArrayAdvanceToByteBoundary() 
{
  ape_int32 nMod = Ape_gCurrentBitIndex % 8;
  if (nMod != 0)
  {
    Ape_gCurrentBitIndex += 8 - nMod;
  }
}

ape_int32 ApeUnBitArrayFillAndResetBitArray(ape_int32 nFileLocation, ape_int32 nNewBitIndex) 
{
  ape_uint32 nBytesRead = 0;
  //ape_uint32 nPassToRead = ((BLOCKS_PER_BITSTREAMREAD+16)*4/512);
  // reset the bit index
  Ape_gCurrentBitIndex = nNewBitIndex;
  
  // seek if necessary
  if (nFileLocation != -1)
  {
    if (Ape_pIO->Seek(Ape_pIO,nFileLocation, FILE_BEGIN) != 0)
    {
      return ERROR_IO_READ;
    }
  }
      
  // read the new data into the bit array
  
  //Important Mod By XW 20070324
  if (Ape_pIO->Read((void *)Ape_pIO,((void *) &Ape_pBitArray[0]), Ape_gBytesNum,(unsigned int*) &nBytesRead) != 0)
  {	
    return ERROR_IO_READ;
  }
 
  return 0;
}

#pragma arm section code

#endif
#endif
