#define __PROFILE_IMPLEMENTATION__

#ifndef __HYPDEFS_H__
#include "hypdefs.h"
#endif
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
#include <limits.h>
#include <math.h>
#ifndef HYP_URL
#include "hv_vers.h"
#endif

#define CR 0x0d
#define LF 0x0a


#if defined(_DEBUG) || defined(MAINX)
#define PRINTF(x) printf x
#else
#define PRINTF(x)
#endif


typedef struct _section_name {
	size_t offset;
	size_t len;
} SECTION_NAME;

struct _profile {
	char *filename;
	char *buf;
	gboolean new_file;
	gboolean changed;
	const char *truestring;
	const char *falsestring;
	size_t leng;
	size_t alloc_len;
	size_t num_sections;
	size_t alloc_sections;
	size_t section_name_size;
	SECTION_NAME *section_names;
};

#ifndef G_LOCK_DEFINE_STATIC
#define G_LOCK_DEFINE_STATIC(x) extern int x
#define G_LOCK(x)
#define G_UNLOCK(x)
#endif

G_LOCK_DEFINE_STATIC(x_utils_global);

static	char	*x_tmp_dir = NULL;
static	char	*x_user_name = NULL;
static	char	*x_user_appdata = NULL;
static	char	*x_real_name = NULL;
static	char	*x_home_dir = NULL;

#undef __set_errno
#undef __clear_errno
#define __set_errno(e) errno = e
#define __clear_errno() __set_errno(0)

#ifndef NAN
# define NAN HUGE_VAL
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

