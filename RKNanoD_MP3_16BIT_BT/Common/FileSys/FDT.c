/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   FDT.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/
#define   IN_FDT
#include "FsInclude.h"

/*********************************************************************************************************
** Name	:ReadFDTInfo
** Description	:��ȡFDT��Ϣ
** Input	:Rt���洢������Ϣ��ָ��
**        			 SecIndex��������
**         			 ByteIndex��ƫ����
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	: null
** call module	:NandFlashReadSector
********************************************************************************************************/
IRAM_FAT
uint8 ReadFDTInfo(FDT *Rt, uint32 SecIndex, uint16 ByteIndex)
{
	uint8 i;
	uint8 *pRt=(uint8 *)Rt;
	uint8 *Buf;
	uint8  status;
    
    UserIsrDisable();
    
	status = RETURN_OK;
	if (SecIndex != FdtCacheSec)
	{
		FdtCacheSec = SecIndex;
		if (OK != FATReadSector(SecIndex, FdtBuf))
		{
			status = NOT_FIND_FDT;
			goto exit;
		}
	}

	Buf = FdtBuf + ByteIndex;
    memcpy(pRt, Buf, sizeof(FDT));

    if(Rt->Attr != ATTR_LFN_ENTRY)
    {
        if(Rt->Name[0] == 0xFF && Rt->Name[1] == 0xFF && Rt->Name[2] == 0xFF && Rt->Name[3] == 0xFF)
        {
            Rt->Name[0] = 0;
        }
    }

exit:
    UserIsrEnable();   
	return (status);
    
}

#ifdef ENCODE
/*********************************************************************************************************
** Name	:WriteFDTInfo
** Description	:дFDT��Ϣ
** Input	:SecIndex��������
**        			 ByteIndex��ƫ����
**        			 FDT *FDTData:����
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	: null
** call module	:NandFlashReadSector,NandFlashWriteSector
********************************************************************************************************/
IRAM_ENCODE
uint8 WriteFDTInfo(FDT *FDTData, uint32 SecIndex, uint16 ByteIndex)
{
	uint8 i;
	uint8 *pRt=(uint8 *)FDTData;
	uint8 *Buf;

    if(FdtCacheSec != SecIndex)
    {
        FATReadSector(SecIndex, FdtBuf);    
	    FdtCacheSec = SecIndex;
    }
    
	Buf = FdtBuf + ByteIndex;
    memcpy(Buf, pRt, sizeof(FDT));

	if (OK != FATWriteSector(SecIndex, FdtBuf))
	{
		return (NOT_FIND_FDT);
	}
	else		
	{
		return (RETURN_OK);
	}
}
#endif

/*********************************************************************************************************
** Name	:GetRootFDTInfo
** Description	:��ȡ��Ŀ¼ָ���ļ�(Ŀ¼)��Ϣ
** Input	:Rt���洢������Ϣ��ָ��
**        			 Index���ļ�(Ŀ¼)��FDT�е�λ��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	:FATType, BootSector
** call module	:ReadFDTInfo
********************************************************************************************************/
IRAM_FAT
uint8 GetRootFDTInfo(FDT *Rt, uint32 Index)
{
	uint16 ByteIndex;
	uint32 SecIndex;
	uint8  temp;
    
	temp = NOT_FAT_DISK;
	Index = Index << 5;        /* 32:sizeof(FDT) */
	if (FATType == FAT12 || FATType == FAT16)
	{
		temp = FDT_OVER;
		if (Index < ((uint32)BootSector.RootDirSectors << LogBytePerSec))
		{
			ByteIndex = Index & (BootSector.BPB_BytsPerSec - 1);
            SecIndex  = (Index >> LogBytePerSec) + (BootSector.FirstDataSector - BootSector.RootDirSectors);

            temp      = ReadFDTInfo(Rt, SecIndex, ByteIndex);
        }
	}
    
	return (temp);
}

