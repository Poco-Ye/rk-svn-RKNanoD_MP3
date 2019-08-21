/*
 * Copyright (c) 2003 EISLAB, Lulea University of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwBT Bluetooth stack.
 *
 * Author: Conny Ohult <conny@sm.luth.se>
 *
 */

/*-----------------------------------------------------------------------------------*/
/* sdp.c
 *
 * Implementation of the service discovery protocol (SDP)
 */
/*-----------------------------------------------------------------------------------*/

#include "lwbt.h"

#include "SysInclude.h"

#include "sdp.h"
#include "lwbt_memp.h"
#include "lwbtopts.h"
#include "lwbtdebug.h"
#include "pbuf.h"
#include "inet.h"
#include "l2cap.h"

#ifdef _BLUETOOTH_
/* Next service record handle to be used */
_ATTR_LWBT_BSS_ uint32 rhdl_next;

/* Next transaction id to be used */
_ATTR_LWBT_BSS_ uint16 tid_next;

/* The SDP PCB lists */
_ATTR_LWBT_BSS_ struct sdp_pcb *sdp_pcbs;
_ATTR_LWBT_BSS_ struct sdp_pcb *sdp_tmp_pcb;
_ATTR_LWBT_BSS_ struct l2cap_pcb *sdp_l2cap;


/* List of all active service records in the SDP server */
_ATTR_LWBT_BSS_ struct sdp_record *sdp_server_records;
_ATTR_LWBT_BSS_ struct sdp_record *sdp_tmp_record; /* Only used for temp storage */
_ATTR_LWBT_BSS_ static uint16 sdp_err_rsp_code;
_ATTR_LWBT_BSS_ void (* sdp_event_handler)(uint8 event_type, uint8 *parameter,
                     uint16 param_size, err_t err);


int sdp_get_total_attrib_size(uint8 size, struct pbuf *p);
int de_get_len(uint8 *header);
/* The SDP protocol control block */
__packed struct sdp_user_req
{
    uint8 ssp[32];
    uint8 ssplen;
    uint8 attrids[32];
    uint8 attrlen;

    void (* user_attributes_searched)(void *arg, struct sdp_pcb *pcb, uint16 attribl_bc, struct pbuf *p);
};

_ATTR_LWBT_BSS_ struct sdp_user_req user_req;


/* The SDP protocol control block */
__packed struct sdp_continuation_State
{
    uint8 continuationEnable;
    uint8 continuationLen;
    uint32 recordMemAdrr;
    uint16 recordoffset;
    uint16 prevSize;
};

_ATTR_LWBT_BSS_ struct sdp_continuation_State continuation_State;
_ATTR_LWBT_BSS_ struct sdp_continuation_State recv_continuation_State;


#define SDP_SEND_EVENT(event_type,parameter, param_size, err)    \
if(sdp_event_handler != NULL)                                        \
{                                                                    \
    if(sdp_event_handler != NULL)                                    \
    {                                                                \
        sdp_event_handler(event_type, parameter, param_size, err);   \
    }                                                                \
}

_ATTR_LWBT_INIT_CODE_
void sdp_register_event_handler(void (*event_handler)(uint8 event_type,uint8 *packet, uint16 size,err_t err))
{
    sdp_event_handler = event_handler;
}

_ATTR_LWBT_CODE_
struct l2cap_pcb * sdp_get_auto_l2cappcb(void)
{
    //printf("sdp_get_auto_l2cappcb = 0x%x\n",sdp_l2cap);
    return sdp_l2cap;
}

_ATTR_LWBT_CODE_
struct l2cap_pcb * sdp_clear_auto_l2cappcb(struct l2cap_pcb *pcb)
{
    if(sdp_l2cap == pcb)
        sdp_l2cap = NULL;
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
err_t sdp_l2cap_connected_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    err_t ret = ERR_OK;
    struct sdp_pcb *sdppcb;

    LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("l2cap_disconnected_ind: L2CAP disconnected\n"));

    if(pcb->psm == SDP_PSM)
    {
        l2cap_recv(pcb, sdp_recv);
        sdp_l2cap = pcb;
        //     sdppcb = sdp_new(pcb);
        //     SDP_REG(&sdp_pcbs, pcb); /* Register request */

        l2cap_disconnect_ind(pcb, sdp_lp_disconnected);


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
void sdp_init(void)
{
    /* Clear globals */
    sdp_event_handler = NULL;
    sdp_server_records = NULL;
    sdp_tmp_record = NULL;
    sdp_pcbs = NULL;
    /* Inialize service record handles */
    rhdl_next = 0x0000FFFF;

    /* Initialize transaction ids */
    tid_next = 0x0000;
#if LWBT_LAP
    l2cap_connect_ind(NULL, SDP_PSM,sdp_l2cap_connected_ind);

#endif

    memset(&user_req, 0, sizeof(struct sdp_user_req));

}

_ATTR_LWBT_INIT_CODE_
void sdp_deinit(void)
{
    rhdl_next = 0;
    tid_next = 0;
    sdp_pcbs = NULL;
    sdp_tmp_pcb = NULL;
    sdp_server_records = NULL;
    sdp_tmp_record = NULL;
    sdp_event_handler = NULL;
}



/*-----------------------------------------------------------------------------------*/
/* Server API
 */
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_next_rhdl():
 *
 * Issues a service record handler.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
uint32 sdp_next_rhdl(void)
{
    ++rhdl_next;
    if(rhdl_next == 0)
    {
        rhdl_next = 0x0000FFFF;
    }
    return rhdl_next;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_record_new():
 *
 * Creates a new service record.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
struct sdp_record *sdp_record_new(uint8 *record_de_list, uint8 rlen)
{
    struct sdp_record *record;

    record = lwbt_memp_malloc(MEMP_SDP_RECORD);
    if(record != NULL)
    {
        record->hdl = sdp_next_rhdl();
        record->record_de_list = record_de_list;
        record->len = rlen;
        return record;
    }
    return NULL;
}
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void sdp_record_free(struct sdp_record *record)
{
    lwbt_memp_free(MEMP_SDP_RECORD, record);
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_register_service():
 *
 * Add a record to the list of records in the service record database, making it
 * available to clients.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
err_t sdp_register_service(struct sdp_record *record)
{
    if(record == NULL)
    {
        return ERR_ARG;
    }
    SDP_RECORD_REG(&sdp_server_records, record);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_unregister_service():
 *
 * Remove a record from the list of records in the service record database, making it
 * unavailable to clients.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void sdp_unregister_service(struct sdp_record *record)
{
    SDP_RECORD_RMV(&sdp_server_records, record);
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_next_transid():
 *
 * Issues a transaction identifier that helps matching a request with the reply.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
uint16 sdp_next_transid(void)
{
    ++tid_next;
    return tid_next;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_pattern_search():
 *
 * Check if the given service search pattern matches the record.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
uint8 sdp_pattern_search(struct sdp_record *record, uint8 size, struct pbuf *p)
{
    uint8 i, j;
    uint8 *payload = (uint8 *)p->payload;

    sdp_err_rsp_code = 0;
    for(i = 0; i < size; ++i)
    {
        if(SDP_DE_TYPE(payload[i]) == SDP_DE_TYPE_UUID)
        {
            switch(SDP_DE_SIZE(payload[i]))
            {
                case SDP_DE_SIZE_16:
                    SDP_SEND_EVENT(SDP_EVENT_SEARCHED_UUID,(payload + i + 1),2,0);
                    for(j = 0; j < record->len; ++j)
                    {
                        if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_UUID)
                        {
                            if(*((uint16 *)(payload + i + 1)) == *((uint16 *)(record->record_de_list + j + 1)))
                            {
                                return 1; /* Found a matching UUID in record */
                            }
                            ++j;
                        }
                    }
                    i += 2;
                    break;
                case SDP_DE_SIZE_32:
                {
                    uint16 uuid_1;
                    uuid_1 = *((uint16 *)(payload + i + 3));
                    SDP_SEND_EVENT(SDP_EVENT_SEARCHED_UUID,(payload + i + 3),2,0);
                    for(j = 0; j < record->len; ++j)
                    {
                        if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_UUID)
                        {
                            if(uuid_1  == *((uint16 *)(record->record_de_list + j + 1)))
                            {
                                return 1; /* Found a matching UUID in record */
                            }
                            ++j;
                        }
                    }

                    i += 4;
                    break;
                }
                case SDP_DE_SIZE_128:
                {
                    uint16 uuid_1;
                    uuid_1 = *((uint16 *)(payload + i + 3));
                    SDP_SEND_EVENT(SDP_EVENT_SEARCHED_UUID,(payload + i + 3),2,0);
                    for(j = 0; j < record->len; ++j)
                    {
                        if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_UUID)
                        {
                            if(uuid_1  == *((uint16 *)(record->record_de_list + j + 1)))
                            {
                                return 1; /* Found a matching UUID in record */
                            }
                            ++j;
                        }
                    }


                }
                i+= 16;
                break;
                default:
                break;
            }
        }
        else
        {
            sdp_err_rsp_code = SDP_ERR_RSP_CODE_REQUEST_SYNTAX;
            break;

        }
    }
    return 0;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_attribute_search():
 *
 * Searches a record for attributes and add them to a given packet buffer.
 */
/*-----------------------------------------------------------------------------------*/
#if LWBT_LAP
_ATTR_LWBT_CODE_
struct pbuf *sdp_attribute_search(uint16 max_attribl_bc, struct pbuf *p, struct sdp_record *record)
{
    struct pbuf *q = NULL;
    struct pbuf *r;
    struct pbuf *s = NULL;
    uint8 *payload = (uint8 *)p->payload;
    uint16 size;
    uint8 i = 0, j;
    uint8 joffset = 0;
    uint16 attr_id = 0, attr_id2 = 0;
    uint16 arrt_len = 0; //wp
    uint16 attribl_bc = 0; /* Byte count of the sevice attributes */
    uint32 hdl = htonl(record->hdl);

    uint32 attriblesize = 0;

    sdp_err_rsp_code = SDP_ERR_RSP_CODE_RESERVED;
    attriblesize = sdp_get_single_attribute_size(p, record);
    if(sdp_err_rsp_code)
        return NULL;

    if(attriblesize >= (0xff+3))
    {
        attriblesize -=3;
        if(recv_continuation_State.continuationEnable == 0)
        {
            max_attribl_bc =  max_attribl_bc > 3 ?(max_attribl_bc-3):0;
        }
    }
    else
    {
        attriblesize -=2;

        if(recv_continuation_State.continuationEnable == 0)
        {
            max_attribl_bc =  max_attribl_bc > 2 ?(max_attribl_bc-2):0;
        }
    }

    if(max_attribl_bc == 0)
    {
        return NULL;
    }
    if(recv_continuation_State.continuationEnable)
    {
        joffset = (uint8)recv_continuation_State.recordoffset;
    }

    if(SDP_DE_TYPE(payload[0]) == SDP_DE_TYPE_DES  &&
       SDP_DE_SIZE(payload[0]) == SDP_DE_SIZE_N1)
    {
        /* Get size of attribute ID list */
        size = payload[1]; //TODO: correct to assume only one size byte in remote request? probably

        while(i < size)
        {
            /* Check if this is an attribute ID or a range of attribute IDs */
            if(payload[2+i] == (SDP_DE_TYPE_UINT  | SDP_DE_SIZE_16))
            {
                attr_id = *((uint16 *)(payload+3+i));
                attr_id2 = attr_id; /* For the range to cover this attribute ID only */
                i += 3;
            }
            else if(payload[2+i] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_32))
            {
                attr_id = *((uint16 *)(payload+3+i));
                attr_id2 = *((uint16 *)(payload+5+i));
                i += 5;
            }
            else
            {
                /* ERROR: Invalid req syntax */
                //TODO

                sdp_err_rsp_code = SDP_ERR_RSP_CODE_REQUEST_SYNTAX;
                return NULL;
            }

            //for(j = 0; j < record->len; ++j)
            for(j = joffset; j < record->len;)
            {
                if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_DES)
                {
                    if(record->record_de_list[j + 2] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_16))
                    {
                        if(*((uint16 *)(record->record_de_list + j + 3)) >= attr_id &&
                           *((uint16 *)(record->record_de_list + j + 3)) <= attr_id2)
                        {
                            if(attribl_bc +  record->record_de_list[j + 1] + 2 > max_attribl_bc)
                            {
                                /* Abort attribute search since attribute list byte count must not
                                   exceed max attribute byte count in req */
                                continuation_State.continuationEnable = 1;
                                continuation_State.continuationLen = 8;
                                continuation_State.recordMemAdrr = (uint32)record;
                                continuation_State.recordoffset  = (uint32)j;

                                //printf("continuation_State recordMemAdrr = %d, j = %d\r\n", (uint32)record,j );
                                break;
                            }
                            /* Allocate a pbuf for the service attribute */
                            arrt_len = record->record_de_list[j + 1]+2;

#if 0
                            r = pbuf_alloc(PBUF_RAW, record->record_de_list[j + 1], PBUF_RAM);
                            memcpy((uint8 *)r->payload, record->record_de_list + j + 2, r->len);

#endif
                            if(s == NULL)
                            {
                                s = pbuf_alloc(PBUF_RAW, record->record_de_list[j + 1], PBUF_RAM);
                                memcpy((uint8 *)s->payload, record->record_de_list + j + 2, s->len);


                                if(*((uint16 *)(record->record_de_list + j + 3)) == 0)
                                {
                                    memcpy(((uint8 *)s->payload) + 4, &hdl, 4);
                                }


                            }
                            else
                            {
                                uint offset = 0;
                                offset = s->len;

                                pbuf_header(s, -offset);

                                s->len = s->tot_len = record->record_de_list[j + 1];

                                memcpy((uint8 *)s->payload, record->record_de_list + j + 2, s->len);


                                if(*((uint16 *)(record->record_de_list + j + 3)) == 0)
                                {
                                    memcpy(((uint8 *)s->payload) + 4, &hdl, 4);
                                }

                                pbuf_header(s, offset);
                            }


                            //attribl_bc += r->len;


                            attribl_bc += arrt_len-2;
                            /* If request included a service record handle attribute id, add the correct id to the
                            response */

                            if(attr_id2 == attr_id)
                            {
                                break;
                            }

                            j += arrt_len;

                            continue;
                        }
                    }
                }

                ++j;

            } /* for */
        } /* while */
    }
    else if(SDP_DE_TYPE(payload[0]) == SDP_DE_TYPE_DES&&
        SDP_DE_SIZE(payload[0]) == SDP_DE_SIZE_N2)
    {

        size = (payload[1]<<8 & 0xff00)+payload[2];; //TODO: correct to assume only one size byte in remote request? probably
        while(i < size)
        {
            /* Check if this is an attribute ID or a range of attribute IDs */
            if(payload[3+i] == (SDP_DE_TYPE_UINT  | SDP_DE_SIZE_16))
            {
                attr_id = *((uint16 *)(payload+4+i));
                attr_id2 = attr_id; /* For the range to cover this attribute ID only */
                i += 3;
            }
            else if(payload[3+i] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_32))
            {
                attr_id = *((uint16 *)(payload+4+i));
                attr_id2 = *((uint16 *)(payload+6+i));
                i += 5;
            }
            else
            {
                /* ERROR: Invalid req syntax */
                //TODO

                sdp_err_rsp_code = SDP_ERR_RSP_CODE_REQUEST_SYNTAX;
                return NULL;
            }

            //for(j = 0; j < record->len; ++j)
            for(j = joffset; j < record->len;)
            {
                if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_DES)
                {
                    if(record->record_de_list[j + 2] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_16))
                    {
                        if(*((uint16 *)(record->record_de_list + j + 3)) >= attr_id &&
                           *((uint16 *)(record->record_de_list + j + 3)) <= attr_id2)
                        {
                            if(attribl_bc +  record->record_de_list[j + 1] + 2 > max_attribl_bc)
                            {
                                /* Abort attribute search since attribute list byte count must not
                                   exceed max attribute byte count in req */
                                continuation_State.continuationEnable = 1;
                                continuation_State.continuationLen = 8;
                                continuation_State.recordMemAdrr = (uint32)record;
                                continuation_State.recordoffset  = (uint32)j;

                                //printf("continuation_State recordMemAdrr = %d, j = %d\r\n", (uint32)record,j );
                                break;
                            }
                            /* Allocate a pbuf for the service attribute */
                            arrt_len = record->record_de_list[j + 1]+2;

#if 0
                            r = pbuf_alloc(PBUF_RAW, record->record_de_list[j + 1], PBUF_RAM);
                            memcpy((uint8 *)r->payload, record->record_de_list + j + 2, r->len);

#endif
                            if(s == NULL)
                            {
                                s = pbuf_alloc(PBUF_RAW, record->record_de_list[j + 1], PBUF_RAM);
                                memcpy((uint8 *)s->payload, record->record_de_list + j + 2, s->len);


                                if(*((uint16 *)(record->record_de_list + j + 3)) == 0)
                                {
                                    memcpy(((uint8 *)s->payload) + 4, &hdl, 4);
                                }


                            }
                            else
                            {
                                uint offset = 0;
                                offset = s->len;

                                pbuf_header(s, -offset);

                                s->len = s->tot_len = record->record_de_list[j + 1];

                                memcpy((uint8 *)s->payload, record->record_de_list + j + 2, s->len);


                                if(*((uint16 *)(record->record_de_list + j + 3)) == 0)
                                {
                                    memcpy(((uint8 *)s->payload) + 4, &hdl, 4);
                                }

                                pbuf_header(s, offset);
                            }


                            //attribl_bc += r->len;


                            attribl_bc += arrt_len-2;
                            /* If request included a service record handle attribute id, add the correct id to the
                            response */

                            if(attr_id2 == attr_id)
                            {
                                break;
                            }

                            j += arrt_len;

                            continue;
                        }
                    }
                }

                ++j;

            } /* for */
        } /* while */

    }
    else
    {
        /* ERROR: Invalid req syntax */
        LWBT_DEBUGF(SDP_DEBUG, _DBG_SERIOUS_, ("sdp_attribute_search: Invalid req syntax"));
        sdp_err_rsp_code = SDP_ERR_RSP_CODE_REQUEST_SYNTAX;
        return NULL;
    }
