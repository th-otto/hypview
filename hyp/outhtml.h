#include "base64.h"

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
static char const html_hyp_xrefs_id[] = "hyp_xrefs";
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
static char const html_dropdown_xrefs_style[] = "hypview_dropdown_xrefs";
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
	char *destname;
	char *destfilename;
	char *text;	/* text to display */
	hyp_nodenr dest_page;
	hyp_indextype desttype;
	hyp_lineno line;
	struct html_xref *next;
};

#define HTML_DEFAULT_PIC_TYPE HYP_PIC_GIF
static gboolean const force_overwrite = TRUE;
static gboolean css_written = FALSE;
static gboolean js_written = FALSE;
static int html_popup_id;

#define QUOTE_CONVSLASH  0x0001
#define QUOTE_SPACE      0x0002
#define QUOTE_URI        0x0004
#define QUOTE_JS         0x0008
#define QUOTE_ALLOWUTF8  0x0010
#define QUOTE_LABEL      0x0020
#define QUOTE_UNICODE    0x0040
#define QUOTE_NOLTR      0x0080

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *html_quote_name(const char *name, unsigned int flags)
{
	char *str, *ret;
	size_t len;
	static char const hex[] = "0123456789ABCDEF";
	
	if (name == NULL)
		return NULL;
	len = strlen(name);
	str = ret = g_new(char, len * 20 + 1);
	if (str != NULL)
	{
		if (*name != '\0' && (flags & QUOTE_LABEL) && *name != '_' && !g_ascii_isalpha(*name))
			*str++ = '_';
		while (*name)
		{
			unsigned char c = *name++;
#define STR(s) strcpy(str, s), str += sizeof(s) - 1
			switch (c)
			{
			case '\\':
				if (flags & QUOTE_URI)
				{
					STR("%2F");
				} else if (flags & QUOTE_CONVSLASH)
				{
					*str++ = '/';
				} else
				{
					*str++ = '\\';
				}
				break;
			case ' ':
				if (flags & QUOTE_URI)
				{
					STR("%20");
				} else if (flags & QUOTE_SPACE)
				{
					STR("&nbsp;");
				} else if (flags & QUOTE_LABEL)
				{
					*str++ = '_';
				} else
				{
					*str++ = ' ';
				}
				break;
			case '"':
				if (flags & QUOTE_JS)
				{
					STR("\\&quot;");
				} else if (flags & QUOTE_URI)
				{
					STR("%22");
				} else
				{
					STR("&quot;");
				}
				break;
			case '&':
				if (flags & QUOTE_URI)
				{
					STR("%26");
				} else
				{
					STR("&amp;");
				}
				break;
			case '\'':
				if (flags & QUOTE_URI)
				{
					STR("%27");
				} else
				{
					STR("&apos;");
				}
				break;
			case '<':
				if (flags & QUOTE_URI)
				{
					STR("%3C");
				} else
				{
					STR("&lt;");
				}
				break;
			case '>':
				if (flags & QUOTE_URI)
				{
					STR("%3E");
				} else
				{
					STR("&gt;");
				}
				break;
			case '-':
			case '.':
			case '_':
			case '~':
				*str++ = c;
				break;
			case 0x01:
				if (flags & QUOTE_URI)
				{
					STR("%01");
				} else
				{
					STR("&soh;");
				}
				break;
			case 0x02:
				if (flags & QUOTE_URI)
				{
					STR("%02");
				} else
				{
					STR("&stx;");
				}
				break;
			case 0x03:
				if (flags & QUOTE_URI)
				{
					STR("%03");
				} else
				{
					STR("&etx;");
				}
				break;
			case 0x04:
				if (flags & QUOTE_URI)
				{
					STR("%04");
				} else
				{
					STR("&eot;");
				}
				break;
			case 0x05:
				if (flags & QUOTE_URI)
				{
					STR("%05");
				} else
				{
					STR("&enq;");
				}
				break;
			case 0x06:
				if (flags & QUOTE_URI)
				{
					STR("%06");
				} else
				{
					STR("&ack;");
				}
				break;
			case 0x07:
				if (flags & QUOTE_URI)
				{
					STR("%07");
				} else
				{
					STR("&bel;");
				}
				break;
			case 0x08:
				if (flags & QUOTE_URI)
				{
					STR("%08");
				} else
				{
					STR("&bs;");
				}
				break;
			case 0x09:
				if (flags & QUOTE_URI)
				{
					STR("%09");
				} else
				{
					STR("&ht;");
				}
				break;
			case 0x0a:
				if (flags & QUOTE_URI)
				{
					STR("%0A");
				} else
				{
					STR("&lf;");
				}
				break;
			case 0x0b:
				if (flags & QUOTE_URI)
				{
					STR("%0B");
				} else
				{
					STR("&vt;");
				}
				break;
			case 0x0c:
				if (flags & QUOTE_URI)
				{
					STR("%0C");
				} else
				{
					STR("&ff;");
				}
				break;
			case 0x0d:
				if (flags & QUOTE_URI)
				{
					STR("%0D");
				} else
				{
					STR("&cr;");
				}
				break;
			case 0x0e:
				if (flags & QUOTE_URI)
				{
					STR("%0E");
				} else
				{
					STR("&so;");
				}
				break;
			case 0x0f:
				if (flags & QUOTE_URI)
				{
					STR("%0F");
				} else
				{
					STR("&si;");
				}
				break;
			case 0x10:
				if (flags & QUOTE_URI)
				{
					STR("%10");
				} else
				{
					STR("&dle;");
				}
				break;
			case 0x11:
				if (flags & QUOTE_URI)
				{
					STR("%11");
				} else
				{
					STR("&dc1;");
				}
				break;
			case 0x12:
				if (flags & QUOTE_URI)
				{
					STR("%12");
				} else
				{
					STR("&dc2;");
				}
				break;
			case 0x13:
				if (flags & QUOTE_URI)
				{
					STR("%13");
				} else
				{
					STR("&dc3;");
				}
				break;
			case 0x14:
				if (flags & QUOTE_URI)
				{
					STR("%14");
				} else
				{
					STR("&dc4;");
				}
				break;
			case 0x15:
				if (flags & QUOTE_URI)
				{
					STR("%15");
				} else
				{
					STR("&nak;");
				}
				break;
			case 0x16:
				if (flags & QUOTE_URI)
				{
					STR("%16");
				} else
				{
					STR("&syn;");
				}
				break;
			case 0x17:
				if (flags & QUOTE_URI)
				{
					STR("%17");
				} else
				{
					STR("&etb;");
				}
				break;
			case 0x18:
				if (flags & QUOTE_URI)
				{
					STR("%18");
				} else
				{
					STR("&can;");
				}
				break;
			case 0x19:
				if (flags & QUOTE_URI)
				{
					STR("%19");
				} else
				{
					STR("&em;");
				}
				break;
			case 0x1a:
				if (flags & QUOTE_URI)
				{
					STR("%1A");
				} else
				{
					STR("&sub;");
				}
				break;
			case 0x1b:
				if (flags & QUOTE_URI)
				{
					STR("%1B");
				} else
				{
					STR("&esc;");
				}
				break;
			case 0x1c:
				if (flags & QUOTE_URI)
				{
					STR("%1C");
				} else
				{
					STR("&fs;");
				}
				break;
			case 0x1D:
				if (flags & QUOTE_URI)
				{
					STR("%1D");
				} else
				{
					STR("&gs;");
				}
				break;
			case 0x1E:
				if (flags & QUOTE_URI)
				{
					STR("%1E");
				} else
				{
					STR("&rs;");
				}
				break;
			case 0x1F:
				if (flags & QUOTE_URI)
				{
					STR("%1F");
				} else
				{
					STR("&us;");
				}
				break;
			default:
				if (c >= 0x80 && (flags & QUOTE_ALLOWUTF8))
				{
					*str++ = c;
				} else if (g_ascii_isalnum(c))
				{
					*str++ = c;
				} else if (flags & QUOTE_URI)
				{
					*str++ = '%';
					*str++ = hex[c >> 4];
					*str++ = hex[c & 0x0f];
				} else if (c >= 0x80 && (flags & QUOTE_UNICODE))
				{
					h_unichar_t wc;
					--name;
					name = hyp_utf8_getchar(name, &wc);
					/*
					 * neccessary for hebrew characters to prevent switching to rtl
					 */
					if (wc >= 0x590 && wc <= 0x5ff && !(flags & QUOTE_NOLTR))
						str += sprintf(str, "<span dir=\"ltr\">&#x%lx;</span>", (unsigned long) wc);
					else
						str += sprintf(str, "&#x%lx;", (unsigned long) wc);
				} else
				{
					*str++ = c;
				}
				break;
			}
#undef STR
		}
		*str++ = '\0';
		ret = g_renew(char, ret, str - ret);
	}
	return ret;
}

