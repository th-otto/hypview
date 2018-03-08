/*****************************************************************************
 * GIF.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

#define get_long() \
	(((_ULONG)(buf[3]) << 24) | \
	 ((_ULONG)(buf[2]) << 16) | \
	 ((_ULONG)(buf[1]) <<  8) | \
	 ((_ULONG)(buf[0])      )), buf += 4
#define get_word() \
	(((_UWORD)(buf[1]) <<  8) | \
	 ((_UWORD)(buf[0])      )), buf += 2
#define get_byte() \
	*buf++




#define GIFMAXVAL 255
#define MAXCMAPSIZE 256

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
typedef int code_int;

typedef int_fast32_t count_int;

struct cmap
{
	/* This is the information for the GIF colormap (aka palette). */

	int red[MAXCMAPSIZE];
	int green[MAXCMAPSIZE];
	int blue[MAXCMAPSIZE];
	/* These arrays arrays map a color index, as is found in
	   the raster part of the GIF, to an intensity value for the indicated
	   RGB component.
	 */
	int perm[MAXCMAPSIZE];
	int cmapsize;
	/* Number of entries in the GIF colormap.  I.e. number of colors
	   in the image, plus possibly one fake transparency color.
	 */
	int transparent;
	/* color index number in GIF palette of the color that is to be
	   transparent.  -1 if no color is transparent.
	 */
};


/*
 * General DEFINEs
 */

#define MAX_LZW_BITS   12

#define HSIZE  5003						/* 80% occupancy */

#define INTERLACE      0x40
#define LOCALCOLORMAP  0x80


/* should NEVER generate this code */
#define MAXCODE(n_bits)        (((code_int) 1 << (n_bits)) - 1)

#define maxmaxcode (((code_int) 1) << (MAX_LZW_BITS))


#define BitIsSet(byte, bit)      (((byte) & (bit)) != 0)

#define LM_to_uint(a,b)                        (((b)<<8)|(a))


struct byteBuffer
{
	/* Number of bytes so far in the current data block */
	unsigned int count;
	/* The current data block, under construction */
	unsigned char buffer[256];
};


struct codeBuffer
{
	unsigned int initBits;				/* user settable max # bits/code */
	unsigned int n_bits;				/* number of bits/code */
	code_int maxcode;					/* maximum code, given n_bits */

	uint_fast32_t curAccum;
	int curBits;

	int_fast32_t htab[HSIZE];
	unsigned short codetab[HSIZE];
#define HashTabOf(i)       gif->codeBuffer.htab[i]
#define CodeTabOf(i)    gif->codeBuffer.codetab[i]
};


enum pass { MULT8PLUS0, MULT8PLUS4, MULT4PLUS2, MULT2PLUS1 };


struct pixelCursor
{
	/* Width of the image, in columns */
	unsigned int width;
	/* Height of the image, in rows */
	unsigned int height;
	/* We're accessing the image in interlace fashion */
	int interlace;
	/* Number of pixels left to be read */
	uint_fast32_t nPixelsLeft;
	/* Location of pointed-to pixel, column */
	unsigned int curCol;
	/* Location of pointed-to pixel, row */
	unsigned int curRow;
	/* The interlace pass.  Undefined if !interlace */
	enum pass pass;
};



struct gif_dest
{
	/* This structure controls output of uncompressed GIF raster */
	FILE *outfile;						/* The file to which to output */
	struct byteBuffer byteBuffer;		/* Where the full bytes go */
	struct codeBuffer codeBuffer;
	int width, height;
	struct pixelCursor pixelCursor;
	
	/* State for GIF code assignment */
	code_int ClearCode;					/* clear code (doesn't change) */
	code_int EOFCode;					/* EOF code (ditto) */
	code_int code_counter;				/* counts output symbols */
	code_int free_ent;					/* first unused entry */

	/*
	 * block compression parameters -- after all codes are used up,
	 * and compression rate changes, start over.
	 */
	int clear_flg;

	long int in_count;					/* length of input */
	long int out_count;					/* # of codes output (for debugging) */

	PICTURE *pic;
	const unsigned char *pixels;
};



struct gif89
{
	int transparent;
	int delayTime;
	int inputFlag;
	int disposal;
};


struct getCodeState
{
	unsigned char buf[280];
	/* This is the buffer through which we read the data from the 
	   stream.  We must buffer it because we have to read whole data
	   blocks at a time, but our client wants one code at a time.
	   The buffer typically contains the contents of one data block
	   plus two bytes from the previous data block.
	 */
	int bufCount;
	/* This is the number of bytes of contents in buf[]. */
	int curbit;
	/* The bit number (starting at 0) within buf[] of the next bit
	   to be returned.  If the next bit to be returned is not yet in
	   buf[] (we've already returned everything in there), this points
	   one beyond the end of the buffer contents.
	 */
	int streamExhausted;
	/* The last time we read from the input stream, we got an EOD marker
	   or EOF
	 */
};


struct stack
{
	/* Stack grows from low addresses to high addresses */
	int stack[maxmaxcode * 2];		/* array */
	int *sp;						/* stack pointer */
	int *top;						/* next word above top of stack */
};




struct gifScreen
{
	unsigned int Width;
	unsigned int Height;
	PALETTE ColorMap;
	unsigned int ColorMapSize;
	/* Number of colors in the color map. */
	unsigned int ColorResolution;
	unsigned int Background;
	/* Aspect ratio of each pixel, times 64, minus 15.  (i.e. 1 => 1:4).
	   But Zero means 1:1.
	 */
	unsigned int AspectRatio;
	/* Boolean: global colormap has at least one gray color
	   (not counting black and white) 
	 */
	int hasGray;
	/* Boolean: global colormap has at least one non-gray,
	   non-black, non-white color 
	 */
	int hasColor;
};


struct decompressor
{
	/* The stream is right after a clear code or at the very beginning */
	gboolean fresh;
	/* The current code size -- each LZW code in this part of the image
	   is this many bits.  Ergo, we read this many bits at a time from
	   the stream.
	 */
	int codeSize;
	/* The maximum number of LZW codes that can be represented with the 
	   current code size.  (1 << codeSize)
	 */
	int maxnum_code;
	/* Index in the code translation table of the next free entry */
	int next_tableSlot;
	/* This is always a true data element code */
	int firstcode;
	/* The code just before, in the image, the one we're processing now */
	int prevcode;

	/* The following are constant for the life of the decompressor */
	int init_codeSize;
	int max_dataVal;
	int clear_code;
	int end_code;

	/* must be last because structure exceeds 32K; Pure-C generates wrong code otherwise */
	struct stack stack;
	int table[2][(1 << MAX_LZW_BITS)];
};



struct pnmBuffer
{
	unsigned char *pixels;
	unsigned int top;
	unsigned int left;
	unsigned int col;
	unsigned int row;
};


struct gif_src {
	struct {
		int type;
		union {
			FILE *infile;
			struct {
				const unsigned char *src;
				size_t len;
				size_t pos;
			} mem;
		} u;
	} in;
	FILE *imageout_file;
	FILE *alpha_file;
	int got_errors;
	int ext_errors;
	
	enum pass pass;
	int **alphabits;
	struct pnmBuffer pnmBuffer;
	unsigned int cols, rows;			/* Dimensions of the image */
	int interlaced;
	int transparent;
	struct gifScreen screen;
	PICTURE *pic;
	gboolean got_colormap;
	
	/* the most recently read DataBlock was an EOD marker, i.e. had
	   zero length */
	int zeroDataBlock;

	/* must be last because structure exceeds 32K; Pure-C generates wrong code otherwise */
	struct getCodeState getCodeState;
	struct decompressor decomp;
};

#define WRITE_OUTTXT 0

#if WRITE_OUTTXT
static FILE *outf;
#endif



/*********************************************************************************/
/* GIF output                                                                    */
/*********************************************************************************/

/*
 * Write out a word to the GIF file
 */
static void Putword(int w, FILE *fp)
{
	fputc(w & 0xff, fp);
	fputc((w >> 8) & 0xff, fp);
}


/*
 * Current location in the input pixels.
 */
static void initPixelCursor(struct pixelCursor *pixelCursor, unsigned int width, unsigned int height, int interlace)
{
	pixelCursor->width = width;
	pixelCursor->height = height;
	pixelCursor->interlace = interlace;
	pixelCursor->pass = MULT8PLUS0;
	pixelCursor->curCol = 0;
	pixelCursor->curRow = 0;
	pixelCursor->nPixelsLeft = (uint_fast32_t) width * (uint_fast32_t) height;
}



