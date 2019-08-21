
#include "SysInclude.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define                                   
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _HIFI_TEST_

#include "Device.h"	
#include "hifi.h"
#include "Hw_hifi.h"
#include "dma.h"

#include "hifi_ape.h"
#include "hifi_test_data_ape.h"

#include "hifi_alac.h"
#include "hifi_test_data_alac.h"

#include "hifi_flac.h"
#include "hifi_test_data_flac.h"

#include "hifi_fft.h"
#include "hifi_test_data_fft.h"

#include "hifi_mac.h"


#define DMA_TRAN_FFT
#define DMA_TRAN_APE
#define DMA_TRAN_ALAC
#define DMA_TRAN_FLAC
#define DMA_TRAN_MAC
/*******************************************************************************
** Name: SHELL_APE_TEST
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 16:21:38
*******************************************************************************/







SHELL API rk_err_t ape_test(int com_level,int *input)
{
    int count = 2048;
    int i,j,k;
    int count_rc =0;
    int  SysTick,Tick,Time; 
    #ifdef APE_VERSION_3990
    int version = 3990;
    #else    
    int version = 3970;
    #endif
    int compress_level = com_level;
    int circle =2048/count;
   //SysTick = SysTickCounter;
   /**********计时*************/
   uint64 loadcount = 80*1000;
  int timer_time = 0;
  IntUnregister(INT_ID_TIMER0);
  IntPendingClear(INT_ID_TIMER0);
  IntDisable(INT_ID_TIMER0);
  
  IntRegister(INT_ID_TIMER0 ,NULL);
  IntPendingClear(INT_ID_TIMER0);
  IntEnable(INT_ID_TIMER0); 
  
  TimerPeriodSet(TIMER0,loadcount,0);
  HIFI_DMA_TO_register();
  timer_time = TimerGetCount(TIMER0);
  
    for(k =0;k<960;k++)
    {
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_L);//开始传输配置数据和初始化系数(不往fifo送)
    APE_Set_CFG(0,version,compress_level);  
    APE_Clear(0);
    Hifi_Set_ACC_clear(0);//fpga 内部已实现    
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    Hifi_Enable_FUN_DONE_FLAG(0);   
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_L);//可以开始往fifo传数据，并且可以取数据
    
	  for(i=0;i<circle;i++)
    {
    /*********左声道************/
       
        Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_L);//开始传输配置数据和初始化系数(不往fifo送) 
        APE_Set_CFG(0,version,compress_level); 
        Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_L);//可以开始往fifo传数据，并且可以取数据      
       
#ifdef DMA_TRAN_APE
        HIFI_DMA_TO_ACC(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_L[i*count]);   
#else  
        count_rc = 0;
        HIFITranData_fifo(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_L[i*count],&count_rc);
        printf("count_read %d\n ",count_rc);
#endif 
        

        /*********右声道************/
        Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_R);//开始传输配置数据和初始化系数(不往fifo送)   
        APE_Set_CFG(0,version,compress_level); 
        Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_R);//可以开始往fifo传数据，并且可以取数据
        count_rc = 0;
        {
#ifdef DMA_TRAN_APE
        HIFI_DMA_TO_ACC(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_R[i*count]);   
#else
        HIFITranData_fifo(&input[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_ape_R[i*count],&count_rc);
        printf("count_read %d\n ",count_rc);
#endif    
        }

      }   
    }
     timer_time -= TimerGetCount(TIMER0);
    printf("time %d \n",timer_time);   
    HIFI_DMA_TO_Unregister();
    /**********关闭timer**************/
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0);
    {   
       
        for(i=0;i<count*circle;i++)
        {
           if((test_output_ape_L[i] != output_ape[i])  )
            {
              break;
            }
        }
         for(j=0;j<count*circle;j++)
        {
           if((test_output_ape_R[j] != output_ape[j]))
            {
              break;
            }
        }
        if((i == count*circle) &&(j == count*circle) )    
        {
            printf("\n%d %d  %d ape test over\n",version,compress_level,circle*count);            
            return  RK_SUCCESS;
        }
        else
        {
            printf("%d %d  %d ape test error from L:%d R:%d \n",version,compress_level,circle*count,i,j);                
            return RK_ERROR  ;
        }
    }
}


SHELL API rk_err_t hifi_ape_shell(void)
{
    {
  printf("\n-------------APE START-------------\n");
  ape_test(2000,test_data_ape2000);
  ape_test(3000,test_data_ape3000);
  ape_test(4000,test_data_ape4000);
  #ifdef APE_VERSION_3990
  ape_test(5000,test_data_ape5000);
  #endif 
   return RK_EXIT;
    }
}


/*******************************************************************************
** Name: SHELL_ALAC_TEST
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 18:25:06
*******************************************************************************/



