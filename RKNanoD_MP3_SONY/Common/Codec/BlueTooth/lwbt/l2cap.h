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

#ifndef __LWBT_L2CAP_H__
#define __LWBT_L2CAP_H__

#include "lwbtopts.h"

#if L2CAP_HCI
#include "hci.h"
#else
#include "netif/lwbt/bb.h"
#include "netif/lwbt/lmp.h"
#endif /* L2CAP_HCI_UART */

struct l2cap_pcb;
struct l2cap_cfg;
struct l2cap_sig;

int l2cap_is_psm_serve_connected(struct bd_addr *bdaddr, uint16 psm);
struct l2cap_pcb *  l2cap_get_psm_pcb(struct bd_addr *bdaddr, uint16 psm);
/* Functions for interfacing with L2CAP */
void l2cap_init(void); /* Must be called first to initialize L2CAP */
void l2cap_tmr(void); /* Must be called every 1s */
void l2cap_deinit(void);

/* Higher layers interface */
struct l2cap_pcb *l2cap_new(void);
err_t l2cap_close(struct l2cap_pcb *pcb);
void l2cap_reset_all(void);
void l2cap_arg(struct l2cap_pcb *pcb, void *arg);
#define l2cap_psm(pcb) ((pcb)->psm)
err_t l2cap_connect_ind(struct l2cap_pcb *lpcb, uint8 psm,
                        err_t (* l2ca_connect_ind)(void *arg, struct l2cap_pcb *pcb, err_t err));
void l2cap_disconnect_ind(struct l2cap_pcb *pcb,
                          err_t (* l2ca_disconnect_ind)(void *arg, struct l2cap_pcb *newpcb,
                                  err_t err));
void l2cap_timeout_ind(struct l2cap_pcb *pcb,
                       err_t (* l2ca_timeout_ind)(void *arg, struct l2cap_pcb *newpcb,
                               err_t err));
void l2cap_recv(struct l2cap_pcb *pcb,
                err_t (* l2ca_recv)(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err));

err_t l2ca_connect_req(struct l2cap_pcb *pcb, struct bd_addr *bdaddr, uint16 psm, uint8 role_switch,
                       err_t (* l2ca_connect_cfm)(void *arg, struct l2cap_pcb *lpcb,
                               uint16 result, uint16 status));
err_t l2ca_config_req(struct l2cap_pcb *pcb, int flag);
err_t l2ca_disconnect_req(struct l2cap_pcb *pcb,
                          err_t (* l2ca_disconnect_cfm)(void *arg, struct l2cap_pcb *pcb));
//err_t l2ca_datawrite(struct l2cap_pcb *pcb, struct pbuf *p);

err_t _l2ca_datawrite(struct l2cap_pcb *pcb, struct pbuf *p, void (*func)(void));

#define l2ca_datawrite(pcb, p)  _l2ca_datawrite(pcb, p, NULL)

err_t l2ca_ping(struct bd_addr *bdaddr, struct l2cap_pcb *tpcb,
                err_t (* l2ca_pong)(void *arg, struct l2cap_pcb *pcb, uint8 result));

/* Only used by lower layers to pass a L2CAP segment to L2CAP */
void l2cap_input(struct pbuf *p, struct bd_addr *bdaddr);

/* Used by lower layers to signal events to L2CAP */
void lp_connect_cfm(struct bd_addr *bdaddr, uint8 encrypt_mode, err_t err);
void lp_connect_ind(struct bd_addr *bdaddr);
void lp_disconnect_ind(struct bd_addr *bdaddr);

/* Used within L2CAP code only */
err_t l2cap_signal(struct l2cap_pcb *pcb, uint8 code, uint16 ursp_id, struct bd_addr *remote_bdaddr,
                   struct pbuf *data);
err_t l2cap_rexmit_signal(struct l2cap_pcb *pcb, struct l2cap_sig *sig);
uint8 l2cap_next_sigid(void);

void l2cap_register_event_handler(struct l2cap_pcb *pcb,
                                  void (* l2ca_event_handler)(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size));



#define l2cap_can_send_packet_now(x)    hci_can_send_packet_now(x)
#define l2cap_reserve_packet_buffer(x)    hci_take_packet_buffer(x)
#define l2cap_release_packet_buffer(x)    hci_give_packet_buffer(x)

err_t l2ca_echo_req(struct l2cap_pcb *pcb);
err_t l2ca_info_req(struct l2cap_pcb *pcb, int InfoType);


