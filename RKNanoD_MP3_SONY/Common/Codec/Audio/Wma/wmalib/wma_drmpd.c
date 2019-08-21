/*
************************************************************************************************************************
*
*  Copyright (C),2006, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     drmpd.c
*  Author:         hwg, zsx, zym
*  Description:     
*  Remark:         
*                       
*  History:        
*           <author>      <time>     <version>       <desc>
*                         06/8/21       1.0           
*
************************************************************************************************************************
*/
#include "../include/audio_main.h"
#include "..\wmaInclude\predefine.h"

#ifdef A_CORE_DECODE
#ifndef WMAAPI_NO_DRM
#include "..\wmaInclude\drmtype.h"
#include "..\wmaInclude\drmpd.h"
#ifdef WMA_DEC_INCLUDE
__attribute__((section("WmaCommonCode")))
struct_DRM_state DRM_state;

//#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData",rodate = "WmaCommonCode"

//struct_DRM_state DRM_state;


#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

void memcpyDWORD(UWORD32* pto, UWORD32* pfrom, LENGTH_TYPE len)
{
	while(len > 0)
	{
		*pto++ = *pfrom++;
		len--;
	}
}
#pragma arm section code

#endif

#if 0//defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

LENGTH_TYPE ZSP_strlen(BYTE *string)
{
    LENGTH_TYPE len = 0;
    BYTE hi, low;
    
    while (1)
    {
        hi = (*string >> 8) & 0xFF;
        low = *string++ & 0xFF;  
        if (low == 0)
        {
            break;
        }    
        if (hi == 0)
        {
            len ++;
            break;
        } 
        len += 2;
    }
    return len;  
}


UWORD16 ZSP_memcmp(BYTE* p1, BYTE* p2, LENGTH_TYPE len)
{
	COUNT_TYPE i = 0;
	
	for (i = 0; i < len/2; ++i)
	{
		if (*p1++ != *p2++)
		{
			return (1);
		}
	}
	if (len & 1)
	{
		if ((*p1 & 0xFF) != (*p2 & 0xFF))
		{
			return (1);
		}
	}
	return (0);
}


void ZSP_memcpy(BYTE* pto, BYTE* pfrom, LENGTH_TYPE len)
{
	COUNT_TYPE i = 0;
	
	for (i = 0; i < len/2; ++i)
	{
		*pto++ = *pfrom++;
	}
	if (len & 1)
	{
		BYTE temp = *pfrom & 0xFF;
		*pto &= 0xFF00;
		*pto |= temp;
	}
}
#pragma arm section code 

#endif
/*___________________________________________________________________________
 |                                                                           |
 |   drmpd                                                                   |
 |___________________________________________________________________________|
*/

/**************************************************
 * CDrmPD_Init
 **************************************************
 */
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

DRM_RESULT CDrmPD_Init__(void* pDRM_state)
{
	DRM_RESULT dr = DRM_SUCCESS;
	struct_DRM_state *ptemp = (struct_DRM_state*)pDRM_state;
	
	ptemp->m_DecryptInited = 0;
	ptemp->mem2.m_fInited = 0;
	ptemp->mem2.m_fDecryptInited = 0;
//	DRMInit__(&ptemp->mem2);
	dr = CPDLicStore_Init__(&ptemp->mem3.mem2);
	return(dr);
}	// 完成
#pragma arm section code 

#endif
/**************************************************
 * CDrmPD_Decrypt
 **************************************************
 */
#ifdef WMAHIGHRATE
#pragma arm section code = "WmaHighRateCode"


DRM_RESULT CDrmPD_Decrypt__ (void* pDRM_state, BYTE* pbBuffer, LENGTH_TYPE cbBuffer)
{
	DRM_RESULT dr = 0;
	struct_DRM_state *ptemp = (struct_DRM_state*)pDRM_state;
	if(ptemp->m_DecryptInited != 0)
	{
		dr = DRMDecrypt__(&ptemp->mem2, cbBuffer, pbBuffer);
		return(dr);
	}
	else
	{
		return (3);
	}
}	// 完成
#pragma arm section code 

#endif
/**************************************************
 * CDrmPD_InitDecrypt
 **************************************************
 */
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

