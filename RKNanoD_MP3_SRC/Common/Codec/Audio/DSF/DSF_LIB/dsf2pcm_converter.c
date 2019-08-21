#include "SysInclude.h"
#include "audio_main.h"
#include "audio_file_access.h"

#ifdef A_CORE_DECODE
#ifdef DSF_DEC_INCLUDE

#include <stdio.h>
#pragma arm section code = "DsfDecCode", rodata = "DsfDecCode", rwdata = "DsfDecData", zidata = "DsfDecBss"


#include<string.h>
#include <math.h>
#include <stdio.h>
#include "dsf2pcm_conv.h"
#include "endianess.h"
#include"my.h"
#include<stdbool.h>
//#include<malloc.h>
/*
* The 435-tap symmetric lowpass filter: 20 kHz, 140 dB
*/
_ATTR_DSFDEC_DATA_
#if 1
const static int32_t htaps_315[] ={//FIR order = 512��?winodws:Chebyshe
-41, -37, -55, -78, -107,
    -144, -190, -247, -316, -400,
    -501, -621, -764, -931, -1127,
    -1354, -1615, -1916, -2259, -2648,
    -3087, -3581, -4133, -4748, -5428,
    -6178, -7000, -7897, -8870, -9920,
    -11048, -12252, -13530, -14877, -16289,
    -17755, -19267, -20812, -22372, -23930,
    -25462, -26941, -28337, -29613, -30729,
    -31637, -32285, -32615, -32561, -32049,
    -30999, -29322, -26921, -23689, -19512,
    -14263, -7808, 0, 9317, 20311,
    33160, 48055, 65197, 84800, 107089,
    132300, 160682, 192494, 228008, 267504,
    311275, 359624, 412862, 471309, 535295,
    605154, 681230, 763871, 853427, 950255,
    1054711, 1167153, 1287937, 1417418, 1555946,
    1703865, 1861512, 2029214, 2207289, 2396041,
    2595758, 2806712, 3029156, 3263324, 3509424,
    3767642, 4038135, 4321034, 4616438, 4924413,
    5244991, 5578171, 5923910, 6282131, 6652714,
    7035497, 7430277, 7836807, 8254796, 8683908,
    9123760, 9573927, 10033934, 10503263, 10981351,
    11467588, 11961321, 12461855, 12968452, 13480333,
    13996681, 14516642, 15039326, 15563811, 16089142,
    16614341, 17138399, 17660289, 18178963, 18693358,
    19202399, 19705000, 20200071, 20686521, 21163260,
    21629204, 22083280, 22524427, 22951603, 23363786,
    23759982, 24139222, 24500574, 24843139, 25166059,
    25468521, 25749755, 26009044, 26245720, 26459173,
    26648848, 26814251, 26954950, 27070576, 27160825,
    27225459, 27264308, 27277269, 27264308, 27225459,
    27160825, 27070576, 26954950, 26814251, 26648848,
    26459173, 26245720, 26009044, 25749755, 25468521,
    25166059, 24843139, 24500574, 24139222, 23759982,
    23363786, 22951603, 22524427, 22083280, 21629204,
    21163260, 20686521, 20200071, 19705000, 19202399,
    18693358, 18178963, 17660289, 17138399, 16614341,
    16089142, 15563811, 15039326, 14516642, 13996681,
    13480333, 12968452, 12461855, 11961321, 11467588,
    10981351, 10503263, 10033934, 9573927, 9123760,
    8683908, 8254796, 7836807, 7430277, 7035497,
    6652714, 6282131, 5923910, 5578171, 5244991,
    4924413, 4616438, 4321034, 4038135, 3767642,
    3509424, 3263324, 3029156, 2806712, 2595758,
    2396041, 2207289, 2029214, 1861512, 1703865,
    1555946, 1417418, 1287937, 1167153, 1054711,
    950255, 853427, 763871, 681230, 605154,
    535295, 471309, 412862, 359624, 311275,
    267504, 228008, 192494, 160682, 132300,
    107089, 84800, 65197, 48055, 33160,
    20311, 9317, 0, -7808, -14263,
    -19512, -23689, -26921, -29322, -30999,
    -32049, -32561, -32615, -32285, -31637,
    -30729, -29613, -28337, -26941, -25462,
    -23930, -22372, -20812, -19267, -17755,
    -16289, -14877, -13530, -12252, -11048,
    -9920, -8870, -7897, -7000, -6178,
    -5428, -4748, -4133, -3581, -3087,
    -2648, -2259, -1916, -1615, -1354,
    -1127, -931, -764, -621, -501,
    -400, -316, -247, -190, -144,
    -107, -78, -55, -37, -41
    };
