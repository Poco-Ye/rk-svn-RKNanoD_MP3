//****************************************************************************
//
// FILT0005.H - Polyphase sample rate conversion filter with a ratio of 1.5.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************

#define _ATTR_SRC_TABLE_88_TO_44   __attribute__((section("srctable88_44")))

_ATTR_SRC_TABLE_88_TO_44
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[4][13];
} SRCFilter_0_5 =
{
    4,
    8,
    13,
    {
       {-12 , -44 , 778 , 911 , -2936 , 1926 , 17144 , 13987 , -963 , -2008 , 1277 , 456 , -62 },
        { -28 , 35 , 1097 , 172 , -3280 , 5726 , 18879 , 9959 , -2695 , -867 , 1304 , 199 , -49 },
        { -49 , 199 , 1304 , -867 , -2695 , 9959 , 18879 , 5726 , -3280 , 172 , 1097 , 35 , -28 },
        { -62 , 456 , 1277 , -2008 , -963 , 13987 , 17144 , 1926 , -2936 , 911 , 778 , -44 , -12 },
    }
};




