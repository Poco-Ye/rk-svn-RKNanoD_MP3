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
/* l2cap.c
 *
 * Implementation of the logical link control and adaption protocol (L2CAP). Supports
 * higher level protocol multiplexing, packet segmentation and reassembly, and the
 * conveying of quality of service information.
 */
/*-----------------------------------------------------------------------------------*/
#include "lwbt.h"

#include "SysInclude.h"
#include "hci.h"
#include "l2cap.h"
#include "lwbt_memp.h"
#include "lwbtopts.h"
#include "lwbtdebug.h"
#include "lwbterr.h"
#include "pbuf.h"

#ifdef _BLUETOOTH_
typedef void (*btstack_packet_handler_t) (uint8 packet_type, uint16 channel, uint8 *packet, uint16 size);
_ATTR_LWBT_UARTIF_BSS_ static btstack_packet_handler_t attribute_protocol_packet_handler;
_ATTR_LWBT_UARTIF_BSS_ static btstack_packet_handler_t security_protocol_packet_handler;

/* Next Identifier to be sent */
_ATTR_LWBT_BSS_ uint8 sigid_nxt;
_ATTR_LWBT_BSS_ uint16 cid_next;

/* The L2CAP PCB lists. */
_ATTR_LWBT_BSS_ struct l2cap_pcb_listen *l2cap_listen_pcbs;  /* List of all L2CAP PCBs in CLOSED state
                        but awaiting an incoming conn req */
_ATTR_LWBT_BSS_ struct l2cap_pcb *l2cap_active_pcbs;  /* List of all L2CAP PCBs that are in a
                     state in which they accept or send
                     data */
_ATTR_LWBT_BSS_ struct l2cap_pcb *l2cap_tmp_pcb;

/* Temp signal */
_ATTR_LWBT_BSS_ struct l2cap_sig *l2cap_tmp_sig;

/* Global variable involved in input processing of l2cap data segements */
_ATTR_LWBT_BSS_ struct l2cap_seg *l2cap_insegs;
_ATTR_LWBT_BSS_ struct l2cap_seg *l2cap_tmp_inseg;

/* Forward declarations */
static uint16 l2cap_cid_alloc(void);


#define l2cap_write(bdaddr, p, len)  _l2cap_write(bdaddr, p, len, NULL)



// Bluetooth 4.0 - allows to register handler for Attribute Protocol and Security Manager Protocol
_ATTR_LWBT_INIT_CODE_
void l2cap_register_fixed_channel(btstack_packet_handler_t packet_handler, uint16 channel_id)
{
    switch(channel_id)
    {
        case L2CAP_CID_ATTRIBUTE_PROTOCOL:
            attribute_protocol_packet_handler = packet_handler;
            break;
        case L2CAP_CID_SECURITY_MANAGER_PROTOCOL:
            security_protocol_packet_handler = packet_handler;
            break;
    }
}

/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_is_psm_serve_connected():
 *
 *
 * 检测指定地址指定的PSM 的L2cap链路是否存在
 *
 *
 */
/*-----------------------------------------------------------------------------------*/
int l2cap_is_psm_serve_connected(struct bd_addr *bdaddr, uint16 psm)
{
    int ret = 0;
    struct l2cap_pcb *pcb;
    for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        if(bd_addr_cmp(&(pcb->remote_bdaddr), bdaddr) && pcb->psm == psm)
        {
            return 1;

        }
    }

    return 0;
}

/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_get_psm_pcb():
 *
 *
 * 获取指定地址指定的PSM 的L2cap控制块
 *
 *
 */
/*-----------------------------------------------------------------------------------*/
struct l2cap_pcb *  l2cap_get_psm_pcb(struct bd_addr *bdaddr, uint16 psm)
{
    int ret = 0;
    struct l2cap_pcb *pcb;
    for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        if(bd_addr_cmp(&(pcb->remote_bdaddr), bdaddr) && pcb->psm == psm)
        {
            return pcb;
        }
    }

    return 0;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_init():
 *
 * Initializes the L2CAP layer.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void l2cap_init(void)
{
    /* Clear globals */
    l2cap_listen_pcbs = NULL;
    l2cap_active_pcbs = NULL;
    l2cap_tmp_pcb = NULL;
    l2cap_tmp_sig = NULL;
    l2cap_insegs = NULL;
    l2cap_tmp_inseg = NULL;

    /* Initialize the signal identifier (0x00 shall never be used) */
    sigid_nxt = 0x00;
    cid_next = 0;

    attribute_protocol_packet_handler = NULL;
    security_protocol_packet_handler = NULL;
}


_ATTR_LWBT_INIT_CODE_
void l2cap_deinit(void)
{
    l2cap_init();
}