#if 1
    /* Return service attribute list */
    if(recv_continuation_State.continuationEnable == 0)
    {
        if(s != NULL)
        {

            if(attriblesize > 0xFF)
            {
                uint payloadlen;
                payloadlen = attriblesize;
                pbuf_header(s,3);
                q = s;
                ((uint8 *)q->payload)[0] = SDP_DE_TYPE_DES | SDP_DE_SIZE_N2;
                ((uint8 *)q->payload)[1] = (payloadlen & 0xFF00) >> 8;
                ((uint8 *)q->payload)[2] = payloadlen& 0x00FF;
            }
            else
            {
                uint payloadlen;
                payloadlen = attriblesize;
                pbuf_header(s,2);
                q = s;

                ((uint8 *)q->payload)[0] = SDP_DE_TYPE_DES | SDP_DE_SIZE_N1;
                ((uint8 *)q->payload)[1] = payloadlen;
            }

        }
        else
        {
            q = s;
        }
    }
    else
    {
        q = s;
    }
#else
    q = s;
#endif
    return q;
}
#endif /* LWBT_LAP */
/*-----------------------------------------------------------------------------------*/
/*
 * SDP CLIENT API.
 */
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_new():
 *
 * Creates a new SDP protocol control block but doesn't place it on
 * any of the SDP PCB lists.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
struct sdp_pcb *sdp_new(struct l2cap_pcb *l2cappcb)
{
    struct sdp_pcb *pcb;

    pcb = lwbt_memp_malloc(MEMP_SDP_PCB);
    if(pcb != NULL)
    {
        memset(pcb, 0, sizeof(struct sdp_pcb));
        pcb->l2cappcb = l2cappcb;
        return pcb;
    }
    return NULL;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_free():
 *
 * Free the SDP protocol control block.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
void sdp_free(struct sdp_pcb *pcb)
{
    lwbt_memp_free(MEMP_SDP_PCB, pcb);
    pcb = NULL;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_reset_all():
 *
 * Free all SDP protocol control blocks and registered records.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void sdp_reset_all(void)
{
    struct sdp_pcb *pcb, *tpcb;
    struct sdp_record *record, *trecord;

    for(pcb = sdp_pcbs; pcb != NULL;)
    {
        tpcb = pcb->next;
        SDP_RMV(&sdp_pcbs, pcb);
        sdp_free(pcb);
        pcb = tpcb;
    }

    for(record = sdp_server_records; record != NULL;)
    {
        trecord = record->next;
        sdp_unregister_service(record);
        sdp_record_free(record);
        record = trecord;
    }

    sdp_init();
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_arg():
 *
 * Used to specify the argument that should be passed callback functions.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void sdp_arg(struct sdp_pcb *pcb, void *arg)
{
    pcb->callback_arg = arg;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_lp_disconnected():
 *
 * Called by the application to indicate that the lower protocol disconnected.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void sdp_lp_disconnected(void *arg, struct l2cap_pcb *l2cappcb, err_t err)
{
    struct sdp_pcb *pcb, *tpcb;
    LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_lp_disconnected: L2CAP disconnected\n"));
    pcb = sdp_pcbs;
    while(pcb != NULL)
    {
        tpcb = pcb->next;
        if(bd_addr_cmp(&(l2cappcb->remote_bdaddr), &(pcb->l2cappcb->remote_bdaddr)))
        {
            /* We do not need to notify upper layer, free PCB */
            SDP_RMV(&sdp_pcbs, pcb);
            sdp_free(pcb);
        }
        pcb = tpcb;
    }
    if(sdp_l2cap == l2cappcb)
        sdp_l2cap = NULL;
    l2cap_close(l2cappcb);
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_service_search_req():
 *
 * Sends a request to a SDP server to locate service records that match the service
 * search pattern.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t sdp_service_search_req(struct sdp_pcb *pcb, uint8 *ssp, uint8 ssplen, uint16 max_src,
                             void (* service_searched)(void *arg, struct sdp_pcb *pcb, uint16 tot_src,
                                     uint16 curr_src, uint32 *rhdls))
{
    struct pbuf *p;
    struct sdp_hdr *sdphdr;
    err_t ret;
    /* Update PCB */
    pcb->tid = sdp_next_transid(); /* Set transaction id */

    /* Allocate packet for PDU hdr + service search pattern + max service record count +
     continuation state */
    p = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+ssplen+2+1, PBUF_RAM);
    sdphdr = p->payload;
    /* Add PDU header to packet */
    sdphdr->pdu = SDP_SS_PDU;
    sdphdr->id = htons(pcb->tid);
    sdphdr->len = htons(ssplen + 3); /* Seq descr + ServiceSearchPattern + MaxServiceRecCount + ContState */

    /* Add service search pattern to packet */
    memcpy(((uint8 *)p->payload) + SDP_PDUHDR_LEN, ssp, ssplen);

    /* Add maximum service record count to packet */
    *((uint16 *)(((uint8 *)p->payload) + ssplen + SDP_PDUHDR_LEN)) = htons(max_src);

    ((uint8 *)p->payload)[SDP_PDUHDR_LEN+ssplen+2] = 0; /* No continuation */

    /* Update PCB */
    pcb->service_searched = service_searched; /* Set callback */
    SDP_REG(&sdp_pcbs, pcb); /* Register request */

    ret = l2ca_datawrite(pcb->l2cappcb, p);

    pbuf_free(p);
    return ret;
}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_service_attrib_req():
 *
 * Retrieves specified attribute values from a specific service record.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t sdp_service_attrib_req(struct sdp_pcb *pcb, uint32 srhdl, uint16 max_abc, uint8 *attrids, uint8 attrlen,
                             void (* attributes_recv)(void *arg, struct sdp_pcb *pcb, uint16 attribl_bc, struct pbuf *p))
{
    struct sdp_hdr *sdphdr;
    uint8 *payload;
    struct pbuf *p;
    err_t ret;
    /* Allocate packet for PDU hdr + service rec hdl + max attribute byte count +
     attribute id data element sequense lenght  + continuation state */
    p = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN + attrlen + 7, PBUF_RAM);

    /* Update PCB */
    pcb->tid = sdp_next_transid(); /* Set transaction id */

    /* Add PDU header to packet */
    sdphdr = p->payload;
    sdphdr->pdu = SDP_SA_PDU;
    sdphdr->id = htons(pcb->tid);
    sdphdr->len = htons((attrlen + 7)); /* Service rec hdl + Max attrib B count + Seq descr + Attribute sequence + ContState */

    payload = p->payload;

    /* Add service record handle to packet */
    *((uint32 *)(payload + SDP_PDUHDR_LEN)) = htonl(srhdl);

    /* Add maximum attribute count to packet */
    *((uint16 *)(payload + SDP_PDUHDR_LEN + 4)) = htons(max_abc);

    /* Add attribute id data element sequence to packet */
    memcpy(payload + SDP_PDUHDR_LEN + 6, attrids, attrlen);

    payload[SDP_PDUHDR_LEN + 6 + attrlen] = 0x00; /* No continuation */

    /* Update PCB */
    pcb->attributes_recv = attributes_recv; /* Set callback */
    SDP_REG(&sdp_pcbs, pcb); /* Register request */

    ret = l2ca_datawrite(pcb->l2cappcb, p);

    pbuf_free(p);
    return ret;

}

err_t sdp_error_response(struct l2cap_pcb *pcb,  struct sdp_hdr *reqhdr, uint16 error_code)
{
    struct sdp_hdr *rsphdr;
    struct pbuf *q;
    int ret;
    q  = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+2, PBUF_RAM);
    if(q == NULL)
        return ERR_MEM;

    rsphdr = q->payload;
    rsphdr->pdu = SDP_ERR_PDU;
    rsphdr->id = reqhdr->id;

    *((uint16 *)(((uint8 *)q->payload) + SDP_PDUHDR_LEN)) = htons(error_code);
    rsphdr->len = htons(q->tot_len - SDP_PDUHDR_LEN);
    ret = l2ca_datawrite(pcb, q);
    pbuf_free(q);
    return ret;

}

/*-----------------------------------------------------------------------------------*/
/*
 * sdp_service_search_attrib_req():
 *
 * Combines the capabilities of the SDP_ServiceSearchRequest and the
 * SDP_ServiceAttributeRequest into a single request. Contains both a service search
 * pattern and a list of attributes to be retrieved from service records that match
 * the service search pattern.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t sdp_service_search_attrib_req(struct sdp_pcb *pcb, uint16 max_abc, uint8 *ssp, uint8 ssplen, uint8 *attrids,
                                    uint8 attrlen, void (* attributes_searched)(void *arg, struct sdp_pcb *pcb,
                                            uint16 attribl_bc,
                                            struct pbuf *p), uint8 *buf, uint8 len)
{
    struct sdp_hdr *sdphdr;

    struct pbuf *p;
    uint8 *payload;
    uint16 pbuf_bc;
    err_t ret;
    /* Allocate packet for PDU hdr + service search pattern + max attribute byte count +
     attribute id list + continuation state */
    p = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+ssplen+2+attrlen+1+len, PBUF_RAM);
    if(p == NULL)
        return ERR_MEM;

