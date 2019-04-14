#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "hcp.h"



/*
 * default values for command line options.
 * These are compatible with ST-Guide, and
 * should probably not be changed,
 * If you do, make sure that hcp_get_option_string()
 * constructs the correct string,
 * and update documentation, too.
 */

#define DEFAULT_BLOCKSIZE (8 * 1024)
#define DEFAULT_COMPRESSION TRUE
#define DEFAULT_AUTOREFERENCES TRUE
#define DEFAULT_MIN_REF_DISTANCE 1
#define DEFAULT_ALIAS_TO_INDEX FALSE
#define DEFAULT_ALABEL_TO_INDEX FALSE
#define DEFAULT_NODES_TO_INDEX TRUE
#define DEFAULT_GEN_INDEX TRUE
#define DEFAULT_INDEX_WIDTH 0
#define DEFAULT_COMPAT_FLAGS 0
#define DEFAULT_READ_IMAGES TRUE
#define DEFAULT_SPLIT_LINES TRUE
#define DEFAULT_TABWIDTH 8
#define DEFAULT_TITLE_FOR_INDEX FALSE
#define DEFAULT_WRITE_REFERENCES 0
#define DEFAULT_PIC_FORMAT HYP_PIC_ORIG
#define DEFAULT_CASEINSENSITIVE_FIRST FALSE
#define DEFAULT_VERBOSITY 2


enum hcp_option {
	OPT_HELP = 'h',
	OPT_VERSION = 'V',
	OPT_AUTOREF = 'a',
	OPT_NO_AUTOREF = OPT_AUTOREF + 256,
	OPT_BLOCKSIZE = 'b',
	OPT_COMPRESSION = 'c',
	OPT_NO_COMPRESSION = OPT_COMPRESSION + 256,
	OPT_REF_DISTANCE = 'd',
	OPT_ERRORFILE = 'e',
	OPT_ALIAS_IN_INDEX = 'f',
	OPT_NO_ALIAS_IN_INDEX = OPT_ALIAS_IN_INDEX + 256,
	OPT_ALABEL_IN_INDEX = 'g',
	OPT_NO_ALABEL_IN_INDEX = OPT_ALABEL_IN_INDEX + 256,
	OPT_GEN_INDEX = 'i',
	OPT_NO_GEN_INDEX = OPT_GEN_INDEX + 256,
	OPT_INDEX_WIDTH = 'j',
	OPT_COMPAT_FLAGS = 'k',
	OPT_LIST = 'l',
	OPT_IMAGES = 'm',
	OPT_NO_IMAGES = OPT_IMAGES + 256,
	OPT_NODES_IN_INDEX = 'n',
	OPT_NO_NODES_IN_INDEX = OPT_NODES_IN_INDEX + 256,
	OPT_OUTPUTFILE = 'o',
	OPT_PICFORMAT = 'p',
	OPT_QUIET = 'q',
	OPT_RECOMPILE = 'r',
	OPT_SPLIT = 's',
	OPT_NO_SPLIT = OPT_SPLIT + 256,
	OPT_TABWIDTH = 't',
	OPT_USES = 'u',
	OPT_VIEW = 'v',
	OPT_WAIT = 'w',
	OPT_TITLE_IN_INDEX = 'x',
	OPT_NO_TITLE_IN_INDEX = OPT_TITLE_IN_INDEX + 256,
	OPT_CASEINSENSITIVE_FIRST = 'y',
	OPT_NO_CASEINSENSITIVE_FIRST = OPT_CASEINSENSITIVE_FIRST + 256,
	OPT_REFERENCES = 'z',
	OPT_NO_REFERENCES = OPT_REFERENCES + 256,
	
	OPT_SETVAR = 0,
	OPT_OPTERROR = '?',
	OPT_DUMP = 1024,
	OPT_RECOMPILE_HTML,
	OPT_RECOMPILE_XML,
	OPT_PRINT_UNKNOWN,
	OPT_CHARSET,
	OPT_LONG_FILENAMES,
	OPT_NO_LONG_FILENAMES,
	OPT_HIDEMENU,
	OPT_NO_HIDEMENU
};

