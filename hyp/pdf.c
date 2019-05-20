#include "hypdefs.h"
#include "hypdebug.h"
#include "hcp_opts.h"
#include "pdf.h"

#ifdef WITH_PDF /* whole file */

/*
 * gzip header:
 *    magic 0x1f 0x8b
 *    cm    0x08
 *    flg   0x00
 *    mtime 0x00 0x00 0x00 0x00
 *    xfl   0x02
 *    os    0x03
 * 
 *
 * deflate stream header:
 *   CMF FLG
 *
 *   CMF: compression method and flags
 *        0..3: CM compression method (8)
 *        4..7: CINFO compression info (ln2 window-size, minus 8)
 *              eg. value of 7 = 1 << 15 = 32k windows size
 *
 *   FLG: Flags
 *        0..4 FCHECK check bits for CMF and FLG
 *             (CMF*256 + FLG) must be multiple of 31
 *        5    FDICT preset dictionary
 *             if set, a DICTID identifier follows
 *        6..7 Compression level (informative only)
 */

/*
 * node/label names are case sensitiv
 */
#define namecmp strcmp

static struct {
	uint32_t background;           /* window background color */
	uint32_t text;                 /* Displays text in the specified color */
	int link_effect;               /* Text Effect for references */
	uint32_t link;                 /* Displays references in the specified color */
	uint32_t popup;                /* Displays references to popups in the specified color */
	uint32_t xref;                 /* Displays external references in the specified color */
	uint32_t system;               /* Displays references to {@ system } in the specified color */
	uint32_t rx;                   /* Displays references to {@ rx } in the specified color */
	uint32_t rxs;                  /* Displays references to {@ rxs } in the specified color */
	uint32_t quit;                 /* Displays references to {@ quit } in the specified color */
	uint32_t close;                /* Displays references to {@ close } in the specified color */
	uint32_t ghosted;			   /* for "ghosted" effect (attribute @{G}) */
	uint32_t error;
} viewer_colors;

static uint32_t user_colors[16] = {
	0xffffff,
	0x000000,
	0xff0000,
	0x00ff00,
	0x0000ff,
	0x00ffff,
	0xffff00,
	0xff00ff,
	0xcccccc,
	0x888888,
	0x880000,
	0x008800,
	0x000088,
	0x008888,
	0x888800,
	0x880088
};

typedef struct _symtab_entry symtab_entry;
struct _symtab_entry {
	char *nodename;
	hyp_lineno lineno;
	hyp_reftype type;
	char *name;
	gboolean freeme;
	gboolean from_ref;
	gboolean from_idx;
	gboolean referenced;
	symtab_entry *next;
};

struct textattr {
	unsigned char curattr;
	unsigned char curfg;
	unsigned char curbg;
	unsigned char newattr;
	unsigned char newfg;
	unsigned char newbg;
};

struct pdf_xref {
	char *destname;
	char *text;	/* text to display */
	hyp_nodenr dest_page;
	hyp_indextype desttype;
	hyp_lineno line;
	struct pdf_xref *next;
};

#define PDF_DEFAULT_PIC_TYPE HYP_PIC_PNG

/* ------------------------------------------------------------------------- */

