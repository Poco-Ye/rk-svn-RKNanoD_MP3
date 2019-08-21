/*
********************************************************************************************
*
*				  Copyright (c): 2014 - 2014 + 5, aaron.sun
*							   All rights reserved.
*
* FileName: Cpu\NanoC\lib\hw_vop.h
* Owner: aaron.sun
* Date: 2014.11.21
* Time: 14:42:54
* Desc: vop memory map
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    aaron.sun     2014.11.21     14:42:54   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_HW_VOP_H__
#define __CPU_NANOC_LIB_HW_VOP_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define									 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define						 
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _VOP
{

    uint32 VopMcuCon;
    uint32 VopMcuVersion;
    uint32 VopMcuTiming;
    uint32 VopMcuLcdSize;
    uint32 VopMcuFIFOWaterMark;
    uint32 VopMcuSRT;
    uint32 VopMcuIntEn;
    uint32 VopMcuIntClear;
    uint32 VopMcuIntStatus;
    uint32 VopMcuStatus;
    uint32 VopMcuCmd;
    uint32 VopMcuData;
    uint32 VopMcuStart;
    
}VOP;

#define VopReg              ((VOP *)VOP_BASE)

#define VopPort(n)          ( VopReg + n )


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare							 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare          							 
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

