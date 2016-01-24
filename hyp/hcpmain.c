#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

char const gl_program_name[] = "hcp";

static const char *stg_nl;
static gboolean is_MASTER;
static HYP_CHARSET output_charset;

typedef gboolean (*recompile_func)(HYP_DOCUMENT *hyp, hcp_opts *opt, int argc, const char **argv);


/*
 * node/label names are case sensitiv
 */
#define namecmp strcmp


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
	char *compiler = hyp_compiler_version();
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n"
		"Using %s\n"
		"%s\n",
		gl_program_name, printnull(version),
		HYP_COPYRIGHT,
		printnull(compiler),
		printnull(url));
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", printnull(msg));
	g_free(msg);
	g_free(compiler);
	g_free(version);
	g_free(url);
}

/* ------------------------------------------------------------------------- */

#if 0
static void print_short_version(FILE *out)
{
	char *version = gl_program_version();
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n\n",
		gl_program_name, printnull(version),
		HYP_COPYRIGHT);
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", printnull(msg));
	g_free(msg);
	g_free(version);
}
#endif

/* ------------------------------------------------------------------------- */

static void print_usage(FILE *out)
{
	print_version(out);
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("usage: %s [+-options] file1 [+-options] file2 ...\n"), gl_program_name);
	hyp_utf8_fprintf(out, _("or:    %s -v [-oFILE] hypertext-file node1 node2 ...\n"), gl_program_name);
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("options:\n"));
	hyp_utf8_fprintf(out, _("  +-a, --[no-]autoref           auto references +on/-off\n"));
	hyp_utf8_fprintf(out, _("  -b, --blocksize [SIZE]        set max. node size to SIZE kB\n"));
	hyp_utf8_fprintf(out, _("  +-c, --[no-]compression       turn compression +on/-off\n"));
	hyp_utf8_fprintf(out, _("  -d, --ref-distance [WIDTH]    minimal reference distance\n"));
	hyp_utf8_fprintf(out, _("  -e, --errorfile [FILE]        set error filename\n"));
	hyp_utf8_fprintf(out, _("  +-f, --[no-]alias-in-index    turn automatic adding of alias to index on/off\n"));
	hyp_utf8_fprintf(out, _("  +-g, --[no-]alabel-in-index   turn automatic adding of alabel to index on/off\n"));
	hyp_utf8_fprintf(out, _("  +-i, --[no-]index             turn index-table generation +on/-off\n"));
	hyp_utf8_fprintf(out, _("  -j, --index-width [WIDTH]     minimal index-column width\n"));
	hyp_utf8_fprintf(out, _("  -k, --compat-flags [VAL]      set compatibility flags\n"));
	hyp_utf8_fprintf(out, _("  -l, --list {FLAGS}            list contents of hypertext file\n"));
	hyp_utf8_fprintf(out, _("  +-m, --[no-]images            don't read images (test mode)\n"));
	hyp_utf8_fprintf(out, _("  +-n, --[no-]nodes-in-index    don't add nodes to index table\n"));
	hyp_utf8_fprintf(out, _("  -o, --output [FILE]           set output file name\n"));
	hyp_utf8_fprintf(out, _("  -p, --pic-format [VAL]        image type for recompiling\n"));
	hyp_utf8_fprintf(out, _("  -q[qq], --quiet               set quiet mode\n"));
	hyp_utf8_fprintf(out, _("  -r, --recompile               recompile a hypertext file\n"));
	hyp_utf8_fprintf(out, _("  +-s, --[no-]split             -don't split long lines\n"));
	hyp_utf8_fprintf(out, _("  -t, --tabwidth [VAL]          set tabulator width\n"));
	hyp_utf8_fprintf(out, _("  -u, --uses [FILE]             add a '@uses' file\n"));
	hyp_utf8_fprintf(out, _("  -v, --view                    view listed nodes as ASCII\n"));
	hyp_utf8_fprintf(out, _("  -w, --wait {VAL}              wait for keypress for exiting\n"));
	hyp_utf8_fprintf(out, _("  +-x, --[no]-title-in-index    use +title instead of name for index\n"));
	hyp_utf8_fprintf(out, _("  +-y, --caseinsensitive-first  first char is case insensitive\n"));
	hyp_utf8_fprintf(out, _("  -z[z], --references           write reference file\n"));
	hyp_utf8_fprintf(out, _("                                zz also updates default-reference-file\n"));
	hyp_utf8_fprintf(out, _("  -h, --help                    print help and exit\n"));
	hyp_utf8_fprintf(out, _("  -V, --version                 print version and exit\n"));
}


/* ------------------------------------------------------------------------- */

static void oom(void)
{
	hyp_utf8_fprintf(stderr, "%s: %s\n", gl_program_name, strerror(ENOMEM));
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *ascii_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
}

/* ------------------------------------------------------------------------- */

static void ascii_out_text(FILE *outfile, const char *text, unsigned char textattr)
{
	const char *p = text;
	int i;
	char buf[HYP_UTF8_CHARMAX + 1];
	gboolean converror = FALSE;
	
	while (*p)
	{
		if (textattr & HYP_TXT_UNDERLINED)
		{
			fputc('_', outfile);
			fputc('\b', outfile);
		}
		p = hyp_utf8_conv_char(output_charset, p, buf, &converror);
		for (i = 0; buf[i] != 0; i++)
			fputc(buf[i], outfile);
		if (textattr & HYP_TXT_BOLD)
		{
			fputc('\b', outfile);
			for (i = 0; buf[i] != 0; i++)
				fputc(buf[i], outfile);
		}
	}
}

/* ------------------------------------------------------------------------- */

static void ascii_out_str(HYP_DOCUMENT *hyp, FILE *outfile, const unsigned char *str, size_t len, unsigned char textattr)
{
	char *text = hyp_conv_to_utf8(hyp->comp_charset, str, len);
	ascii_out_text(outfile, text, textattr);
	g_free(text);
}

/* ------------------------------------------------------------------------- */

