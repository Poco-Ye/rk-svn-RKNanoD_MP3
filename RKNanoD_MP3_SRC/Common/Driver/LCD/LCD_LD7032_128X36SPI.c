/*
********************************************************************************
*                   Copyright (c) 2008,Yangwenjie
*                         All rights reserved.
*
* File Name£º   ST7735.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_Lcd_

#include "DriverInclude.h"
#include "interrupt.h"

#if 0
#define LCD_ELW_PIN_RESET 	GPIOPortA_Pin2
#define LCD_ELW_PIN_DC 		GPIOPortC_Pin3//GPIOPortB_Pin0
#define LCD_ELW_PIN_CSB 	GPIOPortB_Pin1
#define LCD_ELW_PIN_SCLK 	GPIOPortB_Pin2
#define LCD_ELW_PIN_SDIN 	GPIOPortB_Pin3
#define LCD_ELW_PIN_MISO 	GPIOPortB_Pin4
#ifdef PEGA_HW_DVT1 //PAGE
#define LCD_ELW_PIN_VCC 	GPIOPortD_Pin2
#else
#define LCD_ELW_PIN_VCC 	GPIOPortC_Pin2
#endif 

#define SPI_CTRL_CONFIG_LCD	(SPI_MASTER_MODE|MOTOROLA_SPI|RXD_SAMPLE_NO_DELAY \
                         	|APB_BYTE_WR|MSB_FBIT|LITTLE_ENDIAN_MODE \
                          	|CS_2_SCLK_OUT_1_CK|CS_KEEP_LOW|SERIAL_CLOCK_POLARITY_LOW \
                        	|SERIAL_CLOCK_PHASE_MIDDLE|DATA_FRAME_8BIT|TRANSMIT_ONLY)

#define LD7032_OFF	0
#define LD7032_ON	1
#define LD7032_STANDBY	2

#define LCD_LD7032_XSIZE	16/////12 * 8 == 96:16 * 8 = 128
#define LCD_LD7032_YSIZE	36

_ATTR_LCDDRIVER_LD7032_CODE_ static UINT8 LD7032_Status = LD7032_OFF;
_ATTR_LCDDRIVER_LD7032_CODE_ static UINT8 LD7032_Update = 0;
_ATTR_LCDDRIVER_LD7032_CODE_ static UINT8 LD7032_Buffer[LCD_LD7032_YSIZE][LCD_LD7032_XSIZE];
_ATTR_LCDDRIVER_LD7032_CODE_ static UINT8 LD7032_Buffer_OledTest[LCD_LD7032_YSIZE][LCD_LD7032_XSIZE];
//PAGE

void LD7032_SetPixel(UINT16 x, UINT16 y, UINT16 data);

//int LD7032_status;
//PAGE
UINT8 OledTestEnabled = 0; // 0: normal mode, 1:test mode
UINT8 OledTestMode = 1; // 0: Full all matrix dot up, 1: Empty all matrix dot down
#if 0
_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_Buffer_printf(int i)
{
	if (i >= LCD_LD7032_YSIZE)
	{
		printf("printf error\n");
	}
	else
	{
		int j;
		for (j=0; j<LCD_LD7032_XSIZE; j++)
		{
			printf("%02X ", LD7032_Buffer[i][j]);
		}
		printf("\n");
	}
}
#endif

_ATTR_LCDDRIVER_LD7032_CODE_
static void LD7032_SPI_Set_Clk(void)
{
    //Configer spi clk to 24M, do not divide frequency
    //Scu->CLKSEL_SPI = ((SPI_CLK_SEL_MASK << 16) | SPI_CLK_SEL_24M);
	//Scu->CLKSEL_SPI =  1<<0 | 0<<1 | 1<<16 | 63<<17;
}


_ATTR_LCDDRIVER_LD7032_CODE_
static void LD7032_SPI_Iomux(void)
{
	/*control gpio init*/
	//LCD reset 
    Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_RESET, IOMUX_GPIOA2_IO);   
    Gpio_SetPinDirection(GPIO_CH0,LCD_ELW_PIN_RESET, GPIO_OUT);
	//GPIO_SetPinPull(LCD_ELW_PIN_RESET, 1);
	Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_RESET,  GPIO_LOW);

	//LCD DC:Data/Command Switch
    Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_DC, IOMUX_GPIOC3_IO);    
    Gpio_SetPinDirection(GPIO_CH0,LCD_ELW_PIN_DC, GPIO_OUT);
	//GPIO_SetPinPull(LCD_ELW_PIN_DC, 1);
	Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_DC,  GPIO_LOW);

    //LCD VCC: 13V switch 
	//PAGE
    #ifdef PEGA_HW_DVT1
	Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_VCC, IOMUX_GPIOD2_IO);
    #else
       Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_VCC, IOMUX_GPIOC2_IO);
    #endif
    Gpio_SetPinDirection(GPIO_CH0,LCD_ELW_PIN_VCC, GPIO_OUT);
	//GPIO_SetPinPull(LCD_ELW_PIN_VCC, 1);
	Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_VCC,  GPIO_LOW);

	/* SPI gpio init*/
	//LCD CSB :Chip Selection == SPI:spi_csn1
    Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_CSB, IOMUX_GPIOB1_SPI_CSN1); 

	//LCD SCLK :Clock == SPI:spi_clk_p1
    Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_SCLK, IOMUX_GPIOB2_SPI_CLKP1);    

	//LCD SDIN: Data  == SPI:spi_txd_p1
    Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_SDIN, IOMUX_GPIOB3_SPI_TXDP1);  

	//LCD SDOUT: Data  == SPI:spi_rxd_p1
    Grf_GpioMuxSet(GPIO_CH0,LCD_ELW_PIN_MISO, IOMUX_GPIOB4_IO);  
    Gpio_SetPinDirection(GPIO_CH0,LCD_ELW_PIN_MISO, GPIO_IN);
	Grf_GPIO_SetPinPull(GPIO_CH0,LCD_ELW_PIN_MISO, 0);
}

