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
*           HuWeiGuo     07/09/27      2.0          �޸��˽ӿں���      
*
*******************************************************************************/

#ifndef __EQ_H__
#define __EQ_H__

void RockEQAdjust(long SmpRate, short *g, short db); // db ΪԤ��˥����db ��, 0: ˥��0dB; 1:˥��6dB; 2:˥��12dB;
void RockEQProcess(short *pData, long PcmLen);
#endif
