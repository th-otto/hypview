#define GDK_DISABLE_DEPRECATION_WARNINGS
#include "hv_gtk.h"
#include "hypdebug.h"

#define MAX_RECENT 10

static GSList *recent_list;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void CenterWindow(GtkWidget *hwnd)
{
	gtk_window_set_position(GTK_WINDOW(hwnd), GTK_WIN_POS_CENTER);
}

/*-----------------------------------------------------------------------*/

#if GTK_CHECK_VERSION(2, 6, 0)

GtkWidget *_gtk_button_new_with_image_and_label(const char *stock, const gchar *label)
{
	GtkWidget *button;
	GtkWidget *image;
	
	if (stock && *stock && (image = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_BUTTON)) != NULL)
	{
		button = (GtkWidget *)g_object_new(GTK_TYPE_BUTTON,
                       "label", label,
                       "use_stock", FALSE,
                       "use_underline", TRUE,
                       "image", image,
                       NULL);
	} else
	{
		button = gtk_button_new_with_mnemonic(label);
	}
	return button;
}

#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * for debugging
 */ 
static void g_object_list_properties_for_type(const char *name, GObject *obj, GType type, int indent)
{
	int ind;
	
	for (ind = 0; ind < indent; ind++)
		printf("  ");
	printf("%s (%s):\n", name, g_type_name(type));
	if (G_OBJECT_TYPE(obj) == type)
	{
		GObjectClass *object_class;
		GParamSpec **specs;
		guint i, num;
		GValue value = G_VALUE_INIT;
		
		object_class = (GObjectClass *)g_type_class_peek(type);
		
		specs = g_object_class_list_properties(object_class, &num);
		for (i = 0; i < num; i++)
		{
			GParamSpec *param = specs[i];
			char *val;
			
			for (ind = 0; ind < indent; ind++)
				printf("  ");
			if (param->flags & G_PARAM_READABLE)
			{
				g_value_init(&value, param->value_type);
				g_object_get_property(obj, param->name, &value);
				val = g_strdup_value_contents(&value);
				printf("  %-20s: %s\n", param->name, val);
				g_free(val);
				g_value_unset(&value);
			} else
			{
				printf("  %-20s: (read-protected)\n", param->name);
			}
		}
		g_free(specs);
	}
}

/*** ---------------------------------------------------------------------- ***/

static gboolean list_root_props(const char *name, GObject *obj, GType type, int *max_indent, int indent)
{
	gboolean found = FALSE;
	GType currtype = type;
	
	if (currtype == G_TYPE_OBJECT)
	{
		found = TRUE;
	} else
	{
		currtype = g_type_parent(currtype);
		++(*max_indent);
		found = list_root_props(name, obj, currtype, max_indent, indent + 1);
	}
	if (found)
		g_object_list_properties_for_type(name, obj, type, *max_indent - indent);
	return found;
}

/*** ---------------------------------------------------------------------- ***/

void g_object_list_properties(const char *name, void *_obj)
{
	GObject *obj = (GObject *)_obj;
	GType type = G_OBJECT_TYPE(obj);
	int indent = 0;
	
	if (!list_root_props(name, obj, type, &indent, 0))
	{
		g_object_list_properties_for_type(name, obj, type, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

void RecentUpdate(WINDOW_DATA *win)
{
	GtkWidget *menu = win->recent_menu;
	GList *children, *child;
	GSList *l;
	
	children = gtk_container_get_children(GTK_CONTAINER(menu));
	l = recent_list;
	for (child = children; child; child = child->next)
	{
		GtkMenuItem *item = (GtkMenuItem *)child->data;
		const char *name = gtk_widget_get_name(GTK_WIDGET(item));
		if (strncmp(name, "recent-", 7) == 0)
		{
			if (l)
			{
				const char *path = (const char *)l->data;
				gtk_menu_item_set_label(item, hyp_basename(path));
				gtk_menu_item_set_use_underline(item, FALSE);
				gtk_widget_set_tooltip_text(GTK_WIDGET(item), path);
				gtk_widget_show(GTK_WIDGET(item));
				l = l->next;
			} else
			{
				gtk_widget_hide(GTK_WIDGET(item));
			}
		}
	}
	g_list_free(children);
}

/*** ---------------------------------------------------------------------- ***/

void on_recent_selected(GtkAction *action, WINDOW_DATA *win)
{
	const char *action_name = gtk_action_get_name(action);
	int sel;
	GdkModifierType mask;
	GSList *l;
	
	if (action_name == NULL || strncmp(action_name, "recent-", 7) != 0)
		return;
	sel = (int)strtol(action_name + 7, NULL, 10) - 1;
	gdk_display_get_pointer(gtk_widget_get_display(GTK_WIDGET(win)), NULL, NULL, NULL, &mask);
	for (l = recent_list; l; l = l->next)
	{
		if (sel == 0)
		{
			const char *path = (const char *)l->data;
			hv_recent_add(path); /* move it to top of list */
			if (OpenFileInWindow(win, path, NULL, 0, TRUE, FALSE, FALSE) == NULL)
			{
				ASSERT(recent_list);
				g_free(recent_list->data);
				recent_list = g_slist_delete_link(recent_list, recent_list);
				RecentUpdate(win);
			}
			return;
		}
		sel--;
	}
}

/*** ---------------------------------------------------------------------- ***/

void hv_recent_add(const char *path)
{
	GSList *l, *last;
	int count;
	
	last = NULL;
	for (l = recent_list, count = 0; l; l = l->next, count++)
	{
		const char *oldpath = (const char *)l->data;
		if (filename_cmp(path, oldpath) == 0)
		{
			recent_list = g_slist_remove_link(recent_list, l);
			recent_list = g_slist_prepend(recent_list, l->data);
			g_slist_free_1(l);
			return;
		}
		last = l;
	}
	if (count >= MAX_RECENT && last)
	{
		g_free(last->data);
		recent_list = g_slist_delete_link(recent_list, last);
	}
	recent_list = g_slist_prepend(recent_list, g_strdup(path));
}

/*** ---------------------------------------------------------------------- ***/

void RecentInit(void)
{
	int i;
	char *name;
	gboolean found;
	Profile *profile = gl_profile.profile;
	char *path;
	
	g_slist_free_full(recent_list, g_free);
	recent_list = NULL;
	i = 0;
	for (;;)
	{
		if (i >= MAX_RECENT)
			break;
		name = g_strdup_printf("recent-%d", i);
		path = NULL;
		found = Profile_ReadString(profile, "Recent", name, &path);
		g_free(name);
		if (!found)
			break;
		recent_list = g_slist_append(recent_list, path);
		i++;
	}
}

/*** ---------------------------------------------------------------------- ***/

void RecentExit(void)
{
	g_slist_free_full(recent_list, g_free);
	recent_list = NULL;
}

/*** ---------------------------------------------------------------------- ***/

void RecentSaveToDisk(void)
{
	int i;
	char *name;
	gboolean done;
	Profile *profile = gl_profile.profile;
	GSList *l;
	
	i = 0;
	do
	{
		name = g_strdup_printf("recent-%d", i);
		done = Profile_DeleteKey(profile, "Recent", name);
		g_free(name);
		i++;
	} while (done);
	i = 0;
	for (l = recent_list; l; l = l->next)
	{
		name = g_strdup_printf("recent-%d", i);
		Profile_WriteString(profile, "Recent", name, (const char *)l->data);
		g_free(name);
		i++;
	}
}
