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
/* hci.c
 *
 * Implementation of the Host Controller Interface (HCI). A command interface to the
 * baseband controller and link manager, and gives access to hardware status and
 * control registers.
 *
 */
/*-----------------------------------------------------------------------------------*/
#include "lwbt.h"

#include "SysInclude.h"
#include "lwbtopts.h"
#include "phybusif.h"
#include "hci.h"
#include "l2cap.h"
#include "lwbt_memp.h"
#include "lwbtdebug.h"
#include "lwbterr.h"
//#include "BlueToothWin.h"
//#include "Btbuffer.h"

#ifdef _BLUETOOTH_

//#define  HCI_TEST
/* The HCI LINK lists. */
_ATTR_LWBT_BSS_ struct hci_link *hci_active_links;  /* List of all active HCI LINKs */
_ATTR_LWBT_BSS_ struct hci_link *hci_tmp_link;
_ATTR_LWBT_BSS_ struct hci_pcb *pcb;
/* The number of data already sent */
_ATTR_LWBT_BSS_ int hci_data_send;
/* The number of data have been acked */
_ATTR_LWBT_BSS_ int hci_data_acked;
_ATTR_LWBT_UARTIF_DATA_ uint8 hci_io_capability = 0x03;
//_ATTR_LWBT_DATA_ uint8  hci_scolink_number = 0;
hci_flush_command(uint16 conhdl);
uint8* get_link_key(struct bd_addr *bdaddr);
struct hci_link *hci_get_link_use_conhdl(uint16 conhdl);
int ENCRYPTION_CHANGE = 0;
/*-----------------------------------------------------------------------------------*/
/*
 * hci_init():
 *
 * Initializes the HCI layer.
 */
/*-----------------------------------------------------------------------------------*/
extern int h5_unack_len();
_ATTR_LWBT_UARTIF_CODE_
void hci_tmr(void)
{
    uint systick;
    systick = GetSysTick();
#if 0
    if (systick % 500 == 0)
    {
        printf("hci_num_acl=%d, hci_data_send=%d, hci_data_acked=%d, h5_unack_len=%d\n",
            pcb->hc_num_acl, hci_data_send, hci_data_acked, h5_unack_len());
    }
#endif
    if(pcb != NULL)
    {

        if(pcb->flush_timer_enable)
        {
            if(systick > pcb->flush_timer_timeout + 20)
            {
                pcb->flush_timer_enable = 0;
                pcb->flush_timer_timeout = 0;
                HciServeIsrDisable();
                pcb->hc_num_acl = pcb->hc_num_acl_total;
                HciServeIsrEnable();

                //hci_flush_command(pcb->flush_handle);

#if HCI_FLOW_QUEUEING
                {

                    struct hci_link *link;
                    PBUF *q;
                    link = hci_get_link_use_conhdl(pcb->flush_handle);
                    if(link != NULL)
                    {
                        q = link->p;
                        /* Queued packet present? */
                        if (q != NULL)
                        {
                            /* NULL attached buffer immediately */
                            link->p = NULL;
                            LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Sending queued packet.\n"));
                            /* Send the queued packet */
                            lp_acl_write(&link->bdaddr, q, link->len, link->pb, link->func);
                            /* Free the queued packet */
                            pbuf_free(q);
                        }
                    }
                }

#endif
            }
        }


    }
}


_ATTR_LWBT_INIT_CODE_
void hci_set_io_capability(uint8 io_capability)
{
    hci_io_capability = io_capability;
}


_ATTR_LWBT_INIT_CODE_
int hci_init(void)
{
    if((pcb = lwbt_memp_malloc(MEMP_HCI_PCB)) == NULL)
    {

        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_init: Could not allocate memory for pcb\n"));
        return ERR_MEM;
    }

    memset(pcb, 0, sizeof(struct hci_pcb));

    /* Clear globals */
    hci_active_links = NULL;
    hci_tmp_link     = NULL;
    //hci_scolink_number = 0;
    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
void hci_deinit(void)
{
    pcb = NULL;
    hci_active_links = NULL;
    hci_tmp_link     = NULL;
    //hci_scolink_number = 0;
}


_ATTR_LWBT_CODE_
struct hci_inq_res  * hci_updata_devName(struct bd_addr *bdaddr, uint8 * name)
{

    struct hci_inq_res  *res = NULL;
    struct hci_inq_res *tmpres = NULL;
    struct hci_inq_res *tmpres1 = NULL;
    for(res = pcb->iresAll; res != NULL; res = res->next)
    {
        if(bd_addr_cmp(&(res->bdaddr), bdaddr))
        {
            memcpy(res->name, name, 32);
            HCI_RMV_COMMON(&pcb->iresAll,res, tmpres1 );
            HCI_REG(&(pcb->iresWithName), res);
            break;
        }
    }

    return pcb->iresWithName;

}

_ATTR_LWBT_UARTIF_CODE_
void hci_clean_scan_result(void)
{
    struct hci_inq_res *ires, *tires;
    if(pcb != NULL)
    {
        for(ires = pcb->ires; ires != NULL;)
        {
            tires = ires->next;
            lwbt_memp_free(MEMP_HCI_INQ, ires);
            ires = tires;
        }
    }
    pcb->ires = NULL;

    if(pcb != NULL)
    {
        for(ires = pcb->iresAll; ires != NULL;)
        {
            tires = ires->next;
            lwbt_memp_free(MEMP_HCI_INQ, ires);
            ires = tires;
        }
    }
    if(pcb != NULL)
    {
        for(ires = pcb->iresWithName; ires != NULL;)
        {
            tires = ires->next;
            lwbt_memp_free(MEMP_HCI_INQ, ires);
            ires = tires;
        }
    }

    pcb->iresWithName = NULL;

    pcb->iresAll = NULL;


}





/*-----------------------------------------------------------------------------------*/
/*
 * hci_new():
 *
 * Creates a new HCI link control block
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
struct hci_link * hci_new(void)
{
    struct hci_link *link;

    link = lwbt_memp_malloc(MEMP_HCI_LINK);
    if(link != NULL)
    {
        memset(link, 0, sizeof(struct hci_link));
        return link;
    }
    LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_new: Could not allocate memory for link\n"));
    return NULL;
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_close():
 *
 * Close the link control block.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
int hci_close(struct hci_link *link)
{
#if RFCOMM_FLOW_QUEUEING
    if(link->p != NULL)
    {
        pbuf_free(link->p);
    }
#endif
    HCI_RMV(&(hci_active_links), link);
    lwbt_memp_free(MEMP_HCI_LINK, link);
    link = NULL;
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_reset_all():
 *
 * Closes all active link control blocks.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_reset_all(void)
{
    struct hci_link *link, *tlink;
    struct hci_inq_res *ires, *tires;

    for(link = hci_active_links; link != NULL;)
    {
        tlink = link->next;
        hci_close(link);
        link = tlink;
    }
    hci_active_links = NULL;

#if 0
    if(pcb != NULL)
    {
        for(ires = pcb->ires; ires != NULL;)
        {
            tires = ires->next;
            lwbt_memp_free(MEMP_HCI_INQ, ires);
            ires = tires;
        }
    }
#endif

    hci_clean_scan_result();
    lwbt_memp_free(MEMP_HCI_PCB, pcb);

    hci_init();
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_arg():
 *
 * Used to specify the argument that should be passed callback
 * functions.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_arg(void *arg)
{
    pcb->callback_arg = arg;
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_cmd_complete():
 *
 * Used to specify the function that should be called when HCI has received a
 * command complete event.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_cmd_complete(int (* cmd_complete)(void *arg, struct hci_pcb *pcb, uint8 ogf,
                      uint8 ocf, uint8 result))
{
    pcb->cmd_complete = cmd_complete;
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_pin_req():
 *
 * Used to specify the function that should be called when HCI has received a
 * PIN code request event.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_pin_req(int (* pin_req)(void *arg, struct bd_addr *bdaddr))
{
    pcb->pin_req = pin_req;
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_link_key_not():
 *
 * Used to specify the function that should be called when HCI has received a
 * link key notification event.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_link_key_not(int (* link_key_not)(void *arg, struct bd_addr *bdaddr, uint8 *key))
{
    pcb->link_key_not = link_key_not;
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_connection_complete():
 *
 * Used to specify the function that should be called when HCI has received a
 * connection complete event.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_connection_complete(int (* conn_complete)(void *arg, struct bd_addr *bdaddr))
{
    pcb->conn_complete = conn_complete;
}

/*-----------------------------------------------------------------------------------*/
/*
 * hci_link_key_request():
 *
 * Used to specify the function that should be called when HCI has received a
 * link key request event.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_link_key_request(int (* link_key_request)(void *arg, struct bd_addr *bdaddr))
{
    pcb->link_key_request= link_key_request;
}

_ATTR_LWBT_INIT_CODE_
void hci_sco_connect_notify(void (* sco_connected)(void))
{
    pcb->sco_connected = sco_connected;
}

_ATTR_LWBT_INIT_CODE_
void hci_sco_disconnect_notify ( void(* sco_disconnect)(void))
{
    pcb->sco_disconnect = sco_disconnect;
}


_ATTR_LWBT_INIT_CODE_
void hci_acl_connected_notify ( void(* acl_connected)(uint8 result, struct bd_addr *bdaddr))
{
    pcb->acl_connected = acl_connected;
}

_ATTR_LWBT_INIT_CODE_
void hci_acl_disconnect_notify ( void(* acl_disconnect)(struct bd_addr *bdaddr, uint8 reason))
{
    pcb->acl_disconnect = acl_disconnect;
}


_ATTR_LWBT_INIT_CODE_
void hci_sco_recv( void(* sco_recv)(unsigned char *data, unsigned int len))
{
    pcb->sco_recv = sco_recv;
}


_ATTR_LWBT_INIT_CODE_
void hci_req_remote_name_complete(int (* req_remote_name_complete)(void *arg, struct hci_pcb *pcb,uint8 scan_status, struct bd_addr *bdaddr, uint8 *name, struct hci_inq_res *ires_header))
{
    pcb->req_remote_name_complete = req_remote_name_complete;
}
_ATTR_LWBT_INIT_CODE_
void hci_user_confirmation_request(void (* user_confirmation_request)(struct bd_addr *bdaddr,uint32 Numeric_Value))
{
    pcb->hci_user_confirmation_request = user_confirmation_request;
}

/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_register_event_handler():
 *
 * Used to specify the function that should be called when a L2CAP connection receives
 * data.
 */
/*-----------------------------------------------------------------------------------*/



_ATTR_LWBT_INIT_CODE_
void hci_register_event_handler(void (* hci_event_handler)(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size))
{
    pcb->hci_event_handler = hci_event_handler;
}


/******************************************************************************
 * hci_register_app_event_handler -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * 01a, 27jun2014,  written
 * --------------------
 ******************************************************************************/
_ATTR_LWBT_INIT_CODE_
void hci_register_app_event_handler(void (* hci_app_event_handler)(uint8 event_type, uint8 *parameter, uint16 param_size, err_t err))
{
    pcb->hci_app_event_handler = hci_app_event_handler;
}

/*-----------------------------------------------------------------------------------*/
/*
 * hci_wlp_complete():
 *
 * Used to specify the function that should be called when HCI has received a
 * successful write link policy complete event.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void hci_wlp_complete(int (* wlp_complete)(void *arg, struct bd_addr *bdaddr))
{
    pcb->wlp_complete = wlp_complete;
}
_ATTR_LWBT_INIT_CODE_
void hci_auth_complete(void (* acl_link_auth_complete)(uint8 result))
{
    pcb->acl_link_auth_complete = acl_link_auth_complete;
}


_ATTR_LWBT_INIT_CODE_
void hci_acl_connnect_req(void (* acl_connect_req)(struct bd_addr *bdaddr))
{
    pcb->acl_connect_req = acl_connect_req;
}


_ATTR_LWBT_CODE_
void hci_read_rssi_hook(int (* read_rssi_hook)(struct bd_addr *bdaddr, int8 rssi))
{
    pcb->read_rssi_hook = read_rssi_hook;
}


_ATTR_LWBT_INIT_CODE_
void hci_buf_subtract(void)
{
    if(pcb->hc_num_acl)
    {
        HciServeIsrDisable();
        pcb->hc_num_acl--;
        HciServeIsrEnable();
    }
}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_get_link():
 *
 * Used to get the link structure for that represents an ACL connection.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
struct hci_link *hci_get_link(struct bd_addr *bdaddr, uint8 link_type)
{

    struct hci_link *link = NULL;

    for(link = hci_active_links; link != NULL; link = link->next)
    {
        if(bd_addr_cmp(&(link->bdaddr), bdaddr))
        {
            if(link->link_type == link_type)
                break;
        }
    }
    return link;

}


_ATTR_LWBT_CODE_
int hci_is_link(struct bd_addr *bdaddr, uint8 link_type)
{

    struct hci_link *link = NULL;

    for(link = hci_active_links; link != NULL; link = link->next)
    {
        if(bd_addr_cmp(&(link->bdaddr), bdaddr))
        {
            if(link->link_type == link_type)
                break;
        }
    }
    return (int)CHECK_HCI_FLAG(link->link_flag,HCI_BR_EDR_LINK);
}

_ATTR_LWBT_CODE_
struct hci_link *hci_get_link_use_conhdl(uint16 conhdl)
{

    struct hci_link *link = NULL;
    for(link = hci_active_links; link != NULL; link = link->next)
    {
        if(link->conhdl == conhdl)
        {
            break;
        }
    }
    return link;

}
/*-----------------------------------------------------------------------------------*/
/*
 * hci_acl_input():
 *
 * Called by the physical bus interface. Handles host controller to host flow control,
 * finds a bluetooth address that correspond to the connection handle and forward the
 * packet to the L2CAP layer.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void hci_acl_input(struct pbuf *p)
{
#if 1
    struct hci_acl_hdr *aclhdr;
    struct hci_link *link;
    uint16 conhdl;

    pbuf_header(p, HCI_ACL_HDR_LEN);
    aclhdr = p->payload;
    pbuf_header(p, -HCI_ACL_HDR_LEN);

    conhdl = aclhdr->conhdl_pb_bc & 0x0FFF; /* Get the connection handle from the first
                           12 bits */
    if(pcb->flow)
    {
        //TODO: XXX??? DO WE SAVE NUMACL PACKETS COMPLETED IN LINKS LIST?? SHOULD WE CALL
        //hci_host_num_comp_packets from the main loop when no data has been received from the
        //serial port???
        --pcb->host_num_acl;
        if(pcb->host_num_acl == 0)
        {
            hci_host_num_comp_packets(conhdl, HCI_HOST_MAX_NUM_ACL);
            pcb->host_num_acl = HCI_HOST_MAX_NUM_ACL;
        }
    }

    for(link = hci_active_links; link != NULL; link = link->next)
    {
        if(link->conhdl == conhdl)
        {
            //if(link->link_type == HCI_CONNECT_TYPE_ACL)
            break;
        }
    }

    if(link != NULL)
    {
        if(aclhdr->len)
        {
            LWBT_DEBUGF(HCI_DEBUG, _DBG_DUMP_, ("hci_acl_input: Forward ACL packet to higher layer p->tot_len = %d\n", p->tot_len));
            l2cap_input(p, &(link->bdaddr));
        }
        else
        {
            pbuf_free(p); /* If length of ACL packet is zero, we silently discard it */
        }
    }
    else
    {
        pbuf_free(p); /* If no acitve ACL link was found, we silently discard the packet */
    }
#endif

}



