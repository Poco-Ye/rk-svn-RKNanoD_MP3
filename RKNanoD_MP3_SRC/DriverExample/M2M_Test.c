/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   M2M_Test.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                                           
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _M2M_TEST_

#include "Device.h"	
#include "DriverInclude.h"
#include "Interrupt.h"

uint32 IDRAM_DMA_CH;
uint32 IDRAM_DmaFinish;

void IDRAM_DmaIsr()
{
     IDRAM_DmaFinish = 1;   
     printf("\r\n IDRAM_DmaIsr\n");         
     DmaDisableInt(IDRAM_DMA_CH); 
}
DMA_LLP TEST_llpListn[DMA_CHN_MAX][50];

/*******************************************************************************
** Name: MemoryTest_Menu
** Input:void
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/
void MemoryTest_Menu(void)
{
    printf("================================================================================\n");
    printf(" Memory Test Menu                                                               \n");
    printf(" 1. Memory Test IRAM Read Write                                                 \n");
    printf(" 2. Memory Test DRAM Read Write                                                 \n");
    printf(" 3. Memory Test HIRAM Read Write                                                \n");
    printf(" 4. Memory Test HDRAM Read Write                                                \n");
    printf(" 5. Memory Test PMUSRAM Read Write                                              \n");
    printf(" 6. Memory Test HIRAM HDRAM DMARW                                               \n");
    printf(" 7. Memory Test DMA_LLP Read Write                                              \n");
    printf(" h. Memory Test Menu                                                            \n");
    printf(" 0. EXIT                                                                        \n");
    printf("================================================================================\n");    
}
/*******************************************************************************
** Name: ShellBspMemoryTest_DMA_LLP_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/

rk_err_t MemoryTest_DMA_LLP_RW(uint8 * pstr)
{
    uint32 *pbuf_LLP_0,*pbuf_LLP_1,*pbuf_LLP_2,
           *pbuf_LLP_3,*pbuf_LLP_4,*pbuf_LLP_5,
           *pbuf_LLP_6,*pbuf_LLP_7,*pbuf_LLP_8,
           *pbuf_LLP_9;
    uint32 *pbuf1_LLP_0,*pbuf1_LLP_1,*pbuf1_LLP_2,
           *pbuf1_LLP_3,*pbuf1_LLP_4,*pbuf1_LLP_5,
           *pbuf1_LLP_6,*pbuf1_LLP_7,*pbuf1_LLP_8,
           *pbuf1_LLP_9;
    uint32 i = 0;
    uint32 f1,f2,f3,f4;
    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};
    
    #define H_IRAM_ADDR 0x01000000
    #define H_IRAM_SIZE 0x20000

    #define H_DRAM_ADDR 0x01020000
    #define H_DRAM_SIZE 0x40000
    
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        IDRAM_DMA_CH = 0;
        printf("\r\n IDRAM_DMA_CH start 0\n");    
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        IDRAM_DMA_CH = 1;
        printf("\r\n IDRAM_DMA_CH start 1\n");    
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        IDRAM_DMA_CH = 2;
        printf("\r\n IDRAM_DMA_CH start 2\n");    
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        IDRAM_DMA_CH = 3;
        printf("\r\n IDRAM_DMA_CH start 3\n");    
    }
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        IDRAM_DMA_CH = 4;
        printf("\r\n IDRAM_DMA_CH start 4\n");    
    }
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        IDRAM_DMA_CH = 5;
        printf("\r\n IDRAM_DMA_CH start 5\n");    
    }
    
    pbuf_LLP_0 = (uint32 *)(0x01000000);
    pbuf_LLP_1 = (uint32 *)(0x01000500);
    pbuf_LLP_2 = (uint32 *)(0x01001000);
    pbuf_LLP_3 = (uint32 *)(0x01001500);
    pbuf_LLP_4 = (uint32 *)(0x01002000);
    pbuf_LLP_5 = (uint32 *)(0x01002500);
    pbuf_LLP_6 = (uint32 *)(0x01003000);
    pbuf_LLP_7 = (uint32 *)(0x01003500);
    pbuf_LLP_8 = (uint32 *)(0x01004000);
    pbuf_LLP_9 = (uint32 *)(0x01004500);

    pbuf1_LLP_0 = (uint32 *)0x01020000;
    pbuf1_LLP_1 = (uint32 *)0x01020500;
    pbuf1_LLP_2 = (uint32 *)0x01021000;
    pbuf1_LLP_3 = (uint32 *)0x01021500;
    pbuf1_LLP_4 = (uint32 *)0x01022000;
    pbuf1_LLP_5 = (uint32 *)0x01022500;
    pbuf1_LLP_6 = (uint32 *)0x01023000;
    pbuf1_LLP_7 = (uint32 *)0x01023500;
    pbuf1_LLP_8 = (uint32 *)0x01024000;
    pbuf1_LLP_9 = (uint32 *)0x01024500; 
   
    
    //open uart clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst uart ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA ,IDRAM_DmaIsr);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
    
    DmaEnableInt(IDRAM_DMA_CH);
    IDRAM_DmaFinish = 0;
    

    f1 = f3 = 0x55aa55aa;
    f2 = f4 = 0xaa55aa55; 

    for(i = 0; i < 0x100; i++)
    {
        *pbuf_LLP_0++ = f1;
        *pbuf_LLP_1++ = f1;
        *pbuf_LLP_2++ = f1;
        *pbuf_LLP_3++ = f1;
        *pbuf_LLP_4++ = f1;
        *pbuf_LLP_5++ = f1;
        *pbuf_LLP_6++ = f1;
        *pbuf_LLP_7++ = f1;
        *pbuf_LLP_8++ = f1;
        *pbuf_LLP_9++ = f1;

        /*
        *pbuf1_LLP_0++ = 0;
        *pbuf1_LLP_1++ = 0;
        *pbuf1_LLP_2++ = 0;
        *pbuf1_LLP_3++ = 0;
        *pbuf1_LLP_4++ = 0;
        *pbuf1_LLP_5++ = 0;
        *pbuf1_LLP_6++ = 0;
        *pbuf1_LLP_7++ = 0;
        *pbuf1_LLP_8++ = 0;
        *pbuf1_LLP_9++ = 0;
        */
    }    

    TEST_llpListn[IDRAM_DMA_CH][0].SAR = 0x01000000;
    TEST_llpListn[IDRAM_DMA_CH][0].DAR = 0x01020000;
    TEST_llpListn[IDRAM_DMA_CH][0].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][1].SAR = 0x01000500;
    TEST_llpListn[IDRAM_DMA_CH][1].DAR = 0x01020500;
    TEST_llpListn[IDRAM_DMA_CH][1].SIZE = 0x100; 
    
    TEST_llpListn[IDRAM_DMA_CH][2].SAR = 0x01001000;
    TEST_llpListn[IDRAM_DMA_CH][2].DAR = 0x01021000;
    TEST_llpListn[IDRAM_DMA_CH][2].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][3].SAR = 0x01001500;
    TEST_llpListn[IDRAM_DMA_CH][3].DAR = 0x01021500;
    TEST_llpListn[IDRAM_DMA_CH][3].SIZE = 0x100;
    
    TEST_llpListn[IDRAM_DMA_CH][4].SAR = 0x01002000;
    TEST_llpListn[IDRAM_DMA_CH][4].DAR = 0x01022000;
    TEST_llpListn[IDRAM_DMA_CH][4].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][5].SAR = 0x01002500;
    TEST_llpListn[IDRAM_DMA_CH][5].DAR = 0x01022500;
    TEST_llpListn[IDRAM_DMA_CH][5].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][6].SAR = 0x01003000;
    TEST_llpListn[IDRAM_DMA_CH][6].DAR = 0x01023000;
    TEST_llpListn[IDRAM_DMA_CH][6].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][7].SAR = 0x01003500;
    TEST_llpListn[IDRAM_DMA_CH][7].DAR = 0x01023500;
    TEST_llpListn[IDRAM_DMA_CH][7].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][8].SAR = 0x01004000;
    TEST_llpListn[IDRAM_DMA_CH][8].DAR = 0x01024000;
    TEST_llpListn[IDRAM_DMA_CH][8].SIZE = 0x100;

    TEST_llpListn[IDRAM_DMA_CH][9].SAR = 0x01004500;
    TEST_llpListn[IDRAM_DMA_CH][9].DAR = 0x01024500;
    TEST_llpListn[IDRAM_DMA_CH][9].SIZE = 0x100;

    DmaConfig_for_LLP(IDRAM_DMA_CH, 0x100, 10,&DmaCfg, TEST_llpListn[IDRAM_DMA_CH]);
		while(!IDRAM_DmaFinish);
        

    for(i = 0; i < 0x100; i++)
    {
        f1 = *pbuf1_LLP_0++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_0 = 0x%x", f1);
        }
        
        f1 = *pbuf1_LLP_1++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_1 = 0x%x", f1);
        }       

        
        f1 = *pbuf1_LLP_2++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_2 = 0x%x", f1);
        } 

        f1 = *pbuf1_LLP_3++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_3 = 0x%x", f1);
        }  

        f1 = *pbuf1_LLP_4++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_4 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_5++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_5 = 0x%x", f1);
        } 

        f1 = *pbuf1_LLP_6++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_6 = 0x%x", f1);
        }
