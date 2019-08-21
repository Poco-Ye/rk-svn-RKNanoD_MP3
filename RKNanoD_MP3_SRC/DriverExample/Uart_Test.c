/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   Uart_Test.c
*
* Description:
*
* History:      <author>          <time>        <version>
*
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _UART_TEST_

#include "Device.h"
#include "DriverInclude.h"

void uartTestHelpInfo()
{
    printf("\r\n");
    printf("Uart test start!\n");
    printf("================================================================================\n");
    printf(" Uart Test Menu                                                                  \n");
    printf(" 1. UART_CHN0 Test Start [0] -- port A (W/R) [1] -- port B (W/R)                     \n");
    printf(" 2. UART_CHN1 Test Start [0] -- port A (W/R/RTS/CTS) [1] -- port B (W/R/RTS/CTS)      \n");
    printf(" 3. UART_CHN2 Test Start [0] -- port A (W/R/RTS/CTS) [1] -- port B (W/R/RTS/CTS) [2] -- port C (W/R/RTS/CTS) \n");
    printf(" 4. UART_CHN3 Test Start [0] -- port A (W/R)                                       \n");
    printf(" 5. UART_CHN4 Test Start [0] -- port A (W/R)                                      \n");
    printf(" 6. UART_CHN5 Test Start [0] -- port A (W/R)                                      \n");
    printf(" 7. UART DMA write Test Start                                                    \n");
    printf(" 8. UART DMA read Test Start                                                     \n");
    printf(" h. This test help menu                                                          \n");
    printf(" 0. exit                                                                         \n");

    printf("================================================================================\n");
    printf("\r\n");

}

static char port2char(uint8 portNum)
{
    char c = 0;

    if(portNum <= 0)
        return 0;

    if(portNum % 3 == 0)
        c = 'A';
    else if(portNum % 3 == 1)
        c = 'B';
    else if(portNum % 3 == 2)
        c = 'C';

    return c;
}

//******************************************************************************
//uart chn 0 & 1 function test,hardware to FT232RL

