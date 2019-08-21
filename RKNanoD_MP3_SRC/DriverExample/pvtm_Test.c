/*
********************************************************************************
*                       Copyright (c) 2015 RockChips
*                         All rights reserved.
*
* File Name£º   pvtm_Test.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                                           
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _PVTM_TEST_

#include "Device.h"	
#include "DriverInclude.h"

void PVTM_TestHelpInfo()
{
    printf("================================================================================\n");
    printf(" PVTM Test Menu                                                                 \n");
    printf(" 1. PVTM get counter                                                            \n");
    printf(" 2. PVTM set counter                                                            \n");
    
    printf(" h. PVTM test help menu                                                         \n");
    printf(" 0. EXIT                                                                        \n");
    printf("================================================================================\n");
}

rk_err_t PVTMTestCmdParse(HDC * dev,uint8* pstr)
{
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

    switch(cmd)
    {
        case '0':
            return RK_EXIT;

        case 'h':
            PVTM_TestHelpInfo();
            break;

        case '1':
            {//get
                uint32 cnt;
                PVTM_Start();
                //cnt = PVTM_Get_Cnt();
                cnt = Grf_PVTM_Get_Freq_Cnt();

                printf(" PVTM get cnt is :0x%08x \n",cnt);
            }
            break;

        case '2':
            {//set 
                uint32 cnt;
                PVTM_Start();

                cnt = 1000;
                PVTM_Set_CalCnt(cnt);

                printf(" PVTM set cnt is :%d \n",cnt);
            }
            break;
    }

    PVTM_TestHelpInfo();
    return ret;
}
#endif

