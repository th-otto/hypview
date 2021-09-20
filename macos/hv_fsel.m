#include "hv_defs.h"
#include "hypdebug.h"

char const hypertext_file_filter[] = N_("*.hyp|Hypertext files (*.hyp)\n*.*|All files (*.*)\n");
char const text_file_filter[] = N_("*.txt|Text files (*.txt)\n*.*|All files (*.*)\n");
static char const stg_file_filter[] = N_("*.stg|ST-Guide files (*.stg)\n*.*|All files (*.*)\n");
static char const pdf_file_filter[] = N_("*.pdf|PDF files (*.pdf)\n*.*|All files (*.*)\n");

#define G_SEARCHPATH_SEPARATOR_S ";"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if 0
static wchar_t *hv_file_selector_add_filter(const char *filter)
{
	char **filterlist;
	char *displayname;
	int i;
	wchar_t *wfilter = NULL;
	size_t wlen = 0;
	size_t len;
	wchar_t *wdisplayname;
	wchar_t *wpattern;
	
	if (empty(filter))
		return NULL;
	filterlist = g_strsplit(filter, "\n", 0);
	for (i = 0; filterlist != NULL && filterlist[i] != NULL; i++)
	{
		if (empty(filterlist[i]))
			continue;
		displayname = strchr(filterlist[i], '|');
		if (displayname != NULL)
		{
			*displayname++ = '\0';
			wdisplayname = hyp_utf8_to_wchar(displayname, STR0TERM, NULL);
			wpattern = hyp_utf8_to_wchar(filterlist[i], STR0TERM, NULL);
		} else
		{
			wpattern = hyp_utf8_to_wchar(filterlist[i], STR0TERM, NULL);
			wdisplayname = wpattern;
		}
		len = wcslen(wdisplayname) + 1;
		wfilter = g_renew(wchar_t, wfilter, wlen + len);
		wcscpy(wfilter + wlen, wdisplayname);
		wlen += len;
		len = wcslen(wpattern) + 1;
		wfilter = g_renew(wchar_t, wfilter, wlen + len);
		wcscpy(wfilter + wlen, wpattern);
		wlen += len;
		g_free(wpattern);
		if (wdisplayname != wpattern)
			g_free(wdisplayname);
	}
	wfilter = g_renew(wchar_t, wfilter, wlen + 1);
	wfilter[wlen] = 0;
	g_strfreev(filterlist);
	return wfilter;
}
#endif

/*** ---------------------------------------------------------------------- ***/

static void file_select_done(NSOpenPanel *panel, NSInteger result, char **name, enum choose_file_mode mode, int *presult)
{
	NSArray *filenames;
	NSEnumerator *enumerator;
	id url;
	BOOL didOutput;
	int done = FALSE;
	
	if (result == NSFileHandlingPanelOKButton)
	{
		if (![panel allowsMultipleSelection])
		{
			url = [panel URL];
			if (url)
				filenames = [NSArray arrayWithObject: url];
			else
				filenames = nil;
		} else
		{
			filenames = [panel URLs];
		}
			
		enumerator = [filenames objectEnumerator];
		didOutput = FALSE;
		while ((url = [enumerator nextObject]) != nil)
		{
			const char *str = [[url path] UTF8String];
			if (str && *str)
			{
				g_free(*name);
				*name = g_strdup(str);
				didOutput = TRUE;
			}
		}
	
		if (didOutput)
		{
			done = TRUE;
		}
	}
	*presult = done;
}

/*** ---------------------------------------------------------------------- ***/

