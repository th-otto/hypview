/*
 * HypView - (c)      - 2019 Thorsten Otto
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#include "outcomm.h"
#include "outhtml.h"
#include "outstg.h"
#include "stat_.h"
#include "htmljs.h"
#include "hv_vers.h"

#include "iback_png.h"
#include "iprev_png.h"
#include "itoc_png.h"
#include "inext_png.h"
#include "ixref_png.h"
#include "iload_png.h"
#include "iindex_png.h"
#include "ihelp_png.h"
#include "iinfo_png.h"
#include "itreeview_png.h"

char *html_referer_url;
int html_doctype = HTML_DOCTYPE_XSTRICT;
gboolean html_css_written = FALSE;
gboolean html_js_written = FALSE;
gboolean html_navimages_written = FALSE;

#define IMAGES_DIR "images"
#define IMAGES_DIRS IMAGES_DIR "/"

static char const html_nav_back_png[] = IMAGES_DIRS "iback.png";
static char const html_nav_prev_png[] = IMAGES_DIRS "iprev.png";
static char const html_nav_toc_png[] = IMAGES_DIRS "itoc.png";
static char const html_nav_next_png[] = IMAGES_DIRS "inext.png";
static char const html_nav_xref_png[] = IMAGES_DIRS "ixref.png";
static char const html_nav_load_png[] = IMAGES_DIRS "iload.png";
static char const html_nav_index_png[] = IMAGES_DIRS "iindex.png";
static char const html_nav_help_png[] = IMAGES_DIRS "ihelp.png";
static char const html_nav_info_png[] = IMAGES_DIRS "iinfo.png";
static char const html_nav_treeview_png[] = IMAGES_DIRS "itreeview.png";
static char const html_hyp_info_id[] = "hyp_info";
static char const html_hyp_xrefs_id[] = "hyp_xrefs";
static char const html_nav_dimensions[] = " width=\"32\" height=\"21\"";

#define USE_TREEVIEW_IMAGES 1
#define USE_TREEVIEW_SVG 0
#if USE_TREEVIEW_IMAGES
static char const html_tv_blank_png[] = IMAGES_DIRS "tv_blank.png";
static char const html_tv_nointersec_png[] = IMAGES_DIRS "tv_nointersec.png";
static char const html_tv_expanded_end_png[] = IMAGES_DIRS "tv_expanded_end.png";
static char const html_tv_expanded_png[] = IMAGES_DIRS "tv_expanded.png";
static char const html_tv_collapsed_end_png[] = IMAGES_DIRS "tv_collapsed_end.png";
static char const html_tv_collapsed_png[] = IMAGES_DIRS "tv_collapsed.png";
static char const html_tv_intersec_png[] = IMAGES_DIRS "tv_intersec.png";
static char const html_tv_end_png[] = IMAGES_DIRS "tv_end.png";
static char const html_tv_dimensions[] = " title=\"\" alt=\"\" width=\"16\" height=\"16\" border=\"0\" vspace=\"0\" hspace=\"0\" align=\"top\"";
#include "tv_blank_png.h"
#include "tv_intersec_png.h"
#include "tv_collapsed_png.h"
#include "tv_collapsed_end_png.h"
#include "tv_expanded_png.h"
#include "tv_expanded_end_png.h"
#include "tv_nointersec_png.h"
#include "tv_end_png.h"
#endif

const char *cgi_scriptname = "hypview.cgi";
char const hypview_lineno_tag[] = "hypview_lineno";
static char const html_nav_load_href[] = "index.php";
static char const hypview_css_name[] = "_hypview.css";
static char const hypview_js_name[] = "_hypview.js";

static const char *html_closer = " />";
static const char *html_name_attr = "id";

#define USE_ENTITIES 0

#if USE_ENTITIES
#define ENTITY(ent, code) ent
#else
#define ENTITY(ent, code) code
#endif

/*
 * style names used
 */
char const html_attr_bold_style[] = "hypview_attr_bold";
static char const html_attr_light_style[] = "hypview_attr_light";
static char const html_attr_italic_style[] = "hypview_attr_italic";
static char const html_attr_underlined_style[] = "hypview_attr_underlined";
static char const html_attr_outlined_style[] = "hypview_attr_outlined";
static char const html_attr_shadowed_style[] = "hypview_attr_shadowed";
static char const html_toolbar_style[] = "hypview_nav_toolbar";
static char const html_nav_img_style[] = "hypview_nav_img";
static char const html_nav_search_style[] = "hypview_nav_search";
static char const html_node_style[] = "hypview_node";
static char const html_pnode_style[] = "hypview_pnode";
static char const html_treeview_style[] = "hypview_treeview";
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


static gboolean const force_overwrite = TRUE;
static int html_popup_id;

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

char *html_quote_name(const char *name, unsigned int flags)
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
		if ((flags & QUOTE_URI) && is_weblink(name))
		{
			/*
			 * do not quote the protocol part
			 */
			while (*name && *name != ':')
				*str++ = *name++;
			if (*name == ':')
				*str++ = *name++;
			while (*name == '/')
				*str++ = *name++;
		}
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
					STR(ENTITY("&soh;", "&#9217;"));
				}
				break;
			case 0x02:
				if (flags & QUOTE_URI)
				{
					STR("%02");
				} else
				{
					STR(ENTITY("&stx;", "&#9218;"));
				}
				break;
			case 0x03:
				if (flags & QUOTE_URI)
				{
					STR("%03");
				} else
				{
					STR(ENTITY("&etx;", "&#9219;"));
				}
				break;
			case 0x04:
				if (flags & QUOTE_URI)
				{
					STR("%04");
				} else
				{
					STR(ENTITY("&eot;", "&#9220;"));
				}
				break;
			case 0x05:
				if (flags & QUOTE_URI)
				{
					STR("%05");
				} else
				{
					STR(ENTITY("&enq;", "&#9221;"));
				}
				break;
			case 0x06:
				if (flags & QUOTE_URI)
				{
					STR("%06");
				} else
				{
					STR(ENTITY("&ack;", "&#9222;"));
				}
				break;
			case 0x07:
				if (flags & QUOTE_URI)
				{
					STR("%07");
				} else
				{
					STR(ENTITY("&bel;", "&#9223;"));
				}
				break;
			case 0x08:
				if (flags & QUOTE_URI)
				{
					STR("%08");
				} else
				{
					STR(ENTITY("&bs;", "&#9224;"));
				}
				break;
			case 0x09:
				if (flags & QUOTE_URI)
				{
					STR("%09");
				} else
				{
					STR(ENTITY("&ht;", "&#9225;"));
				}
				break;
			case 0x0a:
				if (flags & QUOTE_URI)
				{
					STR("%0A");
				} else
				{
					STR(ENTITY("&lf;", "&#9226;"));
				}
				break;
			case 0x0b:
				if (flags & QUOTE_URI)
				{
					STR("%0B");
				} else
				{
					STR(ENTITY("&vt;", "&#9227;"));
				}
				break;
			case 0x0c:
				if (flags & QUOTE_URI)
				{
					STR("%0C");
				} else
				{
					STR(ENTITY("&ff;", "&#9228;"));
				}
				break;
			case 0x0d:
				if (flags & QUOTE_URI)
				{
					STR("%0D");
				} else
				{
					STR(ENTITY("&cr;", "&#9229;"));
				}
				break;
			case 0x0e:
				if (flags & QUOTE_URI)
				{
					STR("%0E");
				} else
				{
					STR(ENTITY("&so;", "&#9230;"));
				}
				break;
			case 0x0f:
				if (flags & QUOTE_URI)
				{
					STR("%0F");
				} else
				{
					STR(ENTITY("&si;", "&#9231;"));
				}
				break;
			case 0x10:
				if (flags & QUOTE_URI)
				{
					STR("%10");
				} else
				{
					STR(ENTITY("&dle;", "&#9232;"));
				}
				break;
			case 0x11:
				if (flags & QUOTE_URI)
				{
					STR("%11");
				} else
				{
					STR(ENTITY("&dc1;", "&#9233;"));
				}
				break;
			case 0x12:
				if (flags & QUOTE_URI)
				{
					STR("%12");
				} else
				{
					STR(ENTITY("&dc2;", "&#9234;"));
				}
				break;
			case 0x13:
				if (flags & QUOTE_URI)
				{
					STR("%13");
				} else
				{
					STR(ENTITY("&dc3;", "&#9235;"));
				}
				break;
			case 0x14:
				if (flags & QUOTE_URI)
				{
					STR("%14");
				} else
				{
					STR(ENTITY("&dc4;", "&#9236;"));
				}
				break;
			case 0x15:
				if (flags & QUOTE_URI)
				{
					STR("%15");
				} else
				{
					STR(ENTITY("&nak;", "&#9237;"));
				}
				break;
			case 0x16:
				if (flags & QUOTE_URI)
				{
					STR("%16");
				} else
				{
					STR(ENTITY("&syn;", "&#9238;"));
				}
				break;
			case 0x17:
				if (flags & QUOTE_URI)
				{
					STR("%17");
				} else
				{
					STR(ENTITY("&etb;", "&#9239;"));
				}
				break;
			case 0x18:
				if (flags & QUOTE_URI)
				{
					STR("%18");
				} else
				{
					STR(ENTITY("&can;", "&#9240;"));
				}
				break;
			case 0x19:
				if (flags & QUOTE_URI)
				{
					STR("%19");
				} else
				{
					STR(ENTITY("&em;", "&#9241;"));
				}
				break;
			case 0x1a:
				if (flags & QUOTE_URI)
				{
					STR("%1A");
				} else
				{
					STR(ENTITY("&sub;", "&#9242;"));
				}
				break;
			case 0x1b:
				if (flags & QUOTE_URI)
				{
					STR("%1B");
				} else
				{
					STR(ENTITY("&esc;", "&#9243;"));
				}
				break;
			case 0x1c:
				if (flags & QUOTE_URI)
				{
					STR("%1C");
				} else
				{
					STR(ENTITY("&fs;", "&#9244;"));
				}
				break;
			case 0x1D:
				if (flags & QUOTE_URI)
				{
					STR("%1D");
				} else
				{
					STR(ENTITY("&gs;", "&#9245;"));
				}
				break;
			case 0x1E:
				if (flags & QUOTE_URI)
				{
					STR("%1E");
				} else
				{
					STR(ENTITY("&rs;", "&#9246;"));
				}
				break;
			case 0x1F:
				if (flags & QUOTE_URI)
				{
					STR("%1F");
				} else
				{
					STR(ENTITY("&us;", "&#9247;"));
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

char *html_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node, unsigned int flags)
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

