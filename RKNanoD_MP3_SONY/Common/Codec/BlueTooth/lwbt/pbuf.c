/**
 * @file
 * Packet buffer management
 *
 * Packets are built from the pbuf data structure. It supports dynamic
 * memory allocation for packet contents or can reference externally
 * managed packet contents both in RAM and ROM. Quick allocation for
 * incoming packets is provided through pools with fixed sized pbufs.
 *
 * A packet may span over multiple pbufs, chained as a singly linked
 * list. This is called a "pbuf chain".
 *
 * Multiple packets may be queued, also using this singly linked list.
 * This is called a "packet queue".
 *
 * So, a packet queue consists of one or more pbuf chains, each of
 * which consist of one or more pbufs. CURRENTLY, PACKET QUEUES ARE
 * NOT SUPPORTED!!! Use helper structs to queue multiple packets.
 *
 * The differences between a pbuf chain and a packet queue are very
 * precise but subtle.
 *
 * The last pbuf of a packet has a ->tot_len field that equals the
 * ->len field. It can be found by traversing the list. If the last
 * pbuf of a packet has a ->next field other than NULL, more packets
 * are on the queue.
 *
 * Therefore, looping through a pbuf of a single packet, has an
 * loop end condition (tot_len == p->len), NOT (next == NULL).
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "lwbt.h"
#include "sysinclude.h"
#include "lwbtopts.h"
//#include "def.h"
#include "pbuf.h"

#include <string.h>

#ifdef _BLUETOOTH_
_ATTR_LWBT_DATA_ PBUF * pEmpty;
//PBUF __align(4) stPuf[BUF_MAX_CNT];

#ifdef _A2DP_SINK_
#define  BUF_MAX_CNT   12
#else
#define  BUF_MAX_CNT   25

#endif
_ATTR_LWBT_DATA_ PBUF __align(4) stPuf[BUF_MAX_CNT];

_ATTR_LWBT_DATA_ uint16  bufFreeCnt = 0;
/**
 * Attempt to reclaim some memory from queued out-of-sequence TCP segments
 * if we run out of pool pbufs. It's better to give priority to new packets
 * if we're running out.
 *
 * @return the allocated pbuf.
 */
_ATTR_LWBT_INIT_CODE_
void pbuf_init(void)
{
    uint8 i;

    stPuf[0].next = NULL;

    for (i = 1; i < BUF_MAX_CNT; i++)
    {
        stPuf[i].next = &stPuf[i - 1];
    }
    pEmpty = &stPuf[i - 1];

    bufFreeCnt = BUF_MAX_CNT;
}


/**
 * Allocates a pbuf of the given type (possibly a chain for PBUF_POOL type).
 *
 * The actual memory allocated for the pbuf is determined by the
 * layer at which the pbuf is allocated and the requested size
 * (from the size parameter).
 *
 * @param layer flag to define header size
 * @param length size of the pbuf's payload
 * @param type this parameter decides how and where the pbuf
 * should be allocated as follows:
 *
 * - PBUF_RAM: buffer memory for pbuf is allocated as one large
 *             chunk. This includes protocol headers as well.
 * - PBUF_ROM: no buffer memory is allocated for the pbuf, even for
 *             protocol headers. Additional headers must be prepended
 *             by allocating another pbuf and chain in to the front of
 *             the ROM pbuf. It is assumed that the memory used is really
 *             similar to ROM in that it is immutable and will not be
 *             changed. Memory which is dynamic should generally not
 *             be attached to PBUF_ROM pbufs. Use PBUF_REF instead.
 * - PBUF_REF: no buffer memory is allocated for the pbuf, even for
 *             protocol headers. It is assumed that the pbuf is only
 *             being used in a single thread. If the pbuf gets queued,
 *             then pbuf_take should be called to copy the buffer.
 * - PBUF_POOL: the pbuf is allocated as a pbuf chain, with pbufs from
 *              the pbuf pool that is allocated during pbuf_init().
 *
 * @return the allocated pbuf. If multiple pbufs where allocated, this
 * is the first pbuf of a pbuf chain.
 */