/* Protocol and service multiplexor */
#define SDP_PSM         0x0001
#define RFCOMM_PSM      0x0003
#define BNEP_PSM        0x000F
#define AVCTP_PSM       0x0017
#define AVDTP_PSM       0x0019
#define AVCTP_BROWSING  0x001B

/* Packet header lengths */
#define L2CAP_HDR_LEN 4
#define L2CAP_SIGHDR_LEN 4
#define L2CAP_CFGOPTHDR_LEN 2

/* Signals sizes */
#define L2CAP_CONN_REQ_SIZE 4
#define L2CAP_CONN_RSP_SIZE 8
#define L2CAP_CFG_RSP_SIZE 6
#define L2CAP_DISCONN_RSP_SIZE 4

#define L2CAP_CFG_REQ_SIZE 4

#define L2CAP_DISCONN_REQ_SIZE 4
#define L2CAP_CMD_REJ_SIZE 2

/* Signal codes */
#define L2CAP_CMD_REJ 0x01
#define L2CAP_CONN_REQ 0x02
#define L2CAP_CONN_RSP 0x03
#define L2CAP_CFG_REQ 0x04
#define L2CAP_CFG_RSP 0x05
#define L2CAP_DISCONN_REQ 0x06
#define L2CAP_DISCONN_RSP 0x07
#define L2CAP_ECHO_REQ 0x08
#define L2CAP_ECHO_RSP 0x09
#define L2CAP_INFO_REQ 0x0A
#define L2CAP_INFO_RSP 0x0B
#define L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST   0x12
#define L2CAP_CONNECTION_PARAMETER_UPDATE_RESPONSE  0x13
/* Permanent channel identifiers */
#define L2CAP_NULL_CID 0x0000
#define L2CAP_SIG_CID 0x0001
#define L2CAP_CONNLESS_CID 0x0002
#define L2CAP_CID_ATTRIBUTE_PROTOCOL        0x0004
#define L2CAP_CID_SIGNALING_LE              0x0005
#define L2CAP_CID_SECURITY_MANAGER_PROTOCOL 0x0006

/* Channel identifiers values */
#define L2CAP_MIN_CID 0x0050
#define L2CAP_MAX_CID 0xFFFF

/* Configuration types */
#define L2CAP_CFG_MTU 0x01
#define L2CAP_FLUSHTO 0x02
#define L2CAP_QOS 0x03
#define L2CAP_RET_FLOW 0x04

/* Configuration types length */
#define L2CAP_MTU_LEN 2
#define L2CAP_FLUSHTO_LEN 2
#define L2CAP_QOS_LEN 22

/* Configuration response types */
#define L2CAP_CFG_SUCCESS 0x0000
#define L2CAP_CFG_UNACCEPT 0x0001
#define L2CAP_CFG_REJ 0x0002
#define L2CAP_CFG_UNKNOWN 0x0003
#define L2CAP_CFG_TIMEOUT 0xEEEE

/* QoS types */
#define L2CAP_QOS_NO_TRAFFIC 0x00
#define L2CAP_QOS_BEST_EFFORT 0x01
#define L2CAP_QOS_GUARANTEED 0x02

/* Command reject reasons */
#define L2CAP_CMD_NOT_UNDERSTOOD 0x0000
#define L2CAP_MTU_EXCEEDED 0x0001
#define L2CAP_INVALID_CID 0x0002

/* Connection response results */
#define L2CAP_CONN_SUCCESS 0x0000
#define L2CAP_CONN_PND 0x0001
#define L2CAP_CONN_REF_PSM 0x0002
#define L2CAP_CONN_REF_SEC 0x0003
#define L2CAP_CONN_REF_RES 0x0004
#define L2CAP_CONN_CFG_TO 0x0005 /* Implementation specific result */

/* Echo response results */
#define L2CAP_ECHO_RCVD 0x00
#define L2CAP_ECHO_TO 0x01

/* L2CAP segmentation */
#define L2CAP_ACL_START 0x02
#define L2CAP_ACL_CONT 0x01

/* L2CAP config default parameters */
#define L2CAP_CFG_DEFAULT_INMTU 672 /* Two Baseband DH5 packets (2*341=682) minus the Baseband ACL
                       headers (2*2=4) and L2CAP header (6) */
#define L2CAP_CFG_DEFAULT_OUTFLUSHTO 0xFFFF

/* L2CAP configuration parameter masks */
#define L2CAP_CFG_IR 0x01
#define L2CAP_CFG_IN_SUCCESS 0x02
#define L2CAP_CFG_OUT_SUCCESS 0x04
#define L2CAP_CFG_OUT_REQ 0x08

