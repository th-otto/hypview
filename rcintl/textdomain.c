/* Implementation of the textdomain(3) function.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "windows_.h"

#include <stdlib.h>
#include <string.h>

#include "gettextP.h"
#include "rcintl.h"

/* @@ end of prolog @@ */

/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
const char *TEXTDOMAIN(const char *domainname)
{
	char *new_domain;
	char *old_domain;

	/* A NULL pointer requests the current setting.  */
	if (domainname == NULL)
		return _rc_current_default_domain.cptr;

	old_domain = _rc_current_default_domain.ptr;

	/* If domain name is the null string set to default domain "messages".  */
	if (domainname[0] == '\0' || strcmp(domainname, _rc_default_default_domain) == 0)
	{
		_rc_current_default_domain.cptr = _rc_default_default_domain;
		new_domain = _rc_current_default_domain.ptr;
	} else if (strcmp(domainname, old_domain) == 0)
		/* This can happen and people will use it to signal that some
		   environment variable changed.  */
		new_domain = old_domain;
	else
	{
		/* If the following malloc fails `_rc_current_default_domain'
		   will be NULL.  This value will be returned and so signals we
		   are out of core.  */
		new_domain = strdup(domainname);

		if (new_domain != NULL)
			_rc_current_default_domain.cptr = new_domain;
	}

	/* We use this possibility to signal a change of the loaded catalogs
	   since this is most likely the case and there is no other easy we
	   to do it.  Do it only when the call was successful.  */
	if (new_domain != NULL)
	{
		++_rc_msg_cat_cntr;

		if (old_domain != new_domain && old_domain != _rc_default_default_domain)
			free(old_domain);
	}

	return new_domain;
}