static gboolean ascii_out_attr(FILE *outfile, unsigned char oldattr, unsigned char newattr)
{
	UNUSED(outfile);
	if (oldattr != newattr)
	{
		return TRUE;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static char *invalid_page(hyp_nodenr page)
{
	return g_strdup_printf(_("<invalid destination page %u>"), page);
}

/* ------------------------------------------------------------------------- */

static gboolean ascii_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	unsigned char textattr;
	long lineno;
	gboolean manlike;
	HYP_NODE *nodeptr;
	char *title;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		ascii_out_str(hyp, opts->outfile, textstart, src - textstart, manlike ? textattr : 0); \
		at_bol = FALSE; \
	}
#define FLUSHLINE() \
	if (!at_bol) \
	{ \
		fputs(stg_nl, opts->outfile); \
		at_bol = TRUE; \
	}
#define FLUSHTREE() \
	if (in_tree != -1) \
	{ \
		hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s", stg_nl); \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		struct stat st;
		
		manlike = FALSE;
		if (rpl_fstat(fileno(opts->outfile), &st) == 0)
		{
			if (S_ISFIFO(st.st_mode) || S_ISCHR(st.st_mode))
				manlike = TRUE;
		}
		hyp_node_find_windowtitle(nodeptr);
		title = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
		if (title == NULL)
			title = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[node]->name, STR0TERM);
		hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s%s", title, stg_nl);
		g_free(title);
		
		end = nodeptr->end;

		/*
		 * scan through esc commands, skipping graphic commands
		 */
		src = nodeptr->start;
		while (src < end && *src == HYP_ESC)
		{
			switch (src[1])
			{
			case HYP_ESC_PIC:
			case HYP_ESC_LINE:
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				break;
			case HYP_ESC_WINDOWTITLE:
				break;
			case HYP_ESC_EXTERNAL_REFS:
				{
					hyp_nodenr dest_page;
					char *text;
					
					dest_page = DEC_255(&src[3]);
					text = hyp_conv_to_utf8(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
					chomp(&text);
					if (hypnode_valid(hyp, dest_page))
					{
						str = ascii_quote_nodename(hyp, dest_page);
					} else
					{
						str = invalid_page(dest_page);
					}
					if (empty(text) || strcmp(str, text) == 0)
						hyp_utf8_fprintf_charset(opts->outfile, output_charset, "See also: %s%s", str, stg_nl);
					else
						hyp_utf8_fprintf_charset(opts->outfile, output_charset, "See also: %s%s", text, stg_nl);
					g_free(str);
					g_free(text);
				}
				break;
			default:
				break;
			}
			src = hyp_skip_esc(src);
		}

		/*
		 * now output data
		 */
		src = nodeptr->start;
		textstart = src;
		at_bol = TRUE;
		in_tree = -1;
		textattr = 0;
		lineno = 0;
		
		while (src < end)
		{
			if (*src == HYP_ESC)
			{
				DUMPTEXT();
				src++;
				switch (*src)
				{
				case HYP_ESC_ESC:
					FLUSHTREE();
					fputc(0x1b, opts->outfile);
					at_bol = FALSE;
					src++;
					break;
				
				case HYP_ESC_WINDOWTITLE:
					src++;
					FLUSHTREE();
					FLUSHLINE();
					/* @title already output */
					src += ustrlen(src) + 1;
					break;

				case HYP_ESC_CASE_DATA:
					FLUSHTREE();
					FLUSHLINE();
					src += src[1] - 1;
					break;
				
				case HYP_ESC_LINK:
				case HYP_ESC_LINK_LINE:
				case HYP_ESC_ALINK:
				case HYP_ESC_ALINK_LINE:
					{
						hyp_nodenr dest_page;
						unsigned char type;
						char *dest;
						size_t len;
						
						type = *src;
						if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
						{
							/* line = DEC_255(&src[1]); */
							src += 2;
						}
						dest_page = DEC_255(&src[1]);
						src += 3;
						if (hypnode_valid(hyp, dest_page))
						{
							dest = ascii_quote_nodename(hyp, dest_page);
						} else
						{
							dest = invalid_page(dest_page);
						}
						if (*src <= HYP_STRLEN_OFFSET)
						{
							src++;
							str = g_strdup(dest);
						} else
						{
							len = *src - HYP_STRLEN_OFFSET;
							src++;
							textstart = src;
							str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
							src += len;
						}
						FLUSHTREE();
						ascii_out_text(opts->outfile, str, manlike ? HYP_TXT_BOLD : 0);
						g_free(dest);
						g_free(str);
						at_bol = FALSE;
					}
					break;
					
				case HYP_ESC_EXTERNAL_REFS:
					FLUSHTREE();
					FLUSHLINE();
					/* @xref already output */
					src += src[1] - 1;
					break;
					
				case HYP_ESC_OBJTABLE:
					{
						_WORD tree;

						tree = DEC_255(&src[3]);
						FLUSHLINE();
						if (tree != in_tree)
						{
							FLUSHTREE();
							in_tree = tree;
						}
						src += 9;
					}
					break;
					
				case HYP_ESC_PIC:
					FLUSHTREE();
					FLUSHLINE();
					src += 8;
					break;
					
				case HYP_ESC_LINE:
					FLUSHTREE();
					FLUSHLINE();
					src += 7;
					break;
					
				case HYP_ESC_BOX:
				case HYP_ESC_RBOX:
					FLUSHTREE();
					FLUSHLINE();
					src += 7;
					break;
					
				case HYP_ESC_CASE_TEXTATTR:
					if (ascii_out_attr(opts->outfile, textattr, *src - HYP_ESC_TEXTATTR_FIRST))
						at_bol = FALSE;
					textattr = *src - HYP_ESC_TEXTATTR_FIRST;
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
				FLUSHTREE();
				DUMPTEXT();
				fputs(stg_nl, opts->outfile);
				at_bol = TRUE;
				++lineno;
				src++;
				textstart = src;
			} else
			{
				FLUSHTREE();
				src++;
			}
		}
		DUMPTEXT();
		if (ascii_out_attr(opts->outfile, textattr, 0))
			at_bol = FALSE;
		FLUSHLINE();
		FLUSHTREE();
		++lineno;
		
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(opts->errorfile, _("%s: Node %u: failed to decode\n"), hyp->file, node);
	}

#undef DUMPTEXT
#undef FLUSHLINE
#undef FLUSHTREE
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean recompile_ascii(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	hcp_opts hyp_opts;
	int i;
	gboolean found;
	
	stg_nl = opts->outfile == stdout ? "\n" : "\015\012";
	
	ret = TRUE;
	hcp_opts_copy(&hyp_opts, opts);
	if (hyp->hcp_options != NULL)
	{
		hcp_opts_parse_string(&hyp_opts, hyp->hcp_options, OPTS_FROM_SOURCE);
	}
		
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		if (argc == 0)
		{
			found = TRUE;
		} else
		{
			found = FALSE;
			for (i = 0; i < argc; i++)
			{
				if (argv[i])
				{
					char *s1, *s2;
					
					s1 = hyp_conv_to_utf8(hyp->comp_charset, entry->name, STR0TERM);
					s2 = hyp_conv_to_utf8(hyp_get_current_charset(), argv[i], STR0TERM);
					if (namecmp(s1, s2) == 0)
					{
						argv[i] = NULL;
						found = TRUE;
					}
					g_free(s2);
					g_free(s1);
				}
			}
		}
		if (!found)
			continue;
		if (node == hyp->index_page)
		{
			/*
			 * skip recompiling index page, assuming
			 * that it will be regenerated when
			 * compiling the source file.
			 * Should probably check wether the existing page
			 * is an automatically generated one,
			 * but no idea how to achieve that.
			 * Looking at the option string does not seem
			 * to work, all existing hypertexts i have seen
			 * have "-i" set wether they contain an index or not.
			 */
			if (!opts->gen_index && argc == 0)
				continue;
		}
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= ascii_out_node(hyp, &hyp_opts, node);
			hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s%s%s", stg_nl, stg_nl, stg_nl);
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
	
	for (i = 0; i < argc; i++)
	{
		if (argv[i] != NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, _("'%s' not found.\n"), argv[i]);
			ret = FALSE;
		}
	}
	
	hyp_opts.outfile = NULL;
	hyp_opts.errorfile = NULL;
	hcp_opts_free(&hyp_opts);

	return ret;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *stg_quote_name(HYP_CHARSET charset, const unsigned char *name, size_t len)
{
	char *str, *ret;
	char *buf;
	
	buf = hyp_conv_to_utf8(charset, name, len);
	len = strlen(buf);
	str = ret = g_new(char, len * 2 + 1);
	if (str != NULL)
	{
		name = (const unsigned char *) buf;
		while (*name)
		{
			if (*name == '\\' || *name == '"')
				*str++ = '\\';
			*str++ = *name++;
			len--;
		}
		*str++ = '\0';
		ret = (char *)g_realloc(ret, str - ret);
	}
	g_free(buf);
	return ret;
}

/* ------------------------------------------------------------------------- */

static char *stg_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return stg_quote_name(hyp->comp_charset, entry->name, namelen);
}

/* ------------------------------------------------------------------------- */

static void stg_out_globals(HYP_DOCUMENT *hyp, FILE *outfile)
{
	char *str;

	hyp_utf8_fprintf_charset(outfile, output_charset, "@if VERSION >= 6%s", stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@os %s%s", hyp_osname(hyp->comp_os), stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@charset %s%s", hyp_charset_name(hyp->comp_charset), stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@inputenc %s%s", hyp_charset_name(output_charset), stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@endif%s", stg_nl);
	
	if (hyp->database != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@database \"%s\"%s", hyp->database, stg_nl);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			hyp_utf8_fprintf_charset(outfile, output_charset, "@hostname \"%s\"%s", h->name, stg_nl);
		}
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = stg_quote_nodename(hyp, hyp->default_page);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@default \"%s\"%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@options \"%s\"%s", hyp->hcp_options, stg_nl);
	}
	if (hyp->author != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@author \"%s\"%s", hyp->author, stg_nl);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = stg_quote_nodename(hyp, hyp->help_page);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@help \"%s\"%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@$VER: %s%s", hyp->version, stg_nl);
	}
	if (hyp->subject != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@subject \"%s\"%s", hyp->subject, stg_nl);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@width %d%s", hyp->line_width, stg_nl);
	}
#if 0
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, _("ST-Guide flags: $%04x%s"), hyp->st_guide_flags, stg_nl);
	}
#endif
	fputs(stg_nl, outfile);
	fputs(stg_nl, outfile);
}

/* ------------------------------------------------------------------------- */

static void stg_out_str(FILE *outfile, HYP_CHARSET charset, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(charset, output_charset, str, len, &converror);
	p = dst;
	while (*p)
	{
		if (*p == '@')
			fputc('@', outfile); /* AmigaGuide uses \@ for quoted '@' */
		fputc(*p, outfile);
		p++;
	}
	g_free(dst);
}

/* ------------------------------------------------------------------------- */