/*********************************************************************************************************
** Name	:GetFDTInfo
** Description	:��ȡָ��Ŀ¼ָ���ļ�(Ŀ¼)��Ϣ
** Input	:Rt���洢������Ϣ��ָ��
**        			 ClusIndex��Ŀ¼�״غ�
**        			 Index���ļ�(Ŀ¼)��FDT�е�λ��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	:FATType, BootSector
** call module	:
********************************************************************************************************/
IRAM_FAT
uint8 GetFDTInfo(FDT *Rt, uint32 ClusIndex, uint32 Index)
{
	uint16 ByteIndex;
	uint16 ClusCnt;
	uint32 SecIndex, i;
	uint32 NextClus;

	if (ClusIndex == EMPTY_CLUS)
	{
		if (FATType == FAT32)
		{
			ClusIndex = BootSector.BPB_RootClus;
		}
		else
		{
			return (GetRootFDTInfo(Rt, Index));
		}
	}

	if (FATType == FAT12 || FATType == FAT16 || FATType == FAT32)
	{
		if (ClusIndex != FdtData.DirClus)
		{
			FdtData.DirClus = ClusIndex;	//cache dir clus
			FdtData.CurClus = ClusIndex;
			FdtData.Cnt     = 0;
		}
        
		Index     = Index << 5;
		ByteIndex = Index & (BootSector.BPB_BytsPerSec - 1);
		SecIndex  = Index >> LogBytePerSec;
		ClusCnt   = SecIndex >> LogSecPerClus;
        
		if (ClusCnt < FdtData.Cnt)
		{
			FdtData.Cnt     = 0;
			FdtData.CurClus = ClusIndex;
		}
		else
		{
			SecIndex -= FdtData.Cnt << LogSecPerClus;
		}
        
		/* ����Ŀ¼���������� */
		i = BootSector.BPB_SecPerClus;
		while(SecIndex >= i)
		{
			UserIsrDisable();

            FdtData.CurClus = FATGetNextClus(FdtData.CurClus, 1);
			FdtData.Cnt++;
            
            UserIsrEnable();
            
			if (FdtData.CurClus <= EMPTY_CLUS_1 || FdtData.CurClus >= BAD_CLUS) 
			{
				return (FDT_OVER);
			}
            
			SecIndex -= i;
		}
		SecIndex = ((FdtData.CurClus - 2) << LogSecPerClus) + SecIndex + BootSector.FirstDataSector;

        return (ReadFDTInfo(Rt, SecIndex, ByteIndex));
	}
    
	return (NOT_FAT_DISK);
}

#ifdef ENCODE
/*********************************************************************************************************
** Name	:SetRootFDTInfo
** Description	:���ø�Ŀ¼ָ���ļ�(Ŀ¼)��Ϣ
** Input	:FDTData��Ҫд�����Ϣ
**        			 Index���ļ�(Ŀ¼)��FDT�е�λ��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	:FATType, BootSector
** call module	:WriteFDTInfo
********************************************************************************************************/
IRAM_ENCODE
uint8 SetRootFDTInfo(uint32 Index, FDT *FDTData)
{
	uint16 ByteIndex;
	uint32 SecIndex;
	uint8 Rt;
    
	Rt    = NOT_FIND_DISK;
	Index = Index << 5;
    
	if (FATType == FAT12 || FATType == FAT16)
	{
		Rt = FDT_OVER;
		if (Index < (BootSector.RootDirSectors << LogBytePerSec))
		{
			ByteIndex = Index & (BootSector.BPB_BytsPerSec - 1);
			SecIndex = (Index >> LogBytePerSec) + (BootSector.FirstDataSector-BootSector.RootDirSectors);

            Rt = WriteFDTInfo(FDTData, SecIndex, ByteIndex);
		}
	}
    
	return (Rt);
}

/*********************************************************************************************************
** Name	:SetFDTInfo
** Description	:����ָ��Ŀ¼ָ���ļ�(Ŀ¼)��Ϣ
** Input	:FDTData��Ҫд�����Ϣ
**        			 ClusIndex��Ŀ¼�״غ�
**        			 Index���ļ�(Ŀ¼)��FDT�е�λ��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	:FATType, BootSector
** call module	:
********************************************************************************************************/
IRAM_ENCODE
uint8 SetFDTInfo(uint32 ClusIndex, uint32 Index, FDT *FDTData)
{
	uint16 ByteIndex;
	uint32 SecIndex;
	uint8 i;
    
	if (ClusIndex == EMPTY_CLUS)
	{
		if (FATType == FAT32)
		{
			ClusIndex = BootSector.BPB_RootClus;
		}
		else
		{
			return (SetRootFDTInfo(Index, FDTData));
		}
	}

	if (FATType == FAT12 ||FATType == FAT16 || FATType == FAT32)
	{
		Index     = Index << 5;
		ByteIndex = Index & (BootSector.BPB_BytsPerSec - 1);
		SecIndex  = Index >> LogBytePerSec;	/* ����Ŀ¼������ƫ������ */
		i = BootSector.BPB_SecPerClus;
        
		while(SecIndex >= i)
		{
			ClusIndex = FATGetNextClus(ClusIndex, 1);
            
			if (ClusIndex <= EMPTY_CLUS_1 || ClusIndex >= BAD_CLUS) 
			{
				return (FDT_OVER);
			}
            
			SecIndex -= i;
		}
        
		SecIndex = ((ClusIndex - 2) << LogSecPerClus) + SecIndex + BootSector.FirstDataSector;
        
		return (WriteFDTInfo(FDTData, SecIndex, ByteIndex));
	}
    
	return (NOT_FAT_DISK);
}