DRM_RESULT CDrmPD_InitDecrypt__(void* pDRM_state,
                          PMLICENSE* pmlic,		// 未知的结构体的地址
                          BYTE* pPMID,
                          LENGTH_TYPE cbPMID,
                          BYTE* pLicData,
                          UWORD32* pOffset,
                          UWORD32* pActual)
{
	BYTE b[16];
	BYTE *pb = b;
	struct_DRM_state *ptemp = (struct_DRM_state*)pDRM_state;
	DRM_RESULT dr = DRM_SUCCESS;
	if((pmlic == NULL) || (pPMID == NULL) || (cbPMID == 0) || (pOffset == NULL)
		|| (pActual == NULL) || ((pmlic->ld.KID[0] & 0xFF) == 0) || (STRLEN(pmlic->ld.KID) + 1 > 25))
	{
		dr = 1;
		goto ErrorExit;
	}
	if ((pLicData == NULL) && (*pActual != 0))
	{
		dr = 1;
		goto ErrorExit;
	}
	dr = CPDLicStore_Lookup__(&ptemp->mem3.mem2,
							ptemp,
							pPMID,
							cbPMID,
							&pmlic->ld,
							pmlic,
							pLicData,
							pOffset,
							pActual,
							pb);
	if (dr != 0)
	{
		goto ErrorExit;
	}
	dr = DRMKeySetup__(&ptemp->mem2, b[0], &b[__CB_DECL(1)]);
	if (dr == 0)
	{
		ptemp->m_DecryptInited = 1;
	}
ErrorExit:
	return(dr);
}	// 完成
#pragma arm section code 

#endif

/**************************************************
 * CDrmPD_InitPacket
 **************************************************
 */
#ifdef WMAHIGHRATE
#pragma arm section code = "WmaHighRateCode"


DRM_RESULT CDrmPD_InitPacket__(void* pDRM_state, BYTE* pLast15, UWORD16 cbPayloadSize)
{
	DRM_RESULT dr = 0;//返回值
	struct_DRM_state *pin = (struct_DRM_state*)pDRM_state; 
	
	if( pDRM_state == NULL)
	{
		return(3);	
	}
	dr = DRMInitPacket__(&pin->mem2, cbPayloadSize, pLast15);

  return(dr);
}
#pragma arm section code 

#endif

/*___________________________________________________________________________
 |                                                                           |
 |   pdlicstr                                                                |
 |___________________________________________________________________________|
*/


/**************************************************
 * hash
 **************************************************
 */
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

UWORD32 hash__(BYTE* pin)
{
	BYTE rgbDigest[__CB_DECL(20)];
	SHA_CONTEXT contextSHA;
	LENGTH_TYPE len = 0;
	UWORD32 temp = 0;
	
	A_SHAInit__(&contextSHA);
	len = STRLEN(pin);
	A_SHAUpdate__(&contextSHA, pin, len);
	A_SHAFinal__(&contextSHA, rgbDigest);
//	temp = *((UWORD32*)rgbDigest);
	BYTES_TO_DWORD(temp, rgbDigest);
	temp %= 10;		// __rt_udiv10, 也可能是用div函数
	return (temp);
}


/**************************************************
 * CPDLicStore_Init
 **************************************************
 */
 
DRM_RESULT CPDLicStore_Init__(structX_unknown3* pin)
{
	pin->mem11 = 5;
#if SIXTEEN_BIT_ADDRESSING
	{
		COUNT_TYPE i = 0;
		BYTE *pb = (BYTE*)&pin->ld;
		for (; i < 33; ++i)
		{
			*pb++ = 0;
		}
	}
#else
	memset((BYTE*)&pin->ld, 0, 64);		// 把64BYTE清零，在变为ZSP时特别注意长度
#endif
	pin->pbPMID = 0;
	pin->cbPMID = 0;
	pin->mem5 = 0;
	pin->mem6 = 0;
	pin->mem7 = 0;
	pin->mem8 = 0;
	return(0);
}


/**************************************************
 * MatchLicense
 **************************************************
 */
 #if 0
DRM_RESULT MatchLicense__(LIC_Data* pld, 
					BYTE* pLicData)
{
	
	LIC_Data* ptemp = (LIC_Data*)pLicData;
	DRM_RESULT dr = 0;
	COUNT_TYPE i = 0;
	
	dr = strcmp(pld->KID, ptemp->KID);
	if (dr != 0)
	{
		return (5);
	}
	
	for (i = 0; i < 4; ++i)
	{
		if (pld->appSec[i] < ptemp->appSec[i])
		{
			return (6);
		}
	}
	
	for (i = 0; i < 4; ++i)
	{
		if ((pld->rights[i] & ~ptemp->rights[i]) != 0)
		{
			return (7);
		}
	}
	

	return (0);
}
#endif	


