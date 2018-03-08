/*****************************************************************************
 * PICCONV.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>
#ifdef OS_ATARI
#include <osbind.h>
#endif

LOCAL _UBYTE const masktab[8] = {
	0xC0, 0x30, 0x0C, 0x03, 0xC0, 0x30, 0x0C, 0x03
};

LOCAL TOSPALETTE const palet1 = {
	{ 0x07, 0x77 },
	{ 0x03, 0x33 },
	{ 0x03, 0x33 },
	{ 0x04, 0x44 },
	{ 0x03, 0x33 },
	{ 0x04, 0x44 },
	{ 0x04, 0x44 },
	{ 0x06, 0x66 },
	{ 0x03, 0x33 },
	{ 0x04, 0x44 },
	{ 0x04, 0x44 },
	{ 0x06, 0x66 },
	{ 0x04, 0x44 },
	{ 0x06, 0x66 },
	{ 0x06, 0x66 },
	{ 0x00, 0x00 }
};

LOCAL PALETTE const std256_palette = {
	{ 0xff, 0xff, 0xff }, /*   0 == WHITE (0) */
	{ 0xff, 0x00, 0x00 }, /*   1 == RED (2) */
	{ 0x00, 0xff, 0x00 }, /*   2 == GREEN (3) */
	{ 0xff, 0xff, 0x00 }, /*   3 == YELLOW (6) */
	{ 0x00, 0x00, 0xff }, /*   4 == BLUE (4) */
	{ 0xff, 0x00, 0xff }, /*   5 == MAGENTA (7) */
	{ 0x00, 0xff, 0xff }, /*   6 == CYAN (5) */
	{ 0xcc, 0xcc, 0xcc }, /*   7 == LWHITE (8) */
	{ 0x88, 0x88, 0x88 }, /*   8 == LBLACK (9) */
	{ 0x88, 0x00, 0x00 }, /*   9 == LRED (10) */
	{ 0x00, 0x88, 0x00 }, /*  10 == LGREEN (11) */
	{ 0x88, 0x88, 0x00 }, /*  11 == LYELLOW (14) */
	{ 0x00, 0x00, 0x88 }, /*  12 == LBLUE (12) */
	{ 0x88, 0x00, 0x88 }, /*  13 == LMAGENTA (15) */
	{ 0x00, 0x88, 0x88 }, /*  14 == LCYAN (13) */
	{ 0x00, 0x00, 0x00 }, /*  15 == BLACK (255) */
	{ 0xfd, 0xfd, 0xfd }, /*  16 == 16 */
	{ 0xec, 0xec, 0xec }, /*  17 == 17 */
	{ 0xdb, 0xdb, 0xdb }, /*  18 == 18 */
	{ 0xca, 0xca, 0xca }, /*  19 == 19 */
	{ 0xb9, 0xb9, 0xb9 }, /*  20 == 20 */
	{ 0xa8, 0xa8, 0xa8 }, /*  21 == 21 */
	{ 0x97, 0x97, 0x97 }, /*  22 == 22 */
	{ 0x87, 0x87, 0x87 }, /*  23 == 23 */
	{ 0x76, 0x76, 0x76 }, /*  24 == 24 */
	{ 0x65, 0x65, 0x65 }, /*  25 == 25 */
	{ 0x54, 0x54, 0x54 }, /*  26 == 26 */
	{ 0x43, 0x43, 0x43 }, /*  27 == 27 */
	{ 0x32, 0x32, 0x32 }, /*  28 == 28 */
	{ 0x21, 0x21, 0x21 }, /*  29 == 29 */
	{ 0x10, 0x10, 0x10 }, /*  30 == 30 */
	{ 0x00, 0x00, 0x00 }, /*  31 == 31 */
	{ 0xfd, 0x00, 0x00 }, /*  32 == 32 */
	{ 0xfd, 0x00, 0x10 }, /*  33 == 33 */
	{ 0xfd, 0x00, 0x21 }, /*  34 == 34 */
	{ 0xfd, 0x00, 0x32 }, /*  35 == 35 */
	{ 0xfd, 0x00, 0x43 }, /*  36 == 36 */
	{ 0xfd, 0x00, 0x54 }, /*  37 == 37 */
	{ 0xfd, 0x00, 0x65 }, /*  38 == 38 */
	{ 0xfd, 0x00, 0x76 }, /*  39 == 39 */
	{ 0xfd, 0x00, 0x87 }, /*  40 == 40 */
	{ 0xfd, 0x00, 0x97 }, /*  41 == 41 */
	{ 0xfd, 0x00, 0xa8 }, /*  42 == 42 */
	{ 0xfd, 0x00, 0xb9 }, /*  43 == 43 */
	{ 0xfd, 0x00, 0xca }, /*  44 == 44 */
	{ 0xfd, 0x00, 0xdb }, /*  45 == 45 */
	{ 0xfd, 0x00, 0xec }, /*  46 == 46 */
	{ 0xfd, 0x00, 0xfd }, /*  47 == 47 */
	{ 0xec, 0x00, 0xfd }, /*  48 == 48 */
	{ 0xdb, 0x00, 0xfd }, /*  49 == 49 */
	{ 0xca, 0x00, 0xfd }, /*  50 == 50 */
	{ 0xb9, 0x00, 0xfd }, /*  51 == 51 */
	{ 0xa8, 0x00, 0xfd }, /*  52 == 52 */
	{ 0x97, 0x00, 0xfd }, /*  53 == 53 */
	{ 0x87, 0x00, 0xfd }, /*  54 == 54 */
	{ 0x76, 0x00, 0xfd }, /*  55 == 55 */
	{ 0x65, 0x00, 0xfd }, /*  56 == 56 */
	{ 0x54, 0x00, 0xfd }, /*  57 == 57 */
	{ 0x43, 0x00, 0xfd }, /*  58 == 58 */
	{ 0x32, 0x00, 0xfd }, /*  59 == 59 */
	{ 0x21, 0x00, 0xfd }, /*  60 == 60 */
	{ 0x10, 0x00, 0xfd }, /*  61 == 61 */
	{ 0x00, 0x00, 0xfd }, /*  62 == 62 */
	{ 0x00, 0x10, 0xfd }, /*  63 == 63 */
	{ 0x00, 0x21, 0xfd }, /*  64 == 64 */
	{ 0x00, 0x32, 0xfd }, /*  65 == 65 */
	{ 0x00, 0x43, 0xfd }, /*  66 == 66 */
	{ 0x00, 0x54, 0xfd }, /*  67 == 67 */
	{ 0x00, 0x65, 0xfd }, /*  68 == 68 */
	{ 0x00, 0x76, 0xfd }, /*  69 == 69 */
	{ 0x00, 0x87, 0xfd }, /*  70 == 70 */
	{ 0x00, 0x97, 0xfd }, /*  71 == 71 */
	{ 0x00, 0xa8, 0xfd }, /*  72 == 72 */
	{ 0x00, 0xb9, 0xfd }, /*  73 == 73 */
	{ 0x00, 0xca, 0xfd }, /*  74 == 74 */
	{ 0x00, 0xdb, 0xfd }, /*  75 == 75 */
	{ 0x00, 0xec, 0xfd }, /*  76 == 76 */
	{ 0x00, 0xfd, 0xfd }, /*  77 == 77 */
	{ 0x00, 0xfd, 0xec }, /*  78 == 78 */
	{ 0x00, 0xfd, 0xdb }, /*  79 == 79 */
	{ 0x00, 0xfd, 0xca }, /*  80 == 80 */
	{ 0x00, 0xfd, 0xb9 }, /*  81 == 81 */
	{ 0x00, 0xfd, 0xa8 }, /*  82 == 82 */
	{ 0x00, 0xfd, 0x97 }, /*  83 == 83 */
	{ 0x00, 0xfd, 0x87 }, /*  84 == 84 */
	{ 0x00, 0xfd, 0x76 }, /*  85 == 85 */
	{ 0x00, 0xfd, 0x65 }, /*  86 == 86 */
	{ 0x00, 0xfd, 0x54 }, /*  87 == 87 */
	{ 0x00, 0xfd, 0x43 }, /*  88 == 88 */
	{ 0x00, 0xfd, 0x32 }, /*  89 == 89 */
	{ 0x00, 0xfd, 0x21 }, /*  90 == 90 */
	{ 0x00, 0xfd, 0x10 }, /*  91 == 91 */
	{ 0x00, 0xfd, 0x00 }, /*  92 == 92 */
	{ 0x10, 0xfd, 0x00 }, /*  93 == 93 */
	{ 0x21, 0xfd, 0x00 }, /*  94 == 94 */
	{ 0x32, 0xfd, 0x00 }, /*  95 == 95 */
	{ 0x43, 0xfd, 0x00 }, /*  96 == 96 */
	{ 0x54, 0xfd, 0x00 }, /*  97 == 97 */
	{ 0x65, 0xfd, 0x00 }, /*  98 == 98 */
	{ 0x76, 0xfd, 0x00 }, /*  99 == 99 */
	{ 0x87, 0xfd, 0x00 }, /* 100 == 100 */
	{ 0x97, 0xfd, 0x00 }, /* 101 == 101 */
	{ 0xa8, 0xfd, 0x00 }, /* 102 == 102 */
	{ 0xb9, 0xfd, 0x00 }, /* 103 == 103 */
	{ 0xca, 0xfd, 0x00 }, /* 104 == 104 */
	{ 0xdb, 0xfd, 0x00 }, /* 105 == 105 */
	{ 0xec, 0xfd, 0x00 }, /* 106 == 106 */
	{ 0xfd, 0xfd, 0x00 }, /* 107 == 107 */
	{ 0xfd, 0xec, 0x00 }, /* 108 == 108 */
	{ 0xfd, 0xdb, 0x00 }, /* 109 == 109 */
	{ 0xfd, 0xca, 0x00 }, /* 110 == 110 */
	{ 0xfd, 0xb9, 0x00 }, /* 111 == 111 */
	{ 0xfd, 0xa8, 0x00 }, /* 112 == 112 */
	{ 0xfd, 0x97, 0x00 }, /* 113 == 113 */
	{ 0xfd, 0x87, 0x00 }, /* 114 == 114 */
	{ 0xfd, 0x76, 0x00 }, /* 115 == 115 */
	{ 0xfd, 0x65, 0x00 }, /* 116 == 116 */
	{ 0xfd, 0x54, 0x00 }, /* 117 == 117 */
	{ 0xfd, 0x43, 0x00 }, /* 118 == 118 */
	{ 0xfd, 0x32, 0x00 }, /* 119 == 119 */
	{ 0xfd, 0x21, 0x00 }, /* 120 == 120 */
	{ 0xfd, 0x10, 0x00 }, /* 121 == 121 */
	{ 0xb9, 0x00, 0x00 }, /* 122 == 122 */
	{ 0xb9, 0x00, 0x10 }, /* 123 == 123 */
	{ 0xb9, 0x00, 0x21 }, /* 124 == 124 */
	{ 0xb9, 0x00, 0x32 }, /* 125 == 125 */
	{ 0xb9, 0x00, 0x43 }, /* 126 == 126 */
	{ 0xb9, 0x00, 0x54 }, /* 127 == 127 */
	{ 0xb9, 0x00, 0x65 }, /* 128 == 128 */
	{ 0xb9, 0x00, 0x76 }, /* 129 == 129 */
	{ 0xb9, 0x00, 0x87 }, /* 130 == 130 */
	{ 0xb9, 0x00, 0x97 }, /* 131 == 131 */
	{ 0xb9, 0x00, 0xa8 }, /* 132 == 132 */
	{ 0xb9, 0x00, 0xb9 }, /* 133 == 133 */
	{ 0xa8, 0x00, 0xb9 }, /* 134 == 134 */
	{ 0x97, 0x00, 0xb9 }, /* 135 == 135 */
	{ 0x87, 0x00, 0xb9 }, /* 136 == 136 */
	{ 0x76, 0x00, 0xb9 }, /* 137 == 137 */
	{ 0x65, 0x00, 0xb9 }, /* 138 == 138 */
	{ 0x54, 0x00, 0xb9 }, /* 139 == 139 */
	{ 0x43, 0x00, 0xb9 }, /* 140 == 140 */
	{ 0x32, 0x00, 0xb9 }, /* 141 == 141 */
	{ 0x21, 0x00, 0xb9 }, /* 142 == 142 */
	{ 0x10, 0x00, 0xb9 }, /* 143 == 143 */
	{ 0x00, 0x00, 0xb9 }, /* 144 == 144 */
	{ 0x00, 0x10, 0xb9 }, /* 145 == 145 */
	{ 0x00, 0x21, 0xb9 }, /* 146 == 146 */
	{ 0x00, 0x32, 0xb9 }, /* 147 == 147 */
	{ 0x00, 0x43, 0xb9 }, /* 148 == 148 */
	{ 0x00, 0x54, 0xb9 }, /* 149 == 149 */
	{ 0x00, 0x65, 0xb9 }, /* 150 == 150 */
	{ 0x00, 0x76, 0xb9 }, /* 151 == 151 */
	{ 0x00, 0x87, 0xb9 }, /* 152 == 152 */
	{ 0x00, 0x97, 0xb9 }, /* 153 == 153 */
	{ 0x00, 0xa8, 0xb9 }, /* 154 == 154 */
	{ 0x00, 0xb9, 0xb9 }, /* 155 == 155 */
	{ 0x00, 0xb9, 0xa8 }, /* 156 == 156 */
	{ 0x00, 0xb9, 0x97 }, /* 157 == 157 */
	{ 0x00, 0xb9, 0x87 }, /* 158 == 158 */
	{ 0x00, 0xb9, 0x76 }, /* 159 == 159 */
	{ 0x00, 0xb9, 0x65 }, /* 160 == 160 */
	{ 0x00, 0xb9, 0x54 }, /* 161 == 161 */
	{ 0x00, 0xb9, 0x43 }, /* 162 == 162 */
	{ 0x00, 0xb9, 0x32 }, /* 163 == 163 */
	{ 0x00, 0xb9, 0x21 }, /* 164 == 164 */
	{ 0x00, 0xb9, 0x10 }, /* 165 == 165 */
	{ 0x00, 0xb9, 0x00 }, /* 166 == 166 */
	{ 0x10, 0xb9, 0x00 }, /* 167 == 167 */
	{ 0x21, 0xb9, 0x00 }, /* 168 == 168 */
	{ 0x32, 0xb9, 0x00 }, /* 169 == 169 */
	{ 0x43, 0xb9, 0x00 }, /* 170 == 170 */
	{ 0x54, 0xb9, 0x00 }, /* 171 == 171 */
	{ 0x65, 0xb9, 0x00 }, /* 172 == 172 */
	{ 0x76, 0xb9, 0x00 }, /* 173 == 173 */
	{ 0x87, 0xb9, 0x00 }, /* 174 == 174 */
	{ 0x97, 0xb9, 0x00 }, /* 175 == 175 */
	{ 0xa8, 0xb9, 0x00 }, /* 176 == 176 */
	{ 0xb9, 0xb9, 0x00 }, /* 177 == 177 */
	{ 0xb9, 0xa8, 0x00 }, /* 178 == 178 */
	{ 0xb9, 0x97, 0x00 }, /* 179 == 179 */
	{ 0xb9, 0x87, 0x00 }, /* 180 == 180 */
	{ 0xb9, 0x76, 0x00 }, /* 181 == 181 */
	{ 0xb9, 0x65, 0x00 }, /* 182 == 182 */
	{ 0xb9, 0x54, 0x00 }, /* 183 == 183 */
	{ 0xb9, 0x43, 0x00 }, /* 184 == 184 */
	{ 0xb9, 0x32, 0x00 }, /* 185 == 185 */
	{ 0xb9, 0x21, 0x00 }, /* 186 == 186 */
	{ 0xb9, 0x10, 0x00 }, /* 187 == 187 */
	{ 0x76, 0x00, 0x00 }, /* 188 == 188 */
	{ 0x76, 0x00, 0x10 }, /* 189 == 189 */
	{ 0x76, 0x00, 0x21 }, /* 190 == 190 */
	{ 0x76, 0x00, 0x32 }, /* 191 == 191 */
	{ 0x76, 0x00, 0x43 }, /* 192 == 192 */
	{ 0x76, 0x00, 0x54 }, /* 193 == 193 */
	{ 0x76, 0x00, 0x65 }, /* 194 == 194 */
	{ 0x76, 0x00, 0x76 }, /* 195 == 195 */
	{ 0x65, 0x00, 0x76 }, /* 196 == 196 */
	{ 0x54, 0x00, 0x76 }, /* 197 == 197 */
	{ 0x43, 0x00, 0x76 }, /* 198 == 198 */
	{ 0x32, 0x00, 0x76 }, /* 199 == 199 */
	{ 0x21, 0x00, 0x76 }, /* 200 == 200 */
	{ 0x10, 0x00, 0x76 }, /* 201 == 201 */
	{ 0x00, 0x00, 0x76 }, /* 202 == 202 */
	{ 0x00, 0x10, 0x76 }, /* 203 == 203 */
	{ 0x00, 0x21, 0x76 }, /* 204 == 204 */
	{ 0x00, 0x32, 0x76 }, /* 205 == 205 */
	{ 0x00, 0x43, 0x76 }, /* 206 == 206 */
	{ 0x00, 0x54, 0x76 }, /* 207 == 207 */
	{ 0x00, 0x65, 0x76 }, /* 208 == 208 */
	{ 0x00, 0x76, 0x76 }, /* 209 == 209 */
	{ 0x00, 0x76, 0x65 }, /* 210 == 210 */
	{ 0x00, 0x76, 0x54 }, /* 211 == 211 */
	{ 0x00, 0x76, 0x43 }, /* 212 == 212 */
	{ 0x00, 0x76, 0x32 }, /* 213 == 213 */
	{ 0x00, 0x76, 0x21 }, /* 214 == 214 */
	{ 0x00, 0x76, 0x10 }, /* 215 == 215 */
	{ 0x00, 0x76, 0x00 }, /* 216 == 216 */
	{ 0x10, 0x76, 0x00 }, /* 217 == 217 */
	{ 0x21, 0x76, 0x00 }, /* 218 == 218 */
	{ 0x32, 0x76, 0x00 }, /* 219 == 219 */
	{ 0x43, 0x76, 0x00 }, /* 220 == 220 */
	{ 0x54, 0x76, 0x00 }, /* 221 == 221 */
	{ 0x65, 0x76, 0x00 }, /* 222 == 222 */
	{ 0x76, 0x76, 0x00 }, /* 223 == 223 */
	{ 0x76, 0x65, 0x00 }, /* 224 == 224 */
	{ 0x76, 0x54, 0x00 }, /* 225 == 225 */
	{ 0x76, 0x43, 0x00 }, /* 226 == 226 */
	{ 0x76, 0x32, 0x00 }, /* 227 == 227 */
	{ 0x76, 0x21, 0x00 }, /* 228 == 228 */
	{ 0x76, 0x10, 0x00 }, /* 229 == 229 */
	{ 0x43, 0x00, 0x00 }, /* 230 == 230 */
	{ 0x43, 0x00, 0x10 }, /* 231 == 231 */
	{ 0x43, 0x00, 0x21 }, /* 232 == 232 */
	{ 0x43, 0x00, 0x32 }, /* 233 == 233 */
	{ 0x43, 0x00, 0x43 }, /* 234 == 234 */
	{ 0x32, 0x00, 0x43 }, /* 235 == 235 */
	{ 0x21, 0x00, 0x43 }, /* 236 == 236 */
	{ 0x10, 0x00, 0x43 }, /* 237 == 237 */
	{ 0x00, 0x00, 0x43 }, /* 238 == 238 */
	{ 0x00, 0x10, 0x43 }, /* 239 == 239 */
	{ 0x00, 0x21, 0x43 }, /* 240 == 240 */
	{ 0x00, 0x32, 0x43 }, /* 241 == 241 */
	{ 0x00, 0x43, 0x43 }, /* 242 == 242 */
	{ 0x00, 0x43, 0x32 }, /* 243 == 243 */
	{ 0x00, 0x43, 0x21 }, /* 244 == 244 */
	{ 0x00, 0x43, 0x10 }, /* 245 == 245 */
	{ 0x00, 0x43, 0x00 }, /* 246 == 246 */
	{ 0x10, 0x43, 0x00 }, /* 247 == 247 */
	{ 0x21, 0x43, 0x00 }, /* 248 == 248 */
	{ 0x32, 0x43, 0x00 }, /* 249 == 249 */
	{ 0x43, 0x43, 0x00 }, /* 250 == 250 */
	{ 0x43, 0x32, 0x00 }, /* 251 == 251 */
	{ 0x43, 0x21, 0x00 }, /* 252 == 252 */
	{ 0x43, 0x10, 0x00 }, /* 253 == 253 */
	{ 0xfd, 0xfd, 0xfd }, /* 254 == 254 */
	{ 0x00, 0x00, 0x00 }  /* 255 == BLACK (1) */
};