_ATTR_LCDDRIVER_LD7032_CODE_
static void LD7032_SPI_Init(uint32 ch)
{
	/* enable spi controller clk */
	LD7032_SPI_Set_Clk();

	/* sclk_out = 24M/6 = 4MHz  "SPI_BAUDR >= 4"*/
    SPICtl->SPI_BAUDR = 6;// 4    
    SPICtl->SPI_CTRLR0 = SPI_CTRL_CONFIG_LCD; // 8bit data frame size, CPOL=1,CPHA=1
	SPICtl->SPI_SER    = ((uint32)(0x01) << ch);
	/* iomux  */
	LD7032_SPI_Iomux();
}

_ATTR_LCDDRIVER_LD7032_CODE_
static void LD7032_SPI_CmdData(uint8 Cmd, uint32 CmdLen, uint8 *pData, uint8 Parm,uint32 size)
{
	UINT32 BaudrVal;

    //SPICtl->SPI_CTRLR0 = SPI_CTRL_CONFIG_LCD;
    SPICtl->SPI_ENR = SPI_ENABLE;
	//SPICtl->SPI_SER = 2;

	/* this delay time is import, dgl*/
	BaudrVal = (SPICtl->SPI_BAUDR << 1) + 5;
	if (CmdLen)
	{
		//CMD send
		Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_DC, GPIO_LOW);
		
        SPICtl->SPI_TXDR[0] = Cmd;
		while ((SPICtl->SPI_SR & TRANSMIT_FIFO_EMPTY) != TRANSMIT_FIFO_EMPTY);
		
		/* this delay is import. */
		DelayUs(BaudrVal);
	}

	if (size)
	{
		//Parameter send
		Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_DC, GPIO_HIGH);
		
	    while (size)
	    {
	        if ((SPICtl->SPI_SR & TRANSMIT_FIFO_FULL) != TRANSMIT_FIFO_FULL)
	        {
				if (pData)
				{
		            SPICtl->SPI_TXDR[0] = *pData++;
				}
				else
				{
		            SPICtl->SPI_TXDR[0] = Parm;
				}
	            size--;
	        }
	    }
	    while ((SPICtl->SPI_SR & TRANSMIT_FIFO_EMPTY) != TRANSMIT_FIFO_EMPTY);
		DelayUs(BaudrVal); 
	}

    //SPICtl->SPI_SER = 0;
    SPICtl->SPI_ENR = SPI_DISABLE;
	
	/* cs# high time > 100ns */
    DelayUs(1);   
    
    return;
}


 /*
--------------------------------------------------------------------------------
  Function name : void ST7735_WriteRAM_Prepare(void)
  Author        : yangwenjie
  Description   : to enable LCD data transfer.
                  
  Input         : null
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_LD7032_CODE_ 
void LD7032_WriteRAM_Prepare(uint8 Level)
{  
    //LD7032_SPI_CmdData(0x12, 1, NULL, Level, 1);
    ScuClockGateCtr(PCLK_SPI0_GATE, 1);
    
    #if 1
    if(Level == 0)
    {
       LD7032_SPI_CmdData(0x12, 1, NULL, 0x4, 1); 
    }
    else if(Level == 1)
    {	//PAGE
        LD7032_SPI_CmdData(0x12, 1, NULL, 0x9, 1);
    }
    else if(Level == 2)
    {	//PAGE
        LD7032_SPI_CmdData(0x12, 1, NULL, 0x14, 1);
    }
    else if(Level == 3)
    {	//PAGE
        LD7032_SPI_CmdData(0x12, 1, NULL, 0x19, 1);
    }
    else if(Level == 4)
    {
        LD7032_SPI_CmdData(0x12, 1, NULL, 0x24, 1);
    }
    else if(Level == 5) //Dark
    {
        LD7032_SPI_CmdData(0x12, 1, NULL, 0x2, 1);
    }
    else if(Level == 6) // BL Off
    {
        LD7032_SPI_CmdData(0x12, 1, NULL, 0x0, 1);
    }
    #endif

    ScuClockGateCtr(PCLK_SPI0_GATE, 0);
}
  /*
--------------------------------------------------------------------------------
  Function name :  void ST7735_Init(void)
  Author        : yangwenjie
  Description   : LCD initialization.
                  
  Input         : null
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_LD7032_CODE_ 
void LD7032_Init(void)
{ 
	uint8 parm[2];

	//IntMasterDisable();
	
	/* SPI clock gate open init*/
	ScuClockGateCtr(PCLK_SPI0_GATE, 1);

	/* spi init*/
	LD7032_SPI_Init(1);
	
	/* reset the lcd*/
	Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_RESET, GPIO_LOW);
	DelayUs(1500);
	Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_RESET, GPIO_HIGH);
	DelayUs(1000);

	/* open LCD moudel power vcc*/
	Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_VCC, GPIO_HIGH);
	DelayMs(100);

	//Gpio_SetPinLevel(LCD_ELW_PIN_CSB,  GPIO_LOW);
	
	/* Release Standby Mode */
	LD7032_SPI_CmdData(0x14, 1, NULL, 0x00, 1);

	/* Set Display OFF */
	LD7032_SPI_CmdData(0x02, 1, NULL, 0x00, 1);

	/* Set Frame Frequency */
	LD7032_SPI_CmdData(0x1A, 1, NULL, 0x05, 1);

	/* Set Data WritingDirection 0x0B:bottom to top 0x00:top to bottom*/
	LD7032_SPI_CmdData(0x1D, 1, NULL, 0x00, 1);

	/* Set Scan Direction */
	LD7032_SPI_CmdData(0x09, 1, NULL, 0x00, 1);

	/* Set Column Driver Active Range */
	parm[0] = 0x00;
	parm[1] = 0x7F;//7F==127 5F==95
	LD7032_SPI_CmdData(0x30, 1, parm, 0, 2);

	/* Set ROW Driver Active Range */
	parm[0] = 0x04;
	parm[1] = 0x27;
	LD7032_SPI_CmdData(0x32, 1, parm, 0, 2);

	/* Set Column Start Line */
	LD7032_SPI_CmdData(0x34, 1, NULL, 0x00, 1);
	
	/* Set Column End Line */
	LD7032_SPI_CmdData(0x35, 1, NULL, 0x0F, 1);//0x0F==128 0x0B == 96
	
	/* Set Row Start Line */
	LD7032_SPI_CmdData(0x36, 1, NULL, 0x04, 1);

	/* Set Row End Line */
	LD7032_SPI_CmdData(0x37, 1, NULL, 0x27, 1);

	/* Set Peak Pulse Width */
	LD7032_SPI_CmdData(0x10, 1, NULL, 0x1F, 1);

	/* Set Peak Pulse Delay Width */
	LD7032_SPI_CmdData(0x16, 1, NULL, 0x05, 1);

	/* Set Pre-charge Width */
	LD7032_SPI_CmdData(0x18, 1, NULL, 0x1F, 1);

	/* Set ROW Scan Sequence */
	LD7032_SPI_CmdData(0x13, 1, NULL, 0x01, 1);

	/* Set Contrast Control */
	LD7032_SPI_CmdData(0x12, 1, NULL, 0x3F, 1);

	/* Set VDD Selection */
	LD7032_SPI_CmdData(0x3D, 1, NULL, 0x01, 1);

	memset(LD7032_Buffer, 0, LCD_LD7032_YSIZE*LCD_LD7032_XSIZE);
	
	LD7032_Status = LD7032_ON;

    ScuClockGateCtr(PCLK_SPI0_GATE, 0);

	//IntMasterEnable();
	return;
}

