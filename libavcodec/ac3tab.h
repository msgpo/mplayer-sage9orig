/*
 * AC3 tables
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file ac3tab.h
 * tables taken directly from AC3 spec.
 */

/**
 * Possible frame sizes.
 * from ATSC A/52 Table 5.18 Frame Size Code Table.
 */
const uint16_t ff_ac3_frame_sizes[38][3] = {
    { 64,   69,   96   },
    { 64,   70,   96   },
    { 80,   87,   120  },
    { 80,   88,   120  },
    { 96,   104,  144  },
    { 96,   105,  144  },
    { 112,  121,  168  },
    { 112,  122,  168  },
    { 128,  139,  192  },
    { 128,  140,  192  },
    { 160,  174,  240  },
    { 160,  175,  240  },
    { 192,  208,  288  },
    { 192,  209,  288  },
    { 224,  243,  336  },
    { 224,  244,  336  },
    { 256,  278,  384  },
    { 256,  279,  384  },
    { 320,  348,  480  },
    { 320,  349,  480  },
    { 384,  417,  576  },
    { 384,  418,  576  },
    { 448,  487,  672  },
    { 448,  488,  672  },
    { 512,  557,  768  },
    { 512,  558,  768  },
    { 640,  696,  960  },
    { 640,  697,  960  },
    { 768,  835,  1152 },
    { 768,  836,  1152 },
    { 896,  975,  1344 },
    { 896,  976,  1344 },
    { 1024, 1114, 1536 },
    { 1024, 1115, 1536 },
    { 1152, 1253, 1728 },
    { 1152, 1254, 1728 },
    { 1280, 1393, 1920 },
    { 1280, 1394, 1920 },
};

/**
 * Maps audio coding mode (acmod) to number of full-bandwidth channels.
 * from ATSC A/52 Table 5.8 Audio Coding Mode
 */
const uint8_t ff_ac3_channels[8] = {
    2, 1, 2, 3, 3, 4, 4, 5
};

/* possible frequencies */
const uint16_t ff_ac3_freqs[3] = { 48000, 44100, 32000 };

/* possible bitrates */
const uint16_t ff_ac3_bitratetab[19] = {
    32, 40, 48, 56, 64, 80, 96, 112, 128,
    160, 192, 224, 256, 320, 384, 448, 512, 576, 640
};

/* AC3 MDCT window */

/* MDCT window */
const int16_t ff_ac3_window[256] = {
    4,    7,   12,   16,   21,   28,   34,   42,
   51,   61,   72,   84,   97,  111,  127,  145,
  164,  184,  207,  231,  257,  285,  315,  347,
  382,  419,  458,  500,  544,  591,  641,  694,
  750,  810,  872,  937, 1007, 1079, 1155, 1235,
 1318, 1406, 1497, 1593, 1692, 1796, 1903, 2016,
 2132, 2253, 2379, 2509, 2644, 2783, 2927, 3076,
 3230, 3389, 3552, 3721, 3894, 4072, 4255, 4444,
 4637, 4835, 5038, 5246, 5459, 5677, 5899, 6127,
 6359, 6596, 6837, 7083, 7334, 7589, 7848, 8112,
 8380, 8652, 8927, 9207, 9491, 9778,10069,10363,
10660,10960,11264,11570,11879,12190,12504,12820,
13138,13458,13780,14103,14427,14753,15079,15407,
15735,16063,16392,16720,17049,17377,17705,18032,
18358,18683,19007,19330,19651,19970,20287,20602,
20914,21225,21532,21837,22139,22438,22733,23025,
23314,23599,23880,24157,24430,24699,24964,25225,
25481,25732,25979,26221,26459,26691,26919,27142,
27359,27572,27780,27983,28180,28373,28560,28742,
28919,29091,29258,29420,29577,29729,29876,30018,
30155,30288,30415,30538,30657,30771,30880,30985,
31086,31182,31274,31363,31447,31528,31605,31678,
31747,31814,31877,31936,31993,32046,32097,32145,
32190,32232,32272,32310,32345,32378,32409,32438,
32465,32490,32513,32535,32556,32574,32592,32608,
32623,32636,32649,32661,32671,32681,32690,32698,
32705,32712,32718,32724,32729,32733,32737,32741,
32744,32747,32750,32752,32754,32756,32757,32759,
32760,32761,32762,32763,32764,32764,32765,32765,
32766,32766,32766,32766,32767,32767,32767,32767,
32767,32767,32767,32767,32767,32767,32767,32767,
32767,32767,32767,32767,32767,32767,32767,32767,
};

static uint8_t masktab[253];

