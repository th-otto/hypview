#include "hypdefs.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "stat_.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#include <errno.h>
#include "hypdebug.h"
#include "hv_vers.h"


#if defined(G_OS_WIN32) || defined(G_PLATFORM_WIN32)
#define RESOURCES_PROFILE_DIR "HypView"
#elif defined(G_OS_TOS)
#else
#define RESOURCES_PROFILE_DIR ".hypview"
#endif
#define RESOURCES_GLOBAL_FILENAME "hypview.ini"
#define RESOURCES_IDENTIFIER "HypView"

#if defined(G_OS_TOS)
#define EXT_TTP ".ttp"
#define EXT_TOS ".tos"
#define EXT_PRG ".prg"
#define VIEWER_DEFAULT "ZVIEW"
#define PRINTER_DEFAULT "ILIST"
#else
#define EXT_TTP ""
#define EXT_TOS ""
#define EXT_PRG ""
#endif
#ifdef G_OS_UNIX
#define VIEWER_DEFAULT "xdg-open"
#define PRINTER_DEFAULT "lpr"
#endif
#ifdef G_OS_WIN32
#define VIEWER_DEFAULT "shell open"
#define PRINTER_DEFAULT "shell print"
#endif
#ifndef VIEWER_DEFAULT
#define VIEWER_DEFAULT "edit"
#endif
#ifndef PRINTER_DEFAULT
#define PRINTER_DEFAULT "print"
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

HypProfile gl_profile;

static char *get_profile_dir(const char *basename)
{
#ifdef RESOURCES_PROFILE_DIR
	return g_build_filename(x_get_user_appdata(), RESOURCES_PROFILE_DIR, basename, NULL);
#else
	return g_build_filename(x_get_user_appdata(), basename, NULL);
#endif
}

/*** ---------------------------------------------------------------------- ***/

static char *get_profile_name(void)
{
#ifdef __TOS__
	static gboolean cfg_in_home = TRUE;
	char *filename;
	const char *basename = RESOURCES_GLOBAL_FILENAME;
	struct stat st;
	
#if 0
	if (!_app)
		cfg_in_home = FALSE;
#endif
	filename = NULL;
	if (cfg_in_home)
	{
		filename = g_build_filename(x_get_home_dir(), "defaults", basename, NULL);
		if (rpl_stat(filename, &st) != 0)
		{
			g_free(filename);
			filename = g_build_filename(x_get_home_dir(), basename, NULL);
			if (rpl_stat(filename, &st) != 0)
			{
				g_free(filename);
				filename = NULL;
			}
		}
	}
#if 0
	if (filename == NULL)
	{
		if (rpl_stat(basename, &st) == 0)
		{
			filename = g_strdup(basename);
			cfg_in_home = FALSE;
		}
	}
#endif
	if (filename == NULL)
	{
		char root[3];
		
		strcpy(root, "*:");
		root[0] = GetBootDrive();
		filename = g_build_filename(root, basename, NULL);
		if (rpl_stat(filename, &st) != 0 && _app)
		{
			g_free(filename);
			filename = NULL;
		} else
		{
			cfg_in_home = FALSE;
		}
	}
	if (filename != NULL)
		return filename;
	
#endif
	return get_profile_dir(RESOURCES_GLOBAL_FILENAME);
}

/*** ---------------------------------------------------------------------- ***/

