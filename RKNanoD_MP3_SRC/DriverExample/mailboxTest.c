/*
********************************************************************************
*
*                Copyright (c): 2015 - 2015 + 5, mailboxTest.c
*                             All rights reserved.
*
* FileName: ..\DriverExample\mailboxTest.c
* Owner: mailboxTest.c
* Date: 2015.3.2
* Time: 9:07:17
* Desc: 
* History:
*   <author>    <date>       <time>     <version>     <Desc>
*   
********************************************************************************
*/
/*
*-------------------------------------------------------------------------------
*
*                             #include define                                   
*
*-------------------------------------------------------------------------------

*/
#include "sysinclude.h"

#ifdef _MAILBOX_TEST_

#include "Device.h"	
#include "DriverInclude.h"

void mailBoxTestPrintfInfo(void)
{
    rk_print_stringA("\r\n");
    rk_print_stringA("================================================================================\n");
    rk_print_stringA(" mailBox Test Menu                                                              \n");
    rk_print_stringA(" 1. mailBox A To B  and B TO A Channel 0                                        \n");
    rk_print_stringA(" 2. mailBox A To B  and B TO A Channel 1                                        \n");
    rk_print_stringA(" 3. mailBox A To B  and B TO A Channel 2                                        \n");
    rk_print_stringA(" 4. mailBox A To B  and B TO A Channel 3                                        \n");
    rk_print_stringA(" 5. Core A Use DMA write data to core B memory                                  \n"); 
    rk_print_stringA(" 6. Core A Use DMA read  data from core B memory                                \n"); 
    rk_print_stringA(" 7. Core B Use DMA write data to core A memory                                  \n");   
    rk_print_stringA(" 8. Core B Use DMA read data to core A memory                                   \n");
    rk_print_stringA(" 0. Exit                                                                        \n");
    rk_print_stringA("================================================================================\n");
    rk_print_stringA("\r\n");
}

void MailBoxA2BIsr_0()
{
    uint32 cmd;
    uint32 data;
    

    cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_0);

    MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_0));

    
    printf("B to A channle 0: cmd = %d, data = %d\r\n",cmd, data);
    
}


void MailBoxA2BIsr_1()
{
    uint32 cmd;
    uint32 data;
    

    cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_1));
    printf("B to A channle 1: cmd = %d, data = %d\r\n",cmd, data);
    
}


void MailBoxA2BIsr_2()
{
    uint32 cmd;
    uint32 data;
    

    cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_2));
    printf("B to A channle 2: cmd = %d, data = %d\r\n",cmd, data);     
}


void MailBoxA2BIsr_3()
{
    uint32 cmd;
    uint32 data;
    

    cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_3);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_3);

    MailBoxClearB2AInt(MAILBOX_ID_0, (uint32)(1 << MAILBOX_CHANNEL_3));
    printf("B to A channle 3: cmd = %d, data = %d\r\n",cmd, data);
    
}


DMA_CFGX M2MDmaCfg  = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD,0};

uint8 memDmabuf[1024];
int  memDmach = 0;
int  memDmafinish = 0;

extern uint8 BcoreDmabuf[1024];
void M2M_DmaIsr(void)
{
    printf("dma complete\n");

    memDmafinish = 1;

}

rk_err_t mailBoxTestCmdParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint32 j = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint32 cmd;
    StrCnt = ShellItemExtract(pstr,&pItem);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }    

//    ret = ShellCheckCmd(ShellRepeatName, pItem, StrCnt);
//    if(ret < 0)
//    {
//        return RK_ERROR;
//    }