/**************************************************
 * PMIDToPMKey
 **************************************************
 */
 
DRM_RESULT PMIDToPMKey__(struct_DRM_state* pDRMState,
//				   UWORD32 temp1,
				   UWORD16 temp,
				   SHA_CONTEXT* pContextSHA,
				   BYTE* pPMID,
				   LENGTH_TYPE cbPMID,
				   BYTE* pLicData,
				   BYTE* pbPMKey,
				   LENGTH_TYPE cbPMKey)
{
	DRM_RESULT dr = 0;
	COUNT_TYPE a = 0;
	LENGTH_TYPE cbkid;
	BYTE bdata1[4] = {0x34, 0xA8, 0x7e, 0x84};
	BYTE bdata2[8] = {0xB8, 0xE1, 0x8A, 0x15, 0x4A, 0x58, 0x62, 0x7E};
	BYTE btemp = 0;
	
	if (pPMID == NULL || cbPMID == 0 || pLicData == NULL || pDRMState == NULL)
	{
		return (1);
	}
	cbkid = STRLEN(pLicData);
	if (cbkid + 1 > 25)
	{
		return (1);
	}
	if (pbPMKey == NULL || cbPMKey == 0)
	{
		return (1);
	}
	if (cbPMKey > 0x14)
	{
		return (1);
	}

	A_SHAUpdate__(pContextSHA, bdata1, 4);
	if (temp > 0)
	{
		return (10);
	}

	A_SHAUpdate__(pContextSHA, bdata2, 8);
	MEMCPY(&pDRMState->mem2.m_shadata, pContextSHA, 0x74);
	A_SHAUpdate__(&pDRMState->mem2.m_shadata, pPMID, cbPMID);
	A_SHAUpdate__(&pDRMState->mem2.m_shadata, pLicData, cbkid);
	A_SHAFinal__(&pDRMState->mem2.m_shadata, pDRMState->mem2.m_shaOut);
	MEMCPY(pbPMKey, &pDRMState->mem2.m_shaOut, cbPMKey);
	return (dr);
}


/**************************************************
 * FlipBits
 **************************************************
 */
 
void FlipBits__(BYTE* pbIn, LENGTH_TYPE cbIn)
{
	COUNT_TYPE i = 0;
	
	for (i = 0; i < cbIn; ++i)
	{
		*pbIn = ~(*pbIn) & 0xFF;
		++pbIn;
	}
}


/**************************************************
 * PMContentKeyToContentKey
 **************************************************
 */
 
DRM_RESULT PMContentKeyToContentKey__(struct_DRM_state* pDRMState,
							    BYTE* pbPMKey,
							    LENGTH_TYPE cbKey,
							    BYTE* pbKey)
{
	DRM_RESULT dr = 0;
	UWORD16 i = 0;
	BYTE desBuf[__CB_DECL(16)];
#if SIXTEEN_BIT_ADDRESSING
	BYTE tempBuf[16];
#endif
	BYTE *pbBuf = desBuf;
	
	desSkey_LS__(pDRMState, &pDRMState->mem2.m_destable, pbPMKey, cbKey);
	for (i = 0; i < 16; i += 8)
	{
		desS__(pbBuf + __CB_DECL(i), pbKey + __CB_DECL(i), &pDRMState->mem2.m_destable, 0);
	}
#if SIXTEEN_BIT_ADDRESSING
	for (i = 0; i < 16; ++i)
	{
		tempBuf[i] = GET_BYTE(desBuf, i);
	}
	pbBuf = tempBuf;
#endif
	i = (UWORD16)(*pbBuf & 0xFF);
	pbBuf++;
	if (i > 16)		// 好像应该为15，如果i为16是否会出错？？？？
	{
		return (9);
	}
	i = i < (UWORD16)(15 - i) ? i : (UWORD16)(15 - i);
	FlipBits__(pbBuf + (desBuf[0] & 0xFF), i);
#if SIXTEEN_BIT_ADDRESSING
	dr = MEMCMP(pbBuf, pbBuf + (desBuf[0] & 0xFF), i*2);
#else
	dr = MEMCMP(pbBuf, pbBuf + (desBuf[0] & 0xFF), i);
#endif
	if (dr != 0)
	{
		return (9);
	}
#if SIXTEEN_BIT_ADDRESSING
	for (i = 0; i < 16; ++i)
	{
		PUT_BYTE(pbKey, i, tempBuf[i]);
	}
#else
	MEMCPY(pbKey, desBuf, 16);
#endif
	return (0);
}