static void subst_var(char **str, const char *name, const char *value)
{
	char *p;
	size_t namelen;
	size_t vallen;
	char *p1, *p2, *res;
	
	if (value == NULL || empty(*str))
		return;
	namelen = strlen(name);
	if ((p = strstr(*str, name)) != NULL && (p[namelen] == '\0' || !isalnum(p[namelen])))
	{
		vallen = strlen(value);
		if (vallen > 0 && G_IS_DIR_SEPARATOR(p[namelen]) && G_IS_DIR_SEPARATOR(value[vallen]))
			namelen++;
		p1 = g_strndup(*str, p - *str);
		p2 = g_strdup(p + namelen);
		res = g_strconcat(p1, value, p2, NULL);
		g_free(p2);
		g_free(p1);
		g_free(*str);
		*str = res;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void unsubst_var(char **str, const char *name, const char *value)
{
	char *p;
	size_t vallen;
	const char *p2;
	char *res;
	
	if (value == NULL || empty(*str))
		return;
	p = *str;
	vallen = strlen(value);
	p2 = p + vallen;
	if (filename_ncmp(p, value, vallen) == 0 && (*p2 == '\0' || G_IS_DIR_SEPARATOR(*p2)))
	{
		if (*p2 != '\0')
			p2++;
		res = g_strconcat(name, "/", p2, NULL);
		g_free(*str);
		convslash(res);
		*str = res;
	}
}

/*** ---------------------------------------------------------------------- ***/

char *path_subst(const char *path)
{
	char *filename = g_strdup(path);

	x_subst_homedir(&filename);
	subst_var(&filename, "$REF", gl_profile.general.all_ref);
	subst_var(&filename, "$STOOL", gl_profile.tools.stool_path);
	subst_var(&filename, "$BINDIR", gl_profile.general.bindir);
	subst_var(&filename, "$CATALOG", gl_profile.viewer.catalog_file);
	subst_var(&filename, "$HYPFOLD", gl_profile.general.hypfold);
#ifdef RESOURCES_PROFILE_DIR
	{
		char *tmp = g_build_filename(x_get_user_appdata(), RESOURCES_PROFILE_DIR, NULL);
		subst_var(&filename, "$APPDATA", tmp);
		g_free(tmp);
	}
#else
	subst_var(&filename, "$APPDATA", x_get_user_appdata());
#endif
	subst_var(&filename, "$HOME", x_get_home_dir());
	x_subst_homedir(&filename);
#ifdef G_OS_TOS
	if (filename && filename[0] == '*' && filename[1] == ':' && G_IS_DIR_SEPARATOR(filename[2]))
		 filename[0] = GetBootDrive();
#endif
	convslash(filename);
	return filename;
}

/*** ---------------------------------------------------------------------- ***/

char *path_unsubst(const char *path, gboolean subst_hypfold)
{
	char *filename = g_strdup(path);

	convslash(filename);
	unsubst_var(&filename, "$HOME", x_get_home_dir());
#ifdef RESOURCES_PROFILE_DIR
	{
		char *tmp = g_strconcat("$HOME", "/", RESOURCES_PROFILE_DIR, NULL);
		convslash(tmp);
		unsubst_var(&filename, "$APPDATA", tmp);
		g_free(tmp);
	}
#else
	unsubst_var(&filename, "$APPDATA", "$HOME");
#endif
	if (subst_hypfold)
		unsubst_var(&filename, "$HYPFOLD", "$APPDATA/guides");

	x_unsubst_homedir(&filename);
#ifdef G_OS_TOS
	if (filename && isupper(filename[0]) == GetBootDrive() && filename[1] == ':' && G_IS_DIR_SEPARATOR(filename[2]))
		 filename[0] = '*';
#endif
	convslash(filename);
	return filename;
}

/*** ---------------------------------------------------------------------- ***/

void HypProfile_Load(gboolean save_if_new)
{
	char *fname;
	Profile *profile;
	
	fname = get_profile_name();
	profile = gl_profile.profile = Profile_Load(fname, RESOURCES_IDENTIFIER);
	g_free(fname);
	
	if (Profile_ReadString(profile, "PATH", "BINDIR", &fname))
	{
		g_free(gl_profile.general.bindir);
		gl_profile.general.bindir = fname;
	} else
	{
		gl_profile.general.bindir = g_get_package_bindir();
	}

#define setdefault(act) \
	act, \
	gl_profile.changed = TRUE
	
	if (!Profile_ReadString(profile, "PATH", "HYPFOLD", &gl_profile.general.hypfold))
	{
#ifdef G_OS_TOS
		setdefault(gl_profile.general.hypfold = g_strdup("*:\\GUIDES"));
#else
		setdefault(gl_profile.general.hypfold = g_strdup("$APPDATA/guides"));
#endif
	}
	if (!Profile_ReadString(profile, "PATH", "Pathlist", &gl_profile.general.path_list))
	{
#ifdef G_OS_TOS
		setdefault(gl_profile.general.path_list = g_strdup("$HYPFOLD;*:\\GEMSYS\\GUIDES"));
#else
		setdefault(gl_profile.general.path_list = g_strdup("$HYPFOLD"));
#endif
	}
	if (g_getenv("REF"))
		gl_profile.general.all_ref = g_strdup(g_getenv("REF"));
	else if (!Profile_ReadString(profile, "PATH", "REF", &gl_profile.general.all_ref))
		setdefault(gl_profile.general.all_ref = g_strdup("$HYPFOLD/all.ref"));
	if (!Profile_ReadString(profile, "PATH", "HYPFIND", &gl_profile.general.hypfind_path))
		setdefault(gl_profile.general.hypfind_path = g_strdup("$BINDIR/hypfind" EXT_TTP));
	if (!Profile_ReadString(profile, "PATH", "HCP", &gl_profile.general.hcp_path))
		setdefault(gl_profile.general.hcp_path = g_strdup("$BINDIR/hcp" EXT_TTP));
	
	if (!Profile_ReadString(profile, "HypView", "DEFAULT", &gl_profile.viewer.default_file))
		setdefault(gl_profile.viewer.default_file = g_strdup("$HYPFOLD/hypview.hyp"));
	if (!Profile_ReadString(profile, "HypView", "CATALOG", &gl_profile.viewer.catalog_file))
		setdefault(gl_profile.viewer.catalog_file = g_strdup("$HYPFOLD/catalog.hyp"));
	if (!Profile_ReadString(profile, "HypView", "PRINTER", &gl_profile.viewer.printer))
		setdefault(gl_profile.viewer.printer = g_strdup(PRINTER_DEFAULT));
	if (!Profile_ReadString(profile, "HypView", "EXTVIEW", &gl_profile.viewer.extview))
		setdefault(gl_profile.viewer.extview = g_strdup(VIEWER_DEFAULT));
	if (!Profile_ReadString(profile, "HypView", "SKIN", &gl_profile.viewer.skin_path))
		{}
	if (!Profile_ReadString(profile, "HypView", "LASTFILE", &gl_profile.viewer.last_file))
		gl_profile.viewer.last_file = NULL;
	if (!Profile_ReadInt(profile, "HypView", "STARTUP", &gl_profile.viewer.startup))
		setdefault(gl_profile.viewer.startup = 1);
	
	if (!Profile_ReadInt(profile, "HypView", "WINSIZE.X", &gl_profile.viewer.win_x))
		setdefault(gl_profile.viewer.win_x = 0);
	if (!Profile_ReadInt(profile, "HypView", "WINSIZE.Y", &gl_profile.viewer.win_y))
		setdefault(gl_profile.viewer.win_y = 0);
	if (!Profile_ReadInt(profile, "HypView", "WINSIZE.W", &gl_profile.viewer.win_w))
		setdefault(gl_profile.viewer.win_w = 0);
	if (!Profile_ReadInt(profile, "HypView", "WINSIZE.H", &gl_profile.viewer.win_h))
		setdefault(gl_profile.viewer.win_h = 0);
	if (!Profile_ReadBool(profile, "HypView", "WINADJUST", &gl_profile.viewer.adjust_winsize))
		setdefault(gl_profile.viewer.adjust_winsize = FALSE);
	if (!Profile_ReadInt(profile, "HypView", "TXTXOFFSET", &gl_profile.viewer.text_xoffset))
		setdefault(gl_profile.viewer.text_xoffset = 8);
	if (!Profile_ReadInt(profile, "HypView", "TXTYOFFSET", &gl_profile.viewer.text_yoffset))
		setdefault(gl_profile.viewer.text_yoffset = 6);
#ifdef WITH_GUI_GEM
	if (!Profile_ReadInt(profile, "HypView", "FONT.ID", &gl_profile.viewer.font_id))
		setdefault(gl_profile.viewer.font_id = 0);
	if (!Profile_ReadInt(profile, "HypView", "FONT.SIZE", &gl_profile.viewer.font_pt))
		setdefault(gl_profile.viewer.font_pt = 0);
	if (!Profile_ReadInt(profile, "HypView", "XFONT.ID", &gl_profile.viewer.xfont_id))
		setdefault(gl_profile.viewer.xfont_id = 0);
	if (!Profile_ReadInt(profile, "HypView", "XFONT.SIZE", &gl_profile.viewer.xfont_pt))
		setdefault(gl_profile.viewer.xfont_pt = gl_profile.viewer.font_pt);
#endif
#ifdef WITH_GUI_GTK
	if (!Profile_ReadString(profile, "HypView", "FONT", &gl_profile.viewer.font_name))
		setdefault(gl_profile.viewer.font_name = g_strdup("Sans Serif 12"));
	if (!Profile_ReadString(profile, "HypView", "XFONT", &gl_profile.viewer.xfont_name))
		setdefault(gl_profile.viewer.xfont_name = g_strdup("Courier New 12"));
#endif
#ifdef WITH_GUI_WIN32
	if (!Profile_ReadString(profile, "HypView", "FONT", &gl_profile.viewer.font_name))
		setdefault(gl_profile.viewer.font_name = g_strdup("Arial,,120"));
	if (!Profile_ReadString(profile, "HypView", "XFONT", &gl_profile.viewer.xfont_name))
		setdefault(gl_profile.viewer.xfont_name = g_strdup("Courier New,,120"));
#endif
	if (!Profile_ReadBool(profile, "HypView", "USE_XFONT", &gl_profile.viewer.use_xfont))
		setdefault(gl_profile.viewer.use_xfont = FALSE);

	if (!Profile_ReadString(profile, "Output", "OUTPUT_DIR", &gl_profile.output.output_dir))
		gl_profile.output.output_dir = NULL;
	{
		char *s;
		if (!Profile_ReadString(profile, "Output", "OUTPUT_CHARSET", &s))
			s = NULL;
		if (s == NULL || strcmp(s, "system") == 0)
			gl_profile.output.output_charset = HYP_CHARSET_NONE;
		else
			gl_profile.output.output_charset = hyp_charset_from_name(s);
		g_free(s);
	}
	if (!Profile_ReadBool(profile, "Output", "bracket_links", &gl_profile.output.bracket_links))
		setdefault(gl_profile.output.bracket_links = FALSE);
	if (!Profile_ReadBool(profile, "Output", "all_links", &gl_profile.output.all_links))
		setdefault(gl_profile.output.all_links = FALSE);
	if (!Profile_ReadBool(profile, "Output", "output_index", &gl_profile.output.output_index))
		setdefault(gl_profile.output.output_index = FALSE);

#if 0
	if (!Profile_ReadString(profile, "Colors", "COLOR0", &gl_profile.colors.color[G_WHITE]))
		setdefault(gl_profile.colors.color[G_WHITE] = g_strdup("#ffffff"));
	if (!Profile_ReadString(profile, "Colors", "COLOR1", &gl_profile.colors.color[G_BLACK]))
		setdefault(gl_profile.colors.color[G_BLACK] = g_strdup("#000000"));
	if (!Profile_ReadString(profile, "Colors", "COLOR2", &gl_profile.colors.color[G_RED]))
		setdefault(gl_profile.colors.color[G_RED] = g_strdup("#ff0000"));
	if (!Profile_ReadString(profile, "Colors", "COLOR3", &gl_profile.colors.color[G_GREEN]))
		setdefault(gl_profile.colors.color[G_GREEN] = g_strdup("#00ff00"));
	if (!Profile_ReadString(profile, "Colors", "COLOR4", &gl_profile.colors.color[G_BLUE]))
		setdefault(gl_profile.colors.color[G_BLUE] = g_strdup("#0000ff"));
	if (!Profile_ReadString(profile, "Colors", "COLOR5", &gl_profile.colors.color[G_CYAN]))
		setdefault(gl_profile.colors.color[G_CYAN] = g_strdup("#00ffff"));
	if (!Profile_ReadString(profile, "Colors", "COLOR6", &gl_profile.colors.color[G_YELLOW]))
		setdefault(gl_profile.colors.color[G_YELLOW] = g_strdup("#ffff00"));
	if (!Profile_ReadString(profile, "Colors", "COLOR7", &gl_profile.colors.color[G_MAGENTA]))
		setdefault(gl_profile.colors.color[G_MAGENTA] = g_strdup("#ff00ff"));
	if (!Profile_ReadString(profile, "Colors", "COLOR8", &gl_profile.colors.color[G_LWHITE]))
		setdefault(gl_profile.colors.color[G_LWHITE] = g_strdup("#cccccc"));
	if (!Profile_ReadString(profile, "Colors", "COLOR9", &gl_profile.colors.color[G_LBLACK]))
		setdefault(gl_profile.colors.color[G_LBLACK] = g_strdup("#888888"));
	if (!Profile_ReadString(profile, "Colors", "COLOR10", &gl_profile.colors.color[G_LRED]))
		setdefault(gl_profile.colors.color[G_LRED] = g_strdup("#880000"));
	if (!Profile_ReadString(profile, "Colors", "COLOR11", &gl_profile.colors.color[G_LGREEN]))
		setdefault(gl_profile.colors.color[G_LGREEN] = g_strdup("#008800"));
	if (!Profile_ReadString(profile, "Colors", "COLOR12", &gl_profile.colors.color[G_LBLUE]))
		setdefault(gl_profile.colors.color[G_LBLUE] = g_strdup("#000088"));
	if (!Profile_ReadString(profile, "Colors", "COLOR13", &gl_profile.colors.color[G_LCYAN]))
		setdefault(gl_profile.colors.color[G_LCYAN] = g_strdup("#008888"));
	if (!Profile_ReadString(profile, "Colors", "COLOR14", &gl_profile.colors.color[G_LYELLOW]))
		setdefault(gl_profile.colors.color[G_LYELLOW] = g_strdup("#888800"));
	if (!Profile_ReadString(profile, "Colors", "COLOR15", &gl_profile.colors.color[G_LMAGENTA]))
		setdefault(gl_profile.colors.color[G_LMAGENTA] = g_strdup("#880088"));
#endif

	if (!Profile_ReadBool(profile, "HypView", "TRANSPARENT_PICS", &gl_profile.viewer.transparent_pics))
		setdefault(gl_profile.viewer.transparent_pics = TRUE);
	if (!Profile_ReadBool(profile, "HypView", "SCALE_BITMAPS", &gl_profile.viewer.scale_bitmaps))
		setdefault(gl_profile.viewer.scale_bitmaps = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "EXPAND_SPACES", &gl_profile.viewer.expand_spaces))
		setdefault(gl_profile.viewer.expand_spaces = TRUE);
	if (!Profile_ReadInt(profile, "HypView", "BIN_COLUMNS", &gl_profile.viewer.binary_columns))
		setdefault(gl_profile.viewer.binary_columns = 76);
	if (!Profile_ReadInt(profile, "HypView", "TABSIZE", &gl_profile.viewer.ascii_tab_size))
		setdefault(gl_profile.viewer.ascii_tab_size = 4);
	if (!Profile_ReadInt(profile, "HypView", "ASCII_BREAK", &gl_profile.viewer.ascii_break_len))
		setdefault(gl_profile.viewer.ascii_break_len = 127);
	if (!Profile_ReadInt(profile, "HypView", "VA_START_NEWWIN", &gl_profile.viewer.va_start_newwin))
		setdefault(gl_profile.viewer.va_start_newwin = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "ALINK_NEWWIN", &gl_profile.viewer.alink_newwin))
		setdefault(gl_profile.viewer.alink_newwin = TRUE);
	if (!Profile_ReadBool(profile, "HypView", "CHECK_TIME", &gl_profile.viewer.check_time))
		setdefault(gl_profile.viewer.check_time = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "INTELLIGENT_FULLER", &gl_profile.viewer.intelligent_fuller))
		setdefault(gl_profile.viewer.intelligent_fuller = TRUE);
	if (!Profile_ReadBool(profile, "HypView", "CLIPBRD_NEW_WINDOW", &gl_profile.viewer.clipbrd_new_window))
		setdefault(gl_profile.viewer.clipbrd_new_window = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "AV_WINDOW_CYCLE", &gl_profile.viewer.av_window_cycle))
		setdefault(gl_profile.viewer.av_window_cycle = FALSE);
	if (!Profile_ReadString(profile, "HypView", "MARKFILE", &gl_profile.viewer.marker_path))
	{
#ifdef RESOURCES_PROFILE_DIR
		setdefault(gl_profile.viewer.marker_path = g_strdup("$APPDATA/marks.dat"));
#else
		setdefault(gl_profile.viewer.marker_path = g_strdup("$HYPFOLD/marks.dat"));
#endif
	}
	if (!Profile_ReadBool(profile, "HypView", "MARKFILE_SAVE_ASK", &gl_profile.viewer.marken_save_ask))
		setdefault(gl_profile.viewer.marken_save_ask = TRUE);
	if (!Profile_ReadBool(profile, "HypView", "REFONLY", &gl_profile.viewer.refonly))
		setdefault(gl_profile.viewer.refonly = TRUE);
	if (!Profile_ReadBool(profile, "HypView", "RIGHTBACK", &gl_profile.viewer.rightback))
		setdefault(gl_profile.viewer.rightback = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "BACKWIND", &gl_profile.viewer.backwind))
		setdefault(gl_profile.viewer.backwind = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "ARROWPATCH", &gl_profile.viewer.arrowpatch))
		setdefault(gl_profile.viewer.arrowpatch = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "NOREFBOX", &gl_profile.viewer.norefbox))
		setdefault(gl_profile.viewer.norefbox = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "DETAIL_INFO", &gl_profile.viewer.detail_info))
		setdefault(gl_profile.viewer.detail_info = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "find_casesensitive", &gl_profile.viewer.find_casesensitive))
		setdefault(gl_profile.viewer.find_casesensitive = FALSE);
	if (!Profile_ReadBool(profile, "HypView", "find_word", &gl_profile.viewer.find_word))
		setdefault(gl_profile.viewer.find_word = FALSE);