/*
--------------------------------------------------------------------------------
  Function name :  void ST7735_SendData(UINT16 data)
  Author        : yangwenjie
  Description   : send LCD data.
                  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_LD7032_CODE_ 
void LD7032_SendData(UINT16 data)
{

}
 /*
--------------------------------------------------------------------------------
  Function name : void ST7735_SetCursor(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1)
  Author        : yangwenjie
  Description   : set display area coordinate
                  
  Input         : x0,y0: the start coordinate of display pictrue.
                  x1,y1: the end coordinate of display pictrue.
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_LD7032_CODE_  
void LD7032_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1)
{ 
}

/*
--------------------------------------------------------------------------------
  Function name : void ST7735_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *src)
  Author        : yangwenjie
  Description   : DMA data transfer
                  
  Input         : x0,y0: the start coordinate of display pictrue.
                  x1,y1: the end coordinate of display pictrue.
                  pSrc£º the source address.
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_BufDisp(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1)
{
	LD7032_Update = 1;
}
_ATTR_LCDDRIVER_ST7637_CODE_    
int32 LD7032_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *pSrc, pFunc CallBack)
{
    if (LD7032_Update)
    {
        ScuClockGateCtr(PCLK_SPI0_GATE, 1);
        
        /*0x08 Write start command is issued.*/ /* The image data is issued. */
    	LD7032_SPI_CmdData(0x08, 1, &LD7032_Buffer[0][0], 0, LCD_LD7032_YSIZE * LCD_LD7032_XSIZE);//96 / 8 * 36 == 432 128 / 8 * 36 == 576
    	
    	/* Display ON. */
    	LD7032_SPI_CmdData(0x02, 1, NULL, 0x01, 1);

        LD7032_Update = 0;
        
        ScuClockGateCtr(PCLK_SPI0_GATE, 0);
    }
    
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void LD7032_Standby(UINT16 color)
  Author        : yangwenjie
  Description   : clear the screen
                  
  Input         :   
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_LD7032_CODE_  
void LD7032_Standby(void)
{
	if (LD7032_Status == LD7032_ON)
	{
        ScuClockGateCtr(PCLK_SPI0_GATE, 1);

        //HJ ¹¦ºÄÐèÇó
        LD7032_SPI_CmdData(0x12, 1, NULL, 0x00, 1);
        LD7032_SPI_CmdData(0x02, 1, NULL, 0x00, 1);

		/* Stand-by. */
		LD7032_SPI_CmdData(0x14, 1, NULL, 0x01, 1);

		/* VCC is set in Hi-Z*/
	    Gpio_SetPinDirection(GPIO_CH0,LCD_ELW_PIN_VCC, GPIO_IN);
		Grf_GPIO_SetPinPull(GPIO_CH0,LCD_ELW_PIN_VCC, 0);

		DelayMs(10);

        ScuClockGateCtr(PCLK_SPI0_GATE, 0);

		LD7032_Status = LD7032_STANDBY;
	}
	
	return;
}