//    i = (uint32)ret;
    cmd = pstr[0];

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    switch (cmd)
    {

        case 'I':
            ret = RK_SUCCESS;
            break;

        case 'J': // init cmd ,to be init the dev

            IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0, MailBoxA2BIsr_0);
            IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
            IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
            MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_0));

            IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1, MailBoxA2BIsr_1);
            IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
            IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
            MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_1));


            IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2, MailBoxA2BIsr_2);
            IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
            IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
            MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_2));


            IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3, MailBoxA2BIsr_3);
            IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
            IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
            MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_3));
            ScuSoftResetCtr(CAL_CORE_SRST, TRUE);
            DelayMs(1000);
            ScuSoftResetCtr(CAL_CORE_SRST, FALSE);

            //init the dev
            //.......

            break;
            
        case '0': //end test

            ret =  RK_EXIT;           
           return ret;
            
        case '1':

            printf("channle 0 test start\n");

            MailBoxWriteA2BCmd(11, 0,MAILBOX_CHANNEL_0);
            MailBoxWriteA2BData(22, 0, MAILBOX_CHANNEL_0);
            DelayMs(1000);
            break;

        case '2':
            printf("channle 1 test start\n");
            MailBoxWriteA2BCmd(33, 0,MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(44, 0, MAILBOX_CHANNEL_1);
            DelayMs(1000);
            break;

        case '3':
            printf("channle 2 test start\n");
            MailBoxWriteA2BCmd(55, 0,MAILBOX_CHANNEL_2);
            MailBoxWriteA2BData(66, 0, MAILBOX_CHANNEL_2);
            DelayMs(1000);
            break;
       case '4':
            printf("channle 3 test start\n");
            MailBoxWriteA2BCmd(77, 0,MAILBOX_CHANNEL_3);
            MailBoxWriteA2BData(99, 0, MAILBOX_CHANNEL_3);
            DelayMs(1000);
            break;   

        case '5':
           ScuClockGateCtr(HCLK_DMA_GATE, 1);

           ScuSoftResetCtr(SYSDMA_SRST, 1);
           DelayMs(1);
           ScuSoftResetCtr(SYSDMA_SRST, 0);
           
           IntRegister(INT_ID_DMA,       (void*)DmaInt);
           
           IntPendingClear(INT_ID_DMA);
           IntEnable(INT_ID_DMA);
         
           for(j=0; j< 6; j++)
           {
               printf("A core Dma write test ch = %d\n",j);
               DmaEnableInt(j);
               memDmafinish = 0;
               memset(memDmabuf, 0x55, 1024);
               memset(BcoreDmabuf, 0, 1024);
               DmaStart(j,(uint32)memDmabuf,(uint32)BcoreDmabuf,256, &M2MDmaCfg, M2M_DmaIsr);
               while(!memDmafinish)
               {
               }
               for(i=0; i< 1024; i++)
               {
                   if(BcoreDmabuf[i] != 0x55)
                   {
                        printf("A core Dma write test ch =%d fail\n" , j);
                        return ret;
                   }
               }
               if(i == 1024)
               {
                    printf("A core Dma write test ch =%d OK\n" , j);
               }
           }
        break; 

        case '6':
            
           ScuClockGateCtr(HCLK_DMA_GATE, 1);

           ScuSoftResetCtr(SYSDMA_SRST, 1);
           DelayMs(1);
           ScuSoftResetCtr(SYSDMA_SRST, 0);
           
           IntRegister(INT_ID_DMA,       (void*)DmaInt);
           
           IntPendingClear(INT_ID_DMA);
           IntEnable(INT_ID_DMA);
          
           for(j=0; j< 6; j++)
           {  
               printf("A core Dma read test = %d\n",j);
               DmaEnableInt(j);

               memDmafinish = 0;
               memset(BcoreDmabuf, 66, 1024);
               memset(memDmabuf, 0, 1024);
               DmaStart(j,(uint32)BcoreDmabuf,(uint32)memDmabuf,256, &M2MDmaCfg, M2M_DmaIsr);

               while(!memDmafinish)
               {
               }

               for(i=0; i< 1024; i++)
               {
                   if(memDmabuf[i] != 66)
                   {
                        printf("A core Dma read test ch =%d fail\n" , j);

                        return ret;
                   }
               }
               if(i == 1024)
               {
                    printf("A core Dma read test ch =%d Test OK\n" , j);
               }
           }
        break;


        case '7':

        printf("B core Dma write test\n");
        MailBoxWriteA2BCmd(74, 0,MAILBOX_CHANNEL_3);
        MailBoxWriteA2BData(1, 0, MAILBOX_CHANNEL_3);
        break;

        
        case '8':

        printf("B core Dma read test\n");
        MailBoxWriteA2BCmd(74, 0,MAILBOX_CHANNEL_3);
        MailBoxWriteA2BData(2, 0, MAILBOX_CHANNEL_3);

        break;  
        case 'h':
        
            mailBoxTestPrintfInfo();
            break;
            
        default:
            ret = RK_ERROR;
            break;
			
    }
	mailBoxTestPrintfInfo();
    return ret;

}
#endif

