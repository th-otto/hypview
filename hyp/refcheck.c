#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "hv_vers.h"


/*
 * values where options originated from
 */
typedef enum {
	/* option was specified on command line */
	OPTS_FROM_COMMANDLINE,
	/* option was specified in configuration file */
	OPTS_FROM_CONFIG,
	/* option was specified in environment variable, i.e. $HCP_OPT */
	OPTS_FROM_ENV
} opts_origin;


char const gl_program_name[] = "RefCheck";
char const gl_program_version[] = HYP_VERSION;


struct refcheck_opts {
	FILE *outfile;
	gboolean do_add;
	gboolean do_move;
	gboolean do_delete;
	gboolean do_extract;
	gboolean do_modules;
	gboolean do_list;
	gboolean do_help;
	gboolean do_version;
	gboolean global_flag;
	gboolean interactive;
	gboolean silent;
	gboolean verbose;
	gboolean check_mode;
	int catalog;
	gboolean changed;
	char *refname;
	int optind;
};

#include "../pic/piccolor.c"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static struct option const long_options[] = {
	{ "add", no_argument, NULL, 'a' },
	{ "catalog", no_argument, NULL, 'c' },
	{ "move", no_argument, NULL, 'm' },
	{ "delete", no_argument, NULL, 'd' },
	{ "extract", no_argument, NULL, 'e' },
	{ "force", no_argument, NULL, 'f' },
	{ "interactive", no_argument, NULL, 'i' },
	{ "list", no_argument, NULL, 'l' },
	{ "silent", no_argument, NULL, 's' },
	{ "add-single", no_argument, NULL, 'A' },
	{ "delete-single", no_argument, NULL, 'D' },
	{ "move-single", no_argument, NULL, 'M' },
	{ "files", no_argument, NULL, 'F' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, no_argument, NULL, 0 }
};


/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void print_version(FILE *out)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information)."), gl_program_name, HYP_URL);
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n"
		"%s\n",
		 gl_program_name, gl_program_version,
		 HYP_COPYRIGHT,
		 url);
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", printnull(msg));
	g_free(msg);
	g_free(url);
}

/* ------------------------------------------------------------------------- */

static void print_usage(FILE *out)
{
	hyp_utf8_fprintf(out, _("usage: %s -amdeflhV file...\n"), gl_program_name);
	hyp_utf8_fprintf(out, _("options:\n"));
	hyp_utf8_fprintf(out, _("  -a, --add [file]                    add newer modules to file\n"));
	hyp_utf8_fprintf(out, _("  -m, --move [file]                   move newer modules to file\n"));
	hyp_utf8_fprintf(out, _("  -d, --delete [file]                 delete non-existing modules from file\n"));
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("  -A, --add-single [file] <module>    add module to file\n"));
	hyp_utf8_fprintf(out, _("  -M, --move-single [file] <module>   move module to file\n"));
	hyp_utf8_fprintf(out, _("  -D, --delete-single [file] <module> delete modules from file\n"));
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("  -e, --extract [file] <module> ...   extract modules\n"));
	hyp_utf8_fprintf(out, _("  -F, --files [file] [file...]        list modules in file\n"));
	hyp_utf8_fprintf(out, _("  -l, --list [file] [file...]         list contents\n"));
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("  -r FILE, --reflib=FILE              specify library file\n"));
	hyp_utf8_fprintf(out, _("  -c, --catalog                       recreate catalog if [file] changed\n"));
	hyp_utf8_fprintf(out, _("  -f, --force                         recreate catalog always\n"));
	hyp_utf8_fprintf(out, _("  -s, --silent                        suppress most messages\n"));
	hyp_utf8_fprintf(out, _("  -i, --interactive                   query for add/delete/move\n"));
	hyp_utf8_fprintf(out, _("  -g, --global                        process all found files\n"));
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("  -h, --help                          print help and exit\n"));
	hyp_utf8_fprintf(out, _("  -V, --version                       print version and exit\n"));
}

/* ------------------------------------------------------------------------- */

