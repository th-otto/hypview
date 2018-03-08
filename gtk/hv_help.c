/*****************************************************************************
 * HELP.C
 *****************************************************************************/

#include "hv_gtk.h"

#ifdef G_PLATFORM_WIN32
#include <gdk/gdkwin32.h>
#endif

#ifdef HELP_SUPPORT_CHM
#include "htmlhelp.h"

#ifdef __WIN32__
typedef HWND (WINAPI *HTMLHELP_TYPE)(HWND, LPCTSTR, UINT, DWORD_PTR);
static HINSTANCE hhctrl;
static HTMLHELP_TYPE p_htmlhelp;
static DWORD htmlhelpCookie;
#else
typedef GtkWidget (*HTMLHELP_TYPE)(GtkWidget *, const char *, unsigned int, void *);
static void *htmlhelpCookie;
static HTMLHELP_TYPE p_htmlhelp;
#endif
#endif /* HELP_SUPPORT_CHM */

static char *docdir;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#ifdef HELP_SUPPORT_CHM

static void exit_htmlhelp(void)
{
#ifdef __WIN32__
	if (hhctrl != NULL && hhctrl != (HINSTANCE)-1)
	{
		(*p_htmlhelp)(NULL, NULL, HH_UNINITIALIZE, (DWORD_PTR) htmlhelpCookie);
		FreeLibrary(hhctrl);
		hhctrl = NULL;
		p_htmlhelp = 0;
	}
#endif
}

/*** ---------------------------------------------------------------------- ***/

static gboolean have_htmlhelp(void)
{
	if (p_htmlhelp != 0)
		return TRUE;
#ifdef __WIN32__
	if (hhctrl == NULL)
	{
		UINT old_error_mode;
		
		old_error_mode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
		hhctrl = LoadLibrary("HHCTRL.OCX");
		if (hhctrl == NULL)
			hhctrl = (HINSTANCE) -1;
		SetErrorMode(old_error_mode);
	}
	if (hhctrl == (HINSTANCE)-1)
		return FALSE;
#ifdef _UNICODE
	p_htmlhelp = (HTMLHELP_TYPE)GetProcAddress(hhctrl, "HtmlHelpW");
#else
	p_htmlhelp = (HTMLHELP_TYPE)GetProcAddress(hhctrl, "HtmlHelpA");
#endif
	if (p_htmlhelp == NULL)
	{
		FreeLibrary(hhctrl);
		hhctrl = (HINSTANCE)-1;
		return FALSE;
	}
	
	(*p_htmlhelp)(NULL, NULL, HH_INITIALIZE, (DWORD_PTR) &htmlhelpCookie);
#else
	if (p_htmlhelp == NULL)
		return FALSE;
	(*p_htmlhelp)(NULL, NULL, HH_INITIALIZE, &htmlhelpCookie);
#endif

	return TRUE;
}

#endif /* HELP_SUPPORT_CHM */

/*** ---------------------------------------------------------------------- ***/

