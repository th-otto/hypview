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
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#include "outcomm.h"
#include "outstg.h"
#include "outhtml.h"

#define STG_DEFAULT_PIC_TYPE HYP_PIC_IMG

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *stg_quote_name(const char *name, gboolean convslash)
{
	char *str, *ret;
	size_t len;
	
	if (name == NULL)
		return NULL;
	len = strlen(name);
	str = ret = g_new(char, len * 2 + 1);
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
					*str++ = c;
				}
				break;
			case '"':
				*str++ = '\\';
				*str++ = c;
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

static char *stg_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *p;
	char *buf;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	buf = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
	p = stg_quote_name(buf, entry->type == HYP_NODE_EXTERNAL_REF);
	g_free(buf);
	return p;
}

/* ------------------------------------------------------------------------- */

void stg_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, gboolean *converror)
{
	char *str;

	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "## created by %s Version %s\n", gl_program_name, gl_program_version);

	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@if VERSION >= 6\n");
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@os %s\n", hyp_osname(hyp->comp_os));
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@charset %s\n", hyp_charset_name(hyp->comp_charset));
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@inputenc %s\n", hyp_charset_name(opts->output_charset));
	if (hyp->language != NULL)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@lang \"%s\"\n", hyp->language);
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@endif\n");
	
	if (hyp->database != NULL)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@database \"%s\"\n", hyp->database);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@hostname \"%s\"\n", h->name);
		}
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = stg_quote_nodename(hyp, hyp->default_page);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@default \"%s\"\n", str);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@options \"%s\"\n", hyp->hcp_options);
	}
	if (hyp->author != NULL)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@author \"%s\"\n", hyp->author);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = stg_quote_nodename(hyp, hyp->help_page);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@help \"%s\"\n", str);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@$VER: %s\n", hyp->version);
	}
	if (hyp->subject != NULL)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@subject \"%s\"\n", hyp->subject);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@width %d\n", hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, _("@remark ST-Guide flags: $%04x\n"), hyp->st_guide_flags);
	}
	g_string_append_c(out, '\n');
	g_string_append_c(out, '\n');
}

/* ------------------------------------------------------------------------- */

static void stg_out_str(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(hyp->comp_charset, opts->output_charset, str, len, &converror);
	p = dst;
	if (opts->for_cgi || opts->recompile_format == HYP_FT_XML || opts->recompile_format == HYP_FT_HTML || opts->recompile_format == HYP_FT_HTML_XML)
	{
		while (*p)
		{
			switch (*p)
			{
			case '&':
				g_string_append(out, "&amp;");
				break;
			case '<':
				g_string_append(out, "&lt;");
				break;
			case '>':
				g_string_append(out, "&gt;");
				break;
			case '\'':
				g_string_append(out, "&apos;");
				break;
			case '"':
				g_string_append(out, "&quot;");
				break;
			case '@':
				g_string_append_c(out, '@');
				g_string_append_c(out, '@');
				break;
			case 0x01:
				g_string_append(out, "&0x2401;"); /* maybe uparrow? 0x21e7 */
				break;
			case 0x02:
				g_string_append(out, "&0x2402;"); /* maybe downarrow? 0x21e9 */
				break;
			case 0x03:
				g_string_append(out, "&0x2403;"); /* maybe rightarrow? 0x21e8 */
				break;
			case 0x04:
				g_string_append(out, "&0x2404;"); /* maybe leftarrow? 0x21e6 */
				break;
			case 0x05:
				g_string_append(out, "&0x2405;");
				break;
			case 0x06:
				g_string_append(out, "&0x2406;");
				break;
			case 0x07:
				g_string_append(out, "&0x2407;");
				break;
			case 0x08:
				g_string_append(out, "&0x2408;"); /* maybe checkmark? 0x2713 */
				break;
			case 0x09:
				g_string_append(out, "&0x2409;");
				break;
			case 0x0b:
				g_string_append(out, "&0x240b;");
				break;
			case 0x0c:
				g_string_append(out, "&0x240c;");
				break;
			case 0x0e:
				g_string_append(out, "&0x240e;");
				break;
			case 0x0f:
				g_string_append(out, "&0x240f;");
				break;
			case 0x10:
				g_string_append(out, "&0x2410;");
				break;
			case 0x11:
				g_string_append(out, "&0x2411;");
				break;
			case 0x12:
				g_string_append(out, "&0x2412;");
				break;
			case 0x13:
				g_string_append(out, "&0x2413;");
				break;
			case 0x14:
				g_string_append(out, "&0x2414;");
				break;
			case 0x15:
				g_string_append(out, "&0x2415;");
				break;
			case 0x16:
				g_string_append(out, "&0x2416;");
				break;
			case 0x17:
				g_string_append(out, "&0x2417;");
				break;
			case 0x18:
				g_string_append(out, "&0x2418;");
				break;
			case 0x19:
				g_string_append(out, "&0x2419;");
				break;
			case 0x1a:
				g_string_append(out, "&0x241a;");
				break;
			case 0x1b:
				g_string_append(out, "&0x241b;");
				break;
			case 0x1c:
				g_string_append(out, "&0x241c;");
				break;
			case 0x1d:
				g_string_append(out, "&0x241d;");
				break;
			case 0x1e:
				g_string_append(out, "&0x241e;");
				break;
			case 0x1f:
				g_string_append(out, "&0x241f;");
				break;
			case 0x7f:
				g_string_append(out, "&0x2421;");
				break;
			case 0x0a:
			case 0x0d:
			default:
				g_string_append_c(out, *p);
				break;
			}
			p++;
		}
	} else
	{
		while (*p)
		{
			if (*p == '@')
				g_string_append_c(out, '@'); /* AmigaGuide uses \@ for quoted '@' */
			g_string_append_c(out, *p);
			p++;
		}
	}
	g_free(dst);
}

