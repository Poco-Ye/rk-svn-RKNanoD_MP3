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
#include "..\wmaInclude\audio_table_room.h"
#ifdef WMA_DEC_INCLUDE
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"

#define assert(x)
//UWORD32 *g_pStack__;


/*___________________________________________________________________________
 |                                                                           |
 |   blockbox                                                                |
 |___________________________________________________________________________|
*/

/**************************************************
 * DRMInit
 **************************************************
 */
#ifdef WMAINITIALIZE 
#pragma arm section code = "WmaOpenCodecCode"

void DRMInit__(CIPHER_CONTEXT* pin)
{
	pin->m_fInited = 0;
	pin->m_fDecryptInited = 0;
//	g_pStack__ = pin->mem4;
}

#pragma arm section code
#endif
/**************************************************
 * DRMDecrypt
 **************************************************
 */
#ifdef WMAHIGHRATE
#pragma arm section code = "WmaHighRateCode"


DRM_RESULT DRMDecrypt__(CIPHER_CONTEXT* pin, LENGTH_TYPE cbBuffer, BYTE* pbBuffer)
{
	DRM_RESULT dr = DRM_SUCCESS;
	COUNT_TYPE imac_start        = 0, 
            imac_end          = 0, 
            segment_end       = 0, 
            imac_in_seg_start = 0, 
            imac_in_seg_end   = 0;
            
    if(pin->m_iPacket + cbBuffer > pin->m_cbPacket)
    {
    	dr = 1;
    	goto ErrorExit;
    }
    
    if(cbBuffer == 0)
    {
    	goto ErrorExit;
    }
    
    if(!pin->m_fInited || !pin->m_fDecryptInited)
    {
    	dr = DRM_NOTINITIALIZED;	// 3
    	goto ErrorExit;
    }
    
    /*
    **  small packet case: MAC does not handle it
    */
    if ( pin->m_cbPacket < 16 )
    {
        COUNT_TYPE iData = 0;
        for ( iData = 0; iData < cbBuffer; iData++)
        {
//            pbBuffer[iData] ^= pin->m_shaOut[iData + pin->m_iPacket];
            DRM_BYTE bSHA = GET_BYTE( pin->m_shaOut,
                                      iData + pin->m_iPacket );
            DRM_BYTE bData = GET_BYTE( pbBuffer, iData);

            PUT_BYTE( pbBuffer, iData, bData ^ bSHA);
        }       
        pin->m_iPacket += cbBuffer;
        goto ErrorExit;
    }
    
    /*
    **  RC4 decrypt the content
    */
    rc4__(&pin->m_rc4ks, cbBuffer, pbBuffer);
    
    imac_end    = (pin->m_cbPacket / 8) * 8;
    imac_start  = imac_end - 8;
    segment_end = pin->m_iPacket + cbBuffer;
    
 /*   
    if ( segment_end > imac_start ) 
    {
        /* NOTE:  To remove complexity we do not handle the case where
           a packet is split somewhere in the middle of the last 15 bytes */
        //DRMASSERT( segment_end == pin->m_cbPacket );

        /* Set the last 8 bytes correctly */
        /*DRM_BYT_CopyBytes( f_pbData,
                           imac_start - pin->m_iPacket,
                           (DRM_BYTE*)pin->m_rguiLast8, 
                           0, 
                           2 * SIZEOF(DRM_UINT) );
    }*/
    
    
    if ( pin->m_iPacket < imac_start ) 
    {
    	if ( pin->m_iPacket + cbBuffer >= imac_start ) 
        {
            UWORD32 mac1 = 0;
            UWORD32 mac2 = 0;
            UWORD32 macInverse1 = 0;
            UWORD32 macInverse2 = 0;

            /*
            **  First update MAC with data from this segment
            */            
            CBC64Update__( &pin->m_mackey, 
                         &pin->m_cbcstate, 
                         imac_start - pin->m_iPacket, 
                         pbBuffer );
            
            /*
            **  Finalize MAC to decipher last 8 bytes of encrypted data 
            */
            mac1 = CBC64Finalize__( &pin->m_mackey, 
                                  &pin->m_cbcstate, 
                                  &mac2 );
            macInverse2 = CBC64Invert__( &pin->m_mackey, 
                                       &pin->m_invmackey, 
                                       mac1, 
                                       mac2, 
                                       pin->m_rc4key[0], 
                                       pin->m_rc4key[1], 
                                       &macInverse1 );
            pin->m_rc4key[0] = macInverse1; 
            pin->m_rc4key[1] = macInverse2;
        }
        else 
        {
            /*
            **  Update MAC with data from this segment
            */             
            CBC64Update__( &pin->m_mackey, 
                         &pin->m_cbcstate, 
                         cbBuffer, 
                         pbBuffer );
        }
    }
    
    if ( (pin->m_iPacket < imac_end)
    	&& (segment_end > imac_start) ) 
    {
    	
        COUNT_TYPE iData = 0;
        /*
        **  Insert last 8 bytes of data deciphered
        */ 
        /* 由于是Little Endian，下面的程序被省掉 */  /*   
        BYTE  rgbMac[8];
        
        DWORD_TO_BYTES( rgbMac,                pin->m_rc4key[0] );
        DWORD_TO_BYTES( rgbMac + 4, pin->m_rc4key[1] );*/
        

        imac_in_seg_start = (imac_start >= pin->m_iPacket) ? 
                                            imac_start : pin->m_iPacket;
        imac_in_seg_end = (imac_end <= segment_end) ? imac_end:segment_end;
        
        for ( iData = imac_in_seg_start; iData < imac_in_seg_end; iData++ ) 
        {
            PUT_BYTE( pbBuffer, 
                      iData - pin->m_iPacket, 
                      // GET_BYTE( rgbMac, iData - imac_start ) );
                      GET_BYTE( (BYTE*)&pin->m_rc4key[0], iData - imac_start ) );//carefull
        }
    }
    
    pin->m_iPacket += cbBuffer;
    if ( pin->m_iPacket >= pin->m_cbPacket )
    {
        pin->m_fDecryptInited = 0;
    }

ErrorExit:
    return dr;  
}


