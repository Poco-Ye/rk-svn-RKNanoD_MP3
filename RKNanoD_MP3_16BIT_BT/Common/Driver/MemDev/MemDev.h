/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: MemDev.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _MEMDEV_H
#define _MEMDEV_H

/*-------------------------------- Includes ----------------------------------*/
#include    <stdio.h>
#include    <string.h>
#include    "SysConfig.h"

/*------------------------------ Global Defines ------------------------------*/
#define MEM_DEV_UNKNOW        0
#define MEM_DEV_NAND          1
#define MEM_DEV_SPINOR        2
#define MEM_DEV_EMMC          3
#define MEM_DEV_SD            4

#define     DISK_FLASH_CODE          0
#define     DISK_FLASH_DATA          1
#define     DISK_FLASH_USER          2
#define     DISK_FLASH_USER2         3
#define     DISK_FLASH_TOTAL         0xff

typedef     unsigned long                   MEMDEV_ID;

#define MD_MAJOR(DevID)           ((uint16)(DevID>>16))
#define MD_MINOR(DevID)           ((uint16)(DevID))
#define MD_MKDEV(major, minor)    ((MEMDEV_ID)((major<<16)|(minor))) 
#define MD_MKLUN(DevID)           ((uint8)(DevID))


#define MEM_DEV_NAND0          MD_MKDEV(MEM_DEV_NAND, 0) 
#define MEM_DEV_NAND1          MD_MKDEV(MEM_DEV_NAND, 1)
#define MEM_DEV_NAND2          MD_MKDEV(MEM_DEV_NAND, 2)
#define MEM_DEV_NAND3          MD_MKDEV(MEM_DEV_NAND, 3)

#define MEM_DEV_SPINOR0        MD_MKDEV(MEM_DEV_SPINOR, 0) 
#define MEM_DEV_SPINOR1        MD_MKDEV(MEM_DEV_SPINOR, 1)
#define MEM_DEV_SPINOR2        MD_MKDEV(MEM_DEV_SPINOR, 2)


#define MEM_DEV_EMMC0          MD_MKDEV(MEM_DEV_EMMC, 0) 
#define MEM_DEV_EMMC1          MD_MKDEV(MEM_DEV_EMMC, 1)
#define MEM_DEV_EMMC2          MD_MKDEV(MEM_DEV_EMMC, 2)
#define MEM_DEV_EMMC3          MD_MKDEV(MEM_DEV_EMMC, 3)


#define MEM_DEV_SD0          MD_MKDEV(MEM_DEV_SD, 0) 
#define MEM_DEV_SD1          MD_MKDEV(MEM_DEV_SD, 1)
#define MEM_DEV_SD2          MD_MKDEV(MEM_DEV_SD, 2)
#define MEM_DEV_SD3          MD_MKDEV(MEM_DEV_SD, 3)


#if (1==FW_IN_DEV)

#define SysDiskID            MEM_DEV_NAND0            
#define DataDiskID           MEM_DEV_NAND1

#define UserDisk0ID          MEM_DEV_NAND2

#ifdef  _MULT_DISK_
#define UserDisk1ID          MEM_DEV_NAND3
#else
#define UserDisk2ID          0
#endif

#ifdef _SDCARD_
#define UserDisk2ID          MEM_DEV_SD0
#else
#define UserDisk2ID          0
#endif

#elif(2==FW_IN_DEV)

#define SysDiskID            MEM_DEV_SPINOR0            
#define DataDiskID           MEM_DEV_SPINOR1

#define UserDisk0ID          MEM_DEV_SD0
#define UserDisk1ID          0
#define UserDisk2ID          0

#elif(3==FW_IN_DEV)

#define SysDiskID            MEM_DEV_EMMC0            
#define DataDiskID           MEM_DEV_EMMC1

#define UserDisk0ID          MEM_DEV_EMMC2

#ifdef  _MULT_DISK_
#define UserDisk1ID          MEM_DEV_EMMC3
#else
#define UserDisk1ID          0
#endif

#ifdef _SDCARD_
#define UserDisk2ID          MEM_DEV_SD0
#else
#define UserDisk2ID          0
#endif


#elif(4==FW_IN_DEV)

#define SysDiskID            MEM_DEV_SD0            
#define DataDiskID           MEM_DEV_SD1

#define UserDisk0ID          MEM_DEV_SD2
#define UserDisk1ID          0
#define UserDisk2ID          0

#else

#endif


#define MD_MAX_UDISK         3

#define MD_MAX_NOTIFY        3

