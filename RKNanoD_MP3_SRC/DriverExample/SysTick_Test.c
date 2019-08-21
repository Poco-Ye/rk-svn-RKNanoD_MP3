/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   SysTick_Test.c
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

#ifdef _SYSTICK_TEST_

#include "Device.h"	
#include "DriverInclude.h"
#include "Interrupt.h"
/*******************************************************************************
** Name: MemoryTest_Menu
** Input:void
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/
void SysTick_Menu(void)
{
    printf("================================================================================\n");
    printf(" SysTick Test Menu                                                              \n");
    printf(" 1. SysTick1 Test Start                                                         \n");
    printf(" 2. SysTick2 Test Start                                                         \n");
    printf(" 0. SysTick Test End                                                            \n");
    printf("================================================================================\n");    
}
void SysTick1Run(void)
{
    printf("SysTick1Run!\n");
}
SYSTICK_LIST SysTick1 = 
{
    NULL,
    0,
    100,
    20,
    SysTick1Run,
};

void SysTick2Run(void)
{
    printf("SysTick2Run!\n");
}
SYSTICK_LIST SysTick2 = 
{
    NULL,
    0,
    50,
    10,
    SysTick2Run,
};
rk_err_t SysTick_Test(HDC dev, uint8 * pstr)
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


    IntRegister(FAULT_ID15_SYSTICK, (void*)SysTickHandler);
    IntEnable(FAULT_ID15_SYSTICK);
    SysTickDisable();
    
    SysTickClkSourceSet(NVIC_SYSTICKCTRL_CLKIN);
    SysTickPeriodSet(10);
    SysTickEnable();


    switch (cmd)
    {
        case '1':
            printf("SysTick1 Test Start\n");
            SysTick1.Times = 20;
            SystickTimerStart(&SysTick1); 
            break;
            
        case '2':
            printf("SysTick2 Test Start\n");
            SysTick2.Times = 10;
            SystickTimerStart(&SysTick2);
            break;
            
        case '0':
            ret =  RK_EXIT;           
            return;
            
        default:
            ret = RK_ERROR;
            break;
    }
    SysTick_Menu();
    SysTickDisable();
    IntDisable(FAULT_ID15_SYSTICK);
    IntUnregister(FAULT_ID15_SYSTICK);
}
#endif

