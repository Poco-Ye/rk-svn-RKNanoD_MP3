/* ==========================================================================
	
	Copyright (c) 2013 Sony Corporation.
	All Rights Reserved. Proprietary and Confidential.
	
========================================================================== */

/*
 * FILENAME     : getThaiChar.c
 *
 * ABSTRACT     : THAI FONT API
 *
 * HISTORY      : 2007/08/27   created by Sony
 *                2007/11/22   modified by Sony
 *                  �ؤͺϤ��ե���ȷ���ȡ�Õr�����������å�������
 *                2008/03/07   modified by Sony
 *                  getThaiCharNum�v��׷��
 *                2008/03/17   modified by Sony
 *                  �ե�������ȡ�ùw����ȫ�ƥե���ȥǩ`���΂��򷵤��褦���
 *                2008/03/20   modified by Sony
 *                  1.�}���������Υե���Ȥˌ���
 *                  2.������Υǩ`����Х��ȅgλ���\�ä�����ϤȤ����Ǥʤ����Ϥ�
 *                    ����ѥ��륪�ץ������Ф�Q����褦�ˤ���
 *                  3.�v���إå������Ȥ�����
 *                2008/05/15   modified by Sony
 *                  �����Z��ʾ�˘��� Ver.1.0���`���˰餤�����¤�������gʩ
 *                  1.ǰ�I��(E8,E9,EA,EB�Τ����줫�δΤ�ED���Ф���������椨��)
 *                    ׷�ӌ���
 *                  2.�ե�����x�k���������(һ��ǰ���i������Ť�������ED��׷��)��
 *                    �ե���ȷ����x�k�����ˌ���
 *                2012/3/5  modified by M. Kinoshita (Sony)
 *                  1. 12/16dot�����ީ`��
 */

#include <string.h>
#include "FontUseSetting.h"
#include "getThaiChar.h"
#include "stdint.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* D3������� */
#define _D3CODE_THAI    0x0E33  /* D3����(UNICODE) */
#define _EDCODE_THAI    0x0E4D  /* D3���ŉ�Q���`��1(UNICODE) */
#define _D2CODE_THAI    0x0E32  /* D3���ŉ�Q���`��2(UNICODE) */

/* �Х��ȅgλ���Ф��Ϥ� */
#define ROUNDUP(x)  (((x)+7)/8)

/* �����Z�ե���ȥǩ`�� */
#ifdef USEFONT_12dot
extern unsigned char FontAdr_THAI12[];
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
extern unsigned char FontAdr_THAI16[];
#endif //USEFONT_16dot

/* �����Z�ե���ȥǩ`�����Ʃ`�֥똋���� */
typedef struct {
    short   nType;              /* �ե���ȥ����� */
    short   nWidth;             /* �ե���ȷ� */
    short   nHeight;            /* �ե���ȸߤ� */
    short   nDataWidth;         /* �ե���ȥǩ`���� */
    short   nDataHeight;        /* �ե���ȥǩ`���ߤ� */
    unsigned char *pFontData;	/* �ե���ȥǩ`���ؤΥݥ��� */
} ThaiFontInfo;

/* �����Z�ե���ȥǩ`�����Ʃ`�֥� */
#ifdef USEFONT_12dot
static const ThaiFontInfo _thaiFontInfo12[] = {
    {THAI_TYPE_12,
THAI_TYPE_12_WIDTH, 
THAI_TYPE_12_HEIGHT, 
THAI_TYPE_12_DATA_WIDTH, 
THAI_TYPE_12_DATA_HEIGHT, 
FontAdr_THAI12}  /* 12pt�ե���� */
};
#endif //USEFONT_12dot

#ifdef USEFONT_16dot
static const ThaiFontInfo _thaiFontInfo16[] = {
    {THAI_TYPE_16,
THAI_TYPE_16_WIDTH, 
THAI_TYPE_16_HEIGHT, 
THAI_TYPE_16_DATA_WIDTH, 
THAI_TYPE_16_DATA_HEIGHT, 
FontAdr_THAI16}  /* 16pt�ե���� */
};
#endif //USEFONT_16dot

