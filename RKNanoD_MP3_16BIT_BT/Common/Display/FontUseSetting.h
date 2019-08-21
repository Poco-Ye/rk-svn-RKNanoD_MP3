#ifndef __Font_USE_SETTING_H__
#define __Font_USE_SETTING_H__
/* ==========================================================================

	Copyright (c) 2011-2012 Sony Corporation.
	All Rights Reserved. Proprietary and Confidential.

========================================================================== */
/*!
	@brief
		Declaration of Font Library
	@author
		Masaya Kinoshita
	@note
		セットで使用するフォントサイズを定義する
*/


/********************************************************/
/*【注意点】                                              */
/*   - セットでつかうフォントのみdefineすること!               */
/*   - 少なくとも12 or 16どちらかがdefineされていること        */
/*      (buildエラーになるように仕込んでいます。)              */
/*  -  defineに合わせてmakefileを変更すること。              */
/********************************************************/

#define USEFONT_12dot  //12dot fontを使うときは宣言
//#define USEFONT_16dot  //16dot fontを使うときは宣言

#endif //__Font_USE_SETTING_H__
