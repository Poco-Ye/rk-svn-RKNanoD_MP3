/*
********************************************************************************
*                   Copyright (c) 2015,wangping
*                         All rights reserved.
*
* File Name£º   main2Test.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               wangping      2015-3-3          1.0
*    desc:    ORG.
********************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "SysInclude.h"

#ifdef _MAILBOX_TEST_

#include "mailbox.h"

enum B_CORE_MAIL_BOX_STATUS
{
    MAIL_BOX_STATUS_IDLE = 0,
    MAIL_BOX_STATUS_CHANNEL_0_INT,
    MAIL_BOX_STATUS_CHANNEL_1_INT,
    MAIL_BOX_STATUS_CHANNEL_2_INT,
    MAIL_BOX_STATUS_CHANNEL_3_INT
};
_ATTR_BB_SYS_DATA_
uint BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;
_ATTR_BB_SYS_DATA_
uint BcoreMailBoxCmd = 0;
_ATTR_BB_SYS_DATA_
uint BcoreMailBoxData = 0;

extern uint8 memDmabuf[1024];
uint8 BcoreDmabuf[1024];

uint8 BcoreDmaCh = 0;
void MailBoxBIsr_0(void)
{
    uint32 cmd;
    uint32 data;
    MailBoxClearA2BInt(0, (uint32)(1 << MAILBOX_CHANNEL_0));

    cmd = MailBoxReadA2BCmd(0, MAILBOX_CHANNEL_0);
    data = MailBoxReadA2BData(0, MAILBOX_CHANNEL_0);   


    BcoreMailBoxCmd = cmd;
    BcoreMailBoxData = data;

    BcoreMailBoxstatus = MAIL_BOX_STATUS_CHANNEL_0_INT;
    
}

void MailBoxBIsr_1(void)
{
    uint32 cmd;
    uint32 data;
    MailBoxClearA2BInt(0, (uint32)(1 << MAILBOX_CHANNEL_1));

    cmd = MailBoxReadA2BCmd(0, MAILBOX_CHANNEL_1);
    data = MailBoxReadA2BData(0, MAILBOX_CHANNEL_1);      

    BcoreMailBoxCmd = cmd;
    BcoreMailBoxData = data;

    BcoreMailBoxstatus = MAIL_BOX_STATUS_CHANNEL_1_INT;
}


void MailBoxBIsr_2(void)
{
    uint32 cmd;
    uint32 data;
    MailBoxClearA2BInt(0, (uint32)(1 << MAILBOX_CHANNEL_2));

    cmd = MailBoxReadA2BCmd(0, MAILBOX_CHANNEL_2);
    data = MailBoxReadA2BData(0, MAILBOX_CHANNEL_2);

    BcoreMailBoxCmd = cmd;
    BcoreMailBoxData = data;

    BcoreMailBoxstatus = MAIL_BOX_STATUS_CHANNEL_2_INT;
}

void MailBoxBIsr_3(void)
{
    uint32 cmd;
    uint32 data;
    MailBoxClearA2BInt(0, (uint32)(1 << MAILBOX_CHANNEL_3));

    cmd = MailBoxReadA2BCmd(0, MAILBOX_CHANNEL_3);
    data = MailBoxReadA2BData(0, MAILBOX_CHANNEL_3);

    BcoreMailBoxCmd = cmd;
    BcoreMailBoxData = data;

    BcoreMailBoxstatus = MAIL_BOX_STATUS_CHANNEL_3_INT;
}

_ATTR_BB_SYS_DATA_
uint BcoreDmaFinish = 0;



void CoreBDmaIsr(void)
{
    BcoreDmaFinish = 1;

    DmaDisableInt2(BcoreDmaCh); 
}

void CoreBDmaIsr_ch0(void)
{
    BcoreDmaFinish = 1;

    DmaDisableInt2(0); 
}

void CoreBDmaIsr_ch1(void)
{

    BcoreDmaFinish = 1;
    DmaDisableInt2(1); 
}


int Main2Test(void)
{
    uint outptr;
    uint OutLength;
//    UART_DEV_ARG stUartArg;	
    //UARTInit(1,UART_BR_115200, UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);  

    BSP_Init2();
	IntRegister2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0, MailBoxBIsr_0);
    IntPendingClear2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
    IntEnable2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
    MailBoxEnableA2BInt(0, (int32)(1 << MAILBOX_CHANNEL_0));

    IntRegister2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1, MailBoxBIsr_1);
    IntPendingClear2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
    IntEnable2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
    MailBoxEnableA2BInt(0, (int32)(1 << MAILBOX_CHANNEL_1));

    IntRegister2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2, MailBoxBIsr_2);
    IntPendingClear2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
    IntEnable2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
    MailBoxEnableA2BInt(0, (int32)(1 << MAILBOX_CHANNEL_2));

    IntRegister2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3, MailBoxBIsr_3);
    IntPendingClear2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
    IntEnable2(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
    MailBoxEnableA2BInt(0, (int32)(1 << MAILBOX_CHANNEL_3));

    //UARTWriteByte0(1,"bbcore test\n", 11);
    bb_printf1("bb core work start\n");

 	while(1)
	{
        switch(BcoreMailBoxstatus)
        {
            case MAIL_BOX_STATUS_CHANNEL_0_INT:

            bb_printf1("channle 0 : cmd = %d, data =%d\n", BcoreMailBoxCmd, BcoreMailBoxData);
            MailBoxWriteB2ACmd(1, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
            MailBoxWriteB2AData(2, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
            

            BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;
            break;

            case MAIL_BOX_STATUS_CHANNEL_1_INT:
            bb_printf1("channle 1 : cmd = %d, data =%d\n", BcoreMailBoxCmd, BcoreMailBoxData);   
            MailBoxWriteB2ACmd(3, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteB2AData(4, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            

            BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;
            break;

            case MAIL_BOX_STATUS_CHANNEL_2_INT:
            bb_printf1("channle 2 : cmd = %d, data =%d\n", BcoreMailBoxCmd, BcoreMailBoxData);    
            MailBoxWriteB2ACmd(5, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            MailBoxWriteB2AData(6, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            
            BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;
            break;

            case MAIL_BOX_STATUS_CHANNEL_3_INT:
            bb_printf1("channle 3 : cmd = %d, data =%d\n", BcoreMailBoxCmd, BcoreMailBoxData);   


            if(BcoreMailBoxCmd == 74)
            {
                int i,j;
                DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD,0}; 
                DelayMs2(50);

                if(BcoreMailBoxData == 1)
                {

                    ScuClockGateCtr(HCLK_HIGH_DMA_GATE , TRUE);
                    ScuSoftResetCtr(DMA2_SRST, 1);
                    DelayMs2(1);
                    ScuSoftResetCtr(DMA2_SRST, 0);
                    

                    IntRegister2(INT_ID_DMA2,       CoreBDmaIsr);
                    IntPendingClear2(INT_ID_DMA2);
                    IntEnable2(INT_ID_DMA2);


                    for(i=0;i<2;i++)
                    {
                        BcoreDmaCh = i;
                        DmaEnableInt2(i);

                        MemSet2(BcoreDmabuf, 0x66,1024);
                        MemSet2(memDmabuf, 0,1024);
                        BcoreDmaFinish = 0;
                        DmaConfig2(i, (uint32)BcoreDmabuf, (uint32)memDmabuf,256, &DmaCfg, NULL);       
                        bb_printf1("Core B Dma start\n"); 
                        while(!BcoreDmaFinish)
                        {
                        }
                        bb_printf1("Core B Dma Finish\n");   
                        BcoreDmaFinish = 0;

                        for(j=0;j<1024;j++)
                        {
                            if(memDmabuf[j] != 0x66)
                            {
                                bb_printf1("Core B Dma Write Fail ch = %d,id = %d\n",i,j);

                                BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;

                                break;
                            }
                        }

                        if(j == 1024)
                        {
                            bb_printf1("Core B Dma Write OK ch = %d\n",i);
                        }
                        
                    }

                    
                }

                if(BcoreMailBoxData == 2)
                {
                    ScuClockGateCtr(HCLK_HIGH_DMA_GATE , TRUE);
                    ScuSoftResetCtr(DMA2_SRST, 1);
                    DelayMs2(1);
                    ScuSoftResetCtr(DMA2_SRST, 0);
                    
                    
                    IntRegister2(INT_ID_DMA2,       CoreBDmaIsr);
                    IntPendingClear2(INT_ID_DMA2);
                    IntEnable2(INT_ID_DMA2);

                    for(i=0;i<2;i++)
                    {
                        BcoreDmaCh = i;
                        DmaEnableInt2(i);

                        MemSet2(BcoreDmabuf, 0,1024);
                        MemSet2(memDmabuf, 66,1024);
                        BcoreDmaFinish = 0;
                        DmaConfig2(i,  (uint32)memDmabuf,(uint32)BcoreDmabuf,256, &DmaCfg, NULL);       
                        bb_printf1("Core B Dma start\n"); 
                        while(!BcoreDmaFinish)
                        {
                        }
                        bb_printf1("Core B Dma Finish\n");   
                        BcoreDmaFinish = 0;

                        for(j=0;j<1024;j++)
                        {
                            if(BcoreDmabuf[j] != 66)
                            {
                                bb_printf1("Core B Dma read Fail ch = %d,id = %d\n",i,j);

                                BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;

                                break;
                            }
                        }

                        if(j == 1024)
                        {
                            bb_printf1("Core B Dma read OK ch = %d\n",i);
                        }
                        
                    }
                }

                BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;

                break;
            }
            
            MailBoxWriteB2ACmd(7, MAILBOX_ID_0, MAILBOX_CHANNEL_3);
            MailBoxWriteB2AData(9, MAILBOX_ID_0, MAILBOX_CHANNEL_3);

            BcoreMailBoxstatus = MAIL_BOX_STATUS_IDLE;
            break;

            default:

            break;
            
        }
        //__WFI2();
	}
	
}
#endif

