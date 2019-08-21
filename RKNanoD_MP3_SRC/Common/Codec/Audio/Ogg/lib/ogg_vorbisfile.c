/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2003    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: stdio-based convenience library for opening/seeking/decoding
 last mod: $Id: vorbisfile.c,v 1.6.2.5 2003/11/20 06:16:17 xiphmont Exp $

 ********************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "codec_internal.h"
#include "ivorbisfile.h"

#include "os.h"
#include "misc.h"
#include "audio_file_access.h"


#ifdef A_CORE_DECODE
#ifdef OGG_DEC_INCLUDE

#pragma arm section code = "OggDecCode", rodata = "OggDecCode", rwdata = "OggDecData", zidata = "OggDecBss"



#define  NOTOPEN   0
#define  PARTOPEN  1
#define  OPENED    2
#define  STREAMSET 3 /* serialno and link set, but not to current link */
#define  LINKSET   4 /* serialno and link set to current link */
#define  INITSET   5

ogg_int64_t dataoffsets[1]; //vf->links ==1
ogg_int64_t pcmlengths[2]; //vf->links*2
ogg_int64_t offsets[2];//vf->links+1
ogg_uint32_t serialnos[1];//vf->links

extern int or_s_ind;
extern int ob_s_ind ;

extern int header_flag;//cdd
extern int page2_header_len ;
extern int ogg_comment_len ;//cdd
extern int ogg_stup_len ;
extern int fetch_flag;

int fseek_dbg(void *f, ogg_int64_t off, int whence);

int fseek_dbg(void *f, ogg_int64_t off, int whence)
{
    //_fseek64_wrap(f,off,whence);
    return (-1);
}




/* read a little more data from the file/pipe into the ogg_sync framer */
long _get_data(OggVorbis_File *vf)
{
    errno = 0;

    if (vf->datasource != (void *) - 1)
    {
        unsigned char *buffer;
        long bytes;
     #ifdef JUMP_COMMENT
        if (header_flag == 3 )
	    {
		  ogg_buffer_release(vf->oy->fifo_tail);
		  //ogg_buffer_release(vf->oy->fifo_tail);
		  buffer = ogg_sync_bufferin(vf->oy, page2_header_len);
		  if (!buffer)
		  {
			return -1;
		  }
          bytes = (vf->callbacks.read_func)((void *)buffer, (int)page2_header_len, (FILE*)vf->datasource);
          //bytes = (vf->callbacks.read_func)((FILE*)vf->datasource, (void *)buffer, (int)page2_header_len);
		  if (bytes>0)
			ogg_sync_wrote(vf->oy, bytes);
		  if (bytes == 0 && errno)return -1;
		  (vf->callbacks.seek_func)((FILE*)vf->datasource, ogg_comment_len, SEEK_CUR);
		     return bytes;
	}

	else
  #endif
  {
        buffer = ogg_sync_bufferin(vf->oy, CHUNKSIZE);

        if (!buffer)
        {
            //ogg_DEBUG("ogg_sync_bufferin  return 0");
            return -1;
        }

        //long bytes=(vf->callbacks.read_func)((FILE*)vf->datasource,(void *)buffer,(int)CHUNKSIZE);
        bytes = (vf->callbacks.read_func)((void *)buffer, (int)CHUNKSIZE, (FILE*)vf->datasource);

        if (bytes > 0)ogg_sync_wrote(vf->oy, bytes);

        if (bytes == 0 && errno)return -1;

        return bytes;
    }
  }
    else
    {
        return 0;
    }
}

/* save a tiny smidge of verbosity to make the code more readable */
void _seek_helper(OggVorbis_File *vf, ogg_int64_t offset)
{
    if (vf->datasource != (void *) - 1)
    {
        (vf->callbacks.seek_func)(vf->datasource, offset, SEEK_SET);
        vf->offset = offset;
        ogg_sync_reset(vf->oy);
    }
    else
    {
        /* shouldn't happen unless someone writes a broken callback */
        return;
    }
}

/* The read/seek functions track absolute position within the stream */

/* from the head of the stream, get the next page.  boundary specifies
   if the function is allowed to fetch more data from the stream (and
   how much) or only use internally buffered data.

   boundary: -1) unbounded search
              0) read no additional data; use cached only
          n) search for a new page beginning for n bytes

   return:   <0) did not find a page (OV_FALSE, OV_EOF, OV_EREAD)
              n) found a page at absolute offset n

              produces a refcounted page */

ogg_int64_t _get_next_page(OggVorbis_File *vf, ogg_page *og,
                           ogg_int64_t boundary)
{
    //ogg_DEBUG("boundary = %d",boundary);
    if (boundary > 0)
        boundary += vf->offset;

    //ogg_DEBUG("00 boundary = 0x%x,vf->offset = 0x%x",boundary,vf->offset);

    while (1)
    {
        long more;

        if (boundary > 0 && vf->offset >= boundary)
        {
            //ogg_DEBUG("11 boundary = 0x%x,vf->offset = 0x%x",boundary,vf->offset);
            return OV_FALSE;
        }

        //ogg_DEBUG("22 boundary = 0x%x,vf->offset = 0x%x",boundary,vf->offset);
        more = ogg_sync_pageseek(vf->oy, og);
#ifndef JUMP_COMMENT
        if (more < 0)
        {
            /* skipped n bytes */
            vf->offset -= more;
        }
        else
        {
            if (more == 0) //第四次  more获取错误
            {
                /* send more paramedics */
                if (!boundary)
                {
                    //ogg_DEBUG();
                    return OV_FALSE;
                }

                {
                    long ret;
                    ret = _get_data(vf);
                    //ogg_DEBUG("_get_data ret = %d", ret);

                    if (ret == 0)
                    {
                        //ogg_DEBUG();
                        return OV_EOF;
                    }

                    if (ret < 0)
                    {
                        //ogg_DEBUG();
                        return OV_EREAD;
                    }
                }
            }
            else
            {
                /* got a page.  Return the offset at the page beginning,
                       advance the internal offset past the page end */
                ogg_int64_t ret = vf->offset;
                vf->offset += more;
                //ogg_DEBUG("offset = %d", vf->offset);
                return ret;
            }
        }
  #else

   if(more<0)
    {
      /* skipped n bytes */
      vf->offset-=more;
    }
   else
   {
      if(more==0)
      {  //第四次  more获取错误

	   /* send more paramedics */
	     if(!boundary)return OV_FALSE;
	     {
	       long ret;
	       ret=_get_data(vf);
	       if(ret==0)return OV_EOF;
	       if(ret<0)return OV_EREAD;
	     }
	  }
	  else if (more == 3)//header_flag == 3;先seek再get_data
	  {
		  long ret;
		  (vf->callbacks.seek_func)((FILE*)vf->datasource, 58, SEEK_SET);//identificationheader为58
		  ret = _get_data(vf);
		  if (ret == 0)return OV_EOF;
		  if (ret<0)return OV_EREAD;
		  vf->offset = 0;
	  }
	  else if (more == 65025)//comment_len+27+255 == 65307为一页的最大长度
	  {
		  return more;
	  }
	  else
      {
	/* got a page.  Return the offset at the page beginning,
           advance the internal offset past the page end */

	    ogg_int64_t ret=vf->offset;
	    vf->offset+=more;
	    return ret;
      }
    }

 #endif
    }
}

