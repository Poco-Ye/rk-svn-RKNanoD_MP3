#include <stdlib.h>
#include <stdint.h>
#include "AudioControl.h"
#include "SysInclude.h"
#include "sdp.h"
#include "lwbt_memp.h"
#include "lwbtopts.h"
#include "lwbtdebug.h"
#include "pbuf.h"
#include "lwbt.h"
#include "l2cap.h"
#include "avctp_source.h"

/* Message types */
#define AVCTP_COMMAND           0
#define AVCTP_RESPONSE          1

/* Packet types */
#define AVCTP_PACKET_SINGLE     0
#define AVCTP_PACKET_START      1
#define AVCTP_PACKET_CONTINUE   2
#define AVCTP_PACKET_END        3

#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_

_ATTR_LWBT_BSS_ struct avctp_pcb *avctp_pcbs;
_ATTR_LWBT_BSS_ struct avctp_pcb *avctp_tmp_pcb;
_ATTR_LWBT_BSS_ struct avctp_notify avctp_notify_pcb;

_ATTR_LWBT_DATA_
avctp_conn_state initiator = AVCTP_CONN_NONE;

_ATTR_LWBT_DATA_
SYSTICK_LIST InitiateAvrcpConnTimer =
{
    NULL,
    0,
    400,
    0,
    avctp_InitiateConn,
};


_ATTR_LWBT_DATA_
static struct
{
    const char *name;
    uint8 avc;
    uint16 uinput;
} key_map[] =
{
    { "VOLUME UP",      AVC_VOLUME_UP,      KEY_VOLUMEUP },
    { "VOLUME DOWN",    AVC_VOLUME_DOWN,    KEY_VOLUMEDOWN },
    { "PLAY",           AVC_PLAY,           KEY_PLAYCD },
    { "STOP",           AVC_STOP,           KEY_STOPCD },
    { "PAUSE",          AVC_PAUSE,          KEY_PAUSECD },
    { "FORWARD",        AVC_FORWARD,        KEY_NEXTSONG },
    { "BACKWARD",       AVC_BACKWARD,       KEY_PREVIOUSSONG },
    { "REWIND",         AVC_REWIND,         KEY_REWIND },
    { "FAST FORWARD",   AVC_FAST_FORWARD,   KEY_FASTFORWARD },
    { NULL }
};

_ATTR_LWBT_DATA_
static uint8 avrcp_service_record[] =
{

    0x35, 0x08,
    0x09 ,0x00 ,0x00 ,0x0a ,0x00 ,0x01 ,0x00 ,0x01 ,
    0x35, 0x08,
    0x09 ,0x00 ,0x01 ,0x35 ,0x03 ,0x19 ,0x11 ,0x0C ,
    0x35, 0x08,
    0x09 ,0x00 ,0x02 ,0x0a ,0x00 ,0x00 ,0x00 ,0x06 ,
    0x35, 0x15,
    0x09 ,0x00 ,0x04 ,0x35 ,0x10 ,0x35 ,0x06 ,0x19 ,0x01 ,0x00 ,0x09 ,0x00 ,0x17 ,0x35 ,0x06 ,
    0x19 ,0x00 ,0x17 ,0x09 ,0x01 ,0x04 ,
    0x35, 0x0E,
    0x09 ,0x00 ,0x06 ,0x35 ,0x09 ,0x09 ,0x65 ,0x6e ,0x09 ,0x00 ,0x6a ,0x09 ,0x01 ,0x00 ,
    0x35, 0x0D,
    0x09 ,0x00 ,0x09 ,0x35 ,0x08 ,0x35 ,0x06 ,0x19 ,0x11 ,0x0e ,0x09 ,0x01 ,0x03,
    0x35, 0x15,
    0x09 ,0x01 ,0x00 ,0x25 ,0x10 ,0x41 ,0x56 ,0x52 ,0x43 ,0x50 ,0x20 ,0x43 ,0x6f ,0x6e ,0x74,
    0x72 ,0x6f ,0x6c ,0x6c ,0x65 ,0x72 ,
    0x35, 0x14,
    0x09 ,0x01 ,0x02 ,0x25 ,0x0f ,0x53 ,0x74 ,0x6f ,0x6e ,0x65 ,0x73 ,0x74 ,0x72 ,0x65 ,0x65,
    0x74 ,0x20 ,0x4f ,0x6e ,0x65 ,
    0x35, 0x6,
    0x09 ,0x03 ,0x11 ,0x09 ,0x00 ,0x01 ,0x0

};
static void avctp_set_state(struct avctp_pcb *session, avctp_state_t new_state);

