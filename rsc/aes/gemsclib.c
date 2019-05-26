/*      GEMSCLIB.C      07/10/84 - 02/02/85     Lee Lorenzen            */
/*      to 68k          03/08/85                Lowell Webster          */
/*      for 2.0         10/8/85  - 10/15/85     MDF                     */
/*      merge High C vers. w. 2.2               8/24/87         mdf     */

/*
 *       Copyright 1999, Caldera Thin Clients, Inc.
 *                 2002-2016 The EmuTOS development team
 *
 *       This software is licenced under the GNU Public License.
 *       Please see LICENSE.TXT for further information.
 *
 *                  Historical Copyright
 *       -------------------------------------------------------------
 *       GEM Application Environment Services              Version 2.3
 *       Serial No.  XXXX-0000-654321              All Rights Reserved
 *       Copyright (C) 1987                      Digital Research Inc.
 *       -------------------------------------------------------------
 */

#include "aes.h"


/*
 * AES #80 - scrp_read - get info about current scrap directory
 *
 * copies the current scrap directory path to the passed-in
 * address and returns TRUE if a valid path has already been set.
 */
_WORD sc_read(char *pscrap)
{
	strcpy(pscrap, D.g_scrap);
	return TRUE;
}


/*
 * AES #81 - scrp_write - sets the current scrap directory
 *
 * pscrap must be the address of a valid path. Returns
 * TRUE if no error occurs in validating the path name.
 */
_WORD sc_write(const char *pscrap)
{
	strcpy(D.g_scrap, pscrap);
	return TRUE;
}


#if CONF_WITH_PCGEM
/*
 *
 * AES #82 - scrap_clear() -- delete scrap files from current
 *           scrap directory
 *
 *		Assumes D.g_scrap holds a valid directory path.
 *		Returns TRUE on success.
 *
 */
_WORD sc_clear(void)
{
	char *ptmp;
	_WORD ret;
	static char const scrapmask[] = "\\SCRAP.*";

	if (D.g_scrap[0] == '\0')
		return FALSE;

	ptmp = D.g_scrap;
	while (*ptmp)							/* find null */
		ptmp++;

	strcpy(ptmp, scrapmask);				/* Add mask */

	dos_setdta(&D.g_dta); 					/* make sure dta ok */

	ret = dos_sfirst(D.g_scrap, F_SUBDIR);
	while(ret == 0)
	{
		strcpy(ptmp + 1, D.g_dta.d_fname);	/* Add file name */
		dos_delete(D.g_scrap);				/* delete scrap.* */
		strcpy(ptmp, scrapmask);			/* Add mask */
		ret = dos_snext();
	}

	*ptmp = '\0';							/* keep just path name */

	return TRUE;
}
#endif