/*-----------------------------------------------------------------------------------*/
/*
 * hci_sco_input():
 *
 * Called by the physical bus interface. Handles host controller to host flow control,
 * finds a bluetooth address that correspond to the connection handle and forward the
 * packet to the L2CAP layer.
 */
/*-----------------------------------------------------------------------------------*/
#ifdef _A2DP_SINK_
_ATTR_LWBT_CODE_
void hci_sco_input(struct pbuf *p)
{
#if 1
    struct hci_sco_hdr *scohdr;
    struct hci_link *link;
    uint16 conhdl;

    pbuf_header(p, HCI_SCO_HDR_LEN);
    scohdr = p->payload;
    pbuf_header(p, -HCI_SCO_HDR_LEN);

    conhdl = scohdr->conhdl_ps & 0x0FFF; /* Get the connection handle from the first
                           12 bits */
    if(pcb->flow)
    {
        //TODO: XXX??? DO WE SAVE NUMACL PACKETS COMPLETED IN LINKS LIST?? SHOULD WE CALL
        //hci_host_num_comp_packets from the main loop when no data has been received from the
        //serial port???
        --pcb->host_num_acl;
        if(pcb->host_num_acl == 0)
        {
            hci_host_num_comp_packets(conhdl, HCI_HOST_MAX_NUM_ACL);
            pcb->host_num_acl = HCI_HOST_MAX_NUM_ACL;
        }
    }

    for(link = hci_active_links; link != NULL; link = link->next)
    {
        if(link->conhdl == conhdl)
        {
            if(link->link_type == HCI_CONNECT_TYPE_SCO || link->link_type == HCI_CONNECT_TYPE_ESCO)
                break;
        }
    }

    if(link != NULL)
    {
        if(scohdr->len)
        {
            LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_acl_input: Forward ACL packet to higher layer p->tot_len = %d\n", p->tot_len));
            // l2cap_input(p, &(link->bdaddr));

            //bt_mediadata_input(p->payload, scohdr->len);
            if(pcb->sco_recv != NULL)
            {
                pcb->sco_recv(p->payload, scohdr->len);
            }

            pbuf_free(p);


        }
        else
        {
            pbuf_free(p); /* If length of ACL packet is zero, we silently discard it */
        }
    }
    else
    {
        pbuf_free(p); /* If no acitve ACL link was found, we silently discard the packet */
    }
#endif

}
#endif






/*-----------------------------------------------------------------------------------*/
#if 1
_ATTR_LWBT_UARTIF_CODE_
uint8 *hci_get_error_code(uint8 code)
{
    switch(code)
    {
        case HCI_SUCCESS:
            return("Success");
        case HCI_UNKNOWN_HCI_COMMAND:
            return("Unknown HCI Command");
        case HCI_NO_CONNECTION:
            return("No Connection");
        case HCI_HW_FAILURE:
            return("Hardware Failure");
        case HCI_PAGE_TIMEOUT:
            return("Page Timeout");
        case HCI_AUTHENTICATION_FAILURE:
            return("Authentication Failure");
        case HCI_KEY_MISSING:
            return("Key Missing");
        case HCI_MEMORY_FULL:
            return("Memory Full");
        case HCI_CONN_TIMEOUT:
            return("Connection Timeout");
        case HCI_MAX_NUMBER_OF_CONNECTIONS:
            return("Max Number Of Connections");
        case HCI_MAX_NUMBER_OF_SCO_CONNECTIONS_TO_DEVICE:
            return("Max Number Of SCO Connections To A Device");
        case HCI_ACL_CONNECTION_EXISTS:
            return("ACL connection already exists");
        case HCI_COMMAND_DISSALLOWED:
            return("Command Disallowed");
        case HCI_HOST_REJECTED_DUE_TO_LIMITED_RESOURCES:
            return("Host Rejected due to limited resources");
        case HCI_HOST_REJECTED_DUE_TO_SECURITY_REASONS:
            return("Host Rejected due to security reasons");
        case HCI_HOST_REJECTED_DUE_TO_REMOTE_DEVICE_ONLY_PERSONAL_SERVICE:
            return("Host Rejected due to remote device is only a personal device");
        case HCI_HOST_TIMEOUT:
            return("Host Timeout");
        case HCI_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE:
            return("Unsupported Feature or Parameter Value");
        case HCI_INVALID_HCI_COMMAND_PARAMETERS:
            return("Invalid HCI Command Parameters");
        case HCI_OTHER_END_TERMINATED_CONN_USER_ENDED:
            return("Other End Terminated Connection: User Ended Connection");
        case HCI_OTHER_END_TERMINATED_CONN_LOW_RESOURCES:
            return("Other End Terminated Connection: Low Resources");
        case HCI_OTHER_END_TERMINATED_CONN_ABOUT_TO_POWER_OFF:
            return("Other End Terminated Connection: About to Power Off");
        case HCI_CONN_TERMINATED_BY_LOCAL_HOST:
            return("Connection Terminated by Local Host");
        case HCI_REPETED_ATTEMPTS:
            return("Repeated Attempts");
        case HCI_PAIRING_NOT_ALLOWED:
            return("Pairing Not Allowed");
        case HCI_UNKNOWN_LMP_PDU:
            return("Unknown LMP PDU");
        case HCI_UNSUPPORTED_REMOTE_FEATURE:
            return("Unsupported Remote Feature");
        case HCI_SCO_OFFSET_REJECTED:
            return("SCO Offset Rejected");
        case HCI_SCO_INTERVAL_REJECTED:
            return("SCO Interval Rejected");
        case HCI_SCO_AIR_MODE_REJECTED:
            return("SCO Air Mode Rejected");
        case HCI_INVALID_LMP_PARAMETERS:
            return("Invalid LMP Parameters");
        case HCI_UNSPECIFIED_ERROR:
            return("Unspecified Error");
        case HCI_UNSUPPORTED_LMP_PARAMETER_VALUE:
            return("Unsupported LMP Parameter Value");
        case HCI_ROLE_CHANGE_NOT_ALLOWED:
            return("Role Change Not Allowed");
        case HCI_LMP_RESPONSE_TIMEOUT:
            return("LMP Response Timeout");
        case HCI_LMP_ERROR_TRANSACTION_COLLISION:
            return("LMP Error Transaction Collision");
        case HCI_LMP_PDU_NOT_ALLOWED:
            return("LMP PDU Not Allowed");
        case HCI_ENCRYPTION_MODE_NOT_ACCEPTABLE:
            return("Encryption Mode Not Acceptable");
        case HCI_UNIT_KEY_USED:
            return("Unit Key Used");
        case HCI_QOS_NOT_SUPPORTED:
            return("QoS is Not Supported");
        case HCI_INSTANT_PASSED:
            return("Instant Passed");
        case HCI_PAIRING_UNIT_KEY_NOT_SUPPORTED:
            return("Pairing with Unit Key Not Supported");
        default:
            return("Error code unknown");
    }
}
#else
_ATTR_LWBT_UARTIF_CODE_
uint8 *hci_get_error_code(uint8 code)
{
    return 0;
}
#endif /* HCI_EV_DEBUG */

// reserves outgoing packet buffer. @returns 1 if successful
int hci_take_packet_buffer(void)
{
    if (pcb->hci_packet_buffer_reserved) return 0;
    pcb->hci_packet_buffer_reserved = 1;
    return 1;
}

void hci_give_packet_buffer(void)
{
    pcb->hci_packet_buffer_reserved = 0;
}


_ATTR_LWBT_UARTIF_CODE_
int hci_can_send_packet_now(void)
{
    if(pcb->hci_packet_buffer_reserved)
    {
        return 0;
    }
    if(pcb->hc_num_acl > 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}
/*-----------------------------------------------------------------------------------*/
/* hci_event_input():
 *
 * Called by the physical bus interface. Parses the received event packet to determine
 * which event occurred and handles it.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
void hci_event_input(struct pbuf *p)
{
#if 1
    struct hci_inq_res *inqres;
    struct hci_event_hdr *evhdr;
    struct hci_link *link;
    uint8 i, j;
    struct bd_addr *bdaddr;
    uint8 resp_offset;
    int ret;
    uint16 ocf;
    uint8  ogf;
    uint8 link_type;
    uint16 conhdl;
    uint8* payloadbackup;
    uint16 payloadLenbackup;
    pbuf_header(p, HCI_EVENT_HDR_LEN);
    evhdr = p->payload;
    pbuf_header(p, -HCI_EVENT_HDR_LEN);
    //LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_,("evhdr->code = %d\n",evhdr->code));
    switch(evhdr->code)
    {
        case HCI_NBR_OF_COMPLETED_PACKETS:
            LWIP_DEBUGF(DBG_OFF, ("hci_event_input: Number Of Completed Packets\n"));
            LWIP_DEBUGF(DBG_OFF, ("Number_of_Handles: 0x%x\n", ((uint8 *)p->payload)[0]));
            for(i=0; i<((uint8 *)p->payload)[0]; i++)
            {
                resp_offset = i*4;
                LWIP_DEBUGF(DBG_OFF, ("Conn_hdl: 0x%x%x\n", ((uint8 *)p->payload)[1+resp_offset], ((uint8 *)p->payload)[2+resp_offset]));
                LWIP_DEBUGF(DBG_OFF, ("HC_Num_Of_Completed_Packets: 0x%x\n",*((uint16 *)(((uint8 *)p->payload)+3+resp_offset))));
                /* Add number of completed ACL packets to the number of ACL packets that the
                BT module can buffer */
                pcb->hc_num_acl += *((uint16 *)(((uint8 *)p->payload) + 3 + resp_offset));
                pcb->flush_timer_enable = 0;
                hci_data_acked += *((uint16 *)(((uint8 *)p->payload) + 3 + resp_offset));
#if HCI_FLOW_QUEUEING
                {
                    uint16 conhdl = *((uint16 *)(((uint8 *)p->payload) + 1 + resp_offset));
                    struct pbuf *q;
                    for(link = hci_active_links; link != NULL; link = link->next)
                    {
                        if(link->conhdl == conhdl)
                        {
                            break;
                        }
                    }

                    if(link != NULL)
                    {
                        q = link->p;
                        /* Queued packet present? */
                        if (q != NULL)
                        {
                            /* NULL attached buffer immediately */
                            link->p = NULL;
                            LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Sending queued packet.\n"));
                            /* Send the queued packet */
                            lp_acl_write(&link->bdaddr, q, link->len, link->pb, link->func);
                            /* Free the queued packet */
                            pbuf_free(q);
                        }

                    }


                }
#endif /* RFCOMM_FLOW_QUEUEING */
            }
            break;
        case HCI_INQUIRY_COMPLETE:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Inquiry complete, 0x%x %s\n", ((uint8 *)p->payload)[0], hci_get_error_code(((uint8 *)p->payload)[0])));
            HCI_EVENT_INQ_COMPLETE(pcb,((uint8 *)p->payload)[0],1, ret);
            break;
        case HCI_INQUIRY_RESULT:
            //printf("HCI_INQUIRY_RESULT\n");
            for(i=0; i<((uint8 *)p->payload)[0]; i++)
            {
                resp_offset = i*14;
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Inquiry result %d\nBD_ADDR: 0x", i));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("\n"));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Page_Scan_Rep_Mode: 0x%x\n",((uint8 *)p->payload)[7+resp_offset]));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Page_Scan_Per_Mode: 0x%x\n",((uint8 *)p->payload)[8+resp_offset]));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Page_Scan_Mode: 0x%x\n",((uint8 *)p->payload)[9+resp_offset]));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Class_of_Dev: 0x%x 0x%x 0x%x\n",((uint8 *)p->payload)[10+resp_offset],
                                           ((uint8 *)p->payload)[11+resp_offset], ((uint8 *)p->payload)[12+resp_offset]));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Clock_Offset: 0x%x%x\n",((uint8 *)p->payload)[13+resp_offset],
                                           ((uint8 *)p->payload)[14+resp_offset]));

                bdaddr = (void *)(((uint8 *)p->payload)+(1+resp_offset));
                if((inqres = lwbt_memp_malloc(MEMP_HCI_INQ)) != NULL)
                {
                    bd_addr_set(&(inqres->bdaddr), bdaddr);
                    inqres->psrm = ((uint8 *)p->payload)[7+resp_offset];
                    inqres->psm = ((uint8 *)p->payload)[9+resp_offset];
                    memcpy(inqres->cod, ((uint8 *)p->payload)+10+resp_offset, 3);
                    inqres->co = *((uint16 *)(((uint8 *)p->payload)+13+resp_offset));
                    //HCI_REG(&(pcb->ires), inqres);

                    if(pcb->inq_complete != NULL)
                    {
                        ret = pcb->inq_complete(pcb->callback_arg,pcb,inqres,0, 0);
                    }
                    else
                    {
                        lwbt_memp_free(MEMP_HCI_INQ, inqres);
                    }

                }
                else
                {
                    LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_event_input: Could not allocate memory for inquiry result\n"));
                }
            }
            break;