#define L2CAP_USER_FG_NULL            0x0
#define L2CAP_USER_CFG_MTU            (1<<0)
#define L2CAP_USER_CFG_FLUSH_TIMEOUT  (1<<1)
#define L2CAP_USER_CFG_QOS            (1<<2)
#define L2CAP_USER_CFG_FLOW           (1<<3)

/* L2CAP Information requests infoType*/
#define L2CAP_INFOTYPE_CONN_MTU 0x0001
#define L2CAP_INFOTYPE_Ext_FEATURE_MASK 0x0002

/* L2CAP EXT FEATURE*/
#define L2CAP_EXT_FEATURE_FLOW_CONTROL (1<<0)
#define L2CAP_EXT_FEATURE_RE_MODE      (1<<1)
#define L2CAP_EXT_FEATURE_QOS          (1<<2)
#define L2CAP_EXT_FEATURE_EN_RE_MODE   (1<<3)
#define L2CAP_EXT_FEATURE_STREAM_MODE  (1<<4)
#define L2CAP_EXT_FEATURE_FCS          (1<<5)
#define L2CAP_EXT_FEATURE_EXT_FLOW     (1<<6)
#define L2CAP_EXT_FEATURE_FIXED        (1<<7)
#define L2CAP_EXT_FEATURE_EXT_WINDOW_SIZE (1<<8)
#define L2CAP_EXT_FEATURE_EXT_UNICAST     (1<<9)








__packed struct l2cap_hdr
{
    uint16 len;
    uint16 cid;
};
__packed struct l2cap_sig_hdr
{
    uint8 code;
    uint8 id;
    uint16 len;
};

__packed struct l2cap_cfgopt_hdr
{
    uint8 type;
    uint8 len;
};

enum l2cap_state
{
    L2CAP_CLOSED, L2CAP_LISTEN, W4_L2CAP_CONNECT_RSP, W4_L2CA_CONNECT_RSP, L2CAP_CONFIG,
    L2CAP_OPEN, W4_L2CAP_DISCONNECT_RSP, W4_L2CA_DISCONNECT_RSP
};

__packed struct l2cap_acl_link
{
    struct l2cap_acl_link *next;
    struct bd_addr bdaddr;
};

/* This structure is used to represent L2CAP signals. */
__packed struct l2cap_sig
{
    struct l2cap_sig *next;    /* for the linked list, used when putting signals
                on a queue */
    struct pbuf *p;          /* buffer containing data + L2CAP header */
    uint16 sigid; /* Identification */
    uint16 ertx; /* extended response timeout expired */
    uint8 rtx; /* response timeout expired */
    uint8 nrtx; /* number of retransmissions */
};

__packed struct l2cap_cfg
{
    uint16 inmtu; /* Maximum transmission unit this channel can accept */
    uint16 outmtu; /* Maximum transmission unit that can be sent on this channel */
    uint16 influshto; /* In flush timeout */
    uint16 outflushto; /* Out flush timeout */

    struct pbuf *opt; /* Any received non-hint unknown option(s) or option(s) with unacceptable parameters
               will be temporarily stored here */

    uint8 cfgto; /* Configuration timeout */
    uint8 l2capcfg; /* Bit 1 indicates if we are the initiator of this connection
          * Bit 2 indicates if a successful configuration response has been received
          * Bit 3 indicates if a successful configuration response has been sent
          * Bit 4 indicates if an initial configuration request has been sent
          */
};

__packed struct l2cap_seg
{
    struct l2cap_seg *next; /* For the linked list */

    struct bd_addr bdaddr;

    struct pbuf *p;          /* Buffer containing data + L2CAP header */
    uint16 len;               /* The L2CAP length of this segment */
    struct l2cap_hdr *l2caphdr;  /* The L2CAP header */
    struct l2cap_pcb *pcb; /* The L2CAP Protocol Control Block */
};

