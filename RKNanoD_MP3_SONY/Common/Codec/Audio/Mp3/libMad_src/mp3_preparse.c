/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name��   mp3_preparse.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"

#include "../include/audio_main.h"
#include "mp3_global.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

typedef struct
{
    long samplerate ;
    long bitrate ;
    long channels ;
    long length ;
    long outlength ;
    unsigned char ucIsVBR;
    unsigned char ucChannels;
    unsigned short usSampleRate;
    unsigned long ulFirstFrame;
    unsigned long ulLength;
    unsigned long ulBitRate;
    unsigned long ulTimePos;
    unsigned long ulTimeLength;
    unsigned long ulOutputLength;
} tMP3;

#define ENCODED_DATA_SIZE       (2048)

_ATTR_MP3DEC_BSS_
static int buffer_size;
_ATTR_MP3DEC_BSS_
static unsigned long ulAverageBitrate;
_ATTR_MP3DEC_BSS_
//static char pENCODED_DATA[ENCODED_DATA_SIZE];
char pENCODED_DATA[ENCODED_DATA_SIZE];

extern long synth_out_samples_limit ; //by Vincent, May 8
extern long frame_layer;

//****************************************************************************
//
// The following is a mapping from the sample rate descriptor in the
// tMPEGHeader structure to the integer number of samples per second.
//
//****************************************************************************
_ATTR_MP3DEC_DATA_
unsigned short  usSRMap[] = { 11025, 12000,  8000, 0,
                              22050, 24000, 16000, 0,
                              44100, 48000, 32000, 0
                            };

//****************************************************************************
//
// The following is a mapping from the bitrate_index and ID bit in the MPEG
// sync header to a the bitrate of the frame.  The first 16 entries are for
// ID=0 (i.e. MPEG-2 or MPEG-2.5 half sample rate) and the second 16 entries
// are for ID=1 (i.e. MPEG-1 full sample rate).
//
//****************************************************************************
_ATTR_MP3DEC_DATA_
unsigned short  usBRMap[] = {   0,   8,  16,  24,
                                32,  40,  48,  56,
                                64,  80,  96, 112,
                                128, 144, 160,   0,
                                0,  32,  40,  48,
                                56,  64,  80,  96,
                                112, 128, 160, 192,
                                224, 256, 320,   0
                            };

_ATTR_MP3DEC_DATA_
unsigned short  usBRMap2[] =    {   0,   8,  16,  24,               //MPEG 2
                                    32,  40,  48,  56,
                                    64,  80,  96, 112,
                                    128, 144, 160,   0,
                                    0,  32,  48,  56,       //MPEG 1
                                    64,  80,  96, 112,
                                    128, 160, 192, 224,
                                    256, 320, 384,   0
                                };

_ATTR_MP3DEC_DATA_
unsigned short  usFLMap[12][16] =
{

    {0, 52, 104, 156, 208, 261, 313, 365, 417, 522, 626,  731,  835,  940, 1044, 0},    //11.025    // MPEG2.5
    {0, 48,  96, 144, 192, 240, 288, 336, 384, 480, 576,  672,  768,  864,  960, 0},    //12
    {0, 72, 144, 216, 288, 360, 432, 504, 576, 720, 864, 1008, 1152, 1296, 1440, 0},    //8
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0},    //0
    {0, 26,  52,  78, 104, 130, 156, 182, 208, 261, 313,  365,  417,  470,  522, 0},    //22.05     // MPEG2
    {0, 24,  48,  72,  96, 120, 144, 168, 192, 240, 288,  336,  384,  432,  480, 0},    //24
    {0, 36,  72, 108, 144, 180, 216, 252, 288, 360, 432,  504,  576,  648,  720, 0},    //16
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0},    //0
    {0, 104, 130, 156, 182, 208, 261, 313, 365, 417, 522,  626,  731,  835, 1044, 0},   //44.1      // MPEG1
    {0, 96, 120, 144, 168, 192, 240, 288, 336, 384, 480,  576,  672,  768,  960, 0},    //48
    {0, 144, 180, 216, 252, 288, 360, 432, 504, 576, 720,  864, 1008, 1152, 1440, 0},   //32
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0}     //0
};

