/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: trigtabs.c,v 1.1 2005/02/26 01:47:35 jrecker Exp $ 
 *   
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.  
 *       
 * The contents of this file, and the files included with this file, 
 * are subject to the current version of the RealNetworks Public 
 * Source License (the "RPSL") available at 
 * http://www.helixcommunity.org/content/rpsl unless you have licensed 
 * the file under the current version of the RealNetworks Community 
 * Source License (the "RCSL") available at 
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL 
 * will apply. You may also obtain the license terms directly from 
 * RealNetworks.  You may not use this file except in compliance with 
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable 
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for 
 * the rights, obligations and limitations governing use of the 
 * contents of the file. 
 *   
 * This file is part of the Helix DNA Technology. RealNetworks is the 
 * developer of the Original Code and owns the copyrights in the 
 * portions it created. 
 *   
 * This file, and the files included with this file, is distributed 
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS 
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET 
 * ENJOYMENT OR NON-INFRINGEMENT. 
 *  
 * Technology Compatibility Kit Test Suite(s) Location:  
 *    http://www.helixcommunity.org/content/tck  
 *  
 * Contributor(s):  
 *   
 * ***** END LICENSE BLOCK ***** */  

/**************************************************************************************
 * Fixed-point HE-AAC decoder
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * February 2005
 *
 * trigtabs.c - tables of sin, cos, etc. for IMDCT
 **************************************************************************************/

#include "coder.h"
	#include "../../include/audio_main.h"
#include "aac_gen_table/aac_table.h"

#ifdef A_CORE_DECODE
#ifdef aac_table_test
 _ATTR_AACDEC_DATA_RO
 int *cos4sin4tabOffset=(int *)(base+cos4sin4tabOffset_offset);
#else
 int cos4sin4tabOffset[NUM_IMDCT_SIZES] = {0, 128};
#endif
/* PreMultiply() tables
 * format = Q30 * 2^[-7, -10] for nmdct = [128, 1024] 
 * reordered for sequential access
 *
 * invM = -1.0 / nmdct;
 * for (i = 0; i < nmdct/4; i++) {
 *   angle = (i + 0.25) * M_PI / nmdct;
 *   x = invM * (cos(angle) + sin(angle));
 *   x = invM * sin(angle);
 * 
 *   angle = (nmdct/2 - 1 - i + 0.25) * M_PI / nmdct;
 *   x = invM * (cos(angle) + sin(angle));
 *   x = invM * sin(angle);
 * }
 */
 #ifdef aac_table_test
 _ATTR_AACDEC_DATA_RO
  int *cos4sin4tab = (int *)(base+cos4sin4tab_offset);
 #else 

#endif
/* PostMultiply() tables
 * format = Q30
 * reordered for sequential access
 * decimate (skip by 16 instead of 2) for small transform (128)
 *
 * for (i = 0; i <= (512/2); i++) {
 *   angle = i * M_PI / 1024;
 *   x = (cos(angle) + sin(angle));
 *   x = sin(angle);
 * }
 */

#ifdef aac_table_test
 _ATTR_AACDEC_DATA_RO
 int *cos1sin1tab = (int *)(base +cos1sin1tab_offset);
#else

#endif
#ifdef aac_table_test
_ATTR_AACDEC_DATA_RO
 int *sinWindowOffset = (int *)(base+sinWindowOffset_offset);
#else 
 int sinWindowOffset[NUM_IMDCT_SIZES] = {0, 128};
#endif

/* Synthesis window - SIN
 * format = Q31 for nmdct = [128, 1024]
 * reordered for sequential access
 *
 * for (i = 0; i < nmdct/2; i++) {
 *   angle = (i + 0.5) * M_PI / (2.0 * nmdct);
 *   x = sin(angle);
 *
 *   angle = (nmdct - 1 - i + 0.5) * M_PI / (2.0 * nmdct);
 *   x = sin(angle);
 * }
 */
 #ifdef aac_table_test
 _ATTR_AACDEC_DATA_RO
  int *sinWindow=(int *)(base + sinWindow_offset);
 #else

#endif

#ifdef aac_table_test
  _ATTR_AACDEC_DATA_RO
 int *kbdWindowOffset = (int *)(base +kbdWindowOffset_offset);
#else
 int kbdWindowOffset[NUM_IMDCT_SIZES] = {0, 128};
#endif
/* Synthesis window - KBD
 * format = Q31 for nmdct = [128, 1024]
 * reordered for sequential access
 *
 * aacScaleFact = -sqrt(1.0 / (2.0 * nmdct));
 * for (i = 0; i < nmdct/2; i++) {
 *   x = kbdWindowRef[i] * aacScaleFact;
 *   x = kbdWindowRef[nmdct - 1 - i] * aacScaleFact;
 * }
 * Note: see below for code to generate kbdWindowRef[]
 */

#ifdef aac_table_test
_ATTR_AACDEC_DATA_RO
 int *kbdWindow = (int *)(base +kbdWindow_offset);
#else

#endif


/* bit reverse tables for FFT */
#ifdef aac_table_test
_ATTR_AACDEC_DATA_RO
 int *bitrevtabOffset =(int *)( base +bitrevtabOffset_offset);
_ATTR_AACDEC_DATA_RO
 unsigned char *bitrevtab = base +bitrevtab_offset;
_ATTR_AACDEC_DATA_RO
 unsigned char *uniqueIDTab = base +uniqueIDTab_offset;
_ATTR_AACDEC_DATA_RO
 int *twidTabOdd =(int *)( base +twidTabOdd_offset);
_ATTR_AACDEC_DATA_RO
 int *twidTabEven = (int *)(base +twidTabEven_offset);
#else

#endif
#endif
