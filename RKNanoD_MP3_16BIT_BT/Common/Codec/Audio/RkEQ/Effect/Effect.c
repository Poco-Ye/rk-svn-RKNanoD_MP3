

/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     effect.c
*  Description:    音频效果处理
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     07/09/27      1.0
*
*******************************************************************************/

#include "audio_globals.h"
#include "SysInclude.h"
#ifdef _RK_EQ_
#include "Rk_eq.h"

#include "AudioControl.h"
#include "Effect.h"

extern AudioInOut_Type     AudioIOBuf;
extern int CurrentCodec;

#define EQ_NUM  5

/**************************************************************************
* function description: set parameters of RockEQ
* input:  [pEft]   --
* output:
* return:    0:success; other:failure.
* note:
***************************************************************************/
#define EQADDVOL     6

_ATTR_AUDIO_DATA_ short PresetGain[4][EQ_NUM] =
{
    {15, 12, 9, 15, 12}, // HEAVY
    {12, 15, 15, 9, 12}, // POP
    {18, 12, 12, 9, 15}, // JAZZ
    {18, 6, 16, 13, 18}  // UNIQUE
                         //EQ_CLASS,
                         //EQ_BASS,
                         //EQ_ROCK,
};

_ATTR_AUDIO_DATA_ short EqBassBoostGain[EQ_NUM] = {12, 12, 12, 12, 12}; // BASS BOOST
_ATTR_AUDIO_BSS_  short UseEQ[EQ_NUM];

/**************************************************************************
* function description: audio effect is end.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
_ATTR_AUDIO_TEXT_
ReadEqData(char *p , int off ,int size)
{
    ReadModuleData(MODULE_ID_AUDIO_EQ , p , off , size );
}

_ATTR_AUDIO_TEXT_
void EqAdjustVolume(void)
{
    Codec_SetVolumet(AudioPlayInfo.playVolume);
}

/**************************************************************************
* function description: audio effect is end.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
_ATTR_AUDIO_TEXT_
long RKEQAdjust(RKEffect *pEft)
{
    int i;
    unsigned long SamplingRate = 44100;
	int err = 0;//<----sanshin_20151114

    CodecGetSampleRate(&SamplingRate);
    switch(SamplingRate)
        {
            case 8000: 
			case 11025:
            case 12000 :
				SamplingRate = 11025;     	   	    
     		    break;
            case 16000: 
     	   	case 22050:      
            case 24000: 
     	   	    SamplingRate = 22050;  
     		    break;
            case 32000:    	   	  
     		case 44100:    
            case 48000: 
            case 64000:
            case 88200:
            case 96000:
            case 128000:
            case 176400:
            case 192000:  
               
     	        SamplingRate = 44100;  
     		    break;
        	default:		//<----sanshin_20151114
        		err = 1;	//<----sanshin_20151114
        		break;		//<----sanshin_20151114
	   }

	if(err == 0)			//<----sanshin_20151114
	{
    	switch (pEft->Mode)
    	{
    	    case EQ_ROCK:
    	    case EQ_POP:
    	    case EQ_CLASS:
    	    case EQ_JAZZ:
    	    case EQ_HEAVY:
    	    case EQ_UNIQUE:           
    	        RockEQAdjust(SamplingRate, PresetGain[pEft->Mode], 1);
    	        break;

    	    case EQ_USER:
    	        for (i = 0;i < 5;i++)
    	        {                
    	            UseEQ[i] = pEft->RKCoef.dbGain[i] + 12;                
    	        }
    	        RockEQAdjust(SamplingRate, UseEQ, 1);
    	        break;

    	    case EQ_BASS:
    	        RockEQAdjust(SamplingRate, EqBassBoostGain, 1);
    	        break;
    	    default:
    	        break;
    	}
	}
	else
	{
		SendMsg(MSG_AUDIO_EQSET_UPDATA);	//<----sanshin_20151114
	}
    return 0;
}

/**************************************************************************
* function description: audio effect is processing.
* input   :[pBuffer] -- the buffer put pcm data.
		   [PcmLen] --  the length of pcm data.
* output  : null
* return  : 0:success; other:failure.
* note    : the data that audio effect would take care need use data format LRLRLR.
***************************************************************************/
_ATTR_AUDIO_TEXT_
long EffectProcess(EQ_TYPE *pBuffer, long PcmLen)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;
	int Mode;

    if (pBuffer == 0)
        return 1;

    if (PcmLen <= 0)
        return 0;

    switch (pEffect->Mode)
    {
        case EQ_NOR:
        case EQ_ROCK:
        case EQ_POP:
        case EQ_CLASS:
        case EQ_BASS:
        case EQ_JAZZ:
        case EQ_HEAVY:
        case EQ_UNIQUE:
        case EQ_USER:
			RockEQProcess(pBuffer, PcmLen);
            break;

        default:
            break;
    }

    return 0; //success
}

