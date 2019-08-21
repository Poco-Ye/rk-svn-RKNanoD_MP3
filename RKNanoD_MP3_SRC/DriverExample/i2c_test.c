/*
********************************************************************************
*                   Copyright (c) 2015
*                         All rights reserved.
*
* File Name£º   I2C_Test.c
*
* Description:
*
* History:      <author>          <time>        <version>
*
*    desc:    ORG.
********************************************************************************
*/
#include "sysinclude.h"

#ifdef _I2C_TEST_

#include "Device.h"
#include "DriverInclude.h"



//******************************************************************************
//i2c function test
#define  RTC_HYM8563_IICAddr   0xa2
#define  RTC_HYM8563_IICSpeed  50

#define   RTC_CTL1      0x00
#define   RTC_CTL2      0x01
#define   RTC_SEC       0x02
#define   RTC_MIN       0x03
#define   RTC_HOUR      0x04
#define   RTC_DAY       0x05
#define   RTC_WEEK      0x06
#define   RTC_MON       0x07
#define   RTC_YEAR      0x08
#define   RTC_A_MIN     0x09
#define   RTC_A_HOUR    0x0A
#define   RTC_A_DAY     0x0B
#define   RTC_A_WEEK    0x0C
#define   RTC_CLKOUT    0x0D
#define   RTC_T_CTL     0x0E
#define   RTC_T_COUNT   0x0F
#define   CENTURY   0x80
#define   TI        0x10
#define   AF        0x08
#define   TF        0x04
#define   AIE       0x02
#define   TIE       0x01
#define   FE        0x80
#define   TE        0x80
#define   FD1       0x02
#define   FD0       0x01
#define   TD1       0x02
#define   TD0       0x01
#define   VL        0x80

#define  Codec_IICAdress   (0x1a<<1)
#define  Codec_IICSpeed    200//100

#define  ALC5633_Codec_IICAdress   (0x1C<<1)
#define  ALC5633_Codec_IICSpeed    400



void I2CTestHelpInfo()
{
    printf("\n I2C test start!\n");
    printf("================================================================================\n");
    printf(" I2C Test Menu                                                                  \n");
    printf(" 1. I2C_CHN0 Test Start [0] -- port A,[1] -- port B,[2] -- port C               \n");
    printf(" 2. I2C_CHN1 Test Start [0] -- port A,[1] -- port B,[2] -- port C               \n");
    printf(" 3. I2C_CHN2 Test Start [0] -- port A,[1] -- port B,[2] -- port C               \n");

    printf("\r\n");
    printf(" 0. exit                                                                        \n");
    printf(" h. show i2c test menu                                                          \n");
    printf("================================================================================\n");
    printf("\r\n");
}

static void I2CPortNormalTest(eI2C_IOMUX i2cPort)
{
    int ret;
    uint8 data[2];
    uint32 reg = 0x44;
    uint32 i2cChn;

#define  IICSpeed     50

    i2cChn = i2cPort / 3;
    I2C_Init(i2cPort,WM8987codec, IICSpeed);

    data[0] = 0x55;
    data[1] = 0xaa;
    ret = I2C_Write(i2cChn,reg,data,2, NormalMode);

    while(ret <= 0 ) //ack not recived,try again.
    {
        ret = I2C_Write(i2cChn,reg,data,2, NormalMode);
        if(ret == TIMEOUT) printf("i2c write time out ! \n");
        if(ret == ERROR)   printf("i2c write error happen (noACK)! \n" );
        if(ret == 0)       printf("i2c write not success! \n" );
    }

#undef   IICSpeed
}

static void I2CPortDirectModeTest(eI2C_IOMUX i2cPort,uint8* data,uint32 datasize)
{
    int ret;
    uint8 *pdata;
    uint32 reg = 0x0;   //direct mode no regester address.
    uint32 i2cChn;

#define  IICSpeed     50

    i2cChn = i2cPort / 3;
    I2C_Init(i2cPort,WM8987codec, IICSpeed);

    pdata = data;

    ret = I2C_Write(i2cChn,reg,pdata,datasize, DirectMode);

    while(ret <= 0 ) //ack not recived,try again.
    {
        ret = I2C_Write(i2cChn,reg,pdata,datasize, DirectMode);
        if(ret == TIMEOUT) printf("i2c write time out ! \n");
        if(ret == ERROR)   printf("i2c write error happen (noACK)! \n" );
        if(ret == 0)       printf("i2c write not success! \n" );
    }

#undef   IICSpeed
}

static void RTC_hym8563Test()
{
    uint8 cmd,second,minute,hour;
    uint32 ret;
    uint8 readBuf[3];
    uint8 time[3];
    uint32 cnt = 10;

    second = 59;
    minute = 59;
    hour   = 23;

    time[0] = ((second / 10) << 4) + (second % 10);
    time[1] = ((minute / 10) << 4) + (minute % 10);
    time[2] = ((hour   / 10) << 4) + (hour % 10);

    I2C_Init(I2C_CH0_PA,RTC_HYM8563_IICAddr, RTC_HYM8563_IICSpeed);

    ret = I2C_Write(I2C_CH0,RTC_SEC,time,3, NormalMode);

    while (ret <= 0) //ack not recived,try again.
    {
        DelayMs(5);
        ret = I2C_Write(I2C_CH0,RTC_SEC,time,3, NormalMode);

        printf("send data %d \n",cmd);
    }

    while (cnt--)
    {
        int i = 0;
        printf(" Set time 23:59:59 \n");
        I2C_Read(I2C_CH0, RTC_SEC, readBuf,3,NormalMode);

        printf(" second = %d \n",readBuf[0]);
        printf(" minute = %d \n",readBuf[1]);
        printf(" hour   = %d \n",readBuf[2]);
    }
}

void I2CChannelTest(eI2C_CHANNEL i2cChn,uint8* pstr)
{
    int32 ret = OK;
    uint8  Char[16];
    uint8 *pChar = Char;
    int index = 0;

    uint32 i2cPort;

    DelayMs(10);

    i2cPort = i2cChn * 3;
    if (0 == StrCmpA(pstr, "0", 1))         //port A
    {
        if(i2cChn == 0)
        {
            RTC_hym8563Test();
        }
        else if(i2cChn == 1)
        {
            Codec5633_PowerOnInitial(0);
        }
        else
        {
            I2CPortNormalTest(i2cPort);
        }
    }
    else if (0 == StrCmpA(pstr, "1", 1))    //port B
    {
        I2CPortNormalTest(i2cPort + 1);
    }
    else if (0 == StrCmpA(pstr, "2", 1) )   //port C
    {
        I2CPortNormalTest(i2cPort + 2);
    }

    if (ret != OK)
        return;

    printf("\n");
}

rk_err_t I2CTestCmdParse(HDC dev, uint8 * pstr)
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
    cmd = pstr[0];

    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    switch (cmd)
    {
        case '0': //folder_once
            return RK_EXIT;

        case 'h':
            I2CTestHelpInfo();
            break;

        case '1': //I2C CHN 0
        {
            I2CChannelTest(I2C_CH0,pItem);
        }
            break;

        case '2': //I2C CHN 1
        {
            I2CChannelTest(I2C_CH1,pItem);
        }
            break;

        case '3': //I2C CHN 2
        {
            I2CChannelTest(I2C_CH2,pItem);
        }
        case 'I':
            break;

        default:
            ret = RK_ERROR;
            break;
    }

    DelayMs(10);
    I2CTestHelpInfo();
    return ret;
}


//******************************************************************************
#endif

