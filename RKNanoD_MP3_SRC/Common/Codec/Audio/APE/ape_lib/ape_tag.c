
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_id3genres.h"
#include "ape_tag.h"
//#include "CharacterHelper.h"
#include "ape_io1.h"
#include "ape_globalvardeclaration.h"
//#include "Byte2Word.h"
extern int ape_TitleIndex;
extern int ape_ArtistIndex;
extern int ape_AlbumIndex;
/*****************************************************************************************
CAPETag
*****************************************************************************************/

void ApeTagFileInitialize(CAPETag* aI,ape_uint16 * pFilename, ape_BOOL bAnalyze)
{
  //aI->m_spIO=(IO_CLASS_NAME*)malloc(sizeof(IO_CLASS_NAME));
  aI->m_spIO=&Ape_gApeTagIO;
  ApeIoInitialize(aI->m_spIO);
  
  aI->m_bAnalyzed = FALSE;
  aI->m_nFields = 0;
  aI->m_nTagBytes = 0;
  aI->m_bIgnoreReadOnly = FALSE;
  
  if (bAnalyze)
  {
    aI->Analyze(aI);
  }
}

#if 0 //commented by hxd
void ApeTagIo(CAPETag* aI,CIO * pIO, ape_BOOL bAnalyze)
{
  /*Mod by Wei.Hisung 2007.03.06*/
  //m_spIO.Assign(pIO, FALSE, FALSE); // we don't own the IO source
  aI->m_spIO=pIO; // we don't own the IO source
  aI->m_bAnalyzed = FALSE;
  aI->m_nFields = 0;
  aI->m_nTagBytes = 0;
  
  aI->Analyze =(ape_int32 (*)(void *))ApeTagAnalyze ;
  aI->cCAPETagFile =(void (*)(void *,ape_char *,ape_int32))ApeTagFileInitialize ;
  aI->cCAPETagIO =(void (*)(void *,CIO *,ape_BOOL))ApeTagIo ;
  //aI->dCAPETag =(void ( *)(void *))CAPETag_dCAPETag ;
  aI->GetAPETagVersion =(ape_int32 ( *)(void *))ApeTagGetApeTagVersion ;
  aI->GetHasAPETag =(ape_int32 ( *)(void *))ApeTagGetHasApeTag ;
  aI->GetHasID3Tag =(ape_int32 ( *)(void *))ApeTagGetHasId3Tag ;
  aI->GetTagBytes =(ape_int32 ( *)(void *))ApeTagGetTagBytes ;
  aI->LoadField =(ape_int32 ( *)(void *, ape_char *,ape_int32,ape_int32 *))ApeTagLoadField ;
    
  if (bAnalyze)
  {
     aI->Analyze(aI);
  }
}
#endif

#if 0
void CAPETag_dCAPETag(CAPETag* aI)
{
//    aI->ClearFields(aI);
}
#endif

ape_int32 ApeTagGetTagBytes(CAPETag* aI)
{
  if (aI->m_bAnalyzed == FALSE) 
  { 
    aI->Analyze(aI); 
  }
  
  return aI->m_nTagBytes;
}

