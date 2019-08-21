#include        <stdio.h>

#include       "PCM.H"
#include       "../include/audio_file_access.h"
#include       "../include/audio_main.h"


#ifdef A_CORE_DECODE
#ifdef _RECORD_

extern FILE *pRawFileCache;

#include "sf_wav.h"

//_ATTR_MSADPCM_BSS_
_ATTR_MSADPCM_TEXT_
SF_PRIVATE sf_enc;

unsigned short gPCMDataWidth;

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
    short               *samples ;
    unsigned char       *block ;
    unsigned char       dummydata [4] ; /* Dummy size */
} MSADPCM_PRIVATE ;

/*============================================================================================
** MS ADPCM static data and functions.
*/

_ATTR_MSADPCM_DATA_
static int AdaptationTable []    =
    {       230, 230, 230, 230, 307, 409, 512, 614,
            768, 614, 512, 409, 307, 230, 230, 230
    } ;

/* TODO : The first 7 coef's are are always hardcode and must
   appear in the actual WAVE file.  They should be read in
   in case a sound program added extras to the list. */

_ATTR_MSADPCM_DATA_
/*static*/ int AdaptCoeff11 [MSADPCM_ADAPT_COEFF_COUNT] =
    {       256, 512, 0, 192, 240, 460, 392
    } ;



_ATTR_MSADPCM_DATA_
/*static*/ int AdaptCoeff22 [MSADPCM_ADAPT_COEFF_COUNT] =
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


_ATTR_MSADPCM_TEXT_
static int     msadpcm_encode_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms) ;


_ATTR_MSADPCM_TEXT_
static sf_count_t msadpcm_write_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms, short *ptr, int len) ;

_ATTR_MSADPCM_TEXT_
sf_count_t       msadpcm_write_s (SF_PRIVATE *psf, short *ptr, sf_count_t len , char ** ppOutBuf) ;


_ATTR_MSADPCM_TEXT_
static int     msadpcm_close   (SF_PRIVATE  *psf) ;


_ATTR_MSADPCM_TEXT_
static void    choose_predictor (unsigned int channels, short *data, int *bpred, int *idelta) ;

/*============================================================================================
** MS ADPCM Read Functions.
*/


//_ATTR_MSADPCM_BSS_
_ATTR_MSADPCM_TEXT_
static char fdata_buf[8000];


