//****************************************************************************
//
// FILT1500.H - Polyphase sample rate conversion filter with a ratio of 1.5.
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
} SRCFilter_1_5 =
{
    0x00000003,
    0x00000002,
    0x0000000d,
    {
         { 0x00000063, 0x00000846, 0x000000cb, 0xfffffcc4, 0x0000079c, 0xffffed74, 0x00005b10, 0x0000328f, 0xffffeb5f, 0x00000c46, 0xfffff79e, 0x0000073f, 0x000004e8 },
         { 0x000001ff, 0x000009c4, 0xfffff9e0, 0x0000064c, 0xfffff90f, 0x0000076c, 0x00006b9c, 0x0000076c, 0xfffff90f, 0x0000064c, 0xfffff9e0, 0x000009c4, 0x000001ff },
        { 0x000004e8, 0x0000073f, 0xfffff79e, 0x00000c46, 0xffffeb5f, 0x0000328f, 0x00005b10, 0xffffed74, 0x0000079c, 0xfffffcc4, 0x000000cb, 0x00000846, 0x00000063 },

    }
};
