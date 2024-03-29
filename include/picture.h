/*****************************************************************************
 * PICTURE.H
 *****************************************************************************/

#ifndef __PICTURE_H__
#define __PICTURE_H__

#ifndef __PORTAB_H__
#  include "portab.h"
#endif

#ifdef IN_ORCS
#include <gem.h>
#include <w_draw.h>
#include <debug.h>
#else
#include "hypdefs.h"
#include "hypdebug.h"
#endif

EXTERN_C_BEG


typedef unsigned char TOSPALETTE[16][2];
typedef unsigned char TOSPALETTE256[256][2];
struct _rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
typedef struct _rgb PALETTE[256];

#if !defined(__MFDB) && !defined(__MFDB__)
#define __MFDB
#define __MFDB__
typedef struct
{
	void	*fd_addr;
	_WORD	fd_w;
	_WORD	fd_h;
	_WORD	fd_wdwidth;
	_WORD	fd_stand;
	_WORD	fd_nplanes;
	_WORD	fd_r1;
	_WORD	fd_r2;
	_WORD	fd_r3;
} MFDB;
#endif


/*
 * file types
 */
typedef enum pic_filetype {
	FT_UNKNOWN = 0,

	/* executables */
	FT_EXEC_FIRST = 100,
	FT_EXEC,
	FT_TOS,
	FT_TTP,
	FT_PRG,
	FT_GTP,
	FT_EXEC_LAST,
	
	/* pictures */
	FT_PICTURE_FIRST = 200,
	FT_DEGAS_LOW,
	FT_DEGAS_MED,
	FT_DEGAS_HIGH,
	FT_NEO,
	FT_IFF,
	FT_COLSTAR,
	FT_IMG,
	FT_STAD,
	FT_IMAGIC_LOW,
	FT_IMAGIC_MED,
	FT_IMAGIC_HIGH,
	FT_SCREEN,
	FT_ICO,
	FT_CALAMUS_PAGE,
	FT_BMP,
	FT_GIF,
	FT_TIFF,
	FT_TARGA,
	FT_PBM,
	FT_ICN,
	FT_PNG,
	FT_PICTURE_LAST,
	
	/* archives */
	FT_ARCHIVE_FIRST = 300,
	FT_ARC,
	FT_ZOO,
	FT_LZH,
	FT_ZIP,
	FT_ARJ,
	FT_TAR,
	FT_GZ,
	FT_BZ2,
	FT_ARCHIVE_LAST,
	
	/* documents */
	FT_DOC_FIRST = 400,
	FT_ASCII,
	FT_WORDPLUS,
	FT_SIGDOC,
	FT_DOC_LAST,
	
	/* fonts */
	FT_FONT_FIRST = 500,
	FT_GEMFNT,
	FT_SIGFNT,
	FT_FONT_LAST,
	
	/* others */
	FT_MISC_FIRST = 1000,
	FT_EMPTY,
	FT_DRI,
	FT_DRILIB,
	FT_BOBJECT,
	FT_RSC,
	FT_GFA2,
	FT_GFA3
} pic_filetype;


/*
 * Header for all supported image formats
 */
typedef struct {
    PALETTE pi_palette;         /* color palette */
    _WORD pi_planes;            /* # of planes/bits per pixel */
    _WORD pi_width;             /* # of pixel per row */
    _WORD pi_height;            /* # of rows */
    _WORD pi_bytes;             /* # of bytes per row */
    long pi_datasize;           /* size of compressed data */
    _WORD pi_compressed;        /* compression flag */
    _WORD pi_pat_len;           /* pattern length in bytes */
    long pi_picsize;			/* size of uncompressed data */
	long pi_dataoffset;         /* offset to data in file */
    long pi_version;            /* version number */
    _WORD pi_pix_width;         /* Pixel width of source device in 1/1000 mm */
    _WORD pi_pix_height;        /* Pixel height of source device in 1/1000 mm */
    long pi_serial;             /* serial number */
	pic_filetype pi_type;		/* picture type, for info */
	long pi_filesize;			/* size of file, for info */

	_WORD pi_orig_planes;		/* original value before display conversion */
	_WORD pi_orig_width;		/* original value before display conversion */
	_WORD pi_orig_height;		/* original value before display conversion */

	GRECT pi_snap;				/* snapshot position */
	unsigned char *pi_buf;		/* data buffer */
	MFDB pi_fdb;				/* form descriptor block */
	gboolean pi_active;			/* TRUE if already in core */
	gboolean pi_unsupported;	/* TRUE if valid, but unsupported format */
	gboolean pi_topdown;		/* TRUE if first row of data is displayed at top */
	int pi_transparent;			/* value of transparent pixel, -1 if none */
#ifdef IN_ORCS
	char pi_name[PATH_MAX+1];	/* path name */
#else
	char *pi_name;				/* path name */
#endif
} PICTURE;


