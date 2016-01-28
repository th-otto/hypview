#define GDK_DISABLE_DEPRECATION_WARNINGS
#include "hv_gtk.h"
#include "hypdebug.h"


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

gboolean show_dialog(GtkWidget *parent, const char *type, const char *message, gboolean can_cancel)
{
	char *msg = g_strdup(message);
	char *head;
	GtkWidget *dialog;
	GtkWidget *label = 0;
	GtkWidget *ok = 0;
	GtkWidget *cancel = 0;
	int center = 100;
	int resp;
	
	UNUSED(type);
	while (parent && !gtk_widget_get_window(parent))
		parent = gtk_widget_get_parent(parent);

 	if (!parent /* || !gtk_widget_get_window(parent) */)	/* too early to pop up transient dialogs */
	{
		fprintf(stderr, _("%s: too early for dialog?\n"), gl_program_name);
		fprintf(stderr, "%s: %s\n", gl_program_name, message);
		return FALSE;
	}

	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("message-dialog"));
	
	head = msg;
	while (head)
	{
		char *s = strchr(head, '\n');

		if (s)
			*s = 0;

		{
			label = gtk_label_new(head);
			gtk_label_set_selectable(GTK_LABEL(label), TRUE);
			if (center <= 0)
				gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
			gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, TRUE, TRUE, 0);
			gtk_widget_show(label);
		}

		if (s)
			head = s + 1;
		else
			head = 0;

		center--;
	}

	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	if (can_cancel)
	{
		cancel = gtk_button_new_cancel();
		gtk_widget_set_can_default(cancel, TRUE);
		gtk_widget_show(cancel);
		gtk_dialog_add_action_widget(GTK_DIALOG(dialog), cancel, GTK_RESPONSE_CANCEL);
		g_signal_connect_swapped(G_OBJECT(cancel), "clicked", G_CALLBACK(gtk_widget_destroy), dialog);
	}

	ok = gtk_button_new_ok();
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), ok, GTK_RESPONSE_OK);

	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
	gtk_window_set_title(GTK_WINDOW(dialog), gl_program_name);
	gtk_widget_set_can_default(ok, TRUE);
	gtk_widget_show(ok);
	gtk_widget_grab_focus(ok);

	gtk_widget_show(dialog);

	g_signal_connect_swapped(G_OBJECT(ok), "clicked", G_CALLBACK(gtk_widget_destroy), dialog);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));

	gtk_window_present(GTK_WINDOW(dialog));
	g_free(msg);
	resp = gtk_dialog_run(GTK_DIALOG(dialog));
	return IsResponseOk(resp);
}

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

void hv_recent_add(const char *path)
{
	HYP_DBG(("NYI: recent add"));
	UNUSED(path);
}
