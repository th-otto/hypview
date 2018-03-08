#include "hypdefs.h"
#include <limits.h>


#include <ctype.h>
#include <string.h>
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

#ifdef _MSC_VER
#pragma warning (disable:4146) /* unary minus operator applied to unsigned type, result still unsigned */
#endif

#define STRTOL_LONG_MIN LLONG_MIN
#define STRTOL_LONG_MAX LLONG_MAX
#define STRTOL_ULONG_MAX ULLONG_MAX

#define TYPE_MINIMUM(t) ((t) ~TYPE_MAXIMUM (t))
#define TYPE_MAXIMUM(t) ((t) (((((t) 1 << (sizeof (t) * CHAR_BIT - 2)) - 1) * 2 + 1)))

# ifndef ULLONG_MAX
#  define ULLONG_MAX ((uintmax_t)-1)
# endif
# ifndef LLONG_MAX
#  define LLONG_MAX TYPE_MAXIMUM(intmax_t)
# endif
# ifndef LLONG_MIN
#  define LLONG_MIN TYPE_MINIMUM(intmax_t)
# endif

#define ISSPACE(Ch) isspace (Ch)
#define ISALPHA(Ch) isalpha (Ch)
#define TOUPPER(Ch) toupper (Ch)


intmax_t xs_strtoimax(const char *nptr, const char **endptr, int base)
{
	int negative;
	register uintmax_t cutoff;
	register unsigned int cutlim;
	register uintmax_t i;
	register const char *s;
	register unsigned char c;
	const char *save;
	int overflow;
	size_t len;
	
	errno = 0;
	if (nptr == NULL || (len = strlen(nptr)) == 0)
	{
		if (endptr)
			*endptr = nptr;
		return 0;
	}
	if (base < 0 || base == 1 || base > 36)
	{
		errno = EINVAL;
		return 0;
	}

	save = s = nptr;

	/* Skip white space.  */
	while (len && ISSPACE(*s))
		len--, ++s;
	if (len == 0)
		goto noconv;

	/* Check for a sign.  */
	if (*s == '-')
	{
		negative = 1;
		++s, --len;
	} else if (*s == '+')
	{
		negative = 0;
		++s, --len;
	} else
		negative = 0;

	/* Recognize number prefix and if BASE is zero, figure it out ourselves.  */
	if (len != 0 && *s == '0')
	{
		if ((base == 0 || base == 16) && len >= 2 && TOUPPER(s[1]) == 'X')
		{
			s += 2;
			len -= 2;
			base = 16;
		} else if (base == 0)
			base = 8;
	} else if (base == 0)
		base = 10;

	/* Save the pointer so we can check later if anything happened.  */
	save = s;

	cutoff = __extension__ STRTOL_ULONG_MAX / (uintmax_t) base;
	cutlim = (unsigned int)(__extension__ STRTOL_ULONG_MAX % (uintmax_t) base);

	overflow = 0;
	i = 0;
	for (;;)
	{
		if (len == 0)
			break;
		c = *s;
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (ISALPHA(c))
			c = TOUPPER(c) - 'A' + 10;
		else
			break;
		if ((int) c >= base)
			break;
		/* Check for overflow.  */
		if (i > cutoff || (i == cutoff && c > cutlim))
			overflow = 1;
		else
		{
			i *= (uintmax_t) base;
			i += c;
		}
		s++, --len;
	}

	/* Check if anything actually happened.  */
	if (s == save)
		goto noconv;

	/* Store in ENDPTR the address of one character
	   past the last character we converted.  */
	if (endptr != NULL)
		*endptr = s;

	/* Check for a value that is within the range of
	   'unsigned LONG int', but outside the range of 'LONG int'.  */
	if (overflow == 0
		&& i > (negative ? -((uintmax_t) (__extension__ STRTOL_LONG_MIN + 1)) + 1 : __extension__ (uintmax_t) STRTOL_LONG_MAX))
		overflow = 1;

	if (overflow)
	{
		errno = ERANGE;
		return negative ? __extension__ STRTOL_LONG_MIN : __extension__ STRTOL_LONG_MAX;
	}

	/* Return the result of the appropriate sign.  */
	return negative ? -i : i;

  noconv:
	/* We must handle a special case here: the base is 0 or 16 and the
	   first two characters are '0' and 'x', but the rest are no
	   hexadecimal digits.  This is no error case.  We return 0 and
	   ENDPTR points to the 'x'.  */
	if (endptr != NULL)
	{
		if (save - nptr >= 2 && TOUPPER(save[-1]) == 'X' && save[-2] == '0')
			*endptr = &save[-1];
		else
			/*  There was no number to convert.  */
			*endptr = nptr;
	}

	return 0;
}


char *xs_imaxtostr(uintmax_t val, char *buf, gboolean is_signed)
{
	char sign = '\0';
	
	if (is_signed && (intmax_t)val < 0)
	{
		sign = '-';
		val = -val;
	}
	buf += 50;
	*--buf = '\0';
	while (val > 0)
	{
		*--buf = (char)(val % 10) + '0';
		val /= 10;
	}
	*--buf = (char)val + '0';
	if (sign)
		*--buf = sign;
	return buf;
}
