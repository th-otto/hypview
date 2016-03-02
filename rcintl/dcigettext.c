/* Implementation of the internal dcigettext function.
   Copyright (C) 1995-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Tell glibc's <string.h> to provide a prototype for mempcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "windows_.h"

#include <sys/types.h>

#ifdef __GNUC__
# ifndef alloca
#   define alloca(size) __builtin_alloca(size)
# endif
# define HAVE_ALLOCA 1
#else
# ifdef _MSC_VER
#include <malloc.h>
#    define alloca _alloca
#  else
#    if defined HAVE_ALLOCA_H
#include <alloca.h>
#    else
#      ifdef _AIX
#pragma alloca
#      else
#        ifndef alloca
char *alloca();

#        endif /* alloca */
#      endif /* _AIX */
#    endif /* defined HAVE_ALLOCA_H */
#  endif /* _MSC_VER */
#endif /* __GNUC__ */

#include <errno.h>

#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <locale.h>
#include <signal.h>

#include "gettextP.h"
#include "plural-exp.h"
#include "rcintl.h"
#include "hash-string.h"

/* Some compilers, like SunOS4 cc, don't have offsetof in <stddef.h>.  */
#ifndef offsetof
#  define offsetof(type,ident) ((size_t)&(((type*)0)->ident))
#endif

/* @@ end of prolog @@ */

#if !defined(HAVE_STPCPY) && !defined(__GLIBC__)
#undef stpcpy
#    define stpcpy __stpcpy
static char *stpcpy(char *dest, const char *src);
# endif
#if !defined(HAVE_MEMPCPY) && !defined(__GLIBC__)
static void *mempcpy(void *dest, const void *src, size_t n);
#endif

/* Use a replacement if the system does not provide the `tsearch' function
   family.  */
#ifdef HAVE_TSEARCH
#include <search.h>
#else
#include "tsearch.h"
#endif

/* This is the type used for the search tree where known translations
   are stored.  */
struct known_translation_t
{
	/* Domain in which to search.  */
	const char *domainname;

	/* State of the catalog counter at the point the string was found.  */
	int counter;

	/* Catalog where the string was found.  */
	struct loaded_l10nfile *domain;

	/* And finally the translation.  */
	const char *translation;
	size_t translation_length;

	/* Pointer to the string in question.  */
	union
	{
		char appended[ZERO];			/* used if domain != NULL */
		const char *ptr;				/* used if domain == NULL */
	} msgid;
};

/* Root of the search tree with known translations.  */
static void *root;

/* Function to compare two entries in the table of known translations.  */
static int transcmp(const void *p1, const void *p2)
{
	const struct known_translation_t *s1;
	const struct known_translation_t *s2;
	int result;

	s1 = (const struct known_translation_t *) p1;
	s2 = (const struct known_translation_t *) p2;

	result = strcmp(s1->domain != NULL ? s1->msgid.appended : s1->msgid.ptr,
					s2->domain != NULL ? s2->msgid.appended : s2->msgid.ptr);
	if (result == 0)
	{
		result = strcmp(s1->domainname, s2->domainname);
	}
	return result;
}

/* Name of the default domain used for gettext(3) prior any call to
   textdomain(3).  The default value for this is "messages".  */
const char _rc_default_default_domain[] = "messages";

/* Value used as the default domain for gettext(3).  */
union _strptr _rc_current_default_domain = { _rc_default_default_domain };

/* Contains the default location of the message catalogs.  */
HINSTANCE _rc_default_dirname(void)
{
	static HINSTANCE __rc_default_dirname;
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __MSYS__
	if (__rc_default_dirname == 0)
		__rc_default_dirname = GetModuleHandleW(NULL);
#endif
	return __rc_default_dirname;
}

/* List with bindings of specific domains created by bindtextdomain()
   calls.  */
struct binding *_rc_domain_bindings;

/* Prototypes for local functions.  */
static const char *plural_lookup(const struct loaded_l10nfile *domain, unsigned long int n, const char *translation, size_t translation_len);

static const char *guess_category_value(int category, const char *categoryname);

/* For those loosing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
/* Nothing has to be done.  */
#  define freea(p)						/* nothing */
#else
#undef alloca
#  define alloca(size) (malloc (size))
#  define freea(p) free (p)
#endif /* have alloca */


/* Look up MSGID in the DOMAINNAME message catalog for the current
   CATEGORY locale and, if PLURAL is nonzero, search over string
   depending on the plural form determined by N.  */
