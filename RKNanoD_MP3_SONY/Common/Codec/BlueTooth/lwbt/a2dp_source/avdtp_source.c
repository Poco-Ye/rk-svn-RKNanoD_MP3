
#include "lwbt.h"
#include "SysInclude.h"
#include "FsInclude.h"
#include "File.h"
#include "FDT.h"
#include "sdp.h"
#include "lwbt_memp.h"
#include "lwbtopts.h"
#include "lwbtdebug.h"
#include "pbuf.h"
#include "inet.h"
#include "l2cap.h"
#include "avdtp_source.h"
#include "avctp_source.h"

#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_

#define AVDTP_CMD_RETRY 5
#define AVDTP_CMD_TIMEOUT 300
__packed struct AvdtpTmr
{
    uint  Tick;
    uint8 enable;
    uint8 retry;
    uint8 old_state;
};

#define MEDIA_CH_SETUP 1
#define MEDIA_CH_NO_SETUP 0


//#define COMMAND_LEN_DISCOVER 2
#define COMMAND_LEN_GET_CAPABILITIES  3
#define COMMAND_LEN_OPEN   3
#define COMMAND_LEN_START  3
#define COMMAND_LEN_CLOSE  3
#define COMMAND_LEN_SUSPEND 3
#define COMMAND_LEN_ABORT   3


_ATTR_LWBT_BSS_ struct AvdtpTmr AvdtpStartTmr;
_ATTR_LWBT_BSS_ struct AvdtpTmr AvdtpSuspendTmr;


_ATTR_LWBT_BSS_ struct avdtp_notify avdtp_notify_pcb;

_ATTR_LWBT_BSS_ struct a2dp_pcb *a2dp_pcbs;
_ATTR_LWBT_BSS_ struct l2cap_pcb  *a2dp_meida_pcbs;

_ATTR_LWBT_BSS_ uint16 avdtp_State_s;
_ATTR_LWBT_BSS_ uint8 avdtp_media_ch_state;
_ATTR_LWBT_BSS_ uint8 avdtp_recv_cmd;
_ATTR_LWBT_BSS_ uint8 avdtp_recv_rsp;
_ATTR_LWBT_BSS_ uint8 avdtp_rsp_prev;
_ATTR_LWBT_BSS_ uint8 avdtp_set_conf_retry;
_ATTR_LWBT_BSS_ uint8 avdtp_get_capabilities_retry;

_ATTR_LWBT_BSS_ uint16 avdtp_sequenceNumber_nxt;

extern SYSTICK_LIST AvdtpCheckStateTimer;
extern SYSTICK_LIST AvdtpDiscoverTimer;
extern SYSTICK_LIST AvdtpMediaSetUpTimer;
extern SYSTICK_LIST AvdtpGetGapabilitiesTimer;
//A2DP v1.2¶ÔÓ¦avdtp °æ±¾v1.0
_ATTR_LWBT_DATA_ static uint8 a2dp_service_record[] =
{
    0x35, 0x8,
    0x9, 0x0, 0x0, 0xa, 0x0, 0x1, 0xff, 0xff, /* Service record handle attribute */
    0x35, 0x8,
    0x9, 0x0, 0x1, 0x35, 0x3, 0x19, 0x11, 0x0A, /* Service class ID list attribute */
    0x35, 0x8,
    0x9, 0x0, 0x02,0x0a, 0x00,0x00, 0x00, 0x04, /*Attribute: Service Record State */
    0x35, 0x15,
    0x9, 0x0, 0x4,0x35, 0x10, /* Protocol descriptor list attribute */
    0x35, 0x6, 0x19, 0x01, 0x00, 0x09, 0x00, 0x19,
    0x35, 0x6, 0x19, 0x00, 0x19, 0x09, 0x01, 0x03, /* Protocol descriptor list attribute */
    0x35, 0x0E,                                 /*Attribute: Language Base Attribute ID List*/
    0x09, 0x00, 0x06, 0x35, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00,
    0x35, 0xD,
    0x9, 0x00, 0x09, 0x35,0x08,0x35, 0x06, 0x19,0x11,0x0d,0x09,0x01,0x03, /* BluetoothProfileDescriptorList */
    0x35, 0x1B,                                    /* Attribute: Service Name */
    0x09, 0x01, 0x00, 0x25, 0x16, 0x47, 0x41, 0x56, 0x44, 0x20, 0x41, 0x75,
    0x64, 0x69, 0x6f, 0x20, 0x53, 0x69, 0x6e, 0x6b, 0x20, 0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65,

    //0x35, 0x8,
    //0x09, 0x03, 0x11, 0x35,0x03, 0x09, 0x00, 0x01 /* Supported Features attribute */
};



_ATTR_LWBT_INIT_CODE_
void avdtp_streaming_notify(void(*streaming_notify)(void))
{
    avdtp_notify_pcb.streaming_notify = streaming_notify;
}


_ATTR_LWBT_INIT_CODE_
void avdtp_suspend_notify(void(*suspend_notify)(void))
{
    avdtp_notify_pcb.suspend_notify = suspend_notify;
}


_ATTR_LWBT_INIT_CODE_
void avdtp_close_notify(void(*close_notify)(void))
{
    avdtp_notify_pcb.close_notify = close_notify;
}

_ATTR_LWBT_INIT_CODE_
void avdtp_abort_notify(void(*abort_notify)(void))
{
    avdtp_notify_pcb.abort_notify = abort_notify;
}

_ATTR_LWBT_INIT_CODE_
void avdtp_connected_notify(void(*connected_notify)(void))
{
    avdtp_notify_pcb.connected_notify = connected_notify;
}


_ATTR_LWBT_CODE_
uint16 get_avdtp_state(void)
{
    return avdtp_State_s;
}

_ATTR_LWBT_CODE_
uint32 avdtp_get_profile_versions(void)
{
    return 0x102;
}

_ATTR_LWBT_CODE_
uint32 a2dp_get_profile_versions(void)
{
    return 0x103;
}

_ATTR_LWBT_CODE_
uint8 avdtp_get_codec_type(void)
{
    struct a2dp_pcb *pcb;
    pcb = a2dp_pcbs;
    return pcb->Remotecodec.MediaCodecType;
}


_ATTR_LWBT_CODE_
uint8 avdtp_get_sbc_bitpool(void)
{
    struct a2dp_pcb *pcb;
    pcb = a2dp_pcbs;
    return pcb->Remotecodec.MaximumBitpoolValue;
}

#if 1
_ATTR_LWBT_CODE_
uint8 avdtp_get_sbc_chmode(void)
{
    struct a2dp_pcb *pcb;
    pcb = a2dp_pcbs;

    if(pcb->Remotecodec.Channel_Mode & SBC_CHANNEL_MODE_JOINT_STEREO)
    {
        return 3;
    }
    else if(pcb->Remotecodec.Channel_Mode & SBC_CHANNEL_MODE_STEREO)
    {
        return 2;
    }
    else if(pcb->Remotecodec.Channel_Mode & SBC_CHANNEL_MODE_DUAL_CHANNEL)
    {
        return 1;
    }
    else if(pcb->Remotecodec.Channel_Mode & SBC_CHANNEL_MODE_MONO)
    {
        return 0;
    }

}

_ATTR_LWBT_CODE_
uint8 avdtp_get_sbc_block_len(void)
{
    uint8 block_len;
    struct a2dp_pcb *pcb;
    pcb = a2dp_pcbs;
    if(pcb->Remotecodec.Block_Length & SBC_BLOCK_LENGTH_16)
    {
        return 3;
    }
    else if(pcb->Remotecodec.Block_Length & SBC_BLOCK_LENGTH_12)
    {
        return 2;
    }
    else if(pcb->Remotecodec.Block_Length & SBC_BLOCK_LENGTH_8)
    {
        return 1;
    }
    else if(pcb->Remotecodec.Block_Length & SBC_BLOCK_LENGTH_4)
    {
        return 0;
    }
    return 3;
}

_ATTR_LWBT_CODE_
uint8 avdtp_get_sbc_subbands(void)
{
    struct a2dp_pcb *pcb;
    pcb = a2dp_pcbs;
    return pcb->Remotecodec.Subbands;

    if(pcb->Remotecodec.Subbands & SBC_SUBBANDS_8)
    {
        return 8;
    }
    else if(pcb->Remotecodec.Subbands & SBC_SUBBANDS_4)
    {
        return 4;
    }

    return 8;
}
#endif

_ATTR_LWBT_CODE_
uint16 avdtp_next_sequenceNumber(void)
{
    return avdtp_sequenceNumber_nxt++;
}



_ATTR_LWBT_CODE_
static bool avdtp_Save_Remote_Codec_info(struct avdtp_Command_CapMidea_Data *Cap_Data)
{
    struct a2dp_pcb *pcb;
    uint Bitpooltemp;
    pcb = a2dp_pcbs;
    pcb->Remotecodec.MediaCodecType = Cap_Data->MediaCodecType;
    //pcb->Remotecodec.Sampling_Frequency = (Cap_Data->SF_CM >> 4);
    pcb->Remotecodec.Sampling_Frequency = (Cap_Data->SF_CM & 0xF0);
    pcb->Remotecodec.Channel_Mode = (Cap_Data->SF_CM & 0xF);
    //pcb->Remotecodec.Block_Length = (Cap_Data->BL_BS_AM >> 4);
    pcb->Remotecodec.Block_Length = (Cap_Data->BL_BS_AM& 0xF0);
    //pcb->Remotecodec.Subbands = ((Cap_Data->BL_BS_AM >> 2)& 0x3);
    pcb->Remotecodec.Subbands = (Cap_Data->BL_BS_AM & 0x0C);
    pcb->Remotecodec.Allocation_Method = (Cap_Data->BL_BS_AM & 0x3);
    pcb->Remotecodec.MinimumBitpoolValue = Cap_Data->MinimumBitpoolValue;
    pcb->Remotecodec.MaximumBitpoolValue = Cap_Data->MaximumBitpoolValue;

    Bitpooltemp = Cap_Data->MaximumBitpoolValue;
    Bitpooltemp = (Bitpooltemp > 0x35) ? 0x35 : Bitpooltemp;
    a2dp_pcbs->Remotecodec.MaximumBitpoolValue = Bitpooltemp;


    #if 0
    if(Bitpooltemp <= 18)
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 18;
    }
    else if(Bitpooltemp >=19 && Bitpooltemp < 29)
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 19;
    }
    else if(Bitpooltemp >=29 && Bitpooltemp < 31)
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 29;
    }
    else if(Bitpooltemp >=31 && Bitpooltemp < 33)
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 31;
    }
    else if(Bitpooltemp >=33 && Bitpooltemp < 35)
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 33;
    }
    else if(Bitpooltemp >=35 && Bitpooltemp < 51)
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 35;
    }
    else if(Bitpooltemp >=51 && Bitpooltemp < 53)
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 51;
    }
    else
    {
        a2dp_pcbs->Remotecodec.MaximumBitpoolValue  = 53;
    }
    #endif
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("Codec_info:\n"));
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("Channel_Mode: %d\n",pcb->Remotecodec.Channel_Mode));
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("Block_Length: %d\n",pcb->Remotecodec.Block_Length));
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("Subbands: %d\n",pcb->Remotecodec.Subbands));
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("MinimumBitpoolValue: %d\n",pcb->Remotecodec.MinimumBitpoolValue));
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("MaximumBitpoolValue: %d\n",pcb->Remotecodec.MaximumBitpoolValue));
    return TRUE;
}


_ATTR_LWBT_CODE_
static int avdtp_check_config_info(struct l2cap_pcb *l2_pcb, struct a2dp_pcb *pcb,
                                struct pbuf *p_cap, struct avdtp_single_header *header)
{
    uint8* payload;
    int input_len,i;
    int error;
    uint8 acp_id;
    struct Remote_SBC_CODEC_info Remotecodec = {0};
    struct avdtp_config_reject reject;

    struct avdtp_Command_Cap_Data *Cap_Data = p_cap->payload;
    input_len = p_cap->len -1;

    payload = (char*)Cap_Data + 1;