SHELL API rk_err_t hifi_alac_shell(void)
{
    
    
    int i,k;
    int count_rc=0;
    int  SysTick,Tick,Time; 
    #ifdef  alac_test1
    int lpc_order = 0x20;
    int lpc_quant = 0x9;
    int lpc_bps  = 0x11;
    int frm_size = 128;
    #endif
    #ifdef  alac_test2
    int lpc_order = 0x8;
    int lpc_quant = 0x9;
    int lpc_bps  = 0x11;
    int frm_size = 128;
    #endif 
    #ifdef  alac_test3
    int lpc_order = 0x8;
    int lpc_quant = 0x9;
    int lpc_bps  = 0x11;
    int frm_size = 4096;
    #endif 
    int count = frm_size;  
    int circle = frm_size/count;
     /**********计时*************/
    uint64 loadcount = 80*1000;
    int timer_time = 0;
    printf("\n-------------ALAC START-------------\n");    
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0);

    IntRegister(INT_ID_TIMER0 ,NULL);
    IntPendingClear(INT_ID_TIMER0);
    IntEnable(INT_ID_TIMER0); 
    TimerPeriodSet(TIMER0,loadcount,0); 
    timer_time = TimerGetCount(TIMER0);
  
    Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_ALAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    HIFI_DMA_TO_register();
    for(k =0;k<96;k++)
    {
    Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_ALAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
    Alac_Set_CFG(0,lpc_order,lpc_quant,lpc_bps);
    HIFITranData((UINT32*)alac_coef,(uint32*)ALAC_COEF_ADD,lpc_order);
    Hifi_Set_ACC_XFER_Start(0,frm_size,HIfi_ACC_TYPE_ALAC);//可以开始往fifo传数据，并且可以取数据
    for(i=0;i<circle;i++)
    {
#ifdef DMA_TRAN_ALAC
        HIFI_DMA_TO_ACC(&test_data_alac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_alac[i*count]);   
#else   
	   count_rc=0;
        HIFITranData_fifo(&test_data_alac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_alac[i*count],&count_rc);
#endif 
     }
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);
    }
 
    timer_time -= TimerGetCount(TIMER0);
    printf("\n time %d \n",timer_time);  
    HIFI_DMA_TO_Unregister();

    /**********关闭timer**************/
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0);
    {   
       
        for(i=0;i<count;i++)
        {
            if( test_output_alac[i] != output_alac[i])
            {
              break;
            }
        }
        if(i == count )    
        {
            printf("%d  %d num alac test over\n",lpc_order,frm_size);  
					  return RK_EXIT;
        }
        else
        {
            printf("alac test error from %d \n ",i);   
            return RK_ERROR  ;
        }
    }
 
}

/*******************************************************************************
** Name: SHELL_FLAC_TEST
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 18:23:23
*******************************************************************************/


SHELL API rk_err_t hifi_flac_shell(void)
{    
    int i,k;
    int count_rc=0;
    int  SysTick,Tick,Time; 
    #ifdef flac_test1
    int lpc_order = 0x20;
    int lpc_quant = 0xd;
    int frm_size = 128;
    #endif 
    #ifdef flac_test2
    int lpc_order = 0xc;
    int lpc_quant = 0xd;
    int frm_size = 128;
    #endif 
    #ifdef flac_test3
    int lpc_order = 0x8;
    int lpc_quant = 0xe;
    int frm_size = 4096;
    #endif 
    int count = frm_size; 
    int circle =frm_size/count;
  uint64 loadcount = 80*1000;
  int timer_time = 0; 
  printf("\n-------------FLAC START-------------\n");
  /**********计时*************/ 
  IntUnregister(INT_ID_TIMER0);
  IntPendingClear(INT_ID_TIMER0);
  IntDisable(INT_ID_TIMER0);
  IntRegister(INT_ID_TIMER0 ,NULL);
  IntPendingClear(INT_ID_TIMER0);
  IntEnable(INT_ID_TIMER0);   
  TimerPeriodSet(TIMER0,loadcount,0); 
  HIFI_DMA_TO_register();
  timer_time = TimerGetCount(TIMER0);
   
    Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_FLAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);

    for(k =0;k<96;k++)
    {
	Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_FLAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
    Flac_Set_CFG(0,lpc_order,lpc_quant);
    HIFITranData((uint32*)flac_coef,(uint32*)FLAC_COEF_ADD,lpc_order);
    Hifi_Set_ACC_XFER_Start(0,frm_size,HIfi_ACC_TYPE_FLAC);//可以开始往fifo传数据，并且可以取数据
    for(i=0;i<circle;i++)
    {
       
#ifdef DMA_TRAN_FLAC
        HIFI_DMA_TO_ACC(&test_data_flac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_flac[i*count]);   
#else
		count_rc=0;
        HIFITranData_fifo(&test_data_flac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_flac[i*count],&count_rc);
  #endif 
    }
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);
    }

    timer_time -= TimerGetCount(TIMER0);
    printf("time %d \n",timer_time);  
    HIFI_DMA_TO_Unregister();
    /**********关闭timer**************/
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0);
   
    {   
        
        for(i=0;i<count;i++)
        {
            if( test_output_flac[i] != output_flac[i])
            {
              break;
            }
        }
        if(i == count )    
        {
            printf("%d  %d num flac test over\n",lpc_order,frm_size);
					  return RK_EXIT;
        }
        else
        {
            rk_printf_no_time("flac test error from %d \n ",i);  
            return RK_ERROR  ;
        }
    }
  
}

