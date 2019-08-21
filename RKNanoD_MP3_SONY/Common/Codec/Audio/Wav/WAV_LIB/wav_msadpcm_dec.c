#include        <stdio.h>
//#include        <string.h>

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef WAV_DEC_INCLUDE

#include "../include/audio_file_access.h"

extern FILE *pRawFileCache;
FILE *fOut;


//----------------------------------
//typedef int sf_count_t;
//#include		<math.h>

#include "sf_wav.h"

_ATTR_WAVDEC_BSS_																						   
SF_PRIVATE sf;

#define MSADPCM_ADAPT_COEFF_COUNT       7
//----------------------------------


/* These required here because we write the header in this file. */

#define RIFF_MARKER     (MAKE_MARKER ('R', 'I', 'F', 'F'))
#define WAVE_MARKER     (MAKE_MARKER ('W', 'A', 'V', 'E'))
#define fmt_MARKER      (MAKE_MARKER ('f', 'm', 't', ' '))
#define fact_MARKER     (MAKE_MARKER ('f', 'a', 'c', 't'))
#define data_MARKER     (MAKE_MARKER ('d', 'a', 't', 'a'))

#define WAVE_FORMAT_MS_ADPCM    0x0002

typedef struct
{
        int                 channels, blocksize, samplesperblock, blocks, dataremaining ;
        int                 blockcount ;
        sf_count_t          samplecount ;
        short				*samples ;
        unsigned char		*block ;
        unsigned char		dummydata [4] ; /* Dummy size */
} MSADPCM_PRIVATE ;

/*============================================================================================
** MS ADPCM static data and functions.
*/
_ATTR_WAVDEC_DATA_
static int AdaptationTable []    =
{       230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
} ;

/* TODO : The first 7 coef's are are always hardcode and must
   appear in the actual WAVE file.  They should be read in
   in case a sound program added extras to the list. */
_ATTR_WAVDEC_DATA_
/*static*/ int AdaptCoeff1 [MSADPCM_ADAPT_COEFF_COUNT] =
{       256, 512, 0, 192, 240, 460, 392
} ;

_ATTR_WAVDEC_DATA_
/*static*/ int AdaptCoeff2 [MSADPCM_ADAPT_COEFF_COUNT] =
{       0, -256, 0, 64, 0, -208, -232
} ;

/*============================================================================================
**      MS ADPCM Block Layout.
**      ======================
**      Block is usually 256, 512 or 1024 bytes depending on sample rate.
**      For a mono file, the block is laid out as follows:
**              byte    purpose
**              0               block predictor [0..6]
**              1,2             initial idelta (positive)
**              3,4             sample 1
**              5,6             sample 0
**              7..n    packed bytecodes
**
**      For a stereo file, the block is laid out as follows:
**              byte    purpose
**              0               block predictor [0..6] for left channel
**              1               block predictor [0..6] for right channel
**              2,3             initial idelta (positive) for left channel
**              4,5             initial idelta (positive) for right channel
**              6,7             sample 1 for left channel
**              8,9             sample 1 for right channel
**              10,11   sample 0 for left channel
**              12,13   sample 0 for right channel
**              14..n   packed bytecodes
*/

/*============================================================================================
** Static functions.
*/
_ATTR_WAVDEC_TEXT_
static int     msadpcm_decode_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms) ;

_ATTR_WAVDEC_TEXT_
static sf_count_t msadpcm_read_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms, short *ptr, int len) ;

_ATTR_WAVDEC_TEXT_
sf_count_t       msadpcm_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len) ;

_ATTR_WAVDEC_TEXT_
//static sf_count_t msadpcm_seek   (SF_PRIVATE *psf, int mode, sf_count_t offset) ;
static sf_count_t msadpcm_seek   (SF_PRIVATE *psf, sf_count_t offset) ;

_ATTR_WAVDEC_TEXT_
static int     msadpcm_close   (SF_PRIVATE  *psf) ;

/*============================================================================================
** MS ADPCM Read Functions.
*/

_ATTR_WAVDEC_BSS_
char fdata_buf[14312];

