
#include "SysInclude.h"

#ifdef _RK_ID3_

#include "AudioControl.h"
#include "ID3.h"
#include "wma_parse.h"
#include "AsicToUnicode.h"

#ifdef _MEDIA_MODULE_
#include "MediaBroWin.h"
#include "AddrSaveMacro.h"
#endif

#define INT8U       unsigned char

#define DEBUG_OPT 1

#define ATOMHOFS                (8)
#define ID3_TEXT_WORK_SIZE      (256*3)

/* ID3v2 Header */
typedef struct _id3v2_Header {
    char            Id[3];
    unsigned char   Ver[2];
    unsigned char   Flg;
    unsigned char   Size[4];
} ID3V2_HEADER;

/* ID3v23   Frame */
typedef struct _id3v2_frame {
    char            Id[4];                          /* Frame Id */
    char            Size[4];
    char            Flg[2];
} ID3V23_FRAME;

/* ID3v22   Frame (ID3v2.3) */
typedef struct _id3v22_frame {
    char            Id[3];                          /* Frame Id */
    char            Size[3];
} ID3V22_FRAME;

/* ATOM Header */
typedef struct _atom_header {
    unsigned int    Length;                         /* ATOM Length */
    char            Atomtype[4];                    /* ATOM Type */
} ATOM_HEADER;

/* ATOM Frame */
typedef struct _atom_frame {
    ATOM_HEADER     Header;                         /* ATOM Header */
    char            *Data;                          /* ATOM Data */
} ATOM_FRAME;

typedef struct _itunes_frame {
    ATOM_HEADER     Header;                         /* iTunes Header */
    ATOM_HEADER     dataHeader;                     /* iTunes Header */
    char            atomver;                        /* ATOM Version */
    char            atomflg[3];                     /* ATOM Flag */
    char            null[4];                        /* NULL Data */
    //char          *Data;                          /* iTunes Data */
} ITUNES_FRAME;
/* Text Infomation Frame(v2.2) */
typedef struct _txt_info_frame_v22 {
    ID3V22_FRAME    Fheader;
    char            Txtencode;
    unsigned short  TxtBom;                         /* BOM */
} TXT_INFO_FRAME_V22;

/* Text Infomation Frame(v2.3) */
typedef struct _txt_info_frame_v23 {
    ID3V23_FRAME    Fheader;
    char            Txtencode;
    unsigned short  TxtBom;
} TXT_INFO_FRAME_V23;

static ID3V2_HEADER Id3v2_header;
static int          Id3v2size;
static UINT32       iTunessize;


static void Aac_GetID3(FILE *fp, ID3V2X_INFO *Id3V2x, UINT16 codec);
static int get_iTunes_info(FILE *fp, ID3V2X_INFO *Id3V2x);
static int get_id3_info(FILE *fp, ID3V2X_INFO *Id3V2x);
static int seek_iTunes_frame(FILE *fp, int cnt);
static int seek_v22_frame(FILE *fp, int cnt);
static int seek_v23_frame(FILE *fp, int cnt);
static int parse_iTunes_frame(FILE *fp, unsigned short *txt);
static int parse_v22_frame(FILE *fp, unsigned short *txt);
static int parse_v23_frame(FILE *fp, unsigned short *txt);
static int seekToRelativeOffset(FILE *fp, unsigned int ofs);
static unsigned int Lit2Big32bit(unsigned int ui);
static int atomitunessearch(FILE *fp);
static int atomid32search(FILE *fp);
static int atomheadersearch(ATOM_FRAME *atomframe, char *atomname, unsigned int atomsize, FILE *fp);
static int getheader(FILE *fp);
static void initid3v2header(void);
static int16 TransCodeFromUTF8ToUnicode(UINT16 *longName);
static int GetUTF8Size(uint16 pInput);

typedef uint16 (*pASICIItoUNIFun)(UINT16 code,UINT16 codemode);
_ATTR_ID3_DATA_
/*const*/ char ID3_GenreTable[][23] =
{
    {"Blues",        },
    {"Classic Rock", },
    {"Country",      },
    {"Dance",        },
    {"Disco",        },
    {"Funk",         },
    {"Grunge",       },
    {"Hip-Hop"  ,    },
    {"Jazz",         },
    {"Metal",        },
    {"New Age",      },
    {"Oldies",       },
    {"Other",        },
    {"Pop",          },
    {"R&B",          },
    {"Rap",          },
    {"Reggae",       },
    {"Rock",         },
#if 1
    "Techno",    "Industrial",   "Alternative",  "Ska",         "Death Metal",    "Pranks",
    "Soundtrack", "Euro-Techno",  "Ambient",      "Trip-Hop",    "Vocal",          "Jazz+Funk",
    "Fusion",     "Trance",       "Classical",    "Instrumental",  "Acid",           "House",
    "Game",       "Sound Clip",   "Gospel",       "Noise",         "Alt. Rock",      "Bass",
    "Soul",       "Punk",         "Space",        "Meditative",    "Instrum. Pop",   "Instrum. Rock",
    "Ethnic",     "Gothic",       "Darkwave",     "Techno-Indust.", "Electronic",     "Pop-Folk",
    "Eurodance",  "Dream",        "Southern Rock", "Comedy",        "Cult",           "Gangsta",
    "Top 40",     "Christian Rap", "Pop/Funk",     "Jungle",        "Native American", "Cabaret",
    "New Wave",   "Psychadelic",  "Rave",         "Showtunes",     "Trailer",        "Lo-Fi",
    "Tribal",     "Acid Punk",    "Acid Jazz",    "Polka",         "Retro",          "Musical",
    "Rock & Roll", "Hard Rock",    "Folk",         "Folk/Rock",     "National Folk",  "Swing",
    "Fusion",     "Bebob",        "Latin",        "Revival",       "Celtic",         "Bluegrass",
    "Avantgarde", "Gothic Rock",  "Progress. Rock", "Psychadel. Rock", "Symphonic Rock", "Slow Rock",
    "Big Band",   "Chorus",       "Easy Listening", "Acoustic",    "Humour",          "Speech",
    "Chanson",    "Opera",        "Chamber Music", "Sonata",         "Symphony",    "Booty Bass",
    "Primus",     "Porn Groove",   "Satire",
//extended
    "Slow Jam",   "Club",        "Tango",     "Samba",       "Folklore",    "Ballad",
    "Power Ballad", "Rhythmic Soul", "Freestyle",   "Duet",       "Punk Rock",       "Drum Solo",
    "A Capella",  "Euro-House",   "Dance Hall",    "Goa",      "Drum & Bass",      "Club-House",
    "Hardcore",   "Terror",      "Indie",        "BritPop",  "Negerpunk",        "Polsk Punk",
    "Beat",      "Christian Gangsta Rap",  "Heavy Metal",   "Black Metal", "Crossover", "Contemporary Christian",
    "Christian Rock",
// winamp 1.91 genres
    "Merengue",   "Salsa",      "Thrash Metal",

// winamp 1.92 genres
    "Anime",      "Jpop",       "Synthpop",
// winamp 5.6
    "Abstract",   "Art Rock",   "Baroque",  "Bhangra",
    "Big Beat",   "Breakbeat",  "Chillout", "Downtempo",
    "Dub",        "EBM",        "Eclectic", "Electro",
    "Electroclash", "Emo",      "Experimental",
    "Garage",     "Global",     "IDM",
    "Illbient",   "Industro-Goth","Jam Band","Krautrock",
    "Leftfield",  "Lounge",     "Math Rock",
    "New Romantic","Nu-Breakz",  "Post-Punk",
    "Post-Rock",  "Psytrance",  "Shoegaze",
    "Space Rock", "Trop Rock",  "World Music",
    "Neoclassical","Audiobook", "Audio Theatre",
    "Neue Deutsche Welle","Podcast",
    "Indie Rock",  "G-Funk",    "Dubstep","Garage Rock","Psybient",NULL
#endif
};

_ATTR_ID3_BSS_ ID3 id3;
_ATTR_ID3_BSS_ unsigned char ID3ZeroFlag;
_ATTR_ID3_BSS_ unsigned char ID3TempBuff[MAX_ID3_FIND_SIZE];

_ATTR_ID3_TEXT_
UINT16 GetUtf8CodeType(UINT8 GbkCode)
{
    UINT16 iType;
    if ((GbkCode & 0x80) == 0x00)
        iType = 1;
    else if ((GbkCode & 0xe0) == 0xc0)
        iType = 2;
    else if ((GbkCode & 0xf0) == 0xe0)
        iType = 3;
    else            //����3�ֽڣ���֧��
        iType = 0;

    return iType;
}

_ATTR_ID3_TEXT_
UINT16 Ascii2Unicode(UINT16 *pSbuf, UINT16 *pTbuf, UINT16 Len, UINT8 EncodeMode)
{
    UINT16        iUnicodeChars;
    UINT8         ASICCode;
    UINT8         ASICCode1;
    UINT16        ASICCode2;
    UINT16        iType;                  // 1,signal byte charactor��2, doubule bytes charactors
    // 3, three bytes charactors
    UINT8         *pGbk;
    UINT16        *pUnicode;

    pASICIItoUNIFun  ASICtoUNIFun;

    pGbk            =  (UINT8*) pSbuf;
    pUnicode        =   pTbuf;
    iType           =   1;
    iUnicodeChars   =   0;

    //DEBUG("Language is %x",Language);

    switch (Language)
    {
        case LANGUAGE_CHINESE_S:                    //Simplified Chinese. GBK
            ASICtoUNIFun = (pASICIItoUNIFun) Gbk2Unicode;
            break;
        case LANGUAGE_CHINESE_T:                    //Traditional Chinc    CP950
            ASICtoUNIFun = (pASICIItoUNIFun) CP950toUnicode;
            break;
        case LANGUAGE_ENGLISH:                      //English
            ASICtoUNIFun = (pASICIItoUNIFun) CP1252toUnicode;
            break;
        case LANGUAGE_KOREAN:                       //Korean               CP949
            ASICtoUNIFun = (pASICIItoUNIFun) CP949toUnicode;
            break;
        case LANGUAGE_JAPANESE:                     //Japanese             CP932
            ASICtoUNIFun = (pASICIItoUNIFun) CP932toUnicode;
            break;
            //case LANGUAGE_SPAISH:                       //Spanish
            // ASICtoUNIFun = (pASICIItoUNIFun) CP1252toUnicode;
            // break;
        case LANGUAGE_FRENCH:                       //French
            ASICtoUNIFun = (pASICIItoUNIFun) CP1252toUnicode;
            break;
        case LANGUAGE_GERMAN:                       //Germa
            ASICtoUNIFun = (pASICIItoUNIFun) CP1252toUnicode;
            break;
        case LANGUAGE_PORTUGUESE:                   //Portuguess
            ASICtoUNIFun = (pASICIItoUNIFun) CP1252toUnicode;
            break;
        case LANGUAGE_RUSSIAN:                      //Russian   CP1251
            ASICtoUNIFun = (pASICIItoUNIFun) CP1251toUnicode;
            break;
        default:
            ASICtoUNIFun = (pASICIItoUNIFun) Gbk2Unicode;
            break;
    }

    while ('\0' != (*pGbk))
    {
        if (iUnicodeChars >= Len)
        {
            break;
        }

        ASICCode = *pGbk;
        ASICCode1 = *(pGbk+1);

        // Judgment is still single-byte UTF8 encoding, double-byte coding
        if (EncodeMode == ID3_ENCODE_UTF8)
        {
            iType = GetUtf8CodeType(ASICCode);
        }
        else
        {
            switch (Language)
            {
                case LANGUAGE_CHINESE_S:                    //Simplified Chinese. GBK
                    iType = GetGbkCodeType(ASICCode, ASICCode1);
                    break;
                case LANGUAGE_CHINESE_T:                    //Traditional Chinc    CP950
                    iType = GetCP950CodeType(ASICCode, ASICCode1);
                    break;
                case LANGUAGE_KOREAN:                       //Korean               CP949
                    iType = GetCP949CodeType(ASICCode, ASICCode1);
                    break;
                case LANGUAGE_JAPANESE:                     //Japanese             CP932
                    iType = GetCP932CodeType(ASICCode, ASICCode1);
                    break;

                case LANGUAGE_RUSSIAN:                      //Russian   CP1251
                case LANGUAGE_ENGLISH:                      //Englis
                case LANGUAGE_SPAISH:                       //Spanish
                case LANGUAGE_FRENCH:                       //French
                case LANGUAGE_GERMAN:                       //Germa
                case LANGUAGE_PORTUGUESE:                   //Portuguess
                default:
                    iType = 1;
                    break;                            // Western European languages ??have only one byte
            }
        }

        if (1 == iType)
        {
            ASICCode1 = *pGbk;
            *pUnicode = ASICtoUNIFun((UINT16)ASICCode1,0);
        }
        else if (2 == iType)
        {
            if ('\0' == (*(pGbk + 1)))
            {
                break;
            }
            ASICCode2 = *(UINT16*)pGbk;
            *pUnicode = ASICtoUNIFun(ASICCode2,1);
        }
        else if (3 == iType)
        {
            if ('\0' == (*(pGbk + 1)))
            {
                break;
            }

            if ('\0' == (*(pGbk + 2)))
            {
                break;
            }

            ASICCode = *(UINT16*)pGbk & 0x0f;
            ASICCode1 = *(UINT16*)(pGbk + 1) & 0x3f;
            ASICCode2 = *(UINT16*)(pGbk + 2) & 0x3f;

            *pUnicode = (ASICCode << 12) | (ASICCode1 << 6) | ASICCode2;

        }
        else
        {   //UTF8 byte does not support more than three
            return 0;
        }

#if 0
        if ((*pUnicode == 0x005C) && (Language == LANGUAGE_JAPANESE))
        {
            printf("Language = %d\n", Language);

            *pUnicode = 0x00A5;
        }

        if (*pUnicode == 0x201A )
        {
            *pUnicode = 0x002C;
        }
        else if (*pUnicode == 0x2039 )
        {
            *pUnicode = 0x003C;
        }
        else if (*pUnicode == 0x2022 )
        {
            *pUnicode = 0x30FB;
        }
        else if (*pUnicode == 0x203A )
        {
            *pUnicode = 0x003E;
        }
#endif


        iUnicodeChars += iType;
        if (iUnicodeChars > SYS_SUPPROT_STRING_MAX_LEN)
        {
            break;
        }

        pUnicode++;
        pGbk          += iType;

    }

    *pUnicode = '\0';

    return iUnicodeChars;
}



/*----------------------------------------------------------------------
Name  :  Mp3Id3ToUnicode
Desc  :  turn Mp3ID3 to Unicode code.
Params:  pUnicodeId3��data storage buffer by unicode code.pMp3ID3:Mp3 ID3 source data  size��MP3 ID3 length of byte.
Return:
Author:  phc
Date  :  07-09-29
----------------------------------------------------------------------*/
_ATTR_ID3_TEXT_
void Mp3Id3ToUnicode(UINT16 * pUnicodeId3, UINT8 *pMp3ID3, UINT32 size, UINT8 EncodeMode)
{
    UINT16 i;

    if (size==0 || !pUnicodeId3 || !pMp3ID3)
    {
        return;
    }

    if (*pMp3ID3 == '\0')
    {
        return ;
    }

    if (((pMp3ID3[0] == 0xFF) && (pMp3ID3[1] == 0xFE)) || ((pMp3ID3[1] == 0xFF) && (pMp3ID3[0] == 0xFE)))
    {
        size = size / 2;
        size = size - 1; //remove 0xff 0xfe or 0xfe ff

        if (size > SYS_SUPPROT_STRING_MAX_LEN)
        {
            size = SYS_SUPPROT_STRING_MAX_LEN; //very important
        }

        if (pMp3ID3[1] == 0xFE)
        {
            for (i = 0;i < size; i++)
            {
                pUnicodeId3[i] = pMp3ID3[2 + 2*i] + (pMp3ID3[3 + 2*i] << 8);
                if (pUnicodeId3[i] == 0)
                {
                    break;
                }

            }
        }
        else
        {
            for (i = 0;i < size;i++)
            {
                pUnicodeId3[i] = ((pMp3ID3[2 + 2*i] << 8) + pMp3ID3[3 + 2*i]);
                if (pUnicodeId3[i] == 0)
                {
                    break;
                }
            }
        }

        pUnicodeId3[i] = '\0';

    }
    else
    {
        for (i = 0;i < size;i++) //exclude the situation of content is 0 orr space of id3 information head.
        {
            if (*pMp3ID3 <= 0x20)//filter the characters that ascii code are litter than 0x20.
            {
                pMp3ID3++;
            }
            else
            {
                break;
            }
        }

        if (i < size)
        {
            UINT16 *pUniStr = NULL;

            Ascii2Unicode((UINT16 *)pMp3ID3, pUnicodeId3,(UINT16)size, EncodeMode);

            /* some illegal character be changed to 0x20,when trun to unicode. so to filter again in here.*/
            pUniStr = pUnicodeId3;

            while (*pUniStr && *pUniStr<=0x20)  //filter the space in head.
            {
                *pUniStr = 0;
                pUniStr ++;
            }
        }
    }
}


