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


static struct option const long_options[] = {
	{ "palette", no_argument, NULL, 'p' },
	
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
	char colors[40];
	char compressed[40];
	char unsupported[40];
	
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
		hyp_utf8_fprintf(stdout, "%s: empty file\n", filename);
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
	pic.pi_filesize = size;
	pic_format = pic_type(&pic, buf, size);
	g_free(buf);
	buf = NULL;
	if (pic.pi_planes <= 8)
		sprintf(colors, "x%u", 1 << pic.pi_planes);
	else if (pic.pi_planes <= 16)
		sprintf(colors, " hicolor-%u", pic.pi_planes);
	else
		sprintf(colors, " truecolor-%u", pic.pi_planes);
	strcpy(compressed, pic.pi_compressed ? " (compressed)" : " (uncompressed)");
	strcpy(unsupported, pic.pi_unsupported ? " (unsupported)" : "");
	
	switch (pic_format)
	{
	case FT_DEGAS_LOW:
		hyp_utf8_fprintf(stdout, "%s: Degas Low Resolution %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_DEGAS_MED:
		hyp_utf8_fprintf(stdout, "%s: Degas Medium Resolution %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_DEGAS_HIGH:
		hyp_utf8_fprintf(stdout, "%s: Degas High Resolution %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_NEO:
		hyp_utf8_fprintf(stdout, "%s: Neochrome %ux%u%s%s\n", filename, pic.pi_width, pic.pi_height, colors, unsupported);
		break;;
	case FT_IFF:
		hyp_utf8_fprintf(stdout, "%s: IFF %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_COLSTAR:
		hyp_utf8_fprintf(stdout, "%s: Colorstar %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_IMG:
		hyp_utf8_fprintf(stdout, "%s: GEM IMG %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_STAD:
		hyp_utf8_fprintf(stdout, "%s: Stad %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_IMAGIC_LOW:
		hyp_utf8_fprintf(stdout, "%s: Imagic Low Resolution %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_IMAGIC_MED:
		hyp_utf8_fprintf(stdout, "%s: Imagic Medium Resolution %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_IMAGIC_HIGH:
		hyp_utf8_fprintf(stdout, "%s: Imagic High Resolution %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_SCREEN:
		hyp_utf8_fprintf(stdout, "%s: Atari Screen Dump %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_ICO:
		hyp_utf8_fprintf(stdout, "%s: Windows Icon %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_CALAMUS_PAGE:
		hyp_utf8_fprintf(stdout, "%s: Calamus Print Page %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_BMP:
		hyp_utf8_fprintf(stdout, "%s: Windows Bitmap %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_GIF:
		hyp_utf8_fprintf(stdout, "%s: GIF %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_TIFF:
		hyp_utf8_fprintf(stdout, "%s: TIFF %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_TARGA:
		hyp_utf8_fprintf(stdout, "%s: Targa %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_PBM:
		hyp_utf8_fprintf(stdout, "%s: Portable Bitmap %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_ICN:
		hyp_utf8_fprintf(stdout, "%s: GEM Icon %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
	case FT_PNG:
		hyp_utf8_fprintf(stdout, "%s: Portable Network Graphic %ux%u%s%s%s\n", filename, pic.pi_width, pic.pi_height, colors, compressed, unsupported);
		break;;
		
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
	case FT_EMPTY:
	case FT_DRI:
	case FT_DRILIB:
	case FT_BOBJECT:
	case FT_RSC:
	case FT_GFA2:
	case FT_GFA3:
	default:
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unknown picture format"));
		break;;
	}
	
	if (show_palette && pic.pi_planes <= 8)
	{
		_WORD ncolors = 1 << pic.pi_planes;
		_WORD i;
		
		for (i = 0; i < ncolors; i++)
		{
			hyp_utf8_fprintf(stdout, "  %3d: #%02x%02x%02x\n", i, pic.pi_palette[i].r, pic.pi_palette[i].g, pic.pi_palette[i].b);
		}
	}
	
	goto done;

error:
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
	while ((c = getopt_long_only_r(argc, argv, "phV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case 'p':
			show_palette = TRUE;
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