/*----------------------------------------------------------------------------
   Return the colormap index of the pixel at location (x,y)
-----------------------------------------------------------------------------*/
static __inline int GifGetPixel(struct gif_dest *gif, int x, int y)
{
	size_t bytes = pic_rowsize(gif->pic);
	const unsigned char *pos = gif->pixels + bytes * y;
	unsigned char mask;
	unsigned char color = 0;
	
	mask = 0x80 >> (x & 7);
	switch (gif->pic->pi_planes)
	{
	case 1:
		pos += (x >> 3);
		if (!(*pos & mask))
			color = 1;
		break;
	case 4:
		pos += (x >> 4) << 3;
		if (x & 0x08)
			pos++;
		if (pos[0] & mask) color |= 0x01;
		if (pos[2] & mask) color |= 0x02;
		if (pos[4] & mask) color |= 0x04;
		if (pos[6] & mask) color |= 0x08;
		break;
	case 8:
		pos += (x >> 4) << 4;
		if (x & 0x08)
			pos++;
		if (pos[ 0] & mask) color |= 0x01;
		if (pos[ 2] & mask) color |= 0x02;
		if (pos[ 4] & mask) color |= 0x04;
		if (pos[ 6] & mask) color |= 0x08;
		if (pos[ 8] & mask) color |= 0x10;
		if (pos[10] & mask) color |= 0x20;
		if (pos[12] & mask) color |= 0x40;
		if (pos[14] & mask) color |= 0x80;
		break;
	}
	
	return color;
}



/*----------------------------------------------------------------------------
   Move *pixelCursorP to the next row in the interlace pattern.
-----------------------------------------------------------------------------*/
static void bumpRowInterlace(struct pixelCursor *pixelCursorP)
{
	/* There are 4 passes:
	   MULT8PLUS0: Rows 8, 16, 24, 32, etc.
	   MULT8PLUS4: Rows 4, 12, 20, 28, etc.
	   MULT4PLUS2: Rows 2, 6, 10, 14, etc.
	   MULT2PLUS1: Rows 1, 3, 5, 7, 9, etc.
	 */

	switch (pixelCursorP->pass)
	{
	case MULT8PLUS0:
		pixelCursorP->curRow += 8;
		break;
	case MULT8PLUS4:
		pixelCursorP->curRow += 8;
		break;
	case MULT4PLUS2:
		pixelCursorP->curRow += 4;
		break;
	case MULT2PLUS1:
		pixelCursorP->curRow += 2;
		break;
	}
	/* Set the proper pass for the next read.  Note that if there are
	   more than 4 rows, the sequence of passes is sequential, but
	   when there are fewer than 4, we may skip e.g. from MULT8PLUS0
	   to MULT4PLUS2.
	 */
	while (pixelCursorP->curRow >= pixelCursorP->height)
	{
		switch (pixelCursorP->pass)
		{
		case MULT8PLUS0:
			pixelCursorP->pass = MULT8PLUS4;
			pixelCursorP->curRow = 4;
			break;
		case MULT8PLUS4:
			pixelCursorP->pass = MULT4PLUS2;
			pixelCursorP->curRow = 2;
			break;
		case MULT4PLUS2:
			pixelCursorP->pass = MULT2PLUS1;
			pixelCursorP->curRow = 1;
			break;
		case MULT2PLUS1:
			/* We've read the entire image; pass and current row are
			   now undefined.
			 */
			pixelCursorP->curRow = 0;
			break;
		}
	}
}



/*----------------------------------------------------------------------------
   Bump *pixelCursorP to point to the next pixel to go into the GIF

   Must not call when there are no pixels left.
-----------------------------------------------------------------------------*/
static void bumpPixel(struct gif_dest *gif)
{
	ASSERT(gif->pixelCursor.nPixelsLeft > 0);

	/* Move one column to the right */
	++gif->pixelCursor.curCol;

	if (gif->pixelCursor.curCol >= gif->pixelCursor.width)
	{
		/* That pushed us past the end of a row. */
		/* Reset to the left edge ... */
		gif->pixelCursor.curCol = 0;

		/* ... of the next row */
		if (!gif->pixelCursor.interlace)
			/* Go to the following row */
			++gif->pixelCursor.curRow;
		else
			bumpRowInterlace(&gif->pixelCursor);
	}
	--gif->pixelCursor.nPixelsLeft;
}



/*----------------------------------------------------------------------------
   Return the pre-sort color index (index into the unsorted GIF color map)
   of the next pixel to be processed from the input image.

   'alpha_threshold' is the gray level such that a pixel in the alpha
   map whose value is less that that represents a transparent pixel
   in the output.
-----------------------------------------------------------------------------*/
static __inline int GIFNextPixel(struct gif_dest *gif)
{
	int r;

	if (gif->pixelCursor.nPixelsLeft == 0)
		return EOF;

	r = GifGetPixel(gif, gif->pixelCursor.curCol, gif->pixelCursor.curRow);

	bumpPixel(gif);

	return r;
}



/*----------------------------------------------------------------------------
   Write out extension for transparent color index.
-----------------------------------------------------------------------------*/
static void write_transparent_color_index_extension(struct gif_dest *gif, int Transparent)
{
	fputc('!', gif->outfile);
	fputc(0xf9, gif->outfile);
	fputc(4, gif->outfile);
	fputc(1, gif->outfile);
	fputc(0, gif->outfile);
	fputc(0, gif->outfile);
	fputc(Transparent, gif->outfile);
	fputc(0, gif->outfile);
}



/*----------------------------------------------------------------------------
   Write out extension for a comment
-----------------------------------------------------------------------------*/
static void write_comment_extension(struct gif_dest *gif, const char comment[])
{
	const char *segment;

	fputc('!', gif->outfile);						/* Identifies an extension */
	fputc(0xfe, gif->outfile);					/* Identifies a comment */

	/* Write it out in segments no longer than 255 characters */
	for (segment = comment; segment < comment + strlen(comment); segment += 255)
	{
		int length_this_segment = (int) strlen(segment);
		if (length_this_segment > 255)
			length_this_segment = 255;
		fputc(length_this_segment, gif->outfile);

		fwrite(segment, 1, length_this_segment, gif->outfile);
	}

	fputc(0, gif->outfile);						/* No more comment blocks in this extension */
}



/***************************************************************************
 *
 *  GIFCOMPR.C       - GIF Image compression routines
 *
 *  Lempel-Ziv compression based on 'compress'.  GIF modifications by
 *  David Rowley (mgardi@watdcsu.waterloo.edu)
 *
 ***************************************************************************/

/*
 *
 * GIF Image compression - modified 'compress'
 *
 * Based on: compress.c - File compression ala IEEE Computer, June 1984.
 *
 * By Authors:  Spencer W. Thomas       (decvax!harpo!utah-cs!utah-gr!thomas)
 *              Jim McKie               (decvax!mcvax!jim)
 *              Steve Davies            (decvax!vax135!petsd!peora!srd)
 *              Ken Turkowski           (decvax!decwrl!turtlevax!ken)
 *              James A. Woods          (decvax!ihnp4!ames!jaw)
 *              Joe Orost               (decvax!vax135!petsd!joe)
 *
 */


/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

/***************************************************************************
*                          BYTE OUTPUTTER                 
***************************************************************************/

static void byteBuffer_init(struct byteBuffer *byteBufferP)
{
	byteBufferP->count = 0;
}



/*----------------------------------------------------------------------------
   Write the current data block to the output file, then reset the current 
   data block to empty.
-----------------------------------------------------------------------------*/
static void byteBuffer_flush(struct gif_dest *gif)
{
	if (gif->byteBuffer.count > 0)
	{
		fputc(gif->byteBuffer.count, gif->outfile);
		fwrite(gif->byteBuffer.buffer, 1, gif->byteBuffer.count, gif->outfile);
		gif->byteBuffer.count = 0;
	}
}



/*----------------------------------------------------------------------------
  Add a byte to the end of the current data block, and if it is now 254
  characters, flush the data block to the output file.
-----------------------------------------------------------------------------*/
static void byteBuffer_out(struct gif_dest *gif, unsigned char c)
{
	gif->byteBuffer.buffer[gif->byteBuffer.count++] = c;
	if (gif->byteBuffer.count >= 254)
		byteBuffer_flush(gif);
}



static uint_fast32_t const masks[] = {
	0x0000, 0x0001, 0x0003, 0x0007,
	0x000F, 0x001F, 0x003F, 0x007F,
	0x00FF, 0x01FF, 0x03FF, 0x07FF,
	0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF,
	0xFFFF
};


static void codeBuffer_init(struct gif_dest *gif, unsigned int initBits)
{
	gif->codeBuffer.initBits = initBits;
	gif->codeBuffer.n_bits = initBits;
	gif->codeBuffer.maxcode = MAXCODE(initBits);
	gif->codeBuffer.curAccum = 0;
	gif->codeBuffer.curBits = 0;
	gif->ClearCode = (1 << (initBits - 1));
	gif->EOFCode = gif->ClearCode + 1;
	gif->free_ent = gif->ClearCode + 2;
	gif->code_counter = gif->ClearCode + 2;
	gif->clear_flg = 0;
	gif->in_count = 1;
	gif->out_count = 0;
}



