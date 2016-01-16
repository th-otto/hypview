#include "hv_gtk.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void HypDisplayPage(DOCUMENT *doc)
{
	/* nothing to do */
	UNUSED(doc);
}

/*** ---------------------------------------------------------------------- ***/

static long DrawPicture(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	/* YYY */
	UNUSED(win);
	UNUSED(gfx);
	UNUSED(x);
	return y;
}

/*** ---------------------------------------------------------------------- ***/

static void DrawLine(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	/* YYY */
	UNUSED(win);
	UNUSED(gfx);
	UNUSED(x);
	UNUSED(y);
}

/*** ---------------------------------------------------------------------- ***/

static void DrawBox(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	/* YYY */
	UNUSED(win);
	UNUSED(gfx);
	UNUSED(x);
	UNUSED(y);
}

/*** ---------------------------------------------------------------------- ***/

static char *pagename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_charset(hyp->comp_charset, HYP_CHARSET_UTF8, entry->name, namelen, NULL);
}

/*** ---------------------------------------------------------------------- ***/

static char *invalid_page(hyp_nodenr page)
{
	return g_strdup_printf(_("<invalid destination page %u>"), page);
}

/*** ---------------------------------------------------------------------- ***/

static long draw_graphics(WINDOW_DATA *win, struct hyp_gfx *gfx, long lineno, WP_UNIT sx, WP_UNIT sy)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			switch (gfx->type)
			{
			case HYP_ESC_PIC:
				sy = DrawPicture(win, gfx, sx, sy);
				break;
			case HYP_ESC_LINE:
				DrawLine(win, gfx, sx, sy);
				break;
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				DrawBox(win, gfx, sx, sy);
				break;
			}
		}
		gfx = gfx->next;
	}
	return sy;
}

/*** ---------------------------------------------------------------------- ***/

void HypPrepNode(DOCUMENT *doc)
{
	HYP_NODE *node = doc->displayed_node;
	WINDOW_DATA *win = doc->window;
	HYP_DOCUMENT *hyp = doc->data;
	GtkTextIter iter;
	size_t len;
	const unsigned char *src, *end, *textstart;
	unsigned char textattr;
	long lineno;
	WP_UNIT sx, sy;
	gboolean at_bol;
	char *str;
	
	gtk_text_buffer_set_text(win->text_buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset(win->text_buffer, &iter, 0);
	
#define TEXTOUT(str, tag) \
	{ \
	if (tag) \
		gtk_text_buffer_insert_with_tags_by_name(win->text_buffer, &iter, str, -1, tag, NULL); \
	else \
		gtk_text_buffer_insert(win->text_buffer, &iter, str, -1); \
	}

#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		char *s; \
		len = src - textstart; \
		/* draw remaining text */ \
		s = hyp_conv_charset(hyp->comp_os, HYP_CHARSET_UTF8, textstart, len, NULL); \
		TEXTOUT(s, 0); \
		g_free(s); \
		at_bol = FALSE; \
	}
#define FLUSHLINE() \
	at_bol = TRUE

	end = node->end;
	src = node->start;
	textstart = src;
	textattr = 0;
	lineno = 0;
	sx = sy = 0;
	at_bol = TRUE;
	
	sy = draw_graphics(win, node->gfx, lineno, sx, sy);
	
	while (src < end)
	{
		if (*src == HYP_ESC)		/* ESC-sequence */
		{
			/* unwritten data? */
			DUMPTEXT();
			src++;

			switch (*src)
			{
			case HYP_ESC_ESC:		/* ESC */
				textstart = src;
				src++;
				break;

			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					hyp_nodenr dest_page;	/* Index of destination page */
					
					if (*src == HYP_ESC_LINK_LINE || *src == HYP_ESC_ALINK_LINE)	/* skip destination line number */
						src += 2;

					dest_page = DEC_255(&src[1]);
					src += 3;

					/* get link text for output */
					if (*src <= HYP_STRLEN_OFFSET)	/* no text in link: use nodename */
					{
						if (hypnode_valid(hyp, dest_page))
						{
							str = pagename(hyp, dest_page);
						} else
						{
							str = invalid_page(dest_page);
						}
						len = strlen(str);
						src++;
					} else
					{
						len = *src - HYP_STRLEN_OFFSET;
						src++;
						str = hyp_conv_charset(hyp->comp_charset, HYP_CHARSET_UTF8, src, len, NULL);
						src += len;
					}
					/* vst_color(vdi_handle, gl_profile.viewer.link_color); */
					/* vst_effects(vdi_handle, gl_profile.viewer.link_effect | textattr); */
					TEXTOUT(str, "link");
					g_free(str);

					/* vst_color(vdi_handle, gl_profile.viewer.text_color); */
					/* vst_effects(vdi_handle, textattr); */
					textstart = src;
				}
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				textattr = *src - HYP_ESC_TEXTATTR_FIRST;
				/* vst_effects(vdi_handle, textattr); */
				src++;
				textstart = src;
				break;
			
			case HYP_ESC_WINDOWTITLE:
			case HYP_ESC_CASE_DATA:
			case HYP_ESC_EXTERNAL_REFS:
			case HYP_ESC_OBJTABLE:
			case HYP_ESC_PIC:
			case HYP_ESC_LINE:
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
			default:
				src = hyp_skip_esc(--src);
				textstart = src;
				break;
			}
		} else if (*src == HYP_EOL)
		{
			DUMPTEXT();
			++lineno;
			src++;
			textstart = src;
			at_bol = TRUE;
			gtk_text_buffer_insert(win->text_buffer, &iter, "\n", 1); \
			sy += win->y_raster;
			sy = draw_graphics(win, node->gfx, lineno, sx, sy);
		} else
		{
			src++;
		}
	}
	DUMPTEXT();
	if (!at_bol)
	{
		++lineno;
		sy += win->y_raster;
		sy = draw_graphics(win, node->gfx, lineno, sx, sy);
	}
}
