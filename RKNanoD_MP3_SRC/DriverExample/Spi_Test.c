/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name：   Spi_Test.c
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

#ifdef _SPI_TEST_

#include "Device.h"
#include "DriverInclude.h"
#include "Interrupt.h"
#include "hw_spi.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define SPI_CTL_TXRX_MASTER_TEST ( SPI_MASTER_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | BIG_ENDIAN_MODE | CS_2_SCLK_OUT_1_2_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_LOW \
                            | SERIAL_CLOCK_PHASE_START | DATA_FRAME_8BIT)

#define SPI_CTL_TXRX_SLAVE_TEST ( SPI_SLAVE_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | BIG_ENDIAN_MODE | CS_2_SCLK_OUT_1_2_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_LOW \
                            | SERIAL_CLOCK_PHASE_MIDDLE | DATA_FRAME_8BIT)
uint8 Spi_TestBuffer[1024];
uint8 Spi_TestBuffer_Read[1024] = {0,};

uint32 Spi_NeedTransLen;

uint32 Spi_NeedTransLen_RX;
uint32 Spi_Testflag;

eSPI_TRANSFER_MODE_t SPI_TEST_MODE;
eSPI_ch_t SPI_CH_TEST;
DMA_CFGX SPI0ControlDmaCfg_TX  = {DMA_CTLL_SPI0_8_MTX, DMA_CFGL_SPI0_8_MTX, DMA_CFGH_SPI0_8_MTX,0};
DMA_CFGX SPI1ControlDmaCfg_TX  = {DMA_CTLL_SPI1_8_MTX, DMA_CFGL_SPI1_8_MTX, DMA_CFGH_SPI1_8_MTX,0};

DMA_CFGX SPI0ControlDmaCfg_RX  = {DMA_CTLL_SPI0_8_MRX, DMA_CFGL_SPI0_8_MRX, DMA_CFGH_SPI0_8_MRX,0};
DMA_CFGX SPI1ControlDmaCfg_RX  = {DMA_CTLL_SPI1_8_MRX, DMA_CFGL_SPI1_8_MRX, DMA_CFGH_SPI1_8_MRX,0};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MemoryTest_Menu
** Input:void
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/
void SpiTest_Menu(void)
{
    printf("================================================================================\n");
    printf(" SPI Test Menu                                                                  \n");
    printf(" 1. SPI PIO/DMA Write CH = 0; IO = PA                                           \n");
    printf(" 2. SPI PIO/DMA Write CH = 0; IO = PB                                           \n");
    printf(" 3. SPI PIO/DMA Write CH = 1; IO = PA                                           \n");
    printf(" 4. SPI PIO/DMA Write CH = 1; IO = PB                                           \n");
    printf(" 5. SPI PIO READ Write                                                          \n");
    printf("    0.M = CH0; S = CH1                                                          \n");
    printf("    1.M = CH1; S = CH0                                                          \n");
    printf(" 6. SPI DMA READ Write                                                          \n");
    printf("    0.M = CH0; S = CH1                                                          \n");
    printf("    1.M = CH1; S = CH0                                                          \n");
    printf(" 0. EXIT                                                                        \n");
    printf("================================================================================\n");
}
/*******************************************************************************
** Name: SpiDMAIntIsr_Test
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.9
** Time: 20:39:07
*******************************************************************************/

void SpiDMAIntIsr_Test(void)
{
    uint32 spiIntType, realsize,TX_FIFO_ADDR;

    if(SPI_TRANSMIT_RECEIVE == SPI_TEST_MODE)
    {
        DmaDisableInt(2);
        return;
    }

    TX_FIFO_ADDR = SpiGetTxFIFOaddr(SPI_CH_TEST);
    DmaDisableInt(2);
    if(SPI_CH0 == SPI_CH_TEST)
    {
        DmaEnableInt(2);
        DmaReStart(2, (uint32)Spi_TestBuffer, (uint32)TX_FIFO_ADDR,1024, &SPI0ControlDmaCfg_TX, SpiDMAIntIsr_Test);
    }
    else
    {
        DmaEnableInt(2);
        DmaReStart(2, (uint32)Spi_TestBuffer, (uint32)TX_FIFO_ADDR,1024, &SPI1ControlDmaCfg_TX, SpiDMAIntIsr_Test);
    }

}

/*******************************************************************************
** Name: SpiIntIsr_Test_TX
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.9
** Time: 20:39:07
*******************************************************************************/

