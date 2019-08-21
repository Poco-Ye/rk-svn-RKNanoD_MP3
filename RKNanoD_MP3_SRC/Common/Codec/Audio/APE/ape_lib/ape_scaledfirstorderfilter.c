
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_scaledfirstorderfilter.h"

//这里的M是值31,S是值5
void ApeCScaledFirstOrderFilterSetMS(void* aI,ape_int32 M,ape_int32 S)
{
  ((CScaledFirstOrderFilter*)aI)->MULTIPLY=M;
  ((CScaledFirstOrderFilter*)aI)->SHIFT=S;
  ((CScaledFirstOrderFilter*)aI)->m_nLastValue = 0;
}

void ApeCScaledFirstOrderFilterFlush(void* aI)
{
  ((CScaledFirstOrderFilter*)aI)->m_nLastValue = 0;
}

//MULTIPLY is 31,SHIFT is 5	
ape_int32 ApeCScaledFirstOrderFilterCompress(void* aI, ape_int32 nInput)
{
  ape_int32 nRetVal = nInput 
       - ((((CScaledFirstOrderFilter*)aI)->m_nLastValue * ((CScaledFirstOrderFilter*)aI)->MULTIPLY  ) >>  ((CScaledFirstOrderFilter*)aI)->SHIFT );
  ((CScaledFirstOrderFilter*)aI)->m_nLastValue = nInput;
  return nRetVal;
}
	

ape_int32 ApeCScaledFirstOrderFilterDecompress(void* aI, ape_int32 nInput)
{
  ((CScaledFirstOrderFilter*)aI)->m_nLastValue = nInput + ((((CScaledFirstOrderFilter*)aI)->m_nLastValue * ((CScaledFirstOrderFilter*)aI)->MULTIPLY) >> ((CScaledFirstOrderFilter*)aI)->SHIFT);
  return ((CScaledFirstOrderFilter*)aI)->m_nLastValue;
}

#pragma arm section code

#endif
#endif