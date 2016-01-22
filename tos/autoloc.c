#include "hv_defs.h"
#include "hypdebug.h"


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

LINEPTR *HypGetYLine(HYP_NODE *node, long y)
{
	int i;
	LINEPTR *line_ptr;
	long y1 = 0, y2;

	if (node == NULL)
		return NULL;
	line_ptr = node->line_ptr;
	for (i = 0; i < node->lines; i++)
	{
		y1 += line_ptr->y;
		y2 = y1 + line_ptr->h;
		if (y >= y1 && y < y2)
			break;
		y1 = y2;
		line_ptr++;
	}

	if (i == node->lines)
		return NULL;
	return line_ptr;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Return the real Y value for a line
 * You can not only make "line * win->y_raster", because the picture have also
 * lines. The first line have in his "y" value a offset, otherwise the are
 * zero.
 */
long HypGetLineY(HYP_NODE *node, long line)
{
	long i, sy;
	LINEPTR *line_ptr;

	if (node == NULL)
		return 0;
	line_ptr = node->line_ptr;
	if (line_ptr == NULL || line < 0)
		return 0;
	i = sy = 0;
	while (i < line && i < node->lines)
	{
		sy += line_ptr->y + line_ptr->h;
		line_ptr++;
		i++;
	}
	return sy;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Return the real Textline
 * It is the same Problem like above. If you get a line you can't use this
 * direct to get for example the "line_ptr[line].txt".
 * You must be convert the value "line" before you can use them.
 */
long HypGetRealTextLine(HYP_NODE *node, long y)
{
	long i, sy;
	LINEPTR *line_ptr;

	if (node == NULL)
		return 0;
	line_ptr = node->line_ptr;
	if (line_ptr == NULL)
		return 0;
	i = sy = 0;
	while (y > sy && i < node->lines)
	{
		sy += line_ptr->y + line_ptr->h;
		line_ptr++;
		i++;
	}
	return i;
}

/*** ---------------------------------------------------------------------- ***/

char *HypGetTextLine(DOCUMENT *doc, HYP_NODE *node, long line)
{
	HYP_DOCUMENT *hyp;
	const unsigned char *src;
	char *dst, *ret;
	size_t len;
	
	if (doc == NULL || (hyp = doc->data) == NULL || node == NULL || node->line_ptr == NULL || line < 0 || line >= node->lines)
		return NULL;
	src = node->line_ptr[line].txt;

	if (src == NULL)
		return NULL;

	len = 0;
	while (*src)
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
	src = node->line_ptr[line].txt;
	
	while (*src)
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

long HypAutolocator(DOCUMENT *doc, long line, const char *search)
{
	WINDOW_DATA *win = doc->window;
	const char *src;
	long y;
	HYP_NODE *node;
	char *temp;
	size_t len;
	HYP_DOCUMENT *hyp;

	hyp = (HYP_DOCUMENT *) doc->data;
	if (hyp == NULL)
		return -1;
	node = doc->displayed_node;
	
	if (node == NULL)						/* no node loaded */
		return -1;

	if (empty(search))
		return -1;
	
	len = strlen(search);

	y = line * win->y_raster;
	line = HypGetRealTextLine(node, y);

	if (doc->autolocator_dir > 0)
	{
		while (line < node->lines)
		{
			temp = HypGetTextLine(doc, node, line);
			if (temp != NULL)
			{
				src = temp;
				while (*src)
				{
					if (g_utf8_strncasecmp(src, search, len) == 0)
					{
						y = HypGetLineY(node, line);
						line = y / win->y_raster;	/* get real llne of window */
						g_free(temp);
						return line;
					}
					src++;
				}
				g_free(temp);
			}
			line++;
		}
	} else
	{
		while (line > 0)
		{
			temp = HypGetTextLine(doc, node, line);
			if (temp != NULL)
			{
				src = temp;
				while (*src)
				{
					if (g_utf8_strncasecmp(src, search, len) == 0)
					{
						y = HypGetLineY(node, line);
						line = y / win->y_raster;	/* get real line of window */
						g_free(temp);
						return line;
					}
					src++;
				}
				g_free(temp);
			}
			line--;
		}
	}
	
	return -1;
}
