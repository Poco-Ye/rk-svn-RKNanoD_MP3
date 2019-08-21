
#include "SysInclude.h"

#ifdef _MP3_TEST_

#include "hw_mp3_imdct.h"
#include "hw_mp3_syn.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define                                   
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "Device.h"	

rk_err_t MP3_test(HDC dev, uint8 * pstr)
{
    uint32 cmd;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
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
            
        case '3':  
            ret = hw_imdct_shell();
            break;
        case '2':  
            ret = hw_syn_shell();
            break;
        case '1': 
            ret = hw_imdct_shell();
            ret = hw_syn_shell();
            break;
        default:
            ret = RK_ERROR;
            break;
    } 
    printf("================================================================================\n");
    printf(" MP3 Test Menu                                                                 \n");
    printf(" 1. mp3_shell                                                                  \n");
    printf(" 2. hw_syn_shell                                                               \n");
    printf(" 3. hw_imdct_shell                                                             \n");
    printf(" 0. EXIT                                                                       \n");
    printf("===============================================================================\n");
  
    return ret;    
}

#endif