static symtab_entry *sym_find(symtab_entry *sym, const char *search, hyp_reftype type)
{
	while (sym != NULL)
	{
		if (type == sym->type && strcmp(sym->nodename, search) == 0)
			return sym;
		sym = sym->next;
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */

static gboolean sym_check_links(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, symtab_entry **syms)
{
	char *str;
	long lineno;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		
		end = nodeptr->end;
		src = nodeptr->start;
		lineno = 0;
		
		while (retval && src < end)
		{
			if (*src == HYP_ESC)
			{
				src++;
				switch (*src)
				{
				case HYP_ESC_ESC:
				case HYP_ESC_WINDOWTITLE:
				case HYP_ESC_CASE_DATA:
				case HYP_ESC_EXTERNAL_REFS:
				case HYP_ESC_OBJTABLE:
				case HYP_ESC_PIC:
				case HYP_ESC_LINE:
				case HYP_ESC_BOX:
				case HYP_ESC_RBOX:
				case HYP_ESC_CASE_TEXTATTR:
				case HYP_ESC_LINK:
				case HYP_ESC_ALINK:
				case HYP_ESC_UNKNOWN_A4:
				case HYP_ESC_FG_COLOR:
				case HYP_ESC_BG_COLOR:
					src = hyp_skip_esc(src - 1);
					break;
				
				case HYP_ESC_LINK_LINE:
				case HYP_ESC_ALINK_LINE:
					{
						hyp_nodenr dest_page;
						hyp_lineno line;
						char *dest;
						
						line = DEC_255(&src[1]);
						src += 2;
						dest_page = DEC_255(&src[1]);
						src += 3;
						dest = NULL;
						str = NULL;
						if (hypnode_valid(hyp, dest_page))
						{
							INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
							
							switch ((hyp_indextype) dest_entry->type)
							{
							case HYP_NODE_INTERNAL:
							case HYP_NODE_POPUP:
								dest = hyp_conv_to_utf8(hyp->comp_charset, dest_entry->name, dest_entry->length - SIZEOF_INDEX_ENTRY);
								break;
							case HYP_NODE_EXTERNAL_REF:
							case HYP_NODE_REXX_COMMAND:
							case HYP_NODE_REXX_SCRIPT:
							case HYP_NODE_SYSTEM_ARGUMENT:
							case HYP_NODE_IMAGE:
							case HYP_NODE_QUIT:
							case HYP_NODE_CLOSE:
							default:
							case HYP_NODE_EOF:
								break;
							}
						}
						if (dest)
						{
							if (*src <= HYP_STRLEN_OFFSET)
							{
								src++;
								str = g_strdup(dest);
							} else
							{
								size_t len;
	
								len = *src - HYP_STRLEN_OFFSET;
								src++;
								str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
								src += len;
							}
							{
								gboolean is_xref = FALSE;
								
								char *p = (hyp->st_guide_flags & STG_ALLOW_FOLDERS_IN_XREFS ? strrslash : strslash)(dest);
								if (p != NULL)
								{
									hyp_filetype ft;
									*p = '\0';
									ft = hyp_guess_filetype(dest);
									if (ft == HYP_FT_HYP || ft == HYP_FT_RSC)
									{
										is_xref = TRUE;
									} else
									{
										*p = '/';
									}
								}
								if (!is_xref)
								{
									symtab_entry *sym;
									
									sym = sym_find(*syms, dest, REF_LABELNAME);
									while (sym != NULL)
									{
										if (strcmp(sym->name, str) == 0)
											break;
										if (sym->lineno == line && sym->from_ref)
											break;
										sym = sym_find(sym->next, dest, REF_LABELNAME);
									}
									if (sym == NULL)
									{
										symtab_entry **last = syms;
										symtab_entry *sym;
										
										while (*last)
											last = &(*last)->next;
										sym = g_new(symtab_entry, 1);
										if (sym == NULL)
										{
											retval = FALSE;
										} else
										{
											sym->nodename = dest;
											dest = NULL;
											sym->type = REF_LABELNAME;
											sym->name = str;
											str = NULL;
											sym->lineno = line;
											sym->freeme = TRUE;
											sym->from_ref = FALSE;
											sym->from_idx = node == hyp->index_page;
											sym->referenced = FALSE;
											sym->next = NULL;
											*last = sym;
										}
									}
								}
							}
						}
						g_free(dest);
						g_free(str);
					}
					break;
					
				default:
					break;
				}
			} else if (*src == HYP_EOL)
			{
				++lineno;
				src++;
			} else
			{
				src++;
			}
		}
		++lineno;
		
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(opts->errorfile, _("%s: Node %u: failed to decode\n"), hyp->file, node);
	}

	return retval;
}

/* ------------------------------------------------------------------------- */

static symtab_entry *ref_loadsyms(HYP_DOCUMENT *hyp)
{
	symtab_entry *syms = NULL;
	symtab_entry **last = &syms;
	symtab_entry *sym;
	
	/* load REF if not done already */
	if (hyp->ref == NULL)
	{
		char *filename;
		int ret;

		filename = replace_ext(hyp->file, HYP_EXT_HYP, HYP_EXT_REF);

		ret = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret >= 0)
		{
			hyp->ref = ref_load(filename, ret, FALSE);
			hyp_utf8_close(ret);
		}
		g_free(filename);
	}
	ref_conv_to_utf8(hyp->ref);
	if (hyp->ref != NULL)
	{
		hyp_nodenr node_num;
		REF_MODULE *mod;
		char *nodename = NULL;
		const REF_ENTRY *entry;
		
		for (mod = hyp->ref->modules; mod != NULL; mod = mod->next)
		{
			for (node_num = 0; node_num < mod->num_entries; node_num++)
			{
				entry = &mod->entries[node_num];
				
				if (entry->type == REF_NODENAME)
				{
					nodename = entry->name.utf8;
				} else if (entry->type != REF_ALIASNAME && entry->type != REF_LABELNAME)
				{
					continue;
				}
				sym = g_new(symtab_entry, 1);
				if (sym == NULL)
					return syms;
				sym->nodename = nodename;
				sym->type = entry->type;
				sym->name = entry->name.utf8;
				sym->lineno = entry->lineno;
				sym->freeme = FALSE;
				sym->from_ref = TRUE;
				sym->from_idx = FALSE;
				sym->referenced = FALSE;
				sym->next = NULL;
				*last = sym;
				last = &(sym)->next;
			}
			/* only search the first module */
			break;
		}
	}
	return syms;
}

