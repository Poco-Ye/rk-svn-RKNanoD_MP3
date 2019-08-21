/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   Spi_Test.c
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

#ifdef _I2S_TEST_

#include "Device.h"
#include "DriverInclude.h"
#include "Interrupt.h"
#if 1
uint8 outptr1[32][192] =
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
uint32 length1 = 192 * 32 / 4;
#endif

I2S_mode_t I2S_TEST_MODE;
I2S_CHannel I2S_CH_TEST;
DMA_CFGX I2S0ControlDmaCfg_TX  = {DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX,0};
DMA_CFGX I2S0ControlDmaCfg_RX  = {DMA_CTLL_I2S0_RX, DMA_CFGL_I2S0_RX, DMA_CFGH_I2S0_RX,0};

DMA_CFGX I2S1ControlDmaCfg_TX  = {DMA_CTLL_I2S1_TX, DMA_CFGL_I2S1_TX, DMA_CFGH_I2S1_TX,0};
DMA_CFGX I2S1ControlDmaCfg_RX  = {DMA_CTLL_I2S1_RX, DMA_CFGL_I2S1_RX, DMA_CFGH_I2S1_RX,0};
/*******************************************************************************
** Name: MemoryTest_Menu
** Input:void
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/
void I2sTest_Menu(void)
{
    printf("================================================================================\n");
    printf(" I2S Test Menu                                                                  \n");
    printf(" 1. I2S DMA Write (8k-192k)                                                     \n");
    printf(" 2. I2S DMA Read Write (8k-192k)                                                \n");
    printf(" 3. I2S PIO Write (8k-192k)                                                     \n");
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
I2sFS_en_t I2sDevShellBspGetFs(uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_8000Hz;
        printf("\r\n 8K start 0\n");          //8k
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_11025Hz;
        printf("\r\n 11.025K start 0\n");     //11.025k
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_12KHz;
        printf("\r\n 12K start 0\n");         //12K
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_16KHz;
        printf("\r\n 16K start 0\n");         //16K
    }
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_22050Hz;
        printf("\r\n 22.025K start 0\n");     //22.025K
    }
    //---------------------------------------------------------------//
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_24KHz;
        printf("\r\n 24K start 0\n");         //24K
    }
    else if(StrCmpA(pstr, "6", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_32KHz;
        printf("\r\n 32K   start 0\n");       //32K
    }
    else if(StrCmpA(pstr, "7", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_44100Hz;
        printf("\r\n 44.1K start 0\n");       //44.1K
    }
    else if(StrCmpA(pstr, "8", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_48KHz;
        printf("\r\n 48K start 0\n");         //48K
    }
    else if(StrCmpA(pstr, "9", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_192KHz;
        printf("\r\n 192K  start 0\n");        //192K
    }
    return I2S_TEST_FS;
}

/*******************************************************************************
** Name: I2S_Isr_test
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.8
** Time: 11:50:27
*******************************************************************************/
void I2S_Isr_test(void)
{
    uint32 IntEvent;

    IntEvent = I2SGetIntType(I2S_CH_TEST);

    if (IntEvent & TX_interrupt_active)
    {
        I2S_PIO_Write(I2S_CH_TEST,(uint32*)outptr1,length1);
        return;
    }
}
/*******************************************************************************
** Name: I2S_RW_DmaIntIsr
** Input:void
** Return: void
** Owner:HJ
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
UINT32 I2S_RW_AudioBuf[2][1024] = {0,};
uint32 I2S_RW_BuferIndex = 0;

/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
void I2sRW_DmaIsr_W(void)
{
    uint32 TX_FIFO_ADDR;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH_TEST);

    DmaReStart(1,(uint32)I2S_RW_AudioBuf[1 - I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S1ControlDmaCfg_TX, I2sRW_DmaIsr_W);
}
/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
void I2sRW_DmaIsr_R(void)
{
    uint32 RX_FIFO_ADDR;
    RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH_TEST);
    I2S_RW_BuferIndex = 1 - I2S_RW_BuferIndex;
    DmaReStart(0,RX_FIFO_ADDR,(uint32)I2S_RW_AudioBuf[I2S_RW_BuferIndex],1024, &I2S1ControlDmaCfg_RX, I2sRW_DmaIsr_R);
}
/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
void I2sDmaIsr_test(void)
{
    uint32 TX_FIFO_ADDR;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH_TEST);

    DmaReStart(2, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S1ControlDmaCfg_TX, I2sDmaIsr_test);
    //DmaReStart(2, (uint32)outptr_test, TX_FIFO_ADDR,1024, &I2S0ControlDmaCfg_TX, I2sDmaIsr_test);

}
/*******************************************************************************
** Name: I2sDevShellBspPIO_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:11:58
*******************************************************************************/
rk_err_t I2s0_test(uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;

    DEBUG("I2sDevShellBspPIO_Wtite %d\n",I2S_CH_TEST);
    I2S_CH_TEST = I2S_CH0;
    I2S_TEST_MODE = I2S_SLAVE_MODE;
    I2S_TEST_FS = I2sDevShellBspGetFs(pstr);


    I2S_GPIO_Init(I2S_CH0_PA);  //GPIO init
    //SetI2SClkOutFreq(XIN12M);   //OUT 12M TO CODEC

    Acodec_pll_Source_sel(CLK_ACODEC_PLL);

    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH0);

    Acodec_pll_Source_sel(CLK_ACODEC_PLL);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);

    SetI2SFreq(I2S_CH0,CLK_ACODEC_PLL,0);
    OBS_output_Source_sel(obs_clk_i2s0);

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
    I2s0Cs(I2S0_EXT);

    I2SInit(I2S_CH0,I2S_MASTER_MODE,I2S_FS_44100Hz,I2S_FORMAT,I2S_DATA_WIDTH24,I2S_NORMAL_MODE);

    DmaEnableInt(2);

    DmaStart(2, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S0ControlDmaCfg_TX, I2sDmaIsr_test);
    //DmaStart(2, (uint32)outptr_test, TX_FIFO_ADDR,1024, &I2S0ControlDmaCfg_TX, I2sDmaIsr_test);

    I2SDMAEnable(I2S_CH0,I2S_START_DMA_TX);
    I2SStart(I2S_CH0,I2S_START_DMA_TX);
    while(1);

}
/*******************************************************************************
** Name: I2sDevShellBspPIO_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:11:58
*******************************************************************************/
rk_err_t I2sPIO_Write(uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    DEBUG("I2sDevShellBspPIO_Wtite %d\n",I2S_CH_TEST);
    I2S_CH_TEST = I2S_CH1;
    I2S_TEST_MODE = I2S_SLAVE_MODE;
    I2S_TEST_FS = I2sDevShellBspGetFs(pstr);

    I2S_GPIO_Init(I2S_CH1_PA);  //GPIO init
    SetI2SClkOutFreq(XIN12M);   //OUT 12M TO CODEC

    if(I2S_CH0 == I2S_CH_TEST)
    {
        IntRegister(INT_ID_I2S0 ,I2S_Isr_test);
        IntPendingClear(INT_ID_I2S0);
        IntEnable(INT_ID_I2S0);
    }
    else
    {
        IntRegister(INT_ID_I2S1 ,I2S_Isr_test);
        IntPendingClear(INT_ID_I2S1);
        IntEnable(INT_ID_I2S1);
    }

   //I2S init
   if(I2S_MASTER_MODE == I2S_TEST_MODE)
   {
       printf("I2S_MASTER_MODE\n");
       ScuClockGateCtr(CLK_I2S1_SRC_GATE, 1);  //ACODEC PLL gating open
       ScuClockGateCtr(CLK_I2S1_GATE, 1);      //I2S MCLK gating open
       ScuClockGateCtr(CLK_I2S1_GATE, 1);      //I2S MCLK gating open

       SetI2SFreq(I2S_CH_TEST,CLK_ACODEC_PLL,0);
       Codec5633_PowerOnInitial(1);    //codec is slave
       Codec5633_SetSampleRate(I2S_TEST_FS);

       I2SInit(I2S_CH_TEST,I2S_MASTER_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
   }
   else
   {
       printf("I2S_SLAVE_MODE\n");
       Codec5633_PowerOnInitial(0);    //codec is master
       Codec5633_SetSampleRate(FS_44100Hz);

       I2SInit(I2S_CH_TEST,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
   }

    I2S_PIO_Write(I2S_CH_TEST,(uint32*)outptr1,length1);

    I2SIntEnable(I2S_CH_TEST,I2S_INT_TX);
    I2SStart(I2S_CH_TEST,I2S_START_PIO_TX);

    while(1);
}

/*******************************************************************************
** Name: I2sDevShellBspDMA_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t I2sDMA_RW(uint8 * pstr)
{
   I2sFS_en_t I2S_TEST_FS;
   uint32 i = 0;
   uint32 TX_FIFO_ADDR,RX_FIFO_ADDR;
   pFunc pCallBack_TX,pCallBack_RX;

   printf("I2sDevShellBspDMA_ReadWrite %d\n",I2S_CH_TEST);

   I2S_CH_TEST = I2S_CH1;
   I2S_TEST_MODE = I2S_MASTER_MODE;


   TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH_TEST);
   RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH_TEST);
   I2S_TEST_FS = I2sDevShellBspGetFs(pstr);
   //open  clk
   ScuClockGateCtr(HCLK_DMA_GATE, 1);
   ScuClockGateCtr(CLK_I2S1_OUT_GATE, 1);  //12M gating open

   //open rst ip
   ScuSoftResetCtr(SYSDMA_SRST, 1);
   DelayMs(1);
   ScuSoftResetCtr(SYSDMA_SRST, 0);

   IntRegister(INT_ID_DMA,       (void*)DmaInt);

   IntPendingClear(INT_ID_DMA);
   IntEnable(INT_ID_DMA);

   I2S_GPIO_Init(I2S_CH1_PA);  //GPIO init
    SetI2SFreq(I2S_CH1,I2S_XIN12M,0);
    SetI2SClkOutFreq(I2S1_CLK);   //OUT 12M TO CODEC

   //Codec5633_SetVolumet();
   //I2S init
   if(I2S_MASTER_MODE == I2S_TEST_MODE)
   {
       printf("I2S_MASTER_MODE\n");
       ScuClockGateCtr(CLK_I2S1_SRC_GATE, 1);  //ACODEC PLL gating open
       ScuClockGateCtr(CLK_I2S1_GATE, 1);      //I2S MCLK gating open
       ScuClockGateCtr(CLK_I2S1_GATE, 1);      //I2S MCLK gating open

       SetI2SFreq(I2S_CH_TEST,CLK_ACODEC_PLL,0);
       Codec5633_PowerOnInitial(1);    //codec is slave
       Codec5633_SetSampleRate(I2S_TEST_FS);

       I2SInit(I2S_CH_TEST,I2S_MASTER_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
   }
   else
   {
       printf("I2S_SLAVE_MODE\n");
       Codec5633_PowerOnInitial(0);    //codec is master
        Codec5633_SetSampleRate(I2S_TEST_FS);
        Codec5633_SetVolumet(2);

       I2SInit(I2S_CH_TEST,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
   }

   DmaEnableInt(0);
   DmaEnableInt(1);

   DmaStart(0,RX_FIFO_ADDR,(uint32)I2S_RW_AudioBuf[I2S_RW_BuferIndex],1024, &I2S1ControlDmaCfg_RX, I2sRW_DmaIsr_R);
   DelayMs(5);
   DmaStart(1,(uint32)I2S_RW_AudioBuf[1 - I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S1ControlDmaCfg_TX, I2sRW_DmaIsr_W);

   I2SDMAEnable(I2S_CH_TEST,I2S_START_DMA_RTX);
   I2SStart(I2S_CH_TEST,I2S_START_DMA_RTX);

   while(1);
}
/*******************************************************************************
** Name: I2sDevShellBspDMA_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
rk_err_t I2sDMA_Write(uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;

    I2S_CH_TEST = I2S_CH1;
    I2S_TEST_MODE = I2S_SLAVE_MODE;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH_TEST);
    printf("I2sDevShellBspDMA_Wtite %d\n",I2S_CH_TEST);


    I2S_TEST_FS = I2sDevShellBspGetFs(pstr);

    //open  clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);
    ScuClockGateCtr(CLK_I2S1_OUT_GATE, 1);  //12M gating open

    //open rst ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA,       (void*)DmaInt);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    I2S_GPIO_Init(I2S_CH1_PA);  //GPIO init
    SetI2SFreq(I2S_CH1,I2S_XIN12M,0);
    SetI2SClkOutFreq(I2S1_CLK);   //OUT 12M TO CODEC

    //Codec5633_SetVolumet();
    //I2S init
    if(I2S_MASTER_MODE == I2S_TEST_MODE)
    {
        printf("I2S_MASTER_MODE\n");
        ScuClockGateCtr(CLK_I2S1_SRC_GATE, 1);  //ACODEC PLL gating open
        ScuClockGateCtr(CLK_I2S1_GATE, 1);      //I2S MCLK gating open
        ScuClockGateCtr(CLK_I2S1_GATE, 1);      //I2S MCLK gating open

        SetI2SFreq(I2S_CH_TEST,CLK_ACODEC_PLL,0);
        Codec5633_PowerOnInitial(1);    //codec is slave
        Codec5633_SetSampleRate(I2S_TEST_FS);

        I2SInit(I2S_CH_TEST,I2S_MASTER_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }
    else
    {
        printf("I2S_SLAVE_MODE\n");
        Codec5633_PowerOnInitial(0);    //codec is master
        Codec5633_SetSampleRate(I2S_TEST_FS);
        Codec5633_SetVolumet(2);

        I2SInit(I2S_CH_TEST,I2S_SLAVE_MODE,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }

    DmaEnableInt(2);

    DmaStart(2, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S1ControlDmaCfg_TX, I2sDmaIsr_test);

    I2SDMAEnable(I2S_CH_TEST,I2S_START_DMA_TX);
    I2SStart(I2S_CH_TEST,I2S_START_DMA_TX);
    while(1);
}

/*******************************************************************************
** Name: I2sDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 11:32:09
*******************************************************************************/
rk_err_t I2s_test(HDC dev, uint8 * pstr)
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

        case '1':  //DMA_Wtite
            ret = I2sDMA_Write(pItem);
            break;
        case '2':  //DMA_Read
            ret = I2sDMA_RW(pItem);
            break;
        case '3':  //PIO_Wtite
            ret = I2sPIO_Write(pItem);
            break;
        case '4':  //PIO_Wtite
            ret = I2s0_test(pItem);
            break;
        default:
            ret = RK_ERROR;
            break;
    }
    I2sTest_Menu();
    return ret;
}

#endif

