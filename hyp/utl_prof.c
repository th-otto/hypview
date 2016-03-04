#define __PROFILE_IMPLEMENTATION__

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

#define CR 0x0d
#define LF 0x0a


typedef struct _section_name {
	size_t offset;
	size_t len;
} SECTION_NAME;

struct _profile {
	char *filename;
	char *buf;
	gboolean new_file;
	gboolean changed;
	size_t leng;
	size_t alloc_len;
	size_t num_sections;
	size_t alloc_sections;
	size_t section_name_size;
	SECTION_NAME *section_names;
};

#if defined(_DEBUG) || defined(MAINX)
#define PRINTF(x) printf x
#else
#define PRINTF(x)
#endif


G_LOCK_DEFINE_STATIC(x_utils_global);

static	char	*x_tmp_dir = NULL;
static	char	*x_user_name = NULL;
static	char	*x_user_appdata = NULL;
static	char	*x_real_name = NULL;
static	char	*x_home_dir = NULL;


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void x_subst_homedir(char **dirp)
{
	char *dir, *newdir;
	
	if ((dir = *dirp) != NULL && *dir == '~' && G_IS_DIR_SEPARATOR(dir[1]))
	{
		newdir = g_build_filename(x_get_home_dir(), dir + 2, NULL);
		g_free(*dirp);
		*dirp = newdir;
	}
}

/*** ---------------------------------------------------------------------- ***/