/* ------------------------------------------------------------------------- */

static void pdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
	PDF *pdf = (PDF *)user_data;
	hyp_utf8_fprintf(pdf->opts->errorfile, "PDF ERROR: error_no=%04X (%s), detail_no=%u\n", (unsigned int) error_no, HPDF_ErrorStr(error_no), (unsigned int) detail_no);
#if 0
	longjmp(pdf->error_env, 1);
#endif
}

/* ------------------------------------------------------------------------- */

static hyp_pic_format format_from_pic(hcp_opts *opts, INDEX_ENTRY *entry, hyp_pic_format default_format)
{
	hyp_pic_format format;

	format = opts->pic_format;
#ifndef HAVE_PNG
	if (default_format == HYP_PIC_PNG)
		default_format = HYP_PIC_GIF;
#endif
	if (format == HYP_PIC_ORIG)
	{
		/*
		 * not documented, but HCP seems to write the
		 * orignal file format into the "up" field
		 */
		format = (hyp_pic_format)entry->toc_index;
	}
	if ((opts->recompile_format == HYP_FT_HTML || opts->recompile_format == HYP_FT_HTML_XML) && (format == HYP_PIC_IMG || format == HYP_PIC_ICN))
	{
		format = default_format;
		hyp_utf8_fprintf(opts->errorfile, _("%sGEM images are not displayable in HTML, using %s instead\n"), _("warning: "), hcp_pic_format_to_name(default_format));
	}
	if (opts->recompile_format == HYP_FT_XML && (format == HYP_PIC_IMG || format == HYP_PIC_ICN))
	{
		format = default_format;
		/* hyp_utf8_fprintf(opts->errorfile, _("%sGEM images are not displayable in XML, using %s instead\n"), _("warning: "), hcp_pic_format_to_name(default_format)); */
	}
#ifndef HAVE_PNG
	if (format == HYP_PIC_PNG)
	{
		format = default_format;
		hyp_utf8_fprintf(opts->errorfile, _("%sPNG not supported on this platform, using %s instead\n"), _("warning: "), hcp_pic_format_to_name(default_format));
	}
#endif
	if (format < 1 || format > HYP_PIC_LAST)
	{
		format = default_format;
		hyp_utf8_fprintf(opts->errorfile, _("unknown image source type, using %s instead\n"), hcp_pic_format_to_name(default_format));
	}
	
	return format;
}

/* ------------------------------------------------------------------------- */

static char *pdf_quote_name(HYP_CHARSET from_charset, const char *name, size_t len, HYP_CHARSET output_charset, gboolean *converror)
{
	return hyp_conv_charset(from_charset, output_charset, name, len, converror);
}

/* ------------------------------------------------------------------------- */

