#include "audio_file_access.h"
#include "SysInclude.h"
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef _DSDIFF_DECODE_
#include <stdio.h>
#pragma arm section code = "DsdiffDecCode", rodata = "DsdiffDecCode", rwdata = "DsdiffDecData", zidata = "DsdiffDecBss"

//#include<stdio.h>
#include<stdbool.h>
#include<stdint.h>
//#include<malloc.h>
#include"sacd_dsdiff.h"
#include"endianess.h"
#include"dsd2pcm_conv.h"

Dec_Type DEC_DSF;
sacd_dsdiff_t dff_t;
dsd2pcm_converter_t dsd2pcm_converter;
//读文件
_ATTR_DSDIFFDEC_BIN_TEXT_
static uint32 dsdiff_read(uint8_t *Buf, uint32 Size)
{
    return RKFIO_FRead(Buf, Size, dff_t.pfile);
//  printf("%d \n", fread(Buf, 1, Size, dff_t.pfile));
//  return fread(Buf,1,Size,dff_t.pfile);
}

_ATTR_DSDIFFDEC_BIN_TEXT_
static uint8 dsdiff_skip(int32 offset)
{
    return RKFIO_FSeek(offset, SEEK_CUR, dff_t.pfile);
//  return fseek(dff_t.pfile, offset, SEEK_CUR);
}
_ATTR_DSDIFFDEC_BIN_TEXT_
BOOL dsdiff_decode_end(void) {

    return dff_t.m_decode_frame_count >= dff_t.m_frame_count;
}
_ATTR_DSDIFFDEC_BIN_TEXT_
static BOOL ck_has_id(const uint8_t* id, uint8_t* ckID) {
    return ckID[0] == id[0] && ckID[1] == id[1] && ckID[2] == id[2] && ckID[3] == id[3];
}
_ATTR_DSDIFFDEC_BIN_TEXT_
static uint32 ck_get_size64(uint8_t *ckDataSize) {
    return MAKE_MARKER(ckDataSize[7], ckDataSize[6], ckDataSize[5], ckDataSize[4]);
}
_ATTR_DSDIFFDEC_BIN_TEXT_
static uint32 ck_get_size32(uint8_t *ckDataSize) {
    return MAKE_MARKER(ckDataSize[3], ckDataSize[2], ckDataSize[1], ckDataSize[0]);
}

_ATTR_DSDIFFDEC_BIN_TEXT_
static uint16  ck_get_size16(uint8_t *ckDataSize) {
    return MAKE_MARKER16(ckDataSize[1], ckDataSize[0]);
}
_ATTR_DSDIFFDEC_BIN_TEXT_
static uint32 dsdiff_get_position(void)
{
    return RKFIO_FTell(dff_t.pfile);
//  return ftell(dff_t.pfile);
}

_ATTR_DSDIFFDEC_BIN_TEXT_
BOOL dsdiff_open(void) {
    Chunk ck;
    uint8_t id[4];
    uint8_t data[4];
    uint8_t data1[4];


    if (!(dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck) && ck_has_id("FRM8", ck.ckID))) {
        dsd_printf("FRM8 err \n");
//      printf("FRM8 err \n");
        return FALSE;
    }

    if (!(dsdiff_read(id, sizeof(id)) == sizeof(id) && ck_has_id("DSD ", id))) {
        dsd_printf("DSD err \n");
//      printf("DSD err \n");
        return FALSE;
    }

    for (;;) {
        if (!(dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck))) {
            dsd_printf("read ck1  err \n");
//          printf("read ck1  err \n");
            return FALSE;
        }
