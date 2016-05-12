#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "hcp.h"
#include <dirent.h>
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "hv_vers.h"


char const gl_program_name[] = "HypFind";
char const gl_program_version[] = HYP_VERSION;

static char const hypfind_stg[] = "hypfind" HYP_EXT_STG;
static char const hypfind_hyp[] = HYP_FILENAME_HYPFIND;

/*
 * Jump table structures.
 */
typedef struct {
	size_t last_jump;
	size_t delta[256];
	size_t patlen;
	size_t patlen_minus_1;
	const char *pattern;
	gboolean casesensitive;
	gboolean slowcase;
	size_t *skip_table[256];
} BM_TABLE;

struct hypfind_opts {
	gboolean do_compile;
	gboolean verbose;
	gboolean do_help;
	gboolean do_version;
	gboolean casesensitive;
	gboolean wordonly;
	char *pattern;
	size_t pattern_len;
	int optind;
	char *hypfind_stg;
	char *hypfind_hyp;
	FILE *outfile;
	FILE *errorfile;
	char *error_filename;
	HYP_CHARSET output_charset;
	gboolean print_unknown;
	gboolean multiple;
	unsigned long filecount;
	unsigned long hits;
	unsigned long total_hits;
	BM_TABLE deltapat;
};

static gboolean is_MASTER = TRUE;

static struct option const long_options[] = {
	{ "compile", no_argument, NULL, 'c' },
	{ "quiet", no_argument, NULL, 'q' },
	{ "casesensitive", no_argument, NULL, 'I' },
	{ "ignore-case", no_argument, NULL, 'i' },
	{ "pattern", required_argument, NULL, 'p' },
	{ "word", no_argument, NULL, 'w' },
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

/* ------------------------------------------------------------------------- */

static void print_usage(FILE *out)
{
	print_short_version(out);
	hyp_utf8_fprintf(out, _("usage: %s [-cqiIw] -p <pattern> file...\n"), gl_program_name);
	hyp_utf8_fprintf(out, _("options:\n"));
	hyp_utf8_fprintf(out, _("  -c, --compile                       do not compile output file\n"));
	hyp_utf8_fprintf(out, _("  -q, --quiet                         suppress output messages\n"));
	hyp_utf8_fprintf(out, _("  -I, --casesensitive                 case-sensitive search\n"));
	hyp_utf8_fprintf(out, _("  -i, --ignore-case                   ignore case distinctions (default)\n"));
	hyp_utf8_fprintf(out, _("  -p, --pattern <pattern>             text to search for\n"));
	hyp_utf8_fprintf(out, _("  -w, --word                          force PATTERN to match only whole words\n"));
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

/* ------------------------------------------------------------------------- */

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
	case OPTS_FROM_SOURCE:
		hcp_usage_error(_("%s can not be specified %s"), what, _("in the source file"));
		break;
	}
	return FALSE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *stg_quote_name(const char *name, size_t len)
{
	char *str, *ret;
	
	if (len == STR0TERM)
		len = strlen(name);
	str = ret = g_new(char, len * 2 + 1);
	if (str != NULL)
	{
		while (*name)
		{
			if (*name == '\\' || *name == '"')
				*str++ = '\\';
			*str++ = *name++;
			len--;
		}
		*str++ = '\0';
		ret = g_renew(char, ret, str - ret);
	}
	return ret;
}

/* ------------------------------------------------------------------------- */

static char *stg_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *buf, *str;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	buf = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
	str = stg_quote_name(buf, STR0TERM);
	g_free(buf);
	return str;
}

/* ------------------------------------------------------------------------- */

