//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*
 * Windows Media Audio (WMA) Decoder API (implementation)
 *
 * Copyright (c) Microsoft Corporation 1999.  All Rights Reserved.
 */
 #include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#include "..\wmaInclude\wmaudio_inc.h"
#include "..\wmaInclude\WMAGLOBALVARDeclare.h"
#include "..\wmaInclude\predefine.h"
#ifdef WMDRM_NETWORK
#include "DrmAesEx.h"
#endif

//#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData",rodate = "WmaCommonCode"

#ifndef WMAAPI_NO_DRM
#include "..\wmaInclude\drmpd.h"
const BYTE APPSEC_1100[APPSEC_LEN] = {0x0, 0x0, 0x4, 0x4C};
const BYTE APPSEC_1000[APPSEC_LEN] = {0x0, 0x0, 0x3, 0xE8};
#endif



        static  char wmaversion[] = "Version:0.0.1 \nDate:2012.3.26 \nLib:wma_dec_lib" ;

         char * WmaDecVersion()
        {

                return wmaversion;
        }

//extern int iSubFrm;

//#ifdef WMDRM_PORTABLE
//static const DRM_WCHAR s_szLicense[] = L"c:\\wmdrmpd\\sample.hds";
//static const DRM_CONST_STRING s_strLicense = CREATE_DRM_STRING( s_szLicense );
//#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
WMAPlayerInfo gPlayInfo;//20080724

#ifdef WMA_TABLE_ROOM_GENERATE

extern const SinCosTable g_sct64;
extern const SinCosTable g_sct128;
extern const SinCosTable g_sct256;
extern const SinCosTable g_sct512;
extern const SinCosTable g_sct1024;
extern const SinCosTable g_sct2048;
//extern const BP1Type icosPIbynp[16];
//extern const BP1Type isinPIbynp[16];
//extern const LpType g_rgiLsfReconLevel [LPCORDER] [16];
extern U8 gLZLTable[128];
extern const unsigned long _DRM_Spbox[8][64];
extern const unsigned long _DRM_Sel[8][64];