/* find the latest page beginning before the current stream cursor
   position. Much dirtier than the above as Ogg doesn't have any
   backward search linkage.  no 'readp' as it will certainly have to
   read. */
/* returns offset or OV_EREAD, OV_FAULT and produces a refcounted page */

ogg_int64_t _get_prev_page(OggVorbis_File *vf, ogg_page *og)
{
    ogg_int64_t begin = vf->offset;
    ogg_int64_t end = begin;
    ogg_int64_t ret;
    ogg_int64_t offset = -1;
    static int bad_cnt = 0;

    while (offset == -1)
    {
        begin -= CHUNKSIZE;

        if (begin < 0)
            begin = 0;

        _seek_helper(vf, begin);

        while (vf->offset < end)
        {
            //ogg_DEBUG();
            ret = _get_next_page(vf, og, end - vf->offset);

            if (ret == OV_EREAD)return OV_EREAD;

            if (ret < 0)
            {
                bad_cnt ++;
                //mod by Vincent Hsiung,Jan 22, 2008
                break;
                //return OV_EFAULT;
            }
            else
            {
                offset = ret;
            }
        }
    }

    /* we have the offset.  Actually snork and hold the page now */
    _seek_helper(vf, offset);
    //ogg_DEBUG();
    ret = _get_next_page(vf, og, CHUNKSIZE);

    if (ret < 0)
        /* this shouldn't be possible */
        return OV_EFAULT;

    //printf("badcnt : %i \n",bad_cnt);
    return offset;
}

/* finds each bitstream link one at a time using a bisection search
   (has to begin by knowing the offset of the lb's initial page).
   Recurses for each link so it can alloc the link storage after
   finding them all, then unroll and fill the cache at the same time */
int _bisect_forward_serialno(OggVorbis_File *vf,
                             ogg_int64_t begin,
                             ogg_int64_t searched,
                             ogg_int64_t end,
                             ogg_uint32_t currentno,
                             long m)
{
    ogg_int64_t endsearched = end;
    ogg_int64_t next = end;
    ogg_page og = {0, 0, 0, 0};
    ogg_int64_t ret;

    /* the below guards against garbage seperating the last and
       first pages of two links. */
    while (searched < endsearched)
    {
        ogg_int64_t bisect;

        if (endsearched - searched < CHUNKSIZE)
        {
            bisect = searched;
        }
        else
        {
            bisect = (searched + endsearched) / 2;
        }

        _seek_helper(vf, bisect);
        //ogg_DEBUG();
        ret = _get_next_page(vf, &og, -1);

        if (ret == OV_EREAD)return OV_EREAD;

        if (ret < 0 || ogg_page_serialno(&og) != currentno)
        {
            endsearched = bisect;

            if (ret >= 0)next = ret;
        }
        else
        {
            searched = ret + og.header_len + og.body_len;
        }

        ogg_page_release(&og);
    }

    _seek_helper(vf, next);
    //ogg_DEBUG();
    ret = _get_next_page(vf, &og, -1);

    if (ret == OV_EREAD)return OV_EREAD;

    if (searched >= end || ret < 0)
    {
        ogg_page_release(&og);
        vf->links = m + 1;
        vf->offsets = offsets;
        vf->serialnos = serialnos;
        vf->offsets[m + 1] = searched;
    }
    else
    {
        ret = _bisect_forward_serialno(vf, next, vf->offset,
                                       end, ogg_page_serialno(&og), m + 1);
        ogg_page_release(&og);

        if (ret == OV_EREAD)return OV_EREAD;
    }

    vf->offsets[m] = begin;
    vf->serialnos[m] = currentno;
    return 0;
}

int _decode_clear(OggVorbis_File *vf)
{
    if (vf->ready_state == INITSET)
    {
        vorbis_dsp_destroy(vf->vd);
        vf->vd = 0;
        vf->ready_state = STREAMSET;
    }

    if (vf->ready_state >= STREAMSET)
    {
        vorbis_info_clear(&vf->vi);
        vorbis_comment_clear(&vf->vc);
        vf->ready_state = OPENED;
    }

    return 0;
}

/* uses the local ogg_stream storage in vf; this is important for
   non-streaming input sources */
/* consumes the page that's passed in (if any) */
/* state is LINKSET upon successful return */

