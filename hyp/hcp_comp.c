#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#include "kwset.h"

#undef ASSERT
#define ASSERT(expr) ((void)((expr) ? 0 : __my_assert(#expr, __FILE__, __LINE__)))

typedef struct _filelist FILELIST;
struct _filelist {
	FILELIST *next;
	hyp_filetype type;
	gboolean first_use;				/* for counting images during pass 2 */
	hyp_nodenr extern_node_index;
	FILE_ID id;
	FILE_LOCATION first_reference;	/* for error reporting */
	struct {
		hyp_pic_format format;
		int width;
		int height;
		int planes;
	} pic;
	char name[1];
};

typedef struct _include_file HCP_INCLUDE_FILE;
struct _include_file {
	HCP_INCLUDE_FILE *next;
	FILE_LOCATION loc;
	FILE *file;
};

typedef struct _if_stack_item IF_STACK_ITEM;
struct _if_stack_item
{
	IF_STACK_ITEM *next;
	gboolean skipping;
	int level;
	FILE_LOCATION loc;
};

typedef unsigned int LABIDX; /* beware: must fit into hyp_nodenr (unsigned short) when used as link target */

typedef enum {
	l_node,			/* label is node name */
	l_alias,		/* label is alias */
	l_label,		/* label is line label */
	l_extern,		/* label is external reference */
	l_uses,			/* label is external reference from @uses */
	l_index			/* label is index entry */
} labeltype;

typedef struct _label					/* jump labels to be referenced */
{
	LABIDX	 labindex;					/* lab[1]==1, lab[2]==2, etc. */
	labeltype type;
	hyp_nodenr node_index;				/* belongs to node "node_table[node_index]" */
	hyp_nodenr extern_index;
	gboolean	 add_to_autoref; 		/* autoref this label */
	gboolean	 add_to_index; 			/* index this label (node only!) */
	gboolean     add_to_ref;			/* add to reference file */
	gboolean	 referenced;			/* TRUE: label has been referenced */
	hyp_lineno lineno;
	FILE_LOCATION source_location;
	char		 name[1];				/* label name */
} LABEL;

typedef enum {
	HCP_DEF_DATE,
	HCP_DEF_USER
} predef_type;

typedef struct _hcp_define HCP_DEFINE;
struct _hcp_define {
	HCP_DEFINE *next;
	predef_type type;
	const char *name;
	const char *val;
};

typedef struct _xref_item XREF_ITEM;
struct _xref_item {
	XREF_ITEM *next;
	hyp_nodenr target;
	char *title;
	char name[1];
};

typedef struct _rsc_define RSC_DEFINE;
struct _rsc_define {
	RSC_DEFINE *next;
	unsigned short value;
	FILE_LOCATION loc;
	char name[1];
};

typedef struct _tree_def TREEDEF;
struct _tree_def {
	TREEDEF *next;
	unsigned short treenr;
	unsigned short objnr;
	hyp_nodenr node;
	hyp_lineno lineno;
	LABIDX lab;
	FILE_LOCATION loc;
	char name[1];
};

typedef struct _nodeitem {
	LABIDX		 labindex;				/* lab[]-Position */
	FILE_LOCATION source_location;
	gboolean	 is_popup;				/* node is a popup node */
	hyp_nodenr   node_index;
	hyp_nodenr	 prev_index;
	hyp_nodenr	 next_index;
	hyp_nodenr	 toc_index;
	int tabwidth;
	XREF_ITEM *xrefs;
	TREEDEF *objects;
	struct hyp_gfx *gfx;
	hyp_lineno minheight;
	hyp_lineno page_lines;
	char *window_title;
	char *toc_name;
	char *next_name;
	char *prev_name;
} NODEITEM;

typedef struct _hcp_vars {
	hcp_opts *opts;
	FILE *outfile;
	FILE *errorfile;
	unsigned long seek_offset;
	HYP_DOCUMENT *hyp;
	
	HCP_DEFINE *hcp_defines;
	RSC_DEFINE *rsc_defines;

	FILELIST *filelist;
	FILE_ID last_fileid;
	HCP_INCLUDE_FILE *include_stack;
	FILE_LOCATION first_loc;
	IF_STACK_ITEM *if_stack;
	LABEL **label_table;
	NODEITEM **node_table;
	INDEX_ENTRY **extern_table;

	HYP_CHARSET input_charset;
	long error_count;
	long warning_count;
	gboolean in_preamble;
	int in_node;
	gboolean in_tree;
	hyp_nodenr cur_rsc_tree_nr;
	int status_column;
	const char *pass_msg;
	int hcp_pass;

	/*
	 * counters for pass 1
	 */
	hyp_nodenr p1_node_counter;
	hyp_nodenr p1_node_alloc;
	hyp_nodenr p1_external_node_counter;
	hyp_nodenr p1_external_uses_counter;
	LABIDX p1_lab_counter, p1_lab_alloc;

	/*
	 * counters for pass 2
	 */
	hyp_nodenr p2_node_counter;
	hyp_nodenr p2_external_node_counter;
	hyp_nodenr p2_real_external_node_counter;
	hyp_nodenr external_nodes_alloced;

	/* input line buffer & size */
	char *linebuf;
	size_t linebuf_size;

	/* TRUE if AmigaGuide input file */
	gboolean for_amguide;

	/* current output line in node */
	FILE_LINENO node_lineno;

	/* text of last inputline read. only used for @$VER: command */
	char *cur_fileline;

	/* @toc name to use when node does not have one */
	char *global_toc;

	/* TRUE if some characters could not be converted */
	gboolean global_converror;

	/* current active text attributes */
	unsigned char textattr;

	/* (uncompressed) number of bytes in output page */
	size_t page_used;

	/* size of buffer allocated for page */
	size_t page_alloced;
	size_t page_max_used;

	/* encoded bytes of current page */
	unsigned char *page_buf;

	/* wether @index in popup-nodes is allowed */
	gboolean allow_index_in_pnodes;

	/* working copy of opts->nodes_to_index */
	int nodes_to_index;
	
	/* working copy of opts->autoreferences */
	int autoreferences;
	
	/* working copy of opts->tabwidth */
	int tabwidth;

	/* wether to auto-gen index page */
	gboolean gen_index;

	/*
	 * wether to add an additional empty index entry at EOF
	 * (actually unneccessary according to format spec,
	 * but ST-Guide seems to need it)
	 */
	gboolean need_eof_entry;

	/* any @xref used? */
	gboolean uses_xref;
	
	/* any centered image used? */
	gboolean uses_limage;
		
	/* collection of auto-referenced names */
	kwset_t autorefs;
	
	/* characters that are allowed before and after a auto-referenced item */
	char allowed_prev_chars[256];
	char allowed_next_chars[256];
	
	/*
	 * Statistics printed at end
	 */
	struct {
		hyp_nodenr image_count;
		hyp_nodenr internal_nodes;
		hyp_nodenr external_nodes;
		hyp_nodenr other_nodes;
		unsigned long refs_generated;
		unsigned long comp_diff;
		unsigned long autorefs;
	} stats;

} hcp_vars;


typedef void (*hcp_cmd)(hcp_vars *vars, int argc, char **argv);

struct hcp_command {
	const char *name;
	unsigned int where;
#define CMD_ALWAYS		0x0001
#define CMD_IN_PREAMBLE 0x0002
#define CMD_IN_NODE 	0x0004
#define CMD_NOT_PNODE	0x0008
	hcp_cmd pass1;
	hcp_cmd pass2;
};

/*
 * command names are case insensitive
 */
#define cmdnamecmp g_ascii_strcasecmp

/*
 * node/label names are case sensitive
 */
#define namecmp strcmp


#define C_CR 0x0d
#define C_NL 0x0a


/*
 * Nodetypes used during pass1(). Never be written out.
 */
#define HYP_NODE_XLINK     0xf0
#define HYP_NODE_XREF      0xf1
#define HYP_NODE_XLINK_RSC 0xf2

/*
 * several fields in the ENTRY are reused during pass1
 */
#define pic_node_index previous
#define pic_entry_type toc_index
#define pic_file_id    next

#define extern_labindex  toc_index
#define extern_nodeindex next

#define xref_node_index next
#define xref_index      previous

#define xlink_rsc_treenr toc_index
#define xlink_lineno     toc_index
#define xlink_linenolab  previous
#define xlink_target     previous


/*
 * Map from VDI colors to ST standard pixel values, 4 planes.
 * Only used to construct the %dithermask
 */
static unsigned int const vdi_maptab16[16] = { 0, 15, 1, 2, 4, 6, 3, 5, 7,  8,  9, 10, 12, 14, 11, 13 };

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