    /* Update PCB */
    pcb->tid = sdp_next_transid(); /* Set transaction id */

    /* Add PDU header to packet */
    sdphdr = p->payload;
    sdphdr->pdu = SDP_SSA_PDU;
    sdphdr->id = htons(pcb->tid);
    sdphdr->len = htons(ssplen + 2 + attrlen + 1+len);

    pbuf_bc = SDP_PDUHDR_LEN;
    payload = (uint8 *)p->payload;

    /* Add service search pattern to packet */
    memcpy(((uint8 *)p->payload) + SDP_PDUHDR_LEN, ssp, ssplen);

    /* Add maximum attribute count to packet */
    *((uint16 *)(payload + SDP_PDUHDR_LEN + ssplen)) = htons(max_abc);

    /* Add attribute id data element sequence to packet */
    memcpy(payload + SDP_PDUHDR_LEN + ssplen + 2, attrids, attrlen);

    if(len == 0)
    {
    	payload[SDP_PDUHDR_LEN + ssplen + 2 + attrlen] = 0x00; /* No continuation */
    }
	else
	{
		payload[SDP_PDUHDR_LEN + ssplen + 2 + attrlen] = len;
    	memcpy(payload+SDP_PDUHDR_LEN +2+attrlen+ssplen+1, buf, len);
	}

    pcb->attributes_searched = attributes_searched; /* Set callback */
    SDP_REG(&sdp_pcbs, pcb); /* Register request */

    ret = l2ca_datawrite(pcb->l2cappcb, p);

    pbuf_free(p);
    return ret;

}
/*-----------------------------------------------------------------------------------*/
/*
 * SDP SERVER API.
 */
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_service_search_rsp():
 *
 * The SDP server sends a list of service record handles for service records that
 * match the service search pattern given in the request.
 */
/*-----------------------------------------------------------------------------------*/
#if LWBT_LAP
_ATTR_LWBT_CODE_
err_t sdp_service_search_rsp(struct l2cap_pcb *pcb, struct pbuf *p, struct sdp_hdr *reqhdr)
{
    struct sdp_record *record;
    struct sdp_hdr *rsphdr;

    struct pbuf *q; /* response packet */
    struct pbuf *r; /* tmp buffer */

    uint16 max_src = 0;
    uint16 curr_src = 0;
    uint16 tot_src = 0;
    int16 offset = 0;

    uint16 size = 0;

    err_t ret;

    if(SDP_DE_TYPE(((uint8 *)p->payload)[0]) == SDP_DE_TYPE_DES &&
       SDP_DE_SIZE(((uint8 *)p->payload)[0]) ==  SDP_DE_SIZE_N1)
    {
        /* Size of the search pattern must be in the next byte since only
           12 UUIDs are allowed in one pattern */
        size = ((uint8 *)p->payload)[1];

        /* Get maximum service record count that follows the service search pattern */
        max_src = ntohs(*((uint16 *)(((uint8 *)p->payload)+(2+size))));

        pbuf_header(p, -2);
    }
    else if(SDP_DE_TYPE(((uint8 *)p->payload)[0])== SDP_DE_TYPE_DES &&
        SDP_DE_SIZE(((uint8 *)p->payload)[0]) ==  SDP_DE_SIZE_N2)
    {
        size= (((uint8 *)p->payload)[1]<<8 & 0xff00)+((uint8 *)p->payload)[2];
        //printf("size = %d\n", size);
        //dumpMemoryCharA(((uint8 *)p->payload)+(3+size), p->len-3-size);
        max_src = ntohs(*((uint16 *)(((uint8 *)p->payload)+(3+size))));
        //printf("max_src = %d\n", max_src);
        pbuf_header(p, -3);
    }
    else
    {
        //TODO: INVALID SYNTAX ERROR
        sdp_error_response(pcb,reqhdr,SDP_ERR_RSP_CODE_REQUEST_SYNTAX);
        return ERR_VAL;
    }

    /* Allocate header + Total service rec count + Current service rec count  */
    q  = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+4, PBUF_RAM);
    if(q == NULL)
    {
        return ERR_MEM;
    }

    rsphdr = q->payload;
    rsphdr->pdu = SDP_SSR_PDU;
    rsphdr->id = reqhdr->id;

    for(record = sdp_server_records; record != NULL; record = record->next)
    {
        /* Check if service search pattern matches record */
        if(sdp_pattern_search(record, size, p))
        {
            if(max_src > 0)
            {
                /* Add service record handle to packet */
                //r = pbuf_alloc(PBUF_RAW, 4, PBUF_RAM);
                offset = q->len;

                pbuf_header(q,-offset);

                q->tot_len = q->len = 4;
                *((uint32 *)q->payload) = htonl(record->hdl);

                //pbuf_copy(q,r);
                //pbuf_free(r);

                pbuf_header(q, offset);
                --max_src;
                ++curr_src;
            }
            ++tot_src;
        }
        else
        {
            if(sdp_err_rsp_code)
            {
                sdp_error_response(pcb, reqhdr, sdp_err_rsp_code);
                sdp_err_rsp_code = 0;
                pbuf_free(q);
                return ERR_VAL;
            }
        }
    }

    /* Add continuation state to packet */


    //r = pbuf_alloc(PBUF_BT, 1, PBUF_RAM);
    //((uint8 *)r->payload)[0] = 0x00;
    offset = q->len;

    pbuf_header(q,-offset);
    q->tot_len = q->len = 1;
    ((uint8 *)q->payload)[0] = 0x00;

    pbuf_header(q, offset);

    //pbuf_copy(q,r);
    //pbuf_free(r);

    /* Add paramenter length to header */
    rsphdr->len = htons(q->tot_len - SDP_PDUHDR_LEN);

    /* Add total service record count to packet */
    *((uint16 *)(((uint8 *)q->payload) + SDP_PDUHDR_LEN)) = htons(tot_src);

    /* Add current service record count to packet */
    *((uint16 *)(((uint8 *)q->payload) + SDP_PDUHDR_LEN + 2)) = htons(curr_src);


    ret = l2ca_datawrite(pcb, q);
    pbuf_free(q);
    return ret;
}
#endif /* LWBT_LAP */
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_service_attrib_rsp():
 *
 * Sends a response that contains a list of attributes (both attribute ID and
 * attribute value) from the requested service record.
 */
