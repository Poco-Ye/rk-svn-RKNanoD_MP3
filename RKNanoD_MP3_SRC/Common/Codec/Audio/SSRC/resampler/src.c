//****************************************************************************
//
// SRC.C - Polyphase sample rate converter.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
#include <stdio.h>
#include "SysConfig.h"

//****************************************************************************
//
// The actual polyphase filters.  These filters can have any number of
// polyphases but must have NUMTAPS taps.
//
//****************************************************************************
#ifdef SSRC

#pragma arm section code ="SSRCCode", rodata = "SSRCCode", rwdata = "SSRCData", zidata = "SSRCData"
#include "src.h"

//#include "filt0181.h"
#include "filt0919.h"
#include "filt1088.h"
#include "filt1378.h"
//#include "filt1500.h"
#include "filt1838.h"
#include "filt2000.h"
//#include "filt2177.h"
#include "filt2756.h"
//#include "filt3000.h"
#include "filt3675.h"
#include "filt4000.h"
//#include "filt4354.h"
#include "filt5513.h"
//#include "Filt_48_44120.h"
//#include "Filt_48_44121.h"
#include "filt02297.h"
#include "filt0005.h"
#include "filt0025.h"
#include "filt04593.h"
#include "filt0689.h"



//static short Left_right[NUMTAPS*2];