    i = 0;
    acp_id = *(((uint8*)p_cap->payload)-1);
    acp_id = (acp_id>>2)-1;
    while(input_len)
    {
        reject.service_category = payload[i];
        if((payload[i] != AVDTP_MEDIA_TRANSPORT) && (payload[i] != AVDTP_MEDIA_CODEC))
        {
            error = AVDTP_BAD_SERV_CATEGORY;
            goto err;
        }
        if((payload[i] == AVDTP_MEDIA_TRANSPORT) && (payload[i+1] != 0))
        {
            error = AVDTP_BAD_MEDIA_TRANSPORT_FORMAT;
            goto err;
        }

        i = 2+payload[i+1];
        if(input_len >= i)
        {
            input_len -= i;
        }
        else
        {
            error = AVDTP_BAD_ROHC_FORMAT;
            goto err;
        }
    }
    Remotecodec.MediaType = Cap_Data->MediaType>>4;
    //pcb->Remotecodec.Sampling_Frequency = (Cap_Data->SF_CM >> 4);
    Remotecodec.Sampling_Frequency = (Cap_Data->SF_CM & 0xF0);
    Remotecodec.Channel_Mode = (Cap_Data->SF_CM & 0xF);
    //pcb->Remotecodec.Block_Length = (Cap_Data->BL_BS_AM >> 4);
    Remotecodec.Block_Length = (Cap_Data->BL_BS_AM& 0xF0);
    //pcb->Remotecodec.Subbands = ((Cap_Data->BL_BS_AM >> 2)& 0x3);
    Remotecodec.Subbands = (Cap_Data->BL_BS_AM & 0x0C);
    Remotecodec.Allocation_Method = (Cap_Data->BL_BS_AM & 0x3);
    Remotecodec.MinimumBitpoolValue = Cap_Data->MinimumBitpoolValue;
    Remotecodec.MaximumBitpoolValue = Cap_Data->MaximumBitpoolValue;

    reject.service_category = AVDTP_MEDIA_CODEC;

    if(Remotecodec.MediaType != AVDTP_MEDIA_TYPE_AUDIO)
    {
        //printf("Remotecodec.MediaType = %d\n",Remotecodec.MediaType);
        error = AVDTP_BAD_CP_FORMAT;
        goto err;
    }

    if(Remotecodec.MediaCodecType != AVDTP_AUDIO_CODEC_SBC)
    {
       printf("Remotecodec.MediaCodecType = %d\n",Remotecodec.MediaCodecType);
       error = AVDTP_BAD_CP_FORMAT;
       goto err;
    }
    //printf("acp_id = %d\n",acp_id);

    if((Remotecodec.Sampling_Frequency
        & pcb->Localcodec[acp_id].Sampling_Frequency) == NULL)
    {
        //printf("Remotecodec.Sampling_Frequency = %d\n",Remotecodec.Sampling_Frequency);
        //printf("Localcodec.Sampling_Frequency = %d\n",pcb->Localcodec[acp_id].Sampling_Frequency);
        error = AVDTP_BAD_CP_FORMAT;
        goto err;
    }

    if((Remotecodec.Channel_Mode & pcb->Localcodec[acp_id].Channel_Mode) == NULL)
    {
        //printf("Remotecodec.Channel_Mode = %d\n",Remotecodec.Channel_Mode);
        //printf("Localcodec.Channel_Mode = %d\n",pcb->Localcodec[acp_id].Channel_Mode);
        error = AVDTP_BAD_CP_FORMAT;
        goto err;

    }

    if((Remotecodec.Block_Length & pcb->Localcodec[acp_id].Block_Length) == NULL)
    {
        //printf("Remotecodec.Block_Length = %d\n",Remotecodec.Block_Length);
        //printf("Localcodec.Block_Length = %d\n",pcb->Localcodec[acp_id].Block_Length);
        error = AVDTP_BAD_CP_FORMAT;
        goto err;
    }

    if((Remotecodec.Subbands & pcb->Localcodec[acp_id].Subbands) == NULL)
    {
        //printf("Remotecodec.Subbands = %d\n",Remotecodec.Subbands);
        //printf("Localcodec.Subbands = %d\n",pcb->Localcodec[acp_id].Subbands);
        error = AVDTP_BAD_CP_FORMAT;
        goto err;
    }

    if(Remotecodec.MaximumBitpoolValue >  pcb->Localcodec[acp_id].MaximumBitpoolValue)
    {
        //printf("Remotecodec.MaximumBitpoolValue = %d\n",Remotecodec.MaximumBitpoolValue);
        //printf("Localcodec.MaximumBitpoolValue = %d\n",pcb->Localcodec[acp_id].MaximumBitpoolValue);
        error = AVDTP_BAD_CP_FORMAT;
        goto err;
    }
    pcb->local_sep[acp_id].InUse = AVDTP_SEPID_INUSE;
    a2dp_pcbs->select_seid = acp_id+1;
    return 0;


err:

    reject.error_code = error;
    avdtp_reject_rsp(l2_pcb,header,&reject,sizeof(struct avdtp_config_reject));

    return error;

}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_free():
 *
 * Free the SDP protocol control block.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void avdtp_free(struct a2dp_pcb *pcb)
{
    lwbt_memp_free(MEMP_A2DP_PCB, pcb);
    pcb = NULL;
}
/*-----------------------------------------------------------------------------------*/
/*
 * avdtp_new():
 *
 * Creates a new SDP protocol control block but doesn't place it on
 * any of the SDP PCB lists.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
struct a2dp_pcb *avdtp_new()
{
    struct a2dp_pcb *pcb;

    pcb = lwbt_memp_malloc(MEMP_A2DP_PCB);
    if(pcb != NULL)
    {
        memset(pcb, 0, sizeof(struct a2dp_pcb));
        //pcb->l2cappcb = l2cappcb;
        return pcb;
    }
    return NULL;


}


_ATTR_LWBT_CODE_
err_t avdtp_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header, uint8 command)
{
    err_t ret;
    struct pbuf *start_cmd_pbuf = NULL;
    struct start_cmd *start_cmd_s;
    struct avdtp_Command_Data *open_a2dpData;
    ret = ERR_OK;

    //DEBUG("avdtp cmd = %d\n", command);
    start_cmd_pbuf = pbuf_alloc(PBUF_RAW, 3, PBUF_RAM);
    if (start_cmd_pbuf == NULL)
    {
        LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("pbuf alloc fail:\n"));
        return FALSE;
    }
    start_cmd_s = start_cmd_pbuf->payload;
    start_cmd_s->MPT = ((0 << 4) | (0<< 2) | AVDTP_MSG_TYPE_COMMAND);
    start_cmd_s->SIRFA = command;
    start_cmd_s->Acp_SEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);

    ret = l2ca_datawrite(pcb, start_cmd_pbuf);
    pbuf_free(start_cmd_pbuf);
    return ret;

}

#if 0
_ATTR_LWBT_CODE_
static bool avdtp_start_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *start_cmd_pbuf = NULL;
    struct start_cmd *start_cmd_s;
    struct avdtp_Command_Data *open_a2dpData;


    start_cmd_pbuf = pbuf_alloc(PBUF_RAW, 3, PBUF_RAM);
    if (start_cmd_pbuf == NULL)
        return FALSE;

    start_cmd_s = start_cmd_pbuf->payload;
    start_cmd_s->MPT = ((0 << 4) | (0<< 2) | AVDTP_MSG_TYPE_COMMAND);
    start_cmd_s->SIRFA = AVDTP_START;
    start_cmd_s->Acp_SEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);

    l2ca_datawrite(pcb, start_cmd_pbuf);
    pbuf_free(start_cmd_pbuf);
    return TRUE;
}
_ATTR_LWBT_CODE_
static bool avdtp_Suspend_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *Suspend_cmd_pbuf = NULL;
    struct Suspend_cmd *Suspend_cmd_s;
    struct avdtp_Command_Data *open_a2dpData;


    Suspend_cmd_pbuf = pbuf_alloc(PBUF_RAW, 3, PBUF_RAM);
    if (Suspend_cmd_pbuf == NULL)
        return FALSE;

    Suspend_cmd_s = Suspend_cmd_pbuf->payload;
    Suspend_cmd_s->MPT = ((0 << 4) | (0<< 2) | AVDTP_MSG_TYPE_COMMAND);
    Suspend_cmd_s->SIRFA = AVDTP_SUSPEND;
    Suspend_cmd_s->Acp_SEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);

    l2ca_datawrite(pcb, Suspend_cmd_pbuf);
    pbuf_free(Suspend_cmd_pbuf);
    return TRUE;
}


_ATTR_LWBT_CODE_
static bool avdtp_close_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *Suspend_cmd_pbuf = NULL;
    struct Suspend_cmd *Suspend_cmd_s;
    struct avdtp_Command_Data *open_a2dpData;


    Suspend_cmd_pbuf = pbuf_alloc(PBUF_RAW, 3, PBUF_RAM);
    if (Suspend_cmd_pbuf == NULL)
        return FALSE;

    Suspend_cmd_s = Suspend_cmd_pbuf->payload;
    Suspend_cmd_s->MPT = ((0 << 4) | (0<< 2) | AVDTP_MSG_TYPE_COMMAND);
    Suspend_cmd_s->SIRFA = AVDTP_CLOSE;
    Suspend_cmd_s->Acp_SEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);

    l2ca_datawrite(pcb, Suspend_cmd_pbuf);
    pbuf_free(Suspend_cmd_pbuf);
    return TRUE;
}

_ATTR_LWBT_CODE_
static bool avdtp_abort_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *Suspend_cmd_pbuf = NULL;
    struct Suspend_cmd *Suspend_cmd_s;
    struct avdtp_Command_Data *open_a2dpData;


    Suspend_cmd_pbuf = pbuf_alloc(PBUF_RAW, 3, PBUF_RAM);
    if (Suspend_cmd_pbuf == NULL)
        return FALSE;

    Suspend_cmd_s = Suspend_cmd_pbuf->payload;
    Suspend_cmd_s->MPT = ((0 << 4) | (0<< 2) | AVDTP_MSG_TYPE_COMMAND);
    Suspend_cmd_s->SIRFA = AVDTP_ABORT;
    Suspend_cmd_s->Acp_SEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);

    l2ca_datawrite(pcb, Suspend_cmd_pbuf);
    pbuf_free(Suspend_cmd_pbuf);
    return TRUE;
}








_ATTR_LWBT_CODE_
static bool avdtp_open_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *open_resp_pbuf = NULL;
    struct open_cmd *open_cmd_s;
    struct avdtp_Command_Data *open_a2dpData;


    open_resp_pbuf = pbuf_alloc(PBUF_RAW, 3, PBUF_RAM);
    if (open_resp_pbuf == NULL)
        return FALSE;

    open_cmd_s = open_resp_pbuf->payload;
    open_cmd_s->MPT = ((4 << 4) | (0<< 2) | AVDTP_MSG_TYPE_COMMAND);
    open_cmd_s->SIRFA = AVDTP_OPEN;
    open_cmd_s->Acp_SEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);

    l2ca_datawrite(pcb, open_resp_pbuf);
    pbuf_free(open_resp_pbuf);
    return TRUE;
}

_ATTR_LWBT_CODE_
static bool avdtp_get_capabilities_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{

    uint8 *payload;
    int ret = 0;
    struct pbuf *get_cap_cmd_pbuf = NULL;
    struct get_cap_cmd *cap_cmd;

    get_cap_cmd_pbuf = pbuf_alloc(PBUF_RAW, 3, PBUF_RAM);

    if (get_cap_cmd_pbuf == NULL)
        return FALSE;
    /* Check for minimum required packet size includes:
     *   1. getcap resp header
     *   2. media transport capability (2 bytes)
     *   3. media codec capability type + length (2 bytes)
     *   4. the actual media codec elements
     * */
    cap_cmd = get_cap_cmd_pbuf->payload;
    cap_cmd->MPT = ((2 << 4) | (0 << 2) | AVDTP_MSG_TYPE_COMMAND);
    cap_cmd->SIRFA = AVDTP_GET_CAPABILITIES;
    cap_cmd->RFAINUSESEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);

    ret = l2ca_datawrite(pcb, get_cap_cmd_pbuf);
    pbuf_free(get_cap_cmd_pbuf);
    return TRUE;
}

#endif

