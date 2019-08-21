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
 *                  重ね合せフォント番号取得時の条件チェック部を変更
 *                2008/03/07   modified by Sony
 *                  getThaiCharNum関数追加
 *                2008/03/17   modified by Sony
 *                  フォント情報取得箇所は全てフォントデータの値を返すよう変更
 *                2008/03/20   modified by Sony
 *                  1.複数サイズのフォントに対応
 *                  2.幅方向のデータをバイト単位で運用する場合とそうでない場合を
 *                    コンパイルオプションで切り換えるようにした
 *                  3.関数ヘッダコメントを修正
 *                2008/05/15   modified by Sony
 *                  タイ語表示仕様書 Ver.1.0リリースに伴い、以下の修正を実施
 *                  1.前処理２(E8,E9,EA,EBのいずれかの次にEDを有する場合入替える)
 *                    追加対応
 *                  2.フォント選択条件２変更(一つ前の読んだ符号の条件にEDが追加)、
 *                    フォント番号選択表変更に対応
 *                2012/3/5  modified by M. Kinoshita (Sony)
 *                  1. 12/16dot対応をマージ
 */

#include <string.h>
#include "FontUseSetting.h"
#include "getThaiChar.h"
#include "stdint.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* D3符号情報 */
#define _D3CODE_THAI    0x0E33  /* D3符号(UNICODE) */
#define _EDCODE_THAI    0x0E4D  /* D3符号変換コード1(UNICODE) */
#define _D2CODE_THAI    0x0E32  /* D3符号変換コード2(UNICODE) */

/* バイト単位に切り上げ */
#define ROUNDUP(x)  (((x)+7)/8)

/* タイ語フォントデータ */
#ifdef USEFONT_12dot
extern unsigned char FontAdr_THAI12[];
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
extern unsigned char FontAdr_THAI16[];
#endif //USEFONT_16dot

/* タイ語フォントデータ情報テーブル構造体 */
typedef struct {
    short   nType;              /* フォントタイプ */
    short   nWidth;             /* フォント幅 */
    short   nHeight;            /* フォント高さ */
    short   nDataWidth;         /* フォントデータ幅 */
    short   nDataHeight;        /* フォントデータ高さ */
    unsigned char *pFontData;	/* フォントデータへのポインタ */
} ThaiFontInfo;

/* タイ語フォントデータ情報テーブル */
#ifdef USEFONT_12dot
static const ThaiFontInfo _thaiFontInfo12[] = {
    {THAI_TYPE_12,
THAI_TYPE_12_WIDTH, 
THAI_TYPE_12_HEIGHT, 
THAI_TYPE_12_DATA_WIDTH, 
THAI_TYPE_12_DATA_HEIGHT, 
FontAdr_THAI12}  /* 12ptフォント */
};
#endif //USEFONT_12dot

#ifdef USEFONT_16dot
static const ThaiFontInfo _thaiFontInfo16[] = {
    {THAI_TYPE_16,
THAI_TYPE_16_WIDTH, 
THAI_TYPE_16_HEIGHT, 
THAI_TYPE_16_DATA_WIDTH, 
THAI_TYPE_16_DATA_HEIGHT, 
FontAdr_THAI16}  /* 16ptフォント */
};
#endif //USEFONT_16dot

/* ローカル関数プロトタイプ宣言 */
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

/** タイ語フォント情報を取得する
  *
  *   @param    nType       (i)フォント種類
  *
  *   @return   NULL以外    タイ語フォント情報テーブルアドレス
  *   @return   NULL        取得失敗
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

/** タイ語文字コードのチェックおよびコード変換
  *
  *   @param    nUnicode            (i)文字コード(UNICODE)
  *   @param    nD3ChangeOverlay    (i)D3コード変換動作（TRUE：EDコード、FALSE：D2コード）
  *
  *   @return   0以外   タイ語文字ASCIIコード
  *   @return   0       タイ語以外のコード
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

/** タイ語文字コードのチェック２
  * 前処理２(E8,E9,EA,EBのいずれかの次にEDを有する場合、入替える)対象かチェックする
  *
  *   @param    pString     (i)文字コードへのポインタ(UNICODEのみ対応)
  *
  *   @return   TRUE    入替え対象
  *   @return   FALSE   入替え対象外
  */
