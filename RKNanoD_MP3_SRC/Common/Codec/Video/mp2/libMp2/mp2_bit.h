/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name£º   bit.h
* 
* Description:  use for bitstream
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

# ifndef LIBMAD_BIT_H
# define LIBMAD_BIT_H

struct mad_bitptr {
  unsigned long *ptr;
  unsigned long readbit;
};

void mp2_mad_bit_init(struct mad_bitptr *, unsigned char const *);

# define mp2_mad_bit_finish(bitptr)		/* nothing */

unsigned int mp2_mad_bit_length(struct mad_bitptr const *,
			    struct mad_bitptr const *);

unsigned char mp2_mad_bit_bitsleft(struct mad_bitptr const *bitptr);
unsigned char const *mp2_mad_bit_nextbyte(struct mad_bitptr const *);

void mp2_mad_bit_skip(struct mad_bitptr *, unsigned int);
unsigned long mp2_mad_bit_read(struct mad_bitptr *, unsigned int);
void mp2_mad_bit_write(struct mad_bitptr *, unsigned int, unsigned long);

unsigned short mp2_mad_bit_crc(struct mad_bitptr, unsigned int, unsigned short);

# endif
