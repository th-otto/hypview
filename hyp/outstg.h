typedef struct _symtab_entry symtab_entry;
struct _symtab_entry {
	char *nodename;
	hyp_lineno lineno;
	hyp_reftype type;
	char *name;
	gboolean freeme;
	gboolean from_ref;
	gboolean from_idx;
	gboolean referenced;
	symtab_entry *next;
};

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

static void stg_out_globals(HYP_DOCUMENT *hyp, FILE *outfile)
{
	char *str;

	hyp_utf8_fprintf_charset(outfile, output_charset, "@if VERSION >= 6%s", stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@os %s%s", hyp_osname(hyp->comp_os), stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@charset %s%s", hyp_charset_name(hyp->comp_charset), stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@inputenc %s%s", hyp_charset_name(output_charset), stg_nl);
	hyp_utf8_fprintf_charset(outfile, output_charset, "@endif%s", stg_nl);
	
	if (hyp->database != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@database \"%s\"%s", hyp->database, stg_nl);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			hyp_utf8_fprintf_charset(outfile, output_charset, "@hostname \"%s\"%s", h->name, stg_nl);
		}
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = stg_quote_nodename(hyp, hyp->default_page);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@default \"%s\"%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@options \"%s\"%s", hyp->hcp_options, stg_nl);
	}
	if (hyp->author != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@author \"%s\"%s", hyp->author, stg_nl);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = stg_quote_nodename(hyp, hyp->help_page);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@help \"%s\"%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@$VER: %s%s", hyp->version, stg_nl);
	}
	if (hyp->subject != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@subject \"%s\"%s", hyp->subject, stg_nl);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, "@width %d%s", hyp->line_width, stg_nl);
	}
#if 0
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_fprintf_charset(outfile, output_charset, _("ST-Guide flags: $%04x%s"), hyp->st_guide_flags, stg_nl);
	}
#endif
	fputs(stg_nl, outfile);
	fputs(stg_nl, outfile);
}

/* ------------------------------------------------------------------------- */

static void stg_out_str(FILE *outfile, HYP_CHARSET charset, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(charset, output_charset, str, len, &converror);
	p = dst;
	while (*p)
	{
		if (*p == '@')
			fputc('@', outfile); /* AmigaGuide uses \@ for quoted '@' */
		fputc(*p, outfile);
		p++;
	}
	g_free(dst);
}

/* ------------------------------------------------------------------------- */