const static int32_t htaps_20000[] = {//cheby
19080, 22079, 34559, 51432, 73712,
    102557, 139274, 185326, 242334, 312076,
    396490, 497669, 617851, 759411, 924852,
    1116779, 1337890, 1590945, 1878744, 2204093,
    2569778, 2978521, 3432952, 3935561, 4488661,
    5094346, 5754445, 6470483, 7243637, 8074696,
    8964024, 9911520, 10916592, 11978126, 13094459,
    14263370, 15482060, 16747150, 18054680, 19400120,
    20778383, 22183849, 23610394, 25051426, 26499934,
    27948533, 29389526, 30814962, 32216708, 33586520,
    34916116, 36197256, 37421822, 38581897, 39669843,
    40678382, 41600671, 42430370, 43161715, 43789576,
    44309513, 44717826, 45011592, 45188703, 45247882,
    45188703, 45011592, 44717826, 44309513, 43789576,
    43161715, 42430370, 41600671, 40678382, 39669843,
    38581897, 37421822, 36197256, 34916116, 33586520,
    32216708, 30814962, 29389526, 27948533, 26499934,
    25051426, 23610394, 22183849, 20778383, 19400120,
    18054680, 16747150, 15482060, 14263370, 13094459,
    11978126, 10916592, 9911520, 8964024, 8074696,
    7243637, 6470483, 5754445, 5094346, 4488661,
    3935561, 3432952, 2978521, 2569778, 2204093,
    1878744, 1590945, 1337890, 1116779, 924852,
    759411, 617851, 497669, 396490, 312076,
    242334, 185326, 139274, 102557, 73712,
    51432, 34559, 22079, 19080
};
#else
const static int32_t htaps_20000[] = {
    151, 103, 138, 180, 230, 289,
    358, 437, 529, 635, 754, 889,
    1041, 1210, 1398, 1605, 1833, 2081,
    2352, 2644, 2959, 3296, 3655, 4034,
    4434, 4852, 5286, 5734, 6193, 6658,
    7126, 7591, 8047, 8487, 8903, 9285,
    9625, 9910, 10128, 10265, 10306, 10234,
    10032, 9679, 9154, 8434, 7495, 6310,
    4851, 3089, 990, -1478, -4352, -7667,
    -11464, -15783, -20668, -26161, -32310, -39161,
    -46764, -55169, -64428, -74591, -85714, -97850,
    -111053, -125379, -140881, -157616, -175636, -194995,
    -215746, -237939, -261623, -286846, -313652, -342083,
    -372176, -403967, -437487, -472760, -509808, -548646,
    -589284, -631725, -675964, -721990, -769784, -819319,
    -870557, -923452, -977950, -1033984, -1091478, -1150344,
    -1210483, -1271784, -1334124, -1397367, -1461365, -1525956,
    -1590965, -1656204, -1721471, -1786550, -1851211, -1915211,
    -1978293, -2040186, -2100605, -2159253, -2215819, -2269979,
    -2321397, -2369726, -2414605, -2455665, -2492525, -2524795,
    -2552077, -2573962, -2590037, -2599882, -2603071, -2599176,
    -2587762, -2568397, -2540644, -2504070, -2458242, -2402731,
    -2337112, -2260967, -2173885, -2075464, -1965314, -1843055,
    -1708322, -1560765, -1400051, -1225863, -1037907, -835910,
    -619619, -388807, -143275, 117153, 392621, 683246,
    989111, 1310270, 1646742, 1998510, 2365525, 2747700,
    3144911, 3556996, 3983755, 4424951, 4880305, 5349501,
    5832182, 6327952, 6836376, 7356979, 7889248, 8432630,
    8986537, 9550340, 10123377, 10704949, 11294325, 11890737,
    12493391, 13101457, 13714081, 14330380, 14949445, 15570348,
    16192134, 16813835, 17434461, 18053011, 18668469, 19279813,
    19886009, 20486022, 21078812, 21663342, 22238577, 22803486,
    23357049, 23898255, 24426109, 24939630, 25437857, 25919851,
    26384697, 26831506, 27259419, 27667609, 28055282, 28421680,
    28766083, 29087812, 29386229, 29660743, 29910805, 30135915,
    30335622, 30509524, 30657271, 30778564, 30873159, 30940864,
    30981542, 30995111, 30981542, 30940864, 30873159, 30778564,
    30657271, 30509524, 30335622, 30135915, 29910805, 29660743,
    29386229, 29087812, 28766083, 28421680, 28055282, 27667609,
    27259419, 26831506, 26384697, 25919851, 25437857, 24939630,
    24426109, 23898255, 23357049, 22803486, 22238577, 21663342,
    21078812, 20486022, 19886009, 19279813, 18668469, 18053011,
    17434461, 16813835, 16192134, 15570348, 14949445, 14330380,
    13714081, 13101457, 12493391, 11890737, 11294325, 10704949,
    10123377, 9550340, 8986537, 8432630, 7889248, 7356979,
    6836376, 6327952, 5832182, 5349501, 4880305, 4424951,
    3983755, 3556996, 3144911, 2747700, 2365525, 1998510,
    1646742, 1310270, 989111, 683246, 392621, 117153,
    -143275, -388807, -619619, -835910, -1037907, -1225863,
    -1400051, -1560765, -1708322, -1843055, -1965314, -2075464,
    -2173885, -2260967, -2337112, -2402731, -2458242, -2504070,
    -2540644, -2568397, -2587762, -2599176, -2603071, -2599882,
    -2590037, -2573962, -2552077, -2524795, -2492525, -2455665,
    -2414605, -2369726, -2321397, -2269979, -2215819, -2159253,
    -2100605, -2040186, -1978293, -1915211, -1851211, -1786550,
    -1721471, -1656204, -1590965, -1525956, -1461365, -1397367,
    -1334124, -1271784, -1210483, -1150344, -1091478, -1033984,
    -977950, -923452, -870557, -819319, -769784, -721990,
    -675964, -631725, -589284, -548646, -509808, -472760,
    -437487, -403967, -372176, -342083, -313652, -286846,
    -261623, -237939, -215746, -194995, -175636, -157616,
    -140881, -125379, -111053, -97850, -85714, -74591,
    -64428, -55169, -46764, -39161, -32310, -26161,
    -20668, -15783, -11464, -7667, -4352, -1478,
    990, 3089, 4851, 6310, 7495, 8434,
    9154, 9679, 10032, 10234, 10306, 10265,
    10128, 9910, 9625, 9285, 8903, 8487,
    8047, 7591, 7126, 6658, 6193, 5734,
    5286, 4852, 4434, 4034, 3655, 3296,
    2959, 2644, 2352, 2081, 1833, 1605,
    1398, 1210, 1041, 889, 754, 635,
    529, 437, 358, 289, 230, 180,
    138, 103, 151
};
#endif




