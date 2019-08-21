//****************************************************************************
//
// FILT0025.H - Polyphase sample rate conversion filter with a ratio of 1.5.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
#define _ATTR_SRC_TABLE_96_TO_44   __attribute__((section("srctable96_44")))

_ATTR_SRC_TABLE_96_TO_44
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[17][13];
} SRCFilter_0_45 =
{
    17,
    37,
    13,
    {
    {  2 , 164 , 852 , 112 , -2994 , 1845 , 15044 , 14442 , 1178 , -2896 , 259 , 814 , 140 },
{ 2 , 190 , 887 , -48 , -3062 , 2551 , 15589 , 13787 , 554 , -2771 , 392 , 772 , 118 },
{ 4 , 219 , 916 , -221 , -3097 , 3294 , 16071 , 13085 , -23 , -2623 , 511 , 727 , 99 },
{ 6 , 251 , 939 , -405 , -3095 , 4068 , 16486 , 12342 , -553 , -2456 , 615 , 681 , 83 },
{ 8 , 285 , 954 , -600 , -3053 , 4869 , 16831 , 11564 , -1033 , -2272 , 704 , 634 , 68 },
{ 12 , 322 , 961 , -802 , -2969 , 5692 , 17103 , 10758 , -1462 , -2075 , 780 , 586 , 55 },
{ 16 , 362 , 960 , -1012 , -2840 , 6532 , 17299 , 9930 , -1839 , -1869 , 841 , 539 , 45 },
{ 21 , 403 , 947 , -1226 , -2664 , 7381 , 17417 , 9087 , -2165 , -1657 , 889 , 492 , 35 },
{ 28 , 447 , 924 , -1442 , -2440 , 8235 , 17456 , 8235 , -2440 , -1442 , 924 , 447 , 28 },
{ 35 , 492 , 889 , -1657 , -2165 , 9087 , 17417 , 7381 , -2664 , -1226 , 947 , 403 , 21 },
{ 45 , 539 , 841 , -1869 , -1839 , 9930 , 17299 , 6532 , -2840 , -1012 , 960 , 362 , 16 },
{ 55 , 586 , 780 , -2075 , -1462 , 10758 , 17103 , 5692 , -2969 , -802 , 961 , 322 , 12 },
{ 68 , 634 , 704 , -2272 , -1033 , 11564 , 16831 , 4869 , -3053 , -600 , 954 , 285 , 8 },
{ 83 , 681 , 615 , -2456 , -553 , 12342 , 16486 , 4068 , -3095 , -405 , 939 , 251 , 6 },
{ 99 , 727 , 511 , -2623 , -23 , 13085 , 16071 , 3294 , -3097 , -221 , 916 , 219 , 4 },
{ 118 , 772 , 392 , -2771 , 554 , 13787 , 15589 , 2551 , -3062 , -48 , 887 , 190 , 2 },
{ 140 , 814 , 259 , -2896 , 1178 , 14442 , 15044 , 1845 , -2994 , 112 , 852 , 164 , 2 },
    }
};


