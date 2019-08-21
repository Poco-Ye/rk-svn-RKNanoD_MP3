/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     wma_parse.c
*  Description:    get the id3 information of wma format song.
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     07/11/05      1.0
*
*******************************************************************************/
#include "../include/audio_main.h"
#ifdef WMA_DEC_INCLUDE
#pragma arm section code = "WmaId3Code", rodata = "WmaId3Code", rwdata = "WmaId3Data", zidata = "WmaId3Bss"

//#include <stdio.h>

#include "wma_parse.h"
#include "..\..\Include\audio_file_access.h"

#define MIN_OBJECT_SIZE     24
#define DATA_OBJECT_SIZE    50

#define LOAD_DWORD(p) ((p)[0] | ((p)[1]<<8) | ((p)[2]<<16) | ((p)[3]<<24) )
#define LOAD_WORD(p)  ((p)[0] | ((p)[1]<<8))

#define ECD_STRING 0

#define ECD_STRING 0

#define ECD_INT 3

sByteStreamCtl ByteStreamCtl;

AP_FILE APFileHandle;

 unsigned char AsfHeaderObjectV0[16] =
{
    0x30, 0x26,0xb2,0x75,0x8e,0x66,0xcf,0x11,0xa6,0xd9,0x00,0xaa,0x00,0x62,0xce,0x6c
};

 unsigned char AsfContentDescriptionObjectV0[16] =
{
    0x33,0x26,0xb2,0x75,0x8e,0x66,0xcf,0x11,0xa6,0xd9,0x00,0xaa,0x00,0x62,0xce,0x6c
};

 unsigned char AsfExtendedContentDescObject[16] =
{
    0x40,0xa4,0xd0,0xd2,0x07,0xe3,0xd2,0x11,0x97,0xf0,0x0,0xa0,0xc9,0x5e,0xa8,0x50
};

 unsigned short WM_AlbumTitle[14] =
{
    'W','M','/','A','l','b','u','m','T','i','t','l','e','\0'
};

unsigned short WM_Genre[9] =
{
    'W','M','/','G','e','n','r','e','\0'
};

unsigned short WM_TrackNo[15] =
{
    'W','M','/','T','r','a','c','k','N','u','m','b','e','r','\0'
};

int AP_FileRead(char *buffer, long cbLen, AP_FILE fhandle)
{
    return RKFIO_FRead(buffer, cbLen , (FILE *)fhandle);
    //return fread(buffer, 1, cbLen, (FILE *)fhandle);
}

void InitByteStream(sByteStreamCtl *pByteStreamCtl)
{
    pByteStreamCtl->cbOffset = STREAM_BUF_SIZE;
    pByteStreamCtl->cbSize = STREAM_BUF_SIZE;
}

void FillByteStream(sByteStreamCtl *pByteStreamCtl)
{
    int cbRead;

    if(pByteStreamCtl->cbOffset > STREAM_BUF_SIZE)
    {
        pByteStreamCtl->cbOffset = 0;
    }

    //copy the left data to the start postion of stream.
    if(pByteStreamCtl->cbOffset > 0)
        memcpy(pByteStreamCtl->Stream, pByteStreamCtl->Stream + pByteStreamCtl->cbOffset, pByteStreamCtl->cbSize - pByteStreamCtl->cbOffset);

    //then to fill the stream by the data readed from file.
    cbRead = AP_FileRead(pByteStreamCtl->Stream + pByteStreamCtl->cbSize - pByteStreamCtl->cbOffset, pByteStreamCtl->cbOffset, APFileHandle);
    if(cbRead != pByteStreamCtl->cbOffset)
    {
        pByteStreamCtl->cbSize = pByteStreamCtl->cbSize - pByteStreamCtl->cbOffset + cbRead;
    }
    pByteStreamCtl->cbOffset = 0;
}