#if 1
        case HCI_CONNECTION_COMPLETE:
            bdaddr = (void *)(((uint8 *)p->payload)+3); /* Get the Bluetooth address */
            //DEBUG("HCI_CONNECTION_COMPLETE\n");
            link_type = ((uint8 *)p->payload)[9];
            link = hci_get_link(bdaddr, link_type);
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Conn successfully completed\n"));
                    if(link == NULL)
                    {
                        if((link = hci_new()) == NULL)
                        {
                            /* Could not allocate memory for link. Disconnect */
                            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Could not allocate memory for link. Disconnect\n"));
                            hci_linkdown(bdaddr, HCI_OTHER_END_TERMINATED_CONN_LOW_RESOURCES, link_type);
                            /* Notify L2CAP */
                            lp_disconnect_ind(bdaddr);
                            break;
                        }
                        bd_addr_set(&(link->bdaddr), bdaddr);
                        link->conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
                        link->link_type = link_type;
                        SET_HCI_FLAG(link->link_flag,HCI_BR_EDR_LINK);

                        HCI_REG(&(hci_active_links), link);
                        HCI_EVENT_CONN_COMPLETE(pcb,bdaddr,ret); /* Allow applicaton to do optional configuration of link */
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Calling lp_connect_ind\n"));
                        lp_connect_ind(&(link->bdaddr)); /* Notify L2CAP */
                    }
                    else
                    {
                        link->conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
                        link->link_type = link_type;
                        SET_HCI_FLAG(link->link_flag,HCI_BR_EDR_LINK);
                        HCI_EVENT_CONN_COMPLETE(pcb,bdaddr,ret); /* Allow applicaton to do optional configuration of link */
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Calling lp_connect_cfm\n"));
                        //lp_connect_cfm(bdaddr, ((uint8 *)p->payload)[10], ERR_OK); /* Notify L2CAP */
                        //notify after read remote features
                    }

                    link->encrypt_mode = ((uint8 *)p->payload)[10];
                    if(link_type == HCI_CONNECT_TYPE_ESCO || link_type == HCI_CONNECT_TYPE_SCO )
                    {
                        HCI_EVENT_SCO_CONNECT(pcb);
                        HCI_SEND_EVENT(pcb,HCI_APP_EVENT_SCO_CONNECTED,(uint8*)bdaddr, 6, ERR_OK);
                    }
                    else
                    {
                        hci_read_remote_features(link->conhdl);
                        //DEBUG("acl HCI_CONNECTION_COMPLETE\n");
                        ENCRYPTION_CHANGE = 0;
                        HCI_EVENT_ACL_CONNECTED(pcb, 0, bdaddr);
                        HCI_SEND_EVENT(pcb,HCI_APP_EVENT_ACL_CONNECTED,(uint8*)bdaddr, 6,ERR_OK);
                    }
                    //TODO: MASTER SLAVE SWITCH??

                    //hci_write_scan_enable(0x00);
                    break;
                default:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Conn failed to complete, 0x%x %s\n"
                                               , ((uint8 *)p->payload)[0], hci_get_error_code(((uint8 *)p->payload)[0])));
                    if(link != NULL)
                    {
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Link exists. Notify upper layer\n"));
                        hci_close(link);
                        lp_connect_cfm(bdaddr, ((uint8 *)p->payload)[10], ERR_CONN);
                    }
                    else
                    {
                        /* silently discard */
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Silently discard. Link does not exist\n"));
                    }

                    HCI_EVENT_ACL_CONNECTED(pcb, ((uint8 *)p->payload)[0], bdaddr);
                    break;
            } /* switch */

            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Conn_hdl: 0x%x 0x%x\n", ((uint8 *)p->payload)[1], ((uint8 *)p->payload)[2]));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("BD_ADDR: 0x"));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("\n"));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Link_type: 0x%x\n",((uint8 *)p->payload)[9]));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Encryption_Mode: 0x%x\n",((uint8 *)p->payload)[10]));
            break;
        case HCI_CONNECTION_REQUEST:
            bdaddr = (void *)(((uint8 *)p->payload)); /* Get the Bluetooth address */
            link_type = *(((uint8 *)p->payload)+9);
            //DEBUG("HCI_CONNECTION_REQUEST\n");
            if (link_type == 1)
            {
                // ACL
                link = hci_get_link(bdaddr, link_type);
                if(link)
                {
                    // SYNCHRONOUS CONNECTION LIMIT TO A DEVICE EXCEEDED (0X0A)
                    hci_reject_connection_request(bdaddr, 0x0a);
                }
                else
                {
                    if(pcb->acl_connect_req != NULL)
                    {
                        pcb->acl_connect_req(bdaddr);
                    }
                    else
                    {
                        hci_accept_connection_Request(bdaddr, 0x01);
                    }

                }


            }
            else if(link_type == 0 || link_type == 2) //sco 0r esco
            {
                if(pcb->hci_scolink_number < HCI_SCO_MAX_NUMBER)
                {
                    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);// must be have a acl connect
                    if(link)
                    {
                        pcb->hci_scolink_number++;
                        hci_accpet_synchronus_connection_request(bdaddr);
                    }
                    else
                    {
                        hci_Reject_synchronus_connection_request(bdaddr , 0x0F);
                    }
                }
                else
                {
                    hci_Reject_synchronus_connection_request(bdaddr, 0x0D);
                }
            }

            break;
        case HCI_DISCONNECTION_COMPLETE:
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Disconn has occurred\n"));
                    for(link = hci_active_links; link != NULL; link = link->next)
                    {
                        if(link->conhdl == *((uint16 *)(((uint8 *)p->payload)+1)))
                        {
                            break; /* Found */
                        }
                    }
                    if(link != NULL)
                    {
                        if(link->link_type == HCI_CONNECT_TYPE_ACL)
                        {
                            lp_disconnect_ind(&(link->bdaddr)); /* Notify upper layer */
                            if(CHECK_HCI_FLAG(link->link_flag, HCI_BR_EDR_LINK))
                            {
                                HCI_EVENT_ACL_DISCONNECT(pcb, &(link->bdaddr),((uint8 *)p->payload)[3]);
                                HCI_SEND_EVENT(pcb,HCI_APP_EVENT_ACL_DISCONNECTED,(uint8*)&link->bdaddr, 6, ((uint8 *)p->payload)[3]);
                            }
                            if(CHECK_HCI_FLAG(link->link_flag, HCI_BLE_LINK))
                            {
                                HCI_SEND_EVENT(pcb,HCI_APP_EVENT_BLE_DISCONNECTED,(uint8*)&link->bdaddr, 6, ((uint8 *)p->payload)[3]);
                            }
                            hci_close(link);

                        }
                        else if(link->link_type == HCI_CONNECT_TYPE_BLEACL)
                        {
                            hci_close(link);
                        }
                        else
                        {
                            if(pcb->hci_scolink_number > 0)
                            {
                                pcb->hci_scolink_number--;
                            }

                            HCI_EVENT_SCO_DISCONNECT(pcb);

                            HCI_SEND_EVENT(pcb,HCI_APP_EVENT_SCO_DISCONNECTED,(uint8*)&link->bdaddr, 6, ((uint8 *)p->payload)[3]);
                            hci_close(link);
                        }


                    }
                    /* else silently discard */


                    break;
                default:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Disconn failed to complete, 0x%x %s\n"
                                               , ((uint8 *)p->payload)[0], hci_get_error_code(((uint8 *)p->payload)[0])));
                    return;
            }
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Conn_hdl: 0x%x%x\n", ((uint8 *)p->payload)[1], ((uint8 *)p->payload)[2]));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Reason: 0x%x %s\n", ((uint8 *)p->payload)[3], hci_get_error_code(((uint8 *)p->payload)[3])));
            break;
#endif

        case HCI_QOS_SETUP_COMPLETE:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: QOS setup complete result = 0x%x\n", ((uint8 *)p->payload)[0]));
            break;
        case HCI_COMMAND_COMPLETE:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Command Complete\n"));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Num_HCI_Command_Packets: 0x%x\n", ((uint8 *)p->payload)[0]));

            pcb->numcmd += ((uint8 *)p->payload)[0]; /* Add number of completed command packets to the
                                   number of command packets that the BT module
                                   can buffer */
            pbuf_header(p, -1); /* Adjust payload pointer not to cover
                     Num_HCI_Command_Packets parameter */
            ocf = *((uint16 *)p->payload) & 0x03FF;
            ogf = *((uint16 *)p->payload) >> 10;

            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("OCF == 0x%x OGF == 0x%x\n", ocf, ogf));

            pbuf_header(p, -2); /* Adjust payload pointer not to cover Command_Opcode
                       parameter */
            if(ogf == HCI_INFO_PARAM)
            {
                if(ocf == HCI_READ_BUFFER_SIZE)
                {
                    if(((uint8 *)p->payload)[0] == HCI_SUCCESS)
                    {
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Read_Buffer_Size command succeeded\n"));
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("HC_ACL_Data_Packet_Length: 0x%x%x\n", ((uint8 *)p->payload)[1], ((uint8 *)p->payload)[2]));
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("HC_SCO_Data_Packet_Length: 0x%x\n", ((uint8 *)p->payload)[3]));
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("HC_Total_Num_ACL_Data_Packets: %d\n", *((uint16 *)(((uint8 *)p->payload)+4))));
                        pcb->maxsize = *((uint16 *)(((uint8 *)p->payload)+1)); /* Maximum size of an ACL packet
                                                  that the BT module is able to
                                                  accept */
                        pcb->hc_num_acl = *((uint16 *)(((uint8 *)p->payload)+4)); /* Number of ACL packets that the
                                                     BT module can buffer */
                        pcb->hc_num_acl_total = pcb->hc_num_acl;
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("HC_Total_Num_SCO_Data_Packets: 0x%x%x\n", ((uint8 *)p->payload)[6], ((uint8 *)p->payload)[7]));
                    }
                    else
                    {
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Read_Buffer_Size command failed, 0x%x %s\n", ((uint8 *)p->payload)[0], hci_get_error_code(((uint8 *)p->payload)[0])));
                        return;
                    }
                }

                if(ocf == HCI_READ_BD_ADDR)
                {
                    if(((uint8 *)p->payload)[0] == HCI_SUCCESS)
                    {
                        bdaddr = (void *)(((uint8 *)p->payload) + 1); /* Get the Bluetooth address */
                        memcpy(pcb->bdaddr.addr,bdaddr, 6);
                        HCI_EVENT_RBD_COMPLETE(pcb, bdaddr, ret); /* Notify application.*/

                    }
                }
            }

            if(ogf == HCI_HOST_C_N_BB && ocf == HCI_SET_HC_TO_H_FC)
            {
                if(((uint8 *)p->payload)[0] == HCI_SUCCESS)
                {
                    pcb->flow = 1;
                }
            }
            if(ogf == HCI_LINK_POLICY)
            {

                if(ocf == HCI_W_LINK_POLICY)
                {
                    if(((uint8 *)p->payload)[0] == HCI_SUCCESS)
                    {
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Successful HCI_W_LINK_POLICY.\n"));
                        for(link = hci_active_links; link != NULL; link = link->next)
                        {
                            if(link->conhdl == *((uint16 *)(((uint8 *)p->payload)+1)))
                            {
                                break;
                            }
                        }
                        if(link == NULL)
                        {
                            LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_event_input: Connection does not exist\n"));
                            return; /* Connection does not exist */
                        }
                        HCI_EVENT_WLP_COMPLETE(pcb, &link->bdaddr, ret); /* Notify application.*/
                    }
                    else
                    {
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Unsuccessful HCI_W_LINK_POLICY.\n"));
                        return;
                    }
                }
            }

            if(ogf == HCI_STATUS_PARAM)
            {
                if(ocf == HCI_GET_RSSI)
                {
                    if(((uint8 *)p->payload)[0] == HCI_SUCCESS)
                    {
                        uint16 conhdl_temp;
                        struct hci_link *link_temp;
                        //((uint8 *)p->payload)[3] //RSSI
                        conhdl_temp = *((uint16 *)(((uint8 *)p->payload)+1));
                        link_temp = hci_get_link_use_conhdl(conhdl_temp);
                        if(pcb->read_rssi_hook)
                        {
                            pcb->read_rssi_hook(&link_temp->bdaddr,((int8 *)p->payload)[3]);
                        }
                    }
                }
            }

            if(ogf == HCI_BLE_OGF && ocf == HCI_LE_READ_BUFFER_SIZE)
            {
                pcb->hc_le_max_data_size = *((uint16 *)(((uint8 *)p->payload)+1));
                pcb->hc_le_num_total = pcb->hc_le_num_acl = *((uint16 *)(((uint8 *)p->payload)+2));
                if(pcb->hc_num_acl == 0)
                {
                    pcb->hc_num_acl = pcb->hc_num_acl_total = pcb->hc_le_num_acl;
                    pcb->maxsize = pcb->hc_le_max_data_size;
                }
            }

            if(ogf == HCI_LINK_CONTROL && ocf == 0x0D)
            {
                int status =((uint8 *)p->payload)[0];
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_,("status = %d\n",status));
            }
            if(ogf == HCI_LINK_CONTROL && ocf == HCI_CREATE_CONNECTION_CANNEL)
            {
                int status =((uint8 *)p->payload)[0];
                if(status == HCI_SUCCESS)
                {
                    bdaddr = (void *)(((uint8 *)p->payload) + 1); /* Get the Bluetooth address */
                    link = hci_get_link(bdaddr, 0x01);
                    if(link)
                    {
                        hci_close(link);
                    }
                    //lp_connect_cfm((struct bd_addr *)((uint8 *)p->payload+1), ((uint8 *)p->payload)[10], ERR_CONN);
                    lp_disconnect_ind((struct bd_addr *)((uint8 *)p->payload+1)); /* Notify upper layer */
                }
                else if(status == HCI_ACL_CONNECTION_EXISTS)
                {

                }
                else if(status == HCI_NO_CONNECTION)
                {

                }
            }


            HCI_EVENT_CMD_COMPLETE(pcb,ogf,ocf,((uint8 *)p->payload)[0],ret);
            break;
        case HCI_COMMAND_STATUS:
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Command Status\n"));
                    break;
                default:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Command failed, %s\n", hci_get_error_code(((uint8 *)p->payload)[0])));
                    pbuf_header(p, -2); /* Adjust payload pointer not to cover
                             Num_HCI_Command_Packets and status parameter */
                    ocf = *((uint16 *)p->payload) & 0x03FF;
                    ogf = *((uint16 *)p->payload) >> 10;
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("OCF == 0x%x OGF == 0x%x\n", ocf, ogf));
                    pbuf_header(p, -2); /* Adjust payload pointer not to cover Command_Opcode
                           parameter */
                    HCI_EVENT_CMD_COMPLETE(pcb,ogf,ocf,((uint8 *)p->payload)[0],ret);
                    pbuf_header(p, 4);
                    break;
            }
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Num_HCI_Command_Packets: 0x%x\n", ((uint8 *)p->payload)[1]));
            pcb->numcmd += ((uint8 *)p->payload)[1]; /* Add number of completed command packets to the
                                   number of command packets that the BT module
                                   can buffer */
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Command_Opcode: 0x%x 0x%x\n", ((uint8 *)p->payload)[2], ((uint8 *)p->payload)[3]));
            break;
        case HCI_HARDWARE_ERROR:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Hardware Error\n"));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Hardware_code: 0x%x\n\n", ((uint8 *)p->payload)[0]));
            //TODO: IS THIS FATAL??
            break;
        case HCI_ROLE_CHANGE:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Role change\n"));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Status: 0x%x\n", ((uint8 *)p->payload)[0]));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("New Role: 0x%x\n", ((uint8 *)p->payload)[7]));
            break;

        case HCI_MODE_CHANGE:
            //LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Mode change\n"));
            //LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Status: 0x%x\n", ((uint8 *)p->payload)[0]));
            //LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Conn_hdl: 0x%x\n", ((uint16 *)(((uint8 *)p->payload) + 1))[0]));
#if 1 //def BT_HOST_SNIFF
            {
                uint16 handle = ((uint16 *)(((uint8 *)p->payload) + 1))[0];
                uint8 status = ((uint8 *)p->payload)[0];
                uint8 mode = ((uint8 *)p->payload)[3];
                struct hci_link *link_temp;
                link_temp = hci_get_link_use_conhdl(handle);
                link_temp->Mode = mode;
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_,("hci_event_input: Mode change,Status: 0x%x,mode = %d\n\n", ((uint8 *)p->payload)[0], mode));
                //printf("hci_event_input: Mode change,Status: 0x%x,mode = %d\n\n", ((uint8 *)p->payload)[0], mode);
            }