/*----------------------------------------------------------------------------
   Output one GIF code to the file, through the code buffer.

   The code is represented as n_bits bits in the file -- the lower
   n_bits bits of 'code'.

   If the code is EOF, flush the code buffer to the file.

   In some cases, change n_bits and recalculate maxcode to go with it.
-----------------------------------------------------------------------------*/
static void codeBuffer_output(struct gif_dest *gif, code_int code)
{
	/*
	   Algorithm:
	   Maintain a BITS character long buffer (so that 8 codes will
	   fit in it exactly).  Use the VAX insv instruction to insert each
	   code in turn.  When the buffer fills up empty it and start over.
	 */

	gif->codeBuffer.curAccum &= masks[gif->codeBuffer.curBits];

	if (gif->codeBuffer.curBits > 0)
		gif->codeBuffer.curAccum |= ((int_fast32_t) code << gif->codeBuffer.curBits);
	else
		gif->codeBuffer.curAccum = code;

	gif->codeBuffer.curBits += gif->codeBuffer.n_bits;

	while (gif->codeBuffer.curBits >= 8)
	{
		byteBuffer_out(gif, (unsigned int)(gif->codeBuffer.curAccum & 0xff));
		gif->codeBuffer.curAccum >>= 8;
		gif->codeBuffer.curBits -= 8;
	}

	if (gif->clear_flg)
	{
		gif->codeBuffer.n_bits = gif->codeBuffer.initBits;
		gif->codeBuffer.maxcode = MAXCODE(gif->codeBuffer.n_bits);
		gif->clear_flg = 0;
	} else if (gif->free_ent > gif->codeBuffer.maxcode)
	{
		/* The next entry is going to be too big for the code size, so
		   increase it, if possible.
		 */
		++gif->codeBuffer.n_bits;
		if (gif->codeBuffer.n_bits == MAX_LZW_BITS)
			gif->codeBuffer.maxcode = maxmaxcode;
		else
			gif->codeBuffer.maxcode = MAXCODE(gif->codeBuffer.n_bits);
	}

	if (code == gif->EOFCode)
	{
		/* We're at EOF.  Output the possible partial byte in the buffer */
		if (gif->codeBuffer.curBits > 0)
		{
			byteBuffer_out(gif, gif->codeBuffer.curAccum & 0xff);
			gif->codeBuffer.curBits = 0;
		}
		byteBuffer_flush(gif);
	}
}



/*
 * reset code table
 */
static void cl_hash(struct gif_dest *gif, int hsize)
{
	register int_fast32_t *htab_p = gif->codeBuffer.htab + hsize;
	register int_fast32_t m1 = -1;
	register int i;

	i = hsize - 16;
	do
	{
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
		*--htab_p = m1;
	} while ((i -= 16) >= 0);

	for (i += 16; i > 0; --i)
		*--htab_p = m1;
}



/*----------------------------------------------------------------------------
  Clear out the hash table
-----------------------------------------------------------------------------*/
static void cl_block(struct gif_dest *gif)
{
	cl_hash(gif, HSIZE);
	gif->free_ent = gif->ClearCode + 2;
	gif->clear_flg = 1;

	codeBuffer_output(gif, gif->ClearCode);
}



/*----------------------------------------------------------------------------
   Write the raster to file 'outfile'.

   The raster to write is 'pixels'

   Use the colormap 'cmapP' to generate the raster ('pixels' is 
   composed of RGB samples; the GIF raster is colormap indices).

   Write the raster using LZW compression.
-----------------------------------------------------------------------------*/
static void write_raster_LZW(struct gif_dest *gif)
{
	code_int ent;
	code_int disp;
	int hshift;
	int eof;

	ent = GIFNextPixel(gif);

	{
		int_fast32_t fcode;

		hshift = 0;
		for (fcode = HSIZE; fcode < 65536L; fcode *= 2L)
			++hshift;
		hshift = 8 - hshift;			/* set hash code range bound */
	}
	cl_hash(gif, HSIZE);				/* clear hash table */

	codeBuffer_output(gif, gif->ClearCode);

	eof = FALSE;
	while (!eof)
	{
		int gifpixel;

		/* The value for the pixel in the GIF image.  I.e. the colormap
		   index.  Or -1 to indicate "no more pixels."
		 */
		gifpixel = GIFNextPixel(gif);
		if (gifpixel == EOF)
			eof = TRUE;
		if (!eof)
		{
			int_fast32_t fcode = (int_fast32_t) (((int_fast32_t) gifpixel << MAX_LZW_BITS) + ent);
			code_int i;

			/* xor hashing */

			++gif->in_count;

			i = (((code_int) gifpixel << hshift) ^ ent);

			if (HashTabOf(i) == fcode)
			{
				ent = CodeTabOf(i);
				continue;
			} else if (HashTabOf(i) < 0)	/* empty slot */
				goto nomatch;
			disp = HSIZE - i;			/* secondary hash (after G. Knott) */
			if (i == 0)
				disp = 1;
		  probe:
			if ((i -= disp) < 0)
				i += HSIZE;

			if (HashTabOf(i) == fcode)
			{
				ent = CodeTabOf(i);
				continue;
			}
			if (HashTabOf(i) > 0)
				goto probe;
		  nomatch:
			codeBuffer_output(gif, ent);
			++gif->out_count;
			ent = gifpixel;
			if (gif->free_ent < maxmaxcode)
			{
				CodeTabOf(i) = gif->free_ent++;	/* code -> hashtable */
				HashTabOf(i) = fcode;
			} else
			{
				cl_block(gif);
			}
		}
	}
	/*
	 * Put out the final code.
	 */
	codeBuffer_output(gif, ent);
	++gif->out_count;
	codeBuffer_output(gif, gif->EOFCode);
}



/* Routine to convert variable-width codes into a byte stream */

static void output_uncompressed(struct gif_dest *gif, int code)
{
	/* Emit a code of n_bits bits */
	/* Uses curAccum and curBits to reblock into 8-bit bytes */
	gif->codeBuffer.curAccum |= ((int_fast32_t) code) << gif->codeBuffer.curBits;
	gif->codeBuffer.curBits += gif->codeBuffer.n_bits;

	while (gif->codeBuffer.curBits >= 8)
	{
		byteBuffer_out(gif, (unsigned int)(gif->codeBuffer.curAccum & 0xFF));
		gif->codeBuffer.curAccum >>= 8;
		gif->codeBuffer.curBits -= 8;
	}
}


/*----------------------------------------------------------------------------
   Initialize pseudo-compressor
-----------------------------------------------------------------------------*/
static void write_raster_uncompressed_init(struct gif_dest *gif)
{
	/* GIF specifies an initial Clear code */
	output_uncompressed(gif, gif->ClearCode);
}



/*----------------------------------------------------------------------------
   "Compress" one pixel value and output it as a symbol.

   'colormapIndex' must be less than gif->n_bits wide.
-----------------------------------------------------------------------------*/
static void write_raster_uncompressed_pixel(struct gif_dest *gif, unsigned int colormapIndex)
{
	ASSERT((colormapIndex >> gif->codeBuffer.n_bits) == 0);

	output_uncompressed(gif, colormapIndex);
	/* Issue Clear codes often enough to keep the reader from ratcheting up
	 * its symbol size.
	 */
	if (gif->code_counter < gif->codeBuffer.maxcode)
	{
		++gif->code_counter;
	} else
	{
		output_uncompressed(gif, gif->ClearCode);
		gif->code_counter = gif->ClearCode + 2;	/* reset the counter */
	}
}


/* Clean up at end */
static void write_raster_uncompressed_term(struct gif_dest *gif)
{
	/* Send an EOF code */
	output_uncompressed(gif, gif->EOFCode);

	/* Flush the bit-packing buffer */
	if (gif->codeBuffer.curBits > 0)
		byteBuffer_out(gif, gif->codeBuffer.curAccum & 0xFF);

	/* Flush the packet buffer */
	byteBuffer_flush(gif);
}



/*----------------------------------------------------------------------------
   Write the raster to file 'gif'.
   
   Same as write_raster_LZW(), except written out one code per
   pixel (plus some clear codes), so no compression.  And no use
   of the LZW patent.
-----------------------------------------------------------------------------*/
static void write_raster_uncompressed(struct gif_dest *gif)
{
	/* gray levels below this in the alpha mask indicate transparent
	   pixels in the output image.
	 */
	int eof;

	write_raster_uncompressed_init(gif);

	eof = FALSE;
	while (!eof)
	{
		int gifpixel;

		/* The value for the pixel in the GIF image.  I.e. the colormap
		   index.  Or -1 to indicate "no more pixels."
		 */
		gifpixel = GIFNextPixel(gif);
		if (gifpixel == EOF)
			eof = TRUE;
		else
			write_raster_uncompressed_pixel(gif, gifpixel);
	}
	write_raster_uncompressed_term(gif);
}



/******************************************************************************
 *
 * GIF Specific routines
 *
 *****************************************************************************/

