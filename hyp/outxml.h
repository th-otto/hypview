#ifdef HAVE_PNG
#define XML_DEFAULT_PIC_TYPE HYP_PIC_PNG
#else
#define XML_DEFAULT_PIC_TYPE HYP_PIC_GIF
#endif

#if 0
static char const xml_translatable[] = " translatable=\"1\"";
#else
static char const xml_translatable[] = "";
#endif
static char const xml_space_preserve[] = " xml:space=\"preserve\"";

struct xml_xref {
	char *destname;
	char *text;	/* text to display */
	hyp_nodenr dest_page;
	hyp_indextype desttype;
	hyp_lineno line;
	const char *type;
};

#define xml_quote_name html_quote_name

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *xml_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node, unsigned int flags)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *p;
	char *buf;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	buf = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
	p = xml_quote_name(buf, flags);
	g_free(buf);
	return p;
}

/* ------------------------------------------------------------------------- */

static void xml_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out)
{
	char *str;

	hyp_utf8_sprintf_charset(out, opts->output_charset, "  <header>\n");
	hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"os\" value=\"%s\" />\n", hyp_osname(hyp->comp_os));
	hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"charset\" value=\"%s\" />\n", hyp_charset_name(hyp->comp_charset));
	
	if (hyp->database != NULL)
	{
		str = xml_quote_name(hyp->database, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"database\" value=\"%s\" />\n", str);
		g_free(str);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			str = xml_quote_name(h->name, 0);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"hostname\" value=\"%s\" />\n", str);
			g_free(str);
		}
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = xml_quote_nodename(hyp, hyp->default_page, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"default\" value=\"%s\" />\n", str);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		str = xml_quote_name(hyp->hcp_options, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"options\" value=\"%s\" />\n", str);
		g_free(str);
	}
	if (hyp->author != NULL)
	{
		str = xml_quote_name(hyp->author, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"author\" value=\"%s\" />\n", str);
		g_free(str);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = xml_quote_nodename(hyp, hyp->help_page, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"help\" value=\"%s\" />\n", str);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		str = xml_quote_name(hyp->version, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"version\" value=\"%s\" />\n", str);
		g_free(str);
	}
	if (hyp->subject != NULL)
	{
		str = xml_quote_name(hyp->subject, 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"subject\" value=\"%s\" />\n", str);
		g_free(str);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"width\" value=\"%d\" />\n", hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <param name=\"flags\" value=\"%d\" />\n", hyp->st_guide_flags);
	}

	hyp_utf8_sprintf_charset(out, opts->output_charset, "  </header>\n");
}

/* ------------------------------------------------------------------------- */

static void xml_out_str(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(hyp->comp_charset, opts->output_charset, str, len, &converror);
	p = xml_quote_name(dst, QUOTE_SPACE);
	g_string_append_printf(out, "    <_text%s%s>%s</_text>\n", xml_space_preserve, xml_translatable, p);
	g_free(p);
	g_free(dst);
}

/* ------------------------------------------------------------------------- */

/*
 * FIXME: this should be converted to handle sequences like
 * @{B}bold text @{I}bold and italic{b} italic@{i}
 * <b>bold text <em>bold and italic</em></b><em> italic</em>
 */
static void xml_out_attr(GString *out, unsigned char oldattr, unsigned char newattr)
{
	if (oldattr != newattr)
	{
		g_string_append_printf(out, "    <textattr>%u</textattr>\n", newattr);
	}
}

/* ------------------------------------------------------------------------- */

