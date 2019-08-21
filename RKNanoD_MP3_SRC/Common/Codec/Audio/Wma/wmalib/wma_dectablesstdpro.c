//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
#include "../include/audio_main.h"
#include "..\wmaInclude\DecTables.h"
#include "..\wmaInclude\macros.h"  //for FastFloat && NF2BP1
#include "..\wmaInclude\audio_table_room.h"
#include "..\wmaInclude\predefine.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"

#define F2BP1 NF2BP1

// create these preload tables with cossin project & cut & paste
#ifndef WMA_TABLE_ROOM_VERIFY

#else
SinCosTable *rgSinCosTables[SINCOSTABLE_ENTRIES];
#endif

#endif
#endif
//#endif // BUILD_WMASTD || BUILD_WMAPRO
