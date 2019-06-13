static h_unichar_t const binary_to_unicode[256] = {
/* 00 */	0x2400, 0x2401, 0x2402, 0x2403, 0x2404, 0x2405, 0x2406, 0x2407,
/* 08 */	0x2408, 0x2409, 0x240a, 0x240b, 0x240c, 0x240d, 0x240e, 0x240f,
/* 10 */	0x2410, 0x2411, 0x2412, 0x2413, 0x2414, 0x2415, 0x2416, 0x2417,
/* 18 */	0x2418, 0x2419, 0x241a, 0x241b, 0x241c, 0x241d, 0x241e, 0x241f,
/* 20 */	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
/* 28 */	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
/* 30 */	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
/* 38 */	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
/* 40 */	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
/* 48 */	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
/* 50 */	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
/* 58 */	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
/* 60 */	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
/* 68 */	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
/* 70 */	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
/* 78 */	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x2421,
/* 80 */	0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
/* 88 */	0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0164, 0x017d, 0x0179,
/* 90 */	0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
/* 98 */	0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x0165, 0x017e, 0x0178,
/* a0 */	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
/* a8 */	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
/* b0 */	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
/* b8 */	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
/* c0 */	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
/* c8 */	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
/* d0 */	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
/* d8 */	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
/* e0 */	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
/* e8 */	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
/* f0 */	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
/* f8 */	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

static h_unichar_t const binarytabs_to_unicode[256] = {
/* 00 */	0x2400, 0x2401, 0x2402, 0x2403, 0x2404, 0x2405, 0x2406, 0x2407,
/* 08 */	0x2408, 0x0009, 0x240a, 0x240b, 0x240c, 0x240d, 0x240e, 0x240f,
/* 10 */	0x2410, 0x2411, 0x2412, 0x2413, 0x2414, 0x2415, 0x2416, 0x2417,
/* 18 */	0x2418, 0x2419, 0x241a, 0x241b, 0x241c, 0x241d, 0x241e, 0x241f,
/* 20 */	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
/* 28 */	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
/* 30 */	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
/* 38 */	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
/* 40 */	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
/* 48 */	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
/* 50 */	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
/* 58 */	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
/* 60 */	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
/* 68 */	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
/* 70 */	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
/* 78 */	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x2421,
/* 80 */	0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
/* 88 */	0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0164, 0x017d, 0x0179,
/* 90 */	0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
/* 98 */	0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x0165, 0x017e, 0x0178,
/* a0 */	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
/* a8 */	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
/* b0 */	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
/* b8 */	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
/* c0 */	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
/* c8 */	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
/* d0 */	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
/* d8 */	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
/* e0 */	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
/* e8 */	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
/* f0 */	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
/* f8 */	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

static unsigned char const binary_maptab_00[256] = {
/* 00 */	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
/* 08 */	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
/* 10 */	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
/* 18 */	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
/* 20 */	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
/* 28 */	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
/* 30 */	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
/* 38 */	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
/* 40 */	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
/* 48 */	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
/* 50 */	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
/* 58 */	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
/* 60 */	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
/* 68 */	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
/* 70 */	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
/* 78 */	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
/* 80 */	0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0x90, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a0 */	0xa0, 0xa1, 0xa2, 0xa3, 0xde, 0xa5, 0xa6, 0xa7,
/* a8 */	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
/* b0 */	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
/* b8 */	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
/* c0 */	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
/* c8 */	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
/* d0 */	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
/* d8 */	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
/* e0 */	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
/* e8 */	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
/* f0 */	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
/* f8 */	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static unsigned char const binary_maptab_01[256] = {
/* 00 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 08 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 18 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 20 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 28 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 38 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 48 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 50 */	0xff, 0xff, 0x8c, 0x9c, 0xff, 0xff, 0xff, 0xff,
/* 58 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 60 */	0x8a, 0x9a, 0xff, 0xff, 0x8d, 0x9d, 0xff, 0xff,
/* 68 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 70 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 78 */	0x9f, 0x8f, 0x8f, 0xff, 0xff, 0x8e, 0x9e, 0xff,
/* 80 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0xff, 0x83, 0x83, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char const binary_maptab_02[256] = {
/* 00 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 08 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 18 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 20 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 28 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 38 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 48 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 50 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 58 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 60 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 68 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 70 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 78 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 80 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x88, 0xff,
/* c8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d8 */	0xff, 0xff, 0xff, 0xff, 0x98, 0xff, 0xff, 0xff,
/* e0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char const binary_maptab_03[256] = {
/* 00 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 08 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 18 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 20 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 28 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 38 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 48 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 50 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 58 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 60 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 68 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 70 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 78 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 80 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xb5, 0xff, 0xff, 0xff,
/* a0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b8 */	0xff, 0xff, 0xff, 0xff, 0xb5, 0xff, 0xff, 0xff,
/* c0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char const binary_maptab_20[256] = {
/* 00 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 08 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10 */	0xff, 0xff, 0xff, 0x96, 0x97, 0xff, 0xff, 0xff,
/* 18 */	0x91, 0x92, 0x82, 0xff, 0x93, 0x94, 0x84, 0xff,
/* 20 */	0x86, 0x87, 0x95, 0xff, 0xff, 0xff, 0x85, 0xff,
/* 28 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30 */	0x89, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 38 */	0xff, 0x8b, 0x9b, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 48 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 50 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 58 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 60 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 68 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 70 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 78 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 80 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a8 */	0xff, 0xff, 0xff, 0xff, 0xde, 0xff, 0xff, 0xff,
/* b0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char const binary_maptab_21[256] = {
/* 00 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 08 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 18 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 20 */	0xff, 0xff, 0x99, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 28 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 38 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 48 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 50 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 58 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 60 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 68 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 70 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 78 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 80 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char const binary_maptab_23[256] = {
/* 00 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 08 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf2,
/* 18 */	0xff, 0xff, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 20 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 28 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 38 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 48 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 50 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 58 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 60 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 68 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 70 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 78 */	0xff, 0xff, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 80 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char const binary_maptab_24[256] = {
/* 00 */	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
/* 08 */	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
/* 10 */	0x10, 0x11, 0x12, 0x13, 0x14, 0x14, 0x15, 0x16,
/* 18 */	0x18, 0x19, 0x1a, 0x1b, 0x1b, 0x1d, 0x1e, 0x1f,
/* 20 */	0x20, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 28 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 38 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 48 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 50 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 58 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 60 */	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
/* 68 */	0x19, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 70 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 78 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
/* 80 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 88 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 90 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 98 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* a8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* b8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* c8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* d8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* e8 */	0xff, 0xff, 0x10, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f0 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* f8 */	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const unsigned char (*const utf16_to_binary[256])[256] = {
	&binary_maptab_00, &binary_maptab_01, &binary_maptab_02, &binary_maptab_03, &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&binary_maptab_20, &binary_maptab_21, &maptab_ff,        &binary_maptab_23, &binary_maptab_24, &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,       
	&maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff,        &maptab_ff
};
