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


#define OPT_DUMP 1024
#define OPT_PRINT_UNKNOWN 1025
#define OPT_CHARSET 1026
#define OPT_LONG_FILENAMES 1027

static struct option const long_options[] = {
	{ "autoref", no_argument, NULL, 'a' },
	{ "no-autoref", no_argument, NULL, 'a' + 256 },
	{ "blocksize", required_argument, NULL, 'b' },
	{ "compression", no_argument, NULL, 'c' },
	{ "no-compression", no_argument, NULL, 'c' + 256 },
	{ "ref-distance", required_argument, NULL, 'd' },
	{ "errorfile", required_argument, NULL, 'e' },
	{ "alias-in-index", no_argument, NULL, 'f' },
	{ "no->alias-in-index", no_argument, NULL, 'f' + 256 },
	{ "alabel-in-index", no_argument, NULL, 'g' },
	{ "no-alabel-in-index", no_argument, NULL, 'g' + 256 },
	{ "index", no_argument, NULL, 'i' },
	{ "no-index", no_argument, NULL, 'i' + 256 },
	{ "index-width", required_argument, NULL, 'j' },
	{ "compat-flags", required_argument, NULL, 'k' },
	{ "list", optional_argument, NULL, 'l' },
	{ "images", no_argument, NULL, 'm' },
	{ "no-images", no_argument, NULL, 'm' + 256 },
	{ "no-nodes-in-index", no_argument, NULL, 'n' },
	{ "output", required_argument, NULL, 'o' },
	{ "pic-format", required_argument, NULL, 'p' },
	{ "quiet", no_argument, NULL, 'q' },
	{ "recompile", no_argument, NULL, 'r' },
	{ "split", no_argument, NULL, 's' },
	{ "no-split", no_argument, NULL, 's' + 256 },
	{ "tabwidth", required_argument, NULL, 't' },
	{ "uses", required_argument, NULL, 'u' },
	{ "view", no_argument, NULL, 'v' },
	{ "wait", optional_argument, NULL, 'w' },
	{ "title-in-index", no_argument, NULL, 'x' },
	{ "no-title-in-index", no_argument, NULL, 'x' + 256 },
	{ "caseinsensitive-first", no_argument, NULL, 'y' },
	{ "no-caseinsensitive-first", no_argument, NULL, 'y' + 256 },
	{ "references", no_argument, NULL, 'z' },
	{ "no-references", no_argument, NULL, 'z' + 256 },
	{ "charset", required_argument, NULL, OPT_CHARSET },
	{ "long-filenames", no_argument, NULL, OPT_LONG_FILENAMES },
	{ "no-long-filenames", no_argument, NULL, OPT_LONG_FILENAMES + 256 },
	
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	
	/* undocumented options, for debugging purposes only */
	{ "dump", no_argument, NULL, OPT_DUMP },
	{ "unknown", no_argument, NULL, OPT_PRINT_UNKNOWN },
	
	{ NULL, no_argument, NULL, 0 }
};



void hcp_opts_init(hcp_opts *opts)
{
	memset(opts, 0, sizeof(*opts));
	
	opts->do_ascii_recomp = FALSE;
	opts->do_list = FALSE;
	opts->do_recompile = FALSE;
	opts->do_compile = FALSE;
	opts->do_dump = FALSE;
	opts->list_flags = LIST_ALL;
	opts->do_help = FALSE;
	opts->do_version = FALSE;
	opts->verbose = DEFAULT_VERBOSITY;
#ifdef __TOS__
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
	opts->uses = NULL;
	hcp_copy_uses(&opts->uses, src->uses);
}


