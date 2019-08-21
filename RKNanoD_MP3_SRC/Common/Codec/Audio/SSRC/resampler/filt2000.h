//****************************************************************************
//
// FILT2000.H - Polyphase sample rate conversion filter with a ratio of 2.0.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************

#define _ATTR_SRC_TABLE_22_TO_44     __attribute__((section("srctable22_44")))

_ATTR_SRC_TABLE_22_TO_44
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[2][13];
} SRCFilter_2_0 =
{
    2,
    1,
    13,
    {
        {   197,  2319,  -307,  -245,  1177, -3655, 25106,
          10020, -4650,  2977, -2186,  2124,  1092 },
        {  1092,  2124, -2186,  2977, -4650, 10020, 25106,
          -3655,  1177,  -245,  -307,  2319,   197 }
    }
};