//#define HTAPS 129                     /* number of FIR constants */
//#define CTABLES ((HTAPS + 7) / 8)

static int ctables[256][60];


static int preinit = 0;

static dsd2pcm_converter_t dsd2pcm_saved;
static int conv_saved = 0;
//FILE * re;
//_ATTR_HIFI_DFFDEC_TEXT_
/*It's an implementation of a symmetric 435-taps FIR lowpass filter
optimized for DSD inputs. */
_ATTR_DSFDEC_TEXT_
static int ctable_len = 0;
static void dsd2pcm_converter_preinit(const int32_t* htaps,int order) {
    int ct, i, j, k;
    int coef;
    ctable_len = (order + 7)/8;
    //dsf_printf("%d\n",ctable_len);
    //  fopen_s(&re, "F:\\��Ƶ\\result.txt", "w");
    for (ct = 0; ct < ctable_len; ct++) {
       // dsf_printf("%d\n",CTABLES);
        k = order - ct * 8;
        if (k > 8)
            k = 8;
        for (i = 0; i < 256; i++) {
            coef = 0;
            for (j = 0; j < k; j++) {
                coef += (((i >> j) & 1) * 2 - 1) * htaps[ct * 8 + j];
            }
            ctables[i][ct] = coef;
        }
    }
}


extern Dec_Type DEC_DSF;
_ATTR_DSFDEC_TEXT_
void dsd2pcm_converter_init(dsd2pcm_converter_t* dsd2pcm, int channels, int dsd_samplerate, int pcm_samplerate) {
    int ch, i;

    if (conv_saved) {
        conv_saved = 0;
        if (channels == dsd2pcm_saved.channels && dsd_samplerate == dsd2pcm_saved.dsd_samplerate && pcm_samplerate == dsd2pcm_saved.pcm_samplerate) {
            memcpy(dsd2pcm, &dsd2pcm_saved, sizeof(dsd2pcm_converter_t));
            return;
        }
    }
    if (preinit != pcm_samplerate) {
        switch (DEC_DSF) {
        case DSD128:  //5.6M || 11.2M
        case DSD256:
            dsd2pcm_converter_preinit(htaps_315,315);
            break;
        default:
            dsd2pcm_converter_preinit(htaps_20000,129);
            break;
        }
        preinit = pcm_samplerate;
    }
    dsd2pcm->channels = channels;
    dsd2pcm->dsd_samplerate = dsd_samplerate;
    dsd2pcm->pcm_samplerate = pcm_samplerate;
    dsd2pcm->decimation = dsd_samplerate / pcm_samplerate>>3;
    for (ch = 0; ch < dsd2pcm->channels; ch++) {
        for (i = 0; i < FIFOSIZE; i++) {
            dsd2pcm->ch[ch].fifo[i] = 0x69;
        }
        dsd2pcm->ch[ch].fifopos = FIFOMASK;
    }
    dsd2pcm_converter_set_gain(dsd2pcm, (real_t)0);
}

