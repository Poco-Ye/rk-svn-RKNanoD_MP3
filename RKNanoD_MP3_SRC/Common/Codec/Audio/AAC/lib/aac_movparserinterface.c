#include "../include/audio_main.h"
#include "stdio.h"

#ifdef A_CORE_DECODE
#ifdef AAC_DEC_INCLUDE
#pragma arm section code = "AacDecCode", rodata = "AacDecCode", rwdata = "AacDecData", zidata = "AacDecBss"

#include "MovFile.h"
#define _IN_MOVPARSERINTERFACE_H
#include "movparserinterface.h"

#pragma arm section code
#endif
#endif
