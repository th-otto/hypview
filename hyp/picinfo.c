#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "portab.h"

#include "hypdefs.h"
#include "picture.h"
#include "hypdebug.h"
#include "xgetopt.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "hv_vers.h"

char const gl_program_name[] = "picinfo";
char const gl_program_version[] = HYP_VERSION;

static gboolean do_help = FALSE;
static gboolean do_version = FALSE;
static gboolean show_palette = FALSE;
static gboolean print_histogram = FALSE;


static struct option const long_options[] = {
	{ "palette", no_argument, NULL, 'p' },
	{ "histogram", no_argument, NULL, 'H' },
	
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	
	{ NULL, no_argument, NULL, 0 }
};

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void oom(void)
{
	hyp_utf8_fprintf(stderr, "%s: %s\n", gl_program_name, strerror(ENOMEM));
}

/* ------------------------------------------------------------------------- */

static void usage_error(const char *msg, ...)
{
	va_list args;
	
	hyp_utf8_fprintf(stderr, "%s: ", gl_program_name);
	va_start(args, msg);
	hyp_utf8_vfprintf(stderr, msg, args);
	va_end(args);
	hyp_utf8_fprintf(stderr, "\n");
}

/* ------------------------------------------------------------------------- */

static void print_version(FILE *out)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information)."), gl_program_name, HYP_URL);
	char *compiler = hyp_compiler_version();
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n"
		"Using %s\n"
		"%s\n",
		gl_program_name, gl_program_version,
		HYP_COPYRIGHT,
		printnull(compiler),
		printnull(url));
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", printnull(msg));
	g_free(msg);
	g_free(compiler);
	g_free(url);
}

/* ------------------------------------------------------------------------- */