#if defined(__PUREC__)
static unsigned long ulmul(unsigned short x, unsigned short y) 0xc0c1;
static long lmul(short x, short y) 0xc1c1;
#elif defined(__GNUC__) && defined(__mc68000__)
static __inline unsigned long ulmul(unsigned short x, unsigned short y)
{
	unsigned long z;

	__asm__ __volatile(
		" mulu.w %1,%0"
	: "=d"(z)
	: "d"(y), "0"(x)
	: "cc");
	return z;
}
static __inline long lmul(short x, short y)
{
	long z;

	__asm__ __volatile(
		" muls.w %1,%0"
	: "=d"(z)
	: "d"(y), "0"(x)
	: "cc");
	return z;
}
#else
#define ulmul(x, y) ((unsigned long)(x) * (unsigned long)(y))
#define lmul(x, y) ((long)(x) * (long)(y))
#endif


extern PALETTE const std256_palette;

/*** GEM IMG -------------------------------------------------------------- ***/

#define GEM_HEADER_SIZE 16 /* sizeof(GEM_HEADER) */
#define GEM_HEADER_BUFSIZE (GEM_HEADER_SIZE + 6 + 256 * 3 * sizeof(_WORD))

gboolean pic_type_img(PICTURE *pic, const unsigned char *buf, long size);
long img_header(unsigned char **dest_p, PICTURE *pic);

gboolean img_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic);
gboolean img_unpack_safe(unsigned char *dest, const unsigned char *src, PICTURE *pic);
long img_pack(unsigned char *dest, const unsigned char *src, PICTURE *pic);

/*** GEM ICN -------------------------------------------------------------- ***/

/*
 * slightly different names than the other converters, and also
 * different semantics:
 * unlike the others, the ICN reader reads VDI standard format,
 * and the exporter also expects this format.
 */
gboolean pic_type_icn(PICTURE *pic, const unsigned char *buf, long size);

gboolean icn_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean with_mask);
gboolean icn_fwrite(FILE *fp, const unsigned char *src, PICTURE *pic, gboolean with_mask);

/*** STAD ----------------------------------------------------------------- ***/

#define STAD_HEADER_SIZE 4 /* sizeof(STAD_HEADER) */

gboolean pic_type_stad(PICTURE *pic, const unsigned char *buf, long size);

gboolean stad_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic);
unsigned char *stad_pack(const unsigned char *src, unsigned char *horbuf, unsigned char *verbuf, PICTURE *pic);

/*** Degas Elite ---------------------------------------------------------- ***/

#define DEGAS_HEADER_SIZE 34 /* sizeof(DEGAS_HEADER) */

gboolean pic_type_degas(PICTURE *pic, const unsigned char *buf, long size);
long degas_header(unsigned char *buf, PICTURE *pic);

gboolean degas_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic);
unsigned char *degas_pack(const unsigned char *src, PICTURE *pic);

/*** Amiga IFF ------------------------------------------------------------ ***/

#define IFF_HEADER_SIZE 8 /* sizeof(IFF_HEADER) */
#define IFF_HEADER_BUFSIZE (48 + 3 * 256 + IFF_HEADER_SIZE)

#define iff_unpack degas_unpack
#define iff_pack degas_pack

gboolean pic_type_iff(PICTURE *pic, const unsigned char *buf, long size);
long iff_header(unsigned char *buf, PICTURE *pic);

/*** Color-/Monostar ------------------------------------------------------ ***/

#define COLOR_HEADER_SIZE 32 /* sizeof(COLOR_HEADER) */

gboolean pic_type_colorstar(PICTURE *pic, const unsigned char *buf, long size);
long colstar_header(unsigned char *buf, PICTURE *pic);

gboolean pic_type_monostar(PICTURE *pic, const unsigned char *buf, long size);
long monostar_header(unsigned char *buf, PICTURE *pic);

/*** Neochrome ------------------------------------------------------------ ***/

#define NEO_HEADER_SIZE 128 /* sizeof(NEO_HEADER) */

gboolean pic_type_neo(PICTURE *pic, const unsigned char *buf, long size);
long neo_header(unsigned char *buf, PICTURE *pic);

/*** Calamus -------------------------------------------------------------- ***/

typedef struct {				/* Header fuer Calamus PAGE.IMG */
	unsigned char filler1[12];
	long width_bytes;
	long height;
	unsigned char filler2[28];
} CALAMUS_HEADER;

/*** Imagic --------------------------------------------------------------- ***/

#define IMAGIC_HEADER_SIZE 64 /* sizeof(IMAGIC_HEADER) */

gboolean pic_type_imagic(PICTURE *pic, const unsigned char *buf, long size);
long imagic_header(unsigned char *buf, PICTURE *pic);

gboolean imagic_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic);
long imagic_pack(unsigned char *dest, const unsigned char *src, PICTURE *pic);

