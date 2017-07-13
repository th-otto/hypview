#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef FALSE
#  define FALSE 0
#  define TRUE 1
#endif

#define BMP_PNG_MAGIC 0x474e5089

/*
 * for reference:
 */
typedef struct {
	unsigned char magic[2];
	unsigned char type[2];
	unsigned char count[2];
	struct {
		unsigned char width;		/* width in pixel */
		unsigned char height;		/* height in pixel */
		unsigned char colors;		/* number of colors: 2, 8, 16, 256 */
		unsigned char dummy;
		unsigned char xhotspot[2];
		unsigned char yhotspot[2];
		unsigned char datasize[4];  /* size of BITMAPINFO + data */
		unsigned char offset[4];	/* offset to BITMAPINFO (22 for 1 icon) */
	} ico[1]; /* repeated for #count */
#define ICO_FILE_HEADER_SIZE 22
	
	unsigned char biSize[4];	/* size of BITMAPINFO (12/40/64) */
	unsigned char biWidth[4];
	unsigned char biHeight[4];
	unsigned char biPlanes[2];
	unsigned char biBitCount[2];
	unsigned char biCompression[4];
	unsigned char biSizeImage[4];
	unsigned char biXPelsPerMeter[4];
	unsigned char biYPelsPerMeter[4];
	unsigned char biClrUsed[4];
	unsigned char biClrImportant[4];
	unsigned char bmiColors[256][4];
} ICO_HEADER;


static FILE *in;

static unsigned char get_byte(void)
{
	int c = getc(in);
	if (c == EOF)
	{
		fprintf(stderr, "unexpected end of file\n");
		exit(1);
	}
	return (unsigned char)c;
}

static unsigned short get_word(void)
{
	unsigned int b1, b2;
	b1 = get_byte();
	b2 = get_byte();
	return ((b2 & 0xff) << 8) | (b1 & 0xff);
}


static unsigned int get_long(void)
{
	unsigned int b1, b2, b3, b4;
	b1 = get_byte();
	b2 = get_byte();
	b3 = get_byte();
	b4 = get_byte();
	return (((((((b4 & 0xff) << 8) | (b3 & 0xff)) << 8) | (b2 & 0xff)) << 8) | (b1 & 0xff));
}



static unsigned int bmp_rowsize(unsigned int width, unsigned int planes)
{
	unsigned int bmp_bytes = width;
	
	switch (planes)
	{
	case 1:
		/* get_dib_stride */
		bmp_bytes = ((((bmp_bytes) + 7) >> 3) + 3) & ~3;
		break;
	case 4:
		bmp_bytes = (((bmp_bytes + 1) >> 1) + 3) & ~3;
		break;
	case 8:
		bmp_bytes = (bmp_bytes + 3) & ~3;
		break;
	case 15:
	case 16:
		bmp_bytes = ((bmp_bytes * 2) + 3) & ~3;
		break;
	case 24:
		bmp_bytes = ((bmp_bytes * 3) + 3) & ~3;
		break;
	case 32:
		bmp_bytes = bmp_bytes << 2;
		break;
	default:
		bmp_bytes = 0;
		break;
	}
	return bmp_bytes;
}


static void dumpdata(unsigned int w, unsigned int h)
{
	unsigned int x, y;
	
	for (y = 0; y < h; y++)
	{
		printf("\t\t");
		for (x = 0; x < w; x++)
		{
			printf(" %02x", get_byte());
		}
		printf("\n");
	}
}