static gboolean stg_out_attr(FILE *outfile, unsigned char oldattr, unsigned char newattr)
{
	if (oldattr != newattr)
	{
		fputc('@', outfile);
		fputc('{', outfile);
#define onoff(mask, on, off) \
		if ((oldattr ^ newattr) & mask) \
			fputc(newattr & mask ? on : off, outfile)
		onoff(HYP_TXT_BOLD, 'B', 'b');
		onoff(HYP_TXT_LIGHT, 'G', 'g');
		onoff(HYP_TXT_ITALIC, 'I', 'i');
		onoff(HYP_TXT_UNDERLINED, 'U', 'u');
		onoff(HYP_TXT_OUTLINED, 'O', 'o');
		onoff(HYP_TXT_SHADOWED, 'S', 's');
#undef onoff
		fputc('}', outfile);
		return TRUE;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static char *image_name(hyp_pic_format format, HYP_DOCUMENT *hyp, hyp_nodenr node, const char *name_prefix)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	const char *ext;
	char *name;
	char *res;
	
	switch (format)
	{
	case HYP_PIC_IFF:
		ext = ".iff";
		break;
	case HYP_PIC_ICN:
		ext =".icn";
		break;
	case HYP_PIC_IMG:
		ext = ".img";
		break;
	case HYP_PIC_BMP:
		ext = ".bmp";
		break;
	case HYP_PIC_GIF:
		ext = ".gif";
		break;
	case HYP_PIC_PNG:
		ext = ".png";
		break;
	case HYP_PIC_UNKNOWN:
	default:
		ext = ".dta";
		break;
	}
	/*
	 * some newer hypertext files (hcp version >= 5) seem to have the
	 * basename of the original image in the node name. For most files,
	 * the node name for images is empty
	 */
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	if (namelen > 0)
	{
		name = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
		res = replace_ext(name, NULL, ext);
		g_free(name);
		return res;
	}
	return g_strdup_printf("%s%05u%s", name_prefix, node, ext);
}

/* ------------------------------------------------------------------------- */

static hyp_pic_format format_from_pic(hcp_opts *opts, INDEX_ENTRY *entry, hyp_pic_format default_format)
{
	hyp_pic_format format;

	format = opts->pic_format;
	if (format == HYP_PIC_ORIG)
	{
		/*
		 * not documented, but HCP seems to write the
		 * orignal file format into the "up" field
		 */
		format = (hyp_pic_format)entry->toc_index;
	}
	if (default_format != HYP_PIC_IMG && (format == HYP_PIC_IMG || format == HYP_PIC_ICN))
	{
		format = default_format;
		hyp_utf8_fprintf(opts->errorfile, _("warning: GEM images are not displayable in HTML, using %s instead\n"), hcp_pic_format_to_name(default_format));
	}
#ifndef HAVE_PNG
	if (format == HYP_PIC_PNG)
	{
		format = default_format;
		hyp_utf8_fprintf(opts->errorfile, _("warning: PNG not supported on this platform, using %s instead\n"), hcp_pic_format_to_name(default_format));
	}
#endif
	if (format < 1 || format > HYP_PIC_LAST)
	{
		format = default_format;
		hyp_utf8_fprintf(opts->errorfile, _("unknown image source type, using %s instead\n"), hcp_pic_format_to_name(default_format));
	}
	
	return format;
}

/* ------------------------------------------------------------------------- */

static void stg_out_gfx(hcp_opts *opts, FILE *outfile, HYP_DOCUMENT *hyp, struct hyp_gfx *adm)
{
	switch (adm->type)
	{
	case HYP_ESC_PIC:
		{
			char *fname;
			
			if (!hypnode_valid(hyp, adm->extern_node_index))
				fname = hyp_invalid_page(adm->extern_node_index);
			else if (hyp->indextable[adm->extern_node_index]->type != HYP_NODE_IMAGE)
				fname = g_strdup_printf(_("<non-image node #%u>"), adm->extern_node_index);
			else
				fname = image_name(adm->format, hyp, adm->extern_node_index, opts->image_name_prefix);
			hyp_utf8_fprintf_charset(outfile, output_charset, "@remark %ux%ux%u%s", adm->pixwidth, adm->pixheight, 1 << adm->planes, stg_nl);
			hyp_utf8_fprintf_charset(outfile, output_charset, "%s \"%s\" %d%s",
				adm->islimage ? "@limage" : "@image",
				fname,
				adm->x_offset,
				stg_nl);
			g_free(fname);
		}
		break;
	case HYP_ESC_LINE:
		hyp_utf8_fprintf_charset(outfile, output_charset, "@line %d %d %d %d %d%s",
			adm->x_offset, adm->width, adm->height,
			adm->begend,
			adm->style,
			stg_nl);
		break;
	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		hyp_utf8_fprintf_charset(outfile, output_charset, "%s %d %d %d %d%s",
			adm->type == HYP_ESC_BOX ? "@box" : "@rbox",
			adm->x_offset, adm->width, adm->height, adm->style,
			stg_nl);
		break;
	}
}

/* ------------------------------------------------------------------------- */

static void stg_out_graphics(hcp_opts *opts, FILE *outfile, HYP_DOCUMENT *hyp, struct hyp_gfx *gfx, long lineno)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			gfx->used = TRUE;
			stg_out_gfx(opts, outfile, hyp, gfx);
		}
		gfx = gfx->next;
	}
}

/* ------------------------------------------------------------------------- */

