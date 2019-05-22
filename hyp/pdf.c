#include "hypdefs.h"
#include "hypdebug.h"
#include "hcp_opts.h"
#include "pdf.h"
#include "outcomm.h"

#ifdef WITH_PDF /* whole file */

static struct {
	HPDF_RGBColor background;           /* window background color */
	HPDF_RGBColor text;                 /* Displays text in the specified color */
	HPDF_RGBColor link;                 /* Displays references in the specified color */
	HPDF_RGBColor popup;                /* Displays references to popups in the specified color */
	HPDF_RGBColor xref;                 /* Displays external references in the specified color */
	HPDF_RGBColor system;               /* Displays references to {@ system } in the specified color */
	HPDF_RGBColor rx;                   /* Displays references to {@ rx } in the specified color */
	HPDF_RGBColor rxs;                  /* Displays references to {@ rxs } in the specified color */
	HPDF_RGBColor quit;                 /* Displays references to {@ quit } in the specified color */
	HPDF_RGBColor close;                /* Displays references to {@ close } in the specified color */
	HPDF_RGBColor ghosted;			    /* for "ghosted" effect (attribute @{G}) */
	HPDF_RGBColor error;
} viewer_colors;

static HPDF_RGBColor user_colors[16] = {
	{ 1.0, 1.0, 1.0 },
	{ 0.0, 0.0, 0.0 },
	{ 1.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 1.0 },
	{ 0.0, 1.0, 1.0 },
	{ 1.0, 1.0, 0.0 },
	{ 1.0, 0.0, 1.0 },
	{ 0.8, 0.8, 0.8 },
	{ 0.533, 0.533, 0.533 },
	{ 0.533, 0.0, 0.0 },
	{ 0.0, 0.533, 0.0 },
	{ 0.0, 0.0, 0.533 },
	{ 0.0, 0.533, 0.533 },
	{ 0.533, 0.533, 0.0 },
	{ 0.533, 0.0, 0.533 }
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

#define LINK_BORDER_WIDTH 0

static HPDF_REAL text_xoffset;
static HPDF_REAL text_yoffset;

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

static gboolean pdf_out_color(HPDF_Page page, const HPDF_RGBColor *color)
{
	return HPDF_Page_SetRGBFill(page, color->r, color->g, color->b) == HPDF_NOERROR;
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

static gboolean pdf_out_curattr(PDF *pdf, struct textattr *attr)
{
	gboolean retval = TRUE;

	(void)pdf;
	(void)attr;
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_curcolor(PDF *pdf, struct textattr *attr)
{
	gboolean retval = TRUE;

	if (attr->curfg == HYP_DEFAULT_FG)
		retval &= pdf_out_color(pdf->page, &viewer_colors.text);
	else
		retval &= pdf_out_color(pdf->page, &user_colors[attr->curfg]);
	return retval;
}

/* ------------------------------------------------------------------------- */

/*
 * Disallow certain characters that might clash with
 * the filesystem or uri escape sequences, and also any non-ascii characters.
 * For simplicity, this is done in-place.
 */
static void pdf_convert_filename(char *filename)
{
	char *p = filename;
	unsigned char c;
	
	while ((c = *p) != '\0')
	{
		if (c == ' ' ||
			c == ':' ||
			c == '%' ||
			c == '?' ||
			c == '*' ||
			c == '/' ||
			c == '&' ||
			c == '<' ||
			c == '>' ||
			c == '"' ||
			c == '\'' ||
			c == '\\' ||
			c >= 0x7f ||
			c < 0x20)
		{
			c = '_';
		} else if (c >= 'A' && c <= 'Z')
		{
			/* make it lowercase. should eventually be configurable */
			c = c - 'A' + 'a';
		}
		*p++ = c;
	}
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_generate_link(PDF *pdf, HYP_DOCUMENT *hyp, struct pdf_xref *xref, symtab_entry *syms, gboolean newwindow, struct textattr *attr, HPDF_Point *text_pos)
{
	gboolean retval = TRUE;
	HPDF_Destination dst;
	HPDF_Rect rect;
	HPDF_Annotation annot;

	rect.left = text_pos->x - LINK_BORDER_WIDTH;
	rect.bottom = text_pos->y - LINK_BORDER_WIDTH;
	rect.top = text_pos->y + pdf->line_height + LINK_BORDER_WIDTH;

	switch (xref->desttype)
	{
	case HYP_NODE_EOF:
		retval &= pdf_out_color(pdf->page, &viewer_colors.error);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		break;
	case HYP_NODE_INTERNAL:
	case HYP_NODE_POPUP:
	case HYP_NODE_EXTERNAL_REF:
		{
			gboolean is_xref = FALSE;
			const HPDF_RGBColor *style_color;

			if (xref->desttype == HYP_NODE_EXTERNAL_REF)
			{
				char *p = ((hyp->st_guide_flags & STG_ALLOW_FOLDERS_IN_XREFS) ? strrslash : strslash)(xref->destname);
				char c = '\0';
				hyp_filetype ft;
				style_color = &viewer_colors.xref;
				if (p != NULL)
				{
					c = *p;
					*p = '\0';
				}
				ft = hyp_guess_filetype(xref->destname);
				is_xref = ft != HYP_FT_NONE;
				if (ft == HYP_FT_RSC)
				{
					/*
					 * generate link to a resource file
					 */
					/* TODO */
				} else if (ft == HYP_FT_HYP)
				{
					/*
					 * generate link to a remote hyp file
					 */
					/*
					 * basename here is as specified in the link,
					 * which is often all uppercase.
					 * Always convert to lowercase first.
					 */
					char *base = hyp_utf8_strdown(hyp_basename(xref->destname), STR0TERM);
					char *pdfbase = replace_ext(base, HYP_EXT_HYP, HYP_EXT_PDF);
					pdf_convert_filename(pdfbase);
					retval &= pdf_out_color(pdf->page, style_color);
					retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
					retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
					retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
					*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = NULL; /* XXX should be named dest */
					annot = HPDF_Page_CreateGoToRAnnot(pdf->page, rect, pdfbase, p ? p + 1 : "Main", newwindow);
					HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
					HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
					g_free(pdfbase);
					g_free(base);
				}
				if (p)
					*p = c;
			} else if (xref->desttype == HYP_NODE_POPUP)
			{
				style_color = &viewer_colors.popup;
			} else
			{
				style_color = &viewer_colors.link;
			}
			
			if (!is_xref)
			{
				symtab_entry *sym;
				symtab_entry *label;
				
				label = NULL;
				if (xref->line != 0)
				{
					sym = sym_find(syms, xref->destname, REF_LABELNAME);
					while (sym)
					{
						if (sym->lineno == xref->line /* && !sym->from_idx */)
						{
							label = sym;
							sym->referenced = TRUE;
							break;
						}
						sym = sym_find(sym->next, xref->destname, REF_LABELNAME);
					}
				}
				if (label)
				{
					/*
					 * generate link to a label
					 */
					retval &= pdf_out_color(pdf->page, style_color);
					retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
					retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
					retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
					*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = HPDF_Page_CreateDestination(pdf->pages[xref->dest_page]);
					HPDF_Destination_SetFitBH(dst, (sym->lineno - 1) * pdf->line_height);
					annot = HPDF_Page_CreateGoToAnnot(pdf->page, rect, dst);
					HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
					HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
				} else if (xref->desttype == HYP_NODE_POPUP && !newwindow)
				{
					/*
					 * generate link to a popup node
					 */
					retval &= pdf_out_color(pdf->page, style_color);
					retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
					retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
					retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
					*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = HPDF_Page_CreateDestination(pdf->pages[xref->dest_page]);
					annot = HPDF_Page_CreateLinkAnnot(pdf->page, rect, dst);
					HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
					HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
				} else
				{
					/*
					 * generate link to a regular node
					 */
					retval &= pdf_out_color(pdf->page, style_color);
					retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
					retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
					retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
					*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = HPDF_Page_CreateDestination(pdf->pages[xref->dest_page]);
					annot = HPDF_Page_CreateLinkAnnot(pdf->page, rect, dst);
					HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
					HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
				}
			}
		}
		break;
	case HYP_NODE_REXX_COMMAND:
		retval &= pdf_out_color(pdf->page, &viewer_colors.rx);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateLaunchAnnot(pdf->page, rect, xref->destname, NULL, NULL);
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_REXX_SCRIPT:
		retval &= pdf_out_color(pdf->page, &viewer_colors.rxs);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateLaunchAnnot(pdf->page, rect, xref->destname, NULL, NULL);
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		retval &= pdf_out_color(pdf->page, &viewer_colors.system);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateLaunchAnnot(pdf->page, rect, xref->destname, NULL, NULL);
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_IMAGE:
		/* that would be an inline image; currently not supported by compiler */
		retval &= pdf_out_color(pdf->page, &viewer_colors.system);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		break;
	case HYP_NODE_QUIT:
		retval &= pdf_out_color(pdf->page, &viewer_colors.quit);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateNamedAnnot(pdf->page, rect, "Quit");
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_CLOSE:
		retval &= pdf_out_color(pdf->page, &viewer_colors.close);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		*text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateNamedAnnot(pdf->page, rect, "Close");
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
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
	
	if (attr->curattr != attr->newattr)
	{
		attr->curattr = attr->newattr;
		pdf_out_curattr(pdf, attr);
	}

	if (attr->curfg != attr->newfg)
	{
		attr->curfg = attr->newfg;
		pdf_out_curcolor(pdf, attr);
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
	pdf_out_color(page, &viewer_colors.text);
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
		if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, &user_colors[attr.curfg]); \
		retval &= pdf_out_str(pdf, hyp, textstart, src - textstart, &converror); \
		if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, &viewer_colors.text); \
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
		struct pdf_xref *xrefs = NULL;
		struct pdf_xref **last_xref = &xrefs;
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
					{
						hyp_nodenr dest_page;
						char *destname;
						char *text;
						char *buf;
						hyp_indextype desttype;
						
						dest_page = DEC_255(&src[3]);
						buf = hyp_conv_to_utf8(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
						buf = chomp(buf);
						text = pdf_quote_name(hyp->comp_charset, buf, STR0TERM, pdf->opts->output_charset, &converror);
						g_free(buf);
						if (hypnode_valid(hyp, dest_page))
						{
							INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
							destname = hyp_conv_to_utf8(hyp->comp_charset, dest_entry->name, dest_entry->length - SIZEOF_INDEX_ENTRY);
							desttype = (hyp_indextype) dest_entry->type;
						} else
						{
							destname = hyp_invalid_page(dest_page);
							desttype = HYP_NODE_EOF;
						}
						if (empty(text))
						{
							g_free(text);
							text = g_strdup(destname);
						}
						{
							struct pdf_xref *xref;
							xref = g_new0(struct pdf_xref, 1);
							xref->text = text;
							xref->destname = destname;
							xref->dest_page = dest_page;
							xref->desttype = desttype;
							xref->line = 0;
							xref->next = NULL;
							*last_xref = xref;
							last_xref = &(xref)->next;
						}
					}
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
			
			/*
			 * not yet supported: xrefs
			 */
			
			/*
			 * there seems to be no way to change the window title
			 */
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
			text_pos.x = text_xoffset;
			text_pos.y = pdf->page_height - pdf->line_height - lineno * pdf->line_height - text_yoffset;
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
							
							if (in_text_out)
								text_pos = HPDF_Page_GetCurrentTextPos(pdf->page);
							ENDTEXT();
							retval &= pdf_generate_link(pdf, hyp, &xref, syms, type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE, &attr, &text_pos);
	
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
					text_pos.x = text_xoffset;
					text_pos.y = pdf->page_height - pdf->line_height - lineno * pdf->line_height - text_yoffset;
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
			text_pos.x = text_xoffset;
			text_pos.y = pdf->page_height - pdf->line_height - lineno * pdf->line_height - text_yoffset;
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
		
		{
			struct pdf_xref *xref, *next;
			
			for (xref = xrefs; xref != NULL; xref = next)
			{
				next = xref->next;
				g_free(xref->destname);
				g_free(xref->text);
				g_free(xref);
			}
		}
		
		hyp_node_free(nodeptr);
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
	str = pdf_datestr(s.st_mtime);
	STR(HPDF_INFO_CREATION_DATE, str);
	g_free(str);
	if (hyp_utf8_stat(hyp->file, &s) == 0)
	{
		str = pdf_datestr(s.st_mtime);
		STR(HPDF_INFO_MOD_DATE, str);
		g_free(str);
	}

#undef STR
}

static unsigned char parse_hex(const char *str)
{
	unsigned char val;
	if (str[0] >= '0' && str[0] <= '9')
		val = str[0] - '0';
	else if (str[0] >= 'a' && str[0] <= 'f')
		val = str[0] - 'a' + 10;
	else if (str[0] >= 'A' && str[0] <= 'F')
		val = str[0] - 'A' + 10;
	else
		val = 0;
	val <<= 4;
	if (str[1] >= '0' && str[1] <= '9')
		val |= str[1] - '0';
	else if (str[1] >= 'a' && str[1] <= 'f')
		val |= str[1] - 'a' + 10;
	else if (str[1] >= 'A' && str[1] <= 'F')
		val |= str[1] - 'A' + 10;
	return val;
}

/*** ---------------------------------------------------------------------- ***/

static void parse_color(const char *name, HPDF_RGBColor *color)
{
	if (name == NULL || *name != '#' || strlen(name) != 7)
	{
		color->r = color->g = color->b = 0;
		return;
	}
	color->r = parse_hex(name + 1) / 255.0;
	color->g = parse_hex(name + 3) / 255.0;
	color->b = parse_hex(name + 5) / 255.0;
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

	parse_color(gl_profile.colors.background, &viewer_colors.background);
	parse_color(gl_profile.colors.text, &viewer_colors.text);
	parse_color(gl_profile.colors.link, &viewer_colors.link);
	parse_color(gl_profile.colors.xref, &viewer_colors.xref);
	parse_color(gl_profile.colors.popup, &viewer_colors.popup);
	parse_color(gl_profile.colors.system, &viewer_colors.system);
	parse_color(gl_profile.colors.rx, &viewer_colors.rx);
	parse_color(gl_profile.colors.rxs, &viewer_colors.rxs);
	parse_color(gl_profile.colors.quit, &viewer_colors.quit);
	parse_color(gl_profile.colors.close, &viewer_colors.close);
	parse_color(gl_profile.colors.ghosted, &viewer_colors.ghosted);
	parse_color("#ff0000", &viewer_colors.error); /* used to display invalid links in hypertext files */

	text_xoffset = (gl_profile.viewer.text_xoffset / 90.0) * 72.0;
	text_yoffset = (gl_profile.viewer.text_yoffset / 90.0) * 72.0;

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

	ret &= HPDF_SaveToStream(pdf->hpdf, &stream) == HPDF_NOERROR;
	if (ret)
	{
		idx = 0;
		if (stream)
		{
			while ((buf = HPDF_MemStream_GetBufPtr(stream, idx, &length)) != NULL)
			{
				fwrite(buf, 1, length, opts->outfile);
				idx++;
			}
		}
	}
	
	if (cmdline_version)
		ClearCache(hyp);
	
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