void SpiIntIsr_Test_TX(void)
{
    uint32 spiIntType, realsize;

    spiIntType = SpiGetInt(SPI_CH_TEST);

    if (spiIntType & FIFO_EMPTY)
    {
        if (Spi_NeedTransLen)
        {
            realsize = SPIWriteFIFO(SPI_CH_TEST,&Spi_TestBuffer[Spi_NeedTransLen],Spi_NeedTransLen);
            Spi_NeedTransLen -= realsize;
            if(Spi_NeedTransLen == 0)
            {
                Spi_Testflag = 0;
                SpiDisalbeTxInt(SPI_CH_TEST);
            }
        }
        else
        {
            Spi_Testflag = 0;
            SpiDisalbeTxInt(SPI_CH_TEST);
        }
    }
}

/*******************************************************************************
** Name: SpiDevShellBspDMA_ReadWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:39:07
*******************************************************************************/

rk_err_t SpiDMA_ReadWrite(uint8 * pstr)
{
    rk_size_t realsize,_RXrealsize;
    pSPI_REG Spi_Reg0,Spi_Reg1;
    uint8 Spi_clockBuffer[5] = {0,};
    eSPI_ch_t spiChMaster,spiChSlave;
    uint32 i = 0;
    uint32 SPI0_RX_FIFO_ADDR, SPI0_TX_FIFO_ADDR,SPI1_RX_FIFO_ADDR,SPI1_TX_FIFO_ADDR;

    SPI_TEST_MODE = SPI_TRANSMIT_RECEIVE;

    Spi_Reg0 = SpiGetCH(SPI_CH0);
    Spi_Reg1 = SpiGetCH(SPI_CH1);

    SPI0_RX_FIFO_ADDR = SpiGetRxFIFOaddr(SPI_CH0);
    SPI0_TX_FIFO_ADDR = SpiGetTxFIFOaddr(SPI_CH0);

    SPI1_RX_FIFO_ADDR = SpiGetRxFIFOaddr(SPI_CH1);
    SPI1_TX_FIFO_ADDR = SpiGetTxFIFOaddr(SPI_CH1);


    if(StrCmpA(pstr, "0", 1) == 0)
    {
        spiChMaster = SPI_CH0;
        spiChSlave = SPI_CH1;
        //DEBUG("Master-SPI0;Slave-SPI1");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        spiChMaster = SPI_CH1;
        spiChSlave = SPI_CH0;
        //DEBUG("Master-SPI1;Slave-SPI0");
    }

    Spi_NeedTransLen = 0;
    Spi_NeedTransLen_RX = 5;
    Spi_Testflag = 1;
    for(i=0;i<10;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }

    //open  clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA,       (void*)DmaInt);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(2);

    SPIInit(spiChMaster,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST);

    SPIInit(spiChSlave,0, 12*1000*1000, SPI_CTL_TXRX_SLAVE_TEST);

    SPIDmaRead(spiChSlave,5);

    SpiEanbleChannel(spiChMaster,0);
    SPIDmaWrite(spiChMaster);

    //写入0x55 0xaa 0x55 0xaa 0x55 发送到slave
    if(spiChMaster == SPI_CH0)
    {
        DmaStart(2, (uint32)Spi_TestBuffer, (uint32)SPI0_TX_FIFO_ADDR,5, &SPI0ControlDmaCfg_TX, SpiDMAIntIsr_Test);
    }
    else
    {
        DmaStart(2, (uint32)Spi_TestBuffer, (uint32)SPI1_TX_FIFO_ADDR,5, &SPI1ControlDmaCfg_TX, SpiDMAIntIsr_Test);
    }

    SpiGetInt(spiChMaster);
    //SpiEnableTxInt(SPI_CH_TEST);
    SpiWaitIdle(spiChMaster);

    //从Slave读出0x55 0xaa 0x55 0xaa 0x55
    DmaEnableInt(2);

    if(spiChMaster == SPI_CH0)
    {
        DmaStart(2, (uint32)SPI1_RX_FIFO_ADDR,(uint32)Spi_TestBuffer_Read, 5, &SPI1ControlDmaCfg_RX, SpiDMAIntIsr_Test);
    }
    else
    {
        DmaStart(2, (uint32)SPI0_RX_FIFO_ADDR,(uint32)Spi_TestBuffer_Read, 5, &SPI0ControlDmaCfg_RX, SpiDMAIntIsr_Test);
    }

    //SPIReadFIFO(spiChSlave,Spi_TestBuffer_Read,5);

    //读出master Read FIFO里面的无效数据
    SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);

    //check
    if((Spi_TestBuffer_Read[0] == 0x55)
       && (Spi_TestBuffer_Read[1] == 0xaa)
       && (Spi_TestBuffer_Read[2] == 0x55)
       && (Spi_TestBuffer_Read[3] == 0xaa)
       && (Spi_TestBuffer_Read[4] == 0x55))
    {
       //写入0x55 0xaa 0x55 0xaa 0x55 发送到master
       //SPIDmaWrite(spiChSlave);

       DmaEnableInt(2);
       //写入数据，需要MASTER提供clock
       if(spiChMaster == SPI_CH0)
       {
           DmaStart(2, (uint32)Spi_TestBuffer, (uint32)SPI1_TX_FIFO_ADDR,5, &SPI1ControlDmaCfg_TX, SpiDMAIntIsr_Test);
       }
       else
       {
           DmaStart(2, (uint32)Spi_TestBuffer, (uint32)SPI0_TX_FIFO_ADDR,5, &SPI0ControlDmaCfg_TX, SpiDMAIntIsr_Test);
       }
       //SPIWriteFIFO(spiChSlave,Spi_TestBuffer,5);

       SPIWriteFIFO(spiChMaster,Spi_clockBuffer,5); //master 发送无效数据0x00，只提供clock

       SpiGetInt(spiChMaster);
       //SpiEnableTxInt(SPI_CH_TEST);
       SpiWaitIdle(spiChMaster);

       DelayMs(10);
       //读出0x55 0xaa 0x55 0xaa 0x55
       //SPIDmaRead(spiChMaster,5);
       DmaEnableInt(2);

       if(spiChMaster == SPI_CH0)
       {
           DmaStart(2, (uint32)SPI0_RX_FIFO_ADDR,(uint32)&Spi_TestBuffer_Read[10], 5, &SPI0ControlDmaCfg_RX, SpiDMAIntIsr_Test);
       }
       else
       {
           DmaStart(2,(uint32)SPI1_RX_FIFO_ADDR,(uint32)&Spi_TestBuffer_Read[10],5, &SPI1ControlDmaCfg_RX, SpiDMAIntIsr_Test);
       }

       //SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);
       if((Spi_TestBuffer_Read[10] == 0x55)
       && (Spi_TestBuffer_Read[11] == 0xaa)
       && (Spi_TestBuffer_Read[12] == 0x55)
       && (Spi_TestBuffer_Read[13] == 0xaa)
       && (Spi_TestBuffer_Read[14] == 0x55))
       {
          Spi_Testflag = 0;
       }
    }

    while(Spi_Testflag);
    //DEBUG("SPI_TRANSMIT_RECEIVE is ok");
}
/*******************************************************************************
** Name: SpiDevShellBspDMA_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:38:37
*******************************************************************************/

