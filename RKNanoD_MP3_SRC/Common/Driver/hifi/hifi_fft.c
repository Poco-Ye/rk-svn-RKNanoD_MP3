/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, WJR
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi_fft.c
* Owner: WJR
* Date: 2014.11.28
* Time: 16:50:28
* Desc: 
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.28     16:50:28   1.0
********************************************************************************************
*/

#include "SysInclude.h"
//#ifdef __CPU_NANOD_LIB_HIFI_FFT_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define                                   
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "hifi.h"
#include "hifi_fft.h"
#include "Hw_hifi.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define     					 
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_HIFI_FFT_READ_  __attribute__((section("cpu_nanod_lib_hifi_fft_read")))
#define _CPU_NANOD_LIB_HIFI_FFT_WRITE_ __attribute__((section("cpu_nanod_lib_hifi_fft_write")))
#define _CPU_NANOD_LIB_HIFI_FFT_INIT_  __attribute__((section("cpu_nanod_lib_hifi_fft_init")))
#define _CPU_NANOD_LIB_HIFI_FFT_SHELL_  __attribute__((section("cpu_nanod_lib_hifi_fft_shell")))

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define     	     				 
*
*---------------------------'------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define     	     				 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare     	     				 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define                    				 
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FFT_Set_CFG
** Input:UINT32 HifiId,int N_point
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.28
** Time: 16:50:39
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_FFT_READ_
READ API rk_err_t FFT_Set_CFG(UINT32 HifiId,int num,int type)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->FFT_CFG = type;  
    if(num==64)
    {
       pHifi->FFT_CFG |=FFT_MODE_64;
    }
    else if(num ==128)
    {
       pHifi->FFT_CFG |=FFT_MODE_128;
    }
     else if(num ==256)
    {
       pHifi->FFT_CFG |=FFT_MODE_256;
    }
      else if(num ==512)
    {
       pHifi->FFT_CFG |=FFT_MODE_512;
    }
       else if(num ==1024)
    {
       pHifi->FFT_CFG |=FFT_MODE_1024;
    }
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define      				 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define                    			 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define         			 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define             	     			 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define              	  		 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define              	  		 
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define              	  		 
*
*---------------------------------------------------------------------------------------------------------------------
*/



//#endif

