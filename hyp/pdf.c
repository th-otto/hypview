#define __HYP_PDF_IMPLEMENTATION__
#include "hypdefs.h"
#include "hypdebug.h"
#include "hcp_opts.h"
#include <setjmp.h>
#include "hpdf.h"
#include "pdf.h"
#include "outcomm.h"
#include "pattern.h"
#include <math.h>
#include "picture.h"

#ifdef WITH_PDF /* whole file */

struct pdf_page {
	HPDF_Page page;
	hyp_nodenr node;
};

struct _pdf {
	HPDF_Doc hpdf;
	hcp_opts *opts;
	jmp_buf error_env;
	HPDF_Font regular_font;
	HPDF_Font bold_font;
	HPDF_Font italic_font;
	HPDF_Font bold_italic_font;
	HPDF_REAL font_size;
	HPDF_REAL ascent;
	HPDF_REAL descent;
	HPDF_REAL line_height;
	HPDF_REAL cell_width;
	size_t num_pages;
	size_t curr_page_num;
	struct pdf_page *pages;
	HPDF_Page *links;
	HPDF_Page page;
	HPDF_REAL page_width;
	HPDF_REAL page_height;
	struct hyp_gfx *hyp_gfx;
	HPDF_PatternColorspace pattern_colorspace;
	HPDF_Pattern patterns[NUM_PATTERNS];
	HPDF_Image *images;
};


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

#define IP_HOLLOW		0
#define IP_1PATT		1
#define IP_2PATT		2
#define IP_3PATT		3
#define IP_4PATT		4
#define IP_5PATT		5
#define IP_6PATT		6
#define IP_7PATT		7
#define IP_WIN_SOLID	8

