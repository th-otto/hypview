#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif



char const gl_program_name[] = "RefLink";


static struct option const long_options[] = {
	{ "add", no_argument, NULL, 'a' },
	{ "move", no_argument, NULL, 'm' },
	{ "delete", no_argument, NULL, 'd' },
	{ "extract", no_argument, NULL, 'e' },
	{ "files", no_argument, NULL, 'f' },
	{ "list", no_argument, NULL, 'l' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, no_argument, NULL, 0 }
};


/* ------------------------------------------------------------------------- */

char *gl_program_version(void)
{
	return hyp_lib_version();
}

/* ------------------------------------------------------------------------- */

static void print_version(FILE *out)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information)."), gl_program_name, HYP_URL);
	char *version = gl_program_version();
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n"
		"%s\n",
		 gl_program_name, version,
		 HYP_COPYRIGHT,
		 url);
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", msg);
	g_free(msg);
	g_free(version);
	g_free(url);
}


static void print_usage(FILE *out)
{
	hyp_utf8_fprintf(out, _("usage: %s -amdeflhV file...\n"), gl_program_name);
	hyp_utf8_fprintf(out, _("options:\n"));
	hyp_utf8_fprintf(out, _("  -a, --add [file] <module>          add module to file\n"));
	hyp_utf8_fprintf(out, _("  -m, --move [file] <module>         move module to file\n"));
	hyp_utf8_fprintf(out, _("  -d, --delete [file] <module> ...   delete modules from file\n"));
	hyp_utf8_fprintf(out, _("  -e, --extract [file] <module> ...  extract modules\n"));
	hyp_utf8_fprintf(out, _("  -f, --files [file] [file...]       list modules in file\n"));
	hyp_utf8_fprintf(out, _("  -l, --list [file] [file...]        list contents\n"));
	hyp_utf8_fprintf(out, _("  -h, --help                         print help and exit\n"));
	hyp_utf8_fprintf(out, _("  -V, --version                      print version and exit\n"));
}


static void usage_error(const char *msg)
{
	hyp_utf8_fprintf(stderr, "%s: %s\n", gl_program_name, msg);
}


#include "hypmain.h"

int main(int argc, const char **argv)
{
	int c;
	struct _getopt_data *d;
	int retval = 0;
	int num_args;
	int num_opts;
	
	FILE *outfile = stdout;
	gboolean do_add = FALSE;
	gboolean do_move = FALSE;
	gboolean do_delete = FALSE;
	gboolean do_extract = FALSE;
	gboolean do_modules = FALSE;
	gboolean do_list = FALSE;
	gboolean do_help = FALSE;
	gboolean do_version = FALSE;
	gboolean verbose = TRUE;

	HypProfile_Load();
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(argc, argv, "aAmMdDeEfFlLhV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case 'a':
		case 'A':
			do_add = TRUE;
			break;
		case 'm':
		case 'M':
			do_move = TRUE;
			break;
		case 'd':
			do_delete = TRUE;
			break;
		case 'e':
		case 'E':
			do_extract = TRUE;
			break;
		case 'f':
		case 'F':
			do_modules = TRUE;
			break;
		case 'l':
		case 'L':
			do_list = TRUE;
			break;

		case 'h':
			do_help = TRUE;
			break;
		case 'V':
			do_version = TRUE;
			break;
		case '?':
			if (getopt_opt_r(d) == '?')
				do_help = TRUE;
			else
				retval = 1;
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
	
	num_opts = do_add + do_move + do_delete + do_extract + do_modules + do_list;
	c = getopt_ind_r(d);
	num_args = argc - c;
	
	if (num_opts == 0 && num_args == 0 && !do_version)
	{
		do_help = TRUE;
	}
		
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
		if (retval == 0)
		{
			if (num_opts == 0)
			{
				do_add = TRUE;
			} else if (num_opts > 1)
			{
				usage_error(_("only one of -amdefl may be specified"));
				retval = 1;
			}
		}
		
		if (retval == 0)
		{
			if (do_modules)
			{
				/* all arguments are files to be listed */
				if (num_args == 0)
				{
					char *filename = path_subst(gl_profile.general.all_ref);
					if (!ref_list_entries(filename, outfile, FALSE, verbose))
						retval = 1;
					g_free(filename);
				} else
				{
					while (c < argc)
					{
						const char *filename = argv[c++];
						if (!ref_list_entries(filename, outfile, FALSE, verbose))
							retval = 1;
					}
				}
			} else if (do_list)
			{
				/* all arguments are files to be listed */
				if (num_args == 0)
				{
					char *filename = path_subst(gl_profile.general.all_ref);
					if (!ref_list_entries(filename, outfile, TRUE, verbose))
						retval = 1;
					g_free(filename);
				} else
				{
					while (c < argc)
					{
						const char *filename = argv[c++];
						if (!ref_list_entries(filename, outfile, TRUE, verbose))
							retval = 1;
					}
				}
			} else if (do_add || do_move)
			{
				/*
				 * only 1 or 2 arguments
				 * if first argument is reffile, module is added to this;
				 * last arg is module or reffile to add
				 */
				if (verbose)
					hyp_utf8_fprintf(outfile, _("Adding/replacing files\n"));
				if (num_args == 1)
				{
					char *refname = path_subst(gl_profile.general.all_ref);
					const char *modname = argv[c++];
					if (!ref_add_entries(refname, modname, do_move, outfile, verbose))
						retval = 1;
					g_free(refname);
				} else if (num_args == 2)
				{
					const char *refname = argv[c++];
					const char *modname = argv[c++];
					if (!ref_add_entries(refname, modname, do_move, outfile, verbose))
						retval = 1;
				} else if (num_args == 0)
				{
					usage_error(_("no files to add"));
					retval = 1;
				} else
				{
					usage_error(_("too many arguments"));
					retval = 1;
				}
			} else if (do_delete)
			{
				char *refname;
				
				/*
				 * first arg is maybe ref file name.
				 * other args are modules to be deleted
				 */
				if (num_args > 0 && hyp_guess_filetype(argv[c]) == HYP_FT_REF)
				{
					refname = g_strdup(argv[c++]);
					num_args--;
				} else
				{
					refname = path_subst(gl_profile.general.all_ref);
				}
				if (num_args == 0)
				{
					usage_error(_("no files to delete"));
					retval = 1;
				} else
				{
					if (!ref_del_entries(refname, num_args, &argv[c], outfile, verbose))
						retval = 1;
				}
				g_free(refname);
			} else if (do_extract)
			{
				char *refname;
				
				/*
				 * first arg is maybe ref file name.
				 * other args are modules to be deleted
				 */
				if (num_args > 0 && hyp_guess_filetype(argv[c]) == HYP_FT_REF)
				{
					refname = g_strdup(argv[c++]);
					num_args--;
				} else
				{
					refname = path_subst(gl_profile.general.all_ref);
				}
				if (!ref_extract_entries(refname, num_args, &argv[c], outfile, verbose))
					retval = 1;
				g_free(refname);
			} else
			{
				retval = 1;
				unreachable();
			}
		}
	}
	
	getopt_finish_r(&d);
	HypProfile_Delete();
	x_free_resources();
	
	return retval;
}