static const uint8_t latab[260]= {
0x0040,0x003f,0x003e,0x003d,0x003c,0x003b,0x003a,0x0039,0x0038,0x0037,
0x0036,0x0035,0x0034,0x0034,0x0033,0x0032,0x0031,0x0030,0x002f,0x002f,
0x002e,0x002d,0x002c,0x002c,0x002b,0x002a,0x0029,0x0029,0x0028,0x0027,
0x0026,0x0026,0x0025,0x0024,0x0024,0x0023,0x0023,0x0022,0x0021,0x0021,
0x0020,0x0020,0x001f,0x001e,0x001e,0x001d,0x001d,0x001c,0x001c,0x001b,
0x001b,0x001a,0x001a,0x0019,0x0019,0x0018,0x0018,0x0017,0x0017,0x0016,
0x0016,0x0015,0x0015,0x0015,0x0014,0x0014,0x0013,0x0013,0x0013,0x0012,
0x0012,0x0012,0x0011,0x0011,0x0011,0x0010,0x0010,0x0010,0x000f,0x000f,
0x000f,0x000e,0x000e,0x000e,0x000d,0x000d,0x000d,0x000d,0x000c,0x000c,
0x000c,0x000c,0x000b,0x000b,0x000b,0x000b,0x000a,0x000a,0x000a,0x000a,
0x000a,0x0009,0x0009,0x0009,0x0009,0x0009,0x0008,0x0008,0x0008,0x0008,
0x0008,0x0008,0x0007,0x0007,0x0007,0x0007,0x0007,0x0007,0x0006,0x0006,
0x0006,0x0006,0x0006,0x0006,0x0006,0x0006,0x0005,0x0005,0x0005,0x0005,
0x0005,0x0005,0x0005,0x0005,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
0x0004,0x0004,0x0004,0x0004,0x0004,0x0003,0x0003,0x0003,0x0003,0x0003,
0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0002,
0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0001,0x0001,
0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

static const uint16_t hth[50][3]= {
{ 0x04d0,0x04f0,0x0580 },
{ 0x04d0,0x04f0,0x0580 },
{ 0x0440,0x0460,0x04b0 },
{ 0x0400,0x0410,0x0450 },
{ 0x03e0,0x03e0,0x0420 },
{ 0x03c0,0x03d0,0x03f0 },
{ 0x03b0,0x03c0,0x03e0 },
{ 0x03b0,0x03b0,0x03d0 },
{ 0x03a0,0x03b0,0x03c0 },
{ 0x03a0,0x03a0,0x03b0 },
{ 0x03a0,0x03a0,0x03b0 },
{ 0x03a0,0x03a0,0x03b0 },
{ 0x03a0,0x03a0,0x03a0 },
{ 0x0390,0x03a0,0x03a0 },
{ 0x0390,0x0390,0x03a0 },
{ 0x0390,0x0390,0x03a0 },
{ 0x0380,0x0390,0x03a0 },
{ 0x0380,0x0380,0x03a0 },
{ 0x0370,0x0380,0x03a0 },
{ 0x0370,0x0380,0x03a0 },
{ 0x0360,0x0370,0x0390 },
{ 0x0360,0x0370,0x0390 },
{ 0x0350,0x0360,0x0390 },
{ 0x0350,0x0360,0x0390 },
{ 0x0340,0x0350,0x0380 },
{ 0x0340,0x0350,0x0380 },
{ 0x0330,0x0340,0x0380 },
{ 0x0320,0x0340,0x0370 },
{ 0x0310,0x0320,0x0360 },
{ 0x0300,0x0310,0x0350 },
{ 0x02f0,0x0300,0x0340 },
{ 0x02f0,0x02f0,0x0330 },
{ 0x02f0,0x02f0,0x0320 },
{ 0x02f0,0x02f0,0x0310 },
{ 0x0300,0x02f0,0x0300 },
{ 0x0310,0x0300,0x02f0 },
{ 0x0340,0x0320,0x02f0 },
{ 0x0390,0x0350,0x02f0 },
{ 0x03e0,0x0390,0x0300 },
{ 0x0420,0x03e0,0x0310 },
{ 0x0460,0x0420,0x0330 },
{ 0x0490,0x0450,0x0350 },
{ 0x04a0,0x04a0,0x03c0 },
{ 0x0460,0x0490,0x0410 },
{ 0x0440,0x0460,0x0470 },
{ 0x0440,0x0440,0x04a0 },
{ 0x0520,0x0480,0x0460 },
{ 0x0800,0x0630,0x0440 },
{ 0x0840,0x0840,0x0450 },
{ 0x0840,0x0840,0x04e0 },
};

static const uint8_t baptab[64]= {
    0, 1, 1, 1, 1, 1, 2, 2, 3, 3,
    3, 4, 4, 5, 5, 6, 6, 6, 6, 7,
    7, 7, 7, 8, 8, 8, 8, 9, 9, 9,
    9, 10, 10, 10, 10, 11, 11, 11, 11, 12,
    12, 12, 12, 13, 13, 13, 13, 14, 14, 14,
    14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
    15, 15, 15, 15,
};

const uint8_t ff_sdecaytab[4]={
    0x0f, 0x11, 0x13, 0x15,
};

const uint8_t ff_fdecaytab[4]={
    0x3f, 0x53, 0x67, 0x7b,
};

const uint16_t ff_sgaintab[4]= {
    0x540, 0x4d8, 0x478, 0x410,
};

const uint16_t ff_dbkneetab[4]= {
    0x000, 0x700, 0x900, 0xb00,
};

const int16_t ff_floortab[8]= {
    0x2f0, 0x2b0, 0x270, 0x230, 0x1f0, 0x170, 0x0f0, 0xf800,
};

const uint16_t ff_fgaintab[8]= {
    0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380, 0x400,
};

static const uint8_t bndsz[50]={
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3,
    3, 6, 6, 6, 6, 6, 6, 12, 12, 12, 12, 24, 24, 24, 24, 24
};

static uint8_t bndtab[51];