static void xml_out_labels(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_LABELNAME);
	while (sym)
	{
		if (sym->lineno == lineno)
		{
			char *str = xml_quote_name(sym->name, QUOTE_SPACE);
			if (sym->from_idx && !sym->from_ref)
				hyp_utf8_sprintf_charset(out, opts->output_charset, "    <_index%s>%s</_index><!-- lineno %u -->\n", xml_translatable, str, sym->lineno);
			else
				hyp_utf8_sprintf_charset(out, opts->output_charset, "    <_label%s>%s</_label><!-- lineno %u -->\n", xml_translatable, str, sym->lineno);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void xml_out_alias(GString *out, HYP_DOCUMENT *hyp, hcp_opts *opts, const INDEX_ENTRY *entry, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = xml_quote_name(sym->name, QUOTE_SPACE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <_alias%s>%s</_alias>\n", xml_translatable, str);
		g_free(str);
		sym->referenced = TRUE;
		sym = sym_find(sym->next, nodename, REF_ALIASNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void xml_out_gfx(hcp_opts *opts, GString *out, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx)
{
	const char *type;
	
	switch (gfx->type)
	{
	case HYP_ESC_PIC:
		{
			char *fname;
			char *quoted;
			char *dithermask;
			const char *format;
			
			dithermask = format_dithermask(gfx->dithermask);
			if (!hypnode_valid(hyp, gfx->extern_node_index))
			{
				fname = hyp_invalid_page(gfx->extern_node_index);
				format = "application/octet-stream";
			} else if (hyp->indextable[gfx->extern_node_index]->type != HYP_NODE_IMAGE)
			{
				fname = g_strdup_printf(_("<non-image node #%u>"), gfx->extern_node_index);
				format = "application/octet-stream";
			} else
			{
				fname = image_name(gfx->format, hyp, gfx->extern_node_index, opts->image_name_prefix);
				format = hcp_pic_format_to_mimetype(gfx->format);
			}
			quoted = xml_quote_name(fname, 0);
			type = gfx->islimage ? "limage" : "image";
			hyp_utf8_sprintf_charset(out, opts->output_charset, "    <%s name=\"%s\" type=\"%s\">\n", type, quoted, format);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "      <width>%d</width>\n", gfx->pixwidth);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "      <height>%d</height>\n", gfx->pixheight);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "      <centered>%d</centered>\n", gfx->x_offset == 0);
			if (gfx->x_offset != 0)
				hyp_utf8_sprintf_charset(out, opts->output_charset, "      <xoffset>%d</xoffset>\n", (gfx->x_offset - 1));
			if (!empty(dithermask))
				hyp_utf8_sprintf_charset(out, opts->output_charset, "      <dithermask>%s</dithermask>\n", dithermask);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "    </%s>\n", type);
			g_free(quoted);
			g_free(fname);
			g_free(dithermask);
		}
		break;
	case HYP_ESC_LINE:
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <line>\n");
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <xoffset>%d</xoffset>\n", (gfx->x_offset - 1));
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <width>%d</width>\n", gfx->width);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <height>%d</height>\n", gfx->height);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <begarrow>%d</begarrow>\n", (gfx->begend & (1 << 0)) != 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <endarrow>%d</endarrow>\n", (gfx->begend & (1 << 1)) != 0);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <style>%d</style>\n", gfx->style);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    </line>\n");
		break;
	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		type = gfx->type == HYP_ESC_BOX ? "box" : "rbox";
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <%s>\n", type);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <xoffset>%d</xoffset>\n", (gfx->x_offset - 1));
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <width>%d</width>\n", gfx->width);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <height>%d</height>\n", gfx->height);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <style>%d</style>\n", gfx->style);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "    </%s>\n", type);
		break;
	}
}

/* ------------------------------------------------------------------------- */

static void xml_out_graphics(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, struct hyp_gfx *gfx, long lineno)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			xml_out_gfx(opts, out, hyp, gfx);
		}
		gfx = gfx->next;
	}
}

/* ------------------------------------------------------------------------- */