#if 1
        f1 = *pbuf1_LLP_7++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_7 = 0x%x", f1);
        }

        f1 = *pbuf1_LLP_8++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_8 = 0x%x", f1);
        }
        
        f1 = *pbuf1_LLP_9++;
        if((f1 & f3) != f3)
        {
            DEBUG("pbuf1_LLP_9 = 0x%x", f1);
        }
#endif
        
    }  

    DmaDisableInt(IDRAM_DMA_CH);    

    printf("\r\ndmallp test over");

    #undef H_IRAM_ADDR
    #undef H_IRAM_SIZE
    #undef H_DRAM_ADDR
    #undef H_DRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTest_DMA_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:10:43
*******************************************************************************/

rk_err_t MemoryTest_DMA_RW(uint8 * pstr)
{
    uint32 * pbuf, *pbuf1;
    uint32 i = 0;
    uint32 f1,f2,f3,f4;
    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};
   
    #define H_IRAM_ADDR 0x01000000
    #define H_IRAM_SIZE 0x20000

    #define H_DRAM_ADDR 0x01020000
    #define H_DRAM_SIZE 0x40000
    
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        IDRAM_DMA_CH = 0;
        printf("\r\n IDRAM_DMA_CH start 0\n");    
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        IDRAM_DMA_CH = 1;
        printf("\r\n IDRAM_DMA_CH start 1\n");    
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        IDRAM_DMA_CH = 2;
        printf("\r\n IDRAM_DMA_CH start 2\n");    
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        IDRAM_DMA_CH = 3;
        printf("\r\n IDRAM_DMA_CH start 3\n");    
    }
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        IDRAM_DMA_CH = 4;
        printf("\r\n IDRAM_DMA_CH start 4\n");    
    }
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        IDRAM_DMA_CH = 5;
        printf("\r\n IDRAM_DMA_CH start 5\n");    
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
    
    DmaEnableInt(IDRAM_DMA_CH);
    IDRAM_DmaFinish = 0;
    

    f1 = f3 = 0x55aa55aa;
    f2 = f4 = 0xaa55aa55;