/**************************************************************************
* description   : audio effect adjust.
* input         : null
* output        : null
* return        : 0:success; other:failure.
* note          :
***************************************************************************/
_ATTR_AUDIO_TEXT_
long EffectAdjust(void)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect         *pEffect = &pAudio->EffectCtl;
    unsigned long SamplingRate = 44100;
    int i,j=0;
    int Mode;

    DEBUG("EQ_Mode: = %d", pEffect->Mode);
    
    CodecGetSampleRate(&SamplingRate);

    UserIsrDisable();
    switch (pEffect->Mode)
    {
        case EQ_NOR:
            FREQ_ExitModule(FREQ_EQ);
            break;

        case EQ_POP:
        case EQ_CLASS:
        case EQ_BASS:
        case EQ_JAZZ:
        case EQ_HEAVY:
        case EQ_UNIQUE:
        case EQ_USER:
			/* TODO : here load code */
            FREQ_EnterModule(FREQ_EQ);
            RKEQAdjust(pEffect);
            break;

        default:
            break;
    }

    UserIsrEnable();

    return 0; //success.
}


/**************************************************************************
* function description: audio effect is initialization.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
_ATTR_AUDIO_TEXT_
long EffectInit(void)
{
    ModuleOverlay(MODULE_ID_AUDIO_RKEQ, MODULE_OVERLAY_ALL);
    return 0;
}


/**************************************************************************
* function description: audio effect is end.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
 _ATTR_AUDIO_TEXT_
long EffectEnd(void)
{
	AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl; 
    
    FREQ_ExitModule(FREQ_EQ);
    
    return 0;
}


_ATTR_AUDIO_TEXT_
void RockEQReduce9dB(EQ_TYPE *pwBuffer, long cwBuffer, long LR, long mode)
{
	AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;

    if(pEffect->Mode == EQ_BASS)
    {
        while(cwBuffer--)
        {
            if(g_bass_FilterState.max_DbGain[LR] == 12)
            {
                *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer))>>2);
            } //20log(0.25)=-12  //20log(3/8)=8.5
            else if(g_bass_FilterState.max_DbGain[LR] == 10)
            {
              *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*5)>>4);
            }
            else if(g_bass_FilterState.max_DbGain[LR] == 8)
            {
              *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*3)>>3);
            }
            else if(g_bass_FilterState.max_DbGain[LR] == 6)
            {
              *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer))>>1);
            }
            else if(g_bass_FilterState.max_DbGain[LR] == 4)
            {
              *pwBuffer = (short)(((long)(*pwBuffer)*5)>>3);
            }
            else if(g_bass_FilterState.max_DbGain[LR] == 2)
            {
              *pwBuffer = (short)(((long)(*pwBuffer)*3)>>2);
            }

       		pwBuffer += mode;
        }
        return;
    }

    switch(g_FilterState.mode[LR])
    {
       case EQ_HEAVY:
	   case EQ_POP:
            while(cwBuffer--)
            {
        		*pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*5)>>3);//20log(0.625)=-4
        		pwBuffer += mode;
            }
			 break;
	   case EQ_JAZZ:
       case EQ_UNIQUE:
            while(cwBuffer--)
            {
        		*pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*7)>>4);//20log(0.438)=-7.1
        		pwBuffer += mode;
            }
			 break;
       case EQ_USER:
		   	while(cwBuffer--)
            {
        		*pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer))>>2);//20log(0.25)=-12
                 pwBuffer += mode;
            }
	   	     break;
    }
}

#endif// _RK_EQ_