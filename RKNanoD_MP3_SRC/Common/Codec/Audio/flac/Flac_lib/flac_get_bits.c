/*
 * copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * bitstream reader API header.
 */

//#include <stdint.h>
#include <stdio.h>
#include "flac_get_bits.h"



  unsigned short swap16(unsigned short value)
{
	return (value >> 8) | (value << 8);
}
  unsigned long swap32(unsigned long value)
{
	unsigned long hi = swap16(value >> 16);
	unsigned long lo = swap16(value & 0xffff);

	return (lo << 16) | hi;
}

unsigned int unaligned32( void *v) {
	return *(unsigned int *)v;
}



  int unaligned32_be( void *v){
	return betoh32(unaligned32(v));  // original
}

  int get_bits_count( GetBitContext *s){
    return s->index;
}




/**
 * Read 1-25 bits.
 */
 unsigned int get_bits(GetBitContext *s, int n){
    register int tmp;
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    tmp = SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
    return tmp;
}



  unsigned int get_bits1(GetBitContext *s){
    unsigned int index = s->index;
    unsigned char result = s->buffer[index>>3];

    result <<= index & 7;
    result >>= 8 - 1;
    index++;
    s->index = index;

    return result;

}

/**
 * reads 0-32 bits.
 */


  int init_get_bits(GetBitContext *s,
                    unsigned char *buffer, int bit_size)
{
    int buffer_size = (bit_size+7)>>3;
    if (buffer_size < 0 || bit_size < 0) {
        buffer_size = bit_size = 0;
        buffer = NULL;
		return -1;
    }

    s->buffer       = buffer;
    s->size_in_bits = bit_size;
    s->buffer_end   = buffer + buffer_size;
    s->index        = 0;
	return 0;
}


  int get_bits_left(GetBitContext *gb)
{
    return gb->size_in_bits - get_bits_count(gb);
}



  int my_bui_clz(unsigned int a)
  {
	  int i;
	  unsigned int tmp_index = 1 << 31;
	  if(a == 0)
		  return 31;
	  for(i=0;i<32;i++)
	  {
		  if((a & tmp_index)==tmp_index)
			  return i;
		  else tmp_index = tmp_index >> 1;
	  }
  }