static void html_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, gboolean *converror)
{
	char *str;

	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @os %s -->\n", hyp_osname(hyp->comp_os));
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @charset %s -->\n", hyp_charset_name(hyp->comp_charset));
	
	if (hyp->language != NULL)
	{
		str = html_quote_name(hyp->language, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @lang \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->database != NULL)
	{
		str = html_quote_name(hyp->database, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @database \"%s\" -->\n", str);
		g_free(str);
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = html_quote_nodename(hyp, hyp->default_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @default \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		str = html_quote_name(hyp->hcp_options, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @options \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->author != NULL)
	{
		str = html_quote_name(hyp->author, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @author \"%s\" -->\n", str);
		g_free(str);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = html_quote_nodename(hyp, hyp->help_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @help \"%s\" -->\n", str);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		str = html_quote_name(hyp->version, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @$VER: %s -->\n", str);
		g_free(str);
	}
	if (hyp->subject != NULL)
	{
		str = html_quote_name(hyp->subject, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @subject \"%s\" -->\n", str);
		g_free(str);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @width %d -->\n", hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("<!-- ST-Guide flags: $%04x -->\n"), hyp->st_guide_flags);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			str = html_quote_name(h->name, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- @hostname \"%s\" -->\n", str);
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

static gboolean html_out_attr(GString *out, struct textattr *attr)
{
	gboolean retval = FALSE;
	
	if (attr->curattr != attr->newattr)
	{
#define on(mask, style) \
		if (!(attr->curattr & mask) && (attr->newattr & mask)) \
			g_string_append_printf(out, "<span class=\"%s\">", style)
#define off(mask, str) \
		if ((attr->curattr & mask) && !(attr->newattr & mask)) \
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
		attr->curattr = attr->newattr;
		retval = TRUE;
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

static int html_out_gfx(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, int *gfx_id, gboolean *converror)
{
	char *id;
	int need_nl = FALSE;

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
			int x;
			const char *align;
			
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
					fname = g_strdup_printf("%s?url=%s&index=%u%s", cgi_scriptname, html_referer_url, gfx->extern_node_index, opts->cgi_cached ? "&cached=1" : "");
					alt = g_strdup_printf("index=%u", gfx->extern_node_index);
				} else
				{
					fname = g_strdup("");
					alt = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
				}
				origfname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix, opts->ignore_image_name);
			} else
			{
				fname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix, opts->ignore_image_name);
				origfname = fname;
				alt = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			}
			quoted = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			origquoted = html_quote_name(origfname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
			hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<!-- %s \"%s\" %d%s%s -->",
				gfx->islimage ? "@limage" : "@image",
				origfname,
				gfx->x_offset,
				*dithermask ? " %" : "", dithermask);
			if (gfx->x_offset == 0)
			{
				x = (hyp->line_width - gfx->pixwidth / HYP_PIC_FONTW) / 2;
				if (x < 0)
					x = 0;
				align = " align=\"center\"";
			} else
			{
				x = gfx->x_offset - 1;
				align = "";
				if ((x + (gfx->pixwidth / HYP_PIC_FONTW)) == hyp->line_width)
					align = " align=\"right\"";
			}
			
			if (gfx->islimage)
			{
				/*
				 * the '\n' at the end here shouldn't be there,
				 * but st-guide leaves an empty line after each @limage
				 */
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<div class=\"%s\"%s style=\"%swidth:%dch; left:%dch\"><img src=\"%s\" alt=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0;\"%s</div>",
					html_limage_style,
					align,
					*align ? "" : "position:relative; ",
					hyp->line_width,
					x,
					quoted,
					alt,
					gfx->pixwidth,
					gfx->pixheight,
					html_closer);
				if ((gfx->height % HYP_PIC_FONTH) == 0)
					need_nl = TRUE;
			} else
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<div class=\"%s\" style=\"position:absolute; left:%dch;\"><img src=\"%s\" alt=\"%s\" width=\"%d\" height=\"%d\" style=\"border:0;\"%s</div>",
					html_image_style,
					x,
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
	return need_nl;
}

/* ------------------------------------------------------------------------- */

static void html_out_graphics(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, struct hyp_gfx *gfx, long lineno, int *gfx_id, gboolean *converror)
{
	int need_nl = FALSE;

	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			need_nl |= html_out_gfx(opts, out, hyp, gfx, gfx_id, converror);
		}
		gfx = gfx->next;
	}
	if (need_nl)
		g_string_append(out, "\n");
}

/* ------------------------------------------------------------------------- */

void html_out_stg_gfx(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, char *fname, gboolean *converror)
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
		fname = g_strdup_printf("%s?url=%s&index=%u%s", cgi_scriptname, html_referer_url, gfx->extern_node_index, opts->cgi_cached ? "&cached=1" : "");
	}
	quoted = html_quote_name(fname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
	origquoted = html_quote_name(origfname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "%s \"<a href=\"%s\">%s</a>\" %d",
		gfx->islimage ? "@limage" : "@image",
		quoted,
		origquoted,
		gfx->x_offset);
	g_free(origquoted);
	g_free(quoted);
	if (fname != origfname)
		g_free(fname);
}

