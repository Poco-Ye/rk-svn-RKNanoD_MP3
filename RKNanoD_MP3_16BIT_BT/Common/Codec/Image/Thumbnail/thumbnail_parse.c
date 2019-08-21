/************************************************************************
 *      Thumbnail Parser
 *
 * File:
 *      thumbnail_parse.c
 *
 * Author:
 *      CTF
************************************************************************/
#include "..\ImageInclude\image_main.h"
#include "..\ImageInclude\image_globals.h"

#ifdef THUMB_DEC_INCLUDE

#pragma arm section code = "ImageContrlCode", rodata = "ImageContrlCode", rwdata = "ImageContrlData", zidata = "ImageContrlBss"

#include "FsInclude.h"
#include "SysFindFile.h"
#include "file.h"
#include "ID3.h"
#include "PowerManager.h"
#include "LcdInterface.h"

#include "ImageControl.h"
#include "pJPG.h"
#include "thumbnail_parse.h"

#include "Exif_Read_IF.h"   /* sanshin EXIF */
#include "thumbnail_parse.h"
#include "Thumbnail_Read_IF.h"

#define BUFFER_SIZE     512

static int gPicFileType = NONE_PIC;
static int gDisplayBackground = 0;
static int gFirstOffset = 0;
static unsigned char gJpgOutputBuf[1024 * 4];
static IMAGE_PIXEL gScreenBuffer[IMAGE_MAX_OUTPUT_WIDTH];
static IMAGE_DEC_INFO gJpgDecInfo;

extern int JpgInit(IMAGE_DEC_INFO* pdec_info);
extern int JpgDecode(IMAGE_DEC_INFO* pdec_info);
extern int JpgDecCalcuOutput(void);

//---->sanshin_20151026
#define thumb_KEY_STATUS_SHORT_UP         ((UINT32)0x0004 << 28)      //0x4000,0000
#define thumb_KEY_STATUS_DOWN             ((UINT32)0x0001 << 28)      //0x1000,0000
#define thumb_KEY_VAL_FFW_SHORT_UP        ((KEY_VAL_FFW)|(thumb_KEY_STATUS_SHORT_UP))
#define thumb_KEY_VAL_FFD_SHORT_UP        ((KEY_VAL_FFD)|(thumb_KEY_STATUS_SHORT_UP))
#define thumb_KEY_VAL_FFW_DOWN            ((KEY_VAL_FFW)|(thumb_KEY_STATUS_DOWN))
#define thumb_KEY_VAL_FFD_DOWN            ((KEY_VAL_FFD)|(thumb_KEY_STATUS_DOWN))
//---->sanshin_20151026

typedef union
{
    struct
    {
        unsigned int    bKeyHave:            1,     //flag key is valid
                        bKeyDown:            1,     //set active if any key down.
                        bKeyHold:            1,     //flag hold status.
                        bReserved:           29;
    }bc;
    unsigned int word;
} thumb_KEY_FLAG;
extern thumb_KEY_FLAG            KeyFlag;
//<----sanshin_20151026