static void
writeGifHeader(struct gif_dest *gif,
	int Background,
	int BitsPerPixel, int transparent, const char comment[])
{
	int B;
	int Resolution = BitsPerPixel;
	int ColorMapSize = 1 << BitsPerPixel;

	/* Write the Magic header */
	if (transparent != -1 || comment)
		fwrite("GIF89a", 1, 6, gif->outfile);
	else
		fwrite("GIF87a", 1, 6, gif->outfile);

	/* Write out the screen width and height */
	Putword(gif->width, gif->outfile);
	Putword(gif->height, gif->outfile);

	/* Indicate that there is a global color map */
	B = LOCALCOLORMAP;							/* Yes, there is a color map */

	/* OR in the resolution */
	B |= (Resolution - 1) << 4;

	/* OR in the Bits per Pixel */
	B |= (BitsPerPixel - 1);

	/* Write it out */
	fputc(B, gif->outfile);

	/* Write out the Background color */
	fputc(Background, gif->outfile);

	/* Byte of 0's (future expansion) */
	fputc(0, gif->outfile);

	{
		/* Write out the Global Color Map */
		int i;

		if (Resolution == 1)
		{
			fputc(0, gif->outfile);
			fputc(0, gif->outfile);
			fputc(0, gif->outfile);
			fputc(GIFMAXVAL, gif->outfile);
			fputc(GIFMAXVAL, gif->outfile);
			fputc(GIFMAXVAL, gif->outfile);
		} else
		{
			for (i = 0; i < ColorMapSize; ++i)
			{
				fputc(gif->pic->pi_palette[i].r, gif->outfile);
				fputc(gif->pic->pi_palette[i].g, gif->outfile);
				fputc(gif->pic->pi_palette[i].b, gif->outfile);
			}
		}
	}

	if (transparent >= 0)
		write_transparent_color_index_extension(gif, transparent);

	if (comment)
		write_comment_extension(gif, comment);
}


static void writeImageHeader(struct gif_dest *gif,
	unsigned int leftOffset,
	unsigned int topOffset,
	unsigned int initCodeSize)
{
	Putword(leftOffset, gif->outfile);
	Putword(topOffset, gif->outfile);
	Putword(gif->width, gif->outfile);
	Putword(gif->height, gif->outfile);

	/* Write out whether or not the image is interlaced */
	if (gif->pixelCursor.interlace)
		fputc(INTERLACE, gif->outfile);
	else
		fputc(0x00, gif->outfile);

	/* Write out the initial code size */
	fputc(initCodeSize, gif->outfile);
}



static gboolean GIFEncode(struct gif_dest *gif, int background, int transparent, const char *comment)
{
	unsigned int leftOffset = 0;
	unsigned int topOffset = 0;

	int bitsPerPixel = gif->pic->pi_planes;
	/* The initial code size */
	unsigned int initCodeSize = bitsPerPixel <= 1 ? 2 : bitsPerPixel;
	
	codeBuffer_init(gif, initCodeSize + 1);
	byteBuffer_init(&gif->byteBuffer);

	writeGifHeader(gif, background, bitsPerPixel, transparent, comment);

	/* Write an Image separator */
	fputc(',', gif->outfile);

	/* Write the Image header */
	writeImageHeader(gif, leftOffset, topOffset, initCodeSize);

	/* Write the actual raster */
	if (!gif->pic->pi_compressed)
		write_raster_uncompressed(gif);
	else
		write_raster_LZW(gif);

	/* Write out a zero length data block (to end the series) */
	fputc(0, gif->outfile);

	/* Write the GIF file terminator */
	fputc(';', gif->outfile);

	if (fflush(gif->outfile) != 0 ||
		ferror(gif->outfile))
		return FALSE;
	return TRUE;
}




gboolean gif_fwrite(FILE *fp, const unsigned char *src, PICTURE *pic)
{
	const char *comment = NULL;
	int transparent = -1;
	struct gif_dest *gif;
	gboolean ret;
	
	gif = g_new0(struct gif_dest, 1);
	if (gif == NULL)
		return FALSE;
	gif->outfile = fp;
	gif->pic = pic;
	gif->width = pic->pi_width;
	gif->height = pic->pi_height;
	gif->pixels = src;

	/* Set some global variables for bumpPixel() */
	initPixelCursor(&gif->pixelCursor, pic->pi_width, pic->pi_height, FALSE);
	
	/* All set, let's do it. */
	ret = GIFEncode(gif, 0, transparent, comment);
	g_free(gif);
	return ret;
}


/*********************************************************************************/
/* GIF input                                                                     */
/*********************************************************************************/

#if WRITE_OUTTXT
static void __attribute__((format(printf, 2, 3))) gif_error(struct gif_src *gif, const char *reason, ...)
{
	va_list args;

	gif->got_errors++;
	va_start(args, reason);
	vfprintf(outf, reason, args);
	fprintf(outf, "\n");
	va_end(args);
}
#define GIF_ERROR(x) gif_error x
#else
#define GIF_ERROR(x) gif->got_errors++
#endif


#if WRITE_OUTTXT
static void __attribute__((format(printf, 1, 2))) gif_message(const char *reason, ...)
{
	va_list args;
	
	va_start(args, reason);
	vfprintf(outf, reason, args);
	fprintf(outf, "\n");
	va_end(args);
}
#define GIF_MESSAGE(x) gif_message x
#define GIF_VERBOSE_MESSAGE(x) gif_message x
#else
#define GIF_MESSAGE(x) 
#define GIF_VERBOSE_MESSAGE(x) 
#endif


static __inline int ReadOK(struct gif_src *gif, unsigned char *buffer, size_t len)
{
	size_t bytesRead;

	if (gif->in.type == 0)
	{
		bytesRead = fread(buffer, 1, len, gif->in.u.infile);
		return bytesRead == len;
	} else
	{
		if ((len + gif->in.u.mem.pos) > gif->in.u.mem.len)
			return FALSE;
		memcpy(buffer, gif->in.u.mem.src + gif->in.u.mem.pos, len);
		gif->in.u.mem.pos += len;
	}
	return TRUE;
}


#if WRITE_OUTTXT
static long gif_tell(struct gif_src *gif)
{
	if (gif->in.type == 0)
		return ftell(gif->in.u.infile);
	return gif->in.u.mem.pos;
}
#endif


static void initGif89(struct gif89 *gif89P)
{
	gif89P->transparent = -1;
	gif89P->delayTime = -1;
	gif89P->inputFlag = -1;
	gif89P->disposal = -1;
}


static void readColorMap(struct gif_src *gif, int colormapsize, PALETTE colormap, int *hasGrayP, int *hasColorP)
{
	int i;
	unsigned char rgb[3];

	ASSERT(colormapsize <= MAXCMAPSIZE);

	*hasGrayP = FALSE;					/* initial assumption */
	*hasColorP = FALSE;					/* initial assumption */

	for (i = 0; i < colormapsize; ++i)
	{
		if (!ReadOK(gif, rgb, sizeof(rgb)))
		{
			GIF_ERROR((gif, "Unable to read Color %d from colormap", i));
		}
		GIF_MESSAGE(("colormap %d at %04lx: %02x %02x %02x", i, gif_tell(gif) - 3, rgb[0], rgb[1], rgb[2]));
		if (i == 118)
		{
		GIF_MESSAGE(("mem at 118: %02x %02x %02x", gif->in.u.mem.src[0x16f], gif->in.u.mem.src[0x170], gif->in.u.mem.src[0x171]));
		}
		colormap[i].r = rgb[0];
		colormap[i].g = rgb[1];
		colormap[i].b = rgb[2];

		if (rgb[0] == rgb[1] && rgb[1] == rgb[2])
		{
			if (rgb[0] != 0 && rgb[0] != GIFMAXVAL)
				*hasGrayP = TRUE;
		} else
		{
			*hasColorP = TRUE;
		}
	}
	if (!gif->got_colormap)
	{
		for (i = 0; i < colormapsize; ++i)
			gif->pic->pi_palette[i] = colormap[i];
		gif->got_colormap = TRUE;
	}
	
#if WRITE_OUTTXT
	fprintf(outf, "palette:\n");
	for (i = 0; i < colormapsize; i++)
	{
		fprintf(outf,  "%02x: %02x %02x %02x\n", i, colormap[i].r, colormap[i].g, colormap[i].b);
	}
#endif
}



/*----------------------------------------------------------------------------
   Read a DataBlock from file 'input', return it at 'buf'.

   The first byte of the datablock is the length, in pure binary, of the
   rest of the datablock.  We return the data portion (not the length byte)
   of the datablock at 'buf', and its length as *lengthP.

   Except that if we hit EOF or have an I/O error reading the first
   byte (size field) of the DataBlock, we return *eofP == TRUE and
   *lengthP == 0.

   We return *eofP == FALSE if we don't hit EOF or have an I/O error.

   If we hit EOF or have an I/O error reading the data portion of the
   DataBlock, we exit the program with pm_error().
-----------------------------------------------------------------------------*/
static void getDataBlock(struct gif_src *gif, unsigned char *buf, int * eofP, unsigned int *lengthP)
{
	unsigned char count;
	int successfulRead;

	successfulRead = ReadOK(gif, &count, 1);
	if (!successfulRead)
	{
		GIF_MESSAGE(("EOF or error in reading DataBlock size from file"));
		*eofP = TRUE;
		*lengthP = 0;
	} else
	{
		GIF_VERBOSE_MESSAGE(("\n%d byte block at Position $%lx", count, gif_tell(gif) - 1));
		*eofP = FALSE;
		*lengthP = count;

		if (count == 0)
		{
			gif->zeroDataBlock = TRUE;
		} else
		{
			gif->zeroDataBlock = FALSE;
			successfulRead = ReadOK(gif, buf, count);

			if (!successfulRead)
			{
				GIF_ERROR((gif, "EOF or error reading data portion of %d byte DataBlock from file", count));
			}
		}
	}
}



