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

char const gl_program_name[] = "bmp2gif";
char const gl_program_version[] = HYP_VERSION;

static gboolean do_help = FALSE;
static gboolean do_version = FALSE;
static int compress = -1;

static struct option const long_options[] = {
	{ "compress", no_argument, NULL, 'c' },
	{ "no-compress", no_argument, NULL, 'c' + 256 },
	
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

static gboolean conv_file(const char *filename)
{
	PICTURE pic;
	unsigned char *dest = NULL;
	unsigned char *buf = NULL;
	long size;
	FILE *fp = NULL;
	gboolean retval = TRUE;
	char *outname = NULL;
	FILE *out = NULL;
	
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		hyp_utf8_fprintf(stderr, _("can't open file '%s': %s\n"), filename, strerror(errno));
		goto error;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = g_new(unsigned char, size);
	if (buf == NULL)
	{
		oom();
		goto error;
	}
	fread(buf, 1, size, fp);
	pic_init(&pic);
	pic.pi_filesize = size;
	if (pic_type(&pic, buf, size) != FT_BMP)
	{
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unknown picture format"));
		goto error;
	}
	if (pic.pi_unsupported)
	{
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unsupported picture format"));
		goto error;
	}
	
	dest = g_new(unsigned char, pic.pi_picsize);
	if (dest == NULL)
	{
		oom();
		goto error;
	}
	if (bmp_unpack(dest, buf + pic.pi_dataoffset, &pic) == FALSE)
	{
		hyp_utf8_fprintf(stderr, _("%s: failed to decode\n"), filename);
		goto error;
	}
	
	g_free(buf);
	buf = NULL;
	if (compress == 1)
		pic.pi_compressed = 1;
	else if (compress == 0)
		pic.pi_compressed = 0;
	else
		pic.pi_compressed = 1;

	pic_calcsize(&pic);
	
	outname = replace_ext(filename, NULL, ".gif");
	if (outname == NULL)
	{
		oom();
		goto error;
	}
	
	out = fopen(outname, "wb");
	if (out == NULL)
	{
		hyp_utf8_fprintf(stderr, _("can't create file '%s': %s\n"), outname, strerror(errno));
		goto error;
	}

	if (!gif_fwrite(out, dest, &pic) ||
		fflush(out) != 0 ||
		ferror(out))
	{
		hyp_utf8_fprintf(stderr, _("%s: write error: %s\n"), outname, strerror(errno));
		goto error;
	}

	hyp_utf8_fprintf(stdout, _("created %s %d x %d x %d\n"), outname, pic.pi_width, pic.pi_height, pic.pi_planes);
	
	goto done;

error:
	retval = FALSE;
done:
	g_free(outname);
	g_free(dest);
	g_free(buf);
	if (fp)
		fclose(fp);
	if (out)
		fclose(out);
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
	while ((c = getopt_long_only_r(argc, argv, "chV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case 'c':
			compress = TRUE;
			break;
		case 'c' + 256:
			compress = FALSE;
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
				if (!conv_file(filename))
					retval = 1;
			}
		}
	}
	
	getopt_finish_r(&d);
	x_free_resources();
	
	return retval;
}