/*-----------------------------------------------------------------------------------*/
#if LWBT_LAP
_ATTR_LWBT_CODE_
err_t sdp_service_attrib_rsp(struct l2cap_pcb *pcb, struct pbuf *p, struct sdp_hdr *reqhdr)
{
    struct sdp_record *record;
    struct sdp_hdr *rsphdr;

    struct pbuf *q;
    struct pbuf *r;
    struct pbuf *t;
    uint16 max_attribl_bc = 0; /* Maximum attribute list byte count */

    err_t ret;

    uint16 AttributeIDListSize = 0;
    uint8 continuationSize = 0;
    uint16 totalSize = 0;
    uint8 * Pcontinuation;
    continuation_State.continuationEnable = 0;
    continuation_State.continuationLen = 0;
    recv_continuation_State.continuationEnable = 0;
    recv_continuation_State.prevSize = 0;
    continuation_State.continuationEnable = 0;

    /* Find record */
    for(record = sdp_server_records; record != NULL; record = record->next)
    {
        if(record->hdl == ntohl(*((uint32 *)p->payload)))
        {
            break;
        }
    }
    if(record == NULL)
    {
        return sdp_error_response(pcb, reqhdr, SDP_ERR_RSP_CODE_INVALID_HANDLE);

    }
    if(record != NULL)
    {
        /* Get maximum attribute byte count */
        max_attribl_bc = ntohs(((uint16 *)p->payload)[2]);


        AttributeIDListSize = de_get_len(((uint8 *)p->payload)+6);
        continuationSize = ((uint8 *)p->payload)[6+AttributeIDListSize];
        if(continuationSize)
        {
            Pcontinuation = ((uint8 *)p->payload)+6+AttributeIDListSize+1;
            recv_continuation_State.continuationEnable = 1;
            recv_continuation_State.continuationLen = continuationSize;
            recv_continuation_State.recordMemAdrr = *((uint32 *)Pcontinuation);
            recv_continuation_State.recordoffset  = *((uint16 *)(Pcontinuation+4));
            //printf("continuationSize = %d, recordMemAdrr = %d,recordoffset = %d, prevSize = %d\r\n",continuationSize,recv_continuation_State.recordMemAdrr, recv_continuation_State.recordoffset, recv_continuation_State.prevSize );
        }

        /* Allocate rsp packet header + Attribute list count */

        /* Search for attributes and add them to a pbuf */
        pbuf_header(p, -6);
        r = sdp_attribute_search(max_attribl_bc, p, record);

        /* Return service attribute list */


        if(r != NULL)
        {
            /* Add attribute list byte count length to header */
            uint sdp_payload_len;
            sdp_payload_len = htons(r->tot_len);
            pbuf_header(r, SDP_PDUHDR_LEN+2);

            q = r;
            rsphdr = q->payload;
            rsphdr->pdu = SDP_SAR_PDU;
            rsphdr->id = reqhdr->id;
            *((uint16 *)(((uint8 *)q->payload) + SDP_PDUHDR_LEN)) = sdp_payload_len;

        }
        else
        {
            // *((uint16 *)(((uint8 *)q->payload) + SDP_PDUHDR_LEN)) = 0;
            if(sdp_err_rsp_code)
            {
                sdp_error_response(pcb,reqhdr,sdp_err_rsp_code);
                sdp_err_rsp_code = 0;
                return ERR_VAL;
            }

            q  = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+2+2, PBUF_RAM);
            if(q == NULL)
                return ERR_MEM;

            rsphdr = q->payload;
            rsphdr->pdu = SDP_SAR_PDU;
            rsphdr->id = reqhdr->id;

            *((uint16 *)(((uint8 *)q->payload) + SDP_PDUHDR_LEN)) = htons(2);
            ((uint8 *)q->payload)[7] = 0x35;
            ((uint8 *)q->payload)[8] = 0;
        }

//    {
//        uint offset = 0;
//        offset = q->len;
//        pbuf_header(q, -offset);
//        q->len = q->tot_len = 1;
//        ((uint8 *)q->payload)[0] = 0x00;
//        pbuf_header(q, offset);
//    }


        /* Add continuation state to packet */
        if((r = pbuf_alloc(PBUF_RAW, continuation_State.continuationLen+1, PBUF_RAM)) == NULL)
        {
            pbuf_free(q);
            return ERR_MEM;
        }
        else
        {
            ((uint8 *)r->payload)[0] = continuation_State.continuationLen; //TODO: Is this correct?
            if(continuation_State.continuationLen)
            {
                *((uint32 *)(((uint8 *)r->payload)+1)) = continuation_State.recordMemAdrr;
                *((uint16 *)(((uint8 *)r->payload)+5)) = continuation_State.recordoffset;
                *((uint16 *)(((uint8 *)r->payload)+7)) = 0;
            }

            pbuf_copy(q, r);
            pbuf_free(r);
        }



        /* Add paramenter length to header */
        rsphdr->len = htons(q->tot_len - SDP_PDUHDR_LEN);



        ret = l2ca_datawrite(pcb, q);
        pbuf_free(q);

        return ret;
    }
    //TODO: ERROR NO SERVICE RECORD MATCHING HANDLE FOUND
    return ERR_OK;
}
#endif /* LWBT_LAP */



/******************************************************************************
 * sdp_attribute_search_2 -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * 01a, 24jun2014,  written
 * --------------------
 ******************************************************************************/
 _ATTR_LWBT_CODE_
