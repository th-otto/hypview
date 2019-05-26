/*	GEMBASE.C	1/28/84	- 01/07/85	Lee Jay Lorenzen	*/

/*
 *	-------------------------------------------------------------
 *	GEM Application Environment Services		  Version 1.0
 *	Serial No.  XXXX-0000-654321		  All Rights Reserved
 *	Copyright (C) 1985			Digital Research Inc.
 *	-------------------------------------------------------------
 */

#include "aes.h"

AESPD *rlr;
AESPD *drl;
ACCPD *gl_pacc[NUM_ACCS];		/* total of 6 desk acc, 1 from rom */
_WORD gl_naccs;

_WORD curpid;
