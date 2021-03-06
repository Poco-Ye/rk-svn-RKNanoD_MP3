//****************************************************************************
//
// FILT4354.H - Polyphase sample rate conversion filter with a ratio of
//              ~4.3537.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
static const struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[61][13];
} SRCFilter_4_3537 =
{
    61,
    14,
    13,
    {
        {  -340,  1704,  1087, -1679,  2906, -5712, 18850,
          18328, -5733,  2965, -1745,  1170,  1660 },
        {    75,  1747,  1003, -1609,  2840, -5677, 19363,
          17797, -5741,  3016, -1807,  1250,  1616 },
        {    70,  1790,   916, -1534,  2766, -5627, 19866,
          17258, -5735,  3059, -1864,  1329,  1572 },
        {    69,  1832,   829, -1455,  2684, -5563, 20358,
          16712, -5715,  3095, -1916,  1406,  1528 },
        {    69,  1874,   740, -1372,  2595, -5485, 20839,
          16159, -5684,  3123, -1963,  1481,  1484 },
        {    72,  1915,   650, -1285,  2498, -5391, 21308,
          15601, -5639,  3143, -2006,  1553,  1439 },
        {    76,  1956,   559, -1194,  2394, -5282, 21764,
          15038, -5583,  3156, -2043,  1623,  1395 },
        {    81,  1995,   466, -1099,  2282, -5158, 22206,
          14470, -5515,  3161, -2076,  1691,  1351 },
        {    88,  2034,   373, -1001,  2164, -5019, 22635,
          13899, -5436,  3159, -2104,  1756,  1307 },
        {    96,  2072,   280,  -899,  2038, -4864, 23049,
          13325, -5346,  3150, -2127,  1819,  1264 },
        {   105,  2108,   185,  -795,  1906, -4693, 23447,
          12749, -5245,  3133, -2145,  1879,  1220 },
        {   115,  2144,    91,  -687,  1767, -4507, 23830,
          12172, -5135,  3110, -2158,  1937,  1178 },
        {   126,  2178,    -4,  -577,  1621, -4305, 24197,
          11594, -5015,  3080, -2167,  1991,  1135 },
        {   138,  2211,   -99,  -464,  1469, -4087, 24547,
          11017, -4886,  3043, -2170,  2043,  1093 },
        {   151,  2243,  -194,  -349,  1312, -3854, 24880,
          10440, -4748,  2999, -2169,  2092,  1051 },
        {   165,  2273,  -288,  -232,  1148, -3605, 25195,
           9865, -4602,  2950, -2163,  2139,  1010 },
        {   180,  2301,  -382,  -112,   979, -3340, 25492,
           9292, -4448,  2894, -2152,  2182,   970 },
        {   196,  2328,  -476,     8,   805, -3060, 25770,
           8723, -4288,  2833, -2136,  2223,   930 },
        {   212,  2353,  -568,   131,   626, -2765, 26030,
           8157, -4120,  2765, -2116,  2261,   891 },
        {   230,  2376,  -660,   254,   443, -2455, 26270,
           7596, -3947,  2693, -2091,  2296,   853 },
        {   249,  2398,  -750,   378,   255, -2129, 26490,
           7039, -3768,  2615, -2062,  2328,   815 },
        {   268,  2417,  -840,   503,    63, -1789, 26690,
           6489, -3583,  2532, -2029,  2357,   779 },
        {   288,  2435,  -927,   628,  -132, -1434, 26870,
           5945, -3394,  2444, -1991,  2383,   743 },
        {   310,  2450, -1013,   753,  -331, -1065, 27030,
           5408, -3201,  2352, -1950,  2406,   707 },
        {   332,  2463, -1097,   878,  -532,  -681, 27168,
           4878, -3004,  2256, -1904,  2427,   673 },
        {   355,  2474, -1180,  1002,  -735,  -284, 27286,
           4357, -2804,  2156, -1855,  2445,   640 },
        {   380,  2482, -1260,  1126,  -941,   126, 27382,
           3845, -2601,  2053, -1802,  2460,   607 },
        {   405,  2488, -1337,  1249, -1148,   550, 27457,
           3342, -2396,  1945, -1745,  2472,   575 },
        {   431,  2492, -1412,  1370, -1356,   986, 27511,
           2849, -2190,  1835, -1685,  2481,   545 },
        {   458,  2493, -1485,  1489, -1564,  1434, 27543,
           2366, -1982,  1722, -1621,  2488,   515 },
        {   486,  2492, -1555,  1607, -1773,  1895, 27554,
           1895, -1773,  1607, -1555,  2492,   486 },
        {   515,  2488, -1621,  1722, -1982,  2366, 27543,
           1434, -1564,  1489, -1485,  2493,   458 },
        {   545,  2481, -1685,  1835, -2190,  2849, 27511,
            986, -1356,  1370, -1412,  2492,   431 },
        {   575,  2472, -1745,  1945, -2396,  3342, 27457,
            550, -1148,  1249, -1337,  2488,   405 },
        {   607,  2460, -1802,  2053, -2601,  3845, 27382,
            126,  -941,  1126, -1260,  2482,   380 },
        {   640,  2445, -1855,  2156, -2804,  4357, 27286,
           -284,  -735,  1002, -1180,  2474,   355 },
        {   673,  2427, -1904,  2256, -3004,  4878, 27168,
           -681,  -532,   878, -1097,  2463,   332 },
        {   707,  2406, -1950,  2352, -3201,  5408, 27030,
          -1065,  -331,   753, -1013,  2450,   310 },
        {   743,  2383, -1991,  2444, -3394,  5945, 26870,
          -1434,  -132,   628,  -927,  2435,   288 },
        {   779,  2357, -2029,  2532, -3583,  6489, 26690,
          -1789,    63,   503,  -840,  2417,   268 },
        {   815,  2328, -2062,  2615, -3768,  7039, 26490,
          -2129,   255,   378,  -750,  2398,   249 },
        {   853,  2296, -2091,  2693, -3947,  7596, 26270,
          -2455,   443,   254,  -660,  2376,   230 },
        {   891,  2261, -2116,  2765, -4120,  8157, 26030,
          -2765,   626,   131,  -568,  2353,   212 },
        {   930,  2223, -2136,  2833, -4288,  8723, 25770,
          -3060,   805,     8,  -476,  2328,   196 },
        {   970,  2182, -2152,  2894, -4448,  9292, 25492,
          -3340,   979,  -112,  -382,  2301,   180 },
        {  1010,  2139, -2163,  2950, -4602,  9865, 25195,
          -3605,  1148,  -232,  -288,  2273,   165 },
        {  1051,  2092, -2169,  2999, -4748, 10440, 24880,
          -3854,  1312,  -349,  -194,  2243,   151 },
        {  1093,  2043, -2170,  3043, -4886, 11017, 24547,
          -4087,  1469,  -464,   -99,  2211,   138 },
        {  1135,  1991, -2167,  3080, -5015, 11594, 24197,
          -4305,  1621,  -577,    -4,  2178,   126 },
        {  1178,  1937, -2158,  3110, -5135, 12172, 23830,
          -4507,  1767,  -687,    91,  2144,   115 },
        {  1220,  1879, -2145,  3133, -5245, 12749, 23447,
          -4693,  1906,  -795,   185,  2108,   105 },
        {  1264,  1819, -2127,  3150, -5346, 13325, 23049,
          -4864,  2038,  -899,   280,  2072,    96 },
        {  1307,  1756, -2104,  3159, -5436, 13899, 22635,
          -5019,  2164, -1001,   373,  2034,    88 },
        {  1351,  1691, -2076,  3161, -5515, 14470, 22206,
          -5158,  2282, -1099,   466,  1995,    81 },
        {  1395,  1623, -2043,  3156, -5583, 15038, 21764,
          -5282,  2394, -1194,   559,  1956,    76 },
        {  1439,  1553, -2006,  3143, -5639, 15601, 21308,
          -5391,  2498, -1285,   650,  1915,    72 },
        {  1484,  1481, -1963,  3123, -5684, 16159, 20839,
          -5485,  2595, -1372,   740,  1874,    69 },
        {  1528,  1406, -1916,  3095, -5715, 16712, 20358,
          -5563,  2684, -1455,   829,  1832,    69 },
        {  1572,  1329, -1864,  3059, -5735, 17258, 19866,
          -5627,  2766, -1534,   916,  1790,    70 },
        {  1616,  1250, -1807,  3016, -5741, 17797, 19363,
          -5677,  2840, -1609,  1003,  1747,    75 },
        {  1660,  1170, -1745,  2965, -5733, 18328, 18850,
          -5712,  2906, -1679,  1087,  1704,  -340 }
    }
};