/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_tmr():
 *
 * Called every 1s and implements the retransmission timer that
 * removes a channel if it has been waiting for a request enough
 * time. It also includes a configuration timer.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void l2cap_tmr(void)
{
    struct l2cap_sig *sig;
    struct l2cap_pcb *pcb;
    err_t ret;

    /* Step through all of the active pcbs */
    for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        /* Step through any unresponded signals */
        for(sig = pcb->unrsp_sigs; sig != NULL; sig = sig->next)
        {
            /* Check if channel is not reliable */
            if(pcb->cfg.outflushto < 0xFFFF)
            {
                /* Check if rtx is active. Otherwise ertx is active */
                if(sig->rtx > 0)
                {
                    /* Adjust rtx timer */
                    --sig->rtx;
                    /* Check if rtx has expired */
                    if(sig->rtx == 0)
                    {
                        if(sig->nrtx == 0)
                        {
                            /* Move pcb to closed state */
                            pcb->state = L2CAP_CLOSED;
                            /* Indicate disconnect to upper layer */
                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_tmr: Max number of retransmissions (rtx) has expired\n"));
                            L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);
                        }
                        else
                        {
                            --sig->nrtx;
                            /* Indicate timeout to upper layer */
                            L2CA_ACTION_TO_IND(pcb,ERR_OK,ret);
                            /* Retransmitt signal w timeout doubled */
                            sig->rtx += sig->rtx;
                            ret = l2cap_rexmit_signal(pcb, sig);
                        }
                    } /* if */
                }
                else
                {
                    /* Adjust ertx timer */
                    --sig->ertx;
                    /* Check if ertx has expired */
                    if(sig->ertx == 0)
                    {
                        if(sig->nrtx == 0)
                        {
                            /* Move pcb to closed state */
                            pcb->state = L2CAP_CLOSED;
                            /* Indicate disconnect to upper layer */
                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_tmr: Max number of retransmissions (ertx) has expired\n"));
                            L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);
                        }
                        else
                        {
                            --sig->nrtx;
                            /* Indicate timeout to upper layer */
                            L2CA_ACTION_TO_IND(pcb,ERR_OK,ret);
                            /* Disable ertx, activate rtx and retransmitt signal */
                            sig->ertx = 0;
                            sig->rtx = L2CAP_RTX;
                            ret = l2cap_rexmit_signal(pcb, sig);
                        }
                    } /* if */
                } /* else */
            } /* if */
        } /* for */

        /* Check configuration timer */
        if(pcb->state == L2CAP_CONFIG)
        {
            /* Check if configuration timer is active */
            if(pcb->cfg.cfgto > 0)
            {
                --pcb->cfg.cfgto;
                LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_tmr: Configuration timer = %d\n", pcb->cfg.cfgto));
                /* Check if config timer has expired */
                if(pcb->cfg.cfgto == 0)
                {
                    /* Connection attempt failed. Disconnect */
                    l2ca_disconnect_req(pcb, NULL);
                    /* Notify the application that the connection attempt failed */
                    if(pcb->cfg.l2capcfg & L2CAP_CFG_IR)
                    {
                        L2CA_ACTION_CONN_CFM(pcb, L2CAP_CONN_CFG_TO, 0x0000, ret);
                    }
                    else
                    {
                        L2CA_ACTION_CONN_IND(pcb, ERR_OK, ret);
                    }
                    pcb->cfg.cfgto = L2CAP_CFG_TO; /* Reset timer */
                }
                else if(L2CAP_CFG_TO/2 == pcb->cfg.cfgto)
                {
                    l2ca_config_req(pcb,L2CAP_USER_FG_NULL);
                }
            }
        }
    } /* for */
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_write():
 *
 * Output L2CAP data to the lower layers. Segments the packet in to PDUs.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t _l2cap_write(struct bd_addr *bdaddr, struct pbuf *p, uint16 len, void (*func)(void))
{
    uint8 pb = L2CAP_ACL_START;
    uint16 maxsize;
    uint16 outsize;
    err_t ret = ERR_OK;
    struct pbuf *q;
    uint16 i = 0;


    maxsize = lp_pdu_maxsize();
    q = p;
    HciServeIsrDisable();
    while(len && ret == ERR_OK)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_write: len %d maxsize %d p->len %d\n", len, maxsize, p->len));
        if(len > maxsize)
        {   pb = 0x80|pb; //need copy pbuf
            //ret = lp_acl_write(bdaddr, q, maxsize, pb, func);
            ret = lp_acl_write(bdaddr, q, maxsize, pb, NULL);
            len -= maxsize;
            outsize = maxsize;
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_write: Outsize before %d\n", outsize));
            while(q->len < outsize)
            {
                outsize -= q->len;
                q = q->next;
            }
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_write: Outsize after %d\n", outsize));
            if(outsize)
            {
                pbuf_header(q, -outsize);
                i += outsize;
            }
            pb = 0x80|L2CAP_ACL_CONT;
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_write: FRAG\n"));
        }
        else
        {
            ret = lp_acl_write(bdaddr, q, len, pb, func);
            len = 0;
        }
    }
    HciServeIsrEnable();
    //pbuf_header(q, i);
    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_write: DONE\n"));
    return ret;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_process_sig():
 *
 * Parses the received message handles it.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void l2cap_process_sig(struct pbuf *q, struct l2cap_hdr *l2caphdr, struct bd_addr *bdaddr)
{

    struct l2cap_sig_hdr *sighdr;
    struct l2cap_sig *sig = NULL;
    struct l2cap_pcb *pcb = NULL;
    struct l2cap_pcb_listen *lpcb;
    struct l2cap_cfgopt_hdr *opthdr;
    uint16 result, status, flags, psm, dcid, scid, infoType;
    uint16 len;
    uint16 siglen;
    struct pbuf *p, *r = NULL, *s = NULL, *data;
    err_t ret;
    uint8 i;
    uint16 rspstate = L2CAP_CFG_SUCCESS;
    int16 payloadlenbak = 0;
    if(q->len != q->tot_len)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Fragmented packet received. Reassemble into one buffer\n"));
        if((p = pbuf_alloc(PBUF_RAW, q->tot_len, PBUF_RAM)) != NULL)
        {
            i = 0;
            for(r = q; r != NULL; r = r->next)
            {
                memcpy(((uint8 *)p->payload) + i, r->payload, r->len);
                i += r->len;
            }
        }
        else
        {
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SEVERE_, ("l2cap_process_sig: Could not allocate buffer for fragmented packet\n"));
            return;
        }
    }
    else
    {
        p = q;
    }

    len = l2caphdr->len;

    while(len > 0)
    {
        /* Set up signal header */
        sighdr = p->payload;
        pbuf_header(p, -L2CAP_SIGHDR_LEN);

        /* Check if this is a response/reject signal, and if so, find the matching request */
        if(sighdr->code % 2)
        {
            /* if odd this is a resp/rej signal */
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Response/reject signal received id = %d code = %d\n",
                                      sighdr->id, sighdr->code));
            for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
            {
                for(sig = pcb->unrsp_sigs; sig != NULL; sig = sig->next)
                {
                    if(sig->sigid == sighdr->id)
                    {
                        break; /* found */
                    }
                }
                if(sig != NULL)
                {
                    break;
                }
            }
        }
        else
        {
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SEVERE_, ("l2cap_process_sig: Request signal received id = %d code = %d\n",
                                      sighdr->id, sighdr->code));
        }

        /* Reject packet if length exceeds MTU */
        if(l2caphdr->len > L2CAP_MTU)
        {
            /* Alloc size of reason in cmd rej + MTU */
            if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+2, PBUF_RAM)) != NULL)
            {
                ((uint16 *)data->payload)[0] = L2CAP_MTU_EXCEEDED;
                ((uint16 *)data->payload)[1] = L2CAP_MTU;

                l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
            }
            break;
        }

        switch(sighdr->code)
        {
            case L2CAP_CMD_REJ:
                /* Remove signal from unresponded list and deallocate it */
                if(sig != NULL)
                {

                    L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);

                    pbuf_free(sig->p);

                    lwbt_memp_free(MEMP_L2CAP_SIG, sig);

                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_process_sig: Our command was rejected so we disconnect\n"));

                    l2ca_disconnect_req(pcb, NULL);
                }
                break;
            case L2CAP_CONN_REQ:
                psm = ((uint16 *)p->payload)[0];
                /* Search for a listening pcb */
                for(lpcb = l2cap_listen_pcbs; lpcb != NULL; lpcb = lpcb->next)
                {
                    if(lpcb->psm == psm)
                    {
                        /* Found a listening pcb with the correct PSM */
                        break;
                    }
                }
                /* If no matching pcb was found, send a connection rsp neg (PSM) */
                if(lpcb == NULL)
                {
                    /* Alloc size of data in conn rsp signal */
                    if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_RSP_SIZE, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = L2CAP_CONN_REF_PSM;
                        ((uint16 *)data->payload)[1] = 0; /* No further info available */
                        ret = l2cap_signal(pcb, L2CAP_CONN_RSP, sighdr->id, &(pcb->remote_bdaddr), data);
                    }
                }
                else
                {
                    /* Initiate a new active pcb */
                    pcb = l2cap_new();
                    if(pcb == NULL)
                    {
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_process_sig: could not allocate PCB\n"));
                        /* Send a connection rsp neg (no resources available) and alloc size of data in conn rsp
                           signal */
                        if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_RSP_SIZE, PBUF_RAM)) != NULL)
                        {
                            ((uint16 *)data->payload)[0] = L2CAP_CONN_REF_RES;
                            ((uint16 *)data->payload)[1] = 0; /* No further info available */
                            ret = l2cap_signal(pcb, L2CAP_CONN_RSP, sighdr->id, bdaddr, data);
                        }

                    }
                    else
                    {
                        bd_addr_set(&(pcb->remote_bdaddr),bdaddr);

                        pcb->scid = l2cap_cid_alloc();
                        pcb->dcid = ((uint16 *)p->payload)[1];
                        pcb->psm = psm;
                        pcb->callback_arg = lpcb->callback_arg;
                        pcb->l2ca_connect_ind = lpcb->l2ca_connect_ind;

                        pcb->state = L2CAP_CONFIG;
                        L2CAP_REG(&l2cap_active_pcbs, pcb);

                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: A connection request was received. Send a response\n"));
                        data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_RSP_SIZE, PBUF_RAM);
                        if(data == NULL)
                        {
                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_connect_rsp: Could not allocate memory for pbuf\n"));
                            break;
                        }
                        ((uint16 *)data->payload)[0] = pcb->scid;
                        ((uint16 *)data->payload)[1] = pcb->dcid;
                        ((uint16 *)data->payload)[2] = L2CAP_CONN_SUCCESS;
                        ((uint16 *)data->payload)[3] = 0x0000; /* No further information available */

                        /* Send the response */
                        ret = l2cap_signal(pcb, L2CAP_CONN_RSP, sighdr->id, &(pcb->remote_bdaddr), data);
                    }
                }
                break;
            case L2CAP_CONN_RSP:
                if(pcb == NULL)
                {
                    /* A response without a matching request is silently discarded */
                    break;
                }
                LWIP_ASSERT("l2cap_process_sig: conn rsp, active pcb->state == W4_L2CAP_CONNECT_RSP\n",
                            pcb->state == W4_L2CAP_CONNECT_RSP);
                result = ((uint16 *)p->payload)[2];
                status = ((uint16 *)p->payload)[3];
                switch(result)
                {
                    case L2CAP_CONN_SUCCESS:
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: Conn_rsp_sucess, status %d\n", status));

                        LWIP_ASSERT("l2cap_process_sig: conn rsp success, pcb->scid == ((uint16 *)p->payload)[1]\n",
                                    pcb->scid == ((uint16 *)p->payload)[1]);

                        /* Set destination connection id */
                        pcb->dcid = ((uint16 *)p->payload)[0];

                        /* Remove signal from unresponded list and deallocate it */
                        L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
                        pbuf_free(sig->p);
                        lwbt_memp_free(MEMP_L2CAP_SIG, sig);

                        /* Configure connection */
                        pcb->state = L2CAP_CONFIG;

                        /* If initiator send a configuration request */
                        if(pcb->cfg.l2capcfg & L2CAP_CFG_IR)
                        {
                            l2ca_config_req(pcb,L2CAP_USER_FG_NULL);
                            pcb->cfg.l2capcfg |= L2CAP_CFG_OUT_REQ;
                        }
                        break;
                    case L2CAP_CONN_PND:
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Conn_rsp_pnd, status %d\n", status));

                        /* Disable rtx and enable ertx */
                        sig->rtx = 0;
                        sig->ertx = L2CAP_ERTX;
                        break;
                    default:
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Conn_rsp_neg, result %d\n", result));
                        /* Remove signal from unresponded list and deallocate it */
                        L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
                        pbuf_free(sig->p);
                        lwbt_memp_free(MEMP_L2CAP_SIG, sig);

                        L2CA_ACTION_CONN_CFM(pcb,result,status,ret);
                        break;
                }
                break;
            case L2CAP_CFG_REQ:
                siglen = sighdr->len;
                dcid = ((uint16 *)p->payload)[0];
                flags = ((uint16 *)p->payload)[1];
                siglen -= 4;

                payloadlenbak = p->len;
                pbuf_header(p, -4);


                LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: Congfiguration request, flags = %d\n", flags));

                /* Find PCB with matching cid */
                for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
                {
                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: dcid = 0x%x, pcb->scid = 0x%x, pcb->dcid = 0x%x\n\n", dcid, pcb->scid, pcb->dcid));
                    if(pcb->scid == dcid)
                    {
                        /* Matching cid found */
                        break;
                    }
                }
                /* If no matching cid was found, send a cmd reject (Invalid cid) */
                if(pcb == NULL)
                {
                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_WARNING_, ("l2cap_process_sig: Cfg req: no matching cid was found\n"));
                    /* Alloc size of reason in cmd rej + data (dcid + scid) */
                    if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+4, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = L2CAP_INVALID_CID;
                        ((uint16 *)data->payload)[1] = dcid; /* Requested local cid */
                        ((uint16 *)data->payload)[2] = L2CAP_NULL_CID; /* Remote cid not known */

                        ret = l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
                    }
                }
                else
                {
                    /* Handle config request */
                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: Handle configuration request\n"));
                    pcb->ursp_id = sighdr->id; /* Set id of request to respond to */

                    /* Parse options and add to pcb */
                    while(siglen > 0)
                    {
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: Siglen = %d\n", siglen));
                        opthdr = p->payload;
                        /* Check if type of action bit indicates a non-hint. Hints are ignored */
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: Type of action bit = %d\n", L2CAP_OPTH_TOA(opthdr)));
                        if(L2CAP_OPTH_TOA(opthdr) == 0)
                        {

                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: Type = %d\n", L2CAP_OPTH_TYPE(opthdr)));
                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_process_sig: Length = %d\n", opthdr->len));
                            switch(L2CAP_OPTH_TYPE(opthdr))
                            {
                                case L2CAP_CFG_MTU:
                                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Out MTU = %d\n", ((uint16 *)p->payload)[1]));
                                    pcb->cfg.outmtu = ((uint16 *)p->payload)[1];
                                    break;
                                case L2CAP_FLUSHTO:
                                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: In flush timeout = %d\n", ((uint16 *)p->payload)[1]));
                                    pcb->cfg.influshto = ((uint16 *)p->payload)[1];
                                    break;
                                case L2CAP_QOS:
                                    /* If service type is Best Effort or No Traffic the remainder fields will be ignored */
                                    if(((uint8 *)p->payload)[3] == L2CAP_QOS_GUARANTEED)
                                    {

                                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: This implementation does not support the guaranteed QOS service type"));
                                        if(rspstate == L2CAP_CFG_SUCCESS)
                                        {
                                            rspstate = L2CAP_CFG_UNACCEPT;
                                            if(pcb->cfg.opt != NULL)
                                            {
                                                pbuf_free(pcb->cfg.opt);
                                                pcb->cfg.opt = NULL;
                                            }
                                        }
                                        s = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + opthdr->len, PBUF_RAM);
                                        memcpy((uint8 *)s->payload, (uint8 *)p->payload, L2CAP_CFGOPTHDR_LEN + opthdr->len);
                                        if(pcb->cfg.opt == NULL)
                                        {
                                            pcb->cfg.opt = s;
                                        }
                                        else
                                        {
                                            //pbuf_chain(pcb->cfg.opt, s);
                                            pbuf_copy(pcb->cfg.opt, s);
                                            pbuf_free(s);
                                        }
                                    }
                                    break;


                                default:

                                    if(rspstate != L2CAP_CFG_REJ)
                                    {
                                        /* Unknown option. Add to unknown option type buffer */
                                        if(rspstate != L2CAP_CFG_UNKNOWN)
                                        {
                                            rspstate = L2CAP_CFG_UNKNOWN;
                                            if(pcb->cfg.opt != NULL)
                                            {
                                                pbuf_free(pcb->cfg.opt);
                                                pcb->cfg.opt = NULL;
                                            }
                                        }
                                        s = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + opthdr->len, PBUF_RAM);
                                        memcpy((uint8 *)s->payload, (uint8 *)p->payload, L2CAP_CFGOPTHDR_LEN + opthdr->len);
                                        if(pcb->cfg.opt == NULL)
                                        {
                                            pcb->cfg.opt = s;
                                        }
                                        else
                                        {
                                            //pbuf_chain(pcb->cfg.opt, s);
                                            pbuf_copy(pcb->cfg.opt, s);
                                            pbuf_free(s);
                                        }
                                    }
                                    break;
                            } /* switch */
                        } /* if(L2CAP_OPTH_TOA(opthdr) == 0) */
                        pbuf_header(p, -(L2CAP_CFGOPTHDR_LEN + opthdr->len));
                        siglen -= L2CAP_CFGOPTHDR_LEN + opthdr->len;
                    } /* while */

                    /* If continuation flag is set we don't send the final response just yet */
                    if((flags & 0x0001) == 1)
                    {
                        /* Send success result with no options until the full request has been received */
                        if((data = pbuf_alloc(PBUF_RAW, L2CAP_CFG_RSP_SIZE, PBUF_RAM)) == NULL)
                        {
                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SEVERE_, ("l2cap_process_sig: Could not allocate memory for pbuf\n"));
                            break;
                        }
                        ((uint16 *)data->payload)[0] = pcb->dcid;
                        ((uint16 *)data->payload)[1] = 1;/*when request set 1,respone is also set 1*/
                        ((uint16 *)data->payload)[2] = L2CAP_CFG_SUCCESS;
                        ret = l2cap_signal(pcb, L2CAP_CFG_RSP, pcb->ursp_id, &(pcb->remote_bdaddr), data);
                        break;
                    }

                    /* Send response to configuration request */
                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Send response to configuration request\n"));


                    if((data = pbuf_alloc(PBUF_RAW, L2CAP_CFG_RSP_SIZE, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = pcb->dcid;
                        ((uint16 *)data->payload)[1] = 0; /* Flags (No continuation) */
                        ((uint16 *)data->payload)[2] = rspstate; /* Result */
                        if(pcb->cfg.opt != NULL)
                        {
                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: pcb->cfg.opt->len = %d\n", pcb->cfg.opt->len));
                            //pbuf_chain(data, pcb->cfg.opt); /* Add option type buffer to data buffer */
                            pbuf_copy(data, pcb->cfg.opt);
                            pbuf_free(pcb->cfg.opt);
                            pcb->cfg.opt = NULL;
                        }
                        ret = l2cap_signal(pcb, L2CAP_CFG_RSP, pcb->ursp_id, &(pcb->remote_bdaddr), data);
                    }

                    /* Send a configure request for outgoing link if it hasnt been configured */
                    if(!(pcb->cfg.l2capcfg & L2CAP_CFG_IR) && !(pcb->cfg.l2capcfg & L2CAP_CFG_OUT_REQ))
                    {
                        l2ca_config_req(pcb, L2CAP_USER_FG_NULL);
                        pcb->cfg.l2capcfg |= L2CAP_CFG_OUT_REQ;
                    }

                    if(rspstate == L2CAP_CFG_SUCCESS)
                    {
                        pcb->cfg.l2capcfg |= L2CAP_CFG_OUT_SUCCESS;
                        /* L2CAP connection established if a successful configuration response has been sent */
                        if(pcb->cfg.l2capcfg & L2CAP_CFG_IN_SUCCESS)
                        {
                            /* IPCP connection established, notify upper layer that connection is open */
                            pcb->state = L2CAP_OPEN;
                            if(pcb->cfg.l2capcfg & L2CAP_CFG_IR)
                            {
                                L2CA_ACTION_CONN_CFM(pcb, L2CAP_CONN_SUCCESS, 0x0000, ret);
                            }
                            else
                            {
                                L2CA_ACTION_CONN_IND(pcb, ERR_OK, ret);
                            }
                        }
                    }
                } /* else */

                pbuf_header(p, payloadlenbak- p->len);
                break;
            case L2CAP_CFG_RSP:
                if(pcb == NULL)
                {
                    /* A response without a matching request is silently discarded */
                    break;
                }

                /* Remove signal from unresponded list and deallocate it */
                L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
                pbuf_free(sig->p);
                lwbt_memp_free(MEMP_L2CAP_SIG, sig);

                LWIP_ASSERT(("l2cap_process_sig: cfg rsp, active pcb->state == L2CAP_CONFIG\n"),
                            pcb->state == L2CAP_CONFIG);

                siglen = sighdr->len;
                scid = ((uint16 *)p->payload)[0];
                flags = ((uint16 *)p->payload)[1];
                result = ((uint16 *)p->payload)[2];
                siglen -= 6;

                payloadlenbak = p->len;
                pbuf_header(p, -6);

                LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Outgoing configuration result == %d continuation flag == %d\n", result, flags));

                /* Handle config request */
                switch(result)
                {
                    case L2CAP_CFG_SUCCESS:
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Successfull outgoing configuration\n"));
                        pcb->cfg.l2capcfg |= L2CAP_CFG_IN_SUCCESS; /* Local side of the connection
                                          has been configured for outgoing data */
                        pcb->cfg.cfgto = L2CAP_CFG_TO; /* Reset configuration timeout */

                        if(pcb->cfg.outflushto != L2CAP_CFG_DEFAULT_OUTFLUSHTO)
                        {
                            lp_write_flush_timeout(&pcb->remote_bdaddr, pcb->cfg.outflushto);
                        }

                        /* L2CAP connection established if a successful configuration response has been sent */
                        if(pcb->cfg.l2capcfg & L2CAP_CFG_OUT_SUCCESS)
                        {
                            pcb->state = L2CAP_OPEN;
                            if(pcb->cfg.l2capcfg & L2CAP_CFG_IR)
                            {
                                L2CA_ACTION_CONN_CFM(pcb, L2CAP_CONN_SUCCESS, 0x0000, ret);
                            }
                            else
                            {
                                L2CA_ACTION_CONN_IND(pcb, ERR_OK, ret);
                            }
                        }
                        break;
                    case L2CAP_CFG_UNACCEPT:
                        /* Parse and add options to pcb */
                        while(siglen > 0)
                        {
                            opthdr = p->payload;
                            /* Check if type of action bit indicates a non-hint. Hints are ignored */
                            if(L2CAP_OPTH_TOA(opthdr) == 0)
                            {
                                switch(L2CAP_OPTH_TYPE(opthdr))
                                {
                                    case L2CAP_CFG_MTU:
                                        if(L2CAP_MTU >= ((uint16 *)p->payload)[1])
                                        {
                                            pcb->cfg.outmtu = ((uint16 *)p->payload)[1];
                                        }
                                        else
                                        {
                                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_process_sig: Configuration of MTU failed\n"));
                                            l2ca_disconnect_req(pcb, NULL);
                                            return;
                                        }
                                        break;
                                    case L2CAP_FLUSHTO:
                                        pcb->cfg.influshto = ((uint16 *)p->payload)[1];
                                        break;
                                    case L2CAP_QOS:
                                        /* If service type Best Effort is not accepted we will close the connection */
                                        if(((uint8 *)p->payload)[3] != L2CAP_QOS_BEST_EFFORT)
                                        {
                                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_process_sig: Unsupported service type\n"));
                                            l2ca_disconnect_req(pcb, NULL);
                                            return;
                                        }
                                        break;
                                    default:
                                        /* Should not happen, skip option */
                                        break;
                                } /* switch */
                            } /* if(L2CAP_OPTH_TOA(opthdr) == 0) */
                            pbuf_header(p, -(L2CAP_CFGOPTHDR_LEN + opthdr->len));
                            siglen -= L2CAP_CFGOPTHDR_LEN + opthdr->len;
                        } /* while */

                        /* Send out a new configuration request if the continuation flag isn't set */
                        if((flags & 0x0001) == 0)
                        {
                            l2ca_config_req(pcb, L2CAP_USER_CFG_MTU);
                        }
                        break;
                    case L2CAP_CFG_REJ:
                    /* Fallthrough */
                    case L2CAP_CFG_UNKNOWN:
                    /* Fallthrough */
                    default:
                        if((flags & 0x0001) == 0)
                        {
                            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Configuration failed\n"));
                            l2ca_disconnect_req(pcb, NULL);
                            return;
                        }
                        break;
                } /* switch(result) */

                /* If continuation flag is set we must send a NULL configuration request */
                if((flags & 0x0001) == 1)
                {
                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Continuation flag is set. Send empty (default) config request signal\n"));
                    if((data = pbuf_alloc(PBUF_RAW, L2CAP_CFG_REQ_SIZE, PBUF_RAM)) == NULL)
                    {
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_process_sig: Could not allocate memory for pbuf\n"));
                        return;

                    }
                    /* Assemble config request packet */
                    ((uint16 *)data->payload)[0] = pcb->scid;
                    ((uint16 *)data->payload)[2] = 0;
                    l2cap_signal(pcb, L2CAP_CFG_REQ, 0, &(pcb->remote_bdaddr), data);
                }


                pbuf_header(p, payloadlenbak- p->len);
                break;
            case L2CAP_DISCONN_REQ:
                siglen = sighdr->len;
                dcid = ((uint16 *)p->payload)[0];
                siglen = siglen - 2;
                flags = ((uint16 *)p->payload)[1];
                siglen = siglen - 2;

                payloadlenbak = p->len;
                pbuf_header(p, -4);

                /* Find PCB with matching cid */
                for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
                {
                    if(pcb->scid == dcid)
                    {
                        /* Matching cid found */
                        break;
                    }
                }
                /* If no matching cid was found, send a cmd reject (Invalid cid) */
                if(pcb == NULL)
                {
                    /* Alloc size of reason in cmd rej + data (dcid + scid) */
                    if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+4, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = L2CAP_INVALID_CID;
                        ((uint16 *)data->payload)[1] = dcid; /* Requested local cid */
                        ((uint16 *)data->payload)[2] = L2CAP_NULL_CID; /* Remote cid not known */

                        ret = l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
                    }
                }
                else
                {
                    /* Handle disconnection request */
                    if((data = pbuf_alloc(PBUF_RAW, L2CAP_DISCONN_RSP_SIZE, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = pcb->scid;
                        ((uint16 *)data->payload)[1] = pcb->dcid;
                        ret = l2cap_signal(pcb, L2CAP_DISCONN_RSP, sighdr->id, &(pcb->remote_bdaddr), data);

                        /* Give upper layer indication */
                        pcb->state = L2CAP_CLOSED;
                        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_process_sig: Disconnection request\n"));
                        L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);
                    }
                }

                pbuf_header(p, payloadlenbak- p->len);
                break;
            case L2CAP_DISCONN_RSP:
                if(pcb == NULL)
                {
                    /* A response without a matching request is silently discarded */
                    break;
                }
                /* Remove signal from unresponded list and deallocate it */
                L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
                pbuf_free(sig->p);
                lwbt_memp_free(MEMP_L2CAP_SIG, sig);

                L2CA_ACTION_DISCONN_CFM(pcb,ret); /* NOTE: Application should
                           now close the connection */
                break;
            case L2CAP_ECHO_REQ:
                //pcb->ursp_id = sighdr->id;
                //ret = l2cap_signal(pcb, L2CAP_ECHO_RSP, sighdr->id, &(pcb->remote_bdaddr), NULL);

                if(L2CAP_ECHO_REQ == sighdr->code)
                {
                    ret = l2cap_signal(NULL, L2CAP_ECHO_RSP, sighdr->id, bdaddr, NULL);
                }
                break;
            case L2CAP_ECHO_RSP:
                if(pcb == NULL)
                {
                    /* A response without a matching request is silently discarded */
                    break;
                }
                /* Remove signal from unresponded list and deallocate it */
                L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
                pbuf_free(sig->p);
                lwbt_memp_free(MEMP_L2CAP_SIG, sig);

                /* Remove temporary pcb from active list */
                L2CAP_RMV(&l2cap_active_pcbs, pcb);
                L2CA_ACTION_PING_CFM(pcb,L2CAP_ECHO_RCVD,ret);
                break;

            case L2CAP_INFO_REQ:

                infoType = ((uint16 *)p->payload)[0];
                if(infoType == L2CAP_INFOTYPE_CONN_MTU)
                {
                    /* Alloc size of reason in cmd rej */
                    if((data = pbuf_alloc(PBUF_RAW, 6, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = infoType;
                        ((uint16 *)data->payload)[1] = 0x0000; //Success
                        ((uint16 *)data->payload)[1] = L2CAP_CFG_DEFAULT_INMTU; //Success
                        ret = l2cap_signal(NULL, L2CAP_INFO_RSP, sighdr->id, bdaddr, data);
                    }
                }
                else if(infoType == L2CAP_INFOTYPE_Ext_FEATURE_MASK)
                {
                    /* Alloc size of reason in cmd rej */
                    if((data = pbuf_alloc(PBUF_RAW, 8, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = infoType;
                        ((uint16 *)data->payload)[1] = 0x0000; //Success
                        ((uint16 *)data->payload)[2] = 0x0000; //Success
                        ((uint16 *)data->payload)[3] = 0x0000; //Success
                        ret = l2cap_signal(NULL, L2CAP_INFO_RSP, sighdr->id, bdaddr, data);
                    }
                }
                else
                {
                    /* Alloc size of reason in cmd rej */
                    if((data = pbuf_alloc(PBUF_RAW, 4, PBUF_RAM)) != NULL)
                    {
                        ((uint16 *)data->payload)[0] = infoType;
                        ((uint16 *)data->payload)[1] = 0x0001; //Not supported
                        ret = l2cap_signal(NULL, L2CAP_INFO_RSP, sighdr->id, bdaddr, data);
                    }
                }


                break;
            case L2CAP_INFO_RSP:
                break;
            default:
                /* Alloc size of reason in cmd rej */
                if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE, PBUF_RAM)) != NULL)
                {
                    ((uint16 *)data->payload)[0] = L2CAP_CMD_NOT_UNDERSTOOD;

                    ret = l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
                }
                break;
        } /* switch */
        len = len - (sighdr->len + L2CAP_SIGHDR_LEN);
        pbuf_header(p, -(sighdr->len));
    } /* while */
}