HCP_USES *hcp_add_uses(HCP_USES **uses, const char *filename)
{
	HCP_USES *u;
	HCP_USES **last;
	
	u = g_new(HCP_USES, 1);
	if (G_UNLIKELY(u == NULL))
		return NULL;
	u->filename = g_strdup(filename);
	if (G_UNLIKELY(u->filename == NULL))
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
		switch (c)
		{
		case 'a':
			opts->autoreferences = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'a' + 256:
			opts->autoreferences = FALSE;
			break;
		case 'b':
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
		case 'c':
			opts->compression = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'c' + 256:
			opts->compression = FALSE;
			break;
		case 'd':
			opts->min_ref_distance = (int)strtol(getopt_arg_r(d), NULL, 0);
			if (opts->min_ref_distance <= 0 || !g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--ref-distance");
			break;
		case 'e':
			if (origin != OPTS_FROM_COMMANDLINE && origin != OPTS_FROM_ENV)
			{
				retval = not_here(origin, "--errorfile");
			} else
			{
				g_free(opts->error_filename);
				opts->error_filename = g_strdup(getopt_arg_r(d));
			}
			break;
		case 'f':
			opts->alias_to_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'f' + 256:
			opts->alias_to_index = FALSE;
			break;
		case 'g':
			opts->alabel_to_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'g' + 256:
			opts->alabel_to_index = FALSE;
			break;
		case 'i':
			opts->gen_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'i' + 256:
			opts->gen_index = FALSE;
			break;
		case 'j':
			opts->index_width = (int)strtol(getopt_arg_r(d), NULL, 0);
			if (opts->index_width < 0 || !g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--index-width");
			break;
		case 'k':
			opts->compat_flags = (int)strtoul(getopt_arg_r(d), NULL, 0);
			if (!g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--compat-flags");
			break;
		case 'l':
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
		case 'm':
			opts->read_images = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'm' + 256:
			opts->read_images = FALSE;
			break;
		case 'n':
			opts->nodes_to_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'n' + 256:
			opts->nodes_to_index = FALSE;
			break;
		case 'o':
			if (origin != OPTS_FROM_COMMANDLINE && origin != OPTS_FROM_ENV)
			{
				retval = not_here(origin, "--output");
			} else
			{
				g_free(opts->output_filename);
				opts->output_filename = g_strdup(getopt_arg_r(d));
			}
			break;
		case 'p':
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
		case 'q':
			if (getopt_on_r(d))
				opts->verbose++;
			else
				opts->verbose--;
			break;
		case 'r':
			if (origin == OPTS_FROM_SOURCE || origin == OPTS_FROM_CONFIG)
				retval = not_here(origin, "--recompile");
			else
				opts->do_recompile = TRUE;
			break;
		case 's':
			opts->split_lines = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 's' + 256:
			opts->split_lines = FALSE;
			break;
		case 't':
			opts->tabwidth = (int)strtol(getopt_arg_r(d), NULL, 0);
			if (opts->tabwidth < 1 || opts->tabwidth > 9 || !g_is_number(getopt_arg_r(d), TRUE))
				retval = bad_value("--tabwidth");
			break;
		case 'u':
			if (hcp_add_uses(&opts->uses, getopt_arg_r(d)) == NULL)
				retval = FALSE;
			break;
		case 'v':
			if (origin == OPTS_FROM_SOURCE || origin == OPTS_FROM_CONFIG)
				retval = not_here(origin, "--view");
			else
				opts->do_ascii_recomp = TRUE;
			break;
		case 'w':
			if (origin == OPTS_FROM_SOURCE)
				retval = not_here(origin, "--wait");
			else if (getopt_arg_r(d) != NULL)
				opts->wait_key = (int)strtol(getopt_arg_r(d), NULL, 0);
			else
				opts->wait_key = 2;
			break;
		case 'x':
			opts->title_for_index = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'x' + 256:
			opts->title_for_index = FALSE;
			break;
		case 'y':
			opts->caseinsensitive_first = getopt_on_r(d) ? TRUE : FALSE;
			break;
		case 'y' + 256:
			opts->caseinsensitive_first = TRUE;
			break;
		case 'z':
			if (getopt_on_r(d))
				opts->write_references++;
			else
				opts->write_references--;
			break;
		case 'z' + 256:
			opts->write_references = DEFAULT_WRITE_REFERENCES;
			break;
		
		case OPT_CHARSET:
			if (origin == OPTS_FROM_SOURCE)
			{
				retval = not_here(origin, "--charset");
			} else
			{
				opts->output_charset = hyp_charset_from_name(getopt_arg_r(d));
				if (opts->output_charset == HYP_CHARSET_NONE)
				{
					hcp_usage_error(_("unrecognized character set %s"), getopt_arg_r(d));
					retval = FALSE;
				}
			}
			break;

		case 'h':
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
		case 'V':
			if (origin == OPTS_FROM_SOURCE)
				retval = not_here(origin, "--version");
			else
				opts->do_version = TRUE;
			break;
		
		case OPT_DUMP:
			opts->do_dump = TRUE;
			break;
			
		case OPT_PRINT_UNKNOWN:
			opts->print_unknown = TRUE;
			break;
		
		case OPT_LONG_FILENAMES:
			opts->long_filenames = TRUE;
			break;
		
		case OPT_LONG_FILENAMES + 256:
			opts->long_filenames = FALSE;
			break;
		
		case '?':
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
		
		case 0:
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
