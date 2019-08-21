#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
//#include "CharacterHelper.h"
#if 0
str_ansi * GetANSIFromUTF8(const str_utf8 * pUTF8)
{
    char * pUTF16 = GetUTF16FromUTF8(pUTF8);
    str_ansi * pANSI = GetANSIFromUTF16(pUTF16);
    //delete [] pUTF16;
	free(pUTF16);
    return pANSI;
}

str_ansi * GetANSIFromUTF16(const char * pUTF16)
{
#if 0
    const ape_int32 nCharacters = pUTF16 ? wcslen(pUTF16) : 0;//noted by xw 20070323
    ape_int32 z=0;
    #ifdef _WIN32
        ape_int32 nANSICharacters = (2 * nCharacters);
        str_ansi * pANSI =(str_ansi*)malloc(sizeof(str_ansi)*(nANSICharacters + 1));
        memset(pANSI, 0, (nANSICharacters + 1) * sizeof(str_ansi));
        if (pUTF16)
            WideCharToMultiByte(CP_ACP, 0, pUTF16, -1, pANSI, nANSICharacters, NULL, NULL);
    #else
        str_utf8 * pANSI =(str_utf8*) malloc(sizeof(str_utf8)*(nCharacters + 1));
        for (z = 0; z < nCharacters; z++)
            pANSI[z] = (pUTF16[z] >= 256) ? '?' : (str_utf8) pUTF16[z];
        pANSI[nCharacters] = 0;
    #endif

    return (str_ansi *) pANSI;
#endif
   return 0;
}

char * GetUTF16FromANSI(const str_ansi * pANSI)
{
    const ape_int32 nCharacters = pANSI ? strlen(pANSI) : 0;
    ape_int32 z = 0;
    char * pUTF16 = (char*)malloc(sizeof(char)*(nCharacters + 1));

    #ifdef _WIN32
        memset(pUTF16, 0, sizeof(char) * (nCharacters + 1));
        if (pANSI)
            MultiByteToWideChar(CP_ACP, 0, pANSI, -1, pUTF16, nCharacters);
    #else
        for (z = 0; z < nCharacters; z++)
            pUTF16[z] = (char) ((str_utf8) pANSI[z]);
        pUTF16[nCharacters] = 0;
    #endif

    return pUTF16;
}

char * GetUTF16FromUTF8(const str_utf8 * pUTF8)
{
    // get the length
    ape_int32 nCharacters = 0; 
	ape_int32 nIndex = 0;
	char * pUTF16;

    while (pUTF8[nIndex] != 0)
    {
        if ((pUTF8[nIndex] & 0x80) == 0)
            nIndex += 1;
        else if ((pUTF8[nIndex] & 0xE0) == 0xE0)
            nIndex += 3;
        else
            nIndex += 2;

        nCharacters += 1;
    }

    // make a UTF-16 string
    pUTF16 = (char*)malloc(sizeof(char)*(nCharacters + 1));
    nIndex = 0; nCharacters = 0;
    while (pUTF8[nIndex] != 0)
    {
        if ((pUTF8[nIndex] & 0x80) == 0)
        {
            pUTF16[nCharacters] = pUTF8[nIndex];
            nIndex += 1;
        }
        else if ((pUTF8[nIndex] & 0xE0) == 0xE0)
        {
            pUTF16[nCharacters] = ((pUTF8[nIndex] & 0x1F) << 12) | ((pUTF8[nIndex + 1] & 0x3F) << 6) | (pUTF8[nIndex + 2] & 0x3F);
            nIndex += 3;
        }
        else
        {
            pUTF16[nCharacters] = ((pUTF8[nIndex] & 0x3F) << 6) | (pUTF8[nIndex + 1] & 0x3F);
            nIndex += 2;
        }

        nCharacters += 1;
    }
    pUTF16[nCharacters] = 0;

    return pUTF16; 
}

str_utf8 * GetUTF8FromANSI(const str_ansi * pANSI)
{
    char * pUTF16 = GetUTF16FromANSI(pANSI);
    str_utf8 * pUTF8 = GetUTF8FromUTF16(pUTF16);
    free(pUTF16);
    return pUTF8;
}

str_utf8 * GetUTF8FromUTF16(const char * pUTF16)
{
#if 0
    // get the size(s)
    ape_int32 nCharacters = wcslen(pUTF16);
    ape_int32 nUTF8Bytes = 0;
	ape_int32 z = 0;
	ape_int32 nUTF8Index;
	str_utf8 * pUTF8;
    for (z = 0; z < nCharacters; z++)
    {
        if (pUTF16[z] < 0x0080)
            nUTF8Bytes += 1;
        else if (pUTF16[z] < 0x0800)
            nUTF8Bytes += 2;
        else
            nUTF8Bytes += 3;
    }

    // allocate a UTF-8 string
    pUTF8 = (str_utf8*)malloc(sizeof(str_utf8)*(nUTF8Bytes + 1));

    // create the UTF-8 string
    nUTF8Index = 0;
    for (z = 0; z < nCharacters; z++)
    {
        if (pUTF16[z] < 0x0080)
        {
            pUTF8[nUTF8Index++] = (str_utf8) pUTF16[z];
        }
        else if (pUTF16[z] < 0x0800)
        {
            pUTF8[nUTF8Index++] = 0xC0 | (pUTF16[z] >> 6);
            pUTF8[nUTF8Index++] = 0x80 | (pUTF16[z] & 0x3F);
        }
        else
        {
            pUTF8[nUTF8Index++] = 0xE0 | (pUTF16[z] >> 12);
            pUTF8[nUTF8Index++] = 0x80 | ((pUTF16[z] >> 6) & 0x3F);
            pUTF8[nUTF8Index++] = 0x80 | (pUTF16[z] & 0x3F);
        }
    }
    pUTF8[nUTF8Index++] = 0;

    // return the UTF-8 string
    return pUTF8;
#endif
}
#endif

#pragma arm section code

#endif
#endif
