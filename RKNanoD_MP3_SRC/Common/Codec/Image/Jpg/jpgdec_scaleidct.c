#include "image_main.h"
#ifdef JPG_DEC_INCLUDE

#pragma arm section code = "JpgDecCode", rodata = "JpgDecCode", rwdata = "JpgDecData", zidata = "JpgDecBss"

#include "jpgdec_decompress.h"
#include "jpgdec_globalvardeclare.h"

#define CONST_BITS  13
#define PASS1_BITS  2
#define SCALEDONE ((int32) 1)
#define CONST_SCALE (SCALEDONE << CONST_BITS)



#define FIX_0_211164243  ((int32)  1730) /* FIX(0.211164243) */
#define FIX_0_509795579  ((int32)  4176) /* FIX(0.509795579) */
#define FIX_0_601344887  ((int32)  4926) /* FIX(0.601344887) */
#define FIX_0_720959822  ((int32)  5906) /* FIX(0.720959822) */
#define FIX_0_765366865  ((int32)  6270) /* FIX(0.765366865) */
#define FIX_0_850430095  ((int32)  6967) /* FIX(0.850430095) */
#define FIX_0_899976223  ((int32)  7373) /* FIX(0.899976223) */
#define FIX_1_061594337  ((int32)  8697) /* FIX(1.061594337) */
#define FIX_1_272758580  ((int32)  10426) /* FIX(1.272758580) */
#define FIX_1_451774981  ((int32)  11893) /* FIX(1.451774981) */
#define FIX_1_847759065  ((int32)  15137) /* FIX(1.847759065) */
#define FIX_2_172734803  ((int32)  17799) /* FIX(2.172734803) */
#define FIX_2_562915447  ((int32)  20995) /* FIX(2.562915447) */
#define FIX_3_624509785  ((int32)  29692) /* FIX(3.624509785) */