//      printf("%d \n", ck_get_size64(ck.ckDataSize));
//      printf("%s \n", ck.ckDataSize);
        if (ck_has_id("FVER", ck.ckID) && ck_get_size64(ck.ckDataSize) == 4) {
            uint8_t *version = data;

            if (!(dsdiff_read(version, sizeof(version)) == sizeof(version))) {
                dsd_printf("version   err \n");
//              printf("version err  \n");
                return FALSE;
            }
            dff_t.m_version = ck_get_size32(version);

        }
        else if (ck_has_id("PROP", ck.ckID)) {
            uint32_t id_prop_size, id_prop_read;
            //Hifi_Dff_Printf("PROP:%s \n",ck.ckID);
            if (!(dsdiff_read(id, sizeof(id)) == sizeof(id) && ck_has_id("SND ", id))) {
                dsd_printf("SND   err \n");
//              printf("SND   err \n");
                return FALSE;
            }
            id_prop_size = ck_get_size64(ck.ckDataSize) - sizeof(id);
            id_prop_read = 0;
            while (id_prop_read < id_prop_size) {
                if (!(dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck))) {
                    return FALSE;
                }
                if (ck_has_id("FS  ", ck.ckID) && ck_get_size64(ck.ckDataSize) == 4) {
                    uint8_t* samplerate = data;
                    //Hifi_Dff_Printf("FS:%s \n",ck.ckID);
                    if (!(dsdiff_read(samplerate, sizeof(samplerate)) == sizeof(samplerate))) {
                        dsd_printf("samplerate   err \n");
                //      printf("samplerate   err \n");
                        return FALSE;
                    }
                    dff_t.m_samplerate = ck_get_size32(samplerate);
                }
                else if (ck_has_id("CHNL", ck.ckID)) {
                    uint8_t* channel_count = data1;
                    //Hifi_Dff_Printf("CHNL:%s \n",ck.ckID);
                    if (!(dsdiff_read(channel_count, sizeof(channel_count)) == sizeof(channel_count))) {
                        dsd_printf("channel_count   err \n");
//                      printf("channel_count   err \n");
                        return FALSE;
                    }
                    dff_t.m_channel_count = ck_get_size16(channel_count);
                    dsdiff_skip(ck_get_size64(ck.ckDataSize) - sizeof(channel_count));
                }
                else if (ck_has_id("CMPR", ck.ckID)) {
                    //Hifi_Dff_Printf("CMPR:%s \n",ck.ckID);
                    if (!(dsdiff_read(id, sizeof(id)) == sizeof(id))) {
                        dsd_printf("CMPR   err \n");
//                      printf("CMPR   err \n");
                        return FALSE;
                    }
                    if (ck_has_id("DSD ", id)) {
                        //Hifi_Dff_Printf("DSD:%s \n",id);
                        dff_t.m_dst_encoded = 0;
                    }
                    if (ck_has_id("DST ", id)) {
                        dff_t.m_dst_encoded = 1;
                    }
                    dsdiff_skip(ck_get_size64(ck.ckDataSize) - sizeof(id));
                }
                else {
                    dsdiff_skip(ck_get_size64(ck.ckDataSize));
                }
                id_prop_read += sizeof(ck)+ck_get_size64(ck.ckDataSize);
            }
            continue;
        }
        else if (ck_has_id("DSD ", ck.ckID)) {
            dff_t.m_data_offset = dsdiff_get_position();
            dff_t.m_data_size = ck_get_size64(ck.ckDataSize);
            dff_t.m_framerate = 75;
            dff_t.m_frame_size = 9408;
            //dff_t.m_frame_size = dff_t.m_samplerate / 8 * dff_t.m_channel_count / dff_t.m_framerate;
            dff_t.m_frame_count = (uint32_t)(dff_t.m_data_size / dff_t.m_frame_size);
            return TRUE;
        }
        else if (ck_has_id("DST ", ck.ckID)) {// add dst

            return TRUE;
        }
        else {
            dsdiff_skip(ck_get_size64(ck.ckDataSize));
        }
    }
    dsd_printf("dff  read ck id   err\n");
    return FALSE;
}

_ATTR_DSDIFFDEC_BIN_TEXT_
BOOL dsdiff_read_frame(uint8_t* frame_data, uint32_t* frame_size) {
    //Hifi_Dff_Printf("dff read frame\n");
    if (dff_t.m_dst_encoded) {
        Chunk ck;
//      Hifi_Dff_Printf("dstf read frame!");
        dsd_printf("dstf read frame!");
        while (dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck)) {
            if (ck_has_id("DSTF", ck.ckID)) {
                if (dsdiff_read(frame_data, ck_get_size64(ck.ckDataSize)) == ck_get_size64(ck.ckDataSize)) {
                    *frame_size = (uint32_t)ck_get_size64(ck.ckDataSize);
                    dsdiff_skip(ck_get_size64(ck.ckDataSize) & 1);
                    return TRUE;
                }
                break;
            }
            else if (ck_has_id("DSTC", ck.ckID)) {
                if (dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck)) {
                    dsdiff_skip(ck_get_size64(ck.ckDataSize));
                    dsdiff_skip(ck_get_size64(ck.ckDataSize) & 1);
                }
            }
            else {
//              Hifi_Dff_Printf("dst read frame err!");
                dsd_printf("dst read frame err!");
                return FALSE;
            }
        }
    }
    else {
        *frame_size = dsdiff_read(frame_data, dff_t.m_frame_size);
        //Hifi_Dff_Printf("frame_size:%d ",*frame_size);
        dff_t.m_decode_frame_count++;
        if (*frame_size > 0) {
            return TRUE;
        }
    }

//  Hifi_Dff_Printf("dsdiff read frame err!");
    if (dff_t.m_decode_frame_count<dff_t.m_frame_count)
        dsd_printf("NO.%d frame dsdiff read frame err!", dff_t.m_decode_frame_count);
    return FALSE;
}