static char *pdf_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node, HYP_CHARSET output_charset, gboolean *converror)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_charset(hyp->comp_charset, output_charset, entry->name, namelen, converror);
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_alias(PDF *pdf, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, symtab_entry *syms, gboolean *converror)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = pdf_quote_name(hyp->comp_charset, sym->name, STR0TERM, pdf->opts->output_charset, converror);
		/* hyp_utf8_sprintf_charset(out, pdf->opts->output_charset, "<a %s=\"%s\"></a>", html_name_attr, str); */
		g_free(str);
		sym->referenced = TRUE;
		sym = sym_find(sym->next, nodename, REF_ALIASNAME);
	}
	g_free(nodename);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_labels(PDF *pdf, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms, gboolean *converror)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_LABELNAME);
	while (sym)
	{
		if (sym->lineno == lineno)
		{
			char *str = pdf_quote_name(hyp->comp_charset, sym->name, STR0TERM, pdf->opts->output_charset, converror);
			/* hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- lineno %u --><a %s=\"%s\"></a>", sym->lineno, html_name_attr, str); */
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_gfx(PDF *pdf, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, int *gfx_id)
{
	(void)pdf;
	(void)hyp;
	(void)gfx;
	(void)gfx_id;
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_color(HPDF_Page page, uint32_t color)
{
	return HPDF_Page_SetRGBFill(page, ((color >> 16) & 0xff) / 255.0, ((color >> 8) & 0xff) / 255.0, ((color) & 0xff) / 255.0) == HPDF_NOERROR;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_str(PDF *pdf, HYP_DOCUMENT *hyp, const unsigned char *str, size_t len, gboolean *converror)
{
	char *dst;
	gboolean ret;
	
	dst = hyp_conv_charset(hyp->comp_charset, pdf->opts->output_charset, str, len, converror);
	ret = HPDF_Page_ShowText(pdf->page, dst) == HPDF_NOERROR;
	g_free(dst);
	return ret;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_generate_link(PDF *pdf, HYP_DOCUMENT *hyp, struct pdf_xref *xref, symtab_entry *syms, gboolean newwindow, unsigned char curtextattr, HPDF_Point *text_pos)
{
	gboolean retval = TRUE;
	HPDF_Destination dst;
	HPDF_Rect rect;
	HPDF_Annotation annot;

	(void)hyp;
	(void)syms;
	(void)newwindow;
	(void)curtextattr;

	switch (xref->desttype)
	{
	case HYP_NODE_EOF:
		retval &= pdf_out_color(pdf->page, viewer_colors.error);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_color(pdf->page, viewer_colors.text);
		break;
	case HYP_NODE_INTERNAL:
	case HYP_NODE_POPUP:
	case HYP_NODE_EXTERNAL_REF:
		rect.left = text_pos->x - 4;
		rect.bottom = text_pos->y - 4;
		rect.top = text_pos->y + pdf->line_height + 4;
		retval &= pdf_out_color(pdf->page, viewer_colors.link);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_color(pdf->page, viewer_colors.text);
		rect.right = text_pos->x + 4;
		dst = HPDF_Page_CreateDestination(pdf->pages[xref->dest_page]);
		annot = HPDF_Page_CreateLinkAnnot(pdf->page, rect, dst);
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_REXX_COMMAND:
		break;
	case HYP_NODE_REXX_SCRIPT:
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		break;
	case HYP_NODE_IMAGE:
		break;
	case HYP_NODE_QUIT:
		break;
	case HYP_NODE_CLOSE:
		break;
	default:
		/* hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"%s\">%s %u</a>", html_error_link_style, xref->destfilename, _("link to unknown node type"), hyp->indextable[xref->dest_page]->type); */
		break;
	}
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_graphics(PDF *pdf, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, long lineno, int *gfx_id)
{
	gboolean retval = TRUE;
	
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			retval &= pdf_out_gfx(pdf, hyp, gfx, gfx_id);
		}
		gfx = gfx->next;
	}
	return retval;
}

/* ------------------------------------------------------------------------- */

PDF *pdf_new(hcp_opts *opts)
{
	PDF *pdf;
	const char *encoding = NULL;

	pdf = g_new(PDF, 1);
	if (pdf == NULL)
		return pdf;
	pdf->opts = opts;
	pdf->hpdf = HPDF_New(pdf_error_handler, pdf);
	pdf->pages = NULL;
	switch (opts->output_charset)
	{
	case HYP_CHARSET_UTF8:
		HPDF_UseUTF8Encodings(pdf->hpdf);
		break;
	case HYP_CHARSET_MACROMAN:
		encoding = HPDF_ENCODING_MAC_ROMAN;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1250:
		encoding = HPDF_ENCODING_CP1250;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1252:
		encoding = HPDF_ENCODING_CP1252;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_LATIN1:
		encoding = HPDF_ENCODING_STANDARD;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_NONE:
	case HYP_CHARSET_BINARY:
	case HYP_CHARSET_BINARY_TABS:
	case HYP_CHARSET_ATARI:
	case HYP_CHARSET_CP850:
		break;
	}
	HPDF_SetCompressionMode(pdf->hpdf, HPDF_COMP_NONE);
	pdf->font = HPDF_GetFont(pdf->hpdf, "Courier", encoding);
	pdf->font_size = 12.0;
	pdf->line_height = 0;
	/* HPDF_SetViewerPreference(pdf->hpdf, HPDF_DISPLAY_TOC_TITLE); */

	return pdf;
}
	
/* ------------------------------------------------------------------------- */

void pdf_delete(PDF *pdf)
{
	if (pdf == NULL)
		return;
	HPDF_Free(pdf->hpdf);
	g_free(pdf->pages);
	g_free(pdf);
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_attr(PDF *pdf, struct textattr *attr)
{
	gboolean retval = TRUE;
	
	(void)pdf;
	if (attr->curattr != attr->newattr)
	{
		attr->curattr = attr->newattr;
	}

	if (attr->curfg != attr->newfg)
	{
		attr->curfg = attr->newfg;
	}

	if (attr->curbg != attr->newbg)
	{
		attr->curbg = attr->newbg;
	}

	return retval;
}

/* ------------------------------------------------------------------------- */

static HPDF_Page pdf_newpage(PDF *pdf)
{
	HPDF_Page page;
	
	page = pdf->page = HPDF_AddPage(pdf->hpdf);
	pdf_out_color(page, viewer_colors.text);
	HPDF_Page_SetFontAndSize(page, pdf->font, pdf->font_size); 
	HPDF_Page_SetTextRenderingMode(page, HPDF_FILL);
	pdf->line_height = (HPDF_Font_GetAscent(pdf->font) - HPDF_Font_GetDescent(pdf->font)) * pdf->font_size / 1000;
	pdf->page_height = HPDF_Page_GetHeight(pdf->page);

	return page;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_node(PDF *pdf, HYP_DOCUMENT *hyp, hyp_nodenr node, symtab_entry *syms)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	struct textattr attr;
	long lineno;
	struct hyp_gfx *hyp_gfx = NULL;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	int gfx_id = 0;
	gboolean converror = FALSE;
	gboolean in_text_out = FALSE;
	HPDF_Point text_pos;

#define BEGINTEXT() \
	if (!in_text_out) \
	{ \
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR; \
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos.x, text_pos.y) == HPDF_NOERROR; \
		in_text_out = TRUE; \
	}
#define ENDTEXT() \
	if (in_text_out) \
	{ \
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR; \
		in_text_out = FALSE; \
	}
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		BEGINTEXT(); \
		if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, user_colors[attr.curfg]); \
		retval &= pdf_out_str(pdf, hyp, textstart, src - textstart, &converror); \
		if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, viewer_colors.text); \
		at_bol = FALSE; \
	}
#define FLUSHLINE() \
	if (!at_bol) \
	{ \
		/* g_string_append_c(out, '\n'); */ \
		at_bol = TRUE; \
	}
#define FLUSHTREE() \
	if (in_tree != -1) \
	{ \
		/* hyp_utf8_sprintf_charset(out, opts->output_charset, "end tree %d -->\n", in_tree); */ \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	pdf->page = pdf->pages[node];

	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		INDEX_ENTRY *entry;
		unsigned short dithermask;

		entry = hyp->indextable[node];
		hyp_node_find_windowtitle(nodeptr);
		
		{
		char *title;
		
		if (nodeptr->window_title)
		{
			title = hyp_conv_charset(hyp->comp_charset, pdf->opts->output_charset, nodeptr->window_title, STR0TERM, &converror);
		} else
		{
			title = pdf_quote_nodename(hyp, node, pdf->opts->output_charset, &converror);
		}

		/*
		 * scan through esc commands, gathering graphic commands
		 */
		src = nodeptr->start;
		end = nodeptr->end;
		dithermask = 0;
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
						hyp_decode_gfx(hyp, src + 1, adm, pdf->opts->errorfile, pdf->opts->read_images);
						if (adm->type == HYP_ESC_PIC)
						{
							adm->format = format_from_pic(pdf->opts, hyp->indextable[adm->extern_node_index], PDF_DEFAULT_PIC_TYPE);
							adm->dithermask = dithermask;
							dithermask = 0;
						}
					}
				}
				break;
			case HYP_ESC_WINDOWTITLE:
				/* @title already output */
				break;
			case HYP_ESC_EXTERNAL_REFS:
				break;
			case HYP_ESC_DITHERMASK:
				if (src[2] == 5u)
					dithermask = short_from_chars(&src[3]);
				break;
			default:
				break;
			}
			src = hyp_skip_esc(src);
		}

		/*
		 * join vertical lines,
		 * otherwise we get small gaps.
		 * downcase: this will print wrong commands when embedding the stg source in html
		 */
		{
			struct hyp_gfx *gfx1, *gfx2;
			
			for (gfx1 = hyp_gfx; gfx1 != NULL; gfx1 = gfx1->next)
			{
				if (gfx1->type == HYP_ESC_LINE && gfx1->width == 0 && gfx1->begend == 0)
				{
					for (gfx2 = gfx1->next; gfx2 != NULL; gfx2 = gfx2->next)
					{
						if (gfx2->type == HYP_ESC_LINE && gfx2->width == 0 && gfx2->begend == 0 &&
							gfx1->x_offset == gfx2->x_offset &&
							gfx1->style == gfx2->style &&
							(gfx1->y_offset + gfx1->height) == gfx2->y_offset)
						{
							gfx1->height += gfx2->height;
							gfx2->type = 0;
							gfx2->used = TRUE;
						}
					}
				}
			}
		}
		
		g_free(title);
		}

		{
			/*
			 * check for alias names in ref file
			 */
			retval &= pdf_out_alias(pdf, hyp, entry, syms, &converror);
			/*
			 * now output data
			 */
			src = nodeptr->start;
			textstart = src;
			at_bol = TRUE;
			in_tree = -1;
			attr.curattr = attr.newattr = 0;
			attr.curfg = attr.newfg = HYP_DEFAULT_FG;
			attr.curbg = attr.newbg = HYP_DEFAULT_BG;
			lineno = 0;
			text_pos.x = 0;
			text_pos.y = pdf->page_height - pdf->line_height - lineno * pdf->line_height;
			retval &= pdf_out_labels(pdf, hyp, entry, lineno, syms, &converror);
			retval &= pdf_out_graphics(pdf, hyp, hyp_gfx, lineno, &gfx_id);
			
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
						BEGINTEXT();
						retval &= pdf_out_str(pdf, hyp, (const unsigned char *)"\033", 1, &converror);
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
							gboolean str_equal;
							struct pdf_xref xref;
							
							xref.line = 0;
							type = *src;
							if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
							{
								xref.line = DEC_255(&src[1]);
								src += 2;
							}
							xref.dest_page = DEC_255(&src[1]);
							src += 3;
							if (hypnode_valid(hyp, xref.dest_page))
							{
								INDEX_ENTRY *dest_entry = hyp->indextable[xref.dest_page];
								xref.destname = hyp_conv_to_utf8(hyp->comp_charset, dest_entry->name, dest_entry->length - SIZEOF_INDEX_ENTRY);
								xref.desttype = (hyp_indextype) dest_entry->type;
							} else
							{
								xref.destname = hyp_invalid_page(xref.dest_page);
								xref.desttype = HYP_NODE_EOF;
							}
	
							if (*src <= HYP_STRLEN_OFFSET)
							{
								src++;
								if (hypnode_valid(hyp, xref.dest_page))
								{
									INDEX_ENTRY *entry = hyp->indextable[xref.dest_page];
									len = entry->length - SIZEOF_INDEX_ENTRY;
									xref.text = pdf_quote_nodename(hyp, xref.dest_page, pdf->opts->output_charset, &converror);
									str_equal = entry->type == HYP_NODE_INTERNAL;
								} else
								{
									str_equal = FALSE;
									xref.text = g_strdup(xref.destname);
									len = strlen(xref.text);
								}
							} else
							{
								len = *src - HYP_STRLEN_OFFSET;
								src++;
								xref.text = pdf_quote_name(hyp->comp_charset, (const char *)src, len, pdf->opts->output_charset, &converror);
								src += len;
								if (hypnode_valid(hyp, xref.dest_page))
								{
									INDEX_ENTRY *dest_entry = hyp->indextable[xref.dest_page];
									str_equal = dest_entry->type == HYP_NODE_INTERNAL && strcmp(xref.text, xref.destname) == 0;
								} else
								{
									str_equal = FALSE;
								}
							}
							FLUSHTREE();
							UNUSED(str_equal);
							
							text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
							ENDTEXT();
							retval &= pdf_generate_link(pdf, hyp, &xref, syms, type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE, attr.curattr, &text_pos);
	
							g_free(xref.destname);
							g_free(xref.text);
							at_bol = FALSE;
						}
						break;
						
					case HYP_ESC_EXTERNAL_REFS:
						FLUSHTREE();
						FLUSHLINE();
						/* @xref already output */
						if (src[1] < 5u)
							src += 4;
						else
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
								str = pdf_quote_nodename(hyp, dest_page, pdf->opts->output_charset, &converror);
							} else
							{
								str = hyp_invalid_page(dest_page);
							}
							FLUSHLINE();
							if (tree != in_tree)
							{
								FLUSHTREE();
								/* hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- begin tree %d\n", tree); */
								in_tree = tree;
							}
							/* hyp_utf8_sprintf_charset(out, opts->output_charset, "   %d \"%s\" %u\n", obj, str, line); */
							(void)line;
							(void)obj;
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
						FLUSHTREE();
						attr.newattr = *src - HYP_ESC_TEXTATTR_FIRST;
						retval &= pdf_out_attr(pdf, &attr);
						src++;
						break;
					
					case HYP_ESC_FG_COLOR:
						FLUSHTREE();
						src++;
						attr.newfg = *src;
						retval &= pdf_out_attr(pdf, &attr);
						src++;
						break;
				
					case HYP_ESC_BG_COLOR:
						FLUSHTREE();
						src++;
						attr.newbg = *src;
						retval &= pdf_out_attr(pdf, &attr);
						src++;
						break;
				
					case HYP_ESC_UNKNOWN_A4:
						if (pdf->opts->print_unknown)
							hyp_utf8_fprintf(pdf->opts->errorfile, _("<unknown hex esc $%02x>\n"), *src);
						src++;
						break;

					default:
						if (pdf->opts->print_unknown)
							hyp_utf8_fprintf(pdf->opts->errorfile, _("<unknown hex esc $%02x>\n"), *src);
						src++;
						break;
					}
					textstart = src;
				} else if (*src == HYP_EOL)
				{
					FLUSHTREE();
					DUMPTEXT();
					ENDTEXT();
					at_bol = TRUE;
					++lineno;
					text_pos.x = 0;
					text_pos.y = pdf->page_height - pdf->line_height - lineno * pdf->line_height;
					retval &= pdf_out_labels(pdf, hyp, entry, lineno, syms, &converror);
					retval &= pdf_out_graphics(pdf, hyp, hyp_gfx, lineno, &gfx_id);
					src++;
					textstart = src;
				} else
				{
					FLUSHTREE();
					src++;
				}
			}
			DUMPTEXT();
			ENDTEXT();
			attr.newattr = 0;
			attr.newfg = HYP_DEFAULT_FG;
			attr.newbg = HYP_DEFAULT_BG;
			retval &= pdf_out_attr(pdf, &attr);
			at_bol = FALSE;
			FLUSHLINE();
			FLUSHTREE();
			++lineno;
			text_pos.x = 0;
			text_pos.y = pdf->page_height - pdf->line_height - lineno * pdf->line_height;
			retval &= pdf_out_labels(pdf, hyp, entry, lineno, syms, &converror);
			retval &= pdf_out_graphics(pdf, hyp, hyp_gfx, lineno, &gfx_id);
			
			if (hyp_gfx != NULL)
			{
				struct hyp_gfx *gfx, *next;
				
				for (gfx = hyp_gfx; gfx != NULL; gfx = next)
				{
					if (!gfx->used)
					{
						/* hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- gfx unused: "); */
						pdf_out_gfx(pdf, hyp, gfx, &gfx_id);
						/* hyp_utf8_sprintf_charset(out, opts->output_charset, "-->\n"); */
					}
					next = gfx->next;
					g_free(gfx);
				}
			}
		}
		