_ATTR_DSFDEC_TEXT_
int dsd2pcm_converter_convert(dsd2pcm_converter_t* dsd2pcm, uint8_t* dsd_data, uint32_t dsd_size, uint8* pcm_out, uint16_t bps) {
    int ch, i, ct;
    uint8_t dsd_byte;
    int pcm_sample;
    int pcm_samples;
   // uint8* pcm_ptr;
    int* pcm_ptr;
    int32 tmp;
    real_t pcm_tmp;
    uint8_t* ppcm_vol;
    int vol_temp;
    int channels = dsd2pcm->channels;

    pcm_samples = dsd_size / dsd2pcm->decimation;
//  for (ch = 0; ch < 1; ch++){
    for (ch = 0; ch < channels; ch++) {

        uint8_t * fifo = dsd2pcm->ch[ch].fifo;
        uint32_t fifopos = dsd2pcm->ch[ch].fifopos;

        /* setup pcm pointer */
        pcm_ptr =(int*)( pcm_out + ch * (bps >> 3));
       //pcm_ptr =(uint8_t*) pcm_out + ch * (bps >> 3);
       /* if(((uint32)pcm_out)&0x03!=0){

            dsf_printf("0x%x",pcm_out);
            DelayMs2(100);
            while(1);
        }*/

        /* filter one channel */
        for (i = 0; i < dsd_size / channels; i++) {
//      for (i = 0; i < dsd_size ; i++) {
            /* get incoming DSD byte */
//          dsd_byte = dsd_data[i * dsd2pcm->channels + ch];
            dsd_byte = dsd_data[i + ch * dsd_size/ channels];
            /* put incoming DSD byte into queue, advance queue */
            fifopos = (fifopos + 1) & FIFOMASK;
            fifo[fifopos] = dsd_byte;

            /* filter out on each pcm sample */
            if ((i % dsd2pcm->decimation) == 0) {
                pcm_sample = 0;
               // for (ct = 0; ct < CTABLES; ct++) {
                for (ct = 0; ct < ctable_len; ct++) {
                    dsd_byte = fifo[(fifopos + ct) & FIFOMASK];
                    pcm_sample += ctables[dsd_byte][ct];
                }
               // pcm_tmp = dsd2pcm->gain * (real_t)pcm_sample;

#if 1           //  if (32 == bps)
                {//24bit
                    //tmp = (int)(2147483648.0f * pcm_tmp);
                    //tmp = MAX(tmp, -2147483648); // CLIP < 32768
                    //tmp = MIN(tmp, 2147483647);  // CLIP > 32767
                    //vol_temp = tmp;
                    pcm_sample = MAX(pcm_sample, -2147483648); // CLIP < 32768
                    pcm_sample = MIN(pcm_sample, 2147483647);  // CLIP > 32767

                    *pcm_ptr++ = pcm_sample;
                    ppcm_vol = (uint8*)&pcm_sample;
                    ////*pcm_ptr++ = ppcm_vol[0];
                    //*pcm_ptr++ = ppcm_vol[1];
                    //*pcm_ptr++ = ppcm_vol[2];
                    //*pcm_ptr++ = ppcm_vol[3];
                    //printf("  on 24 bps");
                }
#else//16bit
                //else if (16 == bps)
                {//16bit
                    tmp = (int)(32768.0f * pcm_tmp);
                    tmp = MAX(tmp, -32768); // CLIP < 32768
                    tmp = MIN(tmp, 32767);  // CLIP > 32767
                    vol_temp = (short)tmp;
                    ppcm_vol = (uint8*)&vol_temp;

                    *pcm_ptr++ = ppcm_vol[0];
                    *pcm_ptr++ = ppcm_vol[1];
                }
#endif
#if 0
            //  else // 8bit
                {// add othcer bps
                    printf("  no valid bps");
                }
#endif
                //pcm_ptr += (bps >> 3);
                pcm_ptr ++;
            }
            dsd2pcm->ch[ch].fifopos = fifopos  ;
        }
    }

    return pcm_samples;
}

_ATTR_DSFDEC_TEXT_
void dsd2pcm_converter_set_gain(dsd2pcm_converter_t* dsd2pcm, real_t dB_gain) {
    dsd2pcm->gain = (real_t)(4.66e-10 * sqrt(pow(10.0, dB_gain / 20.0)));
}

_ATTR_DSFDEC_TEXT_
void dsd2pcm_converter_save(dsd2pcm_converter_t* dsd2pcm) {
    memcpy(&dsd2pcm_saved, dsd2pcm, sizeof(dsd2pcm_converter_t));
    conv_saved = 1;
}

#endif
#endif