/* ------------------------------------------------------------------------- */

static char *html_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node, unsigned int flags)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *p;
	char *buf;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	buf = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
	p = html_quote_name(buf, (entry->type == HYP_NODE_EXTERNAL_REF ? QUOTE_CONVSLASH : 0) | flags);
	g_free(buf);
	return p;
}

/* ------------------------------------------------------------------------- */

static void html_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out)
{
	char *str;

	hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @os %s -->\n", hyp_osname(hyp->comp_os));
	hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @charset %s -->\n", hyp_charset_name(hyp->comp_charset));
	
	if (hyp->language != NULL)
	{
		str = html_quote_name(hyp->language, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @lang \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->database != NULL)
	{
		str = html_quote_name(hyp->database, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @database \"%s\" -->\n", str);
		g_free(str);
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = html_quote_nodename(hyp, hyp->default_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @default \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		str = html_quote_name(hyp->hcp_options, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @options \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->author != NULL)
	{
		str = html_quote_name(hyp->author, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @author \"%s\" -->\n", str);
		g_free(str);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = html_quote_nodename(hyp, hyp->help_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @help \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		str = html_quote_name(hyp->version, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @$VER: %s -->\n", str);
		g_free(str);
	}
	if (hyp->subject != NULL)
	{
		str = html_quote_name(hyp->subject, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @subject \"%s\" -->\n", str);
		g_free(str);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @width %d -->\n", hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("<!-- ST-Guide flags: $%04x -->\n"), hyp->st_guide_flags);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			str = html_quote_name(h->name, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- @hostname \"%s\" -->\n", str);
			g_free(str);
		}
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_str(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(hyp->comp_charset, opts->output_charset, str, len, &converror);
	p = html_quote_name(dst, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
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
			char *dithermask;
			char *origfname;
			char *origquoted;
			
			dithermask = format_dithermask(gfx->dithermask);
			if (!hypnode_valid(hyp, gfx->extern_node_index))
			{
				fname = hyp_invalid_page(gfx->extern_node_index);
				origfname = fname;
				alt = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			} else if (hyp->indextable[gfx->extern_node_index]->type != HYP_NODE_IMAGE)
			{
				fname = g_strdup_printf(_("<non-image node #%u>"), gfx->extern_node_index);
				origfname = fname;
				alt = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
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
					alt = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
				}
				origfname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix);
			} else
			{
				fname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix);
				origfname = fname;
				alt = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			}
			quoted = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			origquoted = html_quote_name(origfname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- %s \"%s\" %d%s%s -->",
				gfx->islimage ? "@limage" : "@image",
				origfname,
				gfx->x_offset,
				*dithermask ? " %" : "", dithermask);
			if (gfx->islimage)
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<div class=\"%s\" align=\"center\" style=\"width:%dex;\"><img src=\"%s\" alt=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0;\"%s</div>\n",
					html_limage_style,
					hyp->line_width,
					quoted,
					alt,
					gfx->pixwidth,
					gfx->pixheight,
					html_closer);
			} else
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<div class=\"%s\" style=\"position:absolute; left:%dch;\"><img src=\"%s\" alt=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0;\"%s</div>",
					html_image_style,
					gfx->x_offset > 0 ? gfx->x_offset - 1 : 0,
					quoted,
					alt,
					gfx->pixwidth,
					gfx->pixheight,
					html_closer);
			}
			g_free(origquoted);
			g_free(quoted);
			g_free(alt);
			if (origfname != fname)
				g_free(origfname);
			g_free(fname);
			g_free(dithermask);
		}
		break;
	case HYP_ESC_LINE:
		g_string_append_printf(out, "<!-- @line %d %d %d %d %d -->",
			gfx->x_offset, gfx->width, gfx->height,
			gfx->begend,
			gfx->style);
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
		g_string_append_printf(out, "<!-- %s %d %d %d %d -->",
			gfx->type == HYP_ESC_BOX ? "@box" : "@rbox",
			gfx->x_offset, gfx->width, gfx->height, gfx->style);
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