#if 0
		{
			char buf[512];
			HPDF_Point p;
			
			if (HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR)
			{
				HPDF_Page_MoveTextPos(pdf->page, 100, 500);
				HPDF_Page_ShowText(pdf->page, "Hello, ");
				HPDF_Page_ShowText(pdf->page, "world");
				HPDF_Page_MoveTextPos(pdf->page, 0, -30);
				p = HPDF_Page_GetCurrentTextPos(pdf->page);
				sprintf(buf, "pos before %.2f %.2f", p.x, p.y);
				HPDF_Page_ShowText(pdf->page, buf);
				p = HPDF_Page_GetCurrentTextPos(pdf->page);
				sprintf(buf, " after %.2f %.2f", p.x, p.y);
				HPDF_Page_ShowText(pdf->page, buf);
				/* HPDF_Page_EndText(pdf->page); */
			}
		}
#endif
	} else
	{
		char *text = g_strdup_printf(_("%s: Node %u: failed to decode"), hyp->file, node);
		HPDF_REAL tw = HPDF_Page_TextWidth(pdf->page, text);
		HPDF_Page_BeginText(pdf->page);
		HPDF_Page_TextOut(pdf->page, (HPDF_Page_GetWidth(pdf->page) - tw) / 2, HPDF_Page_GetHeight(pdf->page) - 50, text);
		HPDF_Page_EndText(pdf->page);
		g_free(text);
	}
	