/******************************************************
Name:  ID3_SeekData
Desc:  Seek data in a File
Param: fd:file handle. offset  iwhence:mode
Return: 0:failure 1-success.
Global: null
Note:  null
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_SeekData(HANDLE fd, int offset, int iwhence)
{
    int result;
    result = FileSeek( offset, iwhence , (HANDLE)fd);//FileSeek( offset, iwhence , (FILE*)fd);// RKFIO_FSeek( offset, iwhence , (FILE*)fd);
    return result;
}

/******************************************************
Name:  ID3_ReadData
Desc:  Read data from a File
Param: fd:file handle buff:buffer; iSize :size
Return: 0-failure 1-success
Global: null
Note:  null
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_ReadData(HANDLE fd, char *buff, int iSize)
{
    int result;
    result = FileRead( buff, iSize , fd);// FileRead( buff, iSize , (FILE*)fd);//RKFIO_FRead( buff, iSize , (FILE*)fd);
    return result;
}

_ATTR_ID3_TEXT_
static void SpaceCovert2Null(unsigned char * buff,int length)
{
    for(;length > 0;length--)
    {
        if(buff[length-1] <= 0x20)
            buff[length-1] = 0;
        else
            break;
    }
}

/******************************************************
Name:  ID3_GetID3_V1X
Desc:  get VI information of id3
Param: file handle, id3 information.charactor cache.
Return:
Global: null
Note:  null
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_GetID3_V1X(HANDLE fHandle, ID3V2X_INFO * pstId3v2x, char *tempbuf)
{
    pstID3InfoType  pID3Info;
    int result;

    if (    !(
                ((pID3Info->mTitle[0] != NULL) && (pstId3v2x->id3_title[0] == NULL))    ||
                ((pID3Info->mArtist[0] != NULL) && (pstId3v2x->id3_singer[0] == NULL))  ||
                ((pID3Info->mAlbum[0] != NULL) && (pstId3v2x->id3_album[0] == NULL))    ||
                ((pID3Info->mGenre<17) && (pstId3v2x->id3_genre[0] == NULL))            ||
                ((pID3Info->mTrack > 0) && (pstId3v2x->nTrack[0] == NULL))
            )
       )
    {
        return -1;
    }

//  printf("ID3 v1 tag get \n");

    result = ID3_SeekData(fHandle, MAX_ID3V1_SIZE, FSEEK_END);
    result = ID3_ReadData(fHandle, tempbuf, MAX_ID3V1_SIZE);
    if (result <= 0)
    {
        return -1;
    }

    pID3Info = (pstID3InfoType)tempbuf;

    if (!memcmp((INT8U *)pID3Info->mTAG, "TAG", 3))
    {
        if ((pID3Info->mTitle[0] != NULL) && (pstId3v2x->id3_title[0] == NULL))
        {
            SpaceCovert2Null(pID3Info->mTitle, 30);
            Mp3Id3ToUnicode((uint16 *)(pstId3v2x->id3_title), pID3Info->mTitle, 30, ID3_ENCODE_ISO88591);
            pstId3v2x->id3_title[30] = 0;
        }

        if ((pID3Info->mArtist[0] != NULL) && (pstId3v2x->id3_singer[0] == NULL))
        {
            SpaceCovert2Null(pID3Info->mArtist, 30);
            Mp3Id3ToUnicode((uint16 *)(pstId3v2x->id3_singer), pID3Info->mArtist, 30, ID3_ENCODE_ISO88591);
            pstId3v2x->id3_singer[30] = 0;
        }
        if ((pID3Info->mAlbum[0] != NULL) && (pstId3v2x->id3_album[0] == NULL))
        {
            SpaceCovert2Null(pID3Info->mAlbum, 30);
            Mp3Id3ToUnicode((uint16 *)(pstId3v2x->id3_album), pID3Info->mAlbum, 30, ID3_ENCODE_ISO88591);
            pstId3v2x->id3_album[30] = 0;
        }

        if ((pID3Info->mGenre < ID3_GENRE_MAX_CNT) && (pstId3v2x->id3_genre[0] == NULL))//memery starvation.
        {
            Mp3Id3ToUnicode((uint16 *)(pstId3v2x->id3_genre), ID3_GenreTable[pID3Info->mGenre], 23, ID3_ENCODE_ISO88591);
            pstId3v2x->id3_genre[23] = 0;
        }

        if ((pID3Info->mTrack > 0) && (pstId3v2x->nTrack[0] == NULL))
        {
            //printf("mTrack = %d\n",pID3Info->mTrack);
            pstId3v2x->nTrack[0] = pID3Info->mTrack / 1000 + 0x30;
            pID3Info->mTrack =  pID3Info->mTrack % 1000;
            pstId3v2x->nTrack[1] = pID3Info->mTrack / 100 + 0x30;
            pID3Info->mTrack =  pID3Info->mTrack % 100;
            pstId3v2x->nTrack[2] = pID3Info->mTrack / 10 + 0x30;
            pID3Info->mTrack =  pID3Info->mTrack % 10;
            pstId3v2x->nTrack[3] = pID3Info->mTrack + 0x30;
        }

        return 1;
    }

    return -1;
}

/******************************************************
Name:  ID3_GetID3V2xHeader
Desc:  get the v2 head of id3
Param: charactor cache,frame offset,version.
Return:
Global: null
Note:  null
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_GetID3V2xHeader(unsigned char *tempbuf, int *iFrameOffset, int *iVersion)
{
    int iTotalSize;
    pID3V2XHeaderType   pID3V2XHeader;

    pID3V2XHeader = (pID3V2XHeaderType)tempbuf;

    if (pID3V2XHeader->mVersion > 4) // because big endian
    {
        return -1; //not support
    }

    *iVersion  = (int)pID3V2XHeader->mVersion;

    //iTotalSize =  (ID3_GetIntValue(pID3V2XHeader->mSize) + (int)sizeof(ID3V2XHeaderType));
    iTotalSize =  ID3_GetIntValue(pID3V2XHeader->mSize); // changed by phc, 2007-11-08

    if (pID3V2XHeader->mFlags & ID3_FLAG_FOOTER)
    {
        iTotalSize += sizeof(ID3V2XFooterType);
    }

    *iFrameOffset = (int)sizeof(ID3V2XHeaderType);
    if (pID3V2XHeader->mFlags & ID3_FLAG_EXTHEADER)
    {
        pID3V2XExtHeaderType pID3V2XExtHeader;

        pID3V2XExtHeader = (pID3V2XExtHeaderType) & tempbuf[*iFrameOffset];

        // tag extheader info
        if (pID3V2XExtHeader->mExtFlags & ID3_FLAG_UPDATE)
        {
            *iFrameOffset += ID3_GetIntValue(pID3V2XExtHeader->mHeaderSize);
        }
    }

    return iTotalSize;
}

/******************************************************
Name:  ID3_GetFrameInfo
Desc:
Param:
Return:
Global:
Note:
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_GetFrameInfo(char *tempbuf, int iVersion)
{
    int iFrameInfo = ID3_NONINFO;

    if (iVersion < 3)   //ID3V2.2
    {
        switch (tempbuf[0])
        {
            case 'T':
                switch (tempbuf[1])
                {
                    case 'C':
                        if (tempbuf[2]  == 'O')
                            iFrameInfo = ID3_GENRE;
                        break;

                    case 'Y':
                        if (tempbuf[2]  == 'E')
                            iFrameInfo = ID3_YEAR;
                        break;
                    case 'R':
                        if (tempbuf[2]  == 'K')
                            iFrameInfo = ID3_TRACK;
                        break;

                    case 'T':
                        if (tempbuf[2]  == '2')
                            iFrameInfo = ID3_TITLE;
                        break;
                    case 'A':
                        if (tempbuf[2]  == 'L')
                            iFrameInfo = ID3_ALBUM;
                        break;
                    case 'P':
                        if (tempbuf[2]  == '1')
                            iFrameInfo = ID3_ARTIST;
                        break;
                }
                if (
                    (   (tempbuf[1]  == 'B') &&
                      (tempbuf[2]  == 'P')  ) ||
                    (   (tempbuf[1]  == 'C') &&
                      (tempbuf[2]  == 'M')  ) ||
                    (   (tempbuf[1]  == 'C') &&
                      (tempbuf[2]  == 'R')  ) ||
                    (   (tempbuf[1]  == 'D') &&
                      (tempbuf[2]  == 'A')  ) ||
                    (   (tempbuf[1]  == 'D') &&
                      (tempbuf[2]  == 'Y')  ) ||
                    (   (tempbuf[1]  == 'E') &&
                      (tempbuf[2]  == 'N')  ) ||
                    (   (tempbuf[1]  == 'F') &&
                      (tempbuf[2]  == 'T')  ) ||
                    (   (tempbuf[1]  == 'I') &&
                      (tempbuf[2]  == 'M')  ) ||
                    (   (tempbuf[1]  == 'K') &&
                      (tempbuf[2]  == 'E')  ) ||
                    (   (tempbuf[1]  == 'L') &&
                      (tempbuf[2]  == 'A')  ) ||
                    (   (tempbuf[1]  == 'L') &&
                      (tempbuf[2]  == 'E')  ) ||
                    (   (tempbuf[1]  == 'M') &&
                      (tempbuf[2]  == 'T')  ) ||
                    (   (tempbuf[1]  == 'O') &&
                      (tempbuf[2]  == 'A')  ) ||
                    (   (tempbuf[1]  == 'O') &&
                      (tempbuf[2]  == 'F')  ) ||
                    (   (tempbuf[1]  == 'O') &&
                      (tempbuf[2]  == 'L')  ) ||
                    (   (tempbuf[1]  == 'O') &&
                      (tempbuf[2]  == 'R')  ) ||
                    (   (tempbuf[1]  == 'O') &&
                      (tempbuf[2]  == 'T')  ) ||
                    (   (tempbuf[1]  == 'P') &&
                      (tempbuf[2]  == '2')  ) ||
                    (   (tempbuf[1]  == 'P') &&
                      (tempbuf[2]  == '3')  ) ||
                    (   (tempbuf[1]  == 'P') &&
                      (tempbuf[2]  == '4')  ) ||
                    (   (tempbuf[1]  == 'P') &&
                      (tempbuf[2]  == 'A')  ) ||
                    (   (tempbuf[1]  == 'P') &&
                      (tempbuf[2]  == 'B')  ) ||
                    (   (tempbuf[1]  == 'R') &&
                      (tempbuf[2]  == 'C')  ) ||
                    (   (tempbuf[1]  == 'R') &&
                      (tempbuf[2]  == 'D')  ) ||
                    (   (tempbuf[1]  == 'R') &&
                      (tempbuf[2]  == 'K')  ) ||
                    (   (tempbuf[1]  == 'S') &&
                      (tempbuf[2]  == 'I')  ) ||
                    (   (tempbuf[1]  == 'S') &&
                      (tempbuf[2]  == 'S')  ) ||
                    (   (tempbuf[1]  == 'T') &&
                      (tempbuf[2]  == '1')  ) ||
                    (   (tempbuf[1]  == 'T') &&
                      (tempbuf[2]  == '3')  ) ||
                    (   (tempbuf[1]  == 'X') &&
                      (tempbuf[2]  == 'T')  ) ||
                    (   (tempbuf[1]  == 'X') &&
                      (tempbuf[2]  == 'X')  ) ||
                    (   (tempbuf[1]  == 'Y') &&
                      (tempbuf[2]  == 'E')  )
                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                    //printf("ID3_NOTUSEDFRAME\n");
                }
                break;

            case 'P':
                switch (tempbuf[1])
                {
                    case 'I':
                        if (tempbuf[2] == 'C')
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        break;
                    case 'O':
                        if (tempbuf[2] == 'P')
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        break;
                }
                break;
            case 'C':
                if (
                    (   (tempbuf[1]  == 'O') &&
                      (tempbuf[2]  == 'M')  ) ||
                    (   (tempbuf[1]  == 'R') &&
                      (tempbuf[2]  == 'A')  ) ||
                    (   (tempbuf[1]  == 'N') &&
                      (tempbuf[2]  == 'T')  ) ||
                    (   (tempbuf[1]  == 'R') &&
                      (tempbuf[2]  == 'M')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                    //printf("ID3_NOTUSEDFRAME\n");
                }
                break;

            case 'U':
                if (
                    (   (tempbuf[1]  == 'F') &&
                      (tempbuf[2]  == 'I')  ) ||
                    (   (tempbuf[1]  == 'L') &&
                      (tempbuf[2]  == 'T')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                    //printf("ID3_NOTUSEDFRAME\n");
                }
                break;


            case 'E':
                if (
                    (   (tempbuf[1]  == 'Q') &&
                      (tempbuf[2]  == 'U')  ) ||
                    (   (tempbuf[1]  == 'T') &&
                      (tempbuf[2]  == 'C')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'G':
                if (
                    (   (tempbuf[1]  == 'E') &&
                      (tempbuf[2]  == 'O')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'I':
                if (
                    (   (tempbuf[1]  == 'P') &&
                      (tempbuf[2]  == 'L')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'L':
                if (
                    (   (tempbuf[1]  == 'N') &&
                      (tempbuf[2]  == 'K')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'M':
                if (
                    (   (tempbuf[1]  == 'C') &&
                      (tempbuf[2]  == 'I')  ) ||
                    (   (tempbuf[1]  == 'L') &&
                      (tempbuf[2]  == 'L')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'B':
                if (
                    (   (tempbuf[1]  == 'U') &&
                      (tempbuf[2]  == 'F')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'R':
                if (
                    (   (tempbuf[1]  == 'V') &&
                      (tempbuf[2]  == 'A')  ) ||
                    (   (tempbuf[1]  == 'E') &&
                      (tempbuf[2]  == 'V')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'S':
                if (
                    (   (tempbuf[1]  == 'L') &&
                      (tempbuf[2]  == 'T')  ) ||
                    (   (tempbuf[1]  == 'T') &&
                      (tempbuf[2]  == 'C')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'W':
                if (
                    (   (tempbuf[1]  == 'C') &&
                      (tempbuf[2]  == 'M')  ) ||
                    (   (tempbuf[1]  == 'C') &&
                      (tempbuf[2]  == 'P')  ) ||
                    (   (tempbuf[1]  == 'A') &&
                      (tempbuf[2]  == 'F')  ) ||
                    (   (tempbuf[1]  == 'A') &&
                      (tempbuf[2]  == 'R')  ) ||
                    (   (tempbuf[1]  == 'A') &&
                      (tempbuf[2]  == 'S')  ) ||
                    (   (tempbuf[1]  == 'P') &&
                      (tempbuf[2]  == 'B')  ) ||
                    (   (tempbuf[1]  == 'X') &&
                      (tempbuf[2]  == 'X')  )

                )
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;
        }
    }
    else    //ID3V2.3, ID3V2.4
    {
        switch (tempbuf[0])
        {
            case 'T':
                switch (tempbuf[1])
                {
                    case 'E':
                        if (tempbuf[2]  == 'N' && tempbuf[3]  == 'C')
                        {
                            iFrameInfo = ID3_ENCODEDBY;
                        }
                        if (tempbuf[2]  == 'X' && tempbuf[3]  == 'T')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'C':
                        if (tempbuf[2]  == 'O')
                        {
                            switch (tempbuf[3])
                            {
                                case 'P':
                                    iFrameInfo = ID3_COPYRIGHT;
                                    break;
                                case 'M':
                                    iFrameInfo = ID3_COMPOSER;
                                    break;

                                case 'N':
                                    iFrameInfo = ID3_GENRE;
                                    break;
                            }
                        }
                        break;

                    case 'O':
                        if (tempbuf[2]  == 'P' && tempbuf[3]  == 'E')
                        {
                            iFrameInfo = ID3_ORIGARTIST;
                        }
                        if (tempbuf[2]  == 'R' && tempbuf[3]  == 'Y')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'A' && tempbuf[3]  == 'L')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'F' && tempbuf[3]  == 'N')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'L' && tempbuf[3]  == 'Y')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'W' && tempbuf[3]  == 'N')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'Y':
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == 'R')
                            iFrameInfo = ID3_YEAR;
                        break;

                    case 'R':
                        if (tempbuf[2]  == 'C' && tempbuf[3]  == 'K')
                        {
                            iFrameInfo = ID3_TRACK;
                        }
                        if (tempbuf[2]  == 'D' && tempbuf[3]  == 'A')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'S' && tempbuf[3]  == 'N')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'S' && tempbuf[3]  == 'O')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'I':
                        if (tempbuf[2]  == 'T' && tempbuf[3]  == '2')
                        {
                            iFrameInfo = ID3_TITLE;
                        }
                        if (tempbuf[2]  == 'M' && tempbuf[3]  == 'E')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'T' && tempbuf[3]  == '1')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'T' && tempbuf[3]  == '3')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'P' && tempbuf[3]  == 'L')   //ID3V2.4
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'A':
                        if (tempbuf[2]  == 'L' && tempbuf[3]  == 'B')
                            iFrameInfo = ID3_ALBUM;
                        break;

                    case 'B':
                        if (tempbuf[2]  == 'P' && tempbuf[3]  == 'M')
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        break;

                    case 'P':
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == '1')
                        {
                            iFrameInfo = ID3_ARTIST;
                        }
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == '2')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == '3')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == '4')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'O' && tempbuf[3]  == 'S')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'U' && tempbuf[3]  == 'B')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'R' && tempbuf[3]  == 'O')   //ID3V2.4
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'D':
                        if (tempbuf[2]  == 'A' && tempbuf[3]  == 'T')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'L' && tempbuf[3]  == 'Y')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }

                        //ID3V2.4
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == 'N')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'O' && tempbuf[3]  == 'R')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'R' && tempbuf[3]  == 'C')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'R' && tempbuf[3]  == 'L')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'T' && tempbuf[3]  == 'G')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'L':
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == 'N')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'A' && tempbuf[3]  == 'N')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'F':
                        if (tempbuf[2]  == 'L' && tempbuf[3]  == 'T')
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        break;

                    case 'K':
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == 'Y')
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        break;

                    case 'M':
                        if (tempbuf[2]  == 'E' && tempbuf[3]  == 'D')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }

                        //ID3V2.4
                        if (tempbuf[2]  == 'C' && tempbuf[3]  == 'L')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'O' && tempbuf[3]  == 'O')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'S':
                        if (tempbuf[2]  == 'I' && tempbuf[3]  == 'Z')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'R' && tempbuf[3]  == 'C')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'S' && tempbuf[3]  == 'E')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }

                        //ID3V2.4
                        if (tempbuf[2]  == 'O' && tempbuf[3]  == 'A')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'O' && tempbuf[3]  == 'P')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'O' && tempbuf[3]  == 'T')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        if (tempbuf[2]  == 'S' && tempbuf[3]  == 'T')
                        {
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        }
                        break;

                    case 'X':
                        if (tempbuf[2]  == 'X' && tempbuf[3]  == 'X')
                            iFrameInfo = ID3_NOTUSEDFRAME;
                        break;
                }
                break;

            case 'C':
                if (!memcmp(&tempbuf[1], "OMM", 3))
                {
                    iFrameInfo = ID3_COMMENT;
                }
                if (!memcmp(&tempbuf[1], "OMR", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'A':
                if (!memcmp(&tempbuf[1], "PIC", 3))
                {
                    iFrameInfo = ID3_PICTURE;
                }
                if (!memcmp(&tempbuf[1], "ENC", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "SPI", 3)) //ID3V2.4
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'G':
                if (!memcmp(&tempbuf[1], "EOB", 3))
                {
                    iFrameInfo = ID3_GEOB;
                }
                if (!memcmp(&tempbuf[1], "RID", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'M':
                if (!memcmp(&tempbuf[1], "CDI", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "LLT", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'E':
                if (!memcmp(&tempbuf[1], "NCR", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "QUA", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "TCO", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "QU2", 3)) //ID3V2.4
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'I':
                if (!memcmp(&tempbuf[1], "PLS", 3))
                    iFrameInfo = ID3_NOTUSEDFRAME;
                break;

            case 'L':
                if (!memcmp(&tempbuf[1], "INK", 3))
                    iFrameInfo = ID3_NOTUSEDFRAME;
                break;

            case 'O':
                if (!memcmp(&tempbuf[1], "WNE", 3))
                    iFrameInfo = ID3_NOTUSEDFRAME;
                break;

            case 'P':
                if (!memcmp(&tempbuf[1], "RIV", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "CNT", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "OPM", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "OSS", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'R':
                if (!memcmp(&tempbuf[1], "BUF", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "VAD", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "VRB", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "VA2", 3)) //ID3V2.4
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'S':
                if (!memcmp(&tempbuf[1], "YLT", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "YTC", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "EEK", 3)) //ID3V2.4
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "IGN", 3)) //ID3V2.4
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'U':
                if (!memcmp(&tempbuf[1], "FID", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "SER", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "SLT", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;

            case 'W':
                if (!memcmp(&tempbuf[1], "COM", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "COP", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "OAF", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "OAR", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "OAS", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "ORS", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "PAY", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "PUB", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                if (!memcmp(&tempbuf[1], "XXX", 3))
                {
                    iFrameInfo = ID3_NOTUSEDFRAME;
                }
                break;
        }
    }

    return iFrameInfo;
}

/******************************************************
Name:  ID3_ParseMetaData
Desc:
Param:
Return:
Global:
Note:
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_ParseMetaData(unsigned char *tempbuf, unsigned char *cTargetBuf, int iStringSize)
{
    pID3V2XEncodeInfoType pID3V2XEncodeInfo;
    unsigned char *pSrcBuf;
    int iStringCnt;
    uint16 iBufSize;
    uint16 iNewBuffSize = 0;


    //UINT16 UnicodeBuff[MAX_STRING_SIZE_CHAR];  //zhuzhe note
    UINT8  SrcBuff[MAX_ID3V2_FIELD];
    UINT8  EncodeMode;
    uint16 i;

    //zhuzhe  write in 2013 for when the id3 0xff 0x00 0xxx
    if (ID3ZeroFlag == 1)
    {
        for (iBufSize = 0;(iBufSize < iStringSize)&&(iNewBuffSize < MAX_ID3V2_FIELD);iNewBuffSize++)
        {
            SrcBuff[iNewBuffSize] = tempbuf[iBufSize];
            if ((tempbuf[iBufSize] == 0xff) &&  (tempbuf[iBufSize + 1] == 0x00 ))
            {
                iBufSize += 2;
            }
            else
            {
                iBufSize ++;
            }
        }
        iStringSize = iNewBuffSize;
        tempbuf = SrcBuff;
    }
    pID3V2XEncodeInfo = (pID3V2XEncodeInfoType)tempbuf;
    iStringCnt      = 0;
    EncodeMode = pID3V2XEncodeInfo->mEncodeType & ID3_ENCODE_MASK;

    switch (EncodeMode)
    {
        case ID3_ENCODE_UTF16:
            if ((pID3V2XEncodeInfo->mUnicodeBOM != ID3_UNICODE_BOM1) &&
                    (pID3V2XEncodeInfo->mUnicodeBOM != ID3_UNICODE_BOM2))
            {
                break;
            }

            //iStringSize-=3;
            iStringSize -= 1;
            if (iStringSize <= 0)
            {
                break;
            }

            //pSrcBuf   = &tempbuf[3];
            pSrcBuf   = &tempbuf[1];
            iStringCnt = iStringSize;

            goto TO_UNICODE;

            break;

        case ID3_ENCODE_UTF16BE:
        case ID3_ENCODE_UTF8:
        case ID3_ENCODE_ISO88591:
            iStringSize--;
            if (iStringSize <= 0)
            {
                break;
            }

            pSrcBuf    = &tempbuf[1];
            iStringCnt = iStringSize;

            goto TO_UNICODE;

            break;
    }

TO_UNICODE:
    Mp3Id3ToUnicode((uint16 *)cTargetBuf, pSrcBuf, iStringSize, EncodeMode);

    return iStringCnt;
}

/******************************************************
Name:  ID3_GetMetaData
Desc:
Param:
Return:
Global:
Note:
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_GetMetaData(unsigned char *tempbuf, unsigned char *cTargetBuf, int iStringSize)
{
    int Result;

    Result = ID3_ParseMetaData(tempbuf, cTargetBuf, iStringSize);
    if (Result == -1)
    {
        // ID3_ENCODE_ISO88591
        memcpy(cTargetBuf, tempbuf, iStringSize);
        cTargetBuf[iStringSize] = NULL;

        return iStringSize;
    }

    return Result;
}

/******************************************************
Name:  ID3_UpdateBufferData
Desc:  update cache data.
Param: file handle,destion cache,saved length,offset.
Return:
Global: null
Note:
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_UpdateBufferData(HANDLE fHandle, unsigned char *tempbuf, int iRemainSize, int iOffset)
{
    int result;
    int iReadSize;

    if (iRemainSize < MAX_ID3_FIND_SIZE)
    {
        iReadSize = iRemainSize;
    }
    else
    {
        iReadSize = MAX_ID3_FIND_SIZE;
    }

    result = ID3_SeekData(fHandle, iOffset, FSEEK_SET);
    if (result != RETURN_OK)
    {
        return -1;
    }

    result = ID3_ReadData(fHandle, (char*)tempbuf, iReadSize);
    if (result <= 0)
    {
        return -1;
    }

    return iReadSize;
}


_ATTR_ID3_TEXT_
long pow (int x, int y)
{
    int i;
    long result = 1;
    for (i = 0; i < y; i++)
    {
        result *= x;
    }

    return result;
}

_ATTR_ID3_TEXT_
uint32 strlenwide1(const uint16* str)
{
    uint32 len = 0;
    while (str[len] != 0)
    {
        len++;
    }
    return len;
}

/******************************************************
Name:  ID3_GetID3_V2X
Desc:  read v2x information of id3
Param:
Return:
Global:
Note:
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int ID3_GetID3_V2X(HANDLE fHandle, ID3V2X_INFO * pstId3v2x , unsigned char *tempbuf, int WavFlag, int ID3ChunkSize, int ID3Offset)
{
    int i, j;
    int iFrameOffset = 0;
    int iFrameInfo = 0;
    int iFrameSize = 0;
    int iFileSize = 0;
    int iReadSize = 0;
    int iTotalSize = 0;
    int iRemainSize = 0;
    int iCurrentOffset = 0;
    int iMetaCnt = -1;
    int iVersion = 0;
    int iOffset = 0;
    int GenreID = -1;

    unsigned int  tc = 0, trackIdSize = 0;
    unsigned short  trackbuf[8];
    int nSize = 0, mSize = 0;

    pID3V2XFrameInfoType pID3V2XFrameInfo;
    pID3V22FrameInfoType pID3V22FrameInfo;

    //////////////////////////////////////////////////////////////////////
    if(!WavFlag)
    {
        iFileSize   =  RKFLength((FILE*)fHandle);
        iRemainSize = iFileSize;
        iReadSize = ID3_UpdateBufferData(fHandle, tempbuf, iRemainSize, 0);
    }
    else //wav
    {
        iFileSize   = ID3ChunkSize;
        iRemainSize = iFileSize;
        iReadSize = ID3_UpdateBufferData(fHandle, tempbuf, iRemainSize, ID3Offset);
    }

    if(iReadSize <= 0 )
        return -1;

    iRemainSize -= iReadSize;

    if (tempbuf[0] == 'I' && tempbuf[1] == 'D' && tempbuf[2] == '3'
        && (tempbuf[3] == 0x02 || tempbuf[3] == 0x03 || tempbuf[3] == 0x04))  // ID3V2.2, ID3V2.3, ID3V2.4
    {
        iTotalSize = ID3_GetID3V2xHeader(&tempbuf[0], &iFrameOffset, &iVersion);

        if ((iTotalSize == -1) || (iTotalSize == 0) || iTotalSize >= iFileSize)
        {
            return -1;
        }
        if ((tempbuf[5] & 0x80)!= 0)
            ID3ZeroFlag = 1;
        else
            ID3ZeroFlag = 0;
    }
    else
    {
        return -1;
    }

    if (iVersion < 3)
    {
        iOffset = sizeof(ID3V22FrameInfoType);
    }
    else
    {
        iOffset = sizeof(ID3V2XFrameInfoType);
    }

    for (iCurrentOffset = 10, j = 10; iCurrentOffset < iTotalSize;)
    {
        if((iReadSize - j) < iOffset)
        {
            int size = iReadSize - j;
            iReadSize = ID3_UpdateBufferData(fHandle, tempbuf, iRemainSize, iCurrentOffset);
            iRemainSize -= (iReadSize - size);
            j = 0;

            continue;
        }

        iFrameInfo = ID3_GetFrameInfo((char*) & tempbuf[j], iVersion);

        if (iFrameInfo != ID3_NONINFO)
        {
            iMetaCnt = 1;
            if (iVersion < 3)
            {
                pID3V22FrameInfo = (pID3V22FrameInfoType) & tempbuf[j];
                iFrameSize = ID3_Get3ByteValue(pID3V22FrameInfo->mSize);
            }
            else
            {
                pID3V2XFrameInfo = (pID3V2XFrameInfoType) & tempbuf[j];
                iFrameSize =  ID3_Get4byteIntValue(pID3V2XFrameInfo->mSize);
            }

            if (iFrameSize <= 0)
                return -1;

            /*field is too large,just reserve the half of MAX_ID3V2_FIELD*/
            if(iFrameSize > MAX_ID3V2_FIELD)
                iFrameSize = MAX_ID3V2_FIELD / 2;

            if(iFrameInfo == ID3_TRACK || iFrameInfo == ID3_TITLE || iFrameInfo == ID3_ARTIST
                || iFrameInfo == ID3_GENRE || iFrameInfo == ID3_ALBUM)
            {
                if((iReadSize - (j + iOffset)) < iFrameSize)
                {
                    int size = iReadSize - (j + iOffset);
                    iReadSize = ID3_UpdateBufferData(fHandle, tempbuf, iRemainSize, iCurrentOffset);
                    iRemainSize -= (iReadSize - size);
                    j = 0;

                    continue;
                }
            }

            switch (iFrameInfo)
            {
                case ID3_TRACK:
                    if ((iFrameSize - 1) >  sizeof(trackbuf))    //00��01��02��03
                        break;

                    ID3_GetMetaData((unsigned char*)&tempbuf[j+iOffset], (unsigned char *)trackbuf, iFrameSize);
                    trackIdSize = strlenwide1((uint16 *)trackbuf);

                    for (i = 0; i < trackIdSize; i++)
                    {
                        if (trackbuf[0] != 0x2F && trackbuf[i] == 0x2F)
                        {
                            nSize = i;
                            mSize = trackIdSize - i - 1;
                            break;
                        }
                    }

                    if (nSize == 0 && mSize == 0)
                    {
                        nSize = trackIdSize;
                        mSize = 0;
                    }

                    if (nSize < 4)
                    {
                        for (i = 0; i < (4 - nSize); i++)
                        {
                            pstId3v2x->nTrack[i] = 0x30;
                        }
                    }
                    else
                    {
                        i = 0;
                    }
                    memcpy((unsigned char *)(pstId3v2x->nTrack + i), (unsigned char *)trackbuf, nSize * 2);
                    break;

                    //case ID3_YEAR:
                    //ID3_GetMetaData((unsigned char*)&tempbuf[j+iOffset], (unsigned char *)pstId3v2x->mYear, iCopySize);
                    //break;

                case ID3_NOTUSEDFRAME:
                    break;

                case ID3_TITLE:
                    ID3_GetMetaData((unsigned char*)&tempbuf[j+iOffset], (unsigned char *)pstId3v2x->id3_title, iFrameSize);
                    break;

                case ID3_ARTIST:
                    ID3_GetMetaData((unsigned char*)&tempbuf[j+iOffset], (unsigned char *)pstId3v2x->id3_singer, iFrameSize);
                    break;

                case ID3_GENRE:
                    //zhuzhe data 1120
                    ID3_GetMetaData((unsigned char*)&tempbuf[j+iOffset], (unsigned char *)pstId3v2x->id3_genre, iFrameSize);
                    GenreID = Mp3GenreParse(pstId3v2x->id3_genre);
                    if (GenreID == -1)
                    {
                    }
                    else if (GenreID < ID3_GENRE_MAX_CNT)
                    {
                        Ascii2Unicode((UINT16*)ID3_GenreTable[GenreID], (UINT16*)(pstId3v2x->id3_genre),23, ID3_ENCODE_UTF8);  //�23���ֽ�
                    }
                    break;

                case ID3_ALBUM:
                    ID3_GetMetaData((unsigned char*)&tempbuf[j+iOffset], (unsigned char *)pstId3v2x->id3_album, iFrameSize);
                    break;

                default:
                    break;

            }

            j += (iOffset + iFrameSize);
            iCurrentOffset += (iOffset + iFrameSize);

            if (j >= iReadSize)
            {
                iReadSize = ID3_UpdateBufferData(fHandle, tempbuf, iRemainSize, iCurrentOffset);
                iRemainSize    -= iReadSize;
                j = 0;
            }
        }
        else
        {
            break;
        }
    }

    return iMetaCnt;
}

