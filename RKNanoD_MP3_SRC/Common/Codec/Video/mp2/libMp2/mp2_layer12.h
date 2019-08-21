/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name£º   layer12.h
* 
* Description:  layer I, layer II
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

# ifndef LIBMAD_LAYER12_H
# define LIBMAD_LAYER12_H

# include "mp2_stream.h"
# include "mp2_frame.h"

int mad_layer_I(struct mad_stream *, struct mad_frame *);
int mad_layer_II(struct mad_stream *, struct mad_frame *);

# endif