/*******************************************************************************
** Name: SHELL_FFT_TEST
** Input:void   
**注意:
******FFT必须在xfer disable时才能传数据，并且必须使用fun_done标志，才能取输出数据。
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 16:47:18
*******************************************************************************/



SHELL API rk_err_t FFT_TEST(int fft_point,int* test_data_fft,int* output_fft_R ,int* output_fft_I)
{
      
    int i,k;    
	  int count = fft_point;
    int times = 1024/count;
    int  SysTick,Tick,Time; 
    int circle = 192*2;//*1024/count;	  
    /**********计时*************/
    uint64 loadcount = 80*1000;
    int timer_time = 0;
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0);
    IntRegister(INT_ID_TIMER0 ,NULL);
    IntPendingClear(INT_ID_TIMER0);
    IntEnable(INT_ID_TIMER0); 
    TimerPeriodSet(TIMER0,loadcount,0);
    HIFI_DMA_M2M_register();
    timer_time = TimerGetCount(TIMER0);   
    for(k =0;k<circle;k++)
    {
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    Hifi_Enable_FUN_DONE_FLAG(0);
    /****************fft********************/
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    FFT_Set_CFG(0,count,FFT_FLAG);
   
     for(i=0;i<times;i++)
    {
#ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC(&test_data_fft[i*count],(uint32*)FFT_DATR_ADD,count);
#else
    HIFITranData(&test_data_fft[i*count],(uint32*)FFT_DATR_ADD,count);
#endif 
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_FFT);//可以开始往fifo传数据，并且可以取数据
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active);
    Hifi_Clear_FUN_DONE_FLAG(0);//清中断 
    Hifi_Set_ACC_XFER_Disable(0,0,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
#ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC((uint32*)FFT_DATR_ADD,&test_output_fft_R[i*count],count);
    HIFI_DMA_MEM_ACC((uint32*)FFT_DATI_ADD,&test_output_fft_I[i*count],count);
#else
    HIFITranData((uint32*)FFT_DATR_ADD,&test_output_fft_R[i*count],count);  
    HIFITranData((uint32*)FFT_DATI_ADD,&test_output_fft_I[i*count],count);      
#endif
   }
#if 1
    /****************Ifft********************/
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)   
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    FFT_Set_CFG(0,count,IFFT_FLAG); 
    for(i=0;i<times;i++)
    {
#ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC(&output_fft_I[i*count],(uint32*)FFT_DATI_ADD,count);
    HIFI_DMA_MEM_ACC(&output_fft_R[i*count],(uint32*)FFT_DATR_ADD,count);
#else
    HIFITranData(&output_fft_I[i*count],(uint32*)FFT_DATI_ADD,count);
    HIFITranData(&output_fft_R[i*count],(uint32*)FFT_DATR_ADD,count);
#endif 
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_FFT);//可以开始往fifo传数据，并且可以取数据
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);//清中断  
    Hifi_Set_ACC_XFER_Disable(0,0,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
 #ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC((uint32*)FFT_DATR_ADD,&test_output_ifft_R[i*count],count);
#else
    HIFITranData((uint32*)FFT_DATR_ADD,&test_output_ifft_R[i*count],count);  
#endif
   }
    #endif
   }
    timer_time -= TimerGetCount(TIMER0);   
    printf("\nFFT IFFT %d num is %d(ms) \n",count,timer_time);
    HIFI_DMA_M2M_Unregister();
    IntUnregister(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntDisable(INT_ID_TIMER0);
    { 
        for(i=0;i<count;i++)
        {
           int diff_r = test_output_fft_R[i] - output_fft_R[i];
					 int diff_i = test_output_fft_I[i] - output_fft_I[i];
           
            if( (ABS(diff_r) )||(ABS(diff_i)))
            {
              break;
            }
        }
        if(i == count )    
        {
            printf("%d fft test over\n",count);            
        }
        else
        {
            printf("%d fft test error %d\n",count,i); 
	
        }
    }
    { 
        for(i=0;i<count;i++)
        {
           int diff = test_output_ifft_R[i] - test_data_fft[i];
           
            if( ABS(diff) > 5)
            {
              break;
            }
        }
        if(i == count )    
        {
            printf("%d ifft test over\n",count); 
            return  RK_SUCCESS;
        }
        else
        {
            printf("%d ifft test error %d \n ",count,i); 
            return RK_ERROR  ;
        }
    }
 
}

