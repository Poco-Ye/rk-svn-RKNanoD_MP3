/*
********************************************************************************
*
*                Copyright (c): 2015 - 2015 + 5, SdTest.c
*                             All rights reserved.
*
* FileName: ..\DriverExample\SdTest.c
* Owner: SdTest.c
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

#ifdef _SD_TEST_

#include "Device.h"	
#include "DriverInclude.h"
#include "MDconfig.h"
#include "SDConfig.h"
void SdCardTestPrintfInfo(void)
{
    printf("\r\n");
    printf("================================================================================\n");
    printf(" SdCard Test Menu                                                               \n");
    printf(" 1. SdCard Test Start                                                           \n");
    printf(" 0. exit                                                                        \n");
    printf("================================================================================\n");
    printf("\r\n");
}

extern uint32 ProbeReadBuf[1024];   //FLASH探测时用的PAGE BUF
extern uint32 ProbeWriteBuf[1024];  //FLASH探测时用的PAGE BUF
//static uint8 TestRet;
/*
Name:       EmmcTest
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
_ATTR_FLASH_CODE_
void SDM_Test(void)
{
    uint32 i;

    #ifdef _SDCARD_
    
    //TestRet = 0;
	for (i=0; i<512*4; i++)
	{
	    ProbeWriteBuf[i]=i+1;
	    //ProbeWriteBuf[i] = 0xFFFFFFFF;
	}

    if (SDM_SUCCESS != SDCardInit())
    {
        printf("sd card init fail\n");
        return;
        //while(1);
    }
    
    if (SDM_SUCCESS != SDM_Open(SDM_SD_ID, UNKNOW_CARD))
    {
        printf("sd card open fail\n");
        return;
        //while(1);
    }


    printf("sd card capability is %d\n", SDM_GetCapability(SDM_SD_ID));


    memset(ProbeReadBuf, 0, 512*4);
    if (SDM_SUCCESS != SDM_Read(SDM_SD_ID, 4, 4, ProbeReadBuf))
    {
         printf("sd card read fail\n");
         return;
         //while(1);
    }

   
    if (SDM_SUCCESS != SDM_Write(SDM_SD_ID, 0, 4, ProbeWriteBuf))
    {
        printf("sd card write fail\n");
        return;          
        //while(1);
    }

    memset(ProbeReadBuf, 0, 512*4);
    if (SDM_SUCCESS != SDM_Read(SDM_SD_ID, 0, 4, ProbeReadBuf))
    {
        printf("sd card read fail\n");
        return;        
        //while(1);
    }


    for(i=0; i<4*128; i++)
    {
        if (ProbeReadBuf[i] != ProbeWriteBuf[i])
        {
            printf("sd card test fail\n");
            return;              
            //while(1);
        }
    }

    #endif

    
    printf("sd card test ok\n");
    return;   
    //while(1);
}

rk_err_t SdCardTestCmdParsing(HDC dev, uint8 * pstr)
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
            printf("sd card test start\n");
            SDM_Test();
            break;

        case '2':
            break;

        case 'h':
        
            SdCardTestPrintfInfo();
            break;
        default:
            ret = RK_ERROR;
            break;
			
    }
	SdCardTestPrintfInfo();
    return ret;

}

#endif