/*----------------------------------------------------------------------------
  Read the file 'input' through the next EOD marker.  An EOD marker is a
  a zero length data block.

  If there is no EOD marker between the present file position and EOF,
  we read to EOF and issue warning message about a missing EOD marker.
-----------------------------------------------------------------------------*/
static void readThroughEod(struct gif_src *gif)
{
	unsigned char buf[260];
	int eod;

	eod = FALSE;						/* initial value */
	while (!eod)
	{
		int eof;
		unsigned int count;

		getDataBlock(gif, buf, &eof, &count);
		if (eof)
		{
			GIF_MESSAGE(("EOF encountered before EOD marker.  The GIF "
					   "file is malformed, but we are proceeding "
					   "anyway as if an EOD marker were at the end " "of the file."));
		}
		if (eof || count == 0)
			eod = TRUE;
	}
}




/*----------------------------------------------------------------------------
   Read the rest of a comment extension from the input file 'input' and handle
   it.
   
   We ought to deal with the possibility that the comment is not text.  I.e.
   it could have nonprintable characters or embedded nulls.  I don't know if
   the GIF spec requires regular text or not.
-----------------------------------------------------------------------------*/
static void doCommentExtension(struct gif_src *gif)
{
	char buf[255 + 1];
	unsigned int blocklen;
	int done;

	done = FALSE;
	while (!done)
	{
		int eof;

		getDataBlock(gif, (unsigned char *) buf, &eof, &blocklen);
		if (blocklen == 0 || eof)
			done = TRUE;
		else
		{
			buf[blocklen] = '\0';
			GIF_VERBOSE_MESSAGE(("gif comment: %s", buf));
		}
	}
}



static void doGraphicControlExtension(struct gif_src *gif, struct gif89 *gif89P)
{
	int eof;
	unsigned int length;
	unsigned char buf[256];

	getDataBlock(gif, buf, &eof, &length);
	if (eof)
	{
		GIF_ERROR((gif, "EOF/error encountered reading 1st DataBlock of Graphic Control Extension."));
	} else if (length < 4)
	{
		GIF_ERROR((gif, "graphic control extension 1st DataBlock too short.  "
				 "It must be at least 4 bytes; it is %d bytes.", length));
	} else
	{
		gif89P->disposal = (buf[0] >> 2) & 0x7;
		gif89P->inputFlag = (buf[0] >> 1) & 0x1;
		gif89P->delayTime = LM_to_uint(buf[1], buf[2]);
		if ((buf[0] & 0x1) != 0)
			gif89P->transparent = buf[3];
		readThroughEod(gif);
	}
}



static void doExtension(struct gif_src *gif, int label, struct gif89 *gif89P)
{
#if WRITE_OUTTXT
	const char *str;
	char buf[256];
#endif

	switch (label)
	{
	case 0x01:							/* Plain Text Extension */
#if WRITE_OUTTXT
		str = "Plain Text";
#endif
#ifdef notdef
		GetDataBlock(gif, (unsigned char *) buf, &eof, &length);

		lpos = LM_to_uint(buf[0], buf[1]);
		tpos = LM_to_uint(buf[2], buf[3]);
		width = LM_to_uint(buf[4], buf[5]);
		height = LM_to_uint(buf[6], buf[7]);
		cellw = buf[8];
		cellh = buf[9];
		foreground = buf[10];
		background = buf[11];

		while (GetDataBlock(gif, (unsigned char *) buf) != 0)
		{
			PPM_ASSIGN(xels[ypos][xpos], cmap[v].r, cmap[v].g, cmap[v].b);
			++index;
		}
#else
		readThroughEod(gif);
#endif
		break;
	case 0xff:							/* Application Extension */
#if WRITE_OUTTXT
		str = "Application";
#endif
		readThroughEod(gif);
		break;
	case 0xfe:							/* Comment Extension */
#if WRITE_OUTTXT
		str = "Comment";
#endif
		doCommentExtension(gif);
		break;
	case 0xf9:							/* Graphic Control Extension */
#if WRITE_OUTTXT
		str = "Graphic Control";
#endif
		doGraphicControlExtension(gif, gif89P);
		break;
	default:
#if WRITE_OUTTXT
		str = buf;
		sprintf(buf, "UNKNOWN (0x%02x)", label);
#endif
		GIF_MESSAGE(("Ignoring unrecognized extension (type 0x%02x)", label));
		readThroughEod(gif);
		break;
	}
	GIF_VERBOSE_MESSAGE((" got a '%s' extension", str));
}



/*----------------------------------------------------------------------------
   Initialize the code getter.
-----------------------------------------------------------------------------*/
static void initGetCode(struct gif_src *gif)
{
	/* Fake a previous data block */
	gif->getCodeState.buf[0] = 0;
	gif->getCodeState.buf[1] = 0;
	gif->getCodeState.bufCount = 2;
	gif->getCodeState.curbit = gif->getCodeState.bufCount * 8;
	gif->getCodeState.streamExhausted = FALSE;
}



static void getAnotherBlock(struct gif_src *gif)
{
	unsigned int count;
	unsigned int assumed_count;
	int eof;
	struct getCodeState *gsP = &gif->getCodeState;
	
	/* Shift buffer down so last two bytes are now the
	   first two bytes.  Shift 'curbit' cursor, which must
	   be somewhere in or immediately after those two
	   bytes, accordingly.
	 */
	gsP->buf[0] = gsP->buf[gsP->bufCount - 2];
	gsP->buf[1] = gsP->buf[gsP->bufCount - 1];

	gsP->curbit -= (gsP->bufCount - 2) * 8;
	gsP->bufCount = 2;

	/* Add the next block to the buffer */
	getDataBlock(gif, &gsP->buf[gsP->bufCount], &eof, &count);
	if (eof)
	{
		GIF_MESSAGE(("EOF encountered in image before EOD marker.  The GIF file is malformed, but we are proceeding "
				   "anyway as if an EOD marker were at the end of the file."));
		assumed_count = 0;
	} else
	{
		assumed_count = count;
	}
	
	gsP->streamExhausted = (assumed_count == 0);

	gsP->bufCount += assumed_count;
}



/*----------------------------------------------------------------------------
   Read and return the next lzw code from the file input.

   'codeSize' is the number of bits in the code we are to get.

   Return -1 instead of a code if we encounter the end of the file.
-----------------------------------------------------------------------------*/
static int getCode(struct gif_src *gif, int codeSize)
{
	struct getCodeState *gsP = &gif->getCodeState;
	int retval;
	
	while (gsP->curbit + codeSize > gsP->bufCount * 8 && !gsP->streamExhausted)
		/* Not enough left in buffer to satisfy request.  Get the next
		   data block into the buffer.

		   Note that a data block may be as small as one byte, so we may need
		   to do this multiple times to get the full code.  (This probably
		   never happens in practice).
		 */
		getAnotherBlock(gif);

	if ((gsP->curbit + codeSize) > gsP->bufCount * 8)
	{
		/* If the buffer still doesn't have enough bits in it, that means
		   there were no data blocks left to read.
		 */
		retval = -1;					/* EOF */

		{
			int bitsUnused = gsP->bufCount * 8 - gsP->curbit;

			if (bitsUnused > 0)
			{
				GIF_MESSAGE(("Stream ends with a partial code (%d bits left in file; expected a %d bit code).  Ignoring.", bitsUnused, codeSize));
			}
		}
	} else
	{
		int i, j;
		int code;
		unsigned char *buf = gsP->buf;

		code = 0;						/* initial value */
		for (i = gsP->curbit, j = 0; j < codeSize; ++i, ++j)
			code |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;
		gsP->curbit += codeSize;
		retval = code;
	}
	return retval;
}



static void initStack(struct stack *stackP)
{
	stackP->sp = stackP->stack;
	stackP->top = stackP->stack + maxmaxcode * 2;
}



static void pushStack(struct gif_src *gif, int value)
{
	struct stack *stackP = &gif->decomp.stack;
	
	if (stackP->sp >= stackP->top)
	{
		GIF_ERROR((gif, "stack overflow"));
	} else
	{
		*(stackP->sp++) = value;
	}
}



static int stackIsEmpty(struct gif_src *gif)
{
	struct stack *stackP = &gif->decomp.stack;
	return stackP->sp == stackP->stack;
}



static int popStack(struct gif_src *gif)
{
	struct stack *stackP = &gif->decomp.stack;
	if (stackP->sp <= stackP->stack)
	{
		GIF_ERROR((gif, "stack underflow"));
		return 0;
	}
	return *(--stackP->sp);
}



/*----------------------------------------------------------------------------
   Some notes on LZW.

   LZW is an extension of Limpel-Ziv.  The two extensions are:

     1) in Limpel-Ziv, codes are all the same number of bits.  In
        LZW, they start out small and increase as the stream progresses.

     2) LZW has a clear code that resets the string table and code
        size.

   The LZW code space is allocated as follows:

   The true data elements are dataWidth bits wide, so the maximum
   value of a true data element is 2**dataWidth-1.  We call that
   max_dataVal.  The first byte in the stream tells you what dataWidth
   is.

   LZW codes 0 - max_dataVal are direct codes.  Each on represents
   the true data element whose value is that of the LZW code itself.
   No decompression is required.

   max_dataVal + 1 and up are compression codes.  They encode
   true data elements:

   max_dataVal + 1 is the clear code.
         
   max_dataVal + 2 is the end code.

   max_dataVal + 3 and up are string codes.  Each string code 
   represents a string of true data elements.  The translation from a
   string code to the string of true data elements varies as the stream
   progresses.  In the beginning and after every clear code, the
   translation table is empty, so no string codes are valid.  As the
   stream progresses, the table gets filled and more string codes 
   become valid.

-----------------------------------------------------------------------------*/


