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

static void stg_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, FILE *outfile)
{
	char *str;

	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@if VERSION >= 6%s", stg_nl);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@os %s%s", hyp_osname(hyp->comp_os), stg_nl);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@charset %s%s", hyp_charset_name(hyp->comp_charset), stg_nl);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@inputenc %s%s", hyp_charset_name(opts->output_charset), stg_nl);
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@endif%s", stg_nl);
	
	if (hyp->database != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@database \"%s\"%s", hyp->database, stg_nl);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@hostname \"%s\"%s", h->name, stg_nl);
		}
	}
	if (hypnode_valid(hyp, hyp->default_page))
	{
		str = stg_quote_nodename(hyp, hyp->default_page);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@default \"%s\"%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->hcp_options != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@options \"%s\"%s", hyp->hcp_options, stg_nl);
	}
	if (hyp->author != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@author \"%s\"%s", hyp->author, stg_nl);
	}
	if (hypnode_valid(hyp, hyp->help_page))
	{
		str = stg_quote_nodename(hyp, hyp->help_page);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@help \"%s\"%s", str, stg_nl);
		g_free(str);
	}
	if (hyp->version != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@$VER: %s%s", hyp->version, stg_nl);
	}
	if (hyp->subject != NULL)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@subject \"%s\"%s", hyp->subject, stg_nl);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@width %d%s", hyp->line_width, stg_nl);
	}
#if 0
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, _("ST-Guide flags: $%04x%s"), hyp->st_guide_flags, stg_nl);
	}
#endif
	fputs(stg_nl, outfile);
	fputs(stg_nl, outfile);
}

/* ------------------------------------------------------------------------- */

static void stg_out_str(HYP_DOCUMENT *hyp, hcp_opts *opts, FILE *outfile, const unsigned char *str, size_t len)
{
	char *dst, *p;
	gboolean converror = FALSE;
	
	dst = hyp_conv_charset(hyp->comp_charset, opts->output_charset, str, len, &converror);
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
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@remark %ux%ux%u%s", adm->pixwidth, adm->pixheight, 1 << adm->planes, stg_nl);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s \"%s\" %d%s",
				adm->islimage ? "@limage" : "@image",
				fname,
				adm->x_offset,
				stg_nl);
			g_free(fname);
		}
		break;
	case HYP_ESC_LINE:
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@line %d %d %d %d %d%s",
			adm->x_offset, adm->width, adm->height,
			adm->begend,
			adm->style,
			stg_nl);
		break;
	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s %d %d %d %d%s",
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

static void stg_out_labels(FILE *outfile, HYP_DOCUMENT *hyp, hcp_opts *opts, const INDEX_ENTRY *entry, long lineno, symtab_entry *syms)
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
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@symbol %s \"%s\"%s", sym_typename(sym), str, stg_nl);
			g_free(str);
			sym->referenced = TRUE;
		}
		sym = sym_find(sym->next, nodename, REF_LABELNAME);
	}
	g_free(nodename);
}

/* ------------------------------------------------------------------------- */

static void stg_out_alias(FILE *outfile, HYP_DOCUMENT *hyp, hcp_opts *opts, const INDEX_ENTRY *entry, symtab_entry *syms)
{
	char *nodename;
	symtab_entry *sym;
	
	nodename = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
	sym = sym_find(syms, nodename, REF_ALIASNAME);
	while (sym)
	{
		char *str = stg_quote_name(sym->name, FALSE);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@symbol %s \"%s\"%s", sym_typename(sym), str, stg_nl);
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
		stg_out_str(hyp, opts, outfile, textstart, src - textstart); \
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
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@endtree%s", stg_nl); \
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
				hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s \"%s\" \"%s\"%s", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str, title, stg_nl);
				g_free(title);
				g_free(buf);
			} else
			{
				hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s \"%s\"%s", entry->type == HYP_NODE_INTERNAL ? "@node" : "@pnode", str, stg_nl);
			}
			g_free(str);
			/*
			 * check for alias names in ref file
			 */
			stg_out_alias(outfile, hyp, opts, entry, syms);
			
			if (entry->type == HYP_NODE_INTERNAL)
			{
				if (hypnode_valid(hyp, entry->next) &&
					/* physical next page is default if not set */
					(node + 1u) != entry->next)
				{
					str = stg_quote_nodename(hyp, entry->next);
					hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@next \"%s\"%s", str, stg_nl);
					g_free(str);
				}
				
				if (hypnode_valid(hyp, entry->previous) &&
					 /* physical prev page is default if not set */
					(node - 1u) != entry->previous &&
					(entry->previous != 0 || node != 0))
				{
					str = stg_quote_nodename(hyp, entry->previous);
					hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@prev \"%s\"%s", str, stg_nl);
					g_free(str);
				}
				
				if (hypnode_valid(hyp, entry->toc_index) &&
					/* physical first page is default if not set */
					entry->toc_index != 0)
				{
					str = stg_quote_nodename(hyp, entry->toc_index);
					hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@toc \"%s\"%s", str, stg_nl);
					g_free(str);
				}
			}
		}
				
		if (node == hyp->index_page)
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@autorefon%s", stg_nl);

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
						hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@xref \"%s\"%s", str, stg_nl);
					else
						hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@xref \"%s\" \"%s\"%s", str, text, stg_nl);
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
		stg_out_labels(outfile, hyp, opts, entry, lineno, syms);
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
								hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@{\"%s\" %s}", str, cmd);
							else if (!str_equal /* || node == hyp->index_page */ || opts->all_links)
								hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@{\"%s\" %s \"%s\"}", str, cmd, dest);
							else
								stg_out_str(hyp, opts, outfile, textstart, len);
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
										hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@{\"%s\" %s \"%s/%u\"}", str, cmd, dest, line);
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
										hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@{\"%s\" %s \"%s\" \"%s\"}", str, cmd, dest, quoted);
										g_free(quoted);
									} else
									{
										hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@{\"%s\" %s \"%s\" %u}", str, cmd, dest, line + 1);
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
							hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@tree %d%s", tree, stg_nl);
							in_tree = tree;
						}
						hyp_utf8_fprintf_charset(outfile, opts->output_charset, "   %d \"%s\" %u%s", obj, str, line, stg_nl);
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
				stg_out_labels(outfile, hyp, opts, entry, lineno, syms);
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
		stg_out_labels(outfile, hyp, opts, entry, lineno, syms);
		stg_out_graphics(opts, outfile, hyp, hyp_gfx, lineno);
		
		if (hyp_gfx != NULL)
		{
			struct hyp_gfx *gfx, *next;
			
			for (gfx = hyp_gfx; gfx != NULL; gfx = next)
			{
				if (!gfx->used)
				{
					hyp_utf8_fprintf_charset(outfile, opts->output_charset, "##gfx unused: ");
					stg_out_gfx(opts, outfile, hyp, gfx);
				}
				next = gfx->next;
				g_free(gfx);
			}
		}
		
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "@endnode%s", stg_nl);
		if (node < hyp->last_text_page)
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s%s", stg_nl, stg_nl);
			
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
	stg_nl = (outfile == stdout || opts->output_charset != HYP_CHARSET_ATARI) ? "\n" : "\015\012";
	
	ret = TRUE;
	
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, "## created by %s Version %s%s", gl_program_name, gl_program_version, stg_nl);
	stg_out_globals(hyp, opts, outfile);
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
				hyp_utf8_fprintf_charset(outfile, opts->output_charset, "##symbol unused: \"%s\" \"%s\"%s", sym->nodename, sym->name, stg_nl);
			}
		}
	}
		
	free_symtab(syms);
	return ret;
}