#define XMAX_PLANES 32


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
		/* hyp_utf8_sprintf_charset(out, pdf->opts->output_charset, converror, "<a %s=\"%s\"></a>", html_name_attr, str); */
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
			/* hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- lineno %u --><a %s=\"%s\"></a>", sym->lineno, html_name_attr, str); */
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_fill_color(HPDF_Page page, const HPDF_RGBColor *color)
{
	return HPDF_Page_SetRGBFill(page, color->r, color->g, color->b) == HPDF_NOERROR;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_stroke_color(HPDF_Page page, const HPDF_RGBColor *color)
{
	return HPDF_Page_SetRGBStroke(page, color->r, color->g, color->b) == HPDF_NOERROR;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_color(HPDF_Page page, const HPDF_RGBColor *color)
{
	return pdf_out_fill_color(page, color) && pdf_out_stroke_color(page, color);
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_str(PDF *pdf, HYP_DOCUMENT *hyp, HPDF_Point *text_pos, const unsigned char *str, size_t len, gboolean *converror, struct textattr *attr)
{
	char *dst;
	gboolean ret = TRUE;

	dst = hyp_conv_charset(hyp->comp_charset, pdf->opts->output_charset, str, len, converror);
	if (attr->curbg != HYP_DEFAULT_BG)
	{
		HPDF_TextWidth tw;
		HPDF_RGBColor color;
		
		HPDF_Font_TextWidth(pdf->regular_font, str, len, &tw);
		HPDF_Page_GetRGBFill(pdf->page, &color);
		pdf_out_fill_color(pdf->page, &user_colors[attr->curbg]);
		HPDF_Page_Rectangle(pdf->page, text_pos->x, text_pos->y, tw.width * pdf->font_size / 1000, pdf->line_height);
		HPDF_Page_Fill(pdf->page);
		
		pdf_out_fill_color(pdf->page, &color);
	}
	ret &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
	ret &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
	if (attr->curattr & HYP_TXT_UNDERLINED)
	{
		HPDF_Point oldpos;
		HPDF_REAL y;
		
		oldpos = *text_pos;
		ret &= HPDF_Page_ShowText(pdf->page, dst) == HPDF_NOERROR;
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		ret &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		y = oldpos.y + pdf->descent;
		HPDF_Page_MoveTo(pdf->page, oldpos.x, y);
		HPDF_Page_LineTo(pdf->page, text_pos->x, y);
		HPDF_Page_Stroke(pdf->page);
	} else
	{
		ret = HPDF_Page_ShowText(pdf->page, dst) == HPDF_NOERROR;
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		ret &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
	}
	g_free(dst);
	return ret;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_curattr(PDF *pdf, struct textattr *attr)
{
	gboolean retval = TRUE;

	switch (attr->curattr & (HYP_TXT_BOLD | HYP_TXT_ITALIC))
	{
		case 0: HPDF_Page_SetFontAndSize(pdf->page, pdf->regular_font, pdf->font_size); break;
		case HYP_TXT_BOLD: HPDF_Page_SetFontAndSize(pdf->page, pdf->bold_font, pdf->font_size); break;
		case HYP_TXT_ITALIC: HPDF_Page_SetFontAndSize(pdf->page, pdf->italic_font, pdf->font_size); break;
		case HYP_TXT_BOLD | HYP_TXT_ITALIC: HPDF_Page_SetFontAndSize(pdf->page, pdf->regular_font, pdf->font_size); break;
	}
	if (attr->curattr & HYP_TXT_LIGHT)
		retval &= pdf_out_color(pdf->page, &viewer_colors.ghosted);
	else if (attr->curfg == HYP_DEFAULT_FG)
		retval &= pdf_out_color(pdf->page, &viewer_colors.text);
	else
		retval &= pdf_out_color(pdf->page, &user_colors[attr->curfg]);

	if (attr->curattr & HYP_TXT_SHADOWED)
	{
		HPDF_Page_SetTextRenderingMode(pdf->page, HPDF_FILL_THEN_STROKE);
		pdf_out_fill_color(pdf->page, &viewer_colors.background);
	} else if (attr->curattr & HYP_TXT_OUTLINED)
	{
		HPDF_Page_SetTextRenderingMode(pdf->page, HPDF_STROKE);
	} else
	{
		HPDF_Page_SetTextRenderingMode(pdf->page, HPDF_FILL);
	}

	/*
	 * underlines are drawn manually
	 */

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
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
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
					HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = NULL; /* XXX should be named dest */
					annot = HPDF_Page_CreateGoToRAnnot(pdf->page, &rect, pdfbase, p ? p + 1 : "Main", newwindow);
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
					HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = HPDF_Page_CreateDestination(pdf->links[xref->dest_page]);
					HPDF_Destination_SetFitBH(dst, (label->lineno - 1) * pdf->line_height);
					annot = HPDF_Page_CreateGoToAnnot(pdf->page, &rect, dst);
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
					HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = HPDF_Page_CreateDestination(pdf->links[xref->dest_page]);
					annot = HPDF_Page_CreateLinkAnnot(pdf->page, &rect, dst);
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
					HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
					retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
					retval &= pdf_out_curcolor(pdf, attr);
					rect.right = text_pos->x + LINK_BORDER_WIDTH;
					dst = HPDF_Page_CreateDestination(pdf->links[xref->dest_page]);
					annot = HPDF_Page_CreateLinkAnnot(pdf->page, &rect, dst);
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
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateLaunchAnnot(pdf->page, &rect, xref->destname, NULL, NULL);
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_REXX_SCRIPT:
		retval &= pdf_out_color(pdf->page, &viewer_colors.rxs);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateLaunchAnnot(pdf->page, &rect, xref->destname, NULL, NULL);
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		retval &= pdf_out_color(pdf->page, &viewer_colors.system);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateLaunchAnnot(pdf->page, &rect, xref->destname, NULL, NULL);
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_IMAGE:
		/* that would be an inline image; currently not supported by compiler */
		retval &= pdf_out_color(pdf->page, &viewer_colors.system);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		break;
	case HYP_NODE_QUIT:
		retval &= pdf_out_color(pdf->page, &viewer_colors.quit);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateNamedAnnot(pdf->page, &rect, "Quit");
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	case HYP_NODE_CLOSE:
		retval &= pdf_out_color(pdf->page, &viewer_colors.close);
		retval &= HPDF_Page_BeginText(pdf->page) == HPDF_NOERROR;
		retval &= HPDF_Page_MoveTextPos(pdf->page, text_pos->x, text_pos->y) == HPDF_NOERROR;
		retval &= HPDF_Page_ShowText(pdf->page, xref->text) == HPDF_NOERROR;
		HPDF_Page_GetCurrentTextPos(pdf->page, text_pos);
		retval &= HPDF_Page_EndText(pdf->page) == HPDF_NOERROR;
		retval &= pdf_out_curcolor(pdf, attr);
		rect.right = text_pos->x + LINK_BORDER_WIDTH;
		annot = HPDF_Page_CreateNamedAnnot(pdf->page, &rect, "Close");
		HPDF_LinkAnnot_SetBorderStyle(annot, 0, 0, 0);
		HPDF_LinkAnnot_SetHighlightMode(annot, HPDF_ANNOT_INVERT_BOX);
		break;
	default:
		/* hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"%s\">%s %u</a>", html_error_link_style, xref->destfilename, _("link to unknown node type"), hyp->indextable[xref->dest_page]->type); */
		break;
	}
	return retval;
}

/* ------------------------------------------------------------------------- */

static HPDF_Image convert_image(PDF *pdf, HYP_IMAGE *pic)
{
	int width = pic->pic.fd_w;
	int height = pic->pic.fd_h;
	int planes = pic->pic.fd_nplanes;
	unsigned char *src = (unsigned char *)pic->pic.fd_addr;
	HPDF_Image image;
	unsigned char *pixbuf;
	unsigned char *dst;
	int dststride;
	int srcstride;
	int x, planesize, pos;
	int i;
	PALETTE pal;
	unsigned char *plane_ptr[XMAX_PLANES];
	guint16 back[XMAX_PLANES];
	int np;
	int pixel;
	guint16 color;

	dststride = width * 3;
	
	pixbuf = g_new(unsigned char, dststride * height);
	if (pixbuf == NULL)
		return NULL;
	srcstride = ((width + 15) >> 4) << 1;
	planesize = srcstride * height;
	pic_stdpalette(pal, planes);

	dst = pixbuf;
	
	for (i = 0; i < planes; i++)
		plane_ptr[i] = &src[i * planesize];
	
	pos = 0;
	width *= 3; /* we write 3 bytes per pixel */
	for (x = 0; x < planesize; x += 2)
	{
		for (np = 0; np < planes; np++)
			back[np] = (plane_ptr[np][x] << 8) | plane_ptr[np][x + 1];
		
		for (pixel = 0; pixel < 16; pixel++)
		{
			color = 0;
			for (np = 0; np < planes; np++)
			{
				color |= ((back[np] & 0x8000) >> (15 - np));
				back[np] <<= 1;
			}
			if (pos < width)
			{
				dst[pos++] = pal[color].r;
				dst[pos++] = pal[color].g;
				dst[pos++] = pal[color].b;
			}
		}
		if (pos >= width)
		{
			pos = 0;
			dst += dststride;
		}
	}

	image = HPDF_Image_LoadRawImageFromMem(pdf->hpdf->mmgr, pixbuf, pdf->hpdf->xref, pic->pic.fd_w, height, HPDF_CS_DEVICE_RGB, 8);
	g_free(pixbuf);
	return image;
}


static HPDF_REAL draw_image(PDF *pdf, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, HPDF_REAL x, HPDF_REAL y)
{
	HYP_IMAGE *pic;
	HPDF_REAL tx, ty, tw, th;
	HPDF_Image image;
	void *orig_data;
	
	pic = (HYP_IMAGE *)AskCache(hyp, gfx->extern_node_index);

	if (pic)
	{
		/*
		 * in the GUI, this may have been already
		 * converted to a system dependant format
		 * (BITMAP or GdkPixbuf).
		 * We need the original data here.
		 */
		orig_data = NULL;
		if (!cmdline_version)
		{
			if (pic->decompressed)
			{
				orig_data = pic->pic.fd_addr;
				pic->pic.fd_addr = hyp_loaddata(hyp, gfx->extern_node_index);
				pic->decompressed = FALSE;
			}
		}

		if (gfx->x_offset == 0)
		{
			tx = (hyp->line_width * pdf->cell_width - gfx->pixwidth) / 2;
		} else
		{
			int xc;
			
			xc = gfx->x_offset - 1;
			if ((xc + (gfx->pixwidth / HYP_PIC_FONTW)) == hyp->line_width)
				tx = pdf->page_width - gfx->pixwidth;
			else
				tx = xc * pdf->cell_width;
		}
		if (tx < 0)
			tx = 0;
		
		tx += x;
		ty = y;
		tw = (gfx->pixwidth / HYP_PIC_FONTW) * pdf->cell_width;
		th = ((gfx->pixheight + HYP_PIC_FONTH - 1) / HYP_PIC_FONTH) * pdf->line_height;
		if (gfx->islimage)
		{
			y += gfx->pixheight;
			/* st-guide leaves an empty line after each @limage */
			if ((gfx->pixheight % HYP_PIC_FONTH) == 0)
				y += pdf->line_height;
		}

		if (tx >= pdf->page_width || (tx + gfx->pixwidth) <= 0)
			return y;
		if (ty >= pdf->page_height || (ty + gfx->pixheight) <= 0)
			return y;

		if (pdf->images == NULL)
		{
			pdf->images = g_new0(HPDF_Image, hyp->num_index);
			if (pdf->images == NULL)
				return y;
		}
		if (pdf->images[gfx->extern_node_index] == NULL)
		{
			if (!pic->decompressed)
			{
				if (hyp_transform_image(hyp, gfx) == FALSE)
					return y;
			}
			image = convert_image(pdf, pic);
			pdf->images[gfx->extern_node_index] = image;
			if (cmdline_version)
			{
				char *data = (char *)pic->pic.fd_addr;
				data -= SIZEOF_HYP_PICTURE;
				g_free(data);
				pic->pic.fd_addr = NULL;
			}
		} else
		{
			image = pdf->images[gfx->extern_node_index];
		}
		if (orig_data)
		{
			pic->pic.fd_addr = orig_data;
			pic->decompressed = TRUE;
		}
		if (image)
		{
			HPDF_Page_DrawImage(pdf->page, image, tx, pdf->page_height - 1 - ty - th, tw, th);
		}
	}
	return y;
}

static int adjust_for_limage(PDF *pdf, long lineno, int h)
{
	struct hyp_gfx *gfx;
	int diff = 0;
	long end = lineno + h / pdf->line_height;
	
	for (gfx = pdf->hyp_gfx; gfx != NULL; gfx = gfx->next)
	{
		if (gfx->type == HYP_ESC_PIC && gfx->islimage && gfx->y_offset >= lineno && gfx->y_offset < end)
		{
			int adj = ((gfx->pixheight + HYP_PIC_FONTH - 1) / HYP_PIC_FONTH);
			if (adj > 0)
			{
				diff += adj * (HYP_PIC_FONTH - pdf->line_height);
			}
		}
	}
	return diff;
}

/*** ---------------------------------------------------------------------- ***/

static void draw_arrows(PDF *pdf, HPDF_REAL x0, HPDF_REAL y0, HPDF_REAL x1, HPDF_REAL y1, unsigned char line_ends)
{
	/* FIXME: not yet implemented */
	(void) pdf;
	(void) x0;
	(void) y0;
	(void) x1;
	(void) y1;
	(void) line_ends;
}

/*** ---------------------------------------------------------------------- ***/

static HPDF_REAL draw_line(PDF *pdf, struct hyp_gfx *gfx, HPDF_REAL x, HPDF_REAL y)
{
	int w, h;
	HPDF_REAL ret = y;
	int diff;
	HPDF_REAL x0, y0, x1, y1;

	static HPDF_UINT16 const long_dash[] = { 12, 4 };
	static HPDF_UINT16 const dot[] = { 2, 6, 2, 6 };
	static HPDF_UINT16 const dashdot[] = { 8, 3, 2, 3 };
	static HPDF_UINT16 const dash[] = { 8, 8 };
	static HPDF_UINT16 const dashdotdot[] = { 4, 3, 2, 2, 1, 3, 1, 0 };
	static HPDF_UINT16 const userline[] = { 1, 1 };

	w = gfx->width * pdf->cell_width;
	h = gfx->height * pdf->line_height;

	if (w == 0)
	{
		w = gfx->pixwidth = 1;
	} else if (w < 0)
	{
		w = -w;
		x -= w;
	}
	if (h == 0)
	{
		h = gfx->pixheight = 1;
	} else if (h < 0)
	{
		h = -h;
		diff = adjust_for_limage(pdf, gfx->y_offset - h / pdf->line_height, h);
		h += diff;
		y -= h;
	} else
	{
		diff = adjust_for_limage(pdf, gfx->y_offset, h);
		h += diff;
	}
	gfx->pixwidth = w;
	gfx->pixheight = h;

	x = x + (gfx->x_offset - 1) * pdf->cell_width;
	
	if (gfx->width < 0)
	{
		/* draw from right to left */
		x0 = x + w - 1;
		x1 = x;
	} else
	{
		/* draw from left to right */
		x0 = x;
		x1 = x + w - 1;
	}
	if (gfx->height < 0)
	{
		/* draw from bottom to top */
		y0 = y + h - 1;
		y1 = y;
	} else
	{
		/* draw from top to bottom */
		y0 = y;
		y1 = y + h - 1;
	}

	HPDF_Page_GSave(pdf->page);
	pdf_out_color(pdf->page, &viewer_colors.text);

	switch (gfx->style)
	{
	case 0:
	case 1:
	default:
		/* solid */
		break;
	case 2: /* long dash */
		HPDF_Page_SetDash(pdf->page, long_dash, 2, 0);
		break;
	case 3: /* dot */
		HPDF_Page_SetDash(pdf->page, dot, 4, 0);
		break;
	case 4: /* dashdot */
		HPDF_Page_SetDash(pdf->page, dashdot, 4, 0);
		break;
	case 5: /* dash */
		HPDF_Page_SetDash(pdf->page, dash, 2, 0);
		break;
	case 6: /* dashdotdot */
		HPDF_Page_SetDash(pdf->page, dashdotdot, 8, 0);
		break;
	case 7: /* userline */
		/* FIXME: maybe better use gray instead? */
		HPDF_Page_SetDash(pdf->page, userline, 2, 0);
		break;
	}
	HPDF_Page_MoveTo(pdf->page, x0, pdf->page_height - 1 - y0);
	HPDF_Page_LineTo(pdf->page, x1, pdf->page_height - 1 - y1);
	HPDF_Page_Stroke(pdf->page);

	draw_arrows(pdf, x0, y0, x1, y1, gfx->begend);
	
	HPDF_Page_GRestore(pdf->page);

	return ret;
}


static void rounded_box(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL w, HPDF_REAL h)
{
	HPDF_REAL rdeltax, rdeltay;
	HPDF_REAL xrad, yrad;
	HPDF_REAL x1, y1, x2, y2;

	x1 = x;
	y1 = y;
	x2 = x1 + w;
	y2 = y1 + h;

	rdeltax = w / 2;
	rdeltay = h / 2;

	xrad = 10;
	if (xrad > rdeltax)
	    xrad = rdeltax;

	yrad = xrad;
	if (yrad > rdeltay)
	    yrad = rdeltay;

	HPDF_Page_Arc(page, x2 - xrad, y2 - yrad, xrad, yrad, 0, 90);
	HPDF_Page_Arc(page, x1 + xrad, y2 - yrad, xrad, yrad, 90, 180);
	HPDF_Page_Arc(page, x1 + xrad, y1 + yrad, xrad, yrad, 180, 270);
	HPDF_Page_Arc(page, x2 - xrad, y1 + yrad, xrad, yrad, 270, 0);
	HPDF_Page_ClosePath(page);
}


static HPDF_REAL draw_box(PDF *pdf, struct hyp_gfx *gfx, HPDF_REAL x, HPDF_REAL y)
{
	HPDF_REAL ret = y;
	HPDF_REAL w, h;
	HPDF_REAL tx, ty;
	int fillstyle;

	tx = (gfx->x_offset - 1) * pdf->cell_width;
	w = gfx->width * pdf->cell_width;
	h = gfx->height * pdf->line_height;
	h += adjust_for_limage(pdf, gfx->y_offset, h);

	gfx->pixwidth = w;
	gfx->pixheight = h;

	tx += x;
	ty = y;
	
	if (tx >= pdf->page_width || (tx + w) <= 0)
		return ret;
	if (ty >= pdf->page_height || (ty + h) <= 0)
		return ret;

	HPDF_Page_GSave(pdf->page);

	if (gfx->style != 0)
	{
		fillstyle = gfx->style;
	} else
	{
		fillstyle = IP_HOLLOW;
	}

	switch (fillstyle)
	{
	case IP_HOLLOW:
		pdf_out_stroke_color(pdf->page, &viewer_colors.text);
		break;
	case IP_1PATT:
	case IP_2PATT:
	case IP_3PATT:
	case IP_4PATT:
	case IP_5PATT:
	case IP_6PATT:
	case IP_7PATT:
		/*
		 * translate these into gray levels instead of patterns
		 */
		{
			static HPDF_REAL const gray_levels[ IP_7PATT - IP_1PATT + 1] = {
				0.875, 0.750, 0.625, 0.500, 0.375, 0.250, 0.125
			};
			HPDF_Page_SetGrayFill(pdf->page, gray_levels[fillstyle - IP_1PATT]);
			pdf_out_stroke_color(pdf->page, &viewer_colors.text);
		}
		break;
	case 8:
		pdf_out_color(pdf->page, &viewer_colors.text);
		break;
	case  9: case 10: case 11: case 12: case 13: case 14: case 15:
	case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
	case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
	case 32: case 33: case 34: case 35: case 36:
		if (pdf->patterns[fillstyle] == 0)
		{
			HPDF_Image pattern_image;
			HPDF_Array array;
			const unsigned char *data = pattern_bits + fillstyle * PATTERN_SIZE;
			unsigned char mask[PATTERN_SIZE];
			int j;
			
			for (j = 0; j < PATTERN_SIZE; j++)
				mask[j] = bitrevtab[data[j]];
			pattern_image = HPDF_Image_LoadRawImageFromMem(pdf->hpdf->mmgr, mask, pdf->hpdf->xref, 16, 16, HPDF_CS_DEVICE_GRAY, 1);
			HPDF_Image_SetMask(pattern_image, HPDF_TRUE);
			array = HPDF_Array_New(pattern_image->mmgr);
			HPDF_Dict_Add(pattern_image, "Decode", array);
			HPDF_Array_Add(array, HPDF_Number_New(pattern_image->mmgr, 1));
			HPDF_Array_Add(array, HPDF_Number_New(pattern_image->mmgr, 0));
			pdf->patterns[fillstyle] = HPDF_Pattern_New(pdf->hpdf->mmgr, pdf->hpdf->xref,
				HPDF_PATTERN_TYPE_TILED, HPDF_PAINT_TYPE_UNCOLORED, HPDF_TILING_TYPE_NO_DISTORTION, pattern_image);
		}
		if (pdf->pattern_colorspace == NULL)
			pdf->pattern_colorspace = HPDF_PatternColorspace_New(pdf->hpdf->mmgr, HPDF_CS_DEVICE_RGB);
		HPDF_Page_SetColorspaceFill(pdf->page, pdf->pattern_colorspace);
		HPDF_Page_SetPatternFill(pdf->page, pdf->patterns[fillstyle], viewer_colors.text.r, viewer_colors.text.g, viewer_colors.text.b);
		pdf_out_stroke_color(pdf->page, &viewer_colors.text);
		break;
	}

	ty = pdf->page_height - 1 - ty - h;
	if (gfx->type == HYP_ESC_BOX)
	{
		HPDF_Page_Rectangle(pdf->page, tx, ty, w, h);
	} else
	{
		rounded_box(pdf->page, tx, ty, w, h);
	}
	if (fillstyle == IP_WIN_SOLID)
		HPDF_Page_Fill(pdf->page);
	else if (fillstyle == IP_HOLLOW)
		HPDF_Page_Stroke(pdf->page);
	else
		HPDF_Page_FillStroke(pdf->page);
	
	HPDF_Page_GRestore(pdf->page);

	return ret;
}

static HPDF_REAL pdf_out_graphics(PDF *pdf, HYP_DOCUMENT *hyp, long lineno, HPDF_REAL sx, HPDF_REAL sy)
{
	HPDF_REAL max_y, y;
	struct hyp_gfx *gfx;
	
	max_y = sy;
	gfx = pdf->hyp_gfx;
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			switch (gfx->type)
			{
			case HYP_ESC_PIC:
				y = draw_image(pdf, hyp, gfx, sx, sy);
				break;
			case HYP_ESC_LINE:
				y = draw_line(pdf, gfx, sx, sy);
				break;
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				y = draw_box(pdf, gfx, sx, sy);
				break;
			default:
				y = sy;
				break;
			}
			if (y > max_y)
				max_y = y;
		}
		gfx = gfx->next;
	}
	return max_y;
}

/* ------------------------------------------------------------------------- */

PDF *pdf_new(hcp_opts *opts)
{
	PDF *pdf;
	const char *encoding = NULL;
	HPDF_TextWidth tw;
	int i;
	
	pdf = g_new(PDF, 1);
	if (pdf == NULL)
		return pdf;
	pdf->opts = opts;
	pdf->hpdf = HPDF_New(pdf_error_handler, 0, 0, 0, pdf);
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
	case HYP_CHARSET_CP1251:
	case HYP_CHARSET_ATARI_RU:
		encoding = HPDF_ENCODING_CP1251;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1252:
		encoding = HPDF_ENCODING_CP1252;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1253:
		encoding = HPDF_ENCODING_CP1253;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1254:
		encoding = HPDF_ENCODING_CP1254;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1255:
		encoding = HPDF_ENCODING_CP1255;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1256:
		encoding = HPDF_ENCODING_CP1256;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1257:
		encoding = HPDF_ENCODING_CP1257;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_CP1258:
		encoding = HPDF_ENCODING_CP1258;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_LATIN1:
		encoding = HPDF_ENCODING_STANDARD;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_LATIN2:
		encoding = HPDF_ENCODING_ISO8859_2;
		HPDF_SetCurrentEncoder(pdf->hpdf, encoding);
		break;
	case HYP_CHARSET_NONE:
	case HYP_CHARSET_BINARY:
	case HYP_CHARSET_BINARY_TABS:
	case HYP_CHARSET_ATARI:
	case HYP_CHARSET_CP850:
		break;
	}
	HPDF_SetCompressionMode(pdf->hpdf, opts->compression ? HPDF_COMP_ALL : HPDF_COMP_NONE);
	pdf->regular_font = HPDF_GetFont(pdf->hpdf, "Courier", encoding);
	pdf->bold_font = HPDF_GetFont(pdf->hpdf, "Courier-Bold", encoding);
	pdf->italic_font = HPDF_GetFont(pdf->hpdf, "Courier-Oblique", encoding);
	pdf->bold_italic_font = HPDF_GetFont(pdf->hpdf, "Courier-BoldOblique", encoding);
	pdf->font_size = 12.0;
	pdf->ascent = HPDF_Font_GetAscent(pdf->regular_font) * pdf->font_size / 1000;
	pdf->descent = HPDF_Font_GetDescent(pdf->regular_font) * pdf->font_size / 1000;
	pdf->line_height = pdf->ascent - pdf->descent;
	HPDF_Font_TextWidth(pdf->regular_font, (const HPDF_BYTE *) "m", 1, &tw);
	pdf->cell_width = tw.width * pdf->font_size / 1000;
	/* HPDF_SetViewerPreference(pdf->hpdf, HPDF_DISPLAY_DOC_TITLE); */

	pdf->pattern_colorspace = NULL;
	for (i = 0; i < NUM_PATTERNS; i++)
		pdf->patterns[i] = NULL;

	pdf->images = NULL;

	return pdf;
}
	
