#include "hypdoc.h"
#include "hv_ascii.h"
#include "hypdebug.h"


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void AsciiClose(DOCUMENT *doc)
{
	FMT_ASCII *ascii = (FMT_ASCII *) doc->data;

	if (ascii == NULL)
		return;
	g_free(ascii->line_ptr);
	g_free(ascii);
	doc->data = NULL;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean AsciiGotoNode(DOCUMENT *doc, const char *chapter, hyp_nodenr node)
{
	UNUSED(doc);
	UNUSED(chapter);
	UNUSED(node);
	HYP_DBG(("AsciiGotoNode(Chapter: <%s> / <%u>)", printnull(chapter), node));
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static hyp_nodenr AsciiGetNode(DOCUMENT *doc)
{
	UNUSED(doc);
	HYP_DBG(("AsciiGetNode not implemented."));
	return HYP_NOINDEX;
}

/*** ---------------------------------------------------------------------- ***/

unsigned char *AsciiGetTextLine(const unsigned char *src, const unsigned char *end)
{
	unsigned char *dst;
	unsigned char val;
	unsigned char *ret;
	const unsigned char *ptr;
	size_t len;
	
	ptr = src;
	len = 0;
	while (ptr < end)
	{
		val = *ptr++;
		if ((val == '\t') && (gl_profile.viewer.ascii_tab_size))
		{
			len += gl_profile.viewer.ascii_tab_size - len % gl_profile.viewer.ascii_tab_size;
		} else if (val)
		{
			len++;
		} else
		{
			break;
		}
	}
	
	dst = ret = g_new(unsigned char, len + 1);
	if (ret == NULL)
		return NULL;
	
	ptr = src;
	while (ptr < end)
	{
		val = *ptr++;
		if ((val == '\t') && (gl_profile.viewer.ascii_tab_size))
		{
			val = gl_profile.viewer.ascii_tab_size - (dst - ret) % gl_profile.viewer.ascii_tab_size;
			while (val--)
				*dst++ = ' ';
		} else if (val)
		{
			*dst++ = val;
		} else
		{
			break;
		}
	}
	ASSERT((size_t)(dst - ret) == len);
	*dst = '\0';
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

static void AsciiPrep(DOCUMENT *doc)
{
	UNUSED(doc);
}

/*** ---------------------------------------------------------------------- ***/

long AsciiAutolocator(DOCUMENT *doc, long line, const char *search)
{
	FMT_ASCII *ascii = (FMT_ASCII *) doc->data;
	unsigned char *src;

	long len = strlen(search);

	if (!ascii)							/* no file loaded */
		return -1;

	if (doc->autolocator_dir > 0)
	{
		while (line < doc->lines)
		{
			src = ascii->line_ptr[line];
			if (src)
			{
				while (*src)
				{
					if (strncasecmp((const char *)src, search, len) == 0)
						return line;
					src++;
				}
			}
			line++;
		}
	} else
	{
		while (line > 0)
		{
			src = ascii->line_ptr[line];
			if (src)
			{
				while (*src)
				{
					if (strncasecmp((const char *)src, search, len) == 0)
						return line;
					src++;
				}
			}
			line--;
		}
	}
	return -1;
}

/*** ---------------------------------------------------------------------- ***/

gboolean AsciiBlockOperations(DOCUMENT *doc, hyp_blockop op, BLOCK *block, void *param)
{
	FMT_ASCII *ascii = (FMT_ASCII *) doc->data;

	if (!block->valid)
	{
		HYP_DBG(("Operation on invalid block"));
		return FALSE;
	}

	switch (op)
	{
	case BLK_ASCIISAVE:
		{
			long line;
			size_t ret;
			size_t len;
			unsigned char *line_buffer;
			const unsigned char *src;
			int *handle = (int *) param;

			line = block->start.line;

			while ((line < doc->lines) && (line <= block->end.line))
			{
				line_buffer = AsciiGetTextLine(ascii->line_ptr[line], ascii->line_ptr[line + 1]);

				if (line_buffer != NULL)
				{
					if (line == block->start.line)
					{
						if (block->start.offset > (long)ustrlen(line_buffer))
							src = line_buffer + ustrlen(line_buffer);
						else
							src = &line_buffer[block->start.offset];
					} else
					{
						src = line_buffer;
					}
					
					if (line == block->end.line && block->end.offset < (long)ustrlen(line_buffer))
						line_buffer[block->end.offset] = 0;

					len = ustrlen(src);
					ret = write(*handle, src, len);
					if (ret != len)
					{
						HYP_DBG(("Error %s writing file. Abort.", strerror(errno)));
						return FALSE;
					}
				}
								
				if (line != block->end.line || block->end.offset == 0)
					write(*handle, "\n", 1);

				line++;
				g_free(line_buffer);
			}
		}
		break;
		
	case BLK_PRINT:
		/* TODO */
		break;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

hyp_filetype AsciiLoad(DOCUMENT *doc, int handle)
{
	ssize_t ret, file_len;
	FMT_ASCII *ascii;

	/* determine file size */
	if (lseek(handle, 0, SEEK_END) != 0)
		return HYP_FT_LOADERROR;
	file_len = lseek(handle, 0, SEEK_CUR);
	lseek(handle, 0, SEEK_SET);
	if (file_len < 0)
		return HYP_FT_LOADERROR;

	/* allocate memory for the whole file */
	ascii = (FMT_ASCII *)g_malloc(sizeof(FMT_ASCII) + file_len);
	if (ascii)
	{
		ascii->length = file_len;
		ascii->line_ptr = NULL;
		ascii->line_height = font_ch;
		ascii->char_width = font_cw;
		
		/* load file into memory */
		ret = read(handle, ascii->start, file_len);
		if (ret != file_len)
		{
			g_free(ascii);
			FileError(hyp_basename(doc->path), _("while reading"));
		} else
		{
			unsigned char *start = ascii->start;
			unsigned char *end = start + file_len;
			unsigned char *ptr = start;
			unsigned char val;
			long columns = 0;
			hyp_filetype type = HYP_FT_ASCII;

			/* init lines and columns */
			doc->lines = 1;
			doc->columns = 0;
			*end = 0;

			/*
			 * this loop determines wether file is really ASCII text
			 * or wether it contain null bytes.
			 * Also determine number of lines and columns
			 */
			while (ptr < end)
			{
				val = *ptr;

				if (columns >= gl_profile.viewer.ascii_break_len)
				{
					unsigned char *old_ptr = ptr;

					doc->lines++;		/* count lines */

					/* search for beginning of word */
					while (columns)
					{
						ptr--;
						columns--;
						if ((*ptr == ' ') || (*ptr == '\t'))
						{
							break;
						}
					}

					if (columns)
					{
						*ptr++ = '\n';	/* insert line break */
						doc->columns = max(doc->columns, columns);
					} else
					{
						ptr = old_ptr;
						doc->columns = max(doc->columns, gl_profile.viewer.ascii_break_len);
					}
					columns = 0;
				} else if ((val == 0x0d) || (val == 0x0a))	/* CR or LF? */
				{
					doc->lines++;		/* count lines */
					doc->columns = max(doc->columns, columns);
					columns = 0;
					ptr++;				/* skip line ending */
					if (val == 0x0d && *ptr == 0x0a)
						ptr++;
				} else if (val == '\t')	/* tab-stop?... */
				{
					columns += gl_profile.viewer.ascii_tab_size - columns % gl_profile.viewer.ascii_tab_size;
					ptr++;				/* skip tab */
				} else if (val)
				{
					ptr++;				/* skip regular character */
					columns++;
				} else
				{
					/* ... it is a binary file */
					doc->lines = (file_len + gl_profile.viewer.binary_columns) / gl_profile.viewer.binary_columns;
					doc->columns = gl_profile.viewer.binary_columns;
					columns = 0;
					type = HYP_FT_BINARY;
					break;
				}
			}

			doc->height = doc->lines * ascii->line_height;
			doc->data = ascii;
			doc->start_line = 0;
			doc->type = type;
			doc->closeProc = AsciiClose;
			doc->gotoNodeProc = AsciiGotoNode;
			doc->getNodeProc = AsciiGetNode;
			doc->prepNode = AsciiPrep;
			
			/* is it an ASCII file? */
			if (type == HYP_FT_ASCII)
			{
				long line = 0;

				doc->columns = max(doc->columns, columns);
				ptr = (unsigned char *) &ascii->start;

				/* allocate table of lines */
				ascii->line_ptr = g_new(unsigned char *, doc->lines + 2);
				if (ascii->line_ptr == NULL)
				{
					g_free(ascii);
					return HYP_FT_LOADERROR;
				}

				ascii->line_ptr[line++] = ptr;
				/*
				 * convert all CR/LF and store line pointers
				 */
				ptr = start;
				columns = 0;
				while (ptr < end)
				{
					val = *ptr;

					if (columns >= gl_profile.viewer.ascii_break_len)
					{
						columns = 0;
						ascii->line_ptr[line++] = ptr;
					} else if ((val == 0x0d) || (val == 0x0a))	/* CR or LF? */
					{
						columns = 0;
						*ptr++ = 0;
						if ((val == 0x0d) && (*ptr == 0x0a))
							ptr++;
						ascii->line_ptr[line++] = ptr;
					} else if (val == '\t')	/* tab-stop?... */
					{
						columns += gl_profile.viewer.ascii_tab_size - columns % gl_profile.viewer.ascii_tab_size;
						ptr++;			/* skip tab */
					} else
					{
						ptr++;			/* skip regular character */
						columns++;
					}
				}
				ascii->line_ptr[line] = end;

				doc->displayProc = AsciiDisplayPage;
				doc->autolocProc = AsciiAutolocator;
				doc->getCursorProc = AsciiGetCursorPosition;
				doc->blockProc = AsciiBlockOperations;

				/* indicate that ASCII Export is supported */
				doc->buttons.save = TRUE;
			} else
			{
				doc->displayProc = BinaryDisplayPage;
				doc->autolocProc = BinaryAutolocator;
				doc->getCursorProc = BinaryGetCursorPosition;
				doc->blockProc = BinaryBlockOperations;
			}

			return type;
		}
	} else
	{
		FileErrorErrno(hyp_basename(doc->path));
	}
	return HYP_FT_LOADERROR;
}