static struct option const long_options[] = {
	{ "autoref", no_argument, NULL, OPT_AUTOREF },
	{ "no-autoref", no_argument, NULL, OPT_NO_AUTOREF },
	{ "blocksize", required_argument, NULL, OPT_BLOCKSIZE },
	{ "compression", no_argument, NULL, OPT_COMPRESSION },
	{ "no-compression", no_argument, NULL, OPT_NO_COMPRESSION },
	{ "ref-distance", required_argument, NULL, OPT_REF_DISTANCE },
	{ "errorfile", required_argument, NULL, OPT_ERRORFILE },
	{ "alias-in-index", no_argument, NULL, OPT_ALIAS_IN_INDEX },
	{ "no->alias-in-index", no_argument, NULL, OPT_NO_ALIAS_IN_INDEX },
	{ "alabel-in-index", no_argument, NULL, OPT_ALABEL_IN_INDEX },
	{ "no-alabel-in-index", no_argument, NULL, OPT_NO_ALABEL_IN_INDEX },
	{ "index", no_argument, NULL, OPT_GEN_INDEX },
	{ "no-index", no_argument, NULL, OPT_NO_GEN_INDEX },
	{ "index-width", required_argument, NULL, OPT_INDEX_WIDTH },
	{ "compat-flags", required_argument, NULL, OPT_COMPAT_FLAGS },
	{ "list", optional_argument, NULL, OPT_LIST },
	{ "images", no_argument, NULL, OPT_IMAGES },
	{ "no-images", no_argument, NULL, OPT_NO_IMAGES },
	{ "no-nodes-in-index", no_argument, NULL, OPT_NO_NODES_IN_INDEX },
	{ "output", required_argument, NULL, OPT_OUTPUTFILE },
	{ "pic-format", required_argument, NULL, OPT_PICFORMAT },
	{ "quiet", no_argument, NULL, OPT_QUIET },
	{ "recompile", no_argument, NULL, OPT_RECOMPILE },
	{ "stg", no_argument, NULL, OPT_RECOMPILE },
	{ "split", no_argument, NULL, OPT_SPLIT },
	{ "no-split", no_argument, NULL, OPT_NO_SPLIT },
	{ "tabwidth", required_argument, NULL, OPT_TABWIDTH },
	{ "uses", required_argument, NULL, OPT_USES },
	{ "view", no_argument, NULL, OPT_VIEW },
	{ "html", no_argument, NULL, OPT_RECOMPILE_HTML },
	{ "xml", no_argument, NULL, OPT_RECOMPILE_XML },
	{ "wait", optional_argument, NULL, OPT_WAIT },
	{ "title-in-index", no_argument, NULL, OPT_TITLE_IN_INDEX },
	{ "no-title-in-index", no_argument, NULL, OPT_NO_TITLE_IN_INDEX },
	{ "caseinsensitive-first", no_argument, NULL, OPT_CASEINSENSITIVE_FIRST },
	{ "no-caseinsensitive-first", no_argument, NULL, OPT_NO_CASEINSENSITIVE_FIRST },
	{ "references", no_argument, NULL, OPT_REFERENCES },
	{ "no-references", no_argument, NULL, OPT_NO_REFERENCES },
	{ "charset", required_argument, NULL, OPT_CHARSET },
	{ "long-filenames", no_argument, NULL, OPT_LONG_FILENAMES },
	{ "no-long-filenames", no_argument, NULL, OPT_NO_LONG_FILENAMES },
	{ "hidemenu", no_argument, NULL, OPT_HIDEMENU },
	{ "no-hidemenu", no_argument, NULL, OPT_NO_HIDEMENU },
	
	{ "help", no_argument, NULL, OPT_HELP },
	{ "version", no_argument, NULL, OPT_VERSION },
	
	/* undocumented options, for debugging purposes only */
	{ "dump", no_argument, NULL, OPT_DUMP },
	{ "unknown", no_argument, NULL, OPT_PRINT_UNKNOWN },
	