static void stg_out_str(FILE *outfile, HYP_CHARSET charset, const char *str, const char *matchstart, const char *matchend)
{
	const char *p;
	gboolean converror = FALSE;
	char buf[HYP_UTF8_CHARMAX + 1];
	
	p = str;
	while (*p)
	{
		if (p == matchstart)
		{
			fputs("@{B}", outfile);
		}
		if (*p == '@')
		{
			fputc('@', outfile); /* AmigaGuide uses \@ for quoted '@' */
			fputc('@', outfile);
			p++;
		} else if (*((const unsigned char *)p) < 0x80)
		{
			fputc(*p, outfile);
			p++;
		} else
		{
			p = hyp_utf8_conv_char(charset, p, buf, &converror);
			fputs(buf, outfile);
		}
		if (p == matchend)
		{
			fputs("@{b}", outfile);
		}
	}
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void bm_init_block(BM_TABLE *tbl, size_t *delta)
{
	size_t j;

	for (j = 0; j < 256; j++)
		delta[j] = tbl->patlen;
}

/* ------------------------------------------------------------------------- */

static gboolean bm_add_delta(BM_TABLE *tbl, h_unichar_t ch, size_t delta)
{
	unsigned int idx;
	
	if (ch >= 0x10000UL)
	{
		tbl->slowcase = TRUE;
		return TRUE;
	}
	idx = (unsigned int)(ch >> 8);
	if (tbl->skip_table[idx] == tbl->delta)
	{
		tbl->skip_table[idx] = g_new(size_t, 256);
		if (tbl->skip_table[idx] == NULL)
			return FALSE;
		bm_init_block(tbl, tbl->skip_table[idx]);
	}
	tbl->skip_table[idx][ch & 0xff] = delta;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

/*
 * bm_init -- Create the delta tables.
 */
static gboolean bm_init(BM_TABLE *tbl, const char *pstring, gboolean casesensitive)
{
	size_t j;
	size_t jump_by;
	
	for (j = 0; j < 256; j++)
		tbl->skip_table[j] = tbl->delta;
		
	tbl->pattern = pstring;

	if (pstring == NULL)
	{
		tbl->patlen = 0;
		bm_init_block(tbl, tbl->delta);
		return TRUE;
	}
	
	/*
	 * we cannot scan for an empty pattern
	 */
	if (*pstring == '\0')
		return FALSE;
	
	tbl->casesensitive = casesensitive;
	tbl->slowcase = FALSE;
	
	jump_by = strlen(pstring);
	tbl->patlen = jump_by;

	bm_init_block(tbl, tbl->delta);

	if (casesensitive)
	{
		const unsigned char *sp;
		unsigned char ch;
		
		/*
		 * we don't have to care for any mapping,
		 * and can just search for the bytes.
		 */
		jump_by -= 1;
		tbl->patlen_minus_1 = jump_by;
		
		/* Now put in the characters contained
		 * in the pattern.
		 */
		sp = (const unsigned char *)tbl->pattern;
		for (j = 0; j < jump_by; j++)
		{
			ch = *sp++;
			tbl->delta[ch] = jump_by - j;
		}
		
		/* The last character (left over from the loop above) will
		 * have the pattern length, unless there are duplicates of
		 * it.  Get the number to jump from the delta array, and
		 * overwrite with zeroes in delta.
		 */
		ch = *sp;
		tbl->last_jump = tbl->delta[ch];
		
		tbl->delta[ch] = 0;
	} else
	{
		h_unichar_t ch;
		const char *sp;
		
		/*
		 * get byte-length of pattern without the last character
		 */
		sp = tbl->pattern;
		do {
			jump_by = sp - tbl->pattern;
			sp = g_utf8_skipchar(sp);
		} while (*sp != '\0');
		tbl->patlen_minus_1 = jump_by;
		
		/* Now put in the characters contained
		 * in the pattern, duplicating the CASE.
		 */
		sp = tbl->pattern;
		for (;;)
		{
			j = jump_by - (sp - tbl->pattern);
			ch = hyp_utf8_get_char(sp);
			sp = g_utf8_skipchar(sp);
			if (*sp == '\0')
				break;
			if (!bm_add_delta(tbl, ch, j))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_tolower(ch), j))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_toupper(ch), j))
				return FALSE;
		}
		
		/* The last character (left over from the loop above) will
		 * have the pattern length, unless there are duplicates of
		 * it.  Get the number to jump from the delta array, and
		 * overwrite with zeroes in delta duplicating the CASE.
		 */
		if (ch >= 0x10000UL)
		{
			tbl->slowcase = TRUE;
		} else
		{
			unsigned int idx = (unsigned int)(ch >> 8);
			tbl->last_jump = tbl->skip_table[idx][ch & 0xff];
			if (!bm_add_delta(tbl, ch, 0))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_tolower(ch), 0))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_toupper(ch), 0))
				return FALSE;
		}
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static void bm_exit(BM_TABLE *tbl)
{
	size_t j;
	
	if (tbl->pattern == NULL || tbl->casesensitive)
		return;
	for (j = 0; j < 256; j++)
		if (tbl->skip_table[j] != tbl->delta)
		{
			g_free(tbl->skip_table[j]);
			tbl->skip_table[j] = NULL;
		}
}