static avdtp_reject_rsp(struct l2cap_pcb *pcb,struct avdtp_single_header *header,
                        char *rspdata, int rsp_len)
{
    uint8 *payload;
    int ret = 0;
    struct pbuf *rsp_pbuf = NULL;
    struct avdtp_single_header *header_tmp;

    rsp_pbuf  = pbuf_alloc(PBUF_RAW, sizeof(struct avdtp_single_header)+rsp_len, PBUF_RAM);

    if (rsp_pbuf == NULL)
        return FALSE;

    memset(rsp_pbuf->payload,0,sizeof(struct avdtp_single_header)+rsp_len);
    /* Add PDU header to packet */
    header_tmp = rsp_pbuf->payload;
    header_tmp->message_type = AVDTP_MSG_TYPE_REJECT;
    header_tmp->packet_type = AVDTP_PKT_TYPE_SINGLE;
    header_tmp->transaction = header->transaction;
    header_tmp->signal_id = header->signal_id;
    payload = (char *)rsp_pbuf->payload + sizeof(struct avdtp_single_header);
    if(rsp_len != 0)
    {
       memcpy(payload,rspdata, rsp_len);
    }
    ret = l2ca_datawrite(pcb, rsp_pbuf);
    pbuf_free(rsp_pbuf);
    return ret;    

}


static avdtp_general_reject_rsp(struct l2cap_pcb *pcb,struct avdtp_single_header *header
                        )
{
    uint8 *payload;
    int ret = 0;
    struct pbuf *rsp_pbuf = NULL;
    struct avdtp_single_header *header_tmp;

    rsp_pbuf  = pbuf_alloc(PBUF_RAW, sizeof(struct avdtp_single_header), PBUF_RAM);

    if (rsp_pbuf == NULL)
        return FALSE;

    memset(rsp_pbuf->payload,0,sizeof(struct avdtp_single_header));
    /* Add PDU header to packet */
    header_tmp = rsp_pbuf->payload;
    header_tmp->message_type =AVDTP_MSG_TYPE_COMMAND; //AVDTP_MSG_TYPE_REJECT;
    header_tmp->packet_type = AVDTP_PKT_TYPE_SINGLE;
    header_tmp->transaction = header->transaction;
    header_tmp->signal_id = 0x00;//header->signal_id;
    payload = (char *)rsp_pbuf->payload + sizeof(struct avdtp_single_header);

    ret = l2ca_datawrite(pcb, rsp_pbuf);
    pbuf_free(rsp_pbuf);
    return ret;
}

#define CHECK_SEID_FLAG_EXIST (1<<0)
#define CHECK_SEID_FLAG_USED  (1<<1)
#define CHECK_SEID_FLAG_ROLE  (1<<2)
static err_t avdtp_check_seid(int seid, int flag)
{
    int i;

    for(i=0;i<AVDTP_SEP_MAX;i++)
    {
    //rk_printf("&&&&&&&&&&& sed = %d = %d,flag = %d",a2dp_pcbs->local_sep[i].SEID,seid, flag);

//    uint8 SEID ;
//    uint8 TSEP;
 //   uint8 Media_Type;
//    bool delay_reporting;

       rk_printf("i = %d, Inuse = %d, tsep = %d, media_type = %d, delay_reporting = %d",i,
        a2dp_pcbs->local_sep[i].InUse,a2dp_pcbs->local_sep[i].TSEP,a2dp_pcbs->local_sep[i].Media_Type,
        a2dp_pcbs->local_sep[i].delay_reporting);

        //rk_printf("seid = %d, role = %d", seid, a2dp_pcbs->role);

        if(a2dp_pcbs->local_sep[i].SEID == seid)
        {
            if(flag == CHECK_SEID_FLAG_EXIST)
            {
                return ERR_OK;
            }
#if 1
            if(flag == CHECK_SEID_FLAG_ROLE)
            {
                 if(!(a2dp_pcbs->local_sep[i].InUse))
                    return ERR_OK;
            }
#endif
            if(flag == CHECK_SEID_FLAG_USED)
            {
                if(a2dp_pcbs->local_sep[i].InUse)
                    return ERR_OK;
            }
        }
    }

    return ERR_VAL;
}




_ATTR_LWBT_CODE_
static bool avdtp_discover_cmd(struct l2cap_pcb *pcb)
{
    uint8 *payload;
    int ret = 0;
    bool getcap_pending = FALSE;
    struct pbuf *discover_cmd_pbuf = NULL;
    struct discover_cmd *cmd;

    discover_cmd_pbuf  = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);

    if (discover_cmd_pbuf == NULL)
        return FALSE;

    /* Add PDU header to packet */
    cmd = discover_cmd_pbuf->payload;
    cmd->MPT = ((1 << 4) | (0 << 2) | AVDTP_MSG_TYPE_COMMAND);
    cmd->SIRFA = AVDTP_DISCOVER;

    ret = l2ca_datawrite(pcb, discover_cmd_pbuf);
    pbuf_free(discover_cmd_pbuf);

    return ret;
}



_ATTR_LWBT_CODE_
static bool avdtp_set_configuration_cmd(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *set_configuration_cmd_pbuf = NULL;
    struct set_configuration_cmd *configuration_cmd;
    struct get_cap_resp *config_Data;
    struct avdtp_Command_Data *config_a2dpData;

    config_Data = p->payload;
    //pbuf_header(p, -4);

    set_configuration_cmd_pbuf = pbuf_alloc(PBUF_RAW, 14, PBUF_RAM);
    if (set_configuration_cmd_pbuf == NULL)
        return FALSE;

    configuration_cmd = set_configuration_cmd_pbuf->payload;
    configuration_cmd->MPT = ((3 << 4) | (0<< 2) | AVDTP_MSG_TYPE_COMMAND);
    configuration_cmd->SIRFA = AVDTP_SET_CONFIGURATION;
    configuration_cmd->Acp_SEID = (a2dp_pcbs->Remotecodec.Int_SEID << 2);
    configuration_cmd->Int_SEID = (a2dp_pcbs->select_seid << 2);
    a2dp_pcbs->local_sep[a2dp_pcbs->select_seid-1].InUse = AVDTP_SEPID_INUSE;
    configuration_cmd->ServiceCategory = AVDTP_MEDIA_TRANSPORT;
    configuration_cmd->LOSC = 0;

    configuration_cmd->ServiceCategory1 = AVDTP_MEDIA_CODEC;
    configuration_cmd->LOSC1 = 6;
    configuration_cmd->MediaType = a2dp_pcbs->local_sep[a2dp_pcbs->select_seid-1].Media_Type;
    configuration_cmd->MediaCodecType = AVDTP_AUDIO_CODEC_SBC;
    if(header != NULL)
    {
        configuration_cmd->SF_CM = ((a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Sampling_Frequency) | (a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Channel_Mode));

        configuration_cmd->BL_BS_AM = ((a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Block_Length) |
                                       (a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Subbands) |
                                       (a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Allocation_Method));
        configuration_cmd->MinimumBitpoolValue = a2dp_pcbs->Remotecodec.MinimumBitpoolValue;
        configuration_cmd->MaximumBitpoolValue = a2dp_pcbs->Remotecodec.MaximumBitpoolValue;
    }
    else
    {

        if(AudioGetSbcChannelMode() == 3)//0 MONO ; 1 DUAL CHANNEL; 2 STEREO; 3 JOINT STEREO;
        {
            configuration_cmd->SF_CM = 0x21;/*44100 SBC_CHANNEL_MODE_STEREO*/
        }
        else
        {
            configuration_cmd->SF_CM = 0x22;/*44100 SBC_CHANNEL_MODE_STEREO*/
        }

        configuration_cmd->BL_BS_AM = 0x15;
        configuration_cmd->MinimumBitpoolValue = a2dp_pcbs->Remotecodec.MinimumBitpoolValue;
        configuration_cmd->MaximumBitpoolValue = a2dp_pcbs->Remotecodec.MaximumBitpoolValue;
    }
    //configuration_cmd->MinimumBitpoolValue = a2dp_pcbs->Localcodec.MinimumBitpoolValue;
    //configuration_cmd->MaximumBitpoolValue = a2dp_pcbs->Localcodec.MaximumBitpoolValue;

    l2ca_datawrite(pcb, set_configuration_cmd_pbuf);
    pbuf_free(set_configuration_cmd_pbuf);
    return TRUE;
}


_ATTR_LWBT_CODE_
void avdtp_start(void)
{
    err_t ret;
    //DEBUG("avdtp_start,avdtp_State_s=%d\n", avdtp_State_s);
    HciServeIsrDisable();
    if(avdtp_State_s ==  AVDTP_STATE_SUSPEND || avdtp_State_s == AVDTP_STATE_OPEN)
    {
        //avdtp_start_cmd(a2dp_pcbs->l2cappcb, NULL, NULL);

        ret = avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_START);
        //if(ret == ERR_OK)
        {
            avdtp_State_s = AVDTP_STATE_WAIT_START_COMPLETE;

            AvdtpStartTmr.enable = 1;
            AvdtpStartTmr.Tick = GetSysTick();
            AvdtpStartTmr.retry = AVDTP_CMD_RETRY;
            AvdtpStartTmr.old_state = avdtp_State_s;
        }
    }
    else if(avdtp_State_s == AVDTP_STATE_WAIT_SUSPEND_COMPLETE)
    {
        avdtp_State_s = AVDTP_STATE_GOTO_START_QUEUE;
    }

    HciServeIsrEnable();
    //DEBUG("Leaving avdtp_start\n");
}

_ATTR_LWBT_CODE_
void avdtp_suspend(void)
{
    err_t ret;
    //DEBUG("avdtp_suspend\n");
    HciServeIsrDisable();
    if(avdtp_State_s == AVDTP_STATE_STREAMING)
    {
        //avdtp_Suspend_cmd(a2dp_pcbs->l2cappcb, NULL, NULL);
        ret = avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_SUSPEND);
        //if(ret == ERR_OK)
        {
            avdtp_State_s = AVDTP_STATE_WAIT_SUSPEND_COMPLETE;

            AvdtpSuspendTmr.enable = 1;
            AvdtpSuspendTmr.Tick = GetSysTick();
            AvdtpSuspendTmr.retry = AVDTP_CMD_RETRY;
            AvdtpSuspendTmr.old_state = avdtp_State_s;
        }
    }
    else if(avdtp_State_s == AVDTP_STATE_WAIT_START_COMPLETE)
    {
        avdtp_State_s = AVDTP_STATE_GOTO_SUSPEND_QUEUE;
    }
    HciServeIsrEnable();
    //DEBUG("Leaving avdtp_suspend\n");
}


_ATTR_LWBT_CODE_
static bool avdtp_get_all_capabilities_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    uint8 seid;
    uint8 *payload;
    int ret = 0;
    struct avdtp_Command_Data *a2dpData;
    struct pbuf *get_cap_resp_pbuf = NULL;
    struct get_cap_resp *cap_resp;

    if(p->len != 1)
    {
        uint8 error_code;
        error_code = AVDTP_BAD_LENGTH;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;

    }

    a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    //if(a2dpData->Acp_SEID != AVDTP_SEP_ID)
    //    return FALSE;
    if(avdtp_check_seid(a2dpData->Acp_SEID,CHECK_SEID_FLAG_EXIST))
    {
        uint8 error_code;
        error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }
#ifdef A2DP_ACC
    get_cap_resp_pbuf = pbuf_alloc(PBUF_RAW, 26, PBUF_RAM);
#else
    get_cap_resp_pbuf = pbuf_alloc(PBUF_RAW, 12, PBUF_RAM);
#endif

    if (get_cap_resp_pbuf == NULL)
        return FALSE;
    /* Check for minimum required packet size includes:
     *   1. getcap resp header
     *   2. media transport capability (2 bytes)
     *   3. media codec capability type + length (2 bytes)
     *   4. the actual media codec elements
     * */
    cap_resp = get_cap_resp_pbuf->payload;
    cap_resp->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    cap_resp->SIRFA = AVDTP_GET_ALL_CAPABILITIES;
    cap_resp->ServiceCategory = AVDTP_MEDIA_TRANSPORT;
    cap_resp->LOSC = 0;