double g_ascii_strtodouble(const char *nptr, const char **endptr)
{
	const char *fail_pos;
	double val;
	const char *decimal_point;
	int decimal_point_len;
	const char *p, *decimal_point_pos;
	const char *end = NULL;				/* Silence gcc */
	int strtod_errno;
	size_t len;
	
	__clear_errno();
	if (nptr == NULL)
	{
		if (endptr)
			*endptr = NULL;
		return 0;
	}
	len = strlen(nptr);
	
	fail_pos = NULL;

	decimal_point = ".";
#ifdef HAVE_LOCALE_H
	{
		struct lconv *locale_data;
		locale_data = localeconv();
		if (locale_data != NULL && locale_data->decimal_point != NULL && locale_data->decimal_point[0] != '\0')
			decimal_point = locale_data->decimal_point;
	}
#endif
	decimal_point_len = (int)strlen(decimal_point);
	
	decimal_point_pos = NULL;
	end = NULL;

	{
		int negative = 0;
		
		p = nptr;
		/* Skip leading space */
		while (len && g_ascii_isspace(*p))
			p++, len--;

		/* Skip leading optional sign */
		if (len && (*p == '+' || *p == '-'))
		{
			negative = *p == '-';
			p++;
			len--;
		}
		if (len >= 3)
		{
			if (toupper(p[0]) == 'N' && toupper(p[1]) == 'A' && toupper(p[2]) == 'N')
			{
				if (endptr)
					*endptr = p + 3;
				return negative ? -NAN : NAN;
			}
			if (toupper(p[0]) == 'I' && toupper(p[1]) == 'N' && toupper(p[2]) == 'F')
			{
				if (endptr)
					*endptr = p + 3;
				return negative ? -HUGE_VAL : HUGE_VAL;
			}
		}
				
		if (len >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
		{
			p += 2;
			len -= 2;
			/* HEX - find the (optional) decimal point */

			while (len && g_ascii_isxdigit(*p))
				p++, len--;

			if (len && *p == '.')
			{
				decimal_point_pos = p++;
				len--;
			}
			
			while (len && g_ascii_isxdigit(*p))
				p++, len--;

			if (len && (*p == 'p' || *p == 'P'))
			{
				p++, len--;
				if (len && (*p == '+' || *p == '-'))
					p++, len--;
				while (len && g_ascii_isdigit(*p))
					p++, len--;
 			}
 			
			end = p;
		} else
		{
			while (len && g_ascii_isdigit(*p))
				p++, len--;

			if (len && *p == '.')
			{
				decimal_point_pos = p++;
				len--;
			}
			while (len && g_ascii_isdigit(*p))
				p++, len--;

			if (len && (*p == 'e' || *p == 'E'))
			{
				p++, len--;
				if (len && (*p == '+' || *p == '-'))
					p++, len--;
				while (len && g_ascii_isdigit(*p))
					p++, len--;
			}
			
			end = p;
		}
		/* For the other cases, we need not convert the decimal point */
	}

	if (decimal_point_pos && (decimal_point[0] != '.' || decimal_point[1] != 0))
	{
		char *copy, *c;
		char *fpos = NULL;
		
		/* We need to convert the '.' to the locale specific decimal point */
		copy = (char *)g_malloc(end - nptr + 1 + decimal_point_len);

		c = copy;
		memcpy(c, nptr, decimal_point_pos - nptr);
		c += decimal_point_pos - nptr;
		memcpy(c, decimal_point, decimal_point_len);
		c += decimal_point_len;
		memcpy(c, decimal_point_pos + 1, end - (decimal_point_pos + 1));
		c += end - (decimal_point_pos + 1);
		*c = 0;

		__clear_errno();
		val = strtod(copy, &fpos);
		strtod_errno = errno;

		if (fpos)
		{
			if (fpos - copy > decimal_point_pos - nptr)
				fail_pos = nptr + (fpos - copy) - (decimal_point_len - 1);
			else
				fail_pos = nptr + (fpos - copy);
		}

		g_free(copy);

	} else
	{
		char *copy;
		char *fpos = NULL;

		copy = (char *)g_malloc(end - nptr + 1);
		memcpy(copy, nptr, end - nptr);
		*(copy + (end - nptr)) = 0;

		__clear_errno();
		val = strtod(copy, &fpos);
		strtod_errno = errno;

		if (fpos)
		{
			fail_pos = nptr + (fpos - copy);
		}

		g_free(copy);
	}

	if (endptr)
		*endptr = fail_pos;

	__set_errno(strtod_errno);

	return val;
}

/*** ---------------------------------------------------------------------- ***/

char *g_ascii_dtostr(char *buffer, int buf_len, double d)
{
	return g_ascii_formatd(buffer, buf_len, "%.17g", d);
}

/*** ---------------------------------------------------------------------- ***/

char *g_ascii_formatd(char *buffer, int buf_len, const char *format, double d)
{
	const char *decimal_point;
	int decimal_point_len;
	char *p;
	int rest_len;

	snprintf(buffer, buf_len, format, d);

	decimal_point = ".";
#ifdef HAVE_LOCALE_H
	{
		struct lconv *locale_data;
		locale_data = localeconv();
		if (locale_data != NULL && locale_data->decimal_point != NULL && locale_data->decimal_point[0] != '\0')
			decimal_point = locale_data->decimal_point;
	}
#endif
	decimal_point_len = (int) strlen(decimal_point);

	if (decimal_point[0] != '.' || decimal_point[1] != 0)
	{
		p = buffer;

		while (g_ascii_isspace(*p))
			p++;

		if (*p == '+' || *p == '-')
			p++;

		while (isdigit((unsigned char) *p))
			p++;

		if (strncmp(p, decimal_point, decimal_point_len) == 0)
		{
			*p = '.';
			p++;
			if (decimal_point_len > 1)
			{
				rest_len = (int) strlen(p + (decimal_point_len - 1));
				memmove(p, p + (decimal_point_len - 1), rest_len);
				p[rest_len] = 0;
			}
		}
	}

	return buffer;
}

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

/*** ---------------------------------------------------------------------- ***/

const char *g_secure_getenv(const char *name)
{
#if defined(HAVE_GETEUID) && defined(HAVE_GETEGID)
	if (getuid() != geteuid() || getgid() != getegid())
		return NULL;
#endif
	return g_getenv(name);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if defined(G_OS_WIN32) || defined(G_PLATFORM_WIN32)

#include "windows_.h"
#include <shellapi.h>
#include <shlobj.h>
#include <lmcons.h>

static char *prf_wchar_to_utf8(const wchar_t *wc)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, wc, -1, NULL, 0, NULL, NULL);
	char *result = g_new(char, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wc, -1, result, len, NULL, NULL);
	return result;
}

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
		{
			retval = prf_wchar_to_utf8(path.wc);
		}
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
			x_user_name = prf_wchar_to_utf8(buffer);
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
		profile->truestring = "True";
		profile->falsestring = "False";
		if (profile->filename == NULL && filename != NULL)
		{
			Profile_Delete(profile);
			profile = NULL;
		}
	}
	return profile;
}

/*** ---------------------------------------------------------------------- ***/

