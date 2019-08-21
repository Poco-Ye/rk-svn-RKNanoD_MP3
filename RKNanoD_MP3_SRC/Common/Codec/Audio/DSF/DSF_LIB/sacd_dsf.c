#include "audio_file_access.h"
#include "SysInclude.h"
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef _DSF_DECODE_
#include <stdio.h>
#pragma arm section code = "DsfDecCode", rodata = "DsfDecCode", rwdata = "DsfDecData", zidata = "DsfDecBss"


#include<stdio.h>
#include<stdbool.h>
#include<stdint.h>
//#include<malloc.h>
#include"sacd_dsf.h"
#include"endianess.h"
#include"dsf2pcm_conv.h"
#include"my.h"

sacd_dsf_t dsf_t;
dsd2pcm_converter_t dsd2pcm_converter;

_ATTR_DSFDEC_BIN_TEXT_
static uint32 dsdiff_read(uint8_t *Buf, uint32 Size)
{
        return RKFIO_FRead(Buf, Size, dsf_t.pfile);
}
_ATTR_DSFDEC_BIN_TEXT_
static BOOL ck_has_id(const uint8_t* id, uint8_t* ckID) {

    //Hifi_Dff_Printf("id:%s, ckID:%s \n",id, ckID);
    return ckID[0] == id[0] && ckID[1] == id[1] && ckID[2] == id[2] && ckID[3] == id[3];
}
_ATTR_DSFDEC_BIN_TEXT_
static uint8 dsdiff_skip(int32 offset)
{
     return RKFIO_FSeek(offset, SEEK_CUR, dsf_t.pfile);
    //return fseek(dsf_t.pfile, offset, SEEK_CUR);
}
_ATTR_DSFDEC_BIN_TEXT_
BOOL dsf_decode_end(void) {

    return dsf_t.m_decode_frame_count >= dsf_t.m_frame_count;
}
_ATTR_DSFDEC_BIN_TEXT_
static uint32 ck_get_size64(uint8_t *ckDataSize) {
    return MAKE_MARKER(ckDataSize[7], ckDataSize[6], ckDataSize[5], ckDataSize[4]);
}
_ATTR_DSFDEC_BIN_TEXT_
static uint32 ck_get_size32(uint8_t *ckDataSize) {
    return MAKE_MARKER(ckDataSize[3], ckDataSize[2], ckDataSize[1], ckDataSize[0]);
}

_ATTR_DSFDEC_BIN_TEXT_
static uint16  ck_get_size16(uint8_t *ckDataSize) {
    return MAKE_MARKER16(ckDataSize[1], ckDataSize[0]);
}
_ATTR_DSFDEC_BIN_TEXT_
static uint32 dsdiff_get_position(void)
{
     return RKFIO_FTell(dsf_t.pfile);
    //return ftell(dsf_t.pfile);

}