/* ��`�����v���ץ�ȥ��������� */
static ThaiFontInfo* _getThaiCharInfo(short nType);
static unsigned short _checkCharTHAI(unsigned short nUnicode, short nD3ChangeOverlay);
static short _checkCharTHAI2(unsigned short *pString);
static short _checkOverlayCharTHAI(unsigned short nAscii);
static void _copyFontDataTHAI(unsigned char *pDst, unsigned char *pSrc, short nSize, short nBitOR);
static unsigned short _getType1FontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode);
static unsigned short _getType2FontNumTHAI(unsigned short nCode);
static short _getAddFontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode);
static short _execOverlayTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode,
                              unsigned char *pFontData, short nDataSize, unsigned char *pImage);


#define TRUE    1
#define FALSE   0

/** �����Z�ե��������ȡ�ä���
  *
  *   @param    nType       (i)�ե���ȷN�
  *
  *   @return   NULL����    �����Z�ե�������Ʃ`�֥륢�ɥ쥹
  *   @return   NULL        ȡ��ʧ��
  */
ThaiFontInfo* _getThaiCharInfo(short nType)
{
    switch( nType ){
#ifdef USEFONT_12dot
    case THAI_TYPE_12:
        return (ThaiFontInfo*)&_thaiFontInfo12[0];
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
    case THAI_TYPE_16:
        return (ThaiFontInfo*)&_thaiFontInfo16[0];
#endif //USEFONT_16dot
    default:
        return NULL;
    }
}

/** �����Z���֥��`�ɤΥ����å�����ӥ��`�ɉ�Q
  *
  *   @param    nUnicode            (i)���֥��`��(UNICODE)
  *   @param    nD3ChangeOverlay    (i)D3���`�ɉ�Q������TRUE��ED���`�ɡ�FALSE��D2���`�ɣ�
  *
  *   @return   0����   �����Z����ASCII���`��
  *   @return   0       �����Z����Υ��`��
  */
static unsigned short _checkCharTHAI(unsigned short nUnicode, short nD3ChangeOverlay)
{
    if( (nUnicode >= 0x0E01 && nUnicode <= 0x0E3A) || (nUnicode >= 0x0E3F && nUnicode <= 0x0E5B) ){
        if( nUnicode == _D3CODE_THAI ){
            if( nD3ChangeOverlay == TRUE ){
                nUnicode = _EDCODE_THAI;
            }else{
                nUnicode = _D2CODE_THAI;
            }
        }
        return (nUnicode - 0x0D60);
    }else{
        return 0x0;
    }
}

/** �����Z���֥��`�ɤΥ����å���
  * ǰ�I��(E8,E9,EA,EB�Τ����줫�δΤ�ED���Ф�����ϡ����椨��)���󤫥����å�����
  *
  *   @param    pString     (i)���֥��`�ɤؤΥݥ���(UNICODE�Τߌ���)
  *
  *   @return   TRUE    ���椨����
  *   @return   FALSE   ���椨������
  */
static short _checkCharTHAI2(unsigned short *pString)
{
    if( *pString >= 0x0E48 && *pString <= 0x0E4B ){
        switch( *(++pString) ){
        case 0x0E4D:    /* ED���� */
        case 0x0E33:    /* D3����(ǰ�I����ED�ˤʤ뤿��) */
            return TRUE;
        default:
            return FALSE;
        }
    }else{
        return FALSE;
    }
}

/** �ؤͺϤ碌���󥳩`�ɤΥ����å�
  *
  *   @param    nAscii  (i)���֥��`��(ASCII)
  *
  *   @return   TRUE    �ؤͺϤ碌����
  *   @return   FALSE   �ؤͺϤ碌������(�����ʤ�)
  */
static short _checkOverlayCharTHAI(unsigned short nAscii)
{
    if( (nAscii == 0xD1) ||
        (nAscii >= 0xD4 && nAscii <= 0xDA) ||
        (nAscii >= 0xE7 && nAscii <= 0xEE) ){
        return TRUE;
    }else{
        return FALSE;
    }
}

/** �ե���ȥ���`�����ؤͺϤ碌
  *
  *   @param    pDst      (i/o)���ꥸ�ʥ�ե���ȥ���`��
  *   @param    pSrc      (i)�����ե���ȥ���`��
  *   @param    nSize     (i)�Хåե��Х��ȥ�����
  *   @param    nBitOR    (i)�ؤͺϤ碌����(TRUE��OR��FALSE�����ԩ`)
  *
  *   @return   �ʤ�
  */