/**************************************************
 * DRMInitPacket
 **************************************************
 */
 
DRM_RESULT DRMInitPacket__(CIPHER_CONTEXT* pin, UWORD16 cbPayloadSize, BYTE* pLast15)
{
    BYTE   *plast;
    UWORD32 pTemp[2];
    BYTE   *rgbIn;
    BYTE   *rgbOut;
    
    if (cbPayloadSize == 0)
        return 1;
    if (pin->m_fInited == 0)
        return 3;
    
    CBC64InitState__(&pin->m_cbcstate);
    
    pin->m_iPacket = 0;
    pin->m_cbPacket = cbPayloadSize;
    pin->m_fDecryptInited = 1;
    
    if (cbPayloadSize < 0x10)
        return 0;
    
    plast = pLast15 + (7 - (cbPayloadSize & 7));
    pTemp[0] = AllignedDWORD__(plast);    
    pTemp[0] ^= (pin->m_desS1[0]);
    
    plast += 4;
    pTemp[1] = AllignedDWORD__(plast);
    pTemp[1] ^= (pin->m_desS1[1]);
    
    rgbIn = (BYTE*) &pin->m_rc4key;
    rgbOut = (BYTE*) pTemp;
    des__(rgbIn, rgbOut, &pin->m_destable, 0);
    
    pin->m_rc4key[0] ^= pin->m_desS2[0];
    pin->m_rc4key[1] ^= pin->m_desS2[1];
    
    rc4_key__(&pin->m_rc4ks, 8, (BYTE*)&pin->m_rc4key );
    
    return 0;               
}
#pragma arm section code 

#endif

/**************************************************
 * AllignedDWORD
 **************************************************
 */
#ifdef  WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

UWORD32 AllignedDWORD__(BYTE* pin)
{
	UWORD32 temp = 0;
/*
	BYTE *p1 = (BYTE*)&temp;
	
	*p1++ = *pin++;
	*p1++ = *pin++;
	*p1++ = *pin++;
	*p1++ = *pin++;
*/
	temp = *pin | (*(pin+1) << 8) | (*(pin+2) << 16) | (*(pin+3) << 24);
	return(temp);
}
#pragma arm section code

#endif

/**************************************************
 * DRMKeySetup
 **************************************************
 */
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

DRM_RESULT DRMKeySetup__(CIPHER_CONTEXT* pContext, BYTE cbData, BYTE* pbData)
{
	DRM_RESULT dr = 0;
	COUNT_TYPE i = 0;
	UWORD32 keybuf[16];
	
	if ((pbData != NULL) && (cbData != 0))
	{
		A_SHAInit__(&pContext->m_shadata);
		A_SHAUpdate__(&pContext->m_shadata, pbData, cbData);
		A_SHAFinal__(&pContext->m_shadata, pContext->m_shaOut);
		deskey__(&pContext->m_destable, pContext->m_shaOut + 12);
//		memset((BYTE*)keybuf, 0, 64);
		for (i = 0; i < 16; ++i)
		{
			keybuf[i] = 0;
		}
		rc4_key__(&pContext->m_rc4ks, 12, pContext->m_shaOut);
		rc4__(&pContext->m_rc4ks, 64, (BYTE*)keybuf);
		CBC64Init__(&pContext->m_mackey, &pContext->m_cbcstate, keybuf);
		CBC64InvKey__(&pContext->m_mackey, &pContext->m_invmackey);
		pContext->m_desS1[0] = keybuf[14]; 
    	pContext->m_desS1[1] = keybuf[15];
    	pContext->m_desS2[0] = keybuf[12]; 
    	pContext->m_desS2[1] = keybuf[13];
    	pContext->m_fInited = 1;
	}
	else
	{
		dr = 1;
	}
	return (dr);
}	// 完成

/*___________________________________________________________________________
 |                                                                           |
 |   cbc64ws4                                                                |
 |___________________________________________________________________________|
*/

/**************************************************
 * CBC64WS4_asm
 **************************************************
 */

void CBC64WS4_asm__(UWORD32 *pbData,
				  LENGTH_TYPE cBlocks,
				  UWORD32 rgdwKeys[2],
				  DRM_CBCKey *pCBCKey)
{
	rgdwKeys[0] = rgdwKeys[1] = 0;
    while ( cBlocks > 0)
    {
		MP_C_STEP(*pbData, pCBCKey->a1, pCBCKey->b1, pCBCKey->c1, pCBCKey->d1, pCBCKey->e1, pCBCKey->f1, rgdwKeys[1], rgdwKeys[0]);
        pbData++;
        MP_C_STEP(*pbData, pCBCKey->a2, pCBCKey->b2, pCBCKey->c2, pCBCKey->d2, pCBCKey->e2, pCBCKey->f2, rgdwKeys[1], rgdwKeys[0]);
        pbData++;
        cBlocks -= 2;    
    }
}


/**************************************************
 * CBC64Init
 **************************************************
 */
 