#ifdef WITH_GUI_GEM
	if (!Profile_ReadString(profile, "HypView", "APPLNAME", &gl_profile.viewer.applname))
		setdefault(gl_profile.viewer.applname = g_strdup("ST-GUIDE"));
#endif

	if (!Profile_ReadString(profile, "Colors", "background", &gl_profile.colors.background))
		setdefault(gl_profile.colors.background = g_strdup("#ffffff"));
	if (!Profile_ReadString(profile, "Colors", "text", &gl_profile.colors.text))
		setdefault(gl_profile.colors.text = g_strdup("#000000"));
	if (!Profile_ReadString(profile, "Colors", "link", &gl_profile.colors.link))
		setdefault(gl_profile.colors.link = g_strdup("#0000ff"));
	if (!Profile_ReadInt(profile, "Colors", "link_effect", &gl_profile.colors.link_effect))
		setdefault(gl_profile.colors.link_effect = HYP_TXT_BOLD | HYP_TXT_UNDERLINED);
	if (!Profile_ReadString(profile, "Colors", "popup", &gl_profile.colors.popup))
		setdefault(gl_profile.colors.popup = g_strdup("#00ff00"));
	if (!Profile_ReadString(profile, "Colors", "xref", &gl_profile.colors.xref))
		setdefault(gl_profile.colors.xref = g_strdup("#ff0000"));
	if (!Profile_ReadString(profile, "Colors", "system", &gl_profile.colors.system))
		setdefault(gl_profile.colors.system = g_strdup("#ff00ff"));
	if (!Profile_ReadString(profile, "Colors", "rx", &gl_profile.colors.rx))
		setdefault(gl_profile.colors.rx = g_strdup("#ff00ff"));
	if (!Profile_ReadString(profile, "Colors", "rxs", &gl_profile.colors.rxs))
		setdefault(gl_profile.colors.rxs = g_strdup("#ff00ff"));
	if (!Profile_ReadString(profile, "Colors", "quit", &gl_profile.colors.quit))
		setdefault(gl_profile.colors.quit = g_strdup("#ff0000"));
	if (!Profile_ReadString(profile, "Colors", "close", &gl_profile.colors.close))
		setdefault(gl_profile.colors.close = g_strdup("#ff0000"));
	if (!Profile_ReadString(profile, "Colors", "ghosted", &gl_profile.colors.ghosted))
		setdefault(gl_profile.colors.ghosted = g_strdup("#cccccc"));

	if (!Profile_ReadString(profile, "HCP", "Options", &gl_profile.hcp.options))
		setdefault(gl_profile.hcp.options = g_strdup(""));

	if (!Profile_ReadString(profile, "Remarker", "REMARKER", &gl_profile.remarker.path))
		setdefault(gl_profile.remarker.path = g_strdup("*:/remarker/remarker" EXT_PRG));
	if (!Profile_ReadBool(profile, "Remarker", "RunOnStartup", &gl_profile.remarker.run_on_startup))
		setdefault(gl_profile.remarker.run_on_startup = FALSE);
	
	if (!Profile_ReadString(profile, "HypTree", "Options", &gl_profile.hyptree.options))
		setdefault(gl_profile.hyptree.options = g_strdup(""));
	if (!Profile_ReadInt(profile, "HypTree", "WINSIZE.X", &gl_profile.hyptree.win_x))
		setdefault(gl_profile.hyptree.win_x = 40);
	if (!Profile_ReadInt(profile, "HypTree", "WINSIZE.Y", &gl_profile.hyptree.win_y))
		setdefault(gl_profile.hyptree.win_y = 40);
	if (!Profile_ReadInt(profile, "HypTree", "WINSIZE.W", &gl_profile.hyptree.win_w))
		setdefault(gl_profile.hyptree.win_w = 100);
	if (!Profile_ReadInt(profile, "HypTree", "WINSIZE.H", &gl_profile.hyptree.win_h))
		setdefault(gl_profile.hyptree.win_h = 200);
	if (!Profile_ReadBool(profile, "HypTree", "OPENALL", &gl_profile.hyptree.openall))
		setdefault(gl_profile.hyptree.openall = TRUE);
	if (!Profile_ReadBool(profile, "HypTree", "MACLIKE", &gl_profile.hyptree.maclike))
		setdefault(gl_profile.hyptree.maclike = FALSE);
	if (!Profile_ReadString(profile, "HypTree", "CALLSTG", &gl_profile.hyptree.stg_start))
		setdefault(gl_profile.hyptree.stg_start = g_strdup("-s1 %p %s"));
	if (!Profile_ReadString(profile, "HypTree", "STOPSTG", &gl_profile.hyptree.stg_stop))
		setdefault(gl_profile.hyptree.stg_stop = g_strdup("-s0"));
	if (!Profile_ReadBool(profile, "HypTree", "UseQuotes", &gl_profile.hyptree.usequotes))
		setdefault(gl_profile.hyptree.usequotes = TRUE);
	if (!Profile_ReadInt(profile, "HypTree", "Debug", &gl_profile.hyptree.debug))
		setdefault(gl_profile.hyptree.debug = 0);

	if (!Profile_ReadBool(profile, "KatMaker", "ShortFilenames", &gl_profile.katmaker.short_filenames))
		setdefault(gl_profile.katmaker.short_filenames = TRUE);
	if (!Profile_ReadBool(profile, "KatMaker", "LowercaseFilenames", &gl_profile.katmaker.lower_filenames))
		setdefault(gl_profile.katmaker.lower_filenames = TRUE);

	if (!Profile_ReadString(profile, "RefCheck", "Options", &gl_profile.refcheck.options))
		setdefault(gl_profile.refcheck.options = g_strdup("-a -d -s"));
	if (!Profile_ReadString(profile, "RefCheck", "Pathlist", &gl_profile.refcheck.path_list) || empty(gl_profile.refcheck.path_list))
	{
		g_free(gl_profile.refcheck.path_list);
		setdefault(gl_profile.refcheck.path_list = g_strdup(gl_profile.general.path_list));
	}
	
	if (!Profile_ReadString(profile, "HypFind", "Database", &gl_profile.hypfind.database))
		setdefault(gl_profile.hypfind.database = g_strdup(_("HypFind Hit List")));
	if (!Profile_ReadString(profile, "HypFind", "Subject", &gl_profile.hypfind.subject))
		setdefault(gl_profile.hypfind.subject = g_strdup(_("Personal")));
	if (!Profile_ReadString(profile, "HypFind", "Title", &gl_profile.hypfind.title))
		setdefault(gl_profile.hypfind.title = g_strdup(_("Hit List: ")));
	if (!Profile_ReadString(profile, "HypFind", "Wordchars", &gl_profile.hypfind.wordchars))
		setdefault(gl_profile.hypfind.wordchars = g_strdup("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"));

	if (!Profile_ReadString(profile, "TOOLS", "STOOL", &gl_profile.tools.stool_path))
		setdefault(gl_profile.tools.stool_path = g_strdup("$BINDIR/stool" EXT_TOS));