#define DESCALE(x,n)  (((x) + (SCALEDONE << ((n)-1))) >> (n))
#define MULTIPLY(var,cnst)  ((var) * (cnst))
#define clamp(i) if (i & 0xFF00) i = (((~i) >> 15) & 0xFF);
void JpgDecIdct4x4(int16 *data, int16 *Pdst_ptr)
{
    int32 tmp0, tmp2;
    int32 tmp10, tmp12;
    int32 z1, z2, z3, z4;
    register int16 *dataptr;
    int rowctr;
    int16 i;
    int16 dcval;

    dataptr = data;

#if 0
    for (j = 0;j < 64;j++)
    {
        if (j == 0 || j == 1 || j == 2 || j == 3
                || j == 8 || j == 9 || j == 10 || j == 11
                || j == 16 || j == 17 || j == 18 || j == 19
                || j == 24 || j == 25 || j == 26 || j == 27)
            continue;
        else
            dataptr[j] = 0;
    }

    //fwrite(dataptr,sizeof(int16),64,IDCTInFile);
#endif
    for (rowctr = 8; rowctr > 0; rowctr--)
    {    /* Don't bother to process column 4, because second pass won't use it */
        if (rowctr == 4)
        {
            dataptr += 8;
            continue;
        }

        if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
                dataptr[5] | dataptr[6] | dataptr[7]) == 0)
        {
            dcval = (int16)(dataptr[0] << PASS1_BITS);

            dataptr[0] = dcval;
            dataptr[1] = dcval;
            dataptr[2] = dcval;
            dataptr[3] = dcval;

            dataptr += 8;       /* advance pointer to next row */
            continue;
        }

        tmp0 = (int32) dataptr[0];
        tmp0 <<= (CONST_BITS + 1);

        z2 = (int32) dataptr[2];
        z3 = (int32) dataptr[6];


        tmp2 = MULTIPLY(z2, FIX_1_847759065) + MULTIPLY(z3, - FIX_0_765366865);

        tmp10 = tmp0 + tmp2;
        tmp12 = tmp0 - tmp2;

        z1 = (int32) dataptr[7];
        z2 = (int32) dataptr[5];
        z3 = (int32) dataptr[3];
        z4 = (int32) dataptr[1];

        tmp0 = MULTIPLY(z1, - FIX_0_211164243) /* sqrt(2) * (c3-c1) */
               + MULTIPLY(z2, FIX_1_451774981) /* sqrt(2) * (c3+c7) */
               + MULTIPLY(z3, - FIX_2_172734803) /* sqrt(2) * (-c1-c5) */
               + MULTIPLY(z4, FIX_1_061594337); /* sqrt(2) * (c5+c7) */

        tmp2 = MULTIPLY(z1, - FIX_0_509795579) /* sqrt(2) * (c7-c5) */
               + MULTIPLY(z2, - FIX_0_601344887) /* sqrt(2) * (c5-c1) */
               + MULTIPLY(z3, FIX_0_899976223) /* sqrt(2) * (c3-c7) */
               + MULTIPLY(z4, FIX_2_562915447); /* sqrt(2) * (c1+c3) */


        dataptr[0] = (int) DESCALE(tmp10 + tmp2, CONST_BITS - PASS1_BITS + 1);
        dataptr[3] = (int) DESCALE(tmp10 - tmp2, CONST_BITS - PASS1_BITS + 1);
        dataptr[1] = (int) DESCALE(tmp12 + tmp0, CONST_BITS - PASS1_BITS + 1);
        dataptr[2] = (int) DESCALE(tmp12 - tmp0, CONST_BITS - PASS1_BITS + 1);


        dataptr += 8;
    }

    dataptr = data;
    for (rowctr = 4; rowctr > 0; rowctr--)
    {

        if ((dataptr[8*1] | dataptr[8*2] | dataptr[8*3] |
                dataptr[8*5] | dataptr[8*6] | dataptr[8*7]) == 0)
        {
            dcval = (int16) DESCALE((int32) dataptr[0], PASS1_BITS + 3);

            if ((dcval += 128) < 0)
                dcval = 0;
            else if (dcval > 255)
                dcval = 255;


            Pdst_ptr[8*0] = (int16)dcval;
            Pdst_ptr[8*1] = (int16)dcval;
            Pdst_ptr[8*2] = (int16)dcval;
            Pdst_ptr[8*3] = (int16)dcval;

            dataptr++;
            Pdst_ptr++;
            continue;
        }
        tmp0 = ((int32) dataptr[0]) << (CONST_BITS + 1);
        tmp2 = MULTIPLY((int32) dataptr[8*2], FIX_1_847759065)
               + MULTIPLY((int32) dataptr[8*6], - FIX_0_765366865);

        tmp10 = tmp0 + tmp2;
        tmp12 = tmp0 - tmp2;


        z1 = (int32) dataptr[8*7];
        z2 = (int32) dataptr[8*5];
        z3 = (int32) dataptr[8*3];
        z4 = (int32) dataptr[8*1];

        tmp0 = MULTIPLY(z1, - FIX_0_211164243) /* sqrt(2) * (c3-c1) */
               + MULTIPLY(z2, FIX_1_451774981) /* sqrt(2) * (c3+c7) */
               + MULTIPLY(z3, - FIX_2_172734803) /* sqrt(2) * (-c1-c5) */
               + MULTIPLY(z4, FIX_1_061594337); /* sqrt(2) * (c5+c7) */

        tmp2 = MULTIPLY(z1, - FIX_0_509795579) /* sqrt(2) * (c7-c5) */
               + MULTIPLY(z2, - FIX_0_601344887) /* sqrt(2) * (c5-c1) */
               + MULTIPLY(z3, FIX_0_899976223) /* sqrt(2) * (c3-c7) */
               + MULTIPLY(z4, FIX_2_562915447); /* sqrt(2) * (c1+c3) */


        i = (int16) DESCALE(tmp10 + tmp2, CONST_BITS + PASS1_BITS + 3 + 1) + 128;
        clamp(i)
        Pdst_ptr[8*0] = (int16)i;

        i = (int16) DESCALE(tmp10 - tmp2, CONST_BITS + PASS1_BITS + 3 + 1) + 128;
        clamp(i)
        Pdst_ptr[8*3] = (int16)i;

        i = (int16) DESCALE(tmp12 + tmp0, CONST_BITS + PASS1_BITS + 3 + 1) + 128;
        clamp(i)
        Pdst_ptr[8*1] = (int16)i;

        i = (int16) DESCALE(tmp12 - tmp0, CONST_BITS + PASS1_BITS + 3 + 1) + 128;
        clamp(i)
        Pdst_ptr[8*2] = (int16)i;

        dataptr++;
        Pdst_ptr++;
    }
}


