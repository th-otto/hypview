/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"
#include "hypdebug.h"
#include "av.h"
#include "bgh.h"
#include "tos/mem.h"


#define AC_HELP		1025				/* PureC Hilfe-Protokoll */

static hyp_nodenr last_node;

/*******************************************************/
/****** Events                                    ******/
/*******************************************************/

void DoButton(EVNT *event)
{
	UNUSED(event);

#if USE_BUBBLEGEM
	if (event->button == 2)
		Bubble(event->mx, event->my);
#endif
}


void DoUserEvents(EVNT *event)
{
	if (event->mwhich & MU_MESAG)
	{
		switch (event->msg[0])
		{
		case AC_OPEN:
			{
				WINDOW_DATA *win;
				
				va_proto_init(NULL);
				win = find_window_by_proc(HelpWindow);
				if (win == NULL && !empty(gl_profile.viewer.last_file))
				{
					win = OpenFileInWindow(NULL, gl_profile.viewer.last_file, NULL, last_node, FALSE, TRUE, FALSE);
				}
	
				if (win == NULL)
				{
					if (!empty(gl_profile.viewer.default_file))
					{
						char *filename = path_subst(gl_profile.viewer.default_file);
						if (OpenFileInWindow(win, filename, NULL, HYP_NOINDEX, FALSE, TRUE, FALSE) == NULL)
						{
							g_freep(&gl_profile.viewer.default_file);
#if 0
							if (gl_profile.profile)
								gl_profile.profile->changed = TRUE;
#endif
							SelectFileLoad(NULL);
						}
						g_free(filename);
					} else
					{
						SelectFileLoad(NULL);
					}
				} else
				{
					SendTopped(win->whandle);
				}
			}
			event->mwhich &= ~MU_MESAG;
			break;

		case AC_CLOSE:
			event->mwhich &= ~MU_MESAG;
			{
				/* window handles are no longer valid when we receive this message */
				CHAIN_DATA *ptr;
				for (ptr = all_list; ptr; ptr = ptr->next)
					ptr->whandle = -1;
			}
			RemoveItems();
			va_proto_init(NULL);
			break;

		case AC_HELP:
			{
				char *data = *(char **) &event->msg[3];
	
				if (data != NULL)
				{
					char *name;
					
					HYP_DBG(("AC_HELP from %d: <%s>", event->msg[1], printnull(data)));
					name = hyp_conv_to_utf8(hyp_get_current_charset(), data, STR0TERM);
					search_allref(NULL, name, FALSE);
					g_free(name);
				}
			}
			event->mwhich &= ~MU_MESAG;
			break;

		case WM_CLOSED:
			/*
			 * save the path of the last window closed,
			 * so it can be reopenend again on receive of AC_OPEN.
			 * Only need to do this when running as accessory,
			 * since a regular application will exit when all
			 * windows are closed.
			 */
			if (!_app)
			{								/* find the last window */
				/*
				 * MU_MESAG must not be removed otherwise
				 * the window will not be closed
				 */
				WINDOW_DATA *win;
	
				win = find_window_by_whandle(event->msg[3]);
				if (win && (win->status & WIS_OPEN) && win->owner == gl_apid)
				{
					if (count_window() == 1)
					{
						DOCUMENT *doc;
	
						doc = (DOCUMENT *) win->data;
						last_node = doc->getNodeProc(win);
						g_free(gl_profile.viewer.last_file);
						gl_profile.viewer.last_file = g_strdup(doc->path);
					}
				}
			}
			break;

		case CH_EXIT:
			HypfindFinish(event->msg[3], event->msg[4]);
			event->mwhich &= ~MU_MESAG;
			break;
		}

	} else if (event->mwhich & MU_KEYBD)
	{
		_WORD ascii = event->key;
		_WORD kstate = event->kstate;

		ConvertKeypress(&ascii, &kstate);

		ascii = ascii & 0xff;

		if (kstate & KbCTRL)
		{
			if (ascii == 'W')
			{
				short global_cycle = (kstate & KbSHIFT ? !gl_profile.viewer.av_window_cycle : gl_profile.viewer.av_window_cycle);

				if (av_server_cfg != 0 && global_cycle)
				{
					SendAV_SENDKEY(event->kstate, event->key);
					event->mwhich &= ~MU_KEYBD;
				} else
				{
					CycleItems();
					event->mwhich &= ~MU_KEYBD;
				}
			}
		}
	}
}



