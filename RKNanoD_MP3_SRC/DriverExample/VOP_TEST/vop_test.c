
#include "SysInclude.h"

#ifdef _VOP_TEST_

#include "Device.h"	
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "LcdInterface.h"
#include "vop.h"
#include "yuv2rgb.h"
#include "Hw_vop.h"
#include "dma.h"
//#include "hw_160_128_uvy_testdata.h"
#include "hw_160_128_rgb565_testdata.h"
#include "hw_160_128_yuv_testdata.h"

/*******************************************************************************
** Name: hw_yuv2rgb_shell
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.12
** Time: 14:23:26
*******************************************************************************/

#if(LCD_WIDTH == 160 ||(LCD_WIDTH == 128))
SHELL API rk_err_t hw_yuv2rgb_shell(void)
{
    uint32 i,j;
    uint32 IntStatus;
    int frm_cnt = 0;
    int Tick  = 0,Time;
	//  long long SysTick;
    uint64 loadcount = 80*1000;
    int timer_time = 0;
    /*IntRegister(INT_ID_VOP ,YuvIsr);
    IntPendingClear(INT_ID_VOP);
    IntEnable(INT_ID_VOP);*/
    ScuClockGateCtr( HCLK_LCDC_GATE, 1);
    ScuSoftResetCtr(LCDC_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(LCDC_SRST, 0); 
    ST7735_Init();
    ST7735_SetWindow(0,0,VOP_WIDTH-1, VOP_HEIGNT-1);
    for(i = 0; i < VOP_HEIGNT; i++)
    {
        for(j = 0; j < VOP_WIDTH; j++)
        {
            VopSendData(0, 0);
        }
    }
    VopEnableInt(0, VOP_INT_TYPE_FRAME_DONE);
   
    VopIntClear(0);	  
  	ST7735_SetWindow(0, 0, VOP_WIDTH-1, VOP_HEIGNT-1); 
    VopSetTiming(0,5,5,5);
    /**********计时*************/
    printf("\n-------------VOP START-------------\n");    
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0);
    IntRegister(INT_ID_TIMER0 ,NULL);
    IntPendingClear(INT_ID_TIMER0);
    IntEnable(INT_ID_TIMER0); 
    TimerPeriodSet(TIMER0,loadcount,0); 
    timer_time = TimerGetCount(TIMER0);
    while(1)
    {
       // printf("%d\n",frm_cnt++);  
			  frm_cnt++;
			#if 0
       /******第一帧***YUV****/
        ST7735_SetWindow(0, 0, VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_0); 
        VopIntClear(0);
#ifndef DMA_TRAN
        send_UVY_data((uint32 *)testdata_UVY1_160_128);  
#else
        YuvWrite(0, (uint32)testdata_UVY1_160_128, VOP_WIDTH, VOP_HEIGNT);
#endif 
        
        while(VopGetInt(0) == 0) ;
         #endif 
       #if 0
        
        /******第二帧***YUV****/
        ST7735_SetWindow(0, 0, VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_2);         
        VopIntClear(0);
#ifndef DMA_TRAN
        send_UVY_data((uint32 *)testdata_UVY_160_128);
#else
        YuvWrite(0, (uint32)testdata_UVY_160_128, VOP_WIDTH, VOP_HEIGNT);
#endif
        while(VopGetInt(0) == 0);  

        
       #endif 
        #if 1
        /******第三帧****RGB***/
        #ifndef DMA_TRAN 
        ST7735_SetWindow(0, 0, VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_RGB565, YUV_VERSION_0);         
        VopIntClear(0);
        send_rgb_data((uint16 *) testdata_160_128_RGB565);
        while(VopGetInt(0) == 0);  
        #else
	    ST7735_SetWindow(0, 0,VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_RGB565, YUV_VERSION_0); 
        VopSetSplit(0, VOP_CON_SPLIT_FOUR);//  1
        VopSetHWMode(0,VOP_CON_HWORD_SWAP);
        VopIntClear(0);
        Rgb565Write(0,(uint32)testdata_160_128_RGB565,VOP_WIDTH, VOP_HEIGNT);
				while(VopGetInt(0) == 0);  
        #endif
	     #endif
	    #if 1
        /******第四帧*****128*160*YUV*/
        ST7735_SetWindow(0, 0,VOP_WIDTH - 1, VOP_HEIGNT - 1 );
        VopSetStart(0,1);
        VopSetWidthHeight(0,VOP_WIDTH,  VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_1);         
        VopIntClear(0);
#ifndef DMA_TRAN
        send_Y_UV_data((uint32 *)testdata_Y_160_128,(uint32 *)testdata_UV_160_128);
#else
        YuvLlpWrite(0,(uint32 )testdata_Y_160_128,(uint32 )testdata_UV_160_128,VOP_WIDTH,VOP_HEIGNT);
#endif 
        while(VopGetInt(0) == 0);  
     #endif 
       if(frm_cnt == 300)
        {
           break;
        }
    }
     
    timer_time -= TimerGetCount(TIMER0);
    printf("time %d \n",timer_time);   
    HIFI_DMA_TO_Unregister();
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0); 
    printf(" DMA yuv 30 frm  is over %d \r\n",timer_time/10);
    return RK_EXIT;
}
#else //320*240
SHELL API rk_err_t hw_yuv2rgb_shell(void)
{
    uint32 i,j;
    uint32 IntStatus;
    int frm_cnt = 0;
    int Tick  = 0,Time;
    ScuClockGateCtr( HCLK_LCDC_GATE, 1);
    ScuSoftResetCtr(LCDC_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(LCDC_SRST, 0); 
    ST7735_Init();
    ST7735_SetWindow(0,0,VOP_WIDTH-1, VOP_HEIGNT-1);
    for(i = 0; i < VOP_HEIGNT; i++)
    {
        for(j = 0; j < VOP_WIDTH; j++)
        {
            VopSendData(0, 0);
        }
    }
    /***************** vop测试************************/
    VopEnableInt(0, VOP_INT_TYPE_FRAME_DONE);   
    VopIntClear(0);	  
  	ST7735_SetWindow(0, 0, VOP_WIDTH-1, VOP_HEIGNT-1); 
    VopSetTiming(0,5,5,5);   
    while(1)
    {
	   frm_cnt++;
        #if 0
        /******第一帧****RGB***/

	    ST7735_SetWindow(0, 0,VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_RGB565, YUV_VERSION_0); 
        VopSetSplit(0, VOP_CON_SPLIT_FOUR);//  1
        VopSetHWMode(0,VOP_CON_HWORD_SWAP);
        VopIntClear(0);
        Rgb565Write(0,(uint32)testdata_320_240_RGB565,VOP_WIDTH, VOP_HEIGNT);
	    while(VopGetInt(0) == 0);         
        #endif
	    #if 0
        /******第二帧*****320*240*YUV*/
        ST7735_SetWindow(0, 0,VOP_WIDTH - 1, VOP_HEIGNT - 1 );
        VopSetStart(0,1);
        VopSetWidthHeight(0,VOP_WIDTH,  VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_1);         
        VopIntClear(0);
        YuvLlpWrite(0,(uint32 )testdata_Y_320_240,(uint32 )testdata_UV_320_240,VOP_WIDTH,VOP_HEIGNT);
        while(VopGetInt(0) == 0);       
         #endif 
       if(frm_cnt == 300)
        {
           break;
        }
    }
    
    rk_printf_no_time(" DMA yuv 30 frm over\n");
}

#endif 

rk_err_t VOP_test(HDC dev, uint8 * pstr)
{
    uint32 cmd;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    StrCnt = ShellItemExtract(pstr,&pItem);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    } 
    cmd = pstr[0];
    pItem += StrCnt;
    pItem++; 
    switch (cmd)
    {
        case '0': 
            ret =  RK_EXIT;           
            return ret;
            
        case '1':  
            ret = hw_yuv2rgb_shell();
            break;
            
        default:
            ret = RK_ERROR;
            break;
    } 
    
    printf("================================================================================\n");
    printf(" VOP Test Menu                                                                 \n");
    printf(" 1. vop_test                                                                  \n");   
    printf(" 0. EXIT                                                                       \n");
    printf("===============================================================================\n");
  
    return ret;    
}

#endif
