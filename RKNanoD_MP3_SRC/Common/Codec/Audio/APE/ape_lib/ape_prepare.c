
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_prepare.h"
#include "ape_globalvardeclaration.h"

//Implementing nR=X-Y/2 and nL = nR+Y to get 8bits/16bits/24bits pcm output
void ApeUnprepare(ape_int32 X, ape_int32 Y,  WAVEFORMATEX * pWaveFormatEx, ape_uchar * pOutputLeft,ape_uchar * pOutputRight)
{
  ape_int32 RV, LV;
  //ape_int16 poutput1,poutput2;//added by hxd
  ape_uint32 nTemp = 0;
  ape_int32  nRTemp,nLTemp;
  
    #define CALCULATE_CRC_BYTE    *pCRC = (*pCRC >> 8) ^ Ape_gtCRC32[(*pCRC & 0xFF) ^ *pOutput++];
    // decompress and convert from (x,y) -> (l,r)
    // sort of long and ugly.... sorry
    
    if (pWaveFormatEx->nChannels == 2) 
    {
        if (pWaveFormatEx->wBitsPerSample == 16) 
        {
            // get the right and left values
            ape_int32 nR = X - (Y / 2);
            ape_int32 nL = nR + Y;
            
            //以下部分优化 added by hxd 20070705 
            nRTemp = nR + 0x8000;
            nRTemp >>= 16;
            nLTemp = nL + 0x8000;
            nLTemp >>= 16;            
            // error check (for overflows)
            if((nRTemp | nLTemp) == 0)//if ((nR < -32768) || (nR > 32767) || (nL < -32768) || (nL > 32767))
            {
	            *(ape_int16 *) pOutputRight = (ape_int16) nR;
              //pOutput++;//CALCULATE_CRC_BYTE
              //pOutput++;//CALCULATE_CRC_BYTE
              *(ape_int16 *) pOutputLeft = (ape_int16) nL;
              //pOutput++;//CALCULATE_CRC_BYTE
              //pOutput++;//CALCULATE_CRC_BYTE
            }
            else
            {
              //nR = 0;
              //nL = 0;//exit(-1);
              Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;//because ape bitstream is corrupted,error of decompressing frame appears. 
            }



        }
        else if (pWaveFormatEx->wBitsPerSample == 8) 
        {
            ape_uchar R = (X - (Y / 2) + 128);
            *pOutputRight = R;
            pOutputRight++;//CALCULATE_CRC_BYTE
            *pOutputLeft = (ape_uchar) (R + Y);
            pOutputLeft++;//CALCULATE_CRC_BYTE
        }
        else if (pWaveFormatEx->wBitsPerSample == 24) 
        {
            

            RV = X - (Y / 2);
            LV = RV + Y;
            
            
            if (RV < 0)
                nTemp = ((ape_uint32) (RV + 0x800000)) | 0x800000;
            else
                nTemp = (ape_uint32) RV;    
            
            *pOutputRight = (ape_uchar) ((nTemp >> 0) & 0xFF);
            pOutputRight++;//CALCULATE_CRC_BYTE
            *pOutputRight = (ape_uchar) ((nTemp >> 8) & 0xFF);
            pOutputRight++;//CALCULATE_CRC_BYTE
            *pOutputRight = (ape_uchar) ((nTemp >> 16) & 0xFF);
            pOutputRight++;//CALCULATE_CRC_BYTE

            nTemp = 0;
            if (LV < 0)
                nTemp = ((ape_uint32) (LV + 0x800000)) | 0x800000;
            else
                nTemp = (ape_uint32) LV;    
            
            *pOutputLeft = (ape_uchar) ((nTemp >> 0) & 0xFF);
            pOutputLeft++;//CALCULATE_CRC_BYTE
            
            *pOutputLeft = (ape_uchar) ((nTemp >> 8) & 0xFF);
            pOutputLeft++;//CALCULATE_CRC_BYTE
            
            *pOutputLeft = (ape_uchar) ((nTemp >> 16) & 0xFF);
            pOutputLeft++;//CALCULATE_CRC_BYTE
        }
    }
    else if (pWaveFormatEx->nChannels == 1) 
    {
        if (pWaveFormatEx->wBitsPerSample == 16) 
        {
            ape_int16 R = X;
                
            *(ape_int16 *) pOutputRight = (ape_int16) R;
            pOutputRight++;//CALCULATE_CRC_BYTE
            pOutputRight++;//CALCULATE_CRC_BYTE

        }
        else if (pWaveFormatEx->wBitsPerSample == 8) 
        {
            ape_uchar R = X + 128;
            *pOutputRight = R;
            pOutputRight++;//CALCULATE_CRC_BYTE
        }
        else if (pWaveFormatEx->wBitsPerSample == 24) 
        {
            ape_int32 RV = X;
            
            ape_uint32 nTemp = 0;
            if (RV < 0)
                nTemp = ((ape_uint32) (RV + 0x800000)) | 0x800000;
            else
                nTemp = (ape_uint32) RV;    
            
            *pOutputRight = (ape_uchar) ((nTemp >> 0) & 0xFF);
            pOutputRight++;//CALCULATE_CRC_BYTE
            *pOutputRight = (ape_uchar) ((nTemp >> 8) & 0xFF);
            pOutputRight++;//CALCULATE_CRC_BYTE
            *pOutputRight = (ape_uchar) ((nTemp >> 16) & 0xFF);
            pOutputRight++;//CALCULATE_CRC_BYTE
        }
    }
}

#ifdef BACKWARDS_COMPATIBILITY