int _fetch_headers(OggVorbis_File *vf,
                   vorbis_info *vi,
                   vorbis_comment *vc,
                   ogg_uint32_t *serialno,
                   ogg_page *og_ptr)
{
    ogg_page og = {0, 0, 0, 0};
    ogg_packet op = {0, 0, 0, 0, 0, 0};
    int i, ret;

    if (vf->ready_state > OPENED)
    {
        _decode_clear(vf);
    }

    if (!og_ptr)
    {
        ogg_int64_t llret = _get_next_page(vf, &og, CHUNKSIZE);
        //ogg_DEBUG();

        if (llret == OV_EREAD)return OV_EREAD;

        if (llret < 0)return OV_ENOTVORBIS;

        og_ptr = &og;
    }

    ogg_stream_reset_serialno(vf->os, ogg_page_serialno(og_ptr));

    if (serialno)*serialno = vf->os->serialno;

    /* extract the initial header from the first page and verify that the
       Ogg bitstream is in fact Vorbis data */
    vorbis_info_init(vi);
    vorbis_comment_init(vc);
    i = 0;
#ifndef JUMP_COMMENT
    while (i < 3)
    {
        ogg_stream_pagein(vf->os, og_ptr); //Submits a complete page to the stream layer.

        while (i < 3)
        {
            int result = ogg_stream_packetout(vf->os, &op);

            if (result == 0)break;

            if (result == -1)
            {
                //ogg_DEBUG();
                ret = OV_EBADHEADER;
                goto bail_header;
            }

            if ((ret = vorbis_dsp_headerin(vi, vc, &op)))
            {
                goto bail_header;
            }

            i++;
        }

        if (i < 3)
        {
            //ogg_DEBUG();
            if (_get_next_page(vf, og_ptr, CHUNKSIZE) < 0)
            {
                //ogg_DEBUG();
                ret = OV_EBADHEADER;
                goto bail_header;
            }
        }
    }
    #else
    while(i<2){
    ogg_stream_pagein(vf->os,og_ptr); //Submits a complete page to the stream layer.

    while(i<2){
      int result=ogg_stream_packetout(vf->os,&op);//读取页结构，正常返回1
      if(result==0)break;
      if(result==-1){
		ret=OV_EBADHEADER;
		goto bail_header;
      }
      if((ret=vorbis_dsp_headerin(vi,vc,&op))){ //标识头，注释头，装备头
		goto bail_header;
      }
      i++;
      ogg_DEBUG("%d " ,i);
    }
    if(i<2)
	{
		/*if(ob_s_ind >=10	|| or_s_ind >=15)
		{
			return -1;
		}*/
#if 1
      if(_get_next_page(vf,og_ptr,CHUNKSIZE)<0){//头中只有三页
        ogg_DEBUG();
		ret=OV_EBADHEADER;
		goto bail_header;
      }
      //ogg_DEBUG("llret = %d ",llret);
#else//cdd 2016.11.8
		ret = _get_next_page(vf, og_ptr, CHUNKSIZE);
		//ogg_DEBUG("ret = %d\n",ret);
		//ogg_DEBUG("offset = %ld",vf->offset);
		if (ret < 0){
			ret = OV_EBADHEADER;
			goto bail_header;
		}
		//跳过注释块
		if (ret == 65025)
		{
			(vf->callbacks.seek_func)((FILE*)vf->datasource, (65307+58), SEEK_SET);
			ret = _get_next_page(vf, og_ptr, CHUNKSIZE);
			if (ret < 0){
				ret = OV_EBADHEADER;
				goto bail_header;
			}
			//ogg_stream_pagein(vf->os, og_ptr);
			//int result = ogg_stream_packetout(vf->os, &op);
			//ret = 0;//int packtype = oggpack_read(&opb, 8);
			{

			}
		}
#endif
	}
  }
#endif

    ogg_packet_release(&op);
    ogg_page_release(&og);
    vf->ready_state = LINKSET;
    return 0;
bail_header:
// ogg_packet_release(&op);
// ogg_page_release(&og);
// vorbis_info_clear(vi);
// vorbis_comment_clear(vc);
// vf->ready_state=OPENED;
    return ret;
}

/* we no longer preload all vorbis_info (and the associated
   codec_setup) structs.  Call this to seek and fetch the info from
   the bitstream, if needed */
int _set_link_number(OggVorbis_File *vf, int link)
{
    if (link != vf->current_link) _decode_clear(vf);

    if (vf->ready_state < STREAMSET)
    {
        _seek_helper(vf, vf->offsets[link]);
        ogg_stream_reset_serialno(vf->os, vf->serialnos[link]);
        vf->current_serialno = vf->serialnos[link];
        vf->current_link = link;
        return _fetch_headers(vf, &vf->vi, &vf->vc, &vf->current_serialno, NULL);
    }

    return 0;
}

int _set_link_number_preserve_pos(OggVorbis_File *vf, int link)
{
    ogg_int64_t pos = vf->offset;
    int ret = _set_link_number(vf, link);

    if (ret)return ret;

    _seek_helper(vf, pos);

    if (pos < vf->offsets[link] || pos >= vf->offsets[link + 1])
        vf->ready_state = STREAMSET;

    return 0;
}

/* last step of the OggVorbis_File initialization; get all the offset
   positions.  Only called by the seekable initialization (local
   stream storage is hacked slightly; pay attention to how that's
   done) */

/* this is void and does not propogate errors up because we want to be
   able to open and use damaged bitstreams as well as we can.  Just
   watch out for missing information for links in the OggVorbis_File
   struct */