/* ------------------------------------------------------------------------- */

static void html_out_lineno(hcp_opts *opts, GString *out, long lineno)
{
	hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<a %s=\"%s%ld\"></a>", html_name_attr, hypview_lineno_tag, lineno + 1);
}

/* ------------------------------------------------------------------------- */

static void html_out_labels(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms, gboolean *converror)
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
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<!-- lineno %u --><a %s=\"%s\"></a>", sym->lineno, html_name_attr, str);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void html_out_alias(GString *out, HYP_DOCUMENT *hyp, hcp_opts *opts, const INDEX_ENTRY *entry, symtab_entry *syms, gboolean *converror)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = html_quote_name(sym->name, QUOTE_LABEL | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a %s=\"%s\"></a>", html_name_attr, str);
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
static void html_convert_filename(char *filename, gboolean allow_nonascii)
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
			c == 0x7f ||
			(c >= 0x80 && !allow_nonascii) ||
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
 * TODO: check whether long filenames are supported
 */
char *html_filename_for_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, gboolean quote)
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
		html_convert_filename(filename, quote || !opts->for_cgi);
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
			char *tmp = html_quote_name(html_referer_url, QUOTE_URI | QUOTE_NOLTR);
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
		if (opts->for_cgi || html_css_written)
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
		html_css_written = TRUE;
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
	
	g_string_append(out, "/* style used for the search field in the navigation toolbar */\n");
	g_string_append_printf(out, ".%s {\n", html_nav_search_style);
	g_string_append(out, "  height:28px;\n");
	g_string_append(out, "  font-size: 80%;\n");
	g_string_append(out, "  padding: 0px;\n");
	g_string_append(out, "  border-width: 1px;\n");
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
	g_string_append(out, "  border: 0;\n");
	g_string_append(out, "  padding: 0;\n");
	g_string_append(out, "  z-index:-1;\n");
	g_string_append(out, "}\n");

	g_string_append(out, "/* styles for user colors */\n");
	g_string_append(out, ".fgcolor_white { color: #ffffff; }\n");
	g_string_append(out, ".fgcolor_black { color: #000000; }\n");
	g_string_append(out, ".fgcolor_red { color: #ff0000; }\n");
	g_string_append(out, ".fgcolor_green { color: #00ff00; }\n");
	g_string_append(out, ".fgcolor_blue { color: #0000ff; }\n");
	g_string_append(out, ".fgcolor_cyan { color: #00ffff; }\n");
	g_string_append(out, ".fgcolor_yellow { color: #ffff00; }\n");
	g_string_append(out, ".fgcolor_magenta { color: #ff00ff; }\n");
	g_string_append(out, ".fgcolor_lgray { color: #cccccc; }\n");
	g_string_append(out, ".fgcolor_dgray { color: #888888; }\n");
	g_string_append(out, ".fgcolor_dred { color: #880000; }\n");
	g_string_append(out, ".fgcolor_dgreen { color: #008800; }\n");
	g_string_append(out, ".fgcolor_dblue { color: #000088; }\n");
	g_string_append(out, ".fgcolor_dcyan { color: #008888; }\n");
	g_string_append(out, ".fgcolor_dyellow { color: #888800; }\n");
	g_string_append(out, ".fgcolor_dmagenta { color: #880088; }\n");
	g_string_append(out, ".bgcolor_white { background-color: #ffffff; }\n");
	g_string_append(out, ".bgcolor_black { background-color: #000000; }\n");
	g_string_append(out, ".bgcolor_red { background-color: #ff0000; }\n");
	g_string_append(out, ".bgcolor_green { background-color: #00ff00; }\n");
	g_string_append(out, ".bgcolor_blue { background-color: #0000ff; }\n");
	g_string_append(out, ".bgcolor_cyan { background-color: #00ffff; }\n");
	g_string_append(out, ".bgcolor_yellow { background-color: #ffff00; }\n");
	g_string_append(out, ".bgcolor_magenta { background-color: #ff00ff; }\n");
	g_string_append(out, ".bgcolor_lgray { background-color: #cccccc; }\n");
	g_string_append(out, ".bgcolor_dgray { background-color: #888888; }\n");
	g_string_append(out, ".bgcolor_dred { background-color: #880000; }\n");
	g_string_append(out, ".bgcolor_dgreen { background-color: #008800; }\n");
	g_string_append(out, ".bgcolor_dblue { background-color: #000088; }\n");
	g_string_append(out, ".bgcolor_dcyan { background-color: #008888; }\n");
	g_string_append(out, ".bgcolor_dyellow { background-color: #888800; }\n");
	g_string_append(out, ".bgcolor_dmagenta { background-color: #880088; }\n");

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
		
		hyp_utf8_sprintf_charset(outstr, opts->output_charset, NULL, "<script type=\"text/javascript\" src=\"%s\" charset=\"%s\"></script>\n", hypview_js_name, charset);
		if (opts->for_cgi || html_js_written)
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
		html_js_written = TRUE;
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

static void html_out_navimage(const char *dir, const char *name, const unsigned char *data, size_t size)
{
	char *filename;
	FILE *fp;
	
	filename = g_build_filename(dir, name, NULL);
	if (filename == NULL)
		return;
	fp = fopen(filename, "wb");
	if (fp != NULL)
	{
		fwrite(data, 1, size, fp);
		fclose(fp);
	}
	g_free(filename);
}

static void html_out_navimages(hcp_opts *opts)
{
	char *dir;
	
	if (html_navimages_written)
		return;
	/*
	 * assumes these images are always available on the server
	 */
	if (opts->for_cgi)
		return;
		
	dir = g_build_filename(opts->output_dir, IMAGES_DIR, NULL);
	if (mkdir(dir, 0750) < 0 && errno != EEXIST)
		fprintf(opts->errorfile, "%s: %s\n", dir, strerror(errno));
	html_navimages_written = TRUE;
	
	html_out_navimage(opts->output_dir, html_nav_back_png, nav_back_data, sizeof(nav_back_data));
	html_out_navimage(opts->output_dir, html_nav_prev_png, nav_prev_data, sizeof(nav_prev_data));
	html_out_navimage(opts->output_dir, html_nav_toc_png, nav_toc_data, sizeof(nav_toc_data));
	html_out_navimage(opts->output_dir, html_nav_next_png, nav_next_data, sizeof(nav_next_data));
	html_out_navimage(opts->output_dir, html_nav_xref_png, nav_xref_data, sizeof(nav_xref_data));
	html_out_navimage(opts->output_dir, html_nav_load_png, nav_load_data, sizeof(nav_load_data));
	html_out_navimage(opts->output_dir, html_nav_index_png, nav_index_data, sizeof(nav_index_data));
	html_out_navimage(opts->output_dir, html_nav_help_png, nav_help_data, sizeof(nav_help_data));
	html_out_navimage(opts->output_dir, html_nav_info_png, nav_info_data, sizeof(nav_info_data));
	html_out_navimage(opts->output_dir, html_nav_treeview_png, nav_treeview_data, sizeof(nav_treeview_data));
#if USE_TREEVIEW_IMAGES
	html_out_navimage(opts->output_dir, html_tv_blank_png, tv_blank_data, sizeof(tv_blank_data));
	html_out_navimage(opts->output_dir, html_tv_intersec_png, tv_intersec_data, sizeof(tv_intersec_data));
	html_out_navimage(opts->output_dir, html_tv_collapsed_png, tv_collapsed_data, sizeof(tv_collapsed_data));
	html_out_navimage(opts->output_dir, html_tv_collapsed_end_png, tv_collapsed_end_data, sizeof(tv_collapsed_end_data));
	html_out_navimage(opts->output_dir, html_tv_expanded_png, tv_expanded_data, sizeof(tv_expanded_data));
	html_out_navimage(opts->output_dir, html_tv_expanded_end_png, tv_expanded_end_data, sizeof(tv_expanded_end_data));
	html_out_navimage(opts->output_dir, html_tv_nointersec_png, tv_nointersec_data, sizeof(tv_nointersec_data));
	html_out_navimage(opts->output_dir, html_tv_end_png, tv_end_data, sizeof(tv_end_data));
#endif
	
	g_free(dir);
}