_ATTR_LWBT_UARTIF_CODE_
void l2cap_dispatch_event(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size)
{
    int ret = 0;
    struct l2cap_pcb *pcb;
    switch(packet[0])
    {
        case HCI_DISCONNECTION_COMPLETE:
        case HCI_NBR_OF_COMPLETED_PACKETS:

            break;

        default:

            return;
            break;

    }

    for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        if(pcb->l2ca_event_handler)
        {
            pcb->l2ca_event_handler(packet_type, channel, packet, size);
        }
    }
}

_ATTR_LWBT_UARTIF_CODE_
void l2cap_event_packet_handler(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size)
{

    l2cap_dispatch_event(packet_type, channel, packet, size);
    if(security_protocol_packet_handler)
    {
        security_protocol_packet_handler(packet_type, channel, packet, size);
    }

    if (attribute_protocol_packet_handler)
    {
        (*attribute_protocol_packet_handler)(packet_type, channel, packet, size);
    }
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_input():
 *
 * Called by the lower layer. Reassembles the packet, parses the header and forward
 * it to the upper layer or the signal handler.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void l2cap_input(struct pbuf *p, struct bd_addr *bdaddr)
{
    struct l2cap_seg *inseg;
    struct hci_acl_hdr *aclhdr;
    struct pbuf *data;
    err_t ret;

    pbuf_header(p, HCI_ACL_HDR_LEN);
    aclhdr = p->payload;
    pbuf_header(p, -HCI_ACL_HDR_LEN);

    pbuf_realloc(p, aclhdr->len);

    for(inseg = l2cap_insegs; inseg != NULL; inseg = inseg->next)
    {
        if(bd_addr_cmp(bdaddr, &(inseg->bdaddr)))
        {
            break;
        }
    }

    /* Reassembly procedures */
    /* Check if continuing fragment or start of L2CAP packet */
    if(((aclhdr->conhdl_pb_bc >> 12) & 0x03)== L2CAP_ACL_CONT)
    {
        /* Continuing fragment */
        if(inseg == NULL)
        {
            /* Discard packet */
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_input: Continuing fragment. Discard packet\n"));
            pbuf_free(p);
            return;
        }
        else if(inseg->p->tot_len + p->tot_len > inseg->len)
        {
            /* Check if length of
                                    segment exceeds
                                    l2cap header length */
            /* Discard packet */
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_input: Continuing fragment. Length exceeds L2CAP hdr length. Discard packet\n"));
            pbuf_free(inseg->p);
            L2CAP_SEG_RMV(&(l2cap_insegs), inseg);
            lwbt_memp_free(MEMP_L2CAP_SEG, inseg);

            pbuf_free(p);
            return;
        }
        /* Add pbuf to segement */
        //pbuf_chain(inseg->p, p);
        pbuf_copy(inseg->p, p);
        pbuf_free(p);

    }
    else if(((aclhdr->conhdl_pb_bc >> 12) & 0x03) == L2CAP_ACL_START)
    {
        /* Start of L2CAP packet */
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_input: Start of L2CAP packet p->len = %d, p->tot_len = %d\n",
                                  p->len, p->tot_len));
        if(inseg != NULL)
        {
            /* Check if there are segments missing in a previous packet */
            /* Discard previous packet */
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_input: Start of L2CAP packet. Discard previous packet\n"));
            pbuf_free(inseg->p);
        }
        else
        {
            inseg = lwbt_memp_malloc(MEMP_L2CAP_SEG);
            bd_addr_set(&(inseg->bdaddr), bdaddr);
            L2CAP_SEG_REG(&(l2cap_insegs), inseg);
        }
        inseg->p = p;
        inseg->l2caphdr = p->payload;
        inseg->len = inseg->l2caphdr->len + L2CAP_HDR_LEN;
        for(inseg->pcb = l2cap_active_pcbs; inseg->pcb != NULL; inseg->pcb = inseg->pcb->next)
        {
            if(inseg->pcb->scid == inseg->l2caphdr->cid)
            {
                break; /* found */
            }
        }
    }
    else
    {
        /* Discard packet */
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_input: Discard packet\n"));
        pbuf_free(inseg->p);
        L2CAP_SEG_RMV(&(l2cap_insegs), inseg);
        lwbt_memp_free(MEMP_L2CAP_SEG, inseg);

        pbuf_free(p);
        return;
    }

    if(inseg->p->tot_len < inseg->len)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_input: Get continuing segments\n"));
        return; /* Get continuing segments */
    }

    /* Handle packet */
    switch(inseg->l2caphdr->cid)
    {
        case L2CAP_NULL_CID:
            /* Illegal */
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_WARNING_,("l2cap_input: Illegal null cid\n"));
            pbuf_free(inseg->p);
            break;

        case L2CAP_SIG_CID:
            pbuf_header(inseg->p, -L2CAP_HDR_LEN);
            l2cap_process_sig(inseg->p, inseg->l2caphdr, bdaddr);
            pbuf_free(inseg->p);
            break;

        case L2CAP_CONNLESS_CID:
            /* Not needed by PAN, LAN access or DUN profiles */
            pbuf_free(inseg->p);
            break;

        case L2CAP_CID_ATTRIBUTE_PROTOCOL:
        {
            pbuf_header(inseg->p, -L2CAP_HDR_LEN);
            if (attribute_protocol_packet_handler)
            {
                (*attribute_protocol_packet_handler)(ATT_DATA_PACKET, *((uint16*)(&bdaddr->addr[0]+6)), inseg->p->payload, inseg->p->len);
            }

            pbuf_free(inseg->p);
        }
        break;

        case L2CAP_CID_SIGNALING_LE:
            pbuf_free(inseg->p);
            break;

        case L2CAP_CID_SECURITY_MANAGER_PROTOCOL:
        {

            pbuf_header(inseg->p, -L2CAP_HDR_LEN);
            //sm_packet_handler(SM_DATA_PACKET, *((uint16*)(&bdaddr->addr[0]+6)), inseg->p->payload, inseg->p->len);
            if(security_protocol_packet_handler)
            {
                security_protocol_packet_handler(SM_DATA_PACKET, *((uint16*)(&bdaddr->addr[0]+6)), inseg->p->payload, inseg->p->len);
            }
            pbuf_free(inseg->p);
        }
        break;

        default:
            if(inseg->l2caphdr->cid < 0x0040 || inseg->pcb == NULL)
            {
                /* Reserved for specific L2CAP functions or channel does not exist */
                /* Alloc size of reason in cmd rej */
                if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+4, PBUF_RAM)) != NULL)
                {
                    ((uint16 *)data->payload)[0] = L2CAP_INVALID_CID;
                    ((uint16 *)data->payload)[1] = inseg->l2caphdr->cid;
                    ((uint16 *)data->payload)[2] = L2CAP_NULL_CID;

                    ret = l2cap_signal(NULL, L2CAP_CMD_REJ, l2cap_next_sigid(), bdaddr, data);
                }
                pbuf_free(inseg->p);
                break;
            }

            pbuf_header(inseg->p, -L2CAP_HDR_LEN);

            /* Forward packet to higher layer */
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_input: Forward packet to higher layer\n"));
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_input: Remote BD address: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",
                                      inseg->pcb->remote_bdaddr.addr[5],
                                      inseg->pcb->remote_bdaddr.addr[4],
                                      inseg->pcb->remote_bdaddr.addr[3],
                                      inseg->pcb->remote_bdaddr.addr[2],
                                      inseg->pcb->remote_bdaddr.addr[1],
                                      inseg->pcb->remote_bdaddr.addr[0]));

            L2CA_ACTION_RECV(inseg->pcb,inseg->p,ERR_OK,ret);
            break;
    }

    /* Remove input segment */
    L2CAP_SEG_RMV(&(l2cap_insegs), inseg);
    lwbt_memp_free(MEMP_L2CAP_SEG, inseg);

}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_cid_alloc():
 *
 * Allocates a channel identifier (CID). They are local names representing a logical
 * channel endpoint on the device.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