/******************************************************
Name: Mp3GenrePrase
Desc: judge whether the genre information is digital number.
Param: pGenre:song genre,unicode string.
Return: -1:string is genre information itself,other number represent genre serial number.
Author: xw
******************************************************/
_ATTR_ID3_TEXT_
int Mp3GenreParse(UINT16 *pGenre)
{
    int GenreID = 0;
    UINT32 Flag = 0;
    UINT16 *p = pGenre;
    UINT16 DefCount = 0;

    if (*p == '(')
    {
        if ((*(p+1) >= '0' )&& (*(p+1) <= '9'))
        {
            if ((*(p+2) >= '0') && (*(p+2) <= '9'))
            {
                if ((*(p+3) >= '0') && (*(p+3) <= '9'))
                {
                    if (*(p+4) == ')')
                        GenreID = ((*(p+1)-'0')*100+(*(p+2)-'0')*10+(*(p+3)-'0'));
                    else
                        return -1;

                }
                else if (*(p+3) == ')')
                {
                    GenreID = ((*(p+1)-'0')*10+(*(p+2)-'0'));
                }
                else
                    return -1;
            }
            else if (*(p+2) == ')')
            {
                GenreID =(*(p+1)-'0') ;
            }
            else
                return -1;
        }
        else
            return -1;
    }
    else
        return -1;

    return GenreID;
    //while (*p)
    //{
    //  if (*p >= '0' && *p <= '9')
    //   {
    //     Flag = 1;  // find number flag.
    //       DefCount = 0;
    //       GenreID = GenreID * 10 + (*p - '0');
    // }
    //   else if (Flag == 1)
    //   {
    //       break;
    //   }

    //   DefCount++;
    //   if (DefCount == 3) //if the frist 3 characters all are not number,then we think this information is genre information. (website always be followed by a bunch of number.)
    //       return -1;

    //   p++;
    //}

    //if (GenreID == 0 && Flag == 0)
    // {
    //     return -1;
    // }
}