static symtab_entry *sym_find(symtab_entry *sym, const char *search, hyp_reftype type)
{
	while (sym != NULL)
	{
		if (type == sym->type && strcmp(sym->nodename, search) == 0)
			return sym;
		sym = sym->next;
	}
	return NULL;
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

static void stg_out_labels(FILE *outfile, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms)
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
			hyp_utf8_fprintf_charset(outfile, output_charset, "@symbol %s \"%s\"%s", sym_typename(sym), str, stg_nl);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void stg_out_alias(FILE *outfile, HYP_DOCUMENT *hyp, const INDEX_ENTRY *entry, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = stg_quote_name(sym->name, FALSE);
		hyp_utf8_fprintf_charset(outfile, output_charset, "@symbol %s \"%s\"%s", sym_typename(sym), str, stg_nl);
		g_free(str);
		sym->referenced = TRUE;
		sym = sym_find(sym->next, nodename, REF_ALIASNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static gboolean stg_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, symtab_entry *syms)
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
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		stg_out_str(outfile, hyp->comp_charset, textstart, src - textstart); \
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
		hyp_utf8_fprintf_charset(outfile, output_charset, "@endtree%s", stg_nl); \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		INDEX_ENTRY *entry;
		
		entry = hyp->indextable[node];
		{
			hyp_node_find_windowtitle(nodeptr);
			
			str = stg_quote_nodename(hyp, node);
			if (nodeptr->window_title)
			{
				char *buf = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
				char *title = stg_quote_name(buf, FALSE);
				hyp_utf8_fprintf_charset(outfile, output_charset, "%s \"%s\" \"%s\"%s", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str, title, stg_nl);
				g_free(title);
				g_free(buf);
			} else
			{
				hyp_utf8_fprintf_charset(outfile, output_charset, "%s \"%s\"%s", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str, stg_nl);
			}
			g_free(str);
			/*
			 * check for alias names in ref file
			 */
			stg_out_alias(outfile, hyp, entry, syms);
			
			if (entry->type == HYP_NODE_INTERNAL)
			{
				if (hypnode_valid(hyp, entry->next) &&
					/* physical next page is default if not set */
					(node + 1u) != entry->next)
				{
					str = stg_quote_nodename(hyp, entry->next);
					hyp_utf8_fprintf_charset(outfile, output_charset, "@next \"%s\"%s", str, stg_nl);
					g_free(str);
				}
				
				if (hypnode_valid(hyp, entry->previous) &&
					 /* physical prev page is default if not set */
					(node - 1u) != entry->previous &&
					(entry->previous != 0 || node != 0))
				{
					str = stg_quote_nodename(hyp, entry->previous);
					hyp_utf8_fprintf_charset(outfile, output_charset, "@prev \"%s\"%s", str, stg_nl);
					g_free(str);
				}
				
				if (hypnode_valid(hyp, entry->toc_index) &&
					/* physical first page is default if not set */
					entry->toc_index != 0)
				{
					str = stg_quote_nodename(hyp, entry->toc_index);
					hyp_utf8_fprintf_charset(outfile, output_charset, "@toc \"%s\"%s", str, stg_nl);
					g_free(str);
				}
			}
		}
				
		if (node == hyp->index_page)
			hyp_utf8_fprintf_charset(outfile, output_charset, "@autorefon%s", stg_nl);

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
							adm->format = format_from_pic(opts, hyp->indextable[adm->extern_node_index], STG_DEFAULT_PIC_TYPE);
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
						hyp_utf8_fprintf_charset(outfile, output_charset, "@xref \"%s\"%s", str, stg_nl);
					else
						hyp_utf8_fprintf_charset(outfile, output_charset, "@xref \"%s\" \"%s\"%s", str, text, stg_nl);
					g_free(str);
					g_free(text);
				}
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
		textattr = 0;
		lineno = 0;
		stg_out_labels(outfile, hyp, entry, lineno, syms);
		stg_out_graphics(opts, outfile, hyp, hyp_gfx, lineno);
		
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
					fputc(0x1b, outfile);
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
								INDEX_ENTRY *entry = hyp->indextable[dest_page];
								len = entry->length - SIZEOF_INDEX_ENTRY;
								textstart = entry->name;
								str_equal = entry->type == HYP_NODE_INTERNAL;
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
								INDEX_ENTRY *entry = hyp->indextable[dest_page];
								str_equal = entry->type == HYP_NODE_INTERNAL && strcmp(str, dest) == 0;
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
								hyp_utf8_fprintf_charset(outfile, output_charset, "@{\"%s\" %s}", str, cmd);
							else if (!str_equal /* || node == hyp->index_page */ || opts->all_links)
								hyp_utf8_fprintf_charset(outfile, output_charset, "@{\"%s\" %s \"%s\"}", str, cmd, dest);
							else
								stg_out_str(outfile, hyp->comp_charset, textstart, len);
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
										hyp_utf8_fprintf_charset(outfile, output_charset, "@{\"%s\" %s \"%s/%u\"}", str, cmd, dest, line);
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
										hyp_utf8_fprintf_charset(outfile, output_charset, "@{\"%s\" %s \"%s\" \"%s\"}", str, cmd, dest, quoted);
										g_free(quoted);
									} else
									{
										hyp_utf8_fprintf_charset(outfile, output_charset, "@{\"%s\" %s \"%s\" %u}", str, cmd, dest, line + 1);
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
							hyp_utf8_fprintf_charset(outfile, output_charset, "@tree %d%s", tree, stg_nl);
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
					if (stg_out_attr(outfile, textattr, *src - HYP_ESC_TEXTATTR_FIRST))
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
				stg_out_labels(outfile, hyp, entry, lineno, syms);
				stg_out_graphics(opts, outfile, hyp, hyp_gfx, lineno);
				src++;
				textstart = src;
			} else
			{
				FLUSHTREE();
				src++;
			}
		}
		DUMPTEXT();
		if (stg_out_attr(outfile, textattr, 0))
			at_bol = FALSE;
		FLUSHLINE();
		FLUSHTREE();
		++lineno;
		stg_out_labels(outfile, hyp, entry, lineno, syms);
		stg_out_graphics(opts, outfile, hyp, hyp_gfx, lineno);
		
		if (hyp_gfx != NULL)
		{
			struct hyp_gfx *gfx, *next;
			
			for (gfx = hyp_gfx; gfx != NULL; gfx = next)
			{
				if (!gfx->used)
				{
					hyp_utf8_fprintf_charset(outfile, output_charset, "##gfx unused: ");
					stg_out_gfx(opts, outfile, hyp, gfx);
				}
				next = gfx->next;
				g_free(gfx);
			}
		}
		
		hyp_utf8_fprintf_charset(outfile, output_charset, "@endnode%s", stg_nl);
		if (node < hyp->last_text_page)
			hyp_utf8_fprintf_charset(outfile, output_charset, "%s%s", stg_nl, stg_nl);
			
		hyp_node_free(nodeptr);
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

