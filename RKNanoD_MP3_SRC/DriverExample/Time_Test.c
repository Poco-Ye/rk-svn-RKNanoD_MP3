/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   Time_Test.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                                           
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _TIME_TEST_

#include "Device.h"	
#include "DriverInclude.h"
#include "Interrupt.h"

uint32 TIME_intstatus;
/*******************************************************************************
** Name: MemoryTest_Menu
** Input:void
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.27
** Time: 9:11:16
*******************************************************************************/
void TimeTest_Menu(void)
{
    rk_print_stringA("================================================================================\n");
    rk_print_stringA(" Time Test Menu                                                                 \n");
    rk_print_stringA(" 1. Time Test :                                                                 \n");
    rk_print_stringA("    0.Time0 5s                                                                  \n");
    rk_print_stringA("    1.Time0 10s                                                                 \n");
    rk_print_stringA("    2.Time0 15s                                                                 \n");
    rk_print_stringA("    3.Time0 20s                                                                 \n");    
    rk_print_stringA("    4.Time0 5400s                                                               \n");
    rk_print_stringA("    5.Time1 5s                                                                  \n");
    rk_print_stringA("    6.Time1 10s                                                                 \n");
    rk_print_stringA("    7.Time1 15s                                                                 \n");
    rk_print_stringA("    8.Time1 20s                                                                 \n");
    rk_print_stringA("    9.Time1 5400s                                                               \n");
    rk_print_stringA(" 0. EXIT                                                                        \n");
    rk_print_stringA("================================================================================\n");    
}
/*******************************************************************************
** Name: Timer1_Test_ISR
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.3
** Time: 15:55:11
*******************************************************************************/
void Timer1_Test_ISR(void)
{
    pFunc callback;
    pTIMER_REG timerReg;
    Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_HIGH);

    TIME_intstatus = 0;
	printf("\r\n ------------Timer11111_Test_ISR------------\n");         
    TimerClrInt(TIMER1);
    TimerStop(TIMER1);
    //callback = g_timerIRQ[0];
    //if (callback)
    {
        //callback();
    }   
}
/*******************************************************************************
** Name: Timer0_Test_ISR
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.3
** Time: 15:52:18
*******************************************************************************/
void Timer0_Test_ISR(void)
{
    pFunc callback;
    pTIMER_REG timerReg;
    Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_HIGH);

    TIME_intstatus = 0;
	printf("\r\n ------------Timer00000_Test_ISR------------\n");         
    TimerClrInt(TIMER0);
    TimerStop(TIMER0);
    //callback = g_timerIRQ[0];
    //if (callback)
    {
        //callback();
    }
}
/*******************************************************************************
** Name: TimeDevShellBspDeinit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 17:56:41
*******************************************************************************/
rk_err_t TimeDevShellBspDeinit(uint8 * pstr)
{
    
}
/*******************************************************************************
** Name: TimeDevShellBspTime_Test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 17:55:58
*******************************************************************************/
rk_err_t TimeShorttime_Test(uint8 * pstr)
{
    uint64 loadcount = 0;
    uint32 loadCounthigh = 0,loadCountlow = 0;
    uint32 time_reg_num;

    TIME_intstatus = 1;   
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_OUT);
    Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_HIGH);
    time_reg_num = 1;
    
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        loadcount = 5*1000;      //5s = 5 * 1000us
        printf("\r\n Time0 5s start 0\n");    
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        loadcount = 50*1000;      //10s = 10 * 1000us
        printf("\r\n Time0 10s start 0\n");         
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        loadcount = 500*1000;      //15s = 15 * 1000us
        printf("\r\n Time0 15s start 0\n");         
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        loadcount = 5;      //20s = 20 * 1000us
        printf("\r\n Time0 20s start 0\n");         
    } 
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        loadcount = 50;      //5400s = 5400 * 1000us
        printf("\r\n Time0 5400s start 0\n");         
    } 
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        loadcount = 500;      //5400s = 5400 * 1000us
        printf("\r\n Time0 5400s start 0\n");         
    } 
    TIME_intstatus = 1;

    if(0 == time_reg_num)
    {
        IntUnregister(INT_ID_TIMER0);
        IntPendingClear(INT_ID_TIMER0);
        IntDisable(INT_ID_TIMER0);
        
        IntRegister(INT_ID_TIMER0 ,Timer0_Test_ISR);
        IntPendingClear(INT_ID_TIMER0);
        IntEnable(INT_ID_TIMER0); 
        
        //Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_LOW);
        
        TimerPeriodSet(TIMER0,loadcount,0);

    }
    else if(1 == time_reg_num)
    {
        IntUnregister(INT_ID_TIMER1);
        IntPendingClear(INT_ID_TIMER1);
        IntDisable(INT_ID_TIMER1);

        IntRegister(INT_ID_TIMER1 ,Timer1_Test_ISR);
        IntPendingClear(INT_ID_TIMER1);
        IntEnable(INT_ID_TIMER1); 
        //Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_LOW);

        TimerPeriodSet(TIMER1,loadcount,0);
       
    }
    while(TIME_intstatus);
    
    printf("\r\ntime init over");
    return 0;
}
/*******************************************************************************
** Name: TimeDevShellBspTime_Test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 17:55:58
*******************************************************************************/
rk_err_t TimeDevShellBspTime_Test(uint8 * pstr)
{
    uint64 loadcount = 0;
    uint32 loadCounthigh = 0,loadCountlow = 0;

    uint32 time_reg_num;
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 5*1000*1000;      //5s = 5 * 1000us
        printf("\r\n Time0 5s start 0\n");    
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 10*1000*1000;      //10s = 10 * 1000us
        printf("\r\n Time0 10s start 0\n");         
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 15*1000*1000;      //15s = 15 * 1000us
        printf("\r\n Time0 15s start 0\n");         
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 20*1000*1000;      //20s = 20 * 1000us
        printf("\r\n Time0 20s start 0\n");         
    } 
    else if(StrCmpA(pstr, "4", 1) == 0)
    {
        time_reg_num = 0;
        loadcount = 5400000*1000;      //5400s = 5400 * 1000us
        printf("\r\n Time0 5400s start 0\n");         
    } 
    //---------------------------------------------------------------//
    else if(StrCmpA(pstr, "5", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 5*1000*1000;      //5s = 5 * 1000us
        printf("\r\n Time1 5s start 0\n");         
    } 
    else if(StrCmpA(pstr, "6", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 10*1000*1000;      //10s = 10 * 1000us
        printf("\r\n Time1 10s start 0\n");         
    }
    else if(StrCmpA(pstr, "7", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 15*1000*1000;      //15s = 15 * 1000us
        printf("\r\n Time1 15s start 0\n");         
    } 
    else if(StrCmpA(pstr, "8", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 20*1000*1000;      //20s = 20 * 1000us
        printf("\r\n Time1 20s start 0\n");         
    }
    else if(StrCmpA(pstr, "9", 1) == 0)
    {
        time_reg_num = 1;
        loadcount = 5400000*1000;      //5400s = 5400 * 1000us
        printf("\r\n Time1 5400s start 0\n");         
    }

    TIME_intstatus = 1;

    if(0 == time_reg_num)
    {
        IntUnregister(INT_ID_TIMER0);
        IntPendingClear(INT_ID_TIMER0);
        IntDisable(INT_ID_TIMER0);
        
        IntRegister(INT_ID_TIMER0 ,Timer0_Test_ISR);
        IntPendingClear(INT_ID_TIMER0);
        IntEnable(INT_ID_TIMER0); 
        
        TimerPeriodSet(TIMER0,loadcount,0);

    }
    else if(1 == time_reg_num)
    {
        IntUnregister(INT_ID_TIMER1);
        IntPendingClear(INT_ID_TIMER1);
        IntDisable(INT_ID_TIMER1);

        IntRegister(INT_ID_TIMER1 ,Timer1_Test_ISR);
        IntPendingClear(INT_ID_TIMER1);
        IntEnable(INT_ID_TIMER1); 

        TimerPeriodSet(TIMER1,loadcount,0);
       
    }
    while(TIME_intstatus);
    
    printf("\r\ntime init over");
    return 0;
}

rk_err_t Time_test(HDC dev, uint8 * pstr)
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
            ret = TimeDevShellBspTime_Test(pItem);
            break; 
        case '2':
            ret = TimeShorttime_Test(pItem);
            break;            
        case 'I':
            //init
            break; 
        default:
            break;
    }    
    TimeTest_Menu();
    return ret;
           
}
#endif