static void resetDecompressor(struct decompressor *decompP)
{
	decompP->codeSize = decompP->init_codeSize + 1;
	decompP->maxnum_code = 1 << decompP->codeSize;
	decompP->next_tableSlot = decompP->max_dataVal + 3;
	decompP->fresh = TRUE;
}



static void lzwInit(struct decompressor *decomp, int init_codeSize)
{
	decomp->init_codeSize = init_codeSize;

	ASSERT(decomp->init_codeSize < (int)sizeof(decomp->max_dataVal) * 8);

	decomp->max_dataVal = (1 << init_codeSize) - 1;
	decomp->clear_code = decomp->max_dataVal + 1;
	decomp->end_code = decomp->max_dataVal + 2;

	/* The entries in the translation table for true data codes are
	   constant throughout the stream.  We set them now and they never
	   change.
	 */
	{
		int i;
		int (*table)[1 << MAX_LZW_BITS];
		
		table = decomp->table;
		for (i = 0; i <= decomp->max_dataVal; ++i)
		{
			table[0][i] = 0;
			table[1][i] = i;
		}
	}
	resetDecompressor(decomp);

	decomp->fresh = TRUE;

	initStack(&decomp->stack);
}



/*----------------------------------------------------------------------------
   'incode' is an LZW string code.  It represents a string of true data
   elements, as defined by the string translation table in *decompP.

   Expand the code to a string of LZW direct codes and push them onto the
   stack such that the leftmost code is on top.

   Also add to the translation table where appropriate.

   Iff the translation table contains a cycle (which means the LZW stream
   from which it was built is invalid), return *errorP == TRUE.
-----------------------------------------------------------------------------*/
static int expandCodeOntoStack(struct gif_src *gif, int incode)
{
	int code;
	int error = FALSE;
	int (*table)[1 << MAX_LZW_BITS] = gif->decomp.table;

	if (incode < gif->decomp.next_tableSlot)
	{
		code = incode;
	} else
	{
		/* It's a code that isn't in our translation table yet */
		pushStack(gif, gif->decomp.firstcode);
		code = gif->decomp.prevcode;
	}

	{
		/* Get the whole string that this compression code
		   represents and push it onto the code stack so the
		   leftmost code is on top.  Set gif->decomp.firstcode to the
		   first (leftmost) code in that string.
		 */

		unsigned int stringCount;
		stringCount = 0;

		while (code > gif->decomp.max_dataVal && !error)
		{
			if (stringCount > maxmaxcode)
			{
				GIF_MESSAGE(("Error in GIF image: contains LZW string loop"));
				error = TRUE;
			} else
			{
				
				++stringCount;
				pushStack(gif, table[1][code]);
				code = table[0][code];
			}
		}
		gif->decomp.firstcode = table[1][code];
		pushStack(gif, gif->decomp.firstcode);
	}

	if (gif->decomp.next_tableSlot < maxmaxcode)
	{
		table[0][gif->decomp.next_tableSlot] = gif->decomp.prevcode;
		table[1][gif->decomp.next_tableSlot] = gif->decomp.firstcode;
		++gif->decomp.next_tableSlot;
		if (gif->decomp.next_tableSlot >= gif->decomp.maxnum_code)
		{
			/* We've used up all the codes of the current code size.
			   Future codes in the stream will have codes one bit longer.
			   But there's an exception if we're already at the LZW
			   maximum, in which case the codes will simply continue
			   the same size.
			 */
			if (gif->decomp.codeSize < MAX_LZW_BITS)
			{
				++gif->decomp.codeSize;
				gif->decomp.maxnum_code = 1 << gif->decomp.codeSize;
			}
		}
	}

	gif->decomp.prevcode = incode;
	return error;
}



/*----------------------------------------------------------------------------
  Return the next data element of the decompressed image.  In the context
  of a GIF, a data element is the color table index of one pixel.

  We read and return the next byte of the decompressed image, or:

    Return -1 if we hit EOF prematurely (i.e. before an "end" code.  We
    forgive the case that the "end" code is followed by EOF instead of
    an EOD marker (zero length DataBlock)).

    Return -2 if there are no more bytes in the image.  In that case,
    make sure the file is positioned immediately after the image (i.e.
    after the EOD marker that marks the end of the image or EOF).

    Return -3 if we encounter errors in the LZW stream.
-----------------------------------------------------------------------------*/
static int lzwReadByte(struct gif_src *gif)
{
	int retval;

	if (!stackIsEmpty(gif))
	{
		retval = popStack(gif);
	} else if (gif->decomp.fresh)
	{
		gif->decomp.fresh = FALSE;
		/* Read off all initial clear codes, read the first non-clear code,
		   and return it.  There are no strings in the table yet, so the next
		   code must be a direct true data code.
		 */
		do
		{
			gif->decomp.firstcode = getCode(gif, gif->decomp.codeSize);
			gif->decomp.prevcode = gif->decomp.firstcode;
		} while (gif->decomp.firstcode == gif->decomp.clear_code);
		if (gif->decomp.firstcode == gif->decomp.end_code)
		{
			if (!gif->zeroDataBlock)
				readThroughEod(gif);
			retval = -2;
		} else
		{
			retval = gif->decomp.firstcode;
		}
	} else
	{
		int code;

		code = getCode(gif, gif->decomp.codeSize);
		if (code == -1)
		{
			retval = -1;
		} else
		{
			ASSERT(code >= 0);			/* -1 is only possible error return */
			if (code == gif->decomp.clear_code)
			{
				resetDecompressor(&gif->decomp);
				retval = lzwReadByte(gif);
			} else
			{
				if (code == gif->decomp.end_code)
				{
					if (!gif->zeroDataBlock)
						readThroughEod(gif);
					retval = -2;
				} else
				{
					int error;

					error = expandCodeOntoStack(gif, code);
					if (error)
						retval = -3;
					else
						retval = popStack(gif);
				}
			}
		}
	}
	return retval;
}



/*----------------------------------------------------------------------------
   Move *pixelCursorP to the next row in the interlace pattern.
-----------------------------------------------------------------------------*/
static void bumpInputRowInterlace(struct gif_src *gif, unsigned int *rowP, unsigned int rows)
{
	/* There are 4 passes:
	   MULT8PLUS0: Rows 8, 16, 24, 32, etc.
	   MULT8PLUS4: Rows 4, 12, 20, 28, etc.
	   MULT4PLUS2: Rows 2, 6, 10, 14, etc.
	   MULT2PLUS1: Rows 1, 3, 5, 7, 9, etc.
	 */

	switch (gif->pass)
	{
	case MULT8PLUS0:
		*rowP += 8;
		break;
	case MULT8PLUS4:
		*rowP += 8;
		break;
	case MULT4PLUS2:
		*rowP += 4;
		break;
	case MULT2PLUS1:
		*rowP += 2;
		break;
	}
	/* Set the proper pass for the next read.  Note that if there are
	   more than 4 rows, the sequence of passes is sequential, but
	   when there are fewer than 4, we may skip e.g. from MULT8PLUS0
	   to MULT4PLUS2.
	 */
	while (*rowP >= rows && gif->pass != MULT2PLUS1)
	{
		switch (gif->pass)
		{
		case MULT8PLUS0:
			gif->pass = MULT8PLUS4;
			*rowP = 4;
			break;
		case MULT8PLUS4:
			gif->pass = MULT4PLUS2;
			*rowP = 2;
			break;
		case MULT4PLUS2:
			gif->pass = MULT2PLUS1;
			*rowP = 1;
			break;
		case MULT2PLUS1:
			/* We've read the entire image */
			break;
		}
	}
}


