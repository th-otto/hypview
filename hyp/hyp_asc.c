#include "hypdoc.h"
#include "hv_ascii.h"
#include "hypdebug.h"

#ifdef __PUREC__
#include "hv_defs.h" /* Pure-C does not like anonymous struct _window_data_ */
#endif


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

static gboolean AsciiGotoNode(WINDOW_DATA *win, const char *chapter, hyp_nodenr node)
{
	DOCUMENT *doc = hypwin_doc(win);
	UNUSED(chapter);
	UNUSED(node);
	HYP_DBG(("AsciiGotoNode(Chapter: <%s> / <%u>)", printnull(chapter), node));
	doc->prepNode(win, NULL);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static hyp_nodenr AsciiGetNode(WINDOW_DATA *win)
{
	UNUSED(win);
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
		} else if (val != 0 && val != 0x0d && val != 0x0a)
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
		} else if (val != 0 && val != 0x0d && val != 0x0a)
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

long AsciiAutolocator(WINDOW_DATA *win, long line, const char *search, gboolean casesensitive, gboolean wordonly)
{
	DOCUMENT *doc = hypwin_doc(win);
	FMT_ASCII *ascii = (FMT_ASCII *) doc->data;
	const unsigned char *src;
	int ret;
	
	long len = strlen(search);

	if (!ascii)							/* no file loaded */
		return -1;

	UNUSED(wordonly); /* TODO */
	
	if (doc->autolocator_dir > 0)
	{
		while (line < ascii->lines)
		{
			src = ascii->line_ptr[line];
			if (src)
			{
				while (*src)
				{
					ret = casesensitive ? strncmp((const char *)src, search, len) : strncasecmp((const char *)src, search, len);
					if (ret == 0)
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
					ret = casesensitive ? strncmp((const char *)src, search, len) : strncasecmp((const char *)src, search, len);
					if (ret == 0)
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

gboolean AsciiBlockOperations(WINDOW_DATA *win, hyp_blockop op, BLOCK *block, void *param)
{
	DOCUMENT *doc = hypwin_doc(win);
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
			ssize_t ret;
			ssize_t len;
			unsigned char *line_buffer;
			const unsigned char *src;
			int *handle = (int *) param;

			line = block->start.line;

			while ((line < ascii->lines) && (line <= block->end.line))
			{
				line_buffer = AsciiGetTextLine(ascii->line_ptr[line], ascii->line_ptr[line + 1]);

				if (line_buffer != NULL)
				{
					len = ustrlen(line_buffer);
					if (line == block->end.line && block->end.offset < len)
					{
						line_buffer[block->end.offset] = 0;
						len = block->end.offset;
					}
					
					if (line == block->start.line)
					{
						if (block->start.offset > len)
						{
							src = line_buffer + len;
							len = 0;
						} else
						{
							src = &line_buffer[block->start.offset];
							len -= block->start.offset;
						}
					} else
					{
						src = line_buffer;
					}
					
					ret = write(*handle, src, len);
					g_free(line_buffer);
					if (ret != len)
					{
						HYP_DBG(("Error %s writing file. Abort.", strerror(errno)));
						return FALSE;
					}
				}
								
				if (line != block->end.line || block->end.offset == 0)
					write(*handle, "\n", 1);

				line++;
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

hyp_filetype AsciiCalcLines(DOCUMENT *doc, FMT_ASCII *ascii)
{
	unsigned char *start = ascii->start;
	unsigned char *end = start + ascii->length;
	unsigned char *ptr;
	unsigned char val;
	long columns = 0;
	long wordend_columns = 0;
	long line = 0;
	unsigned char *wordend = NULL;
	hyp_filetype type = HYP_FT_ASCII;

	/* init lines and columns */
	ascii->line_ptr = NULL;
	ascii->charset = hyp_get_current_charset();
	ascii->lines = 0;
	ascii->columns = 0;
	*end = 0;

	if (memchr(start, 0, ascii->length))
	{
		type = HYP_FT_BINARY;
	} else
	{
		if (ascii->charset == HYP_CHARSET_UTF8 && !g_utf8_validate((const char *)start, ascii->length, NULL))
			ascii->charset = HYP_CHARSET_BINARY_TABS;
		if (ascii->charset == HYP_CHARSET_BINARY_TABS || ascii->charset == hyp_get_current_charset())
		{
			/*
			 * Heuristic to check for Atari character set.
			 * 0x81 is not defined in iso-8859-1.
			 * The others are defined, but rather uncommon in ascii text files
			 */
			if (memchr(start, 0x81, ascii->length) ||
				memchr(start, 0x84, ascii->length) ||
				memchr(start, 0x90, ascii->length) ||
				memchr(start, 0x94, ascii->length))
			{
				ascii->charset = HYP_CHARSET_ATARI;
			}
		}
		
		/* allocate table of lines */
		ascii->line_ptr = g_new(unsigned char *, line + 2);
		if (ascii->line_ptr == NULL)
		{
			g_free(ascii);
			return HYP_FT_LOADERROR;
		}

		/*
		 * this loop determines wether file is really ASCII text
		 * or wether it contain null bytes.
		 * Also determine number of lines and columns
		 */
		ptr = start;
		ascii->line_ptr[line] = ptr;
		while (ptr < end)
		{
			val = *ptr;

			if (columns >= gl_profile.viewer.ascii_break_len)
			{
				if (wordend)
				{
					ptr = wordend;
					ptr++;	/* insert line break */
					ascii->columns = max(ascii->columns, wordend_columns);
				} else
				{
					ascii->columns = max(ascii->columns, gl_profile.viewer.ascii_break_len);
				}
				columns = 0;
				wordend_columns = 0;
				wordend = NULL;
				ascii->line_ptr = g_renew(unsigned char *, ascii->line_ptr, line + 2);
				if (ascii->line_ptr == NULL)
					break;
				ascii->line_ptr[++line] = ptr;
			} else if (val == 0x0d || val == 0x0a)	/* CR or LF? */
			{
				ascii->columns = max(ascii->columns, columns);
				columns = 0;
				wordend_columns = 0;
				wordend = NULL;
				ptr++;				/* skip line ending */
				if (val == 0x0d && *ptr == 0x0a)
					ptr++;
				ascii->line_ptr = g_renew(unsigned char *, ascii->line_ptr, line + 2);
				if (ascii->line_ptr == NULL)
					break;
				ascii->line_ptr[++line] = ptr;
			} else if (val == '\t')	/* tab-stop?... */
			{
				if (!wordend)
					wordend_columns = columns;
				wordend = ptr;
				columns += gl_profile.viewer.ascii_tab_size - columns % gl_profile.viewer.ascii_tab_size;
				ptr++;				/* skip tab */
			} else if (val == ' ')
			{
				if (!wordend)
					wordend_columns = columns;
				wordend = ptr;
				ptr++;				/* skip regular character */
				columns++;
			} else if (val)
			{
				if (ascii->charset == HYP_CHARSET_UTF8)
				{
					ptr = g_utf8_next_char(ptr);
				} else
				{
					ptr++;				/* skip regular character */
				}
				columns++;
			} else
			{
				/* ... it is a binary file */
				columns = 0;
				type = HYP_FT_BINARY;
				break;
			}
		}
	}
	
	if (type == HYP_FT_ASCII)
	{
		if (columns != 0)
		{
			line++;
			ascii->columns = max(ascii->columns, columns);
		}

		ascii->line_ptr = g_renew(unsigned char *, ascii->line_ptr, line + 1);
		if (ascii->line_ptr == NULL)
		{
			g_free(ascii);
			return HYP_FT_LOADERROR;
		}
		
		ascii->line_ptr[line] = ptr;
		ascii->lines = line;

		doc->displayProc = AsciiDisplayPage;
		doc->autolocProc = AsciiAutolocator;
		doc->getCursorProc = AsciiGetCursorPosition;
		doc->blockProc = AsciiBlockOperations;

		/* indicate that ASCII Export is supported */
		doc->buttons.save = TRUE;
	} else
	{
		ascii->charset = HYP_CHARSET_BINARY;
		ascii->lines = (ascii->length + gl_profile.viewer.binary_columns - 1) / gl_profile.viewer.binary_columns;
		ascii->columns = gl_profile.viewer.binary_columns;
		doc->displayProc = BinaryDisplayPage;
		doc->autolocProc = BinaryAutolocator;
		doc->getCursorProc = BinaryGetCursorPosition;
		doc->blockProc = BinaryBlockOperations;
	}

	doc->data = ascii;
	doc->start_line = 0;
	doc->type = type;
	doc->closeProc = AsciiClose;
	doc->gotoNodeProc = AsciiGotoNode;
	doc->getNodeProc = AsciiGetNode;
	doc->prepNode = AsciiPrep;
	
	return type;
}

/*** ---------------------------------------------------------------------- ***/

hyp_filetype AsciiLoad(DOCUMENT *doc, int handle)
{
	ssize_t ret, file_len;
	FMT_ASCII *ascii;

	/* determine file size */
	lseek(handle, 0, SEEK_END);
	file_len = lseek(handle, 0, SEEK_CUR);
	lseek(handle, 0, SEEK_SET);
	if (file_len < 0)
		return HYP_FT_LOADERROR;

	/* allocate memory for the whole file */
	ascii = (FMT_ASCII *)g_malloc(sizeof(FMT_ASCII) + file_len);
	if (ascii)
	{
		ascii->length = file_len;
		
		/* load file into memory */
		ret = read(handle, ascii->start, file_len);
		if (ret != file_len)
		{
			g_free(ascii);
			FileError(hyp_basename(doc->path), _("while reading"));
		} else
		{
			return AsciiCalcLines(doc, ascii);
		}
	} else
	{
		FileErrorErrno(hyp_basename(doc->path));
	}
	return HYP_FT_LOADERROR;
}
