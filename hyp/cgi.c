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

/*
 * This is the main CGI program that is invoked from
 * the HypView web service.
 */

#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#define CURL_DISABLE_DEPRECATION
#include <curl/curl.h>
#include <sys/time.h>
#include <utime.h>
#include "cgic.h"
#include "outcomm.h"
#include "outstg.h"
#include "outhtml.h"
#include "stat_.h"
#include "cgirsc.h"
#include "bm.h"
#include "hv_vers.h"

char const gl_program_name[] = "hypview.cgi";
char const gl_program_version[] = HYP_VERSION;

static char const cgi_cachedir[] = "cache";

struct curl_parms {
	const char *filename;
	FILE *fp;
	hcp_opts *opts;
};

#define ALLOWED_PROTOS ( \
	CURLPROTO_FTP | \
	CURLPROTO_FTPS | \
	CURLPROTO_HTTP | \
	CURLPROTO_HTTPS | \
	CURLPROTO_SCP | \
	CURLPROTO_SFTP | \
	CURLPROTO_TFTP)

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

struct hypfind_opts {
	gboolean casesensitive;
	gboolean wordonly;
	char *pattern;
	size_t pattern_len;
	gboolean multiple;
	unsigned long filecount;
	unsigned long pagehits;
	unsigned long hits;
	unsigned long total_hits;
	BM_TABLE deltapat;
};

/* ------------------------------------------------------------------------- */