#endif
            break;
        case HCI_DATA_BUFFER_OVERFLOW:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Data Buffer Overflow\n"));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Link_Type: 0x%x\n", ((uint8 *)p->payload)[0]));
            //TODO: IS THIS FATAL????
            break;
        case HCI_MAX_SLOTS_CHANGE:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Max slots changed\n"));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Conn_hdl: 0x%x\n", ((uint16 *)p->payload)[0]));
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("LMP max slots: 0x%x\n", ((uint8 *)p->payload)[2]));
            break;
        case HCI_PIN_CODE_REQUEST:
            bdaddr = (void *)((uint8 *)p->payload); /* Get the Bluetooth address */
            HCI_EVENT_PIN_REQ(pcb, bdaddr, ret); /* Notify application. If event is not registered,
                                send a negative reply */
            break;
        case HCI_LINK_KEY_NOTIFICATION:
            bdaddr = (void *)((uint8 *)p->payload); /* Get the Bluetooth address */

            HCI_EVENT_LINK_KEY_NOT(pcb, bdaddr, ((uint8 *)p->payload) + 6, ret); /* Notify application.*/
            break;

        case HCI_LINK_KEY_RESQUEST:

            bdaddr = (void *)((uint8 *)p->payload); /* Get the Bluetooth address */

            HCI_EVENT_LINK_KEY_REQ(pcb, bdaddr, ret);

            break;

        case HCI_IO_CAPABILITY_REQUEST:

            bdaddr = (void *)((uint8 *)p->payload); /* Get the Bluetooth address */
            //hci_Io_Capability_req_reply(bdaddr, 0x03, 0x00, 0x04);//noinputnooutput
            hci_Io_Capability_req_reply(bdaddr, hci_io_capability, 0x00, 0x04);//noinputnooutput
            break;

        case HCI_IO_CAPABILITY_RESPONSE:
            {
                uint8 io_capabiliy;
                uint8 oob;
                uint8 Authentication_Requirements;
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("io capability response\n"));
                bdaddr = (void *)((uint8 *)p->payload); /* Get the Bluetooth address */
                io_capabiliy = ((uint8 *)p->payload)[6];
                oob = ((uint8 *)p->payload)[7];
                Authentication_Requirements = ((uint8 *)p->payload)[8];
                link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);
                link->remote_io_capability = io_capabiliy;
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("remote:io_capabiliy %d \n",io_capabiliy));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("remote:oob %d\n", oob));
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("remote:Authentication_Requirements %d\n",Authentication_Requirements));
            }
            break;

        case HCI_USER_CONFIRMATION_REQUEST:
            {
                uint32 Numeric_Value;
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("user configmation request\n"));
                bdaddr = (void *)((uint8 *)p->payload); /* Get the Bluetooth address */
                Numeric_Value = *((uint32*)((uint8 *)p->payload+6));
                link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);
                if(link->remote_io_capability == 0x01 && hci_io_capability == 0x01)/*dispalyYesNo*/
                {
                    HCI_EVENT_USER_CONFIRMATION_REQUEST(pcb,bdaddr,Numeric_Value);
                }
                else
                {

hci_user_confirmati_req_reply(bdaddr);
                }
            }
            break;

        case HCI_SIMPLE_PAIRING_COMPLETE:

            //to notification  ui  show that simple pairing complete

            break;

        case HCI_SYNCHRONOUS_CONNECTION_COMPLETE:
            bdaddr = (void *)(((uint8 *)p->payload)+3); /* Get the Bluetooth address */
            link_type = ((uint8 *)p->payload)[9];
            link = hci_get_link(bdaddr, link_type);
            //DEBUG("Enter HCI_SYNCHRONOUS_CONNECTION_COMPLETE,link=%04x,link_flag=%d\n", link, link->link_flag);
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Conn successfully completed\n"));
                    if(link == NULL)
                    {
                        if((link = hci_new()) == NULL)
                        {
                            /* Could not allocate memory for link. Disconnect */
                            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Could not allocate memory for link. Disconnect\n"));
                            hci_linkdown(bdaddr, HCI_OTHER_END_TERMINATED_CONN_LOW_RESOURCES ,link_type);
                            break;
                        }
                        bd_addr_set(&(link->bdaddr), bdaddr);
                        link->conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
                        link->link_type = link_type;
                        HCI_REG(&(hci_active_links), link);
                    }
                    else
                    {
                        link->conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
                        link->link_type = link_type;
                    }
                    if(link_type == HCI_CONNECT_TYPE_ESCO || link_type == HCI_CONNECT_TYPE_SCO )
                    {
                        HCI_EVENT_SCO_CONNECT(pcb);

                        HCI_SEND_EVENT(pcb,HCI_APP_EVENT_SCO_CONNECTED,(uint8*)bdaddr, 6, ERR_OK);
                    }

                    //TODO: MASTER SLAVE SWITCH??

                    break;
                default:
                    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Conn failed to complete, 0x%x %s\n"
                                               , ((uint8 *)p->payload)[0], hci_get_error_code(((uint8 *)p->payload)[0])));
                    if(link != NULL)
                    {
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Link exists. Notify upper layer\n"));
                        hci_close(link);
                    }
                    else
                    {
                        /* silently discard */
                        LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Silently discard. Link does not exist\n"));
                    }
                    break;
            } /* switch */

            //DEBUG("Leaving HCI_SYNCHRONOUS_CONNECTION_COMPLETE,link=%04x,link_flag=%d\n", link, link->link_flag);
            break;

        case HCI_READ_REMOTE_FEATURES_COMPLETE:
            //DEBUG("Enter HCI_READ_REMOTE_FEATURES_COMPLETE\n");
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                {

                    uint8 * features = &((uint8 *)p->payload)[3];
                    conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
                    //DEBUG("Enter HCI_SUCCESS\n");
                    link = hci_get_link_use_conhdl(conhdl);
                    //DEBUG("Enter HCI_SUCCESS 1,link=%04x,link_flag=%d\n",link, link->link_flag);
                    if (features[6] & (1 << 3))
                    {
                        SET_HCI_FLAG(link->link_flag, HCI_REMOTE_SUPPORT_SPP);
                        //DEBUG("Enter HCI_SUCCESS 2,link=%04x,link_flag=%d\n", link, link->link_flag);
                    }
                    else
                    {
                        //DEBUG("Enter HCI_SUCCESS 3\n");
                        if(CHECK_HCI_FLAG(link->link_flag,HCI_FLAG_IR))
                        {
                            //lp_connect_cfm(&link->bdaddr, link->encrypt_mode, ERR_OK); /* Notify L2CAP */
                            // remote not support ssp, initiate l2cap req
                            // if remote suuport ssp , initiate after encryption
                            // all initiate after encrypion by wp 20160712
                        }
                    }
                    //DEBUG("Enter HCI_SUCCESS 4,link_flag=%d\n", link->link_flag);
                    if(CHECK_HCI_FLAG(link->link_flag,HCI_FLAG_IR))
                    {
                        hci_Authentication_Requested(conhdl);
                    }
                    //DEBUG("Enter HCI_SUCCESS 5,link_flag=%d\n", link->link_flag);

                    SET_HCI_FLAG(pcb->hci_flag, HCI_AUTO_READ_REMOTE_NAME);
                    hci_read_remote_name(&link->bdaddr, 0 , 0);
                    //DEBUG("Leaving HCI_SUCCESS,link=%04x,link_flag=%d\n", link, link->link_flag);
                }
                break;

                default:
                    break;
            }
            //DEBUG("Leaving HCI_READ_REMOTE_FEATURES_COMPLETE,link=%04x,link_flag=%d\n", link, link->link_flag);

            break;

        case HCI_READ_REMOTE_EXTENDED_FEATURES_COMPLETE:
            //DEBUG("Enter HCI_READ_REMOTE_EXTENDED_FEATURES_COMPLETE\n");
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                    conhdl = *((uint16 *)(((uint8 *)p->payload)+1));

                    link = hci_get_link_use_conhdl(conhdl);
                    //DEBUG("Enter HCI_READ_REMOTE_EXTENDED_FEATURES_COMPLETE,link=%04x,link_flag=%d\n", link, link->link_flag);
//                if(CHECK_HCI_FLAG(link->link_flag,HCI_FLAG_IR))
//                {
//                    hci_Authentication_Requested(conhdl);
//                }
                    break;

                default:
                    break;
            }
            break;
            //DEBUG("Leaving HCI_READ_REMOTE_EXTENDED_FEATURES_COMPLETE,link=%04x,link_flag=%d\n", link, link->link_flag);
        case HCI_AUTHENTICATION_COMPLETE:
            //DEBUG("Enter HCI_AUTHENTICATION_COMPLETE\n");
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                {

                    conhdl = *((uint16 *)(((uint8 *)p->payload)+1));

                    link = hci_get_link_use_conhdl(conhdl);
                    //DEBUG("Enter HCI_AUTHENTICATION_COMPLETE,link=%04x,link_flag=%d\n", link, link->link_flag);
                    SET_HCI_FLAG(link->link_flag, HCI_AUTH_COMPLETE);
                    if(CHECK_HCI_FLAG(link->link_flag,HCI_FLAG_IR))
                    {
                        hci_set_conn_encrypt(conhdl, 0x01);
                    }

                    //HCI_EVENT_ACL_AUTH_COMPLETE(pcb, ((uint8 *)p->payload)[0]);

                }
                break;

                default:
                {
                    conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
                    link = hci_get_link_use_conhdl(conhdl);
                    //if(CHECK_HCI_FLAG(link->link_flag, HCI_REMOTE_SUPPORT_SPP))
                    {
                        //lp_connect_cfm(&link->bdaddr, link->encrypt_mode, ERR_CONN); /* Notify L2CAP */
                    }
                    if(CHECK_HCI_FLAG(link->link_flag,HCI_FLAG_IR))
                    {
                        //hci_Authentication_Requested(conhdl);
                        HCI_EVENT_ACL_AUTH_COMPLETE(pcb, ((uint8 *)p->payload)[0]);
                    }
                }
                break;
            }

            //DEBUG("Leaving HCI_AUTHENTICATION_COMPLETE,link =%04x,link_flag=%d\n", link, link->link_flag);
            break;
        case HCI_ENCRYPTION_CHANGE:
           // DEBUG("Enter HCI_ENCRYPTION_CHANGE\n");
            if(ENCRYPTION_CHANGE == 0)
            {
            conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
            link = hci_get_link_use_conhdl(conhdl);
            //DEBUG("Enter HCI_ENCRYPTION_CHANGE 1,link=%04x,link_flag=%d\n", link, link->link_flag);
            if(CHECK_HCI_FLAG(link->link_flag,HCI_FLAG_IR))
            {
                //if(CHECK_HCI_FLAG(link->link_flag, HCI_REMOTE_SUPPORT_SPP))
                {
                    //DEBUG("Enter HCI_ENCRYPTION_CHANGE 3\n");
                    lp_connect_cfm(&link->bdaddr, link->encrypt_mode, ERR_OK); /* Notify L2CAP */
                }
            }
            HCI_EVENT_ACL_AUTH_COMPLETE(pcb, ((uint8 *)p->payload)[0]);

           // DEBUG("Leaving HCI_ENCRYPTION_CHANGE,link=%04x,link_flag=%d\n", link, link->link_flag);
                ENCRYPTION_CHANGE = 1;
            }
            break;
        case HCI_REMOTE_NAME_REQUEST_COMPLETE_EVENT:
            //DEBUG("Enter HCI_REMOTE_NAME_REQUEST_COMPLETE_EVENT\n");
            switch(((uint8 *)p->payload)[0])
            {
                case HCI_SUCCESS:
                {
                    struct hci_link *link = NULL;
                    bdaddr = (void *)(((uint8 *)p->payload)+1); /* Get the Bluetooth address */

                    if(CHECK_HCI_FLAG(pcb->hci_flag,HCI_AUTO_READ_REMOTE_NAME))
                    {
                        for(link = hci_active_links; link != NULL; link = link->next)
                        {
                            if(bd_addr_cmp(&(link->bdaddr), bdaddr))
                            {
                                memcpy(link->name,((uint8 *)p->payload)+7, 32);
                                updata_devname_to_sysconfig(bdaddr, link->name);
                                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("remote name : %s", link->name));
                                break;
                            }
                        }
                        CLEAR_HCI_FLAG(pcb->hci_flag, HCI_AUTO_READ_REMOTE_NAME);
                    }
                    else
                    {
                        HCI_EVENT_READ_REMOTE_NAME_COMPLETE(pcb,((uint8 *)p->payload)[0], bdaddr, ((uint8 *)p->payload)+7, ret);
                    }
                }
                break;
                default:
                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: read remote name fail, 0x%x %s\n"
                           , ((uint8 *)p->payload)[0], hci_get_error_code(((uint8 *)p->payload)[0])));
                    bdaddr = (void *)(((uint8 *)p->payload)+1);
                    if(CHECK_HCI_FLAG(pcb->hci_flag,HCI_AUTO_READ_REMOTE_NAME))
                    {
                        CLEAR_HCI_FLAG(pcb->hci_flag, HCI_AUTO_READ_REMOTE_NAME);
                    }
                    HCI_EVENT_READ_REMOTE_NAME_COMPLETE(pcb,((uint8 *)p->payload)[0], bdaddr, ((uint8 *)p->payload)+7, ret);
                    break;


            }


            //DEBUG("Leaving HCI_REMOTE_NAME_REQUEST_COMPLETE_EVENT,link=%04x,link_flag=%d\n", link, link->link_flag);

            break;
        case HCI_FLUSH_OCCURRED:
            pcb->flush_timer_enable = 0;
            //DEBUG("Enter HCI_FLUSH_OCCURRED\n");
            if(pcb->hc_num_acl == 0)
            {
                pcb->hc_num_acl = pcb->hc_num_acl_total;
            }
            else
            {
                break;
            }
#if HCI_FLOW_QUEUEING
            {
                struct pbuf *q;
                conhdl = *((uint16 *)(((uint8 *)p->payload)));
                for(link = hci_active_links; link != NULL; link = link->next)
                {
                    if(link->conhdl == conhdl)
                    {
                        break;
                    }
                }
    LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("Enter HCI_FLUSH_OCCURRED,link_flag=%d\n", link->link_flag));

                if(link != NULL)
                {
                    q = link->p;
                    /* Queued packet present? */
                    if (q != NULL)
                    {
                        /* NULL attached buffer immediately */
                        link->p = NULL;
                        LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Sending queued packet.\n"));
                        /* Send the queued packet */
                        lp_acl_write(&link->bdaddr, q, link->len, link->pb, link->func);
                        /* Free the queued packet */
                        pbuf_free(q);
                    }
                }
            }