_ATTR_LWBT_UARTIF_CODE_
PBUF *pbuf_alloc(pbuf_layer layer, uint16 length, pbuf_type type)
{
    PBUF *p, *q, *r;
    uint16 offset;
    int32 rem_len; /* remaining length */
    //IntDisable(INT_ID17_UART);

	//printf("Icnt=%d", bufFreeCnt);
    IntMasterDisable();

    if (length > PHY_FRAME_MAX_SIZE)
    {
        //IntEnable(INT_ID17_UART);

        IntMasterEnable();
        return NULL;
    }
    if (pEmpty == NULL)
    {
        //IntEnable(INT_ID17_UART);
        IntMasterEnable();
        //printf("Icnt=%d", bufFreeCnt);
        return NULL;
    }


    /* determine header offset */
    offset = 0;
    switch (layer)
    {
        case PBUF_TRANSPORT:
            /* add room for transport (often TCP) layer header */
            offset += PBUF_TRANSPORT_HLEN;

        case PBUF_IP:
            /* add room for IP layer header */
            offset += PBUF_IP_HLEN;

        case PBUF_ETHNET:
            /* add room for link layer header */
            offset += PBUF_ETHNET_HLEN;

        case PBUF_PHY:
        //    offset += PBUF_PHY_HLEN ;
        //    offset += 8;//reservd some space.

        case PBUF_RAW:
            offset += 64;
            break;

        default:
            //IntEnable(INT_ID17_UART);

            IntMasterEnable();
            return NULL;
    }

    switch (type)
    {
        case PBUF_POOL:
        case PBUF_RAM:
            p = pEmpty;

            if (p == NULL)
                return NULL;
            pEmpty = p->next;


            //if((length != PHY_FRAME_MAX_SIZE) && (length != 0))
            //{
            //    memset((void*)p->buf, 0, length);
            //}

            p->next = NULL;
            p->len = length;
            p->tot_len = length;
			p->buflen = 0;
            /* make the payload pointer point 'offset' bytes into pbuf data memory */
            p->payload = (void *)(p->buf + offset);
            break;

        default:
            //IntEnable(INT_ID17_UART);
            IntMasterEnable();
            return NULL;
    }

    p->ref = 1;
    p->callback = NULL;
    //IntEnable(INT_ID17_UART);
    bufFreeCnt--;
    IntMasterEnable();
    return p;
}


/**
 * Adjusts the payload pointer to hide or reveal headers in the payload.
 *
 * Adjusts the ->payload pointer so that space for a header
 * (dis)appears in the pbuf payload.
 *
 * The ->payload, ->tot_len and ->len fields are adjusted.
 *
 * @param p pbuf to change the header size.
 * @param header_size_increment Number of bytes to increment header size which
 * increases the size of the pbuf. New space is on the front.
 * (Using a negative value decreases the header size.)
 * If hdr_size_inc is 0, this function does nothing and returns succesful.
 *
 * PBUF_ROM and PBUF_REF type buffers cannot have their sizes increased, so
 * the call will fail. A check is made that the increase in header size does
 * not move the payload pointer in front of the start of the buffer.
 * @return non-zero on failure, zero on success.
 *
 */
_ATTR_LWBT_UARTIF_CODE_
uint8 pbuf_header(PBUF *p, int16 header_size_increment)
{
    if ((header_size_increment == 0) || (p == NULL))
        return 0;

    p->payload = (uint8 *)p->payload - header_size_increment;
    p->len += header_size_increment;
    p->tot_len += header_size_increment;
    return 1;

}




/**
 * Dereference a pbuf chain or queue and deallocate any no-longer-used
 * pbufs at the head of this chain or queue.
 *
 * Decrements the pbuf reference count. If it reaches zero, the pbuf is
 * deallocated.
 *
 * For a pbuf chain, this is repeated for each pbuf in the chain,
 * up to the first pbuf which has a non-zero reference count after
 * decrementing. So, when all reference counts are one, the whole
 * chain is free'd.
 *
 * @param p The pbuf (chain) to be dereferenced.
 *
 * @return the number of pbufs that were de-allocated
 * from the head of the chain.
 *
 * @note MUST NOT be called on a packet queue (Not verified to work yet).
 * @note the reference counter of a pbuf equals the number of pointers
 * that refer to the pbuf (or into the pbuf).
 *
 * @internal examples:
 *
 * Assuming existing chains a->b->c with the following reference
 * counts, calling pbuf_free(a) results in:
 *
 * 1->2->3 becomes ...1->3
 * 3->3->3 becomes 2->3->3
 * 1->1->2 becomes ......1
 * 2->1->1 becomes 1->1->1
 * 1->1->1 becomes .......
 *
 */