/* ------------------------------------------------------------------------- */

static gboolean stg_out_attr(GString *out, struct textattr *attr)
{
	gboolean retval = FALSE;
	
	if (attr->curattr != attr->newattr)
	{
		g_string_append_c(out, '@');
		g_string_append_c(out, '{');
#define onoff(mask, on, off) \
		if ((attr->curattr ^ attr->newattr) & mask) \
			g_string_append_c(out, attr->newattr & mask ? on : off)
		onoff(HYP_TXT_BOLD, 'B', 'b');
		onoff(HYP_TXT_LIGHT, 'G', 'g');
		onoff(HYP_TXT_ITALIC, 'I', 'i');
		onoff(HYP_TXT_UNDERLINED, 'U', 'u');
		onoff(HYP_TXT_OUTLINED, 'O', 'o');
		onoff(HYP_TXT_SHADOWED, 'S', 's');
#undef onoff
		g_string_append_c(out, '}');
		attr->curattr = attr->newattr;
		retval = TRUE;
	}
	if (attr->curbg != attr->newbg)
	{
		attr->curbg = attr->newbg;
		g_string_append_printf(out, "@{BPEN %u}", attr->curbg);
	}
	if (attr->curfg != attr->newfg)
	{
		attr->curfg = attr->newfg;
		g_string_append_printf(out, "@{APEN %u}", attr->curfg);
	}
	return retval;
}

/* ------------------------------------------------------------------------- */

static void stg_out_gfx(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, gboolean *converror)
{
	switch (gfx->type)
	{
	case HYP_ESC_PIC:
		{
			char *fname;
			char *dithermask;
			char *colors;
			
			dithermask = format_dithermask(gfx->dithermask);
			if (!hypnode_valid(hyp, gfx->extern_node_index))
				fname = hyp_invalid_page(gfx->extern_node_index);
			else if (hyp->indextable[gfx->extern_node_index]->type != HYP_NODE_IMAGE)
				fname = g_strdup_printf(_("<non-image node #%u>"), gfx->extern_node_index);
			else
				fname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix, opts->ignore_image_name);
			colors = pic_colornameformat(gfx->planes);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@remark %ux%u%s\n", gfx->pixwidth, gfx->pixheight, colors);
			if (opts->for_cgi && opts->showstg)
			{
				html_out_stg_gfx(opts, out, hyp, gfx, fname, converror);
			} else
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "%s \"%s\" %d",
					gfx->islimage ? "@limage" : "@image",
					fname,
					gfx->x_offset);
			}
			if (!empty(dithermask))
				g_string_append_printf(out, " %%%s", dithermask);
			g_string_append_c(out, '\n');
			g_free(colors);
			g_free(fname);
			g_free(dithermask);
		}
		break;
	case HYP_ESC_LINE:
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@line %d %d %d %d %d\n",
			gfx->x_offset, gfx->width, gfx->height,
			gfx->begend,
			gfx->style);
		break;
	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "%s %d %d %d %d\n",
			gfx->type == HYP_ESC_BOX ? "@box" : "@rbox",
			gfx->x_offset, gfx->width, gfx->height, gfx->style);
		break;
	}
}

