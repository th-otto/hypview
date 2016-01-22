#include "hv_gtk.h"
#include "hypdebug.h"
#include "gdkkeysyms.h"


#define AUTOLOC_SIZE		26

#define IsModifierKey(key) \
	((key) == GDK_KEY_Shift_L || \
	 (key) == GDK_KEY_Shift_R || \
	 (key) == GDK_KEY_Control_L || \
	 (key) == GDK_KEY_Control_R || \
	 (key) == GDK_KEY_Caps_Lock || \
	 (key) == GDK_KEY_Shift_Lock || \
	 (key) == GDK_KEY_Meta_L || \
	 (key) == GDK_KEY_Meta_R || \
	 (key) == GDK_KEY_Alt_L || \
	 (key) == GDK_KEY_Alt_R || \
	 (key) == GDK_KEY_Super_L || \
	 (key) == GDK_KEY_Super_R || \
	 (key) == GDK_KEY_Hyper_L || \
	 (key) == GDK_KEY_Hyper_R || \
	 ((key) >= GDK_KEY_ISO_Lock && (key) <= GDK_KEY_ISO_Last_Group_Lock) || \
	 (key) == GDK_KEY_Scroll_Lock)

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * Initialize and activate the autolocator.
 * Returns position of next character
 */
static char *AutolocatorInit(DOCUMENT *doc)
{
	char *ptr;

	/* memory already allocated? */
	if (doc->autolocator == NULL)
	{
		ptr = g_new(char, AUTOLOC_SIZE);
		if (ptr == NULL)
		{
			return NULL;
		}
		doc->autolocator = ptr;
		ptr[AUTOLOC_SIZE - 1] = 0;
		*ptr = 0;
	} else
	{
		/* to end of string */
		ptr = doc->autolocator;
		while (*ptr)
			ptr++;
	}

	/* search box already active? */
	if (!doc->buttons.searchbox)
		doc->buttons.searchbox = TRUE;

	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

/* Update the autolocator and start a search */
static void AutolocatorUpdate(DOCUMENT *doc, long start_line)
{
	WINDOW_DATA *win = doc->window;
	long line = start_line;

	if (!doc->buttons.searchbox)
		return;

	gtk_widget_hide(win->strnotfound);

	/* if autolocator is not empty... */
	if (*doc->autolocator)
	{
		line = doc->autolocProc(doc, start_line);
	}

	if (line >= 0)
	{
		long topline = hv_win_topline(win);
		if (line != topline)
		{
			hv_win_scroll_to_line(win, line);
		}
	} else
	{
		gtk_widget_show(win->strnotfound);
		gdk_display_beep(gtk_widget_get_display(win->hwnd));
	}
}

/*** ---------------------------------------------------------------------- ***/

/* add a new character to the Autolocator and start search */
gboolean AutolocatorKey(DOCUMENT *doc, GdkEventKey *event)
{
	WINDOW_DATA *win = doc->window;
	char *ptr;
	long line = hv_win_topline(win);

	if (!event->keyval)
		return FALSE;
	if (IsModifierKey(event->keyval))
		return FALSE;

	ptr = AutolocatorInit(doc);
	doc->autolocator_dir = 1;

	switch (event->keyval)
	{
	case GDK_KEY_BackSpace:			/* Backspace */
		if (ptr > doc->autolocator)
			ptr--;
		*ptr = 0;
		break;
	case GDK_KEY_KP_Enter:
	case GDK_KEY_Return:			/* Return */
		if (event->state & GDK_SHIFT_MASK)
		{
			doc->autolocator_dir = 0;
			line--;
		} else
		{
			line++;
		}
		break;
	case GDK_KEY_Escape:			/* Escape */
		if (ptr > doc->autolocator)
		{
			ptr = doc->autolocator;
			*ptr = 0;
		} else
		{
			RemoveSearchBox(doc);
		}
		break;
	case GDK_KEY_space:
		/* ignore space at start of string */
		if (ptr != doc->autolocator)
			*ptr++ = ' ';
		*ptr = 0;
		break;
	default:
		if (event->keyval > ' ')
		{
			if (ptr - doc->autolocator < AUTOLOC_SIZE)
			{
				*ptr++ = event->keyval;
				*ptr = 0;
			} else
			{
				--ptr;
				*ptr = event->keyval;
			}
		}
		break;
	}

	ToolbarUpdate(doc, FALSE);
	AutolocatorUpdate(doc, line);

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* insert contents of clipboard in autolocator. */
void AutoLocatorPaste(DOCUMENT *doc)
{
	/* YYY */
	if (!doc->buttons.searchbox)
		return;
}