#if 1
    cap_resp->ServiceCategory1 = AVDTP_MEDIA_CODEC;
    cap_resp->LOSC1 = 6;
    cap_resp->MediaType = a2dp_pcbs->local_sep[a2dp_pcbs->select_seid-1].Media_Type;
    cap_resp->MediaCodecType = AVDTP_AUDIO_CODEC_SBC;
    cap_resp->SF_CM = ((a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Sampling_Frequency) | (a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Channel_Mode));

    cap_resp->BL_BS_AM = ((a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Block_Length) |
                          (a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Subbands) |
                          (a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].Allocation_Method));

    cap_resp->MinimumBitpoolValue = a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].MinimumBitpoolValue;
    cap_resp->MaximumBitpoolValue = a2dp_pcbs->Localcodec[a2dp_pcbs->select_seid-1].MaximumBitpoolValue;

#ifdef A2DP_ACC
    cap_resp->AAC_ServiceCategory1 = AVDTP_MEDIA_CODEC;
    cap_resp->AAC_LOSC1 = 8;
    cap_resp->AAC_MediaType = a2dp_pcbs->local_sep.Media_Type;
    cap_resp->AAC_MediaCodecType = AVDTP_AUDIO_CODEC_AAC;
    cap_resp->AAC_SF_CM = 0x80;

    cap_resp->AAC_BL_BS_AM = 0xff;

    cap_resp->AAC_MinimumBitpoolValue = 0xfc;
    cap_resp->AAC_MaximumBitpoolValue = 0x80;
    cap_resp->AAC_MaximumBitpoolValue1 = 0x00;
    cap_resp->AAC_MaximumBitpoolValue2 = 0x00;
#endif

#else
    cap_resp->ServiceCategory1 = AVDTP_MEDIA_CODEC;
    cap_resp->LOSC1 = 6;
    cap_resp->MediaType = AVDTP_MEDIA_TYPE_AUDIO;
    cap_resp->MediaCodecType = AVDTP_AUDIO_CODEC_MP3;
    cap_resp->SF_CM = 0xff;
    cap_resp->BL_BS_AM = 0x7f;
    cap_resp->MinimumBitpoolValue = 0xff;
    cap_resp->MaximumBitpoolValue = 0xfe;
#endif

#if 0
    cap_resp->ServiceCategory2 = AVDTP_CONTENT_PROTECTION;
    cap_resp->LOSC2 = 2;
    cap_resp->CP_TYPE_LSB = 2;
    cap_resp->CP_TYPE_MSB = 0;

    get_cap_resp_pbuf->tot_len = get_cap_resp_pbuf->len = 12;
#endif

    ret = l2ca_datawrite(pcb, get_cap_resp_pbuf);
    pbuf_free(get_cap_resp_pbuf);
    return TRUE;
}
_ATTR_LWBT_CODE_
static bool avdtp_get_capabilities_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    uint8 seid_id;
    uint8 *payload;
    int ret = 0;
    struct avdtp_Command_Data *a2dpData;
    struct pbuf *get_cap_resp_pbuf = NULL;
    struct get_cap_resp *cap_resp;

    if(p->len != 1)
    {
        uint8 error_code;
        error_code = AVDTP_BAD_LENGTH;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;

    }

    a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    //if(a2dpData->Acp_SEID != AVDTP_SEP_ID)
    //    return FALSE;
    if(avdtp_check_seid(a2dpData->Acp_SEID,CHECK_SEID_FLAG_EXIST))
    {
        uint8 error_code;
        error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }

    seid_id = a2dpData->Acp_SEID-1;
#ifdef A2DP_ACC
    get_cap_resp_pbuf = pbuf_alloc(PBUF_RAW, 26, PBUF_RAM);
#else
    get_cap_resp_pbuf = pbuf_alloc(PBUF_RAW, 12, PBUF_RAM);

#endif
    if (get_cap_resp_pbuf == NULL)
        return FALSE;
    /* Check for minimum required packet size includes:
     *   1. getcap resp header
     *   2. media transport capability (2 bytes)
     *   3. media codec capability type + length (2 bytes)
     *   4. the actual media codec elements
     * */
    cap_resp = get_cap_resp_pbuf->payload;
    cap_resp->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    cap_resp->SIRFA = AVDTP_GET_CAPABILITIES;
    cap_resp->ServiceCategory = AVDTP_MEDIA_TRANSPORT;
    cap_resp->LOSC = 0;

    cap_resp->ServiceCategory1 = AVDTP_MEDIA_CODEC;
    cap_resp->LOSC1 = 6;
    cap_resp->MediaType = a2dp_pcbs->local_sep[seid_id].Media_Type;
    cap_resp->MediaCodecType = AVDTP_AUDIO_CODEC_SBC;
    cap_resp->SF_CM = ((a2dp_pcbs->Localcodec[seid_id].Sampling_Frequency) |
                        (a2dp_pcbs->Localcodec[seid_id].Channel_Mode));

    cap_resp->BL_BS_AM = ((a2dp_pcbs->Localcodec[seid_id].Block_Length) |
                          (a2dp_pcbs->Localcodec[seid_id].Subbands) |
                          (a2dp_pcbs->Localcodec[seid_id].Allocation_Method));

    cap_resp->MinimumBitpoolValue = a2dp_pcbs->Localcodec[seid_id].MinimumBitpoolValue;
    cap_resp->MaximumBitpoolValue = a2dp_pcbs->Localcodec[seid_id].MaximumBitpoolValue;

#ifdef A2DP_ACC
    cap_resp->AAC_ServiceCategory1 = AVDTP_MEDIA_CODEC;
    cap_resp->AAC_LOSC1 = 8;
    cap_resp->AAC_MediaType = a2dp_pcbs->local_sep[seid_id].Media_Type;

    cap_resp->AAC_MediaCodecType = AVDTP_AUDIO_CODEC_AAC;
    cap_resp->AAC_SF_CM = 0x80;

    cap_resp->AAC_BL_BS_AM = 0xff;

    cap_resp->AAC_MinimumBitpoolValue = 0xfc;
    cap_resp->AAC_MaximumBitpoolValue = 0x80;
    cap_resp->AAC_MaximumBitpoolValue1 = 0x00;
    cap_resp->AAC_MaximumBitpoolValue2 = 0x00;

#endif

    ret = l2ca_datawrite(pcb, get_cap_resp_pbuf);
    pbuf_free(get_cap_resp_pbuf);
    return TRUE;
}
_ATTR_LWBT_INIT_CODE_
int avdtp_register_sep(struct a2dp_pcb *pcbs,
                       uint8 seid_id,
                       uint8 SEID,
                       uint8 TSEP,
                       uint8 InUse,
                       uint8 media_type,
                       uint8 Sampling_Frequency,
                       uint8 Channel_Mode,
                       uint8 Block_Length,
                       uint8 Subbands,
                       uint8 Allocation_Method,
                       uint8 MinimumBitpoolValue,
                       uint8 MaximumBitpoolValue,
                       bool delay_reporting)
{
    pcbs->local_sep[seid_id].SEID = SEID;
    pcbs->local_sep[seid_id].Media_Type = media_type;
    pcbs->local_sep[seid_id].InUse = InUse;
    pcbs->local_sep[seid_id].TSEP = TSEP;
    pcbs->Localcodec[seid_id].Sampling_Frequency = Sampling_Frequency;
    pcbs->Localcodec[seid_id].Channel_Mode = Channel_Mode;
    pcbs->Localcodec[seid_id].Block_Length = Block_Length;
    pcbs->Localcodec[seid_id].Subbands = Subbands;
    pcbs->Localcodec[seid_id].Allocation_Method = Allocation_Method;
    pcbs->Localcodec[seid_id].MinimumBitpoolValue = MinimumBitpoolValue;
    pcbs->Localcodec[seid_id].MaximumBitpoolValue = MaximumBitpoolValue;
    pcbs->Localcodec[seid_id].delay_reporting = delay_reporting;

}
_ATTR_LWBT_INIT_CODE_
void avdtp_register(struct a2dp_pcb *pcbs)
{
    uint8 Sampling_Frequency;
    uint8 Channel_Mode;
    uint8 Block_Length;
    uint8 Subbands;
    uint8 Allocation_Method;
    //Sampling_Frequency = (SBC_SAMPLING_FREQ_16000 | SBC_SAMPLING_FREQ_32000 | SBC_SAMPLING_FREQ_44100 | SBC_SAMPLING_FREQ_48000);
    //Channel_Mode = (SBC_CHANNEL_MODE_MONO | SBC_CHANNEL_MODE_DUAL_CHANNEL | SBC_CHANNEL_MODE_STEREO | SBC_CHANNEL_MODE_JOINT_STEREO);
    Sampling_Frequency = SBC_SAMPLING_FREQ_44100;
    if(AudioGetSbcChannelMode() == 3)//0 MONO ; 1 DUAL CHANNEL; 2 STEREO; 3 JOINT STEREO;
    {
        Channel_Mode = (SBC_CHANNEL_MODE_MONO | SBC_CHANNEL_MODE_STEREO|SBC_CHANNEL_MODE_JOINT_STEREO);
    }
    else
    {
        Channel_Mode = (SBC_CHANNEL_MODE_MONO | SBC_CHANNEL_MODE_STEREO);
    }
    Block_Length = (SBC_BLOCK_LENGTH_4 | SBC_BLOCK_LENGTH_8 | SBC_BLOCK_LENGTH_12 | SBC_BLOCK_LENGTH_16);
    Subbands = (SBC_SUBBANDS_4 | SBC_SUBBANDS_8);
    Allocation_Method = (SBC_ALLOCATION_SNR | SBC_ALLOCATION_LOUDNESS);

    avdtp_register_sep(pcbs,0,
                       AVDTP_SEP_ID_SOUCRE,
                       AVDTP_SEP_TYPE_SOURCE,
                       AVDTP_SEPID_NOT_INUSE,
                       AVDTP_MEDIA_TYPE_AUDIO,
                       Sampling_Frequency,
                       Channel_Mode,
                       Block_Length,
                       Subbands,
                       Allocation_Method,
                       MIN_BITPOOL,
                       MAX_BITPOOL,
                       FALSE);

    Sampling_Frequency = SBC_SAMPLING_FREQ_44100;
    Channel_Mode = (SBC_CHANNEL_MODE_MONO | SBC_CHANNEL_MODE_DUAL_CHANNEL | SBC_CHANNEL_MODE_STEREO | SBC_CHANNEL_MODE_JOINT_STEREO);
    Block_Length = (SBC_BLOCK_LENGTH_4 | SBC_BLOCK_LENGTH_8 | SBC_BLOCK_LENGTH_12 | SBC_BLOCK_LENGTH_16);
    Subbands = (SBC_SUBBANDS_4 | SBC_SUBBANDS_8);
    Allocation_Method = (SBC_ALLOCATION_SNR | SBC_ALLOCATION_LOUDNESS);
    avdtp_register_sep(pcbs,1,
                   AVDTP_SEP_ID_SINK,
                   AVDTP_SEP_TYPE_SINK,
                   AVDTP_SEPID_NOT_INUSE,
                   AVDTP_MEDIA_TYPE_AUDIO,
                   Sampling_Frequency,
                   Channel_Mode,
                   Block_Length,
                   Subbands,
                   Allocation_Method,
                   MIN_BITPOOL,
                   MAX_BITPOOL,
                   FALSE);
}
_ATTR_LWBT_CODE_
static bool avdtp_Abort_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *Abort_resp_pbuf = NULL;
    struct Abort_resp *Abort_resp_s;


    Abort_resp_pbuf = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);
    if (Abort_resp_pbuf == NULL)
        return FALSE;

    Abort_resp_s = Abort_resp_pbuf->payload;
    Abort_resp_s->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    Abort_resp_s->SIRFA = AVDTP_ABORT;

    l2ca_datawrite(pcb, Abort_resp_pbuf);
    pbuf_free(Abort_resp_pbuf);
    return TRUE;
}
_ATTR_LWBT_CODE_
static bool avdtp_Suspend_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *Suspend_resp_pbuf = NULL;
    struct Suspend_resp *Suspend_resp_s;

    struct avdtp_Command_Data *Suspend_a2dpData;

    Suspend_a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    //if(Suspend_a2dpData->Acp_SEID != AVDTP_SEP_ID)
    //    return FALSE;
    if(avdtp_check_seid(Suspend_a2dpData->Acp_SEID,CHECK_SEID_FLAG_USED))
    {
        uint8 error_code;
        error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }

    Suspend_resp_pbuf = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);
    if (Suspend_resp_pbuf == NULL)
        return FALSE;

    Suspend_resp_s = Suspend_resp_pbuf->payload;
    Suspend_resp_s->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    Suspend_resp_s->SIRFA = AVDTP_SUSPEND;

    l2ca_datawrite(pcb, Suspend_resp_pbuf);
    pbuf_free(Suspend_resp_pbuf);
    return TRUE;
}
_ATTR_LWBT_CODE_
static bool avdtp_close_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *close_resp_pbuf = NULL;
    struct close_resp *close_resp_s;
    struct avdtp_Command_Data *close_a2dpData;


    if(p->len != 1)
    {
        uint8 error_code;
        error_code = AVDTP_BAD_LENGTH;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;

    }

    close_a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    //if(close_a2dpData->Acp_SEID != AVDTP_SEP_ID)
    //    return FALSE;
    if(avdtp_check_seid(close_a2dpData->Acp_SEID,CHECK_SEID_FLAG_USED))
    {
        uint8 error_code;
        error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }

    close_resp_pbuf = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);
    if (close_resp_pbuf == NULL)
        return FALSE;

    close_resp_s = close_resp_pbuf->payload;
    close_resp_s->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    close_resp_s->SIRFA = AVDTP_CLOSE;