/* ------------------------------------------------------------------------- */

static void html_out_nav_toolbar(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, struct html_xref *xrefs, gboolean *converror)
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
	
	g_string_append(out, "<form action=\"hypview.cgi\" method=\"get\" onsubmit=\"return submitSearch(event);\">\n");
	g_string_append(out, "<fieldset style=\"border:0;margin-left:0;margin-right:0;padding-top:0;padding-bottom:0;padding-left:0;padding-right:0;\">\n");
	g_string_append(out, "<legend></legend>\n");
	g_string_append(out, "<ul>\n");
	alt = _("Back");
	g_string_append_printf(out,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
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
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
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
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
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
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
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
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
		"<a href=\"%s\" class=\"%s%s\" accesskey=\"x\" rel=\"index\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_index_png, title, title, html_nav_dimensions, html_closer);
	g_free(title);
	g_free(str);
	xpos += button_w;

	if (opts->for_cgi)
	{
		char *params = html_cgi_params(opts);
		char *tmp = html_quote_name(html_referer_url, QUOTE_URI | QUOTE_NOLTR);
		str = g_strdup_printf("%s?url=%s%s&amp;treeview=1", cgi_scriptname, tmp, params);
		g_free(tmp);
		g_free(params);
	} else
	{
		str = html_quote_name("_treeview" HYP_EXT_HTML, 0);
	}
	disabled = "";
	title = g_strdup(_("Tree View"));
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
		"<a href=\"%s\" class=\"%s%s\" ><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
		"</li>\n",
		xpos,
		str, html_nav_img_style, disabled, html_nav_treeview_png, title, title, html_nav_dimensions, html_closer);
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
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
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
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
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
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
		"<li style=\"position:absolute;left:%dpx;top:1px;\">"
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
			"<li style=\"position:absolute;left:%dpx;top:1px;\">"
			"<a href=\"%s\" class=\"%s%s\" accesskey=\"o\"><img src=\"%s\" alt=\"&nbsp;%s&nbsp;\" title=\"%s\"%s%s</a>"
			"</li>\n",
			xpos,
			html_nav_load_href, html_nav_img_style, disabled, html_nav_load_png, alt, alt, html_nav_dimensions, html_closer);
		xpos += button_w;
	
		g_string_append_printf(out, "<li style=\"position:absolute;left:%dpx;top:1px;padding:0px;\">\n", xpos);
		str = html_quote_name(html_referer_url, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		g_string_append_printf(out, "<input type=\"hidden\" name=\"url\" value=\"%s\"%s\n", str, html_closer);
		g_free(str);
		g_string_append_printf(out, "<input accesskey=\"s\" type=\"text\" id=\"searchfield\" name=\"q\" class=\"%s\" size=\"10\" value=\"\"%s\n", html_nav_search_style, html_closer);
		g_string_append_printf(out, "<script type=\"text/javascript\">document.getElementById('searchfield').placeholder = '%s';</script>\n", _("Search"));
		g_string_append(out, "</li>\n");
	}
	g_string_append(out, "</ul>\n");
	g_string_append(out, "</fieldset>\n");
	g_string_append(out, "</form>\n");

	g_string_append(out, "</div>\n");
}

/* ------------------------------------------------------------------------- */

static void html_generate_href(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, struct html_xref *xref, symtab_entry *syms, gboolean newwindow, unsigned char curtextattr, gboolean *converror)
{
	const char *target = newwindow ? " target=\"_new\"" : "";
	char *quoted;
	
	switch (xref->desttype)
	{
	case HYP_NODE_EOF:
		quoted = html_quote_name(xref->destname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"%s\">%s</a>", html_error_link_style, xref->destfilename, quoted);
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
				/*
				 * generate link to a resource file
				 */
				const char *base = hyp_basename(xref->destname);
				
				if (opts->for_cgi)
				{
					char *dir = hyp_path_get_dirname(html_referer_url);
					char *ref;
					char *params = html_cgi_params(opts);
					
					ref = g_strconcat(dir, *dir ? "/" : "", base, NULL);
					quoted = html_quote_name(ref, QUOTE_CONVSLASH | QUOTE_URI | QUOTE_NOLTR | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0));
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a%s href=\"%s?url=%s%s&amp;index=%u&amp;isrsc=1\">%s</a>", style, cgi_scriptname, quoted, params, xref->line, xref->text);
					g_free(quoted);
					g_free(ref);
					g_free(params);
					g_free(dir);
				} else
				{
					quoted = html_quote_name(base, QUOTE_CONVSLASH | QUOTE_SPACE | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a%s href=\"%s#%u\">%s</a>", style, quoted, xref->line, xref->text);
					g_free(quoted);
				}
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
				if (opts->for_cgi)
				{
					char *dir = hyp_path_get_dirname(html_referer_url);
					char *ref;
					char *params = html_cgi_params(opts);
					
					ref = g_strconcat(dir, *dir ? "/" : "", base, NULL);
					quoted = html_quote_name(ref, QUOTE_CONVSLASH | QUOTE_URI | QUOTE_NOLTR | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0));
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a%s href=\"%s?url=%s%s\">%s</a>", style, cgi_scriptname, quoted, params, xref->text);
					g_free(quoted);
					g_free(ref);
					g_free(params);
					g_free(dir);
				} else
				{
					char *htmlbase = replace_ext(base, HYP_EXT_HYP, "");
					html_convert_filename(htmlbase, TRUE);
					quoted = html_quote_name(htmlbase, QUOTE_CONVSLASH | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a%s href=\"../%s/%s.html\">%s</a>", style, quoted, quoted, xref->text);
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
				if (label == NULL)
				{
					char *tmp = g_strdup_printf("%s#%s%u", xref->destfilename, hypview_lineno_tag, xref->line + 1);
					g_free(xref->destfilename);
					xref->destfilename = tmp;
				}
			}
			if (label)
			{
				/*
				 * generate link to a label
				 */
				char *quoted = html_quote_name(label, QUOTE_LABEL | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0));
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a%s href=\"%s#%s\"%s>%s</a>", style, xref->destfilename, quoted, target, xref->text);
				g_free(quoted);
			} else if (xref->desttype == HYP_NODE_POPUP && !newwindow)
			{
				char *id;
				GString *tmp;
				struct textattr attr;
				
				/*
				 * generate link to a popup node
				 */
				++html_popup_id;
				id = g_strdup_printf("hypview_popup_%d", html_popup_id);
				attr.curattr = curtextattr;
				attr.newattr = 0;
				attr.curfg = attr.newfg = HYP_DEFAULT_FG;
				attr.curbg = attr.newbg = HYP_DEFAULT_BG;
				html_out_attr(out, &attr);
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<span class=\"%s\">", html_dropdown_style);
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a%s id=\"%s_btn\" href=\"javascript:void(0);\" onclick=\"showPopup('%s')\">%s</a>", style, id, id, xref->text);
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<span class=\"%s\" style=\"position:fixed;\" id=\"%s_content\" onclick=\"hidePopup('%s');\">", html_dropdown_pnode_style, id, id);
				
				tmp = g_string_new(NULL);
				if (html_out_node(hyp, opts, tmp, xref->dest_page, syms, TRUE, converror))
				{
					g_string_append_len(out, tmp->str, tmp->len);
				} else
				{
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("failed to decode node"));
				}
				g_string_free(tmp, TRUE);
				g_string_append(out, "</span></span>");
				g_free(id);
				attr.newattr = curtextattr;
				html_out_attr(out, &attr);
			} else
			{
				/*
				 * generate link to a regular node
				 */
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a%s href=\"%s\"%s>%s</a>", style, xref->destfilename, target, xref->text);
			}
		}
		g_free(style);
		}
		break;
	case HYP_NODE_REXX_COMMAND:
		if (is_weblink(xref->destname))
		{
			quoted = html_quote_name(xref->destname, QUOTE_NOLTR | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0));
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"%s\">%s</a>", html_rx_link_style, quoted, xref->text);
			g_free(quoted);
		} else
		{
			quoted = html_quote_name(xref->destname, QUOTE_JS);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"javascript:execRx(&quot;%s&quot;, &quot;%s&quot;)\">%s</a>", html_rx_link_style, _("Execute REXX command"), quoted, xref->text);
			g_free(quoted);
		}
		break;
	case HYP_NODE_REXX_SCRIPT:
		if (is_weblink(xref->destname))
		{
			quoted = html_quote_name(xref->destname, QUOTE_NOLTR | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0));
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"%s\">%s</a>", html_rxs_link_style, quoted, xref->text);
			g_free(quoted);
		} else
		{
			quoted = html_quote_name(xref->destname, QUOTE_JS);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"javascript:execRxs(&quot;%s&quot;, &quot;%s&quot;)\">%s</a>", html_rxs_link_style, _("Execute REXX script"), quoted, xref->text);
			g_free(quoted);
		}
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		if (is_weblink(xref->destname))
		{
			quoted = html_quote_name(xref->destname, QUOTE_NOLTR | (opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0));
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"%s\">%s</a>", html_system_link_style, quoted, xref->text);
			g_free(quoted);
		} else
		{
			quoted = html_quote_name(xref->destname, QUOTE_JS);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"javascript:execSystem(&quot;%s&quot;, &quot;%s&quot;)\">%s</a>", html_system_link_style, _("Execute command"), quoted, xref->text);
			g_free(quoted);
		}
		break;
	case HYP_NODE_IMAGE:
		/* that would be an inline image; currently not supported by compiler */
		quoted = html_quote_name(xref->destname, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"%s\">%s</a>", html_image_link_style, quoted, xref->text);
		g_free(quoted);
		break;
	case HYP_NODE_QUIT:
		/* not really quit, but best we can do */
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_quit_link_style, xref->text);
		break;
	case HYP_NODE_CLOSE:
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"javascript: window.close()\">%s</a>", html_close_link_style, xref->text);
		break;
	default:
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<a class=\"%s\" href=\"%s\">%s %u</a>", html_error_link_style, xref->destfilename, _("link to unknown node type"), hyp->indextable[xref->dest_page]->type);
		break;
	}
}

