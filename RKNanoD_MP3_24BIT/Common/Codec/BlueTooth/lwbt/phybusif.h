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

#ifndef __LWBT_PHYBUSIF_H__
#define __LWBT_PHYBUSIF_H__

#include "pbuf.h"
#include "lwbterr.h"
#include "btHwControl.h"

struct phybusif_cb;

/* Application program's interface: */
void phybusif_init(bt_hw_control_t * hw_control); /* Must be called first to initialize the physical bus interface */
int phybusif_reset(struct phybusif_cb *cb);
int phybusif_input(unsigned char c);

void hci_serve(void); //wp add
void cc2564_reset(void);
void cc2564_init(uint32 script_module_id);
/* Upper layer interface: */
//void phybusif_output(struct pbuf *p, uint16 len);

void _phybusif_output(struct pbuf *p, uint16 len, void (*func)(void)) ;
#define  phybusif_output(p, len)   _phybusif_output(p, len , NULL)


uint32 HciServeIsrDisable(void);
void HciServeIsrEnable(void);
//void send_go_to_sleep_ack();

//void send_wake_up_ind(void);
//void send_wake_up_ack(void);
//void hcill_go_to_sleep_ind_notify(void(*hcill_go_to_sleep_ind_notify)(void));
//void hcill_wake_up_ind_notify(void(*hcill_wake_up_ind_notify)(void));
//void hcill_wake_up_ack_notify(void(*hcill_wake_up_ack_notify)(void));

//void phybusif_output_insleep_proc(void(*output_insleep_proc)(void));
//void phybusif_get_cc2564_sleep_state(uint8(*get_cc2564_sleep_state)(void));





enum phybusif_state
{
    W4_PACKET_TYPE, W4_EVENT_HDR, W4_EVENT_PARAM, W4_ACL_HDR, W4_ACL_DATA, W4_SCO_HDR, W4_SCO_DATA, W4_WAIT_BUF
};

/* The physical bus interface control block */
__packed struct phybusif_cb
{
    enum phybusif_state state;

    struct pbuf *p;
    struct pbuf *q;
    struct pbuf *hci_event_unread;
    struct pbuf *hci_acl_unread;
    struct pbuf *hci_sco_unread;
    struct pbuf *waitsend;
	struct pbuf *h5_unread;
    struct hci_event_hdr *evhdr;
    struct hci_acl_hdr *aclhdr;
    struct hci_sco_hdr *scohdr;

    bt_hw_control_t *hw_conrtol;
    void(*hcill_go_to_sleep_ind_notify)(void);
    void(*hcill_wake_up_ind_notify)(void);
    void(*hcill_wake_up_ack_notify)(void);
    void(*output_insleep_proc)(void);//�ڷ���������˯��ʱ�Ĵ�����
    uint8(*get_cc2564_sleep_state)(void);
    unsigned int tot_recvd;
    unsigned int recvd;
    uint8  isOutputComplete;
    uint8  isGetEvent;
    uint8  isGetACL;
    uint8  unused[3];
};

#endif /* __LWBT_PHYBUSIF_H__ */
