#include "hv_gtk.h"
#include "hypdebug.h"


#define AUTOLOC_SIZE		26

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
		if (line != win->docsize.y)
		{
			win->docsize.y = line;
			SendRedraw(win);
		}
	} else
	{
		gtk_widget_show(win->strnotfound);
		gdk_display_beep(gtk_widget_get_display(win->hwnd));
	}
}


/*add a new character to the Autolocator and start search */
short AutolocatorKey(DOCUMENT *doc, GdkModifierType state, int ascii)
{
	WINDOW_DATA *win = doc->window;
	char *ptr;
	long line = win->docsize.y;

	if (!ascii)
		return FALSE;

	ptr = AutolocatorInit(doc);
	doc->autolocator_dir = 1;

	if (ascii == 8)						/* Backspace */
	{
		if (ptr > doc->autolocator)
			ptr--;
		*ptr = 0;
	} else if (ascii == 13)				/* Return */
	{
		if (state & GDK_SHIFT_MASK)
		{
			doc->autolocator_dir = 0;
			line--;
		} else
			line++;
	} else if (ascii == 27)				/* Escape */
	{
		if (ptr > doc->autolocator)
		{
			ptr = doc->autolocator;
			*ptr = 0;
		} else
		{
			RemoveSearchBox(doc);
		}
	} else if (ascii == ' ')
	{
		/* ignore space at start of string */
		if (ptr != doc->autolocator)
			*ptr++ = ' ';
		*ptr = 0;
	} else if (ascii > ' ')
	{
		if (ptr - doc->autolocator < AUTOLOC_SIZE)
		{
			*ptr++ = ascii;
			*ptr = 0;
		} else
		{
			--ptr;
			*ptr = ascii;
		}
	}

	ToolbarUpdate(doc, FALSE);
	AutolocatorUpdate(doc, line);

	return TRUE;
}


/* insert contents of clipboard in autolocator. */
void AutoLocatorPaste(DOCUMENT *doc)
{
	/* YYY */
	UNUSED(doc);
}
