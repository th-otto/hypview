#include "hv_defs.h"
#include "picture.h"
#include "w_draw.h"

static _WORD screen_w, screen_h;
static _WORD screen_colors;
static _WORD screen_planes;

static _WORD const pix2col_8[256] = {
   2,	3,	 6,   5,   4,	8,	 7,   9,  10,  11,	14,  13,  12, 255,	15,   1,
  16,  17,	18,  19,  20,  21,	22,  23,  24,  25,	26,  27,  28,  29,	30,  31,
  32,  33,	34,  35,  36,  37,	38,  39,  40,  41,	42,  43,  44,  45,	46,  47,
  48,  49,	50,  51,  52,  53,	54,  55,  56,  57,	58,  59,  60,  61,	62,  63,
  64,  65,	66,  67,  68,  69,	70,  71,  72,  73,	74,  75,  76,  77,	78,  79,
  80,  81,	82,  83,  84,  85,	86,  87,  88,  89,	90,  91,  92,  93,	94,  95,
  96,  97,	98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,   0
};
static _WORD const pix2col_4[16] = {  1, 10, 11, 14, 12, 15, 13,  9,  8,  2,  3,  6,  4,  7,  5,  0 };
static _WORD const pix2col_2[4]  = { 1, 2, 3, 0 };

/*
 * mappings of atari pixel values to windows palette indices
 */
static _WORD const coltab4[4]    = { 3, 1, 2, 0 };
static _WORD const coltab16[16]  = { 15,  9, 10, 11, 12, 13, 14,  8,  7,  1,  2,  3,  4,  5,  6,  0 };

static _WORD const coltab256[256] = {
 255,	0,	 1,   2,   4,	6,	 3,   5,   7,	8,	 9,  10,  12,  14,	11,  13,
  16,  17,	18,  19,  20,  21,	22,  23,  24,  25,	26,  27,  28,  29,	30,  31,
  32,  33,	34,  35,  36,  37,	38,  39,  40,  41,	42,  43,  44,  45,	46,  47,
  48,  49,	50,  51,  52,  53,	54,  55,  56,  57,	58,  59,  60,  61,	62,  63,
  64,  65,	66,  67,  68,  69,	70,  71,  72,  73,	74,  75,  76,  77,	78,  79,
  80,  81,	82,  83,  84,  85,	86,  87,  88,  89,	90,  91,  92,  93,	94,  95,
  96,  97,	98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,  15
};