void _prefetch_all_offsets(OggVorbis_File *vf, ogg_int64_t dataoffset)
{
    ogg_page og = {0, 0, 0, 0};
    int i;
    ogg_int64_t ret;
    vf->dataoffsets = dataoffsets;
    vf->pcmlengths = pcmlengths;

    for (i = 0; i < vf->links; i++)
    {
        if (i == 0)
        {
            /* we already grabbed the initial header earlier.  Just set the offset */
            vf->dataoffsets[i] = dataoffset;
            _seek_helper(vf, dataoffset);
        }
        else
        {
            /* seek to the location of the initial header */
            _seek_helper(vf, vf->offsets[i]);

            if (_fetch_headers(vf, &vf->vi, &vf->vc, NULL, NULL) < 0)
            {
                vf->dataoffsets[i] = -1;
            }
            else
            {
                vf->dataoffsets[i] = vf->offset;
            }
        }

        /* fetch beginning PCM offset */

        if (vf->dataoffsets[i] != -1)
        {
            ogg_int64_t accumulated = 0, pos;
            long        lastblock = -1;
            int         result;
            ogg_stream_reset_serialno(vf->os, vf->serialnos[i]);

            while (1)
            {
                ogg_packet op = {0, 0, 0, 0, 0, 0};
                //ogg_DEBUG();
                ret = _get_next_page(vf, &og, -1);

                if (ret < 0)
                    /* this should not be possible unless the file is
                           truncated/mangled */
                    break;

                if (ogg_page_serialno(&og) != vf->serialnos[i])
                    break;

                pos = ogg_page_granulepos(&og);
                /* count blocksizes of all frames in the page */
                ogg_stream_pagein(vf->os, &og);

                while ((result = ogg_stream_packetout(vf->os, &op)))
                {
                    if (result > 0) /* ignore holes */
                    {
                        long thisblock = vorbis_packet_blocksize(&vf->vi, &op);

                        if (lastblock != -1)
                            accumulated += (lastblock + thisblock) >> 2;

                        lastblock = thisblock;
                    }
                }

                ogg_packet_release(&op);

                if (pos != -1)
                {
                    /* pcm offset of last packet on the first audio page */
                    accumulated = pos - accumulated;
                    break;
                }
            }

            /* less than zero?  This is a stream with samples trimmed off
               the beginning, a normal occurrence; set the offset to zero */
            if (accumulated < 0)accumulated = 0;

            vf->pcmlengths[i * 2] = accumulated;
        }

        /* get the PCM length of this link. To do this,
           get the last page of the stream */
        {
            ogg_int64_t end = vf->offsets[i + 1];
            _seek_helper(vf, end);

            while (1)
            {
                ret = _get_prev_page(vf, &og);

                if (ret < 0)
                {
                    /* this should not be possible */
                    vorbis_info_clear(&vf->vi);
                    vorbis_comment_clear(&vf->vc);
                    break;
                }

                if (ogg_page_granulepos(&og) != -1)
                {
                    vf->pcmlengths[i * 2 + 1] = ogg_page_granulepos(&og) - vf->pcmlengths[i * 2];
                    break;
                }

                vf->offset = ret;
            }
        }
    }

    ogg_page_release(&og);
}

int _make_decode_ready(OggVorbis_File *vf)
{
    int i;

    switch (vf->ready_state)
    {
        case OPENED:
        case STREAMSET:
            for (i = 0; i < vf->links; i++)
                if (vf->offsets[i + 1] >= vf->offset)break;

            if (i == vf->links)return -1;

            i = _set_link_number_preserve_pos(vf, i);

            if (i)return i;

        /* fall through */
        case LINKSET:
            vf->vd = vorbis_dsp_create(&vf->vi);
            vf->ready_state = INITSET;
            vf->bittrack = 0;
            vf->samptrack = 0;

        case INITSET:
            return 0;

        default:
            return -1;
    }
}

int _open_seekable2(OggVorbis_File *vf)
{
    ogg_uint32_t serialno = vf->current_serialno;
    ogg_uint32_t tempserialno;
    ogg_int64_t dataoffset = vf->offset, end;
    ogg_page og = {0, 0, 0, 0};
    /* we're partially open and have a first link header state in
       storage in vf */
    /* we can seek, so set out learning all about this file */
    (vf->callbacks.seek_func)(vf->datasource, 0, SEEK_END);
    vf->offset = vf->end = (vf->callbacks.tell_func)(vf->datasource);
    /* We get the offset for the last page of the physical bitstream.
       Most OggVorbis files will contain a single logical bitstream */
    end = _get_prev_page(vf, &og);

    if (end < 0)return end;

    /* more than one logical bitstream? */
    tempserialno = ogg_page_serialno(&og);
    ogg_page_release(&og);

    //printf("ov open2\n");
    if (tempserialno != serialno)
    {
        /* Chained bitstream. Bisect-search each logical bitstream
           section.  Do so based on serial number only */
        if (_bisect_forward_serialno(vf, 0, 0, end + 1, serialno, 0) < 0)return OV_EREAD;
    }
    else
    {
        /* Only one logical bitstream */
        if (_bisect_forward_serialno(vf, 0, end, end + 1, serialno, 0))return OV_EREAD;
    }

    /* the initial header memory is referenced by vf after; don't free it */
    _prefetch_all_offsets(vf, dataoffset);
    return ov_raw_seek(vf, 0);
}