_ATTR_LWBT_UARTIF_CODE_
uint8 pbuf_free(PBUF *p)
{
    PBUF * q;
    IntMasterDisable();
    if (p == NULL)
    {
        IntMasterEnable();
        return 0;
    }

    while(p)
    {
        q = p;
        p = p->next;
        q->ref--;
        if(q->ref == 0)
        {
            q->next = pEmpty;
            pEmpty = q;
            bufFreeCnt++;
        }
        else
        {
            IntMasterEnable();
            return 1;
        }

    }

    IntMasterEnable();
    //printf("ocnt=%d", bufFreeCnt);
    return 1;
}



/**
 * Concatenate two pbufs (each may be a pbuf chain) and take over
 * the caller's reference of the tail pbuf.
 *
 * @note The caller MAY NOT reference the tail pbuf afterwards.
 * Use pbuf_chain() for that purpose.
 *
 * @see pbuf_chain()
 */
_ATTR_LWBT_UARTIF_CODE_
void pbuf_cat(PBUF *h, PBUF *t)
{
    struct pbuf *p;
    /* proceed to last pbuf of chain */
    for (p = h; p->next != NULL; p = p->next)
    {
        /* add total length of second chain to all totals of first chain */
        p->tot_len += t->tot_len;
    }
    /* { p is last pbuf of first h chain, p->next == NULL } */
    /* add total length of second chain to last pbuf total of first chain */
    p->tot_len += t->tot_len;
    /* chain last pbuf of head (p) with first of tail (t) */
    p->next = t;
    /* p->next now references t, but the caller will drop its reference to t,
     * so netto there is no change to the reference count of t.
     */
}


/**
 * Chain two pbufs (or pbuf chains) together.
 *
 * The caller MUST call pbuf_free(t) once it has stopped
 * using it. Use pbuf_cat() instead if you no longer use t.
 *
 * @param h head pbuf (chain)
 * @param t tail pbuf (chain)
 * @note The pbufs MUST belong to the same packet.
 * @note MAY NOT be called on a packet queue.
 *
 * The ->tot_len fields of all pbufs of the head chain are adjusted.
 * The ->next field of the last pbuf of the head chain is adjusted.
 * The ->ref field of the first pbuf of the tail chain is adjusted.
 *
 */
_ATTR_LWBT_UARTIF_CODE_
void pbuf_chain(struct pbuf *h, struct pbuf *t)
{
    pbuf_cat(h, t);
    /* t is now referenced by h */
    if(t)
    {
        t->ref++;
    }
}

_ATTR_LWBT_UARTIF_CODE_
void pbuf_ref(struct pbuf *h)
{
    /* t is now referenced by h */
    if(h)
    {
        IntMasterDisable();
        h->ref++;
        IntMasterEnable();
    }
}



/**
 * Dechains the first pbuf from its succeeding pbufs in the chain.
 *
 * Makes p->tot_len field equal to p->len.
 * @param p pbuf to dechain
 * @return remainder of the pbuf chain, or NULL if it was de-allocated.
 * @note May not be called on a packet queue.
 */
_ATTR_LWBT_UARTIF_CODE_
struct pbuf *pbuf_dechain(struct pbuf *p)
{
    struct pbuf *q;
    uint8 tail_gone = 1;
    /* tail */
    q = p->next;
    /* pbuf has successor in chain? */
    if (q != NULL)
    {
        /* enforce invariant if assertion is disabled */
        q->tot_len = p->tot_len - p->len;
        /* decouple pbuf from remainder */
        p->next = NULL;
        /* total length of pbuf p is its own length only */
        p->tot_len = p->len;
        /* q is no longer referenced by p, free it */

        tail_gone = pbuf_free(q);
        if (tail_gone > 0)
        {

        }
        /* return remaining tail or NULL if deallocated */
    }
    /* assert tot_len invariant: (p->tot_len == p->len + (p->next? p->next->tot_len: 0) */

    return ((tail_gone > 0) ? NULL : q);

}
/**
 * Shrink a pbuf chain to a desired length.
 *
 * @param p pbuf to shrink.
 * @param new_len desired new length of pbuf chain
 *
 * Depending on the desired length, the first few pbufs in a chain might
 * be skipped and left unchanged. The new last pbuf in the chain will be
 * resized, and any remaining pbufs will be freed.
 *
 * @note If the pbuf is ROM/REF, only the ->tot_len and ->len fields are adjusted.
 * @note May not be called on a packet queue.
 *
 * @note Despite its name, pbuf_realloc cannot grow the size of a pbuf (chain).
 */
