#include "hypdefs.h"
#include <mintbind.h>
#include <mint/cookie.h>
#include <mint/ssystem.h>

typedef struct {
	union {
		unsigned long id;
		long end;
	} cookie;
	long cookie_value;
} COOKIE;


static COOKIE *_get_jarptr(void)
{
	COOKIE *p;
	
	/*
	 * cookie jar ptr is longword aligned, thus
	 * we can use Setexc to fetch its value
	 */
	p = (COOKIE *)Setexc(0x5A0 / 4, VEC_INQUIRE);
	return p;
}


/******************************************************************************/
/* Cookie_JarInstalled()                                                      */
/* -------------------------------------------------------------------------- */
/* See if the cookie jar is installed.                                        */
/* -------------------------------------------------------------------------- */
/* Parameter:                                                                 */
/*                   none                                                     */
/* -------------------------------------------------------------------------- */
/* Return value:                                                              */
/*                   TRUE if jar is installed                                 */
/******************************************************************************/

int Cookie_JarInstalled(void)
{
	return _get_jarptr() != NULL;
}

/******************************************************************************/
/* Cookie_UsedEntries()                                                       */
/* -------------------------------------------------------------------------- */
/* Inquire the number of used cookie jar entries. The number includes         */
/* the null cookie, so a return value of 0 means that there is no             */
/* cookie jar at all.                                                         */
/* -------------------------------------------------------------------------- */
/* Parameter:                                                                 */
/*                   none                                                     */
/* -------------------------------------------------------------------------- */
/* Return value:                                                              */
/*                   number of used cookie jar entries                        */
/******************************************************************************/

int Cookie_UsedEntries(void)
{
	COOKIE *p;
	int entries = 0;
	
	p = _get_jarptr();
	while (p != NULL)
	{
		++entries;
		if (p->cookie.end == 0)
			break;
		p++;
	}
	return entries;
}

/******************************************************************************/
/* Cookie_JarSize()                                                           */
/* -------------------------------------------------------------------------- */
/* Inquire the total number of cookie jar entries.                            */
/* -------------------------------------------------------------------------- */
/* Parameter:                                                                 */
/*                   none                                                     */
/* -------------------------------------------------------------------------- */
/* Return value:                                                              */
/*                   total number of cookie jar entries                       */
/******************************************************************************/

int Cookie_JarSize(void)
{
	int size = 0;
	COOKIE *p;
	
	p = _get_jarptr();
	if (p != NULL)
	{
		while (p->cookie.end != 0)
		{
			p++;
		}
		size = (int)p->cookie_value;
	}
	return size;
}

/******************************************************************************/
/* Cookie_ResizeJar()                                                         */
/* -------------------------------------------------------------------------- */
/* Resize the cookie jar to the desired size.                                 */
/* -------------------------------------------------------------------------- */
/* Parameter:                                                                 */
/* -> newsize        desired cookie jar size, number of entries               */
/* -------------------------------------------------------------------------- */
/* Return value:                                                              */
/*                   TRUE if successful                                       */
/******************************************************************************/

int Cookie_ResizeJar(int newsize)
{
	/* NYI */
	UNUSED(newsize);
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static COOKIE *SearchJar(unsigned long id)
{
	COOKIE *p;
	
	p = _get_jarptr();
	while (p != NULL)
	{
		if (p->cookie.end == 0)
			p = NULL;
		else if (p->cookie.id == id)
			break;
		else
			p++;
	}
	return p;
}

/******************************************************************************/
/* Cookie_ReadJar()                                                           */
/* -------------------------------------------------------------------------- */
/* Read the value of the specified cookie.                                    */
/* -------------------------------------------------------------------------- */
/* Parameter:                                                                 */
/* -> id             cookie name                                              */
/* <- value          pointer to cookie value (may be NULL)                    */
/* -------------------------------------------------------------------------- */
/* Return value:                                                              */
/*                   TRUE if successful                                       */
/******************************************************************************/

int Cookie_ReadJar(unsigned long id, long *value)
{
	COOKIE *p;

	p = SearchJar(id);
	if (p != NULL)
	{
		if (value != NULL)
			*value = p->cookie_value;
		return TRUE;
	}
	return FALSE;
}


#ifdef _LIBC
/*
 * internal
 */
int __get_cpu(void)
{
	static long cpuval = -1;
	
	if (cpuval < 0)
	{
		if (!Cookie_ReadJar(C__CPU, &cpuval))
		{
			/* if _CPU cookie is not set, you loose */
			cpuval = 0;
		}
	}
	return (int)cpuval;
}
#endif


/*
 * MiNTLib compatibily
 */

int Getcookie(long cookie, long *val)
{
	return Cookie_ReadJar(cookie, val) ? C_FOUND : C_NOTFOUND;
}