void JpgDecIdct2x2(int16 *data, int16 *Pdst_ptr)
{
#if 0
    int32 tmp0, tmp10, z1, j;
    register int16 *dataptr;
    int rowctr;
    int16 i;
    int16 dcval;

    dataptr = data;
#if 0
    for (j = 0;j < 64;j++)
    {
        if (j == 0 || j == 1 || j == 8 || j == 9)
            continue;
        else
            dataptr[j] = 0;
    }
    //fwrite(dataptr,sizeof(int16),64,IDCTInFile);
#endif
    for (rowctr = 8; rowctr > 0; rowctr--)
    {
        /* Don't bother to process columns 2,4,6 */
        if (rowctr == 6 || rowctr == 4 || rowctr == 2)
        {
            dataptr ++;
            continue;
        }
        if ((dataptr[8*1] | dataptr[8*3] | dataptr[8*5] | dataptr[8*7]) == 0)
        {
            dcval = (int16)(dataptr[8*0] << PASS1_BITS);

            dataptr[8*0] = dcval;
            dataptr[8*1] = dcval;

            dataptr ++;       /* advance pointer to next row */
            continue;
        }

        z1 = (int32) dataptr[0];
        tmp10 = z1 << (CONST_BITS + 2);


        z1 = (int32) dataptr[8*7];
        tmp0 = MULTIPLY(z1, - FIX_0_720959822); /* sqrt(2) * (c7-c5+c3-c1) */
        z1 = (int32) dataptr[8*5];
        tmp0 += MULTIPLY(z1, FIX_0_850430095); /* sqrt(2) * (-c1+c3+c5+c7) */
        z1 = (int32) dataptr[8*3];
        tmp0 += MULTIPLY(z1, - FIX_1_272758580); /* sqrt(2) * (-c1+c3-c5-c7) */
        z1 = (int32) dataptr[8*1];
        tmp0 += MULTIPLY(z1, FIX_3_624509785); /* sqrt(2) * (c1+c3+c5+c7) */


        dataptr[8*0] = (int) DESCALE(tmp10 + tmp0, CONST_BITS - PASS1_BITS + 2);
        dataptr[8*1] = (int) DESCALE(tmp10 - tmp0, CONST_BITS - PASS1_BITS + 2);


        dataptr ++;
    }

    dataptr = data;

    for (rowctr = 2; rowctr > 0; rowctr--)
    {



        if ((dataptr[1] | dataptr[3] | dataptr[5] |  dataptr[7]) == 0)
        {
            dcval = (int16) DESCALE((int32) dataptr[0], PASS1_BITS + 3);

            if ((dcval += 128) < 0)
                dcval = 0;
            else if (dcval > 255)
                dcval = 255;


            Pdst_ptr[0] = (int16)dcval;
            Pdst_ptr[1] = (int16)dcval;

//     DEBUG("  %3d   %3d   \n", Pdst_ptr[0], Pdst_ptr[1]);

            dataptr += 8;
            Pdst_ptr += 8;
            continue;
        }
        tmp10 = ((int32) dataptr[0]) << (CONST_BITS + 2);

        tmp0 = MULTIPLY((int32) dataptr[7], - FIX_0_720959822) /* sqrt(2) * (c7-c5+c3-c1) */
               + MULTIPLY((int32) dataptr[5], FIX_0_850430095) /* sqrt(2) * (-c1+c3+c5+c7) */
               + MULTIPLY((int32) dataptr[3], - FIX_1_272758580) /* sqrt(2) * (-c1+c3-c5-c7) */
               + MULTIPLY((int32) dataptr[1], FIX_3_624509785); /* sqrt(2) * (c1+c3+c5+c7) */


        i = (int16) DESCALE(tmp10 + tmp0, CONST_BITS + PASS1_BITS + 3 + 2) + 128;
        clamp(i)
        Pdst_ptr[0] = (int16)i;

        i = (int16) DESCALE(tmp10 - tmp0, CONST_BITS + PASS1_BITS + 3 + 2) + 128;
        clamp(i)
        Pdst_ptr[1] = (int16)i;

//    DEBUG("  %3d   %3d   \n", Pdst_ptr[0], Pdst_ptr[1]);

        dataptr += 8;
        Pdst_ptr += 8;
    }
#else
    {
        int32 tmp0, tmp10, z1;
        register int16 *dataptr;
        int rowctr;
        int16 i;
//        int16 dcval;

        dataptr = data;


        //column transform
        for (rowctr = 2; rowctr > 0; rowctr--)
        {

            tmp10 = ((int32)dataptr[8*0] << 14) + 2048;

            z1 = dataptr[8*1];

            tmp0 = MULTIPLY(z1, 14846); /* sqrt(2) * (c1+c3+c5+c7) */

            dataptr[8*0] = (int)((tmp10 + tmp0) >> 12);
            dataptr[8*1] = (int)((tmp10 - tmp0) >> 12);

            dataptr++;
        }

        dataptr = data;

        //row transform
        for (rowctr = 2; rowctr > 0; rowctr--)
        {
            tmp10 = (int32)(dataptr[0] + 0x1010) << 15;
            tmp0 = MULTIPLY((int32) dataptr[1], FIX_3_624509785);


            i = (int16)((tmp10 + tmp0) >> 20);
            if (i < 0)
                i = 0;
            else if (i > 255)
                i = 255;
            Pdst_ptr[0] = i;


            i = (int16)((tmp10 - tmp0) >> 20);
            if (i < 0)
                i = 0;
            else if (i > 255)
                i = 255;
            Pdst_ptr[1] = i;


            dataptr += 8;
            Pdst_ptr += 8;
        }
    }
#endif
}

void JpgDecIdct1x1(int16 *data, int16 *Pdst_ptr)
{
    int dcval;
    dcval = data[0];
	 dcval = (int) DESCALE(dcval, 3) + 128;
	 if (dcval < 0)
        dcval = 0;
     else if (dcval > 255)
        dcval = 255;
     Pdst_ptr[0] = dcval;
}
#pragma arm section code
#endif