static void _copyFontDataTHAI(unsigned char *pDst, unsigned char *pSrc, short nSize, short nBitOR)
{
    short i;

    uint32_t CharInNFAddr;
    uint8_t Src[24];
    
    CharInNFAddr = FontLogicAddress + pSrc - _thaiFontInfo12[0].pFontData + FONT_TAIWEN_START;  

    if( nBitOR == FALSE ){
        /* �ե���ȥǩ`�����ԩ` */
        //memcpy( pDst, pSrc, nSize );

        LcdGetResourceData(CharInNFAddr, pDst, FONT_TAIWEN_LEN);        
        
    }else{

        LcdGetResourceData(CharInNFAddr, Src, FONT_TAIWEN_LEN);     
        
        /* �ե���ȥǩ`���ؤͺϤ碌 */
        for( i = 0; i < nSize; i++ ){
            pDst[i] |= Src[i];
        }
    }
}


/* �ؤͺϤ碌�ե���ȷ����x�k�Ʃ`�֥륵�������x */
#define THAI_TYPE1_SELECT_NUM   4
#define THAI_TYPE1_FONT_NUM     13

/* �ؤͺϤ碌�ե���ȷ����x�k�Ʃ`�֥똋���� */
typedef struct {
    unsigned short nCode;   /* �ؤͺϤ碌�������֥��`�� */
    unsigned short nFontNum[THAI_TYPE1_SELECT_NUM]; /* �ե���ȷ��� */
} ThaiFontNumSelect;

/* �ؤͺϤ碌�ե���ȷ����x�k�Ʃ`�֥�
 * ��D1,D4��D7,E7��EE���Ť��ؤͺϤ碌�� */
static const ThaiFontNumSelect _thaiFontNumSelect[THAI_TYPE1_FONT_NUM] = {
    {0xD1, {0x31, 0x60, 0x00, 0x00}},   /* [00] */
    {0xD4, {0x34, 0x61, 0x00, 0x00}},   /* [01] */
    {0xD5, {0x35, 0x62, 0x00, 0x00}},   /* [02] */
    {0xD6, {0x36, 0x63, 0x00, 0x00}},   /* [03] */
    {0xD7, {0x37, 0x64, 0x00, 0x00}},   /* [04] */
    {0xE7, {0x47, 0x65, 0x00, 0x00}},   /* [05] */
    {0xE8, {0x48, 0x68, 0x70, 0x78}},   /* [06] */
    {0xE9, {0x49, 0x69, 0x71, 0x79}},   /* [07] */
    {0xEA, {0x4A, 0x6A, 0x72, 0x7A}},   /* [08] */
    {0xEB, {0x4B, 0x6B, 0x73, 0x7B}},   /* [09] */
    {0xEC, {0x4C, 0x6C, 0x74, 0x7C}},   /* [10] */
    {0xED, {0x4D, 0x6D, 0x00, 0x00}},   /* [11] */
    {0xEE, {0x4E, 0x6E, 0x00, 0x00}}    /* [12] */
};

/** �ؤͺϤ碌�ե���ȷ���ȡ��(D1,D4��D7,E7��EE���Ť��ؤͺϤ碌��)
  *
  *   @param    nCode     (i)����
  *   @param    nL1Code   (i)L1����
  *   @param    nBfCode   (i)�I��g����
  *
  *   @return   0����       �ե���ȷ���
  *   @return   0           �ؤͺϤ碌�ե���Ȥʤ�
  */
