/* Handle list of needed message catalogs
   Copyright (C) 1995-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Ulrich Drepper <drepper@gnu.org>, 1995.

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

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#if defined HAVE_UNISTD_H || defined _LIBC
# include <unistd.h>
#endif

#include "gettextP.h"
#include "rcintl.h"

/* @@ end of prolog @@ */
/* List of already loaded domains.  */
static struct loaded_l10nfile *_rc_loaded_domains;


/* Return a data structure describing the message catalog described by
   the DOMAINNAME and CATEGORY parameters with respect to the currently
   established bindings.  */
struct loaded_l10nfile *_rc_find_domain(HINSTANCE dirname, char *locale, const char *domainname, struct binding *domainbinding)
{
	struct loaded_l10nfile *retval;
	const char *language;
	const char *modifier;
	const char *territory;
	const char *codeset;
	int mask;

	/* LOCALE can consist of up to four recognized parts for the XPG syntax:

	   language[_territory][.codeset][@modifier]

	   Beside the first part all of them are allowed to be missing.  If
	   the full specified locale is not found, the less specific one are
	   looked for.  The various parts will be stripped off according to
	   the following order:
	   (1) codeset
	   (2) normalized codeset
	   (3) territory
	   (4) modifier
	 */

	/* If we have already tested for this locale entry there has to
	   be one data set in the list of loaded domains.  */
	(void) domainname;
	retval = _rc_make_l10nflist(&_rc_loaded_domains, &dirname, 1, 0, locale, NULL, NULL, NULL, 0);

	if (retval != NULL)
	{
		/* We know something about this locale.  */
		int cnt;

		if (retval->decided <= 0)
			_rc_load_domain(retval, domainbinding);

		if (retval->domain != NULL)
			return retval;

		for (cnt = 0; retval->successor[cnt] != NULL; ++cnt)
		{
			if (retval->successor[cnt]->decided <= 0)
				_rc_load_domain(retval->successor[cnt], domainbinding);

			if (retval->successor[cnt]->domain != NULL)
				break;
		}

		return retval;
		/* NOTREACHED */
	}

	/* Now we determine the single parts of the locale name.  First
	   look for the language.  Termination symbols are `_', '.', and `@' if
	   we use XPG4 style, and `_', `+', and `,' if we use CEN syntax.  */
	mask = _rc_explode_name(locale, &language, &modifier, &territory, &codeset);
	if (mask == -1)
		/* This means we are out of core.  */
		return NULL;

	/* Create all possible locale entries which might be interested in
	   generalization.  */
	retval = _rc_make_l10nflist(&_rc_loaded_domains, &dirname, 1, mask, language, territory, codeset, modifier, 1);

	if (retval == NULL)
		/* This means we are out of core.  */
		goto out;

	if (retval->decided <= 0)
		_rc_load_domain(retval, domainbinding);
	if (retval->domain == NULL)
	{
		int cnt;

		for (cnt = 0; retval->successor[cnt] != NULL; ++cnt)
		{
			if (retval->successor[cnt]->decided <= 0)
				_rc_load_domain(retval->successor[cnt], domainbinding);
			if (retval->successor[cnt]->domain != NULL)
				break;
		}
	}

  out:

	return retval;
}


static void _rc_unload_domain(struct loaded_domain *domain)
{
	if (domain->plural.cptr != &GERMANIC_PLURAL)
		FREE_EXPRESSION(domain->plural.ptr);

	free(domain->malloced);

	(void) UnlockResource(domain->hglbl);
	FreeResource(domain->hglbl);

	free(domain);
}


/* This is called from iconv/gconv_db.c's free_mem, as locales must
   be freed before freeing gconv steps arrays.  */
void _rc_finddomain_subfreeres(void)
{
	struct loaded_l10nfile *runp = _rc_loaded_domains;

	while (runp != NULL)
	{
		struct loaded_l10nfile *here = runp;

		if (runp->domain != NULL)
			_rc_unload_domain(runp->domain);
		runp = runp->next;
		free(here);
	}
}

