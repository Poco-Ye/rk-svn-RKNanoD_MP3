#ifndef JPGDEC_FIXED_H
#define JPGDEC_FIXED_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint32_t fixed_t;

#define BK_RENDER_PRECISION 16					            // number of fractional bits
#define BK_RENDER_ONE		(1 << BK_RENDER_PRECISION)	    // representation of 1

#if 1
#define fixed_1			            (fixed_from_int(1))
#define fixed_from_int(i)           ((fixed_t) ((i) << 16))
#define int_from_fixed(f)           ((int) ((f) >> 16))
#define float_from_fixed(f)         (float)((double)(f) * (1.0f/(float)(BK_RENDER_ONE)))
#define fixed_from_float(f)         (fixed_t)(f * (float)(BK_RENDER_ONE))
#else
fixed_t fixed_from_int(int value)
{
	return value << BK_RENDER_PRECISION;
}

int int_from_fixed(fixed_t value) 
{
	return value >> BK_RENDER_PRECISION;
}

float float_from_fixed(fixed_t value) 
{
	return (float)((double)(value) * (1.0f/(float)(BK_RENDER_ONE)));
}

fixed_t fixed_from_float(float value) {
	if (value >= 32767.5f)
		return 0x7fffffff;
	else if (value <= -32768.0f)
		return 0x80000000;
	else
		return (fixed_t)(value * (float)(BK_RENDER_ONE));
}
#endif

#if 1
__inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
	fixed_t	alow = a & 0x0ffff;
	fixed_t	ahigh = a >> 16;
	fixed_t	blow = b & 0x0ffff;
	fixed_t	bhigh = b >> 16;

	fixed_t	abhigh = ahigh * bhigh;

	fixed_t ret = (fixed_t)(((alow * blow) >> 16)
						+ ahigh * blow
						+ alow * bhigh
						+ (abhigh << 16) );

//	if ( a != 0 && b != 0 && ret == 0 )
//		printf("\nfixed point underflow!\n");;	// fixed point underflow! Not necessarily bad, but often an indication that steps should be taken before multiplication to make the computation in question more robust
	
	return ret;
}

#else
__inline __asm fixed_t FixedMul(fixed_t a, fixed_t b)
{
    PUSH    {r2-r5}
    SMULL   r3, r2, r1, r0
    LSR     r4, r3, #16
    LSL     r5, r2, #16
    ADD     r0, r4, r5
    POP     {r2-r5}
    BX      LR
}
#endif

__inline fixed_t FixedMulWithOverflowCheck(fixed_t a, fixed_t b, uint32* pOverflow)
{
    fixed_t	ret;
	fixed_t	alow;
	fixed_t	blow;
	fixed_t   expectedSignBit;

	fixed_t	ahigh;
	fixed_t	bhigh;
	fixed_t	abhigh;
	fixed_t   abhightest;
	
	if (!a || !b)
		return 0;

	ahigh = a >> 16;
	bhigh = b >> 16;
	abhigh = ahigh * bhigh;
	abhightest = abhigh>>15;
	
	if (abhightest >= 0)
	{
		if (abhightest != 0)  //上溢，有符号整型超过16位
		{
		    printf("\n =====overflow=====\n");
			*pOverflow = 1;
			return (fixed_t)0x7fffffff;
		}
	}
	else
	{
		if (abhightest != -1)  //下溢，有符号整型超过16位
		{
		    printf("\n =====underflow=====\n");
			*pOverflow = 1;
			return (fixed_t)0x80000001;
		}
	}

	alow = a & 0x0ffff;
	blow = b & 0x0ffff;
	ret = (fixed_t)(((alow * blow) >> 16)
		+ ahigh * blow
		+ alow * bhigh
		+ (((fixed_t)abhigh) << 16) );

	expectedSignBit = (a&0x80000000) ^ (b&0x80000000);
	if ((fixed_t)(ret&0x80000000) != expectedSignBit)
	{
		*pOverflow = 1;
		return expectedSignBit != 0 ? (fixed_t)0x80000001 : (fixed_t)0x7fffffff;
	}
	return ret;
}

__inline int PositionOfMostSignificantBitOfNonzeroInteger(fixed_t x )
{
	int n = 31;
	
	if(x == 0)
	    return 0;
	    
	if ( x <= 0x0000FFFF ) { n -= 16; x <<= 16; }
	if ( x <= 0x00FFFFFF ) { n -= 8;  x <<= 8; }
	if ( x <= 0x0FFFFFFF ) { n -= 4;  x <<= 4; }
	if ( x <= 0x3FFFFFFF ) { n -= 2;  x <<= 2; }
	if ( x <= 0x7FFFFFFF ) { n -= 1; }
	return n;
}

// return a / b;
// this routine does an integer divide of (a << 16) / b
// without using either 64 bit integers or any integer divide instructions
__inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
//	int	neg = 0;
	int msb_a, msb_b;
    int	msb_result, b_shift;
    
    fixed_t remainder;
	fixed_t	tmp, result = 0;
	fixed_t	bit;
	int b_shift_minus_msb_result;
	int b_shift_min;

//	if ( a < 0 ) {
//		neg = 1;
//		a = -a;
//		if ( a < 0 )
//			--a;
//	}
//	if ( b < 0 ) {
//		neg = !neg;
//		b = -b;
//		if ( b < 0 )
//			--b;
//	}

	// check for some early conditions that would cause infinite loops
	if ( a == 0 )
		return 0;
	if ( b == 0 )
		return 0x7fffffff;

	// find the position of the most significant bit of 'a'
	msb_a = PositionOfMostSignificantBitOfNonzeroInteger( (fixed_t)a );
	msb_b = PositionOfMostSignificantBitOfNonzeroInteger( (fixed_t)b );

	// the most significant bit of the result is either msb_a + 16 - msb_b or one less than this value (depending on whether the mantissa of a or b is larger)
	// if this is greater than 30 then we have an overflow
	msb_result = msb_a + 16 - msb_b;

	// if it is less than zero then we have an underflow
	if ( msb_result < 0 )
		return 0;

	b_shift = 31 - msb_b;

	if ( msb_result > 30 ) {
		if ( msb_result == 31 && ((fixed_t)b<<15) > (fixed_t)a ) {
			msb_result = 30;
			--b_shift;
		}
		else {
//			return neg ? -0x7fffffff : 0x7fffffff;
            return 0x7fffffff;
		}
	}

	remainder = a << (31 - msb_a);

	bit = 1 << msb_result;
	b_shift_minus_msb_result = b_shift - msb_result;
	b_shift_min = b_shift_minus_msb_result >= 0 ? b_shift_minus_msb_result : 0; // max( 0, b_shift - msb_result )
	while ( b_shift >= b_shift_min )	{
	    if(bit == 0)
	        return 0;
		tmp = (fixed_t)b << b_shift;
		if ( remainder >= tmp )	{
			remainder -= tmp;
			result += bit;
		}
		--b_shift;
		bit >>= 1;
	}
	
    if(b_shift != -1 && bit != 0)
        return 0;
        
	b_shift = 1; 
	while ( bit )	{
		tmp = (fixed_t)b >> b_shift;
		if ( tmp == 0 )
			break;
		if ( remainder >= tmp )	{
			remainder -= tmp;
			result += bit;
		}
		++b_shift;
		bit >>= 1;
	}

//	return neg ? -(fixed_t)result : (fixed_t)result;
    return result;
}
#endif

