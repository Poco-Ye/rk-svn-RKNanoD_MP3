/* bitstream.c */
#include  "SysInclude.h"
#include  "audio_main.h"

#include  "FsInclude.h"
#include  "File.h"
#include  "FDT.h"


#include "mp3_enc_types.h"

#ifdef MP3_ENC_INCLUDE2
#include  "RecordControl.h"
#pragma arm section code = "EncodeMP3Code", rodata = "EncodeMP3Code", rwdata = "EncodeMP3Data", zidata = "EncodeMP3Bss"


extern int *scalefac_band_long;

static void encodeSideInfo( L3_side_info_t  *si );
static int encodeMainData(int l3_enc[2][2][samp_per_frame2],
                L3_side_info_t  *si );
static void Huffmancodebits( int *ix, gr_info *gi );
static int HuffmanCode(int table_select, int x, int y, unsigned long *code,
                unsigned long *extword, int *codebits, int *extbits);
static int L3_huffman_coder_count1( struct huffcodetab *h,
                int v, int w, int x, int y );
static unsigned char buf_out[BUFFER_SIZE];

 extern int    l3_enc[2][2][samp_per_frame2];   // 576*4*4 = 9216

 extern long   mdct_freq[2][2][samp_per_frame2];  //576*4*4 = 9216  ¹²32256

extern long   l3_sb_sample[2][3][18][SBLIMIT];   //32*6*18*4 = 13824

/* bitstream buffers.
 * At a 24K samplerate and 8k bitrate in stereo, sideinfo can be up to 86 frames
 * ahead of its main data ( very silly combination ) */
#define FIFO_SIZE 128

static struct
{
  int si_len; /* number of bytes in side info */
  int fr_len; /* number of data bytes in frame */
  unsigned char side[40]; /* side info max len = (288 bits / 8) + 1 which gets cleared ×î³õ40*/
} fifo[FIFO_SIZE];
#define main_size 16384   //main_size = si->resv_drain
 unsigned char main_[main_size];  /* main data buffer (length always less than this)Êµ²â68*/
 int wr, rd; /* read and write index for side fifo */
 int by , bi; /* byte and bit counts within main or side stores */
 int count;  /* byte counter within current frame */
 int bits;   /* bit counter used when encoding side and main data */




static unsigned char header[4];



/*
 * putbytes
 * --------
 * put n bytes in the output buffer. Empty the buffer when full.
 */
void putbytes(unsigned char *data, int n)
{
  unsigned int free = BUFFER_SIZE - bs.i;

  if(n >= free)
  {
    bb_printf1("putbytes  n >= free");
   //DEBUG("BUFFER_SIZE\n");
  }
  else
  {
    memcpy(&bs.b[bs.i],data,n);
    bs.i += n;

  }
}

/*
 * open_bit_stream
 * ---------------
 * open the device to write the bit stream into it
 */
void open_bit_stream()
{

  bs.b = buf_out;
  bs.i = 0;

  /* setup header (the only change between frames is the padding bit) */
  header[0] = 0xff;

  header[1] = 0xe0 |
              (config.mpeg.type << 3) |
              (config.mpeg.layr << 1) |
              !config.mpeg.crc;

  header[2] = (config.mpeg.bitrate_index << 4) |
              (config.mpeg.samplerate_index << 2) |
            /* config,mpeg.padding inserted later */
               config.mpeg.ext;

  header[3] = (config.mpeg.mode << 6) |
              (config.mpeg.mode_ext << 4) |
              (config.mpeg.copyright << 3) |
              (config.mpeg.original << 2) |
               config.mpeg.emph;

}

/*
 * close_bit_stream
 * ----------------
 * close the device containing the bit stream
 */

/*
 * L3_format_bitstream
 * -------------------
 * This is called after a frame of audio has been quantized and coded.
 * It will write the encoded audio to the bitstream. Note that
 * from a layer3 encoder's perspective the bit stream is primarily
 * a series of main_data() blocks, with header and side information
 * inserted at the proper locations to maintain framing. (See Figure A.7
 * in the IS).
 *
 * note. both header/sideinfo and main data are multiples of 8 bits.
 * this means that the encoded data can be manipulated as bytes
 * which is easier and quicker than bits.
 */