void CBC64Init__( DRM_CBCKey *cbckey, DRM_CBCState *cbcstate, UWORD32 *pKey ) 
{
  	UWORD32 *p = pKey;
  	
  	cbcstate->sum = 0; cbcstate->t = 0; cbcstate->dwBufLen = 0;
//  	p = (UWORD32 *)pKey;
  	cbckey->a1 = *p++ | 0x00000001;
  	cbckey->b1 = *p++ | 0x00000001;
  	cbckey->c1 = *p++ | 0x00000001;
  	cbckey->d1 = *p++ | 0x00000001;
  	cbckey->e1 = *p++ | 0x00000001;
  	cbckey->f1 = *p++ | 0x00000001;
  	cbckey->a2 = *p++ | 0x00000001;
  	cbckey->b2 = *p++ | 0x00000001;
  	cbckey->c2 = *p++ | 0x00000001;
  	cbckey->d2 = *p++ | 0x00000001;
  	cbckey->e2 = *p++ | 0x00000001;
  	cbckey->f2 = *p++ | 0x00000001;
}




UWORD32 inv32__(UWORD32 in)
{
	COUNT_TYPE i = 0;
	UWORD32 temp;
	UWORD32 a = 0xFFFFFFFF;
	UWORD32 b = in;
	UWORD32 buf[49];

	if (b <= 1)
	{
		return (1);	
	}
	buf[i] = a / b;
	temp = a; 
	a = b;
	b = (temp % b) + 1;
	if (b == a)
	{
		b = 0;
		buf[i] += 1;
	}
	i++;
	while (b != 0)
	{
		if (i == 49)
		{
			return (0);
		}
		buf[i] = a / b;
		temp = a;
		a = b;
		b = temp % b;
		i++;
	}
	a = 1;
	b = 1;
	while (i >= 1)
	{
		i--;
		temp = a;
		a = b;
		b = temp - b * buf[i];
	}
	return (b);
}



/**************************************************
 * CBC64InvKey
 **************************************************
 */
 
void CBC64InvKey__( DRM_CBCKey *cbckey, DRM_CBCKey *cbcInvKey )
{
  	cbcInvKey->a1 = inv32__( cbckey->a1 );
  	cbcInvKey->a2 = inv32__( cbckey->a2 );
  	cbcInvKey->b1 = inv32__( cbckey->b1 );
  	cbcInvKey->b2 = inv32__( cbckey->b2 );
  	cbcInvKey->c1 = inv32__( cbckey->c1 );
  	cbcInvKey->c2 = inv32__( cbckey->c2 );
  	cbcInvKey->d1 = inv32__( cbckey->d1 );
  	cbcInvKey->d2 = inv32__( cbckey->d2 );
  	cbcInvKey->e1 = inv32__( cbckey->e1 );
  	cbcInvKey->e2 = inv32__( cbckey->e2 );
  	cbcInvKey->f1 = inv32__( cbckey->f1 );
  	cbcInvKey->f2 = inv32__( cbckey->f2 );
}
#pragma arm section code

#endif
/**************************************************
 * CBC64Update
 **************************************************
 */
#ifdef WMAHIGHRATE
#pragma arm section code = "WmaHighRateCode"


void CBC64Update__( 
    IN      DRM_CBCKey *key, 
    IN  OUT DRM_CBCState *cbcstate,
    IN      LENGTH_TYPE cbData, 
    IN      BYTE *pbData )
{
    COUNT_TYPE    iData  = 0;
    LENGTH_TYPE    cbCopy = 0;
    LENGTH_TYPE    cbTemp  = 0;
    BYTE    *pbTemp = NULL;
    
    if ( cbcstate->dwBufLen > 0 ) 
    {
        //cbCopy = min( cbData, 8 - cbcstate->dwBufLen );
        cbCopy = (cbData < (8 - cbcstate->dwBufLen)) ? 
                                            cbData : (8 - cbcstate->dwBufLen);
        
        for ( iData=0; iData < cbCopy; iData++ )
        {
            PUT_BYTE( cbcstate->buf, 
                      cbcstate->dwBufLen + iData, 
                      GET_BYTE(pbData, iData) );
        }
        
        cbcstate->dwBufLen += cbCopy;
        if ( cbcstate->dwBufLen == 8 ) 
        {
            pbTemp = cbcstate->buf;
            MP_C_STEP_P( pbTemp, key->a1, key->b1, key->c1, key->d1, key->e1, key->f1, cbcstate->t, cbcstate->sum );
            MP_C_STEP_P( pbTemp, key->a2, key->b2, key->c2, key->d2, key->e2, key->f2, cbcstate->t, cbcstate->sum );
            cbcstate->dwBufLen = 0;
        }
    }

    cbTemp = (cbData - cbCopy) / 8;
    pbTemp = pbData + __CB_DECL(cbCopy);

    while (cbTemp > 0) 
    {
        MP_C_STEP_P( pbTemp, key->a1, key->b1, key->c1, key->d1, key->e1, key->f1, cbcstate->t, cbcstate->sum );
        MP_C_STEP_P( pbTemp, key->a2, key->b2, key->c2, key->d2, key->e2, key->f2, cbcstate->t, cbcstate->sum );
        cbTemp--;
    }

    cbTemp = cbCopy + ((cbData-cbCopy) / 8) * 8;
    if ( cbTemp < cbData ) 
    {
        for ( iData=cbTemp; iData<cbData; iData++ )
        {
            PUT_BYTE( cbcstate->buf, iData - cbTemp, GET_BYTE( pbData, iData ) );
        }
        cbcstate->dwBufLen = cbData - cbTemp;
    }
}


/**************************************************
 * CBC64Finalize
 **************************************************
 */
 