static gboolean stg_check_links(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, symtab_entry **syms)
{
	char *str;
	long lineno;
	HYP_NODE *nodeptr;
	gboolean retval = TRUE;
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		
		end = nodeptr->end;
		src = nodeptr->start;
		lineno = 0;
		
		while (retval && src < end)
		{
			if (*src == HYP_ESC)
			{
				src++;
				switch (*src)
				{
				case HYP_ESC_ESC:
				case HYP_ESC_WINDOWTITLE:
				case HYP_ESC_CASE_DATA:
				case HYP_ESC_EXTERNAL_REFS:
				case HYP_ESC_OBJTABLE:
				case HYP_ESC_PIC:
				case HYP_ESC_LINE:
				case HYP_ESC_BOX:
				case HYP_ESC_RBOX:
				case HYP_ESC_CASE_TEXTATTR:
				case HYP_ESC_LINK:
				case HYP_ESC_ALINK:
					src = hyp_skip_esc(src - 1);
					break;
				
				case HYP_ESC_LINK_LINE:
				case HYP_ESC_ALINK_LINE:
					{
						hyp_nodenr dest_page;
						hyp_lineno line;
						char *dest;
						
						line = DEC_255(&src[1]);
						src += 2;
						dest_page = DEC_255(&src[1]);
						src += 3;
						dest = NULL;
						str = NULL;
						if (hypnode_valid(hyp, dest_page))
						{
							INDEX_ENTRY *dest_entry = hyp->indextable[dest_page];
							
							switch ((hyp_indextype) dest_entry->type)
							{
							case HYP_NODE_INTERNAL:
							case HYP_NODE_POPUP:
								dest = hyp_conv_to_utf8(hyp->comp_charset, dest_entry->name, dest_entry->length - SIZEOF_INDEX_ENTRY);
								break;
							case HYP_NODE_EXTERNAL_REF:
							case HYP_NODE_REXX_COMMAND:
							case HYP_NODE_REXX_SCRIPT:
							case HYP_NODE_SYSTEM_ARGUMENT:
							case HYP_NODE_IMAGE:
							case HYP_NODE_QUIT:
							case HYP_NODE_CLOSE:
							default:
							case HYP_NODE_EOF:
								break;
							}
						}
						if (dest)
						{
							if (*src <= HYP_STRLEN_OFFSET)
							{
								src++;
								str = g_strdup(dest);
							} else
							{
								size_t len;
	
								len = *src - HYP_STRLEN_OFFSET;
								src++;
								str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
								src += len;
							}
							{
								gboolean is_xref = FALSE;
								
								char *p = (hyp->st_guide_flags & STG_ALLOW_FOLDERS_IN_XREFS ? strrslash : strslash)(dest);
								if (p != NULL)
								{
									hyp_filetype ft;
									*p = '\0';
									ft = hyp_guess_filetype(dest);
									if (ft == HYP_FT_HYP || ft == HYP_FT_RSC)
									{
										is_xref = TRUE;
									} else
									{
										*p = '/';
									}
								}
								if (!is_xref)
								{
									symtab_entry *sym;
									
									sym = sym_find(*syms, dest, REF_LABELNAME);
									while (sym != NULL)
									{
										if (strcmp(sym->name, str) == 0)
											break;
										if (sym->lineno == line && sym->from_ref)
											break;
										sym = sym_find(sym->next, dest, REF_LABELNAME);
									}
									if (sym == NULL)
									{
										symtab_entry **last = syms;
										symtab_entry *sym;
										
										while (*last)
											last = &(*last)->next;
										sym = g_new(symtab_entry, 1);
										if (sym == NULL)
										{
											retval = FALSE;
										} else
										{
											sym->nodename = dest;
											dest = NULL;
											sym->type = REF_LABELNAME;
											sym->name = str;
											str = NULL;
											sym->lineno = line;
											sym->freeme = TRUE;
											sym->from_ref = FALSE;
											sym->from_idx = node == hyp->index_page;
											sym->referenced = FALSE;
											sym->next = NULL;
											*last = sym;
										}
									}
								}
							}
						}
						g_free(dest);
						g_free(str);
					}
					break;
					
				default:
					break;
				}
			} else if (*src == HYP_EOL)
			{
				++lineno;
				src++;
			} else
			{
				src++;
			}
		}
		++lineno;
		
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(opts->errorfile, _("%s: Node %u: failed to decode\n"), hyp->file, node);
	}

	return retval;
}