/******************************************************
Name: WMAID3_Cpy
Desc: copy wma id3 information, filter the illegal character its code number is litter than 0x20.
Param: DBuffer:destination string.  SBuffer:source string.  Size:copy byte size
Return:
Global: null
Note:  null
Author: phc
Log:
******************************************************/
_ATTR_ID3_TEXT_
int WMAID3_Cpy(UINT16 *DBuffer, UINT16 *SBuffer, UINT16 Size)
{
    UINT16 i = 0;
    UINT16 *pTemp;

    pTemp = SBuffer;

    while (*pTemp <= 0x0020 && (i < Size / 2))
    {
        pTemp++;
        i++;
    }

    memcpy(DBuffer, pTemp, Size - i*2);

    return TRUE;
}

/******************************************************
Name: MediaID3_GetID3
Desc: media libary get id3 information of mp3
Param: file handle
Return:
Global: null
Note:  null
Author: FSH
Log:
******************************************************/
_ATTR_ID3_TEXT_
int MediaID3_GetID3(HANDLE fHandle, ID3V2X_INFO * pstId3V2x)
{
    unsigned char   *tempbuf;
    tempbuf  = ID3TempBuff;

    ID3_GetID3_V2X(fHandle, pstId3V2x, tempbuf, 0, 0, 0);
    ID3_GetID3_V1X(fHandle, pstId3V2x, (char *)tempbuf);

    return 0;
}

/******************************************************
Name: MediaID3_GetID3
Desc: media libary get id3 information of WAV
Param: file handle
Return:
Global: null
Note:  null
Author: ctf
Log:
******************************************************/
_ATTR_ID3_TEXT_
int WAV_GetID3(HANDLE fHandle, ID3V2X_INFO * pstId3V2x)
{
    int ret = -1;
    int offset = 0;
    int FileSize = 0, ChunkSize = 0;
    unsigned char buf[12];
    unsigned char ID[4];

    unsigned char   *tempbuf;
    tempbuf  = ID3TempBuff;

    FileSize = RKFLength((FILE*)fHandle);
    if(FileSize <= 0)
        return -1;

    if(FileRead(buf, 12, fHandle) < 12) return -1;
    if ((buf[0] != 'R') || (buf[1] != 'I') ||
            (buf[2] != 'F') || (buf[3] != 'F') ||
            (buf[8] != 'W') || (buf[9] != 'A') ||
            (buf[10] != 'V') || (buf[11] != 'E'))
    {
        return -1;
    }

    FileSize -= 12;
    offset += 12;

    while(FileSize > 0)
    {
        if(FileRead(ID, 4, fHandle) < 4) return -1;

        if(FileRead((uint8*)&ChunkSize, 4, fHandle) < 4) return -1;
        if(ChunkSize < 0) return -1;

        FileSize -= 8;
        offset += 8;

        if(memcmp(ID, "id3 ", 4) == 0)
        {
            ret = ID3_GetID3_V2X(fHandle, pstId3V2x, tempbuf, 1, ChunkSize, offset);
        }
        else
        {
            FileSeek(ChunkSize, FSEEK_CUR, fHandle);
            FileSize -= ChunkSize;
            offset += ChunkSize;
        }
    }

    return ret;
}

/******************************************************
Name:
Desc: APEV1 and APEV2 parse
Param: file handle
Return:
Global: null
Note:  null
Author: CTF
Log:
******************************************************/
_ATTR_ID3_TEXT_
static int APEV2_Parse(HANDLE fHandle, ID3V2X_INFO * pstId3v2x, unsigned char *tempbuf)
{
    int i, j;
    unsigned int version, size, fields, flag;
    unsigned int taginfolen, tagfieldlen, readsize = 0;
    TAG_ID tagid = TAGID_NULL;
    unsigned short* TargetBuf;

    if(FileRead((uint8*)&version, 4, fHandle) < 4) return -1;
    if(FileRead((uint8*)&size, 4, fHandle) < 4) return -1;      //���б�ǩ֡�ͱ�ǩβ���ܳ���, ������APE Tags Header
    if(FileRead((uint8*)&fields, 4, fHandle) < 4) return -1;    //��ǩ֡����
    if(FileRead((uint8*)&flag, 4, fHandle) < 4) return -1;

    if(fields <= 65536 && (flag & APE_TAG_FLAG_CONTAINS_HEADER))
    {
        FileSeek(8, FSEEK_CUR, fHandle);    //skip, 8byte�����ֽڣ�ȫΪ0

        for(i = 0; i < fields; i++)
        {
            if(FileRead((uint8*)&taginfolen, 4, fHandle) < 4) return -1; //֡���ݳ���
            FileSeek(4, FSEEK_CUR, fHandle);    //skip��4byte�����

            readsize += 8;
            if(readsize > size) break;

            for(j = 0; j < MAX_ID3_FIND_SIZE; j++)
            {
                if(FileRead(tempbuf + j, 1, fHandle) < 1) return -1;

                readsize++;
                if(readsize > size) break;

                if(tempbuf[j] == '\0') break; //֡��ʶ�Ľ����ַ�����Ϊ0
            }

            //tagfield is too long
            if(j == MAX_ID3_FIND_SIZE)
                return -1;

            tagfieldlen = strlen(tempbuf);  //֡��ʶ�ĳ���

            if(strcasecmp(tempbuf, APE_TAG_FIELD_TITLE) == 0) {
                tagid = TAGID_TITLE;
                TargetBuf = (unsigned short*)pstId3v2x->id3_title;
            } else if(strcasecmp(tempbuf, APE_TAG_FIELD_ARTIST) == 0) {
                tagid = TAGID_ARTIST;
                TargetBuf = (unsigned short*)pstId3v2x->id3_singer;
            } else if(strcasecmp(tempbuf, APE_TAG_FIELD_ALBUM) == 0) {
                tagid = TAGID_ALBUM;
                TargetBuf = (unsigned short*)pstId3v2x->id3_album;
            } else if(strcasecmp(tempbuf, APE_TAG_FIELD_TRACK) == 0) {
                tagid = TAGID_TRACK;
                TargetBuf = (unsigned short*)pstId3v2x->nTrack;
            } else if(strcasecmp(tempbuf, APE_TAG_FIELD_GENRE) == 0){
                tagid = TAGID_GENRE;
                TargetBuf = (unsigned short*)pstId3v2x->id3_genre;
            } else {
                tagid = TAGID_NULL;
            }

            memset(tempbuf, 0, tagfieldlen);

            if(tagid != TAGID_NULL)
            {
                if(taginfolen > MAX_ID3_FIND_SIZE) return -1;   //tag content is too long
                if(FileRead(tempbuf, taginfolen, fHandle) < taginfolen) return -1;  //��ȡ֡����
                readsize += taginfolen;
                if(readsize > size) break;

                Mp3Id3ToUnicode(TargetBuf, tempbuf, taginfolen, ID3_ENCODE_UTF8);
                if (tagid == TAGID_TRACK)
                {
                    uint32 len;
                    len = StrLenA(tempbuf);
                    //printf("\nlen = %d, tempStr = %s, 0x%x, 0x%x, 0x%x, 0x%x", len,tempbuf, tempbuf[0],tempbuf[1],tempbuf[2],tempbuf[3]);                
                    if ((0 < len) && (len < 4)) 
                    {
                        if (len == 1)
                        {
                            TargetBuf[3] = TargetBuf[0];
                            TargetBuf[2] = 0x30;
                            TargetBuf[1] = 0x30;
                            TargetBuf[0] = 0x30;
                        }
                        if (len == 2)
                        {
                            TargetBuf[3] = TargetBuf[1];
                            TargetBuf[2] = TargetBuf[0];
                            TargetBuf[1] = 0x30;
                            TargetBuf[0] = 0x30;
                        }
                        if (len == 3)
                        {
                            TargetBuf[3] = TargetBuf[2];
                            TargetBuf[2] = TargetBuf[1];
                            TargetBuf[1] = TargetBuf[0];
                            TargetBuf[0] = 0x30;
                        }
                    }
                }
            }
            else
            {
                FileSeek(taginfolen, FSEEK_CUR, fHandle);
                readsize += taginfolen;

                if(readsize > size) break;
            }
        }
    }
    return 0;
}

_ATTR_ID3_TEXT_
static int APEV1_Parse(HANDLE fHandle, ID3V2X_INFO * pstId3v2x, unsigned char *tempbuf, unsigned int size, unsigned int fields)
{
    int i, j;
    unsigned int taginfolen, tagfieldlen, readsize = 0;
    TAG_ID tagid = TAGID_NULL;
    unsigned short* TargetBuf;

    for(i = 0; i < fields; i++)
    {
        if(FileRead((uint8*)&taginfolen, 4, fHandle) < 4) return -1;
        FileSeek(4, FSEEK_CUR, fHandle);    //skip

        readsize += 8;
        if(readsize > size) break;

        for(j = 0; j < MAX_ID3_FIND_SIZE; j++)
        {
            if(FileRead(tempbuf + j, 1, fHandle) < 1) return -1;

            readsize++;
            if(readsize > size) break;

            if(tempbuf[j] == '\0') break;
        }

        //tagfield is too long
        if(j == MAX_ID3_FIND_SIZE)
            return -1;

        tagfieldlen = strlen(tempbuf);

        if(strcasecmp(tempbuf, APE_TAG_FIELD_TITLE) == 0) {
            tagid = TAGID_TITLE;
            TargetBuf = (unsigned short*)pstId3v2x->id3_title;
        } else if(strcasecmp(tempbuf, APE_TAG_FIELD_ARTIST) == 0) {
            tagid = TAGID_ARTIST;
            TargetBuf = (unsigned short*)pstId3v2x->id3_singer;
        } else if(strcasecmp(tempbuf, APE_TAG_FIELD_ALBUM) == 0) {
            tagid = TAGID_ALBUM;
            TargetBuf = (unsigned short*)pstId3v2x->id3_album;
        } else if(strcasecmp(tempbuf, APE_TAG_FIELD_TRACK) == 0) {
            tagid = TAGID_TRACK;
            TargetBuf = (unsigned short*)pstId3v2x->nTrack;
        } else if(strcasecmp(tempbuf, APE_TAG_FIELD_GENRE) == 0){
            tagid = TAGID_GENRE;
            TargetBuf = (unsigned short*)pstId3v2x->id3_genre;
        } else {
            tagid = TAGID_NULL;
        }

        memset(tempbuf, 0, tagfieldlen);

        if(tagid != TAGID_NULL)
        {
            if(taginfolen > MAX_ID3_FIND_SIZE) return -1;   //tag content is too long
            if(FileRead(tempbuf, taginfolen, fHandle) < taginfolen) return -1;  //��ȡ֡����
            readsize += taginfolen;
            if(readsize > size) break;

            Mp3Id3ToUnicode(TargetBuf, tempbuf, taginfolen, ID3_ENCODE_UTF8);
            if (tagid == TAGID_TRACK)
            {
                uint32 len;
                len = StrLenA(tempbuf);
                //printf("\nlen = %d, tempStr = %s, 0x%x, 0x%x, 0x%x, 0x%x", len,tempbuf, tempbuf[0],tempbuf[1],tempbuf[2],tempbuf[3]);                
                if ((0 < len) && (len < 4)) 
                {
                    if (len == 1)
                    {
                        TargetBuf[3] = TargetBuf[0];
                        TargetBuf[2] = 0x30;
                        TargetBuf[1] = 0x30;
                        TargetBuf[0] = 0x30;
                    }
                    if (len == 2)
                    {
                        TargetBuf[3] = TargetBuf[1];
                        TargetBuf[2] = TargetBuf[0];
                        TargetBuf[1] = 0x30;
                        TargetBuf[0] = 0x30;
                    }
                    if (len == 3)
                    {
                        TargetBuf[3] = TargetBuf[2];
                        TargetBuf[2] = TargetBuf[1];
                        TargetBuf[1] = TargetBuf[0];
                        TargetBuf[0] = 0x30;
                    }
                }
            }
        }
        else
        {
            FileSeek(taginfolen, FSEEK_CUR, fHandle);
            readsize += taginfolen;
            if(readsize > size) break;
        }
    }
    return 0;
}