UWORD32 CBC64Finalize__( DRM_CBCKey *key, DRM_CBCState *cbcstate, UWORD32 *pKey2 ) 
{
    COUNT_TYPE   i = 0;
    BYTE    *p = NULL;

    if ( cbcstate->dwBufLen > 0 ) 
    {
        for ( i=cbcstate->dwBufLen; i<8; i++ )
        {
            PUT_BYTE( cbcstate->buf, i, 0);
        }
        p = cbcstate->buf;
        MP_C_STEP_P( p, key->a1, key->b1, key->c1, key->d1, key->e1, key->f1, cbcstate->t, cbcstate->sum );
        MP_C_STEP_P( p, key->a2, key->b2, key->c2, key->d2, key->e2, key->f2, cbcstate->t, cbcstate->sum );
        cbcstate->dwBufLen = 0;
    }

    *pKey2 = cbcstate->t;
    return cbcstate->sum;
}

/**************************************************
 * CBC64Invert
 **************************************************
 */
 
UWORD32 CBC64Invert__( DRM_CBCKey *key, DRM_CBCKey *ikey, 
                      UWORD32 MacA1,   UWORD32 MacA2,
                      UWORD32 MacB1,   UWORD32 MacB2,    UWORD32 *pInvKey2 )
{
    UWORD32 tmp = 0;
    UWORD32 yn = 0, yn1 = 0, xn = 0, xn1 = 0;

    MacA1 += MacB2;
    yn = MacB2;
    yn1 = MacB1 - MacA1;

    /* last word */
    tmp = yn - key->f2;
    INV_STEP_C(ikey->a2, ikey->b2, ikey->c2, ikey->d2, ikey->e2);
    xn = tmp - yn1;

    /* next-to-last word */
    tmp = yn1 - key->f1;
    INV_STEP_C(ikey->a1, ikey->b1, ikey->c1, ikey->d1, ikey->e1);
    xn1 = tmp - MacA2;

    *pInvKey2 = (UWORD32) xn1;
    return (UWORD32) xn;
}


/**************************************************
 * CBC64InitState
 **************************************************
 */
 
void CBC64InitState__( DRM_CBCState *cbcstate ) 
{
  	cbcstate->sum = 0; 
  	cbcstate->t = 0; 
  	cbcstate->dwBufLen = 0;
}
#pragma arm section code 

#endif
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

/*___________________________________________________________________________
 |                                                                           |
 |   desport                                                                 |
 |___________________________________________________________________________|
*/

static	void F_des(DRM_DWORD* pleft,DRM_DWORD* pright,const int S,DRM_DWORD* pzork,DRM_DWORD* pwork,DRM_DWORD *pTable) 
{ 
		*pwork = pTable[S]; 
		*pzork = pTable[S+1];	
		*pwork ^= *pright; 
		*pzork ^= *pright; 
		*pzork = ROTATE_RIGHT(*pzork, 4); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + __CB_DECL((*pwork	   & 0xfc))); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + 1*64*4 + __CB_DECL((*pzork	  & 0xfc))); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + 2*64*4 + __CB_DECL(((*pwork>> 8)& 0xfc))); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + 3*64*4 + __CB_DECL(((*pzork>> 8)& 0xfc))); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + 4*64*4 + __CB_DECL(((*pwork>>16)& 0xfc))); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + 5*64*4 + __CB_DECL(((*pzork>>16)& 0xfc))); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + 6*64*4 + __CB_DECL(((*pwork>>24)& 0xfc))); 
		*pleft ^= *(DRM_DWORD *)((DRM_BYTE*)p__DRM_Spbox + 7*64*4 + __CB_DECL(((*pzork>>24)& 0xfc))); 
}

/**************************************************
 * des
 **************************************************
 */
 
