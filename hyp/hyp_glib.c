#ifndef CC_FOR_BUILD
#include "hypdefs.h"
#endif
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif


#ifndef unreachable
# define unreachable() abort()
#endif

#ifndef HAVE_GLIB

#ifndef NAN
# define NAN HUGE_VAL
#endif

#undef min
#define	min(a, b)	((a) < (b) ? (a) : (b))
#undef max
#define	max(a, b)	((a) > (b) ? (a) : (b))

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

#ifndef CC_FOR_BUILD
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
#endif

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
	res = g_renew(char, res, (len + 1));
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
		res = g_renew(char, res, initsize);
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
	while (end > str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
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

unsigned int g_strv_length(char **str_array)
{
	int i = 0;
	if (str_array)
	{
		for (; str_array[i] != NULL; i++)
			;
	}
	return i;
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

/*** ---------------------------------------------------------------------- ***/

GSList *g_slist_remove(GSList *list, gconstpointer data)
{
	GSList *l, **last;
	
	for (last = &list; (l = *last) != NULL; last = &(*last)->next)
	{
		if (l->data == data)
		{
			*last = l->next;
			g_slist_free_1(l);
			break;
		}
	}
	return list;
}

/*** ---------------------------------------------------------------------- ***/

GSList *g_slist_prepend(GSList *list, gpointer data)
{
	GSList *l;
	
	l = g_new(GSList, 1);
	l->data = data;
	l->next = list;
	return l;
}

/*** ---------------------------------------------------------------------- ***/

GSList *g_slist_append(GSList *list, gpointer data)
{
	GSList *l, **last;
	
	for (last = &list; *last != NULL; last = &(*last)->next)
		;
	l = g_new(GSList, 1);
	l->data = data;
	l->next = NULL;
	*last = l;
	return list;
}

/*** ---------------------------------------------------------------------- ***/

void g_slist_free_full(GSList *list, void (*freefunc)(void *))
{
	GSList *l, *next;
	
	for (l = list; l; l = next)
	{
		next = l->next;
		if (freefunc)
 			freefunc(l->data);
		g_free(l);
	}
}

/*** ---------------------------------------------------------------------- ***/

void g_slist_free(GSList *list)
{
	g_slist_free_full(list, 0);
}

/*** ---------------------------------------------------------------------- ***/

int g_ascii_xdigit_value(char c)
{
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return -1;
}

/*** ---------------------------------------------------------------------- ***/

#define MY_MAXSIZE ((gsize)-1)

static inline gsize nearest_power(gsize base, gsize num)
{
	if (num > MY_MAXSIZE / 2)
	{
		return MY_MAXSIZE;
	} else
	{
		gsize n = base;

		while (n < num)
			n <<= 1;

		return n;
	}
}

static void g_string_maybe_expand(GString *string, gsize len)
{
	if (string->len + len >= string->allocated_len)
	{
		string->allocated_len = nearest_power(1, string->len + len + 1);
		string->str = g_renew(char, string->str, string->allocated_len);
	}
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_insert_c(GString *string, gssize pos, char c)
{
	if (string == NULL)
		return NULL;

	g_string_maybe_expand(string, 1);

	if (pos < 0)
		pos = string->len;
	else if ((gsize)pos > string->len)
		return string;

	/* If not just an append, move the old stuff */
	if ((gsize)pos < string->len)
		memmove(string->str + pos + 1, string->str + pos, string->len - pos);

	string->str[pos] = c;
	string->len += 1;
	string->str[string->len] = 0;

	return string;
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_append_c(GString *string, char c)
{
	if (string == NULL)
		return NULL;

	return g_string_insert_c(string, -1, c);
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_sized_new(gsize dfl_size)
{
	GString *string = g_new(GString, 1);

	string->allocated_len = 0;
	string->len = 0;
	string->str = NULL;

	g_string_maybe_expand(string, max(dfl_size, 2));
	string->str[0] = 0;

	return string;
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_new(const char *init)
{
	GString *string;

	if (init == NULL || *init == '\0')
		string = g_string_sized_new(2);
	else
	{
		gsize len;

		len = strlen(init);
		string = g_string_sized_new(len + 2);

		g_string_append_len(string, init, len);
	}

	return string;
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_insert_len(GString *string, gssize pos, const char *val, gssize len)
{
	if (string == NULL)
		return NULL;
	if (len != 0 && val == NULL)
		return string;

	if (len == 0)
		return string;

	if (len < 0)
		len = strlen(val);

	if (pos < 0)
		pos = string->len;
	else if ((gsize)pos > string->len)
		return string;

	/* Check whether val represents a substring of string.
	 * This test probably violates chapter and verse of the C standards,
	 * since ">=" and "<=" are only valid when val really is a substring.
	 * In practice, it will work on modern archs.
	 */
	if (G_UNLIKELY(val >= string->str && val <= string->str + string->len))
	{
		gsize offset = val - string->str;
		gsize precount = 0;

		g_string_maybe_expand(string, len);
		val = string->str + offset;
		/* At this point, val is valid again.  */

		/* Open up space where we are going to insert.  */
		if ((gsize)pos < string->len)
			memmove(string->str + pos + len, string->str + pos, string->len - pos);

		/* Move the source part before the gap, if any.  */
		if (offset < (gsize)pos)
		{
			precount = min((gsize)len, (gsize)pos - offset);
			memcpy(string->str + pos, val, precount);
		}

		/* Move the source part after the gap, if any.  */
		if ((gsize)len > precount)
			memcpy(string->str + pos + precount, val + /* Already moved: */ precount + /* Space opened up: */ len,
				   len - precount);
	} else
	{
		g_string_maybe_expand(string, len);

		/* If we aren't appending at the end, move a hunk
		 * of the old string to the end, opening up space
		 */
		if ((gsize)pos < string->len)
			memmove(string->str + pos + len, string->str + pos, string->len - pos);

		/* insert the new string */
		if (len == 1)
			string->str[pos] = *val;
		else
			memcpy(string->str + pos, val, len);
	}

	string->len += len;

	string->str[string->len] = 0;

	return string;
}

/*** ---------------------------------------------------------------------- ***/

char *g_string_free(GString *string, gboolean free_segment)
{
	char *segment;

	if (string == NULL)
		return NULL;

	if (free_segment)
	{
		g_free(string->str);
		segment = NULL;
	} else
		segment = string->str;

	g_free(string);

	return segment;
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_append_len(GString *string, const char *val, gssize len)
{
	return g_string_insert_len(string, -1, val, len);
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_append(GString *string, const char *val)
{
	return g_string_insert_len(string, -1, val, -1);
}

/*** ---------------------------------------------------------------------- ***/

void g_string_append_vprintf(GString *string, const char *format, va_list args)
{
	char *buf;
	gssize len;

	if (string == NULL)
		return;
	if (format == NULL)
		return;

	buf = g_strdup_vprintf(format, args);
	if (buf == NULL)
		return;
	len = strlen(buf);
	
	if (len > 0)
	{
		g_string_maybe_expand(string, len);
		memcpy(string->str + string->len, buf, len + 1);
		string->len += len;
		g_free(buf);
	}
}

/*** ---------------------------------------------------------------------- ***/

void g_string_append_printf(GString *string, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	g_string_append_vprintf(string, format, args);
	va_end(args);
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_truncate(GString *string, gsize len)
{
	if (string == NULL)
		return NULL;

	string->len = min(len, string->len);
	string->str[string->len] = 0;

	return string;
}

/*** ---------------------------------------------------------------------- ***/

GString *g_string_set_size(GString *string, gsize len)
{
	if (string == NULL)
		return NULL;

	if (len >= string->allocated_len)
		g_string_maybe_expand(string, len - string->len);

	string->len = len;
	string->str[len] = 0;

	return string;
}

#endif /* HAVE_GLIB */

/*** ---------------------------------------------------------------------- ***/

static int unescape_character(const char *scanner)
{
	int first_digit;
	int second_digit;

	first_digit = g_ascii_xdigit_value(*scanner++);
	if (first_digit < 0)
		return -1;

	second_digit = g_ascii_xdigit_value(*scanner++);
	if (second_digit < 0)
		return -1;

	return (first_digit << 4) | second_digit;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_uri_unescape_segment(const char *escaped_string, const char *escaped_string_end, const char *illegal_characters)
{
	const char *in;
	char *out, *result;
	int character;

	if (escaped_string == NULL)
		return NULL;

	if (escaped_string_end == NULL)
		escaped_string_end = escaped_string + strlen(escaped_string);

	result = g_new(char, escaped_string_end - escaped_string + 1);

	out = result;
	for (in = escaped_string; in < escaped_string_end; in++)
	{
		character = *in;

		if (character == '%')
		{
			in++;

			if (escaped_string_end - in < 2)
			{
				/* Invalid escaped char (to short) */
				g_free(result);
				return NULL;
			}

			character = unescape_character(in);

			/* Check for an illegal character. We consider '\0' illegal here. */
			if (character <= 0 || (illegal_characters != NULL && strchr(illegal_characters, (char) character) != NULL))
			{
				g_free(result);
				return NULL;
			}

			in++;						/* The other char will be eaten in the loop header */
		} else if (character == '+')
		{
			character = ' ';
		}
		*out++ = (char) character;
	}

	*out = '\0';

	return result;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_uri_unescape_string(const char *escaped_string, const char *illegal_characters)
{
	return hyp_uri_unescape_segment(escaped_string, NULL, illegal_characters);
}

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
	convslash(ret);
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_path_get_dirname(const char *path)
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
	if (p != NULL)
	{
		while (*p)
		{
			if (G_IS_DIR_SEPARATOR(*p))
				*p = G_DIR_SEPARATOR;
			p++;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean convexternalslash(char *str)
{
	gboolean replaced = FALSE;
	
	char *p = str;
	if (p != NULL)
	{
		while (*p)
		{
			if (*p == '\\')
			{
				*p = '/';
				replaced = TRUE;
			}
			p++;
		}
	}
	return replaced;
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