rk_err_t SpiDMA_Write()
{
    rk_size_t realsize;
    pSPI_REG Spi_Reg;

    uint32 i = 0;
    //DEBUG("SpiDevShellBspDMA_Write %d",SPI_CH_TEST);
    Spi_Reg = SpiGetCH(SPI_CH_TEST);
    Spi_NeedTransLen = 0;
    for(i=0;i<1024;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }

    //open uart clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst uart ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA,       (void*)DmaInt);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(2);

    SPIInit(SPI_CH_TEST,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST | TRANSMIT_ONLY);
    SpiEanbleChannel(SPI_CH_TEST,0);

    SPI_TEST_MODE = SPI_TRANSMIT_ONLY;

    if(SPI_TRANSMIT_ONLY == SPI_TEST_MODE)
    {
        //DEBUG("SPI_TRANSMIT_ONLY\n");

        Spi_Testflag = 1;
        SPIDmaWrite(SPI_CH_TEST);
        if(SPI_CH0 == SPI_CH_TEST)
        {
            DmaStart(2, (uint32)Spi_TestBuffer, (uint32)(&(Spi_Reg->SPI_TXDR)),1024, &SPI0ControlDmaCfg_TX, SpiDMAIntIsr_Test);
        }
        else
        {
            DmaStart(2, (uint32)Spi_TestBuffer, (uint32)(&(Spi_Reg->SPI_TXDR)),1024, &SPI1ControlDmaCfg_TX, SpiDMAIntIsr_Test);
        }
    }
    else
    {
       //DEBUG("SPI_RECEIVE_ONLY");

       //I2SInit(I2S_CH_TEST,I2S_SLAVE_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }


    while(1);
}
/*******************************************************************************
** Name: SpiDevShellBspPIO_ReadWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:38:05
*******************************************************************************/

rk_err_t SpiPIO_ReadWrite(uint8 * pstr)
{
    rk_size_t realsize,_RXrealsize;
    pSPI_REG Spi_Reg0,Spi_Reg1;
    uint8 Spi_clockBuffer[5] = {0,};
    eSPI_ch_t spiChMaster,spiChSlave;
    uint32 i = 0;

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        spiChMaster = SPI_CH0;
        spiChSlave = SPI_CH1;
        //DEBUG("Master-SPI0;Slave-SPI1");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        spiChMaster = SPI_CH1;
        spiChSlave = SPI_CH0;
        //DEBUG("Master-SPI1;Slave-SPI0");
    }

    Spi_NeedTransLen = 0;
    Spi_NeedTransLen_RX = 5;

    for(i=0;i<10;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }


    SPIInit(spiChMaster,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST);

    SPIInit(spiChSlave,0, 12*1000*1000, SPI_CTL_TXRX_SLAVE_TEST);

    SPIPioRead(spiChSlave,5);

    SpiEanbleChannel(spiChMaster,0);
    SPIPioWrite(spiChMaster);



    Spi_Testflag = 1;
    //写入0x55 0xaa 0x55 0xaa 0x55 发送到slave
    SPIWriteFIFO(spiChMaster,&Spi_TestBuffer[0],5);

    SpiGetInt(spiChMaster);
    //SpiEnableTxInt(SPI_CH_TEST);
    SpiWaitIdle(spiChMaster);

    //读出0x55 0xaa 0x55 0xaa 0x55
    SPIReadFIFO(spiChSlave,Spi_TestBuffer_Read,5);

    //读出master Read FIFO里面的无效数据
    SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);

    //check
    if((Spi_TestBuffer_Read[0] == 0x55)
       && (Spi_TestBuffer_Read[1] == 0xaa)
       && (Spi_TestBuffer_Read[2] == 0x55)
       && (Spi_TestBuffer_Read[3] == 0xaa)
       && (Spi_TestBuffer_Read[4] == 0x55))
    {
       //写入0x55 0xaa 0x55 0xaa 0x55 发送到master
       SPIWriteFIFO(spiChSlave,Spi_TestBuffer,5); //写入数据，需要MASTER提供clock

       SPIWriteFIFO(spiChMaster,Spi_clockBuffer,5); //master 发送无效数据0x00，只提供clock

       SpiGetInt(spiChMaster);
       //SpiEnableTxInt(SPI_CH_TEST);
       SpiWaitIdle(spiChMaster);

       DelayMs(10);
       //读出0x55 0xaa 0x55 0xaa 0x55
       SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);
       if((Spi_TestBuffer_Read[10] == 0x55)
       && (Spi_TestBuffer_Read[11] == 0xaa)
       && (Spi_TestBuffer_Read[12] == 0x55)
       && (Spi_TestBuffer_Read[13] == 0xaa)
       && (Spi_TestBuffer_Read[14] == 0x55))
       {
          Spi_Testflag = 0;
       }
    }

    while(Spi_Testflag);
    //DEBUG("SPI_TRANSMIT_RECEIVE is ok");

}
/*******************************************************************************
** Name: SpiDevShellBspPIO_Wtite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:37:26
*******************************************************************************/

