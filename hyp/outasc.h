/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *ascii_quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
}

/* ------------------------------------------------------------------------- */

static void ascii_out_text(hcp_opts *opts, FILE *outfile, const char *text, unsigned char textattr)
{
	const char *p = text;
	int i;
	char buf[HYP_UTF8_CHARMAX + 1];
	gboolean converror = FALSE;
	
	while (*p)
	{
		if (textattr & HYP_TXT_UNDERLINED)
		{
			fputc('_', outfile);
			fputc('\b', outfile);
		}
		p = hyp_utf8_conv_char(opts->output_charset, p, buf, &converror);
		for (i = 0; buf[i] != 0; i++)
			fputc(buf[i], outfile);
		if (textattr & HYP_TXT_BOLD)
		{
			fputc('\b', outfile);
			for (i = 0; buf[i] != 0; i++)
				fputc(buf[i], outfile);
		}
	}
}

/* ------------------------------------------------------------------------- */

static void ascii_out_str(HYP_DOCUMENT *hyp, hcp_opts *opts, FILE *outfile, const unsigned char *str, size_t len, unsigned char textattr)
{
	char *text = hyp_conv_to_utf8(hyp->comp_charset, str, len);
	ascii_out_text(opts, outfile, text, textattr);
	g_free(text);
}

/* ------------------------------------------------------------------------- */

static gboolean ascii_out_attr(FILE *outfile, unsigned char oldattr, unsigned char newattr)
{
	UNUSED(outfile);
	if (oldattr != newattr)
	{
		return TRUE;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static gboolean ascii_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node)
{
	char *str;
	gboolean at_bol;
	int in_tree;
	unsigned char textattr;
	unsigned char linkattr;
	long lineno;
	gboolean manlike;
	HYP_NODE *nodeptr;
	char *title;
	FILE *outfile = opts->outfile;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		ascii_out_str(hyp, opts, outfile, textstart, src - textstart, manlike ? textattr : 0); \
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
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s", stg_nl); \
		in_tree = -1; \
		at_bol = TRUE; \
	}
	
	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		struct stat st;
		
		manlike = FALSE;
		linkattr = HYP_TXT_NORMAL;
		if (rpl_fstat(fileno(outfile), &st) == 0)
		{
			if (S_ISFIFO(st.st_mode) || S_ISCHR(st.st_mode))
			{
				manlike = TRUE;
				linkattr = HYP_TXT_BOLD;
			}
		}
		hyp_node_find_windowtitle(nodeptr);
		title = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
		if (title == NULL)
			title = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[node]->name, STR0TERM);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s%s", title, stg_nl);
		g_free(title);
		
		end = nodeptr->end;

		/*
		 * scan through esc commands, skipping graphic commands
		 */
		src = nodeptr->start;
		while (src < end && *src == HYP_ESC)
		{
			switch (src[1])
			{
			case HYP_ESC_PIC:
			case HYP_ESC_LINE:
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				break;
			case HYP_ESC_WINDOWTITLE:
				break;
			case HYP_ESC_EXTERNAL_REFS:
				{
					hyp_nodenr dest_page;
					char *text;
					
					dest_page = DEC_255(&src[3]);
					text = hyp_conv_to_utf8(hyp->comp_charset, src + 5, max(src[2], 5u) - 5u);
					text = chomp(text);
					if (hypnode_valid(hyp, dest_page))
					{
						str = ascii_quote_nodename(hyp, dest_page);
					} else
					{
						str = hyp_invalid_page(dest_page);
					}
					if (empty(text) || strcmp(str, text) == 0)
						hyp_utf8_fprintf_charset(outfile, opts->output_charset, "See also: %s%s", str, stg_nl);
					else
						hyp_utf8_fprintf_charset(outfile, opts->output_charset, "See also: %s%s", text, stg_nl);
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
		
		while (src < end)
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
						char *dest;
						size_t len;
						
						type = *src;
						if (type == HYP_ESC_LINK_LINE || type == HYP_ESC_ALINK_LINE)
						{
							/* line = DEC_255(&src[1]); */
							src += 2;
						}
						dest_page = DEC_255(&src[1]);
						src += 3;
						if (hypnode_valid(hyp, dest_page))
						{
							dest = ascii_quote_nodename(hyp, dest_page);
						} else
						{
							dest = hyp_invalid_page(dest_page);
						}
						if (*src <= HYP_STRLEN_OFFSET)
						{
							src++;
							str = g_strdup(dest);
						} else
						{
							len = *src - HYP_STRLEN_OFFSET;
							src++;
							textstart = src;
							str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
							src += len;
						}
						FLUSHTREE();
						if (opts->bracket_links)
						{
							ascii_out_text(opts, outfile, "[", linkattr);
							ascii_out_text(opts, outfile, str, linkattr);
							ascii_out_text(opts, outfile, "]", linkattr);
						} else
						{
							ascii_out_text(opts, outfile, str, linkattr);
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
						_WORD tree;

						tree = DEC_255(&src[3]);
						FLUSHLINE();
						if (tree != in_tree)
						{
							FLUSHTREE();
							in_tree = tree;
						}
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
					if (ascii_out_attr(outfile, textattr, *src - HYP_ESC_TEXTATTR_FIRST))
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
				src++;
				textstart = src;
			} else
			{
				FLUSHTREE();
				src++;
			}
		}
		DUMPTEXT();
		if (ascii_out_attr(outfile, textattr, 0))
			at_bol = FALSE;
		FLUSHLINE();
		FLUSHTREE();
		++lineno;
		
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(opts->errorfile, _("%s: Node %u: failed to decode\n"), hyp->file, node);
	}

#undef DUMPTEXT
#undef FLUSHLINE
#undef FLUSHTREE
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean recompile_ascii(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	int i;
	gboolean found;
	FILE *outfile = opts->outfile;
	
	stg_nl = outfile == stdout ? "\n" : "\015\012";
	
	ret = TRUE;
		
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		if (argc == 0)
		{
			found = TRUE;
		} else
		{
			found = FALSE;
			for (i = 0; i < argc; i++)
			{
				if (argv[i])
				{
					char *s1, *s2;
					
					s1 = hyp_conv_to_utf8(hyp->comp_charset, entry->name, STR0TERM);
					s2 = hyp_conv_to_utf8(hyp_get_current_charset(), argv[i], STR0TERM);
					if (namecmp(s1, s2) == 0)
					{
						argv[i] = NULL;
						found = TRUE;
					}
					g_free(s2);
					g_free(s1);
				}
			}
		}
		if (!found)
			continue;
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
			if (!opts->gen_index && argc == 0)
				continue;
		}
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= ascii_out_node(hyp, opts, node);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, "%s%s%s", stg_nl, stg_nl, stg_nl);
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
	
	for (i = 0; i < argc; i++)
	{
		if (argv[i] != NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, _("'%s' not found.\n"), argv[i]);
			ret = FALSE;
		}
	}
	
	return ret;
}
