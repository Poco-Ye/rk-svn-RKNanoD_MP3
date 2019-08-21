#ifndef _SBC_INTERFACE_H_
#define _SBC_INTERFACE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>	//for memcpy(),memmove()
#include "sbc.h"

int sbc_open();
int sbc_get_buffer(unsigned long ulParam1 , unsigned long ulParam2);
int sbc_dec();
int sbc_get_samplerate();
int sbc_get_channels();
int sbc_get_bitrate();
int sbc_get_length();
int sbc_get_timepos();
int sbc_seek(long time);
int sbc_close();

#endif