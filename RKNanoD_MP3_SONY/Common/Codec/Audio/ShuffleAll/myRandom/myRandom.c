/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\all\Audio\myRandom.c
* Owner: aaron.sun
* Date: 2016.6.22
* Time: 11:17:02
* Version: 1.0
* Desc: shuffle
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.6.22     11:17:02   1.0
********************************************************************************************
*/

#define __APP_ALL_AUDIO_MYRANDOM_C__
#ifdef __APP_ALL_AUDIO_MYRANDOM_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "sysinclude.h"
#include "myRandom.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define file_num_bound  8192


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct
{
    uint16 buf_index;              //当前正在播的序号所在的buffer偏移
    uint16 seed_buf[file_num_bound];  //总buffer
    uint16 total_num;              //文件总数
    uint16 prev_index;            //随机序列的上一首，主要用于连续寻找上一首
} seed_buf_info;

_APP_ALL_AUDIO_MYRANDOM_COMMON_ seed_buf_info rand_seed  ;
_APP_ALL_AUDIO_MYRANDOM_COMMON_ uint32  invert_flag = 0;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint32 RandomGetNext(uint32 seed);
uint32 RandomGetPrevious(uint32 seed);
void InitGlobeVar(uint32 num, uint32 seed);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: CreateRandomList
** Input:uint32 num, uint32 seed
** Return: void
** Owner:aaron.sun
** Date: 2016.6.22
** Time: 11:24:15
*******************************************************************************/
_APP_ALL_AUDIO_MYRANDOM_COMMON_
COMMON API void CreateRandomList(uint32 num, uint32 seed)
{
    invert_flag = 0;
    InitGlobeVar(num, seed);
}

/*******************************************************************************
** Name: randomGenerator
** Input:DIRECTION_TYPE direction, uint32 seed
** Return: uint32
** Owner:aaron.sun
** Date: 2016.6.22
** Time: 11:23:22
*******************************************************************************/
_APP_ALL_AUDIO_MYRANDOM_COMMON_
COMMON API uint32 randomGenerator(DIRECTION_TYPE direction, uint32 seed)
{
    seed_buf_info *si = &rand_seed;
    DIRECTION_TYPE real_direction;

    if(invert_flag)
    {
        if(direction == RANDOM_PREVIOUS)
        {
            real_direction = RANDOM_NEXT;
        }
        else if(direction == RANDOM_NEXT)
        {
            real_direction = RANDOM_PREVIOUS;
        }
        else
        {
            real_direction = direction;
        }
    }
    else
    {
        real_direction = direction;
    }

    switch (real_direction)
    {
        case RANDOM_PREVIOUS:
        {
            if(si->prev_index == 0)
            {
                InitGlobeVar(si->total_num, si->total_num);
                if((invert_flag && (direction == RANDOM_NEXT)) || ((invert_flag == 0) && (direction == RANDOM_PREVIOUS)))
                {
                    invert_flag = 1 - invert_flag;
                }
                seed = si->seed_buf[si->buf_index - 1];
            }
            else
            {
                seed = RandomGetPrevious(seed);
            }
            break;
        }

        case RANDOM_CUR:
        {
            return si->seed_buf[si->buf_index - 1];
        }

        case RANDOM_NEXT:
        {
            if(si->buf_index == si->total_num)
            {
               InitGlobeVar(si->total_num, si->total_num);
               seed = si->seed_buf[si->buf_index - 1];
            }
            else
            {
                seed = RandomGetNext(seed);
            }
            break;
        }
        default:
            break;
    }
    return (seed);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RandomGetNext
** Input:uint32 seed
** Return: uint32
** Owner:aaron.sun
** Date: 2016.6.22
** Time: 11:22:38
*******************************************************************************/
_APP_ALL_AUDIO_MYRANDOM_COMMON_
COMMON FUN uint32 RandomGetNext(uint32 seed)
{
    uint32 temp;
    seed_buf_info *si = &rand_seed;

    if (si->prev_index < (si->buf_index - 1)) //连续向前搜索之后向后搜索
    {
        // printf("下 prev %d\n",si->prev_index);
        si->prev_index ++;
        temp = si->seed_buf[si->prev_index] ;
    }
    else                                    //连续向后搜索
    {
        seed = seed % (si->total_num - si->buf_index); // 剩余未播放个数
        seed += si->buf_index;                        //剩余未播放的索引起始

        //printf(">>>>si->buf_index = %d.\n", si->buf_index);

        temp = si->seed_buf[seed];
        si->seed_buf[seed] = si->seed_buf[si->buf_index] ;
        si->seed_buf[si->buf_index] = temp;
        si->buf_index ++;
        si->prev_index = si->buf_index - 1;
    }

    return  temp;
}

/*******************************************************************************
** Name: RandomGetPrevious
** Input:uint32 seed
** Return: uint32
** Owner:aaron.sun
** Date: 2016.6.22
** Time: 11:21:53
*******************************************************************************/
_APP_ALL_AUDIO_MYRANDOM_COMMON_
COMMON FUN uint32 RandomGetPrevious(uint32 seed)
{
    uint32 temp;
    seed_buf_info *si = &rand_seed;
    si->prev_index -= 1;                   //连续向前搜索后在一个序列内向后搜索
    {
        return si->seed_buf[si->prev_index];
    }
}

/*******************************************************************************
** Name: InitGlobeVar
** Input:uint32 num, uint32 seed
** Return: void
** Owner:aaron.sun
** Date: 2016.6.22
** Time: 11:20:07
*******************************************************************************/
_APP_ALL_AUDIO_MYRANDOM_COMMON_
COMMON FUN void InitGlobeVar(uint32 num, uint32 seed)
{

    uint32 i;
    seed_buf_info *si = &rand_seed;
    uint32 temp, temp1, temp2;

    if(seed >= num)
    {
        temp1 = si->seed_buf[0];
        temp2 = si->seed_buf[num - 1];
    }


    if(num > file_num_bound)
    {
        num = file_num_bound;
    }

    for (i = 0; i < num; i++)
    {
        si->seed_buf[i] = i;
    }

    si->buf_index = 0;
    si->prev_index = 0;
    si->total_num = num;

    if(seed >= num)
    {
        if(num == 1)
        {
            seed = 0;
        }
        else if(num == 2)
        {
            seed = 1 - temp2;
        }
        else
        {
            do
            {
                seed = SysTickCounter % num;
            }
            while((seed == temp1) || (seed == temp2));
        }
    }

    temp = si->seed_buf[seed];
    si->seed_buf[seed] = si->seed_buf[si->buf_index] ;
    si->seed_buf[si->buf_index] = temp;
    si->buf_index ++;
    si->prev_index = si->buf_index - 1;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#endif