//读取文件头
_ATTR_DSFDEC_BIN_TEXT_
BOOL dsf_open(void){
    Chunk ck;
    uint8_t  data[8];
    uint8_t version[4];
    uint8_t formatid[4];
    uint8_t channelType[4];
    uint8_t channelNum[4];
    uint8_t samplerate[4];
    uint8_t bit_per_sample[4];
    uint8_t sampleCount[8];
    uint8_t block[4];
    uint8_t metadata[8];

    if (!(dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck) && ck_has_id("DSD ", ck.ckID))) {
        dsf_printf("DSD err \n");
        return FALSE;
    }
    if ((dsdiff_read((uint8_t*)&data, sizeof(data)) == sizeof(data))){
        dsf_t.m_file_size = ck_get_size32(data);
        dsf_printf("fileSize:%d\n", dsf_t.m_file_size);
    }

    //如果存在ID3V2信息就怎么处理？？？？
    if (!((dsdiff_read((uint8_t*)&metadata, sizeof(metadata)) == sizeof(metadata)) && ck_get_size32(metadata) == 0)){
        dsf_printf("ID3V2 start:%d\n", ck_get_size32(metadata));
//      return FALSE;
    }


    if (!(dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck) && ck_has_id("fmt ", ck.ckID))) {
        dsf_printf("fmt err \n");
        return FALSE;
    }
    if ((dsdiff_read((uint8_t*)&version, sizeof(version)) == sizeof(version))){
        dsf_t.m_version = ck_get_size32(version);
    }
    if (!((dsdiff_read((uint8_t*)&formatid, sizeof(formatid)) == sizeof(formatid)) && (ck_get_size32(formatid) == 0))){
        dsf_printf("formatid err\n");//0:DSD raw
        return FALSE;
    }
    if ((dsdiff_read((uint8_t*)&channelType, sizeof(channelType)) == sizeof(channelType))){
        dsf_t.m_channel_type = ck_get_size32(channelType);
//      printf("channelType = %d\n", channelType);
    }
    if ((dsdiff_read((uint8_t*)&channelNum, sizeof(channelNum)) == sizeof(channelNum))){
        dsf_t.m_channel_count = ck_get_size32(channelNum);
    }
    if ((dsdiff_read((uint8_t*)&samplerate, sizeof(samplerate)) == sizeof(samplerate))){
        dsf_t.m_samplerate = ck_get_size32(samplerate);
        /*if ((dsf_t.m_samplerate = ck_get_size32(samplerate)) > 2822400){
            dsf_printf("samplerate no support!\n");
            return FALSE;
        }*/
    }
    if ((dsdiff_read((uint8_t*)&bit_per_sample, sizeof(bit_per_sample)) == sizeof(bit_per_sample))){
        dsf_t.m_bit_per_sample = ck_get_size32(bit_per_sample);
        if ((dsf_t.m_bit_per_sample) >= 8)
        {
            dsf_printf("bit_per_sample not support!\n");
            return FALSE;
        }
    }
    if ((dsdiff_read((uint8_t*)&sampleCount, sizeof(sampleCount)) == sizeof(sampleCount))){
        //dsf_t.m_sampleCount = ck_get_size32(sampleCount);
        dsf_printf("sampleCount:%lu\n", ck_get_size32(sampleCount));
    }
    if ((dsdiff_read((uint8_t*)&block, sizeof(block)) == sizeof(block))){
        dsf_t.m_frame_size = ck_get_size32(block) * dsf_t.m_channel_count;
//      dsf_t.m_frame_size = 9408;
    }
    dsdiff_skip(4);//4

    if ((dsdiff_read((uint8_t*)&ck, sizeof(ck)) == sizeof(ck) && ck_has_id("data", ck.ckID))) {
        dsf_t.m_data_offset = dsdiff_get_position();
        dsf_t.m_data_size = ck_get_size32(ck.ckDataSize)-12;
        dsf_t.m_frame_count = (uint32_t)(dsf_t.m_data_size / dsf_t.m_frame_size);
        dsf_printf("data_size = %d\n",dsf_t.m_data_size);
        return TRUE;
    }
    dsf_printf("dsf read ckid err\n");
    return FALSE;
}
_ATTR_DSFDEC_BIN_TEXT_
BOOL dsf_read_frame(uint8_t* frame_data, uint32_t* frame_size) {
        *frame_size = dsdiff_read(frame_data, dsf_t.m_frame_size);
        dsf_t.m_decode_frame_count++;
        if (*frame_size > 0) {
            return TRUE;
        }

    if (dsf_t.m_decode_frame_count < dsf_t.m_frame_count)
        dsf_printf("NO.%d frame dsf read frame err!", dsf_t.m_decode_frame_count);
    return FALSE;
}



Dec_Type DEC_DSF;