#endif //ENCODE

/*********************************************************************************************************
** Name	:FindFDTInfo
** Description	:��ָ��Ŀ¼����ָ���ļ�(Ŀ¼)
** Input	:Rt���洢������Ϣ��ָ��
**        			 ClusIndex��Ŀ¼�״غ�
**        			 FileName���ļ�(Ŀ¼)��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	: null
** call module	:GetFDTInfo
********************************************************************************************************/
IRAM_FAT
uint8 FindFDTInfo(FDT *Rt, uint32 ClusIndex, uint8 *FileName)
{
	uint32 i;
	uint8 temp, j;
    
	i = 0;
	if (FileName[0] == FILE_DELETED)
	{
		FileName[0] = ESC_FDT;
	}
    
	while (1)
	{
		temp = GetFDTInfo(Rt, ClusIndex, i);		//����RETURN_OK\NOT_FAT_DISK\FDT_OVER
		if (temp != RETURN_OK)
		{
			break;
		}
        
		if (Rt->Name[0] == FILE_NOT_EXIST)
		{
			temp = NOT_FIND_FDT;
			break;
		}
        
		if ((Rt->Attr & ATTR_VOLUME_ID) == 0)
		{
			for (j = 0; j < 11; j++)
			{
				if (FileName[j] != Rt->Name[j])
				{
					break;
				}
			}

            if (j == 11)
			{
				temp = RETURN_OK;
				break;
			}
		}
        
		i++;
	}
    
	if (FileName[0] == ESC_FDT)
	{
		FileName[0] = FILE_DELETED;
	}
    
	return (temp);
}

#ifdef LONG_DIR_PATH
/*******************************************************************************
** Name: FatCheckSum
** Input:uint8 * pFileName
** Return: uint8
** Owner:Aaron.sun
** Date: 2014.3.18
** Time: 14:16:13
*******************************************************************************/
IRAM_FAT
uint8 FatCheckSum(uint8 * pFileName)
{
    uint8 i;
    uint8 Sum;
    Sum = 0;
    for (i=11; i!=0; i--)
    {
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFileName++;
    }
    return (Sum);
}


