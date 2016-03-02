/* Implementation of the bindtextdomain(3) function
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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "windows_.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "gettextP.h"
#include "rcintl.h"

/* Some compilers, like SunOS4 cc, don't have offsetof in <stddef.h>.  */
#ifndef offsetof
# define offsetof(type,ident) ((size_t)&(((type*)0)->ident))
#endif

/* @@ end of prolog @@ */

/* Specifies the directory name *DIRNAMEP and the output codeset *CODESETP
   to be used for the DOMAINNAME message catalog.
   If *DIRNAMEP or *CODESETP is NULL, the corresponding attribute is not
   modified, only the current value is returned.
   If DIRNAMEP or CODESETP is NULL, the corresponding attribute is neither
   modified nor returned.  */
static void set_binding_values(const char *domainname, HINSTANCE *dirnamep, const char **codesetp)
{
	struct binding *binding;
	int modified;

	/* Some sanity checks.  */
	if (domainname == NULL || domainname[0] == '\0')
	{
		if (dirnamep)
			*dirnamep = NULL;
		if (codesetp)
			*codesetp = NULL;
		return;
	}

	modified = 0;

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

	if (binding != NULL)
	{
		if (dirnamep)
		{
			HINSTANCE dirname = *dirnamep;

			if (dirname == NULL)
				/* The current binding has be to returned.  */
				*dirnamep = binding->instance;
			else
			{
				/* The domain is already bound.  If the new value and the old
				   one are equal we simply do nothing.  Otherwise replace the
				   old binding.  */
				HINSTANCE result = binding->instance;

				if (result != dirname)
				{
					if (dirname == _rc_default_dirname())
						result = _rc_default_dirname();
					else
						result = dirname;

					if (likely(result != NULL))
					{
						binding->instance = result;
						modified = 1;
					}
				}
				*dirnamep = result;
			}
		}

		if (codesetp)
		{
			const char *codeset = *codesetp;

			if (codeset == NULL)
				/* The current binding has be to returned.  */
				*codesetp = binding->codeset.cptr;
			else
			{
				/* The domain is already bound.  If the new value and the old
				   one are equal we simply do nothing.  Otherwise replace the
				   old binding.  */
				char *result = binding->codeset.ptr;

				if (result == NULL || strcmp(codeset, result) != 0)
				{
					result = strdup(codeset);
					if (likely(result != NULL))
					{
						free(binding->codeset.ptr);

						binding->codeset.ptr = result;
						modified = 1;
					}
				}
				*codesetp = result;
			}
		}
	} else if ((dirnamep == NULL || *dirnamep == NULL) && (codesetp == NULL || *codesetp == NULL))
	{
		/* Simply return the default values.  */
		if (dirnamep)
			*dirnamep = _rc_default_dirname();
		if (codesetp)
			*codesetp = NULL;
	} else
	{
		/* We have to create a new binding.  */
		size_t len = strlen(domainname) + 1;

		struct binding *new_binding = (struct binding *) malloc(offsetof(struct binding, domainname) + len);

		if (unlikely(new_binding == NULL))
			goto failed;

		memcpy(new_binding->domainname, domainname, len);

		if (dirnamep)
		{
			HINSTANCE dirname = *dirnamep;

			if (dirname == NULL)
				/* The default value.  */
				dirname = _rc_default_dirname();
			else
			{
				if (dirname == _rc_default_dirname())
					dirname = _rc_default_dirname();
				else
				{
					HINSTANCE result;

					result = dirname;
					dirname = result;
				}
			}
			*dirnamep = dirname;
			new_binding->instance = dirname;
		} else
			/* The default value.  */
			new_binding->instance = _rc_default_dirname();

		if (codesetp)
		{
			const char *codeset = *codesetp;

			if (codeset != NULL)
			{
				char *result;

				result = strdup(codeset);
				if (unlikely(result == NULL))
					goto failed_codeset;
				codeset = result;
			}
			*codesetp = codeset;
			new_binding->codeset.cptr = codeset;
		} else
			new_binding->codeset.ptr = NULL;

		/* Now enqueue it.  */
		if (_rc_domain_bindings == NULL || strcmp(domainname, _rc_domain_bindings->domainname) < 0)
		{
			new_binding->next = _rc_domain_bindings;
			_rc_domain_bindings = new_binding;
		} else
		{
			binding = _rc_domain_bindings;
			while (binding->next != NULL && strcmp(domainname, binding->next->domainname) > 0)
				binding = binding->next;

			new_binding->next = binding->next;
			binding->next = new_binding;
		}

		modified = 1;

		/* Here we deal with memory allocation failures.  */
		if (0)
		{
		  failed_codeset:
			if (new_binding->instance != _rc_default_dirname())
				{}
			free(new_binding);
		  failed:
			if (dirnamep)
				*dirnamep = NULL;
			if (codesetp)
				*codesetp = NULL;
		}
	}

	/* If we modified any binding, we flush the caches.  */
	if (modified)
		++_rc_msg_cat_cntr;
}

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
HINSTANCE BINDTEXTDOMAIN(const char *domainname, HINSTANCE dirname)
{
	set_binding_values(domainname, &dirname, NULL);
	return dirname;
}

/* Specify the character encoding in which the messages from the
   DOMAINNAME message catalog will be returned.  */
const char *BIND_TEXTDOMAIN_CODESET(const char *domainname, const char *codeset)
{
	set_binding_values(domainname, NULL, &codeset);
	return codeset;
}
