/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     rk_eq.h
*  Description:
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     06/12/30      1.0
*           HuWeiGuo     07/09/27      2.0          修改了接口函数
*
*******************************************************************************/

#ifndef __EQ_H__
#define __EQ_H__

typedef struct
{
    short x[8] ;
    long long y[8] ;
    int _al[4];
    int _bl[6];
    int _ar[4];
    int _br[6];
    int _as[4];
    int _bs[6];
    int ls[2];
    int rs[2];
    int l[2];
    int r[2];
    int max_DbGain[2];
    int EqAjustOk[2];

    // 加密控制参数, 为1 才能正确处理，其它会产生错误

} IIRFilterState_BASS;

typedef struct
{                               // address      bytes
    long  temp_mem;             // 0x00          4    no
    long  i_32buff[20];         // 0x04          4*20
    long  i_32sclaedX;          // 0x54          4    no
    long  PassBandDetect;       // 0x58          4
    long  After_Scale;          // 0x5C          4  标志factored_B是否超过13bit
    short _al[10];               // 0x60          2*10
    short _bl[10];               // 0x74          2*10
    short _ar[10];               // 0x88
    short _br[10];               // 0x9c
    short factored_BL;           // 0x88          2
    short factored_BR;           // 0x88          2
    short EQ_ps_factor;         // 0x8A          2

    // 加密控制参数, 为1 才能正确处理，其它会产生错误
    short fReal;

    long mode[2];
    short _as[10];               // 0x60          2*10
    short _bs[10];               // 0x74          2*10
    short factored_BS;           // 0x88          2
    long EqAjustOk[2];
    int ls[2];
    int rs[2];
    int l[2];
    int r[2];

} IIRFilterState;

extern IIRFilterState_BASS g_bass_FilterState;
extern IIRFilterState g_FilterState;

void RockEQAdjust(long SmpRate, short *g, short db); // db 为预先衰减的db 数, 0: 衰减0dB; 1:衰减6dB; 2:衰减12dB;
void RockEQProcess(short *pData, long PcmLen);
#endif