static uint16 l2cap_cid_alloc(void)
{
    uint16 cid;
    struct l2cap_pcb *pcb;

    for (cid = L2CAP_MIN_CID+cid_next; cid < L2CAP_MAX_CID; ++cid)
    {
        for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
        {
            if(pcb->scid == cid)
            {
                break;
            }
        }
        if(pcb == NULL)
        {
            cid_next++;
            if((L2CAP_MIN_CID+cid_next) == L2CAP_MAX_CID)
            {
                cid_next = 0;
            }

            return cid;
        }
    }
    return 0;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_new():
 *
 * Creates a new L2CAP protocol control block but doesn't place it on
 * any of the L2CAP PCB lists.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
struct l2cap_pcb *l2cap_new(void)
{
    struct l2cap_pcb *pcb;

    pcb = lwbt_memp_malloc(MEMP_L2CAP_PCB);
    if(pcb != NULL)
    {
        memset(pcb, 0, sizeof(struct l2cap_pcb));
        pcb->state = L2CAP_CLOSED;

        /* Initialize configuration parameter options with default values */

        /* Maximum Transmission Unit */
        pcb->cfg.inmtu = L2CAP_MTU; /* The MTU that this implementation support */
        //pcb->cfg.inmtu = L2CAP_CFG_DEFAULT_INMTU;
        pcb->cfg.outmtu = 672; /* Default MTU. Two Baseband DH5 packets minus the Baseband ACL headers and
                      L2CAP header. This can be set here since we will never send any signals
                      larger than the L2CAP sig MTU (48 bytes) before L2CAP has been configured
                   */

        /* Flush Timeout */
        pcb->cfg.influshto = 0xFFFF;
        pcb->cfg.outflushto = 0xFFFF;

        pcb->cfg.cfgto = L2CAP_CFG_TO; /* Maximum time before terminating a negotiation.
                          Cfg shall not last more than 120s */
        pcb->cfg.opt = NULL;
        return pcb;
    }
    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_new: Could not allocate memory for pcb\n"));
    return NULL;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_close():
 *
 * Closes the L2CAP protocol control block.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
err_t l2cap_close(struct l2cap_pcb *pcb)
{
    struct l2cap_sig *tmpsig;

    if(pcb->state == L2CAP_LISTEN)
    {
        L2CAP_RMV((struct l2cap_pcb **)&l2cap_listen_pcbs, pcb);
        lwbt_memp_free(MEMP_L2CAP_PCB_LISTEN, pcb);
    }
    else
    {
        L2CAP_RMV(&l2cap_active_pcbs, pcb);
        /* Free any unresponded signals */
        while(pcb->unrsp_sigs != NULL)
        {
            tmpsig = pcb->unrsp_sigs;
            pcb->unrsp_sigs = pcb->unrsp_sigs->next;
            if(tmpsig->p != NULL)
            {
                pbuf_free(tmpsig->p);
            }
            lwbt_memp_free(MEMP_L2CAP_SIG, tmpsig);
        }

        lwbt_memp_free(MEMP_L2CAP_PCB, pcb);
    }
    pcb = NULL;
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_reset_all():
 *
 * Closes all active and listening L2CAP protocol control blocks.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void l2cap_reset_all(void)
{
    struct l2cap_pcb *pcb, *tpcb;
    struct l2cap_pcb_listen *lpcb, *tlpcb;
    struct l2cap_seg *seg, *tseg;

    for(pcb = l2cap_active_pcbs; pcb != NULL;)
    {
        tpcb = pcb->next;
        l2cap_close(pcb);
        pcb = tpcb;
    }

    for(lpcb = l2cap_listen_pcbs; lpcb != NULL;)
    {
        tlpcb = lpcb->next;
        l2cap_close((struct l2cap_pcb *)lpcb);
        lpcb = tlpcb;
    }

    for(seg = l2cap_insegs; seg != NULL;)
    {
        tseg = seg->next;
        L2CAP_SEG_RMV(&(l2cap_insegs), seg);
        lwbt_memp_free(MEMP_L2CAP_SEG, seg);
        seg = tseg;
    }

    l2cap_init();
}
/*-----------------------------------------------------------------------------------*/
/* L2CAP to L2CAP signalling events
 */
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_signal():
 *
 * Assembles the signalling packet and passes it to the lower layer.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2cap_signal(struct l2cap_pcb *pcb, uint8 code, uint16 ursp_id, struct bd_addr *remote_bdaddr,
                   struct pbuf *data)
{
    struct l2cap_sig *sig;
    struct l2cap_sig_hdr *sighdr;
    struct l2cap_hdr *hdr;
    err_t ret;

    /* Alloc a new signal */
    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_signal: Allocate memory for l2cap_sig. Code = 0x%x\n", code));
    if((sig = lwbt_memp_malloc(MEMP_L2CAP_SIG)) == NULL)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_signal: could not allocate memory for l2cap_sig\n"));
        return ERR_MEM;
    }

    /* Alloc a pbuf for signal */

#if 0
    if((sig->p = pbuf_alloc(PBUF_BT, L2CAP_HDR_LEN+L2CAP_SIGHDR_LEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_signal: could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }



    /* Setup signal header and leave room for l2cap hdr */
    sighdr = (struct l2cap_sig_hdr *)(((uint8 *)sig->p->payload)+L2CAP_HDR_LEN);
#endif

    /* Chain data to signal and set length of signal data */
    if(data == NULL)
    {

        if((sig->p = pbuf_alloc(PBUF_RAW, L2CAP_HDR_LEN+L2CAP_SIGHDR_LEN, PBUF_RAM)) == NULL)
        {
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_SERIOUS_, ("l2cap_signal: could not allocate memory for pbuf\n"));
            return ERR_MEM;
        }

        /* Setup signal header and leave room for l2cap hdr */
        sighdr = (struct l2cap_sig_hdr *)(((uint8 *)sig->p->payload)+L2CAP_HDR_LEN);
        sighdr->len = 0;
    }
    else
    {
        //pbuf_chain(sig->p, data);

        pbuf_header(data,L2CAP_HDR_LEN+L2CAP_SIGHDR_LEN);

        sig->p = data;

        sighdr = (struct l2cap_sig_hdr *)(((uint8 *)sig->p->payload)+L2CAP_HDR_LEN);
        //pbuf_copy(sig->p, data);
        sighdr->len = data->tot_len- (L2CAP_HDR_LEN+L2CAP_SIGHDR_LEN);
        //pbuf_free(data);
    }

    sighdr->code = code;

    if(sighdr->code % 2)
    {
        /* If odd this is a resp/rej signal */
        sig->sigid = ursp_id; /* Get id */
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_signal: Sending response/reject signal with id = %d code = %d\n",
                                  sig->sigid, sighdr->code));
    }
    else
    {
        sig->sigid = l2cap_next_sigid(); /* Alloc id */
        sig->rtx = L2CAP_RTX; /* Set Response Timeout Expired timer (in seconds)
                     should be at least as large as the BB flush timeout */
        sig->nrtx = L2CAP_MAXRTX; /* Set max number of retransmissions */
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_signal: Sending request signal with id = %d code = %d\n",
                                  sig->sigid, sighdr->code));
    }
    sighdr->id = sig->sigid; /* Set id */

    /* Set up L2CAP hdr */
    hdr = sig->p->payload;
    hdr->len = sig->p->tot_len - L2CAP_HDR_LEN;
    hdr->cid = L2CAP_SIG_CID; /* 0x0001 */

    ret = l2cap_write(remote_bdaddr, sig->p, sig->p->tot_len); /* Send peer L2CAP signal */

    /* Put signal on unresponded list if it's a request signal, else deallocate it */
    if(ret == ERR_OK && (sighdr->code % 2) == 0)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_signal: Registering sent request signal with id = %d code = %d\n",
                                  sig->sigid, sighdr->code));
        L2CAP_SIG_REG(&(pcb->unrsp_sigs), sig);
    }
    else
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_signal: Deallocating sent response/reject signal with id = %d code = %d\n",
                                  sig->sigid, sighdr->code));
        pbuf_free(sig->p);
        sig->p = NULL;
        lwbt_memp_free(MEMP_L2CAP_SIG, sig);
    }

    return ret;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_rexmit_signal():
 *
 * Called by the l2cap timer. Retransmitts a signal.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2cap_rexmit_signal(struct l2cap_pcb *pcb, struct l2cap_sig *sig)
{
    err_t ret;

    /* Set up L2CAP hdr */
    ret = l2cap_write(&(pcb->remote_bdaddr), sig->p, sig->p->tot_len); /* Send peer L2CAP signal */

    return ret;
}
/*-----------------------------------------------------------------------------------*/
/* Upper-Layer to L2CAP signaling events
 */
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/*
 * l2ca_connect_req():
 *
 * Initiates the sending of a connect request message. Requests the creation of a
 * channel representing a logicalconnection to a physical address. Input parameters
 * are the target protocol(PSM) and remote devices 48-bit address (BD_ADDR). Also
 * specify the function to be called when a confirm has been received.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2ca_connect_req(struct l2cap_pcb *pcb, struct bd_addr *bdaddr, uint16 psm,
                       uint8 role_switch, err_t (* l2ca_connect_cfm)(void *arg, struct l2cap_pcb *lpcb,
                               uint16 result, uint16 status))
{
    err_t ret;
    struct pbuf *data;

    if(bdaddr != NULL)
    {
        bd_addr_set(&(pcb->remote_bdaddr),bdaddr);
    }
    else
    {
        return ERR_VAL;
    }

    pcb->psm = psm;
    pcb->l2ca_connect_cfm = l2ca_connect_cfm;
    pcb->scid = l2cap_cid_alloc();
    LWIP_ASSERT("l2ca_connect_req: out of CIDs\n", pcb->scid != 0);

    pcb->cfg.l2capcfg |= L2CAP_CFG_IR; /* We are the initiator of this connection */

    if(!lp_is_connected(bdaddr))
    {
        ret = lp_connect_req(bdaddr, role_switch); /* Create ACL link w pcb state == CLOSED */
    }
    else
    {
        if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_REQ_SIZE, PBUF_RAM)) == NULL)
        {
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_connect_req: Could not allocate memory for pbuf\n"));
            return ERR_MEM;
        }
        ((uint16 *)data->payload)[0] = psm;
        ((uint16 *)data->payload)[1] = pcb->scid;
        ret = l2cap_signal(pcb, L2CAP_CONN_REQ, 0, &(pcb->remote_bdaddr), data); /* Send l2cap_conn_req signal */

        pcb->state = W4_L2CAP_CONNECT_RSP;
    }

    L2CAP_REG(&l2cap_active_pcbs, pcb);
    return ret;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2ca_config_req():
 *
 * Requests the initial configuration (or reconfiguration) of a channel to a new set
 * of channel parameters. Input parameters are the local CID endpoint, new incoming
 * receivable MTU (InMTU), new outgoing flow specification, and flush and link
 * timeouts. Also specify the function to be called when a confirm has been received.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2ca_config_req(struct l2cap_pcb *pcb, int flag)
{
    struct pbuf *p, *q;
    struct l2cap_cfgopt_hdr *opthdr;
    err_t ret;
    switch(pcb->state)
    {
        case L2CAP_OPEN:
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_config_req: state = L2CAP_OPEN. Suspend transmission\n"));
            /* Note: Application should have suspended data transmission, otherwise outgoing data will be
            dropped */
            pcb->state = L2CAP_CONFIG;
        /* Fallthrough */
        case L2CAP_CONFIG:
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_config_req: state = L2CAP_CONFIG\n"));

            if((p = pbuf_alloc(PBUF_RAW, L2CAP_CFG_REQ_SIZE, PBUF_RAM)) == NULL)
            {
                LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_config_req: Could not allocate memory for pbuf\n"));
                return ERR_MEM;
            }

            /* Assemble config request packet. Only options that has to be changed will be
               sent */
            ((uint16 *)p->payload)[0] = pcb->dcid;
            /* In this implementation we do not send multiple cmds in one
               signal packet. Therefore we will never send a config_req packet
               that will cause the signal to be larger than the minimum L2CAP MTU
               48 bytes. Hence, this flag will always be cleared */
            ((uint16 *)p->payload)[1] = 0;

            /* Add MTU and out flush timeout to cfg packet if not default value. QoS (Best effort) is always
               set to default and can be skipped */
            if((pcb->cfg.inmtu != L2CAP_CFG_DEFAULT_INMTU) || (flag & L2CAP_USER_CFG_MTU))
            {
                if((q = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + L2CAP_MTU_LEN, PBUF_RAM)) == NULL)
                {
                    pbuf_free(p);
                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_config_req: Could not allocate memory for pbuf\n"));
                    return ERR_MEM;
                }
                opthdr = q->payload;
                opthdr->type = L2CAP_CFG_MTU;
                opthdr->len = L2CAP_MTU_LEN;
                ((uint16 *)q->payload)[1] = pcb->cfg.inmtu;
                //pbuf_chain(p, q);
                pbuf_copy(p, q);
                pbuf_free(q);
            }

            if((L2CAP_OUT_FLUSHTO != L2CAP_CFG_DEFAULT_OUTFLUSHTO) || (flag & L2CAP_USER_CFG_FLUSH_TIMEOUT))
            {
                if((q = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + L2CAP_FLUSHTO_LEN, PBUF_RAM)) == NULL)
                {
                    pbuf_free(p);
                    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_config_req: Could not allocate memory for pbuf\n"));
                    return ERR_MEM;
                }
                opthdr = q->payload;
                opthdr->type = L2CAP_FLUSHTO;
                opthdr->len = L2CAP_FLUSHTO_LEN;
                ((uint16 *)q->payload)[1] = pcb->cfg.outflushto = L2CAP_OUT_FLUSHTO;
                //pbuf_chain(p, q);
                pbuf_copy(p, q);
                pbuf_free(q);
            }

            /* Send config request signal */
            ret = l2cap_signal(pcb, L2CAP_CFG_REQ, 0, &(pcb->remote_bdaddr), p);
            break;
        default:
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_config_req: state = L2CAP_?. Invalid state\n"));
            return ERR_CONN; /* Invalid state. Connection is not in OPEN or CONFIG state */
    }
    return ret;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2ca_disconnect_req():
 *
 * Requests the disconnection of the channel. Also specify the function to be called
 * when a confirm is received
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2ca_disconnect_req(struct l2cap_pcb *pcb, err_t (* l2ca_disconnect_cfm)(void *arg, struct l2cap_pcb *pcb))
{
    struct pbuf *data;
    err_t ret;
    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2ca_disconnect_req"));
    if(pcb->state == L2CAP_OPEN || pcb->state == L2CAP_CONFIG)
    {
        if((data = pbuf_alloc(PBUF_RAW, L2CAP_DISCONN_REQ_SIZE, PBUF_RAM)) == NULL)
        {
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_disconnect_req: Could not allocate memory for pbuf\n"));
            return ERR_MEM;
        }
        pcb->l2ca_disconnect_cfm = l2ca_disconnect_cfm;

        ((uint16 *)data->payload)[0] = pcb->dcid;
        ((uint16 *)data->payload)[1] = pcb->scid;

        ret = l2cap_signal(pcb, L2CAP_DISCONN_REQ, 0, &(pcb->remote_bdaddr), data);

        if(ret == ERR_OK)
        {
            pcb->state = W4_L2CAP_DISCONNECT_RSP;
        }
    }
    else
    {
        return ERR_CONN; /* Signal not supported in this state */
    }

    return ret;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2ca_datawrite():
 *
 * Transfers data across the channel.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t _l2ca_datawrite(struct l2cap_pcb *pcb, struct pbuf *p, void (*func)(void))
{
    err_t ret;
    struct l2cap_hdr *l2caphdr;
    struct pbuf *q;

    if(pcb->state != L2CAP_OPEN)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_datawrite: State != L2CAP_OPEN. Dropping data\n"));
        return ERR_CONN;
    }
    q = p;
    pbuf_ref(q);
    pbuf_header(q,L2CAP_HDR_LEN);
    l2caphdr = q->payload;
    l2caphdr->cid = pcb->dcid;

    /* If length of the data exceeds the OutMTU then only the first OutMTU bytes are sent */
    if(p->tot_len > pcb->cfg.outmtu)
    {
        /* Send peer L2CAP data */
        l2caphdr->len = pcb->cfg.outmtu;
        if((ret = _l2cap_write(&(pcb->remote_bdaddr), q, pcb->cfg.outmtu + L2CAP_HDR_LEN, func)) == ERR_OK)
        {
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_datawrite: Length of data exceeds the OutMTU p->tot_len = %d\n", p->tot_len));
            ret = ERR_BUF; /* Length of data exceeds the OutMTU */
        }
    }
    else
    {
        /* Send peer L2CAP data */
        l2caphdr->len = p->tot_len-4;
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_DUMP_, ("l2cap_datawrite: q->tot_len = %d\n", q->tot_len));
        ret = _l2cap_write(&(pcb->remote_bdaddr), q, q->tot_len, func);
    }

    /* Free L2CAP header. Higher layers will handle rest of packet */
    //p = pbuf_dechain(q);
    pbuf_free(q);

    return ret;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2ca_ping():
 *
 * Sends an empty L2CAP echo request message. Also specify the function that should
 * be called when a L2CAP echo reply has been received.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2ca_ping(struct bd_addr *bdaddr, struct l2cap_pcb *tpcb,
                err_t (* l2ca_pong)(void *arg, struct l2cap_pcb *pcb, uint8 result))
{
    err_t ret;

    if(!lp_is_connected(bdaddr))
    {
        return ERR_CONN;
    }

    bd_addr_set(&(tpcb->remote_bdaddr), bdaddr);
    tpcb->l2ca_pong = l2ca_pong;

    L2CAP_REG(&l2cap_active_pcbs, tpcb);

    ret = l2cap_signal(tpcb, L2CAP_ECHO_REQ, 0, &(tpcb->remote_bdaddr), NULL); /* Send l2cap_echo_req signal */

    return ret;
}
/*-----------------------------------------------------------------------------------*/
/* Lower-Layer to L2CAP signaling events
 */
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/*
 * lp_connect_cfm():
 *
 * Confirms the request to establish a lower layer (Baseband) connection.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void lp_connect_cfm(struct bd_addr *bdaddr, uint8 encrypt_mode, err_t err)
{
    struct l2cap_pcb *pcb;
    struct pbuf *data;
    struct hci_link *link;
    err_t ret;

	//DEBUG("lp_connect_cfm\n");
    for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        if(bd_addr_cmp(&(pcb->remote_bdaddr), bdaddr))
        {
            break;
        }
    }
    if(pcb == NULL)
    {
        /* Silently discard */
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("lp_connect_cfm: Silently discard\n"));
    }
    else
    {
        if(err == ERR_OK)
        {
            //link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);
            //hci_Authentication_Requested(link->conhdl);
            //hci_set_connection_encryption(link->conhdl, 1);
            //hci_read_remote_extended_features(link->conhdl);
            pcb->encrypt = encrypt_mode;
            /* Send l2cap_conn_req signal if no error */
            if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_REQ_SIZE, PBUF_RAM)) != NULL)
            {
                ((uint16 *)data->payload)[0] = pcb->psm;
                ((uint16 *)data->payload)[1] = pcb->scid;
                if((ret = l2cap_signal(pcb, L2CAP_CONN_REQ, 0, &(pcb->remote_bdaddr), data)) == ERR_OK)
                {
                    pcb->state = W4_L2CAP_CONNECT_RSP;
                }
                else
                {
                    L2CA_ACTION_CONN_CFM(pcb,L2CAP_CONN_REF_RES,0x0000,ret); /* No resources available? */
                }
                LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("lp_connect_cfm: l2cap_conn_req signal sent. err = %d\nPSM = 0x%x\nscid = 0x%x\nencrypt mode = 0x%x\n", err, pcb->psm, pcb->scid, pcb->encrypt));
            }
            else
            {
                LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("lp_connect_cfm: No resources available\n"));
                L2CA_ACTION_CONN_CFM(pcb,L2CAP_CONN_REF_RES,0x0000,ret); /* No resources available */
            }
        }
        else
        {
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("lp_connect_cfm: Connection falied\n"));
            L2CA_ACTION_CONN_CFM(pcb,L2CAP_CONN_REF_RES,0x0000,ret); /* No resources available */
        }
    }
}
/*-----------------------------------------------------------------------------------*/
/*
 * lp_connect_ind():
 *
 * Indicates the lower protocol has successfully established a connection.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void lp_connect_ind(struct bd_addr *bdaddr)
{
    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("lp_connect_ind\n"));

}
/*-----------------------------------------------------------------------------------*/
/*
 * lp_disconnect_ind():
 *
 * Indicates the lower protocol (Baseband) has been shut down by LMP commands or a
 * timeout event..
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void lp_disconnect_ind(struct bd_addr *bdaddr)
{
    struct l2cap_pcb *pcb, *tpcb;
    err_t ret;
    LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_,("lp_disconnect_ind"));
    for(pcb = l2cap_active_pcbs; pcb != NULL;)
    {
        tpcb = pcb->next;
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("lp_disconnect_ind: Find a pcb with a matching Bluetooth address\n"));
        /* All PCBs with matching Bluetooth address have been disconnected */
        if(bd_addr_cmp(&(pcb->remote_bdaddr), bdaddr))
        {
            // && pcb->state != L2CAP_CLOSED) {
            pcb->state = L2CAP_CLOSED;
            LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("lp_disconnect_ind: Notify application\n"));
            L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);
        }
        pcb = tpcb;
    }
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_next_sigid():
 *
 * Issues a signal identifier that helps matching a request with the reply.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
