/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   Acodec_Test.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                                           
*    desc:    ORG.
********************************************************************************
*/

/*-------------------------------- Includes ----------------------------------*/

#include "sysinclude.h"

#ifdef _ACODEC_TEST_

#include "Device.h"	
#include "DriverInclude.h"
#include "Interrupt.h"
#if 1
uint8 Acodec_outptr1[32][192] = 
{
    //0
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    //8
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    //16
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    //24
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    { 
    #include "test01_1K0_441.data" 
    },
    //32    
};
uint32 Acodec_length1 = 192 * 32 / 4; 
#endif
I2S_mode_t Acodec_I2S_TEST_MODE;
I2S_CHannel Acodec_I2S_CH_TEST;

DMA_CFGX I2S0DmaCfg_TX  = {DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX,0};
DMA_CFGX I2S0DmaCfg_RX  = {DMA_CTLL_I2S0_RX, DMA_CFGL_I2S0_RX, DMA_CFGH_I2S0_RX,0};
DMA_CFGX I2S1DmaCfg_TX  = {DMA_CTLL_I2S1_TX, DMA_CFGL_I2S1_TX, DMA_CFGH_I2S1_TX,0};
DMA_CFGX I2S1DmaCfg_RX  = {DMA_CTLL_I2S1_RX, DMA_CFGL_I2S1_RX, DMA_CFGH_I2S1_RX,0};
/*******************************************************************************
** Name: MemoryTest_Menu
** Input:void
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/
void AcodecTest_Menu(void)
{
    printf("================================================================================\n");
    printf(" I2S Test Menu                                                                  \n");
    printf(" 1. Acodec_Dac2HP_test          (8k-192k)                                       \n");
    printf(" 2. Acodec_Dac2LO_test          (8k-192k)                                       \n");
    printf(" 3. Acodec_Line2in_test         (8k-192k)                                       \n");
    printf(" 4. Acodec_Mic2in_test          (8k-192k)                                       \n");
    printf(" 5. Acodec_MIC1ADC2DAC_test     (8k-192k)                                       \n");
    printf(" 6. Acodec_Line2ADC_test        (8k-192k)                                       \n");
    printf(" 0. EXIT                                                                        \n");
    printf("================================================================================\n");    
}
/*******************************************************************************
** Name: I2sDevShellBspGetFs
** Input:uint8 * pstr
** Return: I2sFS_en_t
** Owner:hj
** Date: 2014.12.8
** Time: 14:01:19
*******************************************************************************/
CodecFS_en_t AcodecGetFs(uint8 * pstr)
{
    CodecFS_en_t TEST_FS;   
    uint32 i = 0;
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        TEST_FS = FS_8000Hz;
        printf("\r\n 8K start 0\n");          //8k
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        TEST_FS = FS_11025Hz;
        printf("\r\n 11.025K start 0\n");     //11.025k    
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        TEST_FS = FS_12KHz;  
        printf("\r\n 12K start 0\n");         //12K
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        TEST_FS = FS_16KHz;
        printf("\r\n 16K start 0\n");         //16K
    } 
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        TEST_FS = FS_22050Hz;
        printf("\r\n 22.025K start 0\n");     //22.025K    
    } 
    //---------------------------------------------------------------//
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        TEST_FS = FS_24KHz;
        printf("\r\n 24K start 0\n");         //24K      
    } 
    else if(StrCmpA(pstr, "6", 1) == 0)
    {
        TEST_FS = FS_32KHz;
        printf("\r\n 32K   start 0\n");       //32K      
    }
    else if(StrCmpA(pstr, "7", 1) == 0)
    {
        TEST_FS = FS_44100Hz;
        printf("\r\n 44.1K start 0\n");       //44.1K      
    } 
    else if(StrCmpA(pstr, "8", 1) == 0)
    {
        TEST_FS = FS_48KHz;
        printf("\r\n 48K start 0\n");         //48K      
    }
    else if(StrCmpA(pstr, "9", 1) == 0)
    {
        TEST_FS = FS_192KHz;
        printf("\r\n 192K  start 0\n");        //192K      
    }
    return TEST_FS;
}

