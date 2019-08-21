#include "image_main.h"
#ifdef JPG_DEC_INCLUDE

#pragma arm section code = "JpgDecCode", rodata = "JpgDecCode", rwdata = "JpgDecData", zidata = "JpgDecBss"
/*----------------------------------------------------------------------------*/
#include "jpgdec_decompress.h"
#include "jpgdec_globalvardeclare.h"
/*----------------------------------------------------------------------------*/

#define CONST_BITS  13
#define PASS1_BITS  2
#define SCALEDONE ((int32) 1)
#define CONST_SCALE (SCALEDONE << CONST_BITS)
//#define FIX(x)  ((int32) ((x) * CONST_SCALE + 0.5))
/*----------------------------------------------------------------------------*/
#define FIX_0_298631336  ((int32)  2446)        /* FIX(0.298631336) */
#define FIX_0_390180644  ((int32)  3196)        /* FIX(0.390180644) */
#define FIX_0_541196100  ((int32)  4433)        /* FIX(0.541196100) */
#define FIX_0_765366865  ((int32)  6270)        /* FIX(0.765366865) */
#define FIX_0_899976223  ((int32)  7373)        /* FIX(0.899976223) */
#define FIX_1_175875602  ((int32)  9633)        /* FIX(1.175875602) */
#define FIX_1_501321110  ((int32)  12299)       /* FIX(1.501321110) */
#define FIX_1_847759065  ((int32)  15137)       /* FIX(1.847759065) */
#define FIX_1_961570560  ((int32)  16069)       /* FIX(1.961570560) */
#define FIX_2_053119869  ((int32)  16819)       /* FIX(2.053119869) */
#define FIX_2_562915447  ((int32)  20995)       /* FIX(2.562915447) */
#define FIX_3_072711026  ((int32)  25172)       /* FIX(3.072711026) */
/*----------------------------------------------------------------------------*/
#define DESCALE(x,n)  (((x) + (SCALEDONE << ((n)-1))) >> (n))
/*----------------------------------------------------------------------------*/
#define MULTIPLY(var,cnst)  ((var) * (cnst))
/*----------------------------------------------------------------------------*/
void JpgDecIdct(int16 *data, int16 *Pdst_ptr)
{
    int32 tmp0, tmp1, tmp2, tmp3;
    int32 tmp10, tmp11, tmp12, tmp13;
    int32 z1, z2, z3, z4, z5;
    register int16 *dataptr;
    int rowctr;

    dataptr = data;
    for (rowctr = 8 - 1; rowctr >= 0; rowctr--)
    {
        if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
                dataptr[5] | dataptr[6] | dataptr[7]) == 0)
        {
            int16 dcval = (int16)(dataptr[0] << PASS1_BITS);

            dataptr[0] = dcval;
            dataptr[1] = dcval;
            dataptr[2] = dcval;
            dataptr[3] = dcval;
            dataptr[4] = dcval;
            dataptr[5] = dcval;
            dataptr[6] = dcval;
            dataptr[7] = dcval;

            dataptr += 8;       /* advance pointer to next row */
            continue;
        }

        z2 = (int32) dataptr[2];
        z3 = (int32) dataptr[6];

        z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
        tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
        tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

        tmp0 = ((int32) dataptr[0] + (int32) dataptr[4]) << CONST_BITS;
        tmp1 = ((int32) dataptr[0] - (int32) dataptr[4]) << CONST_BITS;

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        tmp0 = (int32) dataptr[7];
        tmp1 = (int32) dataptr[5];
        tmp2 = (int32) dataptr[3];
        tmp3 = (int32) dataptr[1];

        z1 = tmp0 + tmp3;
        z2 = tmp1 + tmp2;
        z3 = tmp0 + tmp2;
        z4 = tmp1 + tmp3;
        z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

        tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
        tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
        tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
        tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
        z1 = MULTIPLY(z1, - FIX_0_899976223);
        z2 = MULTIPLY(z2, - FIX_2_562915447);
        z3 = MULTIPLY(z3, - FIX_1_961570560);
        z4 = MULTIPLY(z4, - FIX_0_390180644);

        z3 += z5;
        z4 += z5;

        tmp0 += z1 + z3;
        tmp1 += z2 + z4;
        tmp2 += z2 + z3;
        tmp3 += z1 + z4;

        dataptr[0] = (int16) DESCALE(tmp10 + tmp3, CONST_BITS - PASS1_BITS);
        dataptr[7] = (int16) DESCALE(tmp10 - tmp3, CONST_BITS - PASS1_BITS);
        dataptr[1] = (int16) DESCALE(tmp11 + tmp2, CONST_BITS - PASS1_BITS);
        dataptr[6] = (int16) DESCALE(tmp11 - tmp2, CONST_BITS - PASS1_BITS);
        dataptr[2] = (int16) DESCALE(tmp12 + tmp1, CONST_BITS - PASS1_BITS);
        dataptr[5] = (int16) DESCALE(tmp12 - tmp1, CONST_BITS - PASS1_BITS);
        dataptr[3] = (int16) DESCALE(tmp13 + tmp0, CONST_BITS - PASS1_BITS);
        dataptr[4] = (int16) DESCALE(tmp13 - tmp0, CONST_BITS - PASS1_BITS);

        dataptr += 8;
    }

    dataptr = data;
    for (rowctr = 8 - 1; rowctr >= 0; rowctr--)
    {
        int16 i;

        if ((dataptr[8*1] | dataptr[8*2] | dataptr[8*3] |
                dataptr[8*4] | dataptr[8*5] | dataptr[8*6] |
                dataptr[8*7]) == 0)
        {
            int16 dcval = (int16) DESCALE((int32) dataptr[0], PASS1_BITS + 3);

            if ((dcval += 128) < 0)
                dcval = 0;
            else if (dcval > 255)
                dcval = 255;

            Pdst_ptr[8*0] = (int16)dcval;
            Pdst_ptr[8*1] = (int16)dcval;
            Pdst_ptr[8*2] = (int16)dcval;
            Pdst_ptr[8*3] = (int16)dcval;
            Pdst_ptr[8*4] = (int16)dcval;
            Pdst_ptr[8*5] = (int16)dcval;
            Pdst_ptr[8*6] = (int16)dcval;
            Pdst_ptr[8*7] = (int16)dcval;

            dataptr++;
            Pdst_ptr++;
            continue;
        }

        z2 = (int32) dataptr[8*2];
        z3 = (int32) dataptr[8*6];

        z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
        tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
        tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

        tmp0 = ((int32) dataptr[8*0] + (int32) dataptr[8*4]) << CONST_BITS;
        tmp1 = ((int32) dataptr[8*0] - (int32) dataptr[8*4]) << CONST_BITS;

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        tmp0 = (int32) dataptr[8*7];
        tmp1 = (int32) dataptr[8*5];
        tmp2 = (int32) dataptr[8*3];
        tmp3 = (int32) dataptr[8*1];

        z1 = tmp0 + tmp3;
        z2 = tmp1 + tmp2;
        z3 = tmp0 + tmp2;
        z4 = tmp1 + tmp3;
        z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

        tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
        tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
        tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
        tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
        z1 = MULTIPLY(z1, - FIX_0_899976223);
        z2 = MULTIPLY(z2, - FIX_2_562915447);
        z3 = MULTIPLY(z3, - FIX_1_961570560);
        z4 = MULTIPLY(z4, - FIX_0_390180644);

        z3 += z5;
        z4 += z5;

        tmp0 += z1 + z3;
        tmp1 += z2 + z4;
        tmp2 += z2 + z3;
        tmp3 += z1 + z4;

#define clamp(i) if (i & 0xFF00) i = (((~i) >> 15) & 0xFF);

        i = (int16) DESCALE(tmp10 + tmp3, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*0] = (int16)i;

        i = (int16) DESCALE(tmp10 - tmp3, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*7] = (int16)i;

        i = (int16) DESCALE(tmp11 + tmp2, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*1] = (int16)i;

        i = (int16) DESCALE(tmp11 - tmp2, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*6] = (int16)i;

        i = (int16) DESCALE(tmp12 + tmp1, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*2] = (int16)i;

        i = (int16) DESCALE(tmp12 - tmp1, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*5] = (int16)i;

        i = (int16) DESCALE(tmp13 + tmp0, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*3] = (int16)i;

        i = (int16) DESCALE(tmp13 - tmp0, CONST_BITS + PASS1_BITS + 3) + 128;
        clamp(i)
        Pdst_ptr[8*4] = (int16)i;

        dataptr++;
        Pdst_ptr++;
    }
}
/*----------------------------------------------------------------------------*/
#pragma arm section code
#endif
