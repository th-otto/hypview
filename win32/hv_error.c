/*
 * HypView - (c)      - 2019 Thorsten Otto
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
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hv_defs.h"

static gboolean have_console;
GSList *all_list;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

int toplevels_open_except(WINDOW_DATA *top)
{
	GSList *l;
	int num_open;
	
	for (l = all_list, num_open = 0; l != NULL; l = l->next)
		if (l->data != top)
		{
			num_open++;
		}
	return num_open;
}

/*** ---------------------------------------------------------------------- ***/

void check_toplevels(WINDOW_DATA *toplevel)
{
	int num_open;
	
	if ((num_open = toplevels_open_except(toplevel)) == 0)
	{
		PostQuitMessage(0);
	}
}

/*** ---------------------------------------------------------------------- ***/

HWND top_window(void)
{
	if (all_list)
	{
		WINDOW_DATA *win = (WINDOW_DATA *)all_list->data;
		return win->hwnd;
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

void check_console(void)
{
#ifdef G_PLATFORM_WIN32
	CONSOLE_SCREEN_BUFFER_INFO ci;
	
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &ci))
	{
#ifdef G_OS_UNIX
		/* G_PLATFORM_WIN32 + G_OS_UNIX = CYGWIN */
		have_console = TRUE;
#endif
	} else
	{
		have_console = TRUE;
	}
#endif /* G_PLATFORM_WIN32 */

#ifdef G_OS_UNIX
	have_console = isatty(0);
#endif
}

/*** ---------------------------------------------------------------------- ***/

void show_message(HWND parent, const char *title, const char *text, gboolean big)
{
	wchar_t *wtext = hyp_utf8_to_wchar(text, STR0TERM, NULL);
	wchar_t *wtitle = hyp_utf8_to_wchar(title, STR0TERM, NULL);
	
	UNUSED(big);
	MessageBoxW(parent, wtext, wtitle, MB_ICONERROR | MB_OK);
	g_free(wtitle);
	g_free(wtext);
}

/*** ---------------------------------------------------------------------- ***/

gboolean ask_yesno(HWND parent, const char *text)
{
	wchar_t *wtext = hyp_utf8_to_wchar(text, STR0TERM, NULL);
	wchar_t *wtitle = hyp_utf8_to_wchar(gl_program_name, STR0TERM, NULL);
	gboolean ret = MessageBoxW(parent, wtext, wtitle, MB_ICONQUESTION | MB_YESNO) == IDYES;
	g_free(wtitle);
	g_free(wtext);
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * write a message to stdout/stderr if we are attached to a tty,
 * otherwise pop up a dialog
 */
void write_console(const char *s, gboolean use_gui, gboolean to_stderr, gboolean big)
{
	if (have_console)
	{
		fflush(stdout);
		fflush(stderr);
		hyp_utf8_fprintf(to_stderr ? stderr : stdout, "%s\n", s);
	}
	if (use_gui)
	{
		show_message(NULL, to_stderr ? _("Error") : _("Warning"), s, big);
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void FileError(const char *path, const char *msg)
{
	char *filename;
	char *str;

	filename = hyp_path_get_basename(path);
	str = g_strdup_printf(_("File '%s'\n%s"), path, msg);
	write_console(str, TRUE, TRUE, FALSE);
	g_free(str);
	g_free(filename);
}