/* ------------------------------------------------------------------------- */

static gboolean write_image(HYP_DOCUMENT *hyp, FILE *outfile, hcp_opts *opts, hyp_nodenr node, hyp_pic_format default_format)
{
	unsigned char *data;
	long data_size;
	unsigned char *buf = NULL;
	unsigned char *conv = NULL;
	FILE *fp = NULL;
	gboolean retval = TRUE;
	HYP_PICTURE hyp_pic;
	hyp_pic_format format;
	PICTURE pic;
	long header_size;
	HYP_IMAGE *image;
	char *path;
	
	pic_init(&pic);
	
	image = (HYP_IMAGE *)AskCache(hyp, node);
	if (image == NULL)
	{
		data = hyp_loaddata(hyp, node);
		if (data == NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, _("failed to load image node %u"), node);
			goto error;
		}
		image = g_new0(HYP_IMAGE, 1);
		if (image == NULL)
		{
			g_free(data);
			goto error;
		}
		image->number = node;
		image->pic.fd_addr = data;
		if (!TellCache(hyp, node, (HYP_NODE *) image))
		{
			g_free(image);
			g_free(data);
			hyp_utf8_fprintf(opts->errorfile, _("failed to cache compressed image data for %u"), node);
			goto error;
		}
	} else
	{
		ASSERT(!image->decompressed);
		data = (unsigned char *)image->pic.fd_addr;
	}
	data_size = GetDataSize(hyp, node);
	buf = g_new(unsigned char, data_size);
	if (buf == NULL)
		goto error;
	if (data_size < SIZEOF_HYP_PICTURE ||
		!GetEntryBytes(hyp, node, data, buf, data_size))
	{
		hyp_utf8_fprintf(opts->errorfile, _("failed to decode image header for %u"), node);
		goto error;
	}
	
	hyp_pic_get_header(&hyp_pic, buf);
	
	format = format_from_pic(opts, hyp->indextable[node], default_format);
	data_size -= SIZEOF_HYP_PICTURE;
	
	pic.pi_width = hyp_pic.width;
	pic.pi_height = hyp_pic.height;
	pic.pi_planes = hyp_pic.planes;
	pic.pi_compressed = 1;
	pic_stdpalette(pic.pi_palette, pic.pi_planes);
	pic_calcsize(&pic);
	if (data_size != pic.pi_picsize)
	{
		hyp_utf8_fprintf(opts->errorfile, _("format error in image of node %u: %dx%dx%d datasize=%ld picsize=%ld\n"), node, hyp_pic.width, hyp_pic.height, hyp_pic.planes, data_size, pic.pi_picsize);
		goto error;
	}
	pic.pi_name = image_name(format, hyp, node, opts->image_name_prefix);
	if (empty(pic.pi_name))
		goto error;
	
	conv = g_new(_UBYTE, data_size);
	if (conv == NULL)
		goto error;
	
	path = g_build_filename(opts->output_dir, pic.pi_name, NULL);
	fp = hyp_utf8_fopen(path, "wb");
	if (fp == NULL)
	{
		FileErrorErrno(path);
		g_free(path);
		goto error;
	}
	g_free(path);
	
	switch (format)
	{
	case HYP_PIC_IMG:
		pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
		
		g_free(buf);
		buf = NULL;
		header_size = img_header(&buf, &pic);
		if (buf == NULL)
		{
			oom();
			goto error;
		}
		data_size = img_pack(buf, conv, &pic);
		if ((long) fwrite(buf, 1, header_size + data_size, fp) != header_size + data_size)
		{
			FileErrorErrno(pic.pi_name);
			goto error;
		}
		break;

	case HYP_PIC_IFF:
		{
			_UBYTE headerbuf[IFF_HEADER_BUFSIZE];
			
			pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
			header_size = iff_header(headerbuf, &pic);
			if ((long) fwrite(headerbuf, 1, header_size, fp) != header_size)
			{
				FileErrorErrno(pic.pi_name);
				goto error;
			}
			g_free(buf);
			buf = iff_pack(conv, &pic);
			if (buf == NULL)
			{
				goto error;
			}
			if ((long) fwrite(buf, 1, pic.pi_datasize, fp) != pic.pi_datasize)
			{
				FileErrorErrno(pic.pi_name);
				goto error;
			}
		}
		break;

	case HYP_PIC_BMP:
		pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
		g_free(buf);
		buf = NULL;
		header_size = bmp_header(&buf, &pic, pic.pi_planes == 4 ? bmp_coltab4 : bmp_coltab8);
		if (buf == NULL)
		{
			oom();
			goto error;
		}
		data_size = bmp_pack(buf, conv, &pic, TRUE, pic.pi_planes == 4 ? bmp_revtab4 : bmp_revtab8);
		if ((long) fwrite(buf, 1, header_size + data_size, fp) != header_size + data_size)
		{
			FileErrorErrno(pic.pi_name);
			goto error;
		}
		break;

	case HYP_PIC_ICN:
		if (icn_fwrite(fp, buf + SIZEOF_HYP_PICTURE, &pic) == FALSE)
		{
			FileErrorErrno(pic.pi_name);
			goto error;
		}
		break;
	
	case HYP_PIC_GIF:
		pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
		
		g_free(buf);
		buf = NULL;
		if (!gif_fwrite(fp, conv, &pic) ||
			fflush(fp) != 0 ||
			ferror(fp))
		{
			FileErrorErrno(pic.pi_name);
			goto error;
		}
		break;

	case HYP_PIC_UNKNOWN:
	case HYP_PIC_PNG:
		unreachable();
		break;
	}
	
	fclose(fp);
	fp = NULL;
	if (opts->verbose >= 2 && outfile != stdout)
		hyp_utf8_fprintf(stdout, _("wrote image %s (%dx%dx%d)\n"), pic.pi_name, pic.pi_width, pic.pi_height, 1 << pic.pi_planes);
	goto done;