/* ------------------------------------------------------------------------- */

/*
 * bm_streq -- compare the string versus the current characters in the line.
 *  Returns 0 (no match) or the number of characters matched.
 */
static gboolean bm_streq(BM_TABLE *tbl, const unsigned char *text)
{
	const unsigned char *strptr = (const unsigned char *)tbl->pattern;
	
	while (*strptr)
	{
		if (*strptr != *text)
			return FALSE;
		strptr++;
		text++;
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean bm_strcaseeq(BM_TABLE *tbl, const unsigned char *text)
{
	const char *strptr = tbl->pattern;
	const char *textptr = (const char *)text;
	h_unichar_t ch1, ch2;
	
	while (*strptr)
	{
		ch1 = g_unichar_tolower(hyp_utf8_get_char(textptr));
		if (G_UNLIKELY(ch1 >= 0x10000UL))
			return hyp_utf8_strncasecmp((const char *)text, tbl->pattern, tbl->patlen) == 0;
		ch2 = g_unichar_tolower(hyp_utf8_get_char(strptr));
		if (ch1 != ch2)
			return FALSE;
		strptr = g_utf8_skipchar(strptr);
		textptr = g_utf8_skipchar(textptr);
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static const unsigned char *bm_boundary(BM_TABLE *tbl, const unsigned char *text, size_t jump, const unsigned char *end)
{
	while (jump != 0)
	{
		text += jump;
		if (text >= end)
			return NULL;

		jump = tbl->delta[*text];
	}
	return text;
}

/* ------------------------------------------------------------------------- */

static const unsigned char *bm_caseboundary(BM_TABLE *tbl, const unsigned char *text, size_t jump, const unsigned char *end)
{
	h_unichar_t ch;
	unsigned int idx;
	
	while (jump != 0)
	{
		text += jump;
		if (text >= end)
			return NULL;
		ch = hyp_utf8_get_char((const char *)text);
		if (G_UNLIKELY(ch >= 0x10000UL))
		{
			tbl->slowcase = TRUE;
			return text;
		}
		idx = (unsigned int)(ch >> 8);
		jump = tbl->skip_table[idx][ch & 0xff];
	}
	return text;
}

/* ------------------------------------------------------------------------- */

static const char *bm_scanner(BM_TABLE *tbl, const char *buf, size_t len)
{
	register size_t patlenadd;
	const unsigned char *text = (const unsigned char *)buf;
	const unsigned char *end = text + len;
	
	patlenadd = tbl->patlen_minus_1;
	
	/* Scan each character until we hit the head link record.
	 * Get the character resolving newlines, offset
	 * by the pattern length, i.e. the last character of the
	 * potential match.
	 */
	if ((text = bm_boundary(tbl, text, patlenadd, end)) != NULL)
	{
		do
		{
			/* Save the current position in case we match
			 * the search string at this point.
			 */
			text -= patlenadd;
			
			if (bm_streq(tbl, text))
			{
				return (const char *)text;
			}
			
			text += patlenadd;
		} while ((text = bm_boundary(tbl, text, tbl->last_jump, end)) != NULL);
	}

	return NULL;	/* We could not find a match */
}

/* ------------------------------------------------------------------------- */

static const char *bm_casescanner(BM_TABLE *tbl, const char *buf, size_t len)
{
	register size_t patlenadd;
	const unsigned char *text = (const unsigned char *)buf;
	const unsigned char *end = text + len;
	
	patlenadd = tbl->patlen_minus_1;
	
	/* Scan each character until we hit the head link record.
	 * Get the character resolving newlines, offset
	 * by the pattern length, i.e. the last character of the
	 * potential match.
	 */
	if ((text = bm_caseboundary(tbl, text, patlenadd, end)) != NULL)
	{
		do
		{
			if (G_UNLIKELY(tbl->slowcase))
				return hyp_utf8_strcasestr(buf, tbl->pattern);
			
			/* Save the current position in case we match
			 * the search string at this point.
			 */
			text -= patlenadd;
			
			if (bm_strcaseeq(tbl, text))
			{
				return (const char *)text;
			}
			
			text += patlenadd;
		} while ((text = bm_caseboundary(tbl, text, tbl->last_jump, end)) != NULL);
	}

	return NULL;	/* We could not find a match */
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean is_word_char(unsigned char ch)
{
	if (ch >= 0x80)
		return TRUE;
	if (strchr(gl_profile.hypfind.wordchars, ch) != NULL)
		return TRUE;
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static void search_text(HYP_DOCUMENT *hyp, struct hypfind_opts *opts, const char *text, size_t textlen, HYP_NODE *nodeptr)
{
	char *linktext;
	char *nodename;
	const char *match;
	const char *scan = text;
	size_t scanlen = textlen;
	const char *target;
	
	for (;;)
	{
		size_t offset;
		
		if (scanlen == 0)
			return;
		if (opts->casesensitive)
			match = bm_scanner(&opts->deltapat, scan, scanlen);
		else if (G_UNLIKELY(opts->deltapat.slowcase))
			match = hyp_utf8_strcasestr(scan, opts->pattern);
		else
			match = bm_casescanner(&opts->deltapat, scan, scanlen);
		if (match == NULL)
			return;
		if (!opts->wordonly)
			break;
		offset = match - scan;
		if ((offset == 0 || !is_word_char(match[-1])) &&
			(offset + opts->pattern_len >= scanlen || !is_word_char(match[opts->pattern_len])))
			break;
		offset += opts->pattern_len;
		scan += offset;
		ASSERT(scanlen <= offset);
		scanlen -= offset;
	} 
	nodename = stg_quote_nodename(hyp, nodeptr->number);
	if (nodeptr->window_title)
	{
		char *buf = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
		linktext = stg_quote_name(buf, STR0TERM);
		g_free(buf);
	} else
	{
		linktext = nodename;
	}
	target = hyp_basename(hyp->file);
	hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@{\"%s, Node '%s'\" link \"%s/%s\" %ld}\n", target, linktext, target, nodename, nodeptr->height);
	fputs(": ", opts->outfile);
	stg_out_str(opts->outfile, opts->output_charset, text, match, match + opts->pattern_len);
	fputs("\n", opts->outfile);
	fputs("\n", opts->outfile);
	
	if (nodename != linktext)
		g_free(nodename);
	g_free(linktext);
	opts->hits++;
}

/* ------------------------------------------------------------------------- */

static gboolean search_node(HYP_DOCUMENT *hyp, struct hypfind_opts *opts, HYP_NODE *nodeptr)
{
	char *str;
	const unsigned char *src;
	const unsigned char *end;
	const unsigned char *textstart;
	char *text;
	size_t len, textlen, text_alloced;
	gboolean retval = TRUE;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		str = hyp_conv_to_utf8(hyp->comp_charset, textstart, src - textstart); \
		if (str == NULL) { retval = FALSE; goto error; } \
		len = strlen(str); \
		if ((textlen + len) >= text_alloced) \
		{ \
			text_alloced = textlen + len + 1024; \
			text = g_renew(char, text, text_alloced); \
			if (text == NULL) { retval = FALSE; goto error; } \
		} \
		strcpy(text + textlen, str); \
		textlen += len; \
		g_free(str); \
	}
	
	end = nodeptr->end;

	/*
	 * now output data
	 */
	src = nodeptr->start;
	textstart = src;
	nodeptr->height = 0;
	textlen = 0;
	text_alloced = 1024;
	text = g_new(char, text_alloced);
	if (text == NULL) { retval = FALSE; goto error; }
	
	while (src < end)
	{
		if (*src == HYP_ESC)
		{
			DUMPTEXT();
			src++;
			switch (*src)
			{
			case HYP_ESC_ESC:
				textstart = src;
				src++;
				DUMPTEXT();
				break;
			
			case HYP_ESC_WINDOWTITLE:
				src++;
				nodeptr->window_title = src;
				src += ustrlen(src) + 1;
				break;

			case HYP_ESC_CASE_DATA:
				if (src[1] < 3u)
					src += 2;
				else
					src += src[1] - 1;
				break;
			
			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					unsigned char type;
					size_t len;
					
					type = *src;
					if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
					{
						src += 2;
					}
					src += 3;
					if (*src <= HYP_STRLEN_OFFSET)
					{
						src++;
					} else
					{
						len = *src - HYP_STRLEN_OFFSET;
						src++;
						textstart = src;
						src += len;
						DUMPTEXT();
					}
				}
				break;
				
			case HYP_ESC_EXTERNAL_REFS:
				if (src[1] < 5u)
					src += 4;
				else
					src += src[1] - 1;
				break;
				
			case HYP_ESC_OBJTABLE:
				src += 9;
				break;
				
			case HYP_ESC_PIC:
				src += 8;
				break;
				
			case HYP_ESC_LINE:
				src += 7;
				break;
				
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				src += 7;
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				src++;
				break;
			
			case HYP_ESC_UNKNOWN_A4:
				if (opts->print_unknown)
					hyp_utf8_fprintf(opts->errorfile, _("<unknown hex esc $%02x>\n"), *src);
				src++;
				break;
			
			default:
				if (opts->print_unknown)
					hyp_utf8_fprintf(opts->errorfile, _("<unknown hex esc $%02x>\n"), *src);
				break;
			}
			textstart = src;
		} else if (*src == HYP_EOL)
		{
			DUMPTEXT();
			++nodeptr->height;
			text[textlen] = '\0';
			search_text(hyp, opts, text, textlen, nodeptr);
			src++;
			textstart = src;
			textlen = 0;
		} else
		{
			src++;
		}
	}
	
	if (retval)
	{
		DUMPTEXT();
		++nodeptr->height;
		text[textlen] = '\0';
		search_text(hyp, opts, text, textlen, nodeptr);
	}
	
error:
	g_free(text);
	
#undef DUMPTEXT
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean search_hyp(HYP_DOCUMENT *hyp, struct hypfind_opts *opts)
{
	hyp_nodenr node_num;
	INDEX_ENTRY *entry;
	hcp_opts hyp_opts;
	gboolean retval = TRUE;
	HYP_NODE *node;
	
	hcp_opts_init(&hyp_opts);
	hyp_opts.outfile = opts->outfile;
	hyp_opts.errorfile = opts->errorfile;
	hyp_opts.verbose = opts->verbose;
	if (hyp->hcp_options != NULL)
	{
		hcp_opts_parse_string(&hyp_opts, hyp->hcp_options, OPTS_FROM_SOURCE);
	}
	for (node_num = 0; node_num < hyp->num_index; node_num++)
	{
		if (opts->verbose)
		{
			fputc('.', stdout);
			fflush(stdout);
		}
		entry = hyp->indextable[node_num];
		if (node_num == hyp->index_page)
			continue;
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
			node = hyp_loadtext(hyp, node_num);
			if (node != NULL)
			{
				retval &= search_node(hyp, opts, node);
				hyp_node_free(node);
			}
			break;
		case HYP_NODE_POPUP:
		case HYP_NODE_IMAGE:
		case HYP_NODE_EXTERNAL_REF:
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
		case HYP_NODE_EOF:
			break;
		default:
			if (opts->print_unknown)
				hyp_utf8_fprintf(opts->errorfile, _("unknown index entry type %u\n"), entry->type);
			break;
		}
	}
	hyp_opts.outfile = NULL;
	hyp_opts.errorfile = NULL;
	hcp_opts_free(&hyp_opts);
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean process_file(const char *filename, struct hypfind_opts *opts)
{
	gboolean retval;
	HYP_DOCUMENT *hyp;
	hyp_filetype ftype;
	int handle;
	
	opts->hits = 0;
	handle = hyp_utf8_open(filename, O_RDONLY|O_BINARY, HYP_DEFAULT_FILEMODE);
	if (handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}
	hyp = hyp_load(filename, handle, &ftype);
	if (hyp == NULL)
	{
		hyp_utf8_close(handle);
		return FALSE;
	}
	
	if (ftype != HYP_FT_HYP)
	{
		hyp_utf8_close(handle);
		hyp_unref(hyp);
		hyp_utf8_fprintf(opts->errorfile, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_utf8_close(handle);
		hyp_unref(hyp);
		hyp_utf8_fprintf(opts->errorfile, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, filename);
		return FALSE;
	}
	if (hyp->comp_vers > HCP_COMPILER_VERSION)
		hyp_utf8_fprintf(opts->errorfile, _("%s%s created by compiler version %u\n"), _("warning: "), hyp->file, hyp->comp_vers);
	if (opts->verbose)
	{
		hyp_utf8_fprintf(stdout, "%s: ", filename);
	}
	
	retval = search_hyp(hyp, opts);
	hyp_utf8_close(handle);
	hyp_unref(hyp);
	opts->total_hits += opts->hits;
	opts->filecount++;
	
	if (opts->verbose)
	{
		if (opts->hits != 0)
			hyp_utf8_fprintf(stdout, _(" %lu matches"), opts->hits);
		hyp_utf8_fprintf(stdout, "\n");
	}
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean maybe_process_file(const char *filename, void *data)
{
	if (hyp_guess_filetype(filename) != HYP_FT_HYP)
		return TRUE;
	if (strcasecmp(hyp_basename(filename), hypfind_hyp) == 0)
		return TRUE;
	process_file(filename, (struct hypfind_opts *)data);
	/* ignore errors and continue scanning directories */
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean compile_output(struct hypfind_opts *opts)
{
	char *hcp = path_subst(gl_profile.general.hcp_path);
	gboolean retval = TRUE;
	const char *argv[10];
	int argc;
	int retcode;
	int i;
	
	argc = 0;
	argv[argc++] = hcp;
	argv[argc++] = "-q";
	argv[argc++] = "-q";
	argv[argc++] = "-q";
	argv[argc++] = "-o";
	argv[argc++] = opts->hypfind_hyp;
	argv[argc++] = opts->hypfind_stg;
	argv[argc] = NULL;
	if (opts->verbose)
	{
		for (i = 0; i < argc; i++)
		{
			putc(i == 0 ? '>' : ' ', stdout);
			hyp_utf8_fprintf(stdout, "%s", argv[i]);
		}
		fputs("\n", stdout);
	}
	retcode = hyp_utf8_spawnvp(P_WAIT, argc, argv);
	if (retcode < 0)
	{
		hyp_utf8_fprintf(stderr, _("can't find %s: %s\n"), printnull(hcp), hyp_utf8_strerror(errno));
		retval = FALSE;
	} else if (retcode != 0)
	{
		hyp_utf8_fprintf(stderr, _("hcp exit code was %d\n"), retcode);
		retval = FALSE;
	}
	g_free(hcp);
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean hypfind_opts_parse(struct hypfind_opts *opts, int argc, const char **argv, opts_origin origin)
{
	struct _getopt_data *d;
	gboolean retval = TRUE;
	int c;
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(argc, argv, "cqiIp:whV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case 'q':
			opts->verbose = FALSE;
			break;
		case 'c':
			opts->do_compile = FALSE;
			break;
		case 'i':
			opts->casesensitive = FALSE;
			break;
		case 'I':
			opts->casesensitive = TRUE;
			break;
		case 'w':
			opts->wordonly = TRUE;
			break;
		case 'p':
			if (opts->pattern)
			{
				usage_error(_("can only search for one pattern"));
				retval = FALSE;
			} else
			{
				opts->pattern = g_strdup(getopt_arg_r(d));
			}
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

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#include "hypmain.h"

int main(int argc, const char **argv)
{
	int c;
	int retval = 0;
	int num_args;
	struct hypfind_opts _opts, *opts;
	gboolean delete_stg = FALSE;
	gboolean delete_hyp = FALSE;
	
	opts = &_opts;
	memset(opts, 0, sizeof(*opts));
	opts->do_compile = TRUE;
	opts->verbose = TRUE;
	opts->casesensitive = FALSE;
	opts->wordonly = FALSE;
	opts->pattern = NULL;
	opts->do_help = FALSE;
	opts->do_version = FALSE;
	opts->error_filename = NULL;
	opts->errorfile = stderr;
	opts->output_charset = hyp_get_current_charset();
	bm_init(&opts->deltapat, NULL, TRUE);
	
	HypProfile_Load(TRUE);
	
	{
		char *dirname = path_subst(gl_profile.general.hypfold);
		opts->hypfind_stg = g_build_filename(dirname, hypfind_stg, NULL);
		opts->hypfind_hyp = g_build_filename(dirname, hypfind_hyp, NULL);
		g_free(dirname);
	}
	
	if (!hypfind_opts_parse(opts, argc, argv, OPTS_FROM_COMMANDLINE))
		retval = 1;
	c = opts->optind;
	num_args = argc - c;
	
	if (retval != 0)
	{
	} else if (opts->do_version)
	{
		print_version(stdout);
	} else if (opts->do_help)
	{
		print_usage(stdout);
	} else
	{
		if (opts->pattern == NULL)
		{
			usage_error(_("missing search pattern"));
			retval = 1;
		} else if ((opts->pattern_len = strlen(opts->pattern)) == 0)
		{
			usage_error(_("empty search pattern"));
			retval = 1;
		}
	
		if (retval == 0)
		{
			if ((opts->errorfile == NULL || opts->errorfile == stderr) && opts->error_filename != NULL)
			{
				opts->errorfile = hyp_utf8_fopen(opts->error_filename, "w");
				if (opts->errorfile == NULL)
				{
					hyp_utf8_fprintf(stderr, "%s: %s: %s\n", gl_program_name, opts->error_filename, hyp_utf8_strerror(errno));
					opts->errorfile = stderr;
				}
			}
		}
		
		if (retval == 0)
		{
			if (opts->verbose)
				print_short_version(stdout);
		}
		
		if (retval == 0)
		{
			opts->outfile = hyp_utf8_fopen(opts->hypfind_stg, "w");
			if (opts->outfile == NULL)
			{
				hyp_utf8_fprintf(stderr, "%s: %s: %s\n", gl_program_name, opts->hypfind_stg, hyp_utf8_strerror(errno));
				retval = 1;
			}
		}
		
		if (retval == 0)
		{
			char *str, *str2;
			
			hyp_utf8_fprintf(opts->outfile, "@if VERSION >= 6\n");
			hyp_utf8_fprintf(opts->outfile, "@os %s\n", hyp_osname(hyp_get_current_os()));
			hyp_utf8_fprintf(opts->outfile, "@charset %s\n", hyp_charset_name(hyp_get_current_charset()));
			hyp_utf8_fprintf(opts->outfile, "@inputenc %s\n", hyp_charset_name(opts->output_charset));
			hyp_utf8_fprintf(opts->outfile, "@endif\n");
			
			str = stg_quote_name(gl_profile.hypfind.database, STR0TERM);
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@database \"%s\"\n", str);
			g_free(str);
			str = stg_quote_name(gl_profile.hypfind.subject, STR0TERM);
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@subject \"%s\"\n", str);
			g_free(str);
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@author \"Program %s\"\n", gl_program_name);
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@$VER: %s 1.00 (@:__DATE__)\n", hypfind_hyp);
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@options \"-s -i\"\n");
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "\n");

			str = stg_quote_name(gl_profile.hypfind.title, STR0TERM);
			str2 = stg_quote_name(opts->pattern, STR0TERM);
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@node Main \"%s%s\"\n", str, str2);
			g_free(str2);
			g_free(str);
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "\n");
		}
		
		if (retval == 0)
		{
			if (opts->verbose)
				hyp_utf8_fprintf(stdout, _("searching for <%s>%s...\n"), opts->pattern, opts->wordonly ? _(" as word") : "");
			if (!bm_init(&opts->deltapat, opts->pattern, opts->casesensitive))
				retval = 1;
		}
		
		if (retval == 0)
		{
			if (num_args == 0)
			{
				opts->multiple = TRUE;
				walk_pathlist(gl_profile.general.path_list, maybe_process_file, opts);
			} else
			{
				int i;
				const char *filename;
				
				opts->multiple = num_args > 1;
				for (i = 0; i < num_args; i++)
				{
					filename = argv[opts->optind + i];
					if (process_file(filename, opts) == FALSE)
					{
						retval = 1;
						delete_hyp = TRUE;
					}
				}
			}
		}
		
		if (retval == 0)
		{
			hyp_utf8_fprintf_charset(opts->outfile, opts->output_charset, "@endnode\n");
		}
		
		if (opts->outfile != NULL && opts->outfile != stdout)
		{
			hyp_utf8_fclose(opts->outfile);
			opts->outfile = NULL;
		}

		if (retval == 0)
		{
			if (opts->total_hits == 0)
			{
				if (opts->verbose)
					hyp_utf8_fprintf(opts->errorfile, _("%s: No matches for this pattern!\n"), gl_program_name);
				retval = 1;
				delete_stg = TRUE;
				if (opts->do_compile)
					delete_hyp = TRUE;
			} else
			{
				if (opts->verbose)
					hyp_utf8_fprintf(stdout, _("%lu matches total in %lu files\n"), opts->total_hits, opts->filecount);
			}
		}
		
		bm_exit(&opts->deltapat);
		
		if (retval == 0 && opts->do_compile)
		{
			if (!compile_output(opts))
			{
				retval = 1;
				delete_stg = FALSE;
				delete_hyp = TRUE;
			} else
			{
				delete_stg = TRUE;
			}
		}
		
		if (opts->outfile != NULL && opts->outfile != stdout)
		{
			hyp_utf8_fclose(opts->outfile);
			opts->outfile = NULL;
		}

		fflush(opts->errorfile);
		if (opts->errorfile != NULL && opts->errorfile != stderr)
		{
			fclose(opts->errorfile);
			opts->errorfile = NULL;
		}
		
		if (delete_stg)
			hyp_utf8_unlink(opts->hypfind_stg);
		if (delete_hyp)
			hyp_utf8_unlink(opts->hypfind_hyp);
	}
	
	g_free(opts->pattern);
	g_free(opts->hypfind_stg);
	g_free(opts->hypfind_hyp);
	
	HypProfile_Delete();
	x_free_resources();

	return retval;
}