_ATTR_LWBT_UARTIF_CODE_
void pbuf_realloc(struct pbuf *p, uint16 new_len)
{
    struct pbuf *q;
    uint16 rem_len; /* remaining length */
    int32 grow;

    /* desired length larger than current length? */
    if (new_len >= p->tot_len)
    {
        /* enlarging not yet supported */
        return;
    }

    /* the pbuf chain grows by (new_len - p->tot_len) bytes
     * (which may be negative in case of shrinking) */
    grow = new_len - p->tot_len;

    /* first, step over any pbufs that should remain in the chain */
    rem_len = new_len;
    q = p;
    /* should this pbuf be kept? */
    while (rem_len > q->len)
    {
        /* decrease remaining length by pbuf length */
        rem_len -= q->len;
        /* decrease total length indicator */

        q->tot_len += (uint16)grow;
        /* proceed to next pbuf in chain */
        q = q->next;

    }
    /* we have now reached the new last pbuf (in q) */
    /* rem_len == desired length for pbuf q */

    /* adjust length fields for new last pbuf */
    q->len = rem_len;
    q->tot_len = q->len;

    /* any remaining pbufs in chain? */
    if (q->next != NULL)
    {
        /* free remaining pbufs in chain */
        pbuf_free(q->next);
    }
    /* q is last packet in chain */
    q->next = NULL;
}
_ATTR_LWBT_UARTIF_CODE_
void pbuf_copy(struct pbuf *p_to, struct pbuf *p_from)
{
    uint16 offset_to=0, offset_from=0, len;
    struct pbuf *p;
    /* proceed to last pbuf of chain */
    for (p = p_from; p->next != NULL; p = p->next)
    {
        /* add total length of second chain to all totals of first chain */

        memcpy((uint8*)p_to->payload+p_to->len, p->payload, p->len);
        p_to->tot_len += p->len;
        p_to->len += p->len;

    }
    memcpy((uint8*)p_to->payload+p_to->len, p->payload, p->len);
    p_to->tot_len += p->len;
    p_to->len += p->len;

}

_ATTR_LWBT_UARTIF_CODE_
void pbuf_ncopy(struct pbuf *p_to, struct pbuf *p_from, int len)
{
    memcpy((uint8*)p_to->payload+p_to->len, p_from->payload, len);
    p_to->tot_len += len;
    p_to->len += len;
}

_ATTR_LWBT_UARTIF_CODE_
int pbuf_queue_len(struct pbuf **head)
{
    int i =0;
    struct pbuf *q;
    if(*head == NULL)
    {
        return 0;
    }
	IntMasterDisable();
    q = *head;
    i++;
    while(q->next)
    {
        q = q->next;
        i++;
    }
	IntMasterEnable();
    return i;
}

_ATTR_LWBT_UARTIF_CODE_
int pbuf_queue_empty(struct pbuf **head)
{
    if(*head == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

_ATTR_LWBT_UARTIF_CODE_
void pbuf_queue_head(struct pbuf **head,struct pbuf *p)
{
	IntMasterDisable();
    if(*head == NULL)
    {
        *head = p;
    }
    else
    {
        p->next = *head;
        *head = p;

    }
	IntMasterEnable();
}

_ATTR_LWBT_UARTIF_CODE_
void pbuf_queue_tail(struct pbuf **head,struct pbuf *p)
{
    struct pbuf *q;
	IntMasterDisable();
    if(*head == NULL)
    {
        *head = p;
    }
    else
    {
        q = *head;
        while(q->next)
        {
            q = q->next;
        }

        q->next = p;

    }
	IntMasterEnable();
}

_ATTR_LWBT_UARTIF_CODE_
PBUF* pbuf_dequeue_head(struct pbuf **head)
{
    struct pbuf *p;

    if((*head == NULL))
    {
        return NULL;
    }
    else
    {
    	IntMasterDisable();
        p = *head;
        *head = (*head)->next;
        p->next = NULL;
		IntMasterEnable();
        return p;
    }
}

_ATTR_LWBT_UARTIF_CODE_
PBUF* pbuf_dequeue_tail(struct pbuf **head)
{
    struct pbuf *q;
    struct pbuf *p;

    if((*head == NULL))
    {
        return NULL;
    }
    else
    {
    	IntMasterDisable();
        q = *head;
        p = q;
        while(q->next)
        {
            p = q;
            q = q->next;
        }
        p->next = NULL;
        if(q == *head)
        {
            *head = NULL;
        }
		IntMasterEnable();
        return q;
    }
}

#endif