__packed struct l2cap_pcb
{
    struct l2cap_pcb *next; /* For the linked list */

    enum l2cap_state state; /* L2CAP state */

    void *callback_arg;

    uint16 scid; /* Source CID */
    uint16 dcid; /* Destination CID */

    uint16 psm; /* Protocol/Service Multiplexer */

    uint16 ursp_id; /* Signal id to respond to */
    uint8 encrypt; /* encryption mode */

    struct l2cap_sig *unrsp_sigs;  /* List of sent but unresponded signals */

    struct bd_addr remote_bdaddr;

    struct l2cap_cfg cfg; /* Configuration parameters */

    /* Upper layer to L2CAP confirmation functions */

    /* Function to be called when a connection has been set up */
    err_t (* l2ca_connect_cfm)(void *arg, struct l2cap_pcb *pcb, uint16 result, uint16 status);
    /* Function to be called when a connection has been closed */
    err_t (* l2ca_disconnect_cfm)(void *arg, struct l2cap_pcb *pcb);
    /* Function to be called when a echo reply has been received */
    err_t (* l2ca_pong)(void *arg, struct l2cap_pcb *pcb, uint8 result);

    /* L2CAP to upper layer indication functions */

    /* Function to be called when a connection indication event occurs */
    err_t (* l2ca_connect_ind)(void *arg, struct l2cap_pcb *pcb, err_t err);
    /* Function to be called when a disconnection indication event occurs */
    err_t (* l2ca_disconnect_ind)(void *arg, struct l2cap_pcb *pcb, err_t err);
    /* Function to be called when a timeout indication event occurs */
    err_t (* l2ca_timeout_ind)(void *arg, struct l2cap_pcb *newpcb, err_t err);
    /* Function to be called when a L2CAP connection receives data */
    err_t (* l2ca_recv)(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err);

    void (* l2ca_event_handler)(uint8 packet_type, uint16 channel, uint8 *packet, uint16 size);
};

__packed struct l2cap_pcb_listen
{
    struct l2cap_pcb_listen *next; /* for the linked list */

    enum l2cap_state state; /* L2CAP state */

    void *callback_arg;

    uint16 psm; /* Protocol/Service Multiplexer */

    /* Function to call when a connection request has been received
       from a remote device. */
    err_t (* l2ca_connect_ind)(void *arg, struct l2cap_pcb *pcb, err_t err);
};

/* Internal functions and global variables */
#define L2CA_ACTION_CONN_CFM(pcb,result,status,ret) if((pcb)->l2ca_connect_cfm != NULL) (ret = (pcb)->l2ca_connect_cfm((pcb)->callback_arg,(pcb),(result),(status)))
//#define L2CA_ACTION_DISCONN_CFM(pcb,ret) if((pcb)->l2ca_disconnect_cfm != NULL) (ret = (pcb)->l2ca_disconnect_cfm((pcb)->callback_arg,(pcb)))

#define L2CA_ACTION_DISCONN_CFM(pcb,ret) \
                                       if((pcb)->l2ca_disconnect_cfm != NULL)   { \
                                       (ret = (pcb)->l2ca_disconnect_cfm((pcb)->callback_arg,(pcb)));\
                                       }else { \
                                       l2cap_close(pcb); \
                                       }

#define L2CA_ACTION_PING_CFM(pcb,result,ret) if((pcb)->l2ca_pong != NULL) (ret = (pcb)->l2ca_pong((pcb)->callback_arg,(pcb),(result)))

#define L2CA_ACTION_CONN_IND(pcb,err,ret) if((pcb)->l2ca_connect_ind != NULL) (ret = (pcb)->l2ca_connect_ind((pcb)->callback_arg,(pcb),(err)))
#define L2CA_ACTION_DISCONN_IND(pcb,err,ret) \
                                if((pcb)->l2ca_disconnect_ind != NULL) { \
                                LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_disconnect_ind called\n")); \
                                (ret = (pcb)->l2ca_disconnect_ind((pcb)->callback_arg,(pcb),(err))); \
                                } else { \
                                l2cap_close(pcb); \
                                }
#define L2CA_ACTION_TO_IND(pcb,err,ret) if((pcb)->l2ca_timeout_ind != NULL) (ret = (pcb)->l2ca_timeout_ind((pcb)->callback_arg,(pcb),(err)))
#define L2CA_ACTION_RECV(pcb,p,err,ret) \
                         if((pcb)->l2ca_recv != NULL) { \
             (ret = (pcb)->l2ca_recv((pcb)->callback_arg,(pcb),(p),(err))); \
             } else { \
             pbuf_free(p); \
                         }

#define L2CAP_OPTH_TYPE(hdr) (((hdr)->type) & 0x7f)
#define L2CAP_OPTH_TOA(hdr) (((hdr)->type) >> 7)

/* The L2CAP PCB lists. */
extern struct l2cap_pcb_listen *l2cap_listen_pcbs; /* List of all L2CAP PCBs in CLOSED state
                        but awaing an incoming conn req. */
extern struct l2cap_pcb *l2cap_active_pcbs; /* List of all L2CAP PCBs that has
                           established or is about to establish
                           an ACL link */

extern struct l2cap_pcb *l2cap_tmp_pcb;      /* Only used for temporary storage. */

/* Axoims about the above list:
   1) A PCB is only in one of the lists.
   2) All PCBs in the l2cap_active_pcbs list has established or is about to
   establish an ACL link.
*/

/* Define two macros, L2CAP_REG and L2CAP_RMV that registers a L2CAP PCB
   with a PCB list or removes a PCB from a list, respectively. */
#ifdef LWBT_DEBUG
#define L2CAP_REG(pcbs, npcb) do { \
                            for(l2cap_tmp_pcb = *pcbs; \
                  l2cap_tmp_pcb != NULL; \
                l2cap_tmp_pcb = l2cap_tmp_pcb->next) { \
                                LWIP_ASSERT("L2CAP_REG: already registered\n", l2cap_tmp_pcb != npcb); \
                            } \
                            npcb->next = *pcbs; \
                            LWIP_ASSERT("L2CAP_REG: npcb->next != npcb", npcb->next != npcb); \
                            *pcbs = npcb; \
                            } while(0)