//------------------------------------------------------------//    
    pbuf = (uint32 *)H_IRAM_ADDR;

    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        *pbuf++ = f1;
    }
    
    DmaStart(IDRAM_DMA_CH, H_IRAM_ADDR, H_DRAM_ADDR, H_DRAM_SIZE/4, &DmaCfg, IDRAM_DmaIsr);
		while(!IDRAM_DmaFinish);
        
    DmaDisableInt(IDRAM_DMA_CH);  

    pbuf1 = (uint32 *)H_DRAM_ADDR;
    
    f1 = 0;
    
    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        f1 = *pbuf1++;
        if((f1 & f3) != f3)
        {
            DEBUG("err data f1 = 0x%x", f1);
            DEBUG("err address = 0x%x", pbuf1);
        }
        
        if((i%500) == 0)
        {
           printf("0x%x,", f1); 
        }       
    }
    printf("\r\n 55aa test over\n");
//-------------------------------------------------------------------//
    pbuf = (uint32 *)H_IRAM_ADDR;

    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        *pbuf++ = f2;
    }

    DmaEnableInt(IDRAM_DMA_CH);
    IDRAM_DmaFinish = 0;
    
    DmaStart(IDRAM_DMA_CH, H_IRAM_ADDR, H_DRAM_ADDR, H_DRAM_SIZE/4, &DmaCfg, IDRAM_DmaIsr);
		while(!IDRAM_DmaFinish);
        
    DmaDisableInt(IDRAM_DMA_CH); 

    pbuf1 = (uint32 *)H_DRAM_ADDR;
    
    f2 = 0;
    
    for(i = 0; i < H_DRAM_SIZE/4; i++)
    {
        f2 = *pbuf1++;
        if((f2 & f4) != f4)
        {
            DEBUG("err data f2 = 0x%x", f2);
            DEBUG("err address = 0x%x", pbuf1);
        }
        
        if((i%500) == 0)
        {
           printf("0x%x,", f2); 
        }       
    }
    printf("\r\n aa55 test over\n");
    #undef H_IRAM_ADDR
    #undef H_IRAM_SIZE
    #undef H_DRAM_ADDR
    #undef H_DRAM_SIZE
    printf("\r\ndma test over");

}

