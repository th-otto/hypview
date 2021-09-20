#include "hv_defs.h"
#include "picture.h"
#include "w_draw.h"

static _WORD screen_w, screen_h;
static _WORD screen_colors;
static _WORD screen_planes;

#if 0
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

static Pixel const std_palette[256] = {
	 /* white		  */ 0xffffffff,
	 /* black		  */ 0xff000000,
	 /* red           */ 0xffff0000,
	 /* green		  */ 0xff00ff00,
	 /* blue		  */ 0xff0000ff,
	 /* cyan		  */ 0xff00ffff,
	 /* yellow		  */ 0xffffff00,
	 /* magenta       */ 0xffff00ff,
	 /* light gray	  */ 0xffcccccc,
	 /* dark gray	  */ 0xff888888,
	 /* dark red	  */ 0xff880000,
	 /* dark green    */ 0xff008800,
	 /* dark blue	  */ 0xff000088,
	 /* dark cyan	  */ 0xff008888,
	 /* dark yellow   */ 0xff888800,
	 /* dark magenta  */ 0xff880088,
	0xfffdfdfd,
	0xffececec,
	0xffdbdbdb,
	0xffcacaca,
	0xffb9b9b9,
	0xffa8a8a8,
	0xff979797,
	0xff878787,
	0xff767676,
	0xff656565,
	0xff545454,
	0xff434343,
	0xff323232,
	0xff212121,
	0xff101010,
	0xff000000,
	0xfffd0000,
	0xfffd0010,
	0xfffd0021,
	0xfffd0032,
	0xfffd0043,
	0xfffd0054,
	0xfffd0065,
	0xfffd0076,
	0xfffd0087,
	0xfffd0097,
	0xfffd00a8,
	0xfffd00b9,
	0xfffd00ca,
	0xfffd00db,
	0xfffd00ec,
	0xfffd00fd,
	0xffec00fd,
	0xffdb00fd,
	0xffca00fd,
	0xffb900fd,
	0xffa800fd,
	0xff9700fd,
	0xff8700fd,
	0xff7600fd,
	0xff6500fd,
	0xff5400fd,
	0xff4300fd,
	0xff3200fd,
	0xff2100fd,
	0xff1000fd,
	0xff0000fd,
	0xff0010fd,
	0xff0021fd,
	0xff0032fd,
	0xff0043fd,
	0xff0054fd,
	0xff0065fd,
	0xff0076fd,
	0xff0087fd,
	0xff0097fd,
	0xff00a8fd,
	0xff00b9fd,
	0xff00cafd,
	0xff00dbfd,
	0xff00ecfd,
	0xff00fdfd,
	0xff00fdec,
	0xff00fddb,
	0xff00fdca,
	0xff00fdb9,
	0xff00fda8,
	0xff00fd97,
	0xff00fd87,
	0xff00fd76,
	0xff00fd65,
	0xff00fd54,
	0xff00fd43,
	0xff00fd32,
	0xff00fd21,
	0xff00fd10,
	0xff00fd00,
	0xff10fd00,
	0xff21fd00,
	0xff32fd00,
	0xff43fd00,
	0xff54fd00,
	0xff65fd00,
	0xff76fd00,
	0xff87fd00,
	0xff97fd00,
	0xffa8fd00,
	0xffb9fd00,
	0xffcafd00,
	0xffdbfd00,
	0xffecfd00,
	0xfffdfd00,
	0xfffdec00,
	0xfffddb00,
	0xfffdca00,
	0xfffdb900,
	0xfffda800,
	0xfffd9700,
	0xfffd8700,
	0xfffd7600,
	0xfffd6500,
	0xfffd5400,
	0xfffd4300,
	0xfffd3200,
	0xfffd2100,
	0xfffd1000,
	0xffb90000,
	0xffb90010,
	0xffb90021,
	0xffb90032,
	0xffb90043,
	0xffb90054,
	0xffb90065,
	0xffb90076,
	0xffb90087,
	0xffb90097,
	0xffb900a8,
	0xffb900b9,
	0xffa800b9,
	0xff9700b9,
	0xff8700b9,
	0xff7600b9,
	0xff6500b9,
	0xff5400b9,
	0xff4300b9,
	0xff3200b9,
	0xff2100b9,
	0xff1000b9,
	0xff0000b9,
	0xff0010b9,
	0xff0021b9,
	0xff0032b9,
	0xff0043b9,
	0xff0054b9,
	0xff0065b9,
	0xff0076b9,
	0xff0087b9,
	0xff0097b9,
	0xff00a8b9,
	0xff00b9b9,
	0xff00b9a8,
	0xff00b997,
	0xff00b987,
	0xff00b976,
	0xff00b965,
	0xff00b954,
	0xff00b943,
	0xff00b932,
	0xff00b921,
	0xff00b910,
	0xff00b900,
	0xff10b900,
	0xff21b900,
	0xff32b900,
	0xff43b900,
	0xff54b900,
	0xff65b900,
	0xff76b900,
	0xff87b900,
	0xff97b900,
	0xffa8b900,
	0xffb9b900,
	0xffb9a800,
	0xffb99700,
	0xffb98700,
	0xffb97600,
	0xffb96500,
	0xffb95400,
	0xffb94300,
	0xffb93200,
	0xffb92100,
	0xffb91000,
	0xff760000,
	0xff760010,
	0xff760021,
	0xff760032,
	0xff760043,
	0xff760054,
	0xff760065,
	0xff760076,
	0xff650076,
	0xff540076,
	0xff430076,
	0xff320076,
	0xff210076,
	0xff100076,
	0xff000076,
	0xff001076,
	0xff002176,
	0xff003276,
	0xff004376,
	0xff005476,
	0xff006576,
	0xff007676,
	0xff007665,
	0xff007654,
	0xff007643,
	0xff007632,
	0xff007621,
	0xff007610,
	0xff007600,
	0xff107600,
	0xff217600,
	0xff327600,
	0xff437600,
	0xff547600,
	0xff657600,
	0xff767600,
	0xff766500,
	0xff765400,
	0xff764300,
	0xff763200,
	0xff762100,
	0xff761000,
	0xff430000,
	0xff430010,
	0xff430021,
	0xff430032,
	0xff430043,
	0xff320043,
	0xff210043,
	0xff100043,
	0xff000043,
	0xff001043,
	0xff002143,
	0xff003243,
	0xff004343,
	0xff004332,
	0xff004321,
	0xff004310,
	0xff004300,
	0xff104300,
	0xff214300,
	0xff324300,
	0xff434300,
	0xff433200,
	0xff432100,
	0xff431000,
	0xfffdfdfd,
	0xff000000
};
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void init_screen(void)
{
	if (screen_w == 0)
	{
		NSScreen *screen;
		NSRect rect;
		NSArray *screens;
		
		screens = [NSScreen screens];
		screen = [NSScreen mainScreen];
		rect = [screen frame];
		screen_w = rect.size.width;
		screen_h = rect.size.height;
		screen_planes = NSBitsPerPixelFromDepth([screen depth]);
		if (screen_planes >= 15)
			screen_colors = 32767;
		else
			screen_colors = 1 << screen_planes;
	}
}

