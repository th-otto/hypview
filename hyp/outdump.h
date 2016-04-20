/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean dump_node(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node)
{
	char *str;
	HYP_NODE *nodeptr;
	FILE *outfile = opts->outfile;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		str = hyp_conv_to_utf8(hyp->comp_charset, textstart, src - textstart); \
		if (str != NULL) \
			hyp_utf8_fprintf(outfile, _("Text: <%s>\n"), str); \
		g_free(str); \
	}

	if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
	{
		const unsigned char *src;
		const unsigned char *end;
		const unsigned char *textstart;
		
		src = nodeptr->start;
		end = nodeptr->end;
		textstart = src;
		
		while (src < end)
		{
			if (*src == HYP_ESC)
			{
				DUMPTEXT();
				src++;
				switch (*src)
				{
				case HYP_ESC_ESC:
					hyp_utf8_fprintf(outfile, _("<ESC>\n"));
					src++;
					break;
				
				case HYP_ESC_WINDOWTITLE:
					src++;
					str = hyp_conv_to_utf8(hyp->comp_charset, src, STR0TERM);
					hyp_utf8_fprintf(outfile, _("Title: %s\n"), str);
					g_free(str);
					src += ustrlen(src) + 1;
					break;

				case HYP_ESC_CASE_DATA:
					hyp_utf8_fprintf(outfile, _("Data: type %u, len %u\n"), src[0], src[1]);
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
							dest = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dest_page]->name, STR0TERM);
						} else
						{
							dest = hyp_invalid_page(dest_page);
						}
						if (*src <= HYP_STRLEN_OFFSET)
						{
							str = g_strdup(dest);
						} else
						{
							size_t len = *src - HYP_STRLEN_OFFSET;
							str = hyp_conv_to_utf8(hyp->comp_charset, src + 1, len);
							src += len;
						}
						switch (type)
						{
						case HYP_ESC_LINK:
							hyp_utf8_fprintf(outfile, _("Link: \"%s\" %u \"%s\"\n"), str, dest_page, dest);
							break;
						case HYP_ESC_LINK_LINE:
							hyp_utf8_fprintf(outfile, _("Link: \"%s\" %u \"%s\" %u\n"), str, dest_page, dest, line);
							break;
						case HYP_ESC_ALINK:
							hyp_utf8_fprintf(outfile, _("ALink: \"%s\" %u \"%s\"\n"), str, dest_page, dest);
							break;
						case HYP_ESC_ALINK_LINE:
							hyp_utf8_fprintf(outfile, _("ALink: \"%s\" %u \"%s\" %u\n"), str, dest_page, dest, line);
							break;
						}
						g_free(dest);
						g_free(str);
						src++;
					}
					break;
					
				case HYP_ESC_EXTERNAL_REFS:
					{
						hyp_nodenr dest_page;
						char *text;
						
						dest_page = DEC_255(&src[2]);
						text = hyp_conv_to_utf8(hyp->comp_charset, src + 4, max(src[1], 5u) - 5u);
						if (hypnode_valid(hyp, dest_page))
						{
							str = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dest_page]->name, STR0TERM);
						} else
						{
							str = hyp_invalid_page(dest_page);
						}
						hyp_utf8_fprintf(outfile, _("XRef \"%s\" \"%s\"\n"), text, str);
						g_free(str);
						g_free(text);
						src += src[1] - 1;
					}
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
							str = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dest_page]->name, STR0TERM);
						} else
						{
							str = hyp_invalid_page(dest_page);
						}
						hyp_utf8_fprintf(outfile, _("Tree: tree=%d, obj=%d, line=%u: \"%s\"\n"), tree, obj, line, str);
						g_free(str);
						src += 9;
					}
					break;
					
				case HYP_ESC_PIC:
					{
						hyp_nodenr num;
						_WORD x_offset;
						_WORD y_offset;
						_WORD width;
						_WORD height;
						gboolean islimage;
						
						num = DEC_255(&src[1]);
						x_offset = src[3];
						y_offset = DEC_255(&src[4]);
						width = src[6];
						height = src[7];
						islimage = hyp->comp_vers >= 3 && src[6] == 1;
						hyp_utf8_fprintf(outfile, _("%s: x=%d, y=%d, w=%d, h=%d, num=%u\n"),
							islimage ? _("limage") : _("image"),
							x_offset, y_offset, width, height, num);
						src += 8;
					}
					break;
					
				case HYP_ESC_LINE:
					{
						_WORD x_offset;
						_WORD y_offset;
						_WORD width;
						_WORD height;
						unsigned char attr;
						
						x_offset = src[1];
						y_offset = DEC_255(&src[2]);
						width = src[4];
						height = src[5];
						attr = src[6];
						hyp_utf8_fprintf(outfile, _("Line: x=%d, y=%d, w=%d, h=%d, attr=0x%02x, begend=%x, style=%d\n"),
							x_offset, y_offset, width - 128, height - 1, attr, (attr - 1) & 7, min(max(((attr - 1) >> 3), 0), 6) + 1);
						src += 7;
					}
					break;
					
				case HYP_ESC_BOX:
				case HYP_ESC_RBOX:
					{
						_WORD x_offset;
						_WORD y_offset;
						_WORD width;
						_WORD height;
						unsigned char attr;
						
						x_offset = src[1];
						y_offset = DEC_255(&src[2]);
						width = src[4];
						height = src[5];
						attr = src[6];
						hyp_utf8_fprintf(outfile, _("%s: x=%d, y=%d, w=%d, h=%d, attr=%u\n"),
							*src == HYP_ESC_BOX ? _("Box") : _("RBox"), x_offset, y_offset, width, height, attr);
						src += 7;
					}
					break;
					
				case HYP_ESC_CASE_TEXTATTR:
					hyp_utf8_fprintf(outfile, _("Textattr: $%x\n"), *src - HYP_ESC_TEXTATTR_FIRST);
					src++;
					break;
				
				default:
					hyp_utf8_fprintf(outfile, _("<unknown hex esc $%02x>\n"), *src);
					break;
				}
				textstart = src;
			} else if (*src == HYP_EOL)
			{
				DUMPTEXT();
				hyp_utf8_fprintf(outfile, _("<EOL>\n"));
				src++;
				textstart = src;
			} else
			{
				src++;
			}
		}
		DUMPTEXT();
		
		hyp_node_free(nodeptr);
	} else
	{
		hyp_utf8_fprintf(outfile, _("%s: Node %u: failed to decode\n"), hyp->file, node);
	}

