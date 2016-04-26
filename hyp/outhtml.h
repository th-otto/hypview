#include "base64.h"
#include "pattern.h"

#define HTML_DOCTYPE_OLD              0           /* HTML 3.2 */
#define HTML_DOCTYPE_STRICT           1           /* HTML 4.01 */
#define HTML_DOCTYPE_TRANS            2           /* HTML 4.01 Transitional */
#define HTML_DOCTYPE_FRAME            3           /* HTML 4.01 Frameset */
#define HTML_DOCTYPE_XSTRICT          4           /* XHTML 1.0 Strict */
#define HTML_DOCTYPE_XTRANS           5           /* XHTML 1.0 Transitional */
#define HTML_DOCTYPE_XFRAME           6           /* XHTML 1.0 Frameset */
#define HTML_DOCTYPE_HTML5            7           /* HTML 5 */
 
static char const html_nav_back_png[] = "images/iback.png";
static char const html_nav_prev_png[] = "images/iprev.png";
static char const html_nav_toc_png[] = "images/itoc.png";
static char const html_nav_next_png[] = "images/inext.png";
static char const html_nav_xref_png[] = "images/ixref.png";
static char const html_nav_load_png[] = "images/iload.png";
static char const html_nav_index_png[] = "images/iindex.png";
static char const html_nav_help_png[] = "images/ihelp.png";
static char const html_nav_info_png[] = "images/iinfo.png";
static char const html_hyp_info_id[] = "hyp_info";
static char const html_nav_dimensions[] = " width=\"32\" height=\"21\"";

static char const html_referer_url[] = "";
static char const html_nav_load_href[] = "../libhyp/";
static char const hypview_css_name[] = "_hypview.css";
static char const hypview_js_name[] = "_hypview.js";
static char const html_view_rsc_href[] = "rscview.cgi";
static int html_doctype = HTML_DOCTYPE_XSTRICT;

static const char *html_closer = " />";
static const char *html_name_attr = "id";

/*
 * style names used
 */
static char const html_attr_bold_style[] = "hypview_attr_bold";
static char const html_attr_light_style[] = "hypview_attr_light";
static char const html_attr_italic_style[] = "hypview_attr_italic";
static char const html_attr_underlined_style[] = "hypview_attr_underlined";
static char const html_attr_outlined_style[] = "hypview_attr_outlined";
static char const html_attr_shadowed_style[] = "hypview_attr_shadowed";
static char const html_toolbar_style[] = "hypview_nav_toolbar";
static char const html_nav_img_style[] = "hypview_nav_img";
static char const html_node_style[] = "hypview_node";
static char const html_pnode_style[] = "hypview_pnode";
static char const html_dropdown_pnode_style[] = "hypview_dropdown_pnode";
static char const html_dropdown_info_style[] = "hypview_dropdown_info";
static char const html_rx_link_style[] = "hypview_rx";
static char const html_rxs_link_style[] = "hypview_rxs";
static char const html_system_link_style[] = "hypview_system";
static char const html_image_link_style[] = "hypview_image";
static char const html_close_link_style[] = "hypview_close";
static char const html_quit_link_style[] = "hypview_quit";
static char const html_error_link_style[] = "hypview_error";
static char const html_xref_link_style[] = "hypview_xref";
static char const html_popup_link_style[] = "hypview_popup";
static char const html_graphics_style[] = "hypview_graphics";
static char const html_dropdown_style[] = "hypview_dropdown";


struct html_xref {
	char *str;	/* referenced name */
	char *text;	/* text to display */
	struct html_xref *next;
};

#define HTML_DEFAULT_PIC_TYPE HYP_PIC_GIF

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *html_quote_name(const char *name, gboolean convslash)
{
	char *str, *ret;
	size_t len;
	
	if (name == NULL)
		return NULL;
	len = strlen(name);
	str = ret = g_new(char, len * 6 + 1);
	if (str != NULL)
	{
		while (*name)
		{
			char c = *name++;
			switch (c)
			{
			case '\\':
				if (convslash)
				{
					*str++ = '/';
				} else
				{
					*str++ = '\\';
				}
				break;
			case '"':
				strcpy(str, "&quot;");
				str += 6;
				break;
			case '&':
				strcpy(str, "&amp;");
				str += 5;
				break;
			case '\'':
				strcpy(str, "&apos;");
				str += 6;
				break;
			case '<':
				strcpy(str, "&lt;");
				str += 4;
				break;
			case '>':
				strcpy(str, "&gt;");
				str += 4;
				break;
			default:
				*str++ = c;
				break;
			}
		}
		*str++ = '\0';
		ret = g_renew(char, ret, str - ret);
	}
	return ret;
}

/* ------------------------------------------------------------------------- */

static char *html_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *p;
	char *buf;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	buf = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
	p = html_quote_name(buf, entry->type == HYP_NODE_EXTERNAL_REF);
	g_free(buf);
	return p;
}

/* ------------------------------------------------------------------------- */