_ATTR_DSDIFFDEC_TEXT_
int dsdiff_decode_init(FILE* pfile)
{
    int dsdiff_out_len;
    dff_t.pfile = pfile;

    if (dsdiff_open())
    {
//      Hifi_Dff_Printf("m_samplerate:%d,channels:%d,m_frame_count:%d,m_frame_size:%d", dff_t.m_samplerate, dff_t.m_channel_count, dff_t.m_frame_count, dff_t.m_frame_size);
        dsd_printf("m_samplerate:%d\n", dff_t.m_samplerate);
        dsd_printf("channels:%d\n", dff_t.m_channel_count);
        dsd_printf("m_frame_count:%d\n", dff_t.m_frame_count);
        dsd_printf("m_frame_size:%d\n", dff_t.m_frame_size);
    }
    else
    {
        dsd_printf("DSD IFF OPEN err!");
        return -1;
    }
    dff_t.pcm_out_channels = dff_t.m_channel_count;
    //dsd_printf("channel =%d\n",dff_t.pcm_out_channels);
    dff_t.dsd_samplerate = dff_t.m_samplerate;
    //dff_t.pcm_out_samplerate = 176400;
    switch (dff_t.dsd_samplerate){
    case 2822400:
        DEC_DSF = DSD64;
        dff_t.pcm_out_samplerate = 44100;//176400;
        break;
    case 5644800:
        dff_t.pcm_out_samplerate = 44100;//88200;
        DEC_DSF = DSD128;
        break;
    case 11289600:
        dff_t.pcm_out_samplerate = 44100;//88200;
        DEC_DSF = DSD256;
        break;
    }
    dff_t.bps = 32;
    dsdiff_out_len =  dff_t.m_frame_size  / ((dff_t.m_samplerate / dff_t.pcm_out_samplerate)>>3);
    dsdiff_out_len = dsdiff_out_len / dff_t.m_channel_count;
    dff_t.bitrate = dff_t.dsd_samplerate*dff_t.m_channel_count;
    dff_t.frameLength = ((double)(dff_t.m_frame_size * 1000 * 8) / (dff_t.m_channel_count * dff_t.m_samplerate));
    dsd2pcm_converter_init(&dsd2pcm_converter, dff_t.pcm_out_channels, dff_t.dsd_samplerate, dff_t.pcm_out_samplerate);
  //  dsd_printf("%d,dsdiff_out_len =%d\n",dff_t.m_frame_size,dsdiff_out_len);
    return dsdiff_out_len;
}
/*input:*/

_ATTR_DSDIFFDEC_TEXT_
int dsdiff_decode(uint8* pcm_out_buf, uint32_t* OutLength)
{
    int  Length = 0;
    dsdiff_read_frame(dff_t.dsd_data, &dff_t.dsd_size);
    //dsd_printf("262");
    if (dff_t.dsd_size > 0)
    {
        Length = dsd2pcm_converter_convert(&dsd2pcm_converter, dff_t.dsd_data, dff_t.dsd_size, pcm_out_buf, dff_t.bps);
      //  *OutLength = Length / dff_t.pcm_out_channels;
     *OutLength = Length / dff_t.m_channel_count;
    }
    //dsd_printf("Length=%d\n",Length);
   // dsd_printf("dff_t.pcm_out_channels1=%d\n",dff_t.m_channel_count);
    return *OutLength;
}
_ATTR_DSDIFFDEC_BIN_TEXT_
int dsdiff_seek(int seconds)
{
    unsigned long FFWOffset, FFWSampleNumber;
    int i;
    uint32_t FileSize = RKFIO_FLength(dff_t.pfile);

    FFWOffset = (((long long)seconds * dff_t.bitrate) >> 3) + (dff_t.m_data_offset);
    //  FFWOffset = (((long long)seconds * s_FLAC_INFO.bitrate)/8) + (metsize);
    //  FFWSampleNumber = (unsigned long)seconds * dff_t.samplerate;
    //dsd_printf("seek 0x%x ", FFWOffset);
    //dsd_printf("filesize = %lu ", FileSize);
    if (FFWOffset > FileSize)
    {
        goto dsdiff_seek_fail;
    }
    if (RKFIO_FSeek(FFWOffset,SEEK_SET,dff_t.pfile ) != 0)
    {
        goto dsdiff_seek_fail;
    }
    //按second seek，不需要校正，1s 75帧,如果按ms seek则需要加上这一部分
    if ((FFWOffset - dff_t.m_data_offset) % dff_t.m_frame_size != 0)
    {
        FFWOffset += dff_t.m_frame_size - (FFWOffset - dff_t.m_data_offset) % dff_t.m_frame_size;
    }
   // dsd_printf("seek校正 0x%x ", FFWOffset);
    if (RKFIO_FSeek(FFWOffset,SEEK_SET,dff_t.pfile) != 0)
    {
        goto dsdiff_seek_fail;
    }
dsdiff_seek_ok:

    //dsd_printf("seek_ok");
    dff_t.m_decode_frame_count = FFWOffset/dff_t.m_frame_size;
    return 0;

dsdiff_seek_fail:
    dsd_printf("dff seek_fail");
    return -1;
}
#pragma arm section code
#endif
#endif