static void xml_generate_link(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, struct xml_xref *xref, symtab_entry *syms, gboolean newwindow)
{
	const char *linktype;
	char *targetfile = NULL;
	char *quoted;
	
	switch (xref->desttype)
	{
	case HYP_NODE_INTERNAL:
	case HYP_NODE_POPUP:
	case HYP_NODE_EXTERNAL_REF:
		{
		gboolean is_xref = FALSE;
		if (xref->desttype == HYP_NODE_EXTERNAL_REF)
		{
			char *p = ((hyp->st_guide_flags & STG_ALLOW_FOLDERS_IN_XREFS) ? strrslash : strslash)(xref->destname);
			char c = '\0';
			hyp_filetype ft;
			linktype = "external";
			if (p != NULL)
			{
				c = *p;
				*p = '\0';
			}
			ft = hyp_guess_filetype(xref->destname);
			is_xref = ft != HYP_FT_NONE;
			if (ft == HYP_FT_RSC || ft == HYP_FT_HYP)
			{
				/*
				 * basename here is as specified in the link,
				 * which is often all uppercase.
				 * Always convert to lowercase first.
				 */
				targetfile = hyp_utf8_strdown(hyp_basename(xref->destname), STR0TERM);
				if (ft == HYP_FT_RSC && p)
					xref->line = strtoul(p + 1, NULL, 0);
				if (p)
				{
					memmove(xref->destname, p + 1, strlen(p + 1) + 1);
				} else
				{
					g_free(xref->destname);
					xref->destname = NULL;
				}
				p = NULL;
			}
			if (p)
				*p = c;
		} else if (xref->desttype == HYP_NODE_POPUP)
		{
			linktype = "popup";
		} else
		{
			linktype = "internal";
		}
		
		if (!is_xref)
		{
			symtab_entry *sym;
			
			if (xref->line != 0)
			{
				sym = sym_find(syms, xref->destname, REF_LABELNAME);
				while (sym)
				{
					if (sym->lineno == xref->line /* && !sym->from_idx */)
					{
						sym->referenced = TRUE;
						break;
					}
					sym = sym_find(sym->next, xref->destname, REF_LABELNAME);
				}
			}
		}
		}
		break;
	case HYP_NODE_REXX_COMMAND:
		linktype = "rx";
		targetfile = xref->destname;
		xref->destname = NULL;
		break;
	case HYP_NODE_REXX_SCRIPT:
		linktype = "rxs";
		targetfile = xref->destname;
		xref->destname = NULL;
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		linktype = "system";
		targetfile = xref->destname;
		xref->destname = NULL;
		break;
	case HYP_NODE_IMAGE:
		/* that would be an inline image; currently not supported by compiler */
		linktype = "image";
		targetfile = xref->destname;
		xref->destname = NULL;
		break;
	case HYP_NODE_QUIT:
		linktype = "quit";
		targetfile = xref->destname;
		xref->destname = NULL;
		break;
	case HYP_NODE_CLOSE:
		linktype = "close";
		targetfile = xref->destname;
		xref->destname = NULL;
		break;
	case HYP_NODE_EOF:
	default:
		linktype = "invalid";
		break;
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset, "    <%s type=\"%s\">\n", xref->type, linktype);
	hyp_utf8_sprintf_charset(out, opts->output_charset, "      <targetnode>%u</targetnode>\n", xref->dest_page);
	if (targetfile)
	{
		quoted = xml_quote_name(targetfile, QUOTE_SPACE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <targetfile%s>%s</targetfile>\n", xml_space_preserve, quoted);
		g_free(quoted);
	}
	if (xref->destname)
	{
		quoted = xml_quote_name(xref->destname, QUOTE_SPACE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "      <_targetname%s%s>%s</_targetname>\n", xml_space_preserve, xml_translatable, quoted);
		g_free(quoted);
	}
	hyp_utf8_sprintf_charset(out, opts->output_charset, "      <targetline>%u</targetline>\n", xref->line);
	hyp_utf8_sprintf_charset(out, opts->output_charset, "      <_text%s%s>%s</_text>\n", xml_space_preserve, xml_translatable, xref->text);
	hyp_utf8_sprintf_charset(out, opts->output_charset, "      <newwindow>%d</newwindow>\n", newwindow);
	hyp_utf8_sprintf_charset(out, opts->output_charset, "    </%s>\n", xref->type);
	g_free(targetfile);
}

/* ------------------------------------------------------------------------- */

static gboolean xml_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node, symtab_entry *syms)
{
	char *str;
	int in_tree;
	unsigned char textattr;
	long lineno;
	struct hyp_gfx *hyp_gfx = NULL;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	INDEX_ENTRY *entry;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		xml_out_str(hyp, opts, out, textstart, src - textstart); \
	}
