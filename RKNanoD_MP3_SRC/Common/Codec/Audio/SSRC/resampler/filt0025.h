//****************************************************************************
//
// FILT0025.H - Polyphase sample rate conversion filter with a ratio of 1.5.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
#define _ATTR_SRC_TABLE_176_TO_44   __attribute__((section("srctable176_44")))

_ATTR_SRC_TABLE_176_TO_44
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[4][13];
} SRCFilter_0_25 =
{
    4,
    16,
    13,
    {
      {1 , 86 , 253 , -723 , -1742 , 4365 , 14320 , 12420 , 1966 , -1858 , -306 , 248 , 46 },
        { 7 , 141 , 181 , -1196 , -1121 , 7108 , 15343 , 9906 , 108 , -1622 , 0 , 201 , 20 },
        { 20 , 201 , 0 , -1622 , 108 , 9906 , 15343 , 7108 , -1121 , -1196 , 181 , 141 , 7 },
        { 46 , 248 , -306 , -1858 , 1966 , 12420 , 14320 , 4365 , -1742 , -723 , 253 , 86 , 1 },

    }
};


