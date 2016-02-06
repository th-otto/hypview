#include "hv_defs.h"
#include "hypdebug.h"


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *HypGetTextLine(WINDOW_DATA *win, HYP_NODE *node, long line)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp;
	const unsigned char *src;
	const unsigned char *end;
	char *dst, *ret;
	size_t len;
	
	if (doc == NULL || (hyp = doc->data) == NULL || node == NULL || node->line_ptr == NULL || line < 0 || (line * win->y_raster) >= win->docsize.h)
		return NULL;
	src = node->line_ptr[line];
	end = node->line_ptr[line + 1];

	if (src == NULL)
		return NULL;

	len = 0;
	while (src < end && *src)
	{
		if (*src == HYP_ESC)					/* ESC-Sequence ?? */
		{
			src++;
			switch (*src)
			{
			case HYP_ESC_ESC:					/* ESC */
				len++;
				break;
			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					hyp_nodenr i;	/* index of target page */

					if (*src == HYP_ESC_LINK_LINE || *src == HYP_ESC_ALINK_LINE)	/* skip destination line number */
						src += 2;

					i = DEC_255(&src[1]);
					src += 3;

					/* get and display link text */
					if (*src <= HYP_STRLEN_OFFSET)		/* no text specified in link */
					{
						len += ustrlen(hyp->indextable[i]->name);
						src++;
					} else
					{
						unsigned char num = *src - HYP_STRLEN_OFFSET;

						src += num + 1;
						len += num;
					}
				}
				break;
			default:
				src = hyp_skip_esc(src - 1);
				break;
			}
		} else
		{
			src++;
			len++;
		}
	}
	
	dst = ret = g_new(char, len + 1);
	src = node->line_ptr[line];
	
	while (src < end && *src)
	{
		if (*src == HYP_ESC)					/* ESC-sequence ?? */
		{
			*dst = 0;							/* mark end of buffer */
			src++;
			switch (*src)
			{
			case HYP_ESC_ESC:					/* ESC */
				*dst++ = HYP_ESC_ESC;
				break;
			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					hyp_nodenr i;	/* index of target page */

					if (*src == HYP_ESC_LINK_LINE || *src == HYP_ESC_ALINK_LINE)		/* skip line number */
						src += 2;

					i = DEC_255(&src[1]);
					src += 3;

					/* get and display link text */
					if (*src <= HYP_STRLEN_OFFSET)	/* no text specified in link */
					{
						strcpy(dst, (const char *)hyp->indextable[i]->name);
						src++;
						dst += strlen(dst);
					} else
					{
						unsigned char num = (*src) - HYP_STRLEN_OFFSET;

						memcpy(dst, src + 1, num);
						src += num + 1;
						dst += num;
					}
				}
				break;
			default:
				src = hyp_skip_esc(src - 1);
				break;
			}
		} else
		{
			*dst++ = *src++;
		}
	}
	*dst = 0;							/* mark end of buffer */
	ASSERT(dst == ret + len);
	
	dst = hyp_conv_to_utf8(hyp->comp_charset, ret, len);
	g_free(ret);
	ret = dst;

	return ret;
}

/*** ---------------------------------------------------------------------- ***/

long HypAutolocator(WINDOW_DATA *win, long line, const char *search, gboolean casesensitive, gboolean wordonly)
{
	DOCUMENT *doc = win->data;
	const char *src;
	long y;
	HYP_NODE *node;
	char *temp;
	size_t len;
	HYP_DOCUMENT *hyp;
	int ret;
	
	hyp = (HYP_DOCUMENT *) doc->data;
	if (hyp == NULL)
		return -1;
	node = win->displayed_node;
	
	if (node == NULL)						/* no node loaded */
		return -1;

	if (empty(search))
		return -1;
	
	UNUSED(wordonly); /* TODO */
	
	len = strlen(search);

	y = line * win->y_raster;

	if (doc->autolocator_dir > 0)
	{
		while (y < win->docsize.h)
		{
			temp = HypGetTextLine(win, node, line);
			if (temp != NULL)
			{
				src = temp;
				while (*src)
				{
					ret = casesensitive ? strncmp(src, search, len) : g_utf8_strncasecmp(src, search, len);
					if (ret == 0)
					{
						g_free(temp);
						return line;
					}
					src++;
				}
				g_free(temp);
			}
			line++;
			y += win->y_raster;
		}
	} else
	{
		while (y > 0)
		{
			temp = HypGetTextLine(win, node, line);
			if (temp != NULL)
			{
				src = temp;
				while (*src)
				{
					ret = casesensitive ? strncmp(src, search, len) : g_utf8_strncasecmp(src, search, len);
					if (ret == 0)
					{
						g_free(temp);
						return line;
					}
					src++;
				}
				g_free(temp);
			}
			line--;
			y -= win->y_raster;
		}
	}
	
	return -1;
}