error:
	retval = FALSE;
done:
	if (fp != NULL)
		hyp_utf8_fclose(fp);
	g_free(pic.pi_name);
	g_free(conv);
	g_free(buf);
	
	return retval;
}

/* ------------------------------------------------------------------------- */

static symtab_entry *ref_loadsyms(HYP_DOCUMENT *hyp)
{
	symtab_entry *syms = NULL;
	symtab_entry **last = &syms;
	symtab_entry *sym;
	
	/* load REF if not done already */
	if (hyp->ref == NULL)
	{
		char *filename;
		int ret;

		filename = replace_ext(hyp->file, HYP_EXT_HYP, HYP_EXT_REF);

		ret = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret >= 0)
		{
			hyp->ref = ref_load(filename, ret, FALSE);
			hyp_utf8_close(ret);
		}
		g_free(filename);
	}
	ref_conv_to_utf8(hyp->ref);
	if (hyp->ref != NULL)
	{
		hyp_nodenr node_num;
		REF_MODULE *mod;
		char *nodename = NULL;
		const REF_ENTRY *entry;
		
		for (mod = hyp->ref->modules; mod != NULL; mod = mod->next)
		{
			for (node_num = 0; node_num < mod->num_entries; node_num++)
			{
				entry = &mod->entries[node_num];
				
				if (entry->type == REF_NODENAME)
				{
					nodename = entry->name.utf8;
				} else if (entry->type != REF_ALIASNAME && entry->type != REF_LABELNAME)
				{
					continue;
				}
				sym = g_new(symtab_entry, 1);
				if (sym == NULL)
					return syms;
				sym->nodename = nodename;
				sym->type = entry->type;
				sym->name = entry->name.utf8;
				sym->lineno = entry->lineno;
				sym->freeme = FALSE;
				sym->from_ref = TRUE;
				sym->from_idx = FALSE;
				sym->referenced = FALSE;
				sym->next = NULL;
				*last = sym;
				last = &(sym)->next;
			}
			/* only search the first module */
			break;
		}
	}
	return syms;
}

