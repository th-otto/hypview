static const char *stg_nl;
static gboolean is_MASTER = TRUE;

typedef gboolean (*recompile_func)(HYP_DOCUMENT *hyp, hcp_opts *opt, int argc, const char **argv);


/*
 * node/label names are case sensitiv
 */
#define namecmp strcmp


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

/*
 * Map from VDI colors to ST standard pixel values, 4 planes.
 * Only used to construct the %dithermask
 */
static unsigned int const vdi_maptab16[16] = { 0, 15, 1, 2, 4, 6, 3, 5, 7,  8,  9, 10, 12, 14, 11, 13 };

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#ifdef CMDLINE_VERSION
static void oom(void)
{
	hyp_utf8_fprintf(stderr, "%s: %s\n", gl_program_name, strerror(ENOMEM));
}
#else
#define oom()
#endif

/* ------------------------------------------------------------------------- */

static gboolean check_long_filenames(const char *dir)
{
	long test1, test2;
	char *s, *s1, *s2;
	struct stat st;
	int ret;
	int fd;
	
	s1 = NULL;
	test1 = 11111110L;
	for (;;)
	{
		g_free(s1);
		test1++;
		s = g_strdup_printf("%ld.tmp", test1);
		s1 = g_build_filename(dir, s, NULL);
		g_free(s);
		test2 = test1 * 10 + 1;
		s = g_strdup_printf("%ld.tmp", test2);
		s2 = g_build_filename(dir, s, NULL);
		g_free(s);
		ret = rpl_stat(s2, &st);
		g_free(s2);
		if (ret == 0)
		{
			continue;
		}
		if (errno == ENAMETOOLONG)
		{
			g_free(s1);
			return FALSE;
		}
		if (errno != ENOENT)
		{
			continue;
		}
		/*
		 * try to create tmp file. if it exists, we should get EEXIST or some access error
		 */
		fd = open(s1, O_CREAT, 0644);
		if (fd >= 0)
			break;
	}
	
	close(fd);
	s = g_strdup_printf("%ld.tmp", test2);
	s2 = g_build_filename(dir, s, NULL);
	g_free(s);
	ret = rpl_stat(s2, &st);
	fd = errno;
	g_free(s2);
	
	unlink(s1);
	g_free(s1);
	if (ret != 0 && fd == ENOENT)
	{
		return TRUE;
	}
	
	return FALSE;
}

/* ------------------------------------------------------------------------- */

static char *format_dithermask(unsigned short dithermask)
{
	char *buf, *ret;
	int color;
	
	ret = buf = g_new(char, 17);
	for (color = 0; color < 16 && dithermask != 0; color++)
	{
		unsigned short mask = (1 << vdi_maptab16[color]);
		if (dithermask & mask)
		{
			*buf++ = '1';
			dithermask &= ~mask;
		} else
		{
			*buf++ = '0';
		}
	}
	*buf = '\0';
	return ret;
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
		hyp_utf8_fprintf(opts->errorfile, _("%sGEM images are not displayable in HTML, using %s instead\n"), _("warning: "), hcp_pic_format_to_name(default_format));
	}