/* ------------------------------------------------------------------------- */

static void stg_out_graphics(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, struct hyp_gfx *gfx, long lineno, gboolean *converror)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			stg_out_gfx(opts, out, hyp, gfx, converror);
		}
		gfx = gfx->next;
	}
}

/* ------------------------------------------------------------------------- */

static const char *sym_typename(const symtab_entry *sym)
{
	if (sym->from_ref && sym->from_idx)
		return "ari";
	if (sym->from_ref)
		return "ar";
	if (sym->from_idx)
		return "i";
	return "a";
}

/* ------------------------------------------------------------------------- */

static void stg_out_labels(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms, gboolean *converror)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_LABELNAME);
	while (sym)
	{
		if (sym->lineno == lineno)
		{
			char *str = stg_quote_name(sym->name, FALSE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@symbol %s \"%s\"\n", sym_typename(sym), str);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void stg_out_alias(GString *out, HYP_DOCUMENT *hyp, hcp_opts *opts, const INDEX_ENTRY *entry, symtab_entry *syms, gboolean *converror)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = stg_quote_name(sym->name, FALSE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@symbol %s \"%s\"\n", sym_typename(sym), str);
		g_free(str);
		sym->referenced = TRUE;
		sym = sym_find(sym->next, nodename, REF_ALIASNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

gboolean stg_out_nodedata(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, HYP_NODE *nodeptr, symtab_entry *syms, gboolean *converror)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	struct textattr attr;
	long lineno;
	gboolean retval = TRUE;
	hyp_nodenr node = nodeptr->number;
	const unsigned char *src;
	const unsigned char *end;
	const unsigned char *textstart;
	INDEX_ENTRY *entry;
 	unsigned short dithermask;
	struct hyp_gfx *hyp_gfx = NULL;
	
	entry = hyp->indextable[node];
	src = nodeptr->start;
	end = nodeptr->end;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		stg_out_str(hyp, opts, out, textstart, src - textstart); \
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
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "@endtree\n"); \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	hyp_node_find_windowtitle(nodeptr);
	
	str = stg_quote_nodename(hyp, node);
	if (nodeptr->window_title)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "%s \"%s\" \"", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str);
		if (opts->for_cgi || opts->recompile_format == HYP_FT_XML || opts->recompile_format == HYP_FT_HTML || opts->recompile_format == HYP_FT_HTML_XML)
		{
			stg_out_str(hyp, opts, out, nodeptr->window_title, STR0TERM);
		} else
		{
			char *buf = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
			char *title = stg_quote_name(buf, FALSE);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "%s", title);
			g_free(title);
			g_free(buf);
		}
		g_string_append(out, "\"\n");
	} else
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "%s \"%s\"\n", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str);
	}
	g_free(str);
	/*
	 * check for alias names in ref file
	 */
	stg_out_alias(out, hyp, opts, entry, syms, converror);
	
	if (entry->type == HYP_NODE_INTERNAL)
	{
		if (hypnode_valid(hyp, entry->next) &&
			/* physical next page is default if not set */
			(node + 1u) != entry->next)
		{
			str = stg_quote_nodename(hyp, entry->next);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@next \"%s\"\n", str);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->previous) &&
			 /* physical prev page is default if not set */
			(node - 1u) != entry->previous &&
			(entry->previous != 0 || node != 0))
		{
			str = stg_quote_nodename(hyp, entry->previous);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@prev \"%s\"\n", str);
			g_free(str);
		}
		
		if (hypnode_valid(hyp, entry->toc_index) &&
			/* physical first page is default if not set */
			entry->toc_index != 0)
		{
			str = stg_quote_nodename(hyp, entry->toc_index);
			hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@toc \"%s\"\n", str);
			g_free(str);
		}
	}
			
	if (node == hyp->index_page)
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@autorefon\n");

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
						adm->format = format_from_pic(opts, hyp->indextable[adm->extern_node_index], STG_DEFAULT_PIC_TYPE);
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
				char *text;
				char *buf;
				
				dest_page = DEC_255(&src[3]);
				buf = hyp_conv_to_utf8(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
				text = stg_quote_name(buf, FALSE);
				g_free(buf);
				text = chomp(text);
				if (hypnode_valid(hyp, dest_page))
				{
					str = stg_quote_nodename(hyp, dest_page);
				} else
				{
					str = hyp_invalid_page(dest_page);
				}
				if (empty(text) || strcmp(str, text) == 0)
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@xref \"%s\"\n", str);
				else
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@xref \"%s\" \"%s\"\n", str, text);
				g_free(str);
				g_free(text);
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
	stg_out_labels(hyp, opts, out, entry, lineno, syms, converror);
	stg_out_graphics(hyp, opts, out, hyp_gfx, lineno, converror);
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
				g_string_append_c(out, 0x1b);
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
					hyp_nodenr dest_page;
					unsigned char type;
					hyp_lineno line = 0;
					char *dest;
					size_t len;
					gboolean str_equal;
					const char *cmd;
					gboolean print_target;
					hyp_indextype desttype;
					
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
						desttype = (hyp_indextype) hyp->indextable[dest_page]->type;
						switch (desttype)
						{
						default:
						case HYP_NODE_EOF:
							if (opts->print_unknown)
								hyp_utf8_fprintf(opts->errorfile, _("link to unknown node type %u\n"), hyp->indextable[dest_page]->type);
							/* fall through */
						case HYP_NODE_INTERNAL:
						case HYP_NODE_POPUP:
							cmd = type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE ? "ALINK" : "LINK";
							break;
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
						dest = hyp_invalid_page(dest_page);
						desttype = HYP_NODE_EOF;
						cmd = "LINK";
					}
					if (*src <= HYP_STRLEN_OFFSET)
					{
						src++;
						str = g_strdup(dest);
						if (hypnode_valid(hyp, dest_page))
						{
							INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
							len = dest_entry->length - SIZEOF_INDEX_ENTRY;
							textstart = dest_entry->name;
							str_equal = dest_entry->type == HYP_NODE_INTERNAL;
						} else
						{
							textstart = (const unsigned char *)str;
							len = strlen(str);
							str_equal = FALSE;
						}
					} else
					{
						char *buf;
						
						len = *src - HYP_STRLEN_OFFSET;
						src++;
						textstart = src;
						buf = hyp_conv_to_utf8(hyp->comp_charset, src, len);
						str = stg_quote_name(buf, FALSE);
						g_free(buf);
						src += len;
						if (hypnode_valid(hyp, dest_page))
						{
							INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
							str_equal = dest_entry->type == HYP_NODE_INTERNAL && strcmp(str, dest) == 0;
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
							hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@{\"%s\" %s}", str, cmd);
						else if (!str_equal /* || node == hyp->index_page */ || opts->all_links)
							hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@{\"%s\" %s \"%s\"}", str, cmd, dest);
						else
							stg_out_str(hyp, opts, out, textstart, len);
						break;
					case HYP_ESC_LINK_LINE:
					case HYP_ESC_ALINK_LINE:
						{
							gboolean is_rsc_link = FALSE;
							char *p = ((desttype == HYP_NODE_EXTERNAL_REF && (hyp->st_guide_flags & STG_ALLOW_FOLDERS_IN_XREFS)) ? strrslash : strslash)(dest);
							if (p != NULL && strcmp(p + 1, "MAIN") == 0)
							{
								*p = '\0';
								if (hyp_guess_filetype(dest) == HYP_FT_RSC)
								{
									hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@{\"%s\" %s \"%s/%u\"}", str, cmd, dest, line);
									is_rsc_link = TRUE;
								} else
								{
									*p = '/';
								}
							}
							if (!is_rsc_link)
							{
								symtab_entry *sym;
								const char *label;
								
								label = NULL;
								/*
								 * fixme: dest is already quoted here, sym->nodename is not
								 */
								sym = sym_find(syms, dest, REF_LABELNAME);
								while (sym)
								{
									if (sym->lineno == line && !sym->from_idx)
									{
										label = sym->name;
										sym->referenced = TRUE;
										break;
									}
									sym = sym_find(sym->next, dest, REF_LABELNAME);
								}
								if (label)
								{
									char *quoted = stg_quote_name(label, FALSE);
									hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@{\"%s\" %s \"%s\" \"%s\"}", str, cmd, dest, quoted);
									g_free(quoted);
								} else
								{
									hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@{\"%s\" %s \"%s\" %u}", str, cmd, dest, line + 1);
								}
							}
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
						str = hyp_invalid_page(dest_page);
					}
					FLUSHLINE();
					if (tree != in_tree)
					{
						FLUSHTREE();
						hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@tree %d\n", tree);
						in_tree = tree;
					}
					hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "   %d \"%s\"", obj, str);
					if (line != 0)
						hyp_utf8_sprintf_charset(out, opts->output_charset, converror, " %u", line);
					g_string_append_c(out, '\n');
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
				if (stg_out_attr(out, &attr))
					at_bol = FALSE;
				src++;
				break;
			
			case HYP_ESC_FG_COLOR:
				FLUSHTREE();
				src++;
				attr.newfg = *src;
				if (stg_out_attr(out, &attr))
					at_bol = FALSE;
				src++;
				break;
				
			case HYP_ESC_BG_COLOR:
				FLUSHTREE();
				src++;
				attr.newbg = *src;
				if (stg_out_attr(out, &attr))
					at_bol = FALSE;
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
			stg_out_labels(hyp, opts, out, entry, lineno, syms, converror);
			stg_out_graphics(hyp, opts, out, hyp_gfx, lineno, converror);
			src++;
			textstart = src;
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
	if (stg_out_attr(out, &attr))
		at_bol = FALSE;
	FLUSHLINE();
	FLUSHTREE();
	++lineno;
	stg_out_labels(hyp, opts, out, entry, lineno, syms, converror);
	stg_out_graphics(hyp, opts, out, hyp_gfx, lineno, converror);
	
	if (hyp_gfx != NULL)
	{
		struct hyp_gfx *gfx, *next;
		
		for (gfx = hyp_gfx; gfx != NULL; gfx = next)
		{
			if (!gfx->used)
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "## gfx unused: ");
				stg_out_gfx(opts, out, hyp, gfx, converror);
			}
			next = gfx->next;
			g_free(gfx);
		}
	}
	
	hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "@endnode\n");
	if (node < hyp->last_text_page)
		hyp_utf8_sprintf_charset(out, opts->output_charset, converror, "\n\n");
	
