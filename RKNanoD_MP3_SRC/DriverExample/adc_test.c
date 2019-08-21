/*
********************************************************************************
*                       Copyright (c) 2015 RockChips
*                         All rights reserved.
*
* File Name£º   ADC_Test.c
*
* Description:
*
* History:      <author>          <time>        <version>
*
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _ADC_TEST_

#include "Device.h"
#include "DriverInclude.h"


void ADCTestHelpInfo(void)
{
    printf("================================================================================\n");
    printf(" ADC Test Menu                                                                  \n");
    printf(" 1. ADC Channel 0 DATA READ                                                               \n");
    printf(" 2. ADC Channel 1  DATA READ                                                               \n");
    printf(" 3. ADC Channel 2  DATA READ                                                               \n");
    printf(" 4. ADC Channel 3  DATA READ                                                               \n");
    printf(" 5. ADC Channel 4  DATA READ                                                               \n");
    printf(" 6. ADC Channel 5  DATA READ                                                               \n");
    printf(" 7. ADC Channel 6  DATA READ                                                               \n");
    printf(" 8. ADC Channel 7  DATA READ                                                               \n");
    printf(" h. adc test help menu                                                          \n");
    printf(" 0. EXIT                                                                        \n");
    printf("================================================================================\n");

}
 
void Adc_test_Isr(void)
{
    Adc->ADC_CTRL &= ~ADC_INT_CLEAR;
    printf("adc test isr!\n");
}

void ADCGetChannelData(uint32 chn)
{
    uint32 tmp = 0;
    uint32 adc_val = 0;

    if(chn > ADC_CHANEL_MAX)
        return; //adc has 0 - 7 channels

    Adc->ADC_CTRL = chn	| ADC_START | ADC_POWERUP | ADC_INT_ENBALE;
    printf("Adc channel %d Start read...\n", chn);
    while (1)
    {
        DelayMs(1);
        tmp = Adc->ADC_STAS;
        if ((tmp & 0x01) == 0)
        {
            break;
        }
    }

    adc_val = (UINT32)(Adc->ADC_DATA);
    printf("Adc channel %d read value = %d\n", chn, adc_val);
}


rk_err_t ADCTestCmdParse(HDC * dev,uint8* pstr)
{
    uint32 i = 0,temp;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint32 cmd;
    uint32 pull_flag = ENABLE;

    StrCnt = ShellItemExtract(pstr,&pItem);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    cmd = pstr[0];

    pItem += StrCnt;
    pItem++; 

    AdcInit();
    IntUnregister(INT_ID_SRADC);
    IntRegister(INT_ID_SRADC, (void*)Adc_test_Isr);
    IntEnable(INT_ID_SRADC);

    switch (cmd)
    {
        case '0':
            return RK_EXIT;

        case 'h':
            ADCTestHelpInfo();
            break;

        case '1':
            ADCGetChannelData(0);
            break;
        case '2':
            ADCGetChannelData(1);
            break;
        case '3':
            ADCGetChannelData(2);
            break;
        case '4':
            ADCGetChannelData(3);
            break;
        case '5':
            ADCGetChannelData(4);
            break;
        case '6':
            ADCGetChannelData(5);
            break;
        case '7':
            ADCGetChannelData(6);
            break;
        case '8':
            ADCGetChannelData(7);
            break;            

         default:
            ret = RK_ERROR;
            break;
    }

    ADCTestHelpInfo();
    return ret;
}
#endif