static short _checkCharTHAI2(unsigned short *pString)
{
    if( *pString >= 0x0E48 && *pString <= 0x0E4B ){
        switch( *(++pString) ){
        case 0x0E4D:    /* ED符号 */
        case 0x0E33:    /* D3符合(前処理１でEDになるため) */
            return TRUE;
        default:
            return FALSE;
        }
    }else{
        return FALSE;
    }
}

/** 重ね合わせ対象コードのチェック
  *
  *   @param    nAscii  (i)文字コード(ASCII)
  *
  *   @return   TRUE    重ね合わせ対象
  *   @return   FALSE   重ね合わせ対象外(子音など)
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

/** フォントイメージの重ね合わせ
  *
  *   @param    pDst      (i/o)オリジナルフォントイメージ
  *   @param    pSrc      (i)入力フォントイメージ
  *   @param    nSize     (i)バッファバイトサイズ
  *   @param    nBitOR    (i)重ね合わせ方法(TRUE：OR、FALSE：コピー)
  *
  *   @return   なし
  */
static void _copyFontDataTHAI(unsigned char *pDst, unsigned char *pSrc, short nSize, short nBitOR)
{
    short i;

    uint32_t CharInNFAddr;
    uint8_t Src[24];
    
    CharInNFAddr = FontLogicAddress + pSrc - _thaiFontInfo12[0].pFontData + FONT_TAIWEN_START;  

    if( nBitOR == FALSE ){
        /* フォントデータコピー */
        //memcpy( pDst, pSrc, nSize );

        LcdGetResourceData(CharInNFAddr, pDst, FONT_TAIWEN_LEN);        
        
    }else{

        LcdGetResourceData(CharInNFAddr, Src, FONT_TAIWEN_LEN);     
        
        /* フォントデータ重ね合わせ */
        for( i = 0; i < nSize; i++ ){
            pDst[i] |= Src[i];
        }
    }
}


/* 重ね合わせフォント番号選択テーブルサイズ定義 */
#define THAI_TYPE1_SELECT_NUM   4
#define THAI_TYPE1_FONT_NUM     13

/* 重ね合わせフォント番号選択テーブル構造体 */
typedef struct {
    unsigned short nCode;   /* 重ね合わせ対象文字コード */
    unsigned short nFontNum[THAI_TYPE1_SELECT_NUM]; /* フォント番号 */
} ThaiFontNumSelect;

/* 重ね合わせフォント番号選択テーブル
 * ※D1,D4～D7,E7～EE符号の重ね合わせ用 */
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

/** 重ね合わせフォント番号取得(D1,D4～D7,E7～EE符号の重ね合わせ用)
  *
  *   @param    nCode     (i)符号
  *   @param    nL1Code   (i)L1符合
  *   @param    nBfCode   (i)処理済符号
  *
  *   @return   0以外       フォント番号
  *   @return   0           重ね合わせフォントなし
  */