/*******************************************************************
APE tag�������metadata���ݣ�����������ݳ��ߡ�ר�����ȵ�;

APE tag�������汾����APEV1��APEV2, APEV1һ������ļ���ĩβ;

���б�ʾ��С�����ζ���С��
********************************************************************/
_ATTR_ID3_TEXT_
int APEInfo_Parse(HANDLE fHandle, ID3V2X_INFO * pstId3V2x)
{
    unsigned char   *tempbuf;
    unsigned int FileSize;

    unsigned char ID[8];
    unsigned int version, size, fields, flag;

    tempbuf = ID3TempBuff;
    memset(tempbuf, 0, MAX_ID3_FIND_SIZE);

    FileSize = RKFLength((FILE*)fHandle);
    if(FileSize <= 0)
        return -1;

    //seek to APE tags footer
    FileSeek(FileSize - APE_TAG_FOOTER_BYTES, FSEEK_SET, fHandle);
    if(FileRead(ID, 8, fHandle) < 8) return -1;
    if(memcmp(ID, "APETAGEX", 8)) return -1;

    if(FileRead((uint8*)&version, 4, fHandle) < 4) return -1;
    if(FileRead((uint8*)&size, 4, fHandle) < 4) return -1;  //���б�ǩ֡�ͱ�ǩβ���ܳ���, ������APE Tags Header�ĳ���
    if(FileRead((uint8*)&fields, 4, fHandle) < 4) return -1; //��ǩ֡����
    if(FileRead((uint8*)&flag, 4, fHandle) < 4) return -1;  //�����

    //if(version > APE_TAG_VERSION) return -1;
    //if(size - APE_TAG_FOOTER_BYTES > (1024 * 1024 * 16)) return -1;
    if(fields > 65536) return -1;
    if(flag & APE_TAG_FLAG_IS_HEADER) return -1;
    if(size > FileSize) return -1;

    //seek to APE tags header
    FileSeek(FileSize - (size + APE_TAG_FOOTER_BYTES), FSEEK_SET, fHandle);
    if(FileRead(ID, 8, fHandle) < 8) return -1;
    if(memcmp(ID, "APETAGEX", 8))
    {
        //APEV1������APE Tags Header
        FileSeek(APE_TAG_FOOTER_BYTES - 8, FSEEK_CUR, fHandle);
        APEV1_Parse(fHandle, pstId3V2x, tempbuf, size, fields);
    }
    else
    {
        //APEV2����APE Tags Header
        APEV2_Parse(fHandle, pstId3V2x, tempbuf);
    }

    return 0;
}

/******************************************************
Name:
Desc: OGG and FLAC Comments parse
Param: file handle
Return:
Global: null
Note:  null
Author: CTF
Log:
******************************************************/
_ATTR_ID3_TEXT_
static int vorbis_comment_parse(HANDLE fHandle, ID3V2X_INFO * pstId3v2x, unsigned char *tempbuf, unsigned int packet_size)
{
    int i;
    unsigned int vendorlen;         //���������Ϣ��ռ�õ��ֽ���
    unsigned int commentslen;       //����ע�͵��ܳ���
    unsigned int tagfieldlen;       //������ǩ���ܳ���
    unsigned int taginfolen = 0;    //������ǩ���ݵĳ���
    unsigned int readsize = 0;      //�Ѷ�ȡ��ע�ͳ���

    unsigned char buf[4];
    unsigned short* TargetBuf;
    TAG_ID tagid = TAGID_NULL;

    if(packet_size > MAX_PAGE_SIZE)
        return -1;

    if(FileRead((uint8*)buf, 4, fHandle) < 4) return -1;
    vendorlen = ((int)buf[0])|(((int)buf[1])<<8)|(((int)buf[2])<<16)|(((int)buf[3])<<24);
    if(vendorlen < 0 || vendorlen > packet_size)
        return -1;

    FileSeek(vendorlen + 4, FSEEK_CUR, fHandle);    //skip, ���������Ϣ + 4byte�����ֽ�
    commentslen = packet_size - vendorlen - 8;

    while(1) //����OGG, ��ע�Ͱ��ĵ�һ��page, û�ж�ȡ��METADATA_BLOCK_PICTURE��־������Ϊ������ר��ͼƬ
    {
        //��ȡ��ǩ�ַ�����ռ�õ��ֽ���
        if(FileRead((uint8*)buf, 4, fHandle) < 4) return -1;
        readsize += 4;
        if(readsize > commentslen) break;

        tagfieldlen = ((int)buf[0])|(((int)buf[1])<<8)|(((int)buf[2])<<16)|(((int)buf[3])<<24);
        if(tagfieldlen < 0 || tagfieldlen > commentslen)
            return -1;

        memset(tempbuf, 0, MAX_ID3_FIND_SIZE);
        for(i = 0; i < MAX_ID3_FIND_SIZE; i++)
        {
            if(FileRead(tempbuf + i, 1, fHandle) < 1) return -1;

            readsize++;
            if(readsize > commentslen) break;

            if(tempbuf[i] == '=') break; //֡��ʶ�Ľ����ַ�����Ϊ0
        }

        //tagfield is too long
        if(i == MAX_ID3_FIND_SIZE)
            return -1;

        taginfolen = tagfieldlen - (i + 1);

        if(strncasecmp(tempbuf, VORBIS_TAG_FIELD_TITLE, i) == 0) {
            tagid = TAGID_TITLE;
            TargetBuf = (unsigned short*)pstId3v2x->id3_title;
        } else if(strncasecmp(tempbuf, VORBIS_TAG_FIELD_ARTIST, i) == 0) {
            tagid = TAGID_ARTIST;
            TargetBuf = (unsigned short*)pstId3v2x->id3_singer;
        } else if(strncasecmp(tempbuf, VORBIS_TAG_FIELD_ALBUM, i) == 0) {
            tagid = TAGID_ALBUM;
            TargetBuf = (unsigned short*)pstId3v2x->id3_album;
        } else if(strncasecmp(tempbuf, VORBIS_TAG_FIELD_TRACKNUMBER, i) == 0) {
            tagid = TAGID_TRACK;
            TargetBuf = (unsigned short*)pstId3v2x->nTrack;
        } else if(strncasecmp(tempbuf, VORBIS_TAG_FIELD_GENRE, i) == 0){
            tagid = TAGID_GENRE;
            TargetBuf = (unsigned short*)pstId3v2x->id3_genre;
        } else {
            tagid = TAGID_NULL;
        }

        memset(tempbuf, 0, i+1);
        if(tagid != TAGID_NULL)
        {
            if(taginfolen > MAX_ID3_FIND_SIZE) return -1;   //tag content is too long
            if(FileRead(tempbuf, taginfolen, fHandle) < taginfolen) return -1;  //��ȡ��ǩ����
            readsize += taginfolen;
            if(readsize > commentslen) break;

            Mp3Id3ToUnicode(TargetBuf, tempbuf, taginfolen, ID3_ENCODE_UTF8);
            
            if (tagid == TAGID_TRACK)
            {
                uint32 len;
                len = StrLenA(tempbuf);
                //printf("\nlen = %d, tempStr = %s, 0x%x, 0x%x, 0x%x, 0x%x", len,tempbuf, tempbuf[0],tempbuf[1],tempbuf[2],tempbuf[3]);                
                if ((0 < len) && (len < 4)) 
                {
                    if (len == 1)
                    {
                        TargetBuf[3] = TargetBuf[0];
                        TargetBuf[2] = 0x30;
                        TargetBuf[1] = 0x30;
                        TargetBuf[0] = 0x30;
                    }
                    if (len == 2)
                    {
                        TargetBuf[3] = TargetBuf[1];
                        TargetBuf[2] = TargetBuf[0];
                        TargetBuf[1] = 0x30;
                        TargetBuf[0] = 0x30;
                    }
                    if (len == 3)
                    {
                        TargetBuf[3] = TargetBuf[2];
                        TargetBuf[2] = TargetBuf[1];
                        TargetBuf[1] = TargetBuf[0];
                        TargetBuf[0] = 0x30;
                    }
                }
            }
        }
        else
        {
            FileSeek(taginfolen, FSEEK_CUR, fHandle);
            readsize += taginfolen;
            if(readsize > commentslen) break;
        }
    }

    return 0;
}

/*****************************************************************************
 ogg�ļ���������ҳ��page�����, ҳ��ʶΪOggS��ASCII��;

 ҳ��ҳͷ����pageheader����ҳ���ݣ�pagedata�����, һ��page�ɰ��������(packet);

 Ogg�ļ�������γɱ���������������ǰ����������ͷ���������ļ��е�˳�������ǣ�
    ��ʶͷ��identification header����ע��ͷ��comment header����װ��ͷ��Setup Header��,
    ͨ������£���ʶͷ�ָ���ogg�ļ���1ҳ��ע��ͷ��װ��ͷ�ָ���ogg�ļ���2ҳ.
******************************************************************************/
_ATTR_ID3_TEXT_
int OGGInfo_Parse(HANDLE fHandle, ID3V2X_INFO * pstId3V2x)
{
    int i;
    unsigned char buf[8];

    //page header info
    unsigned char version;              //Ogg�ļ���ʽ�İ汾, ĿǰΪ0
    unsigned char header_type_flag;     //ҳͷ�����ͱ�ʶ, ��ʶ��ǰҳ��������
    int granule_position;               //ý�������ز�����Ϣ
    unsigned char number_page_segments; //��ҳ��segment_table����segement�ĸ���
    unsigned char segment_table[MAX_NUM_PAGE_SEGMENTS]; //���γ��ȱ�, ��ʾÿ��segment�ĳ���
    unsigned int total_segments_size;                   //����segment�ܳ���

    //packet header info
    unsigned char packet_header_type;   //packet����
    unsigned int packet_size, picture_size;
    unsigned char header_flag[6];

    unsigned char   *tempbuf;
    tempbuf = ID3TempBuff;
    memset(segment_table, 0, MAX_NUM_PAGE_SEGMENTS);

    while(1)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        if(FileRead(buf, 4, fHandle) < 4) return -1; //��ȡҳ��ʶ
        if(memcmp(buf, "OggS", 4)) return -1;

        if(FileRead((uint8*)&version, 1, fHandle) < 1) return -1;
        if(version != STREAM_STRUCTURE_VERSION) return -1;

        if(FileRead((uint8*)&header_type_flag, 1, fHandle) < 1) return -1;

        if(FileRead(buf, 8, fHandle) < 8) return -1;
        granule_position = ((int)buf[0])|(((int)buf[1])<<8)|(((int)buf[2])<<16)|(((int)buf[3])<<24)
                            | (((int)buf[4])<<32)|(((int)buf[5])<<40)|(((int)buf[6])<<48)|(((int)buf[7])<<56);

        FileSeek(12, FSEEK_CUR, fHandle);    //seek 4�ֽڵ������к�, 4�ֽڸ�ҳ���߼����е����к�, 4�ֽڵ�CRCУ����

        //��ȡ��ҳ����������
        if(FileRead((uint8*)&number_page_segments, 1, fHandle) < 1) return -1;
        if(number_page_segments > MAX_NUM_PAGE_SEGMENTS) return -1;

        //��ȡ���γ��ȱ�
        if(FileRead(segment_table, number_page_segments, fHandle) < number_page_segments) return -1;

        if(header_type_flag != STREAM_CONTINUS_PAGE)
        {
            //��ȡ��ͷ���͵ı�ʶ
            if(FileRead((uint8*)&header_type_flag, 1, fHandle) < 1) return -1;
            if(FileRead(header_flag, 6, fHandle) < 6) return -1;

            //�жϸð��Ƿ�Ϊע�Ͱ�
            if(header_type_flag == 3 && strncmp(header_flag, "vorbis", 6) == 0)
            {
                packet_size = 0;
                for(i = 0; i < number_page_segments; i++)
                {
                    if(segment_table[i] != 0xff)
                    {
                        packet_size += segment_table[i];
                        break;
                    }
                    packet_size += segment_table[i];
                }

                vorbis_comment_parse(fHandle, pstId3V2x, tempbuf, packet_size - 7); //packet_size - "03vorbis"
                break;
            }
            else
            {
                total_segments_size = 0;
                FileSeek(-7, FSEEK_CUR, fHandle);
                for(i = 0; i < number_page_segments; i++)
                    total_segments_size += segment_table[i];

                FileSeek(total_segments_size, FSEEK_CUR, fHandle);
            }
        }
        else
        {
            total_segments_size = 0;

            for(i = 0; i < number_page_segments; i++)
                    total_segments_size += segment_table[i];

            FileSeek(total_segments_size, FSEEK_CUR, fHandle);
        }

        if(header_type_flag == STREAM_LAST_PAGE)
            break;
    }

    return 0;
}

_ATTR_ID3_TEXT_
int FLACInfo_Parse(HANDLE fHandle, ID3V2X_INFO * pstId3V2x)
{
    unsigned char tag, type;     //����Ϣ����
    unsigned int metadata_block_size;   //��Ϣ���С��������ͷ��С
    unsigned char   *tempbuf;
    unsigned int FileSize, ReadSize = 0;

    unsigned char ID[4];
    unsigned char metadata_block_header[4]; //��Ϣ��ͷ���ݣ����������ͺͿ��С��Ϣ

    tempbuf = ID3TempBuff;
    memset(tempbuf, 0, MAX_ID3_FIND_SIZE);

    FileSize = RKFLength((FILE*)fHandle);
    if(FileSize <= 0)
        return -1;

    if(FileRead(ID, 4, fHandle) < 4) return -1;
    if(memcmp(ID, "fLaC", 4)) return -1;
    ReadSize += 4;

    while(ReadSize < FileSize)
    {
        //��ȡmetadata ͷ��Ϣ
        if(FileRead(metadata_block_header, 4, fHandle) < 4) return -1;
        ReadSize += 4;

        tag = (metadata_block_header[0] & 0x80) >> 7;
        type = metadata_block_header[0] & 0x7f; //��ȡmetadata����
        metadata_block_size = ((int)metadata_block_header[3])|(((int)metadata_block_header[2])<<8)|(((int)metadata_block_header[1])<<16);

        if(type == METADATA_TYPE_VORBIS_COMMENT)
        {
            vorbis_comment_parse(fHandle, pstId3V2x, tempbuf, metadata_block_size);
            break;
        }
        else
        {
            FileSeek(metadata_block_size, FSEEK_CUR, fHandle);
            ReadSize += metadata_block_size;
        }

        if(tag == 1) break; //���һ��metadataΪ 1,����Ϊ 0
    }

    return 0;
}


/*----------------------------------------------------------------------
Name  :  ID3GetFileType
Desc  :  save the audio id3 information to struct FILE_SAVE_STRUCT.
Params:  [hFile]     -- audio file handle.
         [pSaveStru] -- the obtained id3 informatin
         [FileName]  -- audio file name,for judge the song type.!! must be short file name.
Return:  0: success; 1: failure
----------------------------------------------------------------------*/
_ATTR_SYS_CODE_
UINT8 ID3GetFileType(UINT8 *pBuffer, UINT8 *pStr)
{
    UINT8 Len;
    UINT8 Retval = 0xff;
    UINT8 i;

    i = 0;
    Len = strlen((char*)pStr);

    while (i <= Len)
    {
        i += 3;
        if ((*(pBuffer + 0) == *(pStr + 0)) && (*(pBuffer + 1) == *(pStr + 1)) &&
                (*(pBuffer + 2) == *(pStr + 2)))
        {
            break;
        }
        pStr += 3;
    }

    if (i <= Len)
    {
        Retval = i / 3;
    }
    return (Retval);
}