static void html_out_globals(HYP_DOCUMENT *hyp, FILE *outfile)
{
	char *str;

	hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @os %s -->%s", hyp_osname(hyp->comp_os), stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @charset %s -->%s", hyp_charset_name(hyp->comp_charset), stg_nl);
	
	if (hyp->database != NULL)
	{
		str = html_quote_name(hyp->database, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @database \"%s\" -->%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			str = html_quote_name(h->name, FALSE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @hostname \"%s\" -->%s", str, stg_nl);
			g_free(str);
		}
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = html_quote_nodename(hyp, hyp->default_page);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @default \"%s\" -->%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		str = html_quote_name(hyp->hcp_options, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @options \"%s\" -->%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->author != NULL)
	{
		str = html_quote_name(hyp->author, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @author \"%s\" -->%s", str, stg_nl);
		g_free(str);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = html_quote_nodename(hyp, hyp->help_page);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @help \"%s\" -->%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		str = html_quote_name(hyp->version, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @$VER: %s -->%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->subject != NULL)
	{
		str = html_quote_name(hyp->subject, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @subject \"%s\" -->%s", str, stg_nl);
		g_free(str);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- @width %d -->%s", hyp->line_width, stg_nl);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, _("<!-- ST-Guide flags: $%04x -->%s"), hyp->st_guide_flags, stg_nl);
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_str(FILE *outfile, HYP_CHARSET charset, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(charset, output_charset, str, len, &converror);
	p = html_quote_name(dst, FALSE);
	fputs(p, outfile);
	g_free(p);
	g_free(dst);
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_attr(FILE *outfile, unsigned char oldattr, unsigned char newattr)
{
	if (oldattr != newattr)
	{
#define on(mask, style) \
		if (!(oldattr & mask) && (newattr & mask)) \
			fprintf(outfile, "<span class=\"%s\">", style)
#define off(mask, str) \
		if ((oldattr & mask) && !(newattr & mask)) \
			fputs(str, outfile)
		on(HYP_TXT_BOLD, html_attr_bold_style);
		on(HYP_TXT_LIGHT, html_attr_light_style);
		on(HYP_TXT_ITALIC, html_attr_italic_style);
		on(HYP_TXT_UNDERLINED, html_attr_underlined_style);
		on(HYP_TXT_OUTLINED, html_attr_outlined_style);
		on(HYP_TXT_SHADOWED, html_attr_shadowed_style);
		off(HYP_TXT_SHADOWED, "</span>");
		off(HYP_TXT_OUTLINED, "</span>");
		off(HYP_TXT_UNDERLINED, "</span>");
		off(HYP_TXT_ITALIC, "</span>");
		off(HYP_TXT_LIGHT, "</span>");
		off(HYP_TXT_BOLD, "</span>");
#undef on
#undef off
		return TRUE;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

/* uses image_name() from outstg.h */

/* ------------------------------------------------------------------------- */

/* uses format_from_pic() from outstg.h */

/* ------------------------------------------------------------------------- */

static void html_out_gfx(hcp_opts *opts, FILE *outfile, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, int *gfx_id)
{
	char *id;
	
	switch (gfx->type)
	{
	case HYP_ESC_PIC:
		{
			char *fname;
			char *quoted;
			if (!hypnode_valid(hyp, gfx->extern_node_index))
				fname = hyp_invalid_page(gfx->extern_node_index);
			else if (hyp->indextable[gfx->extern_node_index]->type != HYP_NODE_IMAGE)
				fname = g_strdup_printf(_("<non-image node #%u>"), gfx->extern_node_index);
			else
				fname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix);
			quoted = html_quote_name(fname, TRUE);
			if (gfx->islimage)
			{
				hyp_utf8_fprintf_charset(outfile, output_charset, "<div align=\"center\"><img src=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0\"%s</div>%s",
					quoted,
					gfx->pixwidth,
					gfx->pixheight,
					html_closer,
					stg_nl);
			} else
			{
				hyp_utf8_fprintf_charset(outfile, output_charset, "<div style=\"position:fixed; left:%dch;\"><img src=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0\"%s</div>%s",
					gfx->x_offset > 0 ? gfx->x_offset - 1 : 0,
					quoted,
					gfx->pixwidth,
					gfx->pixheight,
					html_closer,
					stg_nl);
			}
			g_free(quoted);
			g_free(fname);
		}
		break;
	case HYP_ESC_LINE:
		++(*gfx_id);
		id = g_strdup_printf("hypview_gfx_%d", *gfx_id);
		fprintf(outfile,
			"<canvas class=\"%s\" id=\"%s\">"
			"<script type=\"text/javascript\">drawLine('%s', %d, %d, %d, %d, %d, %d);</script>"
			"</canvas>",
			html_graphics_style,
			id,
			id,
			gfx->x_offset,
			gfx->width, gfx->height,
			gfx->begend & (1 << 0), gfx->begend & (1 << 1),
			gfx->style);
		g_free(id);
		break;
	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		++(*gfx_id);
		id = g_strdup_printf("hypview_gfx_%d", *gfx_id);
		fprintf(outfile,
			"<canvas class=\"%s\" id=\"%s\">"
			"<script type=\"text/javascript\">drawBox('%s', %d, %d, %d, %d, %d);</script>"
			"</canvas>",
			html_graphics_style,
			id,
			id,
			gfx->x_offset,
			gfx->width, gfx->height,
			gfx->style,
			gfx->type == HYP_ESC_BOX ? 0 : 1);
		g_free(id);
		break;
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_graphics(hcp_opts *opts, FILE *outfile, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, long lineno, int *gfx_id)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			html_out_gfx(opts, outfile, hyp, gfx, gfx_id);
		}
		gfx = gfx->next;
	}
}

/* ------------------------------------------------------------------------- */

/* uses sym_find() from outstg.h */

/* ------------------------------------------------------------------------- */

static void html_out_labels(FILE *outfile, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_LABELNAME);
	while (sym)
	{
		if (sym->lineno == lineno)
		{
			char *str = html_quote_name(sym->name, FALSE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- lineno %u --><a %s=\"%s\"></a>", sym->lineno, html_name_attr, str);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void html_out_alias(FILE *outfile, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = html_quote_name(sym->name, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<a %s=\"%s\"></a>", html_name_attr, str);
		g_free(str);
		sym->referenced = TRUE;
		sym = sym_find(sym->next, nodename, REF_ALIASNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static const char *html_basename(const char *name)
{
	const char *base = hyp_basename(name);
	if (base == NULL)
		base = "stdout";
	return base;
}

/* ------------------------------------------------------------------------- */

/*
 * Disallow certain characters that might clash with
 * the filesystem or uri escape sequences, and also any non-ascii characters.
 * For simplicity, this is done in-place.
 */
static void html_convert_filename(char *filename)
{
	char *p = filename;
	unsigned char c;
	
	while ((c = *p) != '\0')
	{
		if (c == ' ' ||
			c == ':' ||
			c == '%' ||
			c == '?' ||
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

/*
 * TODO: check wether long filenames are supported
 */
static char *html_filename_for_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, gboolean quote)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *filename;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	if (node == 0)
	{
		filename = g_strdup(html_basename(opts->output_filename));
	} else
	{
		char *name = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
		if (entry->type == HYP_NODE_EXTERNAL_REF)
		{
			filename = name;
		} else
		{
			filename = g_strconcat(name, ".html", NULL);
			g_free(name);
		}
	}
	
	/*
	 * don't do conversion for external entries,
	 * as they contain arguments to commands like "rx"
	 * and nodenames of external files
	 */
	if (entry->type != HYP_NODE_EXTERNAL_REF)
	{
		html_convert_filename(filename);
	}
	
	/*
	 * the default name for the index page is "Index";
	 * do not use that for html, as it might clash
	 * with "index.html".
	 */
	if (g_ascii_strcasecmp(filename, "index.html") == 0 && node != 0)
	{
		g_free(filename);
		filename = g_strdup_printf("index_%s", html_basename(opts->output_filename));
	}
	if (quote)
	{
		char *p = html_quote_name(filename, entry->type == HYP_NODE_EXTERNAL_REF);
		g_free(filename);
		filename = p;
	}
	return filename;	
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_stylesheet(hcp_opts *opts, FILE *outfile, gboolean do_inline)
{
	FILE *out;
	
	if (do_inline)
	{
		out = outfile;
		fprintf(out, "<style type=\"text/css\">%s", stg_nl);
	} else
	{
		char *fname;
		int exists;
		struct stat st;
		
		if (html_doctype >= HTML_DOCTYPE_XSTRICT)
			fprintf(outfile, "<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\"%s%s", hypview_css_name, html_closer, stg_nl);
		else
			fprintf(outfile, "<style type=\"text/css\" href=\"%s\"></style>%s", hypview_css_name, stg_nl);
		fname = g_build_filename(opts->output_dir, hypview_css_name, NULL);
		exists = hyp_utf8_stat(fname, &st) == 0;
		if (exists)
		{
			g_free(fname);
			return TRUE;
		}
		out = hyp_utf8_fopen(fname, "wb");
		if (out == NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, fname, hyp_utf8_strerror(errno));
			g_free(fname);
			return FALSE;
		}
		if (opts->verbose >= 2 && outfile != stdout)
			hyp_utf8_fprintf(stdout, _("writing %s\n"), hypview_css_name);
		g_free(fname);
	}
	
	fprintf(out, "body {%s", stg_nl);
	fprintf(out, "  background-color: %s;%s", gl_profile.colors.background, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.text, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to internal nodes */%s", stg_nl);
	fprintf(out, "a:link, a:visited {%s", stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.link_effect & HYP_TXT_OUTLINED ? gl_profile.colors.background : gl_profile.colors.link, stg_nl);
	if (gl_profile.colors.link_effect & HYP_TXT_UNDERLINED)
		fprintf(out, "  text-decoration: underline;%s", stg_nl);
#if 0
	/* The text-outline property is not supported in any of the major browsers. sigh. */
	if (gl_profile.colors.link_effect & HYP_TXT_OUTLINED)
		fprintf(out, "  text-outline: 2px 2px %s;%s", gl_profile.colors.link, stg_nl);
#else
	if (gl_profile.colors.link_effect & HYP_TXT_OUTLINED)
		fprintf(out, "  text-shadow: -1px -1px 0 %s, 1px -1px 0 %s, -1px 1px 0 %s, 1px 1px 0 %s;%s",
			gl_profile.colors.link, gl_profile.colors.link, gl_profile.colors.link, gl_profile.colors.link,
			stg_nl);
#endif
	if (gl_profile.colors.link_effect & HYP_TXT_ITALIC)
		fprintf(out, "  font-style: italic;%s", stg_nl);
	if (gl_profile.colors.link_effect & HYP_TXT_BOLD)
		fprintf(out, "  font-weight: bold;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to popup nodes */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_popup_link_style, html_popup_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.popup, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to external pages */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_xref_link_style, html_xref_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.xref, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to system commands */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_system_link_style, html_system_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.system, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to rx commands */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_rx_link_style, html_rx_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.rx, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to rxs commands */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_rxs_link_style, html_rxs_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.rxs, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to 'quit' */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_quit_link_style, html_quit_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.quit, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to 'close' */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_close_link_style, html_close_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.close, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display links to illegal pages */%s", stg_nl);
	fprintf(out, "a:link.%s, a:visited.%s {%s", html_error_link_style, html_error_link_style, stg_nl);
	fprintf(out, "  color: %s;%s", "#ff0000", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display outlined text @{O} */%s", stg_nl);
	fprintf(out, ".%s {%s", html_attr_outlined_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.background, stg_nl);
		fprintf(out, "  text-shadow: -1px -1px 0 %s, 1px -1px 0 %s, -1px 1px 0 %s, 1px 1px 0 %s;%s",
			gl_profile.colors.text, gl_profile.colors.text, gl_profile.colors.text, gl_profile.colors.text,
			stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display italic text @{I} */%s", stg_nl);
	fprintf(out, ".%s {%s", html_attr_italic_style, stg_nl);
	fprintf(out, "  font-style: italic;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display bold text @{B} */%s", stg_nl);
	fprintf(out, ".%s {%s", html_attr_bold_style, stg_nl);
	fprintf(out, "  font-weight: bold;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display underlined text @{U} */%s", stg_nl);
	fprintf(out, ".%s {%s", html_attr_underlined_style, stg_nl);
	fprintf(out, "  text-decoration: underline;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display shadowed text @{S} */%s", stg_nl);
	fprintf(out, ".%s {%s", html_attr_shadowed_style, stg_nl);
		fprintf(out, "  text-shadow: 1px 1px 0 %s;%s",
			gl_profile.colors.text,
			stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used to display ghosted text @{G} */%s", stg_nl);
	fprintf(out, ".%s {%s", html_attr_light_style, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.ghosted, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "ul {%s", stg_nl);
	fprintf(out, "  list-style-type: none;%s", stg_nl);
	fprintf(out, "  margin: 0;%s", stg_nl);
	fprintf(out, "  padding: 0;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for the navigation toolbar */%s", stg_nl);
	fprintf(out, ".%s {%s", html_toolbar_style, stg_nl);
	fprintf(out, "  position:fixed;%s", stg_nl);
	fprintf(out, "  top:0;%s", stg_nl);
	fprintf(out, "  width:100%%;%s", stg_nl);
	fprintf(out, "  height:28px;%s", stg_nl);
	fprintf(out, "  overflow:hidden;%s", stg_nl);
	fprintf(out, "  z-index:2;%s", stg_nl);
	fprintf(out, "  margin:0;%s", stg_nl);
	fprintf(out, "  padding:0;%s", stg_nl);
	fprintf(out, "  background-color: %s;%s", gl_profile.colors.background, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.text, stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for images in the navigation toolbar */%s", stg_nl);
	fprintf(out, ".%s {%s", html_nav_img_style, stg_nl);
	fprintf(out, "  display:block;%s", stg_nl);
	fprintf(out, "  border:0;%s", stg_nl);
	fprintf(out, "  width:40px;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for disabled images in the navigation toolbar */%s", stg_nl);
	fprintf(out, ".%s_disabled {%s", html_nav_img_style, stg_nl);
	fprintf(out, "  display:block;%s", stg_nl);
	fprintf(out, "  border:0;%s", stg_nl);
	fprintf(out, "  width:40px;%s", stg_nl);
	fprintf(out, "  opacity: 0.4;%s", stg_nl);
	fprintf(out, "  /* filter: alpha(opacity=40); For IE8 and earlier */%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "li {%s", stg_nl);
	fprintf(out, "  float: left;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for the body of regular nodes */%s", stg_nl);
	fprintf(out, ".%s {%s", html_node_style, stg_nl);
	fprintf(out, "  position:absolute;%s", stg_nl);
	fprintf(out, "  top:32px;%s", stg_nl);
	fprintf(out, "  z-index:1;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for the body of popup nodes */%s", stg_nl);
	fprintf(out, ".%s {%s", html_pnode_style, stg_nl);
	fprintf(out, "  background-color: %s;%s", gl_profile.colors.background, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.text, stg_nl);
	fprintf(out, "  z-index:2;%s", stg_nl);
	fprintf(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for the body of popup nodes */%s", stg_nl);
	fprintf(out, ".%s {%s", html_dropdown_pnode_style, stg_nl);
	fprintf(out, "  background-color: %s;%s", gl_profile.colors.background, stg_nl);
	fprintf(out, "  color: %s;%s", gl_profile.colors.text, stg_nl);
	fprintf(out, "  z-index:2;%s", stg_nl);
	fprintf(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);%s", stg_nl);
	fprintf(out, "  border: solid 1px;%s", stg_nl);
	fprintf(out, "  display:none;%s", stg_nl);
	fprintf(out, "  position:relative;%s", stg_nl);
	fprintf(out, "  padding: 3px 6px;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for the body of the file info */%s", stg_nl);
	fprintf(out, ".%s {%s", html_dropdown_info_style, stg_nl);
	fprintf(out, "  background-color: #ffffff;%s", stg_nl);
	fprintf(out, "  color: #000000;%s", stg_nl);
	fprintf(out, "  z-index:2;%s", stg_nl);
	fprintf(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);%s", stg_nl);
	fprintf(out, "  border: solid 1px;%s", stg_nl);
	fprintf(out, "  display:none;%s", stg_nl);
	fprintf(out, "  position:fixed;%s", stg_nl);
	fprintf(out, "  top: 34px;%s", stg_nl);
	fprintf(out, "  padding: 3px 6px;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	fprintf(out, "/* style used for the outer element of popups */%s", stg_nl);
	fprintf(out, ".%s {%s", html_dropdown_style, stg_nl);
	fprintf(out, "  position:relative;%s", stg_nl);
	fprintf(out, "  display:inline;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, ".%s:hover .%s {%s", html_dropdown_style, html_dropdown_pnode_style, stg_nl);
	fprintf(out, "  display:block;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	
	fprintf(out, "/* style used for @line and @box elements */%s", stg_nl);
	fprintf(out, ".%s {%s", html_graphics_style, stg_nl);
	fprintf(out, "  margin: 0;%s", stg_nl);
	fprintf(out, "  position: absolute;%s", stg_nl);
	fprintf(out, "  display: inline;%s", stg_nl);
	fprintf(out, "  z-index:1;%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	if (do_inline)
	{
		fprintf(out, "</style>%s", stg_nl);
	}
	
	if (out != outfile)
		hyp_utf8_fclose(out);
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_javascript(hcp_opts *opts, FILE *outfile, gboolean do_inline)
{
	FILE *out;
	const char *charset = hyp_charset_name(HYP_CHARSET_UTF8);
	
	if (do_inline)
	{
		out = outfile;
		fprintf(out, "<script type=\"text/javascript\">%s", stg_nl);
		if (html_doctype >= HTML_DOCTYPE_XSTRICT)
			fprintf(out, "//<![CDATA[%s", stg_nl);
	} else
	{
		char *fname;
		int exists;
		struct stat st;
		
		hyp_utf8_fprintf_charset(outfile, output_charset, "<script type=\"text/javascript\" src=\"%s\" charset=\"%s\"></script>%s", hypview_js_name, charset, stg_nl);
		fname = g_build_filename(opts->output_dir, hypview_js_name, NULL);
		exists = hyp_utf8_stat(fname, &st) == 0;
		if (exists)
		{
			g_free(fname);
			return TRUE;
		}
		out = hyp_utf8_fopen(fname, "wb");
		if (out == NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, fname, hyp_utf8_strerror(errno));
			g_free(fname);
			return FALSE;
		}
		if (opts->verbose >= 2 && outfile != stdout)
			hyp_utf8_fprintf(stdout, _("writing %s\n"), hypview_js_name);
		g_free(fname);
	}
	
	fprintf(out, "\"use strict;\"%s", stg_nl);

	fprintf(out, "var patterns = [%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAADEDgAAxA4AAAAAAAAAAAAAAAAAAP///wB3dwAA//8AAN3dAAD//wAAd3cAAP//AADd3QAA//8AAHd3AAD//wAA3d0AAP//AAB3dwAA//8AAN3dAAD//wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wCqqgAA//8AAKqqAAD//wAAqqoAAP//AACqqgAA//8AAKqqAAD//wAAqqoAAP//AACqqgAA//8AAKqqAAD//wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wCqqgAAd3cAAKqqAADd3QAAqqoAAHd3AACqqgAA3d0AAKqqAAB3dwAAqqoAAN3dAACqqgAAd3cAAKqqAADd3QAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wBVVQAAqqoAAFVVAACqqgAAVVUAAKqqAABVVQAAqqoAAFVVAACqqgAAVVUAAKqqAABVVQAAqqoAAFVVAACqqgAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAREQAAqqoAAEREAACqqgAAEREAAKqqAABERAAAqqoAABERAACqqgAAREQAAKqqAAAREQAAqqoAAEREAACqqgAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAqqoAAAAAAACqqgAAAAAAAKqqAAAAAAAAqqoAAAAAAACqqgAAAAAAAKqqAAAAAAAAqqoAAAAAAACqqgAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAIiIAAAAAAACIiAAAAAAAACIiAAAAAAAAiIgAAAAAAAAiIgAAAAAAAIiIAAAAAAAAIiIAAAAAAACIiAAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDv7wAA7+8AAO/vAAAAAAAA/v4AAP7+AAD+/gAAAAAAAO/vAADv7wAA7+8AAAAAAAD+/gAA/v4AAP7+AAAAAAAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD39wAA7+8AANfXAAC7uwAAfX0AAP7+AAD9/QAA+/sAAPf3AADv7wAA19cAALu7AAB9fQAA/v4AAP39AAD7+wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wC+vgAAf38AAP//AAD//wAA6+sAAPf3AAD//wAA//8AAL6+AAB/fwAA//8AAP//AADr6wAA9/cAAP//AAD//wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wBfXwAAqqoAAPv7AAD7+wAA9fUAAKqqAAC/vwAAv78AAF9fAACqqgAA+/sAAPv7AAD19QAAqqoAAL+/AAC/vwAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD7+wAA//8AAL+/AADf3wAA7+8AAP//AAD+/gAA/f0AAPv7AAD//wAAv78AAN/fAADv7wAA//8AAP7+AAD9/QAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD/+QAAM88AAHJOAAB+fgAA5+cAAOTkAACcnAAAn5kAAP/5AAAzzwAAck4AAH5+AADn5wAA5OQAAJycAACfmQAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//4AAP//AAD3/wAA//8AAP/fAAD//wAA//8AAP//AAD//gAA//8AAPf/AAD//wAA/98AAP//AAD//wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wBwcAAAOTkAAJOTAAAHBwAADg4AAJycAADJyQAA4OAAAHBwAAA5OQAAk5MAAAcHAAAODgAAnJwAAMnJAADg4AAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA7u4AAH19AAC7uwAA19cAAO7uAAD//wAAqqoAAP//AADu7gAAfX0AALu7AADX1wAA7u4AAP//AACqqgAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA7e4AAP//AADv7wAA//8AAKqqAAD//wAA7+8AAP//AADu7gAA//8AAO/vAAD//wAAqqoAAP//AADv7wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAODgAADg4AAG5uAAAREQAA4OAAAODgAADm5gAAEREAAA4OAAAODgAAbm4AABERAADg4AAA4OAAAObmAAAREQAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wA4OAAA19cAAO/vAADv7wAAg4MAAH19AAD+/gAA/v4AADg4AADX1wAA7+8AAO/vAACDgwAAfX0AAP7+AAD+/gAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAH9/AACfnwAA5+cAANvbAAC9vQAAfn4AAP7+AAD+/gAAf38AAJ+fAADn5wAA29sAAL29AAB+fgAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAPDwAADw8AAA8PAAAPDwAA8PAAAPDwAADw8AAA8PAAAA8PAAAPDwAADw8AAA8PAADw8AAA8PAAAPDwAADw8AAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDHxwAAg4MAAAEBAAAAAAAAAQEAAIODAADHxwAA7+8AAMfHAACDgwAAAQEAAAAAAAABAQAAg4MAAMfHAADv7wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAu7sAAN3dAADu7gAAAAAAAN3dAAC7uwAAd3cAAAAAAAC7uwAA3d0AAO7uAAAAAAAA3d0AALu7AAB3dwAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/f0AAPv7AAD39wAA7+8AAN/fAAC/vwAAf38AAP7+AAD9/QAA+/sAAPf3AADv7wAA398AAL+/AAB/fwAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDz8wAA5+cAAM/PAACfnwAAPz8AAH5+AAD8/AAA+fkAAPPzAADn5wAAz88AAJ+fAAA/PwAAfn4AAPz8AAD5+QAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDb2wAA5+cAAOfnAADb2wAAvb0AAH5+AAB+fgAAvb0AANvbAADn5wAA5+cAANvbAAC9vQAAfn4AAH5+AAC9vQAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAAAAAAAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAAAAAAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAAAAAAAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAAAAAAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//gAA//0AAP/7AAD/9wAA/+8AAP/fAAD/vwAA/38AAP7/AAD9/wAA+/8AAPf/AADv/wAA3/8AAL//AAB//wAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wB//AAA//gAAP/xAAD/4wAA/8cAAP+PAAD/HwAA/j8AAPx/AAD4/wAA8f8AAOP/AADH/wAAj/8AAB//AAA//gAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wB//gAAv/0AAN/7AADv9wAA9+8AAPvfAAD9vwAA/n8AAP5/AAD9vwAA+98AAPfvAADv9wAA3/sAAL/9AAB//gAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAAAAAAA',%s", stg_nl);
	fprintf(out, "  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAAAAAAAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAAAAAAA'%s", stg_nl);
	fprintf(out, "];%s", stg_nl);

	fprintf(out, "function showPopup (id) {%s", stg_nl);
	fprintf(out, "  var a = document.getElementById(id + '_content');%s", stg_nl);
	fprintf(out, "  if (a.style.display == 'none' || a.style.display == '') {%s", stg_nl);
	fprintf(out, "    a.style.display = 'block';%s", stg_nl);
	fprintf(out, "  } else {%s", stg_nl);
	fprintf(out, "    a.style.display = 'none';%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	fprintf(out, "function hidePopup (id) {%s", stg_nl);
	fprintf(out, "  var a = document.getElementById(id + '_content');%s", stg_nl);
	fprintf(out, "  a.style.display = 'none';%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	fprintf(out, "function showInfo () {%s", stg_nl);
	fprintf(out, "  var id = 'hyp_info';%s", stg_nl);
	fprintf(out, "  var a = document.getElementById(id + '_content');%s", stg_nl);
	fprintf(out, "  if (a.style.display == 'none' || a.style.display == '') {%s", stg_nl);
	fprintf(out, "    a.style.display = 'inline-block';%s", stg_nl);
	fprintf(out, "  } else {%s", stg_nl);
	fprintf(out, "    a.style.display = 'none';%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	fprintf(out, "function getQueryVariable(variable)%s", stg_nl);
	fprintf(out, "{%s", stg_nl);
	fprintf(out, "  var query = window.location.search.substring(1);%s", stg_nl);
	fprintf(out, "  var vars = query.split('&');%s", stg_nl);
	fprintf(out, "  for (var i = 0; i < vars.length; i++) {%s", stg_nl);
	fprintf(out, "    var pair = vars[i].split('=');%s", stg_nl);
	fprintf(out, "    if (decodeURIComponent(pair[0]) == variable) {%s", stg_nl);
	fprintf(out, "      return decodeURIComponent(pair[1]);%s", stg_nl);
	fprintf(out, "    }%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  return '';%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	fprintf(out, "var lang;%s", stg_nl);
	fprintf(out, "var languages = ['en', 'de'];%s", stg_nl);

	fprintf(out, "function getSupportedLanguage(l)%s", stg_nl);
	fprintf(out, "{%s", stg_nl);
	fprintf(out, "  for (var j = 0; j < languages.length; j++) {%s", stg_nl);
	fprintf(out, "    if (languages[j] == l) return l;%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  return '';%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	fprintf(out, "function getAcceptLanguage()%s", stg_nl);
	fprintf(out, "{%s", stg_nl);
	fprintf(out, "  if (window.navigator.languages) {%s", stg_nl);
	fprintf(out, "    for (var i = 0; i < window.navigator.languages.length; i++) {%s", stg_nl);
	fprintf(out, "      var l = window.navigator.languages[i].split('-')[0].split('_')[0];%s", stg_nl);
	fprintf(out, "      l = getSupportedLanguage(l);%s", stg_nl);
	fprintf(out, "      if (l != '') return l;%s", stg_nl);
	fprintf(out, "    }%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  return '';%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	fprintf(out, "function getLanguage()%s", stg_nl);
	fprintf(out, "{%s", stg_nl);
	fprintf(out, "  var l = getQueryVariable('lang').split('-')[0].split('_')[0];%s", stg_nl);
	fprintf(out, "  if (l == '') {%s", stg_nl);
	fprintf(out, "    l = getAcceptLanguage();%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  l = getSupportedLanguage(l);%s", stg_nl);
	fprintf(out, "  if (l == '')%s", stg_nl);
	fprintf(out, "    l = getSupportedLanguage(navigator.language);%s", stg_nl);
	fprintf(out, "  if (l == '')%s", stg_nl);
	fprintf(out, "    l = 'en';%s", stg_nl);
	fprintf(out, "  lang = l;%s", stg_nl);
	fprintf(out, "  var html = document.getElementsByTagName('html')[0];%s", stg_nl);
	fprintf(out, "  html.setAttribute('lang', lang);%s", stg_nl);
	fprintf(out, "  html.setAttribute('xml:lang', lang);%s", stg_nl);
	fprintf(out, "}%s", stg_nl);
	fprintf(out, "getLanguage();%s", stg_nl);

	fprintf(out, "function drawLine(id, xoffset, width, height, begarrow, endarrow, linestyle)%s", stg_nl);
	fprintf(out, "{%s", stg_nl);
	fprintf(out, "  var c = document.getElementById(id);%s", stg_nl);
	fprintf(out, "  var ctx = c.getContext('2d');%s", stg_nl);
	fprintf(out, "  var style = window.getComputedStyle(c);%s", stg_nl);
	fprintf(out, "  ctx.font = style.getPropertyValue('font-size') + ' ' + style.getPropertyValue('font-family');%s", stg_nl);
	fprintf(out, "  var d = document.getElementById('hypview_linetest');%s", stg_nl);
	fprintf(out, "  var s;%s", stg_nl);
	fprintf(out, "  if (d == null)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    d = document.createElement('div');%s", stg_nl);
	fprintf(out, "    var p = document.createElement('pre');%s", stg_nl);
	fprintf(out, "    var s = document.createElement('span');%s", stg_nl);
	fprintf(out, "    var t = document.createTextNode('x');%s", stg_nl);
	fprintf(out, "    s.appendChild(t);%s", stg_nl);
	fprintf(out, "    p.appendChild(s);%s", stg_nl);
	fprintf(out, "    d.appendChild(p);%s", stg_nl);
	fprintf(out, "    d.style.visibility = 'hidden';%s", stg_nl);
	fprintf(out, "    document.body.appendChild(d);%s", stg_nl);
	fprintf(out, "  } else {%s", stg_nl);
	fprintf(out, "    s = d.firstChild().firstChild();%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  var xraster = s.offsetWidth;%s", stg_nl);
	fprintf(out, "  var yraster = s.offsetHeight;%s", stg_nl);
	fprintf(out, "  var x0, y0, x1, y1;%s", stg_nl);
	fprintf(out, "  if (width < 0)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    xoffset += width;%s", stg_nl);
	fprintf(out, "    /* draw from right to left */%s", stg_nl);
	fprintf(out, "    width = (-width) * xraster;%s", stg_nl);
	fprintf(out, "    x0 = width + 0.5;%s", stg_nl);
	fprintf(out, "    x1 = 0.5;%s", stg_nl);
	fprintf(out, "  } else if (width == 0)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    /* vertical line */%s", stg_nl);
	fprintf(out, "    width = 1;%s", stg_nl);
	fprintf(out, "    x0 = 0.5;%s", stg_nl);
	fprintf(out, "    x1 = 0.5;%s", stg_nl);
	fprintf(out, "  } else%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    /* draw from left to right */%s", stg_nl);
	fprintf(out, "    width = width * xraster;%s", stg_nl);
	fprintf(out, "    x0 = 0.5;%s", stg_nl);
	fprintf(out, "    x1 = width + 0.5;%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  if (height < 0)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    /* draw from bottom to top */%s", stg_nl);
	fprintf(out, "    height = (-height) * yraster;%s", stg_nl);
	fprintf(out, "    y0 = height + 0.5;%s", stg_nl);
	fprintf(out, "    y1 = 0.5;%s", stg_nl);
	fprintf(out, "  } else if (height == 0)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    /* horizontal line */%s", stg_nl);
	fprintf(out, "    height = 1;%s", stg_nl);
	fprintf(out, "    y0 = 0.5;%s", stg_nl);
	fprintf(out, "    y1 = 0.5;%s", stg_nl);
	fprintf(out, "  } else%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    /* draw from top to bottom */%s", stg_nl);
	fprintf(out, "    height = height * yraster;%s", stg_nl);
	fprintf(out, "    y0 = 0.5;%s", stg_nl);
	fprintf(out, "    y1 = height + 0.5;%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  c.width = width + 1;%s", stg_nl);
	fprintf(out, "  c.height = height + 1;%s", stg_nl);
	fprintf(out, "  c.style.left = ((xoffset - 1) * xraster).toString() + 'px';%s", stg_nl);
	fprintf(out, "  ctx.lineWidth = 1;%s", stg_nl);
	fprintf(out, "  ctx.shadowBlur = 0;%s", stg_nl);
	fprintf(out, "  ctx.filter = 'none';%s", stg_nl);
	fprintf(out, "  ctx.strokeStyle = style.getPropertyValue('color');%s", stg_nl);
	fprintf(out, "  ctx.imageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  ctx.mozImageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  ctx.webkitImageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  ctx.msImageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  try {%s", stg_nl);
	fprintf(out, "    switch (linestyle)%s", stg_nl);
	fprintf(out, "    {%s", stg_nl);
	fprintf(out, "    default:%s", stg_nl);
	fprintf(out, "    case 1: /* SOLID */%s", stg_nl);
	fprintf(out, "      break;%s", stg_nl);
	fprintf(out, "    case 2: /* LONGDASH */%s", stg_nl);
	fprintf(out, "      ctx.setLineDash([12, 4]);%s", stg_nl);
	fprintf(out, "      break;%s", stg_nl);
	fprintf(out, "    case 3: /* DOT */%s", stg_nl);
	fprintf(out, "      ctx.setLineDash([2, 6, 2, 6]);%s", stg_nl);
	fprintf(out, "      break;%s", stg_nl);
	fprintf(out, "    case 4: /* DASHDOT */%s", stg_nl);
	fprintf(out, "      ctx.setLineDash([8, 3, 2, 3]);%s", stg_nl);
	fprintf(out, "      break;%s", stg_nl);
	fprintf(out, "    case 5: /* DASH */%s", stg_nl);
	fprintf(out, "      ctx.setLineDash([8, 8]);%s", stg_nl);
	fprintf(out, "      break;%s", stg_nl);
	fprintf(out, "    case 6: /* DASH2DOT */%s", stg_nl);
	fprintf(out, "      ctx.setLineDash([4, 3, 2, 2, 1, 3, 1, 0]);%s", stg_nl);
	fprintf(out, "      break;%s", stg_nl);
	fprintf(out, "    case 7: /* USERLINE */%s", stg_nl);
	fprintf(out, "      ctx.setLineDash([1, 1]);%s", stg_nl);
	fprintf(out, "      break;%s", stg_nl);
	fprintf(out, "    }%s", stg_nl);
	fprintf(out, "  } catch (e)%s", stg_nl);
	                  /* setLineDash not supported, but not much we can do about it */
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  ctx.beginPath();%s", stg_nl);
	fprintf(out, "  ctx.moveTo(x0, y0);%s", stg_nl);
	fprintf(out, "  ctx.lineTo(x1, y1);%s", stg_nl);
	fprintf(out, "  ctx.stroke();%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	fprintf(out, "function roundedBox(ctx, width, height)%s", stg_nl);
	fprintf(out, "{%s", stg_nl);
	fprintf(out, "  var deltax, deltay, xrad, yrad;%s", stg_nl);
	fprintf(out, "  rdeltax = width / 2;%s", stg_nl);
	fprintf(out, "  rdeltay = height / 2;%s", stg_nl);
	fprintf(out, "  xrad = 15;%s", stg_nl);
	fprintf(out, "  if (xrad > rdeltax)%s", stg_nl);
	fprintf(out, "    xrad = rdeltax;%s", stg_nl);
	fprintf(out, "  yrad = xrad;%s", stg_nl);
	fprintf(out, "  if (yrad > rdeltay)%s", stg_nl);
	fprintf(out, "    yrad = rdeltay;%s", stg_nl);
	fprintf(out, "  ctx.beginPath();%s", stg_nl);
	fprintf(out, "  ctx.moveTo(0.5, 0.5 + yrad);%s", stg_nl);
	fprintf(out, "  ctx.quadraticCurveTo(0.5, 0.5, 0.5 + xrad, 0.5);%s", stg_nl);
	fprintf(out, "  ctx.lineTo(width + 0.5 - xrad, 0.5);%s", stg_nl);
	fprintf(out, "  ctx.quadraticCurveTo(width + 0.5, 0.5, width + 0.5, 0.5 + yrad);%s", stg_nl);
	fprintf(out, "  ctx.lineTo(width + 0.5, height + 0.5 - yrad);%s", stg_nl);
	fprintf(out, "  ctx.quadraticCurveTo(width + 0.5, height + 0.5, width + 0.5 - xrad, height + 0.5);%s", stg_nl);
	fprintf(out, "  ctx.lineTo(0.5 + xrad, height + 0.5);%s", stg_nl);
	fprintf(out, "  ctx.quadraticCurveTo(0.5, height + 0.5, 0.5, height + 0.5 - yrad);%s", stg_nl);
	fprintf(out, "  ctx.lineTo(0.5, 0.5 + yrad);%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	fprintf(out, "function drawBox(id, xoffset, width, height, fillstyle, rounded)%s", stg_nl);
	fprintf(out, "{%s", stg_nl);
	fprintf(out, "  var c = document.getElementById(id);%s", stg_nl);
	fprintf(out, "  var ctx = c.getContext('2d');%s", stg_nl);
	fprintf(out, "  var style = window.getComputedStyle(c);%s", stg_nl);
	fprintf(out, "  ctx.font = style.getPropertyValue('font-size') + ' ' + style.getPropertyValue('font-family');%s", stg_nl);
	fprintf(out, "  var d = document.getElementById('hypview_linetest');%s", stg_nl);
	fprintf(out, "  var s;%s", stg_nl);
	fprintf(out, "  if (d == null)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    d = document.createElement('div');%s", stg_nl);
	fprintf(out, "    var p = document.createElement('pre');%s", stg_nl);
	fprintf(out, "    var s = document.createElement('span');%s", stg_nl);
	fprintf(out, "    var t = document.createTextNode('x');%s", stg_nl);
	fprintf(out, "    s.appendChild(t);%s", stg_nl);
	fprintf(out, "    p.appendChild(s);%s", stg_nl);
	fprintf(out, "    d.appendChild(p);%s", stg_nl);
	fprintf(out, "    d.style.visibility = 'hidden';%s", stg_nl);
	fprintf(out, "    document.body.appendChild(d);%s", stg_nl);
	fprintf(out, "  } else {%s", stg_nl);
	fprintf(out, "    s = d.firstChild().firstChild();%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  var xraster = s.offsetWidth;%s", stg_nl);
	fprintf(out, "  var yraster = s.offsetHeight;%s", stg_nl);
	fprintf(out, "  width = width * xraster;%s", stg_nl);
	fprintf(out, "  height = height * yraster;%s", stg_nl);
	fprintf(out, "  c.style.left = ((xoffset - 1) * xraster).toString() + 'px';%s", stg_nl);
	fprintf(out, "  ctx.lineWidth = 1;%s", stg_nl);
	fprintf(out, "  ctx.shadowBlur = 0;%s", stg_nl);
	fprintf(out, "  ctx.filter = 'none';%s", stg_nl);
	fprintf(out, "  ctx.strokeStyle = style.getPropertyValue('color');%s", stg_nl);
	fprintf(out, "  ctx.imageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  ctx.mozImageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  ctx.webkitImageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  ctx.msImageSmoothingEnabled = false;%s", stg_nl);
	fprintf(out, "  if (rounded)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    roundedBox(ctx, width, height);%s", stg_nl);
	fprintf(out, "  } else%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    ctx.rect(0.5, 0.5, width, height);%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  if (fillstyle != 0)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    if ((fillstyle >= 1 && fillstyle <= 7) || (fillstyle >= 9 && fillstyle <= 36))%s", stg_nl);
	fprintf(out, "    {%s", stg_nl);
	fprintf(out, "      var img;%s", stg_nl);
	fprintf(out, "      img = new Image();%s", stg_nl);
	fprintf(out, "      img.src = patterns[fillstyle];%s", stg_nl);
	fprintf(out, "      img.onload = function() {%s", stg_nl);
	fprintf(out, "        ctx.fillStyle = ctx.createPattern(img, 'repeat');%s", stg_nl);
	fprintf(out, "        ctx.fill();%s", stg_nl);
	fprintf(out, "        if (fillstyle != 8)%s", stg_nl);
	fprintf(out, "        {%s", stg_nl);
	fprintf(out, "          ctx.stroke();%s", stg_nl);
	fprintf(out, "        }%s", stg_nl);
	fprintf(out, "      };%s", stg_nl);
	fprintf(out, "      return;%s", stg_nl);
	fprintf(out, "    } else%s", stg_nl);
	fprintf(out, "    {%s", stg_nl);
	fprintf(out, "      ctx.fill();%s", stg_nl);
	fprintf(out, "    }%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "  if (fillstyle != 8)%s", stg_nl);
	fprintf(out, "  {%s", stg_nl);
	fprintf(out, "    ctx.stroke();%s", stg_nl);
	fprintf(out, "  }%s", stg_nl);
	fprintf(out, "}%s", stg_nl);

	if (do_inline)
	{
		if (html_doctype >= HTML_DOCTYPE_XSTRICT)
			fprintf(out, "//]]>%s", stg_nl);
		fprintf(out, "</script>%s", stg_nl);
	}
	
	if (out != outfile)
		hyp_utf8_fclose(out);
	
	return TRUE;
}
	
/* ------------------------------------------------------------------------- */

static void html_out_nav_toolbar(HYP_DOCUMENT *hyp, hcp_opts *opts, FILE *outfile, hyp_nodenr node, struct html_xref *xrefs)
{
	INDEX_ENTRY *entry = hyp->indextable[node];
	char *str;
	char *title;
	const char *alt;
	const char *disabled;
	const char *void_href = "javascript:void(0);";
	
	fprintf(outfile, "<div class=\"%s\">%s", html_toolbar_style, stg_nl);
	
	fprintf(outfile, "<form action=\"hypview.cgi\" method=\"get\">%s", stg_nl);
	fprintf(outfile, "<fieldset style=\"border:0;margin-left:0;margin-right:0;padding-top:0;padding-bottom:0;padding-left:0;padding-right:0;\">%s", stg_nl);
	fprintf(outfile, "<legend></legend>%s", stg_nl);
	fprintf(outfile, "<ul>%s", stg_nl);
	alt = N_("Back");
	fprintf(outfile,
		"<li style=\"position:absolute;left:0px;\">"
		"<a href=\"javascript: window.history.go(-1)\" class=\"%s\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		html_nav_img_style, html_nav_back_png, alt, alt, html_nav_dimensions, html_closer, stg_nl);
	
	if (hypnode_valid(hyp, entry->previous) &&
		node != entry->previous)
	{
		str = html_filename_for_node(hyp, opts, entry->previous, TRUE);
		title = html_quote_nodename(hyp, entry->previous);
		disabled = "";
	} else
	{
		title = g_strdup(N_("Previous page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_fprintf_charset(outfile, output_charset,
		"<li style=\"position:absolute;left:40px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"p\" rel=\"prev\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		str, html_nav_img_style, disabled, html_nav_prev_png, title, title, html_nav_dimensions, html_closer, stg_nl);
	g_free(title);
	g_free(str);
	
	if (hypnode_valid(hyp, entry->toc_index) &&
		node != entry->toc_index)
	{
		str = html_filename_for_node(hyp, opts, entry->toc_index, TRUE);
		title = html_quote_nodename(hyp, entry->toc_index);
		disabled = "";
	} else
	{
		title = g_strdup(N_("Up"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_fprintf_charset(outfile, output_charset,
		"<li style=\"position:absolute;left:80px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"t\" rel=\"up\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		str, html_nav_img_style, disabled, html_nav_toc_png, title, title, html_nav_dimensions, html_closer, stg_nl);
	g_free(title);
	g_free(str);

	if (hypnode_valid(hyp, entry->next) &&
		node != entry->next)
	{
		str = html_filename_for_node(hyp, opts, entry->next, TRUE);
		title = html_quote_nodename(hyp, entry->next);
		disabled = "";
	} else
	{
		title = g_strdup(N_("Next page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_fprintf_charset(outfile, output_charset,
		"<li style=\"position:absolute;left:120px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"n\" rel=\"next\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		str, html_nav_img_style, disabled, html_nav_next_png, title, title, html_nav_dimensions, html_closer, stg_nl);
	g_free(title);
	g_free(str);

	if (hypnode_valid(hyp, hyp->index_page) &&
		node != hyp->index_page)
	{
		str = html_filename_for_node(hyp, opts, hyp->index_page, TRUE);
		title = html_quote_nodename(hyp, hyp->index_page);
		disabled = "";
	} else
	{
		title = g_strdup(N_("Index page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_fprintf_charset(outfile, output_charset,
		"<li style=\"position:absolute;left:160px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"x\" rel=\"index\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		str, html_nav_img_style, disabled, html_nav_index_png, title, title, html_nav_dimensions, html_closer, stg_nl);
	g_free(title);
	g_free(str);

	if (xrefs != NULL)
	{
		title = g_strdup(N_("Cross references"));
		str = g_strdup(void_href);
		disabled = "";
	} else
	{
		title = g_strdup(N_("Cross references"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_fprintf_charset(outfile, output_charset,
		"<li style=\"position:absolute;left:200px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"c\" rel=\"bookmark\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		str, html_nav_img_style, disabled, html_nav_xref_png, title, title, html_nav_dimensions, html_closer, stg_nl);
	g_free(title);
	g_free(str);

	if (hypnode_valid(hyp, hyp->help_page) &&
		node != hyp->help_page)
	{
		str = html_filename_for_node(hyp, opts, hyp->help_page, TRUE);
		title = html_quote_nodename(hyp, hyp->help_page);
		disabled = "";
	} else
	{
		title = g_strdup(N_("Help page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_fprintf_charset(outfile, output_charset,
		"<li style=\"position:absolute;left:240px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"h\" rel=\"help\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		str, html_nav_img_style, disabled, html_nav_help_png, title, title, html_nav_dimensions, html_closer, stg_nl);
	g_free(title);
	g_free(str);

	alt = N_("Show info about hypertext");
	str = g_strdup(void_href);
	disabled = "";
	hyp_utf8_fprintf_charset(outfile, output_charset,
		"<li style=\"position:absolute;left:280px;\">"
		"<a href=\"%s\" class=\"%s%s\" onclick=\"showInfo()\" accesskey=\"i\" rel=\"copyright\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		str, html_nav_img_style, disabled, html_nav_info_png, alt, alt, html_nav_dimensions, html_closer, stg_nl);
	g_free(str);

	alt = N_("Load a file");
	disabled = "";
	fprintf(outfile,
		"<li style=\"position:absolute;left:340px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"o\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>%s",
		html_nav_load_href, html_nav_img_style, disabled, html_nav_load_png, alt, alt, html_nav_dimensions, html_closer, stg_nl);
	
	fprintf(outfile, "<li style=\"position:absolute;left:380px;\">%s", stg_nl);
	fprintf(outfile, "<input type=\"hidden\" name=\"url\" value=\"%s%s\"%s%s", html_referer_url, hyp_basename(hyp->file), html_closer, stg_nl);
	fprintf(outfile, "<input accesskey=\"s\" type=\"text\" name=\"q\" size=\"10\" value=\"\"%s%s", html_closer, stg_nl);
	fprintf(outfile, "</li>%s", stg_nl);
	fprintf(outfile, "</ul>%s", stg_nl);
	fprintf(outfile, "</fieldset>%s", stg_nl);
	fprintf(outfile, "</form>%s", stg_nl);

	fprintf(outfile, "</div>%s", stg_nl);
}
	
/* ------------------------------------------------------------------------- */

static void html_out_header(HYP_DOCUMENT *hyp, hcp_opts *opts, FILE *outfile, const char *title, hyp_nodenr node, struct hyp_gfx *hyp_gfx, struct html_xref *xrefs)
{
	const char *charset = hyp_charset_name(output_charset);
	INDEX_ENTRY *entry = hyp->indextable[node];
	char *str;
	
	switch (html_doctype)
	{
	case HTML_DOCTYPE_OLD:
		fprintf(outfile, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">%s", stg_nl);
		fprintf(outfile, "<html lang=\"en\">%s", stg_nl);
		break;
	case HTML_DOCTYPE_TRANS:
		fprintf(outfile, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"%s", stg_nl);
		fprintf(outfile, "        \"http://www.w3.org/TR/html4/loose.dtd\">%s", stg_nl);
		fprintf(outfile, "<html lang=\"en\">%s", stg_nl);
		break;
	
	case HTML_DOCTYPE_XSTRICT:
#if 0
		fprintf(outfile, "<?xml version=\"1.0\" encoding=\"%s\"?>%s", charset, stg_nl);
#endif
		fprintf(outfile, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"%s", stg_nl);
		fprintf(outfile, "          \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">%s", stg_nl);
		fprintf(outfile, "<html xml:lang=\"en\" lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\"");
		if (hyp_gfx != NULL)
			fprintf(outfile, " xmlns:svg=\"http://www.w3.org/2000/svg\"");
		fprintf(outfile, ">%s", stg_nl);
		break;
	case HTML_DOCTYPE_STRICT:
	default:
		fprintf(outfile, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"%s", stg_nl);
		fprintf(outfile, "          \"http://www.w3.org/TR/html4/strict.dtd\">%s", stg_nl);
		fprintf(outfile, "<html lang=\"en\">%s", stg_nl);
		break;
	case HTML_DOCTYPE_HTML5:
		fprintf(outfile, "<!DOCTYPE html>%s", stg_nl);
		fprintf(outfile, "<html xml:lang=\"en\" lang=\"en\">%s", stg_nl);
		break;
	case HTML_DOCTYPE_FRAME:
	case HTML_DOCTYPE_XFRAME:
		abort();
		break;
	}
	fprintf(outfile, "<!-- This file was automatically generated by %s version %s -->%s", gl_program_name, gl_program_version, stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- %s -->%s", HYP_COPYRIGHT, stg_nl);
	if (node == 0)
		html_out_globals(hyp, outfile);
	hyp_utf8_fprintf_charset(outfile, output_charset, _("<!-- Node #%u -->%s"), node, stg_nl);
	fprintf(outfile, "<head>%s", stg_nl);
	fprintf(outfile, "<meta http-equiv=\"content-type\" content=\"text/html;charset=%s\"%s%s", charset, html_closer, stg_nl);
	if (html_doctype >= HTML_DOCTYPE_HTML5)
		fprintf(outfile, "<meta charset=\"%s\"%s%s", charset, html_closer, stg_nl);
	fprintf(outfile, "<meta name=\"GENERATOR\" content=\"%s %s\"%s%s", gl_program_name, gl_program_version, html_closer, stg_nl);
	if (hyp->author != NULL)
	{
		char *str = html_quote_name(hyp->author, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<meta name=\"author\" content=\"%s\"%s%s", str, html_closer, stg_nl);
		g_free(str);
	}
	if (hyp->database != NULL)
	{
		char *str = html_quote_name(hyp->database, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "<meta name=\"description\" content=\"%s\"%s%s", str, html_closer, stg_nl);
		g_free(str);
	}
	hyp_utf8_fprintf_charset(outfile, output_charset, "<title>%s</title>%s", title, stg_nl);

	if (entry->type == HYP_NODE_INTERNAL)
	{
		if (hypnode_valid(hyp, hyp->first_text_page) &&
			node != hyp->first_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->first_text_page, TRUE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"%s\"%s%s", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "start" : "first", html_closer, stg_nl);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->previous) &&
			node != entry->previous)
		{
			str = html_filename_for_node(hyp, opts, entry->previous, TRUE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"prev\"%s%s", str, html_closer, stg_nl);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->toc_index) &&
			node != entry->toc_index)
		{
			str = html_filename_for_node(hyp, opts, entry->toc_index, TRUE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"up\"%s%s", str, html_closer, stg_nl);
			g_free(str);
		}

		if (hypnode_valid(hyp, entry->next) &&
			node != entry->next)
		{
			str = html_filename_for_node(hyp, opts, entry->next, TRUE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"next\"%s%s", str, html_closer, stg_nl);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->last_text_page) &&
			node != hyp->last_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->last_text_page, TRUE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"%s\"%s%s", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "end" : "last", html_closer, stg_nl);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, hyp->index_page) &&
			node != hyp->index_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->index_page, TRUE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"index\"%s%s", str, html_closer, stg_nl);
			g_free(str);
		}

		if (xrefs != NULL)
		{
			str = g_strdup("");
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"bookmark\"%s%s", str, html_closer, stg_nl);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->help_page) &&
			node != hyp->help_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->help_page, TRUE);
			hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"help\"%s%s", str, html_closer, stg_nl);
			g_free(str);
		}

		str = g_strdup("javascript: showInfo();");
		hyp_utf8_fprintf_charset(outfile, output_charset, "<link href=\"%s\" rel=\"copyright\"%s%s", str, html_closer, stg_nl);
		g_free(str);
	}
	
	html_out_stylesheet(opts, outfile, FALSE);
	html_out_javascript(opts, outfile, FALSE);
	
	if (html_doctype >= HTML_DOCTYPE_HTML5)
	{
		fprintf(outfile, "<!--[if lt IE 9]>%s", stg_nl);
		fprintf(outfile, "<script src=\"http://html5shiv.googlecode.com/svn/trunk/html5.js\" type=\"text/javascript\"></script>%s", stg_nl);
		fprintf(outfile, "<![endif]-->%s", stg_nl);
	}

	fprintf(outfile, "</head>%s", stg_nl);
	fprintf(outfile, "<body>%s", stg_nl);
	fprintf(outfile, "<div style=\"width:%dex;\">%s", hyp->line_width, stg_nl);

	if (entry->type == HYP_NODE_INTERNAL)
	{
		html_out_nav_toolbar(hyp, opts, outfile, node, xrefs);
		fprintf(outfile, "<div class=\"%s\">%s", html_node_style, stg_nl);
	} else
	{
		fprintf(outfile, "<div class=\"%s\">%s", html_pnode_style, stg_nl);
	}
	fprintf(outfile, "<pre>%s", stg_nl);

	/*
	 * this element is displayed for "About"
	 */
	hyp_utf8_fprintf_charset(outfile, output_charset, "<span class=\"%s\">", html_dropdown_style);
	hyp_utf8_fprintf_charset(outfile, output_charset, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_info_style, html_hyp_info_id);
	str = html_quote_name(hyp->database, FALSE);
	hyp_utf8_fprintf_charset(outfile, output_charset, N_("Topic       : %s%s"), fixnull(str), stg_nl);
	g_free(str);
	str = html_quote_name(hyp->author, FALSE);
	hyp_utf8_fprintf_charset(outfile, output_charset, N_("Author      : %s%s"), fixnull(str), stg_nl);
	g_free(str);
	str = html_quote_name(hyp->version, FALSE);
	hyp_utf8_fprintf_charset(outfile, output_charset, N_("Version     : %s%s"), fixnull(str), stg_nl);
	g_free(str);
	str = html_quote_name(hyp->subject, FALSE);
	hyp_utf8_fprintf_charset(outfile, output_charset, N_("Subject     : %s%s"), fixnull(str), stg_nl);
	g_free(str);
	str = g_strdup_printf(_("Nodes       : %u\n"
	                        "Index Size  : %ld\n"
	                        "HCP-Version : %u\n"
	                        "Compiled on : %s\n"
	                        "@charset    : %s\n"
	                        "@default    : %s\n"
	                        "@help       : %s\n"
	                        "@options    : %s\n"
	                        "@width      : %u"),
	                        hyp->num_index,
	                        hyp->itable_size,
	                        hyp->comp_vers,
	                        hyp_osname(hyp->comp_os),
	                        hyp_charset_name(hyp->comp_charset),
	                        fixnull(hyp->default_name),
	                        fixnull(hyp->help_name),
	                        fixnull(hyp->hcp_options),
	                        hyp->line_width);
	hyp_utf8_fprintf_charset(outfile, output_charset, "%s%s", str, stg_nl);
	g_free(str);
	hyp_utf8_fprintf_charset(outfile, output_charset, "</span></span>");
	{
		HYP_HOSTNAME *h;
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			str = html_quote_name(h->name, FALSE);
			hyp_utf8_fprintf_charset(outfile, output_charset, N_("@hostname   : %s%s"), str, stg_nl);
			g_free(str);
		}
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_trailer(FILE *outfile)
{
	fprintf(outfile, "</pre>%s", stg_nl);
	fprintf(outfile, "</div>%s", stg_nl);
	fprintf(outfile, "</div>%s", stg_nl);
	fprintf(outfile, "</body>%s", stg_nl);
	fprintf(outfile, "</html>%s", stg_nl);
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, symtab_entry *syms, gboolean for_inline)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	unsigned char textattr;
	long lineno;
	struct hyp_gfx *hyp_gfx = NULL;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	FILE *outfile = opts->outfile;
	int popup_id = 0;
	int gfx_id = 0;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		html_out_str(outfile, hyp->comp_charset, textstart, src - textstart); \
		at_bol = FALSE; \
	}
#define FLUSHLINE() \
	if (!at_bol) \
	{ \
		fputs(stg_nl, outfile); \
		at_bol = TRUE; \
	}
#define FLUSHTREE() \
	if (in_tree != -1) \
	{ \
		hyp_utf8_fprintf_charset(outfile, output_charset, "end tree %d -->%s", in_tree, stg_nl); \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		INDEX_ENTRY *entry;
		struct html_xref *xrefs = NULL;
		struct html_xref **last_xref = &xrefs;

		{
		char *title;
		
		if (for_inline)
		{
			char *name = g_strdup_printf("___%u.tmp", node);
			char *output_filename;
			FILE *out;
			
			output_filename = g_build_filename(opts->output_dir, name, NULL);
			out = hyp_utf8_fopen(output_filename, "wb");
			if (out == NULL)
			{
				hyp_node_free(nodeptr);
				hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, output_filename, hyp_utf8_strerror(errno));
				g_free(output_filename);
				g_free(name);
				return FALSE;
			}
			g_free(output_filename);
			g_free(name);
			outfile = out;
		} else if (outfile != stdout && node != 0)
		{
			char *name = html_filename_for_node(hyp, opts, node, FALSE);
			char *output_filename;
			FILE *out;
			
			/*
			 * the first file was alreday created in the main loop,
			 * create the output file for the next node
			 */
			output_filename = g_build_filename(opts->output_dir, name, NULL);
			out = hyp_utf8_fopen(output_filename, "wb");
			if (out == NULL)
			{
				hyp_node_free(nodeptr);
				hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, output_filename, hyp_utf8_strerror(errno));
				g_free(output_filename);
				g_free(name);
				return FALSE;
			}
			if (opts->verbose >= 2 && outfile != stdout)
				hyp_utf8_fprintf(stdout, _("writing %s\n"), name);
			g_free(output_filename);
			g_free(name);
			outfile = out;
		}
		
		entry = hyp->indextable[node];
		hyp_node_find_windowtitle(nodeptr);
		
		if (nodeptr->window_title)
		{
			char *buf = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
			title = html_quote_name(buf, FALSE);
			g_free(buf);
		} else
		{
			title = html_quote_nodename(hyp, node);
		}

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
							adm->format = format_from_pic(opts, hyp->indextable[adm->extern_node_index], HTML_DEFAULT_PIC_TYPE);
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
					char *buf;
					
					dest_page = DEC_255(&src[3]);
					buf = hyp_conv_to_utf8(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
					buf = chomp(buf);
					text = html_quote_name(buf, FALSE);
					g_free(buf);
					if (hypnode_valid(hyp, dest_page))
					{
						str = html_filename_for_node(hyp, opts, dest_page, TRUE);
					} else
					{
						str = hyp_invalid_page(dest_page);
					}
					if (empty(text))
					{
						g_free(text);
						text = html_quote_nodename(hyp, dest_page);
					}
					{
						struct html_xref *xref;
						xref = g_new(struct html_xref, 1);
						xref->str = str;
						xref->text = text;
						xref->next = NULL;
						*last_xref = xref;
						last_xref = &(xref)->next;
					}
				}
				break;
			default:
				break;
			}
			src = hyp_skip_esc(src);
		}

		if (!for_inline)
			html_out_header(hyp, opts, outfile, title, node, hyp_gfx, xrefs);

		g_free(title);
		}
		
		/*
		 * check for alias names in ref file
		 */
		html_out_alias(outfile, hyp, entry, syms);
		
		/*
		 * now output data
		 */
		src = nodeptr->start;
		textstart = src;
		at_bol = TRUE;
		in_tree = -1;
		textattr = 0;
		lineno = 0;
		html_out_labels(outfile, hyp, entry, lineno, syms);
		html_out_graphics(opts, outfile, hyp, hyp_gfx, lineno, &gfx_id);
		
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
					fputs("&#x1b;", outfile);
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
						char *destfilename;
						char *destname;
						size_t len;
						gboolean str_equal;
						hyp_indextype desttype;
						const char *target;
						
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
							INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
							destfilename = html_filename_for_node(hyp, opts, dest_page, TRUE);
							destname = html_quote_nodename(hyp, dest_page);
							desttype = (hyp_indextype) dest_entry->type;
						} else
						{
							destfilename = g_strdup("/nonexistent.html");
							str = hyp_invalid_page(dest_page);
							destname = html_quote_name(str, FALSE);
							g_free(str);
							desttype = HYP_NODE_EOF;
						}

						if (*src <= HYP_STRLEN_OFFSET)
						{
							src++;
							if (hypnode_valid(hyp, dest_page))
							{
								INDEX_ENTRY *entry = hyp->indextable[dest_page];
								len = entry->length - SIZEOF_INDEX_ENTRY;
								textstart = entry->name;
								str = html_quote_nodename(hyp, dest_page);
								str_equal = entry->type == HYP_NODE_INTERNAL;
							} else
							{
								textstart = (const unsigned char *)str;
								str_equal = FALSE;
								str = g_strdup(destname);
								len = strlen(str);
							}
						} else
						{
							char *buf;
							
							len = *src - HYP_STRLEN_OFFSET;
							src++;
							textstart = src;
							buf = hyp_conv_to_utf8(hyp->comp_charset, src, len);
							str = html_quote_name(buf, FALSE);
							g_free(buf);
							src += len;
							if (hypnode_valid(hyp, dest_page))
							{
								INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
								str_equal = dest_entry->type == HYP_NODE_INTERNAL && strcmp(str, destname) == 0;
							} else
							{
								str_equal = FALSE;
							}
						}
						FLUSHTREE();
						UNUSED(str_equal);
						
						switch (desttype)
						{
						case HYP_NODE_EOF:
							hyp_utf8_fprintf(outfile, _("<a class=\"%s\" href=\"%s\">%s</a>"), html_error_link_style, destfilename, destname);
							break;
						case HYP_NODE_INTERNAL:
						case HYP_NODE_POPUP:
						case HYP_NODE_EXTERNAL_REF:
							target = type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE ? " target=\"_new\"" : "";
							{
							gboolean is_xref = FALSE;
							char *style;
							if (desttype == HYP_NODE_EXTERNAL_REF)
							{
								char *p = ((hyp->st_guide_flags & STG_ALLOW_FOLDERS_IN_XREFS) ? strrslash : strslash)(destname);
								style = g_strdup_printf(" class=\"%s\"", html_xref_link_style);
								if (p != NULL)
								{
									char c = *p;
									*p = '\0';
									is_xref = hyp_guess_filetype(destname) != HYP_FT_NONE;
									if (hyp_guess_filetype(destname) == HYP_FT_RSC)
									{
										is_xref = TRUE;
										hyp_utf8_fprintf_charset(outfile, output_charset, "<a%s href=\"%s&file=%s&tree=%u\">%s></a>", style, html_view_rsc_href, destname, line, str);
									} else if (hyp_guess_filetype(destname) == HYP_FT_HYP)
									{
										char *base = replace_ext(hyp_basename(destname), HYP_EXT_HYP, NULL);
										is_xref = TRUE;
										html_convert_filename(base);
										hyp_utf8_fprintf_charset(outfile, output_charset, "<a%s href=\"../%s/%s.html\">%s></a>", style, base, base, str);
										g_free(base);
									}
									if (!is_xref)
										*p = c;
								}
							} else if (desttype == HYP_NODE_POPUP)
							{
								style = g_strdup_printf(" class=\"%s\"", html_popup_link_style);
							} else
							{
								style = g_strdup("");
							}
							
							if (!is_xref)
							{
								symtab_entry *sym;
								const char *label;
								
								label = NULL;
								if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
								{
									sym = sym_find(syms, destname, REF_LABELNAME);
									while (sym)
									{
										if (sym->lineno == line /* && !sym->from_idx */)
										{
											label = sym->name;
											sym->referenced = TRUE;
											break;
										}
										sym = sym_find(sym->next, destname, REF_LABELNAME);
									}
								}
								if (label)
								{
									char *quoted = html_quote_name(label, FALSE);
									hyp_utf8_fprintf_charset(outfile, output_charset, "<a%s href=\"%s#%s\"%s>%s</a>", style, destfilename, quoted, target, str);
									g_free(quoted);
								} else if (desttype == HYP_NODE_POPUP && *target == '\0')
								{
									char *id;
									char *name;
									char *output_filename;
									FILE *in;
									char buf[1024];
									int nread;
									
									++popup_id;
									id = g_strdup_printf("hypview_popup_%d", popup_id);
									hyp_utf8_fprintf_charset(outfile, output_charset, "<span class=\"%s\">", html_dropdown_style);
									hyp_utf8_fprintf_charset(outfile, output_charset, "<a%s id=\"%s_btn\" href=\"javascript:void(0);\" onclick=\"showPopup('%s')\">%s</a>", style, id, id, str);
									hyp_utf8_fprintf_charset(outfile, output_charset, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_pnode_style, id);
									
									name = g_strdup_printf("___%u.tmp", dest_page);
									output_filename = g_build_filename(opts->output_dir, name, NULL);
									if (html_out_node(hyp, opts, dest_page, syms, TRUE))
									{
										in = hyp_utf8_fopen(output_filename, "rb");
										if (in == NULL)
										{
											hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, output_filename, hyp_utf8_strerror(errno));
										} else
										{
											while ((nread = (int)fread(buf, 1, sizeof(buf), in)) > 0)
												fwrite(buf, 1, nread, outfile);
											hyp_utf8_fclose(in);
										}
									}
									fprintf(outfile, "</span></span>");
									hyp_utf8_unlink(output_filename);
									g_free(output_filename);
									g_free(name);
									g_free(id);
								} else
								{
									hyp_utf8_fprintf_charset(outfile, output_charset, "<a%s href=\"%s\"%s>%s</a>", style, destfilename, target, str);
								}
							}
							g_free(style);
							}
							break;
						case HYP_NODE_REXX_COMMAND:
							hyp_utf8_fprintf_charset(outfile, output_charset, "<a class=\"%s\" href=\"%s\">%s</a>", html_rx_link_style, destname, str);
							break;
						case HYP_NODE_REXX_SCRIPT:
							hyp_utf8_fprintf_charset(outfile, output_charset, "<a class=\"%s\" href=\"%s\">%s</a>", html_rxs_link_style, destname, str);
							break;
						case HYP_NODE_SYSTEM_ARGUMENT:
							hyp_utf8_fprintf_charset(outfile, output_charset, "<a class=\"%s\" href=\"%s\">%s</a>", html_system_link_style, destname, str);
							break;
						case HYP_NODE_IMAGE:
							/* that would be an inline image; currently not supported by compiler */
							hyp_utf8_fprintf_charset(outfile, output_charset, "<a class=\"%s\" href=\"%s\">%s</a>", html_image_link_style, destname, str);
							break;
						case HYP_NODE_QUIT:
							/* not really quit, but best we can do */
							hyp_utf8_fprintf_charset(outfile, output_charset, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_quit_link_style, str);
							break;
						case HYP_NODE_CLOSE:
							hyp_utf8_fprintf_charset(outfile, output_charset, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_close_link_style, str);
							break;
						default:
							hyp_utf8_fprintf(outfile, _("<a class=\"%s\" href=\"%s\">link to unknown node type %u</a>"), html_error_link_style, destfilename, hyp->indextable[dest_page]->type);
							break;
						}

						g_free(destname);
						g_free(destfilename);
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
							str = html_quote_nodename(hyp, dest_page);
						} else
						{
							str = hyp_invalid_page(dest_page);
						}
						FLUSHLINE();
						if (tree != in_tree)
						{
							FLUSHTREE();
							hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- begin tree %d%s", tree, stg_nl);
							in_tree = tree;
						}
						hyp_utf8_fprintf_charset(outfile, output_charset, "   %d \"%s\" %u%s", obj, str, line, stg_nl);
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
					if (html_out_attr(outfile, textattr, *src - HYP_ESC_TEXTATTR_FIRST))
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
				fputs(stg_nl, outfile);
				at_bol = TRUE;
				++lineno;
				html_out_labels(outfile, hyp, entry, lineno, syms);
				html_out_graphics(opts, outfile, hyp, hyp_gfx, lineno, &gfx_id);
				src++;
				textstart = src;
			} else
			{
				FLUSHTREE();
				src++;
			}
		}
		DUMPTEXT();
		if (html_out_attr(outfile, textattr, 0))
			at_bol = FALSE;
		FLUSHLINE();
		FLUSHTREE();
		++lineno;
		html_out_labels(outfile, hyp, entry, lineno, syms);
		html_out_graphics(opts, outfile, hyp, hyp_gfx, lineno, &gfx_id);
		
		if (hyp_gfx != NULL)
		{
			struct hyp_gfx *gfx, *next;
			
			for (gfx = hyp_gfx; gfx != NULL; gfx = next)
			{
				if (!gfx->used)
				{
					hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- gfx unused: ");
					html_out_gfx(opts, outfile, hyp, gfx, &gfx_id);
					hyp_utf8_fprintf_charset(outfile, output_charset, "-->%s", stg_nl);
				}
				next = gfx->next;
				g_free(gfx);
			}
		}
		
		{
			struct html_xref *xref, *next;
			
			for (xref = xrefs; xref != NULL; xref = next)
			{
				next = xref->next;
				g_free(xref->str);
				g_free(xref->text);
				g_free(xref);
			}
		}
		
		hyp_node_free(nodeptr);
		
		if (!for_inline)
		{
			html_out_trailer(outfile);
		
			if (node < hyp->last_text_page && outfile == stdout)
				hyp_utf8_fprintf_charset(outfile, output_charset, "%s%s", stg_nl, stg_nl);
		}
		
		if (outfile != stdout && outfile != opts->outfile)
			hyp_utf8_fclose(outfile);
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

/* uses stg_check_links() from outstg.h */

/* ------------------------------------------------------------------------- */

/* uses write_image() from outstg.h */

/* ------------------------------------------------------------------------- */

/* uses ref_loadsyms() from outstg.h */

/* ------------------------------------------------------------------------- */

/* uses free_symtab() from outstg.h */

/* ------------------------------------------------------------------------- */

#if 0
static void create_patterns(void)
{
	Base64 *enc;
	unsigned char data[PATTERN_WIDTH * PATTERN_HEIGHT * 4];
	int x, y;
	int px, py;
	int i;
	unsigned char r = 0, g = 0, b = 0, a = 0xff;
	const unsigned char *pbits;
	unsigned char *dst;
	int dst_stride = PATTERN_WIDTH * 4;
	
	enc = Base64_New();
	for (i = 0; i < NUM_PATTERNS; i++)
	{
		py = 0;
		pbits = pattern_bits + i * PATTERN_SIZE;
		for (y = 0; y < PATTERN_HEIGHT; y++)
		{
			dst = data + y * dst_stride;
			px = 0;
			for (x = 0; x < PATTERN_WIDTH; x++)
			{
				dst[0] = r;
				dst[1] = g;
				dst[2] = b;
				if (pbits[(px >> 3) & 1] & (0x80 >> (px & 7)))
					dst[3] = a;
				else
					dst[3] = 0;
				px++;
				dst += 4;
			}
			pbits += PATTERN_WIDTH / 8;
			py++;
			if (py == PATTERN_HEIGHT)
			{
				py = 0;
				pbits = pattern_bits;
			}
		}
		Base64_Encode(enc, data, sizeof(data));
		printf("  '%s',\n", Base64_EncodedMessage(enc));
	}
	exit(0);
}
#endif

static gboolean recompile_html(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	FILE *outfile = opts->outfile;
	
	UNUSED(argc);
	UNUSED(argv);
	
	stg_nl = (outfile == stdout || output_charset != HYP_CHARSET_ATARI) ? "\n" : "\015\012";
	html_closer = html_doctype >= HTML_DOCTYPE_XSTRICT ? " />" : ">";
	html_name_attr = html_doctype >= HTML_DOCTYPE_XSTRICT ? "id" : "name";
	
	ret = TRUE;
	
	if (output_charset == HYP_CHARSET_ATARI)
	{
		hyp_utf8_fprintf(opts->errorfile, _("warning: writing html output in atari encoding might not work with non-atari browsers\n"));
	}
			
	if (opts->read_images && hyp->cache == NULL)
		InitCache(hyp);
	
	/* load REF if not done already */
	syms = ref_loadsyms(hyp);
	
#if 0
	create_patterns();
#endif
	
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= stg_check_links(hyp, opts, node, &syms);
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
			ret &= html_out_node(hyp, opts, node, syms, FALSE);
			break;
		case HYP_NODE_POPUP:
			ret &= html_out_node(hyp, opts, node, syms, FALSE);
			break;
		case HYP_NODE_IMAGE:
			if (opts->read_images)
				ret &= write_image(hyp, outfile, opts, node, HTML_DEFAULT_PIC_TYPE);
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
	
#ifdef CMDLINE_VERSION
	ClearCache(hyp);
#endif
	
	{
		symtab_entry *sym;
		
		for (sym = syms; sym != NULL; sym = sym->next)
		{
			if (!sym->referenced && sym->type != REF_NODENAME)
			{
				hyp_utf8_fprintf_charset(outfile, output_charset, "<!-- symbol unused: \"%s\" \"%s\" -->%s", sym->nodename, sym->name, stg_nl);
			}
		}
	}
		
	free_symtab(syms);
	return ret;
}

/*
      var img = new Image();
      img.src = 'https://mdn.mozillademos.org/files/222/Canvas_createpattern.png';
      img.onload = function() {
        ctx.fillStyle = ctx.createPattern(img, 'repeat');
        ctx.fill();
        if (fillstyle != 8)
        {
          ctx.stroke();
        }
      };
      return;
*/