#define                     MD_EVENT_PRESENT    1
#define                     MD_EVENT_CHANGE     2
#define                     MD_EVENT_VAILID     3
//#define                     MD_EVENT_REMOVE     4
//#define                     MD_EVENT_HOTPLUG    5


#define     SYS_PROTECT    
#define     FLASH_PROT_MAGIC        0x444e414e  //NAND

#define     _ATTR_FTL_INIT_CODE_      __attribute__((section("FlashInitCode")))//__attribute__((section("FlashInitCode")))
#define     _ATTR_FTL_INIT_DATA_      __attribute__((section("FlashData")))//__attribute__((section("FlashInitData")))
#define     _ATTR_FTL_INIT_BSS_       __attribute__((section("FlashBss"),zero_init))//__attribute__((section("FlashInitBss"),zero_init))

#define     _ATTR_FTL_CODE_           __attribute__((section("FlashCode")))
#define     _ATTR_FTL_DATA_           __attribute__((section("FlashData")))
#define     _ATTR_FTL_BSS_            __attribute__((section("FlashBss"), zero_init))

#define     CHIP_SIGN           0x4F4E414E                  //NANO
#define     FLASH_MAGIC         0x4E414E44


/*------------------------------ Global Typedefs -----------------------------*/

/*ID BLOCK SECTOR 0 INFO*/
typedef __packed struct tagIDSEC0
{
    uint32  magic;              //0x0ff0aa55, MASKROM�޶����ܸ���
    uint8   reserved[56-4];
    uint32  nandboot2offset;
    uint32  nandboot2len;
    uint32  nandboot1offset1;
    uint32  nandboot1len1;
    uint32  nandboot1offset2;
    uint32  nandboot1len2;
    uint8   CRC[16];
    uint8   reserved1[512 - 96];
} IDSEC0, *pIDSEC0;

#if 0
typedef __packed struct tagRKNANO_IDB_SEC3
{
    uint16  usSNSize;
    uint8   sn[30]; // 2-31
    uint8   reserved[419]; // 32-450
    uint8   uidSize;
    uint8   uid[15];
    uint8   blueToothSize;
    uint8   blueToothAddr[30];
    uint8   macSize;
    uint8   macAddr[6];
} RKNANO_IDB_SEC3, *PRKNANO_IDB_SEC3;

#else

#define RKDEVICE_SN_LEN 60
#define RKDEVICE_UID_LEN 30
#define RKDEVICE_MAC_LEN 6
#define RKDEVICE_WIFI_LEN 6
#define RKDEVICE_BT_LEN 6
#define RKNANO_SEC3_RESERVED_LEN 405

typedef __packed struct tagRKNANO_IDB_SEC3
{
    uint16  usSNSize;
    uint8   sn[RKDEVICE_SN_LEN];
    uint8   reserved[RKNANO_SEC3_RESERVED_LEN];
    uint8   uidSize;
    uint8   uid[RKDEVICE_UID_LEN];
    uint8   blueToothSize;
    uint8   blueToothAddr[RKDEVICE_BT_LEN];
    uint8   macSize;
    uint8   macAddr[RKDEVICE_MAC_LEN];
} RKNANO_IDB_SEC3, *PRKNANO_IDB_SEC3;
#endif


#if 0
/*ID BLOCK SECTOR 1 INFO*/
typedef __packed struct tagIDSEC1
{
    uint16  sysAreaBlockRaw;        //ϵͳ������, ��ԭʼBlockΪ��λ
    uint16  sysProgDiskCapacity;    //ϵͳ�̼�������, ��MBΪ��λ
    uint16  sysDataDiskCapacity;    //ϵͳ����������, ��MBΪ��λ
    uint16  reserved0[2];           //����������0
    uint16  chipSignL;              // 28
    uint16  chipSignH;              // RK
    uint16  MachineUIDL;            //����UID,����ʱƥ��̼���
    uint16  MachineUIDH;
    uint16  year;                   //�м������������
    uint16  data;                   //�м��������������
    uint16  mainVer;                //���汾��
    uint16  FirmMasterVer;
    uint16  FirmSlaveVer;
    uint16  FirmSmallVer;
    uint16  LasterLoader;
    uint16  UpgradeTimes;
    uint8   reserved1[480-30];      //����������0
    uint32  ChipSize;
    uint16  blockSize;              //��SEC��ʾ��BLOCK SIZE
    uint8   pageSize;               //��SEC��ʾ��PAGE SIZE
    uint8   eccBits;                //��Ҫ��ECC BIT��, eg. 8/14
    uint8   accessTime;             //��дcycleʱ��, ns
    uint8   reserved2[498-493];      //����������0
    uint16  bStartRB;
    uint16  bEndRB;
    uint16  idBlk[5];                //ID��λ��
}IDSEC1, *pIDSEC1;
#endif