SHELL API rk_err_t hifi_fft_shell(void )
{
  printf("-------------FFT START-------------\n");

   FFT_TEST(64,test_data_fft_64,output_fft_R_64,output_fft_I_64);
   FFT_TEST(128,test_data_fft_128,output_fft_R_128,output_fft_I_128);
   FFT_TEST(256,test_data_fft_256,output_fft_R_256,output_fft_I_256);
   FFT_TEST(512,test_data_fft_512,output_fft_R_512,output_fft_I_512);
   FFT_TEST(1024,test_data_fft_1024,output_fft_R_1024,output_fft_I_1024);
   return RK_EXIT; 

}

__align(4) int test_data_mac[] = {0x94777fff,0xfeff8000,0x88856807,0xfe23456};
__align(4) int test_coef_mac[] = {0x9acbd,0xeffeadc,0xff7000ab,0x3456789a};
int res_l= 0xe56e1bac;
int res_r = 0x036f6cf9;

__align(4) int a[4];
__align(4) int b[4];


SHELL API rk_err_t hifi_mac_shell(void)
{
    int count = 4;
    long long res=0;
    int  SysTick,Tick,Time; 
	  int  circle = 192;//1024/4;
	  int i;
    //SysTick = SysTickCounter;
    for(i=0;i<circle;i++)        
    {
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_MAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    Hifi_Enable_FUN_DONE_FLAG(0);
    MAC_Set_CFG(0,count);
	HIFI_DMA_M2M_register();
#ifdef DMA_TRAN_MAC
    HIFI_DMA_MEM_ACC(test_data_mac,(uint32*)FFT_DATR_ADD,count);
    HIFI_DMA_MEM_ACC(test_coef_mac,(uint32*)FFT_DATI_ADD,count);

#else
   	HIFITranData(test_coef_mac,(uint32*)FFT_DATR_ADD,count);
    HIFITranData(test_data_mac,(uint32*)FFT_DATI_ADD,count);
    
#endif 
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_MAC);//可以开始往fifo传数据，并且可以取数据
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);
    res =MAC_Get_result(0);   
	}

    HIFI_DMA_M2M_Unregister();
    if(res == ((res_r <<32)|res_l))    
    {
        printf("\r\n mac test over\n");            
		return RK_EXIT;
    }
    else
    {
        printf("\r\n mac test error\n ");                
        return RK_ERROR  ;
    }
	
}
/*******************************************************************************
** Name: HIFI_SHELL
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.22
** Time: 9:41:20
*******************************************************************************/

SHELL API rk_err_t HIFI_SHELL(void)
{

  hifi_mac_shell();
   #if 1//提频操作
  FREQ_EnterModule(FREQ_MAX);
  Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
  OBS_output_Source_sel(obs_clk_cal_core);    
    CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_1 << 16) | CAL_CORE_SRC_SEL_PLL) << 15;
    CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | 0) << 12;
    CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_1 << 16) | SYS_CORE_SRC_SEL_PLL) << 3;
    CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | 1) << 0;
  Grf_Memory_Set_High_Speed((0xF << 12));
  Grf_Memory_Set_High_Speed((0x1 << 6));
  #endif
   hifi_fft_shell();
   hifi_alac_shell();
   hifi_flac_shell();
   hifi_ape_shell(); 
   printf("hifi test over\n");   
   return  RK_SUCCESS;
 }
rk_err_t hifi_test(HDC dev, uint8 * pstr)
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
            
        case '1':  
            ret = HIFI_SHELL();
            break;
        case '2':  
            ret = hifi_mac_shell();
            break;
        case '3':  
            ret = hifi_fft_shell();
            break;
        case '4':  
            ret = hifi_alac_shell();
            break;
        case '5':  
            ret = hifi_flac_shell();
            break;

        case '6':  
            ret = hifi_ape_shell();
            break;

        case 'I':
            break;
            
        default:
            ret = RK_ERROR;
            break;
    } 
    printf("================================================================================\n");
    printf(" HIFI Test Menu                                                                  \n");
    printf(" 1. HIFI TEST                                                                    \n");
    printf(" 2. mac TEST                                                                     \n");
    printf(" 3. fft TEST                                                                     \n");
    printf(" 4. alac TEST                                                                    \n");
    printf(" 5. flac TEST                                                                    \n");
    printf(" 6. ape TEST                                                                     \n");
    printf(" 0. EXIT                                                                         \n");
    printf("================================================================================ \n");
   
    return ret;    
}
#endif