static unsigned int CheckID3V2Tag(unsigned  char *pucBuffer)
{
    // The first three bytes of the tag should be "ID3".
    if ((pucBuffer[0] !=    'I') || (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next byte should be the value 3 (i.e. we support ID3v2.3.0).
    if(pucBuffer[3] != 3)
    //if ((pucBuffer[3] < 3) && (pucBuffer[3] > 4))
    {
        return(0);
    }

    // The next byte should be less than 0xff.
    if (pucBuffer[4] == 0xff)
    {
        return(0);
    }

    // We don't care about the next byte.  The following four bytes should be
    // less than 0x80.
    if ((pucBuffer[6] >= 0x80) || (pucBuffer[7] >= 0x80)    ||
            (pucBuffer[8] >= 0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of the ID3v2 tag.
    return((pucBuffer[6] << 21) | (pucBuffer[7] << 14) |
           (pucBuffer[8] <<  7) |  pucBuffer[9]);
}

//output: ID3 APIC FrameSize.
static int ID3V23GetPicInfo(FILE* hFile, int* picType, int ID3Length)
{
    int i;
    int RemainSize = 0;
    int ReadSize = 0;
    int FrameSize = 0;
    int ImageOffset = 0;

    UINT8 image_tag[6] = {"0"};
    UINT8 pic_type[4] = {"0"};

    ID3V2XFrameInfoType FrameBuf;
    memset(&FrameBuf, 0, sizeof(ID3V2XFrameInfoType));

    RemainSize = ID3Length;

    ReadSize = FileRead((UINT8*)&FrameBuf, 10, (HANDLE)hFile);
    if(ReadSize <= 0)
    {
        DEBUG("=====FrameBuf FileRead Error=====\n");
        return 0;
    }
    RemainSize -= ReadSize;

    //查找图片标签所在位置
    while(RemainSize > 0)
    {
        if(strncmp(FrameBuf.mFrameID, "APIC", 4) == 0)
        {
            break;
        }
        else if((strncmp(FrameBuf.mFrameID, "AENC", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "ASPI", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "COMM", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "COMR", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "ENCR", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "EQUA", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "ETCO", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "GEOB", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "GRID", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "IPLS", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "LINK", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "MCDI", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "MLLT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "OWNE", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "PRIV", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "PCNT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "POPM", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "POSS", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "RBUF", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "RVAD", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "RVRB", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "SEEK", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "SIGN", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "SYLT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "SYTC", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TALB", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TBPM", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TCOM", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TCON", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TCOP", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TDAT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TDEN", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TDLY", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TDOR", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TDRC", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TDRL", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TDTG", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TENC", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TEXT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TFLT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TIME", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TIPL", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TIT1", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TIT2", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TIT3", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TKEY", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TLAN", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TLEN", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TMED", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TMOO", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TOAL", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TOFN", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TOLY", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TOPE", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TORY", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TOWN", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TPE1", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TPE2", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TPE3", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TPE4", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TPOS", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TPRO", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TPUB", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TRCK", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TRDA", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TRSN", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TRSO", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSIZ", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSRC", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSOA", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSOC", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSOT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSOP", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSO2", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSSE", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TSST", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TYER", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "TXXX", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "UFID", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "USER", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "USLT", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WCOM", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WCOP", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WOAF", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WOAR", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WOAS", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WORS", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WPAY", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WPUB", 4) == 0)
                    || (strncmp(FrameBuf.mFrameID, "WXXX", 4) == 0))
        {
            FrameSize = FrameBuf.mSize[0] * 0x1000000
                + FrameBuf.mSize[1] * 0x10000
                + FrameBuf.mSize[2] * 0x100
                + FrameBuf.mSize[3];

            FileSeek(FrameSize, SEEK_CUR, (HANDLE)hFile);
            RemainSize -= FrameSize;

            ReadSize = FileRead((UINT8*)&FrameBuf, 10, (HANDLE)hFile);
            if(ReadSize <= 0)
            {
                DEBUG("-----FrameBuf FileRead Error-----\n");
                return 0;
            }
            RemainSize -= ReadSize;
        }
        else
        {
            DEBUG("-----Not Find Picture-----\n");
            return 0;
        }
    }

    if(RemainSize <= 0)
    {
        DEBUG("=====Not Find Picture=====\n");
        return 0;
    }

    //计算出图片标签的大小
    FrameSize = FrameBuf.mSize[0] * 0x1000000
        + FrameBuf.mSize[1] * 0x10000
        + FrameBuf.mSize[2] * 0x100
        + FrameBuf.mSize[3];

    if(FrameSize > 512)
        ImageOffset = 512;
    else
        ImageOffset = FrameSize;
    ReadSize = FileRead(image_tag, 6, (HANDLE)hFile);
    if(ReadSize <= 0)
    {
        DEBUG("=====image_tag FileRead Error=====\n");
        return 0;
    }

    i = 0;
    while(1)
    {
        //没有找到标识图像类型的标签
        if(i > ImageOffset)
        {
            DEBUG("=====Could not find the image identification=====\n");
            return 0;
        }

        if(0 == (strncmp(image_tag, "image/", 6)))
        {
            FrameSize -= 6;
            FileRead(pic_type, 4, (HANDLE)hFile);
            if(ReadSize <= 0)
            {
                DEBUG("=====pic_type FileRead Error=====\n");
                return 0;
            }

            if(0==strncmp(pic_type, "jpeg", 4))
            {
                FrameSize -= 4;
                *picType = PIC_JPEG;
                //DEBUG("=====The album cover is JPEG format=====\n");
                break;
            }
            else if(0==strncmp(pic_type, "jpg", 3))
            {
                FrameSize -= 3;
                *picType = PIC_JPG;
                FileSeek(-1, SEEK_CUR, (HANDLE)hFile);
                //DEBUG("=====The album cover is JPG format=====\n");
                break;
            }
#if 0
            else if(0==strncmp(pic_type,"bmp",3))
            {
                FrameSize -= 3;
                *picType = PIC_BMP;
                FileSeek(-1, SEEK_CUR, (HANDLE)hFile);
                DEBUG("=====The album cover is PEG format=====\n");
                break;
            }
            else if(0==strncmp(pic_type,"peg",3))
            {
                FrameSize -= 3;
                *picType = PIC_PEG;
                FileSeek(-1, SEEK_CUR, (HANDLE)hFile);
                DEBUG("=====The album cover is PEG format=====\n");
                break;
            }
#endif
            else
            {
                DEBUG("=====The album cover format is not supported=====\n");
                return 0;
            }
        }
        else
        {
            i++;
            FrameSize--;
            FileSeek(-5, SEEK_CUR, (HANDLE)hFile);
            FileRead(image_tag, 6, (HANDLE)hFile);
            if(ReadSize <= 0)
            {
                DEBUG("-----image_tag FileRead Error-----\n");
                return 0;
            }
            continue;
        }
    }

    return FrameSize;
}

static int Thumbnail_Parse(FILE* hFile, int* picType)
{
    long ThumbOffset = 0, ThumbSize = 0;
    int ThumbType = 0;

    Thumbnail_IF_SetFilePointer(hFile);
    Thumbnail_Read_Main();
    ThumbOffset = Thumbnail_IF_GetThumbnailOffset();
    ThumbSize = Thumbnail_IF_GetThumbnailSize();
    ThumbType = Thumbnail_IF_GetPictureType();

    if ((ThumbOffset < 0) || (ThumbSize < 0) || (ThumbType < 0))
    {
        DEBUG("=====WMA/AAC Error or CheckID3V2Tag Error=====\n");
        return 0;
    }

    *picType = ThumbType;
    FileSeek((int)ThumbOffset, SEEK_SET, (HANDLE)hFile);
    return ThumbSize;
}

static int ID3GetPicInfo(FILE* hFile, int* picType)
{
    int FrameSize = 0;
    int ID3Length = 0, ReadSize = 0;
    UINT8 ID3buf[10] = {"0"};

    ReadSize = FileRead(ID3buf, 10, (HANDLE)hFile);
    if(ReadSize <= 0)
    {
        DEBUG("=====ID3buf FileRead Error=====\n");
        return 0;
    }

    ID3Length = CheckID3V2Tag(ID3buf);
    if (ID3Length == 0)
        FrameSize = Thumbnail_Parse(hFile, picType);  //ID3v2.2, ID3v2.4
    else
        FrameSize = ID3V23GetPicInfo(hFile, picType, ID3Length); //ID3v2.3

    return FrameSize;
}

static int FLACGetPicInfo(HANDLE fHandle, int* picType)
{
    int i;
    unsigned char tag, type;     //块信息类型
    unsigned int metadata_block_size;   //信息块大小，不包含头大小
    unsigned int FileSize, ReadSize = 0;

    unsigned char ID[4];
    unsigned char metadata_block_header[4]; //信息块头数据，包含块类型和块大小信息
    unsigned char tempbuf[BUFFER_SIZE];

    FileSize = RKFLength((FILE*)fHandle);
    if(FileSize <= 0)
        return 0;

    if(FileRead(ID, 4, fHandle) < 4) return 0;
    if(memcmp(ID, "fLaC", 4)) return 0;
    ReadSize += 4;

    while(ReadSize < FileSize)
    {
        //读取metadata 头信息
        if(FileRead(metadata_block_header, 4, fHandle) < 4) return 0;
        ReadSize += 4;

        tag = (metadata_block_header[0] & 0x80) >> 7;
        type = metadata_block_header[0] & 0x7f; //获取metadata类型
        metadata_block_size = ((int)metadata_block_header[3])|(((int)metadata_block_header[2])<<8)|(((int)metadata_block_header[1])<<16);

        if(type == METADATA_TYPE_PICTURE)
        {
            if(FileRead(tempbuf, BUFFER_SIZE, fHandle) < BUFFER_SIZE) return 0;
            for(i = 0; i < BUFFER_SIZE; i++)
            {
                if(0 == strncmp(&tempbuf[i], "image/", 6))
                {
                    if(0 == strncmp(&tempbuf[i + 6], "jpeg", 4) || 0 == strncmp(&tempbuf[i + 6], "jpg", 3))
                        *picType = PIC_JPEG;
                    else if(0 == strncmp(&tempbuf[i + 6], "bmp", 3))
                        *picType = PIC_BMP;

                    break;
                }
            }

            FileSeek(-BUFFER_SIZE, FSEEK_CUR, fHandle);

            if(metadata_block_size > FileSize)
                metadata_block_size = 0;

            return metadata_block_size;
        }
        else
        {
            FileSeek(metadata_block_size, FSEEK_CUR, fHandle);
            ReadSize += metadata_block_size;
        }

        if(tag == 1) break; //最后一个metadata为 1,其他为 0
    }

    return 0;
}


//WAV本身不带ID3信息, 但是可以通过软件插入ID3信息
static int WAVGetPicInfo(HANDLE fHandle, int* picType)
{
    int FileSize, ChunkSize;

    unsigned char buf[12];
    unsigned char ID[4];

    FileSize = RKFLength((FILE*)fHandle);
    if(FileSize <= 0)
        return 0;

    if(FileRead(buf, 12, fHandle) < 12) return 0;
    if ((buf[0] != 'R') || (buf[1] != 'I') ||
            (buf[2] != 'F') || (buf[3] != 'F') ||
            (buf[8] != 'W') || (buf[9] != 'A') ||
            (buf[10] != 'V') || (buf[11] != 'E'))
    {
        return 0;
    }

    FileSize -= 12;

    while(FileSize > 0)
    {
        if(FileRead(ID, 4, fHandle) < 4) return 0;

        if(FileRead((uint8*)&ChunkSize, 4, fHandle) < 4) return 0;
        if(ChunkSize < 0) return 0;

        FileSize -= 8;

        if(memcmp(ID, "id3 ", 4) == 0)
        {
            return ID3GetPicInfo((FILE*)fHandle, picType);
        }
        else
        {
            FileSeek(ChunkSize, FSEEK_CUR, fHandle);
            FileSize -= ChunkSize;
        }
    }

    return 0;

}

int AudioGetPicInfo(FILE* hFile, int* picType)
{
    char FileName[3];
    UINT16 codec;
    int ret = 0;

    FileSeek(0, SEEK_SET, (HANDLE)hFile);
    AudioCheckStreamType(FileName, hFile);
    codec = ID3GetFileType(FileName,(UINT8 *)MusicFileExtString);
    DEBUG("codec = %d",codec);


    switch (codec)
    {
        case 3:     // MP3
            ret = ID3GetPicInfo(hFile, picType);
            break;

        case 4:     // WMA
        case 8:     // AAC
        case 9:     // M4A
            ret = Thumbnail_Parse(hFile, picType);
            break;

        case 5:
            ret = WAVGetPicInfo((HANDLE)hFile, picType);
            break;

        case 6:     // APE
            break;

        case 7:     // FLAC
            ret = FLACGetPicInfo((HANDLE)hFile, picType);
            break;

        case 10:    // OGG
            break;

        default:
            break;
    }

    return ret;
}

static int ImageGetExifInfo(FILE* hFile, int* picType)
{
    int ret;

    IMAGE_EXIF_INFO gJpgExifInfo;
    memset(&gJpgExifInfo, 0, sizeof(gJpgExifInfo));

    *picType = PIC_JPEG;

    Exif_IF_SetFilePointer(hFile);
    ret = Exif_Read_Main();
    if(!ret)
    {
        DEBUG("=====Read EXIF info Error=====\n");
        return ret;
    }

    ret = Exif_IF_GetThumbnailOffset(&gJpgExifInfo);
    if(ret <= 0)
    {
        DEBUG("=====To get a thumbnail offset error=====\n");
        return 0;
    }

    FileSeek(gJpgExifInfo.ThumbnailOffset, SEEK_SET, (HANDLE)hFile);
    return 1;
}

static int GetOutputWH(int *w,int *h)
{
    *w = ImageMaxWidth;
    *h = ImageMaxHeight;

    return IMAGE_VERTIAL;
}

static int ThumbJpgInit(FILE* hFile)
{
    gJpgDecInfo.fhandle = hFile;
    gJpgDecInfo.OutputFormat = IMAGE_RGB565;

    gJpgDecInfo.ptr_output_buf = (unsigned long)gJpgOutputBuf;
    gJpgDecInfo.ScreenBuffer = gScreenBuffer;
    gJpgDecInfo.CurrentDecLine = 0;

    if (-1 == (int)gJpgDecInfo.fhandle||0 == gJpgDecInfo.ptr_output_buf)
        return 0;

    if (0 != JpgInit(&gJpgDecInfo))
    {
        DEBUG("=====JpgInit Error=====\n");
        return 0;
    }

    GetOutputWH(&gJpgDecInfo.OutputW,&gJpgDecInfo.OutputH);
    if (gJpgDecInfo.OutputW > IMAGE_MAX_OUTPUT_WIDTH)
    {
        DEBUG("=====OutputW > IMAGE_MAX_OUTPUT_WIDTH=====\n");
        return 0;
    }

    if (0 != JpgDecCalcuOutput())
    {
        DEBUG("=====JpgDecCalcuOutput Error=====\n");
        return 0;
    }

    if(gDisplayBackground)
        gJpgDecInfo.dispOffset = 0;
    else
        gJpgDecInfo.dispOffset = (gJpgDecInfo.OutputH - gJpgDecInfo.ValidH)>>1;

    gFirstOffset = gJpgDecInfo.dispOffset;
    return 1;
}

static void ThumbJpgDec(void)
{
    UINT32 KeyValTemp = 0;              //<----sanshin_20151026

    while (1)
    {
        //---->sanshin_20151026
        if(TRUE == gIsMusicWinFlg)
        {
            if(1 == KeyFlag.bc.bKeyHave)
            {
                KeyValTemp = GetKeyVal();
                if(
                    (thumb_KEY_VAL_FFD_SHORT_UP == KeyValTemp) ||
                    (thumb_KEY_VAL_FFW_SHORT_UP == KeyValTemp)
                ){
                    LCD_ClrRect(0,0,128,104);
                            KeyFlag.bc.bKeyHave = 1;


                    break;

                }else if(
                    (thumb_KEY_VAL_FFD_DOWN == KeyValTemp) ||
                    (thumb_KEY_VAL_FFW_DOWN == KeyValTemp)
                ){
                    KeyValTemp = 0;// key lost
                }
            }


            if(KeyValTemp != 0)
            {
                KeyFlag.bc.bKeyHave = 1;
            }

        }
        //<----sanshin_20151026

        if(gJpgDecInfo.dispOffset - gFirstOffset >= gJpgDecInfo.ValidH)
            break;

#ifdef _WATCH_DOG_  //albums cover have a large size
        if(gJpgDecInfo.ImageH > 2000 || gJpgDecInfo.ImageW > 2000)
        {
            if((gJpgDecInfo.dispOffset - gFirstOffset) % (gJpgDecInfo.ValidH / 4) == 0)
            {
                WatchDogReload();
            }
        }
#endif

        if (0 != JpgDecode(&gJpgDecInfo))
        {
            LCD_DrawBmp(ImageLeft, ImageTop+gJpgDecInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(gJpgDecInfo.ScreenBuffer));
            gJpgDecInfo.dispOffset++;
        }
        else
        {
            LCD_DrawBmp(ImageLeft, ImageTop+gJpgDecInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(gJpgDecInfo.ScreenBuffer));
            break;
        }
    }
}

int ThumbInit(FILE* hFile, int picType)
{
    int ret = 0;

    ImageFileFuncInit();

    switch (picType)
    {
        case (PIC_JPEG):
        case (PIC_JPG):
        {
            ModuleOverlay(MODULE_ID_JPG_DECODE, MODULE_OVERLAY_ALL);
            ret = ThumbJpgInit(hFile);
            break;
        }
        default:
        {
            DEBUG("=====Thumbnail format is not supported=====\n");
            break;
        }
    }

    return ret;
}

static void ThumbDeInit(int picType)
{
    switch (picType)
    {
        case (PIC_JPEG):
        case (PIC_JPG):
            {
                FREQ_ExitModule(FREQ_JPG);
                break;
            }
    }
}

int ThumbDecode(int picType)
{
    switch (picType)
    {
        case (PIC_JPEG):
        case (PIC_JPG):
            {
                ThumbJpgDec();
                break;
            }
        default:
            {
                DEBUG("=====Thumbnail format is not supported=====\n");
                return 0;
            }
    }

    return 1;
}

void IsDisplayBackground(int DisplayBackground)
{
    gDisplayBackground = DisplayBackground;
}

void SetPicFileType(int PicFileType)
{
    gPicFileType = PicFileType;
}

int ThumbParse(FILE* hFile)
{
    int ret = 0;
    int picType = -1;

    if ((int)hFile > MAX_OPEN_FILES || (int)hFile == NOT_OPEN_FILE)
    {
        return 0;
    }

    if(gPicFileType == AUDIO_PIC)
    {
        //带ID3专辑封面的音频解析
        ret = AudioGetPicInfo(hFile, &picType);
    }
    else if(gPicFileType == IMAGE_PIC)
    {
        //带EXIF的图片解析
        ret = ImageGetExifInfo(hFile, &picType);
    }
    else
    {
        DEBUG("=====Not set the PicFileType=====\n");
        return 0;
    }

    if(picType == -1)
        return 0;

    if(!ret)
    {
        DEBUG("=====Do not include the thumbnail=====\n");
        return ret;
    }

    ret = ThumbInit(hFile, picType);
    if(ret)
    {
        ThumbDecode(picType);
    }

    //ThumbDeInit(picType);
    FileSeek(0, SEEK_SET, (HANDLE)hFile);
    return ret;
}

extern uint16 CurrentFrameIndex;
uint32 ThumbJpgDecSub(void)
{
    uint16 FrameBufferIndexBak;

    {
        if(gJpgDecInfo.dispOffset - gFirstOffset >= gJpgDecInfo.ValidH)
        {
            return 0;
        }

        if (0 != JpgDecode(&gJpgDecInfo))
        {
            FrameBufferIndexBak = CurrentFrameIndex;
            CurrentFrameIndex = BUFFER_MAX_NUM - 1;
            LCD_DrawBmp(ImageLeft, ImageTop+gJpgDecInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(gJpgDecInfo.ScreenBuffer));
            gJpgDecInfo.dispOffset++;
            CurrentFrameIndex = FrameBufferIndexBak;
        }
        else
        {
            FrameBufferIndexBak = CurrentFrameIndex;
            CurrentFrameIndex = BUFFER_MAX_NUM - 1;
            LCD_DrawBmp(ImageLeft, ImageTop+gJpgDecInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(gJpgDecInfo.ScreenBuffer));
            CurrentFrameIndex = FrameBufferIndexBak;
            return 0;
        }
    }
    return 1;
}

//---->sanshin_20151026
int ThumbJpgFileSet(FILE* hFile)
{
    UINT32  i, j, t;
    int picType = -1;
    int ret = 0;

    ret = AudioGetPicInfo(hFile, &picType);
    if(ret)
    {
        ret = ThumbInit(hFile, picType);
    }
    return ret;
}

int ThumbJpgDecOnly(UINT16 *pDestBuf)
{
    int ret, i;
	unsigned short *Sbufpt = gJpgDecInfo.ScreenBuffer;	//<----sanshin_20151103

    ret = JpgDecode(&gJpgDecInfo);

    for(i=0; i < 12; i++)
    {
		*pDestBuf = *Sbufpt;	//<----sanshin_20151103
        pDestBuf++;				//<----sanshin_20151103
    	Sbufpt++;				//<----sanshin_20151103
    }

    return ret;
}

void ThumbJpgDrawBmp(UINT16 *pdata)
{
    int loop = 0;
    int offset = 0;
    for(loop=0; loop < 12; loop++)
    {
        LCD_DrawBmp(ImageLeft, ImageTop+offset , ImageMaxWidth, 1, 16, pdata);
        offset++;
        pdata += 12;
    }
}
//<----sanshin_20151026

#pragma arm section code
#endif

