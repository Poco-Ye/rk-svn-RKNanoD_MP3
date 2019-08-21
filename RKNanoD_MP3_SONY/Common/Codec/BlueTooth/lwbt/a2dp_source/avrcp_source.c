#include <stdlib.h>
#include <stdint.h>
#include "SysInclude.h"
#include "sdp.h"
#include "lwbt_memp.h"
#include "lwbtopts.h"
#include "lwbtdebug.h"
#include "pbuf.h"
#include "avctp_source.h"
#include "lwbt.h"
#include "avrcp_source.h"
#include "l2cap.h"
#include "AudioControl.h"

#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_
#ifdef _AVRCP_

/* Company IDs for vendor dependent commands */
#define IEEEID_BTSIG                        0x001958

/* Status codes */
#define AVRCP_STATUS_INVALID_COMMAND          0x00
#define AVRCP_STATUS_INVALID_PARAM                0x01
#define AVRCP_STATUS_PARAM_NOT_FOUND          0x02
#define AVRCP_STATUS_INTERNAL_ERROR               0x03
#define AVRCP_STATUS_SUCCESS                  0x04
#define AVRCP_STATUS_INVALID_PLAYER_ID            0x11
#define AVRCP_STATUS_NO_AVAILABLE_PLAYERS     0x15
#define AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED 0x16

/* Packet types */
#define AVRCP_PACKET_TYPE_SINGLE                0x00
#define AVRCP_PACKET_TYPE_START                 0x01
#define AVRCP_PACKET_TYPE_CONTINUING            0x02
#define AVRCP_PACKET_TYPE_END                   0x03

/* PDU types for metadata transfer */
#define AVRCP_GET_CAPABILITIES                  0x10
#define AVRCP_LIST_PLAYER_ATTRIBUTES            0X11
#define AVRCP_LIST_PLAYER_VALUES                0x12
#define AVRCP_GET_CURRENT_PLAYER_VALUE          0x13
#define AVRCP_SET_PLAYER_VALUE                  0x14
#define AVRCP_GET_PLAYER_ATTRIBUTE_TEXT         0x15
#define AVRCP_GET_PLAYER_VALUE_TEXT             0x16
#define AVRCP_DISPLAYABLE_CHARSET               0x17
#define AVRCP_CT_BATTERY_STATUS                 0x18
#define AVRCP_GET_ELEMENT_ATTRIBUTES            0x20
#define AVRCP_GET_PLAY_STATUS                   0x30
#define AVRCP_REGISTER_NOTIFICATION             0x31
#define AVRCP_REQUEST_CONTINUING                0x40
#define AVRCP_ABORT_CONTINUING                  0x41
#define AVRCP_SET_ABSOLUTE_VOLUME               0x50
#define AVRCP_SET_BROWSED_PLAYER                0x70
#define AVRCP_GET_ITEM_ATTRIBUTES               0x73
#define AVRCP_GET_FOLDER_ITEMS                  0x71
#define AVRCP_GENERAL_REJECT                    0xA0

/* Capabilities for AVRCP_GET_CAPABILITIES pdu */
#define CAP_COMPANY_ID                          0x02
#define CAP_EVENTS_SUPPORTED                    0x03

#define AVRCP_REGISTER_NOTIFICATION_PARAM_LENGTH    5
#define AVRCP_GET_CAPABILITIES_PARAM_LENGTH         1

#define AVRCP_FEATURE_CATEGORY_1                0x0001
#define AVRCP_FEATURE_CATEGORY_2                0x0002
#define AVRCP_FEATURE_CATEGORY_3                0x0004
#define AVRCP_FEATURE_CATEGORY_4                0x0008

#define AVRCP_FEATURE_PLAYER_SETTINGS           0x0010
#define AVRCP_FEATURE_BROWSING                  0x0040

#define AVRCP_BATTERY_STATUS_NORMAL             0
#define AVRCP_BATTERY_STATUS_WARNING            1
#define AVRCP_BATTERY_STATUS_CRITICAL           2
#define AVRCP_BATTERY_STATUS_EXTERNAL           3
#define AVRCP_BATTERY_STATUS_FULL_CHARGE        4

#define AVRCP_MTU       (AVC_MTU - AVC_HEADER_LENGTH)
#define AVRCP_PDU_MTU   (AVRCP_MTU - AVRCP_HEADER_LENGTH)

#define AVRCP_FRAGMENTTATION_LEN 512

_ATTR_LWBT_DATA_ uint8 avrcp_play_volume = -1;
_ATTR_LWBT_BSS_ BOOL player_status_changed_notify;
_ATTR_LWBT_BSS_ uint8 player_status_tid;

_ATTR_LWBT_BSS_ BOOL track_changed_notify;
_ATTR_LWBT_BSS_ uint8 track_changed_tid;

_ATTR_LWBT_BSS_ BOOL avrcp_batterystatus_notify;
_ATTR_LWBT_BSS_ uint8 batterystatus_tid;

_ATTR_LWBT_BSS_ BOOL avrcp_playback_pos_notify;
_ATTR_LWBT_BSS_ uint8 playback_pos_tid;

_ATTR_LWBT_BSS_ BOOL avrcp_app_setting_notify;
_ATTR_LWBT_BSS_ uint8 app_setting_tid;
_ATTR_LWBT_BSS_ uint32  avrcp_playback_pos_interval;

_ATTR_LWBT_BSS_ BOOL avrcp_volume_manage;
_ATTR_LWBT_BSS_ BOOL avrcp_volume_changed_flag;


__packed
struct avrcp_metadata_info_save
{
    struct avrcp_header avrcp_hdr;
    int leftcnt;
    uint8 buf[512];  //params[0]
};

_ATTR_LWBT_BSS_ struct avrcp_metadata_info_save metadata_info;



/* Company IDs supported by this device */
_ATTR_LWBT_DATA_
static uint32 company_ids[] =
{
    IEEEID_BTSIG,
};

extern struct avctp_pcb *avctp_pcbs;
extern struct avctp_notify avctp_notify_pcb;

__packed
struct avrtp_attribute_entry
{
    uint32 att_id;
    uint16 character_set;
    uint16 att_value_len;
    uint8 *att_value;
};
#define ATTRIBUTE_ENTRY_HEAD_LEN 8
__packed
struct avrcp_play_status
{
    uint32 song_length;
    uint32 song_position;
    uint8  play_status;
};
__packed
struct avrcp_capability
{
    uint8 capability_id;
    uint8 capability_count;
    uint8 event_id[256];
};

/*-----------------------------------------------------------------
//                      global variable
-----------------------------------------------------------------*/
_ATTR_LWBT_BSS_
struct avrcp_pcb *avrcp_pcbs;

//_ATTR_LWBT_CODE_
//static char *status_to_str(uint8 status)
//{
//  switch(status){
//      case AVRCP_STATUS_INVALID_COMMAND:
//          return "Invalid Command";
//      case AVRCP_STATUS_INVALID_PARAM:
//          return "Invalid Parameter";
//      case AVRCP_STATUS_INTERNAL_ERROR:
//          return "Internal Error";
//      case AVRCP_STATUS_SUCCESS:
//          return "Success";
//      default:
//          return "Unknown";
//  }
//}

