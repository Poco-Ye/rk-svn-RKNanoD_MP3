#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_Infodec.h"
//#include "ape_globalvardeclaration.h"
#include "ape_globalvardefine_keyData.h"
#include "ape_headerdec.h"
#include "ape_decompress.h"
#include "ape_io1.h"
//#include "pAPE_DEC.h"

//////////////////////////////
//#include "../include/globals.h"
//#include "../include/CommonCmd.h"
//#include "../buffer/buffer.h"
//////////////////////////////////
//#include "../include/file_access.h"

#pragma arm section code

#endif
#endif