/**************************************************
 * CPDLicStore_Lookup
 **************************************************
 */
 
DRM_RESULT CPDLicStore_Lookup__(
              				structX_unknown3* pin,
							struct_DRM_state* pDRMState,
							BYTE* pPMID,
							LENGTH_TYPE cbPMID,
							LIC_Data* pld,
							PMLICENSE* pmlic,
							BYTE* pLicData,
							UWORD32* pOffset,
							UWORD32* pActual,
							BYTE* pbKey)
{
	DRM_RESULT dr = 0;
	LIC_Data *ptempld = &pin->ld;
	WORD16 flag = 0;
	UWORD32 temp = 0;
	COUNT_TYPE i = 0;
	SHA_CONTEXT contextSHA;
	BYTE bdata1[__CB_DECL(4)] = {0x92, 0x87, 0xe8, 0x06};
	BYTE bdata2[__CB_DECL(4)] = {0x89, 0x10, 0xef, 0x86};
	BYTE bPMKey[__CB_DECL(20)];
	BYTE bCheckSum[__CB_DECL(16)];
	BYTE btemp = 0;
	BYTE b2w[4];
	
	if (pbKey == NULL)
	{
		dr = 1;
		goto ErrorExit;
	}
#if SIXTEEN_BIT_ADDRESSING
	flag = MEMCMP((BYTE*)pld, (BYTE*)ptempld, 66);
#else
	flag = MEMCMP((BYTE*)pld, (BYTE*)ptempld, 64);
#endif
	if ((flag == 0) && (pin->pbPMID != NULL) && (pin->cbPMID == cbPMID))
	{
		flag = MEMCMP(pPMID, pin->pbPMID, cbPMID);
		if (flag == 0)
		{
			goto loc_B4C;
		}
	}
	if ((*pOffset != 0) || (*pActual < 0x54))
	{
		*pOffset = 0;
		*pActual = 0x54;
		dr = 4;
		goto ErrorExit;
	}
	temp = *(UWORD32*)pLicData;
//	temp -= 0x20000;
//	temp += 0xFFFF;
//	if (temp != 0)	// *(UWORD32*)pLicData != 0x10001
	if (temp != 0x10001)
	{
		dr = 3;
		goto ErrorExit;
	}
//	if (dr == 0)
	{
		temp = hash__(pld->KID);
		pin->mem9 = temp;
		temp <<= 3;
		b2w[0] = GET_BYTE(pLicData, temp+8);
		b2w[1] = GET_BYTE(pLicData, temp+9);
		b2w[2] = GET_BYTE(pLicData, temp+10);
		b2w[3] = GET_BYTE(pLicData, temp+11);
		pin->mem7 = AllignedDWORD__(b2w);
//		pin->mem7 = *(UWORD32*)(pLicData + temp + 8);
		b2w[0] = GET_BYTE(pLicData, temp+4);
		b2w[1] = GET_BYTE(pLicData, temp+5);
		b2w[2] = 0;
		b2w[3] = 0;
		pin->mem8 = AllignedDWORD__(b2w);
//		pin->mem8 = (UWORD32)*(UWORD16*)(pLicData + temp + 4);
		if ((pin->mem7 == 0) || (pin->mem8 == 0))
		{
			dr = 5;
			goto ErrorExit;
		}
//		if(dr == 0)
		{
			for (i = 0; i < 10; ++i)
			{
				UWORD16 temp1 = 0;
				temp = *(UWORD32*)( pLicData + __CB_DECL((i << 3) + 8) );
				temp1 = *(UWORD16*)( pLicData + __CB_DECL((i << 3) + 4) );
				if ((temp == 0) && (temp1 != 0))
				{
					dr = 9;
					goto ErrorExit;
				}
				else if ((temp != 0) && (temp1 == 0))
				{
					dr = 9;
					goto ErrorExit;
				}
				else if ((temp != 0) && (temp1 != 0))
				{
					if (temp < 0x54)
					{
						dr = 9;
						goto ErrorExit;
					}
					else
					{
						temp -= 0x54;
						temp %= 0x58;
						if (temp != 0)
						{
							dr = 9;
							goto ErrorExit;
						}
					}
				}
			}		
//			if (dr == 0)
			{
				pin->mem5 = pin->mem7;
				pin->mem6 = 0;
				pin->mem11 = 5;
#if SIXTEEN_BIT_ADDRESSING
				MEMCPY(ptempld, pld, 66); // copy pld to ptempld
#else
				MEMCPY(ptempld, pld, 64); // copy pld to ptempld
#endif
				pin->pbPMID = pPMID;
				pin->cbPMID = cbPMID;
loc_B4C:
//				if (dr == 0)
				{
					if ((*pOffset > pin->mem5) ||
						((*pOffset + *pActual) < (pin->mem5 + 0x58)))
					{
						dr = 4;
					}
					else
					{
						A_SHAInit__(&contextSHA);
						A_SHAUpdate__(&contextSHA, bdata1, 4);
						temp = pin->mem5 - *pOffset;
						pLicData += __CB_DECL(temp);	// temp为奇数时可能会出问题
						temp = *pActual - temp;
						temp /= 0x58;
						temp <<= 16;
						temp >>= 16;
						while (temp != 0)
						{
							pin->mem6 += 1;
							//dr = MatchLicense__( pld, pLicData+__CB_DECL(8) );
							dr = 0;
							if (dr != 5)
							{
								pin->mem11 = (BYTE)dr;
							}
							if (dr != 0)
							{
								temp -= 1;
								temp <<= 16;
								temp >>= 16;
								pLicData += __CB_DECL(0x58);
							}
							else
							{
								UWORD32 temp1 = 0;
								
								A_SHAUpdate__(&contextSHA, bdata2, 4);
								temp1 = (*(UWORD16*)(pLicData + __CB_DECL(0x44)));
//								temp1 = (UWORD32)(*(UWORD16*)(pLicData + 0x3E));
								dr = PMIDToPMKey__(pDRMState,
//												 temp1,
												 (UWORD16)temp1,
												 &contextSHA,
												 pPMID,
												 cbPMID,
												 pLicData + __CB_DECL(8),
												 bPMKey,
												 0x14);
								if (dr != 0)	// dr = 1
								{
									goto ErrorExit;
								}
								dr = checksum2__(pDRMState,
											   bPMKey,
											   0x14,
											   pLicData + __CB_DECL(8),
											   64,
											   bCheckSum);
								if (dr != 0)	// dr = 1
								{
									goto ErrorExit;
								}
								flag = MEMCMP(bCheckSum, 
											  pLicData + 0x48,
											  16);
								if (flag == 0)
								{
#if SIXTEEN_BIT_ADDRESSING
									MEMCPY((BYTE*)pmlic, pLicData, 34);
									pmlic->ld.KID[12] &= 0xFF;
									for (i = 33; i < 62; ++i)
									{
										PUT_BYTE(pmlic, i+1, GET_BYTE(pLicData, i));
									}
									PUT_BYTE(pmlic, 63, 0);
									MEMCPY(((BYTE*)pmlic) + 32, pLicData + 31, 10);
#else
									MEMCPY((BYTE*)pmlic, pLicData, 0x48);
#endif
									MEMCPY(pbKey, &pmlic->ld.PMContentKey, 16);
//									temp = (UWORD32)(pmlic->ld.cbContentKey);
									dr = PMContentKeyToContentKey__(pDRMState,
																  bPMKey,
																  pmlic->ld.cbContentKey,
																  pbKey);
									goto ErrorExit;		// dr = 0 or dr = 9
								}
								else
								{
									dr = 9;
									pin->mem11 = 9;
									temp--;
								}
							}
						}
						dr = pin->mem11;
						if (dr == 0)
						{
							goto ErrorExit;
						}
					}
				}
			}
		}
	}
	if ((dr == 5) || (dr == 6) || (dr == 7) || (dr == 8) || (dr == 4))
	{
		if (pin->mem8 > pin->mem6)
		{
			temp = pin->mem7 + (pin->mem6*11 << 3);
			*pOffset = temp;
			pin->mem5 = temp;
			temp = pin->mem8 - pin->mem6;
			if (temp >= 5)
			{
				temp = 5;
			}
			temp *= 0x58;
			*pActual = temp;
		}
	}
ErrorExit:
	return (dr);
}	// 完成