static gboolean file_available(const char *cs)
{
	FILE *file;

	file = fopen(cs, "rb");
	if (file == NULL)
		return FALSE;
	fclose(file);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean file_found(char *filename)
{
	char *path;
	
	if (file_available(filename))
		return TRUE;
	path = g_build_filename("Manual/en/htmlhelp", filename, NULL);
	if (file_available(path))
	{
		strcpy(filename, path);
		g_free(path);
		return TRUE;
	}
	g_free(path);
	path = g_build_filename(docdir, filename, NULL);
	if (file_available(path))
	{
		strcpy(filename, path);
		g_free(path);
		return TRUE;
	}
	g_free(path);
	return FALSE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

gboolean Help_Show(GtkWidget *parent, const char *filename, const char *entry)
{
	gboolean found;
	char *my_help_name;
	char *p;
	int type;
	
	UNUSED(entry);
	if (empty(filename))
		return FALSE;
	
	parent = gtk_widget_get_toplevel(parent);
	
	found = FALSE;
	if ((my_help_name = g_new(char, PATH_MAX)) == NULL)
		return FALSE;
	type = -1;

	strcpy(my_help_name, filename);
	if (file_found(my_help_name))
	{
		p = strrchr(my_help_name, '.');
		found = TRUE;
		if (p != NULL)
		{
			if (filename_cmp(p, ".hlp") == 0)
				type = 0;
#ifdef HELP_SUPPORT_CHM
			if (filename_cmp(p, ".chm") == 0 && have_htmlhelp())
				type = 1;
#endif
		}
	}
	
	p = strrchr(my_help_name, '.');
	if (p == NULL)
		p = my_help_name + strlen(my_help_name);
	if (!found)
	{
		strcpy(p, ".hlp");
		if (file_found(my_help_name))
		{
			found = TRUE;
			type = 0;
		}
	}
#ifdef HELP_SUPPORT_CHM
	if (!found)
	{
		strcpy(p, ".chm");
		if (file_found(my_help_name))
		{
			found = TRUE;
			if (have_htmlhelp())
				type = 1;
		}
	}
#endif

	if (found && type == 0)
	{
#ifdef G_PLATFORM_WIN32
		HWND parent_hwnd = GDK_WINDOW_HWND(gtk_widget_get_window(parent));
		
		if (entry == NULL)
		{
			WinHelp(parent_hwnd, my_help_name, HELP_PARTIALKEY, (DWORD_PTR) "");
		} else if (IS_INTRESOURCE(entry))
		{
			WinHelp(parent_hwnd, my_help_name, HELP_CONTEXT, (DWORD_PTR) entry);
		} else if (strcmp(entry, "$Contents") == 0)
		{
			WinHelp(parent_hwnd, my_help_name, HELP_CONTEXT, (DWORD_PTR) "UDOTOC");
		} else if (strcmp(entry, "$Index") == 0)
		{
			WinHelp(parent_hwnd, my_help_name, HELP_PARTIALKEY, (DWORD_PTR) "");
		} else if (strcmp(entry, "$Help") == 0)
		{
			WinHelp(parent_hwnd, my_help_name, HELP_HELPONHELP, 0);
		} else
		{
			WinHelp(parent_hwnd, my_help_name, HELP_KEY, (DWORD_PTR) entry);
		}
#endif
	}

#ifdef HELP_SUPPORT_CHM
	if (found && type == 1)
	{
#ifdef __WIN32__
		/* DWORD dwCookie; */
		HWND help_hwnd;
		HWND parent_hwnd = GDK_WINDOW_HWND(gtk_widget_get_window(parent));
		
		/* (*htmlhelp_addr)(NULL, NULL, HH_INITIALIZE, (DWORD_PTR) &dwCookie); */
		if (entry == NULL)
		{
			help_hwnd = (*p_htmlhelp)(parent_hwnd, my_help_name, HH_DISPLAY_INDEX, (DWORD_PTR) "");
		} else if (strcmp(entry, "$Contents") == 0)
		{
			help_hwnd = (*p_htmlhelp)(parent_hwnd, my_help_name, HH_DISPLAY_TOC, (DWORD_PTR) NULL);
		} else if (strcmp(entry, "$Index") == 0)
		{
			help_hwnd = (*p_htmlhelp)(parent_hwnd, my_help_name, HH_DISPLAY_INDEX, (DWORD_PTR) "");
		} else if (strcmp(entry, "$Help") == 0)
		{
			help_hwnd = (*p_htmlhelp)(parent_hwnd, my_help_name, HH_DISPLAY_TOC, (DWORD_PTR) NULL);
		} else
		{
			HH_AKLINK link;
			
			memset(&link, 0, sizeof(link));
			link.cbStruct = sizeof(link);
			link.fReserved = FALSE;
			link.pszKeywords = entry;
			link.pszUrl = NULL;
			link.pszMsgText = NULL;
			link.pszMsgTitle = NULL;
			link.pszWindow = NULL;
			link.fIndexOnFail = TRUE;
			/* (*p_htmlhelp)(parent_hwnd, my_help_name, HH_DISPLAY_TOPIC, (DWORD_PTR) NULL); */
			help_hwnd = (*p_htmlhelp)(parent_hwnd, my_help_name, HH_KEYWORD_LOOKUP, (DWORD_PTR) &link);
			/* help_hwnd = (*p_htmlhelp)(parent_hwnd, my_help_name, HH_DISPLAY_TOC, (DWORD_PTR) entry); */
		}
		if (help_hwnd != NULL)
		{
		}
#endif
	}
#endif /* HELP_SUPPORT_CHM */
	g_free(my_help_name);
	if (!found)
	{
		show_dialog(parent, GTK_STOCK_DIALOG_ERROR, _("No help file found"), NULL, NULL);
	}
	return found;
}

/*** ---------------------------------------------------------------------- ***/

void Help_Index(GtkWidget *parent, const char *filename)
{
	Help_Show(parent, filename, "$Index");
}

/*** ---------------------------------------------------------------------- ***/

void Help_Contents(GtkWidget *parent, const char *filename)
{
	Help_Show(parent, filename, "$Contents");
}

/*** ---------------------------------------------------------------------- ***/

void Help_Using_Help(GtkWidget *parent, const char *filename)
{
	Help_Show(parent, filename, "$Help");
}

/*** ---------------------------------------------------------------------- ***/

void Help_Exit(void)
{
#ifdef HELP_SUPPORT_CHM
	exit_htmlhelp();
#endif
}

/*** ---------------------------------------------------------------------- ***/

void Help_Init(void)
{
	char *root;
	
	root = g_get_package_installation_directory();
	docdir = g_build_filename(root, "/share/doc/hypview" , NULL);
	g_free(root);
}