static void addPixelToRaster(struct gif_src *gif, unsigned int color, PALETTE cmap, unsigned int cmapSize)
{
	unsigned int x = gif->pnmBuffer.col + gif->pnmBuffer.left;
	unsigned int y = gif->pnmBuffer.row + gif->pnmBuffer.top;
	size_t bytes = pic_rowsize(gif->pic);
	unsigned char *pos = gif->pnmBuffer.pixels + bytes * y;
	unsigned char mask;
		
	if (color >= cmapSize)
	{
		GIF_ERROR((gif, "Invalid color index %u in an image that has only "
				 "%u colors in the color map.", color, cmapSize));
	}
	ASSERT(color < MAXCMAPSIZE);

	mask = 0x80 >> (x & 7);
	switch (gif->pic->pi_planes)
	{
	case 1:
		pos += (x >> 3);
		if (cmap[color].r == 0 &&
			cmap[color].g == 0 &&
			cmap[color].b == 0)
			*pos |= mask;
		else
			*pos &= ~mask;
		break;
	case 4:
		pos += (x >> 4) << 3;
		if (x & 0x08)
			pos++;
		if (color & 0x01) pos[0] |= mask;
		if (color & 0x02) pos[2] |= mask;
		if (color & 0x04) pos[4] |= mask;
		if (color & 0x08) pos[6] |= mask;
		break;
	case 8:
		pos += (x >> 4) << 4;
		if (x & 0x08)
			pos++;
		if (color & 0x01) pos[ 0] |= mask;
		if (color & 0x02) pos[ 2] |= mask;
		if (color & 0x04) pos[ 4] |= mask;
		if (color & 0x08) pos[ 6] |= mask;
		if (color & 0x10) pos[ 8] |= mask;
		if (color & 0x20) pos[10] |= mask;
		if (color & 0x40) pos[12] |= mask;
		if (color & 0x80) pos[14] |= mask;
		break;
	}

#if WRITE_OUTTXT
	fprintf(outf, "%02x ", color);
#endif
	
	if (gif->alphabits)
		gif->alphabits[gif->pnmBuffer.row][gif->pnmBuffer.col] = ((int)color == gif->transparent) ? 1 : 0;

	++gif->pnmBuffer.col;
	if (gif->pnmBuffer.col == gif->cols)
	{
#if WRITE_OUTTXT
		fprintf(outf, "\n");
#endif
		gif->pnmBuffer.col = 0;
		if (gif->interlaced)
			bumpInputRowInterlace(gif, &gif->pnmBuffer.row, gif->rows);
		else
			++gif->pnmBuffer.row;
	}
}




static void readImageData(struct gif_src *gif, PALETTE cmap, unsigned int cmapSize)
{
	unsigned char lzwMinCodeSize;
	int gotMinCodeSize;

	gif->pass = MULT8PLUS0;

	gif->pnmBuffer.col = 0;
	gif->pnmBuffer.row = 0;
	
	gotMinCodeSize = ReadOK(gif, &lzwMinCodeSize, 1);
	if (!gotMinCodeSize)
	{
		GIF_ERROR((gif, "GIF stream ends (or read error) right after an image separator; no image data follows."));
		return;
	}
	
	if (lzwMinCodeSize > MAX_LZW_BITS)
	{
		GIF_ERROR((gif, "Invalid minimum code size value in image data: %u.  "
				 "Maximum allowable code size in GIF is %u", lzwMinCodeSize, MAX_LZW_BITS));
		return;
	}
	
	GIF_VERBOSE_MESSAGE(("Image says the initial compression code size is %d bits", lzwMinCodeSize));
	
	lzwInit(&gif->decomp, lzwMinCodeSize);
	initGetCode(gif);

	GIF_VERBOSE_MESSAGE(("Initial code size is %u bits; clear code = 0x%x, "
		"end code = 0x%x", gif->decomp.init_codeSize, gif->decomp.clear_code, gif->decomp.end_code));
	
	while (gif->pnmBuffer.row < gif->rows)
	{
		int rc = lzwReadByte(gif);

		switch (rc)
		{
		case -3:
			GIF_ERROR((gif, "Error in GIF input stream"));
			return;
		case -2:
			GIF_ERROR((gif, "Error in GIF image: Not enough raster data to fill "
					 "%u x %u dimensions.  Ran out of raster data in " "row %u", gif->cols, gif->rows, gif->pnmBuffer.row));
			return;
		case -1:
			GIF_ERROR((gif, "Premature end of file; no proper GIF closing"));
			return;
		default:
			addPixelToRaster(gif, rc, cmap, cmapSize);
			break;
		}
	}
	if (lzwReadByte(gif) >= 0)
	{
		GIF_MESSAGE(("Extraneous data at end of image.  Skipped to end of image"));
	}
}



/*----------------------------------------------------------------------------
   If user wants verbose output, tell him that the color with index
   'transparentIndex' is supposed to be a transparent background color.
   
   If transparentIndex == -1, tell him there is no transparent background
   color.
-----------------------------------------------------------------------------*/
#if WRITE_OUTTXT
static void transparencyMessage(struct gif_src *gif, PALETTE cmap)
{
	if (gif->transparent == -1)
	{
		GIF_VERBOSE_MESSAGE(("no transparency"));
	} else
	{
		GIF_VERBOSE_MESSAGE(("transparent background color: rgb:%02x/%02x/%02x "
				   "Index %d",
				   cmap[gif->transparent].r,
				   cmap[gif->transparent].g,
				   cmap[gif->transparent].b,
				   gif->transparent));
	}
}
#else
#define transparencyMessage(gof, cmap)
#endif


static unsigned int gif_resolution_to_planes(unsigned int resolution)
{
	return resolution == 1 ? 1 : resolution <= 4 ? 4 : 8;
}


/*----------------------------------------------------------------------------
   Read the GIF stream header off the file gifFile, which is present
   positioned to the beginning of a GIF stream.  Return the info from it
   as *gifScreenP.
-----------------------------------------------------------------------------*/
static gboolean readGifHeader(struct gif_src *gif)
{
	unsigned char buf[16];

	if (!ReadOK(gif, buf, 6))
	{
		GIF_ERROR((gif, "error reading magic number"));
		return FALSE;
	}

	if (!(buf[0] == 'G' &&
		  buf[1] == 'I' &&
		  buf[2] == 'F'))
	{
		GIF_ERROR((gif, "File does not contain a GIF stream.  It does not start with 'GIF'."));
		return FALSE;
	}
	
	GIF_VERBOSE_MESSAGE(("GIF format version is '%c%c%c'", buf[3], buf[4], buf[5]));
	
	if (!(buf[3] == '8' &&
		  (buf[4] == '7' || buf[4] == '9') &&
		  buf[5] == 'a'))
	{
		GIF_ERROR((gif, "bad version number, not '87a' or '89a'"));
		return FALSE;
	}
	
	if (!ReadOK(gif, buf, 7))
	{
		GIF_ERROR((gif, "failed to read screen descriptor"));
		return FALSE;
	}
	
	gif->screen.Width = LM_to_uint(buf[0], buf[1]);
	gif->screen.Height = LM_to_uint(buf[2], buf[3]);
	gif->pic->pi_planes = (buf[4] & 0x07) + 1;
	gif->screen.ColorMapSize = 1 << gif->pic->pi_planes;
	gif->screen.ColorResolution = ((buf[4] & 0x70) >> 4) + 1;
	gif->screen.Background = buf[5];
	gif->screen.AspectRatio = buf[6];
	gif->pic->pi_width = gif->screen.Width;
	gif->pic->pi_height = gif->screen.Height;
	gif->pic->pi_planes = gif_resolution_to_planes(gif->pic->pi_planes);
	pic_normal_planes(gif->pic);
	
	if (gif->pnmBuffer.pixels == NULL)
	{
		size_t rowsize;
		size_t size;
		
		rowsize = pic_rowsize(gif->pic);
		size = rowsize * gif->screen.Height;
		gif->pnmBuffer.pixels = g_new0(unsigned char, size);
		if (gif->pnmBuffer.pixels == NULL)
			return FALSE;
	}
	
	GIF_VERBOSE_MESSAGE(("GIF Width = %d GIF Height = %d Pixel aspect ratio = %d",
		gif->screen.Width, gif->screen.Height, gif->screen.AspectRatio));
	GIF_VERBOSE_MESSAGE(("Colors = %d   Color Resolution = %d", gif->screen.ColorMapSize, gif->screen.ColorResolution));
	
	if (BitIsSet(buf[4], LOCALCOLORMAP))
	{									/* Global Colormap */
		readColorMap(gif, gif->screen.ColorMapSize, gif->screen.ColorMap,
					 &gif->screen.hasGray, &gif->screen.hasColor);
		GIF_VERBOSE_MESSAGE(("Color map %s grays, %s colors",
			gif->screen.hasGray ? "contains" : "doesn't contain",
			gif->screen.hasColor ? "contains" : "doesn't contain"));
	}
	return TRUE;
}



/*----------------------------------------------------------------------------
   Read extension blocks from the GIF stream to which the file input is
   positioned.  Read up through the image separator that begins the
   next image or GIF stream terminator.

   If we encounter EOD (end of GIF stream) before we find an image 
   separator, we return *eodP == TRUE.  Else *eodP == FALSE.

   If we hit end of file before an EOD marker, we abort the program with
   an error message.
-----------------------------------------------------------------------------*/
static void readExtensions(struct gif_src *gif, struct gif89 *gif89P, int * eodP)
{
	int imageStart;
	int eod;

	eod = FALSE;
	imageStart = FALSE;

	/* Read the image descriptor */
	while (!imageStart && !eod)
	{
		unsigned char c;

		if (!ReadOK(gif, &c, 1))
		{
			GIF_ERROR((gif, "EOF / read error on image data"));
			return;
		}
		
		if (c == ';')
		{								/* GIF terminator */
			eod = TRUE;
		} else if (c == '!')
		{								/* Extension */
			if (!ReadOK(gif, &c, 1))
			{
				GIF_ERROR((gif, "EOF / read error on extension function code"));
				return;
			}
			doExtension(gif, c, gif89P);
		} else if (c == ',')
		{
			imageStart = TRUE;
		} else
		{
			GIF_MESSAGE(("bogus character 0x%02x, ignoring", (int) c));
		}
	}
	*eodP = eod;
}