#undef DUMPTEXT
#undef BEGINTEXT
#undef FLUSHLINE
#undef FLUSHTREE
	return retval;
}

/* ------------------------------------------------------------------------- */

static char *pdf_datestr(time_t t)
{
	struct tm tm;
	int gmtoff;
	int c;
	
	localtime_r(&t, &tm);
	gmtoff = (int)(tm.tm_gmtoff / 60);
	c = gmtoff < 0 ? '-' : '+';
	if (gmtoff < 0)
		gmtoff = -gmtoff;
	return g_strdup_printf("D:%04d%02d%02d%02d%02d%02d%c%02d'%02d'",
		tm.tm_year + 1900,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec,
		c,
		gmtoff / 60,
		gmtoff % 60);
}

/* ------------------------------------------------------------------------- */

static void pdf_out_globals(HYP_DOCUMENT *hyp, PDF *pdf)
{
	char *str;
	struct stat s;
	
#define STR(t, x) \
	if (x != NULL) \
	{ \
		HPDF_SetInfoAttr(pdf->hpdf, t, x); \
	}

	STR(HPDF_INFO_TITLE, hyp->database);
	STR(HPDF_INFO_AUTHOR, hyp->author);
	STR(HPDF_INFO_SUBJECT, hyp->subject);

	HPDF_SetInfoAttr(pdf->hpdf, HPDF_INFO_CREATOR, hyp->comp_vers >= 6 || hyp->language != NULL ? gl_program_name : "ST-GUIDE");
	str = g_strdup_printf("%s %s for %s, using Haru Free PDF Library %s", gl_program_name, gl_program_version, hyp_osname(hyp_get_current_os()), HPDF_GetVersion());
	STR(HPDF_INFO_PRODUCER, str);
	g_free(str);
	if (hyp_utf8_stat(hyp->file, &s) == 0)
	{
		str = pdf_datestr(s.st_mtime);
		STR(HPDF_INFO_CREATION_DATE, str);
		g_free(str);
	}

#undef STR
}