/*___________________________________________________________________________
 |                                                                           |
 |   dess                                                                     |
 |___________________________________________________________________________|
*/

/**************************************************
 * desSkey
 **************************************************
 
void  desSkey__(
    IN OUT DESTable      *pTable,    
    IN OUT DRM_BYTE      *p, 
    IN     DRM_UINT      c )
{
	DRM_BYTE i;
	DRM_BYTE buf[0x10];
	DRM_BYTE buf0[0x18];//0x18个BYTE，类型未定,其中后8个byte为函数deskey的R1
	RC4_KEYSTRUCT *pks;
	DRM_BYTE* rgbKey = buf;
	
	for (i = 0; i < 0x18; i++)
	{
		buf0[i] = 0; 
 	}
	//.text:00000010                 MOV     R0, SP
  //.text:00000014                 MOV     R1, #0x18
  //.text:0000001C                 BL      __rt_memclr_w
  
  rc4_key__(pks, c, p);
  //.text:00000020                 MOV     R2, R5
  //.text:00000024                 MOV     R1, R6
  //.text:00000028                 ADD     R0, SP, #0x12C+var_114
  //.text:0000002C                 BL      rc4_key
  
  rc4__(pks, 0x18, buf0);
  //.text:00000030                 MOV     R1, #0x18
  //.text:00000034                 ADD     R0, SP, R1
  //.text:00000038                 MOV     R2, SP
  //.text:0000003C                 BL      rc4
  
  deskey__(pTable, rgbKey);
  //.text:00000040                 ADD     R1, SP, #0x12C+var_11C
  //.text:00000044                 MOV     R0, R4
  //.text:00000048                 BL      deskey
  
  memcpy(pTable->mem2, buf0, 0x10);
  //.text:0000004C                 ADD     R0, R4, #0x80
  //.text:00000050                 MOV     R2, #0x10
  //.text:00000054                 MOV     R1, SP
  //.text:00000058                 BL      __rt_memcpy
}
*/

