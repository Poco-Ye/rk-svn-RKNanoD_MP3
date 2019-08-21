/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   bit.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

/*
    1. 将mp3_crc_table[256]表格移至SoC芯片中的TableROM中；
    2. 将mad_bit_read()函数改为汇编优化;
    
    by Vincent Hsiung.
*/

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE


#define CHAR_BIT  8
# include "mp3_global.h"
# include "mp3_bit.h"

//By Vincent Hsiung, Move Const Table Out of Here.
/*
 * This is the lookup table for computing the CRC-check word.
 * As described in section 2.4.3.1 and depicted in Figure A.9
 * of ISO/IEC 11172-3, the generator polynomial is:
 *
 * G(X) = X^16 + X^15 + X^2 + 1
 */
extern unsigned short const mp3_crc_table[256] ;
#define crc_table	mp3_crc_table

# define CRC_POLY  0x8005

/*
 * NAME:	bit->init()
 * DESCRIPTION:	initialize bit pointer struct
 */
_ATTR_MP3DEC_TEXT_
#if !MACROHL
void mad_bit_init(struct mad_bitptr *bitptr, unsigned char const *byte)
{
  bitptr->ptr     = (unsigned long*)((unsigned int)byte & ~3);
  bitptr->readbit = ((unsigned long)byte & 3) << 3;
  //DEBUG("bye = %x,ptr=%x,readbit=%x\n",(unsigned int )byte,(unsigned int )bitptr->ptr ,(unsigned int )bitptr->readbit);
}
#endif

/*
 * NAME:	bit->length()
 * DESCRIPTION:	return number of bits between start and end points
 */
_ATTR_MP3DEC_TEXT_
#if !MACROHL
unsigned int mad_bit_length(struct mad_bitptr const *begin,
			    struct mad_bitptr const *end)
{
  return end->readbit - begin->readbit;
}

#endif
_ATTR_MP3DEC_TEXT_
#if !MACROHL
unsigned char mad_bit_bitsleft(struct mad_bitptr const *bitptr)
{
  return 8 - (bitptr->readbit & 7);
}

#endif

/*
 * NAME:	bit->nextbyte()
 * DESCRIPTION:	return pointer to next unprocessed byte
 */
_ATTR_MP3DEC_TEXT_
#if  !MACROHL
unsigned char const *mad_bit_nextbyte(struct mad_bitptr const *bitptr)
{
  return (unsigned char const*)bitptr->ptr + ((bitptr->readbit + 7) >> 3);
}

#endif

/*
 * NAME:	bit->skip()
 * DESCRIPTION:	advance bit pointer
 */
_ATTR_MP3DEC_TEXT_
#if !MACROHL
void mad_bit_skip(struct mad_bitptr *bitptr, unsigned int len)
{
  bitptr->readbit += len;
}
#endif


/*
 * NAME:	bit->read()
 * DESCRIPTION:	read an arbitrary number of bits and return their UIMSBF value
 */
 //By Vincent Hsiung, for Speed Opt
_ATTR_MP3DEC_TEXT_
__asm unsigned long mad_bit_read(struct mad_bitptr *bitptr, unsigned int len)
{
	PUSH     {r4-r6,lr}
	MOV      r4,r0		;r4 = bitptr
	MOVS     r5,r1		;r5 = len
	ITT      EQ			;if len == 0
	MOVEQ    r0,#0
	POPEQ    {r4-r6,pc}	;return 0

	;len != 0
	LDR      r1,[r4,#4]			;r1 = bitptr->readbit

	;early store
	ADD      r2,r1,r5			;r0 = bitptr->readbit + len
	STR      r2,[r4,#4]			;bitptr->readbit += len;

	LDR      r0,[r0,#0]
	LSR      r2,r1,#5
	ADD      r3,r0,r2,LSL #2	;r3 = &bitptr->ptr[bitptr->readbit>>5]

	LDR      r0,[r3,#0]
	REV		 r0,r0
	AND      r2,r1,#0x1f
	LSL      r6,r0,r2			;r = betoh32(curr[0]) << (bitptr->readbit & 31);
	ADDS     r0,r2,r5			;(bitptr->readbit & 31) + len 
	CMP      r0,#0x20			; > 32 ?
	;BLS      less_than_32		; <= 32
	ITTT	  LS
	RSBLS      r0,r5,#0x20		;r0 = 32-len
	LSRLS      r0,r6,r0
	POPLS      {r4-r6,pc}

	LDR      r0,[r3,#4]			;r0 = curr[1]
	REV		 r0,r0
	MOV		 r2,r1				;r2 = r1 = bitptr->readbit
	RSB		 r2,r2,#0
	AND		 r2,r2,#0x1f		;r2 = -bitptr->readbit & 31
	LSR      r0,r0,r2
	ADD      r6,r6,r0			;r += betoh32(curr[1]) >> (-bitptr->readbit & 31);

;less_than_32
	;ADD      r0,r1,r5			;r0 = bitptr->readbit + len
	;STR      r0,[r4,#4]			;bitptr->readbit += len;
	RSB      r0,r5,#0x20
	LSR      r0,r6,r0
	POP      {r4-r6,pc}
}

/*
 * NAME:	bit->crc()
 * DESCRIPTION:	compute CRC-check word
 */
_ATTR_MP3DEC_TEXT_
unsigned short mad_bit_crc(struct mad_bitptr bitptr, unsigned int len,
			   unsigned short init)
{
  register unsigned int crc;

  for (crc = init; len >= 32; len -= 32) {
    register unsigned long data;

    data = mad_bit_read(&bitptr, 32);

    crc = (crc << 8) ^ crc_table[((crc >> 8) ^ (data >> 24)) & 0xff];
    crc = (crc << 8) ^ crc_table[((crc >> 8) ^ (data >> 16)) & 0xff];
    crc = (crc << 8) ^ crc_table[((crc >> 8) ^ (data >>  8)) & 0xff];
    crc = (crc << 8) ^ crc_table[((crc >> 8) ^ (data >>  0)) & 0xff];
  }

  switch (len / 8) {
  case 3: crc = (crc << 8) ^
	    crc_table[((crc >> 8) ^ mad_bit_read(&bitptr, 8)) & 0xff];
  case 2: crc = (crc << 8) ^
	    crc_table[((crc >> 8) ^ mad_bit_read(&bitptr, 8)) & 0xff];
  case 1: crc = (crc << 8) ^
	    crc_table[((crc >> 8) ^ mad_bit_read(&bitptr, 8)) & 0xff];

  len %= 8;

  case 0: break;
  }

  while (len--) {
    register unsigned int msb;

    msb = mad_bit_read(&bitptr, 1) ^ (crc >> 15);

    crc <<= 1;
    if (msb & 1)
      crc ^= CRC_POLY;
  }

  return crc & 0xffff;
}

#endif
#endif