/*ID BLOCK SECTOR 1 INFO*/
typedef __packed struct tagIDSEC1
{
    uint16  SysAreaBlockRaw;        //ϵͳ������, ��ԭʼBlockΪ��λ
    uint16  SysProgDiskCapacity;    //ϵͳ�̼�������, ��MBΪ��λ			
	uint16  SysDataDiskCapacity;			
	uint16  Disk2Size;				
	uint16  Disk3Size;				
	uint32	ChipTag;
	uint32	MachineId;
    
	uint16	LoaderYear;
	uint16	LoaderDate;
	uint16	LoaderVer;
	uint16	FirmMasterVer;		// (24-25)	BCD���� �̼����汾
    uint16	FirmSlaveVer;			// (26-27)	BCD���� �̼��Ӱ汾
    uint16  FirmSmallVer;			// (28-29)	BCD���� �̼��Ӱ汾  
	uint16  LastLoaderVer;
	uint16  UpgradeTimes;
	uint16  MachineInfoLen;
	uint8	MachineInfo[30];
	uint16	ManufactoryInfoLen;
	uint8	ManufactoryInfo[30];
	uint16	FlashInfoOffset;
	uint16	FlashInfoLen;
    uint8	Reserved2[382];				// (102-483)����
	uint32	FlashSize;				    //��sectorΪ��λ
	uint16  BlockSize;                  //��SEC��ʾ��BLOCK SIZE
	uint8   PageSize;                  //��SEC��ʾ��PAGE SIZE
	uint8   ECCBits;
	uint8   AccessTime;                //��дcycleʱ��, ns
    uint8   reserved2[5];
	uint16  FirstReservedBlock;		// (498-499)Idblock֮���һ��������ĵ�λ��		(��λ:block)
    uint16  LastReservedBlock;		// (500-501)Idblock֮�����һ��������ĵ�λ��	(��λ:block)
	uint16  IdBlock0;
	uint16  IdBlock1;
	uint16  IdBlock2;
	uint16  IdBlock3;
	uint16  IdBlock4;
}IDSEC1, *pIDSEC1;


typedef __packed struct tagMEMDEV_INFO
{
    uint32  capacity;           //(SectorΪ��λ)  4Byte
    uint16  BlockSize;          //(SectorΪ��λ)  2Byte
    uint16  PageSize;           //(SectorΪ��λ)  1Byte
} MEMDEV_INFO, *pMEMDEV_INFO;


typedef void (*MEN_NOTIFIER)(uint16 major, uint32 event, uint32 param);

/*----------------------------- External Variables ---------------------------*/

extern  uint32 	SysProgDiskCapacity;			//ϵͳ����������
extern  uint32 	SysProgRawDiskCapacity;			//ϵͳ����������
extern  uint32 	SysDataDiskCapacity;			//ϵͳ����������
extern  uint32 	SysUserDisk2Capacity;			//ϵͳ����������

extern  uint16  UpgradeTimes;
extern  uint16  LoaderVer;
extern  uint8   ValidSysDisk;

/*------------------------- Global Function Prototypes -----------------------*/

extern uint32 MemDevInit(void);    
extern uint32 MDDeInitAll(void);

extern uint32 MDProbe(MEMDEV_ID DevID);

extern uint32 MDRead(MEMDEV_ID DevID, uint32 SecAdrr, uint32 nSec, void* pBuf);
extern uint32 MDWrite(MEMDEV_ID DevID, uint32 SecAdrr, uint32 nSec, void* pBuf);

extern uint32 MDGetInfo(MEMDEV_ID DevID, pMEMDEV_INFO pDevInfo);
extern uint32 MDGetCapacity(MEMDEV_ID DevID);

extern  bool MDChkValid(MEMDEV_ID DevID);
extern  bool MDChkPresent(MEMDEV_ID DevID);

extern  uint32 MDGetUDiskTab(MEMDEV_ID *pUDiskID, uint32 mask);
extern  uint32 MDScanDev(uint8 AutoInit);

extern  uint32 MDRegNotify(MEN_NOTIFIER notify);
extern  uint32 MDUnRegNotify(MEN_NOTIFIER notify);

extern  uint32 MDSyncIO(void);

extern  uint32 MDSysWPSet(MEMDEV_ID DevID, uint32 magic);
extern  int32 MDGetProductSn(MEMDEV_ID DevID, void *pSn);

#endif
