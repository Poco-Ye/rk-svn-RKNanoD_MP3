/* utils.c
 *
 * 32 bit fractional multiplication. Requires 64 bit integer support.
 */

 #include  "SysInclude.h"
#include  "audio_main.h"
#ifdef MP3_ENC_INCLUDE2
     
#pragma arm section code = "EncodeMP3Code", rodata = "EncodeMP3Code", rwdata = "EncodeMP3Data", zidata = "EncodeMP3Bss"

/* Fractional multiply. */
long mul(long x, long y)
{
  return (long)(((long long)x * (long long)y) >> 32);
}

/* Left justified fractional multiply. */
long muls(long x, long y)
{
  return (long)(((long long)x * (long long)y) >> 31);
}

/* Fractional multiply with rounding. */
long mulr(long x, long y)
{
  return (long)((((long long)x * (long long)y) + 0x80000000) >> 32);
}

/* Left justified fractional multiply with rounding. */
long mulsr(long x, long y)
{
  return (long)((((long long)x * (long long)y) + 0x40000000) >> 31);
}


#pragma arm section code
#endif