/* ------------------------------------------------------------------------- */

void html_out_entities(GString *out)
{
#if USE_ENTITIES
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
#else
	UNUSED(out);
#endif
}

/* ------------------------------------------------------------------------- */

void html_out_header(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const char *title, hyp_nodenr node, gboolean uses_graphics, struct html_xref *xrefs, symtab_entry *syms, gboolean for_error, gboolean *converror)
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
			if (uses_graphics)
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
		html_out_entities(out);
		if (doctype)
			g_string_append(out, ">\n");
		g_string_append(out, "<html");
		g_string_append(out, html_extra);
		g_string_append(out, ">\n");
		g_free(html_extra);
		g_free(html_lang);
	}
	
	g_string_append(out, "<head>\n");
	if (html_doctype >= HTML_DOCTYPE_HTML5)
		g_string_append_printf(out, "<meta charset=\"%s\"%s\n", charset, html_closer);
	else
		g_string_append_printf(out, "<meta http-equiv=\"content-type\" content=\"text/html;charset=%s\"%s\n", charset, html_closer);

	g_string_append_printf(out, "<!-- This file was automatically generated by %s version %s -->\n", gl_program_name, gl_program_version);
	hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<!-- %s -->\n", HYP_COPYRIGHT);
	if (hyp != NULL && node == hyp->main_page)
		html_out_globals(hyp, opts, out, converror);
	if (node != HYP_NOINDEX)
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, _("<!-- Node #%u -->\n"), node);

	g_string_append_printf(out, "<meta name=\"GENERATOR\" content=\"%s %s\"%s\n", gl_program_name, gl_program_version, html_closer);
	if (hyp && hyp->author != NULL)
	{
		char *str = html_quote_name(hyp->author, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<meta name=\"author\" content=\"%s\"%s\n", str, html_closer);
		g_free(str);
	}
	if (hyp && hyp->database != NULL)
	{
		char *str = html_quote_name(hyp->database, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<meta name=\"description\" content=\"%s\"%s\n", str, html_closer);
		g_free(str);
	}
	if (title)
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<title>%s</title>\n", title);

	if (entry && entry->type == HYP_NODE_INTERNAL)
	{
		if (hypnode_valid(hyp, hyp->first_text_page) &&
			node != hyp->first_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->first_text_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"%s\"%s\n", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "start" : "first", html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->previous) &&
			node != entry->previous)
		{
			str = html_filename_for_node(hyp, opts, entry->previous, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"prev\"%s\n", str, html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->toc_index) &&
			node != entry->toc_index)
		{
			str = html_filename_for_node(hyp, opts, entry->toc_index, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"up\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, entry->next) &&
			node != entry->next)
		{
			str = html_filename_for_node(hyp, opts, entry->next, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"next\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->last_text_page) &&
			node != hyp->last_text_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->last_text_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"%s\"%s\n", str, html_doctype >= HTML_DOCTYPE_XSTRICT ? "end" : "last", html_closer);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, hyp->index_page) &&
			node != hyp->index_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->index_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"index\"%s\n", str, html_closer);
			g_free(str);
		}

		if (xrefs != NULL)
		{
			str = g_strdup("javascript: showXrefs();");
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"bookmark\"%s\n", str, html_closer);
			g_free(str);
		}

		if (hypnode_valid(hyp, hyp->help_page) &&
			node != hyp->help_page)
		{
			str = html_filename_for_node(hyp, opts, hyp->help_page, TRUE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"help\"%s\n", str, html_closer);
			g_free(str);
		}

		str = g_strdup("javascript: showInfo();");
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<link href=\"%s\" rel=\"copyright\"%s\n", str, html_closer);
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
			html_out_navimages(opts);
			html_out_nav_toolbar(hyp, opts, out, node, xrefs, converror);
			g_string_append_printf(out, "<div class=\"%s\" style=\"position:absolute; top:32px;\">\n", html_node_style);
		}
		g_string_append(out, "<pre style=\"margin-top:0;\">");
	} else
	{
		g_string_append_printf(out, "<div class=\"%s\">\n", opts->treeview ? html_treeview_style : html_pnode_style);
		if (!opts->treeview)
			g_string_append(out, "<pre>");
	}

	if (hyp)
	{
		/*
		 * this element is displayed for "About"
		 */
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<span class=\"%s\">", html_dropdown_style);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_info_style, html_hyp_info_id);
		str = html_quote_name(hyp->database, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("Topic       : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->author, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("Author      : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->version, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("Version     : %s\n"), fixnull(str));
		g_free(str);
		str = html_quote_name(hyp->subject, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("Subject     : %s\n"), fixnull(str));
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
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "%s", str);
		g_free(str);
		{
			HYP_HOSTNAME *h;
			for (h = hyp->hostname; h != NULL; h = h->next)
			{
				str = html_quote_name(h->name, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("\n@hostname   : %s"), str);
				g_free(str);
			}
		}
		if (opts->for_cgi && hyp->ref)
		{
			char *params = html_cgi_params(opts);
			char *refname = replace_ext(html_referer_url, NULL, HYP_EXT_REF);
			char *tmp = html_quote_name(refname, QUOTE_URI | QUOTE_NOLTR);
			char *filename = g_strdup_printf("%s?url=%s%s", cgi_scriptname, tmp, params);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "\n<a href=\"%s\">%s</a>", filename, _("View Ref-File"));
			g_free(filename);
			g_free(tmp);
			g_free(refname);
			g_free(params);
		}
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "</span></span>");
		if (opts->showstg && node == 0)
		{
			stg_out_globals(hyp, opts, out, converror);
		}
	}

	if (hyp && xrefs)
	{
		struct html_xref *xref;
		
		/*
		 * this element is displayed for the cross references
		 */
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<span class=\"%s\">", html_dropdown_style);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "<span class=\"%s\" id=\"%s_content\">", html_dropdown_xrefs_style, html_hyp_xrefs_id);
		for (xref = xrefs; xref != NULL; xref = xref->next)
		{
			html_generate_href(hyp, opts, out, xref, syms, FALSE, 0, converror);
			g_string_append(out, "\n");
		}
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "</span></span>");
	}
}