void des__(
          DRM_BYTE  rgbOut[__CB_DECL(DES_BLOCKLEN)],
    const DRM_BYTE  rgbIn [__CB_DECL(DES_BLOCKLEN)],          
          DESTable *pTable,
          DRM_INT   op)
{
    DRM_DWORD left, 
              work, 
              right, 
              zork;

    /* Read input block and place in left, right */
    BYTES_TO_DWORD( right, rgbIn);
    BYTES_TO_DWORD( left,  rgbIn + __CB_DECL(4) );

    /* Hoey's clever initial permutation algorithm, from Outerbridge
     * (see Schneier p 478)
     *
     * The convention here is the same as Outerbridge: rotate each
     * register left by 1 bit, i.e., so that "left" contains permuted
     * input bits 2, 3, 4, ... 1 and "right" contains 33, 34, 35, ... 32
     * (using origin-1 numbering as in the FIPS). This allows us to avoid
     * one of the two rotates that would otherwise be required in each of
     * the 16 rounds.
     */

    right = ROTATE_LEFT(right, 4) & 0xFFFFFFFF;
    work = right;
    right ^= left;
    right &= 0xf0f0f0f0;
    work ^= right;
    left ^= right;

    left = ROTATE_LEFT(left, 20) & 0xFFFFFFFF;
    right = work;
    work ^= left;
    work &= 0xfff0000f;
    right ^= work;
    left ^= work;

    left = ROTATE_LEFT(left,14) &0xFFFFFFFF;
    work = right;
    right ^= left;
    right &= 0x33333333;
    work ^= right;
    left ^= right;

    work = ROTATE_LEFT(work, 22) & 0xFFFFFFFF;
    right = work;
    work ^= left;
    work &= 0x03fc03fc;
    right ^= work;
    left ^= work;

    right = ROTATE_LEFT(right, 9) & 0xFFFFFFFF;
    work = right;
    right ^= left;
    right &= 0xaaaaaaaa;
    work ^= right;
    left ^= right;

    left = ROTATE_LEFT(left, 1) & 0xFFFFFFFF;
    right = work;


    /* Now do the 16 rounds */
    if(op == DES_DECRYPT)
    {
#if 0		
        F(left, right, 30);
        F(right, left, 28);
        F(left, right, 26);
        F(right, left, 24);
        F(left, right, 22);
        F(right, left, 20);
        F(left, right, 18);
        F(right, left, 16);
        F(left, right, 14);
        F(right, left, 12);
        F(left, right, 10);
        F(right, left, 8);
        F(left, right, 6);
        F(right, left, 4);
        F(left, right, 2);
        F(right, left, 0);
#else
        F_des(&left, &right, 30,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  28,&zork,&work,(DRM_DWORD*)pTable);
		F_des(&left, &right, 26,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  24,&zork,&work,(DRM_DWORD*)pTable);
		F_des(&left, &right, 22,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  20,&zork,&work,(DRM_DWORD*)pTable);
		F_des(&left, &right, 18,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  16,&zork,&work,(DRM_DWORD*)pTable);
		F_des(&left, &right, 14,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  12,&zork,&work,(DRM_DWORD*)pTable);
		F_des(&left, &right, 10,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  8,&zork,&work,(DRM_DWORD*)pTable);
		F_des(&left, &right, 6,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  4,&zork,&work,(DRM_DWORD*)pTable);
		F_des(&left, &right, 2,&zork,&work,(DRM_DWORD*)pTable);
        F_des(&right,&left,  0,&zork,&work,(DRM_DWORD*)pTable);

#endif

    }/*
    else
    {
        F(left, right, 0);
        F(right, left, 2);
        F(left, right, 4);
        F(right, left, 6);
        F(left, right, 8);
        F(right, left, 10);
        F(left, right, 12);
        F(right, left, 14);
        F(left, right, 16);
        F(right, left, 18);
        F(left, right, 20);
        F(right, left, 22);
        F(left, right, 24);
        F(right, left, 26);
        F(left, right, 28);
        F(right, left, 30);
    }*/


    /* Inverse permutation, also from Hoey via Outerbridge and Schneier */

    right = ROTATE_RIGHT(right, 1)& 0xFFFFFFFF;
    work = left;
    work ^= right;
    work &= 0xaaaaaaaa;
    right ^= work;
    left ^= work;

    left = ROTATE_RIGHT(left, 9)& 0xFFFFFFFF;
    work = right;
    right ^= left;
    right &= 0x03fc03fc;
    work ^= right;
    left ^= right;

    left = ROTATE_RIGHT(left, 22)& 0xFFFFFFFF;
    right = work;
    work ^= left;
    work &= 0x33333333;
    right ^= work;
    left ^= work;

    right = ROTATE_RIGHT(right, 14) & 0xFFFFFFFF;
    work = right;
    right ^= left;
    right &= 0xfff0000f;
    work ^= right;
    left ^= right;

    work = ROTATE_RIGHT(work, 20) &0xFFFFFFFF;
    right = work;
    work ^= left;
    work &= 0xf0f0f0f0;
    right ^= work;
    left ^= work;

    left = ROTATE_RIGHT(left, 4)& 0xFFFFFFFF;

    /* Put the block back into the user's buffer with final swap */
    DWORD_TO_BYTES(rgbOut, left);
    DWORD_TO_BYTES(rgbOut+__CB_DECL(4), right);
}
#pragma arm section code

#endif

/*___________________________________________________________________________
 |                                                                           |
 |   deskey                                                                   |
 |___________________________________________________________________________|
*/

/**************************************************
 * deskey
 **************************************************
 */
/* Compress bit flags into a WORD as there are 16 of them
   {0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0} = 0x3F7E */
//static const UWORD16 double_shift = 0x3F7E;
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