#define L2CAP_RMV(pcbs, npcb) do { \
                            LWIP_ASSERT("L2CAP_RMV: pcbs != NULL", *pcbs != NULL); \
                            LWIP_DEBUGF(L2CAP_DEBUG, ("L2CAP_RMV: removing %p from %p\n", npcb, *pcbs)); \
                            if(*pcbs == npcb) { \
                               *pcbs = (*pcbs)->next; \
                            } else for(l2cap_tmp_pcb = *pcbs; l2cap_tmp_pcb != NULL; l2cap_tmp_pcb = l2cap_tmp_pcb->next) { \
                               if(l2cap_tmp_pcb->next != NULL && l2cap_tmp_pcb->next == npcb) { \
                                  l2cap_tmp_pcb->next = npcb->next; \
                                  break; \
                               } \
                            } \
                            npcb->next = NULL; \
                            LWIP_DEBUGF(L2CAP_DEBUG, ("L2CAP_RMV: removed %p from %p\n", npcb, *pcbs)); \
                            } while(0)

#else /* LWBT_DEBUG */
#define L2CAP_REG(pcbs, npcb) do { \
                            npcb->next = *pcbs; \
                            *pcbs = npcb; \
                            } while(0)
#define L2CAP_RMV(pcbs, npcb) do { \
                            if(*pcbs == npcb) { \
                               *pcbs = (*pcbs)->next; \
                            } else for(l2cap_tmp_pcb = *pcbs; l2cap_tmp_pcb != NULL; l2cap_tmp_pcb = l2cap_tmp_pcb->next) { \
                               if(l2cap_tmp_pcb->next != NULL && l2cap_tmp_pcb->next == npcb) { \
                                  l2cap_tmp_pcb->next = npcb->next; \
                                  break; \
                               } \
                            } \
                            npcb->next = NULL; \
                            } while(0)
#endif /* LWBT_DEBUG */

/* The L2CAP SIG list macros */
extern struct l2cap_sig *l2cap_tmp_sig;      /* Only used for temporary storage. */

#define L2CAP_SIG_REG(ursp_sigs, nsig) do { \
                            nsig->next = *ursp_sigs; \
                            *ursp_sigs = nsig; \
                            } while(0)
#define L2CAP_SIG_RMV(ursp_sigs, nsig) do { \
                            if(*ursp_sigs == nsig) { \
                               *ursp_sigs = (*ursp_sigs)->next; \
                            } else for(l2cap_tmp_sig = *ursp_sigs; l2cap_tmp_sig != NULL; l2cap_tmp_sig = l2cap_tmp_sig->next) { \
                               if(l2cap_tmp_sig->next != NULL && l2cap_tmp_sig->next == nsig) { \
                                  l2cap_tmp_sig->next = nsig->next; \
                                  break; \
                               } \
                            } \
                            nsig->next = NULL; \
                            } while(0)

/* The L2CAP incoming segments list macros */
extern struct l2cap_seg *l2cap_tmp_inseg;      /* Only used for temporary storage. */

#define L2CAP_SEG_REG(segs, nseg) do { \
                            nseg->next = *segs; \
                            *segs = nseg; \
                            } while(0)
#define L2CAP_SEG_RMV(segs, nseg) do { \
                            if(*segs == nseg) { \
                               *segs = (*segs)->next; \
                            } else for(l2cap_tmp_inseg = *segs; l2cap_tmp_inseg != NULL; l2cap_tmp_inseg = l2cap_tmp_inseg->next) { \
                               if(l2cap_tmp_inseg->next != NULL && l2cap_tmp_inseg->next == nseg) { \
                                  l2cap_tmp_inseg->next = nseg->next; \
                                  break; \
                               } \
                            } \
                            nseg->next = NULL; \
                            } while(0)
#endif /* __LWBT_L2CAP_H__ */