int _fetch_and_process_packet(OggVorbis_File *vf,
                              int readp,
                              int spanp)
{
    ogg_page og = {0, 0, 0, 0};
    ogg_packet op = {0, 0, 0, 0, 0, 0};
    int ret = 0;

    /* handle one packet.  Try to fetch it from current stream state */
    /* extract packets from page */
    while (1)
    {
        /* process a packet if we can.  If the machine isn't loaded,
           neither is a page */
        if (vf->ready_state == INITSET)
        {
            while (1)
            {
                int result = ogg_stream_packetout(vf->os, &op);
                ogg_int64_t granulepos;
                fetch_flag = 0;

                if (result < 0)
                {
                    ret = OV_HOLE; /* hole in the data. */
                    goto cleanup;
                }

                if (result > 0)
                {
                    /* got a packet.  process it */
                    granulepos = op.granulepos;

                    if (!vorbis_dsp_synthesis(vf->vd, &op, 1))
                    {
                        /* lazy check for lazy
                                            header handling.  The
                                            header packets aren't
                                            audio, so if/when we
                                            submit them,
                                            vorbis_synthesis will
                                            reject them */
                        vf->samptrack += vorbis_dsp_pcmout(vf->vd, NULL, 0);
                        vf->bittrack += op.bytes * 8;

                        /* update the pcm offset. */
                        if (granulepos != -1 && !op.e_o_s)
                        {
                            int link = (vf->seekable ? vf->current_link : 0);
                            int i, samples;

                            if (vf->seekable && link > 0)
                                granulepos -= vf->pcmlengths[link * 2];

                            if (granulepos < 0)granulepos = 0; /* actually, this

                           shouldn't be possible
                           here unless the stream
                           is very broken */
                            samples = vorbis_dsp_pcmout(vf->vd, NULL, 0);
                            granulepos -= samples;

                            for (i = 0; i < link; i++)
                                granulepos += vf->pcmlengths[i * 2 + 1];

                            vf->pcm_offset = granulepos;
                        }

                        ret = 1;
                        goto cleanup;
                    }
                }
                else
                    break;
            }
        }

        if (vf->ready_state >= OPENED)
        {
            int ret;

            if (!readp)
            {
                ret = 0;
                goto cleanup;
            }

            //ogg_DEBUG();
            if ((ret = _get_next_page(vf, &og, -1)) < 0)
            {
                ret = OV_EOF; /* eof. leave unitialized */
                goto cleanup;
            }

            /* bitrate tracking; add the header's bytes here, the body bytes
               are done by packet above */
            vf->bittrack += og.header_len * 8;

            /* has our decoding just traversed a bitstream boundary? */
            if (vf->ready_state == INITSET)
            {
                if (vf->current_serialno != ogg_page_serialno(&og))
                {
                    if (!spanp)
                    {
                        ret = OV_EOF;
                        goto cleanup;
                    }

                    _decode_clear(vf);
                }
            }
        }

        /* Do we need to load a new machine before submitting the page? */
        /* This is different in the seekable and non-seekable cases.

           In the seekable case, we already have all the header
           information loaded and cached; we just initialize the machine
           with it and continue on our merry way.

           In the non-seekable (streaming) case, we'll only be at a
           boundary if we just left the previous logical bitstream and
           we're now nominally at the header of the next bitstream
        */

        if (vf->ready_state != INITSET)
        {
            int link, ret;

            if (vf->ready_state < STREAMSET)
            {
                if (vf->seekable)
                {
                    vf->current_serialno = ogg_page_serialno(&og);

                    /* match the serialno to bitstream section.  We use this rather than
                       offset positions to avoid problems near logical bitstream
                       boundaries */
                    for (link = 0; link < vf->links; link++)
                        if (vf->serialnos[link] == vf->current_serialno)break;

                    if (link == vf->links)
                    {
                        ret = OV_EBADLINK; /* sign of a bogus stream.  error out,
                leave machine uninitialized */
                        goto cleanup;
                    }

                    vf->current_link = link;
                    ret = _fetch_headers(vf, &vf->vi, &vf->vc, &vf->current_serialno, &og);

                    if (ret) goto cleanup;
                }
                else
                {
                    /* we're streaming */
                    /* fetch the three header packets, build the info struct */
                    int ret = _fetch_headers(vf, &vf->vi, &vf->vc, &vf->current_serialno, &og);

                    if (ret) goto cleanup;

                    vf->current_link++;
                }
            }

            if (_make_decode_ready(vf)) return OV_EBADLINK;
        }

        ogg_stream_pagein(vf->os, &og);
    }

cleanup:
    ogg_packet_release(&op);
    ogg_page_release(&og);
    return ret;
}

/* if, eg, 64 bit stdio is configured by default, this will build with
   fseek64 */
int _fseek64_wrap(FILE *f, ogg_int64_t off, int whence)
{
    if (f == (void *) - 1)return -1;

    return RKFIO_FSeek(off, whence, f); //RKFIO_FSeek(f,off,whence);//fseek(f,off,whence);
}

int _ov_open1(void *f, OggVorbis_File *vf, char *initial,
              long ibytes, ov_callbacks callbacks)
{
    int offsettest = ((f != (void *) - 1) ? callbacks.seek_func(f, 0, SEEK_CUR) : -1);
    int ret;
    ogg_MemSet(vf, 0, sizeof(*vf));

    /* Tremor assumes in multiple places that right shift of a signed
       integer is an arithmetic shift */
    if ( (-1 >> 1) != -1) return OV_EIMPL;

    vf->datasource = f;
    vf->callbacks = callbacks;
    /* init the framing state */
    vf->oy = ogg_sync_create();

    /* perhaps some data was previously read into a buffer for testing
       against other stream types.  Allow initialization from this
       previously read data (as we may be reading from a non-seekable
       stream) */
    if (initial)
    {
        unsigned char *buffer;
        buffer = ogg_sync_bufferin(vf->oy, ibytes);

        if (!buffer)
        {
            return -1;
        }

        ogg_Memcpy(buffer, initial, ibytes);
        ogg_sync_wrote(vf->oy, ibytes);
    }

    /* can we seek? Stevens suggests the seek test was portable */
    if (offsettest != -1)vf->seekable = 1;

    /* No seeking yet; Set up a 'single' (current) logical bitstream
       entry for partial open */
    vf->links = 1;
    vf->os = ogg_stream_create(-1); /* fill in the serialno later */

    /* Try to fetch the headers, maintaining all the storage */
    if ((ret = _fetch_headers(vf, &vf->vi, &vf->vc, &vf->current_serialno, NULL)) < 0)
    {
        ogg_DEBUG("_fetch_headers fail...");
        vf->datasource = (void *) - 1;
        //ov_clear(vf);
    }
    else if (vf->ready_state < PARTOPEN)
        vf->ready_state = PARTOPEN;

    return ret;
}

int _ov_open2(OggVorbis_File *vf)
{
    if (vf->ready_state < OPENED)
        vf->ready_state = OPENED;

    if (vf->seekable)
    {
        int ret = _open_seekable2(vf);

        if (ret)
        {
            vf->datasource = (void *) - 1;
            ov_clear(vf);
        }

        return ret;
    }

    return 0;
}


/* clear out the OggVorbis_File struct */
int ov_clear(OggVorbis_File *vf)
{
    if (vf)
    {
        vorbis_dsp_destroy(vf->vd);
        vf->vd = 0;
        ogg_stream_destroy(vf->os);
        vorbis_info_clear(&vf->vi);
        vorbis_comment_clear(&vf->vc);
        vf->dataoffsets = NULL;
        vf->pcmlengths = NULL;
        vf->serialnos = NULL;
        vf->offsets = NULL;
        ogg_MemSet(dataoffsets, 0, 1);
        ogg_MemSet(pcmlengths, 0, 2);
        ogg_MemSet(serialnos, 0, 1);
        ogg_MemSet(offsets, 0, 2);
        ogg_sync_destroy(vf->oy);

        if (vf->datasource != (void *) - 1)(vf->callbacks.close_func)(vf->datasource);

        ogg_MemSet(vf, 0, sizeof(*vf));
    }

#ifdef DEBUG_LEAKS
    _VDBG_dump();
#endif
    return 0;
}

