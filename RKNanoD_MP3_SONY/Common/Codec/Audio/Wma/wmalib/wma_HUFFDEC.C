//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) Microsoft Corporation, 1996 - 1999

Module Name:

    HuffDec.cpp

Abstract:

    Huffman decoder. Simplified from Sanjeevm's huffman.cpp

Author:

    Wei-ge Chen (wchen) 19-July-1999

Revision History:

 Peter X. Zuo (peterzuo) 9-June-2003: C Opt: Unroll loop

*************************************************************************/

//#include <stdio.h>
//#include <stdlib.h>
#include "../include/audio_main.h"
#include "..\wmaInclude\macros.h"
#include "..\wmaInclude\huffdec.h"
#include "..\wmaInclude\msaudiodec.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

#if (!WMA_OPT_HUFFDEC_ARM)
// Removed redundant & operation, unrolled for speed.
// Currenty used code.
WMARESULT huffDecGet(const U16 *pDecodeTable, CWMAInputBitStream *bs,
                     U32* puBitCnt, U32 *puResult, U32* puSign)
{
    const int FIRST_LOAD = 10;
    const int SECOND_LOAD = 12;

    register unsigned int ret_value;
    register const unsigned short* node_base = pDecodeTable;

    U32 uBits;
    register U32 codeword;

    WMARESULT  wmaResult;
//#ifdef PROFILE
    //profiling a function that gets called this often has too much overhead.
    //FunctionProfile fp;
    //FunctionProfileStart(&fp,HUFF_DEC_GET_PROFILE);
//#endif

    TRACEWMA_EXIT(wmaResult, ibstrmPeekBits(bs, FIRST_LOAD + SECOND_LOAD + 1, &uBits));
    codeword = uBits;

    // Do first five 2-bit tables

    //unroll, 1
    node_base += codeword >> 30;  // Use top 2 bits as offset node_base is basic address of huffdectbl16sO
    ret_value = *node_base;
    if (ret_value & 0x8000)//直接查找到叶子结点
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset 子树的位置
    codeword <<= 2;            // Advance to next 2 bits

    //unroll, 2
    node_base += codeword >> 30;  // Use top 2 bits as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 2;            // Advance to next 2 bits

    //unroll, 3
    node_base += codeword >> 30;  // Use top 2 bits as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 2;            // Advance to next 2 bits

    //unroll, 4
    node_base += codeword >> 30;  // Use top 2 bits as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 2;            // Advance to next 2 bits

    //unroll, 5
    node_base += codeword >> 30;  // Use top 2 bits as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 2;            // Advance to next 2 bits

    // Do remaining three 2-bit tables
    //unroll 1
    node_base += codeword >> 30;  // Use top 2 bits as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 2;            // Advance to next 2 bits

    //unroll 2
    node_base += codeword >> 30;  // Use top 2 bits as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 2;            // Advance to next 2 bits

    //unroll 3
    node_base += codeword >> 30;  // Use top 2 bits as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 2;            // Advance to next 2 bits


    // Do six 1-bit tables
    //unroll 6
    node_base += codeword >> 31;  // Use top bit as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 1;            // Advance to next bit


    node_base += codeword >> 31;  // Use top bit as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 1;            // Advance to next bit


    node_base += codeword >> 31;  // Use top bit as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 1;            // Advance to next bit


    node_base += codeword >> 31;  // Use top bit as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 1;            // Advance to next bit


    node_base += codeword >> 31;  // Use top bit as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 1;            // Advance to next bit


    node_base += codeword >> 31;  // Use top bit as offset
    ret_value = *node_base;
    if (ret_value & 0x8000)
        goto decode_complete;  // Found a leaf node!

    node_base += ret_value;    // No leaf node, follow the offset
    codeword <<= 1;            // Advance to next bit

decode_complete:
    assert(ret_value & 0x8000);
    *puBitCnt = ((ret_value >> 10) & (0x0000001F));
    *puResult = ret_value & 0x000003FF;
    if (*puResult >= 0x03FC)
        *puResult = *(node_base + (*puResult & 0x0003) + 1);

    if (puSign != NULL)
        *puSign = uBits << *puBitCnt;

exit:
//#ifdef PROFILE
    //FunctionProfileStop(&fp);
//#endif
    return wmaResult;
}
#endif
#pragma arm section code

#endif
#endif
#endif