static void print_usage(FILE *out)
{
	print_version(out);
	hyp_utf8_fprintf(out, _("usage: %s [-options] file1 [file2 ...]\n"), gl_program_name);
	hyp_utf8_fprintf(out, _("options:\n"));
	hyp_utf8_fprintf(out, _("  -p, --palette                 print palette values\n"));
	hyp_utf8_fprintf(out, _("  -H, --histogram               print a histogram\n"));
	hyp_utf8_fprintf(out, _("  -h, --help                    print help and exit\n"));
	hyp_utf8_fprintf(out, _("  -V, --version                 print version and exit\n"));
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

struct hist {
	unsigned long count;
	unsigned char pixel;
};

static int cmp_hist(const void *_p1, const void *_p2)
{
	const struct hist *p1 = (const struct hist *)_p1;
	const struct hist *p2 = (const struct hist *)_p2;
	
	if (p1->count > p2->count)
		return -1;
	if (p1->count < p2->count)
		return 1;
	return 0;
}

/* ------------------------------------------------------------------------- */

static gboolean histogram(PICTURE *pic, const char *filename)
{
	struct hist histo[256];
	unsigned char *dest = NULL;
	gboolean retval = TRUE;
	unsigned long rowsize;
	int x, y;
	int planes;
	int i, colors;
	long count = 0;
	const char *transparency;
	
	if (pic->pi_unsupported)
	{
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unsupported picture format"));
		goto error;
	}
	
	dest = g_new(unsigned char, pic->pi_picsize);
	if (dest == NULL)
	{
		oom();
		goto error;
	}
	
	switch (pic->pi_type)
	{
	case FT_BMP:
		if (bmp_unpack(dest, pic->pi_buf + pic->pi_dataoffset, pic, FALSE) == FALSE)
		{
			hyp_utf8_fprintf(stderr, _("%s: failed to decode\n"), filename);
			goto error;
		}
		break;

	case FT_ICO:
		if (ico_unpack(dest, pic->pi_buf + pic->pi_dataoffset, pic, FALSE) == FALSE)
		{
			hyp_utf8_fprintf(stderr, _("%s: failed to decode\n"), filename);
			goto error;
		}
		break;

	case FT_GIF:
		if (gif_unpack(&dest, pic->pi_buf, pic) == FALSE)
		{
			hyp_utf8_fprintf(stderr, _("%s: failed to decode\n"), filename);
			goto error;
		}
		break;
		
	case FT_IMG:
		if (img_unpack_safe(dest, pic->pi_buf + pic->pi_dataoffset, pic) == FALSE)
		{
			hyp_utf8_fprintf(stderr, _("%s: failed to decode\n"), filename);
			goto error;
		}
		break;
	
	default:
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unimplemented picture format"));
		break;
	}
	
	memset(histo, 0, sizeof(histo));
	for (i = 0; i < 256; i++)
		histo[i].pixel = i;

	rowsize = pic_rowsize(pic, pic->pi_planes);
	for (y = 0; y < pic->pi_height; y++)
	{
		const unsigned char *src = dest + y * rowsize;
		
		for (x = 0; x < pic->pi_width; x++)
		{
			const unsigned char *p = src + (x >> 3);
			unsigned char mask = 0x80 >> (x & 0x07);
			unsigned char colmask = 0x01;
			unsigned char pixel = 0;
			for (planes = 0; planes < pic->pi_planes; planes++)
			{
				if (*p & mask)
					pixel |= colmask;
				colmask <<= 1;
				p += 2;
			}
			histo[pixel].count++;
		}
	}
	qsort(histo, 256, sizeof(histo[0]), cmp_hist);
	colors = 1 << pic->pi_planes;
	transparency = pic->pi_transparent >= 0 ? "+1" : "";
	for (i = 0; i < colors; i++)
	{
		unsigned char pixel = histo[i].pixel;
		if (histo[i].count != 0)
		{
			hyp_utf8_fprintf(stdout, "  %3d: #%02x%02x%02x: %lu\n", pixel, pic->pi_palette[pixel].r, pic->pi_palette[pixel].g, pic->pi_palette[pixel].b, histo[i].count);
			count++;
			if (pixel == pic->pi_transparent)
				transparency = "";
		}
	}
	printf(_("%ld%s colors used\n"), count, transparency);
	fflush(stdout);
	
	goto done;

error:
	fflush(stdout);
	fflush(stderr);
	retval = FALSE;
done:
	g_free(dest);
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean identify_file(const char *filename)
{
	PICTURE pic;
	unsigned char *buf = NULL;
	long size, ret;
	FILE *fp = NULL;
	gboolean retval = TRUE;
	pic_filetype pic_format;
	char *colors;
	const char *compressed;
	const char *unsupported;
	char *colordiff;
	char *transparency;
	
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		hyp_utf8_fprintf(stderr, _("can't open file '%s': %s\n"), filename, strerror(errno));
		goto error;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (size == 0)
	{
		hyp_utf8_fprintf(stdout, _("%s: empty file\n"), filename);
		goto done;
	}
	buf = g_new(unsigned char, size);
	if (buf == NULL)
	{
		oom();
		goto error;
	}
	ret = fread(buf, 1, size, fp);
	if (ret != size)
	{
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		goto error;
	}
	pic_init(&pic);
	pic.pi_buf = buf;
	pic.pi_filesize = size;
	pic_format = pic_type(&pic, buf, size);
	colors = pic_colornameformat(pic.pi_planes);
	compressed = pic.pi_compressed ? _(" (compressed)") : _(" (uncompressed)");
	unsupported = pic.pi_unsupported ? _(" (unsupported)") : "";
	if (pic_format >= FT_PICTURE_FIRST && pic_format <= FT_PICTURE_LAST)
	{
		unsigned long stddiff = pic_pal_stddiff(&pic);
		if (stddiff != 0)
			colordiff = g_strdup_printf(_(" paldiff %lu"), stddiff);
		else
			colordiff = g_strdup("");;
	} else
	{
		colordiff = g_strdup("");;
	}
	if (pic.pi_transparent >= 0)
		transparency = g_strdup_printf(_(" transparent %d"), pic.pi_transparent);
	else
		transparency = g_strdup("");
	
	switch (pic_format)
	{
	case FT_DEGAS_LOW:
		hyp_utf8_fprintf(stdout, "%s: Degas Low Resolution %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_DEGAS_MED:
		hyp_utf8_fprintf(stdout, "%s: Degas Medium Resolution %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_DEGAS_HIGH:
		hyp_utf8_fprintf(stdout, "%s: Degas High Resolution %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_NEO:
		hyp_utf8_fprintf(stdout, "%s: Neochrome %ux%u%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, unsupported, transparency, colordiff);
		break;
	case FT_IFF:
		hyp_utf8_fprintf(stdout, "%s: IFF %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_COLSTAR:
		hyp_utf8_fprintf(stdout, "%s: Colorstar %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_IMG:
		hyp_utf8_fprintf(stdout, "%s: GEM IMG %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_STAD:
		hyp_utf8_fprintf(stdout, "%s: Stad %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_IMAGIC_LOW:
		hyp_utf8_fprintf(stdout, "%s: Imagic Low Resolution %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_IMAGIC_MED:
		hyp_utf8_fprintf(stdout, "%s: Imagic Medium Resolution %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_IMAGIC_HIGH:
		hyp_utf8_fprintf(stdout, "%s: Imagic High Resolution %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_SCREEN:
		hyp_utf8_fprintf(stdout, "%s: Atari Screen Dump %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_ICO:
		hyp_utf8_fprintf(stdout, "%s: Windows Icon %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_CALAMUS_PAGE:
		hyp_utf8_fprintf(stdout, "%s: Calamus Print Page %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_BMP:
		hyp_utf8_fprintf(stdout, "%s: Windows Bitmap %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_GIF:
		hyp_utf8_fprintf(stdout, "%s: GIF %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_TIFF:
		hyp_utf8_fprintf(stdout, "%s: TIFF %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_TARGA:
		hyp_utf8_fprintf(stdout, "%s: Targa %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_PBM:
		hyp_utf8_fprintf(stdout, "%s: Portable Bitmap %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_ICN:
		hyp_utf8_fprintf(stdout, "%s: GEM Icon %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;
	case FT_PNG:
		hyp_utf8_fprintf(stdout, "%s: Portable Network Graphic %ux%u%s%s%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, transparency, colordiff, unsupported);
		break;

	case FT_EMPTY:
		hyp_utf8_fprintf(stdout, _("%s: empty file\n"), filename);
		break;
		
	case FT_UNKNOWN:
	case FT_EXEC_FIRST:
	case FT_EXEC:
	case FT_TOS:
	case FT_TTP:
	case FT_PRG:
	case FT_GTP:
	case FT_EXEC_LAST:
	case FT_PICTURE_FIRST:
	case FT_PICTURE_LAST:
	case FT_ARCHIVE_FIRST:
	case FT_ARC:
	case FT_ZOO:
	case FT_LZH:
	case FT_ZIP:
	case FT_ARJ:
	case FT_TAR:
	case FT_GZ:
	case FT_BZ2:
	case FT_ARCHIVE_LAST:
	case FT_DOC_FIRST:
	case FT_ASCII:
	case FT_WORDPLUS:
	case FT_SIGDOC:
	case FT_DOC_LAST:
	case FT_FONT_FIRST:
	case FT_GEMFNT:
	case FT_SIGFNT:
	case FT_FONT_LAST:
	case FT_MISC_FIRST:
	case FT_DRI:
	case FT_DRILIB:
	case FT_BOBJECT:
	case FT_RSC:
	case FT_GFA2:
	case FT_GFA3:
	default:
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unknown picture format"));
		break;
	}
	
	if (show_palette && pic_format >= FT_PICTURE_FIRST && pic_format <= FT_PICTURE_LAST && pic.pi_planes <= 8)
	{
		_WORD ncolors = 1 << pic.pi_planes;
		_WORD i;
		
		fflush(stdout);
		for (i = 0; i < ncolors; i++)
		{
			hyp_utf8_fprintf(stdout, "  %3d: #%02x%02x%02x\n", i, pic.pi_palette[i].r, pic.pi_palette[i].g, pic.pi_palette[i].b);
		}
	}
	
	if (print_histogram && pic_format >= FT_PICTURE_FIRST && pic_format <= FT_PICTURE_LAST)
	{
		fflush(stdout);
		if (pic.pi_planes > 8)
		{
			hyp_utf8_fprintf(stderr, _("%s: histogram for %s NYI\n"), filename, colors);
		} else
		{
			histogram(&pic, filename);
		}
	}
	
	g_free(colordiff);
	g_free(colors);
	g_free(transparency);
	
	goto done;

error:
	fflush(stdout);
	fflush(stderr);
	retval = FALSE;
done:
	g_free(buf);
	if (fp)
		fclose(fp);
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#include "hypmain.h"

int main(int argc, const char **argv)
{
	int retval = 0;
	const char *filename;
	int c;
	struct _getopt_data *d;
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(argc, argv, "pHhV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case 'p':
			show_palette = TRUE;
			break;
		
		case 'H':
			print_histogram = TRUE;
			break;
		
		case 'h':
			do_help = TRUE;
			break;
		case 'V':
			do_version = TRUE;
			break;

		case '?':
			if (getopt_opt_r(d) == '?')
			{
				do_help = TRUE;
			} else
			{
				retval = 1;
			}
			break;
		
		case 0:
			/* option which just sets a var */
			break;
		
		default:
			/* error message already issued */
			retval = 1;
			break;
		}
	}
	c = getopt_ind_r(d);
	
	if (retval != 0)
	{
	} else if (do_version)
	{
		print_version(stdout);
	} else if (do_help)
	{
		print_usage(stdout);
	} else
	{
		if (c == argc)
		{
			usage_error(_("no files specified"));
			retval = 1;
		} else
		{
			while (c < argc)
			{
				filename = argv[c++];
				if (!identify_file(filename))
					retval = 1;
			}
		}
	}
	
	getopt_finish_r(&d);
	x_free_resources();
	
	return retval;
}