/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
void Acodec_DmaIsr_test(void)
{
    uint32 TX_FIFO_ADDR;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);

    DmaReStart(2, (uint32)Acodec_outptr1, TX_FIFO_ADDR,Acodec_length1, &I2S0DmaCfg_TX, Acodec_DmaIsr_test);

}
/*******************************************************************************
** Name: I2S_RW_DmaIntIsr
** Input:void
** Return: void
** Owner:HJ
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
UINT32 Acodec_I2S_RW_AudioBuf[2][1024] = {0,};
uint32 Acodec_I2S_RW_BuferIndex = 0;

UINT32 Acodec_ADC_R_AudioBuf[1024*5] = {0,};
UINT32 Acodec_ADC_R_Size;
UINT32 Acodec_ADC_R_log = 1;
/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/

void Acodec_LINE2ADC_DmaIsr_R(void)
{
    uint32 RX_FIFO_ADDR;
    RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH0);
    Acodec_ADC_R_Size += 1024;
    if(Acodec_ADC_R_Size <= 5120)
        DmaReStart(0,RX_FIFO_ADDR,(uint32)(&Acodec_ADC_R_AudioBuf[Acodec_ADC_R_Size - 1024]),1024, &I2S0DmaCfg_RX, Acodec_LINE2ADC_DmaIsr_R);
    else
   {
        Acodec_ADC_R_log = 0;
        DmaDisableInt(0);  
   }
}
/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
void Acodec_RW_DmaIsr_W(void)
{
    uint32 TX_FIFO_ADDR;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);
    DmaReStart(1,(uint32)Acodec_I2S_RW_AudioBuf[1 - Acodec_I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S0DmaCfg_TX, Acodec_RW_DmaIsr_W);        
}
void Acodec_RW_DmaIsr_R(void)
{
    uint32 RX_FIFO_ADDR;
    RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH1);
    Acodec_I2S_RW_BuferIndex = 1 - Acodec_I2S_RW_BuferIndex;
    DmaReStart(0,RX_FIFO_ADDR,(uint32)Acodec_I2S_RW_AudioBuf[Acodec_I2S_RW_BuferIndex],1024, &I2S1DmaCfg_RX, Acodec_RW_DmaIsr_R);
}
/*******************************************************************************
** Name: Acodec_Dac2HP_test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t Acodec_Mic2in_test(uint8 * pstr)
{
    CodecFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;

    I2S_TEST_FS = AcodecGetFs(pstr);
    
    Acodec_pll_Source_sel(CLK_ACODEC_PLL);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);

    SetI2SFreq(I2S_CH0,CLK_ACODEC_PLL,0);
    OBS_output_Source_sel(obs_clk_i2s0);
    
    printf("Acodec_Mic2in_test\n"); 
    I2s0Cs(I2S0_ACODEC);
    Codec_PowerOnInitial();    //codec is master
    ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
    Codec_SetMode(Codec_DACoutHP,I2S_TEST_FS);
    Codec_SetMode(Codec_MICAdc,I2S_TEST_FS);
    //Codec_MUX_Power_down();
    //Codec_ADCPOWER_DOWN();
    Codec_SetSampleRate(I2S_TEST_FS);
    Codec_DACUnMute();
    
    //ACodec_Set_ADCMUX_Vol(11);    
    //ACodec_Set_MIC_AnaVol(2);
}
/*******************************************************************************
** Name: Acodec_Dac2HP_test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t Acodec_Line2in_test(uint8 * pstr)
{
    CodecFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;

    I2S_TEST_FS = AcodecGetFs(pstr);
    Acodec_pll_Source_sel(CLK_ACODEC_PLL);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);

    SetI2SFreq(I2S_CH0,CLK_ACODEC_PLL,0);
    OBS_output_Source_sel(obs_clk_i2s0);
    printf("Acodec_Line2in_test\n"); 
    I2s0Cs(I2S0_ACODEC);
    Codec_PowerOnInitial();    //codec is master
    ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
    Codec_SetMode(Codec_DACoutHP,I2S_TEST_FS);
    Codec_SetMode(Codec_Line2in,I2S_TEST_FS);
    Codec_SetSampleRate(I2S_TEST_FS);
    Codec_DACUnMute();
   // ACodec_Set_ADCMUX_Vol(11);
    
}
/*******************************************************************************
** Name: Acodec_Dac2HP_test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t Acodec_MIC1ADC2DAC_test(uint8 * pstr)
{    
   CodecFS_en_t I2S_TEST_FS;
   uint32 i = 0;
   uint32 TX_FIFO_ADDR,RX_FIFO_ADDR;
   pFunc pCallBack_TX,pCallBack_RX;

   printf("Acodec_MIC1ADC2DAC_test\n"); 
   
   Acodec_I2S_CH_TEST = I2S_CH0;
   Acodec_I2S_TEST_MODE = I2S_SLAVE_MODE;
   
   
   TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);
   RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH0);
   I2S_TEST_FS = AcodecGetFs(pstr);
   //open  clk
   ScuClockGateCtr(HCLK_DMA_GATE, 1);
   
   //open rst ip
   ScuSoftResetCtr(SYSDMA_SRST, 1);
   DelayMs(1);
   ScuSoftResetCtr(SYSDMA_SRST, 0);
   
   IntRegister(INT_ID_DMA,       (void*)DmaInt);
   
   IntPendingClear(INT_ID_DMA);
   IntEnable(INT_ID_DMA);
      
   //I2S init
   I2s0Cs(I2S0_ACODEC);
   
   printf("Codec_PowerOnInitial\n"); 
   Codec_PowerOnInitial();    //codec is master
   ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
   Codec_SetMode(Codec_MICAdc,I2S_TEST_FS);
   Codec_SetMode(Codec_DACoutHP,I2S_TEST_FS);
   Codec_SetSampleRate(I2S_TEST_FS);
   Codec_DACUnMute();  
   
   I2SInit(I2S_CH0,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE); 
 
   DmaEnableInt(0);
   DmaEnableInt(1);
   
   DmaStart(0,RX_FIFO_ADDR,(uint32)Acodec_I2S_RW_AudioBuf[Acodec_I2S_RW_BuferIndex],1024, &I2S0DmaCfg_RX, Acodec_RW_DmaIsr_R);
   DelayMs(5);
   DmaStart(1,(uint32)Acodec_I2S_RW_AudioBuf[1 - Acodec_I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S0DmaCfg_TX, Acodec_RW_DmaIsr_W);        
   
   I2SDMAEnable(I2S_CH0,I2S_START_DMA_RTX);
   I2SStart(I2S_CH0,I2S_START_DMA_RTX);    
  
   //while(1);
}
/*******************************************************************************
** Name: Acodec_Dac2HP_test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t Acodec_Line2ADC_test(uint8 * pstr)
{    
   CodecFS_en_t I2S_TEST_FS;
   int16* pCodecReceiveData;

   uint32 i = 0;
   uint32 TX_FIFO_ADDR,RX_FIFO_ADDR;
   pFunc pCallBack_TX,pCallBack_RX;
   
   Acodec_pll_Source_sel(CLK_ACODEC_PLL);
   Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
    
   SetI2SFreq(I2S_CH0,CLK_ACODEC_PLL,0);
   OBS_output_Source_sel(obs_clk_i2s0);
   
   printf("Acodec_Line2ADC2DAC_test %d\n",Acodec_I2S_CH_TEST); 
   
   Acodec_I2S_CH_TEST = I2S_CH0;
   
   
   //TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);
   RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH0);
   I2S_TEST_FS = AcodecGetFs(pstr);
   //open  clk
   ScuClockGateCtr(HCLK_DMA_GATE, 1);
   
   //open rst ip
   ScuSoftResetCtr(SYSDMA_SRST, 1);
   DelayMs(1);
   ScuSoftResetCtr(SYSDMA_SRST, 0);
   
   IntRegister(INT_ID_DMA,       (void*)DmaInt);
   
   IntPendingClear(INT_ID_DMA);
   IntEnable(INT_ID_DMA);
      
   //I2S init
      //I2S init
   I2s0Cs(I2S0_ACODEC);
   
   printf("Codec_PowerOnInitial\n"); 
   Codec_PowerOnInitial();    //codec is master
   ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
   Codec_SetMode(Codec_Line2ADC,I2S_TEST_FS);
   //Codec_SetMode(Codec_DACoutHP,I2S_TEST_FS);
   Codec_SetSampleRate(I2S_TEST_FS);
   //Codec_DACUnMute();  
   
   I2SInit(I2S_CH0,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE); 
    Codec_MUX_Power_down();
   DmaEnableInt(0);
   //DmaEnableInt(1);
   
   DmaStart(0,RX_FIFO_ADDR,(uint32)(&Acodec_ADC_R_AudioBuf[0]),1024, &I2S0DmaCfg_RX, Acodec_LINE2ADC_DmaIsr_R);
   DelayMs(5);
   //DmaStart(1,(uint32)Acodec_I2S_RW_AudioBuf[1 - Acodec_I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S0DmaCfg_TX, Acodec_RW_DmaIsr_W);        
   Acodec_ADC_R_Size = 1024;
   
   I2SDMAEnable(I2S_CH0,I2S_START_DMA_RX);
   I2SStart(I2S_CH0,I2S_START_DMA_RX);    
  
   while(Acodec_ADC_R_log);
    I2SStop(I2S_CH0,I2S_START_DMA_RX);
   printf("\n**************HJ ODD data*****************\n"); 
   pCodecReceiveData = (int16*)(&Acodec_ADC_R_AudioBuf[0]);
    for(i=0;i<=(5120);i++)
	{

	   printf("%d,",*pCodecReceiveData++);
	   printf("%d,",*pCodecReceiveData++);
    	 if((i%16) == 0)
    		printf("\n");
    } 	
		printf("\n***********************************************\n"); 
		printf("\n*************HJ even data************************  \n");
        while(1);
}
rk_err_t Acodec_Line2ADC2DAC_test(uint8 * pstr)
{    
   CodecFS_en_t I2S_TEST_FS;
   uint32 i = 0;
   uint32 TX_FIFO_ADDR,RX_FIFO_ADDR;
   pFunc pCallBack_TX,pCallBack_RX;

   
   //TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH1);
   //RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH0);
   
   
   TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);
   RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH1);
   I2S_TEST_FS = AcodecGetFs(pstr);
   Codec_PowerOnInitial();    //codec is master
   //open  clk
   ScuClockGateCtr(HCLK_DMA_GATE, 1);
   
   //open rst ip
   ScuSoftResetCtr(SYSDMA_SRST, 1);
   DelayMs(1);
   ScuSoftResetCtr(SYSDMA_SRST, 0);
   
   IntRegister(INT_ID_DMA,       (void*)DmaInt);
   
   IntPendingClear(INT_ID_DMA);
   IntEnable(INT_ID_DMA);
      
   Acodec_pll_Source_sel(CLK_ACODEC_PLL);
   Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
   OBS_output_Source_sel(obs_clk_i2s1);
   I2S_GPIO_Init(I2S_CH1_PA);  //GPIO init   
   SetI2SFreq(I2S_CH1,CLK_ACODEC_PLL,0);
   SetI2SClkOutFreq(I2S1_CLK); 
   Codec5633_PowerOnInitial(1);    //codec is slave
   Codec5633_SetSampleRate(I2S_TEST_FS);
   Codec5633_SetVolumet(2);
   I2SInit(I2S_CH1,I2S_MASTER_MODE,I2S_FS_48KHz,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE); 
   //I2S init
      //I2S init
   I2s0Cs(I2S0_ACODEC);
   
   printf("Codec_PowerOnInitial\n"); 
   ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
   //Codec_SetMode(Codec_Line2ADC,I2S_TEST_FS);
   Codec_SetMode(Codec_DACoutHP,I2S_TEST_FS);
   Codec_SetSampleRate(I2S_TEST_FS);
   
   I2SInit(I2S_CH0,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE); 
 
   DmaEnableInt(0);
   DmaEnableInt(1);
   
   DmaStart(0,RX_FIFO_ADDR,(uint32)Acodec_I2S_RW_AudioBuf[Acodec_I2S_RW_BuferIndex],1024, &I2S1DmaCfg_RX, Acodec_RW_DmaIsr_R);
   DelayMs(5);
   DmaStart(1,(uint32)Acodec_I2S_RW_AudioBuf[1 - Acodec_I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S0DmaCfg_TX, Acodec_RW_DmaIsr_W);        
   
   I2SDMAEnable(I2S_CH1,I2S_START_DMA_RX);
   I2SStart(I2S_CH1,I2S_START_DMA_RX);    
  
   I2SDMAEnable(I2S_CH0,I2S_START_DMA_TX);
   I2SStart(I2S_CH0,I2S_START_DMA_TX);    
   //while(1);
}
/*******************************************************************************
** Name: Acodec_Dac2HP_test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t Acodec_Dac2LO_test(uint8 * pstr)
{
    CodecFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;
    
    Acodec_I2S_CH_TEST = I2S_CH0;
    Acodec_I2S_TEST_MODE = I2S_SLAVE_MODE;
    
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);
    printf("Acodec_Dac2LO_test %d\n",Acodec_I2S_CH_TEST); 


    I2S_TEST_FS = AcodecGetFs(pstr);

    //open  clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);
    
    //open rst ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA,       (void*)DmaInt);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    //Codec5633_SetVolumet();
    //I2S init
    I2s0Cs(I2S0_ACODEC);

    printf("Codec_PowerOnInitial\n"); 
    Codec_PowerOnInitial();    //codec is master
    ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
    Codec_SetMode(Codec_DACoutLINE,I2S_TEST_FS);
    Codec_SetSampleRate(I2S_TEST_FS);
    Codec_DACUnMute();
    
    I2SInit(I2S_CH0,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE); 
    
    DmaEnableInt(2);

    DmaStart(2, (uint32)Acodec_outptr1, TX_FIFO_ADDR,Acodec_length1, &I2S0DmaCfg_TX, Acodec_DmaIsr_test);        
    
    I2SDMAEnable(I2S_CH0,I2S_START_DMA_TX);
	I2SStart(I2S_CH0,I2S_START_DMA_TX);
    //while(1);    
}
/*******************************************************************************
** Name: I2sDevShellBspDMA_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t Acodec_Dac2HP_test(uint8 * pstr)
{
    CodecFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;
    
    Acodec_I2S_CH_TEST = I2S_CH0;
    Acodec_I2S_TEST_MODE = I2S_SLAVE_MODE;
    
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);
    printf("Acodec_Dac2HP_test %d\n",Acodec_I2S_CH_TEST); 

    Acodec_pll_Source_sel(CLK_ACODEC_PLL);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
    SetI2SFreq(I2S_CH0,CLK_ACODEC_PLL,0);
    OBS_output_Source_sel(obs_clk_i2s0);

    I2S_TEST_FS = AcodecGetFs(pstr);

    //open  clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);
    
    //open rst ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA,       (void*)DmaInt);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    //I2S init
    I2s0Cs(I2S0_ACODEC);

    printf("Codec_PowerOnInitial\n"); 
    Codec_PowerOnInitial();    //codec is master
    ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
    Codec_SetSampleRate(I2S_TEST_FS);
    Codec_SetMode(Codec_DACoutHP,I2S_TEST_FS);

    //ACodec_ADC2DAC_MIX(CodecMIX_ENABLE);
    Codec_DACUnMute();
    
    Codec_SetVolumet(1);
    I2SInit(I2S_CH0,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE); 
    
    DmaEnableInt(2);

    DmaStart(2, (uint32)Acodec_outptr1, TX_FIFO_ADDR,Acodec_length1, &I2S0DmaCfg_TX, Acodec_DmaIsr_test);        
    
    I2SDMAEnable(I2S_CH0,I2S_START_DMA_TX);
	I2SStart(I2S_CH0,I2S_START_DMA_TX);
    //while(1);
}
rk_err_t Acodec_REG_Write_test(uint8 * pstr)
{
    CodecFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;
    Acodec_I2S_CH_TEST = I2S_CH0;
    Acodec_I2S_TEST_MODE = I2S_SLAVE_MODE;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);
    printf("Acodec_REG_Write_test \n"); 
    Acodec_pll_Source_sel(CLK_ACODEC_PLL);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
    OBS_output_Source_sel(obs_clk_i2s0);
    I2S_TEST_FS = AcodecGetFs(pstr);
    I2s0Cs(I2S0_ACODEC);
    printf("Codec_PowerOnInitial\n"); 
    Codec_PowerOnInitial();    //codec is master
    ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
    Codec_SetMode(Codec_DACoutHP,I2S_TEST_FS);
    Codec_SetSampleRate(I2S_TEST_FS);
    Codec_DACUnMute();
    I2SInit(I2S_CH0,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE); 
}
/*******************************************************************************
** Name: I2sDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 11:32:09
*******************************************************************************/
rk_err_t Acodec_test(HDC dev, uint8 * pstr)
{
    uint32 cmd;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

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
        case '0':  //bsp help
            ret =  RK_EXIT;           
            return;
            
        case '1':  //Acodec_Dac2HP_test
            ret = Acodec_Dac2HP_test(pItem);
            break;
        case '2':  //Acodec_Dac2LO_test
            ret = Acodec_Dac2LO_test(pItem);
            break;            
        case '3':  //Acodec_Line2in_test
            ret = Acodec_Line2in_test(pItem);
            break;  
        case '4':  //Acodec_Mic2in_test
            //ret = Acodec_Line2ADC2DAC_test(pItem);
            ret = Acodec_Mic2in_test(pItem);            
            break;            
        case '5':  //Acodec_MIC1ADC2DAC_test
            ret = Acodec_MIC1ADC2DAC_test(pItem);
            break;        
        case '6':  //Acodec_Line2ADC_test
            ret = Acodec_Line2ADC2DAC_test(pItem);
            break; 
        default:
            ret = RK_ERROR;
            break;
    } 
    AcodecTest_Menu();
    return ret;    
}
#endif