_ATTR_MP3DEC_DATA_
unsigned short  usFLMap2[8][16] =
{
    {0, 52, 104, 156, 208, 261, 313, 365, 417, 522, 626,  731,  835,  940, 1044, 0},    //22.05     // MPEG2
    {0, 48,  96, 144, 192, 240, 288, 336, 384, 480, 576,  672,  768,  864,  960, 0},    //32
    {0, 72, 144, 216, 288, 360, 432, 504, 576, 720, 864, 1008, 1152, 1296, 1440, 0},    //16
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0},    //0
    {0, 104, 156, 182, 208, 261, 313, 365, 417, 522, 626,  731,  835, 1044, 1253, 0},   //44.1      // MPEG1
    {0, 96, 144, 168, 192, 240, 288, 336, 384, 480, 576,  672,  768,  960, 1152, 0},    //48
    {0, 144, 216, 252, 288, 360, 432, 504, 576, 720, 864, 1008, 1152, 1440, 1728, 0},   //32
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0}     //0
};


_ATTR_MP3DEC_DATA_
static unsigned short   usBRMapLayer1V1[] =
{
    0,  32,  64,  96,
    128, 160, 192, 224,
    256, 288, 320, 352,
    384, 416, 448,   0
};

_ATTR_MP3DEC_DATA_
static unsigned short   usBRMapLayer1V2[] =
{
    0,  32,  48,  56,
    64,  80,  96, 112,
    128, 144, 160, 176,
    192, 224, 256,   0
};

/* 0,   32,  64,  96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448,  0 */

#if 0
_ATTR_MP3DEC_DATA_
unsigned short  usFLMap3[8][16] = {       /*80*/                 /*144*/   /*176*/
    {0, 48,  96,/* before no used */ 120, 136,  0,  208,    240, 276,  0,  348,   0 , 416,  484,  556, /* after no used */ 0},  //22.05
    {0, 52, 104,/* before no used */ 112, 128,  0,  192,    224, 256,  0,  320,   0 , 384,  448,  512, /* after no used */ 0},  //24        // MPEG2
    {0, 72, 144,/* before no used */ 168, 192,  0,  288,    336, 384,  0,  480,   0 , 576,  672,  768, /* after no used */ 0},  //16
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0 },   //0
    /*288:TODO*/
    {0, 104, 156, 182, /* before no used */136, 172, 208, 240, 276, 0, 348,/* after no used */   731,    835, 1044, 1253, 0},   //44.1      // MPEG1
    {0, 96, 144, 168, /* before no used */128, 160, 192, 224, 256, 0, 320,/* after no used */   672,    768,  960, 1152, 0},    //48
    {0, 144, 216, 252, /* before no used */192, 240, 288, 336, 384, 0, 480,/* after no used */  1008, 1152, 1440, 1728, 0}, //32
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0}     //0
};
#else
_ATTR_MP3DEC_DATA_
unsigned short  usFLMap3[8][16] = {       /*80*/                 /*144*/   /*176*/
    {0, 48,  96,/* before no used */ 120, 136,  0,  208,    240, 276,  0,  348,   0 , 416,  484,  556, /* after no used */ 0},  //22.05
    {0, 52, 104,/* before no used */ 112, 128,  0,  192,    224, 256,  0,  320,   0 , 384,  448,  512, /* after no used */ 0},  //24        // MPEG2
    {0, 72, 144,/* before no used */ 168, 192,  0,  288,    336, 384,  0,  480,   0 , 576,  672,  768, /* after no used */ 0},  //16
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0 },   //0
    /*288:TODO*/
    {0, 104, 156, 182, /* before no used */136, 172, 208, 240, 276, 0, 348,/* after no used */383, 419, 452, 487, 0}, //44.1      // MPEG1
    {0, 96, 144, 168, /* before no used */128, 160, 192, 224, 256, 0, 320,/* after no used */352, 384, 416, 448, 0}, //48
    {0, 144, 216, 252, /* before no used */192, 240, 288, 336, 384, 0, 480,/* after no used */528, 576, 624, 672, 0}, //32
    {0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0, 0}     //0
};
#endif


