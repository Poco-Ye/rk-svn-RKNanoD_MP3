/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   Lcd.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_Lcd_
#define LCD_XL_UC1604

#include "DriverInclude.h"
#include "LcdInclude.h"
#define  SH1108G_column_offset  128
#define  SH1108G_LCD_BUF_XSIZE  128
#define  SH1108G_LCD_BUF_YSIZE  20
#define OLED_PAGES (LCD_HEIGHT / 8)

#define _GUI_SH1108G_READ_  __attribute__((section("gui_SH1108G_read")))
#define _GUI_SH1108G_WRITE_ __attribute__((section("gui_SH1108G_write")))
#define _GUI_SH1108G_INIT_  __attribute__((section("gui_SH1108G_init")))

_ATTR_LCDDRIVER_SH1108G_CODE_ UINT8  SH1108G_Buffer[SH1108G_LCD_BUF_XSIZE][SH1108G_LCD_BUF_YSIZE];

 _ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_Command(UINT16 cmd)
 {
	/*
	f_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_LCD_RS);  
	Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin1,GPIO_OUT); 
	Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin1,GPIO_LOW);	//A0 RS

	Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_LCD_WRN);  
	Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin0,GPIO_OUT); 
	Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin0,GPIO_LOW);	//WR
	*/
     	Lcd->cmd = cmd;

	/*
 	Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_LCD_WRN);  
	Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin0,GPIO_OUT); 
	Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin0,GPIO_HIGH); //WR
	*/	
 }


 _ATTR_LCDDRIVER_SH1108G_CODE_ 
 void SH1108G_Data(UINT16 data)
 {
 	/*
 	Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_LCD_RS);  
	Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin1,GPIO_OUT); 
	Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin1,GPIO_HIGH);	//A0 RS

	Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_LCD_WRN);  
	Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin0,GPIO_OUT); 
	Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin0,GPIO_LOW);	//WR
	*/
    	Lcd->data = data;
	/*
	Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_LCD_WRN);  
	Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin0,GPIO_OUT); 
	Gpio_SetPinLevel(GPIO_CH0,GPIOPortD_Pin0,GPIO_HIGH); //WR
	*/
	 	
 }