_ATTR_MSADPCM_TEXT_
int msadpcm_enc_init (SF_PRIVATE *psf, int blockalign, int samplesperblock)
{
    MSADPCM_PRIVATE *pms ;
    unsigned int    pmssize ;
    int             count ;

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

    if (psf->mode == SFM_WRITE)
    {
        pms->samples = (short*) (pms->dummydata + blockalign) ;

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


/*==========================================================================================
** MS ADPCM Write Functions.
*/

_ATTR_MSADPCM_TEXT_
static int
msadpcm_encode_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms)
{
    unsigned int    blockindx ;
    unsigned char   byte ;
    int                             chan, k, predict, bpred [2], idelta [2], errordelta, newsamp ;

    choose_predictor (pms->channels, pms->samples, bpred, idelta) ;

    /* Write the block header. */

    if (pms->channels == 1)
    {
        pms->block [0]  = bpred [0] ;
        pms->block [1]  = idelta [0] & 0xFF ;
        pms->block [2]  = idelta [0] >> 8 ;
        pms->block [3]  = pms->samples [1] & 0xFF ;
        pms->block [4]  = pms->samples [1] >> 8 ;
        pms->block [5]  = pms->samples [0] & 0xFF ;
        pms->block [6]  = pms->samples [0] >> 8 ;

        blockindx = 7 ;
        byte = 0 ;

        /* Encode the samples as 4 bit. */

        for (k = 2 ; k < pms->samplesperblock ; k++)
        {
            predict = (pms->samples [k-1] * AdaptCoeff11 [bpred [0]] + pms->samples [k-2] * AdaptCoeff22 [bpred [0]]) >> 8 ;
            errordelta = (pms->samples [k] - predict) / idelta [0] ;
            if (errordelta < -8)
                errordelta = -8 ;
            else if (errordelta > 7)
                errordelta = 7 ;
            newsamp = predict + (idelta [0] * errordelta) ;
            if (newsamp > 32767)
                newsamp = 32767 ;
            else if (newsamp < -32768)
                newsamp = -32768 ;
            if (errordelta < 0)
                errordelta += 0x10 ;

            byte = (byte << 4) | (errordelta & 0xF) ;
            if (k % 2)
            {
                pms->block [blockindx++] = byte ;
                byte = 0 ;
            } ;

            idelta [0] = (idelta [0] * AdaptationTable [errordelta]) >> 8 ;
            if (idelta [0] < 16)
                idelta [0] = 16 ;
            pms->samples [k] = newsamp ;
        } ;
    }
    else
    {       /* Stereo file. */
        pms->block [0]  = bpred [0] ;
        pms->block [1]  = bpred [1] ;

        pms->block [2]  = idelta [0] & 0xFF ;
        pms->block [3]  = idelta [0] >> 8 ;
        pms->block [4]  = idelta [1] & 0xFF ;
        pms->block [5]  = idelta [1] >> 8 ;

        pms->block [6]  = pms->samples [2] & 0xFF ;
        pms->block [7]  = pms->samples [2] >> 8 ;
        pms->block [8]  = pms->samples [3] & 0xFF ;
        pms->block [9]  = pms->samples [3] >> 8 ;

        pms->block [10] = pms->samples [0] & 0xFF ;
        pms->block [11] = pms->samples [0] >> 8 ;
        pms->block [12] = pms->samples [1] & 0xFF ;
        pms->block [13] = pms->samples [1] >> 8 ;

        blockindx = 14 ;
        byte = 0 ;
        chan = 1 ;

        for (k = 4 ; k < 2 * pms->samplesperblock ; k++)
        {
            chan = k & 1 ;

            predict = (pms->samples [k-2] * AdaptCoeff11 [bpred [chan]] + pms->samples [k-4] * AdaptCoeff22 [bpred [chan]]) >> 8 ;
            errordelta = (pms->samples [k] - predict) / idelta [chan] ;


            if (errordelta < -8)
                errordelta = -8 ;
            else if (errordelta > 7)
                errordelta = 7 ;
            newsamp = predict + (idelta [chan] * errordelta) ;
            if (newsamp > 32767)
                newsamp = 32767 ;
            else if (newsamp < -32768)
                newsamp = -32768 ;
            if (errordelta < 0)
                errordelta += 0x10 ;

            byte = (byte << 4) | (errordelta & 0xF) ;

            if (chan)
            {
                pms->block [blockindx++] = byte ;
                byte = 0 ;
            } ;

            idelta [chan] = (idelta [chan] * AdaptationTable [errordelta]) >> 8 ;
            if (idelta [chan] < 16)
                idelta [chan] = 16 ;
            pms->samples [k] = newsamp ;
        } ;
    } ;

    /* Write the block to disk. */

    //if ((k = psf_fwrite (pms->block, 1, pms->blocksize, psf)) != pms->blocksize)
    //TODO: WRITE
    //if ((k = psf_fwrite (pms->block, 1, pms->blocksize, fOut )) != pms->blocksize)
    //        psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", k, pms->blocksize) ;

    memset (pms->samples, 0, pms->samplesperblock * sizeof (short)) ;

    pms->blockcount ++ ;
    pms->samplecount = 0 ;

    return 1 ;
} /* msadpcm_encode_block */


_ATTR_MSADPCM_TEXT_
static sf_count_t
msadpcm_write_block (SF_PRIVATE *psf, MSADPCM_PRIVATE *pms, short *ptr, int len)
{
    int             count, total = 0, indx = 0 ;

    while (indx < len)
    {
        count = (pms->samplesperblock - pms->samplecount) * pms->channels ;

        if (count > len - indx)
            count = len - indx ;

        memcpy (&(pms->samples [pms->samplecount * pms->channels]), &(ptr [total]), count * sizeof (short)) ;
        indx += count ;
        pms->samplecount += count / pms->channels ;
        total = indx ;

        if (pms->samplecount >= pms->samplesperblock)
            msadpcm_encode_block (psf, pms) ;
    } ;

    return total ;
} /* msadpcm_write_block */


_ATTR_MSADPCM_TEXT_
sf_count_t
msadpcm_write_s (SF_PRIVATE *psf, short *ptr, sf_count_t len, char ** ppOutBuf )
{
    MSADPCM_PRIVATE *pms ;
    int                     writecount, count ;
    sf_count_t      total = 0 ;

    if (! psf->fdata)
        return 0 ;
    pms = (MSADPCM_PRIVATE*) psf->fdata ;

    /* len must equal block size , so we can directly use pms->block as output , by Vincent */
    while (len > 0)
    {
        //writecount = (len > 0x10000000) ? 0x10000000 : (int) len ;
        writecount = len;

        count = msadpcm_write_block (psf, pms, ptr, writecount) ;

        total += count ;
        len -= count ;
        if (count != writecount)
            break ;
    } ;

    /* by Vincent @ Apr 12,2009*/
    *ppOutBuf = (char *) pms->block;

    /**/
    //pms->blocksize

    //return total ;
    return pms->blocksize;
} /* msadpcm_write_s */

/*========================================================================================
*/


_ATTR_MSADPCM_TEXT_
static int
msadpcm_close   (SF_PRIVATE  *psf)
{
    MSADPCM_PRIVATE *pms ;

    if (! psf->fdata)
        return 0 ;

    pms = (MSADPCM_PRIVATE*) psf->fdata ;

    if (psf->mode == SFM_WRITE)
    {       /*  Now we know static int for certain the length of the file we can
                **  re-write the header.
                */

        if (pms->samplecount && pms->samplecount < pms->samplesperblock)
            msadpcm_encode_block (psf, pms) ;
        /*
                        if (psf->write_header)
                                psf->write_header (psf, SF_TRUE) ;
        */
    } ;


    return 0 ;
} /* msadpcm_close */

/*========================================================================================
** Static functions.
*/

/*----------------------------------------------------------------------------------------
**      Choosing the block predictor.
**      Each block requires a predictor and an idelta for each channel.
**      The predictor is in the range [0..6] which is an indx into the  two AdaptCoeff tables.
**      The predictor is chosen by trying all of the possible predictors on a small set of
**      samples at the beginning of the block. The predictor with the smallest average
**      abs (idelta) is chosen as the best predictor for this block.
**      The value of idelta is chosen to to give a 4 bit code value of +/- 4 (approx. half the
**      max. code value). If the average abs (idelta) is zero, the sixth predictor is chosen.
**      If the value of idelta is less then 16 it is set to 16.
**
**      Microsoft uses an IDELTA_COUNT (number of sample pairs used to choose best predictor)
**      value of 3. The best possible results would be obtained by using all the samples to
**      choose the predictor.
*/

#define         IDELTA_COUNT    3

_ATTR_MSADPCM_TEXT_
static  void
choose_predictor (unsigned int channels, short *data, int *block_pred, int *idelta)
{
    unsigned int    chan, k, bpred, idelta_sum, best_bpred, best_idelta ;

    for (chan = 0 ; chan < channels; chan++)
    {
        best_bpred = best_idelta = 0 ;

        for (bpred = 0 ; bpred < 7 ; bpred++)
        {
            idelta_sum = 0 ;
            for (k = 2 ; k < 2 + IDELTA_COUNT ; k++)
                idelta_sum += abs (data [k*channels] - ((data [(k-1)*channels] * AdaptCoeff11 [bpred] + data [(k-2)*channels] * AdaptCoeff22 [bpred]) >> 8)) ;
            idelta_sum /= (4 * IDELTA_COUNT) ;

            if (bpred == 0 || idelta_sum < best_idelta)
            {
                best_bpred = bpred ;
                best_idelta = idelta_sum ;
            } ;

            if (! idelta_sum)
            {
                best_bpred = bpred ;
                best_idelta = 16 ;
                break ;
            } ;

        }
        ; /* for bpred ... */
        if (best_idelta < 16)
            best_idelta = 16 ;

        block_pred [chan] = best_bpred ;
        idelta [chan]     = best_idelta ;
    } ;

    return ;
} /* choose_predictor */


//****************************************************************************
//
// InitADPCMEncoder prepares to encode a file using the MS ADPCM encoder.
//
//****************************************************************************

_ATTR_MSADPCM_TEXT_
void InitADPCMEncoder(tPCM_enc *pPCM)
{
    if(pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
    {
        switch (pPCM->usSampleRate)
        {
            case    8000:
            case    11025:
                pPCM->usBytesPerBlock = 256;
                break;
            case    12000:
            case    16000:
                pPCM->usBytesPerBlock = 256;
                break;

            case    22050:
            case    24000:
                pPCM->usBytesPerBlock = 512;
                break;
            case    32000:
            case    44100:
            case    48000:
            case    64000:
            case    88200:
            case    96000:
            case    128000:
            case    176400:
            case    192000:
            pPCM->usBytesPerBlock = 1024;
                break;
            default:
            pPCM->usBytesPerBlock = 1024;
            break;
        }

        pPCM->usSamplesPerBlock = ((pPCM->usBytesPerBlock << 1) >> (pPCM->ucChannels - 1)) - 12;
        pPCM->usByteRate = pPCM->usSampleRate * pPCM->usBytesPerBlock / pPCM->usSamplesPerBlock;
        pPCM->wFormatTag = WAVE_FORMAT_ADPCM;
        //bb_printf1(" usSamplesPerBlock = %d",pPCM->usSamplesPerBlock);
    }
    else if(pPCM->wFormatTag == WAVE_FORMAT_PCM)
    {        
        if(gPCMDataWidth == 0xF)
        {
            pPCM->usBytesPerBlock = 4096*3;
            pPCM->usByteRate = pPCM->ucChannels * 2 * pPCM->usSampleRate;
            pPCM->usSamplesPerBlock = pPCM->usBytesPerBlock >> pPCM->ucChannels ;

        }
        else if(gPCMDataWidth = 0x17)
        {
            
            pPCM->usBytesPerBlock = 4096*3;
            pPCM->usByteRate = pPCM->ucChannels * 3 * pPCM->usSampleRate;
            pPCM->usSamplesPerBlock = pPCM->usBytesPerBlock >> pPCM->ucChannels ;
        }
    }

//    pPCM->usSamplesPerBlock = ((pPCM->usBytesPerBlock << 1) >> (pPCM->ucChannels - 1)) - 12;
//    pPCM->usByteRate = pPCM->usSampleRate * pPCM->usBytesPerBlock / pPCM->usSamplesPerBlock;


    pPCM->sPCMHeader.ucChannels = pPCM->ucChannels;
    pPCM->sPCMHeader.usSamplesPerBlock = pPCM->usSamplesPerBlock;

    pPCM->usValid = 90;

    pPCM->ulLength = 0;
    pPCM->ulTimePos = 0;
    pPCM->ulDataValid = 0;
//  pPCM->pOutput = 0;

    //pPCM->wFormatTag = WAVE_FORMAT_ADPCM;

}


#if 0
int RKFIO_FRead(char *b, int s , FILE *f)
{
    return fread(b,1,s,f);
}

int RKFIO_FSeek( long off , int whence , FILE *f)
{
    return fseek(f,off,whence);
}

#include "pcm.h"

short out[1152];
int i;

#if 0

int main()
{
    //int wav_w64_msadpcm_init (SF_PRIVATE *psf, int blockalign, int samplesperblock);

    tPCM PCM_s;

    SF_PRIVATE sf;

    int isErr;

    //pRawFileCache = fopen("c:\\sandbox\\MS_ADPCM_ST_44K.wav","rb");
    //fOut = fopen("c:\\sandbox\\MS_ADPCM_ST_44K.pcm","wb");

    pRawFileCache = fopen("c:\\sandbox\\ADPCM_02.wav","rb");
    fOut = fopen("c:\\sandbox\\ADPCM_02.pcm","wb");



    isErr = InitPCMDecoder(&PCM_s);

    sf.sf.channels = PCM_s.ucChannels;
    sf.sf.samplerate = PCM_s.usSampleRate;
    sf.sf.format = (SF_FORMAT_WAV | SF_FORMAT_MS_ADPCM);
    sf.mode = SFM_READ;

    //psf->datalength
    sf.datalength = PCM_s.ulLength;

    //wav_w64_msadpcm_init (&sf, PCM_s.usBytesPerBlock, PCM_s.usSamplesPerBlock);
    wav_w64_msadpcm_init (&sf, PCM_s.usBytesPerBlock, PCM_s.usSamplesPerBlock);

    //for(i = 0; i < 3000; i ++)
    while (1)
    {
        int ret;
        ret = msadpcm_read_s (&sf, &out[0] , 1152);

        //DEBUG("ret:\t%i\n",ret);
        if (ret == 0)
            break;

        fwrite(&out[0],2,1152,fOut);
    }

    fclose(fOut);

}

#else

#if 0
int main()
{
    //int wav_w64_msadpcm_init (SF_PRIVATE *psf, int blockalign, int samplesperblock);

    tPCM PCM_s;

    SF_PRIVATE sf;

    int isErr;

    //pRawFileCache = fopen("c:\\sandbox\\MS_ADPCM_ST_44K.pcm","rb");
    //fOut = fopen("c:\\sandbox\\MS_ADPCM_ST_44K_by_enc.wav","wb");
    //PCM_s.usSampleRate = 44100;
    //PCM_s.ucChannels = 2;

    pRawFileCache = fopen("c:\\sandbox\\ADPCM_02.pcm","rb");
    fOut = fopen("c:\\sandbox\\ADPCM_02_enc.wav","wb");
    PCM_s.usSampleRate = 8000;
    PCM_s.ucChannels = 1;

    //isErr = InitPCMDecoder(&PCM_s);
    InitADPCMEncoder(&PCM_s);

    sf.sf.channels = PCM_s.ucChannels;
    sf.sf.samplerate = PCM_s.usSampleRate;
    sf.sf.format = (SF_FORMAT_WAV | SF_FORMAT_MS_ADPCM);
    //sf.mode = SFM_READ;
    sf.mode = SFM_WRITE;

    //psf->datalength
    sf.datalength = PCM_s.ulLength;

    //wav_w64_msadpcm_init (&sf, PCM_s.usBytesPerBlock, PCM_s.usSamplesPerBlock);
    wav_w64_msadpcm_init (&sf, PCM_s.usBytesPerBlock, PCM_s.usSamplesPerBlock);

    {
        PCMWAVEFORMAT sWaveFormat;
        int ulIdx;

        PCM_s.pucEncodedData[70] = 'f';
        PCM_s.pucEncodedData[71] = 'a';
        PCM_s.pucEncodedData[72] = 'c';
        PCM_s.pucEncodedData[73] = 't';
        PCM_s.pucEncodedData[74] = 4;
        PCM_s.pucEncodedData[75] = 0;
        PCM_s.pucEncodedData[76] = 0;
        PCM_s.pucEncodedData[77] = 0;

        PCM_s.pucEncodedData[78] = PCM_s.ulTimePos;
        PCM_s.pucEncodedData[79] = PCM_s.ulTimePos >> 8;
        PCM_s.pucEncodedData[80] = PCM_s.ulTimePos >> 16;
        PCM_s.pucEncodedData[81] = PCM_s.ulTimePos >> 24;

        PCM_s.pucEncodedData[82] = 'd';
        PCM_s.pucEncodedData[83] = 'a';
        PCM_s.pucEncodedData[84] = 't';
        PCM_s.pucEncodedData[85] = 'a';

        PCM_s.pucEncodedData[86] = PCM_s.ulDataValid;
        PCM_s.pucEncodedData[87] = PCM_s.ulDataValid >> 8;
        PCM_s.pucEncodedData[88] = PCM_s.ulDataValid >> 16;
        PCM_s.pucEncodedData[89] = PCM_s.ulDataValid >> 24;

        PCM_s.ulDataValid += 82;

        PCM_s.pucEncodedData[0] = 'R';
        PCM_s.pucEncodedData[1] = 'I';
        PCM_s.pucEncodedData[2] = 'F';
        PCM_s.pucEncodedData[3] = 'F';

        PCM_s.pucEncodedData[4] = PCM_s.ulDataValid;
        PCM_s.pucEncodedData[5] = PCM_s.ulDataValid >> 8;
        PCM_s.pucEncodedData[6] = PCM_s.ulDataValid >> 16;
        PCM_s.pucEncodedData[7] = PCM_s.ulDataValid >> 24;

        PCM_s.pucEncodedData[8] = 'W';
        PCM_s.pucEncodedData[9] = 'A';
        PCM_s.pucEncodedData[10] = 'V';
        PCM_s.pucEncodedData[11] = 'E';

        PCM_s.pucEncodedData[12] = 'f';
        PCM_s.pucEncodedData[13] = 'm';
        PCM_s.pucEncodedData[14] = 't';
        PCM_s.pucEncodedData[15] = ' ';
        PCM_s.pucEncodedData[16] = 50;
        PCM_s.pucEncodedData[17] = 0;
        PCM_s.pucEncodedData[18] = 0;

        PCM_s.pucEncodedData[19] = 0;

        sWaveFormat.wFormatTag = 2;
        sWaveFormat.nChannels = PCM_s.ucChannels;
        sWaveFormat.nSamplesPerSec = PCM_s.usSampleRate;
        sWaveFormat.nAvgBytesPerSec = PCM_s.usByteRate;
        sWaveFormat.nBlockAlign = PCM_s.usBytesPerBlock;
        sWaveFormat.wBitsPerSample = 4;
        sWaveFormat.cbSize = 32;
        sWaveFormat.wSamplesPerBlock = PCM_s.usSamplesPerBlock;
        sWaveFormat.wNumCoef = 7;

        for (ulIdx = 0; ulIdx < 7; ulIdx++)
        {
            sWaveFormat.aCoef[ulIdx].iCoef1 = AdaptCoeff11[ulIdx];
            sWaveFormat.aCoef[ulIdx].iCoef2 = AdaptCoeff22[ulIdx];
        }

        memcpy(PCM_s.pucEncodedData + 20, (void *)&sWaveFormat, 50);

#define ADPCM_FILL_HDR  90
        fwrite(PCM_s.pucEncodedData,1,ADPCM_FILL_HDR,fOut);
    }


    //for(i = 0; i < 3000; i ++)
    while (1)
    {
        int ret;

        ret = fread(&out[0],2,1152,pRawFileCache);

        if (ret == 0)
            break;

        //ret = msadpcm_read_s (&sf, &out[0] , 1152);

        msadpcm_write_s (&sf, &out[0] , 1152);

        //DEBUG("ret:\t%i\n",ret);



    }

    fclose(fOut);

}
#endif
#endif
#endif

#endif
#endif

