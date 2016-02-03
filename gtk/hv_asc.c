#include "hv_gtk.h"
#include "hv_ascii.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void AsciiDisplayPage(WINDOW_DATA *win)
{
	UNUSED(win);
	/* nothing to do */
}

/*** ---------------------------------------------------------------------- ***/

void AsciiPrep(WINDOW_DATA *win, HYP_NODE *node)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	long line;
	GtkTextBuffer *text_buffer = win->text_buffer;
	GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(text_buffer);
	int tab_id;
	GtkTextIter iter;
	
	UNUSED(node);
	/*
	 * clear buffer
	 */
	gtk_text_buffer_set_text(text_buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset(text_buffer, &iter, 0);

	/*
	 * remove old tabulator tags
	 */
	tab_id = 0;
	for (;;)
	{
		char *tag_name = g_strdup_printf("hv-tabtag-%d", tab_id);
		GtkTextTag *tag = gtk_text_tag_table_lookup(tag_table, tag_name);
		g_free(tag_name);
		if (tag == 0)
			break;
		gtk_text_tag_table_remove(tag_table, tag);
		tab_id++;
	}
	tab_id = 0;

	if (doc->type == HYP_FT_ASCII)
	{
		for (line = 0; line < ascii->lines; line++)
		{
			const unsigned char *src = ascii->line_ptr[line];
			size_t n = ascii->line_ptr[line + 1] - ascii->line_ptr[line];
			char *txt;
			
			if (n > 0 && src[n - 1] == 0x0a)
				n--;
			if (n > 0 && src[n - 1] == 0x0d)
				n--;
			txt = hyp_conv_to_utf8(ascii->charset, src, n);
			gtk_text_buffer_insert(text_buffer, &iter, txt, -1);
			g_free(txt);
			gtk_text_buffer_insert(text_buffer, &iter, "\n", 1);
		}
	} else
	{
		const unsigned char *src = ascii->start;
		const unsigned char *end = src + ascii->length;
		
		while (src < end)
		{
			int n = (int)min(gl_profile.viewer.binary_columns, end - src);
			char *txt = hyp_conv_to_utf8(HYP_CHARSET_BINARY, src, n);
			gtk_text_buffer_insert(text_buffer, &iter, txt, -1);
			g_free(txt);
			gtk_text_buffer_insert(text_buffer, &iter, "\n", 1);
			src += n;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void AsciiGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;
	
	if (doc->type != HYP_FT_ASCII)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

	/* not needed in GTK */
	UNUSED(x);
	UNUSED(y);
	UNUSED(pos);
}
