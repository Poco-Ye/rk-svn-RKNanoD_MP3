/*
************************************************************************************************************************
*
*  Copyright (C),2009, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     audio_table_room.c
*  Author:         HuWeiGuo
*  Description:
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*                        09/01/14       v1.0
*
************************************************************************************************************************
*/
#include "../include/audio_main.h"
#include "..\wmaInclude\audio_table_room.h"
#include "..\wmaInclude\DecTables.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#ifdef WMAINITIALIZE
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"
#include "..\wmaInclude\wma_table_rom.h"
// wma
unsigned char *p_getMask                                     ;
unsigned char *p_CLSID_CAsfHeaderObjectV0                    ;
unsigned char *p_CLSID_CAsfPropertiesObjectV2                ;
unsigned char *p_CLSID_CAsfStreamPropertiesObjectV1          ;
unsigned char *p_CLSID_CAsfContentDescriptionObjectV0        ;
unsigned char *p_CLSID_AsfXAcmAudioErrorMaskingStrategy      ;
unsigned char *p_CLSID_AsfXSignatureAudioErrorMaskingStrategy;
unsigned char *p_CLSID_AsfXStreamTypeAcmAudio                ;
unsigned char *p_CLSID_CAsfContentEncryptionObject           ;
unsigned char *p_CLSID_CAsfExtendedContentDescObject         ;
unsigned char *p_CLSID_CAsfMarkerObjectV0                    ;
unsigned char *p_CLSID_CAsfLicenseStoreObject                ;
unsigned char *p_CLSID_CAsfStreamPropertiesObjectV2          ;
unsigned char *p_CLSID_CAsfExtendedStreamPropertiesObject    ;
unsigned char *p_CLSID_CAsfClockObjectV0                     ;
unsigned char *p_CLSID_AsfXMetaDataObject                    ;
unsigned char *p_CLSID_CAsfPacketClock1                      ;
unsigned char *p_g_rgiHuffDecTblMsk                          ;
unsigned char *p_g_rgiHuffDecTblNoisePower                   ;
unsigned char *p_g_rgiHuffDecTbl16smOb                       ;
unsigned char *p_g_rgiHuffDecTbl44smOb                       ;
unsigned char *p_g_rgiHuffDecTbl16ssOb                       ;
unsigned char *p_g_rgiHuffDecTbl44ssOb                       ;
unsigned char *p_g_rgiHuffDecTbl44smQb                       ;
unsigned char *p_g_rgiHuffDecTbl44ssQb                       ;
unsigned char *p_rgiMaskMinusPower10                         ;
unsigned char *p_rgiMaskPlusPower10                          ;
unsigned char *p_g_rgiBarkFreqV2                             ;
unsigned char *p_gRun16smOb                                  ;
unsigned char *p_gLevel16smOb                                ;
unsigned char *p_gRun16ssOb                                  ;
unsigned char *p_gLevel16ssOb                                ;
unsigned char *p_gRun44smOb                                  ;
unsigned char *p_gLevel44smOb                                ;
unsigned char *p_gRun44ssOb                                  ;
unsigned char *p_gLevel44ssOb                                ;
unsigned char *p_gRun44smQb                                  ;
unsigned char *p_gLevel44smQb                                ;
unsigned char *p_gRun44ssQb                                  ;
unsigned char *p_gLevel44ssQb                                ;
unsigned char *p_g_InvQuadRootFraction                       ;
unsigned char *p_g_InvQuadRootExponent                       ;
unsigned char *p_g_InverseFraction                           ;
unsigned char *p_g_SqrtFraction                              ;
#if 1
unsigned char *p_g_sct64                                     ;
unsigned char *p_g_sct128                                    ;
unsigned char *p_g_sct256                                    ;
unsigned char *p_g_sct512                                    ;
unsigned char *p_g_sct1024                                   ;
unsigned char *p_g_sct2048                                   ;
#endif
unsigned char *p_rgDBPower10                                 ;
unsigned char *p_icosPIbynp                                  ;
unsigned char *p_isinPIbynp                                  ;
unsigned char *p_g_rgiLsfReconLevel                          ;
unsigned char *p_gLZLTable                                   ;