#if 0 //commented by hxd 20070710
ape_int32 ApeTagAnalyze(CAPETag* aI)
{
  // clean-up
  //struct ID3_TAG Ape_gId3Tag;
  ape_int32 nOriginalPosition;
  ape_uint32 nBytesRead;
  ape_int32 nRetVal;
  //aI->ClearFields(aI);
  aI->m_nTagBytes = 0;
  
  aI->m_bAnalyzed = TRUE;
  
  // store the original location
  nOriginalPosition = aI->m_spIO->GetPosition(aI->m_spIO);
  
  // check for a tag
  
  aI->m_bHasID3Tag = FALSE;
  aI->m_bHasAPETag = FALSE;
  aI->m_nAPETagVersion = -1;
  aI->m_spIO->Seek(aI->m_spIO,-ID3_TAG_BYTES, FILE_END);
  nRetVal = aI->m_spIO->Read(aI->m_spIO,(ape_uchar *) &Ape_gId3Tag, sizeof(struct ID3_TAG), &nBytesRead);
  //nRetVal = aI->m_spIO->Read(aI->m_spIO,&Ape_gReadBuffer, sizeof(struct ID3_TAG)*2, &nBytesRead);
  //Byte2Word2(&Ape_gId3Tag,&Ape_gReadBuffer,sizeof(struct ID3_TAG));
  
  if ((nBytesRead == sizeof(struct ID3_TAG)) && (nRetVal == 0))
  {
    if (Ape_gId3Tag.Header[0] == 'T' && Ape_gId3Tag.Header[1] == 'A' && Ape_gId3Tag.Header[2] == 'G') 
    {
        aI->m_bHasID3Tag = TRUE;
        aI->m_nTagBytes += ID3_TAG_BYTES;
    }
  }
  
  // set the fields
  if (aI->m_bHasID3Tag)
  {
  /*
  char cTemp[16];
  
  aI->SetFieldID3String(aI,APE_TAG_FIELD_ARTIST, Ape_gId3Tag.Artist, 30);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_ALBUM, Ape_gId3Tag.Album, 30);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_TITLE, Ape_gId3Tag.Title, 30);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_COMMENT, Ape_gId3Tag.Comment, 28);
  aI->SetFieldID3String(aI,APE_TAG_FIELD_YEAR, Ape_gId3Tag.Year, 4);
  
  sprintf(cTemp, "%d", Ape_gId3Tag.Track);
  aI->SetFieldStringCC(aI,APE_TAG_FIELD_TRACK, cTemp, FALSE);
  
  if ((Ape_gId3Tag.Genre == GENRE_UNDEFINED) || (Ape_gId3Tag.Genre >= GENRE_COUNT)) 
      aI->SetFieldStringUtf(aI,APE_TAG_FIELD_GENRE, APE_TAG_GENRE_UNDEFINED);
  else 
      aI->SetFieldStringUtf(aI,APE_TAG_FIELD_GENRE, g_ID3Genre[Ape_gId3Tag.Genre]);
  */
  }
  
  // try loading the APE tag
  if (aI->m_bHasID3Tag == FALSE)
  {
    APE_TAG_FOOTER APETagFooter;
    APETagFooter.cAPE_TAG_FOOTER =(void (*)(void *,ape_int32,ape_int32))ApeTagFooter ;
    APETagFooter.GetFieldBytes =(ape_int32 (*)(void *))ApeTagFooterGetFieldBytes; 
    APETagFooter.GetFieldsOffset= (ape_int32 (*)(void *))ApeTagFooterGetFieldsOffset;
    APETagFooter.GetHasHeader=(ape_int32 (*)(void *))ApeTagFooterGetHasHeader ;
    APETagFooter.GetIsHeader=(ape_int32 (*)(void *))ApeTagFooterGetIsHeader ;
    APETagFooter.GetIsValid=(ape_int32 (*)(void *,ape_int32))ApeTagFooterGetIsValid ;
    APETagFooter.GetNumberFields=(ape_int32 (*)(void *))ApeTagFooterGetNumberFields ;
    APETagFooter.GetTotalTagBytes=(ape_int32 (*)(void *))ApeTagFooterGetTotalTagBytes ;
    APETagFooter.GetVersion=(ape_int32 (*)(void *))ApeTagFooterGetVersion; 
    
    aI->m_spIO->Seek(aI->m_spIO,-(ape_int32)(APE_TAG_FOOTER_BYTES), FILE_END);
    nRetVal = aI->m_spIO->Read(aI->m_spIO,(ape_uchar *) &APETagFooter, APE_TAG_FOOTER_BYTES, &nBytesRead);
    //nRetVal = aI->m_spIO->Read(aI->m_spIO,&Ape_gReadBuffer, APE_TAG_FOOTER_BYTES, &nBytesRead);
    //Byte2Word2(&APETagFooter,&Ape_gReadBuffer,(ape_uint16)(APE_TAG_FOOTER_BYTES/2));
    if ((nBytesRead == APE_TAG_FOOTER_BYTES) && (nRetVal == 0))
    {
      if (APETagFooter.GetIsValid(&APETagFooter,FALSE))
      {
        ape_int32 nRawFieldBytes;
        ape_char* spRawTag;
        
        aI->m_bHasAPETag = TRUE;
        aI->m_nAPETagVersion = APETagFooter.GetVersion(&APETagFooter);
        
        nRawFieldBytes = APETagFooter.GetFieldBytes(&APETagFooter);
        aI->m_nTagBytes += APETagFooter.GetTotalTagBytes(&APETagFooter);
        
        spRawTag=Ape_gReadBuffer;
        
        aI->m_spIO->Seek(aI->m_spIO,-(APETagFooter.GetTotalTagBytes(&APETagFooter) - APETagFooter.GetFieldsOffset(&APETagFooter)), FILE_END);
        /*Mod by Wei.Hisung 2007.03.06*/
        //nRetVal = m_spIO->Read((ape_uchar *) spRawTag.GetPtr(), nRawFieldBytes, &nBytesRead);
        //nRetVal = aI->m_spIO->Read(aI->m_spIO,(ape_uchar *) spRawTag, nRawFieldBytes, &nBytesRead);
        
        //if ((nRetVal == 0) && (nRawFieldBytes == (ape_int32)(nBytesRead)))
        {
          // parse out the raw fields
          ape_int32 nLocation = 512;
          ape_int32 nBytesRead = 512;
          ape_int32 z=0;
          //add by Wei.hsiung
          aI->m_nFields=0;
          
          for (z = 0; z < APETagFooter.GetNumberFields(&APETagFooter); z++)
          {
            //ape_int32 nMaximumFieldBytes = nRawFieldBytes - nLocation;
            
            ape_int32 nBytes = 0;
            
            aI->m_spIO->Seek(aI->m_spIO,(nLocation-nBytesRead), FILE_CURRENT);
            
            aI->m_spIO->Read(aI->m_spIO,(ape_uchar *) spRawTag, 512 , (ape_uint32*)&nBytesRead);
            
            //nLocation+=nBytesRead;
            
            nLocation = aI->LoadField(aI,spRawTag, nBytesRead, &nBytes);
            
            if ((nLocation == (-1))||(z>APETAG_FIELDS_NUM_MAX-1))
            {
        	    break;
        	  }
                    //if (aI->LoadField(aI,&spRawTag[nLocation], nMaximumFieldBytes, &nBytes) != ERROR_SUCCESS)
                    //{
                        // if LoadField(...) fails, it means that the tag is corrupt (accidently or intentionally)
                        // we'll just bail out -- leaving the fields we've already set
                        //break;
                    //}
                    //nLocation += nBytes;
          }
        }
      }
    }
  }

    // restore the file pointer
    aI->m_spIO->Seek(aI->m_spIO,nOriginalPosition, FILE_BEGIN);
    
    return ERROR_SUCCESS;
}
#endif