/*********************************************************************************************************
** Name	:FindFDTInfo
** Description	:��ָ��Ŀ¼����ָ���ļ�(Ŀ¼)
** Input	:Rt���洢������Ϣ��ָ��
**        			 ClusIndex��Ŀ¼�״غ�
**        			 FileName���ļ�(Ŀ¼)��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	: null
** call module	:GetFDTInfo
********************************************************************************************************/
IRAM_FAT
uint8 FindFDTInfoLong(FDT *Rt, uint32 ClusIndex, uint32 *pIndex,  uint16 *FileName)
{
	uint32 i, index;
	uint8 temp, j;
    uint16 state = 0;
    uint16 *strCur, *plfName;
    uint8 *buf;
    uint16 lfName[13];

    uint32 LfnCnt;
    uint8 CheckSum;
    LfnCnt = StrLenW(FileName) / 13 + (StrLenW(FileName) % 13? 1:0);

    if (LfnCnt > MAX_LFN_ENTRIES)
    {
        LfnCnt = MAX_LFN_ENTRIES;
    }
  
	i = 0;
	if (FileName[0] == FILE_DELETED)
	{
		FileName[0] = ESC_FDT;
	}
    
	while (1)
	{
		temp = GetFDTInfo(Rt, ClusIndex, i);		//����RETURN_OK\NOT_FAT_DISK\FDT_OVER
		if (temp != RETURN_OK)
		{
			break;
		}
        
		if (Rt->Name[0] == FILE_NOT_EXIST)
		{
			temp = NOT_FIND_FDT;
			break;
		}

        if  (Rt->Name[0] == FILE_DELETED)
        {
            i++;
            continue;
        }

        if(state != 2)
        {
            if  (Rt->Attr != ATTR_LFN_ENTRY)
            {
                i++;
                continue;
            }
        }

        if (state == 0)
        {
            if(Rt->Name[0] == (LfnCnt | 0x40)) //first          //�ҵ����ļ����ĵ�1��
            {
                state = 1;
                index = LfnCnt;
                CheckSum = Rt->Name[13];
            }
            else
            {
                i++;
                continue;
            }
        }

        if (state == 1)
        {

            if (CheckSum != Rt->Name[13])
            {
                i++;                
                state = 0;
                continue;
            }

            if ((Rt->Name[0] & LFN_SEQ_MASK) != index)
            {
                i++;
                state = 0;
                continue;
            }

            strCur = FileName + (index - 1) * 13;

            /*��ȡ�ļ���*/
            buf = (uint8 *)Rt;
            plfName = lfName;
            buf++;
            for (j = 0; j<5; j++)
            {//ǰ��10����byte
                *plfName = *(uint16 *)buf;
                //printf("char = %04x\n", *plfName);
                buf += 2;
                //*plfName |= ((uint16)(*buf++))<<8;
                plfName++;
            }
            buf += 3;

            for (j = 0; j<6;j++)
            {
                *plfName = *(uint16 *)buf;
                //printf("char = %04x\n", *plfName);
                buf += 2;
                //*plfName |= ((uint16)(*buf++))<<8;
                plfName++;
            }
            buf += 2;

            for (j = 0; j<2; j++)
            {
                *plfName = *(uint16 *)buf;
                //printf("char = %04x\n", *plfName);
                buf += 2;
                //*plfName |= ((uint16)(*buf++))<<8;
                plfName++;
            }

            j = 13;

            plfName = lfName;
            //printf("strcur len = %d\n", StrLenW(strCur));
            if(StrCmpW(plfName, strCur, (StrLenW(strCur) > 13)? 13 : StrLenW(strCur)) == 0)
            {
                index--;
                if (index == 0)
                {
                    state = 2;
                }
                i++;              
                continue;
            }
            else
            {
                i++;
                state = 0;
                continue;
            }      
        }


        if (state == 2)
        {
            if (FatCheckSum(Rt->Name) == CheckSum)
            {
                *pIndex = i;
                temp = RETURN_OK;
                //printf("ok,Attr= %d\n", Rt->Attr);
                break;
            }
            else
            {
                state = 0;
                i++;
                continue;
            }
        }

	}
    
	if (FileName[0] == ESC_FDT)
	{
		FileName[0] = FILE_DELETED;
	}
    
	return (temp);
}
#endif

#ifdef ENCODE