ape_int32 ApeUnprepareOld(void *aI,ape_int32 *pInputX, ape_int32 *pInputY, ape_int32 nBlocks,  WAVEFORMATEX *pWaveFormatEx, ape_uchar *pRawData, ape_uint32 *pCRC, ape_int32 *pSpecialCodes, ape_int32 nFileVersion)
{
    // the CRC that will be figured during decompression
    ape_uint32 CRC = 0xFFFFFFFF;

    // decompress and convert from (x,y) -> (l,r)
    // sort of int and ugly.... sorry
    if (pWaveFormatEx->nChannels == 2) 
    {
        // convert the x,y data to raw data
        if (pWaveFormatEx->wBitsPerSample == 16) 
        {
            ape_int16 R;
            ape_uchar *Buffer = &pRawData[0];
            ape_int32 *pX = pInputX;
            ape_int32 *pY = pInputY;

            for (; pX < &pInputX[nBlocks]; pX++, pY++) 
            {
                R = *pX - (*pY / 2);
                
                *(ape_int16 *) Buffer = (ape_int16) R;
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                
                *(ape_int16 *) Buffer = (ape_int16) R + *pY;
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
            }
        }
        else if (pWaveFormatEx->wBitsPerSample == 8) 
        {
            ape_uchar *R = (ape_uchar *) &pRawData[0];
            ape_uchar *L = (ape_uchar *) &pRawData[1];
            ape_int32 SampleIndex;

            if (nFileVersion > 3830) 
            {
                for (SampleIndex = 0; SampleIndex < nBlocks; SampleIndex++, L+=2, R+=2) 
                {
                    *R = (ape_uchar) (pInputX[SampleIndex] - (pInputY[SampleIndex] / 2) + 128);
                    CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *R];
                    *L = (ape_uchar) (*R + pInputY[SampleIndex]);
                    CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *L];
                }
            }
            else 
            {
                for (SampleIndex = 0; SampleIndex < nBlocks; SampleIndex++, L+=2, R+=2)
                {
                    *R = (ape_uchar) (pInputX[SampleIndex] - (pInputY[SampleIndex] / 2));
                    CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *R];
                    *L = (ape_uchar) (*R + pInputY[SampleIndex]);
                    CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *L];

                }
            }
        }
        else if (pWaveFormatEx->wBitsPerSample == 24) 
        {
            ape_uchar *Buffer = (ape_uchar *) &pRawData[0];
            ape_int32 RV, LV;
            ape_int32 SampleIndex;
            ape_uint32 nTemp = 0;

            for (SampleIndex = 0; SampleIndex < nBlocks; SampleIndex++)
            {
                RV = pInputX[SampleIndex] - (pInputY[SampleIndex] / 2);
                LV = RV + pInputY[SampleIndex];
                                
                if (RV < 0)
                    nTemp = ((ape_uint32) (RV + 0x800000)) | 0x800000;
                else
                    nTemp = (ape_uint32) RV;    
                
                *Buffer = (ape_uchar) ((nTemp >> 0) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                
                *Buffer = (ape_uchar) ((nTemp >> 8) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];

                *Buffer = (ape_uchar) ((nTemp >> 16) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];

                nTemp = 0;
                if (LV < 0)
                    nTemp = ((ape_uint32) (LV + 0x800000)) | 0x800000;
                else
                    nTemp = (ape_uint32) LV;    
                
                *Buffer = (ape_uchar) ((nTemp >> 0) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                
                *Buffer = (ape_uchar) ((nTemp >> 8) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                
                *Buffer = (ape_uchar) ((nTemp >> 16) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
            }
        }
    }
    else if (pWaveFormatEx->nChannels == 1) 
    {
        // convert to raw data
        if (pWaveFormatEx->wBitsPerSample == 8) 
        {
            ape_uchar *R = (ape_uchar *) &pRawData[0];
            ape_int32 SampleIndex;

            if (nFileVersion > 3830) 
            {
                for (SampleIndex = 0; SampleIndex < nBlocks; SampleIndex++, R++)
                {
                    *R = pInputX[SampleIndex] + 128;
                    CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *R];
                }
            }
            else 
            {
                for (SampleIndex = 0; SampleIndex < nBlocks; SampleIndex++, R++)
                {
                    *R = (ape_uchar) (pInputX[SampleIndex]);
                    CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *R];
                }
            }

        }
        else if (pWaveFormatEx->wBitsPerSample == 24) 
        {

            ape_uchar *Buffer = (ape_uchar *) &pRawData[0];
            ape_int32 RV;
            ape_int32 SampleIndex ;
            ape_uint32 nTemp = 0;
            for (SampleIndex = 0; SampleIndex<nBlocks; SampleIndex++) 
            {
                RV = pInputX[SampleIndex];

                
                if (RV < 0)
                    nTemp = ((ape_uint32) (RV + 0x800000)) | 0x800000;
                else
                    nTemp = (ape_uint32) RV;    
                
                *Buffer = (ape_uchar) ((nTemp >> 0) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                
                *Buffer = (ape_uchar) ((nTemp >> 8) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                
                *Buffer = (ape_uchar) ((nTemp >> 16) & 0xFF);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
            }
        }
        else 
        {
            ape_uchar *Buffer = &pRawData[0];
            ape_int32 SampleIndex;

            for (SampleIndex = 0; SampleIndex < nBlocks; SampleIndex++) 
            {
                *(ape_int16 *) Buffer = (ape_int16) (pInputX[SampleIndex]);
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
                CRC = (CRC >> 8) ^ Ape_gtCRC32[(CRC & 0xFF) ^ *Buffer++];
            }
        }
    }

    CRC = CRC ^ 0xFFFFFFFF;

    *pCRC = CRC;

    return 0;
}

#endif // #ifdef BACKWARDS_COMPATIBILITY

#pragma arm section code

#endif
#endif