/*----------------------------------------------------------------------
Name  :  GetAudioId3Info
Desc  :  save the audio id3 information to struct FILE_SAVE_STRUCT.
Params:  [hFile]     -- audio file handle.
         [pSaveStru] -- the obtained id3 informatin
         [FileName]  -- audio file name,for judge the song type.!! must be short file name.
Return:  0: success; 1: failure
----------------------------------------------------------------------*/
_ATTR_ID3_TEXT_
UINT16 GetAudioId3Info(HANDLE hFile, ID3V2X_INFO *pId3Info, char *FileName)
{
    ID3 id3;
    UINT16 codec = ID3GetFileType(FileName,(UINT8 *)MusicFileExtString);
    UINT16 UnicodeBuff[MEDIA_ID3_SAVE_CHAR_NUM];

    int GenreID = -1;

    RKFileFuncInit();
    switch (codec)
    {
        case 1:     // mp1
        case 2:     // mp2
        case 3:     // mp3
        case 8:     // AAC
            //FileFuncInit();
            MediaID3_GetID3(hFile, pId3Info);
            break;

#ifdef  WMA_DEC_INCLUDE
        case 4:
            memset(&id3, 0, sizeof(id3));
            if (AP_SUCESS == WMA_ParseAsf_Header((AP_FILE)hFile, &id3))
            {
                WMAID3_Cpy((UINT16 *)pId3Info->id3_title, (UINT16*)id3.Title, MEDIA_ID3_SAVE_CHAR_NUM * 2);
                WMAID3_Cpy((UINT16 *)pId3Info->id3_singer, (UINT16*)id3.Author, MEDIA_ID3_SAVE_CHAR_NUM * 2);
                WMAID3_Cpy((UINT16 *)pId3Info->id3_album, (UINT16*)id3.Album, MEDIA_ID3_SAVE_CHAR_NUM * 2);
                WMAID3_Cpy((UINT16 *)pId3Info->id3_genre, (UINT16*)id3.Genre, MEDIA_ID3_SAVE_CHAR_NUM * 2);

                if (id3.Track[0] < 9999)
                {
                    pId3Info->nTrack[0] = id3.Track[0] / 1000 + 0x30;
                    id3.Track[0] =  id3.Track[0] % 1000;
                    pId3Info->nTrack[1] = id3.Track[0] / 100 + 0x30;
                    id3.Track[0] =  id3.Track[0] % 100;
                    pId3Info->nTrack[2] = id3.Track[0] / 10 + 0x30;
                    id3.Track[0] =  id3.Track[0] % 10;
                    pId3Info->nTrack[3] = id3.Track[0] + 0x30;
                }


            }
            break;
#endif

        case 9:     // M4A
        case 11:    // MP4
        case 12:    // 3GP
            Aac_GetID3((FILE *)hFile, pId3Info, codec);
            break;

        case 5:     //WAV
            WAV_GetID3(hFile, pId3Info);
            break;

        case 6:     // APE
            APEInfo_Parse(hFile, pId3Info);
            break;

        case 7:     // FLAC
            FLACInfo_Parse(hFile, pId3Info);
            break;

        case 10:    //OGG
            OGGInfo_Parse(hFile, pId3Info);
            break;
    }

    return 0;
}

_ATTR_ID3_TEXT_
void Aac_GetID3(FILE *fp, ID3V2X_INFO *Id3V2x, UINT16 codec)
{
    int        res;
    int        pos = 0;
    ATOM_FRAME ftypframe;

    memset(&ftypframe, 0, sizeof(ATOM_FRAME));

    (int)FileSeek(0, SEEK_SET, (HANDLE)fp);

    if (FileRead((uint8*) &ftypframe, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER))
    {
        return;
    }

    if (strncmp(ftypframe.Header.Atomtype, "ftyp", 4) != 0)
    {
        /* TYPE OTHER */
        //DEBUG("FILE FORMAT ERROR\n");//hoshi
        return;
    }

    ftypframe.Header.Length = Lit2Big32bit(ftypframe.Header.Length);
    if(ftypframe.Header.Length < ATOMHOFS)
    {
        return;
    }
    seekToRelativeOffset(fp, ftypframe.Header.Length - sizeof(ATOM_HEADER) - 1);
    pos = FileTell((HANDLE)(fp));

    if(codec == 9)
    {
        /*-----------------*/
        /* m4a file format */
        /*-----------------*/

        /* Search moov.udta.meta.ilst */
        res = atomitunessearch(fp);
        if (res == 1)
        {
            /* Found iTunes ATOM */
        }
        else
        {
            /* Next Searching moov/meta */
            FileSeek(pos, SEEK_SET, (HANDLE) fp);
            res = atomid32search(fp);
            if (res == 2)
            {
                /* Found ID3 ATOM */
            }
            else
            {
                /* Can't Find ID3 ATOM */
                //DEBUG("M4A FORMAT : Can't Find ID3 ATOM\n");//hoshi
                return;
            }
        }
    }
    else
    {
        /*-----------------------*/
        /* other aac file format */
        /*-----------------------*/

        /* Search moov/meta */
        res = atomid32search(fp);
        if (res == 2)
        {
            /* Found ID3 ATOM */
        }
        else
        {
            FileSeek(pos, SEEK_SET, (HANDLE) fp);
            res = atomitunessearch(fp);
            if (res == 1)
            {
                /* Found iTunes ATOM */
            }
            else
            {
                /* Can't Find ID3 ATOM */
                //DEBUG("Other AAC FORMAT : Can't Find ID3 ATOM\n");//hoshi
                return;
            }
        }
    }
    if (res == 1)
    {
        res = get_iTunes_info(fp, Id3V2x);
    }
    else if (res == 2)
    {
        pos = FileTell((HANDLE)(fp)) + 14;
        (int) FileSeek(pos, SEEK_SET, (HANDLE) fp);
        res = get_id3_info(fp, Id3V2x);
    }
}

_ATTR_ID3_TEXT_
static int get_iTunes_info(FILE *fp, ID3V2X_INFO *Id3V2x)
{
    int res = 0;
    int cnt = 0;
    int pos = 0;

    pos = FileTell((HANDLE)(fp));

    for (cnt = 0; cnt < 4; cnt++)
    {
        FileSeek(pos, SEEK_SET, (HANDLE) fp);

        res = seek_iTunes_frame(fp, cnt);
        if(res < 0)
        {
            //DEBUG("Not Found cnt=%d Frame\n", cnt);//hoshi
            continue;
        }

        if(cnt == 0)
        {
            res = parse_iTunes_frame(fp, (unsigned short *)Id3V2x->id3_title);     /* �^�C�g���擾 */
        }
        else if(cnt == 1)
        {
            res = parse_iTunes_frame(fp, (unsigned short *)Id3V2x->id3_singer);    /* �̎薼�擾 */
        }
        else if(cnt == 2)
        {
            res = parse_iTunes_frame(fp, (unsigned short *)Id3V2x->id3_album);     /* �A���o�����擾 */
        }
        else if(cnt == 3)
        {
            res = parse_iTunes_frame(fp, (unsigned short *)Id3V2x->id3_genre);     /* �W�������擾 */
        }

        if(res < 0)
        {
            return -1;
        }
    }

    return res;
}

_ATTR_ID3_TEXT_
static int get_id3_info(FILE *fp, ID3V2X_INFO *Id3V2x)
{
    int             res = 0;
    int             cnt = 0;
    int             pos = 0;

    res = getheader(fp);
    if (res < 0)
    {
        return -1;
    }

    pos = FileTell((HANDLE)(fp));

    if (Id3v2_header.Ver[0] == 0x02)
    {
        /* ID3v2.2 */
        for (cnt = 0; cnt < 4; cnt++)
        {
            FileSeek(pos, SEEK_SET, (HANDLE) fp);
            res = seek_v22_frame(fp, cnt);
            if(res < 0)
            {
                continue;
            }

            if(cnt == 0)
            {
                res = parse_v22_frame(fp, (unsigned short *)Id3V2x->id3_title);
            }
            else if(cnt == 1)
            {
                res = parse_v22_frame(fp, (unsigned short *)Id3V2x->id3_singer);
            }
            else if(cnt == 2)
            {
                res = parse_v22_frame(fp, (unsigned short *)Id3V2x->id3_album);
            }
            else if(cnt == 3)
            {
                res = parse_v22_frame(fp, (unsigned short *)Id3V2x->id3_genre);
            }
            if(res < 0)
            {
                return -1;
            }
        }
    }
    else
    {
        /* ID3v2.3 & ID3v2.4 */
        for (cnt = 0; cnt < 4; cnt++)
        {
            (int) FileSeek(pos, SEEK_SET, (HANDLE) fp);
            res = seek_v23_frame(fp, cnt);
            if(res < 0)
            {
                continue;
            }

            if(cnt == 0)
            {
                res = parse_v23_frame(fp, (unsigned short *)Id3V2x->id3_title);
            }
            else if(cnt == 1)
            {
                res = parse_v23_frame(fp, (unsigned short *)Id3V2x->id3_singer);
            }
            else if(cnt == 2)
            {
                res = parse_v23_frame(fp, (unsigned short *)Id3V2x->id3_album);
            }
            else if(cnt == 3)
            {
                res = parse_v23_frame(fp, (unsigned short *)Id3V2x->id3_genre);
            }
            if(res < 0)
            {
                return -1;
            }
        }
    }

    return res;
}

_ATTR_ID3_TEXT_
static int seek_iTunes_frame(FILE *fp, int cnt)
{
    int     res = -1;
    int     pos = 0;
    //int     upper = 0;
    int     size = 0;
    UINT32  total_size;
    char    fid[5];
    char    frame_id[3][5] =
    {
        {0xA9, 0x6E, 0x61, 0x6D, 0x00},//title
        {0xA9, 0x41, 0x52, 0x54, 0x00},//singer
        {0xA9, 0x61, 0x6C, 0x62, 0x00} //album
    };
    char    frame_id2[2][5] =
    {
        {0x67, 0x6E, 0x72, 0x65, 0x00},//genre
        {0xA9, 0x67, 0x65, 0x6E, 0x00} //genre
    };
    ITUNES_FRAME iTunes_frame;

    pos = FileTell((HANDLE)(fp));
    //upper = pos + iTunessize;
    total_size = 8;

    //DEBUG("cnt = %d\n", cnt);//hoshi
    //DEBUG("pos = 0x%x, upper = 0x%x\n", pos, upper);//hoshi

    /* �Ō�܂Ō��� */
    //while (pos <= upper)
    while (total_size < iTunessize)
    {
        /* iTunes Frame Header Read */
        if (FileRead((uint8*) &iTunes_frame, (1 * sizeof(ITUNES_FRAME)), (HANDLE) fp) < sizeof(ITUNES_FRAME))
        {
            return -1;
        }

        strncpy(fid, iTunes_frame.Header.Atomtype, 4);
        fid[4] = '\0';
        //DEBUG("fid = %s\n", fid);//hoshi

        size =
            ((iTunes_frame.Header.Length & 0x000000FF) << 24) +
            ((iTunes_frame.Header.Length & 0x0000FF00) << 8 ) +
            ((iTunes_frame.Header.Length & 0x00FF0000) >> 8 ) +
            ((iTunes_frame.Header.Length & 0xFF000000) >> 24);
        //DEBUG("size = 0x%x\n", size);//hoshi
        if(size <= 0)
        {
            return -1;
        }

        if(cnt < 3)
        {
            if (strncmp(fid, frame_id[cnt], 4) == 0)
            {
                //DEBUG("cnt = %d\n", cnt);//hoshi
                //DEBUG("[FIND]pos = 0x%x\n", pos);//hoshi
                if ((int) FileSeek(pos, SEEK_SET, (HANDLE) fp) != 0)
                {
                    return -1;
                }
                res = 0;
                break;
            }
        }
        else
        {
            if ((strncmp(fid, frame_id2[0], 4) == 0) || (strncmp(fid, frame_id2[1], 4) == 0))
            {
                //DEBUG("cnt = %d\n", cnt);//hoshi
                //DEBUG("[FIND]pos = 0x%x\n", pos);//hoshi
                if ((int) FileSeek(pos, SEEK_SET, (HANDLE) fp) != 0)
                {
                    return -1;
                }
                res = 0;
                break;
            }
        }

        //DEBUG("[while] pos = 0x%x, cur = 0x%x\n", pos, pos+size);//hoshi
        FileSeek(pos+size, SEEK_SET, (HANDLE) fp);
        pos = FileTell((HANDLE)(fp));
        total_size += size;
    }

    return res;
}
_ATTR_ID3_TEXT_
static int seek_v22_frame(FILE *fp, int cnt)
{
    int     res = -1;
    int     pos = 0;
    int     size = 0;
    int     total_size = 0;
    char    fid[5];
    char    frame_id[4][5] =
    {
        "TT2", "TP2", "TAL", "TCO"
    };
    ID3V22_FRAME        id3v22_frame;

    pos = FileTell((HANDLE)(fp));

    //while (pos <= Id3v2size)
    while (total_size < Id3v2size)
    {
        /* ID3v22 Frame Header Read */
        if (FileRead((uint8*) &id3v22_frame, (1 * sizeof(ID3V22_FRAME)), (HANDLE) fp) < sizeof(ID3V22_FRAME))
        {
            return -1;
        }

        strncpy(fid, id3v22_frame.Id, 3);

        size =
              ((unsigned char)(id3v22_frame.Size[0]) << 16)
            + ((unsigned char)(id3v22_frame.Size[1]) << 8)
            + ((unsigned char)(id3v22_frame.Size[2]));

        if(size <= 0)
        {
            return -1;
        }

        if (strncmp(fid, frame_id[cnt], 3) == 0)
        {
            if ((int) FileSeek(pos, SEEK_SET, (HANDLE) fp) != 0)
            {
                return -1;
            }
            res = 0;
            break;
        }
#if 1
        else if((strncmp(fid, "BUF", 3) != 0)
                    && (strncmp(fid, "COM", 3) != 0)
                    && (strncmp(fid, "CRA", 3) != 0)
                    && (strncmp(fid, "CNT", 3) != 0)
                    && (strncmp(fid, "CRM", 3) != 0)
                    && (strncmp(fid, "EQU", 3) != 0)
                    && (strncmp(fid, "ETC", 3) != 0)
                    && (strncmp(fid, "GEO", 3) != 0)
                    && (strncmp(fid, "IPL", 3) != 0)
                    && (strncmp(fid, "LNK", 3) != 0)
                    && (strncmp(fid, "MCI", 3) != 0)
                    && (strncmp(fid, "MLL", 3) != 0)
                    && (strncmp(fid, "PIC", 3) != 0)
                    && (strncmp(fid, "POP", 3) != 0)
                    && (strncmp(fid, "RVA", 3) != 0)
                    && (strncmp(fid, "REV", 3) != 0)
                    && (strncmp(fid, "SLT", 3) != 0)
                    && (strncmp(fid, "STC", 3) != 0)
                    && (strncmp(fid, "TAL", 3) != 0)
                    && (strncmp(fid, "TBP", 3) != 0)
                    && (strncmp(fid, "TCO", 3) != 0)
                    && (strncmp(fid, "TCP", 3) != 0)
                    && (strncmp(fid, "TCM", 3) != 0)
                    && (strncmp(fid, "TCR", 3) != 0)
                    && (strncmp(fid, "TDA", 3) != 0)
                    && (strncmp(fid, "TDY", 3) != 0)
                    && (strncmp(fid, "TEN", 3) != 0)
                    && (strncmp(fid, "TXT", 3) != 0)
                    && (strncmp(fid, "TFT", 3) != 0)
                    && (strncmp(fid, "TIM", 3) != 0)
                    && (strncmp(fid, "TT1", 3) != 0)
                    && (strncmp(fid, "TT2", 3) != 0)
                    && (strncmp(fid, "TT3", 3) != 0)
                    && (strncmp(fid, "TOA", 3) != 0)
                    && (strncmp(fid, "TOF", 3) != 0)
                    && (strncmp(fid, "TOL", 3) != 0)
                    && (strncmp(fid, "TOT", 3) != 0)
                    && (strncmp(fid, "TOR", 3) != 0)
                    && (strncmp(fid, "TP1", 3) != 0)
                    && (strncmp(fid, "TP2", 3) != 0)
                    && (strncmp(fid, "TP3", 3) != 0)
                    && (strncmp(fid, "TP4", 3) != 0)
                    && (strncmp(fid, "TPA", 3) != 0)
                    && (strncmp(fid, "TPB", 3) != 0)
                    && (strncmp(fid, "TRC", 3) != 0)
                    && (strncmp(fid, "TRD", 3) != 0)
                    && (strncmp(fid, "TRK", 3) != 0)
                    && (strncmp(fid, "TSC", 4) != 0)
                    && (strncmp(fid, "TSI", 3) != 0)
                    && (strncmp(fid, "TSS", 3) != 0)
                    && (strncmp(fid, "TYE", 3) != 0)
                    && (strncmp(fid, "TXX", 3) != 0)
                    && (strncmp(fid, "TLA", 3) != 0)
                    && (strncmp(fid, "TLE", 3) != 0)
                    && (strncmp(fid, "TMT", 3) != 0)
                    && (strncmp(fid, "TKE", 3) != 0)
                    && (strncmp(fid, "UFI", 3) != 0)
                    && (strncmp(fid, "ULT", 3) != 0)
                    && (strncmp(fid, "WCM", 3) != 0)
                    && (strncmp(fid, "WCP", 3) != 0)
                    && (strncmp(fid, "WAF", 3) != 0)
                    && (strncmp(fid, "WAR", 3) != 0)
                    && (strncmp(fid, "WAS", 3) != 0)
                    && (strncmp(fid, "WPB", 3) != 0)
                    && (strncmp(fid, "WXX", 3) != 0))
        {
            DEBUG("=====Check FIDv2.2 Error=====");
            return -1;
        }
#endif

        if (seekToRelativeOffset(fp, size - 1) != 0)
        {
            return -1;
        }

        pos = FileTell((HANDLE)(fp));
        total_size += (size + sizeof(ID3V22_FRAME));
    }

    return res;
}

