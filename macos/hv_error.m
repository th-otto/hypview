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
#if 0 /* TODO */
		PostQuitMessage(0);
#endif
	}
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *top_window(void)
{
	if (all_list)
	{
		WINDOW_DATA *win = (WINDOW_DATA *)all_list->data;
		return win;
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

void check_console(void)
{
#ifdef G_OS_UNIX
	have_console = isatty(0);
#endif
}

/*** ---------------------------------------------------------------------- ***/

void show_message(WINDOW_DATA *parent, const char *title, const char *text, gboolean big)
{
	wchar_t *wtext = hyp_utf8_to_wchar(text, STR0TERM, NULL);
	wchar_t *wtitle = hyp_utf8_to_wchar(title, STR0TERM, NULL);
	
	UNUSED(big);
#if 0 /* TODO */
	MessageBoxW(parent, wtext, wtitle, MB_ICONERROR | MB_OK);
#endif
	g_free(wtitle);
	g_free(wtext);
}

/*** ---------------------------------------------------------------------- ***/

gboolean ask_yesno(WINDOW_DATA *parent, const char *text)
{
	wchar_t *wtext = hyp_utf8_to_wchar(text, STR0TERM, NULL);
	wchar_t *wtitle = hyp_utf8_to_wchar(gl_program_name, STR0TERM, NULL);
#if 0 /* TODO */
	gboolean ret = MessageBoxW(parent, wtext, wtitle, MB_ICONQUESTION | MB_YESNO) == IDYES;
#else
	gboolean ret = FALSE;
#endif
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