/* ------------------------------------------------------------------------- */

static void free_symtab(symtab_entry *sym)
{
	symtab_entry *next;
	
	while (sym)
	{
		if (sym->freeme)
		{
			g_free(sym->nodename);
			g_free(sym->name);
		}
		next = sym->next;
		g_free(sym);
		sym = next;
	}
}

/* ------------------------------------------------------------------------- */

static gboolean recompile_stg(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	FILE *outfile = opts->outfile;
	
	UNUSED(argc);
	UNUSED(argv);
	
	/* output_charset = HYP_CHARSET_ATARI; */
	stg_nl = (outfile == stdout || output_charset != HYP_CHARSET_ATARI) ? "\n" : "\015\012";
	
	ret = TRUE;
	
	hyp_utf8_fprintf_charset(outfile, output_charset, "## created by %s Version %s%s", gl_program_name, gl_program_version, stg_nl);
	stg_out_globals(hyp, outfile);
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
			ret &= stg_out_node(hyp, opts, node, syms);
			break;
		case HYP_NODE_POPUP:
			ret &= stg_out_node(hyp, opts, node, syms);
			break;
		case HYP_NODE_IMAGE:
			if (opts->read_images)
				ret &= write_image(hyp, outfile, opts, node, STG_DEFAULT_PIC_TYPE);
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
				hyp_utf8_fprintf_charset(outfile, output_charset, "##symbol unused: \"%s\" \"%s\"%s", sym->nodename, sym->name, stg_nl);
			}
		}
	}
		
	free_symtab(syms);
	return ret;
}