_ATTR_WAVDEC_TEXT_
//int wav_w64_msadpcm_init (SF_PRIVATE *psf, int blockalign, int samplesperblock)
int msadpcm_dec_init (SF_PRIVATE *psf, int blockalign, int samplesperblock)
{
		MSADPCM_PRIVATE *pms ;
        unsigned int    pmssize ;
        int				count ;

        if (psf->mode == SFM_WRITE)
                samplesperblock = 2 + 2 * (blockalign - 7 * psf->sf.channels) / psf->sf.channels ;

        pmssize = sizeof (MSADPCM_PRIVATE) + blockalign + 3 * psf->sf.channels * samplesperblock ;

		//DEBUG("pmssize: \t %i \n",pmssize);

        //if (! (psf->fdata = malloc (pmssize)))
        //        return SFE_MALLOC_FAILED ;
		psf->fdata = &fdata_buf[0];

        pms = (MSADPCM_PRIVATE*) psf->fdata ;
        memset (pms, 0, pmssize) ;

        pms->block   = (unsigned char*) pms->dummydata ;
        pms->samples = (short*) (pms->dummydata + blockalign) ;

        pms->channels        = psf->sf.channels ;
        pms->blocksize       = blockalign ;
        pms->samplesperblock = samplesperblock ;

        if (psf->mode == SFM_READ)
        {       pms->dataremaining       = psf->datalength ;

                if (psf->datalength % pms->blocksize)
                        pms->blocks = psf->datalength / pms->blocksize  + 1 ;
                else
                        pms->blocks = psf->datalength / pms->blocksize ;

                count = 2 * (pms->blocksize - 6 * pms->channels) / pms->channels ;

/*
                if (pms->samplesperblock != count)
                        psf_log_printf (psf, "*** Warning : samplesperblock shoud be %d.\n", count) ;
*/

                psf->sf.frames = (psf->datalength / pms->blocksize) * pms->samplesperblock ;

/*
                psf_log_printf (psf, " bpred   idelta\n") ;
*/

                msadpcm_decode_block (psf, pms) ;
/*
                psf->read_short  = msadpcm_read_s ;
                psf->read_int    = msadpcm_read_i ;
                psf->read_float  = msadpcm_read_f ;
                psf->read_double = msadpcm_read_d ;
*/
                } ;

        if (psf->mode == SFM_WRITE)
        {       pms->samples = (short*) (pms->dummydata + blockalign) ;

                pms->samplecount = 0 ;
/*
                psf->write_short  = msadpcm_write_s ;
                psf->write_int    = msadpcm_write_i ;
                psf->write_float  = msadpcm_write_f ;
                psf->write_double = msadpcm_write_d ;
*/
                } ;

//        psf->seek       = msadpcm_seek ;
//        psf->close      = msadpcm_close ;

        return 0 ;
} /* wav_w64_msadpcm_init */

