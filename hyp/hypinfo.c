#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#include "llangid.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "hv_vers.h"

char const gl_program_name[] = "hypinfo";
char const gl_program_version[] = HYP_VERSION;

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#define CMDLINE_VERSION 1
#define OUT_ASCII_ONLY 1

#include "outcomm.h"
#include "outasc.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

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

#if 0
static void print_short_version(FILE *out)
{
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n\n",
		gl_program_name, gl_program_version,
		HYP_COPYRIGHT);
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", printnull(msg));
	g_free(msg);
}
#endif

/* ------------------------------------------------------------------------- */

static void print_usage(FILE *out)
{
	print_version(out);
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("usage: %s [-options] file1 [file2 ...]\n"), gl_program_name);
}


/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *buf;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	buf = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
	return buf;
}

/* ------------------------------------------------------------------------- */

static gboolean hypinfo(const char *filename, hcp_opts *opts, gboolean print_filename, LanguageIdentifier *lid)
{
	HYP_DOCUMENT *hyp;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	FILE *outfile = opts->outfile;
	char *prefix;
	char *str;
	char *newer;
	hyp_nodenr num_nodes;
	hyp_nodenr num_pnodes;
	hyp_nodenr num_images;
	hyp_nodenr num_external;
	hyp_nodenr num_other;
	hyp_nodenr num_eof;
	hyp_nodenr num_unknown;
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	
	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}

	hyp = hyp_load(filename, handle, &type);
	if (hyp == NULL)
	{
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, filename);
		return FALSE;
	}
	
	prefix = print_filename ? g_strdup_printf("%s: ", hyp->file) : g_strdup("");
	
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@os: %s\n", prefix, hyp_osname(hyp->comp_os));
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@charset: %s\n", prefix, hyp_charset_name(hyp->comp_charset));

	if (hyp->language == NULL && lid != NULL)
	{
		double prob;
		const char *lang;
		GString *out;
		hyp_nodenr count;
		
		if (hyp->first_text_page == HYP_NOINDEX)
		{
			lang = "empty";
		} else
		{
			out = g_string_new(NULL);
			count = 0;
			for (node = hyp->first_text_page; node < hyp->num_index && count < 5; node++)
			{
				entry = hyp->indextable[node];
				if (node == hyp->index_page)
					continue;
				switch ((hyp_indextype) entry->type)
				{
				case HYP_NODE_INTERNAL:
				case HYP_NODE_POPUP:
					ascii_out_node(hyp, opts, out, node);
					count++;
					break;
				case HYP_NODE_IMAGE:
				case HYP_NODE_EXTERNAL_REF:
				case HYP_NODE_SYSTEM_ARGUMENT:
				case HYP_NODE_REXX_SCRIPT:
				case HYP_NODE_REXX_COMMAND:
				case HYP_NODE_QUIT:
				case HYP_NODE_CLOSE:
				case HYP_NODE_EOF:
				default:
					break;
				}
			}
			
			lang = langid_identify(lid, out->str, out->len, &prob);
			g_string_free(out, TRUE);
			hyp->language_guessed = TRUE;
		}
		hyp->language = g_strdup(lang);
	}
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@lang: %s\n", prefix, hyp->language ? hyp->language : "unknown");
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:lang_guessed: %d\n", prefix, hyp->language_guessed);
	
	if (hyp->database != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@database: %s\n", prefix, hyp->database);
	}

	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = quote_nodename(hyp, hyp->default_page);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@default: %s\n", prefix, str);
		g_free(str);
	}

	if (hyp->hcp_options != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@options: %s\n", prefix, hyp->hcp_options);
	}
	if (hyp->author != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@author: %s\n", prefix, hyp->author);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = quote_nodename(hyp, hyp->help_page);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@help: %s\n", prefix, str);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@version: %s\n", prefix, hyp->version);
	}
	if (hyp->subject != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@subject: %s\n", prefix, hyp->subject);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@width: %d\n", prefix, hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@flags: 0x%04x\n", prefix, hyp->st_guide_flags);
	}
	
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s@hostname: %s\n", prefix, h->name);
		}
	}

	newer = hyp->comp_vers > HCP_COMPILER_VERSION ? g_strdup_printf(" (> %u)", HCP_COMPILER_VERSION) : g_strdup("");
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:hcpversion: %u%s\n", prefix, hyp->comp_vers, newer);
	g_free(newer);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:hyptree: %u\n", prefix, hyp->hyptree_len);
	
	num_nodes = num_pnodes = num_images = num_external = num_other = num_eof = num_unknown = 0;
	for (node = hyp->first_text_page; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
			num_nodes++;
			break;
		case HYP_NODE_POPUP:
			num_pnodes++;
			break;
		case HYP_NODE_IMAGE:
			num_images++;
			break;
		case HYP_NODE_EXTERNAL_REF:
			num_external++;
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
			num_other++;
			break;
		case HYP_NODE_EOF:
			if ((node + 1u) != hyp->num_index)
				hyp_utf8_fprintf(opts->errorfile, _("%s: EOF entry at %u\n"), hyp->file, node);
			else
				hyp_utf8_fprintf(opts->outfile, "%s:eof: %u\n", prefix, node);
			num_eof++;
			break;
		default:
			hyp_utf8_fprintf(opts->errorfile, _("%s: unknown index entry type %u\n"), hyp->file, entry->type);
			num_unknown++;
			break;
		}
	}

	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:indexsize: %lu\n", prefix, hyp->itable_size);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:totalnodes: %u\n", prefix, hyp->num_index);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:nodes: %u\n", prefix, num_nodes);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:pnodes: %u\n", prefix, num_pnodes);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:images: %u\n", prefix, num_images);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:external: %u\n", prefix, num_external);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:other: %u\n", prefix, num_other);
	if (num_unknown != 0)
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s:unknown: %u\n", prefix, num_unknown);
		
	g_free(prefix);
	
	hyp_unref(hyp);
	hyp_utf8_close(handle);
	
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