uint8 l2cap_next_sigid(void)
{
    ++sigid_nxt;
    if(sigid_nxt == 0)
    {
        sigid_nxt = 1;
    }
    return sigid_nxt;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_arg():
 *
 * Used to specify the argument that should be passed callback functions.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void l2cap_arg(struct l2cap_pcb *pcb, void *arg)
{
    pcb->callback_arg = arg;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_connect_ind():
 *
 * Set the state of the connection to be LISTEN, which means that it is able to accept
 * incoming connections. The protocol control block is reallocated in order to consume
 * less memory. Setting the connection to LISTEN is an irreversible process. Also
 * specify the function that should be called when the channel has received a
 * connection request.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
#if LWBT_LAP
err_t l2cap_connect_ind(struct l2cap_pcb *npcb, uint8 psm,
                        err_t (* l2ca_connect_ind)(void *arg, struct l2cap_pcb *pcb, err_t err))
{
    struct l2cap_pcb_listen *lpcb;
    lpcb = lwbt_memp_malloc(MEMP_L2CAP_PCB_LISTEN);
    if(lpcb == NULL)
    {
        LWBT_DEBUGF(L2CAP_DEBUG, _DBG_INFO_, ("l2cap_connect_ind: Could not allocate memory for lpcb\n"));
        return ERR_MEM;
    }
    lpcb->psm = psm;
    lpcb->l2ca_connect_ind = l2ca_connect_ind;
    lpcb->state = L2CAP_LISTEN;
//    lpcb->callback_arg = npcb->callback_arg;  by wp 20130322
//    lwbt_memp_free(MEMP_L2CAP_PCB, npcb);
    lpcb->callback_arg = NULL;
    L2CAP_REG((struct l2cap_pcb **)&l2cap_listen_pcbs, (struct l2cap_pcb *)lpcb);
    return ERR_OK;
}
#endif /* LWBT_LAP */
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_disconnect_ind():
 *
 * Used to specify the a function to be called when a disconnection request has been
 * received from a remote device or the remote device has been disconnected because it
 * has failed to respond to a signalling request.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
void l2cap_disconnect_ind(struct l2cap_pcb *pcb,
                          err_t (* l2ca_disconnect_ind)(void *arg, struct l2cap_pcb *newpcb, err_t err))
{
    pcb->l2ca_disconnect_ind = l2ca_disconnect_ind;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_timeout_ind():
 *
 * Used to specify the function to be called when RTX or ERTX timer has expired.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void l2cap_timeout_ind(struct l2cap_pcb *pcb,
                       err_t (* l2ca_timeout_ind)(void *arg, struct l2cap_pcb *newpcb, err_t err))
{
    pcb->l2ca_timeout_ind = l2ca_timeout_ind;
}
/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_recv():
 *
 * Used to specify the function that should be called when a L2CAP connection receives
 * data.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void l2cap_recv(struct l2cap_pcb *pcb,
                err_t (* l2ca_recv)(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err))
{
    pcb->l2ca_recv = l2ca_recv;
}


/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_register_event_handler():
 *
 * Used to specify the function that should be called when a L2CAP connection receives
 * data.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void l2cap_register_event_handler(struct l2cap_pcb *pcb,
                                  void (* l2ca_event_handler)(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size))
{
    pcb->l2ca_event_handler = l2ca_event_handler;
}



#ifdef HAVE_BLE
// Request LE connection parameter update
int l2cap_le_request_connection_parameter_update(uint16_t handle, uint16_t interval_min, uint16_t interval_max, uint16_t slave_latency, uint16_t timeout_multiplier)
{

    struct hci_acl_hdr *aclhdr;
    uint timeout;
    struct pbuf *p;
    if(!hci_can_send_packet_now())
    {
        return ERR_BUF;
    }

    if((p = pbuf_alloc(PBUF_RAW, 12+9, PBUF_RAM)) == NULL)
    {
        LWIP_DEBUGF(HCI_DEBUG, ("hci_write_scan_enable: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    ((uint8*)p->payload)[0] = HCI_ACL_DATA_PACKET;
    aclhdr = (void *)(((uint8*)p->payload)+1);
    aclhdr->conhdl_pb_bc = handle; /* Received from connection complete event */
    aclhdr->conhdl_pb_bc |= L2CAP_ACL_START << 12; /* Packet boundary flag */
    aclhdr->conhdl_pb_bc &= 0x3FFF; /* Point-to-point */
    aclhdr->len = 12+4;
    ((uint8 *)p->payload)[5] = (12) & 0xFF;
    ((uint8 *)p->payload)[6] = (12>>8) & 0xFF;
    ((uint8 *)p->payload)[7] = 5 & 0xFF; // chanle 5
    ((uint8 *)p->payload)[8] = (5>> 8) & 0xFF;


    ((uint8 *)p->payload)[9] = L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST;
    ((uint8 *)p->payload)[10] = 1;  //id
    ((uint8 *)p->payload)[11] = 8 & 0xFF;       // sig len 8
    ((uint8 *)p->payload)[12] = (8>> 8) & 0xFF; // sig len 8

    ((uint8 *)p->payload)[13] = interval_min & 0xFF;
    ((uint8 *)p->payload)[14] = (interval_min>> 8) & 0xFF;

    ((uint8 *)p->payload)[15] = interval_max & 0xFF;
    ((uint8 *)p->payload)[16] = (interval_max>> 8) & 0xFF;

    ((uint8 *)p->payload)[17] = slave_latency & 0xFF;
    ((uint8 *)p->payload)[18] = (slave_latency>> 8) & 0xFF;

    ((uint8 *)p->payload)[19] = timeout_multiplier & 0xFF;
    ((uint8 *)p->payload)[20] = (timeout_multiplier>> 8) & 0xFF;

    _phybusif_output(p, p->len, NULL);

    hci_buf_subtract();

    /* Free ACL header. Upper layers will handle rest of packet */
    //p = pbuf_dechain(q);
    pbuf_free(p);
    return ERR_OK;
}
#endif
#endif
/*-----------------------------------------------------------------------------------*/