//_ATTR_LWBT_CODE_
//static char *paly_status_to_string(uint8 status)
//{
//  switch(status){
//  case AVRCP_PLAY_STATUS_STOPPED:
//      return "stopped";
//  case AVRCP_PLAY_STATUS_PLAYING:
//      return "playing";
//  case AVRCP_PLAY_STATUS_PAUSED:
//      return "paused";
//  case AVRCP_PLAY_STATUS_FWD_SEEK:
//      return "forward-seek";
//  case AVRCP_PLAY_STATUS_REV_SEEK:
//      return "reverse_seek";
//  case AVRCP_PLAY_STATUS_ERROR:
//      return "error";
//  default :
//      return NULL;
//  }
//}
//
/*-----------------------------------------------------------------------------------*/
/*
 * avrcp_new():
 *
 * malloc the avrcp protocol control block.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
struct avrcp_pcb * avrcp_new()
{
    struct avrcp_pcb *pcb;
    pcb = lwbt_memp_malloc(MEMP_AVRCP_PCB);
    if(pcb != NULL)
    {
        memset(pcb, 0, sizeof(struct avrcp_pcb));
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
_ATTR_LWBT_CODE_
void avrcp_free(struct avrcp_pcb *pcb)
{
    lwbt_memp_free(MEMP_AVRCP_PCB, pcb);
    pcb = NULL;
}

/*
 * set_company_id:
 *
 * Set three-byte Company_ID into outgoing AVRCP message
 */
_ATTR_LWBT_CODE_
static void set_company_id(uint8 cid[3], const uint32 cid_in)
{
    cid[0] = cid_in >> 16;
    cid[1] = cid_in >> 8;
    cid[2] = cid_in;
}

_ATTR_LWBT_CODE_
static void avrcp_register_notification(struct avrcp_pcb *rcp_session, uint8 event)
{
    uint8 buf[AVRCP_HEADER_LENGTH + AVRCP_REGISTER_NOTIFICATION_PARAM_LENGTH];
    struct avrcp_header *pdu = (void *) buf;
    uint8 length;
    uint32 *pos_tmp;
    memset(buf, 0, sizeof(buf));

    set_company_id(pdu->company_id, IEEEID_BTSIG);
    pdu->pdu_id = AVRCP_REGISTER_NOTIFICATION;
    pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;
    pdu->params[0] = event;
    pdu->params_len = htons(AVRCP_REGISTER_NOTIFICATION_PARAM_LENGTH);

    length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);
    pos_tmp = (uint32*)(buf+8);
    *pos_tmp = htonl(1);
    avctp_send_vendordep_req(rcp_session->session, AVC_CTYPE_NOTIFY,
                             AVC_SUBUNIT_PANEL, buf, length);
}

#if 0
_ATTR_LWBT_CODE_
static void avrcp_list_player_attributes(struct avrcp_pcb *rcp_session)
{
    uint8 buf[AVRCP_HEADER_LENGTH];
    struct avrcp_header *pdu = (void *) buf;

    memset(buf, 0, sizeof(buf));

    set_company_id(pdu->company_id, IEEEID_BTSIG);
    pdu->pdu_id = AVRCP_LIST_PLAYER_ATTRIBUTES;
    pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

    avctp_send_vendordep_req(rcp_session->session, AVC_CTYPE_STATUS,
                             AVC_SUBUNIT_PANEL, buf, sizeof(buf));
}

_ATTR_LWBT_CODE_
static void avrcp_get_element_attributes(struct avrcp_pcb *rcp_session)
{
    uint8 buf[AVRCP_HEADER_LENGTH + 9];
    struct avrcp_header *pdu = (void *) buf;
    uint16 length;

    memset(buf, 0, sizeof(buf));

    set_company_id(pdu->company_id, IEEEID_BTSIG);
    pdu->pdu_id = AVRCP_GET_ELEMENT_ATTRIBUTES;
    pdu->params_len = htons(9);
    pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

    length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

    avctp_send_vendordep_req(rcp_session->session, AVC_CTYPE_STATUS,
                             AVC_SUBUNIT_PANEL, buf, length);
}

_ATTR_LWBT_CODE_
static void avrcp_get_play_status(struct avrcp_pcb *rcp_session)
{
    uint8 buf[AVRCP_HEADER_LENGTH];
    struct avrcp_header *pdu = (void *) buf;

    memset(buf, 0, sizeof(buf));

    set_company_id(pdu->company_id, IEEEID_BTSIG);
    pdu->pdu_id = AVRCP_GET_PLAY_STATUS;
    pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

    avctp_send_vendordep_req(rcp_session->session, AVC_CTYPE_STATUS,
                             AVC_SUBUNIT_PANEL, buf, sizeof(buf));
}


_ATTR_LWBT_CODE_
static bool avrcp_get_play_status_rsp(struct avrcp_pcb *rcp_session,uint8 code,uint8 *operands)
{
    uint32 duration;
    uint32 position;
    uint8 status;
    struct avrcp_header *pdu;

    pdu = (void *)operands;

    if(code == AVC_CTYPE_REJECTED || ntohs(pdu->params_len) != 9)
        return FALSE;

    memcpy(&duration,pdu->params,sizeof(uint32));
    duration = ntohl(duration);
    //2 TODO  to use 'duration'
    //...

    memcpy(&position,pdu->params + 4,sizeof(uint32));
    position = ntohl(position);
    //2 TODO  to use 'position'
    //...

    memcpy(&status,pdu->params + 8,sizeof(uint32));
    status = ntohl(status);
    //2 TODO  to use 'status'
    //...

    return TRUE;
}


_ATTR_LWBT_CODE_
static bool avrcp_get_capabilities_resp(uint8 code, uint8 subunit,
                                        uint8 *operands, size_t operand_count,
                                        void *user_data)
{
    struct avrcp_pcb *session = user_data;
    struct avrcp_header *pdu = (void *) operands;
    uint16 events = 0;
    uint8 count;

    if (pdu->params[0] != CAP_EVENTS_SUPPORTED)
        return FALSE;

    count = pdu->params[1];

    for (; count > 0; count--)
    {
        uint8 event = pdu->params[1 + count];

        events |= (1 << event);

        switch (event)
        {
            case AVRCP_EVENT_STATUS_CHANGED:
            case AVRCP_EVENT_TRACK_CHANGED:
            case AVRCP_EVENT_SETTINGS_CHANGED:
            case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
                avrcp_register_notification(session, event);
                break;
        }
    }

    if (!(events & (1 << AVRCP_EVENT_SETTINGS_CHANGED)))
        avrcp_list_player_attributes(session);

    if (!(events & (1 << AVRCP_EVENT_STATUS_CHANGED)))
        avrcp_get_play_status(session);

    if (!(events & (1 << AVRCP_EVENT_STATUS_CHANGED)))
        avrcp_get_element_attributes(session);

    return FALSE;
}

_ATTR_LWBT_CODE_
static void avrcp_get_capabilities(struct avrcp_pcb *rcp_session)
{
    uint8 buf[AVRCP_HEADER_LENGTH + AVRCP_GET_CAPABILITIES_PARAM_LENGTH];
    struct avrcp_header *pdu = (void *) buf;
    uint8 length;

    memset(buf, 0, sizeof(buf));

    set_company_id(pdu->company_id, IEEEID_BTSIG);
    pdu->pdu_id = AVRCP_GET_CAPABILITIES;
    pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;
    pdu->params[0] = CAP_EVENTS_SUPPORTED;
    pdu->params_len = htons(AVRCP_GET_CAPABILITIES_PARAM_LENGTH);

    length = AVRCP_HEADER_LENGTH + ntohs(pdu->params_len);

    avctp_send_vendordep_req(rcp_session->session, AVC_CTYPE_STATUS,
                             AVC_SUBUNIT_PANEL, buf, length);
}
#endif