/**************************************************
 * desSkey_LS
 **************************************************
 */
 
void desSkey_LS__(struct_DRM_state* pDRMState, 
				DESTable* pdestable, 
				BYTE* pbKey, 
				LENGTH_TYPE cbKey)
{
	BYTE buffer[__CB_DECL(24)];
	BYTE *pbBuffer = buffer;
	COUNT_TYPE i = 0;
	
	for (; i < __CB_DECL(24); ++i)
	{
		*pbBuffer++ = 0;
	}
	pbBuffer = buffer;
//	memset(pbBuffer, 0, 0x18);
	rc4_key__(&pDRMState->mem2.m_rc4ks, cbKey, pbKey);
	rc4__(&pDRMState->mem2.m_rc4ks, 0x18, pbBuffer);
	deskey__(pdestable, pbBuffer + __CB_DECL(16));
	MEMCPY(&pdestable->mem2, pbBuffer, 16);
}


/**************************************************
 * desS
 **************************************************
 */
 
void desS__(
    DRM_BYTE*      rgbOut,//8byte 
    DRM_BYTE*      p,//可能为8byte            
    DESTable*      pTable,
    DRM_INT        op )
{
	DRM_BYTE  buf[__CB_DECL(0x10)];
	DRM_BYTE  i;
	DRM_BYTE  var;
	DRM_BYTE* pbuf;
	
	for(i = 0;i < __CB_DECL(0x10); i++ )
	{
		buf[i] = pTable->mem2[i];
	}
	 
	if(op == 1)//.text:000000F4                 CMP     R5, #1
	{
		var = __CB_DECL(0);
		//.text:000000FC                 MOVEQ   R1, #0
		pbuf = &buf[__CB_DECL(0)];
	}
	else
	{
		var = __CB_DECL(8);
		//.text:000000F8                 MOVNE   R1, #8
		pbuf = &buf[__CB_DECL(8)];
	}
	
	for(i = 0;i < __CB_DECL(8); i++ )
	{
		buf[i + var] ^= p[i];
	}
	
	des__(rgbOut, pbuf, pTable, op);
	
	if(op == 1)//.text:00000148                 ADD     R2, R0, R12
	{
		var = __CB_DECL(8);
	}
	else
	{
		var = __CB_DECL(0);
	}
	
	for(i = 0;i < __CB_DECL(8); i++)
	{
		rgbOut[i] ^= buf[i + var];
	}
}