// drm9
unsigned char *p__DRM_Sel                                    ;
unsigned char *p__DRM_Spbox                                  ;

#if 0
// flac
unsigned char *p_byte_to_unary_table                         ;
unsigned char *p_FLAC__crc8_table                            ;
unsigned char *p_FLAC__crc16_table                           ;

// ape
unsigned char *p_Ape_gtPowersOfTwoMinusOne                   ;
unsigned char *p_Ape_gtKSumMinBoundary                       ;
unsigned char *p_Ape_gtRangeTotalOne                         ;
unsigned char *p_Ape_gtRangeWidthOne                         ;
unsigned char *p_Ape_gtRangeTotalTwo                         ;
unsigned char *p_Ape_gtRangeWidthTwo                         ;
unsigned char *p_Ape_gtCRC32                                 ;
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WMA_TABLE_ROOM_VERIFY

#ifndef rk_nano_board



#endif//rk_nano_board
#pragma arm section code = "WmaOpenCodecCode"
//const SinCosTable * const rgSinCosTables[SINCOSTABLE_ENTRIES] = {&g_sct64, &g_sct128, &g_sct256,
//        NULL, &g_sct512, NULL, NULL, NULL, &g_sct1024, NULL, NULL, NULL, NULL, NULL,
//        NULL, NULL, &g_sct2048};

void rgSinCosTablesInit(unsigned char* pTableBase)
{
	rgSinCosTables[0] = (SinCosTable *)(pTableBase + TBL_OFFSET_g_sct64);
	rgSinCosTables[1] = (SinCosTable *)(pTableBase + TBL_OFFSET_g_sct128);
	rgSinCosTables[2] = (SinCosTable *)(pTableBase + TBL_OFFSET_g_sct256);
	
	rgSinCosTables[3] = NULL;
	
	rgSinCosTables[4] = (SinCosTable *)(pTableBase + TBL_OFFSET_g_sct512);

    rgSinCosTables[5] = NULL;
	rgSinCosTables[6] = NULL;
	rgSinCosTables[7] = NULL;
	
	rgSinCosTables[8] = (SinCosTable *)(pTableBase + TBL_OFFSET_g_sct1024);

    rgSinCosTables[9] = NULL;
	rgSinCosTables[10] = NULL;
	rgSinCosTables[11] = NULL;
    rgSinCosTables[12] = NULL;
	rgSinCosTables[13] = NULL;
	rgSinCosTables[14] = NULL;
    rgSinCosTables[15] = NULL;
	
	rgSinCosTables[16] = (SinCosTable *)(pTableBase + TBL_OFFSET_g_sct2048);
}
// 初始化wma table 指针
extern unsigned  char wma_nanob[];
#define NANO_C

