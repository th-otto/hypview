#include "hypdefs.h"
#include <limits.h>
#include "hypdebug.h"


#if defined (__TOS__)
char *(*g_tos_get_bindir)(void);
#endif

#ifdef ENABLE_NLS

static char *localedir = NULL;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

const char *xs_dgettext(const char *domain, const char *msgid)
{
#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__MSYS__)
	const char *ret = rc_dgettext(domain, msgid);
	const char *ctxt;
	
	if (ret == msgid)
	{
		ctxt = strchr(msgid, GETTEXT_CONTEXT_GLUE[0]);
		if (ctxt != NULL)
			ret = rc_dgettext(domain, ctxt + 1);
	}
#else
	const char *ret = dgettext(domain, msgid);
	const char *ctxt;
	
	if (ret == msgid)
	{
		ctxt = strchr(msgid, GETTEXT_CONTEXT_GLUE[0]);
		if (ctxt != NULL)
			ret = dgettext(domain, ctxt + 1);
	}
#endif
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

const char *xs_dngettext(const char *domain, const char *msgid, const char *msgid_plural, unsigned long n)
{
#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__MSYS__)
	const char *ret = rc_dngettext(domain, msgid, msgid_plural, n);
	const char *ctxt;
	
	if (ret == msgid || ret == msgid_plural)
	{
		ctxt = strchr(msgid, GETTEXT_CONTEXT_GLUE[0]);
		if (ctxt != NULL)
			ret = rc_dngettext(domain, ctxt + 1, msgid_plural, n);
	}
#else
	const char *ret = dngettext(domain, msgid, msgid_plural, n);
	const char *ctxt;
	
	if (ret == msgid || ret == msgid_plural)
	{
		ctxt = strchr(msgid, GETTEXT_CONTEXT_GLUE[0]);
		if (ctxt != NULL)
			ret = dngettext(domain, ctxt + 1, msgid_plural, n);
	}
#endif
	return ret;
}

#endif /* ENABLE_NLS */

/*** ---------------------------------------------------------------------- ***/

char *g_get_package_bindir(void)
{
	char *retval;
	char *p;

#if defined(__WIN32__) || defined(_WIN32)
	wchar_t wc_fn[MAX_PATH];
	int len;
	
	if (!GetModuleFileNameW(NULL, wc_fn, MAX_PATH))
		return NULL;
	len = WideCharToMultiByte(CP_UTF8, 0, wc_fn, -1, NULL, 0, NULL, NULL);
	retval = g_new(char, len);
	WideCharToMultiByte(CP_UTF8, 0, wc_fn, -1, retval, len, NULL, NULL);
	if ((p = strrchr(retval, '\\')) != NULL)
		*p = '\0';
#elif defined (__TOS__)
	ASSERT(g_tos_get_bindir != 0);
	retval = (*g_tos_get_bindir)();
	if (retval != NULL)
	{
		if ((p = strrchr(retval, '\\')) != NULL)
		{
			*p = '\0';
		} else
		{
			g_free(retval);
			retval = NULL;
		}
	}
#else
	retval = NULL;
	{
		char path[PATH_MAX];
		ssize_t len;
		
		if ((len = readlink("/proc/self/exe", path, sizeof(path) - 1)) > 0)
		{
			path[len] = '\0';
			retval = g_strdup(path);
			if ((p = strrchr(retval, '/')) != NULL)
				*p = '\0';
		}
	}
#endif

#ifdef BINDIR
	if (retval == NULL)
		retval = g_strdup(BINDIR);
#endif

	if (retval == NULL)
		retval = g_strdup(".");
	
#ifdef G_WITH_CYGWIN
	/* In Cygwin we need to have POSIX paths */
	{
		char tmp[MAX_PATH];

		cygwin_conv_to_posix_path(retval, tmp);
		g_free(retval);
	    retval = g_strdup(tmp);
	}
#endif

#if G_DIR_SEPARATOR == '/'
	while ((p = strrchr(retval, '\\')) != NULL)
	   *p = '/';
#else
	while ((p = strrchr(retval, '/')) != NULL)
	   *p = '\\';
#endif

	return retval;
}

/*** ---------------------------------------------------------------------- ***/

char *g_get_package_installation_directory(void)
{
	char *retval;
	char *p;

	retval = g_get_package_bindir();

#if G_DIR_SEPARATOR == '/'
	p = strrchr(retval, '/');
#else
	p = strrchr(retval, '\\');
#endif

	if (p && (g_ascii_strcasecmp(p + 1, "bin") == 0 ||
        g_ascii_strcasecmp(p + 1, "bin64") == 0 ||
        g_ascii_strcasecmp(p + 1, "lib64") == 0 ||
	    g_ascii_strcasecmp(p + 1, "lib") == 0))
	    *p = '\0';

	return retval;
}

/*** ---------------------------------------------------------------------- ***/

#ifdef ENABLE_NLS
const char *xs_get_locale_dir(void)
{
	if (localedir == NULL)
	{
		char *root, *temp;
		const char *env;
		
		env = getenv("LOCALEDIR");
		if (env)
		{
			temp = g_strdup(env);
		} else
		{
			root = g_get_package_installation_directory();
			temp = g_new(char, strlen(root) + sizeof("/share/locale"));
#if G_DIR_SEPARATOR == '/'
			strcat(strcpy(temp, root), "/share/locale");
#else
			strcat(strcpy(temp, root), "\\share\\locale");
#endif
			g_free(root);
		}
		
#if defined(__WIN32__)
		/* localedir is passed to bindtextdomain() which isn't
		 * UTF-8-aware.
		 */
		{
			int len;
			wchar_t *wc_fn;
			
			len = MultiByteToWideChar(CP_UTF8, 0, temp, -1, NULL, 0);
			wc_fn = g_new(wchar_t, len);
			MultiByteToWideChar(CP_UTF8, 0, temp, -1, wc_fn, len);
			len = WideCharToMultiByte(CP_ACP, 0, wc_fn, -1, NULL, 0, NULL, NULL);
			localedir = g_new(char, len);
			WideCharToMultiByte(CP_ACP, 0, wc_fn, -1, localedir, len, NULL, NULL);
			g_free(wc_fn);
			g_free(temp);
		}
#else
		localedir = temp;
#endif
	}
	
	return localedir;
}

/*** ---------------------------------------------------------------------- ***/

void xs_locale_exit(void)
{
	if (localedir)
	{
		g_free(localedir);
		localedir = NULL;
	}
}

#endif