#if WRITE_OUTTXT
static void reportImageInfo(struct gif_src *gif, int useGlobalColormap, unsigned int localColorMapSize)
{
	GIF_VERBOSE_MESSAGE(("reading %u by %u%s GIF image", gif->cols, gif->rows, gif->interlaced ? " interlaced" : ""));
	if (gif->pnmBuffer.top != 0 || gif->pnmBuffer.left != 0)
		GIF_VERBOSE_MESSAGE(("located at %u.%u", gif->pnmBuffer.left, gif->pnmBuffer.top));
	if (useGlobalColormap)
		GIF_VERBOSE_MESSAGE(("  Uses global colormap"));
	else
		GIF_VERBOSE_MESSAGE(("  Uses local colormap of %u colors", localColorMapSize));
}
#else
#define reportImageInfo(gif, map, size)
#endif


/*----------------------------------------------------------------------------
   Read a single GIF image from the current position of file 'input'.

   If 'skipIt' is TRUE, don't do anything else.  Otherwise, write the
   image to the current position of files 'imageout_file' and 'alpha_file'.
   If 'alpha_file' is NULL, though, don't write any alpha information.
-----------------------------------------------------------------------------*/
static gboolean convertImage(struct gif_src *gif, int skipIt)
{
	unsigned char buf[16];
	int useGlobalColormap;
	
	/* The image alpha mask, in libpbm format.  NULL if we aren't computing
	   an alpha mask.
	 */
	PALETTE localColorMap;
	unsigned int localColorMapSize;

	if (!ReadOK(gif, buf, 9))
	{
		GIF_ERROR((gif, "couldn't read left/top/width/height"));
		return FALSE;
	}
	
	useGlobalColormap = !BitIsSet(buf[8], LOCALCOLORMAP);
	localColorMapSize = 1u << ((buf[8] & 0x07) + 1);
	gif->pnmBuffer.left = LM_to_uint(buf[0], buf[1]);
	gif->pnmBuffer.top = LM_to_uint(buf[2], buf[3]);
	gif->cols = LM_to_uint(buf[4], buf[5]);
	gif->rows = LM_to_uint(buf[6], buf[7]);
	gif->interlaced = BitIsSet(buf[8], INTERLACE);
	
	reportImageInfo(gif, useGlobalColormap, localColorMapSize);

	if (gif->alpha_file)
	{
		/* gif->alphabits = pbm_allocarray(gif->cols, gif->rows); */
		if (!gif->alphabits)
		{
			GIF_ERROR((gif, "couldn't alloc space for alpha image"));
			return FALSE;
		}
	} else
	{
		gif->alphabits = NULL;
	}
	
	if (!useGlobalColormap)
	{
		int hasGray, hasColor;

		readColorMap(gif, localColorMapSize, localColorMap, &hasGray, &hasColor);
		transparencyMessage(gif, localColorMap);
		readImageData(gif, localColorMap, localColorMapSize);
		if (!skipIt)
		{
			/* writePnm(gif, hasGray, hasColor); */
		}
	} else
	{
		transparencyMessage(gif, gif->screen.ColorMap);
		readImageData(gif, gif->screen.ColorMap, gif->screen.ColorMapSize);
		if (!skipIt)
		{
			/* writePnm(gif, gif->screen.hasGray, gif->screen.hasColor); */
		}
	}
	return TRUE;
}



/*----------------------------------------------------------------------------
   Read a GIF stream from file 'input' and write one or more images from
   it as PNM images to file 'imageout_file'.  If the images have transparency
   and 'alpha_file' is non-NULL, write PGM alpha masks to file 'alpha_file'.

   'allImages' means Caller wants all the images in the stream.  

   'requestedImageSeq' is meaningful only when 'allImages' is FALSE.  It 
   is the sequence number of the one image Caller wants from the stream,
   with the first image being 0.

   'drainInput' means to read the entire GIF stream, even after
   reading the image Caller asked for.  We read the stream, not just
   the file it's in, so we still recognize certain errors in the GIF
   format in the tail of the stream and there may yet be more stuff in
   the file when we return.
-----------------------------------------------------------------------------*/
static gboolean convertImages(struct gif_src *gif, gboolean allImages, int requestedImageSeq, gboolean drainStream)
{
	int imageSeq;
	/* Sequence within GIF stream of image we are currently processing.
	   First is 0.
	 */
	struct gif89 gif89;
	int eod;

	/* We've read through the GIF terminator character */

	initGif89(&gif89);

	if (readGifHeader(gif) == FALSE)
		return FALSE;

	for (imageSeq = 0, eod = FALSE; !eod && (imageSeq <= requestedImageSeq || allImages || drainStream); ++imageSeq)
	{
		readExtensions(gif, &gif89, &eod);
		gif->transparent = gif89.transparent;
		
		if (eod)
		{
			/* GIF stream ends before image with sequence imageSeq */
			if (!allImages && (imageSeq <= requestedImageSeq))
			{
				GIF_ERROR((gif, "You requested Image %d, but "
						 "only %d image%s found in GIF stream",
						 requestedImageSeq + 1, imageSeq, imageSeq > 1 ? "s" : ""));
				return FALSE;
			}
		} else
		{
			GIF_VERBOSE_MESSAGE(("Reading Image Sequence %d", imageSeq));
			if (!convertImage(gif, !allImages && (imageSeq != requestedImageSeq)))
				return FALSE;
		}
	}
	return TRUE;
}


gboolean gif_fread(_UBYTE **dest, FILE *fp, PICTURE *pic)
{
	struct gif_src *gif;
	gboolean retval;
	
	gif = g_new0(struct gif_src, 1);
	if (gif == NULL)
		return FALSE;
	gif->in.type = 0;
	gif->in.u.infile = fp;
	gif->pic = pic;
	
	if (*dest)
		gif->pnmBuffer.pixels = *dest;
	
	retval = convertImages(gif, FALSE, 0, FALSE);
	if (gif->got_errors)
		retval = FALSE;
	if (*dest == NULL)
	{
		if (retval == FALSE)
			g_free(gif->pnmBuffer.pixels);
		else
			*dest = gif->pnmBuffer.pixels;
	}
	
	g_free(gif);
	return retval;
}


gboolean gif_unpack(_UBYTE **dest, const _UBYTE *src, PICTURE *pic)
{
	struct gif_src *gif;
	gboolean retval;
	
	gif = g_new0(struct gif_src, 1);
	if (gif == NULL)
		return FALSE;
	gif->in.type = 1;
	gif->in.u.mem.src = src;
	gif->in.u.mem.pos = 0;
	gif->in.u.mem.len = pic->pi_filesize;
	gif->pic = pic;
	
	if (*dest)
		gif->pnmBuffer.pixels = *dest;
	
#if WRITE_OUTTXT
	{
		outf = fopen("gifout.txt", "w");
		{
			size_t i;
			fprintf(outf, "Data:\n");
			for (i = 0; i < gif->in.u.mem.len; i++)
			{
				if (((i) % 16) == 0)
					fprintf(outf, "$%04lx: ", i);
				fprintf(outf, "%02x ", src[i]);
				if (((i + 1) % 16) == 0)
					fprintf(outf, "\n");
			}
			fprintf(outf, "\n");
		}
	}
#endif
	
	retval = convertImages(gif, FALSE, 0, FALSE);
	if (gif->got_errors)
		retval = FALSE;
	if (*dest == NULL)
	{
		if (retval == FALSE)
			g_free(gif->pnmBuffer.pixels);
		else
			*dest = gif->pnmBuffer.pixels;
	}
	
	g_free(gif);
#if WRITE_OUTTXT
	fclose(outf);
#endif
	return retval;
}


gboolean pic_type_gif(PICTURE *pic, const unsigned char *buf, _LONG size)
{
	unsigned char flags;
	const unsigned char *end = buf + size;
	unsigned int i;
	
	if (!(size >= 13 &&
		  buf[0] == 'G' &&
		  buf[1] == 'I' &&
		  buf[2] == 'F' &&
		  buf[3] == '8' &&
		  (buf[4] == '7' || buf[4] == '9') &&
		  buf[5] == 'a'))
		return FALSE;

	pic->pi_type = FT_GIF;
	buf += 6;
	pic->pi_width = get_word();
	pic->pi_height = get_word();
	flags = get_byte();
	buf++; /* background */
	buf++; /* aspect ratio */
	pic->pi_planes = ((flags >> 4) & 0x07) + 1;
	pic->pi_planes = gif_resolution_to_planes(pic->pi_planes);
	if (BitIsSet(flags, LOCALCOLORMAP))
	{
		unsigned int ColorMapSize = 1 << ((flags & 0x07) + 1);

		for (i = 0; i < ColorMapSize && buf < end; i++)
		{
			pic->pi_palette[i].r = *buf++;
			pic->pi_palette[i].g = *buf++;
			pic->pi_palette[i].b = *buf++;
		}
	}
	/*
	 * impossible to figure out wether it was really compressed
	 * without decoding at least part of the image
	 */
	pic->pi_compressed = 1;
	pic->pi_dataoffset = 0;
	
	pic_calcsize(pic);
	return TRUE;
}
