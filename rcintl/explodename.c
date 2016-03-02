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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "windows_.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "gettextP.h"
#include "loadinfo.h"

/* On some strange systems still no definition of NULL is found.  Sigh!  */
#ifndef NULL
# if defined __STDC__ && __STDC__
#  define NULL ((void *) 0)
# else
#  define NULL 0
# endif
#endif

/* @@ end of prolog @@ */

/* Split a locale name NAME into a leading language part and all the
   rest.  Return a pointer to the first character after the language,
   i.e. to the first byte of the rest.  */
static char *_nl_find_language(char *name)
{
	while (name[0] != '\0' && name[0] != '_' && name[0] != '@' && name[0] != '.')
		++name;

	return name;
}


int _rc_explode_name(char *name, const char **language, const char **modifier, const char **territory, const char **codeset)
{
	char *cp;
	int mask;

	*modifier = NULL;
	*territory = NULL;
	*codeset = NULL;

	/* Now we determine the single parts of the locale name.  First
	   look for the language.  Termination symbols are `_', '.', and `@'.  */
	mask = 0;
	*language = cp = name;
	cp = _nl_find_language(name);

	if (*language == cp)
		/* This does not make sense: language has to be specified.  Use
		   this entry as it is without exploding.  Perhaps it is an alias.  */
		cp = strchr(*language, '\0');
	else if (cp[0] != '@')
	{
		if (cp[0] == '_')
		{
			/* Next is the territory.  */
			cp[0] = '\0';
			*territory = ++cp;

			while (cp[0] != '\0' && cp[0] != '.' && cp[0] != '@')
				++cp;

			mask |= XPG_TERRITORY;
		}

		if (cp[0] == '.')
		{
			/* Next is the codeset.  */
			cp[0] = '\0';
			*codeset = ++cp;

			while (cp[0] != '\0' && cp[0] != '@')
				++cp;

			mask |= XPG_CODESET;
		}
	}

	if (cp[0] == '@')
	{
		/* Next is the modifier.  */
		cp[0] = '\0';
		*modifier = ++cp;

		if (cp[0] != '\0')
			mask |= XPG_MODIFIER;
	}

	if (*territory != NULL && (*territory)[0] == '\0')
		mask &= ~XPG_TERRITORY;

	if (*codeset != NULL && (*codeset)[0] == '\0')
		mask &= ~XPG_CODESET;

	return mask;
}
