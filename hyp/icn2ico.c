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

char const gl_program_name[] = "icn2ico";
char const gl_program_version[] = HYP_VERSION;

static gboolean do_help = FALSE;
static gboolean do_version = FALSE;


static struct option const long_options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	
	{ NULL, no_argument, NULL, 0 }
};

static unsigned char const ico_coltab8[256] = {
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
static unsigned char const ico_revtab8[256] = {
   1,   2,   3,   6,   4,   7,   5,   8,   9,  10,  11,  14,  12,  15,  13, 255,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
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

static unsigned char const ico_coltab4[16] = { 15, 9, 10, 11, 12, 13, 14, 8, 7, 1, 2, 3, 4, 5, 6, 0 };
static unsigned char const ico_revtab4[16] = { 15, 9, 10, 11, 12, 13, 14, 8, 7, 1, 2, 3, 4, 5, 6, 0 };

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
	long headerlen;
	long datalen;
	long masksize;
	const unsigned char *maptab;
	const unsigned char *revtab;
	
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
	if (pic_type(&pic, buf, size) != FT_ICN)
	{
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unknown picture format"));
		goto error;
	}
	if (pic.pi_unsupported)
	{
		hyp_utf8_fprintf(stderr, "%s: %s\n", filename, _("unsupported picture format"));
		goto error;
	}
	
	masksize = pic_rowsize(&pic, 1) * pic.pi_height;
	pic.pi_datasize = pic.pi_picsize + masksize;
	dest = g_new(unsigned char, pic.pi_datasize);
	if (dest == NULL)
	{
		oom();
		goto error;
	}
	if (icn_unpack(dest, buf, &pic, TRUE) == FALSE)
	{
		hyp_utf8_fprintf(stderr, _("%s: failed to decode\n"), filename);
		goto error;
	}
	
	g_free(buf);
	buf = NULL;
	
	if (pic.pi_planes <= 8 && pic.pi_planes != 1)
	{
		/*
		 * ICN reader reads separate planes, but other converters
		 * expect interleaved planes
		 */
		unsigned char *planebuf = g_new(unsigned char, pic.pi_datasize);
		if (planebuf == NULL)
		{
			oom();
			goto error;
		}
		pic_planes_to_interleaved(planebuf, dest, &pic);
		memcpy(planebuf + pic.pi_picsize, dest + pic.pi_picsize, masksize);
		g_free(dest);
		dest = planebuf;
	}

	outname = replace_ext(filename, ".icn", ".ico");
	
	out = fopen(outname, "wb");
	if (out == NULL)
	{
		hyp_utf8_fprintf(stderr, _("can't create file '%s': %s\n"), outname, strerror(errno));
		goto error;
	}
	
	maptab = pic.pi_planes == 4 ? ico_coltab4 : ico_coltab8;
	revtab = pic.pi_planes == 4 ? ico_revtab4 : ico_revtab8;
	headerlen = ico_header(&buf, &pic, maptab);
	datalen = ico_pack(buf, dest, dest + pic.pi_picsize, &pic, revtab);
	if ((long) fwrite(buf, 1, headerlen + datalen, out) != headerlen + datalen ||
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
	while ((c = getopt_long_only_r(argc, argv, "hV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
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
	
	return retval;
}