int sdp_get_single_attribute_size(struct pbuf *p, struct sdp_record *record)
{
    struct pbuf *q = NULL;
    struct pbuf *r;
    struct pbuf *s = NULL;
    uint8 *payload = (uint8 *)p->payload;
    uint16 size;
    uint8 i = 0, j;
    uint8 joffset = 0;
    uint16 attr_id = 0, attr_id2 = 0;
    uint16 arrt_len = 0; //wp
    uint16 attribl_bc = 0; /* Byte count of the sevice attributes */
    uint32 hdl = htonl(record->hdl);

    sdp_err_rsp_code = SDP_ERR_RSP_CODE_RESERVED;
    if(SDP_DE_TYPE(payload[0]) == SDP_DE_TYPE_DES  &&
       SDP_DE_SIZE(payload[0]) == SDP_DE_SIZE_N1)
    {
        /* Get size of attribute ID list */
        size = payload[1]; //TODO: correct to assume only one size byte in remote request? probably

        while(i < size)
        {
            /* Check if this is an attribute ID or a range of attribute IDs */
            if(payload[2+i] == (SDP_DE_TYPE_UINT  | SDP_DE_SIZE_16))
            {
                attr_id = *((uint16 *)(payload+3+i));
                attr_id2 = attr_id; /* For the range to cover this attribute ID only */
                i += 3;
            }
            else if(payload[2+i] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_32))
            {
                attr_id = *((uint16 *)(payload+3+i));
                attr_id2 = *((uint16 *)(payload+5+i));
                i += 5;
            }
            else
            {
                /* ERROR: Invalid req syntax */
                //TODO
                sdp_err_rsp_code = SDP_ERR_RSP_CODE_REQUEST_SYNTAX;
                return 0;
            }

            //for(j = 0; j < record->len; ++j)
            for(j = joffset; j < record->len;)
            {
                if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_DES)
                {
                    if(record->record_de_list[j + 2] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_16))
                    {
                        if(*((uint16 *)(record->record_de_list + j + 3)) >= attr_id &&
                           *((uint16 *)(record->record_de_list + j + 3)) <= attr_id2)
                        {
                            //if(attribl_bc +  record->record_de_list[j + 1] + 2 > max_attribl_bc)
                            //{
                            //break;
                            //}
                            /* Allocate a pbuf for the service attribute */
                            arrt_len = record->record_de_list[j + 1]+2;


                            attribl_bc += arrt_len-2;
                            /* If request included a service record handle attribute id, add the correct id to the
                            response */

                            if(attr_id2 == attr_id)
                            {
                                break;
                            }

                            j += arrt_len;

                            continue;
                        }
                    }
                }

                ++j;

            } /* for */
        } /* while */
    }
    else if(SDP_DE_TYPE(payload[0]) == SDP_DE_TYPE_DES&&
      SDP_DE_SIZE(payload[0]) == SDP_DE_SIZE_N2)
    {
         size = (payload[1]<<8 & 0xff00)+payload[2];

         while(i < size)
         {
            /* Check if this is an attribute ID or a range of attribute IDs */
            if(payload[3+i] == (SDP_DE_TYPE_UINT  | SDP_DE_SIZE_16))
            {
                attr_id = *((uint16 *)(payload+4+i));
                attr_id2 = attr_id; /* For the range to cover this attribute ID only */
                i += 3;
            }
            else if(payload[3+i] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_32))
            {
                attr_id = *((uint16 *)(payload+4+i));
                attr_id2 = *((uint16 *)(payload+5+i));
                i += 5;
            }
            else
            {
                /* ERROR: Invalid req syntax */
                //TODO
                sdp_err_rsp_code = SDP_ERR_RSP_CODE_REQUEST_SYNTAX;
                return 0;
            }

             //for(j = 0; j < record->len; ++j)
            for(j = joffset; j < record->len;)
            {
                if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_DES)
                {
                    if(record->record_de_list[j + 2] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_16))
                    {
                        if(*((uint16 *)(record->record_de_list + j + 3)) >= attr_id &&
                           *((uint16 *)(record->record_de_list + j + 3)) <= attr_id2)
                        {
                            //if(attribl_bc +  record->record_de_list[j + 1] + 2 > max_attribl_bc)
                            //{
                            //break;
                            //}
                            /* Allocate a pbuf for the service attribute */
                            arrt_len = record->record_de_list[j + 1]+2;


                            attribl_bc += arrt_len-2;
                            /* If request included a service record handle attribute id, add the correct id to the
                            response */

                            if(attr_id2 == attr_id)
                            {
                                break;
                            }

                            j += arrt_len;

                            continue;
                        }
                    }
                }

                ++j;
             }//for
         }//while
    }
    else
    {
        /* ERROR: Invalid req syntax */
        LWBT_DEBUGF(SDP_DEBUG, _DBG_SERIOUS_, ("sdp_attribute_search: Invalid req syntax"));
        sdp_err_rsp_code = SDP_ERR_RSP_CODE_REQUEST_SYNTAX;
        return 0;
    }

    /* Return service attribute list */


    if(attribl_bc > 0xFF)
    {
        attribl_bc +=3;
    }
    else
    {
        attribl_bc +=2;
    }


    return attribl_bc;
}


_ATTR_LWBT_CODE_
int sdp_get_total_attrib_size(uint8 size, struct pbuf *p)
{
    struct sdp_record *record;
    int totalsize = 0;
    int sizetemp = 0;
    for(record = sdp_server_records; record != NULL; record = record->next)
    {
        /* Check if service search pattern matches record */
        if(sdp_pattern_search(record, size, p))
        {
            /* Search for attributes and add them to a pbuf */
            pbuf_header(p, -(size + 2));
            sizetemp = sdp_get_single_attribute_size(p, record);
            if(sdp_err_rsp_code)
                return 0;
            totalsize+= sizetemp;
            pbuf_header(p, size + 2);


        }
        else
        {
            if(sdp_err_rsp_code)
                return 0;
        }
    }


    return totalsize;
}

_ATTR_LWBT_CODE_
int de_get_len(uint8 *header)
{
    uint16 size;
    if(header[0] == 0x35)
    {

        return header[1]+2;
    }
    else if(header[0] == 0x36)
    {
        size = ((uint16)header[1]<< 8) | header[2];

        return size+3;
    }

    return 0;

}
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_service_search_attrib_rsp():
 *
 * Sends a response that contains a list of attributes (both attribute ID and
 * attribute value) from the service records that match the requested service search
 * pattern.
 */