static const unsigned char byte_to_unary_table[] =
{
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* CRC-8, poly = x^8 + x^2 + x^1 + x^0, init = 0 */

unsigned char FLAC__crc8_table[256] =
{
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

/* CRC-16, poly = x^16 + x^15 + x^2 + x^0, init = 0 */

unsigned short FLAC__crc16_table[256] =
{
    0x0000,  0x8005,  0x800f,  0x000a,  0x801b,  0x001e,  0x0014,  0x8011,
    0x8033,  0x0036,  0x003c,  0x8039,  0x0028,  0x802d,  0x8027,  0x0022,
    0x8063,  0x0066,  0x006c,  0x8069,  0x0078,  0x807d,  0x8077,  0x0072,
    0x0050,  0x8055,  0x805f,  0x005a,  0x804b,  0x004e,  0x0044,  0x8041,
    0x80c3,  0x00c6,  0x00cc,  0x80c9,  0x00d8,  0x80dd,  0x80d7,  0x00d2,
    0x00f0,  0x80f5,  0x80ff,  0x00fa,  0x80eb,  0x00ee,  0x00e4,  0x80e1,
    0x00a0,  0x80a5,  0x80af,  0x00aa,  0x80bb,  0x00be,  0x00b4,  0x80b1,
    0x8093,  0x0096,  0x009c,  0x8099,  0x0088,  0x808d,  0x8087,  0x0082,
    0x8183,  0x0186,  0x018c,  0x8189,  0x0198,  0x819d,  0x8197,  0x0192,
    0x01b0,  0x81b5,  0x81bf,  0x01ba,  0x81ab,  0x01ae,  0x01a4,  0x81a1,
    0x01e0,  0x81e5,  0x81ef,  0x01ea,  0x81fb,  0x01fe,  0x01f4,  0x81f1,
    0x81d3,  0x01d6,  0x01dc,  0x81d9,  0x01c8,  0x81cd,  0x81c7,  0x01c2,
    0x0140,  0x8145,  0x814f,  0x014a,  0x815b,  0x015e,  0x0154,  0x8151,
    0x8173,  0x0176,  0x017c,  0x8179,  0x0168,  0x816d,  0x8167,  0x0162,
    0x8123,  0x0126,  0x012c,  0x8129,  0x0138,  0x813d,  0x8137,  0x0132,
    0x0110,  0x8115,  0x811f,  0x011a,  0x810b,  0x010e,  0x0104,  0x8101,
    0x8303,  0x0306,  0x030c,  0x8309,  0x0318,  0x831d,  0x8317,  0x0312,
    0x0330,  0x8335,  0x833f,  0x033a,  0x832b,  0x032e,  0x0324,  0x8321,
    0x0360,  0x8365,  0x836f,  0x036a,  0x837b,  0x037e,  0x0374,  0x8371,
    0x8353,  0x0356,  0x035c,  0x8359,  0x0348,  0x834d,  0x8347,  0x0342,
    0x03c0,  0x83c5,  0x83cf,  0x03ca,  0x83db,  0x03de,  0x03d4,  0x83d1,
    0x83f3,  0x03f6,  0x03fc,  0x83f9,  0x03e8,  0x83ed,  0x83e7,  0x03e2,
    0x83a3,  0x03a6,  0x03ac,  0x83a9,  0x03b8,  0x83bd,  0x83b7,  0x03b2,
    0x0390,  0x8395,  0x839f,  0x039a,  0x838b,  0x038e,  0x0384,  0x8381,
    0x0280,  0x8285,  0x828f,  0x028a,  0x829b,  0x029e,  0x0294,  0x8291,
    0x82b3,  0x02b6,  0x02bc,  0x82b9,  0x02a8,  0x82ad,  0x82a7,  0x02a2,
    0x82e3,  0x02e6,  0x02ec,  0x82e9,  0x02f8,  0x82fd,  0x82f7,  0x02f2,
    0x02d0,  0x82d5,  0x82df,  0x02da,  0x82cb,  0x02ce,  0x02c4,  0x82c1,
    0x8243,  0x0246,  0x024c,  0x8249,  0x0258,  0x825d,  0x8257,  0x0252,
    0x0270,  0x8275,  0x827f,  0x027a,  0x826b,  0x026e,  0x0264,  0x8261,
    0x0220,  0x8225,  0x822f,  0x022a,  0x823b,  0x023e,  0x0234,  0x8231,
    0x8213,  0x0216,  0x021c,  0x8219,  0x0208,  0x820d,  0x8207,  0x0202
};

unsigned long Ape_gtPowersOfTwoMinusOne[33] =
{
    0ul, 1ul, 3ul, 7ul,
    15ul, 31ul, 63ul, 127ul,
    255ul, 511ul, 1023ul, 2047ul,
    4095ul, 8191ul, 16383ul, 32767ul,
    65535ul, 131071ul, 262143ul, 524287ul,
    1048575ul, 2097151ul, 4194303ul, 8388607ul,
    16777215ul, 33554431ul, 67108863ul, 134217727ul,
    268435455ul, 536870911ul, 1073741823ul, 2147483647ul,
    4294967295ul
};

unsigned long Ape_gtKSumMinBoundary[32] =
{
    0ul, 32ul, 64ul, 128ul,
    256ul, 512ul, 1024ul, 2048ul,
    4096ul, 8192ul, 16384ul, 32768ul,
    65536ul, 131072ul, 262144ul, 524288ul,
    1048576ul, 2097152ul, 4194304ul, 8388608ul,
    16777216ul, 33554432ul, 67108864ul, 134217728ul,
    268435456ul, 536870912ul, 1073741824ul, 2147483648ul,
    0ul, 0ul, 0ul, 0ul
};

unsigned long Ape_gtRangeTotalOne[65] =
{
    0ul, 14824ul, 28224ul, 39348ul,
    47855ul, 53994ul, 58171ul, 60926ul,
    62682ul, 63786ul, 64463ul, 64878ul,
    65126ul, 65276ul, 65365ul, 65419ul,
    65450ul, 65469ul, 65480ul, 65487ul,
    65491ul, 65493ul, 65494ul, 65495ul,
    65496ul, 65497ul, 65498ul, 65499ul,
    65500ul, 65501ul, 65502ul, 65503ul,
    65504ul, 65505ul, 65506ul, 65507ul,
    65508ul, 65509ul, 65510ul, 65511ul,
    65512ul, 65513ul, 65514ul, 65515ul,
    65516ul, 65517ul, 65518ul, 65519ul,
    65520ul, 65521ul, 65522ul, 65523ul,
    65524ul, 65525ul, 65526ul, 65527ul,
    65528ul, 65529ul, 65530ul, 65531ul,
    65532ul, 65533ul, 65534ul, 65535ul,
    65536ul
};

unsigned long Ape_gtRangeWidthOne[64] =
{
    14824ul, 13400ul, 11124ul, 8507ul,
    6139ul, 4177ul, 2755ul, 1756ul,
    1104ul, 677ul, 415ul, 248ul,
    150ul, 89ul, 54ul, 31ul,
    19ul, 11ul, 7ul, 4ul,
    2ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul
};

unsigned long Ape_gtRangeTotalTwo[65] =
{
    0ul, 19578ul, 36160ul, 48417ul,
    56323ul, 60899ul, 63265ul, 64435ul,
    64971ul, 65232ul, 65351ul, 65416ul,
    65447ul, 65466ul, 65476ul, 65482ul,
    65485ul, 65488ul, 65490ul, 65491ul,
    65492ul, 65493ul, 65494ul, 65495ul,
    65496ul, 65497ul, 65498ul, 65499ul,
    65500ul, 65501ul, 65502ul, 65503ul,
    65504ul, 65505ul, 65506ul, 65507ul,
    65508ul, 65509ul, 65510ul, 65511ul,
    65512ul, 65513ul, 65514ul, 65515ul,
    65516ul, 65517ul, 65518ul, 65519ul,
    65520ul, 65521ul, 65522ul, 65523ul,
    65524ul, 65525ul, 65526ul, 65527ul,
    65528ul, 65529ul, 65530ul, 65531ul,
    65532ul, 65533ul, 65534ul, 65535ul,
    65536ul
};

unsigned long Ape_gtRangeWidthTwo[64] =
{
    19578ul, 16582ul, 12257ul, 7906ul,
    4576ul, 2366ul, 1170ul, 536ul,
    261ul, 119ul, 65ul, 31ul,
    19ul, 10ul, 6ul, 3ul,
    3ul, 2ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul,
    1ul, 1ul, 1ul, 1ul
};

unsigned long Ape_gtCRC32[256] =
    {0ul,        1996959894ul, 3993919788ul, 2567524794ul,
     124634137ul, 1886057615ul, 3915621685ul, 2657392035ul,
     249268274ul, 2044508324ul, 3772115230ul, 2547177864ul,
     162941995ul, 2125561021ul, 3887607047ul, 2428444049ul,
     498536548ul, 1789927666ul, 4089016648ul, 2227061214ul,
     450548861ul, 1843258603ul, 4107580753ul, 2211677639ul,
     325883990ul, 1684777152ul, 4251122042ul, 2321926636ul,
     335633487ul, 1661365465ul, 4195302755ul, 2366115317ul,
     997073096ul, 1281953886ul, 3579855332ul, 2724688242ul,
     1006888145ul, 1258607687ul, 3524101629ul, 2768942443ul,
     901097722ul, 1119000684ul, 3686517206ul, 2898065728ul,
     853044451ul, 1172266101ul, 3705015759ul, 2882616665ul,
     651767980ul, 1373503546ul, 3369554304ul, 3218104598ul,
     565507253ul, 1454621731ul, 3485111705ul, 3099436303ul,
     671266974ul, 1594198024ul, 3322730930ul, 2970347812ul,
     795835527ul, 1483230225ul, 3244367275ul, 3060149565ul,
     1994146192ul, 31158534ul, 2563907772ul, 4023717930ul,
     1907459465ul, 112637215ul, 2680153253ul, 3904427059ul,
     2013776290ul, 251722036ul, 2517215374ul, 3775830040ul,
     2137656763ul, 141376813ul, 2439277719ul, 3865271297ul,
     1802195444ul, 476864866ul, 2238001368ul, 4066508878ul,
     1812370925ul, 453092731ul, 2181625025ul, 4111451223ul,
     1706088902ul, 314042704ul, 2344532202ul, 4240017532ul,
     1658658271ul, 366619977ul, 2362670323ul, 4224994405ul,
     1303535960ul, 984961486ul, 2747007092ul, 3569037538ul,
     1256170817ul, 1037604311ul, 2765210733ul, 3554079995ul,
     1131014506ul, 879679996ul, 2909243462ul, 3663771856ul,
     1141124467ul, 855842277ul, 2852801631ul, 3708648649ul,
     1342533948ul, 654459306ul, 3188396048ul, 3373015174ul,
     1466479909ul, 544179635ul, 3110523913ul, 3462522015ul,
     1591671054ul, 702138776ul, 2966460450ul, 3352799412ul,
     1504918807ul, 783551873ul, 3082640443ul, 3233442989ul,
     3988292384ul, 2596254646ul, 62317068ul, 1957810842ul,
     3939845945ul, 2647816111ul, 81470997ul, 1943803523ul,
     3814918930ul, 2489596804ul, 225274430ul, 2053790376ul,
     3826175755ul, 2466906013ul, 167816743ul, 2097651377ul,
     4027552580ul, 2265490386ul, 503444072ul, 1762050814ul,
     4150417245ul, 2154129355ul, 426522225ul, 1852507879ul,
     4275313526ul, 2312317920ul, 282753626ul, 1742555852ul,
     4189708143ul, 2394877945ul, 397917763ul, 1622183637ul,
     3604390888ul, 2714866558ul, 953729732ul, 1340076626ul,
     3518719985ul, 2797360999ul, 1068828381ul, 1219638859ul,
     3624741850ul, 2936675148ul, 906185462ul, 1090812512ul,
     3747672003ul, 2825379669ul, 829329135ul, 1181335161ul,
     3412177804ul, 3160834842ul, 628085408ul, 1382605366ul,
     3423369109ul, 3138078467ul, 570562233ul, 1426400815ul,
     3317316542ul, 2998733608ul, 733239954ul, 1555261956ul,
     3268935591ul, 3050360625ul, 752459403ul, 1541320221ul,
     2607071920ul, 3965973030ul, 1969922972ul, 40735498ul,
     2617837225ul, 3943577151ul, 1913087877ul, 83908371ul,
     2512341634ul, 3803740692ul, 2075208622ul, 213261112ul,
     2463272603ul, 3855990285ul, 2094854071ul, 198958881ul,
     2262029012ul, 4057260610ul, 1759359992ul, 534414190ul,
     2176718541ul, 4139329115ul, 1873836001ul, 414664567ul,
     2282248934ul, 4279200368ul, 1711684554ul, 285281116ul,
     2405801727ul, 4167216745ul, 1634467795ul, 376229701ul,
     2685067896ul, 3608007406ul, 1308918612ul, 956543938ul,
     2808555105ul, 3495958263ul, 1231636301ul, 1047427035ul,
     2932959818ul, 3654703836ul, 1088359270ul, 936918000ul,
     2847714899ul, 3736837829ul, 1202900863ul, 817233897ul,
     3183342108ul, 3401237130ul, 1404277552ul, 615818150ul,
     3134207493ul, 3453421203ul, 1423857449ul, 601450431ul,
     3009837614ul, 3294710456ul, 1567103746ul, 711928724ul,
     3020668471ul, 3272380065ul, 1510334235ul, 755167117ul
    };

#define NANO_C
#ifdef NANO_C
unsigned long table_size[] =
{
             0,
/*01*/     132,    //getMask
/*02*/      16,    //CLSID_CAsfHeaderObjectV0
/*03*/      16,    //CLSID_CAsfPropertiesObjectV2
/*04*/      16,    //CLSID_CAsfStreamPropertiesObjectV1
/*05*/      16,    //CLSID_CAsfContentDescriptionObjectV0
/*06*/      16,    //CLSID_AsfXAcmAudioErrorMaskingStrategy
/*07*/      16,    //CLSID_AsfXSignatureAudioErrorMaskingStrategy
/*08*/      16,    //CLSID_AsfXStreamTypeAcmAudio
/*09*/      16,    //CLSID_CAsfContentEncryptionObject
/*10*/      16,    //CLSID_CAsfExtendedContentDescObject
/*11*/      16,    //CLSID_CAsfMarkerObjectV0
/*12*/      16,    //CLSID_CAsfLicenseStoreObject
/*13*/      16,    //CLSID_CAsfStreamPropertiesObjectV2
/*14*/      16,    //CLSID_CAsfExtendedStreamPropertiesObject
/*15*/      16,    //CLSID_CAsfClockObjectV0
/*16*/      16,    //CLSID_AsfXMetaDataObject
/*17*/      16,    //CLSID_CAsfPacketClock1
/*18*/     452,    //g_rgiHuffDecTblMsk
/*19*/     152,    //g_rgiHuffDecTblNoisePower
/*20*/    5694,    //g_rgiHuffDecTbl44smOb
/*21*/     288,    //rgiMaskMinusPower10
/*22*/     248,    //rgiMaskPlusPower10
/*23*/     100,    //g_rgiBarkFreqV2
/*24*/     433,    //gRun16ssOb
/*25*/     433,    //gLevel16ssOb
/*26*/    1334,    //gRun44smOb
/*27*/    1334,    //gLevel44smOb
/*28*/    1070,    //gRun44ssOb
/*29*/    1070,    //gLevel44ssOb
/*30*/    664,    //gRun44smQb
/*31*/    664,    //gLevel44smQb
/*32*/    553,    //gRun44ssQb
/*33*/    553,    //gLevel44ssQb
/*34*/    1028,    //g_InvQuadRootFraction
/*35*/     260,    //g_InvQuadRootExponent
/*36*/    1028,    //g_InverseFraction
/*37*/    1028,    //g_SqrtFraction
/*38*/      52,    //g_sct64
/*39*/      52,    //g_sct128
/*40*/      52,    //g_sct256
/*41*/      52,    //g_sct512
/*42*/      52,    //g_sct1024
/*43*/      52,    //g_sct2048
/*44*/     512,    //rgDBPower10
/*45*/      64,    //icosPIbynp
/*46*/      64,    //isinPIbynp
/*47*/     640,    //g_rgiLsfReconLevel
/*48*/     128,    //gLZLTable
};

// éú3é wma table room, ????±í????ò?4byte????
// 0:3é1|￡?other:ê§°ü
int wma_table_room_generate(void)
{
    FILE *fpTable;
    int ret = 0;
    unsigned long size; // 4byte????oóμ?3¤?è
    unsigned long offset = 0;

    // ′′?¨???t
    fpTable = fopen("wma_table_room.bin", "wb");
    if(fpTable == NULL)
    {
        DEBUG("Can't create wma_table_room.bin!\n");
        ret = 1;
        goto Error;
    }

    //////////////////////////////////////////////// WMA TABLE /////////////////////////////////////////////////////////////

    // (1) getMask
    size = (table_size[1]+3)&(~3); // 4byte????
    if(fwrite(getMask, 1, size, fpTable) != size)
    {
        DEBUG("write getMask error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_getMask                                              = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (2) CLSID_CAsfHeaderObjectV0
    size = (table_size[2]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfHeaderObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfHeaderObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfHeaderObjectV0                              = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (3) CLSID_CAsfPropertiesObjectV2
    size = (table_size[3]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfPropertiesObjectV2, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfPropertiesObjectV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfPropertiesObjectV2                          = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (4) CLSID_CAsfStreamPropertiesObjectV1
    size = (table_size[4]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfStreamPropertiesObjectV1, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfStreamPropertiesObjectV1 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV1                    = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (5) CLSID_CAsfContentDescriptionObjectV0
    size = (table_size[5]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfContentDescriptionObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentDescriptionObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentDescriptionObjectV0                  = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (6) CLSID_AsfXAcmAudioErrorMaskingStrategy
    size = (table_size[6]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXAcmAudioErrorMaskingStrategy, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXAcmAudioErrorMaskingStrategy error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXAcmAudioErrorMaskingStrategy                = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (7) CLSID_AsfXSignatureAudioErrorMaskingStrategy
    size = (table_size[7]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXSignatureAudioErrorMaskingStrategy, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXSignatureAudioErrorMaskingStrategy error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXSignatureAudioErrorMaskingStrategy           = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (8) CLSID_AsfXStreamTypeAcmAudio
    size = (table_size[8]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXStreamTypeAcmAudio, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXStreamTypeAcmAudio error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXStreamTypeAcmAudio                           = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (9) CLSID_CAsfContentEncryptionObject
    size = (table_size[9]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfContentEncryptionObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentEncryptionObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentEncryptionObject                      = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (10) CLSID_CAsfExtendedContentDescObject
    size = (table_size[10]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfExtendedContentDescObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfExtendedContentDescObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfExtendedContentDescObject                    = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (11) CLSID_CAsfMarkerObjectV0
    size = (table_size[11]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfMarkerObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfMarkerObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfMarkerObjectV0                               = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (12) CLSID_CAsfLicenseStoreObject
    size = (table_size[12]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfLicenseStoreObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfLicenseStoreObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfLicenseStoreObject                           = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (13) CLSID_CAsfStreamPropertiesObjectV2
    size = (table_size[13]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfStreamPropertiesObjectV2, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfStreamPropertiesObjectV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV2                     = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (14) CLSID_CAsfExtendedStreamPropertiesObject
    size = (table_size[14]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfExtendedStreamPropertiesObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfExtendedStreamPropertiesObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfExtendedStreamPropertiesObject               = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (15) CLSID_CAsfClockObjectV0
    size = (table_size[15]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfClockObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfClockObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfClockObjectV0                                = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (16) CLSID_AsfXMetaDataObject
    size = (table_size[16]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXMetaDataObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXMetaDataObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXMetaDataObject                               = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    // (17) CLSID_CAsfPacketClock1
    size = (table_size[17]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfPacketClock1, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfPacketClock1 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfPacketClock1                                = 0x%04x,        size_aligned = %d\n", offset, size);
    offset += size;

    /*
    // (18) CLSID_CAsfContentEncryptionObjectEx
    size = 16; // 4byte????
    if(fwrite(&CLSID_CAsfContentEncryptionObjectEx, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentEncryptionObjectEx error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentEncryptionObjectEx                   = %x,        size_aligned = %x\n", offset, size);
    offset += size;
    */

    // (18) g_rgiHuffDecTblMsk
    size = (table_size[18]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTblMsk, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTblMsk error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTblMsk                                    = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (19) g_rgiHuffDecTblNoisePower
    size = (table_size[19]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTblNoisePower, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTblNoisePower error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTblNoisePower                              = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (20) g_rgiHuffDecTbl44smOb
    size = (table_size[20]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTbl44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44smOb                                  = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (21) rgiMaskMinusPower10
    size = (table_size[21]+3)&(~3); // 4byte????
    if(fwrite(rgiMaskMinusPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgiMaskMinusPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgiMaskMinusPower10                                    = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (22) rgiMaskPlusPower10
    size = (table_size[22]+3)&(~3); // 4byte????
    if(fwrite(rgiMaskPlusPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgiMaskPlusPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgiMaskPlusPower10                                     = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (23) g_rgiBarkFreqV2
    size = (table_size[23]+3)&(~3); // 4byte????
    if(fwrite(g_rgiBarkFreqV2, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiBarkFreqV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiBarkFreqV2                                        = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (24) gRun16ssOb
    size = (table_size[24]+3)&(~3); // 4byte????
    if(fwrite(gRun16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun16ssOb                                             = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (25) gLevel16ssOb
    size = (table_size[25]+3)&(~3); // 4byte????
    if(fwrite(gLevel16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel16ssOb                                          = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (26) gRun44smOb
    size = (table_size[26]+3)&(~3); // 4byte????
    if(fwrite(gRun44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44smOb                                            = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (27) gLevel44smOb
    size = (table_size[27]+3)&(~3); // 4byte????
    if(fwrite(gLevel44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44smOb                                          = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (28) gRun44ssOb
    size = (table_size[28]+3)&(~3); // 4byte????
    if(fwrite(gRun44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44ssOb                                            = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (29) gLevel44ssOb
    size = (table_size[29]+3)&(~3); // 4byte????
    if(fwrite(gLevel44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44ssOb                                          = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (30) gRun44smQb
    size = (table_size[30]+3)&(~3); // 4byte????
    if(fwrite(gRun44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44smQb                                            = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (31) gLevel44smQb
    size = (table_size[31]+3)&(~3); // 4byte????
    if(fwrite(gLevel44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44smQb                                          = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (32) gRun44ssQb
    size = (table_size[32]+3)&(~3); // 4byte????
    if(fwrite(gRun44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44ssQb                                            = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (33) gLevel44ssQb
    size = (table_size[33]+3)&(~3); // 4byte????
    if(fwrite(gLevel44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44ssQb                                          = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (34) g_InvQuadRootFraction
    size = (table_size[34]+3)&(~3); // 4byte????
    if(fwrite(g_InvQuadRootFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_InvQuadRootFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InvQuadRootFraction                                 = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (35) g_InvQuadRootExponent
    size = (table_size[35]+3)&(~3); // 4byte????
    if(fwrite(g_InvQuadRootExponent, 1, size, fpTable) != size)
    {
        DEBUG("write g_InvQuadRootExponent error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InvQuadRootExponent                                 = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (36) g_InverseFraction
    size = (table_size[36]+3)&(~3); // 4byte????
    if(fwrite(g_InverseFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_InverseFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InverseFraction                                     = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (37) g_SqrtFraction
    size = (table_size[37]+3)&(~3); // 4byte????
    if(fwrite(g_SqrtFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_SqrtFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_SqrtFraction                                        = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (38) g_sct64
    size = (table_size[38]+3)&(~3); // 4byte????
    if(fwrite(&g_sct64, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct64 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct64                                               = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (39) g_sct128
    size = (table_size[39]+3)&(~3); // 4byte????
    if(fwrite(&g_sct128, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct128 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct128                                              = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (40) g_sct256
    size = (table_size[40]+3)&(~3); // 4byte????
    if(fwrite(&g_sct256, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct256 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct256                                              = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (41) g_sct512
    size = (table_size[41]+3)&(~3); // 4byte????
    if(fwrite(&g_sct512, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct512 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct512                                              = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (42) g_sct1024
    size = (table_size[42]+3)&(~3); // 4byte????
    if(fwrite(&g_sct1024, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct1024 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct1024                                             = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (43) g_sct2048
    size = (table_size[43]+3)&(~3); // 4byte????
    if(fwrite(&g_sct2048, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct2048 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct2048                                             = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (44) rgDBPower10
    size = (table_size[44]+3)&(~3); // 4byte????
    if(fwrite(rgDBPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgDBPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgDBPower10                                           = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (45) icosPIbynp
    size = (table_size[45]+3)&(~3); // 4byte????
    if(fwrite(icosPIbynp, 1, size, fpTable) != size)
    {
        DEBUG("write icosPIbynp error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_icosPIbynp                                            = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (46) isinPIbynp
    size = (table_size[46]+3)&(~3); // 4byte????
    if(fwrite(isinPIbynp, 1, size, fpTable) != size)
    {
        DEBUG("write isinPIbynp error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_isinPIbynp                                            = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (47) g_rgiLsfReconLevel
    size = (table_size[47]+3)&(~3); // 4byte????
    if(fwrite(g_rgiLsfReconLevel[0], 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiLsfReconLevel error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiLsfReconLevel                                    = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

    // (48) gLZLTable
    size = (table_size[48]+3)&(~3); // 4byte????
    if(fwrite(gLZLTable, 1, size, fpTable) != size)
    {
        DEBUG("write gLZLTable error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLZLTable                                             = 0x%04x,         size_aligned = %d\n", offset, size);
    offset += size;

Error:

    // 1?±????t
    if(fpTable)
        fclose(fpTable);

    return ret;
}
#else
unsigned long table_size[] =
{
             0,
/*01*/     132,    //getMask
/*02*/      16,    //CLSID_CAsfHeaderObjectV0
/*03*/      16,    //CLSID_CAsfPropertiesObjectV2
/*04*/      16,    //CLSID_CAsfStreamPropertiesObjectV1
/*05*/      16,    //CLSID_CAsfContentDescriptionObjectV0
/*06*/      16,    //CLSID_AsfXAcmAudioErrorMaskingStrategy
/*07*/      16,    //CLSID_AsfXSignatureAudioErrorMaskingStrategy
/*08*/      16,    //CLSID_AsfXStreamTypeAcmAudio
/*09*/      16,    //CLSID_CAsfContentEncryptionObject
/*10*/      16,    //CLSID_CAsfExtendedContentDescObject
/*11*/      16,    //CLSID_CAsfMarkerObjectV0
/*12*/      16,    //CLSID_CAsfLicenseStoreObject
/*13*/      16,    //CLSID_CAsfStreamPropertiesObjectV2
/*14*/      16,    //CLSID_CAsfExtendedStreamPropertiesObject
/*15*/      16,    //CLSID_CAsfClockObjectV0
/*16*/      16,    //CLSID_AsfXMetaDataObject
/*17*/      16,    //CLSID_CAsfPacketClock1
/*18*/     452,    //g_rgiHuffDecTblMsk
/*19*/     152,    //g_rgiHuffDecTblNoisePower
/*20*/    1744,    //g_rgiHuffDecTbl16smOb
/*21*/    5694,    //g_rgiHuffDecTbl44smOb
/*22*/    1696,    //g_rgiHuffDecTbl16ssOb
/*23*/    4004,    //g_rgiHuffDecTbl44ssOb
/*24*/    2488,    //g_rgiHuffDecTbl44smQb
/*25*/    2208,    //g_rgiHuffDecTbl44ssQb
/*26*/     288,    //rgiMaskMinusPower10
/*27*/     248,    //rgiMaskPlusPower10
/*28*/     100,    //g_rgiBarkFreqV2
/*29*/     948,    //gRun16smOb
/*30*/     948,    //gLevel16smOb
/*31*/     866,    //gRun16ssOb
/*32*/     866,    //gLevel16ssOb
/*33*/    2668,    //gRun44smOb
/*34*/    2668,    //gLevel44smOb
/*35*/    2140,    //gRun44ssOb
/*36*/    2140,    //gLevel44ssOb
/*37*/    1328,    //gRun44smQb
/*38*/    1328,    //gLevel44smQb
/*39*/    1106,    //gRun44ssQb
/*40*/    1106,    //gLevel44ssQb
/*41*/    1028,    //g_InvQuadRootFraction
/*42*/     260,    //g_InvQuadRootExponent
/*43*/    1028,    //g_InverseFraction
/*44*/    1028,    //g_SqrtFraction
/*45*/      52,    //g_sct64
/*46*/      52,    //g_sct128
/*47*/      52,    //g_sct256
/*48*/      52,    //g_sct512
/*49*/      52,    //g_sct1024
/*50*/      52,    //g_sct2048
/*51*/     512,    //rgDBPower10
/*52*/      64,    //icosPIbynp
/*53*/      64,    //isinPIbynp
/*54*/     640,    //g_rgiLsfReconLevel
/*55*/     128,    //gLZLTable

/*56*/    2048,    //_DRM_Sel
/*57*/    2048,    //_DRM_Spbox

/*58*/     256,    //byte_to_unary_table
/*59*/     256,    //FLAC__crc8_table
/*60*/     512,    //FLAC__crc16_table

/*61*/     132,    //Ape_gtPowersOfTwoMinusOne
/*62*/     128,    //Ape_gtKSumMinBoundary
/*63*/     260,    //Ape_gtRangeTotalOne
/*64*/     256,    //Ape_gtRangeWidthOne
/*65*/     260,    //Ape_gtRangeTotalTwo
/*66*/     256,    //Ape_gtRangeWidthTwo
/*67*/    1024     //Ape_gtCRC32
};

// éú3é wma table room, ????±í????ò?4byte????
// 0:3é1|￡?other:ê§°ü
int wma_table_room_generate(void)
{
    FILE *fpTable;
    int ret = 0;
    unsigned long size; // 4byte????oóμ?3¤?è
    unsigned long offset = 0;

    // ′′?¨???t
    fpTable = fopen("wma_table_room.bin", "wb");
    if(fpTable == NULL)
    {
        DEBUG("Can't create wma_table_room.bin!\n");
        ret = 1;
        goto Error;
    }

    //////////////////////////////////////////////// WMA TABLE /////////////////////////////////////////////////////////////

    // (1) getMask
    size = (table_size[1]+3)&(~3); // 4byte????
    if(fwrite(getMask, 1, size, fpTable) != size)
    {
        DEBUG("write getMask error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_getMask                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (2) CLSID_CAsfHeaderObjectV0
    size = (table_size[2]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfHeaderObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfHeaderObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfHeaderObjectV0                              = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (3) CLSID_CAsfPropertiesObjectV2
    size = (table_size[3]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfPropertiesObjectV2, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfPropertiesObjectV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfPropertiesObjectV2                          = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (4) CLSID_CAsfStreamPropertiesObjectV1
    size = (table_size[4]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfStreamPropertiesObjectV1, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfStreamPropertiesObjectV1 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV1                    = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (5) CLSID_CAsfContentDescriptionObjectV0
    size = (table_size[5]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfContentDescriptionObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentDescriptionObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentDescriptionObjectV0                  = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (6) CLSID_AsfXAcmAudioErrorMaskingStrategy
    size = (table_size[6]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXAcmAudioErrorMaskingStrategy, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXAcmAudioErrorMaskingStrategy error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXAcmAudioErrorMaskingStrategy                = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (7) CLSID_AsfXSignatureAudioErrorMaskingStrategy
    size = (table_size[7]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXSignatureAudioErrorMaskingStrategy, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXSignatureAudioErrorMaskingStrategy error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXSignatureAudioErrorMaskingStrategy           = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (8) CLSID_AsfXStreamTypeAcmAudio
    size = (table_size[8]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXStreamTypeAcmAudio, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXStreamTypeAcmAudio error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXStreamTypeAcmAudio                           = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (9) CLSID_CAsfContentEncryptionObject
    size = (table_size[9]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfContentEncryptionObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentEncryptionObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentEncryptionObject                      = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (10) CLSID_CAsfExtendedContentDescObject
    size = (table_size[10]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfExtendedContentDescObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfExtendedContentDescObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfExtendedContentDescObject                    = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (11) CLSID_CAsfMarkerObjectV0
    size = (table_size[11]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfMarkerObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfMarkerObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfMarkerObjectV0                               = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (12) CLSID_CAsfLicenseStoreObject
    size = (table_size[12]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfLicenseStoreObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfLicenseStoreObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfLicenseStoreObject                           = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (13) CLSID_CAsfStreamPropertiesObjectV2
    size = (table_size[13]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfStreamPropertiesObjectV2, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfStreamPropertiesObjectV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV2                     = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (14) CLSID_CAsfExtendedStreamPropertiesObject
    size = (table_size[14]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfExtendedStreamPropertiesObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfExtendedStreamPropertiesObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfExtendedStreamPropertiesObject               = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (15) CLSID_CAsfClockObjectV0
    size = (table_size[15]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfClockObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfClockObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfClockObjectV0                                = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (16) CLSID_AsfXMetaDataObject
    size = (table_size[16]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_AsfXMetaDataObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXMetaDataObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXMetaDataObject                               = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (17) CLSID_CAsfPacketClock1
    size = (table_size[17]+3)&(~3); // 4byte????
    if(fwrite(&CLSID_CAsfPacketClock1, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfPacketClock1 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfPacketClock1                                = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    /*
    // (18) CLSID_CAsfContentEncryptionObjectEx
    size = 16; // 4byte????
    if(fwrite(&CLSID_CAsfContentEncryptionObjectEx, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentEncryptionObjectEx error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentEncryptionObjectEx                   = %x,        size_aligned = %x\n", offset, size);
    offset += size;
    */

    // (18) g_rgiHuffDecTblMsk
    size = (table_size[18]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTblMsk, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTblMsk error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTblMsk                                    = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (19) g_rgiHuffDecTblNoisePower
    size = (table_size[19]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTblNoisePower, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTblNoisePower error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTblNoisePower                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (20) g_rgiHuffDecTbl16smOb
    size = (table_size[20]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTbl16smOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl16smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl16smOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (21) g_rgiHuffDecTbl44smOb
    size = (table_size[21]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTbl44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44smOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (22) g_rgiHuffDecTbl16ssOb
    size = (table_size[22]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTbl16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl16ssOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (23) g_rgiHuffDecTbl44ssOb
    size = (table_size[23]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTbl44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44ssOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (24) g_rgiHuffDecTbl44smQb
    size = (table_size[24]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTbl44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44smQb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (25) g_rgiHuffDecTbl44ssQb
    size = (table_size[25]+3)&(~3); // 4byte????
    if(fwrite(g_rgiHuffDecTbl44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44ssQb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (26) rgiMaskMinusPower10
    size = (table_size[26]+3)&(~3); // 4byte????
    if(fwrite(rgiMaskMinusPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgiMaskMinusPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgiMaskMinusPower10                                    = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (27) rgiMaskPlusPower10
    size = (table_size[27]+3)&(~3); // 4byte????
    if(fwrite(rgiMaskPlusPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgiMaskPlusPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgiMaskPlusPower10                                     = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (28) g_rgiBarkFreqV2
    size = (table_size[28]+3)&(~3); // 4byte????
    if(fwrite(g_rgiBarkFreqV2, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiBarkFreqV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiBarkFreqV2                                        = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (29) gRun16smOb
    size = (table_size[29]+3)&(~3); // 4byte????
    if(fwrite(gRun16smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun16smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun16smOb                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (30) gLevel16smOb
    size = (table_size[30]+3)&(~3); // 4byte????
    if(fwrite(gLevel16smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel16smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel16smOb                                           = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (31) gRun16ssOb
    size = (table_size[31]+3)&(~3); // 4byte????
    if(fwrite(gRun16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun16ssOb                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (32) gLevel16ssOb
    size = (table_size[32]+3)&(~3); // 4byte????
    if(fwrite(gLevel16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel16ssOb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (33) gRun44smOb
    size = (table_size[33]+3)&(~3); // 4byte????
    if(fwrite(gRun44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44smOb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (34) gLevel44smOb
    size = (table_size[34]+3)&(~3); // 4byte????
    if(fwrite(gLevel44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44smOb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (35) gRun44ssOb
    size = (table_size[35]+3)&(~3); // 4byte????
    if(fwrite(gRun44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44ssOb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (36) gLevel44ssOb
    size = (table_size[36]+3)&(~3); // 4byte????
    if(fwrite(gLevel44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44ssOb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (37) gRun44smQb
    size = (table_size[37]+3)&(~3); // 4byte????
    if(fwrite(gRun44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44smQb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (38) gLevel44smQb
    size = (table_size[38]+3)&(~3); // 4byte????
    if(fwrite(gLevel44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44smQb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (39) gRun44ssQb
    size = (table_size[39]+3)&(~3); // 4byte????
    if(fwrite(gRun44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44ssQb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (40) gLevel44ssQb
    size = (table_size[40]+3)&(~3); // 4byte????
    if(fwrite(gLevel44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44ssQb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (41) g_InvQuadRootFraction
    size = (table_size[41]+3)&(~3); // 4byte????
    if(fwrite(g_InvQuadRootFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_InvQuadRootFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InvQuadRootFraction                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (42) g_InvQuadRootExponent
    size = (table_size[42]+3)&(~3); // 4byte????
    if(fwrite(g_InvQuadRootExponent, 1, size, fpTable) != size)
    {
        DEBUG("write g_InvQuadRootExponent error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InvQuadRootExponent                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (43) g_InverseFraction
    size = (table_size[43]+3)&(~3); // 4byte????
    if(fwrite(g_InverseFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_InverseFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InverseFraction                                     = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (44) g_SqrtFraction
    size = (table_size[44]+3)&(~3); // 4byte????
    if(fwrite(g_SqrtFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_SqrtFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_SqrtFraction                                        = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (45) g_sct64
    size = (table_size[45]+3)&(~3); // 4byte????
    if(fwrite(&g_sct64, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct64 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct64                                               = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (46) g_sct128
    size = (table_size[46]+3)&(~3); // 4byte????
    if(fwrite(&g_sct128, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct128 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct128                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (47) g_sct256
    size = (table_size[47]+3)&(~3); // 4byte????
    if(fwrite(&g_sct256, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct256 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct256                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (48) g_sct512
    size = (table_size[48]+3)&(~3); // 4byte????
    if(fwrite(&g_sct512, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct512 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct512                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (49) g_sct1024
    size = (table_size[49]+3)&(~3); // 4byte????
    if(fwrite(&g_sct1024, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct1024 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct1024                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (50) g_sct2048
    size = (table_size[50]+3)&(~3); // 4byte????
    if(fwrite(&g_sct2048, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct2048 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct2048                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (51) rgDBPower10
    size = (table_size[51]+3)&(~3); // 4byte????
    if(fwrite(rgDBPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgDBPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgDBPower10                                           = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (52) icosPIbynp
    size = (table_size[52]+3)&(~3); // 4byte????
    if(fwrite(icosPIbynp, 1, size, fpTable) != size)
    {
        DEBUG("write icosPIbynp error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_icosPIbynp                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (53) isinPIbynp
    size = (table_size[53]+3)&(~3); // 4byte????
    if(fwrite(isinPIbynp, 1, size, fpTable) != size)
    {
        DEBUG("write isinPIbynp error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_isinPIbynp                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (54) g_rgiLsfReconLevel
    size = (table_size[54]+3)&(~3); // 4byte????
    if(fwrite(g_rgiLsfReconLevel[0], 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiLsfReconLevel error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiLsfReconLevel                                    = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (55) gLZLTable
    size = (table_size[55]+3)&(~3); // 4byte????
    if(fwrite(gLZLTable, 1, size, fpTable) != size)
    {
        DEBUG("write gLZLTable error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLZLTable                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    //////////////////////////////////////////////// DRM9 TABLE /////////////////////////////////////////////////////////////

    // (56) _DRM_Sel
    size = (table_size[56]+3)&(~3); // 4byte????
    if(fwrite(_DRM_Sel[0], 1, size, fpTable) != size)
    {
        DEBUG("write _DRM_Sel error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET__DRM_Sel                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (57) _DRM_Spbox
    size = (table_size[57]+3)&(~3); // 4byte????
    if(fwrite(_DRM_Spbox[0], 1, size, fpTable) != size)
    {
        DEBUG("write _DRM_Spbox error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET__DRM_Spbox                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;
#if 0
    //////////////////////////////////////////////// FLAC TABLE /////////////////////////////////////////////////////////////

    // (58) byte_to_unary_table
    size = (table_size[58]+3)&(~3); // 4byte????
    if(fwrite(byte_to_unary_table, 1, size, fpTable) != size)
    {
        DEBUG("write byte_to_unary_table error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_byte_to_unary_table                                   = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (59) FLAC__crc8_table
    size = (table_size[59]+3)&(~3); // 4byte????
    if(fwrite(FLAC__crc8_table, 1, size, fpTable) != size)
    {
        DEBUG("write FLAC__crc8_table error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_FLAC__crc8_table                                      = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (60) FLAC__crc16_table
    size = (table_size[60]+3)&(~3); // 4byte????
    if(fwrite(FLAC__crc16_table, 1, size, fpTable) != size)
    {
        DEBUG("write FLAC__crc16_table error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_FLAC__crc16_table                                     = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    //////////////////////////////////////////////// APE TABLE /////////////////////////////////////////////////////////////

    // (61) Ape_gtPowersOfTwoMinusOne
    size = (table_size[61]+3)&(~3); // 4byte????
    if(fwrite(Ape_gtPowersOfTwoMinusOne, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtPowersOfTwoMinusOne error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtPowersOfTwoMinusOne                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (62) Ape_gtKSumMinBoundary
    size = (table_size[62]+3)&(~3); // 4byte????
    if(fwrite(Ape_gtKSumMinBoundary, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtKSumMinBoundary error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtKSumMinBoundary                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (63) Ape_gtRangeTotalOne
    size = (table_size[63]+3)&(~3); // 4byte????
    if(fwrite(Ape_gtRangeTotalOne, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeTotalOne error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeTotalOne                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (64) Ape_gtRangeWidthOne
    size = (table_size[64]+3)&(~3); // 4byte????
    if(fwrite(Ape_gtRangeWidthOne, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeWidthOne error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeWidthOne                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (65) Ape_gtRangeTotalTwo
    size = (table_size[65]+3)&(~3); // 4byte????
    if(fwrite(Ape_gtRangeTotalTwo, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeTotalTwo error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeTotalTwo                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (66) Ape_gtRangeWidthTwo
    size = (table_size[66]+3)&(~3); // 4byte????
    if(fwrite(Ape_gtRangeWidthTwo, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeWidthTwo error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeWidthTwo                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (67) Ape_gtCRC32
    size = (table_size[67]+3)&(~3); // 4byte????
    if(fwrite(Ape_gtCRC32, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtCRC32 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtCRC32                                           = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;
    #endif
Error:

    // 1?±????t
    if(fpTable)
        fclose(fpTable);

    return ret;
}
#endif
#endif

#if 0
unsigned long table_size[] =
{
    0,
    /*01*/     132,    //getMask
    /*02*/      16,    //CLSID_CAsfHeaderObjectV0
    /*03*/      16,    //CLSID_CAsfPropertiesObjectV2
    /*04*/      16,    //CLSID_CAsfStreamPropertiesObjectV1
    /*05*/      16,    //CLSID_CAsfContentDescriptionObjectV0
    /*06*/      16,    //CLSID_AsfXAcmAudioErrorMaskingStrategy
    /*07*/      16,    //CLSID_AsfXSignatureAudioErrorMaskingStrategy
    /*08*/      16,    //CLSID_AsfXStreamTypeAcmAudio
    /*09*/      16,    //CLSID_CAsfContentEncryptionObject
    /*10*/      16,    //CLSID_CAsfExtendedContentDescObject
    /*11*/      16,    //CLSID_CAsfMarkerObjectV0
    /*12*/      16,    //CLSID_CAsfLicenseStoreObject
    /*13*/      16,    //CLSID_CAsfStreamPropertiesObjectV2
    /*14*/      16,    //CLSID_CAsfExtendedStreamPropertiesObject
    /*15*/      16,    //CLSID_CAsfClockObjectV0
    /*16*/      16,    //CLSID_AsfXMetaDataObject
    /*17*/      16,    //CLSID_CAsfPacketClock1
    /*18*/     452,    //g_rgiHuffDecTblMsk
    /*19*/     152,    //g_rgiHuffDecTblNoisePower
    /*20*/    1744,    //g_rgiHuffDecTbl16smOb
    /*21*/    5694,    //g_rgiHuffDecTbl44smOb
    /*22*/    1696,    //g_rgiHuffDecTbl16ssOb
    /*23*/    4004,    //g_rgiHuffDecTbl44ssOb
    /*24*/    2488,    //g_rgiHuffDecTbl44smQb
    /*25*/    2208,    //g_rgiHuffDecTbl44ssQb
    /*26*/     288,    //rgiMaskMinusPower10
    /*27*/     248,    //rgiMaskPlusPower10
    /*28*/     100,    //g_rgiBarkFreqV2
    /*29*/     948,    //gRun16smOb
    /*30*/     948,    //gLevel16smOb
    /*31*/     866,    //gRun16ssOb
    /*32*/     866,    //gLevel16ssOb
    /*33*/    2668,    //gRun44smOb
    /*34*/    2668,    //gLevel44smOb
    /*35*/    2140,    //gRun44ssOb
    /*36*/    2140,    //gLevel44ssOb
    /*37*/    1328,    //gRun44smQb
    /*38*/    1328,    //gLevel44smQb
    /*39*/    1106,    //gRun44ssQb
    /*40*/    1106,    //gLevel44ssQb
    /*41*/    1028,    //g_InvQuadRootFraction
    /*42*/     260,    //g_InvQuadRootExponent
    /*43*/    1028,    //g_InverseFraction
    /*44*/    1028,    //g_SqrtFraction
    /*45*/      52,    //g_sct64
    /*46*/      52,    //g_sct128
    /*47*/      52,    //g_sct256
    /*48*/      52,    //g_sct512
    /*49*/      52,    //g_sct1024
    /*50*/      52,    //g_sct2048
    /*51*/     512,    //rgDBPower10
    /*52*/      64,    //icosPIbynp
    /*53*/      64,    //isinPIbynp
    /*54*/     640,    //g_rgiLsfReconLevel
    /*55*/     128,    //gLZLTable

    /*56*/    2048,    //_DRM_Sel
    /*57*/    2048,    //_DRM_Spbox

    /*58*/     256,    //byte_to_unary_table
    /*59*/     256,    //FLAC__crc8_table
    /*60*/     512,    //FLAC__crc16_table

    /*61*/     132,    //Ape_gtPowersOfTwoMinusOne
    /*62*/     128,    //Ape_gtKSumMinBoundary
    /*63*/     260,    //Ape_gtRangeTotalOne
    /*64*/     256,    //Ape_gtRangeWidthOne
    /*65*/     260,    //Ape_gtRangeTotalTwo
    /*66*/     256,    //Ape_gtRangeWidthTwo
    /*67*/    1024     //Ape_gtCRC32
};

// 生成 wma table room, 每个表格偏移4byte对齐
// 0:成功；other:失败
int wma_table_room_generate(void)
{
    FILE *fpTable;
    int ret = 0;
    unsigned long size; // 4byte对齐后的长度
    unsigned long offset = 0;

    // 创建文件
    fpTable = fopen("wma_table_room.bin", "wb");
    if (fpTable == NULL)
    {
        DEBUG("Can't create wma_table_room.bin!\n");
        ret = 1;
        goto Error;
    }

    //////////////////////////////////////////////// WMA TABLE /////////////////////////////////////////////////////////////

    // (1) getMask
    size = (table_size[1] + 3) & (~3); // 4byte对齐
    if (fwrite(getMask, 1, size, fpTable) != size)
    {
        DEBUG("write getMask error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_getMask                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (2) CLSID_CAsfHeaderObjectV0
    size = (table_size[2] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfHeaderObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfHeaderObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfHeaderObjectV0                              = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (3) CLSID_CAsfPropertiesObjectV2
    size = (table_size[3] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfPropertiesObjectV2, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfPropertiesObjectV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfPropertiesObjectV2                          = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (4) CLSID_CAsfStreamPropertiesObjectV1
    size = (table_size[4] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfStreamPropertiesObjectV1, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfStreamPropertiesObjectV1 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV1                    = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (5) CLSID_CAsfContentDescriptionObjectV0
    size = (table_size[5] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfContentDescriptionObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentDescriptionObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentDescriptionObjectV0                  = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (6) CLSID_AsfXAcmAudioErrorMaskingStrategy
    size = (table_size[6] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_AsfXAcmAudioErrorMaskingStrategy, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXAcmAudioErrorMaskingStrategy error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXAcmAudioErrorMaskingStrategy                = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (7) CLSID_AsfXSignatureAudioErrorMaskingStrategy
    size = (table_size[7] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_AsfXSignatureAudioErrorMaskingStrategy, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXSignatureAudioErrorMaskingStrategy error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXSignatureAudioErrorMaskingStrategy           = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (8) CLSID_AsfXStreamTypeAcmAudio
    size = (table_size[8] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_AsfXStreamTypeAcmAudio, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXStreamTypeAcmAudio error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXStreamTypeAcmAudio                           = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (9) CLSID_CAsfContentEncryptionObject
    size = (table_size[9] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfContentEncryptionObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfContentEncryptionObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentEncryptionObject                      = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (10) CLSID_CAsfExtendedContentDescObject
    size = (table_size[10] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfExtendedContentDescObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfExtendedContentDescObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfExtendedContentDescObject                    = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (11) CLSID_CAsfMarkerObjectV0
    size = (table_size[11] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfMarkerObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfMarkerObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfMarkerObjectV0                               = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (12) CLSID_CAsfLicenseStoreObject
    size = (table_size[12] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfLicenseStoreObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfLicenseStoreObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfLicenseStoreObject                           = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (13) CLSID_CAsfStreamPropertiesObjectV2
    size = (table_size[13] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfStreamPropertiesObjectV2, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfStreamPropertiesObjectV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV2                     = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (14) CLSID_CAsfExtendedStreamPropertiesObject
    size = (table_size[14] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfExtendedStreamPropertiesObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfExtendedStreamPropertiesObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfExtendedStreamPropertiesObject               = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (15) CLSID_CAsfClockObjectV0
    size = (table_size[15] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfClockObjectV0, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfClockObjectV0 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfClockObjectV0                                = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (16) CLSID_AsfXMetaDataObject
    size = (table_size[16] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_AsfXMetaDataObject, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_AsfXMetaDataObject error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_AsfXMetaDataObject                               = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    // (17) CLSID_CAsfPacketClock1
    size = (table_size[17] + 3) & (~3); // 4byte对齐
    if (fwrite(&CLSID_CAsfPacketClock1, 1, size, fpTable) != size)
    {
        DEBUG("write CLSID_CAsfPacketClock1 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfPacketClock1                                = 0x%04x,        size_aligned = %x\n", offset, size);
    offset += size;

    /*
    // (18) CLSID_CAsfContentEncryptionObjectEx
    size = 16; // 4byte对齐
    if(fwrite(&CLSID_CAsfContentEncryptionObjectEx, 1, size, fpTable) != size)
    {
     DEBUG("write CLSID_CAsfContentEncryptionObjectEx error!\n");
     ret = 1;
     goto Error;
    }
    DEBUG("TBL_OFFSET_CLSID_CAsfContentEncryptionObjectEx                   = %x,        size_aligned = %x\n", offset, size);
    offset += size;
    */

    // (18) g_rgiHuffDecTblMsk
    size = (table_size[18] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTblMsk, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTblMsk error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTblMsk                                    = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (19) g_rgiHuffDecTblNoisePower
    size = (table_size[19] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTblNoisePower, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTblNoisePower error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTblNoisePower                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (20) g_rgiHuffDecTbl16smOb
    size = (table_size[20] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTbl16smOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl16smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl16smOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (21) g_rgiHuffDecTbl44smOb
    size = (table_size[21] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTbl44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44smOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (22) g_rgiHuffDecTbl16ssOb
    size = (table_size[22] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTbl16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl16ssOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (23) g_rgiHuffDecTbl44ssOb
    size = (table_size[23] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTbl44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44ssOb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (24) g_rgiHuffDecTbl44smQb
    size = (table_size[24] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTbl44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44smQb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (25) g_rgiHuffDecTbl44ssQb
    size = (table_size[25] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiHuffDecTbl44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiHuffDecTbl44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiHuffDecTbl44ssQb                                  = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (26) rgiMaskMinusPower10
    size = (table_size[26] + 3) & (~3); // 4byte对齐
    if (fwrite(rgiMaskMinusPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgiMaskMinusPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgiMaskMinusPower10                                    = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (27) rgiMaskPlusPower10
    size = (table_size[27] + 3) & (~3); // 4byte对齐
    if (fwrite(rgiMaskPlusPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgiMaskPlusPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgiMaskPlusPower10                                     = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (28) g_rgiBarkFreqV2
    size = (table_size[28] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiBarkFreqV2, 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiBarkFreqV2 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiBarkFreqV2                                        = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (29) gRun16smOb
    size = (table_size[29] + 3) & (~3); // 4byte对齐
    if (fwrite(gRun16smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun16smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun16smOb                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (30) gLevel16smOb
    size = (table_size[30] + 3) & (~3); // 4byte对齐
    if (fwrite(gLevel16smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel16smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel16smOb                                           = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (31) gRun16ssOb
    size = (table_size[31] + 3) & (~3); // 4byte对齐
    if (fwrite(gRun16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun16ssOb                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (32) gLevel16ssOb
    size = (table_size[32] + 3) & (~3); // 4byte对齐
    if (fwrite(gLevel16ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel16ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel16ssOb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (33) gRun44smOb
    size = (table_size[33] + 3) & (~3); // 4byte对齐
    if (fwrite(gRun44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44smOb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (34) gLevel44smOb
    size = (table_size[34] + 3) & (~3); // 4byte对齐
    if (fwrite(gLevel44smOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44smOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44smOb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (35) gRun44ssOb
    size = (table_size[35] + 3) & (~3); // 4byte对齐
    if (fwrite(gRun44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44ssOb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (36) gLevel44ssOb
    size = (table_size[36] + 3) & (~3); // 4byte对齐
    if (fwrite(gLevel44ssOb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44ssOb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44ssOb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (37) gRun44smQb
    size = (table_size[37] + 3) & (~3); // 4byte对齐
    if (fwrite(gRun44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44smQb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (38) gLevel44smQb
    size = (table_size[38] + 3) & (~3); // 4byte对齐
    if (fwrite(gLevel44smQb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44smQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44smQb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (39) gRun44ssQb
    size = (table_size[39] + 3) & (~3); // 4byte对齐
    if (fwrite(gRun44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write gRun44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gRun44ssQb                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (40) gLevel44ssQb
    size = (table_size[40] + 3) & (~3); // 4byte对齐
    if (fwrite(gLevel44ssQb, 1, size, fpTable) != size)
    {
        DEBUG("write gLevel44ssQb error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLevel44ssQb                                          = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (41) g_InvQuadRootFraction
    size = (table_size[41] + 3) & (~3); // 4byte对齐
    if (fwrite(g_InvQuadRootFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_InvQuadRootFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InvQuadRootFraction                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (42) g_InvQuadRootExponent
    size = (table_size[42] + 3) & (~3); // 4byte对齐
    if (fwrite(g_InvQuadRootExponent, 1, size, fpTable) != size)
    {
        DEBUG("write g_InvQuadRootExponent error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InvQuadRootExponent                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (43) g_InverseFraction
    size = (table_size[43] + 3) & (~3); // 4byte对齐
    if (fwrite(g_InverseFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_InverseFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_InverseFraction                                     = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (44) g_SqrtFraction
    size = (table_size[44] + 3) & (~3); // 4byte对齐
    if (fwrite(g_SqrtFraction, 1, size, fpTable) != size)
    {
        DEBUG("write g_SqrtFraction error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_SqrtFraction                                        = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (45) g_sct64
    size = (table_size[45] + 3) & (~3); // 4byte对齐
    if (fwrite(&g_sct64, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct64 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct64                                               = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (46) g_sct128
    size = (table_size[46] + 3) & (~3); // 4byte对齐
    if (fwrite(&g_sct128, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct128 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct128                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (47) g_sct256
    size = (table_size[47] + 3) & (~3); // 4byte对齐
    if (fwrite(&g_sct256, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct256 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct256                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (48) g_sct512
    size = (table_size[48] + 3) & (~3); // 4byte对齐
    if (fwrite(&g_sct512, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct512 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct512                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (49) g_sct1024
    size = (table_size[49] + 3) & (~3); // 4byte对齐
    if (fwrite(&g_sct1024, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct1024 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct1024                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (50) g_sct2048
    size = (table_size[50] + 3) & (~3); // 4byte对齐
    if (fwrite(&g_sct2048, 1, size, fpTable) != size)
    {
        DEBUG("write g_sct2048 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_sct2048                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (51) rgDBPower10
    size = (table_size[51] + 3) & (~3); // 4byte对齐
    if (fwrite(rgDBPower10, 1, size, fpTable) != size)
    {
        DEBUG("write rgDBPower10 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_rgDBPower10                                           = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (52) icosPIbynp
    size = (table_size[52] + 3) & (~3); // 4byte对齐
    if (fwrite(icosPIbynp, 1, size, fpTable) != size)
    {
        DEBUG("write icosPIbynp error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_icosPIbynp                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (53) isinPIbynp
    size = (table_size[53] + 3) & (~3); // 4byte对齐
    if (fwrite(isinPIbynp, 1, size, fpTable) != size)
    {
        DEBUG("write isinPIbynp error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_isinPIbynp                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (54) g_rgiLsfReconLevel
    size = (table_size[54] + 3) & (~3); // 4byte对齐
    if (fwrite(g_rgiLsfReconLevel[0], 1, size, fpTable) != size)
    {
        DEBUG("write g_rgiLsfReconLevel error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_g_rgiLsfReconLevel                                    = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (55) gLZLTable
    size = (table_size[55] + 3) & (~3); // 4byte对齐
    if (fwrite(gLZLTable, 1, size, fpTable) != size)
    {
        DEBUG("write gLZLTable error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_gLZLTable                                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    //////////////////////////////////////////////// DRM9 TABLE /////////////////////////////////////////////////////////////

    // (56) _DRM_Sel
    size = (table_size[56] + 3) & (~3); // 4byte对齐
    if (fwrite(_DRM_Sel[0], 1, size, fpTable) != size)
    {
        DEBUG("write _DRM_Sel error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET__DRM_Sel                                              = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (57) _DRM_Spbox
    size = (table_size[57] + 3) & (~3); // 4byte对齐
    if (fwrite(_DRM_Spbox[0], 1, size, fpTable) != size)
    {
        DEBUG("write _DRM_Spbox error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET__DRM_Spbox                                            = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    //////////////////////////////////////////////// FLAC TABLE /////////////////////////////////////////////////////////////

    // (58) byte_to_unary_table
    size = (table_size[58] + 3) & (~3); // 4byte对齐
    if (fwrite(byte_to_unary_table, 1, size, fpTable) != size)
    {
        DEBUG("write byte_to_unary_table error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_byte_to_unary_table                                   = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (59) FLAC__crc8_table
    size = (table_size[59] + 3) & (~3); // 4byte对齐
    if (fwrite(FLAC__crc8_table, 1, size, fpTable) != size)
    {
        DEBUG("write FLAC__crc8_table error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_FLAC__crc8_table                                      = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (60) FLAC__crc16_table
    size = (table_size[60] + 3) & (~3); // 4byte对齐
    if (fwrite(FLAC__crc16_table, 1, size, fpTable) != size)
    {
        DEBUG("write FLAC__crc16_table error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_FLAC__crc16_table                                     = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    //////////////////////////////////////////////// APE TABLE /////////////////////////////////////////////////////////////

    // (61) Ape_gtPowersOfTwoMinusOne
    size = (table_size[61] + 3) & (~3); // 4byte对齐
    if (fwrite(Ape_gtPowersOfTwoMinusOne, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtPowersOfTwoMinusOne error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtPowersOfTwoMinusOne                             = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (62) Ape_gtKSumMinBoundary
    size = (table_size[62] + 3) & (~3); // 4byte对齐
    if (fwrite(Ape_gtKSumMinBoundary, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtKSumMinBoundary error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtKSumMinBoundary                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (63) Ape_gtRangeTotalOne
    size = (table_size[63] + 3) & (~3); // 4byte对齐
    if (fwrite(Ape_gtRangeTotalOne, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeTotalOne error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeTotalOne                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (64) Ape_gtRangeWidthOne
    size = (table_size[64] + 3) & (~3); // 4byte对齐
    if (fwrite(Ape_gtRangeWidthOne, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeWidthOne error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeWidthOne                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (65) Ape_gtRangeTotalTwo
    size = (table_size[65] + 3) & (~3); // 4byte对齐
    if (fwrite(Ape_gtRangeTotalTwo, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeTotalTwo error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeTotalTwo                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (66) Ape_gtRangeWidthTwo
    size = (table_size[66] + 3) & (~3); // 4byte对齐
    if (fwrite(Ape_gtRangeWidthTwo, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtRangeWidthTwo error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtRangeWidthTwo                                 = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

    // (67) Ape_gtCRC32
    size = (table_size[67] + 3) & (~3); // 4byte对齐
    if (fwrite(Ape_gtCRC32, 1, size, fpTable) != size)
    {
        DEBUG("write Ape_gtCRC32 error!\n");
        ret = 1;
        goto Error;
    }
    DEBUG("TBL_OFFSET_Ape_gtCRC32                                           = 0x%04x,         size_aligned = %x\n", offset, size);
    offset += size;

Error:

    // 关闭文件
    if (fpTable)
        fclose(fpTable);

    return ret;
}
//#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

//BYTE bGlobalDataBuffer[256];
#if 0
/****************************************************************************/
extern "C" size_t
    strlen(const char *p)
{
    int cnt = 0;
    while (*p++)
    {
        cnt++;
    }
    return cnt;
}
#endif

/****************************************************************************/
/*
static tWMAFileStatus
TranslateCode(tWMAStatus rc)
{
    switch(rc)
    {
    case kWMA_NoErr:
        return cWMA_NoErr;
    case kWMA_BrokenFrame:
        return cWMA_BrokenFrame;
    case kWMA_NoMoreFrames:
        return cWMA_NoMoreFrames;
    case kWMA_BadSamplingRate:
        return cWMA_BadSamplingRate;
    case kWMA_BadNumberOfChannels:
        return cWMA_BadNumberOfChannels;
    case kWMA_BadVersionNumber:
        return cWMA_BadVersionNumber;
    case kWMA_BadWeightingMode:
        return cWMA_BadWeightingMode;
    case kWMA_BadPacketisation:
        return cWMA_BadPacketization;
    }

#ifdef LOCAL_DEBUG
    while(1);
#else
    return cWMA_Internal;
#endif
}
*/

//WMA_I32 WMA_CompareWToC(WMA_U8 *pC, WMA_U16 *pW,  WMA_U16 len, WMA_U32 *pValues )
//{
//
//    WMA_U8 *pbTemp = NULL;
//    WMA_U16 i =0, j=0;
//    WMA_U8  fWildFound =0;
//
//    for (i=0; i <len/2; i++)
//    {
//        pbTemp = (WMA_U8 *)pW;
//
//        if (1 == fWildFound && (*pbTemp < '0' || *pbTemp > '9' ))
//        {
//            fWildFound =0;
//            j++;
//        }
//
//        // Using % to skip the comparision of numbers and if pValues are not NULL, Get those values.
//        if (*pC == '%')
//        {
//            fWildFound =1;
//            if (NULL != pValues)
//                pValues[j] = 0;
//        }
//
//        if (1 == fWildFound && NULL != pValues)
//        {
//            // Try to get this values
//            pValues[j] = pValues[j] *10 + *pbTemp - '0';
//        }
//
//
//
//        if (0 == fWildFound && *pC != '?') // Using '?' for wildcard char
//        {
//            if (*pbTemp != *pC)
//               return -1;
//        }
//        pW++;
//        pC++;
//    }
//    return 0;


//WMARESULT WMA_ParseFoldDown(WMA_U16 *pW,  WMA_U32 len, WMA_I32 *pValues, WMA_U16 wNumValIn )
//{
//
//    WMA_U8 *pbTemp = NULL;
//    WMA_U16 i =0, j=0, digits =0;
//    WMA_U8  fMakeNegative = 0;
//
//    if (NULL == pValues)
//        return WMAERR_FAIL;
//
//    memset(pValues, 0, sizeof(WMA_I32)*wNumValIn);
//
//    for (i=0; i <len/2; i++)
//    {
//        pbTemp = (WMA_U8 *)pW;
//
//        if (*pbTemp == ',' )
//        {
//            if (1 == fMakeNegative)
//            {
//                pValues[j] = 0 - pValues[j];
//                fMakeNegative =0;
//            }
//
//            if (j >= wNumValIn -1)
//                return WMAERR_FAIL;
//            j++;
//            digits =0;
//
//        }
//        else if (*pbTemp == '-')
//        {
//            if (digits > 0)
//                return WMAERR_FAIL;
//            fMakeNegative = 1;
//
//        }
//        else if (*pbTemp >= '0' && *pbTemp <= '9' )
//        {
//             pValues[j] = pValues[j] *10 + *pbTemp - '0';
//             digits++;
//        }
//        else if (*pbTemp == 0) // end
//        {
//            if (1 == fMakeNegative)
//            {
//                pValues[j] = 0 - pValues[j];
//                fMakeNegative =0;
//            }
//        }
//        else
//            return WMAERR_FAIL;
//        pW++;
//    }
//
//    if (j != wNumValIn -1)
//        return WMAERR_FAIL;
//
//    return WMA_OK;


#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

/****************************************************************************/
tWMAFileStatus WMAF_UpdateNewPayload(tWMAFileStateInternal *pInt)
{
    /* internal call, so pInt should be okay */

    /* HongCho: separated out these parts from the parsing loop, because
     *          other parts (when a new payload is about to be retrieved)
     *          need to update the offsets and other states.
     *
     *          used in WMAFileDecodeData() and WMARawDecCBGetData ().
     */

    WMAERR wmarc;
    static short packetnumber = 0;//hxd
#ifndef WMAAPI_NO_DRM
    tWMA_U32 cbRead =0;
    tWMAFileStatus hr;
    BYTE *pLast15 = NULL;
#endif

//#ifndef WMAAPI_NO_DRM_STREAM
//    tWMA_U32 cbLeftover =0;
//    BYTE *pbDRMPayload = NULL;
//    WORD wDRMOffset =0;
//#endif


    do
    {
        switch (pInt->parse_state)
        {
            case csWMA_NewAsfPacket:

                packetnumber++;//hxd
                if (pInt->hdr_parse.nextPacketOffset > pInt->hdr_parse.cbLastPacketOffset)
                {
                    return cWMA_NoMoreFrames;
                }

                pInt->hdr_parse.currPacketOffset = pInt->hdr_parse.nextPacketOffset;
                pInt->hdr_parse.nextPacketOffset += pInt->hdr_parse.cbPacketSize;

                /* packet header is parsed hxd*/
                wmarc = WMA_ParsePacketHeader(pInt);
                //DEBUG("packetnumber=%d\n",packetnumber);//hxd
                if (wmarc == WMAERR_BUFFERTOOSMALL)
                {
                    pInt->hdr_parse.nextPacketOffset = pInt->hdr_parse.currPacketOffset;
                    //  pInt->hdr_parse.currPacketOffset -= pInt->hdr_parse.cbPacketSize;
                    return cWMA_NoMoreDataThisTime;
                }


                if (wmarc != WMAERR_OK)
                {
                    return cWMA_BadPacketHeader;
                }

                if (pInt->ppex.fEccPresent && pInt->ppex.fParityPacket)
                {
                    /* HongCho: for some reason, ARM's code thinks a parity packet is
                     *          only at the end...  Here, I am not assuming that.
                     */
                    break;
                }

                pInt->parse_state = csWMA_DecodePayloadStart;//after decoding packetheader we start to decode payload header
                pInt->iPayload = 0;
                break;

                //we start to decode payload header
            case csWMA_DecodePayloadStart:

                if (pInt->iPayload >= pInt->ppex.cPayloads)
                {
                    pInt->parse_state = csWMA_NewAsfPacket;
                    break;
                }

                wmarc = WMA_ParsePayloadHeader(pInt);
                if (wmarc != WMAERR_OK)
                {
                    pInt->parse_state = csWMA_DecodePayloadEnd;
                    break;
                }

                pInt->wPayStart = pInt->payload.cbPacketOffset + pInt->payload.cbTotalSize
                                  - pInt->payload.cbPayloadSize;

//#ifndef WMAAPI_NO_DRM_STREAM
//            if( pInt->payload.bStreamId == pInt->hdr_parse.bDRMAuxStreamNum )
//            {
//                /* Read this payload's data (should be 7-15 bytes) */
//                cbRead = WMAFileCBGetData(
//                                    (tHWMAFileState *) pInt,
//                                    pInt->hdr_parse.currPacketOffset + pInt->wPayStart,
//                                    pInt->payload.cbPayloadSize,
//                                    &pbDRMPayload );
//
//                if (cbRead != pInt->payload.cbPayloadSize)
//                    return (cWMA_NoMoreFrames);
//
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(pbDRMPayload, cbRead))
//                    return cWMA_NoMoreFrames;
//#endif
//
//
//                if (cbRead > sizeof (pInt->rgbNextPayloadEnd))
//                    cbRead = sizeof (pInt->rgbNextPayloadEnd);
//
//                cbLeftover = sizeof (pInt->rgbNextPayloadEnd) - cbRead;
//
//                /* Copy this payload's data into our internal state data */
//
//                // Check if payload size id 8 bytes or 9 bytes. Amit on 09/10/2001
//
//                memset( pInt->rgbNextPayloadEnd, 0, 15 );
//
//                if (pInt->payload.cbPayloadSize == 9)
//                {
//                    pInt->cbNextPayloadEndValid = (BYTE) cbRead - 1;
//
//                    memcpy( pInt->rgbNextPayloadEnd + 7, pbDRMPayload+1, cbRead-1 );
//                }
//                else if (pInt->payload.cbPayloadSize == 8)
//                {
//                    pInt->cbNextPayloadEndValid = (BYTE) cbRead ;
//                    memcpy( pInt->rgbNextPayloadEnd + 7, pbDRMPayload, cbRead);
//                }
//
//                /* Move on to the next payload, which should be the corresponding audio data */
//                pInt->parse_state = csWMA_DecodePayloadEnd;
//                break;
//            }
//#endif  /* WMAAPI_NO_DRM_STREAM */


                if (pInt->payload.cbRepData != 1)
                {
                    pInt->parse_state = csWMA_DecodePayload;
                    pInt->payload.bIsCompressedPayload = 0; // Amit to take care of compressed payloads.
                    break;
                }
                else if (pInt->payload.cbRepData == 1)   // Amit to take care of compressed payloads.
                {
                    pInt->parse_state = csWMA_DecodeCompressedPayload;
                    pInt->payload.bIsCompressedPayload = 1;
                    break;
                }
                /* a payload has to be a multiple of audio "packets" */

                if (pInt->payload.cbPayloadSize % pInt->hdr_parse.nBlockAlign != 0)
                    return cWMA_BrokenFrame;

                pInt->parse_state = csWMA_DecodePayloadEnd;
                break;

            case csWMA_DecodePayload:
                if (pInt->payload.bStreamId != pInt->hdr_parse.wAudioStreamId) // Added by Amit to skip Video Payload
                {
                    pInt->parse_state = csWMA_DecodePayloadEnd;
                    break;

                }
//                pInt->cbPayloadOffset = pInt->hdr_parse.currPacketOffset + pInt->wPayStart;
//                pInt->bBlockStart     = TRUE;
//                pInt->cbBlockLeft     = pInt->hdr_parse.nBlockAlign;
//                pInt->cbPayloadLeft   = pInt->payload.cbPayloadSize - pInt->cbBlockLeft;

                pInt->cbPayloadOffset = pInt->hdr_parse.currPacketOffset + pInt->wPayStart;
                pInt->bBlockStart     = TRUE;
                pInt->cbBlockLeft     = pInt->hdr_parse.nBlockAlign;

                //if(pInt->hdr_parse.nBlockAlign > pInt->hdr_parse.cbPacketSize)

//                if ( pInt->hdr_parse.msDuration == 189354
//                    && pInt->hdr_parse.cbFirstPacketOffset == 672
//                    && pInt->hdr_parse.nBlockAlign == 3413
//                    && pInt->hdr_parse.nChannels == 1
//                    && pInt->hdr_parse.cbLastPacketOffset == 14199072)

                if(pInt->hdr_parse.nBlockAlign == 3413
                    &&pInt->hdr_parse.cbPacketSize==3200
                    &&(pInt->payload.cbPayloadSize==3174||pInt->payload.cbPayloadSize==239))
                {
                    pInt->cbPayloadLeft = 0;
                }
                else
                    pInt->cbPayloadLeft   = pInt->payload.cbPayloadSize - pInt->cbBlockLeft;


                /* new payload, so take care of DRM */

#ifdef WMDRM_PORTABLE
                if (pInt->bHasDRM || pInt->bHasJanusDRM)
#else
                if (pInt->bHasDRM)
#endif
                {

#ifdef WMAAPI_NO_DRM

                    return cWMA_DRMUnsupported;

#else  /* WMAAPI_NO_DRM */

//#ifndef WMAAPI_NO_DRM_STREAM
//
//                if( 0 != pInt->cbNextPayloadEndValid )
//                {
//                    /* We pre-cached the last bytes of this payload - no need to seek / read */
//                    pLast15 = pInt->rgbNextPayloadEnd;
//
//                    /* Move the bytes to the appropriate offset */
//                    wDRMOffset = pInt->payload.cbPayloadSize % 8;
//
//                    if( ( 0 != wDRMOffset ) && ( 8 == pInt->cbNextPayloadEndValid ) )
//                    {
//                        memmove( pLast15 + 7 - wDRMOffset, pLast15 + 7, 8 );
//                        memset( pLast15 + 15 - wDRMOffset, 0, wDRMOffset );
//                    }
//                }
//                else
//                {
//
//#endif  /* WMAAPI_NO_DRM_STREAM */


                    /* We need to seek & read the last data from the end of this payload */
                    cbRead = WMAFileCBGetData(
                                    (tHWMAFileState *)pInt,
                                    pInt->cbPayloadOffset + pInt->payload.cbPayloadSize - 15,
                                    15,
                                    &pLast15 );

                    if (cbRead != 15)
                        return (cWMA_NoMoreFrames);

//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(pLast15, cbRead))
//                    return cWMA_NoMoreFrames;
//#endif



//#ifndef WMAAPI_NO_DRM_STREAM
//                }
//#endif  /* WMAAPI_NO_DRM_STREAM */


                /* Initialize DRM so it can decrypt this payload properly */
//#ifdef WMDRM_PORTABLE
//                if( pInt->bHasJanusDRM )
//                {
//                    hr = DRM_MGR_InitDecrypt( pInt->pJanusDecryptContext,
//                                              pLast15,
//                                              pInt->payload.cbPayloadSize );
//                }
//                else
//#endif // WMDRM_PORTABLE


                hr = CDrmPD_InitPacket__(pInt->pDRM_state, pLast15, pInt->payload.cbPayloadSize);
                if(hr != S_OK)
                {
                    return cWMA_DRMFailed;
                }

//#ifndef WMAAPI_NO_DRM_STREAM
//                pInt->cbNextPayloadEndValid = 0;
//#endif  /* WMAAPI_NO_DRM_STREAM */


#endif /* WMAAPI_NO_DRM */


                }

                /* Done updating */
                /*            if (pInt->bDecInWaitState == 1)
                            {
                                pInt->parse_state = csWMA_DecodeLoopStart;
                                pInt->bDecInWaitState =0;

                            }
                            else */
                {
                    pInt->parse_state = csWMA_DecodeAudioData;
                }
                return cWMA_NoErr;

            case csWMA_DecodePayloadEnd:
                pInt->iPayload++;
                //DEBUG("payloadnumber=%d\n",pInt->iPayload);
                pInt->parse_state = csWMA_DecodePayloadStart;
                break;

            case csWMA_DecodeCompressedPayload: // Added by Amit to take care of compressed payloads
                if (pInt->payload.bStreamId != pInt->hdr_parse.wAudioStreamId) // Added by Amit to skip Video Payload
                {
                    pInt->parse_state = csWMA_DecodePayloadEnd;
                    break;
                }
                pInt->cbPayloadOffset = pInt->hdr_parse.currPacketOffset + pInt->wPayStart;
                pInt->bBlockStart     = TRUE;
                pInt->cbBlockLeft     = pInt->hdr_parse.nBlockAlign;
                pInt->payload.wBytesRead = 0;
                pInt->payload.bSubPayloadState = 1;

                /****************************************************************************************/

                /* new payload, so take care of DRM */

//#ifdef WMDRM_PORTABLE
//                if (pInt->bHasDRM || pInt->bHasJanusDRM)
//#else
                if (pInt->bHasDRM)
//#endif
                {

#ifdef WMAAPI_NO_DRM

                    return cWMA_DRMUnsupported;

#else  /* WMAAPI_NO_DRM */

//#ifndef WMAAPI_NO_DRM_STREAM
//
//                if( 0 != pInt->cbNextPayloadEndValid )
//                {
//                    /* We pre-cached the last bytes of this payload - no need to seek / read */
//                    pLast15 = pInt->rgbNextPayloadEnd;
//
//                    /* Move the bytes to the appropriate offset */
//                    wDRMOffset = pInt->payload.cbPayloadSize % 8;
//
//                    if( ( 0 != wDRMOffset ) && ( 8 == pInt->cbNextPayloadEndValid ) )
//                    {
//                        memmove( pLast15 + 7 - wDRMOffset, pLast15 + 7, 8 );
//                        memset( pLast15 + 15 - wDRMOffset, 0, wDRMOffset );
//                    }
//                }
//                else
//                {
//
//#endif  /* WMAAPI_NO_DRM_STREAM */


                    /* We need to seek & read the last data from the end of this payload */
                    cbRead = WMAFileCBGetData(
                                    (tHWMAFileState *)pInt,
                                    pInt->cbPayloadOffset + pInt->payload.cbPayloadSize - 15,
                                    15,
                                    &pLast15 );

                    if (cbRead != 15)
                        return (cWMA_NoMoreFrames);

//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(pLast15, cbRead))
//                    return cWMA_NoMoreFrames;
//#endif


//#ifndef WMAAPI_NO_DRM_STREAM
//                }
//#endif  /* WMAAPI_NO_DRM_STREAM */

                /* Initialize DRM so it can decrypt this payload properly */

//#ifdef WMDRM_PORTABLE
//                if( pInt->bHasJanusDRM )
//                {
//                    hr = DRM_MGR_InitDecrypt( pInt->pJanusDecryptContext,
//                                              pLast15,
//                                              pInt->payload.cbPayloadSize );
//                }
//                else
//#endif // WMDRM_PORTABLE
                hr = CDrmPD_InitPacket__(pInt->pDRM_state, pLast15, pInt->payload.cbPayloadSize);
                if(hr != S_OK)
                {
//#ifdef LOCAL_DEBUG
//                    SerialPrintf("++ WMA_UpdateNewPayload: CDrmPD_InitPacket failed (0x%08x).\n\r", hr);
//#endif /* LOCAL_DEBUG */
                    return cWMA_DRMFailed;
                }

//#ifndef WMAAPI_NO_DRM_STREAM
//                pInt->cbNextPayloadEndValid = 0;
//#endif  /* WMAAPI_NO_DRM_STREAM */


#endif /* WMAAPI_NO_DRM */


                }

                /*******************************************************************************************/
                pInt->parse_state = csWMA_DecodeAudioData;

                return cWMA_NoErr;
                break;

            default:
                return cWMA_Internal;
        }

    }
    while (1);

    return cWMA_NoErr;
}
#pragma arm section code

#endif


/* ===========================================================================
 * WMAFileMBRAudioStreams
--------------------------------------------------------------------------- */
//tWMAFileStatus WMAFileMBRAudioStreams (tWMAFileHdrState *pstate,
//                                       tWMA_U16 *pTotalAudioStreams)
//{
//
//    tWMAFileHdrStateInternal *pInt = (tWMAFileHdrStateInternal *)pstate;
//    WMAERR wmarc = WMAERR_OK;
//
//    if(sizeof(tWMAFileHdrState) != sizeof(tWMAFileHdrStateInternal))
//    {
//        /* this should NOT happen */
//        WMADebugMessage("** Internal Error: sizeof(tWMAFileHdrStateInternal) = %d.\n\r",
//                sizeof(tWMAFileHdrStateInternal));
//        // while(1);
//        return cWMA_BadArgument;
//    }
//    if(NULL == pInt ||
//       NULL == pTotalAudioStreams)
//    {
//        return cWMA_BadArgument;
//    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadWritePtr(pTotalAudioStreams, sizeof(tWMA_U16)))
//        return cWMA_BadArgument;
//#endif
//
//    *pTotalAudioStreams = pInt->cAudioStreams;
//
//    return wmarc;
//} // WMAFileMBRAudioStreams


/* ===========================================================================
 * WMAFileMBRAudioStreams
--------------------------------------------------------------------------- */
//tWMAFileStatus WMAFileSetTargetMBRAudioStream (tWMAFileHdrState *pstate,
//                                               tWMA_U16 wTargetAudioStream)
//{
//
//    tWMAFileHdrStateInternal *pInt = (tWMAFileHdrStateInternal *)pstate;
//    WMAERR wmarc = WMAERR_OK;
//
//    if(sizeof(tWMAFileHdrState) != sizeof(tWMAFileHdrStateInternal))
//    {
//        /* this should NOT happen */
//        WMADebugMessage("** Internal Error: sizeof(tWMAFileHdrStateInternal) = %d.\n\r",
//                sizeof(tWMAFileHdrStateInternal));
//        // while(1);
//        return cWMA_BadArgument;
//    }
//    if(NULL == pInt ||
//       0 == wTargetAudioStream)
//    {
//        return cWMA_BadArgument;
//    }
//
//    pInt->wTargetAudioStreamNumber = wTargetAudioStream;
//
//
//    return wmarc;
//} // WMAFileMBRAudioStreams



/* ===========================================================================
 * WMAFileIsWMA
--------------------------------------------------------------------------- */
//tWMAFileStatus WMAFileIsWMA (tWMAFileHdrState *pstate)
//{

//    tWMAFileHdrStateInternal *pInt = (tWMAFileHdrStateInternal *)pstate;
//    WMAERR wmarc = WMAERR_OK;

//    if(sizeof(tWMAFileHdrState) != sizeof(tWMAFileHdrStateInternal))
//    {
//        /* this should NOT happen */
//        WMADebugMessage("** Internal Error: sizeof(tWMAFileHdrStateInternal) = %d.\n\r",
//                sizeof(tWMAFileHdrStateInternal));
//        // while(1);
//        return cWMA_BadArgument;
//    }
//    if(pInt == NULL)
//        return cWMA_BadArgument;
//
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pstate, sizeof(tWMAFileHdrState)))
//        return cWMA_BadArgument;
//#endif


/* parse ASF header */

//    wmarc = WMA_ParseAsfHeader(pInt, 0);
//    if(wmarc != WMAERR_OK)
//        return cWMA_BadAsfHeader;

// Amit 03/15/2002 Check Audio properties before init codec

//    if (pInt->nSamplesPerSec == 0||
//        pInt->nChannels == 0||
//        pInt->nBlockAlign == 0 ||
//        pInt->nAvgBytesPerSec ==0
//        )
//        return cWMA_BadAsfHeader;


//    return cWMA_NoErr;
//}

/* ===========================================================================
 * WMAGetLicenseStore
--------------------------------------------------------------------------- */
//BYTE * WMAGetLicenseStore (tWMAFileHdrState *pstate,tWMA_U32 *pLen)
//{
//    tWMAFileHdrStateInternal *pInt = (tWMAFileHdrStateInternal *)pstate;
//
//    if(sizeof(tWMAFileHdrState) != sizeof(tWMAFileHdrStateInternal))
//    {
//        /* this should NOT happen */
//        WMADebugMessage("** Internal Error: sizeof(tWMAFileHdrStateInternal) = %d.\n\r",
//                sizeof(tWMAFileHdrStateInternal));
//        // while(1);
//        return NULL;
//    }
//    if((pInt == NULL) || (pLen == NULL))
//        return NULL;
//
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pstate, sizeof(tWMAFileHdrState)))
//        return NULL;
//    if (IsBadReadPtr(pLen, sizeof(tWMA_U32)))
//        return NULL;
//#endif
//
//
//    *pLen = pInt->m_dwLicenseLen;
//
//    return pInt->m_pLicData;


/* ===========================================================================
 * WMAFileDecodeClose
--------------------------------------------------------------------------- */
//tWMAFileStatus WMAFileDecodeClose (tHWMAFileState* phstate)
//{
//    tWMAFileStateInternal *pInt;
//    unsigned int i =0;

//    if ((phstate == NULL) ||(*phstate == NULL))
//        return cWMA_BadArgument;

//    pInt = (tWMAFileStateInternal*) (*phstate);

//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(*phstate, sizeof(tWMAFileStateInternal)))
//        return cWMA_BadArgument;
//#endif



//    if (pInt != NULL ) {

//#ifdef  USE_SPDTX
//        spdtxDelete (pInt->hSPDTX);
//        if( pInt->szmdata ) free( pInt->szmdata );
//        pInt->szmdata = NULL;
//        pInt->hSPDTX  = NULL;
//#else

//audecDelete (pInt->hWMA);
//        pInt->hWMA = NULL;
// #endif

//#ifndef WMAAPI_NO_DRM
//        if (pInt->pDRM_state != NULL) {
//            free (pInt->pDRM_state);
//            pInt->pDRM_state = NULL;
//        }
//#endif


//#ifdef WMDRM_PORTABLE
//        if( pInt->pJanusContext != NULL )
//        {
//            free( pInt->pJanusContext );
//            pInt->pJanusContext = NULL;
//        }
//
//        if( pInt->pJanusDecryptContext != NULL )
//        {
//            free( pInt->pJanusDecryptContext );
//            pInt->pJanusDecryptContext = NULL;
//        }
//
//        if( pInt->hdr_parse.pbCEncExData != NULL )
//        {
//            free( pInt->hdr_parse.pbCEncExData );
//            pInt->hdr_parse.pbCEncExData = NULL;
//        }
//#endif


//#if 0
//        if (pInt->hdr_parse.m_pMarkers) {
//            for (i=0;i<pInt->hdr_parse.m_dwMarkerNum;i++)
//                if ( pInt->hdr_parse.m_pMarkers[i].m_pwDescName != NULL )
//                {
//                    free (pInt->hdr_parse.m_pMarkers[i].m_pwDescName);
//                    pInt->hdr_parse.m_pMarkers[i].m_pwDescName = NULL;
//                }
//            free (pInt->hdr_parse.m_pMarkers);
//            pInt->hdr_parse.m_pMarkers = NULL;
//        }
//#endif

//        if (pInt->hdr_parse.m_pLicData)
//        {
//            free (pInt->hdr_parse.m_pLicData);
//            pInt->hdr_parse.m_pLicData = NULL;
//        }
//
//        if(NULL != pInt->hdr_parse.m_pDesc)
//  {
//   tWMAFileContDesc *pDesc = pInt->hdr_parse.m_pDesc;
//   free(pDesc->pTitle);
//   free(pDesc->pAuthor);
//   free(pDesc->pCopyright);
//   free(pDesc->pDescription);
//   free(pDesc->pRating);
//   free(pDesc);
//   pInt->hdr_parse.m_pDesc = 0;
//  }
//
//        if(pInt->hdr_parse.m_pECDesc != NULL) {
//            if(pInt->hdr_parse.m_pECDesc->cDescriptors > 0) {
//                for (i = 0; i < (unsigned int) pInt->hdr_parse.m_pECDesc->cDescriptors; i++) {
//                    free(pInt->hdr_parse.m_pECDesc->pDescriptors[i].uValue.pbBinary);
//                    free(pInt->hdr_parse.m_pECDesc->pDescriptors[i].pwszName);
//                    pInt->hdr_parse.m_pECDesc->pDescriptors[i].uValue.pbBinary = NULL;
//                    pInt->hdr_parse.m_pECDesc->pDescriptors[i].pwszName = NULL;
//                }
//                free(pInt->hdr_parse.m_pECDesc->pDescriptors);
//                pInt->hdr_parse.m_pECDesc->pDescriptors = NULL;
//            }
//            free(pInt->hdr_parse.m_pECDesc);
//            pInt->hdr_parse.m_pECDesc = NULL;
//        }
//
//
//        // Free Metadata
//        if(pInt->hdr_parse.ptMetaDataEntry != NULL)
//        {
//            if(pInt->hdr_parse.ptMetaDataEntry->m_wDescRecordsCount > 0)
//            {
//                for (i = 0; i < (unsigned int) pInt->hdr_parse.ptMetaDataEntry->m_wDescRecordsCount; i++)
//                {
//                    if (NULL != pInt->hdr_parse.ptMetaDataEntry->pDescRec)
//                    {
//                        if (pInt->hdr_parse.ptMetaDataEntry->pDescRec[i].pwName)
//                            free(pInt->hdr_parse.ptMetaDataEntry->pDescRec[i].pwName);
//                        pInt->hdr_parse.ptMetaDataEntry->pDescRec[i].pwName = NULL;
//
//                        if (pInt->hdr_parse.ptMetaDataEntry->pDescRec[i].pData)
//                            free(pInt->hdr_parse.ptMetaDataEntry->pDescRec[i].pData);
//                        pInt->hdr_parse.ptMetaDataEntry->pDescRec[i].pData = NULL;
//                    }
//                }
//                free(pInt->hdr_parse.ptMetaDataEntry->pDescRec);
//                pInt->hdr_parse.ptMetaDataEntry->pDescRec = NULL;
//            }
//            free(pInt->hdr_parse.ptMetaDataEntry);
//            pInt->hdr_parse.ptMetaDataEntry = NULL;
//        }
//
//

// if (pInt->pCallBackBuffer != NULL)
//     free(pInt->pCallBackBuffer);
//        pInt->pCallBackBuffer = NULL;

// if (pInt->pPlayerInfo != NULL)
// {
//  WMAPlayerInfo *pPlayerInfo = (WMAPlayerInfo *)pInt->pPlayerInfo;
//  if (NULL != pPlayerInfo->rgiMixDownMatrix)
//   free(pPlayerInfo->rgiMixDownMatrix);

//  free(pPlayerInfo);
// }
//        pInt->pPlayerInfo = NULL;


//    free (pInt);
//        *phstate = NULL;
//    }
//    phstate = NULL;
//    return cWMA_NoErr;
//}

/* ===========================================================================
 * WMAFileDecodeCreate
--------------------------------------------------------------------------- */
//tWMAFileStateInternal gStateInt;//20080724
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

tWMAFileStatus WMAFileDecodeCreate(tHWMAFileState *phstate)
{
    tWMAFileStateInternal *pInt;

//    if (NULL == phstate)
    //        return cWMA_BadArgument;






    // first try to close in case someone calls us without prior close
    //WMAFileDecodeClose (phstate);

    // Now start to allocate and initialize

    pInt = (tWMAFileStateInternal*) & gStateInt;//malloc (sizeof (tWMAFileStateInternal));

    if (pInt == NULL)
        return cWMA_Failed;

    memset(pInt, 0, sizeof(tWMAFileStateInternal));  //make element of wmafilestateinternal structure to 0

    /* initialize the some state */

    pInt->parse_state = csWMA_HeaderStart;

    /* parse ASF header */

    *phstate = (tHWMAFileState) pInt; //g_state=&phstate=&pInt

    return cWMA_NoErr;
}


/* ===========================================================================
 * WMAFileDecodeInit
--------------------------------------------------------------------------- */
tWMAFileStatus WMAFileDecodeInitEx(tHWMAFileState hstate, tWMA_U16 nDecoderFlags,
                                   tWMA_U16 nDRCSetting,  BOOL bDropPacket,
                                   tWMA_U32 nDstChannelMask,
                                   tWMA_U32 nInterpResampRate,
                                   PCMFormat* pPCMFormat,
                                   tWMA_U16 wTargetAudioStream)
{
    tWMAFileStateInternal *pInt;
    WMAERR wmarc = WMAERR_OK;
    WMARESULT wmar = WMA_OK;
    WMAPlayerInfo PlayerInfo;
    WMAFormat WMAFormat;
    tWMA_U16 nPlayerOpt = 0;
    WMA_U16 i = 0;


    WMA_I32 *rgiMixDownMatrix = NULL; // Can be null to invoke defaults
    WMA_I32 iPeakAmplitudeRef = 0;
    WMA_I32 iRmsAmplitudeRef = 0;
    WMA_I32 iPeakAmplitudeTarget = 0;
    WMA_I32 iRmsAmplitudeTarget = 0;
    WMA_U16 nInChannels = 0;
    WMA_U16 nOutChannels = 0;
    WMA_U32 nMask = 0;



#ifndef WMAAPI_NO_DRM
    tWMAFileStatus hr;
#endif // WMAAPI_NO_DRM
#ifdef USE_WOW_FILTER
    WowControls         tempWowControls;
#endif

    // we dont have drc headers from ASF file yet.
    if (NULL == pPCMFormat ||
            0 == wTargetAudioStream)
    {
        return cWMA_BadArgument;
    }

    pInt = (tWMAFileStateInternal*) hstate;

    if (pInt == NULL)
        return cWMA_BadArgument;


    pInt->hdr_parse.wTargetAudioStreamNumber = wTargetAudioStream;

    /* asf header parsing hxd*/
    wmarc = WMA_ParseAsfHeader(&pInt->hdr_parse, 1);
    if (wmarc != WMAERR_OK)
        return cWMA_BadAsfHeader;

// Amit 03/15/2002 Check Audio properties before init codec

    if (pInt->hdr_parse.nSamplesPerSec == 0 ||
            pInt->hdr_parse.nChannels == 0 ||
            pInt->hdr_parse.nBlockAlign == 0 ||
            pInt->hdr_parse.nAvgBytesPerSec == 0
       )
        return cWMA_BadAsfHeader;







    /* Set up and initialize the WMA bitstreamd decoder */


    pInt->hWMA = audecNew(NULL, 0);
    if (!pInt->hWMA) return cWMA_Failed;


    memset(&PlayerInfo, 0, sizeof(PlayerInfo));
    PlayerInfo.nPlayerOpt = nPlayerOpt;
    WMAFormat.wFormatTag           = (WMA_U16) pInt->hdr_parse.wFormatTag;
    WMAFormat.nChannels            = (WMA_U16) pInt->hdr_parse.nChannels;
    WMAFormat.nSamplesPerSec       = (WMA_U32) pInt->hdr_parse.nSamplesPerSec;
    WMAFormat.nAvgBytesPerSec      = (WMA_U32) pInt->hdr_parse.nAvgBytesPerSec;
    WMAFormat.nBlockAlign          = (WMA_U16) pInt->hdr_parse.nBlockAlign;
    WMAFormat.nValidBitsPerSample  = (WMA_U16) pInt->hdr_parse.wValidBitsPerSample;
    WMAFormat.nChannelMask         = (WMA_U32) pInt->hdr_parse.dwChannelMask;
    WMAFormat.wEncodeOpt           = (WMA_U16) pInt->hdr_parse.nEncodeOpt;
    pPCMFormat->cbPCMContainerSize   = (WMA_U32) pInt->hdr_parse.wBitsPerSample / 8;//output is 16bit pcm
    pPCMFormat->nSamplesPerSec       = (WMA_U32) pInt->hdr_parse.nSamplesPerSec;
    pPCMFormat->nValidBitsPerSample  = (WMA_U16) pInt->hdr_parse.wValidBitsPerSample;

    pPCMFormat->nSamplesPerSec = WMAFormat.nSamplesPerSec;
    pPCMFormat->nChannels = WMAFormat.nChannels;
    pPCMFormat->nChannelMask = WMAFormat.nChannelMask;
    pPCMFormat->nValidBitsPerSample = WMAFormat.nValidBitsPerSample;
    pPCMFormat->cbPCMContainerSize = (WMAFormat.nValidBitsPerSample + 7) / 8;
    pPCMFormat->pcmData = PCMDataPCM;

    wmar = audecInit(pInt->hWMA, &WMAFormat, pPCMFormat, &PlayerInfo, &pInt->audecState, NULL);
    if (WMA_FAILED(wmar)) return wmar;

    // Dont Free rgiMixDownMatrix as we need it in pInt->pPlayerInfo


    if (wmar != WMA_OK)
        return cWMA_Failed;




    /* Set up the decryption if necessary */

    pInt->bHasDRM = (BOOL) 0;
#ifdef WMDRM_PORTABLE
        pInt->bHasJanusDRM = (BOOL) 0 ;
#endif

#ifdef WMDRM_NETWORK
        if( !pInt->bHasWMDRMNetworkDRM )
        {
#endif
#ifdef WMDRM_PORTABLE
    //    if( ( pInt->hdr_parse.cbSecretData > 0 ) ||
    //        ( pInt->hdr_parse.cbCEncExData > 0 ) )
    //    {
    //        DRM_RESULT dr;
    //        DRM_STRING *rgRights[ 1 ];
    //
    //        if( pInt->pJanusContext != NULL )
    //        {
    //            free( pInt->pJanusContext );
    //        }
    //
    //        pInt->pJanusContext = malloc( sizeof( DRM_MANAGER_CONTEXT ) );
    //        if( NULL == pInt->pJanusContext )
    //        {
    //            return( cWMA_DRMFailed );
    //        }
    //
    //        if( pInt->pJanusDecryptContext )
    //        {
    //            free( pInt->pJanusDecryptContext );
    //        }
    //
    //        pInt->pJanusDecryptContext = malloc( sizeof( DRM_MANAGER_DECRYPT_CONTEXT ) );
    //        if( NULL == pInt->pJanusDecryptContext )
    //        {
    //            return( cWMA_DRMFailed );
    //        }
    //
    //        //
    //        // Request the playback right; change this if the device is doing
    //        // anything else with the file.
    //        //
    //        rgRights[ 0 ] = (DRM_STRING *) &g_dstrWMDRM_RIGHT_PLAYBACK;
    //
    //        dr = DRM_MGR_Initialize( pInt->pJanusContext, &s_strLicense );
    //        if( DRM_SUCCESS != dr )
    //        {
    //            return( cWMA_DRMFailed );
    //        }
    //
    //        dr = DRM_SUCCESS;
    //
    //        do
    //        {
    //            if( pInt->hdr_parse.cbSecretData > 0 )
    //            {
    //                //
    //                // Tell DRM_MGR about the old header object, if it exists.
    //                //
    //                dr = DRM_MGR_SetV1Header( pInt->pJanusContext,
    //                                          (DRM_BYTE *) pInt->hdr_parse.pbKeyID,
    //                                          32,
    //                                          (DRM_BYTE *) pInt->hdr_parse.pbSecretData,
    //                                          pInt->hdr_parse.cbSecretData,
    //                                          (DRM_BYTE *) "",
    //                                          2 );
    //                if( DRM_SUCCESS != dr )
    //                {
    //                    break;
    //                }
    //            }
    //
    //            if( pInt->hdr_parse.cbCEncExData > 0 )
    //            {
    //                //
    //                // The DRM_MGR about the new header object, which will exist.
    //                //
    //                dr = DRM_MGR_SetV2Header( pInt->pJanusContext,
    //                                          (DRM_BYTE *) pInt->hdr_parse.pbCEncExData,
    //                                          pInt->hdr_parse.cbCEncExData );
    //                if( DRM_SUCCESS != dr )
    //                {
    //                    break;
    //                }
    //            }
    //
    //            //
    //            // Find the license for this content.
    //            //
    //            dr = DRM_MGR_Bind( pInt->pJanusContext,
    //                               (const DRM_CONST_STRING **) rgRights,
    //                               1,
    //                               NULL,
    //                               NULL,
    //                               pInt->pJanusDecryptContext );
    //            if( DRM_SUCCESS == dr )
    //            {
    //                //
    //                // Since we're definitely going to play the file, go ahead
    //                // and commit right now.
    //                //
    //                dr = DRM_MGR_Commit( pInt->pJanusContext );
    //                if( DRM_SUCCESS != dr )
    //                {
    //                    break;
    //                }
    //
    //                pInt->bHasJanusDRM = (BOOL) 1;
    //            }
    //            else
    //            {
    //                //
    //                // OK, just no license. That's fine; try for a PDDRM license.
    //                //
    //                dr = DRM_SUCCESS;
    //            }
    //        }
    //        while( FALSE );
    //
    //        DRM_MGR_Uninitialize( pInt->pJanusContext );
    //        free( pInt->pJanusContext );
    //        pInt->pJanusContext = NULL;
    //
    //        if( DRM_SUCCESS != dr )
    //        {
    //            return( cWMA_DRMFailed );
    //        }
    //
    //    }

#endif
#ifdef WMDRM_NETWORK
        }
#endif

#ifdef WMDRM_NETWORK
        if( !pInt->bHasWMDRMNetworkDRM )
        {
#endif
#ifdef WMDRM_PORTABLE
        if( !pInt->bHasJanusDRM && ( pInt->hdr_parse.cbSecretData > 0 ) )
#else
        if(pInt->hdr_parse.cbSecretData > 0)
#endif
        {
            /* only for DRM now */
            char *p = (char *)pInt->hdr_parse.pbType;

            if(p[0] == 'D' && p[1] == 'R' && p[2] == 'M' && p[3] == '\0')
            {
                pInt->bHasDRM = (BOOL)( 1 );

#ifndef WMAAPI_NO_DRM
                pInt->pDRM_state = (void*)&DRM_state;//malloc (SAFE_DRM_SIZE);
                if (pInt->pDRM_state == NULL)
                    return cWMA_DRMFailed;

                hr = CDrmPD_Init__(pInt->pDRM_state);
                if(hr != S_OK)
                    return cWMA_DRMFailed;
#endif /* WMAAPI_NO_DRM */
            }
            else
                return cWMA_BadDRMType;
        }
#ifdef WMDRM_NETWORK
        }
#endif

#ifdef WMDRM_PORTABLE
        if( ( pInt->hdr_parse.cbSecretData > 0 ) ||
            ( pInt->hdr_parse.cbCEncExData > 0 ) )
        {
            if( !pInt->bHasDRM && !pInt->bHasJanusDRM )
            {
                //
                // No license.
                //
                return( cWMA_DRMFailed );
            }
        }
#endif

#ifdef WMDRM_NETWORK
        if( pInt->bHasWMDRMNetworkDRM )
        {
            if(pInt->hdr_parse.cbSecretData == 0)
            {
                //
                // Sanity: There should be a content encryption object.
                //
                return cWMA_DRMFailed;
            }
        }
#endif




    pInt->hdr_parse.cbLastPacketOffset = pInt->hdr_parse.cbFirstPacketOffset;
    if (pInt->hdr_parse.cPackets > 0)
    {
        if (bDropPacket) --pInt->hdr_parse.cPackets;
        pInt->hdr_parse.cbLastPacketOffset += ((U64)pInt->hdr_parse.cPackets - 1) * pInt->hdr_parse.cbPacketSize;
    }

    // If broadcast flag is set, packet count is invalid. Override cbLastPacketOffset
    if (FILEPROPFLAG_BROADCAST & pInt->hdr_parse.dwFilePropertiesFlags)
    {
        // Set last packet offset to MAX - 1 packet. This prevents us from wrapping around.
        pInt->hdr_parse.cbLastPacketOffset = -1; // By -1 we mean MAX UNSIGNED VALUE
        pInt->hdr_parse.cbLastPacketOffset -= pInt->hdr_parse.cbPacketSize;
    }


    pInt->hdr_parse.currPacketOffset = pInt->hdr_parse.cbHeader;// Added by amit
    pInt->hdr_parse.nextPacketOffset = pInt->hdr_parse.cbHeader;
    pInt->parse_state = csWMA_NewAsfPacket;



//#ifdef WMDRM_NETWORK
//    if( pInt->bHasWMDRMNetworkDRM )
//    {
//        //
//        // Allocate a larger buffer so we can decrypt whole media objects
//        // at a time.
//        //
//        pInt->pCallBackBuffer = (BYTE *)malloc(pInt->hdr_parse.cbAudioSize);
//    }
//    else
//    {
//        pInt->pCallBackBuffer = (BYTE *)malloc(CALLBACK_BUFFERSIZE);
//    }
//#else

    //pInt->pCallBackBuffer = (BYTE *)gCallBackBuf;//malloc(CALLBACK_BUFFERSIZE);
//#endif
    //if(NULL == pInt->pCallBackBuffer)
    //    return cWMA_Internal;

    return cWMA_NoErr;
}

/* ===========================================================================
 * WMAFileDecodeInfo
--------------------------------------------------------------------------- */

tWMAFileStatus WMAFileDecodeInfo(tHWMAFileState hstate,
                                 tWMAFileHeader *hdr)
{
    tWMAFileStateInternal *pInt;
    pInt = (tWMAFileStateInternal*) hstate;

    //if(pInt == NULL || hdr == NULL)
    //        return cWMA_BadArgument;




    /* Fill in the structure */

    hdr->version      = (tWMAFileVersion)pInt->hdr_parse.nVersion;
    hdr->num_channels = (tWMAFileChannels)pInt->hdr_parse.nChannels;

    hdr->sample_rate = pInt->hdr_parse.nSamplesPerSec;
    hdr->duration            = pInt->hdr_parse.msDuration;
    hdr->packet_size         = pInt->hdr_parse.cbPacketSize;
    hdr->first_packet_offset = pInt->hdr_parse.cbFirstPacketOffset;
    hdr->last_packet_offset  = pInt->hdr_parse.cbLastPacketOffset;

    hdr->has_DRM             = (tWMA_U32)pInt->bHasDRM;

    hdr->LicenseLength       = (tWMA_U32)pInt->hdr_parse.m_dwLicenseLen;

    hdr->bitrate             = pInt->hdr_parse.nAvgBytesPerSec * 8;

    // Added in V9
    hdr->pcm_format_tag        = pInt->hdr_parse.wPCMFormatTag;

    hdr->bits_per_sample       = pInt->hdr_parse.wBitsPerSample;

    hdr->valid_bits_per_sample = pInt->hdr_parse.wValidBitsPerSample;

    hdr->channel_mask          = pInt->hdr_parse.dwChannelMask;

    hdr->subformat_data1       = pInt->hdr_parse.SubFormat.Data1;

    hdr->subformat_data2       = pInt->hdr_parse.SubFormat.Data2;

    hdr->subformat_data3       = pInt->hdr_parse.SubFormat.Data3;

    memcpy(hdr->subformat_data4, pInt->hdr_parse.SubFormat.Data4, 8);

    return cWMA_NoErr;
}


/* ===========================================================================
 * WMAFileContentDesc
--------------------------------------------------------------------------- */
tWMAFileStatus WMAFileContentDesc(tHWMAFileState hstate, const tWMAFileContDesc **ppDesc)
{
    tWMAFileStateInternal *pInt;
    DWORD cbOffset = 0;
    DWORD cbWanted = 0;
    DWORD cbActual = 0;

    pInt = (tWMAFileStateInternal*) hstate;
    //if(pInt == NULL || ppDesc == NULL)
    //        return cWMA_BadArgument;


//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pInt, sizeof(tWMAFileStateInternal)))
//        return cWMA_BadArgument;
//#endif


    if (pInt->hdr_parse.m_pDesc == NULL)
    {
        *ppDesc = NULL;
    }
    else
    {
        *ppDesc  = pInt->hdr_parse.m_pDesc;
    }

    return cWMA_NoErr;
}

/* ===========================================================================
 * WMAFileExtendedContentDesc
--------------------------------------------------------------------------- */
tWMAFileStatus WMAFileExtendedContentDesc(tHWMAFileState hstate, const tWMAExtendedContentDesc **pECDesc)
{
    tWMAFileStateInternal *pInt;

    pInt = (tWMAFileStateInternal*) hstate;
    //if((pInt == NULL) || (pECDesc == NULL))
    //        return cWMA_BadArgument;


//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pInt, sizeof(tWMAFileStateInternal)))
//        return cWMA_BadArgument;
//#endif



    if (pInt->hdr_parse.m_pECDesc == NULL)
    {
        *pECDesc = NULL;
    }
    else
    {
        *pECDesc = pInt->hdr_parse.m_pECDesc;
    }

    return cWMA_NoErr;
}

#ifndef WMAAPI_NO_DRM
/* ===========================================================================
 * WMAFormatDate
--------------------------------------------------------------------------- */
void WMAFormatDate( tWMADateParams LicDate, BYTE *pbuffer )
{
    pbuffer[0] = (BYTE)(LicDate.year /100);
    pbuffer[1] = (BYTE)(LicDate.year %100);
    pbuffer[2] = (BYTE)(LicDate.month);
    pbuffer[3] = (BYTE)(LicDate.day);
}
/* ===========================================================================
 * WMAFileLicenseInit
--------------------------------------------------------------------------- */
tWMAFileStatus WMAFileLicenseInit (tHWMAFileState hstate, tWMAFileLicParams *lic_params, tWMA_U32 rights, tWMADateParams currentDate)
{
    tWMAFileStateInternal *pInt;

    pInt = (tWMAFileStateInternal*) hstate;

    if(pInt == NULL || lic_params == NULL)
        return cWMA_BadArgument;

#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pInt, sizeof(tWMAFileStateInternal)))
        return cWMA_BadArgument;
    if (IsBadReadPtr(lic_params, sizeof(tWMAFileLicParams)))
        return cWMA_BadArgument;
#endif

#ifdef WMDRM_PORTABLE
    if( pInt->bHasJanusDRM )
    {
        return( cWMA_NoErr );
    }
#endif

    if(pInt->bHasDRM)
    {

#ifdef WMAAPI_NO_DRM

        return cWMA_DRMUnsupported;

#else  /* WMAAPI_NO_DRM */

        tWMAFileStatus hr;
        PMLICENSE pmlic;
        DWORD dwRight;

        BYTE *pData;
        DWORD dwOffset;
        DWORD dwActual;
        BYTE dateArray[DATE_LEN];

        /* set up for InitDecrypt */

        memset (&pmlic, 0, sizeof(pmlic));
        memcpy (pmlic.ld.KID, (char *)pInt->hdr_parse.pbKeyID,
               strlen((const char *)pInt->hdr_parse.pbKeyID) + 1);
        if (currentDate.day) {
            WMAFormatDate(currentDate,dateArray);
            memcpy (pmlic.ld.appSec, APPSEC_1100, APPSEC_LEN);
            memcpy (pmlic.ld.expiryDate, dateArray, DATE_LEN);
        }
        else {
            memcpy (pmlic.ld.appSec, APPSEC_1000, APPSEC_LEN);
            memset(pmlic.ld.expiryDate, 0, DATE_LEN);
        }


        dwRight = rights;
        //NOTE: This statement masks out any unwanted set bit in the user supplied flag
        //Any new rights added to the API will have to be added here as well
        dwRight &= (WMA_NONSDMI_LIC | WMA_SDMI_LIC | WMA_BURNCD_LIC);

        dwRight = 3;

        memcpy (pmlic.ld.rights, (BYTE *)&dwRight, RIGHTS_LEN);

        dwOffset = 0;
        dwActual = WMA_MAX_DATA_REQUESTED;

        pData = pInt->hdr_parse.m_pLicData;
        dwActual = pInt->hdr_parse.m_dwLicenseLen;

        //NOTE: The dwOffset and dwAcual were suppose to return the number of bytes still to be supplied for
        // complete license if we do not supply entire license at one time. now they are not needed anymore
        //as we supply the entire license at once. So no checking on them
        hr = CDrmPD_InitDecrypt__(pInt->pDRM_state,
                                    &pmlic,
                                    lic_params->pPMID,
                                    (LENGTH_TYPE)lic_params->cbPMID,
                                    pData,
                                    &dwOffset,
                                    &dwActual);

        if (hr == S_OK)
            return cWMA_NoErr;
#endif /* WMAAPI_NO_DRM */
    }

    return cWMA_DRMFailed;
}
#endif
#pragma arm section code
#endif

/******************************************************************************/
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

tWMAFileStatus WMAFileSeek (tHWMAFileState hstate, tWMA_U32 msSeek, tWMA_U32 *pmsActual)
{
    tWMA_U32 iPacket = 0;
    return (WMAFileSeekBase (hstate, & msSeek, & iPacket, pmsActual, WMAB_FALSE, WMAB_FALSE));
}

/* ===========================================================================
 * WMAFileSeek
--------------------------------------------------------------------------- */
tWMAFileStatus WMAFileSeekBase(tHWMAFileState hstate, tWMA_U32 * pmsSeek,
                               tWMA_U32 * piPacket, tWMA_U32 * pmsActual,
                               BOOL fProWMAEncoder, BOOL fSeekToPacket)
{
    tWMA_U32 msSeek = *pmsSeek;
    tWMAFileStateInternal *pInt;
    tWMA_U32 nPacket = *piPacket;
    WMARESULT wmar = WMA_OK;
    tWMA_U8 bDone =0;
    tWMA_U32 i =0;
    tWMA_U32 msPacketDuration = 0;
    tWMA_U32 msDiff=0;
    tWMA_U8 goForward =0;
    tWMA_U8 dwGoingForward =0;
    tWMA_U8 dwGoingBackword =0;
    tWMA_U8 bGotAudioPayload =0;


    pInt = (tWMAFileStateInternal*) hstate;
    //if(pInt == NULL || pmsActual == NULL )
    //    {
    //        return cWMA_BadArgument;
    //    }


    //if(pInt->hWMA   == NULL) return cWMA_BadArgument;
//
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pInt, sizeof(tWMAFileStateInternal)))
//        return cWMA_BadArgument;
//#endif


    *pmsActual = 0;

    /* which packet? */

    if (fSeekToPacket != WMAB_TRUE)
    {
        nPacket = 0;
        if (pInt->hdr_parse.msDuration > 0)
        {
            /* rounding down because I want the requested time frame to be
            * played */

            if (msSeek > pInt->hdr_parse.msDuration)
                return cWMA_BadArgument;

            msPacketDuration = pInt->hdr_parse.msDuration / pInt->hdr_parse.cPackets;
            if (0 != msPacketDuration)
                nPacket = msSeek / msPacketDuration;
            else
                nPacket = 1;    //fix a bug when cPackets is faked to be very big

            //NQF, 061800, temp fix for the seek() bug reported by Creative
            if ((nPacket) && (nPacket % 16 == 0))
            {
                nPacket++;
            }
        }
    }
    /* see if it's within the bounds */

    if (nPacket >= pInt->hdr_parse.cPackets)
        nPacket = pInt->hdr_parse.cPackets - 1; // Go to last packet

    /* Amit: Skip VideoPayload  Bug : if that packet doesnt contain any audio payload we should go to next packet*/

    do
    {
        /* parse the packet and the payload header
         *
         * a bit of a duplication from WMAF_UpdateNewPayload...
         */

        pInt->hdr_parse.currPacketOffset = pInt->hdr_parse.cbHeader
                                           + nPacket * pInt->hdr_parse.cbPacketSize;

        if (WMA_ParsePacketHeader(pInt) != WMAERR_OK)
        {
            return cWMA_BadPacketHeader;
        }

        if (pInt->ppex.fEccPresent && pInt->ppex.fParityPacket)
        {
            return cWMA_BadPacketHeader;
        }

        pInt->iPayload = 0;

        for (i = 0; i < pInt->ppex.cPayloads; i++)
        {
            if (WMA_ParsePayloadHeader(pInt) != WMAERR_OK)
            {
                return cWMA_BadPacketHeader;
            }

            /* Amit: Skip VideoPayload */

            if (pInt->payload.bStreamId == pInt->hdr_parse.wAudioStreamId)
            {
                *pmsActual = pInt->payload.msObjectPres - pInt->hdr_parse.msPreroll;
                bGotAudioPayload = 1;
                break;
            }
            pInt->iPayload++;
        }
        if (bGotAudioPayload)
        {
            if (fSeekToPacket == WMAB_TRUE)
            {
                bDone = 1;
                break;
            }
            bGotAudioPayload = 0;
            if ((dwGoingBackword == 1) && (dwGoingForward == 1))
                bDone = 1;

            if (*pmsActual > msSeek)
            {
                msDiff = *pmsActual - msSeek;
                goForward = 0;
                dwGoingForward = 1;
            }
            else
            {
                msDiff =  msSeek - *pmsActual;
                goForward = 1;
                dwGoingBackword = 1;
            }
        }
        else
        {
            if (fSeekToPacket == WMAB_TRUE)
            {
                goForward = 0;
            }

            if (goForward == 0)
            {
                if (nPacket > 0)
                {
                    nPacket--;
                }
                else
                {
                    goForward = 1;
                    dwGoingBackword = 1;
                    nPacket++;
                }
            }
            else
                nPacket++;

            continue;
        }

        if (fProWMAEncoder == WMAB_TRUE)
        {
            if ((msDiff == 0) || (bDone == 1))
                goto lexit;
            if (nPacket == 0)
                goto lexit;
        }
        else
        {
            if ((msDiff <  5 * msPacketDuration / 4) /*|| ((dwGoingBackword == 1) &&(dwGoingForward == 1)) */)
                bDone = 1;

            //for the hack
            if ((nPacket == 0) || (bDone && nPacket % 16 != 0))
            {
                goto lexit;
            }
        }

        if (goForward)
            nPacket++;
        else
            nPacket--;


    }
    while ((nPacket < pInt->hdr_parse.cPackets));

    if (bDone != 1)
    {
        nPacket = pInt->hdr_parse.cPackets;
        *pmsActual = pInt->hdr_parse.msDuration;
    }

lexit:

    /* reset the states */

    wmar = audecReset(pInt->hWMA);
    pInt->audecState = audecStateInput;

    if (wmar != WMA_OK)
        return cWMA_Failed;
    pInt->hdr_parse.nextPacketOffset = pInt->hdr_parse.cbHeader
                                       + nPacket * pInt->hdr_parse.cbPacketSize;
    pInt->parse_state = csWMA_NewAsfPacket;
    //wchen: it seems after seek decodeStatus won't be called until this guy is 2
//    pInt->bDecInWaitState = 2;
    *pmsSeek = *pmsActual;
    *piPacket = nPacket;
    return cWMA_NoErr;
}

/****************************************************************************/
WMARESULT WMAFileGetInput(tWMAFileStateInternal *pInt, U8 **ppBuffer, U32 *pcbBuffer, WMAFileGetInputParam* pgdParam)
{

    tWMA_U32 num_bytes = WMA_MAX_DATA_REQUESTED;
    tWMA_U32 cbActual = 0;
    tWMA_U32 cbWanted = 0;
    BYTE *pbuff = NULL;
    tWMAFileStatus rc;
    tWMAParseState parse_state;

#ifndef WMAAPI_NO_DRM
      WMARESULT hr;
#endif // WMAAPI_NO_DRM


    pgdParam->m_fTimeIsValid = 0;

    //if(pInt == NULL || ppBuffer == NULL || pcbBuffer == NULL)
    //    {
    //        if(ppBuffer != NULL)
    //        {
    //            *ppBuffer = NULL;
    //        }
    //        if(pcbBuffer != NULL)
    //        {
    //            *pcbBuffer = 0;
    //        }
    //
    //        return WMA_E_INVALIDARG;
    //    }



    *ppBuffer = NULL;
    *pcbBuffer = 0;

    /* If we used up the current payload, try to get the
     * next one.
     */

    // Added by Amit to take care of compressed payloads
    do
    {
        switch (pInt->payload.bIsCompressedPayload)
        {


            case 1:
                {
                    do
                    {
                        switch (pInt->payload.bSubPayloadState)
                        {
                            case 1: // Compressed payload just started
                                cbWanted = 1; //to read subpayload length
                                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, pInt->cbPayloadOffset, cbWanted, &pbuff);
                                if ((cbActual != cbWanted) || NULL == pbuff)
                                    return WMA_E_NO_MORE_SRCDATA;


                                pInt->cbPayloadOffset += cbWanted;
                                pInt->bBlockStart = TRUE;
                                pInt->cbBlockLeft = pInt->hdr_parse.nBlockAlign;
                                pInt->payload.wSubCount = 0;

                                pInt->payload.bNextSubPayloadSize = pbuff[0];
                                pInt->payload.wSubpayloadLeft = pInt->payload.bNextSubPayloadSize;
                                if (pInt->payload.wSubpayloadLeft > 0)
                                    pInt->payload.wSubpayloadLeft -= (WORD)pInt->cbBlockLeft;

                                if (pInt->payload.wTotalDataBytes > pInt->payload.bNextSubPayloadSize)
                                    pInt->payload.wBytesRead = pInt->payload.bNextSubPayloadSize + 1;
                                else if (pInt->payload.wTotalDataBytes == pInt->payload.bNextSubPayloadSize)
                                    pInt->payload.wBytesRead = pInt->payload.bNextSubPayloadSize;

                                pInt->payload.bSubPayloadState = 2;
                                break;
                            case 2: // Subpayload started
                                if (pInt->cbBlockLeft == 0 && pInt->payload.wSubpayloadLeft == 0)
                                {
                                    pInt->payload.bSubPayloadState = 3;
                                    break;
                                }
                                else
                                {
                                    if (pInt->cbBlockLeft == 0)
                                    {
                                        if (/*pInt->cbPayloadLeft*/pInt->payload.wSubpayloadLeft == 0) /* done with the file */
                                            return WMA_S_NEWPACKET;

                                        if (pInt->payload.wSubpayloadLeft > 0)
                                            pInt->payload.wSubpayloadLeft -= (WORD) pInt->hdr_parse.nBlockAlign;
                                        pInt->bBlockStart = TRUE;
                                        pInt->cbBlockLeft = pInt->hdr_parse.nBlockAlign;
                                    }
                                    if (num_bytes > pInt->cbBlockLeft)
                                        num_bytes = pInt->cbBlockLeft;

                                    *pcbBuffer = (unsigned int)WMAFileCBGetData((tHWMAFileState *)pInt,
                                                 pInt->cbPayloadOffset, num_bytes, ppBuffer);
                                    if (*pcbBuffer != num_bytes)
                                        return WMA_E_NO_MORE_SRCDATA;


//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                            if (IsBadReadPtr(ppBuffer, *pcbBuffer))
//                                return WMA_E_NO_MORE_SRCDATA;
//#endif


                                    if ((*pcbBuffer <= CALLBACK_BUFFERSIZE) && ppBuffer && *ppBuffer)
                                    {
                                        //memcpy(pInt->pCallBackBuffer, *ppBuffer,*pcbBuffer);
                                        pInt->pCallBackBuffer = g_pBuffer;
                                        //*ppBuffer = pInt->pCallBackBuffer;
                                    }
                                    else
                                    {
                                        *ppBuffer = NULL;
                                        return WMA_S_NEWPACKET;

                                    }

                                    pInt->cbPayloadOffset += *pcbBuffer;
                                    //pInt->payload.wSubpayloadLeft -= *pcbBuffer;
                                    pInt->cbBlockLeft     -= *pcbBuffer;

//#ifdef WMDRM_PORTABLE
//                            if (pInt->bHasJanusDRM)
//                            {
//                                hr = DRM_MGR_Decrypt(
//                                    pInt->pJanusDecryptContext,
//                                    *ppBuffer,
//                                    *pcbBuffer);
//                                if( hr != S_OK )
//                                {
//                                    *ppBuffer = NULL;
//                                    *pcbBuffer = 0;
//                                    return WMA_S_NEWPACKET;
//                                }
//                            }
//#endif // WMDRM_PORTABLE


                            if (pInt->bHasDRM)
                            {
#ifdef WMAAPI_NO_DRM
                                return WMA_S_NEWPACKET;
#else  /* WMAAPI_NO_DRM */
                                hr = CDrmPD_Decrypt__ (pInt->pDRM_state, *ppBuffer, (LENGTH_TYPE)(*pcbBuffer));
                                if (hr != S_OK)
                                {
                                    *ppBuffer = NULL;
                                    *pcbBuffer = 0;
                                    return WMA_S_NEWPACKET;
                                }

#endif /* WMAAPI_NO_DRM */

                            }

                                    if (pInt->bBlockStart)
                                    {
                                        if (pInt->payload.wSubpayloadLeft == pInt->payload.bNextSubPayloadSize - pInt->hdr_parse.nBlockAlign)
                                        {
                                            pgdParam->m_fTimeIsValid = 1;
                                            pgdParam->m_iTime = 10000 * (tWMA_I64)((pInt->payload.msObjectPres - pInt->hdr_parse.msPreroll) + pInt->payload.dwDeltaTime * pInt->payload.wSubCount);
                                            pInt->payload.wSubCount++;
                                        }

                                        pInt->bBlockStart = FALSE;
                                        return WMA_S_NEWPACKET;
                                    }

                                    return WMA_OK;
                                }

                                break;
                            case 3: // Subpayload finished
                                if (pInt->payload.wTotalDataBytes > pInt->payload.wBytesRead)
                                { // there are payloads to decode
                                    cbWanted = 1; //to read subpayload length
                                    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                                                pInt->cbPayloadOffset, cbWanted, &pbuff);
                                    if ((cbActual != cbWanted) || NULL == pbuff)
                                        return WMA_E_NO_MORE_SRCDATA;

                                    pInt->cbPayloadOffset += cbWanted;
                                    pInt->bBlockStart     = TRUE;
                                    pInt->cbBlockLeft     = pInt->hdr_parse.nBlockAlign;


                                    pInt->payload.bNextSubPayloadSize = pbuff[0];
                                    pInt->payload.wSubpayloadLeft = pInt->payload.bNextSubPayloadSize;
                                    if (pInt->payload.wSubpayloadLeft > 0)
                                        pInt->payload.wSubpayloadLeft -= (WORD)pInt->cbBlockLeft;
                                    pInt->payload.wBytesRead += pInt->payload.bNextSubPayloadSize + 1;
                                    pInt->payload.bSubPayloadState = 2;
                                }
                                else
                                    pInt->payload.bSubPayloadState = 4; // all subpayloads finished
                                break;

                            case 4: // All Subpayloads finished

                                parse_state = pInt->parse_state;
                                pInt->payload.bSubPayloadState = 0;
                                pInt->cbPayloadLeft = 0;
                                pInt->payload.bIsCompressedPayload = 0;

                                pInt->parse_state = csWMA_DecodePayloadEnd;
                                rc = WMAF_UpdateNewPayload(pInt);
                                if (rc == cWMA_NoMoreDataThisTime)
                                {
                                    *pcbBuffer = 0;
                                    return WMA_OK;
                                }

                                pInt->parse_state = parse_state;  //restore
                                if ((rc != cWMA_NoErr))
                                    return WMA_S_NEWPACKET;
                                break;
                            default:
                                return WMA_S_NEWPACKET;

                        }
                    }
                    while (1);

                    break;
                }
            default :
                {
                    if (pInt->cbBlockLeft == 0 && pInt->cbPayloadLeft == 0)
                    {
                        tWMAFileStatus rc1;
                        tWMAParseState parse_state1;

                        parse_state1 = pInt->parse_state;

                        pInt->parse_state = csWMA_DecodePayloadEnd;//payloadend and new payload start
                        rc1 = WMAF_UpdateNewPayload(pInt);//begin new payload
                        if (rc1 == cWMA_NoMoreDataThisTime)
                        {
                            *pcbBuffer = 0;
                            return WMA_OK;
                        }
                        pInt->parse_state = parse_state1; /* restore */

                        if (pInt->payload.bIsCompressedPayload == 1)
                            break;

                        if (cWMA_NoMoreFrames == rc1)
                        {
                            *pcbBuffer = 0;
                            return WMA_E_NO_MORE_SRCDATA;
                        }

                        if (rc1 != cWMA_NoErr)
                            return WMA_S_NEWPACKET;
                    }

                    /* return as much as we currently have left */

                    if (pInt->cbBlockLeft == 0)
                    {
                        if (pInt->cbPayloadLeft == 0)
                        {
                            /* done with the file */
                            return WMA_S_NEWPACKET;
                        }

                        pInt->cbPayloadLeft -= pInt->hdr_parse.nBlockAlign;
                        pInt->bBlockStart = TRUE;
                        pInt->cbBlockLeft = pInt->hdr_parse.nBlockAlign;
                    }
                    if (num_bytes > pInt->cbBlockLeft)
                        num_bytes = pInt->cbBlockLeft;

                    *pcbBuffer = (unsigned int)WMAFileCBGetData((tHWMAFileState *)pInt,
                                 pInt->cbPayloadOffset, num_bytes, ppBuffer);

                    if (*pcbBuffer != num_bytes)
                        return WMA_E_NO_MORE_SRCDATA;
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(ppBuffer, *pcbBuffer))
//                    return WMA_E_NO_MORE_SRCDATA;
//#endif


                    if (*pcbBuffer <= CALLBACK_BUFFERSIZE)
                    {
                        //memcpy(pInt->pCallBackBuffer, *ppBuffer,*pcbBuffer);
                        pInt->pCallBackBuffer = g_pBuffer;
                        //*ppBuffer = pInt->pCallBackBuffer;//delete by huweiguo,2009-5-16
                    }
                    else
                    {
                        *ppBuffer = NULL;
                        return WMA_S_NEWPACKET;

                    }

                    pInt->cbPayloadOffset += *pcbBuffer;
                    pInt->cbBlockLeft     -= *pcbBuffer;

                    /* DRM decryption if necessary */

//#ifdef WMDRM_PORTABLE
//                if( pInt->bHasJanusDRM )
//                {
//                    hr = DRM_MGR_Decrypt( pInt->pJanusDecryptContext,
//                                          *ppBuffer,
//                                          *pcbBuffer);
//                    if( hr != S_OK )
//                    {
//                        *ppBuffer = NULL;
//                        *pcbBuffer = 0;
//                        return WMA_S_NEWPACKET;
//                    }
//                }
//#endif // WMDRM_PORTABLE


                if (pInt->bHasDRM)
                {

#ifdef WMAAPI_NO_DRM
                    return WMA_S_NEWPACKET;
#else  /* WMAAPI_NO_DRM */

                    hr = CDrmPD_Decrypt__ (pInt->pDRM_state, *ppBuffer, (LENGTH_TYPE)(*pcbBuffer));
                    if (hr != S_OK)
                    {
                        *ppBuffer = NULL;
                        *pcbBuffer = 0;
                        return WMA_S_NEWPACKET;
                    }

#endif /* WMAAPI_NO_DRM */

                }


                    if (pInt->bBlockStart)
                    {
                        pInt->bBlockStart = FALSE;
                        //hopefully the following test if the beginning of a payload
                        if (pInt->cbPayloadLeft == pInt->payload.cbPayloadSize - pInt->hdr_parse.nBlockAlign)
                        {
                            pgdParam->m_fTimeIsValid = 1;
                            pgdParam->m_iTime = 10000 * (tWMA_I64)(pInt->payload.msObjectPres - pInt->hdr_parse.msPreroll);
                        }

//                        if (pInt->payload.cbPayloadSize == 239
//                            && pInt->hdr_parse.msDuration == 189354
//                            && pInt->hdr_parse.cbFirstPacketOffset == 672
//                            && pInt->hdr_parse.nBlockAlign==3413
//                            && pInt->hdr_parse.nChannels == 1
//                            &&  pInt->hdr_parse.cbLastPacketOffset == 14199072)
                            if(pInt->payload.cbPayloadSize == 239
                                &&pInt->hdr_parse.nBlockAlign==3413
                                &&pInt->hdr_parse.cbPacketSize==3200)

                            break;
                        else
                            return WMA_S_NEWPACKET;

                    }

                    return WMA_OK;
                }
        }


    }
    while (1);

    return WMA_OK;
}
#pragma arm section code

#endif

#endif
#endif
