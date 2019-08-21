#include "sysinclude.h"
#include "audio_main.h"

#ifdef MP3_DEC_INCLUDE
_ATTR_MP3DEC_BIN_TEXT_
const uint8 mp3_code_bin[] =
{
    #include "mp3_code.bin"
};


_ATTR_MP3DEC_BIN_DATA_
uint8 mp3_data_bin[] =
{
    #include "mp3_data.bin"  
};
#endif

#ifdef MP3_ENC_INCLUDE
_ATTR_MP3ENC_BIN_TEXT_
const uint8 mp3_enc_code_bin[] =
{
    #include "mp3_enc_code.bin"
};
_ATTR_MP3ENC_BIN_DATA_
uint8 mp3_enc_data_bin[] =
{
    #include "mp3_enc_data.bin"  
};
#endif

#ifdef WAV_DEC_INCLUDE
_ATTR_WAVDEC_BIN_TEXT_
const uint8 wav_code_bin[] =
{
    #include "wav_code.bin"
};


_ATTR_WAVDEC_BIN_DATA_
uint8 wav_data_bin[] =
{
    #include "wav_data.bin"  
};
#endif

#ifdef AAC_DEC_INCLUDE
_ATTR_AACDEC_BIN_TEXT_
const uint8 aac_code_bin[] =
{
    #include "aac_code.bin"
};


_ATTR_AACDEC_BIN_DATA_
uint8 aac_data_bin[] =
{
    #include "aac_data.bin"  
};
#endif

#ifdef WMA_DEC_INCLUDE
_ATTR_WMADEC_BIN_TEXT_
const uint8 wma_code_bin[] =
{
    #include "wma_code.bin"
};


_ATTR_WMADEC_BIN_DATA_
uint8 wma_data_bin[] =
{
    #include "wma_data.bin"  
};
#endif

#ifdef FLAC_DEC_INCLUDE
_ATTR_FLACDEC_BIN_TEXT_
const uint8 flac_code_bin[] =
{
    #include "flac_code.bin"
};

_ATTR_FLACDEC_BIN_DATA_
uint8 flac_data_bin[] =
{
    #include "flac_data.bin"
};
#endif