//========================================================
// Decripton    : Check Valid Frame Header by Input String
// Parameter    : header    -> Expected data to a Frame Header.
// Return       : 0         -> Invalid Frame Header
//                1         -> Valid Frame Header
//========================================================
_ATTR_MP3DEC_TEXT_
unsigned char CheckValidHeader(unsigned char *header)
{
    // Check Frame Sync [ AAAAAAAA AAA ]
    if ( !((header[0] == 0xFF) && ((header[1] & 0xE0) == 0xE0)))
        return 0;

    // Check MPEG ID [ BB ]
    if ( (header[1] & 0x18) == 0x08 )           // Reserved 01
        return 0;

#define ENABLE_MP1 1
//by Vincent
#if ENABLE_MP1

    // Check Layer Description [ CC ]
    if ( ((header[1] & 0x06) == 0x00 )/*||            // Reserved 00
         ((header[1]&0x06) == 0x06 ) */)            // LayerI   11
        return 0;

#else

    // Check Layer Description [ CC ]
    if ( ((header[1] & 0x06) == 0x00 ) ||       // Reserved 00
         ((header[1] & 0x06) == 0x06 )  )       // LayerI   11
        return 0;

#endif

    // Check Bitrate Index [ EEEE ]
    if ( ((header[2] & 0xF0) == 0x00 ) ||       // Free 0000
         ((header[2] & 0xF0) == 0xF0 ) )        // Bad  1111
        return 0;

    // Check Sampling Rate Frequency [ FF ]
    if ( (header[2] & 0x0C) == 0x0C )           // Reserved 11
        return 0;

    return 1;
}

//=============================================================================
// Decripton    : Determines if the current frame is VBR header frame
//                and decodes it if it is. (Xing and VBRI)
// Parameter    : buffer    -> the first pointer position of the MP3 file
// Return       : 0         -> Fail to read the number of frame in Header
//                Other     -> The Number of Frame in Header
//========================================================
_ATTR_MP3DEC_TEXT_
unsigned long DecodeVBRHeader(unsigned char *buffer, unsigned int *framesize)
{
    int offset;
    unsigned char ucLayer;
    ucLayer         = (buffer[1]    & 0x06) >> 1;       // 0x00 : reserve
                                                        // 0x01 : Layer3
                                                        // 0x02 : Layer2
                                                        // 0x03 : Layer1

    switch (ucLayer)
    {
        case 0x01:
            frame_layer = 2;
            break;

        case 0x02:
            frame_layer = 1;
            break;

        case 0x03:
            frame_layer = 0;
            break;

        default :
            frame_layer = 2;
            break;
    }

    //To Find first position of Header(Xing, VBRI)
    if ((buffer[1] & 0x18) == 0x18) //MPEG Version 1
    {
        if ((buffer[3] & 0xc0) != 0xc0) offset = 36; //�ǵ�����
        else offset = 21;       //������

        *framesize = 1152;
        synth_out_samples_limit = 1152;
    }
    else //MPEG Version 2 or 2.5
    {
        if ((buffer[3] & 0xc0) != 0xc0) offset = 21; //�ǵ�����
        else offset = 13;       //������

        *framesize = 576;
        synth_out_samples_limit = 576;
    }

    //if VBRI,
    if ((buffer[offset] == 'X') && (buffer[offset + 1] == 'i') && (buffer[offset + 2] == 'n') && (buffer[offset + 3] == 'g'))
    {
        //if Xing,
        if (buffer[offset + 7] & 0x01) //Check Frames Flag
            return ( ((unsigned long)buffer[offset + 8] << 24)
                     | ((unsigned long)buffer[offset + 9] << 16)
                     | ((unsigned long)buffer[offset + 10] << 8)
                     | (unsigned long)buffer[offset + 11] ); //Read Number of Frame
    }

    offset = 36;

    if ((buffer[offset] == 'V') && (buffer[offset + 1] == 'B') && (buffer[offset + 2] == 'R') && (buffer[offset + 3] == 'I'))
    {
        return ( ((unsigned long)buffer[offset + 14] << 24)
                 | ((unsigned long)buffer[offset + 15] << 16)
                 | ((unsigned long)buffer[offset + 16] << 8)
                 | (unsigned long)buffer[offset + 17] ); //Read Number of Frames
    }

    return 0;
}