/*-----------------------------------------------------------------------------------*/
#if LWBT_LAP
_ATTR_LWBT_CODE_
err_t sdp_service_search_attrib_rsp(struct l2cap_pcb *pcb, struct pbuf *p, struct sdp_hdr *reqhdr)
{
    struct sdp_record *record;
    struct sdp_hdr *rsphdr;

    struct pbuf *q; /* response packet */
    struct pbuf *r = NULL; /* tmp buffer */
    struct pbuf *s = NULL; /* tmp buffer */

    struct pbuf *sdpdata = NULL; /* tmp buffer */

    uint16 max_attribl_bc = 0;
    uint16 sdp_payload_len = 0;
    uint8 size = 0;
    uint16 AttributeIDListSize = 0;
    uint8 continuationSize = 0;
    err_t ret;

    uint16 totalSize = 0;
    uint8 * Pcontinuation;
    continuation_State.continuationEnable = 0;
    continuation_State.continuationLen = 0;
    recv_continuation_State.continuationEnable = 0;
    recv_continuation_State.prevSize = 0;
    /* Get size of service search pattern */
    if(SDP_DE_TYPE(((uint8 *)p->payload)[0]) == SDP_DE_TYPE_DES &&
       SDP_DE_SIZE(((uint8 *)p->payload)[0]) ==  SDP_DE_SIZE_N1)
    {
        /* Size of the search pattern must be in the next byte since only
           12 UUIDs are allowed in one pattern */
        size = ((uint8 *)p->payload)[1];

        /* Get maximum attribute byte count that follows the service search pattern */
        max_attribl_bc = ntohs(*((uint16 *)(((uint8 *)p->payload)+(2+size))));

        //AttributeIDListSize = ((uint8 *)p->payload)[2+size+2+1];

        AttributeIDListSize = de_get_len(((uint8 *)p->payload)+2+size+2);
        continuationSize = ((uint8 *)p->payload)[2+size+2+AttributeIDListSize];

        if(continuationSize)
        {
            Pcontinuation = ((uint8 *)p->payload)+2+size+2+AttributeIDListSize+1;
            recv_continuation_State.continuationEnable = 1;
            recv_continuation_State.continuationLen = continuationSize;
            recv_continuation_State.recordMemAdrr = *((uint32 *)Pcontinuation);
            recv_continuation_State.recordoffset  = *((uint16 *)(Pcontinuation+4));
            //recv_continuation_State.prevSize = *((uint16 *)(((uint8 *)p->payload)+(2+size+2+2+AttributeIDListSize+7)));
            //printf("continuationSize = %d, recordMemAdrr = %d,recordoffset = %d, prevSize = %d\r\n",continuationSize,recv_continuation_State.recordMemAdrr, recv_continuation_State.recordoffset, recv_continuation_State.prevSize );
        }
        else
        {
            max_attribl_bc = totalSize>0xFF ?(max_attribl_bc-3):(max_attribl_bc-2);
        }

        pbuf_header(p, -2);
    }
    else
    {
        //TODO: INVALID SYNTAX ERROR
        sdp_error_response(pcb, reqhdr,SDP_ERR_RSP_CODE_REQUEST_SYNTAX);
        return ERR_VAL;
    }

    /* Allocate header + attribute list count */


    sdpdata  = pbuf_alloc(PBUF_RAW, 0, PBUF_RAM);
    if(sdpdata == NULL)
    {
        //TODO: ERROR
        return ERR_MEM;
    }

    totalSize = sdp_get_total_attrib_size(size, p);
    if(sdp_err_rsp_code)
    {
        sdp_error_response(pcb, reqhdr,sdp_err_rsp_code);
        sdp_err_rsp_code = 0;
        return ERR_VAL;
    }

    for(record = sdp_server_records; record != NULL; record = record->next)
    {
        if(recv_continuation_State.continuationEnable)
        {
            if((uint32)record != recv_continuation_State.recordMemAdrr)
            {
                continue;
            }
        }
        /* Check if service search pattern matches record */
        if(sdp_pattern_search(record, size, p))
        {
            /* Search for attributes and add them to a pbuf */
            pbuf_header(p, -(size + 2));
            r = sdp_attribute_search(max_attribl_bc, p, record);
            if(r != NULL)
            {

                max_attribl_bc -= r->tot_len; /* Calculate remaining number of bytes of attribute
                                           data the server is to return in response to the
                                           request */
                //pbuf_chain(q, r); /* Chain attribute id list for service to response packet */
                pbuf_copy(sdpdata, r);
                pbuf_free(r);
            }
            else
            {
                BT_DEBUG();
                if(sdp_err_rsp_code)
                {
                    sdp_error_response(pcb, reqhdr, sdp_err_rsp_code);
                    sdp_err_rsp_code = 0;
                    pbuf_free(sdpdata);
                    return ERR_VAL;
                }
            }
            pbuf_header(p, size + 2);

            if(continuation_State.continuationEnable)
            {
                recv_continuation_State.continuationEnable = 0;
                break;
            }
        }

        recv_continuation_State.continuationEnable = 0;
    }

    if(continuationSize == 0)
    {

        if(totalSize > 0xFF )
        {
            uint16 lenbak;
            lenbak = totalSize;
            pbuf_header(sdpdata,3);
            s = sdpdata;
            ((uint8 *)s->payload)[0] = SDP_DE_TYPE_DES | SDP_DE_SIZE_N2;
            ((uint8 *)s->payload)[1] = (lenbak & 0xFF00) >> 8;
            ((uint8 *)s->payload)[2] = lenbak & 0x00FF;

        }
        else
        {
            uint16 lenbak;
            lenbak = totalSize;
            pbuf_header(sdpdata,2);
            s = sdpdata;
            ((uint8 *)s->payload)[0] = SDP_DE_TYPE_DES | SDP_DE_SIZE_N1;
            ((uint8 *)s->payload)[1] = lenbak& 0x00FF;

        }
    }
    else
    {
        s = sdpdata;
    }

    sdp_payload_len = htons(s->tot_len);

    pbuf_header(s,SDP_PDUHDR_LEN + 2);

    q = s;
    rsphdr = q->payload;
    rsphdr->pdu = SDP_SSAR_PDU;
    rsphdr->id = reqhdr->id;


    *((uint16 *)(((uint8 *)q->payload) + SDP_PDUHDR_LEN)) = sdp_payload_len;


    /* Add continuation state to packet */
    if((r = pbuf_alloc(PBUF_RAW, continuation_State.continuationLen+1, PBUF_RAM)) == NULL)
    {
        //TODO: ERROR
    }
    else
    {
        ((uint8 *)r->payload)[0] = continuation_State.continuationLen; //TODO: Is this correct?
        if(continuation_State.continuationLen)
        {
            *((uint32 *)(((uint8 *)r->payload)+1)) = continuation_State.recordMemAdrr;
            *((uint16 *)(((uint8 *)r->payload)+5)) = continuation_State.recordoffset;
            //*((uint16 *)(((uint8 *)r->payload)+7)) = htons(sdp_payload_len)+recv_continuation_State.prevSize;
            *((uint16 *)(((uint8 *)r->payload)+7)) = 0;
        }

        //pbuf_chain(q, r);
        pbuf_copy(q, r);
        pbuf_free(r);
    }

    /* Add paramenter length to header */
    rsphdr->len = htons(q->tot_len - SDP_PDUHDR_LEN);

    ret = l2ca_datawrite(pcb, q);
    pbuf_free(q);
    return ret;
}
#endif /* LWBT_LAP */
/*-----------------------------------------------------------------------------------*/
/*
 * sdp_recv():
 *
 * Called by the lower layer. Parses the header and handle the SDP message.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t sdp_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *s, err_t err)
{
    struct sdp_hdr *sdphdr;
    struct sdp_pcb *sdppcb;
    err_t ret = ERR_OK;
    uint16 i;
    struct pbuf *p, *q, *r;

    if(s->len != s->tot_len)
    {
        LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Fragmented packet received. Reassemble into one buffer\n"));
        if((p = pbuf_alloc(PBUF_RAW, s->tot_len, PBUF_RAM)) != NULL)
        {
            i = 0;
            for(r = s; r != NULL; r = r->next)
            {
                memcpy(((uint8 *)p->payload) + i, r->payload, r->len);
                i += r->len;
            }
            pbuf_free(s);
        }
        else
        {
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Could not allocate buffer for fragmented packet\n"));
            pbuf_free(s);
            return ERR_MEM;
        }
    }
    else
    {
        p = s;
    }

    sdphdr = p->payload;
    pbuf_header(p, -SDP_PDUHDR_LEN);

    //LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdphdr->len = %d, p->len= %d\n",htons(sdphdr->len), p->len));
    if(ntohs(sdphdr->len) != p->len)
    {
        sdp_error_response(pcb,sdphdr,SDP_ERR_RSP_CODE_PDU_SIZE);
        pbuf_free(p);
        return ERR_VAL;
    }
    switch(sdphdr->pdu)
    {
        case SDP_ERR_PDU:
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Error response 0x%x\n", ntohs(*((uint16 *)p->payload))));
            pbuf_free(p);
            break;
#if LWBT_LAP
        case SDP_SS_PDU: /* Client request */
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Service search request\n"));
            ret = sdp_service_search_rsp(pcb, p, sdphdr);
            pbuf_free(p);
            break;
#endif /* LWBT_LAP */
        case SDP_SSR_PDU: /* Server response */
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Service search response\n"));
            /* Find the original request */
            for(sdppcb = sdp_pcbs; sdppcb != NULL; sdppcb = sdppcb->next)
            {
                if(sdppcb->tid == ntohs(sdphdr->id))
                {
                    break; /* Found */
                } /* if */
            } /* for */
            if(sdppcb != NULL)
            {
                /* Unregister the request */
                SDP_RMV(&sdp_pcbs, sdppcb);
                /* Callback function for a service search response */
                SDP_ACTION_SERVICE_SEARCHED(sdppcb, ntohs(((uint16 *)p->payload)[0]), ntohs(((uint16 *)p->payload)[1]), ((uint32 *)p->payload) + 1);
            }
            pbuf_free(p);
            break;