gboolean choose_file(WINDOW_DATA *parent, char **name, enum choose_file_mode mode, const char *title, const char *filter)
{
	char *dir;
	char *freeme = NULL;
	NSOpenPanel *panel;
	int presult;
	NSInteger result;

#if 0
	int fd2;

	fd2 = dup(2);
	close(2);
	open("/dev/null", O_RDWR);
#endif
	
	/* filter not implemented */	
	UNUSED(filter);

	if (mode == file_dirsel)
	{
		dir = *name;
	} else
	{
		dir = freeme = hyp_path_get_dirname(*name);
	}

	if (mode == file_save)
		panel = (NSOpenPanel *)([NSSavePanel savePanel]);
	else
		panel = [NSOpenPanel openPanel];
		
	[panel setTitle: [[[NSString alloc] initWithCString: title encoding: NSUTF8StringEncoding] autorelease]];
	[panel setAllowsOtherFileTypes:YES];
	[panel setCanChooseDirectories:mode == file_dirsel];
	[panel setCanChooseFiles:mode != file_dirsel];
	[panel setAllowsMultipleSelection:mode == file_open];
	[panel setDirectoryURL: [NSURL fileURLWithPath: [[[NSString alloc] initWithCString: dir encoding: NSUTF8StringEncoding] autorelease]]];
	[panel setExtensionHidden:NO];
	[panel setTreatsFilePackagesAsDirectories:YES];
	
	presult = -1;
	/* [panel beginWithCompletionHandler: ^(NSInteger result) { file_select_done(panel, result, name, mode, &presult); }]; */
	
	[panel makeKeyAndOrderFront: panel];
	result = [panel runModal];
	file_select_done(panel, result, name, mode, &presult);
	
#if 0
	close(2);
	dup2(2, fd2);
	close(fd2);
#endif
	
	return presult;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win)
{
	char *name;
	char *subst;
	WINDOW_DATA *parent = win;
	char **paths;
	
	if (win)
	{
		DOCUMENT *doc = (DOCUMENT *)win->data;
		name = g_strdup(doc->path);
	} else
	{
		subst = path_subst(gl_profile.general.path_list);
		paths = g_strsplit(subst, G_SEARCHPATH_SEPARATOR_S, 0);
		name = g_build_filename(paths[0], "*.hyp", NULL);
		g_strfreev(paths);
		g_free(subst);
	}
	
	if (choose_file(parent, &name, file_open, _("Open Hypertext..."), _(hypertext_file_filter)))
	{
		hv_recent_add(name);
		win = OpenFileInWindow(win, name, NULL, 0, TRUE, FALSE, FALSE);
	}
	g_free(name);
	return win;
}

/*** ---------------------------------------------------------------------- ***/

char *SelectFileSave(WINDOW_DATA *win, hyp_filetype type)
{
	DOCUMENT *doc = win->data;
	char *filepath;
	WINDOW_DATA *parent = win;
	const char *defext;
	const char *filter;
	const char *title;
	
	switch (type)
	{
	case HYP_FT_ASCII:
	case HYP_FT_CHEADER:
		defext = HYP_EXT_TXT;
		filter = _(text_file_filter);
		title = _("Save ASCII text as");
		break;
	case HYP_FT_STG:
	case HYP_FT_GUIDE:
		defext = HYP_EXT_STG;
		filter = _(stg_file_filter);
		title = _("Recompile to");
		break;
	case HYP_FT_PDF:
#ifdef WITH_PDF
		defext = HYP_EXT_PDF;
		filter = _(pdf_file_filter);
		title = _("Recompile to PDF");
		break;
#else
		return NULL;
#endif
	case HYP_FT_NONE:
	case HYP_FT_UNKNOWN:
	case HYP_FT_LOADERROR:
	case HYP_FT_BINARY:
	case HYP_FT_REF:
	case HYP_FT_RSC:
	case HYP_FT_IMAGE:
	case HYP_FT_HTML:
	case HYP_FT_XML:
	case HYP_FT_HTML_XML:
	case HYP_FT_HYP:
	case HYP_FT_TREEVIEW:
	default:
		return NULL;
	}
	
	if (gl_profile.output.output_dir)
	{
		char *name = replace_ext(hyp_basename(doc->path), NULL, defext);
		filepath = g_build_filename(gl_profile.output.output_dir, name, NULL);
		g_free(name);
	} else
	{
		filepath = replace_ext(doc->path, NULL, defext);
	}
	
	if (choose_file(parent, &filepath, file_save, title, filter))
	{
		g_free(gl_profile.output.output_dir);
		gl_profile.output.output_dir = hyp_path_get_dirname(filepath);
		HypProfile_SetChanged();
	} else
	{
		g_free(filepath);
		filepath = NULL;
	}
	return filepath;
}