#define FLUSHTREE() \
	if (in_tree != -1) \
	{ \
		hyp_utf8_sprintf_charset(out, opts->output_charset, "end tree %d -->\n", in_tree); \
		in_tree = -1; \
	}
	
	entry = hyp->indextable[node];
	hyp_utf8_sprintf_charset(out, opts->output_charset, "  <node index=\"%u\" type=\"%s\">\n",
		node,
		entry->type == HYP_NODE_INTERNAL ? "internal" : "popup");

	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		unsigned short dithermask;
		
		{
		char *title;
		char *nodename;
		
		hyp_node_find_windowtitle(nodeptr);
		
		nodename = xml_quote_nodename(hyp, node, QUOTE_SPACE);
		if (nodeptr->window_title)
		{
			char *buf = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
			title = xml_quote_name(buf, QUOTE_SPACE);
			g_free(buf);
		} else
		{
			title = NULL;
		}

		hyp_utf8_sprintf_charset(out, opts->output_charset, "    <_name%s%s>%s</_name>\n", xml_space_preserve, xml_translatable, nodename);
		if (title)
			hyp_utf8_sprintf_charset(out, opts->output_charset, "    <_title%s%s>%s</_title>\n", xml_space_preserve, xml_translatable, title);
			
		g_free(title);
		g_free(nodename);
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
							adm->format = format_from_pic(opts, hyp->indextable[adm->extern_node_index], XML_DEFAULT_PIC_TYPE);
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
					struct xml_xref xref;
					char *buf;
					
					xref.dest_page = DEC_255(&src[3]);
					buf = hyp_conv_to_utf8(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
					buf = chomp(buf);
					xref.text = xml_quote_name(buf, QUOTE_SPACE);
					g_free(buf);
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
					if (empty(xref.text))
					{
						g_free(xref.text);
						xref.text = g_strdup(xref.destname);
					}
					xref.line = 0;
					xref.type = "xref";
					xml_generate_link(hyp, opts, out, &xref, syms, FALSE);
					g_free(xref.destname);
					g_free(xref.text);
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
		 * check for alias names in ref file
		 */
		xml_out_alias(out, hyp, opts, entry, syms);
		
		/*
		 * now output data
		 */
		src = nodeptr->start;
		textstart = src;
		in_tree = -1;
		textattr = 0;
		lineno = 0;
		xml_out_labels(hyp, opts, out, entry, lineno, syms);
		xml_out_graphics(hyp, opts, out, hyp_gfx, lineno);
		
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
					g_string_append(out, "    <text>&#x1b;</text>\n");
					src++;
					break;
				
				case HYP_ESC_WINDOWTITLE:
					src++;
					FLUSHTREE();
					/* @title already output */
					src += ustrlen(src) + 1;
					break;

				case HYP_ESC_CASE_DATA:
					FLUSHTREE();
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
						struct xml_xref xref;
						
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
								xref.text = xml_quote_nodename(hyp, xref.dest_page, QUOTE_SPACE);
							} else
							{
								xref.text = g_strdup(xref.destname);
							}
						} else
						{
							char *buf;
							size_t len;
							
							len = *src - HYP_STRLEN_OFFSET;
							src++;
							buf = hyp_conv_to_utf8(hyp->comp_charset, src, len);
							xref.text = xml_quote_name(buf, QUOTE_SPACE);
							g_free(buf);
							src += len;
						}
						FLUSHTREE();
						
						xref.type = "link";
						xml_generate_link(hyp, opts, out, &xref, syms, type == HYP_ESC_ALINK || type == HYP_ESC_ALINK_LINE);
						g_free(xref.destname);
						g_free(xref.text);
					}
					break;
					
				case HYP_ESC_EXTERNAL_REFS:
					FLUSHTREE();
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
							str = xml_quote_nodename(hyp, dest_page, 0);
						} else
						{
							str = hyp_invalid_page(dest_page);
						}
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
					src += 8;
					break;
					
				case HYP_ESC_LINE:
					FLUSHTREE();
					src += 7;
					break;
					
				case HYP_ESC_BOX:
				case HYP_ESC_RBOX:
					FLUSHTREE();
					src += 7;
					break;
					
				case HYP_ESC_CASE_TEXTATTR:
					xml_out_attr(out, textattr, *src - HYP_ESC_TEXTATTR_FIRST);
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
				g_string_append(out, "    <newline />\n");
				++lineno;
				xml_out_labels(hyp, opts, out, entry, lineno, syms);
				xml_out_graphics(hyp, opts, out, hyp_gfx, lineno);
				src++;
				textstart = src;
			} else
			{
				FLUSHTREE();
				src++;
			}
		}
		DUMPTEXT();
		xml_out_attr(out, textattr, 0);
		FLUSHTREE();
		++lineno;
		xml_out_labels(hyp, opts, out, entry, lineno, syms);
		xml_out_graphics(hyp, opts, out, hyp_gfx, lineno);
		
		if (hyp_gfx != NULL)
		{
			struct hyp_gfx *gfx, *next;
			
			for (gfx = hyp_gfx; gfx != NULL; gfx = next)
			{
				if (!gfx->used)
				{
					hyp_utf8_sprintf_charset(out, opts->output_charset, "<!-- gfx unused: ");
					xml_out_gfx(opts, out, hyp, gfx);
					hyp_utf8_sprintf_charset(out, opts->output_charset, "-->\n");
				}
				next = gfx->next;
				g_free(gfx);
			}
		}
					
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(opts->errorfile, _("<!-- %s: Node %u: failed to decode -->\n"), hyp->file, node);
	}

	hyp_utf8_sprintf_charset(out, opts->output_charset, "  </node>\n");

#undef DUMPTEXT
#undef FLUSHLINE
#undef FLUSHTREE
	return retval;
}

/* ------------------------------------------------------------------------- */