#ifdef _AVRCP_VERSION_1_4_
_ATTR_LWBT_CODE_
int avrcp_set_absolute_volume(uint8 volume)
{
    uint8 *params;
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];

    if (avrcp_volume_changed_flag == FALSE)
    {
        //printf("avctp_set_absolute_volume=%d\n", volume);
        avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
        avrcp_hdr_tmp->company_id[0] = 0x00;
        avrcp_hdr_tmp->company_id[1] = 0x19;
        avrcp_hdr_tmp->company_id[2] = 0x58;
        avrcp_hdr_tmp->rsvd = 0;
        avrcp_hdr_tmp->packet_type = AVRCP_PACKET_TYPE_SINGLE;
        avrcp_hdr_tmp->pdu_id = AVRCP_VENDOR_DEP_SET_ABSOLUTEVOLUME;
        avrcp_hdr_tmp->params_len = htons(1);

        params = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
        params[0] = volume;

        //avctp_send_req(avctp_pcbs, AVC_CTYPE_CONTROL, AVC_SUBUNIT_PANEL, AVC_OP_VENDORDEP, send_buf, 1+AVRCP_HEADER_LENGTH);
        avctp_send_vendordep_req(avctp_pcbs, AVC_CTYPE_CONTROL, AVC_SUBUNIT_PANEL, send_buf, 1+AVRCP_HEADER_LENGTH);
    }
    return 0;
}
#endif

_ATTR_LWBT_CODE_
void avrcp_player_status_changed(uint8 player_status)
{
    if (player_status_changed_notify == TRUE)
    {
        uint8 *params;
        struct avrcp_header *avrcp_hdr_tmp;
        uint8 send_buf[1024];

        //printf("avctp_player_status_changed, player status=0x%x\n", player_status);

        avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
        avrcp_hdr_tmp->company_id[0] = 0x00;
        avrcp_hdr_tmp->company_id[1] = 0x19;
        avrcp_hdr_tmp->company_id[2] = 0x58;
        avrcp_hdr_tmp->rsvd = 0;
        avrcp_hdr_tmp->packet_type = AVRCP_PACKET_TYPE_SINGLE;
        avrcp_hdr_tmp->pdu_id = AVRCP_VENDOR_DEP_REG_NOTIFICATION;
        avrcp_hdr_tmp->params_len = htons(2);

        params = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
        params[0] = AVRCP_EVENT_STATUS_CHANGED; // event id
        params[1] = player_status;  // player status
        //avctp_send_rsp(avctp_pcbs, AVC_CTYPE_CHANGED, AVC_SUBUNIT_PANEL, AVC_OP_VENDORDEP, send_buf, 2+AVRCP_HEADER_LENGTH);
        avctp_pcbs->resp_tid = player_status_tid;
        avctp_send_vendordep_rsp(avctp_pcbs, AVC_CTYPE_CHANGED, AVC_SUBUNIT_PANEL, send_buf, 2+AVRCP_HEADER_LENGTH);
    }
}

_ATTR_LWBT_CODE_
void avrcp_track_changed()
{
    if (track_changed_notify == TRUE)
    {
        uint8 *params;
        struct avrcp_header *avrcp_hdr_tmp;
        uint8 send_buf[1024];
        int i;

        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("avctp_track_changed\n"));

        avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
        avrcp_hdr_tmp->company_id[0] = 0x00;
        avrcp_hdr_tmp->company_id[1] = 0x19;
        avrcp_hdr_tmp->company_id[2] = 0x58;
        avrcp_hdr_tmp->rsvd = 0;
        avrcp_hdr_tmp->packet_type = AVRCP_PACKET_TYPE_SINGLE;
        avrcp_hdr_tmp->pdu_id = AVRCP_VENDOR_DEP_REG_NOTIFICATION;
        avrcp_hdr_tmp->params_len = htons(9);

        params = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
        params[0] = AVRCP_EVENT_TRACK_CHANGED; // event id
        // track id
        for (i = 0; i < 8; ++i)
            params[i+1] = 0x00;
        avctp_pcbs->resp_tid = track_changed_tid;
        avctp_send_vendordep_rsp(avctp_pcbs, AVC_CTYPE_CHANGED, AVC_SUBUNIT_PANEL, send_buf, 9+AVRCP_HEADER_LENGTH);
    }
}

_ATTR_LWBT_CODE_
void avarp_cg_batterystatus_changed(uint8 battery_status)
{
    if (avrcp_batterystatus_notify == TRUE)
    {
        uint8 *params;
        struct avrcp_header *avrcp_hdr_tmp;
        uint8 send_buf[1024];
        int i;

        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("avctp_track_changed\n"));

        avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
        avrcp_hdr_tmp->company_id[0] = 0x00;
        avrcp_hdr_tmp->company_id[1] = 0x19;
        avrcp_hdr_tmp->company_id[2] = 0x58;
        avrcp_hdr_tmp->rsvd = 0;
        avrcp_hdr_tmp->packet_type = AVRCP_PACKET_TYPE_SINGLE;
        avrcp_hdr_tmp->pdu_id = AVRCP_VENDOR_DEP_REG_NOTIFICATION;
        avrcp_hdr_tmp->params_len = htons(2);

        params = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
        params[0] = AVRCP_EVENT_BATTERY_STATUS_CHANGED; // event id
        params[1] = battery_status;
        avctp_pcbs->resp_tid = batterystatus_tid;
        avctp_send_vendordep_rsp(avctp_pcbs, AVC_CTYPE_CHANGED, AVC_SUBUNIT_PANEL, send_buf, 2+AVRCP_HEADER_LENGTH);
    }
}

_ATTR_LWBT_CODE_
void avrcp_playback_pos_changed(uint32 pos)
{
    if (avrcp_playback_pos_notify == TRUE)
    {
        uint8 *params;
        struct avrcp_header *avrcp_hdr_tmp;
        uint8 send_buf[1024];
        int i;
        uint32 *pos_tmp;
        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("avrcp_playback_pos_changed\n"));
        //printf("avrcp_playback_pos_notify =%d\n ", avrcp_playback_pos_notify);
        avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
        avrcp_hdr_tmp->company_id[0] = 0x00;
        avrcp_hdr_tmp->company_id[1] = 0x19;
        avrcp_hdr_tmp->company_id[2] = 0x58;
        avrcp_hdr_tmp->rsvd = 0;
        avrcp_hdr_tmp->packet_type = AVRCP_PACKET_TYPE_SINGLE;
        avrcp_hdr_tmp->pdu_id = AVRCP_VENDOR_DEP_REG_NOTIFICATION;
        avrcp_hdr_tmp->params_len = htons(5);

        params = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
        params[0] = AVRCP_EVENT_POSITION_CHANGED; // event id
        pos_tmp = (uint32*)(params+1);
        *pos_tmp = htonl(pos);
        avctp_pcbs->resp_tid = playback_pos_tid;
        avctp_send_vendordep_rsp(avctp_pcbs, AVC_CTYPE_CHANGED, AVC_SUBUNIT_PANEL, send_buf, 5+AVRCP_HEADER_LENGTH);
    }
}

_ATTR_LWBT_CODE_
void avrcp_app_setting_changed(void)
{
    if (avrcp_app_setting_notify == TRUE)
    {
        uint8 *params;
        struct avrcp_header *avrcp_hdr_tmp;
        uint8 send_buf[1024];
        uint8 attr_num = 0;
        uint8 ids[10] = {0};
        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("avrcp_app_setting_changed\n"));
        //printf("avrcp_app_setting_changed =%d\n ", avrcp_app_setting_changed);
        avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
        avrcp_hdr_tmp->company_id[0] = 0x00;
        avrcp_hdr_tmp->company_id[1] = 0x19;
        avrcp_hdr_tmp->company_id[2] = 0x58;
        avrcp_hdr_tmp->rsvd = 0;
        avrcp_hdr_tmp->packet_type = AVRCP_PACKET_TYPE_SINGLE;
        avrcp_hdr_tmp->pdu_id = AVRCP_VENDOR_DEP_REG_NOTIFICATION;

        params = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
        params[0] = AVRCP_EVENT_SETTINGS_CHANGED;
        GetListPlayerAppSettingAttributes(&attr_num, ids);
        GetCurrentPlayerApplicationSettingValue(attr_num, ids,params+2);
        params[1] = attr_num;
        avrcp_hdr_tmp->params_len = htons(2+attr_num*2);
        avctp_pcbs->resp_tid = app_setting_tid;
        avctp_send_vendordep_rsp(avctp_pcbs, AVC_CTYPE_CHANGED, AVC_SUBUNIT_PANEL, send_buf, 2+attr_num*2+AVRCP_HEADER_LENGTH);
    }
}