/* ------------------------------------------------------------------------- */

void html_out_trailer(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, gboolean for_error, gboolean warn_gfx, gboolean *converror)
{
	INDEX_ENTRY *entry = hyp ? hyp->indextable[node] : NULL;

	if (hyp != NULL && node == hyp->main_page && opts->output_charset == HYP_CHARSET_ATARI && opts->for_cgi)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
			"\n\n<span class=\"%s\">%s%s</span>\n", html_error_note_style, _("warning: "), _("writing html output in atari encoding might not work with non-atari browsers"));
	}
	if (warn_gfx)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror,
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
		if (hyp && *converror)
		{
			if (opts->for_cgi)
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, NULL,
					"\n\n<p><span style=\"color:red\">\n"
					"<b>%s</b>\n"
					"</span>\n"
					"<br /></p>\n",
					_("Some characters could not be converted"));
			} else
			{
				hyp_utf8_fprintf(opts->errorfile, "%s%s\n", _("warning: "), _("Some characters could not be converted"));
			}
		}
		if (!opts->treeview)
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

static void html_out_color(GString *out, unsigned char color, gboolean bg)
{
	g_string_append(out, "<span class=\"");
	g_string_append(out, bg ? "bg" : "fg");
	switch (color)
	{
		case HYP_COLOR_WHITE: g_string_append(out, "color_white"); break;
		case HYP_COLOR_BLACK: g_string_append(out, "color_black"); break;
		case HYP_COLOR_RED: g_string_append(out, "color_red"); break;
		case HYP_COLOR_GREEN: g_string_append(out, "color_green"); break;
		case HYP_COLOR_BLUE: g_string_append(out, "color_blue"); break;
		case HYP_COLOR_CYAN: g_string_append(out, "color_cyan"); break;
		case HYP_COLOR_YELLOW: g_string_append(out, "color_yellow"); break;
		case HYP_COLOR_MAGENTA: g_string_append(out, "color_magenta"); break;
		case HYP_COLOR_LGRAY: g_string_append(out, "color_lgray"); break;
		case HYP_COLOR_DGRAY: g_string_append(out, "color_dgray"); break;
		case HYP_COLOR_DRED: g_string_append(out, "color_dred"); break;
		case HYP_COLOR_DGREEN: g_string_append(out, "color_dgreen"); break;
		case HYP_COLOR_DBLUE: g_string_append(out, "color_dblue"); break;
		case HYP_COLOR_DCYAN: g_string_append(out, "color_dcyan"); break;
		case HYP_COLOR_DYELLOW: g_string_append(out, "color_dyellow"); break;
		case HYP_COLOR_DMAGENTA: g_string_append(out, "color_dmagenta"); break;
		default: g_string_append(out, "color_unknown"); break;
	}
	g_string_append(out, "\">");
}

/* ------------------------------------------------------------------------- */

gboolean html_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, symtab_entry *syms, gboolean for_inline, gboolean *converror)
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
	FILE *outfp = NULL;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		if (attr.curbg != HYP_DEFAULT_BG) html_out_color(out, attr.curbg, TRUE); \
		if (attr.curfg != HYP_DEFAULT_FG) html_out_color(out, attr.curfg, FALSE); \
		html_out_str(hyp, opts, out, textstart, src - textstart); \
		if (attr.curfg != HYP_DEFAULT_FG) g_string_append(out, "</span>"); \
		if (attr.curbg != HYP_DEFAULT_BG) g_string_append(out, "</span>"); \
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
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "end tree %d -->\n", in_tree); \
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
			unsigned int quote_flags;

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
			quote_flags = opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0;
			
			if (nodeptr->window_title)
			{
				char *buf = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
				title = html_quote_name(buf, quote_flags);
				g_free(buf);
			} else
			{
				title = html_quote_nodename(hyp, node, quote_flags);
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
							destname = hyp_invalid_page(dest_page);
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
				html_out_header(hyp, opts, out, title, node, hyp_gfx != NULL, xrefs, syms, FALSE, converror);
	
			g_free(title);
		}

		if (opts->showstg)
		{
			retval = stg_out_nodedata(hyp, opts, out, nodeptr, syms, converror);
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
			html_out_alias(out, hyp, opts, entry, syms, converror);
			
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
			if (!for_inline)
				html_out_lineno(opts, out, lineno);
			html_out_labels(hyp, opts, out, entry, lineno, syms, converror);
			html_out_graphics(hyp, opts, out, hyp_gfx, lineno, &gfx_id, converror);
			
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
						g_string_append(out, ENTITY("&esc;", "&#x241b;"));
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
									xref.text = html_quote_nodename(hyp, xref.dest_page, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
									str_equal = entry->type == HYP_NODE_INTERNAL;
								} else
								{
									str_equal = FALSE;
									xref.text = g_strdup(xref.destname);
								}
							} else
							{
								char *buf;
								size_t len;
								
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
							
							html_generate_href(hyp, opts, out, &xref, syms, type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE, attr.curattr, converror);
	
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
								hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<!-- begin tree %d\n", tree);
								in_tree = tree;
							}
							hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "   %d \"%s\" %u\n", obj, str, line);
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
						html_out_attr(out, &attr);
						src++;
						break;
					
					case HYP_ESC_FG_COLOR:
						FLUSHTREE();
						src++;
						attr.newfg = *src;
						html_out_attr(out, &attr);
						src++;
						break;
				
					case HYP_ESC_BG_COLOR:
						FLUSHTREE();
						src++;
						attr.newbg = *src;
						html_out_attr(out, &attr);
						src++;
						break;
				
					case HYP_ESC_ATTR_TYPEWRITER:
						if (opts->print_unknown)
							hyp_utf8_fprintf(opts->errorfile, _("<unknown hex esc $%02x>\n"), *src);
						src++;
						break;

					default:
						if (opts->print_unknown)
							hyp_utf8_fprintf(opts->errorfile, _("<unknown hex esc $%02x>\n"), *src);
						src++;
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
					src++;
					textstart = src;
					if (src < end && !for_inline)
						html_out_lineno(opts, out, lineno);
					html_out_labels(hyp, opts, out, entry, lineno, syms, converror);
					html_out_graphics(hyp, opts, out, hyp_gfx, lineno, &gfx_id, converror);
				} else
				{
					FLUSHTREE();
					src++;
				}
			}
			DUMPTEXT();
			attr.newattr = 0;
			attr.newfg = HYP_DEFAULT_FG;
			attr.newbg = HYP_DEFAULT_BG;
			if (html_out_attr(out, &attr))
				at_bol = FALSE;
			FLUSHLINE();
			FLUSHTREE();
			++lineno;
			if (src != textstart && !for_inline)
				html_out_lineno(opts, out, lineno);
			html_out_labels(hyp, opts, out, entry, lineno, syms, converror);
			html_out_graphics(hyp, opts, out, hyp_gfx, lineno, &gfx_id, converror);
			
			if (hyp_gfx != NULL)
			{
				struct hyp_gfx *gfx, *next;
				
				for (gfx = hyp_gfx; gfx != NULL; gfx = next)
				{
					if (!gfx->used)
					{
						hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<!-- gfx unused: ");
						html_out_gfx(opts, out, hyp, gfx, &gfx_id, converror);
						hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "-->\n");
					}
					next = gfx->next;
					g_free(gfx);
				}
			}
		}
					
		if (!for_inline)
		{
			html_out_trailer(hyp, opts, out, node, FALSE, needs_javascript && !opts->showstg, converror);
		
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


void html_init(hcp_opts *opts)
{
	force_crlf = (opts->outfile == stdout || opts->output_charset != HYP_CHARSET_ATARI) ? FALSE : TRUE;
	html_closer = html_doctype >= HTML_DOCTYPE_XSTRICT ? " />" : ">";
	html_name_attr = html_doctype >= HTML_DOCTYPE_XSTRICT ? "id" : "name";
}


gboolean recompile_html(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	GString *out;
	gboolean converror = FALSE;

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
			ret &= html_out_node(hyp, opts, out, node, syms, FALSE, &converror);
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
	
	if (cmdline_version)
		ClearCache(hyp);
	
	{
		symtab_entry *sym;
		
		for (sym = syms; sym != NULL; sym = sym->next)
		{
			if (!sym->referenced && sym->type != REF_NODENAME)
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<!-- symbol unused: \"%s\" \"%s\" -->\n", sym->nodename, sym->name);
			}
		}
		write_strout(out, opts->outfile);
	}
	
	g_string_free(out, TRUE);
	
	free_symtab(syms);
	return ret;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#define line_down              "\342\224\202"