//****************************************************************************
//
// SRCInit initializes the persistent state of the sample rate converter.  It
// must be called before SRCFilter or SRCFilter_S.	It is the responsibility
// of the caller to allocate the memory needed by the sample rate converter,
// for both the persistent state structure and the delay line(s).  The number
// of elements in the delay line array must be (Max input samples per
// SRCFilter call) + NUMTAPS (less will result in errors and more will go
// unused).
//
//****************************************************************************
int SRCInit(SRCState *pSRC, unsigned long ulInputRate, unsigned long ulOutputRate)
{
	long lNumPolyPhases, lSampleIncrement, lNumTaps;
	short sTap;

	//
	// Choose the polyphase filter based on the ratio of input rate to output
	// rate.
	//
	pSRC->last_sample_num = 0;
	pSRC->process_num = 160;
    #ifdef _A2DP_SINK_
	memset(pSRC->Last_sample,0,720); // BY WP 20140603
	#endif
	memset(pSRC->Left_right,0,NUMTAPS*4);

    //DEBUG("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^0X%x\n",(ulOutputRate << 16) / ulInputRate);
	switch((ulOutputRate << 16) / ulInputRate)
	{
		//
		// Ratio of ~0.1814 (44.1kHz -> 8kHz).
		//
//		case 0x00002E70:
//		{
//			lNumPolyPhases = SRCFilter_0_1814.sNumPolyPhases;
//			lNumTaps = SRCFilter_0_1814.sNumTaps;
//			lSampleIncrement = SRCFilter_0_1814.sSampleIncrement;
//			pSRC->psFilter = (short *)SRCFilter_0_1814.sCoefs;
//			break;
//		}

		//
		// Ratio of 0.91875 (48kHz -> 44.1kHz).
		//

		case 0x0000EB33:
		{
			lNumPolyPhases = SRCFilter_0_91875.sNumPolyPhases;//23
			lNumTaps = SRCFilter_0_91875.sNumTaps;//13
			lSampleIncrement = SRCFilter_0_91875.sSampleIncrement;//25
			pSRC->psFilter = (short *)SRCFilter_0_91875.sCoefs;
			break;
		}

		//
		// Ratio of ~1.0884 (44.1kHz -> 48kHz).
		//
//		case 0x000116A3:
//		{
//
//			lNumPolyPhases = SRCFilter_1_0884.sNumPolyPhases;
//			lNumTaps = SRCFilter_1_0884.sNumTaps;
//			lSampleIncrement = SRCFilter_1_0884.sSampleIncrement;
//			pSRC->psFilter = (short *)SRCFilter_1_0884.sCoefs;
//			break;
//		}

		//
		// Ratio of 1.378125 (32khz -> 44.1kHz).
		//
		case 0x000160CC:
		{
			lNumPolyPhases = SRCFilter_1_378125.sNumPolyPhases;
			lNumTaps = SRCFilter_1_378125.sNumTaps;
			lSampleIncrement = SRCFilter_1_378125.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_1_378125.sCoefs;
			break;
		}

		//
		// Ratio of 1.5 (32kHz -> 48kHz)
		//
//		case 0x00018000:
//		{
//			lNumPolyPhases = SRCFilter_1_5.sNumPolyPhases;
//			lNumTaps = SRCFilter_1_5.sNumTaps;
//			lSampleIncrement = SRCFilter_1_5.sSampleIncrement;
//			pSRC->psFilter = (short *)SRCFilter_1_5.sCoefs;
//			break;
//		}

		//
		// Ratio of 1.8375 (24kHz -> 44.1kHz).
		//
		case 0x0001D666:
		{
			lNumPolyPhases = SRCFilter_1_8375.sNumPolyPhases;
			lNumTaps = SRCFilter_1_8375.sNumTaps;
			lSampleIncrement = SRCFilter_1_8375.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_1_8375.sCoefs;
			break;
		}

		//
		// Ratio of 2.0 (22.05kHz -> 44.1kHz, 24kHz -> 48kHz).
		//
		case 0x00020000:
		{
			lNumPolyPhases = SRCFilter_2_0.sNumPolyPhases;
			lNumTaps = SRCFilter_2_0.sNumTaps;
			lSampleIncrement = SRCFilter_2_0.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_2_0.sCoefs;
			break;
		}

		//
		// Ratio of ~2.1769 (22.05kHz -> 48kHz).
		//
//		case 0x00022D47:
//		{
//			lNumPolyPhases = SRCFilter_2_1769.sNumPolyPhases;
//			lNumTaps = SRCFilter_2_1769.sNumTaps;
//			lSampleIncrement = SRCFilter_2_1769.sSampleIncrement;
//			pSRC->psFilter = (short *)SRCFilter_2_1769.sCoefs;
//			break;
//		}

		//
		// Ratio of 2.75625 (16kHz -> 44.1kHz).
		//
		case 0x0002C199:
		{
			lNumPolyPhases = SRCFilter_2_75625.sNumPolyPhases;
			lNumTaps = SRCFilter_2_75625.sNumTaps;
			lSampleIncrement = SRCFilter_2_75625.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_2_75625.sCoefs;
			break;
		}

		//
		// Ratio of 3.0 (16kHz -> 48kHz).
		//
//		case 0x00030000:
//		{
//			lNumPolyPhases = SRCFilter_3_0.sNumPolyPhases;
//			lNumTaps = SRCFilter_3_0.sNumTaps;
//			lSampleIncrement = SRCFilter_3_0.sSampleIncrement;
//			pSRC->psFilter = (short *)SRCFilter_3_0.sCoefs;
//			break;
//		}

		//
		// Ratio of 3.675 (12kHz -> 44.1kHz).
		//
		case 0x0003ACCC:
		{
			lNumPolyPhases = SRCFilter_3_675.sNumPolyPhases;
			lNumTaps = SRCFilter_3_675.sNumTaps;
			lSampleIncrement = SRCFilter_3_675.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_3_675.sCoefs;
			break;
		}

		//
		// Ratio of 4.0 (11.025kHz -> 44.1kHz, 12kHz -> 48kHz).
		//
		case 0x00040000:
		{
			lNumPolyPhases = SRCFilter_4_0.sNumPolyPhases;
			lNumTaps = SRCFilter_4_0.sNumTaps;
			lSampleIncrement = SRCFilter_4_0.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_4_0.sCoefs;
			break;
		}

		//
		// Ratio of ~4.3537 (11.025kHz -> 48kHz).
		//
//		case 0x00045A8E:
//		{
//			lNumPolyPhases = SRCFilter_4_3537.sNumPolyPhases;
//			lNumTaps = SRCFilter_4_3537.sNumTaps;
//			lSampleIncrement = SRCFilter_4_3537.sSampleIncrement;
//			pSRC->psFilter = (short *)SRCFilter_4_3537.sCoefs;
//			break;
//		}

		//
		// Ratio of 5.5125 (8kHz -> 44.1kHz).
		//
		case 0x00058333:
		{
			lNumPolyPhases = SRCFilter_5_5125.sNumPolyPhases;
			lNumTaps = SRCFilter_5_5125.sNumTaps;
			lSampleIncrement = SRCFilter_5_5125.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_5_5125.sCoefs;
			break;
		}
        //192k->44.1k
        case 0x00003ACC:
		{
           // DEBUG();
			lNumPolyPhases = SRCFilter_0_2297.sNumPolyPhases;
			lNumTaps = SRCFilter_0_2297.sNumTaps;
			lSampleIncrement = SRCFilter_0_2297.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_0_2297.sCoefs;
			break;
		}

        //88200->44100
        case 0x00008000:
        {
           // DEBUG();
			lNumPolyPhases = SRCFilter_0_5.sNumPolyPhases;
			lNumTaps = SRCFilter_0_5.sNumTaps;
			lSampleIncrement = SRCFilter_0_5.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_0_5.sCoefs;
			break;
		}

        //176400->44100
        case 0x00004000:
        {
           // DEBUG();
			lNumPolyPhases = SRCFilter_0_25.sNumPolyPhases;
			lNumTaps = SRCFilter_0_25.sNumTaps;
			lSampleIncrement = SRCFilter_0_25.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_0_25.sCoefs;
			break;
		}

        //96000->44100
        case 0x00007599:
        {
           // DEBUG();
			lNumPolyPhases = SRCFilter_0_45.sNumPolyPhases;
			lNumTaps = SRCFilter_0_45.sNumTaps;
			lSampleIncrement = SRCFilter_0_45.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_0_45.sCoefs;
			break;
		}

        //64000->44100
        case 0x0000b066:
        {
           // DEBUG();
			lNumPolyPhases = SRCFilter_0_689.sNumPolyPhases;
			lNumTaps = SRCFilter_0_689.sNumTaps;
			lSampleIncrement = SRCFilter_0_689.sSampleIncrement;
			pSRC->psFilter = (short *)SRCFilter_0_689.sCoefs;
			break;
		}



		//
		// Ratio of 6.0 (8kHz -> 48kHz).
		//
//		case 0x00060000:
//		{
//			lNumPolyPhases = SRCFilter_6_0.sNumPolyPhases;
//			lNumTaps = SRCFilter_6_0.sNumTaps;
//			lSampleIncrement = SRCFilter_6_0.sSampleIncrement;
//			pSRC->psFilter = (short *)SRCFilter_6_0.sCoefs;
//			break;
//		}

		//
		// An unsupported sample rate ratio was specified.	Return an error.
		//
		default:
		{
			return(0);
		}
	}

	//
	// Make sure that the number of taps in the filter matches the number of
	// taps supported by our filtering code.
	//
	if(lNumTaps != NUMTAPS)
	{
		return(0);
	}

	//
	// Initialize the persistent SRC state.
	//
	pSRC->lFilterOffset = 0;
	pSRC->lFilterIncr = lSampleIncrement * NUMTAPS;
	pSRC->lFilterSize = lNumPolyPhases * NUMTAPS;

	//
	// Set the initial state of the delay lines to silence.
	//
	for(sTap = 0; sTap < NUMTAPS; sTap++)
	{
		pSRC->Left_right[2*sTap] = 0;
		pSRC->Left_right[2*sTap + 1] = 0;
	}

	//
	// We've successfully initialized the requested sample rate converter.
	//
	return(1);
}