_ATTR_LWBT_CODE_
int avrcp_get_element_attritutes_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    uint8 number_att, i;
    uint32 *attributes_id;
    uint16 index = 0;
    uint8 *send_buf_arry;
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint32 attributes_all[7] = {1,2,3,4,5,6,7};
    number_att = *(operands + 8);
    attributes_id = (uint32 *)(&operands[8+1]); // Identifier+NumAttributes

    if (number_att == 0)
    {
        int j;
        number_att = 7;
        for(j=0;j<7;j++)
        {
            attributes_all[j] = ntohl(attributes_all[j]);
        }

        attributes_id = attributes_all;
    }
    //DEBUG("number_att=%d\n", number_att);
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);

    send_buf_arry = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
    send_buf_arry[0] = number_att; // Number of attributes
    index += 1;

    for (i = 0; i < number_att; ++i)
    {
        struct avrtp_attribute_entry attribute;
        memset(&attribute, 0, sizeof(struct avrtp_attribute_entry));
        attribute.character_set = htons(0x006a); // utf-8
        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_,("attributes_id[%d]=0x%02x\n", i, ntohl(attributes_id[i])));
        switch(ntohl(attributes_id[i]))
        {
            case AVRCP_MEDIA_ATTRIBUTE_TITLE:
                #if 0
                //avctp_audio_get_element_attributes(&attribute, &index, send_buf_arry, AVRCP_MEDIA_ATTRIBUTE_TITLE);
                if (FALSE == ThreadCheck(pMainThread, &MusicThread))
                {
                    attribute.att_value_len = 0;
                    attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_TITLE);

                    memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                    index += ATTRIBUTE_ENTRY_HEAD_LEN;
                }
                else if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    uint16 title_utf8_len = 0;
                    uint8 att_file_name_utf8[MAX_FILENAME_LEN*6] = {0};
                    uint32 unicodeLen = 0;
                    uint16 *unicodeData;
                    unicodeData = (uint16 *)&gAudioId3Info.id3_title[0];
                    //printf("unicodeData = %x\n",unicodeData);
                    unicodeLen = get_unicode_len(unicodeData, MAX_FILENAME_LEN);
                    if(unicodeLen == 0)
                    {
                        unicodeLen = get_unicode_len(MusicLongFileName, MAX_FILENAME_LEN);
                        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_,("music file name unicodeLen=%d", unicodeLen));
                        title_utf8_len = UnicodeToUTF_8(MusicLongFileName, unicodeLen, att_file_name_utf8, 1);
                    }
                    else
                    {
                        title_utf8_len = UnicodeToUTF_8(unicodeData, unicodeLen, att_file_name_utf8, 1);
                    }
                    //printf("title_utf8_len = %x\n",title_utf8_len);


                    if (title_utf8_len > 0)
                    {
                        attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_TITLE);
                        attribute.att_value_len = htons(title_utf8_len);
                        attribute.att_value = att_file_name_utf8;

                        memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                        index += ATTRIBUTE_ENTRY_HEAD_LEN;
                        memcpy(send_buf_arry + index, att_file_name_utf8, title_utf8_len);
                        index += title_utf8_len;
                    }
                }
                #endif
                {
                uint8 att_utf8[1024] = {0};
                int attr_len = 0;
                audio_get_media_attribute_title(att_utf8,1024,&attr_len);
                attribute.att_value_len = htons(attr_len);;
                attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_TITLE);
                memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                index += ATTRIBUTE_ENTRY_HEAD_LEN;
                if(attr_len > 0)
                {
                   memcpy(send_buf_arry + index, att_utf8, attr_len);
                }
                index += attr_len;

                }
                break;
            case AVRCP_MEDIA_ATTRIBUTE_ARTIST:
                #if 0
                //avctp_audio_get_element_attributes(&attribute, &index, send_buf_arry, AVRCP_MEDIA_ATTRIBUTE_ARTIST);
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    uint8 str_artist_unknown[8] = "unknown";
                    uint8 str_artist[256];
                    int32 title_utf8_len = 0;
                    int32 unicodeLen;
                    uint16 *unicodeData;
                    unicodeData = AudioGetId3Singer();
                    unicodeLen = get_unicode_len(unicodeData, MAX_FILENAME_LEN);
                    memset(str_artist, 0 ,256);
                    title_utf8_len = UnicodeToUTF_8(unicodeData, unicodeLen, str_artist, 256);
                    if(title_utf8_len > 0)
                    {
                        attribute.att_value_len = htons(title_utf8_len);
                        attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ARTIST);
                        attribute.att_value = str_artist;

                        memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                        index += ATTRIBUTE_ENTRY_HEAD_LEN;
                        memcpy(send_buf_arry + index, str_artist, title_utf8_len);
                        index += title_utf8_len;
                    }
                    else
                    {
                        attribute.att_value_len = htons(8);
                        attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ARTIST);
                        attribute.att_value = str_artist_unknown;

                        memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                        index += ATTRIBUTE_ENTRY_HEAD_LEN;
                        memcpy(send_buf_arry + index, str_artist_unknown, 8);
                        index += 8;
                    }
                }
                else
                {
                    attribute.att_value_len = 0;
                    attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ARTIST);

                    memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                    index += ATTRIBUTE_ENTRY_HEAD_LEN;
                }
                #endif
                {
                uint8 att_utf8[1024];
                int attr_len = 0;
                audio_get_media_attribute_artist(att_utf8,1024,&attr_len);
                attribute.att_value_len = htons(attr_len);;
                attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ARTIST);
                memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                index += ATTRIBUTE_ENTRY_HEAD_LEN;
                if(attr_len > 0)
                {
                   memcpy(send_buf_arry + index, att_utf8, attr_len);
                }
                index += attr_len;
                }
                break;
            case AVRCP_MEDIA_ATTRIBUTE_ALBUM:
                #if 0
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    uint8 str_artist_unknown[8] = "unknown";
                    uint8 str_artist[256];
                    int32 title_utf8_len = 0;
                    int32 unicodeLen;
                    uint16 *unicodeData;
                    unicodeData = (uint16 *)&gAudioId3Info.id3_album[0];
                    unicodeLen = get_unicode_len(unicodeData, MAX_FILENAME_LEN);
                    memset(str_artist, 0 ,256);
                    title_utf8_len = UnicodeToUTF_8(unicodeData, unicodeLen, str_artist, 256);
                    if(title_utf8_len > 0)
                    {
                        attribute.att_value_len = htons(title_utf8_len);
                        attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ALBUM);
                        attribute.att_value = str_artist;

                        memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                        index += ATTRIBUTE_ENTRY_HEAD_LEN;
                        memcpy(send_buf_arry + index, str_artist, title_utf8_len);
                        index += title_utf8_len;
                    }
                    else
                    {
                        attribute.att_value_len = htons(8);
                        attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ALBUM);
                        attribute.att_value = str_artist_unknown;

                        memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                        index += ATTRIBUTE_ENTRY_HEAD_LEN;
                        memcpy(send_buf_arry + index, str_artist_unknown, 8);
                        index += 8;
                    }
                }
                else
                {
                    attribute.att_value_len = 0;
                    attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ALBUM);

                    memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                    index += ATTRIBUTE_ENTRY_HEAD_LEN;
                }
                #endif
                {
                uint8 att_utf8[1024] = {0};
                int attr_len = 0;
                audio_get_media_attribute_album(att_utf8,1024,&attr_len);
                attribute.att_value_len = htons(attr_len);;
                attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_ALBUM);
                memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                index += ATTRIBUTE_ENTRY_HEAD_LEN;
                if(attr_len > 0)
                {
                   memcpy(send_buf_arry + index, att_utf8, attr_len);
                }
                index += attr_len;
                }
                break;

            case AVRCP_MEDIA_ATTRIBUTE_GENRE:
                #if 0
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    uint8 str_artist_unknown[8] = "unknown";
                    uint8 str_artist[256];
                    int32 title_utf8_len = 0;
                    int32 unicodeLen;
                    uint16 *unicodeData;
                    unicodeData = (uint16 *)&gAudioId3Info.id3_genre[0];
                    unicodeLen = get_unicode_len(unicodeData, MAX_FILENAME_LEN);
                    memset(str_artist, 0 ,256);
                    title_utf8_len = UnicodeToUTF_8(unicodeData, unicodeLen, str_artist, 256);
                    if(title_utf8_len > 0)
                    {
                        attribute.att_value_len = htons(title_utf8_len);
                        attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_GENRE);
                        attribute.att_value = str_artist;

                        memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                        index += ATTRIBUTE_ENTRY_HEAD_LEN;
                        memcpy(send_buf_arry + index, str_artist, title_utf8_len);
                        index += title_utf8_len;
                    }
                    else
                    {
                        attribute.att_value_len = htons(8);
                        attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_GENRE);
                        attribute.att_value = str_artist_unknown;

                        memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                        index += ATTRIBUTE_ENTRY_HEAD_LEN;
                        memcpy(send_buf_arry + index, str_artist_unknown, 8);
                        index += 8;
                    }
                }
                else
                {
                    attribute.att_value_len = 0;
                    attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_GENRE);

                    memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                    index += ATTRIBUTE_ENTRY_HEAD_LEN;
                }
                #endif
                {
                uint8 att_utf8[1024];
                int attr_len = 0;
                audio_get_media_attribute_genre(att_utf8,1024,&attr_len);
                attribute.att_value_len = htons(attr_len);;
                attribute.att_id = htonl(AVRCP_MEDIA_ATTRIBUTE_GENRE);
                memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                index += ATTRIBUTE_ENTRY_HEAD_LEN;
                if(attr_len > 0)
                {
                   memcpy(send_buf_arry + index, att_utf8, attr_len);
                }
                index += attr_len;
                }
                break;

            case AVRCP_MEDIA_ATTRIBUTE_TRACK:
                //DEBUG("Not implemente element!\n");
                //avctp_vendordep_response(session, AVC_CTYPE_REJECTED, send_pbuf, index+AVRCP_HEADER_LENGTH);
                //return 0;
                //if (0)
                {
                    uint8 buf[32];
                    int outputlen = 0;
                    attribute.att_id = attributes_id[i];
                    AudioGetTrack(buf, 16, &outputlen);
                    attribute.att_value_len = htons(outputlen);
                    memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                    index += ATTRIBUTE_ENTRY_HEAD_LEN;
                    memcpy(&send_buf_arry[index],buf,outputlen);
                    index += outputlen;
                }

                break;
            case AVRCP_MEDIA_ATTRIBUTE_N_TRACKS:
                {
                    uint8 buf[32];
                    int outputlen = 0;
                    attribute.att_id = attributes_id[i];
                    AudioGetN_Track(buf, 16, &outputlen);
                    attribute.att_value_len = htons(outputlen);
                    memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                    index += ATTRIBUTE_ENTRY_HEAD_LEN;
                    memcpy(&send_buf_arry[index],buf,outputlen);
                    index += outputlen;
                }
                break;

           case AVRCP_MEDIA_ATTRIBUTE_DURATION:
               {
                   uint8 buf[32];
                   int outputlen = 0;
                   attribute.att_id = attributes_id[i];
                   AudioGetPlayingTime(buf, 16, &outputlen);
                   attribute.att_value_len = htons(outputlen);
                   memcpy(send_buf_arry + index, &attribute, ATTRIBUTE_ENTRY_HEAD_LEN);
                   index += ATTRIBUTE_ENTRY_HEAD_LEN;
                   memcpy(&send_buf_arry[index],buf,outputlen);
                   index += outputlen;
               }

                break;

            default:
                return 0;
                break;
        }
    }
    avrcp_hdr_tmp->params_len = htons(index);
    if(index > AVRCP_FRAGMENTTATION_LEN)
    {
        avrcp_hdr_tmp->packet_type = AVRCP_PKT_START;
        memcpy(&metadata_info.avrcp_hdr,avrcp_hdr_tmp,AVRCP_HEADER_LENGTH);
        metadata_info.leftcnt = index-AVRCP_FRAGMENTTATION_LEN;
        index = AVRCP_FRAGMENTTATION_LEN;
        //metadata_info.leftcnt = AVRCP_FRAGMENTTATION_LEN;
        memcpy(metadata_info.buf, send_buf+AVRCP_HEADER_LENGTH+AVRCP_FRAGMENTTATION_LEN, metadata_info.leftcnt);
        //avrcp_hdr_tmp->params_len = htons(index);
    }
    avctp_vendordep_response(session, code, send_buf, index+AVRCP_HEADER_LENGTH);
    return 0;
}