static unsigned short _getType1FontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode)
{
    short i;
    unsigned short nFontNum = 0;
    unsigned short nCondition = 0x0;
    const ThaiFontNumSelect *pTable = NULL;

    /* フォント選択テーブル番号取得 */
    for( i = 0; i < THAI_TYPE1_FONT_NUM; i++ ){
        if( _thaiFontNumSelect[i].nCode == nCode ){
            pTable = &_thaiFontNumSelect[i];
            break;
        }
    }

    if( pTable == NULL ){
        return 0;
    }

    /* 条件１チェック
     * ※Ｌ１符合が、以下の符号のいずれかである
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

    /* 条件２チェック
     * ※一つ前に読んだ符号が、以下の符号のいずれかである
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

    /* フォント番号選択 */
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

/** 重ね合わせフォント番号取得(38～3A符号の重ね合わせ用)
  *
  *   @param    nCode     (i)符号
  *
  *   @return   0以外   フォント番号
  *   @return   0       重ね合わせフォントなし
  */
static unsigned short _getType2FontNumTHAI(unsigned short nCode)
{
    unsigned short nFontNum = 0;

    /* 重ね合わせフォント番号選択 */
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

/** 重ね合わせフォント番号取得
  *
  *   @param    nCode     (i)符号
  *   @param    nL1Code   (i)L1符合
  *   @param    nBfCode   (i)処理済符号
  *
  *   @return   0以外       フォント番号
  *   @return   0           重ね合わせフォントなし
  */
static short _getAddFontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode)
{
    unsigned short nFontNum = 0;

    /* 重ね合わせ対象か？ */
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
        /* フォント選択タイプ１ */
        nFontNum = _getType1FontNumTHAI( nCode, nL1Code, nBfCode );
        break;

    case 0xD8:
    case 0xD9:
    case 0xDA:
        /* フォント選択タイプ２ */
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

/** フォントイメージの重ね合わせ実施
  *
  *   @param    nCode       (i)符号
  *   @param    nL1Code     (i)L1符合
  *   @param    nBfCode     (i)処理済符号
  *   @param    pFontData   (i)フォントデータへのポインタ
  *   @param    nDataSize   (i)フォントパターンデータサイズ
  *   @param    pImage      (i/o)フォントパターン出力バッファ(NULLなら出力しない)
  *
  *   @return   TRUE        重ね合わせ実施
  *   @return   FALSE       重ね合わせ未実施
  */
static short _execOverlayTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode,
                              unsigned char *pFontData, short nDataSize, unsigned char *pImage)
{
    unsigned short nAddFont = 0;    /* 重ね合わせフォント番号 */
    unsigned short nRefFont = 0;    /* フォントパターン書き換えフォント番号 */
    unsigned char *pData = NULL;    /* フォントデータポインタ */

    /* 重ね合わせ対象かチェック */
    nAddFont = _getAddFontNumTHAI( nCode, nL1Code, nBfCode );
    if( nAddFont == 0x0 ){
        return FALSE;
    }

    /* 重ね合わせ実施 */
    if( pImage ){
        /* フォントデータ書き換え */
        nRefFont = (nAddFont >> 8) & 0xFF;
        if( nRefFont != 0 ){
            pData = pFontData + nRefFont * nDataSize;
            _copyFontDataTHAI( pImage, pData, nDataSize, FALSE );
            nAddFont &= 0xFF;
        }

        /* フォントパターン重ね合わせ */
        pData = pFontData + nAddFont * nDataSize;
        _copyFontDataTHAI( pImage, pData, nDataSize, TRUE );
    }

    return TRUE;
}

/** タイ語フォントサイズを取得する
  *
  *   @param    nType       (i)フォント種類
  *   @param    pnWidth     (o)フォント幅／フォントデータ幅
  *   @param    pnHeight    (o)フォント高さ／フォントデータ高さ
  *
  *   @return   TRUE        取得成功
  *   @return   FALSE       取得失敗
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

/** 指定文字のタイ語フォントパターンを取得する
  *
  *   @param    nType       (i)フォント種類
  *   @param    pString     (i)文字コードへのポインタ(UNICODEのみ対応)
  *   @param    pImage      (o)フォントパターン出力バッファ(NULLなら出力しない)
  *   @param    pnWidth     (o)フォント幅／フォントデータ幅
  *   @param    pnHeight    (o)フォント高さ／フォントデータ高さ
  *   @param    pnNum       (o)文字コード読み込み数
  *
  *   @return   THAI_ERR_NONE           0   エラーなし(成功)
  *   @return   THAI_ERR_INVALID_PARAM  1   不正な引数(タイ語範囲外など)
  *   @return   THAI_ERR_INVALID_GRIF   2   無効なグリフ(最初にオーバレイ符号が来た)
  */
short getThaiCharImage(short nType, unsigned short* pString, unsigned char *pImage, short *pnWidth, short *pnHeight, short *pnNum)
{
    unsigned short nCode = 0;       /* 最初の符号 */
    unsigned short nL1Code = 0;     /* L1符号 */
    unsigned short nNxCode = 0;     /* 次の符号 */
    unsigned short nBfCode = 0;     /* 処理済符号 */
    unsigned short nDataSize = 0;   /* フォントパターンデータサイズ */
    ThaiFontInfo *pInfo = NULL;     /* フォント情報テーブル */
    unsigned char *pFontData = NULL;

    /* 出力データ初期化 */
    *pnNum = 0;
    *pnWidth = 0;
    *pnHeight = 0;

    /* フォント情報取得 */
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

    /* 最初の符号読込み、符号範囲チェック＆コード変換(前処理１) */
    if( (nCode = _checkCharTHAI( *pString, FALSE )) == 0x0 ){
        return THAI_ERR_INVALID_PARAM;
    }

    /* 読み込んだ符号の数を初期化 */
    *pnNum = 1;

    /* フォントパターンデータサイズ */
    nDataSize = ROUNDUP((pInfo->nDataWidth) * (pInfo->nDataHeight));

    /* オーバーレイ符号が最初に来たかチェック
     * ※最初にオーバーレイ符号が来た場合はその符号は無視する(何も書かない) */
    if( _checkOverlayCharTHAI( nCode ) == TRUE ){
        /* *pnWidth = 0;*/
        return THAI_ERR_INVALID_GRIF;
    }

    /* L1符号フォントデータ読み込み */
    if( pImage ){
        pFontData = pInfo->pFontData + ((nCode - 0xA0) * nDataSize);
        _copyFontDataTHAI( pImage, pFontData, nDataSize, FALSE );
    }
    
    /* L1符号として記憶 */
    nL1Code = nBfCode = nCode;

    /*
     * フォントデータの重ね合わせ
     */
    while(1){
        /* 一符合読み込み */
        nNxCode = *(pString + *pnNum);

        /* データの終わり？ */
        if( nNxCode == 0x0 ){
            goto Exit;
        }

        /* 符号範囲チェック＆コード変換(前処理１) */
        if( (nNxCode = _checkCharTHAI( nNxCode, TRUE )) == 0x0 ){
            goto Exit;
        }

        /* ED符号との入替え処理が必要かチェック(前処理２) */
        if( _checkCharTHAI2( (pString + *pnNum) ) == TRUE ){
            /* ED符号との重ね合せ実施 */
            if( _execOverlayTHAI( 0xED, nL1Code, nBfCode, pInfo->pFontData, nDataSize, pImage ) == FALSE ){
            goto Exit;
        }
            /* 処理済み符号更新 */
            nBfCode = 0xED;
            }

        /* 重ね合わせ実施 */
        if( _execOverlayTHAI( nNxCode, nL1Code, nBfCode, pInfo->pFontData, nDataSize, pImage ) == FALSE ){
            goto Exit;
        }

        /* 処理済み符号更新 */
        nBfCode = nNxCode;

        /* D3符号だったら終了
         * ※D3変換した後の二つ目はL1符合となるため、重ね合わせはありえない */
        if( *(pString + *pnNum) == _D3CODE_THAI ){
            goto Exit;
        }

        /* 読み込んだ符号の数をインクリメント */
        *pnNum = *pnNum + 1;
    }

Exit:
    return THAI_ERR_NONE;
}

/** 指定文字からタイ語１文字を生成するのに必要な文字コード数を取得する
  *
  *   @param    nType       (i)フォント種類
  *   @param    pString     (i)文字コードへのポインタ(UNICODEのみ対応)
  *   @param    pnWidth     (o)フォント幅／フォントデータ幅
  *   @param    pnHeight    (o)フォント高さ／フォントデータ高さ
  *   @param    pnNum       (o)文字コード読み込み数
  *
  *   @return   THAI_ERR_NONE           0   エラーなし(成功)
  *   @return   THAI_ERR_INVALID_PARAM  1   不正な引数(タイ語範囲外など)
  *   @return   THAI_ERR_INVALID_GRIF   2   無効なグリフ(最初にオーバレイ符号が来た)
  */
short getThaiCharNum(short nType, unsigned short* pString, short *pnWidth, short *pnHeight, short *pnNum)
{
    short nRet = getThaiCharImage(nType, pString, NULL, pnWidth, pnHeight, pnNum);
    return nRet;
}

#ifdef __cplusplus
};
#endif  /* __cplusplus */