/*******************************************************/
/****** menu selection                                 */
/*******************************************************/
#if USE_MENU
void SelectMenu(short title, short entry)
{
	switch (title)
	{
	case ME_PROGRAM:
		switch (entry)
		{
		case ME_ABOUT:
			OpenDialog(HandleAbout, about_tree, rs_string(WDLG_ABOUT), -1, -1, NULL);
			break;
		}
		break;
	case ME_FILE:
		switch (entry)
		{
		case ME_QUIT:
			doneFlag = TRUE;
			break;
		}
		break;
	}
}
#endif



/*******************************************************/
/****** dialog objects with extended edit fields  ******/
/*******************************************************/
#if USE_LONGEDITFIELDS
LONG_EDIT long_edit[] = {
	{ MAIN, MA_EDIT, 20 },
	{ ABOUT, AB_EDIT, 40 }
};

short long_edit_count = (short) (sizeof(long_edit) / sizeof(LONG_EDIT));
#endif



/*******************************************************/
/****** Drag&Drop protokol                        ******/
/*******************************************************/
#if USE_DRAGDROP
/*
	Hier koennen je nach Zielobjekt <obj> die D&D-Daten <data> anders
	ausgewertet werden. <data> hat eines der gewuenschten Formate
	(<format>).
	Um <data> in seine Einzelbestaende zu zerlegen muss ParseData ver-
	wendet werden.
*/
void DD_Object(DIALOG *dial, GRECT *rect, OBJECT *tree, short obj, char *data, unsigned long format)
{
	char *next, *ptr = data;

	UNUSED(format);
	UNUSED(obj);
	UNUSED(tree);
	UNUSED(rect);
	UNUSED(dial);

	do
	{
		next = ParseData(ptr);
		ptr = next;
	} while (*next);
}


/*
 * select target format depending on object tree and object
 */
void DD_DialogGetFormat(OBJECT *tree, short obj, unsigned long format[])
{
	short i;

	UNUSED(obj);
	UNUSED(tree);

	for (i = 0; i < MAX_DDFORMAT; i++)
		format[i] = 0L;

	format[0] = 0x41524753L;			/* 'ARGS' */
}
#endif



/*******************************************************/
/****** AV Protokoll                              ******/
/*******************************************************/
/*
 * The server activates the program and passes a command line.
 */
void DoVA_START(_WORD msg[8])
{
	if (modal_items < 0)				/* only do this if we have no modal dialog */
	{
		char *data;

		data = *(char **) &msg[3];
		if (data != NULL)
		{
			char *arg;
			char *chapter;
			int count;
			char *filename;
			WINDOW_DATA *win;
			
			count = count_window();
			arg = hyp_conv_to_utf8(hyp_get_current_charset(), data, STR0TERM);
			SendAV_STARTED(msg);
			chapter = ParseData(arg);
			if (chapter != arg && *chapter == '\'')
				ParseData(chapter);
			filename = path_subst(arg);
			
			win = get_first_window();
			
			if (empty(filename))
			{
				g_free(filename);
				filename = path_subst(empty(gl_profile.viewer.default_file) ? gl_profile.viewer.catalog_file : gl_profile.viewer.default_file);
				chapter = NULL;
			}
			if (chapter == arg || empty(chapter))
				chapter = NULL;
			win = OpenFileInWindow(win, filename, chapter, HYP_NOINDEX, TRUE, gl_profile.viewer.va_start_newwin, FALSE);

			if (count == 0 && win != NULL)
			{
			}
			g_free(filename);
			g_free(arg);
			
		} else
		{
			WINDOW_DATA *win = get_first_window();

			SendAV_STARTED(msg);
			if (gl_profile.viewer.va_start_newwin == 2 || !win)
				win = NULL;
			if (!empty(gl_profile.viewer.default_file))
				win = OpenFileInWindow(win, gl_profile.viewer.default_file, NULL, HYP_NOINDEX, FALSE, gl_profile.viewer.va_start_newwin, FALSE);
			else
				SelectFileLoad(win);
		}
	} else
	{
		SendAV_STARTED(msg);
	}
}


void DoVA_DRAGACCWIND(_WORD msg[8])
{
	if (modal_items < 0)		/* only do this if we have no modal dialog */
	{
		char *data;

		data = *(char **) &msg[6];
		if (data != NULL)
		{
			char *arg;
			WINDOW_DATA *win;
			
			HYP_DBG(("VA_DRAGACCWIND from %d: <%s>", msg[1], printnull(data)));
			arg = hyp_conv_to_utf8(hyp_get_current_charset(), data, STR0TERM);
			ParseData(arg);

			win = find_window_by_whandle(msg[3]);
			if (win && win->owner != gl_apid)
				win = NULL;
			
			win = OpenFileInWindow(win, arg, NULL, HYP_NOINDEX, FALSE, gl_profile.viewer.va_start_newwin, FALSE);
			g_free(arg);
		}
	}
}