//    l2cap_close(pcb);
    l2ca_datawrite(pcb, close_resp_pbuf);
    pbuf_free(close_resp_pbuf);
    return TRUE;
}
_ATTR_LWBT_CODE_
static bool avdtp_start_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *start_resp_pbuf = NULL;
    struct start_resp *start_resp_s;
    struct avdtp_Command_Data *start_a2dpData;

    start_a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    //if(start_a2dpData->Acp_SEID != AVDTP_SEP_ID)
    //    return FALSE;
    if(avdtp_check_seid(start_a2dpData->Acp_SEID,CHECK_SEID_FLAG_USED))
    {
        uint8 error_code;
        error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }

    start_resp_pbuf = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);
    if (start_resp_pbuf == NULL)
        return FALSE;

    start_resp_s = start_resp_pbuf->payload;
    start_resp_s->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    start_resp_s->SIRFA = AVDTP_START;

    l2ca_datawrite(pcb, start_resp_pbuf);
    pbuf_free(start_resp_pbuf);
    return TRUE;
}
_ATTR_LWBT_CODE_
static bool avdtp_open_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *open_resp_pbuf = NULL;
    struct open_resp *open_resp_s;
    struct avdtp_Command_Data *open_a2dpData;

    open_a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    //if(open_a2dpData->Acp_SEID != AVDTP_SEP_ID)
    //    return FALSE;
    if(avdtp_check_seid(open_a2dpData->Acp_SEID,CHECK_SEID_FLAG_USED))
    {
        uint8 error_code;
        error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }

    open_resp_pbuf = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);
    if (open_resp_pbuf == NULL)
        return FALSE;

    open_resp_s = open_resp_pbuf->payload;
    open_resp_s->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    open_resp_s->SIRFA = AVDTP_OPEN;

    l2ca_datawrite(pcb, open_resp_pbuf);
    pbuf_free(open_resp_pbuf);
    return TRUE;
}
_ATTR_LWBT_CODE_
static bool avdtp_set_configuration_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *set_configuration_resp_pbuf = NULL;
    struct set_configuration_resp *configuration_resp;
    struct avdtp_Command_CapMidea_Data *config_Data;
    struct avdtp_Command_Data *config_a2dpData;
    int i=0;
    int8 error;

    config_a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    if(avdtp_check_seid(config_a2dpData->Acp_SEID,CHECK_SEID_FLAG_EXIST))
    {
        uint8 error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }
#if 1
    if(avdtp_check_seid(config_a2dpData->Acp_SEID, CHECK_SEID_FLAG_ROLE))
    {
        uint8 set_config_servercapabilities[5] = {0};

       set_config_servercapabilities[0] = AVDTP_MEDIA_TRANSPORT;
       set_config_servercapabilities[1] = AVDTP_SEP_IN_USE;
       avdtp_reject_rsp(pcb,header,set_config_servercapabilities, 2);
       return FALSE;
    }
#endif
    error = (uint8)avdtp_check_config_info(pcb, a2dp_pcbs, p,header);
    if(error)
    {
        return error;
    }
    a2dp_pcbs->Remotecodec.Int_SEID = (((uint8*)p->payload)[0] >> 2);

    pbuf_header(p, -1);
    while(p->len)
    {
        if(((uint8 *)p->payload)[0] != AVDTP_MEDIA_CODEC)
        {

pbuf_header(p, -(((int8 *)p->payload)[1]+2));
        }
        else
        {
             break;
        }

    }
    config_Data = p->payload;

    a2dp_pcbs->Remotecodec.MediaType = (config_Data->MediaType >> 4);

    avdtp_Save_Remote_Codec_info(config_Data);

    set_configuration_resp_pbuf = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);
    if (set_configuration_resp_pbuf == NULL)
        return FALSE;

    configuration_resp = set_configuration_resp_pbuf->payload;
    configuration_resp->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    configuration_resp->SIRFA = AVDTP_SET_CONFIGURATION;

    l2ca_datawrite(pcb, set_configuration_resp_pbuf);
    pbuf_free(set_configuration_resp_pbuf);
    return TRUE;
}

_ATTR_LWBT_CODE_
static bool avdtp_get_configuration_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    struct pbuf *get_configuration_resp_pbuf = NULL;
    struct get_cap_resp *configuration_resp;
    struct avdtp_Command_Cap_Data *config_Data;
    struct avdtp_Command_Data *config_a2dpData;
    struct a2dp_pcb *a2dp_pcbs_get = NULL;
    int8 error;
    uint8 i=0;

    config_a2dpData = p->payload;
    pbuf_header(p, -A2DP_DATA_LEN);

    if(avdtp_check_seid(config_a2dpData->Acp_SEID, CHECK_SEID_FLAG_EXIST))
    {
        uint8 error_code = AVDTP_BAD_ACP_SEID;
        avdtp_reject_rsp(pcb,header,&error_code,1);
        return FALSE;
    }

#if 1


#ifdef A2DP_ACC
    get_configuration_resp_pbuf = pbuf_alloc(PBUF_RAW, 26, PBUF_RAM);
#else
    get_configuration_resp_pbuf = pbuf_alloc(PBUF_RAW, 12, PBUF_RAM);
#endif
    if (get_configuration_resp_pbuf == NULL)
        return FALSE;

    configuration_resp = get_configuration_resp_pbuf->payload;
    configuration_resp->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    configuration_resp->SIRFA = AVDTP_GET_CONFIGURATION;

    configuration_resp->ServiceCategory = AVDTP_MEDIA_TRANSPORT;
    configuration_resp->LOSC = 0;

    configuration_resp->ServiceCategory1 = AVDTP_MEDIA_CODEC;
    configuration_resp->LOSC1 = 6;
    configuration_resp->MediaType = a2dp_pcbs->Remotecodec.MediaType;
    configuration_resp->MediaCodecType = AVDTP_AUDIO_CODEC_SBC;
    configuration_resp->SF_CM = ((a2dp_pcbs->Remotecodec.Sampling_Frequency) |
                        (a2dp_pcbs->Remotecodec.Channel_Mode));

    configuration_resp->BL_BS_AM = ((a2dp_pcbs->Remotecodec.Block_Length) |
                          (a2dp_pcbs->Remotecodec.Subbands) |
                          (a2dp_pcbs->Remotecodec.Allocation_Method));

    configuration_resp->MinimumBitpoolValue = a2dp_pcbs->Remotecodec.MinimumBitpoolValue;
    configuration_resp->MaximumBitpoolValue = a2dp_pcbs->Remotecodec.MaximumBitpoolValue;

#ifdef A2DP_ACC
    configuration_resp->AAC_ServiceCategory1 = AVDTP_MEDIA_CODEC;
    configuration_resp->AAC_LOSC1 = 8;
    configuration_resp->AAC_MediaType = a2dp_pcbs->Remotecodec.Media_Type;

    configuration_resp->AAC_MediaCodecType = AVDTP_AUDIO_CODEC_AAC;
    configuration_resp->AAC_SF_CM = 0x80;

    configuration_resp->AAC_BL_BS_AM = 0xff;

    configuration_resp->AAC_MinimumBitpoolValue = 0xfc;
    configuration_resp->AAC_MaximumBitpoolValue = 0x80;
    configuration_resp->AAC_MaximumBitpoolValue1 = 0x00;
    configuration_resp->AAC_MaximumBitpoolValue2 = 0x00;
#endif

    l2ca_datawrite(pcb, get_configuration_resp_pbuf);
    pbuf_free(get_configuration_resp_pbuf);
#endif
    return TRUE;
}


_ATTR_LWBT_CODE_
static bool avdtp_discover_resp(struct l2cap_pcb *pcb, struct pbuf *p,struct avdtp_single_header *header)
{
    uint8 *payload;
    int ret = 0;
    int i;
    bool getcap_pending = FALSE;
    struct pbuf *discover_resp_pbuf = NULL;
    struct discover_resp *resp;

    discover_resp_pbuf  = pbuf_alloc(PBUF_RAW, 2+2*AVDTP_SEP_MAX, PBUF_RAM);

    if (discover_resp_pbuf == NULL)
        return FALSE;

    /* Add PDU header to packet */
    resp = discover_resp_pbuf->payload;
    resp->MPT = ((header->transaction << 4) | (header->packet_type<< 2) | AVDTP_MSG_TYPE_ACCEPT);
    resp->SIRFA = AVDTP_DISCOVER;
    payload = ((uint8*)discover_resp_pbuf->payload)+2;
    for(i=0;i<AVDTP_SEP_MAX;i++)
    {
        payload[i*2] = ((a2dp_pcbs->local_sep[i].SEID << 2) | (AVDTP_SEPID_NOT_INUSE << 1));
        payload[i*2+1] = (AVDTP_MEDIA_TYPE_AUDIO << 4) | (a2dp_pcbs->local_sep[i].TSEP << 3);
    }
    //resp->RFAINUSESEID = ((AVDTP_SEP_ID << 2) | (AVDTP_SEPID_NOT_INUSE << 1));
    //resp->RFATSEPMEDAI = (AVDTP_MEDIA_TYPE_AUDIO << 4) | (AVDTP_SEP_TYPE_SOURCE << 3);

    ret = l2ca_datawrite(pcb, discover_resp_pbuf);
    pbuf_free(discover_resp_pbuf);

    return ret;
}


_ATTR_LWBT_INIT_CODE_
int avdtp_unregister_sep(struct avdtp_local_sep *sep)
{
    if (!sep)
        return FALSE;

    //FREE(sep);

    return 0;
}


_ATTR_LWBT_CODE_
void avdtp_lp_disconnected(void *arg, struct l2cap_pcb *l2cappcb, err_t err)
{
    struct a2dp_pcb *pcb;
    if(a2dp_meida_pcbs == l2cappcb)
    {
        avdtp_State_s = AVDTP_STATE_IDLE;
        avdtp_media_ch_state = MEDIA_CH_NO_SETUP;
        a2dp_meida_pcbs = NULL;
        avdtp_sequenceNumber_nxt = 0;
    }

    if(a2dp_pcbs->l2cappcb == l2cappcb)
    {
        pcb = a2dp_pcbs;
        avdtp_recv_cmd = 0;
        avdtp_rsp_prev = 0;
        avdtp_recv_rsp = 0;
        avdtp_State_s = AVDTP_STATE_IDLE;
        avdtp_media_ch_state = MEDIA_CH_NO_SETUP;
        if(avdtp_notify_pcb.disconnected_result)
        {
            avdtp_notify_pcb.disconnected_result(RETURN_OK);
        }
        if(pcb != NULL)
        {
            pcb->l2cappcb = NULL;
            pcb->local_sep[a2dp_pcbs->select_seid-1].InUse = AVDTP_SEPID_NOT_INUSE;
        }
        avctp_StopTimer();
        SystickTimerStop(&AvdtpCheckStateTimer);
        SystickTimerStop(&AvdtpDiscoverTimer);
        SystickTimerStop(&AvdtpMediaSetUpTimer);
        SystickTimerStop(&AvdtpGetGapabilitiesTimer);
    }
    l2cap_close(l2cappcb);
}