_ATTR_ID3_TEXT_
static int seek_v23_frame(FILE *fp, int cnt)
{
    int     res = -1;
    int     pos = 0;
    int     size = 0;
    int     total_size = 0;
    char    fid[5];
    char    frame_id[4][5] =
    {
        "TIT2", "TPE1", "TALB", "TCON"
    };
    ID3V23_FRAME        id3v23_frame;

    pos = FileTell((HANDLE)(fp));

    //while (pos <= (Id3v2size + FileTell((HANDLE)(fp))))
    while (total_size < Id3v2size)
    {
        /* ID3v23 Frame Header Read */
        if (FileRead((uint8*) &id3v23_frame, (1 * sizeof(ID3V23_FRAME)), (HANDLE) fp) < sizeof(ID3V23_FRAME))
        {
            return -1;
        }

        strncpy(fid, id3v23_frame.Id, 4);

        if (Id3v2_header.Ver[0] == 0x03)
        {
            /* ID3v2.3 */
            size =
                  ((unsigned char)(id3v23_frame.Size[0]) << 24)
                + ((unsigned char)(id3v23_frame.Size[1]) << 16)
                + ((unsigned char)(id3v23_frame.Size[2]) << 8)
                + ((unsigned char)(id3v23_frame.Size[3]));
        }
        else {
            /* ID3v2.4 */
            size =
                  ((unsigned char)(id3v23_frame.Size[0]) << 21)
                + ((unsigned char)(id3v23_frame.Size[1]) << 14)
                + ((unsigned char)(id3v23_frame.Size[2]) << 7)
                + ((unsigned char)id3v23_frame.Size[3]);
        }

        if(size <= 0)
        {
            return -1;
        }

        if (strncmp(fid, frame_id[cnt], 4) == 0)
        {
            if ((int) FileSeek(pos, SEEK_SET, (HANDLE) fp) != 0)
            {
                return -1;
            }
            res = 0;
            break;
        }
#if 1
        else if((strncmp(fid, "AENC", 4) != 0)
                    && (strncmp(fid, "APIC", 4) != 0)
                    && (strncmp(fid, "ASPI", 4) != 0)
                    && (strncmp(fid, "COMM", 4) != 0)
                    && (strncmp(fid, "COMR", 4) != 0)
                    && (strncmp(fid, "ENCR", 4) != 0)
                    && (strncmp(fid, "EQUA", 4) != 0)
                    && (strncmp(fid, "ETCO", 4) != 0)
                    && (strncmp(fid, "GEOB", 4) != 0)
                    && (strncmp(fid, "GRID", 4) != 0)
                    && (strncmp(fid, "IPLS", 4) != 0)
                    && (strncmp(fid, "LINK", 4) != 0)
                    && (strncmp(fid, "MCDI", 4) != 0)
                    && (strncmp(fid, "MLLT", 4) != 0)
                    && (strncmp(fid, "OWNE", 4) != 0)
                    && (strncmp(fid, "PRIV", 4) != 0)
                    && (strncmp(fid, "PCNT", 4) != 0)
                    && (strncmp(fid, "POPM", 4) != 0)
                    && (strncmp(fid, "POSS", 4) != 0)
                    && (strncmp(fid, "RBUF", 4) != 0)
                    && (strncmp(fid, "RVAD", 4) != 0)
                    && (strncmp(fid, "RVRB", 4) != 0)
                    && (strncmp(fid, "SEEK", 4) != 0)
                    && (strncmp(fid, "SIGN", 4) != 0)
                    && (strncmp(fid, "SYLT", 4) != 0)
                    && (strncmp(fid, "SYTC", 4) != 0)
                    && (strncmp(fid, "TALB", 4) != 0)
                    && (strncmp(fid, "TBPM", 4) != 0)
                    && (strncmp(fid, "TCMP", 4) != 0)
                    && (strncmp(fid, "TCOM", 4) != 0)
                    && (strncmp(fid, "TCON", 4) != 0)
                    && (strncmp(fid, "TCOP", 4) != 0)
                    && (strncmp(fid, "TDAT", 4) != 0)
                    && (strncmp(fid, "TDEN", 4) != 0)
                    && (strncmp(fid, "TDLY", 4) != 0)
                    && (strncmp(fid, "TDOR", 4) != 0)
                    && (strncmp(fid, "TDRC", 4) != 0)
                    && (strncmp(fid, "TDRL", 4) != 0)
                    && (strncmp(fid, "TDTG", 4) != 0)
                    && (strncmp(fid, "TENC", 4) != 0)
                    && (strncmp(fid, "TEXT", 4) != 0)
                    && (strncmp(fid, "TFLT", 4) != 0)
                    && (strncmp(fid, "TIME", 4) != 0)
                    && (strncmp(fid, "TIPL", 4) != 0)
                    && (strncmp(fid, "TIT1", 4) != 0)
                    && (strncmp(fid, "TIT2", 4) != 0)
                    && (strncmp(fid, "TIT3", 4) != 0)
                    && (strncmp(fid, "TKEY", 4) != 0)
                    && (strncmp(fid, "TLAN", 4) != 0)
                    && (strncmp(fid, "TLEN", 4) != 0)
                    && (strncmp(fid, "TMED", 4) != 0)
                    && (strncmp(fid, "TMOO", 4) != 0)
                    && (strncmp(fid, "TOAL", 4) != 0)
                    && (strncmp(fid, "TOFN", 4) != 0)
                    && (strncmp(fid, "TOLY", 4) != 0)
                    && (strncmp(fid, "TOPE", 4) != 0)
                    && (strncmp(fid, "TORY", 4) != 0)
                    && (strncmp(fid, "TOWN", 4) != 0)
                    && (strncmp(fid, "TPE1", 4) != 0)
                    && (strncmp(fid, "TPE2", 4) != 0)
                    && (strncmp(fid, "TPE3", 4) != 0)
                    && (strncmp(fid, "TPE4", 4) != 0)
                    && (strncmp(fid, "TPOS", 4) != 0)
                    && (strncmp(fid, "TPRO", 4) != 0)
                    && (strncmp(fid, "TPUB", 4) != 0)
                    && (strncmp(fid, "TRCK", 4) != 0)
                    && (strncmp(fid, "TRDA", 4) != 0)
                    && (strncmp(fid, "TRSN", 4) != 0)
                    && (strncmp(fid, "TRSO", 4) != 0)
                    && (strncmp(fid, "TSIZ", 4) != 0)
                    && (strncmp(fid, "TSRC", 4) != 0)
                    && (strncmp(fid, "TSOA", 4) != 0)
                    && (strncmp(fid, "TSOC", 4) != 0)
                    && (strncmp(fid, "TSOT", 4) != 0)
                    && (strncmp(fid, "TSOP", 4) != 0)
                    && (strncmp(fid, "TSO2", 4) != 0)
                    && (strncmp(fid, "TSSE", 4) != 0)
                    && (strncmp(fid, "TSST", 4) != 0)
                    && (strncmp(fid, "TYER", 4) != 0)
                    && (strncmp(fid, "TXXX", 4) != 0)
                    && (strncmp(fid, "UFID", 4) != 0)
                    && (strncmp(fid, "USER", 4) != 0)
                    && (strncmp(fid, "USLT", 4) != 0)
                    && (strncmp(fid, "WCOM", 4) != 0)
                    && (strncmp(fid, "WCOP", 4) != 0)
                    && (strncmp(fid, "WOAF", 4) != 0)
                    && (strncmp(fid, "WOAR", 4) != 0)
                    && (strncmp(fid, "WOAS", 4) != 0)
                    && (strncmp(fid, "WORS", 4) != 0)
                    && (strncmp(fid, "WPAY", 4) != 0)
                    && (strncmp(fid, "WPUB", 4) != 0)
                    && (strncmp(fid, "WXXX", 4) != 0))
        {
            DEBUG("======Check FIDv2.3 Error");
            return -1;
        }
#endif

        if (seekToRelativeOffset(fp, size-1) != 0)
        {
            return -1;
        }

        pos = FileTell((HANDLE)(fp));
        total_size += (size + sizeof(ID3V23_FRAME));
    }

    return res;
}


_ATTR_ID3_TEXT_
static int parse_iTunes_frame(FILE *fp, unsigned short *txt)
{
    int                 i;
    int                 res = 0;
    int                 framesize = 0;
    unsigned int        datasize = 0;
    uint16              txt_size;
    ITUNES_FRAME        iTunes_frame;
    uint8               num;
    uint8               txt8[ID3_TEXT_WORK_SIZE];
    uint16              txt16[ID3_TEXT_WORK_SIZE];

    /* iTunes Header Read */
    if (FileRead((uint8*) &iTunes_frame, (1 * sizeof(ITUNES_FRAME)), (HANDLE) fp) < sizeof(ITUNES_FRAME))
    {
        return -1;
    }

    framesize =
            ((iTunes_frame.dataHeader.Length & 0x000000FF) << 24) +
            ((iTunes_frame.dataHeader.Length & 0x0000FF00) << 8 ) +
            ((iTunes_frame.dataHeader.Length & 0x00FF0000) >> 8 ) +
            ((iTunes_frame.dataHeader.Length & 0xFF000000) >> 24);

    if(framesize <= 0)
    {
        return -1;
    }

    /* �e�L�X�g�f�[�^�擾 */
    datasize = framesize - 0x10;

    if(datasize > ID3_TEXT_WORK_SIZE)
    {
        datasize = ID3_TEXT_WORK_SIZE;
    }


    if(iTunes_frame.atomflg[2] == 1)
    {
        FileRead((uint8 *)txt8, datasize, (HANDLE)fp);


        for(i=0; i < datasize; i++)
        {
            txt16[i] = (uint16)(txt8[i]);
        }
        txt16[i] = 0x0000;
        txt_size = TransCodeFromUTF8ToUnicode(txt16);
        if(txt_size >= 256)
        {
            txt_size = 256;
        }

        for(i = 0; i < txt_size; i++)
        {
            txt[i] = txt16[i];
        }
    }
    else
    {
        FileRead((uint8 *)txt8, datasize, (HANDLE)fp);
        //{
            //int i;
            num = txt8[1];

            if((num > 0) && (num <= ID3_GENRE_MAX_CNT))
            {
                num -= 1;
            }
            else
            {
                return 0;
            }

            for(i=0; ID3_GenreTable[num][i] != 0x00; i++)
            {
                txt[i] = (uint16)(ID3_GenreTable[num][i]);
            }
            txt[i] = 0x0000;
        //}
    }

    return res;
}
_ATTR_ID3_TEXT_
static int parse_v22_frame(FILE *fp, unsigned short *txt)
{
    int                 res = 0;
    int                 framesize = 0;
    unsigned int        datasize = 0;
    uint16              txt_size;
    TXT_INFO_FRAME_V22  txt_frame;
    int i, loop;
    uint8               txt8[ID3_TEXT_WORK_SIZE];
    uint16              txt16[ID3_TEXT_WORK_SIZE];

    /* ID3v2 Header Read */
    if (FileRead((uint8*) &txt_frame.Fheader, (1 * sizeof(ID3V22_FRAME)), (HANDLE) fp) < sizeof(ID3V22_FRAME))
    {
        return -1;
    }

    framesize =
          ((unsigned char)(txt_frame.Fheader.Size[0]) << 16)
        + ((unsigned char)(txt_frame.Fheader.Size[1]) << 8)
        + ((unsigned char)(txt_frame.Fheader.Size[2]));

    if(framesize <= 0)
    {
        return -1;
    }

    if (FileRead((uint8*) &txt_frame.Txtencode, (1 * sizeof(txt_frame.Txtencode)), (HANDLE) fp) < sizeof(txt_frame.Txtencode))
    {
        return -1;
    }
    datasize += sizeof(txt_frame.Txtencode);

    /* BOM check */
    if(txt_frame.Txtencode == 1)
    {
        FileRead((uint8*) &txt_frame.TxtBom, sizeof(txt_frame.TxtBom), (HANDLE) fp);
        datasize += sizeof(txt_frame.TxtBom);
    }
    else
    {
        txt_frame.TxtBom = 0;
    }

    datasize = framesize - datasize;

    if((txt_frame.Txtencode == 0) || (txt_frame.Txtencode == 3))
    {
        /* ISO-8859-1 / UTF-8 */
        if(datasize > ID3_TEXT_WORK_SIZE)
        {
            datasize = ID3_TEXT_WORK_SIZE;
        }
        FileRead(txt8, datasize, (HANDLE) fp);
        for(i = 0; i < datasize; i++)
        {
            txt16[i] = (uint16)(txt8[i]);
        }
        txt16[i] = 0x0000;
        txt_size = TransCodeFromUTF8ToUnicode(txt16);
        if(txt_size >= 256)
        {
            txt_size = 256;
        }
        for(i = 0; i < txt_size; i++)
        {
            txt[i] = txt16[i];
        }
        //TransCodeFromUTF8ToUnicode(txt);
    }
    else
    {
        /* UTF-16 */
        if(datasize > 512)
        {
            datasize = 512;
        }

        FileRead((uint8*) txt, datasize, (HANDLE) fp);
    }

    /* Endian Check */
    if(
        ((txt_frame.Txtencode == 1) && (txt_frame.TxtBom == 0xFFFE))
        ||
        (txt_frame.Txtencode == 2)
    ){
        /* Big -> Little */
        loop = datasize/2;
        for(i=0; i<loop; i++)
        {
            txt[i] = ((txt[i] >> 8) & 0x00FF) + ((txt[i] << 8) & 0xFF00);
        }
    }

    return res;
}