/*******************************************************************************
** Name: ShellBspMemoryTestPMUSRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:53:07
*******************************************************************************/

rk_err_t MemoryTestPMUSRAM_RW(uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
        uint32 * pbuf2, *pbuf3;

    uint32 i = 0;
    uint32 f1,f2,f3,f4;
        uint32 f11,f22,f33,f44;

    #define PMU_SRAM_ADDR0 0x00000000
    #define PMU_SRAM_ADDR1 0x03090000
    #define PMU_SRAM_SIZE  0x10000   
    //test pmu sram
    pbuf2 = (uint32 *)PMU_SRAM_ADDR1;
    
    for(i = 0; i < PMU_SRAM_SIZE/8; i++)
    {
        *pbuf2++ = (uint32)pbuf2;
        *pbuf2++ = (uint32)pbuf2;
    }

    pbuf2 = (uint32 *)PMU_SRAM_ADDR1;

    for(i = 0; i < PMU_SRAM_SIZE/8; i++)
    {
        f11 = *pbuf2;
        if((f11 & (uint32)pbuf2) != (uint32)pbuf2)
        {
            DEBUG("err data f1 = %x", f11);
            DEBUG("err address = %x", pbuf2);
        }
        pbuf2++;
        f22 = *pbuf2;
        
        if((f22 & (uint32)pbuf2) != (uint32)pbuf2)
        {
            DEBUG("err data f2 = %x", f22);
            DEBUG("err address = %x", pbuf2);
        }
        pbuf2++;
        if((i%500) == 0)
        {
           printf("%x,%x,", f11,f22);
        }
    }
    pbuf = (uint8 *)PMU_SRAM_ADDR1;
    
    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        printf("\r\npmu sram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        printf("\r\npmu sram read write test start 0xaa55\n");
    }
    


    for(i = 0; i < PMU_SRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)PMU_SRAM_ADDR1;

    for(i = 0; i < PMU_SRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            DEBUG("err data f1 = %x", f1);
            DEBUG("err address = %x", pbuf);
        }
        
        f2 = *pbuf++;
        
        if((f2 & f4) != f4)
        {
            DEBUG("err data f2 = %x", f2);
            DEBUG("err address = %x", pbuf);
        }

        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    printf("\r\npmu sram read write test over");
    #undef PMU_SRAM_ADDR0
    #undef PMU_SRAM_ADDR1
    #undef PMU_SRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestHIRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:51:37
*******************************************************************************/

rk_err_t MemoryTestHIRAM_RW(uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint32 * pbuf2, *pbuf3;

    uint8 f1,f2,f3,f4;
    uint32 f11,f22,f33,f44;
    #define H_IRAM_ADDR 0x01000000
    #define H_IRAM_SIZE 0x20000
    
    pbuf2 = (uint32 *)H_IRAM_ADDR;
    
    for(i = 0; i < H_IRAM_SIZE/8; i++)
    {
        *pbuf2++ = (uint32)pbuf2;
        *pbuf2++ = (uint32)pbuf2;
    }

    pbuf2 = (uint32 *)H_IRAM_ADDR;

    for(i = 0; i < H_IRAM_SIZE/8; i++)
    {
        f11 = *pbuf2;
        if((f11 & (uint32)pbuf2) != (uint32)pbuf2)
        {
            DEBUG("err data f1 = %x", f11);
            DEBUG("err address = %x", pbuf2);
        }
        pbuf2++;
        f22 = *pbuf2;
        
        if((f22 & (uint32)pbuf2) != (uint32)pbuf2)
        {
            DEBUG("err data f2 = %x", f22);
            DEBUG("err address = %x", pbuf2);
        }
        pbuf2++;
        if((i%500) == 0)
        {
           printf("%x,%x,", f11,f22);
        }
    }

    
    //test H iram
    pbuf = (uint8 *)H_IRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        printf("\r\nHiram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        printf("\r\nHiram read write test start 0xaa55\n");
    }

    for(i = 0; i < H_IRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)H_IRAM_ADDR;

    for(i = 0; i < H_IRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            DEBUG("err data f1 = %x", f1);
            DEBUG("err address = %x", pbuf);
        }
        
        f2 = *pbuf++;
        
        if((f2 & f4) != f4)
        {
            DEBUG("err data f2 = %x", f2);
            DEBUG("err address = %x", pbuf);
        }
        
        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    printf("\r\nhiram read write test over");
    #undef H_IRAM_ADDR
    #undef H_IRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestHDRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:49:32
*******************************************************************************/

rk_err_t MemoryTestHDRAM_RW(uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
        uint32 * pbuf2, *pbuf3;

    uint8 f1,f2,f3,f4;
     uint32 f11,f22,f33,f44;   
    #define H_DRAM_ADDR 0x01020000
    #define H_DRAM_SIZE 0x40000
    
    pbuf2 = (uint32 *)H_DRAM_ADDR;
    
    for(i = 0; i < H_DRAM_SIZE/8; i++)
    {
        *pbuf2++ = (uint32)pbuf2;
        *pbuf2++ = (uint32)pbuf2;
    }

    pbuf2 = (uint32 *)H_DRAM_ADDR;

    for(i = 0; i < H_DRAM_SIZE/8; i++)
    {
        f11 = *pbuf2;
        if((f11 & (uint32)pbuf2) != (uint32)pbuf2)
        {
            DEBUG("err data f1 = %x", f11);
            DEBUG("err address = %x", pbuf2);
        }
        pbuf2++;
        f22 = *pbuf2;
        
        if((f22 & (uint32)pbuf2) != (uint32)pbuf2)
        {
            DEBUG("err data f2 = %x", f22);
            DEBUG("err address = %x", pbuf2);
        }
        pbuf2++;
        if((i%500) == 0)
        {
           printf("%x,%x,", f11,f22);
        }
    }

    //test H dram
    pbuf = (uint8 *)H_DRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        printf("\r\nHdram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        printf("\r\nHdram read write test start 0xaa55\n");
    }

    for(i = 0; i < H_DRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)H_DRAM_ADDR;

    for(i = 0; i < H_DRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            DEBUG("err data f1 = %x", f1);
            DEBUG("err address = %x", pbuf);
        }
        
        f2 = *pbuf++;
        
        if((f2 & f4) != f4)
        {
            DEBUG("err data f2 = %x", f2);
            DEBUG("err address = %x", pbuf);
        }
        
        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2); 
        }       
    }

    printf("\r\nhdram read write test over");
    #undef H_DRAM_ADDR
    #undef H_DRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestDRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:45:16