const char *DCIGETTEXT(const char *domainname, const char *msgid1, const char *msgid2, int plural, unsigned long int n)
{
	struct loaded_l10nfile *domain;
	struct binding *binding;
	const char *categoryname = "LC_MESSAGES";
	const char *categoryvalue;
	HINSTANCE dirname;
	char *xdomainname;
	char *single_locale;
	const char *retval;
	size_t retlen;
	int saved_errno;
	struct known_translation_t search;
	struct known_translation_t **foundp = NULL;

	size_t domainname_len;

	/* If no real MSGID is given return NULL.  */
	if (msgid1 == NULL)
		return NULL;

	/* Preserve the `errno' value.  */
	saved_errno = errno;

	/* If DOMAINNAME is NULL, we are interested in the default domain.  If
	   CATEGORY is not LC_MESSAGES this might not make much sense but the
	   definition left this undefined.  */
	if (domainname == NULL)
		domainname = _rc_current_default_domain.cptr;

	/* Try to find the translation among those which we found at
	   some time.  */
	search.domain = NULL;
	search.msgid.ptr = msgid1;
	search.domainname = domainname;

	foundp = (struct known_translation_t **) tfind(&search, &root, transcmp);

	if (foundp != NULL && (*foundp)->counter == _rc_msg_cat_cntr)
	{
		/* Now deal with plural.  */
		if (plural)
			retval = plural_lookup((*foundp)->domain, n, (*foundp)->translation, (*foundp)->translation_length);
		else
			retval = (*foundp)->translation;

		__set_errno(saved_errno);
		return retval;
	}

	/* First find matching binding.  */
	for (binding = _rc_domain_bindings; binding != NULL; binding = binding->next)
	{
		int compare = strcmp(domainname, binding->domainname);

		if (compare == 0)
			/* We found it!  */
			break;
		if (compare < 0)
		{
			/* It is not in the list.  */
			binding = NULL;
			break;
		}
	}

	if (binding == NULL)
	{
		dirname = _rc_default_dirname();
	} else
	{
		dirname = binding->instance;
	}
	
	/* Now determine the symbolic name of CATEGORY and its value.  */
	categoryvalue = guess_category_value(LC_MESSAGES, categoryname);

	domainname_len = strlen(domainname);
	xdomainname = (char *) alloca(strlen(categoryname) + domainname_len + 5);

	stpcpy((char *) mempcpy(stpcpy(stpcpy(xdomainname, categoryname), "/"), domainname, domainname_len), ".mo");

	/* Creating working area.  */
	single_locale = (char *) alloca(strlen(categoryvalue) + 1);

	/* Search for the given string.  This is a loop because we perhaps
	   got an ordered list of languages to consider for the translation.  */
	while (1)
	{
		/* Make CATEGORYVALUE point to the next element of the list.  */
		while (categoryvalue[0] != '\0' && categoryvalue[0] == ':')
			++categoryvalue;
		if (categoryvalue[0] == '\0')
		{
			/* The whole contents of CATEGORYVALUE has been searched but
			   no valid entry has been found.  We solve this situation
			   by implicitly appending a "C" entry, i.e. no translation
			   will take place.  */
			single_locale[0] = 'C';
			single_locale[1] = '\0';
		} else
		{
			char *cp = single_locale;

			while (categoryvalue[0] != '\0' && categoryvalue[0] != ':')
				*cp++ = *categoryvalue++;
			*cp = '\0';
		}

		/* If the current locale value is C (or POSIX) we don't load a
		   domain.  Return the MSGID.  */
		if (strcmp(single_locale, "C") == 0 || strcmp(single_locale, "POSIX") == 0)
			break;

		/* Find structure describing the message catalog matching the
		   DOMAINNAME and CATEGORY.  */
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __MSYS__
		domain = _rc_find_domain(dirname, gl_locale_name_posify(single_locale), xdomainname, binding);
#else
		domain = _rc_find_domain(dirname, single_locale, xdomainname, binding);
#endif

		if (domain != NULL)
		{
			retval = _rc_find_msg(domain, binding, msgid1, &retlen);

			if (retval == NULL)
			{
				int cnt;

				for (cnt = 0; domain->successor[cnt] != NULL; ++cnt)
				{
					retval = _rc_find_msg(domain->successor[cnt], binding, msgid1, &retlen);

					/* Resource problems are not fatal, instead we return no
					   translation.  */
					if (unlikely(retval == (const char *) -1))
						goto return_untranslated;

					if (retval != NULL)
					{
						domain = domain->successor[cnt];
						break;
					}
				}
			}

			/* Returning -1 means that some resource problem exists
			   (likely memory) and that the strings could not be
			   converted.  Return the original strings.  */
			if (unlikely(retval == (const char *) -1))
				break;

			if (retval != NULL)
			{
				/* Found the translation of MSGID1 in domain DOMAIN:
				   starting at RETVAL, RETLEN bytes.  */
				freea(single_locale);
				freea(xdomainname);
				if (foundp == NULL)
				{
					/* Create a new entry and add it to the search tree.  */
					size_t msgid_len;
					size_t size;
					struct known_translation_t *newp;

					msgid_len = strlen(msgid1) + 1;
					size = offsetof(struct known_translation_t, msgid) + msgid_len + domainname_len + 1;

					newp = (struct known_translation_t *) malloc(size);
					if (newp != NULL)
					{
						char *new_domainname;

						new_domainname = (char *) mempcpy(newp->msgid.appended, msgid1, msgid_len);
						memcpy(new_domainname, domainname, domainname_len + 1);
						newp->domainname = new_domainname;
						newp->counter = _rc_msg_cat_cntr;
						newp->domain = domain;
						newp->translation = retval;
						newp->translation_length = retlen;

						/* Insert the entry in the search tree.  */
						foundp = (struct known_translation_t **) tsearch(newp, &root, transcmp);

						if (unlikely(foundp == NULL || *foundp != newp))
							/* The insert failed.  */
							free(newp);
					}
				} else
				{
					/* We can update the existing entry.  */
					(*foundp)->counter = _rc_msg_cat_cntr;
					(*foundp)->domain = domain;
					(*foundp)->translation = retval;
					(*foundp)->translation_length = retlen;
				}

				__set_errno(saved_errno);

				/* Now deal with plural.  */
				if (plural)
					retval = plural_lookup(domain, n, retval, retlen);

				return retval;
			}
		}
	}

  return_untranslated:
	/* Return the untranslated MSGID.  */
	freea(single_locale);
	freea(xdomainname);
	__set_errno(saved_errno);
	return plural == 0 ? msgid1
			/* Use the Germanic plural rule.  */
			: n == 1 ? msgid1 : msgid2;
}


