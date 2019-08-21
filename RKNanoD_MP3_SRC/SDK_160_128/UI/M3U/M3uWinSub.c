/*
********************************************************************************
*                   Copyright (c) 2008,CHENFEN
*                         All rights reserved.
*
* File Name:  M3uWinSub.c
* 
* Description: 
*
* History:      <author>          <time>        <version>
*               
*    desc:      ORG.
********************************************************************************
*/

#define _IN_M3U_CORE_

#include "SysInclude.h"

#ifdef  _M3U_

#include "FsInclude.h"
#include "M3uWin.h"
#include "M3uWinSub.h"


/*
--------------------------------------------------------------------------------
  Function name : M3uGetFileInfo
  Author        : chenfen
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_CORE_CODE_
M3uFileInfo * M3uGetFileInfo(uint8 * FileName)
{
	M3uFileInfo * FileInfo = gM3uFileInfo;

	while( FileInfo->FileType!= FileTypeALL)
	{

		if(TRUE == FileExtNameMatch(FileName+8,FileInfo->FileExtName, ATTR_ARCHIVE))//ATTR_ARCHIVE is use as file attr
		{

			return FileInfo;
		}
		FileInfo++;
	}
	return FileInfo;
}

/*
--------------------------------------------------------------------------------
  Function name : GetSearchFileInfo
  Author        : chenfen
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_CORE_CODE_
M3uFileInfo *M3uGetSearchFileInfo(FileType FileType)
{
	M3uFileInfo * FileInfo = gM3uFileInfo;

	while( FileInfo->FileType!= FileTypeALL)
	{
		if(FileInfo->FileType == FileType)//text type
		{
		    break;
		}
		FileInfo++;
	}
	return (FileInfo);
}

/*
--------------------------------------------------------------------------------
  Function name : M3uCheckFileType
  Author        : chenfen
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_M3U_CORE_CODE_
uint16 M3uCheckFileType( uint8 *SrcExtName, FileType FileType )
{

	M3uFileInfo * FileInfo;


	if( FileType == FileTypeALL)
	{
		return TRUE;
	}

    FileInfo = M3uGetFileInfo( SrcExtName );
 
	return( FileInfo->FileType == FileType );
}
/*
********************************************************************************
*
*                         End of M3uCore.c
*
********************************************************************************
*/
#endif