#if 0
ape_int32 CAPETag_ClearFields(CAPETag* aI)
{
  ape_int32 z;
  for (z = 0; z < aI->m_nFields; z++)
  {
      SAFE_DELETE(aI->m_aryFields[z])
  }
  
  aI->m_nFields = 0;
  
  return ERROR_SUCCESS;
}
#endif

//Re-write by Vincent.Hisung 2007.03.22
ape_int32 ApeTagLoadField(CAPETag* aI,ape_char * pBuffer, ape_int32 nMaximumBytes, ape_int32 * pBytes)
{
  ape_int32 nFieldValueSize ;
  ape_int32 nFieldFlags ;
  ape_int32 nMaximumRead;
  ape_BOOL bSafe = TRUE;
  ape_int32 z;
  
  //Ape_pApeTagAryFields=(ape_char (*)[APETAG_FIELDS_NUM_MAX][32])Ape_gInBuffer;
  Ape_pApeTagAryFields=(ape_char*)Ape_pInBuffer;//Ape_gInBuffer;
      
  // size and flags    
  nFieldValueSize = *((ape_int32 *) &pBuffer[0]);
  nFieldFlags = *((ape_int32 *) &pBuffer[4]);
  
  // safety check (so we can't get buffer overflow attacked)
  nMaximumRead = nMaximumBytes - 8 - nFieldValueSize;
  
  for (z = 0; (z < nMaximumRead) && (bSafe == TRUE); z++)
  {
      ape_int32 nCharacter = pBuffer[8 + z];
      if (nCharacter == 0)
      {
        break;
      }
      if ((nCharacter < 0x20) || (nCharacter > 0x7E))
      {
        bSafe = FALSE;
      }
      //ADD BY VINCENT
      if (z==1)
        	{
        		if (pBuffer[8+z]==0x69)//=='i'
        			{
        				ape_TitleIndex=aI->m_nFields;
        			}
        		else if (pBuffer[8+z]==0x72)//=='r'
        			{
        				ape_ArtistIndex=aI->m_nFields;
        			}
        		else if (pBuffer[8+z]==0x6c)//=='l'
        			{
        				ape_AlbumIndex=aI->m_nFields;
        			}        		
        	}
  }
  if (bSafe == FALSE)
  {
    return -1;
  }
  
  // name
  //nNameCharacters = strlen(&pBuffer[nLocation]);
  //TODO!
  
  // value
  if (nFieldValueSize<30)
  {
    memcpy(&Ape_pApeTagAryFields[aI->m_nFields*32], &pBuffer[z+9], nFieldValueSize);		
    Ape_pApeTagAryFields[aI->m_nFields*32+nFieldValueSize]=0;
  }
  else
  {
    memcpy(&Ape_pApeTagAryFields[aI->m_nFields*32], &pBuffer[z+9], 30);
    Ape_pApeTagAryFields[aI->m_nFields*32+31]=0;
  }
  
  aI->m_aryFields[aI->m_nFields]=&Ape_pApeTagAryFields[aI->m_nFields*32];
  aI->m_nFields++;
    
  //return z+9+nFieldValueSize;
  return z+9+nFieldValueSize;
}

