/*		GEMPD.C 		1/27/84 - 03/20/85		Lee Jay Lorenzen		*/
/*		merge High C vers. w. 2.2				8/21/87 		mdf 	*/

/*
 *		Copyright 1999, Caldera Thin Clients, Inc.
 *				  2002-2016 The EmuTOS development team
 *
 *		This software is licenced under the GNU Public License.
 *		Please see LICENSE.TXT for further information.
 *
 *				   Historical Copyright
 *		-------------------------------------------------------------
 *		GEM Application Environment Services			  Version 2.3
 *		Serial No.	XXXX-0000-654321			  All Rights Reserved
 *		Copyright (C) 1987						Digital Research Inc.
 *		-------------------------------------------------------------
 */

#include "aes.h"
#include "gemlib.h"
#include "gempd.h"


#if 0
/* returns the AESPD for the given index */
AESPD *pd_index(_WORD i)
{
	return i < 2 ? &D.g_int[i].a_pd : &D.g_acc[i - 2].a_pd;
}
#endif


static _BOOL fapd(const char *pname, _WORD pid, AESPD *ppd)
{
	_BOOL ret;
	char temp[AP_NAMELEN + 1];

	ret = FALSE;
	temp[AP_NAMELEN] = 0;
	if (pname != NULL)
	{
		memcpy(temp, ppd->p_name, AP_NAMELEN);
		ret = streq(pname, temp);
	} else
	{
		ret = ppd->p_pid == pid;
	}
	return ret;
}


/* returns the AESPD for the given name, or if pname is NULL for the given pid */
AESPD *fpdnm(const char *pname, _UWORD pid)
{
	_WORD i;

	for (i = 0; i < NUM_PDS; i++)
	{
		if (fapd(pname, pid, &D.g_pd[i]))
			return &D.g_pd[i];
	}
	for (i = 0; i < gl_naccs; i++)
	{
		if (fapd(pname, pid, &gl_pacc[i]->ac_pd))
			return &gl_pacc[i]->ac_pd;
	}
	return NULL;
}


static AESPD *getpd(void)
{
	AESPD *p;

	if (curpid < NUM_PDS)				/* get a new AESPD     */
	{
		p = &D.g_pd[curpid];
		p->p_pid = curpid++;
	} else								/* otherwise get it     */
	{									/* accessory's AESPD list  */
		p = &gl_pacc[gl_naccs]->ac_pd;
		p->p_pid = NUM_PDS + gl_naccs++;
	}

	p->p_uda->u_insuper = 1;
	/* return the pd we got */
	return p;
}


/*
 * name an AESPD from the 8 first chars of the given string, stopping at the first
 * '.' (remove the file extension)
 */
void p_nameit(AESPD *p, const char *pname)
{
	memset(p->p_name, ' ', AP_NAMELEN);
	strscn(pname, p->p_name, '.');
}


/* set the application directory of an AESPD */
void p_setappdir(AESPD *pd, const char *pfilespec)
{
	const char *p;
	const char *plast;
	char *pdest;

	/* find the position *after* the last backslash */
	for (p = plast = pfilespec; *p; )	/* assume no backslash */
	{
		if (*p++ == '\\')
			plast = p;			/* after backslash ... */
	}

	/* copy the directory name including the final backslash */
	for (pdest = pd->p_appdir, p = pfilespec; p < plast; )
		*pdest++ = *p++;
	*pdest = '\0';
}


AESPD *pstart(PFVOID pcode, const char *pfilespec, intptr_t ldaddr)
{
	AESPD *px;

	/* create process to execute it */
	px = getpd();
	px->p_ldaddr = ldaddr;

	/* copy in name of file */
	p_nameit(px, pfilespec);
	p_setappdir(px, pfilespec);

	/* set pcode to be the return address when this process runs */
	psetup(px, pcode);

	/* link him up: put it on top of the drl list */
	px->p_stat = PS_RUN;
	px->p_link = drl;
	drl = px;

	return px;
}


/* put pd pi into list *root at the end */
void insert_process(AESPD *pi, AESPD **root)
{
	AESPD *p, *q;

	/* find the end */
	for (p = (q = (AESPD *)root)->p_link; p; p = (q = p)->p_link)
		;

	/* link him in */
	pi->p_link = p;
	q->p_link = pi;
}
