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
#include "rcintl.h"

/*
 * if this file is used in a cross-compilation environment,
 * it is compiled for the build system.
 * We cannot use the libraries that were compiled for the target,
 * so just include the needed sources here for simplicity
 */
#include "xgetopt.h"
#include "../hyp/xgetopt.c"
#include "../hyp/hyp_glib.c"
#define LANG_STRINGS 1
#include "../rcintl/localenm.c"

#ifndef CATOBJEXT
#define CATOBJEXT ".gmo"
#endif


char const gl_program_name[] = "polinguas";

static int num_errors;
static int num_warnings;
static gboolean bShowVersion;
static gboolean bShowHelp;
static GSList *filelist;
static const char *podir = "../po";

struct fileentry {
	char *filename;
	char *lang;
};

#define SUBLANG_SHIFT 10
#undef MAKELANGID
#define MAKELANGID(primary, sub) (((sub) << SUBLANG_SHIFT) | (primary))
#undef PRIMARYLANGID
#define PRIMARYLANGID(id) ((id) & ~(1 << SUBLANG_SHIFT))
#undef SUBLANGID
#define SUBLANGID(id) ((id) >> SUBLANG_SHIFT)
#undef MAKELCID
#define MAKELCID(l,s) ((LCID)((((LCID)((uint16_t)(s)))<<16)|((LCID)((uint16_)(l)))))
#undef LANGIDFROMLCID
#define LANGIDFROMLCID(l) ((uint16_t)((l) & 0xffff))

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

static char *translations;

static gboolean dofile(FILE *out, struct fileentry *entry)
{
	po_file_t file = po_file_read(entry->filename, &my_xerror_handler);
	const char *const *domains;
	const char *header;
	char *lang;
	int i, n;
	char *moname;
	char *p;
	char *tmp, *tmp2;
	
	if (file == NULL)
	{
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, entry->filename, strerror(errno));
		return FALSE;
	}
	domains = po_file_domains(file);
	if (domains == NULL || domains[0] == NULL || strcmp(domains[0], "messages") != 0)
	{
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, entry->filename, _("not a messages file"));
		po_file_free(file);
		return FALSE;
	}
	header = po_file_domain_header(file, domains[0]);
	lang = po_header_field(header, "Language");
	if (lang)
	{
		g_free(entry->lang);
		entry->lang = g_strdup(lang);
		free(lang);
	}
	po_file_free(file);
	
	n = (int)(sizeof(sublang_table) / sizeof(sublang_table[0])) - 1;
	for (i = 0; i < n; i++)
	{
		if (strncmp(entry->lang, sublang_table[i].po_name, sublang_table[i].namelen) == 0)
			break;
	}
	if (i >= n)
	{
		for (i = 0; i < n; i++)
		{
			if (strncmp(entry->lang, sublang_table[i].po_name, 2) == 0 &&
				(SUBLANGID(sublang_table[i].id) == SUBLANG_DEFAULT ||
				 SUBLANGID(sublang_table[i].id) == SUBLANG_NEUTRAL))
				break;
		}
	}
	if (i >= n)
	{
		fprintf(stderr, _("%s: %s: unsupported language %s\n"), gl_program_name, entry->filename, entry->lang);
	}
	fprintf(out, "LANGUAGE %s, %s\n", sublang_table[i].langname, sublang_table[i].subname);
	moname = g_strdup(hyp_basename(entry->filename));
	p = strrchr(moname, '.');
	if (p)
		*p = '\0';
	fprintf(out, "1 RT_MOFILE LOADONCALL MOVEABLE DISCARDABLE IMPURE \"%s/%s" CATOBJEXT "\"\n", podir, moname);
	g_free(moname);
	if (translations == NULL)
	{
		translations = g_strdup("0x0409, 1200"); /* LANG_ENGLISH, SUBLANG_ENGLISH_US */
	}
	tmp = g_strdup_printf(", 0x%04x, 1200", sublang_table[i].id);
	tmp2 = g_strconcat(translations, tmp, NULL);
	g_free(tmp);
	g_free(translations);
	translations = tmp2;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void addfile(const char *path)
{
	char *filename = NULL;
	struct stat s;
	struct fileentry *entry;
	char *lang;
	char *p;
	
	if (stat(path, &s) == 0)
		filename = g_strdup(path);
	if (filename == NULL)
		filename = g_build_filename(podir, path, NULL);
	entry = g_new(struct fileentry, 1);
	entry->filename = filename;
	lang = g_strdup(hyp_basename(path));
	p = strchr(lang, '.');
	if (p)
		*p = '\0';
	entry->lang = lang;
	filelist = g_slist_append(filelist, entry);
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
		FILE *out = stdout;

		for (i = optind; i < argc; i++)
		{
			addfile(argv[i]);
		}
		if (filelist == NULL)
		{
			fprintf(stderr, _("%s: no files\n"), gl_program_name);
			retval = FALSE;
		}
		fprintf(out, "#define RT_MOFILE %d\n", RT_MOFILE);
		fprintf(out, "\n");
		for (l = filelist; l; l = l->next)
		{
			retval &= dofile(out, (struct fileentry *)l->data);
		}
		if (translations)
		{
			fprintf(out, "\n");
			fprintf(out, "#define PO_TRANSLATIONS %s\n", translations);
			g_free(translations);
			translations = NULL;
		}
		fprintf(out, "\n");
		fprintf(out, "LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US\n");
		fprintf(out, "#pragma code_page(1252)\n");
		fprintf(out, "\n");
	}
	
	g_slist_free_full(filelist, g_free);
	
	return retval == FALSE || num_errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