/* inspects the OggVorbis file and finds/documents all the logical
   bitstreams contained in it.  Tries to be tolerant of logical
   bitstream sections that are truncated/woogie.

   return: -1) error
            0) OK
*/

int ov_open_callbacks(void *f, OggVorbis_File *vf, char *initial, long ibytes,
                      ov_callbacks callbacks)
{
    int ret;
    ret = _ov_open1(f, vf, initial, ibytes, callbacks);
    //ogg_DEBUG("_ov_open1 ret = %d", ret);

    if (ret)return ret;

    return _ov_open2(vf);
}

int ov_open(FILE *f, OggVorbis_File *vf, char *initial, long ibytes)
{
    /*
    ov_callbacks callbacks = {
      (size_t (*)(void *, void * ,size_t))          RKFIO_FRead, //fread,
      (int (*)(void *, ogg_int64_t, int))           _fseek64_wrap,
      (int (*)(void *))                             RKFIO_FClose,
      (long (*)(void *))                            RKFIO_FTell
    };
    */
    ov_callbacks callbacks;
    callbacks.read_func = (size_t(*)(void *, size_t, void *))RKFIO_FRead;
    callbacks.seek_func = (int (*)(void *, ogg_int64_t , int))_fseek64_wrap;
    callbacks.close_func = (int (*)(void *))RKFIO_FClose;
    callbacks.tell_func = (long (*)(void *))RKFIO_FTell;
    return ov_open_callbacks((void *)f, vf, initial, ibytes, callbacks);
}



ogg_int64_t ov_pcm_total(OggVorbis_File *vf, int i)
{
    if (vf->ready_state < OPENED)return OV_EINVAL;

    if (!vf->seekable || i >= vf->links)return OV_EINVAL;

    if (i < 0)
    {
        ogg_int64_t acc = 0;
        int i;

        for (i = 0; i < vf->links; i++)
            acc += ov_pcm_total(vf, i);

        return acc;
    }
    else
    {
        return vf->pcmlengths[i * 2 + 1];
    }
}

/* returns: total milliseconds of content if i==-1
            milliseconds in that logical bitstream for i==0 to n
        OV_EINVAL if the stream is not seekable (we can't know the
        length) or only partially open
*/
ogg_int64_t ov_time_total(OggVorbis_File *vf, int i)
{
    if (vf->ready_state < OPENED)return OV_EINVAL;

    if (!vf->seekable || i >= vf->links)return OV_EINVAL;

    if (i < 0)
    {
        ogg_int64_t acc = 0;
        int i;

        for (i = 0; i < vf->links; i++)
            acc += ov_time_total(vf, i);

        return acc;
    }
    else
    {
        return ((ogg_int64_t)vf->pcmlengths[i * 2 + 1]) * 1000 / vf->vi.rate;
    }
}