enum hypinfo_option {
	OPT_HELP = 'h',
	OPT_VERSION = 'V',
	OPT_QUIET = 'q',
	OPT_WAIT = 'w',

	OPT_SETVAR = 0,
	OPT_OPTERROR = '?',

	OPT_CHARSET = 1024,
};

static struct option const long_options[] = {
	{ "wait", optional_argument, NULL, OPT_WAIT },
	{ "quiet", no_argument, NULL, OPT_QUIET },
	{ "charset", required_argument, NULL, OPT_CHARSET },

	{ "help", no_argument, NULL, OPT_HELP },
	{ "version", no_argument, NULL, OPT_VERSION },
	
	{ NULL, no_argument, NULL, 0 }
};
	
static int getopt_on_r(struct _getopt_data *d)
{
	return getopt_switch_r(d) == '+';
}


static gboolean opts_parse(hcp_opts *opts, int argc, const char **argv)
{
	struct _getopt_data *d;
	gboolean retval = TRUE;
	int c;
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(argc, argv, "qw::hV?", long_options, NULL, d)) != EOF)
	{
		switch ((enum hypinfo_option)c)
		{
		case OPT_QUIET:
			if (getopt_on_r(d))
				opts->verbose++;
			else
				opts->verbose--;
			break;

		case OPT_WAIT:
			if (getopt_arg_r(d) != NULL)
				opts->wait_key = (int)strtol(getopt_arg_r(d), NULL, 0);
			else
				opts->wait_key = 2;
			break;

		case OPT_CHARSET:
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
			opts->do_help = TRUE;
			break;
		case OPT_VERSION:
			opts->do_version = TRUE;
			break;

		case OPT_OPTERROR:
			if (getopt_opt_r(d) == '?')
			{
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
	
	
	return retval;
}


#include "hypmain.h"

int main(int argc, const char **argv)
{
	int c;
	int retval = EXIT_SUCCESS;
	hcp_opts _opts;
	hcp_opts *opts = &_opts;
	int wait_key;
	
	HypProfile_Load(TRUE);
	
	hcp_opts_init(opts);
	if (!hcp_opts_parse_string(opts, gl_profile.hcp.options, OPTS_FROM_CONFIG))
		retval = EXIT_FAILURE;
	if (opts_parse(opts, argc, argv) == FALSE)
		retval = EXIT_FAILURE;
	
	if (retval != EXIT_SUCCESS)
	{
	} else if (opts->do_version)
	{
		print_version(stdout);
	} else if (opts->do_help)
	{
		print_usage(stdout);
	} else
	{
		int num_args;
		
		c = opts->optind;
		num_args = argc - c;

		if (retval == EXIT_SUCCESS && num_args <= 0)
		{
			hcp_usage_error(_("no files specified"));
			retval = EXIT_FAILURE;
		}
		
		if (retval == EXIT_SUCCESS)
		{
			LanguageIdentifier *lid;
			
			if (opts->output_charset == HYP_CHARSET_NONE)
				opts->output_charset = hyp_get_current_charset();
			lid = langid_get_default_identifier();
			while (c < argc)
			{
				const char *filename = argv[c++];
				if (!hypinfo(filename, opts, num_args > 1, lid))
				{
					retval = EXIT_FAILURE;
				}
			}
			langid_destroy_identifier(lid);
		}
	}
	
	wait_key = opts->wait_key;
	hcp_opts_free(opts);
	
	if (wait_key == 2 ||
		(wait_key == 1 && retval != EXIT_SUCCESS))
	{
		fflush(stderr);
#ifdef __TOS__
		hyp_utf8_printf(_("<press any key>"));
		fflush(stdout);
		Cnecin();
#else
		hyp_utf8_printf(_("<press RETURN>"));
		fflush(stdout);
		getchar();
#endif
	}
	
	HypProfile_Delete();
	x_free_resources();

	(void) vdi_maptab16;
	return retval;
}