	{ NULL, no_argument, NULL, 0 }
};



void hcp_opts_init(hcp_opts *opts)
{
	memset(opts, 0, sizeof(*opts));
	
	opts->recompile_format = HYP_FT_NONE;
	opts->do_list = FALSE;
	opts->do_compile = FALSE;
	opts->list_flags = LIST_ALL;
	opts->do_help = FALSE;
	opts->do_version = FALSE;
	opts->verbose = DEFAULT_VERBOSITY;
#if defined(__TOS__) || defined(__atarist__)
	opts->wait_key = 1;
#else
	opts->wait_key = 0;
#endif
	opts->block_size = DEFAULT_BLOCKSIZE;
	opts->compression = DEFAULT_COMPRESSION;
	opts->autoreferences = DEFAULT_AUTOREFERENCES;
	opts->min_ref_distance = DEFAULT_MIN_REF_DISTANCE;
	opts->alias_to_index = DEFAULT_ALIAS_TO_INDEX;
	opts->alabel_to_index = DEFAULT_ALABEL_TO_INDEX;
	opts->nodes_to_index = DEFAULT_NODES_TO_INDEX;
	opts->gen_index = DEFAULT_GEN_INDEX;
	opts->index_width = DEFAULT_INDEX_WIDTH;
	opts->compat_flags = DEFAULT_COMPAT_FLAGS;
	opts->read_images = DEFAULT_READ_IMAGES;
	opts->split_lines = DEFAULT_SPLIT_LINES;
	opts->tabwidth = DEFAULT_TABWIDTH;
	opts->title_for_index = DEFAULT_TITLE_FOR_INDEX;
	opts->caseinsensitive_first = DEFAULT_CASEINSENSITIVE_FIRST;
	opts->write_references = DEFAULT_WRITE_REFERENCES;
	opts->error_filename = NULL;
	opts->output_filename = NULL;
	opts->pic_format = DEFAULT_PIC_FORMAT;
	opts->outfile = stdout;
	opts->errorfile = stderr;
	opts->optind = 0;
	opts->uses = NULL;
	opts->long_filenames = -1;
	opts->warn_compat = TRUE;
	opts->image_name_prefix = g_strdup("img");
	opts->print_unknown = FALSE;
	opts->output_charset = HYP_CHARSET_NONE;
}


static void hcp_copy_uses(HCP_USES **uses, HCP_USES *u)
{
	if (u)
	{
		hcp_add_uses(uses, u->filename);
		hcp_copy_uses(uses, u->next);
	}
}


void hcp_opts_copy(hcp_opts *opts, const hcp_opts *src)
{
	*opts = *src;
	opts->output_filename = g_strdup(src->output_filename);
	opts->error_filename = g_strdup(src->error_filename);
	opts->image_name_prefix = g_strdup(src->image_name_prefix);
	opts->output_dir = g_strdup(src->output_dir);
	opts->uses = NULL;
	hcp_copy_uses(&opts->uses, src->uses);
}


HCP_USES *hcp_add_uses(HCP_USES **uses, const char *filename)
{
	HCP_USES *u;
	HCP_USES **last;
	
	u = g_new(HCP_USES, 1);
	if (u == NULL)
		return NULL;
	u->filename = g_strdup(filename);
	if (u->filename == NULL)
	{
		g_free(u);
		return NULL;
	}
	u->source_location.id = 0;
	u->source_location.lineno = 0;
	u->next = NULL;
	last = uses;
	while (*last != NULL)
		last = &(*last)->next;
	*last = u;
	return u;
}


void hcp_free_uses(HCP_USES **uses)
{
	HCP_USES *p, *next;
	
	p = *uses;
	*uses = NULL;
	while (p != NULL)
	{
		next = p->next;
		g_free(p->filename);
		g_free(p);
		p = next;
	}
}


