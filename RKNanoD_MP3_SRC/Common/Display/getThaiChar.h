/* ==========================================================================
	
	Copyright (c) 2013 Sony Corporation.
	All Rights Reserved. Proprietary and Confidential.
	
========================================================================== */

/*
 * タイ語フォントデータ取得API
 *     [変更履歴]
 *      - 2012/3/5 12dot/16dotのコードをマージ
 *                                                   M. Kinoshita
 */

#ifndef __GETTHAICHAR_H__
#define __GETTHAICHAR_H__

#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus */

/*
 * タイ語フォントライブラリコンパイルオプション
 */
#define THAI_FONT_BYTEUNIT

/*
 * タイ語フォントタイプの定義
 */
#define THAI_TYPE_12    0   /* 固定ピッチ12ptフォント(高さが12dot) */
#define THAI_TYPE_16    1   /* 固定ピッチ16ptフォント(高さが16dot) */

/*
 * タイ語フォントサイズ定義
 */
#define THAI_TYPE_12_WIDTH          9
#define THAI_TYPE_12_HEIGHT         12
#define THAI_TYPE_12_DATA_WIDTH     16
#define THAI_TYPE_12_DATA_HEIGHT    12

#define THAI_TYPE_16_WIDTH          9
#define THAI_TYPE_16_HEIGHT         16
#define THAI_TYPE_16_DATA_WIDTH     16
#define THAI_TYPE_16_DATA_HEIGHT    16

/*
 * Error Code
 */
#define THAI_ERR_NONE           0    /* エラーなし(成功) */
#define THAI_ERR_INVALID_PARAM  1    /* 不正な引数(タイ語範囲外など) */
#define THAI_ERR_INVALID_GRIF   2    /* 無効なグリフ(最初にオーバレイ符号が来た) */


/*
 * タイ語フォントデータ取得API
 */
short getThaiCharSize(short nType, short *pnWidth, short *pnHeight);
short getThaiCharImage(short nType, unsigned short* pString, unsigned char *pImage, short *pnWidth, short *pnHeight, short *pnNum);
short getThaiCharNum(short nType, unsigned short* pString, short *pnWidth, short *pnHeight, short *pnNum);

#ifdef __cplusplus
};
#endif  /* __cplusplus */

#endif  /* __GETTHAICHAR_H__ */


