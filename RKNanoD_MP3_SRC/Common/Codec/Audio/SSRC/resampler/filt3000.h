//****************************************************************************
//
// FILT3000.H - Polyphase sample rate conversion filter with a ratio of 3.0.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[3][13];
} SRCFilter_3_0 =
{
    3,
    1,
    13,
    {
        {    99,  2118,   203,  -828,  1948, -4748, 23312,
          12943, -5281,  3142, -2146,  1855,  1256 },
        {   511,  2500, -1568,  1612, -1777,  1900, 27548,
           1900, -1777,  1612, -1568,  2500,   511 },
        {  1256,  1855, -2146,  3142, -5281, 12943, 23312,
          -4748,  1948,  -828,   203,  2118,    99 }
    }
};