LOCAL _UBYTE const pattern1[16 * 16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
	0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xAA, 0x00,
	0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xAA, 0x00,
	0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00,
	0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00,
	0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44,
	0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44,
	0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11,
	0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11,
	0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55,
	0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55,
	0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
	0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
	0x55, 0xBB, 0x55, 0xEE, 0x55, 0xBB, 0x55, 0xEE,
	0x55, 0xBB, 0x55, 0xEE, 0x55, 0xBB, 0x55, 0xEE,
	0x55, 0xBB, 0x55, 0xFF, 0x55, 0xBB, 0x55, 0xFF,
	0x55, 0xBB, 0x55, 0xFF, 0x55, 0xBB, 0x55, 0xFF,
	0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF,
	0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF,
	0x77, 0xFF, 0x55, 0xFF, 0x77, 0xFF, 0x55, 0xFF,
	0x77, 0xFF, 0x55, 0xFF, 0x77, 0xFF, 0x55, 0xFF,
	0x77, 0xFF, 0xDD, 0xFF, 0x77, 0xFF, 0xDD, 0xFF,
	0x77, 0xFF, 0xDD, 0xFF, 0x77, 0xFF, 0xDD, 0xFF,
	0x77, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF,
	0x77, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

LOCAL _UBYTE const pattern2[16 * 16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
	0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,
	0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xAA, 0x00,
	0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xAA, 0x00,
	0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00,
	0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00,
	0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44,
	0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44, 0xAA, 0x44,
	0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11,
	0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11,
	0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55,
	0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55,
	0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
	0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
	0x55, 0xBB, 0x55, 0xEE, 0x55, 0xBB, 0x55, 0xEE,
	0x55, 0xBB, 0x55, 0xEE, 0x55, 0xBB, 0x55, 0xEE,
	0x55, 0xBB, 0x55, 0xFF, 0x55, 0xBB, 0x55, 0xFF,
	0x55, 0xBB, 0x55, 0xFF, 0x55, 0xBB, 0x55, 0xFF,
	0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF,
	0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF,
	0x77, 0xFF, 0x55, 0xFF, 0x77, 0xFF, 0x55, 0xFF,
	0x77, 0xFF, 0x55, 0xFF, 0x77, 0xFF, 0x55, 0xFF,
	0x77, 0xFF, 0xDD, 0xFF, 0x77, 0xFF, 0xDD, 0xFF,
	0x77, 0xFF, 0xDD, 0xFF, 0x77, 0xFF, 0xDD, 0xFF,
	0x77, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF,
	0x77, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
LOCAL const _UBYTE *patptr1[16];
LOCAL const _UBYTE *patptr2[16];

#define toword(pixels) (((pixels) + 15) >> 4)
#define tobyte(pixels) (toword(pixels) << 1)

static _WORD const rgb_to_vdi_tab[256] = {
   0,    4,    8,   12,   16,   20,   24,   28,   32,   36,   40,   43,   47,   51,   55,   59,
  63,   67,   71,   75,   79,   83,   86,   90,   94,   98,  102,  106,  110,  114,  118,  122,
 126,  129,  133,  137,  141,  145,  149,  153,  157,  161,  165,  168,  172,  176,  180,  184,
 188,  192,  196,  200,  204,  208,  211,  215,  219,  223,  227,  231,  235,  239,  243,  247,
 251,  254,  258,  262,  266,  270,  274,  278,  282,  286,  290,  294,  298,  301,  305,  309,
 313,  317,  321,  325,  329,  333,  337,  341,  345,  349,  352,  356,  360,  364,  368,  372,
 376,  380,  384,  388,  392,  396,  400,  403,  407,  411,  415,  419,  423,  427,  431,  435,
 439,  443,  447,  450,  454,  458,  462,  466,  470,  474,  478,  482,  486,  490,  494,  498,
 501,  505,  509,  513,  517,  521,  525,  529,  533,  537,  541,  545,  549,  552,  556,  560,
 564,  568,  572,  576,  580,  584,  588,  592,  596,  600,  603,  607,  611,  615,  619,  623,
 627,  631,  635,  639,  643,  647,  650,  654,  658,  662,  666,  670,  674,  678,  682,  686,
 690,  694,  698,  701,  705,  709,  713,  717,  721,  725,  729,  733,  737,  741,  745,  749,
 752,  756,  760,  764,  768,  772,  776,  780,  784,  788,  792,  796,  800,  803,  807,  811,
 815,  819,  823,  827,  831,  835,  839,  843,  847,  850,  854,  858,  862,  866,  870,  874,
 878,  882,  886,  890,  894,  898,  901,  905,  909,  913,  917,  921,  925,  929,  933,  937,
 941,  945,  949,  952,  956,  960,  964,  968,  972,  976,  980,  984,  988,  992,  996, 1000
};

/*** ---------------------------------------------------------------------- ***/

_WORD pic_rgb_to_vdi(unsigned char c)
{
	return rgb_to_vdi_tab[c];
}

/*** ---------------------------------------------------------------------- ***/

void pic_rgb_to_vdipal(_WORD *vdi, PALETTE pal, _WORD colors, const _WORD pixel_to_color[])
{
	_WORD i;
	
	for (i = 0; i < colors && i < 256; i++)
	{
		vdi[0] = rgb_to_vdi_tab[pal[i].r];
		vdi[1] = rgb_to_vdi_tab[pal[i].g];
		vdi[2] = rgb_to_vdi_tab[pal[i].b];
		if (pixel_to_color)
		{
			vdi[3] = pixel_to_color[i];
			vdi += 4;
		} else
		{
			vdi += 3;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_vdi_to_rgbpal(PALETTE pal, _WORD *vdi, _WORD colors, _WORD components)
{
	_WORD i;
	_ULONG color;
	
	for (i = 0; i < colors && i < 256; i++)
	{
		color = vdi[i * components + 0];
		pal[i].r = (unsigned char)((color * 25599u) / 100000lu);
		color = vdi[i * components + 1];
		pal[i].g = (unsigned char)((color * 25599u) / 100000lu);
		color = vdi[i * components + 2];
		pal[i].b = (unsigned char)((color * 25599u) / 100000lu);
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_vdi_to_rgbcolor(_WORD vdi[3])
{
	_ULONG color;
	
	color = vdi[0];
	vdi[0] = (_WORD)((color * 25599u) / 100000lu);
	color = vdi[1];
	vdi[1] = (_WORD)((color * 25599u) / 100000lu);
	color = vdi[2];
	vdi[2] = (_WORD)((color * 25599u) / 100000lu);
}

/*** ---------------------------------------------------------------------- ***/

LOCAL void convinit(PALETTE pal)
{
	_WORD i;
	_WORD color;
	
	for (i = 0; i < 16; i++)
	{
		color = (pal[i].r >> 5) & 0x007;
		color += (pal[i].g >> 5) & 0x007;
		color += (pal[i].b >> 5) & 0x007;
		color = ((color * 3) & 0x3c) << 2;
		patptr1[i] = &pattern1[color];
		patptr2[i] = &pattern2[color];
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_16to2(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	_WORD lines;
	_WORD planes;
	_WORD words;
	_UBYTE p1, p2, p3, p4;
	_UBYTE *line0;
	_UBYTE *line1;
	_WORD dots;
	_WORD pix;
	const _UBYTE *pat;
	const _UBYTE *mask;

	convinit(pic->pi_palette);

	bytes <<= 1;
	pic->pi_width <<= 1;
	pic->pi_height = height << 1;
	pic->pi_planes = 1;
	line0 = dest;
	line1 = dest + bytes;
	for (lines = height, planes = 0; --lines >= 0; )
	{
		for (words = (bytes + 2) >> 2; --words >= 0; )
		{
			p1 = src[0];
			p2 = src[2];
			p3 = src[4];
			p4 = src[6];
			*line0 = 0;
			*line1 = 0;
			for (dots = 8, mask = masktab; --dots >= 0; )
			{
				pix = 0;
				if (p4 & 0x80) pix++;
				p4 += p4;
				pix += pix;
				if (p3 & 0x80) pix++;
				p3 += p3;
				pix += pix;
				if (p2 & 0x80) pix++;
				p2 += p2;
				pix += pix;
				if (p1 & 0x80) pix++;
				p1 += p1;
				pat = patptr1[pix] + planes;
				*line0 |= (*pat++ & *mask);
				*line1 |= (*pat & *mask++);
				if (dots == 4)
				{
					line0++;
					line1++;
					*line0 = 0;
					*line1 = 0;
				}
			}
			line0++;
			line1++;

			if (words != 0 || !(bytes & 2))
			{
				p1 = src[1];
				p2 = src[3];
				p3 = src[5];
				p4 = src[7];
				*line0 = 0;
				*line1 = 0;
				for (dots = 8, mask = masktab; --dots >= 0; )
				{
					pix = 0;
					if (p4 & 0x80) pix++;
					p4 += p4;
					pix += pix;
					if (p3 & 0x80) pix++;
					p3 += p3;
					pix += pix;
					if (p2 & 0x80) pix++;
					p2 += p2;
					pix += pix;
					if (p1 & 0x80) pix++;
					p1 += p1;
					pat = patptr2[pix] + planes;
					*line0 |= (*pat++ & *mask);
					*line1 |= (*pat & *mask++);
					if (dots == 4)
					{
						line0++;
						line1++;
						*line0 = 0;
						*line1 = 0;
					}
				}
				line0++;
				line1++;
			}

			src += 8;
		}

		line0 += bytes;
		line1 += bytes;
		planes += 2;
		if (planes == 16)
			planes = 0;
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_4to2(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	_WORD lines;
	_WORD planes;
	_WORD words;
	_WORD p1, p2;
	_UBYTE *line0;
	_UBYTE *line1;
	_UWORD mask;
	_WORD pix;
	const _UBYTE *pat;

	convinit(pic->pi_palette);

	pic->pi_height = height << 1;
	pic->pi_planes = 1;
	bytes = (bytes + 1) & ~1;
	line0 = dest;
	line1 = dest + bytes;
	for (lines = height, planes = 0; --lines >= 0; )
	{
		for (words = bytes >> 1; --words >= 0; )
		{
			p1 = src[0];
			p2 = src[2];
			*line0 = 0;
			*line1 = 0;
			for (mask = 0x80; mask != 0; mask >>= 1)
			{
				pix = 0;
				if (p2 & 0x80) pix++;
				p2 += p2;
				pix += pix;
				if (p1 & 0x80) pix++;
				p1 += p1;
				pat = patptr1[pix] + planes;
				*line0 |= (*pat++ & mask);
				*line1 |= (*pat & mask);
			}
			line0++;
			line1++;

			p1 = src[1];
			p2 = src[3];
			*line0 = 0;
			*line1 = 0;
			for (mask = 0x80; mask != 0; mask >>= 1)
			{
				pix = 0;
				if (p2 & 0x80) pix++;
				p2 += p2;
				pix += pix;
				if (p1 & 0x80) pix++;
				p1 += p1;
				pat = patptr2[pix] + planes;
				*line0 |= (*pat++ & mask);
				*line1 |= (*pat & mask);
			}
			line0++;
			line1++;
			src += 4;
		}
		line0 += bytes;
		line1 += bytes;
		planes += 2;
		if (planes == 16)
			planes = 0;
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_16to4(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	_WORD lines;
	_WORD planes;
	_WORD words;
	_UBYTE p1, p2, p3, p4;
	_UBYTE *line0;
	_WORD dots;
	_WORD pix;
	const _UBYTE *pat;
	const _UBYTE *mask;

	convinit(pic->pi_palette);
	pic->pi_palette[0].r = 0x00;
	pic->pi_palette[0].g = 0x00;
	pic->pi_palette[0].b = 0x00;
	pic->pi_palette[1].r = 0x80;
	pic->pi_palette[1].g = 0x80;
	pic->pi_palette[1].b = 0x80;
	pic->pi_palette[2].r = 0x80;
	pic->pi_palette[2].g = 0x80;
	pic->pi_palette[2].b = 0x80;
	pic->pi_palette[3].r = 0xe0;
	pic->pi_palette[3].g = 0xe0;
	pic->pi_palette[3].b = 0xe0;

	pic->pi_width <<= 1;
	pic->pi_planes = 2;

	line0 = dest;
	for (lines = height, planes = 0; --lines >= 0; )
	{
		for (words = bytes >> 1; --words >= 0; )
		{
			p1 = src[0];
			p2 = src[2];
			p3 = src[4];
			p4 = src[6];
			line0[0] = 0;
			line0[2] = 0;
			for (dots = 8, mask = masktab; --dots >= 0; )
			{
				pix = 0;
				if (p4 & 0x80) pix++;
				p4 += p4;
				pix += pix;
				if (p3 & 0x80) pix++;
				p3 += p3;
				pix += pix;
				if (p2 & 0x80) pix++;
				p2 += p2;
				pix += pix;
				if (p1 & 0x80) pix++;
				p1 += p1;
				pat = patptr1[pix] + planes;
				line0[0] |= (*pat++ & *mask);
				line0[2] |= (*pat & *mask++);
				if (dots == 4)
				{
					line0++;
					line0[0] = 0;
					line0[2] = 0;
				}
			}

			p1 = src[1];
			p2 = src[3];
			p3 = src[5];
			p4 = src[7];
			line0 += 3;
			line0[0] = 0;
			line0[2] = 0;
			for (dots = 8, mask = masktab; --dots >= 0; )
			{
				pix = 0;
				if (p4 & 0x80) pix++;
				p4 += p4;
				pix += pix;
				if (p3 & 0x80) pix++;
				p3 += p3;
				pix += pix;
				if (p2 & 0x80) pix++;
				p2 += p2;
				pix += pix;
				if (p1 & 0x80) pix++;
				p1 += p1;
				pat = patptr2[pix] + planes;
				line0[0] |= (*pat++ & *mask);
				line0[2] |= (*pat & *mask++);
				if (dots == 4)
				{
					line0++;
					line0[0] = 0;
					line0[2] = 0;
				}
			}
			line0 += 3;
			src += 8;
		}
		planes += 2;
		if (planes == 16)
			planes = 0;
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_2to16(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	_WORD words;
	_UBYTE p1;
	_UBYTE *line0;
	_UBYTE *line1;
	_UBYTE pix1, pix2;
	_WORD mask;

	convinit(pic->pi_palette);
	pic_getpalette(pic->pi_palette, &palet1);
	bytes = (bytes + 1) & ~1;
	line0 = src;
	line1 = src + bytes;

	height >>= 1;
	bytes >>= 1;
	pic->pi_height = height;
	pic->pi_width >>= 1;
	pic->pi_planes = 4;
	for (; height > 0; height--)
	{
		for (words = bytes; --words >= 0; )
		{
			pix1 = pix2 = 0;
			p1 = *line0++;
			for (mask = 4; --mask >= 0; )
			{
				pix1 += pix1;
				if (p1 & 0x80) pix1++;
				p1 += p1;
				pix2 += pix2;
				if (p1 & 0x80) pix2++;
				p1 += p1;
			}
			p1 = *line0++;
			for (mask = 4; --mask >= 0; )
			{
				pix1 += pix1;
				if (p1 & 0x80) pix1++;
				p1 += p1;
				pix2 += pix2;
				if (p1 & 0x80) pix2++;
				p1 += p1;
			}
			dest[0] = pix1;
			dest[2] = pix2;
			pix1 = pix2 = 0;
			if (words != 0)
			{
				p1 = *line0++;
				for (mask = 4; --mask >= 0; )
				{
					pix1 += pix1;
					if (p1 & 0x80) pix1++;
					p1 += p1;
					pix2 += pix2;
					if (p1 & 0x80) pix2++;
					p1 += p1;
				}
				p1 = *line0++;
				for (mask = 4; --mask >= 0; )
				{
					pix1 += pix1;
					if (p1 & 0x80) pix1++;
					p1 += p1;
					pix2 += pix2;
					if (p1 & 0x80) pix2++;
					p1 += p1;
				}
			}
			dest[1] = pix1;
			dest[3] = pix2;
			dest += 4;

			pix1 = pix2 = 0;
			p1 = *line1++;
			for (mask = 4; --mask >= 0; )
			{
				pix1 += pix1;
				if (p1 & 0x80) pix1++;
				p1 += p1;
				pix2 += pix2;
				if (p1 & 0x80) pix2++;
				p1 += p1;
			}
			p1 = *line1++;
			for (mask = 4; --mask >= 0; )
			{
				pix1 += pix1;
				if (p1 & 0x80) pix1++;
				p1 += p1;
				pix2 += pix2;
				if (p1 & 0x80) pix2++;
				p1 += p1;
			}
			dest[0] = pix1;
			dest[2] = pix2;
			pix1 = pix2 = 0;
			if (words != 0)
			{
				--words;
				p1 = *line1++;
				for (mask = 4; --mask >= 0; )
				{
					pix1 += pix1;
					if (p1 & 0x80) pix1++;
					p1 += p1;
					pix2 += pix2;
					if (p1 & 0x80) pix2++;
					p1 += p1;
				}
				p1 = *line1++;
				for (mask = 4; --mask >= 0; )
				{
					pix1 += pix1;
					if (p1 & 0x80) pix1++;
					p1 += p1;
					pix2 += pix2;
					if (p1 & 0x80) pix2++;
					p1 += p1;
				}
			}
			dest[1] = pix1;
			dest[3] = pix2;
			dest += 4;
		}
		line0 = line0 + bytes + bytes;
		line1 = line1 + bytes + bytes;
	}

}

/*** ---------------------------------------------------------------------- ***/

void pic_2to4(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	_WORD lines;
	_WORD words;
	_UBYTE *line0;
	_UBYTE *line1;

	convinit(pic->pi_palette);
	pic->pi_palette[0].r = 0xe0;
	pic->pi_palette[0].g = 0xe0;
	pic->pi_palette[0].b = 0xe0;
	pic->pi_palette[1].r = 0x80;
	pic->pi_palette[1].g = 0x80;
	pic->pi_palette[1].b = 0x80;
	pic->pi_palette[2].r = 0x80;
	pic->pi_palette[2].g = 0x80;
	pic->pi_palette[2].b = 0x80;
	pic->pi_palette[3].r = 0x00;
	pic->pi_palette[3].g = 0x00;
	pic->pi_palette[3].b = 0x00;

	height >>= 1;
	pic->pi_height = height;
	pic->pi_planes = 2;
	bytes = (bytes + 1) & ~1;
	line0 = src;
	line1 = line0 + bytes;
	for (lines = height; --lines >= 0; )
	{
		for (words = bytes >> 1; --words >= 0; )
		{
			*dest++ = *line0++;
			*dest++ = *line0++;
			*dest++ = *line1++;
			*dest++ = *line1++;
		}
		line0 += bytes;
		line1 += bytes;
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_4to16(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	_WORD lines;
	_WORD words;
	_UBYTE p1, p2, p3, p4;
	_WORD dots;
	_WORD pix1, pix2;

	convinit(pic->pi_palette);

	pic->pi_width >>= 1;
	pic->pi_planes = 4;
	bytes >>= 1;
	for (lines = height; --lines >= 0; )
	{
		for (words = bytes >> 1; --words >= 0; )
		{
			p1 = p2 = p3 = p4 = 0;
			pix2 = src[0];
			pix1 = src[2];
			for (dots = 4; --dots >= 0; )
			{
				p1 += p1;
				if (pix1 & 0x80) p1++;
				pix1 += pix1;
				p2 += p2;
				if (pix2 & 0x80) p2++;
				pix2 += pix2;
				p3 += p3;
				if (pix1 & 0x80) p3++;
				pix1 += pix1;
				p4 += p4;
				if (pix2 & 0x80) p4++;
				pix2 += pix2;
			}
			pix2 = src[1];
			pix1 = src[3];
			for (dots = 4; --dots >= 0; )
			{
				p1 += p1;
				if (pix1 & 0x80) p1++;
				pix1 += pix1;
				p2 += p2;
				if (pix2 & 0x80) p2++;
				pix2 += pix2;
				p3 += p3;
				if (pix1 & 0x80) p3++;
				pix1 += pix1;
				p4 += p4;
				if (pix2 & 0x80) p4++;
				pix2 += pix2;
			}
			dest[0] = p1;
			dest[2] = p2;
			dest[4] = p3;
			dest[6] = p4;

			p1 = p2 = p3 = p4 = 0;
			pix2 = src[4];
			pix1 = src[6];
			for (dots = 4; --dots >= 0; )
			{
				p1 += p1;
				if (pix1 & 0x80) p1++;
				pix1 += pix1;
				p2 += p2;
				if (pix2 & 0x80) p2++;
				pix2 += pix2;
				p3 += p3;
				if (pix1 & 0x80) p3++;
				pix1 += pix1;
				p4 += p4;
				if (pix2 & 0x80) p4++;
				pix2 += pix2;
			}
			pix2 = src[5];
			pix1 = src[7];
			for (dots = 4; --dots >= 0; )
			{
				p1 += p1;
				if (pix1 & 0x80) p1++;
				pix1 += pix1;
				p2 += p2;
				if (pix2 & 0x80) p2++;
				pix2 += pix2;
				p3 += p3;
				if (pix1 & 0x80) p3++;
				pix1 += pix1;
				p4 += p4;
				if (pix2 & 0x80) p4++;
				pix2 += pix2;
			}
			dest[1] = p1;
			dest[3] = p2;
			dest[5] = p3;
			dest[7] = p4;

			src += 8;
			dest += 8;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_getpalette(PALETTE pal, const TOSPALETTE *tospal)
{
	_WORD i;

	for (i = 0; i < 16; i++)
	{
		pal[i].r = ((((*tospal)[i][0]     ) & 0x07) << 5) | ((((*tospal)[i][0] >> 3) & 0x01) << 4);
		pal[i].g = ((((*tospal)[i][1] >> 4) & 0x07) << 5) | ((((*tospal)[i][1] >> 7) & 0x01) << 4);
		pal[i].b = ((((*tospal)[i][1]     ) & 0x07) << 5) | ((((*tospal)[i][1] >> 3) & 0x01) << 4);
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_setpalette(TOSPALETTE tospal, PALETTE pal)
{
	_WORD i;

	for (i = 0; i < 16; i++)
	{
		tospal[i][0] = (((pal[i].r >> 5) & 0x07)     ) | (((pal[i].r >> 4) & 0x01) << 3);
		tospal[i][1] = (((pal[i].g >> 5) & 0x07) << 4) | (((pal[i].g >> 4) & 0x01) << 7) |
					   (((pal[i].b >> 5) & 0x07)     ) | (((pal[i].b >> 4) & 0x01) << 3);
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_stdpalette(PALETTE pal, _WORD planes)
{
	_WORD i;
	_WORD colors;
	
	if (planes > 8)
		planes = 8;
	colors = 1 << planes;
	for (i = 0; i < colors; i++)
		pal[i] = std256_palette[i];
	if (planes < 8)
	{
		pal[colors - 1] = std256_palette[255];
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_savepalette(PALETTE pal)
{
#if defined(__TOS__) || defined(__atarist__)
	TOSPALETTE tospal;
	_WORD *palp = (_WORD *)(tospal);
	int i;
	
	for (i = 0; i < 16; i++)
		palp[i] = Setcolor(i, -1);
	pic_getpalette(pal, (const TOSPALETTE *)&tospal);
#else
	pic_stdpalette(pal, 1);
#endif
}

/*** ---------------------------------------------------------------------- ***/

void pic_showpalette(PALETTE pal)
{
#if defined(__TOS__) || defined(__atarist__)
	TOSPALETTE tospal;
	int i;
	_WORD *palp = (_WORD *)(tospal);
	
	pic_setpalette(tospal, pal);
	for (i = 0; i < 16; i++)
		(void) Setcolor(i, palp[i]);
	Vsync();
	pic_getpalette(pal, (const TOSPALETTE *)&tospal);
#else
	UNUSED(pal);
#endif
}

/*** ---------------------------------------------------------------------- ***/

void pic_invert(_UBYTE *buf, _LONG size)
{
	while (--size >= 0)
	{
		*buf = ~(*buf);
		buf++;
	}
}

/*** ---------------------------------------------------------------------- ***/

_WORD pic_calcrez(_WORD planes)
{
	return planes == 4 ? 0 : planes == 2 ? 1 : planes == 8 ? 3 : 2;
}

/*** ---------------------------------------------------------------------- ***/

_WORD pic_calcplanes(_WORD rez)
{
	return rez == 0 ? 4 : rez == 1 ? 2 : rez == 3 ? 8 : 1;
}

/*** ---------------------------------------------------------------------- ***/

_LONG pic_rowsize(PICTURE *pic)
{
	return (_LONG)tobyte(pic->pi_width) * (_LONG)pic->pi_planes;
}

/*** ---------------------------------------------------------------------- ***/

void pic_calcsize(PICTURE *pic)
{
	_LONG bytes = pic_rowsize(pic);
	
	pic->pi_bytes = tobyte(pic->pi_width);
	pic->pi_picsize = bytes * (_LONG)pic->pi_height;
	
	pic->pi_fdb.fd_nplanes = pic->pi_planes;
	pic->pi_fdb.fd_w = pic->pi_width;
	pic->pi_fdb.fd_h = pic->pi_height;
	pic->pi_fdb.fd_wdwidth = toword(pic->pi_width);
	pic->pi_fdb.fd_stand = 0;
	pic->pi_fdb.fd_r1 = 0;
	pic->pi_fdb.fd_r2 = 0;
	pic->pi_fdb.fd_r3 = 0;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Some formats (like GIF) can have some unusual number of colors.
 * We want do deal only with monochrome, 16 and 256 colors.
 */
void pic_normal_planes(PICTURE *pic)
{
	switch (pic->pi_planes)
	{
	case 1:
		break;
	case 2:
		if (pic->pi_type != FT_DEGAS_MED && pic->pi_type != FT_IMAGIC_MED)
			pic->pi_planes = 4;
		break;
	case 3:
	case 4:
		pic->pi_planes = 4;
		break;
	case 15:
	case 16:
		pic->pi_planes = 16;
		break;
	case 24:
	case 32:
		break;
	default:
		pic->pi_planes = 8;
		break;
	}
	pic_calcsize(pic);
}

/*** ---------------------------------------------------------------------- ***/

void pic_stdsize(PICTURE *pic)
{
	LOCAL _WORD const maxwidth[4] = { 320, 640, 640, 640 };
	LOCAL _WORD const maxheight[4] = { 200, 200, 400, 480 };
	_WORD rez;

	rez = pic_calcrez(pic->pi_planes);
	pic->pi_orig_planes = pic->pi_planes;
	pic->pi_width = pic->pi_orig_width = maxwidth[rez];
	pic->pi_height = pic->pi_orig_height = maxheight[rez];
	pic_calcsize(pic);
}

/*** ---------------------------------------------------------------------- ***/

LOCAL gboolean is_palette(const TOSPALETTE pal)
{
	_WORD i;

	for (i = 0; i < 16; i++)
	{
		if ((pal[i][0] & 0x08) != 0)
			return TRUE;
		if ((pal[i][1] & 0x88) != 0)
			return TRUE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

typedef struct {                /* Header fuer Neochrome-Bilder */
	_UBYTE stuff1[4];
	TOSPALETTE palette;         /* Farbpalette */
	_UBYTE stuff2[92];
} NEO_HEADER;

gboolean pic_type_neo(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	const NEO_HEADER *header = (const NEO_HEADER *)buf;

	if (pic->pi_filesize >= 32128l &&
		pic->pi_filesize <= 32768l &&
		size >= 128 &&
		is_palette(header->palette))
	{
		pic->pi_type = FT_NEO;
		pic->pi_planes = 4;
		pic_stdsize(pic);
		pic->pi_datasize = size - sizeof(NEO_HEADER);
		pic->pi_dataoffset = sizeof(NEO_HEADER);
		pic_getpalette(pic->pi_palette, &header->palette);
		pic->pi_compressed = FALSE;
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

_LONG neo_header(_UBYTE *buf, PICTURE *pic)
{
	NEO_HEADER *header = (NEO_HEADER *)buf;

	memset(buf, 0, NEO_HEADER_SIZE);
	pic_setpalette(header->palette, pic->pi_palette);
	pic->pi_datasize = 32000;
	return NEO_HEADER_SIZE;
}

/*** ---------------------------------------------------------------------- ***/

typedef struct {                /* Header fuer Colorstar-Bilder */
	TOSPALETTE palette;         /* Farbpalette */
} COLOR_HEADER;

gboolean pic_type_colorstar(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	const COLOR_HEADER *header = (const COLOR_HEADER *)buf;

	if (pic->pi_filesize == 32032l &&
		size > 32 &&
		is_palette(header->palette))
	{
		pic->pi_type = FT_COLSTAR;
		pic->pi_planes = 4;
		pic_stdsize(pic);
		pic_getpalette(pic->pi_palette, &header->palette);
		pic->pi_datasize = pic->pi_filesize - COLOR_HEADER_SIZE;
		pic->pi_dataoffset = COLOR_HEADER_SIZE;
		pic->pi_compressed = FALSE;
		return TRUE;
	}
	return FALSE;
}


_LONG colstar_header(_UBYTE *buf, PICTURE *pic)
{
	COLOR_HEADER *header = (COLOR_HEADER *)buf;

	pic_setpalette(header->palette, pic->pi_palette);
	pic->pi_datasize = 32000l;
	return COLOR_HEADER_SIZE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_monostar(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	UNUSED(buf);
	UNUSED(size);
	if (pic->pi_filesize == 32000l)
	{
		pic->pi_type = FT_SCREEN;
		pic->pi_planes = 1;
		pic_stdsize(pic);
		pic->pi_datasize = size;
		pic->pi_dataoffset = 0;
		pic->pi_compressed = FALSE;
		return TRUE;
	}
	return FALSE;
}

_LONG monostar_header(_UBYTE *buf, PICTURE *pic)
{
	UNUSED(buf);
	pic->pi_datasize = 32000l;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

void pic_planes_to_interleaved(_UBYTE *dst, _UBYTE *src, PICTURE *pic)
{
	long planesize;
	_UWORD *row1, *row2, *row3, *row4, *row5, *row6, *row7, *row8;
	_UWORD *p;
	_WORD x, y;
	
	planesize = (_LONG)pic->pi_height * toword(pic->pi_width);
	p = (_UWORD *)dst;
	row1 = (_UWORD *)src;
	
	switch (pic->pi_planes)
	{
	case 1:
		memcpy(dst, src, planesize << 1);
		break;
	case 2:
		row2 = row1 + planesize;
		for (y = 0; y < pic->pi_height; y++)
		{
			for (x = 0; x < pic->pi_width; x += 16)
			{
				*p++ = *row1++;
				*p++ = *row2++;
			}
		}
		break;
	case 4:
		row2 = row1 + planesize;
		row3 = row2 + planesize;
		row4 = row3 + planesize;
		for (y = 0; y < pic->pi_height; y++)
		{
			for (x = 0; x < pic->pi_width; x += 16)
			{
				*p++ = *row1++;
				*p++ = *row2++;
				*p++ = *row3++;
				*p++ = *row4++;
			}
		}
		break;
	case 8:
		row2 = row1 + planesize;
		row3 = row2 + planesize;
		row4 = row3 + planesize;
		row5 = row4 + planesize;
		row6 = row5 + planesize;
		row7 = row6 + planesize;
		row8 = row7 + planesize;
		for (y = 0; y < pic->pi_height; y++)
		{
			for (x = 0; x < pic->pi_width; x += 16)
			{
				*p++ = *row1++;
				*p++ = *row2++;
				*p++ = *row3++;
				*p++ = *row4++;
				*p++ = *row5++;
				*p++ = *row6++;
				*p++ = *row7++;
				*p++ = *row8++;
			}
		}
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void pic_interleaved_to_planes(unsigned char *dst, const unsigned char *src, _WORD width, _WORD height, _WORD planes)
{
	long planesize;
	_UWORD *row1, *row2, *row3, *row4, *row5, *row6, *row7, *row8;
	const _UWORD *p;
	_WORD x, y;
	
	planesize = (_LONG)height * toword(width);
	p = (const _UWORD *)src;
	row1 = (_UWORD *)dst;
	
	switch (planes)
	{
	case 1:
		memcpy(dst, src, planesize << 1);
		break;
	case 2:
		row2 = row1 + planesize;
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x += 16)
			{
				*row1++ = *p++;
				*row2++ = *p++;
			}
		}
		break;
	case 4:
		row2 = row1 + planesize;
		row3 = row2 + planesize;
		row4 = row3 + planesize;
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x += 16)
			{
				*row1++ = *p++;
				*row2++ = *p++;
				*row3++ = *p++;
				*row4++ = *p++;
			}
		}
		break;
	case 8:
		row2 = row1 + planesize;
		row3 = row2 + planesize;
		row4 = row3 + planesize;
		row5 = row4 + planesize;
		row6 = row5 + planesize;
		row7 = row6 + planesize;
		row8 = row7 + planesize;
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x += 16)
			{
				*row1++ = *p++;
				*row2++ = *p++;
				*row3++ = *p++;
				*row4++ = *p++;
				*row5++ = *p++;
				*row6++ = *p++;
				*row7++ = *p++;
				*row8++ = *p++;
			}
		}
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

pic_filetype pic_type(PICTURE *pic, _UBYTE *buf, _LONG size)
{
	pic_filetype type;

	type = FT_UNKNOWN;
	pic->pi_filesize = size;
	if (pic_type_iff(pic, buf, size) ||
		pic_type_gif(pic, buf, size) ||
		pic_type_img(pic, buf, size) ||
		pic_type_imagic(pic, buf, size) ||
		pic_type_stad(pic, buf, size) ||
		pic_type_colorstar(pic, buf, size) ||
		pic_type_bmp(pic, buf, size) ||
		pic_type_ico(pic, buf, size) ||
		pic_type_monostar(pic, buf, size) ||
		pic_type_degas(pic, buf, size) ||
		pic_type_neo(pic, buf, size) ||
		pic_type_icn(pic, buf, size) ||
		pic_type_png(pic, buf, size))
	{
		type = pic->pi_type;
	} else
	{
	}

	pic->pi_orig_planes = pic->pi_planes;
	pic->pi_orig_width = pic->pi_width;
	pic->pi_orig_height = pic->pi_height;
	return type;
}

/*** ---------------------------------------------------------------------- ***/

void pic_init(PICTURE *pic)
{
	memset(pic, 0, sizeof(*pic));
	pic->pi_planes = 1;
	pic_stdsize(pic);
	pic_savepalette(pic->pi_palette);
	pic->pi_datasize = 0;
	pic->pi_compressed = 0;
	pic->pi_pat_len = 2;
	pic->pi_dataoffset = 0;
	pic->pi_version = 1;
	pic->pi_pix_width = 0;
	pic->pi_pix_height = 0;
	pic->pi_serial = 0;
	pic->pi_type = FT_UNKNOWN;
	pic->pi_filesize = 0;

	pic->pi_snap.g_x =
	pic->pi_snap.g_y =
	pic->pi_snap.g_w =
	pic->pi_snap.g_h = 0;
	pic->pi_buf = NULL;
	pic->pi_active = FALSE;
	pic->pi_unsupported = FALSE;
	pic->pi_topdown = TRUE;
#ifdef IN_ORCS
	pic->pi_name[0] = '\0';
#else
	pic->pi_name = NULL;
#endif
}


void pic_free(PICTURE *pic)
{
	if (pic)
	{
		g_free(pic->pi_buf),
		pic->pi_buf = NULL;
	}
}


void pic_256to2(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	UNUSED(dest);
	UNUSED(src);
	UNUSED(pic);
	UNUSED(bytes);
	UNUSED(height);
}
void pic_256to4(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	UNUSED(dest);
	UNUSED(src);
	UNUSED(pic);
	UNUSED(bytes);
	UNUSED(height);
}
void pic_256to16(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	UNUSED(dest);
	UNUSED(src);
	UNUSED(pic);
	UNUSED(bytes);
	UNUSED(height);
}
void pic_16to256(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	UNUSED(dest);
	UNUSED(src);
	UNUSED(pic);
	UNUSED(bytes);
	UNUSED(height);
}
void pic_4to256(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	UNUSED(dest);
	UNUSED(src);
	UNUSED(pic);
	UNUSED(bytes);
	UNUSED(height);
}
void pic_2to256(_UBYTE *dest, _UBYTE *src, PICTURE *pic, _WORD bytes, _WORD height)
{
	UNUSED(dest);
	UNUSED(src);
	UNUSED(pic);
	UNUSED(bytes);
	UNUSED(height);
}