static gboolean is_word_char(unsigned char ch)
{
	if (ch >= 0x80)
		return TRUE;
	if (strchr(gl_profile.hypfind.wordchars, ch) != NULL)
		return TRUE;
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static void search_out_str(hcp_opts *opts, GString *out, const char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(HYP_CHARSET_UTF8, opts->output_charset, str, len, &converror);
	p = html_quote_name(dst, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
	g_string_append(out, p);
	g_free(p);
	g_free(dst);
}

/* ------------------------------------------------------------------------- */

static void search_text(HYP_DOCUMENT *hyp, hcp_opts *opts, struct hypfind_opts *search, const char *text, size_t textlen, GString *out, HYP_NODE *nodeptr)
{
	char *title;
	char *destname;
	char *destfilename;
	const char *match;
	const char *scan = text;
	size_t scanlen = textlen;
	const char *style = "";
	INDEX_ENTRY *dest_entry;
	size_t offset;

	for (;;)
	{
		if (scanlen == 0)
			return;
		if (search->casesensitive)
			match = bm_scanner(&search->deltapat, scan, scanlen);
		else if (search->deltapat.slowcase)
			match = hyp_utf8_strcasestr(scan, search->pattern);
		else
			match = bm_casescanner(&search->deltapat, scan, scanlen);
		if (match == NULL)
			return;
		if (!search->wordonly)
			break;
		offset = match - scan;
		if ((offset == 0 || !is_word_char(match[-1])) &&
			(offset + search->pattern_len >= scanlen || !is_word_char(match[search->pattern_len])))
			break;
		offset += search->pattern_len;
		scan += offset;
		ASSERT(scanlen >= offset);
		scanlen -= offset;
	}
	dest_entry = hyp->indextable[nodeptr->number];
	destname = hyp_conv_to_utf8(hyp->comp_charset, dest_entry->name, dest_entry->length - SIZEOF_INDEX_ENTRY);
	destfilename = html_filename_for_node(hyp, opts, nodeptr->number, TRUE);
	if (search->pagehits == 0)
	{
		if (search->hits != 0)
		{
			g_string_append_c(out, '\n');
		} else
		{
			title = html_quote_name(gl_profile.hypfind.title, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
			html_out_header(NULL, opts, out, title, HYP_NOINDEX, FALSE, NULL, NULL, FALSE, NULL);
			g_free(title);
		}
		if (nodeptr->window_title)
		{
			char *buf = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
			title = html_quote_name(buf, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE|QUOTE_NOLTR : 0);
			g_free(buf);
		} else
		{
			title = html_quote_nodename(hyp, nodeptr->number, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		}
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<a%s href=\"%s\">%s</a>\n", style, destfilename, title);

		g_free(title);
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, "<a%s href=\"%s#%s%ld\">%ld:</a> ", style, destfilename, hypview_lineno_tag, nodeptr->height, nodeptr->height);
	offset = match - text;
	search_out_str(opts, out, text, offset);
	g_string_append_printf(out, "<span class=\"%s\">", html_attr_bold_style);
	search_out_str(opts, out, match, search->pattern_len);
	g_string_append_printf(out, "</span>");
	search_out_str(opts, out, match + search->pattern_len, textlen - offset - search->pattern_len);
	g_string_append_c(out, '\n');
	g_free(destfilename);
	g_free(destname);
	
	search->pagehits++;
	search->hits++;
}

/* ------------------------------------------------------------------------- */

static gboolean search_node(HYP_DOCUMENT *hyp, hcp_opts *opts, struct hypfind_opts *search, GString *out, HYP_NODE *nodeptr)
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
	search->pagehits = 0;
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
			
			case HYP_ESC_FG_COLOR:
			case HYP_ESC_BG_COLOR:
				src += 2;
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
			DUMPTEXT();
			++nodeptr->height;
			text[textlen] = '\0';
			search_text(hyp, opts, search, text, textlen, out, nodeptr);
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
		search_text(hyp, opts, search, text, textlen, out, nodeptr);
	}
	
error:
	g_free(text);
	
#undef DUMPTEXT
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean search_hyp(HYP_DOCUMENT *hyp, hcp_opts *opts, struct hypfind_opts *search, GString *out, hyp_nodenr node)
{
	hyp_nodenr node_num;
	INDEX_ENTRY *entry;
	gboolean retval = TRUE;
	HYP_NODE *nodeptr;

	for (node_num = 0; node_num < hyp->num_index; node_num++)
	{
		entry = hyp->indextable[node_num];
		if (node_num == hyp->index_page)
			continue;
		if (node != HYP_NOINDEX && node_num != node)
			continue;
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
			nodeptr = hyp_loadtext(hyp, node_num);
			if (nodeptr != NULL)
			{
				retval &= search_node(hyp, opts, search, out, nodeptr);
				hyp_node_free(nodeptr);
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

	if (cmdline_version)
		ClearCache(hyp);

	if (search->hits == 0)
	{
		html_out_header(NULL, opts, out, _("No match"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
		hyp_utf8_sprintf_charset(out, opts->output_charset, NULL, _("%s: No matches for this pattern!\n"), _("error"));
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE, NULL);
	} else
	{
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, FALSE, FALSE, NULL);
	}
	
	return retval;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean recompile_html_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr output_node, hyp_pic_format *pic_format)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	char *destname;
	gboolean converror = FALSE;
	
	ret = TRUE;

	if (output_node == HYP_NOINDEX)
		output_node = 0;

	if (hyp->first_text_page == HYP_NOINDEX)
	{
		html_out_header(NULL, opts, out, _("Corrupt HYP file"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, &converror);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _("%s%s does not have any text pages\n"), _("error: "), hyp_basename(hyp->file));
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE, &converror);
		return FALSE;
	}
	if (!hypnode_valid(hyp, output_node))
	{
		html_out_header(NULL, opts, out, _("Invalid Node index"), output_node, FALSE, NULL, NULL, TRUE, &converror);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _("node index %u invalid\n"), output_node);
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE, &converror);
		return FALSE;
	}
			
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
	
	node = output_node;
	entry = hyp->indextable[node];
	switch ((hyp_indextype) entry->type)
	{
	case HYP_NODE_INTERNAL:
	case HYP_NODE_POPUP:
		ret &= html_out_node(hyp, opts, out, node, syms, FALSE, &converror);
		if (out->len == 0)
		{
			html_out_header(NULL, opts, out, _("Error"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, &converror);
			hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _("%s: Node %u: failed to decode\n"), hyp_basename(hyp->file), node);
			html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE, &converror);
		}
		break;
	case HYP_NODE_IMAGE:
		{
			ret &= write_image(hyp, opts, node, HTML_DEFAULT_PIC_TYPE, out);
			if (ret)
			{
				*pic_format = format_from_pic(opts, entry, HTML_DEFAULT_PIC_TYPE);
			} else
			{
				hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "%s", hyp_utf8_strerror(errno));
			}
		}
		break;
	case HYP_NODE_EXTERNAL_REF:
		html_out_header(NULL, opts, out, "@{ link }", node, FALSE, NULL, syms, FALSE, NULL);
		destname = html_quote_nodename(hyp, node, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "@{ link \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE, NULL);
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		html_out_header(NULL, opts, out, "@{ system }", node, FALSE, NULL, syms, FALSE, NULL);
		destname = html_quote_nodename(hyp, node, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "@{ system \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE, NULL);
		break;
	case HYP_NODE_REXX_SCRIPT:
		html_out_header(NULL, opts, out, "@{ rxs }", node, FALSE, NULL, syms, FALSE, NULL);
		destname = html_quote_nodename(hyp, node, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "@{ rxs \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE, NULL);
		break;
	case HYP_NODE_REXX_COMMAND:
		html_out_header(NULL, opts, out, "@{ rx }", node, FALSE, NULL, syms, FALSE, NULL);
		destname = html_quote_nodename(hyp, node, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "@{ rx \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE, NULL);
		break;
	case HYP_NODE_QUIT:
		html_out_header(NULL, opts, out, "@{ quit }", node, FALSE, NULL, syms, FALSE, NULL);
		destname = html_quote_nodename(hyp, node, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "@{ quit \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE, NULL);
		break;
	case HYP_NODE_CLOSE:
		html_out_header(NULL, opts, out, "@{ close }", node, FALSE, NULL, syms, FALSE, NULL);
		destname = html_quote_nodename(hyp, node, opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, "@{ close \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE, NULL);
		break;
	case HYP_NODE_EOF:
	default:
		if (opts->print_unknown)
			hyp_utf8_fprintf(opts->errorfile, _("unknown index entry type %u\n"), entry->type);
		break;
	}
	
	if (cmdline_version)
		ClearCache(hyp);
	
	free_symtab(syms);
	return ret;
}

/* ------------------------------------------------------------------------- */

static char *ref_name(REF_FILE *ref, REF_MODULE *mod, const REF_ENTRY *entry, char **freeme, unsigned int quote_flags)
{
	char *tmp;
	
	if (ref->is_converted || mod->charset == HYP_CHARSET_UTF8)
	{
		tmp = entry->name.utf8;
		*freeme = NULL;
	} else
	{
		tmp = hyp_conv_to_utf8(mod->charset, entry->name.hyp, STR0TERM);
		*freeme = tmp;
	}
	return html_quote_name(tmp, quote_flags);
}

/* ------------------------------------------------------------------------- */

static void html_ref_list(REF_FILE *ref, hcp_opts *opts, GString *out, gboolean all)
{
	REF_MODULE *mod;
	long num;
	char *str;
	char *freeme;
	gboolean converror = FALSE;
	unsigned int quote_flags = opts->output_charset == HYP_CHARSET_UTF8 ? QUOTE_UNICODE : 0;
	
	for (mod = ref->modules; mod != NULL; mod = mod->next)
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _("Hypertext file: %s with %ld entries\n"),
			mod->filename ? mod->filename : _("<unnamed file>"),
			mod->num_entries);
		for (num = 0; num < mod->num_entries; num++)
		{
			const REF_ENTRY *entry = &mod->entries[num];
			
			switch (entry->type)
			{
			case REF_FILENAME:
				if (all)
				{
					str = ref_name(ref, mod, entry, &freeme, quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Module         %s\n"), str);
					g_free(str);
					g_free(freeme);
				}
				break;
			case REF_NODENAME:
				if (all)
				{
					str = ref_name(ref, mod, entry, &freeme, quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Node           %s\n"), str);
					g_free(str);
					g_free(freeme);
				}
				break;
			case REF_ALIASNAME:
				if (all)
				{
					str = ref_name(ref, mod, entry, &freeme, quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Alias          %s\n"), str);
					g_free(str);
					g_free(freeme);
				}
				break;
			case REF_LABELNAME:
				if (all)
				{
					str = ref_name(ref, mod, entry, &freeme, quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Label %5u    %s\n"), mod->entries[num].lineno, str);
					g_free(str);
					g_free(freeme);
				}
				break;
			case REF_DATABASE:
				if (all)
				{
					str = html_quote_name(mod->database, quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Database       %s\n"), str);
					g_free(str);
				}
				break;
			case REF_OS:
				if (all)
				{
					str = html_quote_name(hyp_osname(mod->os), quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" OS             %s\n"), str);
					g_free(str);
				}
				break;
			case REF_CHARSET:
				if (all)
				{
					str = html_quote_name(hyp_charset_name(mod->charset), quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Charset        %s\n"), str);
					g_free(str);
				}
				break;
			case REF_LANGUAGE:
				if (all)
				{
					str = html_quote_name(mod->language, quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Language       %s\n"), str);
					g_free(str);
				}
				break;
			case REF_TITLE:
				if (all)
				{
					str = ref_name(ref, mod, entry, &freeme, quote_flags);
					hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Title          %s\n"), str);
					g_free(str);
					g_free(freeme);
				}
				break;
			case REF_UNKNOWN:
			default:
				str = ref_name(ref, mod, entry, &freeme, quote_flags);
				hyp_utf8_sprintf_charset(out, opts->output_charset, &converror, _(" Unknown REF type %u: %s\n"), entry->type, str);
				g_free(str);
				g_free(freeme);
				break;
			}
		}
	}
}

/* ------------------------------------------------------------------------- */

static gboolean recompile(const char *filename, hcp_opts *opts, struct hypfind_opts *search, GString *out, hyp_nodenr node, hyp_pic_format *pic_format)
{
	gboolean retval;
	HYP_DOCUMENT *hyp;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	hcp_opts hyp_opts;

	*pic_format = HYP_PIC_UNKNOWN;
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
		REF_FILE *ref = ref_load(filename, handle, FALSE);

		hyp_utf8_close(handle);
		if (ref != NULL)
		{
			char *title = g_strdup_printf(_("Listing of %s"), hyp_basename(filename));
			html_out_header(NULL, opts, out, title, HYP_NOINDEX, FALSE, NULL, NULL, FALSE, NULL);
			html_ref_list(ref, opts, out, TRUE);
			ref_close(ref);
			html_out_trailer(NULL, opts, out, 0, FALSE, FALSE, NULL);
			return TRUE;
		}
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
	if (search->pattern)
		retval = search_hyp(hyp, &hyp_opts, search, out, node);
	else
		retval = recompile_html_node(hyp, &hyp_opts, out, node, pic_format);
	hyp_opts.outfile = NULL;
	hyp_opts.errorfile = NULL;
	hcp_opts_free(&hyp_opts);
	hyp_unref(hyp);
	hyp_utf8_close(handle);
	
	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean uri_has_scheme(const char *uri)
{
	gboolean colon = FALSE;
	
	if (uri == NULL)
		return FALSE;
	while (*uri)
	{
		if (*uri == ':')
			colon = TRUE;
		else if (*uri == '/')
			return colon;
		uri++;
	}
	return colon;
}

/* ------------------------------------------------------------------------- */

static void html_out_response_header(FILE *out, HYP_CHARSET charset, unsigned long len, hyp_pic_format pic_format, gboolean xml)
{
	const char *mimetype;
	
	mimetype = hcp_pic_format_to_mimetype(pic_format);
	if (mimetype)
	{
		fprintf(out, "Content-Type: %s\015\012", mimetype);
	} else
	{
		fprintf(out, "Content-Type: %s;charset=%s\015\012", xml ? "application/xhtml+xml" : "text/html", hyp_charset_name(charset));
	}
	fprintf(out, "Content-Length: %lu\015\012", len);
	fprintf(out, "\015\012");
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static size_t mycurl_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct curl_parms *parms = (struct curl_parms *) userdata;
	
	if (size == 0 || nmemb == 0)
		return 0;
	if (parms->fp == NULL)
	{
		parms->fp = hyp_utf8_fopen(parms->filename, "wb");
		if (parms->fp == NULL)
			hyp_utf8_fprintf(parms->opts->errorfile, "%s: %s\n", parms->filename, hyp_utf8_strerror(errno));
	}
	if (parms->fp == NULL)
		return 0;
	return fwrite(ptr, size, nmemb, parms->fp);
}

/* ------------------------------------------------------------------------- */

static int mycurl_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userdata)
{
	struct curl_parms *parms = (struct curl_parms *) userdata;

	UNUSED(handle);
	switch (type)
	{
	case CURLINFO_TEXT:
		hyp_utf8_fprintf(parms->opts->errorfile, "== Info: %s", data);
		if (size == 0 || data[size - 1] != '\n')
			fputc('\n', parms->opts->errorfile);
		break;
	case CURLINFO_HEADER_OUT:
		hyp_utf8_fprintf(parms->opts->errorfile, "=> Send header %ld\n", (long)size);
		fwrite(data, 1, size, parms->opts->errorfile);
		break;
	case CURLINFO_DATA_OUT:
		hyp_utf8_fprintf(parms->opts->errorfile, "=> Send data %ld\n", (long)size);
		break;
	case CURLINFO_SSL_DATA_OUT:
		hyp_utf8_fprintf(parms->opts->errorfile, "=> Send SSL data %ld\n", (long)size);
		break;
	case CURLINFO_HEADER_IN:
		hyp_utf8_fprintf(parms->opts->errorfile, "<= Recv header %ld\n", (long)size);
		fwrite(data, 1, size, parms->opts->errorfile);
		break;
	case CURLINFO_DATA_IN:
		hyp_utf8_fprintf(parms->opts->errorfile, "<= Recv data %ld\n", (long)size);
		break;
	case CURLINFO_SSL_DATA_IN:
		hyp_utf8_fprintf(parms->opts->errorfile, "<= Recv SSL data %ld\n", (long)size);
		break;
	case CURLINFO_END:
	default:
		break;
 	}
	return 0;
}

/* ------------------------------------------------------------------------- */

static const char *currdate(void)
{
	struct tm *tm;
	static char buf[40];
	time_t t;
	t = time(NULL);
	tm = localtime(&t);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
	return buf;
}
	
/* ------------------------------------------------------------------------- */

static char *curl_download(CURL *curl, hcp_opts *opts, GString *body, const char *filename)
{
	char *local_filename;
	struct curl_parms parms;
	struct stat st;
	long respcode;
	CURLcode curlcode;
	char err[CURL_ERROR_SIZE];
	char *content_type;

	curl_easy_setopt(curl, CURLOPT_URL, filename);
	curl_easy_setopt(curl, CURLOPT_REFERER, filename);
	local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
	parms.filename = local_filename;
	parms.fp = NULL;
	parms.opts = opts;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mycurl_write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &parms);
	curl_easy_setopt(curl, CURLOPT_STDERR, opts->errorfile);
	curl_easy_setopt(curl, CURLOPT_PROTOCOLS, ALLOWED_PROTOS);
	curl_easy_setopt(curl, CURLOPT_ENCODING, "");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, (long)1);
	curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);
	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, mycurl_trace);
	curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &parms);
	*err = 0;
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err);
	
	/* set this to 1 to activate debug code above */
	curl_easy_setopt(curl, CURLOPT_VERBOSE, (long)0);

	if (hyp_utf8_stat(local_filename, &st) == 0)
	{
		curlcode = curl_easy_setopt(curl, CURLOPT_TIMECONDITION, (long)CURL_TIMECOND_IFMODSINCE);
		curlcode = curl_easy_setopt(curl, CURLOPT_TIMEVALUE, (long)st.st_mtime);
	}
	
	/*
	 * TODO: reject attempts to connect to local addresses
	 */
	curlcode = curl_easy_perform(curl);
	
	{
		long unmet;

#if LIBCURL_VERSION_NUM >= 0x073700 /* 7.55.0 */
		off_t size = 0;
		curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &size);
#else
		double size = 0;
		curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size);
#endif
		respcode = 0;
		unmet = -1;
		content_type = NULL;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
		curl_easy_getinfo(curl, CURLINFO_CONDITION_UNMET, &unmet);
		curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
		hyp_utf8_fprintf(opts->errorfile, "%s: GET from %s, url=%s, curl=%d, resp=%ld, size=%ld, content=%s\n", currdate(), fixnull(cgiRemoteHost), filename, curlcode, respcode, (long)size, printnull(content_type));
	}
	
	if (parms.fp)
	{
		hyp_utf8_fclose(parms.fp);
		parms.fp = NULL;
	}
	
	if (curlcode != CURLE_OK || stat(local_filename, &st) != 0)
	{
		html_out_header(NULL, opts, body, err, HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
		g_string_append_printf(body, "%s:\n%s", _("Download error"), err);
		html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE, NULL);
		unlink(local_filename);
		g_free(local_filename);
		local_filename = NULL;
	} else if ((respcode != 200 && respcode != 304) ||
		(respcode == 200 && content_type != NULL && strcmp(content_type, "text/plain") == 0))
	{
		/* most likely the downloaded data will contain the error page */
		parms.fp = hyp_utf8_fopen(local_filename, "rb");
		if (parms.fp != NULL)
		{
			size_t nread;
			
			while ((nread = fread(err, 1, sizeof(err), parms.fp)) > 0)
				g_string_append_len(body, err, nread);
			hyp_utf8_fclose(parms.fp);
		}
		unlink(local_filename);
		g_free(local_filename);
		local_filename = NULL;
		/*
		 * FIXME: if we got plain text, we must also annnounce that in the response header
		 */
		opts->recompile_format = HYP_FT_HTML;
	} else
	{
		long ft = -1;
		if (curl_easy_getinfo(curl, CURLINFO_FILETIME, &ft) == CURLE_OK && ft != -1)
		{
			struct utimbuf ut;
			ut.actime = ut.modtime = ft;
			utime(local_filename, &ut);
		}
	}
	
	return local_filename;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void mk_output_dir(hcp_opts *opts)
{
	char *dir = hyp_path_get_dirname(cgiScriptFilename);
	char *cache_dir = g_build_filename(dir, cgi_cachedir, NULL);
	opts->output_dir = g_build_filename(cache_dir, cgiRemoteAddr, NULL);

	if (mkdir(cache_dir, 0750) < 0 && errno != EEXIST)
		fprintf(opts->errorfile, "%s: %s\n", cache_dir, strerror(errno));
	if (mkdir(opts->output_dir, 0750) < 0 && errno != EEXIST)
		fprintf(opts->errorfile, "%s: %s\n", opts->output_dir, strerror(errno));
	
	g_free(cache_dir);
	g_free(dir);
}
	
/* ------------------------------------------------------------------------- */

#if defined(__LINUX_GLIBC_WRAP_H)

/* ugly hack to get __libc_start_main versioned */

#if __GLIBC_PREREQ(2, 34)

#define STR_(s) #s
#define STR(s)  STR_(s)
#include <dlfcn.h>

#ifdef __UCLIBC__
#define __libc_start_main       __uClibc_main
#endif

int __libc_start_main(
        int (*main)(int,char**,char**), int ac, char **av,
        int (*init)(void), void (*fini)(void),
        void (*rtld_fini)(void), void *stack_end);
int __libc_start_main(
        int (*main)(int,char**,char**), int ac, char **av,
        int (*init)(void), void (*fini)(void),
        void (*rtld_fini)(void), void *stack_end)
{
	typeof(__libc_start_main) *real_lsm;
	if ((*(void**)&real_lsm = dlsym(RTLD_NEXT, STR(__libc_start_main))) != 0)
		return real_lsm(main, ac, av, init, fini, rtld_fini, stack_end);
	fputs("BUG: dlsym error\n", stderr);
	return 1;
}
#undef STR
#undef STR_
#endif
#endif

/*
 * this module does not get any parameters, so we don't need our wrappers */
/* #include "hypmain.h" */

int main(void)
{
	int retval = EXIT_SUCCESS;
	hcp_opts _opts;
	hcp_opts *opts = &_opts;
	struct hypfind_opts _search;
	struct hypfind_opts *search = &_search;
	FILE *out = stdout;
	GString *body;
	hyp_pic_format pic_format = HYP_PIC_UNKNOWN;
	CURL *curl = NULL;
	char *lang;
	hyp_nodenr node = HYP_NOINDEX;
	long lineno = 0;
	char *val;
	gboolean isrsc = FALSE;

#ifdef __WIN32__
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

	_crtinit();

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, xs_get_locale_dir());
	textdomain(GETTEXT_PACKAGE);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

	HypProfile_Load(FALSE);
	
	hcp_opts_init(opts);
	opts->tabwidth = gl_profile.viewer.ascii_tab_size;
	if (!hcp_opts_parse_string(opts, gl_profile.hcp.options, OPTS_FROM_CONFIG))
		{}
	opts->all_links = !opts->autoreferences;

	memset(search, 0, sizeof(*search));
	search->casesensitive = FALSE;
	search->wordonly = FALSE;
	bm_init(&search->deltapat, NULL, TRUE);
	
	g_freep(&opts->error_filename);
	opts->errorfile = fopen("hypview.log", "a");
	if (opts->errorfile == NULL)
		opts->errorfile = stderr;
	else
		dup2(fileno(opts->errorfile), 2);
	g_freep(&opts->output_filename);
	opts->outfile = out;
	opts->pic_format = HTML_DEFAULT_PIC_TYPE;
	opts->for_cgi = TRUE;
	opts->hidemenu = FALSE;
	opts->hideimages = FALSE;
	opts->cgi_cached = FALSE;
	opts->showstg = FALSE;
	opts->recompile_format = HYP_FT_HTML;
	
	body = g_string_new(NULL);
	cgiInit(body);

	html_init(opts);

	if (cgiAccept && strstr(cgiAccept, "application/xhtml+xml") != NULL)
		opts->recompile_format = HYP_FT_HTML_XML;

	if (cgiScriptName)
		cgi_scriptname = cgiScriptName;
	
	lang = cgiFormString("lang");
	
	if ((val = cgiFormString("charset")) != NULL)
	{
		opts->output_charset = hyp_charset_from_name(val);
		g_free(val);
	}
	if (opts->output_charset == HYP_CHARSET_NONE)
		opts->output_charset = HYP_CHARSET_UTF8;
	if ((val = cgiFormString("hidemenu")) != NULL)
	{
		opts->hidemenu = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("showstg")) != NULL)
	{
		opts->showstg = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("hideimages")) != NULL)
	{
		opts->hideimages = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("index")) != NULL)
	{
		node = (int)strtoul(val, NULL, 10);
		g_free(val);
	}
	if ((val = cgiFormString("lineno")) != NULL)
	{
		lineno = (int)strtoul(val, NULL, 10);
		g_free(val);
		(void)lineno;
	}
	if ((val = cgiFormString("isrsc")) != NULL)
	{
		isrsc = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("treeview")) != NULL)
	{
		opts->treeview = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("cached")) != NULL)
	{
		opts->cgi_cached = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("q")) != NULL)
	{
		search->pattern = val;
		search->pattern_len = strlen(search->pattern);
		if (!bm_init(&search->deltapat, search->pattern, search->casesensitive) ||
			search->pattern_len == 0)
		{
			html_out_header(NULL, opts, body, _("500 Internal Server Error"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
			g_string_append(body, _("empty search pattern\n"));
			html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE, NULL);
			retval = EXIT_FAILURE;
		}
	}
	opts->read_images = !opts->hideimages || opts->showstg;
	
	if (retval == EXIT_SUCCESS &&
		g_ascii_strcasecmp(cgiRequestMethod, "GET") == 0)
	{
		char *url = cgiFormString("url");
		char *filename = g_strdup(url);
		char *scheme = empty(filename) ? g_strdup("undefined") : uri_has_scheme(filename) ? g_strndup(filename, strchr(filename, ':') - filename) : g_strdup("file");
		
		if (filename && filename[0] == '/')
		{
			html_referer_url = filename;
			filename = g_strconcat(cgiDocumentRoot, filename, NULL);
		} else if (empty(hyp_basename(filename)) || (!opts->cgi_cached && g_ascii_strcasecmp(scheme, "file") == 0))
		{
			/*
			 * disallow file URIs, they would resolve to local files on the WEB server
			 */
			hyp_utf8_fprintf(opts->errorfile, "%s: GET from %s, query=%s\n", currdate(), fixnull(cgiRemoteHost), fixnull(cgiQueryString));
			html_out_header(NULL, opts, body, _("403 Forbidden"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
			g_string_append_printf(body,
				_("Sorry, this type of\n"
				  "<a href=\"http://www.w3.org/Addressing/\">URL</a>\n"
				  "<a href=\"http://www.iana.org/assignments/uri-schemes.html\">scheme</a>\n"
				  "(<q>%s</q>) is not\n"
				  "supported by this service. Please check that you entered the URL correctly.\n"),
				scheme);
			html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE, NULL);
			g_free(filename);
			filename = NULL;
		} else
		{
			if (!opts->cgi_cached &&
				(curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK ||
				(curl = curl_easy_init()) == NULL))
			{
				html_out_header(NULL, opts, body, _("500 Internal Server Error"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
				g_string_append(body, _("could not initialize curl\n"));
				html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE, NULL);
				retval = EXIT_FAILURE;
			} else
			{
				char *local_filename;
				
				mk_output_dir(opts);
				if (opts->cgi_cached)
				{
					html_referer_url = g_strdup(hyp_basename(filename));
					local_filename = g_build_filename(opts->output_dir, html_referer_url, NULL);
					g_free(filename);
					filename = local_filename;
				} else
				{
					html_referer_url = g_strdup(filename);
					local_filename = curl_download(curl, opts, body, filename);
					g_free(filename);
					filename = local_filename;
					if (filename)
						opts->cgi_cached = TRUE;
				}
			}
		}
		if (filename && retval == EXIT_SUCCESS)
		{
			if (isrsc)
			{
				if (node == HYP_NOINDEX)
					node = 0;
				if (show_resource(filename, opts, body, node, &pic_format) == FALSE)
				{
					retval = EXIT_FAILURE;
				}
			} else if (opts->treeview)
			{
				if (html_print_treeview(filename, opts, body) == FALSE)
				{
					retval = EXIT_FAILURE;
				}
			} else
			{
				if (recompile(filename, opts, search, body, node, &pic_format) == FALSE)
				{
					retval = EXIT_FAILURE;
				}
			}
			g_free(filename);
		}
		g_free(scheme);
		g_freep(&html_referer_url);

		g_free(url);
	} else if (retval == EXIT_SUCCESS &&
		(g_ascii_strcasecmp(cgiRequestMethod, "POST") == 0 ||
		 g_ascii_strcasecmp(cgiRequestMethod, "BOTH") == 0))
	{
		char *filename;
		int len;
		
		g_string_truncate(body, 0);
		filename = cgiFormFileName("file", &len);
		if (filename == NULL || len == 0)
		{
			const char *scheme = "undefined";
			html_out_header(NULL, opts, body, _("403 Forbidden"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
			g_string_append_printf(body,
				_("Sorry, this type of\n"
				  "<a href=\"http://www.w3.org/Addressing/\">URL</a>\n"
				  "<a href=\"http://www.iana.org/assignments/uri-schemes.html\">scheme</a>\n"
				  "(<q>%s</q>) is not\n"
				  "supported by this service. Please check that you entered the URL correctly.\n"),
				scheme);
			html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE, NULL);
		} else
		{
			FILE *fp;
			char *local_filename;
			const char *data;
			
			mk_output_dir(opts);
			if (*filename == '\0')
			{
				g_free(filename);
#if defined(HAVE_MKSTEMPS)
				{
				int fd;
				filename = g_strdup("tmpfile.XXXXXX.hyp");
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
				fd = mkstemps(local_filename, 4);
				if (fd > 0)
					close(fd);
				}
#elif defined(HAVE_MKSTEMP)
				{
				int fd;
				filename = g_strdup("tmpfile.hyp.XXXXXX");
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
				fd = mkstemp(local_filename);
				if (fd > 0)
					close(fd);
				}
#else
				filename = g_strdup("tmpfile.hyp.XXXXXX");
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
				mktemp(local_filename);
#endif
			} else
			{
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
			}

			hyp_utf8_fprintf(opts->errorfile, "%s: POST from %s, file=%s, size=%d\n", currdate(), fixnull(cgiRemoteHost), hyp_basename(filename), len);

			fp = hyp_utf8_fopen(local_filename, "wb");
			if (fp == NULL)
			{
				const char *err = hyp_utf8_strerror(errno);
				hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", local_filename, err);
				html_out_header(NULL, opts, body, _("404 Not Found"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
				g_string_append_printf(body, "%s: %s\n", hyp_basename(filename), err);
				html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE, NULL);
				retval = EXIT_FAILURE;
			} else
			{
				data = cgiFormFileData("file", &len);
				fwrite(data, 1, len, fp);
				fclose(fp);
				opts->cgi_cached = TRUE;
				html_referer_url = g_strdup(filename);
				if (isrsc)
				{
					if (node == HYP_NOINDEX)
						node = 0;
					if (show_resource(local_filename, opts, body, node, &pic_format) == FALSE)
					{
						retval = EXIT_FAILURE;
					}
				} else
				{
					if (recompile(local_filename, opts, search, body, node, &pic_format) == FALSE)
					{
						retval = EXIT_FAILURE;
					}
				}
			}
			g_free(local_filename);
		}
		g_free(filename);
		g_freep(&html_referer_url);
	}
	
#if 0
	{
		FILE *tmp = fopen("test.out", "wb");
		if (tmp)
		{
			html_out_response_header(tmp, opts->output_charset, body->len, pic_format, opts->recompile_format == HYP_FT_HTML_XML);
			write_strout(body, tmp);
			fclose(tmp);
		}
	}
#endif
	html_out_response_header(out, opts->output_charset, body->len, pic_format, opts->recompile_format == HYP_FT_HTML_XML);
	cgiExit();
	write_strout(body, out);
	g_string_free(body, TRUE);
	
	g_free(lang);
	hcp_opts_free(opts);
	
	if (curl)
	{
		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}
	
	bm_exit(&search->deltapat);
	g_free(search->pattern);
	HypProfile_Delete();
	x_free_resources();

#ifdef ENABLE_NLS
	xs_locale_exit();
#endif

	_crtexit();

	return retval;
}


/*
HTTP_HOST=127.0.0.2
HTTP_USER_AGENT=Mozilla/5.0 (X11; Linux x86_64; rv:45.0) Gecko/20100101 Firefox/45.0
HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9;q=0.8
HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.8,de-DE;q=0.5,de;q=0.3
HTTP_ACCEPT_ENCODING=gzip, deflate
HTTP_DNT=1
HTTP_REFERER=http://127.0.0.2/hypview/
HTTP_CONNECTION=keep-alive
PATH=/usr/bin:/bin:/usr/sbin:/sbin
SERVER_SIGNATURE=
Apache/2.2.17 (Linux/SUSE) Server at 127.0.0.2 Port 80

SERVER_SOFTWARE=Apache/2.2.17 (Linux/SUSE)
SERVER_NAME=127.0.0.2
SERVER_ADDR=127.0.0.2
SERVER_PORT=80
REMOTE_ADDR=127.0.0.1
DOCUMENT_ROOT=/srv/www/htdocs
SERVER_ADMIN=[no address given]
SCRIPT_FILENAME=/srv/www/htdocs/hypview/hypview.cgi
REMOTE_PORT=48150
GATEWAY_INTERFACE=CGI/1.1
SERVER_PROTOCOL=HTTP/1.1
REQUEST_METHOD=GET
QUERY_STRING=url=&hideimages=0&hidemenu=0&dstenc=
REQUEST_URI=/hypview/hypview.cgi?url=&hideimages=0&hidemenu=0&dstenc=
SCRIPT_NAME=/hypview/hypview.cgi
*/