void Profile_SetBoolStrings(Profile *profile, const char *truestring, const char *falsestring)
{
	if (profile == NULL)
		return;
	profile->truestring = truestring;
	profile->falsestring = falsestring;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_IsNew(Profile *profile)
{
	if (profile)
		return profile->new_file;
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_Read(Profile *profile, const char *creator)
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
					"; %s Preferences File\n"
					"; Written by %s %s for %s on %s.\n"
					"; " HYP_URL "\n"
					"\n",
					creator, gl_program_name, gl_program_version, whoami, timestr);
			profile->leng = profile->alloc_len = strlen(profile->buf);
		}
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

Profile *Profile_Load(const char *filename, const char *creator)
{
	Profile *profile;

	if ((profile = Profile_New(filename)) != NULL)
	{
		Profile_Read(profile, creator);
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

gboolean Profile_Dump(Profile *profile, FILE *fp)
{
	gboolean retV;

	retV = FALSE;
	if (profile != NULL)
	{
		if (profile->buf != NULL && profile->leng != 0)
		{
			if (fwrite(profile->buf, sizeof(*profile->buf), profile->leng, fp) == profile->leng &&
				fflush(fp) == 0)
			{
				retV = TRUE;
			}
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

/*** ---------------------------------------------------------------------- ***/

gboolean Profile_SetSection(Profile *profile, const char *section, const char *value)
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
		size_t newsize;
		size_t pos;
		size_t oldlen;
		char *sectstart;
		size_t i;

		use = profile->buf;
		end = use + profile->leng;

		lengsection = strlen(section);

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
				lenval + 1;
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
			*use++ = '\n';
			use[0] = '\0';
			end = profile->buf + profile->leng;
		}

		/*
		 * section found; "use" now points to the line just behind the section name
		 */
		found = FALSE;
		sectstart = use;
		while (use != end)
		{
			if (use == end || *use == '[')
			{
				/*
				 * end of section, key not found
				 */
				break;
			}

			while (use != end && *use != CR && *use != LF)
				use++;
			if (use != end && *use == CR)
				use++;
			if (use != end && *use == LF)
				use++;
		}
		
		start = use;
		pos = (size_t)(start - profile->buf);
		oldlen = use - sectstart;
		
		newsize = profile->leng + lenval - oldlen;
		if (oldlen != 0)
			newsize++;
		if (oldlen >= lenval)
		{
			memmove(start + lenval, start + oldlen, (profile->leng - pos - oldlen) * sizeof(*profile->buf));
			UpdateOffsets(profile, start + oldlen - profile->buf, lenval - oldlen);
			for (pos = 0; pos < lenval; pos++)
			{
				start[pos] = value[pos];
			}
			if (oldlen != 0)
				start[pos] = '\n';
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
			if (oldlen != 0)
				start[pos] = '\n';
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

/*** ---------------------------------------------------------------------- ***/

char **Profile_GetSection(Profile *profile, const char *section)
{
	char *use;
	char *end;
	size_t lengsection;
	size_t i;
	size_t n;
	char *linestart;
	char **lines = NULL;
	
	if (profile == NULL)
		return NULL;

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
	
	if (use == end)
		return NULL;
	
	/*
	 * skip to end of line of section name
	 */
	while (use != end && *use != CR && *use != LF)
		use++;
	if (use != end && *use == CR)
		use++;
	if (use != end && *use == LF)
		use++;
	
	n = 0;
	while (use != end)
	{
		linestart = use;
		
		while (use != end && *use != CR && *use != LF)
			use++;
		lines = g_renew(char *, lines, n + 2);
		if (lines == NULL)
			return NULL;
		lines[n] = g_strndup(linestart, use - linestart);
		n++;
		
		if (use != end && *use == CR)
			use++;
		if (use != end && *use == LF)
			use++;

		if (use == end || *use == '[')
		{
			/*
			 * end of file or start of new section reached;
			 * we are done
			 */
			break;
		}
	}
	if (n != 0)
		lines[n] = NULL;
	
	return lines;
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

void Profile_WriteStringUnquoted(Profile *profile, const char *section, const char *key, const char *str)
{
	Profile_PutValue(profile, section, key, str);
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

		sprintf(buf, "0x%x", val);
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
		const char *buf;

		buf = val == -1 ? "undef" : val ? profile->truestring : profile->falsestring;
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

gboolean Profile_ReadStringUnquoted(Profile *profile, const char *section, const char *key, char **strp)
{
	char *str;
	gboolean found;
	char delim;
	
	found = Profile_GetLongValue(profile, section, key, &str, &delim);
	if (found == FALSE || str == NULL)
	{
		str = NULL;
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