__attribute__((noreturn))
__attribute__((noinline))
static void oom(hcp_vars *vars)
{
	/* no hyp_utf8_xx here; it might call malloc again */
	fputs(strerror(ENOMEM), stderr);
	fputs("\n", stderr);
	vars->error_count++;
	abort();
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* ----------------------
 * handling of file names
 * ----------------------
 */

__attribute__((noinline))
static FILELIST *file_lookup(hcp_vars *vars, FILE_ID id)
{
	FILELIST *f;

	if (id != 0)
	{
		for (f = vars->filelist; f != NULL; f = f->next)
			if (f->id == id)
			{
				return f;
			}
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */

__attribute__((noinline))
static const char *file_lookup_name(hcp_vars *vars, FILE_ID id)
{
	FILELIST *f;

	f = file_lookup(vars, id);
	if (f)
		return f->name;
	return _("<no filename>");
}

/* ------------------------------------------------------------------------- */

static FILELIST *file_listadd(hcp_vars *vars, const char *name, hyp_filetype *type, gboolean *is_new)
{
	FILELIST *f;
	size_t len;
	
	*is_new = FALSE;
	if (empty(name))
	{
		return NULL;
	}
	
	/*
	 * should actually normalize whole pathname.
	 * For now, at least skip leading ".". FIXME
	 */
	while (*name == '.' && G_IS_DIR_SEPARATOR(name[1]))
		name += 2;
	
	for (f = vars->filelist; f != NULL; f = f->next)
		if (hyp_utf8_strcasecmp(f->name, name) == 0)
		{
			*type = f->type;
			return f;
		}
	len = strlen(name);
	f = (FILELIST *)g_malloc(sizeof(*f) + len);
	if (G_UNLIKELY(f == NULL))
	{
		oom(vars);
		return NULL;
	}
	++vars->last_fileid;
	f->id = vars->last_fileid;
	f->type = *type;
	f->first_use = TRUE;
	if (vars->include_stack)
	{
		f->first_reference = vars->include_stack->loc;
	} else
	{
		f->first_reference.id = 0;
		f->first_reference.lineno = 0;
	}
	f->pic.format = HYP_PIC_UNKNOWN;
	f->pic.width = 0;
	f->pic.height = 0;
	f->pic.planes = 0;
	strcpy(f->name, name);
	f->next = vars->filelist;
	vars->filelist = f;
	*is_new = TRUE;
	return f;
}

/* ------------------------------------------------------------------------- */

static FILE_ID file_listadd_id(hcp_vars *vars, const char *name, hyp_filetype *type, gboolean *is_new)
{
	FILELIST *f;
	f = file_listadd(vars, name, type, is_new);
	return f ? f->id : 0;
}

/* ------------------------------------------------------------------------- */

static char *find_include_file(hcp_vars *vars, const char *filename)
{
	struct stat st;
	char *dir;
	char *include_dir;
	char *path;
	HCP_INCLUDE_FILE *includer;
	
	/*
	 * try name as passed
	 */
	if (hyp_utf8_stat(filename, &st) == 0)
		return g_strdup(filename);
	/*
	 * no further trys if absolute name
	 */
	if (g_path_is_absolute(filename))
		return g_strdup(filename);
	
	/*
	 * try in directory of current file
	 */
	dir = hyp_path_get_dirname(filename);
	if (empty(dir))
	{
		g_free(dir);
		dir = g_strdup(".");
	}
	includer = vars->include_stack;
	include_dir = hyp_path_get_dirname(file_lookup_name(vars, includer->loc.id));
	if (empty(include_dir))
	{
		g_free(include_dir);
		include_dir = g_strdup(".");
	}
	path = g_build_filename(include_dir, dir, hyp_basename(filename), NULL);
	g_free(include_dir);
	if (hyp_utf8_stat(path, &st) == 0)
	{
		g_free(dir);
		return path;
	}
	g_free(path);
	
	/*
	 * try in directory of main file
	 */
	while (includer->next != NULL)
		includer = includer->next;
	include_dir = hyp_path_get_dirname(file_lookup_name(vars, includer->loc.id));
	if (empty(include_dir))
	{
		g_free(include_dir);
		include_dir = g_strdup(".");
	}
	path = g_build_filename(include_dir, dir, hyp_basename(filename), NULL);
	g_free(include_dir);
	g_free(dir);
	if (hyp_utf8_stat(path, &st) == 0)
	{
		return path;
	}
	
	g_free(path);
	return g_strdup(filename);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* ---------------------------
 * handling of progress status
 * ---------------------------
 */

static void flush_status_output(hcp_vars *vars)
{
	if (vars->status_column != 0)
	{
		fputs("\n", stdout);
		vars->status_column = 0;
	}
}

/* ------------------------------------------------------------------------- */

static void hcp_status_msg(hcp_vars *vars, const char *msg)
{
	int pass_len;
	int i;
	int msglen;
	
	msglen = (int) g_utf8_str_len(msg, STR0TERM);
	pass_len = vars->pass_msg ? (int) g_utf8_str_len(vars->pass_msg, STR0TERM) : 0;
	for (i = pass_len; i < vars->status_column; i++)
	{
		fputs("\b \b", stdout);
	}
	if (pass_len != 0 && vars->status_column == 0)
		hyp_utf8_fprintf(stdout, "%s", vars->pass_msg);
	hyp_utf8_fprintf(stdout, "%s", msg);
	vars->status_column = pass_len + msglen;
	fflush(stdout);
}

/* ------------------------------------------------------------------------- */

static void hcp_status_file(hcp_vars *vars, const char *filename)
{
	if (vars->opts->verbose < (3 - vars->hcp_pass))
		return;
	hcp_status_msg(vars, filename);
}

/* ------------------------------------------------------------------------- */

static void hcp_status_node(hcp_vars *vars, const char *nodename)
{
	if (vars->opts->verbose < (3 - vars->hcp_pass))
		return;
	hcp_status_msg(vars, nodename);
}

/* ------------------------------------------------------------------------- */

static void hcp_status_image(hcp_vars *vars, const char *filename, unsigned int left)
{
	char *msg;
	
	if (vars->opts->verbose < 1)
		return;
	if (left)
		msg = g_strdup_printf(_("%s, %u left"), filename, left);
	else
		msg = g_strdup_printf(_("%s, none left"), filename);
	hcp_status_msg(vars, msg);
	g_free(msg);
}

/* ------------------------------------------------------------------------- */

static void hcp_status_pass(hcp_vars *vars, const char *msg)
{
	flush_status_output(vars);
	vars->pass_msg = msg;
	hyp_utf8_fprintf(stdout, "%s", msg);
	fflush(stdout);
	vars->status_column = (int)g_utf8_str_len(msg, STR0TERM);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------
 * Errors & warnings
 * -----------------
 */

static void hcp_error_va(hcp_vars *vars, const FILE_LOCATION *loc, const char *format, va_list args)
{
	if (vars->errorfile == stderr)
		flush_status_output(vars);
	hyp_utf8_fprintf(vars->errorfile, _("error: "));
	if (loc == NULL && vars->include_stack != NULL)
		loc = &vars->include_stack->loc;
	if (loc != NULL)
	{
		if (loc->id == 0 && loc->lineno == 0)
			hyp_utf8_fprintf(vars->errorfile, "%s: ", _("<command-line>"));
		else
			hyp_utf8_fprintf(vars->errorfile, "%s:%lu: ", file_lookup_name(vars, loc->id), loc->lineno);
	}
	hyp_utf8_vfprintf(vars->errorfile, format, args);
	fputs("\n", vars->errorfile);
	vars->error_count++;
}

/* ------------------------------------------------------------------------- */

__attribute__((format(printf, 3, 4)))
static void hcp_error(hcp_vars *vars, const FILE_LOCATION *loc, const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	hcp_error_va(vars, loc, format, args);
	va_end(args);
}

/* ------------------------------------------------------------------------- */

static void hcp_warning_va(hcp_vars *vars, const FILE_LOCATION *loc, const char *format, va_list args)
{
	if (vars->errorfile == stderr)
		flush_status_output(vars);
	fprintf(vars->errorfile, _("warning: "));
	if (loc == NULL && vars->include_stack != NULL)
		loc = &vars->include_stack->loc;
	if (loc != NULL)
	{
		if (loc->id == 0 && loc->lineno == 0)
			fprintf(vars->errorfile, "%s: ", _("<command-line>"));
		else
			fprintf(vars->errorfile, "%s:%lu: ", file_lookup_name(vars, loc->id), loc->lineno);
	}
	vfprintf(vars->errorfile, format, args);
	fputs("\n", vars->errorfile);
	vars->warning_count++;
}

/* ------------------------------------------------------------------------- */

__attribute__((format(printf, 3, 4)))
static void hcp_warning(hcp_vars *vars, const FILE_LOCATION *loc, const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	hcp_warning_va(vars, loc, format, args);
	va_end(args);
}

/* ------------------------------------------------------------------------- */

static void error_missing_args(hcp_vars *vars)
{
	if (vars->hcp_pass == 1)
		hcp_error(vars, NULL, _("missing argument"));
}

/* ------------------------------------------------------------------------- */

static void error_outfile(hcp_vars *vars)
{
	hcp_error(vars, NULL, _("error writing output file"));
}

/* ------------------------------------------------------------------------- */

static void warn_extra_args(hcp_vars *vars)
{
	if (vars->hcp_pass == 1)
		hcp_warning(vars, NULL, _("extra arguments ignored"));
}

/* ------------------------------------------------------------------------- */

static void warn_converror(hcp_vars *vars, const FILE_LOCATION *loc, int pass)
{
	if (vars->global_converror)
	{
		if (pass == 0 || pass == vars->hcp_pass)
			hcp_warning(vars, loc, _("Some characters could not be converted"));
		vars->global_converror = FALSE;
	}
}

/* ------------------------------------------------------------------------- */

static void empty_arg(hcp_vars *vars, const char *what, gboolean is_error)
{
	if (vars->hcp_pass == 1)
	{
		if (is_error)
			hcp_error(vars, NULL, _("empty %s"), what);
		else
			hcp_warning(vars, NULL, _("empty %s"), what);
	}
}

/* ------------------------------------------------------------------------- */

static void warn_duplicate_arg(hcp_vars *vars, const char *what)
{
	if (vars->hcp_pass == 1)
		hcp_warning(vars, NULL, _("duplicate %s"), what);
}

/* ------------------------------------------------------------------------- */

/*
 * for strings that are still in utf-8 in memory,
 * but are written in target encoding to file
 */
static size_t target_strlen(hcp_vars *vars, const char *name)
{
	return vars->hyp->comp_charset == HYP_CHARSET_UTF8 ? strlen(name) : g_utf8_str_len(name, STR0TERM);
}

/* ------------------------------------------------------------------------- */

#if 0
static size_t target_strnlen(const char *name, size_t len)
{
	return hyp->comp_charset == HYP_CHARSET_UTF8 ? len : g_utf8_str_len(name, len);
}
#endif

/* ------------------------------------------------------------------------- */

static gboolean check_namelen(hcp_vars *vars, const char *what, const char *name, gboolean is_error)
{
	if (empty(name))
	{
		empty_arg(vars, what, is_error);
		return !is_error;
	} else if (target_strlen(vars, name) > HYP_NODENAME_MAX)
	{
		if (vars->hcp_pass == 1)
			hcp_warning(vars, NULL, _("%s longer than %d chars, truncated"), what, HYP_NODENAME_MAX);
	}
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* ----------------------
 * Stack of include files
 * ----------------------
 */

static gboolean push_file_stack(hcp_vars *vars, const char *filename, FILE *file)
{
	HCP_INCLUDE_FILE *inc;
	gboolean is_new;
	hyp_filetype type;
	
	inc = g_new(HCP_INCLUDE_FILE, 1);
	if (G_UNLIKELY(inc == NULL))
	{
		oom(vars);
		return FALSE;
	}
	inc->loc.lineno = 0;
	type = HYP_FT_STG;
	inc->loc.id = file_listadd_id(vars, filename, &type, &is_new);
	inc->file = file;
	inc->next = vars->include_stack;
	vars->include_stack = inc;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean pop_file_stack(hcp_vars *vars)
{
	HCP_INCLUDE_FILE *inc, *next;
	
	if ((inc = vars->include_stack) != NULL)
	{
		fclose(inc->file);
		next = inc->next;
		vars->include_stack = next;
		g_free(inc);
		return TRUE;
	}
	return FALSE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* --------------------------------
 * Stack of conditional compilation
 * --------------------------------
 */

static gboolean toggle_if_stack(hcp_vars *vars)
{
	if (vars->if_stack != NULL)
	{
		vars->if_stack->skipping = !(vars->if_stack->skipping);
		return TRUE;
	}
	hcp_error(vars, NULL, _("@else without @if"));
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static gboolean pop_if_stack(hcp_vars *vars)
{
	IF_STACK_ITEM *s;
	
	if ((s = vars->if_stack) != NULL)
	{
		vars->if_stack = s->next;
		g_free(s);
		return TRUE;
	}
	hcp_error(vars, NULL, _("@endif without @if"));
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static gboolean push_if_stack(hcp_vars *vars, gboolean skipping)
{
	IF_STACK_ITEM *s;
	
	s = g_new(IF_STACK_ITEM, 1);
	if (G_UNLIKELY(s == NULL))
	{
		oom(vars);
		return FALSE;
	}
	s->level = vars->if_stack ? vars->if_stack->level + 1 : 1;
	s->skipping = skipping;
	s->loc = vars->include_stack->loc;
	s->next = vars->if_stack;
	vars->if_stack = s;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean is_if_stack_skipping(hcp_vars *vars)
{
	IF_STACK_ITEM *s;
	
	for (s = vars->if_stack; s != NULL; s = s->next)
	{
		if (s->skipping)
			return TRUE;
	}
	return FALSE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* --------------------------------
 * File input
 * --------------------------------
 */

static char *readline(hcp_vars *vars, FILE *fp)
{
	size_t sl;
	int c;
	
	sl = 0;
	++vars->include_stack->loc.lineno;
	for (;;)
	{
		c = getc(fp);

		if (c == EOF)
		{
			if (sl == 0)
				return NULL;
			break;
		}
		if (c == C_CR)
		{
			c = getc(fp);
			if (c != C_NL && c != EOF)
				ungetc(c, fp);
			break;
		}
		if (c == C_NL)
			break;
		if (sl >= vars->linebuf_size)
		{
			vars->linebuf_size += 1020;
			vars->linebuf = g_renew(char, vars->linebuf, vars->linebuf_size + 1);
			if (G_UNLIKELY(vars->linebuf == NULL))
			{
				oom(vars);
				return NULL;
			}
		}
		vars->linebuf[sl++] = c;
	}
	if (sl >= vars->linebuf_size)
	{
		vars->linebuf_size += 1020;
		vars->linebuf = g_renew(char, vars->linebuf, vars->linebuf_size + 1);
		if (G_UNLIKELY(vars->linebuf == NULL))
		{
			oom(vars);
			return NULL;
		}
	}
	vars->linebuf[sl] = '\0';
	return hyp_conv_to_utf8(vars->input_charset, vars->linebuf, STR0TERM);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Helper functions for labels & nodes
 * -----------------------------------
 */

static LABEL *add_label(hcp_vars *vars, const char *name, labeltype type)
{
	LABEL *lab;
	size_t len;
	
	if (G_UNLIKELY(name == NULL))
		return NULL;
	if (vars->p1_lab_counter >= vars->p1_lab_alloc)
	{
		LABIDX new_size = vars->p1_lab_alloc + 1024;
		
		vars->label_table = g_renew(LABEL *, vars->label_table, new_size);
		if (vars->label_table == NULL)
		{
			oom(vars);
			return NULL;
		}
		vars->p1_lab_alloc = new_size;
	}
	len = strlen(name);
	lab = (LABEL *)g_malloc0(sizeof(LABEL) + len + 1); /* +1 here for aligning the entries (entry already has 1 byte for terminating '\0' */
	if (G_UNLIKELY(lab == NULL))
	{
		oom(vars);
		return NULL;
	}
	strcpy(lab->name, name);
	lab->node_index = vars->p1_node_counter;
	lab->lineno = vars->node_lineno;
	lab->source_location = vars->include_stack->loc;
	lab->labindex = vars->p1_lab_counter;
	lab->type = type;
	vars->label_table[vars->p1_lab_counter] = lab;
	vars->p1_lab_counter++;
	return lab;
}

/* ------------------------------------------------------------------------- */

static LABEL *add_alias(hcp_vars *vars, const char *name)
{
	LABEL *lab;

	lab = add_label(vars, name, l_alias);
	if (G_UNLIKELY(lab == NULL))
		return NULL;
	lab->add_to_autoref = TRUE;
	lab->add_to_ref = TRUE;
	lab->add_to_index = vars->opts->alias_to_index;
	return lab;
}

/* ------------------------------------------------------------------------- */

static LABEL *find_label(hcp_vars *vars, const char *name, labeltype type)
{
	LABIDX i;
	LABEL *lab;
	
	for (i = 0; i < vars->p1_lab_counter; i++)
	{
		lab = vars->label_table[i];
		if (type == lab->type && (type != l_label || lab->lineno != HYP_NOINDEX) && namecmp(lab->name, name) == 0)
			return lab;
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */

static LABEL *find_node_label(hcp_vars *vars, const char *name, hyp_nodenr node)
{
	LABIDX i;
	LABEL *lab;
	
	for (i = 0; i < vars->p1_lab_counter; i++)
	{
		lab = vars->label_table[i];
		if (lab->type == l_label && lab->lineno != HYP_NOINDEX && lab->node_index == node && namecmp(lab->name, name) == 0)
			return lab;
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */

static LABEL *add_index(hcp_vars *vars, const char *name)
{
	LABEL *lab = add_label(vars, name, l_index);
	if (G_UNLIKELY(lab == NULL))
		return lab;
	lab->add_to_index = TRUE;
	return lab;
}

/* ------------------------------------------------------------------------- */

static NODEITEM *add_node(hcp_vars *vars, const char *name)
{
	LABEL *lab;
	NODEITEM *node;
	
	if ((vars->p1_node_counter + vars->p1_external_node_counter) >= HYP_NODE_MAX)
	{
		hcp_error(vars, NULL, _("too many nodes (max. %lu)"), (unsigned long)HYP_NODE_MAX);
		return NULL;
	}
	
	lab = add_label(vars, name, l_node);
	if (G_UNLIKELY(lab == NULL))
		return NULL;
	if (vars->p1_node_counter >= vars->p1_node_alloc)
	{
		size_t new_size = vars->p1_node_alloc + 1024;
		if (new_size > HYP_NODE_MAX)
			new_size = HYP_NODE_MAX;
		
		vars->node_table = g_renew(NODEITEM *, vars->node_table, new_size);
		if (vars->node_table == NULL)
		{
			oom(vars);
			return NULL;
		}
		vars->p1_node_alloc = (hyp_nodenr)new_size;
	}
	node = g_new0(NODEITEM, 1);
	if (G_UNLIKELY(node == NULL))
	{
		oom(vars);
		return NULL;
	}
	node->labindex = lab->labindex;
	node->source_location = vars->include_stack->loc;
	node->node_index = vars->p1_node_counter;
	node->prev_index = HYP_NOINDEX;
	node->next_index = HYP_NOINDEX;
	node->toc_index = HYP_NOINDEX;
	node->toc_name = g_strdup(vars->global_toc);
	node->tabwidth = vars->tabwidth;
	vars->node_table[vars->p1_node_counter] = node;
	/* p1_node_counter will be incremented by endnode */
	return node;
}

/* ------------------------------------------------------------------------- */

static INDEX_ENTRY *alloc_index(hcp_vars *vars, const char *name)
{
	INDEX_ENTRY *entry;
	size_t len;

	len = strlen(name);
	entry = (INDEX_ENTRY *)g_malloc0(sizeof(INDEX_ENTRY) + len + 1); /* +1 here for aligning the entries (entry already has 1 byte for terminating '\0' */
	if (G_UNLIKELY(entry == NULL))
	{
		oom(vars);
		return NULL;
	}
	strcpy((char *)entry->name, name);
	if (len & 1)
	{
		entry->name[len++] = '\0';
	}
	entry->length = (unsigned char)(len + SIZEOF_INDEX_ENTRY);
	return entry;
}

static INDEX_ENTRY *add_external_node(hcp_vars *vars, hyp_indextype type, const char *name)
{
	INDEX_ENTRY *entry;
	
	if ((vars->p1_node_counter + vars->p1_external_node_counter) >= HYP_NODE_MAX)
	{
		hcp_error(vars, NULL, _("too many nodes (max. %lu)"), (unsigned long)HYP_NODE_MAX);
		return NULL;
	}
	
	if (vars->p1_external_node_counter >= vars->external_nodes_alloced)
	{
		size_t new_size = vars->external_nodes_alloced + 1024;
		if (new_size > HYP_NODE_MAX)
			new_size = HYP_NODE_MAX;
		
		vars->extern_table = g_renew(INDEX_ENTRY *, vars->extern_table, new_size);
		if (vars->extern_table == NULL)
		{
			oom(vars);
			return NULL;
		}
		vars->external_nodes_alloced = (hyp_nodenr)new_size;
	}
	entry = alloc_index(vars, name);
	if (entry)
	{
		entry->type = type;
		/* other fields filled in finish_pass1 */
		vars->extern_table[vars->p1_external_node_counter] = entry;
		vars->p1_external_node_counter++;
	}
	return entry;
}

/* ------------------------------------------------------------------------- */

static NODEITEM *find_node(hcp_vars *vars, const char *name)
{
	hyp_nodenr i;
	LABEL *lab;
	
	for (i = 0; i < vars->p1_lab_counter; i++)
	{
		lab = vars->label_table[i];
		if ((lab->type == l_node || lab->type == l_alias) && namecmp(name, lab->name) == 0)
			return vars->node_table[lab->node_index];
	}
	return NULL;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Cleanup
 * -----------------------------------
 */

static void hcp_comp_exit(hcp_vars *vars)
{
	while (pop_file_stack(vars))
		;
	{
		FILELIST *f, *next;
		
		for (f = vars->filelist; f != NULL; f = next)
		{
			next = f->next;
			g_free(f);
		}
	}
	vars->filelist = NULL;
	vars->last_fileid = 0;
	g_freep(&vars->linebuf);
	vars->linebuf_size = 0;
	while (vars->if_stack != NULL)
		pop_if_stack(vars);
	
	{
		HCP_DEFINE *d, *next;

		for (d = vars->hcp_defines; d != NULL; d = next)
		{
			next = d->next;
			g_free(d);
		}
		vars->hcp_defines = NULL;
	}
	
	{
		RSC_DEFINE *d, *next;

		for (d = vars->rsc_defines; d != NULL; d = next)
		{
			next = d->next;
			g_free(d);
		}
		vars->rsc_defines = NULL;
	}
	
	if (vars->hyp)
	{
		vars->hyp->file = NULL;
		/*
		 * don't let hyp_delete free the entries from the extern_table
		 */
		if (vars->hyp->num_index > 0)
		{
			g_free(vars->hyp->indextable[vars->hyp->num_index]);
			vars->hyp->indextable[vars->hyp->num_index] = NULL;
			vars->hyp->num_index = vars->p1_node_counter;
			vars->hyp->indextable[vars->hyp->num_index] = NULL;
		}
		hyp_unref(vars->hyp);
		vars->hyp = NULL;
	}
	
	if (vars->node_table != NULL)
	{
		hyp_nodenr i;
		NODEITEM *node;
		
		for (i = 0; i < vars->p1_node_counter; i++)
		{
			node = vars->node_table[i];
			g_free(node->window_title);
			mem_garbage_clear(node->window_title);
			g_free(node->toc_name);
			mem_garbage_clear(node->toc_name);
			g_free(node->next_name);
			mem_garbage_clear(node->prev_name);
			g_free(node->prev_name);
			mem_garbage_clear(node->next_name);
			{
				XREF_ITEM *x, *next;
				
				for (x = node->xrefs; x != NULL; x = next)
				{
					next = x->next;
					g_free(x->title);
					mem_garbage_clear(x->title);
					g_free(x);
				}
				mem_garbage_clear(node->xrefs);
			}
			{
				TREEDEF *tree, *next;
				
				for (tree = node->objects; tree != NULL; tree = next)
				{
					next = tree->next;
					g_free(tree);
				}
				mem_garbage_clear(node->objects);
			}
			{
				struct hyp_gfx *a, *next;
				
				for (a = node->gfx; a != NULL; a = next)
				{
					next = a->next;
					g_free(a);
				}
				mem_garbage_clear(node->gfx);
			}
			g_free(node);
			mem_garbage_clear(vars->node_table[i]);
		}
		g_free(vars->node_table);
		vars->node_table = NULL;
	}
	vars->p1_node_counter = vars->p2_node_counter = vars->p1_node_alloc = 0;
	g_freep(&vars->global_toc);
	
	if (vars->label_table != NULL)
	{
		LABIDX i;
		LABEL *lab;
		
		for (i = 0; i < vars->p1_lab_counter; i++)
		{
			lab = vars->label_table[i];
			g_free(lab);
			mem_garbage_clear(vars->label_table[i]);
		}
		g_free(vars->label_table);
		vars->label_table = NULL;
	}
	vars->p1_lab_counter = vars->p1_lab_alloc = 0;
	
	if (vars->extern_table != NULL)
	{
		hyp_nodenr i;
		
		for (i = 0; i < vars->p1_external_node_counter; i++)
		{
			g_free(vars->extern_table[i]);
			mem_garbage_clear(vars->extern_table[i]);
		}
		g_free(vars->extern_table);
	}
	vars->extern_table = NULL;
	vars->p1_external_node_counter = 0;
	vars->p1_external_uses_counter = 0;
	vars->p2_external_node_counter = 0;
	vars->external_nodes_alloced = 0;
	
	if (vars->outfile != NULL)
	{
		fflush(vars->outfile);
		if (vars->outfile != stdout)
			fclose(vars->outfile);
		vars->outfile = NULL;
	}
	
	if (vars->page_buf)
	{
		g_free(vars->page_buf);
		vars->page_buf = NULL;
	}
	vars->page_used = vars->page_alloced = vars->page_max_used = 0;
	
	kwsfree(vars->autorefs);
	vars->autorefs = NULL;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------------------
 * Simple, very limited parser for C include files
 * -----------------------------------------------
 */

typedef enum {
	T_EOF = 0,
	T_EXCLAMATION = '!',
	T_DOLLAR = '$',
	T_CPP = '#',
	T_MOD = '%',
	T_AND = '&',
	T_LPAREN = '(',
	T_RPAREN = ')',
	T_MUL = '*',
	T_PLUS = '+',
	T_COMMA = ',',
	T_MINUS = '-',
	T_PERIOD = '.',
	T_DIV = '/',
	T_COLON = ':',
	T_SEMI = ';',
	T_LESS = '<',
	T_EQUAL = '=',
	T_GREATER = '>',
	T_QUEST = '?',
	T_AT = '@',
	T_LBRACKET = '[',
	T_BACKSLASH = '\\',
	T_RBRACKET = ']',
	T_LBRACE = '{',
	T_OR = '|',
	T_RBRACE = '}',
	T_TILDE = '~',
	T_NUMCONST = 256,
	T_CHARCONST,
	T_STRCONST,
	T_ID,
	T_INVALID = T_EOF
} TOKEN;

typedef struct _cinput {
	const unsigned char *buf;
	size_t dataoffset;
	size_t filesize;
	FILE_LOCATION loc;
} C_INPUT;

#define C_VALLEN 128

#define put_numconst(val) \
	buf[0] = (val >> 24) & 0xff; \
	buf[1] = (val >> 16) & 0xff; \
	buf[2] = (val >>  8) & 0xff; \
	buf[3] = (val      ) & 0xff; \
	buf += 4
#define get_numconst(buf) \
	((((unsigned long)(buf[0])) << 24) | \
	 (((unsigned long)(buf[1])) << 16) | \
	 (((unsigned long)(buf[2])) <<  8) | \
	 (((unsigned long)(buf[3]))      ))

/* ------------------------------------------------------------------------- */

static int c_escchar(int c)
{
	switch (c)
	{
		case 'n': return C_NL;
		case 'r': return C_CR;
		case 'f': return '\f';
		case 'v': return '\v';
		case 't': return '\t';
		case 'a': return '\007';
		case 'e': return '\033';
		case '"': return '"';
		case '\'': return '\'';
		case '\\': return '\\';
	}		
	return c;
}

/* ------------------------------------------------------------------------- */

static int c_parse_getc(C_INPUT *in)
{
	int c;
	
	if (in->dataoffset >= in->filesize)
		return EOF;
	c = in->buf[in->dataoffset++];
	if (c == C_NL)
	{
		in->loc.lineno++;
		c = '\n';
	} else if (c == C_CR)
	{
		in->loc.lineno++;
		c = '\n';
		if (in->dataoffset < in->filesize && in->buf[in->dataoffset] == C_NL)
			in->dataoffset++;
	}
	return c;
}

/* ------------------------------------------------------------------------- */

static void c_parse_ungetc(C_INPUT *in)
{
	if (in->dataoffset > 0)
	{
		--in->dataoffset;
		if (in->buf[in->dataoffset] == C_NL || in->buf[in->dataoffset] == C_CR)
			in->loc.lineno--;
	}
}

/* ------------------------------------------------------------------------- */

static TOKEN c_parse_gettok(C_INPUT *in, unsigned char *buf)
{
	int c;
	TOKEN toktype;
	int len;
	
again:;
	while ((c = c_parse_getc(in)) == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f')
		;
	
	if (c == '/')
	{
		if ((c = c_parse_getc(in)) == '*')
		{
			for (;;)
			{
				while ((c = c_parse_getc(in)) != EOF && c != '*')
					;
				if (c == EOF)
					return T_EOF;
				if ((c = c_parse_getc(in)) == '/')
					goto again;
				if (c == EOF)
					return T_EOF;
				c_parse_ungetc(in);
			}
		} else
		{
			*buf++ = '/';
			toktype = T_DIV;
		}
	} else if (isalpha(c) || c == '_')
	{
		*buf++ = (unsigned char)c;
		len = 1;
		while (isalnum(c = c_parse_getc(in)) || c == '_')
		{
			if (len < C_VALLEN)
			{
				*buf++ = (unsigned char)c;
				len++;
			}
		}
		toktype = T_ID;
	} else if (isdigit(c))
	{
		const char *valid = "0123456789ABCDEF";
		const char *pos;
		unsigned long base;
		unsigned long val;
		
		base = 10;
		val = 0;
		if (c == '0')
		{
			c = c_parse_getc(in);
			if (toupper(c) == 'X')
			{
				base = 16;
			} else
			{
				base = 8;
				if (c != EOF)
					c_parse_ungetc(in);
			}
		} else
		{
			val = c - '0';
		}
		while ((c = c_parse_getc(in)) != EOF)
		{
			if ((pos = strchr(valid, toupper(c))) == NULL ||
				(unsigned int)(pos - valid) >= base)
				break;
			val = val * base + (unsigned int)(pos - valid);
		}
		put_numconst(val);
		toktype = T_NUMCONST;
	} else if (c == '\'')
	{
		if ((c = (c_parse_getc(in))) == '\\')
			c = c_escchar(c_parse_getc(in));
		*buf++ = (unsigned char)c;
		while ((c = c_parse_getc(in)) != EOF && c != '\'')
			;
		c = EOF;
		toktype = T_CHARCONST;
	} else if (c == '"')
	{
		len = 0;
		while ((c = c_parse_getc(in)) != EOF)
		{
			if (c == '\\')
				c = c_escchar(c_parse_getc(in));
			else if (c == '"')
				break;
			if (len < C_VALLEN)
			{
				*buf++ = (unsigned char)c;
				len++;
			}
		}
		c = EOF;
		toktype = T_STRCONST;
	} else
	{
		*buf++ = (unsigned char)c;
		switch (c)
		{
			case '!': toktype = T_EXCLAMATION; break;
			case '$': toktype = T_DOLLAR; break;
			case '#': toktype = T_CPP; break;
			case '%': toktype = T_MOD; break;
			case '&': toktype = T_AND; break;
			case '(': toktype = T_LPAREN; break;
			case ')': toktype = T_RPAREN; break;
			case '*': toktype = T_MUL; break;
			case '+': toktype = T_PLUS; break;
			case ',': toktype = T_COMMA; break;
			case '-': toktype = T_MINUS; break;
			case '.': toktype = T_PERIOD; break;
			case '/': toktype = T_DIV; break;
			case ':': toktype = T_COLON; break;
			case ';': toktype = T_SEMI; break;
			case '<': toktype = T_LESS; break;
			case '=': toktype = T_EQUAL; break;
			case '>': toktype = T_GREATER; break;
			case '?': toktype = T_QUEST; break;
			case '@': toktype = T_AT; break;
			case '[': toktype = T_LBRACKET; break;
			case '\\': toktype = T_BACKSLASH; break;
			case ']': toktype = T_RBRACKET; break;
			case '{': toktype = T_LBRACE; break;
			case '|': toktype = T_OR; break;
			case '}': toktype = T_RBRACE; break;
			case '~': toktype = T_TILDE; break;
			case EOF: toktype = T_EOF; break;
			default: toktype = T_INVALID; break;
		}
		c = EOF;
	}
	*buf = '\0';
	if (c != EOF)
		c_parse_ungetc(in);
	return toktype;
}

/* ------------------------------------------------------------------------- */

__attribute__((noinline))
static RSC_DEFINE *find_rsc_define(hcp_vars *vars, const char *name)
{
	RSC_DEFINE *d;
	
	for (d = vars->rsc_defines; d != NULL; d = d->next)
		if (strcmp(name, d->name) == 0)
			return d;
	return NULL;
}

/* ------------------------------------------------------------------------- */

static gboolean c_include_scan(hcp_vars *vars, C_INPUT *in)
{
	TOKEN tok;
	unsigned char buf[C_VALLEN + 1];
	unsigned char buf2[C_VALLEN + 1];
	unsigned short value;
	RSC_DEFINE *define;
	size_t len;
	int c;
	
	while ((tok = c_parse_gettok(in, buf)) != T_EOF)
	{
		switch (tok)
		{
		case T_CPP:
			if (c_parse_gettok(in, buf) == T_ID &&
				strcmp((char *) buf, "define") == 0 &&
				c_parse_gettok(in, buf) == T_ID &&
				c_parse_gettok(in, buf2) == T_NUMCONST)
			{
				value = (unsigned short) get_numconst(buf2);
				define = find_rsc_define(vars, (const char *)buf);
				if (define != NULL)
				{
					if (define->value != value && vars->hcp_pass == 1)
						hcp_warning(vars, &in->loc, _("%s redefined"), define->name);
					define->value = value;
				} else
				{
					len = ustrlen(buf);
					define = (RSC_DEFINE *)g_malloc(sizeof(RSC_DEFINE) + len);
					if (define == NULL)
					{
						oom(vars);
						return FALSE;
					}
					strcpy(define->name, (char *)buf);
					define->value = value;
					define->next = vars->rsc_defines;
					vars->rsc_defines = define;
				}
				define->loc = in->loc;
			}
			c = c_parse_getc(in);
			while (c != '\n' && c != EOF)
			{
				if (c == '\\')
				{
					c = c_parse_getc(in);
					if (c == '\n')
						c = c_parse_getc(in);
					else if (c != EOF)
						c_parse_ungetc(in);
				} else
				{
					c = c_parse_getc(in);
				}
			}
			break;
		case T_EOF:
		case T_EXCLAMATION:
		case T_DOLLAR:
		case T_MOD:
		case T_AND:
		case T_LPAREN:
		case T_RPAREN:
		case T_MUL:
		case T_PLUS:
		case T_COMMA:
		case T_MINUS:
		case T_PERIOD:
		case T_DIV:
		case T_COLON:
		case T_SEMI:
		case T_LESS:
		case T_EQUAL:
		case T_GREATER:
		case T_QUEST:
		case T_AT:
		case T_LBRACKET:
		case T_BACKSLASH:
		case T_RBRACKET:
		case T_LBRACE:
		case T_OR:
		case T_RBRACE:
		case T_TILDE:
		case T_NUMCONST:
		case T_CHARCONST:
		case T_STRCONST:
		case T_ID:
		default:
			break;
		}
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean read_c_include(hcp_vars *vars, const char *filename)
{
	C_INPUT in;
	FILE *infile;
	unsigned char *buf;
	size_t ret;
	gboolean retvalue;
	hyp_filetype type;
	gboolean is_new;
	
	infile = hyp_utf8_fopen(filename, "rb");
	if (infile == NULL)
	{
		hcp_error(vars, NULL, "%s: %s", filename, hyp_utf8_strerror(errno));
		return FALSE;
	}
	hcp_status_file(vars, filename);
	
	type = HYP_FT_CHEADER;
	in.loc.id = file_listadd_id(vars, filename, &type, &is_new);
	in.loc.lineno = 1;
	
	fseek(infile, 0, SEEK_END);
	in.filesize = ftell(infile);
	fseek(infile, 0, SEEK_SET);
	if (in.filesize == 0)
	{
		hyp_utf8_fclose(infile);
		return TRUE;
	}
	buf = g_new(unsigned char, in.filesize);
	if (buf == NULL)
	{
		oom(vars);
		hyp_utf8_fclose(infile);
		return FALSE;
	}
	ret = fread(buf, 1, in.filesize, infile);
	hyp_utf8_fclose(infile);
	if (ret != in.filesize)
	{
		hcp_error(vars, NULL, _("%s: read error"), filename);
		g_free(buf);
		return FALSE;
	}
	in.buf = buf;
	in.dataoffset = 0;
	retvalue = c_include_scan(vars, &in);
	g_free(buf);
	return retvalue;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------------------
 * Defines (from @define, not include file)
 * -----------------------------------------------
 */

__attribute__((noinline)) 
static HCP_DEFINE *find_hcp_define(hcp_vars *vars, const char *name)
{
	HCP_DEFINE *d;
	
	for (d = vars->hcp_defines; d != NULL; d = d->next)
	{
		if (namecmp(name, d->name) == 0)
		{
			return d;
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */

__attribute__((noinline)) 
static gboolean add_define(hcp_vars *vars, const char *name, predef_type type, const char *val)
{
	HCP_DEFINE *d;
	size_t namelen;
	char *p;
	
	d = find_hcp_define(vars, name);
	if (d != NULL)
	{
		hcp_warning(vars, NULL, _("redefining %s"), name);
	}
	namelen = strlen(name) + 1;
	d = (HCP_DEFINE *)g_malloc(sizeof(*d) + namelen + strlen(val) + 1);
	if (G_UNLIKELY(d == NULL))
	{
		oom(vars);
		return FALSE;
	}
	p = (char *)d + sizeof(*d);
	d->name = p;
	strcpy(p, name);
	p += namelen;
	d->val = p;
	strcpy(p, val);
	d->next = vars->hcp_defines;
	d->type = type;
	vars->hcp_defines = d;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

__attribute__((noinline))
static char *replace_define(hcp_vars *vars, HCP_DEFINE *d, char *line, char *end, char **linep)
{
	size_t namelen, vallen;
	const char *val;
	
	namelen = end - line;
	val = d->val;
	vallen = strlen(val);
	if (namelen >= vallen)
	{
		/*
		 * replace text in-place
		 */
		memcpy(line, val, vallen);
		line += vallen;
		if (namelen > vallen)
		{
			namelen -= vallen;
			memmove(line, line + namelen, strlen(line + namelen) + 1);
		}
	} else
	{
		/*
		 * have to reallocate
		 */
		size_t start = line - *linep;
		
		*linep = g_renew(char, *linep, strlen(*linep) + 1 + (vallen - namelen));
		if (G_UNLIKELY(*linep == NULL))
		{
			oom(vars);
			return end;
		}
		line = *linep + start;
		memmove(line + vallen, line + namelen, strlen(line + namelen) + 1);
		memcpy(line, val, vallen);
		line += vallen;
	}
	return line;
}
 
/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Split string into arguments
 * -----------------------------------
 */

static int parse_args(hcp_vars *vars, const char *line, char eos, char ***pargv, char **pend, gboolean *first_arg_quoted)
{
	int count = 0;
	int i;
	const char *p, *start;
	char *s;
	
	*pargv = NULL;
	p = line;
	while (g_ascii_isspace(*p))
		p++;
	*first_arg_quoted = FALSE;
	while (*p != '\0' && *p != eos)
	{
		if (*p == '"')
		{
			p++;
			if (count == 0)
				*first_arg_quoted = TRUE;
			while (*p != '"' && *p != '\0')
			{
				if (*p == '\\' && p[1] != '\0')
					p++;
				p++;
			}
			if (*p == '\0')
			{
				hcp_error(vars, NULL, _("missing terminating '%c'"), '"');
			} else
			{
				p++;
			}
		} else
		{
			while (*p != '\0' && *p != eos && !g_ascii_isspace(*p))
				p++;
		}
		while (g_ascii_isspace(*p))
			p++;
		count++;
	}
	*pend = (char *)NO_CONST(p);
	if (*p == '\0' && eos != '\0')
	{
		hcp_error(vars, NULL, _("missing terminating '%c'"), eos);
		return -1;
	}
	if (count == 0)
		return 0;
	*pargv = g_new(char *, count + 1);
	if (G_UNLIKELY(*pargv == NULL))
	{
		oom(vars);
		return -1;
	}
	
	p = line;
	while (g_ascii_isspace(*p))
		p++;
	i = 0;
	while (*p != '\0' && *p != eos)
	{
		if (*p == '"')
		{
			p++;
			start = p;
			while (*p != '"' && *p != '\0')
			{
				if (*p == '\\' && p[1] != '\0')
					p++;
				p++;
			}
			(*pargv)[i] = s = g_new(char, p - start + 1);
			if (G_UNLIKELY(s == NULL))
			{
				oom(vars);
				break;
			}
			p = start;
			while (*p != '"' && *p != '\0')
			{
				if (*p == '\\')
				{
					p++;
					switch (*p)
					{
					case '"':
					case '\\':
						*s++ = *p++;
						break;
					default:
						*s++ = '\\';
						break;
					}
				} else
				{
					*s++ = *p++;
				}
			}
			*s = '\0';
			if (*p != '\0')
				p++;
		} else
		{
			start = p;
			while (*p != '\0' && *p != eos && !g_ascii_isspace(*p))
				p++;
			(*pargv)[i] = g_strndup(start, p - start);
			if (G_UNLIKELY((*pargv)[i] == NULL))
			{
				oom(vars);
				count = i;
				break;
			}
		}
		while (g_ascii_isspace(*p))
			p++;
		i++;
	}
	(*pargv)[i] = NULL;
	
	return count;
}

/* ------------------------------------------------------------------------- */

static char *parse_1arg(hcp_vars *vars, const char *line, char eos, char **pend)
{
	const char *p, *start;
	char *s;
	char *res = NULL;
	
	p = line;
	while (g_ascii_isspace(*p))
		p++;
	if (*p == '"')
	{
		p++;
		while (*p != '"' && *p != '\0')
		{
			if (*p == '\\' && p[1] != '\0')
				p++;
			p++;
		}
		if (*p == '\0')
		{
			hcp_error(vars, NULL, _("missing terminating '%c'"), '"');
		} else
		{
			p++;
		}
	} else
	{
		while (*p != '\0' && *p != eos && !g_ascii_isspace(*p))
			p++;
	}
	*pend = (char *)NO_CONST(p);
	if (*p == '\0' && eos != '\0')
	{
		hcp_error(vars, NULL, _("missing terminating '%c'"), eos);
		return NULL;
	}

	p = line;
	while (g_ascii_isspace(*p))
		p++;
	if (*p == '"')
	{
		p++;
		start = p;
		while (*p != '"' && *p != '\0')
		{
			if (*p == '\\' && p[1] != '\0')
				p++;
			p++;
		}
		res = s = g_new(char, p - start + 1);
		if (s == NULL)
		{
			oom(vars);
			return NULL;
		}
		p = start;
		while (*p != '"' && *p != '\0')
		{
			if (*p == '\\')
			{
				p++;
				switch (*p)
				{
				case '"':
				case '\\':
					*s++ = *p++;
					break;
				default:
					*s++ = '\\';
					break;
				}
			} else
			{
				*s++ = *p++;
			}
		}
		*s = '\0';
	} else
	{
		start = p;
		while (*p != '\0' && *p != eos && !g_ascii_isspace(*p))
			p++;
		res = g_strndup(start, p - start);
		if (res == NULL)
		{
			oom(vars);
			return NULL;
		}
	}
	return res;
}

/* ------------------------------------------------------------------------- */

static gboolean expect_number(hcp_vars *vars, const char *str, long *val)
{
	if (!g_is_number(str, FALSE))
	{
		hcp_error(vars, NULL, _("number expected"));
		*val = 0;
		return FALSE;
	}
	*val = strtol(str, NULL, 0);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean expect_unumber(hcp_vars *vars, const char *str, unsigned long *val)
{
	if (!g_is_number(str, TRUE))
	{
		hcp_error(vars, NULL, _("number expected"));
		*val = 0;
		return FALSE;
	}
	*val = strtoul(str, NULL, 0);
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Output during pass2
 * -----------------------------------
 */

static void addbyte(hcp_vars *vars, unsigned char ch)
{
	ASSERT(vars->in_node);
	if (vars->hcp_pass == 1)
	{
		++vars->page_used;
	} else
	{
		if (vars->page_used >= vars->page_alloced)
		{
			size_t newsize = vars->page_alloced + 1024 * 4;
			
			vars->page_buf = g_renew(unsigned char, vars->page_buf, newsize);
			if (vars->page_buf == NULL)
			{
				oom(vars);
				return;
			}
			vars->page_alloced = newsize;
		}
		if (vars->page_buf == NULL)
			return;
		vars->page_buf[vars->page_used++] = ch;
	}
	if (vars->page_used > vars->page_max_used)
		vars->page_max_used = vars->page_used;
}

/* ------------------------------------------------------------------------- */

/*
 * Encode a base 255 value.
 */
static void addenc255(hcp_vars *vars, unsigned short val)
{
	addbyte(vars, val % 255U + 1);
	addbyte(vars, val / 255U + 1);
}

/* ------------------------------------------------------------------------- */

static int addtext(hcp_vars *vars, const char *text, size_t len, size_t maxlen)
{
	char buf[HYP_UTF8_CHARMAX + 1];
	size_t i;
	const char *p, *end;
	
	if (len == STR0TERM)
		len = strlen(text);
	end = text + len;
	for (i = 0; text < end && i < maxlen; i++)
	{
		text = hyp_utf8_conv_char(vars->hyp->comp_charset, text, buf, &vars->global_converror);
		p = buf;
		/*
		 * this can still happen if we translate some utf sequence to ESC
		 */
		if (*p == HYP_ESC && p[1] == '\0')
		{
			addbyte(vars, HYP_ESC);
			addbyte(vars, HYP_ESC_ESC);
		} else
		{	
			while (*p)
			{
				addbyte(vars, *p++);
			}
		}
	}
	return (int) i;
}

/* ------------------------------------------------------------------------- */

static gboolean finish_page(hcp_vars *vars, hyp_nodenr num)
{
	unsigned long prev_offset, new_offset;
	unsigned long bytes = vars->page_used;
	gboolean retval = TRUE;
	
	new_offset = prev_offset = vars->seek_offset;
	if (G_LIKELY(!ferror(vars->outfile)))
	{
		ASSERT(prev_offset == (unsigned long)ftell(vars->outfile));
		
		if (G_LIKELY(bytes != 0))
		{
			if (G_UNLIKELY(WriteEntryBytes(vars->hyp, num, vars->page_buf, &bytes, vars->outfile, vars->opts->compression) == FALSE))
			{
				error_outfile(vars);
				retval = FALSE;
				bytes = ftell(vars->outfile) - prev_offset;
			}
		}
		new_offset = prev_offset + bytes;
		ASSERT(new_offset == (unsigned long)ftell(vars->outfile));
	}
	SetCompressedSize(vars->hyp, num, prev_offset, new_offset);
	if (G_UNLIKELY(SetDataSize(vars->hyp, num, vars->page_used) == FALSE))
	{
		hcp_error(vars, NULL, _("Node too big for compression (size diff > 64KB)!"));
	}
	vars->stats.comp_diff += vars->page_used - bytes;
	vars->seek_offset = new_offset;
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *find_uses_file(hcp_vars *vars, const char *filename)
{
	struct stat st;
	char *output_dir;
	char *path;
	char *tmp, *dirname;
	const char *list, *end;
	
	UNUSED(vars);
	/*
	 * try name as passed
	 */
	if (hyp_utf8_stat(filename, &st) == 0)
		return g_strdup(filename);
	/*
	 * no further trys if absolute name
	 */
	if (g_path_is_absolute(filename))
		return NULL;
	
	/*
	 * try in directory of output file
	 */
	output_dir = hyp_path_get_dirname(vars->hyp->file);
	if (empty(output_dir))
	{
		g_free(output_dir);
		output_dir = g_strdup(".");
	}
	path = g_build_filename(output_dir, filename, NULL);
	g_free(output_dir);
	if (hyp_utf8_stat(path, &st) == 0)
	{
		return path;
	}
	g_free(path);
	
	/*
	 * try in default search path
	 */
	if (!empty(gl_profile.general.path_list))
	{
		list = gl_profile.general.path_list;
		while (*list)
		{
			end = list;
			while (*end != '\0' && (*end != G_SEARCHPATH_SEPARATOR))
				end++;
			tmp = g_strndup(list, end - list);
			dirname = path_subst(tmp);
			g_free(tmp);
			path = g_build_filename(dirname, filename, NULL);
			g_free(dirname);
			if (hyp_utf8_stat(path, &st) == 0)
			{
				return path;
			}
		
			g_free(path);
			list = end;
			if (*list != '\0')
				list++;
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */

static gboolean load_uses_from_ref(hcp_vars *vars, REF_FILE *ref)
{
	REF_MODULE *mod;
	long num;
	char *str;
	char *filename;
	char *name;
	LABEL *lab;
	INDEX_ENTRY *entry;
	
	for (mod = ref->modules; mod != NULL; mod = mod->next)
	{
		filename = g_strdup(hyp_basename(mod->filename));
		if (G_UNLIKELY(filename == NULL))
			return FALSE;
		for (num = 0; num < mod->num_entries; num++)
		{
			switch (mod->entries[num].type)
			{
			case REF_FILENAME:
				break;
			case REF_NODENAME:
			case REF_ALIASNAME:
			case REF_LABELNAME:
				str = hyp_conv_to_utf8(mod->charset, mod->entries[num].name.hyp, STR0TERM);
				lab = add_label(vars, str, l_uses);
				if (G_UNLIKELY(lab == NULL))
				{
					g_free(str);
					g_free(filename);
					return FALSE;
				}
				lab->extern_index = vars->p1_external_node_counter;
				lab->add_to_autoref = TRUE;
				lab->lineno = HYP_NOINDEX;
				name = g_strconcat(filename, "/", str, NULL);
				entry = add_external_node(vars, HYP_NODE_EXTERNAL_REF, name);
				g_free(name);
				g_free(str);
				if (G_UNLIKELY(name == NULL || entry == NULL))
				{
					g_free(filename);
					return FALSE;
				}
				if (G_UNLIKELY(lab->labindex >= HYP_NODE_MAX))
				{
					hcp_error(vars, NULL, _("too many labels (max. %lu)"), (unsigned long)HYP_NODE_MAX);
					g_free(filename);
					return FALSE;
				}
				entry->extern_labindex = lab->labindex;
				entry->extern_nodeindex = HYP_NOINDEX;
				entry->previous = HYP_NOINDEX;
				if (mod->entries[num].type == REF_LABELNAME && mod->entries[num].lineno > 0 && mod->entries[num].lineno <= HYP_LINENO_MAX)
					lab->lineno = mod->entries[num].lineno + 1;
				if (kwsincr(vars->autorefs, lab->name, strlen(lab->name), lab) == FALSE)
				{
					oom(vars);
					g_free(filename);
					return FALSE;
				}
				vars->p1_external_uses_counter++;
				break;
			case REF_UNKNOWN:
			case REF_OS:
			case REF_CHARSET:
			case REF_DATABASE:
			case REF_LANGUAGE:
			case REF_TITLE:
			default:
				break;
			}
		}
		g_free(filename);
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean load_uses_from_hyp(hcp_vars *vars, HYP_DOCUMENT *hyp)
{
	hyp_nodenr num;
	char *str;
	char *filename;
	char *name;
	LABEL *lab;
	INDEX_ENTRY *entry;
	
	filename = g_strdup(hyp_basename(hyp->file));
	if (G_UNLIKELY(filename == NULL))
		return FALSE;
	for (num = 0; num < hyp->num_index; num++)
	{
		entry = hyp->indextable[num];
		switch (entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, STR0TERM);
			lab = add_label(vars, str, l_uses);
			if (G_UNLIKELY(lab == NULL))
			{
				g_free(str);
				g_free(filename);
				return FALSE;
			}
			lab->extern_index = vars->p1_external_node_counter;
			lab->add_to_autoref = TRUE;
			lab->lineno = HYP_NOINDEX;
			name = g_strconcat(filename, "/", str, NULL);
			entry = add_external_node(vars, HYP_NODE_EXTERNAL_REF, name);
			g_free(name);
			g_free(str);
			if (G_UNLIKELY(name == NULL || entry == NULL))
			{
				g_free(filename);
				return FALSE;
			}
			if (G_UNLIKELY(lab->labindex >= HYP_NODE_MAX))
			{
				hcp_error(vars, NULL, _("too many labels (max. %lu)"), (unsigned long)HYP_NODE_MAX);
				g_free(filename);
				return FALSE;
			}
			entry->extern_labindex = lab->labindex;
			entry->extern_nodeindex = HYP_NOINDEX;
			entry->previous = HYP_NOINDEX;
			if (kwsincr(vars->autorefs, lab->name, strlen(lab->name), lab) == FALSE)
			{
				oom(vars);
				g_free(filename);
				return FALSE;
			}
			vars->p1_external_uses_counter++;
			break;
		case HYP_NODE_EXTERNAL_REF:
			break;
		case HYP_NODE_IMAGE:
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
		default:
			break;
		}
	}
	g_free(filename);
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean load_uses(hcp_vars *vars)
{
	HCP_USES *u;
	char *path;
	int handle;
	REF_FILE *ref;
	HYP_DOCUMENT *hyp;
	hyp_filetype ftype;
	
	vars->autorefs = kwsalloc(NULL);
	if (vars->autorefs == NULL)
		return FALSE;
	for (u = vars->opts->uses; u != NULL; u = u->next)
	{
		path = find_uses_file(vars, u->filename);
		if (path == NULL || (handle = hyp_utf8_open(path, O_RDONLY|O_BINARY, HYP_DEFAULT_FILEMODE)) < 0)
		{
			hcp_warning(vars, &u->source_location, _("can't open %s. Command ignored"), path ? path : u->filename);
			g_free(path);
			continue;
		}
		if ((ref = ref_load(path, handle, FALSE)) != NULL)
		{
			load_uses_from_ref(vars, ref);
			ref_close(ref);
			hyp_utf8_close(handle);
		} else if ((hyp = hyp_load(path, handle, &ftype)) != NULL)
		{
			hyp->file = path;
			if (hyp->comp_vers > HCP_COMPILER_VERSION)
				hcp_warning(vars, &u->source_location, _("%s created by compiler version %u"), hyp->file, hyp->comp_vers);
			load_uses_from_hyp(vars, hyp);
			hyp_unref(hyp);
		} else
		{
			hcp_warning(vars, &u->source_location, _("%s is no hypertext- or reference-file. Command ignored"), path);
			hyp_utf8_close(handle);
		}
		g_free(path);
	}
	if (kwsprep(vars->autorefs) == FALSE)
		return FALSE;
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Command callbacks
 * -----------------------------------
 */

/*
 * dummy callback for commands that do nothing in pass 2
 */
static void c_do_nothing(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	UNUSED(vars);
}

/* ------------------------------------------------------------------------- */

/*
 * @master, @dnode etc.
 */
static void c_compat(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argc);
	if (vars->hcp_pass == 1)
	{
		if (vars->for_amguide)
			hcp_warning(vars, NULL, _("%s not implemented"), argv[0]);
		else
			hcp_warning(vars, NULL, _("%s ignored for AmigaGuide compatibility"), argv[0]);
	}
}

static void c_compat2(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argc);
	if (vars->hcp_pass == 1 && !vars->for_amguide)
		hcp_warning(vars, NULL, _("%s ignored for AmigaGuide compatibility"), argv[0]);
}

/* ------------------------------------------------------------------------- */

/*
 * @else
 */
static void c_else(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	toggle_if_stack(vars);
}

/* ------------------------------------------------------------------------- */

/*
 * @endif
 */
static void c_endif(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	pop_if_stack(vars);
}

/* ------------------------------------------------------------------------- */

static const char *eval_arg(hcp_vars *vars, const char *name)
{
	if (g_ascii_strcasecmp(name, "HCP") == 0)
		return "HCP";
	if (g_ascii_strcasecmp(name, "VERSION") == 0)
		return HCP_COMPILER_VERSION_STRING;
	if (g_ascii_strcasecmp(name, "MAINFILE") == 0)
	{
		HCP_INCLUDE_FILE *inc;
		
		for (inc = vars->include_stack; inc->next != NULL; inc = inc->next)
			;
		return hyp_basename(file_lookup_name(vars, inc->loc.id));
	}
	if (g_ascii_strcasecmp(name, "__FILE__") == 0)
	{
		return hyp_basename(file_lookup_name(vars, vars->include_stack->loc.id));
	}
	if (g_ascii_strcasecmp(name, "NODE") == 0)
	{
		hyp_nodenr n;
		if (!vars->in_node)
			return "<none>";
		n = vars->hcp_pass == 1 ? vars->p1_node_counter : vars->p2_node_counter;
		return vars->label_table[vars->node_table[n]->labindex]->name;
	}
	if (strcmp(name, "0") == 0)
		return "0";
	return "";
}

/* ------------------------------------------------------------------------- */

/*
 * @if <symbol> [op <string|number>]
 */
static void c_if(hcp_vars *vars, int argc, char **argv)
{
	gboolean val;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 4)
		warn_extra_args(vars);
	val = FALSE;
	if (argc == 2)
	{
		HYP_OS os;
		const char *eval;
		
		os = hyp_os_from_name(argv[1]);
		if (os != HYP_OS_UNKNOWN)
		{
			val = os == hyp_get_current_os();
		} else
		{
			eval = eval_arg(vars, argv[1]);
			val = !empty(eval) && strcmp(eval, "0") != 0;
		}
	} else if (argc >= 4)
	{
		HYP_OS os;
		const char *eval1, *eval2;
		const char *op;
		
		/*
		 * totally bogus, but this seems to be the way it works in original hcp:
		 * @if AMIGA == AMIGA
		 *    returns FALSE when compiling on Atari
		 * maybe FIXME: the documentation says the operands
		 * are always compared as strings, but this does not seem to be true for ">=" etc.,
		 * but i cant find the logic:
		 * @if MAINFILE >= x
		 * @if MAINFILE <= x
		 *    both return FALSE???
		 */
		os = hyp_os_from_name(argv[1]);
		if (os != HYP_OS_UNKNOWN)
		{
			eval1 = os == hyp_get_current_os() ? hyp_osname(os) : "unknown";
		} else
		{
			eval1 = eval_arg(vars, argv[1]);
		}
		eval2 = argv[3];
		op = argv[2];
		if (strcmp(op, "==") == 0)
			val = strcmp(eval1, eval2) == 0;
		else if (strcmp(op, "!=") == 0)
			val = strcmp(eval1, eval2) != 0;
		else if (strcmp(op, ">=") == 0)
			val = strcmp(eval1, eval2) >= 0;
		else if (strcmp(op, "<=") == 0)
			val = strcmp(eval1, eval2) <= 0;
		else if (strcmp(op, ">") == 0)
			val = strcmp(eval1, eval2) > 0;
		else if (strcmp(op, "<") == 0)
			val = strcmp(eval1, eval2) < 0;
		else if (vars->hcp_pass == 1)
			hcp_error(vars, NULL, _("bad operator"));
	} else
	{
		if (vars->hcp_pass == 1)
			hcp_error(vars, NULL, _("missing second operand"));
	}
	push_if_stack(vars, !val);
}

/* ------------------------------------------------------------------------- */

/*
 * @uses <file> [<file>...]
 */
static void c_uses(hcp_vars *vars, int argc, char **argv)
{
	int i;
	HCP_USES *u;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	/*
	 * FIXME: need a copy of opts here when compiling multiple files
	 */
	for (i = 1; i < argc; i++)
	{
		if (empty(argv[i]))
		{
			empty_arg(vars, _("filename"), TRUE);
		} else
		{
			u = hcp_add_uses(&vars->opts->uses, argv[i]);
			if (u == NULL)
				return;
			u->source_location = vars->include_stack->loc;
		}
	}
}

/* ------------------------------------------------------------------------- */

static void check_endnode(hcp_vars *vars, gboolean endnode)
{
	if (vars->in_tree && vars->hcp_pass == 1)
		hcp_warning(vars, NULL, _("missing @endtree"));
	if (vars->in_node)
	{
		if (vars->hcp_pass == 1)
		{
			if (!endnode)
				hcp_warning(vars, NULL, _("missing @endnode"));
			if (vars->textattr != 0)
			{
#define check(mask, c) if (vars->textattr & mask) hcp_warning(vars, NULL, _("textattribute still active: %c"), c)
				check(HYP_TXT_BOLD, 'B');
				check(HYP_TXT_LIGHT, 'G');
				check(HYP_TXT_ITALIC, 'I');
				check(HYP_TXT_UNDERLINED, 'U');
				check(HYP_TXT_OUTLINED, 'O');
				check(HYP_TXT_SHADOWED, 'S');
#undef check
				vars->textattr = 0;
				addbyte(vars, HYP_ESC);
				addbyte(vars, HYP_ESC_TEXTATTR_FIRST + vars->textattr);
			}
			vars->node_table[vars->p1_node_counter]->page_lines = vars->node_lineno;
			vars->p1_node_counter++;
		} else
		{
			finish_page(vars, vars->p2_node_counter);
			vars->p2_node_counter++;
		}
	}
	vars->in_node = FALSE;
	vars->in_tree = FALSE;
	vars->cur_rsc_tree_nr = HYP_NOINDEX;
	vars->node_lineno = 1;
	vars->page_used = 0;
	vars->textattr = 0;
}

/* ------------------------------------------------------------------------- */

/*
 * @node <name> [<title>]
 * @pnode <name> [<title>]
 */
static void c_do_node(hcp_vars *vars, int argc, char **argv, gboolean is_popup)
{
	NODEITEM *node;
	LABEL *lab;
	const char *nodename;
	
	check_endnode(vars, FALSE);
	if (argc < 2)
	{
		error_missing_args(vars);
		nodename = "__undefined__";
	} else
	{
		nodename = argv[1];
		check_namelen(vars, _("nodename"), nodename, TRUE);
	}
	if (argc > 3)
		warn_extra_args(vars);

	if (vars->in_preamble && vars->opts->uses)
	{
		if (vars->hcp_pass == 1)
			load_uses(vars);
		else
			vars->p2_external_node_counter += vars->p1_external_uses_counter;
	}
	
	hcp_status_node(vars, nodename);

	/*
	 * first node ends preamble
	 */
	vars->in_preamble = FALSE;

	if (vars->hcp_pass == 1)
	{
		node = find_node(vars, nodename);
		if (node != NULL)
		{
			lab = vars->label_table[node->labindex];
			hcp_error(vars, NULL,
				lab->type == l_alias ? _("redefined alias %s at %s:%lu") : _("redefined nodename %s at %s:%lu"),
				lab->name,
				file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
		}
		/* add new node anyway to keep the counter up-to-date */
		node = add_node(vars, nodename);
		if (G_UNLIKELY(node == NULL))
			return;
		if (argc > 2)
		{
			/*
			 * window title comes from extended header,
			 * where length is encoded in 2 bytes,
			 * but newer versions of HCP seems to
			 * write it also to REF file, where
			 * length is encoded in 1 byte.
			 * To be consistent, restrict it to the
			 * length of a nodename
			 */
			check_namelen(vars, _("window title"), argv[2], FALSE);
			if (!empty(argv[2]))
			{
				node->window_title = g_strdup(argv[2]);
				if (G_UNLIKELY(node->window_title == NULL))
				{
					oom(vars);
				}
			}
		}
		
		if (is_popup)
		{
			if (namecmp(nodename, hyp_default_main_node_name) == 0 ||
				namecmp(nodename, hyp_default_index_node_name) == 0 ||
				namecmp(nodename, hyp_default_help_node_name) == 0)
			{
				hcp_warning(vars, NULL, _("'%s' should not be a popup node"), nodename);
			} else if (vars->p1_node_counter == 0)
			{
				hcp_warning(vars, NULL, _("First node should not be a popup node"));
			}
		}
		
		node->is_popup = is_popup;
		lab = vars->label_table[node->labindex];
		lab->add_to_autoref = TRUE;
		lab->add_to_ref = TRUE;
		lab->add_to_index = vars->nodes_to_index > 0 && *nodename != '%';
		if (vars->opts->caseinsensitive_first)
		{
			NODEITEM *alias;
			h_unichar_t ch;
			char buf[HYP_UTF8_CHARMAX + 1];
			const char *nextc;
			size_t len;
			char *lowername;
			LABEL *aliaslab;
			
			ch = hyp_utf8_get_char(nodename);
			if (g_unichar_isupper(ch))
			{
				ch = g_unichar_tolower(ch);
				nextc = g_utf8_skipchar(nodename);
				len = hyp_unichar_to_utf8(buf, ch);
				len += strlen(nextc) + 1;
				lowername = g_new(char, len);
				if (lowername != NULL)
				{
					strcpy(lowername, buf);
					strcat(lowername, nextc);
					alias = find_node(vars, lowername);
					if (alias == NULL)
					{
						aliaslab = add_alias(vars, lowername);
						if (aliaslab)
							aliaslab->add_to_index |= vars->nodes_to_index;
					}
					g_free(lowername);
				}
			}
		}
	} else
	{
		node = vars->node_table[vars->p2_node_counter];
	}
	vars->in_node = node->is_popup ? 2 : 1;
	
	if (vars->hcp_pass == 2)
	{
		struct hyp_gfx *a;
		XREF_ITEM *xref;
		TREEDEF *tree;
		
		/*
		 * spit out escapes that should be at start of page
		 */
		
		/* @title */
		if (node->window_title)
		{
			addbyte(vars, HYP_ESC);
			addbyte(vars, HYP_ESC_WINDOWTITLE);
			addtext(vars, node->window_title, STR0TERM, STR0TERM);
			addbyte(vars, 0);
		}
		
		/* @image/@limage/@box/@rbox/@line */
		for (a = node->gfx; a != NULL; a = a->next)
		{
			switch (a->type)
			{
			case HYP_ESC_PIC:
				if (a->dithermask != 0)
				{
					addbyte(vars, HYP_ESC);
					addbyte(vars, HYP_ESC_DITHERMASK);
					addbyte(vars, 5u);
					addbyte(vars, a->dithermask >> 8);
					addbyte(vars, a->dithermask);
				}
				addbyte(vars, HYP_ESC);
				addbyte(vars, a->type);
				addenc255(vars, vars->extern_table[a->extern_node_index]->pic_node_index);	/* the real index, updated in finish_pass1 */
				addbyte(vars, a->x_offset);
				addenc255(vars, a->y_offset);
				addbyte(vars, a->islimage ? 1 : 0); /* width */
				addbyte(vars, 0);                   /* height */
				break;
			case HYP_ESC_LINE:
				addbyte(vars, HYP_ESC);
				addbyte(vars, a->type);
				addbyte(vars, a->x_offset);
				addenc255(vars, a->y_offset);
				addbyte(vars, a->width + 128);
				addbyte(vars, a->height + 1);
				addbyte(vars, a->attr + 1);
				break;
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				addbyte(vars, HYP_ESC);
				addbyte(vars, a->type);
				addbyte(vars, a->x_offset);
				addenc255(vars, a->y_offset);
				addbyte(vars, a->width);
				addbyte(vars, a->height);
				addbyte(vars, a->style + 1);
				break;
			default:
				unreachable();
				break;
			}
		}
		
		/* @xref */
		for (xref = node->xrefs; xref != NULL; xref = xref->next)
		{
			if (xref->target == HYP_NOINDEX)
			{
				/* something went wrong */
				HYP_DBG(("@xref entry %s was never resolved", xref->name));
			} else
			{
				const char *title;
				size_t len;
				
				addbyte(vars, HYP_ESC);
				addbyte(vars, HYP_ESC_EXTERNAL_REFS);
				title = xref->title ? xref->title : xref->name;
				len = target_strlen(vars, title);
				addbyte(vars, 7 + len);
				addenc255(vars, xref->target);
				addbyte(vars, ' ');	/* for the entry text; not strictly neccessary */
				addtext(vars, title, STR0TERM, STR0TERM);
				addbyte(vars, 0);
			}
		}
		
		/* data blocks?? */
		
		/* @tree */
		for (tree = node->objects; tree != NULL; tree = tree->next)
		{
			if (tree->node == HYP_NOINDEX)
			{
				/* unresolved node name, ignore */
				continue;
			}
			addbyte(vars, HYP_ESC);
			addbyte(vars, HYP_ESC_OBJTABLE);
			addenc255(vars, tree->lineno);
			addenc255(vars, tree->treenr);
			addenc255(vars, tree->objnr);
			addenc255(vars, tree->node);
		}
	}
}

/*
 * @node <name> [<title>]
 */
static void c_node(hcp_vars *vars, int argc, char **argv)
{
	c_do_node(vars, argc, argv, FALSE);
}

/*
 * @pnode <name> [<title>]
 */
static void c_pnode(hcp_vars *vars, int argc, char **argv)
{
	c_do_node(vars, argc, argv, TRUE);
}

/* ------------------------------------------------------------------------- */

/*
 * @title <name>
 */
static void c_title(hcp_vars *vars, int argc, char **argv)
{
	NODEITEM *node;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	node = vars->node_table[vars->p1_node_counter];
	/*
	 * window title comes from extended header,
	 * where length is encoded in 2 bytes,
	 * but newer versions of HCP seems to
	 * write it also to REF file, where
	 * length is encoded in 1 byte.
	 * To be consistent, restrict it to the
	 * length of a nodename
	 */
	check_namelen(vars, _("window title"), argv[1], FALSE);
	if (!empty(argv[1]))
	{
		if (node->window_title)
		{
			hcp_warning(vars, NULL, _("redefined %s"), _("window title"));
			g_free(node->window_title);
		}
		node->window_title = g_strdup(argv[1]);
		if (G_UNLIKELY(node->window_title == NULL))
		{
			oom(vars);
			return;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @toc <name>
 */
static void c_toc(hcp_vars *vars, int argc, char **argv)
{
	NODEITEM *node;
	char **namep;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (check_namelen(vars, _("nodename"), argv[1], TRUE) == FALSE)
	{
		return;
	}
	if (vars->in_node)
	{
		node = vars->node_table[vars->p1_node_counter];
		namep = &node->toc_name;
		if (*namep)
			warn_duplicate_arg(vars, argv[0]);
	} else
	{
		namep = &vars->global_toc;
	}
	g_free(*namep);
	*namep = g_strdup(argv[1]);
	if (G_UNLIKELY(*namep == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @next <name>
 */
static void c_next(hcp_vars *vars, int argc, char **argv)
{
	NODEITEM *node;
	char **namep;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (check_namelen(vars, _("nodename"), argv[1], TRUE) == FALSE)
	{
		return;
	}
	node = vars->node_table[vars->p1_node_counter];
	namep = &node->next_name;
	if (*namep)
		warn_duplicate_arg(vars, argv[0]);
	g_free(*namep);
	*namep = g_strdup(argv[1]);
	if (G_UNLIKELY(*namep == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @prev <name>
 */
static void c_prev(hcp_vars *vars, int argc, char **argv)
{
	NODEITEM *node;
	char **namep;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (check_namelen(vars, _("nodename"), argv[1], TRUE) == FALSE)
	{
		return;
	}
	node = vars->node_table[vars->p1_node_counter];
	namep = &node->prev_name;
	if (*namep)
		warn_duplicate_arg(vars, argv[0]);
	g_free(*namep);
	*namep = g_strdup(argv[1]);
	if (G_UNLIKELY(*namep == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @endnode
 */
static void c_endnode(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	if (!vars->in_node && vars->hcp_pass == 1)
		hcp_warning(vars, NULL, _("superfluous @endnode"));
	check_endnode(vars, TRUE);
}

/* ------------------------------------------------------------------------- */

/*
 * @tree <name|number> 
 */
static void c_tree(hcp_vars *vars, int argc, char **argv)
{
	RSC_DEFINE *define;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (vars->in_tree && vars->hcp_pass == 1)
		hcp_warning(vars, NULL, _("missing @endtree"));
	vars->in_tree = TRUE;
	define = find_rsc_define(vars, argv[1]);
	if (define == NULL || define->value == HYP_NOINDEX)
	{
		if (vars->hcp_pass == 1)
			hcp_warning(vars, NULL, _("Define %s not found. Skipping whole tree."), argv[1]);
		return;
	}
	vars->cur_rsc_tree_nr = define->value;
}

/* ------------------------------------------------------------------------- */

/*
 * @endtree
 */
static void c_endtree(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	if (!vars->in_tree && vars->hcp_pass == 1)
		hcp_warning(vars, NULL, _("superfluous @endtree"));
	vars->in_tree = FALSE;
	vars->cur_rsc_tree_nr = HYP_NOINDEX;
}

/* ------------------------------------------------------------------------- */

/*
 * @define <name> <value>
 */
static void c_define(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 3)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 3)
		warn_extra_args(vars);
	if (empty(argv[1]))
	{
		empty_arg(vars, _("symbol name"), TRUE);
		return;
	}
	add_define(vars, argv[1], HCP_DEF_USER, argv[2]);
}

/* ------------------------------------------------------------------------- */

/*
 * @index <name> [<name2> ...]
 */
static void c_index(hcp_vars *vars, int argc, char **argv)
{
	int i;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (vars->for_amguide)
	{
		/*
		 * for AmigaGuide this specifies the name of the index page
		 */
	} else
	{
		if (!vars->in_node)
		{
			hcp_error(vars, NULL, _("%s ignored outside @node"), argv[0]);
			return;
		}
		if (vars->in_node == 2 && !vars->allow_index_in_pnodes)
		{
			if (vars->hcp_pass == 1)
				hcp_warning(vars, NULL, _("%s not allowed in @pnode"), argv[0]);
			return;
		}
		if (vars->hcp_pass == 1)
		{
			for (i = 1; i < argc; i++)
				add_index(vars, argv[i]);
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @indexon
 */
static void c_indexon(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	vars->nodes_to_index++;
}

/* ------------------------------------------------------------------------- */

/*
 * @indexoff
 */
static void c_indexoff(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	vars->nodes_to_index--;
}

/* ------------------------------------------------------------------------- */

/*
 * @autorefon
 */
static void c_autorefon(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	vars->autoreferences++;
}

/* ------------------------------------------------------------------------- */

/*
 * @autorefoff
 */
static void c_autorefoff(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	vars->autoreferences--;
}

/* ------------------------------------------------------------------------- */

/*
 * @help <name>
 */
static void c_help(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (check_namelen(vars, _("nodename"), argv[1], TRUE) == FALSE)
	{
		return;
	}
	if (vars->hyp->help_name)
	{
		warn_duplicate_arg(vars, argv[0]);
		g_free(vars->hyp->help_name);
	}
	vars->hyp->help_name = g_strdup(argv[1]);
	if (G_UNLIKELY(vars->hyp->help_name == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @subject <name>
 */
static void c_subject(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (empty(argv[1]))
	{
		empty_arg(vars, _("subject"), FALSE);
		return;
	}
	if (vars->hyp->subject)
	{
		warn_duplicate_arg(vars, argv[0]);
		g_free(vars->hyp->subject);
	}
	vars->hyp->subject = g_strdup(argv[1]);
	if (G_UNLIKELY(vars->hyp->subject == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @default <name>
 */
static void c_default(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (check_namelen(vars, _("nodename"), argv[1], TRUE) == FALSE)
	{
		return;
	}
	if (vars->hyp->default_name)
	{
		warn_duplicate_arg(vars, argv[0]);
		g_free(vars->hyp->default_name);
	}
	vars->hyp->default_name = g_strdup(argv[1]);
	if (G_UNLIKELY(vars->hyp->default_name == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @author <name>
 */
static void c_author(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (empty(argv[1]))
	{
		empty_arg(vars, _("author"), FALSE);
		return;
	}
	if (vars->hyp->author)
	{
		warn_duplicate_arg(vars, argv[0]);
		g_free(vars->hyp->author);
	}
	vars->hyp->author = g_strdup(argv[1]);
	if (G_UNLIKELY(vars->hyp->author == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @$VER: <version>
 */
static void c_version(hcp_vars *vars, int argc, char **argv)
{
	char *p, *end;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (empty(argv[1]))
	{
		empty_arg(vars, _("version string"), FALSE);
		return;
	}
	if (vars->hyp->version)
	{
		warn_duplicate_arg(vars, argv[0]);
		g_free(vars->hyp->version);
	}
	/*
	 * ST-Guide includes everything on the line.
	 * This should maybe be fixed one day
	 */
	vars->hyp->version = g_strdup(vars->cur_fileline + 1);
	if (G_UNLIKELY(vars->hyp->version == NULL))
	{
		oom(vars);
		return;
	}
	
	while ((p = strchr(vars->hyp->version, '@')) != NULL && p[1] == ':')
	{
		HCP_DEFINE *d;
		char eos = p > vars->hyp->version && p[-1] == '(' ? ')' : '\0';
		
		char *name = parse_1arg(vars, p + 2, eos, &end);
		if (name == NULL)
			break;
		if (empty(name))
		{
			error_missing_args(vars);
			g_free(name);
			break;
		}
		d = find_hcp_define(vars, name);
		if (d == NULL)
		{
			hcp_error(vars, NULL, _("undefined variable %s"), name);
			g_free(name);
			break;
		}
		g_free(name);
		replace_define(vars, d, p, end, &vars->hyp->version);
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @database <name>
 */
static void c_database(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (empty(argv[1]))
	{
		empty_arg(vars, _("database name"), FALSE);
		return;
	}
	if (vars->hyp->database)
	{
		warn_duplicate_arg(vars, argv[0]);
		g_free(vars->hyp->database);
	}
	vars->hyp->database = g_strdup(argv[1]);
	if (G_UNLIKELY(vars->hyp->database == NULL))
	{
		oom(vars);
		return;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @hostname <app-name1> [<app-name2> ...]
 */
static void c_hostname(hcp_vars *vars, int argc, char **argv)
{
	HYP_HOSTNAME **last;
	HYP_HOSTNAME *h;
	int i;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	for (i = 1; i < argc; i++)
	{
		if (empty(argv[i]))
		{
			empty_arg(vars, _("application name"), FALSE);
		} else
		{
			h = g_new(HYP_HOSTNAME, 1);
			if (G_UNLIKELY(h == NULL))
			{
				oom(vars);
				return;
			}
			h->name = g_strdup(argv[i]);
			if (G_UNLIKELY(h->name == NULL))
			{
				g_free(h);
				oom(vars);
				return;
			}
			h->next = NULL;
			last = &vars->hyp->hostname;
			while (*last)
				last = &(*last)->next;
			*last = h;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * (re)set global vars that come from options,
 * but may change during processing
 */
static void set_globals(hcp_vars *vars)
{
	vars->nodes_to_index = vars->opts->nodes_to_index;
	vars->autoreferences = vars->opts->autoreferences;
	vars->tabwidth = vars->opts->tabwidth;
}

/* ------------------------------------------------------------------------- */

/*
 * @options <string>
 */
static void c_options(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	/*
	 * FIXME: need a copy of opts here when compiling multiple files
	 */
	if (hcp_opts_parse_string(vars->opts, argv[1], OPTS_FROM_SOURCE) == FALSE)
	{
		hcp_error(vars, NULL, _("bad option"));
	} else
	{
		set_globals(vars);
		vars->gen_index = vars->opts->gen_index;
		if (vars->hyp->hcp_options)
		{
			/*
			 * do not warn about duplicate use here
			 */
			g_free(vars->hyp->hcp_options);
			vars->hyp->hcp_options = NULL;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @os <os>
 */
static void c_os(hcp_vars *vars, int argc, char **argv)
{
	HYP_OS os;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	os = hyp_os_from_name(argv[1]);
	if (os == HYP_OS_UNKNOWN)
	{
		hcp_error(vars, NULL, _("unrecognized os %s"), argv[1]);
		return;
	}
	vars->hyp->comp_os = os;
	vars->hyp->comp_charset = hyp_default_charset(os);
}

/* ------------------------------------------------------------------------- */

/*
 * @charset <charset>
 */
static void c_charset(hcp_vars *vars, int argc, char **argv)
{
	HYP_CHARSET charset;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	charset = hyp_charset_from_name(argv[1]);
	if (charset == HYP_CHARSET_NONE)
	{
		hcp_error(vars, NULL, _("unrecognized character set %s"), argv[1]);
		return;
	}
	vars->hyp->comp_charset = charset;
}

/* ------------------------------------------------------------------------- */

/*
 * @lang <lang>
 */
static void c_lang(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (empty(argv[1]))
	{
		empty_arg(vars, _("language name"), FALSE);
		return;
	}
	/*
	 * do not warn about duplicate use here; it might be set manually,
	 * and also automatically generated by UDO
	 */
	if (vars->hyp->language && strcmp(vars->hyp->language, argv[1]) != 0)
	{
		if (vars->hcp_pass == 1)
			hcp_warning(vars, NULL, _("language changed from %s to %s"), vars->hyp->language, argv[1]);
	}
	g_free(vars->hyp->language);
	vars->hyp->language = g_strdup(argv[1]);
	vars->hyp->language_guessed = empty(vars->hyp->language);
}

/* ------------------------------------------------------------------------- */

/*
 * @inputenc <charset>
 */
static void c_inputenc(hcp_vars *vars, int argc, char **argv)
{
	HYP_CHARSET charset;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	charset = hyp_charset_from_name(argv[1]);
	if (charset == HYP_CHARSET_NONE)
	{
		hcp_error(vars, NULL, _("unrecognized character set %s"), argv[1]);
		return;
	}
	vars->input_charset = charset;
}

/* ------------------------------------------------------------------------- */

/*
 * @keywords <name> [<name2> ...]
 */
static void c_keywords(hcp_vars *vars, int argc, char **argv)
{
	NODEITEM *node;
	LABEL *lab;
	int i;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	for (i = 1; i < argc; i++)
	{
		/* labels that are to be added to REF must be unique */
		node = find_node(vars, argv[i]);
		if (G_UNLIKELY(node != NULL))
			lab = vars->label_table[node->labindex];
		else
			lab = find_label(vars, argv[i], l_label);
		if (G_UNLIKELY(lab != NULL))
		{
			hcp_warning(vars, NULL,
				lab->type == l_alias ? _("redefined alias %s at %s:%lu (ignored)") :
				lab->type == l_node ? _("redefined nodename %s at %s:%lu (ignored)") :
				_("redefined label %s at %s:%lu (ignored)"),
				lab->name,
				file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
		} else
		{
			lab = add_label(vars, argv[i], l_label);
			if (G_UNLIKELY(lab == NULL))
				return;
			lab->add_to_ref = TRUE;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @label <name> [<name2> ...]
 */
static void c_label(hcp_vars *vars, int argc, char **argv)
{
	LABEL *lab;
	int i;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (vars->hcp_pass == 1)
	{
		for (i = 1; i < argc; i++)
		{
			if (g_is_number(argv[i], TRUE))
			{
				hcp_error(vars, NULL, _("label names must not be numbers"));
			} else
			{
				lab = find_node_label(vars, argv[i], vars->p1_node_counter);
				if (G_UNLIKELY(lab != NULL))
				{
					hcp_warning(vars, NULL, _("redefined label %s at %s:%lu (ignored)"),
						lab->name,
						file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
				} else
				{
					lab = add_label(vars, argv[i], l_label);
					if (G_UNLIKELY(lab == NULL))
						return;
				}
			}
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @alabel <name> [<name2> ...]
 */
static void c_alabel(hcp_vars *vars, int argc, char **argv)
{
	LABEL *lab;
	int i;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	for (i = 1; i < argc; i++)
	{
		lab = find_label(vars, argv[i], l_label);
		if (lab == NULL)
			lab = find_label(vars, argv[i], l_alias);
		if (G_UNLIKELY(lab != NULL))
		{
			hcp_warning(vars, NULL,
				lab->type == l_alias ? _("redefined alias %s at %s:%lu (ignored)") : _("redefined label %s at %s:%lu (ignored)"),
				lab->name,
				file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
		} else
		{
			lab = add_label(vars, argv[i], l_label);
			if (G_UNLIKELY(lab == NULL))
				return;
			lab->add_to_autoref = TRUE;
			lab->add_to_index = vars->opts->alabel_to_index;
			lab->add_to_ref = TRUE;
			if (vars->node_lineno == 1)
				lab->type = l_alias;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @alias <name> [<name2> ...]
 */
static void c_alias(hcp_vars *vars, int argc, char **argv)
{
	NODEITEM *node;
	LABEL *lab;
	int i;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	for (i = 1; i < argc; i++)
	{
		node = find_node(vars, argv[i]);
		if (G_UNLIKELY(node != NULL))
		{
			lab = vars->label_table[node->labindex];
			hcp_warning(vars, NULL,
				lab->type == l_alias ? _("redefined alias %s at %s:%lu (ignored)") : _("redefined nodename %s at %s:%lu (ignored)"),
				lab->name,
				file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
		} else
		{
			add_alias(vars, argv[i]);
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @symbol [air] <name> [<name2 ...]
 */
static void c_symbol(hcp_vars *vars, int argc, char **argv)
{
	gboolean add_to_autoref = FALSE;
	gboolean add_to_index = FALSE;
	gboolean add_to_ref = FALSE;
	const char *options;
	NODEITEM *node;
	LABEL *lab;
	int i;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	i = 1;
	if (argc == 2)
	{
		options = "air";
	} else
	{
		options = argv[i];
		if (empty(options))
		{
			empty_arg(vars, _("symbol flags"), TRUE);
			return;
		}
		i++;
	}
	while (*options)
	{
		switch (*options)
		{
		case 'a':
		case 'A':
			add_to_autoref = TRUE;
			break;
		case 'i':
		case 'I':
			add_to_index = TRUE;
			break;
		case 'r':
		case 'R':
			add_to_ref = TRUE;
			break;
		default:
			hcp_error(vars, NULL, _("illegal flag '%c' for symbol"), *options);
			return;
		}
		options++;
	}

	if (vars->in_node == 2 &&
		(add_to_autoref || add_to_ref || (add_to_index && !vars->allow_index_in_pnodes)))
	{
		if (vars->hcp_pass == 1)
			hcp_warning(vars, NULL, _("%s not allowed in @pnode"), argv[0]);
		return;
	}

	for (; i < argc; i++)
	{
		if (add_to_ref)
		{
			/* labels that are to be added to REF must be unique */
			node = find_node(vars, argv[i]);
			if (G_UNLIKELY(node != NULL))
				lab = vars->label_table[node->labindex];
			else
				lab = find_label(vars, argv[i], l_label);
			if (G_UNLIKELY(lab != NULL))
			{
				hcp_warning(vars, NULL,
					lab->type == l_alias ? _("redefined alias %s at %s:%lu (ignored)") :
					lab->type == l_node ? _("redefined nodename %s at %s:%lu (ignored)") :
					_("redefined label %s at %s:%lu (ignored)"),
					lab->name,
					file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
				continue;
			}
		}
		lab = find_node_label(vars, argv[i], vars->p1_node_counter);
		if (G_UNLIKELY(lab != NULL))
		{
			hcp_warning(vars, NULL, _("redefined label %s at %s:%lu (ignored)"),
				lab->name,
				file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
		} else
		{
			lab = add_label(vars, argv[i], l_label);
			if (G_UNLIKELY(lab == NULL))
				return;
			lab->add_to_autoref = add_to_autoref;
			lab->add_to_index = add_to_index;
			lab->add_to_ref = add_to_ref;
			if (!add_to_autoref && !add_to_ref)
				lab->type = l_index;
			else if (lab->lineno == 1)
				lab->type = l_alias;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @box <X-Offset> <width> <height> [<pattern>]
 * @rbox <X-Offset> <width> <height> [<pattern>]
 */
static void c_do_box(hcp_vars *vars, int argc, char **argv, short type)
{
	long val;
	struct hyp_gfx adm;
	struct hyp_gfx *a, **last;
	NODEITEM *node;
	
	if (vars->hcp_pass == 1)
	{
		if (argc < 4)
		{
			error_missing_args(vars);
			return;
		}
		if (argc > 5)
			warn_extra_args(vars);
		if (vars->in_node == 2 && vars->opts->warn_compat)
			hcp_warning(vars, NULL, _("ST-Guide is not able to display graphics in popup nodes"));
		
		memset(&adm, 0, sizeof(adm));
		adm.type = type;
		adm.y_offset = vars->node_lineno - 1;
		
		/*
		 * most of the range restriction of the arguments
		 * are due to the way they are encoded in the binary file
		 */
		
		if (expect_number(vars, argv[1], &val))
		{
			if (val <= 0 || val > 255)
				hcp_error(vars, NULL, _("X-Position out of range"));
			else
				adm.x_offset = (_WORD)val;
		}
		
		if (expect_number(vars, argv[2], &val))
		{
			if (val <= 0 || val > 255)
				hcp_error(vars, NULL, _("bad width"));
			else
				adm.width = (_WORD)val;
		}
				
		if (expect_number(vars, argv[3], &val))
		{
			if (val <= 0 || val > 255)
				hcp_error(vars, NULL, _("bad height"));
			else
				adm.height = (_WORD)val;
		}
		
		adm.style = 0;
		if (argc > 4)
		{
			if (expect_number(vars, argv[4], &val))
			{
				if (val < 0 || val > 36)
					hcp_error(vars, NULL, _("bad pattern"));
				else
					adm.style = (_WORD)val;
			}
		}

		a = g_new(struct hyp_gfx, 1);
		if (G_UNLIKELY(a == NULL))
		{
			oom(vars);
			return;
		}
		*a = adm;
		node = vars->node_table[vars->p1_node_counter];
		last = &node->gfx;
		while (*last != NULL)
			last = &(*last)->next;
		*last = a;
		
		if ((adm.y_offset + adm.height) > node->minheight)
			node->minheight = adm.y_offset + adm.height;
		
		/* account for gfx escape */
		vars->page_used += 8;
	} else
	{
		/* gfx command already written at start of page */
	}
}

static void c_box(hcp_vars *vars, int argc, char **argv)
{
	c_do_box(vars, argc, argv, HYP_ESC_BOX);
}

static void c_rbox(hcp_vars *vars, int argc, char **argv)
{
	c_do_box(vars, argc, argv, HYP_ESC_RBOX);
}

/* ------------------------------------------------------------------------- */

/*
 * @line <X-Offset> <width> <height> [<attribute> [<style>]]
 */
static void c_line(hcp_vars *vars, int argc, char **argv)
{
	long val;
	struct hyp_gfx adm;
	struct hyp_gfx *a, **last;
	NODEITEM *node;
	
	if (vars->hcp_pass == 1)
	{
		if (argc < 4)
		{
			error_missing_args(vars);
			return;
		}
		if (argc > 6)
			warn_extra_args(vars);
		if (vars->in_node == 2 && vars->opts->warn_compat)
			hcp_warning(vars, NULL, _("ST-Guide is not able to display graphics in popup nodes"));
		
		memset(&adm, 0, sizeof(adm));
		adm.type = HYP_ESC_LINE;
		adm.y_offset = vars->node_lineno - 1;
		
		if (expect_number(vars, argv[1], &val))
		{
			if (val <= 0 || val > 255)
				hcp_error(vars, NULL, _("X-Position out of range"));
			else
				adm.x_offset = (_WORD)val;
		}
		
		if (expect_number(vars, argv[2], &val))
		{
			if (val < -127 || val > 127)
				hcp_error(vars, NULL, _("bad width"));
			else
				adm.width = (_WORD)val;
		}
				
		if (expect_number(vars, argv[3], &val))
		{
			if (val < 0 || val > 254)
				hcp_error(vars, NULL, _("bad height"));
			else
				adm.height = (_WORD)val;
		}
				
		adm.style = 1;
		adm.begend = 0;
		if (argc > 4)
		{
			if (expect_number(vars, argv[4], &val))
			{
				if (val < 0 || val > 3)
					hcp_error(vars, NULL, _("bad type"));
				else
					adm.begend = (_WORD)val;
			}
		}
		if (argc > 5)
		{
			if (expect_number(vars, argv[5], &val))
			{
				if (val < 1 || val > 7)
					hcp_error(vars, NULL, _("bad style"));
				else
					adm.style = (_WORD)val;
			}
		}
		adm.attr = (((adm.style - 1) << 3) + adm.begend);
		
		a = g_new(struct hyp_gfx, 1);
		if (G_UNLIKELY(a == NULL))
		{
			oom(vars);
			return;
		}
		*a = adm;
		node = vars->node_table[vars->p1_node_counter];
		last = &node->gfx;
		while (*last != NULL)
			last = &(*last)->next;
		*last = a;
		
		if ((adm.y_offset + adm.height) > node->minheight)
			node->minheight = adm.y_offset + adm.height;
		
		/* account for gfx escape */
		vars->page_used += 8;
	} else
	{
		/* gfx command already written at start of page */
	}
}

/* ------------------------------------------------------------------------- */

static hyp_pic_format get_image_type(hcp_vars *vars, int handle, FILELIST *f, PICTURE *pic, unsigned char **bufp)
{
	size_t size;
	size_t ret;
	pic_filetype pic_format;
	hyp_pic_format format;
	
	*bufp = NULL;
	size = lseek(handle, 0, SEEK_END);
	if (size == (size_t)-1)
	{
		hcp_error(vars, &f->first_reference, _("%s: can't determine size: %s"), f->name, hyp_utf8_strerror(errno));
		return HYP_PIC_UNKNOWN;
	}
	if (size == 0)
	{
		hcp_error(vars, &f->first_reference, _("%s: empty file"), f->name);
		return HYP_PIC_UNKNOWN;
	}
	*bufp = g_new(unsigned char, size);
	if (*bufp == NULL)
	{
		oom(vars);
		return HYP_PIC_UNKNOWN;
	}
	lseek(handle, 0, SEEK_SET);
	ret = read(handle, *bufp, size);
	if (ret != size)
	{
		hcp_error(vars, &f->first_reference, _("%s: read error"), f->name);
		return HYP_PIC_UNKNOWN;
	}
	
	pic_init(pic);
	pic->pi_filesize = size;
	pic_format = pic_type(pic, *bufp, size);
	
	switch (pic_format)
	{
	case FT_IFF:
		format = HYP_PIC_IFF;
		break;
	case FT_ICN:
		format = HYP_PIC_ICN;
		break;
	case FT_IMG:
		format = HYP_PIC_IMG;
		break;
	case FT_BMP:
		format = HYP_PIC_BMP;
		break;
	case FT_PNG:
		format = HYP_PIC_PNG;
#ifndef HAVE_PNG
		hcp_error(vars, &f->first_reference, _("%s: PNG not supported on this platform"), f->name);
#endif
		break;
	case FT_UNKNOWN:
		hcp_error(vars, &f->first_reference, _("%s: unknown file format"), f->name);
		return HYP_PIC_UNKNOWN;

	case FT_EXEC_FIRST:
	case FT_EXEC:
	case FT_TOS:
	case FT_TTP:
	case FT_PRG:
	case FT_GTP:
	case FT_EXEC_LAST:
	case FT_PICTURE_FIRST:
	case FT_DEGAS_LOW:
	case FT_DEGAS_MED:
	case FT_DEGAS_HIGH:
	case FT_NEO:
	case FT_COLSTAR:
	case FT_STAD:
	case FT_IMAGIC_LOW:
	case FT_IMAGIC_MED:
	case FT_IMAGIC_HIGH:
	case FT_SCREEN:
	case FT_ICO:
	case FT_CALAMUS_PAGE:
	case FT_GIF:
	case FT_TIFF:
	case FT_TARGA:
	case FT_PBM:
	case FT_PICTURE_LAST:
	case FT_ARCHIVE_FIRST:
	case FT_ARC:
	case FT_ZOO:
	case FT_LZH:
	case FT_ZIP:
	case FT_ARJ:
	case FT_TAR:
	case FT_GZ:
	case FT_BZ2:
	case FT_ARCHIVE_LAST:
	case FT_DOC_FIRST:
	case FT_ASCII:
	case FT_WORDPLUS:
	case FT_SIGDOC:
	case FT_DOC_LAST:
	case FT_FONT_FIRST:
	case FT_GEMFNT:
	case FT_SIGFNT:
	case FT_FONT_LAST:
	case FT_MISC_FIRST:
	case FT_EMPTY:
	case FT_DRI:
	case FT_DRILIB:
	case FT_BOBJECT:
	case FT_RSC:
	case FT_GFA2:
	case FT_GFA3:
	default:
		if (pic_format >= FT_PICTURE_FIRST && pic_format < FT_PICTURE_LAST)
			hcp_error(vars, &f->first_reference, "%s: %s", f->name, _("unsupported picture format"));
		else
			hcp_error(vars, &f->first_reference, _("%s: not a picture format"), f->name);
		return HYP_PIC_UNKNOWN;
	}
	
	if (pic->pi_planes != 1 && pic->pi_planes != 4 && pic->pi_planes != 8 && pic->pi_planes != 24 && pic->pi_planes != 32)
	{
		char *colors;
		
		colors = pic_colornameformat(pic->pi_planes);
		hcp_error(vars, &f->first_reference, _("%s: unsupported number of colors (%s)"), f->name, colors + 1);
		g_free(colors);
		return HYP_PIC_UNKNOWN;
	}

	f->pic.format = format;
	f->pic.width = pic->pi_width;
	f->pic.height = pic->pi_height;
	f->pic.planes = pic->pi_planes;
	return format;
}
	
/*
 * @image <file> <X-Offset> [%<dither-mask>]
 * @limage <file> <X-Offset> [%<dither-mask>]
 */
static void c_do_image(hcp_vars *vars, int argc, char **argv, gboolean islimage)
{
	long val;
	struct hyp_gfx adm;
	struct hyp_gfx *a, **last;
	NODEITEM *node;
	const char *filename;
	gboolean is_new;
	hyp_filetype type;
	FILELIST *f;
	char *path;
	const char *realname;
	
	if (vars->hcp_pass == 1)
	{
		gboolean exists = FALSE;
		
		if (argc < 3)
		{
			error_missing_args(vars);
			return;
		}
		if (argc > 4)
			warn_extra_args(vars);
		if (vars->in_node == 2 && vars->opts->warn_compat)
			hcp_warning(vars, NULL, _("ST-Guide is not able to display graphics in popup nodes"));
		filename = argv[1];
		if (empty(filename))
		{
			empty_arg(vars, _("filename"), TRUE);
			return;
		}
		
		memset(&adm, 0, sizeof(adm));
		adm.type = HYP_ESC_PIC;
		adm.y_offset = vars->node_lineno - 1;
		adm.width = islimage ? 1 : 0;
		adm.islimage = islimage;
		type = HYP_FT_IMAGE;
		path = find_include_file(vars, filename);
		f = file_listadd(vars, path, &type, &is_new);
		g_free(path);
		if (f == NULL)
			return;
		realname = f->name;
		adm.id = f->id;
		if (is_new)
		{
			const char *name;
			INDEX_ENTRY *entry;
			int fd;
			PICTURE pic;
			
			adm.extern_node_index = vars->p1_external_node_counter;
			f->extern_node_index = adm.extern_node_index;
			name = hyp_basename(realname);
			fd = hyp_utf8_open(realname, O_RDONLY|O_BINARY, 0);
			if (fd >= 0)
			{
				exists = TRUE;
				if (vars->opts->read_images)
				{
					unsigned char *buf;
					f->pic.format = get_image_type(vars, fd, f, &pic, &buf);
					g_free(buf);
				}
				hyp_utf8_close(fd);
				if (vars->opts->warn_compat && f->pic.format != HYP_PIC_UNKNOWN && f->pic.planes > 8)
					hcp_warning(vars, NULL, _("ST-Guide is not able to display images with %d planes"), f->pic.planes);
			} else
			{
				if (!vars->opts->read_images)
					hcp_warning(vars, NULL, _("can't open file '%s': %s"), filename, hyp_utf8_strerror(errno));
				else
					hcp_error(vars, NULL, _("can't open file '%s': %s"), filename, hyp_utf8_strerror(errno));
			}
			if (exists && vars->opts->read_images)
			{
				entry = add_external_node(vars, HYP_NODE_IMAGE, name);
				if (G_UNLIKELY(entry == NULL))
					return;
				entry->pic_file_id = adm.id;
			} else
			{
				f->extern_node_index = adm.extern_node_index = HYP_NOINDEX;
			}
		} else
		{
			adm.extern_node_index = f->extern_node_index;
			exists = f->extern_node_index != HYP_NOINDEX;
		}
		
		if (type != HYP_FT_IMAGE)
			hcp_warning(vars, NULL, _("image-file %s already used as sourcefile"), filename);
		
		if (expect_number(vars, argv[2], &val))
		{
			if (val < 0 || val > 255)
				hcp_error(vars, NULL, _("X-Position out of range"));
			else
				adm.x_offset = (_WORD)val;
			if (val == 0)
				vars->uses_limage = TRUE;
		}
		
		if (argc > 3)
		{
			const char *s = argv[3];
			int bit = 0;
			gboolean err = FALSE;
			
			if (*s == '%')
				s++;
			while (*s)
			{
				if (bit >= 16)
					err = TRUE;
				else if (*s == '1')
					adm.dithermask |= 1 << vdi_maptab16[bit];
				else if (*s != '0')
					err = TRUE;
				s++;
				bit++;
			}
			if (err)
			{
				hcp_error(vars, NULL, _("invalid dithermask '%s'"), argv[3]);
				adm.dithermask = 0;
			} else if (f->pic.format == HYP_PIC_UNKNOWN)
			{
				adm.dithermask = 0;
			} else if (f->pic.planes <= 1 || f->pic.planes > 8)
			{
				hcp_error(vars, NULL, _("dithermask not applyable to images with %d planes"), f->pic.planes);
				adm.dithermask = 0;
			} else if (bit > (1 << f->pic.planes))
			{
				hcp_error(vars, NULL, _("too many bits in dithermask"));
				adm.dithermask = 0;
			} else if (adm.dithermask == 0)
			{
				hcp_warning(vars, NULL, _("empty dithermask will be ignored"));
				adm.dithermask = 0;
			} else if (adm.dithermask == (1 << f->pic.planes) - 1)
			{
				hcp_warning(vars, NULL, _("ignoring dithermask of all black"));
				adm.dithermask = 0;
			}
		}
				
		if (!exists || !vars->opts->read_images)
		{
			/*
			 * write a @box command instead
			 */
			adm.type = HYP_ESC_BOX;
			/*
			 * offset == 0 for centered images not supported by box command
			 */
			if (adm.x_offset == 0)
				adm.x_offset = 1;
			adm.width = 1;
			adm.height = 1;
			adm.style = 1;
		}
		
		a = g_new(struct hyp_gfx, 1);
		if (G_UNLIKELY(a == NULL))
		{
			oom(vars);
			return;
		}
		*a = adm;
		node = vars->node_table[vars->p1_node_counter];
		last = &node->gfx;
		while (*last != NULL)
			last = &(*last)->next;
		*last = a;
		
		/* account for gfx escape */
		vars->page_used += 9;
		if (adm.dithermask != 0)
			vars->page_used += 5;
	} else
	{
		FILELIST *f;
		
		type = HYP_FT_IMAGE;
		filename = argv[1];
		path = find_include_file(vars, filename);
		f = file_listadd(vars, path, &type, &is_new);
		g_free(path);
		ASSERT(f != NULL);
		ASSERT(!is_new);
		if (f != NULL && f->first_use)
		{
			f->first_use = FALSE;
			if (f->extern_node_index != HYP_NOINDEX)
			{
				vars->p2_external_node_counter++;
				vars->p2_real_external_node_counter++;
			}
		}
		/* gfx command already written at start of page */
	}
}

static void c_image(hcp_vars *vars, int argc, char **argv)
{
	c_do_image(vars, argc, argv, FALSE);
}

static void c_limage(hcp_vars *vars, int argc, char **argv)
{
	c_do_image(vars, argc, argv, TRUE);
}

/* ------------------------------------------------------------------------- */

/*
 * @width <number>
 */
static void c_width(hcp_vars *vars, int argc, char **argv)
{
	unsigned long width;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (!expect_unumber(vars, argv[1], &width))
		return;
	if (width < HYP_STGUIDE_MIN_LINEWIDTH)
	{
		if (vars->hcp_pass == 1)
			hcp_error(vars, NULL, _("bad width"));
		return;
	}
	if (width > HYP_STGUIDE_MAX_LINEWIDTH)
	{
		if (vars->hcp_pass == 1 && vars->opts->warn_compat)
			hcp_warning(vars, NULL, _("width may not be accepted by ST-Guide"));
	}
	vars->hyp->line_width = (int) width;
}

/* ------------------------------------------------------------------------- */

/*
 * @tabsize <number>
 * @tab <number>
 */
static void c_tabsize(hcp_vars *vars, int argc, char **argv)
{
	unsigned long tab;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (!expect_unumber(vars, argv[1], &tab))
		return;
	if (tab < 1 || tab > 9)
	{
		if (vars->hcp_pass == 1)
			hcp_error(vars, NULL, _("illegal tabsize"));
		return;
	}
	if (vars->in_node)
	{
		hyp_nodenr n = vars->hcp_pass == 1 ? vars->p1_node_counter : vars->p2_node_counter;
		vars->node_table[n]->tabwidth = (int) tab;
		if (vars->hcp_pass == 1 && vars->opts->warn_compat)
			hcp_warning(vars, NULL, _("@tabsize will only affect this node"));
	} else
	{
		vars->tabwidth = (int) tab;
	}
}

/* ------------------------------------------------------------------------- */

/* recursive forward declaration */
static gboolean pass(hcp_vars *vars, const char *filename);

/*
 * @include <name>
 */
static void c_include(hcp_vars *vars, int argc, char **argv)
{
	const char *filename;
	char *path;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 2)
		warn_extra_args(vars);
	filename = argv[1];
	if (empty(filename))
	{
		empty_arg(vars, _("filename"), TRUE);
		return;
	}
	path = find_include_file(vars, filename);
	if (hyp_guess_filetype(path) == HYP_FT_CHEADER)
	{
		read_c_include(vars, path);
	} else
	{
		pass(vars, path);
	}
	g_free(path);
}

/* ------------------------------------------------------------------------- */

/*
 * @extern <text> file/<name> [line number]
 */
static void c_extern(hcp_vars *vars, int argc, char **argv)
{
	LABEL *lab;
	unsigned long lineno;
	INDEX_ENTRY *entry;
	
	if (argc < 3)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 4)
		warn_extra_args(vars);
	if (empty(argv[1]))
	{
		empty_arg(vars, _("label"), TRUE);
		return;
	}
	if (empty(argv[2]))
	{
		empty_arg(vars, _("nodename"), TRUE);
		return;
	}
	if (vars->hcp_pass == 2)
	{
		entry = vars->extern_table[vars->p2_external_node_counter];
		vars->p2_external_node_counter++;
		if (entry->extern_labindex == HYP_NOINDEX)
			return;
		lab = vars->label_table[entry->extern_labindex];
		if (lab->extern_index == HYP_NOINDEX)
			return;
		vars->p2_real_external_node_counter++;
		return;
	}
	lab = find_label(vars, argv[1], l_extern);
	if (lab == NULL)
		lab = find_label(vars, argv[1], l_uses);
	if (G_UNLIKELY(lab != NULL))
	{
		if (vars->hcp_pass == 1 && lab->type == l_extern)
			hcp_warning(vars, NULL, _("redefined extern name %s at %s:%lu"),
				lab->name,
				file_lookup_name(vars, lab->source_location.id), lab->source_location.lineno);
		lab->source_location = vars->include_stack->loc;
		lab->type = l_extern;
		entry = vars->extern_table[lab->extern_index];
		entry->extern_labindex = HYP_NOINDEX;
	} else
	{
		lab = add_label(vars, argv[1], l_extern);
		if (G_UNLIKELY(lab == NULL))
			return;
	}
	lab->extern_index = vars->p1_external_node_counter;
	entry = add_external_node(vars, HYP_NODE_EXTERNAL_REF, argv[2]);
	if (G_UNLIKELY(entry == NULL))
		return;
	if (G_UNLIKELY(lab->labindex >= HYP_NODE_MAX))
	{
		hcp_error(vars, NULL, _("too many labels (max. %lu)"), (unsigned long)HYP_NODE_MAX);
		return;
	}
	entry->extern_labindex = lab->labindex;
	entry->extern_nodeindex = HYP_NOINDEX;
	entry->previous = HYP_NOINDEX;
	lab->add_to_autoref = TRUE;
	lab->lineno = HYP_NOINDEX;
	
	/*
	 * @extern is allowed everywhere;
	 * we do not know wether is was really referenced until after pass2
	 * but need to write it to the index table at the start of pass2
	 */
	lab->referenced = TRUE;
	
	if (argc > 3)
	{
		if (expect_unumber(vars, argv[3], &lineno))
		{
			if (lineno > HYP_LINENO_MAX)
			{
				hcp_error(vars, NULL, _("bad line number"));
			} else
			{
				lab->lineno = (hyp_lineno) lineno;
			}
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * @noref
 */
static void c_noref(hcp_vars *vars, int argc, char **argv)
{
	UNUSED(argv);
	if (argc > 1)
		warn_extra_args(vars);
	vars->label_table[vars->node_table[vars->p1_node_counter]->labindex]->add_to_autoref = FALSE;
}

/* ------------------------------------------------------------------------- */

/*
 * @xref <name> [<title>]
 */
static void c_xref(hcp_vars *vars, int argc, char **argv)
{
	XREF_ITEM *xref;
	XREF_ITEM **last;
	NODEITEM *node;
	size_t len;
	int count;
	INDEX_ENTRY *entry;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return;
	}
	if (argc > 3)
		warn_extra_args(vars);
	if (check_namelen(vars, _("nodename"), argv[1], TRUE) == FALSE)
	{
		return;
	}
	if (vars->hcp_pass == 1)
	{
		len = strlen(argv[1]);
		xref = (XREF_ITEM *)g_malloc(sizeof(XREF_ITEM) + len);
		if (G_UNLIKELY(xref == NULL))
		{
			oom(vars);
			return;
		}
		xref->next = NULL;
		xref->title = NULL;
		xref->target = HYP_NOINDEX;
		strcpy(xref->name, argv[1]);
		if (argc > 2)
		{
			char *title = chomp(argv[2]);
			check_namelen(vars, _("window title"), title, FALSE);
			if (!empty(title) && namecmp(xref->name, title) != 0)
			{
				xref->title = g_strdup(title);
				if (G_UNLIKELY(xref->title == NULL))
				{
					g_free(xref);
					oom(vars);
					return;
				}
			}
		}
		node = vars->node_table[vars->p1_node_counter];
		count = 0;
		last = &node->xrefs;
		while (*last)
		{
			last = &(*last)->next;
			count++;
		}
		*last = xref;
		if (count >= 12 && vars->opts->warn_compat)
			hcp_warning(vars, NULL, _("ST-Guide may not be able to display more than 12 @xref items"));
		
		entry = add_external_node(vars, (hyp_indextype)HYP_NODE_XREF, argv[1]);
		if (G_UNLIKELY(entry == NULL))
			return;
		/*
		 * temporary use to identify the entry when trying to resolve xrefs
		 */
		entry->xref_node_index = vars->p1_node_counter;
		entry->xref_index = count;
		
		/* account for escape sequence */
		vars->page_used += 6 + target_strlen(vars, xref->title ? xref->title : xref->name);
		
		vars->uses_xref = TRUE;
	} else
	{
		/* output already done at start of page */
		/* hyp_nodenr target = vars->p2_real_external_node_counter; */
		entry = vars->extern_table[vars->p2_external_node_counter];
		vars->p2_external_node_counter++;
		if (entry->type == HYP_NODE_INTERNAL || entry->type == HYP_NODE_POPUP)
		{
			/*
			 * link has been resolved internally.
			 * That entry will not be written to the index table
			 */
		} else if (entry->type == HYP_NODE_EXTERNAL_REF && entry->extern_labindex != HYP_NOINDEX)
		{
			if (entry->extern_nodeindex == HYP_NOINDEX)
				vars->p2_real_external_node_counter++;
		}
	}
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Command table
 * -----------------------------------
 */

static struct hcp_command const hcp_commands[] = {
	{ "if",           CMD_ALWAYS,                    c_if,         c_if },
	{ "else",         CMD_ALWAYS,                    c_else,       c_else },
	{ "endif",        CMD_ALWAYS,                    c_endif,      c_endif },
	{ "default",      CMD_IN_PREAMBLE,               c_default,    c_do_nothing },
	{ "help",         CMD_IN_PREAMBLE,               c_help,       c_do_nothing },
	{ "uses",         CMD_IN_PREAMBLE,               c_uses,       c_do_nothing },
	{ "subject",      CMD_IN_PREAMBLE,               c_subject,    c_do_nothing },
	{ "author",       CMD_IN_PREAMBLE,               c_author,     c_do_nothing },
	{ "database",     CMD_IN_PREAMBLE,               c_database,   c_do_nothing },
	{ "$VER:",        CMD_IN_PREAMBLE,               c_version,    c_do_nothing },
	{ "options",      CMD_IN_PREAMBLE,               c_options,    c_do_nothing },
	{ "os",           CMD_IN_PREAMBLE,               c_os,         c_do_nothing },
	{ "charset",      CMD_IN_PREAMBLE,               c_charset,    c_do_nothing },
	{ "lang",         CMD_IN_PREAMBLE,               c_lang,       c_do_nothing },
	{ "inputenc",     0,                             c_inputenc,   c_inputenc },
	{ "node",         0,                             c_node,       c_node },
	{ "pnode",        0,                             c_pnode,      c_pnode },
	{ "endnode",      0,                             c_endnode,    c_endnode },
	{ "label",        CMD_IN_NODE|CMD_NOT_PNODE,     c_label,      c_do_nothing },
	{ "next",         CMD_IN_NODE,                   c_next,       c_do_nothing },
	{ "prev",         CMD_IN_NODE,                   c_prev,       c_do_nothing },
	{ "toc",          0,                             c_toc,        c_do_nothing },
	{ "width",        0,                             c_width,      c_width },
	{ "xref",         CMD_IN_NODE|CMD_NOT_PNODE,     c_xref,       c_xref },
	{ "alabel",       CMD_IN_NODE|CMD_NOT_PNODE,     c_alabel,     c_do_nothing },
	{ "alias",        CMD_IN_NODE,                   c_alias,      c_do_nothing },
	{ "autorefoff",   0,                             c_autorefoff, c_autorefoff },
	{ "autorefon",    0,                             c_autorefon,  c_autorefon },
	{ "extern",       0,                             c_extern,     c_extern },
	{ "noref",        CMD_IN_NODE,                   c_noref,      c_do_nothing },
	{ "symbol",       CMD_IN_NODE,                   c_symbol,     c_do_nothing },
	{ "indexoff",     0,                             c_indexoff,   c_indexoff },
	{ "indexon",      0,                             c_indexon,    c_indexon },
	{ "box",          CMD_IN_NODE /* |CMD_NOT_PNODE */,     c_box,        c_box },
	{ "rbox",         CMD_IN_NODE /* |CMD_NOT_PNODE */,     c_rbox,       c_rbox },
	{ "image",        CMD_IN_NODE /* |CMD_NOT_PNODE */,     c_image,      c_image },
	{ "limage",       CMD_IN_NODE /* |CMD_NOT_PNODE */,     c_limage,     c_limage },
	{ "line",         CMD_IN_NODE /* |CMD_NOT_PNODE */,     c_line,       c_line },
	{ "remark",       0,                             c_do_nothing, c_do_nothing },
	{ "rem",          0,                             c_do_nothing, c_do_nothing },
	{ "include",      0,                             c_include,    c_include },
	{ "index",        0,                             c_index,      c_do_nothing },
	{ "title",        CMD_IN_NODE,                   c_title,      c_do_nothing },
	{ "hostname",     0,                             c_hostname,   c_do_nothing },
	{ "tabsize",      0,                             c_tabsize,    c_tabsize },
	{ "tab",          0,                             c_tabsize,    c_tabsize },			/* AmigaGuide uses this name */
	{ "tree",         CMD_IN_NODE,                   c_tree,       c_tree },
	{ "endtree",      CMD_IN_NODE,                   c_endtree,    c_endtree },
	{ "define",       0,                             c_define,     c_do_nothing },
	{ "(c)",          CMD_IN_PREAMBLE,               c_compat2,    c_do_nothing },
	{ "master",       CMD_IN_PREAMBLE,               c_compat,     c_do_nothing },
	{ "dnode",        0,                             c_compat,     c_do_nothing },
	{ "font",         0,                             c_compat,     c_do_nothing },
	{ "height",       0,                             c_compat,     c_do_nothing },
	{ "keywords",     0,                             c_keywords,   c_do_nothing },
	{ "wordwrap",     0,                             c_compat,     c_do_nothing },
	{ "macro",        0,                             c_compat,     c_do_nothing },
	{ "onclose",      0,                             c_compat,     c_do_nothing },
	{ "onopen",       0,                             c_compat,     c_do_nothing },
	{ "smartwrap",    0,                             c_compat,     c_do_nothing },
	{ "embed",        CMD_IN_NODE,                   c_compat,     c_do_nothing },
	{ "proportional", CMD_IN_NODE,                   c_compat,     c_do_nothing }
};
#define NUM_HCP_COMMANDS (sizeof(hcp_commands) / sizeof(hcp_commands[0]))

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Compiler phases
 * -----------------------------------
 */

static gboolean build_index_table(hcp_vars *vars)
{
	hyp_nodenr i, j;
	size_t namelen;
	char *str;
	NODEITEM *node;
	LABEL *lab;
	INDEX_ENTRY *entry;
	
	UNUSED(vars);
	vars->hyp->itable_size = 0;

	/*
	 * calculate size of Index table, which includes the structures
	 * and the strings. Will also convert all node names to target format.
	 */
	vars->hyp->num_index = vars->p1_node_counter;
	vars->hyp->indextable = g_new0(INDEX_ENTRY *, vars->p2_real_external_node_counter + 1);
	if (G_UNLIKELY(vars->hyp->indextable == NULL))
	{
		oom(vars);
		return FALSE;
	}
	for (i = 0; i < vars->p1_node_counter; i++)
	{
		node = vars->node_table[i];
		lab = vars->label_table[node->labindex];
		str = hyp_utf8_to_charset(vars->hyp->comp_charset, lab->name, STR0TERM, &vars->global_converror);
		warn_converror(vars, &node->source_location, 0);
		namelen = strlen(str);
		if (namelen > HYP_NODENAME_MAX)
		{
			namelen = HYP_NODENAME_MAX;
			str[namelen] = '\0';
		}
		namelen++;
		if (namelen & 1)
			namelen++;
		entry = (INDEX_ENTRY *)g_malloc(sizeof(INDEX_ENTRY) + namelen);
		if (G_UNLIKELY(entry == NULL))
		{
			oom(vars);
			return FALSE;
		}
		vars->hyp->indextable[i] = entry;
		strcpy((char *) entry->name, str);
		entry->name[namelen - 1] = '\0';
		namelen += SIZEOF_INDEX_ENTRY;
		vars->hyp->itable_size += namelen;
		entry->length = (unsigned char)namelen;
		entry->type = node->is_popup ? HYP_NODE_POPUP : HYP_NODE_INTERNAL;
		entry->seek_offset = 0; /* not yet known */
		entry->comp_diff = 0; /* not yet known */
		entry->next = node->next_index;
		entry->previous = node->prev_index;
		entry->toc_index = node->toc_index;
		g_free(str);
	}
	
	j = vars->p1_node_counter;
	for (i = 0; i < vars->p1_external_node_counter; i++)
	{
		entry = vars->extern_table[i];
		switch (entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			/* resolved to internal page, do not write */
			continue;
		case HYP_NODE_XLINK_RSC:
			/* unresolved resource link. no external node written */
			continue;
		case HYP_NODE_XREF:
			/* unresolved xref. no external node written */
			continue;
		case HYP_NODE_EXTERNAL_REF:
			if (entry->extern_labindex == HYP_NOINDEX || entry->extern_nodeindex != HYP_NOINDEX)
				continue;
			break;
		case HYP_NODE_IMAGE:
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
			if (entry->extern_nodeindex != HYP_NOINDEX)
				continue;
			break;
		case HYP_NODE_EOF:
			break;
		default:
			unreachable();
			break;
		}
		str = hyp_utf8_to_charset(vars->hyp->comp_charset, entry->name, STR0TERM, &vars->global_converror);
		/* FIXME: cant warn about converror here, at least not with a location that makes sense */
		namelen = strlen(str);
		if (namelen > HYP_NODENAME_MAX)
		{
			namelen = HYP_NODENAME_MAX;
			str[namelen] = '\0';
		}
		/* copy back in place, the result string will never be longer */
		if (namelen != 0)
		{
			strcpy((char *)entry->name, str);
			namelen++;
			if (namelen & 1)
				entry->name[namelen++] = '\0';
		}
		g_free(str);
		namelen += SIZEOF_INDEX_ENTRY;
		vars->hyp->itable_size += namelen;
		entry->length = (unsigned char)namelen;
		entry->seek_offset = 0; /* not yet known */
		entry->comp_diff = 0; /* not yet known */
		vars->hyp->indextable[j] = entry;
		j++;
	}
	ASSERT(j == vars->p2_real_external_node_counter);
	vars->hyp->num_index = j;
	
	/*
	 * allocate the entry after the last index entry
	 * that tells the seek_offset just beyond the last node
	 */
	entry = g_new0(INDEX_ENTRY, 1);
	if (G_UNLIKELY(entry == NULL))
	{
		oom(vars);
		return FALSE;
	}
	vars->hyp->indextable[vars->hyp->num_index] = entry;
	
	return TRUE;
}
	
/* ------------------------------------------------------------------------- */

static gboolean hcp_tree_alloc(hcp_vars *vars)
{
	hyp_nodenr node;
	size_t bitlen;
	long titlelen = 0;
	NODEITEM *nodeptr;
	HYP_DOCUMENT *hyp = vars->hyp;
	
	if (hyp->hyptree_data != NULL)
	{
		HYP_DBG(("hyptree already allocated"));
		return FALSE;
	}
	hyp->first_text_page = hyp_first_text_page(hyp);
	hyp->last_text_page = hyp_last_text_page(hyp);
	bitlen = SIZEOF_LONG + (((hyp->last_text_page + 16u) >> 4) << 1);

	hyp->hyptree_data = g_new0(unsigned char, bitlen);
	if (hyp->hyptree_data == NULL)
	{
		oom(vars);
		return FALSE;
	}
	hyp->hyptree_len = bitlen;
	
	for (node = 0; node < vars->p1_node_counter; node++)
	{
		nodeptr = vars->node_table[node];
		if (nodeptr->window_title)
		{
			titlelen += target_strlen(vars, nodeptr->window_title) + 1;
			hyp_tree_setbit(hyp, node);
		}
	}
	long_to_chars(titlelen, hyp->hyptree_data);
	
	/*
	 * do not write the bit table when there are no titlss
	 */
	if (titlelen == 0)
		hyp->hyptree_len = SIZEOF_LONG;
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean finish_pass1(hcp_vars *vars)
{
	NODEITEM *node, *link;
	hyp_nodenr i, j;
	INDEX_ENTRY *entry;
	HYP_DOCUMENT *hyp = vars->hyp;
	
	if (vars->p1_node_counter == 0)
	{
		hcp_error(vars, NULL, _("no nodes found"));
		return FALSE;
	}
	
	memset(&vars->stats, 0, sizeof(vars->stats));
	
	/*
	 * resolve global commands: @default, @help
	 */
	if (hyp->default_name)
	{
		node = find_node(vars, hyp->default_name);
		if (node == NULL)
		{
			hcp_warning(vars, NULL, _("%s node-name not found: %s"), "@default", hyp->default_name);
		} else if (node->is_popup)
		{
			hcp_error(vars, &node->source_location, _("%s node %s must not be popup-node"), "@default", hyp->default_name);
		} else
		{
			hyp->default_page = node->node_index;
		}
	}
	if (hyp->help_name)
	{
		node = find_node(vars, hyp->help_name);
		if (node == NULL)
		{
			hcp_warning(vars, NULL, _("%s node-name not found: %s"), "@help", hyp->help_name);
		} else if (node->is_popup)
		{
			hcp_error(vars, &node->source_location, _("%s node %s must not be popup-node"), "@help", hyp->help_name);
		} else
		{
			hyp->help_page = node->node_index;
		}
	}
	if (!hypnode_valid(hyp, hyp->main_page))
	{
		node = find_node(vars, hyp_default_main_node_name);
		if (node == NULL)
		{
			hcp_warning(vars, NULL, _("There is no node with name '%s'"), hyp_default_main_node_name);
		} else
		{
			hyp->main_page = node->node_index;
		}
	}
	
	/*
	 * add @index node if neccessary
	 */
	node = find_node(vars, hyp_default_index_node_name);
	if (node != NULL)
	{
		if (node->is_popup)
			hcp_error(vars, &node->source_location, _("%s node %s must not be popup-node"), "@index", hyp_default_index_node_name);
		hyp->index_page = node->node_index;
		vars->gen_index = FALSE;
	} else if (vars->gen_index)
	{
		node = add_node(vars, hyp_default_index_node_name);
		if (G_UNLIKELY(node == NULL))
			return FALSE;
		hyp->index_page = vars->p1_node_counter;
		vars->p1_node_counter++;
		vars->label_table[node->labindex]->referenced = TRUE;
	} else
	{
		hyp->index_page = HYP_NOINDEX;
	}
	
	/*
	 * resolve node links: @toc, @next, @prev
	 */
	for (i = 0; i < vars->p1_node_counter; i++)
	{
		TREEDEF *tree;
		
		node = vars->node_table[i];
		/*
		 * for AmigaGuide @next/@prev/@toc may specify external files
		 */
		node->next_index = i + 1;
		while (node->next_index < vars->p1_node_counter && vars->node_table[node->next_index]->is_popup)
			node->next_index++;
		if (node->next_index >= vars->p1_node_counter)
			node->next_index = i;
		if (node->next_name)
		{
			link = find_node(vars, node->next_name);
			if (link == NULL)
			{
				hcp_warning(vars, &node->source_location, _("%s node-name not found: %s"), "@next", node->next_name);
			} else
			{
				if (link->is_popup)
					hcp_warning(vars, &node->source_location, _("%s-link in %s is a popup"), "@next", vars->label_table[node->labindex]->name);
				node->next_index = link->node_index;
			}
		}
		
		node->prev_index = i == 0 ? i : i - 1;
		while (node->prev_index > 0 && vars->node_table[node->prev_index]->is_popup)
			node->prev_index--;
		if (node->prev_index == 0 && i != 1)
			node->prev_index = i;
		if (node->prev_name)
		{
			link = find_node(vars, node->prev_name);
			if (link == NULL)
			{
				hcp_warning(vars, &node->source_location, _("%s node-name not found: %s"), "@prev", node->prev_name);
			} else
			{
				if (link->is_popup)
					hcp_warning(vars, &node->source_location, _("%s-link in %s is a popup"), "@prev", vars->label_table[node->labindex]->name);
				node->prev_index = link->node_index;
			}
		}
		
		node->toc_index = 0;
		if (node->toc_name)
		{
			link = find_node(vars, node->toc_name);
			if (link == NULL)
			{
				hcp_warning(vars, &node->source_location, _("%s node-name not found: %s"), "@toc", node->toc_name);
			} else
			{
				if (link->is_popup)
					hcp_warning(vars, &node->source_location, _("%s-link in %s is a popup"), "@toc", vars->label_table[node->labindex]->name);
				node->toc_index = link->node_index;
			}
		}
		
		if (node->is_popup)
			vars->stats.other_nodes++;
		else
			vars->stats.internal_nodes++;
		
		/*
		 * resolve tree references
		 */
		for (tree = node->objects; tree != NULL; tree = tree->next)
		{
			NODEITEM *target = find_node(vars, tree->name);
			LABIDX idx;
			LABEL *lab;
			
			if (target == NULL)
			{
				hcp_warning(vars, &tree->loc, _("node-name %s for tree not found (object ignored)"), tree->name);
			} else if (target->is_popup)
			{
				hcp_warning(vars, &tree->loc, _("node %s for object-tree must not be popup-node (object ignored)"), tree->name);
			} else
			{
				tree->node = target->node_index;
				idx = tree->lab;
				if (idx != HYP_NOINDEX)
				{
					lab = find_node_label(vars, vars->label_table[idx]->name, tree->node);
					if (lab == NULL)
					{
						hcp_warning(vars, &tree->loc,
							_("label '%s' not found in node %s"),
							vars->label_table[idx]->name,
							tree->name);
						tree->lineno = 0;
					} else
					{
						tree->lineno = lab->lineno;
					}
				}
			}
		}
	}
	
	/*
	 * resolve explicit links & xrefs.
	 */
	vars->p2_real_external_node_counter = vars->p1_node_counter;
	for (i = 0; i < vars->p1_external_node_counter; i++)
	{
		entry = vars->extern_table[i];
		
		switch (entry->type)
		{
		case HYP_NODE_XLINK:
			link = find_node(vars, (const char *) entry->name);
			if (link != NULL)
			{
				LABEL *lab;
				LABIDX idx;
				
				/* found. no index entry written */
				
				/* fetch label name that was used */
				idx = entry->xlink_linenolab;
				
				/* update target link */
				entry->xlink_target = link->node_index;
				if (idx != HYP_NOINDEX)
				{
					lab = vars->label_table[idx];
					lab->referenced = TRUE;
					
					if (lab->lineno == HYP_NOINDEX)
					{
						/*
						 * "text" link <node> <label> was used,
						 */
						const char *name = lab->name;
						lab = find_node_label(vars, name, link->node_index);
						if (lab == NULL)
							lab = find_label(vars, name, l_alias);
						if (lab == NULL)
						{
							NODEITEM *target = find_node(vars, name);
							if (target != NULL && target->node_index == link->node_index)
								lab = vars->label_table[target->labindex];
						}
						if (lab == NULL)
						{
							hcp_error(vars, &vars->label_table[idx]->source_location,
								_("label '%s' not found in node %s"),
								vars->label_table[idx]->name,
								entry->name);
							entry->xlink_lineno = 0;
						} else
						{
							entry->xlink_lineno = lab->lineno;
							lab->referenced = TRUE;
						}
					} else
					{
						/*
						 * "text" link <node> <lineno> was used,
						 */
						if (entry->xlink_lineno > link->page_lines)
							hcp_warning(vars, &lab->source_location,
								_("line number %u exceeds page size %u in %s"),
								entry->xlink_lineno,
								link->page_lines,
								entry->name);
					}
				} else
				{
					/*
					 * no lineno was used,
					 */
					entry->xlink_lineno = 0;
				}
				entry->type = link->is_popup ? HYP_NODE_POPUP : HYP_NODE_INTERNAL;
			} else
			{
				/* not found. turn into external reference */
				entry->type = HYP_NODE_EXTERNAL_REF;
				entry->extern_labindex = 0;
				ASSERT(entry->extern_nodeindex == HYP_NOINDEX);
				for (j = 0; j < i; j++)
					if (vars->extern_table[j]->type == HYP_NODE_EXTERNAL_REF && strcmp((const char *) vars->extern_table[j]->name, (const char *) entry->name) == 0)
						break;
				if (j < i)
				{
					if (vars->extern_table[j]->xlink_target == HYP_NOINDEX)
					{
						vars->extern_table[j]->xlink_target = vars->p2_real_external_node_counter;
						vars->p2_real_external_node_counter++;
					} else
					{
						entry->extern_nodeindex = j;
					}
					entry->xlink_target = vars->extern_table[j]->xlink_target;
				} else
				{
					entry->xlink_target = vars->p2_real_external_node_counter;
					vars->p2_real_external_node_counter++;
					vars->stats.external_nodes++;
				}
			}
			break;

		case HYP_NODE_XREF:
			link = find_node(vars, (const char *) entry->name);
			{
				int count;
				XREF_ITEM *x;
				
				/* this was from @xref */
				node = vars->node_table[entry->xref_node_index];
				count = entry->xref_index;
				entry->xref_node_index = HYP_NOINDEX;
				entry->xref_index = HYP_NOINDEX;
				x = node->xrefs;
				while (x && count)
				{
					count--;
					x = x->next;
				}
				if (x == NULL)
				{
					/* something went wrong */
					HYP_DBG(("@xref entry %s #%d not found in node %u", entry->name, entry->xref_index, entry->xref_node_index));
				} else
				{
					if (link)
					{
						x->target = link->node_index;
						entry->type = link->is_popup ? HYP_NODE_POPUP : HYP_NODE_INTERNAL;
					} else
					{
						x->target = vars->p2_real_external_node_counter;
						entry->type = HYP_NODE_EXTERNAL_REF;
						entry->extern_labindex = 0;
						ASSERT(entry->extern_nodeindex == HYP_NOINDEX);
						for (j = 0; j < i; j++)
							if (vars->extern_table[j]->type == HYP_NODE_EXTERNAL_REF && strcmp((const char *) vars->extern_table[j]->name, (const char *) entry->name) == 0)
								break;
						if (j < i)
						{
							if (vars->extern_table[j]->xlink_target == HYP_NOINDEX)
							{
								vars->extern_table[j]->xlink_target = vars->p2_real_external_node_counter;
								vars->p2_real_external_node_counter++;
							} else
							{
								entry->extern_nodeindex = j;
							}
							entry->xlink_target = vars->extern_table[j]->xlink_target;
							x->target = j;
						} else
						{
							entry->xlink_target = vars->p2_real_external_node_counter;
							vars->p2_real_external_node_counter++;
							vars->stats.external_nodes++;
						}
					}
				}	
			}
			break;

		case HYP_NODE_XLINK_RSC:
			if (entry->xlink_rsc_treenr == HYP_NOINDEX)
			{
				/* unresolved resource link. no index entry written */
			} else
			{
				entry->type = HYP_NODE_EXTERNAL_REF;
				entry->extern_labindex = 0;
				vars->p2_real_external_node_counter++;
				vars->stats.external_nodes++;
			}
			break;

		case HYP_NODE_EXTERNAL_REF:
			{
				LABIDX idx;
				LABEL *lab;
				
				idx = entry->extern_labindex;
				if (idx != HYP_NOINDEX)
				{
					lab = vars->label_table[idx];
					if (lab->referenced)
					{
						/* prefer internal nodename if we have one */
						if (find_node(vars, lab->name) != NULL ||
							find_label(vars, lab->name, l_label) != NULL)
						{
							lab->add_to_autoref = FALSE;
						}
					}
					if (lab->referenced && lab->add_to_autoref)
					{
						/* auto-referenced @extern entry; update index */
						lab->extern_index = vars->p2_real_external_node_counter;
						vars->p2_real_external_node_counter++;
						vars->stats.external_nodes++;
					} else
					{
						lab->extern_index = HYP_NOINDEX;
						entry->extern_labindex = HYP_NOINDEX;
						lab->add_to_autoref = FALSE;
					}
				}
			}
			break;

		case HYP_NODE_IMAGE:
			entry->pic_node_index = vars->p2_real_external_node_counter;
			vars->p2_real_external_node_counter++;
			vars->stats.image_count++;
			break;
		
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
			for (j = 0; j < i; j++)
				if (vars->extern_table[j]->type == entry->type && strcmp((const char *) vars->extern_table[j]->name, (const char *) entry->name) == 0)
					break;
			if (j < i)
			{
				entry->extern_nodeindex = j;
				entry->xlink_target = vars->extern_table[j]->xlink_target;
			} else
			{
				entry->xlink_target = vars->p2_real_external_node_counter;
				vars->p2_real_external_node_counter++;
				vars->stats.other_nodes++;
			}
			break;
		case HYP_NODE_EOF:
			break;
		
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
		default:
			unreachable();
			break;
		}
	}
	
	/*
	 * For some unknown reason, ST-Guide sometimes seems to need
	 * an additional EOF entry in the index table
	 */
	/* if (vars->p2_real_external_node_counter == vars->p1_node_counter) */
	{
		entry = add_external_node(vars, HYP_NODE_EOF, "");
		if (G_UNLIKELY(entry == NULL))
			return FALSE;
		vars->p2_real_external_node_counter++;
		vars->need_eof_entry = TRUE;
	}
	 
	/*
	 * now that we know the size, build the complete index table
	 */
	if (build_index_table(vars) == FALSE)
		return FALSE;
	if (!(vars->hyp->st_guide_flags & STG_ENCRYPTED))
		if (hcp_tree_alloc(vars) == FALSE)
			return FALSE;
	
	/*
	 * init counters for pass2
	 */
	vars->p2_real_external_node_counter = vars->p1_node_counter;
	vars->p2_external_node_counter = 0;

	/*
	 * preallocate page buffer
	 */
	if (vars->page_max_used > 0)
	{
		vars->page_buf = g_new(unsigned char, vars->page_max_used);
		if (vars->page_buf == NULL)
		{
			oom(vars);
			return FALSE;
		}
		vars->page_alloced = vars->page_max_used;
	}
	
	/*
	 * build the lookup table for the auto-referencer
	 */
	kwsfree(vars->autorefs);
	vars->autorefs = kwsalloc(NULL);
	if (vars->autorefs == NULL)
		return FALSE;
	{
		LABIDX l;
		LABEL *lab;
		
		for (l = 0; l < vars->p1_lab_counter; l++)
		{
			lab = vars->label_table[l];
			if (lab->add_to_autoref)
				if (kwsincr(vars->autorefs, lab->name, strlen(lab->name), lab) == FALSE)
				{
					oom(vars);
					return FALSE;
				}
		}
		if (kwsprep(vars->autorefs) == FALSE)
			return FALSE;
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static int addlink(hcp_vars *vars, unsigned char type, const char *text, const unsigned char *name, hyp_nodenr target, hyp_lineno lineno)
{
	size_t len;

	if (empty(text) && name != NULL)
	{
		len = vars->hcp_pass == 1 ? target_strlen(vars, (const char *) name) : ustrlen(name);
	} else
	{
		len = target_strlen(vars, text);
		if (len > HYP_LINKTEXT_MAX)
		{
			if (vars->hcp_pass == 1)
				hcp_warning(vars, NULL, _("%s longer than %d chars, truncated"), _("text"), HYP_LINKTEXT_MAX);
			len = HYP_LINKTEXT_MAX;
		}
	}
	if (name != NULL /* && target != HYP_NOINDEX */)
	{
		ASSERT(target != HYP_NOINDEX);
		addbyte(vars, HYP_ESC);
		addbyte(vars, type);
		if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
			addenc255(vars, lineno);
		addenc255(vars, target);
	}
	if (empty(text) && name != NULL)
	{
		addbyte(vars, HYP_STRLEN_OFFSET);
		/* no text output, dest name will be used */
	} else
	{
		if (vars->hcp_pass == 1)
		{
			/* just leave room for text */
			vars->page_used += 1 + len;
		} else
		{
			if (name != NULL)
				addbyte(vars, HYP_STRLEN_OFFSET + len);
			addtext(vars, text, STR0TERM, len);
		}
	}
	return (int) len;
}

/* ------------------------------------------------------------------------- */

#define N_INDEX_CHARS 27

#include "sorttab.h"

static int index_char(const char *name)
{
	h_unichar_t wc;
	size_t start, end, half;
	
	if (name[0] >= 'a' && name[0] <= 'z')
		return name[0] - 'a';
	if (name[0] >= 'A' && name[0] <= 'Z')
		return name[0] - 'A';

	/*
	 * use unicode table to get index for umlauts
	 */
	wc = hyp_utf8_get_char(name);
	start = 0;
	end = sizeof(sorttable) / sizeof(sorttable[0]);
	if (wc >= sorttable[start].wc && wc <= sorttable[end - 1].wc)
	{
		for (;;)
		{
			half = (start + end) / 2;
	
			if (wc == sorttable[half].wc)
				return sorttable[half].c - 'A';
			else if (half == start)
				break;
			else if (wc > sorttable[half].wc)
				start = half;
			else
				end = half;
		}
	}

	return N_INDEX_CHARS - 1;
}


static int comp_lab(const void *_l1, const void *_l2)
{
	const LABEL *l1 = *((const LABEL *const *)_l1);
	const LABEL *l2 = *((const LABEL *const *)_l2);
	/* FIXME: this should actually be something like strcoll() */
	return hyp_utf8_strcasecmp(l1->name, l2->name);
}


static gboolean autogen_index(hcp_vars *vars)
{
	static char const index_chars[N_INDEX_CHARS] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', '*'
	};
	size_t used[N_INDEX_CHARS];
	LABIDX i;
	int c, c2;
	LABEL *lab;
	size_t len;
	gboolean first;
	int maxlen;
	int columns;
	int column;
	gboolean retval = TRUE;
	HYP_DOCUMENT *hyp = vars->hyp;
	
	NODEITEM *node = vars->node_table[vars->p2_node_counter];
	
	hcp_status_node(vars, vars->label_table[node->labindex]->name);
	memset(used, 0, sizeof(used));
	vars->page_used = 0;
	maxlen = 0;
	for (i = 0; i < vars->p1_lab_counter; i++)
	{
		if (i == node->labindex)
			continue;
		lab = vars->label_table[i];
		if (lab && lab->add_to_index && lab->node_index != HYP_NOINDEX &&
			(lab->type == l_node || lab->type == l_alias || lab->type == l_label || lab->type == l_index))
		{
			c = index_char(lab->name);
			len = g_utf8_str_len(lab->name, STR0TERM);
			if ((int) len > maxlen)
				maxlen = (int)len;
			used[c]++;
			lab->referenced = TRUE;
		}
	}
	maxlen += 4;
	if (vars->opts->index_width != 0 && vars->opts->index_width > (int)maxlen)
		maxlen = vars->opts->index_width;
	if (maxlen < hyp->line_width)
		columns = (hyp->line_width + 2) / (maxlen + 2);
	else
		columns = 1;

	if (maxlen == 0)
	{
		hcp_warning(vars, NULL, _("empty index page"));
	} else
	{
		vars->in_node = 1;
		first = TRUE;
		for (c = 0; c < N_INDEX_CHARS; c++)
		{
			if (used[c] != 0)
			{
				LABEL **labs;
				size_t count = 0;
				
				labs = g_new(LABEL *, used[c]);
				if (G_UNLIKELY(labs == NULL))
				{
					oom(vars);
					return FALSE;
				}
				for (i = 0; i < vars->p1_lab_counter; i++)
				{
					if (i == node->labindex)
						continue;
					lab = vars->label_table[i];
					if (lab && lab->add_to_index && lab->node_index != HYP_NOINDEX &&
						(lab->type == l_node || lab->type == l_alias || lab->type == l_label || lab->type == l_index))
					{
						c2 = index_char(lab->name);
						if (c == c2)
						{
							labs[count] = lab;
							count++;
						}
					}
				}
				ASSERT(count == used[c]);
				qsort(labs, count, sizeof(LABEL *), comp_lab);
				
				if (!first)
					addbyte(vars, HYP_EOL);
				first = FALSE;
				column = 0;
				for (i = 0; i < count; i++)
				{
					const char *text;
					unsigned char type;
					hyp_nodenr target;
					NODEITEM *dest;
					hyp_lineno lineno = 0;
					
					lab = labs[i];
					if (column == 0)
					{
						addbyte(vars, ' ');
						if (i == 0)
						{
							addbyte(vars, index_chars[c]);
						} else
						{
							addbyte(vars, ' ');
						}
					}
					addbyte(vars, ' ');
					addbyte(vars, ' ');
					type = HYP_ESC_LINK;
					target = lab->node_index;
					dest = vars->node_table[target];
					
					if ((lab->type == l_node || (lab->type == l_alias && lab->lineno == 1)) &&
						namecmp(lab->name, vars->label_table[dest->labindex]->name) == 0)
					{
						text = NULL;
					} else
					{
						text = lab->name;
					}
					if ((lab->type == l_alias || lab->type == l_index || lab->type == l_label) &&
						lab->lineno > 1 && lab->lineno <= HYP_LINENO_MAX)
					{
						type = HYP_ESC_LINK_LINE;
						lineno = lab->lineno - 1;
					}
					addlink(vars, type, text, hyp->indextable[target]->name, target, lineno);
					++column;
					if (column == columns)
					{
						addbyte(vars, HYP_EOL);
						column = 0;
					} else if (column != 0)
					{
						len = g_utf8_str_len(lab->name, STR0TERM);
						while ((int)len < maxlen)
						{
							addbyte(vars, ' ');
							len++;
						}
					}
				}
				if (column != 0)
					addbyte(vars, HYP_EOL);
				g_free(labs);
			}
		}
	}
	if (finish_page(vars, vars->p2_node_counter) == FALSE)
		retval = FALSE;
	vars->p2_node_counter++;
	vars->in_node = FALSE;
	vars->page_used = 0;
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean finish_pass2(hcp_vars *vars)
{
	if (vars->gen_index)
	{
		ASSERT(vars->p2_node_counter == vars->hyp->index_page);
		if (autogen_index(vars) == FALSE)
			return FALSE;
	}
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------
 * Inline commands
 * -----------------------------------
 */

/*
 * @{ <attributes> }
 */

#define c_inline_attr_on(mask, ch) \
	if (oldattr & mask) \
	{ \
		if (vars->hcp_pass == 1) \
			hcp_warning(vars, NULL, _("already active: %c"), ch); \
	} else \
	{ \
		newattr |= mask; \
	}
#define c_inline_attr_off(mask, ch) \
	if (!(oldattr & mask)) \
	{ \
		if (vars->hcp_pass == 1) \
			hcp_warning(vars, NULL, _("not active: %c"), ch); \
	} else \
	{ \
		newattr &= ~mask; \
	}
	
static int c_inline_attr(hcp_vars *vars, const char *line)
{
	unsigned char oldattr, newattr;
	
	oldattr = newattr = vars->textattr;

	while (*line)
	{
		switch (*line)
		{
		case 'b':
			c_inline_attr_off(HYP_TXT_BOLD, 'B');
			break;
		case 'B':
			c_inline_attr_on(HYP_TXT_BOLD, 'B');
			break;
		case 'g':
			c_inline_attr_off(HYP_TXT_LIGHT, 'G');
			break;
		case 'G':
			c_inline_attr_on(HYP_TXT_LIGHT, 'G');
			break;
		case 'i':
			c_inline_attr_off(HYP_TXT_ITALIC, 'I');
			break;
		case 'I':
			c_inline_attr_on(HYP_TXT_ITALIC, 'I');
			break;
		case 'u':
			c_inline_attr_off(HYP_TXT_UNDERLINED, 'U');
			break;
		case 'U':
			c_inline_attr_on(HYP_TXT_UNDERLINED, 'U');
			break;
		case 'o':
			c_inline_attr_off(HYP_TXT_OUTLINED, 'O');
			break;
		case 'O':
			c_inline_attr_on(HYP_TXT_OUTLINED, 'O');
			break;
		case 's':
			c_inline_attr_off(HYP_TXT_SHADOWED, 'S');
			break;
		case 'S':
			c_inline_attr_on(HYP_TXT_SHADOWED, 'S');
			break;
		case '0':
			c_inline_attr_off(oldattr, '0');
			break;
		default:
			if (vars->hcp_pass == 1)
				hcp_warning(vars, NULL, _("illegal textattribute %c"), *line);
			break;
		}
		line++;
	}
	if (oldattr != newattr)
	{
		addbyte(vars, HYP_ESC);
		addbyte(vars, HYP_ESC_TEXTATTR_FIRST + newattr);
		vars->textattr = newattr;
	}
	return 0;
}

static int c_inline_attr_onoff(hcp_vars *vars, unsigned char attr, unsigned char ch, gboolean on)
{
	unsigned char oldattr, newattr;
	
	oldattr = newattr = vars->textattr;
	if (on)
	{
		c_inline_attr_on(attr, ch);
	} else
	{
		c_inline_attr_off(attr, ch);
	}
	if (oldattr != newattr)
	{
		addbyte(vars, HYP_ESC);
		addbyte(vars, HYP_ESC_TEXTATTR_FIRST + newattr);
		vars->textattr = newattr;
	}
	return 0;
}

/* ------------------------------------------------------------------------- */

#if 0
static gboolean is_external_link(const char *link)
{
	return !empty(link) && (strchr(link, '/') != NULL || strchr(link, '\\') != NULL);
}
#endif

/* ------------------------------------------------------------------------- */

/*
 * @{ "text" link <name> [<line>] }
 * @{ "text" alink <name> [<line>] }
 */
static int c_inline_link(hcp_vars *vars, int argc, char **argv, gboolean alink)
{
	INDEX_ENTRY *entry;
	int len = 0;
	hyp_nodenr target;
	hyp_lineno lineno = HYP_NOINDEX;
	unsigned char type;
	unsigned long val;
	gboolean have_lineno = FALSE;
	char *dest;
	char *text;
	gboolean same_node;
	
	if (argc < 3)
	{
		error_missing_args(vars);
		return len;
	}
	if (argc > 4)
		warn_extra_args(vars);
	text = argv[0];
	dest = argv[2];
	if (check_namelen(vars, _("link"), dest, TRUE) == FALSE)
		return len;

	target = vars->hcp_pass == 1 ? vars->p1_node_counter : vars->p2_node_counter;
	same_node = namecmp(dest, vars->label_table[vars->node_table[target]->labindex]->name) == 0;
	if (same_node && argc == 3)
	{
		if (vars->hcp_pass == 1)
			hcp_warning(vars, NULL, _("link to current node"));
		return addtext(vars, argv[0], STR0TERM, STR0TERM);
	}
	if (vars->hcp_pass == 1)
	{
		char *p;
		hyp_nodenr treenr;
		char *tmp;
		gboolean is_resource = FALSE;
		RSC_DEFINE *define;
		
		target = vars->p1_external_node_counter;

		entry = add_external_node(vars, (hyp_indextype)HYP_NODE_XLINK, dest);
		if (G_UNLIKELY(entry == NULL))
			return len;
		/*
		 * lineno will be resolved in finish_pass1(),
		 * since it can also be label name
		 */
		entry->xlink_lineno = lineno;

		/* check for RSC link */
		p = (vars->opts->compat_flags & STG_ALLOW_FOLDERS_IN_XREFS ? strrslash : strslash)(dest);
		if (p != NULL)
		{
			char c = *p;
			*p = '\0';
			if (hyp_guess_filetype(dest) == HYP_FT_RSC)
			{
				entry->type = HYP_NODE_XLINK_RSC;
				define = find_rsc_define(vars, p + 1);
				if (define == NULL || define->value == HYP_NOINDEX)
				{
					hcp_warning(vars, NULL, _("Define %s not found. Resource-Link ignored"), p + 1);
					/*
					 * add it to table anyway, so we can access
					 * the entry and write the link text instead in pass2
					 */
					treenr = HYP_NOINDEX;
				} else
				{
					treenr = define->value;
				}
				tmp = g_strconcat(dest, "/MAIN", NULL);
				g_free(dest);
				dest = argv[2] = tmp;
				entry->xlink_rsc_treenr = treenr;
				is_resource = TRUE;
				have_lineno = TRUE;
				if (argc == 4)
					warn_extra_args(vars);
			} else
			{
				if (c == '\\' && hyp_guess_filetype(dest) != HYP_FT_NONE)
				{
					hcp_warning(vars, NULL, _("Using backward slashes in external references is deprecated"));
					*p = '/';
				} else
				{
					*p = c;
				}
			}
		}

		entry->next = HYP_NOINDEX;
		entry->xlink_linenolab = HYP_NOINDEX;
		
		if (!is_resource && argc > 3)
		{
			LABEL *lab = add_label(vars, argv[3], l_label);
			if (G_UNLIKELY(lab == NULL))
				return len;
			lab->referenced = TRUE;
			
			if (G_UNLIKELY(lab->labindex >= HYP_NODE_MAX))
			{
				hcp_error(vars, NULL, _("too many labels (max. %lu)"), (unsigned long)HYP_NODE_MAX);
				return len;
			}
			entry->xlink_linenolab = lab->labindex;
			lab->node_index = HYP_NOINDEX;
			if (g_is_number(argv[3], TRUE))
			{
				val = strtoul(argv[3], NULL, 0);
				if (val > HYP_LINENO_MAX)
				{
					hcp_error(vars, NULL, _("bad line number"));
					lineno = 0;
				} else
				{
					lineno = (hyp_lineno) val;
				}
				entry->xlink_lineno = lab->lineno = lineno;
			} else
			{
				lab->lineno = HYP_NOINDEX;
			}
			have_lineno = TRUE;
		}
	} else
	{
		target = vars->p2_real_external_node_counter;
		entry = vars->extern_table[vars->p2_external_node_counter];
		vars->p2_external_node_counter++;
		lineno = entry->xlink_lineno;
		have_lineno = lineno != HYP_NOINDEX && lineno > 0;
		if (entry->type == HYP_NODE_INTERNAL || entry->type == HYP_NODE_POPUP)
		{
			/*
			 * link has been resolved internally.
			 * That entry will not be written to the index table
			 */
			target = entry->xlink_target;
		} else if (entry->type == HYP_NODE_EXTERNAL_REF && entry->extern_labindex != HYP_NOINDEX)
		{
			if (entry->extern_nodeindex == HYP_NOINDEX)
				vars->p2_real_external_node_counter++;
			target = entry->xlink_target;
		} else if (entry->type == HYP_NODE_XLINK_RSC)
		{
			/* unresolved link; write text only */
			return addtext(vars, text, STR0TERM, STR0TERM);
		}
	}
	type = have_lineno ? (alink ? HYP_ESC_ALINK_LINE : HYP_ESC_LINK_LINE) : (alink ? HYP_ESC_ALINK : HYP_ESC_LINK);
	return addlink(vars, type, text, entry ? entry->name : NULL, target, lineno > 0 ? lineno - 1 : 0);
}

/* ------------------------------------------------------------------------- */

/*
 * @{ "text" system <command> }
 */
static int c_inline_system(hcp_vars *vars, int argc, char **argv)
{
	INDEX_ENTRY *entry;
	int len = 0;
	hyp_nodenr target;
	
	if (argc < 3)
	{
		error_missing_args(vars);
		return len;
	}
	if (argc > 3)
		warn_extra_args(vars);
	if (check_namelen(vars, _("command"), argv[2], TRUE) == FALSE)
		return len;
	if (vars->hcp_pass == 1)
	{
		target = vars->p1_external_node_counter;
		entry = add_external_node(vars, HYP_NODE_SYSTEM_ARGUMENT, argv[2]);
		if (G_UNLIKELY(entry == NULL))
			return len;
		entry->extern_nodeindex = HYP_NOINDEX;
		entry->previous = HYP_NOINDEX;
	} else
	{
		target = vars->p2_real_external_node_counter;
		entry = vars->extern_table[vars->p2_external_node_counter];
		vars->p2_external_node_counter++;
		if (entry->extern_nodeindex == HYP_NOINDEX)
			vars->p2_real_external_node_counter++;
		target = entry->xlink_target;
	}
	return addlink(vars, HYP_ESC_LINK, argv[0], entry ? entry->name : NULL, target, 0);
}

/* ------------------------------------------------------------------------- */

/*
 * @{ "text" quit }
 */
static int c_inline_quit(hcp_vars *vars, int argc, char **argv)
{
	INDEX_ENTRY *entry;
	int len = 0;
	hyp_nodenr target;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return len;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (vars->hcp_pass == 1)
	{
		target = vars->p1_external_node_counter;
		entry = add_external_node(vars, HYP_NODE_QUIT, "");
		if (G_UNLIKELY(entry == NULL))
			return len;
		entry->extern_nodeindex = HYP_NOINDEX;
		entry->previous = HYP_NOINDEX;
	} else
	{
		target = vars->p2_real_external_node_counter;
		entry = vars->extern_table[vars->p2_external_node_counter];
		vars->p2_external_node_counter++;
		if (entry->extern_nodeindex == HYP_NOINDEX)
			vars->p2_real_external_node_counter++;
		target = entry->xlink_target;
	}
	return addlink(vars, HYP_ESC_LINK, argv[0], entry ? entry->name : NULL, target, 0);
}

/* ------------------------------------------------------------------------- */

/*
 * @{ "text" close }
 */
static int c_inline_close(hcp_vars *vars, int argc, char **argv)
{
	INDEX_ENTRY *entry;
	int len = 0;
	hyp_nodenr target;
	
	if (argc < 2)
	{
		error_missing_args(vars);
		return len;
	}
	if (argc > 2)
		warn_extra_args(vars);
	if (vars->hcp_pass == 1)
	{
		target = vars->p1_external_node_counter;
		entry = add_external_node(vars, HYP_NODE_CLOSE, "");
		if (G_UNLIKELY(entry == NULL))
			return len;
		entry->extern_nodeindex = HYP_NOINDEX;
		entry->previous = HYP_NOINDEX;
	} else
	{
		target = vars->p2_real_external_node_counter;
		entry = vars->extern_table[vars->p2_external_node_counter];
		vars->p2_external_node_counter++;
		if (entry->extern_nodeindex == HYP_NOINDEX)
			vars->p2_real_external_node_counter++;
		target = entry->xlink_target;
	}
	return addlink(vars, HYP_ESC_LINK, argv[0], entry ? entry->name : NULL, target, 0);
}

/* ------------------------------------------------------------------------- */

/*
 * @{ "text" rx <command> }
 * @{ "text" rxs <command> }
 */
static int c_inline_rexx(hcp_vars *vars, int argc, char **argv, gboolean script)
{
	INDEX_ENTRY *entry;
	int len = 0;
	hyp_nodenr target;
	
	if (argc < 3)
	{
		error_missing_args(vars);
		return len;
	}
	if (argc > 3)
		warn_extra_args(vars);
	if (check_namelen(vars, _("command"), argv[2], TRUE) == FALSE)
		return len;
	if (vars->hcp_pass == 1)
	{
		target = vars->p1_external_node_counter;
		entry = add_external_node(vars, script ? HYP_NODE_REXX_SCRIPT : HYP_NODE_REXX_COMMAND, argv[2]);
		if (G_UNLIKELY(entry == NULL))
			return len;
		entry->extern_nodeindex = HYP_NOINDEX;
		entry->previous = HYP_NOINDEX;
	} else
	{
		target = vars->p2_real_external_node_counter;
		entry = vars->extern_table[vars->p2_external_node_counter];
		vars->p2_external_node_counter++;
		if (entry->extern_nodeindex == HYP_NOINDEX)
			vars->p2_real_external_node_counter++;
		target = entry->xlink_target;
	}
	return addlink(vars, HYP_ESC_LINK, argv[0], entry ? entry->name : NULL, target, 0);
}

/* ------------------------------------------------------------------------- */

/*
 * @{ "text" ignore }
 */
static int c_inline_ignore(hcp_vars *vars, int argc, char **argv)
{
	if (argc < 2)
	{
		error_missing_args(vars);
		return 0;
	}
	if (argc > 2)
		warn_extra_args(vars);
	return addtext(vars, argv[0], STR0TERM, STR0TERM);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------------
 * Resource lines between @tree and @endtree
 * -----------------------------------------
 */

/* ------------------------------------------------------------------------- */

/*
 * <object-no> <nodename> [lineno]
 */
static void process_treeline(hcp_vars *vars, const char *line)
{
	int argc;
	char **argv;
	gboolean first_arg_quoted;
	char *end;
	RSC_DEFINE *define;
	unsigned short value;
	hyp_lineno lineno = 0;
	LABIDX idx = HYP_NOINDEX;
	TREEDEF *tree, **last;
	size_t len;
	NODEITEM *node;
	
	if (vars->hcp_pass == 2)
		return;
	argc = parse_args(vars, line, '\0', &argv, &end, &first_arg_quoted);
	if (argc < 2)
	{
		error_missing_args(vars);
		g_strfreev(argv);
		return;
	}
	if (argc > 3)
		warn_extra_args(vars);
	define = find_rsc_define(vars, argv[0]);
	if (define)
	{
		value = define->value;
	} else if (g_is_number(argv[0], TRUE))
	{
		value = strtoul(argv[0], NULL, 0);
	} else
	{
		hcp_warning(vars, NULL, _("Define %s not found (object ignored)"), argv[0]);
		g_strfreev(argv);
		return;
	}
	if (argc > 2)
	{
		if (g_is_number(argv[2], TRUE))
		{
			lineno = strtoul(argv[3], NULL, 0);
			if (lineno > HYP_LINENO_MAX || lineno < 1)
			{
				hcp_warning(vars, NULL, _("bad line number"));
				lineno = 0;
			}
		} else
		{
			LABEL *lab = add_label(vars, argv[2], l_label);
			if (G_UNLIKELY(lab == NULL))
				return;
			idx = lab->labindex;
			lab->node_index = HYP_NOINDEX;
		}
	}
	len = strlen(argv[1]);
	tree = (TREEDEF *)g_malloc(sizeof(TREEDEF) + len);
	if (G_UNLIKELY(tree == NULL))
	{
		g_strfreev(argv);
		return;
	}
	strcpy(tree->name, argv[1]);
	tree->next = NULL;
	tree->treenr = vars->cur_rsc_tree_nr;
	tree->objnr = value;
	tree->node = HYP_NOINDEX;
	tree->lineno = lineno;
	tree->lab = idx;
	tree->loc = vars->include_stack->loc;
	node = vars->node_table[vars->p1_node_counter];
	last = &node->objects;
	while (*last)
	{
		last = &(*last)->next;
	}
	*last = tree;
	g_strfreev(argv);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------------
 * Plain text
 * -----------------------------------------
 */

static int do_autoref(hcp_vars *vars, const char *text, size_t len)
{
	struct kwsmatch match;
	int columns = 0;
	hyp_nodenr target;
	LABEL *lab;
	
	if (len == 0)
		return 0;
	if (vars->autoreferences <= 0)
		return addtext(vars, text, len, STR0TERM);

	if (vars->hcp_pass == 1)
	{
		columns += addtext(vars, text, len, STR0TERM);

		/*
		 * if we have an autorefs collection here, they are from
		 * @uses. Check them in pass1 so we know which where referenced.
		 */
		while (len && kwsexec(vars->autorefs, text, len, &match) != (size_t)-1)
		{
			gboolean refit = TRUE;
			unsigned char prevchar, nextchar;
			
			lab = (LABEL *)match.data;

			if (match.offset[0] == 0)
				prevchar = '\0';
			else
				prevchar = text[match.offset[0] - 1];
			if (match.offset[0] + match.size[0] >= len)
				nextchar = '\0';
			else
				nextchar = text[match.offset[0] + match.size[0]];
			
			target = lab->node_index;
			if (!vars->allowed_prev_chars[prevchar] || !vars->allowed_next_chars[nextchar])
			{
				refit = FALSE;
			} else
			{
				if (lab->type == l_extern || lab->type == l_uses)
				{
					target = lab->extern_index;
				}
				if (target == vars->p1_node_counter)
					refit = FALSE;
			}
						
			if (refit)
			{
				lab->referenced = TRUE;
			}
			text += match.offset[0] + match.size[0];
			len -= match.offset[0] + match.size[0];
			ASSERT((ssize_t)len >= 0);
		}
		return columns;
	}
	
	/*
	 * real auto-referencer for pass2
	 */
	while (len && kwsexec(vars->autorefs, text, len, &match) != (size_t)-1)
	{
		unsigned char type;
		NODEITEM *dest;
		hyp_lineno lineno = 0;
		const char *atext;
		gboolean refit = TRUE;
		unsigned char prevchar, nextchar;
		
		lab = (LABEL *)match.data;
		columns += addtext(vars, text, match.offset[0], STR0TERM);

		type = HYP_ESC_LINK;
		target = lab->node_index;
		dest = vars->node_table[target];
		
		if (match.offset[0] == 0)
			prevchar = '\0';
		else
			prevchar = text[match.offset[0] - 1];
		if (match.offset[0] + match.size[0] >= len)
			nextchar = '\0';
		else
			nextchar = text[match.offset[0] + match.size[0]];
		
		if (!vars->allowed_prev_chars[prevchar] || !vars->allowed_next_chars[nextchar])
		{
			refit = FALSE;
		} else
		{
			atext = lab->name;
			if ((lab->type == l_node || (lab->type == l_alias && lab->lineno == 1)) &&
				namecmp(lab->name, vars->label_table[dest->labindex]->name) == 0)
			{
				atext = NULL;
			}
			if ((lab->type == l_alias || lab->type == l_index || lab->type == l_label) &&
				lab->lineno > 1 && lab->lineno <= HYP_LINENO_MAX)
			{
				type = HYP_ESC_LINK_LINE;
				lineno = lab->lineno - 1;
			} else if (lab->type == l_extern || lab->type == l_uses)
			{
				target = lab->extern_index;
				if (lab->lineno != HYP_NOINDEX)
				{
					type = HYP_ESC_LINK_LINE;
					lineno = lab->lineno;
				}
			}
			if (target == vars->p2_node_counter)
				refit = FALSE;
		}
					
		if (refit)
		{
			columns += addlink(vars, type, atext, vars->hyp->indextable[target]->name, target, lineno);
			vars->stats.autorefs++;
			lab->referenced = TRUE;
		} else
		{
			columns += addtext(vars, text + match.offset[0], match.size[0], STR0TERM);
		}
		text += match.offset[0] + match.size[0];
		len -= match.offset[0] + match.size[0];
		ASSERT((ssize_t)len >= 0);
	}
	columns += addtext(vars, text, len, STR0TERM);
	return columns;
}

/* ------------------------------------------------------------------------- */

static gboolean is_unhandled_amguide(const char *cmd)
{
	if (cmdnamecmp(cmd, "fg") == 0 ||
		cmdnamecmp(cmd, "bg") == 0 ||
		cmdnamecmp(cmd, "beep") == 0 ||
		cmdnamecmp(cmd, "guide") == 0 ||
		cmdnamecmp(cmd, "amigaguide") == 0 ||
		cmdnamecmp(cmd, "apen") == 0 ||
		cmdnamecmp(cmd, "body") == 0 ||
		cmdnamecmp(cmd, "bpen") == 0 ||
		cmdnamecmp(cmd, "cleartabs") == 0 ||
		cmdnamecmp(cmd, "code") == 0 ||
		cmdnamecmp(cmd, "jcenter") == 0 ||
		cmdnamecmp(cmd, "jleft") == 0 ||
		cmdnamecmp(cmd, "jright") == 0 ||
		cmdnamecmp(cmd, "lindent") == 0 ||
		cmdnamecmp(cmd, "line") == 0 ||
		cmdnamecmp(cmd, "par") == 0 ||
		cmdnamecmp(cmd, "pard") == 0 ||
		cmdnamecmp(cmd, "pari") == 0 ||
		cmdnamecmp(cmd, "plain") == 0 ||
		cmdnamecmp(cmd, "settabs") == 0 ||
		cmdnamecmp(cmd, "tab") == 0)
		return TRUE;
	return FALSE;
}

static void process_text(hcp_vars *vars, char **linep)
{
	int argc;
	char **argv;
	gboolean first_arg_quoted;
	char *end;
	int column = 0;
	NODEITEM *node;
	char *start_text;
	char *line = *linep;
	
	node = vars->node_table[vars->hcp_pass == 1 ? vars->p1_node_counter : vars->p2_node_counter];
	start_text = line;
	while (*line)
	{
		switch (*line)
		{
		case HYP_ESC:
			column += do_autoref(vars, start_text, line - start_text);
			addbyte(vars, HYP_ESC);
			addbyte(vars, HYP_ESC_ESC);
			line++;
			start_text = line;
			column++;
			break;
		case '@':
			switch (line[1])
			{
			case '@':
				/*
				 * @@ : quoted '@'
				 */
				/*
				 * remove quote from input rather than spitting out text and starting over,
				 * so the auto-referencer works for nodenames with '@' in it
				 */
				memmove(line, line + 1, strlen(line + 1) + 1);
				line++;
				break;
			case '{':
				/*
				 * @{ ... }: inline commands
				 */
				column += do_autoref(vars, start_text, line - start_text);
				argc = parse_args(vars, line + 2, '}', &argv, &end, &first_arg_quoted);
				switch (argc)
				{
				case -1:
					/* error message already printed */
					break;
				case 0:
					hcp_error(vars, NULL, _("missing command"));
					break;
				case 1:
					if (vars->for_amguide)
					{
						if (cmdnamecmp(argv[0], "b") == 0)
							column += c_inline_attr_onoff(vars, HYP_TXT_BOLD, 'B', TRUE);
						else if (cmdnamecmp(argv[0], "ub") == 0)
							column += c_inline_attr_onoff(vars, HYP_TXT_BOLD, 'B', FALSE);
						else if (cmdnamecmp(argv[0], "i") == 0)
							column += c_inline_attr_onoff(vars, HYP_TXT_ITALIC, 'I', TRUE);
						else if (cmdnamecmp(argv[0], "ui") == 0)
							column += c_inline_attr_onoff(vars, HYP_TXT_ITALIC, 'I', FALSE);
						else if (cmdnamecmp(argv[0], "u") == 0)
							column += c_inline_attr_onoff(vars, HYP_TXT_UNDERLINED, 'U', TRUE);
						else if (cmdnamecmp(argv[0], "uu") == 0)
							column += c_inline_attr_onoff(vars, HYP_TXT_UNDERLINED, 'U', FALSE);
						else if (is_unhandled_amguide(argv[0]))
						{
							if (vars->hcp_pass == 1)
								hcp_warning(vars, NULL, _("@{%s} not implemented"), argv[0]);
						} else
						{
							hcp_error(vars, NULL, _("unknown command '%s'"), argv[0]);
						}
					} else
					{
						column += c_inline_attr(vars, argv[0]);
					}
					break;
				default:
					if (cmdnamecmp(argv[1], "link") == 0)
						column += c_inline_link(vars, argc, argv, FALSE);
					else if (cmdnamecmp(argv[1], "alink") == 0)
						column += c_inline_link(vars, argc, argv, TRUE);
					else if (cmdnamecmp(argv[1], "system") == 0)
						column += c_inline_system(vars, argc, argv);
					else if (cmdnamecmp(argv[1], "rx") == 0)
						column += c_inline_rexx(vars, argc, argv, FALSE);
					else if (cmdnamecmp(argv[1], "rxs") == 0)
						column += c_inline_rexx(vars, argc, argv, TRUE);
					else if (cmdnamecmp(argv[1], "quit") == 0)
						column += c_inline_quit(vars, argc, argv);
					else if (cmdnamecmp(argv[1], "close") == 0)
						column += c_inline_close(vars, argc, argv);
					else if (cmdnamecmp(argv[1], "ignore") == 0)
						column += c_inline_ignore(vars, argc, argv);
					else if (vars->for_amguide && is_unhandled_amguide(argv[0]))
					{
						if (vars->hcp_pass == 1)
							hcp_warning(vars, NULL, _("@{%s} not implemented"), argv[0]);
					} else
					{
						hcp_error(vars, NULL, _("unknown command '%s'"), argv[1]);
					}
					break;
				}
				g_strfreev(argv);
				line = end;
				if (*line != '\0')
					line++;
				start_text = line;
				break;
			case ':':
				/*
				 * @: variable reference
				 */
				{
					HCP_DEFINE *d;
					size_t start_text_pos;

					char *name = parse_1arg(vars, line + 2, '\0', &end);
					if (name == NULL)
						break;
					if (empty(name))
					{
						error_missing_args(vars);
						g_free(name);
						line += 2;
						break;
					}
					d = find_hcp_define(vars, name);
					if (d == NULL)
					{
						hcp_error(vars, NULL, _("undefined variable %s"), name);
						g_free(name);
						line = end;
						break;
					}
					g_free(name);
					/*
					 * start_text points into the line buffer,
					 * which might be reallocated by replace_define;
					 * save its offset
					 */
					start_text_pos = start_text - *linep;
					line = replace_define(vars, d, line, end, linep);
					/* restore start_text from offset */
					start_text = *linep + start_text_pos;
				}
				break;
			default:
				/* sole '@' */
				line++;
				break;
			}
			break;
		case '\t':
			/*
			 * <TAB>
			 */
			column += do_autoref(vars, start_text, line - start_text);
			do
			{
				addbyte(vars, ' ');
				++column;
			} while ((column % node->tabwidth) != 0);
			line++;
			start_text = line;
			break;
		case '\\':
			/* AmigaGuide use this for quoting '\\' and '@' */
			if (vars->for_amguide)
			{
				switch (line[1])
				{
				case '@':
				case '\\':
					/*
					 * @@ : quoted '@'
					 */
					/*
					 * remove quote from input rather than spitting out text and starting over,
					 * so the auto-referencer works for nodenames with '@' in it
					 */
					memmove(line, line + 1, strlen(line + 1) + 1);
					line++;
					break;
				default:
					line = (char *)NO_CONST(g_utf8_skipchar(line));
					break;
				}
			} else
			{
				line = (char *)NO_CONST(g_utf8_skipchar(line));
			}
			break;
		default:
			/*
			 * plain text
			 */
			line = (char *)NO_CONST(g_utf8_skipchar(line));
			break;
		}
	}
	column += do_autoref(vars, start_text, line - start_text);
	addbyte(vars, HYP_EOL);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------------
 * Main processing loop
 * -----------------------------------------
 */

static gboolean pass(hcp_vars *vars, const char *filename)
{
	FILE *infile;
	FILE_ID id;
	HCP_INCLUDE_FILE *inc;
	char *line;
	gboolean skipping;
	int start_if_level, end_if_level;
	gboolean retval;
	gboolean is_new;
	hyp_filetype type;
	
	infile = hyp_utf8_fopen(filename, "rb");
	if (infile == NULL)
	{
		hcp_error(vars, NULL, "%s: %s", filename, hyp_utf8_strerror(errno));
		return FALSE;
	}
	
	/* test recursive includes */
	type = HYP_FT_STG;
	id = file_listadd_id(vars, filename, &type, &is_new);
	for (inc = vars->include_stack; inc != NULL; inc = inc->next)
	{
		if (inc->loc.id == id)
		{
			hcp_error(vars, NULL, _("Recursive include detected"));
			fclose(infile);
			return FALSE;
		}
	}
	if (push_file_stack(vars, filename, infile) == FALSE)
	{
		fclose(infile);
		return FALSE;
	}
	if (vars->include_stack->next == NULL && vars->first_loc.id == 0)
		vars->first_loc = vars->include_stack->loc;
	
	start_if_level = vars->if_stack ? vars->if_stack->level : 0;
	
	hcp_status_file(vars, filename);

	skipping = is_if_stack_skipping(vars);
	while ((line = readline(vars, infile)) != NULL)
	{
		if (line[0] == '#' && line[1] == '#')
		{
			g_free(line);
			continue;
		}
		vars->cur_fileline = line;
		if (*line == '@' &&   /* maybe start of command... */
			line[1] != '{' && /* ...but not if inline comand */
			line[1] != '@' && /* ...and not if quoted '@' */
			line[1] != ':')   /* ...and not if variable reference */
			                  /* maybe also not when in_tree,
			                     but that matters only when there could
			                     be defines starting with '@'
			                     (and we have to check at least for @endtree then)
			                     */
		{
			size_t i;
			int argc;
			char **argv;
			hcp_cmd cmd;
			gboolean first_arg_quoted;
			char *end;
			
			argc = parse_args(vars, line, '\0', &argv, &end, &first_arg_quoted);
			if (G_UNLIKELY(argc <= 0))
			{
				g_free(line);
				continue;
			}
			for (i = 0; i < NUM_HCP_COMMANDS; i++)
			{
				if (cmdnamecmp(argv[0] + 1, hcp_commands[i].name) == 0)
					break;
			}
			if (G_UNLIKELY(i >= NUM_HCP_COMMANDS))
			{
				if (!skipping)
					hcp_error(vars, NULL, _("illegal command %s"), argv[0]);
				g_strfreev(argv);
				continue;
			}
			cmd = vars->hcp_pass == 1 ? hcp_commands[i].pass1 : hcp_commands[i].pass2;
			if (hcp_commands[i].where & CMD_ALWAYS)
			{
				/* commands like @if that are allowed everywhere */
				cmd(vars, argc, argv);
				skipping = is_if_stack_skipping(vars);
			} else if (skipping)
			{
				/* skipping source due to @if evaluating to FALSE */
			} else if ((hcp_commands[i].where & (CMD_IN_PREAMBLE|CMD_IN_NODE)) == CMD_IN_PREAMBLE)
			{
				/* commands like @author that are only allowed in preamble */
				if (!vars->in_preamble)
				{
					hcp_error(vars, NULL, _("%s command ignored after first @node"), argv[0]);
				} else
				{
					cmd(vars, argc, argv);
				}
			} else if ((hcp_commands[i].where & (CMD_IN_PREAMBLE|CMD_IN_NODE)) == CMD_IN_NODE)
			{
				/* commands like @title that are only allowed inside a node */
				if (!vars->in_node)
				{
					hcp_error(vars, NULL, _("%s ignored outside @node"), argv[0]);
				} else if (vars->in_node == 2 && (hcp_commands[i].where & CMD_NOT_PNODE))
				{
					hcp_error(vars, NULL, _("%s not allowed in @pnode"), argv[0]);
				} else
				{
					cmd(vars, argc, argv);
				}
			} else if ((hcp_commands[i].where & (CMD_IN_PREAMBLE|CMD_IN_NODE)) == (CMD_IN_PREAMBLE|CMD_IN_NODE))
			{
				/* commands that are allowed both in preamble or node, with different semantics, but not between nodes */
				if (!vars->in_node && !vars->in_preamble)
				{
					hcp_error(vars, NULL, _("%s not allowed here"), argv[0]);
				} else if (vars->in_node == 2 && (hcp_commands[i].where & CMD_NOT_PNODE))
				{
					hcp_error(vars, NULL, _("%s not allowed in @pnode"), argv[0]);
				} else
				{
					cmd(vars, argc, argv);
				}
			} else
			{
				/* commands like @toc that are allowed everywhere, with different semantics */
				if (vars->in_node == 2 && (hcp_commands[i].where & CMD_NOT_PNODE))
				{
					hcp_error(vars, NULL, _("%s not allowed in @pnode"), argv[0]);
				} else
				{
					cmd(vars, argc, argv);
				}
			}
			g_strfreev(argv);
		} else
		{
			/*
			 * plain text, or inline command expanding to text
			 */
			g_strchomp(line);
			if (!skipping)
			{
				if (!vars->in_node)
				{
					if (!empty(line) && vars->hcp_pass == 1)
						hcp_warning(vars, NULL, _("plain text outside node ignored"));
				} else
				{
					if (vars->in_tree)
					{
						if (vars->cur_rsc_tree_nr != HYP_NOINDEX)
							process_treeline(vars, line);
					} else
					{
						process_text(vars, &line);
						warn_converror(vars, NULL, 2);
						++vars->node_lineno;
					}
				}
			}
		}
		g_free(line);
	}
	end_if_level = vars->if_stack ? vars->if_stack->level : 0;
	if (end_if_level > start_if_level)
	{
		hcp_error(vars, NULL, _("missing @endif"));
		fprintf(vars->errorfile, _("last @if: %s line %lu\n"), file_lookup_name(vars, vars->if_stack->loc.id), vars->if_stack->loc.lineno);
	}
	
	retval = TRUE;
	check_endnode(vars, FALSE);
	ASSERT(vars->include_stack != NULL);
	if (vars->include_stack->next == NULL)
	{
		/*
		 * finish pass 1/2 here instead of in main function,
		 * so we still have an input location for error messages
		 * before removing the (main) inputfile from the stack.
		 */
		if (vars->hcp_pass == 1)
		{
			if (finish_pass1(vars) == FALSE)
				retval = FALSE;
		} else
		{
			if (finish_pass2(vars) == FALSE)
				retval = FALSE;
			ASSERT(vars->p1_node_counter == vars->p2_node_counter);
			if (vars->need_eof_entry)
			{
				vars->p2_external_node_counter++;
				vars->p2_real_external_node_counter++;
			}
			ASSERT(vars->p1_external_node_counter == vars->p2_external_node_counter);
			/*
			 * that assertion is no longer true:
			 * p2_real_external_node_counter goes over all external entries,
			 * but unreferenced extern entries are not written to the index table
			 */
			/* ASSERT(vars->hyp->num_index == vars->p2_real_external_node_counter); */
		}
		flush_status_output(vars);
	}
	pop_file_stack(vars);
		
	return retval;
}

/* ------------------------------------------------------------------------- */

static void hcp_start_pass(hcp_vars *vars, int passno)
{
	vars->hcp_pass = passno;
	vars->in_node = FALSE;
	vars->in_tree = FALSE;
	vars->cur_rsc_tree_nr = HYP_NOINDEX;
	vars->in_preamble = TRUE;
	vars->p2_node_counter = 0;
	vars->page_used = 0;
	
	set_globals(vars);
	vars->hyp->line_width = HYP_STGUIDE_DEFAULT_LINEWIDTH;
	vars->input_charset = hyp_get_current_charset();
	
	if (vars->opts->verbose >= 2)
		hcp_status_pass(vars, passno == 1 ? _("Pass 1: ") : _("Pass 2: "));
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------------
 * Output at end
 * -----------------------------------------
 */

static gboolean write_index(HYP_DOCUMENT *hyp, FILE *outfile, gboolean update)
{
	size_t ret;
	size_t size;
	hyp_nodenr i;
	INDEX_ENTRY *entry;
	unsigned char rawent[SIZEOF_INDEX_ENTRY];
	
	for (i = 0; i < hyp->num_index; i++)
	{
		entry = hyp->indextable[i];
		switch (entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			/* next, prev, toc already updated */
			break;
		case HYP_NODE_IMAGE:
			if (update)
			{
				entry->pic_node_index = 0;
				/* keep next for extending comp_diff, and toc_index for the file format */
			}
			break;
		case HYP_NODE_EXTERNAL_REF:
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
		case HYP_NODE_EOF:
			/* reset unused fields for others */
			if (update)
			{
				entry->previous = 0;
				entry->next = 0;
				entry->toc_index = 0;
			}
			break;
		default:
			unreachable();
			break;
		}
		rawent[0] = entry->length;
		rawent[1] = entry->type;
		long_to_chars(entry->seek_offset, rawent + 2);
		short_to_chars(entry->comp_diff, rawent + 6);
		short_to_chars(entry->next, rawent + 8);
		short_to_chars(entry->previous, rawent + 10);
		short_to_chars(entry->toc_index, rawent + 12);
		ret = fwrite(rawent, 1, SIZEOF_INDEX_ENTRY, outfile);
		if (G_UNLIKELY(ret != SIZEOF_INDEX_ENTRY))
			return FALSE;
		size = entry->length - SIZEOF_INDEX_ENTRY;
		ret = fwrite(entry->name, 1, size, outfile);
		if (G_UNLIKELY(ret != size))
			return FALSE;
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

__attribute__((noinline))
static gboolean write_ext_header(FILE *outfile, hyp_ext_header type, unsigned short len, void *data)
{
	unsigned char info[2 * SIZEOF_SHORT];
	size_t ret;
	
	short_to_chars(type, info);
	if (len & 1)
		short_to_chars(len + 1, info + 2);
	else
		short_to_chars(len, info + 2);
	ret = fwrite(info, 1, sizeof(info), outfile);
	if (G_UNLIKELY(ret != sizeof(info)))
		return FALSE;
	if (len != 0)
	{
		ret = fwrite(data, 1, len, outfile);
		if (G_UNLIKELY(ret != len))
			return FALSE;
		if (len & 1)
		{
			if (G_UNLIKELY(fputc('\0', outfile) != 0))
				return FALSE;
		}
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

__attribute__((noinline))
static gboolean write_ext_header_string(hcp_vars *vars, HYP_DOCUMENT *hyp, FILE *outfile, hyp_ext_header type, const char *str)
{
	char *data;
	unsigned short len;
	
	if (str == NULL)
		return TRUE;
	data = hyp_utf8_to_charset(hyp->comp_charset, str, STR0TERM, &vars->global_converror);
	if (G_UNLIKELY(data == NULL))
		return FALSE;
	len = (unsigned short)strlen(data);
	if (G_UNLIKELY(write_ext_header(outfile, type, len + 1, data) == FALSE))
	{
		g_free(data);
		return FALSE;
	}
	g_free(data);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean write_header(hcp_vars *vars)
{
	unsigned char rawhead[SIZEOF_HYP_HEADER];
	ssize_t ret;
	HYP_HEADER head;
	unsigned char info[SIZEOF_SHORT];
	FILE *outfile = vars->outfile;
	HYP_DOCUMENT *hyp = vars->hyp;
	
	head.magic = HYP_MAGIC_HYP;
	head.itable_size = hyp->itable_size;
	head.itable_num = hyp->num_index;
	head.compiler_os = hyp->comp_os;

	/*
	 * Last version of ST-Guide V(09.09.1996) seems to write a compiler version
	 * depending on the features used. As far as i can tell:
	 * - minimum compiler version is 3
	 * - use of @xrefs bumps version up to 4
	 * - use of centered images (X-offset == 0) bumps version up to 5
	 * Unfortunately, that behaviour was never documented.
	 */
	
	head.compiler_vers =
		hyp->comp_charset != HYP_CHARSET_ATARI ? 6 :
		vars->uses_limage ? 5 :
		vars->uses_xref ? 4 :
		3;
	
	long_to_chars(head.magic, rawhead);
	long_to_chars(head.itable_size, rawhead + 4);
	short_to_chars(head.itable_num, rawhead + 8);
	rawhead[10] = head.compiler_vers;
	rawhead[11] = head.compiler_os;
	ret = fwrite(rawhead, 1, SIZEOF_HYP_HEADER, outfile);
	if (G_UNLIKELY(ret != SIZEOF_HYP_HEADER))
		return FALSE;
	
	/*
	 * write the index table out. Some of the data is
	 * not yet known and will be updated later
	 */
	if (G_UNLIKELY(write_index(hyp, vars->outfile, FALSE) == FALSE))
		return FALSE;
	
	/*
	 * write extra headers
	 */
	
	/* charset first because the other strings are encoded using it */
	
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_CHARSET, hyp_charset_name(hyp->comp_charset)) == FALSE)
		return FALSE;
	
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_DATABASE, hyp->database) == FALSE)
		return FALSE;
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_DEFAULT, hyp->default_name) == FALSE)
		return FALSE;
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
			if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_HOSTNAME, h->name) == FALSE)
				return FALSE;
	}
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_OPTIONS, hyp->hcp_options) == FALSE)
		return FALSE;
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_AUTHOR, hyp->author) == FALSE)
		return FALSE;
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_VERSION, hyp->version) == FALSE)
		return FALSE;
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_HELP, hyp->help_name) == FALSE)
		return FALSE;
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_SUBJECT, hyp->subject) == FALSE)
		return FALSE;
	if (hyp->hyptree_len != 0)
		if (write_ext_header(outfile, HYP_EXTH_TREEHEADER, hyp->hyptree_len, hyp->hyptree_data) == FALSE)
			return FALSE;
	/*
	 * see hyp_load() for comments about why next two fields are
	 * written in little endian order.
	 */
	short_to_lechars(hyp->st_guide_flags, info);
	if (write_ext_header(outfile, HYP_EXTH_STGUIDE_FLAGS, SIZEOF_SHORT, info) == FALSE)
		return FALSE;
	short_to_lechars(hyp->line_width, info);
	if (write_ext_header(outfile, HYP_EXTH_WIDTH, SIZEOF_SHORT, info) == FALSE)
		return FALSE;
	if (write_ext_header_string(vars, hyp, outfile, HYP_EXTH_LANGUAGE, hyp->language) == FALSE)
		return FALSE;
	if (write_ext_header(outfile, HYP_EXTH_EOF, 0, NULL) == FALSE)
		return FALSE;
	
	fflush(outfile);
	if (ferror(outfile))
		return FALSE;
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean update_index(hcp_vars *vars)
{
	fflush(vars->outfile);
	if (ferror(vars->outfile))
		return FALSE;
	/*
	 * seek back to beginning of index, just beyond the file header
	 */
	if (fseek(vars->outfile, SIZEOF_HYP_HEADER, SEEK_SET) < 0)
		return FALSE;
	return write_index(vars->hyp, vars->outfile, TRUE);
}

/* ------------------------------------------------------------------------- */

static hyp_pic_format load_image(hcp_vars *vars, int handle, FILELIST *f)
{
	PICTURE pic;
	unsigned char *buf;
	unsigned char *planebuf = NULL;
	unsigned char *to_free = NULL;
	hyp_pic_format format;
	size_t newsize;
	unsigned char planemask;
	
	format = get_image_type(vars, handle, f, &pic, &buf);
#ifndef HAVE_PNG
	if (format == HYP_PIC_PNG)
	{
		/* error already issued in pass 1 */
		g_free(buf);
		return HYP_PIC_UNKNOWN;
	}
#endif
	
	vars->in_node = TRUE;
	vars->page_used = 0;
	addbyte(vars, pic.pi_width >> 8);
	addbyte(vars, pic.pi_width & 0xff);
	addbyte(vars, pic.pi_height >> 8);
	addbyte(vars, pic.pi_height & 0xff);
	addbyte(vars, pic.pi_planes);
	planemask = pic.pi_planes <= 8 ? (1u << pic.pi_planes) - 1 : 0xff;
	addbyte(vars, planemask); /* planemask */
	addbyte(vars, 0x00); /* planeonoff */
	addbyte(vars, 0x00); /* filler */
	
	newsize = vars->page_used + pic.pi_picsize;
	if (newsize > vars->page_alloced)
	{
		vars->page_buf = g_renew(unsigned char, vars->page_buf, newsize);
		if (vars->page_buf == NULL)
		{
			oom(vars);
			g_free(buf);
			return HYP_PIC_UNKNOWN;
		}
		vars->page_alloced = newsize;
	}
	if (pic.pi_planes <= 8 &&		/* truecolor is not converted */
		pic.pi_planes != 1 &&		/* interleaved and standard formats are identical for monochrome */
		format != HYP_PIC_ICN &&	/* ICN reader reads in standard format already */
		format != HYP_PIC_GIF)		/* GIF reader does it own allocation */
	{
		/* need a separate buffer for conversion interleaved -> standard */
		planebuf = g_new0(unsigned char, pic.pi_picsize);
		if (planebuf == NULL)
		{
			oom(vars);
			g_free(buf);
			return HYP_PIC_UNKNOWN;
		}
		to_free = planebuf;
	} else
	{
		planebuf = vars->page_buf + vars->page_used;
	}
	
	switch (format)
	{
	case HYP_PIC_IFF:
		if (iff_unpack(planebuf, buf + pic.pi_dataoffset, &pic) == FALSE)
			format = HYP_PIC_UNKNOWN;
		break;
	case HYP_PIC_ICN:
		if (icn_unpack(planebuf, buf + pic.pi_dataoffset, &pic, FALSE) == FALSE)
			format = HYP_PIC_UNKNOWN;
		break;
	case HYP_PIC_IMG:
		if (img_unpack_safe(planebuf, buf + pic.pi_dataoffset, &pic) == FALSE)
			format = HYP_PIC_UNKNOWN;
		break;
	case HYP_PIC_BMP:
		if (bmp_unpack(planebuf, buf + pic.pi_dataoffset, &pic) == FALSE)
			format = HYP_PIC_UNKNOWN;
		break;
	case HYP_PIC_GIF:
		{
			unsigned char *dest = NULL;
			if (gif_unpack(&dest, buf, &pic) == FALSE || dest == NULL)
				format = HYP_PIC_UNKNOWN;
			planebuf = to_free = dest;
		}
		break;
	case HYP_PIC_PNG:
#ifdef HAVE_PNG
		NYI();
#else
		unreachable();
#endif
		break;
	case HYP_PIC_UNKNOWN:
		unreachable();
		break;
	}
	
	if (format == HYP_PIC_UNKNOWN)
	{
		g_free(to_free);
		g_free(buf);
		hcp_error(vars, &f->first_reference, _("%s: can't unpack"), f->name);
		return HYP_PIC_UNKNOWN;
	}
	
	if (pic.pi_planes <= 8)
	{
		if (planebuf == to_free)
			pic_interleaved_to_planes(vars->page_buf + vars->page_used, planebuf, pic.pi_width, pic.pi_height, pic.pi_planes);
	}
	vars->page_used = newsize;
	vars->in_node = FALSE;
	
	g_free(to_free);
	g_free(buf);
	
	return format;
}

/* ------------------------------------------------------------------------- */

static gboolean write_images(hcp_vars *vars)
{
	unsigned int left = vars->stats.image_count;
	hyp_nodenr i;
	INDEX_ENTRY *entry;
	FILELIST *f;
	int fd;
	gboolean retval = TRUE;
	hyp_pic_format format;
	HYP_DOCUMENT *hyp = vars->hyp;
	
	if (left != 0 && vars->opts->verbose >= 1)
		hcp_status_pass(vars, _("Image : "));
	for (i = 0; i < hyp->num_index; i++)
	{
		entry = hyp->indextable[i];
		switch (entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			/* seek_offset already updated in pass2 */
			break;
		case HYP_NODE_IMAGE:
			left--;
			f = file_lookup(vars, entry->pic_file_id);
			ASSERT(f);
			fd = hyp_utf8_open(f->name, O_RDONLY|O_BINARY, HYP_DEFAULT_FILEMODE);
			if (fd < 0)
			{
				hcp_error(vars, &f->first_reference, _("can't open file '%s': %s"), f->name, hyp_utf8_strerror(errno));
				retval = FALSE;
			} else
			{
				hcp_status_image(vars, f->name, left);
				format = load_image(vars, fd, f);
				if (format == HYP_PIC_UNKNOWN)
					retval = FALSE;
				entry->pic_entry_type = format; /* used only for recompiling */
				close(fd);
			}
			if (!finish_page(vars, i))
				retval = FALSE;
			break;
		case HYP_NODE_EXTERNAL_REF:
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
		case HYP_NODE_EOF:
			entry->seek_offset = vars->seek_offset;
			break;
		}
	}
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static inline void write_long(long val, FILE *fp)
{
	unsigned char p[SIZEOF_LONG];

	long_to_chars(val, p);	
	fwrite(p, 1, SIZEOF_LONG, fp);
}


static inline void write_short(short val, FILE *fp)
{
	unsigned char p[SIZEOF_SHORT];

	short_to_chars(val, p);	
	fwrite(p, 1, SIZEOF_SHORT, fp);
}


static inline void write_str(hcp_vars *vars, unsigned char type, const char *str, hyp_lineno lineno, FILE *fp)
{
	size_t len;
	char *out;
	
	out = hyp_utf8_to_charset(vars->hyp->comp_charset, str, STR0TERM, &vars->global_converror);
	if (G_UNLIKELY(out == NULL))
	{
		oom(vars);
		return;
	}
	putc(type, fp);
	len = strlen(out) + 1 + 2;
	if (lineno != HYP_NOINDEX)
		len += 2;
	putc((int)len, fp);
	fputs(out, fp);
	putc('\0', fp);
	if (lineno != HYP_NOINDEX)
		write_short(lineno, fp);
	g_free(out);
}


static gboolean write_references(hcp_vars *vars)
{
	FILE *reffile = NULL;
	char *filename;
	gboolean retval = TRUE;
	unsigned long num_entries = 0;
	HYP_DOCUMENT *hyp = vars->hyp;
	
	if (vars->opts->write_references <= 0)
		return TRUE;
	filename = replace_ext(vars->hyp->file, NULL, HYP_EXT_REF);
	if (vars->opts->verbose >= 1)
		hcp_status_pass(vars, _("Writing reference-file"));
	if (G_UNLIKELY(filename == NULL))
	{
		oom(vars);
		retval = FALSE;
	}
	if (retval)
	{
		reffile = hyp_utf8_fopen(filename, "wb");
		if (reffile == NULL)
		{
			hcp_error(vars, NULL, "%s: %s", filename, hyp_utf8_strerror(errno));
			retval = FALSE;
		}
	}
	if (retval)
	{
		char *str;
		LABIDX i;
		LABEL *lab;
		long pos;
		
		/* write file header */
		write_long(HYP_MAGIC_REF, reffile);
		/* write empty module header; will be updated below */
		write_long(0, reffile);
		write_long(0, reffile);
		
		/* write charset */
		write_str(vars, REF_CHARSET, hyp_charset_name(hyp->comp_charset), HYP_NOINDEX, reffile);
		++num_entries;
		
		/* write OS */
		write_str(vars, REF_OS, ref_osname(hyp->comp_os), HYP_NOINDEX, reffile);
		++num_entries;
		
		/* write filename */
		str = ref_hyp_basename(hyp->file);
		write_str(vars, REF_FILENAME, str, HYP_NOINDEX, reffile);
		++num_entries;
		g_free(str);
		
		/* write database */
		if (hyp->database)
		{
			write_str(vars, REF_DATABASE, hyp->database, HYP_NOINDEX, reffile);
			++num_entries;
		}

		/* write language */
		if (hyp->language)
		{
			write_str(vars, REF_LANGUAGE, hyp->language, HYP_NOINDEX, reffile);
			++num_entries;
		}

		for (i = 0; i < vars->p1_lab_counter; i++)
		{
			lab = vars->label_table[i];
			
			if (!lab->add_to_ref)
				continue;
			switch (lab->type)
			{
			case l_node:
				write_str(vars, REF_NODENAME, lab->name, HYP_NOINDEX, reffile);
				++num_entries;
				++vars->stats.refs_generated;
				if (!(vars->hyp->st_guide_flags & STG_ENCRYPTED) && vars->node_table[lab->node_index]->window_title)
				{
					write_str(vars, REF_TITLE, vars->node_table[lab->node_index]->window_title, HYP_NOINDEX, reffile);
					++num_entries;
					++vars->stats.refs_generated;
				}
				break;
			case l_alias:
				write_str(vars, REF_ALIASNAME, lab->name, HYP_NOINDEX, reffile);
				++num_entries;
				++vars->stats.refs_generated;
				break;
			case l_label:
				write_str(vars, REF_LABELNAME, lab->name, lab->lineno - 1, reffile);
				++num_entries;
				++vars->stats.refs_generated;
				break;
			case l_index:
			case l_extern:
			case l_uses:
				break;
			}
		}
		
		pos = ftell(reffile);
		
		/* write file trailer */
		write_long(0, reffile);
		write_long(0, reffile);
		
		fflush(reffile);
		if (ferror(reffile))
		{
			retval = FALSE;
			hcp_error(vars, NULL, _("error writing references file"));
		}
	
		/* update module header */
		fseek(reffile, SIZEOF_LONG, SEEK_SET);
		write_long(pos - 3 * SIZEOF_LONG, reffile);
		write_long(num_entries, reffile);
	}
	if (reffile)
		fclose(reffile);
	warn_converror(vars, NULL, 0);
	
	if (retval && vars->opts->write_references >= 2)
	{
		char *allref;
		
		if (vars->opts->verbose >= 1)
			hcp_status_pass(vars, _("Updating default-reference-file"));
		allref = path_subst(gl_profile.general.all_ref);
		retval = ref_add_entries(allref, filename, FALSE, vars->errorfile, FALSE);
		g_free(allref);
	}
	
	g_free(filename);
	
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* -----------------------------------------
 * Entry point
 * -----------------------------------------
 */

__attribute__((noinline))
static gboolean add_predefs(hcp_vars *vars)
{
	struct tm tm;
	time_t t;
	char datestr[2 + 1 + 2 + 1 + 4 + 1];
	
	time(&t);
	localtime_r(&t, &tm);
	sprintf(datestr, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
	if (G_UNLIKELY(add_define(vars, "__DATE__", HCP_DEF_DATE, datestr) == FALSE))
		return FALSE;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

gboolean hcp_compile(const char *filename, hcp_opts *opts)
{
	gboolean retval = TRUE;
	hcp_vars _vars;
	hcp_vars *vars = &_vars;
	char *output_filename = NULL;
	
	memset(vars, 0, sizeof(*vars));

	vars->hyp = hyp_new();
	if (G_UNLIKELY(vars->hyp == NULL))
	{
		oom(vars);
		return FALSE;
	}
	vars->hyp->st_guide_flags = opts->compat_flags;

	vars->opts = opts;
	vars->outfile = opts->outfile;
	vars->errorfile = opts->errorfile;
	
	if (opts->output_charset != HYP_CHARSET_NONE)
		vars->hyp->comp_charset = opts->output_charset;

	vars->filelist = NULL;
	vars->last_fileid = 0;
	vars->linebuf = NULL;
	vars->linebuf_size = 0;
	vars->include_stack = NULL;
	vars->first_loc.id = 0;
	vars->if_stack = NULL;
	vars->error_count = 0;
	vars->warning_count = 0;
	vars->p1_node_counter = vars->p1_node_alloc = 0;
	vars->p1_lab_counter = vars->p1_lab_alloc = 0;
	vars->status_column = 0;
	vars->hcp_defines = NULL;
	vars->pass_msg = NULL;
	vars->hcp_pass = 0;
	vars->global_toc = NULL;
	vars->page_alloced = 0;
	vars->page_buf = NULL;
	vars->extern_table = NULL;
	vars->p1_external_node_counter = 0;
	vars->p1_external_uses_counter = 0;
	vars->p2_external_node_counter = 0;
	vars->external_nodes_alloced = 0;
	vars->rsc_defines = NULL;
	vars->need_eof_entry = FALSE;
	vars->gen_index = FALSE;
	vars->autorefs = NULL;
	vars->uses_xref = FALSE;
	vars->uses_limage = FALSE;
	
	vars->for_amguide = hyp_guess_filetype(filename) == HYP_FT_GUIDE;
	
	vars->input_charset = hyp_get_current_charset();
	
	{
		const char *s;
		unsigned int i;
		
		s = "\033\012\015\010 !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~\177";
		while (*s != EOS)
		{
			vars->allowed_next_chars[(unsigned char)*s] = 1;
			s++;
		}
		vars->allowed_next_chars['\0'] = 1;
		for (i = 0x80; i < 0x100; i++)
			vars->allowed_next_chars[i] = 1;
		memcpy(vars->allowed_prev_chars, vars->allowed_next_chars, sizeof(vars->allowed_prev_chars));
	}

	if (retval)
	{
		if (opts->output_filename == NULL)
		{
			/*
			 * if no output file specified, create in the default folder
			 */
			char *name = replace_ext(hyp_basename(filename), NULL, HYP_EXT_HYP);
			char *filename = g_build_filename(gl_profile.general.hypfold, name, NULL);
			output_filename = path_subst(filename);
			g_free(filename);
			g_free(name);
		} else if (strcmp(opts->output_filename, HCP_OUTPUT_WILDCARD) == 0)
		{
			output_filename = replace_ext(filename, NULL, HYP_EXT_HYP);
		} else
		{
			output_filename = g_strdup(opts->output_filename);
		}
		if (G_UNLIKELY(output_filename == NULL))
		{
			oom(vars);
			retval = FALSE;
		} else if (hyp_utf8_strcasecmp(filename, output_filename) == 0)
		{
			hcp_error(vars, NULL, _("would overwrite input file"));
			retval = FALSE;
		}

		if (retval)
		{
			if (vars->opts->verbose >= 0)
			{
				fprintf(stdout, _("compiling %s to %s\n"), filename, output_filename);
			}
		}
		vars->hyp->file = output_filename;
	}
	
	if (retval && !add_predefs(vars))
		retval = FALSE;
	
	if (retval)
	{
		hcp_start_pass(vars, 1);
		vars->gen_index = vars->opts->gen_index;
		if (pass(vars, filename) == FALSE)
		{
			retval = FALSE;
		}
	}
	
	if (vars->error_count != 0)
		retval = FALSE;
	
	if (retval)
	{
		vars->outfile = hyp_utf8_fopen(vars->hyp->file, "wb");
		if (vars->outfile == NULL)
		{
			hcp_error(vars, NULL, "%s: %s", vars->hyp->file, hyp_utf8_strerror(errno));
			retval = FALSE;
		}
	}

	if (retval)
	{
		if (vars->hyp->subject == NULL && !vars->for_amguide)
			hcp_warning(vars, &vars->first_loc, _("Please add a @subject-command to this text"));
		
		if (vars->hyp->comp_charset != HYP_CHARSET_ATARI && vars->opts->warn_compat)
			hcp_warning(vars, NULL, _("ST-Guide may not be able to display character set %s"), hyp_charset_name(vars->hyp->comp_charset));
		
		if (retval)
		{
			/* write header & index */
			vars->hyp->hcp_options = hcp_get_option_string(vars->opts);
			retval = write_header(vars);
			if (retval == FALSE)
				error_outfile(vars);
			else
				warn_converror(vars, &vars->first_loc, 0);
		}
		vars->seek_offset = ftell(vars->outfile);
		
		hcp_start_pass(vars, 2);
		
		if (retval)
			retval = pass(vars, filename);
		
		if (retval)
			retval = write_images(vars);
			
		if (retval)
		{
			retval = update_index(vars);
			if (retval == FALSE)
				error_outfile(vars);
		}
		
		if (retval)
			retval = write_references(vars);
		
		if (vars->error_count != 0)
			retval = FALSE;
	
		flush_status_output(vars);
		if (retval && opts->verbose >= 0)
		{
			hyp_utf8_fprintf(stdout, _("internal nodes: %u\n"), vars->stats.internal_nodes);
			hyp_utf8_fprintf(stdout, _("external nodes: %u\n"), vars->stats.external_nodes);
			hyp_utf8_fprintf(stdout, _("images        : %u\n"), vars->stats.image_count);
			hyp_utf8_fprintf(stdout, _("other nodes   : %u\n"), vars->stats.other_nodes);
			hyp_utf8_fprintf(stdout, _("autorefs      : %lu\n"), vars->stats.autorefs);
			if (vars->opts->write_references > 0)
			hyp_utf8_fprintf(stdout, _("Refs generated: %lu\n"), vars->stats.refs_generated);
			if (vars->opts->compression)
			hyp_utf8_fprintf(stdout, _("compression   : %lu (%lu%%)\n"), vars->stats.comp_diff, (vars->stats.comp_diff * 100) / (vars->seek_offset + vars->stats.comp_diff));
		}
		
		if (retval)
		{
			LABIDX l;
			LABEL *lab;
			
			for (l = 0; l < vars->p1_lab_counter; l++)
			{
				lab = vars->label_table[l];
				if (!lab->referenced && !lab->add_to_ref && lab->type != l_uses && (lab->type != l_index || vars->gen_index))
					hcp_warning(vars, &lab->source_location, _("not referenced: %s"), lab->name);
			}
		}
	} else
	{
		if (vars->hcp_pass != 0)
			hcp_error(vars, NULL, _("cannot start pass2"));
	}
	flush_status_output(vars);
	
	if (vars->error_count != 0)
		retval = FALSE;
	
	hcp_comp_exit(vars);
	g_free(output_filename);
	
	/*
	 * original hcp exits with code 2 in case of warnings;
	 * i consider this a bug since a exit code != 0 should
	 * indicate errors
	 */
	
	return retval;
}