static unsigned short _getType1FontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode)
{
    short i;
    unsigned short nFontNum = 0;
    unsigned short nCondition = 0x0;
    const ThaiFontNumSelect *pTable = NULL;

    /* �ե�����x�k�Ʃ`�֥뷬��ȡ�� */
    for( i = 0; i < THAI_TYPE1_FONT_NUM; i++ ){
        if( _thaiFontNumSelect[i].nCode == nCode ){
            pTable = &_thaiFontNumSelect[i];
            break;
        }
    }

    if( pTable == NULL ){
        return 0;
    }

    /* �����������å�
     * ���̣����Ϥ������¤η��ŤΤ����줫�Ǥ���
     *    - 00BB,00BD,00BF,00CB
     */
    switch( nL1Code ){
    case 0xBB:
    case 0xBD:
    case 0xBF:
    case 0xCB:
        nCondition = 0x10;
        break;
    default:
        break;
    }

    /* �����������å�
     * ��һ��ǰ���i������Ť������¤η��ŤΤ����줫�Ǥ���
     *    - 00D1,00D4,00D5,00D6,00D7,00E7,00ED
     */
    switch( nBfCode ){
    case 0xD1:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
    case 0xE7:
    case 0xED:
        nCondition |= 0x01;
        break;
    default:
        break;
    }

    /* �ե���ȷ����x�k */
    switch( nCondition ){
    case 0x00:
        nFontNum = pTable->nFontNum[0];
        break;
    case 0x10:
        nFontNum = pTable->nFontNum[1];
        break;
    case 0x01:
        nFontNum = pTable->nFontNum[2];
        break;
    case 0x11:
        nFontNum = pTable->nFontNum[3];
        break;
    default:
        break;
    }

    return nFontNum;
}

/** �ؤͺϤ碌�ե���ȷ���ȡ��(38��3A���Ť��ؤͺϤ碌��)
  *
  *   @param    nCode     (i)����
  *
  *   @return   0����   �ե���ȷ���
  *   @return   0       �ؤͺϤ碌�ե���Ȥʤ�
  */
static unsigned short _getType2FontNumTHAI(unsigned short nCode)
{
    unsigned short nFontNum = 0;

    /* �ؤͺϤ碌�ե���ȷ����x�k */
    switch( nCode ){
    case 0xD8:
        nFontNum = 0x38;
        break;
    case 0xD9:
        nFontNum = 0x39;
        break;
    case 0xDA:
        nFontNum = 0x3A;
        break;
    default:
        break;
    }

    return nFontNum;
}

/** �ؤͺϤ碌�ե���ȷ���ȡ��
  *
  *   @param    nCode     (i)����
  *   @param    nL1Code   (i)L1����
  *   @param    nBfCode   (i)�I��g����
  *
  *   @return   0����       �ե���ȷ���
  *   @return   0           �ؤͺϤ碌�ե���Ȥʤ�
  */
static short _getAddFontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode)
{
    unsigned short nFontNum = 0;

    /* �ؤͺϤ碌���󤫣� */
    switch( nCode ){
    case 0xD1:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
    case 0xE7:
    case 0xE8:
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
        /* �ե�����x�k�����ף� */
        nFontNum = _getType1FontNumTHAI( nCode, nL1Code, nBfCode );
        break;

    case 0xD8:
    case 0xD9:
    case 0xDA:
        /* �ե�����x�k�����ף� */
        if( nL1Code == 0xAD ){
            nFontNum = 0x8000;
        }else if( nL1Code == 0xB0 ){
            nFontNum = 0x8100;
        }
        nFontNum |= _getType2FontNumTHAI( nCode );
        break;

    default: 
        break;
    }

    return nFontNum;
}

/** �ե���ȥ���`�����ؤͺϤ碌�gʩ
  *
  *   @param    nCode       (i)����
  *   @param    nL1Code     (i)L1����
  *   @param    nBfCode     (i)�I��g����
  *   @param    pFontData   (i)�ե���ȥǩ`���ؤΥݥ���
  *   @param    nDataSize   (i)�ե���ȥѥ��`��ǩ`��������
  *   @param    pImage      (i/o)�ե���ȥѥ��`������Хåե�(NULL�ʤ�������ʤ�)
  *
  *   @return   TRUE        �ؤͺϤ碌�gʩ
  *   @return   FALSE       �ؤͺϤ碌δ�gʩ
  */