/*-----------------------------------------------------------------------------------*/
/*
* sdp_l2cap_connected_ind():
*
*
*
*
*
*/
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t avdtp_l2cap_connected_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    err_t ret = ERR_OK;

    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("l2cap_disconnected_ind: L2CAP disconnected\n"));

    if(pcb->psm == AVDTP_PSM)
    {

        if(a2dp_pcbs->l2cappcb == NULL)
        {
            //l2cap_recv(pcb, avdtp_recv);
            l2cap_recv(pcb, avdtp_Cmd_recv);
            a2dp_pcbs->l2cappcb = pcb;
            l2cap_disconnect_ind(pcb, avdtp_lp_disconnected);
        }
        else
        {
            avdtp_media_ch_state = MEDIA_CH_SETUP;
            //l2cap_recv(pcb, avdtp_data_recv);

            a2dp_meida_pcbs = pcb;
            l2cap_disconnect_ind(pcb, avdtp_lp_disconnected);

            if(avdtp_notify_pcb.connected_notify != NULL)
            {
                avdtp_notify_pcb.connected_notify();
                if(avdtp_notify_pcb.connected_result != NULL)
                {
                    avdtp_notify_pcb.connected_result(RETURN_OK);
                }
            }
        }
    }
    return ret;
}


/*-----------------------------------------------------------------------------------*/
/*
 * sdp_init():
 *
 * Initializes the SDP layer.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void avdtp_init(void)
{
    struct sdp_record *record;
    a2dp_pcbs = NULL;
//    media_recv = NULL;
    avdtp_State_s = AVDTP_STATE_IDLE;
    avdtp_media_ch_state = MEDIA_CH_NO_SETUP;
    avdtp_recv_cmd = 0;
    avdtp_recv_rsp= 0;
    avdtp_rsp_prev= 0;

    avdtp_sequenceNumber_nxt = 0;


#if LWBT_LAP

    a2dp_pcbs = avdtp_new();

    avdtp_register(a2dp_pcbs);
    l2cap_connect_ind(NULL, AVDTP_PSM,avdtp_l2cap_connected_ind);
    a2dp_pcbs->select_seid = AVDTP_SEP_TYPE_SOURCE;

#endif
#if 1
    if((record = sdp_record_new((uint8 *)a2dp_service_record, sizeof(a2dp_service_record))) == NULL)
    {
        LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("avdtp_init: Could not alloc SDP record\n"));
        //return ERR_MEM;
    }
    else
    {
        sdp_register_service(record);
    }

#endif

}



void avdtp_checkState(void);
_ATTR_LWBT_DATA_
SYSTICK_LIST AvdtpCheckStateTimer =
{
    NULL,
    0,
    300,
    0,
    avdtp_checkState,
};


_ATTR_LWBT_CODE_
void avdtp_checkState(void)
{
    SystickTimerStop(&AvdtpCheckStateTimer);

    if(avdtp_State_s == AVDTP_STATE_IDLE && avdtp_recv_rsp == 0)
    {
        l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
        a2dp_pcbs->l2cappcb = NULL;
    }
    else if(avdtp_State_s == AVDTP_STATE_IDLE && avdtp_recv_rsp != 0)
    {
        if(avdtp_rsp_prev == avdtp_recv_rsp)
        {
            l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
            a2dp_pcbs->l2cappcb = NULL;
        }
        else
        {
            avdtp_rsp_prev = avdtp_recv_rsp;
            SystickTimerStart(&AvdtpCheckStateTimer);
        }
    }


}




void avdtp_discover(void);
_ATTR_LWBT_DATA_
SYSTICK_LIST AvdtpDiscoverTimer =
{
    NULL,
    0,
    //200,
    50,
    0,
    avdtp_discover,
};


_ATTR_LWBT_CODE_
void avdtp_discover(void)
{
    if(avdtp_State_s == AVDTP_STATE_IDLE && avdtp_recv_cmd == 0)
    {
        avdtp_discover_cmd(a2dp_pcbs->l2cappcb);
        SystickTimerStart(&AvdtpCheckStateTimer);
    }


    SystickTimerStop(&AvdtpDiscoverTimer);
}





_ATTR_LWBT_CODE_
err_t avdtp_l2cap_connected_cfm(void *arg, struct l2cap_pcb *pcb, uint16 result, uint16 status)
{
    err_t ret = ERR_OK;


    if(result == L2CAP_CONN_SUCCESS)
    {

        if(pcb->psm == AVDTP_PSM)
        {

            if(a2dp_pcbs->l2cappcb == NULL)
            {
                l2cap_recv(pcb, avdtp_Cmd_recv);
                a2dp_pcbs->l2cappcb = pcb;
                l2cap_disconnect_ind(pcb, avdtp_lp_disconnected);
                LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("avdtp control channle conneced"));
                SystickTimerStart(&AvdtpDiscoverTimer);

            }
            else
            {
                avdtp_media_ch_state = MEDIA_CH_SETUP;
                //l2cap_recv(pcb, avdtp_data_recv);
                l2cap_disconnect_ind(pcb, avdtp_lp_disconnected);

                a2dp_meida_pcbs = pcb;
                LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("avdtp media channle conneced"));
                if(avdtp_notify_pcb.connected_notify != NULL)
                {
                    avdtp_notify_pcb.connected_notify();
                    if(avdtp_notify_pcb.connected_result != NULL)
                    {
                        avdtp_notify_pcb.connected_result(RETURN_OK);
                    }
                    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("avdtp conneced OK"));
                }

                //avdtp_start();
            }
        }
    }
    else
    {
        if(avdtp_notify_pcb.connected_result != NULL)
        {
            avdtp_notify_pcb.connected_result(RETURN_FAIL);
        }
        l2cap_close(pcb);
        LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("avdtp conneced fail"));
    }
    return ret;
}


_ATTR_LWBT_CODE_
int avdtp_connect(struct bd_addr *bdaddr, void(*connect_result)(int result))
{
    struct l2cap_pcb *l2cappcb;
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("avdtp connecting"));
    if(avdtp_recv_cmd == 0 && avdtp_recv_rsp == 0 &&  a2dp_pcbs->l2cappcb == NULL)
    {
        if((l2cappcb = l2cap_new()) == NULL)
        {
            LWBT_DEBUGF(AVDVT_DEBUG, _DBG_SERIOUS_, ("Could not alloc L2CAP pcb\n"));
            return ERR_MEM;
        }

        avdtp_notify_pcb.connected_result = connect_result;
        return l2ca_connect_req(l2cappcb, bdaddr, AVDTP_PSM, HCI_ALLOW_ROLE_SWITCH, avdtp_l2cap_connected_cfm);
    }
    else if(a2dp_pcbs->l2cappcb)
    {
        avdtp_notify_pcb.connected_result = connect_result;
        SystickTimerStart(&AvdtpDiscoverTimer);
    }

    return 0;
}


_ATTR_LWBT_CODE_
err_t avdtp_disconnected_cfm(void *arg, struct l2cap_pcb *pcb)
{
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_,("avdtp_disconnected_cfm"));
    if(pcb->psm == AVCTP_PSM)
    {
        avctp_lp_disconnected(NULL, pcb, 0);
        if(a2dp_pcbs->l2cappcb)
        {
            l2ca_disconnect_req(a2dp_pcbs->l2cappcb, avdtp_disconnected_cfm);
        }
        else
        {
            hci_linkdown(&pcb->remote_bdaddr, HCI_OTHER_END_TERMINATED_CONN_USER_ENDED, HCI_CONNECT_TYPE_ACL);
        }
        LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_,("avrcp disconneced"));

        //l2ca_disconnect_req(a2dp_meida_pcbs, avdtp_disconnected_cfm);
    }
    else if(pcb->psm == AVDTP_PSM)
    {

        if(a2dp_meida_pcbs == pcb)
        {
            //l2ca_disconnect_req(a2dp_pcbs->l2cappcb, avdtp_disconnected_cfm);
            LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_,("meida ch disconneced"));
            if(avctp_disconnected_request(avdtp_disconnected_cfm)== RETURN_FAIL)
            {
                if(a2dp_pcbs->l2cappcb)
                {
                    l2ca_disconnect_req(a2dp_pcbs->l2cappcb, avdtp_disconnected_cfm);
                }
                else
                {
                    hci_linkdown(&pcb->remote_bdaddr, HCI_OTHER_END_TERMINATED_CONN_USER_ENDED, HCI_CONNECT_TYPE_ACL);
                }
            }
        }

        if(a2dp_pcbs->l2cappcb == pcb)
        {
            hci_linkdown(&pcb->remote_bdaddr, HCI_OTHER_END_TERMINATED_CONN_USER_ENDED, HCI_CONNECT_TYPE_ACL);

//            if(avdtp_notify_pcb.disconnected_result)
//            {
//                avdtp_notify_pcb.disconnected_result(RETURN_OK);
//            }
            LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_,("control ch disconneced"));
        }


        avdtp_lp_disconnected(NULL, pcb, 0);
    }
}


_ATTR_LWBT_CODE_
int avdtp_disconnect_request(struct bd_addr *bdaddr, void(*disconnect_result)(int result))
{

    int ret;
    avdtp_notify_pcb.disconnected_result = disconnect_result;
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_,("avdtp_disconnect_request"));
    if(avdtp_State_s == AVDTP_STATE_CLOSING || avdtp_State_s == AVDTP_STATE_IDLE)
    {
        if(a2dp_meida_pcbs)
        {
            l2ca_disconnect_req(a2dp_meida_pcbs, avdtp_disconnected_cfm);
        }
        else
        {
            if(a2dp_pcbs && a2dp_pcbs->l2cappcb);
            l2ca_disconnect_req(a2dp_pcbs->l2cappcb, avdtp_disconnected_cfm);
        }


    }
    else
    {
        avdtp_State_s = AVDTP_STATE_WAIT_CLOSE;

        //avdtp_abort_cmd(a2dp_pcbs->l2cappcb, NULL, NULL);
        //avdtp_close_cmd(a2dp_pcbs->l2cappcb, NULL, NULL);
        avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL, AVDTP_CLOSE);
    }
}


void avdtp_setup_meida_channel(void);

_ATTR_LWBT_DATA_
SYSTICK_LIST AvdtpMediaSetUpTimer =
{
    NULL,
    0,
    //200,
    50,
    0,
    avdtp_setup_meida_channel,
};

_ATTR_LWBT_CODE_
void avdtp_setup_meida_channel(void)
{
    struct l2cap_pcb *l2cappcb_media;
    SystickTimerStop(&AvdtpMediaSetUpTimer);
    if( avdtp_media_ch_state == MEDIA_CH_NO_SETUP)
    {
        if((l2cappcb_media = l2cap_new()) == NULL)
        {
            LWIP_DEBUGF(BT_IP_DEBUG, ("Could not alloc L2CAP pcb\n"));
            return ;
        }
        l2ca_connect_req(l2cappcb_media, &a2dp_pcbs->l2cappcb->remote_bdaddr, AVDTP_PSM, HCI_ALLOW_ROLE_SWITCH, avdtp_l2cap_connected_cfm);
    }

}




/*-----------------------------------------------------------------------------------*/
/*
 * sdp_reset_all():
 *
 * Free all SDP protocol control blocks and registered records.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void avdtp_reset_all(void)
{
    struct a2dp_pcb *pcb, *tpcb;

    for(pcb = a2dp_pcbs; pcb != NULL;)
    {
        tpcb = pcb->next;
        avdtp_free(pcb);
        pcb = tpcb;
    }
    avdtp_init();
}

/*--------------------------------------------------------------*/
/*
 * sdp_recv():
 *
 * Called by the lower layer. Parses the header and handle the SDP message.
 */
/*-----------------------------------------------------------------------------------*/