static gboolean stg_out_attr(FILE *outfile, unsigned char oldattr, unsigned char newattr)
{
	if (oldattr != newattr)
	{
		fputc('@', outfile);
		fputc('{', outfile);
#define onoff(mask, on, off) \
		if ((oldattr ^ newattr) & mask) \
			fputc(newattr & mask ? on : off, outfile)
		onoff(HYP_TXT_BOLD, 'B', 'b');
		onoff(HYP_TXT_LIGHT, 'G', 'g');
		onoff(HYP_TXT_ITALIC, 'I', 'i');
		onoff(HYP_TXT_UNDERLINED, 'U', 'u');
		onoff(HYP_TXT_OUTLINED, 'O', 'o');
		onoff(HYP_TXT_SHADOWED, 'S', 's');
#undef onoff
		fputc('}', outfile);
		return TRUE;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static char *image_name(hyp_pic_format format, HYP_DOCUMENT *hyp, hyp_nodenr node, const char *name_prefix)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	const char *ext;
	char *name;
	char *res;
	
	switch (format)
	{
	case HYP_PIC_IFF:
		ext = ".iff";
		break;
	case HYP_PIC_ICN:
		ext =".icn";
		break;
	case HYP_PIC_IMG:
		ext = ".img";
		break;
	case HYP_PIC_BMP:
		ext = ".bmp";
		break;
	case HYP_PIC_UNKNOWN:
	default:
		ext = ".dta";
		break;
	}
	/*
	 * some newer hypertext files (hcp version >= 5) seem to have the
	 * basename of the original image in the node name. For most files,
	 * the node name for images is empty
	 */
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	if (namelen > 0)
	{
		name = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
		res = replace_ext(name, NULL, ext);
		g_free(name);
		return res;
	}
	return g_strdup_printf("%s%05u%s", name_prefix, (unsigned int)node, ext);
}

/* ------------------------------------------------------------------------- */

static hyp_pic_format format_from_pic(hcp_opts *opts, INDEX_ENTRY *entry)
{
	hyp_pic_format format;

	format = opts->pic_format;
	if (format == HYP_PIC_ORIG)
	{
		/*
		 * not documented, but HCP seems to write the
		 * orignal file format into the "up" field
		 */
		format = (hyp_pic_format)entry->toc_index;
	}
	if (format < 1 || format > HYP_PIC_LAST)
	{
		format = HYP_PIC_IMG;
		hyp_utf8_fprintf(opts->errorfile, _("unknown image source type, using IMG instead\n"));
	}
	
	return format;
}

/* ------------------------------------------------------------------------- */

static void stg_out_gfx(hcp_opts *opts, HYP_DOCUMENT *hyp, struct hyp_gfx *adm)
{
	switch (adm->type)
	{
	case HYP_ESC_PIC:
		{
			char *fname;
			
			if (!hypnode_valid(hyp, adm->extern_node_index))
				fname = invalid_page(adm->extern_node_index);
			else if (hyp->indextable[adm->extern_node_index]->type != HYP_NODE_IMAGE)
				fname = g_strdup_printf(_("<non-image node #%u>"), adm->extern_node_index);
			else
				fname = image_name(adm->format, hyp, adm->extern_node_index, opts->image_name_prefix);
			hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@remark %ux%ux%u%s", adm->pixwidth, adm->pixheight, 1 << adm->planes, stg_nl);
			hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s \"%s\" %d%s",
				adm->islimage ? "@limage" : "@image",
				fname,
				adm->x_offset,
				stg_nl);
			g_free(fname);
		}
		break;
	case HYP_ESC_LINE:
		hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@line %d %d %d %d %d%s",
			adm->x_offset, adm->width, adm->height,
			adm->begend,
			adm->style,
			stg_nl);
		break;
	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s %d %d %d %d%s",
			adm->type == HYP_ESC_BOX ? "@box" : "@rbox",
			adm->x_offset, adm->width, adm->height, adm->style,
			stg_nl);
		break;
	}
}

/* ------------------------------------------------------------------------- */

static void stg_out_graphics(hcp_opts *opts, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, long lineno)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			stg_out_gfx(opts, hyp, gfx);
		}
		gfx = gfx->next;
	}
}

/* ------------------------------------------------------------------------- */

static gboolean stg_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	unsigned char textattr;
	long lineno;
	struct hyp_gfx *hyp_gfx = NULL;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		stg_out_str(opts->outfile, hyp->comp_charset, textstart, src - textstart); \
		at_bol = FALSE; \
	}
#define FLUSHLINE() \
	if (!at_bol) \
	{ \
		fputs(stg_nl, opts->outfile); \
		at_bol = TRUE; \
	}
