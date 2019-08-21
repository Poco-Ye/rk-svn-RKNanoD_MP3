/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\all\Audio\myRandom.h
* Owner: aaron.sun
* Date: 2016.6.22
* Time: 11:17:36
* Version: 1.0
* Desc: shuffle
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.6.22     11:17:36   1.0
********************************************************************************************
*/


#ifndef __APP_ALL_AUDIO_MYRANDOM_H__
#define __APP_ALL_AUDIO_MYRANDOM_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _APP_ALL_AUDIO_MYRANDOM_COMMON_  __attribute__((section("app_all_audio_myrandom_common")))
#define _APP_ALL_AUDIO_MYRANDOM_INIT_  __attribute__((section("app_all_audio_myrandom_init")))
#define _APP_ALL_AUDIO_MYRANDOM_SHELL_  __attribute__((section("app_all_audio_myrandom_shell")))

typedef enum
{
    RANDOM_PREVIOUS = -1,
    RANDOM_CUR = 0,
    RANDOM_NEXT = 1
}DIRECTION_TYPE;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void CreateRandomList(uint32 num, uint32 seed);
extern uint32 randomGenerator(DIRECTION_TYPE direction, uint32 seed);
#endif