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

#if 0 /* for debugging */
static void print_text_and_tabs(WINDOW_DATA *win, GtkTextIter *start, GtkTextIter *end, PangoTabArray *tab_array)
{
	char *line;
	int i, j, size;
	int x;
	int tabnum;
	
	line = gtk_text_buffer_get_text(win->text_buffer, start, end, FALSE);
	size = pango_tab_array_get_size(tab_array);
	x = 0;
	i = 0;
	tabnum = 0;
	for (i = 0; line[i]; i++)
	{
		if (line[i] == '\t')
		{
			int pos;
			pango_tab_array_get_tab(tab_array, tabnum, NULL, &pos);
			tabnum++;
			pos /= win->x_raster;
			for (j = x; j < pos; j++)
				putchar(' ');
			x = pos;
		} else
		{
			putchar(line[i]);
		}
		x++;
	}
	printf("\n");
	x = 0;
	for (i = 0; i < size; i++)
	{
		int pos;
		pango_tab_array_get_tab(tab_array, i, NULL, &pos);
		pos /= win->x_raster;
		for (j = x; j < pos; j++)
			putchar(' ');
		putchar('^');
		x = pos + 1;
	}
	printf("\n");
	g_free(line);
}
#endif

/*** ---------------------------------------------------------------------- ***/

struct prep_info {
	PangoTabArray *tab_array;
	int tab_array_size;
	GtkTextMark *linestart;
	GtkTextMark *tagstart;
	GtkTextMark *attrstart;
	GtkTextBuffer *text_buffer;
	GtkTextTagTable *tag_table;
	int last_was_space;
	int tab_id;
	int target_link_id;
	WP_UNIT x;
	WP_UNIT x_raster;
	GtkTextIter iter;
	unsigned char textattr;
};

static GtkTextTag *insert_str(struct prep_info *info, const char *str, const char *tag)
{
	GtkTextTag *target_tag = NULL;
	
	if (tag)
	{
		if (info->last_was_space > 1)
		{
			gtk_text_buffer_insert(info->text_buffer, &info->iter, "\t", 1);
			pango_tab_array_set_tab(info->tab_array, info->tab_array_size, PANGO_TAB_LEFT, info->x * info->x_raster);
			info->tab_array_size++;
		} else if (info->last_was_space == 1)
		{
			gtk_text_buffer_insert(info->text_buffer, &info->iter, " ", 1);
		}
		gtk_text_buffer_move_mark(info->text_buffer, info->tagstart, &info->iter);
		info->last_was_space = 0;
	}
		
	if (gl_profile.viewer.expand_spaces)
	{
		const char *scan = str;
		const char *next;
		while (*scan)
		{
			next = g_utf8_skipchar(scan);
			if (info->last_was_space)
			{
				if (*scan != ' ' || *scan == '\t')
				{
					if (info->last_was_space > 1)
					{
						gtk_text_buffer_insert(info->text_buffer, &info->iter, "\t", 1);
						pango_tab_array_set_tab(info->tab_array, info->tab_array_size, PANGO_TAB_LEFT, info->x * info->x_raster);
						info->tab_array_size++;
					} else if (info->last_was_space == 1)
					{
						gtk_text_buffer_insert(info->text_buffer, &info->iter, " ", 1);
					}
					gtk_text_buffer_insert(info->text_buffer, &info->iter, scan, next - scan);
					info->last_was_space = 0;
				} else
				{
					info->last_was_space++;
				}
			} else if (*scan == ' ' || *scan == '\t')
			{
				info->last_was_space++;
			} else
			{
				gtk_text_buffer_insert(info->text_buffer, &info->iter, scan, next - scan);
				info->last_was_space = 0;
			}
			scan = next;
			info->x++;
		}
	} else
	{ 
		gtk_text_buffer_insert(info->text_buffer, &info->iter, str, -1);
	}
	
	if (tag)
	{
		GtkTextIter tagstart_iter, tagend_iter;
		char *target_name;
		
		gtk_text_buffer_get_iter_at_mark(info->text_buffer, &tagstart_iter, info->tagstart);
		tagend_iter = info->iter;
		gtk_text_buffer_apply_tag_by_name(info->text_buffer, tag, &tagstart_iter, &tagend_iter);
		target_name = g_strdup_printf("hv-link-%d", info->target_link_id);
		target_tag = gtk_text_buffer_create_tag(info->text_buffer, target_name, NULL);
		gtk_text_buffer_apply_tag(info->text_buffer, target_tag, &tagstart_iter, &tagend_iter);
		info->target_link_id++;
		g_free(target_name);
	}
	
	if (info->textattr)
	{
		GtkTextIter tagstart_iter, tagend_iter;
		
		gtk_text_buffer_get_iter_at_mark(info->text_buffer, &tagstart_iter, info->attrstart);
		tagend_iter = info->iter;
		if (info->textattr & HYP_TXT_BOLD)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bold", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_LIGHT)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "ghosted", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_ITALIC)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "italic", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_UNDERLINED)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "underlined", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_SHADOWED)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "shadowed", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_OUTLINED)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "outlined", &tagstart_iter, &tagend_iter);
	}
	
	return target_tag;
}