_ATTR_LWBT_CODE_
int avrcp_get_capability_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_capability *capability;
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    int i;

    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);

    capability = (struct avrcp_capability *)operands;

    switch(capability->capability_id)
    {
        case AVRCP_CAPABILITY_COMPANYID:
            {
                char *p;
                p = (char *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
                p[0] = AVRCP_CAPABILITY_COMPANYID;
                p[1] = 0x01;
                set_company_id(p+2, IEEEID_BTSIG);
                avrcp_hdr_tmp->params_len = htons(5);
                avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH + 5);
            }
            break;
        case AVRCP_CAPABILITY_EVENTID_SUPPORTED:
            {
                struct avrcp_capability *event_cap;
                uint8 supported_cap[] = {AVRCP_EVENT_POSITION_CHANGED, AVRCP_EVENT_TRACK_CHANGED,
                    AVRCP_EVENT_SETTINGS_CHANGED, AVRCP_EVENT_STATUS_CHANGED,
                    AVRCP_EVENT_BATTERY_STATUS_CHANGED};

                event_cap = (struct avrcp_capability *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
                event_cap->capability_count = sizeof(supported_cap);
                event_cap->capability_id = AVRCP_CAPABILITY_EVENTID_SUPPORTED;
                for (i = 0; i < sizeof(supported_cap); ++i)
                    event_cap->event_id[i] = supported_cap[i];
                avrcp_hdr_tmp->params_len = htons(2 + sizeof(supported_cap)); // avrcp header + capablity head+event supported
                avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH + 2 + sizeof(supported_cap));
            }
            break;

        default:
            avrcp_send_error_code(avctp_pcbs,avrcp_hdr , AVRCP_DRROR_CODE_INVALID_PARAM);
            break;
    }

#if 0 //def _AVRCP_VERSION_1_4_
    {
        uint8 *params;
        uint32 *tmp;
        avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
        memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
        avrcp_hdr_tmp->pdu_id = AVRCP_VENDOR_DEP_REG_NOTIFICATION;
        avrcp_hdr_tmp->params_len = htons(5);

        params = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
        params[0] = AVRCP_EVENT_VOLUME_CHANGED;
        tmp = (uint32*)(params+1);
        *tmp = htonl(1);
        avctp_send_vendordep_req(avctp_pcbs, AVC_CTYPE_NOTIFY, AVC_SUBUNIT_PANEL, send_buf, 5+AVRCP_HEADER_LENGTH);
    }
