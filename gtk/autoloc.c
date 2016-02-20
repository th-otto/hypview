#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *HypGetTextLine(WINDOW_DATA *win, HYP_NODE *node, long line)
{
	DOCUMENT *doc;
	GtkTextIter start, end;
	char *txt;
	
	if (win == NULL || (doc = win->data) == NULL || doc->data == NULL || node == NULL)
		return NULL;

	gtk_text_buffer_get_iter_at_line(win->text_buffer, &start, line);
	if (gtk_text_iter_is_end(&start))
		return NULL;
	end = start;
	if (!gtk_text_iter_forward_line(&end))
		return NULL;
	txt = gtk_text_buffer_get_text(win->text_buffer, &start, &end, FALSE);
	if (txt && *txt)
	{
		char *end = txt + strlen(txt) - 1;
		if (*end == '\n')
			*end = '\0';
	}
	return txt;
}

/*** ---------------------------------------------------------------------- ***/

long HypAutolocator(WINDOW_DATA *win, long line, const char *search, gboolean casesensitive, gboolean wordonly)
{
	DOCUMENT *doc = hypwin_doc(win);
	HYP_NODE *node;
	char *temp;
	HYP_DOCUMENT *hyp;
	const char *res;
	
	hyp = (HYP_DOCUMENT *) doc->data;
	if (hyp == NULL)
		return -1;
	node = win->displayed_node;
	
	if (node == NULL)						/* no node loaded */
		return -1;

	if (empty(search))
		return -1;
	
	UNUSED(wordonly); /* TODO */
	
	if (doc->autolocator_dir > 0)
	{
		while ((temp = HypGetTextLine(win, node, line)) != NULL)
		{
			res = casesensitive ? strstr(temp, search) : g_utf8_strcasestr(temp, search);
			if (res != NULL)
			{
				g_free(temp);
				return line;
			}
			g_free(temp);
			line++;
		}
	} else
	{
		while (line > 0)
		{
			temp = HypGetTextLine(win, node, line);
			if (temp != NULL)
			{
				res = casesensitive ? strstr(temp, search) : g_utf8_strcasestr(temp, search);
				if (res != NULL)
				{
					g_free(temp);
					return line;
				}
				g_free(temp);
			}
			line--;
		}
	}
	
	return -1;
}
