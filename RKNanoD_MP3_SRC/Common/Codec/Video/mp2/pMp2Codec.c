


#include "pMp2Codec.h"

#ifdef MP2_INCLUDE

_ATTR_MP2DEC_TEXT_
unsigned long MP2Function(unsigned long ulIoctl, unsigned long ulParam1,
            unsigned long ulParam2, unsigned long ulParam3)
{
    switch (ulIoctl)
    {

        case MP2_CODEC_OPEN_DEC:
            {
                return mp2_open(1);
            }

        case MP2_CODEC_GETBUFFER:
            {
                mp2_get_buffer((short **)ulParam1,(int *)ulParam2);
                return 1;
            }

        case MP2_CODEC_DECODE:
            {
                return mp2_decode();
            }

		case MP2_CODEC_GETSAMPLERATE:
            {
				*(int *)ulParam1 = mp2_get_samplerate();
                return(1);
            }

        case MP2_CODEC_GETCHANNELS:
            {
				*(int *)ulParam1 = mp2_get_channels();
                return(1);
            }

		case MP2_CODEC_GETBITRATE:
            {
				*(int *)ulParam1 = mp2_get_bitrate();
                return(1);
            }

        case MP2_CODEC_GETLENGTH:
            {
                *(int *)ulParam1 = mp2_get_length();
				return 1;
            }

        case MP2_CODEC_GETTIME:
            {
               *(int *)ulParam1 = (long long) mp2_get_timepos() * 1000 / mp2_get_samplerate(); 
				return 1;
            }

        case MP2_CODEC_SEEK:
            {
				mp2_seek(ulParam1);                
                return 1;
            }

        case MP2_CODEC_CLOSE:
            {
                mp2_close();
                return 1;
            }

        default:
            {
                return 0;
            }
    }
	return -1;
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecOpen(unsigned long ulCodec, unsigned long ulFlags)
{
    unsigned long ulRet;
    
    // Pass the open request to the entry point for the codec.
    ulRet = MP2Function(MP2_CODEC_OPEN_DEC, 0, 0, 0);
    // Return the result to the caller.
    return(ulRet);
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecClose(void)
{
     MP2Function(MP2_CODEC_CLOSE, 0, 0, 0);
    // Return the result to the caller.
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecDecode(void)
{
    return(MP2Function(MP2_CODEC_DECODE, 0, 0, 0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecSeek(unsigned long ulTime, unsigned long ulSeekType)
{
    // Pass the seek request to the entry point for the specified codec.
    return(MP2Function(MP2_CODEC_SEEK, ulTime, ulSeekType,0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecGetTime(unsigned long *pulTime)
{
    // Pass the time request to the entry point for the specified codec.
    return(MP2Function(MP2_CODEC_GETTIME, (unsigned long)pulTime, 0, 0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecGetBitrate(unsigned long *pulBitrate)
{
    // Pass the bitrate request to the entry point for the specified codec.
    return(MP2Function(MP2_CODEC_GETBITRATE, (unsigned long)pulBitrate, 0, 0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecGetSampleRate(unsigned long *pulSampleRate)
{
    // Pass the sample rate request to the entry point for the specified codec.
    return(MP2Function(MP2_CODEC_GETSAMPLERATE, (unsigned long)pulSampleRate, 0, 0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecGetChannels(unsigned long *pulChannels)
{
    // Pass the channels request to the entry point for the specified codec.
    return(MP2Function(MP2_CODEC_GETCHANNELS, (unsigned long)pulChannels, 0, 0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecGetLength(unsigned long *pulLength)
{
    // Pass the length request to the entry point for the specified codec.
    return(MP2Function(MP2_CODEC_GETLENGTH, (unsigned long)pulLength, 0, 0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecSetBuffer(short* psBuffer)
{
    // Pass the set buffer request to the entry point for the specified codec.
    return(MP2Function(MP2_CODEC_SETBUFFER, (unsigned long)psBuffer, 0, 0));
}

_ATTR_MP2DEC_TEXT_
unsigned long Mp2CodecGetCaptureBuffer(short *ppsBuffer, long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    return(MP2Function(MP2_CODEC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)plLength, 0));
}

#endif