/*** ---------------------------------------------------------------------- ***/

void HypPrepNode(DOCUMENT *doc)
{
	HYP_NODE *node = doc->displayed_node;
	WINDOW_DATA *win = doc->window;
	HYP_DOCUMENT *hyp = doc->data;
	const unsigned char *src, *end, *textstart;
	long lineno;
	WP_UNIT sx, sy;
	gboolean at_bol;
	struct prep_info info;
	
#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		char *s; \
		size_t len = src - textstart; \
		/* draw remaining text */ \
		s = hyp_conv_charset(hyp->comp_os, HYP_CHARSET_UTF8, textstart, len, NULL); \
		insert_str(&info, s, NULL); \
		g_free(s); \
		at_bol = FALSE; \
	}

	ToolbarUpdate(doc, FALSE);
	
	end = node->end;
	src = node->start;

	info.text_buffer = win->text_buffer;
	info.x_raster = win->x_raster;
	
	/*
	 * clear buffer
	 */
	gtk_text_buffer_set_text(info.text_buffer, "", 0);
	/*
	 * remove old tabulator tags
	 */
	info.tag_table = gtk_text_buffer_get_tag_table(info.text_buffer);
	info.tab_id = 0;
	for (;;)
	{
		char *tag_name = g_strdup_printf("hv-tabtag-%d", info.tab_id);
		GtkTextTag *tag = gtk_text_tag_table_lookup(info.tag_table, tag_name);
		g_free(tag_name);
		if (tag == 0)
			break;
		gtk_text_tag_table_remove(info.tag_table, tag);
		info.tab_id++;
	}
	info.tab_id = 0;

	/*
	 * remove old target links
	 */
	info.target_link_id = 0;
	for (;;)
	{
		char *tag_name = g_strdup_printf("hv-link-%d", info.target_link_id);
		GtkTextTag *tag = gtk_text_tag_table_lookup(info.tag_table, tag_name);
		g_free(tag_name);
		if (tag == 0)
			break;
		gtk_text_tag_table_remove(info.tag_table, tag);
		info.target_link_id++;
	}
	info.target_link_id = 0;
	
	gtk_text_buffer_get_iter_at_offset(info.text_buffer, &info.iter, 0);
	info.linestart = gtk_text_buffer_get_mark(info.text_buffer, "hv-linestart");
	if (info.linestart == NULL)
		info.linestart = gtk_text_buffer_create_mark(info.text_buffer, "hv-linestart", &info.iter, TRUE);
	info.tagstart = gtk_text_buffer_get_mark(info.text_buffer, "hv-tagstart");
	if (info.tagstart == NULL)
		info.tagstart = gtk_text_buffer_create_mark(info.text_buffer, "hv-tagstart", &info.iter, TRUE);
	gtk_text_buffer_move_mark(info.text_buffer, info.linestart, &info.iter);
	info.attrstart = gtk_text_buffer_get_mark(info.text_buffer, "hv-attrstart");
	if (info.attrstart == NULL)
		info.attrstart = gtk_text_buffer_create_mark(info.text_buffer, "hv-attrstart", &info.iter, TRUE);
	gtk_text_buffer_move_mark(info.text_buffer, info.attrstart, &info.iter);
	
	textstart = src;
	info.textattr = 0;
	lineno = 0;
	info.x = sx = sy = 0;
	at_bol = TRUE;
	info.last_was_space = 0;
	
	info.tab_array_size = 0;
	info.tab_array = pango_tab_array_new(info.tab_array_size, TRUE);
	
	sy = draw_graphics(win, node->gfx, lineno, sx, sy);
	
	while (src < end)
	{
		if (*src == HYP_ESC)		/* ESC-sequence */
		{
			/* unwritten data? */
			DUMPTEXT();
			src++;
			
			gtk_text_buffer_move_mark(info.text_buffer, info.attrstart, &info.iter);

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
					unsigned char link_type = *src;	/* remember link type */
					hyp_lineno line_nr = 0;	/* line number to go to */
					hyp_nodenr dest_page;	/* Index of destination page */
					char *str;
					size_t len;
					unsigned short link_len;
					const char *tagtype = "link";
					hyp_indextype dst_type = HYP_NODE_EOF;
					char *tip = NULL;
					LINK_INFO *linkinfo;
					GtkTextTag *target_tag;
					
					src++;
					
					if (link_type == HYP_ESC_LINK_LINE || link_type == HYP_ESC_ALINK_LINE)	/* skip destination line number */
					{
						line_nr = DEC_255(src);
						src += 2;
					}
					
					dest_page = DEC_255(src);
					src += 2;

					link_len = *src;
					src++;
					
					/* get link text for output */
					if (link_len <= HYP_STRLEN_OFFSET)	/* no text in link: use nodename */
					{
						if (hypnode_valid(hyp, dest_page))
						{
							str = pagename(hyp, dest_page);
						} else
						{
							str = invalid_page(dest_page);
						}
						len = strlen(str);
					} else
					{
						len = link_len - HYP_STRLEN_OFFSET;
						str = hyp_conv_charset(hyp->comp_charset, HYP_CHARSET_UTF8, src, len, NULL);
						src += len;
					}

					if (hypnode_valid(hyp, dest_page))
					{
						dst_type = hyp->indextable[dest_page]->type;
						tip = pagename(hyp, dest_page);
						switch (dst_type)
						{
						case HYP_NODE_INTERNAL:
							tagtype = "link";
							break;
						case HYP_NODE_POPUP:
							tagtype = "popup";
							break;
						case HYP_NODE_EXTERNAL_REF:
							tagtype = "xref";
							break;
						case HYP_NODE_REXX_COMMAND:
						case HYP_NODE_REXX_SCRIPT:
							tagtype = "rexx";
							break;
						case HYP_NODE_QUIT:
						case HYP_NODE_CLOSE:
							tagtype = "close";
							break;
						case HYP_NODE_SYSTEM_ARGUMENT:
							tagtype = "system";
							break;
						case HYP_NODE_IMAGE:
						case HYP_NODE_EOF:
						default:
							tagtype = "red";
							g_free(tip);
							tip = g_strdup_printf(_("Link to node of type %u not Implemented."), dst_type);
							break;
						}
					} else
					{
						tip = invalid_page(dest_page);
					}
					
					target_tag = insert_str(&info, str, tagtype);
					g_free(str);
					
					linkinfo = g_new(LINK_INFO, 1);
					linkinfo->link_type = link_type;
					linkinfo->dst_type = dst_type;
					linkinfo->tip = tip;
					linkinfo->dest_page = dest_page;
					linkinfo->line_nr = line_nr;
					g_object_set_data(G_OBJECT(target_tag), "hv-linkinfo", linkinfo);
					
					textstart = src;
				}
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				info.textattr = *src - HYP_ESC_TEXTATTR_FIRST;
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
			if (info.tab_array_size > 0)
			{
				char *tag_name = g_strdup_printf("hv-tabtag-%d", info.tab_id);
				GtkTextIter start;
				GtkTextTag *tag = gtk_text_tag_new(tag_name);
				
				g_free(tag_name);
				info.tab_id++;
				g_object_set(G_OBJECT(tag), "tabs", info.tab_array, NULL);
				gtk_text_tag_table_add(info.tag_table, tag);
				gtk_text_buffer_get_iter_at_mark(info.text_buffer, &start, info.linestart);
				
				/* print_text_and_tabs(win, &start, &info.iter, info.tab_array); */
				
				gtk_text_buffer_apply_tag(info.text_buffer, tag, &start, &info.iter);
				info.tab_array_size = 0;
				info.tab_array = pango_tab_array_new(info.tab_array_size, TRUE);
			}
			++lineno;
			src++;
			textstart = src;
			info.last_was_space = 0;
			info.x = sx;
			at_bol = TRUE;
			gtk_text_buffer_insert(info.text_buffer, &info.iter, "\n", 1);
			gtk_text_buffer_move_mark(info.text_buffer, info.linestart, &info.iter);
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
		if (info.tab_array_size > 0)
		{
			char *tag_name = g_strdup_printf("hv-tabtag-%d", info.tab_id);
			GtkTextIter start;
			GtkTextTag *tag = gtk_text_tag_new(tag_name);
			
			g_free(tag_name);
			info.tab_id++;
			g_object_set(G_OBJECT(tag), "tabs", info.tab_array, NULL);
			gtk_text_tag_table_add(info.tag_table, tag);
			gtk_text_buffer_get_iter_at_mark(info.text_buffer, &start, info.linestart);
			gtk_text_buffer_apply_tag(info.text_buffer, tag, &start, &info.iter);
			info.tab_array_size = 0;
			info.tab_array = NULL;
		}
		++lineno;
		sy += win->y_raster;
		sy = draw_graphics(win, node->gfx, lineno, sx, sy);
	}
	if (info.tab_array)
		pango_tab_array_free(info.tab_array);
}