_ATTR_LWBT_CODE_
void avdtp_sendMessage(struct l2cap_pcb *l2_pcb,uint16 state,void (* avctp_startTimer)(struct l2cap_pcb *pcb ))
{
    if( state == AVDTP_STATE_OPEN )
        (* avctp_startTimer)(l2_pcb);
    else
        return;
}


void avdtp_get_capabilities_timer(void);
_ATTR_LWBT_DATA_
SYSTICK_LIST AvdtpGetGapabilitiesTimer =
{
    NULL,
    0,
    200,
    0,
    avdtp_get_capabilities_timer,
};


_ATTR_LWBT_CODE_
void avdtp_get_capabilities_timer(void)
{
    if(avdtp_State_s == AVDTP_STATE_IDLE && avdtp_recv_cmd == 0)
    {
        //avdtp_get_capabilities_cmd(a2dp_pcbs->l2cappcb, NULL,NULL);
        avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_GET_CAPABILITIES);
    }

    SystickTimerStop(&AvdtpGetGapabilitiesTimer);
}


/*--------------------------------------------------------------*/
/*
 * avdtp_Cmd_recv():
 *
 * Called by the lower layer. Parses the header and handle the SDP message.
 */
/*-----------------------------------------------------------------------------------*/

_ATTR_LWBT_CODE_
err_t avdtp_Cmd_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *s, err_t err)
{
    struct avdtp_single_header *a2dphdr;

    err_t ret = ERR_OK;
    uint16 i;
    struct pbuf *p, *q, *r;
    uint8 message_tpye;

    p = s;

    a2dphdr = p->payload;
    pbuf_header(p, -A2DP_HEADER_LEN);

    message_tpye = a2dphdr->message_type;
    LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_,("avdtp cmd=0x%02x, mssage type=0x%02x\n", a2dphdr->signal_id, message_tpye));
    switch(a2dphdr->signal_id)
    {
        case AVDTP_DISCOVER:
            switch(message_tpye)
            {
                case AVDTP_MSG_TYPE_COMMAND:
                    avdtp_discover_resp(pcb, p,a2dphdr);
                    avdtp_recv_cmd = 1;
                    break;

                case AVDTP_MSG_TYPE_ACCEPT:
                    avdtp_recv_rsp = 1;
                    {
                        uint8 seid;
                        uint8 len;
                        uint8 seidcnt;
                        uint8 TESP;
                        uint8 in_use;
                        uint8 MediaType;
                        struct discover_cmd_resp *a2dpData;

                        //a2dpData = p->payload;
                        len = p->tot_len;

                        memset(&a2dp_pcbs->RemoteSEID, 0 , 16);
                        if(len > 16)
                        {
                            len = 16;
                            LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_, ("SEID too long\n"));
                        }

                        seidcnt = 0;
                        while(len)
                        {
                            if(len % 2)
                            {
                                LWBT_DEBUGF(AVDVT_DEBUG, _DBG_INFO_,("SEID len is err,size =%d\n",len));
                                return RETURN_FAIL;
                            }
                            a2dpData = p->payload;
                            seid = (a2dpData->RFAINUSESEID >> 2);
                            in_use = (a2dpData->RFAINUSESEID >> 1) & 0x01;
                            TESP = (a2dpData->RFATSEPMEDAI >> 3) & 0x01;
                            MediaType = a2dpData->RFATSEPMEDAI >> 4;

                            if((MediaType == AVDTP_MEDIA_TYPE_AUDIO) &&
                                (TESP == (a2dp_pcbs->role^1)) &&
                                (in_use == AVDTP_SEPID_NOT_INUSE) )
                            {
                                a2dp_pcbs->RemoteSEID[seidcnt].RFAINUSESEID = a2dpData->RFAINUSESEID;
                                a2dp_pcbs->RemoteSEID[seidcnt].RFATSEPMEDAI = a2dpData->RFATSEPMEDAI;
                                seidcnt++;
                            }
                            pbuf_header(p, -2);
                            len = len - 2;
                        }

                        a2dp_pcbs->RemoteSEIDCnt = seidcnt;
                        if(seidcnt)
                            a2dp_pcbs->Remotecodec.Int_SEID = a2dp_pcbs->RemoteSEID[0].RFAINUSESEID >> 2;
                        else
                            a2dp_pcbs->Remotecodec.Int_SEID = seid;
                        a2dp_pcbs->Remotecodec.MediaType = AVDTP_MEDIA_TYPE_AUDIO;
                    }

                    //SystickTimerStart(&AvdtpGetGapabilitiesTimer);
                    avdtp_get_capabilities_retry = 0;
                    if(avdtp_recv_cmd == 0)
                        //avdtp_get_capabilities_cmd(a2dp_pcbs->l2cappcb, NULL,NULL);
                        avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_GET_CAPABILITIES);

                    break;

                case AVDTP_MSG_TYPE_GEN_REJECT:

                     break;

                case AVDTP_MSG_TYPE_REJECT:

                    l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
                    a2dp_pcbs->l2cappcb = NULL;

                    break;
            }

            pbuf_free(p);
            break;


        case AVDTP_GET_ALL_CAPABILITIES:
            switch(message_tpye)
            {
                case AVDTP_MSG_TYPE_COMMAND:
                    avdtp_get_all_capabilities_resp(pcb, p,a2dphdr);
                    break;
            }
            pbuf_free(p);
            break;


        case AVDTP_GET_CAPABILITIES:
            switch(message_tpye)
            {
                case AVDTP_MSG_TYPE_COMMAND:
                    avdtp_recv_cmd = 1;
                    avdtp_get_capabilities_resp(pcb, p,a2dphdr);
                    break;

                case AVDTP_MSG_TYPE_ACCEPT:
                    avdtp_recv_rsp = 2;
                    if(avdtp_recv_cmd == 0)
                    {
                        while(p->len)
                        {
                            if(((uint8 *)p->payload)[0] != AVDTP_MEDIA_CODEC)
                            {

                                 pbuf_header(p, -(((int8 *)p->payload)[1]+2));
                            }
                            else
                            {
                                 break;
                            }

                        }

                        if(((uint8 *)p->payload)[3] == AVDTP_AUDIO_CODEC_SBC)
                        {
                            struct avdtp_Command_CapMidea_Data *config_Data;

                            config_Data = (struct avdtp_Command_CapMidea_Data *)((uint8 *)p->payload);

                            avdtp_Save_Remote_Codec_info(config_Data);

                            avdtp_set_configuration_cmd(pcb, p,NULL);
                        }
#ifdef A2DP_ACC
                        else if
                        if(((uint8 *)p->payload)[3] == AVDTP_AUDIO_CODEC_AAC)
                            {
                                avdtp_set_configuration_cmd(pcb, p,NULL);
                            }
#endif
                            else
                            {
                                if(avdtp_get_capabilities_retry+1 < a2dp_pcbs->RemoteSEIDCnt)
                                {
                                    a2dp_pcbs->Remotecodec.Int_SEID = a2dp_pcbs->RemoteSEID[avdtp_get_capabilities_retry+1].RFAINUSESEID >> 2;
                                    a2dp_pcbs->Remotecodec.MediaType = AVDTP_MEDIA_TYPE_AUDIO;
                                    avdtp_get_capabilities_retry++;

                                    //avdtp_get_capabilities_cmd(a2dp_pcbs->l2cappcb, NULL,NULL);
                                    avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_GET_CAPABILITIES);
                                }

                            }

                    }

                    break;

                case AVDTP_MSG_TYPE_GEN_REJECT:
                    break;
                case AVDTP_MSG_TYPE_REJECT:

                   // l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
                    //a2dp_pcbs->l2cappcb = NULL;
                   //
                    break;
            }

            pbuf_free(p);
            break;

        case AVDTP_SET_CONFIGURATION:
            switch(message_tpye)
            {
                case AVDTP_MSG_TYPE_COMMAND:
                    avdtp_recv_cmd = 1;
                    avdtp_set_configuration_resp(pcb, p,a2dphdr);
                    avdtp_State_s = AVDTP_STATE_CONFIGURED;
                    break;

                case AVDTP_MSG_TYPE_ACCEPT:

                    avdtp_recv_rsp = 3;
                    if(avdtp_recv_cmd == 0)
                        //avdtp_open_cmd(pcb, p,a2dphdr);
                        avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_OPEN);
                    avdtp_State_s = AVDTP_STATE_CONFIGURED;

                    break;


                case AVDTP_MSG_TYPE_GEN_REJECT:
                case AVDTP_MSG_TYPE_REJECT:

                    if(avdtp_set_conf_retry == 0)
                    {
                        avdtp_set_configuration_cmd(pcb, p,(struct avdtp_single_header *)1);
                        avdtp_set_conf_retry++ ;
                    }
                    else
                    {
                        avdtp_set_conf_retry = 0;

                        l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
                        a2dp_pcbs->l2cappcb = NULL;
                    }

                    break;


                    break;
            }
            pbuf_free(p);
            break;

        case AVDTP_GET_CONFIGURATION:
            switch(message_tpye)
            {
                case AVDTP_MSG_TYPE_COMMAND:
                    //avdtp_recv_cmd = 1;
                    avdtp_get_configuration_resp(pcb, p,a2dphdr);
                    break;
                case  AVDTP_MSG_TYPE_ACCEPT:

                    break;
                case AVDTP_MSG_TYPE_GEN_REJECT:

                    break;
                case AVDTP_MSG_TYPE_REJECT:

                    break;

            }
            break;
        case AVDTP_OPEN:


            switch(message_tpye)
            {
                case AVDTP_MSG_TYPE_COMMAND:
                    if((avdtp_State_s == AVDTP_STATE_CONFIGURED) || (avdtp_State_s == AVDTP_STATE_STREAMING))
                    {
                        avdtp_open_resp(pcb, p,a2dphdr);
                        avdtp_State_s = AVDTP_STATE_OPEN;
                        avdtp_sendMessage(pcb,avdtp_State_s,avctp_startTimer);

                        //if(avdtp_notify_pcb.connected_notify != NULL)
                        //{
                        //    avdtp_notify_pcb.connected_notify();
                        //}
                    }
                    else
                    {
                        uint8 error_code;
                        error_code = AVDTP_BAD_STATE;
                        avdtp_reject_rsp(pcb,a2dphdr,&error_code,1);
                    }
                    break;

                case AVDTP_MSG_TYPE_ACCEPT:

                    avdtp_recv_rsp = 4;
                    if((avdtp_State_s == AVDTP_STATE_CONFIGURED) || (avdtp_State_s == AVDTP_STATE_STREAMING))
                    {
                        avdtp_State_s = AVDTP_STATE_OPEN;

                       // printf("avdtp mediaset \r\n");
                        SystickTimerStart(&AvdtpMediaSetUpTimer);

                        avdtp_sendMessage(pcb,avdtp_State_s,avctp_startTimer);

                        
                    }

                    //if(avdtp_notify_pcb.connected_notify != NULL)
                    //{
                    //    avdtp_notify_pcb.connected_notify();
                    //}

                    break;

                case AVDTP_MSG_TYPE_GEN_REJECT:
                case AVDTP_MSG_TYPE_REJECT:
                    l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
                    a2dp_pcbs->l2cappcb = NULL;

                    break;
            }

            pbuf_free(p);
            break;

        case AVDTP_START:


            switch(message_tpye)
            {

                case AVDTP_MSG_TYPE_COMMAND:
                    if(avdtp_State_s == AVDTP_STATE_OPEN)
                    {
                        avdtp_start_resp(pcb, p,a2dphdr);
                        avdtp_State_s = AVDTP_STATE_STREAMING;

                        //avdtp_sendMessage(pcb,avdtp_State_s,avctp_startTimer);

                    }
                    else if(avdtp_State_s == AVDTP_STATE_SUSPEND)
                    {
                        avdtp_start_resp(pcb, p,a2dphdr);
                        avdtp_State_s = AVDTP_STATE_STREAMING;
                        //bt_buf_reset();
                    }
                    else if(avdtp_State_s == AVDTP_STATE_WAIT_START_COMPLETE)
                    {
                        avdtp_start_resp(pcb, p,a2dphdr);
                        avdtp_State_s = AVDTP_STATE_STREAMING;
                    }
                    else
                    {
                        uint8 error_code[2];
                        uint8 *p1 = p->payload;
                        error_code[0] = *p1;
                        error_code[1] = AVDTP_BAD_STATE;
                        avdtp_reject_rsp(pcb,a2dphdr, error_code, 2);
                        break;
                    }

                    if(avdtp_notify_pcb.streaming_notify != NULL)
                    {
                        avdtp_notify_pcb.streaming_notify();
                    }

                    break;
                case AVDTP_MSG_TYPE_ACCEPT:
                    if(avdtp_State_s == AVDTP_STATE_GOTO_SUSPEND_QUEUE)
                    {
                        avdtp_State_s = AVDTP_STATE_STREAMING;
                        avdtp_suspend();
                    }
                    else
                    {
                        avdtp_State_s = AVDTP_STATE_STREAMING;
                    }

                    AvdtpStartTmr.enable = 0;
                    AvdtpStartTmr.retry = 0;
                    AvdtpStartTmr.Tick = 0xFFFFF000;

                    //avdtp_State_s = AVDTP_STATE_STREAMING;

                    if(avdtp_notify_pcb.streaming_notify != NULL)
                    {
                        avdtp_notify_pcb.streaming_notify();
                    }

                    break;
                case AVDTP_MSG_TYPE_GEN_REJECT:
                case AVDTP_MSG_TYPE_REJECT:
                    //l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
                    //a2dp_pcbs->l2cappcb = NULL;

                    break;


            }
            pbuf_free(p);
            break;

        case AVDTP_CLOSE:


            switch(message_tpye)
            {

                case AVDTP_MSG_TYPE_COMMAND:
                    if((avdtp_State_s == AVDTP_STATE_STREAMING) || (avdtp_State_s == AVDTP_STATE_OPEN) || (avdtp_State_s == AVDTP_STATE_SUSPEND))
                    {
                        avdtp_close_resp(pcb, p,a2dphdr);
                        avdtp_State_s = AVDTP_STATE_CLOSING;

                        if(avdtp_notify_pcb.close_notify != NULL)
                        {
                            avdtp_notify_pcb.close_notify();
                        }
                    }
                    else
                    {
                        uint8 error_code[1];
                        //uint8 *p1 = p->payload;
                        //error_code[0] = *p1;
                        error_code[0] = AVDTP_BAD_STATE;
                        avdtp_reject_rsp(pcb,a2dphdr, error_code, 1);
                    }

                    break;


                case AVDTP_MSG_TYPE_ACCEPT:
                {

                    if(avdtp_State_s == AVDTP_STATE_WAIT_CLOSE)
                    {

                        if(a2dp_meida_pcbs)
                        {
                            l2ca_disconnect_req(a2dp_meida_pcbs, avdtp_disconnected_cfm);
                        }
                        else
                        {
                            if(a2dp_pcbs && a2dp_pcbs->l2cappcb);
                            l2ca_disconnect_req(a2dp_pcbs->l2cappcb, avdtp_disconnected_cfm);
                        }

                    }

                    avdtp_State_s = AVDTP_STATE_CLOSING;

                    if(avdtp_notify_pcb.close_notify != NULL)
                    {
                        avdtp_notify_pcb.close_notify();
                    }
                }
                break;

                default :
                    break;
            }


            pbuf_free(p);
            break;

        case AVDTP_SUSPEND:

            switch(message_tpye)
            {

                case AVDTP_MSG_TYPE_COMMAND:
                    if(avdtp_State_s == AVDTP_STATE_STREAMING)
                    {
                        avdtp_Suspend_resp(pcb, p,a2dphdr);
                        avdtp_State_s = AVDTP_STATE_SUSPEND;

                        if(avdtp_notify_pcb.suspend_notify != NULL)
                        {
                            avdtp_notify_pcb.suspend_notify();
                        }
                    }
                    else
                    {
                        uint8 error_code[2];
                        uint8 *p1 = p->payload;
                        error_code[0] = *p1;
                        error_code[1] = AVDTP_BAD_STATE;
                        avdtp_reject_rsp(pcb,a2dphdr,error_code,2);
                    }

                    break;

                case AVDTP_MSG_TYPE_ACCEPT:


                    if(avdtp_State_s == AVDTP_STATE_GOTO_START_QUEUE)
                    {
                        avdtp_State_s = AVDTP_STATE_SUSPEND;

                        avdtp_start();
                    }
                    else
                    {
                        avdtp_State_s = AVDTP_STATE_SUSPEND;
                    }

                    AvdtpSuspendTmr.enable = 0;
                    AvdtpSuspendTmr.retry = 0;
                    AvdtpSuspendTmr.Tick = 0xFFFFF000;


                    if(avdtp_notify_pcb.suspend_notify != NULL)
                    {
                        avdtp_notify_pcb.suspend_notify();
                    }


                    break;


                default:

                    break;



            }
            pbuf_free(p);
            break;
        case AVDTP_ABORT:


            switch(message_tpye)
            {

                case AVDTP_MSG_TYPE_COMMAND:
                {
                    avdtp_Abort_resp(pcb, p,a2dphdr);
                    avdtp_State_s = AVDTP_STATE_ABORTING;
                    if(avdtp_notify_pcb.abort_notify != NULL)
                    {
                        avdtp_notify_pcb.abort_notify();
                    }
                }
                break;



                case AVDTP_MSG_TYPE_ACCEPT:


                    if(avdtp_State_s == AVDTP_STATE_WAIT_ABORT)
                    {

                        if(a2dp_meida_pcbs)
                        {
                            l2ca_disconnect_req(a2dp_meida_pcbs, avdtp_disconnected_cfm);
                        }

                    }

                    avdtp_State_s = AVDTP_STATE_SUSPEND;

                    if(avdtp_notify_pcb.abort_notify != NULL)
                    {
                        avdtp_notify_pcb.abort_notify();
                    }

                    break;

                default:

                    break;

            }
            pbuf_free(p);
            break;
       case AVDTP_DELAY_REPORT:
       case AVDTP_RECONFIGURE:
       case AVDTP_SECURITY_CONTROL:
            switch(message_tpye)
            {

                case AVDTP_MSG_TYPE_COMMAND:
                {
                      //avdtp_delayreport_resp(pcb, p,a2dphdr);

                        uint8 error_code[1];
                        error_code[0] = AVDTP_NOT_SUPPORTED_COMMAND;
                        avdtp_reject_rsp(pcb,a2dphdr, error_code, 1);

                }
                break;

            }
            break;

        default:
            {
               // uint8 error_code;
                //error_code = AVDTP_BAD_STATE;
                avdtp_general_reject_rsp(pcb,a2dphdr);
                //l2ca_disconnect_req(a2dp_pcbs->l2cappcb, NULL);
            }
            break;
    }
    return ret;
}

