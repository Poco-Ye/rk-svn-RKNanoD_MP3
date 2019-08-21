//****************************************************************************
//
// FILT2756.H - Polyphase sample rate conversion filter with a ratio of
//              2.75625.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
#define _ATTR_SRC_TABLE_16_TO_44     __attribute__((section("srctable16_44")))

_ATTR_SRC_TABLE_16_TO_44
static  struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[69][13];
} SRCFilter_2_75625 =
{
    69,
    25,
    13,
    {
        {  -390,  1701,  1092, -1683,  2910, -5714, 18820,
          18359, -5732,  2962, -1742,  1165,  1663 },
        {    74,  1740,  1017, -1621,  2852, -5684, 19275,
          17890, -5740,  3008, -1796,  1237,  1624 },
        {    70,  1778,   942, -1556,  2788, -5643, 19722,
          17415, -5738,  3047, -1848,  1307,  1585 },
        {    68,  1815,   865, -1487,  2718, -5591, 20160,
          16934, -5725,  3081, -1895,  1375,  1546 },
        {    68,  1852,   787, -1416,  2642, -5527, 20590,
          16448, -5702,  3109, -1939,  1442,  1507 },
        {    69,  1889,   708, -1341,  2561, -5452, 21010,
          15958, -5669,  3131, -1979,  1507,  1468 },
        {    72,  1925,   628, -1263,  2473, -5365, 21421,
          15463, -5627,  3147, -2015,  1571,  1428 },
        {    76,  1961,   547, -1182,  2380, -5267, 21822,
          14964, -5575,  3157, -2048,  1632,  1389 },
        {    81,  1996,   465, -1098,  2281, -5156, 22212,
          14462, -5514,  3161, -2077,  1692,  1350 },
        {    87,  2030,   383, -1011,  2176, -5033, 22592,
          13957, -5444,  3159, -2102,  1750,  1312 },
        {    94,  2064,   300,  -922,  2066, -4899, 22960,
          13450, -5366,  3152, -2123,  1806,  1273 },
        {   101,  2096,   217,  -830,  1951, -4752, 23316,
          12941, -5280,  3139, -2140,  1859,  1235 },
        {   110,  2128,   133,  -736,  1830, -4592, 23660,
          12431, -5185,  3121, -2153,  1911,  1197 },
        {   119,  2159,    50,  -640,  1704, -4421, 23992,
          11921, -5084,  3097, -2163,  1961,  1159 },
        {   130,  2189,   -34,  -541,  1573, -4237, 24311,
          11410, -4974,  3069, -2168,  2008,  1121 },
        {   140,  2218,  -118,  -441,  1438, -4041, 24616,
          10900, -4858,  3034, -2170,  2054,  1084 },
        {   152,  2245,  -202,  -339,  1298, -3833, 24908,
          10390, -4735,  2995, -2168,  2097,  1048 },
        {   164,  2272,  -285,  -235,  1153, -3612, 25186,
           9882, -4606,  2951, -2163,  2138,  1012 },
        {   178,  2297,  -369,  -130,  1004, -3380, 25450,
           9375, -4471,  2902, -2153,  2176,   976 },
        {   191,  2321,  -451,   -23,   851, -3135, 25700,
           8871, -4330,  2849, -2141,  2213,   941 },
        {   206,  2344,  -533,    84,   694, -2878, 25934,
           8370, -4184,  2791, -2124,  2247,   906 },
        {   221,  2365,  -615,   193,   534, -2610, 26154,
           7872, -4033,  2729, -2104,  2279,   872 },
        {   237,  2385,  -695,   302,   370, -2329, 26358,
           7377, -3877,  2663, -2080,  2309,   838 },
        {   254,  2403,  -775,   412,   202, -2037, 26547,
           6887, -3717,  2592, -2054,  2336,   805 },
        {   271,  2420,  -854,   523,    32, -1733, 26720,
           6402, -3554,  2518, -2023,  2361,   773 },
        {   289,  2435,  -931,   633,  -141, -1418, 26878,
           5921, -3386,  2440, -1990,  2384,   741 },
        {   308,  2449, -1007,   744,  -316, -1092, 27019,
           5446, -3215,  2359, -1953,  2405,   710 },
        {   328,  2460, -1082,   854,  -494,  -755, 27144,
           4978, -3042,  2275, -1913,  2423,   679 },
        {   348,  2471, -1155,   964,  -673,  -407, 27252,
           4515, -2865,  2187, -1870,  2440,   650 },
        {   369,  2479, -1226,  1074,  -854,   -48, 27344,
           4059, -2687,  2096, -1824,  2454,   621 },
        {   391,  2485, -1296,  1183, -1036,   321, 27420,
           3610, -2506,  2003, -1776,  2465,   592 },
        {   413,  2490, -1364,  1291, -1220,   700, 27479,
           3169, -2325,  1907, -1724,  2475,   564 },
        {   437,  2492, -1429,  1398, -1404,  1089, 27521,
           2736, -2142,  1809, -1670,  2483,   537 },
        {   461,  2493, -1493,  1503, -1589,  1487, 27546,
           2311, -1958,  1709, -1614,  2488,   512 },
        {   486,  2492, -1554,  1607, -1773,  1895, 27554,
           1895, -1773,  1607, -1554,  2492,   486 },
        {   512,  2488, -1614,  1709, -1958,  2311, 27546,
           1487, -1589,  1503, -1493,  2493,   461 },
        {   537,  2483, -1670,  1809, -2142,  2736, 27521,
           1089, -1404,  1398, -1429,  2492,   437 },
        {   564,  2475, -1724,  1907, -2325,  3169, 27479,
            700, -1220,  1291, -1364,  2490,   413 },
        {   592,  2465, -1776,  2003, -2506,  3610, 27420,
            321, -1036,  1183, -1296,  2485,   391 },
        {   621,  2454, -1824,  2096, -2687,  4059, 27344,
            -48,  -854,  1074, -1226,  2479,   369 },
        {   650,  2440, -1870,  2187, -2865,  4515, 27252,
           -407,  -673,   964, -1155,  2471,   348 },
        {   679,  2423, -1913,  2275, -3042,  4978, 27144,
           -755,  -494,   854, -1082,  2460,   328 },
        {   710,  2405, -1953,  2359, -3215,  5446, 27019,
          -1092,  -316,   744, -1007,  2449,   308 },
        {   741,  2384, -1990,  2440, -3386,  5921, 26878,
          -1418,  -141,   633,  -931,  2435,   289 },
        {   773,  2361, -2023,  2518, -3554,  6402, 26720,
          -1733,    32,   523,  -854,  2420,   271 },
        {   805,  2336, -2054,  2592, -3717,  6887, 26547,
          -2037,   202,   412,  -775,  2403,   254 },
        {   838,  2309, -2080,  2663, -3877,  7377, 26358,
          -2329,   370,   302,  -695,  2385,   237 },
        {   872,  2279, -2104,  2729, -4033,  7872, 26154,
          -2610,   534,   193,  -615,  2365,   221 },
        {   906,  2247, -2124,  2791, -4184,  8370, 25934,
          -2878,   694,    84,  -533,  2344,   206 },
        {   941,  2213, -2141,  2849, -4330,  8871, 25700,
          -3135,   851,   -23,  -451,  2321,   191 },
        {   976,  2176, -2153,  2902, -4471,  9375, 25450,
          -3380,  1004,  -130,  -369,  2297,   178 },
        {  1012,  2138, -2163,  2951, -4606,  9882, 25186,
          -3612,  1153,  -235,  -285,  2272,   164 },
        {  1048,  2097, -2168,  2995, -4735, 10390, 24908,
          -3833,  1298,  -339,  -202,  2245,   152 },
        {  1084,  2054, -2170,  3034, -4858, 10900, 24616,
          -4041,  1438,  -441,  -118,  2218,   140 },
        {  1121,  2008, -2168,  3069, -4974, 11410, 24311,
          -4237,  1573,  -541,   -34,  2189,   130 },
        {  1159,  1961, -2163,  3097, -5084, 11921, 23992,
          -4421,  1704,  -640,    50,  2159,   119 },
        {  1197,  1911, -2153,  3121, -5185, 12431, 23660,
          -4592,  1830,  -736,   133,  2128,   110 },
        {  1235,  1859, -2140,  3139, -5280, 12941, 23316,
          -4752,  1951,  -830,   217,  2096,   101 },
        {  1273,  1806, -2123,  3152, -5366, 13450, 22960,
          -4899,  2066,  -922,   300,  2064,    94 },
        {  1312,  1750, -2102,  3159, -5444, 13957, 22592,
          -5033,  2176, -1011,   383,  2030,    87 },
        {  1350,  1692, -2077,  3161, -5514, 14462, 22212,
          -5156,  2281, -1098,   465,  1996,    81 },
        {  1389,  1632, -2048,  3157, -5575, 14964, 21822,
          -5267,  2380, -1182,   547,  1961,    76 },
        {  1428,  1571, -2015,  3147, -5627, 15463, 21421,
          -5365,  2473, -1263,   628,  1925,    72 },
        {  1468,  1507, -1979,  3131, -5669, 15958, 21010,
          -5452,  2561, -1341,   708,  1889,    69 },
        {  1507,  1442, -1939,  3109, -5702, 16448, 20590,
          -5527,  2642, -1416,   787,  1852,    68 },
        {  1546,  1375, -1895,  3081, -5725, 16934, 20160,
          -5591,  2718, -1487,   865,  1815,    68 },
        {  1585,  1307, -1848,  3047, -5738, 17415, 19722,
          -5643,  2788, -1556,   942,  1778,    70 },
        {  1624,  1237, -1796,  3008, -5740, 17890, 19275,
          -5684,  2852, -1621,  1017,  1740,    74 },
        {  1663,  1165, -1742,  2962, -5732, 18359, 18820,
          -5714,  2910, -1683,  1092,  1701,  -390 }
    }
};