void deskey__(
    OUT       DESTable *pTable,
    IN  const DRM_BYTE  rgbKey[__CB_DECL(DES_KEYSIZE)] )
{
    DRM_DWORD csel,
              dsel,
              t,
              s,
              i;
    
    DRM_DWORD *kp = (DRM_DWORD *)pTable;

    BYTES_TO_DWORD(csel, rgbKey);
    BYTES_TO_DWORD(dsel, rgbKey + __CB_DECL(4) );

    PERM_OP (dsel,csel,t,4,0x0f0f0f0f);
    dsel = dsel & 0xFFFFFFFF;
    csel = csel & 0xFFFFFFFF;
    t    = t    & 0xFFFFFFFF;
    HPERM_OP(csel,t,-2,0xcccc0000);
    csel = csel & 0xFFFFFFFF;
    t    = t    & 0xFFFFFFFF;
    HPERM_OP(dsel,t,-2,0xcccc0000);
    csel = csel & 0xFFFFFFFF;
    t    = t    & 0xFFFFFFFF;
    PERM_OP (dsel,csel,t,1,0x55555555);
    dsel = dsel & 0xFFFFFFFF;
    csel = csel & 0xFFFFFFFF;
    t    = t    & 0xFFFFFFFF;
    PERM_OP (csel,dsel,t,8,0x00ff00ff);
    dsel = dsel & 0xFFFFFFFF;
    csel = csel & 0xFFFFFFFF;
    t    = t    & 0xFFFFFFFF;
    PERM_OP (dsel,csel,t,1,0x55555555);
    dsel = dsel & 0xFFFFFFFF;
    csel = csel & 0xFFFFFFFF;
    t    = t    & 0xFFFFFFFF;

    dsel=  (((dsel & 0x000000ff) << 16) | (dsel & 0x0000ff00)    |
            ((dsel & 0x00ff0000) >> 16) | ((csel & 0xf0000000)>>4));
    csel &= 0x0fffffff;

    for(i = 0; i < 16; i++)
    {
        DRM_DWORD temp;

        if(0x3F7E & (1 << (15 - i)))//与x_extable对应
        {
            csel = ((csel >> 2) | (csel << 26));
            dsel = ((dsel >> 2) | (dsel << 26));
        }
        else
        {
            csel = ((csel >> 1) | (csel << 27));
            dsel = ((dsel >> 1) | (dsel << 27));
        }

        csel &= 0x0fffffff;
        dsel &= 0x0fffffff;
        //汇编程序中用R5指向_DRM_Sel
#ifdef WMA_TABLE_ROOM_VERIFY  
		//( const unsigned long* )p__DRM_Sel;
		s =  *(unsigned long*)(p__DRM_Sel + (0*64 + ((csel 	 )&0x3f) )*4);
		s |= *(unsigned long*)(p__DRM_Sel + (1*64 + (((csel >>	6)&0x03) | ((csel >>  7)&0x3c)))*4);
		s |= *(unsigned long*)(p__DRM_Sel + (2*64 + (((csel >> 13)&0x0f) | ((csel >> 14)&0x30)))*4);
		s |= *(unsigned long*)(p__DRM_Sel + (3*64 + (((csel >> 20)&0x01) | ((csel >> 21)&0x06)|((csel >> 22)&0x38)))*4);
		t =  *(unsigned long*)(p__DRM_Sel + (4*64 + ((dsel 	 )&0x3f) 			   )*4);
		t |= *(unsigned long*)(p__DRM_Sel + (5*64 + (((dsel >>	7)&0x03) | ((dsel >>  8)&0x3c)))*4);
		t |= *(unsigned long*)(p__DRM_Sel + (6*64 + ((dsel >> 15)&0x3f)			   )*4);
		t |= *(unsigned long*)(p__DRM_Sel + (7*64 + (((dsel >> 21)&0x0f) | ((dsel >> 22)&0x30)))*4);

#else
        s =  _DRM_Sel[0][ (csel      )&0x3f                ];
        s |= _DRM_Sel[1][((csel >>  6)&0x03) | ((csel >>  7)&0x3c)];
        s |= _DRM_Sel[2][((csel >> 13)&0x0f) | ((csel >> 14)&0x30)];
        s |= _DRM_Sel[3][((csel >> 20)&0x01) | ((csel >> 21)&0x06)|((csel >> 22)&0x38)];
        t =  _DRM_Sel[4][ (dsel      )&0x3f                ];
        t |= _DRM_Sel[5][((dsel >>  7)&0x03) | ((dsel >>  8)&0x3c)];
        t |= _DRM_Sel[6][ (dsel >> 15)&0x3f                ];
        t |= _DRM_Sel[7][((dsel >> 21)&0x0f) | ((dsel >> 22)&0x30)];
#endif
        temp = ((t << 16) | (s & 0x0000ffff)) ;
        temp = temp & 0xFFFFFFFF;
        *(kp++) = ROTATE_LEFT(temp, 2) &0xFFFFFFFF;
        *kp  = *kp & 0xFFFFFFFF;

        temp = ((s >> 16) | (t & 0xffff0000));
        temp = temp & 0xFFFFFFFF;
        *(kp++) = ROTATE_LEFT(temp, 6) & 0xFFFFFFFF;
        *kp  = *kp & 0xFFFFFFFF;
    }
}
#pragma arm section code

#endif

/*___________________________________________________________________________
 |                                                                           |
 |   msrc4                                                                   |
 |___________________________________________________________________________|
*/

/**************************************************
 * rc4
 **************************************************
 */
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

void rc4__(
    IN OUT RC4_KEYSTRUCT *pKS,
    IN     LENGTH_TYPE   cbBuffer,
    IN OUT BYTE      	 *pbBuffer )
{
    BYTE  i = pKS->i;
    BYTE  j = pKS->j;    
    BYTE  *p = pKS->S;
    COUNT_TYPE ib = 0;

    while (cbBuffer--)
    {
        BYTE bTemp1 = 0;
        BYTE bTemp2 = 0;

        i = ((i + 1) & (RC4_TABLESIZE - 1));
        bTemp1 = GET_BYTE(p,i);
        j = ((j + bTemp1) & (RC4_TABLESIZE - 1));
        
        PUT_BYTE(p,i,GET_BYTE(p,j));
        PUT_BYTE(p,j,bTemp1);
        bTemp2 = GET_BYTE(pbBuffer,ib);

        bTemp2 ^= GET_BYTE(p, (GET_BYTE(p,i) + bTemp1) & (RC4_TABLESIZE - 1));
        PUT_BYTE(pbBuffer,ib,bTemp2);
        ib++;
    }

    pKS->i = i;
    pKS->j = j;
}

/**************************************************
 * rc4_key
 **************************************************
 */
 
void rc4_key__(
        OUT   RC4_KEYSTRUCT  *pKS,
    IN        LENGTH_TYPE       cbKey,
    IN  const DRM_BYTE       *pbKey )
{
    DRM_BYTE j;
    DRM_BYTE k;
    DRM_BYTE t;
    COUNT_TYPE  i;

    for (i=0; i<RC4_TABLESIZE; i++)
    {
        PUT_BYTE( pKS->S, i, (DRM_BYTE)i);
    }
    
    pKS->i = 0;
    pKS->j = 0;
    j      = 0;
    k      = 0;
    for (i=0; i<RC4_TABLESIZE; i++)
    {
        t = GET_BYTE(pKS->S,i);
        j = (DRM_BYTE)((j + t + GET_BYTE(pbKey, k)) % RC4_TABLESIZE);        
        PUT_BYTE( pKS->S, i, GET_BYTE(pKS->S, j) );
        PUT_BYTE(pKS->S, j, t);
        k = (DRM_BYTE) ((k + 1) % cbKey);
    }
}
#pragma arm section code 