volatile _ATTR_MP3DEC_TEXT_
unsigned int CheckID3V2Tag(unsigned char *pucBuffer)
{
    // The first three bytes of the tag should be "ID3".
    if ((pucBuffer[0] != 'I') || (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return (0);
    }

    // The next byte should be the value 3 (i.e. we support ID3v2.3.0).
    //if(pucBuffer[3]   != 3)
    if (pucBuffer[3] < 2  && pucBuffer[3] > 4)
    {
        return (0);
    }

    // The next byte should be less than 0xff.
    if (pucBuffer[4] == 0xff)
    {
        return (0);
    }

    // We don't care about the next byte.  The following four bytes should be
    // less than 0x80.
    if ((pucBuffer[6] >= 0x80) || (pucBuffer[7] >= 0x80) ||
        (pucBuffer[8] >= 0x80) || (pucBuffer[9] >= 0x80))
    {
        return (0);
    }

    // Return the length of the ID3v2 tag.
    return ((pucBuffer[6] << 21) | (pucBuffer[7] << 14) |
            (pucBuffer[8] <<  7) |  pucBuffer[9]);
}
volatile _ATTR_MP3DEC_TEXT_
unsigned int CheckAPETag(unsigned   char *pucBuffer)
{
    unsigned int val, tag_bytes;
    // The first three bytes of the tag should be "APETAGEX".

    if (strncmp(pucBuffer, "APETAGEX", 8))
    {
        return 0;
    }

    val = ((pucBuffer[11] << 24) | (pucBuffer[10] << 16) |
           (pucBuffer[9] <<  8) |  pucBuffer[8]);        // APE tag version

    if (val > 2000)
    {
        return 0;
    }

    tag_bytes = ((pucBuffer[15] << 24)    | (pucBuffer[14] << 16) |
                 (pucBuffer[13] << 8)  |  pucBuffer[12]); // tag size

    if (tag_bytes - 32 > (1024 * 1024 * 16))
    {
        return 0;
    }

    // Return the length of the APE tag.
    return  tag_bytes;
}



_ATTR_MP3DEC_TEXT_
unsigned short GetFrameSize(unsigned char* pucBuffer)
{
    unsigned char ucMpegIDIdx;
    unsigned char ucSamplingIdx;
    unsigned char ucBitrateidx;
    unsigned char ucLayer;
    unsigned short usReturn;

    ucMpegIDIdx     = (pucBuffer[1] & 0x18) >> 1;       // 0x00 : Mpeg 2.5
                                                        // 0x08 : Mpeg 2
                                                        // 0x0C : Mpeg 1

    ucLayer         = (pucBuffer[1] & 0x06) >> 1;       // 0x00 : reserve
                                                        // 0x01 : Layer3
                                                        // 0x02 : Layer2
                                                        // 0x03 : Layer1

    switch (ucLayer)
    {
        case 0x01:
            frame_layer = 2;
            break;

        case 0x02:
            frame_layer = 1;
            break;

        case 0x03:
            frame_layer = 0;
            break;

        default :
            frame_layer = 2;
            break;
    }

    if (ucLayer == 0x03)                // layer1
        synth_out_samples_limit = 384;  /* layer I */
    else if (ucLayer == 0x01)           // layer3
    {
        //if (ucMpegIDIdx == 0x00)  // mpeg 2.5
        if ((ucMpegIDIdx == 0x00) || (ucMpegIDIdx == 0x08)) // mpeg 2.5 mpeg 2
            synth_out_samples_limit = 576;  /* layer III , MPEG 2.5 */
        else
            synth_out_samples_limit = 1152; /* layer III */
    }
    else
    {
        synth_out_samples_limit = 1152; /* layer III */
    }

    ucSamplingIdx   = (pucBuffer[2] & 0x0C) >> 2;       // 0x00 : 44.1  case Mpeg1
                                                        // 0x01 : 48
                                                        // 0x02 : 32
                                                        // 0x03 : reserve
    ucBitrateidx    = (pucBuffer[2] & 0xF0) >> 4;

    if (ucLayer == 0x01)
    {
        if (ucMpegIDIdx) ucMpegIDIdx -= 4;

        usReturn = usFLMap[ucMpegIDIdx  | ucSamplingIdx][ucBitrateidx] + ((pucBuffer[2] & 0x02) >> 1);
    }
    else if (ucLayer == 0x02)
    {
        if (ucMpegIDIdx) ucMpegIDIdx -= 8;

        usReturn = usFLMap2[ucMpegIDIdx | ucSamplingIdx][ucBitrateidx] + ((pucBuffer[2] & 0x02) >> 1);
    }
    /* Layer 1, add by Vincent */
    else
    {
        //usReturn = 136;   //only for test
        //usReturn = 168;   //only for test
        if (ucMpegIDIdx) ucMpegIDIdx -= 8;

        usReturn = usFLMap3[ucMpegIDIdx | ucSamplingIdx][ucBitrateidx] + ((pucBuffer[2] & 0x02) >> 1);
    }

    return usReturn;
}

_ATTR_MP3DEC_TEXT_
unsigned short GetSamplingRate(unsigned char* pucBuffer)
{
    unsigned char ucMpegIDIdx;
    unsigned char ucSamplingIdx;
    ucMpegIDIdx     = (pucBuffer[1] & 0x18) >> 1;
    ucSamplingIdx   = (pucBuffer[2] & 0x0C) >> 2;

    if (ucMpegIDIdx) ucMpegIDIdx -= 4;

    return usSRMap[ucMpegIDIdx | ucSamplingIdx];
}

_ATTR_MP3DEC_TEXT_
unsigned short GetBitrate(unsigned char* pucBuffer)
{
    unsigned char ucMpegIDIdx;
    unsigned char ucBitrateIdx;
    unsigned char ucLayer;
    ucMpegIDIdx     = (pucBuffer[1] & 0x18) >> 1;
    ucBitrateIdx    = (pucBuffer[2] & 0xF0) >> 4;
    //by Vincent
    ucLayer         = (pucBuffer[1] & 0x06) >> 1;       // 0x00 : reserve
    // 0x01 : Layer3
    // 0x02 : Layer2
    // 0x03 : Layer1

    if (ucMpegIDIdx == 0x0C)
    {
        if (ucLayer == 0x03)
            return usBRMapLayer1V1[ucBitrateIdx];   //layer 1
        else if (ucLayer == 0x02)
            return usBRMap2[ucBitrateIdx | 0x10];   //layer 2
        else
            return usBRMap[ucBitrateIdx | 0x10];
    }
    else
    {
        if (ucLayer == 0x03)
            return usBRMapLayer1V2[ucBitrateIdx];   //layer 1
        else if (ucLayer == 0x02)
            return usBRMap2[ucBitrateIdx];          //layer 2
        else
            return usBRMap[ucBitrateIdx];
    }
}

_ATTR_MP3DEC_TEXT_
unsigned long GetHeaderInfo(unsigned char* pucBuffer)
{
    unsigned long ulHeaderInfo;
    ulHeaderInfo =  (unsigned long)pucBuffer[0] << 24 |
                    (unsigned long)pucBuffer[1] << 16 |
                    (unsigned long)pucBuffer[2] << 8 |
                    (unsigned long)pucBuffer[3];
    return ulHeaderInfo;
}

_ATTR_MP3DEC_TEXT_
unsigned char GetChannels(unsigned char* pucBuffer)
{
    unsigned char ucChannel;
    ucChannel = (pucBuffer[3] & 0xC0) >> 6;

    if (ucChannel == 0x03)   ucChannel = 1;
    else                    ucChannel = 2;

    return ucChannel;
}

_ATTR_MP3DEC_TEXT_
unsigned char GetLayerDescription(unsigned char* pucBuffer)
{
    unsigned char ucLayer;
    ucLayer = (pucBuffer[1] & 0x06) >> 1;
    return ucLayer;
}

_ATTR_MP3DEC_TEXT_
int SearchTwoHeader(unsigned char* pucBuffer, unsigned long ulInputHeaderInfo)
{
    unsigned char* InputBuffer = 0;
    unsigned short searchcount = 1440;
    unsigned char returnvalue = 0;
    unsigned long ulNextHeader = 0;
    unsigned long ulSumOfBitrate = 0;
    unsigned long temp = 0;
    unsigned long ulCheckHeaderInfo;
    int result;
    InputBuffer = pucBuffer;

    while (searchcount)
    {
        returnvalue = CheckValidHeader(InputBuffer);

        if (returnvalue)
        {
            ulCheckHeaderInfo = GetHeaderInfo(InputBuffer);

            if ((ulCheckHeaderInfo & 0x001E0C00) == (ulInputHeaderInfo & 0x001E0C00))
            {
                // get next header position
                ulNextHeader    = GetFrameSize(InputBuffer);
                // save first bitrate
                ulSumOfBitrate  = GetBitrate(InputBuffer);
                // increase header position
                temp = (int)InputBuffer + ulNextHeader - (int)pucBuffer;

                if ( temp >= 2044)
                {
                    InputBuffer = pucBuffer;
                    memcpy(InputBuffer, InputBuffer + 2044, 4);
                    result = RKFIO_FRead(InputBuffer + 4, 0x600, pRawFileCache);
                    //if(result<=0) return -1;
                    InputBuffer += temp - 2044;
                }
                else
                {
                    InputBuffer += ulNextHeader;
                }

                // check next header
                returnvalue     = CheckValidHeader(InputBuffer);

                if (returnvalue)
                {
                    ulSumOfBitrate += GetBitrate(InputBuffer);
                    return ulSumOfBitrate;
                }
            }
        }

        searchcount--;
        InputBuffer++;
    }

    return 0;
}


//========================================================
// Decripton    : Search MP3 Header position
// Parameter    : MP3Data       -> String pointer of working data
// Return       : -1            -> Invalid MP3 file
//                signed digit  -> Positoin of first frame header
//========================================================
_ATTR_MP3DEC_DATA_
int firstframe = 0;
_ATTR_MP3DEC_DATA_
unsigned int ape_tag_size = 0;
_ATTR_MP3DEC_TEXT_
int GetHeaderPosition(tMP3 *pMP3)
{
    unsigned char* MP3Data;
    unsigned int firstheaderposition = 0;
    //unsigned int  searchcount = 100*1024;//1024*1024;//1000;
    unsigned int  searchcount = 100 * 1024; //1024*1024;//1000;
    unsigned long ulFirstHeaderInfo;
    unsigned long ulSecondHeaderInfo;
    unsigned long ulNextHeader;
    unsigned long ulFramesize;
    unsigned long ulTotalframes;
    unsigned long ulFirstBitrate;
    float ulTotalTime; //add by wjr
    long temp;
    unsigned char returnvalue;
    int result;
    unsigned char VBR_Flag = 0;
    unsigned char   comparecount = 0, ulBitrateCount = 0;
    ulAverageBitrate = 0;
    MP3Data = (unsigned char*) pENCODED_DATA;
    // Check ape tag
    //////////////////////////////////////////////////////////////////////////////
    RKFIO_FSeek(RKFIO_FLength(pRawFileCache)-32,0 , pRawFileCache);
    result = RKFIO_FRead(MP3Data , 32 , pRawFileCache);
    ape_tag_size = CheckAPETag(MP3Data);
    mp3_printf("apetag = %d\n",ape_tag_size);
    //ape_tag_size = 0;
    RKFIO_FSeek( 0, 0 , pRawFileCache);
    // Read mp3 data
    //////////////////////////////////////////////////////////////////////////////
    //RKFIO_FSeek( 0,0 , pRawFileCache);
    result = RKFIO_FRead(MP3Data , 2048 , pRawFileCache);
    // Check ID3 TAG v2.x
    //////////////////////////////////////////////////////////////////////////////
   // while()
    firstheaderposition = CheckID3V2Tag(MP3Data);
    mp3_printf("0x%x\n",firstheaderposition);

    if (firstheaderposition)
    {
        while (firstheaderposition)
        {
            firstheaderposition += 10;
            firstframe += firstheaderposition;

            //  mp3_printf("֡��ʼλ�� %d ",firstframe);
            //mp3_printf("pos = %d,gyu = %d  \n",firstframe,firstheaderposition&~511);
            if ( firstheaderposition > 0xC00 )
            {
                RKFIO_FSeek( firstframe & ~511, 0 , pRawFileCache);
                result = RKFIO_FRead( MP3Data , 2048 , pRawFileCache );
                MP3Data += firstframe & 511;
            }
            else if ( firstheaderposition > 0x600 )
            {
                memcpy(MP3Data, MP3Data + 0x600, 0x200);
                result = RKFIO_FRead(MP3Data + 0x200, 0x600, pRawFileCache );
                MP3Data += firstheaderposition - 0x600;
            }
            else
            {
                MP3Data += firstheaderposition;
            }

            firstheaderposition = CheckID3V2Tag(MP3Data);
        }

        firstheaderposition = firstframe;
        returnvalue = CheckValidHeader(MP3Data);

        if (!returnvalue)
        {
            int search_bytes = 2044;
            unsigned char   *buffer = MP3Data;
            int  i = 0;
            int vbr_flag = 0;

            if ((firstframe - 1024) > 0)
                RKFIO_FSeek(firstframe - 1024 , 0 , pRawFileCache);
            else
                RKFIO_FSeek(0 , 0 , pRawFileCache);

            result = RKFIO_FRead( MP3Data , 2048 , pRawFileCache );

            while (search_bytes--)
            {
                if ((buffer[i] == 'X') && (buffer[i + 1] == 'i') && (buffer[i + 2] == 'n') && (buffer[i + 3] == 'g'))
                {
                    vbr_flag = 1;
                    break;
                }
                else if ((buffer[i] == 'V') && (buffer[i + 1] == 'B') && (buffer[i + 2] == 'R') && (buffer[i + 3] == 'I'))
                {
                    vbr_flag = 1;
                    break;
                }

                i++;
            }

            if (vbr_flag)
            {
                if ((firstframe - 1024) > 0)
                {
                    firstframe = firstframe - 1024 + i - 36;//36 --- MPEG-1 and non- mono used
                }
                else
                {
                    firstframe = i - 36;
                }

                RKFIO_FSeek(firstframe , 0 , pRawFileCache);
                result = RKFIO_FRead( MP3Data , 2048 , pRawFileCache );
                mp3_printf("id3 contain half VBR frame");
            }
            else
            {
                RKFIO_FSeek(firstframe , 0 , pRawFileCache);
                result = RKFIO_FRead( MP3Data , 2048 , pRawFileCache );
            }
        }
    }
    else
        firstframe = 0;

    //=============================================================================

    // If valid header, check 20 times
    // else, check 1000 times. exept all 0xFF, 0x00
    while (searchcount)
    {
        returnvalue = CheckValidHeader(MP3Data);

        if (returnvalue)
        {
            // check Xing & VBRI =============  VBR=============================================
            ulTotalframes = DecodeVBRHeader(MP3Data, (unsigned int *)&ulFramesize);

            if (ulTotalframes)
            {
                // VBR Flag setting
                VBR_Flag            = 1;
                // get header information
                pMP3->ucIsVBR       = 1;
                pMP3->ucChannels    = GetChannels(MP3Data);
                pMP3->usSampleRate  = GetSamplingRate(MP3Data);
                pMP3->ulFirstFrame  = firstheaderposition;
                pMP3->ulLength      = (unsigned long)RKFIO_FLength(pRawFileCache) - pMP3->ulFirstFrame - ape_tag_size;
#if 1
                ulTotalTime = (float)ulTotalframes * ulFramesize / pMP3->usSampleRate;
                pMP3->ulBitRate  = (float)pMP3->ulLength * 8 / ulTotalTime;
#else
                //pMP3->ulBitRate        = ((pMP3->ulLength / ulTotalframes) *
                //pMP3->usSampleRate)   / (ulFramesize / 8);
                pMP3->ulBitRate     = ((pMP3->ulLength / ulTotalframes) *
                                       pMP3->usSampleRate) / (ulFramesize / 8);
#endif
                //mp3_printf("\n ������ = %d �ļ����ֽ�  =%d ��֡�� %d ֡��С %d������ =%d \n",pMP3->usSampleRate,pMP3->ulLength,ulTotalframes,ulFramesize,pMP3->ulBitRate);
                //  mp3_printf("��ʱ��%d \n",ulTotalframes*ulFramesize /pMP3->usSampleRate);


                if(pMP3->ulBitRate == 0)
                {
                   // mp3_DEBUG();
                    return 0;
                }
                return 1;
            }
            //=============================================================================

            // get next header pointer ====================================================
            // get first header raw data
            ulFirstHeaderInfo   = GetHeaderInfo(MP3Data);
            // calculate next header position
            ulNextHeader        = GetFrameSize(MP3Data);
            // get first bitrate
            ulFirstBitrate      = GetBitrate(MP3Data);

            pMP3->ulBitRate     = ulFirstBitrate * 1000;

            // jump to next header
            temp = (int)MP3Data + ulNextHeader - (int)pENCODED_DATA;

            if ( temp >= 2044)
            {
                MP3Data = (unsigned char*) pENCODED_DATA;
                memcpy(MP3Data, MP3Data + 2044, 4);
                result = RKFIO_FRead(MP3Data + 4, 0x600 , pRawFileCache);
//              if(result<=0) return -1;
                MP3Data += temp - 2044;
            }
            else
            {
                MP3Data += ulNextHeader;
            }

            //========================CBR=====================================================
            // check next header
            returnvalue = CheckValidHeader(MP3Data);

            if (returnvalue)
            {
                // get second header raw data
                ulSecondHeaderInfo = GetHeaderInfo(MP3Data);

                // compare previous and current header
                if ( (ulFirstHeaderInfo & 0x001E0C03) == (ulSecondHeaderInfo & 0x001E0C03) )
                {
                    // check MP2 file or not
                    //  if( GetLayerDescription(MP3Data) == 0x02 ) return 2;
                    // get header information
                    pMP3->ucChannels    = GetChannels(MP3Data);
                    pMP3->usSampleRate  = GetSamplingRate(MP3Data);
                    pMP3->ulFirstFrame  = firstheaderposition;
                    pMP3->ulLength      = (unsigned long)RKFIO_FLength(pRawFileCache) - pMP3->ulFirstFrame - ape_tag_size;
                    // mp3_printf("\n�ļ���12 =%d \n",pMP3->ulFirstFrame);
#if 1
//�������������ʣ����򲿷ָ�����ʱ��ͱ�������ʾ��׼
                    // check 20 header's bitrate ===================================================
                    ulBitrateCount = 0;

                    for (comparecount = 0; comparecount < 20; comparecount++)
                    {
                        // jump file position
                        MP3Data = (unsigned char*) pENCODED_DATA;

                        /*�˴�Ҫע��:����Ҫ�ӵ�һ֡��ʼ��λ��ȥ���ң������Ǹ����ļ���Сȥ����Ĳ���*/
                        RKFIO_FSeek(((int) ((pMP3->ulLength * (5 + comparecount)) / 30) & ~511) + firstframe, 0 , pRawFileCache );
                        result = RKFIO_FRead(MP3Data, 2048, pRawFileCache);

                        if (result <= 0) return -1;

                        // get header value
                        temp = SearchTwoHeader(MP3Data, ulFirstHeaderInfo);

                        if (temp < 0) return -1;

                        if (temp > 0)
                        {
                            ulAverageBitrate += temp;
                            ulBitrateCount += 2;
                        }

                        if(temp == 0)
                        {
                           ;// mp3_DEBUG();
                        }
                    }

                    // =============================================================================
                    // compare average bitrate and first bitrate value

                    if (ulBitrateCount)
                    {
                        ulAverageBitrate = ulAverageBitrate / ulBitrateCount;
                    }
                    else
                    {
                       // ulAverageBitrate = 0;
                      // mp3_DEBUG();
                        return 0;
                    }

#endif

                    if (ulFirstBitrate != ulAverageBitrate)
                    {
                        VBR_Flag            = 2;                        // Unknown VBR
                        pMP3->ucIsVBR       = 1;
                        pMP3->ulBitRate     = ulAverageBitrate * 1000;
                    }
                    else
                    {
                        VBR_Flag            = 0;                        // CBR
                        pMP3->ucIsVBR       = 0;
                        pMP3->ulBitRate     = ulFirstBitrate * 1000;
                    }

                    return 1;
                }
            }

            // The first searched header is not valid header
            // MP3Data pointer back to first header
            MP3Data = (unsigned char*) pENCODED_DATA;
            RKFIO_FSeek( firstheaderposition &  ~511, 0 , pRawFileCache);
            result = RKFIO_FRead(MP3Data, 2048, pRawFileCache);
//          if(result<=0) return -1;
            MP3Data += firstheaderposition & 511;
        }

        // increase pointer and another variables =====================================
        //if(*MP3Data   != 0xFF && *MP3Data != 0x00)
        searchcount--;

        // If buffer empty, read 512byte from mp3 file
        if ( (int)MP3Data - (int)pENCODED_DATA   >= 0x600)
        {
            MP3Data = (unsigned char*) pENCODED_DATA;
            memcpy(MP3Data, MP3Data + 0x600, 0x200);
            result = RKFIO_FRead(MP3Data + 0x200, 0x600, pRawFileCache);

            if (result <= 0) return -1;

            //  MP3Data -= 0x600;
        }

        // else, update pointer
        firstheaderposition++;
        MP3Data++;
        //=============================================================================
    }
    return 0;
}
#endif
#endif