#undef setdefault

	gl_profile.viewer.ascii_break_len = min(LINE_BUF - 1, max(0, gl_profile.viewer.ascii_break_len));
	
	if (gl_profile.changed && save_if_new)
		HypProfile_Save(FALSE);
}

/*** ---------------------------------------------------------------------- ***/

void HypProfile_SetChanged(void)
{
	Profile *profile = gl_profile.profile;
	
	if (profile != NULL)
		gl_profile.changed = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean HypProfile_Save(gboolean report_error)
{
	Profile *profile = gl_profile.profile;
	
	if (profile == NULL)
		return TRUE;
	if (!gl_profile.changed)
	{
		return TRUE;
	} else
	{
	}
	if (Profile_IsNew(profile))
	{
#ifdef HAVE_MKDIR
		char *dir = get_profile_dir(NULL);
		int ret = mkdir(dir, 0700);
		g_free(dir);
		if (ret != 0)
		{
			if (errno != EEXIST)
			{
				if (report_error)
					hyp_utf8_fprintf(stderr, _("%s: could not create directory %s: %s\n"), gl_program_name, dir, hyp_utf8_strerror(errno));
				return FALSE;
			}
		}
#endif
	}

	/* bindir not written; default is to locate it based on program name */
	Profile_WriteString(profile, "PATH", "HYPFOLD", gl_profile.general.hypfold);
	Profile_WriteString(profile, "PATH", "Pathlist", gl_profile.general.path_list);
	Profile_WriteString(profile, "PATH", "REF", gl_profile.general.all_ref);
	Profile_WriteString(profile, "PATH", "HYPFIND", gl_profile.general.hypfind_path);
	Profile_WriteString(profile, "PATH", "HCP", gl_profile.general.hcp_path);
	
	Profile_WriteString(profile, "HypView", "DEFAULT", gl_profile.viewer.default_file);
	Profile_WriteString(profile, "HypView", "CATALOG", gl_profile.viewer.catalog_file);
	Profile_WriteString(profile, "HypView", "PRINTER", gl_profile.viewer.printer);
	Profile_WriteString(profile, "HypView", "EXTVIEW", gl_profile.viewer.extview);
	Profile_WriteString(profile, "HypView", "LASTFILE", gl_profile.viewer.last_file);
	Profile_WriteInt(profile, "HypView", "STARTUP", gl_profile.viewer.startup);
	Profile_WriteInt(profile, "HypView", "WINSIZE.X", gl_profile.viewer.win_x);
	Profile_WriteInt(profile, "HypView", "WINSIZE.Y", gl_profile.viewer.win_y);
	Profile_WriteInt(profile, "HypView", "WINSIZE.W", gl_profile.viewer.win_w);
	Profile_WriteInt(profile, "HypView", "WINSIZE.H", gl_profile.viewer.win_h);
	Profile_WriteString(profile, "HypView", "SKIN", gl_profile.viewer.skin_path);
	Profile_WriteBool(profile, "HypView", "WINADJUST", gl_profile.viewer.adjust_winsize);
	Profile_WriteInt(profile, "HypView", "TXTXOFFSET", gl_profile.viewer.text_xoffset);
	Profile_WriteInt(profile, "HypView", "TXTYOFFSET", gl_profile.viewer.text_yoffset);
#ifdef WITH_GUI_GEM
	Profile_WriteInt(profile, "HypView", "FONT.ID", gl_profile.viewer.font_id);
	Profile_WriteInt(profile, "HypView", "FONT.SIZE", gl_profile.viewer.font_pt);
	Profile_WriteInt(profile, "HypView", "XFONT.ID", gl_profile.viewer.xfont_id);
	Profile_WriteInt(profile, "HypView", "XFONT.SIZE", gl_profile.viewer.xfont_pt);
#else
	Profile_WriteString(profile, "HypView", "FONT", gl_profile.viewer.font_name);
	Profile_WriteString(profile, "HypView", "XFONT", gl_profile.viewer.xfont_name);
#endif
	Profile_WriteBool(profile, "HypView", "USE_XFONT", gl_profile.viewer.use_xfont);
	Profile_WriteBool(profile, "HypView", "TRANSPARENT_PICS", gl_profile.viewer.transparent_pics);
	Profile_WriteBool(profile, "HypView", "SCALE_BITMAPS", gl_profile.viewer.scale_bitmaps);
	Profile_WriteBool(profile, "HypView", "EXPAND_SPACES", gl_profile.viewer.expand_spaces);
	Profile_WriteInt(profile, "HypView", "BIN_COLUMNS", gl_profile.viewer.binary_columns);
	Profile_WriteInt(profile, "HypView", "TABSIZE", gl_profile.viewer.ascii_tab_size);
	Profile_WriteInt(profile, "HypView", "ASCII_BREAK", gl_profile.viewer.ascii_break_len);
	Profile_WriteInt(profile, "HypView", "VA_START_NEWWIN", gl_profile.viewer.va_start_newwin);
	Profile_WriteBool(profile, "HypView", "ALINK_NEWWIN", gl_profile.viewer.alink_newwin);
	Profile_WriteBool(profile, "HypView", "CHECK_TIME", gl_profile.viewer.check_time);
	Profile_WriteBool(profile, "HypView", "INTELLIGENT_FULLER", gl_profile.viewer.intelligent_fuller);
	Profile_WriteBool(profile, "HypView", "CLIPBRD_NEW_WINDOW", gl_profile.viewer.clipbrd_new_window);
	Profile_WriteBool(profile, "HypView", "AV_WINDOW_CYCLE", gl_profile.viewer.av_window_cycle);
	Profile_WriteString(profile, "HypView", "MARKFILE", gl_profile.viewer.marker_path);
	Profile_WriteBool(profile, "HypView", "MARKFILE_SAVE_ASK", gl_profile.viewer.marken_save_ask);
	Profile_WriteBool(profile, "HypView", "REFONLY", gl_profile.viewer.refonly);
	Profile_WriteBool(profile, "HypView", "RIGHTBACK", gl_profile.viewer.rightback);
	Profile_WriteBool(profile, "HypView", "BACKWIND", gl_profile.viewer.backwind);
	Profile_WriteBool(profile, "HypView", "ARROWPATCH", gl_profile.viewer.arrowpatch);
	Profile_WriteBool(profile, "HypView", "NOREFBOX", gl_profile.viewer.norefbox);
	Profile_WriteBool(profile, "HypView", "DETAIL_INFO", gl_profile.viewer.detail_info);
	Profile_WriteBool(profile, "HypView", "find_casesensitive", gl_profile.viewer.find_casesensitive);
	Profile_WriteBool(profile, "HypView", "find_word", gl_profile.viewer.find_word);
#ifdef WITH_GUI_GEM
	Profile_WriteString(profile, "HypView", "APPLNAME", gl_profile.viewer.applname);
#endif

	Profile_WriteString(profile, "Colors", "background", gl_profile.colors.background);
	Profile_WriteString(profile, "Colors", "text", gl_profile.colors.text);
	Profile_WriteString(profile, "Colors", "link", gl_profile.colors.link);
	Profile_WriteInt(profile, "Colors", "link_effect", gl_profile.colors.link_effect);
	Profile_WriteString(profile, "Colors", "popup", gl_profile.colors.popup);
	Profile_WriteString(profile, "Colors", "xref", gl_profile.colors.xref);
	Profile_WriteString(profile, "Colors", "system", gl_profile.colors.system);
	Profile_WriteString(profile, "Colors", "rx", gl_profile.colors.rx);
	Profile_WriteString(profile, "Colors", "rxs", gl_profile.colors.rxs);
	Profile_WriteString(profile, "Colors", "quit", gl_profile.colors.quit);
	Profile_WriteString(profile, "Colors", "close", gl_profile.colors.close);
	Profile_WriteString(profile, "Colors", "ghosted", gl_profile.colors.ghosted);

	Profile_WriteString(profile, "Output", "OUTPUT_DIR", gl_profile.output.output_dir);
	Profile_WriteString(profile, "Output", "OUTPUT_CHARSET", gl_profile.output.output_charset == HYP_CHARSET_NONE ? "system" : hyp_charset_name(gl_profile.output.output_charset));
	Profile_WriteBool(profile, "Output", "bracket_links", gl_profile.output.bracket_links);
	Profile_WriteBool(profile, "Output", "all_links", gl_profile.output.all_links);
	Profile_WriteBool(profile, "Output", "output_index", gl_profile.output.output_index);

	Profile_WriteString(profile, "Remarker", "REMARKER", gl_profile.remarker.path);
	Profile_WriteBool(profile, "Remarker", "RunOnStartup", gl_profile.remarker.run_on_startup);

	Profile_WriteString(profile, "HCP", "Options", gl_profile.hcp.options);

	Profile_WriteString(profile, "HypTree", "Options", gl_profile.hyptree.options);
	Profile_WriteInt(profile, "HypTree", "WINSIZE.X", gl_profile.hyptree.win_x);
	Profile_WriteInt(profile, "HypTree", "WINSIZE.Y", gl_profile.hyptree.win_y);
	Profile_WriteInt(profile, "HypTree", "WINSIZE.W", gl_profile.hyptree.win_w);
	Profile_WriteInt(profile, "HypTree", "WINSIZE.H", gl_profile.hyptree.win_h);
	Profile_WriteBool(profile, "HypTree", "OPENALL", gl_profile.hyptree.openall);
	Profile_WriteBool(profile, "HypTree", "MACLIKE", gl_profile.hyptree.maclike);
	Profile_WriteString(profile, "HypTree", "CALLSTG", gl_profile.hyptree.stg_start);
	Profile_WriteString(profile, "HypTree", "STOPSTG", gl_profile.hyptree.stg_stop);
	Profile_WriteBool(profile, "HypTree", "UseQuotes", gl_profile.hyptree.usequotes);
	Profile_WriteInt(profile, "HypTree", "Debug", gl_profile.hyptree.debug);

	Profile_WriteBool(profile, "KatMaker", "ShortFilenames", gl_profile.katmaker.short_filenames);
	Profile_WriteBool(profile, "KatMaker", "LowercaseFilenames", gl_profile.katmaker.lower_filenames);

	Profile_WriteString(profile, "RefCheck", "Options", gl_profile.refcheck.options);
	Profile_WriteString(profile, "RefCheck", "Pathlist", gl_profile.refcheck.path_list);

	Profile_WriteString(profile, "HypFind", "Database", gl_profile.hypfind.database);
	Profile_WriteString(profile, "HypFind", "Subject", gl_profile.hypfind.subject);
	Profile_WriteString(profile, "HypFind", "Title", gl_profile.hypfind.title);
	Profile_WriteString(profile, "HypFind", "Wordchars", gl_profile.hypfind.wordchars);

	Profile_WriteString(profile, "TOOLS", "STOOL", gl_profile.tools.stool_path);

	return Profile_Save(profile);
}

/*** ---------------------------------------------------------------------- ***/

void HypProfile_Delete(void)
{
	g_freep(&gl_profile.general.bindir);
	g_freep(&gl_profile.general.path_list);
	g_freep(&gl_profile.general.hypfold);
	g_freep(&gl_profile.general.all_ref);
	g_freep(&gl_profile.general.hypfind_path);
	g_freep(&gl_profile.general.hcp_path);
	
	g_freep(&gl_profile.viewer.default_file);
	g_freep(&gl_profile.viewer.catalog_file);
	g_freep(&gl_profile.viewer.last_file);
	g_freep(&gl_profile.viewer.printer);
	g_freep(&gl_profile.viewer.extview);
	g_freep(&gl_profile.viewer.skin_path);
	g_freep(&gl_profile.viewer.marker_path);
#ifndef WITH_GUI_GEM
	g_freep(&gl_profile.viewer.font_name);
	g_freep(&gl_profile.viewer.xfont_name);
#endif
#ifdef WITH_GUI_GEM
	g_freep(&gl_profile.viewer.applname);
#endif

	g_freep(&gl_profile.output.output_dir);

	g_freep(&gl_profile.colors.background);
	g_freep(&gl_profile.colors.text);
	g_freep(&gl_profile.colors.link);
	g_freep(&gl_profile.colors.popup);
	g_freep(&gl_profile.colors.xref);
	g_freep(&gl_profile.colors.system);
	g_freep(&gl_profile.colors.rx);
	g_freep(&gl_profile.colors.rxs);
	g_freep(&gl_profile.colors.quit);
	g_freep(&gl_profile.colors.close);
	g_freep(&gl_profile.colors.ghosted);

	g_freep(&gl_profile.hcp.options);

	g_freep(&gl_profile.hyptree.options);
	g_freep(&gl_profile.hyptree.stg_start);
	g_freep(&gl_profile.hyptree.stg_stop);

	g_freep(&gl_profile.remarker.path);

	g_freep(&gl_profile.refcheck.options);
	g_freep(&gl_profile.refcheck.path_list);

	g_freep(&gl_profile.hypfind.database);
	g_freep(&gl_profile.hypfind.subject);
	g_freep(&gl_profile.hypfind.title);
	g_freep(&gl_profile.hypfind.wordchars);

	g_freep(&gl_profile.tools.stool_path);

	Profile_Delete(gl_profile.profile);
	gl_profile.profile = NULL;
}
