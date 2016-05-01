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

static char *html_referer_url;
static const char *cgi_scriptname = "hypview.cgi";
static char const html_nav_load_href[] = "index.php";
static char const hypview_css_name[] = "_hypview.css";
static char const hypview_js_name[] = "_hypview.js";
static char const html_view_rsc_href[] = "rscview.cgi";

static int html_doctype = HTML_DOCTYPE_XSTRICT;
static const char *html_closer = " />";
static const char *html_name_attr = "id";

#include "htmljs.h"

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
static char const html_error_note_style[] = "hypview_error_note";
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
static char const html_image_style[] = "hypview_image";
static char const html_limage_style[] = "hypview_limage";


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

static void html_out_globals(HYP_DOCUMENT *hyp, GString *out)
{
	char *str;

	hyp_utf8_sprintf_charset(out, output_charset, "<!-- @os %s -->\n", hyp_osname(hyp->comp_os));
	hyp_utf8_sprintf_charset(out, output_charset, "<!-- @charset %s -->\n", hyp_charset_name(hyp->comp_charset));
	
	if (hyp->database != NULL)
	{
		str = html_quote_name(hyp->database, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @database \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			str = html_quote_name(h->name, FALSE);
			hyp_utf8_sprintf_charset(out, output_charset, "<!-- @hostname \"%s\" -->\n", str);
			g_free(str);
		}
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = html_quote_nodename(hyp, hyp->default_page);
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @default \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		str = html_quote_name(hyp->hcp_options, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @options \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->author != NULL)
	{
		str = html_quote_name(hyp->author, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @author \"%s\" -->\n", str);
		g_free(str);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = html_quote_nodename(hyp, hyp->help_page);
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @help \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		str = html_quote_name(hyp->version, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @$VER: %s -->\n", str);
		g_free(str);
	}
	if (hyp->subject != NULL)
	{
		str = html_quote_name(hyp->subject, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @subject \"%s\" -->\n", str);
		g_free(str);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_sprintf_charset(out, output_charset, "<!-- @width %d -->\n", hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_sprintf_charset(out, output_charset, _("<!-- ST-Guide flags: $%04x -->\n"), hyp->st_guide_flags);
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_str(GString *out, HYP_CHARSET charset, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(charset, output_charset, str, len, &converror);
	p = html_quote_name(dst, FALSE);
	g_string_append(out, p);
	g_free(p);
	g_free(dst);
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_attr(GString *out, unsigned char oldattr, unsigned char newattr)
{
	if (oldattr != newattr)
	{
#define on(mask, style) \
		if (!(oldattr & mask) && (newattr & mask)) \
			g_string_append_printf(out, "<span class=\"%s\">", style)
#define off(mask, str) \
		if ((oldattr & mask) && !(newattr & mask)) \
			g_string_append(out, str)
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

static void html_out_gfx(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, int *gfx_id)
{
	char *id;
	
	switch (gfx->type)
	{
	case HYP_ESC_PIC:
		{
			char *fname;
			char *quoted;
			char *alt;
			if (!hypnode_valid(hyp, gfx->extern_node_index))
			{
				fname = hyp_invalid_page(gfx->extern_node_index);
				alt = html_quote_name(fname, TRUE);
			} else if (hyp->indextable[gfx->extern_node_index]->type != HYP_NODE_IMAGE)
			{
				fname = g_strdup_printf(_("<non-image node #%u>"), gfx->extern_node_index);
				alt = html_quote_name(fname, TRUE);
			} else if (opts->for_cgi)
			{
				if (opts->read_images)
				{
					if (opts->cgi_cached)
						fname = g_strdup_printf("%s?url=%s&index=%u&cached=1", cgi_scriptname, html_referer_url, gfx->extern_node_index);
					else
						fname = g_strdup_printf("%s?url=%s&index=%u", cgi_scriptname, html_referer_url, gfx->extern_node_index);
					alt = g_strdup_printf("index=%u", gfx->extern_node_index);
				} else
				{
					fname = g_strdup("");
					alt = html_quote_name(fname, TRUE);
				}
			} else
			{
				fname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix);
				alt = html_quote_name(fname, TRUE);
			}
			quoted = html_quote_name(fname, TRUE);
			if (gfx->islimage)
			{
				hyp_utf8_sprintf_charset(out, output_charset, "<div class=\"%s\" align=\"center\"><img src=\"%s\" alt=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0\"%s</div>\n",
					html_limage_style,
					quoted,
					alt,
					gfx->pixwidth,
					gfx->pixheight,
					html_closer);
			} else
			{
				hyp_utf8_sprintf_charset(out, output_charset, "<div class=\"%s\" style=\"position:fixed; left:%dch;\"><img src=\"%s\" alt=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0\"%s</div>\n",
					html_image_style,
					gfx->x_offset > 0 ? gfx->x_offset - 1 : 0,
					quoted,
					alt,
					gfx->pixwidth,
					gfx->pixheight,
					html_closer);
			}
			g_free(quoted);
			g_free(alt);
			g_free(fname);
		}
		break;
	case HYP_ESC_LINE:
		++(*gfx_id);
		id = g_strdup_printf("hypview_gfx_%d", *gfx_id);
		g_string_append_printf(out,
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
		g_string_append_printf(out,
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

static void html_out_graphics(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, long lineno, int *gfx_id)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			html_out_gfx(opts, out, hyp, gfx, gfx_id);
		}
		gfx = gfx->next;
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_labels(GString *out, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms)
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
			hyp_utf8_sprintf_charset(out, output_charset, "<!-- lineno %u --><a %s=\"%s\"></a>", sym->lineno, html_name_attr, str);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void html_out_alias(GString *out, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = html_quote_name(sym->name, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<a %s=\"%s\"></a>", html_name_attr, str);
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

static char *html_cgi_params(hcp_opts *opts)
{
	return g_strconcat(
		opts->hidemenu ? "&hidemenu=1" : "",
		opts->hideimages ? "&hideimages=1" : "",
		"&charset=", hyp_charset_name(output_charset),
		opts->cgi_cached ? "&cached=1" : "",
		NULL);
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
	 * as they contain nodenames of external files
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
		char *p;
		
		if (opts->for_cgi)
		{
			char *params = html_cgi_params(opts);
			char *tmp = g_strdup_printf("%s?url=%s%s&index=%u", cgi_scriptname, html_referer_url, params, node);
			p = html_quote_name(tmp, FALSE);
			g_free(tmp);
			g_free(params);
		} else
		{
			p = html_quote_name(filename, entry->type == HYP_NODE_EXTERNAL_REF);
		}
		g_free(filename);
		filename = p;
	}
	return filename;	
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_stylesheet(hcp_opts *opts, GString *outstr, gboolean do_inline)
{
	GString *out;
	FILE *outfp = NULL;
	
	if (do_inline)
	{
		out = outstr;
		g_string_append(out, "<style type=\"text/css\">\n");
	} else
	{
		char *fname;
		int exists;
		struct stat st;
		
		if (html_doctype >= HTML_DOCTYPE_XSTRICT)
			g_string_append_printf(outstr, "<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\"%s\n", hypview_css_name, html_closer);
		else
			g_string_append_printf(outstr, "<style type=\"text/css\">@import url(\"%s\");</style>\n", hypview_css_name);
		if (opts->for_cgi)
			return TRUE;
		fname = g_build_filename(opts->output_dir, hypview_css_name, NULL);
		exists = hyp_utf8_stat(fname, &st) == 0;
		if (exists)
		{
			g_free(fname);
			return TRUE;
		}
		outfp = hyp_utf8_fopen(fname, "wb");
		if (outfp == NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, fname, hyp_utf8_strerror(errno));
			g_free(fname);
			return FALSE;
		}
		if (opts->verbose >= 2 && opts->outfile != stdout)
			hyp_utf8_fprintf(stdout, _("writing %s\n"), hypview_css_name);
		g_free(fname);
		out = g_string_sized_new(5000);
	}
	
	g_string_append(out, "body {\n");
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "}");
	
	g_string_append(out, "/* style used to display links to internal nodes */\n");
	g_string_append(out, "a:link, a:visited {\n");
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.link_effect & HYP_TXT_OUTLINED ? gl_profile.colors.background : gl_profile.colors.link);
	if (gl_profile.colors.link_effect & HYP_TXT_UNDERLINED)
		g_string_append(out, "  text-decoration: underline;\n");
#if 0
	/* The text-outline property is not supported in any of the major browsers. sigh. */
	if (gl_profile.colors.link_effect & HYP_TXT_OUTLINED)
		g_string_append(out, "  text-outline: 2px 2px %s;\n", gl_profile.colors.link);
#else
	if (gl_profile.colors.link_effect & HYP_TXT_OUTLINED)
		g_string_append_printf(out, "  text-shadow: -1px -1px 0 %s, 1px -1px 0 %s, -1px 1px 0 %s, 1px 1px 0 %s;\n",
			gl_profile.colors.link, gl_profile.colors.link, gl_profile.colors.link, gl_profile.colors.link);
#endif
	if (gl_profile.colors.link_effect & HYP_TXT_ITALIC)
		g_string_append(out, "  font-style: italic;\n");
	if (gl_profile.colors.link_effect & HYP_TXT_BOLD)
		g_string_append(out, "  font-weight: bold;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to popup nodes */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_popup_link_style, html_popup_link_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.popup);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to external pages */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_xref_link_style, html_xref_link_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.xref);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to system commands */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_system_link_style, html_system_link_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.system);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to rx commands */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_rx_link_style, html_rx_link_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.rx);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to rxs commands */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_rxs_link_style, html_rxs_link_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.rxs);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to 'quit' */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_quit_link_style, html_quit_link_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.quit);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to 'close' */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_close_link_style, html_close_link_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.close);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display links to illegal pages */\n");
	g_string_append_printf(out, "a:link.%s, a:visited.%s {\n", html_error_link_style, html_error_link_style);
	g_string_append_printf(out, "  color: %s;\n", "#ff0000");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display outlined text @{O} */\n");
	g_string_append_printf(out, ".%s {\n", html_attr_outlined_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  text-shadow: -1px -1px 0 %s, 1px -1px 0 %s, -1px 1px 0 %s, 1px 1px 0 %s;\n",
			gl_profile.colors.text, gl_profile.colors.text, gl_profile.colors.text, gl_profile.colors.text);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display italic text @{I} */\n");
	g_string_append_printf(out, ".%s {\n", html_attr_italic_style);
	g_string_append(out, "  font-style: italic;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display bold text @{B} */\n");
	g_string_append_printf(out, ".%s {\n", html_attr_bold_style);
	g_string_append(out, "  font-weight: bold;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display underlined text @{U} */\n");
	g_string_append_printf(out, ".%s {\n", html_attr_underlined_style);
	g_string_append(out, "  text-decoration: underline;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display shadowed text @{S} */\n");
	g_string_append_printf(out, ".%s {\n", html_attr_shadowed_style);
	g_string_append_printf(out, "  text-shadow: 1px 1px 0 %s;\n", gl_profile.colors.text);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used to display ghosted text @{G} */\n");
	g_string_append_printf(out, ".%s {\n", html_attr_light_style);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.ghosted);
	g_string_append(out, "}\n");
	
	g_string_append(out, "ul {\n");
	g_string_append(out, "  list-style-type: none;\n");
	g_string_append(out, "  margin: 0;\n");
	g_string_append(out, "  padding: 0;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the navigation toolbar */\n");
	g_string_append_printf(out, ".%s {\n", html_toolbar_style);
	g_string_append(out, "  position:fixed;\n");
	g_string_append(out, "  top:0;\n");
	g_string_append(out, "  width:100%;\n");
	g_string_append(out, "  height:28px;\n");
	g_string_append(out, "  overflow:hidden;\n");
	g_string_append(out, "  z-index:2;\n");
	g_string_append(out, "  margin:0;\n");
	g_string_append(out, "  padding:0;\n");
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for images in the navigation toolbar */\n");
	g_string_append_printf(out, ".%s {\n", html_nav_img_style);
	g_string_append(out, "  display:block;\n");
	g_string_append(out, "  border:0;\n");
	g_string_append(out, "  width:40px;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for disabled images in the navigation toolbar */\n");
	g_string_append_printf(out, ".%s_disabled {\n", html_nav_img_style);
	g_string_append(out, "  display:block;\n");
	g_string_append(out, "  border:0;\n");
	g_string_append(out, "  width:40px;\n");
	g_string_append(out, "  opacity: 0.4;\n");
	g_string_append(out, "  /* filter: alpha(opacity=40); For IE8 and earlier */\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "li {\n");
	g_string_append(out, "  float: left;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the body of regular nodes */\n");
	g_string_append_printf(out, ".%s {\n", html_node_style);
	g_string_append(out, "  z-index:1;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the body of popup nodes */\n");
	g_string_append_printf(out, ".%s {\n", html_pnode_style);
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "  z-index:2;\n");
	g_string_append(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the body of popup nodes */\n");
	g_string_append_printf(out, ".%s {\n", html_dropdown_pnode_style);
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "  z-index:3;\n");
	g_string_append(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);\n");
	g_string_append(out, "  border: solid 1px;\n");
	g_string_append(out, "  display:none;\n");
	g_string_append(out, "  position:relative;\n");
	g_string_append(out, "  padding: 3px 6px;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the body of the file info */\n");
	g_string_append_printf(out, ".%s {\n", html_dropdown_info_style);
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "  z-index:4;\n");
	g_string_append(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);\n");
	g_string_append(out, "  border: solid 1px;\n");
	g_string_append(out, "  display:none;\n");
	g_string_append(out, "  position:fixed;\n");
	g_string_append(out, "  top: 34px;\n");
	g_string_append(out, "  padding: 3px 6px;\n");
	g_string_append(out, "}\n");

	g_string_append(out, "/* style used for error messages */\n");
	g_string_append_printf(out, ".%s {\n", html_error_note_style);
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the outer element of popups */\n");
	g_string_append_printf(out, ".%s {\n", html_dropdown_style);
	g_string_append(out, "  position:relative;\n");
	g_string_append(out, "  display:inline;\n");
	g_string_append(out, "}\n");
	
	g_string_append_printf(out, ".%s:hover .%s {\n", html_dropdown_style, html_dropdown_pnode_style);
	g_string_append(out, "  display:block;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for @line and @box elements */\n");
	g_string_append_printf(out, ".%s {\n", html_graphics_style);
	g_string_append(out, "  margin: 0;\n");
	g_string_append(out, "  position: absolute;\n");
	g_string_append(out, "  display: inline;\n");
	g_string_append(out, "  z-index:1;\n");
	g_string_append(out, "}\n");

	g_string_append(out, "/* style used for @image elements */\n");
	g_string_append_printf(out, ".%s {\n", html_image_style);
	g_string_append(out, "  margin: 0;\n");
	g_string_append(out, "  z-index:1;\n");
	g_string_append(out, "}\n");

	g_string_append(out, "/* style used for @limage elements */\n");
	g_string_append_printf(out, ".%s {\n", html_limage_style);
	g_string_append(out, "  margin: 0;\n");
	g_string_append(out, "  z-index:1;\n");
	g_string_append(out, "}\n");

	if (do_inline)
	{
		g_string_append(out, "</style>\n");
	}
	
	if (outfp)
	{
		write_strout(out, outfp);
		g_string_free(out, TRUE);
		hyp_utf8_fclose(outfp);
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_javascript(hcp_opts *opts, GString *outstr, gboolean do_inline)
{
	GString *out;
	FILE *outfp = NULL;
	const char *charset = hyp_charset_name(HYP_CHARSET_UTF8);
	
	if (do_inline)
	{
		out = outstr;
		g_string_append(out, "<script type=\"text/javascript\">\n");
		if (html_doctype >= HTML_DOCTYPE_XSTRICT)
			g_string_append(out, "//<![CDATA[\n");
	} else
	{
		char *fname;
		int exists;
		struct stat st;
		
		hyp_utf8_sprintf_charset(outstr, output_charset, "<script type=\"text/javascript\" src=\"%s\" charset=\"%s\"></script>\n", hypview_js_name, charset);
		if (opts->for_cgi)
			return TRUE;
		fname = g_build_filename(opts->output_dir, hypview_js_name, NULL);
		exists = hyp_utf8_stat(fname, &st) == 0;
		if (exists)
		{
			g_free(fname);
			return TRUE;
		}
		outfp = hyp_utf8_fopen(fname, "wb");
		if (outfp == NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, fname, hyp_utf8_strerror(errno));
			g_free(fname);
			return FALSE;
		}
		if (opts->verbose >= 2 && opts->outfile != stdout)
			hyp_utf8_fprintf(stdout, _("writing %s\n"), hypview_js_name);
		g_free(fname);
		out = g_string_sized_new(sizeof(html_javascript_code));
	}
	
	g_string_append(out, html_javascript_code);

	if (do_inline)
	{
		if (html_doctype >= HTML_DOCTYPE_XSTRICT)
			g_string_append(out, "//]]>\n");
		g_string_append(out, "</script>\n");
	}
	
	if (outfp)
	{
		write_strout(out, outfp);
		g_string_free(out, TRUE);
		hyp_utf8_fclose(outfp);
	}
		
	return TRUE;
}
	
/* ------------------------------------------------------------------------- */

static void html_out_nav_toolbar(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, struct html_xref *xrefs)
{
	INDEX_ENTRY *entry = hyp->indextable[node];
	char *str;
	char *title;
	const char *alt;
	const char *disabled;
	const char *void_href = "javascript:void(0);";
	
	g_string_append_printf(out, "<div class=\"%s\">\n", html_toolbar_style);
	
	g_string_append(out, "<form action=\"hypview.cgi\" method=\"get\">\n");
	g_string_append(out, "<fieldset style=\"border:0;margin-left:0;margin-right:0;padding-top:0;padding-bottom:0;padding-left:0;padding-right:0;\">\n");
	g_string_append(out, "<legend></legend>\n");
	g_string_append(out, "<ul>\n");
	alt = _("Back");
	g_string_append_printf(out,
		"<li style=\"position:absolute;left:0px;\">"
		"<a href=\"javascript: window.history.go(-1)\" class=\"%s\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		html_nav_img_style, html_nav_back_png, alt, alt, html_nav_dimensions, html_closer);
	
	if (hypnode_valid(hyp, entry->previous) &&
		node != entry->previous)
	{
		str = html_filename_for_node(hyp, opts, entry->previous, TRUE);
		title = html_quote_nodename(hyp, entry->previous);
		disabled = "";
	} else
	{
		title = g_strdup(_("Previous page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, output_charset,
		"<li style=\"position:absolute;left:40px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"p\" rel=\"prev\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		str, html_nav_img_style, disabled, html_nav_prev_png, title, title, html_nav_dimensions, html_closer);
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
		title = g_strdup(_("Up"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, output_charset,
		"<li style=\"position:absolute;left:80px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"t\" rel=\"up\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		str, html_nav_img_style, disabled, html_nav_toc_png, title, title, html_nav_dimensions, html_closer);
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
		title = g_strdup(_("Next page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, output_charset,
		"<li style=\"position:absolute;left:120px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"n\" rel=\"next\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		str, html_nav_img_style, disabled, html_nav_next_png, title, title, html_nav_dimensions, html_closer);
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
		title = g_strdup(_("Index page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, output_charset,
		"<li style=\"position:absolute;left:160px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"x\" rel=\"index\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		str, html_nav_img_style, disabled, html_nav_index_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);

	if (xrefs != NULL)
	{
		title = g_strdup(_("Cross references"));
		str = g_strdup(void_href);
		disabled = "";
	} else
	{
		title = g_strdup(_("Cross references"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, output_charset,
		"<li style=\"position:absolute;left:200px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"c\" rel=\"bookmark\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		str, html_nav_img_style, disabled, html_nav_xref_png, title, title, html_nav_dimensions, html_closer);
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
		title = g_strdup(_("Help page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, output_charset,
		"<li style=\"position:absolute;left:240px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"h\" rel=\"help\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		str, html_nav_img_style, disabled, html_nav_help_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);

	alt = _("Show info about hypertext");
	str = g_strdup(void_href);
	disabled = "";
	hyp_utf8_sprintf_charset(out, output_charset,
		"<li style=\"position:absolute;left:280px;\">"
		"<a href=\"%s\" class=\"%s%s\" onclick=\"showInfo()\" accesskey=\"i\" rel=\"copyright\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		str, html_nav_img_style, disabled, html_nav_info_png, alt, alt, html_nav_dimensions, html_closer);
	g_free(str);

	if (opts->for_cgi)
	{
		char *str;
		
		alt = _("Load a file");
		disabled = "";
		g_string_append_printf(out,
			"<li style=\"position:absolute;left:340px;\">"
			"<a href=\"%s\" class=\"%s%s\" accesskey=\"o\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
			"</li>\n",
			html_nav_load_href, html_nav_img_style, disabled, html_nav_load_png, alt, alt, html_nav_dimensions, html_closer);
	
		g_string_append(out, "<li style=\"position:absolute;left:380px;\">\n");
		str = html_quote_name(html_referer_url, FALSE);
		g_string_append_printf(out, "<input type=\"hidden\" name=\"url\" value=\"%s\"%s\n", str, html_closer);
		g_free(str);
		g_string_append_printf(out, "<input accesskey=\"s\" type=\"text\" name=\"q\" size=\"10\" value=\"\"%s\n", html_closer);
		g_string_append(out, "</li>\n");
	}
	g_string_append(out, "</ul>\n");
	g_string_append(out, "</fieldset>\n");
	g_string_append(out, "</form>\n");

	g_string_append(out, "</div>\n");
}

/* ------------------------------------------------------------------------- */

static void html_out_header(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const char *title, hyp_nodenr node, struct hyp_gfx *hyp_gfx, struct html_xref *xrefs, gboolean for_error)
{
	const char *charset = hyp_charset_name(output_charset);
	INDEX_ENTRY *entry = hyp ? hyp->indextable[node] : NULL;
	char *str;
	
	switch (html_doctype)
	{
	case HTML_DOCTYPE_OLD:
		g_string_append(out, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n");
		g_string_append(out, "<html lang=\"en\">\n");
		break;
	case HTML_DOCTYPE_TRANS:
		g_string_append(out, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n");
		g_string_append(out, "        \"http://www.w3.org/TR/html4/loose.dtd\">\n");
		g_string_append(out, "<html lang=\"en\">\n");
		break;
	
	case HTML_DOCTYPE_XSTRICT:
#if 0
		g_string_append(out, "<?xml version=\"1.0\" encoding=\"%s\"?>\n", charset);
#endif
		g_string_append(out, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n");
		g_string_append(out, "          \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
		g_string_append(out, "<html xml:lang=\"en\" lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\"");
		if (hyp_gfx != NULL)
			g_string_append(out, " xmlns:svg=\"http://www.w3.org/2000/svg\"");
		g_string_append(out, ">\n");
		break;
	case HTML_DOCTYPE_STRICT:
	default:
		g_string_append(out, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n");
		g_string_append(out, "          \"http://www.w3.org/TR/html4/strict.dtd\">\n");
		g_string_append(out, "<html lang=\"en\">\n");
		break;
	case HTML_DOCTYPE_HTML5:
		g_string_append(out, "<!DOCTYPE html>\n");
		g_string_append(out, "<html xml:lang=\"en\" lang=\"en\">\n");
		break;
	case HTML_DOCTYPE_FRAME:
	case HTML_DOCTYPE_XFRAME:
		abort();
		break;
	}
	g_string_append_printf(out, "<!-- This file was automatically generated by %s version %s -->\n", gl_program_name, gl_program_version);
	hyp_utf8_sprintf_charset(out, output_charset, "<!-- %s -->\n", HYP_COPYRIGHT);
	if (hyp != NULL && node == 0)
		html_out_globals(hyp, out);
	if (node != HYP_NOINDEX)
		hyp_utf8_sprintf_charset(out, output_charset, _("<!-- Node #%u -->\n"), node);
	g_string_append(out, "<head>\n");
	if (html_doctype >= HTML_DOCTYPE_HTML5)
		g_string_append_printf(out, "<meta charset=\"%s\"%s\n", charset, html_closer);
	else
		g_string_append_printf(out, "<meta http-equiv=\"content-type\" content=\"text/html;charset=%s\"%s\n", charset, html_closer);
	g_string_append_printf(out, "<meta name=\"GENERATOR\" content=\"%s %s\"%s\n", gl_program_name, gl_program_version, html_closer);
	if (hyp && hyp->author != NULL)
	{
		char *str = html_quote_name(hyp->author, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<meta name=\"author\" content=\"%s\"%s\n", str, html_closer);
		g_free(str);
	}
	if (hyp && hyp->database != NULL)
	{
		char *str = html_quote_name(hyp->database, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, "<meta name=\"description\" content=\"%s\"%s\n", str, html_closer);
		g_free(str);
	}
	if (title)
		hyp_utf8_sprintf_charset(out, output_charset, "<title>%s</title>\n", title);

	if (entry && entry->type == HYP_NODE_INTERNAL)
	{
		if (hypnode_valid(hyp, hyp->first_text_page) &&
			node != hyp->first_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->first_text_page, TRUE);
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"%s\"%s\n", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "start" : "first", html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->previous) &&
			node != entry->previous)
		{
			str = html_filename_for_node(hyp, opts, entry->previous, TRUE);
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"prev\"%s\n", str, html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->toc_index) &&
			node != entry->toc_index)
		{
			str = html_filename_for_node(hyp, opts, entry->toc_index, TRUE);
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"up\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, entry->next) &&
			node != entry->next)
		{
			str = html_filename_for_node(hyp, opts, entry->next, TRUE);
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"next\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->last_text_page) &&
			node != hyp->last_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->last_text_page, TRUE);
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"%s\"%s\n", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "end" : "last", html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, hyp->index_page) &&
			node != hyp->index_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->index_page, TRUE);
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"index\"%s\n", str, html_closer);
			g_free(str);
		}

		if (xrefs != NULL)
		{
			str = g_strdup("");
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"bookmark\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->help_page) &&
			node != hyp->help_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->help_page, TRUE);
			hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"help\"%s\n", str, html_closer);
			g_free(str);
		}

		str = g_strdup("javascript: showInfo();");
		hyp_utf8_sprintf_charset(out, output_charset, "<link href=\"%s\" rel=\"copyright\"%s\n", str, html_closer);
		g_free(str);
	}
	
	html_out_stylesheet(opts, out, FALSE);
	html_out_javascript(opts, out, FALSE);
	
	if (html_doctype >= HTML_DOCTYPE_HTML5)
	{
		g_string_append(out, "<!--[if lt IE 9]>\n");
		g_string_append(out, "<script src=\"http://html5shiv.googlecode.com/svn/trunk/html5.js\" type=\"text/javascript\"></script>\n");
		g_string_append(out, "<![endif]-->\n");
	}

	g_string_append(out, "</head>\n");
	g_string_append(out, "<body>\n");
	if (hyp)
		g_string_append_printf(out, "<div style=\"width:%dex;\">\n", hyp ? hyp->line_width : HYP_STGUIDE_DEFAULT_LINEWIDTH);

	if (for_error)
	{
		g_string_append_printf(out, "<div class=\"%s\">\n", html_error_note_style);
		g_string_append(out, "<p>\n");
	} else if (entry != NULL && entry->type == HYP_NODE_INTERNAL)
	{
		if (opts->hidemenu)
		{
			g_string_append_printf(out, "<div class=\"%s\">\n", html_node_style);
		} else
		{
			html_out_nav_toolbar(hyp, opts, out, node, xrefs);
			g_string_append_printf(out, "<div class=\"%s\" style=\"position:absolute; top:32px;\">\n", html_node_style);
		}
		g_string_append(out, "<pre>\n");
	} else
	{
		g_string_append_printf(out, "<div class=\"%s\">\n", html_pnode_style);
		g_string_append(out, "<pre>\n");
	}

	if (hyp)
	{
		/*
		 * this element is displayed for "About"
		 */
		hyp_utf8_sprintf_charset(out, output_charset, "<span class=\"%s\">", html_dropdown_style);
		hyp_utf8_sprintf_charset(out, output_charset, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_info_style, html_hyp_info_id);
		str = html_quote_name(hyp->database, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, _("Topic       : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->author, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, _("Author      : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->version, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, _("Version     : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->subject, FALSE);
		hyp_utf8_sprintf_charset(out, output_charset, _("Subject     : %s\n"), fixnull(str));
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
		hyp_utf8_sprintf_charset(out, output_charset, "%s\n", str);
		g_free(str);
		hyp_utf8_sprintf_charset(out, output_charset, "</span></span>");
		{
			HYP_HOSTNAME *h;
			for (h = hyp->hostname; h != NULL; h = h->next)
			{
				str = html_quote_name(h->name, FALSE);
				hyp_utf8_sprintf_charset(out, output_charset, _("@hostname   : %s\n"), str);
				g_free(str);
			}
		}
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_trailer(GString *out, gboolean for_error)
{
	if (for_error)
	{
		g_string_append(out, "</p>\n");
		g_string_append(out, "</div>\n");
	} else
	{
		g_string_append(out, "</pre>\n");
		g_string_append(out, "</div>\n");
		g_string_append(out, "</div>\n");
	}
	g_string_append(out, "</body>\n");
	g_string_append(out, "</html>\n");
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, symtab_entry *syms, gboolean for_inline)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	unsigned char textattr;
	long lineno;
	struct hyp_gfx *hyp_gfx = NULL;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	int popup_id = 0;
	int gfx_id = 0;
	FILE *outfp = NULL;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		html_out_str(out, hyp->comp_charset, textstart, src - textstart); \
		at_bol = FALSE; \
	}
#define FLUSHLINE() \
	if (!at_bol) \
	{ \
		g_string_append_c(out, '\n'); \
		at_bol = TRUE; \
	}
#define FLUSHTREE() \
	if (in_tree != -1) \
	{ \
		hyp_utf8_sprintf_charset(out, output_charset, "end tree %d -->\n", in_tree); \
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
		
		if (!for_inline && opts->outfile != stdout && node != 0)
		{
			char *name = html_filename_for_node(hyp, opts, node, FALSE);
			char *output_filename;
			
			/*
			 * the first file was alreday created in the main loop,
			 * create the output file for the next node
			 */
			output_filename = g_build_filename(opts->output_dir, name, NULL);
			outfp = hyp_utf8_fopen(output_filename, "wb");
			if (outfp == NULL)
			{
				hyp_node_free(nodeptr);
				hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, output_filename, hyp_utf8_strerror(errno));
				g_free(output_filename);
				g_free(name);
				return FALSE;
			}
			if (opts->verbose >= 2 && opts->outfile != stdout)
				hyp_utf8_fprintf(stdout, _("writing %s\n"), name);
			g_free(output_filename);
			g_free(name);
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
			html_out_header(hyp, opts, out, title, node, hyp_gfx, xrefs, FALSE);

		g_free(title);
		}
		
		/*
		 * check for alias names in ref file
		 */
		html_out_alias(out, hyp, entry, syms);
		
		/*
		 * now output data
		 */
		src = nodeptr->start;
		textstart = src;
		at_bol = TRUE;
		in_tree = -1;
		textattr = 0;
		lineno = 0;
		html_out_labels(out, hyp, entry, lineno, syms);
		html_out_graphics(opts, out, hyp, hyp_gfx, lineno, &gfx_id);
		
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
					g_string_append(out, "&#x1b;");
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
							hyp_utf8_sprintf_charset(out, output_charset, _("<a class=\"%s\" href=\"%s\">%s</a>"), html_error_link_style, destfilename, destname);
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
										hyp_utf8_sprintf_charset(out, output_charset, "<a%s href=\"%s&file=%s&tree=%u\">%s></a>", style, html_view_rsc_href, destname, line, str);
									} else if (hyp_guess_filetype(destname) == HYP_FT_HYP)
									{
										is_xref = TRUE;
										if (opts->for_cgi)
										{
											char *dir = hyp_path_get_dirname(html_referer_url);
											char *base = g_strdup(hyp_basename(destname));
											char *ref, *quoted;
											char *params = html_cgi_params(opts);
											ref = g_strconcat(dir, *dir ? "/" : "", base, params, NULL);
											quoted = html_quote_name(ref, FALSE);
											hyp_utf8_sprintf_charset(out, output_charset, "<a%s href=\"%s?url=%s\">%s></a>", style, cgi_scriptname, quoted, str);
											g_free(quoted);
											g_free(ref);
											g_free(params);
											g_free(base);
											g_free(dir);
										} else
										{
											char *base = replace_ext(hyp_basename(destname), HYP_EXT_HYP, "");
											char *quoted = html_quote_name(base, FALSE);
											html_convert_filename(base);
											hyp_utf8_sprintf_charset(out, output_charset, "<a%s href=\"../%s/%s.html\">%s></a>", style, quoted, quoted, str);
											g_free(quoted);
											g_free(base);
										}
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
									hyp_utf8_sprintf_charset(out, output_charset, "<a%s href=\"%s#%s\"%s>%s</a>", style, destfilename, quoted, target, str);
									g_free(quoted);
								} else if (desttype == HYP_NODE_POPUP && *target == '\0')
								{
									char *id;
									GString *tmp;
									
									++popup_id;
									id = g_strdup_printf("hypview_popup_%d", popup_id);
									hyp_utf8_sprintf_charset(out, output_charset, "<span class=\"%s\">", html_dropdown_style);
									hyp_utf8_sprintf_charset(out, output_charset, "<a%s id=\"%s_btn\" href=\"javascript:void(0);\" onclick=\"showPopup('%s')\">%s</a>", style, id, id, str);
									hyp_utf8_sprintf_charset(out, output_charset, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_pnode_style, id);
									
									tmp = g_string_new(NULL);
									if (html_out_node(hyp, opts, tmp, dest_page, syms, TRUE))
									{
										g_string_append_len(out, tmp->str, tmp->len);
									}
									g_string_free(tmp, TRUE);
									g_string_append(out, "</span></span>");
									g_free(id);
								} else
								{
									hyp_utf8_sprintf_charset(out, output_charset, "<a%s href=\"%s\"%s>%s</a>", style, destfilename, target, str);
								}
							}
							g_free(style);
							}
							break;
						case HYP_NODE_REXX_COMMAND:
							hyp_utf8_sprintf_charset(out, output_charset, "<a class=\"%s\" href=\"javascript:execRx('%s', '%s')\">%s</a>", html_rxs_link_style, _("Execute REXX command"), destname, str);
							break;
						case HYP_NODE_REXX_SCRIPT:
							hyp_utf8_sprintf_charset(out, output_charset, "<a class=\"%s\" href=\"javascript:execRxs('%s', '%s')\">%s</a>", html_system_link_style, _("Execute REXX script"), destname, str);
							break;
						case HYP_NODE_SYSTEM_ARGUMENT:
							hyp_utf8_sprintf_charset(out, output_charset, "<a class=\"%s\" href=\"javascript:execSystem('%s', '%s')\">%s</a>", html_rx_link_style, _("Execute"), destname, str);
							break;
						case HYP_NODE_IMAGE:
							/* that would be an inline image; currently not supported by compiler */
							hyp_utf8_sprintf_charset(out, output_charset, "<a class=\"%s\" href=\"%s\">%s</a>", html_image_link_style, destname, str);
							break;
						case HYP_NODE_QUIT:
							/* not really quit, but best we can do */
							hyp_utf8_sprintf_charset(out, output_charset, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_quit_link_style, str);
							break;
						case HYP_NODE_CLOSE:
							hyp_utf8_sprintf_charset(out, output_charset, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_close_link_style, str);
							break;
						default:
							hyp_utf8_sprintf_charset(out, output_charset, "<a class=\"%s\" href=\"%s\">%s %u</a>", html_error_link_style, destfilename, _("link to unknown node type"), hyp->indextable[dest_page]->type);
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
							hyp_utf8_sprintf_charset(out, output_charset, "<!-- begin tree %d\n", tree);
							in_tree = tree;
						}
						hyp_utf8_sprintf_charset(out, output_charset, "   %d \"%s\" %u\n", obj, str, line);
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
					if (html_out_attr(out, textattr, *src - HYP_ESC_TEXTATTR_FIRST))
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
				g_string_append_c(out, '\n');
				at_bol = TRUE;
				++lineno;
				html_out_labels(out, hyp, entry, lineno, syms);
				html_out_graphics(opts, out, hyp, hyp_gfx, lineno, &gfx_id);
				src++;
				textstart = src;
			} else
			{
				FLUSHTREE();
				src++;
			}
		}
		DUMPTEXT();
		if (html_out_attr(out, textattr, 0))
			at_bol = FALSE;
		FLUSHLINE();
		FLUSHTREE();
		++lineno;
		html_out_labels(out, hyp, entry, lineno, syms);
		html_out_graphics(opts, out, hyp, hyp_gfx, lineno, &gfx_id);
		
		if (hyp_gfx != NULL)
		{
			struct hyp_gfx *gfx, *next;
			
			for (gfx = hyp_gfx; gfx != NULL; gfx = next)
			{
				if (!gfx->used)
				{
					hyp_utf8_sprintf_charset(out, output_charset, "<!-- gfx unused: ");
					html_out_gfx(opts, out, hyp, gfx, &gfx_id);
					hyp_utf8_sprintf_charset(out, output_charset, "-->\n");
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
			html_out_trailer(out, FALSE);
		
			if (node < hyp->last_text_page && opts->outfile == stdout)
				g_string_append(out, "\n\n");
		}
		
		if (outfp)
		{
			write_strout(out, outfp);
			hyp_utf8_fclose(outfp);
		}
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


static void html_init(FILE *out)
{
	stg_nl = (out == stdout || output_charset != HYP_CHARSET_ATARI) ? "\n" : "\015\012";
	html_closer = html_doctype >= HTML_DOCTYPE_XSTRICT ? " />" : ">";
	html_name_attr = html_doctype >= HTML_DOCTYPE_XSTRICT ? "id" : "name";
}


static gboolean recompile_html(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	FILE *outfile = opts->outfile;
	GString *out;
	
	UNUSED(argc);
	UNUSED(argv);
	
	html_init(outfile);
	
	ret = TRUE;
	
	if (output_charset == HYP_CHARSET_ATARI && opts->errorfile != stdout)
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
			ret &= sym_check_links(hyp, opts, node, &syms);
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
	
	out = g_string_new(NULL);
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
			ret &= html_out_node(hyp, opts, out, node, syms, FALSE);
			break;
		case HYP_NODE_POPUP:
			ret &= html_out_node(hyp, opts, out, node, syms, FALSE);
			break;
		case HYP_NODE_IMAGE:
			if (opts->read_images)
				ret &= write_image(hyp, opts, node, HTML_DEFAULT_PIC_TYPE, NULL);
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
		if (node == 0 || opts->outfile == stdout)
			write_strout(out, opts->outfile);
		g_string_truncate(out, 0);
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
				hyp_utf8_sprintf_charset(out, output_charset, "<!-- symbol unused: \"%s\" \"%s\" -->\n", sym->nodename, sym->name);
			}
		}
		write_strout(out, opts->outfile);
	}
	
	g_string_free(out, TRUE);
	
	free_symtab(syms);
	return ret;
}