#define line_hor               "\342\224\200"
#define line_intersec          "\342\224\234"
#define line_collapsed         "\342\224\234"
#define line_collapsed_end     "\342\224\224"
#define line_expanded          "\342\224\234"
#define line_expanded_end      "\342\224\224"
#define line_end               "\342\224\224"

static void print_indent(GString *out, HYPTREE *tree, hyp_nodenr current)
{
	hyp_nodenr parent;
	hyp_nodenr pparent;
	gboolean is_last;
	
	if (current == 0 || current == HYP_NOINDEX)
		return;
	parent = tree[current].parent;
	print_indent(out, tree, parent);
	is_last = FALSE;
	if (parent == HYP_NOINDEX)
	{
		is_last = TRUE;
	} else
	{
		pparent = tree[parent].parent;
		if (pparent == HYP_NOINDEX || tree[pparent].tail == parent)
			is_last = TRUE;
	}
#if USE_TREEVIEW_IMAGES
	if (is_last)
		g_string_append_printf(out, "<img src=\"%s\"%s%s", html_tv_blank_png, html_tv_dimensions, html_closer);
	else
		g_string_append_printf(out, "<img src=\"%s\"%s%s", html_tv_nointersec_png, html_tv_dimensions, html_closer);
	g_string_append(out, " ");
#elif USE_TREEVIEW_SVG
	if (is_last)
		g_string_append(out, "<span class=\"tv_blank\"></span>");
	else
		g_string_append(out, "<span class=\"tv_nointersec\"></span>");
#else
	if (is_last)
		g_string_append(out, "&nbsp;");
	else
		g_string_append(out, line_down);
	g_string_append(out, "&nbsp;");
#endif
}

/* ------------------------------------------------------------------------- */