#endif /* RFCOMM_FLOW_QUEUEING */
            //DEBUG("Leaving HCI_FLUSH_OCCURRED,link_flag=%d\n", link->link_flag);
        case HCI_EXTENDED_INQUIRY_RESULT_EVENT:
            //printf("HCI_EXTENDED_INQUIRY_RESULT_EVENT\n");

            bdaddr = (void *)(((uint8 *)p->payload)+1);
            if((inqres = lwbt_memp_malloc(MEMP_HCI_INQ)) != NULL)
            {
                char * ex_inq_req;
                int ex_inq_req_len;
                int ad_len;
                int ad_flag;
                char *  ad_value;
                bd_addr_set(&(inqres->bdaddr), bdaddr);
                inqres->psrm = ((uint8 *)p->payload)[7];
                memcpy(inqres->cod, ((uint8 *)p->payload)+9, 3);
                inqres->co = *((uint16 *)(((uint8 *)p->payload)+12));
                inqres->rssi = ((uint8 *)p->payload)[14];
                //HCI_REG(&(pcb->ires), inqres);
                ex_inq_req_len = evhdr->len - 15;
                ex_inq_req = (void *)(((uint8 *)p->payload)+15);
                //printf("ex_inq_req_len = %d\n",ex_inq_req_len);

                //debug_hex(ex_inq_req,ex_inq_req_len , 16);
                while(ex_inq_req_len && ex_inq_req[0] != 0)
                {
                    ad_len = ex_inq_req[0]+1;
                    ad_flag = ex_inq_req[1];
                    ad_value = ex_inq_req+2;

                    if(ad_flag == 0x08 || ad_flag == 0x09)
                    {
                        int namelen = (ad_len-2) > 32 ? 32 : (ad_len-2);
                        memcpy(inqres->name, ad_value, namelen);
                        //printf("inqres->name = %s\n",inqres->name);
                        break;
                    }
                    ex_inq_req_len -= ad_len;
                    ex_inq_req += ad_len;
                }
                if(pcb->inq_complete != NULL)
                {
                    ret = pcb->inq_complete(pcb->callback_arg,pcb,inqres,0, 0);
                }
                else
                {
                    lwbt_memp_free(MEMP_HCI_INQ, inqres);
                }

            }
            else
            {
                LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_event_input: Could not allocate memory for inquiry result\n"));
            }

            break;

        case HCI_INQUIRY_RESULT_RSSI_EVENT:
            bdaddr = (void *)(((uint8 *)p->payload)+1);
            if((inqres = lwbt_memp_malloc(MEMP_HCI_INQ)) != NULL)
            {
                char * ex_inq_req;
                int ex_inq_req_len;
                int ad_len;
                int ad_flag;
                char *  ad_value;
                bd_addr_set(&(inqres->bdaddr), bdaddr);
                inqres->psrm = ((uint8 *)p->payload)[7];
                memcpy(inqres->cod, ((uint8 *)p->payload)+9, 3);
                inqres->co = *((uint16 *)(((uint8 *)p->payload)+12));
                inqres->rssi = ((uint8 *)p->payload)[14];

                if(pcb->inq_complete != NULL)
                {
                    ret = pcb->inq_complete(pcb->callback_arg,pcb,inqres,0, 0);
                }
                else
                {
                    lwbt_memp_free(MEMP_HCI_INQ, inqres);
                }

            }
            else
            {
                LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_event_input: Could not allocate memory for inquiry result\n"));
            }

        break;

#ifdef HAVE_BLE
        case HCI_BLE_META_EVENT:
        {
            uint8 bleEventCode;

            bleEventCode = ((uint8 *)p->payload)[0];

            switch(bleEventCode)
            {
                case HCI_BLE_CONNECTION_COMPLETE_EVENT:
                {
                    bdaddr = (void *)(((uint8 *)p->payload)+6); /* Get the Bluetooth address */
                    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_BLEACL);
                    switch(((uint8 *)p->payload)[1])
                    {
                        case HCI_SUCCESS:

                            if(link == NULL)
                            {
                                if((link = hci_new()) == NULL)
                                {

                                    hci_linkdown(bdaddr, HCI_OTHER_END_TERMINATED_CONN_LOW_RESOURCES, HCI_CONNECT_TYPE_ACL);
                                    /* Notify L2CAP */
                                    lp_disconnect_ind(bdaddr);
                                    break;
                                }
                                bd_addr_set(&(link->bdaddr), bdaddr);
                                link->conhdl = *((uint16 *)(((uint8 *)p->payload)+2));
                                link->role = *((uint8 *)(((uint8 *)p->payload)+4));
                                link->Conn_Interval = *((uint16 *)(((uint8 *)p->payload)+12));
                                link->Conn_Latency  = *((uint16 *)(((uint8 *)p->payload)+14));
                                link->Supervision_Timeout = *((uint16 *)(((uint8 *)p->payload)+14));
                                link->Master_Clock_Accuracy = *((uint8 *)(((uint8 *)p->payload)+15));
                                link->link_type = HCI_CONNECT_TYPE_BLEACL;
                                SET_HCI_FLAG(link->link_flag,HCI_BLE_LINK);
                                HCI_REG(&(hci_active_links), link);
                                //HCI_EVENT_CONN_COMPLETE(pcb,bdaddr,ret); /*  */
                                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Calling lp_connect_ind\n"));
                                lp_connect_ind(&(link->bdaddr)); /* Notify L2CAP */
                            }
                            else
                            {
                                link->conhdl = *((uint16 *)(((uint8 *)p->payload)+1));
                                link->link_type = HCI_CONNECT_TYPE_BLEACL;
                                //HCI_EVENT_CONN_COMPLETE(pcb,bdaddr,ret); /* Allow applicaton to do optional configuration of link */
                                LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Calling lp_connect_cfm\n"));
                                lp_connect_cfm(bdaddr, ((uint8 *)p->payload)[10], ERR_OK); /* Notify L2CAP */
                            }

                            HCI_SEND_EVENT(pcb,HCI_APP_EVENT_BLE_CONNECTED,(uint8*)bdaddr, 6,ERR_OK);
                            //HCI_EVENT_BLE_CONNECTED(pcb, 0, bdaddr);


                            break;
                        default:

                            break;
                    } /* switch */

                }
                break;

                case HCI_BLE_ADVERTISING_REPORT_EVENT:

                    break;

                case HCI_BLE_CONNETION_UPDATE_COMPLETE_EVENT:
                {
                    uint16 conhdl;
                    conhdl = *((uint16 *)(((uint8 *)p->payload)+2));
                    link = hci_get_link_use_conhdl(conhdl);
                    switch(((uint8 *)p->payload)[1])
                    {
                        case HCI_SUCCESS:

                            if(link == NULL)
                            {

                            }
                            else
                            {
                                link->Conn_Interval = *((uint16 *)(((uint8 *)p->payload)+4));
                                link->Conn_Latency  = *((uint16 *)(((uint8 *)p->payload)+6));
                                link->Supervision_Timeout = *((uint16 *)(((uint8 *)p->payload)+8));
                                //HCI_EVENT_BLE_CONNETION_UPDATE_COMPLETE(pcb, 0, bdaddr);
                            }





                            break;
                        default:

                            break;
                    } /* switch */

                }

                break;

                case HCI_BLE_REMOTE_USED_FEATURES_COMPLETE_EVENT:

                    break;

                case HCI_LONG_TERM_KEY_REQUEST_EVENT:

                    break;

                default:
                    break;
            }
        }
        break;

#endif

        default:
            LWBT_DEBUGF(HCI_EV_DEBUG, _DBG_INFO_, ("hci_event_input: Undefined event code 0x%x\n", evhdr->code));
            break;
    }/* switch */

    l2cap_event_packet_handler(HCI_EVENT_PACKET, 0, (uint8*)evhdr, evhdr->len+2);

    if(pcb->hci_event_handler)
    {
        pcb->hci_event_handler(HCI_EVENT_PACKET, 0, (uint8*)evhdr, evhdr->len+2);
    }


#endif
}
/*-----------------------------------------------------------------------------------*/
/* HCI Commands */
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
/* hci_cmd_ass():
 *
 * Assemble the command header.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
struct pbuf *hci_cmd_ass(struct pbuf *p, uint16 ocf, uint8 ogf, uint8 len)
{
    ((uint8 *)p->payload)[0] = HCI_COMMAND_DATA_PACKET; /* cmd packet type */
    ((uint8 *)p->payload)[1] = (ocf & 0xff); /* OCF & OGF */
    ((uint8 *)p->payload)[2] = (ocf >> 8)|(ogf << 2);
    ((uint8 *)p->payload)[3] = len-HCI_CMD_HDR_LEN-1; /* Param len = plen - cmd hdr - ptype */
    if(pcb->numcmd != 0)
    {
        --pcb->numcmd; /* Reduce number of cmd packets that the host controller can buffer */
    }
    return p;
}
/*-----------------------------------------------------------------------------------*/
/* hci_inquiry():
 *
 * Cause the Host contoller to enter inquiry mode to discovery other nearby Bluetooth
 * devices.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_inquiry(uint32 lap, uint8 inq_len, uint8 num_resp,
                int (* inq_complete)(void *arg, struct hci_pcb *pcb,
                                     struct hci_inq_res *ires, uint16 result))
{
    struct pbuf *p;
    struct hci_inq_res *tmpres;
    /* Free any previous inquiry result list */
    while(pcb->ires != NULL)
    {
        tmpres = pcb->ires;
        pcb->ires = pcb->ires->next;
        lwbt_memp_free(MEMP_HCI_INQ, tmpres);
    }

    pcb->inq_complete = inq_complete;

    if((p = pbuf_alloc(PBUF_RAW, HCI_INQUIRY_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_,("hci_inquiry: Could not allocate memory for pbuf\n"));
        return ERR_MEM; /* Could not allocate memory for pbuf */
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_INQUIRY_OCF, HCI_LINK_CTRL_OGF, HCI_INQUIRY_PLEN);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = lap & 0xFF;
    ((uint8 *)p->payload)[5] = lap >> 8;
    ((uint8 *)p->payload)[6] = lap >> 16;
    //memcpy(((uint8 *)p->payload)+4, inqres->cod, 3);
    ((uint8 *)p->payload)[7] = inq_len;
    ((uint8 *)p->payload)[8] = num_resp;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;


}
/*-----------------------------------------------------------------------------------*/
/* hci_disconnect():
 *
 * Used to terminate an existing connection.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_linkdown(struct bd_addr *bdaddr, uint8 reason, uint8 link_type)
{
    struct pbuf *p;
    struct hci_link *link;

    link = hci_get_link(bdaddr, link_type);

    if(link == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_disconnect: Connection does not exist\n"));
        return ERR_CONN; /* Connection does not exist */
    }
    if((p = pbuf_alloc(PBUF_RAW, HCI_DISCONN_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_disconnect: Could not allocate memory for pbuf\n"));
        return ERR_MEM; /* Could not allocate memory for pbuf */
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_DISCONN_OCF, HCI_LINK_CTRL_OGF, HCI_DISCONN_PLEN);

    /* Assembling cmd prameters */
    ((uint16 *)p->payload)[2] = link->conhdl;
    ((uint8 *)p->payload)[6] = reason;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_reject_connection_request():
 *
 * Used to decline a new incoming connection request.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_reject_connection_request(struct bd_addr *bdaddr, uint8 reason)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_REJECT_CONN_REQ_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_reject_connection_request: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_REJECT_CONN_REQ_OCF, HCI_LINK_CTRL_OGF, HCI_REJECT_CONN_REQ_PLEN);
    /* Assembling cmd prameters */
    memcpy(((uint8 *)p->payload) + 4, bdaddr->addr, 6);
    ((uint8 *)p->payload)[10] = reason;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_pin_code_request_reply():
 *
 * Used to reply to a PIN Code Request event from the Host Controller and specifies
 * the PIN code to use for a connection.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_pin_code_request_reply(struct bd_addr *bdaddr, uint8 pinlen, uint8 *pincode)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_PIN_CODE_REQ_REP_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_pin_code_request_reply: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Reset buffer content just to make sure */
    memset((uint8 *)p->payload, 0, HCI_PIN_CODE_REQ_REP_PLEN);

    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_PIN_CODE_REQ_REP, HCI_LINK_CTRL_OGF, HCI_PIN_CODE_REQ_REP_PLEN);
    /* Assembling cmd prameters */
    memcpy(((uint8 *)p->payload) + 4, bdaddr->addr, 6);
    ((uint8 *)p->payload)[10] = pinlen;
    memcpy(((uint8 *)p->payload) + 11, pincode, pinlen);

    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_pin_code_request_neg_reply():
 *
 * Used to reply to a PIN Code Request event from the Host Controller when the Host
 * cannot specify a PIN code to use for a connection.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_pin_code_request_neg_reply(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_PIN_CODE_REQ_NEG_REP_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_pin_code_request_neg_reply: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_PIN_CODE_REQ_NEG_REP, HCI_LINK_CTRL_OGF, HCI_PIN_CODE_REQ_NEG_REP_PLEN);
    /* Assembling cmd prameters */
    memcpy(((uint8 *)p->payload)+4, bdaddr->addr, 6);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_sniff_mode():
 *
 * Sets an ACL connection to low power Sniff mode.
 */
/*-----------------------------------------------------------------------------------*/

_ATTR_LWBT_UARTIF_CODE_
int hci_sniff_mode(struct bd_addr *bdaddr, uint16 max_interval, uint16 min_interval,
                   uint16 attempt, uint16 timeout)
{
    struct pbuf *p;
    struct hci_link *link;

    /* Check if an ACL connection exists */
    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);

    if(link == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_sniff_mode: ACL connection does not exist\n"));
        return ERR_CONN;
    }

    if((p = pbuf_alloc(PBUF_TRANSPORT, HCI_SNIFF_PLEN, PBUF_RAM)) == NULL)
    {
        /* Alloc len of packet */
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_sniff_mode: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_SNIFF_MODE, HCI_LINK_POLICY, HCI_SNIFF_PLEN);
    /* Assembling cmd prameters */
    ((uint16 *)p->payload)[2] = link->conhdl;
    ((uint16 *)p->payload)[3] = max_interval;
    ((uint16 *)p->payload)[4] = min_interval;
    ((uint16 *)p->payload)[5] = attempt;
    ((uint16 *)p->payload)[6] = timeout;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}


