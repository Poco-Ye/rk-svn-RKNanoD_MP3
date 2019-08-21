/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   Pwm_Test.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                                           
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _PWM_TEST_

#include "Device.h"	
#include "DriverInclude.h"

#define     PWM_DUTY_10        10
#define     PWM_DUTY_20        20
#define     PWM_DUTY_50        50
#define     PWM_DUTY_80        80

#define     PWM_FREQ_10K        10000
#define     PWM_FREQ_20K        20000
#define     PWM_FREQ_50K        50000
#define     PWM_FREQ_100K       100000

void PWMTestHelpInfo()
{
    printf("\t PWM test start!\n");
    printf("================================================================================\n");
    printf(" PWM Test Menu                                                                  \n");
    printf(" 1. PWM_CHN0 CLK Test Start,duty 20% [1] --OUT 10K [2] --OUT 50K [3] --OUT 100K  \n");
    printf(" 2. PWM_CHN1 CLK Test Start,duty 20% [1] --OUT 10K [2] --OUT 50K [3] --OUT 100K \n");
    printf(" 3. PWM_CHN2 CLK Test Start,duty 20% [1] --OUT 10K [2] --OUT 50K [3] --OUT 100K \n");
    printf(" 4. PWM_CHN3 CLK Test Start,duty 20% [1] --OUT 10K [2] --OUT 50K [3] --OUT 100K \n");
    printf(" 5. PWM_CHN4 CLK Test Start,duty 20% [1] --OUT 10K [2] --OUT 50K [3] --OUT 100K \n");
    printf(" 6. PWM_CHN0 DUTY Test Start,CLK OUT 10K  [1] --DUTY 10% [2] --DUTY 50% [3] --DUTY 80% \n");
    printf(" 7. PWM_CHN1 DUTY Test Start,CLK OUT 20K  [1] --DUTY 10% [2] --DUTY 50% [3] --DUTY 80% \n");
    printf(" 8. PWM_CHN2 DUTY Test Start,CLK OUT 50K  [1] --DUTY 10% [2] --DUTY 50% [3] --DUTY 80% \n");
    printf(" 9. PWM_CHN3 DUTY Test Start,CLK OUT 100K [1] --DUTY 10% [2] --DUTY 50% [3] --DUTY 80% \n");
    printf(" a. PWM_CHN4 DUTY Test Start,CLK OUT 100K [1] --DUTY 10% [2] --DUTY 50% [3] --DUTY 80% \n");

    printf("\r\n");
    printf(" 0. exit                                                                        \n");
    printf(" h. show pwm test menu                                                          \n");
    printf("================================================================================\n");
    printf("\r\n");
    
}

void PWMClkTest(ePWM_CHN ch,uint8 * pstr)
{
    PWM_GPIO_Init(ch);

    PWM_Start(ch);
    PwmRegReset(ch);

    if((StrCmpA(pstr, "1", 1) == 0))
    {
        printf(" \n PWM_FREQ OUT 10K. \n");
        PwmRateSet(ch,PWM_DUTY_20,PWM_FREQ_10K);
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        printf(" \n PWM_FREQ OUT 50K. \n");
        PwmRateSet(ch,PWM_DUTY_20,PWM_FREQ_50K);
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        printf(" \n PWM_FREQ OUT 100K. \n");
        PwmRateSet(ch,PWM_DUTY_20,PWM_FREQ_100K);
    }        
        
}

void PWMSetDutyTest(ePWM_CHN ch,uint32 outclk,uint8 * pstr)
{
    PWM_GPIO_Init(ch);

    PWM_Start(ch);
    PwmRegReset(ch);

    if((StrCmpA(pstr, "1", 1) == 0))
    {
        printf(" \n duty 10% .clk = %d \n",outclk);
        PwmRateSet(ch,PWM_DUTY_10,outclk);
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        printf(" \n duty 50% .clk = %d \n",outclk);
        PwmRateSet(ch,PWM_DUTY_50,outclk);
    }
    else if(StrCmpA(pstr, "3", 1) == 0)
    {
        printf(" \n duty 80%.clk = %d \n",outclk);
        PwmRateSet(ch,PWM_DUTY_80,outclk);
    }        
        
}

//******************************************************************************
//pwm function test
rk_err_t PWMTestCmdParse(HDC dev, uint8 * pstr)
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
    pItem++; 
 
        
    switch (cmd)
    {
        case '0':
            {
                return RK_EXIT;
            }
            
        case '1':
            {//0
                printf(" PWM_CHN0 duty 20% Test Start \n");
                PWMClkTest(PWM_CHN0,pItem);
            }
            break;
            
        case '2':
            {// 1
                printf(" PWM_CHN1 duty 20% Test Start \n");
                PWMClkTest(PWM_CHN1,pItem);
            }
            break;

        case '3':
            {// 2
                printf(" PWM_CHN2 duty 20% Test Start \n");
                PWMClkTest(PWM_CHN2,pItem);
            }
            break; 

        case '4':
            {// 3
                printf(" PWM_CHN3 duty 20% Test Start \n");
                PWMClkTest(PWM_CHN3,pItem);
            }
            break;
            
        case '5':
            {// 4
                printf(" PWM_CHN4 duty 20% Test Start \n");
                PWMClkTest(PWM_CHN4,pItem);
            }
            break;

        case '6':
            {// 0
                printf(" PWM_CHN0 output 10k Test Start \n");
                PWMSetDutyTest(PWM_CHN0,PWM_FREQ_10K,pItem);
            }
            break;
        case '7':
            {// 1
                printf(" PWM_CHN1 output 20k Test Start \n");
                PWMSetDutyTest(PWM_CHN1,PWM_FREQ_20K,pItem);
            }
            break;
        case '8':
            {// 2
                printf(" PWM_CHN2 output 50k Test Start \n");
                PWMSetDutyTest(PWM_CHN2,PWM_FREQ_50K,pItem);
            }
            break;
        case '9':
            {// 3
                printf(" PWM_CHN3 output 100k Test Start \n");
                PWMSetDutyTest(PWM_CHN3,PWM_FREQ_100K,pItem);
            }
            break;
        case 'a':
            {// 4
                printf(" PWM_CHN4 output 100k Test Start \n");
                PWMSetDutyTest(PWM_CHN4,PWM_FREQ_100K,pItem);
            }
            break;            
            
        case 'h':
           PWMTestHelpInfo();
        default:
            ret = RK_ERROR;
            break;
   
    }
        
    PWMTestHelpInfo();
    return ret;
    
}
#endif
 
