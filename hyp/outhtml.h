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

#ifndef __HYPOUTHTML_H__
#define __HYPOUTHTML_H__ 1

#define HTML_DOCTYPE_OLD              0           /* HTML 3.2 */
#define HTML_DOCTYPE_STRICT           1           /* HTML 4.01 */
#define HTML_DOCTYPE_TRANS            2           /* HTML 4.01 Transitional */
#define HTML_DOCTYPE_FRAME            3           /* HTML 4.01 Frameset */
#define HTML_DOCTYPE_XSTRICT          4           /* XHTML 1.0 Strict */
#define HTML_DOCTYPE_XTRANS           5           /* XHTML 1.0 Transitional */
#define HTML_DOCTYPE_XFRAME           6           /* XHTML 1.0 Frameset */
#define HTML_DOCTYPE_HTML5            7           /* HTML 5 */

#define HTML_DEFAULT_PIC_TYPE HYP_PIC_GIF
 
extern char *html_referer_url;
extern int html_doctype;
extern const char *cgi_scriptname;
extern gboolean html_css_written;
extern gboolean html_js_written;
extern char const hypview_lineno_tag[];
extern char const html_attr_bold_style[];

#define QUOTE_CONVSLASH  0x0001
#define QUOTE_SPACE      0x0002
#define QUOTE_URI        0x0004
#define QUOTE_JS         0x0008
#define QUOTE_ALLOWUTF8  0x0010
#define QUOTE_LABEL      0x0020
#define QUOTE_UNICODE    0x0040
#define QUOTE_NOLTR      0x0080


struct html_xref {
	char *destname;
	char *destfilename;
	char *text;	/* text to display */
	hyp_nodenr dest_page;
	hyp_indextype desttype;
	hyp_lineno line;
	struct html_xref *next;
};

char *html_quote_name(const char *name, unsigned int flags);
char *html_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node, unsigned int flags);
char *html_filename_for_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, gboolean quote);
void html_out_entities(GString *out);
void html_out_header(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const char *title, hyp_nodenr node, struct hyp_gfx *hyp_gfx, struct html_xref *xrefs, symtab_entry *syms, gboolean for_error);
void html_out_trailer(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, gboolean for_error, gboolean warn_gfx);
gboolean html_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, symtab_entry *syms, gboolean for_inline);
void html_init(hcp_opts *opts);
void html_out_stg_gfx(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, char *fname);
gboolean recompile_html(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv);

#endif /* __HYPOUTHTML_H__ */