unsigned int L3_format_bitstream( int l3_enc[2][2][samp_per_frame2], L3_side_info_t  *l3_side,unsigned char ** ppOutBuf)
{
  int main_bytes;
  encodeSideInfo( l3_side ); /* store in fifo */
  main_bytes = encodeMainData( l3_enc, l3_side );


  /* send data */
  by = 0;
  bs.i = 0;

  while(main_bytes)
  {
    if (!count)
    { /* end of frame so output next header/sideinfo */
      putbytes(fifo[rd].side, fifo[rd].si_len);
      count = fifo[rd].fr_len;
      if(++rd == FIFO_SIZE) rd = 0; /* point to next header/sideinfo */

    }

    if(main_bytes <= count)
    { /* enough room in frame to output rest of main data, this will exit the while loop */
      putbytes(&main_[by],main_bytes);
      count -= main_bytes;
      main_bytes = 0;

    }
    else
    { /* fill current frame up, start new frame next time around the while loop */
      putbytes(&main_[by],count);
      main_bytes -= count;
      by += count;
      count = 0;

    }
  }
  *ppOutBuf = bs.b;


 return  bs.i;
}

/*
 * putbits:
 * --------
 * write N bits into the encoded data buffer.
 * buf = encoded data buffer
 * val = value to write into the buffer
 * n = number of bits of val
 *
 * Bits in value are assumed to be right-justified.
 */
 void putbits (unsigned char *buf, unsigned long val, unsigned int n)
{
  static int mask[9]={0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
  int k, tmp;

  bits += n;

  while (n)
  {
    k = (n < bi) ? n : bi;
    tmp = val >> (n - k);
    buf[by] |= (tmp & mask[k]) << (bi - k);
    bi -= k;
    n -= k;
    if (!bi)
    {
      bi = 8;
      by++;
      buf[by]=0;
    }
  }
}

/*
 * encodeMainData
 * --------------
 * Encodes the spectrum and places the coded
 * main data in the buffer main.
 * Returns the number of bytes stored.
 */
 int encodeMainData(int l3_enc[2][2][samp_per_frame2],
                           L3_side_info_t  *si)
{
  int gr, ch;

  bits = 0;
  by = 0;
  bi = 8;
  main_[0]=0;

  /* huffmancodes plus reservoir stuffing */
  for(gr = 0; gr < config.mpeg.granules; gr++)
    for (ch = 0; ch < config.mpeg.channels; ch++)
      Huffmancodebits( l3_enc[gr][ch], &si->gr[gr].ch[ch].tt ); /* encode the spectrum */

  /* ancillary data, used for reservoir stuffing overflow */

  if(si->resv_drain)
  {
    int words = si->resv_drain >> 5;
    int remainder = si->resv_drain & 31;
     if(si->resv_drain > main_size)
     {
       printf("main_ too little");
     }
    /* pad with zeros */


    while(words--)
      putbits(main_, 0, 32 );
    if(remainder)
      putbits(main_, 0, remainder );


  }



 // printf("%d ",si->resv_drain);
  return bits >> 3;
}

/*
 * encodeSideInfo
 * --------------
 * Encodes the header and sideinfo and stores the coded data
 * in the side fifo for transmission at the appropriate place
 * in the bitstream.
 */
 void encodeSideInfo( L3_side_info_t  *si )
{
  int gr, ch, region;
  unsigned char * sf = fifo[wr].side;

  /* header */
  *sf     = header[0];
  *(sf+1) = header[1];
  *(sf+2) = header[2] | (config.mpeg.padding << 1);
  *(sf+3) = header[3];
  *(sf+4) = 0;

  bits = 32;
  by = 4;
  bi = 8;

  /* side info */
  if(config.mpeg.type == MPEG1)
  {
    putbits(sf, si->main_data_begin, 9 );
    putbits(sf, 0, ( config.mpeg.channels == 2 ) ? 3 : 5 ); /* private bits */

    for ( ch = 0; ch < config.mpeg.channels; ch++ )
      putbits(sf, 0, 4 ); /* scfsi */

    for ( gr = 0; gr < 2; gr++ )
      for ( ch = 0; ch < config.mpeg.channels ; ch++ )
      {
        gr_info *gi = &(si->gr[gr].ch[ch].tt);
        putbits(sf, gi->part2_3_length,    12 );
        putbits(sf, gi->big_values,         9 );
        putbits(sf, gi->global_gain,        8 );
        putbits(sf, 0, 5 ); /* scalefac_compress, window switching flag */

        for ( region = 0; region < 3; region++ )
          putbits(sf, gi->table_select[region], 5 );

        putbits(sf, gi->region0_count,      4 );
        putbits(sf, gi->region1_count,      3 );
        putbits(sf, 0, 2 ); /* preflag, scalefac_scale */
        putbits(sf, gi->count1table_select, 1 );
      }
  }
  else /* mpeg 2/2.5 */
  {
    putbits(sf, si->main_data_begin, 8 );
    putbits(sf, 0, ( config.mpeg.channels == 1 ) ? 1 : 2 ); /* private bits */

    for ( ch = 0; ch < config.mpeg.channels ; ch++ )
    {
      gr_info *gi = &(si->gr[0].ch[ch].tt);
      putbits(sf, gi->part2_3_length,    12 );
      putbits(sf, gi->big_values,         9 );
      putbits(sf, gi->global_gain,        8 );
      putbits(sf, 0, 10 ); /* scalefac_compress, window switching flag */

      for ( region = 0; region < 3; region++ )
        putbits(sf, gi->table_select[region], 5 );

      putbits(sf, gi->region0_count,      4 );
      putbits(sf, gi->region1_count,      3 );
      putbits(sf, 0, 1 ); /* scalefac_scale */
      putbits(sf, gi->count1table_select, 1 );
    }
  }

  fifo[wr].fr_len = (config.mpeg.bits_per_frame - bits) >> 3; /* data bytes in this frame */
  fifo[wr].si_len = bits >> 3;                       /* bytes in side info */
  if(++wr == FIFO_SIZE) wr = 0; /* point to next buffer */
}

/*
 * Huffmancodebits
 * ---------------
 * Note the discussion of huffmancodebits() on pages 28
 * and 29 of the IS, as well as the definitions of the side
 * information on pages 26 and 27.
 */
 void Huffmancodebits( int *ix, gr_info *gi )
{
  int region1Start;
  int region2Start;
  int i, bigvalues, count1End;
  int v, w, x, y, cx_bits, cbits, xbits, stuffingBits;
  unsigned long code, ext;
  struct huffcodetab *h;
  int bvbits, c1bits, tablezeros, r0, r1, r2, rt, *pr;
  int bitsWritten = 0;
//  int idx = 0;
  tablezeros = 0;
  r0 = r1 = r2 = 0;

  /* 1: Write the bigvalues */
  bigvalues = gi->big_values <<1;
  {
    unsigned scalefac_index = 100;

    scalefac_index = gi->region0_count + 1;
    region1Start = scalefac_band_long[ scalefac_index ];
    scalefac_index += gi->region1_count + 1;
    region2Start = scalefac_band_long[ scalefac_index ];

    for ( i = 0; i < bigvalues; i += 2 )
    {
      unsigned tableindex = 100;

      /* get table pointer */
      if ( i < region1Start )
      {
        tableindex = gi->table_select[0];
        pr = &r0;
      }
      else if ( i < region2Start )
      {
        tableindex = gi->table_select[1];
        pr = &r1;
      }
      else
      {
        tableindex = gi->table_select[2];
        pr = &r2;
      }

      h = &ht[ tableindex ];
      /* get huffman code */
      x = ix[i];
      y = ix[i + 1];
      if ( tableindex )
      {
        cx_bits = HuffmanCode( tableindex, x, y, &code, &ext, &cbits, &xbits );
        putbits(main_,  code, cbits );
        putbits(main_,  ext, xbits );
        bitsWritten += rt = cx_bits;
        *pr += rt;
      }
      else
      {
        tablezeros += 1;
        *pr = 0;
      }
    }
  }
  bvbits = bitsWritten;

  /* 2: Write count1 area */
  h = &ht[gi->count1table_select + 32];
  count1End = bigvalues + (gi->count1 <<2);
  for ( i = bigvalues; i < count1End; i += 4 )
  {
    v = ix[i];
    w = ix[i+1];
    x = ix[i+2];
    y = ix[i+3];
    bitsWritten += L3_huffman_coder_count1( h, v, w, x, y );
  }
  c1bits = bitsWritten - bvbits;
  if ((stuffingBits = gi->part2_3_length - bitsWritten) != 0)
  {
    int words = stuffingBits >> 5;
    int remainder = stuffingBits & 31;

    /*
     * Due to the nature of the Huffman code
     * tables, we will pad with ones
     */
    while(words--)
      putbits(main_, ~0, 32 );
    if(remainder)
      putbits(main_, ~0, remainder );
  }
}

/*
 * abs_and_sign
 * ------------
 */
 int abs_and_sign( int *x )
{
  if ( *x > 0 ) return 0;
  *x *= -1;
  return 1;
}

/*
 * L3_huffman_coder_count1
 * -----------------------
 */
 int L3_huffman_coder_count1( struct huffcodetab *h, int v, int w, int x, int y )
{
  HUFFBITS huffbits;
  unsigned int signv, signw, signx, signy, p;
  int len;
  int totalBits;

  signv = abs_and_sign( &v );
  signw = abs_and_sign( &w );
  signx = abs_and_sign( &x );
  signy = abs_and_sign( &y );

  p = v + (w << 1) + (x << 2) + (y << 3);
  huffbits = h->table[p];
  len = h->hlen[p];
  putbits(main_,  huffbits, len );
  totalBits = len;
  if ( v )
  {
    putbits(main_,  signv, 1 );
    totalBits++;
  }
  if ( w )
  {
    putbits(main_,  signw, 1 );
    totalBits++;
  }

  if ( x )
  {
    putbits(main_,  signx, 1 );
    totalBits++;
  }
  if ( y )
  {
    putbits(main_,  signy, 1 );
    totalBits++;
  }
  return totalBits;
}

/*
 * HuffmanCode
 * -----------
 * Implements the pseudocode of page 98 of the IS
 */
 int HuffmanCode(int table_select, int x, int y, unsigned long *code,
                unsigned long *ext, int *cbits, int *xbits )
{
  unsigned signx, signy, linbitsx, linbitsy, linbits, xlen, ylen, idx;
  struct huffcodetab *h;

  *cbits = 0;
  *xbits = 0;
  *code  = 0;
  *ext   = 0;

  if(table_select==0) return 0;

  signx = abs_and_sign( &x );
  signy = abs_and_sign( &y );
  h = &(ht[table_select]);
  xlen = h->xlen;
  ylen = h->ylen;
  linbits = h->linbits;
  linbitsx = linbitsy = 0;

  if ( table_select > 15 )
  { /* ESC-table is used */
    if ( x > 14 )
    {
      linbitsx = x - 15;
      x = 15;
    }
    if ( y > 14 )
    {
      linbitsy = y - 15;
      y = 15;
    }

    idx = (x * ylen) + y;
    *code  = h->table[idx];
    *cbits = h->hlen [idx];
    if ( x > 14 )
    {
      *ext   |= linbitsx;
      *xbits += linbits;
    }
    if ( x != 0 )
    {
      *ext = ((*ext) << 1) | signx;
      *xbits += 1;
    }
    if ( y > 14 )
    {
      *ext = ((*ext) << linbits) | linbitsy;
      *xbits += linbits;
    }
    if ( y != 0 )
    {
      *ext = ((*ext) << 1) | signy;
      *xbits += 1;
    }
  }
  else
  { /* No ESC-words */
    idx = (x * ylen) + y;
    *code = h->table[idx];
    *cbits += h->hlen[ idx ];
    if ( x != 0 )
    {
      *code = ((*code) << 1) | signx;
      *cbits += 1;
    }
    if ( y != 0 )
    {
      *code = ((*code) << 1) | signy;
      *cbits += 1;
    }
  }
  return *cbits + *xbits;
}


#pragma arm section code
#endif