void UartChn01Test(eUART_CH uartChn,uint8* pstr)
{
    int32 ret;
    uint8  Char[16];
    uint8 *pChar = Char;
    int index = 0;

    uint32 uartPort;

    printf("Uart test start, please enter keys start test!\n");
    printf("The [ENTER] key Exit uart test!\n");
    DelayMs(10);

    uartPort = uartChn * 3;
    if (0 == StrCmpA(pstr, "0", 1))     //port A
    {
        printf("chn %d port %c\n",uartChn,port2char(uartPort));
        DelayMs(50);
        ret = UARTInit(uartPort, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    }
    else if (0 == StrCmpA(pstr, "1", 1))//port B    
    {
        printf("chn %d port %c\n",uartChn,port2char(uartPort+1));
        DelayMs(50);
        ret = UARTInit(uartPort+1, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    }
//    else if (0 == StrCmpA(pstr, "2", 1) )//port C 
//    {
//        printf("chn %d port %c\n",uartChn,port2char(uartPort+2));
//        DelayMs(50);
//        ret = UARTInit(uartPort+2, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
//    }

    if (ret != OK)
        return;

    while (1)
    {
        UARTSetIntEnabled(uartChn,UART_IE_RX);
        ret = UARTReadByte(uartChn,pChar, 1);
        if (ret > 0)
        {
            if (*pChar == 0x0d || *pChar == 'q' || *pChar == 'Q')
            {
                break;
            }
            UARTWriteByte(uartChn,pChar, 1);
        }
    }
    printf("\n");
}

/* uart1 B   uart2 (A B C)
**  Rx   --->   Tx
**  Tx   --->   Rx
**  CTS   --->  RTS
**  RTS   --->  CTS
*/

void UartChn2Test(eUART_CH uartChn,uint8* pstr)
{
    int32 ret;
    uint8 *Char = "abcdefghijklmnoprstABCDEFG";
    uint8 recv[128],recv2[128];
    uint8 *pSend = Char;
    uint8 *pRev = recv;
    uint8 *pRev2 = recv2;
    int32 size,index = 0;
    uint32 status;
    uint32 uartPort;

    index = size = StrLenA(Char);
    memset(recv,0,sizeof(recv));
    memset(recv2,0,sizeof(recv2));
    
    uartPort = uartChn * 3;
    if (0 == StrCmpA(pstr, "0", 1))     //port A
    {
        printf("chn %d port %c\n",uartChn,port2char(uartPort));
        DelayMs(50);
        ret = UARTInit(uartPort, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    }
    else if (0 == StrCmpA(pstr, "1", 1))//port B    
    {
        printf("chn %d port %c\n",uartChn,port2char(uartPort+1));
        DelayMs(50);
        ret = UARTInit(uartPort+1, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    }
    else if (0 == StrCmpA(pstr, "2", 1) )//port C 
    {
        printf("chn %d port %c\n",uartChn,port2char(uartPort+2));
        DelayMs(50);
        ret = UARTInit(uartPort+2, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    }

    UARTInit(UART_CH1_PB, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);

    //set uart 2 auto RTS
    UartEnableFlowControl(uartChn);
    UartSetAutoRTS(uartChn);

//    //set uart 1 auto RTS
//    UartEnableFlowControl(UART_CH1);
//    UartSetAutoRTS(UART_CH1);    

    while(1)
    {
        UARTSetIntEnabled(UART_CH1,UART_IE_RX);
        UARTSetIntEnabled(uartChn,UART_IE_RX);

        UARTWriteByte(UART_CH1,pSend, size);

        //get uart1 cts status,if 0 means cts_n input is de-asserted (logic 1)
        //if 1 means cts_n input is asserted (logic 0)
        status = UartGetCTSState(uartChn);
        while(status == 0);

        if( status == 1)
        {
            printf("Chn 1 receive rts,RTS sign asserted,send Stop.\n");
        }


        ret = 0;
        while(ret == 0)
        {
            ret = UARTReadByte(uartChn,pRev, size);
        }

        printf("read from chn %d:%s",uartChn,pRev);
        
        DelayMs(50);
        DEBUG("ret = %d",ret);
        if (ret > 0)
        {
            int r_num = 0;
            int w_num = 0;
            
            w_num = UARTWriteByte(uartChn,pRev, size);
            DEBUG("w_num = %d \n",w_num); 

            while(r_num == 0)
            {
                r_num = UARTReadByte(UART_CH1,pRev2, size);
            }

            DEBUG("r_num = %d \n",r_num);
            if(r_num > 0)
            {
                printf("%s \n",pRev2);
            }
        }
    }
    printf("\n"); 
}

void UartChn3Test(eUART_CH uartChn,uint8* pstr)
{
    int32 ret;
    uint8 *Char = "abcdefghijklmnoprstABCDEFG";
    uint8 recv[128],recv2[128];
    uint8 *pSend = Char;
    uint8 *pRev = recv;
    uint8 *pRev2 = recv2;
    int32 size = 0;
    uint32 uartPort;

    size = StrLenA(Char);
    uartPort = uartChn * 3;
    printf("chn %d port %c\n",uartChn,port2char(uartPort));
    
    UARTInit(UART_CH1_PB, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    UARTInit(UART_CH3_PA, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);

    while(1)
    {
        UARTSetIntEnabled(UART_CH1,UART_IE_RX);
        UARTSetIntEnabled(uartChn,UART_IE_RX);

        ret = UARTWriteByte(UART_CH1,pSend, size);
        DEBUG("ret = %d",ret);
        
        ret = 0;
        while(ret == 0)
        {
            ret = UARTReadByte(uartChn,pRev, size);
        }

        DEBUG("ret = %d",ret);
        if (ret > 0)
        {
            int r_num = 0;
            int w_num = 0;
            w_num = UARTWriteByte(uartChn,pRev, size);
            DEBUG("w_num = %d",w_num);

            while(!r_num)
            {
                r_num = UARTReadByte(UART_CH1,pRev2, size);
            }

            DEBUG("r_num = %d",r_num);
            if(r_num > 0)
            {
                printf("%s",pRev2);
            }
        }
    }
    printf("\n");         
}

void UartChn4Test(eUART_CH uartChn,uint8* pstr)
{
    int32 ret;
    uint8 *Char = "abcdefghijklmnoprstABCDEFG";
    uint8 recv[128],recv2[128];
    uint8 *pSend = Char;
    uint8 *pRev = recv;
    uint8 *pRev2 = recv2;
    int32 size = 0;
    uint32 uartPort;

    size = StrLenA(Char);
    uartPort = uartChn * 3;
    printf("chn %d port %c\n",uartChn,port2char(uartPort));
    
    UARTInit(UART_CH1_PB, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    UARTInit(UART_CH4_PA, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);

    while(1)
    {
        UARTSetIntEnabled(UART_CH1,UART_IE_RX);
        UARTSetIntEnabled(uartChn,UART_IE_RX);

        ret = UARTWriteByte(UART_CH1,pSend, size); 
        DEBUG("ret = %d",ret);
        
        ret = 0;
        while(ret == 0)
        {
            ret = UARTReadByte(uartChn,pRev, size);
        }
        
        if (ret > 0)
        {
            int r_num = 0;
            int w_num = 0;
            w_num = UARTWriteByte(uartChn,pRev, size);
            DEBUG("w_num = %d",w_num);

            while(!r_num )
            {
                r_num = UARTReadByte(UART_CH1,pRev2, size);
            }

            DEBUG("r_num = %d",r_num);
            if(r_num > 0)
            {
                printf("%s",pRev2);
            }
        }
    }
    printf("\n");   
}

void UartChn5Test(eUART_CH uartChn,uint8* pstr)
{
    int32 ret;
    uint8 *Char = "abcdefghijklmnoprstABCDEFG";
    uint8 recv[128],recv2[128];
    uint8 *pSend = Char;
    uint8 *pRev = recv;
    uint8 *pRev2 = recv2;
    int32 size = 0;
    uint32 uartPort;

    size = StrLenA(Char);
    uartPort = uartChn * 3;
    printf("chn %d port %c\n",uartChn,port2char(uartPort));
    
    UARTInit(UART_CH1_PB, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    UARTInit(UART_CH5_PA, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);

    while(1)
    {
        UARTSetIntEnabled(UART_CH1,UART_IE_RX);
        UARTSetIntEnabled(uartChn,UART_IE_RX);

        ret = UARTWriteByte(UART_CH1,pSend, size); 
        DEBUG("ret = %d",ret);   

        ret = 0;
        while(ret == 0)
        {
            ret = UARTReadByte(uartChn,pRev, size);
        }
        
        if (ret > 0)
        {
            int r_num = 0;
            int w_num = 0;
            
            w_num = UARTWriteByte(uartChn,pRev, size);
            DEBUG("w_num = %d",w_num);

            while(!r_num)
            {
                r_num = UARTReadByte(UART_CH1,pRev2, size);
            }

            DEBUG("r_num = %d",r_num);
            if(r_num > 0)
            {
                printf("%s",pRev2);
            }
        }
    }
    printf("\n");      
}


void uartDmaWriteCallback()
{
    printf("uart dma write OK! \n");
}

void uartDmaReadCallback()
{
    printf("uart dma read OK! \n");
}

void UartDmaWriteTest()
{
    uint32 uartPort;
    uint8 testData[16] = {'A','B','C','D','a','b','c','d','A','B','C','D','a','b','c','d'};

    //open  clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA,       (void*)DmaInt);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    uartPort = UART_CH1_PA;

    UARTInit(uartPort, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);

    DmaEnableInt(2);

    UartDmaWrite(2, uartPort, testData, sizeof(testData) , /*uartDmaWriteCallback*/NULL);

    DelayMs(10);
}

void UartDmaReadTest()
{
    uint32 uartPort;
    uint8 testData;

    //open  clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA,       (void*)DmaInt);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    uartPort = UART_CH1_PA;

    UARTInit(uartPort, UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);

    DmaEnableInt(2);

    while (1)
    {
        UARTSetIntEnabled(uartPort/3,UART_IE_RX);
        UartDmaRead(2, uartPort, &testData, 1, NULL/*uartDmaReadCallback*/);

        if (testData == 'q' || testData == 'Q' || testData == 0x0d)
            break;

        UARTWriteByte(uartPort/3,(uint8*)&testData,1);
    }
}


rk_err_t UartTestCmdParse(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint32 cmd;
    StrCnt = ShellItemExtract(pstr,&pItem);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }
    cmd = pstr[0];

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    switch (cmd)
    {
        case '0': //folder_once
            return RK_EXIT;

        case 'h':
            uartTestHelpInfo();
            break;

        case '1':
            UartChn01Test(UART_CH0,pItem);
            break;
        case '2':
            UartChn01Test(UART_CH1,pItem);
            break;
        case '3':
            UartChn2Test(UART_CH2,pItem);
            break;
        case '4':
            UartChn3Test(UART_CH3,pItem);
            break;
        case '5':
            UartChn4Test(UART_CH4,pItem);
            break;
        case '6':
            UartChn5Test(UART_CH5,pItem);
            break;

        case '7':
            UartDmaWriteTest();
            break;

        case '8':
            UartDmaReadTest();
            break;

        case 'I':
            break;

        default:
            break;

    }

    DelayMs(10);
    uartTestHelpInfo();
    return ret;
}

//******************************************************************************
#endif