_ATTR_LWBT_UARTIF_CODE_
int avdtp_get_state()
{
   return avdtp_State_s;
}


_ATTR_LWBT_CODE_
int avdtp_send_media(char * buf, int len, int frameCnt, void (*func)(void))
{
    //media_recv = media_data_recv;
    struct pbuf *p = NULL;
    uint16 seqNum;
    int ret = 0;
    if(len > a2dp_meida_pcbs->cfg.outmtu)
    {
        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("send media len is > outmtu\n"));
        return -1;
    }
    if(avdtp_State_s == AVDTP_STATE_STREAMING && a2dp_meida_pcbs != NULL)
    {
        p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
        if (p == NULL)
            return FALSE;

        memcpy(p->payload, buf, len);

        pbuf_header(p,13);

        ((uint8 *)p->payload)[0] = 0x80;
        ((uint8 *)p->payload)[1] = 0x60;
        seqNum = avdtp_next_sequenceNumber();
        ((uint8 *)p->payload)[2] = seqNum >> 8;
        ((uint8 *)p->payload)[3] = seqNum &0x00FF;
        ((uint32 *)p->payload)[1] = 0;
        ((uint32 *)p->payload)[2] = 0;

        ((uint8 *)p->payload)[12] = frameCnt; //sbc Ö¡¸öÊý


        ret = _l2ca_datawrite(a2dp_meida_pcbs,p, func);

        pbuf_free(p);
    }
    return ret;
}

_ATTR_LWBT_UARTIF_CODE_
void avdtp_set_role(int role)
{
    if(a2dp_pcbs)
    {
        a2dp_pcbs->role = role;
        a2dp_pcbs->select_seid = a2dp_pcbs->role+1;
    }
}




_ATTR_LWBT_CODE_
void avdtp_deinit(void)
{

    SystickTimerStop(&AvdtpCheckStateTimer);
    SystickTimerStop(&AvdtpDiscoverTimer);
    SystickTimerStop(&AvdtpMediaSetUpTimer);
    SystickTimerStop(&AvdtpGetGapabilitiesTimer);

    memset(&avdtp_notify_pcb, 0 , sizeof(struct avdtp_notify));
    a2dp_pcbs = NULL;
    avdtp_State_s = 0;
    avdtp_media_ch_state = 0;
    avdtp_recv_cmd = 0;
    avdtp_recv_rsp = 0;
    avdtp_rsp_prev = 0;
//    media_recv = NULL;
}

_ATTR_LWBT_CODE_
void avdtp_tmr(void)
{
    uint systick;
    systick= GetSysTick();
    if(AvdtpStartTmr.enable)
    {
        if(systick > AvdtpStartTmr.Tick+AVDTP_CMD_TIMEOUT)
        {
            if(avdtp_State_s == AVDTP_STATE_WAIT_START_COMPLETE)
            {
                if(AvdtpStartTmr.retry)
                {
                    avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_START);
                    AvdtpStartTmr.retry--;
                    AvdtpStartTmr.Tick = GetSysTick();
                }
                else
                {
                    AvdtpStartTmr.enable = 0;
                    avdtp_State_s = AvdtpStartTmr.old_state;
                }
            }
            else if(avdtp_State_s == AVDTP_STATE_GOTO_SUSPEND_QUEUE)
            {
                //avdtp_State_s = AvdtpStartTmr.old_state;

                avdtp_State_s = AVDTP_STATE_SUSPEND;
                AvdtpStartTmr.enable = 0;
            }


        }
    }



    if(AvdtpSuspendTmr.enable)
    {
        if(systick > AvdtpSuspendTmr.Tick+AVDTP_CMD_TIMEOUT)
        {
            if(avdtp_State_s == AVDTP_STATE_WAIT_SUSPEND_COMPLETE)
            {
                if(AvdtpSuspendTmr.retry)
                {
                    avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL,AVDTP_SUSPEND);
                    AvdtpSuspendTmr.retry--;
                    AvdtpSuspendTmr.Tick = GetSysTick();
                }
                else
                {
                    AvdtpSuspendTmr.enable = 0;
                    avdtp_State_s = AvdtpSuspendTmr.old_state;
                }
            }
            else if(avdtp_State_s == AVDTP_STATE_GOTO_START_QUEUE)
            {
                // avdtp_State_s = AvdtpSuspendTmr.old_state;

                avdtp_State_s = AVDTP_STATE_STREAMING;
                AvdtpSuspendTmr.enable = 0;
            }


        }
    }
}


// lyb add 2017 /1/18
int avdtp_get_configuration_cmd(void)
{
    return(avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL, AVDTP_GET_CONFIGURATION));
}
int avdtp_start_cmd(void)
{
   return(avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL, AVDTP_START));
}

void avdtp_disconnect_pts(void)
{
    //l2ca_disconnect_req(a2dp_pcbs->l2cappcb, avdtp_disconnected_cfm);
    avdtp_disconnect_request(NULL, NULL);
}

int avdtp_close_cmd(void)
{
   return(avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL, AVDTP_CLOSE));
}

int avdtp_abort_cmd(void)
{
   return(avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL, AVDTP_ABORT));
}

int avdtp_get_capabilities_cmd(void)
{
   return(avdtp_cmd(a2dp_pcbs->l2cappcb, NULL, NULL, AVDTP_GET_CAPABILITIES));
}
#if 1
void avdtp_set_configuration_cmd_pts(void)
{
    avdtp_set_configuration_cmd(a2dp_pcbs->l2cappcb, NULL, NULL);
}
#endif
#endif
#endif
/*-----------------------------------------------------------------------------------*/