#endif

/*___________________________________________________________________________
 |                                                                           |
 |   sha                                                                     |
 |___________________________________________________________________________|
*/

/**************************************************
 * A_SHAInit
 **************************************************
 */
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

 void A_SHAInit__(
      IN OUT SHA_CONTEXT* pShaContext )
{
  //  /*Zero out the buffer*/
	//ZEROMEM((DRM_BYTE*)pShaContext, SIZEOF(SHA_CONTEXT));	
	//
  //  /* Set the initial magic numbers */
  //  pShaContext->ABCDE[A] = 0x67452301;
	//pShaContext->ABCDE[B] = 0xEFCDAB89;
	//pShaContext->ABCDE[C] = 0x98BADCFE;
	//pShaContext->ABCDE[D] = 0x10325476;
	//pShaContext->ABCDE[E] = 0xC3D2E1F0;
	pShaContext->dwHighByteCount = 0;
	pShaContext->dwLowByteCount = 0;
	pShaContext->ABCDE[0] = 0x67452301;
	pShaContext->ABCDE[1] = 0xEFCDAB89;
	pShaContext->ABCDE[2] = 0x98BADCFE;
	pShaContext->ABCDE[3] = 0x10325476;
	pShaContext->ABCDE[4] = 0xC3D2E1F0;
}

/**************************************************
 * A_SHAUpdate
 **************************************************
 */
 
void  A_SHAUpdate__(
    IN OUT   SHA_CONTEXT *pShaContext,  //R0
    IN const DRM_BYTE    *pbData,       //R1
    IN       DRM_DWORD    cbData        //R2
     )
{
#if SIXTEEN_BIT_ADDRESSING
    SHA_UpdateOffset__( pbData, 1, cbData, pShaContext );
#else
    SHA_UpdateOffset__( pbData, 0, cbData, pShaContext );
#endif    
    return;
}

/**************************************************
 * SHA_UpdateOffset
 **************************************************
 */
 
void  SHA_UpdateOffset__(
    IN const DRM_BYTE    *pbData,
    IN       DRM_DWORD    ibData,
    IN       DRM_DWORD    cbData,
    IN OUT   SHA_CONTEXT *pShaContext )
{
    /* If we aren't a multiple of 64 bytes we should save the last bytes to odr buffer
    ** and sha what is left
    */
    DRM_DWORD dwTempLen;


    /* How many bytes do we have remaining? */
    dwTempLen = pShaContext->dwLowByteCount & (SHA_BLOCK_SIZE - 1);
    pShaContext->dwLowByteCount += cbData;

    if(pShaContext->dwLowByteCount < cbData)
    {
        /* We overflowed and wrapped around.  This means
        ** we need to increment the high order byte counter 
        */
        pShaContext->dwHighByteCount++;
    }

    if (   dwTempLen > 0 
        && cbData   >= (SHA_BLOCK_SIZE - dwTempLen) )//carefull
    {
        /* We have enough to complete the last block.  Fill it and sha it */
        DRM_BYT_CopyBytes(pShaContext->bTempBuffer, dwTempLen, pbData, ibData, SHA_BLOCK_SIZE-dwTempLen);
        SHATransform__(pShaContext->ABCDE,pShaContext->bTempBuffer);        
        ibData += SHA_BLOCK_SIZE - dwTempLen;//初值为0，作为后面SHATransform函数的pbData的偏移地址
        cbData -= SHA_BLOCK_SIZE - dwTempLen;

        dwTempLen = 0;
    }

    /* Sha each portion of the buffer that is big enough */
    while(cbData>=SHA_BLOCK_SIZE) //carefull
    {
        SHATransform__( pShaContext->ABCDE,pbData + __CB_DECL(ibData));
        
        ibData += SHA_BLOCK_SIZE;
        cbData -= SHA_BLOCK_SIZE;
    }

    if(cbData)
    {
        DRM_BYT_CopyBytes(pShaContext->bTempBuffer, dwTempLen, pbData, ibData, cbData);
    }
}


/**************************************************
 * SHATransform
 **************************************************
 */
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

/* Define S (circular shift operator) */
#define S ROTATE_LEFT


static void SHATransform__(
    IN       DRM_DWORD ABCDE[5],
    IN const DRM_BYTE  bInput[__CB_DECL(SHA_BLOCK_SIZE)] )
{
    DRM_DWORD Buf2[5];
    DRM_DWORD W[80];                               
    COUNT_TYPE cCount;
    DRM_DWORD TEMP;


    _GetDRMDWORD(W,16,bInput);
//    memcpy((DRM_BYTE*)Buf2, (DRM_BYTE*)ABCDE, SIZEOF(DRM_DWORD) * 5);
    memcpyDWORD(Buf2, ABCDE, 5);
    for(cCount = 16; cCount < 80; cCount++)
    {
        TEMP = W[cCount-3] ^ W[cCount-8] ^ W[cCount-14] ^ W[cCount-16];
        W[cCount] = S(TEMP,1);
    }
    for(cCount = 0; cCount < 80; cCount++ )
    {
        TEMP = S(ABCDE[A],5);
        TEMP += _ft(ABCDE[B],ABCDE[C],ABCDE[D], cCount);
        TEMP += ABCDE[E];
        TEMP += W[cCount];
        TEMP += _K(cCount);
        ABCDE[E] = ABCDE[D];
        ABCDE[D] = ABCDE[C];
        ABCDE[C] = S(ABCDE[B],30);
        ABCDE[B] = ABCDE[A];
        ABCDE[A] = TEMP;
    }
    for (cCount = 0; cCount < 5; cCount++)
    {
        Buf2[cCount] += ABCDE[cCount];
    }
//    memcpy((DRM_BYTE*)ABCDE, (DRM_BYTE*)Buf2, SIZEOF(DRM_DWORD) * 5);
    memcpyDWORD(ABCDE, Buf2, 5);
}

