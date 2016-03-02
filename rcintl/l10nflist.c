/* Copyright (C) 1995-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

/* Tell glibc's <string.h> to provide a prototype for stpcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "windows_.h"

#include <stdio.h>
#include <string.h>

#include <ctype.h>
#include <sys/types.h>
#include <stdlib.h>

#include "gettextP.h"
#include "localename.h"

/* On some strange systems still no definition of NULL is found.  Sigh!  */
#ifndef NULL
# if defined __STDC__ && __STDC__
#  define NULL ((void *) 0)
# else
#  define NULL 0
# endif
#endif

/* @@ end of prolog @@ */

/* Define function which are usually not available.  */

/* Return number of bits set in X.  */
#ifndef ARCH_POP
static inline int pop(int x)
{
	/* We assume that no more than 16 bits are used.  */
	x = ((x & ~0x5555) >> 1) + (x & 0x5555);
	x = ((x & ~0x3333) >> 2) + (x & 0x3333);
	x = ((x >> 4) + x) & 0x0f0f;
	x = ((x >> 8) + x) & 0xff;

	return x;
}
#endif


struct loaded_l10nfile *_rc_make_l10nflist(struct loaded_l10nfile **l10nfile_list,
										   const HINSTANCE *dirlist, size_t dirlist_len,
										   int mask, const char *language, const char *territory,
										   const char *codeset, const char *modifier, int do_allocate)
{
	struct loaded_l10nfile **lastp;
	struct loaded_l10nfile *retval;
	size_t dirlist_count;
	size_t entries;
	int cnt;
	char *lang = strdup(language);
	char *dot;
	
	dot = strchr(lang, '.');
	if (dot)
		*dot = '\0';
	dot = strchr(lang, '@');
	if (dot)
		*dot = '\0';
	/* Look in list of already loaded domains whether it is already
	   available.  */
	lastp = l10nfile_list;
	for (retval = *l10nfile_list; retval != NULL; retval = retval->next)
		if (retval->language.cptr != NULL)
		{
			int compare = strcmp(retval->language.cptr, lang);
			
			if (compare == 0)
				/* We found it!  */
				break;
			if (compare < 0)
			{
				/* It's not in the list.  */
				retval = NULL;
				break;
			}

			lastp = &retval->next;
		}

	if (retval != NULL || do_allocate == 0)
	{
		free(lang);
		return retval;
	}

	dirlist_count = dirlist_len;

	/* Allocate a new loaded_l10nfile.  */
	retval =
		(struct loaded_l10nfile *)
		malloc(sizeof(*retval)
			   + (((dirlist_count << pop(mask)) + (dirlist_count > 1 ? 1 : 0)) * sizeof(struct loaded_l10nfile *)));
	if (retval == NULL)
	{
		return NULL;
	}

	retval->instance = *dirlist;
	retval->language.ptr = lang;
	retval->langid = gl_locale_win32_langid_from_name(lang);
	
	/* We set retval->data to NULL here; it is filled in later.
	   Setting retval->decided to 1 here means that retval does not
	   correspond to a real file (dirlist_count > 1) or is not worth
	   looking up (if an unnormalized codeset was specified).  */
	retval->decided = dirlist_count > 1;
	retval->domain = NULL;

	retval->next = *lastp;
	*lastp = retval;

	entries = 0;
	/* Recurse to fill the inheritance list of RETVAL.
	   If the DIRLIST is a real list (i.e. DIRLIST_COUNT > 1), the RETVAL
	   entry does not correspond to a real file; retval->filename contains
	   colons.  In this case we loop across all elements of DIRLIST and
	   across all bit patterns dominated by MASK.
	   If the DIRLIST is a single directory or entirely redundant (i.e.
	   DIRLIST_COUNT == 1), we loop across all bit patterns dominated by
	   MASK, excluding MASK itself.
	   In either case, we loop down from MASK to 0.  This has the effect
	   that the extra bits in the locale name are dropped in this order:
	   first the modifier, then the territory, then the codeset, then the
	   normalized_codeset.  */
	for (cnt = dirlist_count > 1 ? mask : mask - 1; cnt >= 0; --cnt)
		if ((cnt & ~mask) == 0 && !((cnt & XPG_CODESET) != 0))
		{
			if (dirlist_count > 1)
			{
				/* Iterate over all elements of the DIRLIST.  */
				size_t i;
				HINSTANCE dir;
				
				for (i = 0; i < dirlist_len; i++)
				{
					dir = dirlist[i];
					retval->successor[entries++]
						= _rc_make_l10nflist(l10nfile_list, &dir, 1,
											 cnt, language, territory, codeset,
											 modifier, 1);
				}
			} else
			{
				retval->successor[entries++]
					= _rc_make_l10nflist(l10nfile_list, dirlist, dirlist_len, cnt, language, territory, codeset, modifier, 1);
			}
		}
	retval->successor[entries] = NULL;

	return retval;
}