int ov_raw_seek(OggVorbis_File *vf, ogg_int64_t pos)
{
    ogg_stream_state work_os = {0};
    ogg_page og = {0, 0, 0, 0};
    ogg_packet op = {0, 0, 0, 0, 0, 0};

    if (vf->ready_state < OPENED)return OV_EINVAL;

    if (!vf->seekable)
        return OV_ENOSEEK; /* don't dump machine if we can't seek */

    if (pos < 0 || pos > vf->end)return OV_EINVAL;

    vf->pcm_offset = -1;
    ogg_stream_reset_serialno(vf->os, vf->current_serialno); /* must set serialno */
    vorbis_dsp_restart(vf->vd);
    _seek_helper(vf, pos);
    {
        int lastblock = 0;
        int accblock = 0;
        int thisblock;
        int eosflag;
        //work_os=ogg_stream_create(vf->current_serialno); /* get the memory ready */
        work_os.serialno = vf->current_serialno;
        work_os.pageno = -1;

        while (1)
        {
            if (vf->ready_state >= STREAMSET)
            {
                /* snarf/scan a packet if we can */
                int result = ogg_stream_packetout(&work_os, &op);

                if (result > 0)
                {
                    if (vf->vi.codec_setup)
                    {
                        thisblock = vorbis_packet_blocksize(&vf->vi, &op);

                        if (thisblock < 0)
                        {
                            ogg_stream_packetout(vf->os, NULL);
                            thisblock = 0;
                        }
                        else
                        {
                            if (eosflag)
                                ogg_stream_packetout(vf->os, NULL);
                            else if (lastblock)accblock += (lastblock + thisblock) >> 2;
                        }

                        if (op.granulepos != -1)
                        {
                            int i, link = vf->current_link;
                            ogg_int64_t granulepos = op.granulepos - vf->pcmlengths[link * 2];

                            if (granulepos < 0)granulepos = 0;

                            for (i = 0; i < link; i++)
                            {
                                granulepos += vf->pcmlengths[i * 2 + 1];
                            }

                            vf->pcm_offset = granulepos - accblock;
                            break;
                        }

                        lastblock = thisblock;
                        continue;
                    }
                    else
                    {
                        ogg_stream_packetout(vf->os, NULL);
                    }
                }
            }

            if (!lastblock)
            {
                //ogg_DEBUG();
                if (_get_next_page(vf, &og, -1) < 0)
                {
                    vf->pcm_offset = ov_pcm_total(vf, -1);
                    break;
                }
            }
            else
            {
                /* huh?  Bogus stream with packets but no granulepos */
                vf->pcm_offset = -1;
                break;
            }

            /* did we just grab a page from other than current link? */
            if (vf->ready_state >= STREAMSET)
            {
                if (vf->current_serialno != ogg_page_serialno(&og))
                {
                    _decode_clear(vf); /* clear out stream state */
                    ogg_stream_destroy(&work_os);
                }
            }

            if (vf->ready_state < STREAMSET)
            {
                int link;
                vf->current_serialno = ogg_page_serialno(&og);

                for (link = 0; link < vf->links; link++)
                    if (vf->serialnos[link] == vf->current_serialno)break;

                if (link == vf->links)
                    goto seek_error; /* sign of a bogus stream.  error out,

                  leave machine uninitialized */
                /* need to initialize machine to this link */
                {
                    int ret = _set_link_number_preserve_pos(vf, link);

                    if (ret) goto seek_error;
                }
                ogg_stream_reset_serialno(vf->os, vf->current_serialno);
                ogg_stream_reset_serialno(&work_os, vf->current_serialno);
            }

            {
                ogg_page dup;
                ogg_page_dup(&dup, &og);
                eosflag = ogg_page_eos(&og);
                ogg_stream_pagein(vf->os, &og);
                ogg_stream_pagein(&work_os, &dup);
            }
        }
    }
    ogg_packet_release(&op);
    ogg_page_release(&og);
    ogg_stream_destroy(&work_os);
    vf->bittrack = 0;
    vf->samptrack = 0;
    return 0;
seek_error:
    ogg_packet_release(&op);
    ogg_page_release(&og);
    /* dump the machine so we're in a known state */
    vf->pcm_offset = -1;
    ogg_stream_destroy(&work_os);
    _decode_clear(vf);
    return OV_EBADLINK;
}
int ov_pcm_seek_page(OggVorbis_File *vf, ogg_int64_t pos)
{
    int link = -1;
    ogg_int64_t result = 0;
    ogg_int64_t total = ov_pcm_total(vf, -1);
    ogg_page og = {0, 0, 0, 0};
    ogg_packet op = {0, 0, 0, 0, 0, 0};

    if (vf->ready_state < OPENED)return OV_EINVAL;

    if (!vf->seekable)return OV_ENOSEEK;

    if (pos < 0 || pos > total)return OV_EINVAL;

    for (link = vf->links - 1; link >= 0; link--)
    {
        total -= vf->pcmlengths[link * 2 + 1];

        if (pos >= total)break;
    }

    if (link != vf->current_link)
    {
        int ret = _set_link_number(vf, link);

        if (ret) goto seek_error;
    }
    else
    {
        vorbis_dsp_restart(vf->vd);
    }

    ogg_stream_reset_serialno(vf->os, vf->serialnos[link]);
    {
        ogg_int64_t end = vf->offsets[link + 1];
        ogg_int64_t begin = vf->offsets[link];
        ogg_int64_t begintime = vf->pcmlengths[link * 2];
        ogg_int64_t endtime = vf->pcmlengths[link * 2 + 1] + begintime;
        ogg_int64_t target = pos - total + begintime;
        ogg_int64_t best = begin;

        while (begin < end)
        {
            ogg_int64_t bisect;

            if (end - begin < CHUNKSIZE)
            {
                bisect = begin;
            }
            else
            {
                bisect = begin +
                         (target - begintime) * (end - begin) / (endtime - begintime) - CHUNKSIZE;

                if (bisect <= begin)
                    bisect = begin + 1;
            }

            _seek_helper(vf, bisect);

            while (begin < end)
            {
                //ogg_DEBUG();
                result = _get_next_page(vf, &og, end - vf->offset);

                if (result == OV_EREAD) goto seek_error;

                if (result < 0)
                {
                    if (bisect <= begin + 1)
                        end = begin; /* found it */
                    else
                    {
                        if (bisect == 0) goto seek_error;

                        bisect -= CHUNKSIZE;

                        if (bisect <= begin)bisect = begin + 1;

                        _seek_helper(vf, bisect);
                    }
                }
                else
                {
                    ogg_int64_t granulepos = ogg_page_granulepos(&og);

                    if (granulepos == -1)continue;

                    if (granulepos < target)
                    {
                        best = result; /* raw offset of packet with granulepos */
                        begin = vf->offset; /* raw offset of next page */
                        begintime = granulepos;

                        if (target - begintime > 44100)break;

                        bisect = begin; /* *not* begin + 1 */
                    }
                    else
                    {
                        if (bisect <= begin + 1)
                            end = begin; /* found it */
                        else
                        {
                            if (end == vf->offset) /* we're pretty close - we'd be stuck in */
                            {
                                end = result;
                                bisect -= CHUNKSIZE; /* an endless loop otherwise. */

                                if (bisect <= begin)bisect = begin + 1;

                                _seek_helper(vf, bisect);
                            }
                            else
                            {
                                end = result;
                                endtime = granulepos;
                                break;
                            }
                        }
                    }
                }
            }
        }

        {
            _seek_helper(vf, best);
            vf->pcm_offset = -1;

            //ogg_DEBUG();
            if (_get_next_page(vf, &og, -1) < 0)
            {
                ogg_page_release(&og);
                return OV_EOF; /* shouldn't happen */
            }

            ogg_stream_pagein(vf->os, &og);

            while (1)
            {
                result = ogg_stream_packetpeek(vf->os, &op);

                if (result == 0)
                {
                    _seek_helper(vf, best);

                    while (1)
                    {
                        result = _get_prev_page(vf, &og);

                        if (result < 0) goto seek_error;

                        if (ogg_page_granulepos(&og) > -1 ||
                            !ogg_page_continued(&og))
                        {
                            return ov_raw_seek(vf, result);
                        }

                        vf->offset = result;
                    }
                }

                if (result < 0)
                {
                    result = OV_EBADPACKET;
                    goto seek_error;
                }

                if (op.granulepos != -1)
                {
                    vf->pcm_offset = op.granulepos - vf->pcmlengths[vf->current_link * 2];

                    if (vf->pcm_offset < 0)vf->pcm_offset = 0;

                    vf->pcm_offset += total;
                    break;
                }
                else
                    result = ogg_stream_packetout(vf->os, NULL);
            }
        }
    }

    if (vf->pcm_offset > pos || pos > ov_pcm_total(vf, -1))
    {
        result = OV_EFAULT;
        goto seek_error;
    }

    vf->bittrack = 0;
    vf->samptrack = 0;
    ogg_page_release(&og);
    ogg_packet_release(&op);
    return 0;
seek_error:
    ogg_page_release(&og);
    ogg_packet_release(&op);
    vf->pcm_offset = -1;
    _decode_clear(vf);
    return (int)result;
}
int ov_pcm_seek(OggVorbis_File *vf, ogg_int64_t pos)
{
    ogg_packet op = {0, 0, 0, 0, 0, 0};
    ogg_page og = {0, 0, 0, 0};
    int thisblock, lastblock = 0;
    int ret = ov_pcm_seek_page(vf, pos);

    if (ret < 0)return ret;

    if (_make_decode_ready(vf))return OV_EBADLINK;

    while (1)
    {
        int ret = ogg_stream_packetpeek(vf->os, &op);

        if (ret > 0)
        {
            thisblock = vorbis_packet_blocksize(&vf->vi, &op);

            if (thisblock < 0)
            {
                ogg_stream_packetout(vf->os, NULL);
                continue; /* non audio packet */
            }

            if (lastblock)vf->pcm_offset += (lastblock + thisblock) >> 2;

            if (vf->pcm_offset + ((thisblock +
                                   vorbis_info_blocksize(&vf->vi, 1)) >> 2) >= pos)break;

            ogg_stream_packetout(vf->os, NULL);
            vorbis_dsp_synthesis(vf->vd, &op, 0);  /* set up a vb with
                          only tracking, no
                          pcm_decode */

            /* end of logical stream case is hard, especially with exact
            length positioning. */
            if (op.granulepos > -1)
            {
                int i;
                vf->pcm_offset = op.granulepos - vf->pcmlengths[vf->current_link * 2];

                if (vf->pcm_offset < 0)vf->pcm_offset = 0;

                for (i = 0; i < vf->current_link; i++)
                    vf->pcm_offset += vf->pcmlengths[i * 2 + 1];
            }

            lastblock = thisblock;
        }
        else
        {
            if (ret < 0 && ret != OV_HOLE)break;

            //ogg_DEBUG();
            if (_get_next_page(vf, &og, -1) < 0)break;

            if (vf->current_serialno != ogg_page_serialno(&og))_decode_clear(vf);

            if (vf->ready_state < STREAMSET)
            {
                int link, ret;
                vf->current_serialno = ogg_page_serialno(&og);

                for (link = 0; link < vf->links; link++)
                    if (vf->serialnos[link] == vf->current_serialno)break;

                if (link == vf->links)
                {
                    ogg_page_release(&og);
                    ogg_packet_release(&op);
                    return OV_EBADLINK;
                }

                vf->current_link = link;
                ret = _fetch_headers(vf, &vf->vi, &vf->vc, &vf->current_serialno, &og);

                if (ret) return ret;

                if (_make_decode_ready(vf))return OV_EBADLINK;

                lastblock = 0;
            }

            ogg_stream_pagein(vf->os, &og);
        }
    }

    vf->bittrack = 0;
    vf->samptrack = 0;

    while (vf->pcm_offset < pos)
    {
        ogg_int64_t target = pos - vf->pcm_offset;
        long samples = vorbis_dsp_pcmout(vf->vd, NULL, 0);

        if (samples > target)samples = target;

        vorbis_dsp_read(vf->vd, samples);
        vf->pcm_offset += samples;

        if (samples < target)
            if (_fetch_and_process_packet(vf, 1, 1) <= 0)
                vf->pcm_offset = ov_pcm_total(vf, -1); /* eof */
    }

    ogg_page_release(&og);
    ogg_packet_release(&op);
    return 0;
}
int ov_time_seek_page(OggVorbis_File *vf, ogg_int64_t milliseconds)
{
    int link = -1;
    ogg_int64_t pcm_total = ov_pcm_total(vf, -1);
    ogg_int64_t time_total = ov_time_total(vf, -1);

    if (vf->ready_state < OPENED)return OV_EINVAL;

    if (!vf->seekable)return OV_ENOSEEK;

    if (milliseconds < 0 || milliseconds > time_total)return OV_EINVAL;

    for (link = vf->links - 1; link >= 0; link--)
    {
        pcm_total -= vf->pcmlengths[link * 2 + 1];
        time_total -= ov_time_total(vf, link);

        if (milliseconds >= time_total)break;
    }

    {
        int ret = _set_link_number(vf, link);

        if (ret)return ret;

        return
            ov_pcm_seek_page(vf, pcm_total + (milliseconds - time_total) *
                             vf->vi.rate / 1000);
    }
}