_ATTR_DSFDEC_BIN_TEXT_
int dsf_decode_init(FILE* pfile)
{

  int dsf_out_len;
  dsf_t.pfile = pfile;

    if (dsf_open())
    {
//      Hifi_Dff_Printf("m_samplerate:%d,channels:%d,m_frame_count:%d,m_frame_size:%d", dsf_t.m_samplerate, dsf_t.m_channel_count, dsf_t.m_frame_count, dsf_t.m_frame_size);
        dsf_printf("m_samplerate:%d\n", dsf_t.m_samplerate);
        dsf_printf("channels:%d\n", dsf_t.m_channel_count);
        dsf_printf("m_frame_count:%d\n", dsf_t.m_frame_count);
        dsf_printf("m_frame_size:%d\n", dsf_t.m_frame_size);
    }
    else
    {
        dsf_printf("DSD IFF OPEN err!");
        return -1;
    }
    dsf_t.pcm_out_channels = dsf_t.m_channel_count;
    dsf_t.dsd_samplerate = dsf_t.m_samplerate;
    switch(dsf_t.dsd_samplerate){
        case 2822400:
            DEC_DSF = DSD64;
            dsf_t.pcm_out_samplerate = 44100;//176400;
            //dsf_printf("DSD64 samp = %d",dsf_t.pcm_out_samplerate);
            break;

        case 5644800:
            dsf_t.pcm_out_samplerate = 44100;//88200;
            DEC_DSF = DSD128;
            //dsf_printf("DSD128 samp = %d",dsf_t.pcm_out_samplerate);
            break;

        case 11289600:
            dsf_t.pcm_out_samplerate = 44100;//88200;
            DEC_DSF = DSD256;
            //dsf_printf("DSD256 samp = %d",dsf_t.pcm_out_samplerate);
            break;

        default:
            DEC_DSF = DSD64;
            dsf_t.pcm_out_samplerate = 176400;
            //dsf_printf("DSD64 samp = %d",dsf_t.pcm_out_samplerate);
            break;
    }
   // dsf_t.pcm_out_samplerate = 176400;
    dsf_out_len = dsf_t.m_frame_size  / ((dsf_t.m_samplerate / dsf_t.pcm_out_samplerate)>>3);
    dsf_out_len = dsf_out_len / dsf_t.m_channel_count;
    dsf_t.bps = 32;

    dsf_t.bitrate = dsf_t.dsd_samplerate*dsf_t.m_channel_count;
     dsf_t.frameLength = ((double)(dsf_t.m_frame_size * 1000 * 8) / (dsf_t.m_channel_count * dsf_t.m_samplerate));

    dsd2pcm_converter_init(&dsd2pcm_converter, dsf_t.pcm_out_channels, dsf_t.dsd_samplerate, dsf_t.pcm_out_samplerate);
  //  dsf_printf("%d ",dsf_out_len);
    return dsf_out_len;
}

_ATTR_DSFDEC_BIN_TEXT_
int dsf_decode(uint8* pcm_out_buf, uint32_t* OutLength)
{
    int  Length;
    dsf_read_frame(dsf_t.dsd_data, &dsf_t.dsd_size);

    if (dsf_t.dsd_size > 0)
    {
        Length = dsd2pcm_converter_convert(&dsd2pcm_converter, dsf_t.dsd_data, dsf_t.dsd_size, pcm_out_buf, dsf_t.bps);
        *OutLength = Length / dsf_t.pcm_out_channels;
    }
    return *OutLength;
}

//FFW/FFD
_ATTR_DSFDEC_BIN_TEXT_
int dsf_seek(int seconds)
{
    unsigned long FFWOffset, FFWSampleNumber;
    int i;
    int FileSize = dsf_t.m_file_size;

    FFWOffset = (((long long)seconds * dsf_t.bitrate) >> 3) + (dsf_t.m_data_offset);
//  FFWOffset = (((long long)seconds * s_FLAC_INFO.bitrate)/8) + (metsize);
//  FFWSampleNumber = (unsigned long)seconds * dsf_t.samplerate;
    //dsf_printf("seek 0x%x ", FFWOffset);
    if (FFWOffset > FileSize)
    {
        goto dsf_seek_fail;
    }
    //if (fseek(dsf_t.pfile, FFWOffset, SEEK_SET) != 0)
    if(RKFIO_FSeek(FFWOffset,SEEK_SET,dsf_t.pfile) != 0)
    {
        goto dsf_seek_fail;
    }
    if ((FFWOffset - dsf_t.m_data_offset)%dsf_t.m_frame_size!=0)
    {
        FFWOffset += dsf_t.m_frame_size - (FFWOffset - dsf_t.m_data_offset) % dsf_t.m_frame_size;
    }
        //dsf_printf("seek校正 0x%x ", FFWOffset);

    if(RKFIO_FSeek(FFWOffset,SEEK_SET,dsf_t.pfile) != 0)
    {
        goto dsf_seek_fail;
    }

dsf_seek_ok :
    dsf_t.m_decode_frame_count = FFWOffset / dsf_t.m_frame_size;
    return 0;
dsf_seek_fail:
    return -1;
}





#endif
#endif
