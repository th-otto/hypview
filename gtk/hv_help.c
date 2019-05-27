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

/*****************************************************************************
 * HELP.C
 *****************************************************************************/

#define GDK_DISABLE_DEPRECATION_WARNINGS
#include "hv_gtk.h"
#include "localename.h"

static const char *help_filename;
static char *docdir;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

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

static char *file_found(const char *filename)
{
	char *path;
	char *locale;
	char *p;
	
	if (file_available(filename))
		return g_strdup(filename);
	locale = g_strdup(gl_locale_name(LC_MESSAGES, "LC_MESSAGES"));
	if (locale)
	{
		p = strchr(locale, '_');
		if (p)
			*p = '\0';
	}

	if (locale)
	{
		path = g_build_filename("..", "doc", "output", locale, "stguide", filename, NULL);
		if (file_available(path))
		{
			g_free(locale);
			return path;
		}
		g_free(path);
	}

	path = g_build_filename("..", "doc", "output", "en", "stguide", filename, NULL);
	if (file_available(path))
	{
		g_free(locale);
		return path;
	}
	g_free(path);

	if (locale)
	{
		path = g_build_filename(docdir, locale, filename, NULL);
		if (file_available(path))
		{
			g_free(locale);
			return path;
		}
		g_free(path);
	}

	path = g_build_filename(docdir, "en", filename, NULL);
	if (file_available(path))
	{
		g_free(locale);
		return path;
	}
	g_free(path);

	g_free(locale);
	return NULL;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

gboolean Help_Show(WINDOW_DATA *parent, const char *entry)
{
	gboolean found;
	char *my_help_name;
	
	if (empty(help_filename))
		return FALSE;
	
	found = FALSE;

	if ((my_help_name = file_found(help_filename)) != NULL)
	{
		found = TRUE;
		OpenFileInWindow(parent, my_help_name, entry, HYP_NOINDEX, TRUE, FORCE_NEW_WINDOW, FALSE);
	}
	
	g_free(my_help_name);
	if (!found)
	{
		show_message(GTK_WIDGET(parent), _("Error"), _("No help file found"), FALSE);
	}
	return found;
}

/*** ---------------------------------------------------------------------- ***/

void Help_Index(WINDOW_DATA *parent)
{
	Help_Show(parent, "Index");
}

/*** ---------------------------------------------------------------------- ***/

void Help_Contents(WINDOW_DATA *parent)
{
	Help_Show(parent, "Main");
}

/*** ---------------------------------------------------------------------- ***/

void Help_Using_Help(WINDOW_DATA *parent)
{
	Help_Show(parent, "Help");
}

/*** ---------------------------------------------------------------------- ***/

void Help_Exit(void)
{
	g_freep(&docdir);
}

/*** ---------------------------------------------------------------------- ***/

void Help_Init(void)
{
	char *root;
	
	root = g_get_package_installation_directory();
	docdir = g_build_filename(root, "share", "hypview", NULL);
	help_filename = "hypview.hyp";
	g_free(root);
}