/*-----------------------------------------------------------------------------------*/
/* hci_exit_sniff_mode():
 *
 * Sets an ACL connection to low power Sniff mode.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_exit_sniff_mode(uint16 conhdl)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_EXIT_SNIFF_MODE, HCI_LINK_POLICY, HCI_CMD_HDR_LEN+1+2);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_write_extended_inquiry_response_commad(uint8 FEC_Required, char * response_data )
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_TRANSPORT, 4+241, PBUF_RAM)) == NULL)
    {
        /* Alloc len of packet */
        return ERR_MEM;
    }

    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x52, HCI_HOST_C_N_BB, 4+241);
    /* Assembling cmd prameters */
    ((int8 *)p->payload)[4] = FEC_Required;
    memcpy((int8 *)p->payload+5, response_data, 240);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_write_link_policy_settings():
 *
 * Control the modes (park, sniff, hold) that an ACL connection can take.
 *
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_write_link_policy_settings(struct bd_addr *bdaddr, uint16 link_policy)
{
    struct pbuf *p;
    struct hci_link *link;

    /* Check if an ACL connection exists */
    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);

    if(link == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_write_link_policy_settings: ACL connection does not exist\n"));
        return ERR_CONN;
    }

    if( (p = pbuf_alloc(PBUF_TRANSPORT, HCI_W_LINK_POLICY_PLEN, PBUF_RAM)) == NULL)
    {
        /* Alloc len of packet */
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_write_link_policy_settings: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_LINK_POLICY, HCI_LINK_POLICY, HCI_W_LINK_POLICY_PLEN);

    /* Assembling cmd prameters */
    ((uint16 *)p->payload)[2] = link->conhdl;
    ((uint16 *)p->payload)[3] = link_policy;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_reset():
 *
 * Reset the Bluetooth host controller, link manager, and radio module.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_reset(void)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_RESET_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_reset: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_RESET_OCF, HCI_HC_BB_OGF, HCI_RESET_PLEN);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_set_event_filter():
 *
 * Used by the host to specify different event filters.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_set_event_filter(uint8 filter_type, uint8 filter_cond_type, uint8* cond)
{
    uint8 cond_len = 0x00;
    struct pbuf *p;
    switch(filter_type)
    {
        case 0x00:
            LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_set_event_filter: Clearing all filters\n"));
            cond_len = 0x00;
            break;
        case 0x01:
            switch(filter_cond_type)
            {
                case 0x00:
                    cond_len = 0x00;
                    break;
                case 0x01:
                    cond_len = 0x06;
                    break;
                case 0x02:
                    cond_len = 0x06;
                    break;
                default:
                    LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_set_event_filter: Entered an unspecified filter condition type!\n"));
                    break;
            }
            break;
        case 0x02:
            switch(filter_cond_type)
            {
                case 0x00:
                    cond_len = 0x01;
                    break;
                case 0x01:
                    cond_len = 0x07;
                    break;
                case 0x02:
                    cond_len = 0x07;
                    break;
                default:
                    LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_set_event_filter: Entered an unspecified filter condition type!\n"));
                    break;
            }
            break;
        default:
            LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_set_event_filter: Entered an unspecified filter type!\n"));
            break;
    }

    if((p = pbuf_alloc(PBUF_RAW, HCI_SET_EV_FILTER_PLEN+cond_len, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_set_event_filter: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_SET_EV_FILTER_OCF, HCI_HC_BB_OGF, HCI_SET_EV_FILTER_PLEN+cond_len);
    ((uint8 *)p->payload)[4] = filter_type;
    ((uint8 *)p->payload)[5] = filter_cond_type;
    /* Assembling cmd prameters */
    if(cond_len)
    {
        memcpy(((uint8 *)p->payload)+6, cond, cond_len);
    }
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_write_stored_link_key():
 *
 * Writes a link key to be stored in the Bluetooth host controller.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_write_stored_link_key(struct bd_addr *bdaddr, uint8 *link)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_WRITE_STORED_LINK_KEY_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_write_stored_link_key: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_WRITE_STORED_LINK_KEY, HCI_HC_BB_OGF, HCI_WRITE_STORED_LINK_KEY_PLEN);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = 0x01;
    memcpy(((uint8 *)p->payload) + 5, bdaddr->addr, 6);
    memcpy(((uint8 *)p->payload) + 11, link, 16);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_change_local_name():
 *
 * Writes a link key to be stored in the Bluetooth host controller.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_change_local_name(uint8 *name, uint8 len)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CHANGE_LOCAL_NAME_PLEN + len, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_change_local_name: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_CHANGE_LOCAL_NAME, HCI_HC_BB_OGF, HCI_CHANGE_LOCAL_NAME_PLEN + len);
    /* Assembling cmd prameters */
    memcpy(((uint8 *)p->payload) + 4, name, len);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_write_page_timeout():
 *
 * Define the amount of time a connection request will wait for the remote device
 * to respond before the local device returns a connection failure.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_write_page_timeout(uint16 page_timeout)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_W_PAGE_TIMEOUT_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_write_page_timeout: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_PAGE_TIMEOUT_OCF, HCI_HC_BB_OGF, HCI_W_PAGE_TIMEOUT_PLEN);
    /* Assembling cmd prameters */
    ((uint16 *)p->payload)[2] = page_timeout;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_write_scan_enable():
 *
 * Controls whether or not the Bluetooth device will periodically scan for page
 * attempts and/or inquiry requests from other Bluetooth devices.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