/* return time offset (milliseconds) of next PCM sample to be read */
ogg_int64_t ov_time_tell(OggVorbis_File *vf)   //当前时间
{
    int link = 0;
    ogg_int64_t pcm_total = 0;
    ogg_int64_t time_total = 0;

    if (vf->ready_state < OPENED)return OV_EINVAL;

    if (vf->seekable)
    {
        pcm_total = ov_pcm_total(vf, -1);
        time_total = ov_time_total(vf, -1);

        /* which bitstream section does this time offset occur in? */
        for (link = vf->links - 1; link >= 0; link--)
        {
            pcm_total -= vf->pcmlengths[link * 2 + 1];
            time_total -= ov_time_total(vf, link);

            if (vf->pcm_offset >= pcm_total)break;
        }
    }

    return time_total + (1000 * vf->pcm_offset - pcm_total) / vf->vi.rate;
}




long ov_read(OggVorbis_File *vf, void *buffer, int bytes_req, int *bitstream)
{
    long samples;
    long channels;
   // ogg_DEBUG();
    if (vf->ready_state < OPENED)return OV_EINVAL;

    while (1)
    {
        if (vf->ready_state == INITSET)
        {
            channels = vf->vi.channels;
            samples = vorbis_dsp_pcmout(vf->vd, buffer, (bytes_req >> 1) / channels);

            if (samples)
            {
                if (samples > 0)
                {
                    vorbis_dsp_read(vf->vd, samples);
                    vf->pcm_offset += samples;

                    if (bitstream)*bitstream = vf->current_link;

                    return samples * 2 * channels;
                }

                return samples;
            }
        }

        /* suck in another packet */
        {
            int ret = _fetch_and_process_packet(vf, 1, 1);

            if (ret == OV_EOF)
                return 0;

            if (ret <= 0)
                return ret;
        }
    }
}

#pragma arm section code

#endif
#endif