rk_err_t SpiPIO_Write()
{
    rk_size_t realsize;

    uint32 i = 0;
    //DEBUG("SpiDevShellBspPIO_Write %d",SPI_CH_TEST);
    Spi_NeedTransLen = 0;

    for(i=0;i<1024;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }

    SPIInit(SPI_CH_TEST,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST | TRANSMIT_ONLY);

    if(SPI_CH0 == SPI_CH_TEST)
    {
        IntRegister(INT_ID_SPI0 ,SpiIntIsr_Test_TX);
        IntPendingClear(INT_ID_SPI0);
        IntEnable(INT_ID_SPI0);
    }
    else
    {
        IntRegister(INT_ID_SPI1 ,SpiIntIsr_Test_TX);
        IntPendingClear(INT_ID_SPI1);
        IntEnable(INT_ID_SPI1);
    }

    if(SPI_TRANSMIT_ONLY == SPI_TEST_MODE)
    {
        SpiEanbleChannel(SPI_CH_TEST,0);
        SPIPioWrite(SPI_CH_TEST);
        //DEBUG("SPI_TRANSMIT_ONLY");
        while(1)
        {
            Spi_Testflag = 1;
            realsize = SPIWriteFIFO(SPI_CH_TEST,Spi_TestBuffer,1024);
            if(realsize == 1024)
            {
                SpiWaitIdle(SPI_CH_TEST);
            }
            else
            {
                Spi_NeedTransLen += realsize;
            }

            SpiGetInt(SPI_CH_TEST);
            SpiEnableTxInt(SPI_CH_TEST);
            SpiWaitIdle(SPI_CH_TEST);

            while(Spi_Testflag);
         }
    }
    else
    {
       //DEBUG("SPI_RECEIVE_ONLY");

       //I2SInit(I2S_CH_TEST,I2S_SLAVE_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }

}
/*******************************************************************************
** Name: SpiDevShellBspDeinit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:46:22
*******************************************************************************/

rk_err_t SpiDeinit(uint8 * pstr)
{

}

/*******************************************************************************
** Name: SpiDevShellBspInit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:45:43
*******************************************************************************/

rk_err_t SpiInit(uint8 * pstr)
{

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
        SPI_CH_TEST = SPI_CH0;
        //DEBUG("SPI_TRANSMIT_ONLY 0");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
        SPI_CH_TEST = SPI_CH1;
        //DEBUG("SPI_TRANSMIT_ONLY 1");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        SPI_TEST_MODE = SPI_TRANSMIT_RECEIVE;
        //DEBUG("SPI_Read_Write");
    }
    //ScuClockGateCtr(CLOCK_GATE_I2C, 1);
    //ScuClockGateCtr(CLOCK_GATE_I2S, 1);
    //ScuClockGateCtr(CLOCK_GATE_GRF, 1);
    //I2s_Iomux_Set();
    //I2c_Iomux_Set();

    //printf("\r\nspi init over");
}