int hci_write_scan_enable(uint8 scan_enable)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_W_SCAN_EN_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_SCAN_EN_OCF, HCI_HC_BB_OGF, HCI_W_SCAN_EN_PLEN);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = scan_enable;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_write_cod():
 *
 * Write the value for the Class_of_Device parameter, which is used to indicate its
 * capabilities to other devices.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_write_cod(uint8 *cod)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_W_COD_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_write_cod: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_COD_OCF, HCI_HC_BB_OGF, HCI_W_COD_PLEN);
    /* Assembling cmd prameters */
    memcpy(((uint8 *)p->payload)+4, cod, 3);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_set_hc_to_h_fc():
 *
 * Used by the Host to turn flow control on or off in the direction from the Host
 * Controller to the Host.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_set_hc_to_h_fc(void)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_SET_HC_TO_H_FC_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_set_hc_to_h_fc: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_SET_HC_TO_H_FC_OCF, HCI_HC_BB_OGF, HCI_SET_HC_TO_H_FC_PLEN);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = 0x01; /* Flow control on for HCI ACL Data Packets and off for HCI
                     SCO Data Packets in direction from Host Controller to
                     Host */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_host_buffer_size():
 *
 * Used by the Host to notify the Host Controller about the maximum size of the data
 * portion of HCI ACL Data Packets sent from the Host Controller to the Host.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_host_buffer_size(void)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_H_BUF_SIZE_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_host_buffer_size: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_H_BUF_SIZE_OCF, HCI_HC_BB_OGF, HCI_H_BUF_SIZE_PLEN);
    ((uint16 *)p->payload)[2] = HCI_HOST_ACL_MAX_LEN; /* Host ACL data packet maximum length */
    ((uint8 *)p->payload)[6] = 255; /* Host SCO Data Packet Length */
    *((uint16 *)(((uint8 *)p->payload)+7)) = HCI_HOST_MAX_NUM_ACL; /* Host max total num ACL data packets */
    ((uint16 *)p->payload)[4] = 1; /* Host Total Num SCO Data Packets */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    pcb->host_num_acl = HCI_HOST_MAX_NUM_ACL;

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_host_num_comp_packets():
 *
 * Used by the Host to indicate to the Host Controller the number of HCI Data Packets
 * that have been completed for each Connection Handle since the previous
 * Host_Number_Of_Completed_Packets command was sent to the Host Controller.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_host_num_comp_packets(uint16 conhdl, uint16 num_complete)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_H_NUM_COMPL_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_host_num_comp_packets: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_H_NUM_COMPL_OCF, HCI_HC_BB_OGF, HCI_H_NUM_COMPL_PLEN);
    ((uint16 *)p->payload)[2] = conhdl;
    ((uint16 *)p->payload)[3] = num_complete; /* Number of completed acl packets */

    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    pcb->host_num_acl += num_complete;

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_read_buffer_size():
 *
 * Used to read the maximum size of the data portion of HCI ACL packets sent from the
 * Host to the Host Controller.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_read_buffer_size(void)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_R_BUF_SIZE_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_read_buffer_size: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_R_BUF_SIZE_OCF, HCI_INFO_PARAM_OGF, HCI_R_BUF_SIZE_PLEN);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* hci_read_bd_addr():
 *
 * Used to retreive the Bluetooth address of the host controller.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_read_bd_addr(int (* rbd_complete)(void *arg, struct bd_addr *bdaddr))
{
    struct pbuf *p;

    pcb->rbd_complete = rbd_complete;

    if((p = pbuf_alloc(PBUF_RAW, HCI_R_BD_ADDR_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("hci_read_buffer_size: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_READ_BD_ADDR, HCI_INFO_PARAM_OGF, HCI_R_BD_ADDR_PLEN);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* lp_write_flush_timeout():
 *
 * Called by L2CAP to set the flush timeout for the ACL link.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int lp_write_flush_timeout(struct bd_addr *bdaddr, uint16 flushto)
{
    struct hci_link *link;
    struct pbuf *p;

    /* Check if an ACL connection exists */
    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);

    if(link == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_write_flush_timeout: ACL connection does not exist\n"));
        return ERR_CONN;
    }

    if((p = pbuf_alloc(PBUF_TRANSPORT, HCI_W_FLUSHTO_PLEN, PBUF_RAM)) == NULL)
    {
        /* Alloc len of packet */
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_write_flush_timeout: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_FLUSHTO, HCI_HC_BB_OGF, HCI_W_FLUSHTO_PLEN);
    /* Assembling cmd prameters */
    ((uint16 *)p->payload)[2] = link->conhdl;
    ((uint16 *)p->payload)[3] = flushto;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* lp_connect_req():
 *
 * Called by L2CAP to cause the Link Manager to create a connection to the
 * Bluetooth device with the BD_ADDR specified by the command parameters.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int lp_connect_req(struct bd_addr *bdaddr, uint8 allow_role_switch)
{
#if 1
    uint8 page_scan_repetition_mode, page_scan_mode;
    uint16 clock_offset;
    struct pbuf *p;
    struct hci_link *link = hci_new();
    struct hci_inq_res *inqres;
    LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("Enter lp_connect_req\n"));
    if(link == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_connect_req: Could not allocate memory for link\n"));
        return ERR_MEM; /* Could not allocate memory for link */
    }
    link->link_type = HCI_CONNECT_TYPE_ACL;
    SET_HCI_FLAG(link->link_flag,HCI_FLAG_IR);
    bd_addr_set(&(link->bdaddr), bdaddr);
    HCI_REG(&(hci_active_links), link);


    /* Check if module has been discovered in a recent inquiry */
    for(inqres = pcb->ires; inqres != NULL; inqres = inqres->next)
    {
        if(bd_addr_cmp(&inqres->bdaddr, bdaddr))
        {
            page_scan_repetition_mode = inqres->psrm;
            page_scan_mode = inqres->psm;
            clock_offset = inqres->co;
            break;
        }
    }
    if(inqres == NULL)
    {
        /* No information on parameters from an inquiry. Using default values */
        page_scan_repetition_mode = 0x01; /* Assuming worst case: time between
                     successive page scans starting
                     <= 2.56s */
        page_scan_mode = 0x00; /* Assumes the device uses mandatory scanning, most
                  devices use this. If no conn is established, try
                  again w this parm set to optional page scanning */
        clock_offset = 0x00; /* If the device was not found in a recent inquiry
                this  information is irrelevant */
    }

    if((p = pbuf_alloc(PBUF_RAW, HCI_CREATE_CONN_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_connect_req: Could not allocate memory for pbuf\n"));
        return ERR_MEM; /* Could not allocate memory for pbuf */
    }

    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_CREATE_CONN_OCF, HCI_LINK_CTRL_OGF, HCI_CREATE_CONN_PLEN);
    /* Assembling cmd prameters */
    memcpy(((uint8 *)p->payload)+4, bdaddr->addr, 6);

    ((uint16 *)p->payload)[5] = HCI_PACKET_TYPE;
    ((uint8 *)p->payload)[12] = page_scan_repetition_mode;
    ((uint8 *)p->payload)[13] = page_scan_mode;
    ((uint16 *)p->payload)[7] = clock_offset;
    ((uint8 *)p->payload)[16] = allow_role_switch;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("Leaving lp_connect_req\n"));
    return ERR_OK;
#endif

}

_ATTR_LWBT_UARTIF_CODE_
int hci_create_connection_cancel_command(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    struct hci_link *link;

    /* Check if an ACL connection exists */
    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);
    if(link == NULL)
    {
        return ERR_ISCONN;
    }

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+6, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x08, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+6);

    memcpy(((uint8 *)p->payload) + 4, bdaddr->addr, 6);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}


_ATTR_LWBT_UARTIF_CODE_
int l2cap_send_connectionless(uint16 handle, uint16 cid, uint8 *data, uint16 len)
{
    struct hci_link *link;
    struct hci_acl_hdr *aclhdr;
    uint timeout;
    struct pbuf *p;
    if(pcb->hc_num_acl == 0)
    {
        return ERR_BUF;
    }

    if((p = pbuf_alloc(PBUF_RAW, len+9, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    ((uint8*)p->payload)[0] = HCI_ACL_DATA_PACKET;
    aclhdr = (void *)(((uint8*)p->payload)+1);
    aclhdr->conhdl_pb_bc = handle; /* Received from connection complete event */
    aclhdr->conhdl_pb_bc |= L2CAP_ACL_START << 12; /* Packet boundary flag */
    aclhdr->conhdl_pb_bc &= 0x3FFF; /* Point-to-point */
    aclhdr->len = len+4;
    ((uint8 *)p->payload)[5] = (len) & 0xFF;
    ((uint8 *)p->payload)[6] = (len>>8) & 0xFF;
    ((uint8 *)p->payload)[7] = cid & 0xFF;
    ((uint8 *)p->payload)[8] = (cid>> 8) & 0xFF;

    memcpy((uint8 *)p->payload+9, data, len);
    _phybusif_output(p, p->len, NULL);

    HciServeIsrDisable();
    --pcb->hc_num_acl;
    HciServeIsrEnable();

    /* Free ACL header. Upper layers will handle rest of packet */
    //p = pbuf_dechain(q);
    pbuf_free(p);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* lp_acl_write():
 *
 * Called by L2CAP to send data to the Host Controller that will be transfered over
 * the ACL link from there.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int lp_acl_write(struct bd_addr *bdaddr, struct pbuf *p, uint16 len, uint8 pb, void (*func)(void))
{
#if 1
    struct hci_link *link;
    _ATTR_LWBT_DATA_
    static struct hci_acl_hdr *aclhdr;
    struct pbuf *q;
    uint timeout;
    uint needcopy = 0;
    if(pb & 0x80)
    {
        needcopy = 1;
        pb = pb & 0x7F;
    }
    /* Check if an ACL connection exists */
    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);

    if(link == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_acl_write: ACL connection does not exist\n"));
        return ERR_CONN;
    }

    LWBT_DEBUGF(HCI_DEBUG, _DBG_DUMP_, ("lp_acl_write: HC num ACL %d\n", pcb->hc_num_acl));
    /* H5 already have flow control, in consequence close the hci flow control. */
#if (BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H4)
    if (pcb->hc_num_acl == 0)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_acl_write: HC out of buffer space\n"));

        if(pcb->flush_timer_enable ==0)
        {
            pcb->flush_timer_enable = 1;
            pcb->flush_timer_timeout = GetSysTick();

            pcb->flush_handle = link->conhdl;
        }
#if HCI_FLOW_QUEUEING
        if(p != NULL)
        {
            /* Packet can be queued? */
            if(link->p != NULL)
            {
                LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_acl_write: Host buffer full. Dropped packet\n"));
                return ERR_BUF; /* Drop packet */
            }
            else
            {
                /* Copy PBUF_REF referenced payloads into PBUF_RAM */
                //p = pbuf_take(p);
                /* Remember pbuf to queue, if any */
                link->p = p;
                link->len = len;
                link->pb = pb;
                link->func = func;
                /* Pbufs are queued, increase the reference count */
                pbuf_ref(p);
                LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_acl_write: Host queued packet %p\n", (void *)p));

                return ERR_OK;
            }
        }
#else
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_acl_write: Dropped packet\n"));
#endif
        return ERR_BUF;
    }
#endif
    if(needcopy)
    {
        if((q = pbuf_alloc(PBUF_RAW, 1+HCI_ACL_HDR_LEN, PBUF_RAM)) == NULL)
        {
            /* Could not allocate memory for pbuf */
            LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("lp_acl_write: Could not allocate memory for pbuf\n"));
            return ERR_MEM;
        }
        pbuf_ncopy(q, p, len);
        //pbuf_copy(q, p);
        //q->len = 1+HCI_ACL_HDR_LEN+len;
        //q->tot_len = 1+HCI_ACL_HDR_LEN+len;
        LWBT_DEBUGF(HCI_DEBUG, _DBG_DUMP_, ("lp_acl_write: copy pbuf\n"));
    }
    else
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_DUMP_, ("lp_acl_write: zero copy\n"));
        q = p;
        pbuf_ref(q);
        pbuf_header(q,1+HCI_ACL_HDR_LEN);
    }

    ((uint8*)q->payload)[0] = HCI_ACL_DATA_PACKET;
    aclhdr = (void *)(((uint8*)q->payload)+1);
    aclhdr->conhdl_pb_bc = link->conhdl; /* Received from connection complete event */
    aclhdr->conhdl_pb_bc |= pb << 12; /* Packet boundary flag */
    aclhdr->conhdl_pb_bc &= 0x3FFF; /* Point-to-point */
    aclhdr->len = len;

    LWBT_DEBUGF(HCI_DEBUG, _DBG_DUMP_, ("lp_acl_write: q->tot_len = %d aclhdr->len + q->len = %d\n", q->tot_len, aclhdr->len + q->len));

    _phybusif_output(q, aclhdr->len + q->len, func);

    HciServeIsrDisable();
    ++hci_data_send;
    --pcb->hc_num_acl;
    HciServeIsrEnable();

    /* Free ACL header. Upper layers will handle rest of packet */
    //p = pbuf_dechain(q);
    pbuf_free(q);
    return ERR_OK;
#endif
}



/*-----------------------------------------------------------------------------------*/
/* lp_sco_write():
 *
 * Called by L2CAP to send data to the Host Controller that will be transfered over
 * the ACL link from there.
 */
/*-----------------------------------------------------------------------------------*/
#ifdef _A2DP_SINK_
_ATTR_LWBT_CODE_
int lp_sco_write(uint8 *data, uint16 len, void (*func)(void))
{

    struct hci_link *link;
    struct hci_sco_hdr *scohdr;
    struct pbuf *q;
    uint8  hdrbuf[4];
    /* Check if an SCO connection exists */

    for(link = hci_active_links; link != NULL; link = link->next)
    {
        if(link->link_type == HCI_CONNECT_TYPE_ESCO || link->link_type == HCI_CONNECT_TYPE_SCO)
        {
            break;
        }
    }
    if(link == NULL)
    {
        return ERR_CONN;
    }

    hdrbuf[0] = 0x03;
    scohdr = (void *)(hdrbuf+1);
    scohdr->conhdl_ps= link->conhdl; /* Received from connection complete event */
    /// scohdr->conhdl_pb_bc |= pb << 12; /* Packet boundary flag */
    scohdr->conhdl_ps &= 0xFFF; /* Point-to-point */
    scohdr->len = len;
    phybusif_sco_output(hdrbuf, data, len, func);

    return ERR_OK;

}
#endif

/*-----------------------------------------------------------------------------------*/
/* lp_is_connected():
 *
 * Called by L2CAP to check if an active ACL connection exists for the specified
 * Bluetooth address.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
uint8 lp_is_connected(struct bd_addr *bdaddr)
{
    struct hci_link *link;

    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);

    if(link == NULL)
    {
        return 0;
    }
    return 1;
}
/*-----------------------------------------------------------------------------------*/
/* lp_pdu_maxsize():
 *
 * Called by L2CAP to check the maxsize of the PDU. In this case it is the largest
 * ACL packet that the Host Controller can buffer.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
uint16 lp_pdu_maxsize(void)
{
    return pcb->maxsize;
}
/*-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------*/
/* hci_write_page_scan_activity():
 *
 * scan_interva 0x0012 to 0x1000 time = N*0.625
 * scan_window  0x0011 to 0x1000 time = N*0.625
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_write_page_scan_activity(uint16 scan_interva, uint16 scan_window)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_W_PAGE_SCAN_ACTIVITY_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_PAGE_SCAN_ACTIVITY_OCF, HCI_HC_BB_OGF, HCI_W_PAGE_SCAN_ACTIVITY_PLEN);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = scan_interva & 0xFF;
    ((uint8 *)p->payload)[5] = (scan_interva>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = scan_window & 0xFF;
    ((uint8 *)p->payload)[7] = (scan_window>>8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/* hci_write_inquiry_scan_activity():
 *
 * scan_interva 0x0012 to 0x1000 time = N*0.625
 * scan_window  0x0011 to 0x1000 time = N*0.625
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_write_inquiry_scan_activity(uint16 scan_interva, uint16 scan_window)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_W_INQUIRY_SCAN_ACTIVITY_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_INQUIRY_SCAN_ACTIVITY_OCF, HCI_HC_BB_OGF, HCI_W_INQUIRY_SCAN_ACTIVITY_PLEN);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = scan_interva & 0xFF;
    ((uint8 *)p->payload)[5] = (scan_interva>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = scan_window & 0xFF;
    ((uint8 *)p->payload)[7] = (scan_window>>8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}



/*-----------------------------------------------------------------------------------*/
/* Accept Connection Request Command():
 *
 * bdaddr: BD_ADDR of the Device to be connected
 * role  :0x00 Become the Master for this connect ion. The LM will  perform the role switch
 *
 *
 *        0x01 Remain the Slave for this connection. The LM will NOT perform the role  switch
 *
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
int hci_accept_connection_Request(struct bd_addr *bdaddr, uint8 role)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_ACCEPT_CONNECTION_REQUEST_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_ACCEPT_CONNECTION_REQUEST_OCF, HCI_LINK_CTRL_OGF, HCI_ACCEPT_CONNECTION_REQUEST_PLEN);
    /* Assembling cmd prameters */
    memcpy(((uint8 *)p->payload) + 4, bdaddr->addr, 6);
    ((uint8 *)p->payload)[10] = role;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);


    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/* Write Connection Accept Timeout Command():
 *
 * bdaddr: BD_ADDR of the Device to be connected
 * role  :0x00 Become the Master for this connect ion. The LM will  perform the role switch
 *
 *
 *        0x01 Remain the Slave for this connection. The LM will NOT perform the role  switch
 *
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_write_connection_timeout(uint16 timeout)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_W_CONNECTION_ACCEPT_TIMEOUT_PLEN, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_W_CONNECTION_ACCEPT_TIMEOUT_OCF, HCI_HC_BB_OGF, HCI_W_CONNECTION_ACCEPT_TIMEOUT_PLEN);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = timeout & 0xFF;
    ((uint8 *)p->payload)[5] = (timeout>> 8) & 0xFF;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);


    return ERR_OK;
}


_ATTR_LWBT_CODE_
int hci_link_key_request_reply(struct bd_addr *bdaddr, uint8 * key)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+22, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_LINK_KEY_REQUEST_REPLY, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+22);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    memcpy((uint8 *)p->payload+10, key, 16);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);


    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_link_key_request_negative_reply(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+6, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_LINK_KEY_REQUEST_NEGATIVE, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+6);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);


    return ERR_OK;
}

_ATTR_LWBT_CODE_
err_t hci_accpet_synchronus_connection_request(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    uint32 Transmit_Bandwidth = 0x00001F40;
    uint32 Receive_Bandwidth = 0x00001F40;
    uint16 Content_Format = 0x0060;
    uint16 Packet_Type = 0xFC3F;
    uint16 Max_Latency = 0x000A;
    uint8  Retransmission_Effort = 0x01;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+21, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+21);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);


    memcpy((uint8 *)p->payload+10, &Transmit_Bandwidth, 4);
    memcpy((uint8 *)p->payload+14, &Receive_Bandwidth, 4);
    memcpy((uint8 *)p->payload+18, &Max_Latency, 2);


    ((uint8 *)p->payload)[20] = Content_Format & 0xFF;
    ((uint8 *)p->payload)[21] = (Content_Format>> 8) & 0xFF;

    ((uint8 *)p->payload)[22] = Retransmission_Effort;
    ((uint8 *)p->payload)[23] = Packet_Type & 0xFF;
    ((uint8 *)p->payload)[24] = (Packet_Type>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);


    return ERR_OK;
}




/*-----------------------------------------------------------------------------------*/
/* hci_Reject_synchronus_connection_request():
 *
 * bdaddr: BD_ADDR of the Device to be connected
 * Reason  :Connection Rejected due to Limited Resources (0X0D)
 *         Connection Rejected due to Secu rity Reasons (0X0E)
 *
 *        Connection Rejected due to Unacceptable BD_ADDR (0X0F)
 *
 /*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t hci_Reject_synchronus_connection_request(struct bd_addr *bdaddr , uint8 Reason)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+7, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+7);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);

    ((uint8 *)p->payload)[10] = Reason;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);


    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_write_simple_pairing_mode(uint8 simple_pairing_enable)
{

    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_WRITE_SIMPLE_PAIRING_MODE, HCI_HC_BB_OGF, HCI_CMD_HDR_LEN+1+1);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = simple_pairing_enable;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;

}





_ATTR_LWBT_CODE_
int hci_Io_Capability_req_reply(struct bd_addr *bdaddr, uint8 Io_Capability, uint8 OOB_Data, uint8 Auth_Req)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+9, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_IO_CAPABILITY_REQUEST_REPLY, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+9);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    ((uint8 *)p->payload)[10] = Io_Capability;
    ((uint8 *)p->payload)[11] = OOB_Data;
    ((uint8 *)p->payload)[12] = Auth_Req;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);


    return ERR_OK;
}


_ATTR_LWBT_CODE_
int hci_Io_Capability_req_negative_reply(struct bd_addr *bdaddr, uint8 Reason)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+7, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_IO_CAPABILITY_REQUEST_NEGATIVE, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+6);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    ((uint8 *)p->payload)[10] = Reason;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_user_confirmati_req_reply(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+6, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_USER_CONFIRMATI_REQUEST_REPLY, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+6);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_user_confirmati_req_negative_reply(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+6, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, HCI_USER_CONFIRMATI_REQUEST_NEGATIVE_REPLY, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+6);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}


_ATTR_LWBT_INIT_CODE_
int hci_Read_Remote_Extended_Features(uint16 conhdl, uint8 Page_Number)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+3, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1C, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+3);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;

    ((uint8 *)p->payload)[6] = Page_Number;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}


_ATTR_LWBT_CODE_
int hci_Authentication_Requested(uint16 conhdl)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x11, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+2);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_set_conn_encrypt(uint16 conhdl, uint8 isable)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+3, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x13, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+3);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = isable;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}


_ATTR_LWBT_CODE_
int hci_read_remote_features(uint16 conhdl)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1B, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+2);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_read_remote_extended_features(uint16 conhdl, uint8 page)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+3, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1C, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+3);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = page;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_read_remote_name(struct bd_addr *bdaddr, uint8 psrm , uint16 clk_offset)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+10, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x19, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+10);
    /* Assembling cmd prameters */
    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    ((uint8 *)p->payload)[10] = psrm;
    ((uint8 *)p->payload)[11] = 0;
    ((uint8 *)p->payload)[12] = clk_offset & 0xFF;
    ((uint8 *)p->payload)[13] = (clk_offset>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_remote_name_request_cancle(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+6, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1A, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1+6);
    /* Assembling cmd prameters */
    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_inquiry_cancel(void)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x02, HCI_LINK_CTRL_OGF, HCI_CMD_HDR_LEN+1);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}



_ATTR_LWBT_CODE_
int hci_flush_command(uint16 conhdl)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x08, HCI_HC_BB_OGF, HCI_CMD_HDR_LEN+1+2);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}


/*-----------------------------------------------------------------------------------*/
/* hci_delete_link_key():
 *
 * delete_all_flag 0: Delete only the Link Key for specified BD_ADDR
                   1: Delete all stored Link Keys
 */
/*-----------------------------------------------------------------------------------*/