#undef DUMPTEXT
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean dump_image(HYP_DOCUMENT *hyp, FILE *outfile, hyp_nodenr node)
{
	unsigned char *data;
	unsigned char hyp_pic_raw[SIZEOF_HYP_PICTURE];
	HYP_PICTURE hyp_pic;
	gboolean retval = TRUE;
	
	data = hyp_loaddata(hyp, node);

	if (data == NULL)
		goto error;

	if (!GetEntryBytes(hyp, node, data, hyp_pic_raw, SIZEOF_HYP_PICTURE))
		goto error;
	
	hyp_pic_get_header(&hyp_pic, hyp_pic_raw);

	hyp_utf8_fprintf(outfile, _("  Width: %d\n"), hyp_pic.width);
	hyp_utf8_fprintf(outfile, _("  Height: %d\n"), hyp_pic.height);
	hyp_utf8_fprintf(outfile, _("  Planes: %d\n"), hyp_pic.planes);
	hyp_utf8_fprintf(outfile, _("  Plane Mask: $%02x\n"), hyp_pic.plane_pic);
	hyp_utf8_fprintf(outfile, _("  Plane On-Off: $%02x\n"), hyp_pic.plane_on_off);
	hyp_utf8_fprintf(outfile, _("  Filler: $%02x\n"), hyp_pic.filler);
	
	goto done;
error:
	retval = FALSE;
done:
	g_free(data);
	
	return retval;
}

/* ------------------------------------------------------------------------- */

static void dump_globals(HYP_DOCUMENT *hyp, FILE *outfile)
{
	hyp_utf8_fprintf(outfile, _("OS: %s\n"), hyp_osname(hyp->comp_os));
	hyp_utf8_fprintf(outfile, _("Charset: %s\n"), hyp_charset_name(hyp->comp_charset));
	if (hyp->database != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Database: %s\n"), hyp->database);
	}
	if (hyp->author != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Author: %s\n"), hyp->author);
	}
	if (hyp->version != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Version: %s\n"), hyp->version);
	}
	if (hyp->help_name != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Help Node: %s\n"), hyp->help_name);
	}
	if (hyp->default_name != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Default Node: %s\n"), hyp->default_name);
	}
	if (hyp->hostname != NULL)
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
		{
			hyp_utf8_fprintf(outfile, _("Host-Application: %s\n"), h->name);
		}
	}
	if (hyp->hcp_options != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Compiler-Options: %s\n"), hyp->hcp_options);
	}
	if (hyp->subject != NULL)
	{
		hyp_utf8_fprintf(outfile, _("Subject: %s\n"), hyp->subject);
	}
	/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
	{
		hyp_utf8_fprintf(outfile, _("Line width: %d\n"), hyp->line_width);
	}
	/* if (hyp->st_guide_flags != 0) */
	{
		hyp_utf8_fprintf(outfile, _("ST-Guide flags: $%04x\n"), hyp->st_guide_flags);
	}
	hyp_utf8_fprintf(outfile, _("Compiler Version: %u\n"), hyp->comp_vers);
}

