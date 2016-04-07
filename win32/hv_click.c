#include "hv_defs.h"
#include "hypdebug.h"
#include "w_draw.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void HypClick(WINDOW_DATA *win, LINK_INFO *info)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	POINT p;
	gboolean ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
	
	if (hypnode_valid(hyp, info->dest_page))
	{
		switch (info->dst_type)
		{
		case HYP_NODE_INTERNAL:
			if (ctrl || (info->link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin))
			{
				char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
				OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, FALSE, FORCE_NEW_WINDOW, FALSE);
				g_free(name);
			} else
			{
				AddHistoryEntry(win, doc);
				GotoPage(win, info->dest_page, info->line_nr, TRUE);
			}
			break;
		case HYP_NODE_POPUP:
			GetCursorPos(&p);
			OpenPopup(win, info->dest_page, p.x, p.y);
			break;
		case HYP_NODE_EXTERNAL_REF:
			{
				char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
				HypOpenExtRef(win, name, ctrl || (info->link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin));
				g_free(name);
			}
			break;
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_REXX_SCRIPT:
			{
				char *tool = NULL;
				HYP_HOSTNAME *h;
				const char *argv[3];
				
				/* search for host application */
				if (hyp->hostname == NULL)
				{
					show_message(win ? win->hwnd : NULL, _("Error"), _("No host application defined."), FALSE);
					break;
				}
				for (h = hyp->hostname; h != NULL && tool == NULL; h = h->next)
					if (Profile_ReadString(gl_profile.profile, "TOOLS", h->name, &tool))
						break;
				if (empty(tool))	/* host application found? */
				{
					char *str = g_strdup_printf(_("No application defined to handle %s."), hyp->hostname->name);
					show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
					g_free(str);
					break;		/* ... cancel */
				}

				{
					char *prog = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
					char *dir;
					char *dfn;
					struct stat s;
					
					/* search for file in directory of hypertext */
					dir = hyp_path_get_dirname(hyp->file);
					dfn = g_build_filename(dir, prog, NULL);
					g_free(dir);

					/* can file be located with shel_find()? */
					if (hyp_utf8_stat(dfn, &s) != 0)
					{
						g_free(dfn);
						/* use filename as specified */
						dfn = prog;
						prog = NULL;
					}

					argv[0] = tool;
					argv[1] = dfn;
					argv[2] = 0;
					hyp_utf8_spawnvp(P_NOWAIT, 2, argv);
					g_free(dfn);
					g_free(prog);
					g_free(tool);
				}
			}
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
			{
				char *prog = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
				const char *argv[4];
				int ret;
				
				argv[0] = prog;
				argv[1] = 0;
				ret = hyp_utf8_spawnvp(P_WAIT, 1, argv);
				if (ret < 0)
				{
					char *str = g_strdup_printf(_("can't find %s: %s"), prog, hyp_utf8_strerror(errno));
					show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
					g_free(str);
				}
				g_free(prog);
			}
			break;
		case HYP_NODE_QUIT:	/* quit application */
			{
				GSList *l;
				
				for (l = all_list; l; l = l->next)
					SendCloseWindow((WINDOW_DATA *)l->data);
			}
			break;
		case HYP_NODE_CLOSE:	/* close window */
			SendCloseWindow(win);
			break;
		case HYP_NODE_IMAGE:
		case HYP_NODE_EOF:
		default:
			{
				char *str = g_strdup_printf(_("Link to node of type %u not implemented."), info->dst_type);
				show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
				g_free(str);
			}
			break;
		}
	} else
	{
		HYP_DBG(("Link to invalid node %u", info->dest_page));
	}
}

/*** ---------------------------------------------------------------------- ***/

static long SkipPicture(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	HYP_IMAGE *pic;
	
	pic = (HYP_IMAGE *)AskCache(hyp, gfx->extern_node_index);
	UNUSED(x);
	
	if (pic && pic->decompressed)
	{
		if (gfx->islimage)
		{
			y += ((gfx->pixheight + win->y_raster - 1) / win->y_raster) * win->y_raster;
		}
	}
	return y;
}

/*** ---------------------------------------------------------------------- ***/

static long skip_graphics(WINDOW_DATA *win, struct hyp_gfx *gfx, long lineno, WP_UNIT sx, WP_UNIT sy)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			switch (gfx->type)
			{
			case HYP_ESC_PIC:
				sy = SkipPicture(win, gfx, sx, sy);
				break;
			case HYP_ESC_LINE:
				break;
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				break;
			}
		}
		gfx = gfx->next;
	}
	return sy;
}

/*** ---------------------------------------------------------------------- ***/

static char *pagename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
}

/*** ---------------------------------------------------------------------- ***/

static inline int isword(unsigned char ch)
{
	return isalnum(ch) || ch >= 0x80 || ch == '-';
}

/*** ---------------------------------------------------------------------- ***/

static void get_word(const unsigned char *min, const unsigned char *src, unsigned char *dst)
{
	const unsigned char *ptr = src;

	while (ptr >= min && isword(*ptr))
	{
		ptr--;
	}

	if (ptr != src)
	{
		ptr++;

		do
		{
			*dst++ = *ptr++;
		} while (isword(*ptr));
	}
	*dst = 0;
}

/*** ---------------------------------------------------------------------- ***/