_ATTR_LWBT_CODE_
int hci_delete_link_key(struct bd_addr *bdaddr, uint8 delete_all_flag)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+7, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x12, HCI_HC_BB_OGF, HCI_CMD_HDR_LEN+1+7);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, bdaddr->addr, 6);
    ((uint8 *)p->payload)[10] = delete_all_flag;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_set_event_mask(long long event_mask)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+8, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x01, HCI_HC_BB_OGF, HCI_CMD_HDR_LEN+1+8);
    /* Assembling cmd prameters */

    memcpy((uint8 *)p->payload+4, &event_mask, 8);
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_write_default_link_policy_setting(uint16 setting)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x0F, 0x02, HCI_CMD_HDR_LEN+1+2);
    /* Assembling cmd prameters */

    ((uint8 *)p->payload)[4] = setting & 0xFF;
    ((uint8 *)p->payload)[5] = (setting>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);

    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_enable_device_under_test_mode_command(void)
{
    //0x01, 0x03, 0x18, 0x0, // test mode

    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = 0x03;
    ((uint8 *)p->payload)[2] = 0x18;
    ((uint8 *)p->payload)[3] = 0x0;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_read_rssi_command(struct bd_addr *bdaddr)
{
    struct pbuf *p;
    struct hci_link * link;
    uint16 conhdl;
    link = hci_get_link(bdaddr, HCI_CONNECT_TYPE_ACL);
    if(link)
    {
        conhdl = link->conhdl;
        if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
        {
            LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
            return ERR_MEM;
        }
        /* Assembling command packet */
        p = hci_cmd_ass(p, 0x05, 0x05, HCI_CMD_HDR_LEN+1+2);
        /* Assembling cmd prameters */

        ((uint8 *)p->payload)[4] = conhdl & 0xFF;
        ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
        phybusif_output(p, p->tot_len);

        pbuf_free(p);
    }
    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_write_inquiry_mode_command(uint8 inquiry_mode)
{
    struct pbuf *p;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x45, HCI_HOST_C_N_BB, HCI_CMD_HDR_LEN+1+1);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = inquiry_mode;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

_ATTR_LWBT_UARTIF_CODE_
int hci_get_link_mode(struct bd_addr *bdaddr, uint8 link_type)
{
    struct hci_link * link;
    link = hci_get_link(bdaddr,link_type);
    return link->Mode;
}

_ATTR_LWBT_UARTIF_CODE_
int hci_write_link_supervision_timeout(uint16 conhdl, uint16 timeout,struct bd_addr *addr)
{
    struct pbuf *p;
    if(conhdl == 0)
    {
        struct hci_link * link;
        link = hci_get_link(addr,HCI_CONNECT_TYPE_ACL);
        conhdl = link->conhdl;
    }
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+4, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x37, HCI_HOST_C_N_BB, HCI_CMD_HDR_LEN+1+4);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = timeout & 0xFF;
    ((uint8 *)p->payload)[7] = (timeout>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

#ifdef HAVE_BLE

// get addr type and address used in advertisement packets
_ATTR_LWBT_CODE_
void hci_le_advertisement_address(uint8 * addr_type, struct bd_addr *addr)
{
    *addr_type = pcb->adv_addr_type;
    if (pcb->adv_addr_type)
    {
        //memcpy(addr, &pcb->adv_address, 6);
        bt_flip_addr(addr, &pcb->adv_address);
    }
    else
    {
        //memcpy(addr, &pcb->bdaddr, 6);
        bt_flip_addr(addr, &pcb->bdaddr);
    }
}

/*-----------------------------------------------------------------------------------*/
/* hci_write_le_host_supported_command():
 *
 * LE_Supported_Host :
   0x00 LE Supported (Host) disabled (default)
   0x01 LE Supported (Host) enabled
   Simultaneous_LE_Host :
   Value Parameter Description
   0x00 Simultaneous LE and BR/EDR to Same Device Capable (Host)
   disabled (default)
   0x01 Simultaneous LE and BR/EDR to Same Device Capable (Host)
   enabled
 * attempts and/or inquiry requests from other Bluetooth devices.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
int hci_write_le_host_supported_command(uint8 LE_Supported_Host,uint8 Simultaneous_LE_Host)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x6d, HCI_HOST_C_N_BB, HCI_CMD_HDR_LEN+1+2);
    ((uint8 *)p->payload)[4] = LE_Supported_Host;
    ((uint8 *)p->payload)[5] =  Simultaneous_LE_Host;
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_le_set_event_mask_command(long long mask_flag)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+8, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x01, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+8);
    /* Assembling cmd prameters */
    memcpy((uint8 *)p->payload+4, &mask_flag, 8);
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_le_read_buffer_size_command(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x02, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */

    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_le_read_local_supported_features_command(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x03, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */

    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_le_set_random_address_command(struct bd_addr *bdaddr)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+6, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x05, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+6);
    /* Assembling cmd prameters */
    memcpy((uint8 *)p->payload+4, &bdaddr->addr, 6);
    memcpy(&pcb->adv_address, &bdaddr->addr, 6);
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_UARTIF_CODE_
int hci_le_advertising_parameters_command(uint16 Advertising_Interval_Min,
        uint16 Advertising_Interval_Max,
        uint8 Advertising_Type,
        uint8 Own_Address_Type,
        uint8 Direct_Address_Type,
        struct bd_addr *Direct_Address,
        uint8 Advertising_Channel_Map,
        uint8 Advertising_Filter_Policy
                                         )
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+15, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x06, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+15);
    ((uint8 *)p->payload)[4] = Advertising_Interval_Min & 0xFF;
    ((uint8 *)p->payload)[5] = (Advertising_Interval_Min>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = Advertising_Interval_Max & 0xFF;
    ((uint8 *)p->payload)[7] = (Advertising_Interval_Max>> 8) & 0xFF;
    ((uint8 *)p->payload)[8] = Advertising_Type;

    pcb->adv_addr_type = Own_Address_Type;
    ((uint8 *)p->payload)[9] = Own_Address_Type;
    ((uint8 *)p->payload)[10] = Direct_Address_Type;
    /* Assembling cmd prameters */
    memcpy((uint8 *)p->payload+11, &Direct_Address->addr, 6);
    ((uint8 *)p->payload)[17] = Advertising_Channel_Map;
    ((uint8 *)p->payload)[18] = Advertising_Filter_Policy;
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
int hci_le_read_advertising_channel_tx_power_command(struct bd_addr *bdaddr)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x07, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}


_ATTR_LWBT_INIT_CODE_
int hci_le_set_advertising_data_command(uint8 Advertising_Data_Length, uint8* Advertising_Data)
{

    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+32, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x08, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+32);
    ((uint8 *)p->payload)[4] = Advertising_Data_Length;
    /* Assembling cmd prameters */
    memcpy((uint8 *)p->payload+5, Advertising_Data, 31);
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;

}

_ATTR_LWBT_INIT_CODE_
int hci_le_set_response_data_command(uint8 Scan_Response_Data_Length, uint8* Scan_Response_Data)
{

    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+32, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x09, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+32);
    ((uint8 *)p->payload)[4] = Scan_Response_Data_Length;
    /* Assembling cmd prameters */
    memcpy((uint8 *)p->payload+5, Scan_Response_Data, 31);
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;

}




_ATTR_LWBT_UARTIF_CODE_
int hci_le_set_advertise_enable_command(uint8 Advertising_Enable)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x0A, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+1);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = Advertising_Enable;
    phybusif_output(p, p->tot_len);

    pbuf_free(p);
}


#ifndef BLE_SMALL_MEMROY
_ATTR_LWBT_CODE_
int hci_le_scan_parameters_command(uint8 LE_Scan_Type, uint16 LE_Scan_Interval, uint16 LE_Scan_Window, uint8 Own_Address_Type, uint8 Scanning_Filter_Policy )
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+23, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x0B, HCI_BLE_OGF, HCI_CMD_HDR_LEN+23);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = LE_Scan_Type;
    ((uint8 *)p->payload)[5] = LE_Scan_Interval & 0xFF;
    ((uint8 *)p->payload)[6] = (LE_Scan_Interval>> 8) & 0xFF;
    ((uint8 *)p->payload)[7] = LE_Scan_Window & 0xFF;
    ((uint8 *)p->payload)[8] = (LE_Scan_Window>> 8) & 0xFF;
    ((uint8 *)p->payload)[9] = Own_Address_Type;
    ((uint8 *)p->payload)[10] = Scanning_Filter_Policy;
    phybusif_output(p, p->tot_len);

    pbuf_free(p);
}

/*-----------------------------------------------------------------------------------*/
/* hci_le_set_scan_enable_command():
 *
 * LE_Scan_Enable :
   0x00 Scanning disabled.
   0x01 Scanning enabled.
   Filter_Duplicates :
   0x00 Duplicate filtering disabled.
   0x01 Duplicate filtering enabled.
 *
 */
/*-----------------------------------------------------------------------------------*/

int hci_le_set_scan_enable_command(uint8 LE_Scan_Enable, uint8 Filter_Duplicates)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x0C, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+2);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = LE_Scan_Enable;
    ((uint8 *)p->payload)[5] = Filter_Duplicates;
    phybusif_output(p, p->tot_len);

    pbuf_free(p);
}

_ATTR_LWBT_CODE_
int hci_le_create_connection_command(struct bd_addr *bdaddr, uint8 address_type)
{
    struct pbuf *p;
    uint16 scan_interval = 1000;
    uint16 scan_windos = 1000;
    uint8  Initiator_Filter_Policy = 0;
    //uint8  Peer_Address_Type;
    //struct bd_addr *bdaddr,
    uint8  Own_Address_Type = 0;
    uint16 Conn_Interval_Min = 80;
    uint16 Conn_Interval_Max = 80;
    uint16 Conn_Latency = 0;
    uint16 Supervision_Timeout = 2000;
    uint16 Minimum_CE_Length = 0;
    uint16 Maximum_CE_Length = 1000;
    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+23, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x0D, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+23);
    /* Assembling cmd prameters */
    ((uint8 *)p->payload)[4] = scan_interval & 0xFF;
    ((uint8 *)p->payload)[5] = (scan_interval>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = scan_windos & 0xFF;
    ((uint8 *)p->payload)[7] = (scan_windos>> 8) & 0xFF;
    ((uint8 *)p->payload)[8] = Initiator_Filter_Policy;
    ((uint8 *)p->payload)[9] = address_type;
    memcpy((uint8 *)p->payload+10, &bdaddr->addr, 6);
    ((uint8 *)p->payload)[16] = Own_Address_Type;
    ((uint8 *)p->payload)[17] = Conn_Interval_Min & 0xFF;
    ((uint8 *)p->payload)[18] = (Conn_Interval_Min>> 8) & 0xFF;
    ((uint8 *)p->payload)[19] = Conn_Interval_Max & 0xFF;
    ((uint8 *)p->payload)[20] = (Conn_Interval_Max>> 8) & 0xFF;
    ((uint8 *)p->payload)[21] = Conn_Latency & 0xFF;
    ((uint8 *)p->payload)[22] = (Conn_Latency>> 8) & 0xFF;
    ((uint8 *)p->payload)[21] = Supervision_Timeout & 0xFF;
    ((uint8 *)p->payload)[22] = (Supervision_Timeout>> 8) & 0xFF;
    ((uint8 *)p->payload)[23] = Minimum_CE_Length & 0xFF;
    ((uint8 *)p->payload)[24] = (Minimum_CE_Length>> 8) & 0xFF;
    ((uint8 *)p->payload)[25] = Maximum_CE_Length & 0xFF;
    ((uint8 *)p->payload)[26] = (Maximum_CE_Length>> 8) & 0xFF;
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

}


_ATTR_LWBT_CODE_
int hci_le_create_connection_cancel_command(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x0E, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
_ATTR_LWBT_CODE_
int hci_le_read_white_list_size_command(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x0F, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
#endif

_ATTR_LWBT_INIT_CODE_
int hci_le_clear_white_list_command(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x10, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
#ifndef BLE_SMALL_MEMROY
_ATTR_LWBT_CODE_
int hci_le_add_device_to_white_list_command(uint8 Address_Type, struct bd_addr *Address)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x11, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+7);

    ((uint8 *)p->payload)[4] = Address_Type;
    memcpy((uint8 *)p->payload+5, &Address->addr, 6);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_le_remove_device_from_white_list_command(uint8 Address_Type, struct bd_addr *Address)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x12, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+7);
    ((uint8 *)p->payload)[4] = Address_Type;
    memcpy((uint8 *)p->payload+5, &Address->addr, 6);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
#endif


_ATTR_LWBT_CODE_
int hci_le_connection_update_command(uint16 Connection_Handle,
                                     uint16 Conn_Interval_Min,
                                     uint16 Conn_Interval_Max,
                                     uint16 Conn_Latency,
                                     uint16 Supervision_Timeout,
                                     uint16 Minimum_CE_Length,
                                     uint16 Maximum_CE_Length)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+14, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x13, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+14);
    ((uint8 *)p->payload)[4] = Connection_Handle & 0xFF;
    ((uint8 *)p->payload)[5] = (Connection_Handle>> 8) & 0xFF;
    ((uint8 *)p->payload)[6] = Conn_Interval_Min & 0xFF;
    ((uint8 *)p->payload)[7] = (Conn_Interval_Min>> 8) & 0xFF;
    ((uint8 *)p->payload)[8] = Conn_Interval_Max & 0xFF;
    ((uint8 *)p->payload)[9] = (Conn_Interval_Max>> 8) & 0xFF;
    ((uint8 *)p->payload)[10] = Conn_Latency & 0xFF;
    ((uint8 *)p->payload)[11] = (Conn_Latency>> 8) & 0xFF;
    ((uint8 *)p->payload)[12] = Supervision_Timeout & 0xFF;
    ((uint8 *)p->payload)[13] = (Supervision_Timeout>> 8) & 0xFF;
    ((uint8 *)p->payload)[14] = Minimum_CE_Length & 0xFF;
    ((uint8 *)p->payload)[15] = (Minimum_CE_Length>> 8) & 0xFF;
    ((uint8 *)p->payload)[16] = Maximum_CE_Length & 0xFF;
    ((uint8 *)p->payload)[17] = (Maximum_CE_Length>> 8) & 0xFF;
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
#ifndef BLE_SMALL_MEMROY
_ATTR_LWBT_CODE_
int hci_le_set_host_chanel_classification_command(long long Channel_Map)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+5, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x14, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+5);
    memcpy((uint8 *)p->payload+4,&Channel_Map, 5);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_le_read_channel_map_commad(long long Connection_Handle)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x15, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+2);
    ((uint8 *)p->payload)[4] = Connection_Handle & 0xFF;
    ((uint8 *)p->payload)[5] = (Connection_Handle>> 8) & 0xFF;
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
_ATTR_LWBT_CODE_
int hci_le_read_remote_used_features_command(uint16 conhdl)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x16, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+2);
    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}



_ATTR_LWBT_CODE_
int hci_le_encrypt_command(uint8 *Key, uint8 *Plaintext_Data)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+32, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x17, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+32);
    memcpy((uint8 *)p->payload+4,Key, 16);
    memcpy((uint8 *)p->payload+20, Plaintext_Data, 16);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_le_rand_command (void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x18, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}



_ATTR_LWBT_CODE_
int hci_le_start_encryption_command(uint16 conhdl, long long Random_Number, uint16  Encrypted_Diversifier, uint8 *Long_Term_Key)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+28, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x19, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+28);
    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    memcpy((uint8 *)p->payload+6, &Random_Number, 8);
    memcpy((uint8 *)p->payload+12,&Encrypted_Diversifier, 2);
    memcpy((uint8 *)p->payload+14, Long_Term_Key, 16);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
_ATTR_LWBT_CODE_
int hci_le_term_key_request_reply_command(uint16 conhdl, uint8 * long_term_key)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+18, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1A, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+18);
    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    memcpy((uint8 *)p->payload+6, long_term_key, 16);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_le_long_term_key_request_negative_reply_command(uint16 conhdl)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+2, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1B, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+2);
    ((uint8 *)p->payload)[4] = conhdl & 0xFF;
    ((uint8 *)p->payload)[5] = (conhdl>> 8) & 0xFF;
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_le_read_supported_states_command(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1C, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_le_receiver_test_command(uint8 RX_Frequenc)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1D, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+1);
    ((uint8 *)p->payload)[4] = RX_Frequenc;
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

_ATTR_LWBT_CODE_
int hci_le_transmitter_test_command(uint8 TX_Frequenc, uint8 Length_Of_Test_Data, uint8 Packet_Payload)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1+3, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1E, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1+3);
    ((uint8 *)p->payload)[4] = TX_Frequenc;
    ((uint8 *)p->payload)[5] = Length_Of_Test_Data;
    ((uint8 *)p->payload)[6] = Packet_Payload;
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}


_ATTR_LWBT_CODE_
int hci_le_test_end_command(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, HCI_CMD_HDR_LEN+1, PBUF_RAM)) == NULL)
    {
        LWBT_DEBUGF(HCI_DEBUG, _DBG_INFO_, ("  Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }
    /* Assembling command packet */
    p = hci_cmd_ass(p, 0x1F, HCI_BLE_OGF, HCI_CMD_HDR_LEN+1);
    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}
#endif //BLE_SMALL_MEMROY


#endif

#endif