/* ------------------------------------------------------------------------- */

static gboolean recompile_dump(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	char *str;
	unsigned long compressed_size, size;
	size_t namelen;
	size_t slen;
	INDEX_ENTRY *entry;
	gboolean ret;
	gboolean found;
	int i;
	FILE *outfile = opts->outfile;
	
	dump_globals(hyp, outfile);
	hyp_utf8_fprintf(outfile, _("Index nodes: %u\n"), hyp->num_index);
	hyp_utf8_fprintf(outfile, _("Index table size: %ld\n"), hyp->itable_size);
	hyp_utf8_fprintf(outfile, "\n\n");
	
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
		compressed_size = GetCompressedSize(hyp, node);
		size = GetDataSize(hyp, node);
		namelen = entry->length - SIZEOF_INDEX_ENTRY;
		hyp_utf8_fprintf(outfile, _("Index entry %u:\n"), node);
		str = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
		hyp_utf8_fprintf(outfile, _("  Name: %s"), str);
		slen = hyp->comp_charset == HYP_CHARSET_UTF8 ? strlen(str) : g_utf8_str_len(str, STR0TERM);
		g_free(str);
		if (namelen > 0)
		{
			/* dump maybe garbage after name */
			slen += 1;
			if ((slen & 1) /* && entry->name[slen] == 0 */)
				slen++;
			if (slen < namelen)
			{
				size_t i;
				
				hyp_utf8_fprintf(outfile, " (");
				for (i = slen; i < namelen; i++)
				{
					if (i != slen)
						hyp_utf8_fprintf(outfile, " ");
					hyp_utf8_fprintf(outfile, "$%02x", entry->name[i]);
				}
				hyp_utf8_fprintf(outfile, ")");
			}
		}
		hyp_utf8_fprintf(outfile, "\n");
		hyp_utf8_fprintf(outfile, _("  Entry length: %u\n"), entry->length);
		hyp_utf8_fprintf(outfile, _("  Offset: %ld $%lx\n"), entry->seek_offset, entry->seek_offset);
		hyp_utf8_fprintf(outfile, _("  Compressed: %ld $%lx\n"), compressed_size, compressed_size);
		hyp_utf8_fprintf(outfile, _("  Uncompressed: %ld $%lx\n"), size, size);
		hyp_utf8_fprintf(outfile, _("  Next: %u $%04x\n"), entry->next, entry->next);
		hyp_utf8_fprintf(outfile, _("  Previous: %u $%04x\n"), entry->previous, entry->previous);
		hyp_utf8_fprintf(outfile, _("  Up: %u $%04x\n"), entry->toc_index, entry->toc_index);
		
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
			hyp_utf8_fprintf(outfile, _("  Type: internal node\n"));
			ret &= dump_node(hyp, opts, node);
			break;
		case HYP_NODE_POPUP:
			hyp_utf8_fprintf(outfile, _("  Type: pop-up node\n"));
			ret &= dump_node(hyp, opts, node);
			break;
		case HYP_NODE_EXTERNAL_REF:
			hyp_utf8_fprintf(outfile, _("  Type: external node\n"));
			break;
		case HYP_NODE_IMAGE:
			hyp_utf8_fprintf(outfile, _("  Type: image\n"));
			ret &= dump_image(hyp, outfile, node);
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
			hyp_utf8_fprintf(outfile, _("  Type: system node\n"));
			break;
		case HYP_NODE_REXX_SCRIPT:
			hyp_utf8_fprintf(outfile, _("  Type: REXX script\n"));
			break;
		case HYP_NODE_REXX_COMMAND:
			hyp_utf8_fprintf(outfile, _("  Type: REXX command\n"));
			break;
		case HYP_NODE_QUIT:
			hyp_utf8_fprintf(outfile, _("  Type: quit\n"));
			break;
		case HYP_NODE_CLOSE:
			hyp_utf8_fprintf(outfile, _("  Type: close\n"));
			break;
		case HYP_NODE_EOF:
			hyp_utf8_fprintf(outfile, _("  Type: EOF\n"));
			break;
		default:
			hyp_utf8_fprintf(outfile, _("  Type: unknown type %u\n"), entry->type);
			break;
		}
		hyp_utf8_fprintf(outfile, "\n\n");
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