/* Look up the translation of msgid within DOMAIN_FILE and DOMAINBINDING.
   Return it if found.  Return NULL if not found or in case of a conversion
   failure (problem in the particular message catalog).  Return (char *) -1
   in case of a memory allocation failure during conversion (only if
   ENCODING != NULL resp. CONVERT == true).  */
const char *_rc_find_msg(struct loaded_l10nfile *domain_file, struct binding *domainbinding, const char *msgid, size_t *lengthp)
{
	const struct loaded_domain *domain;
	nls_uint32 nstrings;
	size_t act;
	const char *result;
	size_t resultlen;

	if (domain_file->decided <= 0)
		_rc_load_domain(domain_file, domainbinding);

	if (domain_file->domain == NULL)
		return NULL;

	domain = domain_file->domain;

	nstrings = domain->nstrings;

	/* Locate the MSGID and its translation.  */
	if (domain->hash_tab != NULL)
	{
		/* Use the hashing table.  */
		nls_uint32 len = (nls_uint32) strlen(msgid);
		nls_uint32 hash_val = __hash_string(msgid);
		nls_uint32 idx = hash_val % domain->hash_size;
		nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));

		while (1)
		{
			nls_uint32 nstr = W(domain->must_swap_hash_tab, domain->hash_tab[idx]);

			if (nstr == 0)
				/* Hash table entry is empty.  */
				return NULL;

			nstr--;

			/* Compare msgid with the original string at index nstr.
			   We compare the lengths with >=, not ==, because plural entries
			   are represented by strings with an embedded NUL.  */
			if (nstr < nstrings
				? W(domain->must_swap, domain->orig_tab[nstr].length) >= len
				&& (strcmp(msgid, (const char *)domain->header + W(domain->must_swap, domain->orig_tab[nstr].offset)) == 0)
				: domain->orig_sysdep_tab[nstr - nstrings].length > len
				&& (strcmp(msgid, domain->orig_sysdep_tab[nstr - nstrings].pointer) == 0))
			{
				act = nstr;
				goto found;
			}
			if (idx >= domain->hash_size - incr)
				idx -= domain->hash_size - incr;
			else
				idx += incr;
		}
		/* NOTREACHED */
	} else
	{
		/* Try the default method:  binary search in the sorted array of
		   messages.  */
		size_t top, bottom;

		bottom = 0;
		top = nstrings;
		while (bottom < top)
		{
			int cmp_val;

			act = (bottom + top) / 2;
			cmp_val = strcmp(msgid, ((const char *)domain->header + W(domain->must_swap, domain->orig_tab[act].offset)));
			if (cmp_val < 0)
				top = act;
			else if (cmp_val > 0)
				bottom = act + 1;
			else
				goto found;
		}
		/* No translation was found.  */
		return NULL;
	}

  found:
	/* The translation was found at index ACT.  If we have to convert the
	   string to use a different character set, this is the time.  */
	if (act < nstrings)
	{
		result = (const char *)domain->header + W(domain->must_swap, domain->trans_tab[act].offset);
		resultlen = W(domain->must_swap, domain->trans_tab[act].length) + 1;
	} else
	{
		result = domain->trans_sysdep_tab[act - nstrings].pointer;
		resultlen = domain->trans_sysdep_tab[act - nstrings].length;
	}

	*lengthp = resultlen;
	return result;
}