//****************************************************************************
//
// SRCFilterStereo runs the sample rate conversion filter over the given
// streams of samples, thereby performing the sample rate conversion requested
// by the initial call to SRCInit.	This function works on a pair of mono
// streams of samples.
//
//****************************************************************************
short lastSampleLeft, lastSampleRight;
void SRCFilterStereo(SRCState *pSRC, short *psInDataLeft, short *psInDataRight,
				short *psOutDataLeft, short *psOutDataRight,
				long lNumInputSamples, long *plNumOutputSamples)//lNumInputSamples为单声道长度
																//plNumOutputSamples为单声道输出
{
	register long lOutDataLeft, lOutDataRight;
	register short *psPtr1, *psPtr2, *psPtr3;
	short *psSampleLeft, *psSampleRight, sCoeff;
	int iLoop;
    int i;
	//
	// Append the new data to the filter memory buffers.


	 memcpy(psInDataLeft - (NUMTAPS << 1), pSRC->Left_right, (NUMTAPS << 2));
	//
	// Point to the last sample of the pre-existing audio data.
    #if 1
     i = 1;
     while (1)
     {
         psSampleLeft = psInDataLeft - 2*i;
         psSampleRight = psInDataRight - 2*i;
         i++;
         if ((*psSampleLeft == lastSampleLeft) && (*psSampleRight == lastSampleRight))
         {
            // printf("%d,%d  ",i-1,count);
            // fseek(in_pcm,-2*2*(i-1),SEEK_CUR);
             break;
         }

     }
#else
    psSampleLeft = psInDataLeft - 2;
    psSampleRight = psInDataRight - 2;
#endif
    //
    // Compute the number of output samples which we will produce.
    //
    iLoop = (((lNumInputSamples + i-1) * pSRC->lFilterSize) -
             pSRC->lFilterOffset - 1) / pSRC->lFilterIncr;
   // DEBUG("iLoop = %d",iLoop);
    *plNumOutputSamples = iLoop;

	// Loop through each output sample.
	//
	while(iLoop--)
	{
		//
		// Increment the offset into the filter.
		//
		pSRC->lFilterOffset += pSRC->lFilterIncr;

		//
		// See if we've passed the entire filter, indicating that we've
		// consumed one of the input samples.
		//
		while(pSRC->lFilterOffset >= pSRC->lFilterSize)
		{
			//
			// We have, so wrap the filter offset (i.e. treat the filter as if
			// it were a circular buffer in memory).
			//
			pSRC->lFilterOffset -= pSRC->lFilterSize;

			//
			// Consume the input sample.
			//
			psSampleLeft+=2;
			psSampleRight+=2;
		}

		//
		// Get pointers to the filter and the two sample data streams.
		//
		psPtr1 = pSRC->psFilter + pSRC->lFilterOffset;
		psPtr2 = psSampleLeft;
		psPtr3 = psSampleRight;

		//
		// Run the filter over the two sample streams.	The number of MACs here
		// must agree with NUMTAPS.
		//
	    lOutDataLeft = 0;
   		lOutDataRight = 0;
		sCoeff = *psPtr1++;
		i = 0;
	   while(i<NUMTAPS)
	   {

		lOutDataLeft += sCoeff * *psPtr2;
		lOutDataRight += sCoeff * *psPtr3;
		  if(i!=(NUMTAPS-1))
		  {
		sCoeff = *psPtr1++;
		psPtr2 -=2;
		psPtr3 -=2;

		  }
		   i++;

	   }
		//
		// Clip the filtered samples if necessary.
		//
		if((lOutDataLeft + 0x40000000) < 0)
		{
			lOutDataLeft = (lOutDataLeft & 0x80000000) ? 0xc0000000 :
														 0x3fffffff;
		}
		if((lOutDataRight + 0x40000000) < 0)
		{
			lOutDataRight = (lOutDataRight & 0x80000000) ? 0xc0000000 :
														   0x3fffffff;
		}

		//
		// Write out the samples.
		//
		*psOutDataLeft = (short)(lOutDataLeft >> 15);
		 psOutDataLeft += 2;
		*psOutDataRight = (short)(lOutDataRight >> 15);
		 psOutDataRight += 2;

	}

	//
	// Copy the tail of the filter memory buffer to the beginning, therefore
	// "remembering" the last few samples to use when processing the next batch
	// of samples.
	//
    lastSampleLeft = *psSampleLeft;
    lastSampleRight = *psSampleRight;
    memcpy(pSRC->Left_right,psInDataLeft + (lNumInputSamples<<1) -(NUMTAPS<<1),(NUMTAPS <<2));
}