/*___________________________________________________________________________
 |                                                                           |
 |   xsum                                                                   |
 |___________________________________________________________________________|
*/

/**************************************************
 * checksum2
 **************************************************
 */
 
DRM_RESULT checksum2__(struct_DRM_state* pDRMState,
			     BYTE* pbPMKey,
			     LENGTH_TYPE cbPMKey,
			     BYTE* pLicData,
			     LENGTH_TYPE licDataLen,	// 实际上固定为64
			     BYTE* pbCheckSum)
{
	COUNT_TYPE i = 0;
	LENGTH_TYPE temp = 0;
	UWORD32 licData[16];
	BYTE *pb1 = pLicData;
	BYTE *pb2 = (BYTE*)licData;
	UWORD32 rgdwKeys[2];
	UWORD32 *pw1;
	UWORD32 *pw2;
	
	
	if (pbPMKey == NULL || cbPMKey == 0 || pLicData == NULL ||
		licDataLen == 0 || pbCheckSum == NULL || licDataLen > 64)
	{
		return (1);
	}
//	memset((BYTE*)&pDRMState->mem2.mem4, 0, 0x60);
	for (i = 0; i < 24; ++i)
	{
		pDRMState->mem2.mem4[i] = 0;
	}
	rc4_key__(&pDRMState->mem2.m_rc4ks, cbPMKey, pbPMKey);
	rc4__(&pDRMState->mem2.m_rc4ks, 0x60, (BYTE*)&pDRMState->mem2.mem4);
	for (i = 0; i < __CB_DECL(licDataLen); ++i)
	{
		*pb2++ = *pb1++;
	}
	temp = ((licDataLen+7) >> 3) << 3;
	/*
	for (; i < temp; ++i)
	{
		pb1 = pb2 - 1;
		*pb2++ = (BYTE)(*pb1 + 0xD7);
	}
	*/
	temp >>= 2;
	pw1 = (UWORD32*)&pDRMState->mem2.m_mackey;
	pw2 = (UWORD32*)&pDRMState->mem2.mem4;
	for (i = 0; i < 12; ++i)
	{
		*pw1++ = *pw2++ | 1;
	}
	/*
	pDRMState->mem2.m_mackey.a1 = pDRMState->mem2.mem4[0] | 1;
	pDRMState->mem2.m_mackey.b1 = pDRMState->mem2.mem4[1] | 1;
	pDRMState->mem2.m_mackey.c1 = pDRMState->mem2.mem4[2] | 1;
	pDRMState->mem2.m_mackey.d1 = pDRMState->mem2.mem4[3] | 1;
	pDRMState->mem2.m_mackey.e1 = pDRMState->mem2.mem4[4] | 1;
	pDRMState->mem2.m_mackey.f1 = pDRMState->mem2.mem4[5] | 1;
	pDRMState->mem2.m_mackey.a2 = pDRMState->mem2.mem4[6] | 1;
	pDRMState->mem2.m_mackey.b2 = pDRMState->mem2.mem4[7] | 1;
	pDRMState->mem2.m_mackey.c2 = pDRMState->mem2.mem4[8] | 1;
	pDRMState->mem2.m_mackey.d2 = pDRMState->mem2.mem4[9] | 1;
	pDRMState->mem2.m_mackey.e2 = pDRMState->mem2.mem4[10] | 1;
	pDRMState->mem2.m_mackey.f2 = pDRMState->mem2.mem4[11] | 1;
	*/
	CBC64WS4_asm__(licData, temp, rgdwKeys, &pDRMState->mem2.m_mackey);
    MEMCPY(pbCheckSum, (BYTE*)&rgdwKeys, 8);
    
    pw1 = (UWORD32*)&pDRMState->mem2.m_mackey;
	pw2 = (UWORD32*)&pDRMState->mem2.mem4 + 13;
	*pw1++ = pDRMState->mem2.mem4[0] + 0xc | 1;
	for (i = 0; i < 11; ++i)
	{
		*pw1++ = *pw2++ | 1;
	}
	CBC64WS4_asm__(licData, temp, rgdwKeys, &pDRMState->mem2.m_mackey);
    MEMCPY(pbCheckSum + 8, (BYTE*)&rgdwKeys, 8);
    return(0);
}
#pragma arm section code

#endif

#endif
#endif
#endif