#endif

    return 0;
}

_ATTR_LWBT_CODE_
int avrcp_list_app_setting_attr_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint8 NumAttributes;
    uint8 AttributesIDs[6];
    uint8 *attr_rsp;
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);

    if(GetListPlayerAppSettingAttributes(&NumAttributes,AttributesIDs) != 0)
    {
        avrcp_hdr_tmp->params_len = htons(1);
        send_buf[AVRCP_HEADER_LENGTH] = AVRCP_DRROR_CODE_INVALID_PARAM;
        avctp_vendordep_response(session, AVC_CTYPE_REJECTED, send_buf, AVRCP_HEADER_LENGTH + 1);
        return 0;
    }
    attr_rsp =  send_buf+AVRCP_HEADER_LENGTH;
    attr_rsp[0] = NumAttributes;
    memcpy(attr_rsp+1, AttributesIDs, NumAttributes);
    avrcp_hdr_tmp->params_len = htons(1 + NumAttributes);
    avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH + 1 + NumAttributes);
    return 0;
}

_ATTR_LWBT_CODE_
int avrcp_list_app_setting_values_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint8 NumValue;
    uint8 AttributesID;
    uint8 *attr_rsp;
    uint8 AttributesValues[6];
    AttributesID = operands[0];
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    if(GetListPlayerApplicationSettingValues(AttributesID, &NumValue, AttributesValues) !=0 )
    {
        avrcp_hdr_tmp->params_len = htons(1);
        send_buf[AVRCP_HEADER_LENGTH] = AVRCP_DRROR_CODE_INVALID_PARAM;
        avctp_vendordep_response(session, AVC_CTYPE_REJECTED, send_buf, AVRCP_HEADER_LENGTH + 1);
        return 0;
    }

    attr_rsp =  send_buf+AVRCP_HEADER_LENGTH;
    attr_rsp[0] = NumValue;
    memcpy(attr_rsp+1, AttributesValues, NumValue);
    avrcp_hdr_tmp->params_len = htons(1 + NumValue);
    avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH + 1 + NumValue);
    return 0;

}


_ATTR_LWBT_CODE_
int avrcp_get_cur_app_setting_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint8 NumValue;
    uint8 NumAttributes;
    uint8 *attr_rsp;
    uint8 AttributesValues[6];
    uint8 AttValue = 0;
    NumAttributes = operands[0];
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    attr_rsp =  send_buf+AVRCP_HEADER_LENGTH;
    attr_rsp[0] = NumAttributes;
    if(GetCurrentPlayerApplicationSettingValue(NumAttributes,operands+1,attr_rsp+1) != 0)
    {
        avrcp_hdr_tmp->params_len = htons(1);
        send_buf[AVRCP_HEADER_LENGTH] = AVRCP_DRROR_CODE_INVALID_PARAM;
        avctp_vendordep_response(session, AVC_CTYPE_REJECTED, send_buf, AVRCP_HEADER_LENGTH + 1);
        return 0;
    }
    avrcp_hdr_tmp->params_len = htons(1+NumAttributes*2);
    avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH + 1+NumAttributes*2);
    return 0;

}

_ATTR_LWBT_CODE_
int avrcp_set_app_setting_value_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint8 NumValue;
    uint8 AttributesID;
    uint8 NumAttributes;
    uint8 *attr_rsp;
    uint8 AttributesValues[6];
    uint8 AttValue = 0;
    NumAttributes = operands[0];
    AttributesID = operands[1];
    AttValue = operands[2];
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    if(SetCurrentPlayerApplicationSettingValue(AttributesID,AttValue) != 0)
    {
        avrcp_hdr_tmp->params_len = htons(1);
        send_buf[AVRCP_HEADER_LENGTH] = AVRCP_DRROR_CODE_INVALID_PARAM;
        avctp_vendordep_response(session, AVC_CTYPE_REJECTED, send_buf, AVRCP_HEADER_LENGTH + 1);
        return 0;
    }
    avrcp_hdr_tmp->params_len = htons(0);
    avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH);
    return 0;

}

_ATTR_LWBT_CODE_
int avrcp_get_app_setting_attr_text_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_play_status *play_sta;
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
}

_ATTR_LWBT_CODE_
int avrcp_get_app_setting_value_text_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_play_status *play_sta;
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
}

_ATTR_LWBT_CODE_
int avrcp_inform_charset_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    avrcp_hdr_tmp->params_len = htons(0);
    avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH);
    return 0;

}

_ATTR_LWBT_CODE_
int avrcp_inform_battery_status_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint8 CtBatterystatus = 0;
    CtBatterystatus = operands[0];
    CtBatteryStatusChangeCallback(CtBatterystatus);
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    avrcp_hdr_tmp->params_len = htons(0);
    avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH);
    return 0;

}

_ATTR_LWBT_CODE_
int avrcp_requset_continution_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    if(metadata_info.leftcnt)
    {
        if(metadata_info.leftcnt > AVRCP_FRAGMENTTATION_LEN)
        {
            memcpy(avrcp_hdr_tmp, &metadata_info.avrcp_hdr, AVRCP_HEADER_LENGTH);
            memcpy(send_buf+AVRCP_HEADER_LENGTH,metadata_info.buf, AVRCP_FRAGMENTTATION_LEN);
            avrcp_hdr_tmp->packet_type = AVRCP_PKT_CONTINUE;
            avctp_vendordep_response(session, code, send_buf, AVRCP_FRAGMENTTATION_LEN+AVRCP_HEADER_LENGTH);
            metadata_info.leftcnt -= AVRCP_FRAGMENTTATION_LEN;
            memcpy(metadata_info.buf, metadata_info.buf+AVRCP_FRAGMENTTATION_LEN, metadata_info.leftcnt);
            return 0;
        }
        else
        {
            memcpy(avrcp_hdr_tmp, &metadata_info.avrcp_hdr, AVRCP_HEADER_LENGTH);
            memcpy(send_buf+AVRCP_HEADER_LENGTH,metadata_info.buf, metadata_info.leftcnt);
            avrcp_hdr_tmp->packet_type = AVRCP_PKT_END;
            avctp_vendordep_response(session, code, send_buf, metadata_info.leftcnt+AVRCP_HEADER_LENGTH);
            metadata_info.leftcnt = 0;
            return 0;
        }

    }
    else
    {

    }


}


_ATTR_LWBT_CODE_
int avrcp_abort_continuation_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    avrcp_hdr_tmp->params_len = htons(0);
    avctp_vendordep_response(session, code, send_buf, AVRCP_HEADER_LENGTH);
    return 0;

}




extern uint32 audio_get_player_status(void);

_ATTR_LWBT_CODE_
int avrcp_get_play_status_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    struct avrcp_play_status *play_sta;
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];

    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);

    play_sta = (struct avrcp_play_status *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);
    //play_sta->play_status = AVRCP_PLAY_STATUS_STOPPED;
    //play_sta->song_length = htonl(0);
    //play_sta->song_position = htonl(0);

    if (TRUE == ThreadCheck(pMainThread, &MusicThread))
    {
        play_sta->play_status = audio_get_player_status();
        play_sta->song_length = htonl(pAudioRegKey->TotalTime);
        play_sta->song_position = htonl(pAudioRegKey->CurrentTime);
    }
    else
    {
        play_sta->play_status = AVRCP_PLAY_STATUS_STOPPED;
        play_sta->song_length = htonl(0);
        play_sta->song_position = htonl(0);
    }

    avrcp_hdr_tmp->params_len = htons(sizeof(struct avrcp_play_status));
    avctp_vendordep_response(session, AVC_CTYPE_STABLE, send_buf, AVRCP_HEADER_LENGTH + sizeof(struct avrcp_play_status));

    return 0;
}