//****************************************************************************
//
// SRCFilter runs the sample rate conversion filter over the given streams of
// samples, thereby performing the sample rate conversion requested
// by the initial call to SRCInit.
//long lNumSamples 双声道长度
// psInLeft指向buffer的26的位置
//****************************************************************************
long SRCFilter(SRCState *pSRC,short *psInLeft,short *psLeft, long lNumSamples)
{//lNumSamples 双声道short数据长度
	long lLength;
	long lNumOutputSamples;
	int i = 0;
	int out_length = 0;
    int in_length = 0;
    int process_num = pSRC->process_num;

    lLength = lNumSamples>>1;
	#ifdef _A2DP_SINK_
	lNumOutputSamples = 0;
	if (pSRC->last_sample_num)
	{
        psInLeft =psInLeft - (pSRC->last_sample_num<<1);
		memcpy(psInLeft,pSRC->Last_sample,(pSRC->last_sample_num<<2));
        lLength += pSRC->last_sample_num;
	}

//	for (i = 0;i<(lLength/process_num);i++)
//	{
//       SRCFilterStereo(pSRC, &psInLeft[(process_num<<1)*i], &psInLeft[(process_num<<1)*i+1], &psLeft[out_length<<1], &psLeft[(out_length<<1)+1],
//                        process_num, &lNumOutputSamples);
//		out_length +=lNumOutputSamples;
//	}

    in_length = lLength- (lLength%process_num);

    SRCFilterStereo(pSRC, psInLeft, &psInLeft[1], psLeft, &psLeft[1],
                        in_length, &lNumOutputSamples);
	out_length +=lNumOutputSamples;


    i = lLength/process_num;
	if (lLength%process_num)
	{
		pSRC->last_sample_num = lLength%process_num;
		memcpy(pSRC->Last_sample,&psInLeft[(process_num<<1)*i],(pSRC->last_sample_num<<2));
	}
	else
	{
        pSRC->last_sample_num = 0;
	}
    return (out_length<<1);//返回双声道长度
	#else
	SRCFilterStereo(pSRC, psInLeft, &psInLeft[1], &psLeft[0], &psLeft[1],
						lLength, &lNumOutputSamples);

	return (lNumOutputSamples<<1);//返回双声道长度
	#endif
}


#pragma arm section code

#endif