/*** Windows Icons -------------------------------------------------------- ***/

gboolean pic_type_ico(PICTURE *pic, const unsigned char *buf, long size);
long ico_header(unsigned char **buf, PICTURE *pic);
long ico_multi_header(unsigned char **buf, PICTURE *pic, const _WORD *planetab, _WORD planecount);

gboolean ico_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean with_mask);
long ico_pack(unsigned char *dest, const unsigned char *data, const unsigned char *mask, PICTURE *pic);

/*** BMP ------------------------------------------------------------------ ***/

#define BMP_STD_HSIZE 40

unsigned char *bmp_put_palette(unsigned char *buf, PICTURE *pic, const unsigned char *maptab);
extern unsigned char const bmp_coltab8[256];
extern unsigned char const bmp_revtab8[256];
extern unsigned char const bmp_coltab4[16];
extern unsigned char const bmp_revtab4[16];
extern unsigned char const bmp_idtab[256];

gboolean pic_type_bmp(PICTURE *pic, const unsigned char *buf, long size);
long bmp_header(unsigned char **buf, PICTURE *pic, const unsigned char *maptab);
long bmp_rowsize(PICTURE *pic, _WORD planes);

gboolean bmp_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean with_mask);
long bmp_pack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean update_header);
long bmp_pack_planes(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean update_header);
long bmp_pack_data_and_mask(unsigned char *dest, const unsigned char *src, const unsigned char *mask, PICTURE *pic, gboolean update_header);

/*** GIF ------------------------------------------------------------------ ***/

gboolean pic_type_gif(PICTURE *pic, const unsigned char *buf, long size);

int pic_find_transparent(PICTURE *pic, const unsigned char *data);
gboolean gif_fwrite(FILE *fp, const unsigned char *src, PICTURE *pic);
unsigned char *gif_pack(const unsigned char *src, PICTURE *pic);
gboolean gif_unpack(unsigned char **dest, const unsigned char *src, PICTURE *pic);
gboolean gif_fread(unsigned char **dest, FILE *fp, PICTURE *pic);

/*** PNG ------------------------------------------------------------------ ***/

gboolean pic_type_png(PICTURE *pic, const unsigned char *buf, long size);
#ifdef HAVE_PNG
gboolean png_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean with_mask);
gboolean png_fwrite(FILE *fp, const unsigned char *src, PICTURE *pic);
unsigned char *png_pack(const unsigned char *src, PICTURE *pic);
long png_rowsize(PICTURE *pic, _WORD planes);
#endif

/*** ---------------------------------------------------------------------- ***/

/*
 * functions for converting resolution,
 * adjusting the image size.
 * i.e. pic_16to2 will double both the width and height of the source
 */

void pic_16to4    (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_16to2    (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_4to2     (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_4to16    (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_2to16    (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_2to4     (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_256to2   (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_256to4   (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_256to16  (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_16to256  (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_4to256   (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);
void pic_2to256   (unsigned char *dest, unsigned char *src, PICTURE *pic, _WORD bytes, _WORD height);

/* misc helper functions */

void pic_invert(unsigned char *, long size);
void pic_planes_to_interleaved(unsigned char *dst, const unsigned char *src, PICTURE *pic);
void pic_interleaved_to_planes(unsigned char *dst, const unsigned char *src, _WORD width, _WORD height, _WORD planes);
void pic_savepalette(PALETTE);
void pic_showpalette(PALETTE);
void pic_getpalette(PALETTE, const TOSPALETTE *);
void pic_setpalette(TOSPALETTE, PALETTE);
void pic_stdpalette(PALETTE pal, _WORD planes);
long pic_rowsize(PICTURE *pic, _WORD planes);
void pic_calcsize(PICTURE *pic);
void pic_stdsize(PICTURE *pic);
_WORD pic_calcrez(_WORD planes);
_WORD pic_calcplanes(_WORD rez);
void pic_normal_planes(PICTURE *pic);
pic_filetype pic_type(PICTURE *pic, unsigned char *buf, long size);
void pic_init(PICTURE *pic);
_WORD pic_rgb_to_vdi(unsigned char c);
void pic_rgb_to_vdipal(_WORD *vdi, PALETTE pal, _WORD colors, const _WORD pixel_to_color[]);
void pic_vdi_to_rgbpal(PALETTE pal, _WORD *vdi, _WORD colors, _WORD components);
void pic_vdi_to_rgbcolor(_WORD vdi[3]);

void err_bmp_rle(const char *path);

void pic_free(PICTURE *pic);

unsigned long pic_pal_stddiff(const PICTURE *pic);
gboolean pic_match_stdpal(PICTURE *pic, unsigned char *buf);
char *pic_colornameformat(int planes);

EXTERN_C_END

#endif /* __PICTURE_H__ */