static gboolean xml_out_image(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr node)
{
	GString *image;
	Base64 *b;
	hyp_pic_format format;
	const char *formatname;
	gboolean ret;
	char *quoted;
	char *fname;
	
	image = g_string_new(NULL);
	ret = write_image(hyp, opts, node, XML_DEFAULT_PIC_TYPE, image);
	if (ret)
	{
		b = Base64_New();
		if (Base64_Encode(b, image->str, image->len))
		{
			format = format_from_pic(opts, hyp->indextable[node], XML_DEFAULT_PIC_TYPE);
			fname = image_name(format, hyp, node, opts->image_name_prefix);
			formatname = hcp_pic_format_to_mimetype(format);
			quoted = xml_quote_name(fname, 0);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "  <node index=\"%u\" type=\"%s\">\n",
				node,
				"image");
			hyp_utf8_sprintf_charset(out, opts->output_charset, "    <_name%s%s>%s</_name>\n", xml_space_preserve, xml_translatable, quoted);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "    <data href=\"data:%s;base64,", formatname);
			g_string_append_len(out, Base64_EncodedMessage(b), Base64_EncodedMessageSize(b));
			hyp_utf8_sprintf_charset(out, opts->output_charset, "\" />\n");
			hyp_utf8_sprintf_charset(out, opts->output_charset, "  </node>\n");
			g_free(quoted);
			g_free(fname);
		}
		Base64_Delete(b);
	}
	g_string_free(image, TRUE);
	return ret;
}

/* ------------------------------------------------------------------------- */

static gboolean recompile_xml(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms = NULL;
	GString *out;
	char *str;
	const char *nodetype;
	char *nodename;
	
	UNUSED(argc);
	UNUSED(argv);
	
	ret = TRUE;
	
	if (opts->read_images && hyp->cache == NULL)
		InitCache(hyp);
	
	out = g_string_new(NULL);

	g_string_append(out, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
	g_string_append(out, "<!DOCTYPE hypfile SYSTEM \"http://www.tho-otto.de/dtd/hypfile.dtd\"");
	html_out_entities(out);
	g_string_append(out, ">\n");
	g_string_append_printf(out, "<!-- This file was automatically generated by %s version %s -->\n", gl_program_name, gl_program_version);
	str = xml_quote_name(hyp_basename(hyp->file), 0);
	g_string_append_printf(out, "<hypfile name=\"%s\" generator=\"%s\" version=\"%s\">\n", str, gl_program_name, gl_program_version);
	g_free(str);
	xml_out_globals(hyp, opts, out);
	
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
	
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		nodetype = NULL;
		nodename = NULL;
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
			ret &= xml_out_node(hyp, opts, out, node, syms);
			break;
		case HYP_NODE_IMAGE:
			if (opts->read_images)
				ret &= xml_out_image(hyp, opts, out, node);
			break;
		case HYP_NODE_EXTERNAL_REF:
			nodetype = "external";
			nodename = xml_quote_nodename(hyp, node, QUOTE_SPACE);
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
			nodetype = "system";
			nodename = xml_quote_nodename(hyp, node, QUOTE_SPACE);
			break;
		case HYP_NODE_REXX_SCRIPT:
			nodetype = "rxs";
			nodename = xml_quote_nodename(hyp, node, QUOTE_SPACE);
			break;
		case HYP_NODE_REXX_COMMAND:
			nodetype = "rx";
			nodename = xml_quote_nodename(hyp, node, QUOTE_SPACE);
			break;
		case HYP_NODE_QUIT:
			nodetype = "quit";
			nodename = xml_quote_nodename(hyp, node, QUOTE_SPACE);
			break;
		case HYP_NODE_CLOSE:
			nodetype = "close";
			nodename = xml_quote_nodename(hyp, node, QUOTE_SPACE);
			break;
		case HYP_NODE_EOF:
			break;
		default:
			if (opts->print_unknown)
				hyp_utf8_fprintf(opts->errorfile, _("unknown index entry type %u\n"), entry->type);
			break;
		}
		if (nodetype && nodename)
		{
			hyp_utf8_sprintf_charset(out, opts->output_charset, "  <node index=\"%u\" type=\"%s\">\n",
				node,
				nodetype);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "    <_name%s%s>%s</_name>\n", xml_space_preserve, xml_translatable, nodename);
			g_free(nodename);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "  </node>\n");
		}
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
	}
	
	g_string_append(out, "</hypfile>\n");
	write_strout(out, opts->outfile);

	g_string_free(out, TRUE);
	
	free_symtab(syms);
	return ret;
}
