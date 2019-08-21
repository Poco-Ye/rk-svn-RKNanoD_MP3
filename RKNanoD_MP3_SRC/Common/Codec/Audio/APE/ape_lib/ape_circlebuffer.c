#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_circlebuffer.h"
#include "ape_globalvardeclaration.h"

#if 0
void CCircleBuffer_cCCircleBuffer(void *aI)
{	
    ((CCircleBuffer*)aI)->m_pBuffer = NULL;
    ((CCircleBuffer*)aI)->m_nTotal = 0;
    ((CCircleBuffer*)aI)->m_nHead = 0;
    ((CCircleBuffer*)aI)->m_nTail = 0;
    ((CCircleBuffer*)aI)->m_nEndCap = 0;
    ((CCircleBuffer*)aI)->m_nMaxDirectWriteBytes = 0;
}

void CCircleBuffer_dCCircleBuffer(void *aI)
{
    SAFE_ARRAY_DELETE(((CCircleBuffer*)aI)->m_pBuffer)
}

void CCircleBuffer_CreateBuffer(void *aI,ape_int32 nBytes, ape_int32 nMaxDirectWriteBytes)
{
    //SAFE_ARRAY_DELETE(((CCircleBuffer*)aI)->m_pBuffer)
    
    ((CCircleBuffer*)aI)->m_nMaxDirectWriteBytes = nMaxDirectWriteBytes;
    ((CCircleBuffer*)aI)->m_nTotal = nBytes + 1 + nMaxDirectWriteBytes;
    ((CCircleBuffer*)aI)->m_pBuffer = (ape_uchar*)&ape_dec_buffer[0];//(ape_uint16*)malloc(sizeof(ape_uint16)*LENGTH(((CCircleBuffer*)aI)->m_nTotal));
    ((CCircleBuffer*)aI)->m_nHead = 0;
    ((CCircleBuffer*)aI)->m_nTail = 0;
    ((CCircleBuffer*)aI)->m_nEndCap = ((CCircleBuffer*)aI)->m_nTotal;
}

ape_int32 CCircleBuffer_MaxAdd(void *aI)
{
    ape_int32 nMaxAdd = (((CCircleBuffer*)aI)->m_nTail >= ((CCircleBuffer*)aI)->m_nHead) ? (((CCircleBuffer*)aI)->m_nTotal - 1 - ((CCircleBuffer*)aI)->m_nMaxDirectWriteBytes) - (((CCircleBuffer*)aI)->m_nTail - ((CCircleBuffer*)aI)->m_nHead) : ((CCircleBuffer*)aI)->m_nHead - ((CCircleBuffer*)aI)->m_nTail - 1;
    return nMaxAdd;
}

ape_int32 CCircleBuffer_MaxGet(void *aI)
{
    return (((CCircleBuffer*)aI)->m_nTail >= ((CCircleBuffer*)aI)->m_nHead) ? ((CCircleBuffer*)aI)->m_nTail - ((CCircleBuffer*)aI)->m_nHead : (((CCircleBuffer*)aI)->m_nEndCap - ((CCircleBuffer*)aI)->m_nHead) + ((CCircleBuffer*)aI)->m_nTail;
}

ape_int32 CCircleBuffer_Get(void *aI,ape_uchar * pBuffer, ape_int32 nBytes)
{
    ape_int32 nTotalGetBytes = 0;

    if (pBuffer != NULL && nBytes > 0)
    {
        ape_int32 nHeadBytes = min(((CCircleBuffer*)aI)->m_nEndCap - ((CCircleBuffer*)aI)->m_nHead, nBytes);
        ape_int32 nFrontBytes = nBytes - nHeadBytes;

        //memcpy(&pBuffer[0], &(((CCircleBuffer*)aI)->m_pBuffer[LENGTH(((CCircleBuffer*)aI)->m_nHead)]), MEMOP_LEN(nHeadBytes));
        memcpy(&pBuffer[0], &(((CCircleBuffer*)aI)->m_pBuffer[LENGTH(((CCircleBuffer*)aI)->m_nHead)]), LENGTH(nHeadBytes));
        nTotalGetBytes = nHeadBytes;

        if (nFrontBytes > 0)
        {
            //memcpy(&pBuffer[LENGTH(nHeadBytes)], &(((CCircleBuffer*)aI)->m_pBuffer[0]), MEMOP_LEN(nFrontBytes));
            memcpy(&pBuffer[LENGTH(nHeadBytes)], &(((CCircleBuffer*)aI)->m_pBuffer[0]), LENGTH(nFrontBytes));
            nTotalGetBytes += nFrontBytes;
        }

        CCircleBuffer_RemoveHead(aI,nBytes);
    }

    return nTotalGetBytes;
}

void CCircleBuffer_Empty(void *aI)
{
    ((CCircleBuffer*)aI)->m_nHead = 0;
    ((CCircleBuffer*)aI)->m_nTail = 0;
    ((CCircleBuffer*)aI)->m_nEndCap = ((CCircleBuffer*)aI)->m_nTotal;
}

ape_int32 CCircleBuffer_RemoveHead(void *aI,ape_int32 nBytes)
{
    nBytes = min(CCircleBuffer_MaxGet(aI), nBytes);
    ((CCircleBuffer*)aI)->m_nHead += nBytes;
    if (((CCircleBuffer*)aI)->m_nHead >= ((CCircleBuffer*)aI)->m_nEndCap)
        ((CCircleBuffer*)aI)->m_nHead -= ((CCircleBuffer*)aI)->m_nEndCap;
    return nBytes;
}

ape_int32 CCircleBuffer_RemoveTail(void *aI,ape_int32 nBytes)
{
    nBytes = min(CCircleBuffer_MaxGet(aI), nBytes);
    ((CCircleBuffer*)aI)->m_nTail -= nBytes;
    if (((CCircleBuffer*)aI)->m_nTail < 0)
        ((CCircleBuffer*)aI)->m_nTail += ((CCircleBuffer*)aI)->m_nEndCap;
    return nBytes;
}

// direct writing
ape_uchar * CCircleBuffer_GetDirectWritePointer(void *aI)
{
    // return a pointer to the tail -- note that it will always be safe to write
    // at least m_nMaxDirectWriteBytes since we use an end cap region
    return &(((CCircleBuffer*)aI)->m_pBuffer[LENGTH(((CCircleBuffer*)aI)->m_nTail)]);
}

void CCircleBuffer_UpdateAfterDirectWrite(void *aI,ape_int32 nBytes)
{
    // update the tail
    ((CCircleBuffer*)aI)->m_nTail += nBytes;

    // if the tail enters the "end cap" area, set the end cap and loop around
    if (((CCircleBuffer*)aI)->m_nTail >= (((CCircleBuffer*)aI)->m_nTotal - ((CCircleBuffer*)aI)->m_nMaxDirectWriteBytes))
	{
         ((CCircleBuffer*)aI)->m_nEndCap = ((CCircleBuffer*)aI)->m_nTail;
         ((CCircleBuffer*)aI)->m_nTail = 0;
	}
}
#endif

#pragma arm section code

#endif
#endif