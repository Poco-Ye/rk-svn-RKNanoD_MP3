/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
#include "lwbt.h"
#include "SysInclude.h"
#include "lwbt_memp.h"
#include "hci.h"
#include "l2cap.h"
#include "sdp.h"
//#include "rfcomm.h"
//#include "OBEX.h"
//#include "opp.h"
//#include "netif/lwbt/ppp.h"
#include "lwbtopts.h"

//#include "Hsp_hfp.h"
#ifdef _A2DP_SINK_
#include "Avdtp.h"
#include "avctp.h"
#include "avrcp.h"
#endif
#ifdef _A2DP_SOUCRE_
#include "./a2dp_source/avdtp_source.h"
#include "./a2dp_source/avctp_source.h"
#include "./a2dp_source/avrcp_source.h"

#endif
//#include "lwip/mem.h"
__packed
struct memp
{
    struct memp *next;
};

#ifdef _BLUETOOTH_

_ATTR_LWBT_DATA_ static struct memp *memp_tab[MEMP_LWBT_MAX];

_ATTR_LWBT_DATA_ static uint16 memp_sizes[MEMP_LWBT_MAX] =
{
    sizeof(struct hci_pcb),
    sizeof(struct hci_link),
    sizeof(struct hci_inq_res),
    sizeof(struct l2cap_pcb),
    sizeof(struct l2cap_pcb_listen),
    sizeof(struct l2cap_sig),
    sizeof(struct l2cap_seg),
    sizeof(struct sdp_pcb),
    sizeof(struct sdp_record),
    sizeof(struct a2dp_pcb),

    sizeof(struct avctp_pcb),
    sizeof(struct avrcp_pcb),

//    sizeof(struct ppp_pcb),
//    sizeof(struct ppp_req)
};

_ATTR_LWBT_DATA_ static uint16 memp_num[MEMP_LWBT_MAX] =
{
    MEMP_NUM_HCI_PCB,
    MEMP_NUM_HCI_LINK,
    MEMP_NUM_HCI_INQ,
    MEMP_NUM_L2CAP_PCB,
    MEMP_NUM_L2CAP_PCB_LISTEN,
    MEMP_NUM_L2CAP_SIG,
    MEMP_NUM_L2CAP_SEG,
    MEMP_NUM_SDP_PCB,
    MEMP_NUM_SDP_RECORD,
    MEMP_NUM_A2DP_PCB,

    MEMP_NUM_AVCTP_PCB,
    MEMP_NUM_AVRCP_PCB,
//    MEMP_NUM_PPP_PCB,
//    MEMP_NUM_PPP_REQ
};

_ATTR_LWBT_UARTIF_DATA_ static uint8 memp_memory[(MEMP_NUM_HCI_PCB *
        MEM_ALIGN_SIZE(sizeof(struct hci_pcb) +
                       sizeof(struct memp)) +
        MEMP_NUM_HCI_LINK *
        MEM_ALIGN_SIZE(sizeof(struct hci_link) +
                       sizeof(struct memp)) +
        MEMP_NUM_HCI_INQ *
        MEM_ALIGN_SIZE(sizeof(struct hci_inq_res) +
                       sizeof(struct memp)) +
        MEMP_NUM_L2CAP_PCB *
        MEM_ALIGN_SIZE(sizeof(struct l2cap_pcb) +
                       sizeof(struct memp)) +
        MEMP_NUM_L2CAP_PCB_LISTEN *
        MEM_ALIGN_SIZE(sizeof(struct l2cap_pcb_listen) +
                       sizeof(struct memp)) +
        MEMP_NUM_L2CAP_SIG *
        MEM_ALIGN_SIZE(sizeof(struct l2cap_sig) +
                       sizeof(struct memp)) +
        MEMP_NUM_L2CAP_SEG *
        MEM_ALIGN_SIZE(sizeof(struct l2cap_seg) +
                       sizeof(struct memp)) +
        MEMP_NUM_SDP_PCB *
        MEM_ALIGN_SIZE(sizeof(struct sdp_pcb) +
                       sizeof(struct memp)) +
        MEMP_NUM_SDP_RECORD *
        MEM_ALIGN_SIZE(sizeof(struct sdp_record) +
                       sizeof(struct memp)) +
        MEMP_NUM_A2DP_PCB *
        MEM_ALIGN_SIZE(sizeof(struct a2dp_pcb) +
                       sizeof(struct memp))
        +
        MEMP_NUM_AVCTP_PCB *
        MEM_ALIGN_SIZE(sizeof(struct avctp_pcb) +
                       sizeof(struct memp))
        +
        MEMP_NUM_AVRCP_PCB *
        MEM_ALIGN_SIZE(sizeof(struct avrcp_pcb) +
                       sizeof(struct memp))
//          +
//          MEMP_NUM_PPP_PCB *
//           MEM_ALIGN_SIZE(sizeof(struct ppp_pcb) +
//                  sizeof(struct memp)) +
//          MEMP_NUM_PPP_REQ *
//           MEM_ALIGN_SIZE(sizeof(struct ppp_req) +
//                  sizeof(struct memp))
                                                 )];

/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void lwbt_memp_init(void)
{
    struct memp *m, *memp;
    uint16 i, j;
    uint16 size;

    memp = (struct memp *)&memp_memory[0];
    for(i = 0; i < MEMP_LWBT_MAX; ++i)
    {
        size = MEM_ALIGN_SIZE(memp_sizes[i] + sizeof(struct memp));
        if(memp_num[i] > 0)
        {
            memp_tab[i] = memp;
            m = memp;

            for(j = 0; j < memp_num[i]; ++j)
            {
                m->next = (struct memp *)MEM_ALIGN((uint8 *)m + size);
                memp = m;
                m = m->next;
            }
            memp->next = NULL;
            memp = m;
        }
        else
        {
            memp_tab[i] = NULL;
        }

    }
}
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
void *lwbt_memp_malloc(lwbt_memp_t type)
{
    struct memp *memp;
    void *mem;

    memp = memp_tab[type];

    if(memp != NULL)
    {
        memp_tab[type] = memp->next;
        memp->next = NULL;

        mem = MEM_ALIGN((uint8 *)memp + sizeof(struct memp));
        /* initialize memp memory with zeroes */
        memset(mem, 0, memp_sizes[type]);
        return mem;
    }
    else
    {
        return NULL;
    }
}
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
void lwbt_memp_free(lwbt_memp_t type, void *mem)
{
    struct memp *memp;

    if(mem == NULL)
    {
        return;
    }
    memp = (struct memp *)((uint8 *)mem - sizeof(struct memp));

    memp->next = memp_tab[type];
    memp_tab[type] = memp;

    return;
}

#endif
/*-----------------------------------------------------------------------------------*/