static int dofile(const char *name)
{
	unsigned short version;
	unsigned short type;
	unsigned short count;
	unsigned int width, colorcount, filler;
	int height;
	int topdown;
	unsigned int planes, bitcount;
	unsigned int bytes, offset;
	unsigned short i;
	long pos;
	
	in = fopen(name, "rb");
	if (in == NULL)
	{
		fprintf(stderr, "%s: %s\n", name, strerror(errno));
		return FALSE;
	}
	printf("%s:\n", name);
	version = get_word();
	type = get_word();
	count = get_word();
	printf("version: %u\n", version);
	printf("type: %u\n", type);
	printf("count: %u\n", count);
	
	pos = ftell(in);
	for (i = 0; i < count; i++)
	{
		fseek(in, pos, SEEK_SET);
		width = get_byte();
		height = get_byte();
		colorcount = get_byte();
		filler = get_byte();
		planes = get_word();
		bitcount = get_word();
		bytes = get_long();
		offset = get_long();
		pos = ftell(in);
		printf("icon %u:\n", i);
		printf("\twidth: %u\n", width);
		printf("\theight: %u\n", height);
		printf("\tcolorcount: %u\n", colorcount);
		printf("\tfiller: %u\n", filler);
		if (type == 2)
		{
			printf("\txhotspot: %u\n", planes);
			printf("\tyhotspot: %u\n", bitcount);
		} else
		{
			printf("\tplanes: %u\n", planes);
			printf("\tbits per pixel: %u\n", bitcount);
		}
		printf("\tdata bytes: %u $%x\n", bytes, bytes);
		printf("\tfile offset: %u $%x\n", offset, offset);
		fseek(in, offset, SEEK_SET);
		{
			unsigned int hsize = get_long();
			int mapentrysize;
			unsigned int clr_used;
			unsigned int bytes_per_row;
			unsigned int datasize, masksize;
			
			mapentrysize = 0;
			clr_used = 0;
			if (hsize == 12)
			{
				/* OS/2 1.x, BITMAPCOREHEADER */
				printf("\tBITMAPCOREHEADER:\n");
				width = get_word();
				height = get_word();
				planes = get_word();
				bitcount = get_word();
				if (bitcount >= 1 && bitcount <= 8)
					mapentrysize = 3;
				printf("\twidth: %u\n", width);
				topdown = height < 0;
				if (height < 0)
					height = -height;
				printf("\theight: %d%s\n", height, topdown ? " (topdown)" : "");
				printf("\tplanes: %u\n", planes);
				printf("\tbits per pixel: %u\n", bitcount);
			} else if (hsize == BMP_PNG_MAGIC)
			{
				printf("\tPNG:\n");
				hsize = 0;
				
			} else if (hsize >= 40)
			{
				unsigned int compression, compressed_size;
				unsigned int x_res, y_res;
				unsigned int clr_important;
				
				printf("\tBITMAPINFOHEADER(%u):\n", hsize);
				width = get_long();
				height = get_long();
				planes = get_word();
				bitcount = get_word();
				compression = get_long();
				compressed_size = get_long();
				x_res = get_long();
				y_res = get_long();
				clr_used = get_long();
				clr_important = get_long();
				if (bitcount >= 1 && bitcount <= 8)
					mapentrysize = 4;
				fseek(in, hsize - 40, SEEK_CUR);
				topdown = height < 0;
				if (height < 0)
					height = -height;
				printf("\twidth: %u\n", width);
				printf("\theight: %d%s\n", height, topdown ? " (topdown)" : "");
				printf("\tplanes: %u\n", planes);
				printf("\tbits per pixel: %u\n", bitcount);
				printf("\tcompression: %u\n", compression);
				printf("\tcompressed size: %u $%x\n", compressed_size, compressed_size);
				printf("\tx-res: %u\n", x_res);
				printf("\ty-res: %u\n", y_res);
 				printf("\tcolors used: %u\n", clr_used);
 				printf("\tcolors important: %u\n", clr_important);
			} else
			{
				return FALSE;
			}
			
			if (hsize != 0)
			{
				if (mapentrysize > 0)
				{
					unsigned int c;
					
					if (clr_used == 0)
						clr_used = 1 << bitcount;
					else if (clr_used > (1u << bitcount))
						return FALSE;
					for (c = 0; c < clr_used; c++)
					{
						unsigned char blue = get_byte();
						unsigned char green = get_byte();
						unsigned char red = get_byte();
						if (mapentrysize > 3)
							(void) get_byte();
						printf("\tcolor %u: #%02x%02x%02x\n", c, red, green, blue);
					}
				}
				height /= 2;
				bytes_per_row = bmp_rowsize(width, bitcount);
				datasize = bytes_per_row * height;
				printf("\tdatasize: %u $%x\n", datasize, datasize);
				dumpdata(bytes_per_row, height);
				bytes_per_row = bmp_rowsize(width, 1);
				masksize = bytes_per_row * height;
				printf("\tmasksize: %u $%x\n", masksize, masksize);
				dumpdata(bytes_per_row, height);
			}
		}
	}
	fclose(in);
	return TRUE;
}


int main(int argc, const char **argv)
{
	while (--argc)
		if (!dofile(*++argv))
			return 1;
	return 0;
}
