#ifndef CC_FOR_BUILD
#include "hypdefs.h"
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *hyp_wchar_to_utf8(const wchar_t *str, size_t wlen)
{
	size_t len;
	char *dst, *p;
	h_unichar_t wc;
	
	if (str == NULL)
		return NULL;
	if (wlen == STR0TERM)
		wlen = wcslen(str);
	len = wlen * HYP_UTF8_CHARMAX + 1;
	dst = p = g_new(char, len);
	if (dst == NULL)
		return NULL;
	while (wlen)
	{
		wc = *str++;
		hyp_put_unichar(p, wc);
		wlen--;
	}
	*p++ = '\0';
	return g_renew(char, dst, p - dst);
}

/*** ---------------------------------------------------------------------- ***/

#if defined(_WIN32) && !GLIB_CHECK_VERSION(2, 40, 0) && !defined(CC_FOR_BUILD)
#include <shellapi.h>

char **g_win32_get_command_line(void)
{
	char **result;
	LPWSTR *args;
	int i, n;

	args = CommandLineToArgvW(GetCommandLineW(), &n);

	result = g_new(char *, n + 1);
	for (i = 0; i < n; i++)
		result[i] = hyp_wchar_to_utf8(args[i], STR0TERM);
	result[i] = NULL;

	LocalFree(args);
	return result;
}
#endif