rk_err_t Spi_test(HDC dev, uint8 * pstr)
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

        case '1':  //SPI_CH0_PA
            SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
            SPI_CH_TEST = SPI_CH0;
            SPI_GPIO_Init(SPI_CH0_PA);
            //DEBUG("SPI_TRANSMIT_ONLY SPI_CH0_PA");
            ret = SpiPIO_Write();
            //ret = SpiDMA_Write();
            break;

        case '2':  //SPI_CH0_PB
            SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
            SPI_CH_TEST = SPI_CH0;
            SPI_GPIO_Init(SPI_CH0_PB);
            //DEBUG("SPI_TRANSMIT_ONLY SPI_CH0_PB");
            ret = SpiPIO_Write();
            //ret = SpiDMA_Write();
            break;
        case '3':  //SPI_CH1_PA
            SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
            SPI_CH_TEST = SPI_CH1;
            SPI_GPIO_Init(SPI_CH1_PA);
            //DEBUG("SPI_TRANSMIT_ONLY SPI_CH1_PA");
            ret = SpiPIO_Write();
            //ret = SpiDMA_Write();
            break;
        case '4':  //SPI_CH1_PB
            SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
            SPI_CH_TEST = SPI_CH1;
            SPI_GPIO_Init(SPI_CH1_PB);
            //DEBUG("SPI_TRANSMIT_ONLY SPI_CH0_PB");
            ret = SpiPIO_Write();
            //ret = SpiDMA_Write();
            break;
        case '5':  //PIO_ReadWrite
            SPI_GPIO_Init(SPI_CH0_PA);
            //SPI_GPIO_Init(SPI_CH0_PB);
            SPI_GPIO_Init(SPI_CH1_PA);
            //SPI_GPIO_Init(SPI_CH1_PB);
            ret = SpiPIO_ReadWrite(pItem);
            break;
        case '6':  //SpiDMA_ReadWrite
            SPI_GPIO_Init(SPI_CH0_PA);
            //SPI_GPIO_Init(SPI_CH0_PB);
            SPI_GPIO_Init(SPI_CH1_PA);
            //SPI_GPIO_Init(SPI_CH1_PB);
            ret = SpiDMA_ReadWrite(pItem);
            break;
        case 'I':
            //init
            break;;
        default:
            ret = RK_ERROR;
            break;
    }
    SpiTest_Menu();
    return ret;
}
#endif