#undef DUMPTEXT
#undef FLUSHLINE
#undef FLUSHTREE
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean stg_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, symtab_entry *syms, gboolean *converror)
{
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		retval = stg_out_nodedata(hyp, opts, out, nodeptr, syms, converror);
		
		hyp_node_free(nodeptr);
	} else
	{
		/*
		 * error message was already issued in sym_check_links
		 */
		/* hyp_utf8_fprintf(opts->errorfile, _("%s: Node %u: failed to decode\n"), hyp->file, node); */
	}

	return retval;
}

/* ------------------------------------------------------------------------- */

gboolean recompile_stg(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	GString *out;
	gboolean converror = FALSE;

	UNUSED(argc);
	UNUSED(argv);
	
	/* output_charset = HYP_CHARSET_ATARI; */
	force_crlf = (opts->outfile == stdout || opts->output_charset != HYP_CHARSET_ATARI) ? FALSE : TRUE;
	
	ret = TRUE;
	
	out = g_string_new(NULL);

	stg_out_globals(hyp, opts, out, &converror);
	if (opts->read_images && hyp->cache == NULL)
		InitCache(hyp);
	
	/* load REF if not done already */
	syms = ref_loadsyms(hyp);
	
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
			ret &= stg_out_node(hyp, opts, out, node, syms, &converror);
			break;
		case HYP_NODE_POPUP:
			ret &= stg_out_node(hyp, opts, out, node, syms, &converror);
			break;
		case HYP_NODE_IMAGE:
			if (opts->read_images)
				ret &= write_image(hyp, opts, node, STG_DEFAULT_PIC_TYPE, NULL);
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
				hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "## symbol unused: \"%s\" \"%s\"\n", sym->nodename, sym->name);
			}
		}
		write_strout(out, opts->outfile);
	}
		
	g_string_free(out, TRUE);
	
	free_symtab(syms);
	return ret;
}
