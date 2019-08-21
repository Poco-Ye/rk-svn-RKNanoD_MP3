/*
 * coder.c
 *
 * 22/02/01
 * Calculation of coefficient tables for sub band windowing
 * analysis filters and mdct.
 */

#include  "SysInclude.h"
#include  "audio_main.h"

//#define  MP3_ENC_ORG

#ifdef MP3_ENC_INCLUDE2

#pragma arm section code = "EncodeMP3Code", rodata = "EncodeMP3Code", rwdata = "EncodeMP3Data", zidata = "EncodeMP3Bss"
#include "mp3_enc_types.h"

#include "mp3_enc_table1.h"


long int  x[2][HAN_SIZE], z[512];
int       off[2]; 

extern  long   mdct_freq[2][2][samp_per_frame2];  //576*4*4 = 9216  1232256
extern long   l3_sb_sample[2][3][18][SBLIMIT];   //32*6*18*4 = 13824

/*
 * L3_window_filter_subband:
 * -------------------------
 * Overlapping window on PCM samples
 * 32 16-bit pcm samples are scaled to fractional 2's complement and
 * concatenated to the end of the window buffer #x#. The updated window
 * buffer #x# is then windowed by the analysis window #enwindow# to produce
 * the windowed sample #z#
 * The windowed samples #z# is filtered by the digital filter matrix #filter#
 * to produce the subband samples #s#. This done by first selectively
 * picking out values from the windowed samples, and then multiplying
 * them by the filter matrix, producing 32 subband samples.
 */
void L3_window_filter_subband(unsigned long **buffer, long s[SBLIMIT] , int k)
{
 
  long y[64],s1,s2;
  int i,j;

    /* replace 32 oldest samples with 32 new samples */

    /* data format depends on mode */
    if(config.mpeg.channels == 1)   
    { /* mono data, use upper then lower */
        for (i=15;i>=0;i--)
        {
            x[k][(2*i)+off[k]+1] = (**buffer) << 16;
            x[k][(2*i)+off[k]] = ((*(*buffer)++) >> 16) << 16;
        }
    }
    else if(k)
    { /* stereo left, use upper */
        for (i=31;i>=0;i--)
        {
            x[k][i+off[k]] = (*(*buffer)++) & 0xffff0000;
        }
    }
    else
    { /* stereo right, use lower */
        for (i=31;i>=0;i--)
        {
            x[k][i+off[k]] = (*(*buffer)++) << 16;
        }
    }

    /* shift samples into proper window positions, and window data */
    for (i=HAN_SIZE; i--; )
    {
        z[i] = mul(x[k][(i+off[k])&(HAN_SIZE-1)],ew[i]);
    }
    
    off[k] = (off[k] + 480) & (HAN_SIZE-1); /* offset is modulo (HAN_SIZE)*/

    /* sub sample the windowed data */
    for (i=64; i--; )
    {
        for (j=8, y[i] = 0; j--; )
        {
            y[i] += z[i+(j<<6)];
        }
    }
    /* combine sub samples for the simplified matrix calculation */
    for (i=0; i<16; i++)
    {
        y[i+17] += y[15-i];
    }
    for (i=0; i<15; i++)
    {
        y[i+33] -= y[63-i];
    }

    /* simlplified polyphase filter matrix multiplication */
#ifdef MP3_ENC_ORG
    for (i=16; i--; )
    {
        for (j=0, s[i]= 0, s[31-i]=0; j<32; j += 2)
        {
            s1 = mul(fl[i][j],y[j+16]);
            s2 = mul(fl[i][j+1],y[j+17]);
            s[i] += s1 + s2;
            s[31-i] += s1 - s2;
        }
    }
#else

	for (i=16; i--; )
    {	
		s1 = (long)(((long long)fl[i][0] * (long long)y[16])>>32);
		s2 = (long)(((long long)fl[i][1] * (long long)y[17])>>32);
		s[i] = s1 + s2;
		s[31-i] = s1 - s2;
		
		s1 = (long)(((long long)fl[i][2] * (long long)y[18])>>32);
		s2 = (long)(((long long)fl[i][3] * (long long)y[19])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][4] * (long long)y[20])>>32);
		s2 = (long)(((long long)fl[i][5] * (long long)y[21])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][6] * (long long)y[22])>>32);
		s2 = (long)(((long long)fl[i][7] * (long long)y[23])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][8] * (long long)y[24])>>32);
		s2 = (long)(((long long)fl[i][9] * (long long)y[25])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][10] * (long long)y[26])>>32);
		s2 = (long)(((long long)fl[i][11] * (long long)y[27])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][12] * (long long)y[28])>>32);
		s2 = (long)(((long long)fl[i][13] * (long long)y[29])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][14] * (long long)y[30])>>32);
		s2 = (long)(((long long)fl[i][15] * (long long)y[31])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][16] * (long long)y[32])>>32);
		s2 = (long)(((long long)fl[i][17] * (long long)y[33])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][18] * (long long)y[34])>>32);
		s2 = (long)(((long long)fl[i][19] * (long long)y[35])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][20] * (long long)y[36])>>32);
		s2 = (long)(((long long)fl[i][21] * (long long)y[37])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][22] * (long long)y[38])>>32);
		s2 = (long)(((long long)fl[i][23] * (long long)y[39])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][24] * (long long)y[40])>>32);
		s2 = (long)(((long long)fl[i][25] * (long long)y[41])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][26] * (long long)y[42])>>32);
		s2 = (long)(((long long)fl[i][27] * (long long)y[43])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][28] * (long long)y[44])>>32);
		s2 = (long)(((long long)fl[i][29] * (long long)y[45])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;
		
		s1 = (long)(((long long)fl[i][30] * (long long)y[46])>>32);
		s2 = (long)(((long long)fl[i][31] * (long long)y[47])>>32);
		s[i] += s1 + s2;
		s[31-i] += s1 - s2;		
	}
