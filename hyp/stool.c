#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include <dirent.h>
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "hv_vers.h"


#define CATALOG_TXT "catalog.txt"

#define LABEL_PREFIX "__"
#define STR_INDENT   "    "

#define STR_MAINTITLE  0
#define STR_DATABASE   1
#define STR_CATAUTHOR  2
#define STR_HELPTITLE  3
#define STR_UNKNOWN    4
#define STR_AUTHOR     5
#define STR_VERSION    6
#define STR_FILE       7
#define STR_MISC       8
#define STR_REMAIN     9
#define STR_CREATED    10
#define STR_MAX        11

char const gl_program_name[] = "STooL";

static gboolean do_shrink;
static gboolean do_pullup;
static gboolean do_create;
static gboolean do_index_only;
static gboolean do_compile;
static gboolean do_exonly;
static gboolean do_updateref;
static gboolean do_help;
static gboolean do_version;


static HYP_OS os;
static char *linebuf;
static size_t linebuf_size;
static struct {
	char *val;
	const char *def;
} strings[STR_MAX] = {
	{ NULL, N_("Available hypertexts") },
	{ NULL, N_("Hypertext of hypertexts") },
	{ NULL, N_("Created automatically") },
	{ NULL, N_("Help for this hypertext") },
	{ NULL, N_("Unknown") },
	{ NULL, N_("Author") },
	{ NULL, N_("Version") },
	{ NULL, N_("File") },
	{ NULL, N_("Others") },
	{ NULL, N_("All texts that cannot be classified in the above categories.") },
	{ NULL, N_("This entry has been created automatically.") }
};

static char *help_page;


#define memassert(p) if (G_UNLIKELY((p) == NULL)) oom()


/* UTF-8 of \u2713 */
#define S_CHECKMARK "\342\234\223"

typedef struct _cfg CFG;
struct _cfg {
	CFG *next;
	char *filename;
	gboolean available;
	char *database;
	char *author;
	char *subject;
	char *nodename;
	gboolean defined;
	char *version;
	char *keywords;
	char *desc;
	CFG *right; /* next node on same level or in list of entries */
	CFG *down;  /* down the tree */
	CFG *up;    /* parent */
	CFG *list;  /* list of entries */
	long labelnr;
};

static CFG *cfg_list;
static int hyp_count;
static HYP_CHARSET output_charset;
static HYP_CHARSET input_charset;


/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static __attribute__((noreturn)) void oom(void)
{
	fputs(strerror(ENOMEM), stderr);
	fputs("\n", stderr);
	abort();
}

/* ------------------------------------------------------------------------- */

static char *stg_quote_name(const char *name)
{
	char *str, *ret;
	size_t len;
	
	len = strlen(name);
	str = ret = g_new(char, len * 2 + 1);
	if (str != NULL)
	{
		while (len && *name)
		{
			if (*name == '\\' || *name == '"')
				*str++ = '\\';
			*str++ = *name++;
			len--;
		}
		*str = '\0';
	}
	return ret;
}

/* ------------------------------------------------------------------------- */

static char *readline(FILE *fp)
{
	size_t sl;
	int c;
	
	sl = 0;
	for (;;)
	{
		c = getc(fp);

		if (c == EOF)
		{
			if (sl == 0)
				return NULL;
			break;
		}
		if (c == 0x0d)
		{
			c = getc(fp);
			if (c != 0x0a)
				ungetc(c, fp);
			break;
		}
		if (c == 0x0a)
			break;
		if (sl >= linebuf_size)
		{
			linebuf_size += 1020;
			linebuf = g_renew(char, linebuf, linebuf_size + 1);
			memassert(linebuf);
		}
		linebuf[sl++] = c;
	}
	if (sl >= linebuf_size)
	{
		linebuf_size += 1020;
		linebuf = g_renew(char, linebuf, linebuf_size + 1);
		memassert(linebuf);
	}
	linebuf[sl] = '\0';
	/*
	 * FIXME: need some indication which charset
	 * is used for catalog.txt/catalog.cfg
	 */
	return hyp_conv_to_utf8(input_charset, linebuf, STR0TERM);
}

/* ------------------------------------------------------------------------- */

static char *read_nonempty_line(FILE *fp)
{
	for (;;)
	{
		char *line = readline(fp);
		if (line == NULL)
			return NULL;
		g_strchomp(line);
		g_strchug(line);
		if (*line != '\0' && *line != '#')
			return line;
		g_free(line);
	}
}

/* ------------------------------------------------------------------------- */

static void append_line(char **lines, const char *line)
{
	char *tmp;
	
	if (*lines)
	{
		tmp = g_strconcat(*lines, "\n", line, NULL);
		g_free(*lines);
		*lines = tmp;
	} else
	{
		*lines = g_strdup(line);
	}
	memassert(*lines);
}

/* ------------------------------------------------------------------------- */

/*
 * make sure that subject
 * - does only contain forward slashes and no backslashes
 * - does not contain any leading slashes
 * - does not contain any trailing slashes
 * - does not contain empty strings between slashes
 */
