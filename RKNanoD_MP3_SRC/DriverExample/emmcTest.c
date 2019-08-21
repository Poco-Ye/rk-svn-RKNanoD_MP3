/*
********************************************************************************
*
*                Copyright (c): 2015 - 2015 + 5, emmcTest
*                             All rights reserved.
*
* FileName: ..\DriverExample\emmcTest.c
* Owner: emmcTest.c
* Date: 2015.3.2
* Time: 9:07:17
* Desc: 
* History:
*   <author>    <date>       <time>     <version>     <Desc>
*   wangping
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

#ifdef _EMMC_TEST_

#include "Device.h"	
#include "DriverInclude.h"
#include "MDconfig.h"
#include "SDConfig.h"
void EmmcTestPrintfInfo(void)
{
    printf("\r\n");
    printf("================================================================================\n");
    printf(" EMMC Test Menu                                                                 \n");
    printf(" 1. Emmc Test Start                                                             \n");
    printf(" 0. Exit                                                                        \n");
    printf("================================================================================\n");
    printf("\r\n");
}

uint32 ProbeReadBuf[1024];   //FLASH探测时用的PAGE BUF
uint32 ProbeWriteBuf[1024];  //FLASH探测时用的PAGE BUF
void EmmcTest(void)
{
    uint32 i;

    //TestEmmcRet = 0;
	for (i=0; i<512*4; i++)
	{
	    ProbeWriteBuf[i]=i+1;
	    //ProbeWriteBuf[i] = 0xFFFFFFFF;
	}

    memset(ProbeReadBuf, 0, 512*4);
    if (SDM_SUCCESS != SDM_Read(SDM_EMMC_ID, 0, 4, ProbeReadBuf))
    {
        while(1);
    }

    if (SDM_SUCCESS != SDM_Write(SDM_EMMC_ID, 0, 4, ProbeWriteBuf))
    {
        while(1);
    }

    memset(ProbeReadBuf, 0, 512*4);
    if (SDM_SUCCESS != SDM_Read(SDM_EMMC_ID, 0, 4, ProbeReadBuf))
    {
        while(1);
    }

    for(i=0; i<4*128; i++)
    {
        if (ProbeReadBuf[i] != ProbeWriteBuf[i])
        {
            printf("Emmc Test Fail\r\n");
            return;
            //while(1);
        }
    }

    printf("Emmc Test OK\r\n");
    return;
    //while(1);
}

rk_err_t EmmcTestCmdParsing(HDC dev, uint8 * pstr)
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
            break;
        case '1':
        {
            uint emmcCapacity;
            EmmcInit();
            emmcCapacity = EmmcGetCapacity(0xff);

            printf("emmc Capacity is %d\n", emmcCapacity);

            EmmcTest();
            break;
        }
        case '2':
            break;

        case 'h':
            break;
        default:
            ret = RK_ERROR;
            break;
			
    }
	EmmcTestPrintfInfo(); 
    return ret;

}
#endif