static void SH1108G_all_screen(void)
{
    int i,j;
    for(i=0;i<20;i++)
    {
        VopSendCmd(0, 0xb0);
        VopSendCmd(0, 0x00+i);

        VopSendCmd(0, 0x00);
        VopSendCmd(0, 0x11);
        for(j=0;j<128;j++)
        {
            VopSendData(0,0xff);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_WriteReg(UINT16 addr, UINT16 data)
  Author        : yangwenjie
  Description   :向LCD传输数据和命令
                  
  Input         : data LCD数据
                  addr LCD命令寄存器地址
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_ 
 void SH1108G_WriteReg(UINT16 addr, UINT16 data)
 {
     SH1108G_Command(addr);
     SH1108G_Data(data);
 }

/*
--------------------------------------------------------------------------------
  Function name : void EMCTL_Config(void)
  Author        : yangwenjie
  Description   : 配置LCD的刷屏时间
                  
  Input         : 无
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_ 
void SH1108G_WaitSet(UINT32 data )
 {
    //emctl->LCDWait = data;
 }
 
 /*
--------------------------------------------------------------------------------
  Function name : void Lcd_WriteRAM_Prepare(void)
  Author        : yangwenjie
  Description   :LCD数据传输使能
                  
  Input         : 无
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_ 
  void SH1108G_WriteRAM_Prepare(void)
 {   
    VopSendCmd(0,0xaf);
 }

  /*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Init(void)
  Author        : yangwenjie
  Description   : 初始化LCD
                  
  Input         : 无
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_
 void SH1108G_Init(void)
{ 
    DEBUG("SH1108G_Init test start.");

	Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_IO);  
	Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4,GPIO_OUT); 
	Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin4,GPIO_HIGH);
	
	//IoMuxSetLCD(LCD_MODE);
	//Lcd_WaitSet(0x000011d1);
	VopSetSplit(0, VOP_CON_SPLIT_ONE);
	
    VopSetMode(0,VOP_CON_SPLIT_ONE | VOP_CON_AUTO_CLK_DISABLE | VOP_CON_BYPASS_FIFO | VOP_CON_DATA_WIDTH_8);
	  DelayMs(120);
    //VopSetMode(0,value);
    
    VopSetTiming(0,12,5,5);
    VopSetWidthHeight(0,128,160);
#if 1
	//VopDebugReg(0);
	VopSendCmd(0, 0xae);    //Display OFF
	//VopDebugReg(0);         //xfli debug.
	
	VopSendCmd(0, 0x81);    // Set contrast control
	VopSendCmd(0, 0x90);
	VopSendCmd(0, 0xa0);    // Segment remap
	//VopSendCmd(0xa4);    // Set Entire Display ON
	VopSendCmd(0, 0xa6);    // Normal display
	VopSendCmd(0, 0xa9);    //Set Display Resolution
	VopSendCmd(0, 0x02);     //128*160
	VopSendCmd(0, 0xad);    // Set external VPP
	VopSendCmd(0, 0x80);
	//VopSendCmd(0, 0xaf);
	VopSendCmd(0, 0xc0);    // Set Common scan direction
	// Scan from COM0 to COM[N -1].
	VopSendCmd(0, 0xd5);    // Divide Ratio/Oscillator Frequency Mode Set
	VopSendCmd(0, 0xf1);    // 100Hz  //divide is 2
	VopSendCmd(0, 0xd9);    // Set Dis-charge/Pre-charge Period
	VopSendCmd(0, 0x13);
	VopSendCmd(0, 0xdb);    // Set Vcomh voltage
	VopSendCmd(0, 0x2b);    // 0.687*VPP
	VopSendCmd(0, 0xdc);     //Set VSEGM Deselect Level
	VopSendCmd(0, 0x35);
	VopSendCmd(0, 0x30);  //Set Discharge VSL Level,0V
	
	VopSendCmd(0, 0x20);    // Set Memory addressing mode
	
	//SH1108G_Clear_Screen(); //Clear internal RAM to "00H"
	VopSendCmd(0, 0xaf);    //Display ON
	DelayMs(100);
    DEBUG("SH1108G_Init test SH1108G_photo1.");

#else
	SH1108G_Command(0xae);
	DelayMs(10);
	SH1108G_Command(0x81);
	SH1108G_Command(0x90);
	SH1108G_Command(0xa0);
	SH1108G_Command(0xa6);
	SH1108G_Command(0xa9);
	SH1108G_Command(0x02);
	SH1108G_Command(0xad);
	SH1108G_Command(0x80);
	SH1108G_Command(0xc0);
	SH1108G_Command(0xd5);
	SH1108G_Command(0xf1);
	SH1108G_Command(0xd9);
	SH1108G_Command(0x13);
	SH1108G_Command(0xdb);
	SH1108G_Command(0x2b);

	SH1108G_Command(0xdc);
	SH1108G_Command(0x35);
	SH1108G_Command(0x30);
	SH1108G_Command(0x20);
	//SH1108G_Command(0x21);
	SH1108G_Command(0xaf); 

#endif
	//VopSetSplit(0, VOP_CON_SPLIT_TWO);
	
	DelayMs(100);

	//SH1108G_all_screen();
	//DelayMs(2000);
	//SH1108G_photo2();
	
}

/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_SendData(UINT16 data)
  Author        : yangwenjie
  Description   : 发送LCD数据
                  
  Input         : x0,y0: 显示图形的起始坐标
                  x1,y1: 显示图形的终点坐标
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_ 
void SH1108G_SendData(UINT16 data)
{
    VopSendData(0,data);   
}

 /*
--------------------------------------------------------------------------------
  Function name : void Lcd_SetCursor(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1)
  Author        : yangwenjie
  Description   : 设置显示坐标
                  
  Input         : x0,y0: 显示图形的起始坐标
                  x1,y1: 显示图形的终点坐标
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_  
 void SH1108G_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1)
 {
    DEBUG("SH1108G_SetWindow.");

	//wWidth=wHeight;//无用
       //x0+=19; // modify by Kitty 2010-03-23#1
	//Lcd_Command(0xb7-(y0/8));//设置页地址Page addresss，1个page 8个点高
	//Lcd_Command(  ( (x0>>4)&0x0F )  | 0x10);
	//Lcd_Command(x0&0x0F);
 }
 
 _ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_SetPixel(UINT16 x, UINT16 y, UINT16 data)
{
    if(x>=SH1108G_LCD_BUF_XSIZE||y>=SH1108G_LCD_BUF_YSIZE*8)
        return;
    if(data)
        SH1108G_Buffer[x][(unsigned int )y/8]= SH1108G_Buffer[x][(unsigned int )y/8]|(0x01<<(y%8));
    else
        SH1108G_Buffer[x][(unsigned int )y/8]= SH1108G_Buffer[x][(unsigned int )y/8]&(~(0x01<<(y%8)));
}

_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1)
#if 1
{//show line one by one
    UINT16  x,y,x0_temp,y0_temp,x1_temp,y1_temp,page_temp;
	x0_temp=x0;
	x1_temp=x1;
	page_temp=y0/8;
	y0_temp=page_temp*8;

    if(y1>=SH1108G_LCD_BUF_YSIZE*8)
	    y1=SH1108G_LCD_BUF_YSIZE*8-1;
	
    for (y=y0/8;y<=y1/8;y++)
	{
		//VopSendCmd(0,0x10);
		//VopSendCmd(0,0x02);
		VopSendCmd(0,0xb0);
		VopSendCmd(0,0x00+y);
		VopSendCmd(0,0x00);
		VopSendCmd(0,0x11);
		
		//VopSendCmd(0,(x0 & 0x0f));    //Lower
		//VopSendCmd(0,(x0 & 0xf0)>>4 | 0x10);    //Higher
		
	    for (x=0;x<SH1108G_LCD_BUF_XSIZE;x++)
		{
		    VopSendData(0, SH1108G_Buffer[x][y]);
		    //VopSendData(0,SH1108G_Buffer[SH1108G_LCD_BUF_XSIZE - x - 1][y]);
		}
	    page_temp++;
	
	}
}

#else
{
   // DEBUG("SH1108G_Image..");

    int i,j;
    int start_page;
    int pages_cnt;

    if (x1>LCD_WIDTH-1) x1 = LCD_WIDTH-1;
    if (y1>LCD_HEIGHT-1) y1 = LCD_HEIGHT-1;

    if(y0 < 8) {
        start_page = 0;
    } else {
        start_page = y0 / 8;
    }
    pages_cnt = (y1 - y0) / 8;

    for (i=start_page; i<=pages_cnt; i++)
    {
		VopSendCmd(0,0xb0);    //Page Address Command Set
		VopSendCmd(0,0x00+i);
		VopSendCmd(0,(x0 & 0x0f));    //Lower
		VopSendCmd(0,(x0 & 0xf0)>>4 | 0x10);    //Higher

        for (j = x0; j <= x1; j)      //send 2 tiems. 16bit.
        {
            VopSendData(0,SH1108G_Buffer[SH1108G_LCD_BUF_XSIZE - j - 1][i]);
        }
    }
}

#endif

_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_Buffer_Display(void)
#if 1
{
    unsigned int  x,y,page_;
 	page_=0xB0;
    for (y=0;y<SH1108G_LCD_BUF_YSIZE;y++)
	{
		VopSendCmd(0,0xb0);
		VopSendCmd(0,0x00+y);
		VopSendCmd(0,0x00);
		VopSendCmd(0,0x11);
		
		for (x=0;x<SH1108G_LCD_BUF_XSIZE;x++)
			{
			    VopSendData(0,SH1108G_Buffer[x][y]);
			}
		page_++;
	}
}
#else
{
    int i,j;
    int start_page;
    int pages_cnt;

	int x0,y0,x1,y1;
	x0=0;
	y0=0;
	x1=LCD_WIDTH;
	y1=LCD_HEIGHT;
	
    if (x1>LCD_WIDTH-1) x1 = LCD_WIDTH-1;
    if (y1>LCD_HEIGHT-1) y1 = LCD_HEIGHT-1;

    if(y0 < 8) {
        start_page = 0;
    } else {
        start_page = y0 / 8;
    }
    pages_cnt = (y1 - y0) / 8;

    for (i=start_page; i<=pages_cnt; i++)
    {
		VopSendCmd(0,0xb0);    //Page Address Command Set
		VopSendCmd(0,0x00+i);
		VopSendCmd(0,(x0 & 0x0f));    //Lower
		VopSendCmd(0,(x0 & 0xf0)>>4 | 0x10);    //Higher

        for (j = x0; j <= x1; j+2)      //send 2 tiems. 16bit.
        {
            VopSendData(0,SH1108G_Buffer[SH1108G_LCD_BUF_XSIZE - j - 1][i]);
            //SH1108G_Data(SH1108G_Buffer[SH1108G_LCD_BUF_XSIZE - x - 1][y]);
        }
    }
}
#endif

_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_Buffer_Clr(void)
{
    UINT16 x,y;
    for (y=0;y<SH1108G_LCD_BUF_YSIZE;y++)
	    for (x=0;x<SH1108G_LCD_BUF_XSIZE;x++)
		{
		    SH1108G_Buffer[x][y]=0x00;
		}
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
--------------------------------------------------------------------------------
  Function name : void Lcd_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *src)
  Author        : yangwenjie
  Description   : DMA传输数据
                  
  Input         : x0,y0: 显示图形的起始坐标
                  x1,y1: 显示图形的终点坐标
                  pSrc：  显示图形的源地址
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_    
int32 SH1108G_DMATranfer (UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1,UINT8 *pSrc)
{    
    UINT16  xsize, x, y;

    if(y1>=SH1108G_LCD_BUF_YSIZE*8)
	    y1=SH1108G_LCD_BUF_YSIZE*8-1;
	    
    if(x1>=SH1108G_LCD_BUF_XSIZE)
	    x1=SH1108G_LCD_BUF_XSIZE-1;
	
    xsize = x1 + 1;
    
    for (y=y0/8; y<=y1/8; y++)
	{
		VopSendCmd(0,0xb0);
		VopSendCmd(0,0x00+y);
		VopSendCmd(0,0x00);
		VopSendCmd(0,0x11);
		
	    for (x = y*xsize; x < ((y+1) * xsize); x++)
		{
		    VopSendData(0, pSrc[x]);
		}
	}

 }

//+++++++++++++++++++++++++++++++++++++++++++++fjp
_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_ClrSrc()
{
    SH1108G_Buffer_Clr();
    SH1108G_Buffer_Display();
//LCD_Buffer_Display();
}
//++++++++++++++++++++++++++++++++++++++++++++++++

/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Data(UINT16 data)
  Author        : yangwenjie
  Description   :让屏进入省电模式
                  
  Input         : data LCD数据
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_  
void SH1108G_Standby(void)
{
        DEBUG("SH1108G_Standby.");

	VopSendCmd(0,0xAE);
	DelayMs(100);
}

_ATTR_LCDDRIVER_SH1108G_CODE_  
void SH1108G_PowerOff(void)
{
	//SH1108G_Command(0xAE);
	//SH1108G_Command(0x28);
	//SH1108G_Command(0xA5);	
//    Gpio_SetPinDirection(POWER_ON , GPIO_OUT);      //POWER_ON
 //   Gpio_SetPinLevel(POWER_ON,  GPIO_LOW);	
//	    DelayMs(5);
}

/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Data(UINT16 data)
  Author        : yangwenjie
  Description   :让屏进入省电模式
                  
  Input         : data LCD数据
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_SH1108G_CODE_  
void SH1108G_WakeUp(void)
{
    SH1108G_Init();
	        DEBUG("SH1108G_WakeUp.");

}

//sen #20100701#2 s
_ATTR_LCDDRIVER_SH1108G_CODE_  
void SH1108GSetContrast(uint8 Level)
{
 
 }
 
_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_ClrRect(int x0, int y0, int x1, int y1)
{
	int i,j;
	for(j=y0; j<=y1; j++)
        for (i=x0;i<=x1;i++)
			LCD_SetPixel(i,j,0);

    SH1108G_Buffer_Display1(x0,y0,x1,y1);

}

_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_DEV_FillRect(int x0, int y0, int x1, int y1)
{
	int i,j;
	for (i=x0;i<=x1;i++)
		for(j=y0; j<=y1; j++)
			LCD_SetPixel(i,j,1);
	   SH1108G_Buffer_Display1(x0,y0,x1,y1);

}

_GUI_SH1108G_WRITE_
void SH1108G_Clear(uint16 color)
{
    int i,j;

    //SH1108G_SetWindow(0,0,LCD_WIDTH -1, LCD_HEIGHT -1);
    for(i=0; i<OLED_PAGES; i++)
    {
		VopSendCmd(0, 0xb0);    //Page Address Command Set
		VopSendCmd(0, 0x00+i);
        //Set Column Address:
        //When display resolution is 128*160(w*h),
        //relative columm is Column16 to Column143.
		VopSendCmd(0, 0x00);    //Lower
		VopSendCmd(0, 0x11);    //Higher

        for(j=0; j<LCD_WIDTH; j++)
        {
            VopSendData(0, color);
        }
    }
}

_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_MP4_Init(void)
{
    //VopSetSplit(0, LCD_SPLIT1);
    //VopSendCmd(0, 0x36); //Set Scanning Direction
    //VopSendData(0, 0x68);
    //VopSetSplit(0, LCD_SPLIT2);
}

_ATTR_LCDDRIVER_SH1108G_CODE_
void SH1108G_MP4_DeInit(void)
{
    //VopSetSplit(0, LCD_SPLIT1);
    //VopSendCmd(0, 0x36);
    //VopSendData(0, 0xc8);
    //VopSetSplit(0, LCD_SPLIT2);
}