#define FLUSHTREE() \
	if (in_tree != -1) \
	{ \
		hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@endtree%s", stg_nl); \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		
		{
			INDEX_ENTRY *entry;
			
			entry = hyp->indextable[node];

			hyp_node_find_windowtitle(nodeptr);
			
			str = stg_quote_nodename(hyp, node);
			if (nodeptr->window_title)
			{
				char *title = stg_quote_name(hyp->comp_charset, nodeptr->window_title, STR0TERM);
				hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s \"%s\" \"%s\"%s", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str, title, stg_nl);
				g_free(title);
			} else
			{
				hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s \"%s\"%s", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str, stg_nl);
			}
			g_free(str);

			if (entry->type == HYP_NODE_INTERNAL)
			{
				if (hypnode_valid(hyp, entry->next) &&
					/* physical next page is default if not set */
					(node + 1u) != entry->next)
				{
					str = stg_quote_nodename(hyp, entry->next);
					hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@next \"%s\"%s", str, stg_nl);
					g_free(str);
				}
				
				if (hypnode_valid(hyp, entry->previous) &&
					 /* physical prev page is default if not set */
					(node - 1u) != entry->previous &&
					(entry->previous != 0 || node != 0))
				{
					str = stg_quote_nodename(hyp, entry->previous);
					hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@prev \"%s\"%s", str, stg_nl);
					g_free(str);
				}
				
				if (hypnode_valid(hyp, entry->toc_index) &&
					/* physical first page is default if not set */
					entry->toc_index != 0)
				{
					str = stg_quote_nodename(hyp, entry->toc_index);
					hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@toc \"%s\"%s", str, stg_nl);
					g_free(str);
				}
			}
		}
				
		if (node == hyp->index_page)
			hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@autorefon%s", stg_nl);

		end = nodeptr->end;

		/*
		 * scan through esc commands, gathering graphic commands
		 */
		src = nodeptr->start;
		while (retval && src < end && *src == HYP_ESC)
		{
			switch (src[1])
			{
			case HYP_ESC_PIC:
			case HYP_ESC_LINE:
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				{
					struct hyp_gfx *adm, **last;
					
					last = &hyp_gfx;
					while (*last != NULL)
						last = &(*last)->next;
					adm = g_new0(struct hyp_gfx, 1);
					if (adm == NULL)
					{
						retval = FALSE;
					} else
					{
						*last = adm;
						hyp_decode_gfx(hyp, src + 1, adm);
						if (adm->type == HYP_ESC_PIC)
							adm->format = format_from_pic(opts, hyp->indextable[adm->extern_node_index]);
					}
				}
				break;
			case HYP_ESC_WINDOWTITLE:
				/* @title already output */
				break;
			case HYP_ESC_EXTERNAL_REFS:
				{
					hyp_nodenr dest_page;
					char *text;
					
					dest_page = DEC_255(&src[3]);
					text = stg_quote_name(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
					chomp(&text);
					if (hypnode_valid(hyp, dest_page))
					{
						str = stg_quote_nodename(hyp, dest_page);
					} else
					{
						str = invalid_page(dest_page);
					}
					if (empty(text) || strcmp(str, text) == 0)
						hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@xref \"%s\"%s", str, stg_nl);
					else
						hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@xref \"%s\" \"%s\"%s", str, text, stg_nl);
					g_free(str);
					g_free(text);
				}
				break;
			default:
				break;
			}
			src = hyp_skip_esc(src);
		}

		/*
		 * now output data
		 */
		src = nodeptr->start;
		textstart = src;
		at_bol = TRUE;
		in_tree = -1;
		textattr = 0;
		lineno = 0;
		stg_out_graphics(opts, hyp, hyp_gfx, lineno);
		
		while (retval && src < end)
		{
			if (*src == HYP_ESC)
			{
				DUMPTEXT();
				src++;
				switch (*src)
				{
				case HYP_ESC_ESC:
					FLUSHTREE();
					fputc(0x1b, opts->outfile);
					at_bol = FALSE;
					src++;
					break;
				
				case HYP_ESC_WINDOWTITLE:
					src++;
					FLUSHTREE();
					FLUSHLINE();
					/* @title already output */
					src += ustrlen(src) + 1;
					break;

				case HYP_ESC_CASE_DATA:
					FLUSHTREE();
					FLUSHLINE();
					src += src[1] - 1;
					break;
				
				case HYP_ESC_LINK:
				case HYP_ESC_LINK_LINE:
				case HYP_ESC_ALINK:
				case HYP_ESC_ALINK_LINE:
					{
						hyp_nodenr dest_page;
						unsigned char type;
						hyp_lineno line = 0;
						char *dest;
						size_t len;
						gboolean str_equal;
						const char *cmd;
						gboolean print_target;
						
						type = *src;
						if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
						{
							line = DEC_255(&src[1]);
							src += 2;
						}
						dest_page = DEC_255(&src[1]);
						src += 3;
						print_target = TRUE;
						if (hypnode_valid(hyp, dest_page))
						{
							dest = stg_quote_nodename(hyp, dest_page);
							switch ((hyp_indextype) hyp->indextable[dest_page]->type)
							{
							default:
							case HYP_NODE_EOF:
								if (opts->print_unknown)
									hyp_utf8_fprintf(opts->errorfile, _("link to unknown node type %u\n"), hyp->indextable[dest_page]->type);
								/* fall through */
							case HYP_NODE_INTERNAL:
							case HYP_NODE_POPUP:
							case HYP_NODE_EXTERNAL_REF:
								cmd = type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE ? "ALINK" : "LINK";
								break;
							case HYP_NODE_REXX_COMMAND:
								cmd = "RX";
								break;
							case HYP_NODE_REXX_SCRIPT:
								cmd = "RXS";
								break;
							case HYP_NODE_SYSTEM_ARGUMENT:
								cmd = "SYSTEM";
								break;
							case HYP_NODE_IMAGE:
								cmd = "IMAGE";
								break;
							case HYP_NODE_QUIT:
								cmd = "QUIT";
								print_target = FALSE;
								break;
							case HYP_NODE_CLOSE:
								cmd = "CLOSE";
								print_target = FALSE;
								break;
							}
						} else
						{
							dest = invalid_page(dest_page);
							cmd = "LINK";
						}
						if (*src <= HYP_STRLEN_OFFSET)
						{
							src++;
							str = g_strdup(dest);
							if (hypnode_valid(hyp, dest_page))
							{
								INDEX_ENTRY *entry = hyp->indextable[dest_page];
								len = entry->length - SIZEOF_INDEX_ENTRY;
								textstart = entry->name;
								str_equal = entry->type == HYP_NODE_INTERNAL;
							} else
							{
								textstart = (const unsigned char *)str;
								len = strlen(str);
								str_equal = FALSE;
							}
						} else
						{
							len = *src - HYP_STRLEN_OFFSET;
							src++;
							textstart = src;
							str = stg_quote_name(hyp->comp_charset, src, len);
							src += len;
							if (hypnode_valid(hyp, dest_page))
							{
								INDEX_ENTRY *entry = hyp->indextable[dest_page];
								str_equal = entry->type == HYP_NODE_INTERNAL && strcmp(str, dest) == 0;
							} else
							{
								str_equal = FALSE;
							}
						}
						FLUSHTREE();
						switch (type)
						{
						case HYP_ESC_LINK:
						case HYP_ESC_ALINK:
							if (!print_target)
								hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@{\"%s\" %s}", str, cmd);
							else if (!str_equal /* || node == hyp->index_page */ || !opts->autoreferences)
								hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@{\"%s\" %s \"%s\"}", str, cmd, dest);
							else
								stg_out_str(opts->outfile, hyp->comp_charset, textstart, len);
							break;
						case HYP_ESC_LINK_LINE:
						case HYP_ESC_ALINK_LINE:
							{
								gboolean is_rsc_link = FALSE;
								char *p = strrchr(dest, '/');
								if (p != NULL && strcmp(p, "/MAIN") == 0)
								{
									*p = '\0';
									if (hyp_guess_filetype(dest) == HYP_FT_RSC)
									{
										hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@{\"%s\" %s \"%s/%u\"}", str, cmd, dest, line);
										is_rsc_link = TRUE;
									} else
									{
										*p = '/';
									}
								}
								if (!is_rsc_link)
									hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@{\"%s\" %s \"%s\" %u}", str, cmd, dest, line + 1);
							}
							break;
						}
						g_free(dest);
						g_free(str);
						at_bol = FALSE;
					}
					break;
					
				case HYP_ESC_EXTERNAL_REFS:
					FLUSHTREE();
					FLUSHLINE();
					/* @xref already output */
					src += src[1] - 1;
					break;
					
				case HYP_ESC_OBJTABLE:
					{
						hyp_nodenr dest_page;
						_WORD tree, obj;
						hyp_lineno line;
						
						line = DEC_255(&src[1]);
						tree = DEC_255(&src[3]);
						obj = DEC_255(&src[5]);
						dest_page = DEC_255(&src[7]);
						if (hypnode_valid(hyp, dest_page))
						{
							str = stg_quote_nodename(hyp, dest_page);
						} else
						{
							str = invalid_page(dest_page);
						}
						FLUSHLINE();
						if (tree != in_tree)
						{
							FLUSHTREE();
							hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@tree %d%s", tree, stg_nl);
							in_tree = tree;
						}
						hyp_utf8_fprintf_charset(opts->outfile, output_charset, "   %d \"%s\" %u%s", obj, str, line, stg_nl);
						g_free(str);
						src += 9;
					}
					break;
					
				case HYP_ESC_PIC:
					FLUSHTREE();
					FLUSHLINE();
					src += 8;
					break;
					
				case HYP_ESC_LINE:
					FLUSHTREE();
					FLUSHLINE();
					src += 7;
					break;
					
				case HYP_ESC_BOX:
				case HYP_ESC_RBOX:
					FLUSHTREE();
					FLUSHLINE();
					src += 7;
					break;
					
				case HYP_ESC_CASE_TEXTATTR:
					if (stg_out_attr(opts->outfile, textattr, *src - HYP_ESC_TEXTATTR_FIRST))
						at_bol = FALSE;
					textattr = *src - HYP_ESC_TEXTATTR_FIRST;
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
				FLUSHTREE();
				DUMPTEXT();
				fputs(stg_nl, opts->outfile);
				at_bol = TRUE;
				++lineno;
				stg_out_graphics(opts, hyp, hyp_gfx, lineno);
				src++;
				textstart = src;
			} else
			{
				FLUSHTREE();
				src++;
			}
		}
		DUMPTEXT();
		if (stg_out_attr(opts->outfile, textattr, 0))
			at_bol = FALSE;
		FLUSHLINE();
		FLUSHTREE();
		++lineno;
		stg_out_graphics(opts, hyp, hyp_gfx, lineno);
		
		if (hyp_gfx != NULL)
		{
			struct hyp_gfx *gfx, *next;
			
			for (gfx = hyp_gfx; gfx != NULL; gfx = next)
			{
				if (!gfx->used)
				{
					hyp_utf8_fprintf_charset(opts->outfile, output_charset, "##gfx unused: ");
					stg_out_gfx(opts, hyp, gfx);
				}
				next = gfx->next;
				g_free(gfx);
			}
		}
		
		hyp_utf8_fprintf_charset(opts->outfile, output_charset, "@endnode%s%s%s", stg_nl, stg_nl, stg_nl);
		
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(opts->errorfile, _("%s: Node %u: failed to decode\n"), hyp->file, node);
	}

