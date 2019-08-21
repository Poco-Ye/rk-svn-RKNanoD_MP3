/*
********************************************************************************
*                       Copyright (c) 2015 RockChips
*                         All rights reserved.
*
* File Name：   GPIO_Test.c
*
* Description:
*
* History:      <author>          <time>        <version>
*
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _GPIO_TEST_

#include "Device.h"
#include "DriverInclude.h"


void GPIOPortA_Pin0_Handler(void){printf("GPIOPortA_Pin0_Isr!\n");}
void GPIOPortA_Pin1_Handler(void){printf("GPIOPortA_Pin1_Isr!\n");}
void GPIOPortA_Pin2_Handler(void){printf("GPIOPortA_Pin2_Isr!\n");}
void GPIOPortA_Pin3_Handler(void){printf("GPIOPortA_Pin3_Isr!\n");}
void GPIOPortA_Pin4_Handler(void){printf("GPIOPortA_Pin4_Isr!\n");}
void GPIOPortA_Pin5_Handler(void){printf("GPIOPortA_Pin5_Isr!\n");}
void GPIOPortA_Pin6_Handler(void){printf("GPIOPortA_Pin6_Isr!\n");}
void GPIOPortA_Pin7_Handler(void){printf("GPIOPortA_Pin7_Isr!\n");}

void GPIOPortB_Pin0_Handler(void){printf("GPIOPortB_Pin0_Isr!\n");}
void GPIOPortB_Pin1_Handler(void){printf("GPIOPortB_Pin1_Isr!\n");}
void GPIOPortB_Pin2_Handler(void){printf("GPIOPortB_Pin2_Isr!\n");}
void GPIOPortB_Pin3_Handler(void){printf("GPIOPortB_Pin3_Isr!\n");}
void GPIOPortB_Pin4_Handler(void){printf("GPIOPortB_Pin4_Isr!\n");}
void GPIOPortB_Pin5_Handler(void){printf("GPIOPortB_Pin5_Isr!\n");}
void GPIOPortB_Pin6_Handler(void){printf("GPIOPortB_Pin6_Isr!\n");}
void GPIOPortB_Pin7_Handler(void){printf("GPIOPortB_Pin7_Isr!\n");}

void GPIOPortC_Pin0_Handler(void){printf("GPIOPortC_Pin0_Isr!\n");}
void GPIOPortC_Pin1_Handler(void){printf("GPIOPortC_Pin1_Isr!\n");}
void GPIOPortC_Pin2_Handler(void){printf("GPIOPortC_Pin2_Isr!\n");}
void GPIOPortC_Pin3_Handler(void){printf("GPIOPortC_Pin3_Isr!\n");}
void GPIOPortC_Pin4_Handler(void){printf("GPIOPortC_Pin4_Isr!\n");}
void GPIOPortC_Pin5_Handler(void){printf("GPIOPortC_Pin5_Isr!\n");}
void GPIOPortC_Pin6_Handler(void){printf("GPIOPortC_Pin6_Isr!\n");}
void GPIOPortC_Pin7_Handler(void){printf("GPIOPortC_Pin7_Isr!\n");}

void GPIOPortD_Pin0_Handler(void){printf("GPIOPortD_Pin0_Isr!\n");}
void GPIOPortD_Pin1_Handler(void){printf("GPIOPortD_Pin1_Isr!\n");}
void GPIOPortD_Pin2_Handler(void){printf("GPIOPortD_Pin2_Isr!\n");}
void GPIOPortD_Pin3_Handler(void){printf("GPIOPortD_Pin3_Isr!\n");}

void GPIOTestHelpInfo(void)
{
    printf("\r\n");
    printf("================================================================================\n");
    printf(" GPIO Test Menu                                                                 \n");
    printf(" 1. OUTPUT LOW  LEVEL ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 2. OUTPUT HIGH LEVEL ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 3. IN  LOW LEVEL     ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 4. IN HIGH LEVEL     ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 5. SET PULL          ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 6. RAINSING INT      ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 7. FALLING INT       ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 8. HIGHT LEVEL INT   ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");
    printf(" 9. LOW LEVEL INT     ([0] - channel 0,[1] - channel 1, [2] - channnel 2)       \n");

    printf(" a. CHANNEL 0 IOMUX  (pin 0 ~ pin 31)                                           \n");
    printf(" b. CHANNEL 1 IOMUX  (pin 0 ~ pin 31)                                           \n");
    printf(" c. CHANNEL 2 IOMUX  (pin 0 ~ pin 31)                                           \n");

    printf(" d. CHANNEL 0 PIN OUTPUT LOW LEVEL  (pin 0 ~ pin 31)                               \n");
    printf(" e. CHANNEL 1 PIN OUTPUT LOW LEVEL  (pin 0 ~ pin 31)                              \n");
    printf(" f. CHANNEL 2 PIN OUTPUT LOW LEVEL  (pin 0 ~ pin 31)                              \n");

    printf(" g. CHANNEL 0 PIN OUTPUT HIGH LEVEL (pin 0 ~ pin 31)                                \n");
    printf(" i. CHANNEL 1 PIN OUTPUT HIGH LEVEL (pin 0 ~ pin 31)                                \n");
    printf(" j. CHANNEL 2 PIN OUTPUT HIGH LEVEL (pin 0 ~ pin 31)                                \n");

    printf(" k. CHANNEL 0 PIN IN LOW LEVEL  (pin 0 ~ pin 31)                                \n");
    printf(" l. CHANNEL 1 PIN IN LOW LEVEL  (pin 0 ~ pin 31)                                \n");
    printf(" m. CHANNEL 2 PIN IN LOW LEVEL  (pin 0 ~ pin 31)                                \n");

    printf(" n. CHANNEL 0 PIN IN HIGH LEVEL (pin 0 ~ pin 31)                                \n");
    printf(" o. CHANNEL 1 PIN IN HIGH LEVEL (pin 0 ~ pin 31)                                \n");
    printf(" p. CHANNEL 2 PIN IN HIGH LEVEL (pin 0 ~ pin 31)                                \n");

    printf("\r\n");
    printf(" h. show GPIO help menu                                                         \n");
    printf(" 0. EXIT                                                                        \n");
    printf("================================================================================\n");
    printf("\r\n");
}

void Grf_GpioPortMuxSet(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;

    if(gpioChn >= GPIO_CH_MAX)
        return;

    if(gpioChn == GPIO_CH0)
    {
        Grf->GPIO_IO0MUX[gpioPort] = 0xffff0000;
    }
    else if(gpioChn == GPIO_CH1)
    {
        Grf->GPIO_IO1MUX[gpioPort] = 0xffff0000;
    }
    else if(gpioChn == GPIO_CH2)
    {
        Grf->GPIO_IO2MUX[gpioPort] = 0xffff0000;
    }
}



void Grf_GPIO_SetPortPull(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort, eGPIOPinPull_t pull)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;

    if(gpioChn >= GPIO_CH_MAX)
        return;


    if(gpioChn == GPIO_CH0)
    {
        if(pull == ENABLE)
            Grf->GPIO_IO0PULL[gpioPort] = 0xffff0000;
        else
            Grf->GPIO_IO0PULL[gpioPort] = 0xffff00ff;
    }
    else if(gpioChn == GPIO_CH1)
    {
        if(pull == ENABLE)
            Grf->GPIO_IO1PULL[gpioPort] = 0xffff0000;
        else
            Grf->GPIO_IO1PULL[gpioPort] = 0xffff00ff;
    }
    else if(gpioChn == GPIO_CH2)
    {
        if(pull == ENABLE)
            Grf->GPIO_IO2PULL[gpioPort] = 0xffff0000;
        else
            Grf->GPIO_IO2PULL[gpioPort] = 0xffff00ff;
    }
}

void GPIO_Direction_Set(uint32 data,uint8* pstr)
{//只测试gpio 3个通道的A组io口

    if(0 == StrCmpA(pstr, "0", 1))
    {
        Gpio_SetPortDirec(GPIO_CH0, data );

//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin7,direction);
//
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin7,direction);
//
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin7,direction);
//
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin7,direction);
    }
    else if( 0 == StrCmpA(pstr, "1", 1) )
    {
        Gpio_SetPortDirec(GPIO_CH1,data);

//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin7,direction);
//
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin7,direction);
//
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortC_Pin7,direction);
//
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin1,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin2,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin3,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin5,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH1,GPIOPortD_Pin7,direction);
    }
    else if( 0 == StrCmpA(pstr, "2", 1) )
    {
//        Gpio_SetPortDirec(GPIO_CH2,data);
        int direction;
        direction = 0;

        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin5,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin6,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin7,direction);

        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin0,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin1,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin2,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin3,direction);


        #ifdef _UART_DEBUG_
        #if(DEBUG_UART_PORT == 0)
//        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin4,direction);
//        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin5,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin0,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin1,direction);
        #elif (DEBUG_UART_PORT == 1)
//        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin6,direction);
//        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin7,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin4,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin5,direction);
        #elif (DEBUG_UART_PORT == 2)

        #endif
        #endif

//        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin0,direction);
//        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin1,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin2,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin3,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin4,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin5,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin6,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin7,direction);

        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin0,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin1,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin2,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin3,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin4,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin5,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin6,direction);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortD_Pin7,direction);
    }
}

void GPIO_Level_Set(eGPIOPinLevel_t level,uint8* pstr)
{//只测试gpio 3个通道的A组io口

    if(0 == StrCmpA(pstr, "0", 1))
    {
        Gpio_SetChnLevel(GPIO_CH0,level);

//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortA_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortC_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin7,level);
    }
    else if( 0 == StrCmpA(pstr, "1", 1) )
    {
        Gpio_SetChnLevel(GPIO_CH1,level);

//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortA_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortC_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH1,GPIOPortD_Pin7,level);
    }
    else if( 0 == StrCmpA(pstr, "2", 1) )
    {
        Gpio_SetChnLevel(GPIO_CH2,level);
//
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortB_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortC_Pin7,level);
//
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin0,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin1,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin2,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin3,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin4,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin5,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin6,level);
//        Gpio_SetPinLevel(GPIO_CH2,GPIOPortD_Pin7,level);
    }
}

static uint32 str2num(uint8* pstr)
{
    uint32 strLen;
    uint32 num;
    uint8 temp[2] = {0};

    num = 0;
    strLen = StrLenA(pstr);
    if(strLen == 0 || strLen > 2)
        return -1;

    if( strLen == 1)
    {
        temp[0] = *pstr - 0x30;
        num = temp[0];
    }
    else if(strLen == 2)
    {
        temp[0] = *pstr - 0x30;
        pstr++;
        temp[1] = *pstr - 0x30;

        num = temp[0] * 10 + temp[1];
    }

    return num;
}


void GPIOTEST_IOMuxPin_Set(eGPIO_CHANNEL gpioChn,uint8* pstr)
{
    int pinNum;

    pinNum = 0;

    if(gpioChn > GPIO_CH_MAX)
        return;

    pinNum = str2num(pstr);

    if(pinNum > 31)
    {
        printf("GPIO CHANNEL %d pin %d beyond range, pin[0 ~ 31] .\n",gpioChn,pinNum);
        return;
    }

    printf("GPIO CHANNEL %d iomux pin %d as gpio.\n",gpioChn,pinNum);

    Grf_GpioMuxSet(gpioChn, pinNum, 0);
}

void GPIOTEST_SetPin_Direc(eGPIO_CHANNEL gpioChn,
                                    eGPIOPinDirection_t dir,
                                    uint8* pstr)
{
    int pinNum;

    pinNum = 0;

    if(gpioChn > GPIO_CH_MAX)
        return;

    pinNum = str2num(pstr);

    if(pinNum > 31)
    {
        printf(
"GPIO CHANNEL %d pin %d beyond range, pin[0 ~ 31] .\n",gpioChn,pinNum);
        return;
    }

    if(dir)
        printf(
"GPIO CHANNEL %d Set pin %d direction OUT.\n",gpioChn,pinNum);
    else
        printf(
"GPIO CHANNEL %d Set pin %d direction IN.\n",gpioChn,pinNum);

    Gpio_SetPinDirection(gpioChn,pinNum,dir);
}

void GPIOTEST_SetPinLevel(eGPIO_CHANNEL gpioChn,
                                eGPIOPinLevel_t level,
                                uint8* pstr)
{
    int pinNum;

    pinNum = 0;

    if(gpioChn > GPIO_CH_MAX)
        return;

    pinNum = str2num(pstr);

    if(pinNum > 31)
    {
        printf("GPIO CHANNEL %d pin %d beyond range, pin[0 ~ 31] .\n",gpioChn,pinNum);
        return;
    }

    if(level)
        printf("GPIO CHANNEL %d Set pin %d level HIGH.\n",gpioChn,pinNum);
    else
        printf("GPIO CHANNEL %d Set pin %d level LOW.\n",gpioChn,pinNum);

    Gpio_SetPinLevel(gpioChn, pinNum, level);
}


void GPIO_Iomux_Set(uint8* pstr)
{//只测试gpio 3个通道的A组io口

    if(0 == StrCmpA(pstr, "0", 1))  //channel 0
    {
        Grf_GpioPortMuxSet(GPIO_CH0,GPIO_PORTA );
        Grf_GpioPortMuxSet(GPIO_CH0,GPIO_PORTB );
        Grf_GpioPortMuxSet(GPIO_CH0,GPIO_PORTC );
        Grf_GpioPortMuxSet(GPIO_CH0,GPIO_PORTD );
    }
    else if( 0 == StrCmpA(pstr, "1", 1) ) //channel 1
    {
        Grf_GpioPortMuxSet(GPIO_CH1,GPIO_PORTA );
        Grf_GpioPortMuxSet(GPIO_CH1,GPIO_PORTB );
        Grf_GpioPortMuxSet(GPIO_CH1,GPIO_PORTC );
        Grf_GpioPortMuxSet(GPIO_CH1,GPIO_PORTD );
    }
    else if( 0 == StrCmpA(pstr, "2", 1) ) //channel 2
    {
//        Grf_Force_Jtag_Set(0);  //force (gpiop2b6 gpiop2b7) as gpio
//        Grf_GpioPortMuxSet(GPIO_CH2,GPIO_PORTA );
//        Grf_GpioPortMuxSet(GPIO_CH2,GPIO_PORTB );
//        Grf_GpioPortMuxSet(GPIO_CH2,GPIO_PORTC );
//        Grf_GpioPortMuxSet(GPIO_CH2,GPIO_PORTD );

        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin5,IOMUX_GPIO2A5_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin7,IOMUX_GPIO2A7_IO);

        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin0,IOMUX_GPIO2B0_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin1,IOMUX_GPIO2B1_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin2,IOMUX_GPIO2B2_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin3,IOMUX_GPIO2B3_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_IO);

//        Grf_Force_Jtag_Set(0);  //force as gpio
//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_IO);
//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin7,IOMUX_GPIO2B7_IO);

//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_IO);
//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin2,IOMUX_GPIO2C2_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin3,IOMUX_GPIO2C3_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin4,IOMUX_GPIO2C4_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin5,IOMUX_GPIO2C5_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin6,IOMUX_GPIO2C6_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin7,IOMUX_GPIO2C7_IO);

        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin0,IOMUX_GPIO2D0_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin1,IOMUX_GPIO2D1_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin2,IOMUX_GPIO2D2_IO);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin3,IOMUX_GPIO2D3_IO);
//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin4,IOMUX_GPIO2D4_IO);
//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin5,IOMUX_GPIO2D5_IO);
//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin6,IOMUX_GPIO2D6_IO);
//        Grf_GpioMuxSet(GPIO_CH2,GPIOPortD_Pin7,IOMUX_GPIO2D7_IO);


    }
}


void GPIO_Pull_Set(uint32 enable,uint8* pstr)
{
    if(0 == StrCmpA(pstr, "0", 1))  //channel 0
    {
        Grf_GPIO_SetPortPull(GPIO_CH0,GPIO_PORTA,enable);
        Grf_GPIO_SetPortPull(GPIO_CH0,GPIO_PORTB,enable);
        Grf_GPIO_SetPortPull(GPIO_CH0,GPIO_PORTC,enable);
        Grf_GPIO_SetPortPull(GPIO_CH0,GPIO_PORTD,enable);

//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin7, enable);
//
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin7, enable);
//
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin7, enable);
//
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin7, enable);
    }
    else if( 0 == StrCmpA(pstr, "1", 1) ) //channel 1
    {
        Grf_GPIO_SetPortPull(GPIO_CH1,GPIO_PORTA,enable);
        Grf_GPIO_SetPortPull(GPIO_CH1,GPIO_PORTB,enable);
        Grf_GPIO_SetPortPull(GPIO_CH1,GPIO_PORTC,enable);
        Grf_GPIO_SetPortPull(GPIO_CH1,GPIO_PORTD,enable);

//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin7, enable);
//
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin7, enable);
//
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortC_Pin7, enable);
//
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin1, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin2, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortD_Pin7, enable);
    }
    else if( 0 == StrCmpA(pstr, "2", 1) ) //channel 1
    {
//        Grf_GPIO_SetPortPull(GPIO_CH2,GPIO_PORTA,enable);
//        Grf_GPIO_SetPortPull(GPIO_CH2,GPIO_PORTB,enable);
//        Grf_GPIO_SetPortPull(GPIO_CH2,GPIO_PORTC,enable);
//        Grf_GPIO_SetPortPull(GPIO_CH2,GPIO_PORTD,enable);

        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin0, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin1, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin2, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin3, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin4, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin5, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin6, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin7, enable);

        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin0, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin1, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin2, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin3, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin4, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin7, enable);

//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin0, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin1, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin2, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin3, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin4, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin5, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin6, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin7, enable);

        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin0, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin1, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin2, enable);
        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin3, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin4, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin5, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin6, enable);
//        Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortD_Pin7, enable);
    }

}

void GPIO_IntMode_Set(uint32 Mode,uint8 * pstr)
{
    if(0 == StrCmpA(pstr, "0", 1))  //channel 0
    {
        Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTA, Mode);
        Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTB, Mode);
        Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTC, Mode);
        Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTD, Mode);

//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortA_Pin7,Mode);
//
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortB_Pin7,Mode);
//
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortC_Pin7,Mode);
//
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH0,GPIOPortD_Pin7,Mode);
    }
    else if(0 == StrCmpA(pstr, "1", 1))  //channel 1
    {
        Gpio_SetPortIntMode(GPIO_CH1, GPIO_PORTA, Mode);
        Gpio_SetPortIntMode(GPIO_CH1, GPIO_PORTB, Mode);
        Gpio_SetPortIntMode(GPIO_CH1, GPIO_PORTC, Mode);
        Gpio_SetPortIntMode(GPIO_CH1, GPIO_PORTD, Mode);

//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortA_Pin7,Mode);
//
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortB_Pin7,Mode);
//
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortC_Pin7,Mode);
//
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin0,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin1,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin2,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin5,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin6,Mode);
//        Gpio_SetIntMode(GPIO_CH1,GPIOPortD_Pin7,Mode);
    }
    else if(0 == StrCmpA(pstr, "2", 1))  //channel 2
    {
//        Gpio_SetPortIntMode(GPIO_CH2, GPIO_PORTA, Mode);
//        Gpio_SetPortIntMode(GPIO_CH2, GPIO_PORTB, Mode);
//        Gpio_SetPortIntMode(GPIO_CH2, GPIO_PORTC, Mode);
//        Gpio_SetPortIntMode(GPIO_CH2, GPIO_PORTD, Mode);


        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin0,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin1,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin2,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin3,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin4,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin5,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin6,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin7,Mode);

        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin0,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin1,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin2,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin3,Mode);
//        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin4,Mode);
//        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin5,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin6,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortB_Pin7,Mode);

        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin0,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin1,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin2,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin3,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin4,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin5,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin6,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortC_Pin7,Mode);

        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin0,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin1,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin2,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin3,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin4,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin5,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin6,Mode);
        Gpio_SetIntMode(GPIO_CH2,GPIOPortD_Pin7,Mode);
    }
}

void GPIO_int_init(uint32 enable,uint8 * pstr)
{
    if (enable)
    {
        if(0 == StrCmpA(pstr, "0", 1))  //channel 0
        {
            IntRegister(INT_ID_GPIO0 ,GpioInt0);

            IntPendingClear(INT_ID_GPIO0);
            IntEnable(INT_ID_GPIO0);

            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin0,GPIOPortA_Pin0_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin1,GPIOPortA_Pin1_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin2,GPIOPortA_Pin2_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin3,GPIOPortA_Pin3_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin4,GPIOPortA_Pin4_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin5,GPIOPortA_Pin5_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin6,GPIOPortA_Pin6_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortA_Pin7,GPIOPortA_Pin7_Handler);

//            Gpio_ClearPortInt_Level(GPIO_CH0,GPIO_PORTA);
//            Gpio_ClearPortInt_Level(GPIO_CH0,GPIO_PORTB);
//            Gpio_ClearPortInt_Level(GPIO_CH0,GPIO_PORTC);
//            Gpio_ClearPortInt_Level(GPIO_CH0,GPIO_PORTD);

            Gpio_EnablePortInt(GPIO_CH0,GPIO_PORTA);

//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin0);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin1);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin2);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin3);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin4);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin5);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin6);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortA_Pin7);

            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin0,GPIOPortB_Pin0_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin1,GPIOPortB_Pin1_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin2,GPIOPortB_Pin2_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin3,GPIOPortB_Pin3_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin4,GPIOPortB_Pin4_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin5,GPIOPortB_Pin5_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin6,GPIOPortB_Pin6_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortB_Pin7,GPIOPortB_Pin7_Handler);

            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin0,GPIOPortC_Pin0_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin1,GPIOPortC_Pin1_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin2,GPIOPortC_Pin2_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin3,GPIOPortC_Pin3_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin4,GPIOPortC_Pin4_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin5,GPIOPortC_Pin5_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin6,GPIOPortC_Pin6_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortC_Pin7,GPIOPortC_Pin7_Handler);

            GpioIsrRegister(GPIO_CH0,GPIOPortD_Pin0,GPIOPortD_Pin0_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortD_Pin1,GPIOPortD_Pin1_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortD_Pin2,GPIOPortD_Pin2_Handler);
            GpioIsrRegister(GPIO_CH0,GPIOPortD_Pin3,GPIOPortD_Pin3_Handler);

            Gpio_EnablePortInt(GPIO_CH0,GPIO_PORTB);
            Gpio_EnablePortInt(GPIO_CH0,GPIO_PORTC);
            Gpio_EnablePortInt(GPIO_CH0,GPIO_PORTD);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin0);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin1);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin2);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin3);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin4);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin5);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin6);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortB_Pin7);
//
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin0);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin1);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin2);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin3);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin4);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin5);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin6);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortC_Pin7);
//
//            Gpio_EnableInt(GPIO_CH0,GPIOPortD_Pin0);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortD_Pin1);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortD_Pin2);
//            Gpio_EnableInt(GPIO_CH0,GPIOPortD_Pin3);


        }
        else if(0 == StrCmpA(pstr, "1", 1))  //channel 1
        {
            IntRegister(INT_ID_GPIO1 ,GpioInt1);
            IntPendingClear(INT_ID_GPIO1);
            IntEnable(INT_ID_GPIO1);

            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin0,GPIOPortA_Pin0_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin1,GPIOPortA_Pin1_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin2,GPIOPortA_Pin2_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin3,GPIOPortA_Pin3_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin4,GPIOPortA_Pin4_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin5,GPIOPortA_Pin5_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin6,GPIOPortA_Pin6_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortA_Pin7,GPIOPortA_Pin7_Handler);

            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin0,GPIOPortB_Pin0_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin1,GPIOPortB_Pin1_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin2,GPIOPortB_Pin2_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin3,GPIOPortB_Pin3_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin4,GPIOPortB_Pin4_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin5,GPIOPortB_Pin5_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin6,GPIOPortB_Pin6_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortB_Pin7,GPIOPortB_Pin7_Handler);

            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin0,GPIOPortC_Pin0_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin1,GPIOPortC_Pin1_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin2,GPIOPortC_Pin2_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin3,GPIOPortC_Pin3_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin4,GPIOPortC_Pin4_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin5,GPIOPortC_Pin5_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin6,GPIOPortC_Pin6_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortC_Pin7,GPIOPortC_Pin7_Handler);

            GpioIsrRegister(GPIO_CH1,GPIOPortD_Pin0,GPIOPortD_Pin0_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortD_Pin1,GPIOPortD_Pin1_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortD_Pin2,GPIOPortD_Pin2_Handler);
            GpioIsrRegister(GPIO_CH1,GPIOPortD_Pin3,GPIOPortD_Pin3_Handler);

            Gpio_EnablePortInt(GPIO_CH1,GPIO_PORTA);
            Gpio_EnablePortInt(GPIO_CH1,GPIO_PORTB);
            Gpio_EnablePortInt(GPIO_CH1,GPIO_PORTC);
            Gpio_EnablePortInt(GPIO_CH1,GPIO_PORTD);

//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin0);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin1);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin2);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin3);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin4);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin5);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin6);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortA_Pin7);
//
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin0);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin1);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin2);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin3);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin4);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin5);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin6);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortB_Pin7);
//
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin0);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin1);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin2);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin3);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin4);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin5);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin6);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortC_Pin7);
//
//            Gpio_EnableInt(GPIO_CH1,GPIOPortD_Pin0);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortD_Pin1);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortD_Pin2);
//            Gpio_EnableInt(GPIO_CH1,GPIOPortD_Pin3);
        }
        else if(0 == StrCmpA(pstr, "2", 1))  //channel 2
        {
            IntRegister(INT_ID_GPIO2 ,GpioInt2);
            IntPendingClear(INT_ID_GPIO2);
            IntEnable(INT_ID_GPIO2);

            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin0,GPIOPortA_Pin0_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin1,GPIOPortA_Pin1_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin2,GPIOPortA_Pin2_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin3,GPIOPortA_Pin3_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin4,GPIOPortA_Pin4_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin5,GPIOPortA_Pin5_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin6,GPIOPortA_Pin6_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin7,GPIOPortA_Pin7_Handler);

            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin0,GPIOPortB_Pin0_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin1,GPIOPortB_Pin1_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin2,GPIOPortB_Pin2_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin3,GPIOPortB_Pin3_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin4,GPIOPortB_Pin4_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin5,GPIOPortB_Pin5_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin6,GPIOPortB_Pin6_Handler);
            GpioIsrRegister(GPIO_CH2,GPIOPortB_Pin7,GPIOPortB_Pin7_Handler);

//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin0,GPIOPortC_Pin0_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin1,GPIOPortC_Pin1_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin2,GPIOPortC_Pin2_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin3,GPIOPortC_Pin3_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin4,GPIOPortC_Pin4_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin5,GPIOPortC_Pin5_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin6,GPIOPortC_Pin6_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortC_Pin7,GPIOPortC_Pin7_Handler);
//
//            GpioIsrRegister(GPIO_CH2,GPIOPortD_Pin0,GPIOPortD_Pin0_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortD_Pin1,GPIOPortD_Pin1_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortD_Pin2,GPIOPortD_Pin2_Handler);
//            GpioIsrRegister(GPIO_CH2,GPIOPortD_Pin3,GPIOPortD_Pin3_Handler);


            Gpio_EnablePortInt(GPIO_CH2,GPIO_PORTA);
            Gpio_EnablePortInt(GPIO_CH2,GPIO_PORTB);

//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin0);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin1);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin2);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin3);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin4);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin5);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin6);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortB_Pin7);

//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin0);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin1);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin2);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin3);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin4);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin5);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin6);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortC_Pin7);
//
//            Gpio_EnableInt(GPIO_CH2,GPIOPortD_Pin0);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortD_Pin1);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortD_Pin2);
//            Gpio_EnableInt(GPIO_CH2,GPIOPortD_Pin3);

        }
    }
    else
    {
        if(0 == StrCmpA(pstr, "0", 1))  //channel 0
        {
            IntDisable(INT_ID_GPIO0);

            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin0);
            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin1 );
            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin2);
            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin3);
            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin4);
            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin5);
            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin6);
            GPIOIsrUnRegister(GPIO_CH0,GPIOPortA_Pin7);


            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin0);
            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin1);
            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin2);
            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin3);
            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin4);
            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin5);
            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin6);
            Gpio_DisableInt(GPIO_CH0,GPIOPortA_Pin7);

            IntUnregister(INT_ID_GPIO2);
        }
        else if(0 == StrCmpA(pstr, "1", 1))  //channel 1
        {
            IntDisable(INT_ID_GPIO1);

            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin0);
            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin1);
            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin2);
            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin3);
            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin4);
            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin5);
            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin6);
            GPIOIsrUnRegister(GPIO_CH1,GPIOPortA_Pin7);

            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin0);
            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin1);
            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin2);
            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin3);
            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin4);
            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin5);
            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin6);
            Gpio_DisableInt(GPIO_CH1,GPIOPortA_Pin7);

            IntUnregister(INT_ID_GPIO2);
        }
        else if(0 == StrCmpA(pstr, "2", 1))  //channel 2
        {
            IntDisable(INT_ID_GPIO2);

            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin0 );
            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin1);
            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin2);
            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin3);
            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin4);
            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin5);
            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin6);
            GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin7);

            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin0);
            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin1);
            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin2);
            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin3);
            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin4);
            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin5);
            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin6);
            Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin7);

            IntUnregister(INT_ID_GPIO2);
        }
    }
}

rk_err_t GPIOTestCmdParse(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint32 cmd;
    static uint32 pull_flag = ENABLE;

    StrCnt = ShellItemExtract(pstr,&pItem);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    cmd = pstr[0];

    pItem += StrCnt;
    pItem++;    //remove '.',the point is the useful item

    switch (cmd)
    {
        case 'I': // init cmd ,to be init the dev

            break;

        case '0': //end test
            printf("gpio test Exit\n");
            return RK_EXIT;

        case '1':
            GPIO_Iomux_Set(pItem);
            //GPIO_Pull_Set(ENABLE,pItem);
            GPIO_Direction_Set(0xffffffff,pItem);
            GPIO_Level_Set(GPIO_LOW,pItem);
            printf("GPIO OUT,level is low!\n");
            break;

        case '2':
            GPIO_Iomux_Set(pItem);
            //GPIO_Pull_Set(ENABLE,pItem);
            GPIO_Direction_Set(0xffffffff,pItem);
            GPIO_Level_Set(GPIO_HIGH,pItem);
            printf("GPIO OUT,level is high!\n");
            break;

        case '3':
        {
            GPIO_Iomux_Set(pItem);
            GPIO_Pull_Set(ENABLE,pItem);

//            Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);

            if(*pItem == '0')
            {
                GPIO_Direction_Set(0x000000ff,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);

                //Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTA);
                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTB);
                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTC);
                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTD);
            }
            else if(*pItem == '1')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);   //GPIO 0 portA set as GPIO_OUT

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                DelayMs(5);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTA);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTB);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTC);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTD);
            }
            else if(*pItem == '2')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);   //GPIO 0 portA set as GPIO_OUT

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);

                DelayMs(5);
                Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTA);
                Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTB);
                Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTC);
                //Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTD);
            }
            else if(*pItem == '3')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Pull_Set(ENABLE,&chn);

                GPIO_Direction_Set(0x00ff0000,&chn);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_LOW);

                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTA);
            }

            printf("GPIO IN ,input level is low!\n");
        }
            break;

        case '4':
        {
            GPIO_Iomux_Set(pItem);
            GPIO_Pull_Set(ENABLE,pItem);

//            Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);

            if(*pItem == '0')
            {
                GPIO_Direction_Set(0x000000ff,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);

//                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTA);
                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTB);
                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTC);
                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTD);
            }
            else if(*pItem == '1')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);   //GPIO 0 portA set as GPIO_OUT

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                DelayMs(5);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTA);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTB);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTC);
                Gpio_GetPortLevel(GPIO_CH1, GPIO_PORTD);
            }
            else if(*pItem == '2')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);   //GPIO 0 portA set as GPIO_OUT

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);

                DelayMs(5);
                Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTA);
                Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTB);
                Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTC);
                //Gpio_GetPortLevel(GPIO_CH2, GPIO_PORTD);
            }
            else if(*pItem == '3')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Pull_Set(ENABLE,&chn);

                GPIO_Direction_Set(0x00ff0000,&chn);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_HIGH);

                Gpio_GetPortLevel(GPIO_CH0, GPIO_PORTA);
            }

            printf("GPIO IN ,input level is high!\n");
        }
            break;

        case '5':
            GPIO_Iomux_Set(pItem);
            if (pull_flag == ENABLE)
            {
                GPIO_Pull_Set(DISABLE,pItem);
                pull_flag = DISABLE;
                printf("Gpio Pull Disable!\n");
            }
            else
            {
                GPIO_Pull_Set(ENABLE,pItem);
                pull_flag = ENABLE;
                printf("Gpio Pull Enable!\n");
            }

            break;

        case '6':
        {
            GPIO_Iomux_Set(pItem);
            GPIO_Pull_Set(ENABLE,pItem);

            if(*pItem == '0')
            {
                GPIO_int_init(1,pItem);
                GPIO_Direction_Set(0x000000ff,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                GPIO_IntMode_Set(IntrTypeRisingEdge,pItem);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
            }
            else if(*pItem == '1')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                //GPIO_int_init(1,pItem);

                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH1,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                GPIO_IntMode_Set(IntrTypeRisingEdge,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                Gpio_UnMaskPortInt(GPIO_CH1, 0x00000000);
            }
            else if(*pItem == '2')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                //GPIO_int_init(1,pItem);
                GPIO_Direction_Set(0x00000000,pItem); //0x00003000
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH1,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                GPIO_IntMode_Set(IntrTypeRisingEdge,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                Gpio_UnMaskPortInt(GPIO_CH2, 0x00000000);
            }
            else if(*pItem == '3')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Pull_Set(ENABLE,&chn);
                GPIO_int_init(1,&chn);

                GPIO_Direction_Set(0x00ff0000,&chn);   //portC as GPIO_OUT
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_LOW);

                Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTA, IntrTypeRisingEdge);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_HIGH);

            }
        }

            printf("GPIO int Test: RisingEdge!\n");
            break;

        case '7':
            GPIO_Iomux_Set(pItem);

            GPIO_Pull_Set(ENABLE,pItem);

            if(*pItem == '0')
            {
                GPIO_int_init(1,pItem);
                GPIO_Direction_Set(0x000000ff,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
            }
            else if(*pItem == '1')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                //GPIO_int_init(1,pItem);
                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH1,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                GPIO_IntMode_Set(IntrTypeFallingEdge,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                Gpio_UnMaskPortInt(GPIO_CH1, 0x00000000);
            }
            else if(*pItem == '2')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Direction_Set(0x00000000,pItem);//0x00003000
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH1,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                GPIO_IntMode_Set(IntrTypeFallingEdge,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                Gpio_UnMaskPortInt(GPIO_CH1, 0x00000000);
            }
            else if(*pItem == '3')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
//                GPIO_Pull_Set(ENABLE,&chn);
                Grf_GPIO_SetPortPull(GPIO_CH0,GPIO_PORTA,ENABLE);
                GPIO_int_init(1,&chn);
                GPIO_Direction_Set(0x00ff0000,&chn); //portC as GPIO_OUT

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_HIGH);
                Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTA, IntrTypeRisingEdge);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_LOW);
            }

            printf("GPIO int Test: FallingEdge!\n");
            break;

        case '8':
            GPIO_Iomux_Set(pItem);
            GPIO_Pull_Set(ENABLE,pItem);

            if(*pItem == '0')
            {
                GPIO_int_init(1,pItem);
                GPIO_Direction_Set(0x000000ff,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                GPIO_IntMode_Set(IntrTypeHighLevel,pItem);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
            }
            else if(*pItem == '1')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH1,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                GPIO_IntMode_Set(IntrTypeHighLevel,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                Gpio_UnMaskPortInt(GPIO_CH1, 0x00000000);
            }
            else if(*pItem == '2')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);

                GPIO_Direction_Set(0x00000000,pItem); //0x00003000
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH2,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                GPIO_IntMode_Set(IntrTypeHighLevel,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                Gpio_UnMaskPortInt(GPIO_CH2, 0x00000000);

            }
            else if(*pItem == '3')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                //GPIO_Pull_Set(ENABLE,&chn);
                Grf_GPIO_SetPortPull(GPIO_CH0,GPIO_PORTA,ENABLE);

                GPIO_Direction_Set(0x00ff0000,&chn); //portC as gpio_in
                GPIO_int_init(1,&chn);

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_LOW);
                Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTA, IntrTypeHighLevel);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_HIGH);
            }

            printf("GPIO int Test: HighLevel!\n");
            break;

        case '9':
            GPIO_Iomux_Set(pItem);
            GPIO_Pull_Set(ENABLE,pItem);

            if(*pItem == '0')
            {
                GPIO_int_init(1,pItem);
                GPIO_Direction_Set(0x000000ff,pItem);

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                GPIO_IntMode_Set(IntrTypeLowLevel,pItem);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
            }
            else if(*pItem == '1')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);

                GPIO_Direction_Set(0x00000000,pItem);
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH1,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                GPIO_IntMode_Set(IntrTypeLowLevel,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                Gpio_UnMaskPortInt(GPIO_CH1,0x00000000);

            }
            else if(*pItem == '2')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);

                GPIO_Direction_Set(0x00000000,pItem); //0x00003000
                GPIO_Direction_Set(0x000000ff,&chn);

                Gpio_MaskPortInt(GPIO_CH2,0xffffffff);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_HIGH);
                GPIO_IntMode_Set(IntrTypeLowLevel,pItem);
                DelayMs(10);
                GPIO_int_init(1,pItem);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTA,GPIO_LOW);
                Gpio_UnMaskPortInt(GPIO_CH2,0x00000000);
            }
            else if(*pItem == '3')
            {
                uint8 chn;
                chn = '0';
                GPIO_Iomux_Set(&chn);
                GPIO_Pull_Set(ENABLE,&chn);
                GPIO_int_init(1,&chn);

                GPIO_Direction_Set(0x00ff0000,&chn);   //portC as gpio_in

                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_HIGH);
                Gpio_SetPortIntMode(GPIO_CH0, GPIO_PORTA, IntrTypeLowLevel);
                DelayMs(10);
                Gpio_SetPortLevel(GPIO_CH0,GPIO_PORTC,GPIO_LOW);
            }

            printf("GPIO int Test: LowLevel!\n");
            break;

        case 'a'://channel 0 mux to io
            GPIOTEST_IOMuxPin_Set(GPIO_CH0,pItem);
            break;

        case 'b'://channel 1 mux to io
            GPIOTEST_IOMuxPin_Set(GPIO_CH1,pItem);
            break;

        case 'c'://channel 2 mux to io
            GPIOTEST_IOMuxPin_Set(GPIO_CH2,pItem);
            break;

        case 'd'://channel 0 output low level
            GPIOTEST_IOMuxPin_Set(GPIO_CH0,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH0,GPIO_OUT,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH0,GPIO_LOW,pItem);
            break;

        case 'e'://channel 1 output low level
            GPIOTEST_IOMuxPin_Set(GPIO_CH1,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_OUT,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH1,GPIO_LOW,pItem);
            break;

        case 'f'://channel 2 output low level
            GPIOTEST_IOMuxPin_Set(GPIO_CH2,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_OUT,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH2,GPIO_LOW,pItem);
            break;

        case 'g'://channel 0 output high level
            GPIOTEST_IOMuxPin_Set(GPIO_CH0,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH0,GPIO_OUT,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH0,GPIO_HIGH,pItem);
            break;

        case 'i'://channel 1 output high level
            GPIOTEST_IOMuxPin_Set(GPIO_CH1,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_OUT,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH1,GPIO_HIGH,pItem);
            break;

        case 'j'://channel 2 output high level
            GPIOTEST_IOMuxPin_Set(GPIO_CH2,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_OUT,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH2,GPIO_HIGH,pItem);
            break;

        case 'k'://channel 0 output low level
            GPIOTEST_IOMuxPin_Set(GPIO_CH0,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH0,GPIO_IN,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH0,GPIO_LOW,pItem);
            break;

        case 'l'://channel 1 output low level
            GPIOTEST_IOMuxPin_Set(GPIO_CH1,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_IN,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH1,GPIO_LOW,pItem);
            break;

        case 'm'://channel 2 output low level
            GPIOTEST_IOMuxPin_Set(GPIO_CH2,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_IN,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH2,GPIO_LOW,pItem);
            break;

        case 'n'://channel 0 output high level
            GPIOTEST_IOMuxPin_Set(GPIO_CH0,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH0,GPIO_IN,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH0,GPIO_HIGH,pItem);
            break;

        case 'o'://channel 1 output high level
            GPIOTEST_IOMuxPin_Set(GPIO_CH1,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_IN,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH1,GPIO_HIGH,pItem);
            break;

        case 'p'://channel 2 output high level
            GPIOTEST_IOMuxPin_Set(GPIO_CH2,pItem);
            GPIOTEST_SetPin_Direc(GPIO_CH2,GPIO_IN,pItem);
            GPIOTEST_SetPinLevel(GPIO_CH2,GPIO_HIGH,pItem);
            break;


        case 'h':

            GPIOTestHelpInfo();
            break;

        default:
            ret = RK_ERROR;
            break;

    }

    DelayMs(10);
	GPIOTestHelpInfo();
    return ret;
}
#endif