// 0: enough, 1: not enough; 2: file had read over.
int ByteStreamShowByte(sByteStreamCtl *pByteStreamCtl, int needByte)
{
    if(needByte > pByteStreamCtl->cbSize)
    {
        return 0;
        //while(1); //Do not allow this situation happen.
    }

    if(needByte > pByteStreamCtl->cbSize - pByteStreamCtl->cbOffset)
    {
        if(pByteStreamCtl->cbSize < STREAM_BUF_SIZE)
        {
            return 2;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char *ByteStreamGetBuf(sByteStreamCtl *pByteStreamCtl, int needByte)
{
    unsigned char *buf;

    buf = (unsigned char *)(pByteStreamCtl->Stream + pByteStreamCtl->cbOffset);
    pByteStreamCtl->cbOffset += needByte;
    return buf;
}

void ByteSteamSkip(long cbSkip)
{
    while(cbSkip >= STREAM_BUF_SIZE)
    {
        if(ByteStreamShowByte(&ByteStreamCtl, STREAM_BUF_SIZE) == 1)
        {
            FillByteStream(&ByteStreamCtl);
        }
        ByteStreamGetBuf(&ByteStreamCtl, STREAM_BUF_SIZE);
        cbSkip -= STREAM_BUF_SIZE;
    }

    if(cbSkip > 0)
    {
        if(ByteStreamShowByte(&ByteStreamCtl, cbSkip) == 1)
        {
            FillByteStream(&ByteStreamCtl);
        }
        ByteStreamGetBuf(&ByteStreamCtl, cbSkip);
    }
}

int WMA_LoadExtendedContentDesc_Object(long *pcbConsumed, long currPacketOffset, long cbObject, ID3 *id3)
{
    long cbWanted;
    short cDescriptors =0;
    short cbName, data_type;
    long cbValue;
    long iCurrFileOffset;
    long iMaxFileOffset;
    int i;
    unsigned char *p;
    int count = 0;

    //assert (cbSize >= MIN_OBJECT_SIZE);
    iCurrFileOffset = 0;
    iMaxFileOffset = iCurrFileOffset + cbObject - MIN_OBJECT_SIZE;

    // Get descriptor count
    cbWanted = sizeof(short);
    if (iCurrFileOffset + cbWanted > iMaxFileOffset)
    {
        return AP_FAIL;
    }

    if(ByteStreamShowByte(&ByteStreamCtl, cbWanted) == 1)
    {
        FillByteStream(&ByteStreamCtl);
    }

    p = ByteStreamGetBuf(&ByteStreamCtl, cbWanted);
    cDescriptors = LOAD_WORD(p);
    iCurrFileOffset += cbWanted;

    // Read in each descriptor record
    for (i = 0; i < cDescriptors; i++)
    {
        int fAlbumFind = 0;
        int fGenreFind = 0;
        int fTrackFind = 0;

        // Load in descriptor name length
        cbWanted = sizeof(short);
        if (iCurrFileOffset + cbWanted > iMaxFileOffset)
        {
            return AP_FAIL;
        }

        if(ByteStreamShowByte(&ByteStreamCtl, cbWanted) == 1)
        {
            FillByteStream(&ByteStreamCtl);
        }

        p = ByteStreamGetBuf(&ByteStreamCtl, cbWanted);
        cbName = LOAD_WORD(p);
        iCurrFileOffset += cbWanted;

        if(ByteStreamShowByte(&ByteStreamCtl, cbName + 2*sizeof(short)) == 1)
        {
            FillByteStream(&ByteStreamCtl);
        }

        // Load in descriptor name
        if(0 == memcmp(WM_AlbumTitle, ByteStreamGetBuf(&ByteStreamCtl, 0), sizeof(WM_AlbumTitle)))
        {
            fAlbumFind = 1; //find the album titile then read out them.
        }
        else if(0 == memcmp(WM_Genre, ByteStreamGetBuf(&ByteStreamCtl, 0), sizeof(WM_Genre)))
        {
            fGenreFind = 1; // find the genre information.
        }
        else if(0 == memcmp(WM_TrackNo, ByteStreamGetBuf(&ByteStreamCtl, 0), sizeof(WM_TrackNo)))
        {
            fTrackFind = 1;

        }

        iCurrFileOffset += cbName;
        ByteStreamGetBuf(&ByteStreamCtl, cbName);

        // Load in descriptor value type, and descriptor value length
        cbWanted = 2 * sizeof(short);
        if (iCurrFileOffset + cbWanted > iMaxFileOffset)
        {
          return AP_FAIL;
        }

        p = ByteStreamGetBuf(&ByteStreamCtl, 2);
        data_type = LOAD_WORD(p);
        p = ByteStreamGetBuf(&ByteStreamCtl, 2);
        cbValue = LOAD_WORD(p);
        iCurrFileOffset += cbWanted;

        if(ByteStreamShowByte(&ByteStreamCtl, cbValue) == 1)
        {
            FillByteStream(&ByteStreamCtl);
        }

        // Load in descriptor value
        if (ECD_STRING == data_type)
        {
            if(fAlbumFind)
            {
                cbWanted = cbValue > (MEDIA_ID3_SAVE_CHAR_NUM * 2) ? (MEDIA_ID3_SAVE_CHAR_NUM * 2):cbValue;
                memcpy(id3->Album, ByteStreamGetBuf(&ByteStreamCtl, 0), cbWanted);
                cbWanted >>= 1; // char to wchar, so /2
                id3->Album[cbWanted-1] = 0; // null

                count ++;
            }
            else if(fGenreFind)
            {
                cbWanted = cbValue > (MEDIA_ID3_SAVE_CHAR_NUM * 2) ? (MEDIA_ID3_SAVE_CHAR_NUM * 2):cbValue;
                memcpy(id3->Genre, ByteStreamGetBuf(&ByteStreamCtl, 0), cbWanted);
                cbWanted >>= 1; // char to wchar, so /2
                id3->Genre[cbWanted-1] = 0; // null

                count ++;
            }

        }
        else if(ECD_INT == data_type)
        {
            if(fTrackFind)
            {
                cbWanted = cbValue > (MEDIA_ID3_SAVE_CHAR_NUM * 2) ? (MEDIA_ID3_SAVE_CHAR_NUM * 2):cbValue;
                memcpy(id3->Track, ByteStreamGetBuf(&ByteStreamCtl, 0), cbWanted);
                cbWanted >>= 1; // char to wchar, so /2
                id3->Track[cbWanted-1] = 0; // null
                count ++;
            }
        }
        iCurrFileOffset += cbValue;
        //ByteStreamGetBuf(&ByteStreamCtl, cbValue);
        ByteSteamSkip(cbValue);

        if(count >= 3)
        {
            break;
        }
    } // for

    *pcbConsumed = iCurrFileOffset;

    return AP_SUCESS;
}

int WMA_LoadContentDescription_Object(long *pcbConsumed, long currPacketOffset, long cbObject, ID3 *id3)
{
    long iCurrFileOffset;
    long iMaxFileOffset;
    long cbWanted;
    short title_len, author_len;
    unsigned char *p;

    //assert (cbSize >= MIN_OBJECT_SIZE);
    iCurrFileOffset = 0;
    iMaxFileOffset = iCurrFileOffset + cbObject - MIN_OBJECT_SIZE;

    cbWanted = 5*sizeof(short);
    if (iCurrFileOffset + cbWanted > iMaxFileOffset)
    {
        return AP_FAIL;
    }

    if(ByteStreamShowByte(&ByteStreamCtl, cbWanted) == 1)
    {
        FillByteStream(&ByteStreamCtl);
    }

    p = ByteStreamGetBuf(&ByteStreamCtl, 2);
    title_len = LOAD_WORD(p);
    p = ByteStreamGetBuf(&ByteStreamCtl, 2);
    author_len = LOAD_WORD(p);
    //copyright_len = LOAD_WORD(pData+iCurrFileOffset);
    //description_len = LOAD_WORD(pData+iCurrFileOffset);
    //rating_len = LOAD_WORD(pData+iCurrFileOffset);

    iCurrFileOffset += cbWanted;
    ByteSteamSkip(cbWanted - 4);

    if(ByteStreamShowByte(&ByteStreamCtl, title_len) == 1)
    {
        FillByteStream(&ByteStreamCtl);
    }

    // Get Title
    cbWanted = title_len > (MEDIA_ID3_SAVE_CHAR_NUM * 2) ? (MEDIA_ID3_SAVE_CHAR_NUM * 2):title_len;
    memcpy(id3->Title, ByteStreamGetBuf(&ByteStreamCtl, cbWanted), cbWanted);
    id3->Title[MEDIA_ID3_SAVE_CHAR_NUM-1] = 0; // null

    iCurrFileOffset += title_len;
    ByteSteamSkip(title_len - cbWanted);

    if(ByteStreamShowByte(&ByteStreamCtl, author_len) == 1)
    {
        FillByteStream(&ByteStreamCtl);
    }

    // Get Author
    cbWanted = author_len > (MEDIA_ID3_SAVE_CHAR_NUM * 2) ? (MEDIA_ID3_SAVE_CHAR_NUM * 2):author_len;
    memcpy(id3->Author, ByteStreamGetBuf(&ByteStreamCtl, cbWanted), cbWanted);
    id3->Author[MEDIA_ID3_SAVE_CHAR_NUM-1] = 0; // null

    iCurrFileOffset += author_len;
    ByteSteamSkip(author_len - cbWanted);

    *pcbConsumed = iCurrFileOffset;

    return AP_SUCESS;
}

int WMA_ParseAsf_Header(AP_FILE fhandle,  ID3 *id3)
{
    int wmarc;
    unsigned char *pObjId;
    long objSize, cbConsumed;
    long currPacketOffset, cbHeader, cbFirstPacketOffset;
    int ContentOK = 0;
    unsigned char *p;

    APFileHandle = fhandle;

    memset(id3, 0, sizeof(ID3));

    /* initialize the some state */

    currPacketOffset = 0;

    InitByteStream(&ByteStreamCtl);
    FillByteStream(&ByteStreamCtl);

    if(ByteStreamShowByte(&ByteStreamCtl, 30) == 2)
    {
        return AP_FAIL;
    }

    /* ASF Header Object */

    if(0 != memcmp(AsfHeaderObjectV0, ByteStreamGetBuf(&ByteStreamCtl, 16), 16))
    {
        return AP_FAIL;
    }

    currPacketOffset += 16;
    p = ByteStreamGetBuf(&ByteStreamCtl, 14);
    cbHeader = LOAD_DWORD(p);
    cbFirstPacketOffset = cbHeader + DATA_OBJECT_SIZE;

    currPacketOffset = 30;

    /* Scan Header Objects */

    while(currPacketOffset < cbHeader)
    {
        if(ContentOK >= 2)
        {
            break; //it means find id3 information,do not necessary to decede continue.
        }

        if(ByteStreamShowByte(&ByteStreamCtl, 16+8) == 1)
        {
            FillByteStream(&ByteStreamCtl);
        }

        pObjId = ByteStreamGetBuf(&ByteStreamCtl, 16);
        p = ByteStreamGetBuf(&ByteStreamCtl, 8);
        objSize = LOAD_DWORD(p);
        currPacketOffset += MIN_OBJECT_SIZE;

        if(0 == memcmp(AsfContentDescriptionObjectV0, pObjId, 16))
        {
            if (currPacketOffset + (objSize - MIN_OBJECT_SIZE) > cbFirstPacketOffset)
            {
                return AP_FAIL;
            }

            wmarc = WMA_LoadContentDescription_Object(&cbConsumed, currPacketOffset, objSize, id3);
            if (AP_SUCESS != wmarc)
            {
                return AP_FAIL;
            }
            if(cbConsumed > objSize - MIN_OBJECT_SIZE)
            {
                return AP_FAIL;
            }

            ByteSteamSkip(objSize - MIN_OBJECT_SIZE - cbConsumed);

            currPacketOffset += objSize - MIN_OBJECT_SIZE;
            ContentOK += 1;
        }
        else if(0 == memcmp(AsfExtendedContentDescObject, pObjId, 16))
        {
            if (currPacketOffset + (objSize - MIN_OBJECT_SIZE) > cbFirstPacketOffset)
            {
                return AP_FAIL;
            }

            wmarc = WMA_LoadExtendedContentDesc_Object(&cbConsumed, currPacketOffset, objSize, id3);
            if (AP_SUCESS != wmarc)
            {
                return AP_FAIL;
            }
            if(cbConsumed > objSize - MIN_OBJECT_SIZE)
            {
                return AP_FAIL;
            }

            ByteSteamSkip(objSize - MIN_OBJECT_SIZE - cbConsumed);

            currPacketOffset += objSize - MIN_OBJECT_SIZE;
            ContentOK += 1;
        }
        else
        {
            /* skip over this object */
            ByteSteamSkip(objSize - MIN_OBJECT_SIZE);
            currPacketOffset += objSize - MIN_OBJECT_SIZE;
        }
    }

    return AP_SUCESS;
}

#pragma arm section code
#endif