/******************************************************************************/
/*函数DWORDFromBigEndian*/

static void _GetDRMDWORD(
    OUT      DRM_DWORD *dwTo,
    IN       LENGTH_TYPE  dwCount,
    IN const DRM_BYTE  *bIn )
{
	COUNT_TYPE i = 0;
    COUNT_TYPE j = 0;
	for( ; i < dwCount; i++, j+=4)
	{
        dwTo[i] = ( ( (DRM_DWORD) GET_BYTE(bIn,j)   ) << 24 ) | 
                  ( ( (DRM_DWORD) GET_BYTE(bIn,j+1) ) << 16 ) |
                  ( ( (DRM_DWORD) GET_BYTE(bIn,j+2) ) << 8 )  |
                  (   (DRM_DWORD) GET_BYTE(bIn,j+3) );
	}
}

/******************************************************************************/


static WORD32 _ft(WORD32 b, WORD32 c, WORD32 d, COUNT_TYPE t)
{
	assert(t<80);
	if(t >= 60)
		return (b^c^d);
	if(t>=40)
		return ((b&c)|(b&d)|(c&d));
	if(t>=20)
		return (b^c^d);
	if(t>=0)
		return ((b&c) | ((~b)&d)); //  ((b&c) | ((~b)&d)) = ((C ^ D) & B) ^ D

	return 0;	/* If valid input we should never hit this */

}

/******************************************************************************/


static DRM_DWORD _K(COUNT_TYPE t)
{

	assert(t<80);
	if(t >= 60)
		return 0xCA62C1D6;
	if(t>=40)
		return 0x8F1BBCDC;
	if(t>=20)
		return 0x6ED9EBA1;
	if(t>=0)
		return 0x5A827999;

	return 0;	/* If valid input we should never hit this */
}


/**************************************************
 * A_SHAFinal
 **************************************************
 */

void  _PackDRMDWORD(
    IN const DRM_DWORD *dwFrom,
    IN       LENGTH_TYPE  dwCount, 
    OUT      DRM_BYTE  *bOut)
{
	COUNT_TYPE i = 0;
	COUNT_TYPE j = 0;
	for(; i<dwCount; i++)
	{
		PUT_BYTE(bOut, j, (DRM_BYTE)((dwFrom[i] >> 24) & 0xff)); j++;
        PUT_BYTE(bOut, j, (DRM_BYTE)((dwFrom[i] >> 16) & 0xff)); j++;
        PUT_BYTE(bOut, j, (DRM_BYTE)((dwFrom[i] >> 8 ) & 0xff)); j++;
        PUT_BYTE(bOut, j, (DRM_BYTE)((dwFrom[i]      ) & 0xff)); j++;
	}
} 
 
 
void A_SHAFinal__(
    IN  SHA_CONTEXT *pShaContext,
    OUT DRM_BYTE     rgbDigest[__CB_DECL(SHA_DIGEST_LEN)] )
{
	DRM_DWORD dwTempLen;
	DRM_DWORD dwTotalBitLen[2];
	DRM_BYTE  bPaddingBuffer[0x48];	/* Maximum block size we may need. *///数组长度有改动
	DRM_BYTE i;	
	/* How many bytes do we need to make a SHA block? */
	dwTempLen = SHA_BLOCK_SIZE - (pShaContext->dwLowByteCount & 63);
	
	/* This is there enough room for the padding?.*/
	if(dwTempLen<=8)
	{
		dwTempLen += SHA_BLOCK_SIZE;
	}
	for (i = 0; i < dwTempLen - 6; i++)
	{
		bPaddingBuffer[i] = 0; 
 	}
	/*dwLowByteCount is the number of bytes so we have to multiply that by 8 to get the number of bits */
	dwTotalBitLen[1] = pShaContext->dwLowByteCount << 3;
	dwTotalBitLen[0] = (pShaContext->dwHighByteCount << 3) | 
						(pShaContext->dwLowByteCount >> 29); /* We could have overflowed so put the extra here */

	PUT_BYTE(bPaddingBuffer, 0, 0x80);	/* We add a 10000000 */ 
	
#if SIXTEEN_BIT_ADDRESSING
    {
        DRM_BYTE rgbTemp[__CB_DECL(2*4)];
        /* 
        ** We do the packing to a temporary buffer because the offset into
        ** the padding buffer could be odd, and _PackDRMDWORD doesn't handle
        ** that case
        */
        _PackDRMDWORD(dwTotalBitLen,2,rgbTemp);//参数位置需改动
        DRM_BYT_CopyBytes( bPaddingBuffer, dwTempLen-8, rgbTemp, 0, 2*4 );
    }

#else
  
    /* Last 2 DWORDS are for the total bit length */
    _PackDRMDWORD(dwTotalBitLen,2,&(bPaddingBuffer[(dwTempLen-8)]));
    //_PackDRMDWORD(&(bPaddingBuffer[(dwTempLen-8)]), dwTotalBitLen, 2);//参数位置有改动
#endif

	A_SHAUpdate__(pShaContext, bPaddingBuffer, dwTempLen);//参数位置有改动
	/* Extract the digest and save it. */
	_PackDRMDWORD(pShaContext->ABCDE,5,rgbDigest);		
	return ;
}
#pragma arm section code

#endif
#endif
#endif
#endif