void hcp_usage_error(const char *msg, ...)
{
	va_list args;
	
	hyp_utf8_fprintf(stderr, "%s: ", gl_program_name);
	va_start(args, msg);
	hyp_utf8_vfprintf(stderr, msg, args);
	va_end(args);
	hyp_utf8_fprintf(stderr, "\n");
}
#define hcp_usage_warning hcp_usage_error


static int getopt_on_r(struct _getopt_data *d)
{
	return getopt_switch_r(d) == '+';
}


static gboolean not_here(opts_origin origin, const char *what)
{
	switch (origin)
	{
	case OPTS_FROM_COMMANDLINE:
		hcp_usage_error(_("%s can not be specified %s"), what, _("on the command line"));
		break;
	case OPTS_FROM_ENV:
		hcp_usage_error(_("%s can not be specified %s"), what, _("in the environment"));
		break;
	case OPTS_FROM_CONFIG:
		hcp_usage_error(_("%s can not be specified %s"), what, _("in the configuration file"));
		break;
	case OPTS_FROM_SOURCE:
		hcp_usage_error(_("%s can not be specified %s"), what, _("in the source file"));
		break;
	}
	return FALSE;
}


static gboolean bad_value(const char *what)
{
	hcp_usage_error(_("bad value for %s"), what);
	return FALSE;
}


hyp_pic_format hcp_pic_format_from_name(const char *name)
{
	if (empty(name))
		return HYP_PIC_UNKNOWN;
	if (g_ascii_strcasecmp(name, "iff") == 0)
		return HYP_PIC_IFF;
	if (g_ascii_strcasecmp(name, "ilbm") == 0)
		return HYP_PIC_IFF;
	if (g_ascii_strcasecmp(name, "icn") == 0)
		return HYP_PIC_ICN;
	if (g_ascii_strcasecmp(name, "img") == 0)
		return HYP_PIC_IMG;
	if (g_ascii_strcasecmp(name, "bmp") == 0)
		return HYP_PIC_BMP;
	if (g_ascii_strcasecmp(name, "gif") == 0)
		return HYP_PIC_GIF;
	if (g_ascii_strcasecmp(name, "png") == 0)
		return HYP_PIC_PNG;
	return HYP_PIC_UNKNOWN;
}


const char *hcp_pic_format_to_name(hyp_pic_format format)
{
	switch (format)
	{
		case HYP_PIC_ORIG: break;
		case HYP_PIC_IFF: return "iff";
		case HYP_PIC_ICN: return "icn";
		case HYP_PIC_IMG: return "img";
		case HYP_PIC_BMP: return "bmp";
		case HYP_PIC_GIF: return "gif";
		case HYP_PIC_PNG: return "png";
	}
	return NULL;
}


/* ------------------------------------------------------------------------- */

const char *hcp_pic_format_to_mimetype(hyp_pic_format format)
{
	switch (format)
	{
		case HYP_PIC_ORIG: break;
		case HYP_PIC_IFF: return "image/x-iff";
		case HYP_PIC_ICN: return "image/x-icn";
		case HYP_PIC_IMG: return "image/x-gem";
		case HYP_PIC_BMP: return "image/bmp";
		case HYP_PIC_GIF: return "image/gif";
		case HYP_PIC_PNG: return "image/png";
	}
	return NULL;
}