#if LWBT_LAP
        case SDP_SA_PDU:
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Service attribute request\n"));
            ret = sdp_service_attrib_rsp(pcb, p, sdphdr);
            pbuf_free(p);
            break;
#endif /* LWBT_LAP */
        case SDP_SAR_PDU:
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Service attribute response\n"));
            /* Find the original request */
            for(sdppcb = sdp_pcbs; sdppcb != NULL; sdppcb = sdppcb->next)
            {
                if(sdppcb->tid == ntohs(sdphdr->id))
                {
                    /* Unregister the request */
                    SDP_RMV(&sdp_pcbs, sdppcb);
                    /* If packet is divided into several pbufs we need to merge them */
                    if(p->next != NULL)
                    {
                        r = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
                        i = 0;
                        for(q = p; q != NULL; q = q->next)
                        {
                            memcpy(((uint8 *)r->payload)+i, q->payload, q->len);
                            i += q->len;
                        }
                        pbuf_free(p);
                        p = r;
                    }
                    i = ntohs(*((uint16 *)p->payload));
                    pbuf_header(p, -2);
                    /* Callback function for a service attribute response */
                    SDP_ACTION_ATTRIB_RECV(sdppcb, i, p);
                } /* if */
            } /* for */
            pbuf_free(p);
            break;
#if LWBT_LAP
        case SDP_SSA_PDU:
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Service search attribute request\n"));
            ret = sdp_service_search_attrib_rsp(pcb, p, sdphdr);
            pbuf_free(p);
            break;
#endif /* LWBT_LAP */
        case SDP_SSAR_PDU:
            LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_recv: Service search attribute response\n"));
            /* Find the original request */
            for(sdppcb = sdp_pcbs; sdppcb != NULL; sdppcb = sdppcb->next)
            {
                if(sdppcb->tid == ntohs(sdphdr->id))
                {
                    /* Unregister the request */
                    SDP_RMV(&sdp_pcbs, sdppcb);
                    /* If packet is divided into several pbufs we need to merge them */
                    if(p->next != NULL)
                    {
                        r = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
                        i = 0;
                        for(q = p; q != NULL; q = q->next)
                        {
                            memcpy(((uint8 *)r->payload)+i, q->payload, q->len);
                            i += q->len;
                        }
                        pbuf_free(p);
                        p = r;
                    }
                    i = ntohs(*((uint16 *)p->payload));
                    pbuf_header(p, -2);
                    /* Callback function for a service search attribute response */
                    SDP_ACTION_ATTRIB_SEARCHED(sdppcb, i, p);
                    break; /* Abort request search */
                } /* if */
            } /* for */
            pbuf_free(p);
            break;
        default:
            //TODO: INVALID SYNTAX ERROR
            sdp_error_response(pcb,sdphdr,SDP_ERR_RSP_CODE_REQUEST_SYNTAX);
            break;
    }
    return ret;
}


/*-----------------------------------------------------------------------------------*/
/*
* l2cap_disconnected_ind():
*
* Called by L2CAP to indicate that remote L2CAP protocol disconnected.
* Disconnects the RFCOMM protocol and the ACL link before it initializes a search for
* other devices.
*
*/
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t sdp_l2cap_disconnected_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    err_t ret = ERR_OK;

    LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("sdp_l2cap_disconnected_ind: L2CAP disconnected\n"));

    if(pcb->psm == SDP_PSM)
    {
        sdp_lp_disconnected(arg, pcb, err);
        //l2cap_close(pcb);
    }

    return ret;
}


_ATTR_LWBT_CODE_
static err_t sdp_l2cap_disconnected_cfm(void *arg, struct l2cap_pcb *pcb)
{
    LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("l2cap_disconnected_cfm\n"));

    if(pcb->psm == SDP_PSM)
    {

    }
    if(sdp_l2cap == pcb)
        sdp_l2cap = NULL;

    l2cap_close(pcb);
    return ERR_OK;
}


/*-----------------------------------------------------------------------------------*/
/*
 * sdp_attributes_recv():
 *
 * Can be used as a callback by SDP when a response to a service attribute request or
 * a service search attribute request was received.
 * Disconnects the L2CAP SDP channel and connects to the RFCOMM one.
 * If no RFCOMM channel was found it initializes a search for other devices.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
static void sdp_attributes_recv(void *arg, struct sdp_pcb *sdppcb, uint16 attribl_bc, struct pbuf *p)
{
    struct l2cap_pcb *l2cappcb;

    l2ca_disconnect_req(sdppcb->l2cappcb, sdp_l2cap_disconnected_cfm);
    /* Get the RFCOMM channel identifier from the protocol descriptor list */
    user_req.user_attributes_searched(arg, sdppcb, attribl_bc, p);

    sdp_free(sdppcb);
}


/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_connected():
 *
 * Called by L2CAP when a connection response was received.
 * Sends a L2CAP configuration request.
 * Initializes a search for other devices if the connection attempt failed.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
static err_t sdp_l2cap_connected(void *arg, struct l2cap_pcb *l2cappcb, uint16 result, uint16 status)
{
    struct sdp_pcb *sdppcb;
    struct rfcomm_pcb *rfcommpcb;
    err_t ret;
    if(result == L2CAP_CONN_SUCCESS)
    {
        LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("l2cap_connected: L2CAP connected pcb->state = %d\n", l2cappcb->state));
        /* Tell L2CAP that we wish to be informed of a disconnection request */
        l2cap_disconnect_ind(l2cappcb, sdp_l2cap_disconnected_ind);
        switch(l2cap_psm(l2cappcb))
        {
            case SDP_PSM:
                LWBT_DEBUGF(SDP_DEBUG, _DBG_INFO_, ("l2cap_connected: SDP L2CAP configured. Result = %d\n", result));

                if((sdppcb = sdp_new(l2cappcb)) == NULL)
                {
                    LWBT_DEBUGF(SDP_DEBUG, _DBG_SERIOUS_, ("l2cap_connected: Failed to create a SDP PCB\n"));
                    return ERR_MEM;
                }

                l2cap_recv(l2cappcb, sdp_recv);

                ret = sdp_service_search_attrib_req(sdppcb, 0xFFFF, user_req.ssp, user_req.ssplen, user_req.attrids, user_req.attrlen,
                                                    sdp_attributes_recv, NULL, 0);

                return ret;
            default:
                return ERR_VAL;
        }
    }
    else
    {
        user_req.user_attributes_searched(NULL, NULL, 0, NULL);
    }

    return ERR_OK;
}
_ATTR_LWBT_CODE_
err_t sdp_service_direct_search_attrib_req(struct bd_addr *bdaddr,uint8 *ssp, uint8 ssplen, uint8 *attrids,
        uint8 attrlen, void (* attributes_searched)(void *arg, struct sdp_pcb *pcb,
                uint16 attribl_bc,
                struct pbuf *p))
{
    struct l2cap_pcb *l2cappcb;
    if((l2cappcb = l2cap_new()) == NULL)
    {
        LWBT_DEBUGF(SDP_DEBUG, _DBG_SERIOUS_, ("inquiry_complete: Could not alloc L2CAP pcb\n"));
        return ERR_MEM;
    }

    memcpy(user_req.ssp, ssp, ssplen);
    memcpy(user_req.attrids, attrids, attrlen);
    user_req.ssplen = ssplen;
    user_req.attrlen = attrlen;
    user_req.user_attributes_searched = attributes_searched;
    l2ca_connect_req(l2cappcb, bdaddr, SDP_PSM, HCI_ALLOW_ROLE_SWITCH, sdp_l2cap_connected);


}




#endif

/*-----------------------------------------------------------------------------------*/