ape_BOOL ApeTagGetHasId3Tag(CAPETag* aI) 
{ 
  if (aI->m_bAnalyzed == FALSE) 
  { 
  	aI->Analyze(aI); 
  } 
  return aI->m_bHasID3Tag;    
}

ape_BOOL ApeTagGetHasApeTag(CAPETag* aI) 
{ 
  if (aI->m_bAnalyzed == FALSE) 
  { 
  	aI->Analyze(aI); 
  } 
  return aI->m_bHasAPETag;    
}

ape_int32 ApeTagGetApeTagVersion(CAPETag* aI) 
{ 
  return aI->GetHasAPETag(aI) ? aI->m_nAPETagVersion : -1;    
}

//---------------------------------------
void ApeTagFooter(APE_TAG_FOOTER *aI,ape_int32 nFields, ape_int32 nFieldBytes)
{
  memcpy(aI->m_cID, "APETAGEX", LENGTH(8));
  memset(aI->m_cReserved, 0, LENGTH(8));
  aI->m_nFields = nFields;
  aI->m_nFlags = APE_TAG_FLAGS_DEFAULT;
  aI->m_nSize = nFieldBytes + APE_TAG_FOOTER_BYTES;
  aI->Ape_gVersion = CURRENT_APE_TAG_VERSION;
}

ape_int32 ApeTagFooterGetTotalTagBytes(APE_TAG_FOOTER *aI) 
{ 
  return aI->m_nSize + (aI->GetHasHeader(aI) ? APE_TAG_FOOTER_BYTES : 0); 
}

ape_int32 ApeTagFooterGetFieldBytes(APE_TAG_FOOTER *aI)
{ 
  return aI->m_nSize - APE_TAG_FOOTER_BYTES; 
}

ape_int32 ApeTagFooterGetFieldsOffset(APE_TAG_FOOTER *aI) 
{ 
  return aI->GetHasHeader(aI) ? APE_TAG_FOOTER_BYTES : 0; 
}

ape_int32 ApeTagFooterGetNumberFields(APE_TAG_FOOTER *aI) 
{ 
  return aI->m_nFields; 
}

ape_BOOL ApeTagFooterGetHasHeader(APE_TAG_FOOTER *aI) 
{ 
  return (aI->m_nFlags & APE_TAG_FLAG_CONTAINS_HEADER) ? TRUE : FALSE; 
}

ape_BOOL ApeTagFooterGetIsHeader(APE_TAG_FOOTER *aI) 
{ 
  return (aI->m_nFlags & APE_TAG_FLAG_IS_HEADER) ? TRUE : FALSE; 
}

ape_int32 ApeTagFooterGetVersion(APE_TAG_FOOTER *aI) 
{ 
  return aI->Ape_gVersion; 
}

#if 0 //commented by hxd 20070709
ape_BOOL ApeTagFooterGetIsValid(APE_TAG_FOOTER *aI,ape_BOOL bAllowHeader)
{
  //ape_BOOL bValid = (strncmp(aI->m_cID, "APETAGEX", 8)  
  ape_BOOL bValid = (aI->m_cID[0] == 'A') && (aI->m_cID[1] == 'P') && (aI->m_cID[2] == 'E') && (aI->m_cID[3] == 'T') &&
  (aI->m_cID[4] == 'A') && (aI->m_cID[5] == 'G') && (aI->m_cID[6] == 'E') && (aI->m_cID[7] == 'X') &&
  (aI->Ape_gVersion <= CURRENT_APE_TAG_VERSION) &&
  (aI->m_nFields <= (ape_int32)65536) &&
  (aI->GetFieldBytes(aI) <= ((ape_int32)1024 * (ape_int32)1024 * (ape_int32)16));
  
  if (bValid && (bAllowHeader == FALSE) && aI->GetIsHeader(aI))
  {
    bValid = FALSE;
  }
  
  return bValid ? TRUE : FALSE;
}
#endif
//-----------------------------------------------------

#pragma arm section code

#endif
#endif