/*** ---------------------------------------------------------------------- ***/

_WORD GetNumPlanes(void)
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

_WORD GetScreenHeight(void)
{
	init_screen();
	return screen_h;
}

/*** ---------------------------------------------------------------------- ***/

_WORD GetScreenWidth(void)
{
	init_screen();
	return screen_w;
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
	CGImageRelease((CGImageRef)map->bm_info);
	g_free(map);
}

/*** ---------------------------------------------------------------------- ***/

gboolean W_Fix_Bitmap(void **pdata, _WORD w, _WORD h, _WORD nplanes)
{
	IBITMAP *map;
	
	if (pdata != NULL && *pdata != NULL)
	{
		map = (IBITMAP *)(*pdata);
		if (map->bm_magic == BITMAP_MAGIC)
		{
			hyp_debug("W_Fix_Bitmap: already fixed");
			return FALSE;
		}
		map = g_new(IBITMAP, 1);
		if (map != NULL)
		{
			_UBYTE *src = (_UBYTE *)(*pdata);

			map->bm_orig_data = src;
			map->bm_info = 0;
			map->bm_data = 0;
			map->bm_magic = BITMAP_MAGIC;
			*pdata = map;
			/* return value indicates whether data was converted inplace */
			return FALSE;
		}
	}
	return FALSE;
}
