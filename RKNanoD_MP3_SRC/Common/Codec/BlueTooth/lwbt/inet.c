/**
 * @file
 * Functions common to all TCP/IPv4 modules, such as the byte order functions.
 *
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
//#include "lwipopt.h"
//#include "def.h"
//#include "BlueToothWin.h"
#include "inet.h"
#ifdef _BLUETOOTH_
/* Here for now until needed in other places in lwIP */
#ifndef isprint
#define in_range(c, lo, up)  ((uint8)c >= lo && (uint8)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#endif

/**
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @return ip address in network order
 */

#if 0
_ATTR_LWBT_CODE_
uint32 inet_addr(const char *cp)
{
    IN_ADDR val;

    if (inet_aton(cp, &val))
    {
        return (val.s_addr);
    }
    return (INADDR_NONE);
}

/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
_ATTR_LWBT_CODE_
int inet_aton(const char *cp, IN_ADDR *addr)
{
    uint32 val;
    uint8 base;
    char c;
    uint32 parts[4];
    uint32 *pp = parts;

    c = *cp;
    for (;;)
    {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, 1-9=decimal.
         */
        if (!isdigit(c))
            return (0);
        val = 0;
        base = 10;
        if (c == '0')
        {
            c = *++cp;
            if (c == 'x' || c == 'X')
            {
                base = 16;
                c = *++cp;
            }
            else
                base = 8;
        }
        for (;;)
        {
            if (isdigit(c))
            {
                val = (val * base) + (int)(c - '0');
                c = *++cp;
            }
            else if (base == 16 && isxdigit(c))
            {
                val = (val << 4) | (int)(c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            }
            else
                break;
        }
        if (c == '.')
        {
            /*
             * Internet format:
             *  a.b.c.d
             *  a.b.c   (with c treated as 16 bits)
             *  a.b (with b treated as 24 bits)
             */
            if (pp >= parts + 3)
                return (0);
            *pp++ = val;
            c = *++cp;
        }
        else
            break;
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !isspace(c))
        return (0);
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    switch (pp - parts + 1)
    {

        case 0:
            return (0);       /* initial nondigit */

        case 1:             /* a -- 32 bits */
            break;

        case 2:             /* a.b -- 8.24 bits */
            if (val > 0xffffffUL)
                return (0);
            val |= parts[0] << 24;
            break;

        case 3:             /* a.b.c -- 8.8.16 bits */
            if (val > 0xffff)
                return (0);
            val |= (parts[0] << 24) | (parts[1] << 16);
            break;

        case 4:             /* a.b.c.d -- 8.8.8.8 bits */
            if (val > 0xff)
                return (0);
            val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
            break;
    }
    if (addr)
        addr->s_addr = htonl(val);
    return (1);
}

/**
 * Convert numeric IP address into decimal dotted ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         represenation of addr
 */
_ATTR_LWBT_CODE_
char *inet_ntoa(IN_ADDR addr)
{
    static char str[16];
    uint32 s_addr = addr.s_addr;
    char inv[3];
    char *rp;
    uint8 *ap;
    uint8 rem;
    uint8 n;
    uint8 i;

    rp = str;
    ap = (uint8 *)&s_addr;
    for (n = 0; n < 4; n++)
    {
        i = 0;
        do
        {
            rem = *ap % (uint8)10;
            *ap /= (uint8)10;
            inv[i++] = '0' + rem;
        }
        while (*ap);
        while (i--)
            *rp++ = inv[i];
        *rp++ = '.';
        ap++;
    }
    *--rp = 0;
    return str;
}

#endif

/**
 * These are reference implementations of the byte swapping functions.
 * Again with the aim of being simple, correct and fully portable.
 * Byte swapping is the second thing you would want to optimize. You will
 * need to port it to your architecture and in your cc.h:
 *
 * #define LWIP_PLATFORM_BYTESWAP 1
 * #define LWIP_PLATFORM_HTONS(x) <your_htons>
 * #define LWIP_PLATFORM_HTONL(x) <your_htonl>
 *
 * Note ntohs() and ntohl() are merely references to the htonx counterparts.
 */

#if (LWIP_PLATFORM_BYTESWAP == 0) && (BYTE_ORDER == LITTLE_ENDIAN)

/**
 * Convert an uint16 from host- to network byte order.
 *
 * @param n uint16 in host byte order
 * @return n in network byte order
 */
_ATTR_LWBT_CODE_
uint16 htons(uint16 n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 * Convert an uint16 from network- to host byte order.
 *
 * @param n uint16 in network byte order
 * @return n in host byte order
 */
_ATTR_LWBT_CODE_
uint16 ntohs(uint16 n)
{
    return htons(n);
}

/**
 * Convert an uint32 from host- to network byte order.
 *
 * @param n uint32 in host byte order
 * @return n in network byte order
 */
_ATTR_LWBT_CODE_
uint32 htonl(uint32 n)
{
    return ((n & 0xff) << 24) |
           ((n & 0xff00) << 8) |
           ((n & 0xff0000UL) >> 8) |
           ((n & 0xff000000UL) >> 24);
}

/**
 * Convert an uint32 from network- to host byte order.
 *
 * @param n uint32 in network byte order
 * @return n in host byte order
 */
_ATTR_LWBT_CODE_
uint32 ntohl(uint32 n)
{
    return htonl(n);
}

#endif /* (LWIP_PLATFORM_BYTESWAP == 0) && (BYTE_ORDER == LITTLE_ENDIAN) */

#endif
