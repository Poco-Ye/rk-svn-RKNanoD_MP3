//****************************************************************************
//
// FILT6000.H - Polyphase sample rate conversion filter with a ratio of 6.0.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[6][13];
} SRCFilter_6_0 =
{
    0x00000006,
    0x00000001,
    0x0000000d,
    {
       { 0x0000001e, 0x00000772, 0x000002ab, 0xfffffad7, 0x000009ea, 0xffffeac9, 0x0000527a, 0x00003ddb, 0xffffe9e4, 0x00000c3f, 0xfffff83a, 0x000005f1, 0x000005ba },
       { 0x000000b5, 0x000008df, 0xfffffef2, 0xfffffefc, 0x000004a4, 0xfffff1ad, 0x0000621e, 0x0000271a, 0xffffede0, 0x00000b94, 0xfffff789, 0x0000084f, 0x00000405 },
       { 0x00000164, 0x000009ad, 0xfffffb66, 0x000003e2, 0xfffffd30, 0xfffffec4, 0x00006a8c, 0x00001132, 0xfffff4f9, 0x00000876, 0xfffff8b9, 0x0000098d, 0x0000028b },
       { 0x0000028b, 0x0000098d, 0xfffff8b9, 0x00000876, 0xfffff4f9, 0x00001132, 0x00006a8c, 0xfffffec4, 0xfffffd30, 0x000003e2, 0xfffffb66, 0x000009ad, 0x00000164 },
       { 0x00000405, 0x0000084f, 0xfffff789, 0x00000b94, 0xffffede0, 0x0000271a, 0x0000621e, 0xfffff1ad, 0x000004a4, 0xfffffefc, 0xfffffef2, 0x000008df, 0x000000b5 },
       { 0x000005ba, 0x000005f1, 0xfffff83a, 0x00000c3f, 0xffffe9e4, 0x00003ddb, 0x0000527a, 0xffffeac9, 0x000009ea, 0xfffffad7, 0x000002ab, 0x00000772, 0x0000001e },

    }
};