*******************************************************************************/

rk_err_t MemoryTestDRAM_RW(uint8 * pstr)
{    
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint8 f1,f2,f3,f4;
    #define DRAM_ADDR 0x03050000
    #define DRAM_SIZE 0x40000 


    //test H dram
    pbuf = (uint8 *)DRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        printf("\r\ndram read write test start 0x55aa\n");
    }
    else if(StrCmpA2(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        printf("\r\ndram read write test start 0xaa55\n");
    }


    for(i = 0; i < DRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)DRAM_ADDR;

    for(i = 0; i < DRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            DEBUG("err data f1 = %x", f1);
            DEBUG("err address = %x", pbuf);
        }
        
        f2 = *pbuf++;
        
        if((f2 & f4) != f4)
        {
            DEBUG("err data f2 = %x", f2);
            DEBUG("err address = %x", pbuf);
        }
        
        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    printf("\r\ndram read write test over");

    #undef DRAM_ADDR
    #undef DRAM_SIZE
}
/*******************************************************************************
** Name: ShellBspMemoryTestIRAM_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.24
** Time: 19:42:02
*******************************************************************************/

rk_err_t MemoryTestIRAM_RW(uint8 * pstr)
{
    uint8 * pbuf, *pbuf1;
    uint32 i = 0;
    uint8 f1,f2,f3,f4;
    
    #define IRAM_ADDR 0x03000000
    #define IRAM_SIZE 0x50000  

    //test H dram
    pbuf = (uint8 *)IRAM_ADDR;

    if(StrCmpA(pstr, "1", 1) == 0)
    {
        f1 = f3 = 0x55;
        f2 = f4 = 0xaa;
        printf("\r\niram read write test start 0x55aa\n");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        f1 = f3 = 0xaa;
        f2 = f4 = 0x55;
        printf("\r\niram read write test start 0xaa55\n");
    }

    for(i = 0; i < IRAM_SIZE/2; i++)
    {
        *pbuf++ = f1;
        *pbuf++ = f2;
    }

    pbuf = (uint8 *)IRAM_ADDR;

    for(i = 0; i < IRAM_SIZE/2; i++)
    {
        f1 = *pbuf++;
        if((f1 & f3) != f3)
        {
            DEBUG("err data f1 = %x", f1);
            DEBUG("err address = %x", pbuf);
        }
        
        f2 = *pbuf++;
        
        if((f2 & f4) != f4)
        {
            DEBUG("err data f2 = %x", f2);
            DEBUG("err address = %x", pbuf);
        }
        
        if((i%500) == 0)
        {
           printf("%x,%x,", f1,f2);
        }
    }

    printf("\r\niram read write test over");
    #undef IRAM_ADDR
    #undef IRAM_SIZE


}

/*******************************************************************************
** Name: ShellBspMemoryTestReadWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 10:59:02
*******************************************************************************/

rk_err_t MemoryRW_Test(HDC dev, uint8 * pstr)
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
        case '0':           
           ret =  RK_EXIT;           
           return ret;

        case '1':
            ret = MemoryTestIRAM_RW(pItem);
            break;

        case '2':
            ret = MemoryTestDRAM_RW(pItem);
            break;

        case '3':
            ret = MemoryTestHIRAM_RW(pItem);
            break;

        case '4':
            ret = MemoryTestHDRAM_RW(pItem);
            break;

        case '5':
            ret = MemoryTestPMUSRAM_RW(pItem);
            break;
            
        case '6':
            ret = MemoryTest_DMA_RW(pItem);
            break; 
            
        case '7':
            ret = MemoryTest_DMA_LLP_RW(pItem);
            break; 
            
        case 'I':
            //init
            ret = RK_SUCCESS;
            break; 
        default:
            break;
    }    
    
    MemoryTest_Menu();
    return ret;
    
}
#endif

