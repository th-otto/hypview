#include "hypdefs.h"
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include "hypdebug.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif


#undef __set_errno
#undef __clear_errno
#define __set_errno(e) errno = e
#define __clear_errno() __set_errno(0)

#ifndef HAVE_GLIB

#ifndef NAN
# define NAN HUGE_VAL
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#ifndef g_strdup
char *g_strdup(const char *str)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	dst = g_new(char, strlen(str) + 1);
	if (dst == NULL)
		return NULL;
	return strcpy(dst, str);
}
#endif

/*** ---------------------------------------------------------------------- ***/

#ifndef g_strndup
char *g_strndup(const char *str, size_t len)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen(str);
	dst = g_new(char, len + 1);
	if (dst == NULL)
		return NULL;
	memcpy(dst, str, sizeof(char) * len);
	dst[len] = '\0';
	return dst;
}
#endif

/*** ---------------------------------------------------------------------- ***/

char *g_strconcat(const char *first, ...)
{
	va_list args;
	size_t len;
	const char *str;
	char *ret, *ptr;
	
	if (first == NULL)
		return NULL;
	len = strlen(first) + 1;
	va_start(args, first);
	for (;;)
	{
		str = va_arg(args, const char *);
		if (str == NULL)
			break;
		len += strlen(str);
	}
	va_end(args);
	ret = g_new(char, len);
	if (ret == NULL)
		return NULL;
	strcpy(ret, first);
	ptr = ret + strlen(ret);
	va_start(args, first);
	for (;;)
	{
		str = va_arg(args, const char *);
		if (str == NULL)
			break;
		strcpy(ptr, str);
		ptr += strlen(ptr);
	}
	va_end(args);
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

char *g_get_current_dir(void)
{
#if defined(__TOS__)
	char cwd[PATH_MAX];
	
	cwd[0] = '\0';
	if (Dgetcwd(cwd, 0, (int)sizeof(cwd)) < 0 &&
		Dgetpath(cwd, 0) < 0)
		cwd[0] = '\0';

	if (cwd[0] == '\0')
		strcpy(cwd, ".");
	return hyp_conv_to_utf8(HYP_CHARSET_ATARI, cwd, STR0TERM);
#elif defined(__WIN32__)
	size_t len;
	wchar_t *wcwd;
	char *cwd;
	
	len = GetCurrentDirectoryW(0, NULL);
	if (len == 0)
		return g_strdup(".");
	wcwd = g_new(wchar_t, len);
	if (wcwd == NULL)
		return NULL;
	wcwd[0] = '\0';
	if (GetCurrentDirectoryW(len, wcwd) == 0 || wcwd[0] == '\0')
	{
		g_free(wcwd);
		return g_strdup(".");
	}
	cwd = hyp_wchar_to_utf8(wcwd, len);
	g_free(wcwd);
	return cwd;
#else		
	char cwd[PATH_MAX];
	
	cwd[0] = '\0';
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		cwd[0] = '\0';

	if (cwd[0] == '\0')
		strcpy(cwd, ".");
	return hyp_conv_to_utf8(hyp_get_current_charset(), cwd, STR0TERM);
#endif
}

/*** ---------------------------------------------------------------------- ***/

char *g_strdup_vprintf(const char *format, va_list args)
{
	char *res;
	int len;
	size_t initsize;
	
/* Pure-C lacks vsnprintf() */
#if defined(__PUREC__) && defined(_PUREC_SOURCE)
	initsize = 1000000ul;
	res = g_new(char, initsize);
	while (res == NULL && initsize > 128)
	{
		initsize >>= 1;
		res = g_new(char, initsize);
	}
	if (res == NULL)
	{
		return NULL;
	}

	len = vsprintf(res, format, args);
	if (len >= initsize)
	{
		unreachable();
	}
	res = g_realloc(res, (len + 1) * sizeof(char));
#else
	va_list args2;

	initsize = 1024;
	res = g_new(char, initsize);
	if (res == NULL)
	{
		return NULL;
	}
	G_VA_COPY(args2, args);

	len = vsnprintf(res, initsize, format, args);
	if ((size_t)len >= initsize)
	{
		initsize = len + 1;
		res = (char *)g_realloc(res, initsize * sizeof(char));
		if (res != NULL)
		{
			len = vsnprintf(res, initsize, format, args2);
			if ((size_t)len >= initsize)
			{
				unreachable();
			}
		}
	}
	va_end(args2);
#endif
	
	return res;
}

/*** ---------------------------------------------------------------------- ***/

char *g_strdup_printf(const char *format, ...)
{
	va_list args;
	char *res;
	
	va_start(args, format);
	res = g_strdup_vprintf(format, args);
	va_end(args);
	return res;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Removes trailing whitespace from a string.
 */
char *g_strchomp(char *str)
{
	char *end;
	
	if (str == NULL)
		return NULL;
	end = str + strlen(str) - 1;
	while (end > str && (*end == ' ' || *end == '\t'))
		--end;
	*++end = '\0';
	return str;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Removes leading whitespace from a string, by moving the rest of the characters forward.
 */
char *g_strchug(char *str)
{
	char *src;
	
	if (str == NULL)
		return NULL;
	src = str;
	while (*src == ' ' || *src == '\t')
		src++;
	if (str != src)
		memmove(str, src, strlen(src) + 1);
	return str;
}

/*** ---------------------------------------------------------------------- ***/

void g_strfreev(char **str_array)
{
	if (str_array)
	{
		int i;

		for (i = 0; str_array[i] != NULL; i++)
			g_free(str_array[i]);

		g_free(str_array);
	}
}

/*** ---------------------------------------------------------------------- ***/

char **g_strsplit(const char *string, const char *delimiter, int max_tokens)
{
	char **str_array;
	const char *s;
	unsigned int n;
	const char *remainder;
	size_t delimiter_len;
	int tokens;
	
	if (string == NULL)
		return NULL;
	if (delimiter == NULL)
		return NULL;
	delimiter_len = strlen(delimiter);
	if (delimiter_len == 0)
		return NULL;

	if (max_tokens < 1)
		max_tokens = INT_MAX;

	remainder = string;
	n = 0;
	s = strstr(remainder, delimiter);
	tokens = max_tokens;
	if (s)
	{
		while (--tokens && s)
		{
			n++;
			remainder = s + delimiter_len;
			s = strstr(remainder, delimiter);
		}
	}
	if (*string)
	{
		n++;
	}

	str_array = g_new(char *, n + 1);
	if (str_array == NULL)
		return NULL;
	
	remainder = string;
	n = 0;
	s = strstr(remainder, delimiter);
	tokens = max_tokens;
	if (s)
	{
		while (--tokens && s)
		{
			size_t len;

			len = (const char *)s - remainder;
			str_array[n] = g_strndup(remainder, len);
			n++;
			remainder = s + delimiter_len;
			s = strstr(remainder, delimiter);
		}
	}
	if (*string)
	{
		str_array[n] = g_strdup(remainder);
		n++;
	}

	str_array[n] = NULL;

	return str_array;
}

/*** ---------------------------------------------------------------------- ***/

char *g_stpcpy(char *dest, const char *src)
{
	if (dest == NULL)
		return NULL;
	if (src == NULL)
		return NULL;
#ifdef HAVE_STPCPY
	return stpcpy(dest, src);
#else
	do
		*dest++ = *src;
	while (*src++ != '\0');

	return dest - 1;
#endif
}

/*** ---------------------------------------------------------------------- ***/

char *g_strjoinv(const char *separator, char **str_array)
{
	char *string;
	char *ptr;

	if (str_array == NULL)
		return NULL;

	if (separator == NULL)
		separator = "";

	if (*str_array)
	{
		int i;
		size_t len;
		size_t separator_len;

		separator_len = strlen(separator);
		/* First part, getting length */
		len = 1 + strlen(str_array[0]);
		for (i = 1; str_array[i] != NULL; i++)
			len += strlen(str_array[i]);
		len += separator_len * (i - 1);

		/* Second part, building string */
		string = g_new(char, len);
		ptr = g_stpcpy(string, *str_array);
		for (i = 1; str_array[i] != NULL; i++)
		{
			ptr = g_stpcpy(ptr, separator);
			ptr = g_stpcpy(ptr, str_array[i]);
		}
	} else
	{
		string = g_strdup("");
	}
	
	return string;
}

/*** ---------------------------------------------------------------------- ***/

#define ISSPACE(c)              ((c) == ' ' || (c) == '\f' || (c) == '\n' || \
                                 (c) == '\r' || (c) == '\t' || (c) == '\v')
#define ISUPPER(c)              ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c)              ((c) >= 'a' && (c) <= 'z')
#define ISALPHA(c)              (ISUPPER (c) || ISLOWER (c))
#define TOUPPER(c)              (ISLOWER (c) ? (c) - 'a' + 'A' : (c))
#define TOLOWER(c)              (ISUPPER (c) ? (c) - 'A' + 'a' : (c))

int g_ascii_strcasecmp(const char *s1, const char *s2)
{
	int c1, c2;

	while (*s1 && *s2)
    {
		c1 = (int)(unsigned char) TOLOWER (*s1);
		c2 = (int)(unsigned char) TOLOWER (*s2);
		if (c1 != c2)
			return c1 - c2;
		s1++; s2++;
	}

	return (((int)(unsigned char) *s1) - ((int)(unsigned char) *s2));
}

/*** ---------------------------------------------------------------------- ***/

int g_ascii_strncasecmp(const char *s1, const char *s2, size_t n)
{
	int c1, c2;
	
	while (n && *s1 && *s2)
	{
		n -= 1;
		c1 = (int)(unsigned char) TOLOWER (*s1);
		c2 = (int)(unsigned char) TOLOWER (*s2);
		if (c1 != c2)
			return c1 - c2;
		s1++; s2++;
	}
	
	if (n)
		return (((int) (unsigned char) *s1) - ((int) (unsigned char) *s2));
	return 0;
}

#endif /* HAVE_GLIB */

/*** ---------------------------------------------------------------------- ***/

gboolean g_path_is_absolute(const char *file_name)
{
	if (G_IS_DIR_SEPARATOR(file_name[0]))
	    return TRUE;

#ifdef G_DOSTYLE_PATHNAMES
	/* Recognize drive letter on native Windows */
	if ((isalpha(file_name[0]) || file_name[0] == '*') && file_name[1] == ':')
		return TRUE;
#endif

	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

char *g_build_filename(const char *first, ...)
{
	va_list args;
	size_t len;
	const char *str;
	char *ret, *ptr;
	
	if (first == NULL)
		return NULL;
	len = strlen(first) + 1;
	va_start(args, first);
	for (;;)
	{
		str = va_arg(args, const char *);
		if (str == NULL)
			break;
		while (G_IS_DIR_SEPARATOR(*str))
			str++;
		len += strlen(str) + 1;
	}
	va_end(args);
	ret = g_new(char, len);
	if (ret == NULL)
		return NULL;
	strcpy(ret, first);
	ptr = ret + strlen(ret);
	while (ptr > ret && G_IS_DIR_SEPARATOR(ptr[-1]))
		*--ptr = '\0';
	va_start(args, first);
	for (;;)
	{
		str = va_arg(args, const char *);
		if (str == NULL)
			break;
		while (G_IS_DIR_SEPARATOR(*str))
			str++;
		if (*str == '\0')
			continue;
		ptr = ret + strlen(ret);
		*ptr++ = G_DIR_SEPARATOR;
		strcpy(ptr, str);
		ptr += strlen(ptr);
		while (ptr > ret && G_IS_DIR_SEPARATOR(ptr[-1]))
			*--ptr = '\0';
	}
	va_end(args);
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

char *g_path_get_dirname(const char *path)
{
	const char *base;
	char *dir;
	char *ptr;
	
	if (path == NULL)
		return NULL;
	base = hyp_basename(path);
	dir = g_strndup(path, base - path);
	ptr = dir + strlen(dir);
	while (ptr > dir && G_IS_DIR_SEPARATOR(ptr[-1]))
		*--ptr = '\0';
	return dir;
}

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

void g_freep(char **str)
{
	if (*str != NULL)
	{
		g_free(*str);
		*str = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * like basename() or g_basename(),
 * but using this function throughout the package
 * because we sometimes have to deal with pathnames
 * from different OS than current one
 */
const char *hyp_basename(const char *path)
{
	const char *p;
	const char *base = NULL;
	
	if (path == NULL)
		return path;
	p = path;
	while (*p != '\0')
	{
		if (G_IS_DIR_SEPARATOR(*p))
			base = p + 1;
		++p;
	}
	if (base != NULL)
		return base;
	
	if (isalpha(path[0]) && path[1] == ':')
	{
    	/* can only be X:name, without slash */
    	path += 2;
	}
	
	return path;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_path_get_basename(const char *path)
{
	return g_strdup(hyp_basename(path));
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

/*** ---------------------------------------------------------------------- ***/

gboolean is_allupper(const char *str)
{
	while (*str)
	{
		if (!isupper(*str))
			return FALSE;
		str++;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

char *strslash(const char *str)
{
	char *p;
	p = strchr(str, '/');
	if (p == NULL)
		p = strchr(str, '\\');
	return p;
}

/*** ---------------------------------------------------------------------- ***/

char *strrslash(const char *str)
{
	char *p1, *p2;
	p1 = strrchr(str, '/');
	p2 = strrchr(str, '\\');
	if (p1 == NULL || p2 > p1)
		p1 = p2;
	return p1;
}

/*** ---------------------------------------------------------------------- ***/

void convslash(char *str)
{
	char *p = str;
	while (*p)
	{
		if (G_IS_DIR_SEPARATOR(*p))
			*p = G_DIR_SEPARATOR;
		p++;
	}
}

/*** ---------------------------------------------------------------------- ***/

char *replace_ext(const char *str, const char *from, const char *to)
{
	char *dst;
	char *p;
	
	if (str == NULL)
		return NULL;
	dst = g_new(char, strlen(str) + strlen(to) + 1);
	if (dst == NULL)
		return NULL;
	strcpy(dst, str);
	p = (char *)(void *)strrchr(hyp_basename(dst), '.');
	if (p == NULL)
		p = dst + strlen(dst);
	if (p != NULL)
	{
		if (from == NULL || *p == '\0' || strcasecmp(p, from) == 0)
		{
			if (*p != '\0' && is_allupper(p + 1))
			{
				strcpy(p, to);
				while (*p)
				{
					*p = toupper(*p);
					p++;
				}
			} else
			{
				strcpy(p, to);
			}
		}
	}
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

void chomp(char **str)
{
	if (*str != NULL)
	{
		g_strchomp(*str);
		g_strchug(*str);
	}
	if (empty(*str))
	{
		g_freep(str);
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean g_is_number(const char *val, gboolean is_unsigned)
{
	char *end;
	
	errno = 0;
	if (empty(val))
		return FALSE;
	if (is_unsigned)
		strtoul(val, &end, 0);
	else
		strtol(val, &end, 0);
	if (*end != '\0' || errno == ERANGE)
		return FALSE;
	return TRUE;
}