void x_unsubst_homedir(char **dirp)
{
	char *dir, *newdir;
	const char *home = x_get_home_dir();
	size_t len = strlen(home);
	
	if ((dir = *dirp) != NULL && filename_ncmp(dir, home, len) == 0 && (G_IS_DIR_SEPARATOR(dir[len]) || dir[len] == '\0'))
	{
		newdir = g_build_filename("~", dir + len, NULL);
		g_free(*dirp);
		*dirp = newdir;
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * like strtod(), but always run in "C" locale
 */
double c_strtod(const char *s, const char **end)
{
	double value;
	
	value = g_ascii_strtodouble(s, end);
	return value;
}

/*** ---------------------------------------------------------------------- ***/

char *c_dtostr(double value)
{
	char buf[G_ASCII_DTOSTR_BUF_SIZE];
	
	return g_strdup(g_ascii_dtostr(buf, (int)sizeof(buf), value));
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if defined(G_OS_WIN32) || defined(G_PLATFORM_WIN32)

#include "windows_.h"
#include <shellapi.h>
#include <shlobj.h>
#include <lmcons.h>

char *get_special_folder(int csidl)
{
	union
	{
		char c[MAX_PATH + 1];
		wchar_t wc[MAX_PATH + 1];
	}
	path;
	HRESULT hr;
	LPITEMIDLIST pidl = NULL;
	BOOL b;
	char *retval = NULL;

	hr = SHGetSpecialFolderLocation(NULL, csidl|CSIDL_FLAG_DONT_VERIFY, &pidl);
	if (hr == S_OK)
	{
		b = SHGetPathFromIDListW(pidl, path.wc);
		if (b)
			retval = hyp_wchar_to_utf8(path.wc, STR0TERM);
#ifdef G_OS_WIN32
		CoTaskMemFree(pidl);
#endif
	}
	return retval;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#ifdef G_OS_WIN32

/* Return a directory to be used to store temporary files. This is the
 * value of the TMPDIR, TMP or TEMP environment variables (they are
 * checked in that order). If none of those exist, use P_tmpdir from
 * stdio.h.  If that isn't defined, return "/tmp" on POSIXly systems,
 * and C:\ on Windows.
 */

const char *x_get_tmp_dir(void)
{
	G_LOCK(x_utils_global);
	if (!x_tmp_dir)
	{
		x_tmp_dir = g_strdup(g_getenv("TMPDIR"));
		if (!x_tmp_dir)
			x_tmp_dir = g_strdup(g_getenv("TMP"));
		if (!x_tmp_dir)
			x_tmp_dir = g_strdup(g_getenv("TEMP"));

#ifdef P_tmpdir
		if (!x_tmp_dir)
		{
			size_t k;

			x_tmp_dir = g_strdup(P_tmpdir);
			k = strlen(x_tmp_dir);
			if (k > 1 && G_IS_DIR_SEPARATOR(x_tmp_dir[k - 1]))
				x_tmp_dir[k - 1] = '\0';
		}
#endif

		if (!x_tmp_dir)
		{
			x_tmp_dir = g_strdup("\\");
		}
	}
	G_UNLOCK(x_utils_global);

	return x_tmp_dir;
}

/*** ---------------------------------------------------------------------- ***/

static void x_init_home_dir(void)
{
	struct stat st;
	size_t len;
	
	/* We check $HOME first for Win32, though it is a last resort for Unix
	 * where we prefer the results of getpwuid().
	 */
	x_home_dir = g_strdup(g_secure_getenv("HOME"));

	/* Only believe HOME if it is an absolute path and exists */
	if (x_home_dir)
	{
		if (!(g_path_is_absolute(x_home_dir) && hyp_utf8_stat(x_home_dir, &st) == 0 && S_ISDIR(st.st_mode)))
		{
			g_freep(&x_home_dir);
		}
	}

	/* In case HOME is Unix-style (it happens), convert it to
	 * Windows style.
	 */
	if (x_home_dir)
	{
		char *p;

		while ((p = strchr(x_home_dir, '/')) != NULL)
			*p = '\\';
	}

	if (!x_home_dir)
	{
		/* USERPROFILE is probably the closest equivalent to $HOME? */
		if (g_secure_getenv("USERPROFILE") != NULL)
			x_home_dir = g_strdup(g_secure_getenv("USERPROFILE"));
		if (x_home_dir)
		{
			if (!(g_path_is_absolute(x_home_dir) && hyp_utf8_stat(x_home_dir, &st) == 0 && S_ISDIR(st.st_mode)))
			{
				g_freep(&x_home_dir);
			}
		}
	}

	if (!x_home_dir)
		x_home_dir = get_special_folder(CSIDL_PROFILE);

	if (!x_home_dir)
	{
		/* At least at some time, HOMEDRIVE and HOMEPATH were used
		 * to point to the home directory, I think. But on Windows
		 * 2000 HOMEDRIVE seems to be equal to SYSTEMDRIVE, and
		 * HOMEPATH is its root "\"?
		 */
		if (g_secure_getenv("HOMEDRIVE") != NULL && g_secure_getenv("HOMEPATH") != NULL)
			x_home_dir = g_strconcat(g_secure_getenv("HOMEDRIVE"), g_secure_getenv("HOMEPATH"), NULL);
	}
	if (empty(x_home_dir))
	{
		g_free(x_home_dir);
		x_home_dir = g_strdup(".");
	}
	len = strlen(x_home_dir);
	if (G_IS_DIR_SEPARATOR(x_home_dir[len - 1]))
		x_home_dir[--len] = '\0';
	
	
	{
		DWORD len = UNLEN + 1;
		wchar_t buffer[UNLEN + 1];

		if (GetUserNameW(buffer, &len))
		{
			x_user_name = hyp_wchar_to_utf8(buffer, STR0TERM);
			x_real_name = g_strdup(x_user_name);
		}
	}

	if (!x_user_appdata)
		x_user_appdata = get_special_folder(CSIDL_APPDATA);

	if (!x_user_name)
		x_user_name = g_strdup("nobody");
	if (!x_real_name)
		x_real_name = g_strdup("Unknown");
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_home_dir(void)
{
	G_LOCK(x_utils_global);
	if (!x_home_dir)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);
	
	return x_home_dir;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_user_name(void)
{
	G_LOCK(x_utils_global);
	if (!x_user_name)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_user_name;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_user_appdata(void)
{
	G_LOCK(x_utils_global);
	if (!x_user_appdata)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_user_appdata;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_real_name(void)
{
	G_LOCK(x_utils_global);
	if (!x_real_name)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_real_name;
}

#endif /* G_OS_WIN32 */

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#ifdef G_OS_UNIX

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <limits.h>

const char *x_get_tmp_dir(void)
{
	G_LOCK(x_utils_global);
	if (!x_tmp_dir)
	{
		x_tmp_dir = g_strdup(g_getenv("TMPDIR"));
		if (!x_tmp_dir)
			x_tmp_dir = g_strdup(g_getenv("TMP"));
		if (!x_tmp_dir)
			x_tmp_dir = g_strdup(g_getenv("TEMP"));

#ifdef P_tmpdir
		if (!x_tmp_dir)
		{
			size_t k;

			x_tmp_dir = g_strdup(P_tmpdir);
			k = strlen(x_tmp_dir);
			if (k > 1 && G_IS_DIR_SEPARATOR(x_tmp_dir[k - 1]))
				x_tmp_dir[k - 1] = '\0';
		}
#endif

		if (!x_tmp_dir)
		{
			x_tmp_dir = g_strdup("/tmp");
		}
	}
	G_UNLOCK(x_utils_global);

	return x_tmp_dir;
}

/*** ---------------------------------------------------------------------- ***/

static void x_init_home_dir(void)
{
	size_t len;
	
#if defined(HAVE_PWD_H)
	{
		struct passwd *pw = NULL;

		pw = getpwuid(getuid());
		if (pw != NULL)
		{
			x_user_name = g_strdup(pw->pw_name);

			if (pw->pw_gecos && *pw->pw_gecos != '\0')
			{
				char **gecos_fields;
				char **name_parts;

				/* split the gecos field and substitute '&' */
				gecos_fields = g_strsplit(pw->pw_gecos, ",", 0);
				name_parts = g_strsplit(gecos_fields[0], "&", 0);
				pw->pw_name[0] = g_ascii_toupper(pw->pw_name[0]);
				x_real_name = g_strjoinv(pw->pw_name, name_parts);
				g_strfreev(gecos_fields);
				g_strfreev(name_parts);
			}

			if (!x_home_dir)
				x_home_dir = g_strdup(pw->pw_dir);
		}
		endpwent();
	}

#endif /* !HAVE_PWD_H */

	if (!x_home_dir)
		x_home_dir = g_strdup(g_secure_getenv("HOME"));
	if (empty(x_home_dir))
	{
		g_free(x_home_dir);
		x_home_dir = g_strdup(".");
	}
	len = strlen(x_home_dir);
	if (G_IS_DIR_SEPARATOR(x_home_dir[len - 1]))
		x_home_dir[--len] = '\0';
	
	if (!x_user_name)
		x_user_name = g_strdup("nobody");
	if (!x_real_name)
		x_real_name = g_strdup("Unknown");
	if (!x_user_appdata)
		x_user_appdata = g_strdup(x_home_dir);
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_home_dir(void)
{
	G_LOCK(x_utils_global);
	if (!x_home_dir)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);
	
	return x_home_dir;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_user_name(void)
{
	G_LOCK(x_utils_global);
	if (!x_user_name)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_user_name;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_user_appdata(void)
{
	G_LOCK(x_utils_global);
	if (!x_user_appdata)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_user_appdata;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_real_name(void)
{
	G_LOCK(x_utils_global);
	if (!x_real_name)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_real_name;
}

#endif /* G_OS_UNIX */

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#ifdef G_OS_TOS

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <limits.h>

const char *x_get_tmp_dir(void)
{
	G_LOCK(x_utils_global);
	if (!x_tmp_dir)
	{
		x_tmp_dir = g_strdup(g_getenv("TMPDIR"));
		if (!x_tmp_dir)
			x_tmp_dir = g_strdup(g_getenv("TMP"));
		if (!x_tmp_dir)
			x_tmp_dir = g_strdup(g_getenv("TEMP"));

#ifdef P_tmpdir
		if (!x_tmp_dir)
		{
			size_t k;

			x_tmp_dir = g_strdup(P_tmpdir);
			k = strlen(x_tmp_dir);
			if (k > 1 && G_IS_DIR_SEPARATOR(x_tmp_dir[k - 1]))
				x_tmp_dir[k - 1] = '\0';
		}
#endif

		if (!x_tmp_dir)
		{
			x_tmp_dir = g_strdup("/tmp");
		}
	}
	G_UNLOCK(x_utils_global);

	return x_tmp_dir;
}

/*** ---------------------------------------------------------------------- ***/

static void x_init_home_dir(void)
{
	size_t len;
	
#if defined(HAVE_PWD_H)
	{
		struct passwd *pw = NULL;

		pw = getpwuid(getuid());
		if (pw != NULL)
		{
			x_user_name = g_strdup(pw->pw_name);

			if (pw->pw_gecos && *pw->pw_gecos != '\0')
			{
				char **gecos_fields;
				char **name_parts;

				/* split the gecos field and substitute '&' */
				gecos_fields = g_strsplit(pw->pw_gecos, ",", 0);
				name_parts = g_strsplit(gecos_fields[0], "&", 0);
				pw->pw_name[0] = g_ascii_toupper(pw->pw_name[0]);
				x_real_name = g_strjoinv(pw->pw_name, name_parts);
				g_strfreev(gecos_fields);
				g_strfreev(name_parts);
			}

			if (!x_home_dir)
				x_home_dir = g_strdup(pw->pw_dir);
		}
		endpwent();
	}

#endif /* !HAVE_PWD_H */

	if (!x_home_dir)
		x_home_dir = g_strdup(g_secure_getenv("HOME"));
	if (empty(x_home_dir))
	{
		g_free(x_home_dir);
		x_home_dir = g_strdup(".");
	}
	len = strlen(x_home_dir);
	if (G_IS_DIR_SEPARATOR(x_home_dir[len - 1]))
		x_home_dir[--len] = '\0';
	
	if (!x_user_name)
		x_user_name = g_strdup("nobody");
	if (!x_real_name)
		x_real_name = g_strdup("Unknown");
	if (!x_user_appdata)
		x_user_appdata = g_strdup(x_home_dir);
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_home_dir(void)
{
	G_LOCK(x_utils_global);
	if (!x_home_dir)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);
	
	return x_home_dir;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_user_name(void)
{
	G_LOCK(x_utils_global);
	if (!x_user_name)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_user_name;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_user_appdata(void)
{
	G_LOCK(x_utils_global);
	if (!x_user_appdata)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_user_appdata;
}

/*** ---------------------------------------------------------------------- ***/

const char *x_get_real_name(void)
{
	G_LOCK(x_utils_global);
	if (!x_real_name)
		x_init_home_dir();
	G_UNLOCK(x_utils_global);

	return x_real_name;
}

#endif /* G_OS_TOS */

/*** ---------------------------------------------------------------------- ***/

void x_free_resources(void)
{
	G_LOCK(x_utils_global);
	
	g_freep(&x_tmp_dir);
	g_freep(&x_home_dir);
	g_freep(&x_user_name);
	g_freep(&x_real_name);
	g_freep(&x_user_appdata);
	
	G_UNLOCK(x_utils_global);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean GetSectionStarts(Profile *profile)
{
	char *use;
	char *end;
	size_t lengsection;
	size_t i;
	size_t offset;

	profile->section_name_size = 0;
	use = profile->buf;
	end = use + profile->leng;
	profile->num_sections = 0;
	while (use != end)
	{
		if (*use == '[')
		{
			/*
			 * section found
			 */
			lengsection = 0;
			use++;
			while (use != end && *use != ']' && *use != CR && *use != LF)
			{
				use++;
				lengsection++;
			}
			if (lengsection != 0)
			{
				profile->section_name_size += lengsection + 1;
				profile->num_sections++;
			}
		}
		while (use != end && *use != CR && *use != LF)
			use++;
		if (use != end && *use == CR)
			use++;
		if (use != end && *use == LF)
			use++;
	}
	profile->alloc_sections = profile->num_sections;
	if (profile->num_sections == 0)
		return TRUE;

	profile->section_names = g_new(SECTION_NAME, profile->alloc_sections);
	if (profile->section_names == NULL)
		return FALSE;
	use = profile->buf;
	i = 0;
	while (use != end)
	{
		if (*use == '[')
		{
			/*
			 * section found
			 */
			lengsection = 0;
			use++;
			offset = use - profile->buf;
			while (use != end && *use != ']' && *use != CR && *use != LF)
			{
				use++;
				lengsection++;
			}
			if (lengsection != 0)
			{
				profile->section_names[i].offset = offset;
				profile->section_names[i].len = lengsection;
				i++;
			}
		}
		while (use != end && *use != CR && *use != LF)
			use++;
		if (use != end && *use == CR)
			use++;
		if (use != end && *use == LF)
			use++;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void UpdateOffsets(Profile *profile, size_t pos, ssize_t diff)
{
	size_t i;

	for (i = 0; i < profile->num_sections; i++)
	{
		if (profile->section_names[i].offset >= pos)
			profile->section_names[i].offset += diff;
	}
}

/*** ---------------------------------------------------------------------- ***/

#if 0

static void DbgProfileSections(Profile *profile, const char *where)
{
	size_t i;

	PRINTF(("%s: %u sections found, strsize = %u in %s\n", where, profile->num_sections, profile->section_name_size, profile->filename));
	for (i = 0; i < profile->num_sections; i++)
		PRINTF(("%3u: %8u %5u %.*s\n", i, profile->section_names[i].offset, profile->section_names[i].len, (int)profile->section_names[i].len, profile->buf + profile->section_names[i].offset));
}

#else

#define DbgProfileSections(profile, where)

#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

Profile *Profile_New(const char *filename)
{
	Profile *profile;

	profile = g_new(Profile, 1);
	if (profile != NULL)
	{
		profile->filename = g_strdup(filename);
		profile->leng = 0;
		profile->alloc_len = 0;
		profile->buf = NULL;
		profile->num_sections = 0;
		profile->alloc_sections = 0;
		profile->section_name_size = 0;
		profile->section_names = NULL;
		profile->new_file = TRUE;
		profile->changed = FALSE;
		if (profile->filename == NULL && filename != NULL)
		{
			Profile_Delete(profile);
			profile = NULL;
		}
	}
	return profile;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_Read(Profile *profile)
{
	if (profile != NULL && profile->filename != NULL)
	{
		FILE *fp;

		fp = fopen(profile->filename, "r");
		if (fp != NULL)
		{
			fseek(fp, 0l, SEEK_END);
			profile->alloc_len = ftell(fp);
			fseek(fp, 0l, SEEK_SET);
			profile->buf = g_new(char, (profile->alloc_len + 1));
			if (profile->buf == NULL)
				return FALSE;
			profile->leng = fread(profile->buf, 1, profile->alloc_len, fp);
			profile->buf[profile->leng] = '\0';
			if (GetSectionStarts(profile) == FALSE)
				return FALSE;
			DbgProfileSections(profile, "Profile_Load");
			fclose(fp);
			profile->new_file = FALSE;
		} else
		{
			const char *whoami = x_get_user_name();
			time_t now = time((time_t *) 0);
			char timbuf[40];
			const char *timestr = ctime_r(&now, timbuf);
			char *nl = strchr(timestr, '\n');
			
			if (nl)
				*nl = 0;
			profile->buf = g_strdup_printf(
					"# %s Preferences File\n"
					"# Written by %s %s for %s on %s.\n"
					"# " HYP_URL "\n"
					"\n",
					RESOURCES_IDENTIFIER, gl_program_name, gl_program_version, whoami, timestr);
			profile->leng = profile->alloc_len = strlen(profile->buf);
		}
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

Profile *Profile_Load(const char *filename)
{
	Profile *profile;

	if ((profile = Profile_New(filename)) != NULL)
	{
		Profile_Read(profile);
	}
	return profile;
}

/*** ---------------------------------------------------------------------- ***/

void Profile_Delete(Profile *profile)
{
	if (profile != NULL)
	{
		if (profile->buf != NULL)
			g_free(profile->buf);
		if (profile->section_names != NULL)
			g_free(profile->section_names);
		if (profile->filename != NULL)
			g_free(profile->filename);
		g_free(profile);
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_Save(Profile *profile)
{
	gboolean retV;

	retV = FALSE;
	if (profile != NULL && profile->filename != NULL)
	{
		FILE *fp;

		fp = fopen(profile->filename, "w");
		if (fp != NULL)
		{
			if (profile->buf != NULL && profile->leng != 0)
			{
				if (fwrite(profile->buf, sizeof(*profile->buf), profile->leng, fp) == profile->leng)
				{
					retV = TRUE;
					profile->changed = FALSE;
				}
			}
			fclose(fp);
		}
	}
	return retV;
}

/*** ---------------------------------------------------------------------- ***/

const char *Profile_GetFilename(Profile *profile)
{
	if (profile != NULL)
		return profile->filename;
	return NULL;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *Profile_GetSectionNames(Profile *profile, unsigned long *pNumSections)
{
	char *sections;

	if (profile == NULL)
		return NULL;

	{
		char *use;
		char *dest;
		size_t len;
		size_t i;

		sections = (char *)g_malloc((profile->section_name_size + 1) * sizeof(*sections));
		if (sections != NULL)
		{
			dest = sections;
			for (i = 0; i < profile->num_sections; i++)
			{
				use = profile->buf + profile->section_names[i].offset;
				len = profile->section_names[i].len;

				while (len != 0)
				{
					*dest++ = *use++;
					len--;
				}
				*dest++ = '\0';
			}
			*dest = '\0';
		}
		if (pNumSections != NULL)
			*pNumSections = profile->num_sections;
	}

	return sections;
}

/*** ---------------------------------------------------------------------- ***/

char *Profile_GetKeyNames(Profile *profile, const char *section, unsigned long *pNumKeys)
{
	char *keys;
	unsigned long num_keys;

	if (profile == NULL)
		return NULL;

	{
		char *use;
		char *end;
		char *start;
		char *dest;
		size_t len;
		size_t key_name_size;
		size_t i;
		size_t lengsection;

		use = profile->buf;
		end = use + profile->leng;

		lengsection = strlen(section);
		use = end;
		for (i = 0; i < profile->num_sections; i++)
		{
			if (lengsection == profile->section_names[i].len &&
				g_ascii_strncasecmp(section, profile->buf + profile->section_names[i].offset, lengsection) == 0)
			{
				use = profile->buf + profile->section_names[i].offset + 2;
				break;
			}
		}

		start = use;

		num_keys = 0;
		key_name_size = 0;
		while (use != end)
		{
			while (use != end && *use != CR && *use != LF)
				use++;
			if (use != end && *use == CR)
				use++;
			if (use != end && *use == LF)
				use++;

			if (use == end || *use == '[')
			{
				/*
				 * end of file or start of new section reached
				 */
				break;
			}

			if (*use != ';')
			{
				len = 0;
				while (use != end && *use != '=' && *use != CR && *use != LF)
				{
					len++;
					use++;
				}
				if (use != end && *use == '=' && len != 0)
				{
					key_name_size += len + 1;
					num_keys++;
				}
			}
		}

		keys = (char *)g_malloc((key_name_size + 1) * sizeof(*keys));
		if (keys != NULL)
		{
			use = start;
			dest = keys;
			while (use != end)
			{
				while (use != end && *use != CR && *use != LF)
					use++;
				if (use != end && *use == CR)
					use++;
				if (use != end && *use == LF)
					use++;

				if (use == end || *use == '[')
				{
					/*
					 * end of file or start of new section reached
					 */
					break;
				}

				if (*use != ';')
				{
					len = 0;
					while (use != end && *use != '=' && *use != CR && *use != LF)
					{
						len++;
						*dest++ = *use++;
					}
					if (use != end && *use == '=' && len != 0)
					{
						*dest++ = '\0';
					}
				}
			}
			*dest = '\0';
		}

	}

	if (pNumKeys != NULL)
		*pNumKeys = num_keys;

	return keys;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean Profile_GetValue(Profile *profile, const char *section, const char *key, char *desti, size_t destiSize)
{
	gboolean found;

	if (profile == NULL)
		return FALSE;

	{
		char *use;
		char *end;
		size_t lengsection;
		size_t lenkey;
		size_t i;

		use = profile->buf;
		end = use + profile->leng;

		lengsection = strlen(section);
		lenkey = strlen(key);

		found = FALSE;
		use = end;
		for (i = 0; i < profile->num_sections; i++)
		{
			if (lengsection == profile->section_names[i].len &&
				g_ascii_strncasecmp(section, profile->buf + profile->section_names[i].offset, lengsection) == 0)
			{
				use = profile->buf + profile->section_names[i].offset + 2;
				break;
			}
		}

		while (use != end)
		{
			while (use != end && *use != CR && *use != LF)
				use++;
			if (use != end && *use == CR)
				use++;
			if (use != end && *use == LF)
				use++;

			if (use == end || *use == '[')
			{
				/*
				 * end of file or start of new section reached;
				 * key not found
				 */
				break;
			}

			if (g_ascii_strncasecmp(use, key, lenkey) == 0 &&
				use[lenkey] == '=')
			{
				char delim;
				char *start;

				delim = '\0';
				use += lenkey + 1;
				if (use != end && *use == '"')
				{
					delim = '"';
					use++;
				}
				destiSize--;
				start = use;
				while (use != end && destiSize != 0 && *use != CR && *use != LF && *use != delim)
				{
					*desti++ = *use++;
					destiSize--;
				}
				if (delim == '\0')
				{
					while (desti != start && (desti[-1] == ' ' || desti[-1] == '\t'))
						desti--;
				}
				*desti = '\0';
				found = TRUE;
				break;
			}
		}
	}

	return found;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean Profile_GetLongValue(Profile *profile, const char *section, const char *key, char **dest, char *delim)
{
	gboolean found;

	*dest = NULL;
	*delim = '\0';
	
	if (profile == NULL)
		return FALSE;

	{
		char *use;
		char *end;
		size_t lengsection;
		size_t lenkey;
		size_t i;

		use = profile->buf;
		end = use + profile->leng;

		lengsection = strlen(section);
		lenkey = strlen(key);

		found = FALSE;
		use = end;
		for (i = 0; i < profile->num_sections; i++)
		{
			if (lengsection == profile->section_names[i].len &&
				g_ascii_strncasecmp(section, profile->buf + profile->section_names[i].offset, lengsection) == 0)
			{
				use = profile->buf + profile->section_names[i].offset + 2;
				break;
			}
		}

		while (use != end)
		{
			while (use != end && *use != CR && *use != LF)
				use++;
			if (use != end && *use == CR)
				use++;
			if (use != end && *use == LF)
				use++;

			if (use == end || *use == '[')
			{
				/*
				 * end of file or start of new section reached;
				 * key not found
				 */
				break;
			}

			if (g_ascii_strncasecmp(use, key, lenkey) == 0 &&
				use[lenkey] == '=')
			{
				char *start;
				size_t len;
				char *str;

				use += lenkey + 1;
				if (use != end && *use == '"')
				{
					*delim = '"';
					use++;
				}

				start = use;
				while (use != end && *use != CR && *use != LF && (*use != *delim || use[-1] == '\\'))
					use++;
				if (*delim == '\0')
				{
					while (use != start && (use[-1] == ' ' || use[-1] == '\t'))
						use--;
				}
				len = (size_t)(use - start);
				str = (char *)g_malloc((len + 1) * sizeof(*str));
				*dest = str;
				if (str != NULL)
				{
					while (len != 0)
					{
						*str++ = *start++;
						len--;
					}
					*str = '\0';
				}
				found = TRUE;
				break;
			}
		}
	}

	return found;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean Profile_Realloc(Profile *profile, size_t newsize)
{
	char *newbuf;
	
	newbuf = g_renew(char, profile->buf, (newsize + 1));
	if (newbuf == NULL)
	{
		newbuf = g_new(char, (newsize + 1));
		if (newbuf == NULL)
		{
			return FALSE;
		}
		if (profile->buf != NULL)
		{
			memcpy(newbuf, profile->buf, profile->leng * sizeof(*profile->buf));
			g_free(profile->buf);
		}
	}
	profile->alloc_len = newsize;
	profile->buf = newbuf;
	profile->buf[newsize] = '\0';
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean Profile_PutValue(Profile *profile, const char *section, const char *key, const char *value)
{
	size_t lenval;

	if (profile == NULL || value == NULL)
		return FALSE;

	profile->changed = TRUE;
	lenval = strlen(value);

	{
		gboolean found;
		char *use;
		char *end;
		char *start;
		size_t lengsection;
		size_t lenkey;
		size_t newsize;
		size_t pos;
		size_t oldlen;
		size_t last_start;
		size_t i;

		use = profile->buf;
		end = use + profile->leng;

		lengsection = strlen(section);
		lenkey = strlen(key);

		/*
		 * search for the section
		 */
		found = FALSE;
		for (i = 0; i < profile->num_sections; i++)
		{
			if (lengsection == profile->section_names[i].len &&
				g_ascii_strncasecmp(section, profile->buf + profile->section_names[i].offset, lengsection) == 0)
			{
				use = profile->buf + profile->section_names[i].offset + 2;
				while (use != end && *use != CR && *use != LF)
					use++;
				if (use != end && *use == CR)
					use++;
				if (use != end && *use == LF)
					use++;
				found = TRUE;
				break;
			}
		}
		if (found == FALSE)
		{
			/*
			 * section not found, create it
			 */
			newsize = profile->leng +
				1 +
				1 + lengsection + 1 + 1 +
				lenkey + 1 + lenval + 1;
			if (newsize > profile->alloc_len)
			{
				if (Profile_Realloc(profile, newsize) == FALSE)
				{
					return FALSE;
				}
			}
			use = profile->buf + profile->leng;
			if (profile->leng != 0)
			{
				*use++ = '\n';
				profile->leng++;
			}
			if (profile->num_sections >= profile->alloc_sections)
			{
				SECTION_NAME *new_sections;
				size_t new_alloc;

				new_alloc = profile->alloc_sections + 100;
				new_sections = (SECTION_NAME *)g_realloc(profile->section_names, new_alloc * sizeof(SECTION_NAME));
				if (new_sections == NULL)
					return FALSE;
				profile->alloc_sections = new_alloc;
				profile->section_names = new_sections;
			}
			profile->section_names[profile->num_sections].offset = (use - profile->buf) + 1;
			profile->section_names[profile->num_sections].len = lengsection;
			profile->section_name_size += lengsection + 1;
			profile->num_sections++;
			profile->leng += 1 + lengsection + 1 + 1;
			*use++ = '[';
			while (lengsection != 0)
			{
				*use++ = *section++;
				lengsection--;
			}
			*use++ = ']';
			*use = '\n';
			use[1] = '\0';
			end = profile->buf + profile->leng;
		}

		/*
		 * section found; "use" now points to the line just behind the section name
		 */
		found = FALSE;
		last_start = (size_t)(use - profile->buf);
		while (use != end)
		{
			if (use == end || *use == '[')
			{
				/*
				 * end of section, key not found
				 */
				break;
			}

			if (g_ascii_strncasecmp(use, key, lenkey) == 0 &&
				use[lenkey] == '=')
			{
				/*
				 * key found
				 */
				use += lenkey + 1;
				found = TRUE;
				break;
			}

			while (use != end && *use != CR && *use != LF)
				use++;
			if (use != end && *use == CR)
				use++;
			if (use != end && *use == LF)
				use++;

			if (use == end || (*use != ';' && *use != '['))
				last_start = (size_t)(use - profile->buf);
		}

		if (found == FALSE)
		{
			/*
			 * key not found, create it
			 */
			oldlen = lenkey + 1 + lenval + 1;
			newsize = profile->leng + oldlen;
			/* this is only cosmetic: go back, skipping over empty lines */
			use = profile->buf + last_start;
			pos = (size_t)(use - profile->buf);
			if (newsize > profile->alloc_len)
			{
				if (Profile_Realloc(profile, newsize) == FALSE)
				{
					return FALSE;
				}
				use = profile->buf + pos;
			}
			memmove(use + oldlen, use, (profile->leng - pos) * sizeof(*profile->buf));
			UpdateOffsets(profile, pos, oldlen);
			while (lenkey)
			{
				*use++ = *key++;
				lenkey--;
			}
			*use++ = '=';
			for (pos = 0; pos < lenval; pos++)
			{
				use[pos] = value[pos];
			}
			use[lenval] = '\n';
			profile->leng = newsize;
			end = profile->buf + newsize;
			*end = '\0';
		}

		/*
		 * key found; "use" now points to the value
		 */
		start = use;
		pos = (size_t)(start - profile->buf);
		oldlen = 0;
		while (use != end && *use != CR && *use != LF && *use != '\0')
		{
			oldlen++;
			use++;
		}

		newsize = profile->leng + lenval - oldlen;
		if (oldlen >= lenval)
		{
			memmove(start + lenval, start + oldlen, (profile->leng - pos - oldlen) * sizeof(*profile->buf));
			UpdateOffsets(profile, start + oldlen - profile->buf, lenval - oldlen);
			for (pos = 0; pos < lenval; pos++)
			{
				start[pos] = value[pos];
			}
		} else
		{
			if (newsize > profile->alloc_len)
			{
				if (Profile_Realloc(profile, newsize) == FALSE)
				{
					return FALSE;
				}
				/* end = profile->buf + profile->leng; */
				start = profile->buf + pos;
			}
			memmove(start + lenval, start + oldlen, (profile->leng - pos - oldlen) * sizeof(*profile->buf));
			UpdateOffsets(profile, start + oldlen - profile->buf, lenval - oldlen);
			for (pos = 0; pos < lenval; pos++)
			{
				start[pos] = value[pos];
			}
		}
		profile->leng = newsize;
		profile->buf[profile->leng] = '\0';

		DbgProfileSections(profile, "Profile_PutValue");
	}

	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

gboolean Profile_DeleteKey(Profile *profile, const char *section, const char *key)
{
	gboolean found;

	if (profile == NULL)
		return FALSE;
	profile->changed = TRUE;

	{
		char *use;
		char *end;
		size_t lengsection;
		size_t lenkey;
		size_t pos;
		char *start;
		size_t i;

		use = profile->buf;
		end = use + profile->leng;

		lengsection = strlen(section);
		lenkey = strlen(key);

		found = FALSE;
		use = end;
		for (i = 0; i < profile->num_sections; i++)
		{
			if (lengsection == profile->section_names[i].len &&
				g_ascii_strncasecmp(section, profile->buf + profile->section_names[i].offset, lengsection) == 0)
			{
				use = profile->buf + profile->section_names[i].offset + 2;
				break;
			}
		}

		while (use != end)
		{
			while (use != end && *use != CR && *use != LF)
				use++;
			if (use != end && *use == CR)
				use++;
			if (use != end && *use == LF)
				use++;

			if (use == end || *use == '[')
			{
				break;
			}

			if (g_ascii_strncasecmp(use, key, lenkey) == 0 &&
				use[lenkey] == '=')
			{
				start = use;
				use += lenkey + 1;
				while (use != end && *use != CR && *use != LF && *use != '\0')
					use++;
				if (use != end && *use == CR)
					use++;
				if (use != end && *use == LF)
					use++;
				lenkey = (size_t)(use - start);
				pos = (size_t)(use - profile->buf);
				memmove(start, use, (profile->leng - pos) * sizeof(*profile->buf));
				UpdateOffsets(profile, pos, start - use);
				profile->leng -= lenkey;
				profile->buf[profile->leng] = '\0';
				found = TRUE;
				break;
			}
		}

		DbgProfileSections(profile, "Profile_DeleteKey");
	}

	return found;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void Profile_WriteChar(Profile *profile, const char *section, const char *key, char val)
{
	char buf[2];

	buf[0] = val;
	buf[1] = '\0';
	Profile_PutValue(profile, section, key, buf);
}

/*** ---------------------------------------------------------------------- ***/

void Profile_WriteString(Profile *profile, const char *section, const char *key, const char *str)
{
	size_t len;
	const char *p;
	char *newstr;
	char *dest;

	profile->changed = TRUE;
	if (str != NULL)
	{
		len = 2;
		p = str;
		while (*p != '\0')
		{
			switch (*p)
			{
			case '\n':
				len += 2;
				break;
			case '\t':
				len += 2;
				break;
			case '\\':
				len += 2;
				break;
			case '"':
				len += 2;
				break;
			default:
				len += 1;
				break;
			}
			p++;
		}
		newstr = g_new(char, (len + 1));
		if (newstr != NULL)
		{
			dest = newstr;
			*dest++ = '"';
			p = str;
			while (*p != '\0')
			{
				switch (*p)
				{
				case '\n':
					*dest++ = '\\';
					*dest++ = 'n';
					break;
				case '\t':
					*dest++ = '\\';
					*dest++ = 't';
					break;
				case '"':
					*dest++ = '\\';
					*dest++ = '"';
					break;
				case '\\':
					*dest++ = '\\';
					*dest++ = '\\';
					break;
				default:
					*dest++ = *p;
					break;
				}
				p++;
			}
			*dest++ = '"';
			*dest = '\0';
			Profile_PutValue(profile, section, key, newstr);
			g_free(newstr);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void Profile_WriteByte(Profile *profile, const char *section, const char *key, unsigned char val)
{
	if (profile == NULL)
		return;
	{
		char buf[20];

		sprintf(buf, "%u", val);
		Profile_PutValue(profile, section, key, buf);
	}
}

/*** ---------------------------------------------------------------------- ***/

void Profile_WriteInt(Profile *profile, const char *section, const char *key, int val)
{
	if (profile == NULL)
		return;
	{
		char buf[20];

		sprintf(buf, "%d", val);
		Profile_PutValue(profile, section, key, buf);
	}
}

/*** ---------------------------------------------------------------------- ***/

void Profile_WriteCard(Profile *profile, const char *section, const char *key, unsigned int val)
{
	if (profile == NULL)
		return;
	{
		char buf[20];

		sprintf(buf, "%u", val);
		Profile_PutValue(profile, section, key, buf);
	}
}

/*** ---------------------------------------------------------------------- ***/

void Profile_WriteLong(Profile *profile, const char *section, const char *key, intmax_t val)
{
	if (profile == NULL)
		return;
	{
		char buf[50];

		Profile_PutValue(profile, section, key, xs_imaxtostr(val, buf, TRUE));
	}
}

/*** ---------------------------------------------------------------------- ***/

void Profile_WriteLongCard(Profile *profile, const char *section, const char *key, uintmax_t val)
{
	if (profile == NULL)
		return;
	{
		char buf[50];

		Profile_PutValue(profile, section, key, xs_imaxtostr(val, buf, FALSE));
	}
}

/*** ---------------------------------------------------------------------- ***/

void Profile_WriteBool(Profile *profile, const char *section, const char *key, gboolean val)
{
	if (profile == NULL)
		return;
	{
		char buf[20];

		strcpy(buf, val == -1 ? "undef" : val ? "True" : "False");
		Profile_PutValue(profile, section, key, buf);
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

gboolean Profile_ReadChar(Profile *profile, const char *section, const char *key, char *pval)
{
	char buf[2];
	gboolean found;

	*pval = 0;
	found = Profile_GetValue(profile, section, key, buf, sizeof(buf));
	if (found && buf[0] != '\0')
		*pval = buf[0];
	return found;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_ReadString(Profile *profile, const char *section, const char *key, char **strp)
{
	char *str;
	char *dest;
	const char *p;
	gboolean found;
	char delim;
	
	found = Profile_GetLongValue(profile, section, key, &str, &delim);
	if (found == FALSE || str == NULL)
	{
		str = NULL;
	}
	if (str != NULL)
	{
		dest = str;
		p = str;
		while (*p != '\0')
		{
			if (*p == '\\' && delim != '\0')
			{
				switch (*++p)
				{
				case 'n':
					*dest++ = '\n';
					p++;
					break;
				case 't':
					*dest++ = '\t';
					p++;
					break;
				case '\\':
					*dest++ = '\\';
					p++;
					break;
				case '"':
					*dest++ = '"';
					p++;
					break;
				case '\0':
					break;
				default:
					*dest++ = *p++;
					break;
				}
			} else
			{
				*dest++ = *p++;
			}
		}
		*dest = '\0';
	}
	*strp = str;
	return found;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_ReadByte(Profile *profile, const char *section, const char *key, unsigned char *pval)
{
	gboolean found;
	unsigned int val;

	val = 0;

	if (profile == NULL)
	{
		found = FALSE;
	} else
	{
		char buf[20];

		found = Profile_GetValue(profile, section, key, buf, sizeof(buf));
		if (found)
		{
			val = (unsigned int)strtoul(buf, NULL, 0);
		}
	}
	*pval = val;
	return found;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_ReadInt(Profile *profile, const char *section, const char *key, int *pval)
{
	int val;
	gboolean found;

	val = 0;

	if (profile == NULL)
	{
		found = FALSE;
	} else
	{
		char buf[20];

		found = Profile_GetValue(profile, section, key, buf, sizeof(buf));
		if (found)
		{
			val = (int)strtol(buf, NULL, 0);
		}
	}
	*pval = val;
	return found;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_ReadLong(Profile *profile, const char *section, const char *key, intmax_t *pval)
{
	intmax_t val;
	gboolean found;

	val = 0;

	if (profile == NULL)
	{
		found = FALSE;
	} else
	{
		char buf[20];

		found = Profile_GetValue(profile, section, key, buf, sizeof(buf));
		if (found)
		{
			val = xs_strtoimax(buf, NULL, 0);
		}
	}
	*pval = val;
	return found;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_ReadCard(Profile *profile, const char *section, const char *key, unsigned int *pval)
{
	unsigned int val;
	gboolean found;

	val = 0;

	if (profile == NULL)
	{
		found = FALSE;
    } else
	{
		char buf[20];

		found = Profile_GetValue(profile, section, key, buf, sizeof(buf));
		if (found)
		{
			val = (unsigned int)strtoul(buf, NULL, 0);
		}
	}
	*pval = val;
	return found;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_ReadLongCard(Profile *profile, const char *section, const char *key, uintmax_t *pval)
{
	uintmax_t val;
	gboolean found;

	val = 0;

	if (profile == NULL)
	{
		found = FALSE;
	} else
	{
		char buf[20];

		found = Profile_GetValue(profile, section, key, buf, sizeof(buf));
		if (found)
		{
#if ULONG_MAX > 4294967295UL
			val = strtoull(buf, NULL, 0);
#else
			val = strtoul(buf, NULL, 0);
#endif
		}
	}
	*pval = val;
	return found;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_ReadBool(Profile *profile, const char *section, const char *key, gboolean *pval)
{
	gboolean val;
	gboolean found;

	val = FALSE;

	if (profile == NULL)
	{
		found = FALSE;
	} else
	{
		int x;
		char buf[20];

		found = Profile_GetValue(profile, section, key, buf, sizeof(buf));
		if (found)
		{
			if (g_ascii_strcasecmp(buf, "true") == 0)
				val = TRUE;
			else if (g_ascii_strcasecmp(buf, "false") == 0)
				val = FALSE;
			else if (g_ascii_strcasecmp(buf, "on") == 0)
				val = TRUE;
			else if (g_ascii_strcasecmp(buf, "off") == 0)
				val = FALSE;
			else if (g_ascii_strcasecmp(buf, "yes") == 0)
				val = TRUE;
			else if (g_ascii_strcasecmp(buf, "no") == 0)
				val = FALSE;
			else if (g_ascii_strcasecmp(buf, "undef") == 0)
				val = -1;
			else
			{
#ifdef _DEBUG
				fprintf(stderr, "%s: %s must be boolean, not %s.\n", gl_program_name, key, buf);
#endif
				x = (int)strtol(buf, NULL, 10);
				val = x == -1 ? -1 : x != 0;
			}
		}
	}
	*pval = val;
	return found;
}

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

void HypProfile_Load(void)
{
	char *fname;
	Profile *profile;
	
	fname = get_profile_name();
	profile = gl_profile.profile = Profile_Load(fname);
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
	profile->changed = TRUE
	
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
		setdefault(gl_profile.viewer.refonly = FALSE);
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
	
	if (profile->changed)
		HypProfile_Save(FALSE);
}

/*** ---------------------------------------------------------------------- ***/

void HypProfile_SetChanged(void)
{
	Profile *profile = gl_profile.profile;
	
	if (profile != NULL)
		profile->changed = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean HypProfile_Save(gboolean report_error)
{
	Profile *profile = gl_profile.profile;
	
	if (profile == NULL)
		return TRUE;
	if (!profile->changed)
	{
		return TRUE;
	} else
	{
	}
	if (profile->new_file)
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
