#define CC_FOR_BUILD 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#undef HAVE_GLIB
#undef HAVE_GTK
#include "windows_.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string_.h"
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "time_.h"
#include <sys/stat.h>
#include "stat_.h"
#include "portab.h"
#include "hypmem.h"
#include "hypintl.h"
#include <gettext-po.h>

/*
 * if this file is used in a cross-compilation environment,
 * it is compiled for the build system.
 * We cannot use the libraries that were compiled for the target,
 * so just include the needed sources here for simplicity
 */
#include "xgetopt.h"
#include "../hyp/xgetopt.c"
#include "../hyp/hyp_glib.c"
#include "../rcintl/localenm.c"


char const gl_program_name[] = "po2rc";

static int num_errors;
static int num_warnings;
static gboolean bShowVersion;
static gboolean bShowHelp;
static GSList *filelist;
static const char *podir = "../po";

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void handle_error(int severity)
{
	switch (severity)
	{
	case PO_SEVERITY_WARNING:
		num_warnings++;
		break;
	case PO_SEVERITY_ERROR:
		num_errors++;
		break;
	case PO_SEVERITY_FATAL_ERROR:
	default:
		abort();
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void my_xerror(
	int severity,
	po_message_t message,
	const char *filename,
	size_t lineno,
	size_t column,
	int multiline_p,
	const char *message_text)
{
	UNUSED(message);
	UNUSED(column);
	UNUSED(multiline_p);
	fprintf(stderr, "%s: %s:%ld: %s\n", gl_program_name, filename, (long)lineno, message_text);
	handle_error(severity);
}

/*** ---------------------------------------------------------------------- ***/

/* Signal a problem that refers to two messages.
   Similar to two calls to xerror.
     If possible, a "..." can be appended to MESSAGE_TEXT1 and prepended to
     MESSAGE_TEXT2.  */
static void my_xerror2(
	int severity,
	po_message_t message1,
	const char *filename1, size_t lineno1, size_t column1,
	int multiline_p1, const char *message_text1,
	po_message_t message2,
	const char *filename2, size_t lineno2, size_t column2,
	int multiline_p2, const char *message_text2)
{
	UNUSED(message1);
	UNUSED(column1);
	UNUSED(multiline_p1);
	UNUSED(message2);
	UNUSED(column2);
	UNUSED(multiline_p2);
	fprintf(stderr, "%s: %s:%ld: %s\n", gl_program_name, filename1, (long)lineno1, message_text1);
	fprintf(stderr, "%s: %s:%ld: %s\n", gl_program_name, filename2, (long)lineno2, message_text2);
	handle_error(severity);
}

static const struct po_xerror_handler my_xerror_handler = {
	my_xerror,
	my_xerror2
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void quote(FILE *out, const char *prefix, const char *str)
{
	const char *lf;
	
	fputs(prefix, out);
	fputc('"', out);
	if ((lf = strchr(str, 0x0a)) != NULL && lf[1] != '\0')
	{
		fputs("\"\n\"", out);
	}
	while (*str)
	{
		unsigned char c = *str;
		
		switch (c)
		{
		case 0x0d:
			fputs("\\r", out);
			break;
		case 0x0a:
			fputs("\\n\"\n", out);
			if (str[1] == '\0')
				return;
			fputc('"', out);
			break;
		case '\t':
			fputs("\\t", out);
			break;
		case '\v':
			fputs("\\v", out);
			break;
		case '\f':
			fputs("\\f", out);
			break;
		case '\b':
			fputs("\\b", out);
			break;
		case '\\':
			fputs("\\\\", out);
			break;
		case '"':
			fputs("\\\"", out);
			break;
		default:
			if (c < 0x20 || c >= 0x80)
			{
				fputc('\\', out);
				fputc('0' + ((c >> 6) & 7), out);
				fputc('0' + ((c >> 3) & 7), out);
				fputc('0' + (c & 7), out);
			} else
			{
				fputc(c, out);
			}
			break;
		}
		str++;
	}
	fputc('"', out);
	fputc('\n', out);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean dofile(const char *input_filename)
{
	po_file_t file = po_file_read(input_filename, &my_xerror_handler);
	const char *const *domains;
	po_message_iterator_t iter;
	po_message_t msg;
	FILE *out = stdout;
	
	if (file == NULL)
	{
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, strerror(errno));
		return FALSE;
	}
	domains = po_file_domains(file);
	if (domains == NULL || domains[0] == NULL || strcmp(domains[0], "messages") != 0)
	{
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("not a messages file"));
		po_file_free(file);
		return FALSE;
	}
	iter = po_message_iterator(file, NULL);
	
	while ((msg = po_next_message(iter)) != NULL)
	{
		const char *msgid;
		const char *trans;
		const char *context;
		const char *plural;
		
		if (po_message_is_obsolete(msg))
			continue;
		msgid = po_message_msgid(msg);
		trans = po_message_msgstr(msg);
		context = po_message_msgctxt(msg);
		plural = po_message_msgid_plural(msg);
		
		if (context)
			quote(out, "msgctxt ", context);
		quote(out, "msgid  ", msgid);
		if (plural)
		{
			char buf[40];
			int i;
			
			quote(out, "msgid_plural ", plural);
			i = 0;
			while ((trans = po_message_msgstr_plural(msg, i)) != NULL)
			{
				++i;
				sprintf(buf, "msgstr[%d] ", i);
				quote(out, buf, trans);
			}
		} else
		{
			quote(out, "msgstr ", trans);
		}
		fputc('\n', out);
	}
	po_message_iterator_free(iter);
	po_file_free(file);
	
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void addfile(const char *path)
{
	char *filename = NULL;
	struct stat s;
	
	if (stat(path, &s) == 0)
		filename = g_strdup(path);
	if (filename == NULL)
		filename = g_build_filename(podir, path, NULL);
	filelist = g_slist_append(filelist, filename);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean read_linguas(const char *filename)
{
	FILE *fp;
	char buf[1024];
	
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: %s: %s:\n", gl_program_name, filename, strerror(errno));
		return FALSE;
	}
	while (fgets(buf, (int)sizeof(buf), fp) != NULL)
	{
		g_strchomp(buf);
		g_strchug(buf);
		if (*buf == '\0' || *buf == '#')
			continue;
		strncat(buf, ".po", sizeof(buf));
		addfile(buf);
	}
	
	fclose(fp);
	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

enum {
	OPT_HELP = 'h',
	OPT_VERSION = 'V',
	OPT_LINGUAS = 256,
	OPT_PODIR,
};

static struct option const long_options[] = {
	{ "linguas", required_argument, NULL, OPT_LINGUAS },
	{ "podir", required_argument, NULL, OPT_PODIR },
	{ "help", no_argument, NULL, OPT_HELP },
	{ "version", no_argument, NULL, OPT_VERSION },
	
	{ NULL, no_argument, NULL, 0 }
};

/*** ---------------------------------------------------------------------- ***/

static void show_version(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void show_help(void)
{
}

/*** ---------------------------------------------------------------------- ***/

int main(int argc, const char **argv)
{
	int i;
	int c;
	gboolean retval = TRUE;
	
	while ((c = getopt_long_only(argc, argv, "hV?", long_options, NULL)) != EOF)
	{
		switch (c)
		{
		case OPT_LINGUAS:
			retval &= read_linguas(optarg);
			break;
		
		case OPT_PODIR:
			podir = optarg;
			break;
		
		case OPT_HELP:
			bShowHelp = TRUE;
			break;
		case OPT_VERSION:
			bShowVersion = TRUE;
			break;
		case '?':
			if (optopt == '?')
			{
				bShowHelp = TRUE;
			} else
			{
				retval = FALSE;
			}
			break;
		
		case 0:
			/* option which just sets a var */
			break;
		
		case 1:
			addfile(optarg);
			break;
		
		default:
			/* error message already issued */
			retval = FALSE;
			break;
		}
	}

	if (bShowHelp)
	{
		show_help();
	} else if (bShowVersion)
	{
		show_version();
	} else if (retval)
	{
		GSList *l;
		
		for (i = optind; i < argc; i++)
		{
			addfile(argv[i]);
		}
		if (filelist == NULL)
		{
			fprintf(stderr, _("%s: no files\n"), gl_program_name);
			retval = FALSE;
		}
		for (l = filelist; l; l = l->next)
		{
			retval &= dofile((const char *)l->data);
		}
	}
	
	g_slist_free_full(filelist, g_free);
	
	return retval == FALSE || num_errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