/* ------------------------------------------------------------------------- */

gboolean recompile_pdf(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	volatile gboolean ret;
	int i;
	gboolean found;
	symtab_entry *syms;
	PDF *pdf;
	HPDF_Stream stream;
	HPDF_UINT idx, length;
	HPDF_BYTE *buf;
	
	/* force_crlf = FALSE; */

	/*
	 * TODO: get from config
	 */
	viewer_colors.background = 0xfffff;
	viewer_colors.text = 0x000000;
	viewer_colors.link = 0x0000ff;
	viewer_colors.link_effect = HYP_TXT_BOLD | HYP_TXT_UNDERLINED;
	viewer_colors.popup = 0x00ff00;
	viewer_colors.xref = 0xff0000;
	viewer_colors.system = 0xff00ff;
	viewer_colors.rx = 0xff00ff;
	viewer_colors.rxs = 0xff00ff;
	viewer_colors.quit = 0xff0000;
	viewer_colors.close = 0xff0000;
	viewer_colors.ghosted = 0xcccccc;
	viewer_colors.error = 0xff0000;

	ret = TRUE;
		
	if (opts->read_images && hyp->cache == NULL)
		InitCache(hyp);
	
	/* load REF if not done already */
	syms = ref_loadsyms(hyp);
	
	pdf = pdf_new(opts);

#if 0
	if (setjmp(pdf->error_env))
	{
		pdf_delete(pdf);
		return FALSE;
	}
#endif

	pdf->pages = g_new(HPDF_Page, hyp->num_index);
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			pdf->pages[node] = pdf_newpage(pdf);
			break;
		default:
			pdf->pages[node] = 0;
			break;
		}
	}
	
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= sym_check_links(hyp, pdf->opts, node, &syms);
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
	
	pdf_out_globals(hyp, pdf);

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
			if (!opts->gen_index || argc != 0)
				continue;
		}

		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= pdf_out_node(pdf, hyp, node, syms);
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

	HPDF_SaveToStream(pdf->hpdf, &stream);
	idx = 0;
	while ((buf = HPDF_MemStream_GetBufPtr(stream, idx, &length)) != NULL)
	{
		fwrite(buf, 1, length, opts->outfile);
		idx++;
	}

	for (i = 0; i < argc; i++)
	{
		if (argv[i] != NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, _("'%s' not found.\n"), argv[i]);
			ret = FALSE;
		}
	}
	
	pdf_delete(pdf);

	return ret;
}

#else

extern int _I_dont_care_that_ISO_C_forbids_an_empty_source_file_;

#endif /* WITH_PDF */