/*******************************************************************************
** Name: AddFDTLong
** Input:HDC dev, uint32 ClusIndex,  uint32 * pIndex, uint16 * FileName
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.19
** Time: 9:39:12
*******************************************************************************/
IRAM_ENCODE
uint8 AddFDTLong(uint32 Clus, FDT *FDTData, uint32 * pIndex, uint16 * FileName)
{     
    uint32 i,k;
    FDT TempFDT;
    uint8 ret;
    uint32 PrevClus, ClusCnt;
    uint32 LfnCnt,EmpFdtCnt,EmpFdtClus,EmpFdtIndex;

    uint16 * strCur;
    uint8 * buf;
    uint8 CheckSum;
    uint8 FileNameEnd;


    ret = FindFDTInfoLong(&TempFDT, Clus, pIndex, FileName);		//NOT_FIND_FDT\RETURN_OK

    if (ret == RETURN_OK)
    {
        return (FDT_EXISTS);
    }

    if (FDTData->Name[0] == FILE_DELETED)
    {
        FDTData->Name[0] = ESC_FDT;
    }

    CheckSum = FatCheckSum(FDTData->Name);

    i = 0;

    ret = RETURN_FAIL;

    LfnCnt = StrLenW(FileName) / 13 + ((StrLenW(FileName) % 13)? 1:0);
    EmpFdtCnt = 0;

    ret = RETURN_OK;

    //printf("LfnCnt = %d = %d\n", LfnCnt, StrLenW(FileName));

    while (ret == RETURN_OK)
    {
        ret = GetFDTInfo(&TempFDT, Clus, i);
        if (ret == RETURN_OK)
        {
            if (TempFDT.Name[0] == FILE_DELETED)
            {
                if (EmpFdtCnt == 0)
                {
                    EmpFdtClus = 0;
                    EmpFdtIndex = i;
                    EmpFdtCnt++;
                }
                else
                {
                    EmpFdtCnt++;
                }
            }
            else if (TempFDT.Name[0] == FILE_NOT_EXIST)
            {
                if (EmpFdtCnt == 0)
                {
                    EmpFdtClus = 0;
                    EmpFdtIndex = i;
                    EmpFdtCnt++;
                }
                else
                {
                    EmpFdtCnt++;
                }

                do
                {
                    if (PrevClus != Clus)
                    {
                        PrevClus = Clus;
                        i = 0;
                    }

                    i++;

                    ret = GetFDTInfo(&TempFDT, Clus, i);

                    if (ret == RETURN_OK)
                    {
                        EmpFdtCnt++;
                    }
                    else
                    {
                        break;
                    }

                    if (EmpFdtCnt == (LfnCnt + 1))
                    {
                        break;
                    }

                }
                while (1);

            }
            else
            {
                EmpFdtCnt = 0;
            }

            if (EmpFdtCnt == (LfnCnt + 1))
            {
                break;
            }

        }       
        i++;

    }

    if (ret == FDT_OVER && Clus != EMPTY_CLUS)	//��ǰĿ¼�������,������һ����
    {
        *pIndex = i;

        ClusCnt = ((LfnCnt - EmpFdtCnt + 1) * 32 / 512) / BootSector.BPB_SecPerClus
                  + ((LfnCnt - EmpFdtCnt + 1) * 32 / 512) % BootSector.BPB_SecPerClus? 1:0;


        for (i = 0; i < ClusCnt; i++)
        {
            ret = FATAddClus(Clus);
            if (ret < 0)
            {
                break;
            }
            else
            {
                ClearClus(Clus);		//2006.11.24 debug by lxs
                Clus = (uint32)ret;
            }
        }
    }
    else if (ret == FDT_OVER && Clus == EMPTY_CLUS)
    {
        ret = ROOT_FDT_FULL;
    }

    if (ret >= 0)
    {
        memset(&TempFDT, 0X00, 32);

        FileNameEnd = 0;

        for (i = 0; i < LfnCnt; i++)
        {
            strCur = FileName + (LfnCnt - i - 1) * 13;

            /*��ȡ�ļ���*/
            buf = (uint8 *)&TempFDT;

            if(i == 0)
            {
                buf[0] = LfnCnt | 0x40;
            }
            else
            {
                buf[0] = LfnCnt - i;                
            }
            
            buf[11] = ATTR_LFN_ENTRY;
            buf[13] = CheckSum;

            if(i > 0)
            {
                FileNameEnd = 0;
            }

            buf++;
            for (k = 0; k < 5; k++)
            {//ǰ��10����byte
                if(FileNameEnd)
                {
                    *(uint16 *)buf = 0xffff;
                }
                else
                {
                    *(uint16 *)buf = *strCur;
                }
                //printf("char = %04x\n", *plfName);
                buf += 2;
                //*plfName |= ((uint16)(*buf++))<<8;
                if(*strCur == 0)
                    FileNameEnd = 1;

                if(FileNameEnd == 0)
                    strCur++;
            }
            buf += 3;

            for (k = 0; k < 6;k++)
            {
                if(FileNameEnd)
                {
                   *(uint16 *)buf = 0xffff; 
                }
                else
                {
                    *(uint16 *)buf = *strCur;
                }
                
                //printf("char = %04x\n", *plfName);
                buf += 2;
                //*plfName |= ((uint16)(*buf++))<<8;

                if(*strCur == 0)
                    FileNameEnd = 1;

                if(FileNameEnd == 0)
                    strCur++;
                
            }
            buf += 2;

            for (k = 0; k < 2; k++)
            {
                if(FileNameEnd)
                {
                    *(uint16 *)buf = 0xffff;
                }
                else
                {
                    *(uint16 *)buf = *strCur;
                }
                //printf("char = %04x\n", *plfName);
                buf += 2;
                //*plfName |= ((uint16)(*buf++))<<8;

                if(*strCur == 0)
                    FileNameEnd = 1;

                if(FileNameEnd == 0)
                    strCur++;
            }

            ret = SetFDTInfo(EmpFdtClus, EmpFdtIndex + i, &TempFDT);
            if (ret != RETURN_OK)
            {
                break;
            }
        }

        ret = SetFDTInfo(EmpFdtClus, EmpFdtIndex + i, FDTData);

    }

    if (FDTData->Name[0] == ESC_FDT)
    {
        FDTData->Name[0] = FILE_DELETED;
    }

    return (ret);
}