_ATTR_LWBT_CODE_
int avrcp_send_error_code(struct avctp_pcb *session,struct avrcp_header *avrcp_hdr , uint8 err_code)
{
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint8 NumAttributes;
    uint8 AttributesIDs[6];
    uint8 *attr_rsp;
    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    avrcp_hdr_tmp->params_len = htons(1);
    send_buf[AVRCP_HEADER_LENGTH] = err_code;
    avctp_vendordep_response(session, AVC_CTYPE_REJECTED, send_buf, AVRCP_HEADER_LENGTH + 1);
    return 0;

}


_ATTR_LWBT_CODE_
static int ct_press(struct avrcp_pcb *avrcp, uint8 op)
{
    int err;
    struct avctp_pcb *ctpcb;

    if (avrcp == NULL)
    {
        return -1;
    }

    ctpcb = avrcp->session;


    err = avctp_send_passthrough(ctpcb, op);
    if (err < 0)
        return err;

    return 0;
}



_ATTR_LWBT_CODE_
int ct_volumeup(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_VOLUME_UP);
}

_ATTR_LWBT_CODE_
int ct_volumedown(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_VOLUME_DOWN);
}



#ifdef _AVRCP_VERSION_1_4_
_ATTR_LWBT_CODE_
static void avrcp_volume_changed(struct avrcp_pcb *session, struct avrcp_header *pdu, uint8 volume)
{
    uint8 absolute_volume, mvolume;

    absolute_volume = volume & 0x7f;
    mvolume = absolute_volume;
    if (avrcp_play_volume != mvolume)
    {
        //printf("Event volume changed:volume=%d\n", mvolume);
        avrcp_play_volume = mvolume;
        if (avctp_notify_pcb.audio_volume_change != NULL)
        {
            avctp_notify_pcb.audio_volume_change(avrcp_play_volume);
        }
    }
}

#if 0
_ATTR_LWBT_CODE_
static void avrcp_track_changed(struct avrcp_pcb *session, struct avrcp_header *pdu)
{
    avrcp_get_element_attributes(session);
}
#endif



_ATTR_LWBT_CODE_
static void avrcp_handle_event(struct avrcp_pcb *rcp_session, uint8 code, uint8 *operands)
{
    struct avrcp_header *pdu = (void *) operands;
    uint8 event, *params;

    params = (uint8 *)((int)pdu + AVRCP_HEADER_LENGTH);

    if ((code != AVC_CTYPE_INTERIM && code != AVC_CTYPE_CHANGED) || pdu == NULL)
        return;

    event = params[0];

    if (code == AVC_CTYPE_CHANGED) {
        rcp_session->registered_events ^= (1 << event);
        avrcp_volume_manage = TRUE;

        avrcp_register_notification(rcp_session, event);
        return;
    }

    switch (event) {
        case AVRCP_EVENT_VOLUME_CHANGED:
            //LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_,("Event volume changed:volume=%d\n", params[1]));

            avrcp_volume_manage = TRUE;
            avrcp_volume_changed(rcp_session, pdu, params[1]);
            break;
    }

    rcp_session->registered_events |= (1 << event);
}
#endif

_ATTR_LWBT_CODE_
int avrcp_register_notification_rsp(struct avctp_pcb *session, uint8 code, uint8 *operands, struct avrcp_header *avrcp_hdr)
{
    uint8 event_id, play_status;
    uint32 *interval, play_position;
    struct avrcp_header *avrcp_hdr_tmp;
    uint8 send_buf[1024];
    uint8 *send_arry;
    int i;

    avrcp_hdr_tmp = (struct avrcp_header *)send_buf;
    memcpy(avrcp_hdr_tmp, avrcp_hdr, AVRCP_HEADER_LENGTH);
    send_arry = (uint8 *)((int)avrcp_hdr_tmp + AVRCP_HEADER_LENGTH);

    event_id = operands[0];
    interval = (uint32 *)(event_id + 1);


    LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("notification event id=0x%02x, interval=0x%04x\n", event_id, interval));
    switch(event_id)
    {
        case AVRCP_EVENT_POSITION_CHANGED:
            {
                uint32 *pos;
                LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("register avrcp evnet position changed\n"));
                avrcp_playback_pos_interval = ntohl(interval);
                send_arry[0] = AVRCP_EVENT_POSITION_CHANGED;
                pos = (uint32*)(send_arry + 1);
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    *pos = htonl(pAudioRegKey->CurrentTime);
                }
                else
                {
                    *pos = htonl(0xFFFFFFFF);
                }

                avrcp_hdr_tmp->params_len = htons(5);
                playback_pos_tid = session->resp_tid;
                avctp_vendordep_response(session, code, send_buf, 5 + AVRCP_HEADER_LENGTH);
                avrcp_playback_pos_notify = TRUE;


            }
            break;

        case AVRCP_EVENT_TRACK_CHANGED:
            LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("register avrcp evnet track change\n"));
            track_changed_notify = TRUE;
            send_arry[0] = AVRCP_EVENT_TRACK_CHANGED;
            /* Track id*/
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                for (i = 1; i < 9; ++i)
                    send_arry[i] = 0x00;
            }
            else
            {
                for (i = 1; i < 9; ++i)
                    send_arry[i] = 0xff;
            }
            avrcp_hdr_tmp->params_len = htons(9);
            track_changed_tid = session->resp_tid;
            avctp_vendordep_response(session, code, send_buf, 9 + AVRCP_HEADER_LENGTH);
            break;

        case AVRCP_EVENT_SETTINGS_CHANGED:
            {
                uint8 attr_num = 0;
                uint8 ids[10] = {0};
                send_arry[0] = AVRCP_EVENT_SETTINGS_CHANGED;
                LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("register avrcp evnet setting changed\n"));
                //printf("register avrcp evnet setting changed\n");
                avrcp_app_setting_notify = TRUE;
                app_setting_tid = session->resp_tid;
                GetListPlayerAppSettingAttributes(&attr_num, ids);
                GetCurrentPlayerApplicationSettingValue(attr_num, ids,send_arry+2);
                send_arry[1] = attr_num;
                avrcp_hdr_tmp->params_len = htons(2+attr_num*2);
                avctp_vendordep_response(session, code, send_buf, 2+attr_num*2+ AVRCP_HEADER_LENGTH);

            }
            break;

        case AVRCP_EVENT_STATUS_CHANGED:
            {
                LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("register avrcp evnet status changed\n"));
                player_status_changed_notify = TRUE;
                player_status_tid = session->resp_tid;
                send_arry[0] = AVRCP_EVENT_STATUS_CHANGED;
                send_arry[1] = audio_get_player_status();
                avrcp_hdr_tmp->params_len = htons(2);
                avctp_vendordep_response(session, code, send_buf, 2 + AVRCP_HEADER_LENGTH);
            }
            break;

        case AVRCP_EVENT_VOLUME_CHANGED:
            LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("register avrcp evnet vulume changed\n"));
            printf("register avrcp evnet vulume changed\n");
            //avrcp_volume_changed_flag = TRUE;

            break;
        case AVRCP_EVENT_BATTERY_STATUS_CHANGED:
            avrcp_batterystatus_notify = TRUE;
            batterystatus_tid = session->resp_tid;
            LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("register avrcp battery status changed\n"));
            send_arry[0] = AVRCP_EVENT_BATTERY_STATUS_CHANGED;
            send_arry[1] = GetCgBatteryStatus();
            avrcp_hdr_tmp->params_len = htons(2);
            avctp_vendordep_response(session, code, send_buf, 2 + AVRCP_HEADER_LENGTH);
            break;
        default:
            LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("register avrcp other event_id = %d\n",event_id));
            avrcp_hdr_tmp->params_len = htons(1);
            send_arry[0] = 0x01; // invalid parameter
            avctp_vendordep_response(session, AVC_CTYPE_REJECTED, send_buf, 1 + AVRCP_HEADER_LENGTH);
            break;
    }
    return 0;
}