static void html_print_tree(HYP_DOCUMENT *hyp, hcp_opts *opts, HYPTREE *tree, GString *out, hyp_nodenr parent, int depth)
{
	hyp_nodenr child;
	struct html_xref xref;
	INDEX_ENTRY *entry;
	const char *img;
	
	g_string_append_printf(out, "<div id=\"tv_%u\">\n", parent);
	
	g_string_append(out, "<span>");
	print_indent(out, tree, parent);

#if USE_TREEVIEW_IMAGES
	if (tree[parent].num_childs != 0)
	{
		g_string_append_printf(out, "<span class=\"tv_expand\"><a href=\"#\" onclick=\"return tv_toggleExpand('tv_sub_%u', 'tv_icon_%u');\">\n", parent, parent);
		if (tree[parent].flags & HYPTREE_IS_EXPANDED)
		{
			if (tree[parent].next == HYP_NOINDEX)
				img = html_tv_expanded_end_png;
			else
				img = html_tv_expanded_png;
		} else
		{
			if (tree[parent].next == HYP_NOINDEX)
				img = html_tv_collapsed_end_png;
			else
				img = html_tv_collapsed_png;
		}
		g_string_append_printf(out, "<img id=\"tv_icon_%u\" src=\"%s\"%s%s", parent, img, html_tv_dimensions, html_closer);
		g_string_append(out, "</a></span>");
	} else
	{
		if (tree[parent].next == HYP_NOINDEX)
			img = html_tv_end_png;
		else
			img = html_tv_intersec_png;
		g_string_append_printf(out, "<img src=\"%s\"%s%s", img, html_tv_dimensions, html_closer);
	}
	g_string_append(out, " ");
#elif USE_TREEVIEW_SVG
	if (tree[parent].num_childs != 0)
	{
		g_string_append_printf(out, "<span class=\"tv_expand\"><a href=\"#\" onclick=\"return tv_toggleExpand('tv_sub_%u', 'tv_icon_%u');\">\n", parent, parent);
		if (tree[parent].flags & HYPTREE_IS_EXPANDED)
		{
			if (tree[parent].next == HYP_NOINDEX)
				img = "tv_expanded_end";
			else
				img = "tv_expanded";
		} else
		{
			if (tree[parent].next == HYP_NOINDEX)
				img = "tv_collapsed_end";
			else
				img = "tv_collapsed";
		}
		g_string_append_printf(out, "<span id=\"tv_icon_%u\" class=\"%s\"></span>", parent, img);
		g_string_append(out, "</a></span>");
	} else
	{
		if (tree[parent].next == HYP_NOINDEX)
			img = "tv_end";
		else
			img = "tv_intersec";
		g_string_append_printf(out, "<span class=\"%s\"></span>", img);
	}
#else
	if (tree[parent].num_childs != 0)
	{
		g_string_append_printf(out, "<span class=\"tv_expand\"><a class=\"tv_expand\" href=\"#\" onclick=\"return tv_toggleExpand('tv_sub_%u', '');\">", parent);
		if (tree[parent].flags & HYPTREE_IS_EXPANDED)
		{
			if (tree[parent].next == HYP_NOINDEX)
				img = line_expanded_end;
			else
				img = line_expanded;
		} else
		{
			if (tree[parent].next == HYP_NOINDEX)
				img = line_collapsed_end;
			else
				img = line_collapsed;
		}
		g_string_append(out, img);
		g_string_append(out, "</a></span>");
	} else
	{
		if (tree[parent].next == HYP_NOINDEX)
			img = line_end;
		else
			img = line_intersec;
		g_string_append(out, img);
	}
	g_string_append(out, "&nbsp;");
#endif
	g_string_append(out, "</span>");
	/* hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "%u: ", parent); */
	xref.line = 0;
	xref.dest_page = parent;
	entry = hyp->indextable[parent];
	xref.destfilename = html_filename_for_node(hyp, opts, parent, TRUE);
	xref.destname = tree[parent].name;
	xref.desttype = (hyp_indextype) entry->type;
	xref.text = html_quote_name(tree[parent].title, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
	html_generate_href(hyp, opts, out, &xref, NULL, FALSE, 0, NULL);
	g_free(xref.destfilename);
	g_free(xref.text);
	g_string_append(out, "\n");
	if (tree[parent].num_childs != 0 && (tree[parent].flags & HYPTREE_IS_EXPANDED))
	{
		g_string_append_printf(out, "<div id=\"tv_sub_%u\" style=\"display:block\">\n", parent);
		child = tree[parent].head;
		while (child != HYP_NOINDEX)
		{
			html_print_tree(hyp, opts, tree, out, child, depth + 1);
			child = tree[child].next;
		}
		g_string_append(out, "</div>\n");
	}

	g_string_append_printf(out, "</div>\n");
}

/* ------------------------------------------------------------------------- */

gboolean html_print_treeview(const char *filename, hcp_opts *opts, GString *out)
{
	HYP_DOCUMENT *hyp;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	hcp_opts hyp_opts;
	HYPTREE *tree;

	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (handle < 0)
	{
		html_out_header(NULL, opts, out, _("404 Not Found"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "%s: %s\n", hyp_basename(filename), hyp_utf8_strerror(errno));
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE, NULL);
		return FALSE;
	}

	hyp = hyp_load(filename, handle, &type);
	if (hyp == NULL)
	{
		html_out_header(NULL, opts, out, _("not a HYP file"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "%s: %s\n", hyp_basename(filename), _("not a HYP file"));
		html_out_trailer(NULL, opts, out, 0, TRUE, FALSE, NULL);
		return FALSE;
	}
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		html_out_header(NULL, opts, out, _("protected hypertext"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, _("fatal: %s: %s\n"), _("protected hypertext"), hyp_basename(filename));
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE, NULL);
		return FALSE;
	}

	tree = hyp_tree_build(hyp, handle);
	if (tree == NULL)
	{
		const char *err = hyp_utf8_strerror(errno);
		hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", filename, err);
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		html_out_header(NULL, opts, out, _("503 Service Unavailable"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, _("fatal: %s: %s\n"), hyp_basename(filename), err);
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE, NULL);
		return FALSE;
	}

	if (hyp->comp_vers > HCP_COMPILER_VERSION && opts->errorfile != stdout)
		hyp_utf8_fprintf(opts->errorfile, _("%s%s created by compiler version %u\n"), _("warning: "), hyp->file, hyp->comp_vers);
	opts->long_filenames = TRUE;
	if (opts->long_filenames)
	{
		g_free(opts->image_name_prefix);
		opts->image_name_prefix = replace_ext(hyp_basename(filename), NULL, "_img_");
	}

	hcp_opts_copy(&hyp_opts, opts);
	if (hyp->hcp_options != NULL)
	{
		/* TODO: redirect error messages from option parsing to HTML body */
		hcp_opts_parse_string(&hyp_opts, hyp->hcp_options, OPTS_FROM_SOURCE);
	}
	g_freep(&hyp_opts.output_filename);

	html_out_header(NULL, opts, out, _("TreeView"), HYP_NOINDEX, FALSE, NULL, NULL, FALSE, NULL);
	g_string_append(out, "\
<style type=\"text/css\">\n"
#if USE_TREEVIEW_SVG
".tv_blank {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-size: cover;\n"
"  background-repeat: no-repeat;\n"
"}\n"
".tv_intersec {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-image: url(\"" IMAGES_DIRS "tv_intersec.svg\");\n"
"  background-size: cover;\n"
"  background-position: left top;\n"
"  background-repeat: no-repeat;\n"
"}\n"
".tv_collapsed {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-image: url(\"" IMAGES_DIRS "tv_collapsed.svg\");\n"
"  background-size: cover;\n"
"  background-position: left top;\n"
"  background-repeat: no-repeat;\n"
"}\n"
".tv_collapsed_end {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-image: url(\"" IMAGES_DIRS "tv_collapsed_end.svg\");\n"
"  background-size: cover;\n"
"  background-position: left top;\n"
"  background-repeat: no-repeat;\n"
"}\n"
".tv_expanded {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-image: url(\"" IMAGES_DIRS "tv_expanded.svg\");\n"
"  background-size: cover;\n"
"  background-position: left top;\n"
"  background-repeat: no-repeat;\n"
"}\n"
".tv_expanded_end {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-image: url(\"" IMAGES_DIRS "tv_expanded_end.svg\");\n"
"  background-size: cover;\n"
"  background-position: left top;\n"
"  background-repeat: no-repeat;\n"
"}\n"
".tv_nointersec {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-image: url(\"" IMAGES_DIRS "tv_nointersec.svg\");\n"
"  background-size: cover;\n"
"  background-position: left top;\n"
"  background-repeat: no-repeat;\n"
"}\n"
".tv_end {\n"
"  display: inline-block;\n"
"  width: 32px;\n"
"  background-image: url(\"" IMAGES_DIRS "tv_end.svg\");\n"
"  background-size: cover;\n"
"  background-position: left top;\n"
"  background-repeat: no-repeat;\n"
"}\n"
#endif
".tv_expand, .tv_expand a:link, .tv_expand a:visited, .tv_expand a:active, .tv_expand:hover, .tv_expand:hover a:hover {\n"
"  display: inline;\n"
"  text-align: left;\n"
"  text-decoration: none;\n"
"  color: black;\n"
"}\n"
".hypview_treeview {\n"
#if USE_TREEVIEW_SVG
"  font-family: monospace;\n"
"  font-size: 14pt;\n"
#else
"  font-family: monospace;\n"
"  font-size: 12px;\n"
#endif
"  z-index:2;\n"
"}\n"
"</style>\n"
"<script type=\"text/javascript\">\n"
"function tv_toggleExpand(subelement, iconelement) {\n"
"	var tv_var = document.getElementById(subelement).style.display;\n"
"\n"
"	if (tv_var == \"none\") {\n"
"		tv_var = \"block\";\n"
"	} else {\n"
"		tv_var = \"none\";\n"
"	}\n"
"\n"
"	document.getElementById(subelement).style.display = tv_var;\n"
"\n"
"	if (iconelement != \"\") {\n"
"		var src = document.images[iconelement].src;\n"
"		var lastslash = src.lastIndexOf(\"/\");\n"
"		var dir = src.substr(0, lastslash + 1);\n"
"		var img = src.substring(lastslash + 1);\n"
"		var newimg = img;\n"
"		if (img == \"tv_expanded_end.png\")\n"
"			newimg = \"tv_collapsed_end.png\";\n"
"		if (img == \"tv_collapsed_end.png\")\n"
"			newimg = \"tv_expanded_end.png\";\n"
"		if (img == \"tv_expanded.png\")\n"
"			newimg = \"tv_collapsed.png\";\n"
"		if (img == \"tv_collapsed.png\")\n"
"			newimg = \"tv_expanded.png\";\n"
"		document.images[iconelement].src = dir + newimg;\n"
"	}\n"
"	return false;\n"
"}\n"
"</script>\n");

	html_print_tree(hyp, opts, tree, out, 0, 0);
	html_out_trailer(NULL, opts, out, HYP_NOINDEX, FALSE, FALSE, NULL);

	hyp_opts.outfile = NULL;
	hyp_opts.errorfile = NULL;
	hcp_opts_free(&hyp_opts);
	hyp_tree_free(hyp, tree);
	hyp_unref(hyp);
	hyp_utf8_close(handle);
	
	return TRUE;
}