static COLORREF const std_palette[256] = {
	 /* white		  */ PALETTERGB(255, 255, 255),
	 /* black		  */ PALETTERGB(0, 0, 0),
	 /* red           */ PALETTERGB(255, 0, 0),
	 /* green		  */ PALETTERGB(0, 255, 0),
	 /* blue		  */ PALETTERGB(0, 0, 255),
	 /* cyan		  */ PALETTERGB(0, 255, 255),
	 /* yellow		  */ PALETTERGB(255, 255, 0),
	 /* magenta       */ PALETTERGB(255, 0, 255),
	 /* light gray	  */ PALETTERGB(204, 204, 204),
	 /* dark gray	  */ PALETTERGB(136, 136, 136),
	 /* dark red	  */ PALETTERGB(136, 0, 0),
	 /* dark green    */ PALETTERGB(0, 136, 0),
	 /* dark blue	  */ PALETTERGB(0, 0, 136),
	 /* dark cyan	  */ PALETTERGB(0, 136, 136),
	 /* dark yellow   */ PALETTERGB(136, 136, 0),
	 /* dark magenta  */ PALETTERGB(136, 0, 136),
	PALETTERGB(0xfd, 0xfd, 0xfd), /*  16 == 16 */
	PALETTERGB(0xec, 0xec, 0xec), /*  17 == 17 */
	PALETTERGB(0xdb, 0xdb, 0xdb), /*  18 == 18 */
	PALETTERGB(0xca, 0xca, 0xca), /*  19 == 19 */
	PALETTERGB(0xb9, 0xb9, 0xb9), /*  20 == 20 */
	PALETTERGB(0xa8, 0xa8, 0xa8), /*  21 == 21 */
	PALETTERGB(0x97, 0x97, 0x97), /*  22 == 22 */
	PALETTERGB(0x87, 0x87, 0x87), /*  23 == 23 */
	PALETTERGB(0x76, 0x76, 0x76), /*  24 == 24 */
	PALETTERGB(0x65, 0x65, 0x65), /*  25 == 25 */
	PALETTERGB(0x54, 0x54, 0x54), /*  26 == 26 */
	PALETTERGB(0x43, 0x43, 0x43), /*  27 == 27 */
	PALETTERGB(0x32, 0x32, 0x32), /*  28 == 28 */
	PALETTERGB(0x21, 0x21, 0x21), /*  29 == 29 */
	PALETTERGB(0x10, 0x10, 0x10), /*  30 == 30 */
	PALETTERGB(0x00, 0x00, 0x00), /*  31 == 31 */
	PALETTERGB(0xfd, 0x00, 0x00), /*  32 == 32 */
	PALETTERGB(0xfd, 0x00, 0x10), /*  33 == 33 */
	PALETTERGB(0xfd, 0x00, 0x21), /*  34 == 34 */
	PALETTERGB(0xfd, 0x00, 0x32), /*  35 == 35 */
	PALETTERGB(0xfd, 0x00, 0x43), /*  36 == 36 */
	PALETTERGB(0xfd, 0x00, 0x54), /*  37 == 37 */
	PALETTERGB(0xfd, 0x00, 0x65), /*  38 == 38 */
	PALETTERGB(0xfd, 0x00, 0x76), /*  39 == 39 */
	PALETTERGB(0xfd, 0x00, 0x87), /*  40 == 40 */
	PALETTERGB(0xfd, 0x00, 0x97), /*  41 == 41 */
	PALETTERGB(0xfd, 0x00, 0xa8), /*  42 == 42 */
	PALETTERGB(0xfd, 0x00, 0xb9), /*  43 == 43 */
	PALETTERGB(0xfd, 0x00, 0xca), /*  44 == 44 */
	PALETTERGB(0xfd, 0x00, 0xdb), /*  45 == 45 */
	PALETTERGB(0xfd, 0x00, 0xec), /*  46 == 46 */
	PALETTERGB(0xfd, 0x00, 0xfd), /*  47 == 47 */
	PALETTERGB(0xec, 0x00, 0xfd), /*  48 == 48 */
	PALETTERGB(0xdb, 0x00, 0xfd), /*  49 == 49 */
	PALETTERGB(0xca, 0x00, 0xfd), /*  50 == 50 */
	PALETTERGB(0xb9, 0x00, 0xfd), /*  51 == 51 */
	PALETTERGB(0xa8, 0x00, 0xfd), /*  52 == 52 */
	PALETTERGB(0x97, 0x00, 0xfd), /*  53 == 53 */
	PALETTERGB(0x87, 0x00, 0xfd), /*  54 == 54 */
	PALETTERGB(0x76, 0x00, 0xfd), /*  55 == 55 */
	PALETTERGB(0x65, 0x00, 0xfd), /*  56 == 56 */
	PALETTERGB(0x54, 0x00, 0xfd), /*  57 == 57 */
	PALETTERGB(0x43, 0x00, 0xfd), /*  58 == 58 */
	PALETTERGB(0x32, 0x00, 0xfd), /*  59 == 59 */
	PALETTERGB(0x21, 0x00, 0xfd), /*  60 == 60 */
	PALETTERGB(0x10, 0x00, 0xfd), /*  61 == 61 */
	PALETTERGB(0x00, 0x00, 0xfd), /*  62 == 62 */
	PALETTERGB(0x00, 0x10, 0xfd), /*  63 == 63 */
	PALETTERGB(0x00, 0x21, 0xfd), /*  64 == 64 */
	PALETTERGB(0x00, 0x32, 0xfd), /*  65 == 65 */
	PALETTERGB(0x00, 0x43, 0xfd), /*  66 == 66 */
	PALETTERGB(0x00, 0x54, 0xfd), /*  67 == 67 */
	PALETTERGB(0x00, 0x65, 0xfd), /*  68 == 68 */
	PALETTERGB(0x00, 0x76, 0xfd), /*  69 == 69 */
	PALETTERGB(0x00, 0x87, 0xfd), /*  70 == 70 */
	PALETTERGB(0x00, 0x97, 0xfd), /*  71 == 71 */
	PALETTERGB(0x00, 0xa8, 0xfd), /*  72 == 72 */
	PALETTERGB(0x00, 0xb9, 0xfd), /*  73 == 73 */
	PALETTERGB(0x00, 0xca, 0xfd), /*  74 == 74 */
	PALETTERGB(0x00, 0xdb, 0xfd), /*  75 == 75 */
	PALETTERGB(0x00, 0xec, 0xfd), /*  76 == 76 */
	PALETTERGB(0x00, 0xfd, 0xfd), /*  77 == 77 */
	PALETTERGB(0x00, 0xfd, 0xec), /*  78 == 78 */
	PALETTERGB(0x00, 0xfd, 0xdb), /*  79 == 79 */
	PALETTERGB(0x00, 0xfd, 0xca), /*  80 == 80 */
	PALETTERGB(0x00, 0xfd, 0xb9), /*  81 == 81 */
	PALETTERGB(0x00, 0xfd, 0xa8), /*  82 == 82 */
	PALETTERGB(0x00, 0xfd, 0x97), /*  83 == 83 */
	PALETTERGB(0x00, 0xfd, 0x87), /*  84 == 84 */
	PALETTERGB(0x00, 0xfd, 0x76), /*  85 == 85 */
	PALETTERGB(0x00, 0xfd, 0x65), /*  86 == 86 */
	PALETTERGB(0x00, 0xfd, 0x54), /*  87 == 87 */
	PALETTERGB(0x00, 0xfd, 0x43), /*  88 == 88 */
	PALETTERGB(0x00, 0xfd, 0x32), /*  89 == 89 */
	PALETTERGB(0x00, 0xfd, 0x21), /*  90 == 90 */
	PALETTERGB(0x00, 0xfd, 0x10), /*  91 == 91 */
	PALETTERGB(0x00, 0xfd, 0x00), /*  92 == 92 */
	PALETTERGB(0x10, 0xfd, 0x00), /*  93 == 93 */
	PALETTERGB(0x21, 0xfd, 0x00), /*  94 == 94 */
	PALETTERGB(0x32, 0xfd, 0x00), /*  95 == 95 */
	PALETTERGB(0x43, 0xfd, 0x00), /*  96 == 96 */
	PALETTERGB(0x54, 0xfd, 0x00), /*  97 == 97 */
	PALETTERGB(0x65, 0xfd, 0x00), /*  98 == 98 */
	PALETTERGB(0x76, 0xfd, 0x00), /*  99 == 99 */
	PALETTERGB(0x87, 0xfd, 0x00), /* 100 == 100 */
	PALETTERGB(0x97, 0xfd, 0x00), /* 101 == 101 */
	PALETTERGB(0xa8, 0xfd, 0x00), /* 102 == 102 */
	PALETTERGB(0xb9, 0xfd, 0x00), /* 103 == 103 */
	PALETTERGB(0xca, 0xfd, 0x00), /* 104 == 104 */
	PALETTERGB(0xdb, 0xfd, 0x00), /* 105 == 105 */
	PALETTERGB(0xec, 0xfd, 0x00), /* 106 == 106 */
	PALETTERGB(0xfd, 0xfd, 0x00), /* 107 == 107 */
	PALETTERGB(0xfd, 0xec, 0x00), /* 108 == 108 */
	PALETTERGB(0xfd, 0xdb, 0x00), /* 109 == 109 */
	PALETTERGB(0xfd, 0xca, 0x00), /* 110 == 110 */
	PALETTERGB(0xfd, 0xb9, 0x00), /* 111 == 111 */
	PALETTERGB(0xfd, 0xa8, 0x00), /* 112 == 112 */
	PALETTERGB(0xfd, 0x97, 0x00), /* 113 == 113 */
	PALETTERGB(0xfd, 0x87, 0x00), /* 114 == 114 */
	PALETTERGB(0xfd, 0x76, 0x00), /* 115 == 115 */
	PALETTERGB(0xfd, 0x65, 0x00), /* 116 == 116 */
	PALETTERGB(0xfd, 0x54, 0x00), /* 117 == 117 */
	PALETTERGB(0xfd, 0x43, 0x00), /* 118 == 118 */
	PALETTERGB(0xfd, 0x32, 0x00), /* 119 == 119 */
	PALETTERGB(0xfd, 0x21, 0x00), /* 120 == 120 */
	PALETTERGB(0xfd, 0x10, 0x00), /* 121 == 121 */
	PALETTERGB(0xb9, 0x00, 0x00), /* 122 == 122 */
	PALETTERGB(0xb9, 0x00, 0x10), /* 123 == 123 */
	PALETTERGB(0xb9, 0x00, 0x21), /* 124 == 124 */
	PALETTERGB(0xb9, 0x00, 0x32), /* 125 == 125 */
	PALETTERGB(0xb9, 0x00, 0x43), /* 126 == 126 */
	PALETTERGB(0xb9, 0x00, 0x54), /* 127 == 127 */
	PALETTERGB(0xb9, 0x00, 0x65), /* 128 == 128 */
	PALETTERGB(0xb9, 0x00, 0x76), /* 129 == 129 */
	PALETTERGB(0xb9, 0x00, 0x87), /* 130 == 130 */
	PALETTERGB(0xb9, 0x00, 0x97), /* 131 == 131 */
	PALETTERGB(0xb9, 0x00, 0xa8), /* 132 == 132 */
	PALETTERGB(0xb9, 0x00, 0xb9), /* 133 == 133 */
	PALETTERGB(0xa8, 0x00, 0xb9), /* 134 == 134 */
	PALETTERGB(0x97, 0x00, 0xb9), /* 135 == 135 */
	PALETTERGB(0x87, 0x00, 0xb9), /* 136 == 136 */
	PALETTERGB(0x76, 0x00, 0xb9), /* 137 == 137 */
	PALETTERGB(0x65, 0x00, 0xb9), /* 138 == 138 */
	PALETTERGB(0x54, 0x00, 0xb9), /* 139 == 139 */
	PALETTERGB(0x43, 0x00, 0xb9), /* 140 == 140 */
	PALETTERGB(0x32, 0x00, 0xb9), /* 141 == 141 */
	PALETTERGB(0x21, 0x00, 0xb9), /* 142 == 142 */
	PALETTERGB(0x10, 0x00, 0xb9), /* 143 == 143 */
	PALETTERGB(0x00, 0x00, 0xb9), /* 144 == 144 */
	PALETTERGB(0x00, 0x10, 0xb9), /* 145 == 145 */
	PALETTERGB(0x00, 0x21, 0xb9), /* 146 == 146 */
	PALETTERGB(0x00, 0x32, 0xb9), /* 147 == 147 */
	PALETTERGB(0x00, 0x43, 0xb9), /* 148 == 148 */
	PALETTERGB(0x00, 0x54, 0xb9), /* 149 == 149 */
	PALETTERGB(0x00, 0x65, 0xb9), /* 150 == 150 */
	PALETTERGB(0x00, 0x76, 0xb9), /* 151 == 151 */
	PALETTERGB(0x00, 0x87, 0xb9), /* 152 == 152 */
	PALETTERGB(0x00, 0x97, 0xb9), /* 153 == 153 */
	PALETTERGB(0x00, 0xa8, 0xb9), /* 154 == 154 */
	PALETTERGB(0x00, 0xb9, 0xb9), /* 155 == 155 */
	PALETTERGB(0x00, 0xb9, 0xa8), /* 156 == 156 */
	PALETTERGB(0x00, 0xb9, 0x97), /* 157 == 157 */
	PALETTERGB(0x00, 0xb9, 0x87), /* 158 == 158 */
	PALETTERGB(0x00, 0xb9, 0x76), /* 159 == 159 */
	PALETTERGB(0x00, 0xb9, 0x65), /* 160 == 160 */
	PALETTERGB(0x00, 0xb9, 0x54), /* 161 == 161 */
	PALETTERGB(0x00, 0xb9, 0x43), /* 162 == 162 */
	PALETTERGB(0x00, 0xb9, 0x32), /* 163 == 163 */
	PALETTERGB(0x00, 0xb9, 0x21), /* 164 == 164 */
	PALETTERGB(0x00, 0xb9, 0x10), /* 165 == 165 */
	PALETTERGB(0x00, 0xb9, 0x00), /* 166 == 166 */
	PALETTERGB(0x10, 0xb9, 0x00), /* 167 == 167 */
	PALETTERGB(0x21, 0xb9, 0x00), /* 168 == 168 */
	PALETTERGB(0x32, 0xb9, 0x00), /* 169 == 169 */
	PALETTERGB(0x43, 0xb9, 0x00), /* 170 == 170 */
	PALETTERGB(0x54, 0xb9, 0x00), /* 171 == 171 */
	PALETTERGB(0x65, 0xb9, 0x00), /* 172 == 172 */
	PALETTERGB(0x76, 0xb9, 0x00), /* 173 == 173 */
	PALETTERGB(0x87, 0xb9, 0x00), /* 174 == 174 */
	PALETTERGB(0x97, 0xb9, 0x00), /* 175 == 175 */
	PALETTERGB(0xa8, 0xb9, 0x00), /* 176 == 176 */
	PALETTERGB(0xb9, 0xb9, 0x00), /* 177 == 177 */
	PALETTERGB(0xb9, 0xa8, 0x00), /* 178 == 178 */
	PALETTERGB(0xb9, 0x97, 0x00), /* 179 == 179 */
	PALETTERGB(0xb9, 0x87, 0x00), /* 180 == 180 */
	PALETTERGB(0xb9, 0x76, 0x00), /* 181 == 181 */
	PALETTERGB(0xb9, 0x65, 0x00), /* 182 == 182 */
	PALETTERGB(0xb9, 0x54, 0x00), /* 183 == 183 */
	PALETTERGB(0xb9, 0x43, 0x00), /* 184 == 184 */
	PALETTERGB(0xb9, 0x32, 0x00), /* 185 == 185 */
	PALETTERGB(0xb9, 0x21, 0x00), /* 186 == 186 */
	PALETTERGB(0xb9, 0x10, 0x00), /* 187 == 187 */
	PALETTERGB(0x76, 0x00, 0x00), /* 188 == 188 */
	PALETTERGB(0x76, 0x00, 0x10), /* 189 == 189 */
	PALETTERGB(0x76, 0x00, 0x21), /* 190 == 190 */
	PALETTERGB(0x76, 0x00, 0x32), /* 191 == 191 */
	PALETTERGB(0x76, 0x00, 0x43), /* 192 == 192 */
	PALETTERGB(0x76, 0x00, 0x54), /* 193 == 193 */
	PALETTERGB(0x76, 0x00, 0x65), /* 194 == 194 */
	PALETTERGB(0x76, 0x00, 0x76), /* 195 == 195 */
	PALETTERGB(0x65, 0x00, 0x76), /* 196 == 196 */
	PALETTERGB(0x54, 0x00, 0x76), /* 197 == 197 */
	PALETTERGB(0x43, 0x00, 0x76), /* 198 == 198 */
	PALETTERGB(0x32, 0x00, 0x76), /* 199 == 199 */
	PALETTERGB(0x21, 0x00, 0x76), /* 200 == 200 */
	PALETTERGB(0x10, 0x00, 0x76), /* 201 == 201 */
	PALETTERGB(0x00, 0x00, 0x76), /* 202 == 202 */
	PALETTERGB(0x00, 0x10, 0x76), /* 203 == 203 */
	PALETTERGB(0x00, 0x21, 0x76), /* 204 == 204 */
	PALETTERGB(0x00, 0x32, 0x76), /* 205 == 205 */
	PALETTERGB(0x00, 0x43, 0x76), /* 206 == 206 */
	PALETTERGB(0x00, 0x54, 0x76), /* 207 == 207 */
	PALETTERGB(0x00, 0x65, 0x76), /* 208 == 208 */
	PALETTERGB(0x00, 0x76, 0x76), /* 209 == 209 */
	PALETTERGB(0x00, 0x76, 0x65), /* 210 == 210 */
	PALETTERGB(0x00, 0x76, 0x54), /* 211 == 211 */
	PALETTERGB(0x00, 0x76, 0x43), /* 212 == 212 */
	PALETTERGB(0x00, 0x76, 0x32), /* 213 == 213 */
	PALETTERGB(0x00, 0x76, 0x21), /* 214 == 214 */
	PALETTERGB(0x00, 0x76, 0x10), /* 215 == 215 */
	PALETTERGB(0x00, 0x76, 0x00), /* 216 == 216 */
	PALETTERGB(0x10, 0x76, 0x00), /* 217 == 217 */
	PALETTERGB(0x21, 0x76, 0x00), /* 218 == 218 */
	PALETTERGB(0x32, 0x76, 0x00), /* 219 == 219 */
	PALETTERGB(0x43, 0x76, 0x00), /* 220 == 220 */
	PALETTERGB(0x54, 0x76, 0x00), /* 221 == 221 */
	PALETTERGB(0x65, 0x76, 0x00), /* 222 == 222 */
	PALETTERGB(0x76, 0x76, 0x00), /* 223 == 223 */
	PALETTERGB(0x76, 0x65, 0x00), /* 224 == 224 */
	PALETTERGB(0x76, 0x54, 0x00), /* 225 == 225 */
	PALETTERGB(0x76, 0x43, 0x00), /* 226 == 226 */
	PALETTERGB(0x76, 0x32, 0x00), /* 227 == 227 */
	PALETTERGB(0x76, 0x21, 0x00), /* 228 == 228 */
	PALETTERGB(0x76, 0x10, 0x00), /* 229 == 229 */
	PALETTERGB(0x43, 0x00, 0x00), /* 230 == 230 */
	PALETTERGB(0x43, 0x00, 0x10), /* 231 == 231 */
	PALETTERGB(0x43, 0x00, 0x21), /* 232 == 232 */
	PALETTERGB(0x43, 0x00, 0x32), /* 233 == 233 */
	PALETTERGB(0x43, 0x00, 0x43), /* 234 == 234 */
	PALETTERGB(0x32, 0x00, 0x43), /* 235 == 235 */
	PALETTERGB(0x21, 0x00, 0x43), /* 236 == 236 */
	PALETTERGB(0x10, 0x00, 0x43), /* 237 == 237 */
	PALETTERGB(0x00, 0x00, 0x43), /* 238 == 238 */
	PALETTERGB(0x00, 0x10, 0x43), /* 239 == 239 */
	PALETTERGB(0x00, 0x21, 0x43), /* 240 == 240 */
	PALETTERGB(0x00, 0x32, 0x43), /* 241 == 241 */
	PALETTERGB(0x00, 0x43, 0x43), /* 242 == 242 */
	PALETTERGB(0x00, 0x43, 0x32), /* 243 == 243 */
	PALETTERGB(0x00, 0x43, 0x21), /* 244 == 244 */
	PALETTERGB(0x00, 0x43, 0x10), /* 245 == 245 */
	PALETTERGB(0x00, 0x43, 0x00), /* 246 == 246 */
	PALETTERGB(0x10, 0x43, 0x00), /* 247 == 247 */
	PALETTERGB(0x21, 0x43, 0x00), /* 248 == 248 */
	PALETTERGB(0x32, 0x43, 0x00), /* 249 == 249 */
	PALETTERGB(0x43, 0x43, 0x00), /* 250 == 250 */
	PALETTERGB(0x43, 0x32, 0x00), /* 251 == 251 */
	PALETTERGB(0x43, 0x21, 0x00), /* 252 == 252 */
	PALETTERGB(0x43, 0x10, 0x00), /* 253 == 253 */
	PALETTERGB(0xfd, 0xfd, 0xfd), /* 254 == 254 */
	PALETTERGB(0x00, 0x00, 0x00)  /* 255 == BLACK (1) */
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void init_screen(void)
{
	HDC hDC;
	_WORD bits;

	if (screen_w == 0)
	{
		screen_w = (_WORD) GetSystemMetrics(SM_CXSCREEN);
		screen_h = (_WORD) GetSystemMetrics(SM_CYSCREEN);

		hDC = GetDC(HWND_DESKTOP);
		screen_planes = GetDeviceCaps(hDC, PLANES);
		bits = GetDeviceCaps(hDC, BITSPIXEL);
		screen_planes *= bits;
		screen_colors = GetDeviceCaps(hDC, NUMCOLORS);
		if ((screen_planes * bits) >= 16)
			screen_colors = 32766;		/* more than we need */

		ReleaseDC(HWND_DESKTOP, hDC);
	}
}

/*** ---------------------------------------------------------------------- ***/

GLOBAL _WORD GetNumPlanes(_VOID)
{
	init_screen();
	return screen_planes;
}

/*** ---------------------------------------------------------------------- ***/

_WORD GetNumColors(void)
{
	init_screen();
	return screen_colors;
}

/*** ---------------------------------------------------------------------- ***/

void W_Release_Bitmap(void **pdata, _WORD w, _WORD h, _WORD nplanes)
{
	IBITMAP *map;

	UNUSED(w);
	UNUSED(h);
	UNUSED(nplanes);
	map = (IBITMAP *)(*pdata);
	if (map->bm_magic != BITMAP_MAGIC)
	{
		hyp_debug("W_Release_Bitmap: data not fixed");
		return;
	}
	*pdata = map->bm_orig_data;
	g_free(map);
}

/*** ---------------------------------------------------------------------- ***/

gboolean W_Fix_Bitmap(void **pdata, _WORD w, _WORD h, _WORD nplanes)
{
	size_t src_rowstride = ((w + 15) & ~0x0f) >> 3;
	size_t planesize = src_rowstride * (size_t)h;
	size_t dst_rowstride, dst_imagesize;
	IBITMAP *map;
	size_t addsize;
	_WORD biplanes;
	_WORD num_colors;
	
	if (pdata != NULL && *pdata != NULL)
	{
		map = (IBITMAP *)(*pdata);
		if (map->bm_magic == BITMAP_MAGIC)
		{
			hyp_debug("W_Fix_Bitmap: already fixed");
			return FALSE;
		}
		num_colors = 1 << nplanes;
		if (nplanes == 1)
		{
			addsize = sizeof(BITMAP);
		} else
		{
			if (nplanes > 8)
				num_colors = 0;
			addsize = sizeof(BITMAPINFOHEADER) + num_colors * sizeof(RGBQUAD);
		}
		biplanes = nplanes;
		switch (biplanes)
		{
		case 1:
			/* correct value for writing bmp */
			/* dst_rowstride = ((w + 31) & ~0x1f) >> 3; */
			/* real value because we convert 16 pixel at a time */
			dst_rowstride = src_rowstride;
			break;
		case 2:
			/*
			 * GDI does not handle 2 bits per color, use 4 instead
			 */
			biplanes = 4;
			/* correct value for writing bmp */
			/* dst_rowstride = (((w + 1) >> 1) + 3) & ~3; */
			/* real value because we convert 16 pixel at a time */
			dst_rowstride = src_rowstride << 2;
			break;
		case 4:
			/* correct value for writing bmp */
			/* dst_rowstride = (((w + 1) >> 1) + 3) & ~3; */
			/* real value because we convert 16 pixel at a time */
			dst_rowstride = src_rowstride << 2;
			break;
		case 8:
			/* correct value for writing bmp */
			/* dst_rowstride = (w + 3) & ~3; */
			/* real value because we convert 16 pixel at a time */
			dst_rowstride = src_rowstride << 3;
			break;
		case 15:
		case 16:
			/* correct value for writing bmp */
			/* dst_rowstride = ((w << 1) + 3) & ~3; */
			/* real value because we convert 16 pixel at a time */
			dst_rowstride = src_rowstride << 4;
			break;
		case 24:
			/* correct value for writing bmp */
			/* dst_rowstride = ((w * 3) + 3) & ~3; */
			/* real value because we convert 16 pixel at a time */
			dst_rowstride = src_rowstride * 24;
			break;
		case 32:
			/* correct value for writing bmp */
			/* dst_rowstride = w << 2; */
			/* real value because we convert 16 pixel at a time */
			dst_rowstride = src_rowstride << 5;
			break;
		default:
			hyp_debug("unsupported planes %d\n", biplanes);
			*pdata = NULL;
			return FALSE;
		}
		dst_imagesize = dst_rowstride * (size_t) h;
		map = (IBITMAP *)g_try_malloc(sizeof(*map) + addsize + dst_imagesize);
		if (map != NULL)
		{
			_UBYTE *src = (_UBYTE *)(*pdata);
			_UBYTE *dst, *p;

			map->bm_orig_data = src;
			map->bm_info = (_UBYTE *)(map + 1);
			map->bm_data = (_UBYTE *)(map + 1) + addsize;
			map->bm_magic = BITMAP_MAGIC;
			*pdata = map;

			if (nplanes == 1)
			{
				{
					BITMAP *ic;

					ic = (BITMAP *)map->bm_info;
					ic->bmType = 0;
					ic->bmWidth = w;
					ic->bmHeight = h;
					ic->bmWidthBytes = dst_rowstride;
					ic->bmPlanes = 1;
					ic->bmBitsPixel = 1;
					ic->bmBits = (LPSTR) map->bm_data;
				}
				dst = (_UBYTE *)map->bm_data;
				if (src_rowstride == dst_rowstride)
				{
					_UBYTE *end = src + planesize;
					while (src < end)
					{
						*dst = ~(*src);
						src++;
						dst++;
					}
				} else
				{
					while (h > 0)
					{
						for (w = (_UWORD)src_rowstride; w > 0; w--)
						{
							*dst = ~(*src);
							src++;
							dst++;
						}
						*dst++ = 0;
						*dst++ = 0;
						h--;
					}
				}
			} else
			{
				_UBYTE mask;
				_WORD x, y;
				_UWORD color;

				{
					BITMAPINFO *info;
					_WORD i, ansi_color;
					const _WORD *table;

					num_colors = 1 << nplanes;
					info = (BITMAPINFO *)map->bm_info;
					info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					/* correct value for writing bmp */
					/* info->bmiHeader.biWidth = w; */
					/* real value because we convert 16 pixel at a time */
					info->bmiHeader.biWidth = ((w + 15) & ~0x0f);
					switch (nplanes)
					{
					case 15:
					case 16:
					case 24:
					case 32:
						info->bmiHeader.biWidth = (dst_rowstride << 3) / nplanes;
						break;
					}
					info->bmiHeader.biHeight = h;
					info->bmiHeader.biPlanes = 1;
					info->bmiHeader.biBitCount = biplanes;
					info->bmiHeader.biCompression = BI_RGB;
					info->bmiHeader.biSizeImage = 0;
					info->bmiHeader.biXPelsPerMeter = 100;
					info->bmiHeader.biYPelsPerMeter = 100;
					info->bmiHeader.biClrUsed = num_colors;
					info->bmiHeader.biClrImportant = num_colors;

					if (nplanes == 2)
						table = pix2col_2;
					else if (nplanes == 4)
						table = pix2col_4;
					else
						table = pix2col_8;
					for (i = 0; i < num_colors; i++)
					{
						ansi_color = table[i];
						info->bmiColors[i].rgbRed	= GetRValue(std_palette[ansi_color]);
						info->bmiColors[i].rgbGreen = GetGValue(std_palette[ansi_color]);
						info->bmiColors[i].rgbBlue	= GetBValue(std_palette[ansi_color]);
						info->bmiColors[i].rgbReserved = 0;
					}
				}

				dst = (_UBYTE *)map->bm_data + dst_imagesize;
				if (nplanes == 4)
				{
					w = ((w + 15) & ~0x0f);
					for (y = h; --y >= 0; )
					{
						dst -= dst_rowstride;
						p = dst;
						for (x = w; x > 0; x -= 8)
						{
							for (mask = 0x80; mask != 0; mask >>= 1)
							{
								color = 0;
								if (src[planesize * 0] & mask) color |= 0x01;
								if (src[planesize * 1] & mask) color |= 0x02;
								if (src[planesize * 2] & mask) color |= 0x04;
								if (src[planesize * 3] & mask) color |= 0x08;
								*p = coltab16[color] << 4;
								color = 0;
								mask >>= 1;
								if (src[planesize * 0] & mask) color |= 0x01;
								if (src[planesize * 1] & mask) color |= 0x02;
								if (src[planesize * 2] & mask) color |= 0x04;
								if (src[planesize * 3] & mask) color |= 0x08;
								*p++ |= coltab16[color];
							}
							src++;
						}
					}
				} else if (nplanes == 2)
				{
					w = ((w + 15) & ~0x0f);
					for (y = h; --y >= 0; )
					{
						dst -= dst_rowstride;
						p = dst;
						for (x = w; x > 0; x -= 8)
						{
							for (mask = 0x80; mask != 0; mask >>= 1)
							{
								color = 0;
								if (src[planesize * 0] & mask) color |= 0x01;
								if (src[planesize * 1] & mask) color |= 0x02;
								*p = coltab4[color] << 4;
								color = 0;
								mask >>= 1;
								if (src[planesize * 0] & mask) color |= 0x01;
								if (src[planesize * 1] & mask) color |= 0x02;
								*p++ |= coltab4[color];
								color = 0;
								mask >>= 1;
								if (src[planesize * 0] & mask) color |= 0x01;
								if (src[planesize * 1] & mask) color |= 0x02;
								*p = coltab4[color] << 4;
								color = 0;
								mask >>= 1;
								if (src[planesize * 0] & mask) color |= 0x01;
								if (src[planesize * 1] & mask) color |= 0x02;
								*p++ |= coltab4[color];
							}
							src++;
						}
					}
				} else if (nplanes == 8)
				{
					w = ((w + 15) & ~0x0f);
					for (y = 0; y < h; y++)
					{
						dst -= dst_rowstride;
						p = dst;
						for (x = w; x > 0; x -= 8)
						{
							for (mask = 0x80; mask != 0; mask >>= 1)
							{
								color = 0;
								if (src[planesize * 0] & mask) color |= 0x01;
								if (src[planesize * 1] & mask) color |= 0x02;
								if (src[planesize * 2] & mask) color |= 0x04;
								if (src[planesize * 3] & mask) color |= 0x08;
								if (src[planesize * 4] & mask) color |= 0x10;
								if (src[planesize * 5] & mask) color |= 0x20;
								if (src[planesize * 6] & mask) color |= 0x40;
								if (src[planesize * 7] & mask) color |= 0x80;
								*p++ = coltab256[color];
							}
							src++;
						}
					}
				}
			}
			/* return value indicates wether data was converted inplace */
			return FALSE;
		}
	}
	return FALSE;
}