static short _execOverlayTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode,
                              unsigned char *pFontData, short nDataSize, unsigned char *pImage)
{
    unsigned short nAddFont = 0;    /* �ؤͺϤ碌�ե���ȷ��� */
    unsigned short nRefFont = 0;    /* �ե���ȥѥ��`������Q���ե���ȷ��� */
    unsigned char *pData = NULL;    /* �ե���ȥǩ`���ݥ��� */

    /* �ؤͺϤ碌���󤫥����å� */
    nAddFont = _getAddFontNumTHAI( nCode, nL1Code, nBfCode );
    if( nAddFont == 0x0 ){
        return FALSE;
    }

    /* �ؤͺϤ碌�gʩ */
    if( pImage ){
        /* �ե���ȥǩ`�������Q�� */
        nRefFont = (nAddFont >> 8) & 0xFF;
        if( nRefFont != 0 ){
            pData = pFontData + nRefFont * nDataSize;
            _copyFontDataTHAI( pImage, pData, nDataSize, FALSE );
            nAddFont &= 0xFF;
        }

        /* �ե���ȥѥ��`���ؤͺϤ碌 */
        pData = pFontData + nAddFont * nDataSize;
        _copyFontDataTHAI( pImage, pData, nDataSize, TRUE );
    }

    return TRUE;
}

/** �����Z�ե���ȥ�������ȡ�ä���
  *
  *   @param    nType       (i)�ե���ȷN�
  *   @param    pnWidth     (o)�ե���ȷ����ե���ȥǩ`����
  *   @param    pnHeight    (o)�ե���ȸߤ����ե���ȥǩ`���ߤ�
  *
  *   @return   TRUE        ȡ�óɹ�
  *   @return   FALSE       ȡ��ʧ��
  */
short getThaiCharSize( short nType, short *pnWidth, short *pnHeight )
{
    ThaiFontInfo *pInfo = _getThaiCharInfo( nType );
    if( pInfo == NULL ){
        *pnWidth  = 0;
        *pnHeight = 0;
        return FALSE;
    }else{
#ifdef THAI_FONT_BYTEUNIT
        *pnWidth  = pInfo->nWidth;
        *pnHeight = pInfo->nHeight;
#else
        *pnWidth  = pInfo->nDataWidth;
        *pnHeight = pInfo->nDataHeight;
#endif
        return TRUE;
    }
}

/** ָ�����֤Υ����Z�ե���ȥѥ��`���ȡ�ä���
  *
  *   @param    nType       (i)�ե���ȷN�
  *   @param    pString     (i)���֥��`�ɤؤΥݥ���(UNICODE�Τߌ���)
  *   @param    pImage      (o)�ե���ȥѥ��`������Хåե�(NULL�ʤ�������ʤ�)
  *   @param    pnWidth     (o)�ե���ȷ����ե���ȥǩ`����
  *   @param    pnHeight    (o)�ե���ȸߤ����ե���ȥǩ`���ߤ�
  *   @param    pnNum       (o)���֥��`���i���z����
  *
  *   @return   THAI_ERR_NONE           0   ����`�ʤ�(�ɹ�)
  *   @return   THAI_ERR_INVALID_PARAM  1   ����������(�����Z������ʤ�)
  *   @return   THAI_ERR_INVALID_GRIF   2   �o���ʥ����(����˥��`�Х쥤���Ť�����)
  */