void wma_table_room_init(void)
{
    unsigned char *pTableBase;
#ifdef rk_nano_board

#ifdef NANOC_DECODE
    pTableBase = (unsigned char *)WMA_TAB_BASE_ADDR;
 //  pTableBase = (unsigned char *)wma_nanob;
#else
    pTableBase = (unsigned char *)WMA_TABLE_ROM;//lcs add wma table rom
#endif

#else
    //pTableBase = (unsigned char *)gTableBuf;
	pTableBase = (unsigned char *)WMA_TABLE_ROM;//lcs add wma table rom

#endif
#ifdef NANO_C
		p_getMask									   = pTableBase + TBL_OFFSET_getMask									 ;	  
		p_CLSID_CAsfHeaderObjectV0					   = pTableBase + TBL_OFFSET_CLSID_CAsfHeaderObjectV0					 ;	  
		p_CLSID_CAsfPropertiesObjectV2				   = pTableBase + TBL_OFFSET_CLSID_CAsfPropertiesObjectV2				 ;	  
		p_CLSID_CAsfStreamPropertiesObjectV1		   = pTableBase + TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV1 		 ;	  
		p_CLSID_CAsfContentDescriptionObjectV0		   = pTableBase + TBL_OFFSET_CLSID_CAsfContentDescriptionObjectV0		 ;	  
		p_CLSID_AsfXAcmAudioErrorMaskingStrategy	   = pTableBase + TBL_OFFSET_CLSID_AsfXAcmAudioErrorMaskingStrategy 	 ;	  
		p_CLSID_AsfXSignatureAudioErrorMaskingStrategy = pTableBase + TBL_OFFSET_CLSID_AsfXSignatureAudioErrorMaskingStrategy;	  
		p_CLSID_AsfXStreamTypeAcmAudio				   = pTableBase + TBL_OFFSET_CLSID_AsfXStreamTypeAcmAudio				 ;	  
		p_CLSID_CAsfContentEncryptionObject 		   = pTableBase + TBL_OFFSET_CLSID_CAsfContentEncryptionObject			 ;	  
		p_CLSID_CAsfExtendedContentDescObject		   = pTableBase + TBL_OFFSET_CLSID_CAsfExtendedContentDescObject		 ;	  
		p_CLSID_CAsfMarkerObjectV0					   = pTableBase + TBL_OFFSET_CLSID_CAsfMarkerObjectV0					 ;	  
		p_CLSID_CAsfLicenseStoreObject				   = pTableBase + TBL_OFFSET_CLSID_CAsfLicenseStoreObject				 ;	  
		p_CLSID_CAsfStreamPropertiesObjectV2		   = pTableBase + TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV2 		 ;	  
		p_CLSID_CAsfExtendedStreamPropertiesObject	   = pTableBase + TBL_OFFSET_CLSID_CAsfExtendedStreamPropertiesObject	 ;	  
		p_CLSID_CAsfClockObjectV0					   = pTableBase + TBL_OFFSET_CLSID_CAsfClockObjectV0					 ;	  
		p_CLSID_AsfXMetaDataObject					   = pTableBase + TBL_OFFSET_CLSID_AsfXMetaDataObject					 ;	  
		p_CLSID_CAsfPacketClock1					   = pTableBase + TBL_OFFSET_CLSID_CAsfPacketClock1 					 ;	  
		p_g_rgiHuffDecTblMsk						   = pTableBase + TBL_OFFSET_g_rgiHuffDecTblMsk 						 ;	  
		p_g_rgiHuffDecTblNoisePower 				   = pTableBase + TBL_OFFSET_g_rgiHuffDecTblNoisePower					 ;	  
		//p_g_rgiHuffDecTbl16smOb						 = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl16smOb					   ;	
		p_g_rgiHuffDecTbl44smOb 					   = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44smOb						 ;	  
		//p_g_rgiHuffDecTbl16ssOb						 = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl16ssOb					   ;	
		//p_g_rgiHuffDecTbl44ssOb						 = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44ssOb					   ;	
		//p_g_rgiHuffDecTbl44smQb						 = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44smQb					   ;	
		//p_g_rgiHuffDecTbl44ssQb						 = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44ssQb					   ;	
		p_rgiMaskMinusPower10						   = pTableBase + TBL_OFFSET_rgiMaskMinusPower10						 ;	  
		p_rgiMaskPlusPower10						   = pTableBase + TBL_OFFSET_rgiMaskPlusPower10 						 ;	  
		p_g_rgiBarkFreqV2							   = pTableBase + TBL_OFFSET_g_rgiBarkFreqV2							 ;	  
		//p_gRun16smOb									 = pTableBase + TBL_OFFSET_gRun16smOb								   ;	
		//p_gLevel16smOb								 = pTableBase + TBL_OFFSET_gLevel16smOb 							   ;	
		p_gRun16ssOb								   = pTableBase + TBL_OFFSET_gRun16ssOb 								 ;	  
		p_gLevel16ssOb								   = pTableBase + TBL_OFFSET_gLevel16ssOb								 ;	  
		p_gRun44smOb								   = pTableBase + TBL_OFFSET_gRun44smOb 								 ;	  
		p_gLevel44smOb								   = pTableBase + TBL_OFFSET_gLevel44smOb								 ;	  
		p_gRun44ssOb								   = pTableBase + TBL_OFFSET_gRun44ssOb 								 ;	  
		p_gLevel44ssOb								   = pTableBase + TBL_OFFSET_gLevel44ssOb								 ;	  
		p_gRun44smQb								   = pTableBase + TBL_OFFSET_gRun44smQb 								 ;	  
		p_gLevel44smQb								   = pTableBase + TBL_OFFSET_gLevel44smQb								 ;	  
		p_gRun44ssQb								   = pTableBase + TBL_OFFSET_gRun44ssQb 								 ;	  
		p_gLevel44ssQb								   = pTableBase + TBL_OFFSET_gLevel44ssQb								 ;	  
		p_g_InvQuadRootFraction 					   = pTableBase + TBL_OFFSET_g_InvQuadRootFraction						 ;	  
		p_g_InvQuadRootExponent 					   = pTableBase + TBL_OFFSET_g_InvQuadRootExponent						 ;
		p_g_InverseFraction 						   = pTableBase + TBL_OFFSET_g_InverseFraction							 ;
		p_g_SqrtFraction							   = pTableBase + TBL_OFFSET_g_SqrtFraction 							 ;
		p_g_sct64									   = pTableBase + TBL_OFFSET_g_sct64									 ;
		p_g_sct128									   = pTableBase + TBL_OFFSET_g_sct128									 ;
		p_g_sct256									   = pTableBase + TBL_OFFSET_g_sct256									 ;
		p_g_sct512									   = pTableBase + TBL_OFFSET_g_sct512									 ;
		p_g_sct1024 								   = pTableBase + TBL_OFFSET_g_sct1024									 ;
		p_g_sct2048 								   = pTableBase + TBL_OFFSET_g_sct2048									 ;
		p_rgDBPower10								   = pTableBase + TBL_OFFSET_rgDBPower10								 ;
		p_icosPIbynp								   = pTableBase + TBL_OFFSET_icosPIbynp 								 ;
		p_isinPIbynp								   = pTableBase + TBL_OFFSET_isinPIbynp 								 ;
		p_g_rgiLsfReconLevel						   = pTableBase + TBL_OFFSET_g_rgiLsfReconLevel 						 ;
		p_gLZLTable 								   = pTableBase + TBL_OFFSET_gLZLTable									 ;	
#else
		p_getMask									   = pTableBase + TBL_OFFSET_getMask									 ;	  
		p_CLSID_CAsfHeaderObjectV0					   = pTableBase + TBL_OFFSET_CLSID_CAsfHeaderObjectV0					 ;	  
		p_CLSID_CAsfPropertiesObjectV2				   = pTableBase + TBL_OFFSET_CLSID_CAsfPropertiesObjectV2				 ;	  
		p_CLSID_CAsfStreamPropertiesObjectV1		   = pTableBase + TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV1 		 ;	  
		p_CLSID_CAsfContentDescriptionObjectV0		   = pTableBase + TBL_OFFSET_CLSID_CAsfContentDescriptionObjectV0		 ;	  
		p_CLSID_AsfXAcmAudioErrorMaskingStrategy	   = pTableBase + TBL_OFFSET_CLSID_AsfXAcmAudioErrorMaskingStrategy 	 ;	  
		p_CLSID_AsfXSignatureAudioErrorMaskingStrategy = pTableBase + TBL_OFFSET_CLSID_AsfXSignatureAudioErrorMaskingStrategy;	  
		p_CLSID_AsfXStreamTypeAcmAudio				   = pTableBase + TBL_OFFSET_CLSID_AsfXStreamTypeAcmAudio				 ;	  
		p_CLSID_CAsfContentEncryptionObject 		   = pTableBase + TBL_OFFSET_CLSID_CAsfContentEncryptionObject			 ;	  
		p_CLSID_CAsfExtendedContentDescObject		   = pTableBase + TBL_OFFSET_CLSID_CAsfExtendedContentDescObject		 ;	  
		p_CLSID_CAsfMarkerObjectV0					   = pTableBase + TBL_OFFSET_CLSID_CAsfMarkerObjectV0					 ;	  
		p_CLSID_CAsfLicenseStoreObject				   = pTableBase + TBL_OFFSET_CLSID_CAsfLicenseStoreObject				 ;	  
		p_CLSID_CAsfStreamPropertiesObjectV2		   = pTableBase + TBL_OFFSET_CLSID_CAsfStreamPropertiesObjectV2 		 ;	  
		p_CLSID_CAsfExtendedStreamPropertiesObject	   = pTableBase + TBL_OFFSET_CLSID_CAsfExtendedStreamPropertiesObject	 ;	  
		p_CLSID_CAsfClockObjectV0					   = pTableBase + TBL_OFFSET_CLSID_CAsfClockObjectV0					 ;	  
		p_CLSID_AsfXMetaDataObject					   = pTableBase + TBL_OFFSET_CLSID_AsfXMetaDataObject					 ;	  
		p_CLSID_CAsfPacketClock1					   = pTableBase + TBL_OFFSET_CLSID_CAsfPacketClock1 					 ;	  
		p_g_rgiHuffDecTblMsk						   = pTableBase + TBL_OFFSET_g_rgiHuffDecTblMsk 						 ;	  
		p_g_rgiHuffDecTblNoisePower 				   = pTableBase + TBL_OFFSET_g_rgiHuffDecTblNoisePower					 ;	  
		p_g_rgiHuffDecTbl16smOb 					   = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl16smOb						 ;	  
		p_g_rgiHuffDecTbl44smOb 					   = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44smOb						 ;	  
		p_g_rgiHuffDecTbl16ssOb 					   = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl16ssOb						 ;	  
		p_g_rgiHuffDecTbl44ssOb 					   = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44ssOb						 ;	  
		p_g_rgiHuffDecTbl44smQb 					   = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44smQb						 ;	  
		p_g_rgiHuffDecTbl44ssQb 					   = pTableBase + TBL_OFFSET_g_rgiHuffDecTbl44ssQb						 ;	  
		p_rgiMaskMinusPower10						   = pTableBase + TBL_OFFSET_rgiMaskMinusPower10						 ;	  
		p_rgiMaskPlusPower10						   = pTableBase + TBL_OFFSET_rgiMaskPlusPower10 						 ;	  
		p_g_rgiBarkFreqV2							   = pTableBase + TBL_OFFSET_g_rgiBarkFreqV2							 ;	  
		p_gRun16smOb								   = pTableBase + TBL_OFFSET_gRun16smOb 								 ;	  
		p_gLevel16smOb								   = pTableBase + TBL_OFFSET_gLevel16smOb								 ;	  
		p_gRun16ssOb								   = pTableBase + TBL_OFFSET_gRun16ssOb 								 ;	  
		p_gLevel16ssOb								   = pTableBase + TBL_OFFSET_gLevel16ssOb								 ;	  
		p_gRun44smOb								   = pTableBase + TBL_OFFSET_gRun44smOb 								 ;	  
		p_gLevel44smOb								   = pTableBase + TBL_OFFSET_gLevel44smOb								 ;	  
		p_gRun44ssOb								   = pTableBase + TBL_OFFSET_gRun44ssOb 								 ;	  
		p_gLevel44ssOb								   = pTableBase + TBL_OFFSET_gLevel44ssOb								 ;	  
		p_gRun44smQb								   = pTableBase + TBL_OFFSET_gRun44smQb 								 ;	  
		p_gLevel44smQb								   = pTableBase + TBL_OFFSET_gLevel44smQb								 ;	  
		p_gRun44ssQb								   = pTableBase + TBL_OFFSET_gRun44ssQb 								 ;	  
		p_gLevel44ssQb								   = pTableBase + TBL_OFFSET_gLevel44ssQb								 ;	  
		p_g_InvQuadRootFraction 					   = pTableBase + TBL_OFFSET_g_InvQuadRootFraction						 ;	  
		p_g_InvQuadRootExponent 					   = pTableBase + TBL_OFFSET_g_InvQuadRootExponent						 ;
		p_g_InverseFraction 						   = pTableBase + TBL_OFFSET_g_InverseFraction							 ;
		p_g_SqrtFraction							   = pTableBase + TBL_OFFSET_g_SqrtFraction 							 ;
		p_g_sct64									   = pTableBase + TBL_OFFSET_g_sct64									 ;
		p_g_sct128									   = pTableBase + TBL_OFFSET_g_sct128									 ;
		p_g_sct256									   = pTableBase + TBL_OFFSET_g_sct256									 ;
		p_g_sct512									   = pTableBase + TBL_OFFSET_g_sct512									 ;
		p_g_sct1024 								   = pTableBase + TBL_OFFSET_g_sct1024									 ;
		p_g_sct2048 								   = pTableBase + TBL_OFFSET_g_sct2048									 ;
		p_rgDBPower10								   = pTableBase + TBL_OFFSET_rgDBPower10								 ;
		p_icosPIbynp								   = pTableBase + TBL_OFFSET_icosPIbynp 								 ;
		p_isinPIbynp								   = pTableBase + TBL_OFFSET_isinPIbynp 								 ;
		p_g_rgiLsfReconLevel						   = pTableBase + TBL_OFFSET_g_rgiLsfReconLevel 						 ;
		p_gLZLTable 								   = pTableBase + TBL_OFFSET_gLZLTable									 ;	
#endif


	//rgSinCosTables initialization
	rgSinCosTablesInit(pTableBase);
}
#endif
#if !defined(WMAAPI_NO_DRM) || defined(WMA_TABLE_ROOM_VERIFY)
// 初始化drm9 table 指针
void drm9_table_room_init(void)
{
#ifndef NANO_C
		unsigned char *pTableBase;
		
		p__DRM_Sel									   = pTableBase + TBL_OFFSET__DRM_Sel;		
		p__DRM_Spbox								   = pTableBase + TBL_OFFSET__DRM_Spbox;	
#endif

}
#endif

