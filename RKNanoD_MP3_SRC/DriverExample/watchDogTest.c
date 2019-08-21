/*
********************************************************************************
*
*                Copyright (c): 2015 - 2015 + 5, WatchDogTest.c
*                             All rights reserved.
*
* FileName: ..\DriverExample\WatchDogTest.c
* Owner: WatchDogTest.c
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

#ifdef _WATCHDOG_TEST_

#include "Device.h"
#include "DriverInclude.h"
void WatchDogTestPrintfInfo(void)
{
    printf("\r\n");
    printf("================================================================================\n");
    printf(" WatchDog Test Menu                                                             \n");
    printf(" 1. Test System Reset when not feed dog, watchdog mode  immediately reset       \n");
    printf(" 2. Test System do not Reset when feed dog, watchdog mode  immediately reset    \n");
    printf(" 3. Test System Reset when not feed dog, watchdog mode  int                     \n");
    printf(" 4. Test System do not Reset when feed dog, watchdog mode   int                 \n");
    printf(" 0. exit                                                                        \n");
    printf("================================================================================\n");
    printf("\r\n");
}

int IsWatchDogTestClearInt = 0;

void WatchDogIsrTest(void)
{
    if(IsWatchDogTestClearInt)
    {
        WatchDogClearIntFlag();
    }
    printf("CCRV = %d\n",WatchDogGetCurrentValue());
    //printf("watch dog isr coming\n");
}

rk_err_t WatchDogTestCmdParsing(HDC dev, uint8 * pstr)
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
        case 'I': // init cmd ,to be init the dev
            //init the dev
            //.......

            break;
        case '0': //end test

            //deinit the dev
            //......

            return RK_EXIT;

        case '1':
            printf(" WatchDog Test start\n");
            WatchDogInit(RESP_MODE_RESET, PCLK_CYCLES_128, PERIOD_RANGE_0X7FFFFFFF);
            printf(" ######\n");
            WatchDogReload();
            WatchDogStart();
            printf(" wait for system reset\n");
            break;

        case '2':
            {
                uint test_time = 100;
                printf(" WatchDog Test start\n");

                WatchDogInit(RESP_MODE_RESET, PCLK_CYCLES_128, PERIOD_RANGE_0X7FFFFFFF);

                WatchDogStart();

                while(test_time)
                {
                    DelayMs(200);
                    printf(" WatchDog Feed\n");
                    WatchDogReload();
                    --test_time;

                    printf("ccvr = %d\n",WatchDogGetCurrentValue());
                }
                WatchDogDeInit();
                printf(" WatchDog Feed OK\n");
                printf(" WatchDog Test end\n");
            }
            break;

        case '3':
            printf(" WatchDog Test start\n");
            IsWatchDogTestClearInt= 0;
            ScuClockGateCtr(PCLK_WDT_GATE, FALSE);
            DelayMs(50);
            IntPendingClear(INT_ID_WDT);
            WatchDogInit(RESP_MODE_INT_RESET, PCLK_CYCLES_128, PERIOD_RANGE_0X07FFFFFF);
            WatchDogIntRegister(WatchDogIsrTest);
            WatchDogEnableInt();
            WatchDogStart();


//            while(1)
//            {
//                //DelayMs(1);

//                printf("ccvr = %d\n",WatchDogGetCurrentValue());
//            }
            printf(" wait for system reset\n");

            break;

        case '4':
            {
                uint test_time = 400;
                printf(" WatchDog Test start\n");
                IsWatchDogTestClearInt = 1;
                IntPendingClear(INT_ID_WDT);
                WatchDogInit(RESP_MODE_INT_RESET, PCLK_CYCLES_128, PERIOD_RANGE_0X07FFFFFF);
                WatchDogIntRegister(WatchDogIsrTest);
                WatchDogEnableInt();
                WatchDogStart();
                while(test_time)
                {
                    DelayMs(50);
                    if(WatchDogGetStat())
                    {
                        WatchDogClearIntFlag();
                    }
                    printf("ccvr = %d\n",WatchDogGetCurrentValue());
                     --test_time;
                }
                WatchDogDisableInt();
                WatchDogDeInit();
                printf(" WatchDog Test end\n");
            }
            break;

        case 'h':

            break;
        default:
            ret = RK_ERROR;
            break;

    }
    WatchDogTestPrintfInfo();
    return ret;

}
#endif