short getThaiCharImage(short nType, unsigned short* pString, unsigned char *pImage, short *pnWidth, short *pnHeight, short *pnNum)
{
    unsigned short nCode = 0;       /* ����η��� */
    unsigned short nL1Code = 0;     /* L1���� */
    unsigned short nNxCode = 0;     /* �Τη��� */
    unsigned short nBfCode = 0;     /* �I��g���� */
    unsigned short nDataSize = 0;   /* �ե���ȥѥ��`��ǩ`�������� */
    ThaiFontInfo *pInfo = NULL;     /* �ե�������Ʃ`�֥� */
    unsigned char *pFontData = NULL;

    /* �����ǩ`�����ڻ� */
    *pnNum = 0;
    *pnWidth = 0;
    *pnHeight = 0;

    /* �ե�������ȡ�� */
    if( (pInfo = _getThaiCharInfo( nType )) == NULL ){
        return THAI_ERR_INVALID_PARAM;
    }else{
#ifdef THAI_FONT_BYTEUNIT
        *pnWidth  = pInfo->nWidth;
        *pnHeight = pInfo->nHeight;
#else
        *pnWidth  = pInfo->nDataWidth;
        *pnHeight = pInfo->nDataHeight;
#endif
    }

    /* ����η����i�z�ߡ����Ź�������å������`�ɉ�Q(ǰ�I��) */
    if( (nCode = _checkCharTHAI( *pString, FALSE )) == 0x0 ){
        return THAI_ERR_INVALID_PARAM;
    }

    /* �i���z������Ť�������ڻ� */
    *pnNum = 1;

    /* �ե���ȥѥ��`��ǩ`�������� */
    nDataSize = ROUNDUP((pInfo->nDataWidth) * (pInfo->nDataHeight));

    /* ���`�Щ`�쥤���Ť�����������������å�
     * ������˥��`�Щ`�쥤���Ť��������ϤϤ��η��Ťϟoҕ����(�Τ�����ʤ�) */
    if( _checkOverlayCharTHAI( nCode ) == TRUE ){
        /* *pnWidth = 0;*/
        return THAI_ERR_INVALID_GRIF;
    }

    /* L1���ťե���ȥǩ`���i���z�� */
    if( pImage ){
        pFontData = pInfo->pFontData + ((nCode - 0xA0) * nDataSize);
        _copyFontDataTHAI( pImage, pFontData, nDataSize, FALSE );
    }
    
    /* L1���ŤȤ���ӛ�� */
    nL1Code = nBfCode = nCode;

    /*
     * �ե���ȥǩ`�����ؤͺϤ碌
     */
    while(1){
        /* һ�����i���z�� */
        nNxCode = *(pString + *pnNum);

        /* �ǩ`���νK��ꣿ */
        if( nNxCode == 0x0 ){
            goto Exit;
        }

        /* ���Ź�������å������`�ɉ�Q(ǰ�I��) */
        if( (nNxCode = _checkCharTHAI( nNxCode, TRUE )) == 0x0 ){
            goto Exit;
        }

        /* ED���ŤȤ����椨�I����Ҫ�������å�(ǰ�I��) */
        if( _checkCharTHAI2( (pString + *pnNum) ) == TRUE ){
            /* ED���ŤȤ��ؤͺϤ��gʩ */
            if( _execOverlayTHAI( 0xED, nL1Code, nBfCode, pInfo->pFontData, nDataSize, pImage ) == FALSE ){
            goto Exit;
        }
            /* �I��g�߷��Ÿ��� */
            nBfCode = 0xED;
            }

        /* �ؤͺϤ碌�gʩ */
        if( _execOverlayTHAI( nNxCode, nL1Code, nBfCode, pInfo->pFontData, nDataSize, pImage ) == FALSE ){
            goto Exit;
        }

        /* �I��g�߷��Ÿ��� */
        nBfCode = nNxCode;

        /* D3���Ť��ä���K��
         * ��D3��Q������ζ���Ŀ��L1���ϤȤʤ뤿�ᡢ�ؤͺϤ碌�Ϥ��ꤨ�ʤ� */
        if( *(pString + *pnNum) == _D3CODE_THAI ){
            goto Exit;
        }

        /* �i���z������Ť����򥤥󥯥���� */
        *pnNum = *pnNum + 1;
    }

Exit:
    return THAI_ERR_NONE;
}

/** ָ�����֤��饿���Z�����֤����ɤ���Τ˱�Ҫ�����֥��`������ȡ�ä���
  *
  *   @param    nType       (i)�ե���ȷN�
  *   @param    pString     (i)���֥��`�ɤؤΥݥ���(UNICODE�Τߌ���)
  *   @param    pnWidth     (o)�ե���ȷ����ե���ȥǩ`����
  *   @param    pnHeight    (o)�ե���ȸߤ����ե���ȥǩ`���ߤ�
  *   @param    pnNum       (o)���֥��`���i���z����
  *
  *   @return   THAI_ERR_NONE           0   ����`�ʤ�(�ɹ�)
  *   @return   THAI_ERR_INVALID_PARAM  1   ����������(�����Z������ʤ�)
  *   @return   THAI_ERR_INVALID_GRIF   2   �o���ʥ����(����˥��`�Х쥤���Ť�����)
  */
short getThaiCharNum(short nType, unsigned short* pString, short *pnWidth, short *pnHeight, short *pnNum)
{
    short nRet = getThaiCharImage(nType, pString, NULL, pnWidth, pnHeight, pnNum);
    return nRet;
}

#ifdef __cplusplus
};
#endif  /* __cplusplus */