#if 0
// 初始化flac table 指针
void flac_table_room_init(void)
{
	unsigned char *pTableBase;
#ifdef rk_nano_board
    pTableBase = (unsigned char *)WMA_TAB_BASE_ADDR;
#else
    pTableBase = (unsigned char *)gTableBuf;
#endif

    p_byte_to_unary_table                          = pTableBase + TBL_OFFSET_byte_to_unary_table;
    p_FLAC__crc8_table                             = pTableBase + TBL_OFFSET_FLAC__crc8_table   ;
    p_FLAC__crc16_table                            = pTableBase + TBL_OFFSET_FLAC__crc16_table  ;
}
// 初始化ape table 指针
void ape_table_room_init(void)
{
	unsigned char *pTableBase;
#ifdef rk_nano_board
    pTableBase = (unsigned char *)WMA_TAB_BASE_ADDR;
#else
    pTableBase = (unsigned char *)gTableBuf;
#endif

    p_Ape_gtPowersOfTwoMinusOne                    = pTableBase + TBL_OFFSET_Ape_gtPowersOfTwoMinusOne;
    p_Ape_gtKSumMinBoundary                        = pTableBase + TBL_OFFSET_Ape_gtKSumMinBoundary    ;
    p_Ape_gtRangeTotalOne                          = pTableBase + TBL_OFFSET_Ape_gtRangeTotalOne      ;
    p_Ape_gtRangeWidthOne                          = pTableBase + TBL_OFFSET_Ape_gtRangeWidthOne      ;
    p_Ape_gtRangeTotalTwo                          = pTableBase + TBL_OFFSET_Ape_gtRangeTotalTwo      ;
    p_Ape_gtRangeWidthTwo                          = pTableBase + TBL_OFFSET_Ape_gtRangeWidthTwo      ;
    p_Ape_gtCRC32                                  = pTableBase + TBL_OFFSET_Ape_gtCRC32              ;
}
#endif

#pragma arm section code

#endif
#endif
#endif