_ATTR_LWBT_CODE_
int handle_vendordep_req(struct avctp_pcb *session,uint8 code,uint8 *operands)
{
    struct avrcp_header *avrcp_hdr;

    uint16 params_len;
    uint8 *params;
    int ret = 0, i;

    avrcp_hdr = (struct avrcp_header *)operands;
    params_len = ntohs(avrcp_hdr->params_len);
    params = (uint8 *)((int)avrcp_hdr + AVRCP_HEADER_LENGTH);

    if (avrcp_hdr->packet_type != AVRCP_PACKET_TYPE_SINGLE)
    {
        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_,("error: error packet type.\n"));
        return -1;
    }

    LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_, ("avrcp_hdr->pdu_id=%0x, params_len=%d, avrcp_hdr->packet_type=%d\n", avrcp_hdr->pdu_id, params_len, avrcp_hdr->packet_type));
    switch(avrcp_hdr->pdu_id)
    {
        case AVRCP_VENDOR_DEP_GET_CAPABILITIES:
            ret = avrcp_get_capability_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_LIST_PLAYER_APP_ATTR:
            ret = avrcp_list_app_setting_attr_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_LIST_PLAYER_APP_VALUES:
            ret = avrcp_list_app_setting_values_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_GET_CUR_PLAYER_APP_VALUE:
            ret = avrcp_get_cur_app_setting_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_SET_PLAYER_APP_VALUE:
            ret = avrcp_set_app_setting_value_rsp(session, AVC_CTYPE_ACCEPTED, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_GET_PLAYER_APP_ATTR_TEXT:
            ret = avrcp_get_app_setting_attr_text_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_GET_PLAYER_APP_VALUE_TEXT:
            ret = avrcp_get_app_setting_value_text_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_INFORM_DISPLAY_CHARSET:
            ret = avrcp_inform_charset_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_INFORM_BATTERY_STAT_OF_CT:
            ret = avrcp_inform_battery_status_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;

        case AVRCP_VENDOR_DEP_GET_ELEMENTATTRIBUTES:
            ret = avrcp_get_element_attritutes_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;

        case AVRCP_VENDOR_DEP_GET_PLAYSTATUS:
            ret = avrcp_get_play_status_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;

        case AVRCP_VENDOR_DEP_REG_NOTIFICATION:
            ret = avrcp_register_notification_rsp(session, AVC_CTYPE_INTERIM, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_REQUEST_CONTINUATION_RSP:
            ret = avrcp_requset_continution_rsp(session, AVC_CTYPE_STABLE, params, avrcp_hdr);
            break;
        case AVRCP_VENDOR_DEP_ABORT_CONTINUATION_RSP:
            ret = avrcp_abort_continuation_rsp(session, AVC_CTYPE_ACCEPTED, params, avrcp_hdr);
            break;

        case AVRCP_VENDOR_DEP_SET_ABSOLUTEVOLUME:
            break;

        default:
            avrcp_send_error_code(session,avrcp_hdr, AVRCP_DRROR_CODE_INVALID_COMMAND);
            break;
    }

    return ret;
}

#ifdef _AVRCP_VERSION_1_4_

void avrcp_set_absolutevolume_handle_resp(struct avrcp_pcb *rcp_session, uint8 code, uint8 *operands)
{
    struct avrcp_header *pdu = (void *) operands;
    uint8 volume, *params;

    params = (uint8 *)((int)pdu + AVRCP_HEADER_LENGTH);

    if (code != AVC_CTYPE_ACCEPTED)
        return;

    volume = params[0];

    blueToothSetVolumeRespCallback(volume);
}

_ATTR_LWBT_CODE_
int avrtp_vendordep_rsp(struct avctp_pcb *session,uint8 code,uint8 *operands, struct avc_header *avc_hdr)
{
    struct avrcp_header *avrcp_hdr;

    avrcp_hdr = (struct avrcp_header *)operands;

    if (avrcp_hdr->packet_type != AVRCP_PACKET_TYPE_SINGLE)
    {
        LWBT_DEBUGF(AVRCP_DEBUG, _DBG_INFO_,("error: error packet type.\n"));
        return -1;
    }

    switch(avrcp_hdr->pdu_id)
    {
        case AVRCP_VENDOR_DEP_REG_NOTIFICATION:
            avrcp_handle_event(avrcp_pcbs, code, operands);
            break;

        case AVRCP_VENDOR_DEP_SET_ABSOLUTEVOLUME:
            avrcp_set_absolutevolume_handle_resp(avrcp_pcbs, code, operands);
            break;
    }
    return 0;
}
#endif

_ATTR_LWBT_CODE_
void avrcp_init()
{
    avrcp_play_volume = -1;
    player_status_changed_notify = 0;
    track_changed_notify = 0;
    avrcp_app_setting_notify = 0;
    avrcp_batterystatus_notify = 0;
    avrcp_volume_manage = 0;
    avrcp_volume_changed_flag = 0;
    avrcp_playback_pos_notify = 0;
    avrcp_playback_pos_interval = 0;
    if(avrcp_pcbs != NULL)
    {
        #ifdef _AVRCP_VERSION_1_4_
        if( avrcp_pcbs && avrcp_pcbs->session && IsRegisterVolumeNotification())
        {
            avrcp_register_notification(avrcp_pcbs, AVRCP_EVENT_VOLUME_CHANGED);
        }
        #endif
        return;
    }
    avrcp_pcbs = avrcp_new();
    if( avrcp_pcbs == NULL )
        return;

    if( avctp_pcbs != NULL )
        avrcp_pcbs->session = avctp_pcbs;

    memset(&metadata_info, 0, sizeof(struct avrcp_metadata_info_save));


    #ifdef _AVRCP_VERSION_1_4_
    if( avrcp_pcbs && avrcp_pcbs->session && IsRegisterVolumeNotification())
    {
        avrcp_register_notification(avrcp_pcbs, AVRCP_EVENT_VOLUME_CHANGED);
    }
    #endif
}


_ATTR_LWBT_CODE_
void avrcp_deinit()
{
    avrcp_free(avrcp_pcbs);
    avrcp_pcbs= NULL;

    return;
}


/*
_ATTR_LWBT_CODE_
static int ct_press(struct avrcp_pcb *avrcp, uint8 op)
{
    int err;
    struct avctp_pcb *ctpcb;

    if (avrcp == NULL)
    {
        return -1;
    }

    ctpcb = avrcp->session;


    err = avctp_send_passthrough(ctpcb, op);
    if (err < 0)
        return err;

    return 0;
}

_ATTR_LWBT_CODE_
//static
int ct_play(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;
    return ct_press(rcpcb, AVC_PLAY);
}

_ATTR_LWBT_CODE_
//static
int ct_pause(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_PAUSE);
}

_ATTR_LWBT_CODE_
//static
int ct_stop(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_STOP);
}

_ATTR_LWBT_CODE_
//static
int ct_next(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_BACKWARD);
}

_ATTR_LWBT_CODE_
//static
int ct_previous(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_FORWARD);
}

_ATTR_LWBT_CODE_
//static
int ct_fast_forward(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_FAST_FORWARD);
}

_ATTR_LWBT_CODE_
//static
int ct_mute(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_MUTE);
}

_ATTR_LWBT_CODE_
//static
int ct_rewind(void)
{
    struct avrcp_pcb *rcpcb = avrcp_pcbs;

    return ct_press(rcpcb, AVC_REWIND);
}
*/
#endif
#endif
#endif