#ifdef CGI_VERSION
static void html_out_stg_gfx(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, char *fname)
{
	char *origfname;
	char *quoted;
	char *origquoted;
	
	/*
	 * the source output is embedded in a html doc
	 */
	origfname = fname;
	if (hypnode_valid(hyp, gfx->extern_node_index) &&
		hyp->indextable[gfx->extern_node_index]->type == HYP_NODE_IMAGE)
	{
		if (opts->cgi_cached)
			fname = g_strdup_printf("%s?url=%s&index=%u&cached=1", cgi_scriptname, html_referer_url, gfx->extern_node_index);
		else
			fname = g_strdup_printf("%s?url=%s&index=%u", cgi_scriptname, html_referer_url, gfx->extern_node_index);
	}
	quoted = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
	origquoted = html_quote_name(origfname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
	hyp_utf8_sprintf_charset(out, opts->output_charset, "%s \"<a href=\"%s\">%s</a>\" %d",
		gfx->islimage ? "@limage" : "@image",
		quoted,
		origquoted,
		gfx->x_offset);
	g_free(origquoted);
	g_free(quoted);
	if (fname != origfname)
		g_free(fname);
}
#endif

/* ------------------------------------------------------------------------- */

static void html_out_graphics(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, struct hyp_gfx *gfx, long lineno, int *gfx_id)
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

static void html_out_labels(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_LABELNAME);
	while (sym)
	{
		if (sym->lineno == lineno)
		{
			char *str = html_quote_name(sym->name, QUOTE_LABEL | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- lineno %u --><a %s=\"%s\"></a>", sym->lineno, html_name_attr, str);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void html_out_alias(GString *out, HYP_DOCUMENT *hyp, hcp_opts *opts, const INDEX_ENTRY *entry, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = html_quote_name(sym->name, QUOTE_LABEL | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a %s=\"%s\"></a>", html_name_attr, str);
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
		opts->hidemenu ? "&amp;hidemenu=1" : "",
		opts->hideimages ? "&amp;hideimages=1" : "",
		"&amp;charset=", hyp_charset_name(opts->output_charset),
		opts->cgi_cached ? "&amp;cached=1" : "",
		opts->showstg ? "&amp;showstg=1" : "",
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
	if (node == hyp->main_page)
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
			filename = g_strconcat(name, HYP_EXT_HTML, NULL);
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
	 * with "index.html", unless it is the output
	 * filename specified at command line
	 */
	if (g_ascii_strcasecmp(filename, "index" HYP_EXT_HTML) == 0 && node != hyp->main_page)
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
			char *tmp = html_quote_name(html_referer_url, QUOTE_URI);
			p = g_strdup_printf("%s?url=%s%s&amp;index=%u", cgi_scriptname, tmp, params, node);
			g_free(tmp);
			g_free(params);
		} else
		{
			p = html_quote_name(filename, (entry->type == HYP_NODE_EXTERNAL_REF ? QUOTE_CONVSLASH : 0) | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
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
		if (opts->for_cgi || css_written)
			return TRUE;
		fname = g_build_filename(opts->output_dir, hypview_css_name, NULL);
		exists = hyp_utf8_stat(fname, &st) == 0;
		if (exists && !force_overwrite)
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
		css_written = TRUE;
		g_free(fname);
		out = g_string_sized_new(5000);
		g_string_append_printf(out, "/* This file was automatically generated by %s version %s */\n\n", gl_program_name, gl_program_version);
	}
	
	g_string_append(out, "body {\n");
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "}\n");
	
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
	g_string_append(out, "  text-decoration: none;\n");
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
	g_string_append(out, "  z-index:3;\n");
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
	g_string_append(out, "  z-index:2;\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the body of popup nodes */\n");
	g_string_append_printf(out, ".%s {\n", html_pnode_style);
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "  z-index:3;\n");
	g_string_append(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);\n");
	g_string_append(out, "}\n");
	
	g_string_append(out, "/* style used for the body of popup nodes */\n");
	g_string_append_printf(out, ".%s {\n", html_dropdown_pnode_style);
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "  z-index:4;\n");
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
	g_string_append(out, "  z-index:5;\n");
	g_string_append(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);\n");
	g_string_append(out, "  border: solid 1px;\n");
	g_string_append(out, "  display:none;\n");
	g_string_append(out, "  position:fixed;\n");
	g_string_append(out, "  top: 34px;\n");
	g_string_append(out, "  padding: 3px 6px;\n");
	g_string_append(out, "}\n");

	g_string_append(out, "/* style used for the body of the cross references dropdown */\n");
	g_string_append_printf(out, ".%s {\n", html_dropdown_xrefs_style);
	g_string_append_printf(out, "  background-color: %s;\n", gl_profile.colors.background);
	g_string_append_printf(out, "  color: %s;\n", gl_profile.colors.text);
	g_string_append(out, "  z-index:5;\n");
	g_string_append(out, "  box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);\n");
	g_string_append(out, "  border: solid 1px;\n");
	g_string_append(out, "  display:none;\n");
	g_string_append(out, "  position:fixed;left:200px;\n");
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
	g_string_append(out, "  z-index:-1;\n");
	g_string_append(out, "}\n");

	g_string_append(out, "/* style used for @image elements */\n");
	g_string_append_printf(out, ".%s {\n", html_image_style);
	g_string_append(out, "  margin: 0;\n");
	g_string_append(out, "  z-index:-1;\n");
	g_string_append(out, "}\n");

	g_string_append(out, "/* style used for @limage elements */\n");
	g_string_append_printf(out, ".%s {\n", html_limage_style);
	g_string_append(out, "  margin: 0;\n");
	g_string_append(out, "  z-index:-1;\n");
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
		
		hyp_utf8_sprintf_charset(outstr, opts->output_charset, "<script type=\"text/javascript\" src=\"%s\" charset=\"%s\"></script>\n", hypview_js_name, charset);
		if (opts->for_cgi || js_written)
			return TRUE;
		fname = g_build_filename(opts->output_dir, hypview_js_name, NULL);
		exists = hyp_utf8_stat(fname, &st) == 0;
		if (exists && !force_overwrite)
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
		js_written = TRUE;
		g_free(fname);
		out = g_string_sized_new(sizeof(html_javascript_code) + 200);
		g_string_append_printf(out, "/* This file was automatically generated by %s version %s */\n\n", gl_program_name, gl_program_version);
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
	const char *click;
	int xpos = 0;
	const int button_w = 40;
	
	g_string_append_printf(out, "<div class=\"%s\">\n", html_toolbar_style);
	
	g_string_append(out, "<form action=\"hypview.cgi\" method=\"get\">\n");
	g_string_append(out, "<fieldset style=\"border:0;margin-left:0;margin-right:0;padding-top:0;padding-bottom:0;padding-left:0;padding-right:0;\">\n");
	g_string_append(out, "<legend></legend>\n");
	g_string_append(out, "<ul>\n");
	alt = _("Back");
	g_string_append_printf(out,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"javascript: window.history.go(-1)\" class=\"%s\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		html_nav_img_style, html_nav_back_png, alt, alt, html_nav_dimensions, html_closer);
	xpos += button_w;
	
	if (hypnode_valid(hyp, entry->previous) &&
		node != entry->previous)
	{
		str = html_filename_for_node(hyp, opts, entry->previous, TRUE);
		title = html_quote_nodename(hyp, entry->previous, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		disabled = "";
	} else
	{
		title = g_strdup(_("Previous page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"p\" rel=\"prev\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_prev_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);
	xpos += button_w;
	
	if (hypnode_valid(hyp, entry->toc_index) &&
		node != entry->toc_index)
	{
		str = html_filename_for_node(hyp, opts, entry->toc_index, TRUE);
		title = html_quote_nodename(hyp, entry->toc_index, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		disabled = "";
	} else
	{
		title = g_strdup(_("Up"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"t\" rel=\"up\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_toc_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);
	xpos += button_w;

	if (hypnode_valid(hyp, entry->next) &&
		node != entry->next)
	{
		str = html_filename_for_node(hyp, opts, entry->next, TRUE);
		title = html_quote_nodename(hyp, entry->next, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		disabled = "";
	} else
	{
		title = g_strdup(_("Next page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"n\" rel=\"next\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_next_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);
	xpos += button_w;

	if (hypnode_valid(hyp, hyp->index_page) &&
		node != hyp->index_page)
	{
		str = html_filename_for_node(hyp, opts, hyp->index_page, TRUE);
		title = html_quote_nodename(hyp, hyp->index_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		disabled = "";
	} else
	{
		title = g_strdup(_("Index page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"x\" rel=\"index\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_index_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);
	xpos += button_w;

	title = g_strdup(_("Cross references"));
	str = g_strdup(void_href);
	if (xrefs != NULL)
	{
		disabled = "";
		click = " onclick=\"showXrefs();\"";
	} else
	{
		disabled = "_disabled";
		click = "";
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"%s\" class=\"%s%s\"%s accesskey=\"c\" rel=\"bookmark\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, click, html_nav_xref_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);
	xpos += button_w;

	if (hypnode_valid(hyp, hyp->help_page) &&
		node != hyp->help_page)
	{
		str = html_filename_for_node(hyp, opts, hyp->help_page, TRUE);
		title = html_quote_nodename(hyp, hyp->help_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		disabled = "";
	} else
	{
		title = g_strdup(_("Help page"));
		str = g_strdup(void_href);
		disabled = "_disabled";
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"h\" rel=\"help\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_help_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);
	xpos += button_w;

	alt = _("Show info about hypertext");
	str = g_strdup(void_href);
	disabled = "";
	hyp_utf8_sprintf_charset(out, opts->output_charset,
		"<li style=\"position:absolute;left:%dpx;\">"
		"<a href=\"%s\" class=\"%s%s\" onclick=\"showInfo();\" accesskey=\"i\" rel=\"copyright\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_info_png, alt, alt, html_nav_dimensions, html_closer);
	g_free(str);
	xpos += button_w + 20;

	if (opts->for_cgi)
	{
		char *str;
		
		alt = _("View a new file");
		disabled = "";
		g_string_append_printf(out,
			"<li style=\"position:absolute;left:%dpx;\">"
			"<a href=\"%s\" class=\"%s%s\" accesskey=\"o\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
			"</li>\n",
			xpos,
			html_nav_load_href, html_nav_img_style, disabled, html_nav_load_png, alt, alt, html_nav_dimensions, html_closer);
		xpos += button_w;
	
		g_string_append_printf(out, "<li style=\"position:absolute;left:%dpx;\">\n", xpos);
		str = html_quote_name(html_referer_url, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		g_string_append_printf(out, "<input type=\"hidden\" name=\"url\" value=\"%s\"%s\n", str, html_closer);
		g_free(str);
		g_string_append_printf(out, "<input accesskey=\"s\" type=\"text\" id=\"searchfield\" name=\"q\" size=\"10\" value=\"\"%s\n", html_closer);
		g_string_append_printf(out, "<script type=\"text/javascript\">document.getElementById('searchfield').placeholder = '%s';</script>\n", _("Search"));
		g_string_append(out, "</li>\n");
	}
	g_string_append(out, "</ul>\n");
	g_string_append(out, "</fieldset>\n");
	g_string_append(out, "</form>\n");

	g_string_append(out, "</div>\n");
}

/* ------------------------------------------------------------------------- */

static gboolean html_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, symtab_entry *syms, gboolean for_inline);

static void html_generate_href(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, struct html_xref *xref, symtab_entry *syms, gboolean newwindow, unsigned char curtextattr)
{
	const char *target = newwindow ? " target=\"_new\"" : "";
	char *quoted;
	
	switch (xref->desttype)
	{
	case HYP_NODE_EOF:
		quoted = html_quote_name(xref->destname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"%s\">%s</a>", html_error_link_style, xref->destfilename, quoted);
		g_free(quoted);
		break;
	case HYP_NODE_INTERNAL:
	case HYP_NODE_POPUP:
	case HYP_NODE_EXTERNAL_REF:
		{
		gboolean is_xref = FALSE;
		char *style;
		if (xref->desttype == HYP_NODE_EXTERNAL_REF)
		{
			char *p = ((hyp->st_guide_flags & STG_ALLOW_FOLDERS_IN_XREFS) ? strrslash : strslash)(xref->destname);
			char c = '\0';
			hyp_filetype ft;
			style = g_strdup_printf(" class=\"%s\"", html_xref_link_style);
			if (p != NULL)
			{
				c = *p;
				*p = '\0';
			}
			ft = hyp_guess_filetype(xref->destname);
			is_xref = ft != HYP_FT_NONE;
			if (ft == HYP_FT_RSC)
			{
				quoted = html_quote_name(xref->destname, QUOTE_CONVSLASH | QUOTE_SPACE | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0));
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<a%s href=\"%s&file=%s&tree=%u\">%s></a>", style, html_view_rsc_href, quoted, xref->line, xref->text);
				g_free(quoted);
			} else if (ft == HYP_FT_HYP)
			{
				/*
				 * basename here is as specified in the link,
				 * which is often all uppercase.
				 * Always convert to lowercase first.
				 */
				char *base = hyp_utf8_strdown(hyp_basename(xref->destname), STR0TERM);
				if (opts->for_cgi)
				{
					char *dir = hyp_path_get_dirname(html_referer_url);
					char *ref;
					char *params = html_cgi_params(opts);
					
					ref = g_strconcat(dir, *dir ? "/" : "", base, NULL);
					quoted = html_quote_name(ref, QUOTE_CONVSLASH | QUOTE_URI | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0));
					hyp_utf8_sprintf_charset(out, opts->output_charset, "<a%s href=\"%s?url=%s%s\">%s</a>", style, cgi_scriptname, quoted, params, xref->text);
					g_free(quoted);
					g_free(ref);
					g_free(params);
					g_free(dir);
				} else
				{
					char *htmlbase = replace_ext(base, HYP_EXT_HYP, "");
					html_convert_filename(htmlbase);
					quoted = html_quote_name(htmlbase, QUOTE_CONVSLASH | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
					hyp_utf8_sprintf_charset(out, opts->output_charset, "<a%s href=\"../%s/%s.html\">%s</a>", style, quoted, quoted, xref->text);
					g_free(quoted);
					g_free(htmlbase);
				}
				g_free(base);
			}
			if (p)
				*p = c;
		} else if (xref->desttype == HYP_NODE_POPUP)
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
			if (xref->line != 0)
			{
				sym = sym_find(syms, xref->destname, REF_LABELNAME);
				while (sym)
				{
					if (sym->lineno == xref->line /* && !sym->from_idx */)
					{
						label = sym->name;
						sym->referenced = TRUE;
						break;
					}
					sym = sym_find(sym->next, xref->destname, REF_LABELNAME);
				}
			}
			if (label)
			{
				char *quoted = html_quote_name(label, QUOTE_LABEL | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<a%s href=\"%s#%s\"%s>%s</a>", style, xref->destfilename, quoted, target, xref->text);
				g_free(quoted);
			} else if (xref->desttype == HYP_NODE_POPUP && *target == '\0')
			{
				char *id;
				GString *tmp;
				
				++html_popup_id;
				id = g_strdup_printf("hypview_popup_%d", html_popup_id);
				html_out_attr(out, curtextattr, 0);
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<span class=\"%s\">", html_dropdown_style);
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<a%s id=\"%s_btn\" href=\"javascript:void(0);\" onclick=\"showPopup('%s')\">%s</a>", style, id, id, xref->text);
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<span class=\"%s\" style=\"position:fixed;\" id=\"%s_content\" onclick=\"hidePopup('%s');\">", html_dropdown_pnode_style, id, id);
				
				tmp = g_string_new(NULL);
				if (html_out_node(hyp, opts, tmp, xref->dest_page, syms, TRUE))
				{
					g_string_append_len(out, tmp->str, tmp->len);
				}
				g_string_free(tmp, TRUE);
				g_string_append(out, "</span></span>");
				g_free(id);
				html_out_attr(out, 0, curtextattr);
			} else
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<a%s href=\"%s\"%s>%s</a>", style, xref->destfilename, target, xref->text);
			}
		}
		g_free(style);
		}
		break;
	case HYP_NODE_REXX_COMMAND:
		quoted = html_quote_name(xref->destname, QUOTE_JS);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"javascript:execRx(&quot;%s&quot;, &quot;%s&quot;)\">%s</a>", html_rxs_link_style, _("Execute REXX command"), quoted, xref->text);
		g_free(quoted);
		break;
	case HYP_NODE_REXX_SCRIPT:
		quoted = html_quote_name(xref->destname, QUOTE_JS);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"javascript:execRxs(&quot;%s&quot;, &quot;%s&quot;)\">%s</a>", html_system_link_style, _("Execute REXX script"), quoted, xref->text);
		g_free(quoted);
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		quoted = html_quote_name(xref->destname, QUOTE_JS);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"javascript:execSystem(&quot;%s&quot;, &quot;%s&quot;)\">%s</a>", html_rx_link_style, _("Execute command"), quoted, xref->text);
		g_free(quoted);
		break;
	case HYP_NODE_IMAGE:
		/* that would be an inline image; currently not supported by compiler */
		quoted = html_quote_name(xref->destname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"%s\">%s</a>", html_image_link_style, quoted, xref->text);
		g_free(quoted);
		break;
	case HYP_NODE_QUIT:
		/* not really quit, but best we can do */
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_quit_link_style, xref->text);
		break;
	case HYP_NODE_CLOSE:
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_close_link_style, xref->text);
		break;
	default:
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<a class=\"%s\" href=\"%s\">%s %u</a>", html_error_link_style, xref->destfilename, _("link to unknown node type"), hyp->indextable[xref->dest_page]->type);
		break;
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_entities(GString *out)
{
	g_string_append(out, " [\n");
	g_string_append(out, "<!ENTITY uparrow \"&#8679;\">          <!-- 0x01 U+21E7 -->\n");
	g_string_append(out, "<!ENTITY downarrow \"&#8681;\">        <!-- 0x02 U+21E9 -->\n");
	g_string_append(out, "<!ENTITY rightarrow \"&#8680;\">       <!-- 0x03 U+21E8 -->\n");
	g_string_append(out, "<!ENTITY leftarrow \"&#8678;\">        <!-- 0x04 U+21E6 -->\n");
	g_string_append(out, "<!ENTITY ballotbox \"&#9744;\">        <!-- 0x05 U+2610 -->\n");
	g_string_append(out, "<!ENTITY ballotboxcheck \"&#9745;\">   <!-- 0x06 U+2611 -->\n");
	g_string_append(out, "<!ENTITY ballotboxx \"&#9746;\">       <!-- 0x07 U+2612 -->\n");
	g_string_append(out, "<!ENTITY checkmark \"&#10003;\">       <!-- 0x08 U+2713 -->\n");
	g_string_append(out, "<!ENTITY watch \"&#8986;\">            <!-- 0x09 U+231A -->\n");
	g_string_append(out, "<!ENTITY bell \"&#9086;\">             <!-- 0x0a U+237E -->\n");
	g_string_append(out, "<!ENTITY eightnote \"&#9834;\">        <!-- 0x0b U+266a -->\n");
	g_string_append(out, "<!ENTITY mountain \"&#9968;\">         <!-- 0x0e U+26f0 -->\n");
	g_string_append(out, "<!ENTITY umbrella \"&#9969;\">         <!-- 0x0f U+26f1 -->\n");
	g_string_append(out, "<!ENTITY circledzero \"&#9450;\">      <!-- 0x10 U+24ea -->\n");
	g_string_append(out, "<!ENTITY circledone \"&#9312;\">       <!-- 0x11 U+2460 -->\n");
	g_string_append(out, "<!ENTITY circledtwo \"&#9313;\">       <!-- 0x12 U+2461 -->\n");
	g_string_append(out, "<!ENTITY circledthree \"&#9314;\">     <!-- 0x13 U+2462 -->\n");
	g_string_append(out, "<!ENTITY circledfour \"&#9315;\">      <!-- 0x14 U+2463 -->\n");
	g_string_append(out, "<!ENTITY circledfive \"&#9316;\">      <!-- 0x15 U+2464 -->\n");
	g_string_append(out, "<!ENTITY circledsix \"&#9317;\">       <!-- 0x16 U+2465 -->\n");
	g_string_append(out, "<!ENTITY circledseven \"&#9318;\">     <!-- 0x17 U+2466 -->\n");
	g_string_append(out, "<!ENTITY circledeight \"&#9319;\">     <!-- 0x18 U+2467 -->\n");
	g_string_append(out, "<!ENTITY circlednine \"&#9320;\">      <!-- 0x19 U+2468 -->\n");
	g_string_append(out, "<!ENTITY capitalschwa \"&#399;\">      <!-- 0x1a U+018f -->\n");
	g_string_append(out, "<!ENTITY fountain \"&#9970;\">         <!-- 0x1c U+26f2 -->\n");
	g_string_append(out, "<!ENTITY flaginhole \"&#9971;\">       <!-- 0x1d U+26f3 -->\n");
	g_string_append(out, "<!ENTITY ferry \"&#9972;\">            <!-- 0x1e U+26f4 -->\n");
	g_string_append(out, "<!ENTITY sailboat \"&#9973;\">         <!-- 0x1f U+26f5 -->\n");
	g_string_append(out, "<!ENTITY increment \"&#8710;\">        <!-- 0x7f U+2206 -->\n");

	g_string_append(out, "<!ENTITY nul \"&#9216;\">              <!-- 0x00 U+2400 -->\n");
	g_string_append(out, "<!ENTITY soh \"&#9217;\">              <!-- 0x01 U+2401 -->\n");
	g_string_append(out, "<!ENTITY stx \"&#9218;\">              <!-- 0x02 U+2402 -->\n");
	g_string_append(out, "<!ENTITY etx \"&#9219;\">              <!-- 0x03 U+2403 -->\n");
	g_string_append(out, "<!ENTITY eot \"&#9220;\">              <!-- 0x04 U+2404 -->\n");
	g_string_append(out, "<!ENTITY enq \"&#9221;\">              <!-- 0x05 U+2405 -->\n");
	g_string_append(out, "<!ENTITY ack \"&#9222;\">              <!-- 0x06 U+2406 -->\n");
	g_string_append(out, "<!ENTITY bel \"&#9223;\">              <!-- 0x07 U+2407 -->\n");
	g_string_append(out, "<!ENTITY bs  \"&#9224;\">              <!-- 0x08 U+2408 -->\n");
	g_string_append(out, "<!ENTITY ht  \"&#9225;\">              <!-- 0x09 U+2409 -->\n");
	g_string_append(out, "<!ENTITY lf  \"&#9226;\">              <!-- 0x0a U+240a -->\n");
	g_string_append(out, "<!ENTITY vt  \"&#9227;\">              <!-- 0x0b U+240b -->\n");
	g_string_append(out, "<!ENTITY ff  \"&#9228;\">              <!-- 0x0c U+240c -->\n");
	g_string_append(out, "<!ENTITY cr  \"&#9229;\">              <!-- 0x0d U+240d -->\n");
	g_string_append(out, "<!ENTITY so  \"&#9230;\">              <!-- 0x0e U+240e -->\n");
	g_string_append(out, "<!ENTITY si  \"&#9231;\">              <!-- 0x0f U+240f -->\n");
	g_string_append(out, "<!ENTITY dle \"&#9232;\">              <!-- 0x10 U+2410 -->\n");
	g_string_append(out, "<!ENTITY dc1 \"&#9233;\">              <!-- 0x11 U+2411 -->\n");
	g_string_append(out, "<!ENTITY dc2 \"&#9234;\">              <!-- 0x12 U+2412 -->\n");
	g_string_append(out, "<!ENTITY dc3 \"&#9235;\">              <!-- 0x13 U+2413 -->\n");
	g_string_append(out, "<!ENTITY dc4 \"&#9236;\">              <!-- 0x14 U+2414 -->\n");
	g_string_append(out, "<!ENTITY nak \"&#9237;\">              <!-- 0x15 U+2415 -->\n");
	g_string_append(out, "<!ENTITY syn \"&#9238;\">              <!-- 0x16 U+2416 -->\n");
	g_string_append(out, "<!ENTITY etb \"&#9239;\">              <!-- 0x17 U+2417 -->\n");
	g_string_append(out, "<!ENTITY can \"&#9240;\">              <!-- 0x18 U+2418 -->\n");
	g_string_append(out, "<!ENTITY em  \"&#9241;\">              <!-- 0x19 U+2419 -->\n");
	g_string_append(out, "<!ENTITY sub \"&#9242;\">              <!-- 0x1a U+241a -->\n");
	g_string_append(out, "<!ENTITY esc \"&#9243;\">              <!-- 0x1b U+241b -->\n");
	g_string_append(out, "<!ENTITY fs  \"&#9244;\">              <!-- 0x1c U+241c -->\n");
	g_string_append(out, "<!ENTITY gs  \"&#9245;\">              <!-- 0x1d U+241d -->\n");
	g_string_append(out, "<!ENTITY rs  \"&#9246;\">              <!-- 0x1e U+241e -->\n");
	g_string_append(out, "<!ENTITY us  \"&#9247;\">              <!-- 0x1f U+241f -->\n");
	g_string_append(out, "<!ENTITY del \"&#9249;\">              <!-- 0x7f U+2421 -->\n");

	g_string_append(out, "<!ENTITY nbsp \"&#32;\">\n");
	g_string_append(out, "]");
}

/* ------------------------------------------------------------------------- */

static void html_out_header(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const char *title, hyp_nodenr node, struct hyp_gfx *hyp_gfx, struct html_xref *xrefs, symtab_entry *syms, gboolean for_error)
{
	const char *charset = hyp_charset_name(opts->output_charset);
	INDEX_ENTRY *entry = hyp ? hyp->indextable[node] : NULL;
	char *str;
	const char *doctype;
	
	{
	char *html_extra;
	char *html_lang;
	char *p;
	
	if (hyp && hyp->language != NULL)
	{
		html_lang = g_strdup(hyp->language);
		p = strchr(html_lang, '_');
		if (p)
			*p = '-';
	} else
	{
		html_lang = g_strdup("en");
	}
	
	switch (html_doctype)
	{
	case HTML_DOCTYPE_OLD:
		doctype = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"";
		html_extra = g_strdup_printf(" lang=\"%s\"", html_lang);
		break;
	case HTML_DOCTYPE_TRANS:
		doctype = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
		          "        \"http://www.w3.org/TR/html4/loose.dtd\"";
		html_extra = g_strdup_printf(" lang=\"%s\"", html_lang);
		break;
	
	case HTML_DOCTYPE_XSTRICT:
		if (opts->recompile_format == HYP_FT_HTML_XML)
			g_string_append_printf(out, "<?xml version=\"1.0\" encoding=\"%s\"?>\n", charset);
		doctype = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
		          "          \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\"";
		if (hyp_gfx != NULL)
			html_extra = g_strdup_printf(" xml:lang=\"%s\" lang=\"%s\" xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:svg=\"http://www.w3.org/2000/svg\"", html_lang, html_lang);
		else
			html_extra = g_strdup_printf(" xml:lang=\"%s\" lang=\"%s\" xmlns=\"http://www.w3.org/1999/xhtml\"", html_lang, html_lang);
		break;
	case HTML_DOCTYPE_STRICT:
	default:
		doctype = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n"
		          "          \"http://www.w3.org/TR/html4/strict.dtd\"";
		html_extra = g_strdup_printf(" lang=\"%s\"", html_lang);
		break;
	case HTML_DOCTYPE_HTML5:
		doctype = "<!DOCTYPE html";
		html_extra = g_strdup_printf(" xml:lang=\"%s\" lang=\"%s\"", html_lang, html_lang);
		break;
	case HTML_DOCTYPE_FRAME:
	case HTML_DOCTYPE_XFRAME:
		abort();
		break;
	}

	if (doctype)
		g_string_append(out, doctype);
	if (0)
		html_out_entities(out);
	if (doctype)
		g_string_append(out, ">\n");
	g_string_append(out, "<html");
	g_string_append(out, html_extra);
	g_string_append(out, ">\n");
	g_free(html_extra);
	g_free(html_lang);
	}
	
	g_string_append_printf(out, "<!-- This file was automatically generated by %s version %s -->\n", gl_program_name, gl_program_version);
	hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- %s -->\n", HYP_COPYRIGHT);
	if (hyp != NULL && node == hyp->main_page)
		html_out_globals(hyp, opts, out);
	if (node != HYP_NOINDEX)
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("<!-- Node #%u -->\n"), node);
	g_string_append(out, "<head>\n");
	if (html_doctype >= HTML_DOCTYPE_HTML5)
		g_string_append_printf(out, "<meta charset=\"%s\"%s\n", charset, html_closer);
	else
		g_string_append_printf(out, "<meta http-equiv=\"content-type\" content=\"text/html;charset=%s\"%s\n", charset, html_closer);
	g_string_append_printf(out, "<meta name=\"GENERATOR\" content=\"%s %s\"%s\n", gl_program_name, gl_program_version, html_closer);
	if (hyp && hyp->author != NULL)
	{
		char *str = html_quote_name(hyp->author, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<meta name=\"author\" content=\"%s\"%s\n", str, html_closer);
		g_free(str);
	}
	if (hyp && hyp->database != NULL)
	{
		char *str = html_quote_name(hyp->database, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<meta name=\"description\" content=\"%s\"%s\n", str, html_closer);
		g_free(str);
	}
	if (title)
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<title>%s</title>\n", title);

	if (entry && entry->type == HYP_NODE_INTERNAL)
	{
		if (hypnode_valid(hyp, hyp->first_text_page) &&
			node != hyp->first_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->first_text_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"%s\"%s\n", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "start" : "first", html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->previous) &&
			node != entry->previous)
		{
			str = html_filename_for_node(hyp, opts, entry->previous, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"prev\"%s\n", str, html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->toc_index) &&
			node != entry->toc_index)
		{
			str = html_filename_for_node(hyp, opts, entry->toc_index, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"up\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, entry->next) &&
			node != entry->next)
		{
			str = html_filename_for_node(hyp, opts, entry->next, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"next\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->last_text_page) &&
			node != hyp->last_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->last_text_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"%s\"%s\n", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "end" : "last", html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, hyp->index_page) &&
			node != hyp->index_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->index_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"index\"%s\n", str, html_closer);
			g_free(str);
		}

		if (xrefs != NULL)
		{
			str = g_strdup("javascript: showXrefs();");
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"bookmark\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->help_page) &&
			node != hyp->help_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->help_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"help\"%s\n", str, html_closer);
			g_free(str);
		}

		str = g_strdup("javascript: showInfo();");
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<link href=\"%s\" rel=\"copyright\"%s\n", str, html_closer);
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
		g_string_append_printf(out, "<div style=\"width:%dex;position:absolute;left:%dpx;\">\n", hyp->line_width, gl_profile.viewer.text_xoffset);

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
		g_string_append(out, "<pre style=\"margin-top:0;\">");
	} else
	{
		g_string_append_printf(out, "<div class=\"%s\">\n", html_pnode_style);
		g_string_append(out, "<pre>");
	}

	if (hyp)
	{
		/*
		 * this element is displayed for "About"
		 */
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<span class=\"%s\">", html_dropdown_style);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_info_style, html_hyp_info_id);
		str = html_quote_name(hyp->database, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("Topic       : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->author, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("Author      : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->version, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("Version     : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->subject, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("Subject     : %s\n"), fixnull(str));
		g_free(str);
		str = g_strdup_printf(_("Nodes       : %u\n"
		                        "Index Size  : %ld\n"
		                        "HCP-Version : %u\n"
		                        "Compiled on : %s\n"
		                        "@charset    : %s\n"
		                        "@lang       : %s\n"
		                        "@default    : %s\n"
		                        "@help       : %s\n"
		                        "@options    : %s\n"
		                        "@width      : %u"),
		                        hyp->num_index,
		                        hyp->itable_size,
		                        hyp->comp_vers,
		                        hyp_osname(hyp->comp_os),
		                        hyp_charset_name(hyp->comp_charset),
		                        fixnull(hyp->language),
		                        fixnull(hyp->default_name),
		                        fixnull(hyp->help_name),
		                        fixnull(hyp->hcp_options),
		                        hyp->line_width);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "%s", str);
		g_free(str);
		{
			HYP_HOSTNAME *h;
			for (h = hyp->hostname; h != NULL; h = h->next)
			{
				str = html_quote_name(h->name, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
				hyp_utf8_sprintf_charset(out, opts->output_charset, _("\n@hostname   : %s"), str);
				g_free(str);
			}
		}
		hyp_utf8_sprintf_charset(out, opts->output_charset, "</span></span>");
		if (opts->showstg && node == 0)
		{
			stg_out_globals(hyp, opts, out);
		}
	}

	if (hyp && xrefs)
	{
		struct html_xref *xref;
		
		/*
		 * this element is displayed for the cross references
		 */
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<span class=\"%s\">", html_dropdown_style);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_xrefs_style, html_hyp_xrefs_id);
		for (xref = xrefs; xref != NULL; xref = xref->next)
		{
			html_generate_href(hyp, opts, out, xref, syms, FALSE, 0);
			g_string_append(out, "\n");
		}
		hyp_utf8_sprintf_charset(out, opts->output_charset, "</span></span>");
	}
}

/* ------------------------------------------------------------------------- */

static void html_out_trailer(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, gboolean for_error, gboolean warn_gfx)
{
	INDEX_ENTRY *entry = hyp ? hyp->indextable[node] : NULL;

	if (hyp != NULL && node == hyp->main_page && opts->output_charset == HYP_CHARSET_ATARI && opts->for_cgi)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset,
			"\n\n<span class=\"%s\">%s%s</span>\n", html_error_note_style, _("warning: "), _("writing html output in atari encoding might not work with non-atari browsers"));
	}
	if (warn_gfx)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset,
			"<noscript>\n"
			"<p><span style=\"color:red\">\n"
			"<b>%s</b>\n"
			"%s\n"
			"</span>\n"
			"<br /></p>\n"
			"</noscript>\n",
			_("Your browser does not support JavaScript"),
			_("Graphics commands on this page cannot be displayed"));
	}

	if (for_error)
	{
		g_string_append(out, "</p>\n");
		g_string_append(out, "</div>\n");
	} else
	{
		g_string_append(out, "</pre>\n");
		g_string_append(out, "</div>\n");
		if (entry != NULL && entry->type == HYP_NODE_INTERNAL)
			g_string_append(out, "</div>\n");
	}
#if 0
	/*
	 * hack to remove the "]>" at the start that slips
	 * in from the entity definitions if the file
	 * was not loaded as xml
	 */
	g_string_append(out, "<script type=\"text/javascript\">\n");
	g_string_append(out, "var a = document.body.firstChild;\n");
	g_string_append(out, "if (a.nodeType == 3) document.body.removeChild(a);\n");
	g_string_append(out, "</script>\n");
#endif
	
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
	int gfx_id = 0;
	FILE *outfp = NULL;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		html_out_str(hyp, opts, out, textstart, src - textstart); \
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
		hyp_utf8_sprintf_charset(out, opts->output_charset, "end tree %d -->\n", in_tree); \
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
		unsigned short dithermask;
		gboolean needs_javascript = FALSE;
		
		{
		char *title;
		
		if (!for_inline && opts->outfile != stdout && node != hyp->main_page)
		{
			char *name = html_filename_for_node(hyp, opts, node, FALSE);
			char *output_filename;
			
			/*
			 * the first file was already created in the main loop,
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
			title = html_quote_name(buf, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			g_free(buf);
		} else
		{
			title = html_quote_nodename(hyp, node, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
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
						hyp_decode_gfx(hyp, src + 1, adm, opts->errorfile, opts->read_images);
						if (adm->type == HYP_ESC_PIC)
						{
							adm->format = format_from_pic(opts, hyp->indextable[adm->extern_node_index], HTML_DEFAULT_PIC_TYPE);
							adm->dithermask = dithermask;
							dithermask = 0;
						} else
						{
							needs_javascript = TRUE;
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
					char *destfilename;
					char *text;
					char *buf;
					hyp_indextype desttype;
					
					dest_page = DEC_255(&src[3]);
					buf = hyp_conv_to_utf8(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
					buf = chomp(buf);
					text = html_quote_name(buf, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
					g_free(buf);
					if (hypnode_valid(hyp, dest_page))
					{
						INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
						destfilename = html_filename_for_node(hyp, opts, dest_page, TRUE);
						destname = hyp_conv_to_utf8(hyp->comp_charset, dest_entry->name, dest_entry->length - SIZEOF_INDEX_ENTRY);
						desttype = (hyp_indextype) dest_entry->type;
					} else
					{
						destfilename = g_strdup("/nonexistent.html");
						str = hyp_invalid_page(dest_page);
						destname = g_strdup(str);
						g_free(str);
						desttype = HYP_NODE_EOF;
					}
					if (empty(text))
					{
						g_free(text);
						text = g_strdup(destname);
					}
					{
						struct html_xref *xref;
						xref = g_new0(struct html_xref, 1);
						xref->text = text;
						xref->destname = destname;
						xref->destfilename = destfilename;
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
		
		if (!for_inline)
			html_out_header(hyp, opts, out, title, node, hyp_gfx, xrefs, syms, FALSE);

		g_free(title);
		}

		if (opts->showstg)
		{
			retval = stg_out_nodedata(hyp, opts, out, nodeptr, syms);
			if (hyp_gfx != NULL)
			{
				struct hyp_gfx *gfx, *next;
				
				for (gfx = hyp_gfx; gfx != NULL; gfx = next)
				{
					next = gfx->next;
					g_free(gfx);
				}
				hyp_gfx = NULL;
			}
		} else
		{
			/*
			 * check for alias names in ref file
			 */
			html_out_alias(out, hyp, opts, entry, syms);
			
			/*
			 * now output data
			 */
			src = nodeptr->start;
			textstart = src;
			at_bol = TRUE;
			in_tree = -1;
			textattr = 0;
			lineno = 0;
			html_out_labels(hyp, opts, out, entry, lineno, syms);
			html_out_graphics(hyp, opts, out, hyp_gfx, lineno, &gfx_id);
			
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
						g_string_append(out, "&#x241b;");
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
							struct html_xref xref;
							
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
								xref.destfilename = html_filename_for_node(hyp, opts, xref.dest_page, TRUE);
								xref.destname = hyp_conv_to_utf8(hyp->comp_charset, dest_entry->name, dest_entry->length - SIZEOF_INDEX_ENTRY);
								xref.desttype = (hyp_indextype) dest_entry->type;
							} else
							{
								xref.destfilename = g_strdup("/nonexistent.html");
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
									xref.text = html_quote_nodename(hyp, xref.dest_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
									str_equal = entry->type == HYP_NODE_INTERNAL;
								} else
								{
									str_equal = FALSE;
									xref.text = g_strdup(xref.destname);
									len = strlen(xref.text);
								}
							} else
							{
								char *buf;
								
								len = *src - HYP_STRLEN_OFFSET;
								src++;
								buf = hyp_conv_to_utf8(hyp->comp_charset, src, len);
								xref.text = html_quote_name(buf, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
								g_free(buf);
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
							
							html_generate_href(hyp, opts, out, &xref, syms, type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE, textattr);
	
							g_free(xref.destname);
							g_free(xref.destfilename);
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
								str = html_quote_nodename(hyp, dest_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
							} else
							{
								str = hyp_invalid_page(dest_page);
							}
							FLUSHLINE();
							if (tree != in_tree)
							{
								FLUSHTREE();
								hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- begin tree %d\n", tree);
								in_tree = tree;
							}
							hyp_utf8_sprintf_charset(out, opts->output_charset, "   %d \"%s\" %u\n", obj, str, line);
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
						html_out_attr(out, textattr, *src - HYP_ESC_TEXTATTR_FIRST);
						textattr = *src - HYP_ESC_TEXTATTR_FIRST;
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
					FLUSHTREE();
					DUMPTEXT();
					g_string_append_c(out, '\n');
					at_bol = TRUE;
					++lineno;
					html_out_labels(hyp, opts, out, entry, lineno, syms);
					html_out_graphics(hyp, opts, out, hyp_gfx, lineno, &gfx_id);
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
			html_out_labels(hyp, opts, out, entry, lineno, syms);
			html_out_graphics(hyp, opts, out, hyp_gfx, lineno, &gfx_id);
			
			if (hyp_gfx != NULL)
			{
				struct hyp_gfx *gfx, *next;
				
				for (gfx = hyp_gfx; gfx != NULL; gfx = next)
				{
					if (!gfx->used)
					{
						hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- gfx unused: ");
						html_out_gfx(opts, out, hyp, gfx, &gfx_id);
						hyp_utf8_sprintf_charset(out, opts->output_charset, "-->\n");
					}
					next = gfx->next;
					g_free(gfx);
				}
			}
		}
					
		if (!for_inline)
		{
			html_out_trailer(hyp, opts, out, node, FALSE, needs_javascript && !opts->showstg);
		
			if (node < hyp->last_text_page && opts->outfile == stdout)
				g_string_append(out, "\n\n");
		}
		
		{
			struct html_xref *xref, *next;
			
			for (xref = xrefs; xref != NULL; xref = next)
			{
				next = xref->next;
				g_free(xref->destname);
				g_free(xref->destfilename);
				g_free(xref->text);
				g_free(xref);
			}
		}
		
		hyp_node_free(nodeptr);
		
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
#include "pattern.h"

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


static void html_init(hcp_opts *opts)
{
	force_crlf = (opts->outfile == stdout || opts->output_charset != HYP_CHARSET_ATARI) ? FALSE : TRUE;
	html_closer = html_doctype >= HTML_DOCTYPE_XSTRICT ? " />" : ">";
	html_name_attr = html_doctype >= HTML_DOCTYPE_XSTRICT ? "id" : "name";
}


static gboolean recompile_html(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	GString *out;
	
	UNUSED(argc);
	UNUSED(argv);
	
	html_init(opts);
	
	ret = TRUE;
	
	if (opts->output_charset == HYP_CHARSET_ATARI && opts->errorfile != stdout)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s%s\n", _("warning: "), _("writing html output in atari encoding might not work with non-atari browsers"));
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
		if (node == hyp->main_page || opts->outfile == stdout)
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
				hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- symbol unused: \"%s\" \"%s\" -->\n", sym->nodename, sym->name);
			}
		}
		write_strout(out, opts->outfile);
	}
	
	g_string_free(out, TRUE);
	
	free_symtab(syms);
	return ret;
}