static void fix_subject(char **subject)
{
	char *p;
	
	p = *subject;
	if (p == NULL)
		return;
	/* convert slashes */
	while (*p)
	{
		if (*p == '\\')
			*p = '/';
		p++;
	}
	/* remove duplicates */
	p = *subject;
	while (*p)
	{
		p++;
		if (p[-1] == '/' && p[0] == '/')
			memmove(p, p + 1, strlen(p));
	}
	/* kill trailing slashes */
	while (p > *subject && p[-1] == '/')
		*--p = '\0';
	/* kill leading slashes */
	p = *subject;
	while (*p == '/')
		memmove(p, p + 1, strlen(p));
	/* kill subject if is empty now */
	if (*p == '\0')
		g_freep(subject);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void read_catalogtxt(FILE *fp)
{
	char *line;
	
	input_charset = HYP_CHARSET_NONE;
	line = read_nonempty_line(fp);
	if (line && strncmp(line, "@inputenc ", 10) == 0)
	{
		input_charset = hyp_charset_from_name(line + 10);
		g_free(line);
		line = read_nonempty_line(fp);
	}	
	if (input_charset == HYP_CHARSET_NONE)
		input_charset = hyp_get_current_charset();
	
	while (line && *line == '>')
	{
		if (empty(line + 1))
		{
			hyp_utf8_fprintf(stderr, _("%s: empty pathname\n"), gl_program_name);
		}
		{
			CFG *l;
			
			for (l = cfg_list; l != NULL; l = l->next)
			{
				if (l->filename && g_utf8_strcasecmp(line + 1, hyp_basename(l->filename)) == 0)
					break;
			}
			if (l == NULL && (!do_exonly || hyp_guess_filetype(line + 1) != HYP_FT_HYP))
			{
				l = g_new0(CFG, 1);
				memassert(l);
				l->next = cfg_list;
				cfg_list = l;
				l->filename = g_strdup(line + 1);
				if (hyp_guess_filetype(l->filename) != HYP_FT_HYP)
					l->available = TRUE;
			}
			g_free(line);
			line = read_nonempty_line(fp);

			/* read author */
			if (line && *line == '>')
			{
				if (l && l->author == NULL)
					l->author = g_strdup(line + 1);
				g_free(line);
				line = read_nonempty_line(fp);
			}
			
			/* read subject */
			if (line && *line == '>')
			{
				if (l /* && l->subject == NULL */)
				{
					g_free(l->subject);
					l->subject = g_strdup(line + 1);
					fix_subject(&l->subject);
					l->defined = FALSE;
				}
				g_free(line);
				line = read_nonempty_line(fp);
			}
			
			/* read database */
			if (line && *line == '>')
			{
				if (l && l->database == NULL)
					l->database = g_strdup(line + 1);
				g_free(line);
				line = read_nonempty_line(fp);
			}
			
			/* read version */
			if (line && *line == '>')
			{
				if (l && l->version == NULL)
					l->version = g_strdup(line + 1);
				g_free(line);
				line = read_nonempty_line(fp);
			}
			
			/* read keywords */
			if (line && *line == '!')
			{
				if (l && l->keywords == NULL)
					l->keywords = g_strdup(line + 1);
				g_free(line);
				line = read_nonempty_line(fp);
			}
			
			/* read description */
			while (line && *line != '>')
			{
				if (l)
					append_line(&l->desc, line);
				g_free(line);
				line = read_nonempty_line(fp);
			}
		}
	}
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void set_default_opts(void)
{
	if (do_shrink < 0)
		do_shrink = FALSE;
	if (do_pullup < 0)
		do_pullup = FALSE;
	if (do_create < 0)
		do_create = FALSE;
	if (do_index_only < 0)
		do_index_only = FALSE;
	if (do_compile < 0)
		do_compile = TRUE;
	if (do_exonly < 0)
		do_exonly = FALSE;
	if (do_updateref < 0)
		do_updateref = FALSE;
}

/* ------------------------------------------------------------------------- */

static void read_config(FILE *fp)
{
	char *line;
	gboolean found;
	
	input_charset = HYP_CHARSET_NONE;
	line = read_nonempty_line(fp);
	if (line && strncmp(line, "@inputenc ", 10) == 0)
	{
		input_charset = hyp_charset_from_name(line + 10);
		g_free(line);
		line = read_nonempty_line(fp);
	}	
	if (input_charset == HYP_CHARSET_NONE)
		input_charset = hyp_get_current_charset();
	
	while (line && (*line == '+' || *line == '-'))
	{
		if (g_ascii_strcasecmp(line + 1, "SHRINK") == 0)
		{
			if (do_shrink < 0)
				do_shrink = *line == '+';
		} else if (g_ascii_strcasecmp(line + 1, "PULLUP") == 0)
		{
			if (do_pullup < 0)
				do_pullup = *line == '+';
		} else if (g_ascii_strcasecmp(line + 1, "CREATE") == 0)
		{
			if (do_create < 0)
				do_create = *line == '+';
		} else if (g_ascii_strcasecmp(line + 1, "INDEX") == 0)
		{
			if (do_index_only < 0)
				do_index_only = *line == '+';
		} else if (g_ascii_strcasecmp(line + 1, "COMPILE") == 0)
		{
			if (do_compile < 0)
				do_compile = *line == '+';
		} else if (g_ascii_strcasecmp(line + 1, "EXONLY") == 0)
		{
			if (do_exonly < 0)
				do_exonly = *line == '+';
		} else if (g_ascii_strcasecmp(line + 1, "UPDATEREF") == 0)
		{
			if (do_updateref < 0)
				do_updateref = *line == '+';
		} else
		{
			hyp_utf8_fprintf(stderr, _("%s: unknown option %s\n"), gl_program_name, line);
		}
		g_free(line);
		line = read_nonempty_line(fp);
	}
	
	do
	{
		found = FALSE;
		
		if (line && g_ascii_strcasecmp(line, ">STRINGS") == 0)
		{
			int i;
			
			found = TRUE;
			g_free(line);
			line = read_nonempty_line(fp);
			i = 0;
			for (;;)
			{
				if (line == NULL || *line == '>')
					break;
				if (i < STR_MAX)
				{
					strings[i].val = line;
					i++;
				} else
				{
					hyp_utf8_fprintf(stderr, _("%s: too many strings\n"), gl_program_name);
					g_free(line);
				}
				line = read_nonempty_line(fp);
			}
		}
	
		if (line && g_ascii_strcasecmp(line, ">TRANS") == 0)
		{
			char *p;
			CFG *l;
			
			found = TRUE;
			g_free(line);
			line = read_nonempty_line(fp);
			for (;;)
			{
				if (line == NULL || *line == '>')
					break;
				p = strchr(line, '=');
				if (p != NULL)
				{
					*p++ = 0;
					g_strchomp(line);
					g_strchug(p);
					fix_subject(&line);
					p = g_strdup(p);
					fix_subject(&p);
					if (!empty(line) && !empty(p))
					{
						for (l = cfg_list; l != NULL; l = l->next)
						{
							if (l->subject && strcmp(l->subject, line) == 0)
							{
								g_free(l->subject);
								l->subject = g_strdup(p);
							}
						}
					}
					g_free(p);
				}
				line = read_nonempty_line(fp);
			}
		}

		if (line && g_ascii_strcasecmp(line, ">HELPPAGE") == 0)
		{
			found = TRUE;
			g_free(line);
			line = readline(fp);
			if (line != NULL)
			{
				g_strchomp(line);
			}
			for (;;)
			{
				if (line == NULL || *line == '>')
					break;
				if (*line != '#')
				{
					append_line(&help_page, line);
				}
				g_free(line);
				line = readline(fp);
				if (line != NULL)
				{
					g_strchomp(line);
				}
			}
		}
	} while (found);
	
	while (line && *line == '>')
	{
		if (empty(line + 1))
		{
			hyp_utf8_fprintf(stderr, _("%s: empty pathname\n"), gl_program_name);
		}
		{
			CFG *l;
			
			l = g_new0(CFG, 1);
			memassert(l);
			l->next = cfg_list;
			cfg_list = l;
			l->subject = g_strdup(line + 1);
			l->defined = TRUE;
			fix_subject(&l->subject);
			g_free(line);
			line = read_nonempty_line(fp);
			while (line && *line != '>' && *line != '.')
			{
				append_line(&l->desc, line);
				g_free(line);
				line = read_nonempty_line(fp);
			}
		}
	}
	
	while (line && *line == '.')
	{
		if (empty(line + 1))
		{
			hyp_utf8_fprintf(stderr, _("%s: empty pathname\n"), gl_program_name);
		}
		{
			CFG *l;
			char *subject;
			
			subject = read_nonempty_line(fp);
			if (subject == NULL)
				break;
			fix_subject(&subject);
			
			if (!empty(subject))
			{
				for (l = cfg_list; l != NULL; l = l->next)
				{
					if (l->filename && g_utf8_strcasecmp(line + 1, hyp_basename(l->filename)) == 0)
					{
						g_free(l->subject);
						l->subject = g_strdup(subject);
					}
				}
			}
			g_free(subject);
			g_free(line);
			line = read_nonempty_line(fp);
		}
		g_free(line);
		line = read_nonempty_line(fp);
	}
	
	g_free(line);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

struct find_args {
	const char *catalog_file;
};

static gboolean maybe_load_hypfile(const char *filename, void *data)
{
	CFG *l;
	const char *base = hyp_basename(filename);
	const char *catalog_file = ((struct find_args *)data)->catalog_file;
	
	if (hyp_guess_filetype(base) == HYP_FT_HYP)
	{
		HYP_DOCUMENT *hyp;
		int handle;
		hyp_filetype err = HYP_FT_NONE;
		
		/* to not try to load "all.hyp" here */
		if (g_utf8_strcasecmp(base, "all.hyp") == 0)
			return TRUE;
		
		/* do not try to load the old catalog here */
		if (g_utf8_strcasecmp(base, "catalog.hyp") == 0)
			return TRUE;
		if (g_utf8_strcasecmp(base, "katalog.hyp") == 0)
			return TRUE;
		if (g_utf8_strcasecmp(base, hyp_basename(catalog_file)) == 0)
			return TRUE;
		
		/* ignore the output file of HypFind */
		if (g_utf8_strcasecmp(base, HYP_FILENAME_HYPFIND) == 0)
			return TRUE;
		
		++hyp_count;
		l = g_new0(CFG, 1);
		memassert(l);
		l->next = cfg_list;
		handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (handle < 0)
		{
			g_free(l);
			return TRUE;
		}
		if ((hyp = hyp_load(handle, &err)) == NULL)
		{
			hyp_utf8_close(handle);
			g_free(l);
			return TRUE;
		}
		l->filename = g_strdup(filename);
		
		cfg_list = l;
#if 1
		hyp_utf8_fprintf(stdout, "%s\n", l->filename);
#endif
		l->database = g_strdup(hyp->database);
		l->author = g_strdup(hyp->author);
		l->subject = g_strdup(hyp->subject);
		fix_subject(&l->subject);
		l->version = g_strdup(hyp->version);
		l->available = TRUE;
		hyp_unref(hyp);
		hyp_utf8_close(handle);
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean find_hypfiles(const char *list, const char *catalog_file)
{
	struct find_args args;
	
	hyp_utf8_fprintf(stdout, _("searching for hypertext files\n"));
	hyp_count = 0;
	args.catalog_file = catalog_file;
	walk_pathlist(list, maybe_load_hypfile, &args);
	if (cfg_list == NULL)
	{
		hyp_utf8_fprintf(stderr, _("%s: no hypertext files found\n"), gl_program_name);
	}
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#if 0
static int cmp_subject(const void *_l1, const void *_l2)
{
	const CFG *l1 = *(const CFG *const *)_l1;
	const CFG *l2 = *(const CFG *const *)_l2;
	
	if (l1->subject == NULL && l2->subject == NULL)
		return 0;
	if (l1->subject == NULL && l2->subject != NULL)
		return 1;
	if (l1->subject != NULL && l2->subject == NULL)
		return -1;
	return strcmp(l1->subject, l2->subject);
}
#endif

/* ------------------------------------------------------------------------- */

static int cmp_basename(const void *_l1, const void *_l2)
{
	const CFG *l1 = *(const CFG *const *)_l1;
	const CFG *l2 = *(const CFG *const *)_l2;
	char *n1 = ref_hyp_basename(l1->filename);
	char *n2 = ref_hyp_basename(l2->filename);
	int res = g_utf8_strcasecmp(n1, n2);
	g_free(n1);
	g_free(n2);
	return res;
}

/* ------------------------------------------------------------------------- */

static void insert_node(CFG *parent, CFG *l)
{
	const char *subject = l->subject;
	CFG *s;
	const char *p, *p2;
	size_t len;
	
	while (*subject)
	{
		s = parent->down;
		p = strchr(subject, '/');
		if (p == NULL)
		{
			len = strlen(subject);
			p = subject + len;
		} else
		{
			len = p - subject;
			p++;
		}
		while (s)
		{
			if (strncmp(s->nodename, subject, len) == 0 && s->nodename[len] == '\0')
				break;
			s = s->right;
		}
		if (s)
		{
			if (*p == '\0' && l->desc != NULL && s->desc == NULL)
			{
				/*
				 * me may have created the intermediate node before,
				 * copy the description only from the config file
				 */
				s->desc = l->desc;
				l->desc = NULL;
			}
			if (s->down == NULL && *p == '\0')
			{
				if (l->nodename == NULL)
				{
					l->nodename = g_strdup(subject);
					memassert(l->nodename);
				}
				parent = s;
			} else if (s->down == NULL && *p != '\0')
			{
				parent = s;
				p2 = strchr(p, '/');
				if (p2 == NULL)
				{
					s->down = l;
					if (l->nodename == NULL)
					{
						l->nodename = g_strdup(p);
						memassert(l->nodename);
					}
					l->up = s;
				} else
				{
					len = p2 - p;
					s->down = g_new0(CFG, 1);
					memassert(s->down);
					s->down->nodename = g_strndup(p, len);
					memassert(s->down->nodename);
					s->down->subject = g_strndup(l->subject, p - l->subject - 1);
					memassert(s->down->subject);
					s->down->up = s;
					parent = s->down;
				}
			} else if (s->down != NULL && *p != '\0')
			{
				parent = s;
				p2 = strchr(p, '/');
				if (p2 == NULL)
				{
					l->right = parent->down;
					parent->down = l;
					if (l->nodename == NULL)
					{
						l->nodename = g_strdup(p);
						memassert(l->nodename);
					}
					l->up = parent;
				} else
				{
					len = p2 - p;
					s = g_new0(CFG, 1);
					memassert(s);
					s->right = parent->down;
					parent->down = s;
					s->up = parent;
					s->nodename = g_strndup(p, len);
					memassert(s->nodename);
					s->subject = g_strndup(l->subject, p - l->subject - 1);
					memassert(s->subject);
					parent = s;
				}
			} else /* if (s->down != NULL && *p == '\0') */
			{
				parent = s;
			}
		} else
		{
			s = parent;
			if (s->down == NULL && *p == '\0')
			{
				if (l->nodename == NULL)
				{
					l->nodename = g_strdup(subject);
					memassert(l->nodename);
				}
				s->down = l;
				l->up = parent;
				parent = l;
			} else if (s->down == NULL && *p != '\0')
			{
				s->down = g_new0(CFG, 1);
				memassert(s->down);
				s->down->nodename = g_strndup(subject, len);
				memassert(s->down->nodename);
				s->down->subject = g_strndup(l->subject, p - l->subject - 1);
				memassert(s->down->subject);
				parent = s->down;
			} else if (s->down != NULL && *p != '\0')
			{
				s = g_new0(CFG, 1);
				memassert(s);
				s->right = parent->down;
				parent->down = s;
				s->nodename = g_strndup(subject, len);
				memassert(s->nodename);
				s->subject = g_strndup(l->subject, p - l->subject - 1);
				memassert(s->subject);
				s->up = parent;
				parent = s;
			} else /* if (s->down != NULL && *p == '\0') */
			{
				if (l->nodename == NULL)
				{
					l->nodename = g_strdup(subject);
					memassert(l->nodename);
				}
				l->right = parent->down;
				parent->down = l;
				l->up = parent;
				parent = l;
			}
		}
		if (*p == '/')
			p++;
		subject = p;
	}
}

/* ------------------------------------------------------------------------- */

static void insert_filename(CFG *parent, CFG *l)
{
	const char *subject = l->subject;
	CFG *s;
	const char *p;
	size_t len;
	
	if (subject != NULL)
	{
		while (*subject)
		{
			s = parent->down;
			p = strchr(subject, '/');
			if (p == NULL)
			{
				len = strlen(subject);
				p = subject + len;
			} else
			{
				len = p - subject;
				p++;
			}
			while (s)
			{
				if (strncmp(s->nodename, subject, len) == 0 && s->nodename[len] == '\0')
					break;
				s = s->right;
			}
			if (s == NULL)
			{
				s = parent->down;
				if (s == NULL && *p != '\0')
				{
					/*
					 * still can happen if only part of the nodepath has been
					 * defined in catalog.cfg,
					 * i.e inserting "a/b/c" into "a" only.
					 */
					break;
				}
				while (s && s->right)
					s = s->right;
			}
			if (*p == '\0')
			{
				if (s != NULL)
					parent = s;
				break;
			}
			parent = s;
			subject = p;
		}
		s = parent;
		while (s->down)
		{
			s = s->down;
			while (s->right)
				s = s->right;
		}
	} else
	{
		hyp_utf8_printf(_("no subject: %s\n"), l->filename);
		s = parent;
		while (s->down)
		{
			s = s->down;
			while (s->right)
				s = s->right;
		}
	}
	l->up = s;
	l->list = s->list;
	s->list = l;
}

/* ------------------------------------------------------------------------- */

static void dump_tree(CFG *l, int level)
{
	int i;
	
	while (l)
	{
		for (i = 0; i < level; i++)
			hyp_utf8_printf("    ");
		hyp_utf8_printf("%-20s %s\n", printnull(l->nodename), printnull(l->subject));
#if 1
		{
			CFG *s;
			for (s = l->list; s != NULL; s = s->list)
			{
				for (i = 0; i < level + 1; i++)
					hyp_utf8_printf("    ");
				hyp_utf8_printf("%s %s\n", printnull(s->filename), printnull(s->subject));
			}
		}
#endif
		dump_tree(l->down, level + 1);
		l = l->right;
	}
}

/* ------------------------------------------------------------------------- */

static void make_labels(CFG *l, long *labelnr)
{
	while (l)
	{
		++(*labelnr);
		l->labelnr = *labelnr;
		{
			CFG *s;
			for (s = l->list; s != NULL; s = s->list)
			{
				++(*labelnr);
				s->labelnr = *labelnr;
			}
		}
		make_labels(l->down, labelnr);
		l = l->right;
	}
}

/* ------------------------------------------------------------------------- */

static gboolean in_tree(CFG *l, CFG *search)
{
	while (l)
	{
		if (l == search)
			return TRUE;
		{
			CFG *s;
			for (s = l->list; s != NULL; s = s->list)
			{
				if (s == search)
					return TRUE;
			}
		}
		if (in_tree(l->down, search))
			return TRUE;
		l = l->right;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static void free_cfg(CFG *l)
{
	g_free(l->filename);
	g_free(l->database);
	g_free(l->author);
	g_free(l->version);
	g_free(l->subject);
	g_free(l->keywords);
	g_free(l->desc);
	g_free(l->nodename);
	g_free(l);
}

/* ------------------------------------------------------------------------- */

static CFG *build_tree(void)
{
	size_t i, count;
	CFG *l;
	CFG **arr;
	CFG *root = NULL;
	size_t num_filenames;
	
	for (count = 0, l = cfg_list; l != NULL; l = l->next, count++)
		;
	if (count == 0)
		return NULL;
	arr = g_new(CFG *, count);
	memassert(arr);
	for (i = 0, l = cfg_list; l != NULL; l = l->next, i++)
		arr[i] = l;
#if 0
	qsort(arr, count, sizeof(CFG *), cmp_subject);
#endif
	
	/*
	 * Build the tree, starting from the last entry because
	 * earlier entries might need to see the leaves that are built.
	 * (the last entry is the first in the array because
	 * the list was built in reverse order).
	 */
	root = g_new0(CFG, 1);
	memassert(root);
	root->nodename = g_strdup(hyp_default_main_node_name);
	memassert(root->nodename);
	root->subject = g_strdup(strings[STR_MAINTITLE].val);
	memassert(root->subject);
	
	for (i = 0; i < count; i++)
	{
		l = arr[i];
		if (l->subject)
		{
			if (l->defined || do_create)
			{
				insert_node(root, l);
			}
		}
	}
#if 0
	dump_tree(root->down, 0);
#endif
	if (root->down == NULL)
	{
		/* no subjects defined at all */
		root->down = l = g_new0(CFG, 1);
		memassert(l);
		l->subject = g_strdup(strings[STR_MISC].val);
		memassert(l->subject);
		l->nodename = g_strdup(strings[STR_MISC].val);
		memassert(l->nodename);
		l->desc = g_strdup(strings[STR_REMAIN].val);
		l->defined = TRUE;
		l->up = root;
	}
	
	num_filenames = 0;
	for (i = 0; i < count; i++)
	{
		l = arr[i];
		if (l && l->filename)
		{
			insert_filename(root, l);
			num_filenames++;
		}
	}
	if (num_filenames == 0)
		hyp_utf8_printf(_("No files found\n"));
	else
		hyp_utf8_printf(P_("%lu file inserted into tree\n", "%lu files inserted into tree\n", num_filenames), (unsigned long)num_filenames);
	
	for (i = 0; i < count; i++)
	{
		l = arr[i];
		if (l && !in_tree(root, l))
		{
			/*
			 * should only be possible for nodes in the middle of the tree
			 */
			/* hyp_utf8_printf("not in tree: %s %s\n", printnull(l->filename), printnull(l->subject)); */
			free_cfg(l);
		}
	}
	
	g_free(arr);
	return root;
}

/* ------------------------------------------------------------------------- */

static void shrink_tree(CFG **parent)
{
	CFG *l, *next;
	CFG **prev;
	
	l = *parent;
	if (l == NULL)
		return;
	prev = parent;
	while (l)
	{
		shrink_tree(&l->down);
		next = l->right;
		if (l->down == NULL && l->list == NULL)
		{
			*prev = next;
			free_cfg(l);
		} else if (do_pullup && l->down != NULL && l->down->right == NULL && l->list == NULL)
		{
			*prev = l;
			l->down->right = next;
			l->right = next = l->down;
			l->down = NULL;
#if 0
			g_free(next->nodename);
			next->nodename = g_strdup(l->nodename);
#endif
			prev = &l->right;
		} else
		{
			prev = &l->right;
		}
		l = next;
	}
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *linkname(CFG *l)
{
	if (l->up == NULL)
		return g_strdup(hyp_default_main_node_name);
	return g_strdup_printf("%s%ld", LABEL_PREFIX, l->labelnr);
}

/* ------------------------------------------------------------------------- */

static void print_str(FILE *outfile, const char *indent, const char *str, gboolean skip_indent)
{
	char buf[HYP_UTF8_CHARMAX + 1];
	gboolean converror = FALSE;
	
	if (indent && (!skip_indent || (*str != '@' || (str[1] != '{' && str[1] != '@'))))
		fputs(indent, outfile);
	while (*str)
	{
		if (*str == '@')
		{
			fputc('@', outfile);
			fputc('@', outfile);
			str++;
		} else if (*str == '\n')
		{
			fputc('\n', outfile);
			if (indent && (!skip_indent || (str[1] != '@' || (str[2] != '{' && str[2] != '@'))))
				fputs(indent, outfile);
			str++;
		} else if (*((const unsigned char *)str) < 0x80)
		{
			fputc(*str, outfile);
			str++;
		} else
		{
			str = hyp_utf8_conv_char(output_charset, str, buf, &converror);
			fputs(buf, outfile);
		}
	}
	if (indent)
		fputc('\n', outfile);
}

/* ------------------------------------------------------------------------- */

static void print_dir(FILE *outfile, CFG *l)
{
	char *str;
	char *title;
	
	while (l)
	{
		title = stg_quote_name(l->nodename);
		str = linkname(l);
		hyp_utf8_fprintf_charset(outfile, output_charset, "%s@{\"%s\" link \"%s\"}\n", STR_INDENT, title, str);
		g_free(str);
		g_free(title);
		if (l->desc)
			print_str(outfile, STR_INDENT, l->desc, TRUE);
		
		l = l->right;
		if (l)
			hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	}
}

/* ------------------------------------------------------------------------- */

static void print_filelist(FILE *outfile, CFG *l)
{
	char *str;
	char *tmp;
	char *name;
	
	while (l)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@label %s%ld\n", LABEL_PREFIX, l->labelnr);
		if (l->keywords)
		{
			str = stg_quote_name(l->keywords);
			hyp_utf8_fprintf_charset(outfile, output_charset, "@keywords \"%s\"\n", str);
			g_free(str);
		}
		
		if (l->database)
		{
			str = stg_quote_name(l->database);
		} else
		{
			tmp = ref_hyp_basename(l->filename);
			str = stg_quote_name(tmp);
			g_free(tmp);
		}
		if (hyp_guess_filetype(l->filename) == HYP_FT_HYP)
			tmp = g_strdup_printf("%s/%s", hyp_basename(l->filename), hyp_default_main_node_name);
		else
			tmp = g_strdup_printf("%s/%s", l->filename, hyp_default_main_node_name);
		name = stg_quote_name(tmp);
		g_free(tmp);
		hyp_utf8_fprintf_charset(outfile, output_charset, "%s@{\"%s\" link \"%s\"}\n", STR_INDENT, str, name);
		g_free(name);
		g_free(str);
		
		if (l->author)
			str = l->author;
		else
			str = strings[STR_UNKNOWN].val;
		hyp_utf8_fprintf_charset(outfile, output_charset, "%s%s: %s\n", STR_INDENT, strings[STR_AUTHOR].val, str);
		
		if (l->version)
			str = l->version;
		else
			str = strings[STR_UNKNOWN].val;
		hyp_utf8_fprintf_charset(outfile, output_charset, "%s%s: %s\n", STR_INDENT, strings[STR_VERSION].val, str);
		
		if (hyp_guess_filetype(l->filename) == HYP_FT_HYP)
			tmp = g_strdup(hyp_basename(l->filename));
		else
			tmp = g_strdup_printf(l->filename);
		hyp_utf8_fprintf_charset(outfile, output_charset, "%s%s: %s%s%s\n", STR_INDENT, strings[STR_FILE].val, l->available ? "" : "@{G}", tmp, l->available ? "" : "@{g}");
		g_free(tmp);
		
		if (l->desc)
			print_str(outfile, STR_INDENT, l->desc, TRUE);
		
		l = l->list;
		if (l)
			hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	}
}

/* ------------------------------------------------------------------------- */

static void print_nodes(FILE *outfile, CFG *l)
{
	CFG *prev = NULL;
	char *str;
	char *title;
	
	while (l)
	{
		str = linkname(l);
		title = stg_quote_name(l->subject);
		hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
		hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
		hyp_utf8_fprintf_charset(outfile, output_charset, "@node \"%s\" \"%s\"\n", str, title);
		g_free(title);
		g_free(str);
		if (l->down == NULL && l->up != NULL)
		{
			str = stg_quote_name(l->nodename);
			hyp_utf8_fprintf_charset(outfile, output_charset, "@keywords \"%s\"\n", str);
			g_free(str);
		}
		
		if (l->up)
		{
			str = linkname(l->up);
			hyp_utf8_fprintf_charset(outfile, output_charset, "@toc \"%s\"\n", str);
			g_free(str);
		}
		if (prev)
			str = linkname(prev);
		else
			str = linkname(l);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@prev \"%s\"\n", str);
		g_free(str);
		if (l->right)
			str = linkname(l->right);
		else
			str = linkname(l);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@next \"%s\"\n", str);
		g_free(str);
		if (l->down)
			print_dir(outfile, l->down);
		else
			print_filelist(outfile, l->list);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@endnode\n");
		
		print_nodes(outfile, l->down);
		
		prev = l;
		l = l->right;
	}
}

/* ------------------------------------------------------------------------- */

static char const index_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ*";
#define NUM_INDEX_CHARS ((int)sizeof(index_chars) - 1)

struct index_info {
	FILE *outfile;
	int used[NUM_INDEX_CHARS];
	int max_filename;
	int index_char;
	int filenum;
	CFG **files;
};


static void check_used(CFG *l, struct index_info *info)
{
	char *str = ref_hyp_basename(l->filename);
	int len = (int)strlen(str);
	int c;
	const char *p;
	
	c = toupper(*str);
	if ((p = strchr(index_chars, c)) != NULL)
		c = (int)(p - index_chars);
	else
		c = NUM_INDEX_CHARS - 1;
	info->used[c] += 1;
	if (len > info->max_filename)
		info->max_filename = len;
	g_free(str);
}

/* ------------------------------------------------------------------------- */

static void add_index_file(CFG *l, struct index_info *info)
{
	char *str = ref_hyp_basename(l->filename);
	int c;
	const char *p;
	
	c = toupper(*str);
	if ((p = strchr(index_chars, c)) != NULL)
		c = (int)(p - index_chars);
	else
		c = NUM_INDEX_CHARS - 1;
	if (c == info->index_char)
	{
		info->files[info->filenum] = l;
		info->filenum++;
	}
}

/* ------------------------------------------------------------------------- */

static void do_files(CFG *l, void (*func)(CFG *l, struct index_info *info), struct index_info *info)
{
	CFG *s;
	
	while (l)
	{
		for (s = l->list; s != NULL; s = s->list)
		{
			func(s, info);
		}
		do_files(l->down, func, info);
		l = l->right;
	}
}

/* ------------------------------------------------------------------------- */

static void print_index(FILE *outfile, CFG *root)
{
	struct index_info info;
	char *nodename;
	int i, j;
	char *str, *label, *tmp;
	CFG *l;
	int len;
	gboolean need_nl;
	
	memset(&info, 0, sizeof(info));
	info.outfile = outfile;
	do_files(root, check_used, &info);
	hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	hyp_utf8_fprintf_charset(outfile, output_charset, "@node \"%s\"\n", hyp_default_index_node_name);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@next \"%s\"\n", hyp_default_index_node_name);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@prev \"%s\"\n", hyp_default_index_node_name);
	need_nl = FALSE;
	for (i = 0; i < NUM_INDEX_CHARS; i++)
	{
		info.index_char = i;
		if (info.used[i] > 0)
		{
			if (need_nl)
				fputs("\n", outfile);
			need_nl = TRUE;
			info.filenum = 0;
			info.files = g_new(CFG *, info.used[i]);
			memassert(info.files);
			do_files(root, add_index_file, &info);
			qsort(info.files, info.used[i], sizeof(CFG *), cmp_basename);
			for (j = 0; j < info.used[i]; j++)
			{
				l = info.files[j];
				tmp = ref_hyp_basename(l->filename);
				str = stg_quote_name(tmp);
				nodename = linkname(l->up);
				label = linkname(l);
				hyp_utf8_fprintf_charset(outfile, output_charset, "  %c  %s @{\"%s\" link \"%s\" %s}", j == 0 ? index_chars[i] : ' ', l->available ? S_CHECKMARK : " ", str, nodename, label);
				len = (int)g_utf8_str_len(tmp, STR0TERM);
				while (len < info.max_filename)
				{
					fputc(' ', outfile);
					len++;
				}
				g_free(label);
				g_free(nodename);
				g_free(str);
				g_free(tmp);
				fputs("  ", outfile);
				if (l->database)
					print_str(outfile, NULL, l->database, FALSE);
				fputs("\n", outfile);
			}
			g_free(info.files);
		}
	}
	hyp_utf8_fprintf_charset(outfile, output_charset, "@endnode\n");
}

/* ------------------------------------------------------------------------- */

static void print_index_only(FILE *outfile, CFG *root)
{
	struct index_info info;
	char *nodename;
	int i, j;
	char *str, *label, *tmp;
	CFG *l;
	int len;
	gboolean need_nl;
	
	memset(&info, 0, sizeof(info));
	info.outfile = outfile;
	do_files(root, check_used, &info);
	hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	nodename = linkname(root);
	str = stg_quote_name(root->subject);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@node \"%s\" \"%s\"\n", nodename, str);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@next \"%s\"\n", nodename);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@prev \"%s\"\n", nodename);
	g_free(str);
	g_free(nodename);
	need_nl = FALSE;
	for (i = 0; i < NUM_INDEX_CHARS; i++)
	{
		info.index_char = i;
		if (info.used[i] > 0)
		{
			if (need_nl)
				fputs("\n", outfile);
			need_nl = TRUE;
			info.filenum = 0;
			info.files = g_new(CFG *, info.used[i]);
			memassert(info.files);
			do_files(root, add_index_file, &info);
			qsort(info.files, info.used[i], sizeof(CFG *), cmp_basename);
			for (j = 0; j < info.used[i]; j++)
			{
				l = info.files[j];
				tmp = ref_hyp_basename(l->filename);
				str = stg_quote_name(tmp);
				if (hyp_guess_filetype(l->filename) == HYP_FT_HYP)
					nodename = g_strdup_printf("%s/%s", hyp_basename(l->filename), hyp_default_main_node_name);
				else
					nodename = g_strdup_printf("%s/%s", l->filename, hyp_default_main_node_name);
				label = stg_quote_name(nodename);
				hyp_utf8_fprintf_charset(outfile, output_charset, "  %c  %s @{\"%s\" link \"%s\"}", j == 0 ? index_chars[i] : ' ', l->available ? S_CHECKMARK : " ", str, label);
				len = (int)g_utf8_str_len(tmp, STR0TERM);
				while (len < info.max_filename)
				{
					fputc(' ', outfile);
					len++;
				}
				g_free(label);
				g_free(nodename);
				g_free(str);
				g_free(tmp);
				fputs("  ", outfile);
				if (l->database)
					print_str(outfile, NULL, l->database, FALSE);
				fputs("\n", outfile);
			}
			g_free(info.files);
		}
	}
	hyp_utf8_fprintf_charset(outfile, output_charset, "@endnode\n");
}

/* ------------------------------------------------------------------------- */

static void print_helppage(FILE *outfile)
{
	char *nodename = stg_quote_name(hyp_default_help_node_name);
	char *str;
	
	hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	hyp_utf8_fprintf_charset(outfile, output_charset, "\n");
	str = stg_quote_name(strings[STR_HELPTITLE].val);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@node \"%s\" \"%s\"\n", nodename, str);
	g_free(str);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@prev \"%s\"\n", nodename);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@next \"%s\"\n", nodename);
	print_str(outfile, STR_INDENT, help_page, TRUE);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@endnode\n");
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void free_cfgs(CFG *l)
{
	CFG *next, *s;
	
	while (l)
	{
		for (s = l->list; s != NULL; s = next)
		{
			next = s->list;
			free_cfg(s);
		}
		next = l->right;
		free_cfgs(l->down);
		free_cfg(l);
		l = next;
	}
	cfg_list = NULL;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean create_output(char **outfile_name)
{
	char *pathlist;
	char *hyptop;
	char *config_name;
	FILE *config;
	char *tmp;
	char *catalogtxt_name;
	FILE *catalogtxt;
	char *dirname;
	CFG *root;
	char *str;
	long labelnr;
	gboolean retval = TRUE;
	
	if (getenv("PATHS"))
		pathlist = g_strdup(getenv("PATHS"));
	else
		pathlist = path_subst(gl_profile.general.path_list);
	if (getenv("HYPTOP"))
		hyptop = g_strdup(getenv("HYPTOP"));
	else
		hyptop = path_subst(gl_profile.viewer.catalog_file);
	
	config_name = replace_ext(hyptop, NULL, ".cfg");
	config = fopen(config_name, "rb");
	if (config == NULL)
	{
		g_free(config_name);
		tmp = g_build_filename("$APPDATA", hyp_basename(hyptop), NULL);
		config_name = replace_ext(tmp, NULL, ".cfg");
		g_free(tmp);
		tmp = config_name;
		config_name = path_subst(tmp);
		g_free(tmp);
		config = fopen(config_name, "rb");
	}
	if (config == NULL)
	{
		g_free(config_name);
		tmp = g_build_filename("$BINDIR", hyp_basename(hyptop), NULL);
		config_name = replace_ext(tmp, NULL, ".cfg");
		g_free(tmp);
		tmp = config_name;
		config_name = path_subst(tmp);
		g_free(tmp);
		config = fopen(config_name, "rb");
	}
	if (config == NULL)
	{
		config_name = replace_ext(hyp_basename(hyptop), NULL, ".cfg");
		config = fopen(config_name, "rb");
	}
	
	dirname = hyp_path_get_dirname(hyptop);
	catalogtxt_name = g_build_filename(dirname, CATALOG_TXT, NULL);
	catalogtxt = fopen(catalogtxt_name, "rb");
	if (catalogtxt == NULL)
	{
		g_free(catalogtxt_name);
		tmp = g_build_filename("$APPDATA", CATALOG_TXT, NULL);
		catalogtxt_name = path_subst(tmp);
		g_free(tmp);
		catalogtxt = fopen(catalogtxt_name, "rb");
	}
	if (catalogtxt == NULL)
	{
		g_free(catalogtxt_name);
		tmp = g_build_filename("$BINDIR", CATALOG_TXT, NULL);
		catalogtxt_name = path_subst(tmp);
		g_free(tmp);
		catalogtxt = fopen(catalogtxt_name, "rb");
	}
	if (catalogtxt == NULL)
	{
		catalogtxt_name = g_strdup(CATALOG_TXT);
		memassert(catalogtxt_name);
		catalogtxt = fopen(catalogtxt_name, "rb");
	}
	g_free(dirname);
	
	if (hyptop)
		*outfile_name = replace_ext(hyptop, NULL, HYP_EXT_STG);
	else if (config)
		*outfile_name = replace_ext(config_name, NULL, HYP_EXT_STG);
	else if (catalogtxt)
		*outfile_name = replace_ext(catalogtxt_name, NULL, HYP_EXT_STG);
	else
		*outfile_name = replace_ext(CATALOG_TXT, NULL, HYP_EXT_STG);

	if (!find_hypfiles(pathlist, hyptop))
		retval = FALSE;
	
	if (catalogtxt)
	{
		hyp_utf8_fprintf(stdout, _("reading catalog file '%s'\n"), catalogtxt_name);
		read_catalogtxt(catalogtxt);
		fclose(catalogtxt);
		catalogtxt = NULL;
	} else
	{
		hyp_utf8_fprintf(stderr, _("catalog file %s not found\n"), catalogtxt_name);
	}
	
	if (config)
	{
		hyp_utf8_fprintf(stdout, _("reading config file '%s'\n"), config_name);
		read_config(config);
		fclose(config);
		config = NULL;
	} else
	{
		hyp_utf8_fprintf(stderr, _("config file %s not found\n"), config_name);
	}
	set_default_opts();
	
	{
		int i;
		
		for (i = 0; i < STR_MAX; i++)
			if (strings[i].val == NULL)
				strings[i].val = g_strdup(_(strings[i].def));
	}
	
	if (help_page == NULL)
		help_page = g_strdup(_("\
This hypertext is, so to speak, an index of all available hypertexts.\n\
It has been created automatically by the program STOOL. The available\n\
texts are here sorted by theme into a tree. In order to find a given\n\
text here one only needs to follow the 'theme' cross-references to the\n\
text. If one knows exactly what the text is called then one can also\n\
look in the Index where all texts are listed once more in alphabetical\n\
order.\n\
\n\
@{U}Navigation in this text:@{u}\n\
Index    : Display Index-page\n\
Contents : Move up one level in the tree\n\
Page >   : One page forward in the same level\n\
Page <   : One page back in the same level\n\
\n\
@{B}Warning:@{b} Normally there are also descriptions of texts here that\n\
are generally available. Cross references to such texts will also be\n\
generated so that they can be selected if the text is installed at a\n\
later date. These texts may be recognised by the fact that the file name\n\
is greyed out. In the Index-page all the texts present at the time of\n\
creating this catalog text are shown ticked."));

	
	root = build_tree();
	if (do_shrink)
		shrink_tree(&root);
	labelnr = 0;
	make_labels(root, &labelnr);
	
#if 0
	if (root)
		dump_tree(root->down, 0);
#endif
	if (root == NULL)
		retval = FALSE;
	
	if (retval)
	{
		FILE *outfile = fopen(*outfile_name, "w");
		
		if (outfile == NULL)
		{
			hyp_utf8_fprintf(stderr, _("%s: cannot open '%s', using console instead\n"), gl_program_name, *outfile_name);
			outfile = stdout;
		} else
		{
			hyp_utf8_printf(_("writing output file '%s'\n"), *outfile_name);
		}
		
		hyp_utf8_fprintf_charset(outfile, output_charset, "@inputenc %s\n", hyp_charset_name(output_charset));

		str = stg_quote_name(strings[STR_DATABASE].val);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@database \"%s\"\n", str);
		g_free(str);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@options \"-a -i -n -s%s\"\n", do_updateref ? " +zz" : "");
		str = stg_quote_name(strings[STR_CATAUTHOR].val);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@author \"%s\"\n", str);
		g_free(str);
		tmp = replace_ext(hyp_basename(*outfile_name), HYP_EXT_STG, "");
		str = stg_quote_name(tmp);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@$VER: %s (@:\"__DATE__\")\n", str);
		g_free(str);
		g_free(tmp);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@subject \"TOP\"\n");
		
		if (do_index_only)
		{
			print_index_only(outfile, root);
		} else
		{
			print_nodes(outfile, root);
			print_helppage(outfile);
			print_index(outfile, root);
		}
		
		if (outfile)
		{
			fflush(outfile);
			if (ferror(outfile))
			{
				hyp_utf8_fprintf(stderr, _("%s: write error\n"), gl_program_name);
				retval = FALSE;
			}
			if (outfile != stdout)
			{
				fclose(outfile);
			}
		}
	}
			
	{
		int i;
		
		for (i = 0; i < STR_MAX; i++)
			g_freep(&strings[i].val);
	}
	free_cfgs(root);
	
	g_freep(&catalogtxt_name);
	g_freep(&config_name);
	g_freep(&hyptop);
	g_freep(&pathlist);
	g_freep(&help_page);
	g_freep(&linebuf);
	
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean compile_output(const char *stg_filename)
{
	char *hcp = path_subst(gl_profile.general.hcp_path);
	char *outfile_name = replace_ext(stg_filename, NULL, HYP_EXT_HYP);
	gboolean retval = TRUE;
	const char *argv[10];
	int argc;
	int retcode;
	int i;
	
	argc = 0;
	argv[argc++] = hcp;
	argv[argc++] = "-o";
	argv[argc++] = outfile_name;
	argv[argc++] = stg_filename;
	argv[argc] = NULL;
	for (i = 0; i < argc; i++)
	{
		putc(i == 0 ? '>' : ' ', stdout);
		hyp_utf8_fprintf(stdout, "%s", argv[i]);
	}
	fputs("\n", stdout);
	retcode = hyp_utf8_spawnvp(P_WAIT, argc, argv);
	if (retcode < 0)
	{
		hyp_utf8_fprintf(stderr, _("can't find %s: %s\n"), printnull(hcp), hyp_utf8_strerror(errno));
		retval = FALSE;
	} else if (retcode != 0)
	{
		hyp_utf8_fprintf(stderr, _("hcp exit code was %d\n"), retcode);
		retval = FALSE;
	} else
	{
		hyp_utf8_unlink(stg_filename);
	}
	g_free(outfile_name);
	g_free(hcp);
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static struct option const long_options[] = {
	{ "shrink", no_argument, NULL, 's' },
	{ "no-shrink", no_argument, NULL, 's' + 256 },
	{ "pullup", no_argument, NULL, 'p' },
	{ "no-pullup", no_argument, NULL, 'p' + 256 },
	{ "create", no_argument, NULL, 'C' },
	{ "no-create", no_argument, NULL, 'C' + 256 },
	{ "index", no_argument, NULL, 'i' },
	{ "no-index", no_argument, NULL, 'i' + 256 },
	{ "compile", no_argument, NULL, 'c' },
	{ "no-compile", no_argument, NULL, 'c' + 256 },
	{ "exonly", no_argument, NULL, 'e' },
	{ "no-exonly", no_argument, NULL, 'e' + 256 },
	{ "updateref", no_argument, NULL, 'r' },
	{ "no-updateref", no_argument, NULL, 'r' + 256 },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	
	{ NULL, no_argument, NULL, 0 }
};

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

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

/* ------------------------------------------------------------------------- */

static void print_usage(FILE *out)
{
	print_version(out);
	hyp_utf8_fprintf(out, _("usage: %s [+-options]\n"), gl_program_name);
	hyp_utf8_fprintf(out, _("options:\n"));
	hyp_utf8_fprintf(out, _("  +-s, --[no-]shrink            remove empty pages\n"));
	hyp_utf8_fprintf(out, _("  +-p, --[no-]pullup            move pages with one entry only up\n"));
	hyp_utf8_fprintf(out, _("  +-C, --[no-]create            create missing subjects\n"));
	hyp_utf8_fprintf(out, _("  +-i, --[no-]index             create index page only\n"));
	hyp_utf8_fprintf(out, _("  +-e, --[no-]exonly            include existing files only\n"));
	hyp_utf8_fprintf(out, _("  +-c, --[no-]compile           call hypertext compiler\n"));
	hyp_utf8_fprintf(out, _("  +-r, --[no-]updateref         update global references file\n"));
	hyp_utf8_fprintf(out, _("  -h, --help                    print help and exit\n"));
	hyp_utf8_fprintf(out, _("  -V, --version                 print version and exit\n"));
}

/* ------------------------------------------------------------------------- */

static int getopt_on_r(struct _getopt_data *d)
{
	return getopt_switch_r(d) == '+';
}

/* ------------------------------------------------------------------------- */

#include "hypmain.h"

int main(int argc, const char **argv)
{
	int retval = 0;
	struct _getopt_data *d;
	int c;
	char *outfile_name = NULL;
	
	HypProfile_Load();
	
	do_shrink = -1;
	do_pullup = -1;
	do_create = -1;
	do_index_only = -1;
	do_compile = -1;
	do_exonly = -1;
	do_updateref = -1;
	do_help = FALSE;
	do_version = FALSE;
	os = hyp_get_current_os();
	output_charset = hyp_get_current_charset();
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(argc, argv, "psCcerihV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case 's':
			do_shrink = getopt_on_r(d);
			break;
		case 's' + 256:
			do_shrink = FALSE;
			break;
		case 'p':
			do_pullup = getopt_on_r(d);
			break;
		case 'p' + 256:
			do_pullup = FALSE;
			break;
		case 'C':
			do_create = getopt_on_r(d);
			break;
		case 'C' + 256:
			do_create = FALSE;
			break;
		case 'i':
			do_index_only = getopt_on_r(d);
			break;
		case 'i' + 256:
			do_index_only = FALSE;
			break;
		case 'c':
			do_compile = getopt_on_r(d);
			break;
		case 'c' + 256:
			do_compile = FALSE;
			break;
		case 'e':
			do_exonly = getopt_on_r(d);
			break;
		case 'e' + 256:
			do_exonly = FALSE;
			break;
		case 'r':
			do_updateref = getopt_on_r(d);
			break;
		case 'r' + 256:
			do_updateref = FALSE;
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
		}
	}
	c = getopt_ind_r(d);
	getopt_finish_r(&d);
	
	if (retval == 0)
	{
		if (do_help)
			print_usage(stdout);
		else if (do_version)
			print_version(stdout);
		else if (!create_output(&outfile_name))
			retval = 1;
		else if (do_compile && !compile_output(outfile_name))
			retval = 1;
	}
			
	g_freep(&outfile_name);
	HypProfile_Delete();
	x_free_resources();
	
	(void) dump_tree;
	
	return retval;
}