/*********************************************************************************************************
** Name	:AddFDT
** Description	:��ָ��Ŀ¼������ָ���ļ�(Ŀ¼)
** Input	:ClusIndex��Ŀ¼�״غ�
**        			 FDTData���ļ�(Ŀ¼)��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	:BootSector
** call module	:FindFDTInfo,GetFDTInfo,SetFDTInfo,FATAddClus
********************************************************************************************************/
IRAM_ENCODE
uint8 AddFDT(uint32 ClusIndex, FDT *FDTData, uint32 * Index)
{
	uint32 i;
	FDT TempFDT;
	uint8 temp;

	if (ClusIndex == EMPTY_CLUS)
	{
		if (FATType == FAT32)
		{
			ClusIndex = BootSector.BPB_RootClus;
		}
	}

	temp = FindFDTInfo(&TempFDT, ClusIndex, FDTData->Name);		//NOT_FIND_FDT\RETURN_OK
	if (temp == RETURN_OK)
	{
		return (FDT_EXISTS);
	}

	if (temp != NOT_FIND_FDT && temp != FDT_OVER)		//NOT_FAT_DISK
	{
		return (temp);
	}

	if (FDTData->Name[0] == FILE_DELETED)
	{
		FDTData->Name[0] = ESC_FDT;
	}

	i = 0;
	temp = RETURN_OK;
	while(temp == RETURN_OK)
	{
		temp = GetFDTInfo(&TempFDT, ClusIndex, i);
		if (temp == RETURN_OK)
		{
			if (TempFDT.Name[0] == FILE_DELETED || TempFDT.Name[0] == FILE_NOT_EXIST)
			{
				temp = SetFDTInfo(ClusIndex, i, FDTData);
                *Index = i;
				break;
			}
		}
		i++;
	}   
	
	if (temp == FDT_OVER && ClusIndex != EMPTY_CLUS)	//��ǰĿ¼�������,������һ����
	{
        *Index = i;
        i = FATAddClus(ClusIndex);
		temp = DISK_FULL;
		if (i != BAD_CLUS)
		{
			ClearClus(i);		//2006.11.24 debug by lxs
			temp = SetFDTInfo(i, 0, FDTData);
		}
	}
	
	if (FDTData->Name[0] == ESC_FDT)
	{
		FDTData->Name[0] = FILE_DELETED;
	}
	return (temp);
}

/*********************************************************************************************************
** Name	:DelFDT
** Description	:��ָ��Ŀ¼ɾ��ָ���ļ�(Ŀ¼)
** Input	:ClusIndex��Ŀ¼�״غ�
**        			 FileName���ļ�(Ŀ¼)��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	: null
** call module	:GetFDTInfo,SetFDTInfo
********************************************************************************************************/
IRAM_ENCODE
uint8 DelFDT(uint32 ClusIndex, uint8 *FileName)
{
	uint32 i;
	FDT TempFDT;
	uint8 temp, j;
    
	i = 0;
	if (FileName[0] == FILE_DELETED)
	{
		FileName[0] = ESC_FDT;
	}
    
	while (1)
	{	    
        #ifdef _WATCH_DOG_
        WatchDogReload();
        #endif
            
		temp = GetFDTInfo(&TempFDT, ClusIndex, i);
		if (temp != RETURN_OK)
		{
			break;
		}
            
		if (TempFDT.Name[0] == FILE_NOT_EXIST)
		{
			temp = NOT_FIND_FDT;
			break;
		}
        
//		if ((TempFDT.Attr & ATTR_VOLUME_ID) == 0)		//��겻��ɾ��
		{
			for (j = 0; j < 11; j++)
			{
				if (FileName[j] != TempFDT.Name[j])
				{
					break;
				}
			}
            
			if (j == 11)
			{
				do
				{
                    #ifdef _WATCH_DOG_
                    WatchDogReload();
                    #endif
                    
					TempFDT.Name[0] = FILE_DELETED;
					temp = SetFDTInfo(ClusIndex, i, &TempFDT);
                    
					if (RETURN_OK != GetFDTInfo(&TempFDT, ClusIndex, --i))
					{
						break;
					}
                    
				}while (TempFDT.Attr == ATTR_LFN_ENTRY);			//���ļ�����Ҫ�ҵ����ļ���

				break;
			}
		}

		i++;
	}
    
	if (FileName[0] == ESC_FDT)
	{
		FileName[0] = FILE_DELETED;
	}
    
	return (temp);
}

