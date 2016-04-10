#include "hv_defs.h"
#include "hypdebug.h"
#include "resource.rh"

static volatile int remarker_pid = -1;
static UINT_PTR remarker_timer;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void CALLBACK check_remarker(HWND hwnd, UINT msg, UINT_PTR id, DWORD dwtime)
{
	UNUSED(hwnd);
	UNUSED(msg);
	UNUSED(id);
	UNUSED(dwtime);
	if (remarker_pid > 0)
	{
		DWORD ret;
		
		if (GetExitCodeProcess((HANDLE)remarker_pid, &ret) && ret != STILL_ACTIVE)
		{
			CloseHandle((HANDLE)remarker_pid);
			remarker_pid = -1;
		}
	}
	if (remarker_pid < 0)
	{
		if (remarker_timer != 0)
		{
			KillTimer(NULL, remarker_timer);
			remarker_timer = 0;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

int StartRemarker(WINDOW_DATA *win, remarker_mode mode, gboolean quiet)
{
	char *path;
	const char *argv[5];
	int argc = 0;
	
	if (remarker_pid > 0 || mode == remarker_check)
		return remarker_pid;
	
	path = path_subst(gl_profile.remarker.path);
	
	if (empty(path))
	{
		if (!quiet)
			show_message(win ? win->hwnd : NULL, _("Error"), _("No path to REMARKER configured"), FALSE);
	} else
	{
		char *nodename = NULL;
		
		argv[argc++] = path;
		if (mode == remarker_startup)
		{
			argv[argc++] = "-t";
		} else if (mode == remarker_top && win)
		{
			DOCUMENT *doc = win->data;
			argv[argc++] = "-r";
			argv[argc++] = doc->path;
			if (doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				nodename = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[win->displayed_node->number]->name, STR0TERM);
				argv[argc++] = nodename;
			}
		}
		argv[argc] = NULL;
		if ((remarker_pid = hyp_utf8_spawnvp(P_NOWAIT, 1, argv)) < 0)
		{
			if (!quiet)
			{
				char *str = g_strdup_printf(_("Can not execute\n'%s'\n%s"), path, hyp_utf8_strerror(errno));
				show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
				g_free(str);
			}
		} else
		{
			remarker_timer = SetTimer(NULL, 0, 50, check_remarker);
		}
		g_free(nodename);
	}
	g_free(path);
	return remarker_pid;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void BlockOperation(WINDOW_DATA *win, enum blockop num)
{
	DOCUMENT *doc = win->data;

	switch (num)
	{
	case CO_SAVE:
		SelectFileSave(win);
		break;
	case CO_BACK:
		GoBack(win);
		break;
	case CO_COPY:
		BlockCopy(win);
		break;
	case CO_PASTE:
		if (doc->buttons.searchbox)
			AutoLocatorPaste(win);
		else
			BlockPaste(win, gl_profile.viewer.clipbrd_new_window);
		break;
	case CO_SELECT_ALL:
		SelectAll(win);
		break;
	case CO_SEARCH:
		Hypfind(win, FALSE);
		break;
	case CO_SEARCH_AGAIN:
		Hypfind(win, TRUE);
		break;
	case CO_DELETE_STACK:
		RemoveAllHistoryEntries(win);
		ToolbarUpdate(win, TRUE);
		break;
	case CO_SWITCH_FONT:
		gl_profile.viewer.use_xfont = !gl_profile.viewer.use_xfont;
		SwitchFont(win);
		break;
	case CO_SELECT_FONT:
		SelectFont(win);
		break;
	case CO_REMARKER:
		StartRemarker(win, remarker_top, FALSE);
		ToolbarUpdate(win, FALSE);
		break;
	case CO_PRINT:
		HYP_DBG(("NYI: print"));
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void BlockSelectAll(WINDOW_DATA *win, BLOCK *b)
{
	b->start.line = 0;
	b->start.y = 0;
	b->start.offset = 0;
	b->start.x = 0;
	b->end.line = win->docsize.h / win->y_raster;
	b->end.y = win->docsize.h;
	b->end.offset = 0;
	b->end.x = 0;
	b->valid = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean BlockCopy(WINDOW_DATA *win)
{
	HYP_NODE *node;
	long line;
	ssize_t len;
	size_t wlen;
	char *line_buffer;
	const char *src;
	BLOCK b = win->selection;
	wchar_t *data;
	wchar_t *wtxt;
	size_t datalen;
	HANDLE hdata;
	wchar_t *clipdata;
	gboolean result;
	
	node = hypwin_node(win);
	if (!node)					/* no page loaded */
	{
		HYP_DBG(("Error: Can't save, no page loaded"));
		return FALSE;
	}

	if (!b.valid)
		BlockSelectAll(win, &b);
	
	line = b.start.line;
	data = NULL;
	datalen = 0;
	while (line <= b.end.line)
	{
		line_buffer = HypGetTextLine(win, node, line);

		if (line_buffer != NULL)
		{
			len = strlen(line_buffer);

			if (line == b.end.line && b.end.offset < len)
			{
				line_buffer[b.end.offset] = 0;
				len = b.end.offset;
			}
			if (line == b.start.line)
			{
				if (b.start.offset > len)
				{
					src = line_buffer + len;
					len = 0;
				} else
				{
					src = &line_buffer[b.start.offset];
					len -= b.start.offset;
				}
			} else
			{
				src = line_buffer;
			}
			
			wtxt = hyp_utf8_to_wchar(src, len, &wlen);
			data = g_renew(wchar_t, data, datalen + wlen + 1);
			if (data == NULL)
			{
				return FALSE;
			}
			memcpy(data + datalen, wtxt, wlen * sizeof(*data));
			datalen += wlen;
			g_free(wtxt);
			g_free(line_buffer);
		}
							
		if (line != b.end.line || b.end.offset == 0)
		{
			data = g_renew(wchar_t, data, datalen + 3);
			if (data == NULL)
			{
				return FALSE;
			}
			data[datalen++] = '\r';
			data[datalen++] = '\n';
		}
		
		line++;
	}
	
	if (data == NULL)
		return TRUE;
	data[datalen++] = '\0';
	
	result = FALSE;
	if ((hdata = GlobalAlloc(GMEM_MOVEABLE, datalen * sizeof(*data))) != 0)
	{
		if ((clipdata = (wchar_t *)GlobalLock(hdata)) != NULL)
		{
			memcpy(clipdata, data, datalen * sizeof(*data));
			GlobalUnlock(hdata);
			if (OpenClipboard(win->hwnd))
			{
				if (EmptyClipboard())
				{
					SetClipboardData(CF_UNICODETEXT, hdata);
					result = TRUE;
				} else
				{
					hyp_debug("EmptyCliboard failed: %s\n", win32_errstring(GetLastError()));
					GlobalFree(hdata);
				}
				CloseClipboard();
			} else
			{
				hyp_debug("OpenClipboard failed: %s\n", win32_errstring(GetLastError()));
				GlobalFree(hdata);
			}
		} else
		{
			hyp_debug("GlobalLock failed: %s\n", win32_errstring(GetLastError()));
			GlobalFree(hdata);
		}
	} else
	{
		hyp_debug("GlobalAlloc failed: %s\n", win32_errstring(GetLastError()));
	}
	g_free(data);
	return result;
}

/*** ---------------------------------------------------------------------- ***/

gboolean BlockPaste(WINDOW_DATA *win, gboolean new_window)
{
	gboolean result = FALSE;
	HANDLE hClipData;
	wchar_t *Text;
	const char *path = "$CLIPBRD";
	DOCUMENT *doc;
	DOCUMENT *prev_doc;
	gboolean add_to_hist = TRUE;
	FMT_ASCII *ascii;
	
	if (OpenClipboard(win->hwnd))
	{
		hClipData = GetClipboardData(CF_UNICODETEXT);
		if (hClipData != NULL)
		{
			if ((Text = (wchar_t *)GlobalLock(hClipData)) != NULL)
			{
				size_t len = GlobalSize(hClipData) / sizeof(wchar_t);
				char *txt;
				
				if (len > 0 && Text[len - 1] == 0)
					len--;
				txt = hyp_wchar_to_utf8(Text, len);
				
				len = strlen(txt);
				ascii = (FMT_ASCII *)g_malloc(sizeof(FMT_ASCII) + len);
				if (ascii != NULL)
				{
					memcpy(ascii->start, txt, len);
					ascii->start[len] = '\0';
					
					/* create a new document */
					doc = g_new0(DOCUMENT, 1);

					doc->path = g_strdup(path);
					doc->buttons.load = TRUE;
					doc->type = HYP_FT_UNKNOWN;
					doc->ref_count = 1;
					
					if (new_window)
						win = NULL;
					new_window = FALSE;
					if (!win)
					{
						win = win32_hypview_window_new(doc, FALSE);
						new_window = TRUE;
						add_to_hist = FALSE;
						prev_doc = NULL;
					} else
					{
						prev_doc = win->data;
						win->data = doc;
					}
					
					ascii->length = len;
					ascii->charset =  HYP_CHARSET_UTF8;
					if (AsciiCalcLines(doc, ascii) < 0)
					{
						g_free(doc);
						win->data = prev_doc;
						if (new_window)
							DestroyWindow(win->hwnd);
					} else
					{
						if (add_to_hist)
							AddHistoryEntry(win, prev_doc);
						ReInitWindow(win, TRUE);
						hv_win_open(win);
						result = TRUE;
					}
				}
				GlobalUnlock(hClipData);
				
				g_free(txt);
			}
		}
		CloseClipboard();
	}
	return result;
}

/*** ---------------------------------------------------------------------- ***/

void BlockAsciiSave(WINDOW_DATA *win, const char *path)
{
	DOCUMENT *doc = win->data;
	int handle;

	if (doc->blockProc == NULL)
	{
		return;
	}

	handle = hyp_utf8_open(path, O_WRONLY | O_TRUNC | O_CREAT, HYP_DEFAULT_FILEMODE);
	if (handle < 0)
	{
		FileErrorErrno(path);
	} else
	{
		BLOCK b = win->selection;

		if (!b.valid)					/* no block selected? */
			BlockSelectAll(win, &b);

		doc->blockProc(win, BLK_ASCIISAVE, &b, &handle);
		hyp_utf8_close(handle);
	}
}
