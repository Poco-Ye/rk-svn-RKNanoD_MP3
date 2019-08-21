#include "SysInclude.h"
#include "audio_main.h"
#include "audio_file_access.h"
#include "ape.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_APE_DECODE
#pragma arm section code = "ApeHDecCode", rodata = "ApeHDecCode", rwdata = "ApeHDecData", zidata = "ApeHDecBss"

extern FILE *ape_file_handle;
extern APEContext apeobj;
extern APEContextdec sobj;
ByteIOContext pbobj;
extern int ID3_len;
int ape_tag_len;
unsigned char indata_buf[512];

void init_ape(void)
{
    Hifi_Ape_MemSet(&apeobj, 0, sizeof(APEContext));
    Hifi_Ape_MemSet(&pbobj, 0, sizeof(ByteIOContext));
    Hifi_Ape_MemSet(&sobj, 0, sizeof(APEContextdec));
    pbobj.buffer = indata_buf;
    apeobj.APE_Frm_NUM = 0;
    apeobj.TimePos = 0;
}

int ape_read_header(void)
{
    u32 test;

    ByteIOContext *pb = &pbobj;
    APEContext *ape = &apeobj;


    uint32_t tag;
    int i;
    int total_blocks;
    int64_t pts;
    /**********以下为ape tag 信息**************/

    ape_tag_len  = ff_ape_parse_tag(pb);
    if (ape_tag_len)
        Hifi_Ape_Printf("ape_tag_len =%d ",ape_tag_len);
    /**********以下为ape header 信息**************/
    ape->junklength = 0;

    tag = get_le32(pb);

    if (tag != MKTAG('M', 'A', 'C', ' '))
    {
        Hifi_Ape_Printf("\n tag error!!!");
        return -1;
    }
    ape->fileversion = get_le16(pb);

    if (ape->fileversion < APE_MIN_VERSION || ape->fileversion > APE_MAX_VERSION)
    {
        Hifi_Ape_Printf("\n file version error!!! Max_version 4120,fileVersion = %d",ape->fileversion);
        return -2;
    }

    if (ape->fileversion >= 3980)
    {
        ape->padding1             = get_le16(pb);
        ape->descriptorlength     = get_le32(pb);
        ape->headerlength         = get_le32(pb);
        ape->seektablelength      = get_le32(pb);
        ape->wavheaderlength      = get_le32(pb);
        ape->audiodatalength      = get_le32(pb);
        ape->audiodatalength_high = get_le32(pb);
        ape->wavtaillength        = get_le32(pb);
        get_buffer(pb, ape->md5, 16);

        /* Skip any unknown bytes at the end of the descriptor.
           This is for future compatibility */
        if (ape->descriptorlength > 52)
            url_fseek(pb, ape->descriptorlength - 52, SEEK_CUR);

        /* Read header data */
        ape->compressiontype      = get_le16(pb);
        ape->formatflags          = get_le16(pb);
        ape->blocksperframe       = get_le32(pb);
        ape->finalframeblocks     = get_le32(pb);
        ape->totalframes          = get_le32(pb);
        ape->bps                  = get_le16(pb);
        ape->channels             = get_le16(pb);
        ape->samplerate           = get_le32(pb);
    }
    else
    {
        ape->descriptorlength = 0;
        ape->headerlength = 32;

        ape->compressiontype      = get_le16(pb);
        ape->formatflags          = get_le16(pb);
        ape->channels             = get_le16(pb);
        ape->samplerate           = get_le32(pb);
        ape->wavheaderlength      = get_le32(pb);
        ape->wavtaillength        = get_le32(pb);
        ape->totalframes          = get_le32(pb);
        ape->finalframeblocks     = get_le32(pb);

        if (ape->formatflags & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL)
        {
            url_fseek(pb, 4, SEEK_CUR); /* Skip the peak level */
            ape->headerlength += 4;
        }

        if (ape->formatflags & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS)
        {
            ape->seektablelength = get_le32(pb);
            ape->headerlength += 4;
            ape->seektablelength *= sizeof(int32_t);
        }
        else
            ape->seektablelength = ape->totalframes * sizeof(int32_t);

        if (ape->formatflags & MAC_FORMAT_FLAG_8_BIT)
            ape->bps = 8;
        else if (ape->formatflags & MAC_FORMAT_FLAG_24_BIT)
            ape->bps = 24;
        else
            ape->bps = 16;

        if (ape->fileversion >= 3950)
            ape->blocksperframe = 73728 * 4;
        else if (ape->fileversion >= 3900 || (ape->fileversion >= 3800  && ape->compressiontype >= 4000))
            ape->blocksperframe = 73728;
        else
            ape->blocksperframe = 9216;

        /* Skip any stored wav header */
        if (!(ape->formatflags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
            url_fskip(pb, ape->wavheaderlength);
    }
    #if 0
    if (ape->samplerate ==192000)
    {
        return -2;
    }
    #endif
    ape->file_size= RKFIO_FLength(ape_file_handle);
    ape->total_blocks= ape->blocksperframe*(ape->totalframes-1)+ape->finalframeblocks;
    ape->file_time= (uint32_t)(((double)(ape->total_blocks) * (double)(1000)) / (double)(ape->samplerate));
    ape->bitrate = (uint32_t)(((long long)(ape->file_size -ID3_len - ape_tag_len) * 8000) / (double)(ape->file_time));
    Hifi_Ape_Printf("版本号 %d \n",ape->fileversion);
    Hifi_Ape_Printf("压缩级 %d \n",ape->compressiontype);
    Hifi_Ape_Printf("帧总数 %d \n",ape->totalframes);
    Hifi_Ape_Printf("每帧块数 %d \n",ape->blocksperframe);
    Hifi_Ape_Printf("位深度 %d \n",ape->bps);
    Hifi_Ape_Printf("采样率 %d \n",ape->samplerate);
    Hifi_Ape_Printf("通道数 %d \n",ape->channels);
    Hifi_Ape_Printf("文件大小 %d \n",ape->file_size);
    Hifi_Ape_Printf("总时长 %d \n",ape->file_time);
    Hifi_Ape_Printf("码率  %d \n",ape->bitrate);

    /**********以下为ape offset 信息**************/
    if (ape->totalframes > (UINT_MAX/36))
    {
        Hifi_Ape_Printf("帧数 %d >2200 ",ape->totalframes);
        return -3;
    }
    ape->frames       = (APEFrame *)av_malloc(ape->totalframes * sizeof(APEFrame));

    if (!ape->frames)
        return -4;

    ape->firstframe   = ape->junklength + ape->descriptorlength + ape->headerlength + ape->seektablelength + ape->wavheaderlength;
    ape->currentframe = 0;


    ape->totalsamples = ape->finalframeblocks;
    if (ape->totalframes > 1)
        ape->totalsamples += ape->blocksperframe * (ape->totalframes - 1);

    if (ape->seektablelength > 0)
    {

        ape->seektable = (uint32_t *)av_malloc(ape->totalframes*4);
        for (i = 0; i < ape->totalframes; i++)
        {
            ape->seektable[i] = get_le32(pb) + ID3_len;
        }
        url_fskip(pb,(ape->seektablelength-(ape->totalframes*4)) );

    }

    ape->frames[0].pos     = ape->firstframe + ID3_len;
    ape->frames[0].nblocks = ape->blocksperframe;
    ape->frames[0].skip    = 0;
    for (i = 1; i < ape->totalframes; i++)
    {
        ape->frames[i].pos      = ape->seektable[i]; //ape->frames[i-1].pos + ape->blocksperframe;
        ape->frames[i].nblocks  = ape->blocksperframe;
        ape->frames[i - 1].size = ape->frames[i].pos - ape->frames[i - 1].pos;
        ape->frames[i].skip     = (ape->frames[i].pos - ape->frames[0].pos) & 3;
    }

    ape->frames[ape->totalframes - 1].size    = ape->finalframeblocks * 4;
    ape->frames[ape->totalframes - 1].nblocks = ape->finalframeblocks;

    for (i = 0; i < ape->totalframes; i++)
    {
        if (ape->frames[i].skip)
        {
            ape->frames[i].pos  -= ape->frames[i].skip;
            ape->frames[i].size += ape->frames[i].skip;
        }
        ape->frames[i].size = (ape->frames[i].size + 3) & ~3;
    }

    return 1;
}
#pragma arm section code
#endif
#endif