#undef DUMPTEXT
#undef FLUSHLINE
#undef FLUSHTREE
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean write_image(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node)
{
	unsigned char *data;
	long data_size;
	unsigned char *buf = NULL;
	unsigned char *conv = NULL;
	FILE *fp = NULL;
	gboolean retval = TRUE;
	HYP_PICTURE hyp_pic;
	hyp_pic_format format;
	PICTURE pic;
	long header_size;
	HYP_IMAGE *image;
	
	pic_init(&pic);
	
	image = (HYP_IMAGE *)AskCache(hyp, node);
	if (image == NULL)
	{
		data = hyp_loaddata(hyp, node);
		if (data == NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, _("failed to load image node %u"), node);
			goto error;
		}
		image = g_new0(HYP_IMAGE, 1);
		if (image == NULL)
		{
			g_free(data);
			goto error;
		}
		image->number = node;
		image->pic.fd_addr = data;
		if (!TellCache(hyp, node, (HYP_NODE *) image))
		{
			g_free(image);
			g_free(data);
			hyp_utf8_fprintf(opts->errorfile, _("failed to cache compressed image data for %u"), node);
			goto error;
		}
	} else
	{
		ASSERT(!image->decompressed);
		data = image->pic.fd_addr;
	}
	data_size = GetDataSize(hyp, node);
	buf = g_new(unsigned char, data_size);
	if (buf == NULL)
		goto error;
	if (data_size < SIZEOF_HYP_PICTURE ||
		!GetEntryBytes(hyp, node, data, buf, data_size))
	{
		hyp_utf8_fprintf(opts->errorfile, _("failed to decode image header for %u"), node);
		goto error;
	}
	
	hyp_pic_get_header(&hyp_pic, buf);
	
	format = format_from_pic(opts, hyp->indextable[node]);
	data_size -= SIZEOF_HYP_PICTURE;
	
	pic.pi_width = hyp_pic.width;
	pic.pi_height = hyp_pic.height;
	pic.pi_planes = hyp_pic.planes;
	pic.pi_compressed = 1;
	pic_stdpalette(pic.pi_palette, pic.pi_planes);
	pic_calcsize(&pic);
	if (data_size != pic.pi_picsize)
	{
		hyp_utf8_fprintf(opts->errorfile, _("format error in image of node %u: %dx%dx%d datasize=%ld picsize=%ld\n"), node, hyp_pic.width, hyp_pic.height, hyp_pic.planes, data_size, pic.pi_picsize);
		goto error;
	}
	pic.pi_name = image_name(format, hyp, node, opts->image_name_prefix);
	if (empty(pic.pi_name))
		goto error;
	
	conv = g_new(_UBYTE, data_size);
	if (conv == NULL)
		goto error;
	
	fp = hyp_utf8_fopen(pic.pi_name, "wb");
	if (fp == NULL)
	{
		FileErrorErrno(pic.pi_name);
		goto error;
	}
	
	switch (format)
	{
	case HYP_PIC_IMG:
		pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
		
		g_free(buf);
		buf = NULL;
		header_size = img_header(&buf, &pic);
		if (buf == NULL)
		{
			oom();
			goto error;
		}
		data_size = img_pack(buf, conv, &pic);
		if ((long) fwrite(buf, 1, header_size + data_size, fp) != header_size + data_size)
		{
			FileErrorErrno(pic.pi_name);
			goto error;
		}
		break;

	case HYP_PIC_IFF:
		{
			_UBYTE headerbuf[IFF_HEADER_BUFSIZE];
			
			pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
			header_size = iff_header(headerbuf, &pic);
			if ((long) fwrite(headerbuf, 1, header_size, fp) != header_size)
			{
				FileErrorErrno(pic.pi_name);
				goto error;
			}
			g_free(buf);
			buf = iff_pack(conv, &pic);
			if (buf == NULL)
			{
				goto error;
			}
			if ((long) fwrite(buf, 1, pic.pi_datasize, fp) != pic.pi_datasize)
			{
				FileErrorErrno(pic.pi_name);
				goto error;
			}
		}
		break;

	case HYP_PIC_BMP:
		pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
		g_free(buf);
		buf = NULL;
		header_size = bmp_header(&buf, &pic);
		if (buf == NULL)
		{
			oom();
			goto error;
		}
		data_size = bmp_pack(buf, conv, &pic, TRUE);
		if ((long) fwrite(buf, 1, header_size + data_size, fp) != header_size + data_size)
		{
			FileErrorErrno(pic.pi_name);
			goto error;
		}
		break;

	case HYP_PIC_ICN:
		if (icn_fwrite(fp, buf + SIZEOF_HYP_PICTURE, &pic) == FALSE)
		{
			FileErrorErrno(pic.pi_name);
			goto error;
		}
		break;
	
	case HYP_PIC_UNKNOWN:
		unreachable();
		break;
	}
	
	fclose(fp);
	fp = NULL;
	if (opts->verbose >= 2 && opts->outfile != stdout)
		hyp_utf8_fprintf(stdout, _("wrote image %s (%dx%dx%d)\n"), pic.pi_name, pic.pi_width, pic.pi_height, 1 << pic.pi_planes);
	goto done;
error:
	retval = FALSE;
done:
	if (fp != NULL)
		hyp_utf8_fclose(fp);
	g_free(pic.pi_name);
	g_free(conv);
	g_free(buf);
	
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean recompile_stg(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	hcp_opts hyp_opts;
	
	UNUSED(argc);
	UNUSED(argv);
	
	/* output_charset = HYP_CHARSET_ATARI; */
	stg_nl = (opts->outfile == stdout || output_charset != HYP_CHARSET_ATARI) ? "\n" : "\015\012";
	
	ret = TRUE;
	hcp_opts_copy(&hyp_opts, opts);
	if (hyp->hcp_options != NULL)
	{
		hcp_opts_parse_string(&hyp_opts, hyp->hcp_options, OPTS_FROM_SOURCE);
	}
	
	stg_out_globals(hyp, opts->outfile);
	if (opts->read_images)
		InitCache(hyp);
	
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		if (node == hyp->index_page)
		{
			/*
			 * skip recompiling index page, assuming
			 * that it will be regenerated when
			 * compiling the source file.
			 * Should probably check wether the existing page
			 * is an automatically generated one,
			 * but no idea how to achieve that.
			 * Looking at the option string does not seem
			 * to work, all existing hypertexts i have seen
			 * have "-i" set wether they contain an index or not.
			 */
			if (!opts->gen_index)
				continue;
		}
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
			ret &= stg_out_node(hyp, &hyp_opts, node);
			break;
		case HYP_NODE_POPUP:
			ret &= stg_out_node(hyp, &hyp_opts, node);
			break;
		case HYP_NODE_IMAGE:
			if (opts->read_images)
				ret &= write_image(hyp, &hyp_opts, node);
			break;
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
	
	ClearCache(hyp);
	
	hyp_opts.outfile = NULL;
	hyp_opts.errorfile = NULL;
	hcp_opts_free(&hyp_opts);
	return ret;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean dump_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node)
{
	char *str;
	HYP_NODE *nodeptr;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		str = hyp_conv_to_utf8(hyp->comp_charset, textstart, src - textstart); \
		if (str != NULL) \
			hyp_utf8_fprintf(opts->outfile, _("Text: <%s>\n"), str); \
		g_free(str); \
	}

	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		
		src = nodeptr->start;
		end = nodeptr->end;
		textstart = src;
		
		while (src < end)
		{
			if (*src == HYP_ESC)
			{
				DUMPTEXT();
				src++;
				switch (*src)
				{
				case HYP_ESC_ESC:
					hyp_utf8_fprintf(opts->outfile, _("<ESC>\n"));
					src++;
					break;
				
				case HYP_ESC_WINDOWTITLE:
					src++;
					str = hyp_conv_to_utf8(hyp->comp_charset, src, STR0TERM);
					hyp_utf8_fprintf(opts->outfile, _("Title: %s\n"), str);
					g_free(str);
					src += ustrlen(src) + 1;
					break;

				case HYP_ESC_CASE_DATA:
					hyp_utf8_fprintf(opts->outfile, _("Data: type %u, len %u\n"), src[0], src[1]);
					src += src[1] - 1;
					break;
				
				case HYP_ESC_LINK:
				case HYP_ESC_LINK_LINE:
				case HYP_ESC_ALINK:
				case HYP_ESC_ALINK_LINE:
					{
						hyp_nodenr dest_page;
						unsigned char type;
						hyp_lineno line = 0;
						char *dest;
						
						type = *src;
						if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
						{
							line = DEC_255(&src[1]);
							src += 2;
						}
						dest_page = DEC_255(&src[1]);
						src += 3;
						if (hypnode_valid(hyp, dest_page))
						{
							dest = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dest_page]->name, STR0TERM);
						} else
						{
							dest = invalid_page(dest_page);
						}
						if (*src <= HYP_STRLEN_OFFSET)
						{
							str = g_strdup(dest);
						} else
						{
							size_t len = *src - HYP_STRLEN_OFFSET;
							str = hyp_conv_to_utf8(hyp->comp_charset, src + 1, len);
							src += len;
						}
						switch (type)
						{
						case HYP_ESC_LINK:
							hyp_utf8_fprintf(opts->outfile, _("Link: \"%s\" %u \"%s\"\n"), str, dest_page, dest);
							break;
						case HYP_ESC_LINK_LINE:
							hyp_utf8_fprintf(opts->outfile, _("Link: \"%s\" %u \"%s\" %u\n"), str, dest_page, dest, line);
							break;
						case HYP_ESC_ALINK:
							hyp_utf8_fprintf(opts->outfile, _("ALink: \"%s\" %u \"%s\"\n"), str, dest_page, dest);
							break;
						case HYP_ESC_ALINK_LINE:
							hyp_utf8_fprintf(opts->outfile, _("ALink: \"%s\" %u \"%s\" %u\n"), str, dest_page, dest, line);
							break;
						}
						g_free(dest);
						g_free(str);
						src++;
					}
					break;
					
				case HYP_ESC_EXTERNAL_REFS:
					{
						hyp_nodenr dest_page;
						char *text;
						
						dest_page = DEC_255(&src[2]);
						text = hyp_conv_to_utf8(hyp->comp_charset, src + 4, max(src[1], 5u) - 5u);
						if (hypnode_valid(hyp, dest_page))
						{
							str = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dest_page]->name, STR0TERM);
						} else
						{
							str = invalid_page(dest_page);
						}
						hyp_utf8_fprintf(opts->outfile, _("XRef \"%s\" \"%s\"\n"), text, str);
						g_free(str);
						g_free(text);
						src += src[1] - 1;
					}
					break;
					
				case HYP_ESC_OBJTABLE:
					{
						hyp_nodenr dest_page;
						_WORD tree, obj;
						hyp_lineno line;
						
						line = DEC_255(&src[1]);
						tree = DEC_255(&src[3]);
						obj = DEC_255(&src[5]);
						dest_page = DEC_255(&src[7]);
						if (hypnode_valid(hyp, dest_page))
						{
							str = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dest_page]->name, STR0TERM);
						} else
						{
							str = invalid_page(dest_page);
						}
						hyp_utf8_fprintf(opts->outfile, _("Tree: tree=%d, obj=%d, line=%u: \"%s\"\n"), tree, obj, line, str);
						g_free(str);
						src += 9;
					}
					break;
					
				case HYP_ESC_PIC:
					{
						hyp_nodenr num;
						_WORD x_offset;
						_WORD y_offset;
						_WORD width;
						_WORD height;
						gboolean islimage;
						
						num = DEC_255(&src[1]);
						x_offset = src[3];
						y_offset = DEC_255(&src[4]);
						width = src[6];
						height = src[7];
						islimage = hyp->comp_vers >= 3 && src[6] == 1;
						hyp_utf8_fprintf(opts->outfile, _("%s: x=%d, y=%d, w=%d, h=%d, num=%u\n"),
							islimage ? _("limage") : _("image"),
							x_offset, y_offset, width, height, num);
						src += 8;
					}
					break;
					
				case HYP_ESC_LINE:
					{
						_WORD x_offset;
						_WORD y_offset;
						_WORD width;
						_WORD height;
						unsigned char attr;
						
						x_offset = src[1];
						y_offset = DEC_255(&src[2]);
						width = src[4];
						height = src[5];
						attr = src[6];
						hyp_utf8_fprintf(opts->outfile, _("Line: x=%d, y=%d, w=%d, h=%d, attr=0x%02x, begend=%x, style=%d\n"),
							x_offset, y_offset, width - 128, height - 1, attr, (attr - 1) & 7, min(max(((attr - 1) >> 3), 0), 6) + 1);
						src += 7;
					}
					break;
					
				case HYP_ESC_BOX:
				case HYP_ESC_RBOX:
					{
						_WORD x_offset;
						_WORD y_offset;
						_WORD width;
						_WORD height;
						unsigned char attr;
						
						x_offset = src[1];
						y_offset = DEC_255(&src[2]);
						width = src[4];
						height = src[5];
						attr = src[6];
						hyp_utf8_fprintf(opts->outfile, _("%s: x=%d, y=%d, w=%d, h=%d, attr=%u\n"),
							*src == HYP_ESC_BOX ? _("Box") : _("RBox"), x_offset, y_offset, width, height, attr);
						src += 7;
					}
					break;
					
				case HYP_ESC_CASE_TEXTATTR:
					hyp_utf8_fprintf(opts->outfile, _("Textattr: $%x\n"), *src - HYP_ESC_TEXTATTR_FIRST);
					src++;
					break;
				
				default:
					hyp_utf8_fprintf(opts->outfile, _("<unknown hex esc $%02x>\n"), *src);
					break;
				}
				textstart = src;
			} else if (*src == HYP_EOL)
			{
				DUMPTEXT();
				hyp_utf8_fprintf(opts->outfile, _("<EOL>\n"));
				src++;
				textstart = src;
			} else
			{
				src++;
			}
		}
		DUMPTEXT();
		
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(opts->outfile, _("%s: Node %u: failed to decode\n"), hyp->file, node);
	}