_ATTR_LCDDRIVER_LD7032_CODE_  
void LD7032_WakeUp(void)
{
	if (LD7032_Status == LD7032_STANDBY)
	{
        ScuClockGateCtr(PCLK_SPI0_GATE, 1);
        
		/* Power on VCC Then wait until VCC become recommended operation condition*/
	    Gpio_SetPinDirection(GPIO_CH0,LCD_ELW_PIN_VCC, GPIO_OUT);
		Grf_GPIO_SetPinPull(GPIO_CH0,LCD_ELW_PIN_VCC, 1);
		Gpio_SetPinLevel(GPIO_CH0,LCD_ELW_PIN_VCC, GPIO_HIGH);
		DelayMs(10);

		/* Release Stand-by Mode. */
		LD7032_SPI_CmdData(0x14, 1, NULL, 0x00, 1);

        LD7032_SPI_CmdData(0x12, 1, NULL, 0x00, 1); 

		/* Display ON. */
		LD7032_SPI_CmdData(0x02, 1, NULL, 0x01, 1);
		
		LD7032_Status = LD7032_ON;

        ScuClockGateCtr(PCLK_SPI0_GATE, 0);
	}
	else
	{
	    LD7032_Init();
	}
	return;
}

/*
--------------------------------------------------------------------------------
  Function name : void LD7032_SetPixel(UINT16 color)
  Author        : yangwenjie
  Description   : clear the screen
                  
  Input         :   
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_SetPixel(UINT16 x, UINT16 y, UINT16 data)
{
	int RowId, i, BitPos;
	
    if ((x >= (LCD_LD7032_XSIZE << 3)) || (y >= LCD_LD7032_YSIZE))
    {
        return;
	}

	RowId = y;
	i = (x >> 3);
	BitPos = 7 - (x % 8);

	if (data)
	{
		LD7032_Buffer[RowId][i] |= (1 << BitPos);
	}
	else
	{
		LD7032_Buffer[RowId][i] &= ~(1 << BitPos);
	}
	return;
}

/*
--------------------------------------------------------------------------------
  Function name : void ST7735_Clear(UINT16 color)
  Author        : yangwenjie
  Description   : clear the screen
                  
  Input         : color£ºclear lcd to the color.
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1¡ª15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
//int LcdClrType = 0;
_ATTR_LCDDRIVER_LD7032_CODE_  
void LD7032_Clear(UINT16 color)
{
	if (color)
	{
		memset(LD7032_Buffer, 0xFF, LCD_LD7032_YSIZE * LCD_LD7032_XSIZE);
	}
	else
	{
		memset(LD7032_Buffer, 0x00, LCD_LD7032_YSIZE * LCD_LD7032_XSIZE);
	}

    LD7032_BufDisp(0,0,LCD_LD7032_XSIZE-1, LCD_LD7032_YSIZE-1);
	return;
}

_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_ClrSrc(void)
{
	LD7032_Clear(0);
}

_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_ClrRect(int x0, int y0, int x1, int y1)
{
	int i, j, k, pos;
	UINT8 val;

    if ((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0)
		||(x0 > (LCD_LD7032_XSIZE << 3)) || (y0 > LCD_LD7032_YSIZE) 
		|| (x1 > (LCD_LD7032_XSIZE << 3)) || (y1 > LCD_LD7032_YSIZE)
		|| (x0 >= x1) || (y0 >= y1))
    {
        return;
	}
	
	for (i = x0; i < x1; i++)
	{
		k = (i >> 3);
		pos = 7 - (i % 8);
		val = ~(0x01 << pos);
		for (j = y0; j < y1; j++)
		{
			LD7032_Buffer[j][k] &= val;
		}
	}
	
	LD7032_BufDisp(x0,y0,x1,y1);
    
	return;
}

_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_Anti_FillRect(int x0, int y0, int x1, int y1)
{
	int i, j, k, pos;
	UINT8 val;

    if ((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0)
		||(x0 > (LCD_LD7032_XSIZE << 3)) || (y0 > LCD_LD7032_YSIZE) 
		|| (x1 > (LCD_LD7032_XSIZE << 3)) || (y1 > LCD_LD7032_YSIZE)
		|| (x0 >= x1) || (y0 >= y1))
    {
        return;
	}
	
	for (i = x0; i < x1; i++)
	{
		k = (i >> 3);
		pos = 7 - (i % 8);
		val = (0x01 << pos);
		for (j = y0; j < y1; j++)
		{
			LD7032_Buffer[j][k] |= val;
		}
	}
	
	LD7032_BufDisp(x0,y0,x1,y1);
    
	return;
}


_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_MP4_Init(void)
{
}


_ATTR_LCDDRIVER_LD7032_CODE_
void LD7032_MP4_DeInit(void)
{
}

#if 0
{
    LD7032_WriteRAM_Prepare,
    LD7032_Init,
    LD7032_SendData,
    LD7032_SetWindow,
    0,
    LD7032_DMATranfer,
    LD7032_Standby,
    LD7032_WakeUp,
    0,
    0,
    LD7032_SetPixel,
    LD7032_BufDisp,
    LD7032_ClrSrc,
    LD7032_ClrRect,
    LD7032_Anti_FillRect,
}
#endif
#if 0
{
	int i,j;
	char *pbuf;

	pbuf = &LD7032_Buffer[0][0];

	for (i=0; i<LCD_LD7032_YSIZE; i++)
	{
		for (j=0; j<LCD_LD7032_XSIZE; j++)
		{
			printf("%02d.%02d:%08x %08x\n", i, j, (int)pbuf++, (int)&LD7032_Buffer[i][j]);
		}
	}
}
#endif
#if 0
{
	int x, y;
	
	for (y=0; y<LCD_LD7032_YSIZE-4; y++)
	{
		for (x=0; x<(LCD_LD7032_XSIZE<<3); x++)
		{
			LD7032_SetPixel(x, y, 1);
			/*0x08 Write start command is issued.*/ /* The image data is issued. */
			LD7032_SPI_CmdData(0x08, 1, &LD7032_Buffer[0][0], 0, 432);//96 / 8 * 36 == 432 128 / 8 * 36 == 576
			/* Display ON. */
			LD7032_SPI_CmdData(0x02, 1, NULL, 0x01, 1);
			DelayMs(5);
		}
	}
	printf("LCD screen scan finish\n");
	DelayMs(5000);
}

#endif
#endif
/*
********************************************************************************
*
*                         End of Lcd.c
*
********************************************************************************
*/ 