_ATTR_ID3_TEXT_
static int parse_v23_frame(FILE *fp, unsigned short *txt)
{
    int                 res = 0;
    int                 framesize = 0;
    unsigned int        datasize = 0;
    uint16              txt_size;
    TXT_INFO_FRAME_V23  txt_frame;
    int i, loop;
    uint8               txt8[ID3_TEXT_WORK_SIZE];
    uint16              txt16[ID3_TEXT_WORK_SIZE];

    /* ID3v2 Header Read */
    if (FileRead((uint8*) &txt_frame.Fheader, (1 * sizeof(ID3V23_FRAME)), (HANDLE) fp) < sizeof(ID3V23_FRAME))
    {
        return -1;
    }

    if (Id3v2_header.Ver[0] == 0x03)
    {
        /* ID3v2.3 */
        framesize =
              ((unsigned char)(txt_frame.Fheader.Size[0]) << 24)
            + ((unsigned char)(txt_frame.Fheader.Size[1]) << 16)
            + ((unsigned char)(txt_frame.Fheader.Size[2]) << 8)
            + ((unsigned char)(txt_frame.Fheader.Size[3]));
    }
    else
    {
        /* ID3v2.4 */
        framesize =
              ((unsigned char)(txt_frame.Fheader.Size[0]) << 21)
            + ((unsigned char)(txt_frame.Fheader.Size[1]) << 14)
            + ((unsigned char)(txt_frame.Fheader.Size[2]) << 7)
            + ((unsigned char)(txt_frame.Fheader.Size[3]));
    }

    if(framesize <= 0)
    {
        return -1;
    }


    if (FileRead((uint8*) &txt_frame.Txtencode, (1 * sizeof(txt_frame.Txtencode)), (HANDLE) fp) < sizeof(txt_frame.Txtencode))
    {
        return -1;
    }
    datasize += sizeof(txt_frame.Txtencode);

    /* BOM check */
    if(txt_frame.Txtencode == 1)
    {
        FileRead((uint8*) &txt_frame.TxtBom, sizeof(txt_frame.TxtBom), (HANDLE) fp);
        datasize += sizeof(txt_frame.TxtBom);
    }
    else
    {
        txt_frame.TxtBom = 0;
    }

    datasize = framesize - datasize;
    if((txt_frame.Txtencode == 0) || (txt_frame.Txtencode == 3))
    {
        /* ISO-8859-1 / UTF-8 */
        if(datasize > ID3_TEXT_WORK_SIZE)
        {
            datasize = ID3_TEXT_WORK_SIZE;
        }
        FileRead(txt8, datasize, (HANDLE) fp);
        for(i = 0; i < datasize; i++)
        {
            txt16[i] = (uint16)(txt8[i]);
        }
        txt16[i] = 0x0000;
        txt_size = TransCodeFromUTF8ToUnicode(txt16);
        if(txt_size >= 256)
        {
            txt_size = 256;
        }
        for(i = 0; i < txt_size; i++)
        {
            txt[i] = txt16[i];
        }
        //TransCodeFromUTF8ToUnicode(txt);
    }
    else
    {
        /* UTF-16 */
        if(datasize > 512)
        {
            datasize = 512;
        }
        FileRead((uint8*) txt, datasize, (HANDLE) fp);
    }

    /* Endian Check */
    if(
        ((txt_frame.Txtencode == 1) && (txt_frame.TxtBom == 0xFFFE))
        ||
        (txt_frame.Txtencode == 2)
    ){
        /* Big -> Little */
        loop = datasize/2;
        for(i=0; i<loop; i++)
        {
            txt[i] = ((txt[i] >> 8) & 0x00FF) + ((txt[i] << 8) & 0xFF00);
        }
    }

    return res;
}

_ATTR_ID3_TEXT_
static int seekToRelativeOffset(FILE *fp, unsigned int ofs)
{
    return (int) FileSeek(ofs + 1 , SEEK_CUR, (HANDLE) fp);
}

_ATTR_ID3_TEXT_
static unsigned int Lit2Big32bit(unsigned int ui)
{
    return
        ((ui << 24) & 0xFF000000) | ((ui << 8) & 0x00FF0000) |
        ((ui >> 8) & 0x0000FF00) | ((ui >> 24) & 0x000000FF);
}
_ATTR_ID3_TEXT_
static int atomitunessearch(FILE *fp)
{
    int res = 0, pos = 0;
    int         max_size = 0;
    uint8 size[4];
    ATOM_FRAME          atomframe;
    ATOM_FRAME          moovframe;
    ATOM_FRAME          udtaframe;
    ATOM_FRAME          metaframe;

    memset(&atomframe, 0, sizeof(ATOM_FRAME));
    memset(&moovframe, 0, sizeof(ATOM_FRAME));
    memset(&udtaframe, 0, sizeof(ATOM_FRAME));
    memset(&metaframe, 0, sizeof(ATOM_FRAME));

    if (FileRead((uint8*) &moovframe, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER))
    {
        return -1;
    }

    pos = FileTell((HANDLE)(fp));
    FileSeek(0 , SEEK_END, (HANDLE) fp);
    max_size = FileTell((HANDLE)(fp));
    FileSeek(pos , SEEK_SET, (HANDLE) fp);

    while
    (
        (strncmp(moovframe.Header.Atomtype,"moov", 4) != 0 ) &&
        (max_size > pos)
    )
    {
        moovframe.Header.Length = Lit2Big32bit(moovframe.Header.Length);

        //if(moovframe.Header.Length <= 0)
        if(moovframe.Header.Length < ATOMHOFS)
        {
            return -1;
        }

        seekToRelativeOffset(fp, (moovframe.Header.Length - ATOMHOFS - 1));
        FileRead((uint8*) &moovframe, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER);
        pos = FileTell((HANDLE)(fp));
    }

    res = atomheadersearch(&atomframe, "udta", moovframe.Header.Length, fp);
    if (res < 0)
    {
        /* Can't Find udta of ATOM */
        return res;
    }

    memcpy(&udtaframe.Header, &atomframe.Header, sizeof(ATOM_HEADER));

    res = atomheadersearch(&atomframe, "meta", udtaframe.Header.Length, fp);
    if (res < 0)
    {
        /* Can't Find meta of ATOM */
        return res;
    }
    (int) FileSeek(4, SEEK_CUR, (HANDLE) fp);
    memcpy(&metaframe.Header, &atomframe.Header, sizeof(ATOM_HEADER));
    res = atomheadersearch(&atomframe, "ilst", metaframe.Header.Length, fp);
    if (res < 0)
    {
        /* Can't Find ilst of ATOM */
        return res;
    }

    pos = FileTell((HANDLE)(fp));

    (int) FileSeek(pos-8, SEEK_SET, (HANDLE) fp);
    FileRead(size, 4, (HANDLE) fp);
    iTunessize =    (((UINT32)size[0]) << 24) +
                    (((UINT32)size[1]) << 16) +
                    (((UINT32)size[2]) << 8 ) +
                     ((UINT32)size[3]);

    (int) FileSeek(pos, SEEK_SET, (HANDLE) fp);
    res = 1;

    return res;
}
_ATTR_ID3_TEXT_
static int atomid32search(FILE *fp)
{
    int         res = 0;
    int         pos = 0;
    int         max_size = 0;
    ATOM_FRAME  atomframe;
    ATOM_FRAME  moovframe;
    ATOM_FRAME  udtaframe;
    ATOM_FRAME  metaframe;
    ATOM_FRAME  ilstframe;

    memset(&atomframe, 0, sizeof(ATOM_FRAME));
    memset(&moovframe, 0, sizeof(ATOM_FRAME));
    memset(&udtaframe, 0, sizeof(ATOM_FRAME));
    memset(&metaframe, 0, sizeof(ATOM_FRAME));
    memset(&ilstframe, 0, sizeof(ATOM_FRAME));

    if (FileRead((uint8*) &moovframe, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER))
    {
        return -1;
    }

    pos = FileTell((HANDLE)(fp));
    FileSeek(0 , SEEK_END, (HANDLE) fp);
    max_size = FileTell((HANDLE)(fp));
    FileSeek(pos , SEEK_SET, (HANDLE) fp);

    while
    (
        (strncmp(moovframe.Header.Atomtype,"moov", 4) != 0 ) &&
        (max_size > pos)
    )
    {
        moovframe.Header.Length = Lit2Big32bit(moovframe.Header.Length);

        //if(moovframe.Header.Length <= 0)
        if(moovframe.Header.Length < ATOMHOFS)
        {
            return -1;
        }

        seekToRelativeOffset(fp, (moovframe.Header.Length - ATOMHOFS - 1));
        FileRead((uint8*) &moovframe, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER);
        pos = FileTell((HANDLE)(fp));
    }

    res = atomheadersearch(&metaframe, "meta", moovframe.Header.Length, fp);
    if (res >= 0)
    {
        (int) FileSeek(4, SEEK_CUR, (HANDLE) fp);
        res = atomheadersearch(&atomframe, "ID32", metaframe.Header.Length, fp);
    }

    if (res < 0)
    {
        FileSeek(pos, SEEK_SET, (HANDLE) fp);

        while
        (
            (strncmp(moovframe.Header.Atomtype,"meta", 4) != 0 ) &&
            (max_size > pos)
        )
        {
            moovframe.Header.Length = Lit2Big32bit(moovframe.Header.Length);
            //if(moovframe.Header.Length <= 0)
            if(moovframe.Header.Length < ATOMHOFS)
            {
                return -1;
            }
            seekToRelativeOffset(fp, (moovframe.Header.Length - ATOMHOFS - 1));
            FileRead((uint8*) &moovframe, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER);
            pos = FileTell((HANDLE)(fp));
        }
        FileSeek(4, SEEK_CUR, (HANDLE) fp);
        res = atomheadersearch(&atomframe, "ID32", metaframe.Header.Length, fp);
        if (res < 0)
        {
            /* Can't Find ilst of ATOM */
            return res;
        }
    }
#if 0
    res = atomheadersearch(&metaframe, "meta", moovframe.Header.Length, fp);
    if (res < 0)
    {
        /* Can't Find meta of ATOM */
        return res;
    }
    (int) FileSeek(4, SEEK_CUR, (HANDLE) fp);

    res = atomheadersearch(&atomframe, "ID32", metaframe.Header.Length, fp);
    if (res < 0)
    {
        /* Can't Find ilst of ATOM */
        return res;
    }
#endif

    pos = FileTell((HANDLE)(fp)) - sizeof(ATOM_HEADER);
    (int) FileSeek(pos, SEEK_SET, (HANDLE) fp);
    res = 2;

    return res;
}

_ATTR_ID3_TEXT_
static int atomheadersearch(ATOM_FRAME *atomframe, char *atomname, unsigned int atomsize, FILE *fp)
{
    int res = -1;
    int chk;
    unsigned int totalsize;

    atomsize = Lit2Big32bit(atomsize);

    totalsize = 8;/* moov frame size */
    while (totalsize < atomsize)
    {
        /* ATOM Header �擾 */
        if (FileRead((uint8*) &atomframe->Header, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER))
        {
            /* error : NotFileRead */
            break;
        }

        //if(atomframe->Header.Length <= 0)
        //{
        //    /* error : Length */
        //    break;
        //}

        if (strncmp(atomframe->Header.Atomtype, atomname, 4) == 0)
        {
            /* OK */
            res = 0;
            break;
        }

        atomframe->Header.Length = Lit2Big32bit(atomframe->Header.Length);
        if(atomframe->Header.Length < ATOMHOFS)
        {
            /* error : Length */
            break;
        }
        chk = seekToRelativeOffset(fp, atomframe->Header.Length - ATOMHOFS - 1);
        if(chk != 0)
        {
            /* FileSeekError */
            break;
        }
        totalsize += atomframe->Header.Length;
    }
#if 0
    int res = -1;
    unsigned int    size = 0;
    atomsize = Lit2Big32bit(atomsize);

    size = FileTell((HANDLE)(fp));
    while (size <= (atomsize + FileTell((HANDLE)(fp))))
    {
        if (FileRead((uint8*) &atomframe->Header, (1 * sizeof(ATOM_HEADER)), (HANDLE) fp) < sizeof(ATOM_HEADER))
        {
            return -1;
        }

        if (strncmp(atomframe->Header.Atomtype, atomname, 4) == 0)
        {
            res = 0;
            break;
        }
        atomframe->Header.Length = Lit2Big32bit(atomframe->Header.Length);
        seekToRelativeOffset(fp, atomframe->Header.Length - ATOMHOFS - 1);
        size = FileTell((HANDLE)(fp));
    }
#endif
    return res;
}

_ATTR_ID3_TEXT_
static int getheader(FILE *fp)
{
    int     res = 0;
    int     tmp = 0;

    initid3v2header();

    /* ID3v2 ID Read */
    if (FileRead((uint8*)&Id3v2_header.Id, (1 * sizeof(Id3v2_header.Id)), (HANDLE)fp) < sizeof(Id3v2_header.Id))
    {
        return -1;
    }

    /* ID Check */
    if (strncmp(Id3v2_header.Id,"ID3", 3) != 0)
    {
        return -1;
    }

    /* ID3v2 Version Read */
    if (FileRead((uint8*) &Id3v2_header.Ver, (1 * sizeof(Id3v2_header.Ver)), (HANDLE) fp) < sizeof(Id3v2_header.Ver))
    {
        return -1;
    }

    /* ID3v2 Version Check */
    if ((Id3v2_header.Ver[0] != 0x02) && (Id3v2_header.Ver[0] != 0x03) && (Id3v2_header.Ver[0] != 0x04))
    {
        return -1;
    }

    /* ID3v2 Flag Read */
    if (FileRead((uint8*) &Id3v2_header.Flg, (1 * sizeof(Id3v2_header.Flg)), (HANDLE) fp) < sizeof(Id3v2_header.Flg))
    {
        return -1;
    }

    /* ID3v2 Size Read */
    if (FileRead((uint8*) &Id3v2_header.Size, (1 * sizeof(Id3v2_header.Size)), (HANDLE) fp) < sizeof(Id3v2_header.Size))
    {
        return -1;
    }

    /* Size Calc */
    Id3v2size =
          ((Id3v2_header.Size[0]) << 21)
        + ((Id3v2_header.Size[1]) << 14)
        + ((Id3v2_header.Size[2]) << 7)
        + Id3v2_header.Size[3];

    return res;
}
_ATTR_ID3_TEXT_
static void initid3v2header(void)
{
    memset(&Id3v2_header, 0, sizeof(Id3v2_header));
    strncpy(Id3v2_header.Id, "ID3", 3);
    Id3v2_header.Ver[0] = 0x03;
}
_ATTR_ID3_TEXT_
static int16 TransCodeFromUTF8ToUnicode(UINT16 *longName)
{
    int i = 0;
    int size ;
    int unicodeNameIndex = 0;   //use the same array to store the result ,no need to use buf.
    int err = 0;
    int16 ret;

    //while(longName[i] != 0)
    while((longName[i] != 0) && (err == 0))
    {
        size = GetUTF8Size(longName[i]);
        switch(size){

        case 1:
            //no need to transform
            longName[unicodeNameIndex] = longName[i];
            i += 1;
            unicodeNameIndex += 1;
            break;

        case 2:
            longName[unicodeNameIndex]  = 0x07FF & (longName[i]<<6 | 0x003F & longName[i+1]);  // 110xxxxx 10xxxxxx ; mask:is 0x003f for low 6bit, after combine, the unit16 mask is 0x07FF for low 11bits
            i += 2;
            unicodeNameIndex +=1;
            break;

        case 3:
            longName[unicodeNameIndex]  =  longName[i]<<12 | (0x003F & longName[i+1])<<6 |0x003F &  longName[i+2] ;  //     1110xxxx 10xxxxxx 10xxxxxx => mask: 0x000F 0x003f 0x003f,but the highest byte just need to shift 12bit, the highest 4bit will be clear.
            i += 3;
            unicodeNameIndex +=1;
            break;

        case 4:
            DEBUG("Error Four byte UTF8, need to add case in TransCodeFromUTF8ToUnicode()");
            err = 1;
            break;
        case 5:
            DEBUG("Error Four byte UTF8, need to add case in TransCodeFromUTF8ToUnicode()");
            err = 1;
            break;
        case 6:
            DEBUG("Error Four byte UTF8, need to add case in TransCodeFromUTF8ToUnicode()");
            err = 1;
            break;
        default:
            err = 1;
            break;
        }
    }
    longName[unicodeNameIndex] = 0;

    ret = unicodeNameIndex;
    if(err == 0)
    {
        //clear the unwanted words to 0
        while(longName[++unicodeNameIndex] != 0)
        {
            longName[unicodeNameIndex] = 0;
        }
    }
    return ret;
}


/* **************************************** */
/*  Get UTF8 Byte Size                      */
/* **************************************** */
_ATTR_ID3_TEXT_
static int GetUTF8Size(uint16 pInput)
{
    char c = (uint8)pInput;
    // 0xxxxxxx ����1, 1byte
    // 10xxxxxx ������
    // 110xxxxx ����2, 2byte
    // 1110xxxx ����3, 3byte
    // 11110xxx ����4, 4byte
    // 111110xx ����5, 5byte
    // 1111110x ����6, 6byte
    if(c< 0x80) return 1;
    if(c>=0x80 && c<0xC0) return -1;
    if(c>=0xC0 && c<0xE0) return 2;
    if(c>=0xE0 && c<0xF0) return 3;
    if(c>=0xF0 && c<0xF8) return 4;
    if(c>=0xF8 && c<0xFC) return 5;
    if(c>=0xFC) return 6;
}
#endif