#undef DUMPTEXT
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean dump_image(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node)
{
	unsigned char *data;
	unsigned char hyp_pic_raw[SIZEOF_HYP_PICTURE];
	HYP_PICTURE hyp_pic;
	gboolean retval = TRUE;
	
	data = hyp_loaddata(hyp, node);

	if (data == NULL)
		goto error;

	if (!GetEntryBytes(hyp, node, data, hyp_pic_raw, SIZEOF_HYP_PICTURE))
		goto error;
	
	hyp_pic_get_header(&hyp_pic, hyp_pic_raw);

	hyp_utf8_fprintf(opts->outfile, _("  Width: %d\n"), hyp_pic.width);
	hyp_utf8_fprintf(opts->outfile, _("  Height: %d\n"), hyp_pic.height);
	hyp_utf8_fprintf(opts->outfile, _("  Planes: %d\n"), hyp_pic.planes);
	hyp_utf8_fprintf(opts->outfile, _("  Plane Mask: $%02x\n"), hyp_pic.plane_pic);
	hyp_utf8_fprintf(opts->outfile, _("  Plane On-Off: $%02x\n"), hyp_pic.plane_on_off);
	hyp_utf8_fprintf(opts->outfile, _("  Filler: $%02x\n"), hyp_pic.filler);
	
	goto done;
error:
	retval = FALSE;
done:
	g_free(data);
	
	return retval;
}

/* ------------------------------------------------------------------------- */

static void dump_globals(HYP_DOCUMENT *hyp, FILE *outfile)
{
	hyp_utf8_fprintf(outfile, _("OS: %s\n"), hyp_osname(hyp->comp_os));
	hyp_utf8_fprintf(outfile, _("Charset: %s\n"), hyp_charset_name(hyp->comp_charset));
	if (hyp->database != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Database: %s\n"), hyp->database);
	}
	if (hyp->author != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Author: %s\n"), hyp->author);
	}
	if (hyp->version != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Version: %s\n"), hyp->version);
	}
	if (hyp->help_name != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Help Node: %s\n"), hyp->help_name);
	}
	if (hyp->default_name != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Default Node: %s\n"), hyp->default_name);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			hyp_utf8_fprintf(outfile, _("Host-Application: %s\n"), h->name);
		}
	}
	if (hyp->hcp_options != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Compiler-Options: %s\n"), hyp->hcp_options);
	}
	if (hyp->subject != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Subject: %s\n"), hyp->subject);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_fprintf(outfile, _("Line width: %d\n"), hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_fprintf(outfile, _("ST-Guide flags: $%04x\n"), hyp->st_guide_flags);
	}
	hyp_utf8_fprintf(outfile, _("Compiler Version: %u\n"), hyp->comp_vers);
}

/* ------------------------------------------------------------------------- */