gboolean HypFindLink(WINDOW_DATA *win, int x, int y, LINK_INFO *info, gboolean select_word)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp;
	HYP_NODE *node;
	unsigned textattr = 0;
	char test_mem[2];
	const unsigned char *src, *end;
	const unsigned char *last_esc;
	long lineno;
	WP_UNIT mx, my;
	WP_UNIT sx, sy;
	HDC hdc;
	HFONT oldfont;
	gboolean found = FALSE;
	
	hyp = (HYP_DOCUMENT *)doc->data;

	node = win->displayed_node;

	if (node == NULL)					/* stop if no page loaded */
		return FALSE;
	info->tip = NULL;
	
	WindowCalcScroll(win);

	mx = x - win->scroll.g_x;
	my = y - win->scroll.g_y;
	
	sx = -win->docsize.x;
	sy = -win->docsize.y;

	src = node->start;
	end = node->end;

	hdc = GetDC(win->textwin);
	oldfont = (HFONT)SelectObject(hdc, (HGDIOBJ)win->fonts[HYP_TXT_NORMAL]);

	last_esc = src;
	test_mem[1] = 0;
	lineno = 0;
	x = sx;
	sy = skip_graphics(win, node->gfx, lineno, sx, sy);

#define TEXTOUT(str, len) \
	{ \
		int w, h; \
		wchar_t *wtext = hyp_utf8_to_wchar(str, len, NULL); \
		W_TextExtent(hdc, wtext, &w, &h); \
		g_free(wtext); \
		link_w = w; \
	}

	while (src < end)
	{
		_WORD link_w;
		gboolean target_line = my >= sy && my < (sy + win->y_raster);
		gboolean target_col = FALSE;
		
		if (*src == HYP_ESC)
		{
			src++;
			if (*src == HYP_ESC_ESC)
			{
				*test_mem = HYP_ESC_ESC;
				goto check_char;
			} else if (*src >= HYP_ESC_LINK && *src <= HYP_ESC_ALINK_LINE)
			{
				unsigned char link_type = *src;	/* remember link type */
				hyp_lineno line_nr = 0;	/* line number to go to */
				hyp_nodenr dest_page;	/* index of target node */
				unsigned short link_len;
				char *str;
				hyp_indextype dst_type = HYP_NODE_EOF;
				
				src++;

				if (link_type == HYP_ESC_LINK_LINE || link_type == HYP_ESC_ALINK_LINE)		/* get line number */
				{
					line_nr = DEC_255(src);
					src += 2;
				}

				dest_page = DEC_255(src);
				src += 2;

				SelectObject(hdc, (HGDIOBJ)win->fonts[(gl_profile.colors.link_effect | textattr) & HYP_TXT_MASK]);

				link_len = *src;
				src++;
				
				if (link_len > HYP_STRLEN_OFFSET)
				{
					size_t len = link_len - HYP_STRLEN_OFFSET;
					str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
					src += len;
				} else if (hypnode_valid(hyp, dest_page))
				{
					str = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dest_page]->name, STR0TERM);
				} else
				{
					str = hyp_invalid_page(dest_page);
				}
				TEXTOUT(str, STR0TERM);
				g_free(str);
				
				SelectObject(hdc, (HGDIOBJ)win->fonts[(textattr) & HYP_TXT_MASK]);

				target_col = mx >= x && mx < (x + link_w);
				if (target_line && target_col)
				{
					if (hypnode_valid(hyp, dest_page))
					{
						dst_type = (hyp_indextype)hyp->indextable[dest_page]->type;
						
						info->tip = pagename(hyp, dest_page);
						switch (dst_type)
						{
						case HYP_NODE_INTERNAL:
						case HYP_NODE_POPUP:
						case HYP_NODE_EXTERNAL_REF:
						case HYP_NODE_REXX_COMMAND:
						case HYP_NODE_REXX_SCRIPT:
						case HYP_NODE_QUIT:
						case HYP_NODE_CLOSE:
						case HYP_NODE_SYSTEM_ARGUMENT:
							break;
						case HYP_NODE_IMAGE:
						case HYP_NODE_EOF:
						default:
							g_free(info->tip);
							info->tip = g_strdup_printf(_("Link to node of type %u not implemented."), dst_type);
							break;
						}
					} else
					{
						info->tip = hyp_invalid_page(dest_page);
					}
					info->link_type = link_type;
					info->dst_type = dst_type;
					info->dest_page = dest_page;
					info->line_nr = line_nr;
					found = TRUE;
					break;
				}
			} else
			{
				link_w = 0;
				if (HYP_ESC_IS_TEXATTR(*src))	/* text attributes */
				{
					textattr = *src - HYP_ESC_TEXTATTR_FIRST;
					SelectObject(hdc, (HGDIOBJ)win->fonts[(textattr) & HYP_TXT_MASK]);
					src++;
				} else
				{
					src = hyp_skip_esc(src - 1);
				}
			}
			last_esc = src;
		} else if (*src == HYP_EOL)
		{
			link_w = 0;
			++lineno;
			src++;
			last_esc = src;
			x = sx;
			sy += win->y_raster;
			sy = skip_graphics(win, node->gfx, lineno, sx, sy);
		} else
		{
			*test_mem = *src;
		  check_char:
			TEXTOUT(test_mem, 1);
			src++;
			target_col = mx >= x && mx < (x + link_w);
		}

		if (target_line && target_col)
		{
			if (select_word)
			{
				hyp_nodenr ret;
				unsigned char buffer[256];
				char *name;
				
				get_word(last_esc, src - 1, buffer);
				if (*buffer)
				{
					name = hyp_conv_to_utf8(hyp->comp_charset, buffer, STR0TERM);
					ret = HypFindNode(doc, name);
					if (ret != HYP_NOINDEX)
					{
						info->link_type = HYP_ESC_LINK;
						info->dest_page = ret;
						info->dst_type = (hyp_indextype)hyp->indextable[ret]->type;
						found = TRUE;
					} else
					{
						search_allref(win, name, FALSE);
					}
					g_free(name);
				}
			}
			break;
		}
		x += link_w;
	}

	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(win->textwin, hdc);
	
	return found;
}