gboolean hcp_opts_parse(hcp_opts *opts, int argc, const char **argv, opts_origin origin)
{
	struct _getopt_data *d;
	gboolean retval = TRUE;
	long val;
	int c;
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(argc, argv, "ab:cd:e:fgij:k:l::mno:p:qrst:u:vw::xyzhV?", long_options, NULL, d)) != EOF)
	{
		switch ((enum hcp_option)c)
		{
		case OPT_AUTOREF:
			opts->autoreferences = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_AUTOREF:
			opts->autoreferences = FALSE;
			break;
		case OPT_BLOCKSIZE:
			if (origin == OPTS_FROM_SOURCE)
			{
				retval = not_here(origin, "--blocksize");
			} else
			{
				opts->block_size = strtol(getopt_arg_r(d), NULL, 0) * 1024;
				if (opts->block_size < 0)
					opts->block_size = DEFAULT_BLOCKSIZE;
				hcp_usage_warning(_("--blocksize is no longer supported"));
			}
			break;
		case OPT_COMPRESSION:
			opts->compression = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_COMPRESSION:
			opts->compression = FALSE;
			break;
		case OPT_REF_DISTANCE:
			opts->min_ref_distance = (int)strtol(getopt_arg_r(d), NULL, 0);
			if (opts->min_ref_distance < 0 || !g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--ref-distance");
			break;
		case OPT_ERRORFILE:
			if (origin != OPTS_FROM_COMMANDLINE && origin != OPTS_FROM_ENV)
			{
				retval = not_here(origin, "--errorfile");
			} else
			{
				g_free(opts->error_filename);
				opts->error_filename = g_strdup(getopt_arg_r(d));
			}
			break;
		case OPT_ALIAS_IN_INDEX:
			opts->alias_to_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_ALIAS_IN_INDEX:
			opts->alias_to_index = FALSE;
			break;
		case OPT_ALABEL_IN_INDEX:
			opts->alabel_to_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_ALABEL_IN_INDEX:
			opts->alabel_to_index = FALSE;
			break;
		case OPT_GEN_INDEX:
			/*
			 * FIXME: ST-Guide seems to always write -i to the @options string;
			 * we should ignore it completely in this case
			 */
			/*
			 * when recompiling, do not overwrite the option specified on the commandline
			 */
			if (opts->recompile_format == HYP_FT_NONE || origin != OPTS_FROM_SOURCE)
				opts->gen_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_GEN_INDEX:
			/*
			 * FIXME: ST-Guide seems to always write -i to the @options string;
			 * we should ignore it completely in this case
			 */
			/*
			 * when recompiling, do not overwrite the option specified on the commandline
			 */
			if (opts->recompile_format == HYP_FT_NONE || origin != OPTS_FROM_SOURCE)
				opts->gen_index = FALSE;
			break;
		case OPT_INDEX_WIDTH:
			opts->index_width = (int)strtol(getopt_arg_r(d), NULL, 0);
			if (opts->index_width < 0 || !g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--index-width");
			break;
		case OPT_COMPAT_FLAGS:
			opts->compat_flags = (int)strtoul(getopt_arg_r(d), NULL, 0);
			if (!g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--compat-flags");
			break;
		case OPT_LIST:
			if (origin == OPTS_FROM_SOURCE || origin == OPTS_FROM_CONFIG)
			{
				retval = not_here(origin, "--list");
			} else
			{
				opts->do_list++;
				if (getopt_arg_r(d) != NULL)
				{
					opts->list_flags = (unsigned int)strtoul(getopt_arg_r(d), NULL, 0);
					/* -ll will be interpreted as -l=l */
					if (opts->list_flags == 0)
					{
						if (strcmp(getopt_arg_r(d), "l") == 0)
						{
							opts->list_flags = LIST_ALL;
							opts->do_list++;
						} else if (!g_is_number(getopt_arg_r(d), TRUE))
						{
							retval = bad_value(_("listing flags"));
						}
					}
				}
			}
			break;
		case OPT_IMAGES:
			opts->read_images = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_IMAGES:
			opts->read_images = FALSE;
			break;
		case OPT_NODES_IN_INDEX:
			opts->nodes_to_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_NODES_IN_INDEX:
			opts->nodes_to_index = FALSE;
			break;
		case OPT_OUTPUTFILE:
			if (origin != OPTS_FROM_COMMANDLINE && origin != OPTS_FROM_ENV)
			{
				retval = not_here(origin, "--output");
			} else
			{
				g_free(opts->output_filename);
				opts->output_filename = g_strdup(getopt_arg_r(d));
			}
			break;
		case OPT_PICFORMAT:
			if (g_ascii_strncasecmp(getopt_arg_r(d), "orig", 4) == 0)
			{
				val = HYP_PIC_ORIG;
			} else
			{
				val = hcp_pic_format_from_name(getopt_arg_r(d));
				if (val == HYP_PIC_UNKNOWN)
				{
					if (g_is_number(getopt_arg_r(d), TRUE))
						val = strtoul(getopt_arg_r(d), NULL, 0);
					else
						val = -1;
				}
			}
			if (val < 0 || val > HYP_PIC_LAST)
			{
				hcp_usage_error(_("unknown picture format"));
				retval = FALSE;
			} else if (origin == OPTS_FROM_SOURCE)
			{
				retval = not_here(origin, "--pic-format");
			} else
			{
				opts->pic_format = (hyp_pic_format) val;
			}
			break;
		case OPT_QUIET:
			if (getopt_on_r(d))
				opts->verbose++;
			else
				opts->verbose--;
			break;
		case OPT_RECOMPILE:
			if (origin == OPTS_FROM_SOURCE || origin == OPTS_FROM_CONFIG)
			{
				retval = not_here(origin, "--recompile");
			} else if (opts->recompile_format == HYP_FT_XML || opts->recompile_format == HYP_FT_HTML || opts->recompile_format == HYP_FT_HTML_XML)
			{
				opts->showstg = TRUE;
			} else
			{
				opts->recompile_format = HYP_FT_STG;
			}
			break;
		case OPT_SPLIT:
			opts->split_lines = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_SPLIT:
			opts->split_lines = FALSE;
			break;
		case OPT_TABWIDTH:
			opts->tabwidth = (int)strtol(getopt_arg_r(d), NULL, 0);
			if (opts->tabwidth < 1 || opts->tabwidth > 9 || !g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--tabwidth");
			break;
		case OPT_USES:
			if (hcp_add_uses(&opts->uses, getopt_arg_r(d)) == NULL)
				retval = FALSE;
			break;
		case OPT_VIEW:
			if (origin == OPTS_FROM_SOURCE || origin == OPTS_FROM_CONFIG)
				retval = not_here(origin, "--view");
			else
				opts->recompile_format = HYP_FT_ASCII;
			break;
		case OPT_RECOMPILE_HTML:
			if (origin == OPTS_FROM_SOURCE || origin == OPTS_FROM_CONFIG)
			{
				retval = not_here(origin, "--html");
			} else if (opts->recompile_format == HYP_FT_XML)
			{
				opts->recompile_format = HYP_FT_HTML_XML;
			} else
			{
				if (opts->recompile_format == HYP_FT_STG)
					opts->showstg = TRUE;
				opts->recompile_format = HYP_FT_HTML;
			}
			break;
		case OPT_RECOMPILE_XML:
			if (origin == OPTS_FROM_SOURCE || origin == OPTS_FROM_CONFIG)
			{
				retval = not_here(origin, "--xml");
			} else if (opts->recompile_format == HYP_FT_HTML)
			{
				opts->recompile_format = HYP_FT_HTML_XML;
			} else
			{
				if (opts->recompile_format == HYP_FT_STG)
					opts->showstg = TRUE;
				opts->recompile_format = HYP_FT_XML;
			}
			break;
		case OPT_WAIT:
			if (origin == OPTS_FROM_SOURCE)
				retval = not_here(origin, "--wait");
			else if (getopt_arg_r(d) != NULL)
				opts->wait_key = (int)strtol(getopt_arg_r(d), NULL, 0);
			else
				opts->wait_key = 2;
			break;
		case OPT_TITLE_IN_INDEX:
			opts->title_for_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_TITLE_IN_INDEX:
			opts->title_for_index = FALSE;
			break;
		case OPT_CASEINSENSITIVE_FIRST:
			opts->caseinsensitive_first = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case OPT_NO_CASEINSENSITIVE_FIRST:
			opts->caseinsensitive_first = TRUE;
			break;
		case OPT_REFERENCES:
			if (getopt_on_r(d))
				opts->write_references++;
			else
				opts->write_references--;
			break;
		case OPT_NO_REFERENCES:
			opts->write_references = DEFAULT_WRITE_REFERENCES;
			break;
		
		case OPT_CHARSET:
			if (origin == OPTS_FROM_SOURCE)
			{
				retval = not_here(origin, "--charset");
			} else
			{
				HYP_CHARSET output_charset = hyp_charset_from_name(getopt_arg_r(d));
				if (output_charset == HYP_CHARSET_NONE)
				{
					hcp_usage_error(_("unrecognized character set %s"), getopt_arg_r(d));
					retval = FALSE;
				} else
				{
					opts->output_charset = output_charset;
				}
			}
			break;

		case OPT_HELP:
			if (origin == OPTS_FROM_SOURCE)
			{
				/*
				 * some hypertexts have that option set,
				 * (has it been an option in older versions of hcp?)
				 * simply ignore it.
				 */
#if 0
				retval = not_here(origin, "--help");
#endif
			} else
			{
				opts->do_help = TRUE;
			}
			break;
		case OPT_VERSION:
			if (origin == OPTS_FROM_SOURCE)
				retval = not_here(origin, "--version");
			else
				opts->do_version = TRUE;
			break;
		
		case OPT_DUMP:
			opts->recompile_format = HYP_FT_BINARY;
			break;
			
		case OPT_PRINT_UNKNOWN:
			opts->print_unknown = TRUE;
			break;
		
		case OPT_LONG_FILENAMES:
			opts->long_filenames = TRUE;
			break;
		
		case OPT_NO_LONG_FILENAMES:
			opts->long_filenames = FALSE;
			break;
		
		case OPT_HIDEMENU:
			opts->hidemenu = TRUE;
			break;
		
		case OPT_NO_HIDEMENU:
			opts->hidemenu = FALSE;
			break;
		
		case OPT_OPTERROR:
			if (getopt_opt_r(d) == '?')
			{
				if (origin == OPTS_FROM_SOURCE)
					retval = not_here(origin, "--help");
				else
					opts->do_help = TRUE;
			} else
			{
				retval = FALSE;
			}
			break;
		
		case OPT_SETVAR:
			/* option which just sets a var */
			break;
		
		default:
			/* error message already issued */
			retval = FALSE;
			break;
		}
	}
	
	opts->optind = getopt_ind_r(d);
	getopt_finish_r(&d);
	
	if (opts->optind != argc && origin != OPTS_FROM_COMMANDLINE)
	{
		retval = not_here(origin, _("input files"));
	}
	
	return retval;
}


gboolean hcp_opts_parse_string(hcp_opts *opts, const char *argstring, opts_origin origin)
{
	char **argv;
	int argc;
	const char *s;
	const char *current;
	gboolean retval;
	
#define isdelim(c) ((c) == ' ' || (c) == '\t')

	argc = 1;
	while (isdelim(*argstring))
		argstring++;
	s = current = argstring;
	while (*s != '\0')
	{
		if (isdelim(*s) || ((*s == '+' || *s == '-') && s != current))
		{
			++argc;
			while (isdelim(*s))
				s++;
			current = s;
		} else
		{
			++s;
		}
	}
	if (s != current)
		argc++;
	argv = g_new(char *, argc + 1);

	argc = 0;
	argv[argc++] = g_strdup(gl_program_name);
	s = current = argstring;
	while (*s != '\0')
	{
		if (isdelim(*s) || ((*s == '+' || *s == '-') && s != current))
		{
			argv[argc++] = g_strndup(current, s - current);
			while (isdelim(*s))
				s++;
			current = s;
		} else
		{
			++s;
		}
	}
	if (s != current)
		argv[argc++] = g_strndup(current, s - current);
	argv[argc] = NULL;
#undef isdelim

	retval = hcp_opts_parse(opts, argc, (const char **)(const void **)argv, origin);
	g_strfreev(argv);
	return retval;
}



void hcp_opts_free(hcp_opts *opts)
{
	g_freep(&opts->output_filename);
	g_freep(&opts->error_filename);
	g_freep(&opts->image_name_prefix);
	g_freep(&opts->output_dir);
	hcp_free_uses(&opts->uses);
	if (opts->errorfile != NULL)
	{
		fflush(opts->errorfile);
		if (opts->errorfile != stderr && opts->errorfile != stdout)
			fclose(opts->errorfile);
		opts->errorfile = NULL;
	}
	if (opts->outfile != NULL)
	{
		fflush(opts->outfile);
		if (opts->outfile != stdout)
			fclose(opts->outfile);
		opts->outfile = NULL;
	}
}	


char *hcp_get_option_string(hcp_opts *opts)
{
	char *str = NULL;
	char *tmp;
	char *newstr;
	
#define addstr(s) \
	newstr = g_strconcat(str ? str : "", str ? " " : "", s, NULL), \
	g_free(str), \
	str = newstr
#define addtmp() \
	addstr(tmp), \
	g_free(tmp)

	if (opts->autoreferences != DEFAULT_AUTOREFERENCES)
		addstr(DEFAULT_AUTOREFERENCES ? "-a" : "+a");
	if (opts->block_size != DEFAULT_BLOCKSIZE)
	{
		tmp = g_strdup_printf("-b%ld", (long)opts->block_size);
		addtmp();
	}
	if (opts->compression != DEFAULT_COMPRESSION)
		addstr(DEFAULT_COMPRESSION ? "-c" : "+c");
	if (opts->min_ref_distance != DEFAULT_MIN_REF_DISTANCE)
	{
		tmp = g_strdup_printf("-d%d", opts->min_ref_distance);
		addtmp();
	}
	if (opts->alias_to_index != DEFAULT_ALIAS_TO_INDEX)
		addstr(DEFAULT_ALIAS_TO_INDEX ? "-f" : "+f");
	if (opts->alabel_to_index != DEFAULT_ALABEL_TO_INDEX)
		addstr(DEFAULT_ALABEL_TO_INDEX ? "-g" : "+g");
	if (opts->nodes_to_index != DEFAULT_NODES_TO_INDEX)
		addstr(DEFAULT_NODES_TO_INDEX ? "-n" : "+n");
	if (opts->gen_index != DEFAULT_GEN_INDEX)
		addstr(DEFAULT_GEN_INDEX ? "-i" : "+i");
	if (opts->index_width != DEFAULT_INDEX_WIDTH)
	{
		tmp = g_strdup_printf("-j%d", opts->index_width);
		addtmp();
	}
	if (opts->compat_flags != DEFAULT_COMPAT_FLAGS)
	{
		tmp = g_strdup_printf("-k%u", opts->compat_flags);
		addtmp();
	}
	if (opts->read_images != DEFAULT_READ_IMAGES)
		addstr(DEFAULT_READ_IMAGES ? "-m" : "+m");
	if (opts->split_lines != DEFAULT_SPLIT_LINES)
		addstr(DEFAULT_SPLIT_LINES ? "-s" : "+s");
	if (opts->tabwidth != DEFAULT_TABWIDTH)
	{
		tmp = g_strdup_printf("-t%d", opts->tabwidth);
		addtmp();
	}
	if (opts->title_for_index != DEFAULT_TITLE_FOR_INDEX)
		addstr(DEFAULT_TITLE_FOR_INDEX ? "-x" : "+x");
	if (opts->caseinsensitive_first != DEFAULT_CASEINSENSITIVE_FIRST)
		addstr(DEFAULT_CASEINSENSITIVE_FIRST ? "-y" : "+y");
	if (opts->write_references != DEFAULT_WRITE_REFERENCES)
		addstr(opts->write_references > 1 ? "+zz" : "+z");
	if (opts->verbose != DEFAULT_VERBOSITY)
		addstr(DEFAULT_VERBOSITY - opts->verbose >= 3 ? "-qqq" :
			   DEFAULT_VERBOSITY - opts->verbose >= 2 ? "-qq" :
			   DEFAULT_VERBOSITY - opts->verbose >= 1 ? "-q" :
			   DEFAULT_VERBOSITY - opts->verbose >= -1 ? "+q" :
			   DEFAULT_VERBOSITY - opts->verbose >= -2 ? "+qq" :
			                                             "+qqq");
	
#undef addstr
#undef addtmp

	return str;
}