static gboolean recompile_dump(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	char *str;
	unsigned long compressed_size, size;
	size_t namelen;
	size_t slen;
	INDEX_ENTRY *entry;
	gboolean ret;
	gboolean found;
	int i;
	
	dump_globals(hyp, opts->outfile);
	hyp_utf8_fprintf(opts->outfile, _("Index nodes: %u\n"), hyp->num_index);
	hyp_utf8_fprintf(opts->outfile, _("Index table size: %ld\n"), hyp->itable_size);
	hyp_utf8_fprintf(opts->outfile, "\n\n");
	
	ret = TRUE;
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		if (argc == 0)
		{
			found = TRUE;
		} else
		{
			found = FALSE;
			for (i = 0; i < argc; i++)
			{
				if (argv[i])
				{
					char *s1, *s2;
					
					s1 = hyp_conv_to_utf8(hyp->comp_charset, entry->name, STR0TERM);
					s2 = hyp_conv_to_utf8(hyp_get_current_charset(), argv[i], STR0TERM);
					if (namecmp(s1, s2) == 0)
					{
						argv[i] = NULL;
						found = TRUE;
					}
					g_free(s2);
					g_free(s1);
				}
			}
		}
		if (!found)
			continue;
		compressed_size = GetCompressedSize(hyp, node);
		size = GetDataSize(hyp, node);
		namelen = entry->length - SIZEOF_INDEX_ENTRY;
		hyp_utf8_fprintf(opts->outfile, _("Index entry %u:\n"), node);
		str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
		hyp_utf8_fprintf(opts->outfile, _("  Name: %s"), str);
		slen = hyp->comp_charset == HYP_CHARSET_UTF8 ? strlen(str) : g_utf8_str_len(str, STR0TERM);
		g_free(str);
		if (namelen > 0)
		{
			/* dump maybe garbage after name */
			slen += 1;
			if ((slen & 1) /* && entry->name[slen] == 0 */)
				slen++;
			if (slen < namelen)
			{
				size_t i;
				
				hyp_utf8_fprintf(opts->outfile, " (");
				for (i = slen; i < namelen; i++)
				{
					if (i != slen)
						hyp_utf8_fprintf(opts->outfile, " ");
					hyp_utf8_fprintf(opts->outfile, "$%02x", entry->name[i]);
				}
				hyp_utf8_fprintf(opts->outfile, ")");
			}
		}
		hyp_utf8_fprintf(opts->outfile, "\n");
		hyp_utf8_fprintf(opts->outfile, _("  Entry length: %u\n"), entry->length);
		hyp_utf8_fprintf(opts->outfile, _("  Offset: %ld $%lx\n"), entry->seek_offset, entry->seek_offset);
		hyp_utf8_fprintf(opts->outfile, _("  Compressed: %ld $%lx\n"), compressed_size, compressed_size);
		hyp_utf8_fprintf(opts->outfile, _("  Uncompressed: %ld $%lx\n"), size, size);
		hyp_utf8_fprintf(opts->outfile, _("  Next: %u $%04x\n"), entry->next, entry->next);
		hyp_utf8_fprintf(opts->outfile, _("  Previous: %u $%04x\n"), entry->previous, entry->previous);
		hyp_utf8_fprintf(opts->outfile, _("  Up: %u $%04x\n"), entry->toc_index, entry->toc_index);
		
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
			hyp_utf8_fprintf(opts->outfile, _("  Type: internal node\n"));
			ret &= dump_node(hyp, opts, node);
			break;
		case HYP_NODE_POPUP:
			hyp_utf8_fprintf(opts->outfile, _("  Type: pop-up node\n"));
			ret &= dump_node(hyp, opts, node);
			break;
		case HYP_NODE_EXTERNAL_REF:
			hyp_utf8_fprintf(opts->outfile, _("  Type: external node\n"));
			break;
		case HYP_NODE_IMAGE:
			hyp_utf8_fprintf(opts->outfile, _("  Type: image\n"));
			ret &= dump_image(hyp, opts, node);
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
			hyp_utf8_fprintf(opts->outfile, _("  Type: system node\n"));
			break;
		case HYP_NODE_REXX_SCRIPT:
			hyp_utf8_fprintf(opts->outfile, _("  Type: REXX script\n"));
			break;
		case HYP_NODE_REXX_COMMAND:
			hyp_utf8_fprintf(opts->outfile, _("  Type: REXX command\n"));
			break;
		case HYP_NODE_QUIT:
			hyp_utf8_fprintf(opts->outfile, _("  Type: quit\n"));
			break;
		case HYP_NODE_CLOSE:
			hyp_utf8_fprintf(opts->outfile, _("  Type: close\n"));
			break;
		case HYP_NODE_EOF:
			hyp_utf8_fprintf(opts->outfile, _("  Type: EOF\n"));
			break;
		default:
			hyp_utf8_fprintf(opts->outfile, _("  Type: unknown type %u\n"), entry->type);
			break;
		}
		hyp_utf8_fprintf(opts->outfile, "\n\n");
	}
	
	for (i = 0; i < argc; i++)
	{
		if (argv[i] != NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, _("'%s' not found.\n"), argv[i]);
			ret = FALSE;
		}
	}
	
	return ret;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean check_long_filenames(const char *dir)
{
	long test1, test2;
	char *s, *s1, *s2;
	struct stat st;
	int ret;
	int fd;
	
	s1 = NULL;
	test1 = 11111110L;
	for (;;)
	{
		g_free(s1);
		test1++;
		s = g_strdup_printf("%ld.tmp", test1);
		s1 = g_build_filename(dir, s, NULL);
		g_free(s);
		test2 = test1 * 10 + 1;
		s = g_strdup_printf("%ld.tmp", test2);
		s2 = g_build_filename(dir, s, NULL);
		g_free(s);
		ret = rpl_stat(s2, &st);
		g_free(s2);
		if (ret == 0)
		{
			continue;
		}
		if (errno == ENAMETOOLONG)
		{
			g_free(s1);
			return FALSE;
		}
		if (errno != ENOENT)
		{
			continue;
		}
		/*
		 * try to create tmp file. if it exists, we should get EEXIST or some access error
		 */
		fd = open(s1, O_CREAT, 0644);
		if (fd >= 0)
			break;
	}
	
	close(fd);
	s = g_strdup_printf("%ld.tmp", test2);
	s2 = g_build_filename(dir, s, NULL);
	g_free(s);
	ret = rpl_stat(s2, &st);
	fd = errno;
	g_free(s2);
	
	unlink(s1);
	g_free(s1);
	if (ret != 0 && fd == ENOENT)
	{
		return TRUE;
	}
	
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static gboolean recompile(const char *filename, hcp_opts *opts, recompile_func func, int argc, const char **argv, const char *defext)
{
	gboolean retval;
	HYP_DOCUMENT *hyp;
	char *dir;
	char *output_filename = NULL;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	
	if ((opts->errorfile == NULL || opts->errorfile == stderr) && opts->error_filename != NULL)
	{
		opts->errorfile = hyp_utf8_fopen(opts->error_filename, "w");
		if (opts->errorfile == NULL)
		{
			hyp_utf8_fprintf(stderr, "%s: %s\n", opts->error_filename, strerror(errno));
			return FALSE;
		}
	}

	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}

	hyp = hyp_load(handle, &type);
	if (hyp == NULL)
	{
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	hyp->file = filename;
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, filename);
		return FALSE;
	}
	if (hyp->comp_vers > HCP_COMPILER_VERSION)
		hyp_utf8_fprintf(opts->errorfile, _("%s: warning: %s created by compiler version %u\n"), gl_program_name, hyp->file, hyp->comp_vers);
	if ((opts->outfile == NULL || opts->outfile == stdout) && opts->output_filename != NULL)
	{
		if (strcmp(opts->output_filename, HCP_OUTPUT_WILDCARD) == 0)
		{
			output_filename = replace_ext(filename, NULL, defext);
		} else
		{
			output_filename = g_strdup(opts->output_filename);
		}
		opts->outfile = hyp_utf8_fopen(output_filename, "wb");
		if (opts->outfile == NULL)
		{
			hyp_unref(hyp);
			hyp_utf8_close(handle);
			hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, output_filename, strerror(errno));
			g_free(output_filename);
			return FALSE;
		}
		dir = g_path_get_dirname(output_filename);
	} else
	{
		dir = NULL;
	}
	if (empty(dir))
	{
		g_free(dir);
		dir = g_strdup(".");
	}
	if (opts->long_filenames < 0)
		opts->long_filenames = check_long_filenames(dir);
	g_free(dir);
	if (opts->long_filenames)
	{
		g_free(opts->image_name_prefix);
		opts->image_name_prefix = replace_ext(hyp_basename(filename), NULL, "_img_");
	}
	
	if (opts->verbose >= 0 && opts->outfile != stdout)
	{
		hyp_utf8_fprintf(stdout, _("recompiling %s to %s\n"), filename, output_filename);
	}
	
	retval = func(hyp, opts, argc, argv);
	hyp_unref(hyp);
	hyp_utf8_close(handle);
	if (output_filename)
	{
		hyp_utf8_fclose(opts->outfile);
		opts->outfile = NULL;
		g_free(output_filename);
	}
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean list_entries(const char *filename, hcp_opts *opts)
{
	HYP_DOCUMENT *hyp;
	char *str;
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	unsigned long compressed_size, size;
	size_t namelen;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	
	static char const list_format[] = "%-20s %9lu  %6lu %6lu   %s";
	static char const long_list_format[] = "%5u %5u %5u %5u %-20s %9lu  %6lu %6lu   %3u %s";
	
	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}

	hyp = hyp_load(handle, &type);

	if (hyp == NULL)
	{
		REF_FILE *ref = ref_load(filename, handle, FALSE);
		gboolean ret;
		
		if (ref != NULL)
		{
			hyp_utf8_close(handle);
			ret = ref_list(ref, opts->outfile, opts->list_flags != 0);
			ref_close(ref);
			return ret;
		}
	}
	
	if (hyp == NULL)
	{
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	hyp->file = filename;
	
	if (hyp->comp_vers > HCP_COMPILER_VERSION)
		hyp_utf8_fprintf(opts->errorfile, _("%s: warning: %s created by compiler version %u\n"), gl_program_name, hyp->file, hyp->comp_vers);
	
	dump_globals(hyp, opts->outfile);
	
	if (opts->list_flags != 0)
	{
		hyp_utf8_fprintf(opts->outfile, "\n");
		if (opts->do_list > 1)
		{
			hyp_utf8_fprintf(opts->outfile, _("    #   toc  next  prev type                    offset  packed   size   len name\n"));
			hyp_utf8_fprintf(opts->outfile, _("--------------------------------------------------------------------------------\n"));
			for (node = 0; node < hyp->num_index; node++)
			{
				entry = hyp->indextable[node];
				compressed_size = GetCompressedSize(hyp, node);
				size = GetDataSize(hyp, node);
				namelen = entry->length - SIZEOF_INDEX_ENTRY;
				switch ((hyp_indextype) entry->type)
				{
				case HYP_NODE_INTERNAL:
					if (opts->list_flags & LIST_NODES)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("internal node"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_POPUP:
					if (opts->list_flags & LIST_PNODES)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("pop-up node"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_EXTERNAL_REF:
					if (opts->list_flags & LIST_XREFS)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("external node"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_IMAGE:
					if (opts->list_flags & LIST_PICS)
					{
						unsigned char *data;
						unsigned char hyp_pic_raw[SIZEOF_HYP_PICTURE];
						HYP_PICTURE hyp_pic;
						
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("image"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						data = hyp_loaddata(hyp, node);
						if (data != NULL)
						{
							if (GetEntryBytes(hyp, node, data, hyp_pic_raw, SIZEOF_HYP_PICTURE))
							{
								hyp_pic_get_header(&hyp_pic, hyp_pic_raw);
								hyp_utf8_fprintf(opts->outfile, " (%ux%u", hyp_pic.width, hyp_pic.height);
								if (hyp_pic.planes <= 8)
									hyp_utf8_fprintf(opts->outfile, "x%u", 1 << hyp_pic.planes);
								else if (hyp_pic.planes <= 16)
									hyp_utf8_fprintf(opts->outfile, " hicolor-%u", hyp_pic.planes);
								else
									hyp_utf8_fprintf(opts->outfile, " truecolor-%u", hyp_pic.planes);
								hyp_utf8_fprintf(opts->outfile, _(" mask=$%02x on-off=$%02x)"), hyp_pic.plane_pic, hyp_pic.plane_on_off);
							} else
							{
								hyp_utf8_fprintf(opts->outfile, _(" (decode error)"));
							}
							g_free(data);
						} else
						{
							hyp_utf8_fprintf(opts->outfile, _(" (no data)"));
						}
						g_free(str);
					}
					break;
				case HYP_NODE_SYSTEM_ARGUMENT:
					if (opts->list_flags & LIST_SYSTEM)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("system node"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_REXX_SCRIPT:
					if (opts->list_flags & LIST_RXS)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("REXX script"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_REXX_COMMAND:
					if (opts->list_flags & LIST_RX)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("REXX command"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_QUIT:
					if (opts->list_flags & LIST_QUIT)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("quit node"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_CLOSE:
					if (opts->list_flags & LIST_CLOSE)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("close node"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_EOF:
					if (opts->list_flags & LIST_EOF)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							_("EOF entry"),
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
					}
					break;
				default:
					if (opts->list_flags & LIST_UNKNOWN)
					{
						char *type = g_strdup_printf(_("unknown type %u"), entry->type);
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, long_list_format,
							node, entry->toc_index, entry->next, entry->previous,
							type,
							entry->seek_offset, compressed_size, size, entry->length, printnull(str));
						g_free(str);
						g_free(type);
					}
					break;
				}
				fputs("\n", opts->outfile);
			}
		} else
		{
			hyp_utf8_fprintf(opts->outfile, _("type                    offset  packed   size   name\n"));
			hyp_utf8_fprintf(opts->outfile, _("----------------------------------------------------\n"));
			for (node = 0; node < hyp->num_index; node++)
			{
				entry = hyp->indextable[node];
				compressed_size = GetCompressedSize(hyp, node);
				size = GetDataSize(hyp, node);
				namelen = entry->length - SIZEOF_INDEX_ENTRY;
				switch ((hyp_indextype) entry->type)
				{
				case HYP_NODE_INTERNAL:
					if (opts->list_flags & LIST_NODES)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("internal node"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_POPUP:
					if (opts->list_flags & LIST_PNODES)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("pop-up node"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_EXTERNAL_REF:
					if (opts->list_flags & LIST_XREFS)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("external node"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_IMAGE:
					if (opts->list_flags & LIST_PICS)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("image"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_SYSTEM_ARGUMENT:
					if (opts->list_flags & LIST_SYSTEM)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("system node"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_REXX_SCRIPT:
					if (opts->list_flags & LIST_RXS)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("REXX script"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_REXX_COMMAND:
					if (opts->list_flags & LIST_RX)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("REXX command"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_QUIT:
					if (opts->list_flags & LIST_QUIT)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("quit node"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_CLOSE:
					if (opts->list_flags & LIST_CLOSE)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("close node"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				case HYP_NODE_EOF:
					if (opts->list_flags & LIST_EOF)
					{
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							_("EOF entry"),
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
					}
					break;
				default:
					if (opts->list_flags & LIST_UNKNOWN)
					{
						char *type = g_strdup_printf(_("unknown type %u"), entry->type);
						str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
						hyp_utf8_fprintf(opts->outfile, list_format,
							type,
							entry->seek_offset, compressed_size, size, printnull(str));
						g_free(str);
						g_free(type);
					}
					break;
				}
				fputs("\n", opts->outfile);
			}
		}
	}
	
	hyp_utf8_close(handle);
	hyp_unref(hyp);
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#include "hypmain.h"

int main(int argc, const char **argv)
{
	int c;
	int retval = 0;
	hcp_opts _opts;
	hcp_opts *opts = &_opts;
	int wait_key;
	
	is_MASTER = getenv("TO_MASTER") != NULL;
	
	output_charset = hyp_get_current_charset();
	HypProfile_Load();
	
	hcp_opts_init(opts);
	opts->tabwidth = gl_profile.viewer.ascii_tab_size;
	if (!hcp_opts_parse_string(opts, gl_profile.hcp.options, OPTS_FROM_CONFIG))
		retval = 1;
	if (hcp_opts_parse(opts, argc, argv, OPTS_FROM_COMMANDLINE) == FALSE)
		retval = 1;
	
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
		int num_args;
		int num_opts = (opts->do_list > 0) + (opts->do_ascii_recomp > 0) + (opts->do_recompile > 0) + (opts->do_compile > 0);
		
		c = opts->optind;
		num_args = argc - c;

		if (retval == 0)
		{
			if (num_opts == 0 && num_args > 0)
			{
				opts->do_compile = TRUE;
			} else if (num_opts == 0 && num_args <= 0)
			{
				/* maybe empty commandline from old Desktop */
				retval = 1;
				print_usage(stderr);
			} else if (num_opts > 1)
			{
				hcp_usage_error(_("only one of -lrv may be specified"));
				retval = 1;
			}
		}
		
		if (retval == 0 && num_args <= 0)
		{
			hcp_usage_error(_("no files specified"));
			retval = 1;
		}
		
		if (retval == 0)
		{
			if (opts->do_list)
			{
				/*
				 * maybe TODO: handle -o~ here
				 */
				while (c < argc)
				{
					const char *filename = argv[c++];
					if (num_args > 1)
						hyp_utf8_fprintf(opts->outfile, _("File: %s\n"), filename);
					if (!list_entries(filename, opts))
						retval = 1;
					else if (c < argc)
						hyp_utf8_fprintf(opts->outfile, "\n\n");
				}
			} else if (opts->do_ascii_recomp)
			{
				const char *filename = argv[c++];

				if (opts->output_charset != HYP_CHARSET_NONE)
					output_charset = opts->output_charset;
				/*
				 * args beyond filename are node names to display
				 */
				if (recompile(filename, opts, recompile_ascii, argc - c, &argv[c], ".txt") == FALSE)
				{
					retval = 1;
				}
			} else if (opts->do_recompile)
			{
				if (opts->output_charset != HYP_CHARSET_NONE)
					output_charset = opts->output_charset;
				while (c < argc)
				{
					const char *filename = argv[c++];
					stg_nl = (opts->output_filename == NULL || output_charset != HYP_CHARSET_ATARI) ? "\n" : "\015\012";
					if (num_args > 1)
						hyp_utf8_fprintf_charset(opts->outfile, output_charset, _("@remark File: %s%s"), filename, stg_nl);
					if (recompile(filename, opts, recompile_stg, 0, NULL, HYP_EXT_STG) == FALSE)
					{
						retval = 1;
						break;
					}
					else if (c < argc)
						hyp_utf8_fprintf_charset(opts->outfile, output_charset, "%s%s", stg_nl, stg_nl);
				}
			} else if (opts->do_dump)
			{
				const char *filename = argv[c++];
				
				/*
				 * args beyond filename are node names to display
				 */
				if (recompile(filename, opts, recompile_dump, argc - c, &argv[c], ".txt") == FALSE)
				{
					retval = 1;
				}
			} else if (opts->do_compile)
			{
				if (opts->output_filename && num_args > 1)
				{
					hcp_usage_error(_("cannot compile multiple input files to single output"));
				} else
				{
					while (c < argc)
					{
						const char *filename = argv[c++];
						if (!hcp_compile(filename, opts))
						{
							retval = 1;
							break;
						}
					}
				}
			} else
			{
				retval = 1;
				unreachable();
			}
		}
	}
	
	wait_key = opts->wait_key;
	hcp_opts_free(opts);
	
	if (wait_key == 2 ||
		(wait_key == 1 && retval != 0))
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

	return retval;
}
