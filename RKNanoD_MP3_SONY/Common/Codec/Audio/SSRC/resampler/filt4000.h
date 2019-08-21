//****************************************************************************
//
// FILT4000.H - Polyphase sample rate conversion filter with a ratio of 4.0.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
#define _ATTR_SRC_TABLE_11_TO_44     __attribute__((section("srctable11_44")))

_ATTR_SRC_TABLE_11_TO_44
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[4][13];
} SRCFilter_11025_44100 =
{
    0x00000004,
    0x00000001,
    0x0000000d,
    {
        #if 0
       { 0x00000040, 0x000007dc, 0x000001bf, 0xfffffbc2, 0x000008da, 0xffffebec, 0x000056f4, 0x00003841, 0xffffea7d, 0x00000c5a, 0xfffff7dd, 0x000006a0, 0x00000552 },
       { 0x0000013c, 0x00000991, 0xfffffc38, 0x000002a6, 0xffffff2e, 0xfffffaf2, 0x00006933, 0x00001672, 0xfffff303, 0x0000096d, 0xfffff841, 0x0000095a, 0x000002e2 },
       { 0x000002e2, 0x0000095a, 0xfffff841, 0x0000096d, 0xfffff303, 0x00001672, 0x00006933, 0xfffffaf2, 0xffffff2e, 0x000002a6, 0xfffffc38, 0x00000991, 0x0000013c },
       { 0x00000552, 0x000006a0, 0xfffff7dd, 0x00000c5a, 0xffffea7d, 0x00003841, 0x000056f4, 0xffffebec, 0x000008da, 0xfffffbc2, 0x000001bf, 0x000007dc, 0x00000040 },
#else
      {-18, -636, 1775, -1000, 3055, -6124, 23468, 11786, -2801, 919, 601, 794, -461 },
		{ -88, -572, 2308, -1842, 3507, -5502, 30988, 834, 1284, -1144, 1920, -92, -241 },
		{ -241, -92, 1920, -1144, 1284, 834, 30988, -5502, 3507, -1842, 2308, -572, -88 },
		{ -461, 794, 601, 919, -2801, 11786, 23468, -6124, 3055, -1000, 1775, -636, -18 },
#endif
    }
};