_ATTR_WAVDEC_TEXT_
static int
msadpcm_decode_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms)
{       int             chan, k, blockindx, sampleindx ;
        short   bytecode, bpred [2], chan_idelta [2] ;

	    int predict ;
	    int current ;
	    int idelta ;

        pms->blockcount ++ ;
        pms->samplecount = 0 ;

        if (pms->blockcount > pms->blocks)
        {
		        memset (pms->samples, 0, pms->samplesperblock * pms->channels) ;
                return 1 ;
        };

        //if ((k = psf_fread (pms->block, 1, pms->blocksize, psf)) != pms->blocksize)
		//if ((k = psf_fread (pms->block, 1, pms->blocksize, pRawFileCache)) != pms->blocksize)
		if ((k = RKFIO_FRead (pms->block, pms->blocksize, pRawFileCache)) != pms->blocksize)
		{
		//TODO: ERROR HANDLER
        //        psf_log_printf (psf, "*** Warning : short read (%d != %d).\n", k, pms->blocksize) ;
            return 0;
		}

        /* Read and check the block header. */

        if (pms->channels == 1)
        {       bpred [0] = pms->block [0] ;

                if (bpred [0] >= 7)
				{
						//TODO: ERROR HANDLER
                        //psf_log_printf (psf, "MS ADPCM synchronisation error (%d).\n", bpred [0]) ;
				}

                chan_idelta [0] = pms->block [1] | (pms->block [2] << 8) ;
                chan_idelta [1] = 0 ;

                //psf_log_printf (psf, "(%d) (%d)\n", bpred [0], chan_idelta [0]) ;

                pms->samples [1] = pms->block [3] | (pms->block [4] << 8) ;
                pms->samples [0] = pms->block [5] | (pms->block [6] << 8) ;
                blockindx = 7 ;
                }
        else
        {       bpred [0] = pms->block [0] ;
                bpred [1] = pms->block [1] ;

                if (bpred [0] >= 7 || bpred [1] >= 7)
				{
				//TODO: ERROR HANDLER
                //      psf_log_printf (psf, "MS ADPCM synchronisation error (%d %d).\n", bpred [0], bpred [1]) ;
				}

                chan_idelta [0] = pms->block [2] | (pms->block [3] << 8) ;
                chan_idelta [1] = pms->block [4] | (pms->block [5] << 8) ;

                //psf_log_printf (psf, "(%d, %d) (%d, %d)\n", bpred [0], bpred [1], chan_idelta [0], chan_idelta [1]) ;

                pms->samples [2] = pms->block [6] | (pms->block [7] << 8) ;
                pms->samples [3] = pms->block [8] | (pms->block [9] << 8) ;

                pms->samples [0] = pms->block [10] | (pms->block [11] << 8) ;
                pms->samples [1] = pms->block [12] | (pms->block [13] << 8) ;

                blockindx = 14 ;
                } ;

        /*--------------------------------------------------------
        This was left over from a time when calculations were done
        as ints rather than shorts. Keep this around as a reminder
        in case I ever find a file which decodes incorrectly.

		if (chan_idelta [0] & 0x8000)
					chan_idelta [0] -= 0x10000 ;
		if (chan_idelta [1] & 0x8000)
					chan_idelta [1] -= 0x10000 ;
        --------------------------------------------------------*/

        /* Pull apart the packed 4 bit samples and store them in their
        ** correct sample positions.
        */

        sampleindx = 2 * pms->channels ;
        while (blockindx <  pms->blocksize)
        {       bytecode = pms->block [blockindx++] ;
                pms->samples [sampleindx++] = (bytecode >> 4) & 0x0F ;
                pms->samples [sampleindx++] = bytecode & 0x0F ;
                } ;

        /* Decode the encoded 4 bit samples. */

        for (k = 2 * pms->channels ; k < (pms->samplesperblock * pms->channels) ; k ++)
        {       chan = (pms->channels > 1) ? (k % 2) : 0 ;

                bytecode = pms->samples [k] & 0xF ;

            /** Compute next Adaptive Scale Factor (ASF) **/
            idelta = chan_idelta [chan] ;
            chan_idelta [chan] = (AdaptationTable [bytecode] * idelta) >> 8 ; /* => / 256 => FIXED_POINT_ADAPTATION_BASE == 256 */
            if (chan_idelta [chan] < 16)
                        chan_idelta [chan] = 16 ;
            if (bytecode & 0x8)
                        bytecode -= 0x10 ;

        predict = ((pms->samples [k - pms->channels] * AdaptCoeff1 [bpred [chan]])
                                        + (pms->samples [k - 2 * pms->channels] * AdaptCoeff2 [bpred [chan]])) >> 8 ; /* => / 256 => FIXED_POINT_COEFF_BASE == 256 */
        current = (bytecode * idelta) + predict;

            if (current > 32767)
                        current = 32767 ;
            else if (current < -32768)
                        current = -32768 ;

                pms->samples [k] = current ;
                } ;

        return 1 ;
} /* msadpcm_decode_block */

_ATTR_WAVDEC_TEXT_
static sf_count_t
msadpcm_read_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms, short *ptr, int len)
{       int     count, total = 0, indx = 0 ;

        while (indx < len)
        {       if (pms->blockcount >= pms->blocks && pms->samplecount >= pms->samplesperblock)
                {       memset (&(ptr[indx]), 0, (size_t) ((len - indx) * sizeof (short))) ;
                        return total ;
                        } ;

                if (pms->samplecount >= pms->samplesperblock)
				{
					if (0==msadpcm_decode_block (psf, pms))
						return 0;
				}

                count = (pms->samplesperblock - pms->samplecount) * pms->channels ;
                count = (len - indx > count) ? count : len - indx ;

                memcpy (&(ptr[indx]), &(pms->samples [pms->samplecount * pms->channels]), count * sizeof (short)) ;
                indx += count ;
                pms->samplecount += count / pms->channels ;
                total = indx ;
                } ;

        return total ;
} /* msadpcm_read_block */