/*********************************************************************************************************
** Name	:ChangeFDT
** Description	:�ı�ָ��Ŀ¼ָ���ļ���Ŀ¼��������
** Input	:ClusIndex��Ŀ¼�״غ�
**        			 FileName���ļ���Ŀ¼����
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	: null
** call module	:GetFDTInfo,SetFDTInfo
********************************************************************************************************/
IRAM_ENCODE
uint8 ChangeFDT(uint32 ClusIndex, FDT *FDTData)
{
	uint32 i;
	uint8  temp, j;
	FDT    TempFDT;

	i = 0;
	if (FDTData->Name[0] == FILE_DELETED)
	{
		FDTData->Name[0] = ESC_FDT;
	}
    
	while (1)
	{
		temp = GetFDTInfo(&TempFDT, ClusIndex, i);
		if (temp != RETURN_OK)
		{
			break;
		}
            
		if (TempFDT.Name[0] == FILE_NOT_EXIST)
		{
			temp = NOT_FIND_FDT;
			break;
		}
        
		if ((TempFDT.Attr & ATTR_VOLUME_ID) == 0)
		{
			for (j = 0; j < 11; j++)
			{
				if (FDTData->Name[j] != TempFDT.Name[j])
				{
					break;
				}
			}
            
			if (j==11)
			{
				temp = SetFDTInfo(ClusIndex, i, FDTData);
				break;
			}
		}
        
		i++;
	}
    
	if (FDTData->Name[0] == ESC_FDT)
	{
		FDTData->Name[0] = FILE_DELETED;
	}
    
	return (temp);
}

/*********************************************************************************************************
** Name	:FDTIsLie
** Description	:��ָ��Ŀ¼�鿴ָ���ļ�(Ŀ¼)�Ƿ����
** Input	:ClusIndex��Ŀ¼�״غ�
**        			 FileName���ļ�(Ŀ¼)��
** Output      	:RETURN_OK���ɹ�
** �����ο�  fat.h�й��ڷ���ֵ��˵��
** global	: null
** call module	:GetFDTInfo
********************************************************************************************************/
IRAM_ENCODE
uint8 FDTIsLie(uint32 ClusIndex, uint8 *FileName)
{
	uint32 i;
	uint8  temp, j;
	FDT    temp1;
    
	i = 0;
	if (FileName[0] == FILE_DELETED)
	{
		FileName[0] = ESC_FDT;
	}
    
	while (1)
	{
		temp = GetFDTInfo(&temp1, ClusIndex, i);
		if (temp == FDT_OVER)
		{
			temp = NOT_FIND_FDT;
			break;
		}

		if (temp != RETURN_OK)
		{
			break;
		}

		if (temp1.Name[0] == FILE_NOT_EXIST)
		{
			temp = NOT_FIND_FDT;
			break;
		}
		
		if ((temp1.Attr & ATTR_VOLUME_ID) == 0)
		{
			for (j = 0; j < 11; j++)
			{
				if (FileName[j] != temp1.Name[j])
				{
					break;
				}
			}
            
			if (j == 11)
			{
				temp = FDT_EXISTS;
				break;
			}
		}
        
		i++;
	}
    
	if (FileName[0] == ESC_FDT)
	{
		FileName[0] = FILE_DELETED;
	}
    
	return (temp);
}
#endif

/*
********************************************************************************
*
*                         End of FDT.c
*
********************************************************************************
*/
