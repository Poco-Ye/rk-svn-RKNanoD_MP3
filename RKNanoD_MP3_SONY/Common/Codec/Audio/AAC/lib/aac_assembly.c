

#include "assembly.h"
  	#include "../../include/audio_main.h"
		
		
#ifdef A_CORE_DECODE		
#ifdef AAC_DEC_INCLUDE
#pragma arm section code = "AacDecCode", rodata = "AacDecCode", rwdata = "AacDecData", zidata = "AacDecBss"
#if (defined (_WIN32) && !defined (_WIN32_WCE)) || (defined (__WINS__) && defined (_SYMBIAN)) || (defined (WINCE_EMULATOR)) || (defined (_OPENWAVE_SIMULATOR))

#pragma warning( disable : 4035 )    /* complains about inline asm not returning a value */
 int MULSHIFT32(int x, int y)	
{
    __asm {
		mov		eax, x
	    imul	y
	    mov		eax, edx
	    }
}
 short CLIPTOSHORT(int x)
{
	int sign;
	sign = x >> 31;
	if (sign != (x >> 15))
		x = sign ^ ((1 << 15) - 1);
	return (short)x;
}
 int FASTABS(int x) 
{
	int sign;
	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;
	return x;
}
 int CLZ(int x)
{
	int numZeros;
	if (!x)
		return 32;
	numZeros = 1;
	if (!((unsigned int)x >> 16))	{ numZeros += 16; x <<= 16; }
	if (!((unsigned int)x >> 24))	{ numZeros +=  8; x <<=  8; }
	if (!((unsigned int)x >> 28))	{ numZeros +=  4; x <<=  4; }
	if (!((unsigned int)x >> 30))	{ numZeros +=  2; x <<=  2; }
	numZeros -= ((unsigned int)x >> 31);
	return numZeros;
}
 Word64 MADD64(Word64 sum64, int x, int y)
{
	U64 u;
	u.w64 = sum64;
	sum64 += (Word64)x * (Word64)y;
	return sum64;
   
}
#elif defined (_WIN32) && defined (_WIN32_WCE) && defined (ARM)
 short CLIPTOSHORT(int x)
{
	int sign;
	sign = x >> 31;
	if (sign != (x >> 15))
		x = sign ^ ((1 << 15) - 1);
	return (short)x;
}
 int FASTABS(int x) 
{
	int sign;
	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;
	return x;
}
 int CLZ(int x)
{
	int numZeros;
	if (!x)
		return 32;
	numZeros = 1;
	if (!((unsigned int)x >> 16))	{ numZeros += 16; x <<= 16; }
	if (!((unsigned int)x >> 24))	{ numZeros +=  8; x <<=  8; }
	if (!((unsigned int)x >> 28))	{ numZeros +=  4; x <<=  4; }
	if (!((unsigned int)x >> 30))	{ numZeros +=  2; x <<=  2; }
	numZeros -= ((unsigned int)x >> 31);
	return numZeros;
}
#ifdef __cplusplus
extern "C" {
#endif
#define MULSHIFT32	raac_MULSHIFT32
#define MADD64		raac_MADD64
int MULSHIFT32(int x, int y);
Word64 MADD64(Word64 sum64, int x, int y);
#ifdef __cplusplus
}
#endif
#elif defined (__arm) && defined (__ARMCC_VERSION)
  int MULSHIFT32(int x, int y);
  short CLIPTOSHORT(int x);
  int FASTABS(int x) ;
 int CLZ(int x) ;
  Word64 MADD64(Word64 sum64, int x, int y) ;