/* ------------------------------------------------------------------------- */

void pdf_delete(PDF *pdf)
{
	if (pdf == NULL)
		return;

#if 0
	if (pdf->pattern_colorspace)
		HPDF_Array_Free(pdf->pattern_colorspace);

	/* not needed; they are part of the xref */
	{
		int i;
	
		for (i = 0; i < NUM_PATTERNS; i++)
			if (pdf->patterns[i])
				HPDF_Dict_Free(pdf->patterns[i]);
	}
#endif

	HPDF_Free(pdf->hpdf);
	g_free(pdf->images);
	g_free(pdf->links);
	g_free(pdf->pages);
	g_free(pdf);
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_attr(PDF *pdf, struct textattr *attr, gboolean dryrun)
{
	gboolean retval = TRUE;
	
	if (attr->curattr != attr->newattr)
	{
		attr->curattr = attr->newattr;
		if (!dryrun)
			pdf_out_curattr(pdf, attr);
	}

	if (attr->curfg != attr->newfg)
	{
		attr->curfg = attr->newfg;
		if (!dryrun)
			pdf_out_curcolor(pdf, attr);
	}

	if (attr->curbg != attr->newbg)
	{
		attr->curbg = attr->newbg;
		if (!dryrun)
		{
		}
	}

	return retval;
}

/* ------------------------------------------------------------------------- */

static HPDF_Page pdf_newpage(PDF *pdf, hyp_nodenr node)
{
	HPDF_Page page;
	
	pdf->pages = g_renew(struct pdf_page, pdf->pages, pdf->curr_page_num + 1);
	if (pdf->pages == NULL)
		return NULL;
	page = HPDF_AddPage(pdf->hpdf);
	pdf->pages[pdf->curr_page_num].page = page;
	pdf->curr_page_num++;
	if (pdf->links[node] == NULL)
		pdf->links[node] = page;
	pdf_out_color(page, &viewer_colors.text);
	HPDF_Page_SetFontAndSize(page, pdf->regular_font, pdf->font_size); 
	HPDF_Page_SetTextRenderingMode(page, HPDF_FILL);
	HPDF_Page_SetLineWidth(page, 0);
	pdf->page_width = HPDF_Page_GetWidth(page);
	pdf->page_height = HPDF_Page_GetHeight(page);

	return page;
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_node(PDF *pdf, HYP_DOCUMENT *hyp, hyp_nodenr node, symtab_entry *syms, gboolean dryrun)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	struct textattr attr;
	long lineno;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	gboolean converror = FALSE;
	gboolean in_text_out = FALSE;
	HPDF_Point text_pos;
	HPDF_REAL bottom_margin = 0;
	HPDF_REAL sx, sy, dy;

#define BEGINTEXT() \
	if (!in_text_out) \
	{ \
		if (!dryrun) \
		{ \
			if (lineno != 0 && text_pos.y < bottom_margin) \
			{ \
				pdf->page = pdf->pages[pdf->curr_page_num++].page; \
				text_pos.y = pdf->page_height - pdf->line_height - text_yoffset; \
				sy = text_yoffset; \
			} \
		} else \
		{ \
			if (lineno != 0 && text_pos.y < bottom_margin) \
			{ \
				pdf->page = pdf_newpage(pdf, node); \
				text_pos.y = pdf->page_height - pdf->line_height - text_yoffset; \
				sy = text_yoffset; \
			} \
		} \
		in_text_out = TRUE; \
	}
#define ENDTEXT() \
	if (in_text_out) \
	{ \
		in_text_out = FALSE; \
	}
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		BEGINTEXT(); \
		if (!dryrun) { \
			if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, &user_colors[attr.curfg]); \
			retval &= pdf_out_str(pdf, hyp, &text_pos, textstart, src - textstart, &converror, &attr); \
			if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, &viewer_colors.text); \
		} \
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
		/* hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "end tree %d -->\n", in_tree); */ \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	if (dryrun)
	{
		pdf->page = pdf_newpage(pdf, node);
	} else
	{
		pdf->page = pdf->pages[pdf->curr_page_num++].page;
	}

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
			pdf->hyp_gfx = NULL;
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
						
						last = &pdf->hyp_gfx;
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
				
				for (gfx1 = pdf->hyp_gfx; gfx1 != NULL; gfx1 = gfx1->next)
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
			 * there seems to be no way to change the page/window title
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
			sx = text_xoffset;
			sy = text_yoffset;
			text_pos.x = text_xoffset;
			text_pos.y = pdf->page_height - pdf->line_height - text_yoffset;
			if (!dryrun)
			{
				retval &= pdf_out_labels(pdf, hyp, entry, lineno, syms, &converror);
				dy = pdf_out_graphics(pdf, hyp, lineno, sx, sy) - sy;
				sy += dy;
				text_pos.y -= dy;
			}
				
			while (retval && src < end)
			{
				if (*src == HYP_ESC)
				{
					DUMPTEXT();
					ENDTEXT();
					src++;
					switch (*src)
					{
					case HYP_ESC_ESC:
						FLUSHTREE();
						if (!dryrun)
						{
							if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, &user_colors[attr.curfg]);
							retval &= pdf_out_str(pdf, hyp, &text_pos, (const unsigned char *)"\033", 1, &converror, &attr);
							if (attr.curfg != HYP_DEFAULT_FG) retval &= pdf_out_color(pdf->page, &viewer_colors.text);
						}
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
							
							if (!dryrun)
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
								/* hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "<!-- begin tree %d\n", tree); */
								in_tree = tree;
							}
							/* hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "   %d \"%s\" %u\n", obj, str, line); */
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
						retval &= pdf_out_attr(pdf, &attr, dryrun);
						src++;
						break;
					
					case HYP_ESC_FG_COLOR:
						FLUSHTREE();
						src++;
						attr.newfg = *src;
						retval &= pdf_out_attr(pdf, &attr, dryrun);
						src++;
						break;
				
					case HYP_ESC_BG_COLOR:
						FLUSHTREE();
						src++;
						attr.newbg = *src;
						retval &= pdf_out_attr(pdf, &attr, dryrun);
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
					text_pos.y -= pdf->line_height;
					sy += pdf->line_height;
					if (!dryrun)
					{
						retval &= pdf_out_labels(pdf, hyp, entry, lineno, syms, &converror);
						dy = pdf_out_graphics(pdf, hyp, lineno, sx, sy) - sy;
						sy += dy;
						text_pos.y -= dy;
					}
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
			retval &= pdf_out_attr(pdf, &attr, dryrun);
			at_bol = FALSE;
			FLUSHLINE();
			FLUSHTREE();
			++lineno;
			text_pos.x = text_xoffset;
			text_pos.y -= pdf->line_height;
			sy += pdf->line_height;
			if (!dryrun)
			{
				retval &= pdf_out_labels(pdf, hyp, entry, lineno, syms, &converror);
				dy = pdf_out_graphics(pdf, hyp, lineno, sx, sy) - sy;
				sy += dy;
				text_pos.y -= dy;
			}
			
			if (pdf->hyp_gfx != NULL)
			{
				struct hyp_gfx *gfx, *next;
				
				for (gfx = pdf->hyp_gfx; gfx != NULL; gfx = next)
				{
					if (!gfx->used)
					{
						/* hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<!-- gfx unused: "); */
						/* pdf_out_gfx(pdf, hyp, gfx); */
						/* hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "-->\n"); */
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
		if (!dryrun)
		{
			char *text = g_strdup_printf(_("%s: Node %u: failed to decode"), hyp->file, node);
			HPDF_REAL tw = HPDF_Page_TextWidth(pdf->page, text);
			HPDF_Page_BeginText(pdf->page);
			HPDF_Page_TextOut(pdf->page, (HPDF_Page_GetWidth(pdf->page) - tw) / 2, HPDF_Page_GetHeight(pdf->page) - pdf->line_height - 50, text);
			HPDF_Page_EndText(pdf->page);
			g_free(text);
		}
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
#if defined(__WIN32__) || defined(_WIN32)
	gmtime_r(&t, &tm);
	return g_strdup_printf("D:%04d%02d%02d%02d%02d%02d",
		tm.tm_year + 1900,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec);
#else
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
#endif
}

/* ------------------------------------------------------------------------- */

static void pdf_out_globals(HYP_DOCUMENT *hyp, PDF *pdf)
{
	char *str;
	struct stat s;
	time_t now;

#define STR(t, x) \
	if (x != NULL) \
	{ \
		HPDF_SetInfoAttr(pdf->hpdf, t, x); \
	}

	STR(HPDF_INFO_TITLE, hyp->database);
	STR(HPDF_INFO_AUTHOR, hyp->author);
	STR(HPDF_INFO_SUBJECT, hyp->subject);

	HPDF_SetInfoAttr(pdf->hpdf, HPDF_INFO_CREATOR, gl_program_name);
	str = g_strdup_printf("%s %s for %s, using Haru Free PDF Library %s", gl_program_name, gl_program_version, hyp_osname(hyp_get_current_os()), HPDF_GetVersion());
	STR(HPDF_INFO_PRODUCER, str);
	g_free(str);
	time(&now);
	str = pdf_datestr(now);
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

static gboolean pdf_print_nodes(PDF *pdf, HYP_DOCUMENT *hyp, hcp_opts *opts, symtab_entry *syms, int argc, const char **argv, gboolean dryrun)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret = TRUE;
	gboolean found;
	int i;
	
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

		if (node == hyp->index_page)
		{
			if (!opts->gen_index || argc != 0)
				found = FALSE;
		}
		if (!found)
			continue;

		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= pdf_out_node(pdf, hyp, node, syms, dryrun);
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

	return ret;
}

/* ------------------------------------------------------------------------- */

gboolean recompile_pdf(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	volatile gboolean ret;
	int i;
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

	pdf->pages = NULL;
	pdf->num_pages = 0;
	pdf->curr_page_num = 0;
	pdf->links = g_new0(HPDF_Page, hyp->num_index);
	
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
			lowercase_image_name(hyp, node);
			break;
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

	/*
	 * we have to go through the whole document twice,
	 * because we have to know the PDF target page
	 * for links to work.
	 */
	ret &= pdf_print_nodes(pdf, hyp, opts, syms, argc, argv, TRUE);
	pdf->num_pages = pdf->curr_page_num;
	pdf->curr_page_num = 0;
	ret &= pdf_print_nodes(pdf, hyp, opts, syms, argc, argv, FALSE);
	ASSERT(pdf->num_pages == pdf->curr_page_num);

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
	
	if (opts->verbose >= 0 && opts->outfile != stdout)
	{
		hyp_utf8_fprintf(stdout, _("generated %lu page%s\n"), (unsigned long)pdf->num_pages, pdf->num_pages == 1 ? "" : "s");
	}

	pdf_delete(pdf);

	return ret;
}

#else

extern int _I_dont_care_that_ISO_C_forbids_an_empty_source_file_;

#endif /* WITH_PDF */