/* Look up a plural variant.  */
static const char *plural_lookup(const struct loaded_l10nfile *domain, unsigned long int n, const char *translation, size_t translation_len)
{
	const struct loaded_domain *domaindata = domain->domain;
	unsigned long int index;
	const char *p;

	index = plural_eval(domaindata->plural.cptr, n);
	if (index >= domaindata->nplurals)
		/* This should never happen.  It means the plural expression and the
		   given maximum value do not match.  */
		index = 0;

	/* Skip INDEX strings at TRANSLATION.  */
	p = translation;
	while (index-- > 0)
	{
		p = strchr(p, '\0');
		/* And skip over the NUL byte.  */
		p++;

		if (p >= translation + translation_len)
			/* This should never happen.  It means the plural expression
			   evaluated to a value larger than the number of variants
			   available for MSGID1.  */
			return translation;
	}
	return p;
}


/* Guess value of current locale from value of the environment variables
   or system-dependent defaults.  */
static const char *guess_category_value(int category, const char *categoryname)
{
	const char *language;
	const char *locale;
	const char *language_default;
	int locale_defaulted;

	/* We use the settings in the following order:
	   1. The value of the environment variable 'LANGUAGE'.  This is a GNU
	   extension.  Its value can be a colon-separated list of locale names.
	   2. The value of the environment variable 'LC_ALL', 'LC_xxx', or 'LANG'.
	   More precisely, the first among these that is set to a non-empty value.
	   This is how POSIX specifies it.  The value is a single locale name.
	   3. A system-dependent preference list of languages.  Its value can be a
	   colon-separated list of locale names.
	   4. A system-dependent default locale name.
	   This way:
	   - System-dependent settings can be overridden by environment variables.
	   - If the system provides both a list of languages and a default locale,
	   the former is used.  */

	/* Fetch the locale name, through the POSIX method of looking to `LC_ALL',
	   `LC_xxx', and `LANG'.  On some systems this can be done by the
	   `setlocale' function itself.  */
	locale_defaulted = 0;
	locale = gl_locale_name_thread_unsafe(category, categoryname);
	if (locale == NULL)
	{
		locale = gl_locale_name_posix(category, categoryname);
		if (locale == NULL)
		{
			locale = gl_locale_name_default();
			locale_defaulted = 1;
		}
	}

	/* Ignore LANGUAGE and its system-dependent analogon if the locale is set
	   to "C" because
	   1. "C" locale usually uses the ASCII encoding, and most international
	   messages use non-ASCII characters. These characters get displayed
	   as question marks (if using glibc's iconv()) or as invalid 8-bit
	   characters (because other iconv()s refuse to convert most non-ASCII
	   characters to ASCII). In any case, the output is ugly.
	   2. The precise output of some programs in the "C" locale is specified
	   by POSIX and should not depend on environment variables like
	   "LANGUAGE" or system-dependent information.  We allow such programs
	   to use gettext().  */
	if (strcmp(locale, "C") == 0)
		return locale;

	/* The highest priority value is the value of the 'LANGUAGE' environment
	   variable.  */
	language = getenv("LANGUAGE");
	if (language != NULL && language[0] != '\0')
		return language;
	/* The next priority value is the locale name, if not defaulted.  */
	if (locale_defaulted)
	{
		/* The next priority value is the default language preferences list. */
		language_default = gl_locale_name_default();
		if (language_default != NULL)
			return language_default;
	}
	/* The least priority value is the locale name, if defaulted.  */
	return locale;
}


/* @@ begin of epilog @@ */

#if !defined(HAVE_STPCPY) && !defined(__GLIBC__)
static char *stpcpy(char *dest, const char *src)
{
	while ((*dest++ = *src++) != '\0')
		/* Do nothing. */ ;
	return dest - 1;
}
#endif


#if !defined(HAVE_MEMPCPY) && !defined(__GLIBC__)
static void *mempcpy(void *dest, const void *src, size_t n)
{
	return (void *) ((char *) memcpy(dest, src, n) + n);
}
#endif


/* If we want to free all resources we have to do some work at
   program's end.  */
void _rc_free_mem(void)
{
	while (_rc_domain_bindings != NULL)
	{
		struct binding *oldp = _rc_domain_bindings;

		_rc_domain_bindings = _rc_domain_bindings->next;
		free(oldp->codeset.ptr);
		free(oldp);
	}

	if (_rc_current_default_domain.cptr != _rc_default_default_domain)
		/* Yes, again a pointer comparison.  */
		free(_rc_current_default_domain.ptr);

	/* Remove the search tree with the known translations.  */
	tdestroy(root, free);
	root = NULL;
}