_ATTR_WAVDEC_TEXT_
sf_count_t msadpcm_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len)
{       MSADPCM_PRIVATE         *pms ;
        int                     readcount, count ;
        sf_count_t      total = 0 ;

        if (! psf->fdata)
                return 0 ;
        pms = (MSADPCM_PRIVATE*) psf->fdata ;

        while (len > 0)
        {       readcount = len ; //(len > 0x10000000) ? 0x10000000 : (int) len ;

                count = msadpcm_read_block (psf, pms, ptr, readcount) ;

				/* fread failed , by Vincent Hsiung @ May 13 */
				if (count == 0)
					return 0;

                total += count ;
                len -= count ;
                if (count != readcount)
                        break ;
        } ;

		//mono to stereo , add by Vincent 
		if (psf->sf.channels == 1)
		{
			int i ;
			for ( i = total ; i >= 0 ; i--)
			{
				ptr[i*2] 		= ptr[i];
				ptr[i*2 + 1]	= ptr[i];
			}
		}

        return total ;
} /* msadpcm_read_s */

_ATTR_WAVDEC_TEXT_
static sf_count_t
//msadpcm_seek   (SF_PRIVATE *psf, int mode, sf_count_t offset)
msadpcm_seek   (SF_PRIVATE *psf, sf_count_t offset)
{       MSADPCM_PRIVATE *pms ;
        int                     newblock, newsample ;

        if (! psf->fdata)
                return 0 ;
        pms = (MSADPCM_PRIVATE*) psf->fdata ;

        if (psf->datalength < 0 || psf->dataoffset < 0)
        {       psf->error = SFE_BAD_SEEK ;
                return  ((sf_count_t) -1) ;
                } ;

        if (offset == 0)
        {
				//TODO: SEEK
		        //psf_fseek (psf, psf->dataoffset, SEEK_SET) ;
				RKFIO_FSeek(psf->dataoffset, SEEK_SET , pRawFileCache) ;

                pms->blockcount  = 0 ;
                msadpcm_decode_block (psf, pms) ;
                pms->samplecount = 0 ;
                return 0 ;
        } ;

        if (offset < 0 || offset > pms->blocks * pms->samplesperblock)
        {       psf->error = SFE_BAD_SEEK ;
                return  ((sf_count_t) -1) ;
                } ;

        newblock  = offset / pms->samplesperblock ;
        newsample = offset % pms->samplesperblock ;

        //if (mode == SFM_READ)
        {
				//TODO: SEEK
				//psf_fseek (psf, psf->dataoffset + newblock * pms->blocksize, SEEK_SET) ;
				RKFIO_FSeek(psf->dataoffset + newblock * pms->blocksize , SEEK_SET , pRawFileCache) ;

                pms->blockcount  = newblock ;
                msadpcm_decode_block (psf, pms) ;
                pms->samplecount = newsample ;
        }
#if 0
        else
        {       /* What to do about write??? */
                psf->error = SFE_BAD_SEEK ;
                return  ((sf_count_t) -1) ;
        } ;
#endif

        return newblock * pms->samplesperblock + newsample ;
} /* msadpcm_seek */





_ATTR_WAVDEC_TEXT_
void msadpcm_seek_set   (SF_PRIVATE *psf, sf_count_t offset)
{       MSADPCM_PRIVATE *pms ;
        int                     newblock, newsample ;

        if (! psf->fdata)
                return  ;
        pms = (MSADPCM_PRIVATE*) psf->fdata ;

        if (psf->datalength < 0 || psf->dataoffset < 0)
        {       psf->error = SFE_BAD_SEEK ;
                return;
                } ;

        if (offset == 0)
        {
                pms->blockcount  = 0 ;
                pms->samplecount = 0 ;
                return;
        } ;

        if (offset < 0 || offset > pms->blocks * pms->samplesperblock)
        {       psf->error = SFE_BAD_SEEK ;
                return   ;
                } ;

        newblock  = offset / pms->samplesperblock ;
        newsample = offset % pms->samplesperblock ;

        {
                pms->blockcount  = newblock ;
                pms->samplecount = newsample ;
        }

    return;
} /* msadpcm_seek */
#endif
#endif