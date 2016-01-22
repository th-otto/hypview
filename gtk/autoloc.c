#include "hv_gtk.h"
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
	WINDOW_DATA *win;
	GtkTextIter start, end;
	
	if (doc == NULL || doc->data == NULL || (win = doc->window) == NULL || node == NULL || line < 0 || line >= node->lines)
		return NULL;

	gtk_text_buffer_get_iter_at_line(win->text_buffer, &start, line);
	gtk_text_buffer_get_iter_at_line(win->text_buffer, &end, line + 1);
	return gtk_text_buffer_get_text(win->text_buffer, &start, &end, FALSE);
}

/*** ---------------------------------------------------------------------- ***/

long HypAutolocator(DOCUMENT *doc, long line, const char *search)
{
	const char *src;
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