int UnicodeToUTF_8(uint16 *strUnicode, uint32 strUnicodeLen, uint8 *strUTF8, uint32 strUTF8Len);

_ATTR_LWBT_CODE_
uint32 avctp_get_profile_versions(void)
{
    return 0x104;
}

_ATTR_LWBT_CODE_
uint32 avrcp_get_profile_versions(void)
{
    return 0x103;
}

_ATTR_LWBT_INIT_CODE_
void avctp_connected_notify(void(*connected_notify)(void))
{
    avctp_notify_pcb.connected_notify = connected_notify;
}


_ATTR_LWBT_INIT_CODE_
void avctp_close_notify(void(*close_notify)(void))
{
    avctp_notify_pcb.close_notify = close_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_play_notify(void(*audio_play_notify)(void))
{
    avctp_notify_pcb.audio_play= audio_play_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_stop_notify(void(*audio_stop_notify)(void))
{
    avctp_notify_pcb.audio_stop= audio_stop_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_pause_notify(void(*audio_pause_notify)(void))
{
    avctp_notify_pcb.audio_pause = audio_pause_notify;
}


_ATTR_LWBT_INIT_CODE_
void avctp_audio_next_notify(void(*audio_next_notify)(void))
{
    avctp_notify_pcb.audio_next= audio_next_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_previous_notify(void(*audio_previous_notify)(void))
{
    avctp_notify_pcb.audio_previous= audio_previous_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_mute_notify(void(*audio_mute_notify)(void))
{
    avctp_notify_pcb.audio_mute= audio_mute_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_volumeup_notify(void(*audio_volumeup_notify)(void))
{
    avctp_notify_pcb.audio_volumeup= audio_volumeup_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_volumedown_notify(void(*audio_volumedown_notify)(void))
{
    avctp_notify_pcb.audio_volumedown= audio_volumedown_notify;
}

_ATTR_LWBT_INIT_CODE_
void avctp_audio_volume_change_notify(void(*audio_volume_change_notify)(uint8 volume))
{
    avctp_notify_pcb.audio_volume_change = audio_volume_change_notify;
}

/*-----------------------------------------------------------------------------------*/
/*
 * avctp_new():
 *
 * malloc space the avctp protocol control block.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
struct avctp_pcb *
avctp_new()
{
    struct avctp_pcb *pcb;
    pcb = lwbt_memp_malloc(MEMP_AVCTP_PCB);
    if(pcb != NULL)
    {
        memset(pcb, 0, sizeof(struct avctp_pcb));
//      pcb->l2cappcb = l2cappcb;
        return pcb;
    }

    return NULL;
}

/*-----------------------------------------------------------------------------------*/
/*
 * avctp_free():
 *
 * Free the avctp protocol control block.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void avctp_free(struct avctp_pcb *pcb)
{
    lwbt_memp_free(MEMP_AVCTP_PCB, pcb);
    pcb = NULL;
}

_ATTR_LWBT_INIT_CODE_
void avctp_init(void)
{
    struct sdp_record *record;
    avctp_pcbs = NULL;
    avctp_pcbs = avctp_new();

    if(avctp_pcbs == NULL)
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_init: Could not alloc avctp memory\n"));
        return;
    }

    //allocate memory to use initiating to connect avrcp.
    avctp_tmp_pcb = avctp_new();
    if(avctp_tmp_pcb == NULL)
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_init: Could not alloc avctp memory\n"));
        return;
    }

#if LWBT_LAP
    l2cap_connect_ind(NULL, AVCTP_PSM, avctp_l2cap_connected_ind);
#endif

    if((record = sdp_record_new((uint8 *)avrcp_service_record, sizeof(avrcp_service_record))) == NULL)
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("Could not alloc SDP record\n"));
        //return ERR_MEM;
    }
    else
    {
        sdp_register_service(record);
    }
}


_ATTR_LWBT_INIT_CODE_
void avctp_deinit(void)
{
    SystickTimerStop(&InitiateAvrcpConnTimer);
    memset(&avctp_notify_pcb, 0 ,sizeof(struct avctp_notify));

    initiator = AVCTP_CONN_NONE;
    avctp_pcbs = NULL;
    avctp_tmp_pcb = NULL;
}


/*-----------------------------------------------------------------------------------*/
/*
 * avctp_reset_all():
 *
 * Free all SDP protocol control blocks and registered records.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void avctp_reset_all()
{
    struct avctp_pcb *pcb, *tpcb;

    for(pcb = avctp_pcbs; pcb != NULL;)
    {
        tpcb = pcb->next;
        avctp_free(pcb);
        pcb = tpcb;
    }

    if(avctp_tmp_pcb != NULL)
        avctp_free( avctp_tmp_pcb );

    avctp_init();
}

_ATTR_LWBT_CODE_
void avctp_InitiateConn(void)
{
    if(initiator != AVCTP_CONN_CONNECTED)
    {
        avctp_connect_req(NULL,&(avctp_tmp_pcb->l2cappcb->remote_bdaddr),AVCTP_PSM,1,avctp_connected);
        //initiator = AVCTP_CONN_INITIATOR;
    }
    SystickTimerStop(&InitiateAvrcpConnTimer);
}

_ATTR_LWBT_CODE_
void avctp_startTimer(struct l2cap_pcb *pcb)
{
    //workaround timer start next time.
    if(initiator == AVCTP_CONN_CONNECTED)   //avctp profile has connected successfully .
        return;

    if( pcb != NULL )
        avctp_tmp_pcb->l2cappcb = pcb;

    SystickTimerStart(&InitiateAvrcpConnTimer);
}

_ATTR_LWBT_CODE_
void avctp_StopTimer(void)
{
    //workaround timer start next time.

    SystickTimerStop(&InitiateAvrcpConnTimer);
}


_ATTR_LWBT_CODE_
err_t avctp_l2cap_connected_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    err_t ret = ERR_OK;

    LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_l2cap_connected_ind: L2CAP connected\n"));

    if(pcb->psm == AVCTP_PSM)
    {
        initiator = AVCTP_CONN_CONNECTED;
        //avrtp_init(); //mlc add .

        if(avctp_pcbs->l2cappcb == NULL)
        {
            l2cap_recv(pcb, avctp_recv);
            avctp_pcbs->l2cappcb = pcb;
            l2cap_disconnect_ind(pcb, avctp_lp_disconnected);
        }

        if(avctp_notify_pcb.connected_notify != NULL)
        {
            avctp_notify_pcb.connected_notify();
        }

        avrcp_init();
    }
    else
        ret = ERR_VAL;

    return ret;
}


//_ATTR_LWBT_CODE_
//struct avctp_pcb* avctp_connect()
//{
//  struct avctp_pcb *session;
//  avctp_init();
//
//  session = avctp_pcbs;
//  if (!session)
//      return NULL;
//
//  if (session->state > AVCTP_STATE_DISCONNECTED)
//      return session;
//
//  avctp_set_state(session, AVCTP_STATE_CONNECTING);
//
//  return session;
//}

_ATTR_LWBT_CODE_
err_t avctp_connect_req(struct avctp_pcb *pcb, struct bd_addr *bdaddr, uint16 psm, \
                        uint8 role_switch, err_t (* l2ca_connect_cfm)(void *arg, struct l2cap_pcb *lpcb,\
                                uint16 result, uint16 status))
{
    struct l2cap_pcb *l2cappcb;

    if((l2cappcb = l2cap_new()) == NULL)
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_SERIOUS_, ("avctp_connect_req: Could not alloc L2CAP pcb\n"));
        return;
    }

    l2ca_connect_req(l2cappcb, bdaddr, AVCTP_PSM, role_switch, avctp_connected);
}

_ATTR_LWBT_CODE_
err_t avctp_connected(void *arg, struct l2cap_pcb *l2cappcb, uint16 result, uint16 status)
{
    if(result == L2CAP_CONN_SUCCESS)
    {
        //avrtp_init();
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_SERIOUS_, ("avctp_connected\n"));
        initiator = AVCTP_CONN_CONNECTED;

        if(avctp_pcbs->l2cappcb == NULL)
        {
            l2cap_recv(l2cappcb, avctp_recv);
            avctp_pcbs->l2cappcb = l2cappcb;
        }

        l2cap_disconnect_ind(l2cappcb, avctp_lp_disconnected);

        avctp_set_state(avctp_pcbs, AVCTP_STATE_CONNECTED);
        if(avctp_notify_pcb.connected_notify != NULL)
        {
            avctp_notify_pcb.connected_notify();
        }

        avrcp_init();
    }
    else
    {
        l2cap_close(l2cappcb);
    }

    return ERR_OK;

}


_ATTR_LWBT_CODE_
static void avctp_set_state(struct avctp_pcb *session, avctp_state_t new_state)
{
    avctp_state_t old_state = session->state;

    session->state = new_state;

}

_ATTR_LWBT_CODE_
void avctp_lp_disconnected(void *arg, struct l2cap_pcb *l2cappcb, err_t err)
{
    struct avctp_pcb *pcb;
    LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_lp_disconnected"));
    pcb = avctp_pcbs;
    if(pcb != NULL)
    {
        pcb->l2cappcb = NULL;
        pcb->tid = 0;
    }

    l2cap_close(l2cappcb);
    //avrtp_deinit();
    initiator = AVCTP_CONN_NONE;

    if (pcb->state == AVCTP_STATE_DISCONNECTED)
        return;

    avctp_set_state(pcb, AVCTP_STATE_DISCONNECTED);
}

_ATTR_LWBT_CODE_
int avctp_disconnected_request(err_t (* avctp_disconnect_cfm)(void *arg, struct l2cap_pcb *pcb))
{
    int ret;

    if(initiator != AVCTP_CONN_CONNECTED)
    {
        return RETURN_FAIL;
    }

    LWBT_DEBUGF(AVCTP_DEBUG, _DBG_SERIOUS_, ("avctp_disconnected_request\n"));
    if(avctp_pcbs != NULL && avctp_pcbs->l2cappcb != NULL)
    {
        l2ca_disconnect_req(avctp_pcbs->l2cappcb, avctp_disconnect_cfm);
    }

    return RETURN_OK;

}


_ATTR_LWBT_CODE_
static int avctp_send(uint8 transaction,
                      uint8 cr, uint8 code,
                      uint8 subunit, uint8 opcode,
                      uint8 *operands, size_t operand_count)
{
    int ret;
    struct avctp_header *avctp;
    struct avc_header *avc;
    PBUF * send_pbuf = NULL/*,tmp_buf = NULL*/;
    void * operands_array;
    void * ptmp;

    send_pbuf  = pbuf_alloc(PBUF_RAW, AVCTP_HEADER_LENGTH
                            + AVC_HEADER_LENGTH
                            + operand_count,
                            PBUF_RAM);
    if (send_pbuf == NULL)
        return FALSE;

    avctp = send_pbuf->payload;

    ptmp= /*(struct avc_header *)*/(void *)((int)avctp + AVCTP_HEADER_LENGTH);
    avc = (struct avc_header *)ptmp;

    avctp->transaction = transaction;
    avctp->packet_type = AVCTP_PACKET_SINGLE;
    avctp->cr = cr;
    avctp->pid = htons(AV_REMOTE_SVCLASS_ID);   //0x110e
    avctp->ipid = 0;


    avc->code = code;
    avc->_hdr0 = 0;
    avc->subunit = (0xff & subunit << AVRC_SUBTYPE_SHIFT) | 0x00;
    avc->opcode = opcode;

    operands_array = (void *)((int)avc + AVC_HEADER_LENGTH);
    memcpy(operands_array,operands,operand_count);

    ret = l2ca_datawrite(avctp_pcbs->l2cappcb, send_pbuf);

    pbuf_free(send_pbuf);

    return ret;
}


_ATTR_LWBT_CODE_
int avctp_send_vendordep_req(struct avctp_pcb *session, uint8 code,
                             uint8 subunit, uint8 *operands,
                             size_t operand_count)
{
    return avctp_send_req(session, code, subunit, AVC_OP_VENDORDEP,
                          operands, operand_count);
}

_ATTR_LWBT_CODE_
int avctp_send_vendordep_rsp(struct avctp_pcb *session, uint8 code,
                             uint8 subunit, uint8 *operands,
                             size_t operand_count)
{
    return avctp_send_rsp(session, code, subunit, AVC_OP_VENDORDEP,
                          operands, operand_count);
}

_ATTR_LWBT_CODE_
static int process_control_rsp(struct avctp_control_req *data)
{
    struct avctp_control_req *req = data;

    return avctp_send( req->transaction, AVCTP_RESPONSE, req->code,
                       req->subunit, req->op,
                       req->operands, req->operand_count);

}


_ATTR_LWBT_CODE_
static int process_control(struct avctp_control_req *data)
{
    struct avctp_control_req *req = data;

    return avctp_send( req->transaction, AVCTP_COMMAND, req->code,
                       req->subunit, req->op,
                       req->operands, req->operand_count);

    //4NOTE: transaction id   need update in next request.
}

_ATTR_LWBT_CODE_
int avctp_vendordep_response(struct avctp_pcb *session,uint8 code, uint8 *operands, size_t operands_count)
{
    struct avctp_control_req req;
    int ret = 0;

    req.code = code;
    req.subunit = AVC_SUBUNIT_PANEL;
    req.op = AVC_OP_VENDORDEP;

    req.operands = operands;
    req.operand_count = operands_count;

    req.transaction = session->resp_tid;

    ret = process_control_rsp( &req );
    if(ret != ERR_OK)
    {
        return -1;
    }

    return 0;
}


_ATTR_LWBT_CODE_
static int avctp_send_req(struct avctp_pcb *session, uint8 code,
                          uint8 subunit, uint8 opcode,
                          uint8 *operands, size_t operand_count)
{
    struct avctp_control_req *req;

    struct pbuf * oprand_pbuf = NULL;
    struct pbuf * ctrl_req_pbuf = NULL;
    int ret = 0;

    ctrl_req_pbuf = pbuf_alloc(PBUF_RAW, sizeof(struct avctp_control_req), PBUF_RAM);
    if( ctrl_req_pbuf == NULL)
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_send_req 0 alloc fail\n"));
        return -1;
    }
    req = ctrl_req_pbuf->payload;

    oprand_pbuf = pbuf_alloc(PBUF_RAW, operand_count, PBUF_RAM);
    if( oprand_pbuf == NULL)
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_send_req 1 alloc fail\n"));
        pbuf_free(ctrl_req_pbuf);
        ctrl_req_pbuf = NULL;
        return -1;
    }
    memcpy(oprand_pbuf->payload,operands,operand_count);

    req->code = code;
    req->subunit = subunit;
    req->op = opcode;

    req->operands = oprand_pbuf->payload;
    req->operand_count = operand_count;

    req->transaction = ++session->tid;  //start from 1
    req->transaction %= 16;             //最大16个未处理事务

    ret = process_control( req );
    if(ret != ERR_OK)
    {
    }

    pbuf_free(oprand_pbuf);
    oprand_pbuf = NULL;
    pbuf_free(ctrl_req_pbuf);
    ctrl_req_pbuf = NULL;

    return 0;
}


_ATTR_LWBT_CODE_
static int avctp_send_rsp(struct avctp_pcb *session, uint8 code,
                          uint8 subunit, uint8 opcode,
                          uint8 *operands, size_t operand_count)
{
    struct avctp_control_req *req;

    struct pbuf * ctrl_req_pbuf = NULL;
    int ret = 0;

    ctrl_req_pbuf = pbuf_alloc(PBUF_RAW, sizeof(struct avctp_control_req), PBUF_RAM);
    if( ctrl_req_pbuf == NULL)
        return -1;

    req = ctrl_req_pbuf->payload;

    req->code = code;
    req->subunit = subunit;
    req->op = opcode;

    //DEBUG( " ======= operands[0,1] = 0x%02x,0x%02x",operands[0],operands[1] );

    req->operands = operands;
    req->operand_count = operand_count;

    req->transaction = session->resp_tid;  //start from 1
    req->transaction %= 16;

//  ret = process_control( req );

    ret = avctp_send( req->transaction, AVCTP_RESPONSE, req->code,
                      req->subunit, req->op,
                      req->operands, req->operand_count);
    if(ret != ERR_OK)
    {
        //something error happened.
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_send fail\n"));
        //return ret;
    }

    pbuf_free(ctrl_req_pbuf);
    ctrl_req_pbuf = NULL;

    return ret;
}


static char *op2str(uint8 op)
{
    switch (op & 0x7f)
    {
        case AVC_VOLUME_UP:
            return "VOLUME UP";
        case AVC_VOLUME_DOWN:
            return "VOLUME DOWN";
        case AVC_MUTE:
            return "MUTE";
        case AVC_PLAY:
            return "PLAY";
        case AVC_STOP:
            return "STOP";
        case AVC_PAUSE:
            return "PAUSE";
        case AVC_RECORD:
            return "RECORD";
        case AVC_REWIND:
            return "REWIND";
        case AVC_FAST_FORWARD:
            return "FAST FORWARD";
        case AVC_EJECT:
            return "EJECT";
        case AVC_FORWARD:
            return "FORWARD";
        case AVC_BACKWARD:
            return "BACKWARD";
        default:
            return "UNKNOWN";
    }
}

#if 0
_ATTR_LWBT_CODE_
int avctp_send_passthrough(struct avctp_pcb *session, uint8 op)
{
    return avctp_passthrough_press(session, op);
}


_ATTR_LWBT_CODE_
static int avctp_passthrough_press(struct avctp_pcb *session, uint8 op)
{
    uint8 operands[2];

    /* Button pressed */
    operands[0] = op & 0x7f;        //button is press
    operands[1] = 0;

    return avctp_send_req(session, AVC_CTYPE_CONTROL,
                          AVC_SUBUNIT_PANEL, AVC_OP_PASSTHROUGH,
                          operands, sizeof(operands));
}


_ATTR_LWBT_CODE_
static int avctp_passthrough_release(struct avctp_pcb*session, uint8 op)
{
    uint8 operands[2];

    /* Button released */
    operands[0] = op | 0x80;    //button is release
    operands[1] = 0;

    return avctp_send_req(session, AVC_CTYPE_CONTROL,
                          AVC_SUBUNIT_PANEL, AVC_OP_PASSTHROUGH,
                          operands, sizeof(operands));
}
#endif

_ATTR_LWBT_CODE_
int avctp_passthrough_response(struct avctp_pcb *session,uint8 code,uint8 *operands)
{
    struct avctp_control_req req;

    struct pbuf * oprand_pbuf = NULL;
    struct pbuf * ctrl_req_pbuf = NULL;
    int ret = 0;

    uint8 operands_buf[2];

    operands_buf[0] = operands[0];
    operands_buf[1] = operands[1];

    req.code = code;
    req.subunit = AVC_SUBUNIT_PANEL;
    req.op = AVC_OP_PASSTHROUGH;

    req.operands = operands_buf;
    req.operand_count = sizeof(operands_buf);

    req.transaction = session->resp_tid;

    ret = process_control_rsp( &req );
    if(ret != ERR_OK)
    {
    }

    return 0;
}


_ATTR_LWBT_CODE_
bool avctp_command_process(struct avctp_pcb *session,uint8 code,uint8 *operands)
{
    uint8 op;
    if( code != AVC_CTYPE_CONTROL )
        return FALSE;

    //if( operands[0] & 0x80 == (uint8)0x80)    //means button release command
    if( operands[0] >> 7 == 0x01)   //means button release command
    {
        avctp_passthrough_response(session,AVC_CTYPE_ACCEPTED,operands);    //response the command to CT
        return TRUE;
    }

    op = operands[0] & 0x7f;

    //DEBUG("process cmd op = %s.",op2str(op));
    LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("process cmd op = %s.",op2str(op)));
    switch( op )
    {
        case AVC_PLAY:
            if(avctp_notify_pcb.audio_play != NULL)
                avctp_notify_pcb.audio_play();
            break;

        case AVC_STOP:
            if(avctp_notify_pcb.audio_stop!= NULL)
                avctp_notify_pcb.audio_stop();
            break;

        case AVC_PAUSE:
            if(avctp_notify_pcb.audio_pause!= NULL)
                avctp_notify_pcb.audio_pause();

            break;

        case AVC_RECORD:
            avctp_passthrough_response(session,AVC_CTYPE_NOT_IMPLEMENTED,operands);
            return FALSE;

        case AVC_REWIND:
            if(avctp_notify_pcb.audio_ffw!= NULL)
                avctp_notify_pcb.audio_ffw();
            break;

        case AVC_FAST_FORWARD:
            if(avctp_notify_pcb.audio_ffd!= NULL)
                avctp_notify_pcb.audio_ffd();
            break;

        case AVC_FORWARD:
            if(avctp_notify_pcb.audio_next!= NULL)
                avctp_notify_pcb.audio_next();
            break;

        case AVC_BACKWARD:
            if(avctp_notify_pcb.audio_previous!= NULL)
                avctp_notify_pcb.audio_previous();

            break;

        default:
            avctp_passthrough_response(session,AVC_CTYPE_REJECTED,operands);    //response the command to CT
            return FALSE;
    }

    avctp_passthrough_response(session,AVC_CTYPE_ACCEPTED,operands);    //response the command to CT
    return TRUE;
}


_ATTR_LWBT_CODE_
static size_t handle_unit_info(struct avctp_pcb *session,
                               uint8_t transaction, uint8_t *code,
                               uint8_t *subunit, uint8_t *operands,
                               size_t operand_count)
{
    uint8 *operands_tmp;
    if (*code != AVC_CTYPE_STATUS)
    {
        *code = AVC_CTYPE_REJECTED;
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, (" code Err. "));
        return 0;
    }

    operands_tmp = operands;

    *code = AVC_CTYPE_STABLE;

    /* The first operand should be 0x07 for the UNITINFO response.
     * Neither AVRCP (section 22.1, page 117) nor AVC Digital
     * Interface Command Set (section 9.2.1, page 45) specs
     * explain this value but both use it */
    if (operand_count >= 1)
        *operands_tmp++ = 0x07;

    if (operand_count >= 2)
        *operands_tmp++ = AVC_SUBUNIT_PANEL << 3;

    AVRC_CO_ID_TO_BE_STREAM(operands_tmp,AVRC_CO_BLUETOOTH_SIG);

    return avctp_send_rsp(session, *code,
                          AVRC_SUB_UNIT << AVRC_SUBTYPE_SHIFT | AVRC_SUBID_MASK, AVC_OP_UNITINFO,
                          operands, operand_count);

    return operand_count;
}




_ATTR_LWBT_CODE_
static size_t handle_subunit_info(struct avctp_pcb *session,
                                  uint8_t transaction, uint8_t *code,
                                  uint8_t *subunit, uint8_t *operands,
                                  size_t operand_count)
{
    if (*code != AVC_CTYPE_STATUS)
    {
        *code = AVC_CTYPE_REJECTED;
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, (" code Err. "));
        return 0;
    }

    *code = AVC_CTYPE_STABLE;

    /* The first operand should be 0x07 for the UNITINFO response.
     * Neither AVRCP (section 22.1, page 117) nor AVC Digital
     * Interface Command Set (section 9.2.1, page 45) specs
     * explain this value but both use it */
    if (operand_count >= 2)
        operands[1] = AVC_SUBUNIT_PANEL << 3;

    return avctp_send_rsp(session, *code,
                          AVRC_SUB_UNIT << AVRC_SUBTYPE_SHIFT | AVRC_SUBID_MASK, AVC_OP_SUBUNITINFO,
                          operands, operand_count);

    return operand_count;
}

/*--------------------------------------------------------------
 * avctp_recv():
 *
 * Called by the lower layer.
/*---------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t avctp_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *s, err_t err)
{
    struct avctp_header* avctp_hdr;
    struct avc_header * avc_hdr;

    struct avctp_pcb *avctppcb;

    err_t ret = ERR_OK;
    uint16 i;
    struct pbuf *p, *q, *r;
    void * ptemp;
    uint8 *operands, code, subunit;
    int result,packet_size, operand_count;
    bool bret;

    p = s;

    avctp_hdr = p->payload;

    ptemp = (void *)((int)avctp_hdr + AVCTP_HEADER_LENGTH);
    avc_hdr = (struct avc_header *)ptemp;

    operands = (uint8 *)((int)avctp_hdr + AVCTP_HEADER_LENGTH + AVC_HEADER_LENGTH);

    operand_count =p->len - (operands - (uint8 *)(p->payload));

    avctppcb = avctp_pcbs;

    code =  avc_hdr->code;
    if (code == AVC_CTYPE_REJECTED)
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("recive a reject avrcp packet!"));
        goto done;
    }
//  packet_size = AVCTP_HEADER_LENGTH + AVC_HEADER_LENGTH;

    //avctppcb->tid = avctp_hdr->transaction;
    avctppcb->resp_tid = avctp_hdr->transaction;
    if( avctp_hdr->cr == AVCTP_RESPONSE)
    {
        //DEBUG("AVCTP_RESPONSE:opcode = 0x%x\n", avc_hdr->opcode);
        if (avctp_hdr->packet_type != AVCTP_PACKET_SINGLE)
        {
            avc_hdr->code = AVC_CTYPE_NOT_IMPLEMENTED;
            goto done;
        }

        if(avctp_hdr->pid != htons( AV_REMOTE_SVCLASS_ID ))
        {
            LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("error: error profile ID.\n"));
            goto done;
        }
        else //is the avctp profile.
        {
            code = avc_hdr->code;
            switch( avc_hdr->opcode )
            {
                case AVC_OP_VENDORDEP:
#ifdef _AVRCP_VERSION_1_4_
                    bret = avrtp_vendordep_rsp(avctppcb,code,operands, avc_hdr);
#endif
                    break;
                default:
                    break;
            }
        }
    }
    else if( avctp_hdr->cr == AVCTP_COMMAND )
    {
        LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("avctp_recv: command. "));
        //DEBUG("opcode = 0x%x", avc_hdr->opcode);

        if (avctp_hdr->packet_type != AVCTP_PACKET_SINGLE)
        {
            avc_hdr->code = AVC_CTYPE_NOT_IMPLEMENTED;
            goto done;
        }

        if(avctp_hdr->pid != htons( AV_REMOTE_SVCLASS_ID ))
        {
            LWBT_DEBUGF(AVCTP_DEBUG, _DBG_INFO_, ("error: error profile ID.\n"));
            goto done;
        }
        else //is the avctp profile.
        {
            code = avc_hdr->code;
            subunit = avc_hdr->subunit >> 3 & 0xff;

            switch( avc_hdr->opcode )
            {
                case AVC_OP_PASSTHROUGH:
                    bret = avctp_command_process(avctppcb,code,operands);
                    break;
                case AVC_OP_UNITINFO:
                    handle_unit_info(avctppcb,0,&code,&subunit,operands,operand_count);
                    break;
                case AVC_OP_SUBUNITINFO:
                    handle_subunit_info(avctppcb,0,&code,&subunit,operands,operand_count);
                    break;
                case AVC_OP_VENDORDEP:
                    bret = handle_vendordep_req(avctppcb,code,operands);
                    break;
                default:
                    break;
            }
        }

        if( !bret )
            goto done;
    }

done:
    pbuf_free(p);
    return ret;
}

#endif
#endif