#elif defined(__GNUC__) && defined(__arm__)
int MULSHIFT32(int x, int y)
{
    int zlow;
    __asm__ volatile ("smull %0,%1,%2,%3" : "=&r" (zlow), "=r" (y) : "r" (x), "1" (y) : "cc");
    return y;
}
 short CLIPTOSHORT(int x)
{
	int sign;
	sign = x >> 31;
	if (sign != (x >> 15))
		x = sign ^ ((1 << 15) - 1);
	return (short)x;
}
 int FASTABS(int x) 
{
	int sign;
	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;
	return x;
}
 int CLZ(int x)
{
	int numZeros;
	if (!x)
		return (sizeof(int) * 8);
	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	} 
	return numZeros;
}
typedef long long Word64;
typedef union _U64 {
	Word64 w64;
	struct {
		unsigned int lo32;
		signed int   hi32;
	} r;
} U64;
 Word64 MADD64(Word64 sum64, int x, int y)
{
	U64 u;
	u.w64 = sum64;
	__asm__ volatile ("smlal %0,%1,%2,%3" : "+&r" (u.r.lo32), "+&r" (u.r.hi32) : "r" (x), "r" (y) : "cc");
	return u.w64;
}
#elif defined(__GNUC__) && defined(__i386__)
typedef long long Word64;
 int MULSHIFT32(int x, int y)
{
    int z;
    z = (Word64)x * (Word64)y >> 32;
	return z;
}
 short CLIPTOSHORT(int x)
{
	int sign;
	sign = x >> 31;
	if (sign != (x >> 15))
		x = sign ^ ((1 << 15) - 1);
	return (short)x;
}
 int FASTABS(int x) 
{
	int sign;
	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;
	return x;
}
 int CLZ(int x)
{
	int numZeros;
	if (!x)
		return 32;
	numZeros = 1;
	if (!((unsigned int)x >> 16))	{ numZeros += 16; x <<= 16; }
	if (!((unsigned int)x >> 24))	{ numZeros +=  8; x <<=  8; }
	if (!((unsigned int)x >> 28))	{ numZeros +=  4; x <<=  4; }
	if (!((unsigned int)x >> 30))	{ numZeros +=  2; x <<=  2; }
	numZeros -= ((unsigned int)x >> 31);
	return numZeros;
}
typedef union _U64 {
    Word64 w64;
    struct {
        /* x86 = little endian */
        unsigned int lo32; 
        signed int   hi32;
    } r;
} U64;
 Word64 MADD64(Word64 sum64, int x, int y)
{
	sum64 += (Word64)x * (Word64)y;
	return sum64;
}
#else
#error Unsupported platform in assembly.h
#endif	/* platforms */
#if (defined (_WIN32) && !defined (_WIN32_WCE)) || (defined (__WINS__) && defined (_SYMBIAN)) || (defined (WINCE_EMULATOR)) || (defined (_OPENWAVE_SIMULATOR))
#pragma warning( disable : 4035 )    /* complains about inline asm not returning a value */

/* returns 64-bit value in [edx:eax] */
 

/* toolchain:           MSFT Embedded Visual C++
 * target architecture: ARM v.4 and above (require 'M' type processor for 32x32->64 multiplier)
 */
#elif defined (_WIN32) && defined (_WIN32_WCE) && defined (ARM)
 

/* implemented in asmfunc.s */
#ifdef __cplusplus
extern "C" {
#endif

typedef __int64 Word64;

typedef union _U64 {
    Word64 w64;
    struct {
        /* ARM WinCE = little endian */
        unsigned int lo32; 
        signed int   hi32;
    } r;
} U64;

/* manual name mangling for just this platform (must match labels in .s file) */
#define MULSHIFT32    raac_MULSHIFT32
#define MADD64        raac_MADD64

int    MULSHIFT32(int x, int y);
Word64 MADD64(Word64 sum64, int x, int y);

#ifdef __cplusplus
}
#endif

/* toolchain:           ARM ADS or RealView
 * target architecture: ARM v.4 and above (requires 'M' type processor for 32x32->64 multiplier)
 */
#elif defined (__arm) && defined (__ARMCC_VERSION)

 __asm	  int    MULSHIFT32(int x, int y)
{

  		
	PUSH {R2};

    SMULL  R2,R0,R0,R1 ;

 
	POP {R2};
	
	BX LR;

}

 
   short CLIPTOSHORT(int x)
{
    int sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31;
    if (sign != (x >> 15))
        x = sign ^ ((1 << 15) - 1);
    return (short)x;
}

   int FASTABS(int x) 
{
    int sign;

    sign = x >> (sizeof(int) * 8 - 1);
    x ^= sign;
    x -= sign;

    return x;
}

 __asm  int CLZ(int x)
{



 	PUSH  {R1} ;
    	CLZ R1,R0 ;
	MOV R0,R1;	

	POP {R1};
	
	BX LR;



}



 __asm   Word64 MADD64(Word64 sum64, int x, int y) 
{

        SMLAL R0, R1, R2, R3 ;
		BX LR;
  
}

/* toolchain:           ARM gcc
 * target architecture: ARM v.4 and above (requires 'M' type processor for 32x32->64 multiplier)
 */
#elif defined(__GNUC__) && defined(__arm__)

 
typedef long long Word64;

typedef union _U64 {
    Word64 w64;
    struct {
        /* ARM ADS = little endian */
        unsigned int lo32;
        signed int   hi32;
    } r;
} U64;

   

/* toolchain:           x86 gcc
 * target architecture: x86
 */
#elif defined(__GNUC__) && defined(__i386__)

typedef long long Word64;

   

typedef union _U64 {
    Word64 w64;
    struct {
        /* x86 = little endian */
        unsigned int lo32;
        signed int   hi32;
    } r;
} U64;

 

#else

#error Unsupported platform in assembly.h

#endif    /* platforms */
#pragma arm section code
#endif

#endif