static void __attribute__((format(printf, 1, 2))) usage_error(const char *msg, ...)
{
	va_list args;
	
	hyp_utf8_fprintf(stderr, "%s: ", gl_program_name);
	va_start(args, msg);
	hyp_utf8_vfprintf(stderr, msg, args);
	va_end(args);
	hyp_utf8_fprintf(stderr, "\n");
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

typedef struct _hyplist HYPLIST;
struct _hyplist {
	HYPLIST *next;
	REF_FILE *ref;
	char filename[1];
};

#define HYP_FILE_ONLY ((REF_MODULE *)1)

struct find_args {
	int hyp_count;
	int ref_count;
	int mod_count;
	const struct refcheck_opts *opts;
	HYPLIST *hypfiles;
};

static gboolean maybe_load_hyp_or_ref(const char *filename, void *data)
{
	struct find_args *args = (struct find_args *)data;
	const char *base = hyp_basename(filename);
	char *modname2;
	int mod_handle;
	HYPLIST *l;
	REF_FILE *mod;
	REF_MODULE *mod2;
	
	if (hyp_guess_filetype(base) == HYP_FT_HYP)
	{
		/* do not try to load "all.ref" here if there happens to be a "all.hyp" */
		if (g_ascii_strcasecmp(base, "all.hyp") == 0)
			return TRUE;
		
		++args->hyp_count;
		if (!args->opts->silent)
			hyp_utf8_fprintf(args->opts->outfile, "%-12s", base);
		modname2 = g_strdup(filename);
		l = (HYPLIST *) g_malloc0(sizeof(HYPLIST) + strlen(filename));
		l->next = args->hypfiles;
		args->hypfiles = l;
		strcpy(l->filename, filename);
		
		modname2 = replace_ext(l->filename, HYP_EXT_HYP, HYP_EXT_REF);
		mod_handle = hyp_utf8_open(modname2, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (mod_handle >= 0)
		{
			++args->ref_count;
			if (!args->opts->silent)
				hyp_utf8_fprintf(args->opts->outfile, "   +REF   ");
			mod = ref_load(modname2, mod_handle, args->opts->verbose);
			if (mod == NULL)
			{
				hyp_utf8_close(mod_handle);
				g_free(modname2);
				return FALSE;
			}
			hyp_utf8_close(mod_handle);
			mod_handle = -1;
			l->ref = mod;
			
			{
				gboolean first = TRUE;
				
				for (mod2 = mod->modules; mod2 != NULL; mod2 = mod2->next)
				{
					char *name;
					
					++args->mod_count;
					if (!first && !args->opts->silent)
						hyp_utf8_fprintf(args->opts->outfile, "\n                      ");
					name = ref_hyp_basename(mod2->filename);
					if (!args->opts->silent)
						hyp_utf8_fprintf(args->opts->outfile, "%-12s %c %7ld   %7ld", name, mod2->mod_name_matches ? ' ' : '!', mod2->module_offset, mod2->module_len);
					g_free(name);
					first = FALSE;
				}
			}
		}
		if (!args->opts->silent)
			hyp_utf8_fprintf(args->opts->outfile, "\n");
		g_freep(&modname2);
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean check_entries(const char *refname, const struct refcheck_opts *opts, gboolean *p_changed)
{
	int ref_handle = -1, mod_handle = -1;
	char *refname2 = NULL;
	REF_FILE *ref = NULL, *mod = NULL;
	char *tmpname = NULL;
	char *tmp;
	REF_MODULE *mod1, *mod2;
	gboolean found;
	int num_all_ref;
	REF_MODULE **all_ref = NULL;
	REF_MODULE **all_mods = NULL;
	HYPLIST *l;
	int i, j;
	int new_num_modules;
	struct find_args args;
	
	args.hyp_count = 0;
	args.ref_count = 0;
	args.mod_count = 0;
	args.opts = opts;
	args.hypfiles = NULL;
	ref_handle = hyp_utf8_open(refname, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (ref_handle < 0 && hyp_guess_filetype(refname) != HYP_FT_REF)
	{
		refname2 = g_strconcat(refname, HYP_EXT_REF, NULL);
		refname = refname2;
		ref_handle = hyp_utf8_open(refname, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	}
	
	if (ref_handle < 0)
	{
		ref = ref_new(refname, 0);
	} else
	{
		if (!opts->silent)
			hyp_utf8_fprintf(opts->outfile, _("analyzing %s...\n"), refname);
		ref = ref_load(refname, ref_handle, opts->verbose);
		if (ref == NULL)
			goto error;
		hyp_utf8_close(ref_handle);
		ref_handle = -1;
		if (!opts->silent)
		{
			char *filename;
			
			hyp_utf8_fprintf(opts->outfile, "\n");
			hyp_utf8_fprintf(opts->outfile, _("Modulename      Offset    Length\n"));
			hyp_utf8_fprintf(opts->outfile,   "------------   -------   -------\n");
			for (mod1 = ref->modules; mod1 != NULL; mod1 = mod1->next)
			{
				filename = ref_hyp_basename(mod1->filename);
				hyp_utf8_fprintf(opts->outfile, "%-12s   %7ld   %7ld\n", filename, mod1->module_offset, mod1->module_len);
				g_free(filename);
			}
		}
		if (!opts->silent)
			hyp_utf8_fprintf(opts->outfile, _("found %d modules in %s\n"), ref_num_modules(ref), refname);
	}
	
	if (!empty(gl_profile.refcheck.path_list))
	{
		if (!opts->silent)
			hyp_utf8_fprintf(opts->outfile, _("searching HYP files in PATHS...\n"));
		
		if (!opts->silent)
		{
			hyp_utf8_fprintf(opts->outfile, "\n");
			hyp_utf8_fprintf(opts->outfile, _("HYP-Name       REF?   Modulename   !  Offset    Length\n"));
			hyp_utf8_fprintf(opts->outfile,   "------------   ----   ------------ - -------   -------\n");
		}
		if (!walk_pathlist(gl_profile.refcheck.path_list, maybe_load_hyp_or_ref, &args))
			goto error;
		if (!opts->silent)
		{
			hyp_utf8_fprintf(opts->outfile, "\n");
			hyp_utf8_fprintf(opts->outfile, P_("%d HYP-File", "%d HYP-Files", args.hyp_count), args.hyp_count);
			hyp_utf8_fprintf(opts->outfile, _(" and "));
			hyp_utf8_fprintf(opts->outfile, P_("%d REF-File", "%d REF-Files", args.ref_count), args.ref_count);
			hyp_utf8_fprintf(opts->outfile, P_(" (%d modul)", " (%d modules)", args.mod_count), args.mod_count);
			hyp_utf8_fprintf(opts->outfile, _(" found in PATHS\n"));
			hyp_utf8_fprintf(opts->outfile, "\n");
		}
	}
	
	if (!opts->silent)
	{
		hyp_utf8_fprintf(opts->outfile, _("Comparing...\n"));
	}
	num_all_ref = ref_num_modules(ref);
	all_ref = g_new0(REF_MODULE *, num_all_ref + 1);
	all_mods = g_new0(REF_MODULE *, args.mod_count + 1);
	if (all_ref == NULL || all_mods == NULL)
		goto error;
	
	/*
	 * check wether the modules in all.ref exist in one of the ref files
	 */
	for (mod1 = ref->modules, i = 0; mod1 != NULL; mod1 = mod1->next, i++)
	{
		found = FALSE;
		mod2 = NULL;
		for (l = args.hypfiles; l != NULL; l = l->next)
		{
			mod = l->ref;
			if (mod != NULL)
			{
				for (mod2 = mod->modules; mod2 != NULL; mod2 = mod2->next)
				{
					char *name1 = ref_hyp_basename(mod1->filename);
					char *name2 = ref_hyp_basename(mod2->filename);
					found = hyp_utf8_strcasecmp(name1, name2) == 0;
					g_free(name1);
					g_free(name2);
					if (found)
						break;
				}
			}
			if (found)
				break;
			
			{
				char *name1 = ref_hyp_basename(mod1->filename);
				char *name2 = ref_hyp_basename(l->filename);
				found = hyp_utf8_strcasecmp(name1, name2) == 0;
				g_free(name1);
				g_free(name2);
				if (found)
				{
					mod2 = HYP_FILE_ONLY;
					break;
				}
			}
		}
		if (found)
		{
			all_ref[i] = mod2;
		}
	}

	/*
	 * check wether the modules in the ref files exist in all.ref
	 */
 	for (l = args.hypfiles, i = 0; l != NULL; l = l->next)
	{
		mod = l->ref;
		if (mod != NULL)
		{
			for (mod2 = mod->modules; mod2 != NULL; mod2 = mod2->next, i++)
			{
				found = FALSE;
				for (mod1 = ref->modules, j = 0; mod1 != NULL; mod1 = mod1->next, j++)
				{
					char *name1 = ref_hyp_basename(mod1->filename);
					char *name2 = ref_hyp_basename(mod2->filename);
					found = hyp_utf8_strcasecmp(name1, name2) == 0;
					g_free(name1);
					g_free(name2);
					if (found)
					{
						if (all_ref[j] == HYP_FILE_ONLY)
							all_ref[j] = mod2;
						break;
					}
				}
				if (found)
					all_mods[i] = mod1;
			}
		}
	}

	if (!opts->silent)
	{
		for (mod1 = ref->modules, i = 0; mod1 != NULL; mod1 = mod1->next, i++)
			if (all_ref[i] == NULL)
			{
				char *name = ref_hyp_basename(mod1->filename);
				hyp_utf8_fprintf(opts->outfile, _("HYP-File for module \"%s\" from %s not found in PATHS\n"), name, refname);
				g_free(name);
			}
	 	for (l = args.hypfiles, i = 0; l != NULL; l = l->next)
		{
			mod = l->ref;
			if (mod != NULL)
			{
				for (mod2 = mod->modules; mod2 != NULL; mod2 = mod2->next, i++)
				{
					if (all_mods[i] == NULL)
					{
						char *name = ref_hyp_basename(mod2->filename);
						hyp_utf8_fprintf(opts->outfile, _("%s: module \"%s\" not found in %s\n"), hyp_basename(mod->filename), name, refname);
						g_free(name);
					}
				}
			}
		}
	}
	
	if (!(opts->do_add || opts->do_move || opts->do_delete))
		goto done;
	
	{
		char *dirname = hyp_path_get_dirname(refname);
		tmp = g_strdup_printf("hy(%u).ref", (int)getpid());
		tmpname = g_build_filename(dirname, tmp, NULL);
		g_free(tmp);
		g_free(dirname);
	}
	
	ref_handle = hyp_utf8_open(tmpname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, 0644);
	if (ref_handle < 0)
	{
		FileErrorErrno(tmpname);
		goto error;
	}
	
	/*
	 * write the file header
	 */
	if (!ref_write_header(ref_handle))
		goto write_error;

	new_num_modules = 0;
	for (mod1 = ref->modules, i = 0; mod1 != NULL; mod1 = mod1->next, i++)
	{
		if (all_ref[i] == NULL)
		{
			char *name = ref_hyp_basename(mod1->filename);
			if (!opts->do_delete)
			{
				/* keep old module */
				if (!ref_write_module(ref_handle, mod1, opts->verbose))
					goto write_error;
				new_num_modules++;
			} else
			{
				if (!opts->silent)
					hyp_utf8_fprintf(opts->outfile, _("Deleting %s\n"), name);
				*p_changed = TRUE;
			}
			g_free(name);
		} else if (all_ref[i] == HYP_FILE_ONLY || !opts->do_add)
		{
			/* keep old module */
			if (!ref_write_module(ref_handle, mod1, opts->verbose))
				goto write_error;
			new_num_modules++;
		}
	}
 	for (l = args.hypfiles, i = 0; l != NULL; l = l->next)
	{
		mod = l->ref;
		if (mod != NULL)
		{
			gboolean remove_ref = opts->do_move;
			
			for (mod2 = mod->modules; mod2 != NULL; mod2 = mod2->next, i++)
			{
				if (all_mods[i] == NULL)
				{
					char *name = ref_hyp_basename(mod2->filename);
					if (opts->do_add)
					{
						if (!opts->silent)
							hyp_utf8_fprintf(opts->outfile, _("Adding %s\n"), name);
						if (!ref_write_module(ref_handle, mod2, opts->verbose))
							goto write_error;
						*p_changed = TRUE;
						new_num_modules++;
						if (!mod2->mod_name_matches)
							remove_ref = FALSE;
					} else
					{
						remove_ref = FALSE;
					}
					g_free(name);
				} else
				{
					/* replace already existing module */
					if (!ref_write_module(ref_handle, mod2, opts->verbose))
						goto write_error;
					new_num_modules++;
					if (!mod2->mod_name_matches)
						remove_ref = FALSE;
				}
			}
			if (remove_ref)
			{
				if (!opts->silent)
					hyp_utf8_fprintf(opts->outfile, _("Deleting %s\n"), mod->filename);
				if (hyp_utf8_unlink(mod->filename) < 0)
				{
					FileErrorErrno(mod->filename);
					goto error;
				}
			}
		}
	}
	
	if (!opts->silent && !*p_changed)
	{
		hyp_utf8_fprintf(opts->outfile, _("No changes\n"));
		/*
		 * no changes in module names; we still might
		 * have replaced modules inside all.ref
		 */
	}
		
	/*
	 * write an empty module header as terminator
	 */
	if (!ref_write_trailer(ref_handle))
		goto write_error;
	hyp_utf8_close(ref_handle);
	ref_handle = -1;
	
	if (hyp_utf8_unlink(refname) < 0)
		if (ref->modules != NULL)
		{
			FileErrorErrno(refname);
			goto error;
		}
	ref_close(ref);
	ref = NULL;
	if (new_num_modules == 0)
	{
		if (opts->verbose)
			hyp_utf8_fprintf(opts->outfile, _("No files left\n"));
		if (hyp_utf8_unlink(refname) < 0)
		{
			FileErrorErrno(refname);
			goto error;
		}
		if (hyp_utf8_unlink(tmpname) < 0)
		{
			FileErrorErrno(tmpname);
			goto error;
		}
	} else
	{
		if (hyp_utf8_rename(tmpname, refname) < 0)
		{
			FileErrorErrno(refname);
			goto error;
		}
	}
	
done:
	g_free(tmpname);
	g_free(all_mods);
	g_free(all_ref);
	ref_close(ref);
	{
		HYPLIST *next;
		while (args.hypfiles != NULL)
		{
			next = args.hypfiles->next;
			ref_close(args.hypfiles->ref);
			g_free(args.hypfiles);
			args.hypfiles = next;
		}
	}
	g_free(refname2);
	return TRUE;
	
write_error:
	FileErrorErrno(tmpname);
	
error:
	g_free(all_mods);
	g_free(all_ref);
	ref_close(ref);
	if (ref_handle >= 0)
		hyp_utf8_close(ref_handle);
	if (mod_handle >= 0)
		hyp_utf8_close(mod_handle);
	g_free(refname2);
	if (tmpname)
	{
		hyp_utf8_unlink(tmpname);
		g_free(tmpname);
	}
	{
		HYPLIST *next;
		while (args.hypfiles != NULL)
		{
			next = args.hypfiles->next;
			ref_close(args.hypfiles->ref);
			g_free(args.hypfiles);
			args.hypfiles = next;
		}
	}
	return FALSE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean not_here(opts_origin origin, const char *what)
{
	switch (origin)
	{
	case OPTS_FROM_COMMANDLINE:
		usage_error(_("%s can not be specified %s"), what, _("on the command line"));
		break;
	case OPTS_FROM_ENV:
		usage_error(_("%s can not be specified %s"), what, _("in the environment"));
		break;
	case OPTS_FROM_CONFIG:
		usage_error(_("%s can not be specified %s"), what, _("in the configuration file"));
		break;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static gboolean refcheck_opts_parse(struct refcheck_opts *opts, int argc, const char **argv, opts_origin origin)
{
	struct _getopt_data *d;
	gboolean retval = TRUE;
	int c;
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(argc, argv, "acmdeEfglLir:R:ADFMhV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case 'a':
			opts->do_add = TRUE;
			opts->check_mode = TRUE;
			break;
		case 'c':
			opts->catalog = 1;
			break;
		case 'm':
			opts->do_move = TRUE;
			opts->check_mode = TRUE;
			break;
		case 'd':
			opts->do_delete = TRUE;
			opts->check_mode = TRUE;
			break;
		case 'f':
			opts->catalog = 2;
			break;
		case 'g':
			opts->global_flag = TRUE;
			break;
		case 'r':
		case 'R':
			opts->refname = g_strdup(getopt_arg_r(d));
			break;
		
		case 'l':
		case 'L':
			opts->do_list = TRUE;
			break;
		case 'i':
			opts->interactive = TRUE;
			break;
		case 's':
			opts->silent = TRUE;
			break;
		case 'e':
		case 'E':
			opts->do_extract = TRUE;
			break;
		case 'A':
			opts->do_add = TRUE;
			break;
		case 'D':
			opts->do_delete = TRUE;
			break;
		case 'F':
			opts->do_modules = TRUE;
			break;
		case 'M':
			opts->do_move = TRUE;
			break;

		case 'h':
			opts->do_help = TRUE;
			break;
		case 'V':
			opts->do_version = TRUE;
			break;
		case '?':
			if (getopt_opt_r(d) == '?')
			{
				if (origin != OPTS_FROM_COMMANDLINE)
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
	
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean refcheck_opts_parse_string(struct refcheck_opts *opts, const char *argstring, opts_origin origin)
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

	retval = refcheck_opts_parse(opts, argc, (const char **)(const void **)argv, origin);
	g_strfreev(argv);
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#include "hypmain.h"

int main(int argc, const char **argv)
{
	int c;
	int retval = 0;
	int num_args;
	int num_opts;
	struct refcheck_opts opts;
	gboolean changed = FALSE;
	
	memset(&opts, 0, sizeof(opts));
	opts.outfile = stdout;
	opts.do_add = FALSE;
	opts.do_move = FALSE;
	opts.do_delete = FALSE;
	opts.do_extract = FALSE;
	opts.do_modules = FALSE;
	opts.do_list = FALSE;
	opts.do_help = FALSE;
	opts.do_version = FALSE;
	opts.global_flag = FALSE;
	opts.interactive = FALSE;
	opts.silent = FALSE;
	opts.verbose = TRUE;
	opts.check_mode = FALSE;
	opts.catalog = 0;
	opts.refname = NULL;
	
	HypProfile_Load(TRUE);
	
	if (!refcheck_opts_parse(&opts, argc, argv, OPTS_FROM_COMMANDLINE))
		retval = 1;
	num_opts = opts.do_add + opts.do_move + opts.do_delete + opts.do_extract + opts.do_modules + opts.do_list;
	if (num_opts == 0)
	{
		if (!empty(gl_profile.refcheck.options))
			if (!refcheck_opts_parse_string(&opts, gl_profile.refcheck.options, OPTS_FROM_CONFIG))
				retval = 1;
		num_opts = opts.do_add + opts.do_move + opts.do_delete + opts.do_extract + opts.do_modules + opts.do_list;
	}
	c = opts.optind;
	num_args = argc - c;
	
	if (retval != 0)
	{
	} else if (opts.do_version)
	{
		print_version(stdout);
	} else if (opts.do_help)
	{
		print_usage(stdout);
	} else
	{
		if (retval == 0)
		{
			if (num_opts == 0)
			{
				/* default options "-ads" */
				opts.do_add = TRUE;
				opts.do_delete = TRUE;
				opts.check_mode = TRUE;
				opts.silent = TRUE;
			} else if (num_opts > 1 && !opts.check_mode)
			{
				usage_error(_("only one of -amdeFl may be specified"));
				retval = 1;
			} else if ((opts.do_add | opts.do_move | opts.do_delete) + opts.do_extract + opts.do_modules + opts.do_list > 1 && opts.check_mode)
			{
				usage_error(_("only one of -amdeFl may be specified"));
				retval = 1;
			}
		}
		
		if (retval == 0)
		{
			if (opts.do_modules)
			{
				/* all arguments are files to be listed */
				if (num_args == 0)
				{
					char *filename = path_subst(gl_profile.general.all_ref);
					if (!ref_list_entries(filename, opts.outfile, FALSE, opts.verbose))
						retval = 1;
					g_free(filename);
				} else
				{
					while (c < argc)
					{
						const char *filename = argv[c++];
						if (!ref_list_entries(filename, opts.outfile, FALSE, opts.verbose))
							retval = 1;
					}
				}
			} else if (opts.do_list)
			{
				/* all arguments are files to be listed */
				if (num_args == 0)
				{
					char *filename = path_subst(gl_profile.general.all_ref);
					if (!ref_list_entries(filename, opts.outfile, TRUE, opts.verbose))
						retval = 1;
					g_free(filename);
				} else
				{
					while (c < argc)
					{
						const char *filename = argv[c++];
						if (!ref_list_entries(filename, opts.outfile, TRUE, opts.verbose))
							retval = 1;
					}
				}
			} else if (opts.do_extract)
			{
				/*
				 * first arg is maybe ref file name.
				 * other args are modules to be deleted
				 */
				if (num_args > 0 && hyp_guess_filetype(argv[c]) == HYP_FT_REF && opts.refname == NULL)
				{
					opts.refname = g_strdup(argv[c++]);
					num_args--;
				} else if (opts.refname == NULL)
				{
					opts.refname = path_subst(gl_profile.general.all_ref);
				}
				if (!ref_extract_entries(opts.refname, num_args, &argv[c], opts.outfile, opts.verbose))
					retval = 1;
			} else if (opts.check_mode)
			{
				/*
				 * first arg is maybe ref file name.
				 */
				if (num_args > 0 && hyp_guess_filetype(argv[c]) == HYP_FT_REF && opts.refname == NULL)
				{
					opts.refname = g_strdup(argv[c++]);
					num_args--;
				} else if (opts.refname == NULL)
				{
					opts.refname = path_subst(gl_profile.general.all_ref);
				}
				if (num_args != 0)
				{
					usage_error(_("too many arguments"));
					retval = 1;
				} else
				{
					if (!check_entries(opts.refname, &opts, &changed))
						retval = 1;
				}
			} else if (opts.do_add || opts.do_move)
			{
				/*
				 * only 1 or 2 arguments
				 * if first argument is reffile, module is added to this;
				 * last arg is module or reffile to add
				 */
				if (opts.verbose && !opts.silent)
					hyp_utf8_fprintf(opts.outfile, _("Adding/replacing files\n"));
				if (num_args == 1)
				{
					const char *modname = argv[c++];
					if (opts.refname == NULL)
						opts.refname = path_subst(gl_profile.general.all_ref);
					if (!ref_add_entries(opts.refname, modname, opts.do_move, opts.outfile, opts.verbose))
						retval = 1;
				} else if (num_args == 2 && opts.refname == NULL)
				{
					const char *modname = argv[c++];

					opts.refname = g_strdup(argv[c++]);
					modname = argv[c++];
					if (!ref_add_entries(opts.refname, modname, opts.do_move, opts.outfile, opts.verbose))
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
			} else if (opts.do_delete)
			{
				/*
				 * first arg is maybe ref file name.
				 * other args are modules to be deleted
				 */
				if (num_args > 0 && hyp_guess_filetype(argv[c]) == HYP_FT_REF && opts.refname == NULL)
				{
					opts.refname = g_strdup(argv[c++]);
					num_args--;
				} else if (opts.refname == NULL)
				{
					opts.refname = path_subst(gl_profile.general.all_ref);
				}
				if (num_args == 0)
				{
					usage_error(_("no files to delete"));
					retval = 1;
				} else
				{
					if (!ref_del_entries(opts.refname, num_args, &argv[c], opts.outfile, opts.verbose))
						retval = 1;
				}
			} else
			{
				retval = 1;
				unreachable();
			}
		}
		
		if (retval == 0)
		{
			if ((changed && opts.catalog != 0) || opts.catalog == 2)
			{
			}
		}
	}
	
	g_free(opts.refname);
	HypProfile_Delete();
	x_free_resources();

	return retval;
}
