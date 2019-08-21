/* ==========================================================================
	
	Copyright (c) 2013 Sony Corporation.
	All Rights Reserved. Proprietary and Confidential.
	
========================================================================== */

/*
 * �����Z�ե���ȥǩ`��ȡ��API
 *     [����Ěs]
 *      - 2012/3/5 12dot/16dot�Υ��`�ɤ�ީ`��
 *                                                   M. Kinoshita
 */

#ifndef __GETTHAICHAR_H__
#define __GETTHAICHAR_H__

#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus */

/*
 * �����Z�ե���ȥ饤�֥�ꥳ��ѥ��륪�ץ����
 */
#define THAI_FONT_BYTEUNIT

/*
 * �����Z�ե���ȥ����פζ��x
 */
#define THAI_TYPE_12    0   /* �̶��ԥå�12pt�ե����(�ߤ���12dot) */
#define THAI_TYPE_16    1   /* �̶��ԥå�16pt�ե����(�ߤ���16dot) */

/*
 * �����Z�ե���ȥ��������x
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
#define THAI_ERR_NONE           0    /* ����`�ʤ�(�ɹ�) */
#define THAI_ERR_INVALID_PARAM  1    /* ����������(�����Z������ʤ�) */
#define THAI_ERR_INVALID_GRIF   2    /* �o���ʥ����(����˥��`�Х쥤���Ť�����) */


/*
 * �����Z�ե���ȥǩ`��ȡ��API
 */
short getThaiCharSize(short nType, short *pnWidth, short *pnHeight);
short getThaiCharImage(short nType, unsigned short* pString, unsigned char *pImage, short *pnWidth, short *pnHeight, short *pnNum);
short getThaiCharNum(short nType, unsigned short* pString, short *pnWidth, short *pnHeight, short *pnNum);

#ifdef __cplusplus
};
#endif  /* __cplusplus */

#endif  /* __GETTHAICHAR_H__ */