#ifndef HAVE_PNG
	if (format == HYP_PIC_PNG)
	{
		format = default_format;
		hyp_utf8_fprintf(opts->errorfile, _("%sPNG not supported on this platform, using %s instead\n"), _("warning: "), hcp_pic_format_to_name(default_format));
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

static gboolean sym_check_links(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, symtab_entry **syms)
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

static gboolean write_image(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, hyp_pic_format default_format, GString *out)
{
	unsigned char *data;
	unsigned char *buf = NULL;
	unsigned char *conv = NULL;
	FILE *fp = NULL;
	gboolean retval = TRUE;
	hyp_pic_format format;
	PICTURE pic;
	long header_size;
	long data_size;
	HYP_IMAGE *image;
	
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
		image->data_size = GetDataSize(hyp, node);
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
	buf = g_new(unsigned char, image->data_size);
	if (buf == NULL)
		goto error;
	if (image->data_size < SIZEOF_HYP_PICTURE ||
		!GetEntryBytes(hyp, node, data, buf, image->data_size))
	{
		hyp_utf8_fprintf(opts->errorfile, _("failed to decode image header for %u"), node);
		goto error;
	}
	
	if (hyp_pic_get_header(image, buf, opts->errorfile) == FALSE)
		goto error;
	if (image->incomplete)
	{
		unsigned char *newbuf = g_renew(unsigned char, buf, image->image_size);
		if (newbuf == NULL)
			goto error;
		buf = newbuf;
	}
	hyp_pic_apply_planemasks(image, buf);
	
	format = format_from_pic(opts, hyp->indextable[node], default_format);
	
	pic.pi_width = image->pic.fd_w;
	pic.pi_height = image->pic.fd_h;
	pic.pi_planes = image->pic.fd_nplanes;
	pic.pi_compressed = 1;
	pic_stdpalette(pic.pi_palette, pic.pi_planes);
	pic_calcsize(&pic);
	if (image->image_size != (unsigned long)pic.pi_picsize + SIZEOF_HYP_PICTURE)
	{
		char *colors = pic_colornameformat(image->pic.fd_nplanes);
		hyp_utf8_fprintf(opts->errorfile, _("format error in image of node %u: %dx%d%s datasize=%ld picsize=%ld\n"), node, image->pic.fd_w, image->pic.fd_h, colors, image->image_size, pic.pi_picsize);
		g_free(colors);
		goto error;
	}
	pic.pi_name = image_name(format, hyp, node, opts->image_name_prefix);
	if (empty(pic.pi_name))
		goto error;
	
	conv = g_new(_UBYTE, pic.pi_picsize);
	if (conv == NULL)
		goto error;
	
	if (out == NULL)
	{
		char *path = g_build_filename(opts->output_dir, pic.pi_name, NULL);
		fp = hyp_utf8_fopen(path, "wb");
		if (fp == NULL)
		{
			FileErrorErrno(path);
			g_free(path);
			goto error;
		}
		g_free(path);
	}
	
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
		if (out)
		{
			g_string_set_size(out, header_size + data_size);
			memcpy(out->str, buf, header_size + data_size);
		} else
		{
			if ((long) fwrite(buf, 1, header_size + data_size, fp) != header_size + data_size)
			{
				FileErrorErrno(pic.pi_name);
				goto error;
			}
		}
		break;

	case HYP_PIC_IFF:
		{
			_UBYTE headerbuf[IFF_HEADER_BUFSIZE];
			
			pic_planes_to_interleaved(conv, buf + SIZEOF_HYP_PICTURE, &pic);
			header_size = iff_header(headerbuf, &pic);
			if (out)
			{
				g_string_set_size(out, header_size + pic.pi_datasize);
				memcpy(out->str, buf, header_size);
			} else
			{
				if ((long) fwrite(headerbuf, 1, header_size, fp) != header_size)
				{
					FileErrorErrno(pic.pi_name);
					goto error;
				}
			}
			g_free(buf);
			buf = iff_pack(conv, &pic);
			if (buf == NULL)
			{
				goto error;
			}
			if (out)
			{
				memcpy(out->str + header_size, buf, pic.pi_datasize);
			} else
			{
				if ((long) fwrite(buf, 1, pic.pi_datasize, fp) != pic.pi_datasize)
				{
					FileErrorErrno(pic.pi_name);
					goto error;
				}
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
		if (out)
		{
			g_string_set_size(out, header_size + data_size);
			memcpy(out->str, buf, header_size + data_size);
		} else
		{
			if ((long) fwrite(buf, 1, header_size + data_size, fp) != header_size + data_size)
			{
				FileErrorErrno(pic.pi_name);
				goto error;
			}
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
		if (out)
		{
			buf = gif_pack(conv, &pic);
			if (buf)
			{
				g_string_set_size(out, pic.pi_datasize);
				memcpy(out->str, buf, pic.pi_datasize);
			}
		} else
		{
			if (!gif_fwrite(fp, conv, &pic) ||
				fflush(fp) != 0 ||
				ferror(fp))
			{
				FileErrorErrno(pic.pi_name);
				goto error;
			}
		}
		break;

	case HYP_PIC_UNKNOWN:
	case HYP_PIC_PNG:
		unreachable();
		break;
	}
	
	if (fp != NULL)
		hyp_utf8_fclose(fp);
	fp = NULL;
	if (opts->verbose >= 2 && opts->outfile != stdout)
	{
		char *colors = pic_colornameformat(pic.pi_planes);
		hyp_utf8_fprintf(stdout, _("wrote image %s (%dx%d%s)\n"), pic.pi_name, pic.pi_width, pic.pi_height, colors);
		g_free(colors);
	}
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

static void write_strout(GString *s, FILE *outfp)
{
	if (strcmp(stg_nl, "\n") != 0)
	{
		const char *txt = s->str;
		while (*txt)
		{
			if (*txt == '\n')
				fputs(stg_nl, outfp);
			else
				fputc(*txt, outfp);
			txt++;
		}
	} else
	{
		fwrite(s->str, 1, s->len, outfp);
	}
}