#endif
}

/*
 * L3_mdct_sub:
 * ------------
 */

 #if 1
 void L3_mdct_sub()   
 #else
void L3_mdct_sub(long sb_sample[2][3][18][SBLIMIT],
                 long mdct_freq[2][2][samp_per_frame2])

 #endif 
{
    /* note. we wish to access the array 'mdct_freq[2][2][576]' as
    * [2][2][32][18]. (32*18=576),
    */
    long (*mdct_enc)[18];

    int  ch,gr,band,j,k;
    long mdct_in[36];
    long bu,bd,*m;

    for(gr=0; gr<config.mpeg.granules; gr++)
    {
        for(ch=config.mpeg.channels; ch--; )
        {
            /* set up pointer to the part of mdct_freq we're using */
            mdct_enc = (long (*)[18]) mdct_freq[gr][ch];

            //Compensate for inversion in the analysis filter
            // (every odd index of band AND k)
            for(band=1; band<=31; band+=2 )
            {
                for(k=1; k<=17; k+=2 )
                {
                    l3_sb_sample[ch][gr+1][k][band] *= -1;
                }
            }
            /* Perform imdct of 18 previous subband samples + 18 current subband samples */
            for(band=32; band--; )
            {
                for(k=18; k--; )
                {
                    mdct_in[k]    = l3_sb_sample[ch][ gr ][k][band];
                    mdct_in[k+18] = l3_sb_sample[ch][gr+1][k][band];
                }

                // Calculation of the MDCT
                // In the case of long blocks ( block_type 0,1,3 ) there are
                // 36 coefficients in the time domain and 18 in the frequency
                // domain.
                for(k=18; k--; )
                {
                    m = &mdct_enc[band][k];
#ifdef MP3_ENC_ORG
                    for(j=36, *m=0; j--; )
                    {
                        *m += mul(mdct_in[j],cos_l[k][j]);
                    }
#else          
                    *m =  (long)(((long long)mdct_in[35] * (long long)cos_l[k][35])>>32);
                    *m += (long)(((long long)mdct_in[34] * (long long)cos_l[k][34])>>32);
                    *m += (long)(((long long)mdct_in[33] * (long long)cos_l[k][33])>>32);
                    *m += (long)(((long long)mdct_in[32] * (long long)cos_l[k][32])>>32);
                    *m += (long)(((long long)mdct_in[31] * (long long)cos_l[k][31])>>32);
                    *m += (long)(((long long)mdct_in[30] * (long long)cos_l[k][30])>>32);

                    *m += (long)(((long long)mdct_in[29] * (long long)cos_l[k][29])>>32);
                    *m += (long)(((long long)mdct_in[28] * (long long)cos_l[k][28])>>32);
                    *m += (long)(((long long)mdct_in[27] * (long long)cos_l[k][27])>>32);
                    *m += (long)(((long long)mdct_in[26] * (long long)cos_l[k][26])>>32);
                    *m += (long)(((long long)mdct_in[25] * (long long)cos_l[k][25])>>32);
                    *m += (long)(((long long)mdct_in[24] * (long long)cos_l[k][24])>>32);
                    *m += (long)(((long long)mdct_in[23] * (long long)cos_l[k][23])>>32);
                    *m += (long)(((long long)mdct_in[22] * (long long)cos_l[k][22])>>32);
                    *m += (long)(((long long)mdct_in[21] * (long long)cos_l[k][21])>>32);
                    *m += (long)(((long long)mdct_in[20] * (long long)cos_l[k][20])>>32);

                    *m += (long)(((long long)mdct_in[19] * (long long)cos_l[k][19])>>32);
                    *m += (long)(((long long)mdct_in[18] * (long long)cos_l[k][18])>>32);
                    *m += (long)(((long long)mdct_in[17] * (long long)cos_l[k][17])>>32);
                    *m += (long)(((long long)mdct_in[16] * (long long)cos_l[k][16])>>32);
                    *m += (long)(((long long)mdct_in[15] * (long long)cos_l[k][15])>>32);
                    *m += (long)(((long long)mdct_in[14] * (long long)cos_l[k][14])>>32);
                    *m += (long)(((long long)mdct_in[13] * (long long)cos_l[k][13])>>32);
                    *m += (long)(((long long)mdct_in[12] * (long long)cos_l[k][12])>>32);
                    *m += (long)(((long long)mdct_in[11] * (long long)cos_l[k][11])>>32);
                    *m += (long)(((long long)mdct_in[10] * (long long)cos_l[k][10])>>32);

                    *m += (long)(((long long)mdct_in[ 9] * (long long)cos_l[k][ 9])>>32);
                    *m += (long)(((long long)mdct_in[ 8] * (long long)cos_l[k][ 8])>>32);
                    *m += (long)(((long long)mdct_in[ 7] * (long long)cos_l[k][ 7])>>32);
                    *m += (long)(((long long)mdct_in[ 6] * (long long)cos_l[k][ 6])>>32);
                    *m += (long)(((long long)mdct_in[ 5] * (long long)cos_l[k][ 5])>>32);
                    *m += (long)(((long long)mdct_in[ 4] * (long long)cos_l[k][ 4])>>32);
                    *m += (long)(((long long)mdct_in[ 3] * (long long)cos_l[k][ 3])>>32);
                    *m += (long)(((long long)mdct_in[ 2] * (long long)cos_l[k][ 2])>>32);
                    *m += (long)(((long long)mdct_in[ 1] * (long long)cos_l[k][ 1])>>32);
                    *m += (long)(((long long)mdct_in[ 0] * (long long)cos_l[k][ 0])>>32);
#endif
                }
            }

            /* Perform aliasing reduction butterfly */
            for(band=31; band--; )
            {
#ifdef MP3_ENC_ORG
                for(k=8; k--; )
                {
                    // must left justify result of multiplication here because the centre
                    // two values in each block are not touched.
                    bu = muls(mdct_enc[band][17-k],cs_enc[k]) + muls(mdct_enc[band+1][k],ca_enc[k]);
                    bd = muls(mdct_enc[band+1][k],cs_enc[k]) - muls(mdct_enc[band][17-k],ca_enc[k]);
                    mdct_enc[band][17-k] = bu;
                    mdct_enc[band+1][k]  = bd;
                }
#else
                bd =  (long)(((long long)mdct_enc[band+1][7]  * (long long)cs_enc[7])>>31);
                bd -= (long)(((long long)mdct_enc[band][10]   * (long long)ca_enc[7])>>31);

                bu =  (long)(((long long)mdct_enc[band][10]   * (long long)cs_enc[7])>>31);
                bu += (long)(((long long)mdct_enc[band+1][7]  * (long long)ca_enc[7])>>31);

                mdct_enc[band][10] = bu;
                mdct_enc[band+1][7]  = bd;

                bd =  (long)(((long long)mdct_enc[band+1][6]  * (long long)cs_enc[6])>>31);
                bd -= (long)(((long long)mdct_enc[band][11]   * (long long)ca_enc[6])>>31);

                bu =  (long)(((long long)mdct_enc[band][11]   * (long long)cs_enc[6])>>31);
                bu += (long)(((long long)mdct_enc[band+1][6]  * (long long)ca_enc[6])>>31);

                mdct_enc[band][11] = bu;
                mdct_enc[band+1][6]  = bd;

                bd =  (long)(((long long)mdct_enc[band+1][5]  * (long long)cs_enc[5])>>31);
                bd -= (long)(((long long)mdct_enc[band][12]   * (long long)ca_enc[5])>>31);

                bu =  (long)(((long long)mdct_enc[band][12]   * (long long)cs_enc[5])>>31);
                bu += (long)(((long long)mdct_enc[band+1][5]  * (long long)ca_enc[5])>>31);

                mdct_enc[band][12] = bu;
                mdct_enc[band+1][5]  = bd;

                bd =  (long)(((long long)mdct_enc[band+1][4]  * (long long)cs_enc[4])>>31);
                bd -= (long)(((long long)mdct_enc[band][13]   * (long long)ca_enc[4])>>31);

                bu =  (long)(((long long)mdct_enc[band][13]   * (long long)cs_enc[4])>>31);
                bu += (long)(((long long)mdct_enc[band+1][4]  * (long long)ca_enc[4])>>31);

                mdct_enc[band][13] = bu;
                mdct_enc[band+1][4]  = bd;

                bd =  (long)(((long long)mdct_enc[band+1][3]  * (long long)cs_enc[3])>>31);
                bd -= (long)(((long long)mdct_enc[band][14]   * (long long)ca_enc[3])>>31);

                bu =  (long)(((long long)mdct_enc[band][14]   * (long long)cs_enc[3])>>31);
                bu += (long)(((long long)mdct_enc[band+1][3]  * (long long)ca_enc[3])>>31);

                mdct_enc[band][14] = bu;
                mdct_enc[band+1][3]  = bd;

                bd =  (long)(((long long)mdct_enc[band+1][2]  * (long long)cs_enc[2])>>31);
                bd -= (long)(((long long)mdct_enc[band][15]   * (long long)ca_enc[2])>>31);

                bu =  (long)(((long long)mdct_enc[band][15]   * (long long)cs_enc[2])>>31);
                bu += (long)(((long long)mdct_enc[band+1][2]  * (long long)ca_enc[2])>>31);

                mdct_enc[band][15] = bu;
                mdct_enc[band+1][2]  = bd;

                bd =  (long)(((long long)mdct_enc[band+1][1]  * (long long)cs_enc[1])>>31);
                bd -= (long)(((long long)mdct_enc[band][16]   * (long long)ca_enc[1])>>31);

                bu =  (long)(((long long)mdct_enc[band][16]   * (long long)cs_enc[1])>>31);
                bu += (long)(((long long)mdct_enc[band+1][1]  * (long long)ca_enc[1])>>31);

                mdct_enc[band][16] = bu;
                mdct_enc[band+1][1]  = bd;

                bd =  (long)(((long long)mdct_enc[band+1][0]  * (long long)cs_enc[0])>>31);
                bd -= (long)(((long long)mdct_enc[band][17]   * (long long)ca_enc[0])>>31);

                bu =  (long)(((long long)mdct_enc[band][17]   * (long long)cs_enc[0])>>31);
                bu += (long)(((long long)mdct_enc[band+1][0]  * (long long)ca_enc[0])>>31);

                mdct_enc[band][17] = bu;
                mdct_enc[band+1][0]  = bd;
#endif
            }
        }
    }

    /* Save latest granule's subband samples to be used in the next mdct call */
    for(ch=config.mpeg.channels ;ch--; )
    {
        for(j=18; j--; )
        {
            for(band=32; band--; )
            {
                l3_sb_sample[ch][0][j][band] = l3_sb_sample[ch][config.mpeg.granules][j][band];
            }
        }
    }
}


#pragma arm section code
#endif

